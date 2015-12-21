//
//  obj_viewer.hpp
//  GLSandbox
//
//  Created by semery on 12/20/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef obj_viewer_hpp
#define obj_viewer_hpp

#include "../module.hpp"
#include <glm/glm.hpp>

namespace gl_sandbox {
namespace modules {
    
class ObjViewer : public Module<ObjViewer> {
public:
    ObjViewer ();
    ~ObjViewer ();
    void drawFrame () override;
    
    static constexpr const char * MODULE_NAME = "module-objviewer";
    static constexpr const char * MODULE_DIR  = "obj_viewer";
    
    struct ModelInstance {
        gl::VAO m_vao;
        gl::VBO m_buffers [2];
        std::shared_ptr<gl::Shader> m_shader;
        
        ModelInstance ();
        ModelInstance (const ModelInstance &) = delete;
        void draw ();
    };
private:
    ResourceLoader m_resourceLoader { MODULE_NAME };
    std::vector<ModelInstance> m_modelInstances;
};
    
}; // namespace modules
}; // namespace gl_sandbox

#endif /* obj_viewer_hpp */
