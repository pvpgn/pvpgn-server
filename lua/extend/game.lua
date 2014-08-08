--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Get count of all games 
function games_count()
	local count = 0
	for i,game in pairs(api.server_get_games()) do
		count = count + 1
	end
	return count
end

function server_get_games(clienttag)
	local gamelist = {}
	
	for i,game in pairs(api.server_get_games()) do
		if clienttag then
			if (game.tag == clienttag) then
				table.insert(gamelist, game)
			end
		else
			table.insert(gamelist, game)
		end
	end
	return gamelist
end