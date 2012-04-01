#ifndef KAZPHYSICS2_H_INCLUDED
#define KAZPHYSICS2_H_INCLUDED

#include <cstdint>

#include "kazmath/vec2.h"

typedef unsigned int KPuint;
typedef float KPfloat;

KPuint kpCreateWorld();
void kpWorldAddTriangle(KPuint world, kmVec2* points);
void kpWorldAddMesh(KPuint world, KPuint num_triangles, kmVec2* points);
void kpWorldStep(KPuint world, KPfloat dt);
void kpWorldDebugRenderGL(KPuint world);
void kpDestroyWorld(KPuint world);

void kpObjectSetPosition(KPuint object, float x, float y);
KPfloat kpObjectGetSpeedX(KPuint object);
KPfloat kpObjectGetSpeedY(KPuint object);
void kpObjectSetSpeedX(KPuint object, KPfloat x);
void kpObjectSetSpeedY(KPuint object, KPfloat y);

KPuint kpCreateCharacter(KPuint world);
void kpDestroyCharacter(KPuint entity);

void kpCharacterStartMovingLeft(KPuint character);
void kpCharacterStopMovingLeft(KPuint character);
void kpCharacterStartMovingRight(KPuint character);
void kpCharacterStopMovingRight(KPuint character);
void kpCharacterStartJumping(KPuint character);
void kpCharacterStopJumping(KPuint character);

/*
    USAGE:
        KPfloat gravity[2] = { 0.0f, 0.0f};
        KPfloat pos[2] = { 0.0f, 1.0f };
        KPfloat custom_gravity[2] = { 1.0f, 0.0f };
        KPfloat entity_size = { 0.5f, 1.0f };

        KPuint world = kpCreateWorld();
        kpWorldParameterfv(world, KP_WORLD_GRAVITY, gravity);

        KPuint entity = kpCreateEntity(world);
        kpEntityParameterfv(entity, KP_ENTITY_POSITION, pos);
        kpEntityParameterfv(entity, KP_ENTITY_GRAVITY, custom_gravity);
        kpEntityParameterfv(entity, KP_ENTITY_SIZE, size);
        kpEntityParameteru(entity, KP_ENTITY_COLLISION_TYPE, KP_COLLISION_RAY_TYPE);
        while(running) {
            kpWorldStep(world, dt);
            kpWorldDebugRenderGL();
        }
*/

#endif // KAZPHYSICS2_H_INCLUDED
