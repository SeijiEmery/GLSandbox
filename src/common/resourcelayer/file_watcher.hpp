//
//  file_watcher.hpp
//  GLSandbox
//
//  Created by semery on 1/25/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef file_watcher_hpp
#define file_watcher_hpp

#include "resource_impl.hpp"

namespace gl_sandbox {
namespace resource_impl {

// Handle for an existing directory that is being watched.
// detatch() and the destructor release the directory from the watch list.
struct IDirectoryWatcherHandle {
    virtual void detatch () = 0;
    virtual ~IDirectoryWatcherHandle () {}
};
typedef std::shared_ptr<IDirectoryWatcherHandle> DirectoryWatcherHandleRef;
    
    
struct IFileWatcher {
    // Callback actions (only one of each, so calling again will override)
    virtual IFileWatcher & onFileModified (std::function<void(const FilePath &)>) = 0;
    virtual IFileWatcher & onFileDeleted (std::function<void(const FilePath &)>) = 0;
    virtual IFileWatcher & onFileCreated (std::function<void(const FilePath &)>) = 0;
    virtual IFileWatcher & onFileRenamed (std::function<void(const FilePath &, const FilePath &)>) = 0;
    
    // Release file watcher (stop watching file)
    virtual void release () = 0;
    
    // Set release / don't release on dtor (default on)
    virtual IFileWatcher & setAutorelease (bool) = 0;
    virtual ~IFileWatcher () {}
};
typedef std::shared_ptr<IFileWatcher> FileWatcherRef;
    
    
// Platform-specific osx implementation
namespace platform_osx {
    
// Singleton filesystem watcher (watches directories), implemented on osx using
// FSEvents and a CFRunLoop thread.
//
// Note: not technically a singleton, since you're expected to create one instance and
// pass that around. Creating multiple instances is technically supported for testing
// purposes, but is strongly inadvised (inefficent, and would create multiple threads)
//
class DirectoryWatcherInstance {
public:
    DirectoryWatcherHandleRef watchForChanges (
        const FilePath & dirPath,
        const ThreadCallable<void(const FilePath&)> onChanged,
        const ThreadCallable<void(const ResourceError &)> onError,
        bool autorelease = true);
    
    DirectoryWatcherInstance ();
    ~DirectoryWatcherInstance ();
    
    struct Impl;
protected:
    std::unique_ptr<Impl> impl;
};
    
}; // namespace platform_osx
    
namespace platform_bsd {
    
// Alternative implementation using kqueues.
class DirectoryWatcherInstance {
public:
    DirectoryWatcherHandleRef watchForChanges (
       const FilePath & dirPath,
       const ThreadCallable<void(const FilePath&)> onChanged,
       const ThreadCallable<void(const ResourceError &)> onError,
       bool autorelease = true);
    
    FileWatcherRef watchFileForChanges (
        const FilePath & path,
        const std::function<void(ResourceError &)> onError);
    
    FileWatcherRef watchDirForChanges (
        const FilePath & path,
        const std::function<bool(const FilePath&)>& fileFilter,
        const std::function<void(ResourceError &)>& onError);
    
    FileWatcherRef watchDirForRecursiveChanges (
        const FilePath & path,
        const std::function<bool(const FilePath&)> &fileFilter,
        const std::function<bool(const FilePath&)> &subdirFilter,
        const std::function<void(ResourceError &)> &onError);
    
    DirectoryWatcherInstance ();
    ~DirectoryWatcherInstance ();
    
    struct Impl;
protected:
    std::unique_ptr<Impl> impl;
};
    
}; // namespace platform_bsd
    

namespace platform_windows {
    // ...
};
namespace platform_linux {
    // ...
};
        
}; // namespace resource
}; // namespace gl_sandbox

#endif /* file_watcher_hpp */
