--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


function gh_load()
	local filename = gh_directory() .. "/users.dmp"

	if not file_exists(filename) then
		return
	end
	
	-- load ghost state
	gh_load_userbot(filename)
	
	DEBUG("Loaded GHost state from " .. filename)
end


function gh_unload()
	-- save ghost state
	local filename = gh_directory() .. "/users.dmp"
	gh_save_userbot(filename)
	
	DEBUG("Saved GHost state to " .. filename)
end