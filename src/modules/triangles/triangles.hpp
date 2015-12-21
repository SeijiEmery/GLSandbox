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
    gl::VBO    m_buffers [2];
    double     m_startTime;
    GLint      m_uniform_rotationMatrix;
    
    static constexpr double ROTATION_PERIOD = 2.5;
    
};
    
}; // namespace gl_sandbox

#endif /* triangles_hpp */
