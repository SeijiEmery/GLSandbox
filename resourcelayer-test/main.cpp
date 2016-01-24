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

void tryDirectLoad (std::string filepath) {
    ResourceLayer::doDirectFileLoadAsBuffer(filepath)
        .onLoaded([](auto & buffer) {
            printf("loaded file '%s' (%lu bytes)\n", buffer.path.c_str(), buffer.size);
            printf("contents:\n%s\n", buffer.data);
        })
        .onFail([](auto path) {
            printf("could not load file '%s' (as buffer)\n", path.c_str());
        });
    ResourceLayer::doDirectFileLoadAsFstream(filepath)
        .onLoaded([=](std::fstream & stream) {
            printf("loaded file '%s' (fstream)\n", filepath.c_str());
        })
        .onFail([](auto path) {
            printf("could not load file '%s' (as fstream)\n", path.c_str());
        });
    ResourceLayer::doDirectFileLoadAsFILE(filepath)
        .onLoaded([=](auto file) {
            printf("loaded file '%s' (FILE)\n", filepath.c_str());
        })
        .onFail([](auto path) {
            printf("could not load file '%s' (as FILE)\n", path.c_str());
        });
    
    
    ResourceLayer::doDirectFileLoadAsBuffer("~") // kinda unexpected, but okay... on unix this loads a 4kb file, apparently...
        .onLoaded([](auto & buffer) {
            printf("loaded ~ (%s) -- %lu bytes\n", buffer.path.c_str(), buffer.size);
            printf("contents:\n");
            for (auto i = 0; i < buffer.size; ++i) {
                printf("%d ", buffer.data[i]);
            }
            printf("\n");
        })
        .onFail([](auto path) {
            printf("could not load ~ (%s)\n", path.c_str());
        });
}

//void createTexturePrefix () {
//    ResourceLayer::createResourcePrefix()
//        .inDirectory("~/misc-projects/GLSandbox/assets")
//        .forFileExts(".png", ".jpg")
//        .watchForFileChanges(true)
//        .notifyWhenFileSizeOrHashChanges();
//}
//void createScriptPipeline () {
//    ResourceLayer::createPipeline()
//        .sourceDirectory("~/misc-projects/GLSandbox/script/")
//        .targetDirectory("~/Library/Application Support/script/generated/")
//        .sourceFileExts(".moon")
//        .targetFileExts(".lua")
//        .watchForFileChanges(true)
//        .whenTargetDesync([](auto & src_path, auto & target_path) {
//            // run moonc...
//        })
//        .notifyWhenTargetFileSizeOrHashChanges();
//}


int main(int argc, const char * argv[]) {
    
    tryDirectLoad("~/Library/Application Support/GLSandbox/conf.lua");
    
    return 0;
}

