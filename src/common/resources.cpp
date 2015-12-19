//
//  resources.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#define STB_IMAGE_IMPLEMENTATION
#include "../../libs/stb/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../libs/tinyobjloader/tiny_obj_loader.h"

#include "resources.hpp"
#include "app.hpp"
#include <boost/format.hpp>
#include <iostream>

using namespace gl_sandbox;

boost::filesystem::path ResourceLoader::g_baseResourcePath;

#define RESOURCE_ERROR(msg, rest) ResourceError { (format("ResourceError: " msg) % rest).str() }

bool ResourceLoader::loadTextFile(
    const boost::filesystem::path & filepath,
    std::function<void (const char *)> onComplete,
    std::function<void (const ResourceError &)> onError)
{
    using namespace boost::filesystem;
    using boost::format;
    
    if (!exists(filepath))
        return onError(RESOURCE_ERROR("File '%s' does not exist\n", filepath)), false;
    
    boost::filesystem::ifstream f (filepath);
    if (!f)
        return onError(RESOURCE_ERROR("Cannot open file '%s'", filepath)), false;
    f.seekg(0, std::ios::end);
    size_t size = f.tellg();
    if (size == 0)
        return onError(RESOURCE_ERROR("Empty file '%s'", filepath)), false;
    f.seekg(0, std::ios::beg);
    char * buffer = new char [size+1];
    f.read(buffer, size);
    buffer[size] = 0;
    f.close();
    
    assert(*buffer != '\0');
    
    onComplete(buffer);
    delete[] buffer;
    return true;
}

bool ResourceLoader::loadImage(
    const boost::filesystem::path &filepath,
    ImageHandler onComplete,
    ErrorHandler onError
) {
//    using boost::format;
//    if (!boost::filesystem::exists(filepath))
//        return onError(RESOURCE_ERROR("File '%s' does not exist\n", filepath)), false;

    ImageInfo info;
    const uint8_t * image_data = stbi_load(filepath.string().c_str(), &info.size_x, &info.size_y, &info.image_format, 0);
    if (image_data != nullptr) {
        onComplete(image_data, info);
        stbi_image_free((void*)image_data);
        return true;
    } else {
        onError(ResourceError { stbi_failure_reason() });
        return false;
    }
}

bool ResourceLoader::loadObj(const Path &filepath, ObjHandler onComplete, ErrorHandler onError
) {
    ObjData obj;
    std::string err;
    if (tinyobj::LoadObj(obj.shapes, obj.materials, err, filepath.string().c_str()))
        return onComplete(obj), true;
    else
        return onError(ResourceError { err }), false;
}


bool ResourceLoader::resolvePath(const char *filename, const char *moduleDir, Path &path) {
    using namespace boost::filesystem;
    if (exists(g_baseResourcePath / filename))
        return path = g_baseResourcePath / filename, true;
    if (exists(g_baseResourcePath / "common" / filename))
        return path = g_baseResourcePath / "common" / filename, true;
    if (exists(g_baseResourcePath / moduleDir / filename))
        return path = g_baseResourcePath / moduleDir / filename, true;
    if (exists(g_baseResourcePath / "modules" / moduleDir / filename))
        return path = g_baseResourcePath / "modules" / moduleDir / filename, true;
    return false;
}


bool ResourceLoader::loadTextFile(const char *filename, const char *moduleDir, TextHandler onComplete, ErrorHandler onError)
{
    using boost::format;
    boost::filesystem::path path;
    if (resolvePath(filename, moduleDir, path))
        return loadTextFile(path, onComplete, onError);
    return onError(RESOURCE_ERROR("Cannot load resource (text): '%s' (module '%s')", filename % moduleDir)), false;
}

bool ResourceLoader::loadImage(const char *filename, const char *moduleDir, ImageHandler onComplete, ErrorHandler onError)
{
    using boost::format;
    boost::filesystem::path path;
    if (resolvePath(filename, moduleDir, path))
        return loadImage(path, onComplete, onError);
    return onError(RESOURCE_ERROR("Cannot load resource (image): '%s' (module '%s')", filename % moduleDir)), false;
}

bool ResourceLoader::loadObj(const char * filename, const char * moduleDir, ObjHandler onComplete, ErrorHandler onError) {
    using boost::format;
    boost::filesystem::path path;
    if (resolvePath(filename, moduleDir, path))
        return loadObj(path, onComplete);
    return onError(RESOURCE_ERROR("Cannot load resource (.obj): '%s' (module '%s')", filename % moduleDir)), false;
}

