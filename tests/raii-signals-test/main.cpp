//
//  main.cpp
//  raii-signals-test
//
//  Created by semery on 12/23/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "../../src/common/raii_signal.hpp"
#include <iostream>
#include <cassert>

using namespace gl_sandbox;

struct Foo {
    int foo = -1;
    std::string name = "";
};
struct Bar {
    int foo = -1;
    int baz = -1;
    std::string name = "";
};
struct Emitter {
    raii::Signal<int> broadcastFoo;
    raii::Signal<int> broadcastBar;
    raii::Signal<std::string> broadcastName;
};

void testSignals () {
    Foo foo;
    Bar bar;
    Bar baz;
    Emitter e;
    
    assert(foo.foo == -1 && foo.name == "");
    assert(bar.foo == -1 && bar.baz == -1 && bar.name == "");
    
    {
        auto observer1 = e.broadcastFoo.connect([&](int v){
            foo.foo = v;
        });
        auto observer2 = e.broadcastFoo.connect([&](int v) {
            bar.foo = v;
        });
        auto observer3 = e.broadcastBar.connect([&](int v) {
            bar.foo = v;
        });
        auto observer4 = e.broadcastName.connect([&](std::string name){
            foo.name = name;
        });
        
        e.broadcastFoo.emit(2);
        assert(foo.foo == 2 && bar.foo == 2);
        e.broadcastBar.emit(3);
        assert(foo.foo == 2 && bar.foo == 3);
        
        e.broadcastName.emit("fooble");
        assert(foo.name == "fooble" && bar.name == "");
        {
            auto observer5 = e.broadcastName.connect([&](std::string name){
                bar.name = name;
            });
            e.broadcastName.emit("barble");
            assert(foo.name == "barble" && bar.name == "barble");
        }
        e.broadcastName.emit("fred");
        assert(foo.name == "fred" && bar.name == "barble");
    }
    
    e.broadcastFoo.emit(10);
    assert(foo.foo != 10 && bar.foo != 10);
    e.broadcastBar.emit(-1);
    assert(foo.foo != -1 && bar.foo != -1);
    e.broadcastName.emit("");
    assert(foo.name != "" && bar.name != "");
    
    {
        auto tmp1 = e.broadcastFoo.connect([&](int v){
            foo.foo = v;
        });
        auto tmp2 = e.broadcastFoo.connect([&](int v){
            bar.baz = v;
        });
        e.broadcastFoo.emit(12);
        assert(foo.foo == 12 && bar.baz == 12 && bar.foo != 12);
    }
    e.broadcastFoo.emit(13);
    assert(foo.foo != 13 && bar.baz != 13);
}

int main(int argc, const char * argv[]) {
    testSignals();
    std::cout << "Tests passed\n";
    return 0;
}
