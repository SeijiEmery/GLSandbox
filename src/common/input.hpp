//
//  input.hpp
//  GLSandbox
//
//  Created by semery on 12/23/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef input_hpp
#define input_hpp

#include "raii_signal.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <array>
#include <bitset>

namespace gl_sandbox {

namespace input {
enum GamepadButton {
    BUTTON_A = 0,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_DPAD_UP,
    BUTTON_DPAD_DOWN,
    BUTTON_DPAD_LEFT,
    BUTTON_DPAD_RIGHT,
    BUTTON_LTRIGGER, // triggers can be treated as buttons
    BUTTON_RTRIGGER,
    BUTTON_LBUMPER,
    BUTTON_RBUMPER,
    BUTTON_LSTICK,
    BUTTON_RSTICK,
    BUTTON_START,
    BUTTON_SELECT,
    BUTTON_HOME,
};
constexpr unsigned NUM_GAMEPAD_BUTTONS = 17;
    
enum GamepadAxis {
    AXIS_LX = 0,
    AXIS_LY,
    AXIS_RX,
    AXIS_RY,
    AXIS_LTRIGGER,
    AXIS_RTRIGGER,
    AXIS_DPAD_X,    // dpad can be interpreted as separate buttons or a 2d axis
    AXIS_DPAD_Y
};
constexpr unsigned NUM_GAMEPAD_AXES = 8;

enum class GamepadProfile {
    UNKNOWN_PROFILE,
    XBOX_PROFILE,
    DUALSHOCK_3_PROFILE,
    DUALSHOCK_4_PROFILE
};
    
const char * gamepadButtonToString (GamepadButton button);
const char * gamepadAxisToString (GamepadAxis axis);
const char * gamepadProfileToString (GamepadProfile profile);
    
}; // namespace input
    
class InputManager {
public:
    InputManager ();
    InputManager (const InputManager &) = delete;
    InputManager (InputManager &&) = default;
    InputManager & operator= (const InputManager &) = delete;
    InputManager & operator= (InputManager &&) = default;
    
    typedef raii::Observer<const std::string&, input::GamepadProfile> DeviceConnectedObserver;
    typedef raii::Observer<input::GamepadButton> GamepadButtonObserver;
    typedef raii::Observer<const float*> GamepadAxesObserver;

    raii::Signal<const std::string&, input::GamepadProfile> onDeviceConnected;
    raii::Signal<const std::string&, input::GamepadProfile> onDeviceDisconnected;
    raii::Signal<input::GamepadButton> onGamepadButtonPressed;
    raii::Signal<input::GamepadButton> onGamepadButtonReleased;
    raii::Signal<const float *>        onGamepadAxesUpdate;

    void update ();
protected:
    // Note: we do not support coop-style discrete inputs (ie. separate input channels), but we do
    // support multiple controller inputs from glfw. If two gamepads are plugged in, all button
    // presses are merged into m_buttonPressState, and non-dead axis inputs (so long as they don't
    // overlap) are merged into m_combinedAxes state, where they get broadcast as input events.
    // You could effectively use 2+ controllers at once, provided that they didn't overlap
    // (ie. left stick on gamepad 1, right stick on gamepad 2, etc)
    struct GamepadState {
        std::string name;
        input::GamepadProfile profile = input::GamepadProfile::UNKNOWN_PROFILE;
        bool active = false;
        std::array<float, input::NUM_GAMEPAD_AXES> lastAxesState;
    };
    std::array<GamepadState, GLFW_JOYSTICK_LAST+1> m_gamepadStates;
    std::bitset<input::NUM_GAMEPAD_BUTTONS>    m_buttonPressState;
    std::array<float, input::NUM_GAMEPAD_AXES> m_combinedAxesState;
};

}; // namespace gl_sandbox

#endif /* input_hpp */
