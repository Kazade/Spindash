#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "kazphysics2.h"

int main(int argc, char** argv) {
    float gravity[2] = { 0.0f, 0.0f };

    KPuint world = kpCreateWorld();
    kpWorldParameterfv(world, KP_WORLD_GRAVITY, gravity);

    KPvec2 points[3] = { {-10.0f, 0.0f},
                          {10.0f, 0.0f},
                          {0.0f, -5.0f} };

    KPvec2 size = { 0.5f, 1.0f };
    KPvec2 ent_gravity = { 0.0f, -1.0f };

    kpWorldAddTriangle(world, points);

    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }


    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
    SDL_Surface* screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL);

    glViewport(0, 0, 640, 480);
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();							// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)640/(GLfloat)480,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);						// Select The Modelview Matrix
	glLoadIdentity();							// Reset The Modelview Matrix

    KPuint sonic = kpCreateEntity(world);
/*
    kpEntityParameteru(entity, KP_ENTITY_COLLISION_TYPE, KP_COLLISION_RAY_TYPE);
    kpEntityParameterfv(entity, KP_ENTITY_SIZE, size);
    kpEntityParameterfv(entity, KP_ENTITY_GRAVITY, ent_gravity);
*/

    bool running = true;

    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    running = false;
                break;
                default:
                break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -20.0f);

        kpWorldDebugRenderGL(world);

        SDL_GL_SwapBuffers();

    }

    kpDestroyWorld(world);
    return 0;
}
