#ifndef KAZPHYSICS2_H_INCLUDED
#define KAZPHYSICS2_H_INCLUDED

typedef unsigned int KPuint;
typedef float KPfloat;

typedef struct KPvec2 {
    float x;
    float y;
} KPvec2;

typedef enum KPenum {
    KP_WORLD_GRAVITY,
    KP_ENTITY_GRAVITY,
    KP_ENTITY_POSITION,
    KP_ENTITY_COLLISION_TYPE,
    KP_ENTITY_SIZE,
    KP_ENTITY_FRICTION,
    KP_ENTITY_MAX_SPEED,
    KP_ENTITY_MIN_SPEED,
    KP_ENTITY_VELOCITY,
    KP_COLLISION_RAY_TYPE,
    KP_COLLISION_SPHERE_TYPE
} KPenum;

KPuint kpCreateWorld();
void kpWorldAddTriangle(KPuint world, KPvec2* points);
void kpWorldAddMesh(KPuint world, KPuint num_triangles, KPuint points);
void kpWorldStep(KPuint world, KPfloat dt);
void kpWorldDebugRenderGL(KPuint world);
void kpWorldParameterfv(KPuint world, KPenum pname, KPfloat* param);
void kpDestroyWorld(KPuint world);

KPuint kpCreateEntity(KPuint world);
void kpEntityParameterfv(KPuint entity, KPenum pname, KPfloat* param);
void kpEntityParameteru(KPuint entity, KPenum pname, KPenum param);

void kpEntityGetFloatfv(KPenum pname, KPfloat* pOut);

void kpDestroyEntity(KPuint entity);

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
