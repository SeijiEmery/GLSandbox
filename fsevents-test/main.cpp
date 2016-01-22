//
//  main.cpp
//  fsevents-test
//
//  Created by semery on 1/21/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include <CoreServices/CoreServices.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <vector>
#include <memory>
#include <map>
#include <string>
#include <cstdio>


const char * join_paths (const char * a, size_t a_len, const char * b, char *& out, size_t out_sz) {
    if (a[a_len-1] == '/') {
        snprintf(out, out_sz, "%s%s", a, b);
    } else {
        snprintf(out, out_sz, "%s/%s", a, b);
    }
    return out;
}


struct GlobalFileChangeWatcher {
protected:
    GlobalFileChangeWatcher () {}
    
    std::map<std::string, struct stat> cached_file_info;
    std::map<std::string, std::map<std::string, int>> cached_sweep_count;
    std::map<std::string, int> cur_count;
    std::vector<std::string> tmp;
    
    void traverse_files (const char * dirpath, std::function<void(const char *, struct stat&, struct dirent*)> cb) {
        char sbuf [256];
        bool dirpath_ends_with_slash = dirpath[strlen(dirpath)-1] == '/';
    
        struct dirent ** file_list;
        int n = scandir(dirpath, &file_list, nullptr, nullptr);
        if (n < 0)
            printf("scandir '%s' returned %d files\n", dirpath, n);
        for (auto i = 0; i < n; ++i) {
            struct stat info;
            auto dinfo = file_list[i];
            
            // Ignore '.' and '..' file links
            if ((dinfo->d_namlen == 1 && dinfo->d_name[0] == '.') ||
                (dinfo->d_namlen == 2 && dinfo->d_name[0] == '.' && dinfo->d_name[1] == '.'))
                continue;
            
            // Join dir + filename
            if (dirpath_ends_with_slash || dinfo->d_name[0] == '/')
                snprintf(sbuf, sizeof(sbuf), "%s%s", dirpath, dinfo->d_name);
            else
                snprintf(sbuf, sizeof(sbuf), "%s/%s", dirpath, dinfo->d_name);
            
            // Call lstat to get file info
            auto r = lstat(sbuf, &info);
            r == 0 ?
                cb(sbuf, info, file_list[i]),0 :
                printf("stat '%s' returned %d\n", sbuf, r);
            
            if (S_ISDIR(info.st_mode) && file_list[i]->d_name[0] != '.') {
                traverse_files(sbuf, cb);
            }
        }
        assert((file_list == nullptr) == (n == 0));
        if (n > 0) {
            free(file_list);
        }
    }
public:
    void add_dir (const char * dirpath) {
        printf("Adding directory %s\n", dirpath);
        
        auto & sweep_count = cached_sweep_count[dirpath];
        cur_count[dirpath] = 0;
        
        traverse_files(dirpath, [&](const char * file_path, struct stat& stat_info, struct dirent* dinfo) {
            printf("-- %s\n", file_path);
            cached_file_info[std::string(file_path)] = stat_info;
            sweep_count[file_path] = 0;
        });
        printf("\n");
    }
    void update_dir (const char * dirpath) {
        
//        auto & file_list = cached_dir_file_lists[dirpath];
        auto & sweep_count = cached_sweep_count[dirpath];
        
        auto next = cur_count[dirpath]++;

        traverse_files(dirpath, [&](const char * file_path, struct stat& stat_info, struct dirent* dinfo) {
            if (cached_file_info.find(file_path) == cached_file_info.end()) {
                printf("-- Created file %s (%llu bytes)\n", file_path, stat_info.st_size);
            } else {
                auto & existing = cached_file_info[file_path];
                if (stat_info.st_mtimespec.tv_sec == 0 || existing.st_mtimespec.tv_sec == 0)
                    printf("timestamp '%s': %ld vs %ld\n", file_path, stat_info.st_mtimespec.tv_sec, existing.st_mtimespec.tv_sec);
                if (stat_info.st_mtimespec.tv_sec > existing.st_mtimespec.tv_sec) {
                    printf("-- Modified file %s (%s%d bytes)\n", file_path,
                           stat_info.st_size >= existing.st_size ? "+" : "-",
                           abs((int)stat_info.st_size - (int)existing.st_size));
                } else {
                    printf("-- (No change) %s\n", file_path);
                }
            }
            sweep_count[file_path] = next;
            cached_file_info[std::string(file_path)] = stat_info;
        });
        
        auto expected = next;
        auto & todelete = tmp; todelete.clear();
        for (auto& p : sweep_count) {
            assert(p.second <= expected);
            if (p.second != expected) {
                printf("-- Removed file %s (was %lld bytes)\n", p.first.c_str(), cached_file_info[p.first].st_size);
                cached_file_info.erase(p.first);
                todelete.push_back(p.first);
            }
        }
        for (auto & key : todelete) {
            sweep_count.erase(key);
        }
    }
    
    static GlobalFileChangeWatcher s_instance;
    static GlobalFileChangeWatcher & getInstance () {
        return s_instance;
    }
};
GlobalFileChangeWatcher GlobalFileChangeWatcher::s_instance;

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
        
        GlobalFileChangeWatcher::getInstance().update_dir(paths[i]);
        
//        struct dirent ** file_names;
//        int n = scandir(paths[i], &file_names, nullptr, nullptr);
//        printf("And here's a list of files in that directory:\n");
//        for (auto j = 0; j < n; ++j) {
//            struct dirent * finfo = file_names[j];
//            
//            // Use lstat to get file info
//            snprintf(buf, 256, "%s%s", paths[i], file_names[j]->d_name);
//            auto succ = lstat(buf, &info);
//            
//            printf("-- %d %s %lld bytes | time: %ld|%ld %ld|%ld %ld|%ld\n",
//                   succ,
//                   file_names[j]->d_name,
//                   info.st_size,
//                   info.st_atimespec.tv_sec, info.st_atimespec.tv_nsec,
//                   info.st_mtimespec.tv_sec, info.st_mtimespec.tv_nsec,
//                   info.st_birthtimespec.tv_sec, info.st_birthtimespec.tv_nsec
//            );
//            //            printf("-- %d '%s' %d\n", finfo->d_ino, finfo->d_name, finfo->d_type);
//        }
//        if (n > 0) {
//            assert(file_names != nullptr);
//            free(file_names);
//        }
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
        printf("setup event stream listener on '%s'\n", path);
        GlobalFileChangeWatcher::getInstance().add_dir(path);
        
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
        std::make_shared<FSDirWatcher> ( "/Users/semery/misc-projects/GLSandbox/" ),
        std::make_shared<FSDirWatcher> ( "/Users/semery/Library/Application Support/GLSandbox/" )
    };
    
    CFRunLoopRun();
    return 0;
}
