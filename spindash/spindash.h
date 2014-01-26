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
void sdWorldConstructLoop(SDuint world, SDdouble left, SDdouble top,
	SDdouble width);
void sdWorldRemoveTriangles(SDuint world);
void sdWorldStep(SDuint world, SDdouble dt);
void sdWorldDestroy(SDuint world);
SDuint64 sdWorldGetStepCounter(SDuint world);

void sdWorldSetCompileGeometryCallback(SDuint world_id, SDCompileGeometryCallback callback, void* userData);
void sdWorldSetRenderGeometryCallback(SDuint world_id, SDRenderGeometryCallback callback, void* userData);
void sdWorldRender(SDuint world_id);

void sdWorldDebugEnable(SDuint world);
void sdWorldDebugStep(SDuint world, SDdouble step);
SDbool sdWorldDebugIsEnabled(SDuint world);
void sdWorldDebugDisable(SDuint world);

void sdObjectDestroy(SDuint object);

void sdObjectSetPosition(SDuint object, SDdouble x, SDdouble y);
kmVec2 sdObjectGetPosition(SDuint object);
SDdouble sdObjectGetPositionX(SDuint object);
SDdouble sdObjectGetPositionY(SDuint object);
SDdouble sdObjectGetSpeedX(SDuint object);
SDdouble sdObjectGetSpeedY(SDuint object);
void sdObjectSetSpeedX(SDuint object, SDdouble x);
void sdObjectSetSpeedY(SDuint object, SDdouble y);
SDdouble sdObjectGetRotation(SDuint object);

void sdObjectSetBounciness(SDuint object, SDdouble v);
void sdObjectSetFriction(SDuint object, SDdouble friction);

SDuint sdCharacterCreate(SDuint world);

SDbool sdCharacterIsGrounded(SDuint character);
SDbool sdCharacterIsJumping(SDuint character);
SDbool sdCharacterIsRolling(SDuint character);

void sdCharacterStartMovingLeft(SDuint character);
void sdCharacterStopMovingLeft(SDuint character);
void sdCharacterStartMovingRight(SDuint character);
void sdCharacterStopMovingRight(SDuint character);
void sdCharacterStartPressingJump(SDuint character);
void sdCharacterStopPressingJump(SDuint character);
void sdCharacterStartLookingDown(SDuint character);
void sdCharacterStopLookingDown(SDuint character);
void sdCharacterStopRolling(SDuint character);
void sdCharacterStopJumping(SDuint character);

SDdouble sdCharacterGetWidth(SDuint character);
void sdCharacterSetGroundSpeed(SDuint character, SDdouble value);
SDdouble sdCharacterGetGroundSpeed(SDuint character);
SDdouble sdCharacterGetSpindashCharge(SDuint character);

void sdCharacterEnableSkill(SDuint character, sdSkill skill);
void sdCharacterDisableSkill(SDuint character, sdSkill skill);
SDbool sdCharacterSkillEnabled(SDuint character, sdSkill skill);

SDbool sdObjectIsCharacter(SDuint object);


SDuint sdSpringCreate(SDuint world, SDdouble angle, SDdouble power);

SDuint sdBoxCreate(SDuint world);
SDuint sdCircleCreate(SDuint world);

#ifdef __cplusplus
}
#endif

#endif // KAZPHYSICS2_H_INCLUDED
