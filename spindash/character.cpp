#include <iostream>
#include <cassert>
#include <limits>
#include <boost/format.hpp>
#include <kazbase/logging.h>

#include "character.h"
#include "world.h"


static std::map<std::string, float> CHARACTER_SETTINGS = {
    { "VERTICAL_SENSOR_EXTENSION_LENGTH", 16.0 }
};

float Character::setting(const std::string &setting) {
    auto it = CHARACTER_SETTINGS.find(setting);
    assert(it != CHARACTER_SETTINGS.end());
    return it->second;
}

void Character::override_setting(const std::string &setting, float value) {
    CHARACTER_SETTINGS[setting] = value;
}

const float WORLD_SCALE = 1.0f / 40.0f;

Character::Character(World* world, SDdouble width, SDdouble height):
    Object(world),
    original_height_(height),
    original_width_(width),
    height_(height),
    width_(width),
    moving_left_(false),
    moving_right_(false),
    looking_down_(false),
    jump_pressed_(false),
    waiting_for_jump_release_(false),
    rolling_(false),
    jumping_(false),
    is_grounded_(false),
    horizontal_control_lock_(0),
    facing_(DIRECTION_RIGHT),
    spindash_charge_(0),
    gsp_(0.0),
    enabled_skills_(0) {

    enable_skill(SD_SKILL_ROLL);
    enable_skill(SD_SKILL_SPINDASH);

    standing_shape_ = std::make_shared<RayBox>(
        this,
        width,
        height + (Character::setting("VERTICAL_SENSOR_EXTENSION_LENGTH") * 2)

    );

    crouching_shape_ = std::make_shared<RayBox>(
        this,
        (width * 0.75),
        (height * 0.75) + (Character::setting("VERTICAL_SENSOR_EXTENSION_LENGTH") * 2)
    );

    //Copy the standing shape to set the geom to default
    set_geom(standing_shape_);
}

void Character::set_size(CharacterSize size) {
    size_ = size;

    float quarter_height = height_ * 0.25;

    if(size_ == CHARACTER_SIZE_CROUCHING) {
        //Copy the crouching shape to set the geom to default
        set_geom(crouching_shape_);
        set_position(position().x, position().y - (quarter_height / 2));
    } else {
        //Copy the standing shape to set the geom to default
        set_geom(standing_shape_);
        set_position(position().x, position().y + (quarter_height / 2));
    }
}


SDuint sdCharacterCreate(SDuint world_id) {
    World* world = get_world_by_id(world_id);
    return world->new_character();
}

void Character::pre_prepare(float dt) {

//    gsp = gsp * 60.0f;
}

void Character::prepare(float dt) {
    pre_prepare(dt);
    
    is_grounded_ = false;
    
    post_prepare(dt);
}

Collision Character::find_nearest_collision(const std::vector<Collision>& collisions) {
    float closest = 1000000.0f;
    Collision found;
    RayBox* ray_box = dynamic_cast<RayBox*>(&geom());
    for(Collision c: collisions) {
        char r = (c.object_a == &geom()) ? c.a_ray : c.b_ray;
        kmRay2& ray = ray_box->ray(r);
        
        kmVec2 vec_to_intersection;
        kmVec2Subtract(&vec_to_intersection, &c.point, &ray.start);
        float dist_to_intersection = kmVec2Length(&vec_to_intersection);
        if(dist_to_intersection < closest) {
            found = c;
            closest = dist_to_intersection;
        }
    }
    
    return found;
}

std::pair<Collision, bool> Character::find_collision_with_ray(const std::vector<Collision>& collisions, char ray) {
    bool found = false;
    
    uint32_t i = 0;
    for(; i < collisions.size(); ++i) {
        const Collision& c = collisions.at(i);
        if(c.object_a == &geom() && c.a_ray == ray) {
            found = true;
            break;
        }
        if(c.object_b == &geom() && c.b_ray == ray) {
            found = true;
            break;
        }
    }
    if(found) {
        Collision found_col = collisions.at(i);
        return std::pair<Collision, bool>(found_col, found);
    }
    
    return std::pair<Collision, bool>(Collision(), false);
}

enum FourMode {
	FLOOR = 0,
	RIGHT,
	CEILING,
	LEFT
};

