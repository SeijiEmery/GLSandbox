//
//  triangles.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "triangles.hpp"
#include "../../common/gl/gl_wrapper.hpp"
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

constexpr double TWO_PI  = boost::math::constants::pi<double>() * 2.0;
constexpr double ROTATION_PERIOD = 2.5;

gl::Shader loadShader (ResourceLoader &r, std::string name) {
    gl::Shader shader { name };
    r.loadTextFile(name + ".fs", [&] (auto src) {
        shader.compileFragment(src); CHECK_GL_ERRORS();
    });
    r.loadTextFile(name + ".vs", [&](auto src) {
        shader.compileVertex(src); CHECK_GL_ERRORS();
    });
    if (shader.linkProgram()) {
        std::cout << "Successfully loaded shader '" << name << "'\n";
        glValidateProgram(shader.handle()); CHECK_GL_ERRORS();
    } else {
        std::cout << "Failed to load shader '" << name << "'\n";
    }
    CHECK_GL_ERRORS();
    return shader;
}

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
    
    auto &positionBuffer = m_buffers[0], &colorBuffer = m_buffers[1], &instanceBuffer = m_buffers[2];
    
    std::vector<glm::vec3> instancePositions;
    instancePositions.reserve(INSTANCE_ARRAY_SIZE);
    for (unsigned z = INSTANCE_ARRAY_DEPTH; z --> 0; ) {
        for (unsigned y = INSTANCE_ARRAY_HEIGHT; y --> 0; ) {
            for (unsigned x = INSTANCE_ARRAY_WIDTH; x --> 0; ) {
                instancePositions.emplace_back(
                    ((float)x - INSTANCE_ARRAY_WIDTH * 0.5) * 5.0f,
                    ((float)y - INSTANCE_ARRAY_HEIGHT * 0.5) * 5.0f,
                    ((float)z - INSTANCE_ARRAY_DEPTH * 0.5) * 5.0f
                );
            }
        }
    }
    assert(instancePositions[1] != instancePositions[21]);
    
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer.handle); CHECK_GL_ERRORS();
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), positionData, GL_STATIC_DRAW); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer.handle); CHECK_GL_ERRORS();
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colorData, GL_STATIC_DRAW); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer.handle); CHECK_GL_ERRORS();
    glBufferData(GL_ARRAY_BUFFER, INSTANCE_ARRAY_SIZE * sizeof(float) * 3, &instancePositions[0], GL_STATIC_DRAW); CHECK_GL_ERRORS();
    
    glBindVertexArray(m_vao.handle); CHECK_GL_ERRORS();
    
    glEnableVertexAttribArray(0); CHECK_GL_ERRORS();
    glEnableVertexAttribArray(1); CHECK_GL_ERRORS();
    glEnableVertexAttribArray(2); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer.handle); CHECK_GL_ERRORS();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer.handle); CHECK_GL_ERRORS();
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer.handle); CHECK_GL_ERRORS();
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
    glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_GL_ERRORS();
    glVertexAttribDivisor(2, 1); CHECK_GL_ERRORS(); // instance data: update each draw (1; default 0)
    
    m_uniform_vp_matrix = m_shader.getUniformLocation ("ViewProjMatrix");
    m_uniform_rot_matrix = m_shader.getUniformLocation("RotationMatrix");
    
    m_startTime = m_lastTime = glfwGetTime();
    m_lastFovNotify = m_startTime - m_fovNotifyDelay;
    
    m_buttonObservers[0] = input->onGamepadButtonPressed.connect([this](auto button) {
        if (button == input::GamepadButton::BUTTON_LBUMPER)
            m_lbPressed = true;
        if (button == input::GamepadButton::BUTTON_RBUMPER)
            m_rbPressed = true;
        m_fovDir = (m_lbPressed ? -1.0 : 0.0) + (m_rbPressed ? 1.0 : 0.0);
    });
    m_buttonObservers[1] = input->onGamepadButtonReleased.connect([this](auto button) {
        if (button == input::GamepadButton::BUTTON_LBUMPER)
            m_lbPressed = false;
        if (button == input::GamepadButton::BUTTON_RBUMPER)
            m_rbPressed = false;
        m_fovDir = (m_lbPressed ? -1.0 : 0.0) + (m_rbPressed ? 1.0 : 0.0);
    });
}
TriangleModule::~TriangleModule() {
    std::cout << "Killing triangle module" << std::endl;
}
void TriangleModule::drawFrame() {
    glUseProgram(m_shader.handle());
    glBindVertexArray(m_vao.handle); CHECK_GL_ERRORS();
    
    double curTime     = glfwGetTime();
    double elapsedTime = m_startTime - curTime;
    double dt          = m_lastTime  - curTime;
    m_lastTime = curTime;

    constexpr double TWO_PI  = boost::math::constants::pi<double>() * 2.0;
        
    float angle = (float)fmod((elapsedTime * (TWO_PI / ROTATION_PERIOD)), TWO_PI);
    mat4 rotationMatrix = glm::rotate(mat4(1.0f), angle, vec3(0.0f, 1.0f, 0.0f));
    
    auto fov = std::min(m_cameraMaxFov, std::max(m_cameraMinFov,
        m_cameraFov + m_fovIncrement * m_fovDir * (float)dt));
    if (fov != m_cameraFov && curTime > m_lastFovNotify + m_fovNotifyDelay) {
        m_lastFovNotify = curTime;
        std::cout << "Set triangles fov to " << fov << '\n';
    }
    m_cameraFov = fov;
    
    mat4 view           = Application::mainCamera()->view;
    mat4 proj           = glm::perspective(m_cameraFov, 1.7f, 0.01f, 1e3f);
    
    m_shader.setUniform(m_uniform_vp_matrix, proj * view);
    m_shader.setUniform(m_uniform_rot_matrix, rotationMatrix);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, INSTANCE_ARRAY_SIZE); CHECK_GL_ERRORS();
    
//    auto pv = proj * view;
//    mat4 identity (1.0f);
//
//    for (int i = 0; i < 20; ++i) {
//        for (int j = 0; j < 20; ++j) {
//            for (int k = 0; k < 20; ++k) {
//                
//                auto pos = glm::vec3 { (i - 10), (j - 10), (k - 10) } * 5.0f;
//                auto model = glm::translate(identity, pos);
//                
//                m_shader.setUniform(m_uniform_vp_matrix, pv * model);
//                glDrawArrays(GL_TRIANGLES, 0, 3); CHECK_GL_ERRORS();
//            }
//        }
//    }
}



