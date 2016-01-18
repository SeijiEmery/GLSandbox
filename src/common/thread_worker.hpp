//
//  thread_worker.hpp
//  GLSandbox
//
//  Created by semery on 1/16/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef thread_worker_hpp
#define thread_worker_hpp

#include <thread>
#include <condition_variable>
#include <queue>

namespace gl_sandbox {

template <typename T>
class ThreadWorker {
protected:
    ThreadWorker () {}
    ~ThreadWorker () {}
    
    ThreadWorker (const ThreadWorker &) = delete;
    ThreadWorker (ThreadWorker &&) = default;
    
public:
    // Run worker
    void operator ()() {
        try {
            m_alive = true;
            static_cast<T*>(this)->onThreadBegin();
            while (m_alive) {
                while (m_mutex.lock(), !m_workQueue.empty()) {
                    if (!m_alive) {
                        m_mutex.unlock();
                        goto exit_thread;
                    }
                    auto task = m_workQueue.front();
                    m_workQueue.pop();
                    m_mutex.unlock();
                    
                    task(static_cast<T&>(*this));
                }
                
                m_mutex.lock();
                if (m_alive) {
                    m_waiting = true;
                    m_mutex.unlock();
                    
                    
                    std::unique_lock<decltype(m_mutex)> lock(m_mutex);
                    m_cv.wait(lock, [=]() -> bool { return !m_waiting; });
//                    m_cv.wait(m_mutex, [this]{ return !m_waiting; });
                }
            }
        exit_thread:
            static_cast<T*>(this)->onThreadExit();
        } catch (std::runtime_error & e) {
            m_alive = false;
            static_cast<T*>(this)->onThreadException(e);
        }
    }
    
    // Launch task on worker from another thread
    void exec (std::function<void(T&)> task) {
        m_workQueue.push(task);
        if (m_waiting) {
            m_waiting = false;
            m_cv.notify_all();
        }
    }
    
    // Kill worker from any thread
    void kill () {
        std::lock_guard<std::mutex> lock (m_mutex);
        m_alive = false;
    }
    
    // Check alive
    bool isAlive () const {
        return m_alive;
    }
private:
    std::mutex m_mutex;
    
    std::condition_variable m_cv;
    std::queue<std::function<void(T&)>> m_workQueue;
    bool m_waiting = false;
    bool m_alive = false;
};
    
//template <typename T>
//struct ThreadWorkerHandle {
//    ThreadWorkerHandle (std::shared_ptr<T> worker);
//    
//    void execOnThread (std::function<void(T&)>);
//protected:
//    std::shared_ptr<T> m_worker;
//};

//template <typename T>
//struct ThreadInstance {
//    template <typename... FwdArgs>
//    ThreadInstance (FwdArgs... args);
//    ~ThreadInstance ();
//    
//    void execOnThread (std::function<void(T&)>);
//    bool isAlive ();
//    void kill ();
//    
//protected:
//    std::unique_ptr<T> m_worker;
//    std::thread        m_runningThread;
//};
    
    
};

#endif /* thread_worker_hpp */
