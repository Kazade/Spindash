#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "collision_primitive.h"
#include "../typedefs.h"

class Triangle : public CollisionPrimitive {
public:
    Triangle():
        CollisionPrimitive(nullptr) {}

    const kmVec2& point(const uint32_t i) { return points_[i]; }

    kmVec2* points() { return points_; }

    void set_position(float x, float y) {} //Triangles are absolute
    void set_rotation(float degrees) {}

    void set_geometry_handle(SDGeometryHandle handle) { handle_ = handle; }
    SDGeometryHandle geometry_handle() const { return handle_; }

private:
    SDGeometryHandle handle_ = 0;
    kmVec2 points_[3];
};

#endif
