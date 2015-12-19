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
    
struct IModule {
    const std::string name;
    
    virtual ~IModule (){};
    virtual void drawFrame () = 0;
protected:
    IModule (const char * name) : name(name) {}
};

template <typename T>
struct Module : public IModule {
protected:
    Module () : IModule(T::MODULE_NAME) {}
public:
    static Module * construct () {
        return static_cast<Module*>(new T());
    }
    
    // Utility methods, etc
//    std::shared_ptr<gl::Shader> loadShader (const std::string & shaderName, std::function<void(gl::Shader&)> doLink) const {
//        auto shader = std::make_shared<gl::Shader>(shaderName);
//        
//        auto fs = shaderName + ".fs";
//        auto vs = shaderName + ".vs";
//        
//        loadTextFile(fs.c_str(), [&shader] (const char * src) {
//            shader->compileFragment(src);
//        });
//        loadTextFile(vs.c_str(), [&shader] (const char * src) {
//            shader->compileVertex(src);
//        });
//        doLink(*shader);
//        if (shader->linkProgram()) {
//            std::cout << "Successfully loaded shader '" << shader->name << "'\n";
//        } else {
//            std::cout << "Failed to laod shader '" << shader->name << "'\n";
//        }
//        return shader;
//    }
//    std::shared_ptr<gl::Shader> loadShader (const std::string & shaderName) const {
//        return loadShader(shaderName, [](gl::Shader&) {});
//    }
};
    
}; // namespace gl_sandbox



#endif /* module_h */
