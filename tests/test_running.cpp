#include <unittest++/UnitTest++.h>

#include "spindash.h"

const float EPSILON = 1.0f / 256.0f;

//Sonic runs at 60 fps on NTSC 
float frame_time = 1.0f / 60.0f;

//Sonic is 40 pixels tall, we use this to scale the world so 
//that he is one unit high
float world_scale = 1.0f / 40.0f;

void create_floor_plane(SDuint world) {
    kmVec2 points[6];
    
    kmVec2Fill(&points[0], -1000.0f, 0.0f);
    kmVec2Fill(&points[1], -1000.0f, -1.0f);
    kmVec2Fill(&points[2], 1000.0f, 0.0f); 

    kmVec2Fill(&points[3], 1000.0f, 0.0f);  
    kmVec2Fill(&points[4], -1000.0f, -1.0f);
    kmVec2Fill(&points[5], 1000.0f, -1.0f);
                      
    sdWorldAddMesh(world, 2, points);
}
    
TEST(test_acceleration) {
    SDuint world = sdWorldCreate();
    SDuint character = sdCharacterCreate(world);
    
    //Character by default should be 1.0f units high
    sdObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);
    
    //Check that the X and Y speed are zero
    CHECK_CLOSE(0.0f, sdObjectGetSpeedX(character), EPSILON);    
    CHECK_CLOSE(0.0f, sdObjectGetSpeedY(character), EPSILON);
 
    //Start moving the character right   
    sdCharacterStartMovingRight(character);    
    sdWorldStep(world, frame_time);
 
    //Sonic physics guide: Running   
    float expected = 0.046875f * world_scale;
    CHECK_CLOSE(expected, sdObjectGetSpeedX(character), EPSILON);
    CHECK_CLOSE(0.0f, sdObjectGetSpeedY(character), EPSILON);   
    
    sdWorldStep(world, frame_time);

    CHECK_CLOSE(expected * 2, sdObjectGetSpeedX(character), EPSILON);
    CHECK_CLOSE(0.0f, sdObjectGetSpeedY(character), EPSILON);       
    sdCharacterStopMovingRight(character);

    //Reset the X speed
    sdObjectSetSpeedX(character, 0.0f);
    sdCharacterStartMovingLeft(character);
    
    sdWorldStep(world, frame_time);
    CHECK_CLOSE(-expected, sdObjectGetSpeedX(character), EPSILON);
    CHECK_CLOSE(0.0f, sdObjectGetSpeedY(character), EPSILON);           
    
    sdWorldStep(world, frame_time * 2); //Move two steps
    CHECK_CLOSE(-expected * 3, sdObjectGetSpeedX(character), EPSILON); //Now should be negative
    CHECK_CLOSE(0.0f, sdObjectGetSpeedY(character), EPSILON);                       
    
    sdWorldDestroy(world);
}

TEST(test_deceleration) {
    SDuint world = sdWorldCreate();
    SDuint character = sdCharacterCreate(world);
    
    //Character by default should be 1.0f units high
    sdObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);    
    
    //Start moving the character right   
    sdCharacterStartMovingRight(character);    
    
    //Run 3 world steps
    for(uint32_t i = 0; i < 3; ++i) sdWorldStep(world, frame_time);    
    
    //Should have accelerated to 3 * expected
    float expected = 0.046875f * world_scale;
    CHECK_CLOSE(expected * 3, sdObjectGetSpeedX(character), EPSILON);
    
    //0.5f is the decelaration constant
    float new_expected = (expected * 3) - (0.5f * world_scale);
    
    //We are moving right, and we've pressed left - this will register
    //internally as a direction change
    sdCharacterStopMovingRight(character);
    sdCharacterStartMovingLeft(character);
    sdWorldStep(world, frame_time); 
    sdCharacterStopMovingLeft(character);    
        
    CHECK_CLOSE(new_expected, sdObjectGetSpeedX(character), EPSILON);
    
    sdCharacterStartMovingRight(character);
    //Set the speed so that subtracting decelration would switch sign
    sdObjectSetSpeedX(character, 0.1 * world_scale);
    sdCharacterStartMovingLeft(character); //We have turned around
    
    sdWorldStep(world, frame_time);
    //Emulate weird anomoly (see SPG Running - Deceleration)
    CHECK_CLOSE(-0.5 * world_scale, sdObjectGetSpeedX(character), EPSILON);
    
    sdWorldDestroy(world);
}

TEST(test_friction) {
    SDuint world = sdWorldCreate();
    SDuint character = sdCharacterCreate(world);
    
    //Character by default should be 1.0f units high
    sdObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);    
    
    //We've been accelerating for 1 second
    float initial = 0.046875f * world_scale * 60;
    sdObjectSetSpeedX(character, initial);
     
    for(uint32_t i = 0; i < 60; ++i) {
        sdWorldStep(world, frame_time); //No buttons are being pressed
        CHECK_CLOSE(initial - (i * (0.046875 * world_scale)), sdObjectGetSpeedX(character), EPSILON);        
    }
    
    //OK, now we are barely moving, applying friction would result
    //in a negative speed, it should be zero
    sdObjectSetSpeedX(character, 0.01 * world_scale);   
    sdWorldStep(world, frame_time);
    CHECK_CLOSE(0.0f, sdObjectGetSpeedX(character), EPSILON);
    
    sdWorldDestroy(world);
}

TEST(test_top_speed) {
    SDuint world = sdWorldCreate();
    SDuint character = sdCharacterCreate(world);
    
    //Character by default should be 1.0f units high
    sdObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);        
    
    sdCharacterStartMovingRight(character);
    //Run 10 seconds of game time, accelerating the whole time
    for(uint32_t i = 0; i < 60 * 10; ++i) {
        sdWorldStep(world, frame_time);
    }
 
    CHECK_CLOSE(6.0f * world_scale, sdObjectGetSpeedX(character), EPSILON);
    
    //Set the speed faster than max
    sdObjectSetSpeedX(character, 7.0f * world_scale);
    sdWorldStep(world, frame_time);
    //No acceleration should have been added
    CHECK_CLOSE(7.0f * world_scale, sdObjectGetSpeedX(character), EPSILON);
    
    sdWorldDestroy(world);
}
