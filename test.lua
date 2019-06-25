local ezlib = require 'ezlib'

math.randomseed(os.time())

local fmts = {'zlib', 'gzip', 'raw'}
for i = 1, 1000 do
	local t = {}
	for i = 1, math.random(0, 10000) do
		t[i] = string.char(math.random(0, 255))
	end
	local str = table.concat(t)
	local fmt = fmts[math.random(#fmts)]
	local lvl = math.random(0, 9)
	local data = ezlib.deflate(str, fmt, lvl)
	assert(ezlib.type(data) == (fmt ~= 'raw' and fmt or nil))
	assert(ezlib.inflate(data, fmt ~= 'raw' and math.random() < 0.5 and 'auto' or fmt) == str)
end

assert(not pcall(ezlib.deflate, 'abc', 'INVALID-FORMAT'))
assert(not pcall(ezlib.deflate, 'abc', nil, -1))
assert(not pcall(ezlib.deflate, 'abc', nil, 10))
assert(not pcall(ezlib.inflate, 'abc', 'INVALID-FORMAT'))
assert(not pcall(ezlib.inflate, 'abc'))

assert(ezlib.crc32('abc') == 0x352441c2)
assert(ezlib.adler32('abc') == 0x24d0127)
