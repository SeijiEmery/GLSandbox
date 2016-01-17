//
//  JSInstance.hpp
//  GLSandbox
//
//  Created by semery on 1/16/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef JSInstance_hpp
#define JSInstance_hpp

#include "jsapi.h"

namespace gl_sandbox {
    
using namespace JS;
    
// Derive singleton Application instance from this -- calls JS_Init / JS_Shutdown
class JSSingleEngineInstance {
protected:
    JSSingleEngineInstance ();
    ~JSSingleEngineInstance ();
};

class JSInstance {
    JSInstance ();
    ~JSInstance ();
    JSInstance (const JSInstance &) = delete;
    JSInstance (JSInstance &&) = default;
    JSInstance & operator= (const JSInstance &) = delete;
    JSInstance & operator= (JSInstance &&) = default;
    
    void executeScript (const char * filename);
    
    JSRuntime * m_runtime = nullptr;
    JSContext * m_context = nullptr;
};
    
};

#endif /* JSInstance_hpp */
