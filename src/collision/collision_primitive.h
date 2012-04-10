#ifndef COLLISION_PRIMITIVE_H
#define COLLISION_PRIMITIVE_H

#include <vector>
#include <tr1/memory>
#include "kazmath/vec2.h"

class CollisionPrimitive;

struct Collision {
    kmVec2 point; //Where in space did the collision occur
    kmVec2 normal; //What was the normal of the surface/object we collided with            
    CollisionPrimitive* other;
    char ray; //The ID of the ray that caused this collision, only used for ray-boxes
};

class CollisionPrimitive {
public:
    typedef std::tr1::shared_ptr<CollisionPrimitive> ptr;

    virtual ~CollisionPrimitive();

    virtual void set_position(float x, float y) = 0;
    virtual void set_rotation(float degrees) = 0;
};

#endif
