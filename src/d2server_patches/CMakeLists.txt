enable_language(ASM_MASM)

add_executable(1
	1.asm
	UberQuest.inc
	Warden.inc)

set_property(TARGET 1 PROPERTY PROJECT_LABEL "d2server_patches")

set(CMAKE_ASM_MASM_FLAGS "/Cp /W3" ${CMAKE_ASM_MASM_FLAGS})

target_link_options(1 PRIVATE /INCREMENTAL:NO /DYNAMICBASE:NO /BASE:0x68020000 /FIXED /MANIFEST:NO /SAFESEH:NO /SUBSYSTEM:Console)