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
    KP_ENTITY_MAX_ABS_X_SPEED,
    KP_ENTITY_MAX_ABS_Y_SPEED,
    KP_ENTITY_VELOCITY,
    KP_ENTITY_ROTATION,
    KP_ENTITY_SPEED,
    KP_CHARACTER_COLLISION_RAY_A,
    KP_CHARACTER_COLLISION_RAY_B,
    KP_CHARACTER_COLLISION_RAY_L,
    KP_CHARACTER_COLLISION_RAY_R,
    KP_CHARACTER_GROUND_SPEED,
    KP_CHARACTER_IS_JUMPING
} KPenum;

KPuint kpCreateWorld();
void kpWorldAddTriangle(KPuint world, KPvec2* points);
void kpWorldAddMesh(KPuint world, KPuint num_triangles, KPuint points);
void kpWorldStep(KPuint world, KPfloat dt);
void kpWorldDebugRenderGL(KPuint world);
void kpWorldParameterfv(KPuint world, KPenum pname, KPfloat* param);
void kpDestroyWorld(KPuint world);

KPuint kpCreateCharacter(KPuint world);
void kpBindCharacter(KPuint entity_id);
void kpCharacterCreateMode(const char* new_mode_name);
void kpCharacterSetMode(const char* new_mode_name);
void kpCharacterParameterfv(KPenum pname, KPfloat* param);
void kpCharacterParameteru(KPenum pname, KPenum param);
void kpCharacterGetFloatfv(KPenum pname, KPfloat* pOut);
void kpDestroyCharacter(KPuint entity);

void kpCharacterStartMovingLeft();
void kpCharacterStopMovingLeft();
void kpCharacterStartMovingRight();
void kpCharacterStopMovingRight();
void kpCharacterStartJumping();
void kpCharacterStopJumping();

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
