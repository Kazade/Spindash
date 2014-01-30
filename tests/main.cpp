

#include <functional>
#include <memory>
#include <kaztest/kaztest.h>

#include "/home/lukeb/Git/spindash/tests/test_character_response.h"
#include "/home/lukeb/Git/spindash/tests/test_collisions.h"
#include "/home/lukeb/Git/spindash/tests/test_running.h"
#include "/home/lukeb/Git/spindash/tests/test_solid_tiles.h"

int main(int argc, char* argv[]) {
    std::shared_ptr<TestRunner> runner(new TestRunner());
    
    
    runner->register_case<CollisionTest>(
        std::vector<void (CollisionTest::*)()>({&CollisionTest::test_ray_collisions_on_a_flat_plane}),
        {"CollisionTest::test_ray_collisions_on_a_flat_plane"}
    );


    runner->register_case<TestRunning>(
        std::vector<void (TestRunning::*)()>({&TestRunning::test_acceleration, &TestRunning::test_deceleration, &TestRunning::test_friction, &TestRunning::test_top_speed}),
        {"TestRunning::test_acceleration", "TestRunning::test_deceleration", "TestRunning::test_friction", "TestRunning::test_top_speed"}
    );


    runner->register_case<TestSolidTiles>(
        std::vector<void (TestSolidTiles::*)()>({&TestSolidTiles::test_character_ray_lengths_and_positions, &TestSolidTiles::test_position_changes_when_state_changes}),
        {"TestSolidTiles::test_character_ray_lengths_and_positions", "TestSolidTiles::test_position_changes_when_state_changes"}
    );


    runner->register_case<CollisionGeomTest>(
        std::vector<void (CollisionGeomTest::*)()>({&CollisionGeomTest::test_ray_box_floor_sensors}),
        {"CollisionGeomTest::test_ray_box_floor_sensors"}
    );

    
    return runner->run();
}


