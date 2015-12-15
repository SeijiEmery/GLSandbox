//
//  modules.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef modules_hpp
#define modules_hpp

#include "module.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace gl_sandbox {
    
struct Module_metaclass {
    std::string name;
    std::function<std::unique_ptr<Module>()> constructor;
    
    Module_metaclass (const char * name, const decltype(constructor) & constructor) :
        name(name), constructor(constructor) {}
};

class ModuleInterface {
public:
    ModuleInterface ();
    ~ModuleInterface () {}
    
    void loadModule (const char * moduleName);
    void unloadModule (const char * moduleName);
    void runModules ();
    
    bool hasRunningModuleWithName (const char * moduleName);
    bool hasRunnableModuleWithName (const char * moduleName);
protected:
    std::vector<std::unique_ptr<Module>> m_runningModules;
    std::vector<Module_metaclass> m_runnableModules;
};
    
}; // namespace gl_sandbox

#endif /* modules_hpp */
