#include <iostream>
#include <iomanip>
#include <tuple>
#include <string>
#include <krpc.hpp>
#include <krpc/services/krpc.hpp>
#include <krpc/services/space_center.hpp>

void Log(std::string msg, std::string type="INIT", std::string stage="INIT")
{
    std::cout << msg << std::endl;
}

bool DoStartupCheck(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Doing startup checks...", "STARTUP", "CHECK");
    typedef krpc::services::SpaceCenter::VesselSituation VesselSituation;

    auto situation = vessel.situation();
    if (situation == VesselSituation::pre_launch)
    {
        Log("Situation is pre-launch", "STARTUP", "CHECK");
    }
    else
    {
        Log("Situation is not pre-launch", "ERROR", "CHECK");
        return false;
    }

    Log("Successfully finished startup checks...", "STARTUP", "CHECK");
    return true;
}

void CheckCommunications(bool x)
{
    if (x == false)
    {
        // TODO: this function
        Log("NO COMMUNICATION", "ERROR", "ERROR");
    }
}

void CheckLiquidFuel(float x)
{
    if (x < 1)
    {
        // TODO: this function
        Log("OUT OF FUEL!");
    }
}

void RunLaunchSequence(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Targeting: pitch 90, heading 90", "AUTO-PILOT", "INIT");
    vessel.auto_pilot().target_pitch_and_heading(90, 90);
    Log("Engaging auto-pilot", "AUTO-PILOT", "INIT");
    vessel.auto_pilot().engage();
    Log("Setting throttle to 1", "CONTROL", "INIT");
    vessel.control().set_throttle(1);

    Log("Activating launch stage!", "CONTROL", "INIT");
    vessel.control().activate_next_stage();

    Log("Waiting until out of solid fuel...", "RESOURCES", "STAGE 0");
    while (vessel.resources().amount("SolidFuel") > 0.1)
    {
        continue;
    }
    Log("Activating next stage!", "CONTROL", "STAGE 0");
    vessel.control().activate_next_stage();
}

void RunSuborbitalSequence(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Targeting: pitch 60, heading 90", "AUTO-PILOT", "STAGE 1");
    vessel.auto_pilot().target_pitch_and_heading(60, 90);

    Log("Waiting until apoapsis reaches 72000 meters...", "ORBIT", "STAGE 1");
    while (vessel.orbit().apoapsis_altitude() < 72000)
    {
        continue;
    }
    Log("Setting throttle to 0", "CONTROL", "STAGE 1");
    vessel.control().set_throttle(0);
}

void RunOrbitingSequence(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Setting auto-pilot to target prograde", "AUTO-PILOT", "STAGE 2");
    Log("Waiting until 10 s left to reach apoapsis...", "ORBIT", "STAGE 2");
    while (vessel.orbit().time_to_apoapsis() >= 20)
        vessel.auto_pilot().set_target_direction(vessel.flight().prograde());

    Log("Setting throttle to 1", "CONTROL", "STAGE 2");
    vessel.control().set_throttle(1);

    Log("Setting auto-pilot to target prograde", "AUTO-PILOT", "STAGE 2");
    Log("Waiting until periapsis reaches 72000 meters", "ORBIT", "STAGE 2");
    while (vessel.orbit().periapsis_altitude() < 72000)
        vessel.auto_pilot().set_target_direction(vessel.flight().prograde());

    Log("Setting throttle to 0", "CONTROL", "STAGE 2");
    vessel.control().set_throttle(0);
}

void RunExperiments(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Deploying experiments...", "LOG", "STAGE 2");
    for (auto experiment : vessel.parts().experiments())
    {
        Log("Running experiment : " + experiment.part().name(), "LOG", "STAGE 2");
        try
        {
            experiment.run();
        }
        catch (int e)
        {
            Log(std::to_string(e), "ERROR", "STAGE 2");
        }
        Log("Successfully run experiment...", "LOG", "STAGE 2");
    }

}

void RunDeorbitingSequence(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Setting auto-pilot to target retrograde");
    vessel.auto_pilot().set_target_direction(vessel.flight().retrograde());
    
    Log("Waiting for auto-pilot to point in target direction...");
    while (vessel.auto_pilot().error() > 5) continue;

    Log("Setting throttle to 1");
    vessel.control().set_throttle(1);

    Log("Waiting until periapsis reaches 45000 meters...");
    while (vessel.orbit().periapsis_altitude() > 45000) continue;

    Log("Setting throttle to 0");
    vessel.control().set_throttle(0);

    Log("Activating next stage!");
    vessel.control().activate_next_stage();
}

void RunLandingSequence(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Setting auto-pilot to target retrograde...");
    Log("Waiting until surface altitude reaches 2000 meters...");
    // Need to put set_target_direction() in a while loop, as retrograde direction will be different
    while (vessel.flight().surface_altitude() > 2000)
        vessel.auto_pilot().set_target_direction(vessel.flight().retrograde());


    Log("Deploying parachutes...");
    for (auto parachute : vessel.parts().parachutes())
        parachute.deploy();

    // Waiting to land
    while (vessel.flight(vessel.orbit().body().reference_frame()).vertical_speed() < -0.1) continue;
    Log("Landed!", "LOG", "STAGE 3");
}

int main()
{
    Log("Connecting to vessel...", "INIT", "INIT");
    krpc::Client conn = krpc::connect("Variables");

    krpc::services::KRPC krpc(&conn);
    krpc::services::SpaceCenter sc(&conn);
    typedef krpc::services::KRPC::Expression Expr;

    Log("Successfully connected to vessel...", "INIT", "INIT");
    auto vessel = sc.active_vessel();
    auto ref_frame = vessel.orbit().body().reference_frame();

    if (!DoStartupCheck(vessel)) Log("Failed startup check!", "ERROR", "INIT");

    // ----- Stream setups -----
    auto checkCommunications = vessel.comms().can_communicate_stream();
    checkCommunications.add_callback(CheckCommunications);
    checkCommunications.set_rate(0.5); // Only checking communications every 2 seconds.
    checkCommunications.start();

    auto checkLiquidFuel = vessel.resources().amount_stream("LiquidFuel");
    checkLiquidFuel.add_callback(CheckLiquidFuel);
    checkLiquidFuel.start();


    Log("Press ENTER to start the launch sequence...", "INIT", "INIT");
    std::cin.get();

    // TODO: use references as parameters
    RunLaunchSequence(vessel);
    RunSuborbitalSequence(vessel);
    RunOrbitingSequence(vessel);
    RunExperiments(vessel);
    Log("Press ENTER to start the deorbiting sequence...", "INIT", "INIT");
    std::cin.get();
    RunDeorbitingSequence(vessel);
    RunLandingSequence(vessel);

}