--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Send announce to all connected users
function message_send_all(text)
	for i,account in pairs(api.server_get_users()) do
		api.message_send_text(account.name, message_type_broadcast, nil, text)
	end
end

function localize(username, arg1, arg2, arg3, arg4, arg5)
	return api.localize(username, arg1, arg2, arg3, arg4, arg5)
end