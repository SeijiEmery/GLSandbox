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
    std::function<Module*()> construct;
    
    Module_metaclass (const char * name, const decltype(construct) & construct) :
        name(name), construct(construct) {}
    Module_metaclass (Module_metaclass && other) :
        name(std::move(other.name)), construct(std::move(other.construct)) {}
    Module_metaclass (const Module_metaclass & other) :
        name(other.name), construct(other.construct) {}
};

class ModuleInterface {
public:
    ModuleInterface (ResourceLoader * loader);
    ~ModuleInterface () {}
    
    void loadModule (const char * moduleName);
    void unloadModule (const char * moduleName);
    void runModules ();
    
    bool hasRunningModuleWithName (const char * moduleName);
    bool hasRunnableModuleWithName (const char * moduleName);
protected:
    std::vector<std::unique_ptr<Module>> m_runningModules;
    std::vector<Module_metaclass> m_runnableModules;
    
    ModuleConstructorArgs moduleArgs;
};
    
}; // namespace gl_sandbox

#endif /* modules_hpp */
