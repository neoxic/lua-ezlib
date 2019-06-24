Easy zlib module for Lua
========================

[lua-ezlib] provides the following API:

### ezlib.deflate(data, [gzip], [level])
Deflates `data` into a _zlib_ (or _gzip_ if `gzip = true`) compressed chunk.
Optional compression `level` must be between 0 and 9 (or -1 as the default).

### ezlib.inflate(data, [gzip])
Inflates `data` from a _zlib_ (or _gzip_ if `gzip = true`) compressed chunk.

### ezlib.type(data)
Returns the type of compressed `data` as `'zlib'`, `'gzip'` or `nil`.


Building and installing with LuaRocks
-------------------------------------

To build and install, run:

    luarocks make
    luarocks test

To install the latest release using [luarocks.org], run:

    luarocks install lua-ezlib


[lua-ezlib]: https://github.com/neoxic/lua-ezlib
[luarocks.org]: https://luarocks.org
