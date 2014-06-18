
#include "lua_extensions.h"

#if __cplusplus
extern "C" {
#endif
// socket
#include "luasocket/luasocket.h"
#include "luasocket/mime.h"
#include "luafilesystem/src/lfs.h"
#include "luaexpat-1.3.0/src/lxplib.h"
#include "luacurl/luacurl.h"
#include "luamd5/src/md5.h"

static luaL_Reg luax_exts[] = {
    {"socket.core", luaopen_socket_core},
    {"mime.core", luaopen_mime_core},
	{"lfs",luaopen_lfs},
	{"lxp",luaopen_lxp},
	{"curl",luaopen_luacurl},
	{"md5",luaopen_md5_core},
    {NULL, NULL}
};

void luaopen_lua_extensions(lua_State *L)
{
    // load extensions
    luaL_Reg* lib = luax_exts;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    for (; lib->func; lib++)
    {
        lua_pushcfunction(L, lib->func);
        lua_setfield(L, -2, lib->name);
    }
    lua_pop(L, 2);
}

#if __cplusplus
} // extern "C"
#endif
