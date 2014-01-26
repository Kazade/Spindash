
#include "spring.h"
#include "spindash.h"
#include "world.h"

SDuint sdSpringCreate(SDuint world_id, SDdouble angle, SDdouble power) {
    World* world = get_world_by_id(world_id);
    return world->new_spring(power, angle);
}


bool Spring::respond_to(const std::vector<Collision>& collisions) {
    
    if(collisions.empty()) return false;
    
    //FIXME: search for collisions where the angle of the normal matches angle
    Object* other = (collisions[0].object_a == &geom()) ? collisions[0].object_b->owner() : collisions[0].object_a->owner();    
    
    sdObjectSetSpeedX(other->id(), sinf(kmDegreesToRadians(angle_)) * power_);
    sdObjectSetSpeedY(other->id(), cosf(kmDegreesToRadians(angle_)) * power_);
    
    if(sdObjectIsCharacter(other->id())) {
        sdCharacterStopRolling(other->id());
        sdCharacterStopJumping(other->id());
    }
    
    return false;
}

