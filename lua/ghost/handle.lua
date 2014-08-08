--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


function gh_handle_game_userjoin(game, account)
	-- check that game owner is ghost bot
	if not gh_is_bot(game.owner) then return end
	
	-- send ping silently
	gh_write_silentping(account.name)

	local botaccount = account_get_by_name(game.owner)
	-- send ping to bot (to save result)
	command_ping(botaccount, "/ping")
	
	if (config.ghost_dota_server) then
		-- show stats of each player in the game
		for u in string.split(game.players, ",") do
			-- (except current player)
			if (account.name ~= u) then
				command_stats(botaccount, "/stats " .. u)
			end
		end
	end
end

function gh_handle_user_login(account)
	if gh_is_bot(account.name) then
	-- activate pvpgn mode on ghost side
		gh_message_send(botname, "/pvpgn init");
	end
end