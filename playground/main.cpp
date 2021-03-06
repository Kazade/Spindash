#include <kglt/kglt.h>

#include "spindash/spindash.h"
#include "spindash/character.h"

class KGLTGeometryRenderer {
public:
    KGLTGeometryRenderer(kglt::WindowBase& window, kglt::StageID stage_id=kglt::StageID()):
        window_(window),
        stage_id_(stage_id) {


        stage()->new_material_with_alias_from_file("diffuse_render", "kglt/materials/diffuse_render.kglm", false);

        auto mat = stage()->material(stage()->get_material_with_alias("diffuse_render"));
        mat->pass(0).set_polygon_mode(kglt::POLYGON_MODE_LINE);
    }

    SDGeometryHandle compile_geometry(SDRenderMode mode, SDVec2* vertices, SDuint numVertices, SDuint* indices, SDuint numIndices) {
        kglt::MeshID new_mesh = stage()->new_mesh();

        {
            auto mesh = stage()->mesh(new_mesh);
            auto submesh = mesh->new_submesh(
                stage()->get_material_with_alias("diffuse_render"),
                (mode == SD_RENDER_MODE_TRIANGLES) ? kglt::MESH_ARRANGEMENT_TRIANGLES : kglt::MESH_ARRANGEMENT_LINES, true
            );

            for(SDuint i = 0; i < numVertices; ++i) {
                mesh->shared_data().position(vertices[i]);
                mesh->shared_data().diffuse(kglt::Colour::WHITE);
                mesh->shared_data().move_next();
            }

            for(SDuint i = 0; i < numIndices; ++i) {
                mesh->submesh(submesh).index_data().index(indices[i]);
            }

            mesh->submesh(submesh).index_data().done();
            mesh->shared_data().done();

        }

        kglt::ActorID new_actor = stage()->new_actor(new_mesh);
        return new_actor.value();
    }

    void render_geometry(SDGeometryHandle handle, const SDVec2* translation, const SDfloat angle) {
        auto actor = stage()->actor(kglt::ActorID(handle));
        actor->move_to(translation->x, translation->y);
        actor->rotate_to(kglt::Degrees(angle));
    }

    static SDGeometryHandle compile_geometry_callback(SDRenderMode mode, SDVec2* vertices, SDuint numVertices, SDuint* indices, SDuint numIndexes, void* userData) {
        auto bound_func = std::bind(&KGLTGeometryRenderer::compile_geometry,
            (KGLTGeometryRenderer*) userData,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );

        return bound_func(mode, vertices, numVertices, indices, numIndexes);
    }

    static void render_geometry_callback(SDGeometryHandle handle, const SDVec2* translation, const SDfloat angle, void* userData) {
        auto bound_func = std::bind(&KGLTGeometryRenderer::render_geometry,
            (KGLTGeometryRenderer*) userData,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3
        );

        bound_func(handle, translation, angle);
    }

private:
    kglt::StagePtr stage() {
        return window_.stage(stage_id_);
    }

    kglt::WindowBase& window_;
    kglt::StageID stage_id_;

    std::map<SDGeometryHandle, kglt::MeshID> dynamic_meshes_;
    std::map<SDGeometryHandle, kglt::ActorID> dynamic_actors_;
};

void collision_response_thunk(SDuint lhs, SDuint rhs, CollisionResponse* cr1, CollisionResponse* cr2, void* user_data);

class Playground : public kglt::Application {
public:
    Playground() {
        renderer_.reset(new KGLTGeometryRenderer(window ));
    }

    bool do_init() {
        sdCharacterOverrideSetting("VERTICAL_SENSOR_EXTENSION_LENGTH", 16.0 / 40.0);

        world = sdWorldCreate();

        sdWorldSetCompileGeometryCallback(world, &KGLTGeometryRenderer::compile_geometry_callback, renderer_.get());
        sdWorldSetRenderGeometryCallback(world, &KGLTGeometryRenderer::render_geometry_callback, renderer_.get());

        build_world();

        window->camera->set_orthographic_projection(-10, 10, -10, 10);

        return true;
    }

    void do_step(double dt) {
        if(initialized()) {
            sdWorldStep(world, dt);
            sdWorldRender(world);
        }
    }

    void do_cleanup() {
        sdWorldDestroy(world);
    }

private:
    SDuint world = 0;
    SDuint sonic = 0;
    SDuint box = 0;
    SDuint spring2 = 0;

