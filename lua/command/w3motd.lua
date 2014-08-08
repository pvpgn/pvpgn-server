--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


--
-- Read file w3motd.txt line by line and send text to user
--

local username = nil

function command_w3motd(account, text)
	-- allow warcraft 3 client only
	if not (account.clienttag == CLIENTTAG_WAR3XP or account.clienttag == CLIENTTAG_WARCRAFT3) then
		return 1
	end

	username = account.name
	local data = file_load(config.motdw3file, w3motd_sendline_callback)
	
	return 0
end

function w3motd_sendline_callback(line)
	api.message_send_text(username, message_type_info, nil, line)
end
