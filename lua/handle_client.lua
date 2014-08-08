--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


function handle_client_readmemory(account, request_id, data)
	
	--TRACE("Read memory request Id: " .. request_id)
		
	-- display memory bytes
	DEBUG(data)
	
	if (config.ah) then
		ah_handle_client(account, request_id, data)
	end

end
