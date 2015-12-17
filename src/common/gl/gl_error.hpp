//
//  gl_error.hpp
//  GLSandbox
//
//  Created by semery on 12/16/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef gl_error_h
#define gl_error_h

#include <iostream>

inline void check_gl_errors (const char * file, int line) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch(err) {
            case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        std::cerr << file << ":" << line << "' OpenGL error: '" << error << std::endl;
    }
}
#ifdef DEBUG
#define CHECK_GL_ERRORS() check_gl_errors(__FILE__, __LINE__)
#else
#define CHECK_GL_ERRORS()
#endif


#endif /* gl_error_h */
