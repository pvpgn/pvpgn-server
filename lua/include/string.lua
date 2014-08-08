--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Split text into table by delimeter
--  Usage example: string.split("one,two",",")
function string:split(str, sep)	
	str = str or '%s+'
	local st, g = 1, self:gmatch("()("..str..")")
	local function getter(segs, seps, sep, cap1, ...)
		st = sep and seps + #sep
		return self:sub(segs, (seps or 0) - 1), cap1 or sep, ...
	end
	return function() if st then return getter(st, g()) end end
end

-- Check string is nil or empty
-- bool
function string:empty(str)
	return str == nil or str == ''
end

-- bool
function string.starts(str, starts)
	if string:empty(str) then return false end
	return string.sub(str,1,string.len(starts))==starts
end

-- bool
function string.ends(str, ends)
	if string:empty(str) then return false end
	return ends=='' or string.sub(str,-string.len(ends))==ends
end

-- Replace string
function string.replace(str, pattern, replacement)
	if string:empty(str) then return str end
	local s, n = string.gsub(str, pattern, replacement)
	return s
end

function string:trim(str)
	if string:empty(str) then return str end
	return (str:gsub("^%s*(.-)%s*$", "%1"))
end



-- Replace char in specified position of string
function replace_char(pos, str, replacement)
	if string:empty(str) then return str end
    return str:sub(1, pos-1) .. replacement .. str:sub(pos+1)
end

-- return count of substr in str
function substr_count(str, substr)
	local _, count = string.gsub(str, substr, "")
	return count
end