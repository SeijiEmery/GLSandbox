//
//  lua_instance.hpp
//  GLSandbox
//
//  Created by semery on 1/19/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef lua_instance_hpp
#define lua_instance_hpp

#include "lua.hpp"
#include <string>
#include <array>
#include <boost/filesystem/path.hpp>

namespace gl_sandbox {

struct lua_types {
struct Nil {};
};
    
// Singlethreaded lua state instance + wrapper class
class LuaInstance {
public:
    LuaInstance (std::string instance_name, bool load_libs = true);
    ~LuaInstance ();
    
    // Call an existing lua function with args
    template <typename... T>
    void call (const char * fname, T... args);
    
    // Run script / expr
    void run (const char * buffer, size_t len, const char * fname);
    
    // Run expr and store result in v
    void getVal (const char * expr, int & v);
    void getVal (const char * expr, unsigned & v);
    void getVal (const char * expr, std::string & v);
    void getVal (const char * expr, bool & v);
    void getVal (const char * expr, boost::filesystem::path & v, bool must_be_valid_path = true);
    
    // Hack to get state to implement other ops
    lua_State * getState () { return L; }
    
protected:
    // Helper functions
    void evalExpr (const char * expr, int expected_type);
    
protected:
    lua_State *L;
    std::string m_instance_name;
    std::array<char,512> m_strbuf; // used for tmp string ops
};
    
    
    
    
};

#endif /* lua_instance_hpp */
