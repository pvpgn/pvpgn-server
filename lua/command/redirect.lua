--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Send text to user from server. Works like /announce, 
--   but directly to user and message text is not red.
-- /redirect <username> <message>
function command_redirect(account, text)

	local args = split_command(text, 2)
	
	if not args[1] or not args[2] then
		api.describe_command(account.name, args[0])
		return -1
	end
	
	-- get destination account
	local dest = api.account_get_by_name(args[1])

	if next(dest) == nil or dest.online == "false" then
		api.message_send_text(account.name, message_type_error, account.name, localize(account.name, "User \"{}\" is offline", args[1]))
		return -1
	end
	
	api.message_send_text(dest.name, message_type_info, dest.name, args[2])
	
	return 0
end
