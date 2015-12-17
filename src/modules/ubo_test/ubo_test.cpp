//
//  ubo_test.cpp
//  GLSandbox
//
//  Created by semery on 12/16/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "ubo_test.hpp"

using namespace gl_sandbox::modules;
using namespace gl_sandbox::gl;

const char * UboDynamicModule::MODULE_NAME = "ubo-dynamic-test";
const char * UboDynamicModule::MODULE_DIR =  "ubo_test";
const char * UboStaticModule::MODULE_NAME = "ubo-static-test";
const char * UboStaticModule::MODULE_DIR = "ubo_test";

template <typename T>
void init (T & ctx) {
    ctx.ubo.bufferData(ctx.foo);
}

template <typename T>
void drawFrame (T & context) {
    
}

template <typename T>
void deinit (T & context) {
    
}

struct Foo {
    glm::vec3 innerColor;
    glm::vec3 outerColor;
    float radiusInner;
    float radiusOuter;
    
    Foo () {}
};

std::unique_ptr<Shader> createShader (gl_sandbox::ResourceLoader *resourceLoader, const std::string & shaderName) {
    Shader * shader = new Shader(shaderName.c_str());
    resourceLoader->loadTextFile(shaderName + ".fs", [shader] (const char * src) {
        shader->compileFragment(src);
    });
    resourceLoader->loadTextFile(shaderName + ".vs", [shader] (const char * src) {
        shader->compileVertex(src);
    });
    if (shader->linkProgram()) {
        std::cout << "Successfully loaded shader '" << shader->name << "'\n";
    } else {
        std::cout << "Failed to load shader '" << shader->name << "'\n";
    }
    return std::unique_ptr<Shader> { shader };
}

struct gl_sandbox::modules::UboDynamicImpl {
private:
public:
    UboDynamicImpl (const UboDynamicModule & module) :
        shader(std::move(createShader(module.resourceLoader, "ubo_test"))),
        ubo { *shader, "BlobSettings", (Foo*)&foo, {
            { "InnerColor", sizeof(foo.innerColor), &foo.innerColor },
            { "OuterColor", sizeof(foo.outerColor), &foo.outerColor },
            { "RadiusInner", sizeof(foo.radiusInner), &foo.radiusInner },
            { "RadiusOuter", sizeof(foo.radiusOuter), &foo.radiusOuter }
        }}
    {
        ::init(*this);
    }
    Foo foo;
    std::unique_ptr<Shader> shader;
    ubo_dynamic::UniformBuffer<Foo> ubo;
};
struct gl_sandbox::modules::UboStaticImpl {
    UboStaticImpl (const UboStaticModule & module) :
        shader(std::move(createShader(module.resourceLoader, "ubo_test"))),
        ubo { *shader, "BlobSettings" }
    {
        ::init(*this);
    }
    Foo foo;
    std::unique_ptr<Shader> shader;
    ubo_fast::UniformBuffer<Foo> ubo;
};

UboDynamicModule::UboDynamicModule (const ModuleConstructorArgs & args)
    : Module(args, MODULE_NAME, MODULE_DIR), impl(new UboDynamicImpl(*this))
{
    std::cout << "Initializing UBO dynamic test\n";
}

UboStaticModule::UboStaticModule (const ModuleConstructorArgs & args)
    : Module(args, MODULE_NAME, MODULE_DIR), impl(new UboStaticImpl(*this))
{
    std::cout << "Initializing UBO static / fast test\n";
}

void UboDynamicModule::drawFrame () {
    ::drawFrame(*impl);
}
void UboStaticModule::drawFrame () {
    ::drawFrame(*impl);
}

UboDynamicModule::~UboDynamicModule() {
    std::cout << "Killing UBO dynamic test\n";
    ::deinit(*impl);
}
UboStaticModule::~UboStaticModule () {
    std::cout << "Killing UBO static / fast test\n";
    ::deinit(*impl);
}





