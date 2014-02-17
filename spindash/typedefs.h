#ifndef SD_TYPEDEFS_H
#define SD_TYPEDEFS_H

#include <cstdint>
#include <kazmath/kazmath.h>

typedef uint8_t SDbool;
typedef int32_t SDint;
typedef uint32_t SDuint;
typedef uint64_t SDuint64;

typedef float SDfloat;
typedef double SDdouble;

typedef SDuint SDGeometryHandle;

typedef kmVec2 SDVec2;

enum SDRenderMode {
    SD_RENDER_MODE_TRIANGLES,
    SD_RENDER_MODE_LINES
};

typedef SDGeometryHandle (*SDCompileGeometryCallback)(
    SDRenderMode render_mode, SDVec2* vertices, SDuint numVertices, SDuint* indices, SDuint numIndexes, void* userData
);

typedef void (*SDRenderGeometryCallback)(
    SDGeometryHandle handle, const SDVec2* translation, const SDfloat angle, void* userData
);

typedef enum AnimationState {
    ANIMATION_STATE_STANDING,
    ANIMATION_STATE_WALKING,
    ANIMATION_STATE_RUNNING,
    ANIMATION_STATE_DASHING,
    ANIMATION_STATE_ROLLING,
    ANIMATION_STATE_SPINDASH,
    ANIMATION_STATE_LOOKING_DOWN,
    ANIMATION_STATE_LOOKING_UP,
    ANIMATION_STATE_BRAKING
} AnimationState;

#endif
