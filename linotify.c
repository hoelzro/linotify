/*
* Copyright (c) 2009 Robert Hoelz <rob@hoelzro.net>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <lua.h>
#include <lauxlib.h>

#include <sys/inotify.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define INOTIFY_LIB_NAME "inotify"
#define MT_NAME "INOTIFY_HANDLE"

void push_inotify_handle(lua_State *L, int fd)
{
    int *udata = (int *) lua_newuserdata(L, sizeof(int));
    *udata = fd;
    luaL_getmetatable(L, MT_NAME);
    lua_setmetatable(L, -2);
}

int get_inotify_handle(lua_State *L, int index)
{
    return *((int *) luaL_checkudata(L, index, MT_NAME));
}

static int handle_error(lua_State *L)
{
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno));
    lua_pushinteger(L, errno);
    return 3;
}

static int init(lua_State *L)
{
    int fd;

    if((fd = inotify_init()) == -1) {
        return handle_error(L);
    } else {
        push_inotify_handle(L, fd);
        return 1;
    }
}

static int handle_fileno(lua_State *L)
{
    lua_pushinteger(L, get_inotify_handle(L, 1));
    return 1;
}

static int handle_read(lua_State *L)
{
    int fd;
    int i = 0;
    int n = 1;
    ssize_t bytes;
    struct inotify_event *iev;
    char buffer[1024];

    fd = get_inotify_handle(L, 1);
    if((bytes = read(fd, buffer, 1024)) < 0) {
        return handle_error(L);
    }
    lua_newtable(L);

    while(bytes >= sizeof(struct inotify_event)) {
        iev = (struct inotify_event *) (buffer + i);

        lua_createtable(L, 0, 4);
        lua_pushvalue(L, -1);
        lua_rawseti(L, -3, n++);

        lua_pushinteger(L, iev->wd);
        lua_setfield(L, -2, "wd");

        lua_pushinteger(L, iev->mask);
        lua_setfield(L, -2, "mask");

        lua_pushinteger(L, iev->cookie);
        lua_setfield(L, -2, "cookie");

        if(iev->len) {
            lua_pushstring(L, iev->name);
            lua_setfield(L, -2, "name");
        }

        lua_pop(L, 1);

        i += (sizeof(struct inotify_event) + iev->len);
        bytes -= (sizeof(struct inotify_event) + iev->len);
    }

    return 1;
}

static int handle_close(lua_State *L)
{
    int fd = get_inotify_handle(L, 1);
    close(fd);
    return 0;
}

static int handle_add_watch(lua_State *L)
{
    int fd;
    int wd;
    int top;
    int i;
    const char *path;
    uint32_t mask = 0;

    fd = get_inotify_handle(L, 1);
    path = luaL_checkstring(L, 2);
    top = lua_gettop(L);
    for(i = 3; i <= top; i++) {
        mask |= luaL_checkinteger(L, i);
    }

    if((wd = inotify_add_watch(fd, path, mask)) == -1) {
        return handle_error(L);
    } else {
        lua_pushinteger(L, wd);
        return 1;
    }
}

static int handle_rm_watch(lua_State *L)
{
    int fd;
    int wd;

    fd = get_inotify_handle(L, 1);
    wd = luaL_checkinteger(L, 2);

    if(inotify_rm_watch(fd, wd) == -1) {
        return handle_error(L);
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int handle__gc(lua_State *L)
{
    return handle_close(L);
}

static luaL_reg inotify_funcs[] = {
    {"init", init},
    {NULL, NULL}
};

static luaL_reg handle_funcs[] = {
    {"read", handle_read},
    {"close", handle_close},
    {"addwatch", handle_add_watch},
    {"rmwatch", handle_rm_watch},
    {"fileno", handle_fileno},
    {NULL, NULL}
};

#define register_constant(s)\
    lua_pushinteger(L, s);\
    lua_setfield(L, -2, #s);

int luaopen_inotify(lua_State *L)
{
    luaL_newmetatable(L, MT_NAME);
    lua_createtable(L, 0, sizeof(handle_funcs) / sizeof(luaL_reg) - 1);
    luaL_register(L, NULL, handle_funcs);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, handle__gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_register(L, INOTIFY_LIB_NAME, inotify_funcs);

    register_constant(IN_ACCESS);
    register_constant(IN_ATTRIB);
    register_constant(IN_CLOSE_WRITE);
    register_constant(IN_CLOSE_NOWRITE);
    register_constant(IN_CREATE);
    register_constant(IN_DELETE);
    register_constant(IN_DELETE_SELF);
    register_constant(IN_MODIFY);
    register_constant(IN_MOVE_SELF);
    register_constant(IN_MOVED_FROM);
    register_constant(IN_MOVED_TO);
    register_constant(IN_OPEN);
    register_constant(IN_ALL_EVENTS);
    register_constant(IN_MOVE);
    register_constant(IN_CLOSE);
    register_constant(IN_DONT_FOLLOW);
    register_constant(IN_MASK_ADD);
    register_constant(IN_ONESHOT);
    register_constant(IN_ONLYDIR);
    register_constant(IN_IGNORED);
    register_constant(IN_ISDIR);
    register_constant(IN_Q_OVERFLOW);
    register_constant(IN_UNMOUNT);

    return 1;
}
