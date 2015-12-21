//
//  resources.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef resources_hpp
#define resources_hpp

#include "./gl/gl_wrapper.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "../../libs/tinyobjloader/tiny_obj_loader.h"
#include <string>
#include <array>
#include <future>
#include <mutex>

namespace gl_sandbox {
    
struct ResourceError : public std::runtime_error {
    using std::runtime_error::runtime_error; // constructor
};
class Application;
class ResourceLoader {
    friend class Application;
public:
    ResourceLoader (const char * modulePath) :
        m_modulePath(modulePath) {}
    ~ResourceLoader ();
    
    struct ImageInfo { int size_x, size_y, image_format; };
    struct ObjData {
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        
        ObjData () {}
        ObjData (ObjData && other) :
            shapes(std::move(other.shapes)),
            materials(std::move(other.materials)) {}
    };
    
    typedef boost::filesystem::path Path;
    typedef std::function<void(const char *)> TextHandler;
    typedef std::function<void(const uint8_t *, const ImageInfo & info)> ImageHandler;
    typedef std::function<void (const ObjData &)> ObjHandler;
    typedef std::function<void (const ResourceError &)> ErrorHandler;
    
    bool loadTextFile (const Path & filepath, TextHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadImage (const Path & filepath, ImageHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadObj (const Path & filepath, ObjHandler onComplete, ErrorHandler onError = dumpToStdout);

    // Async versions. Guarantees that:
    // - loading is done in parallel / on another thread (via std::async currently)
    // - callbacks get called on the main thread (cuz that's usually pretty important...)
    // - callbacks get called whenever they feel like it (ie. arbitrary)
    // - calling resourceLoader.finishAsyncTasks() is needed to actually _finish_ any of the tasks
    //   (so callback time is actually not totally arbitrary..)
    // Oh, and don't actually call any of these unless you actually _need_ it, since they have a lot of overhead
    // compared to the regular versions (ie. no async calls for 4kb text files...)
    void loadTextFileAsync (const Path & filepath, TextHandler onComplete, ErrorHandler onError = dumpToStdout);
    void loadImageAsync    (const Path & filepath, ImageHandler onComplete, ErrorHandler onError = dumpToStdout);
    void loadObjAsync      (const Path & filepath, ObjHandler onComplete, ErrorHandler onError = dumpToStdout);
    
    // Call this every frame / whatever if you're using async tasks -- does work (ie. callback execution) that is
    // expected to / needs to be called from the main thread. MUST be called from main thread.
    // Timelimit restricts async execution (as a rough ballpark -- actual time taken is >= timelimit) so this method
    // can't hijack main thread execution + fuck up the framerate.
    void finishAsyncTasks (double timelimit = 1.0 / 60);
    
    // Deprecated
    bool loadTextFile (const char * filename, const char * moduleDir, TextHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadImage (const char * filename, const char * moduleDir, ImageHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadObj (const char * filename, const char * moduleDir, ObjHandler onComplete, ErrorHandler onError = dumpToStdout);
    
protected:
    static void dumpToStdout (const ResourceError & e) {
        std::cerr << e.what() << std::endl;
    }
    bool resolvePath (const char * filename, const char * moduleDir, Path & path);
    void runOnMainThread (std::function<void()>);
protected:
    static boost::filesystem::path g_baseResourcePath;
    boost::filesystem::path m_modulePath;
    
    // thread stuff
    std::mutex                            m_pendingTasksMutex;      // guards m_pendingMainThreadTasks
    std::vector<std::function<void()>> m_pendingMainThreadTasks;    // list of stuff that needs to run on the main thread (LIFO stack; we don't care about execution order)
    std::vector<std::future<void>>        m_runningTasks;           // list of running async stuff (throwaway)
};
}; // namespace gl_sandbox

#endif /* resources_hpp */
