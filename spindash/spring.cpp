
#include "spring.h"
#include "spindash.h"
#include "world.h"

bool Spring::respond_to(const std::vector<Collision>& collisions) {
    
    if(collisions.empty()) return false;
    
    //FIXME: search for collisions where the angle of the normal matches angle
    Object* other = (collisions[0].object_a == &geom()) ? collisions[0].object_b->owner() : collisions[0].object_a->owner();    
    
    sdObjectSetSpeedX(other->id(), sinf(kmDegreesToRadians(angle_)) * power_);
    sdObjectSetSpeedY(other->id(), cosf(kmDegreesToRadians(angle_)) * power_);
    
    
    return false;
}


