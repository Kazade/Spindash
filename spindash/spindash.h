#ifndef KAZPHYSICS2_H_INCLUDED
#define KAZPHYSICS2_H_INCLUDED

#include <cstdint>

#include "kazmath/vec2.h"
#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

enum sdSkill {
	SD_SKILL_NONE = 0,
	SD_SKILL_ROLL = 1,
	SD_SKILL_SPINDASH = 2	
};

SDuint sdWorldCreate();
void sdWorldAddTriangle(SDuint world, kmVec2* points);
void sdWorldAddBox(SDuint world, kmVec2* points);
void sdWorldAddMesh(SDuint world, SDuint num_triangles, kmVec2* points);
void sdWorldConstructLoop(SDuint world, SDfloat left, SDfloat top,
    SDfloat width);
void sdWorldRemoveTriangles(SDuint world);
void sdWorldStep(SDuint world, SDfloat dt);
void sdWorldDestroy(SDuint world);
SDuint64 sdWorldGetStepCounter(SDuint world);
void sdWorldSetCompileGeometryCallback(SDuint world_id, SDCompileGeometryCallback callback, void* userData);
void sdWorldSetRenderGeometryCallback(SDuint world_id, SDRenderGeometryCallback callback, void* userData);
void sdWorldRender(SDuint world_id);
void sdWorldDebugEnable(SDuint world);
void sdWorldDebugStep(SDuint world, SDfloat step);
SDbool sdWorldDebugIsEnabled(SDuint world);
void sdWorldDebugDisable(SDuint world);

void sdWorldCameraTarget(SDuint world, SDuint object);
void sdWorldCameraGetPosition(SDuint world, SDfloat* x, SDfloat* y);

void sdObjectDestroy(SDuint object);
void sdObjectSetPosition(SDuint object, SDfloat x, SDfloat y);
void sdObjectGetPosition(SDuint object, SDfloat *x, SDfloat *y);
SDfloat sdObjectGetPositionX(SDuint object);
SDfloat sdObjectGetPositionY(SDuint object);
SDfloat sdObjectGetSpeedX(SDuint object);
SDfloat sdObjectGetSpeedY(SDuint object);
void sdObjectSetSpeedX(SDuint object, SDfloat x);
void sdObjectSetSpeedY(SDuint object, SDfloat y);
SDfloat sdObjectGetRotation(SDuint object);
void sdObjectSetFixed(SDuint object, SDbool value); //Make object unmoveable

void sdObjectSetBounciness(SDuint object, SDfloat v);
void sdObjectSetFriction(SDuint object, SDfloat friction);

SDuint sdCharacterCreate(SDuint world);
void sdCharacterOverrideSetting(const char* setting, float value);

SDbool sdCharacterIsGrounded(SDuint character);
SDbool sdCharacterIsJumping(SDuint character);
SDbool sdCharacterIsRolling(SDuint character);

void sdCharacterLeftPressed(SDuint character);
void sdCharacterRightPressed(SDuint character);
void sdCharacterUpPressed(SDuint character);
void sdCharacterDownPressed(SDuint character);
void sdCharacterJumpPressed(SDuint character);
SDDirection sdCharacterFacingDirection(SDuint character);

SDAnimationState sdCharacterAnimationState(SDuint character);

SDfloat sdCharacterGetWidth(SDuint character);
void sdCharacterSetGroundSpeed(SDuint character, SDfloat value);
SDfloat sdCharacterGetGroundSpeed(SDuint character);
SDfloat sdCharacterGetSpindashCharge(SDuint character);

void sdCharacterEnableSkill(SDuint character, sdSkill skill);
void sdCharacterDisableSkill(SDuint character, sdSkill skill);
SDbool sdCharacterSkillEnabled(SDuint character, sdSkill skill);

SDbool sdObjectIsCharacter(SDuint object);


SDuint sdSpringCreate(SDuint world, SDfloat angle, SDfloat power);

SDuint sdBoxCreate(SDuint world, SDfloat width, SDfloat height);
SDuint sdCircleCreate(SDuint world, SDfloat diameter);

enum CollisionResponse {
    COLLISION_RESPONSE_NONE,  //Collision is ignored
    COLLISION_RESPONSE_DEFAULT, //Standard collision detection is applied
    COLLISION_RESPONSE_SPRING_LOW, //Object is sprung with low power based on the other objects rotation
    COLLISION_RESPONSE_SPRING_HIGH, //Same, with higher power
    COLLISION_RESPONSE_SPRINGBOARD_LOW,
    COLLISION_RESPONSE_SPRINGBOARD_HIGH,
    COLLISION_RESPONSE_BALLOON,
    COLLISION_RESPONSE_BUMPER,
    COLLISION_RESPONSE_SPRING_CAP,
    COLLISION_RESPONSE_BOUNCE_ONE,
    COLLISION_RESPONSE_BOUNCE_TWO,
    COLLISION_RESPONSE_BOUNCE_THREE,
    COLLISION_RESPONSE_BREAKABLE_OBJECT,
    COLLISION_RESPONSE_REBOUND,
    COLLISION_RESPONSE_HAZARD,
    COLLISION_RESPONSE_DEATH, //Collisions disabled on object, it's thrown into the air then falls
    COLLISION_RESPONSE_POWER_UP,
    COLLISION_RESPONSE_UNFIX //bumps the object a little bit upwards, and calls sdObjectSetFixed(false)
};

typedef void (*ObjectCollisionCallback)(SDuint, SDuint, CollisionResponse*, CollisionResponse*, void* data);
void sdWorldSetObjectCollisionCallback(SDuint world, ObjectCollisionCallback callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif // KAZPHYSICS2_H_INCLUDED
