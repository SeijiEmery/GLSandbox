//
//  main.cpp
//  resourcelayer-test
//
//  Created by semery on 1/23/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include "../src/common/resourcelayer/resource_layer.hpp"
#include <iostream>

using namespace gl_sandbox;
using namespace resource;
using resource_impl::FileBufferRef;

void tryDirectLoad (std::string filepath) {
    ResourceLayer::loadAsBuffer(filepath)
        .onLoaded({[](auto buffer) {
            printf("loaded file '%s' (%lu bytes)\n", buffer->path.c_str(), buffer->size);
            printf("contents:\n%s\n", buffer->data);
        }})
        .onLoadFailed({[](auto path) {
            printf("could not load file '%s' (as buffer)\n", path.c_str());
        }});
    ResourceLayer::loadAsFStream(filepath)
        .onLoaded({[=](auto stream) {
            printf("loaded file '%s' (fstream)\n", filepath.c_str());
        }})
        .onLoadFailed({[](auto path) {
            printf("could not load file '%s' (as fstream)\n", path.c_str());
        }});
    ResourceLayer::loadAsCFile (filepath, "r")
        .onLoaded({[=](auto file) {
            printf("loaded file '%s' (FILE)\n", filepath.c_str());
        }})
        .onLoadFailed({ [](auto path, auto err) {
            printf("could not load file '%s' (as FILE)\n", path.c_str());
        }});
    
    
    ResourceLayer::loadAsBuffer("~") // kinda unexpected, but okay... on unix this loads a 4kb file, apparently...
        .onLoaded({[](const FileBufferRef & buffer) {
            printf("loaded ~ (%s) -- %lu bytes\n", buffer->path.c_str(), buffer->size);
            printf("contents:\n");
            for (auto i = 0; i < buffer->size; ++i) {
                printf("%d ", buffer->data[i]);
            }
            printf("\n");
        }})
        .onLoadFailed({[](auto path) {
            printf("could not load ~ (%s)\n", path.c_str());
        }});
}


int main(int argc, const char * argv[]) {
    
    tryDirectLoad("~/Library/Application Support/GLSandbox/conf.lua");
    
    return 0;
}

