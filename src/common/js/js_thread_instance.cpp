//
//  js_thread_instance.cpp
//  GLSandbox
//
//  Created by semery on 1/16/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include "js_thread_instance.hpp"
#include <string>
#include <iostream>

using namespace gl_sandbox;

void js_reportError (JSContext * cx, const char * message, JSErrorReport * report) {
    fprintf(stderr, "%s:%u:%s\n",
            report->filename ? report->filename : "[no filename]",
            (unsigned int) report->lineno,
            message);
}

static JSClass g_globalClass = {
    "global",
    JSCLASS_GLOBAL_FLAGS
};

static bool fcn_print (JSContext* cx, unsigned argc, JS::Value* vp) {
    
    std::cout << "foo\n";
    
//    auto args = JS_ARGV(cx, vp);
//    for (auto i = 0; i < argc; ++i) {
//        
//    }
//    jsval rv;
//    rv.setNaN();
//    JS_SET_RVAL(cx, vp, rv);
    return true;
    
//    JSNATIVE_WRAPPER()
}



static JSFunctionSpec g_jsGlobalFunctions[] = {
    JS_FS("print", fcn_print, 1, 0)
};

void JSThreadWorker::onThreadBegin () {
    std::cout << "Starting js worker\n";
    
    m_runtime = JS_NewRuntime(8L * 1024L * 1024L);
    if (!m_runtime)
        throw new std::runtime_error("Failed to create js runtime");
    
    m_context = JS_NewContext(m_runtime, 8192);
    if (!m_context)
        throw new std::runtime_error("Failed to create js context");
    JS_SetErrorReporter(m_runtime, js_reportError);
    
    JSAutoRequest ar (m_context);
    
//    RootedObject global (m_context);
    m_global = JS_NewGlobalObject(m_context, &g_globalClass, nullptr, JS::DontFireOnNewGlobalHook);
    
    if (!m_global)
        throw new std::runtime_error("js instance failed to create global object");
    
    JSAutoCompartment ac (m_context, m_global);
    
    if (!JS_InitStandardClasses(m_context, m_global))
        throw new std::runtime_error("js instance failed to initialize global object");
    
    if (!JS_DefineFunctions(m_context, m_global, g_jsGlobalFunctions))
        throw new std::runtime_error("js instance failed to set global functions");
}

void JSThreadWorker::onThreadExit () {
    std::cout << "Terminating js worker\n";
    
    JS_DestroyContext(m_context);
    JS_DestroyRuntime(m_runtime);
}

void JSThreadWorker::onThreadException(const std::runtime_error &e) {
    std::cerr << "EXCEPTION! on js worker thread:\n\t" << e.what() << '\n';
}

void JSThreadWorker::eval (const std::string & contents) {
    exec([=](JSThreadWorker & _) -> void {
        
        JS::RootedValue rv (m_context);
        
        JS::CompileOptions options (m_context);
        options.setVersion(JSVERSION_LATEST);
        
        if (!JS::Evaluate(m_context, m_global, options, contents.c_str(), contents.size(), &rv)) {
            
        }
    });
}
