//
//  main.cpp
//  fsevents-test
//
//  Created by semery on 1/21/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include <vector>
#include <memory>
#include <cstdio>
#include <CoreServices/CoreServices.h>

void fsEventCallback (
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[]
) {
    char **paths = (char **)eventPaths;
    for (auto i = 0; i < numEvents; ++i) {
        printf("Change %llu in %s, flags %d\n", eventIds[i], paths[i], eventFlags[i]);
    }
}

struct FSDirWatcher {
    CFStringRef dir_path;
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 3.0;
    
    // Used to pass a backreference to this from the callback function.
    // I'm _hoping_ that passing a reference to stack-owned memory will be ok...
    // (otherwise I'll have to figure out how to allocate this properly using CFAllocator and set retain/release...)
    FSEventStreamContext streamContextData;
    
    FSDirWatcher (const char * path) {
        dir_path = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
        CFArrayRef pathsToWatch = CFArrayCreate(kCFAllocatorDefault, (const void**)&dir_path, 1, nullptr);
        
        streamContextData.info = this;
        stream = FSEventStreamCreate(kCFAllocatorDefault,
                (FSEventStreamCallback)&fsEventCallback,
                nullptr, //&streamContextData,
                pathsToWatch,
                kFSEventStreamEventIdSinceNow,
                latency,
                kFSEventStreamCreateFlagNone);
        CFRelease(pathsToWatch);
        
        FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        FSEventStreamStart(stream);
        printf("setup event stream listener on '%s'\n", path);
    }
    ~FSDirWatcher () {
        FSEventStreamStop(stream);
        CFRelease(dir_path);
    }
};
typedef std::shared_ptr<FSDirWatcher> FSDirWatcherRef;

int main(int argc, const char * argv[]) {
    CFRunLoopGetCurrent();
    
    std::vector<FSDirWatcherRef> watchers {
        std::make_shared<FSDirWatcher> ( "/Users/semery/" ),
        std::make_shared<FSDirWatcher> ( "~/misc-projects/GLSandbox/script/" ),
        std::make_shared<FSDirWatcher> ( "~/misc-projects/GLSandbox/assets/" ),
        std::make_shared<FSDirWatcher> ( "~/Library/Application Support/GLSandbox/" )
    };
    
    CFRunLoopRun();
    return 0;
}
