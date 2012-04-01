#include "object.h"

std::map<ObjectID, Object*> Object::objects_;

static KPuint generate_id() {
    static KPuint id = 0;    
    return ++id;
}

Object::Object(World* world):
    id_(generate_id()) {
    
    register_object(this);
}

Object::~Object() {
    unregister_object(this);
}

void Object::update(float dt) {
    position_.x += speed_.x * dt;
    position_.y += speed_.y * dt;
}

Object* Object::get_object_by_id(KPuint object_id) {
    std::map<ObjectID, Object*>::iterator it = objects_.find(object_id);
    
    if(it == objects_.end()) throw std::logic_error("Invalid object");
    
    return (*it).second;
}

void Object::register_object(const Object* obj) {
    objects_[obj->id()] = objects_;
}

void Object::unregister_object(const Object* obj) {
    objects_.erase(obj->id());
}

//========================================================

void kpObjectSetPosition(KPuint object, float x, float y) {
    Object* obj = Object::get(object);        
    obj->set_position(x, y);        
}

KPfloat kpObjectGetSpeedX(KPuint object) {
    Object* obj = Object::get(object);
    return obj->speed().x;
}

KPfloat kpObjectGetSpeedY(KPuint object) {
    Object* obj = Object::get(object);
    return obj->speed().y;
}

void kpObjectSetSpeedX(KPuint object, KPfloat x) {
    Object* obj = Object::get(object);
    
    obj->set_speed(x, obj->speed().y);
}

void kpObjectSetSpeedY(KPuint object, KPfloat y) {
    Object* obj = Object::get(object);
    
    obj->set_speed(obj->speed().x, y);
}
