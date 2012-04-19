#include <iostream>
#include <cassert>
#include <limits>
#include "character.h"
#include "world.h"

const float WORLD_SCALE = 1.0f / 40.0f;

SDuint sdCharacterCreate(SDuint world_id) {
    World* world = get_world_by_id(world_id);
    return world->new_character();
}

void Character::pre_prepare(float dt) {

//    gsp = gsp * 60.0f;
}

void Character::prepare(float dt) {
    pre_prepare(dt);
    
    std::cout << "Setting ungrounded" << std::endl;
    is_grounded_ = false; //Reset the grounded flag
    
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

bool Character::respond_to(const std::vector<Collision>& collisions) {
	static uint64_t last_step_with_single_floor_collision = std::numeric_limits<uint64_t>::max();
	
	kmVec2 up;
	kmVec2Fill(&up, 0.0f, 1.0f);

	//Store the original position, we need this to work out
	//if anything changed
	kmVec2 original_position;
	kmVec2Assign(&original_position, &position());
	
	RayBox* ray_box = dynamic_cast<RayBox*>(&geom());
	
	kmVec2 normalized_speed;
	kmVec2Normalize(&normalized_speed, &speed());
	std::map<char, bool> hitmask;
	
	struct CollisionInfo {
		CollisionInfo():other(nullptr), distance(100000) {}
		
		kmVec2 normal;
		kmVec2 point;
		char ray;
		Object* other;
		float distance;
	};
	
	std::map<char, CollisionInfo> collision_info;
	
	std::cout << "Start" << std::endl;
	//Organize the collision data into something more useful
	for(Collision c: collisions) {
		kmVec2 normal = (c.object_a == &geom()) ? c.b_normal : c.a_normal;
		
		char ray = (c.object_a == &geom()) ? c.a_ray : c.b_ray;	 
		
		kmVec2 reversed_normal;
		reversed_normal.x = -normal.x;
		reversed_normal.y = -normal.y;	
		kmScalar angle_between_speed_and_normal = kmVec2DegreesBetween(&reversed_normal, &normalized_speed);
		std::cout << "Ray: " << ray << " Angle: " << angle_between_speed_and_normal << std::endl;		
		
		/*
		//FIXME: Only in the air?
		if(angle_between_speed_and_normal > 90.0 - 0.001) {
			std::cout << "Ignoring collision with ray: " << ray << " because angle is: " << angle_between_speed_and_normal << std::endl;
			continue;
		}
		*/
		
		Object* other = get_other_object_from_collision(c);
							
		kmScalar distance = kmVec2DistanceBetween(&c.point, &ray_box->ray(ray).start);		
		
		if(ray == 'A' || ray == 'B') {
			std::cout << "Setting grounded" << std::endl;
			is_grounded_ = (other) ? !other->has_collision_flag(NOT_GROUND) : true;				
		} 		
		
		kmScalar length = (ray == 'L' || ray == 'R') ? width_ / 2.0f : height_ / 2.0f;
		if(distance < length - 0.0001) {								
			if(hitmask[ray]) {
				//We've already seen a collision for this ray, only
				//store this one if distance is less than the existing one			
				if(distance >= collision_info[ray].distance) {
					std::cout << "Ignoring collision too far" << std::endl;	
					continue;
				}
			}					
			hitmask[ray] = true;			
			collision_info[ray].ray = ray;
			collision_info[ray].normal = normal;
			collision_info[ray].point = c.point;
			collision_info[ray].other = other;		
			collision_info[ray].distance = distance;
		} else {
			std::cout << "Ignoring collision with ray because the distance was too far: " << ray << "(" << distance << ")" << std::endl;
		}
	}
	
	//If both A and B rays hit, we work out a midpoint to calculate
	// the response
	if(hitmask['A'] && hitmask['B']) {
		if(1) {
			/**
			 * 	We've hit the floor with both rays, what do we do??
			 *  1. Find the point mid-way between both intersections
			 *  2. Calculate a normal by averaging the two collision normals
			 *  3. Set the position of the character to the mid-way point - (normal * height / 2)
			 *  4. Rotate the character to the floor
			 */			
			std::cout << "Both" << std::endl;
			kmVec2 intersection, normal;
			kmVec2MidPointBetween(&intersection, &collision_info['A'].point, &collision_info['B'].point);
			kmVec2Add(&normal, &collision_info['A'].normal, &collision_info['B'].normal);
			kmVec2Normalize(&normal, &normal);
			std::cout << "N1: " << collision_info['A'].normal.x << ", " << collision_info['A'].normal.y << std::endl;
			std::cout << "N2: " << collision_info['B'].normal.x << ", " << collision_info['B'].normal.y << std::endl;
			kmVec2 scaled_normal, new_pos;
			kmVec2Scale(&scaled_normal, &normal, (height_ / 2.0f));
			kmVec2Add(&new_pos, &intersection, &scaled_normal);

			std::cout << "New pos: (" << new_pos.x << ", " << new_pos.y << ")" << std::endl;
			assert(!isnan(new_pos.x));
			assert(!isnan(new_pos.y));
						
			set_position(new_pos.x, new_pos.y);
			SDdouble angle = kmVec2DegreesBetween(&normal, &up);
			set_rotation(-angle);	
		} 
	} else if (hitmask['A'] || hitmask['B']) {
		/**
		 * 	We've hit the floor with one ray, what do we do??
		 *  1. Move the character away from the collision along the ray
		 *  2. Rotate the position around the intersection until we are perpendicular 
		 *  3. Set the rotation angle
		 */			
			 		
		std::cout << "Single" << std::endl;
		//Only one floor ray collided, we need to move the character
		//out of the floor and rotate around the collision point
		CollisionInfo info = (hitmask['A']) ? collision_info['A'] : collision_info['B'];
		kmRay2 ray = (hitmask['A']) ? ray_box->ray('A') : ray_box->ray('B');
		
		kmScalar dist = kmVec2DistanceBetween(&info.point, &ray.start);
		
		if(1) {		
			//Move out
			kmVec2 to_move, new_pos;
			kmVec2Normalize(&to_move, &ray.dir);
			kmVec2Scale(&to_move, &to_move, -((height_ / 2.0f) - dist));
			assert(!isnan(to_move.x));
			assert(!isnan(to_move.y));
			kmVec2Add(&new_pos, &position(), &to_move);
			
			kmVec2 reversed_ray_dir;
			kmVec2Normalize(&reversed_ray_dir, &ray.dir);
			kmVec2Fill(&reversed_ray_dir, -reversed_ray_dir.x, -reversed_ray_dir.y);		
			
			//And rotate the position by that angle, around the intersection point
			kmScalar angle = kmVec2DegreesBetween(&reversed_ray_dir, &info.normal);
			if(isnan(angle)) {
				assert(0 && "ERROR: angle is NaN");
			}
					
			kmVec2RotateBy(&new_pos, &new_pos, angle, &info.point);		

			//Finally, we set the angle relative to up
			kmScalar rotation = kmVec2DegreesBetween(&info.normal, &up);
			set_rotation(-rotation);	
			std::cout << "New pos: (" << new_pos.x << ", " << new_pos.y << ")" << std::endl;
			assert(!isnan(new_pos.x));
			assert(!isnan(new_pos.y));
			set_position(position().x, new_pos.y);			
		} 	
	} else {
		std::cout << "No A-B collision" << std::endl;
	}
	
	if (hitmask['L'] || hitmask['R']) {
		CollisionInfo info = (hitmask['L']) ? collision_info['L'] : collision_info['R'];
		kmRay2& ray = (hitmask['L']) ? ray_box->ray('L') : ray_box->ray('R');		
		
		float dist = kmVec2DistanceBetween(&info.point, &ray.start);
		
		//Move out
		kmVec2 to_move, new_pos;
		kmVec2Normalize(&to_move, &ray.dir);
		kmVec2Scale(&to_move, &to_move, (width_ / 2.0f) - dist);
		kmVec2Subtract(&new_pos, &position(), &to_move);
		set_position(new_pos.x, new_pos.y);
		
		if(is_grounded_) {
			gsp_ = 0.0f;
		} else {
			speed_.x = 0.0f; //FIXME: Take into account rotation
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
    
    float sin_rot = sinf(kmDegreesToRadians(rotation()));
    float slp = SLP;
    
    static bool jump_pressed_last_time = false;
    static bool grounded_last_frame = false;
    
    if(!grounded_last_frame && is_grounded_) {
        std::cout << "Reorienting: " << rotation() << std::endl;
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
        std::cout << "Unrolling" << std::endl;
        stop_rolling();
    }
    
    //std::cout << fabs(gsp_) << std::endl;
    if(fabs(gsp_) > ALLOWED_ROLL_SPEED && looking_down_ && !rolling_) {
        std::cout << "Rolling" << std::endl;
        start_rolling();
    }
        
    if(rolling_) {
        if(sgn(sin_rot) == sgn(gsp_) || sgn(gsp_) == 0) {
            slp = SLP_DOWNHILL;
        } else {
            slp = SLP_UPHILL;
        }
    }
    
    if(is_grounded_) {
        gsp_ += slp * sin_rot * dt;
    } else {
        set_rotation(0);        
    }

    //Done this way so pressing left and right at the same time, does nothing
    if(moving_left_ && !(moving_left_ && moving_right_)) {
        if(is_grounded_) {
            if(gsp_ <= 0.0f) {
                //You can't accelerate while rolling
                if(!rolling_) gsp_ += (-ACC * dt);
            } else {
                float dec_val = (rolling_) ? -(DEC * 0.5f) : -DEC;
                gsp_ += (dec_val * dt);
            }
        } else {
            //Accelerate twice as fast in the air
            speed_.x += (-ACC * 2.0f * dt);         
        }
    } else if(moving_right_ && !(moving_left_ && moving_right_)) {
        if(is_grounded_) {
            if(gsp_ >= 0.0f) {
                //You can't accelerate while rolling
                if(!rolling_) gsp_ += (ACC * dt);
            } else {
                float dec_val = (rolling_) ? (DEC * 0.5f) : DEC;
                gsp_ += dec_val * dt;
            }        
        } else {
            //Accelerate twice as fast in the air
            speed_.x += (ACC * 2.0f * dt);         
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
    
    if(moving_left_) {
        if(gsp_ < -TOP) {
            gsp_ = -TOP;
        }
    } else if (moving_right_) {
        if(gsp_ > TOP) {
            gsp_ = TOP;
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
        speed_.y = gsp_ * -sin(kmDegreesToRadians(rotation())) - 0.0001;    
    } else {
        //Air drag effect
        /*
        if(speed_.y > 0 && speed_.y < (4.0f * WORLD_SCALE)) {
            speed_.x = speed_.x - ((speed_.x / (0.125f * WORLD_SCALE)) / 256.0f);
        }*/

        speed_.y -= GRV * dt;        
    }
    
    jump_pressed_last_time = jump_pressed_;
    grounded_last_frame = is_grounded_;
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

void sdCharacterStartJumping(SDuint character) {
    Character* c = get_character(character);
    c->start_jumping();
}

void sdCharacterStopJumping(SDuint character) {
    Character* c = get_character(character);
    c->stop_jumping();
}

void sdCharacterStopRolling(SDuint character) {
    Character* c = get_character(character);
    c->stop_rolling();
}

SDbool sdCharacterIsGrounded(SDuint character) {
    Character* c = get_character(character);
    return c->is_grounded();
}

SDbool sdObjectIsCharacter(SDuint object) {
    Character* c = get_character(object);
    return (c) ? true: false;
}
