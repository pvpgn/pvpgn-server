--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--

-- Log event wrappers

function FATAL(text)
	api.eventlog(eventlog_level_fatal, __function__, text)
end

function ERROR(text)
	api.eventlog(eventlog_level_error, __function__, text)
end

function WARN(text)
	api.eventlog(eventlog_level_warn, __function__, text)
end

function INFO(text)
	api.eventlog(eventlog_level_info, __function__, text)
end

function DEBUG(text)
	if (type(text) == "table") then 
		text = table.dump(text)
	end
	api.eventlog(eventlog_level_debug, __function__, text)
end

function TRACE(text)
	api.eventlog(eventlog_level_trace, __function__, text)
end

function GUI(text)
	api.eventlog(eventlog_level_gui, __function__, text)
end


