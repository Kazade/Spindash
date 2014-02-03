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
};

#endif // TEST_JUMPING_H
