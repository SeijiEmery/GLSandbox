//
//  resourcelayer.hpp
//  GLSandbox
//
//  Created by semery on 1/23/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef resourcelayer_hpp
#define resourcelayer_hpp

//#include <functional>
//#include <string>
//#include <atomic>
//#include <cassert>
//#include <cstdio>
//#include <fstream>

#include "resource_impl.hpp"
#include "resource_layer_declarative.hpp"

namespace gl_sandbox {
namespace resource {
using resource_impl::FilePath;
using resource_impl::FileBuffer;
using resource_impl::FileBufferRef;
    
using resource_impl::utils::resolvedPath;
using resource_impl::utils::joinPath;
using resource_impl::utils::subst;
    
struct ResourceLayer {
    static auto loadAsBuffer (const FilePath & path) {
        return declarative::LoadAsFileBuffer { resolvedPath(path) };
    }
    static auto loadAsCFile (const FilePath & path, const char * mode) {
        return declarative::LoadAsCFile { resolvedPath(path), mode };
    }
    static auto loadAsFStream (const FilePath & path) {
        return declarative::LoadAsIFstream { resolvedPath(path) };
    }
};
    
    
    
//// public api
//struct ResourceLayer;
//    
//
//
//// non-public...
//    
//typedef std::string FilePath;
//typedef std::function<void(const FilePath &)> FilePathCallback;
//
//struct TempFileBuffer {
//    const FilePath path;
//    const uint8_t * data;
//    size_t size;
//protected:
//    std::function<void(TempFileBuffer&)> release;
//public:
//    TempFileBuffer (
//                    decltype(path) path,
//                    decltype(data) data,
//                    decltype(size) size,
//                    decltype(release) onRelease) :
//    path(path), data(data), size(size), release(onRelease) {}
//    
//    ~TempFileBuffer () {
//        release(*this);
//    }
//};
//    
//    
//struct IFilesystemCache {};
//struct IFileChangeNotifier {};
//struct IFileLoader {};
//struct IThreadScheduler {};
//
//struct IResourceExecutor {
//    
//};
//
//template <class FSAccessPolicy, class FSWatcherPolicy, class FSLoadPolicy, class FSSchedulingPolicy>
//class ResourceExecutor : public IResourceExecutor, public FSAccessPolicy, public FSWatcherPolicy, public FSLoadPolicy, public FSSchedulingPolicy {
//    static void execute (std::function<void(IResourceExecutor&)> f) {
//        
//    }
//    static void executeImmediate (std::function<void(IResourceExecutor&)> f) {
//        
//    }
//};
//    
//
//// Shared request functionality
//template <typename T>
//struct ResourceRequest {};
//    
//
//struct ResourcePipelineCreationRequest {
//
//};
//struct ResourcePrefixCreationRequest {
//    ResourcePrefixCreationRequest () {}
//    ResourcePrefixCreationRequest (FilePath prefix) {}
//};
//    
//template <typename T>
//class LoadFileRequest : public ResourceRequest<LoadFileRequest<T>> {
//    typedef LoadFileRequest<T> This;
//    typedef std::function<void(T)> FileCallback;
//    typedef std::function<void(const FilePath&)> FilePathCallback;
//    
//public:
//    LoadFileRequest (FilePath path) {}
//    
//    This & whenLoaded (ThreadRequest requested_thread, FileCallback onLoad) {
//        // ...
//        return *this;
//    }
//    This & onLoadFail (ThreadRequest requested_thread, FilePathCallback onFail) {
//        // ...
//        return *this;
//    }
//    This & onFileDeleted (ThreadRequest requested_thread, FilePathCallback onDeleted) {
//        // ...
//        return *this;
//    }
//    This & whenLoaded (FileCallback onLoad) {
//        return whenLoaded(ThreadRequest::DEFAULT_THREAD, onLoad);
//    }
//    This & onLoadFail (FilePathCallback onFail) {
//        return onLoadFail(ThreadRequest::DEFAULT_THREAD, onFail);
//    }
//    This & onFileDeleted (FilePathCallback onDeleted) {
//        return onFileDeleted(ThreadRequest::DEFAULT_THREAD, onDeleted);
//    }
//
//    ~LoadFileRequest () {
//        // execute request...
//    }
//};
//
//FilePath & resolvePath (FilePath & path) {
//    assert(path.length() > 0);
//    if (path[0] == '~' && path.length() == 1)
//        return path = getenv("HOME"), path;
//    if (path[0] == '~' && path[1] == '/')
//        return path.replace(0, 1, getenv("HOME")), path;
//    else
//        return path;
//}
//    
//    
//void doDirectLoad (FilePath & path, const std::function<void(FILE*)> onLoad, FilePathCallback onFail) {
//    FILE* file = fopen(resolvePath(path).c_str(), "r");
//    if (file) onLoad(file);
//    else      onFail(path);
//}
//void doDirectLoad (FilePath & path, const std::function<void(TempFileBuffer&)> onLoad, FilePathCallback onFail) {
//    std::ifstream stream (resolvePath(path));
//    if (stream) {
//        stream.seekg(0, std::ios::end);
//        size_t size = stream.tellg();
//        stream.seekg(0, std::ios::beg);
//        if (size != 0) {
//            TempFileBuffer buffer (path, new uint8_t[size+1], size, [](auto & buf) {
//                delete[] buf.data;
//            });
//            stream.read((char*)buffer.data, size);
//            onLoad(buffer);
//            return;
//        }
//    }
//    onFail(path);
//}
//void doDirectLoad (FilePath & path, const std::function<void(std::fstream&)> onLoad, FilePathCallback onFail) {
//    std::fstream stream (resolvePath(path));
//    if (stream) onLoad(stream);
//    else        onFail(path);
//}
//
//template <typename T>
//class DirectFileLoadRequest : public ResourceRequest<DirectFileLoadRequest<T>> {
//    typedef DirectFileLoadRequest This;
//    typedef std::function<void(T)> FileCallback;
//    typedef std::function<void(const FilePath&)> FilePathCallback;
//    
//    FilePath         m_path;
//    FileCallback     m_onLoad;
//    FilePathCallback m_onFail;
//    bool hasOnLoad = false;
//    bool hasOnFail = false;
//public:
//    DirectFileLoadRequest (FilePath path) : m_path(path) {}
//    
//    This & onLoaded (FileCallback cb) {
//        assert(!hasOnLoad);
//        m_onLoad = cb; hasOnLoad = true;
//        return *this;
//    }
//    This & onFail (FilePathCallback cb) {
//        assert(!hasOnFail);
//        m_onFail = cb; hasOnFail = true;
//        return *this;
//    }
//    This & onFail (std::function<void()> cb) {
//        return onLoadFailed([=](const FilePath&) { cb(); });
//    }
//    ~DirectFileLoadRequest () {
//        assert(hasOnLoad || hasOnFail);
//        doDirectLoad(m_path,
//                     hasOnLoad ? m_onLoad : [](T _) {},
//                     hasOnFail ? m_onFail : [](auto _) {}
//        );
//    }
//};
//
//
//    
//struct ResourceLayer {
//    // Public API
//    static auto createResourcePipeline () -> ResourcePipelineCreationRequest { return {}; }
//    static auto createResourcePrefix (std::string prefix) -> ResourcePrefixCreationRequest { return { prefix }; }
//    static auto createResourcePrefix () -> ResourcePrefixCreationRequest { return {}; }
//    
//    static auto loadFileAsBuffer  (std::string filename) -> LoadFileRequest<TempFileBuffer&> { return { filename }; }
//    static auto loadFileAsFstream (std::string filename) -> LoadFileRequest<std::fstream&> { return { filename }; }
//    static auto loadFileAsFILE    (std::string filename) -> LoadFileRequest<FILE*> { return { filename }; }
//    
//    static auto doDirectFileLoadAsBuffer (FilePath filename) -> DirectFileLoadRequest<TempFileBuffer&> {
//        return { filename };
//    }
//    static auto doDirectFileLoadAsFstream (FilePath filename) -> DirectFileLoadRequest<std::fstream&> {
//        return { filename };
//    }
//    static auto doDirectFileLoadAsFILE (FilePath filename) -> DirectFileLoadRequest<FILE*> {
//        return { filename };
//    }
//}; // class ResourceLayer
    
}; // namespace resource
}; // namespace gl_sandbox

#endif /* resourcelayer_hpp */
