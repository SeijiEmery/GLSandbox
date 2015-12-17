//
//  resources.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "resources.hpp"
#include <iostream>
#include "./gl/gl_error.hpp"

using namespace gl_sandbox;

template <typename... Args>
void ResourceLoader::logResourceError(const char *fmt, Args... args) {
    static char buffer [256];
    snprintf(buffer, sizeof(buffer), fmt, args...);
    std::cerr << "Resource error -- " << buffer << std::endl;
}

void ResourceLoader::executeAsync(std::function<void ()> fcn) {
    fcn();
}
void ResourceLoader::loadTextFile(const char * filename, std::function<void (const char *)> onComplete, std::function<void (const ResourceError &)> onError) {
    return loadTextFile({ filename }, onComplete, onError);
}
void ResourceLoader::loadTextFile(const boost::filesystem::path & filepath, std::function<void (const char *)> onComplete, std::function<void (const ResourceError &)> onError) {
    std::stringstream err;
    if (!boost::filesystem::exists(filepath)) {
        err << "ResourceError: File '" << filepath << " does not exist. ";
        onError(ResourceError { err.str() });
        return;
    }
    boost::filesystem::ifstream f (filepath);
    if (!f) {
        err << "ResourceError: Error reading file '" << filepath << "'. ";
        onError(ResourceError { err.str() });
        return;
    }
    f.seekg(0, std::ios::end);
    size_t size = f.tellg();
    if (size == 0) {
        err << "ResourceError: empty file '" << filepath << "'.";
        onError(ResourceError { err.str() });
        return;
    }
    f.seekg(0, std::ios::beg);
    char * buffer = new char [size];
    f.read(buffer, size);
    f.close();
    
    assert(*buffer != '\0');
    
    onComplete(buffer);
    delete[] buffer;
}
void ResourceLoader::loadTextFileAsync(const char * filename, std::function<void (const char *)> onComplete, std::function<void (const ResourceError &)> onError) {
    loadTextFile(filename, onComplete, onError);
}

