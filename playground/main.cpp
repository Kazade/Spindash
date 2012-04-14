
#include "SDL.h"
#include "SDL_opengl.h"

#include "spindash.h"

#include "kaztimer/kaztimer.h"

#define WIDTH  640
#define HEIGHT 480

SDuint world = 0;
SDuint sonic = 0;
SDuint spring = 0;

KTIuint timer;

static void repaint() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glTranslatef(0.0f, 0.0f, -15.0f);
    sdWorldDebugRenderGL(world);
    
    SDL_GL_SwapBuffers();
}

static void setup_sdl() {
    const SDL_VideoInfo* video;

    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
        
    /* Quit SDL properly on exit */
    atexit(SDL_Quit);

    /* Get the current video information */
    video = SDL_GetVideoInfo( );
    if( video == NULL ) {
        fprintf(stderr, "Couldn't get video information: %s\n", SDL_GetError());
        exit(1);
    }

    /* Set the minimum requirements for the OpenGL window */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    /* Note the SDL_DOUBLEBUF flag is not required to enable double 
     * buffering when setting an OpenGL video mode. 
     * Double buffering is enabled or disabled using the 
     * SDL_GL_DOUBLEBUFFER attribute.
     */
    if( SDL_SetVideoMode( WIDTH, HEIGHT, video->vfmt->BitsPerPixel, SDL_OPENGL ) == 0 ) {
        fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
        exit(1);
    }
}


static void setup_opengl()
{
    float aspect = (float)WIDTH / (float)HEIGHT;

    /* Make the viewport cover the whole window */
    glViewport(0, 0, WIDTH, HEIGHT);

    /* Set the camera projection matrix:
     * field of view: 90 degrees
     * near clipping plane at 0.1
     * far clipping plane at 100.0
     */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(60.0, aspect, 0.1, 100.0);
    /* We're done with the camera, now matrix operations 
     * will affect the modelview matrix
     * */
    glMatrixMode(GL_MODELVIEW);

    /* set the clear color to gray */
    glClearColor(0.5, 0.5 ,0.5, 0);
    
    /* We want z-buffer tests enabled*/
    glEnable(GL_DEPTH_TEST);

    /* Do draw back-facing polygons*/
    glDisable(GL_CULL_FACE);
}


static void main_loop() 
{
    SDL_Event event;

    while (1) {
        /* process pending events */
        while( SDL_PollEvent( &event ) ) {
            switch( event.type ) {
                case SDL_KEYDOWN:
                    switch ( event.key.keysym.sym ) {
                    case SDLK_ESCAPE:
                        exit(0);
                    break;
                    case SDLK_RIGHT:
                        sdCharacterStartMovingRight(sonic);
                    break;
                    case SDLK_LEFT:
                        sdCharacterStartMovingLeft(sonic);
                    break;
                    case SDLK_DOWN:
                        sdCharacterStartLookingDown(sonic);
                    break;
                    case SDLK_d:
                        sdCharacterStartJumping(sonic);
                    break;
                    default:
                         //no default key processing
                         //(stops compiler warnings for unhandled SDL keydefs
                         break;
                    }
                break;
                case SDL_KEYUP:
                    switch ( event.key.keysym.sym ) {
                        case SDLK_RIGHT:
                            sdCharacterStopMovingRight(sonic);
                        break;
                        case SDLK_LEFT:
                            sdCharacterStopMovingLeft(sonic);
                        break;
                        case SDLK_DOWN:
                            sdCharacterStopLookingDown(sonic);
                        break;
                        case SDLK_d:
                            sdCharacterStopJumping(sonic);
                        break;                        
                        default:
                            break;
                    }
                break;
                case SDL_QUIT:
                    exit (0);
                break;
            }
        }
        
        while(ktiTimerCanUpdate()) { 
            sdWorldStep(world, ktiGetDeltaTime());
        }

    /* update the screen */    
        repaint();
    /* Wait 50ms to avoid using up all the CPU time */
        SDL_Delay(1.0f / 60.0f);
        
        ktiUpdateFrameTime();
    }
}

static void create_floor_plane(SDuint world) {
    kmVec2 points[6];
    
    kmVec2Fill(&points[0], -10.0f, 0.0f);
    kmVec2Fill(&points[1], -10.0f, -1.0f);
    kmVec2Fill(&points[2], 10.0f, 0.0f); 

    kmVec2Fill(&points[3], 10.0f, 0.0f);  
    kmVec2Fill(&points[4], -10.0f, -1.0f);
    kmVec2Fill(&points[5], 10.0f, -1.0f);
                      
    sdWorldAddMesh(world, 2, points);
}

static void build_world() {
    world = sdWorldCreate();
    sonic = sdCharacterCreate(world);
    
    sdObjectSetPosition(sonic, 0.0f, 3.0f);
    create_floor_plane(world);
    
    kmVec2 slope[3];
    kmVec2Fill(&slope[0], 3.0f, 0.0f);
    kmVec2Fill(&slope[1], 10.0f, 0.0f);
    kmVec2Fill(&slope[2], 10.0f, 5.0f); 
    sdWorldAddTriangle(world, slope);
    
    kmVec2 wall[3];
    
    kmVec2Fill(&wall[0], -5.0f, 0.0f);
    kmVec2Fill(&wall[1], -5.0f, 5.0f);
    kmVec2Fill(&wall[2], -10.0f, 0.0f); 
    sdWorldAddTriangle(world, wall);    

    kmVec2Fill(&wall[0], -10.0f, 5.0f);
    kmVec2Fill(&wall[1], -10.0f, 0.0f); 
    kmVec2Fill(&wall[2], -5.0f, 5.0f);
    sdWorldAddTriangle(world, wall);    
    
    kmVec2 platform[3];
    kmVec2Fill(&platform[0], 1.0f, 1.5f);
    kmVec2Fill(&platform[1], 3.0f, 1.5f); 
    kmVec2Fill(&platform[2], 3.0f, 2.0f);
    sdWorldAddTriangle(world, platform);        

    kmVec2Fill(&platform[0], 1.0f, 1.5f);
    kmVec2Fill(&platform[1], 3.0f, 2.0f);
    kmVec2Fill(&platform[2], 1.0f, 2.0f);
    sdWorldAddTriangle(world, platform);        
    
    spring = sdSpringCreate(world, 0, 10.0f / 40.0f);
    sdObjectSetPosition(spring, -4.75f, 0.125f);
}

int main(int argc, char* argv[]) {
    ktiGenTimers(1, &timer);
    ktiBindTimer(timer);
    ktiStartFixedStepTimer(60);
    

    setup_sdl();
    setup_opengl();
    build_world();
    main_loop();
    return 0;
}

