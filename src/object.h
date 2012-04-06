#ifndef KP_OBJECT_H
#define KP_OBJECT_H

#include <cstdint>
#include <map>
#include <tr1/memory>

#include "typedefs.h"
#include "kazmath/vec2.h"
#include "collision/collision_primitive.h"

typedef uint32_t ObjectID;

class World;

class Object {
private:
    SDuint id_;
    
    kmVec2 position_;
    kmVec2 speed_;
    kmVec2 acceleration_;
    float rotation_;

    static std::map<ObjectID, Object*> objects_;

    World* world_;       
    
    virtual void pre_prepare(float dt) {}
    virtual void post_prepare(float dt) {}
    virtual void pre_update(float dt) {}
    virtual void post_update(float dt) {}
    
    CollisionPrimitive::ptr shape_;
    
public:
    typedef std::tr1::shared_ptr<Object> ptr;

    Object(World* world, CollisionPrimitive::ptr shape);
    virtual ~Object();

    static void register_object(Object* obj);
    static void unregister_object(Object* obj);
    static Object* by_id(SDuint object_id);
    static bool exists(SDuint object_id);
    
    void set_position(float x, float y);
    void set_speed(float x, float y);
    void set_acceleration(float x, float y);

    const kmVec2& speed() const { return speed_; }
    const kmVec2& position() const { return position_; }
    const kmVec2& acceleration() const { return acceleration_; }
    float rotation() const { return rotation_; }
    
    void prepare(float dt);
    void update(float dt);
    
    virtual void respond_to(std::vector<Collision>& collisions) {}
    
    SDuint id() const { return id_; }
    
    World* world() { return world_; }
    
    CollisionPrimitive& geom() { return *shape_; }
};

#endif
