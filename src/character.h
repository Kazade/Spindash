#ifndef CHARACTER_H
#define CHARACTER_H

#include <map>

#include "spindash.h"
#include "object.h"
#include "kazmath/ray2.h"
#include "collision/ray_box.h"

enum Direction {
	DIRECTION_LEFT,
	DIRECTION_RIGHT
};

class Character : public Object {
public:
    Character(World* world, SDdouble width, SDdouble height):
        Object(world, CollisionPrimitive::ptr(new RayBox(this, width, height*1.1))),
        height_(height),
        width_(width),
        moving_left_(false),
        moving_right_(false),
        looking_down_(false),
        jump_pressed_(false),
        waiting_for_jump_release_(false),
        rolling_(false),
        is_grounded_(false),
        facing_(DIRECTION_RIGHT),
        spindash_charge_(0),
        gsp_(0.0),
        enabled_skills_(0) {

		enable_skill(SD_SKILL_ROLL);
		enable_skill(SD_SKILL_SPINDASH);
    }
    
    void start_moving_left() { 
		moving_left_ = true; 
	}
    void stop_moving_left() { moving_left_ = false; }
    void start_moving_right() { 
		moving_right_ = true; 
	}
    void stop_moving_right() { moving_right_ = false; }
    
    void start_looking_down() { looking_down_ = true; }
    void stop_looking_down() { looking_down_ = false; }
    void start_rolling() { 
		if(!skill_enabled(SD_SKILL_ROLL)) return;
		rolling_ = true; 
	}
    void stop_rolling() { rolling_ = false; }
    void start_jumping() { 
		
		
		if(skill_enabled(SD_SKILL_SPINDASH) && looking_down_) {
			spindash_charge_ += 2;
			if(spindash_charge_ > 8) spindash_charge_ = 8;
		} else {
			jump_pressed_ = true; 	
		}
	}
    void stop_jumping() { jump_pressed_ = false; }
    
    bool is_grounded() { return is_grounded_; }
    
    bool respond_to(const std::vector<Collision>& collisions);
    
    void set_speed(float x, float y);
    
    SDdouble width() const { return width_; }
    
    void set_ground_speed(double speed) { gsp_ = speed; }
    double ground_speed() const { return gsp_; }
    
    void enable_skill(sdSkill s) { enabled_skills_ |= s; }
    void disable_skill(sdSkill s) { enabled_skills_ &= ~s; }
    bool skill_enabled(sdSkill s) const {
		return enabled_skills_ & s;
	}
    
private:
    Collision find_nearest_collision(const std::vector<Collision>& collisions);
    std::pair<Collision, bool> find_collision_with_ray(const std::vector<Collision>& collisions, char ray);
    
    SDdouble height_;
    SDdouble width_;
    
    bool moving_left_;
    bool moving_right_;
    bool looking_down_;
    bool jump_pressed_;
    bool waiting_for_jump_release_;
    
    bool rolling_;
    bool is_grounded_;
    
    Direction facing_;
    
    double spindash_charge_;
    
    double gsp_;
    
    //override
    void pre_prepare(float dt);
    //override
    void post_prepare(float dt);
    void update_finished(float dt);
    void prepare(float dt);
    
    bool acc_applied_this_frame_;     
    
    uint32_t enabled_skills_;
};

#endif
