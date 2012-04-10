#ifndef CHARACTER_H
#define CHARACTER_H

#include <map>

#include "object.h"
#include "kazmath/ray2.h"
#include "collision/ray_box.h"

class Character : public Object {
public:
    Character(World* world):
        Object(world, CollisionPrimitive::ptr(new RayBox(0.5f, 1.0f))),
        moving_left_(false),
        moving_right_(false),
        looking_down_(false),
        jump_pressed_(false),
        waiting_for_jump_release_(false),
        rolling_(false),
        is_grounded_(false),
        gsp_(0.0f) {

    }
    
    void start_moving_left() { moving_left_ = true; }
    void stop_moving_left() { moving_left_ = false; }
    void start_moving_right() { moving_right_ = true; }
    void stop_moving_right() { moving_right_ = false; }
    
    void start_looking_down() { looking_down_ = true; }
    void stop_looking_down() { looking_down_ = false; }
    void start_rolling() { rolling_ = true; }
    void stop_rolling() { rolling_ = false; }
    void start_jumping() { jump_pressed_ = true; }
    void stop_jumping() { jump_pressed_ = false; }
    
    bool is_grounded() { return is_grounded_; }
    
    void respond_to(const std::vector<Collision>& collisions);
    
    void set_speed(float x, float y);
    
private:
    bool moving_left_;
    bool moving_right_;
    bool looking_down_;
    bool jump_pressed_;
    bool waiting_for_jump_release_;
    
    bool rolling_;
    bool is_grounded_;
    
    float gsp_;
    
    //override
    void pre_prepare(float dt);
    //override
    void post_prepare(float dt);
    bool pre_update(float dt);
    void prepare(float dt);
    
    bool acc_applied_this_frame_;     
};

#endif
