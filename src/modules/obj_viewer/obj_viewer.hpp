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
    
    struct GPUMesh {
        gl::VAO vao;
        gl::VBO vertexBuffer;
        gl::VBO indexBuffer;
        ShaderRef shader;
    };
    
    struct ModelInstance {
        gl::VAO vao;
        gl::VBO buffers [3];
        ShaderRef shader;
        unsigned numIndices = 0;
        double startTime;
        struct {
            GLint LightPosition, Kd, Ld, ModelViewMatrix, NormalMatrix, ProjectionMatrix, MVP;
        } shaderUniforms;
        
        ModelInstance (const ResourceLoader::ObjData &, ShaderRef shader);
        ModelInstance (const ModelInstance &) = delete;
        ModelInstance (ModelInstance &&) = default;
        void draw ();
    };

protected:
    // Loads and/or caches shader w/ name
    ShaderRef loadShader (const std::string & shaderName);
    void loadModel (const std::string & modelName);
    void loadModelAsync (const std::string & modelName);
    
private:
    ResourceLoader m_resourceLoader { MODULE_NAME };
    
    std::vector<ModelInstance> m_modelInstances;
    std::unordered_map<std::string, ShaderRef> m_shaderCache;
};
    
}; // namespace modules
}; // namespace gl_sandbox

#endif /* obj_viewer_hpp */
