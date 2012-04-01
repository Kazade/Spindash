
#include "character.h"
#include "world.h"

KPuint kpCreateCharacter(KPuint world) {
    World* world = get_world_by_id(world);
    return world->new_character();
}

void kpDestroyCharacter(KPuint entity) {
    Object* obj = Object::get_object_by_id(entity);
    
    Character* c = dynamic_cast<Character*> (obj);
    assert(c && "Not a character");
    
    obj->world()->destroy_object(entity);
}
