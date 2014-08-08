--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Get count of all online users 
function users_count()
	local count = 0
	for i,account in pairs(api.server_get_users()) do
		count = count + 1
	end
	return count
end

-- Get count of all server account 
function accounts_count()
	local count = 0
	for i,account in pairs(api.server_get_users(true)) do
		count = count + 1
	end
	return count
end

