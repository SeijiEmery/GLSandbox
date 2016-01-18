//
//  jstest.cpp
//  GLSandbox
//
//  Created by semery on 1/17/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include "jstest.hpp"

using namespace gl_sandbox;

JSTestModule::JSTestModule () {
    std::cout << "loading jstest module\n";
    
    m_jsInstance.doEval("print();print(2+2==5)");
}
JSTestModule::~JSTestModule () {
    std::cout << "killing jstest module\n";
}