FourMode get_mode_from_rotation(kmScalar angle) {
	angle = (angle < 0) ? 360.0 + angle : angle;
	
	if(angle >= (360.0 - 45.0) || angle < 45.0) return FLOOR;
	else if(angle >= (360.0f - (90.0 + 45)) && angle < (360.0 - 45.0)) return RIGHT;
	else if(angle >= (360.0f - (180.0 + 45)) && angle < (360.0 - (90.0 + 45))) return CEILING;
	else return LEFT;
}

kmVec2 get_up_for_mode(FourMode mode) {
	kmVec2 up;
	switch(mode) {
		case FLOOR: {
			kmVec2Fill(&up, 0.0, 1.0);			
		} break;
		case RIGHT: {
			kmVec2Fill(&up, -1.0, 0.0);
		} break;
		case CEILING: { 
			kmVec2Fill(&up, 0.0, -1.0);
		} break;
		case LEFT: {
			kmVec2Fill(&up, 1.0, 0.0);
		} break;
	}
	return up;
}

kmVec2 highest_point(FourMode mode, kmVec2& p1, kmVec2& p2) {
	if(mode == FLOOR) {
		return (p1.y > p2.y) ? p1 : p2;
	} else if (mode == RIGHT) {
		return (p1.x < p2.x) ? p1 : p2;
	} else if (mode == CEILING) {
		return (p1.y < p2.y) ? p1 : p2;
	} else {
		return (p1.x > p2.x) ? p1 : p2;
	}
}

std::pair<Collision, bool> find_nearest_collision_with_ray(
	const std::vector<Collision>& collisions,
	char ray,
	const kmVec2& speed,
	bool ignore_based_on_speed /* normally !is_grounded_ */) {

	kmVec2 normalized_speed;
	double speed_length = kmVec2Length(&speed);
	bool ignore_speed = false;
	if(speed_length) {
		kmVec2Normalize(&normalized_speed, &speed);
	}

	bool found = false;
	double closest = std::numeric_limits<double>::max();
	Collision final;
	
	for(Collision c: collisions) {						
		if(c.a_ray == ray) {			
			RayBox* ray_box = dynamic_cast<RayBox*>(c.object_a);
			
			kmScalar distance = kmVec2DistanceBetween(&c.point, &ray_box->ray(ray).start);		
			
			if(distance < closest) {
				final = c;
				closest = distance;
				found = true;
			}
		}
	}
	
	if(found) {
		RayBox* ray_box = dynamic_cast<RayBox*>(final.object_a);
		
		//Ignore collisions with rays when the angle is too steep for the ray
		double angle_diff = kmVec2DegreesBetween(&final.b_normal, &ray_box->ray(ray).dir);
        if(angle_diff < 180.0 - 45.0) {
			return std::make_pair(final, false);
		}

		if(speed_length && ignore_based_on_speed) {
			kmScalar angle_between_speed_and_normal = kmVec2DegreesBetween(&final.b_normal, &normalized_speed);			
			
			if(angle_between_speed_and_normal < 90.0) {
                //L_DEBUG((boost::format("Ignoring collision with ray: %s because angle is: %f") % ray % angle_between_speed_and_normal).str());
				return std::make_pair(final, false);
            }
		}
	}
	
	
	return std::make_pair(final, found);
}

double calc_angle_from_up(const kmVec2& vec) {
	kmVec2 up, n;
	kmVec2Fill(&up, 0.0, 1.0);
	kmVec2Normalize(&n, &vec);
		
	double angle = kmVec2DegreesBetween(&up, &n);

	if(vec.x * up.y - vec.y * up.x <= 0) {
		angle = -angle;
	}
	
	return (angle < 0.0) ? 360.0 + angle : angle;
}

