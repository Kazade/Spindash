
#include "spring.h"
#include "spindash.h"

void Spring::respond_to(const std::vector<Collision>& collisions) {
    
    if(collisions.empty()) return;
    
    Object* other = (collisions[0].object_a == &geom()) ? collisions[0].object_b : collisions[0].object_a;    
    sdObjectSetSpeedY(other->id(), 16.0f * (1.0f/40.0f));
}
