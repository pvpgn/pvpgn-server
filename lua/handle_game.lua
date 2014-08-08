--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Global function to handle game create
function handle_game_create(game)
	--for i,j in pairs(game) do
	--	api.message_send_text(game.owner, message_type_info, game.owner, i.." = "..j)
	--end
end




-- Global function to handle user join to game
function handle_game_userjoin(game, account)
	if config.ghost then
		gh_handle_game_userjoin(game, account)
	end
	

end


-- Global function to handle user left from game
function handle_game_userleft(game, account)
	--for username in string.split(str,",")  do
	--	if (account.name ~= username) then
	--		api.message_send_text(username, message_type_whisper, nil, "Bye ".. account.name)
	--	end
    --end
end

-- Global function to handle game end
function handle_game_end(game)
	--api.message_send_text(game.owner, message_type_whisper, nil, "End game")
end

-- Global function to handle game report
function handle_game_report(game)

	--for i,j in pairs(game) do
	--	api.message_send_text("harpywar", message_type_info, game.owner, i.." = "..j)
	--	api.message_send_text(game.owner, message_type_info, game.owner, i.." = "..j)
	--end
	
	--DEBUG(game.last_access)
end



-- Global function to handle game destroy
function handle_game_destroy(game)
	--api.message_send_text(game.owner, message_type_whisper, nil, "Destroy game")
end


-- Global function to handle game status
function handle_game_changestatus(game)
	--api.message_send_text(game.owner, message_type_info, nil, "Change status of the game to ".. game.status)
end


function handle_game_list(account)
	local gamelist = {}
	
	if (config.ghost and config.ghost_dota_server and account.clienttag == CLIENTTAG_WAR3XP) then
		-- get gamelist
		local glist = server_get_games()
		-- and add ping field for each game
		for i,game in pairs(glist) do
			glist[i].ping = 1000 -- initial ping value for all games
			-- if game owner is ghost bot
			if gh_is_bot(game.owner) then
				local pings = account_get_botping(account.name)
				local is_found = false
				for k,v in pairs(pings) do
					-- fetch user ping for current bot
					if (v.bot == game.owner) then
						glist[i].ping = pings[k].ping
						is_found = true
					end
				end
				-- if ping not found for the bot then use ping "0" to move the game on top
				-- (so, after join the game user will receive a ping from the bot)
				if not is_found then
					glist[i].ping = 0
				end
			end
		end
		
		-- sort gamelist by ping ascending
		table.sort(glist, function(a,b) return tonumber(a.ping) < tonumber(b.ping) end)
		
		-- iterate sorted gamelist and fill a final list with a new order
		for i,game in pairs(glist) do
			table.insert(gamelist, game.id)
			table.insert(gamelist, game.name)
		end
		
		return {"id", "name"}, gamelist
	end
end
