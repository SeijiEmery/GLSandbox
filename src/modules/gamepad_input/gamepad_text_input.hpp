//
//  gamepad_text_input.hpp
//  GLSandbox
//
//  Created by semery on 1/10/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef gamepad_text_input_hpp
#define gamepad_text_input_hpp

#include "../../common/input.hpp"
#include "../module.hpp"

namespace gl_sandbox {
namespace modules {
    
//enum class RTVisibility {
//    USER_VISIBLE,
//    USER_HIDDEN
//};

static constexpr unsigned USER_HIDDEN = 1;

template <typename T>
class RTProperty {
public:
    RTProperty (std::string name, unsigned flags = 0)
        : m_name(name), m_flags(flags) {}
    
protected:
    std::string  m_name;
    unsigned     m_flags;
    T m_value;
};

struct GamepadTextInputModule : public Module<GamepadTextInputModule> {
    GamepadTextInputModule ();
//    ~GamepadTextInputModule ();
    
    void drawFrame () override;
    
    static constexpr const char * MODULE_NAME = "gamepad-input-test";
    static constexpr const char * MODULE_DIR  = "gamepad_input";
    
protected:
    void onGamepadUpdate ();
    
    RTProperty<int>   m_foo { "settings/gamepadinput/foo" };
    RTProperty<float> m_bar { "settings/gamepadinput/bar", USER_HIDDEN };
    
private:
    InputManager::GamepadButtonObserver m_onGamepadButtonPressed;
    InputManager::GamepadAxesObserver   m_onGamepadAxesUpdate;
    
    int m_lastSectors [2] { 0, 0 };
};
    

    

    
    
    
}; // namespace modules
}; // namespace gl_sandbox

#endif /* gamepad_text_input_hpp */
