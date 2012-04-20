#ifndef KAZPHYSICS2_H_INCLUDED
#define KAZPHYSICS2_H_INCLUDED

#include <cstdint>

#include "kazmath/vec2.h"
#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

SDuint sdWorldCreate();
void sdWorldAddTriangle(SDuint world, kmVec2* points);
void sdWorldAddBox(SDuint world, kmVec2* points);
void sdWorldAddMesh(SDuint world, SDuint num_triangles, kmVec2* points);
void sdWorldRemoveTriangles(SDuint world);
void sdWorldStep(SDuint world, SDdouble dt);
void sdWorldDebugRenderGL(SDuint world);
void sdWorldDestroy(SDuint world);
SDuint64 sdWorldGetStepCounter(SDuint world);

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
void sdCharacterStartMovingLeft(SDuint character);
void sdCharacterStopMovingLeft(SDuint character);
void sdCharacterStartMovingRight(SDuint character);
void sdCharacterStopMovingRight(SDuint character);
void sdCharacterStartJumping(SDuint character);
void sdCharacterStopJumping(SDuint character);
void sdCharacterStartLookingDown(SDuint character);
void sdCharacterStopLookingDown(SDuint character);
void sdCharacterStopRolling(SDuint character);
SDdouble sdCharacterGetWidth(SDuint character);
void sdCharacterSetGroundSpeed(SDuint character, SDdouble value);
SDdouble sdCharacterGetGroundSpeed(SDuint character);

SDbool sdObjectIsCharacter(SDuint object);


SDuint sdSpringCreate(SDuint world, SDdouble angle, SDdouble power);

SDuint sdBoxCreate(SDuint world);
SDuint sdCircleCreate(SDuint world);

#ifdef __cplusplus
}
#endif

#endif // KAZPHYSICS2_H_INCLUDED
