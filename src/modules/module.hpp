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
    ResourceLoader * resourceLoader;
    ModuleConstructorArgs (ResourceLoader * resourceLoader)
        : resourceLoader (resourceLoader) {}
};

struct Module {
    virtual ~Module () {}
    virtual void drawFrame () = 0;
    
//    virtual Module * createModule (const Application & app) = 0;
    
    std::string name;
    std::string dirName;
protected:
    Module (const ModuleConstructorArgs & args, const char * name, const char * dirName) : resourceLoader(args.resourceLoader), name(name), dirName(dirName) {}
    auto loadShader (const char * vertex_shader, const char * fragment_shader) {
        return resourceLoader->loadShader(*this, vertex_shader, fragment_shader);
    }
private:
    ResourceLoader * resourceLoader;
};
    
}; // namespace gl_sandbox



#endif /* module_h */
