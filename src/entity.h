#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED

#include <vector>
#include <kazmath/kazmath.h>

#include "kazphysics2.h"

class World;

struct CollisionInfo {
    kmVec2 intersection;
    kmVec2 surface_normal;
    double distance;
    char identifier;
    kmRay2 ray;
};

class Entity {
public:
    enum Flag {
        ON_GROUND,
        BLOCKED_LEFT,
        BLOCKED_RIGHT
    };

    static KPuint entity_id_counter_;

    Entity();

    void set_world(World* world) { world_ = world; }

    void update(double step);

    void set_flag(Flag f, bool value) { flags_[f] = value; }

    bool get_flag(Flag f) const {
        std::map<KPuint, bool>::const_iterator it = flags_.find(f);
        if(it == flags_.end()) {
            return false;
        }

        return (*it).second;
    }

    kmVec2 get_position() const { return position_; }
    void set_position(float x, float y) {
        position_.x = x;
        position_.y = y;
    }

    kmRay2 get_ray_a() const { return ra_; }
    kmRay2 get_ray_b() const { return rb_; }

private:
    kmRay2 ra_, rb_;

    void clear_collisions();
    void process_collisions();
    void collide_with_world();
    void apply_gravity(double step);
    void calculate_angle();

    World* world_;

    float frc_;
    float angle_;
    float gsp_;

    kmVec2 position_;
    kmVec2 speed_;
    kmVec2 size_;

    std::map<KPuint, bool> flags_;

    std::vector<CollisionInfo> collisions_;
};

#endif // ENTITY_H_INCLUDED
