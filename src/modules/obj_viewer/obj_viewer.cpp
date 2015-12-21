//
//  obj_viewer.cpp
//  GLSandbox
//
//  Created by semery on 12/20/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <future>

#include "obj_viewer.hpp"

using namespace gl_sandbox;
using namespace gl_sandbox::modules;
using namespace gl_sandbox::gl;

typedef ObjViewer::ShaderRef ShaderRef;


// Loads an .obj model 
void ObjViewer::loadModel(const std::string &modelName) {
    loadModelAsync(modelName);
}

void ObjViewer::loadModelAsync(const std::string &modelName) {
    std::cout << "Loading '" << modelName << "'\n";
    double startTime = glfwGetTime();
    m_resourceLoader.loadObjAsync(modelName, [=](const ResourceLoader::ObjData & modelData) {
        double loadTime = glfwGetTime() - startTime;
        
        std::cout << "Loaded '" << modelName << "' (took " << loadTime << " seconds)\n";
        std::cout << "Has " << modelData.shapes.size() << " shapes, "
        << modelData.materials.size() << " materials\n";
        for (auto & shape : modelData.shapes) {
            std::cout << "Shape '" << shape.name << "'\n";
        }
    });
}



ShaderRef ObjViewer::loadShader (const std::string & shaderName) {
    auto it = m_shaderCache.find(shaderName);
    if (it != m_shaderCache.end())
        return it->second;
    
    auto shader = std::make_shared<Shader>(shaderName);
    m_resourceLoader.loadTextFile(shaderName + ".fs", [&shader](const char *src) {
        shader->compileFragment(src);
    }) &&
    m_resourceLoader.loadTextFile(shaderName + ".vs", [&shader](const char *src) {
        shader->compileVertex(src);
    }) &&
    shader->linkProgram() ?
        (std::cout << "Successfully loaded shader '" << shader->name << "'\n") :
        (std::cout << "Failed to load shader '" << shader->name << "'\n");
    
    m_shaderCache.insert({ shaderName, shader });
    return shader;
}

ObjViewer::ObjViewer ()
{
    std::cout << "Initializing model viewer\n";
    
    loadModelAsync("dragon.obj");
    loadModelAsync("cube.obj");
}

ObjViewer::ModelInstance::ModelInstance () {
    
}
    
void ObjViewer::ModelInstance::draw () {

}

ObjViewer::~ObjViewer() {
    std::cout << "Killing model viewer\n";
}
void ObjViewer::drawFrame() {
    m_resourceLoader.finishAsyncTasks();
    for (auto & model : m_modelInstances)
        model.draw();
}



