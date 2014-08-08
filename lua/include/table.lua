--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Clear table
function table.clear(_table)
	for k in pairs(_table) do
		_table[k] = nil
	end
end

-- Get table size
function table.count(T)
	if not T or not next(t) then  return 0 end
	
	local count = 0
	for _ in pairs(T) do count = count + 1 end
	return count
end

-- Get table slice from offset to length
function table.slice(values, offset, length)
	local res = {}
	local n = #values
	-- default values for range
	offset = offset or 0
	if offset < 0 or offset > n then
		return {}
	end
	length = length or n-offset
	if length < 1 then
		length = 1
	elseif length > n-offset then
		length = n-offset
	end

	for i = offset,offset+length-1 do
		res[i-offset] = values[i]
	end
	return res
end


-- (Debug) Print contents of `tbl`, with indentation.
-- `indent` sets the initial level of indentation.
function table.dump(t, ident)
	local output = ""
	if not indent then indent = 0 end
	
	-- if table has 0 index then display this key at start
	if not (t[0] == nil) then
		for k=0, #t do
			local formatting = "\n" .. string.rep("  ", indent) .. k .. ": "
			local v = t[k]
			if type(v) == "table" then
				v = "[" .. table.print(v, indent+1) .. "]"
			elseif type(v) == 'boolean' then
				v = tostring(v)
			end
			
			output = output .. formatting .. v
		end
	else
		for k,v in pairs(t) do
			local formatting = "\n" .. k .. ": "
			local v = t[k]
			if type(v) == "table" then
				v = "[" .. table.print(v, indent+1) .. "]"
			elseif type(v) == 'boolean' then
				v = tostring(v)
			end
			
			output = output .. formatting .. v
		end
	end
	return output
end

-- Find table key by value
function get_key_for_value( t, value )
	for k,v in pairs(t) do
		if v==value then return k end
	end
	return nil
end

--[[
   Save Table to File
   Load Table from File
   v 1.0
   
   Lua 5.2 compatible
   
   Only Saves Tables, Numbers and Strings
   Insides Table References are saved
   Does not save Userdata, Metatables, Functions and indices of these
   ----------------------------------------------------
   table.save( table , filename )
   
   on failure: returns an error msg
   
   ----------------------------------------------------
   table.load( filename or stringtable )
   
   Loads a table that has been saved via the table.save function
   
   on success: returns a previously saved table
   on failure: returns as second argument an error msg
   ----------------------------------------------------
   
   Licensed under the same terms as Lua itself.
]]--
do
   -- declare local variables
   --// exportstring( string )
   --// returns a "Lua" portable version of the string
   local function exportstring( s )
      return string.format("%q", s)
   end

   --// The Save Function
   function table.save(  tbl,filename )
      local charS,charE = "   ","\n"
      local file,err = io.open( filename, "wb" )
      if err then return err end

      -- initiate variables for save procedure
      local tables,lookup = { tbl },{ [tbl] = 1 }
      file:write( "return {"..charE )

      for idx,t in ipairs( tables ) do
         file:write( "-- Table: {"..idx.."}"..charE )
         file:write( "{"..charE )
         local thandled = {}

         for i,v in ipairs( t ) do
            thandled[i] = true
            local stype = type( v )
            -- only handle value
            if stype == "table" then
               if not lookup[v] then
                  table.insert( tables, v )
                  lookup[v] = #tables
               end
               file:write( charS.."{"..lookup[v].."},"..charE )
            elseif stype == "string" then
               file:write(  charS..exportstring( v )..","..charE )
            elseif stype == "number" then
               file:write(  charS..tostring( v )..","..charE )
            end
         end

         for i,v in pairs( t ) do
            -- escape handled values
            if (not thandled[i]) then
            
               local str = ""
               local stype = type( i )
               -- handle index
               if stype == "table" then
                  if not lookup[i] then
                     table.insert( tables,i )
                     lookup[i] = #tables
                  end
                  str = charS.."[{"..lookup[i].."}]="
               elseif stype == "string" then
                  str = charS.."["..exportstring( i ).."]="
               elseif stype == "number" then
                  str = charS.."["..tostring( i ).."]="
               end
            
               if str ~= "" then
                  stype = type( v )
                  -- handle value
                  if stype == "table" then
                     if not lookup[v] then
                        table.insert( tables,v )
                        lookup[v] = #tables
                     end
                     file:write( str.."{"..lookup[v].."},"..charE )
                  elseif stype == "string" then
                     file:write( str..exportstring( v )..","..charE )
                  elseif stype == "number" then
                     file:write( str..tostring( v )..","..charE )
                  end
               end
            end
         end
         file:write( "},"..charE )
      end
      file:write( "}" )
      file:close()
   end
   
   --// The Load Function
   function table.load( sfile )
      local ftables,err = loadfile( sfile )
      if err or not ftables() then return _,err end
      local tables = ftables()
	  
      for idx = 1,#tables do
         local tolinki = {}
         for i,v in pairs( tables[idx] ) do
            if type( v ) == "table" then
               tables[idx][i] = tables[v[1]]
            end
            if type( i ) == "table" and tables[i[1]] then
               table.insert( tolinki,{ i,tables[i[1]] } )
            end
         end
         -- link indices
         for _,v in ipairs( tolinki ) do
            tables[idx][v[2]],tables[idx][v[1]] =  tables[idx][v[1]],nil
         end
      end
      return tables[1]
   end
-- close do
end

-- ChillCode