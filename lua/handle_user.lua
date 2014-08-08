--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


function handle_user_whisper(account_src, account_dst, text)
	--DEBUG(account_src.name.."->"..account_dst.name.. ": ".. text)
	--return 1;
end

function handle_user_login(account)
	if (config.ghost) then
		gh_handle_user_login(account)
	end
	
	--DEBUG(account.name.." logged in")
	--return 1;
end

function handle_user_disconnect(account)
	--DEBUG(account.name.." disconnected")
end

function handle_user_icon(account, iconinfo)
	--TRACE("iconinfo"..iconinfo)
	--return iconinfo
end

