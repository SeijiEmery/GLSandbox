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
#include <boost/filesystem/path.hpp>

#include "script/lua_instance.hpp"
//#include "thread_worker.hpp"
//#include "js/js_thread_instance.hpp"
//#include "js/JSInstance.hpp"

#define OSX_CONF_PATH "/Users/semery/Library/Application Support/GLSandbox/conf.lua"
//#define OSX_CONF_PATH   "~/Library/Application Support/GLSandbox/conf.lua"
#define LINUX_CONF_PATH "~/.config/GLSandbox/conf.lua"
#define WINDOWS_CONF_PATH "<unimplemented>"

#define CONF_PATH OSX_CONF_PATH

#include <stdexcept>

namespace gl_sandbox {
    
struct InitializationError : public std::runtime_error {
    InitializationError (std::string msg) : std::runtime_error(msg) {}
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
    
namespace app_config {

struct Window {
    int width = 800, height = 600;
    std::string app_name = "GLSandbox";
    bool show_fps_counter = true;
    
    void loadConfig (LuaInstance &);
};

struct Resources {
    typedef boost::filesystem::path path;
    path project_dir;
    path asset_dir;
    path asset_cache_dir;
    
    path script_lib_src_dir;
    path script_lib_compiled_dir;
    path script_ui_src_dir;
    path script_ui_compiled_dir;
    path script_modules_src_dir;
    path script_modules_compiled_dir;
    
    path log_dir;
    path log_backup_dir;
    
    path persistent_data_dir;
    path persistent_data_backup_dir;
    
    void loadConfig (LuaInstance &);
};
    
struct AppConfig {
    typedef boost::filesystem::path path;
    path lua_conf_path { CONF_PATH };
    
    Window window;
    Resources resources;
    
    void loadConfig (LuaInstance &);
};
    
    
    
}; // namespace app_config
    

class Application {
public:
    Application ();
    ~Application ();
    
    // Non-copyable, etc
    Application (const Application &) = delete;
    Application & operator= (const Application &) = delete;

protected:
    void loadConfig ();
    void initGL ();
    void initDefaultModules ();
    
public:
    void run ();

    void glfw_keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods);
    void glfw_errorCallback (int error, const char * descr);
protected:
    app_config::AppConfig m_appConfig;
    
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
    static auto getInstance  () { return g_applicationInstance; }
    static auto getInputManager () { return g_inputManager; }
    static auto getAppEvents    () { return g_appEvents; }
    static Camera*  mainCamera () { return &(getInstance()->m_mainCamera); }
};

}; // namespace gl_sandbox

#endif /* app_hpp */
