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

// Initialize glfw, create window, etc.
// Throws a std::runtime_error on failure to init.
Application::Application (const char * baseResourcePath)
    : m_resourceLoader(baseResourcePath),
      m_modules(&m_resourceLoader)
{
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
    
    glewExperimental = true;
    glewInit();// || initFail("Failed to initialize glew\n");
    
    std::cout << "GL Sandbox\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Opengl version: " << glGetString(GL_VERSION) << std::endl;
}

Application::~Application () {
    g_activeApplication = nullptr; // disables application-specific error handling -- after this glfw errors are just directed to stderr (as opposed to log files, visual warnings, etc)
    if (m_mainWindow) {
        glfwDestroyWindow(m_mainWindow);
    }
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
protected:
    void updateWindow (GLFWwindow * window) {
        static char windowTitleBuffer [256];
        snprintf(windowTitleBuffer, sizeof(windowTitleBuffer), "GL sandbox -- %0.2fms (%0.2f fps)", currentDtAvg * 1000, currentFpsAvg);
        glfwSetWindowTitle(window, windowTitleBuffer);
    }
protected:
    double lastTime;
    uint16_t samples = 0;
    static constexpr double UPDATE_FREQUENCY = 0.15; // Re-upate fps every 150 ms
public:
    double currentFpsAvg = 0.0;
    double currentDtAvg  = 0.0;
};

void Application::run () {
    
    FPSWindowCounter counter;
    
    while (!glfwWindowShouldClose(m_mainWindow))
    {
        counter.update(m_mainWindow);
        
        float ratio;
        int width, height;
        glfwGetFramebufferSize(m_mainWindow, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        m_modules.runModules();
        
        glfwSwapBuffers(m_mainWindow);
        glfwPollEvents();
    }
}

void Application::glfw_errorCallback(int error, const char *descr) {
    std::cerr << "glfw error: " << descr << '\n';
}
void Application::glfw_keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_T && !m_modules.hasRunningModuleWithName("module-triangles")) {
        m_modules.loadModule("module-triangles");
    } else if (key == GLFW_KEY_Y && m_modules.hasRunningModuleWithName("module-triangles")) {
        m_modules.unloadModule("module-triangles");
    }
}






