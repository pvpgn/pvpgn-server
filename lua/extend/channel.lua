--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Return channel id by name (if channel not found then return -1)
function channel_get_id_by_name(channel_name)
	for i,channel in pairs(api.server_get_channels()) do
		if channel.name == channel_name then
			return channel.id
		end
	end
	return -1
end


-- Send message in channel
--  message_type: message_type_info | message_type_error
function channel_send_message(channel_name, text, message_type)
	channel_id = channel_get_id_by_name(channel_name)
	if (channel_id == -1) then
		return nil
	end
	
	channel = api.channel_get_by_id(channel_id)
	
	for username in string.split(channel.memberlist,",")  do
		api.message_send_text(username, message_type, nil, text)
    end
end


-- Get count of all channels
function channels_count()
	local count = 0
	for i,channel in pairs(api.server_get_channels()) do
		count = count + 1
	end
	return count
end