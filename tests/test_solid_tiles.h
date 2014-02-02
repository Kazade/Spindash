#ifndef TEST_SOLID_TILES_H
#define TEST_SOLID_TILES_H

#include "spindash/spindash.h"
#include "spindash/character.h"

namespace TST{
    static const float EPSILON = 1.0f / 256.0f;

    //Sonic runs at 60 fps on NTSC
    static float frame_time = 1.0f / 60.0f;

    //Sonic is 40 pixels tall, we use this to scale the world so
    //that he is one unit high
    static float world_scale = 1.0f / 40.0f;
}

class TestSolidTiles : public TestCase {
public:
    void test_character_ray_lengths_and_positions() {
        Character character(nullptr, 20, 40);

        assert_equal(character.size(), CHARACTER_SIZE_STANDING);

        auto ray_box = character.ray_box();

        //Vertical sensors extend 16 pixels below feet/above head
        assert_equal(36, kmVec2Length(&ray_box.ray('A').dir));
        assert_equal(36, kmVec2Length(&ray_box.ray('B').dir));
        assert_equal(36, kmVec2Length(&ray_box.ray('C').dir));
        assert_equal(36, kmVec2Length(&ray_box.ray('D').dir));

        //Vertical sensors are positioned at +9 and -9 on the X
        assert_equal(-9, ray_box.ray('A').start.x);
        assert_equal(9, ray_box.ray('B').start.x);
        assert_equal(-9, ray_box.ray('C').start.x);
        assert_equal(9, ray_box.ray('D').start.x);

        //Vertical sensors have no X direction
        assert_equal(0, ray_box.ray('A').dir.x);
        assert_equal(0, ray_box.ray('B').dir.x);
        assert_equal(0, ray_box.ray('C').dir.x);
        assert_equal(0, ray_box.ray('D').dir.x);

        //Check vertical sensor directions
        assert_equal(-36, ray_box.ray('A').dir.y);
        assert_equal(-36, ray_box.ray('B').dir.y);
        assert_equal(36, ray_box.ray('C').dir.y);
        assert_equal(36, ray_box.ray('D').dir.y);

        //Things change when rolling/jumping etc.
        character.set_size(CHARACTER_SIZE_CROUCHING);
        ray_box = character.ray_box();

        //Vertical sensors extend 16 pixels below feet/above head
        assert_equal(15 + 16, kmVec2Length(&ray_box.ray('A').dir));
        assert_equal(15 + 16, kmVec2Length(&ray_box.ray('B').dir));
        assert_equal(15 + 16, kmVec2Length(&ray_box.ray('C').dir));
        assert_equal(15 + 16, kmVec2Length(&ray_box.ray('D').dir));

        //Vertical sensors are positioned at +7 and -7 on the X
        assert_close(-7, ray_box.ray('A').start.x, 0.25);
        assert_close(7, ray_box.ray('B').start.x, 0.25);
        assert_close(-7, ray_box.ray('C').start.x, 0.25);
        assert_close(7, ray_box.ray('D').start.x, 0.25);

        //Vertical sensors have no X direction
        assert_equal(0, ray_box.ray('A').dir.x);
        assert_equal(0, ray_box.ray('B').dir.x);
        assert_equal(0, ray_box.ray('C').dir.x);
        assert_equal(0, ray_box.ray('D').dir.x);

        //Check vertical sensor directions
        assert_equal(-(15 + 16), ray_box.ray('A').dir.y);
        assert_equal(-(15 + 16), ray_box.ray('B').dir.y);
        assert_equal((15 + 16), ray_box.ray('C').dir.y);
        assert_equal((15 + 16), ray_box.ray('D').dir.y);
    }

    void test_position_changes_when_state_changes() {
        Character character(nullptr, 20, 40);
        assert_equal(0, character.position().y);
        character.set_size(CHARACTER_SIZE_CROUCHING);
        assert_equal(-5, character.position().y);
        character.set_size(CHARACTER_SIZE_STANDING);
        assert_equal(0, character.position().y);
    }

    void test_quadrant_switching() {
        Character character(nullptr, 20, 40);
        Triangle triangle;
        assert_equal(QUADRANT_FLOOR, character.quadrant());

        std::vector<Collision> collisions(1);

        kmVec2Fill(&collisions[0].point, 0, 0);

        kmVec2Fill(&collisions[0].a_normal, 1, 0);
        kmVec2Fill(&collisions[0].b_normal, -1, 0);

        collisions[0].a_ray = 'A';
        collisions[0].object_a = &character.geom();
        collisions[0].object_b = &triangle;

        //Set the position to zero, and simulate a collision on
        //the right wall
        character.set_position(0, 0);
        character.respond_to(collisions);

        assert_equal(QUADRANT_RIGHT_WALL, character.quadrant());

        kmVec2Fill(&collisions[0].a_normal, 0, 1);
        kmVec2Fill(&collisions[0].b_normal, 0, -1);

        //Reset, and simulate a collision on the ceiling
        character.set_position(0, 0);
        character.respond_to(collisions);

        assert_equal(QUADRANT_CEILING, character.quadrant());

        kmVec2Fill(&collisions[0].a_normal, -1, 0);
        kmVec2Fill(&collisions[0].b_normal, 1, 0);

        //Reset, and simulate a collision on the left
        character.set_position(0, 0);
        character.respond_to(collisions);

        assert_equal(QUADRANT_LEFT_WALL, character.quadrant());
    }

