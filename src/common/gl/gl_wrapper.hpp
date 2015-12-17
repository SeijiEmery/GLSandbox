//
//  gl_wrapper.hpp
//  GLSandbox
//
//  Created by semery on 12/16/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef gl_wrapper_hpp
#define gl_wrapper_hpp

#include "gl_traits.hpp"
#include "gl_shaders.hpp"
#include <functional>
#include <boost/format.hpp>
#include <iostream>

namespace gl_sandbox {
namespace gl {
namespace references {
typedef std::shared_ptr<Shader> Shader;
typedef std::shared_ptr<VAO>    VAO;
typedef std::shared_ptr<VBO>    VBO;
    
}; // namespace references
}; // namespace gl
}; // namespace gl_sandbox

#endif /* gl_wrapper_hpp */
