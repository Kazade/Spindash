#include <cassert>
#include <map>
#include <GL/gl.h>
#include <functional>
#include <algorithm>
#include <tr1/functional>
#include <tr1/memory>

#include "collision/collide.h"
#include "kazmath/vec2.h"
#include "world.h"

#include "character.h"
#include "spring.h"

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
        { 0.5f, 1.0f, 0.5f},
        { 0.5f, 1.0f, 0.0f},
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
                glVertex2f(triangles_[i].points[j].x, triangles_[i].points[j].y);
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
                RayBox* box = dynamic_cast<RayBox*>(&c->geom());
                for(char ray_id: {'A', 'B', 'C', 'D', 'L', 'R'}) {
                    kmRay2& ray = box->ray(ray_id);
                    glBegin(GL_LINES);
                        glVertex2f(ray.start.x, ray.start.y);
                        glVertex2f(ray.start.x + ray.dir.x, ray.start.y + ray.dir.y);
                    glEnd();
                }
            } else if (Spring* s = dynamic_cast<Spring*>((*it).get())) {
                Box* box = dynamic_cast<Box*>(&s->geom());
                
                for(uint32_t i = 0; i < 4; ++i) {
                    kmVec2& point = box->point(i);
                    kmVec2& next_point = box->point((i == 3) ? 0 : i+1);
                    glBegin(GL_LINES);
                        glVertex2f(point.x, point.y);
                        glVertex2f(next_point.x, next_point.y);
                    glEnd();
                }                
            }
        glPopMatrix();
    }
    glPopAttrib();
}

void World::update(float step) {
    /*
        How should collisions be processed? Collision detection and respons
        is recursive, and also, we need to do some sweeping tests
        during collision. So, to collide we need to have something like
        
        update_but_dont_move(objects)       
        
        for i in xrange(len(objects)):
            lhs = objects[i]
            
            first_loop = True
            collisions = []
            
            while collisions or first_loop:
                first_loop = False
                
                for triangle in world:
                    collisions.extend(lhs.collide_with(triangle))
                
                for j in xrange(start=i+1, stop=len(objects)):                
                    rhs = objects[j]                
                    collisions.append(lhs.coilide_with(rhs)

                if collisions:                    
                    lhs.respond_to(collisions.sort_by(distance))
    */                                
    
    //Call update on each object, this shouldn't change the objects position, but just velocity etc.
    std::for_each(objects_.begin(), objects_.end(), std::tr1::bind(&Object::prepare, std::tr1::placeholders::_1, step));

    for(uint32_t i = 0; i < objects_.size(); ++i) {
        Object& lhs = *objects_.at(i);
        bool first_loop = true;
        std::vector<Collision> collisions;
        
        lhs.update(step); //Move without responding to collisions

        while(!collisions.empty() || first_loop) {
            first_loop = false;
            
            for(uint32_t j = 0; j < get_triangle_count(); ++j) {
                Triangle& triangle = triangles_.at(j);
                
                std::vector<Collision> new_collisions = collide(&lhs.geom(), &triangle);
                if(!new_collisions.empty()) {
                    collisions.insert(collisions.end(), new_collisions.begin(), new_collisions.end());
                }
            }
            
            for(uint32_t j = i + 1; j < objects_.size(); ++j) {
                Object& rhs = *objects_.at(j);                
                std::vector<Collision> new_collisions = collide(&lhs.geom(), &rhs.geom());
                if(!new_collisions.empty()) {
                    collisions.insert(collisions.end(), new_collisions.begin(), new_collisions.end());
                }                
                
                rhs.respond_to(new_collisions);
            }   
            
            if(!collisions.empty()) {
                lhs.respond_to(collisions);                
                collisions.clear();
            }
        }
        lhs.update_finished(step);
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

ObjectID World::new_spring(float angle, float power) {
    Spring::ptr new_spring(new Spring(this, angle, power));    
    objects_.push_back(new_spring);
    return new_spring->id();
}

//=============================================================

static std::map<SDuint, std::tr1::shared_ptr<World> > worlds_;

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
SDuint sdWorldCreate() {
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

void sdWorldDestroy(SDuint world) {
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

void sdWorldRemoveTriangles(SDuint world_id) {
    World* world = get_world_by_id(world_id);
    assert(world);
    world->remove_all_triangles();
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
