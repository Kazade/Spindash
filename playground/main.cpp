#include <kglt/kglt.h>

#include "spindash/spindash.h"

class Playground : public kglt::App {
public:
    bool do_init() {
        build_world();
        return true;
    }

    void do_step(double dt) {
        sdWorldStep(world, dt);
    }

    void do_cleanup() {
        sdWorldDestroy(world);
    }

private:
    SDuint world = 0;
    SDuint sonic = 0;
    SDuint spring = 0;
    SDuint spring2 = 0;

    bool on_key_release(SDL_Keysym key) override {
        switch(key.scancode) {
            case SDL_SCANCODE_RIGHT:
                sdCharacterStopMovingRight(sonic);
            break;
            case SDL_SCANCODE_LEFT:
                sdCharacterStopMovingLeft(sonic);
            break;
            case SDL_SCANCODE_DOWN:
                sdCharacterStopLookingDown(sonic);
            break;
            case SDL_SCANCODE_D:
                sdCharacterStopPressingJump(sonic);
            break;
            default:break;
        }

        return false;
    }

    bool on_key_press(SDL_Keysym key) override {
        switch(key.scancode) {
            case SDL_SCANCODE_RIGHT:
                sdCharacterStartMovingRight(sonic);
            break;
            case SDL_SCANCODE_LEFT:
                sdCharacterStartMovingLeft(sonic);
            break;
            case SDL_SCANCODE_DOWN:
                sdCharacterStartLookingDown(sonic);
            break;
            case SDL_SCANCODE_D:
                sdCharacterStartPressingJump(sonic);
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
        world = sdWorldCreate();
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

        spring = sdSpringCreate(world, 0, 10.0f / 40.0f);
        sdObjectSetPosition(spring, -4.75f, 0.125f);

     /*   spring2 = sdSpringCreate(world, -45.0f, 10.0f/40.0f);
        sdObjectSetPosition(spring2, 2.0f, 2.25f);*/

        sdWorldConstructLoop(world, 4.0f, 8.0f, 8.0f);
    }
};

int main(int argc, char* argv[]) {
    Playground app;
    return app.run();
}

