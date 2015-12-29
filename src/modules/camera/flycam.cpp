//
//  flycam.cpp
//  GLSandbox
//
//  Created by semery on 12/24/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "flycam.hpp"
#include "../../common/app.hpp"
#include "../../common/camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace gl_sandbox;
using namespace gl_sandbox::modules;

CameraModule::CameraModule () :
    m_camera(Application::mainCamera()),
    m_buttonObserver(input->onGamepadButtonPressed.connect([this](auto button){ onButtonPressed(button); })),
    m_axisObserver(input->onGamepadAxesUpdate.connect([this](auto axes){ onGamepadUpdate(axes); })),
    m_lastUpdate(glfwGetTime())
{
    assert(m_camera != nullptr);
}

void CameraModule::onButtonPressed (input::GamepadButton button) {
    if (button == input::BUTTON_LSTICK)
        m_cameraPos = { 0, 0, 0 };
    if (button == input::BUTTON_RSTICK)
        m_cameraAngles = { 0, 0, 0 };
}
void CameraModule::onGamepadUpdate (const float * axes) {
    double curTime = glfwGetTime();
    float dt       = curTime - m_lastUpdate;
    m_lastUpdate = curTime;
    
    m_cameraAngles.x += axes[input::AXIS_RY] * dt * CAMERA_TURN_SPEED;
    m_cameraAngles.y -= axes[input::AXIS_RX] * dt * CAMERA_TURN_SPEED;
    
    auto clamp = [](auto &x, auto a, auto b) { return x = std::min(std::max(x, a), b); };
    clamp(m_cameraAngles.x, -89.99f, 89.99f);
    
    glm::vec3 dir {
        cos(glm::radians(m_cameraAngles.x)) * cos(glm::radians(m_cameraAngles.y)),
        sin(glm::radians(m_cameraAngles.x)),
        cos(glm::radians(m_cameraAngles.x)) * sin(glm::radians(m_cameraAngles.y))
    };
    
    auto fwd = dir;
    auto right = glm::cross(fwd, { 0, 1, 0 });
    auto up    = glm::cross(fwd, right);
    
    m_cameraPos -= right * axes[input::AXIS_LY] * dt * CAMERA_SPEED;
    m_cameraPos -= fwd   * axes[input::AXIS_LX] * dt * CAMERA_SPEED;
    m_cameraPos += up    * (axes[input::AXIS_RTRIGGER] - axes[input::AXIS_LTRIGGER]) * dt * CAMERA_SPEED;
    
//    m_camera->view = glm::lookAt(m_cameraPos, glm::vec3(0, 0, 0), up);
    m_camera->view = glm::lookAt(m_cameraPos, m_cameraPos + glm::normalize(dir), up);
    
}

