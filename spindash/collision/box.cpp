#include <cstdint>

#include "box.h"

#include "kazmath/mat3.h"
#include "kazmath/vec2.h"

Box::Box(Object* owner, float width, float height):
    CollisionPrimitive(owner),
    x_(0.0f),
    y_(0.0f),
    width_(width),
    height_(height),
    degrees_(0.0f) {

    init();
}

void Box::set_position(float x, float y) {
    x_ = x;
    y_ = y;
    init();
}

void Box::set_rotation(float angle) {
    degrees_ = angle;
    init();
}

void Box::init() {
    float hw = width_ * 0.5f;
    float hh = height_ * 0.5f;

    points_[0].x = -hw;
    points_[0].y = -hh;
    
    points_[1].x = hw;
    points_[1].y = -hh;

    points_[2].x = hw;
    points_[2].y = hh;
    
    points_[3].x = -hw;
    points_[3].y = hh;
    
    kmMat3 rotation;
    kmMat3RotationZ(&rotation, kmDegreesToRadians(degrees_));
    
    kmVec2 translation;
    kmVec2Fill(&translation, x_, y_);
    
    //Rotate the box to match the angle and add the position
    for(uint32_t i = 0; i < 4; ++i) {
        kmVec2Transform(&points_[i], &points_[i], &rotation);
        kmVec2Add(&points_[i], &points_[i], &translation);
    }        
}
