//
//  camera.cpp
//  GLSandbox
//
//  Created by semery on 12/21/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "camera.hpp"
#include <iostream>
#include <sstream>
#include <array>


using namespace gl_sandbox;

CameraController::CameraController (Camera * camera)
    : m_camera(camera), m_lastUpdate(glfwGetTime())
{}
CameraController::CameraController ()
    : m_lastUpdate(glfwGetTime())
{}



void CameraController::update () {
    double currentTime = glfwGetTime();
    double elapsed = currentTime - m_lastUpdate;
    m_lastUpdate = currentTime;
    
//    debugScanGamepadInput();
    
    // Poll for controller/gamepad input
    for (auto i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; ++i) {
        if (glfwJoystickPresent(i)) {
            // Use that joystick
            
            int naxes, nbuttons;
            const float * axes = glfwGetJoystickAxes(i, &naxes);
            const uint8_t * buttons = glfwGetJoystickButtons(i, &nbuttons);
            auto name = glfwGetJoystickName(i);
            
            
            
//            std::cout << "Using joystick '" << name << "'\n";
//            std::cout << naxes << " axes: ";
//            for (auto n = naxes; n --> 0; ) {
//                std::cout << axes[n] << " ";
//            }
//            std::cout << '\n';
            
//            std::cout << nbuttons << " buttons: ";
//            for (auto n = nbuttons; n --> 0; ) {
//                std::cout << (int)buttons[n] << " ";
//            }
//            std::cout << '\n';
        }
    }
    
    
    
    
}

















