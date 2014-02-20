#ifndef TEST_CAMERA_H
#define TEST_CAMERA_H

#include <kaztest/kaztest.h>

#include "spindash/spindash.h"
#include "spindash/character.h"

class TestCamera : public TestCase {
private:
    SDuint world_;
    SDuint character_;
public:
    void set_up() {
        world_ = sdWorldCreate();
        character_ = sdCharacterCreate(world_);
    }

    void tear_down() {
        sdWorldDestroy(world_);
    }

    void test_targetting() {
        SDfloat x, y;

        //Camera should default to zero
        sdWorldCameraGetPosition(world_, &x, &y);
        assert_equal(0, x);
        assert_equal(0, y);

        //Camera should target character
        sdObjectSetPosition(character_, 10, 10);
        sdWorldCameraTarget(world_, character_);
        sdWorldCameraGetPosition(world_, &x, &y);
        assert_equal(10, x);
        assert_equal(10, y);
    }

    void test_horizontal_freedom_of_movement() {
        //Camera should not move if the player moves inside this area
        const float DEFAULT_FREEDOM = (8.0 / 40.0);
        float x, y;

        sdWorldCameraTarget(world_, character_);
        sdWorldCameraGetPosition(world_, &x, &y);

        assert_equal(0, x);


        //Move the character a little, the camera shouldn't change
        sdObjectSetPosition(character_, DEFAULT_FREEDOM - 0.025, 0);
        sdWorldStep(world_, 1.0);

        sdWorldCameraGetPosition(world_, &x, &y);
        assert_equal(0, x);

        sdObjectSetPosition(character_, DEFAULT_FREEDOM + 0.025, 0);
        sdWorldStep(world_, 1.0);

        sdWorldCameraGetPosition(world_, &x, &y);

        assert_close(DEFAULT_FREEDOM + 0.025, x, 0.0001);
    }

    void test_vertical_freedom_of_movement() {
        //Camera should not move if the player moves inside this area
        not_implemented();
    }

    void test_maximum_speed() {
        //Camera should be left behind if the character is too fast
        not_implemented();
    }

    void test_y_speed_slower_on_grounded_target() {
        /*
         * In the original Sonic games, if the player was on the ground, but their y-speed
         * is less than 6, then the camera could move vertically at a speed up to 6.
         *
         * However, if the character is moving faster than that (e.g. downhill) the camera
         * would move at 16 (as normal). This is so that the camera catches up slowly when the player lands.
         */
        not_implemented();
    }

    void test_delay() {
        // This is the "spindash lag" section of the SPG, it's easier to just prevent the camera moving
        // for a small amount of time. So we'll do that, e.g.
        /*
         * sdWorldCameraDelayFor(X); //Don't move for X seconds
         */
        not_implemented();
    }

    void test_up_scroll() {
        not_implemented();
    }

    void test_down_scroll() {
        not_implemented();
    }
};

#endif // TEST_CAMERA_H
