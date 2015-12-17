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
    
    decltype(_program.handle) handle () const {
        return _program.handle;
    }
    typedef std::function<void(const ShaderLoadError &)> ErrorCallback;
    
    bool compileFragment (const char * src, ErrorCallback onError = dumpToStderr);
    bool compileVertex (const char * src, ErrorCallback onError = dumpToStderr);
    bool linkProgram (ErrorCallback onError = dumpToStderr);
    
    // Default impl for the optional onError parameter on compileFragment, etc;
    // Just takes message (e.what) and prints to stdout (cerr).
    static void dumpToStderr (const ShaderLoadError & e);
};
    
}; // namespace gl
}; // namespace gl_sandbox



#endif /* shaders_hpp */
