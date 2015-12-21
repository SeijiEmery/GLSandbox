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
    std::vector<std::future<void>> m_discardedAsyncResults;
    // drawback of std::async -- returns a future object, which joins the thread when its destructor
    // is called. Since we (obviously) don't want to JOIN THE THREAD on our async calls, we "fix" this
    // by stashing its result (nothing -- these are void returns) in a throwaway vector so its dtor
    // won't run until... oh, whenever the module gets unloaded...
    //
    // Oh, and uh, here's an interesting side effect of that: If the module gets reloaded _before_
    // the load finishes, it'll join the thread + hang the app until the load finishes (which is better
    // than doing something undefined b/c the module no longer exists in memory, I guess, but it means
    // stuff can/will get loaded while the module dtor is running >_< Oh well, cross that bridge when
    // we get there...
    
    ResourceLoader m_resourceLoader { MODULE_NAME };
    
    std::mutex m_modelInstances_mutex; // models can be loaded async
    std::vector<ModelInstance> m_modelInstances;
    std::unordered_map<std::string, ShaderRef> m_shaderCache;
};
    
}; // namespace modules
}; // namespace gl_sandbox

#endif /* obj_viewer_hpp */
