--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Global table with timers
__timers = {}

function timer_add(id, interval, callback)
	timer_object = timer:new(id, interval, callback)
	table.insert(__timers, timer_object)
end

function timer_del(id)
	-- safe remove from the timers (we can not just use __timers = nil here)
	local i = 0
	for k,v in pairs(__timers) do
		i = i + 1
		if (v.id == id) then
			table.remove(__timers, i)
			return true
		end
	end
	return false
end

function timer_get(id)
	local i = 0
	for k,v in pairs(__timers) do
		i = i + 1
		if (v.id == id) then
			return v
		end
	end
end


--
-- Timer class
--
timer = {}

-- Create a new timer with unique id and given interval
function timer:new(id, interval, callback)
	options = { id = id,  interval = interval, prev_time = 0, callback = callback }
	self.__index = self
	return setmetatable(options, self)
end

-- Event when timer executes
function timer:tick()
	if os.time() < self.prev_time + self.interval then return 0 end
	self.prev_time = os.time()
	
	-- Debug: display time when the timer ticks
	-- TRACE(self.interval .. ": " .. os.time())
	
	-- execute callback function
	return self.callback(self)
end


