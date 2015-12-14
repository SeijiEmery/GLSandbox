//
//  main.cpp
//  GLSandbox
//
//  Created by semery on 12/8/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "common/app.hpp"
#include <iostream>

using namespace std;

int main(int argc, const char * argv[]) {
    try {
        gl_sandbox::Application app;
        app.run();
    } catch (gl_sandbox::InitializationError & e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
