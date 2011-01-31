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
        BLOCKED_RIGHT,
        MOVING_LEFT,
        MOVING_RIGHT,
        JUMPING
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

    kmRay2 get_ray_a() const { return rays_[RL_A]; }
    kmRay2 get_ray_b() const { return rays_[RL_B]; }
    kmRay2 get_ray_l() const { return rays_[RL_L]; }
    kmRay2 get_ray_r() const { return rays_[RL_R]; }

    float get_angle() const { return angle_; }
    float get_ground_speed() const { return gsp_; }
    float get_x_speed() const { return speed_.x; }
    bool get_is_jumping() const { return is_jumping_; }
private:
    enum RAY_LOOKUP {
        RL_A = 0,
        RL_B,
        RL_L,
        RL_R,
        RL_MAX
    };

    kmRay2 rays_[RL_MAX];
    std::map<KPuint, char> ray_ids_;

    void prevent_pass_through();
    void clear_collisions();
    void process_collisions();
    void collide_with_world();
    void apply_gravity(double step);
    void calculate_angle(CollisionInfo* a, CollisionInfo* b);

    void initialize_ray(kmRay2* out, float rel_start_x, float rel_start_y,
                            float ray_dir_x, float ray_dir_y);

    void respond_to_floor_collisions(std::vector<CollisionInfo>& a_collisions,
                                     std::vector<CollisionInfo>& b_collisions);

    void respond_to_wall_collisions(std::vector<CollisionInfo>& collisions, Flag blocked_flag);

    World* world_;

    float frc_;
    float angle_;
    float gsp_;

    bool jumping_last_frame_;
    bool can_jump_;
    bool jump_released_;
    bool is_jumping_;

    kmVec2 position_;
    kmVec2 speed_;
    kmVec2 size_;

    std::map<KPuint, bool> flags_;

    std::vector<CollisionInfo> collisions_;
};

#endif // ENTITY_H_INCLUDED
