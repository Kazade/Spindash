#include <map>
#include <boost/shared_ptr.hpp>

#include "world.h"

KPuint World::world_id_counter_ = 0;

World::World(KPuint id):
    id_(id) {
    set_gravity(0.0f, -7.0f);
}

void World::set_gravity(float x, float y) {
    kmVec2Fill(&gravity_, x, y);
}

void World::get_gravity(float& x, float& y) {
    x = gravity_.x;
    y = gravity_.y;
}

void World::debug_render() {
    float colours [] = {
        { 1.0f, 0.0f, 0.0f},
        { 0.0f, 1.0f, 0.0f},
        { 0.0f, 0.0f, 1.0f},
        { 1.0f, 1.0f, 0.0f},
        { 1.0f, 0.0f, 1.0f},
        { 0.0f, 1.0f, 1.0f},
        { 0.5f, 0.5f, 0.5f},
        { 0.5f, 0.5f, 0.0f},
        { 0.5f, 0.0f, 0.5f},
        { 0.0f, 0.5f, 0.5f},
    };

    int colour_counter = 0;

    glPushMatrix();
        glBegin(GL_TRIANGLES);
        for(unsigned int i = 0; i < triangles_.size(); ++i) {
            float* colour = colours[colour_counter++];
            glColor3f(colour[0], colour[1], colour[2]);

            for(unsigned int j = 0; j < 3; ++j) {
                glVertex3f(triangles_[i].points[j].x, triangles_[i].points[j].y, 0.0f);
            }
        }
        glEnd();
    glPopMatrix();
}

void World::add_triangle(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3) {
    Triangle new_tri;

    new_tri.points[0] = v1;
    new_tri.points[1] = v2;
    new_tri.points[2] = v3;

    triangles_.push_back(new_tri);
}

//=============================================================

kmVec2 kpTokm(const KPvec2& v) {
    kmVec2 v2;
    v2.x = v.x;
    v2.y = v.y;

    return v2;
}

static std::map<KPuint, boost::shared_ptr<World> > worlds_;

World* get_world_by_id(KPuint world) {
    if(worlds_.find(world) == worlds_.end()) {
        return NULL;
    }

    return worlds_[world].get();
}

KPuint kpCreateWorld() {
    KPuint new_id = ++World::world_id_counter_;
    worlds_[new_id].reset(new World(new_id));
    return new_id;
}

void kpDestroyWorld(KPuint world) {
    if(worlds_.find(world) == worlds_.end()) {
        //TODO: log error
        return;
    }

    worlds_.erase(world);
}

void kpWorldParameterfv(KPuint world_id, KPenum pname, KPfloat* param) {
    World* world = get_world_by_id(world_id);

    if(!world) {
        //Log error
        return;
    }

    switch(pname) {
        case KP_WORLD_GRAVITY:
            world->set_gravity(param[0], param[1]);
        break;
        default:
            //Log error
            return;
    }
}

void kpWorldAddTriangle(KPuint world_id, KPvec2* points) {
    World* world = get_world_by_id(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->add_triangle(kpTokm(points[0]), kpTokm(points[1]), kpTokm(points[2]));
}

void kpWorldDebugRenderGL(KPuint world) {
    World* world = get_world_by_id(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->debug_render();
}
