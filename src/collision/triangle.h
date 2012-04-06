#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "collision_primitive.h"

class Triangle : public CollisionPrimitive {
public:
    kmVec2 points[3];    
};

#endif
