#ifndef KP_OBJECT_H
#define KP_OBJECT_H

#include <cstdint>
#include <map>
#include <tr1/memory>

#include "typedefs.h"
#include "kazmath/vec2.h"
#include "collision/collision_primitive.h"

typedef uint32_t ObjectID;

enum CollisionFlag {
    IGNORE_DOWNWARD_COLLISION = 1,
    IGNORE_UPWARD_COLLISION = 2,
    NOT_GROUND = 4
};

class World;

class Object {
protected:
    SDuint id_;
    
    kmVec2 position_;
    kmVec2 speed_;
    kmVec2 acceleration_;
    float rotation_;

private:
    static std::map<ObjectID, Object*> objects_;

    World* world_;       
    
    virtual void pre_prepare(float dt) {}
    virtual void post_prepare(float dt) {}
    virtual bool pre_update(float dt) { return true; }
    virtual void post_update(float dt) {}
    
    CollisionPrimitive::ptr shape_;
    
    uint32_t collision_flags_;
public:
    typedef std::tr1::shared_ptr<Object> ptr;

    Object(World* world, CollisionPrimitive::ptr shape);
    virtual ~Object();

    static void register_object(Object* obj);
    static void unregister_object(Object* obj);
    static Object* by_id(SDuint object_id);
    static bool exists(SDuint object_id);
    
    void set_position(float x, float y);
    virtual void set_speed(float x, float y);
    void set_acceleration(float x, float y);
    void set_rotation(float degrees);

    const kmVec2& speed() const { return speed_; }
    const kmVec2& position() const { return position_; }
    const kmVec2& acceleration() const { return acceleration_; }
    float rotation() const { return rotation_; }
    
    virtual void prepare(float dt);
    virtual void update(float dt);
    virtual void update_finished(float dt) {}
    
    virtual bool respond_to(const std::vector<Collision>& collisions) { return true; }
    
    SDuint id() const { return id_; }
    
    World* world() { return world_; }
    
    CollisionPrimitive& geom() { return *shape_; }
    
    void set_collision_flag(CollisionFlag flag) { collision_flags_ |= flag; }
    bool has_collision_flag(CollisionFlag flag) const { return (collision_flags_ & flag) == flag; }
    
    Object* get_other_object_from_collision(Collision& c);
};

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0)) || 1;
}

#endif
