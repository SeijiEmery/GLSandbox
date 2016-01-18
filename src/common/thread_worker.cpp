////
////  thread_worker.cpp
////  GLSandbox
////
////  Created by semery on 1/16/16.
////  Copyright © 2016 Seiji Emery. All rights reserved.
////
//
//#include "thread_worker.hpp"
//#include <stdexcept>
//
//using namespace gl_sandbox;
//
//template <typename T>
//gl_sandbox::ThreadWorker<T>::ThreadWorker () {
//    
//}
//template <typename T>
//gl_sandbox::ThreadWorker<T>::~ThreadWorker () {
//    
//}
//
//template <typename T>
//void gl_sandbox::ThreadWorker<T>::operator () () {
//    try {
//        m_alive = true;
//        static_cast<T*>(this)->onThreadBegin();
//        while (m_alive) {
//            while (m_mutex.lock(), !m_workQueue.empty()) {
//                if (!m_alive) {
//                    m_mutex.unlock();
//                    goto exit_thread;
//                }
//                auto task = m_workQueue.front();
//                m_workQueue.pop();
//                m_mutex.unlock();
//                
//                task(static_cast<T&>(*this));
//            }
//            
//            m_mutex.lock();
//            if (m_alive) {
//                m_waiting = true;
//                m_mutex.unlock();
//                
//                m_cv.wait(m_mutex, [this]{ return !m_waiting; });
//            }
//        }
//    exit_thread:
//        static_cast<T*>(this)->onThreadExit();
//    } catch (std::runtime_error & e) {
//        m_alive = false;
//        static_cast<T*>(this)->onThreadException(e);
//    }
//}
//
////template <typename T>
////void ThreadWorker<T>::exec (std::function<void(T&)> task) {
////    std::lock_guard<std::mutex> lock (m_mutex);
////    m_workQueue.push(task);
////    if (m_waiting) {
////        m_waiting = false;
////        m_cv.notify_all();
////    }
////}
//
//template <typename T>
//void ThreadWorker<T>::kill () {
//    std::lock_guard<std::mutex> lock (m_mutex);
//    m_alive = false;
//}
//
//template <typename T>
//bool ThreadWorker<T>::isAlive () const {
//    return m_alive;
//}
//
////template <typename T>
////template <typename... Args>
////ThreadInstance<T>::ThreadInstance(Args... args)
////    : m_worker(new T(args...)),
////      m_runningThread(m_worker)
////{}
////
////template <typename T>
////ThreadInstance<T>::~ThreadInstance () {
////    m_worker->kill();
////    m_runningThread.join();
////}
////
////template <typename T>
////void ThreadInstance<T>::execOnThread(std::function<void(T&)> task) {
////    if (m_worker->isAlive()) {
////        m_worker->execOnThread(task);
////    }
////}
////
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
