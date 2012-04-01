#ifndef KP_OBJECT_H
#define KP_OBJECT_H

#include <cstdint>
#include <map>
#include <tr1/memory>

#include "typedefs.h"
#include "kazmath/vec2.h"

typedef uint32_t ObjectID;

class World;

class Object {
private:
    kmVec2 position_;
    kmVec2 speed_;
    kmVec2 acceleration_;

    static std::map<ObjectID, Object*> objects_;

    World* world_;
    
    SDuint id_;
    
    virtual void pre_update(float dt) {}
    virtual void post_update(float dt) {}
    virtual void post_speed_update(float dt) {}
    
public:
    typedef std::tr1::shared_ptr<Object> ptr;

    Object(World* world);
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
    void update(float dt);
    
    SDuint id() const { return id_; }
    
    World* world() { return world_; }
};

#endif
