//
//  input.cpp
//  GLSandbox
//
//  Created by semery on 12/23/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "input.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace gl_sandbox;

const char * gl_sandbox::input::gamepadButtonToString (input::GamepadButton button) {
    constexpr const char * names [] {
        "BUTTON_A",
        "BUTTON_B",
        "BUTTON_X",
        "BUTTON_Y",
        "BUTTON_DPAD_UP",
        "BUTTON_DPAD_DOWN",
        "BUTTON_DPAD_LEFT",
        "BUTTON_DPAD_RIGHT",
        "BUTTON_LTRIGGER",
        "BUTTON_RTRIGGER",
        "BUTTON_LBUMPER",
        "BUTTON_RBUMBER",
        "BUTTON_LSTICK",
        "BUTTON_RSTICK",
        "BUTTON_START",
        "BUTTON_SELECT",
        "BUTTON_HOME"
    };
    return names[button];
}
const char * gl_sandbox::input::gamepadAxisToString(input::GamepadAxis axis) {
    constexpr const char * names [] {
        "AXIS_LEFT_X",
        "AXIS_LEFT_Y",
        "AXIS_RIGHT_X",
        "AXIS_RIGHT_Y",
        "AXIS_LEFT_TRIGGER",
        "AXIS_RIGHT_TRIGGER",
        "AXIS_DPAD_X",
        "AXIS_DPAD_Y"
    };
    return names[axis];
}

static GamepadProfile getMatchingProfile (const std::string & name) {
    if (name == "Xbox 360 Wired Controller")
        return GamepadProfile::XBOX_PROFILE;
    return GamepadProfile::UNKNOWN_PROFILE;
}

namespace gl_sandbox {
namespace gamepad_profiles {
    namespace xbox_controller {
        constexpr input::GamepadButton buttons[] = {
            input::BUTTON_DPAD_UP,
            input::BUTTON_DPAD_DOWN,
            input::BUTTON_DPAD_LEFT,
            input::BUTTON_DPAD_RIGHT,
            input::BUTTON_START,
            input::BUTTON_SELECT,
            input::BUTTON_LSTICK,
            input::BUTTON_RSTICK,
            input::BUTTON_LBUMPER,
            input::BUTTON_RBUMPER,
            input::BUTTON_HOME,
            input::BUTTON_A,
            input::BUTTON_B,
            input::BUTTON_X,
            input::BUTTON_Y
        };
        constexpr unsigned NUM_BUTTONS = 15;
        constexpr input::GamepadAxis axes[] = {
            input::AXIS_LY,
            input::AXIS_LX,
            input::AXIS_RX,
            input::AXIS_RY,
            input::AXIS_LTRIGGER,
            input::AXIS_RTRIGGER
        };
        constexpr unsigned NUM_AXES = 6;
        constexpr double   LAXIS_DEADZONE = 0.17;
        constexpr double   RAXIS_DEADZONE = 0.17;
        constexpr double   TRIGGER_DEADZONE = 0.1;
        constexpr bool     FLIP_LY = false;
        constexpr bool     FLIP_RY = false;
        constexpr bool     CLAMP_TRIGGERS_TO_0_1 = true;
    };
    
#define EXPOSE_FIELD(field_name) \
static constexpr const decltype(xbox_controller::field_name)& field_name (GamepadProfile profile) { \
    switch(profile) { \
        case GamepadProfile::DUALSHOCK_3_PROFILE: \
        case GamepadProfile::DUALSHOCK_4_PROFILE: \
        case GamepadProfile::UNKNOWN_PROFILE: \
        case GamepadProfile::XBOX_PROFILE: return xbox_controller::field_name; \
    } \
}
    EXPOSE_FIELD(buttons)
    EXPOSE_FIELD(axes)
    EXPOSE_FIELD(NUM_BUTTONS)
    EXPOSE_FIELD(NUM_AXES)
    EXPOSE_FIELD(LAXIS_DEADZONE)
    EXPOSE_FIELD(RAXIS_DEADZONE)
    EXPOSE_FIELD(TRIGGER_DEADZONE)
    EXPOSE_FIELD(FLIP_LY)
    EXPOSE_FIELD(FLIP_RY)
    EXPOSE_FIELD(CLAMP_TRIGGERS_TO_0_1)
    static const double DEADZONE_FOR_AXIS (GamepadProfile profile, input::GamepadAxis axis) {
        switch (axis) {
            case input::AXIS_LX: case input::AXIS_LY: return LAXIS_DEADZONE(profile);
            case input::AXIS_RX: case input::AXIS_RY: return RAXIS_DEADZONE(profile);
            case input::AXIS_LTRIGGER: case input::AXIS_RTRIGGER: return TRIGGER_DEADZONE(profile);
            case input::AXIS_DPAD_X: case input::AXIS_DPAD_Y: return 0.0;
        }
    }
    
#undef EXPOSE_FIELD
    
}; // namespace gamepad_profiles
}; // namespace gl_sandbox

