#ifndef RAYBOX_H
#define RAYBOX_H

#include <map>

#include "kazmath/ray2.h"
#include "collision_primitive.h"

/**
    What is a Ray Box?
    
    A ray box is a primitive made up of 6 rays.
    
    It looks something like this:
    
       (C)   (D)
        |     |
        |     | 
        |--*--|
   (L)--|     |--(R)
        |     |
       (A)   (B)
    
    A ray box is used to represent a character. 
     * Rays C and D are used to detect ceilings
     * Rays L and R are used to detect walls, notice that they are
       below the position of the object. This is so that the character
       notices steps.
     * Rays A and B detect the floor
    
*/

class RayBox : public CollisionPrimitive {
public:
    typedef std::tr1::shared_ptr<RayBox> ptr;
    
    RayBox(float width, float height);    
    kmRay2& ray(char which);    
    
private:
    std::map<char, kmRay2> rays_;    
};

#endif
