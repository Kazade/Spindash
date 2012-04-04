#ifndef CHARACTER_H
#define CHARACTER_H

#include <map>

#include "object.h"
#include "kazmath/ray2.h"

class Character : public Object {
public:
    Character(World* world):
        Object(world),
        moving_left_(false),
        moving_right_(false),
        is_grounded_(false) {
    }
    
    const kmRay2& ray(const char id) { return rays_[id]; }
    
    void start_moving_left() { moving_left_ = true; }
    void stop_moving_left() { moving_left_ = false; }
    void start_moving_right() { moving_right_ = true; }
    void stop_moving_right() { moving_right_ = false; }
    bool is_grounded() { return is_grounded_; }
    
private:
    std::map<char, kmRay2> rays_;
    
    bool moving_left_;
    bool moving_right_;
    bool is_grounded_;
    
    //override
    void pre_update(float dt);
    //override
    void post_speed_update(float dt);
    
    bool acc_applied_this_frame_;
    
};

#endif
