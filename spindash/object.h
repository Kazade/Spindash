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
    kmVec2 velocity_;
    kmVec2 acceleration_;
    kmScalar rotation_;

    kmVec2 last_safe_position_;

    bool is_fixed_ = false;

private:
    World* world_;       
    
    virtual void pre_prepare(float dt) {}
    virtual void post_prepare(float dt) {}
    virtual bool pre_update(float dt) { return true; }
    virtual void post_update(float dt) {}
    
    CollisionPrimitive::ptr shape_;
    
    uint32_t collision_flags_;

    SDGeometryHandle handle_ = 0;

protected:
    //============== NEW STUFF =============

    void set_geom(CollisionPrimitive::ptr shape) {
        shape_ = shape;
    }

    //======================================
public:
    typedef std::tr1::shared_ptr<Object> ptr;

    Object(World* world);
    virtual ~Object();

    static void register_object(Object* obj);
    static void unregister_object(Object* obj);
    static Object* get(SDuint object_id);
    static bool exists(SDuint object_id);
    
    void store_safe_position() {
        kmVec2Assign(&last_safe_position_, &position_);
    }
    void revert_to_safe_position() {
        kmVec2Assign(&position_, &last_safe_position_);
    }
    
    void set_position(kmScalar x, kmScalar y);
    virtual void set_velocity(kmScalar x, kmScalar y);
    void set_acceleration(kmScalar x, kmScalar y);
    virtual void set_rotation(kmScalar degrees);
    void set_fixed(kmBool value);

    const kmVec2& position() const { return position_; }
    const kmVec2& velocity() const { return velocity_; }
    const kmVec2& acceleration() const { return acceleration_; }

    float rotation() const { return rotation_; }

    virtual void prepare(float dt);
    virtual void update(float dt);
    virtual void update_finished(float dt) {}    
    virtual bool respond_to(const std::vector<Collision>& collisions) { return true; }
    
    SDuint id() const { return id_; }
    
    World* world() { return world_; }
    
    CollisionPrimitive& geom() { return *shape_; }
    const CollisionPrimitive& geom() const { return *shape_; }
    
    void set_collision_flag(CollisionFlag flag) { collision_flags_ |= flag; }
    bool has_collision_flag(CollisionFlag flag) const { return (collision_flags_ & flag) == flag; }
    
    Object* get_other_object_from_collision(Collision& c);

    void set_geometry_handle(const SDGeometryHandle handle) { handle_ = handle; }
    SDGeometryHandle geometry_handle() const { return handle_; }
};

template <typename T> int sgn(T val) {
    return (val >= 0) ? 1 : -1;
}

#endif