bool Character::respond_to(const std::vector<Collision>& collisions) {
	std::pair<Collision, bool> a = find_nearest_collision_with_ray(collisions, 'A', speed(), !grounded_last_frame_);
	std::pair<Collision, bool> b = find_nearest_collision_with_ray(collisions, 'B', speed(), !grounded_last_frame_);
	std::pair<Collision, bool> l = find_nearest_collision_with_ray(collisions, 'L', speed(), !grounded_last_frame_);
	std::pair<Collision, bool> r = find_nearest_collision_with_ray(collisions, 'R', speed(), !grounded_last_frame_);
	
	//Store the original position, we need this to work out
	//if anything changed
	kmVec2 original_position;
	kmVec2Assign(&original_position, &position());
	
    is_grounded_ = false;

	if (l.second || r.second) {
		RayBox* ray_box = dynamic_cast<RayBox*>(&geom());

		Collision &info = (l.second) ? l.first : r.first;
		kmRay2& ray = (l.second) ? ray_box->ray('L') : ray_box->ray('R');		

		double dist = kmVec2DistanceBetween(&info.point, &ray.start);
		
		/*kmVec2 new_pos;
		kmVec2 reversed_ray;		
		kmVec2Scale(&reversed_ray, &ray.dir, -1.0);
		kmVec2Normalize(&reversed_ray, &reversed_ray);
		kmVec2Scale(&reversed_ray, &reversed_ray, width_ / 2.0);
		kmVec2Add(&new_pos, &info.point, &reversed_ray);*/
		//Move out
		kmVec2 to_move, new_pos;
		kmVec2Normalize(&to_move, &ray.dir);
		kmVec2Scale(&to_move, &to_move, (width_ / 2.0f) - (dist * 0.99));
		kmVec2Subtract(&new_pos, &position(), &to_move);
		set_position(new_pos.x, new_pos.y);
		
		if(is_grounded_) {
			if(l.second && gsp_ < 0.0) gsp_ = 0.0f;
			if(r.second && gsp_ > 0.0) gsp_ = 0.0f;
		} else {
			speed_.x = 0.0f; //FIXME: Take into account rotation
		}		
		
		if(a.second || b.second) {
			is_grounded_ = true;
		}
		
		return true;
	} 
	
	if(a.second || b.second) {
        RayBox* ray_box = dynamic_cast<RayBox*>(&geom());
        float a_dist = std::numeric_limits<float>::max();
        float b_dist = std::numeric_limits<float>::max();

        if(a.second) {
            a_dist = kmVec2DistanceBetween(&a.first.point, &ray_box->ray('A').start);
        }

        if(b.second) {
            b_dist = kmVec2DistanceBetween(&a.first.point, &ray_box->ray('B').start);
        }

        kmVec2 diff;
        diff.x = 0;
        diff.y = height_ / 2.0;

        kmVec2 up;
        up.x = 0;
        up.y = 1;

        kmVec2 new_location;
        float new_angle;
        if(a_dist <= b_dist) {
            kmVec2Add(&new_location, &a.first.point, &diff);
            new_angle = kmRadiansToDegrees(acos(kmVec2Dot(&up, &a.first.b_normal)));
        } else {
            kmVec2Add(&new_location, &b.first.point, &diff);
            new_angle = kmRadiansToDegrees(acos(kmVec2Dot(&up, &b.first.b_normal)));
        }

        set_position(new_location.x, new_location.y);
        set_rotation(new_angle);
        is_grounded_ = true;

        return !kmVec2AreEqual(&original_position, &position());

		kmVec2 normal, intersection;
		
		if(a.second && b.second) {
			/*
			 * Find the line between the two collision points
			 * Calculate a new normal and intersection
			 * Set the position to intersection + (normal*height)
			 * Set the rotation to the normal vs up
			 */			 
             kmVec2MidPointBetween(&intersection, &a.first.point, &b.first.point);
			 
			 kmVec2Add(&normal, &a.first.b_normal, &b.first.b_normal);
			 kmVec2Normalize(&normal, &normal);			 
		} else {
			/*
			 * if a: intersection = a.point + (a.normal + 90 degrees) * width / 2;
			 * elif b: intersection = b.point - (b.normal - 90 degrees) * width / 2;
			 * Set the position to intersection + (normal * height)
			 * Set the rotation to the normal vs up
			 */
			 kmVec2 origin;
			 kmVec2Fill(&origin, 0, 0);
			 
			 kmVec2 point = (a.second) ? a.first.point : b.first.point;
			 
			 normal = (a.second) ? a.first.b_normal : b.first.b_normal;
			 
			 kmVec2 edge;			 
			 if(a.second) {				 
				 kmVec2RotateBy(&edge, &normal, -90.0, &origin); //Rotate the normal by 90 degrees
			 } else {
				 kmVec2RotateBy(&edge, &normal, 90.0, &origin); //Rotate the normal by -90 degrees
			 }			 			 
			 
			 double vray_diff = kmVec2DistanceBetween(&ray_box->ray('A').start, &ray_box->ray('B').start);
			 
			 kmVec2Scale(&edge, &edge, vray_diff / 2.0);
			 kmVec2Add(&intersection, &point, &edge); //Calculate the point underneath the position			 
		}
		
		kmVec2 new_pos, scaled_normal;
		kmVec2Scale(&scaled_normal, &normal, height_ / 2.0);
		kmVec2Add(&new_pos, &intersection, &scaled_normal);

		set_position(new_pos.x, new_pos.y);
		set_rotation(calc_angle_from_up(normal));		 
		is_grounded_ = true;		

		if((rotation() > 45.0 + kmEpsilon && rotation() < (360.0 - 45.0) - kmEpsilon) && fabs(gsp_) < (2.5 * WORLD_SCALE)) {
			is_grounded_ = false;
		}
	} 
	
	//If the position changed, re-run the collision loop
	return !kmVec2AreEqual(&original_position, &position());
}

