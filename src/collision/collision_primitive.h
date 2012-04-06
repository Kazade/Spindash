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
};

class CollisionPrimitive {
public:
    typedef std::tr1::shared_ptr<CollisionPrimitive> ptr;

    virtual ~CollisionPrimitive();

};

#endif
