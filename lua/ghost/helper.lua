--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- table key=value (user=bot)
-- it fills first when user uses a command
gh_user2bot = {}

-- array with users who will not receive a response from /ping
silentpings = {}

-- return a user mapped bot
-- (nil if bot not mapped)
function gh_get_userbot(username)
	if not next(user2bot) or not user2bot[username] then return nil end
	return user2bot[username].bot
end
function gh_get_usergame(username)
	if not next(user2bot) or not user2bot[username] then return nil end
	return user2bot[username].game
end
function gh_set_userbot(username, botname, gamename)
	user2bot[username] = { bot = botname, game = gamename }
end
function gh_del_userbot(username)
	user2bot[username] = nil
end
-- save table to file
function gh_save_userbot(filename)
	-- do not save empty table
	if not next(gh_user2bot) then return end
	
	table.save(gh_user2bot, filename)
end
-- load table from file
function gh_load_userbot(filename)
	gh_user2bot = table.load(filename)
end




-- add user into a silenttable
function gh_write_silentping(username)
	table.insert(silentpings, account.name)
end
-- return true if user in a silenttable, and remove it
function gh_read_silentping(username)
	for k,v in pairs(silentpings) do
		if v == username then
			table.remove(silentpings, k)
			return true
		end
	end
	return false
end


-- Get path to GHost directory
function gh_directory()
	return config.scriptdir .. "/ghost"
end



-- is user owner of bot in current game
function gh_is_owner(account)
	-- 1) user in game
	if not account.game_id then return false end

	local game = game_get_by_id(account.game_id)
	-- 2) user owner of the bot in current game
	if not game.owner == gh_get_userbot(account.name) then return false end
	
	return true
end




-- is bot in authorized config list?
function gh_is_bot(username)
	for i,bot in pairs(config.ghost_bots) do
		if (bot == username) then return true end
	end
	return false
end

	
-- redirect command from user to bot through PvPGN
function gh_message_send(botname, text)

	-- send message from the server to ghost
	api.message_send_text(nil, message_type_whisper, botname, text)
end

-- return bot name (the best for user by ping)
function gh_select_bot(username)
	local pings = account_get_ping2bot(username)
	local botname = nil
	
	-- iterate all bots in config
	for i,bot in pairs(config.ghost_bots) do
		local is_found = false
		for k,v in pairs(pings) do
			if bot == v.bot then is_found = true end
		end
		-- if user has has no ping for the bot yet - use it
		if not is_found then
			botname = bot
		end
	end
	
	-- if user has all bots pings from config
	if not botname then
		-- select the best bot by ping (the table pings already sorted by ping)
		botname = pings[1].bot
	end
	
	return botname
end