void Character::update_finished(float dt) {
    const float ACC = 0.046875f * WORLD_SCALE * 60.0f;
    const float FRC = ACC;
    const float DEC = 0.5f * WORLD_SCALE * 60.0f;
    const float TOP = 6.0f * WORLD_SCALE;
    const float SLP = 0.125f * WORLD_SCALE * 60.0f;
    const float GRV = 0.21875f * WORLD_SCALE * 60.0f;
    const float JMP = 6.5 * WORLD_SCALE;
    const float SLP_UPHILL = 0.078125f * WORLD_SCALE * 60.0f;
    const float SLP_DOWNHILL = 0.3125f * WORLD_SCALE * 60.0f; 
    const float UNROLL_SPEED = 0.5f * WORLD_SCALE;
    const float ALLOWED_ROLL_SPEED = 1.03125f * WORLD_SCALE;
    
    double sin_rot = sin(kmDegreesToRadians(rotation()));
    double slp = SLP;
    
    static bool jump_pressed_last_time = false;
    
    update_horizontal_control_lock(dt);
    
    //Falling off of walls and ceilings
    if((rotation() > 45.0 && rotation() < 360.0 - 45.0) && fabs(gsp_) < (2.5 * WORLD_SCALE)) {
		gsp_ = 0.0;
		//is_grounded_ = false; //FIXME: this needs setting but causes craziness
		start_horizontal_control_lock(0.5); //FIXME: start when we hit the ground not immediately
	}
        
    if(looking_down_ && spindash_charge_) {
		spindash_charge_ = spindash_charge_ - (floor(spindash_charge_ / 0.125) / 256.0);
	} else if(spindash_charge_) {
		double boost = 8.0 + (floor(spindash_charge_) / 2.0);
		if(facing_ == DIRECTION_RIGHT) {
			gsp_ = boost * WORLD_SCALE;
		} else {
			gsp_ = -boost * WORLD_SCALE;
		}
		spindash_charge_ = 0.0;
		start_rolling();
	}
    
    if(!grounded_last_frame_ && is_grounded_ /*&& !waiting_for_jump_release_*/) {
        L_DEBUG("Reorienting");
        stop_jumping();
        stop_rolling(); //Sonic stops rolling if when you hit the ground
        if(speed_.y < 0.0f) {
            //We just hit the ground, we need to recalculate gsp
            if((rotation() >= 360.0f - 15.0f && rotation() <= 360.0f) ||
               (rotation() >= 0.0f && rotation() <= 15.0f)) {
                gsp_ = speed_.x;
            } else if ((rotation() >= 16.0f && rotation() <=  31.0f) ||
                       (rotation() >= 360.0f - 32.0f && rotation() <= 360.0f - 16.0f)) {
                if(fabs(speed_.x) > speed_.y) {
                    gsp_ = speed_.x;
                } else {
                    gsp_ = speed_.y * 0.5f * sgn(cosf(kmDegreesToRadians(rotation())));
                }
            } else if((rotation() >= 32.0f && rotation() <= 63.0f) ||
                      (rotation() >= 360.0f - 63.0f && rotation() <= 360.0f - 32.0f)){
                if(fabs(speed_.x) > speed_.y) {
                    gsp_ = speed_.x;
                } else {
                    //FIXME: sgn may return zero...
                    gsp_ = speed_.y * -sgn(cosf(kmDegreesToRadians(rotation())));
                }
            }
        }
    }
    
    //Unroll if we go too slow
    if(rolling_ && fabs(gsp_) < UNROLL_SPEED) {
        L_DEBUG("Unrolling as speed dropped too low");
        stop_rolling();
    }
    
    //std::cout << fabs(gsp_) << std::endl;
    if(fabs(gsp_) > ALLOWED_ROLL_SPEED && looking_down_ && !rolling_) {
        L_DEBUG("Rolling");        
		start_rolling();		
    }
        
    if(rolling_) {
        if(sgn(sin_rot) == sgn(gsp_) || sgn(gsp_) == 0) {
            slp = SLP_DOWNHILL;
            L_DEBUG("Using downhill slope speed");
        } else {
            slp = SLP_UPHILL;
            L_DEBUG("Using up slope speed");
        }
    }
    
    if(is_grounded_) {
        gsp_ += slp * sin_rot * dt;
    } else {
        set_rotation(0);        
    }

    //Done this way so pressing left and right at the same time, does nothing
    if(moving_left_ && !(moving_left_ && moving_right_) && !horizontal_control_lock_active()) {
        if(is_grounded_) {
            if(gsp_ <= 0.0f) {
                //You can't accelerate while rolling
                if(!rolling_) {
					if(gsp_ > -TOP) {
						gsp_ += (-ACC * dt);
					}
				}
                facing_ = DIRECTION_LEFT;
            } else {
                float dec_val = (rolling_) ? -(DEC * 0.5f) : -DEC;
                gsp_ += (dec_val * dt);                
            }
        } else {
            //Accelerate twice as fast in the air
            speed_.x += (-ACC * 2.0f * dt);    
            facing_ = DIRECTION_LEFT;           
        }
    } else if(moving_right_ && !(moving_left_ && moving_right_) && !horizontal_control_lock_active()) {
        if(is_grounded_) {
            if(gsp_ >= 0.0f) {
                //You can't accelerate while rolling
                if(!rolling_) { 
					if(gsp_ < TOP) {
						gsp_ += (ACC * dt);
					}
				}
                facing_ = DIRECTION_RIGHT;
            } else {
                float dec_val = (rolling_) ? (DEC * 0.5f) : DEC;                
                gsp_ += dec_val * dt;
            }        
        } else {
            //Accelerate twice as fast in the air
            speed_.x += (ACC * 2.0f * dt);   
            facing_ = DIRECTION_RIGHT;      
        }
    } 
    
    /*
        If we are pressing no directions, or we are pressing both,
        or we are rolling. Apply friction
    */
    if(is_grounded_) {
        if((!moving_left_ && !moving_right_) || (moving_left_ && moving_right_) || rolling_) {
            float frc_val = (rolling_) ? FRC * 0.5f : FRC;
            float frc = std::min((float)fabs(gsp_), frc_val * dt);
            if(gsp_ > 0.0f) gsp_ += -frc;
            if(gsp_ < 0.0f) gsp_ += frc;    
        }
    }
    
    if(waiting_for_jump_release_ && !jump_pressed_) {
        waiting_for_jump_release_ = false;
        if(speed_.y > (4.0f * WORLD_SCALE)) {
            speed_.y = (4.0f * WORLD_SCALE);
        }
    }
    
    if(is_grounded_ && jump_pressed_ && !jump_pressed_last_time) {
        //Did we just start jumping?
        speed_.x += JMP * sinf(kmDegreesToRadians(rotation()));
        speed_.y += JMP * cosf(kmDegreesToRadians(rotation()));
        
        //speed_.y = JMP;
        is_grounded_ = false;
        waiting_for_jump_release_ = true;
        
        start_jumping();
        
        //return false; //Prevent move this frame
    }
    
    if(is_grounded_) {
		/*
		 * 	The 'R' ray of the raybox always points to the local "forwards"
		 *  so we use that to calculate the speed vector based on gsp_
		 *  rather than calculating it with cos/sine
		 */
		/*RayBox* ray_box = dynamic_cast<RayBox*>(&geom());
		kmVec2Normalize(&speed_, &ray_box->ray('R').dir);
		kmVec2Scale(&speed_, &speed_, gsp_);*/
        speed_.x = gsp_ * cos(kmDegreesToRadians(rotation()));       
        
        //Always add a little bit of gravity, to combat floating point errors
        //FIXME: This should use the down vector scaled really fucking small
        speed_.y = gsp_ * -sin(kmDegreesToRadians(rotation()));    
    } else {
        //Air drag effect
        /*
        if(speed_.y > 0 && speed_.y < (4.0f * WORLD_SCALE)) {
            speed_.x = speed_.x - ((speed_.x / (0.125f * WORLD_SCALE)) / 256.0f);
        }*/

        speed_.y -= GRV * dt;        
    }
    
    jump_pressed_last_time = jump_pressed_;
    grounded_last_frame_ = is_grounded_;
}

