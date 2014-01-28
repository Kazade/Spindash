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

#endif
