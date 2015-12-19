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

namespace gl_sandbox {
    
struct ResourceError : public std::runtime_error {
    using std::runtime_error::runtime_error; // constructor
};
    
class Application;
class Module;
class ResourceLoader {
    void executeAsync (std::function<void()>);
public:
    ResourceLoader (const char * baseResourcePath) :
        m_baseResourcePath(baseResourcePath) {}
    
    struct ImageInfo { int size_x, size_y, image_format; };
    struct ObjData {
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
    };
    
    typedef boost::filesystem::path Path;
    typedef std::function<void(const char *)> TextHandler;
    typedef std::function<void(const uint8_t *, const ImageInfo & info)> ImageHandler;
    typedef std::function<void (const ObjData &)> ObjHandler;
    typedef std::function<void (const ResourceError &)> ErrorHandler;
    
    bool loadTextFile (const Path & filepath, TextHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadImage (const Path & filepath, ImageHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadObj (const Path & filepath, ObjHandler onComplete, ErrorHandler onError = dumpToStdout);

    bool loadTextFile (const char * filename, const char * moduleDir, TextHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadImage (const char * filename, const char * moduleDir, ImageHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadObj (const char * filename, const char * moduleDir, ObjHandler onComplete, ErrorHandler onError = dumpToStdout);
    
    
    // CRTP Helper class -- Module derives from this to expose basic resource loading methods to module implementations.
    // This struct thus defines what resource loading methods are exposed to modules, and how they get implemented.
    template <typename T>
    struct ModuleInterface {
    protected:
        ModuleInterface (ResourceLoader * resourceLoaderRef)
            : m_resourceLoaderRef(resourceLoaderRef) { assert(m_resourceLoaderRef != nullptr); }
        ResourceLoader * m_resourceLoaderRef;
        
#define GET_MODULE_DIR() static_cast<const T*>(this)->getDirName()
        bool loadTextFile (const char * filename, TextHandler onComplete) const {
            return m_resourceLoaderRef->loadTextFile(filename, GET_MODULE_DIR(), onComplete);
        }
        bool loadImage (const char * filename, ImageHandler onComplete) const {
            return m_resourceLoaderRef->loadImage(filename, GET_MODULE_DIR(), onComplete);
        }
        bool loadObj (const char * filename, ObjHandler onComplete) const {
            return m_resourceLoaderRef->loadObj(filename, GET_MODULE_DIR(), onComplete);
        }
#undef GET_MODULE_DIR
    };
protected:
    static void dumpToStdout (const ResourceError & e) {
        std::cerr << e.what() << std::endl;
    }
    bool resolvePath (const char * filename, const char * moduleDir, Path & path);
protected:
    boost::filesystem::path m_baseResourcePath;
};

}; // namespace gl_sandbox

#endif /* resources_hpp */
