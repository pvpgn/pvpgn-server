--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


--
-- Bitwise functions:
--   math_not, math_and, math_or, math_xor
--

local function nand(x,y,z)
    z=z or 2^16
    if z<2 then
        return 1-x*y
    else
        return nand((x-x%z)/z,(y-y%z)/z,math.sqrt(z))*z+nand(x%z,y%z,math.sqrt(z))
    end
end
function math_not(y,z)
    return nand(nand(0,0,z),y,z)
end
function math_and(x,y,z)
    return nand(math_not(0,z),nand(x,y,z),z)
end
function math_or(x,y,z)
    return nand(math_not(x,z),math_not(y,z),z)
end
function math_xor(x,y,z)
    return math_and(nand(x,y,z),math_or(x,y,z),z)
end