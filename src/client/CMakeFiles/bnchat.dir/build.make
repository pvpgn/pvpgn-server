# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files (x86)\CMake 2.8\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files (x86)\CMake 2.8\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = "C:\Program Files (x86)\CMake 2.8\bin\cmake-gui.exe"

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\git\pvpgn

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\git\pvpgn

# Include any dependencies generated for this target.
include src\client\CMakeFiles\bnchat.dir\depend.make

# Include the progress variables for this target.
include src\client\CMakeFiles\bnchat.dir\progress.make

# Include the compile flags for this target's objects.
include src\client\CMakeFiles\bnchat.dir\flags.make

src\client\CMakeFiles\bnchat.dir\bnchat.obj: src\client\CMakeFiles\bnchat.dir\flags.make
src\client\CMakeFiles\bnchat.dir\bnchat.obj: src\client\bnchat.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/client/CMakeFiles/bnchat.dir/bnchat.obj"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\bnchat.dir\bnchat.obj /FdC:\git\pvpgn\src\client\bnchat.pdb -c C:\git\pvpgn\src\client\bnchat.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\bnchat.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bnchat.dir/bnchat.i"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\bnchat.dir\bnchat.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\client\bnchat.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\bnchat.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bnchat.dir/bnchat.s"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\bnchat.dir\bnchat.s /c C:\git\pvpgn\src\client\bnchat.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\bnchat.obj.requires:
.PHONY : src\client\CMakeFiles\bnchat.dir\bnchat.obj.requires

src\client\CMakeFiles\bnchat.dir\bnchat.obj.provides: src\client\CMakeFiles\bnchat.dir\bnchat.obj.requires
	$(MAKE) -f src\client\CMakeFiles\bnchat.dir\build.make /nologo -$(MAKEFLAGS) src\client\CMakeFiles\bnchat.dir\bnchat.obj.provides.build
.PHONY : src\client\CMakeFiles\bnchat.dir\bnchat.obj.provides

src\client\CMakeFiles\bnchat.dir\bnchat.obj.provides.build: src\client\CMakeFiles\bnchat.dir\bnchat.obj

src\client\CMakeFiles\bnchat.dir\client.obj: src\client\CMakeFiles\bnchat.dir\flags.make
src\client\CMakeFiles\bnchat.dir\client.obj: src\client\client.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/client/CMakeFiles/bnchat.dir/client.obj"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\bnchat.dir\client.obj /FdC:\git\pvpgn\src\client\bnchat.pdb -c C:\git\pvpgn\src\client\client.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\client.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bnchat.dir/client.i"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\bnchat.dir\client.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\client\client.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\client.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bnchat.dir/client.s"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\bnchat.dir\client.s /c C:\git\pvpgn\src\client\client.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\client.obj.requires:
.PHONY : src\client\CMakeFiles\bnchat.dir\client.obj.requires

src\client\CMakeFiles\bnchat.dir\client.obj.provides: src\client\CMakeFiles\bnchat.dir\client.obj.requires
	$(MAKE) -f src\client\CMakeFiles\bnchat.dir\build.make /nologo -$(MAKEFLAGS) src\client\CMakeFiles\bnchat.dir\client.obj.provides.build
.PHONY : src\client\CMakeFiles\bnchat.dir\client.obj.provides

src\client\CMakeFiles\bnchat.dir\client.obj.provides.build: src\client\CMakeFiles\bnchat.dir\client.obj

src\client\CMakeFiles\bnchat.dir\client_connect.obj: src\client\CMakeFiles\bnchat.dir\flags.make
src\client\CMakeFiles\bnchat.dir\client_connect.obj: src\client\client_connect.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/client/CMakeFiles/bnchat.dir/client_connect.obj"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\bnchat.dir\client_connect.obj /FdC:\git\pvpgn\src\client\bnchat.pdb -c C:\git\pvpgn\src\client\client_connect.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\client_connect.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bnchat.dir/client_connect.i"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\bnchat.dir\client_connect.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\client\client_connect.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\client_connect.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bnchat.dir/client_connect.s"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\bnchat.dir\client_connect.s /c C:\git\pvpgn\src\client\client_connect.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\client_connect.obj.requires:
.PHONY : src\client\CMakeFiles\bnchat.dir\client_connect.obj.requires

src\client\CMakeFiles\bnchat.dir\client_connect.obj.provides: src\client\CMakeFiles\bnchat.dir\client_connect.obj.requires
	$(MAKE) -f src\client\CMakeFiles\bnchat.dir\build.make /nologo -$(MAKEFLAGS) src\client\CMakeFiles\bnchat.dir\client_connect.obj.provides.build
.PHONY : src\client\CMakeFiles\bnchat.dir\client_connect.obj.provides

src\client\CMakeFiles\bnchat.dir\client_connect.obj.provides.build: src\client\CMakeFiles\bnchat.dir\client_connect.obj

