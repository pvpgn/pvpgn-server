--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--

-- Load total records from records.txt to table
function q_load_records()
	local filename = q_directory() .. "/records.txt"
	if not q_records_total or not next(q_records_total) then
		-- fill records table
		return file_load(filename, file_load_dictionary_callback, q_read_records_callback)
	end
	return true
end
function q_read_records_callback(a, b)
	table.insert(q_records_total, { username = a, points = b })
end

-- Save total records from table to records.txt
function q_save_records()
	local filename = q_directory() .. "/records.txt"
	file_save2(filename, q_save_records_callback)
end
function q_save_records_callback(file)
	if (q_records_total) and next(q_records_total) then
		for i,t in pairs(q_records_total) do
			file:write(t.username .. " = " .. t.points)
			file:write("\n")
		end
	end
end


-- Print Top X players records
function quiz_display_top_players()
	
	local output = "Top " .. config.quiz_users_in_top .. " Quiz players:"
	channel_send_message(config.quiz_channel, output, message_type_info)
	
	-- display TOP of total records
	for i,t in pairs(q_records_total) do
		if (i > config.quiz_users_in_top) then break end
		
		local diff = ""
		if q_records_diff[t.username] then
			if q_records_diff[t.username] < 0 then
				diff = "(" .. q_records_diff[t.username] .. ")" -- minus points
			elseif q_records_diff[t.username] > 0 then
				diff = "(+" .. q_records_diff[t.username] .. ")" -- plus points
			end
		end
		
		local output = string.format("  %d. %s [%d points] %s", i, t.username, t.points, diff)
		channel_send_message(config.quiz_channel, output, message_type_info)
	end
end

-- find username and return table index
-- preventnew - do not add user in table if it is not found
function q_records_current_find(username, preventnew)
	-- find username
	for i,t in pairs(q_records_current) do
		if (t.username == username) then return i end
	end
	if preventnew then return nil end
	
	-- if not found then insert
	table.insert(q_records_current, {username = username, points = 0})
	return #q_records_current
end

-- find username and return table index
-- prevent_new = do not add user in table if it is not found
function q_records_total_find(username, prevent_new)
	for i,t in pairs(q_records_total) do
		if (t.username == username) then return i end
	end
	if prevent_new then return nil end

	-- if not found then insert
	table.insert(q_records_total, {username = username, points = 0})
	return #q_records_total
end


