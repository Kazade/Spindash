#ifndef TEST_RUNNING_H
#define TEST_RUNNING_H

#include <kaztest/kaztest.h>
#include "spindash/spindash.h"

namespace TR {
    static const float EPSILON = 1.0f / 256.0f;

    //Sonic runs at 60 fps on NTSC
    static float frame_time = 1.0f / 60.0f;

    //Sonic is 40 pixels tall, we use this to scale the world so
    //that he is one unit high
    static float world_scale = 1.0f / 40.0f;
}

static void create_floor_plane(SDuint world) {
    kmVec2 points[6];

    kmVec2Fill(&points[0], -1000.0f, 0.0f);
    kmVec2Fill(&points[1], -1000.0f, -1.0f);
    kmVec2Fill(&points[2], 1000.0f, 0.0f);

    kmVec2Fill(&points[3], 1000.0f, 0.0f);
    kmVec2Fill(&points[4], -1000.0f, -1.0f);
    kmVec2Fill(&points[5], 1000.0f, -1.0f);

    sdWorldAddMesh(world, 2, points);
}

class TestRunning : public TestCase {
public:
    void test_acceleration() {
        SDuint world = sdWorldCreate();
        SDuint character = sdCharacterCreate(world);

        //Character by default should be 1.0f units high
        sdObjectSetPosition(character, 0.0f, 0.51f);
        create_floor_plane(world);

        //Check that the X and Y speed are zero
        assert_close(0.0f, sdObjectGetSpeedX(character), TR::EPSILON);
        assert_close(0.0f, sdObjectGetSpeedY(character), TR::EPSILON);

        //Start moving the character right
        sdCharacterStartMovingRight(character);
        sdWorldStep(world, TR::frame_time);

        //Sonic physics guide: Running
        float expected = 0.046875f * TR::world_scale;
        assert_close(expected, sdObjectGetSpeedX(character), TR::EPSILON);
        assert_true(sdCharacterIsGrounded(character));
        assert_close(0.0f, sdObjectGetSpeedY(character), TR::EPSILON);

        sdWorldStep(world, TR::frame_time);

        assert_close(expected * 2, sdObjectGetSpeedX(character), TR::EPSILON);
        assert_close(0.0f, sdObjectGetSpeedY(character), TR::EPSILON);
        sdCharacterStopMovingRight(character);

        //Reset the X speed
        sdObjectSetSpeedX(character, 0.0);
        sdCharacterSetGroundSpeed(character, 0.0);
        sdCharacterStartMovingLeft(character);

        sdWorldStep(world, TR::frame_time);
        assert_close(-expected, sdObjectGetSpeedX(character), TR::EPSILON);
        assert_close(0.0f, sdObjectGetSpeedY(character), TR::EPSILON);

        sdWorldStep(world, TR::frame_time * 2); //Move two steps
        assert_close(-expected * 3, sdObjectGetSpeedX(character), TR::EPSILON); //Now should be negative
        assert_close(0.0f, sdObjectGetSpeedY(character), TR::EPSILON);

        sdWorldDestroy(world);
    }

    void test_deceleration() {
        SDuint world = sdWorldCreate();
        SDuint character = sdCharacterCreate(world);

        //Character by default should be 1.0f units high
        sdObjectSetPosition(character, 0.0f, 0.5f);
        create_floor_plane(world);

        //Start moving the character right
        sdCharacterStartMovingRight(character);

        //Run 3 world steps
        for(uint32_t i = 0; i < 3; ++i) sdWorldStep(world, TR::frame_time);

        //Should have accelerated to 3 * expected
        float expected = 0.046875f * TR::world_scale;
        assert_close(expected * 3, sdObjectGetSpeedX(character), TR::EPSILON);

        //0.5f is the decelaration constant
        float new_expected = (expected * 3) - (0.5f * TR::world_scale);

        //We are moving right, and we've pressed left - this will register
        //internally as a direction change
        sdCharacterStopMovingRight(character);
        sdCharacterStartMovingLeft(character);
        sdWorldStep(world, TR::frame_time);
        sdCharacterStopMovingLeft(character);

        assert_close(new_expected, sdObjectGetSpeedX(character), TR::EPSILON);

        sdCharacterStartMovingRight(character);
        //Set the speed so that subtracting decelration would switch sign
        sdCharacterSetGroundSpeed(character, 0.1 * TR::world_scale);
        sdCharacterStopMovingRight(character);
        sdCharacterStartMovingLeft(character); //We have turned around

        sdWorldStep(world, TR::frame_time);
        //Emulate weird anomoly (see SPG Running - Deceleration)
        assert_close(-0.5 * TR::world_scale, sdCharacterGetGroundSpeed(character), TR::EPSILON);

        sdWorldDestroy(world);
    }

    void test_friction() {
        SDuint world = sdWorldCreate();
        SDuint character = sdCharacterCreate(world);

        //Character by default should be 1.0f units high
        sdObjectSetPosition(character, 0.0f, 0.5f);
        create_floor_plane(world);

        //Accelerate for 1 second
        sdCharacterStartMovingRight(character);
        for(uint32_t i = 0; i < 60; ++i) { sdWorldStep(world, TR::frame_time); }
        sdCharacterStopMovingRight(character);

        float initial = sdObjectGetSpeedX(character);

        for(uint32_t i = 0; i < 60; ++i) {
            sdWorldStep(world, TR::frame_time); //No buttons are being pressed
            assert_close(initial - (i * (0.046875 * TR::world_scale)), sdObjectGetSpeedX(character), TR::EPSILON);
        }

        //OK, now we are barely moving, applying friction would result
        //in a negative speed, it should be zero
        sdObjectSetSpeedX(character, 0.01 * TR::world_scale);
        sdWorldStep(world, TR::frame_time);
        assert_close(0.0f, sdObjectGetSpeedX(character), TR::EPSILON);

        sdWorldDestroy(world);
    }

    void test_top_speed() {
        SDuint world = sdWorldCreate();
        SDuint character = sdCharacterCreate(world);

        //Character by default should be 1.0f units high
        sdObjectSetPosition(character, 0.0f, 0.5f);
        create_floor_plane(world);

        sdCharacterStartMovingRight(character);
        //Run 10 seconds of game time, accelerating the whole time
        for(uint32_t i = 0; i < 60 * 10; ++i) {
            sdWorldStep(world, TR::frame_time);
        }

        assert_close(6.0f * TR::world_scale, sdObjectGetSpeedX(character), TR::EPSILON);

        //Set the speed faster than max
        sdCharacterSetGroundSpeed(character, 7.0f * TR::world_scale);
        sdWorldStep(world, TR::frame_time);
        //No acceleration should have been added
        assert_close(7.0f * TR::world_scale, sdCharacterGetGroundSpeed(character), TR::EPSILON);

        sdWorldDestroy(world);
    }
};

#endif // TEST_RUNNING_H
