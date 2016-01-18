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
#include "../thread_worker.hpp"
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
#include <cassert>

using namespace JS;

namespace gl_sandbox {
    
namespace scratch1 {
    
    
// Provides an event loop, and one-directional dispatch system to run arbitrary callbacks on
// a target worker thread. Used to implement multithreaded scripts.
template <typename T>
struct ThreadedDispatchQueue {
protected:
    T* m_target = nullptr;
    std::queue<std::function<void(T&)>> m_queue;
    std::mutex m_queueMutex;
    std::mutex m_notifyMutex;
    std::condition_variable m_cv;
    bool m_isPaused  = false;
    std::atomic<bool> m_isRunning { false };
    
public:
    // Call from worker thread
    void setDispatchTarget (T* target) {
        m_target = target;
    }
    
    // Call from worker thread. Does not terminate until kill() is called.
    void run () {
        assert(!m_isRunning);
        
        m_queueMutex.lock();
        m_isRunning = true;
        m_queue.clear();
        
        assert(m_target != nullptr);
        
        do {
            while (!m_queue.empty() && m_isRunning) {
                auto next = m_queue.front(); m_queue.pop();
                m_queueMutex.unlock();
                
                next(*m_target);
                m_queueMutex.lock();
            }
            if (m_isRunning) {
                m_isPaused = true;
                m_queueMutex.unlock();
                
                std::unique_lock<std::mutex> lock (m_notifyMutex);
                m_cv.wait(m_notifyMutex, lock);
                
                m_queueMutex.lock();
                m_isPaused = false;
            }
        } while (m_isRunning);
        m_queueMutex.unlock();
    }
    
    // Call from main thread
    void call (std::function<void(T&)> fcn) {
        m_queueMutex.lock();
        m_queue.push(fcn);
        m_queueMutex.unlock();
        if (m_isPaused)
            m_cv.notify_all();
    }
    
    // Call from main thread
    void kill () {
        m_queueMutex.lock();
        m_isRunning = false;
        m_queueMutex.unlock();
        if (m_isPaused)
            m_cv.notify_all();
    }
    
    ~ThreadedDispatchQueue () {
        kill();
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
    
    static const constexpr JSClass g_globalClass = {
        "global",
        JSCLASS_GLOBAL_FLAGS
    };
    
protected:
    JSRuntime * m_runtime;
    JSContext * m_context;
    RootedObject m_global;

public:
    JSPlatform () :
        m_runtime(createRuntimeOrThrow("Failed to create js runtime")),
        m_context(createContextOrThrow(m_runtime, "Failed to create js context")),
        m_global(m_context)
    {
        m_global = JS_NewGlobalObject(m_context, &g_globalClass, nullptr, JS::DontFireOnNewGlobalHook);
        if (!m_global)
            throw new std::runtime_error("js instance failed to create global object");
    }
    
    void eval (const std::string & contents) {
        
    }
};
    
// Owns a thread running a JSPlatform instance, and provides a threadsafe handle / wrapper for interacting
// with it using a dispatch queue and callback objects.
struct JSThreadedInstance {
protected:
    ThreadedDispatchQueue<JSPlatform> m_queue;
    std::thread m_thread;
    
public:
    JSThreadedInstance () :
        m_thread(threadTask())
    {}
    
    std::function<void(void)> threadTask () {
        return [=] () {
            try {
                JSPlatform platform;
                m_queue.setDispatchTarget(&platform);
                m_queue.run();
            } catch (const std::runtime_error & e) {
                std:cout << "JS worker thread failed with exception:\n\t" << e.what() << std::endl;
            }
        };
    }
    
    void eval (const std::string & contents) {
        m_queue.call([=](JSPlatform & platform) {
            platform.eval(contents);
        });
    }
    void kill () {
        m_queue.kill();
    }
};
    
}; // namespace scratch1
    
    
    
struct JSThreadWorker : public ThreadWorker<JSThreadWorker> {
    JSThreadWorker () : m_global(JS::NullHandleValue) {}
    ~JSThreadWorker () {}
    
    JSThreadWorker (const JSThreadWorker &) = delete;
    JSThreadWorker (JSThreadWorker &&) = default;
    JSThreadWorker & operator= (const JSThreadWorker &) = delete;
    JSThreadWorker & operator= (JSThreadWorker &&) = default;
    
    void onThreadBegin ();
    void onThreadExit  ();
    void onThreadException (const std::runtime_error & e);
    
    void eval (const std::string & contents);
protected:
    JSRuntime * m_runtime = nullptr;
    JSContext * m_context = nullptr;
    RootedObject m_global;
};

struct JSThreadInstance {
    JSThreadInstance ()
        : m_thread([&](){ m_worker(); }) {}
    ~JSThreadInstance () {
        m_worker.kill();
    }
    void doEval (const std::string & contents) {
        m_worker.eval(contents);
    }
protected:
    JSThreadWorker m_worker;
    std::thread    m_thread;
};
    
    
    
    
}; // namespace gl_sandbox

#endif /* js_thread_instance_hpp */