void Character::set_speed(float x, float y) {
    /*
        Characters ignore the speed when on the ground
        and in-fact override it each frame. The best thing
        we can do here is, if someone sets the speed manually,
        to make the character not on the ground
    */
    is_grounded_ = false;
    speed_.x = x;
    speed_.y = y;
}

void Character::post_prepare(float dt) {
    
}
    
//================================================

static Character* get_character(SDuint object_id) {
    Object* obj = Object::by_id(object_id);
    Character* c = dynamic_cast<Character*>(obj);
    return c;
}

void sdCharacterStartMovingLeft(SDuint character) {
    Character* c = get_character(character);
    c->start_moving_left();
}

void sdCharacterStopMovingLeft(SDuint character) {
    Character* c = get_character(character);
    c->stop_moving_left();
}

void sdCharacterStartMovingRight(SDuint character) {
    Character* c = get_character(character);
    c->start_moving_right();
}

void sdCharacterStopMovingRight(SDuint character) {
    Character* c = get_character(character);
    c->stop_moving_right();
}

void sdCharacterStartLookingDown(SDuint character) {
    Character* c = get_character(character);
    c->start_looking_down();
}

SDdouble sdCharacterGetWidth(SDuint character) {
    Character* c = get_character(character);
    return c->width();	
}

