//
//  obj_viewer.cpp
//  GLSandbox
//
//  Created by semery on 12/20/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/math/constants/constants.hpp>
#include <future>

#include "obj_viewer.hpp"

using namespace gl_sandbox;
using namespace gl_sandbox::modules;
using namespace gl_sandbox::gl;

typedef ObjViewer::ShaderRef ShaderRef;

struct GeometryOptimizer {
    
};

void optimizeGeometry (const ResourceLoader::ObjData & data, unsigned vertLimit = 1600) {
    
}

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
            std::cout << shape.mesh.indices.size() << " indices\n";
            std::cout << shape.mesh.positions.size() << " vertices\n";
            std::cout << shape.mesh.normals.size() << " normals\n";
            std::cout << shape.mesh.texcoords.size() << " uvs\n";
            std::cout << shape.mesh.material_ids.size() << " material ids\n";
        }
        
        // Lock iff we're doing any threading (this is safe, since the callback to loadObjAsync
        // gets called on the main thread during finishAsyncTasks())
        // This is also why we don't need locks for cout, etc.,
        m_modelInstances.emplace_back(modelData, loadShader("diffuse_1light"));
    });
}

ObjViewer::ModelInstance::ModelInstance (const ResourceLoader::ObjData & data, ShaderRef s)
    : shader(s)
{
    
    CHECK_GL_ERRORS();
    
    assert(data.shapes.size() > 0);
    const auto & indices = data.shapes[0].mesh.indices;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[0].handle); CHECK_GL_ERRORS();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    numIndices = indices.size();
    
    const auto & positions = data.shapes[0].mesh.positions;
    const auto & normals   = data.shapes[0].mesh.normals;
    
    glBindVertexArray(vao.handle); CHECK_GL_ERRORS();
    glEnableVertexAttribArray(0); CHECK_GL_ERRORS();
    glEnableVertexAttribArray(1); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1].handle); CHECK_GL_ERRORS();
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), &positions[0], GL_STATIC_DRAW); CHECK_GL_ERRORS();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2].handle); CHECK_GL_ERRORS();
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW); CHECK_GL_ERRORS();
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL_ERRORS();
    
    glBindVertexArray(0); CHECK_GL_ERRORS();
    glUseProgram(0); CHECK_GL_ERRORS();
    
#define GET_UNIFORM(k) (shaderUniforms.k = shader->getUniformLocation(#k))
    GET_UNIFORM(LightPosition);
    GET_UNIFORM(Kd);
    GET_UNIFORM(Ld);
    GET_UNIFORM(ModelViewMatrix);
    GET_UNIFORM(NormalMatrix);
//    GET_UNIFORM(ProjectionMatrix);
    GET_UNIFORM(MVP);
#undef GET_UNIFORM
    
    startTime = glfwGetTime();
}

void ObjViewer::ModelInstance::draw () {
    CHECK_GL_ERRORS();
    
    glUseProgram(shader->handle()); CHECK_GL_ERRORS();
    glBindVertexArray(vao.handle);  CHECK_GL_ERRORS();
    
    glm::vec4 lightPosition;
    
//    uniform vec4 LightPosition;
//    uniform vec3 Kd;
//    uniform vec3 Ld;
//    
//    uniform mat4 ModelViewMatrix;
//    uniform mat3 NormalMatrix;
//    uniform mat4 MVP;
    
    double elapsed = glfwGetTime() - startTime;
    const double LIGHT_ROTATION_SPEED = 5.0 * boost::math::constants::pi<double>();
    
#define SET_UNIFORM(k, v) shader->setUniform(shaderUniforms.k, (v))
    
    double angle = elapsed * LIGHT_ROTATION_SPEED;
    
    SET_UNIFORM(LightPosition, glm::vec4((float)cos(angle), 0, (float)sin(angle), 1)); CHECK_GL_ERRORS();
    SET_UNIFORM(Kd, glm::vec3(0.5)); CHECK_GL_ERRORS();
    SET_UNIFORM(Ld, glm::vec3(1.0)); CHECK_GL_ERRORS();
    
    auto model = glm::translate(glm::mat4x4(1.0), glm::vec3(0, 0, -2.0));
//    auto model = glm::mat4x4(1.0);
    auto view  = glm::mat4x4(1.0);
    glm::mat4x4 proj = glm::perspective(75.0f, 1.7f, 0.1f, 300.0f);
    
    SET_UNIFORM(ModelViewMatrix, model * view); CHECK_GL_ERRORS();
    SET_UNIFORM(NormalMatrix, glm::mat3x3(1.0f)); CHECK_GL_ERRORS();
    SET_UNIFORM(MVP, model * view * proj); CHECK_GL_ERRORS();
    
#undef SET_UNIFORM
    
    CHECK_GL_ERRORS();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[0].handle);            CHECK_GL_ERRORS();
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)0); CHECK_GL_ERRORS();
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
    
//    loadModelAsync("dragon.obj");
    loadModelAsync("cube.obj");
//    loadModelAsync("sibenik.obj");
}

ObjViewer::~ObjViewer() {
    std::cout << "Killing model viewer\n";
}
void ObjViewer::drawFrame() {
    m_resourceLoader.finishAsyncTasks();
    for (auto & model : m_modelInstances)
        model.draw();
}



