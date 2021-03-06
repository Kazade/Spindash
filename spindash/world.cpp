#include <iostream>
#include <cassert>
#include <map>
#include <functional>
#include <algorithm>
#include <tr1/functional>
#include <tr1/memory>

#include "kazbase/logging.h"
#include "collision/collide.h"
#include "kazmath/vec2.h"
#include "world.h"

#include "character.h"
#include "spring.h"
#include "box_object.h"

#include "spindash.h"

const kmVec2 GRAVITY_IN_MPS = { 0, (-0.21875 / 40.0) * 60.0};

SDuint World::world_id_counter_ = 0;

World::World(SDuint id):
    id_(id),
	step_counter_(0),
	step_mode_enabled_(false) {
    set_gravity(0.0f, GRAVITY_IN_MPS.y);
    
    kmVec2Fill(&camera_position_, 0, 0);
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
            render_callback_->callback(handle, &trans, angle, render_callback_->user_data);
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


void World::handle_collision_response(Object& obj, Object& other, CollisionResponse response_type, const std::vector<Collision>& collisions) {
    switch(response_type) {
    case COLLISION_RESPONSE_NONE: return;
    case COLLISION_RESPONSE_DEFAULT: return; //Handled elsewhere
    case COLLISION_RESPONSE_SPRING_LOW:
    case COLLISION_RESPONSE_SPRING_HIGH:
    case COLLISION_RESPONSE_SPRINGBOARD_LOW:
    case COLLISION_RESPONSE_SPRINGBOARD_HIGH:
    case COLLISION_RESPONSE_BALLOON: {
        float x = obj.velocity().x;
        float y = 7.0 / 40.0;
        obj.set_velocity(x, y);
    } break;
    case COLLISION_RESPONSE_BUMPER: {

    }
    case COLLISION_RESPONSE_SPRING_CAP: {

    } break;
    case COLLISION_RESPONSE_BOUNCE_ONE: {
        float x = obj.velocity().x;
        float y = 6.5 / 40.0;
        obj.set_velocity(x, y);
    } break;
    case COLLISION_RESPONSE_BOUNCE_TWO: {
        float x = obj.velocity().x;
        float y = 7.5 / 40.0;
        obj.set_velocity(x, y);
    } break;
    case COLLISION_RESPONSE_BOUNCE_THREE: {
        float x = obj.velocity().x;
        float y = 8.5 / 40.0;
        obj.set_velocity(x, y);
    } break;
    case COLLISION_RESPONSE_BREAKABLE_OBJECT: {
        float x = obj.velocity().x;
        float y = 3.0 / 40.0;
        obj.set_velocity(x, y);
    } break;
    case COLLISION_RESPONSE_REBOUND:
    case COLLISION_RESPONSE_HAZARD: {
        float x = float(sgn(obj.position().x - other.position().x)) / 40.0;
        float y = 4.0 / 40.0;
        obj.set_velocity(x, y);
    } break;
    case COLLISION_RESPONSE_DEATH: {
        float x = 0;
        float y = 7.0 / 40.0;
        obj.set_velocity(x, y);
    } break;
    case COLLISION_RESPONSE_POWER_UP:
    case COLLISION_RESPONSE_UNFIX:
        break;
    }
}

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

        lhs.update(step); //Move without responding to collisions

        //Now, process any Object vs Object collisions, these don't have to be recursive

        std::vector<uint32_t> objects_to_collide_with;

        for(uint32_t j = i + 1; j < objects_.size(); ++j) {
            Object& rhs = *objects_.at(j);
            std::vector<Collision> new_collisions = collide(&lhs.geom(), &rhs.geom());
            if(!new_collisions.empty()) {
                CollisionResponse cr1 = COLLISION_RESPONSE_DEFAULT;
                CollisionResponse cr2 = COLLISION_RESPONSE_DEFAULT;
                if(object_collision_callback_) {
                    object_collision_callback_(lhs.id(), rhs.id(), &cr1, &cr2);
                }

                if(cr1 != COLLISION_RESPONSE_DEFAULT) {
                    handle_collision_response(lhs, rhs, cr1, new_collisions);
                } else {
                    objects_to_collide_with.push_back(j);
                }

                if(cr2 == COLLISION_RESPONSE_DEFAULT) {

                } else {
                    handle_collision_response(rhs, lhs, cr2, new_collisions);
                }
            }
        }

        std::vector<Collision> collisions;

        uint32_t tries = 10;
        while(run_loop && tries--) {
            //FIXME: We only need to test stuff within a certain radius...

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

            //FIXME: this doesn't seem right :/ not sure how to handle object collisions really...
            //Here we collide with all things that above we hit and decided to deal with as a normal collision
            //The problem is that this only works for one side of the collision.. what about when the other side returns
            //that it wants a default response?
            for(uint32_t j: objects_to_collide_with) {
                Object& rhs = *objects_.at(j);
                std::vector<Collision> new_collisions = collide(&lhs.geom(), &rhs.geom());
                if(!new_collisions.empty()) {
                    collisions.insert(collisions.end(), new_collisions.begin(), new_collisions.end());
                }
            }
                        
            run_loop = lhs.respond_to(collisions);
            collisions.clear();

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
        
    //Update the camera
    if(camera_target_) {
        kmVec2 target_position;
        sdObjectGetPosition(camera_target_, &target_position.x, &target_position.y);

        float x_movement = 0;
        float y_movement = 0;

        if(target_position.x > camera_position_.x) {
            x_movement = target_position.x - (camera_position_.x + camera_horizontal_fom_);
            if(x_movement < 0) x_movement = 0;
        } else if(target_position.x < camera_position_.x) {
            x_movement = target_position.x - (camera_position_.x - camera_horizontal_fom_);
            if(x_movement > 0) x_movement = 0;
        }

        if(target_position.y > camera_position_.y) {
            y_movement = target_position.y - (camera_position_.y + camera_vertical_fom_);
            if(y_movement < 0) y_movement = 0;
        } else if(target_position.y < camera_position_.y) {
            y_movement = target_position.y - (camera_position_.y - camera_vertical_fom_);
            if(y_movement > 0) y_movement = 0;
        }


        float max_x_movement = camera_horizontal_max_speed_ * step;
        float max_y_movement = camera_vertical_max_speed_ * step;

        if(fabs(x_movement) > max_x_movement) {
            x_movement = max_x_movement * sgn(x_movement);
        }

        if(fabs(y_movement) > max_y_movement) {
            y_movement = max_y_movement * sgn(y_movement);
        }

        camera_position_.x += x_movement;
        camera_position_.y += y_movement;
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
    Object* obj = Object::get(object_id);
    objects_.erase(std::remove_if(objects_.begin(), objects_.end(), PointerCompare(obj)), objects_.end());

    auto it = all_objects().find(obj->id());
    if(it != all_objects().end()) {
        all_objects().erase(it);
    }

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

ObjectID World::new_box(float width, float height) {
    BoxObject::ptr new_box(new BoxObject(this, width, height));

    //TODO: Compile for rendering

    std::vector<SDVec2> vertices;
    SDVec2 tmp;
    float hw = width * 0.5;
    float hh = height * 0.5;

    kmVec2Fill(&tmp, -hw, -hh);
    vertices.push_back(tmp);

    kmVec2Fill(&tmp, hw, -hh);
    vertices.push_back(tmp);

    kmVec2Fill(&tmp, hw, hh);
    vertices.push_back(tmp);

    kmVec2Fill(&tmp, -hw, hh);
    vertices.push_back(tmp);


    std::vector<SDuint> indices = { 0, 1, 1, 2, 2, 3, 3, 0 };

    if(compile_callback_) {
        SDGeometryHandle new_handle = compile_callback_->callback(
            SD_RENDER_MODE_LINES,
            &vertices[0],
            vertices.size(),
            &indices[0],
            indices.size(),
            compile_callback_->user_data
        );

        new_box->set_geometry_handle(new_handle);
    }

    objects_.push_back(new_box);
    return new_box->id();
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

void World::set_camera_target(SDuint object_id) {
    Object* obj = Object::get(object_id);
    assert(obj);

    camera_target_ = object_id;
    camera_position_ = obj->position();
}

//=============================================================

static std::map<SDuint, std::tr1::shared_ptr<World> > worlds_;

SDuint World::create() {
    SDuint new_id = ++world_id_counter_;
    worlds_[new_id].reset(new World(new_id));
    return new_id;
}

void World::destroy(SDuint world_id) {
    if(worlds_.find(world_id) == worlds_.end()) {
        L_WARN("Tried to destroy a non-existent world");
        return;
    }

    worlds_.erase(world_id);
}

World* World::get(SDuint world) {
    if(worlds_.find(world) == worlds_.end()) {
        return NULL;
    }

    return worlds_[world].get();
}
