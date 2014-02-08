#include <iostream>
#include <cassert>
#include <map>
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

const kmVec2 GRAVITY_IN_MPS = { 0, (-0.21875 / 40.0) * 60.0};

SDuint World::world_id_counter_ = 0;

World::World(SDuint id):
    id_(id),
	step_counter_(0),
	step_mode_enabled_(false) {
    set_gravity(0.0f, GRAVITY_IN_MPS.y);
    
}

void World::set_gravity(float x, float y) {
    kmVec2Fill(&gravity_, x, y);
}

kmVec2 World::gravity() const {
    return gravity_;
}

void World::render() {
    if(!compile_callback_ || !render_callback_) {
        return;
    }

    SDfloat angle = 0;
    SDVec2 translation;
    translation.x = 0;
    translation.y = 0;

    for(const Triangle& triangle: triangles_) {
        auto handle = triangle.geometry_handle();
        if(handle) {
            render_callback_->callback(handle, &translation, angle, render_callback_->user_data);
        }
    }

    for(const Box& box: boxes_) {
        auto handle = box.geometry_handle();
        if(handle) {
            render_callback_->callback(handle, &translation, angle, render_callback_->user_data);
        }
    }

    for(const auto& object: objects_) {
        auto handle = object->geometry_handle();
        if(handle) {
            auto trans = object->position();
            render_callback_->callback(handle, &trans, object->rotation(), render_callback_->user_data);
        }
    }
}

/*
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
        glBegin(GL_QUADS);
        for(unsigned int i = 0; i < boxes_.size(); ++i) {
            float* colour = colours[colour_counter];
            (colour_counter >= 9) ? colour_counter = 0: colour_counter++;

            glColor3f(colour[0], colour[1], colour[2]);

            for(unsigned int j = 0; j < 4; ++j) {
                glVertex2f(boxes_[i].point(j).x, boxes_[i].point(j).y);
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
}*/

void World::update(double step, bool override_step_mode) {
	if(!override_step_mode && step_mode_enabled_) return;
	
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
        bool run_loop = true;
        std::vector<Collision> collisions;
        
        lhs.update(step); //Move without responding to collisions
        uint32_t tries = 10;
        while(run_loop && tries--) {
            for(uint32_t j = 0; j < get_triangle_count(); ++j) {
                Triangle& triangle = triangles_.at(j);
                
                std::vector<Collision> new_collisions = collide(&lhs.geom(), &triangle);
                if(!new_collisions.empty()) {
                    collisions.insert(collisions.end(), new_collisions.begin(), new_collisions.end());
                }
            }

            for(uint32_t j = 0; j < get_box_count(); ++j) {
                Box& triangle = boxes_.at(j);
                
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
                run_loop = lhs.respond_to(collisions);                
                collisions.clear();
            } else {
                run_loop = false;
            }
            
            if(debug_mode_enabled()) { 
				run_loop = false;
			}
        }
        lhs.update_finished(step);
        if(!tries) {
            lhs.revert_to_safe_position();
        } else {
            lhs.store_safe_position();
        }
    }
    
    ++step_counter_;
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

    new_tri.points()[0] = v1;
    new_tri.points()[1] = v2;
    new_tri.points()[2] = v3;

    if(compile_callback_) {
        std::vector<SDuint> indexes = { 0, 1, 2 };

        SDGeometryHandle new_handle = compile_callback_->callback(
            SD_RENDER_MODE_TRIANGLES, &new_tri.points()[0], 3, &indexes[0], indexes.size(), compile_callback_->user_data
        );

        new_tri.set_geometry_handle(new_handle);
    }

    triangles_.push_back(new_tri);
}

void World::add_box(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3, const kmVec2& v4) {
    Box new_box;
    new_box.point(0) = v1;
    new_box.point(1) = v2;
    new_box.point(2) = v3;
    new_box.point(3) = v4;

    if(compile_callback_) {
        std::vector<SDuint> indexes = { 0, 1, 2, 0, 2, 3 };

        SDGeometryHandle new_handle = compile_callback_->callback(
            SD_RENDER_MODE_TRIANGLES, &new_box.points()[0], 4, &indexes[0], indexes.size(), compile_callback_->user_data
        );

        new_box.set_geometry_handle(new_handle);
    }
    
    boxes_.push_back(new_box);
}

