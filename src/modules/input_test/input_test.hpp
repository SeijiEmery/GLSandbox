//
//  input_test.hpp
//  GLSandbox
//
//  Created by semery on 12/23/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef input_test_hpp
#define input_test_hpp

#include "../../common/input.hpp"
#include "../module.hpp"

namespace gl_sandbox {
namespace modules {

struct InputTestModule : public Module<InputTestModule> {
    InputTestModule ();
    ~InputTestModule ();
    void drawFrame () override {}
    
    static constexpr const char * MODULE_NAME = "module-input-logger";
    static constexpr const char * MODULE_DIRNAME = "input_test";
private:
    InputManager::DeviceConnectedObserver m_deviceConnectedObserver, m_deviceDisconnectedObserver;
    InputManager::GamepadButtonObserver   m_gamepadButtonPressedObserver, m_gamepadButtonReleasedObserver;
    InputManager::GamepadAxesObserver     m_gamepadAxesObserver;
};
    
}; // namespace modules
}; // namespace gl_sandbox

    
#endif /* input_test_hpp */
