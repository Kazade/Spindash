#include <cassert>
#include "collide.h"

#include "triangle.h"
#include "ray_box.h"
#include "box.h"

std::vector<Collision> do_collide(Triangle* triangle, RayBox* ray_box, bool swap_result=false) {
    std::vector<Collision> collisions;
    
    for(char which: { 'A', 'B', 'C', 'D', 'L', 'R', 'Y', 'Z' }) {
        kmRay2& ray = ray_box->ray(which);
        
        kmVec2 intersection, a_normal, b_normal, normal;
        kmScalar distance; 
        if(kmRay2IntersectTriangle(&ray, &triangle->points[0], 
                                         &triangle->points[1], 
                                         &triangle->points[2], 
                                         &intersection, &normal, &distance)) {
                                         
            if(distance <= kmVec2Length(&ray.dir)) {
                Collision new_collision;
                kmVec2Normalize(&a_normal, &normal);
                kmVec2Normalize(&b_normal, &ray.dir);
                
                new_collision.object_a = (swap_result) ? (CollisionPrimitive*)ray_box : (CollisionPrimitive*)triangle;
                new_collision.object_b = (swap_result) ? (CollisionPrimitive*)triangle : (CollisionPrimitive*)ray_box;
                
                new_collision.a_normal = (swap_result) ? b_normal : a_normal;
                new_collision.b_normal = (swap_result) ? a_normal : b_normal;
                new_collision.point = intersection;
                
                if(!swap_result){
                    new_collision.b_ray = which;
                } else {
                    new_collision.a_ray = which;
                }
                collisions.push_back(new_collision);
            }
        }
    }

    return collisions;
}
std::vector<Collision> do_collide(RayBox* ray_box, Triangle* triangle) { return do_collide(triangle, ray_box, true); }

//=================== Box - RayBox collisions ===========================

std::vector<Collision> do_collide(Box* box, RayBox* ray_box, bool swap_result=false) {
    std::vector<Collision> collisions;
    
    for(char which: { 'A', 'B', 'C', 'D', 'L', 'R' }) {
        kmRay2& ray = ray_box->ray(which);
        
        kmVec2 intersection, a_normal, b_normal, normal;
        
        if(kmRay2IntersectBox(
            &ray, &box->point(0), &box->point(1), 
            &box->point(2), &box->point(3),
            &intersection, &normal)) {
            
            Collision new_collision;
            kmVec2Normalize(&a_normal, &normal);
            kmVec2Normalize(&b_normal, &ray.dir);
            
            new_collision.object_a = (swap_result) ? (CollisionPrimitive*)ray_box : (CollisionPrimitive*)box;
            new_collision.object_b = (swap_result) ? (CollisionPrimitive*)box : (CollisionPrimitive*)ray_box;
            
            new_collision.a_normal = (swap_result) ? b_normal : a_normal;
            new_collision.b_normal = (swap_result) ? a_normal : b_normal;
            new_collision.point = intersection;
            
            if(!swap_result){
                new_collision.b_ray = which;
            } else {
                new_collision.a_ray = which;
            }
            collisions.push_back(new_collision);
        }
    }

    return collisions;
}
std::vector<Collision> do_collide(RayBox* ray_box, Box* box) { return do_collide(box, ray_box, true); }

//=================== Box - Triangle collisions =========================

std::vector<Collision> do_collide(Box* box, Triangle* triangle, bool swap_result=false) {
    return std::vector<Collision>();
}
std::vector<Collision> do_collide(Triangle* triangle, Box* box) { return do_collide(box, triangle, true); }

//=================== RayBox - RayBox collisions ========================
std::vector<Collision> do_collide(RayBox* a, RayBox* b) {
    return std::vector<Collision>();
}

//=================== Box - Box collisions ==============================
std::vector<Collision> do_collide(Box* a, Box* b) {
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
        } else if(Box* rhs = dynamic_cast<Box*>(b)) {
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
    } else if (Box* lhs = dynamic_cast<Box*>(a)) {
         if(RayBox* rhs = dynamic_cast<RayBox*>(b)) {
            return do_collide(lhs, rhs);
        } else if(Triangle* rhs = dynamic_cast<Triangle*>(b)) {
            return do_collide(lhs, rhs);
        } else if(Box* rhs = dynamic_cast<Box*>(b)) {
            return do_collide(lhs, rhs);
        } else {
            assert(0 && "Not implemented");
        }
    } else {
        assert(0 && "Not implemented");
    }        
}
