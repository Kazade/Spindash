#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "entity.h"
#include "world.h"

KPuint Entity::entity_id_counter_ = 0;
static KPuint bound_entity = 0;

static std::map<KPuint, boost::shared_ptr<Entity> > entities_;

const float DEFAULT_HEIGHT = 1.0f;
const float DEFAULT_WIDTH = DEFAULT_HEIGHT / 2.0f;

const float DEFAULT_ACC = (0.046875f * 60.0f);
const float DEFAULT_DEC = (0.5f * 60.0f);
const float DEFAULT_FRC = (0.046875f * 60.0f);
const float DEFAULT_GRV = (0.21875f * 60.0f);
const float DEFAULT_SLP = (0.125f * 60.0f);

Entity* get_bound_entity() {
    if(bound_entity == 0) {
        return NULL;
    }
    return entities_[bound_entity].get();
}

Entity::Entity():
    frc_(DEFAULT_FRC),
    angle_(0.0f) {

    kmVec2Fill(&position_, 0.0f, 0.0f);
    kmVec2Fill(&speed_, 0.0f, 0.0f);
    kmVec2Fill(&size_, DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void Entity::clear_collisions() {
    collisions_.clear();
}

void Entity::collide_with_world() {
    float x_ray_offset = (size_.x / 2.0f) * 0.9f;

    kmVec2 ra_pos, ra_dir;
    kmVec2 rb_pos, rb_dir;

    kmMat3 rot_matrix;
    kmMat3RotationZ(&rot_matrix, kmDegreesToRadians(angle_));

    kmVec2Fill(&ra_pos, -x_ray_offset, 0.0f);
    kmVec2Fill(&ra_dir, 0.0f, -size_.y / 2.0f);

    kmVec2Fill(&rb_pos, x_ray_offset, 0.0f);
    kmVec2Fill(&rb_dir, 0.0f, -size_.y / 2.0f);

    kmVec2Transform(&ra_pos, &ra_pos, &rot_matrix);
    kmVec2Transform(&ra_dir, &ra_dir, &rot_matrix);

    kmVec2Transform(&rb_pos, &rb_pos, &rot_matrix);
    kmVec2Transform(&rb_dir, &rb_dir, &rot_matrix);

    ra_.start = ra_pos;
    kmVec2Add(&ra_.start, &ra_.start, &position_);
    ra_.dir = ra_dir;

    rb_.start = rb_pos;
    kmVec2Add(&rb_.start, &rb_.start, &position_);
    rb_.dir = rb_dir;

    for(KPuint i = 0; i < world_->get_triangle_count(); ++i) {
        Triangle* tri = world_->get_triangle_at(i);

        kmVec2 intersection, normal;

        if(kmRay2IntersectTriangle(&ra_, &tri->points[0], &tri->points[1], &tri->points[2], &intersection, &normal)) {
            CollisionInfo info;
            info.intersection = intersection;
            info.surface_normal = normal;
            info.ray = ra_;
            kmVec2 tmp;
            kmVec2Subtract(&tmp, &info.intersection, &ra_.start);
            info.distance = kmVec2Length(&tmp);
            info.identifier = 'A';
            collisions_.push_back(info);
        }

        if(kmRay2IntersectTriangle(&rb_, &tri->points[0], &tri->points[1], &tri->points[2], &intersection, &normal)) {
            CollisionInfo info;
            info.intersection = intersection;
            info.surface_normal = normal;
            info.ray = rb_;
            kmVec2 tmp;
            kmVec2Subtract(&tmp, &info.intersection, &rb_.start);
            info.distance = kmVec2Length(&tmp);
            info.identifier = 'B';
            collisions_.push_back(info);
        }
    }

/*
        kmVec2 offset_vector;
        kmVec2 up = { 0.0f, 1.0f };

        if(a_collided && b_collided) {
            if(info_a.distance < info_b.distance) {
                //If a was the closer collision, we need to move out using its normal
                float offset_length = kmVec2Length(&ra_.dir) - info_a.distance;
                kmVec2Scale(&offset_vector, &info_a.surface_normal, offset_length);
                kmVec2Add(&position_, &position_, &offset_vector);
                angle_ = -kmRadiansToDegrees(acosf(kmVec2Dot(&up, &info_a.surface_normal)));
            } else {
                float offset_length = kmVec2Length(&rb_.dir) - info_b.distance;
                kmVec2Scale(&offset_vector, &info_b.surface_normal, offset_length);
                kmVec2Add(&position_, &position_, &offset_vector);
                angle_ = -kmRadiansToDegrees(acosf(kmVec2Dot(&up, &info_b.surface_normal)));
            }
        } else if(a_collided) {
            float offset_length = kmVec2Length(&ra_.dir) - info_a.distance;
            kmVec2Scale(&offset_vector, &info_a.surface_normal, offset_length);
            kmVec2Add(&position_, &position_, &offset_vector);
            angle_ = -kmRadiansToDegrees(acosf(kmVec2Dot(&up, &info_a.surface_normal)));

        } else if(b_collided) {
            float offset_length = kmVec2Length(&rb_.dir) - info_b.distance;
            kmVec2Scale(&offset_vector, &info_b.surface_normal, offset_length);
            kmVec2Add(&position_, &position_, &offset_vector);
            angle_ = -kmRadiansToDegrees(acosf(kmVec2Dot(&up, &info_b.surface_normal)));
        }*/

        /*TODO:
            Get the normal from the intersection, move the character out by the overlap
            update angle_ to reflect the new ground angle
        */
}

void Entity::process_collisions() {
    using namespace boost::lambda;

    std::vector<CollisionInfo> down_collisions;

    for(KPuint i = 0; i < collisions_.size(); ++i) {
        //Store all downward collisions
        if(collisions_[i].identifier == 'A' || collisions_[i].identifier == 'B') {
            down_collisions.push_back(collisions_[i]);
        }
    }

    //Sort them by distance
    std::sort(down_collisions.begin(), down_collisions.end(), bind(&CollisionInfo::distance, _1) < bind(&CollisionInfo::distance, _2));

    if(down_collisions.empty()) {
        //If there weren't any, we're not on the ground
        set_flag(ON_GROUND, false);
    } else {
        //Otherwise, we are, and we should process the nearest collision
        set_flag(ON_GROUND, true);

        CollisionInfo& info = down_collisions[0]; //Get the closest collision
        kmVec2 offset_vector;
        float offset_length = kmVec2Length(&info.ray.dir) - info.distance;
        kmVec2Scale(&offset_vector, &info.surface_normal, offset_length);
        kmVec2Add(&position_, &position_, &offset_vector);

        kmVec2 up = { 0.0f, 1.0f };
        angle_ = -kmRadiansToDegrees(acosf(kmVec2Dot(&up, &info.surface_normal)));
    }
}

void Entity::apply_gravity(double step) {
    if(get_flag(ON_GROUND)) {
        return;
    }
    speed_.y -= DEFAULT_GRV * step;
}

int sign(float v) {
    if(v > 0.0f) {
        return 1;
    }

    return -1;
}

void Entity::update(double step) {
    clear_collisions();
    collide_with_world();
    process_collisions();

    gsp_ += DEFAULT_SLP * step * sin(kmDegreesToRadians(angle_));

    if(get_flag(ON_GROUND)) {
        //Apply ground friction
        gsp_ = gsp_ - std::min(fabs(gsp_), DEFAULT_FRC * step) * sign(gsp_);
    }

    speed_.x = gsp_ * cosf(kmDegreesToRadians(angle_)) * step;
    speed_.y = gsp_ * sinf(kmDegreesToRadians(angle_)) * step;

    //check_entity_collisions();

    apply_gravity(step);

    position_.x += speed_.x;
    position_.y += speed_.y;
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

void kpBindEntity(KPuint entity_id) {
    if(entities_.find(entity_id) == entities_.end()) {
        return;
    }

    bound_entity = entity_id;
}

void kpEntityGetFloatfv(KPenum pname, KPfloat* pOut) {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }

    switch(pname) {
        case KP_ENTITY_POSITION: {
            kmVec2 position = ent->get_position();
            pOut[0] = position.x;
            pOut[1] = position.y;
        }
        break;
        case KP_ENTITY_COLLISION_RAY_A: {
            kmRay2 ray = ent->get_ray_a();
            pOut[0] = ray.start.x;
            pOut[1] = ray.start.y;
            pOut[2] = ray.dir.x;
            pOut[3] = ray.dir.y;
        }
        break;
        case KP_ENTITY_COLLISION_RAY_B: {
            kmRay2 ray = ent->get_ray_b();
            pOut[0] = ray.start.x;
            pOut[1] = ray.start.y;
            pOut[2] = ray.dir.x;
            pOut[3] = ray.dir.y;
        }
        break;
        default:
            assert(0);
            //Log error
            break;
    }
}

void kpEntityParameterfv(KPenum pname, KPfloat* param) {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }

    switch(pname) {
        case KP_ENTITY_POSITION: {
            ent->set_position(param[0], param[1]);
        }
        break;
        default:
            //Log error
            break;
    }
}

void kpEntityUpdate(double step) {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }

    ent->update(step);
}
