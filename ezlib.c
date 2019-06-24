/*
** Copyright (C) 2019 Arseny Vakhrushev <arseny.vakhrushev@gmail.com>
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/

#include <zlib.h>
#include <lauxlib.h>

#define MODNAME "lua-ezlib"
#define VERSION "0.1.0"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT int luaopen_ezlib(lua_State *L);

static int f_type(lua_State *L) {
	size_t len;
	const char *str = luaL_checklstring(L, 1, &len);
	if (len >= 2) {
		unsigned char c0 = str[0];
		unsigned char c1 = str[1];
		if ((c0 & 15) == 8 && !((c0 * 256 + c1) % 31)) {
			lua_pushliteral(L, "zlib");
			return 1;
		}
		if (c0 == 0x1f && c1 == 0x8b) {
			lua_pushliteral(L, "gzip");
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int f_deflate(lua_State *L) {
	const uInt max = -1;
	size_t len, tip = 100, old = 0;
	void *ud, *ptr, *buf = 0;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	int err;
	z_stream zs;
	zs.next_in = (Bytef *)luaL_checklstring(L, 1, &len);
	zs.avail_in = 0;
	zs.avail_out = 0;
	zs.zalloc = 0;
	zs.zfree = 0;
	zs.opaque = 0;
	if ((err = deflateInit2(&zs, luaL_optinteger(L, 3, -1), Z_DEFLATED, lua_toboolean(L, 2) ? 31 : 15, 8, 0))) goto error;
	do {
		if (!zs.avail_in) {
			zs.avail_in = len > max ? max : len;
			len -= zs.avail_in;
		}
		if (!zs.avail_out) {
			zs.avail_out = tip > max ? max : tip;
			ptr = allocf(ud, buf, old, old + zs.avail_out);
			if (!ptr) {
				err = Z_MEM_ERROR;
				break;
			}
			zs.next_out = (Bytef *)ptr + zs.total_out;
			buf = ptr;
			tip <<= 1;
			old += zs.avail_out;
		}
	} while (!(err = deflate(&zs, len ? 0 : Z_FINISH)));
	deflateEnd(&zs);
	if (err == Z_STREAM_END) lua_pushlstring(L, buf, zs.total_out);
	allocf(ud, buf, old, 0);
	if (err == Z_STREAM_END) return 1;
error:
	return luaL_error(L, "%s", zError(err));
}

static int f_inflate(lua_State *L) {
	const uInt max = -1;
	size_t len, tip = 100, old = 0;
	void *ud, *ptr, *buf = 0;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	int err;
	z_stream zs;
	zs.next_in = (Bytef *)luaL_checklstring(L, 1, &len);
	zs.avail_in = 0;
	zs.avail_out = 0;
	zs.zalloc = 0;
	zs.zfree = 0;
	zs.opaque = 0;
	if ((err = inflateInit2(&zs, lua_toboolean(L, 2) ? 31 : 15))) goto error;
	do {
		if (!zs.avail_in) {
			zs.avail_in = len > max ? max : len;
			len -= zs.avail_in;
		}
		if (!zs.avail_out) {
			zs.avail_out = tip > max ? max : tip;
			ptr = allocf(ud, buf, old, old + zs.avail_out);
			if (!ptr) {
				err = Z_MEM_ERROR;
				break;
			}
			zs.next_out = (Bytef *)ptr + zs.total_out;
			buf = ptr;
			tip <<= 1;
			old += zs.avail_out;
		}
	} while (!(err = inflate(&zs, 0)));
	inflateEnd(&zs);
	if (err == Z_STREAM_END) lua_pushlstring(L, buf, zs.total_out);
	allocf(ud, buf, old, 0);
	if (err == Z_STREAM_END) return 1;
error:
	return luaL_error(L, "%s", zError(err));
}

static const luaL_Reg l_ezlib[] = {
	{"type", f_type},
	{"deflate", f_deflate},
	{"inflate", f_inflate},
	{0, 0}
};

int luaopen_ezlib(lua_State *L) {
#if LUA_VERSION_NUM < 502
	luaL_register(L, lua_tostring(L, 1), l_ezlib);
#else
	luaL_newlib(L, l_ezlib);
#endif
	lua_pushliteral(L, MODNAME);
	lua_setfield(L, -2, "_NAME");
	lua_pushliteral(L, VERSION);
	lua_setfield(L, -2, "_VERSION");
	return 1;
}
