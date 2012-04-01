#include <unittest++/UnitTest++.h>

#include "kazphysics2.h"

void CHECK_ALMOST_EQUAL(float lhs, float rhs) {
    const float EPSILON = 1.0f / 128.0f;

    CHECK(rhs > lhs - EPSILON && rhs < lhs + EPSILON);
}

//Sonic runs at 60 fps on NTSC 
float frame_time = 1.0f / 60.0f;

//Sonic is 40 pixels tall, we use this to scale the world so 
//that he is one unit high
float world_scale = 1.0f / 40.0f;

void create_floor_plane(KPuint world) {
    kmVec2 points[6];
    
    kmVec2Fill(&points[0], -1000.0f, 0.0f);
    kmVec2Fill(&points[1], -1000.0f, -1.0f);
    kmVec2Fill(&points[2], 1000.0f, 0.0f); 

    kmVec2Fill(&points[3], 1000.0f, 0.0f);  
    kmVec2Fill(&points[4], -1000.0f, -1.0f);
    kmVec2Fill(&points[5], 1000.0f, -1.0f);
                      
    kpWorldAddMesh(world, 2, points);
}
    
TEST(test_acceleration) {
    KPuint world = kpCreateWorld();
    KPuint character = kpCreateCharacter(world);
    
    //Character by default should be 1.0f units high
    kpObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);
    
    //Check that the X and Y speed are zero
    CHECK_ALMOST_EQUAL(0.0f, kpObjectGetSpeedX(character));    
    CHECK_ALMOST_EQUAL(0.0f, kpObjectGetSpeedY(character));
 
    //Start moving the character right   
    kpCharacterStartMovingRight(character);    
    kpWorldStep(world, frame_time);
 
    //Sonic physics guide: Running   
    float expected = 0.046875f * world_scale;
    CHECK_ALMOST_EQUAL(expected, kpObjectGetSpeedX(character));
    CHECK_ALMOST_EQUAL(0.0f, kpObjectGetSpeedY(character));   
    
    kpWorldStep(world, frame_time);

    CHECK_ALMOST_EQUAL(expected * 2, kpObjectGetSpeedX(character));
    CHECK_ALMOST_EQUAL(0.0f, kpObjectGetSpeedY(character));       

    //Reset the X speed
    kpObjectSetSpeedX(character, 0.0f);    
    kpCharacterStartMovingLeft(character);
    
    kpWorldStep(world, frame_time);
    CHECK_ALMOST_EQUAL(-expected, kpObjectGetSpeedX(character));
    CHECK_ALMOST_EQUAL(0.0f, kpObjectGetSpeedY(character));           
    
    kpWorldStep(world, frame_time * 2); //Move two steps
    CHECK_ALMOST_EQUAL(-expected * 3, kpObjectGetSpeedX(character)); //Now should be negative
    CHECK_ALMOST_EQUAL(0.0f, kpObjectGetSpeedY(character));                       
}

TEST(test_deceleration) {
    KPuint world = kpCreateWorld();
    KPuint character = kpCreateCharacter(world);
    
    //Character by default should be 1.0f units high
    kpObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);    
    
    //Start moving the character right   
    kpCharacterStartMovingRight(character);    
    
    //Run 3 world steps
    for(uint32_t i = 0; i < 3; ++i) kpWorldStep(world, frame_time);    
    
    //Should have accelerated to 3 * expected
    float expected = 0.046875f * world_scale;
    CHECK_ALMOST_EQUAL(expected * 3, kpObjectGetSpeedX(character));
    
    //0.5f is the decelaration constant
    float new_expected = (expected * 3) - (0.5f * world_scale);
    
    //We are moving right, and we've pressed left - this will register
    //internally as a direction change
    kpCharacterStartMovingLeft(character);
    kpWorldStep(world, frame_time); 
    kpCharacterStopMovingLeft(character);    
        
    CHECK_ALMOST_EQUAL(new_expected, kpObjectGetSpeedX(character));
    
    kpCharacterStartMovingRight(character);
    //Set the speed so that subtracting decelration would switch sign
    kpObjectSetSpeedX(character, 0.1 * world_scale);
    kpCharacterStartMovingLeft(character); //We have turned around
    
    kpWorldStep(world, frame_time);
    //Emulate weird anomoly (see SPG Running - Deceleration)
    CHECK_ALMOST_EQUAL(-0.5 * world_scale, kpObjectGetSpeedX(character));
}

TEST(test_friction) {
    KPuint world = kpCreateWorld();
    KPuint character = kpCreateCharacter(world);
    
    //Character by default should be 1.0f units high
    kpObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);    
    
    //We've been accelerating for 1 second
    float initial = 0.046875f * world_scale * 60;
    kpObjectSetSpeedX(character, initial);
     
    for(uint32_t i = 0; i < 60; ++i) {
        kpWorldStep(world, frame_time); //No buttons are being pressed
        CHECK_ALMOST_EQUAL(initial - (i * (0.046875 * world_scale)), kpObjectGetSpeedX(character));        
    }
    
    //OK, now we are barely moving, applying friction would result
    //in a negative speed, it should be zero
    kpObjectSetSpeedX(character, 0.01 * world_scale);   
    kpWorldStep(world, frame_time);
    CHECK_ALMOST_EQUAL(0.0f, kpObjectGetSpeedX(character));
}

TEST(test_top_speed) {
    KPuint world = kpCreateWorld();
    KPuint character = kpCreateCharacter(world);
    
    //Character by default should be 1.0f units high
    kpObjectSetPosition(character, 0.0f, 0.5f);    
    create_floor_plane(world);        
    
    kpCharacterStartMovingRight(character);
    //Run 10 seconds of game time, accelerating the whole time
    for(uint32_t i = 0; i < 60 * 10; ++i) {
        kpWorldStep(world, frame_time);
    }
 
    CHECK_ALMOST_EQUAL(6.0f * world_scale, kpObjectGetSpeedX(character));
    
    //Set the speed faster than max
    kpObjectSetSpeedX(character, 7.0f * world_scale);
    kpWorldStep(world, frame_time);
    //No acceleration should have been added
    CHECK_ALMOST_EQUAL(7.0f * world_scale, kpObjectGetSpeedX(character));
}
