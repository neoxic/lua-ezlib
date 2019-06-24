local ezlib = require 'ezlib'

local function randstr(n)
	local t = {}
	for i = 1, math.random(0, n) do
		t[i] = string.char(math.random(0, 255))
	end
	return table.concat(t)
end

math.randomseed(os.time())

for i = 1, 1000 do
	local str = randstr(10000)
	local gzip = math.random() < 0.5
	local data = ezlib.deflate(str, gzip)
	assert(ezlib.type(data) == (gzip and 'gzip' or 'zlib'))
	assert(ezlib.inflate(data, gzip) == str)
end

assert(ezlib.type('abc') == nil)
assert(not pcall(ezlib.inflate, 'abc'))
