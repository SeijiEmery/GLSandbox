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
    
class TriangleModule : public Module {
public:
    ~TriangleModule ();
    void drawFrame () override;
    
    static const char * MODULE_NAME;
    static const char * MODULE_DIR;
    
    static Module * construct (const ModuleConstructorArgs & args) {
        return static_cast<Module*>(new TriangleModule(args));
    }
private:
    TriangleModule (const ModuleConstructorArgs & args)
        : Module(args, MODULE_NAME, MODULE_DIR) { initModule(); }
    void initModule ();
    
    gl::Shader m_shader { "basic_shader" };
    gl::VAO    m_vao;
    gl::VBO    m_buffers [2];
};
    
}; // namespace gl_sandbox

#endif /* triangles_hpp */
