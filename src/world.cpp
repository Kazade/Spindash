#include <map>
#include <GL/gl.h>
#include <boost/shared_ptr.hpp>

#include "kazmath/vec2.h"
#include "world.h"
#include "character.h"
#include "spindash.h"

SDuint World::world_id_counter_ = 0;

World::World(SDuint id):
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
    float colours [10][3] = {
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

    glPushAttrib(~0);
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPushMatrix();
        glBegin(GL_TRIANGLES);
        for(unsigned int i = 0; i < triangles_.size(); ++i) {
            float* colour = colours[colour_counter];
            (colour_counter >= 9) ? colour_counter = 0: colour_counter++;

            glColor3f(colour[0], colour[1], colour[2]);

            for(unsigned int j = 0; j < 3; ++j) {
                glVertex3f(triangles_[i].points[j].x, triangles_[i].points[j].y, 0.0f);
            }
        }
        glEnd();
    glPopMatrix();
 
    for(std::vector<Object::ptr>::const_iterator it = objects_.begin(); it != objects_.end(); ++it) {
        float* colour = colours[colour_counter];
        (colour_counter >= 9) ? colour_counter = 0: colour_counter++;
        glColor3f(colour[0], colour[1], colour[2]);

        glPushMatrix();
            kmVec2 pos = sdObjectGetPosition((*it)->id());
            
            glBegin(GL_POINTS);
                glVertex2f(pos.x, pos.y);
            glEnd();

            if(Character* c = dynamic_cast<Character*>((*it).get())) {
                kmRay2 ray;
                for(char ray_id: {'A', 'B', 'L', 'R'}) {
                    ray = c->ray(ray_id);
                    glBegin(GL_LINES);
                        glVertex2f(ray.start.x, ray.start.y);
                        glVertex2f(ray.start.x + ray.dir.x, ray.start.y + ray.dir.y);
                    glEnd();
                }
            }
        glPopMatrix();
    }
    glPopAttrib();
}

void World::update(float step) {
    for(std::vector<Object::ptr>::iterator it = objects_.begin(); it != objects_.end(); ++it) {
        (*it)->update(step);
    }
}

void World::destroy_object(ObjectID object_id) {
    struct PointerCompare {
        PointerCompare(Object* ptr): ptr_(ptr) {}
        bool operator()(Object::ptr rhs) { return rhs.get() == ptr_; }
    
        Object* ptr_;
    };

    assert(Object::exists(object_id));
    Object* obj = Object::by_id(object_id);
    objects_.erase(std::remove_if(objects_.begin(), objects_.end(), PointerCompare(obj)), objects_.end());
    assert(!Object::exists(object_id));
}
    
void World::add_triangle(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3) {
    Triangle new_tri;

    new_tri.points[0] = v1;
    new_tri.points[1] = v2;
    new_tri.points[2] = v3;

    triangles_.push_back(new_tri);
}

ObjectID World::new_character() {
    Character::ptr new_character(new Character(this));    
    objects_.push_back(new_character);
    return new_character->id();
}

//=============================================================

static std::map<SDuint, boost::shared_ptr<World> > worlds_;

World* get_world_by_id(SDuint world) {
    if(worlds_.find(world) == worlds_.end()) {
        return NULL;
    }

    return worlds_[world].get();
}

/**
    @brief Creates a new physical world

    This function creates an empty world ready to start accepting
    new entities and polygons.
*/
SDuint sdCreateWorld() {
    SDuint new_id = ++World::world_id_counter_;
    worlds_[new_id].reset(new World(new_id));
    return new_id;
}

/** \brief Destroys a world
 *
 * \param world - The world to destroy
 *
 * Destroys a world and its contents (polygons, entities etc.)
 */

void sdDestroyWorld(SDuint world) {
    if(worlds_.find(world) == worlds_.end()) {
        //TODO: log error
        return;
    }

    worlds_.erase(world);
}

void sdWorldAddTriangle(SDuint world_id, kmVec2* points) {
    World* world = get_world_by_id(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->add_triangle(points[0], points[1], points[2]);
}

void sdWorldAddMesh(SDuint world_id, SDuint num_triangles, kmVec2* points) {
    for(SDuint i = 0; i < num_triangles; ++i) {
        kmVec2 tri[3];
        kmVec2Assign(&tri[0], &points[i * 3]);
        kmVec2Assign(&tri[1], &points[(i * 3) + 1]);        
        kmVec2Assign(&tri[2], &points[(i * 3) + 2]);
        sdWorldAddTriangle(world_id, tri);
    }
}

void sdWorldStep(SDuint world_id, SDfloat dt) {
    World* world = get_world_by_id(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->update(dt);
}

void sdWorldDebugRenderGL(SDuint world_id) {
    World* world = get_world_by_id(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->debug_render();
}
