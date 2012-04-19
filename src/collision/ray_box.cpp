
#include "ray_box.h"
#include "kazmath/ray2.h"
#include "kazmath/mat3.h"

RayBox::RayBox(Object* owner, float width, float height):
    CollisionPrimitive(owner),
    x_(0.0f),
    y_(0.0f),
    width_(width),
    height_(height),
    degrees_(0.0f) {
    
    init();
}

kmRay2& RayBox::ray(char which) {
    //FIXME: assert(which in { 'A', 'B', 'C', 'D', 'L', 'R' })
    return rays_[which];
}

void RayBox::init() {
    //FIXME: Rotation!!
    //TODO: Fix all the ray positions to match the Sonic Physics Guide
    
    kmMat3 rotation;
    kmMat3RotationZ(&rotation, kmDegreesToRadians(degrees_));
    
    kmRay2& l = rays_['L'];
    kmVec2Fill(&l.start, x_, y_); //FIXME: Y-pos should be below center
    kmVec2Fill(&l.dir, -(width_/2), 0.0f);
    kmVec2Transform(&l.dir, &l.dir, &rotation);
    
    kmRay2& r = rays_['R'];
    kmVec2Fill(&r.start, x_, y_); //FIXME: Y-pos should be below center
    kmVec2Fill(&r.dir, (width_/2), 0.0f); 
    kmVec2Transform(&r.dir, &r.dir, &rotation);
    
    kmRay2& a = rays_['A'];
    kmVec2Fill(&a.start, -width_ / 2.0f, 0.0f); 
    kmVec2Fill(&a.dir, 0, -height_ / 2.0f);
    
    kmVec2Transform(&a.start, &a.start, &rotation);
    a.start.x += x_; a.start.y += y_;
    kmVec2Transform(&a.dir, &a.dir, &rotation);
    
    kmRay2& b = rays_['B'];
    kmVec2Fill(&b.start, width_ / 2.0f, 0.0f); 
    kmVec2Fill(&b.dir, 0, -height_ /2.0f);
    
    kmVec2Transform(&b.start, &b.start, &rotation);
    b.start.x += x_; b.start.y += y_;
    kmVec2Transform(&b.dir, &b.dir, &rotation);
        
    kmRay2& c = rays_['C'];
    kmVec2Fill(&c.start, -width_ / 2.0f, 0.0f); 
    kmVec2Fill(&c.dir, 0, height_ / 2.0f);
    
    kmVec2Transform(&c.start, &c.start, &rotation);
    c.start.x += x_; c.start.y += y_;
    kmVec2Transform(&c.dir, &c.dir, &rotation);
    
    kmRay2& d = rays_['D'];
    kmVec2Fill(&d.start, width_ / 2.0f, 0.0f); 
    kmVec2Fill(&d.dir, 0, height_ / 2.0f);
    
    kmVec2Transform(&d.start, &d.start, &rotation);
    d.start.x += x_; d.start.y += y_;
    kmVec2Transform(&d.dir, &d.dir, &rotation);
    
    //EXTRA RAYS FOR DETECTING THE GROUND!
    kmRay2& y = rays_['Y'];
    kmVec2Fill(&y.start, -width_ / 2.0f, 0.0f); 
    kmVec2Fill(&y.dir, 0, (-height_ / 2.0f) * 1.1f);
    
    kmVec2Transform(&y.start, &y.start, &rotation);
    y.start.x += x_; y.start.y += y_;
    kmVec2Transform(&y.dir, &y.dir, &rotation);
    
    kmRay2& z = rays_['Z'];
    kmVec2Fill(&z.start, width_ / 2.0f, 0.0f); 
    kmVec2Fill(&z.dir, 0, (-height_ /2.0f) * 1.1f);
    
    kmVec2Transform(&z.start, &z.start, &rotation);
    z.start.x += x_; z.start.y += y_;
    kmVec2Transform(&z.dir, &z.dir, &rotation);    
}

void RayBox::set_rotation(float degrees) {
    degrees_ = degrees;
    
    init();
}

void RayBox::set_position(float x, float y) {
    x_ = x;
    y_ = y;
    
    init();
}
