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


enum CharacterSize {
    CHARACTER_SIZE_STANDING,
    CHARACTER_SIZE_CROUCHING
};

enum Quadrant {
    QUADRANT_FLOOR = 0,
    QUADRANT_RIGHT_WALL,
    QUADRANT_CEILING,
    QUADRANT_LEFT_WALL,
    QUADRANT_MAX
};

enum GroundState {
    GROUND_STATE_IN_THE_AIR,
    GROUND_STATE_ON_THE_GROUND,
    GROUND_STATE_BALANCING_LEFT,
    GROUND_STATE_BALANCING_RIGHT,
    GROUND_STATE_BALANCING_LEFT_EXTREME,
    GROUND_STATE_BALANCING_RIGHT_EXTREME
};

class Character : public Object {
public:
    static float setting(const std::string& setting);
    static void override_setting(const std::string& setting, float value);

    Character(World* world, SDdouble width, SDdouble height);
    
    //============ NEW STUFF ===============

    const RayBox& ray_box() const {
        return dynamic_cast<const RayBox&>(geom());
    }

    void set_size(CharacterSize size);
    CharacterSize size() const { return size_; }

    void set_quadrant(Quadrant quadrant);
    Quadrant quadrant() const { return quadrant_; }

    GroundState ground_state() const { return ground_state_; }

    //======================================


    void start_moving_left() { 
		moving_left_ = true; 
	}
    void stop_moving_left() { moving_left_ = false; }
    void start_moving_right() { 
		moving_right_ = true; 
	}
    void stop_moving_right() { moving_right_ = false; }
    
    void start_looking_down() { 
        looking_down_ = true; 
        enter_small_mode();
    }
    void stop_looking_down() { 
        looking_down_ = false; 
        exit_small_mode();
    }
    
    void enter_small_mode() {
        float new_width = original_width_ * 0.80; //Sonic is 16, rather than 20 pixels wide when rolling or jumping
        float new_height = original_height_  * 0.75; //Sonic is 30 rather than 40 pixels high when rolling or jumping
        
        width_ = new_width;
        height_ = new_height;
        
        (dynamic_cast<RayBox*>(&geom()))->set_size(width_, height_ * 1.5);
        
        set_position(position().x, position().y - ((original_height_ - height_) / 2.0));
    }
    
    void exit_small_mode() {
        width_ = original_width_;
        height_ = original_height_;
        (dynamic_cast<RayBox*>(&geom()))->set_size(width_, height_ * 1.5);
        
        set_position(position().x, position().y + ((original_height_ - height_) / 2.0));
    }
        
    void start_rolling() { 
		if(!skill_enabled(SD_SKILL_ROLL)) return;
        if(rolling_) return;
        
		rolling_ = true; 
        enter_small_mode();
	}
    
    void stop_rolling() { 
        if(!rolling_) return;
        
        rolling_ = false; 
        exit_small_mode();
    }
    
    void start_jumping() {        
        if(jumping_) return;
                
        stop_rolling(); //Make sure we don't maintain the rolling state while jumping
        enter_small_mode();        
        
        jumping_ = true;
    }
    void stop_jumping() {
        if(!jumping_) return;
        
        jumping_ = false;
        exit_small_mode();
    }  
            
    void start_pressing_jump() { 
		if(skill_enabled(SD_SKILL_SPINDASH) && looking_down_ && !rolling_) {
			spindash_charge_ += 2;
			if(spindash_charge_ > 8) spindash_charge_ = 8;
		} else {
            jump_pressed_ = true; 	
		}
	}
    
    void stop_pressing_jump() {         
        jump_pressed_ = false;       
    }
    
    bool is_grounded() { return is_grounded_; }
    bool jumping() const { return jumping_; }
    bool rolling() const { return rolling_; }
    
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
    
    double spindash_charge() const { return spindash_charge_; }
    
    bool horizontal_control_lock_active() const { return horizontal_control_lock_ > 0.0; }
    
private:

    // ============== NEW STUFF ================

    CharacterSize size_ = CHARACTER_SIZE_STANDING;
    CollisionPrimitive::ptr standing_shape_[QUADRANT_MAX];
    CollisionPrimitive::ptr crouching_shape_[QUADRANT_MAX];
    Quadrant quadrant_ = QUADRANT_FLOOR;
    GroundState ground_state_ = GROUND_STATE_IN_THE_AIR;

    // =========================================

	void start_horizontal_control_lock(double amount) {
		horizontal_control_lock_ = amount;
	}
	
	void update_horizontal_control_lock(double dt) {
		horizontal_control_lock_ -= dt;
		if(horizontal_control_lock_ < 0.0) horizontal_control_lock_ = 0.0;
	}

    Collision find_nearest_collision(const std::vector<Collision>& collisions);
    std::pair<Collision, bool> find_collision_with_ray(const std::vector<Collision>& collisions, char ray);
    
    SDdouble original_height_;
    SDdouble original_width_;
    SDdouble height_;
    SDdouble width_;
    
    bool moving_left_;
    bool moving_right_;
    bool looking_down_;
    bool jump_pressed_;
    bool waiting_for_jump_release_;
    
    bool rolling_;
    bool jumping_;
    bool is_grounded_;
    bool grounded_last_frame_;
    
    double horizontal_control_lock_;
    
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
