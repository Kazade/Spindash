#ifndef TEST_SOLID_TILES_H
#define TEST_SOLID_TILES_H

#include "spindash/spindash.h"
#include "spindash/character.h"
#include "spindash/collision/collide.h"

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
        character.set_position(0, 0);
        character.update(0);
        character.respond_to({a_collision, b_collision});
        assert_equal(GROUND_STATE_ON_THE_GROUND, character.ground_state());

        //Simulate a collision on the A ray and the central (extra) E-ray
        character.set_position(0, 0);
        character.update(0);
        character.respond_to({a_collision, e_collision});
        assert_equal(GROUND_STATE_ON_THE_GROUND, character.ground_state());

        //Simulate a collision on the A ray only
        character.set_position(0, 0);
        character.update(0);
        character.respond_to({a_collision});
        assert_equal(GROUND_STATE_BALANCING_RIGHT, character.ground_state());

        //Simulate a collision on the B ray and the central (extra) E-ray
        character.set_position(0, 0);
        character.update(0);
        character.respond_to({e_collision, b_collision});
        assert_equal(GROUND_STATE_ON_THE_GROUND, character.ground_state());

        //Simulate a collision on the B ray only
        character.set_position(0, 0);
        character.update(0);
        character.respond_to({b_collision});
        assert_equal(GROUND_STATE_BALANCING_LEFT, character.ground_state());

        //Call respond_to without any collisions
        character.set_position(0, 0);
        character.update(0);
        character.respond_to(std::vector<Collision>());
        assert_equal(GROUND_STATE_IN_THE_AIR, character.ground_state());
    }

    void test_low_gsp_and_non_floor_quadrant_results_in_falling() {
        /*
         *  When the character is in any quadrant aside from QUADRANT_FLOOR, and gsp
         *  falls below 2.5, the ground_state should be set to IN_THE_AIR and gsp
         *  should be set to zero
         */

        Character character(nullptr, 40, 20);
        character.set_quadrant(QUADRANT_CEILING);
        character.set_ground_state(GROUND_STATE_ON_THE_GROUND);
        character.set_gsp(10);

        character.update_finished(0);

        assert_equal(QUADRANT_CEILING, character.quadrant());
        assert_true(character.is_grounded());

        character.set_gsp(0.01);
        character.update_finished(0);

        assert_equal(QUADRANT_FLOOR, character.quadrant());
        assert_false(character.is_grounded());
    }

    void test_a_b_sensors_only_work_with_negative_y_velocity() {
        //They only detect hits when y is <= 0 (e.g. not moving up)        
        Character character(nullptr, 20, 40);

        Triangle triangle;
        Collision a_collision;
        Collision b_collision;

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

        assert_equal(GROUND_STATE_IN_THE_AIR, character.ground_state());

        character.set_velocity(0, 4); //Moving upwards
        character.respond_to({a_collision, b_collision});

        assert_equal(GROUND_STATE_IN_THE_AIR, character.ground_state());

        character.set_velocity(0, -1); //Moving downards
        character.respond_to({a_collision, b_collision});

        assert_equal(GROUND_STATE_ON_THE_GROUND, character.ground_state());
    }

    void test_reacquisition_with_ground_under_22_point_5() {
        not_implemented();
    }

    void test_reacquisition_with_ground_under_45() {
        Character ch(nullptr, 0.5, 1.0);
        ch.set_position(-5, 5);
        ch.set_velocity(5, 0);
        ch.set_ground_state(GROUND_STATE_IN_THE_AIR);

        Triangle floor;
        kmVec2Fill(&floor.points()[0], 0, 0);
        kmVec2Fill(&floor.points()[1], -10, 0);
        kmVec2Fill(&floor.points()[2], -10, 9);

        std::vector<Collision> collisions = collide(&ch.geom(), &floor);

        assert_false(collisions.empty());
        assert_equal(QUADRANT_FLOOR, ch.quadrant());

        ch.respond_to(collisions);

        assert_true(ch.rotation() < 45.0);
        assert_true(ch.rotation() >= 22.5);

        assert_equal(ch.velocity().x, ch.gsp());

        ch.set_ground_state(GROUND_STATE_IN_THE_AIR);
        ch.set_position(-5, 5);
        ch.set_velocity(0, -5);
        ch.respond_to(collisions);

        assert_equal(-5 * 0.5 * -sgn(cos(kmDegreesToRadians(ch.rotation()))), ch.gsp());
    }

    void test_reacquisition_with_ground_at_45_plus() {
        //GSP is reset from xsp and ysp

        Character ch(nullptr, 0.5, 1.0);
        ch.set_position(-5, 5);
        ch.set_velocity(5, 0);
        ch.set_ground_state(GROUND_STATE_IN_THE_AIR);

        Triangle floor;
        kmVec2Fill(&floor.points()[0], 0, 0);
        kmVec2Fill(&floor.points()[1], -10, 0);
        kmVec2Fill(&floor.points()[2], -10, 10);

        std::vector<Collision> collisions = collide(&ch.geom(), &floor);

        assert_false(collisions.empty());
        assert_equal(QUADRANT_FLOOR, ch.quadrant());

        ch.respond_to(collisions);

        assert_close(45.0f, ch.rotation(), 0.01);
        assert_equal(ch.velocity().x, ch.gsp());

        ch.set_ground_state(GROUND_STATE_IN_THE_AIR);
        ch.set_position(-5, 5);
        ch.set_velocity(0, -5);
        ch.respond_to(collisions);

        assert_equal(-5 * -sgn(cos(kmDegreesToRadians(ch.rotation()))), ch.gsp());

        /*
         * When moving downward -
         *  if angle < 22.5: gsp = xsp;
         *  elif angle < 45.0:
         *     if xsp > ysp:
         *         gsp = xsp
         *     else:
         *         gsp = ysp * 0.5 * -sign(cos(angle))
         * else:
         *    if xsp > ysp:
         *         gsp = xsp
         *    else:
         *         gsp = ysp * -sign(cos(angle))
         */
    }

    void test_reacquisition_with_ceiling() {
        not_implemented();

        /*
         * if angle > 90 and angle < 135:
         *     set_ground_state(GROUND_STATE_ON_THE_CEILING)
         *     gsp = ysp * -sign(cos(angle))
         * else:
         *     ysp = 0
         */
    }

    void test_horizontal_collision_zeros_gsp() {
        not_implemented();
    }

    void test_horizontal_collision_in_the_air_zeros_x_vel() {
        not_implemented();
    }

    void test_horizontal_collision_repositions_character() {
        //When a character collides with a wall, they need to be pushed out by the width - distance
        not_implemented();
    }

    void test_pushing_anim_state_with_horiz_collision_and_x() {
        not_implemented();
    }

};

#endif // TEST_SOLID_TILES_H
