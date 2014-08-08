--[[
	Simple Anti MapHack for Starcraft: BroodWar 1.16.1

	
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- just unique request id for maphack
ah_mh_request_id = 99
-- map visible offset
ah_mh_offset = 0x0047FD12
-- map visible normal value (without maphack)
ah_mh_value = 139


function ah_init()
	timer_add("ah_timer", config.ah_interval, ah_timer_tick)
	TRACE("Antihack activated")
end

-- send memory check request to all players in games
function ah_timer_tick(options)
	-- iterate all games
	for i,game in pairs(api.server_get_games()) do
		-- check only Starcraft: BroodWar
		if game.clienttag and (game.clienttag == CLIENTTAG_BROODWARS) then
			-- check only games where count of players > 1
			if game.players and (substr_count(game.players, ",") > -1) then
				--DEBUG(game.players)
				-- check every player in the game
				for username in string.split(game.players,",")  do
					api.client_readmemory(username, ah_mh_request_id, ah_mh_offset, 2)
					-- HINT: place here more readmemory requests
					
				end
			end
		end
    end
end

-- handle response from the client
function ah_handle_client(account, request_id, data)
	local is_cheater = false

	-- maphack
	if (request_id == ah_mh_request_id) then
		-- read value from the memory
		local value = bytes_to_int(data, 0, 2)
		DEBUG(account.name .. " memory value: " .. value)
		
		if not (value == ah_mh_value) then
			is_cheater = true
		end

	-- process another hack check
	--elseif (request_id == ...) then
	
	end

	
	if (is_cheater) then
		-- lock cheater account
		account_set_auth_lock(account.name, true)
		account_set_auth_lockreason(account.name, "we do not like cheaters")

		-- notify all players in the game about cheater
		local game = api.game_get_by_id(account.game_id)
		if game then
			for username in string.split(game.players,",")  do
				api.message_send_text(username, message_type_error, nil, account.name .. " was banned by the antihack system.")
			end
		end
		
		-- kick cheater
		api.client_kill(account.name)
		
		INFO(account.name .. " was banned by the antihack system.")
	end
end
