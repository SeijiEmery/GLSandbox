//
//  gl_traits.hpp
//  GLSandbox
//
//  Created by semery on 12/16/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef gl_traits_h
#define gl_traits_h

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "gl_error.hpp"
#include <memory>

namespace gl_sandbox {

namespace gl {
namespace traits {

#ifdef DEBUG
#define return_withCheck(xs) auto x = (xs); return CHECK_GL_ERRORS(), x
#else
#define return_withCheck(xs) return xs
#endif
struct Fragment {
    typedef GLuint value_type;
    static value_type create () { return_withCheck(glCreateShader(GL_FRAGMENT_SHADER)); }
    static void destroy (value_type v) { if (v) glDeleteShader(v), CHECK_GL_ERRORS(); }
};
struct Vertex {
    typedef GLuint value_type;
    static value_type create () { return_withCheck(glCreateShader(GL_VERTEX_SHADER)); }
    static void destroy (value_type v) { if (v) glDeleteShader(v), CHECK_GL_ERRORS(); }
};
struct Program {
    typedef GLuint value_type;
    static value_type create () { return_withCheck(glCreateProgram()); }
    static void destroy (value_type v) { if (v) glDeleteProgram(v), CHECK_GL_ERRORS(); }
};
struct VAO {
    typedef GLuint value_type;
    static value_type create () { value_type v = 0; glGenVertexArrays(1, &v); CHECK_GL_ERRORS(); return v; }
    static void destroy (value_type v) { if (v) glDeleteVertexArrays(1, &v), CHECK_GL_ERRORS(); }
};
struct VBO {
    typedef GLuint value_type;
    static value_type create () {
//        std::cout << "creating vbo\n";
        value_type v = 0;
        glGenBuffers(1, &v); CHECK_GL_ERRORS();
//        std::cout << "done (= " << v << ")\n";
        return v;
    }
    static void destroy (value_type v) {
//        std::cout << "destroying vbo " << v << '\n';
        glDeleteBuffers(1, &v); CHECK_GL_ERRORS();
//        std::cout << "done\n";
    }
};
#undef return_withCheck

}; // namespace traits
    
template <typename traits>
struct GLObject {
    const typename traits::value_type handle;
    GLObject () : handle(traits::create()) {}
    ~GLObject () { traits::destroy(handle); }
    
    GLObject (const GLObject<traits> & other) = delete;
    GLObject (GLObject<traits> && other)
        : handle(other.handle) {}
};
    
template <typename traits>
using GLObjectRef = std::shared_ptr<GLObject<traits>>;
    
typedef GLObject<traits::Fragment> FragmentShader;
typedef GLObject<traits::Vertex>   VertexShader;
typedef GLObject<traits::Program>  ShaderProgram;
typedef GLObject<traits::VAO>      VAO;
typedef GLObject<traits::VBO>      VBO;
    
}; // namespace gl

}; // namespace gl_sandbox

#endif /* gl_traits_h */
