//
//  camera.hpp
//  GLSandbox
//
//  Created by semery on 12/21/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef camera_hpp
#define camera_hpp

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace gl_sandbox {

struct Camera {
    glm::mat4x4 proj;
    glm::mat4x4 view;
};

struct CameraController {
    CameraController ();
    CameraController (Camera * camera);
    CameraController (const CameraController &) = delete;
    CameraController (CameraController &&) = default;
    
    void update ();
private:
    Camera * m_camera = nullptr;
    double   m_lastUpdate;
};
    
}; // namespace gl_sandbox

#endif /* camera_hpp */