    std::shared_ptr<KGLTGeometryRenderer> renderer_;

    bool on_key_press(SDL_Keysym key) {
        if(key.scancode == SDL_SCANCODE_SPACE) {
            debug_break = true;
        }
        return false;
    }

    bool while_key_pressed(SDL_Keysym key, double) {
        switch(key.scancode) {
            case SDL_SCANCODE_RIGHT:
                sdCharacterRightPressed(sonic);
            break;
            case SDL_SCANCODE_LEFT:
                sdCharacterLeftPressed(sonic);
            break;
            case SDL_SCANCODE_DOWN:
                sdCharacterDownPressed(sonic);
            break;
            case SDL_SCANCODE_D:
                sdCharacterJumpPressed(sonic);
            break;
            default:break;
        }

        return false;
    }

    void create_floor_plane(SDuint world) {
        kmVec2 points[6];

        kmVec2Fill(&points[0], -10.0f, 0.0f);
        kmVec2Fill(&points[1], -10.0f, -1.0f);
        kmVec2Fill(&points[2], 10.0f, 0.0f);

        kmVec2Fill(&points[3], 10.0f, 0.0f);
        kmVec2Fill(&points[4], -10.0f, -1.0f);
        kmVec2Fill(&points[5], 10.0f, -1.0f);

        sdWorldAddMesh(world, 2, points);
    }

    void build_world() {
        sonic = sdCharacterCreate(world);

        sdObjectSetPosition(sonic, 3.0f, 3.0f);
        create_floor_plane(world);

        /*kmVec2 slope[3];
        kmVec2Fill(&slope[0], 3.0f, 0.0f);
        kmVec2Fill(&slope[1], 10.0f, 0.0f);
        kmVec2Fill(&slope[2], 10.0f, 3.0f);
        sdWorldAddTriangle(world, slope);*/

        kmVec2 wall[4];

        kmVec2Fill(&wall[0], -10.0f, 0.0f);
        kmVec2Fill(&wall[1], -5.0f, 0.0f);
        kmVec2Fill(&wall[2], -5.0f, 5.0f);
        kmVec2Fill(&wall[3], -10.0f, 5.0f);
        sdWorldAddBox(world, wall);

        /*kmVec2 platform[4];
        kmVec2Fill(&platform[0], 1.0f, 1.5f);
        kmVec2Fill(&platform[1], 3.0f, 1.5f);
        kmVec2Fill(&platform[2], 3.0f, 2.0f);
        kmVec2Fill(&platform[3], 1.0f, 2.0f);
        sdWorldAddBox(world, platform);*/

        box = sdBoxCreate(world, 1.0, 1.0);
        sdObjectSetPosition(box, -4.5f, 0.5f);


     /*   spring2 = sdSpringCreate(world, -45.0f, 10.0f/40.0f);
        sdObjectSetPosition(spring2, 2.0f, 2.25f);*/

        kmVec2 triangle[3];
        kmVec2Fill(&triangle[0], 5, 0);
        kmVec2Fill(&triangle[1], 10, 0);
        kmVec2Fill(&triangle[2], 10, 2);
        sdWorldAddTriangle(world, triangle);

        //sdWorldConstructLoop(world, 4.0f, 8.0f, 8.0f);

        sdWorldSetObjectCollisionCallback(world, &collision_response_thunk, this);
    }

public:
    void collision_callback(SDuint lhs, SDuint rhs, CollisionResponse* cr1, CollisionResponse* cr2) {
        static int bounce_count = 0;
        if(rhs == box && lhs == sonic) {
            if(bounce_count == 0) {
                *cr1 = COLLISION_RESPONSE_BOUNCE_ONE;
            } else if(bounce_count == 1) {
                *cr1 = COLLISION_RESPONSE_BOUNCE_TWO;
            } else {
                *cr1 = COLLISION_RESPONSE_BOUNCE_THREE;
            }

            bounce_count++;
        }
    }
};

void collision_response_thunk(SDuint lhs, SDuint rhs, CollisionResponse* cr1, CollisionResponse* cr2, void* user_data) {
    Playground* _this = (Playground*) user_data;
    _this->collision_callback(lhs, rhs, cr1, cr2);
}

int main(int argc, char* argv[]) {
    Playground app;
    return app.run();
}

