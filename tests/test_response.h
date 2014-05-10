#ifndef TEST_RESPONSE_H
#define TEST_RESPONSE_H

#include <kaztest/kaztest.h>
#include "../spindash/spindash.h"

void spring_low_response(SDuint obj1, SDuint obj2, CollisionResponse* resp1, CollisionResponse* resp2, void* user_data) {
    *resp1 = COLLISION_RESPONSE_NONE;
    *resp2 = COLLISION_RESPONSE_SPRING_LOW;

    bool* collided = (bool*) user_data;
    *collided = true;
}

class CollisionResponseTests : public TestCase {
public:
    void test_response_none() {

    }

    void test_response_spring_low() {
        bool collided = false;

        SDuint world = sdWorldCreate();
        sdWorldSetObjectCollisionCallback(world, &spring_low_response, (void*)&collided);

        SDuint box1 = sdBoxCreate(world, 1.0, 1.0);
        sdObjectSetPosition(box1, 0, -1);
        sdObjectSetFixed(box1, true); //Make sure box1 can't move

        SDuint box2 = sdBoxCreate(world, 1.0, 1.0);
        sdObjectSetPosition(box2, 0, 0);

        //Run some game time, the boxes should collide
        sdWorldStep(world, 0.1);

        assert_true(collided); //Make sure we collided
        assert_equal(10.0, sdObjectGetSpeedY(box2)); //Box2 should've sprung
        assert_equal(0, sdObjectGetSpeedX(box2)); //No horizontal movement
        assert_equal(0, sdObjectGetSpeedX(box1)); //Box 1 shouldn't have moved
        assert_equal(0, sdObjectGetSpeedY(box1)); //Box 1 shouldn't have moved
    }

    void test_response_spring_high() {
        //Exactly the same as spring low, but with a power of 16, rather than 10
        bool collided = false;

        SDuint world = sdWorldCreate();
        sdWorldSetObjectCollisionCallback(world, &spring_low_response, (void*)&collided);

        SDuint box1 = sdBoxCreate(world, 1.0, 1.0);
        sdObjectSetPosition(box1, 0, -1);
        sdObjectSetFixed(box1, true); //Make sure box1 can't move

        SDuint box2 = sdBoxCreate(world, 1.0, 1.0);
        sdObjectSetPosition(box2, 0, 0);

        //Run some game time, the boxes should collide
        sdWorldStep(world, 0.1);

        assert_true(collided); //Make sure we collided
        assert_equal(16.0, sdObjectGetSpeedY(box2)); //Box2 should've sprung
        assert_equal(0, sdObjectGetSpeedX(box2)); //No horizontal movement
        assert_equal(0, sdObjectGetSpeedX(box1)); //Box 1 shouldn't have moved
        assert_equal(0, sdObjectGetSpeedY(box1)); //Box 1 shouldn't have moved
    }

    void test_response_hazard() {

    }
};

#endif // TEST_RESPONSE_H
