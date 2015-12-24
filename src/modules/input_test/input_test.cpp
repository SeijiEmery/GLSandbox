//
//  input_test.cpp
//  GLSandbox
//
//  Created by semery on 12/23/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "input_test.hpp"
#include "../../common/app.hpp"
#include <iostream>

using namespace gl_sandbox;
using namespace gl_sandbox::modules;

InputTestModule::InputTestModule () :
m_deviceConnectedObserver(input->onDeviceConnected.connect([](auto deviceName, auto profile) {
    std::cout << "Device connected: '" << deviceName << "' (profile '" << input::gamepadProfileToString(profile) << "')\n";
})),
m_deviceDisconnectedObserver(input->onDeviceDisconnected.connect([](auto deviceName, auto profile) {
    std::cout << "Device disconnected: '" << deviceName << "' (profile '" << input::gamepadProfileToString(profile) << "')\n";
})),
m_gamepadButtonPressedObserver(input->onGamepadButtonPressed.connect([](auto button) {
    std::cout << "Gamepad button pressed: '" << input::gamepadButtonToString(button) << "'\n";
})),
m_gamepadButtonReleasedObserver(input->onGamepadButtonReleased.connect([](auto button) {
    std::cout << "Gamepad button released: '" << input::gamepadButtonToString(button) << "'\n";
})),
m_gamepadAxesObserver(input->onGamepadAxesUpdate.connect([](auto axes) {
    auto i = 0;
    for (; i < input::NUM_GAMEPAD_AXES; ++i) {
        if (axes[i] != 0)
            goto some_axis_pressed;
    }
    return;
some_axis_pressed:
    std::cout << "Gamepad axes: ";
    for (; i < input::NUM_GAMEPAD_AXES; ++i) {
        if (axes[i] != 0) {
            float v = axes[i];
            std::cout << input::gamepadAxisToString((input::GamepadAxis)i) << ' ' << (float)v << ' ';
        }
    }
    std::cout << '\n';
}))
{
    std::cout << "Loading '" << MODULE_NAME << "'\n";
}

InputTestModule::~InputTestModule() {
    std::cout << "Unloading '" << MODULE_NAME << "'\n";
}






