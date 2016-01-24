//
//  app.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace gl_sandbox;

InputManager * Application::g_inputManager = nullptr;
Application  * Application::g_applicationInstance = nullptr;
AppEvents    * Application::g_appEvents = nullptr;
app_config::AppConfig * Application::g_appConfig = nullptr;

namespace glfw_callbacks {
extern "C" {
    static void errorCallback (int error, const char * description) {
        if (Application::getInstance()) {
            Application::getInstance()->glfw_errorCallback(error, description);
        } else {
            std::cerr << description << '\n';
        }
    }
    static void keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods) {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->glfw_keyCallback(window, key, scancode, action, mods);
    }
}
}; // namespace glfw_callbacks


namespace lua_conf_api {
extern "C" {

    static int join_paths (lua_State * L) {
        int n = lua_gettop(L);
        if (n != 2 || !lua_isstring(L, -1) || !lua_isstring(L, -2)) {
            std::string msg = "invalid arguments to path.join(";
            for (auto i = 1; i <= n; ++i) {
                if (i != 1)
                    msg += ", ";
                switch (lua_type(L, lua_type(L, i))) {
                    case LUA_TSTRING: msg += "string"; break;
                    case LUA_TTABLE:  msg += "table"; break;
                    case LUA_TTHREAD: msg += "thread"; break;
                    case LUA_TUSERDATA: msg += "userdata"; break;
                    case LUA_TNUMBER: msg += "number"; break;
                    case LUA_TNIL: msg += "nil"; break;
                    case LUA_TFUNCTION: msg += "function"; break;
                    case LUA_TLIGHTUSERDATA: msg += "lightuserdata"; break;
                    case LUA_TBOOLEAN: msg += "boolean"; break;
                    case LUA_TNONE: msg += "none";
                }
            }
            msg += ")";
            lua_pushstring(L, msg.c_str());
            lua_error(L);
            return 1;
        } else {
            using boost::filesystem::path;
            auto joined_path = path(lua_tostring(L, 1)) / path(lua_tostring(L, 2));
            lua_pop(L, 2);
            lua_pushstring(L, joined_path.string().c_str());
            return 1;
        }
    }
    static int print (lua_State * L) {
        auto printv = [L](int i) {
            switch (lua_type(L, i)) {
                case LUA_TNUMBER:
                case LUA_TSTRING: std::cout << lua_tostring(L, i); break;
                case LUA_TTABLE: std::cout << "<table>"; break;
                case LUA_TTHREAD: std::cout << "<thread>"; break;
                case LUA_TUSERDATA: std::cout << "<userdata>"; break;
                case LUA_TLIGHTUSERDATA: std::cout << "<lightuserdata>"; break;
                case LUA_TFUNCTION: std::cout << "<function>"; break;
                case LUA_TNIL: std::cout << "nil"; break;
                case LUA_TBOOLEAN: std::cout << (lua_toboolean(L, i) ? "true" : "false"); break;
            }
        };
        
        int n = lua_gettop(L);
        std::cout << "console.log: ";
        for (auto i = 1; i < n; ++i) {
            printv(i); std::cout << ", ";
        }
        if (n != 0) {
            printv(n); std::cout << '\n';
        }
        return 0;
    }
};
};

struct config_error : public std::runtime_error {
    config_error (std::string msg) : std::runtime_error(msg) {}
};


