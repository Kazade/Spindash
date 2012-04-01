#ifndef KP_OBJECT_H
#define KP_OBJECT_H

#include <tr1/memory>

typedef uint32_t ObjectID;

class World;

class Object {
private:
    kmVec2 position_;
    kmVec2 speed_;

    static std::map<ObjectID, Object*> objects_;

    World* world_;
    
public:
    typedef std::tr1::shared_ptr<Object> ptr;
    typedef Object type;

    Object(World* world);
    virtual ~Object();

    static void register_object(const Object* obj);
    static void unregister_object(const Object* obj);
    static Object* get_object_by_id(KPuint object_id);
    
    void set_position(float x, float y);
    void set_speed(float x, float y);

    const kmVec2& speed() const { return speed_; }
    const kmVec2& position() const { return position_; }
    
    void update(float dt);
};

#endif
