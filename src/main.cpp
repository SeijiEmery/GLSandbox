//
//  main.cpp
//  GLSandbox
//
//  Created by semery on 12/8/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;

void errorCallback (int error, const char * descr) {
    cerr << descr << '\n';
}
void keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

int main(int argc, const char * argv[]) {
    auto exitWithMessage = [] (const char * msg, bool terminateGlfw = true) {
        fputs(msg, stderr);
        if (terminateGlfw)
            glfwTerminate();
        exit(EXIT_FAILURE);
        return false;
    };
    
    glfwInit() || exitWithMessage("Failed to initialize glfw\n", false);
    glfwSetErrorCallback(&errorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow * window = glfwCreateWindow(640, 480, "Hello world", NULL, NULL);
    if (!window)
        exitWithMessage("Failed to create window\n");
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    glewExperimental = true;
    glewInit();
    
    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "Opengl version: " << glGetString(GL_VERSION) << endl;
    
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    
    return glfwTerminate(), EXIT_SUCCESS;
}
