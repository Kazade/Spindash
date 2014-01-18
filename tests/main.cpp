

#include <functional>
#include <memory>
#include <kaztest/kaztest.h>

#include "/home/kazade/Git/Spindash/tests/test_character_response.h"
#include "/home/kazade/Git/Spindash/tests/test_solid_tiles.h"
#include "/home/kazade/Git/Spindash/tests/test_running.h"

int main(int argc, char* argv[]) {
    std::shared_ptr<TestRunner> runner(new TestRunner());
    
    
    runner->register_case<TestRunning>(
        std::vector<void (TestRunning::*)()>({&TestRunning::test_acceleration, &TestRunning::test_deceleration, &TestRunning::test_friction, &TestRunning::test_top_speed}), 
        {"TestRunning::test_acceleration", "TestRunning::test_deceleration", "TestRunning::test_friction", "TestRunning::test_top_speed"}
    );

    
    return runner->run();
}


