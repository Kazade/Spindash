#ifndef COLLIDE_H
#define COLLIDE_H

#include "triangle.h"
#include "ray_box.h"

std::vector<Collision> collide(CollisionPrimitive* a, CollisionPrimitive* b);

#endif

