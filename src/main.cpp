//
//  main.cpp
//  GLSandbox
//
//  Created by semery on 12/8/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "common/app.hpp"
#include <iostream>
#include <fstream>
#include "boost/filesystem.hpp"

using namespace std;
using namespace boost::filesystem;

#define OSX_APP_BUNDLE true

std::string resolveResourcePath (const std::string & rootPath, const std::string & moduleDir, const std::string & fileName)
{
    std::ifstream file;
    std::string s = rootPath;
    return (s += "common/"+fileName, file.open(s), file) ||
    (s = rootPath, s += "modules/"+moduleDir+fileName, file.open(s), file) ? s : "";
}

int main(int argc, const char * argv[]) {
#ifdef OSX_APP_BUNDLE
    const path RESOURCE_PATH = path(argv[0]).remove_leaf().remove_leaf() / "Resources";
    assert(exists(RESOURCE_PATH) && is_directory(RESOURCE_PATH));
#endif
    assert(exists(RESOURCE_PATH / "basic_shader.fs"));
    
    try {
        gl_sandbox::Application app (RESOURCE_PATH.string().c_str());
        app.run();
    } catch (gl_sandbox::InitializationError & e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
