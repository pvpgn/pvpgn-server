--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Loop each second
function handle_server_mainloop()
	-- Tick all timers
	for t in pairs(__timers) do
		__timers[t]:tick()
	end
	
	-- DEBUG(os.time())
end


-- When restart Lua VM
function handle_server_rehash()
	if (config.ghost) then
		gh_unload()
	end

end