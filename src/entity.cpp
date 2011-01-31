#include <map>
#include <iostream>
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

const float DEFAULT_ACC = (0.046875f * 60.0f) / 40.0f;
const float DEFAULT_DEC = (0.5f * 60.0f) / 40.0f;
const float DEFAULT_FRC = (0.046875f * 60.0f) / 40.0f;
const float DEFAULT_GRV = (0.21875f * 60.0f) / 40.0f;
const float DEFAULT_SLP = (0.125f * 60.0f) / 40.0f;
const float DEFAULT_JMP = 6.5f / 40.0f;
const float DEFAULT_JMP_CAP = 4.0f  / 40.0f;
const float DEFAULT_MAX = 6.0f / 40.0f;

Entity* get_bound_entity() {
    if(bound_entity == 0) {
        return NULL;
    }
    return entities_[bound_entity].get();
}

Entity::Entity():
    frc_(DEFAULT_FRC),
    angle_(0.0f),
    gsp_(0.0f),
    jumping_last_frame_(false),
    can_jump_(true) {

    kmVec2Fill(&position_, 0.0f, 0.0f);
    kmVec2Fill(&speed_, 0.0f, 0.0f);
    kmVec2Fill(&size_, DEFAULT_WIDTH, DEFAULT_HEIGHT);

    ray_ids_[RL_A] = 'A';
    ray_ids_[RL_B] = 'B';
    ray_ids_[RL_L] = 'L';
    ray_ids_[RL_R] = 'R';
}

void Entity::clear_collisions() {
    collisions_.clear();
}

void Entity::prevent_pass_through() {
    using namespace boost::lambda;

    kmRay2 speed_ray;
    initialize_ray(&speed_ray, 0.0f, 0.0f, speed_.x, speed_.y);

    std::vector<CollisionInfo> speed_collisions;

    for(KPuint i = 0; i < world_->get_triangle_count(); ++i) {
        Triangle* tri = world_->get_triangle_at(i);

        kmVec2 intersection, normal;

        for(KPuint i = 0; i < RL_MAX; ++i) {
            if(kmRay2IntersectTriangle(&speed_ray, &tri->points[0], &tri->points[1], &tri->points[2], &intersection, &normal)) {
                CollisionInfo info;
                info.intersection = intersection;
                info.surface_normal = normal;
                info.ray = speed_ray;
                kmVec2 tmp;
                kmVec2Subtract(&tmp, &info.intersection, &speed_ray.start);
                info.distance = kmVec2Length(&tmp);
                info.identifier = 'H';
                speed_collisions.push_back(info);
            }
        }
    }

    std::sort(speed_collisions.begin(), speed_collisions.end(), bind(&CollisionInfo::distance, _1) < bind(&CollisionInfo::distance, _2));
    CollisionInfo* speed_hit = (speed_collisions.empty()) ? NULL : &speed_collisions[0];
    if(speed_hit) {
        kmVec2 to_move;
        kmVec2Normalize(&to_move, &speed_hit->ray.dir);
        kmVec2Scale(&to_move, &to_move, -1.0f);
        kmVec2Add(&position_, &speed_hit->intersection, &to_move);

        if(fabs(speed_.x) > kmEpsilon) {
            speed_.x = 0.0f;
            gsp_ = 0.0f;
        }

        if(fabs(speed_.y) > kmEpsilon) {
            speed_.y = 0.0f;
        }
    }
}

void Entity::initialize_ray(kmRay2* out, float rel_start_x, float rel_start_y,
                            float ray_dir_x, float ray_dir_y) {
    kmMat3 rot_matrix;
    kmMat3RotationZ(&rot_matrix, kmDegreesToRadians(angle_));

    kmVec2 pos, dir;
    kmVec2Fill(&pos, rel_start_x, rel_start_y);
    kmVec2Fill(&dir, ray_dir_x, ray_dir_y);

    kmVec2Transform(&pos, &pos, &rot_matrix);
    kmVec2Transform(&dir, &dir, &rot_matrix);

    out->start = pos;
    kmVec2Add(&out->start, &out->start, &position_);
    out->dir = dir;
}

