

#include <functional>
#include <memory>
#include <kaztest/kaztest.h>

#include "/home/lukeb/Git/spindash/tests/test_character_response.h"
#include "/home/lukeb/Git/spindash/tests/test_running.h"
#include "/home/lukeb/Git/spindash/tests/test_solid_tiles.h"

int main(int argc, char* argv[]) {
    std::shared_ptr<TestRunner> runner(new TestRunner());
    
    
    runner->register_case<TestRunning>(
        std::vector<void (TestRunning::*)()>({&TestRunning::test_acceleration, &TestRunning::test_deceleration, &TestRunning::test_friction, &TestRunning::test_top_speed}), 
        {"TestRunning::test_acceleration", "TestRunning::test_deceleration", "TestRunning::test_friction", "TestRunning::test_top_speed"}
    );


    runner->register_case<TestSolidTiles>(
        std::vector<void (TestSolidTiles::*)()>({&TestSolidTiles::test_horizontal_sensors, &TestSolidTiles::test_vertical_up_sensors, &TestSolidTiles::test_vertical_up_sensors_with_pass_thru, &TestSolidTiles::test_vertical_down_sensors, &TestSolidTiles::test_slp_factor, &TestSolidTiles::test_low_ceiling_prevents_jump, &TestSolidTiles::test_height_changes, &TestSolidTiles::test_width_changes}), 
        {"TestSolidTiles::test_horizontal_sensors", "TestSolidTiles::test_vertical_up_sensors", "TestSolidTiles::test_vertical_up_sensors_with_pass_thru", "TestSolidTiles::test_vertical_down_sensors", "TestSolidTiles::test_slp_factor", "TestSolidTiles::test_low_ceiling_prevents_jump", "TestSolidTiles::test_height_changes", "TestSolidTiles::test_width_changes"}
    );

    
    return runner->run();
}


