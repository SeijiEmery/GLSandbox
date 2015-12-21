//
//  obj_viewer.cpp
//  GLSandbox
//
//  Created by semery on 12/20/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "obj_viewer.hpp"

using namespace gl_sandbox;
using namespace gl_sandbox::modules;

ObjViewer::ObjViewer ()

{}

ObjViewer::ModelInstance::ModelInstance () {
    
}
    
void ObjViewer::ModelInstance::draw () {
    
}




ObjViewer::~ObjViewer() {
    
}
void ObjViewer::drawFrame() {
    for (auto & model : m_modelInstances)
        model.draw();
}



