

#include <functional>
#include <memory>
#include <kaztest/kaztest.h>

#include "/home/kazade/Git/spindash/tests/test_character_response.h"
#include "/home/kazade/Git/spindash/tests/test_jumping.h"
#include "/home/kazade/Git/spindash/tests/test_solid_tiles.h"
#include "/home/kazade/Git/spindash/tests/test_running.h"
#include "/home/kazade/Git/spindash/tests/test_collisions.h"

int main(int argc, char* argv[]) {
    std::shared_ptr<TestRunner> runner(new TestRunner());
    
    std::string test_case;
    if(argc > 1) {
        test_case = argv[1];
    }

    
    runner->register_case<CollisionTest>(
        std::vector<void (CollisionTest::*)()>({&CollisionTest::test_ray_collisions_on_a_flat_plane}),
        {"CollisionTest::test_ray_collisions_on_a_flat_plane"}
    );


    runner->register_case<TestSolidTiles>(
        std::vector<void (TestSolidTiles::*)()>({&TestSolidTiles::test_character_ray_lengths_and_positions, &TestSolidTiles::test_position_changes_when_state_changes, &TestSolidTiles::test_quadrant_switching, &TestSolidTiles::test_geom_changes_with_quadrant, &TestSolidTiles::test_ground_state, &TestSolidTiles::test_low_gsp_and_non_floor_quadrant_results_in_falling, &TestSolidTiles::test_a_b_sensors_only_work_with_negative_y_velocity, &TestSolidTiles::test_requisition_with_ground}),
        {"TestSolidTiles::test_character_ray_lengths_and_positions", "TestSolidTiles::test_position_changes_when_state_changes", "TestSolidTiles::test_quadrant_switching", "TestSolidTiles::test_geom_changes_with_quadrant", "TestSolidTiles::test_ground_state", "TestSolidTiles::test_low_gsp_and_non_floor_quadrant_results_in_falling", "TestSolidTiles::test_a_b_sensors_only_work_with_negative_y_velocity", "TestSolidTiles::test_requisition_with_ground"}
    );


    runner->register_case<TestJumping>(
        std::vector<void (TestJumping::*)()>({&TestJumping::test_gravity_is_applied_when_in_the_air, &TestJumping::test_acceleration_is_doubled_when_in_the_air, &TestJumping::test_deceleration_matches_acceleration_in_the_air, &TestJumping::test_air_drag_applied, &TestJumping::test_y_velocity_is_limited, &TestJumping::test_jump_velocity_is_applied}),
        {"TestJumping::test_gravity_is_applied_when_in_the_air", "TestJumping::test_acceleration_is_doubled_when_in_the_air", "TestJumping::test_deceleration_matches_acceleration_in_the_air", "TestJumping::test_air_drag_applied", "TestJumping::test_y_velocity_is_limited", "TestJumping::test_jump_velocity_is_applied"}
    );


    runner->register_case<TestRunning>(
        std::vector<void (TestRunning::*)()>({&TestRunning::test_acceleration, &TestRunning::test_deceleration, &TestRunning::test_friction, &TestRunning::test_top_speed}),
        {"TestRunning::test_acceleration", "TestRunning::test_deceleration", "TestRunning::test_friction", "TestRunning::test_top_speed"}
    );


    runner->register_case<CollisionGeomTest>(
        std::vector<void (CollisionGeomTest::*)()>({&CollisionGeomTest::test_ray_box_floor_sensors}),
        {"CollisionGeomTest::test_ray_box_floor_sensors"}
    );

    
    return runner->run(test_case);
}


