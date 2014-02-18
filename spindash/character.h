#ifndef CHARACTER_H
#define CHARACTER_H

#include <map>

#include "spindash.h"
#include "object.h"
#include "kazmath/ray2.h"
#include "collision/ray_box.h"

enum AxisState {
    AXIS_STATE_NEGATIVE = -1,
    AXIS_STATE_NEUTRAL = 0,
    AXIS_STATE_POSITIVE = 1
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

const float DEFAULT_ACCELERATION_IN_MPS = ((0.046875 / 40.0) * 60.0);
const float DEFAULT_DECELERATION_IN_MPS = ((0.5 / 40.0) * 60.0);
const float DEFAULT_FRICTION_IN_MPS = DEFAULT_ACCELERATION_IN_MPS;
const float DEFAULT_TOP_SPEED_IN_M = ((6.0 / 40.0));
const float DEFAULT_TOP_Y_SPEED_IN_M = ((16.0 / 40.0));
const float DEFAULT_SLOPE_IN_MPS = ((0.125 / 40.0) * 60.0);
const float DEFAULT_INITIAL_JUMP_IN_M = ((6.5 / 40.0));
const float DEFAULT_JUMP_CUT_OFF_IN_M = ((4.0 / 40.0));
const float DEFAULT_AIR_DRAG_RATE = 0.96875 * 60.0; //This is the fraction left after a second
const float DEFAULT_AIR_DRAG_MIN_X_SPEED = ((0.125 / 40.0));
const float DEFAULT_AIR_DRAG_MAX_Y_SPEED = DEFAULT_JUMP_CUT_OFF_IN_M;
const float ANIMATION_RUNNING_MIN_X_SPEED = (6.0 / 40.0);
const float ANIMATION_DASHING_MIN_X_SPEED = (10.0 / 40.0);
const float MIN_ROLLING_SPEED = (1.03125 / 40.0);

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

    void set_ground_state(GroundState state) { ground_state_ = state; }
    GroundState ground_state() const { return ground_state_; }

    void set_gsp(float gsp) { gsp_ = gsp; }
    float gsp() const { return gsp_; }

    //======================================


    void move_left() {
        x_axis_state_ = AXIS_STATE_NEGATIVE;
	}

    void move_right() {
        x_axis_state_ = AXIS_STATE_POSITIVE;
	}

    void move_up() {
        y_axis_state_ = AXIS_STATE_POSITIVE;
    }

    void move_down() {
        y_axis_state_ = AXIS_STATE_NEGATIVE;
    }

    void jump() {
        action_button_state_ = true;
    }

    bool is_grounded() { return ground_state_ != GROUND_STATE_IN_THE_AIR; }

    bool respond_to(const std::vector<Collision>& collisions);
    
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


    void update_finished(float dt);

    AnimationState animation_state() const { return animation_state_; }

    Direction facing() const { return facing_; }
private:

    // ============== NEW STUFF ================

    CharacterSize size_ = CHARACTER_SIZE_STANDING;
    CollisionPrimitive::ptr standing_shape_[QUADRANT_MAX];
    CollisionPrimitive::ptr crouching_shape_[QUADRANT_MAX];
    Quadrant quadrant_ = QUADRANT_FLOOR;
    GroundState ground_state_ = GROUND_STATE_IN_THE_AIR;
    bool is_grounded() const { return ground_state_ != GROUND_STATE_IN_THE_AIR; }

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
    
    SDdouble original_height_ = 0;
    SDdouble original_width_ = 0;
    SDdouble height_ = 0;
    SDdouble width_ = 0;

    AxisState x_axis_state_ = AXIS_STATE_NEUTRAL;
    AxisState y_axis_state_ = AXIS_STATE_NEUTRAL;
    AxisState last_x_axis_state_ = AXIS_STATE_NEUTRAL;
    AxisState last_y_axis_state_ = AXIS_STATE_NEUTRAL;

    AnimationState animation_state_ = ANIMATION_STATE_STANDING;

    bool action_button_state_ = false;
    bool last_action_button_state_ = false;

    bool grounded_last_frame_ = false;
    uint32_t enabled_skills_ = 0;

    double horizontal_control_lock_ = 0.0;
    
    Direction facing_ = DIRECTION_RIGHT;
    
    double spindash_charge_ = 0.0;

    double gsp_ = 0.0;

    void pre_prepare(float dt);
    void post_update(float dt) {
        ground_state_ = GROUND_STATE_IN_THE_AIR;
    }

    float acceleration_rate_ = DEFAULT_ACCELERATION_IN_MPS;
    float deceleration_rate_ = DEFAULT_DECELERATION_IN_MPS;
    float friction_rate_ = DEFAULT_FRICTION_IN_MPS;
    float top_speed_ = DEFAULT_TOP_SPEED_IN_M;
    float top_y_speed_ = DEFAULT_TOP_Y_SPEED_IN_M;
    float slope_rate_ = DEFAULT_SLOPE_IN_MPS;
    float jump_rate_ = DEFAULT_INITIAL_JUMP_IN_M;
    float jump_cut_off_ = DEFAULT_JUMP_CUT_OFF_IN_M;
    float air_drag_rate_ = DEFAULT_AIR_DRAG_RATE;
    float air_drag_min_x_ = DEFAULT_AIR_DRAG_MIN_X_SPEED;
    float air_drag_max_y_ = DEFAULT_AIR_DRAG_MAX_Y_SPEED;
};

extern bool debug_break;

#endif
