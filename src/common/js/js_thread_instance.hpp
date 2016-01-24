//
//  js_thread_instance.hpp
//  GLSandbox
//
//  Created by semery on 1/16/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef js_thread_instance_hpp
#define js_thread_instance_hpp

#include "jsapi.h"
#include "js/Conversions.h"
//#include "jsarray.h"
//#include "jsfun.h"
//#include "jsobj.h"
//#include "jsscript.h"
//#include "jsutil.h"


#include "../thread_worker.hpp"
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
#include <cassert>

#include <boost/lockfree/queue.hpp>

using namespace JS;

namespace gl_sandbox {
    

// Provides an event loop, and one-directional dispatch system to run arbitrary callbacks on
// a target worker thread. Used to implement multithreaded scripts.
template <typename T>
struct ThreadedDispatchQueue {
protected:
    typedef std::function<void(T&)> message_type;
    typedef boost::lockfree::queue<message_type*,
        boost::lockfree::fixed_sized<true>,
        boost::lockfree::capacity<4196>
    > message_queue;
    
    T * m_target = nullptr;
    message_queue m_queue;
    
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::atomic<bool> m_isRunning { false };
    std::atomic<bool> m_isPaused  { false };
    
public:
    ThreadedDispatchQueue () {}
    ~ThreadedDispatchQueue () { kill(); }
    
    ThreadedDispatchQueue (const ThreadedDispatchQueue &) = delete;
    ThreadedDispatchQueue & operator= (const ThreadedDispatchQueue &) = delete;
    
    void setDispatchTarget (T * target) {
        std::cout << "dispatch target set from thread " << std::this_thread::get_id() << '\n';
        m_target = target;
    }
    void run () {
        assert(!m_isRunning);
        m_isRunning = true;
//        m_queue.consume_all([](message_type * task) { delete task; }); // clear queue
        
        std::cout << "running dispatch queue on thread " << std::this_thread::get_id() << '\n';
        do {
            while (m_isRunning && !m_queue.empty()) {
                std::cout << "dispatch queue executing call on thread " << std::this_thread::get_id() << '\n';
                message_type * task;
                m_queue.pop(task);
                (*task)(*m_target);
                delete task;
//                m_queue.consume_one([=](T & task) {
//                    task(*m_target);
//                });
            }
            std::unique_lock<std::mutex> lock (m_mutex);
            m_isPaused = true;
            while (m_isRunning && m_queue.empty()) {
                std::cout << "dispatch queue paused (" << std::this_thread::get_id() << ")\n";
                m_cv.wait(lock);
            }
            m_isPaused = false;
        } while (m_isRunning);
        
        std::cout << "dispatch queue stopped (" << std::this_thread::get_id() << ")\n";
    }
    void kill () {
        std::cout << "dispatched kill command from thread " << std::this_thread::get_id() << '\n';
        m_isRunning = false;
        if (m_isPaused)
            m_cv.notify_one();
    }
    void call (message_type fcn) {
        std::cout << "dispatched call from thread " << std::this_thread::get_id() << '\n';
        m_queue.push(new message_type(fcn));
        if (m_isPaused)
            m_cv.notify_one();
    }
    bool isRunning () const {
        return m_isRunning;
    }
};


// Simple wrapper class that is responsible for initializing, managing, and interacting with a
// JSRuntime + JSContext instance (mozjs/spidermonkey). Does not know / care about threads;
// that gets handled by JSThreadedInstance using a ThreadedDispatchQueue.
struct JSPlatform {
private:
    static JSRuntime * createRuntimeOrThrow (const std::string & msg) {
        auto rt = JS_NewRuntime(8L * 1024 * 1024);
        if (!rt)
            throw new std::runtime_error(msg);
        return rt;
    }
    static JSContext * createContextOrThrow (JSRuntime * rt, const std::string & msg) {
        auto cx = JS_NewContext(rt, 8192);
        if (!cx)
            throw new std::runtime_error(msg);
        return cx;
    }
    static JSObject * createGlobalObject (JSContext * cx) {
        static const JSClass globalClass = {
            "global",
            JSCLASS_GLOBAL_FLAGS
        };
        auto object = JS_NewGlobalObject(cx, &globalClass, nullptr, JS::DontFireOnNewGlobalHook);
        if (!object)
            throw std::runtime_error("Failed to create global object");
        return object;
    }
    
    
    static void reportError (JSContext * cx, const char * message, JSErrorReport * report) {
        fprintf(stderr, "JS ERROR: %s:%u:%s\n",
                report->filename ? report->filename : "[no filename]",
                (unsigned int) report->lineno,
                message);
    }
    static bool js_print (JSContext * cx, unsigned argc, JS::Value* vp) {
        CallArgs args = CallArgsFromVp(argc, vp);
        for (unsigned i = 0; i < args.length(); ++i) {
            RootedString str (cx, JS::ToString(cx, args[i]));
            if (!str)
                return false;
            char * bytes = JS_EncodeStringToUTF8(cx, str);
            if (!bytes)
                return false;
            std::cout << (i ? " " : "") << bytes;
        }
        std::cout << '\n';
        args.rval().setUndefined();
        return true;
    }
    
protected:
    JSRuntime * const m_runtime;
    JSContext * const m_context;
    RootedObject m_global;
    JSAutoCompartment m_compartment;

public:
    JSPlatform () :
        m_runtime(createRuntimeOrThrow("Failed to create js runtime")),
        m_context(createContextOrThrow(m_runtime, "Failed to create js context")),
        m_global(m_context, createGlobalObject(m_context)),
        m_compartment(m_context, m_global)
    {
        JS_SetErrorReporter(m_runtime, reportError);
        
//        RootedObject global (m_context);
//        m_global = JS_NewGlobalObject(m_context, &globalClass, nullptr, JS::DontFireOnNewGlobalHook);
//        if (!m_global)
//            throw new std::runtime_error("js instance failed to create global object");
        
//        JSAutoCompartment ac (m_context, global);
//        m_compartment = JS_EnterCompartment(m_context, m_global);
        if (!JS_InitStandardClasses(m_context, m_global))
            throw new std::runtime_error("js instance failed to initialize global object");
        
        static JSFunctionSpec jsGlobalFunctions[] = {
            JS_FS("print", js_print, 1, 0),
        };
    
        if (!JS_DefineFunctions(m_context, m_global, jsGlobalFunctions))
            throw new std::runtime_error("js instance failed to set global functions");
    }
    
