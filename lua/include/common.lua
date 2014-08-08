--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Return the script path and line of the CURRENT function where it is executed
__FUNCTION__ = nil
-- Return the script path and line of the PARENT function where it is executed
__function__ = nil

setmetatable(_G, {__index =
   function(t, k)
      if k == '__FUNCTION__' then
		local w = debug.getinfo(2, "S")
		return w.short_src..":"..w.linedefined
      elseif k == '__function__' then
		local w = debug.getinfo(3, "S")
		return w.short_src..":"..w.linedefined
      end
   end
})