void sdCharacterStopLookingDown(SDuint character) {
    Character* c = get_character(character);
    c->stop_looking_down();
}

void sdCharacterStartPressingJump(SDuint character) {
    Character* c = get_character(character);
    c->start_pressing_jump();
}

void sdCharacterStopPressingJump(SDuint character) {
    Character* c = get_character(character);
    c->stop_pressing_jump();
}

void sdCharacterStopRolling(SDuint character) {
    Character* c = get_character(character);
    c->stop_rolling();
}

void sdCharacterStopJumping(SDuint character) {
    Character* c = get_character(character);
    c->stop_jumping();    
}

SDbool sdCharacterIsGrounded(SDuint character) {
    Character* c = get_character(character);
    return c->is_grounded();
}

SDbool sdObjectIsCharacter(SDuint object) {
    Character* c = get_character(object);
    return (c) ? true: false;
}

void sdCharacterSetGroundSpeed(SDuint character, SDdouble value) {
	Character* c = get_character(character);
	c->set_ground_speed(value);
}

SDdouble sdCharacterGetGroundSpeed(SDuint character) {
	Character* c = get_character(character);
	return c->ground_speed();	
}

void sdCharacterEnableSkill(SDuint character, sdSkill skill) {
	Character* c = get_character(character);
	c->enable_skill(skill);	
}

void sdCharacterDisableSkill(SDuint character, sdSkill skill) {
	Character* c = get_character(character);
	c->disable_skill(skill);		
}

SDbool sdCharacterSkillEnabled(SDuint character, sdSkill skill) {
	Character* c = get_character(character);
	return c->skill_enabled(skill);
}

SDdouble sdCharacterGetSpindashCharge(SDuint character) {
	Character* c = get_character(character);
	return c->spindash_charge();
}

SDbool sdCharacterIsJumping(SDuint character) {
	Character* c = get_character(character);
	return c->jumping();
}

SDbool sdCharacterIsRolling(SDuint character) {
	Character* c = get_character(character);
	return c->rolling();
}

void sdCharacterOverrideSetting(const char* setting, float value) {
    Character::override_setting(setting, value);
}
