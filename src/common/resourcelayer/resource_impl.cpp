//
//  resource_impl.cpp
//  GLSandbox
//
//  Created by semery on 1/24/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "resource_impl.hpp"

namespace gl_sandbox {
namespace resource_impl {
    
// ============================================================
//                       FILE LOADING
// ============================================================
    
namespace detail {
    template <typename FileBufferCallback, typename FilePathCallback>
    bool loadFileAsBuffer (
        const FilePath & path,
        const FileBufferCallback & onLoad,
        const FilePathCallback & onFail
    ) {
        std::ifstream f (path);
        if (f) {
            f.seekg(0, std::ios::end);
            size_t size = f.tellg();
            f.seekg(0, std::ios::beg);
            if (size != 0) {
                auto buffer = std::make_shared<FileBuffer>(
                        path, new uint8_t[size+1], size,
                        [](auto &buf) { delete[] buf.data; });
                
                f.read((char*)buffer->data, buffer->size);
                return onLoad(buffer), true;
            }
        }
        return onFail(path), false;
    }
    
    template <typename IFStreamCallback, typename FilePathCallback>
    bool loadFileAsIFstream (
        const FilePath & path,
        const IFStreamCallback & onLoad,
        const FilePathCallback & onFail
    ) {
        std::ifstream f (path);
        if (f) {
            return onLoad(f), true;
        } else {
            return onFail(f), false;
        }
    }

    template <typename CFileCallback, typename CFileErrorCallback>
    bool loadFileAsCFile (
        const FilePath & path, const char * mode,
        const CFileCallback & onLoad,
        const CFileErrorCallback & onFail
    ) {
        FILE* file = fopen(path.c_str(), mode);
        if (file) {
            onLoad(file);
            fclose(file);
            return true;
        } else {
            onFail(path, errno);
            return false;
        }
    }
};
namespace immediate {
    bool loadFileImmediate (
        const FilePath & path,
        const FileBufferCallback & onLoad,
        const FilePathCallback & onFail
    ) {
        return detail::loadFileAsBuffer(path, onLoad, onFail);
    }
        
    bool loadFileImmediate (
        const FilePath & path,
        const IFStreamCallback & onLoad,
        const FilePathCallback & onFail
    ) {
        return detail::loadFileAsBuffer(path, onLoad, onFail);
    }

    bool loadFileImmediate (
        const FilePath & path,
        const char * mode,
        const CFileCallback & onLoad,
        const CFileErrorCallback & onFail
    ) {
        return detail::loadFileAsCFile(path, mode, onLoad, onFail);
    }
};
namespace async {
    void loadFileAsync (
        const FilePath & path,
        const FileBufferCallback & onLoad,
        const FilePathCallback & onFail
    ) {
        detail::loadFileAsBuffer(path, onLoad, onFail);
    }

    void loadFileAsync (
        const FilePath & path,
        const IFStreamCallback & onLoad,
        const FilePathCallback & onFail
    ) {
        detail::loadFileAsBuffer(path, onLoad, onFail);
    }

    void loadFileAsync (
        const FilePath & path,
        const char * mode,
        const CFileCallback & onLoad,
        const CFileErrorCallback & onFail
    ) {
        detail::loadFileAsCFile(path, mode, onLoad, onFail);
    }
};
    
// ============================================================
//                      FILE PATH UTILS
// ============================================================
    
namespace utils {
    FilePath joinPath (
        const FilePath & first,
        const FilePath & rest
    ) {
        if (first.length() > 0 && first[first.length()-1] == '/')
            return rest[0] == '/' ?
                first + (rest.c_str() + 1) :
                first + rest;
        return rest[0] == '/' ?
            first + rest :
            first + "/" + rest;
    }
    
    FilePath & subst (
        FilePath & path,
        const std::string & pattern,
        const std::string & repl
    ) {
        size_t n = 0;
        while ((n = path.find(pattern, n)) != std::string::npos) {
            path.replace(n, pattern.size(), repl);
        }
        return path;
    }
    
    FilePath & resolveUserDirectory (FilePath & path) {
        assert(path.length() > 0);
        if (path[0] == '~' && path.length() == 1)
            return path = getenv("HOME"), path;
        if (path[0] == '~' && path.length() > 1 && path[1] == '/')
            return path.replace(0, 1, getenv("HOME")), path;
        return path;
    }
    FilePath & resolvePath (FilePath & path) {
        resolveUserDirectory(path);
        return path;
    }
    
    bool getRealPath (
        FilePath & path,
        std::initializer_list<std::function<bool(errno_t)>> errorHandlers
    ) {
        const char * rpath = realpath(path.c_str(), nullptr);
        if (rpath) {
            path = rpath;
            free((void*)rpath);
            return true;
        } else {
            for (auto & handler : errorHandlers) {
                if (handler(errno)) {
                    return false;
                }
            }
            switch (errno) {
                case EACCES: throw ResourceError("Unhandled realpath() error: EACCES (%s)", path);
                case EINVAL: throw ResourceError("Unhandled realpath() error: EINVAL (%s)", path);
                case EIO:    throw ResourceError("Unhandled realpath() error: EIO (%s)", path);
                case ELOOP:  throw ResourceError("Unhandled realpath() error: ELOOP (%s)", path);
                case ENAMETOOLONG: throw ResourceError("Unhandled realpath() error: ENAMETOOLONG (%s)", path);
                case ENOMEM: throw ResourceError("Unhandled realpath() error: ENOMEM (%s)", path);
                case ENOENT: throw ResourceError("Unhandled realpath() error: ENOENT (%s)", path);
                case ENOTDIR: throw ResourceError("Unhandled realpath() error: ENOTDIR (%s)", path);
                default: throw ResourceError("Unhandled realpath() error: %d (%s)", errno, path);
            }
        }
    }
};
    
    
    
    
    
    
    
    
    
    
    

}; // namespace gl_sandbox
}; // namespace resource_impl;











