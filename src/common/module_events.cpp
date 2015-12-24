//
//  module_events.cpp
//  GLSandbox
//
//  Created by semery on 12/23/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#include "module_events.hpp"

#include "app.hpp"

using namespace gl_sandbox;

ModuleExposedEvents::ModuleExposedEvents () :
    input(Application::inputManager()),
    app(Application::appEvents())
{}

