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
using namespace gl_sandbox::gl::references;

const char * TriangleModule::MODULE_NAME = "module-triangles";
const char * TriangleModule::MODULE_DIR  = "triangles";

void TriangleModule::initModule() {
    
    CHECK_GL_ERRORS();
    
    std::cout << "Initializing triangle module" << std::endl;
    
    // Ideally:
//    resourceLoader->loadShaderAsync(m_shader, [] () {
//        std::cout << "Finished shader load: " << m_shader.name << std::endl;
//    }, [] (const auto & err) {
//        std::cout << "Shader failed to load (" << m_shader.name << "):\n" << err.what() << std::endl;
//    });

    // But can settle for this in the meantime:
    resourceLoader->loadTextFile(m_shader.name + ".fs", [this] (const char * src) {
        m_shader.compileFragment(src);
    });
    resourceLoader->loadTextFile(m_shader.name + ".vs", [this] (const char * src) {
        m_shader.compileVertex(src);
    });
    if (m_shader.linkProgram()) {
        std::cout << "Successfully loaded shader '" << m_shader.name << "'\n";
        glUseProgram(m_shader.handle()); CHECK_GL_ERRORS();
    } else {
        std::cout << "Failed to load shader '" << m_shader.name << "'\n";
        glUseProgram(0);
    }
    
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
    
    auto positionBuffer = m_buffers[0], colorBuffer = m_buffers[1];
    
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer.handle); CHECK_GL_ERRORS();
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), positionData, GL_STATIC_DRAW); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer.handle); CHECK_GL_ERRORS();
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colorData, GL_STATIC_DRAW); CHECK_GL_ERRORS();
    
    glBindVertexArray(m_vao.handle); CHECK_GL_ERRORS();
    
    glEnableVertexAttribArray(0); CHECK_GL_ERRORS();
    glEnableVertexAttribArray(1); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer.handle); CHECK_GL_ERRORS();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer.handle); CHECK_GL_ERRORS();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
}
TriangleModule::~TriangleModule() {
    std::cout << "Killing triangle module" << std::endl;
}
void TriangleModule::drawFrame() {
//    std::cout << "Running triangle module" << std::endl;
    glUseProgram(m_shader.handle()); CHECK_GL_ERRORS();
    glBindVertexArray(m_vao.handle); CHECK_GL_ERRORS();
    glDrawArrays(GL_TRIANGLES, 0, 3); CHECK_GL_ERRORS();
}



