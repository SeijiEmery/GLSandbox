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
#include <unordered_map>
#include <mutex>
#include <future>

namespace gl_sandbox {
namespace modules {
    
class ObjViewer : public Module<ObjViewer> {
public:
    ObjViewer ();
    ~ObjViewer ();
    void drawFrame () override;
    
    static constexpr const char * MODULE_NAME = "module-objviewer";
    static constexpr const char * MODULE_DIR  = "obj_viewer";
    
    typedef std::shared_ptr<gl::Shader> ShaderRef;
    
    struct ModelInstance {
        gl::VAO m_vao;
        gl::VBO m_buffers [2];
        ShaderRef m_shader;
        
        ModelInstance ();
        ModelInstance (const ModelInstance &) = delete;
        void draw ();
    };

protected:
    // Loads and/or caches shader w/ name
    ShaderRef loadShader (const std::string & shaderName);
    void loadModel (const std::string & modelName);
    void loadModelAsync (const std::string & modelName);
    
private:
    ResourceLoader m_resourceLoader { MODULE_NAME };
    
    std::mutex m_modelInstances_mutex; // models can be loaded async
    std::vector<ModelInstance> m_modelInstances;
    std::unordered_map<std::string, ShaderRef> m_shaderCache;
    
    std::vector<std::future<void>> m_discardedAsyncResults;
    // drawback of std::async -- returns a future object, which joins the thread when its destructor,
    // is called. Since we (obviously) don't want to do that for our async calls, we can (sorta)
    // safely "throw away" the result by stashing it in a vector...
    // oh god this approach is terrible >_<
};
    
}; // namespace modules
}; // namespace gl_sandbox

#endif /* obj_viewer_hpp */
