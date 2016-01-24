//
//  resource_impl.hpp
//  GLSandbox
//
//  Created by semery on 1/24/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef resource_impl_hpp
#define resource_impl_hpp

#include <functional>
#include <vector>
#include <string>

namespace gl_sandbox {

struct ThreadTarget {
    void enqueue (std::function<void()> closure);
};
    
template <typename T>
struct ThreadCallable {
protected:
    std::function<T> fcn;
    ThreadTarget*    target;
    
public:
    ThreadCallable(decltype(target) target, decltype(fcn) fcn) :
        target(target), fcn(fcn) {}
    
    template <typename... Args>
    void operator () (Args... args) {
        target->enqueue([=]() {
            fcn.call(args...);
        });
    }
};

}; // namespace gl_sandbox



namespace gl_sandbox {
namespace resource_impl {
    
typedef std::string FilePath;
    
struct FileBuffer {
    const FilePath path;
    const uint8_t * data;
    size_t size;
protected:
    std::function<void(FileBuffer&)> release;
public:
    FileBuffer (decltype(path) path,
                decltype(data) data,
                decltype(size) size,
                decltype(release) onRelease) :
    path(path), data(data), size(size), release(onRelease) {}
    
    ~FileBuffer () {
        release(*this);
    }
};
    
namespace utils {
    FilePath joinPath (const FilePath & first, const FilePath & rest);
    FilePath & resolveUserDirectory (FilePath & path);
    FilePath & resolvePath (FilePath & path);

    // Use realpath to resolve / set path.
    // If realpath returns an error, uses errorHandlers (applied in order) to attempt to recover.
    //   errorhandler := (realpath error value) -> true iff handled | false
    //   error values at man7.org/linux/man-pages/man3/realpath.3.html.
    // Throws a resource error if no error handler returns true.
    //
    // Returns true if succeeded, or false for a handled error (unhandled throws an exception).
    // Sets path to real path iff succeeded.
    bool tryGetRealPath (FilePath & path,
        std::initializer_list<std::function<bool(int)>> errorHandlers);
};
namespace filehash {
    // Calculate file hash using XYZ. Can pass in filesize if already known, or can calculate using fseek.
    // Note: the behavior of an invalid FILE* / filesize is completely undefined, but file errors will probably be
    // handled by throwing a resource exception (which you don't want, so make sure these are valid!)
    std::string & md5hash    (FILE* file, std::string & hashvalue, long int filesize = -1);
    std::string & sha1hash   (FILE* file, std::string & hashvalue, long int filesize = -1);
    std::string & murmurhash (FILE* file, std::string & hashvalue, long int filesize = -1);
    
    // Hash timing info generated from hashComparative(...) calls
    struct HashTimingStats {
        struct Sample {
            long file_size = 0;
            long md5_time  = 0;
            long sha1_time = 0;
            long murmur_time = 0;
            long fopen_time  = 0;  // set iff using FilePath, not FILE*
            bool used_fopen = false;
        };
        std::vector<Sample> samples;
    };
    
    // Run all hashes and collect timing info.
    void hashComparative (FILE* file, std::string & md5, std::string & sha1, std::string & murmur,
                          HashTimingStats & stats, long int filesize = -1);
    
    void hashComparative (const FilePath & path, std::string & md5, std::string & sha1, std::string & murmur,
                          HashTimingStats & stats, std::function<void(const FilePath &)> handleFileOpenError);
};
    
// Cross-thread file loading
namespace async {
    typedef ThreadCallable<void(const FileBuffer&)>     FileBufferCallback;
    typedef ThreadCallable<void(const std::ifstream&)>  IFStreamCallback;
    typedef ThreadCallable<void(const FILE *)>          CFileCallback;
    typedef ThreadCallable<void(const FilePath &)>      FilePathCallback;
        
    void loadFileAsync (const FilePath & path, const FileBufferCallback & onLoad, const FilePathCallback & onFail);
    void loadFileAsync (const FilePath & path, const IFStreamCallback & onLoad, const FilePathCallback & onFail);
    void loadFileAsync (const FilePath & path, const CFileCallback & onLoad, const FilePathCallback & onFail);
};
    
// Immediate (non-threaded) file loading
namespace immediate {
    typedef std::function<void(const FileBuffer&)> FileBufferCallback;
    typedef std::function<void(const std::ifstream &)> IFStreamCallback;
    typedef std::function<void(const FILE*)> CFileCallback;
    typedef std::function<void(const FilePath&)> FilePathCallback;
    
    void loadFileImmediate (const FilePath & path, const FileBufferCallback & onLoad, const FilePathCallback & onFail);
    void loadFileImmediate (const FilePath & path, const IFStreamCallback & onLoad, const FilePathCallback & onFail);
    void loadFileImmediate (const FilePath & path, const CFileCallback & onLoad, const FilePathCallback & onFail);
};
    
}; // namespace resource_impl
}; // namespace gl_sandbox

#endif /* resource_impl_hpp */
