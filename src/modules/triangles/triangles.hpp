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
    double     m_startTime;
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
};
    
}; // namespace gl_sandbox

#endif /* triangles_hpp */
