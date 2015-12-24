//
//  camera.hpp
//  GLSandbox
//
//  Created by semery on 12/21/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef camera_hpp
#define camera_hpp

#include <glm/glm.hpp>

namespace gl_sandbox {

struct Camera {
    glm::mat4x4 proj;
    glm::mat4x4 view;

    struct Viewport {
        int x, y, width, height;
    } viewport;
    float aspect_ratio;
};

}; // namespace gl_sandbox

#endif /* camera_hpp */
