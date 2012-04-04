
#include "character.h"
#include "world.h"

const float WORLD_SCALE = 1.0f / 40.0f;

SDuint sdCharacterCreate(SDuint world_id) {
    World* world = get_world_by_id(world_id);
    return world->new_character();
}

void Character::pre_update(float dt) {
    const float ACC = 0.046875f * WORLD_SCALE;
    const float FRC = ACC;
    const float DEC = 0.5f * WORLD_SCALE;
    
    float x_acc = 0.0f;
    
    acc_applied_this_frame_ = false;
    
    //Done this way so pressing left and right at the same time, does nothing
    if(moving_left_ && moving_right_) return;
    
    if(moving_left_) {
        if(speed().x <= 0.0f) {
            x_acc = -ACC;
            acc_applied_this_frame_ = true;
        } else {
            x_acc = -DEC;
        }
    } else if(moving_right_) {
        if(speed().x >= 0.0f) {
            x_acc = ACC;
            acc_applied_this_frame_ = true;
        } else {
            x_acc = DEC;
        }
    } else {
        float frc = std::min((float)fabs(speed().x), FRC);
        if(speed().x > 0.0f) x_acc = -frc;
        if(speed().x < 0.0f) x_acc = frc;    
    }
    
    set_acceleration(x_acc * 60.f, 0.0f);
}

void Character::post_speed_update(float dt) {
    const float TOP = 6.0f * WORLD_SCALE;
    
    if(moving_left_ && moving_right_) return;

    if(moving_left_) {
        if(speed().x < -TOP && acc_applied_this_frame_) {
            set_speed(-TOP, speed().y);
        }
    } else if (moving_right_) {
        if(speed().x > TOP && acc_applied_this_frame_) {
            set_speed(TOP, speed().y);
        }
    }
    
}
    
//================================================

static Character* get_character(SDuint object_id) {
    Object* obj = Object::by_id(object_id);
    Character* c = dynamic_cast<Character*>(obj);
    assert(c && "Not a character");
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

SDbool sdCharacterIsGrounded(SDuint character) {
    Character* c = get_character(character);
    return c->is_grounded();
}

