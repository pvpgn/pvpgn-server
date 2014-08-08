--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Read file line by line 
-- callback 2 is optional
-- You can process line with function callback1(line, callback2)
function file_load(filename, callback1, callback2)
	local file = io.open(filename, "r")
	if file then
		for line in file:lines() do
			if callback2 then
				callback1(line, callback2)
			else
				callback1(line)
			end
		end
		file.close(file)
		TRACE("File read " .. filename)
	else
		ERROR("Could not open file " .. filename)
		return false
	end
	return true
end

-- (callback) for "file_load" to load file with each line format like "key = value"
function file_load_dictionary_callback(line, callback)
	if string:empty(line) then return 0 end
	
	local idx = 0
	local a, b
	for v in string.split(line, "=") do
		if idx == 0 then
			a = string:trim(v)
		else
			b = string:trim(v)
		end
		idx = idx + 1
	end
	if not string:empty(b) and not string:empty(a) then
		callback(a, b)
	end
end






-- Save raw text "data" into a filename
function file_save(data, filename)
	local file = io.open(filename, "w")

	file:write(data)
	file:close()
end

--  Save file using callback
function file_save2(filename, callback)
	local file = io.open(filename, "w")
	
	callback(file)
	file:close()
end
-- Check file for exist
function file_exists(filename)
	local f=io.open(filename, "r")
	if f~=nil then io.close(f) return true else return false end
end


