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
        auto & app = gl_sandbox::Application::instance();
        app.run();
    } catch (std::runtime_error & e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return 0;
}
