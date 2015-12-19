//
//  triangles.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "triangles.hpp"
#include "../../common/gl/gl_error.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/math/constants/constants.hpp>

#include <iostream>
#include <math.h>

using namespace gl_sandbox;
using namespace gl_sandbox::gl::references;

using glm::mat4;
using glm::vec3;

TriangleModule::TriangleModule() {
    
    CHECK_GL_ERRORS();
    
    std::cout << "Initializing triangle module" << std::endl;
    
    // Ideally:
//    resourceLoader->loadShaderAsync(m_shader, [] () {
//        std::cout << "Finished shader load: " << m_shader.name << std::endl;
//    }, [] (const auto & err) {
//        std::cout << "Shader failed to load (" << m_shader.name << "):\n" << err.what() << std::endl;
//    });

    // But can settle for this in the meantime:
    m_resourceLoader.loadTextFile((m_shader.name + ".fs").c_str(), [this] (const char * src) {
        m_shader.compileFragment(src);
    });
    m_resourceLoader.loadTextFile((m_shader.name + ".vs").c_str(), [this] (const char * src) {
        m_shader.compileVertex(src);
    });
    
    if (m_shader.linkProgram()) {
        std::cout << "Successfully loaded shader '" << m_shader.name << "'\n";
        glUseProgram(m_shader.handle()); CHECK_GL_ERRORS();
        glValidateProgram(m_shader.handle()); CHECK_GL_ERRORS();
    } else {
        std::cout << "Failed to load shader '" << m_shader.name << "'\n";
        glUseProgram(0); CHECK_GL_ERRORS();
    }
    
    std::cout << "Loading vaos, etc\n";

    constexpr float positionData [] = {
        -0.8f, -0.8f, 0.0f,
        0.8f, -0.8f, 0.0f,
        0.0f, 0.8f, 0.0f
    };
    constexpr float colorData [] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    
    auto &positionBuffer = m_buffers[0], &colorBuffer = m_buffers[1];
    
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();

//    glBindVertexArray(0); CHECK_GL_ERRORS();
//    glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_GL_ERRORS();
//    glUseProgram(0);
//    glDisableVertexAttribArray(0); CHECK_GL_ERRORS();
//    glDisableVertexAttribArray(1); CHECK_GL_ERRORS();
}
TriangleModule::~TriangleModule() {
    std::cout << "Killing triangle module" << std::endl;
}
void TriangleModule::drawFrame() {
    static auto uniform_rotationMatrix = m_shader.getUniformLocation("RotationMatrix");
    static double startTime = glfwGetTime();
    static double ROTATION_PERIOD = 2.5; // rotate 360 deg every 2.5 secs
    
    if (m_shader.loaded()) {
//        glUseProgram(m_shader.handle()); CHECK_GL_ERRORS();
//        glBindVertexArray(m_vao.handle); CHECK_GL_ERRORS();
        
        double elapsedTime = startTime - glfwGetTime();
        
        constexpr double TWO_PI  = boost::math::constants::pi<double>() * 2.0;
        
        float angle = (float)fmod((elapsedTime * (TWO_PI / ROTATION_PERIOD)), TWO_PI);
        mat4 rotationMatrix = glm::rotate(mat4(1.0f), angle, vec3(1.0f, 0.0f, 0.0f));
        
        m_shader.setUniform(uniform_rotationMatrix, rotationMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 3); CHECK_GL_ERRORS();
    }
}



