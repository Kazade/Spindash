#ifndef SD_SPRING_H
#define SD_SPRING_H

#include "object.h"
#include "collision/box.h"

class Spring : public Object {
public:
    Spring(World* world, float power, float angle):
        Object(world, CollisionPrimitive::ptr(new Box(this, 0.75, 0.6))),
        power_(power),
        angle_(angle) {
        
        geom().set_rotation(angle_);
        set_collision_flag(NOT_GROUND);
    }
    
    bool respond_to(const std::vector<Collision>& collisions);
    
private:
    float power_;
    float angle_;    
};

#endif
