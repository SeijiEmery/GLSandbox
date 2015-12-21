//
//  modules.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "triangles/triangles.hpp"
#include "ubo_test/ubo_test.hpp"
#include "obj_viewer/obj_viewer.hpp"

#include "modules.hpp"
#include "app.hpp"

#include <iostream>

using namespace gl_sandbox;
using namespace gl_sandbox::modules;

//#define CONSTRUCTABLE_MODULE(cls) \
Module_metaclass { cls::MODULE_NAME, [&]() { return cls::construct(m_sharedModuleArgs); } }

#define ADD_MODULE(cls) { std::string { cls::MODULE_NAME }, [](){ return cls::construct(); } }


//Module_metaclass { cls::MODULE_NAME, []() { return static_cast<Module*>(new cls()); } }

ModuleInterface::ModuleInterface () :
    m_moduleConstructors {
        ADD_MODULE(TriangleModule),
        ADD_MODULE(UboDynamicModule),
        ADD_MODULE(UboStaticModule),
        ADD_MODULE(ObjViewer),
    }
{
    // Do other initialization...
}
#undef ADD_MODULE

void ModuleInterface::initModule (IModule * module) {
    // Can add functionality later..
    assert(module != nullptr);
}
void ModuleInterface::deinitModule (IModule * module) {
    // Do nothing for now (can add functionality as needed)
    assert(module != nullptr);
}

bool ModuleInterface::hasRunnableModuleWithName(const std::string & moduleName) const {
    auto x = m_moduleConstructors.find(moduleName);
    return x != m_moduleConstructors.end();
}
bool ModuleInterface::hasRunningModuleWithName(const std::string & moduleName) const {
    for (const auto & module : m_runningModules)
        if (module->name == moduleName)
            return true;
    return false;
}


void ModuleInterface::loadModule(const std::string & moduleName) {
    // Check if module is already loaded. If so, do nothing.
    if (hasRunningModuleWithName(moduleName)) {
        std::cout << "Already running module '" << moduleName << "'\n";
        return;
    }
    
    // Otherwise, find constructable module with a matching name.
    // If no such module exists, log an error.
    auto x = m_moduleConstructors.find(moduleName);
    if (x != m_moduleConstructors.end()) {
        auto & constructor = x->second;
        auto newModule = constructor();
        m_runningModules.emplace_back(newModule);
        return;
    }
    std::cerr << "Cannot load module -- no module registered as '" << moduleName << "'\n";
}

void ModuleInterface::unloadModule (const std::string & moduleName) {
    // Find module and remove
//    std::remove_if(m_runningModules.begin(), m_runningModules.end(),
//                   [&moduleName](const std::unique_ptr<Module> & module) {
//        return module->name == moduleName;
//    });
    
    // wth, we can just implement this manually via swap-delete instead of relying on compiler optimizations to (hopefully) produce something like this:
    for (auto i = m_runningModules.size(); i > 0; --i) {
        if (m_runningModules[i-1]->name == moduleName) {
            deinitModule(m_runningModules[i-1].get());
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

