Easy zlib module for Lua
========================

[lua-ezlib] provides the following API:

### ezlib.deflate(str, [fmt], [lvl])
Deflates `str` using the format `fmt` (a string) that can be one of the following:
- `zlib`: zlib format (default);
- `gzip`: gzip format;
- `raw`: raw deflate;

Optional compression level `lvl` must be between 0 and 9 (the default is 6).

### ezlib.inflate(str, [fmt])
Inflates `str` using the format `fmt` (a string) that can be one of the following:
- `zlib`: zlib format (default);
- `gzip`: gzip format;
- `raw`: raw inflate;
- `auto`: zlib or gzip format;

### ezlib.type(str)
Returns the type of compressed data in `str`. Possible values are `'zlib'`, `'gzip'` or `nil`.

### ezlib.crc32(str)
Returns a CRC-32 checksum for `str`.

### ezlib.adler32(str)
Returns an Adler-32 checksum for `str`.


Building and installing with LuaRocks
-------------------------------------

To build and install, run:

    luarocks make
    luarocks test

To install the latest release using [luarocks.org], run:

    luarocks install lua-ezlib


[lua-ezlib]: https://github.com/neoxic/lua-ezlib
[luarocks.org]: https://luarocks.org
