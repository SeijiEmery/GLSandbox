//
//  main.cpp
//  resourcelayer-test
//
//  Created by semery on 1/23/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include "../src/common/resourcelayer/resource_layer.hpp"
#include "../src/common/resourcelayer/file_watcher.hpp"
#include <iostream>
#include <sstream>
#include <thread>

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

//std::mutex g_coutMutex;
//
//struct TestReporter {
//    std::string title;
//    std::stringstream report;
//    std::function<void(const std::stringstream &)> logReport;
//    
//    unsigned num_passed = 0, num_failed = 0;
//    bool reportFailures = true, reportSuccesses = false;
//    
//    TestReporter (std::string title, decltype(logReport) logReport) : title(title), logReport(logReport) {
//        report << "-- " << title << " --\n";
//    }
//    TestReporter (std::string title) :
//        TestReporter(title, [](const std::stringstream & results) {
//            std::lock_guard<decltype(g_coutMutex)> lock (g_coutMutex);
//            std::cout << results.str() << "\n";
//        }) {}
//    
//    void pass (const std::string & what) {
//        if (reportSuccesses)
//            report << "SUCCESS: " << what << "\n";
//        ++num_passed;
//    }
//    void fail (const std::string & what) {
//        if (reportFailures)
//            report << "FAILED:  " << what << "\n";
//        ++num_failed;
//    }
//    ~TestReporter () {
//        report << num_passed << " / " << (num_passed + num_failed) << " tests passed\n";
//        logReport(report);
//    }
//};
//
//typedef std::function<bool()> TestCase;
//typedef std::pair<std::string, TestCase> LabeledTestCase;
//
//void runTestCases (TestReporter & r, std::initializer_list<LabeledTestCase> cases) {
//    for (auto labeledCase : cases) {
//        if (!labeledCase.second()) {
//            r.pass(labeledCase.first);
//        }
//    }
//}
//
//std::vector<std::thread> g_launchedThreads;
//class TestRunner {
//    std::vector<std::thread> launchedThreads;
//    
//public:
//    void launch (std::function<void()> f) {
//        launchedThreads.emplace_back(f);
//    }
//    void launch (std::string title, std::function<void(TestReporter &)> f) {
//        launch([=]{
//            TestReporter r (title);
//            f(r);
//        });
//    }
//    ~TestRunner () {
//        for (auto & thread : launchedThreads) {
//            thread.join();
//        }
//    }
//};
//
//void TestTests (TestRunner & runner) {
//    runner.launch("Test test", [](TestReporter & reporter) {
//        
//        // do init...
//        
//        runTestCases (reporter, {
//            { "this test should pass", [=]{
//                return true;
//            }},
//            { "this test should fail", [=]{
//                return false;
//            }}
//        });
//    });
//}

void tryWatchingForFiles () {
    using namespace resource_impl::platform_osx;
    
    DirectoryWatcherInstance watcher;
    
    std::cout << "Creating file watcher\n";
    
    auto watchFile = [&](const FilePath & path) {
        return watcher.watchForChanges(path, [](auto path){
            std::cout << "File maybe changed in: " << path << "\n";
        }, [](auto err) { std::cerr << err.what() << "\n"; }, false);
    };
    
    watchFile("/");
    watchFile(resolvedPath("~/foo"))->detatch();
    watchFile(resolvedPath("~/misc-projects"));
    watchFile(resolvedPath("~/"));

    std::this_thread::sleep_for(std::chrono::nanoseconds((long)10e9));
}

int main(int argc, const char * argv[]) {
    
//    tryDirectLoad("~/Library/Application Support/GLSandbox/conf.lua");
    
    tryWatchingForFiles();
    
    return 0;
}