void Application::loadConfig() {
    using namespace boost::filesystem;
    if (!exists(m_appConfig.lua_conf_path)) {
        std::string reason = "Failed to load config file (file does not exist) '" + m_appConfig.lua_conf_path.string() + "'";
        throw InitializationError(reason);
    }
    boost::filesystem::ifstream f (m_appConfig.lua_conf_path);
    if (!f) {
        std::string reason = "Failed to load config file (failed to load file) '" + m_appConfig.lua_conf_path.string() + "'";
        throw InitializationError(reason);
    }
    f.seekg(0, std::ios::end);
    size_t size = f.tellg();
    if (size == 0) {
        std::string reason = "Failed to load config file (empty file) '" + m_appConfig.lua_conf_path.string() + "'";
        throw InitializationError(reason);
    }
    f.seekg(0, std::ios::beg);
    char * buffer = new char [size+1];
    f.read(buffer, size);
    buffer[size] = 0;
    f.close();
    
    LuaInstance lua ("config loader", false);
    
    // Setup API
    
    // Set logging function
    lua.newtable();
    lua.setfield("log", &lua_conf_api::print);
    lua.setglobal("console");
    
    // Get screen size
    int nmonitors;
    auto monitors = glfwGetMonitors(&nmonitors);
    int screen_width = 0, screen_height = 0;
    for (auto i = 0; i < nmonitors; ++i) {
        auto screen = glfwGetVideoMode(monitors[i]);
        if (screen->width * screen->height > screen_width * screen_height) {
            screen_width = screen->width;
            screen_height = screen->height;
        }
    }
    // Set screen table
    lua.newtable();
    lua.setfield("width", screen_width);
    lua.setfield("height", screen_height);
    lua.setglobal("screen");
    
    // Set path table
    lua.newtable();
    lua.setfield("concat", &lua_conf_api::join_paths);
    lua.setglobal("path");
    
    lua.run(buffer, size, m_appConfig.lua_conf_path.string().c_str());
    delete[] buffer;
    
    m_appConfig.loadConfig(lua);
}

std::string lua_type_to_string (lua_State * L, int i) {
    switch (lua_type(L, i)) {
        case LUA_TSTRING: return "string";
        case LUA_TNUMBER: return "number";
        case LUA_TBOOLEAN: return "boolean";
        case LUA_TFUNCTION: return "function";
        case LUA_TTABLE: return "table";
        case LUA_TTHREAD: return "thread";
        case LUA_TUSERDATA: return "userdata";
        case LUA_TLIGHTUSERDATA: return "lightuserdata";
        case LUA_TNIL: return "nil";
        case LUA_TNONE: return "none";
    }
    return "undefined_type";
}

void getField (lua_State * L, std::string var, const char * field, int & v, bool required = true, int i = -1) {
    lua_getfield(L, i, field);
    if (!lua_isnumber(L, -1)) {
        if (!lua_isnil(L, -1))
            std::cout << "warning: config field " << var << "." << field << " has unexpected type " << lua_type_to_string(L, -1) << '\n';
        if (required)
            throw config_error("missing field (number): " + var + "." + field);
    }
    v = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
}
void getField (lua_State * L, std::string var, const char * field, std::string & v, bool required = true, int i = -1) {
    lua_getfield(L, i, field);
    if (!lua_isstring(L, -1)) {
        if (!lua_isnil(L, -1))
            std::cout << "warning: config field " << var << "." << field << " has unexpected type " << lua_type_to_string(L, -1) << '\n';
        if (required)
            throw config_error("missing field (string): " + var + "." + field);
    }
    auto s = lua_tostring(L, -1);
    assert(s);
    v = s;
    lua_pop(L, 1);
}
void getField (lua_State * L, std::string var, const char * field, boost::filesystem::path & v, bool required = true, int i = -1) {
    lua_getfield(L, i, field);
    if (!lua_isstring(L, -1)) {
        if (!lua_isnil(L, -1))
            std::cout << "warning: config field " << var << "." << field << " has unexpected type " << lua_type_to_string(L, -1) << '\n';
        if (required)
            throw config_error("missing field (string): " + var + "." + field);
    }
    auto s = lua_tostring(L, -1);
    assert(s);
    v = boost::filesystem::path(s);
    lua_pop(L, 1);
}
void getField (lua_State * L, std::string var, const char * field, bool & v, bool required = true, int i = -1) {
    lua_getfield(L, i, field);
    if (!lua_isboolean(L, -1)) {
        if (!lua_isnil(L, -1))
            std::cout << "warning: config field " << var << "." << field << " has unexpected type " << lua_type_to_string(L, -1) << '\n';
        if (required)
            throw config_error("missing field (boolean): " + var + "." + field);
    }
    v = lua_toboolean(L, -1);
    lua_pop(L, 1);
}

