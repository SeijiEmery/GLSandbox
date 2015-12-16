//
//  resources.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef resources_hpp
#define resources_hpp

//#include "../modules/module.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <sstream>
#include <iostream>

namespace gl_sandbox {
    
struct ShaderHandle {
    typedef std::shared_ptr<ShaderHandle> Ptr;
    enum class State {
        LOADING,
        RESOURCE_ERROR,
        SHADER_COMPILATION_ERROR,
        SHADER_LINK_ERROR,
        OK
    } state = State::LOADING;
    
    ShaderHandle () {}
    ShaderHandle (State s) : state(s) {}
    ShaderHandle (GLuint handle) : state(State::OK), handle(handle) { assert(handle != 0); }
    
    GLuint handle = 0;
};
    
struct ResourceError : public std::runtime_error {
    using std::runtime_error::runtime_error; // constructor
};
    
    
    
class Module;
class ResourceLoader {
    boost::filesystem::path resourcePath;
    void executeAsync (std::function<void()>);
public:
    ResourceLoader (const char * resourcePath) :
        resourcePath(resourcePath) {}

    void loadTextFile (const boost::filesystem::path & filepath, std::function<void(const char*)> onComplete, std::function<void (const ResourceError &)> onError);
    void loadTextFile (const char * filename, std::function<void(const char*)> onComplete, std::function<void(const ResourceError &)> onError);
    void loadTextFileAsync (const char * filename, std::function<void(const char*)> onComplete, std::function<void(const ResourceError &)> onError);
    
    ShaderHandle::Ptr loadShader (const Module & module, const char * vertex_shader, const char * fragment_shader);
protected:
    template <typename... Args>
    void logResourceError(const char * fmt, Args... args);
};

}; // namespace gl_sandbox

#endif /* resources_hpp */
