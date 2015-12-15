//
//  triangles.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef triangles_hpp
#define triangles_hpp

#include "../module.hpp"

namespace gl_sandbox {
class TriangleModule : public Module {
public:
    TriangleModule ();
    ~TriangleModule ();
    void drawFrame () override;
    
    static const char * MODULE_NAME;
};
    
}; // namespace gl_sandbox

#endif /* triangles_hpp */
