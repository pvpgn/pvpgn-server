--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--

-- Entry point
-- Executes after preload all the lua files
function main()
	
	if (config.ah) then
		-- start antihack
		ah_init()
	end
	
	if (config.ghost) then
		gh_load()
	end
end
