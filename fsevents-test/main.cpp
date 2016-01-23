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
#include <mutex>
#include <stdexcept>
#include <cstdio>


const char * join_paths (const char * a, size_t a_len, const char * b, char *& out, size_t out_sz) {
    if (a[a_len-1] == '/') {
        snprintf(out, out_sz, "%s%s", a, b);
    } else {
        snprintf(out, out_sz, "%s/%s", a, b);
    }
    return out;
}

template <typename... Args>
char * fmtargs (const char * fmt, Args... args) {
    static char buf[512];
    snprintf(buf, sizeof(buf), fmt, args...);
    return buf;
}

struct InvalidPathException : std::runtime_error {
    InvalidPathException (std::string what) : std::runtime_error(what) {}
    
    template <typename... Args>
    InvalidPathException (const char * fmt, Args... args) : std::runtime_error(fmtargs(fmt, args...)) {}
};

struct Path {
protected:
    struct PathStorage {
        char path [PATH_MAX];
    };
    std::shared_ptr<PathStorage> p;
    
    static void resolve_path (char * path) {
        if (!realpath(path, path)) {
            switch (errno) {
                case EACCES: throw InvalidPathException("No access to '%s'", path);
                case EINVAL: throw InvalidPathException("Invalid path '%s'", path);
                case EIO:    throw InvalidPathException("I/O error (cannot resolve path) '%s'", path);
                case ELOOP:  throw InvalidPathException("Path contains too many symbolic links '%s'", path);
                case ENAMETOOLONG: throw InvalidPathException("Path length too long (this should not happen) '%s'", path);
                case ENOENT: throw InvalidPathException("File does not exist: '%s'", path);
                case ENOTDIR: throw InvalidPathException("Directory does not exist: '%s'", path);
                default: throw InvalidPathException("realpath error %d '%s'", errno, path);
            }
        }
    }
    
public:
    // Convert path from string representation
    Path (const char * str, size_t len = 0) : p(std::make_shared<PathStorage>())
    {
        len = len || strlen(str);
        if (len > PATH_MAX)
            throw InvalidPathException("Path too long (%d > PATH_MAX %d) '%s'", len, PATH_MAX, str);
        
        // expand ~
        if (str[0] == '~' && str[1] == '/')
            snprintf(p->path, sizeof(p->path), "%s/%s", getenv("HOME"), str + 2);
        else if (str[0] == '~' && (str[1] == '\0' || isspace(str[1])))
            snprintf(p->path, sizeof(p->path), "%s/", getenv("HOME"));
        else
            strcpy(p->path, str);
        resolve_path(p->path);
    }
    Path (const Path & first, const char * rest, size_t len = 0) {
        snprintf(p->path, sizeof(p->path), "%s/%s", first.p->path, rest);
        resolve_path(p->path);
    }
    Path (const std::string & str) : Path(str.c_str(), str.length()) {}
    Path (const Path & path) = default;
    Path & operator= (const Path & path) = default;
    
    Path operator+ (const char * str) {
        return Path { *this, str };
    }
    Path operator+ (const std::string & str) {
        return Path { *this, str.c_str(), str.length() };
    }
};



struct GlobalFileChangeWatcher {
protected:
    GlobalFileChangeWatcher () {}
    
    std::map<std::string, struct stat> cached_file_info;
    std::map<std::string, std::map<std::string, int>> cached_sweep_count;
    std::map<std::string, int> cur_count;
    std::vector<std::string> tmp;
    
    enum class EventType {
        FILE_ADDED, FILE_MODIFIED, FILE_REMOVED
    };
    std::vector<std::pair<const char *, EventType>> event_queue;
    std::vector<std::function<void(const char*, EventType)>> event_listeners;
    std::recursive_mutex global_mutex;
    
    static int ignore_hidden_files (const struct dirent * d) {
        return !(d->d_name[0] == '.' && (d->d_name[1] == '.' ||
            strcmp(d->d_name, ".git") == 0 ||
            strcmp(d->d_name, ".DS_Store") == 0));
    }
    
