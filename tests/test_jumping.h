#ifndef TEST_JUMPING_H
#define TEST_JUMPING_H

#include <kaztest/kaztest.h>

class TestJumping : public TestCase {
public:
    void test_gravity_is_applied_when_in_the_air() {
        World world(0);
        Character character(&world, 20, 40);

        character.set_position(0, 0);
        character.set_ground_state(GROUND_STATE_ON_THE_GROUND);

        character.prepare(1);
        character.update(1);

        assert_equal(0, character.position().y); //No change in Y position

        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.prepare(1);
        character.update(1);

        assert_true(character.position().y < 0); //Should have moved downward
    }

    void test_acceleration_is_doubled_when_in_the_air() {
        Character character(nullptr, 40, 20);

        character.set_ground_state(GROUND_STATE_ON_THE_GROUND);
        character.move_right();
        character.prepare(1.0);

        float x_vel = character.velocity().x;
        assert_true(x_vel > 0);

        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.set_velocity(0, 0);
        character.set_gsp(0);

        assert_equal(0, character.velocity().x);

        character.move_right();
        character.prepare(1.0);

        float air_x_vel = character.velocity().x;

        assert_equal(x_vel * 2, air_x_vel);
    }

    void test_deceleration_matches_acceleration_in_the_air() {
        Character character(nullptr, 40, 20);

        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.move_right();
        character.prepare(1.0);

        float x_vel = character.velocity().x;
        assert_true(x_vel > 0);

        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.move_left();
        character.prepare(1.0);

        assert_equal(0, character.velocity().x);
    }

    void test_air_drag_applied() {
        float epsilon = std::numeric_limits<float>::epsilon();
        Character character(nullptr, 1.0, 0.5);
        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.set_velocity(DEFAULT_AIR_DRAG_MIN_X_SPEED - 0.001, DEFAULT_AIR_DRAG_MAX_Y_SPEED + 0.001);
        character.prepare(1);

        assert_close(DEFAULT_AIR_DRAG_MIN_X_SPEED - 0.001, character.velocity().x, epsilon); //Air drag not applied

        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.set_velocity(DEFAULT_AIR_DRAG_MIN_X_SPEED + 0.001, DEFAULT_AIR_DRAG_MAX_Y_SPEED + 0.001);
        character.prepare(1);

        assert_close(DEFAULT_AIR_DRAG_MIN_X_SPEED + 0.001, character.velocity().x, epsilon); //Still not applied

        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.set_velocity(DEFAULT_AIR_DRAG_MIN_X_SPEED + 0.001, DEFAULT_AIR_DRAG_MAX_Y_SPEED - 0.001);
        character.prepare(1);

        assert_close((DEFAULT_AIR_DRAG_MIN_X_SPEED + 0.001) * DEFAULT_AIR_DRAG_RATE, character.velocity().x, epsilon);
    }

    void test_y_velocity_is_limited() {
        Character character(nullptr, 1.0, 0.5);
        character.set_velocity(0, -100);
        character.prepare(0);

        assert_equal(DEFAULT_TOP_Y_SPEED_IN_M, fabs(character.velocity().y));
    }

    void test_jump_velocity_is_applied() {
        Character character(nullptr, 1.0, 0.5);

        character.set_ground_state(GROUND_STATE_ON_THE_GROUND);
        character.jump();
        character.prepare(0);
        assert_equal(DEFAULT_INITIAL_JUMP_IN_M, character.velocity().y);
        character.update_finished(0);

        character.set_ground_state(GROUND_STATE_IN_THE_AIR);
        character.prepare(0);
        assert_equal(DEFAULT_JUMP_CUT_OFF_IN_M, character.velocity().y);
    }
};

#endif // TEST_JUMPING_H
