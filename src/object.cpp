#include "object.h"
#include "world.h"
#include "logging/logging.h"

std::map<ObjectID, Object*> Object::objects_;

static SDuint generate_id() {
    static SDuint id = 0;    
    return ++id;
}

Object::Object(World* world, CollisionPrimitive::ptr shape):
    id_(generate_id()),
    rotation_(0.0f),
    shape_(shape) {
    
    register_object(this);
    
    kmVec2Fill(&position_, 0.0f, 0.0f);
    kmVec2Fill(&speed_, 0.0f, 0.0f);
    kmVec2Fill(&acceleration_, 0.0f, 0.0f);
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
    pre_update(dt);
    
    position_.x += speed_.x;
    position_.y += speed_.y;
    
    post_update(dt);
}

Object* Object::by_id(SDuint object_id) {
    std::map<ObjectID, Object*>::iterator it = objects_.find(object_id);
    
    if(it == objects_.end()) throw std::logic_error("Invalid object");
    
    return (*it).second;
}

void Object::register_object(Object* obj) {
    objects_[obj->id()] = obj;
}

void Object::unregister_object(Object* obj) {
    objects_.erase(obj->id());
}

bool Object::exists(SDuint object_id) {
    return objects_.find(object_id) != objects_.end();
}

void Object::set_position(float x, float y) {
    position_.x = x;
    position_.y = y;
}

void Object::set_speed(float x, float y) {
    speed_.x = x;
    speed_.y = y;
}

void Object::set_acceleration(float x, float y) {
    acceleration_.x = x;
    acceleration_.y = y;
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

void sdObjectSetPosition(SDuint object, float x, float y) {
    Object* obj = Object::by_id(object);        
    obj->set_position(x, y);        
}

kmVec2 sdObjectGetPosition(SDuint object) {
    Object* obj = Object::by_id(object);        
    return obj->position();
}

SDfloat sdObjectGetPositionX(SDuint object) {
    Object* obj = Object::by_id(object);        
    return obj->position().x;
}

SDfloat sdObjectGetPositionY(SDuint object) {
    Object* obj = Object::by_id(object);        
    return obj->position().y;
}

SDfloat sdObjectGetSpeedX(SDuint object) {
    Object* obj = Object::by_id(object);
    return obj->speed().x;
}

SDfloat sdObjectGetSpeedY(SDuint object) {
    Object* obj = Object::by_id(object);
    return obj->speed().y;
}

void sdObjectSetSpeedX(SDuint object, SDfloat x) {
    Object* obj = Object::by_id(object);
    
    obj->set_speed(x, obj->speed().y);
}

void sdObjectSetSpeedY(SDuint object, SDfloat y) {
    Object* obj = Object::by_id(object);
    
    obj->set_speed(obj->speed().x, y);
}

SDfloat sdObjectGetRotation(SDuint object) {
    Object* obj = Object::by_id(object);
    return obj->rotation();
}

