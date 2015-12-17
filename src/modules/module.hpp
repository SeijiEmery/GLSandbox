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

struct Module {
    friend class ModuleInterface;

    virtual ~Module () {}
    virtual void drawFrame () = 0;
    
//    virtual Module * createModule (const Application & app) = 0;
    
    std::string name;
    std::string dirName;
protected:
    Module (const ModuleConstructorArgs & args, const char * name, const char * dirName)
        : resourceLoader(args.resourceLoader), name(name), dirName(dirName) {}
protected:
    ResourceLoader * const resourceLoader; // owned by Application
};
    
}; // namespace gl_sandbox



#endif /* module_h */
