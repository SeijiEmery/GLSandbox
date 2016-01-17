//
//  JSInstance.cpp
//  GLSandbox
//
//  Created by semery on 1/16/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include "JSInstance.hpp"
#include <stdexcept>
#include <iostream>

using namespace gl_sandbox;

JSSingleEngineInstance::JSSingleEngineInstance () {
    std::cout << "Initializing spidermonkey\n";
    if (!JS_Init()) {
        throw new std::runtime_error("Failed to initialize js engine");
    }
}
JSSingleEngineInstance::~JSSingleEngineInstance() {
    std::cout << "Killing spidermonkey\n";
    JS_ShutDown();
}

void reportError (JSContext * cx, const char * message, JSErrorReport * report) {
    fprintf(stderr, "%s:%u:%s\n",
            report->filename ? report->filename : "[no filename]",
            (unsigned int) report->lineno,
            message);
}

static JSClass g_globalClass = {
    "global",
    JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub
};

JSInstance::JSInstance () {
    m_runtime = JS_NewRuntime(8L * 1024L * 1024L);
    if (!m_runtime)
        throw new std::runtime_error("Failed to create js runtime");
    
    m_context = JS_NewContext(m_runtime, 8192);
    if (!m_context)
        throw new std::runtime_error("Failed to create js context");
    
    JSAutoRequest ar (m_context);
    
    RootedObject global (m_context);
    global = JS_NewGlobalObject(m_context, &g_globalClass, nullptr, JS::DontFireOnNewGlobalHook);
    
    if (!global)
        throw new std::runtime_error("js instance failed to create global object");
    
    JSAutoCompartment ac (m_context, global);
    
    if (!JS_InitStandardClasses(m_context, global))
        throw new std::runtime_error("js instance failed to initialize global object");
}

JSInstance::~JSInstance () {
    JS_DestroyContext(m_context);
    JS_DestroyRuntime(m_runtime);
}

void JSInstance::executeScript (const char * filename) {
    
}




















