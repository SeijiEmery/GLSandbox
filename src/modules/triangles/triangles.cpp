//
//  triangles.cpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "triangles.hpp"

#include <iostream>

using namespace gl_sandbox;

const char * TriangleModule::MODULE_NAME = "module-triangles";


TriangleModule::TriangleModule ()
    : Module(MODULE_NAME)
{
    std::cout << "Initializing triangle module" << std::endl;
}
TriangleModule::~TriangleModule() {
    std::cout << "Killing triangle module" << std::endl;
}
void TriangleModule::drawFrame() {
    std::cout << "Running triangle module" << std::endl;
}