src\client\CMakeFiles\bnchat.dir\udptest.obj: src\client\CMakeFiles\bnchat.dir\flags.make
src\client\CMakeFiles\bnchat.dir\udptest.obj: src\client\udptest.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/client/CMakeFiles/bnchat.dir/udptest.obj"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\bnchat.dir\udptest.obj /FdC:\git\pvpgn\src\client\bnchat.pdb -c C:\git\pvpgn\src\client\udptest.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\udptest.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bnchat.dir/udptest.i"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\bnchat.dir\udptest.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\client\udptest.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\udptest.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bnchat.dir/udptest.s"
	cd C:\git\pvpgn\src\client
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\bnchat.dir\udptest.s /c C:\git\pvpgn\src\client\udptest.cpp
<<
	cd C:\git\pvpgn

src\client\CMakeFiles\bnchat.dir\udptest.obj.requires:
.PHONY : src\client\CMakeFiles\bnchat.dir\udptest.obj.requires

src\client\CMakeFiles\bnchat.dir\udptest.obj.provides: src\client\CMakeFiles\bnchat.dir\udptest.obj.requires
	$(MAKE) -f src\client\CMakeFiles\bnchat.dir\build.make /nologo -$(MAKEFLAGS) src\client\CMakeFiles\bnchat.dir\udptest.obj.provides.build
.PHONY : src\client\CMakeFiles\bnchat.dir\udptest.obj.provides

src\client\CMakeFiles\bnchat.dir\udptest.obj.provides.build: src\client\CMakeFiles\bnchat.dir\udptest.obj

# Object files for target bnchat
bnchat_OBJECTS = \
"CMakeFiles\bnchat.dir\bnchat.obj" \
"CMakeFiles\bnchat.dir\client.obj" \
"CMakeFiles\bnchat.dir\client_connect.obj" \
"CMakeFiles\bnchat.dir\udptest.obj"

# External object files for target bnchat
bnchat_EXTERNAL_OBJECTS =

src\client\bnchat.exe: src\client\CMakeFiles\bnchat.dir\bnchat.obj
src\client\bnchat.exe: src\client\CMakeFiles\bnchat.dir\client.obj
src\client\bnchat.exe: src\client\CMakeFiles\bnchat.dir\client_connect.obj
src\client\bnchat.exe: src\client\CMakeFiles\bnchat.dir\udptest.obj
src\client\bnchat.exe: src\client\CMakeFiles\bnchat.dir\build.make
src\client\bnchat.exe: src\common\common.lib
src\client\bnchat.exe: src\compat\compat.lib
src\client\bnchat.exe: src\client\CMakeFiles\bnchat.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable bnchat.exe"
	cd C:\git\pvpgn\src\client
	"C:\Program Files (x86)\CMake 2.8\bin\cmake.exe" -E vs_link_exe c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  /nologo @<<
  /DWIN32 /D_WINDOWS /W3 /GR /EHsc /D_DEBUG /MDd /Zi /Ob0 /Od /RTC1 /Febnchat.exe /FdC:\git\pvpgn\src\client\bnchat.pdb @CMakeFiles\bnchat.dir\objects1.rsp /link /implib:bnchat.lib /version:0.0   /machine:X86  /debug /INCREMENTAL /subsystem:console  -LIBPATH:C:\git\pvpgn\src\common  -LIBPATH:C:\git\pvpgn\src\compat  -LIBPATH:C:\git\pvpgn\src\win32  -LIBPATH:C:\git\pvpgn\src\tinycdb  ..\common\common.lib ..\compat\compat.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<
	cd C:\git\pvpgn

# Rule to build all files generated by this target.
src\client\CMakeFiles\bnchat.dir\build: src\client\bnchat.exe
.PHONY : src\client\CMakeFiles\bnchat.dir\build

src\client\CMakeFiles\bnchat.dir\requires: src\client\CMakeFiles\bnchat.dir\bnchat.obj.requires
src\client\CMakeFiles\bnchat.dir\requires: src\client\CMakeFiles\bnchat.dir\client.obj.requires
src\client\CMakeFiles\bnchat.dir\requires: src\client\CMakeFiles\bnchat.dir\client_connect.obj.requires
src\client\CMakeFiles\bnchat.dir\requires: src\client\CMakeFiles\bnchat.dir\udptest.obj.requires
.PHONY : src\client\CMakeFiles\bnchat.dir\requires

src\client\CMakeFiles\bnchat.dir\clean:
	cd C:\git\pvpgn\src\client
	$(CMAKE_COMMAND) -P CMakeFiles\bnchat.dir\cmake_clean.cmake
	cd C:\git\pvpgn
.PHONY : src\client\CMakeFiles\bnchat.dir\clean

src\client\CMakeFiles\bnchat.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\git\pvpgn C:\git\pvpgn\src\client C:\git\pvpgn C:\git\pvpgn\src\client C:\git\pvpgn\src\client\CMakeFiles\bnchat.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : src\client\CMakeFiles\bnchat.dir\depend

