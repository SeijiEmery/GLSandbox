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
#include <memory>

namespace gl_sandbox {

namespace gl {
namespace traits {

struct Fragment {
    typedef GLuint value_type;
    static value_type create () { return glCreateShader(GL_FRAGMENT_SHADER); }
    static void destroy (value_type v) { glDeleteShader(v); }
};
struct Vertex {
    typedef GLuint value_type;
    static value_type create () { return glCreateShader(GL_VERTEX_SHADER); }
    static void destroy (value_type v) { glDeleteShader(v); }
};
struct Program {
    typedef GLuint value_type;
    static value_type create () { return glCreateProgram(); }
    static void destroy (value_type v) { glDeleteProgram(v); }
};
struct VAO {
    typedef GLuint value_type;
    static value_type create () { value_type v [1]; glGenVertexArrays(1, v); return v[0]; }
    static void destroy (value_type v) { value_type vs [] = { v }; glDeleteVertexArrays(1, vs); }
};
struct VBO {
    typedef GLuint value_type;
    static value_type create () { value_type v [1]; glGenBuffers(1, v); return v[0]; }
    static void destroy (value_type v) { value_type vs [] = { v }; glDeleteBuffers(1, vs); }
};

}; // namespace traits
    
template <typename traits>
struct GLObject {
    const typename traits::value_type handle;
    GLObject () : handle(traits::create()) {}
    ~GLObject () { traits::destroy(handle); }
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
