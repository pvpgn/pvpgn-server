--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- return rounded float value
function math.round(num, idp)
 local mult = 10^(idp or 0)
 return math.floor(num * mult + 0.5) / mult
end
