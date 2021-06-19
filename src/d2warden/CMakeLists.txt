enable_language(ASM_MASM)

add_library(d2warden SHARED
	bnethash.cpp
	bnethash.h
	d2callstub.cpp
	d2callstub.h
	d2ptrs.cpp
	d2ptrs.h
	d2warden.cpp
	global.h
	HashGameSeed.asm
	introtate.h
	md5.h
	md5c.cpp
	MyHashGameSeed.cpp
	RC4.cpp
	RC4.h
	d2warden.def)

target_compile_definitions(d2warden PRIVATE D2_1_13_c)

target_link_options(d2warden PRIVATE /SAFESEH:NO)

set_target_properties(d2warden
	PROPERTIES
		CXX_STANDARD 98
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO)

install(TARGETS d2warden RUNTIME DESTINATION ${SBINDIR})