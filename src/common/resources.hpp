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
protected:
    static void dumpToStdout (const ResourceError & e) {
        std::cerr << e.what() << std::endl;
    }
    bool resolvePath (const char * filename, const char * moduleDir, Path & path);
protected:
    static boost::filesystem::path g_baseResourcePath;
    boost::filesystem::path m_modulePath;
};
}; // namespace gl_sandbox

#endif /* resources_hpp */
