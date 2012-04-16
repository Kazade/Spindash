#include <iostream>
#include <cassert>

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

bool Character::respond_to(const std::vector<Collision>& collisions) {
    //First basic collision response
    RayBox* ray_box = dynamic_cast<RayBox*>(&geom());
    bool position_changed = false;
    
    for(Collision c: collisions) {
        char r = (c.object_a == &geom()) ? c.a_ray : c.b_ray;
        kmRay2& ray = ray_box->ray(r);
        kmVec2 other_normal;
        kmVec2Assign(&other_normal, ((c.object_a == &geom()) ? &c.b_normal : &c.a_normal));
        
        if(r == 'A' || r == 'B') {
            is_grounded_ = true;
        }
            
        //1. move the position out from the collision
        float ray_length;
        ray_length = kmVec2Length(&ray.dir);
        
        float dist_to_intersection;
        kmVec2 vec_to_intersection;
        kmVec2Subtract(&vec_to_intersection, &c.point, &ray.start);
        dist_to_intersection = kmVec2Length(&vec_to_intersection);

        //Only respond if we need to
        if(dist_to_intersection < ray_length) {
            if(kmVec2Dot(&other_normal, &speed()) < 0.0f) {
                kmVec2 normalized_speed;
                kmVec2Normalize(&normalized_speed, &speed());

                kmVec2 to_move;
                kmVec2Scale(&to_move, &normalized_speed, fabs(ray_length - dist_to_intersection));

                kmVec2 new_pos;
                kmVec2Subtract(&new_pos, &position(), &to_move);
                set_position(new_pos.x, new_pos.y);
                
                position_changed = true;
            }
    /*
            //2. Find the angle of rotation and rotate position around the intersection
            
            float angle_between_normal_and_ray;

            
            angle_between_normal_and_ray = acosf(kmVec2Dot()*/
        }
    }
    return position_changed;
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
    
    float sin_rot = sinf(rotation());
    float slp = SLP;
    
    static bool jump_pressed_last_time = false;
    static bool grounded_last_frame = false;
    
    if(!grounded_last_frame && is_grounded_) {
        std::cout << "Reorienting" << std::endl;
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
        //set_rotation(0);        
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
        speed_.x = gsp_ * cosf(kmDegreesToRadians(rotation()));
        speed_.y = gsp_ * -sinf(kmDegreesToRadians(rotation()));    
    } else {
        //Air drag effect
        /*
        if(speed_.y > 0 && speed_.y < (4.0f * WORLD_SCALE)) {
            speed_.x = speed_.x - ((speed_.x / (0.125f * WORLD_SCALE)) / 256.0f);
        }*/

        speed_.y -= GRV * dt;        
    }
    
    std::cout << "Grounded: " << is_grounded_ << std::endl;
    
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
