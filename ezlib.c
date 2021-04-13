/*
** Copyright (C) 2019-2021 Arseny Vakhrushev <arseny.vakhrushev@me.com>
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

#include <lauxlib.h>
#include <zlib.h>

#define MODNAME "lua-ezlib"
#define VERSION "1.0.0"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT int luaopen_ezlib(lua_State *L);

#if ZLIB_VERNUM < 0x1290
#define crc32_z crc32
#define adler32_z adler32
#endif

static int wbits[] = {15, 31, -15, 47};
static const char *const dfmt[] = {"zlib", "gzip", "raw", 0};
static const char *const ifmt[] = {"zlib", "gzip", "raw", "auto", 0};

static int f_deflate(lua_State *L) {
	const uInt max = -1;
	void *ud, *ptr, *buf = 0;
	size_t len, tip = 100, size = 0;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	const char *str = luaL_checklstring(L, 1, &len);
	int fmt = luaL_checkoption(L, 2, dfmt[0], dfmt);
	int lvl = luaL_optinteger(L, 3, 6);
	int err;
	z_stream zs;
	zs.next_in = (Bytef *)str;
	zs.avail_in = 0;
	zs.avail_out = 0;
	zs.zalloc = 0;
	zs.zfree = 0;
	zs.opaque = 0;
	luaL_argcheck(L, lvl >= 0 && lvl <= 9, 3, "value out of range");
	if ((err = deflateInit2(&zs, lvl, Z_DEFLATED, wbits[fmt], 8, 0))) goto error;
	do {
		if (!zs.avail_in) {
			zs.avail_in = len > max ? max : len;
			len -= zs.avail_in;
		}
		if (!zs.avail_out) {
			zs.avail_out = tip > max ? max : tip;
			ptr = allocf(ud, buf, size, size + zs.avail_out);
			if (!ptr) {
				err = Z_MEM_ERROR;
				break;
			}
			zs.next_out = (Bytef *)ptr + zs.total_out;
			buf = ptr;
			tip <<= 1;
			size += zs.avail_out;
		}
	} while (!(err = deflate(&zs, len ? 0 : Z_FINISH)));
	deflateEnd(&zs);
	if (err == Z_STREAM_END) lua_pushlstring(L, buf, zs.total_out);
	allocf(ud, buf, size, 0);
	if (err == Z_STREAM_END) return 1;
error:
	return luaL_error(L, "%s", zError(err));
}

static int f_inflate(lua_State *L) {
	const uInt max = -1;
	void *ud, *ptr, *buf = 0;
	size_t len, tip = 100, size = 0;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	const char *str = luaL_checklstring(L, 1, &len);
	int fmt = luaL_checkoption(L, 2, ifmt[0], ifmt);
	int err;
	z_stream zs;
	zs.next_in = (Bytef *)str;
	zs.avail_in = 0;
	zs.avail_out = 0;
	zs.zalloc = 0;
	zs.zfree = 0;
	zs.opaque = 0;
	if ((err = inflateInit2(&zs, wbits[fmt]))) goto error;
	do {
		if (!zs.avail_in) {
			zs.avail_in = len > max ? max : len;
			len -= zs.avail_in;
		}
		if (!zs.avail_out) {
			zs.avail_out = tip > max ? max : tip;
			ptr = allocf(ud, buf, size, size + zs.avail_out);
			if (!ptr) {
				err = Z_MEM_ERROR;
				break;
			}
			zs.next_out = (Bytef *)ptr + zs.total_out;
			buf = ptr;
			tip <<= 1;
			size += zs.avail_out;
		}
	} while (!(err = inflate(&zs, 0)));
	inflateEnd(&zs);
	if (err == Z_STREAM_END) lua_pushlstring(L, buf, zs.total_out);
	allocf(ud, buf, size, 0);
	if (err == Z_STREAM_END) return 1;
error:
	return luaL_error(L, "%s", zError(err));
}

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

static int f_crc32(lua_State *L) {
	size_t len;
	const char *str = luaL_checklstring(L, 1, &len);
	lua_pushinteger(L, crc32_z(crc32(0, 0, 0), (Bytef *)str, len));
	return 1;
}

static int f_adler32(lua_State *L) {
	size_t len;
	const char *str = luaL_checklstring(L, 1, &len);
	lua_pushinteger(L, adler32_z(adler32(0, 0, 0), (Bytef *)str, len));
	return 1;
}

static const luaL_Reg l_ezlib[] = {
	{"deflate", f_deflate},
	{"inflate", f_inflate},
	{"type", f_type},
	{"crc32", f_crc32},
	{"adler32", f_adler32},
	{0, 0}
};

int luaopen_ezlib(lua_State *L) {
#if LUA_VERSION_NUM < 502
	luaL_register(L, "ezlib", l_ezlib);
#else
	luaL_newlib(L, l_ezlib);
#endif
	lua_pushliteral(L, MODNAME);
	lua_setfield(L, -2, "_NAME");
	lua_pushliteral(L, VERSION);
	lua_setfield(L, -2, "_VERSION");
	return 1;
}
