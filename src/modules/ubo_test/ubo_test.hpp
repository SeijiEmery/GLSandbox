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
    
//template <class T, class Impl>
//class PimplModule : public Module {
//    PimplModule (const ModuleConstructorArgs & args) :
//        Module(args, T::MODULE_NAME, T::MODULE_DIR),
//        m_impl(static_cast<Module&>(*this)) {}
//public:
//    ~PimplModule () {}
//    
//    static Module * construct (const ModuleConstructorArgs & args) {
//        return static_cast<Module*>(new T(args));
//    }
//    void drawFrame () override {
//        return m_impl->drawFrame(static_cast<Module&>(*this));
//    }
//protected:
//    std::unique_ptr<Impl> m_impl;
//};
//    
//class UboDynamicImpl;
//class UboStaticImpl;
//class UboDynamicModule;
//class UboStaticModule;
//    
//typedef PimplModule<UboDynamicModule,UboDynamicImpl> UboDynamicBase;
//
//struct UboDynamicModule : public UboDynamicBase {
//    using UboDynamicBase::UboDynamicBase;
//    static const char * MODULE_NAME;
//    static const char * MODULE_DIR;
//};
//struct UboStaticModule : public PimplModule<UboStaticModule,UboStaticImpl> {
//    using PimplModule<UboStaticModule,UboStaticImpl>::PimplModule;
//    static const char * MODULE_NAME;
//    static const char * MODULE_DIR;
//};
    
class UboDynamicImpl;
class UboStaticImpl;

class UboDynamicModule : public Module {
    UboDynamicModule (const ModuleConstructorArgs & args);
    friend class UboDynamicImpl;
public:
    ~UboDynamicModule ();
    
    static const char * MODULE_NAME;
    static const char * MODULE_DIR;
    
    static Module * construct (const ModuleConstructorArgs & args) {
        return static_cast<Module*>(new UboDynamicModule(args));
    }
    void drawFrame () override;
private:
    std::unique_ptr<UboDynamicImpl> impl;
};
class UboStaticModule : public Module {
    UboStaticModule (const ModuleConstructorArgs & args);
    friend class UboStaticImpl;
public:
    ~UboStaticModule ();
    
    static const char * MODULE_NAME;
    static const char * MODULE_DIR;
    
    static Module * construct (const ModuleConstructorArgs & args) {
        return static_cast<Module*>(new UboStaticModule(args));
    }
    void drawFrame () override;
protected:
    std::unique_ptr<UboStaticImpl> impl;
};

}; // namespace modules
}; // namespace gl_sandbox

#endif /* ubo_test_hpp */
