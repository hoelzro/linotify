/*
* Copyright (c) 2009-2017 Robert Hoelz <rob@hoelz.ro>
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

#define MT_NAME "INOTIFY_HANDLE"
#define READ_BUFFER_SIZE 1024
#define INVALID_FD (-1)

struct inotify_context {
    char buffer[READ_BUFFER_SIZE];
    int offset;
    int bytes_remaining;
};

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
    int flags = 0;

    if(lua_type(L, 1) == LUA_TTABLE) {
        lua_getfield(L, 1, "blocking");

        if(lua_type(L, -1) != LUA_TNIL && !lua_toboolean(L, -1)) {
            flags |= IN_NONBLOCK;
        }
        lua_pop(L, 1);
    }

    if((fd = inotify_init1(flags)) == -1) {
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

static void
push_inotify_event(lua_State *L, struct inotify_event *ev)
{
    lua_createtable(L, 0, 4);

    lua_pushinteger(L, ev->wd);
    lua_setfield(L, -2, "wd");

    lua_pushinteger(L, ev->mask);
    lua_setfield(L, -2, "mask");

    lua_pushinteger(L, ev->cookie);
    lua_setfield(L, -2, "cookie");

    if(ev->len) {
        lua_pushstring(L, ev->name);
        lua_setfield(L, -2, "name");
    }
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
        if(errno == EAGAIN || errno == EWOULDBLOCK) {
            lua_newtable(L);
            return 1;
        }
        return handle_error(L);
    }
    lua_newtable(L);

    while(bytes >= sizeof(struct inotify_event)) {
        iev = (struct inotify_event *) (buffer + i);

        push_inotify_event(L, iev);
        lua_rawseti(L, -2, n++);

        i += (sizeof(struct inotify_event) + iev->len);
        bytes -= (sizeof(struct inotify_event) + iev->len);
    }

    return 1;
}

static int
handle_events_iterator(lua_State *L)
{
    struct inotify_context *context;
    struct inotify_event *event;
    int fd;

    fd      = get_inotify_handle(L, 1);
    context = lua_touserdata(L, lua_upvalueindex(1));

    if(context->bytes_remaining < sizeof(struct inotify_event)) {
        context->offset = 0;

        if((context->bytes_remaining = read(fd, context->buffer, READ_BUFFER_SIZE)) < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                lua_pushnil(L);
                return 1;
            }
            return luaL_error(L, "read error: %s\n", strerror(errno));
        }
    }
    event = (struct inotify_event *) (context->buffer + context->offset);

    context->bytes_remaining -= (sizeof(struct inotify_event) + event->len);
    context->offset          += (sizeof(struct inotify_event) + event->len);

    push_inotify_event(L, event);

    return 1;
}

static int
handle_events(lua_State *L)
{
    struct inotify_context *context;

    context = lua_newuserdata(L, sizeof(struct inotify_context));

    memset(context, 0, sizeof(struct inotify_context));

    lua_pushcclosure(L, handle_events_iterator, 1);
    lua_pushvalue(L, 1);

    return 2;
}

static int handle_close(lua_State *L)
{
    int *fd = (int *) luaL_checkudata(L, 1, MT_NAME);
    if( *fd != INVALID_FD ){
        close(*fd);
        *fd = INVALID_FD;
    }
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

static luaL_Reg inotify_funcs[] = {
    {"init", init},
    {NULL, NULL}
};

static luaL_Reg handle_funcs[] = {
    {"read", handle_read},
    {"close", handle_close},
    {"addwatch", handle_add_watch},
    {"rmwatch", handle_rm_watch},
    {"fileno", handle_fileno},
    {"getfd", handle_fileno},
    {"events", handle_events},
    {NULL, NULL}
};

#define register_constant(s)\
    lua_pushinteger(L, s);\
    lua_setfield(L, -2, #s);

int luaopen_inotify(lua_State *L)
{
    luaL_newmetatable(L, MT_NAME);
    lua_createtable(L, 0, sizeof(handle_funcs) / sizeof(luaL_Reg) - 1);
#if LUA_VERSION_NUM > 501
    luaL_setfuncs(L, handle_funcs, 0);
#else
    luaL_register(L, NULL, handle_funcs);
#endif
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, handle__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "inotify_handle");
    lua_setfield(L, -2, "__type");
    lua_pop(L, 1);

    lua_newtable(L);
#if LUA_VERSION_NUM > 501
    luaL_setfuncs(L, inotify_funcs,0);
#else
    luaL_register(L, NULL, inotify_funcs);
#endif

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
