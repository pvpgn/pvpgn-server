--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- https://github.com/HarpyWar/pvpgn/issues/15
-- http://msdn.microsoft.com/en-us/library/windows/desktop/ms645505(v=vs.85).aspx
--
-- use math_or(mb_type1, mb_type2) to combine two messagebox types
--[[ Example code to display Windows MessageBox:
	local mb_type = math_or(MB_DEFBUTTON2, math_or(MB_ABORTRETRYIGNORE, MB_ICONEXCLAMATION) )
	api.messagebox_show(account.name, "aaaaaaaaaaaaaaaaaaaaaaaaawwadwwadawdawdawdwadawdaaw", "MessageBox from " .. config.servername, mb_type)
]]--

MB_ABORTRETRYIGNORE,
MB_CANCELTRYCONTINUE,
MB_HELP,
MB_OK,
MB_OKCANCEL,
MB_RETRYCANCEL,
MB_YESNO,
MB_YESNOCANCEL,

MB_ICONEXCLAMATION,
MB_ICONWARNING,
MB_ICONINFORMATION,
MB_ICONASTERISK,
MB_ICONQUESTION,
MB_ICONSTOP,
MB_ICONERROR,
MB_ICONHAND,

MB_DEFBUTTON1,
MB_DEFBUTTON2,
MB_DEFBUTTON3,
MB_DEFBUTTON4,

MB_APPLMODAL,
MB_SYSTEMMODAL,
MB_TASKMODAL,

MB_DEFAULT_DESKTOP_ONLY,
MB_RIGHT,
MB_RTLREADING,
MB_SETFOREGROUND,
MB_TOPMOST,
MB_SERVICE_NOTIFICATION
= 0x00000002,0x00000006,0x00004000,0x00000000,0x00000001,0x00000005,0x00000004,0x00000003,0x00000030,0x00000030,0x00000040,0x00000040,0x00000020,0x00000010,0x00000010,0x00000010,0x00000000,0x00000100,0x00000200,0x00000300,0x00000000,0x00001000,0x00002000,0x00020000,0x00080000,0x00100000,0x00010000,0x00040000,0x00200000
