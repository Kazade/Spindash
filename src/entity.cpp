#include "entity.h"
#include "world.h"

static std::map<KPuint, boost::shared_ptr<Entity> > entities_;

KPuint kpCreateEntity(KPuint world_id) {
    World* world = get_world_by_id(world_id);



}
