#include <iostream>
#include <iomanip>
#include <tuple>
#include <string>
#include <krpc.hpp>
#include <krpc/services/space_center.hpp>

void Log(std::string msg, std::string type, std::string stage)
{
    std::cout << "[" + type + " - " + stage + "] " + msg << std::endl;
}

bool doStartupCheck(krpc::services::SpaceCenter::Vessel vessel)
{
    Log("Doing startup checks...", "STARTUP", "CHECK");

    
    auto situation = vessel.situation();
    if (situation == krpc::services::SpaceCenter::VesselSituation::pre_launch)
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

int main()
{
    // Initializing
    Log("Connecting to vessel...", "INIT", "INIT");
    krpc::Client conn = krpc::connect("Variables");
    krpc::services::SpaceCenter sc(&conn);
    Log("Successfully connected to vessel...", "INIT", "INIT");
    auto vessel = sc.active_vessel();
    auto ref_frame = vessel.orbit().body().reference_frame();
    doStartupCheck(vessel);
    

    Log("Press ENTER to start the launch sequence...", "INIT", "INIT");
    std::cin.get();
    // Setting up auto-pilot
    Log("Setting up auto-pilot", "LOG", "STAGE 0");
    vessel.auto_pilot().target_pitch_and_heading(90, 90);
    vessel.auto_pilot().engage();
    vessel.control().set_throttle(1);

    // Launch
    Log("Launch!", "LAUNCH", "LAUNCH");
    vessel.control().activate_next_stage();

    // Wait until we are out of fuel for this stage
    Log("Waiting until out of solid fuel...", "LOG", "STAGE 0");
    while (vessel.resources().amount("SolidFuel") > 0.1)
    {
        continue;
    }
    vessel.control().activate_next_stage();

    Log("Initiating gravity turn...", "LOG", "STAGE 1");
    vessel.auto_pilot().target_pitch_and_heading(60, 90);


    // Wait until we are out of liquidfuel
    Log("Waiting until out of liquid fuel...", "LOG", "STAGE 1");
    while (vessel.resources().amount("LiquidFuel") > 0.1)
    {
        continue;
    }
    vessel.control().activate_next_stage();


    Log("Waiting until reaching apoapsis altitude...", "LOG", "STAGE 2");
    while (vessel.orbit().time_to_apoapsis() >= 1)
    {
        continue;
    }
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

    Log("Setting auto-pilot to target retrograde...", "LOG", "STAGE 2");
    Log("Waiting until surface altitude reaches 2000 meters...", "LOG", "STAGE 2");
    // Waiting to deploy parachutes
    while (vessel.flight().surface_altitude() > 2000)
    {
        vessel.auto_pilot().set_target_direction(vessel.flight().retrograde());
    }

    Log("Deploying parachutes...", "LOG", "STAGE 2");
    //for (auto parachute : vessel.parts().parachutes())
    //    parachute.deploy();
    vessel.control().activate_next_stage();

    // Waiting to land
    while (vessel.flight(vessel.orbit().body().reference_frame()).vertical_speed() < -0.1)
    {
        continue;
    }
    Log("Landed!", "LOG", "STAGE 3");
}