#include "kazbase/logging.h"
#include "spindash.h"
#include "character.h"
#include "world.h"

void sdCharacterLeftPressed(SDuint character) {
    Character* c = Character::get(character);
    c->move_left();
}

void sdCharacterRightPressed(SDuint character) {
    Character* c = Character::get(character);
    c->move_right();
}

void sdCharacterUpPressed(SDuint character) {
    Character* c = Character::get(character);
    c->move_up();
}

void sdCharacterDownPressed(SDuint character) {
    Character* c = Character::get(character);
    c->move_down();
}

void sdCharacterJumpPressed(SDuint character) {
    Character* c = Character::get(character);
    c->jump();
}

SDDirection sdCharacterFacingDirection(SDuint character) {
    Character* c = Character::get(character);
    return c->facing();
}

SDAnimationState sdCharacterAnimationState(SDuint character) {
    Character* c = Character::get(character);
    return c->animation_state();
}

SDfloat sdCharacterGetWidth(SDuint character) {
    Character* c = Character::get(character);
    return c->width();
}

SDbool sdCharacterIsGrounded(SDuint character) {
    Character* c = Character::get(character);
    return c->is_grounded();
}

SDbool sdObjectIsCharacter(SDuint object) {
    Character* c = Character::get(object);
    return (c) ? true: false;
}

void sdCharacterSetGroundSpeed(SDuint character, SDfloat value) {
    Character* c = Character::get(character);
    c->set_ground_speed(value);
}

SDfloat sdCharacterGetGroundSpeed(SDuint character) {
    Character* c = Character::get(character);
    return c->ground_speed();
}

void sdCharacterEnableSkill(SDuint character, sdSkill skill) {
    Character* c = Character::get(character);
    c->enable_skill(skill);
}

void sdCharacterDisableSkill(SDuint character, sdSkill skill) {
    Character* c = Character::get(character);
    c->disable_skill(skill);
}

SDbool sdCharacterSkillEnabled(SDuint character, sdSkill skill) {
    Character* c = Character::get(character);
    return c->skill_enabled(skill);
}

SDfloat sdCharacterGetSpindashCharge(SDuint character) {
    Character* c = Character::get(character);
    return c->spindash_charge();
}

void sdCharacterOverrideSetting(const char* setting, float value) {
    Character::override_setting(setting, value);
}

void sdObjectDestroy(SDuint entity) {
    Object* obj = Object::get(entity);
    if(!obj) {
        L_WARN("sdObjectDestroy: No such object");
        return;
    }

    obj->world()->destroy_object(entity);
}

void sdObjectSetPosition(SDuint object, SDfloat x, SDfloat y) {
    Object* obj = Object::get(object);
    obj->set_position(x, y);
}

void sdObjectGetPosition(SDuint object, SDfloat* x, SDfloat* y) {
    Object* obj = Object::get(object);

    *x = obj->position().x;
    *y = obj->position().y;
}

SDfloat sdObjectGetPositionX(SDuint object) {
    Object* obj = Object::get(object);
    return obj->position().x;
}

SDfloat sdObjectGetPositionY(SDuint object) {
    Object* obj = Object::get(object);
    return obj->position().y;
}

SDfloat sdObjectGetSpeedX(SDuint object) {
    Object* obj = Object::get(object);
    return obj->velocity().x;
}

SDfloat sdObjectGetSpeedY(SDuint object) {
    Object* obj = Object::get(object);
    return obj->velocity().y;
}

void sdObjectSetSpeedX(SDuint object, SDfloat x) {
    Object* obj = Object::get(object);

    obj->set_velocity(x, obj->velocity().y);
}

void sdObjectSetSpeedY(SDuint object, SDfloat y) {
    Object* obj = Object::get(object);

    obj->set_velocity(obj->velocity().x, y);
}

SDfloat sdObjectGetRotation(SDuint object) {
    Object* obj = Object::get(object);
    return obj->rotation();
}

SDuint sdSpringCreate(SDuint world_id, SDfloat angle, SDfloat power) {
    World* world = World::get(world_id);
    return world->new_spring(power, angle);
}


/**
    @brief Creates a new physical world

    This function creates an empty world ready to start accepting
    new entities and polygons.
*/
SDuint sdWorldCreate() {
    return World::create();
}

/** \brief Destroys a world
 *
 * \param world - The world to destroy
 *
 * Destroys a world and its contents (polygons, entities etc.)
 */

void sdWorldDestroy(SDuint world) {
    World::destroy(world);
}

void sdWorldAddTriangle(SDuint world_id, kmVec2* points) {
    World* world = World::get(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->add_triangle(points[0], points[1], points[2]);
}

void sdWorldAddBox(SDuint world_id, kmVec2* points) {
    World* world = World::get(world_id);
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
    World* world = World::get(world_id);
    assert(world);
    world->remove_all_triangles();
}

void sdWorldStep(SDuint world_id, SDfloat dt) {
    World* world = World::get(world_id);
    if(!world) {
        //Log error
        return;
    }

    world->update(dt);
}

SDuint64 sdWorldGetStepCounter(SDuint world_id) {
    World* world = World::get(world_id);
    return world->step_counter();
}

/**
 * Mainly for testing, constructs a loop out of triangles
 */
void sdWorldConstructLoop(SDuint world, SDfloat left, SDfloat top,
    SDfloat width) {

    SDfloat thickness = width * 0.1;
    SDfloat height = width;
    SDfloat radius = (width - (thickness * 2)) / 2.0;


    kmVec2 tmp;

    const SDuint slices = 40;

    //Generate the points of a circle
    std::vector<kmVec2> circle_points;
    for(SDuint i = 0; i < slices; ++i) {
        SDfloat a = kmDegreesToRadians((360.0 / SDfloat(slices)) * (SDfloat)i);

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
    World* world = World::get(world_id);
    world->set_compile_callback(callback, data);
}

void sdWorldSetRenderGeometryCallback(SDuint world_id, SDRenderGeometryCallback callback, void* data) {
    World* world = World::get(world_id);
    world->set_render_callback(callback, data);
}

void sdWorldRender(SDuint world_id) {
    World* world = World::get(world_id);
    return world->render();
}

void sdWorldDebugEnable(SDuint world_id) {
    World* world = World::get(world_id);
    return world->enable_debug_mode();
}

void sdWorldDebugStep(SDuint world_id, double step) {
    World* world = World::get(world_id);
    return world->debug_step(step);
}

void sdWorldDebugDisable(SDuint world_id) {
    World* world = World::get(world_id);
    return world->disable_debug_mode();
}

SDbool sdWorldDebugIsEnabled(SDuint world_id) {
    World* world = World::get(world_id);
    return world->debug_mode_enabled();
}

void sdWorldCameraTarget(SDuint world_id, SDuint object) {
    World* world = World::get(world_id);
    world->set_camera_target(object);
}

void sdWorldCameraGetPosition(SDuint world_id, SDfloat* x, SDfloat* y) {
    World* world = World::get(world_id);
    const kmVec2& pos = world->camera_position();
    *x = pos.x;
    *y = pos.y;
}

