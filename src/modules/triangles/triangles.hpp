//
//  triangles.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef triangles_hpp
#define triangles_hpp

#include "../module.hpp"
#include "../../common/gl/gl_wrapper.hpp"
#include "../../common/input.hpp"

namespace gl_sandbox {
    
class TriangleModule : public Module<TriangleModule> {
public:
    TriangleModule ();
    ~TriangleModule ();
    void drawFrame () override;
    
    static constexpr const char* MODULE_NAME = "module-triangles";
    static constexpr const char* MODULE_DIR  = "triangles";
private:
    ResourceLoader m_resourceLoader { MODULE_DIR };
    gl::Shader m_shader { "basic_shader" };
    gl::VAO    m_vao;
    gl::VBO    m_buffers [3];
    double     m_startTime, m_lastTime;
    GLint      m_uniform_vp_matrix;
    GLint      m_uniform_rot_matrix;
    
    static constexpr double ROTATION_PERIOD = 2.5;
    static constexpr unsigned
        INSTANCE_ARRAY_WIDTH  = 50,
        INSTANCE_ARRAY_HEIGHT = 50,
        INSTANCE_ARRAY_DEPTH  = 50;
    static constexpr unsigned
        INSTANCE_ARRAY_SIZE = INSTANCE_ARRAY_WIDTH
        * INSTANCE_ARRAY_HEIGHT * INSTANCE_ARRAY_DEPTH;
    
    float m_cameraFov = 90.0;
    float m_cameraMinFov = 1.0;
    float m_cameraMaxFov = 178.0;
    float m_fovIncrement = 60.0;  // +60.0 deg / sec
    float m_fovDir = 0.0;
    bool  m_lbPressed = false;
    bool  m_rbPressed = false;
    InputManager::GamepadButtonObserver m_buttonObservers[2];
    
    double m_lastFovNotify = 0.0;
    double m_fovNotifyDelay = 0.5;
};
    
}; // namespace gl_sandbox

#endif /* triangles_hpp */
