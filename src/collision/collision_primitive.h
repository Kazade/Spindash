#ifndef COLLISION_PRIMITIVE_H
#define COLLISION_PRIMITIVE_H

#include <vector>
#include <tr1/memory>
#include "kazmath/vec2.h"

class Object;
class CollisionPrimitive;

struct Collision {
    kmVec2 point; //Where in space did the collision occur
    
    kmVec2 a_normal; //The normal of the surface on object_a that collided
    kmVec2 b_normal; //The normal of the surface on object_b that collided
    
    CollisionPrimitive* object_a;
    CollisionPrimitive* object_b;
    
    char a_ray; //The ID of the ray that caused this collision, only used for ray-boxes
    char b_ray;
};

class CollisionPrimitive {
public:
    typedef std::tr1::shared_ptr<CollisionPrimitive> ptr;
    
    CollisionPrimitive(Object* owner):
        owner_(owner) {}

    virtual ~CollisionPrimitive();

    virtual void set_position(float x, float y) = 0;
    virtual void set_rotation(float degrees) = 0;
    
    Object* owner() { return owner_; }
    
private:
    Object* owner_;    
};

#endif
