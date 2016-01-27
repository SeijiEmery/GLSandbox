//
//  file_watcher_osx.cpp
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
        std::cout << "Detatching handle on '" << (ref ? ref->path : "<nullref>") << "'\n";
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
            std::cout << "---- Created " << *this << "\n";
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
            if (stream) {
                kill();
            }
            assert((stream != nullptr) == (pathArray != nullptr) && (stream != nullptr) == (activePaths != nullptr));
        }
    protected:
        void kill () {
            assert(stream != nullptr && pathArray != nullptr && activePaths != nullptr);
            std::cout << "---- Killing " << *this << "\n";
            
            FSEventStreamStop(stream);
            CFRelease(stream);
            CFRelease(pathArray);
            CFRelease(activePaths);
        }
    public:
        
        FSEventStreamInstance (const FSEventStreamInstance&) = delete;
        FSEventStreamInstance & operator= (const FSEventStreamInstance&) = delete;
        
        FSEventStreamInstance (FSEventStreamInstance && s) :
            stream(s.stream), pathArray(s.pathArray), activePaths(s.activePaths)
        {
            s.stream = nullptr;
            s.pathArray = nullptr;
            s.activePaths = nullptr;
        }
        FSEventStreamInstance & operator= (FSEventStreamInstance && s) {
            if (stream) { kill(); }
            stream = s.stream; s.stream = nullptr;
            pathArray = s.pathArray; s.pathArray = nullptr;
            activePaths = s.activePaths; s.activePaths = nullptr;
            return *this;
        }
        
        friend std::ostream & operator << (std::ostream & os, const FSEventStreamInstance & s) {
            os << "FSEventStream instance " << (void*)&s << " ";
            
            if (!s.stream) os << "null stream! ";
            if (!s.pathArray) os << "null pathArray! ";
            if (!s.activePaths) os << "null activePaths! ";
            
            os << "(paths = ";
            for (auto i = CFArrayGetCount(s.pathArray); i --> 0; ) {
                auto v = (CFStringRef)CFArrayGetValueAtIndex(s.pathArray, i);
                auto path = CFStringGetCStringPtr(v, kCFStringEncodingUTF8);
                os << '\'' << path << '\'' << (i ? ", " : "");
            }
            return os << "; activePaths = " << CFSetGetCount(s.activePaths) << ")";
        }
        

    };
    std::vector<FSEventStreamInstance> m_runningStreams;
    unsigned m_deadStreamPaths = 0;

