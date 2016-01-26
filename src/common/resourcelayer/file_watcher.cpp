//
//  file_watcher.cpp
//  GLSandbox
//
//  Created by semery on 1/25/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include <CoreServices/CoreServices.h>
#include <thread>
#include <iostream>
#include <map>
#include <vector>

#include "file_watcher.hpp"

using namespace gl_sandbox;
using namespace resource_impl;
using namespace platform_osx;

typedef ThreadCallable<void(const FilePath &)> FileChangeCallback;

struct RetainedFileChangeCallback {
    FileChangeCallback callback;
    FilePath path;
    bool active = true;
    
    RetainedFileChangeCallback (const decltype(callback) & callback, const decltype(path) & path)
        : callback(callback), path(path) {}
};
typedef std::shared_ptr<RetainedFileChangeCallback> FileChangeCallbackRef;

struct DirectoryWatcherHandle : public IDirectoryWatcherHandle {
    FileChangeCallbackRef ref;
    
    DirectoryWatcherHandle () : ref(nullptr) {}
    DirectoryWatcherHandle (const decltype(ref) ref) : ref(ref) {}
    
    void detatch () override {
        if (ref) ref->active = false;
    }
    ~DirectoryWatcherHandle () {
        detatch();
    }
};

struct DirectoryWatcherInstance::Impl {
    typedef std::vector<FileChangeCallbackRef> CallbackList;
protected:
    std::thread m_cfRunThread;
    std::map<FilePath, CallbackList> m_pathCallbacks;
    
    FSEventStreamContext m_context { 0, (void*)this, nullptr, nullptr, nullptr };
    FSEventStreamRef m_eventStream;
    CFAbsoluteTime   m_latency = 3.0;
    CFMutableArrayRef m_pathList;
    std::mutex m_mutex;
    
public:
    Impl () : m_cfRunThread([](){
        CFRunLoopRun();
    }) {
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        
        m_pathList = CFArrayCreateMutable(nullptr, 0, nullptr);
        m_eventStream = FSEventStreamCreate(
                nullptr,
                (FSEventStreamCallback)&fsCallback,
                &m_context,
                m_pathList,
                kFSEventStreamEventIdSinceNow,
                m_latency,
                kFSEventStreamCreateFlagNone);
        
        FSEventStreamScheduleWithRunLoop(m_eventStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        FSEventStreamStart(m_eventStream);
    }
    ~Impl () {
        FSEventStreamStop(m_eventStream);
        CFRelease(m_eventStream);
        CFRelease(m_pathList);
    }
    
protected:
    static void fsCallback (
        const FSEventStreamRef streamRef,
        void * clientCallbackInfo,
        size_t numEvents,
        void * eventPaths,
        const FSEventStreamEventFlags eventFlags[],
        const FSEventStreamEventId eventIds[]
    ) {
        static_cast<DirectoryWatcherInstance::Impl*>(clientCallbackInfo)
            ->notifyPathsChanged((const char**)eventPaths, numEvents);
    }
public:
    void notifyPathsChanged (const char ** paths, size_t n) {
        
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        
        for (auto i = 0; i < n; ++i) {
            if (m_pathCallbacks.find(paths[i]) != m_pathCallbacks.end()) {
                
                auto & callbacks = m_pathCallbacks[paths[i]];
                for (auto k = callbacks.size(); k > 0; --k) {
                    if (callbacks[k]->active) {
                        assert(callbacks[k]->path == paths[i]);
                        callbacks[k]->callback(callbacks[k]->path);
                    } else {
                        if (k != callbacks.size())
                            std::swap(callbacks.back(), callbacks[k]);
                        callbacks.pop_back();
                    }
                }
                if (callbacks.size() == 0) {
                    m_pathCallbacks.erase(m_pathCallbacks.find(paths[i]));
                    
                    CFStringRef cfPath = CFStringCreateWithCString(nullptr, paths[i], kCFStringEncodingUTF8);
                    CFArrayRemoveValueAtIndex(m_pathList,
                            CFArrayGetFirstIndexOfValue(m_pathList,
                                  CFRangeMake(0, CFArrayGetCount(m_pathList)),
                                  (void*)cfPath));
                    CFRelease(cfPath);
                }
            }
        }
    }
    DirectoryWatcherHandleRef startWatchingPath (
            const FilePath & path,
            const ThreadCallable<void(const FilePath&)> onChanged
    ) {
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        
        FileChangeCallbackRef callback = std::make_shared<RetainedFileChangeCallback>(onChanged, path);
        if (m_pathCallbacks.find(path) != m_pathCallbacks.end()) {
            
            m_pathCallbacks[path] = { callback };
            
            CFStringRef cfPath = CFStringCreateWithCString(nullptr, path.c_str(), kCFStringEncodingUTF8);
            CFArrayAppendValue(m_pathList, (void*)cfPath);
            CFRelease(cfPath);
            return 0;
        } else {
            m_pathCallbacks[path].push_back(callback);
        }
        return DirectoryWatcherHandleRef(new DirectoryWatcherHandle(callback));
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
        const ThreadCallable<void (const ResourceError &)> onError
) {
    if (isValidDirPath(dirPath)) {
        return impl->startWatchingPath(dirPath, onChanged);
    } else {
        onError(ResourceError("Invalid path: %s' is not a directory\n", dirPath.c_str()));
        return std::make_shared<DirectoryWatcherHandle>();
    }
}


