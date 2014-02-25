#include <iostream>
#include <cassert>
#include <limits>
#include <boost/format.hpp>
#include <kazbase/logging.h>

#include "character.h"
#include "world.h"

const float MIN_SPEED_TO_AVOID_FAILING_IN_MPS = (2.5 / 40.0) * 60.0;

bool debug_break = false;

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

Character::Character(World* world, SDfloat width, SDfloat height):
    Object(world),
    original_height_(height),
    original_width_(width),
    height_(height),
    width_(width) {

    enable_skill(SD_SKILL_ROLL);
    enable_skill(SD_SKILL_SPINDASH);

    for(int i = 0; i < QUADRANT_MAX; ++i) {
        auto base_standing = std::make_shared<RayBox>(
            this,
            width,
            height + (Character::setting("VERTICAL_SENSOR_EXTENSION_LENGTH") * 2)

        );

        auto base_crouching = std::make_shared<RayBox>(
            this,
            (width * 0.75),
            (height * 0.75) + (Character::setting("VERTICAL_SENSOR_EXTENSION_LENGTH") * 2)
        );

        //TODO: Transform all the rays...

        kmMat3 rotation;
        kmMat3RotationZ(&rotation, kmDegreesToRadians(90));


        if(Quadrant(i) == QUADRANT_RIGHT_WALL) {
            base_standing->set_rotation(90);
            base_crouching->set_rotation(90);
        } else if(Quadrant(i) == QUADRANT_CEILING) {
            base_standing->set_rotation(180);
            base_crouching->set_rotation(180);
        } else if(Quadrant(i) == QUADRANT_LEFT_WALL) {
            base_standing->set_rotation(-90);
            base_crouching->set_rotation(-90);
        }

        standing_shape_[(Quadrant)i] = base_standing;
        crouching_shape_[(Quadrant)i] = base_crouching;
    }

    //Copy the standing shape to set the geom to default
    set_geom(standing_shape_[quadrant_]);
}

///Override from Object to prevent rotating the geom
void Character::set_rotation(kmScalar angle) {
    //angle = (angle < 0) ? 360.0 + angle : angle;
    rotation_ = angle;
}

void Character::set_quadrant(Quadrant quadrant) {
    quadrant_ = quadrant;

    if(size_ == CHARACTER_SIZE_CROUCHING) {
        set_geom(crouching_shape_[quadrant_]);
    } else {
        set_geom(standing_shape_[quadrant_]);
    }
}

void Character::set_size(CharacterSize size) {
    size_ = size;

    float quarter_height = height_ * 0.25;

    if(size_ == CHARACTER_SIZE_CROUCHING) {
        //Copy the crouching shape to set the geom to default
        set_geom(crouching_shape_[quadrant_]);
        set_position(position().x, position().y - (quarter_height / 2));
    } else {
        //Copy the standing shape to set the geom to default
        set_geom(standing_shape_[quadrant_]);
        set_position(position().x, position().y + (quarter_height / 2));
    }
}