public:
    Impl () : m_cfRunThread([](){
        std::cout << "STARTING CFRUN LOOP\n";
        CFRunLoopRun();
        std::cout << "ENDING CFRUN LOOP\n";
    }) {
        
    }
    ~Impl () {
        while (m_runningStreams.size() > 0)
            m_runningStreams.pop_back();
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
        std::cout << "Recieved events!\n";
        static_cast<DirectoryWatcherInstance::Impl*>(clientCallbackInfo)
            ->notifyPathsChanged((const char**)eventPaths, numEvents);
    }
    
    void appendFSEventStream (const FilePath & path) {
        
        CFStringRef cfpath = CFStringCreateWithCString(nullptr, path.c_str(), kCFStringEncodingUTF8);
        
        m_runningStreams.emplace_back(&m_context, &cfpath, 1);
        
        std::cout << ">>>> Created new stream " << m_runningStreams.back() << "\n";
        
        CFRelease(cfpath);
    }
    
    void removePath (const char * path) {
        
        std::cout << ">>>> Removing path (internal op) '" << path << "'\n";
        
        CFStringRef cfpath = CFStringCreateWithCStringNoCopy(kCFAllocatorNull, path, kCFStringEncodingUTF8, nullptr);
        
        m_deadStreamPaths = 0;
        for (auto & s : m_runningStreams) {
            CFSetRemoveValue(s.activePaths, cfpath);
            m_deadStreamPaths += CFArrayGetCount(s.pathArray) - CFSetGetCount(s.activePaths);
        }
        
        CFRelease(cfpath);
    }
    
    void cleanupEmptyStreams () {
        for (auto i = m_runningStreams.size(); i --> 0; ) {
            auto & s = m_runningStreams[i];
            
            if (CFSetGetCount(s.activePaths) == 0) {
                std::cout << ">>>> Killing empty stream " << s << "\n";
                if (i != m_runningStreams.size() - 1)
                    std::swap(m_runningStreams.back(), s);
                m_runningStreams.pop_back();
            }
        }
    }
    
    void consolidateStreams () {
        if (m_deadStreamPaths > 3 || m_runningStreams.size() > 2) {}
        else return;
        
        std::cout << ">>>> Consolidating streams\n";
        
        unsigned totalActivePaths = 0;
        for (auto & s : m_runningStreams)
            totalActivePaths += CFSetGetCount(s.activePaths);
        
        CFMutableSetRef keepPaths = CFSetCreateMutable(nullptr, totalActivePaths, nullptr);
        std::vector<void*> tmpValues (totalActivePaths + 100);
        
        for (auto & s : m_runningStreams) {
            // I'm kinda surprised there doesn't appear to be CF set union operation. Oh well...
            CFSetGetValues(s.activePaths, (const void**)&tmpValues[0]);
            for (auto i = CFSetGetCount(s.activePaths); i --> 0; ) {
                CFSetAddValue(keepPaths, tmpValues[i]);
            }
        }
        
        auto keepPathsCount = CFSetGetCount(keepPaths);
        if (keepPathsCount > totalActivePaths) {
            std::cout << "ERROR: num keepPaths > totalActivePaths: " << keepPathsCount << " > " << totalActivePaths << "\n";
            tmpValues.resize(keepPathsCount);
        }
        
//        assert(CFSetGetCount(keepPaths) < totalActivePaths);
        
        std::cout << "---- values before get: [";
        for (auto i = keepPathsCount; i --> 0; ) {
            std::cout << tmpValues[i] << (i ? ", " : "]\n");
        }
        CFSetGetValues(keepPaths, (const void**)&tmpValues[0]);
        std::cout << "---- values after  get: [";
        for (auto i = keepPathsCount; i --> 0; ) {
            std::cout << tmpValues[i] << (i ? ", " : "]\n");
        }
        
        std::cout << "---- got " << keepPathsCount << " paths:\n";
        for (auto i = keepPathsCount; i --> 0; ) {
            auto ptr = tmpValues[i];
            auto str = CFStringGetCStringPtr((CFStringRef)ptr, kCFStringEncodingUTF8);
            std::cout << "----     '" << str << "'\n";
        }
        
        
        
        m_runningStreams.emplace_back(&m_context,
                                      (const CFStringRef*)&tmpValues[0],
                                      keepPathsCount);
        std::cout << ">>>> Consolidated streams -- created stream " << m_runningStreams.back() << "\n";
        std::swap(m_runningStreams[0], m_runningStreams.back());
        
        while (m_runningStreams.size() > 1) {
            std::cout << ">>>> Consolidated streams -- removing stream " << m_runningStreams.back() << "\n";
            m_runningStreams.pop_back();
        }
        m_deadStreamPaths = 0;
    }

public:
    void notifyPathsChanged (const char ** paths, size_t n) {
        
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        
        std::cout << "Paths changed: (callback notify)\n";
        for (auto i = 0; i < n; ++i) {
            std::cout << "    '" << paths[i] << "'\n";
        }
        
        unsigned n_removed = 0;
        
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
                    
                    removePath(paths[i]);
                    ++n_removed;
                }
            }
        }
        if (n_removed > 0) {
            cleanupEmptyStreams();
//            consolidateStreams();
        }
    }
    DirectoryWatcherHandleRef startWatchingPath (
            const FilePath & path,
            const ThreadCallable<void(const FilePath&)> onChanged,
            bool autorelease
    ) {
        std::lock_guard<decltype(m_mutex)> lock (m_mutex);
        std::cout << "Starting request to watch path '" << path << "'\n";
        
        FileChangeCallbackRef callback = std::make_shared<RetainedFileChangeCallback>(onChanged, path);
        if (m_pathCallbacks.find(path) == m_pathCallbacks.end()) {
            
            std::cout << "Starting new path watcher\n";
            
            m_pathCallbacks[path] = { callback };
            appendFSEventStream(path);
//            consolidateStreams();
            
        } else {
            std::cout << "Path watcher already exists\n";
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


