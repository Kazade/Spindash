#include <cassert>
#include "collide.h"

std::vector<Collision> do_collide(Triangle* triangle, RayBox* ray_box) {
    std::vector<Collision> collisions;
    
    for(char which: { 'A', 'B', 'C', 'D', 'L', 'R' }) {
        kmRay2& ray = ray_box->ray(which);
        
        kmVec2 intersection, normal;
        
        if(kmRay2IntersectTriangle(&ray, &triangle->points[0], 
                                         &triangle->points[1], 
                                         &triangle->points[2], 
                                         &intersection, &normal)) {
            Collision new_collision;
            kmVec2Normalize(&normal, &normal);
            new_collision.normal = normal;
            new_collision.point = intersection;
            new_collision.ray = which;
            collisions.push_back(new_collision);
        }
    }

    return collisions;
}
std::vector<Collision> do_collide(RayBox* ray_box, Triangle* triangle) { return do_collide(triangle, ray_box); }

std::vector<Collision> do_collide(RayBox* a, RayBox* b) {
    return std::vector<Collision>();
}

std::vector<Collision> collide(CollisionPrimitive* a, CollisionPrimitive* b) {
    /**
        Dynamic cast nastiness :(
    */

    if(RayBox* lhs = dynamic_cast<RayBox*>(a)) {
        if(RayBox* rhs = dynamic_cast<RayBox*>(b)) {
            return do_collide(lhs, rhs);
        } else if(Triangle* rhs = dynamic_cast<Triangle*>(b)) {
            return do_collide(lhs, rhs);
        } else {
            assert(0 && "Not implemented");
        }
    } else if (Triangle* lhs = dynamic_cast<Triangle*>(a)) {
        if(RayBox* rhs = dynamic_cast<RayBox*>(b)) {
            return do_collide(lhs, rhs);
        } else {
            assert(0 && "Not implemented");
        }        
    } else {
        assert(0 && "Not implemented");
    }        
}
