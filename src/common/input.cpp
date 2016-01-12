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
using namespace input;

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
const char * gl_sandbox::input::gamepadProfileToString(input::GamepadProfile profile) {
    switch (profile) {
        case GamepadProfile::UNKNOWN_PROFILE: return "unknown profile";
        case GamepadProfile::XBOX_PROFILE:    return "xbox gamepad";
        case GamepadProfile::DUALSHOCK_3_PROFILE: return "dualshock 3";
        case GamepadProfile::DUALSHOCK_4_PROFILE: return "dualshock 4";
    }
}

static GamepadProfile getMatchingProfile (const std::string & name, int naxes, int nbuttons) {
    if (name == "Xbox 360 Wired Controller" || (naxes == 6 && nbuttons == 15))
        return GamepadProfile::XBOX_PROFILE;
    if (name == "Wireless Controller" && naxes == 6 && nbuttons == 18)
        return GamepadProfile::DUALSHOCK_4_PROFILE;
    
    std::cerr << "input: Unknown profile for gamepad device '" << name << "', axes = " << naxes << ", buttons = " << nbuttons << '\n';
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
            input::AXIS_LX,
            input::AXIS_LY,
            input::AXIS_RX,
            input::AXIS_RY,
            input::AXIS_LTRIGGER,
            input::AXIS_RTRIGGER
        };
        constexpr unsigned NUM_AXES = 6;
        constexpr double   LAXIS_DEADZONE = 0.19;
        constexpr double   RAXIS_DEADZONE = 0.19;
        constexpr double   TRIGGER_DEADZONE = 0.1;
        constexpr bool     FLIP_LY = false;
        constexpr bool     FLIP_RY = false;
        constexpr bool     CLAMP_TRIGGERS_TO_0_1 = true;
    };
    namespace dualshock_4 {
        constexpr input::GamepadButton buttons[] = {
            input::BUTTON_X,  // square
            input::BUTTON_A,  // x
            input::BUTTON_B,  // circle
            input::BUTTON_Y,  // triangle
            input::BUTTON_LBUMPER,
            input::BUTTON_RBUMPER,
            input::BUTTON_LTRIGGER, // ds4 actually has triggers aliased as buttons, apparently
            input::BUTTON_RTRIGGER,
            input::BUTTON_START,
            input::BUTTON_SELECT, // share button
            input::BUTTON_LSTICK,
            input::BUTTON_RSTICK,
            input::BUTTON_HOME,
            input::BUTTON_SELECT, // center button
            input::BUTTON_DPAD_UP,
            input::BUTTON_DPAD_RIGHT,
            input::BUTTON_DPAD_DOWN,
            input::BUTTON_DPAD_LEFT,
        };
        constexpr unsigned NUM_BUTTONS = 15;
        constexpr input::GamepadAxis axes[] = {
            input::AXIS_LX,
            input::AXIS_LY,
            input::AXIS_RX,
            input::AXIS_RY,
            input::AXIS_LTRIGGER,
            input::AXIS_RTRIGGER
        };
        constexpr unsigned NUM_AXES = 6;
        constexpr double   LAXIS_DEADZONE = 0.06;
        constexpr double   RAXIS_DEADZONE = 0.06;
        constexpr double   TRIGGER_DEADZONE = 0.0;
        constexpr bool     FLIP_LY = false;
        constexpr bool     FLIP_RY = false;
        constexpr bool     CLAMP_TRIGGERS_TO_0_1 = true;
    };
    
#define EXPOSE_FIELD(field_name) \
static constexpr const auto field_name (GamepadProfile profile) { \
    switch(profile) { \
        case GamepadProfile::DUALSHOCK_3_PROFILE: \
        case GamepadProfile::DUALSHOCK_4_PROFILE: return dualshock_4::field_name; \
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
    std::bitset<input::NUM_GAMEPAD_BUTTONS> currentButtonsPressed;
    
    for (auto i = 0; i < m_gamepadStates.size(); ++i) {
        auto present = glfwJoystickPresent(i);
        auto & state = m_gamepadStates[i];
        
        if (present != state.active) {
            state.active = present;
            if (present) {
                state.name = glfwGetJoystickName(i);

                int naxes, nbuttons;
                glfwGetJoystickAxes(i, &naxes);
                glfwGetJoystickButtons(i, &nbuttons);
                state.profile = getMatchingProfile(state.name, naxes, nbuttons);
                onDeviceConnected.emit(state.name, state.profile);
            } else {
                onDeviceDisconnected.emit(state.name, state.profile);
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
            
            // Update buttons
            for (auto k = nbuttons; k --> 0; ) {
                auto button = gamepad_profiles::buttons(state.profile)[k];
                if (buttons[k])
                    currentButtonsPressed[button] = true;
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
            auto triggerDeadzone = gamepad_profiles::TRIGGER_DEADZONE(state.profile);
            if (state.lastAxesState[AXIS_LTRIGGER] > triggerDeadzone)
                currentButtonsPressed[BUTTON_LTRIGGER] = true;
            if (state.lastAxesState[AXIS_RTRIGGER] > triggerDeadzone)
                currentButtonsPressed[BUTTON_RTRIGGER] = true;
            
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
    
    // Broadcast button events
    for (auto i = NUM_GAMEPAD_BUTTONS; i --> 0; ) {
        if (currentButtonsPressed[i] != m_buttonPressState[i]) {
            (m_buttonPressState[i] = currentButtonsPressed[i]) ?
                onGamepadButtonPressed.emit((GamepadButton)i) :
                onGamepadButtonReleased.emit((GamepadButton)i);
        }
    }
    // Broadcast gamepad axis values
    onGamepadAxesUpdate.emit(&m_combinedAxesState[0]);
}