    void traverse_files (const char * dirpath, std::function<void(const char *, struct stat&, struct dirent*)> cb) {
        char sbuf [256];
        bool dirpath_ends_with_slash = dirpath[strlen(dirpath)-1] == '/';
    
        struct dirent ** file_list;
        int n = scandir(dirpath, &file_list, ignore_hidden_files, nullptr);
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
//        assert((file_list == nullptr) == (n == 0));
        if (n > 0) {
            free(file_list);
        }
    }
public:
    void add_dir (const char * dirpath) {
        std::lock_guard<std::recursive_mutex> lock(global_mutex);
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
        std::lock_guard<std::recursive_mutex> lock(global_mutex);
        auto & sweep_count = cached_sweep_count[dirpath];
        
        auto next = cur_count[dirpath]++;

        traverse_files(dirpath, [&](const char * file_path, struct stat& stat_info, struct dirent* dinfo) {
            if (cached_file_info.find(file_path) == cached_file_info.end()) {
                printf("-- Created file %s (%llu bytes)\n", file_path, stat_info.st_size);
                event_queue.emplace_back(file_path, EventType::FILE_ADDED);
            } else {
                auto & existing = cached_file_info[file_path];
                if (stat_info.st_mtimespec.tv_sec == 0 || existing.st_mtimespec.tv_sec == 0)
                    printf("timestamp '%s': %ld vs %ld\n", file_path, stat_info.st_mtimespec.tv_sec, existing.st_mtimespec.tv_sec);
                if (stat_info.st_mtimespec.tv_sec > existing.st_mtimespec.tv_sec) {
                    printf("-- Modified file %s (%s%d bytes)\n", file_path,
                           stat_info.st_size >= existing.st_size ? "+" : "-",
                           abs((int)stat_info.st_size - (int)existing.st_size));
                    event_queue.emplace_back(file_path, EventType::FILE_MODIFIED);
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
                event_queue.emplace_back(p.first.c_str(), EventType::FILE_REMOVED);
            }
        }
        for (auto & key : todelete) {
            sweep_count.erase(key);
        }
    }
    
    void process_listeners () {
        std::lock_guard<std::recursive_mutex> lock(global_mutex);
        
        if (event_queue.size() == 0)
            return;
        
        for (auto & listener : event_listeners) {
            auto n = event_queue.size();
            while (n --> 0) {
                listener(event_queue[n].first, event_queue[n].second);
            }
        }
        event_queue.clear();
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
    char **paths = (char **)eventPaths;
    for (auto i = 0; i < numEvents; ++i) {
        printf("Change %llu in %s, flags %d\n", eventIds[i], paths[i], eventFlags[i]);
        
        GlobalFileChangeWatcher::getInstance().update_dir(paths[i]);
    }
}

void whatWeWant () {
    std::vector<ResourceLayer::AbstractFileWatcher> watchers;
    
    watchers.push_back(ResourceLayer::createWatcher()
       .inDirectory("~/misc-projects/GLSandbox/script/")
       .forFileExts(".moon")
       .onFileSizeOrHashChange([](auto& file_path) {
            runCmd("moonc "+file_path+" "+substPath(file_path, "~/misc-projects/GLSandbox/script/", "~/Library/Application Support/script/"));
        })());
    
    watchers.push_back(ResourceLayer::createWatcher()
       .inDirectory("~/Library/Application Support/script/")
       .forFileExts(".lua")
       .onFileSizeOrHashChange([](auto& file_path) {
            reloadScript(file_path);
        })());
    
    watchers.push_back(ResourceLayer::createWatcher()
        .inDirectory("~/misc-projects/GLSandbox/assets/")
        .forFileExts(".jpg", ".png")
        .onFileSizeOrHashChange([](auto& file_path) {
            reloadTexture(file_path);
        })());
    
    watchers.push_back(ResourceLayer::createWatcher()
        .inDirectory("~/misc-projects/GLSandbox/assets/")
        .forFileExts(".fs", ".vs")
        .onFileSizeOrHashChange([](auto& file_path) {
            reloadShader(file_path);
        })());
    
    
    ResourceLayer::forEachFile()
        .inDirectory("~/misc-projects/GLSandbox/script/")
        .withFileExt(".moon")
        .run([](auto & moon_path) {
            auto lua_path = moon_path.subst("~/misc-projects/GLSandbox/script/*.moon", "~/Library/Application Support/script/*.lua");
            if (!lua_path.exists() || moon_path.lastModified() > lua_path.lastModified()) {
                runCmd("moonc"+...);
                waitToRunScript(lua_path, 1);
                ResourceLayer::onFirstChange(lua_path, [](auto & lua_path){
                    assert(lua_path.ext() == ".lua");
                    if (lua_path.exists()) {
                        maybeRunScript(lua_path);
                    }
                });
            }
        })
    ResourceLayer::forEachFile()
        .inDirectory("~/misc-projects/GLSandbox/script/")
        .withFileExt(".lua")
        .run([](auto & lua_path) {
            maybeRunScript(lua_path);
        })
}

void orMaybeEvenThis () {
    
    ResourceLayer::createResourcePipeline()
        .sourceDirectory("~/misc-projects/GLSandbox/script/")
        .sourceExts(".moon")
        .targetDirectory("~/Library/Application Support/script/")
        .targetExts(".lua")
        .onSourceFileSizeOrHashChanged([](auto& src_file, auto& target_file) {
            assert(src_file.hasExt(".moon") && target_file.hasExt(".lua"));
            runCmd("moonc...")
        })
        .onTargetFirstDetected([](auto & lua_file) {
            notifyScriptAvailable(lua_file);
        })
        .onTargetUpdated([](auto & lua_file) {
            tryReloadScript(lua_file);
        })
        .onTargetDeleted([](auto & lua_file) {
            notifyScriptSourceRemoved(lua_file);
        })
        .onSourceDeleted([](auto & moon_file) {
            notifyScriptSourceRemoved(moon_file);
        })();
    
//    ResourceLayer::forFiles()
//        .inDirectory("~/Library/Application Support/assets/")
//        .withFileExts(".png", ".jpg")
//        .cacheAsResourceBundle()
//        .notifyWhenFirstDetected()
//        .notifyWhenFileSizeOrHashChanges()
//    ();

    ResourceLayer::createResourcePrefix("assets:/")
        .inDirectory("~/Library/Application Support/assets/")
        .forFileExts(".png", ".jpg")
        .notifyWhenFirstDetected()
        .notifyWhenFileSizeOrHashChanges()
    ();
    ResourceLayer::createResourcePrefix("assets:/")
        .inDirectory("~/Library/Application Support/assets/")
        .forFileExts(".obj", ".mtl", ".fbx")
        .notifyWhenFirstDetected()
        .notifyWhenFileSizeOrHashChanges()
    ();
    ResourceLayer::createResourcePrefix("shader:/")
        .inDirectory("~/Library/Application Support/assets/")
        .forFileExts(".fs", ".vs")
        .notifyWhenFirstDetected()
        .notifyWhenFileSizeOrHashChanges()
    ();
    
    // to load a file:
    my_texture_handle = ...
    ResourceLayer::loadFileAsBuffer("assets:/sometexture.png")
        .whenLoaded(RUN_ON_GL_THREAD, [](auto & file_buffer) {
            upload_texture(my_texture_handle, file_buffer);
        })
        .ifNotAvailable(RUN_ON_EVENT_THREAD, [](auto & file_name) {
            notifyFailedToLoadTexture(file_name);
        });
    
        // Note on semantics:
        //    whenLoaded / whenAvailable => run when file first loaded, and whenever file updated or created
        //    whenNotAvailable           => run if file does not exist, and whenever file deleted
        //    ifNotAvailable             => run _once_ if file does not exist
        //
        //    loadFileAsBuffer: load file into single fixed-size buffer, calling .whenLoaded(buffer).
        //      If file size is too large for a reasonably sized buffer, logs an error and does not log anything.
        //
        //    loadFileAsChunkedBuffer(chunk size)
        //      .onLoadBegin()
        //      .onLoadChunk(chunk)
        //
        //    loadFileAsFStream(
    
    
    
    // The usual way to do this
    ResourceLayer::loadFileAsBuffer("~/Library/Application Support/conf.lua")
        .expectBufferSizeBelow(4 * 1024 * 1024)
        .whenLoaded(RUN_ON_APP_THREAD, [](auto & file_buffer) {
            // load + exec config
        })
        .ifNotAvailable([](auto & file_name) {
            // crash
        });
    
    // Though we probably want to do this instead: (and call this before we load most resourcelayer stuff)
    ResourceLayer::doDirectFileLoadAsBuffer("~/Library/Application Support/conf.lua")
        .onLoad([](auto & file_buffer) {
            // load + exec config
        })
        .onFail([]() {
            // crash
        });
}




struct FSDirWatcher {
    CFStringRef dir_path;
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 3.0;
    
    FSDirWatcher (const char * path) {
        printf("setup event stream listener on '%s'\n", path);
        GlobalFileChangeWatcher::getInstance().add_dir(path);
        
        dir_path = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
        CFArrayRef pathsToWatch = CFArrayCreate(kCFAllocatorDefault, (const void**)&dir_path, 1, nullptr);
        
        stream = FSEventStreamCreate(kCFAllocatorDefault,
                (FSEventStreamCallback)&fsEventCallback,
                nullptr,
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