void app_config::AppConfig::loadConfig(gl_sandbox::LuaInstance & lua) {
    try {
        window.loadConfig(lua);
        resources.loadConfig(lua);
    } catch (std::runtime_error e) {
        std::cerr << e.what() << '\n';
        throw config_error(e.what());
    }
}
void app_config::Window::loadConfig(gl_sandbox::LuaInstance & lua) {
    
    lua.assertValueIsType("window", LUA_TTABLE);
    lua.getVal("window.width", width);
    lua.getVal("window.height", height);
    lua.getVal("window.appname", app_name);
    lua.getVal("window.display_fps", show_fps_counter);
}
void app_config::Resources::loadConfig(gl_sandbox::LuaInstance & lua) {
    
    lua.assertValueIsType("resources", LUA_TTABLE);
    lua.getVal("resources.project_dir",                     project_dir);
    lua.getVal("resources.asset_dirs.root",                 asset_dir);
    lua.getVal("resources.asset_dirs.cached",               asset_cache_dir);
    lua.getVal("resources.script_dirs.lib_src",             script_lib_src_dir);
    lua.getVal("resources.script_dirs.lib_compiled",        script_lib_compiled_dir);
    lua.getVal("resources.script_dirs.ui_src",              script_ui_src_dir);
    lua.getVal("resources.script_dirs.ui_compiled",         script_ui_compiled_dir);
    lua.getVal("resources.script_dirs.modules_src",          script_modules_src_dir);
    lua.getVal("resources.script_dirs.modules_compiled",     script_modules_compiled_dir);
    
    lua.getVal("resources.log_dir", log_dir);
    lua.getVal("resources.backup_logs.dir", log_backup_dir);
    
    lua.getVal("resources.storage.persistent_data_dir", persistent_data_dir);
    lua.getVal("resources.storage.persistent_data_backups.dir", persistent_data_backup_dir);
}


Application::Application ()
    : m_modules()
{
    Application::g_applicationInstance = this;
    Application::g_inputManager = &m_inputManager;
    Application::g_appEvents = &m_appEvents;
    Application::g_appConfig = &m_appConfig;

    if (!glfwInit())
        throw InitializationError("Failed to initialize glfw\n");
    glfwSetErrorCallback(glfw_callbacks::errorCallback);
    
    loadConfig();
    
    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    m_mainWindow = glfwCreateWindow(
            m_appConfig.window.width,
            m_appConfig.window.height,
            m_appConfig.window.app_name.c_str(),
            nullptr, nullptr);
    if (!m_mainWindow)
        throw InitializationError("Failed to create glfw window\n");
    glfwSetWindowUserPointer(m_mainWindow, this);
    glfwSetKeyCallback(m_mainWindow, &glfw_callbacks::keyCallback);
    
    glfwMakeContextCurrent(m_mainWindow);
    glfwSwapInterval(1);
    
    initGL();
    initDefaultModules();
}

void Application::initGL() {
    CHECK_GL_ERRORS();
    glewExperimental = true;
    glewInit();
    
    // glewInit() seems to always raise an error... which seems to be invalid (just a bug I guess?), so we'll ignore it
    // https://stackoverflow.com/questions/10857335/opengl-glgeterror-returns-invalid-enum-after-call-to-glewinit
    while (glGetError()) {}
    
    glEnable(GL_DEPTH_TEST);
    
    std::cout << "GL Sandbox\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Opengl version: " << glGetString(GL_VERSION) << std::endl;
}

void Application::initDefaultModules () {
    m_modules.loadModule("module-input-logger");
    m_modules.loadModule("module-flycam");
    m_modules.loadModule("gamepad-input-test");
}

Application::~Application () {
    
    m_modules.killAllModules();
    if (m_mainWindow) {
        glfwDestroyWindow(m_mainWindow);
    }
    
    Application::g_applicationInstance = nullptr;
    Application::g_inputManager = nullptr;
    
    CHECK_GL_ERRORS();
    
    std::cout << "Killing glfw" << std::endl;
    glfwTerminate();
}


