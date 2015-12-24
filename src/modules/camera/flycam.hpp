//
//  flycam.hpp
//  GLSandbox
//
//  Created by semery on 12/24/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef module_flycam_hpp
#define module_flycam_hpp

#include <memory>
#include "../module.hpp"
#include "../../common/input.hpp"
#include "../../common/camera.hpp"

namespace gl_sandbox {
namespace modules {
    
struct CameraModule : public Module<CameraModule> {
    CameraModule ();
    void drawFrame () override {}
    
    static constexpr const char * MODULE_NAME = "module-flycam";
    static constexpr const char * MODULE_DIR  = "flycam";
    
private:
    void onButtonPressed (input::GamepadButton button);
    void onGamepadUpdate (const float * axes);
    
    Camera * m_camera;
    InputManager::GamepadButtonObserver m_buttonObserver;
    InputManager::GamepadAxesObserver   m_axisObserver;
    
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraAngles;
    
    const float CAMERA_SPEED = 15.0;
    const float CAMERA_TURN_SPEED = 180;
    
    double m_lastUpdate;
};

}; // namespace modules
}; // namespace gl_sandbox

#endif /* module_flycam_hpp */
