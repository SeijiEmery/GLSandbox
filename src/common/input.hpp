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
    
    
constexpr const char * gamepadButtonToString (GamepadButton button);
constexpr const char * gamepadAxisToString (GamepadAxis axis);
    
}; // namespace input
    
    
enum class GamepadProfile {
    UNKNOWN_PROFILE,
    XBOX_PROFILE,
    DUALSHOCK_3_PROFILE,
    DUALSHOCK_4_PROFILE
};
    
namespace gamepad_profiles {
    struct xbox_controller {
        constexpr static input::GamepadButton buttons[] = {
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
        constexpr static unsigned NUM_BUTTONS = 15;
        constexpr static input::GamepadAxis axes[] = {
            input::AXIS_LY,
            input::AXIS_LX,
            input::AXIS_RX,
            input::AXIS_RY,
            input::AXIS_LTRIGGER,
            input::AXIS_RTRIGGER
        };
        constexpr static unsigned NUM_AXES = 6;
        constexpr static double   LAXIS_DEADZONE = 0.17;
        constexpr static double   RAXIS_DEADZONE = 0.17;
        constexpr static double   TRIGGER_DEADZONE = 0.1;
        constexpr static bool     FLIP_LY = false;
        constexpr static bool     FLIP_RY = false;
    };
};
    
class InputManager {
public:
    InputManager ();
    InputManager (const InputManager &) = delete;
    InputManager (InputManager &&) = default;
    InputManager & operator= (const InputManager &) = delete;
    InputManager & operator= (InputManager &&) = default;
    
    raii::Signal<const std::string&> onDeviceConnected;
    raii::Signal<const std::string&> onDeviceDisconnected;
    raii::Signal<input::GamepadButton> onGamepadButtonPressed;
    raii::Signal<input::GamepadButton> onGamepadButtonReleased;
    raii::Signal<const float *>        onGamepadAxesUpdate;

    void update ();
protected:
    struct GamepadState {
        std::string name;
        GamepadProfile profile = GamepadProfile::UNKNOWN_PROFILE;
        bool active = false;
        std::array<float, input::NUM_GAMEPAD_AXES> lastAxesState;
    };
    std::array<GamepadState, GLFW_JOYSTICK_LAST+1> m_gamepadStates;
    std::bitset<input::NUM_GAMEPAD_BUTTONS>    m_buttonPressState;
    std::array<float, input::NUM_GAMEPAD_AXES> m_combinedAxesState;
};

}; // namespace gl_sandbox

#endif /* input_hpp */
