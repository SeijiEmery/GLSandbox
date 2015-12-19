//
//  module.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef module_h
#define module_h

#include "../common/resources.hpp"
#include <string>

namespace gl_sandbox {
    
struct ModuleConstructorArgs {
    ResourceLoader * const resourceLoader;
    ModuleConstructorArgs (ResourceLoader * const resourceLoader)
        : resourceLoader (resourceLoader) {}
};

struct Module : public ResourceLoader::ModuleInterface<Module> {
    friend class ModuleInterface;

    virtual ~Module () {}
    virtual void drawFrame () = 0;
    
    std::string name;
    std::string dirName;
    
    const char * getDirName () const { return dirName.c_str(); }
protected:
    Module (const ModuleConstructorArgs & args, const char * name, const char * dirName)
        : ResourceLoader::ModuleInterface<Module>(args.resourceLoader), name(name), dirName(dirName) {}
public:
    // Utility methods, etc
    
    std::shared_ptr<gl::Shader> loadShader (const std::string & shaderName, std::function<void(gl::Shader&)> doLink) const {
        auto shader = std::make_shared<gl::Shader>(shaderName);
        
        auto fs = shaderName + ".fs";
        auto vs = shaderName + ".vs";
        
        loadTextFile(fs.c_str(), [&shader] (const char * src) {
            shader->compileFragment(src);
        });
        loadTextFile(vs.c_str(), [&shader] (const char * src) {
            shader->compileVertex(src);
        });
        doLink(*shader);
        if (shader->linkProgram()) {
            std::cout << "Successfully loaded shader '" << shader->name << "'\n";
        } else {
            std::cout << "Failed to laod shader '" << shader->name << "'\n";
        }
        return shader;
    }
    std::shared_ptr<gl::Shader> loadShader (const std::string & shaderName) const {
        return loadShader(shaderName, [](gl::Shader&) {});
    }
};
    
}; // namespace gl_sandbox



#endif /* module_h */
