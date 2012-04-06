
#include "ray_box.h"
#include "kazmath/ray2.h"

RayBox::RayBox(float width, float height) {
    //TODO: Fix all the ray positions to match the Sonic Physics Guide
    
    kmRay2& l = rays_['L'];
    kmVec2Fill(&l.start, 0.0f, 0.0f); //FIXME: Y-pos should be below center
    kmVec2Fill(&l.dir, -(width/2), 0.0f);
    
    kmRay2& r = rays_['R'];
    kmVec2Fill(&r.start, 0.0f, 0.0f); //FIXME: Y-pos should be below center
    kmVec2Fill(&r.dir, (width/2), 0.0f); 
    
    kmRay2& a = rays_['A'];
    kmVec2Fill(&a.start, -width / 2, 0.0f); //FIXME: Y-pos should be below center
    kmVec2Fill(&a.dir, -(width/2), -height/2.0f);
    
    kmRay2& b = rays_['B'];
    kmVec2Fill(&b.start, width / 2, 0.0f); //FIXME: Y-pos should be below center
    kmVec2Fill(&b.dir, (width/2), -height/2.0f);
    
    kmRay2& c = rays_['C'];
    kmVec2Fill(&c.start, -width / 2, 0.0f); //FIXME: Y-pos should be below center
    kmVec2Fill(&c.dir, -(width/2), height/2.0f);
    
    kmRay2& d = rays_['D'];
    kmVec2Fill(&d.start, width / 2, 0.0f); //FIXME: Y-pos should be below center
    kmVec2Fill(&d.dir, (width/2), height/2.0f);    
}

kmRay2& RayBox::ray(char which) {
    //FIXME: assert(which in { 'A', 'B', 'C', 'D', 'L', 'R' })
    return rays_[which];
}
