#ifndef TEST_COLLISIONS_H
#define TEST_COLLISIONS_H

#include <kaztest/kaztest.h>

#include "spindash/spindash.h"
#include "spindash/collision/collide.h"
#include "spindash/collision/ray_box.h"
#include "spindash/collision/box.h"

const SDVec2 box_points[] = {
    { -5, -5 },
    { 5, -5 },
    { 5, 0 },
    { -5, 0 }
};

class CollisionGeomTest : public TestCase {
public:
    void test_ray_box_floor_sensors() {
        RayBox ray_box(nullptr, 0.5f, 1.0f);
        ray_box.set_position(0, 0.5);

        Box floor(nullptr, 10.0f, 1.0f);
        floor.set_position(0, -0.5);

        std::vector<Collision> collisions = collide(&ray_box, &floor);

        assert_equal(2, collisions.size());
        assert_equal('A', collisions[0].a_ray);
        assert_equal('B', collisions[1].a_ray);
        assert_equal(0.0, collisions[0].point.y);
        assert_equal(0.0, collisions[1].point.y);
    }

    void test_negative_angle_calculations() {
        Character ch(nullptr, 0.5, 1.0);
        ch.set_position(5, 5);

        Triangle floor;
        kmVec2Fill(&floor.points()[0], 0, 0);
        kmVec2Fill(&floor.points()[1], 10, 0);
        kmVec2Fill(&floor.points()[2], 10, 10);

        std::vector<Collision> collisions = collide(&ch.geom(), &floor);

        assert_false(collisions.empty());

        ch.respond_to(collisions);

        assert_equal(315.0f, ch.rotation());
    }

    void test_positive_angle_calculations() {
        Character ch(nullptr, 0.5, 1.0);
        ch.set_position(-5, 5);

        Triangle floor;
        kmVec2Fill(&floor.points()[0], 0, 0);
        kmVec2Fill(&floor.points()[1], -10, 0);
        kmVec2Fill(&floor.points()[2], -10, 10);

        std::vector<Collision> collisions = collide(&ch.geom(), &floor);

        assert_false(collisions.empty());

        ch.respond_to(collisions);

        assert_equal(45.0f, ch.rotation());
    }
private:

};

#endif // TEST_COLLISIONS_H
