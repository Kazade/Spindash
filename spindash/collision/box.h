#ifndef SD_BOX_H
#define SD_BOX_H

#include "collision_primitive.h"
#include "../typedefs.h"

class Box : public CollisionPrimitive {
public:
    Box(Object* owner, float width, float height);
    
    Box():
        CollisionPrimitive(nullptr) {
        
    }
    
    void set_position(float x, float y);
    void set_rotation(float angle);
    
    kmVec2& point(const int i) { return points_[i]; }
    kmVec2* points() { return points_; }

    void set_geometry_handle(SDGeometryHandle handle) { handle_ = handle; }
    SDGeometryHandle geometry_handle() const { return handle_; }
private:
    float x_;
    float y_;
    float width_;
    float height_;
    float degrees_;
    
    kmVec2 points_[4];
    
    void init();    

    SDGeometryHandle handle_ = 0;
};

#endif 
