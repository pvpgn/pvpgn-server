--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Get path to Quiz directory
function q_directory()
	return config.scriptdir .. "/quiz"
end

-- Replace each symbol in string with *
-- Example: input = "hello world", return = "***** *****"
function q_hide_unswer(input)
	local output = input
	for i = 1, #input do
		local c = input:sub(i,i)
		if not (c == " ") then
			 output = replace_char(i, output, "*")
		end
	end
	return output
end

-- Open one random symbol in hidden (with *) string from original string
-- Example: hidden = "***** *****", original = "hello world", return = "***l* *****"
function q_show_next_symbol(hidden, original)
	local output = hidden
	
	if hidden == original then
        return output
	end

	local replaced = false
	while not replaced do
		local i = math.random(#hidden)
		local c = hidden:sub(i,i)
		if c == "*" then
			local c2 = original:sub(i,i)
			output = replace_char(i, hidden, c2)
			replaced = true
		end
	end
	return output
end



-- Callback to sort records table descending
function q_compare_desc(a,b)
	return tonumber(a.points) > tonumber(b.points)
end
