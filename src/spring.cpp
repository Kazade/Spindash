
#include "spring.h"
#include "spindash.h"
#include "world.h"

SDuint sdSpringCreate(SDuint world_id, SDfloat angle, SDfloat power) {
    World* world = get_world_by_id(world_id);
    return world->new_spring(power, angle);
}


void Spring::respond_to(const std::vector<Collision>& collisions) {
    
    if(collisions.empty()) return;
    
    //FIXME: search for collisions where the angle of the normal matches angle
    Object* other = (collisions[0].object_a == &geom()) ? collisions[0].object_b->owner() : collisions[0].object_a->owner();    
    
    //FIXME: should set x, and y using sine/cos
    sdObjectSetSpeedX(other->id(), sinf(kmDegreesToRadians(angle_)) * power_);
    sdObjectSetSpeedY(other->id(), cosf(kmDegreesToRadians(angle_)) * power_);
}


