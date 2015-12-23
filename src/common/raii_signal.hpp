//
//  raii_signal.hpp
//  GLSandbox
//
//  Created by semery on 12/21/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef raii_signal_hpp
#define raii_signal_hpp

#include <functional>
#include <vector>

namespace gl_sandbox {
namespace raii {

// Non-standard implementation of the signals and slots / Observer pattern.
// Provides a Signal<T> abstraction (with connect() and emit() methods) that works with
// std::function<void(T...)> (the 'slots').
//
// The main difference between this and other implementations (aside from performance,
// as this is not at all optimized), is that our connect() method returns an
// AutoreleasingObserver, which kills its connection as soon as its dtor gets called.
//
// This means that this:
//
//   someSignal.connect([](auto foo) { /* ... */ });
//
// will not work, as the dtor will immediately get called and unregister the function
// on the next call to emit() (unlinking is done lazily, during emit() calls).
//
// Instead, you have to store the result in some persistant-ish location, that lives for
// as long as the observer callback stays relevant for (ie. in the class it belongs to, for
// instance).
//
//    Observer<const FooType & foo> m_observer;
//    ...
//    m_observer = std::move(someSignal.connect([](auto foo) { /* ... */ }));
//
// This works well with RAII, ensuring that observers automatically unregister themselves
// and don't leave dangling references around.
//
// This is highly desirable for glsandbox, which has the following design characteristics:
// - dynamically loaded / unloaded Modules that use RAII to manage resources
// - an application context that a) owns the event system, and b) outlives all module
// (thus guaranteeing that all event generators will outlive their observers)
// - a threading model where all event callbacks get executed on the main thread
// (so no, this class isn't thread safe, and yes, it's by design)
//
// Oh, and all events are expected to have a type signature of type void(T...).
// Return values don't really make sense for event broadcasts, so... yeah -- returning
// anything from an observer function is illegal.
//
template <typename... Args>
struct Signal {
protected:
    typedef void F(Args...);
    struct Callback {
        std::function<F> fcn;
        bool released = false;
        
        Callback (std::function<F> fcn) : fcn(fcn) {}
    };
public:
    struct AutoreleasingObserver;
    
    // Connect this signal to a function with the same type signature (void(Args...)).
    // Returns an observer reference (used to disconnect from the signal) which *must*
    // be stored somewhere with >= scope to the observer function.
    AutoreleasingObserver connect (std::function<F> fcn) {
        auto cb = std::make_shared<Callback>(fcn);
        m_observerCallbacks.push_back(cb);
        return { cb };
    }
    
    // Fire an event / signal.
    // Sweeps through the list of observers, and lazily removes any that have been released.
    // (by a call to <observer>.release(), which sets a flag on the callback fcn).
    void emit (Args... args) {
        for (auto i = m_observerCallbacks.size(); i --> 0; ) {
            if (m_observerCallbacks[i]->released) {
                if (i != m_observerCallbacks.size())
                    m_observerCallbacks[i] = std::move(m_observerCallbacks.back());
                m_observerCallbacks.pop_back();
            } else {
                m_observerCallbacks[i]->fcn(args...);
            }
        }
    }
    
    // Observer handle from a call to <signal>.connect(fcn...).
    // Has one method, disconnect(), which is called automatically when the observer goes out of scope.
    // Can be moved, but not copied.
    struct AutoreleasingObserver {
        AutoreleasingObserver () : m_callbackRef(nullptr) {}
        AutoreleasingObserver (const std::shared_ptr<Callback> callback)
        : m_callbackRef(callback) {}
        AutoreleasingObserver (const AutoreleasingObserver &) = delete;
        AutoreleasingObserver (AutoreleasingObserver &&) = default;
        
        AutoreleasingObserver & operator= (const AutoreleasingObserver &) = delete;
        AutoreleasingObserver & operator= (AutoreleasingObserver &&) = default;
        
        ~AutoreleasingObserver () {
            if (m_callbackRef)
                m_callbackRef->released = true;
        }
        void disconnect () {
            if (m_callbackRef)
                m_callbackRef->released = true;
        }
    protected:
        std::shared_ptr<Callback> m_callbackRef;
    };
private:
    std::vector<std::shared_ptr<Callback>> m_observerCallbacks;
};
template <typename... Args>
using Observer = typename Signal<Args...>::AutoreleasingObserver;

}; // namespace raii
}; // namespace gl_sandbox
    
#endif /* raii_signal_hpp */
