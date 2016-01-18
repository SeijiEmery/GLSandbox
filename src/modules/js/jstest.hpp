//
//  jstest.hpp
//  GLSandbox
//
//  Created by semery on 1/17/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef jstest_hpp
#define jstest_hpp

#include "../module.hpp"
#include "../../common/thread_worker.hpp"
#include "../../common/js/js_thread_instance.hpp"

namespace gl_sandbox {

class JSTestModule : public Module<JSTestModule> {
public:
    JSTestModule ();
    ~JSTestModule ();
    
    static constexpr const char * MODULE_NAME = "module-js-test";
    static constexpr const char * MODULE_DIR  = "js";
    
    void drawFrame () {}
protected:
    JSThreadedInstance m_jsInstance;
};

}; // namespace gl_sandbox

#endif /* jstest_hpp */
