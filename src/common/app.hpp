//
//  app.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef app_hpp
#define app_hpp

#include "../modules/modules.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace gl_sandbox {
    
struct InitializationError : public std::runtime_error {
    InitializationError (const char * msg) : std::runtime_error(msg) {}
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
    ResourceLoader  m_resourceLoader;
    ModuleInterface m_modules;
};
    
}; // namespace gl_sandbox

#endif /* app_hpp */
