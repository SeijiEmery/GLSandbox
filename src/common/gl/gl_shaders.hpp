//
//  shaders.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef shaders_hpp
#define shaders_hpp

#include "gl_traits.hpp"
#include "gl_error.hpp"
#include <functional>
#include <sstream>
#include <cassert>
#include <vector>
#include <glm/glm.hpp>

namespace gl_sandbox {
namespace gl {

bool compileShader (gl::FragmentShader & shader, const char * src, std::function<void(const char *)> onError);
bool compileShader (gl::VertexShader & shader, const char * src, std::function<void(const char *)> onError);
bool linkShader (gl::ShaderProgram & program, const gl::VertexShader & vertexShader, const gl::FragmentShader & fragmentShader, std::function<void(const char *)> onError);

struct ShaderLoadError : public std::runtime_error {
    ShaderLoadError (const std::string & s) : std::runtime_error(s) {}
};

class Shader {
public:
    Shader (const char * name) : name(name) {}
protected:
    gl::ShaderProgram  _program;
    gl::FragmentShader _fs;
    gl::VertexShader   _vs;
public:
    std::string name;
protected:
    bool fragment_compiled = false;
    bool vertex_compiled = false;
    bool program_linked = false;
public:
    decltype(_program.handle) handle () const {
        return _program.handle;
    }
    typedef std::function<void(const ShaderLoadError &)> ErrorCallback;

    bool compileFragment (const char * src, ErrorCallback onError = dumpToStderr);
    bool compileVertex (const char * src, ErrorCallback onError = dumpToStderr);
    bool linkProgram (ErrorCallback onError = dumpToStderr);
    
    bool loaded () const {
        assert(program_linked ? fragment_compiled && vertex_compiled : true); // sanity check state flags
        return program_linked;
    }
    GLint getUniformLocation (const char * name) {
        auto loc = glGetUniformLocation(handle(), name);
        if (loc < 0)
            std::cerr << "Error: No uniform '" << name << "' in shader '" << this->name << "'\n";
        return loc;
    }
    void setUniform (GLint location, const glm::mat4x4 & m) {
        glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
    }
    void setUniform (GLint location, const glm::vec3 & v) {
        glUniform3fv(location, 1, &v[0]);
    }
    void setUniform (GLint location, const glm::vec4 & v) {
        glUniform4fv(location, 1, &v[0]);
    }
    void setUniform (GLint location, float s) {
        glUniform1f(location, s);
    }
    template <typename T>
    void setUniform (const char * name, const T & v) { setUniform(getUniformLocation(name), v); }
    
protected:
    // Default impl for the optional onError parameter on compileFragment, etc;
    // Just takes message (e.what) and prints to stdout (cerr).
    static void dumpToStderr (const ShaderLoadError & e);
};
    
}; // namespace gl
}; // namespace gl_sandbox



#endif /* shaders_hpp */
