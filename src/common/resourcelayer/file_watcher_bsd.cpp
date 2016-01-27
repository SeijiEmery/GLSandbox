//
//  file_watcher_bsd.cpp
//  GLSandbox
//
//  Created by semery on 1/26/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <errno.h>
#include <inttypes.h>

#include <thread>
#include <vector>
#include <map>
#include <array>
#include <iostream>

#include "file_watcher.hpp"

using namespace gl_sandbox;
using namespace resource_impl;
using namespace platform_bsd;

typedef ThreadCallable<void(const FilePath &)> FileChangeCallback;

struct RetainedFileChangeCallback {
    FileChangeCallback callback;
    bool active = true;
    
    RetainedFileChangeCallback (const decltype(callback) & callback)
        : callback(callback) {}
    
    RetainedFileChangeCallback (const RetainedFileChangeCallback&) = delete;
    RetainedFileChangeCallback & operator= (const RetainedFileChangeCallback&) = delete;
    RetainedFileChangeCallback (RetainedFileChangeCallback &&) = default;
    RetainedFileChangeCallback & operator= (RetainedFileChangeCallback &&) = default;
};
typedef std::shared_ptr<RetainedFileChangeCallback> FileChangeCallbackRef;

struct DirectoryWatcherHandle : public IDirectoryWatcherHandle {
    FileChangeCallbackRef ref;
    bool autorelease = false;
    
    DirectoryWatcherHandle () : ref(nullptr) {}
    DirectoryWatcherHandle (const decltype(ref) ref, bool autorelease) : ref(ref), autorelease(autorelease) {
        std::cout << "---- Created / returning handle to " << (void*)&(ref->callback) << "\n";
    }
    
    DirectoryWatcherHandle (const DirectoryWatcherHandle &) = default;
    DirectoryWatcherHandle & operator= (const DirectoryWatcherHandle &) = default;
    
    void detatch () override {
        std::cout << "---- Detatching handle to " << (void*)&(ref->callback) << "\n";
        if (ref) ref->active = false;
    }
    ~DirectoryWatcherHandle () {
        if (autorelease)
            detatch();
    }
};


struct DirectoryWatcherInstance::Impl {
    typedef std::vector<FileChangeCallbackRef> CallbackList;
protected:
    struct FileWatcher {
        FilePath path;
        std::vector<FileChangeCallbackRef> callbacks;
        unsigned ev_index;
        struct kevent ev;
        int fd = -1;
        
        FileWatcher (const FilePath & path, int fd) : path(path), fd(fd) {}
        
        FileWatcher (const FileWatcher &) = delete;
        FileWatcher & operator= (const FileWatcher &) = delete;
//        FileWatcher (FileWatcher && w) :
//            path(std::move(w.path)),
//            callbacks(std::move(w.callbacks)),
//            ev_index(w.ev_index),
//            ev(w.ev),
//            fd(w.fd)
//        {
//            w.fd = -1;
//        }
//        FileWatcher & operator= (FileWatcher && w) {
//            std::swap(path, w.path);
//            std::swap(callbacks, w.callbacks);
//            std::swap(ev_index, w.ev_index);
//            std::swap(ev, w.ev);
//            std::swap(fd, w.fd);
//            return *this;
//        }
        ~FileWatcher () {
            if (fd > 0)
                close(fd);
        }
    };
    int m_kq;
    std::map<FilePath, std::unique_ptr<FileWatcher>> m_fileWatchers;
    std::vector<struct kevent>      m_kevents;
    std::mutex m_mutex;
    
    enum class ThreadRunState {
        INIT,
        RUNNING,
        PAUSED,
        KILLED
    };
    struct ThreadState {
        std::condition_variable cv;
        std::mutex cv_mutex;
        ThreadRunState runState = ThreadRunState::INIT;
        decltype(m_fileWatchers) & watchers;
        decltype(m_kevents)      & kevents;
        decltype(m_kq)           & kq;
        decltype(m_mutex)        & ev_mutex;
        
        ThreadState (DirectoryWatcherInstance::Impl & instance) :
            watchers(instance.m_fileWatchers),
            kevents(instance.m_kevents),
            kq(instance.m_kq),
            ev_mutex(instance.m_mutex)
        {}
    };
    typedef std::shared_ptr<ThreadState> ThreadStateRef;
    
