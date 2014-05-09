#ifndef KP_WORLD_H
#define KP_WORLD_H

#include <vector>
#include <memory>

#include "kazmath/kazmath.h"
#include "spindash.h"
#include "object.h"

#include "collision/triangle.h"
#include "collision/box.h"

const float DEFAULT_HORIZONTAL_FREEDOM_OF_MOVEMENT = (8.0 / 40.0);
const float DEFAULT_VERTICAL_FREEDOM_OF_MOVEMENT = (0 / 40.0);
const float DEFAULT_MAX_HORIZONTAL_CAMERA_SPEED = ((16.0 / 40.0) * 60.0);
const float DEFAULT_MAX_VERTICAL_CAMERA_SPEED = ((16.0 / 40.0) * 60.0);

typedef std::function<void (SDuint, SDuint, CollisionResponse*, CollisionResponse*)> InternalObjectCollisionCallback;

class World {
public:
    static World* get(SDuint world_id);
    static SDuint create();
    static void destroy(SDuint world_id);

    static SDuint world_id_counter_;

    World(SDuint id);
    ~World() {
        objects_.clear();
    }
    
    void set_gravity(float x, float y);
    kmVec2 gravity() const;

    void add_triangle(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3);
    void add_box(const kmVec2& v1, const kmVec2& v2, const kmVec2& v3, const kmVec2& v4);
    void remove_all_triangles() { triangles_.clear(); }
    
    ObjectID new_sphere();
    ObjectID new_box();
    ObjectID new_character();
    ObjectID new_spring(float angle, float power);
    
    void destroy_object(ObjectID object_id);

    void update(double step, bool override_step_mode=false);

    SDuint get_triangle_count() const { return triangles_.size(); }
    Triangle* get_triangle_at(SDuint i) { return &triangles_[i]; }
    
    SDuint get_box_count() const { return boxes_.size(); }
    Box* get_box_at(SDuint i) { return &boxes_.at(i); }
    
    uint64_t step_counter() const { return step_counter_; }
    
    bool debug_mode_enabled() const { return step_mode_enabled_; }
    void debug_step(double dt) { update(dt, true); }
    void enable_debug_mode() { step_mode_enabled_ = true; }
    void disable_debug_mode() { step_mode_enabled_ = false; }		
    
    void set_compile_callback(SDCompileGeometryCallback callback, void* user_data) {
        compile_callback_.reset(new CompileCallback);
        compile_callback_->callback = callback;
        compile_callback_->user_data = user_data;
    }

    void set_render_callback(SDRenderGeometryCallback callback, void* user_data) {
        render_callback_.reset(new RenderCallback);
        render_callback_->callback = callback;
        render_callback_->user_data = user_data;
    }

    void render();

    void set_camera_target(SDuint object_id);
    const kmVec2& camera_position() const { return camera_position_; }

    static std::map<ObjectID, Object*>& all_objects() {
        static std::map<ObjectID, Object*> objs;
        return objs;
    }


    void set_object_collision_callback(InternalObjectCollisionCallback callback) {
        object_collision_callback_ = callback;
    }

private:
    SDuint id_;
    kmVec2 gravity_;

    std::vector<Triangle> triangles_;
    std::vector<Box> boxes_;
    std::vector<Object::ptr> objects_;

    uint64_t step_counter_;
    
    bool step_mode_enabled_;

    struct CompileCallback {
        SDCompileGeometryCallback callback;
        void* user_data;
    };

    struct RenderCallback {
        SDRenderGeometryCallback callback;
        void* user_data;
    };

    std::shared_ptr<CompileCallback> compile_callback_;
    std::shared_ptr<RenderCallback> render_callback_;

    kmVec2 camera_position_;
    SDuint camera_target_ = 0;
    float camera_horizontal_fom_ = DEFAULT_HORIZONTAL_FREEDOM_OF_MOVEMENT;
    float camera_horizontal_max_speed_ = DEFAULT_MAX_HORIZONTAL_CAMERA_SPEED;
    float camera_vertical_fom_ = DEFAULT_VERTICAL_FREEDOM_OF_MOVEMENT;
    float camera_vertical_max_speed_ = DEFAULT_MAX_VERTICAL_CAMERA_SPEED;


    InternalObjectCollisionCallback object_collision_callback_;

    friend class Object;
};

#endif
