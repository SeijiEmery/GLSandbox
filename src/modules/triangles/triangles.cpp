//
//  triangles.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "triangles.hpp"
#include "../../common/gl/gl_error.hpp"
#include "../../common/app.hpp"

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
    m_resourceLoader.loadTextFile(m_shader.name + ".fs", [this] (auto src) {
        m_shader.compileFragment(src);
    });
    m_resourceLoader.loadTextFile(m_shader.name + ".vs", [this] (const char * src) {
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
    
    m_uniform_mvp_matrix = m_shader.getUniformLocation("MVP");
    m_startTime = glfwGetTime();
}
TriangleModule::~TriangleModule() {
    std::cout << "Killing triangle module" << std::endl;
}
void TriangleModule::drawFrame() {
    if (m_shader.loaded()) {
        glUseProgram(m_shader ? m_shader.handle() : 0); CHECK_GL_ERRORS();
        glBindVertexArray(m_vao.handle); CHECK_GL_ERRORS();
        
        double elapsedTime = m_startTime - glfwGetTime();
        constexpr double TWO_PI  = boost::math::constants::pi<double>() * 2.0;
        
        float angle = (float)fmod((elapsedTime * (TWO_PI / ROTATION_PERIOD)), TWO_PI);
        mat4 rotationMatrix = glm::rotate(mat4(1.0f), angle, vec3(1.0f, 0.0f, 0.0f));
        
        
        mat4 model          = glm::translate(rotationMatrix, vec3(0, 0, -1));
        mat4 view           = Application::mainCamera()->view;
        mat4 proj           = glm::perspective(45.0f, 1.7f, 0.01f, 1e3f);
        
//        m_shader.setUniform(m_uniform_mvp_matrix, proj * view * model);
//        glDrawArrays(GL_TRIANGLES, 0, 3); CHECK_GL_ERRORS();
        
        auto pv = proj * view;
        mat4 identity (1.0f);
        auto axis = vec3(0.0f, 1.0f, 0.0f);
        
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 20; ++j) {
                for (int k = 0; k < 20; ++k) {
                    
                    auto pos = glm::vec3 { (i - 10), (j - 10), (k - 10) } * 5.0f;
                    auto model = glm::rotate(glm::translate(identity, pos), angle, axis);
                    
                    m_shader.setUniform(m_uniform_mvp_matrix, pv * model);
                    glDrawArrays(GL_TRIANGLES, 0, 3); CHECK_GL_ERRORS();
                }
            }
        }
        
        
    }
}



