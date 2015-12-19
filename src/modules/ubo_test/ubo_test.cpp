//
//  ubo_test.cpp
//  GLSandbox
//
//  Created by semery on 12/16/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "ubo_test.hpp"

using namespace gl_sandbox;
using namespace gl_sandbox::modules;
using namespace gl_sandbox::gl;

const char * UboDynamicModule::MODULE_NAME = "ubo-dynamic-test";
const char * UboDynamicModule::MODULE_DIR =  "ubo_test";
const char * UboStaticModule::MODULE_NAME = "ubo-static-test";
const char * UboStaticModule::MODULE_DIR = "ubo_test";

struct Foo {
    glm::vec3 innerColor;
    glm::vec3 outerColor;
    float radiusInner;
    float radiusOuter;
    
    Foo () {}
};
std::unique_ptr<Shader> loadShader (ResourceLoader & resourceLoader, const std::string & shaderName);

template <typename T, typename M>
struct TestImpl {
protected:
    ResourceLoader m_resourceLoader { M::MODULE_DIR };
    Foo            m_foo;
    std::unique_ptr<Shader> m_shader;
    
#define m_ubo static_cast<T*>(this)->m_ubo
public:
    TestImpl () :
        m_shader(loadShader(m_resourceLoader, "ubo_test"))
    {}
    void drawFrame () {
        m_ubo.bufferData(m_foo);
    }
#undef m_ubo
};

std::unique_ptr<Shader> loadShader (ResourceLoader &resourceLoader, const std::string & shaderName) {
    Shader * shader = new Shader(shaderName.c_str());
    resourceLoader.loadTextFile(shaderName + ".fs", [shader] (const char * src) {
        shader->compileFragment(src);
    });
    resourceLoader.loadTextFile(shaderName + ".vs", [shader] (const char * src) {
        shader->compileVertex(src);
    });
    if (shader->linkProgram()) {
        std::cout << "Successfully loaded shader '" << shader->name << "'\n";
    } else {
        std::cout << "Failed to load shader '" << shader->name << "'\n";
    }
    return std::unique_ptr<Shader> { shader };
}

struct gl_sandbox::modules::UboDynamicImpl : public TestImpl<UboDynamicImpl, UboDynamicModule> {
public:
    UboDynamicImpl (const UboDynamicModule & module) :
        m_ubo { *m_shader, "BlobSettings", (Foo*)&m_foo, {
            { "InnerColor", sizeof(m_foo.innerColor), &m_foo.innerColor },
            { "OuterColor", sizeof(m_foo.outerColor), &m_foo.outerColor },
            { "RadiusInner", sizeof(m_foo.radiusInner), &m_foo.radiusInner },
            { "RadiusOuter", sizeof(m_foo.radiusOuter), &m_foo.radiusOuter }
        }}
    {}
public:
    ubo_dynamic::UniformBuffer<Foo> m_ubo;
};
struct gl_sandbox::modules::UboStaticImpl : public TestImpl<UboDynamicImpl, UboDynamicModule> {
    UboStaticImpl (const UboStaticModule & module) :
        m_ubo { *m_shader, "BlobSettings" }
    {}

public:
    ubo_fast::UniformBuffer<Foo> m_ubo;
};

UboDynamicModule::UboDynamicModule ()
    : impl(new UboDynamicImpl(*this))
{
    std::cout << "Initializing UBO dynamic test\n";
}

UboStaticModule::UboStaticModule ()
    : impl(new UboStaticImpl(*this))
{
    std::cout << "Initializing UBO static / fast test\n";
}

void UboDynamicModule::drawFrame () {
    impl->drawFrame();
}
void UboStaticModule::drawFrame () {
    impl->drawFrame();
}

UboDynamicModule::~UboDynamicModule() {
    std::cout << "Killing UBO dynamic test\n";
}
UboStaticModule::~UboStaticModule() {
    std::cout << "Killing UBO static / fast test\n";
}





