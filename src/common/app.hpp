//
//  app.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef app_hpp
#define app_hpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "gl/gl_error.hpp"
#include "gl/gl_wrapper.hpp"

#include "camera.hpp"
#include "input.hpp"
#include "raii_signal.hpp"
#include "resources.hpp"
#include "../modules/modules.hpp"

#include <stdexcept>

namespace gl_sandbox {
    
struct InitializationError : public std::runtime_error {
    InitializationError (const char * msg) : std::runtime_error(msg) {}
};
    
struct AppEvents {
    raii::Signal<const IModule&> onModuleLoaded;
    raii::Signal<const IModule&> onModuleUnloaded;
    raii::Signal<const IModule&> onModuleChangedState;
    
    raii::Signal<float,float> onWindowResized;
    raii::Signal<>            onAppTerminated;
    
    typedef raii::Observer<const IModule&> ModuleObserver;
    typedef raii::Observer<float, float>   WindowResizedObserver;
    typedef raii::Observer<>               AppTerminationObserver;
};

class Application {
public:
    Application (const char * baseResourcePath);
    ~Application ();
    
    // Non-copyable, etc
    Application (const Application &) = delete;
    Application & operator= (const Application &) = delete;

    void run ();
    public:
    void glfw_keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods);
    void glfw_errorCallback (int error, const char * descr);
protected:
    GLFWwindow * m_mainWindow = nullptr;
    ModuleInterface m_modules;
    InputManager m_inputManager;
    AppEvents    m_appEvents;
    Camera       m_mainCamera;
    
protected:
    // Global state (required for modules to hook up to the event system, etc)
    // These are set at application load and cleared at application exit; they point to
    // memory owned by the active Application instance and are safe to use so long as
    // the usage lifetime <= Application lifetime (which should always be the case).
    //
    // Note: you can assume that these values are thread local, as they are intended
    // to be accessed from the main thread only. They are not currently defined as such
    // since apple has intentionally removed thread_local support from their libc++
    // implementation, and getting xcode to work with it would be a pita.
    // TLDR; Don't use these from other threads, as it fucks with our threading model.
    
    static Application  * g_applicationInstance;
    static InputManager * g_inputManager;
    static AppEvents    * g_appEvents;
public:
    static auto appInstance  () { return g_applicationInstance; }
    static auto inputManager () { return g_inputManager; }
    static auto appEvents    () { return g_appEvents; }
    static Camera*  mainCamera () { return &(appInstance()->m_mainCamera); }
};

}; // namespace gl_sandbox

#endif /* app_hpp */
