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
#include <memory>

namespace gl_sandbox {

class Application {
private:
    Application ();
    Application (const Application &) = delete;
    Application & operator= (const Application &) = delete;
    ~Application();
public:
    static Application & instance () {
        static Application instance;
        return instance;
    }
    void run ();
private:
    void glfw_keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods);
    void glfw_errorCallback (int error, const char * descr);
    
protected:
    GLFWwindow * m_mainWindow = nullptr;
};

    
}; // namespace gl_sandbox

#endif /* app_hpp */
