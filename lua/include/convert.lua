--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--

-- Convert bytes table to integer
function bytes_to_int(bytes, offset, length) 

	local t = table.slice(bytes, offset, length)
    local n = 0
    for k = 0, #t do
        n = n + byte_to_utf8(t[k]) * 2^((k)*8)
    end
    return n
end

-- Convert bytes table to string
function bytes_to_string(bytes)
	local t = bytes;
	local bytearr = {}
	for k = 0, #t do
		table.insert( bytearr, string.char(byte_to_utf8(t[k])) )
	end
	return string:trim( table.concat(bytearr) )
end

-- Convert C byte to Dec
function byte_to_utf8(v)
	return v < 0 and (0xff + v + 1) or v
end