struct FPSWindowCounter {
    FPSWindowCounter () : lastTime(glfwGetTime() - UPDATE_FREQUENCY) {}
    
    void update (GLFWwindow * window) {
        double currentTime = glfwGetTime();
        samples += 1;
        if ((currentTime - lastTime) > UPDATE_FREQUENCY) {
            currentDtAvg = (currentTime - lastTime) / (double)samples;
            currentFpsAvg = 1.0 / currentDtAvg;
            samples = 0;
            lastTime = currentTime;
            updateWindow(window);
        }
    }
    void onFrameBegin () {
        frameStart = glfwGetTime();
    }
    void onFrameEnd () {
        realAppRunTime += glfwGetTime() - frameStart;
        ++appRunTimeSamples;
    }
protected:
    void updateWindow (GLFWwindow * window) {
        if (Application::getConfig().window.show_fps_counter) {
            static char windowTitleBuffer [256];
            snprintf(windowTitleBuffer, sizeof(windowTitleBuffer),
                     "%s %0.2fms (%0.2f fps)",
                     Application::getConfig().window.app_name.c_str(),
                     realAppRunTime / (double)appRunTimeSamples,
                     currentFpsAvg);
            glfwSetWindowTitle(window, windowTitleBuffer);
        } else {
            glfwSetWindowTitle(window, Application::getConfig().window.app_name.c_str());
        }
    }
protected:
    double lastTime;
    uint16_t samples = 0;
    static constexpr double UPDATE_FREQUENCY = 0.15; // Re-upate fps every 150 ms
    
    double realAppRunTime = 0;
    double frameStart = 0;
    uint16_t appRunTimeSamples = 0;
public:
    double currentFpsAvg = 0.0;
    double currentDtAvg  = 0.0;
};

void Application::run () {
    
    FPSWindowCounter counter;
    
    CHECK_GL_ERRORS();
    
    while (!glfwWindowShouldClose(m_mainWindow))
    {
        counter.update(m_mainWindow);
        counter.onFrameBegin();
        
        m_inputManager.update();
        
        float ratio;
        int width, height;
        glfwGetFramebufferSize(m_mainWindow, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);  CHECK_GL_ERRORS();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     CHECK_GL_ERRORS();
        
        m_modules.runModules();
        
        glfwSwapBuffers(m_mainWindow);    CHECK_GL_ERRORS();
        glfwPollEvents();
        
        counter.onFrameEnd();
    }
}

void Application::glfw_errorCallback(int error, const char *descr) {
    std::cerr << "glfw error: " << descr << '\n';
}
void Application::glfw_keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    static std::vector<std::tuple<int, std::string, bool>> toggleableModules {
    //    key toggle,     module id,      currently pressed
        { GLFW_KEY_T, "module-triangles", false },
        { GLFW_KEY_Y, "ubo-dynamic-test", false },
        { GLFW_KEY_U, "ubo-static-test",  false },
        { GLFW_KEY_O, "module-objviewer", false },
        { GLFW_KEY_I, "module-input-logger", false },
        { GLFW_KEY_J, "module-js-test", false },
    };
    
    // Toggle modules when specific keys are pressed.
    // Extra logic is to keep track of pressed keys, so we activate only on the edges of key down events, not constantly for however long the key(s) are pressed.
    for (auto & module : toggleableModules) {
        if (key == std::get<0>(module)) {
            if ((action == GLFW_PRESS) && !std::get<2>(module)) { // edge trigger -- key pressed
                std::get<2>(module) = true;
                // Toggle module
                auto moduleName = std::get<1>(module).c_str();
                if (m_modules.hasRunningModuleWithName(moduleName)) {
                    m_modules.unloadModule(moduleName);
                } else {
                    m_modules.loadModule(moduleName);
                }
            } else if (action == GLFW_RELEASE) { // edge trigger -- key released
                std::get<2>(module) = false;
            }
        }
    }
}






