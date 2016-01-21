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
    void getVal (const char * expr, boost::filesystem::path & v, bool must_be_valid_path = false);
    
    // table operations
    void newtable () { lua_newtable(L); }
    void setfield (const char * k, lua_CFunction v, int table_index = -1) {
        lua_pushstring(L, k);
        lua_pushcfunction(L, v);
        lua_rawset(L, table_index - 2);
    }
    void setfield (const char * k, int v, int table_index = -1) {
        lua_pushstring(L, k);
        lua_pushnumber(L, (double)v);
        lua_rawset(L, table_index -2);
    }
    void setfield (const char * k, const std::string & v, int table_index = -1) {
        lua_pushstring(L, k);
        lua_pushlstring(L, v.c_str(), v.size());
        lua_rawset(L, table_index -2);
    }
    void setfield (const char * k, const boost::filesystem::path & v, int table_index = -1) {
        setfield(k, v.string(), table_index);
    }
    
    void setglobal (const char * name) {
        lua_setglobal(L, name);
    }
    
    
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