ObjectID World::new_character() {
    Character::ptr new_character(new Character(this, 0.5f, 1.0f));

    RayBox& box = dynamic_cast<RayBox&>(new_character->geom());

    std::vector<SDVec2> vertices;
    vertices.push_back(box.ray('A').start);
    SDVec2 tmp;
    kmVec2Add(&tmp, &box.ray('A').start, &box.ray('A').dir);
    vertices.push_back(tmp);
    vertices.push_back(box.ray('B').start);
    kmVec2Add(&tmp, &box.ray('B').start, &box.ray('B').dir);
    vertices.push_back(tmp);
    vertices.push_back(box.ray('C').start);
    kmVec2Add(&tmp, &box.ray('C').start, &box.ray('C').dir);
    vertices.push_back(tmp);

    vertices.push_back(box.ray('D').start);
    kmVec2Add(&tmp, &box.ray('D').start, &box.ray('D').dir);
    vertices.push_back(tmp);
    vertices.push_back(box.ray('L').start);
    kmVec2Add(&tmp, &box.ray('L').start, &box.ray('L').dir);
    vertices.push_back(tmp);

    vertices.push_back(box.ray('R').start);
    kmVec2Add(&tmp, &box.ray('R').start, &box.ray('R').dir);
    vertices.push_back(tmp);

    std::vector<SDuint> indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

    if(compile_callback_) {
        SDGeometryHandle new_handle = compile_callback_->callback(
            SD_RENDER_MODE_LINES,
            &vertices[0],
            vertices.size(),
            &indices[0],
            indices.size(),
            compile_callback_->user_data
        );

        new_character->set_geometry_handle(new_handle);
    }

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

void sdWorldAddBox(SDuint world_id, kmVec2* points) {
    World* world = get_world_by_id(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->add_box(points[0], points[1], points[2], points[3]);
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

void sdWorldStep(SDuint world_id, SDdouble dt) {
    World* world = get_world_by_id(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->update(dt);
}

SDuint64 sdWorldGetStepCounter(SDuint world_id) {
	World* world = get_world_by_id(world_id);
	return world->step_counter();
}

/**
 * Mainly for testing, constructs a loop out of triangles
 */
void sdWorldConstructLoop(SDuint world, SDdouble left, SDdouble top,
	SDdouble width) {

	SDdouble thickness = width * 0.1;
	SDdouble height = width;
	SDdouble radius = (width - (thickness * 2)) / 2.0;
		
	
	kmVec2 tmp;

	const SDuint slices = 40;

	//Generate the points of a circle
	std::vector<kmVec2> circle_points;	
	for(SDuint i = 0; i < slices; ++i) {
		SDdouble a = kmDegreesToRadians((360.0 / SDdouble(slices)) * (SDdouble)i);
		
		kmVec2Fill(&tmp, radius * cos(a), radius * sin(a));
		
		tmp.x += (left + radius) + thickness;
		tmp.y += (top - radius) - thickness;		
		circle_points.push_back(tmp);
	}
	
	//Now, build the surrounding triangles
	kmVec2 points[3];
	/*kmVec2Fill(&points[0], left, top - height); //Bottom left of loop
	kmVec2Fill(&points[1], left + width, top - height); //Bottom right of loop
	kmVec2Fill(&points[2], circle_points[0].x, circle_points[0].y);
	sdWorldAddTriangle(world, points);*/
		
	for(SDuint i = 0; i < slices / 4; ++i) {
		kmVec2Fill(&points[0], circle_points[i].x, circle_points[i].y);
		kmVec2Fill(&points[1], left + width, top); //Top right of loop		
		kmVec2Fill(&points[2], circle_points[i + 1].x, circle_points[i + 1].y);
		sdWorldAddTriangle(world, points);
	}
	
	for(SDuint i = slices / 4; i < ((slices / 4) * 2); ++i) {
		kmVec2Fill(&points[0], circle_points[i].x, circle_points[i].y);
		kmVec2Fill(&points[1], left, top); //Top left of loop		
		kmVec2Fill(&points[2], circle_points[i+1].x, circle_points[i+1].y);
		sdWorldAddTriangle(world, points);
	}	
	/*
	for(SDuint i = (slices / 4) * 2; i < ((slices / 4) * 3); ++i) {
		kmVec2Fill(&points[0], circle_points[i].x, circle_points[i].y);
		kmVec2Fill(&points[1], left, top - height); //Bottom right of the loop
		kmVec2Fill(&points[2], circle_points[i+1].x, circle_points[i+1].y);
		sdWorldAddTriangle(world, points);
	}*/		

	for(SDuint i = (slices / 4) * 3; i < slices ; ++i) {
		kmVec2Fill(&points[0], circle_points[i].x, circle_points[i].y);
		kmVec2Fill(&points[1], left + width, top - height); //Bottom right of the loop
		if(i < slices -1 ) {
			kmVec2Fill(&points[2], circle_points[i+1].x, circle_points[i+1].y);
		} else {
			kmVec2Fill(&points[2], circle_points[0].x, circle_points[0].y);
		}
		sdWorldAddTriangle(world, points);
	}			
}

void sdWorldSetCompileGeometryCallback(SDuint world_id, SDCompileGeometryCallback callback, void* data) {
    World* world = get_world_by_id(world_id);
    world->set_compile_callback(callback, data);
}

void sdWorldSetRenderGeometryCallback(SDuint world_id, SDRenderGeometryCallback callback, void* data) {
    World* world = get_world_by_id(world_id);
    world->set_render_callback(callback, data);
}

void sdWorldRender(SDuint world_id) {
    World* world = get_world_by_id(world_id);
    return world->render();
}

void sdWorldDebugEnable(SDuint world_id) {
	World* world = get_world_by_id(world_id);
	return world->enable_debug_mode();	
}

void sdWorldDebugStep(SDuint world_id, double step) {
	World* world = get_world_by_id(world_id);
	return world->debug_step(step);
}

void sdWorldDebugDisable(SDuint world_id) {
	World* world = get_world_by_id(world_id);
	return world->disable_debug_mode();		
}

SDbool sdWorldDebugIsEnabled(SDuint world_id) {
	World* world = get_world_by_id(world_id);
	return world->debug_mode_enabled();			
}
