--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- /quiz <start|stop|stats>
function command_quiz(account, text)
	if not config.quiz then
		return 1
	end
	
	local args = split_command(text, 2)

	if (args[1] == "start") then
		return q_command_start(account, args[2])
	
	elseif (args[1] == "stop") then
		return q_command_stop(account)

	elseif (args[1] == "stats") then
		if not args[2] then
			return q_command_toplist(account)
		else
			return q_command_stats(account, args[2])
		end
	end
	
	api.describe_command(account.name, args[0])
	return -1
end


-- Start quiz in current channel
function q_command_start(account, filename)
	
	if not account_is_operator_or_admin(account.name) then
		api.message_send_text(account.name, message_type_error, account.name, localize(account.name, "You must be at least a Channel Operator to use this command."))
		return -1
	end
	
	local channel = api.channel_get_by_id(account.channel_id)
	if not channel then
		api.message_send_text(account.name, message_type_error, account.name, localize(account.name, "This command can only be used inside a channel."))
		return -1
	end
 
	if config.quiz_channel then
		api.message_send_text(account.name, message_type_error, account.name, localize(account.name, "Quiz has already ran in channel \"{}\". Use /quiz stop to force finish.", config.quiz_channel))
		return -1
	end
	
	-- check if file exists
	if not filename or not file_exists(q_directory() .. "/questions/" .. filename .. ".txt") then
		api.message_send_text(account.name, message_type_error, account.name, localize(account.name, "Available Quiz dictionaries: "))
		api.message_send_text(account.name, message_type_error, account.name, "   " .. config.quiz_filelist)
		return -1
	end

	quiz:start(channel.name, filename)
	return 0
end

-- Stop quiz
function q_command_stop(account)

	if not account_is_operator_or_admin(account.name) then
		api.message_send_text(account.name, message_type_error, account.name, localize(account.name, "You must be at least a Channel Operator to use this command."))
		return -1
	end
	
	if not config.quiz_channel then
		api.message_send_text(account.name, message_type_error, account.name, localize(account.name, "Quiz is not running."))
		return -1
	end

	quiz:stop(account.name)

	return 0
end

-- Display Quiz Top players record
function q_command_toplist(account)
	
	-- load records (if it was not loaded yet)
	if not q_load_records() then
		return 1
	end

	local output = localize(account.name, "Top {} Quiz records:", config.quiz_users_in_top)
	api.message_send_text(account.name, message_type_info, account.name, output)

	-- display TOP of total records
	for i,t in pairs(q_records_total) do
		if (i > config.quiz_users_in_top) then break end

		local output = string.format("  %d. %s [%d %s]", i, t.username, t.points, localize(account.name, "points"))
		api.message_send_text(account.name, message_type_info, account.name, output)
	end

	return 0
end


-- Display single player's record
function q_command_stats(account, username)

	-- load records (if it was not loaded yet)
	if not q_load_records() then
		return 1
	end
	
	local found = false
	-- find user in records
	for i,t in pairs(q_records_total) do
		if string.upper(t.username) == string.upper(username) then
			api.message_send_text(account.name, message_type_info, account.name, localize(account.name, "{}'s Quiz record:", t.username))
			
			local output = string.format("  %d. %s [%d %s]", i, t.username, t.points, localize(account.name, "points"))
			api.message_send_text(account.name, message_type_info, account.name, output)
			
			found = true
		end
	end
	
	if not found then
		api.message_send_text(account.name, message_type_info, account.name, localize(account.name, "{} has never played Quiz.", username))
	end

	return 0
end
