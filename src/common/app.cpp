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

// GLFW C Callbacks
static Application * g_activeApplication = nullptr; // hack
extern "C" {
    static void errorCallback (int error, const char * description) {
        if (g_activeApplication) {
            g_activeApplication->glfw_errorCallback(error, description);
        } else {
            std::cerr << description << '\n';
        }
    }
    static void keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods) {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->glfw_keyCallback(window, key, scancode, action, mods);
    }
}
//
// gl_sandbox::Application
//

InputManager * Application::g_inputManager = nullptr;
Application * Application::g_applicationInstance = nullptr;
AppEvents   * Application::g_appEvents = nullptr;

// Initialize glfw, create window, etc.
// Throws a std::runtime_error on failure to init.
Application::Application (const char * baseResourcePath)
    : m_modules()
{
    ResourceLoader::g_baseResourcePath = baseResourcePath;
    
    // Helper function
    auto initFail = [] (const char * msg, bool terminateGLFW = true) {
        g_activeApplication = nullptr;
        throw InitializationError(msg);
        return false;
    };
    
    g_activeApplication = this;
    
    // Init glfw, render context(s), and create window
    glfwInit() || initFail("Failed to initialize glfw\n", false);
    glfwSetErrorCallback(errorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    m_mainWindow = glfwCreateWindow(640, 480, "Hello world", NULL, NULL);
    m_mainWindow || initFail("Failed to create window\n");
    glfwSetWindowUserPointer(m_mainWindow, this);
    glfwSetKeyCallback(m_mainWindow, &keyCallback);
    
    glfwMakeContextCurrent(m_mainWindow);
    glfwSwapInterval(1);
    
    CHECK_GL_ERRORS();
    
    glewExperimental = true;
    glewInit();// || initFail("Failed to initialize glew\n");
    
    // glewInit() seems to always raise an error... which seems to be invalid (just a bug I guess?), so we'll ignore it
    while (glGetError()) {}
    // https://stackoverflow.com/questions/10857335/opengl-glgeterror-returns-invalid-enum-after-call-to-glewinit
    
//    CHECK_GL_ERRORS();
    
    glEnable(GL_DEPTH_TEST);
    
    std::cout << "GL Sandbox\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Opengl version: " << glGetString(GL_VERSION) << std::endl;
    
    Application::g_applicationInstance = this;
    Application::g_inputManager = &m_inputManager;
    Application::g_appEvents    = &m_appEvents;
    
    m_modules.loadModule("module-input-logger");
    m_modules.loadModule("module-flycam");
}

Application::~Application () {
    
    m_modules.killAllModules();
    
    g_activeApplication = nullptr; // disables application-specific error handling -- after this glfw errors are just directed to stderr (as opposed to log files, visual warnings, etc)
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
        static char windowTitleBuffer [256];
        snprintf(windowTitleBuffer, sizeof(windowTitleBuffer), "GL sandbox -- %0.2fms (%0.2f fps)", realAppRunTime / (double)appRunTimeSamples, currentFpsAvg);
        glfwSetWindowTitle(window, windowTitleBuffer);
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
        
//        for (const auto & camera : m_viewportCameras) {
//            glViewport(camera->viewport.x, camera.viewport.y, camera.viewport.width, camera.viewport.height);
//            glScissor(camera->viewport.x, camera.viewport.y, camera.viewport.width, camera.viewport.height);
//            m_modules.drawModules(*camera);
//        }
        
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






