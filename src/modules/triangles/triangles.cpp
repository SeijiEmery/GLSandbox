//
//  triangles.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "triangles.hpp"
#include "../../common/gl/gl_error.hpp"

#include <iostream>

using namespace gl_sandbox;



//void APIENTRY openglCallbackFunction(GLenum source,
//                                     GLenum type,
//                                     GLuint id,
//                                     GLenum severity,
//                                     GLsizei length,
//                                     const GLchar* message,
//                                     const void* userParam){
//    using namespace std;
//    
//    cout << "---------------------opengl-callback-start------------" << endl;
//    cout << "message: "<< message << endl;
//    cout << "type: ";
//    switch (type) {
//        case GL_DEBUG_TYPE_ERROR:
//            cout << "ERROR";
//            break;
//        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
//            cout << "DEPRECATED_BEHAVIOR";
//            break;
//        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
//            cout << "UNDEFINED_BEHAVIOR";
//            break;
//        case GL_DEBUG_TYPE_PORTABILITY:
//            cout << "PORTABILITY";
//            break;
//        case GL_DEBUG_TYPE_PERFORMANCE:
//            cout << "PERFORMANCE";
//            break;
//        case GL_DEBUG_TYPE_OTHER:
//            cout << "OTHER";
//            break;
//    }
//    cout << endl;
//    
//    cout << "id: " << id << endl;
//    cout << "severity: ";
//    switch (severity){
//        case GL_DEBUG_SEVERITY_LOW:
//            cout << "LOW";
//            break;
//        case GL_DEBUG_SEVERITY_MEDIUM:
//            cout << "MEDIUM";
//            break;
//        case GL_DEBUG_SEVERITY_HIGH:
//            cout << "HIGH";
//            break;
//    }
//    cout << endl;
//    cout << "---------------------opengl-callback-end--------------" << endl;
//}




const char * TriangleModule::MODULE_NAME = "module-triangles";
const char * TriangleModule::MODULE_DIR  = "triangles";

void TriangleModule::initModule() {
    
    CHECK_GL_ERRORS();
    
//    if(glDebugMessageCallback){
//        std::cout << "Register OpenGL debug callback " << std::endl;
//        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//        glDebugMessageCallback(openglCallbackFunction, nullptr);
//        GLuint unusedIds = 0;
//        glDebugMessageControl(GL_DONT_CARE,
//                              GL_DONT_CARE,
//                              GL_DONT_CARE,
//                              0,
//                              &unusedIds,
//                              true);
//    }
//    else
//        std::cout << "glDebugMessageCallback not available" << std::endl;
    
    std::cout << "Initializing triangle module" << std::endl;
    m_shader = loadShader("basic_shader.vs", "basic_shader.fs");
    std::cout << "Loaded shader\n";
    std::cout << "Shader status = ";
    switch(m_shader->state) {
        case ShaderHandle::State::RESOURCE_ERROR: std::cout << "RESOURCE_ERROR\n"; break;
        case ShaderHandle::State::SHADER_LINK_ERROR: std::cout << "LINK_ERROR\n"; break;
        case ShaderHandle::State::SHADER_COMPILATION_ERROR: std::cout << "COMPILATION ERROR\n"; break;
        case ShaderHandle::State::LOADING: std::cout << "LOADING\n"; break;
        case ShaderHandle::State::OK: std::cout << "OK\n";
    }
    std::cout << "Shader id: " << m_shader->handle << std::endl;
    
    if (m_shader->state == ShaderHandle::State::OK) {
        std::cout << "Loading vaos, etc\n";

        constexpr float positionData [] = {
            -8.0f, -8.0f, 0.0f,
            0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f
        };
        constexpr float colorData [] = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };
        
        glUseProgram(m_shader->handle); CHECK_GL_ERRORS();
    
        GLuint vboHandles [2];
        glGenBuffers(2, vboHandles); CHECK_GL_ERRORS();
        
        GLuint positionBufferHandle = vboHandles[0];
        GLuint colorBufferHandle  = vboHandles[1];
        
        glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle); CHECK_GL_ERRORS();
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), positionData, GL_STATIC_DRAW); CHECK_GL_ERRORS();
        
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle); CHECK_GL_ERRORS();
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colorData, GL_STATIC_DRAW); CHECK_GL_ERRORS();
        
        glGenVertexArrays(1, &m_vaoHandle); CHECK_GL_ERRORS();
        glBindVertexArray(m_vaoHandle); CHECK_GL_ERRORS();
        
        glEnableVertexAttribArray(0); CHECK_GL_ERRORS();
        glEnableVertexAttribArray(1); CHECK_GL_ERRORS();
        
        glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle); CHECK_GL_ERRORS();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
        
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle); CHECK_GL_ERRORS();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
    }
}
TriangleModule::~TriangleModule() {
    std::cout << "Killing triangle module" << std::endl;
}
void TriangleModule::drawFrame() {
//    std::cout << "Running triangle module" << std::endl;
    glUseProgram(m_shader->handle); CHECK_GL_ERRORS();
    glBindVertexArray(m_vaoHandle); CHECK_GL_ERRORS();
    glDrawArrays(GL_TRIANGLES, 0, 3); CHECK_GL_ERRORS();
}



