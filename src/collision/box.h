#ifndef SD_BOX_H
#define SD_BOX_H

#include "collision_primitive.h"

class Box : public CollisionPrimitive {
public:
    Box(Object* owner, float width, float height);
    
    Box():
        CollisionPrimitive(nullptr) {
        
    }
    
    void set_position(float x, float y);
    void set_rotation(float angle);
    
    kmVec2& point(const int i) { return points[i]; }
private:
    float x_;
    float y_;
    float width_;
    float height_;
    float degrees_;
    
    kmVec2 points[4];    
    
    void init();    
};

#endif 