void Entity::collide_with_world() {
    float x_ray_offset = (size_.x / 2.0f) * 0.9f;
    initialize_ray(&rays_[RL_A], -x_ray_offset, 0.0f, 0.0f, -size_.y * 0.9f);
    initialize_ray(&rays_[RL_B], x_ray_offset, 0.0f, 0.0f, -size_.y * 0.9f);
    initialize_ray(&rays_[RL_L], 0.0f, -size_.y * 0.1f, -((size_.x / 2.0f)), 0.0f);
    initialize_ray(&rays_[RL_R], 0.0f, -size_.y * 0.1f, ((size_.x / 2.0f)), 0.0f);

    for(KPuint i = 0; i < world_->get_triangle_count(); ++i) {
        Triangle* tri = world_->get_triangle_at(i);

        kmVec2 intersection, normal;

        for(KPuint i = 0; i < RL_MAX; ++i) {
            if(kmRay2IntersectTriangle(&rays_[i], &tri->points[0], &tri->points[1], &tri->points[2], &intersection, &normal)) {
                CollisionInfo info;
                info.intersection = intersection;
                info.surface_normal = normal;
                info.ray = rays_[i];
                kmVec2 tmp;
                kmVec2Subtract(&tmp, &info.intersection, &rays_[i].start);
                info.distance = kmVec2Length(&tmp);
                info.identifier = ray_ids_[i];
                collisions_.push_back(info);
            }
        }
    }
}

void Entity::calculate_angle(CollisionInfo* a, CollisionInfo* b) {
    using namespace boost::lambda;

    float new_angle = 0.0f;
    if(a && b) {
        kmMat3 rot;
        kmMat3RotationZ(&rot, kmDegreesToRadians(-90.0f));
        kmVec2 dir;
        kmVec2Subtract(&dir, &b->intersection, &a->intersection);
        kmVec2Transform(&dir, &dir, &rot);
        kmVec2Normalize(&dir, &dir);

        kmVec2 up = { 0.0f, 1.0f };
        new_angle = -kmRadiansToDegrees(acosf(kmVec2Dot(&up, &dir)));

    } else if (a || b) {
        CollisionInfo* t = (a) ? a : b;

        kmVec2 up = { 0.0f, 1.0f };
        new_angle = -kmRadiansToDegrees(acosf(kmVec2Dot(&up, &t->surface_normal)));

    }

    //Don't change the angle if the floor just turned nearly verticla
    if(fabs(new_angle - angle_) <= 45.0f + kmEpsilon) {
        angle_ = new_angle;
    }

}

void Entity::respond_to_floor_collisions(std::vector<CollisionInfo>& a_collisions,
                                     std::vector<CollisionInfo>& b_collisions) {

    /*
        FIXME: Need to ignore collisions with surfaces that are almost parallel to the rays +/-15 degrees
    */

    if(!a_collisions.empty() || !b_collisions.empty()) {
        //Otherwise, we are, and we should process the nearest collision
        CollisionInfo *a = (a_collisions.empty()) ? NULL : &a_collisions[0];
        CollisionInfo *b = (b_collisions.empty()) ? NULL : &b_collisions[0];

        if((a && a->distance - kmEpsilon <= size_.y / 2.0f) ||
           (b && b->distance - kmEpsilon <= size_.y / 2.0f) ) {
            set_flag(ON_GROUND, true);
        }

        if(a && b) {
            //1. Move out by -ray_dir * greatest distance
            //2. Find the vector b - a;
            //3. Use it to calculate angle
            CollisionInfo* smallest = (a->distance < b->distance)? a : b;
            if(kmAlmostEqual(a->distance, b->distance)) {
                //If they are the same, always prefer A (stops jittering)
                smallest = a;
            }

            float offset_length = (size_.y / 2.0f) - smallest->distance;

            if(offset_length > kmEpsilon) {
                kmVec2 offset_vector;
                kmVec2Normalize(&offset_vector, &smallest->ray.dir);
                kmVec2Scale(&offset_vector, &offset_vector, -offset_length);
                kmVec2Add(&position_, &position_, &offset_vector);
            }
        } else {
            CollisionInfo* t = (a) ? a : b;
            //1. Move out by -ray_dir * distance
            //2. Find the normal of the collision
            //3. Use it to calculate the angle

            float offset_length = (size_.y / 2.0f) - t->distance;
            if(offset_length > kmEpsilon) {
                kmVec2 offset_vector;
                float offset_length = (t->distance - (size_.y / 2.0f));
                kmVec2Normalize(&offset_vector, &t->ray.dir);
                kmVec2Scale(&offset_vector, &offset_vector, -offset_length);
                kmVec2Add(&position_, &position_, &offset_vector);
            }
        }

        calculate_angle(a, b);
    } else {
        angle_ = 0.0f;
    }
}