InputManager::InputManager () {}

void InputManager::update () {
    using namespace input;
    
    // used to integrate multiple gamepad axis values into m_combinedAxesState.
    // Gets set for an axis
    std::bitset<input::NUM_GAMEPAD_AXES> setAxes;
    
    for (auto i = 0; i < m_gamepadStates.size(); ++i) {
        auto present = glfwJoystickPresent(i);
        auto & state = m_gamepadStates[i];
        
        if (present != state.active || state.name != glfwGetJoystickName(i)) {
            state.active = present;
            if (present) {
                state.name = glfwGetJoystickName(i);
                state.profile = getMatchingProfile(state.name);
                onDeviceConnected.emit(state.name);
                if (state.profile == GamepadProfile::UNKNOWN_PROFILE)
                    std::cerr << "input: Unknown profile for gamepad device '" << state.name << "'\n";
            } else {
                onDeviceDisconnected.emit(state.name);
            }
        } else if (present && state.name != glfwGetJoystickName(i)) {
            std::cerr << "Warning: GLFW Joystick name changed (from '" << state.name << "' to '" << glfwGetJoystickName(i) << "'\n";
            state.name = glfwGetJoystickName(i);
        }
        if (present && state.profile != GamepadProfile::UNKNOWN_PROFILE) {
            int naxes, nbuttons;
            auto axes = glfwGetJoystickAxes(i, &naxes);
            auto buttons = glfwGetJoystickButtons(i, &nbuttons);
            
            // trigger axes count as 2 extra buttons
            assert(nbuttons + 2 >= input::NUM_GAMEPAD_BUTTONS);
            
            // Update buttons + fire events
            // Note: this code will have _terrible_ cache performance, but... oh well >.>
            for (auto k = nbuttons; k --> 0; ) {
                auto button = gamepad_profiles::buttons(state.profile)[k];
                auto pressed = buttons[k];
                if (pressed != m_buttonPressState[button]) {
                    (m_buttonPressState[button] = pressed) ?
                        onGamepadButtonPressed.emit(button) :
                        onGamepadButtonReleased.emit(button);
                }
            }
            
            // dpad counts as 2 extra axes
            assert(naxes + 2 >= input::NUM_GAMEPAD_AXES);
            
            // Update axes
            float rawAxes [naxes];
            for (auto k = naxes; k --> 0; ) {
                auto axis = gamepad_profiles::axes(state.profile)[k];
                rawAxes[axis] = axes[k];
            }
            auto setAxis = [&](GamepadAxis axis, float value) {
                state.lastAxesState[axis] =
                    fabs(value) > gamepad_profiles::DEADZONE_FOR_AXIS(state.profile, axis) ?
                    value : 0.0;
            };
            
            // Set left + right stick inputs, flipping y-axes iff configured
            setAxis(AXIS_LX, rawAxes[AXIS_LX]);
            setAxis(AXIS_LY, gamepad_profiles::FLIP_LY(state.profile) ? -rawAxes[AXIS_LY] : rawAxes[AXIS_LY]);
            setAxis(AXIS_RX, rawAxes[AXIS_RX]);
            setAxis(AXIS_RY, gamepad_profiles::FLIP_RY(state.profile) ? -rawAxes[AXIS_RY] : rawAxes[AXIS_RY]);
            
            // Set left + right trigger inputs, converting from [-1,1] to [0,1] iff configured
            setAxis(AXIS_LTRIGGER,
                    gamepad_profiles::CLAMP_TRIGGERS_TO_0_1(state.profile) ?
                        rawAxes[AXIS_LTRIGGER] * 0.5 + 0.5 :
                        rawAxes[AXIS_LTRIGGER]);
            setAxis(AXIS_RTRIGGER,
                    gamepad_profiles::CLAMP_TRIGGERS_TO_0_1(state.profile) ?
                        rawAxes[AXIS_RTRIGGER] * 0.5 + 0.5 :
                        rawAxes[AXIS_RTRIGGER]);
            
            // Set dpad inputs as an extra 2d axis
            setAxis(AXIS_DPAD_X,
                    m_buttonPressState[BUTTON_DPAD_LEFT] ? -1.0 :
                    m_buttonPressState[BUTTON_DPAD_RIGHT] ? 1.0 : 0.0);
            setAxis(AXIS_DPAD_Y,
                    m_buttonPressState[BUTTON_DPAD_DOWN] ? -1.0 :
                    m_buttonPressState[BUTTON_DPAD_UP] ? 1.0 : 0.0);
            
            // Update trigger buttons (triggers axes masquerading as buttons)
            auto ltrigger_pressed = state.lastAxesState[AXIS_LTRIGGER] > gamepad_profiles::TRIGGER_DEADZONE(state.profile);
            auto rtrigger_pressed = state.lastAxesState[AXIS_RTRIGGER] > gamepad_profiles::TRIGGER_DEADZONE(state.profile);
            
            if (ltrigger_pressed != m_buttonPressState[BUTTON_LTRIGGER]) {
                (m_buttonPressState[BUTTON_LTRIGGER] = ltrigger_pressed) ?
                    onGamepadButtonPressed.emit(BUTTON_LTRIGGER) :
                    onGamepadButtonReleased.emit(BUTTON_LTRIGGER);
            }
            if (rtrigger_pressed != m_buttonPressState[BUTTON_RTRIGGER]) {
                (m_buttonPressState[BUTTON_RTRIGGER] = rtrigger_pressed) ?
                    onGamepadButtonPressed.emit(BUTTON_RTRIGGER) :
                    onGamepadButtonReleased.emit(BUTTON_RTRIGGER);
            }
            
            // Integrate axis values
            for (auto k = input::NUM_GAMEPAD_AXES; k --> 0; ) {
                if (!setAxes[k]) {
                    m_combinedAxesState[k] = state.lastAxesState[k];
                    if (fabs(state.lastAxesState[k]) > 0.0) {
                        setAxes[k] = true;
                    }
                }
            }
        }
    } // for each state : m_gamepadStates
    
    // Broadcast gamepad axis values
    onGamepadAxesUpdate.emit(&m_combinedAxesState[0]);
}

