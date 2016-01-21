//
//  lua_instance.cpp
//  GLSandbox
//
//  Created by semery on 1/19/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include "lua_instance.hpp"
#include <cassert>
#include <boost/filesystem.hpp>

using namespace gl_sandbox;

LuaInstance::LuaInstance (std::string instance_name, bool load_libs)
    : m_instance_name(instance_name)
{
    L = luaL_newstate();
    assert(L != nullptr);
    if (load_libs)
        luaL_openlibs(L);
}

LuaInstance::~LuaInstance() {
    lua_close(L);
}

void LuaInstance::run(const char *buffer, size_t len, const char *fname) {
    int error = luaL_loadbuffer(L, buffer, len, fname) ||
                lua_pcall(L, 0, 0, 0);
    if (error) {
        fprintf(stderr, "lua error '%s': %s", fname, lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}



//template <typename... Args>
//struct Length;
//
//template <>
//struct Length<> {
//    enum { value = 0 };
//};
//template <typename T, typename... Rest>
//struct Length<T, Rest...> {
//    enum { value = Length<Rest...>::value + 1 };
//};

template <typename... Args>
struct LuaCall {
    lua_State * L;
    const char * fname;
    int nrv = 1;
    static auto constexpr nargs = sizeof...(Args);
    
#define APPLY_ARGS(T, apply_one) \
    template <typename... Rest> \
    void apply_args (T v, Rest... rest) { \
        apply_one; apply_args(rest...); \
    }
    APPLY_ARGS(int, lua_pushinteger(L, v))
    APPLY_ARGS(unsigned, lua_pushunsigned(L, v))
    APPLY_ARGS(float, lua_pushnumber(L, (double)v))
    APPLY_ARGS(double, lua_pushnumber(L, (double)v))
    APPLY_ARGS(std::string, lua_pushlstring(L, v.c_str(), v.size()))
    APPLY_ARGS(const char *, lua_pushstring(L, v))
    APPLY_ARGS(lua_types::Nil, lua_pushnil(L))
    APPLY_ARGS(bool, lua_pushboolean(L, v))
    
    void apply_args () {
        lua_call(L, nargs, nrv);
    }
    LuaCall (decltype(L) L, decltype(fname) fname, Args... args) :
        L(L), fname(fname) { apply_args(args...); }
};

template <typename... Args>
void LuaInstance::call (const char * fname, Args... args) {
    LuaCall<Args...>(L, fname, args...);
}

struct Foo {
    
};

void test () {
    LuaInstance script ("test-instance");
    script.call("foo", 10, 12);
    
    Foo foo;
//    script.call("foo", foo, 12);
}

void test2 () {
    
}


void LuaInstance::evalExpr (const char * expr, int expected_type) {
    snprintf(&m_strbuf[0], m_strbuf.size(), "return %s", expr);
    auto err = luaL_loadstring(L, &m_strbuf[0]) || lua_pcall(L, 0, 1, 0);
    if (err) {
        throw std::runtime_error(lua_tostring(L, -1));
    }
    if (lua_type(L, -1) != expected_type) {
        throw std::runtime_error(
            std::string("Expected ")+luaL_typename(L,expected_type)+", not "+luaL_typename(L, lua_type(L, -1))+" from expr '"+expr+"'\n");
    }
}

void LuaInstance::getVal (const char * expr, int & v) {
    evalExpr(expr, LUA_TNUMBER);
    v = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
}
void LuaInstance::getVal (const char * expr, unsigned & v) {
    evalExpr(expr, LUA_TNUMBER);
    v = (unsigned)lua_tounsigned(L, -1);
    lua_pop(L, 1);
}
void LuaInstance::getVal (const char * expr, std::string & v) {
    evalExpr(expr, LUA_TSTRING);
    v = lua_tostring(L, -1);
    lua_pop(L, 1);
}
void LuaInstance::getVal (const char * expr, bool & v) {
    evalExpr(expr, LUA_TBOOLEAN);
    v = lua_toboolean(L, -1);
    lua_pop(L, 1);
}
void LuaInstance::getVal (const char * expr, boost::filesystem::path & v, bool must_be_valid_path) {
    evalExpr(expr, LUA_TSTRING);
    v = lua_tostring(L, -1);
    if (must_be_valid_path && !boost::filesystem::exists(v))
        throw std::runtime_error(std::string("Path from '")+expr+"' does not exist ("+v.string()+")");
    lua_pop(L, 1);
}

















