    ThreadStateRef m_threadState;
    std::thread    m_watcherThread;
    
public:
    Impl () :
        m_threadState(std::make_shared<ThreadState>(*this)),
        m_watcherThread(kqRunLoop, m_threadState)
    {
        if ((m_kq = kqueue()) == -1) {
            m_threadState->runState = ThreadRunState::KILLED;
            m_threadState->cv.notify_one();
            m_watcherThread.join();
            throw ResourceError("Failed to create kqueue: %s (%d)", strerror(errno), errno);
        }
        
        m_threadState->runState = ThreadRunState::RUNNING;
        m_threadState->cv.notify_one();
    }
    ~Impl () {
        m_threadState->runState = ThreadRunState::KILLED;
        m_threadState->cv.notify_one();
        m_watcherThread.join();
        close(m_kq);
    }
    
protected:
    static void kqRunLoop (ThreadStateRef state) {
        while (state->runState == ThreadRunState::INIT) {
            std::unique_lock<std::mutex> cv_lock(state->cv_mutex);
            state->cv.wait(cv_lock);
        }
        std::cout << "Starting kqueue run loop\n";
        
        constexpr auto N_EVENT_SLOTS = 64;
        std::array<struct kevent, N_EVENT_SLOTS> event_data;
        struct timespec timeout;
        timeout.tv_sec = 0; timeout.tv_nsec = (long)100e6; // 100 ms
        
        auto flagstring = [](int flags) {
            static char ret[512];
            char *or_ = "";
            ret[0]='\0'; // clear the string.
            if (flags & NOTE_DELETE) {strcat(ret,or_);strcat(ret,"NOTE_DELETE");or_="|";}
            if (flags & NOTE_WRITE) {strcat(ret,or_);strcat(ret,"NOTE_WRITE");or_="|";}
            if (flags & NOTE_EXTEND) {strcat(ret,or_);strcat(ret,"NOTE_EXTEND");or_="|";}
            if (flags & NOTE_ATTRIB) {strcat(ret,or_);strcat(ret,"NOTE_ATTRIB");or_="|";}
            if (flags & NOTE_LINK) {strcat(ret,or_);strcat(ret,"NOTE_LINK");or_="|";}
            if (flags & NOTE_RENAME) {strcat(ret,or_);strcat(ret,"NOTE_RENAME");or_="|";}
            if (flags & NOTE_REVOKE) {strcat(ret,or_);strcat(ret,"NOTE_REVOKE");or_="|";}
            return (const char *)ret;
        };
        
        while (state->runState != ThreadRunState::KILLED) {
            auto n_events = kevent(state->kq, &state->kevents[0], (unsigned)state->kevents.size(), &event_data[0], event_data.size(), &timeout);
            if (state->runState == ThreadRunState::KILLED)
                break;
            
            if ((n_events < 0) || (event_data[0].flags == EV_ERROR)) {
                fprintf(stderr, "\nERROR -- kevent returned %d (error %s: %d)\n", n_events, strerror(errno), errno);
            } else {
                std::lock_guard<decltype(state->ev_mutex)> lock (state->ev_mutex);
                
                for (auto i = 0; i < n_events; ++i) {
                    FileWatcher * watcher = (FileWatcher*)event_data[i].udata;
                    if (event_data[i].fflags & (NOTE_WRITE | NOTE_EXTEND)) {
                        std::cout << "File modified: '" << watcher->path << "'\n";
                    }
                    if (event_data[i].fflags & (NOTE_DELETE | NOTE_REVOKE)) {
                        std::cout << "File deleted: '" << watcher->path << "'\n";
                    }
                    if (event_data[i].fflags & NOTE_ATTRIB) {
                        std::cout << "File attrib changed: '" << watcher->path << "'\n";
                    }
                    if (event_data[i].fflags & NOTE_RENAME) {
                        std::cout << "File renamed: '" << watcher->path << "'\n";
                    }
                }
            }
        }
        std::cout << "kqueue run loop terminated\n";
    }

public:
    DirectoryWatcherHandleRef startWatchingPath (
        const FilePath & path,
        const ThreadCallable<void(const FilePath&)> onChanged,
        const ThreadCallable<void(const ResourceError &)> onError,
        bool autorelease
    ) {
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        
        FileChangeCallbackRef callback = std::make_shared<RetainedFileChangeCallback>(onChanged);
        if (m_fileWatchers.find(path) == m_fileWatchers.end()) {
            std::cout << "Starting new path watcher on '" << path << "'\n";
            
            auto fd = open(path.c_str(), O_EVTONLY);
            if (fd < 0) {
                onError(ResourceError("File does not exist (%s: %d)", strerror(errno), errno));
                return { nullptr };
            }
            
            m_fileWatchers[path] = std::unique_ptr<FileWatcher>(new FileWatcher(path, fd));
            auto & watcher = *m_fileWatchers[path];
            
            m_kevents.emplace_back();
            auto & evt = m_kevents.back();
            watcher.ev_index = (unsigned)(m_kevents.size() - 1);
            
            auto vnode_events = NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE;
            EV_SET(&evt, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, vnode_events, 0, (void*)&watcher);
            
        } else {
            std::cout << "Path watcher already exists on '" << path << "'; adding listener\n";
//            m_pathCallbacks[path].push_back(callback);
        }
        return DirectoryWatcherHandleRef(new DirectoryWatcherHandle(callback, autorelease));
    }
};

static bool isValidDirPath (const FilePath & path) {
    return true;
}

static DirectoryWatcherInstance * g_dirWatcher = nullptr;

DirectoryWatcherInstance::DirectoryWatcherInstance () :
impl(new DirectoryWatcherInstance::Impl())
{
    if (g_dirWatcher)
        std::cerr << "Warning: creating a new DirectoryWatcherInstance (already exists!)\n";
    else
        g_dirWatcher = this;
}
DirectoryWatcherInstance::~DirectoryWatcherInstance() {
    if (this == g_dirWatcher)
        g_dirWatcher = nullptr;
}

DirectoryWatcherHandleRef DirectoryWatcherInstance::watchForChanges(
    const FilePath &dirPath,
    const ThreadCallable<void (const FilePath &)> onChanged,
    const ThreadCallable<void (const ResourceError &)> onError,
    bool autorelease
) {
    if (isValidDirPath(dirPath)) {
        return impl->startWatchingPath(dirPath, onChanged, onError, autorelease);
    } else {
        onError(ResourceError("Invalid path: %s' is not a directory\n", dirPath.c_str()));
        return std::make_shared<DirectoryWatcherHandle>();
    }
}


