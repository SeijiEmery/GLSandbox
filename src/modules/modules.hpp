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
#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

namespace gl_sandbox {
    
class ModuleInterface {
public:
    ModuleInterface ();
    ~ModuleInterface () {}
    
    void loadModule (const std::string & moduleName);
    void unloadModule (const std::string & moduleName);
    void runModules ();
    
    bool hasRunningModuleWithName (const std::string & moduleName) const;
    bool hasRunnableModuleWithName (const std::string & moduleName) const;
    
protected:
    void initModule (IModule * module);
    void deinitModule (IModule * module);
protected:
    typedef std::function<IModule*()> ModuleConstructor;
    
    std::vector<std::unique_ptr<IModule>> m_runningModules;
    std::unordered_map<std::string, ModuleConstructor> m_moduleConstructors;
};
    
}; // namespace gl_sandbox

#endif /* modules_hpp */
