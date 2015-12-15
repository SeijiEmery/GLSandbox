//
//  modules.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "triangles/triangles.hpp"

#include "modules.hpp"

#include <iostream>

using namespace gl_sandbox;

#define CONSTRUCTABLE_MODULE(cls) \
Module_metaclass { cls::MODULE_NAME, []() { return std::unique_ptr<Module>(static_cast<Module*>(new cls())); } }

ModuleInterface::ModuleInterface () : m_runnableModules {
    // List of constructable modules gets defined here
    CONSTRUCTABLE_MODULE(TriangleModule),
//    CONSTRUCTABLE_MODULE(TriangleModule)
} {
    // Do other initialization...
}
                                                        
#undef CONSTRUCTABLE_MODULE

bool ModuleInterface::hasRunnableModuleWithName(const char * moduleName) {
    for (const auto & module : m_runnableModules)
        if (module.name == moduleName)
            return true;
    return false;
}
bool ModuleInterface::hasRunningModuleWithName(const char * moduleName) {
    for (const auto & module : m_runningModules)
        if (module->name == moduleName)
            return true;
    return false;
}


void ModuleInterface::loadModule(const char * moduleName) {
    // Check if module is already loaded. If so, do nothing.
    if (hasRunningModuleWithName(moduleName)) {
        std::cout << "Already running module '" << moduleName << "'\n";
        return;
    }
    
    // Otherwise, find constructable module with a matching name.
    // If no such module exists, log an error.
    for (auto i = 0; i < m_runnableModules.size(); ++i) {
        if (m_runnableModules[i].name == moduleName) {
            auto newModule = m_runnableModules[i].constructor();
            m_runnableModules.emplace_back(std::move(newModule));
            return;
        }
    }
    
    std::cerr << "Cannot load module -- no module registered as '" << moduleName << "'\n";
}

void ModuleInterface::unloadModule (const char * moduleName) {
    // Find module and remove
//    std::remove_if(m_runningModules.begin(), m_runningModules.end(),
//                   [&moduleName](const std::unique_ptr<Module> & module) {
//        return module->name == moduleName;
//    });
    
    // wth, we can just implement this manually via swap-delete instead of relying on compiler optimizations to (hopefully) produce something like this:
    for (auto i = m_runningModules.size(); i > 0; --i) {
        if (m_runningModules[i-1]->name == moduleName) {
            if (i != m_runningModules.size())                 // <- this branch can/should be moved outside when the comipler loop unrolls
                m_runningModules[i-1] = std::move(m_runningModules.back());
            m_runningModules.pop_back();
            return;
        }
    }
    
    // If module isn't loaded (unloading module w/ name failed), issue warnings whether we're
    // a) trying to unload something that hasn't been / isn't loaded, or
    // b) trying to unload something that doen't exist / hasn't been registered
    if (hasRunnableModuleWithName(moduleName)) {
        std::cerr << "Warning: can't unload module (Module '" << moduleName << "' exists but is not currently loaded)\n";
    } else {
        std::cerr << "Warning: can't unload module (Unknown module '" << moduleName << "')\n";
    }
}

void ModuleInterface::runModules() {
    for (auto & module : m_runningModules) {
        module->drawFrame();
    }
}













