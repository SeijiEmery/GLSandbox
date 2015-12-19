//
//  ubo_test.hpp
//  GLSandbox
//
//  Created by semery on 12/16/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef ubo_test_hpp
#define ubo_test_hpp

#include "../module.hpp"
#include "../../common/gl/gl_wrapper.hpp"

namespace gl_sandbox {
namespace modules {

class UboDynamicImpl;
class UboStaticImpl;
    
struct UboDynamicModule : public Module<UboDynamicModule> {
public:
    UboDynamicModule ();
    ~UboDynamicModule ();
    void drawFrame () override;
    
    static const char * MODULE_NAME;
    static const char * MODULE_DIR;
private:
    std::unique_ptr<UboDynamicImpl> impl;
};
    
class UboStaticModule : public Module<UboStaticModule> {
public:
    UboStaticModule ();
    ~UboStaticModule ();
    void drawFrame () override;
    
    static const char * MODULE_NAME;
    static const char * MODULE_DIR;
protected:
    std::unique_ptr<UboStaticImpl> impl;
};

}; // namespace modules
}; // namespace gl_sandbox

#endif /* ubo_test_hpp */
