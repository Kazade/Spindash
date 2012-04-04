#ifndef KAZPHYSICS2_H_INCLUDED
#define KAZPHYSICS2_H_INCLUDED

#include <cstdint>

#include "kazmath/vec2.h"
#include "typedefs.h"

SDuint sdCreateWorld();
void sdWorldAddTriangle(SDuint world, kmVec2* points);
void sdWorldAddMesh(SDuint world, SDuint num_triangles, kmVec2* points);
void sdWorldStep(SDuint world, SDfloat dt);
void sdWorldDebugRenderGL(SDuint world);
void sdDestroyWorld(SDuint world);

void sdObjectDestroy(SDuint object);

void sdObjectSetPosition(SDuint object, float x, float y);
kmVec2 sdObjectGetPosition(SDuint object);
SDfloat sdObjectGetPositionX(SDuint object);
SDfloat sdObjectGetPositionY(SDuint object);
SDfloat sdObjectGetSpeedX(SDuint object);
SDfloat sdObjectGetSpeedY(SDuint object);
void sdObjectSetSpeedX(SDuint object, SDfloat x);
void sdObjectSetSpeedY(SDuint object, SDfloat y);
SDfloat sdObjectGetRotation(SDuint object);

void sdObjectSetBounciness(SDuint object, SDfloat v);
void sdObjectSetFriction(SDuint object, SDfloat friction);

SDuint sdCharacterCreate(SDuint world);

SDbool sdCharacterIsGrounded(SDuint character);
void sdCharacterStartMovingLeft(SDuint character);
void sdCharacterStopMovingLeft(SDuint character);
void sdCharacterStartMovingRight(SDuint character);
void sdCharacterStopMovingRight(SDuint character);
void sdCharacterStartJumping(SDuint character);
void sdCharacterStopJumping(SDuint character);

SDuint sdBoxCreate(SDuint world);
SDuint sdCircleCreate(SDuint world);



#endif // KAZPHYSICS2_H_INCLUDED
