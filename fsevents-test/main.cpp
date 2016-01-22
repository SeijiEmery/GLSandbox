//
//  main.cpp
//  fsevents-test
//
//  Created by semery on 1/21/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include <vector>
#include <memory>
#include <map>
#include <cstdio>
#include <CoreServices/CoreServices.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


struct GlobalFileChangeWatcher {
    std::map<std::string, struct stat> cached_file_info;
    
    void sweep_dir (const char * dirpath) {
        
    }
    void update_dir (const char * filepath) {
        
    }
    
    static GlobalFileChangeWatcher s_instance;
    GlobalFileChangeWatcher * getInstance () {
        return &s_instance;
    }
};

//struct stat {
//    dev_t	 	st_dev;		/* [XSI] ID of device containing file */
//    ino_t	  	st_ino;		/* [XSI] File serial number */
//    mode_t	 	st_mode;	/* [XSI] Mode of file (see below) */
//    nlink_t		st_nlink;	/* [XSI] Number of hard links */
//    uid_t		st_uid;		/* [XSI] User ID of the file */
//    gid_t		st_gid;		/* [XSI] Group ID of the file */
//    dev_t		st_rdev;	/* [XSI] Device ID */
//#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
//    struct	timespec st_atimespec;	/* time of last access */
//    struct	timespec st_mtimespec;	/* time of last data modification */
//    struct	timespec st_ctimespec;	/* time of last status change */
//#else
//    time_t		st_atime;	/* [XSI] Time of last access */
//    long		st_atimensec;	/* nsec of last access */
//    time_t		st_mtime;	/* [XSI] Last data modification time */
//    long		st_mtimensec;	/* last data modification nsec */
//    time_t		st_ctime;	/* [XSI] Time of last status change */
//    long		st_ctimensec;	/* nsec of last status change */
//#endif
//    off_t		st_size;	/* [XSI] file size, in bytes */
//    blkcnt_t	st_blocks;	/* [XSI] blocks allocated for file */
//    blksize_t	st_blksize;	/* [XSI] optimal blocksize for I/O */
//    __uint32_t	st_flags;	/* user defined flags for file */
//    __uint32_t	st_gen;		/* file generation number */
//    __int32_t	st_lspare;	/* RESERVED: DO NOT USE! */
//    __int64_t	st_qspare[2];	/* RESERVED: DO NOT USE! */
//};




void fsEventCallback (
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[]
) {
    struct stat info;
    char buf [256];
    
    char **paths = (char **)eventPaths;
    for (auto i = 0; i < numEvents; ++i) {
        printf("Change %llu in %s, flags %d\n", eventIds[i], paths[i], eventFlags[i]);
        
        struct dirent ** file_names;
        int n = scandir(paths[i], &file_names, nullptr, nullptr);
        printf("And here's a list of files in that directory:\n");
        for (auto j = 0; j < n; ++j) {
            struct dirent * finfo = file_names[j];
            
            // Use lstat to get file info
            snprintf(buf, 256, "%s%s", paths[i], file_names[j]->d_name);
            auto succ = lstat(buf, &info);
            
            printf("-- %d %s %lld bytes | time: %ld|%ld %ld|%ld %ld|%ld\n",
                   succ,
                   file_names[j]->d_name,
                   info.st_size,
                   info.st_atimespec.tv_sec, info.st_atimespec.tv_nsec,
                   info.st_mtimespec.tv_sec, info.st_mtimespec.tv_nsec,
                   info.st_birthtimespec.tv_sec, info.st_birthtimespec.tv_nsec
            );
            //            printf("-- %d '%s' %d\n", finfo->d_ino, finfo->d_name, finfo->d_type);
        }
        if (n > 0) {
            assert(file_names != nullptr);
            free(file_names);
        }
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
        std::make_shared<FSDirWatcher> ( "~/misc-projects/GLSandbox/" ),
        std::make_shared<FSDirWatcher> ( "~/Library/Application Support/GLSandbox/" )
    };
    
    CFRunLoopRun();
    return 0;
}
