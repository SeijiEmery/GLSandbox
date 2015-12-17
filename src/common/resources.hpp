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
    
    typedef std::function<void(const char *)> TextHandler;
    typedef std::function<void (const ResourceError &)> ErrorHandler;
    
    bool loadTextFile (const boost::filesystem::path & filepath, TextHandler onComplete, ErrorHandler onError = dumpToStdout);
    bool loadTextFile (const char * filePath, TextHandler onComplete, ErrorHandler onError = dumpToStdout);
protected:
    static void dumpToStdout (const ResourceError & e) {
        std::cerr << e.what() << std::endl;
    }

protected:
    boost::filesystem::path m_baseResourcePath;
};

}; // namespace gl_sandbox

#endif /* resources_hpp */
