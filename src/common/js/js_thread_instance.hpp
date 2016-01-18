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
        static const JSClass globalClass = {
            "global",
            JSCLASS_GLOBAL_FLAGS
        };
        
        m_global = JS_NewGlobalObject(m_context, &globalClass, nullptr, JS::DontFireOnNewGlobalHook);
        if (!m_global)
            throw new std::runtime_error("js instance failed to create global object");
    }
    
    JSPlatform (const JSPlatform &) = delete;
    JSPlatform & operator= (const JSPlatform &) = delete;
    
    ~JSPlatform () {
        JS_DestroyContext(m_context);
        JS_DestroyRuntime(m_runtime);
    }
    
    void eval (const std::string & contents) {
        std::cout << "Called eval on \"" << contents << "\"\n";
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
