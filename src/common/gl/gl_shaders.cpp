//
//  shaders.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "gl_shaders.hpp"
#include <boost/format.hpp>
#include <iostream>

using namespace gl_sandbox::gl;

namespace impl {
template <typename T>
static bool compileShader (T & shader, const char * src, std::function<void(const char *)> onError) {
    const char * srcs [] = { src };
    glShaderSource(shader.handle, 1, srcs, NULL); CHECK_GL_ERRORS();
    glCompileShader(shader.handle);              CHECK_GL_ERRORS();
    
    GLint result;
    glGetShaderiv(shader.handle, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        GLint size;
        glGetShaderiv(shader.handle, GL_INFO_LOG_LENGTH, &size);
        if (size > 0) {
            char * log = new char [size];
            GLsizei written;
            glGetShaderInfoLog(shader.handle, size, &written, log);
            onError(log);
            delete[] log;
            return false;
        } else {
            return onError("Failed to compile shader\n"), false;
        }
    }
    return true;
}
}; // namespace impl

bool compileShader (FragmentShader & shader, const char * src, std::function<void(const char *)> onError) {
    return impl::compileShader(shader, src, onError);
}

bool compileShader (VertexShader & shader, const char * src, std::function<void(const char *)> onError) {
    return impl::compileShader(shader, src, onError);
}

bool linkShader (ShaderProgram & program, const VertexShader & vertexShader, const FragmentShader & fragmentShader, std::function<void(const char *)> onError) {
    glAttachShader(program.handle, vertexShader.handle); CHECK_GL_ERRORS();
    glAttachShader(program.handle, fragmentShader.handle); CHECK_GL_ERRORS();
    glLinkProgram(program.handle); CHECK_GL_ERRORS();
    glValidateProgram(program.handle); CHECK_GL_ERRORS();
    
    GLint status;
    glGetProgramiv(program.handle, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint size;
        glGetProgramiv(program.handle, GL_INFO_LOG_LENGTH, &size);
        if (size > 0) {
            char * log = new char [size];
            GLsizei written;
            glGetProgramInfoLog(program.handle, size, &written, log);
            onError(log);
            delete[] log;
            return false;
        } else {
            return onError("Failed to link shader program\n"), false;
        }
    }
    return true;
}

bool Shader::compileFragment (const char * src, std::function<void(const ShaderLoadError &)> onError) {
    using boost::format;
    if (fragment_compiled) {
        return onError(ShaderLoadError { (format("Fragment shader already compiled (%s.fs)\n") % name).str() }), false;
    }
    return fragment_compiled = gl::compileShader(_fs, src, [&onError,this](const char * errorLog) {
        onError(ShaderLoadError {
            (format("Error compiling fragment shader (%s.fs):\n%s") % name % errorLog).str()
        });
    });
}

bool Shader::compileVertex (const char * src, std::function<void(const ShaderLoadError &)> onError) {
    using boost::format;
    if (vertex_compiled) {
        return onError(ShaderLoadError { (format("Vertex shader already compiled (%s.vs)\n") % name).str() }), false;
    }
    return vertex_compiled = compileShader(_vs, src, [&onError,this](const char *errorLog) {
        onError(ShaderLoadError {
            (format("Error compiling vertex shader (%s.fs):\n%s") % name % errorLog).str()
        });
    });
}

bool Shader::linkProgram (std::function<void(const ShaderLoadError &)> onError) {
    using boost::format;
    if (!fragment_compiled || !vertex_compiled)
        return onError(ShaderLoadError { (format("Cannot link shader '%s' -- uncompiled vertex/fragment shader(s)") % name).str() }), false;
    if (program_linked)
        return onError(ShaderLoadError { (format("Shader program '%s' already linked ") % name).str() }), false;
    return program_linked = gl::linkShader(_program, _vs, _fs, [&onError,this] (const char * errorLog) {
        onError(ShaderLoadError {
            (format("Error linking shader program '%s':\n%s") % name % errorLog).str()
        });
    });
}

// Default stand in for the onError parameter in the Shader class.
void Shader::dumpToStderr(const gl_sandbox::gl::ShaderLoadError &e) {
    std::cerr << e.what() << std::endl;
}












