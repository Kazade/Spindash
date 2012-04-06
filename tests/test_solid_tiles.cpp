#include <unittest++/UnitTest++.h>

#include "spindash.h"

static const float EPSILON = 1.0f / 256.0f;

//Sonic runs at 60 fps on NTSC 
static float frame_time = 1.0f / 60.0f;

//Sonic is 40 pixels tall, we use this to scale the world so 
//that he is one unit high
static float world_scale = 1.0f / 40.0f;

TEST(test_horizontal_sensors) {
    SDuint world = sdWorldCreate();
    SDuint character = sdCharacterCreate(world);
    
    //Add a floor and a single wall to the right
    kmVec2 floor_triangle[3];
    kmVec2Fill(&floor_triangle[0], -1000, 1.0f);
    kmVec2Fill(&floor_triangle[1], 0.0f, -1.0f);
    kmVec2Fill(&floor_triangle[2], 1000, 1.0f);
    
    sdWorldAddTriangle(world, floor_triangle);
    
    float wall_x = 10.0f;
    
    kmVec2 wall_triangle[3];
    kmVec2Fill(&wall_triangle[0], wall_x, 100.0f);
    kmVec2Fill(&wall_triangle[1], wall_x, -100.0f);
    kmVec2Fill(&wall_triangle[2], 20, 0.0f);
    
    sdWorldAddTriangle(world, wall_triangle);
    
    float character_width = 20.0f * world_scale;

    //Position the character on the floor, next to the wall
    sdObjectSetPosition(character, wall_x - character_width, 0.5f); 
    sdWorldStep(world, frame_time); //Step the world
    
    //Nothing should have changed
    CHECK_CLOSE(wall_x - character_width, sdObjectGetPositionX(character), EPSILON);
    
    sdCharacterStartMovingRight(character);
    sdWorldStep(world, frame_time); //Step the world
    
    //Again nothing should have changed, the wall should've stopped us
    CHECK_CLOSE(wall_x - character_width, sdObjectGetPositionX(character), EPSILON);
    
    //Now, make us intersect the wall
    sdObjectSetPosition(character, (wall_x - character_width) + 0.25f, 0.5f); 
    sdWorldStep(world, frame_time);
    
    //Should be back where we were
    CHECK_CLOSE(wall_x - character_width, sdObjectGetPositionX(character), EPSILON);
    CHECK_CLOSE(0.0f, sdObjectGetSpeedX(character), EPSILON); //No speed
    
    sdWorldDestroy(world);
}

TEST(test_vertical_up_sensors) {

}

TEST(test_vertical_up_sensors_with_pass_thru) {

}

TEST(test_vertical_down_sensors) {
    SDuint world = sdWorldCreate();
    SDuint character = sdCharacterCreate(world);
    
    //Add a floor and a single wall to the right
    kmVec2 floor_triangle[3];
    kmVec2Fill(&floor_triangle[0], -1000.0f, 0.0f);
    kmVec2Fill(&floor_triangle[1], 0.0f, -10.0f);
    kmVec2Fill(&floor_triangle[2], 0.0f, 0.0f);
    
    sdWorldAddTriangle(world, floor_triangle);
    
    sdObjectSetPosition(character, -10.0f * world_scale, 0.0f);
    CHECK(sdCharacterIsGrounded(character));
    sdObjectSetPosition(character, 0.0f, 0.0f);
    CHECK(sdCharacterIsGrounded(character));
    sdObjectSetPosition(character, 10.0f * world_scale, 0.0f);
    CHECK(!sdCharacterIsGrounded(character));
    
    CHECK_CLOSE(0.0f, sdObjectGetRotation(character), EPSILON);
    
    sdWorldRemoveTriangles(world); //Clear the world
        
    //Add a 45 degree slope
    kmVec2Fill(&floor_triangle[0], 0.0f, 0.0f);
    kmVec2Fill(&floor_triangle[1], 100.0f, 0.0f);
    kmVec2Fill(&floor_triangle[2], 100.0f, 100.0f);
    sdWorldAddTriangle(world, floor_triangle);
    
    //Move the character above the slope
    sdObjectSetPosition(character, 50.0f, 50.0f);
    
    //Run 5 seconds of gameplay
    for(uint32_t i = 0; i < 5; ++i) sdWorldStep(world, 1.0f);
    
    //Now the character should have fallen, and be stood on the slope 
    //with a rotation of -45.0 (or 360.0f - 45.0f)
    CHECK(sdCharacterIsGrounded(character));
    CHECK_CLOSE(360.0f - 45.0f, sdObjectGetRotation(character), EPSILON);    
    
    sdWorldDestroy(world);
}

TEST(test_low_ceiling_prevents_jump) {
    /*
        "Sonic can't jump when there is a low ceiling above him. 
        If there is a collision detected with a sensor line stretching 
        from Sonic's X-9 to X+9, at Y-25, Sonic won't bother jumping at all." 
    */
}

TEST(test_height_changes) {
    /*
        Sonic has a different height at different times. When he's standing, 
        running, falling, or springing from a springboard, he's 40 pixels tall. 
        His Y position is always his centre, so that's why he stands 20 pixels 
        above the ground (and 20 pixels below ceilings when he hits into them, 
        etc). However, when he's jumping or rolling, he's only 30 pixels tall, 
        and he sets 15 pixels above the ground (and 15 pixels below ceiling, 
        etc). In the step in which Sonic rolls or jumps, the engine adds 5 to 
        his Y position so that even though he gets shorter and his centre 
        changes position, his bottom point will remain unchanged. 5 also has 
        to be subtracted from Y when he unrolls, or lands from a jump. The 
        camera system also has to keep this offset in mind, otherwise the view 
        will jump when Sonic changes height. 
    */
}

TEST(test_width_changes) {
    /*
        Sonic's A, B, C, and D sensors are described in this guide as being at 
        X-9 and Y+9. This is only true when walking, falling, springing, 
        and so on - any time he's not spinning. When Sonic is rolling or 
        jumping, they are at X-7 and X+7. However, his horizontal sensor line 
        remains the same whether curled up or not. 
    */
}
