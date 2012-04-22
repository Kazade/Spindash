#include "object.h"
#include "world.h"
#include "logging/logging.h"

static std::map<ObjectID, Object*> objects_;

static SDuint generate_id() {
    static SDuint id = 0;    
    return ++id;
}

Object::Object(World* world, CollisionPrimitive::ptr shape):
    id_(generate_id()),
    world_(world),
    rotation_(0.0f),
    shape_(shape) {
    
    register_object(this);
    
    kmVec2Fill(&position_, 0.0f, 0.0f);
    kmVec2Fill(&speed_, 0.0f, 0.0f);
    kmVec2Fill(&acceleration_, 0.0f, 0.0f);
    kmVec2Assign(&last_safe_position_, &position_);
}

Object::~Object() {
    unregister_object(this);
}

void Object::prepare(float dt) {
    pre_prepare(dt);
    
    speed_.x += acceleration_.x * dt;
    speed_.y += acceleration_.y * dt;
    
    post_prepare(dt);
}

void Object::update(float dt) {
    if(pre_update(dt)) {
        set_position(position().x + speed_.x, position().y + speed_.y);
    }

    post_update(dt);
}

Object* Object::by_id(SDuint object_id) {
    std::map<ObjectID, Object*>::iterator it = objects_.find(object_id);
    
    if(it == objects_.end()) throw std::logic_error("Invalid object");
    
    return (*it).second;
}

void Object::register_object(Object* obj) {
    if(!obj) return;
    
    objects_[obj->id()] = obj;
}

void Object::unregister_object(Object* obj) {
    if(!obj) return;
    
    std::map<uint32_t, Object*>::iterator it = objects_.find(obj->id());
    if(it != objects_.end()) {
        objects_.erase(it);
    }
}

bool Object::exists(SDuint object_id) {
    return objects_.find(object_id) != objects_.end();
}

void Object::set_position(kmScalar x, kmScalar y) {
    position_.x = x;
    position_.y = y;
    
    geom().set_position(x, y);
}

void Object::set_rotation(kmScalar angle) {
	angle = (angle < 0) ? 360.0 + angle : angle;
	
    rotation_ = angle;
    geom().set_rotation(angle);
}

void Object::set_speed(kmScalar x, kmScalar y) {
    speed_.x = x;
    speed_.y = y;
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

void sdObjectDestroy(SDuint entity) {
    Object* obj = Object::by_id(entity);
    if(!obj) {
        L_WARN("sdObjectDestroy: No such object");
        return;
    }
    
    obj->world()->destroy_object(entity);
}

void sdObjectSetPosition(SDuint object, SDdouble x, SDdouble y) {
    Object* obj = Object::by_id(object);        
    obj->set_position(x, y);        
}

kmVec2 sdObjectGetPosition(SDuint object) {
    Object* obj = Object::by_id(object);        
    return obj->position();
}

SDdouble sdObjectGetPositionX(SDuint object) {
    Object* obj = Object::by_id(object);        
    return obj->position().x;
}

SDdouble sdObjectGetPositionY(SDuint object) {
    Object* obj = Object::by_id(object);        
    return obj->position().y;
}

SDdouble sdObjectGetSpeedX(SDuint object) {
    Object* obj = Object::by_id(object);
    return obj->speed().x;
}

SDdouble sdObjectGetSpeedY(SDuint object) {
    Object* obj = Object::by_id(object);
    return obj->speed().y;
}

void sdObjectSetSpeedX(SDuint object, SDdouble x) {
    Object* obj = Object::by_id(object);
    
    obj->set_speed(x, obj->speed().y);
}

void sdObjectSetSpeedY(SDuint object, SDdouble y) {
    Object* obj = Object::by_id(object);
    
    obj->set_speed(obj->speed().x, y);
}

SDdouble sdObjectGetRotation(SDuint object) {
    Object* obj = Object::by_id(object);
    return obj->rotation();
}

