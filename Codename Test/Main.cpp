#include <iostream>
#include <iomanip>
#include <tuple>
#include <krpc.hpp>
#include <krpc/services/space_center.hpp>

int main()
{
    // Initializing
    krpc::Client conn = krpc::connect("Suborbital-flight");
    krpc::services::SpaceCenter sc(&conn);
    auto vessel = sc.active_vessel();
    auto ref_frame = vessel.orbit().body().reference_frame();
    
    // Setting up auto-pilot
    vessel.auto_pilot().target_pitch_and_heading(90, 90);
    vessel.auto_pilot().engage();
    vessel.control().set_throttle(1);

    // Launch
    std::cout << "Launch!" << std::endl;
    vessel.control().activate_next_stage();
}