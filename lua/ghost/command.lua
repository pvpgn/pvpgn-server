--[[
	Connection interface between PvPGN and GHost
	https://github.com/OHSystem/ohsystem/issues/279

	
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--



------------------------------
--- USER -> PVPGN -> GHOST ---
------------------------------

-- /host [mode] [type] [gamename]
function command_host(account, text)
	if not config.ghost or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end

	local args = split_command(text, 3)
	if not args[1] or not args[2] or not args[3] then
		api.describe_command(account.name, args[0])
		return -1
	end
	
	if gh_get_userbot(account.name) then
		local gamename = gh_get_usergame(account.name)
		local game = game_get_by_name(gamename, account.clienttag, game_type_all)
		if next(game) then
			api.message_send_text(account.name, message_type_info, localize("You already host a game \"{}\". Use /unhost to destroy it.", gamename))
			return -1
		else
			-- if game doesn't exist then remove mapped bot for user 
			gh_del_userbot(account.name)
		end
	end
	
	-- get bot by ping
	local botname = gh_select_bot(account.name)

	-- redirect message to bot
	message_send_ghost(botname, string.format("/pvpgn host %s %s %s %s", account.name, args[1], args[2], args[3]) )
	return 0
end


gh_maplist = {}

-- /chost [code] [gamename]
function command_chost(account, text)
	if not config.ghost or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end

	local filename = gh_directory() .. "/maplist.txt"
	-- load maps from the file
	file_load(filename, file_load_dictionary_callback, 
		function(a,b) 
			-- split second value to get name and filename
			local mapname,mapfile = string.split(b,"|")
			table.insert(gh_maplist, { code = a, name = mapname, filename = mapfile })
		end)
	
	local args = split_command(text, 2)
	
	if not args[1] or not args[2] then
		api.describe_command(account.name, args[0])
		
		-- send user each map on a new line
		for i,map in pairs(gh_maplist) do
			api.message_send_text(account.name, message_type_info, string.format("%s = %s", map.code, map.name) )
		end
		return -1
	end
	
	-- find map by code
	local mapfile = nil
	for i,map in pairs(gh_maplist) do
		if (args[1] == map.code) then mapfile = map.filename end
	end
	
	if not mapfile then
		api.message_send_text(account.name, message_type_info, localize(account.name, "Invalid map code.") )
		return -1
	end
	
	
	if gh_get_userbot(account.name) then
		local gamename = gh_get_usergame(account.name)
		local game = game_get_by_name(gamename, account.clienttag, game_type_all)
		if next(game) then
			api.message_send_text(account.name, message_type_info, localize("You already host a game \"{}\". Use /unhost to destroy it.", gamename))
			return -1
		else
			-- if game doesn't exist then remove mapped bot for user 
			gh_del_userbot(account.name)
		end
	end
	
	-- get bot by ping
	local botname = gh_select_bot(account.name)

	-- redirect message to bot
	message_send_ghost(botname, string.format("/pvpgn chost %s %s %s", account.name, mapfile, args[2]) )
	return 0
end

-- /unhost
function command_unhost(account, text)
	if not config.ghost or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end
	
	-- check if user has a mapped bot
	if gh_get_userbot(account.name) then
		api.message_send_text(account.name, message_type_info, localize("You don't host a game."))
		return -1
	end
	
	-- do not allow unhost if the game is started
	local game = game_get_by_id(account.game_id)
	if next(game) and (game.status == game_status_started) then
		return -1
	end

	-- redirect message to bot
	local botname = gh_get_userbot(account.name)
	message_send_ghost(botname, string.format("/pvpgn unhost %s %s %s %s", account.name, args[1], args[2], args[3]) )
	
	-- remove mapped bot anyway to make sure that it was removed 
	-- (even if bot casual shutdown before send callback)
	gh_del_userbot(account.name)
	
	return 0
end

-- /ping
function command_ping(account, text)
	if not config.ghost or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end

	local game = game_get_by_id(account.game_id)
	-- if user not in game
	if not next(game) then return 1 end
	
	-- check if game owner is ghost bot
	if not gh_is_bot(game.owner) then return 1 end
	
	-- redirect message to bot
	message_send_ghost(game.owner, string.format("/pvpgn ping %s", account.name))

	return 0
end

-- /swap [slot1] [slot2]
function command_swap(account, text)
	if not config.ghost or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end
	if not gh_is_owner(account) then return 1 end
	
	local args = split_command(text, 2)
	if not args[1] or not args[2] then
		api.describe_command(account.name, args[0])
		return -1
	end
	
	-- redirect message to bot
	local botname = gh_get_userbot(account.name)
	message_send_ghost(botname, string.format("/pvpgn swap %s %s %s %s", account.name, args[1], args[2]) )
	return 0
end

-- /open|close [slot]
function command_open_close(account, text)
	if not config.ghost or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end
	if not gh_is_owner(account) then return 1 end
	
	local args = split_command(text, 1)
	if not args[1] then
		api.describe_command(account.name, args[0])
		return -1
	end
	
	-- redirect message to bot
	local botname = gh_get_userbot(account.name)
	message_send_ghost(botname, string.format("/pvpgn %s %s %s", args[0], account.name, args[1]) )
	return 0
end

-- /start|abort|pub|priv
function command_start_abort_pub_priv(account, text)
	if not config.ghost or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end
	if not gh_is_owner(account) then return 1 end
	
	local args = split_command(text, 0)
	
	-- redirect message to bot
	local botname = gh_get_userbot(account.name)
	message_send_ghost(botname, string.format("/pvpgn %s %s %s", args[0], account.name) )
	return 0
end





-- /stats
function command_stats(account, text)
	if not config.ghost or not config.ghost_dota_server or not account.clienttag == CLIENTTAG_WAR3XP then return 1 end

	local useracc = account
	local args = split_command(text, 1)
	if args[1] then
		useracc = account_get_by_name(args[1])
		-- if user not found
		if not next(useracc) then
			api.message_send_text(account.name, localize(account, "Invalid user."))
			return -1
		end
	end
	
	local win, loss = localize(account.name, "win"), localize(account.name, "loss")

	local rating5x5 = account_get_dotarating_5x5(useracc.name)
	local rating3x3 = account_get_dotarating_3x3(useracc.name)
	local wins5x5 = account_get_dotawins_5x5(useracc.name)
	local wins3x3 = account_get_dotawins_3x3(useracc.name)
	local loses5x5 = account_get_dotaloses_5x5(useracc.name)
	local loses3x3 = account_get_dotaloses_3x3(useracc.name)
	local streaks5x5 = account_get_dotastreaks_5x5(useracc.name)
	local streaks3x3 = account_get_dotastreaks_3x3(useracc.name)
	local leaves5x5 = account_get_dotaleaves_5x5(useracc.name)
	local leaves3x3 = account_get_dotaleaves_3x3(useracc.name)
	
	local rank5x5 = icon_get_rank(rating5x5, CLIENTTAG_WAR3XP)
	local rank3x3 = icon_get_rank(rating3x3, CLIENTTAG_WAR3XP)

	local leaves = leaves5x5 + leaves3x3
	local leaves_percent = math.round(leaves / ((wins5x5+wins3x3+loses5x5+loses3x3)/100), 1)
	
	local country = useracc.country
	if not country then country = "-" end
	
	local game = game_get_by_id(account.game_id)
	-- user in game
	if next(game) then
			
		local gametype = "5x5"
		local rank = rank5x5
		local rating = rating5x5
		
		-- switch gametype if game name has substr "3x3"
		if string.find(game.name, "3x3") then 
			gametype = "3x3"
			rank = rank3x3
			rating = rating3x3
		end
		
		-- bnproxy stats output format
		api.message_send_text(account.name, message_type_info, string.format("[%s] %s DotA (%s): [%s] %d pts. Leave count: %d (%f%%)", 
			country, useracc.name, gametype,
			rank, rating,
			leaves, leaves_percent))
		
	else -- in chat
		api.message_send_text(account.name, message_type_info, localize(account.name, "[{}] {}'s record:", country, useracc.name))
		api.message_send_text(account.name, message_type_info, localize(account.name, "DotA games ({}): {}-{} [{}] {} pts", "5x5", wins5x5, loses5x5, rank5x5, rating5x5))
		api.message_send_text(account.name, message_type_info, localize(account.name, "DotA games ({}): {}-{} [{}] {} pts", "3x3", wins3x3, loses3x3, rank3x3, rating3x3))
		
		-- display streaks in self user stats
		if not args[1] then
			local streaks5x5_result, streaks3x3_result = win, win
			if (streaks5x5 < 0) then streaks5x5_result = loss end
			if (streaks3x3 < 0) then streaks3x3_result = loss end
			
			api.message_send_text(account.name, message_type_info, localize(account.name, "Current {} streak ({}): {}", "5x5", streaks5x5_result, streaks5x5))
			api.message_send_text(account.name, message_type_info, localize(account.name, "Current {} streak ({}): {}", "3x3", streaks3x3_result, streaks3x3))
		end
		api.message_send_text(account.name, message_type_info, localize(account.name, "Current leave count: {} ({}%)", leaves, leaves_percent))
	end
	
	return 0
end

