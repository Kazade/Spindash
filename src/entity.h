#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED

#include <kazmath/kazmath.h>

#include "kazphysics2.h"

class World;

class Entity {
public:
    enum Flag {
        ON_GROUND,
        BLOCKED_LEFT,
        BLOCKED_RIGHT
    };

    static KPuint entity_id_counter_;

    Entity();

    void set_world(World* world) { world_ = world; }

    void update(double step);

    void set_flag(Flag f, bool value) { flags_[f] = value; }

    bool get_flag(Flag f) const {
        std::map<KPuint, bool>::const_iterator it = flags_.find(f);
        if(it == flags_.end()) {
            return false;
        }

        return (*it).second;
    }

private:
    World* world_;

    float acc_;
    float dec_;
    float frc_;
    float angle_;

    kmVec2 position_;
    kmVec2 speed_;
    kmVec2 size_;

    std::map<KPuint, bool> flags_;
};

#endif // ENTITY_H_INCLUDED
