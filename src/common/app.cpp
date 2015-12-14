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

static void errorCallback (int error, const char * descr) {
    std::cerr << descr << '\n';
}
static void keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}


// Initialize glfw, create window, etc.
// Throws a std::runtime_error on failure to init.
Application::Application () {
    // Helper function
    auto initFail = [] (const char * msg, bool terminateGLFW = true) {
//        if (terminateGLFW) glfwTerminate();
        throw std::runtime_error(msg);
        return false;
    };
    
    // Init glfw, render context(s), and create window
    glfwInit() || initFail("Failed to initialize glfw\n", false);
    glfwSetErrorCallback(errorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    m_mainWindow = glfwCreateWindow(640, 480, "Hello world", NULL, NULL);
    m_mainWindow || initFail("Failed to create window\n");
    
    glfwMakeContextCurrent(m_mainWindow);
    glfwSwapInterval(1);
    
    glewExperimental = true;
    glewInit();// || initFail("Failed to initialize glew\n");
    
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Opengl version: " << glGetString(GL_VERSION) << std::endl;
    
//    initFail("Failed to success!");
}

Application::~Application () {
    if (m_mainWindow) {
        glfwDestroyWindow(m_mainWindow);
    }
    std::cout << "Killing glfw" << std::endl;
    glfwTerminate();
}

void Application::run () {
    while (!glfwWindowShouldClose(m_mainWindow))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(m_mainWindow, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(m_mainWindow);
        glfwPollEvents();
    }
}