SDuint sdCharacterCreate(SDuint world_id) {
    World* world = World::get(world_id);
    return world->new_character();
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


std::pair<Collision, bool> Character::find_nearest_collision_with_ray(
	const std::vector<Collision>& collisions,
	char ray,
    float& closest_distance) {

    const float FLOAT_MAX = std::numeric_limits<float>::max();

	bool found = false;
    kmScalar closest = FLOAT_MAX;
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
	

    closest_distance = closest;
    if(closest_distance > height_  / 2) {
        closest_distance = FLOAT_MAX;
    }

	return std::make_pair(final, found);
}

float calc_angle_from_up(const kmVec2& vec) {
	kmVec2 up, n;
	kmVec2Fill(&up, 0.0, 1.0);
	kmVec2Normalize(&n, &vec);

    float angle = kmRadiansToDegrees(atan2(vec.x * up.y - vec.y * up.x, vec.x * up.x + vec.y * up.y));

    if(angle < 0) {
        angle = 360 + angle;
    }

    return angle;
}

void Character::pre_prepare(float dt) {
    if(debug_break) {
        std::cout << "Breaking..." << std::endl;
    }

    if(x_axis_state_ == AXIS_STATE_NEGATIVE) {
        if(is_grounded()) {
            if(gsp_ > 0) {
                gsp_ -= deceleration_rate_ * dt;
            } else  if(gsp_ > -top_speed_) {
                gsp_ -= acceleration_rate_ * dt;
                if(fabs(gsp_) > top_speed_) {
                    gsp_ = -top_speed_;
                }
            }
        } else {
            if(velocity_.x > -top_speed_) {
                velocity_.x -= (acceleration_rate_ * 2.0 * dt);
                if(fabs(velocity_.x) > top_speed_) {
                    velocity_.x = -top_speed_;
                }
            }
        }
    } else if(x_axis_state_ == AXIS_STATE_POSITIVE) {
        if(is_grounded()) {
            if(gsp_ < 0) {
                gsp_ += deceleration_rate_ * dt;
            } else if(gsp_ < top_speed_) {
                gsp_ += acceleration_rate_ * dt;
                if(fabs(gsp_) > top_speed_) {
                    gsp_ = top_speed_;
                }
            }
        } else {
            if(velocity_.x < top_speed_) {
                velocity_.x += (acceleration_rate_ * 2.0 * dt);
                if(fabs(velocity_.x) > top_speed_) {
                    velocity_.x = top_speed_;
                }
            }
        }
    } else if(x_axis_state_ == AXIS_STATE_NEUTRAL){
        if(is_grounded()) {
            gsp_ -= std::min<float>(fabs(gsp_), friction_rate_ * dt) * sgn(gsp_);
            gsp_ += slope_rate_ * sin(kmDegreesToRadians(rotation_)) * dt;
        }
    }

    if(is_grounded()) {
        velocity_.x = gsp_ * cos(kmDegreesToRadians(rotation_));
        velocity_.y = gsp_ * -sin(kmDegreesToRadians(rotation_));
    }

    //Apply gravity if the character is attached to a world
    if(!is_grounded()) {
        //Air drag
        if(velocity_.y > 0 && velocity_.y < air_drag_max_y_ && fabs(velocity_.x) > air_drag_min_x_) {
            velocity_.x *= (air_drag_rate_ * dt);
        }

        if(world()) {
            //Gravity
            kmVec2 grv = world()->gravity();
            kmVec2Scale(&grv, &grv, dt);
            kmVec2Add(&velocity_, &velocity_, &grv);
        }
    }

    if(action_button_state_) {
        if(!last_action_button_state_ && is_grounded()) {
            velocity_.y = jump_rate_;
            animation_state_ = ANIMATION_STATE_JUMPING;
            ground_state_ = GROUND_STATE_IN_THE_AIR;
        }
    } else {
        if(last_action_button_state_ && velocity_.y > jump_cut_off_ && !is_grounded()) {
            velocity_.y = jump_cut_off_;
        }
    }

    if(fabs(velocity_.y) > top_y_speed_) {
        velocity_.y = top_y_speed_ * sgn(velocity_.y);
    }
}

bool Character::respond_to(const std::vector<Collision>& collisions) {
    float a_dist, b_dist, l_dist, r_dist, e_dist;
    std::pair<Collision, bool> a = find_nearest_collision_with_ray(collisions, 'A', a_dist);
    std::pair<Collision, bool> b = find_nearest_collision_with_ray(collisions, 'B', b_dist);
    std::pair<Collision, bool> l = find_nearest_collision_with_ray(collisions, 'L', l_dist);
    std::pair<Collision, bool> r = find_nearest_collision_with_ray(collisions, 'R', r_dist);
    std::pair<Collision, bool> e = find_nearest_collision_with_ray(collisions, 'E', e_dist);

	//Store the original position, we need this to work out
	//if anything changed
	kmVec2 original_position;
	kmVec2Assign(&original_position, &position());
	
    bool was_grounded = is_grounded();

    const float FLOAT_MAX = std::numeric_limits<float>::max();

    bool a_collided_within_height = a_dist < FLOAT_MAX;
    bool b_collided_within_height = b_dist < FLOAT_MAX;
    bool a_collided = a.second;
    bool b_collided = b.second;
    bool e_collided = e.second;

    bool a_b_respond = false;

    if(ground_state_ == GROUND_STATE_IN_THE_AIR && velocity().y <= 0) {
        if(a_collided_within_height && b_collided_within_height) {
            //Both collided
            ground_state_ = GROUND_STATE_ON_THE_GROUND;
            a_b_respond = true;
        } else if(a_collided_within_height) {
            if(e_collided || b_collided) {
                //Both A and E have collisions
                ground_state_ = GROUND_STATE_ON_THE_GROUND;
            } else {
                ground_state_ = GROUND_STATE_BALANCING_RIGHT;
            }
            a_b_respond = true;
        } else if(b_collided_within_height) {
            if(e_collided || a_collided) {
                //Both B and E have collisions
                ground_state_ = GROUND_STATE_ON_THE_GROUND;
            } else {
                ground_state_ = GROUND_STATE_BALANCING_LEFT;
            }
            a_b_respond = true;
        }
    } else if(ground_state_ != GROUND_STATE_IN_THE_AIR) {
        if(a_collided && b_collided) {
            ground_state_ = GROUND_STATE_ON_THE_GROUND;
            a_b_respond = true;
        } else if(a_collided) {
            if(e_collided) {
                ground_state_ = GROUND_STATE_ON_THE_GROUND;
            } else {
                ground_state_ = GROUND_STATE_BALANCING_RIGHT;
            }
            a_b_respond = true;
        } else if(b_collided) {
            if(e_collided) {
                ground_state_ = GROUND_STATE_ON_THE_GROUND;
            } else {
                ground_state_ = GROUND_STATE_BALANCING_LEFT;
            }
            a_b_respond = true;
        } else {
            ground_state_ = GROUND_STATE_IN_THE_AIR;
        }
    } else {
        ground_state_ = GROUND_STATE_IN_THE_AIR;
    }

    if(debug_break && !is_grounded()) {
        std::cout << "Breaking" << std::endl;
    }

    if(a_b_respond) {
        kmVec2 new_location;
        kmVec2Assign(&new_location, &original_position);
        float new_angle;
        if(a_dist <= b_dist) {
            float new_y = a.first.point.y + (height_ / 2.0);
            if(is_grounded()) {
                new_location.y = new_y;
            } else if(new_location.y < new_y) {
                //If we are in the air, only set the new height if we are less than it
                new_location.y = new_y;
            }
            new_angle = calc_angle_from_up(a.first.b_normal);
        } else {
            float new_y = b.first.point.y + (height_ / 2.0);
            if(is_grounded()) {
                new_location.y = new_y;
            } else if(new_location.y < new_y) {
                new_location.y = new_y;
            }
            new_angle = calc_angle_from_up(b.first.b_normal);
        }

        //Handle quadrant switching
        if(new_angle < 45 || new_angle > 315) {
            set_quadrant(QUADRANT_FLOOR);
        } else if(new_angle > 45 && new_angle < 135) {
            set_quadrant(QUADRANT_LEFT_WALL);
        } else if(new_angle > 135 && new_angle < 225) {
            set_quadrant(QUADRANT_CEILING);
        } else if(new_angle > 225 && new_angle < 315) {
            set_quadrant(QUADRANT_RIGHT_WALL);
        }


        set_position(new_location.x, new_location.y);
        set_rotation(new_angle);

        if(!was_grounded && is_grounded()) {
            //We just hit the ground
            float test_angle = (new_angle > 90) ? fabs(new_angle - 360.0) : new_angle;
            if(test_angle < 22.5) {
                gsp_ = velocity().x;
            } else if(test_angle < 45.0) {
                if(velocity().x > fabs(velocity().y)) {
                    gsp_ = velocity().x;
                } else {
                    gsp_ = velocity().y * 0.5 * -sgn(cos(kmDegreesToRadians(new_angle)));
                }
            } else {
                if(velocity().x > fabs(velocity().y)) {
                    gsp_ = velocity().x;
                } else {
                    gsp_ = velocity().y * -sgn(cos(kmDegreesToRadians(new_angle)));
                }
            }
        }
	} 

	//If the position changed, re-run the collision loop
	return !kmVec2AreEqual(&original_position, &position());
}

void Character::update_finished(float dt) {
    //After collisions have been processed

    //If we are going too slow, set the state to in the air, even if there were floor
    //collisions
    if(gsp_ < MIN_SPEED_TO_AVOID_FAILING_IN_MPS && quadrant_ != QUADRANT_FLOOR) {
        set_quadrant(QUADRANT_FLOOR);
        set_ground_state(GROUND_STATE_IN_THE_AIR);
        gsp_ = 0.0;
    }


    if(is_grounded()) {
        if(fabs(gsp_) == 0) {
            if(ground_state_ == GROUND_STATE_BALANCING_LEFT || ground_state_ == GROUND_STATE_BALANCING_RIGHT) {
                animation_state_ = ANIMATION_STATE_BALANCING;
            } else {
                animation_state_ = ANIMATION_STATE_STANDING;
            }
        } else if(fabs(gsp_) < ANIMATION_RUNNING_MIN_X_SPEED) {
            animation_state_ = ANIMATION_STATE_WALKING;
        } else if(fabs(gsp_) < ANIMATION_DASHING_MIN_X_SPEED) {
            animation_state_ = ANIMATION_STATE_RUNNING;
        } else if(fabs(gsp_) >= ANIMATION_DASHING_MIN_X_SPEED) {
            animation_state_ = ANIMATION_STATE_DASHING;
        }
    }

    if(ground_state_ == GROUND_STATE_ON_THE_GROUND && fabs(gsp_) < MIN_ROLLING_SPEED && quadrant_ == QUADRANT_FLOOR && x_axis_state_ == AXIS_STATE_NEUTRAL) {
        if(y_axis_state_ == AXIS_STATE_NEGATIVE) {
            animation_state_ = ANIMATION_STATE_LOOKING_DOWN;
        } else if(y_axis_state_ == AXIS_STATE_POSITIVE) {
            animation_state_ = ANIMATION_STATE_LOOKING_UP;
        }
    }

    if(facing_ == DIRECTION_LEFT) {
        if((gsp_ > 0.0 && x_axis_state_ == AXIS_STATE_POSITIVE) || (ground_state_ == GROUND_STATE_BALANCING_RIGHT && x_axis_state_ == AXIS_STATE_NEUTRAL)) {
            facing_ = DIRECTION_RIGHT;
        }
    } else {
        if((gsp_ < 0.0 && x_axis_state_ == AXIS_STATE_NEGATIVE) || (ground_state_ == GROUND_STATE_BALANCING_LEFT && x_axis_state_ == AXIS_STATE_NEUTRAL)) {
            facing_ = DIRECTION_LEFT;
        }
    }

    //Reset everything
    last_x_axis_state_ = x_axis_state_;
    last_y_axis_state_ = y_axis_state_;
    last_action_button_state_ = action_button_state_;

    x_axis_state_ = AXIS_STATE_NEUTRAL;
    y_axis_state_ = AXIS_STATE_NEUTRAL;
    action_button_state_ = false;

}

//================================================

Character* Character::get(SDuint object_id) {
    Object* obj = Object::get(object_id);
    Character* c = dynamic_cast<Character*>(obj);
    return c;
}

