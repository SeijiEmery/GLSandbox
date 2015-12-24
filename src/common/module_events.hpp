//
//  module_events.hpp
//  GLSandbox
//
//  Created by semery on 12/23/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef module_events_hpp
#define module_events_hpp

namespace gl_sandbox {

class InputManager;
class AppEvents;

struct ModuleExposedEvents {
    ModuleExposedEvents ();
    
    InputManager * input;
    AppEvents    * app;
};

}; // namespace gl_sandbox
    
    
#endif /* module_events_hpp */
