#ifndef KP_WORLD_H
#define KP_WORLD_H

#include <vector>
#include "kazmath/kazmath.h"
#include "kazphysics2.h"

struct Triangle {
    kmVec2 points[3];
};

class World {
public:
    static KPuint world_id_counter_;

    World(KPuint id);
    void set_gravity(float x, float y);
    void get_gravity(float& x, float& y);

    void add_triangle(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3);
    void add_entity(KPuint entity_id) { entities_.push_back(entity_id); }

    void debug_render();

    void update(float step);

    KPuint get_triangle_count() const { return triangles_.size(); }
    Triangle* get_triangle_at(KPuint i) { return &triangles_[i]; }
private:
    KPuint id_;
    kmVec2 gravity_;

    std::vector<Triangle> triangles_;
    std::vector<KPuint> entities_;
};

extern World* get_world_by_id(KPuint world);

#endif