    JSPlatform (const JSPlatform &) = delete;
    JSPlatform & operator= (const JSPlatform &) = delete;
    
    ~JSPlatform () {
//        JS_LeaveCompartment(m_context, m_compartment);
        JS_DestroyContext(m_context);
        JS_DestroyRuntime(m_runtime);
    }
    
    void eval (const std::string & contents) {
        
        assert(m_context && m_runtime);
        
        JSAutoRequest ar (m_context);
        
        std::cout << "Called eval on \"" << contents << "\"\n";
        
        JS::CompileOptions options(m_context);
                
        RootedScript script (m_context);
        if (!JS::Compile(m_context, m_global, options, contents.c_str(), contents.length(), &script)) {
            std::cout << "Failed to compile '" << contents << "'\n";
            return;
        }
        JSAutoCompartment ac (m_context, script);
        
        RootedValue result (m_context);
        if (!JS_ExecuteScript(m_context, m_global, script, &result)) {
            std::cout << "Failed to run '" << contents << "'\n";
            return;
        }
        
        if (!result.isUndefined()) {
            RootedString str(m_context);
            str = JS_ValueToSource(m_context, result);
            if (!str)
                return;
            char * utf8chars = JS_EncodeStringToUTF8(m_context, str);
            if (!utf8chars)
                return;
            std::cout << utf8chars << '\n';
            JS_free(m_context, utf8chars);
        }
    }
};
    
// Owns a thread running a JSPlatform instance, and provides a threadsafe handle / wrapper for interacting
// with it using a dispatch queue and callback objects.
struct JSThreadedInstance {
protected:
    ThreadedDispatchQueue<JSPlatform> m_dispatchQueue;
    std::thread m_thread;
    
public:
    JSThreadedInstance () :
        m_thread(threadTask())
    {}
    ~JSThreadedInstance () {
        kill();
        m_thread.join();
//        m_thread.detach();
    }
    
    std::function<void(void)> threadTask () {
        std::cout << "Starting js platform instance from thread " << std::this_thread::get_id() << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return [=] () {
            try {
                std::cout << "Starting js platform instance on thread " << std::this_thread::get_id() << '\n';
                
                JSPlatform platform;
                m_dispatchQueue.setDispatchTarget(&platform);
                m_dispatchQueue.run();
                
                std::cout << "Js platform thread ending (" << std::this_thread::get_id() << '\n';
            } catch (const std::runtime_error & e) {
                std::cout << "JS worker thread failed with exception:\n\t" << e.what() << std::endl;
            }
        };
    }
#define RUN_ON_WORKER_THREAD m_dispatchQueue.call([=](JSPlatform &platform)
    
    void eval (const std::string & contents) {
        std::cout << "scheduling eval task\n";
        RUN_ON_WORKER_THREAD {
            platform.eval(contents);
        });
    }
    
#undef RUN_ON_WORKER_THREAD
    
//    void eval (const std::string & contents) {
//        m_queue.call([=](JSPlatform & platform) {
//            platform.eval(contents);
//        });
//    }
    void kill () {
        m_dispatchQueue.kill();
    }
};
    
}; // namespace gl_sandbox

#endif /* js_thread_instance_hpp */