void Entity::respond_to_wall_collisions(std::vector<CollisionInfo>& collisions, Flag blocked_flag) {
    set_flag(blocked_flag, false);

    /*
        FIXME:
        For the purposes of the front/back rays, we want to ignore surfaces that
        are < 45 degrees away from horizontal. e.g. we want to collide with surfaces
        that are near upright, but not horizontal ones (or we'll get stuck)
    */

    CollisionInfo *nearest = (collisions.empty()) ? NULL : &collisions[0];
    if(!nearest) {
        return;
    }

    float offset_distance = kmVec2Length(&nearest->ray.dir) - nearest->distance;

    kmVec2 to_move;
    kmVec2Normalize(&to_move, &nearest->ray.dir);
    kmVec2Scale(&to_move, &to_move, -offset_distance);
    kmVec2Add(&position_, &position_, &to_move);

    set_flag(blocked_flag, true);
}

void Entity::process_collisions() {
    using namespace boost::lambda;

    std::vector<CollisionInfo> a_collisions, b_collisions, l_collisions, r_collisions;

    for(KPuint i = 0; i < collisions_.size(); ++i) {
        switch(collisions_[i].identifier) {
            case 'A':
                a_collisions.push_back(collisions_[i]);
            break;
            case 'B':
                b_collisions.push_back(collisions_[i]);
            break;
            case 'L':
                l_collisions.push_back(collisions_[i]);
            break;
            case 'R':
                r_collisions.push_back(collisions_[i]);
            break;
            default:
                break;
        }
    }

    //Sort them by distance
    std::sort(a_collisions.begin(), a_collisions.end(), bind(&CollisionInfo::distance, _1) < bind(&CollisionInfo::distance, _2));
    std::sort(b_collisions.begin(), b_collisions.end(), bind(&CollisionInfo::distance, _1) < bind(&CollisionInfo::distance, _2));
    std::sort(l_collisions.begin(), l_collisions.end(), bind(&CollisionInfo::distance, _1) < bind(&CollisionInfo::distance, _2));
    std::sort(r_collisions.begin(), r_collisions.end(), bind(&CollisionInfo::distance, _1) < bind(&CollisionInfo::distance, _2));

    set_flag(ON_GROUND, false);

    respond_to_floor_collisions(a_collisions, b_collisions);
    respond_to_wall_collisions(l_collisions, BLOCKED_LEFT);
    respond_to_wall_collisions(r_collisions, BLOCKED_RIGHT);
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

    if(get_flag(MOVING_LEFT)) {
        if(get_flag(ON_GROUND)) {
            if(speed_.x > kmEpsilon){
                gsp_ -= DEFAULT_DEC * step;
            } else {
                gsp_ -= DEFAULT_ACC * step;
            }
        } else {
            speed_.x -= DEFAULT_ACC * 2.0f * step;
        }
    }

    if(get_flag(MOVING_RIGHT)) {
        if(get_flag(ON_GROUND)) {
            if(speed_.x < kmEpsilon) {
                gsp_ += DEFAULT_DEC * step;
            } else {
                gsp_ += DEFAULT_ACC * step;
            }

        } else {
            speed_.x += DEFAULT_ACC * 2.0f * step;
        }
    }

    if(get_flag(ON_GROUND)) {
        gsp_ += DEFAULT_SLP * sin(kmDegreesToRadians(angle_)) * step;
        //Apply ground friction (only if not accelerating)
        if(!get_flag(MOVING_LEFT) && !get_flag(MOVING_RIGHT)) {
            gsp_ -= std::min(fabs(gsp_), DEFAULT_FRC * step) * sign(gsp_);
        }
    }

    if(get_flag(ON_GROUND)) {
        //If we are on the ground we derive speed from gsp_ instead of using speed directly
        speed_.x = gsp_ * cosf(kmDegreesToRadians(angle_));
        speed_.y = gsp_ * -sinf(kmDegreesToRadians(angle_));
    }

    //check_entity_collisions();

    if(get_flag(BLOCKED_LEFT) && speed_.x < -kmEpsilon) {
        speed_.x = 0.0f;
        if(gsp_ < 0.0f) {
            gsp_ = 0.0f;
        }
    }

    if(get_flag(BLOCKED_RIGHT) && speed_.x > kmEpsilon) {
        speed_.x = 0.0f;
        if(gsp_ < 0.0f) {
            gsp_ = 0.0f;
        }
    }

    apply_gravity(step);

    prevent_pass_through();

    if(get_flag(ON_GROUND) && !can_jump_) {
        can_jump_ = true;
        is_jumping_ = false;
    }

    //FIXME: Jumping shouldn't occur immediately
    if(!jumping_last_frame_ && get_flag(JUMPING) && get_flag(ON_GROUND) && can_jump_) {
        //We just started jumping
        speed_.x += DEFAULT_JMP * sinf(kmDegreesToRadians(angle_));
        speed_.y += DEFAULT_JMP * cosf(kmDegreesToRadians(angle_));

        can_jump_ = false;
        jump_released_ = false;
        is_jumping_ = true;
    }

    if(jumping_last_frame_ && !get_flag(JUMPING) && !jump_released_ && speed_.y > DEFAULT_JMP_CAP) {
        //We just released jump
        speed_.y = DEFAULT_JMP_CAP;
        jump_released_ = true;
    }

    jumping_last_frame_ = get_flag(JUMPING);

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
        case KP_ENTITY_COLLISION_RAY_L: {
            kmRay2 ray = ent->get_ray_l();
            pOut[0] = ray.start.x;
            pOut[1] = ray.start.y;
            pOut[2] = ray.dir.x;
            pOut[3] = ray.dir.y;
        }
        break;
        case KP_ENTITY_COLLISION_RAY_R: {
            kmRay2 ray = ent->get_ray_r();
            pOut[0] = ray.start.x;
            pOut[1] = ray.start.y;
            pOut[2] = ray.dir.x;
            pOut[3] = ray.dir.y;
        }
        break;
        case KP_ENTITY_ROTATION: {
            *pOut = ent->get_angle();
        }
        break;
        case KP_ENTITY_GROUND_SPEED: {
            *pOut = ent->get_ground_speed();
        }
        break;
        case KP_ENTITY_X_SPEED: {
            *pOut = ent->get_x_speed();
        }
        break;
        //FIXME: Shouldn't be here
        case KP_ENTITY_IS_JUMPING:
            *pOut = (ent->get_is_jumping())? 1 : 0;
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

void kpEntityStartMovingLeft() {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }

    ent->set_flag(Entity::MOVING_LEFT, true);
}

void kpEntityStopMovingLeft() {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }
    ent->set_flag(Entity::MOVING_LEFT, false);
}

void kpEntityStartMovingRight() {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }

    ent->set_flag(Entity::MOVING_RIGHT, true);
}

void kpEntityStopMovingRight() {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }
    ent->set_flag(Entity::MOVING_RIGHT, false);
}

void kpEntityStartJumping() {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }
    ent->set_flag(Entity::JUMPING, true);
}

void kpEntityStopJumping() {
    Entity* ent = get_bound_entity();
    if(!ent) {
        //Log error
        return;
    }
    ent->set_flag(Entity::JUMPING, false);
}
