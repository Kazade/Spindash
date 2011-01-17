#include "kazphysics2.h"

int main(int argc, char** argv) {
    float gravity[2] = { 0.0f, 0.0f };

    KPuint world = kpCreateWorld();
    kpWorldParameterfv(world, KP_WORLD_GRAVITY, gravity);

    KPvec2 points[3] = { {-10.0f, 0.0f},
                          {10.0f, 0.0f},
                          {0.0f, -5.0f} };

    KPvec2 size[2] = { 0.5f, 1.0f };
    KPvec2 ent_gravity[2] { 0.0f, -1.0f };

    kpWorldAddTriangle(world, points);

    KPuint sonic = kpCreateEntity(world);
    kpEntityParameteru(entity, KP_ENTITY_COLLISION_TYPE, KP_COLLISION_RAY_TYPE);
    kpEntityParameterfv(entity, KP_ENTITY_SIZE, size);
    kpEntityParameterfv(entity, KP_ENTITY_GRAVITY, ent_gravity);

    kpDestroyWorld(world);
    return 0;
}
