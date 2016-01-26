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
    DirectoryWatcherHandle (const decltype(ref) ref, bool autorelease) : ref(ref), autorelease(autorelease) {}
    
    DirectoryWatcherHandle (const DirectoryWatcherHandle &) = default;
    DirectoryWatcherHandle & operator= (const DirectoryWatcherHandle &) = default;
    
    void detatch () override {
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
    std::thread m_cfRunThread;
    std::map<FilePath, CallbackList> m_pathCallbacks;
    
    FSEventStreamContext m_context { 0, (void*)this, nullptr, nullptr, nullptr };
    FSEventStreamRef m_eventStream;
    static constexpr CFAbsoluteTime latency = 1.0;
    CFMutableArrayRef m_pathList;
    std::mutex m_mutex;
    
    struct FSEventStreamInstance {
        FSEventStreamRef stream;
        CFArrayRef       pathArray;
        CFMutableSetRef  activePaths;
        
        FSEventStreamInstance (FSEventStreamContext * ctx, const CFStringRef * paths, size_t npaths) :
            pathArray(CFArrayCreate(nullptr, (const void**)paths, npaths, nullptr)),
            activePaths(CFSetCreateMutable(nullptr, npaths, nullptr))
        {
            for (auto i = 0; i < npaths; ++i) {
                CFSetAddValue(activePaths, (const void*)paths[npaths]);
            }
            stream = FSEventStreamCreate(
                 nullptr,
                 (FSEventStreamCallback)&fsCallback,
                 ctx,
                 pathArray,
                 kFSEventStreamEventIdSinceNow,
                 latency,
                 kFSEventStreamCreateFlagNone);
            
            FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            FSEventStreamStart(stream);
        }
        ~FSEventStreamInstance () {
            FSEventStreamStop(stream);
            CFRelease(stream);
            CFRelease(pathArray);
            CFRelease(activePaths);
        }
        
        FSEventStreamInstance (const FSEventStreamInstance&) = delete;
        FSEventStreamInstance & operator= (const FSEventStreamInstance&) = delete;
        FSEventStreamInstance (FSEventStreamInstance &&) = default;
        FSEventStreamInstance & operator= (FSEventStreamInstance &&) = default;
    };
    std::vector<FSEventStreamInstance> m_runningStreams;
    unsigned m_deadStreamPaths = 0;

public:
    Impl () : m_cfRunThread([](){
        CFRunLoopRun();
    }) {
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        
        m_pathList = CFArrayCreateMutable(nullptr, 10, nullptr);
        m_eventStream = FSEventStreamCreate(
                nullptr,
                (FSEventStreamCallback)&fsCallback,
                &m_context,
                m_pathList,
                kFSEventStreamEventIdSinceNow,
                latency,
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
    
    void appendFSEventStream (const FilePath & path) {
        
        m_runningStreams.emplace_back();
        auto & s = m_runningStreams.back();
        
        CFStringRef cfpath = CFStringCreateWithCString(nullptr, path.c_str(), kCFStringEncodingUTF8);
        
        s.pathArray = CFArrayCreate(nullptr, (const void**)&cfpath, 1, nullptr);
        
        CFRelease(cfpath);
        
        s.activePaths = 1;
        s.stream = FSEventStreamCreate(
            nullptr,
            (FSEventStreamCallback)&fsCallback,
            &m_context,
            m_pathList,
            kFSEventStreamEventIdSinceNow,
            m_latency,
            kFSEventStreamCreateFlagNone);
        
        FSEventStreamScheduleWithRunLoop(s.stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        FSEventStreamStart(s.stream);
    }
    
    void removePath (const char * path) {
        CFStringRef cfpath = CFStringCreateWithCStringNoCopy(kCFAllocatorNull, path, kCFStringEncodingUTF8, nullptr);
        
        for (auto & s : m_runningStreams) {
            CFRange range { 0, CFArrayGetCount(s.pathArray) };
            CFIndex index;
            
            CFSetRe
            
            
            while ((index = CFArrayGetFirstIndexOfValue(s.pathArray, range, cfpath)) != kCFNotFound) {
                --s.activePaths;
                ++s.deadPaths;
                range.location = index;
            }
        }
        CFSet
        
        
        CFRelease(cfpath);
    }
    
    
    
    
    void cleanupEmptyStreams () {
        for (auto i = m_runningStreams.size(); i --> 0; ) {
            auto & s = m_runningStreams[i];
            if (s.activePaths == 0) {
                
                FSEventStreamStop(s.stream);
                CFRelease(s.stream);
                CFRelease(s.pathArray);
            
                if (i != m_runningStreams.size() - 1)
                    std::swap(m_runningStreams.back(), s);
                m_runningStreams.pop_back();
            }
        }
    }
    
    void consolidateStreams () {
        CFMutableSetRef keepPaths = CFSetCreateMutable(nullptr, 0, nullptr);
        
        for (auto & s : m_runningStreams) {
            for (auto i = CFArrayGetCount(s.pathArray); i --> 0; ) {
                CFStringRef cfstr = (CFStringRef)CFArrayGetValueAtIndex(s.pathArray, i);
                const char * str = CFStringGetCStringPtr(cfstr, kCFStringEncodingUTF8);
                
                if (m_pathCallbacks.find(str) != m_pathCallbacks.end()) {
                    CFSetAddValue(keepPaths, cfstr);
                }
            }
        }
        
        m_runningStreams.emplace_back();
        std::swap(m_runningStreams[0], m_runningStreams.back());
        auto & s = m_runningStreams[0];
        
        auto npaths = CFSetGetCount(keepPaths);
        std::vector<void*> values (npaths);
        CFSetGetValues(keepPaths, (const void**)&values[0]);
        
        s.pathArray = CFArrayCreate(nullptr, (const void**)&values[0], npaths, nullptr);
        s.activePaths = (unsigned)npaths;
        s.stream = FSEventStreamCreate(nullptr,
                (FSEventStreamCallback)&fsCallback,
                &m_context,
                s.pathArray,
                kFSEventStreamEventIdSinceNow,
                m_latency,
                kFSEventStreamCreateFlagNone);
        FSEventStreamScheduleWithRunLoop(s.stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        FSEventStreamStart(s.stream);
        
        while (m_runningStreams.size() > 1) {
            auto & t = m_runningStreams.back();
            FSEventStreamStop(t.stream);
            CFRelease(t.stream);
            CFRelease(t.pathArray);
            m_runningStreams.pop_back();
        }
    }

public:
    void notifyPathsChanged (const char ** paths, size_t n) {
        
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        
        for (auto i = 0; i < n; ++i) {
            if (m_pathCallbacks.find(paths[i]) != m_pathCallbacks.end()) {
                
                auto & callbacks = m_pathCallbacks[paths[i]];
                for (auto k = callbacks.size(); k --> 0; ) {
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
            const ThreadCallable<void(const FilePath&)> onChanged,
            bool autorelease
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
        return impl->startWatchingPath(dirPath, onChanged, autorelease);
    } else {
        onError(ResourceError("Invalid path: %s' is not a directory\n", dirPath.c_str()));
        return std::make_shared<DirectoryWatcherHandle>();
    }
}


