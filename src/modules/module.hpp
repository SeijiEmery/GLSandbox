//
//  module.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef module_h
#define module_h

#include <string>

namespace gl_sandbox {

struct Module {
    virtual void drawFrame () = 0;
    std::string name;
    
protected:
    Module (const char * name) : name(name) {}
    virtual ~Module () {}
};
    
}; // namespace gl_sandbox



#endif /* module_h */
