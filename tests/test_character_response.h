#ifndef TEST_CHARACTER_RESPONSE_H
#define TEST_CHARACTER_RESPONSE_H

#include <kaztest/kaztest.h>

#include "spindash/spindash.h"
#include "spindash/character.h"
#include "spindash/world.h"

static float world_scale = 1.0f / 40.0f;
static float frame_time = 1.0f / 60.0f;

class CollisionTest: public TestCase {
public:
    void test_ray_collisions_on_a_flat_plane() {
        SDuint world = sdWorldCreate();
        Character ch(get_world_by_id(world), 0.5, 1.0);

        ch.set_position(0.0f, 0.45f);

        //Create a collision for the a-ray only
        Collision a;
        kmVec2Fill(&a.point, -0.5 / 2.0, 0.0);
        kmVec2Fill(&a.a_normal, 0.0, -1.0);
        kmVec2Fill(&a.b_normal, 0.0, 1.0);
        a.object_a = &ch.geom();
        a.object_b = nullptr;
        a.a_ray = 'A';

        std::vector<Collision> collisions;
        collisions.push_back(a);

        ch.respond_to(collisions);

        //The play should now be at 0.5 units above the ground
        assert_close(0.5, sdObjectGetPositionY(ch.id()), 0.001);
        //Rotation should be zero
        assert_close(0.0, sdObjectGetRotation(ch.id()), 0.001);

        ///Retest with the B ray
        Collision b;
        kmVec2Fill(&b.point, 0.5 / 2.0, 0.0);
        kmVec2Fill(&b.a_normal, 0.0, -1.0);
        kmVec2Fill(&b.b_normal, 0.0, 1.0);
        b.object_a = &ch.geom();
        b.object_b = nullptr;
        b.a_ray = 'B';

        collisions.clear();
        collisions.push_back(b);
        ch.set_position(0.0f, 0.45f);

        ch.respond_to(collisions);

        //The play should now be at 0.5 units above the ground
        assert_close(0.5, sdObjectGetPositionY(ch.id()), 0.001);
        //Rotation should be zero
        assert_close(0.0, sdObjectGetRotation(ch.id()), 0.001);

        ///Finally check with both
        collisions.push_back(a);
        ch.set_position(0.0f, 0.45);

        ch.respond_to(collisions);

        //The play should now be at 0.5 units above the ground
        assert_close(0.5, sdObjectGetPositionY(ch.id()), 0.001);
        //Rotation should be zero
        assert_close(0.0, sdObjectGetRotation(ch.id()), 0.001);
    }
};

#endif // TEST_CHARACTER_RESPONSE_H