    void test_geom_changes_with_quadrant() {
        //Test that when we switch quadrant, we also rotate the geom
        Character character(nullptr, 20, 40);

        character.set_quadrant(QUADRANT_RIGHT_WALL);

        auto ray_box = character.ray_box();

        //Should be no vertical direction now
        assert_close(0, ray_box.ray('A').dir.y, 0.001);
        assert_close(0, ray_box.ray('B').dir.y, 0.001);

        //Check vertical sensor directions
        assert_close(36, ray_box.ray('A').dir.x, 0.001);
        assert_close(36, ray_box.ray('B').dir.x, 0.001);

        //Should be no vertical direction now
        assert_close(0, ray_box.ray('C').dir.y, 0.001);
        assert_close(0, ray_box.ray('D').dir.y, 0.001);

        //Check vertical sensor directions
        assert_close(-36, ray_box.ray('C').dir.x, 0.001);
        assert_close(-36, ray_box.ray('D').dir.x, 0.001);

        character.set_quadrant(QUADRANT_CEILING);

        ray_box = character.ray_box();

        assert_close(9, ray_box.ray('A').start.x, 0.001);
        assert_close(0, ray_box.ray('A').start.y, 0.001);

        assert_close(-9, ray_box.ray('B').start.x, 0.001);
        assert_close(0, ray_box.ray('B').start.y, 0.001);

        //Should be no horizontal direction now
        assert_close(0, ray_box.ray('A').dir.x, 0.001);
        assert_close(0, ray_box.ray('B').dir.x, 0.001);

        //Check vertical sensor directions
        assert_close(36, ray_box.ray('A').dir.y, 0.001);
        assert_close(36, ray_box.ray('B').dir.y, 0.001);

        //Should be no vertical direction now
        assert_close(0, ray_box.ray('C').dir.x, 0.001);
        assert_close(0, ray_box.ray('D').dir.x, 0.001);

        //Check vertical sensor directions
        assert_close(-36, ray_box.ray('C').dir.y, 0.001);
        assert_close(-36, ray_box.ray('D').dir.y, 0.001);
    }

    void test_ground_state() {
        /**
         * Tests the ground state flag. We use an additional central ray that doesn't exist in
         * the original sonic games, to determine if the character's central point is over the
         * edge. In the later sonic games there is also an extreme balancing state, that is not implemented
         * here... it will require 2 additional downward rays.
         */
        Character character(nullptr, 20, 40);

        Triangle triangle;
        Collision a_collision;
        Collision b_collision;
        Collision e_collision;

        kmVec2Fill(&a_collision.point, 0, 0);
        kmVec2Fill(&a_collision.a_normal, 1, 0);
        kmVec2Fill(&a_collision.b_normal, -1, 0);
        a_collision.a_ray = 'A';
        a_collision.object_a = &character.geom();
        a_collision.object_b = &triangle;

        kmVec2Fill(&b_collision.point, 0, 0);
        kmVec2Fill(&b_collision.a_normal, 1, 0);
        kmVec2Fill(&b_collision.b_normal, -1, 0);
        b_collision.a_ray = 'B';
        b_collision.object_a = &character.geom();
        b_collision.object_b = &triangle;

        kmVec2Fill(&e_collision.point, 0, 0);
        kmVec2Fill(&e_collision.a_normal, 1, 0);
        kmVec2Fill(&e_collision.b_normal, -1, 0);
        e_collision.a_ray = 'E';
        e_collision.object_a = &character.geom();
        e_collision.object_b = &triangle;


        assert_equal(GROUND_STATE_IN_THE_AIR, character.ground_state());

        //Simulate collisions on both A and B rays
        character.respond_to({a_collision, b_collision});
        assert_equal(GROUND_STATE_ON_THE_GROUND, character.ground_state());

        //Simulate a collision on the A ray and the central (extra) E-ray
        character.respond_to({a_collision, e_collision});
        assert_equal(GROUND_STATE_ON_THE_GROUND, character.ground_state());

        //Simulate a collision on the A ray only
        character.respond_to({a_collision});
        assert_equal(GROUND_STATE_BALANCING_RIGHT, character.ground_state());

        //Simulate a collision on the B ray and the central (extra) E-ray
        character.respond_to({e_collision, b_collision});
        assert_equal(GROUND_STATE_ON_THE_GROUND, character.ground_state());

        //Simulate a collision on the B ray only
        character.respond_to({b_collision});
        assert_equal(GROUND_STATE_BALANCING_LEFT, character.ground_state());

        //Call respond_to without any collisions
        character.respond_to(std::vector<Collision>());
        assert_equal(GROUND_STATE_IN_THE_AIR, character.ground_state());
    }
};

#endif // TEST_SOLID_TILES_H
