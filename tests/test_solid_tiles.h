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
};

#endif // TEST_SOLID_TILES_H
