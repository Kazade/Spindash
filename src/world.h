#ifndef KP_WORLD_H
#define KP_WORLD_H

#include <vector>
#include "kazmath/kazmath.h"
#include "spindash.h"
#include "object.h"

#include "collision/triangle.h"
#include "collision/box.h"

class World {
public:
    static SDuint world_id_counter_;

    World(SDuint id);
    
    void set_gravity(float x, float y);
    void get_gravity(float& x, float& y);

    void add_triangle(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3);
    void add_box(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3, const kmVec2& v4);
    void remove_all_triangles() { triangles_.clear(); }
    
    ObjectID new_sphere();
    ObjectID new_box();
    ObjectID new_character();
    ObjectID new_spring(float angle, float power);
    
    void destroy_object(ObjectID object_id);
    
    void debug_render();

    void update(float step);

    SDuint get_triangle_count() const { return triangles_.size(); }
    Triangle* get_triangle_at(SDuint i) { return &triangles_[i]; }
    
    SDuint get_box_count() const { return boxes_.size(); }
    Box* get_box_at(SDuint i) { return &boxes_.at(i); }
    
    uint64_t step_counter() const { return step_counter_; }
private:
    SDuint id_;
    kmVec2 gravity_;

    std::vector<Triangle> triangles_;
    std::vector<Box> boxes_;
    std::vector<Object::ptr> objects_;
    
    uint64_t step_counter_;
};

extern World* get_world_by_id(SDuint world);

#endif
