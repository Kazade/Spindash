#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "collision_primitive.h"

class Triangle : public CollisionPrimitive {
public:
    Triangle():
        CollisionPrimitive(nullptr) {}

    kmVec2 points[3];    
    
    void set_position(float x, float y) {} //Triangles are absolute
    void set_rotation(float degrees) {}
};

#endif