ShaderHandle::Ptr ResourceLoader::loadShader (const Module & module, const char * vertex_shader, const char * fragment_shader) {
    // Check shader inputs -- make sure exactly one vertex shader and one fragment shader, etc
    
    using namespace boost::filesystem;
    bool err = false;
    if (path(vertex_shader).extension() != ".vs") {
        std::cerr << "Bad arguments to ResourceLoader::loadShader: vertex_shader '" << vertex_shader << "' should have an extension of '.vs', not '" << path(vertex_shader).extension() << "'\n";
        err = true;
    }
    if (path(fragment_shader).extension() != ".fs") {
        std::cerr << "Bad arguments to ResourceLoader::loadShader: fragment_shader '" << fragment_shader << "' should have an extension of '.fs', not '" << path(fragment_shader).extension() << "'\n";
        err = true;
    }
    
    if (err) {
        return std::make_shared<ShaderHandle>(ShaderHandle::State::RESOURCE_ERROR);
    }

    auto vs_path = resourcePath / path(vertex_shader);
    auto fs_path = resourcePath / path(fragment_shader);
    
    auto loadShader = [&err, &vertex_shader, &fragment_shader] (GLuint shaderType, GLuint & shaderHandle) {
//        std::cout << "shaderType = " << shaderType << std::endl;
//        std::cout << "shaderType == GL_VERTEX_SHADER? " << (shaderType == GL_VERTEX_SHADER) << std::endl;
//        std::cout << "shaderType == GL_FRAGMENT_SHADER? " << (shaderType == GL_FRAGMENT_SHADER) << std::endl;
        
        return [&err, shaderType, vertex_shader, fragment_shader, &shaderHandle](const char *shader_src) {
            const char * shaderName = (shaderType == GL_VERTEX_SHADER ? vertex_shader :
                                      (shaderType == GL_FRAGMENT_SHADER ? fragment_shader : nullptr));
            
//            std::cout << "shaderType = " << shaderType << std::endl;
//            std::cout << "shaderType == GL_VERTEX_SHADER? " << (shaderType == GL_VERTEX_SHADER) << std::endl;
//            std::cout << "shaderType == GL_FRAGMENT_SHADER? " << (shaderType == GL_FRAGMENT_SHADER) << std::endl;
//            
//            std::cout << "Contents of '" << shaderName << ":\n" << shader_src << std::endl;
            
            
            shaderHandle = glCreateShader(shaderType); CHECK_GL_ERRORS();
            if (!shaderHandle) {
                std::cerr << "Error creating shader object for '" << shaderName << "'\n";
                err = true;
                return;
            }
            const char * shader_src_array [] = { shader_src };
            glShaderSource(shaderHandle, 1, shader_src_array, NULL); CHECK_GL_ERRORS();
            glCompileShader(shaderHandle); CHECK_GL_ERRORS();
            
            GLint result;
            glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result); CHECK_GL_ERRORS();
            if (result == GL_FALSE) {
                switch (shaderType) {
                    case GL_VERTEX_SHADER: std::cerr << "Vertex shader compilation failed for '" << vertex_shader << "'\n"; break;
                    case GL_FRAGMENT_SHADER: std::cerr << "Fragment shader compilation failed for '" << fragment_shader << "'\n"; break;
                    default: assert(0);
                }
                
                GLint logLen;
                glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLen); CHECK_GL_ERRORS();
                if (logLen > 0) {
                    char * log = new char [logLen];
                    GLsizei written;
                    glGetShaderInfoLog(shaderHandle, logLen, &written, log); CHECK_GL_ERRORS();
                    std::cerr << "Shader log for '" << shaderName << "':\n" << log << std::endl;
                    delete[] log;
                } else {
                    std::cerr << "No error log for '" << shaderName << "'\n";
                }
                
                err = true;
                return;
            }
        };
    };
    auto onError = [&err] (const ResourceError & resourceError) {
        std::cerr << resourceError.what() << std::endl;
        err = true;
    };
    
    GLuint gl_vertex_shader, gl_fragment_shader;
    loadTextFile(vs_path, loadShader(GL_VERTEX_SHADER, gl_vertex_shader), onError);
    loadTextFile(fs_path, loadShader(GL_FRAGMENT_SHADER, gl_fragment_shader), onError);
    
    if (err) {
        return std::make_shared<ShaderHandle>(ShaderHandle::State::SHADER_COMPILATION_ERROR);
    } else {
        GLuint program_object = glCreateProgram(); CHECK_GL_ERRORS();
        if (!program_object) {
            std::cerr << "Error creating shader program for '" << vertex_shader << "' / '" << fragment_shader << "'\n";
            err = true;
        } else {
            glAttachShader(program_object, gl_vertex_shader); CHECK_GL_ERRORS();
            glAttachShader(program_object, gl_fragment_shader); CHECK_GL_ERRORS();
            glLinkProgram(program_object); CHECK_GL_ERRORS();
            
            GLint status;
            glGetProgramiv(program_object, GL_LINK_STATUS, &status); CHECK_GL_ERRORS();
            if (status == GL_FALSE) {
                std::cerr << "Failed to link shader program ('" << vertex_shader << "' / '" << fragment_shader << "')\n";
                GLint logLen;
                glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &logLen); CHECK_GL_ERRORS();
                if (logLen > 0) {
                    char * log = new char [logLen];
                    GLsizei written;
                    glGetProgramInfoLog(program_object, logLen, &written, log); CHECK_GL_ERRORS();
                    std::cerr << "Program log:\n" << log << std::endl;
                    delete[] log;
                } else {
                    std::cerr << "No program log\n";
                }
                err = true;
            }
            
            glValidateProgram(program_object); CHECK_GL_ERRORS();
        }
        if (err) {
            return std::make_shared<ShaderHandle>(ShaderHandle::State::SHADER_LINK_ERROR);
        } else {
            return std::make_shared<ShaderHandle>(program_object);
        }
    }
}