//ShaderHandle::Ptr ResourceLoader::loadShader (const Module & module, const char * vertex_shader, const char * fragment_shader) {
//    // Check shader inputs -- make sure exactly one vertex shader and one fragment shader, etc
//    
//    using namespace boost::filesystem;
//    bool err = false;
//    if (path(vertex_shader).extension() != ".vs") {
//        std::cerr << "Bad arguments to ResourceLoader::loadShader: vertex_shader '" << vertex_shader << "' should have an extension of '.vs', not '" << path(vertex_shader).extension() << "'\n";
//        err = true;
//    }
//    if (path(fragment_shader).extension() != ".fs") {
//        std::cerr << "Bad arguments to ResourceLoader::loadShader: fragment_shader '" << fragment_shader << "' should have an extension of '.fs', not '" << path(fragment_shader).extension() << "'\n";
//        err = true;
//    }
//    
//    if (err) {
//        return std::make_shared<ShaderHandle>(ShaderHandle::State::RESOURCE_ERROR);
//    }
//
//    auto vs_path = resourcePath / path(vertex_shader);
//    auto fs_path = resourcePath / path(fragment_shader);
//    
//    auto loadShader = [&err, &vertex_shader, &fragment_shader] (GLuint shaderType, GLuint & shaderHandle) {
////        std::cout << "shaderType = " << shaderType << std::endl;
////        std::cout << "shaderType == GL_VERTEX_SHADER? " << (shaderType == GL_VERTEX_SHADER) << std::endl;
////        std::cout << "shaderType == GL_FRAGMENT_SHADER? " << (shaderType == GL_FRAGMENT_SHADER) << std::endl;
//        
//        return [&err, shaderType, vertex_shader, fragment_shader, &shaderHandle](const char *shader_src) {
//            const char * shaderName = (shaderType == GL_VERTEX_SHADER ? vertex_shader :
//                                      (shaderType == GL_FRAGMENT_SHADER ? fragment_shader : nullptr));
//            
////            std::cout << "shaderType = " << shaderType << std::endl;
////            std::cout << "shaderType == GL_VERTEX_SHADER? " << (shaderType == GL_VERTEX_SHADER) << std::endl;
////            std::cout << "shaderType == GL_FRAGMENT_SHADER? " << (shaderType == GL_FRAGMENT_SHADER) << std::endl;
////            
////            std::cout << "Contents of '" << shaderName << ":\n" << shader_src << std::endl;
//            
//            
//            shaderHandle = glCreateShader(shaderType); CHECK_GL_ERRORS();
//            if (!shaderHandle) {
//                std::cerr << "Error creating shader object for '" << shaderName << "'\n";
//                err = true;
//                return;
//            }
//            const char * shader_src_array [] = { shader_src };
//            glShaderSource(shaderHandle, 1, shader_src_array, NULL); CHECK_GL_ERRORS();
//            glCompileShader(shaderHandle); CHECK_GL_ERRORS();
//            
//            GLint result;
//            glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result); CHECK_GL_ERRORS();
//            if (result == GL_FALSE) {
//                switch (shaderType) {
//                    case GL_VERTEX_SHADER: std::cerr << "Vertex shader compilation failed for '" << vertex_shader << "'\n"; break;
//                    case GL_FRAGMENT_SHADER: std::cerr << "Fragment shader compilation failed for '" << fragment_shader << "'\n"; break;
//                    default: assert(0);
//                }
//                
//                GLint logLen;
//                glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLen); CHECK_GL_ERRORS();
//                if (logLen > 0) {
//                    char * log = new char [logLen];
//                    GLsizei written;
//                    glGetShaderInfoLog(shaderHandle, logLen, &written, log); CHECK_GL_ERRORS();
//                    std::cerr << "Shader log for '" << shaderName << "':\n" << log << std::endl;
//                    delete[] log;
//                } else {
//                    std::cerr << "No error log for '" << shaderName << "'\n";
//                }
//                
//                err = true;
//                return;
//            }
//        };
//    };
//    auto onError = [&err] (const ResourceError & resourceError) {
//        std::cerr << resourceError.what() << std::endl;
//        err = true;
//    };
//    
//    GLuint gl_vertex_shader, gl_fragment_shader;
//    loadTextFile(vs_path, loadShader(GL_VERTEX_SHADER, gl_vertex_shader), onError);
//    loadTextFile(fs_path, loadShader(GL_FRAGMENT_SHADER, gl_fragment_shader), onError);
//    
//    if (err) {
//        return std::make_shared<ShaderHandle>(ShaderHandle::State::SHADER_COMPILATION_ERROR);
//    } else {
//        GLuint program_object = glCreateProgram(); CHECK_GL_ERRORS();
//        if (!program_object) {
//            std::cerr << "Error creating shader program for '" << vertex_shader << "' / '" << fragment_shader << "'\n";
//            err = true;
//        } else {
//            glAttachShader(program_object, gl_vertex_shader); CHECK_GL_ERRORS();
//            glAttachShader(program_object, gl_fragment_shader); CHECK_GL_ERRORS();
//            glLinkProgram(program_object); CHECK_GL_ERRORS();
//            
//            GLint status;
//            glGetProgramiv(program_object, GL_LINK_STATUS, &status); CHECK_GL_ERRORS();
//            if (status == GL_FALSE) {
//                std::cerr << "Failed to link shader program ('" << vertex_shader << "' / '" << fragment_shader << "')\n";
//                GLint logLen;
//                glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &logLen); CHECK_GL_ERRORS();
//                if (logLen > 0) {
//                    char * log = new char [logLen];
//                    GLsizei written;
//                    glGetProgramInfoLog(program_object, logLen, &written, log); CHECK_GL_ERRORS();
//                    std::cerr << "Program log:\n" << log << std::endl;
//                    delete[] log;
//                } else {
//                    std::cerr << "No program log\n";
//                }
//                err = true;
//            }
//            
//            glValidateProgram(program_object); CHECK_GL_ERRORS();
//        }
//        if (err) {
//            return std::make_shared<ShaderHandle>(ShaderHandle::State::SHADER_LINK_ERROR);
//        } else {
//            return std::make_shared<ShaderHandle>(program_object);
//        }
//    }
//}