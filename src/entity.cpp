#include <map>
#include <boost/shared_ptr.hpp>
#include "entity.h"
#include "world.h"

KPuint Entity::entity_id_counter_ = 0;
static std::map<KPuint, boost::shared_ptr<Entity> > entities_;

const float DEFAULT_HEIGHT = 1.0f;
const float DEFAULT_WIDTH = DEFAULT_HEIGHT / 2.0f;

const float DEFAULT_ACC = ((0.046875f / 40.0f) * 60.0f);
const float DEFAULT_DEC = ((0.5f / 40.0f) * 60.0f);
const float DEFAULT_FRC = ((0.046875f / 40.0f) * 60.0f);
const float DEFAULT_GRV = ((0.21875f / 40.0f) * 60.0f);

Entity::Entity():
    acc_(DEFAULT_ACC),
    dec_(DEFAULT_DEC),
    frc_(DEFAULT_FRC),
    angle_(0.0f) {

    kmVec2Fill(&position_, 0.0f, 0.0f);
    kmVec2Fill(&speed_, 0.0f, 0.0f);
    kmVec2Fill(&size, DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void Entity::update(double step) {
    //clear_collisions();
    //check_world_collisions();
    //check_entity_collisions();

    //Do stuff with the collision info
    float x_ray_offset = (size_.x / 2.0f) * 0.9f;

    kmRay2 ra, rb;
    kmVec2 ra_pos, ra_dir;

    kmVec2Transform(&ra_pos, &ra_pos, &rot_matrix);
    kmVec2Transform(&ra_dir, &ra_dir, &rot_matrix);

    kmVec2Fill(&ra_pos, position_.x - x_ray_offset, position_.y);
    kmVec2Fill(&ra_pos, 0.0f, -size_.y / 2.0f);

    if(!get_flag(ON_GROUND)) {
        //Apply gravity, only if we aren't on the ground
        //apply_gravity();
    }
}

KPuint kpCreateEntity(KPuint world_id) {
    World* world = get_world_by_id(world_id);

    if(!world) {
        //Log error
        return 0;
    }

    KPuint new_id = ++Entity::entity_id_counter_;
    entities_[new_id].reset(new Entity());
    entities_[new_id]->set_world(world);

    world->add_entity(new_id);
    return new_id;
}
