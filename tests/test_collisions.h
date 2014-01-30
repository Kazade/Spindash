#ifndef TEST_COLLISIONS_H
#define TEST_COLLISIONS_H

#include <kaztest/kaztest.h>

#include "spindash/spindash.h"
#include "spindash/collision/collide.h"
#include "spindash/collision/ray_box.h"

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

private:

};

#endif // TEST_COLLISIONS_H
