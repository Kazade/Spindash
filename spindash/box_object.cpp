
#include "box_object.h"
#include "collision/box.h"

BoxObject::BoxObject(World *world, float width, float height):
    Object(world) {

    set_geom(CollisionPrimitive::ptr(new Box(this, width, height)));
}
