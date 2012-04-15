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

void Character::respond_to(const std::vector<Collision>& collisions) {
    //Find the closest ray
    //FIXME: Find which ray is closest and respond to only that set
    // e.g. if A is the closest collision, do the A/B stuff. If L is the
    // closest, do the L stuff
    std::map<char, bool> hitmask;
    std::map<char, Collision> collision_map;
    std::map<char, float> distance; //The distance along the ray the hit occurred

    RayBox* ray_box = dynamic_cast<RayBox*>(&geom());
    
    for(const Collision& c: collisions) {        
        char ray = (c.object_a == &this->geom()) ? c.a_ray : c.b_ray;
    
        kmRay2& r = ray_box->ray(ray);
        kmVec2 diff;

        hitmask[ray] = true;
        kmVec2Subtract(&diff, &c.point, &r.start);
        distance[ray] = kmVec2Length(&diff);
        collision_map[ray] = c;
    }

    if(hitmask['A'] || hitmask['B']) {
        if(!is_grounded_ && kmVec2Dot(&speed(), &ray_box->ray('A').dir) < 0.0f) {
            //std::cout << "Ignoring collision as we are moving away from the surface" << std::endl;
        } else {
            kmVec2 to_move;
            kmVec2Fill(&to_move, 0, 0);
            
            //FIXME: should check both rays!
                    
            Object* other = nullptr;
            if(hitmask['A']) {
                other = get_other_object_from_collision(collision_map['A']);
            } else if (hitmask['B']) {
                other = get_other_object_from_collision(collision_map['B']);
            }

            //Only mark if grounded if the object doesn't have the NOT_GROUND flag                        
            is_grounded_ = (other) ? !other->has_collision_flag(NOT_GROUND) : true;

            kmVec2 up;
            kmVec2Fill(&up, 0.0f, 1.0f);

            Collision a, b;
            bool same_collision = false;
            
            if(hitmask['A'] && hitmask['B']) {
                a = collision_map['A'];
                b = collision_map['B'];
            } else if(hitmask['A']) {
                a = collision_map['A'];
                b = collision_map['A'];        
                same_collision = true;
            } else {
                a = collision_map['B'];
                b = collision_map['B'];
                
                same_collision = true;
            }
            
            /* Inventive logic follows:
                In the Sonic game, there were 4 modes if sonic hit a 45.0f
                angle then we'd go from "floor mode" to "right wall" mode.
                So, all the physics were only based around angles between -45
                and 45. This meant that if two ray hit the floor you could just
                pick the angle of the highest (on the Y-axis) and it would work.
                
                We don't have 4 modes, and we can do better. Here's what we do:
                
                1. Take the two collision points, find the vector between them
                2. Calculate a new intersection point which is A + (Vec * 0.5)
                3. Calculate a new normal by averaging the two other normals
                4. Move the character by normal * half_height - fabs(length(position - new_intersection))
                5. Set the angle based on the new normal
                
                This should result in smooth movement over angles
            */
                

            kmVec2 intersection_a, intersection_b;
            
            kmVec2Assign(&intersection_a, &a.point);
            kmVec2Assign(&intersection_b, &b.point);
            
            kmVec2 diff_vec, diff_normalized, half_diff, new_intersection, new_normal;

            //Get the normal for the other object, but from the hit with ray A (confusing I know :/ )
            CollisionPrimitive* A_object_a = a.object_a;
            kmVec2 a_normal = (A_object_a == &this->geom()) ? a.b_normal : a.a_normal;
            
            CollisionPrimitive* B_object_a = b.object_a;
            kmVec2 b_normal = (B_object_a == &this->geom()) ? b.b_normal : b.a_normal;            
            
            kmVec2 pos_diff;
            if(!same_collision) {
                float diff_length;
                kmVec2Subtract(&diff_vec, &intersection_b, &intersection_a);
                diff_length = kmVec2Length(&diff_vec);
                kmVec2Normalize(&diff_normalized, &diff_vec);
                kmVec2Scale(&half_diff, &diff_normalized, diff_length * 0.5f);        
                kmVec2Add(&new_intersection, &intersection_a, &half_diff);        

                kmVec2Add(&new_normal, &a_normal, &b_normal);
                kmVec2Normalize(&new_normal, &new_normal); //Average the normals            
                
                kmVec2Subtract(&pos_diff, &new_intersection, &position());
            } else {
                kmVec2Assign(&new_intersection, &intersection_a);
                kmVec2Assign(&new_normal, &a_normal);
                
                kmRay2& collision_ray = (A_object_a == &this->geom()) ? ray_box->ray(a.a_ray) : ray_box->ray(a.b_ray);
                
                kmVec2Subtract(&pos_diff, &intersection_a, &collision_ray.start);
            }

            float to_move_length = (ray_box->height() * 0.5f) - fabs(kmVec2Length(&pos_diff));
            kmVec2Scale(&to_move, &new_normal, to_move_length);

            float dot = kmVec2Dot(&new_normal, &up);
            float angle = kmRadiansToDegrees(acosf(dot));
            angle = -angle;
            angle = (angle < 0) ? 360.0f + angle : angle;
            set_rotation(angle);
        
            if(!isnan(to_move.x) &&
               !isnan(to_move.y)) {
                
                kmVec2 pos;
                kmVec2Assign(&pos, &position());
                kmVec2Add(&pos, &pos, &to_move);
                
                set_position(pos.x, pos.y);            
            } else {
                std::cout << "To-move vector is NAN - FIX THIS" << std::endl;
            }
        }
    

        
        

    } 
    if (hitmask['L'] || hitmask['R']) {
        kmRay2& ray_hit = (hitmask['L']) ? ray_box->ray('L') : ray_box->ray('R');
        Collision& hitpoint = (hitmask['L']) ? collision_map['L'] : collision_map['R'];
        kmVec2& hitpoint_normal = (hitpoint.object_a == &this->geom()) ? hitpoint.b_normal : hitpoint.a_normal;
        
        
        kmVec2 diff;
        kmVec2Subtract(&diff, &hitpoint.point, &ray_hit.start);
        float lr_dist = kmVec2Length(&diff);
        
        kmVec2 to_move;
        kmVec2Scale(&to_move, &hitpoint_normal, (kmVec2Length(&ray_hit.dir) - fabs(lr_dist)) * 1.01f);

        kmVec2 pos;
        kmVec2Assign(&pos, &position());
        kmVec2Add(&pos, &pos, &to_move);
        set_position(pos.x, pos.y);

        if(is_grounded_ && hitmask['L'] && gsp_ < 0.0f) gsp_ = 0.0f;
        if(is_grounded_ && hitmask['R'] && gsp_ > 0.0f) gsp_ = 0.0f;
        if(!is_grounded_) {
            //This is a little more complicated... basically, we need to reset
            // the X/Y speed if we are in the air, and we hit the left ray
            // and we are heading towards the wall which we collided with
            //so... we take the dot-product between the speed
            //and the normal. If the resulting angle is greater than 90 degrees
            //(e.g. the dot product is negative) then we were heading towards
            //the surface when we hit it.
            kmVec2 speed_vec;
            kmVec2Assign(&speed_vec, &speed());
            float speed_length = kmVec2Length(&speed_vec);
            kmVec2Normalize(&speed_vec, &speed());
            if(kmVec2Dot(&speed_vec, &hitpoint_normal) < 0.0f) {
                //Now, we need to subtract the normal from the speed, but
                //scaled to the length of the speed
                
                kmVec2 new_speed;                
                //Add the normalized speed_vec and the normal
                kmVec2Add(&new_speed, &speed_vec, &hitpoint_normal);
                
                //Rescale up so we don't lose the energy
                kmVec2Scale(&new_speed, &new_speed, speed_length);
                
                /*
                    Final bit of hacking, we never want the speed.x or speed.y
                    to change sign (otherwise we bounce). So, if the original speed had a positive X
                    and now it's negative. We set it to zero
                */
                if((speed_vec.x < 0.0f && new_speed.x > 0.0f) ||
                   (speed_vec.x > 0.0f && new_speed.x < 0.0f)) {
                   new_speed.x = 0.0f;
                }

                if((speed_vec.y < 0.0f && new_speed.y > 0.0f) ||
                   (speed_vec.y > 0.0f && new_speed.y < 0.0f)) {
                   new_speed.y = 0.0f;
                }
                
                set_speed(new_speed.x, new_speed.y);
            }
        }
    }
    
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
