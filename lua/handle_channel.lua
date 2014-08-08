--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


function handle_channel_message(channel, account, text, message_type)
	if config.quiz and channel.name == config.quiz_channel then
		quiz_handle_message(account.name, text)
	end

	--DEBUG(text)
	--return 1
end
function handle_channel_userjoin(channel, account)
	--DEBUG(account.name.." joined "..channel.name)
end
function handle_channel_userleft(channel, account)
	--DEBUG(account.name.." left "..channel.name)
end
