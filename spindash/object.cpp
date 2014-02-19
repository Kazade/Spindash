#include <kazbase/logging.h>
#include "object.h"
#include "world.h"

static SDuint generate_id() {
    static SDuint id = 0;    
    return ++id;
}

Object::Object(World* world):
    id_(generate_id()),
    world_(world),
    rotation_(0.0f) {
    
    kmVec2Fill(&position_, 0.0f, 0.0f);
    kmVec2Fill(&velocity_, 0.0f, 0.0f);
    kmVec2Fill(&acceleration_, 0.0f, 0.0f);
    kmVec2Assign(&last_safe_position_, &position_);

    World::all_objects()[id()] = this; //Register this object globally
}

Object::~Object() {

}

void Object::prepare(float dt) {
    pre_prepare(dt);
    
    velocity_.x += acceleration_.x;
    velocity_.y += acceleration_.y;
    
    post_prepare(dt);
}

void Object::update(float dt) {
    pre_update(dt);

    set_position(
        position().x + velocity_.x,
        position().y + velocity_.y
    );

    post_update(dt);
}

Object* Object::get(SDuint object_id) {
    auto it = World::all_objects().find(object_id);
    
    if(it == World::all_objects().end()) {
        throw std::logic_error("Invalid object");
    }
    
    return (*it).second;
}

bool Object::exists(SDuint object_id) {
    return World::all_objects().find(object_id) != World::all_objects().end();
}

void Object::set_position(kmScalar x, kmScalar y) {
    position_.x = x;
    position_.y = y;
    
    geom().set_position(x, y);
}

void Object::set_rotation(kmScalar angle) {
	//angle = (angle < 0) ? 360.0 + angle : angle;
	
    rotation_ = angle;
    geom().set_rotation(angle);
}

void Object::set_velocity(kmScalar x, kmScalar y) {
    velocity_.x = x;
    velocity_.y = y;
}

void Object::set_acceleration(kmScalar x, kmScalar y) {
    acceleration_.x = x;
    acceleration_.y = y;
}

Object* Object::get_other_object_from_collision(Collision& c) {
    Object* other = nullptr;

    if(c.object_a == &this->geom()) {
        if(c.object_b) {
            other = c.object_b->owner();
        }
    } else {
        if(c.object_a) {
            other = c.object_a->owner();
        }    
    }
    
    return other;
}

//========================================================