//void debugScanGamepadInput () {
//    using namespace gamepad_profiles::xbox_controller;
//    
//    for (auto i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; ++i) {
//        if (glfwJoystickPresent(i)) {
//            
//            int naxes, nbuttons;
//            auto axes = glfwGetJoystickAxes(i, &naxes);
//            auto buttons = glfwGetJoystickButtons(i, &nbuttons);
//            
//            //            std::cout << glfwGetJoystickName(i) << std::endl;
//            
//            bool any = false;
//            std::stringstream ss;
//            
//#define CHECK_BTN(x) if (x < nbuttons && buttons[x]) any = true, ss << #x " ";
//            if (buttons) {
//                CHECK_BTN(BUTTON_A)
//                CHECK_BTN(BUTTON_B)
//                CHECK_BTN(BUTTON_X)
//                CHECK_BTN(BUTTON_Y)
//                CHECK_BTN(BUTTON_DPAD_LEFT)
//                CHECK_BTN(BUTTON_DPAD_RIGHT)
//                CHECK_BTN(BUTTON_DPAD_UP)
//                CHECK_BTN(BUTTON_DPAD_DOWN)
//                CHECK_BTN(BUTTON_LT)
//                CHECK_BTN(BUTTON_RT)
//                CHECK_BTN(BUTTON_RB)
//                CHECK_BTN(BUTTON_LB)
//                CHECK_BTN(BUTTON_SELECT)
//                CHECK_BTN(BUTTON_START)
//                CHECK_BTN(BUTTON_HOME)
//            }
//#undef CHECK_BTN
//            
//#define CHECK_AXIS(x) if (x < naxes && (fabs(axes[x]) > THRESHOLD)) any = true, ss << #x " " << axes[x] << ' ';
//#define CHECK_TRIGGER(x) if (x < naxes && (axes[x] > (-1 + THRESHOLD))) any = true, ss << #x " " << axes[x] << ' ';
//            
//            const static float THRESHOLD = 0.17;
//            if (axes) {
//                CHECK_AXIS(LX_AXIS);
//                CHECK_AXIS(LY_AXIS);
//                CHECK_AXIS(RX_AXIS);
//                CHECK_AXIS(RY_AXIS);
//                CHECK_TRIGGER(LT_AXIS);
//                CHECK_TRIGGER(RT_AXIS);
//            }
//            if (any) {
//                std::cout << "Joystick input: " << ss.str() << std::endl;
//            }
//            
//#undef CHECK_AXIS
//#undef CHECK_TRIGGER
//        }
//    }
//}


