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
include src\compat\CMakeFiles\compat.dir\depend.make

# Include the progress variables for this target.
include src\compat\CMakeFiles\compat.dir\progress.make

# Include the compile flags for this target's objects.
include src\compat\CMakeFiles\compat.dir\flags.make

src\compat\CMakeFiles\compat.dir\gettimeofday.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\gettimeofday.obj: src\compat\gettimeofday.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/gettimeofday.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\gettimeofday.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\gettimeofday.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\gettimeofday.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/gettimeofday.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\gettimeofday.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\gettimeofday.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\gettimeofday.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/gettimeofday.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\gettimeofday.s /c C:\git\pvpgn\src\compat\gettimeofday.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\gettimeofday.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\gettimeofday.obj.requires

src\compat\CMakeFiles\compat.dir\gettimeofday.obj.provides: src\compat\CMakeFiles\compat.dir\gettimeofday.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\gettimeofday.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\gettimeofday.obj.provides

src\compat\CMakeFiles\compat.dir\gettimeofday.obj.provides.build: src\compat\CMakeFiles\compat.dir\gettimeofday.obj

src\compat\CMakeFiles\compat.dir\inet_aton.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\inet_aton.obj: src\compat\inet_aton.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/inet_aton.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\inet_aton.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\inet_aton.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\inet_aton.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/inet_aton.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\inet_aton.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\inet_aton.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\inet_aton.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/inet_aton.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\inet_aton.s /c C:\git\pvpgn\src\compat\inet_aton.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\inet_aton.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\inet_aton.obj.requires

src\compat\CMakeFiles\compat.dir\inet_aton.obj.provides: src\compat\CMakeFiles\compat.dir\inet_aton.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\inet_aton.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\inet_aton.obj.provides

src\compat\CMakeFiles\compat.dir\inet_aton.obj.provides.build: src\compat\CMakeFiles\compat.dir\inet_aton.obj

src\compat\CMakeFiles\compat.dir\inet_ntoa.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\inet_ntoa.obj: src\compat\inet_ntoa.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/inet_ntoa.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\inet_ntoa.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\inet_ntoa.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\inet_ntoa.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/inet_ntoa.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\inet_ntoa.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\inet_ntoa.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\inet_ntoa.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/inet_ntoa.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\inet_ntoa.s /c C:\git\pvpgn\src\compat\inet_ntoa.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.requires

src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.provides: src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.provides

src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.provides.build: src\compat\CMakeFiles\compat.dir\inet_ntoa.obj

src\compat\CMakeFiles\compat.dir\mmap.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\mmap.obj: src\compat\mmap.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/mmap.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\mmap.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\mmap.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\mmap.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/mmap.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\mmap.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\mmap.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\mmap.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/mmap.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\mmap.s /c C:\git\pvpgn\src\compat\mmap.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\mmap.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\mmap.obj.requires

src\compat\CMakeFiles\compat.dir\mmap.obj.provides: src\compat\CMakeFiles\compat.dir\mmap.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\mmap.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\mmap.obj.provides

src\compat\CMakeFiles\compat.dir\mmap.obj.provides.build: src\compat\CMakeFiles\compat.dir\mmap.obj

src\compat\CMakeFiles\compat.dir\pdir.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\pdir.obj: src\compat\pdir.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_5)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/pdir.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\pdir.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\pdir.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\pdir.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/pdir.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\pdir.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\pdir.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\pdir.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/pdir.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\pdir.s /c C:\git\pvpgn\src\compat\pdir.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\pdir.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\pdir.obj.requires

src\compat\CMakeFiles\compat.dir\pdir.obj.provides: src\compat\CMakeFiles\compat.dir\pdir.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\pdir.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\pdir.obj.provides

src\compat\CMakeFiles\compat.dir\pdir.obj.provides.build: src\compat\CMakeFiles\compat.dir\pdir.obj

src\compat\CMakeFiles\compat.dir\pgetopt.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\pgetopt.obj: src\compat\pgetopt.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_6)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/pgetopt.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\pgetopt.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\pgetopt.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\pgetopt.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/pgetopt.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\pgetopt.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\pgetopt.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\pgetopt.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/pgetopt.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\pgetopt.s /c C:\git\pvpgn\src\compat\pgetopt.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\pgetopt.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\pgetopt.obj.requires

src\compat\CMakeFiles\compat.dir\pgetopt.obj.provides: src\compat\CMakeFiles\compat.dir\pgetopt.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\pgetopt.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\pgetopt.obj.provides

src\compat\CMakeFiles\compat.dir\pgetopt.obj.provides.build: src\compat\CMakeFiles\compat.dir\pgetopt.obj

src\compat\CMakeFiles\compat.dir\psock.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\psock.obj: src\compat\psock.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_7)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/psock.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\psock.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\psock.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\psock.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/psock.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\psock.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\psock.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\psock.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/psock.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\psock.s /c C:\git\pvpgn\src\compat\psock.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\psock.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\psock.obj.requires

src\compat\CMakeFiles\compat.dir\psock.obj.provides: src\compat\CMakeFiles\compat.dir\psock.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\psock.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\psock.obj.provides

src\compat\CMakeFiles\compat.dir\psock.obj.provides.build: src\compat\CMakeFiles\compat.dir\psock.obj

src\compat\CMakeFiles\compat.dir\snprintf.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\snprintf.obj: src\compat\snprintf.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_8)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/snprintf.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\snprintf.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\snprintf.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\snprintf.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/snprintf.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\snprintf.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\snprintf.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\snprintf.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/snprintf.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\snprintf.s /c C:\git\pvpgn\src\compat\snprintf.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\snprintf.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\snprintf.obj.requires

src\compat\CMakeFiles\compat.dir\snprintf.obj.provides: src\compat\CMakeFiles\compat.dir\snprintf.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\snprintf.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\snprintf.obj.provides

src\compat\CMakeFiles\compat.dir\snprintf.obj.provides.build: src\compat\CMakeFiles\compat.dir\snprintf.obj

src\compat\CMakeFiles\compat.dir\strcasecmp.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\strcasecmp.obj: src\compat\strcasecmp.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_9)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/strcasecmp.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\strcasecmp.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\strcasecmp.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strcasecmp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/strcasecmp.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\strcasecmp.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\strcasecmp.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strcasecmp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/strcasecmp.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\strcasecmp.s /c C:\git\pvpgn\src\compat\strcasecmp.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strcasecmp.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\strcasecmp.obj.requires

src\compat\CMakeFiles\compat.dir\strcasecmp.obj.provides: src\compat\CMakeFiles\compat.dir\strcasecmp.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\strcasecmp.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\strcasecmp.obj.provides

src\compat\CMakeFiles\compat.dir\strcasecmp.obj.provides.build: src\compat\CMakeFiles\compat.dir\strcasecmp.obj

src\compat\CMakeFiles\compat.dir\strdup.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\strdup.obj: src\compat\strdup.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_10)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/strdup.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\strdup.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\strdup.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strdup.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/strdup.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\strdup.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\strdup.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strdup.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/strdup.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\strdup.s /c C:\git\pvpgn\src\compat\strdup.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strdup.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\strdup.obj.requires

src\compat\CMakeFiles\compat.dir\strdup.obj.provides: src\compat\CMakeFiles\compat.dir\strdup.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\strdup.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\strdup.obj.provides

src\compat\CMakeFiles\compat.dir\strdup.obj.provides.build: src\compat\CMakeFiles\compat.dir\strdup.obj

src\compat\CMakeFiles\compat.dir\strerror.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\strerror.obj: src\compat\strerror.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_11)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/strerror.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\strerror.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\strerror.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strerror.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/strerror.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\strerror.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\strerror.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strerror.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/strerror.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\strerror.s /c C:\git\pvpgn\src\compat\strerror.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strerror.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\strerror.obj.requires

src\compat\CMakeFiles\compat.dir\strerror.obj.provides: src\compat\CMakeFiles\compat.dir\strerror.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\strerror.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\strerror.obj.provides

src\compat\CMakeFiles\compat.dir\strerror.obj.provides.build: src\compat\CMakeFiles\compat.dir\strerror.obj

src\compat\CMakeFiles\compat.dir\strncasecmp.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\strncasecmp.obj: src\compat\strncasecmp.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_12)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/strncasecmp.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\strncasecmp.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\strncasecmp.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strncasecmp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/strncasecmp.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\strncasecmp.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\strncasecmp.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strncasecmp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/strncasecmp.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\strncasecmp.s /c C:\git\pvpgn\src\compat\strncasecmp.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strncasecmp.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\strncasecmp.obj.requires

src\compat\CMakeFiles\compat.dir\strncasecmp.obj.provides: src\compat\CMakeFiles\compat.dir\strncasecmp.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\strncasecmp.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\strncasecmp.obj.provides

src\compat\CMakeFiles\compat.dir\strncasecmp.obj.provides.build: src\compat\CMakeFiles\compat.dir\strncasecmp.obj

src\compat\CMakeFiles\compat.dir\strsep.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\strsep.obj: src\compat\strsep.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_13)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/strsep.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\strsep.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\strsep.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strsep.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/strsep.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\strsep.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\strsep.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strsep.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/strsep.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\strsep.s /c C:\git\pvpgn\src\compat\strsep.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strsep.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\strsep.obj.requires

src\compat\CMakeFiles\compat.dir\strsep.obj.provides: src\compat\CMakeFiles\compat.dir\strsep.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\strsep.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\strsep.obj.provides

src\compat\CMakeFiles\compat.dir\strsep.obj.provides.build: src\compat\CMakeFiles\compat.dir\strsep.obj

src\compat\CMakeFiles\compat.dir\strtoul.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\strtoul.obj: src\compat\strtoul.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_14)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/strtoul.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\strtoul.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\strtoul.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strtoul.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/strtoul.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\strtoul.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\strtoul.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strtoul.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/strtoul.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\strtoul.s /c C:\git\pvpgn\src\compat\strtoul.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\strtoul.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\strtoul.obj.requires

src\compat\CMakeFiles\compat.dir\strtoul.obj.provides: src\compat\CMakeFiles\compat.dir\strtoul.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\strtoul.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\strtoul.obj.provides

src\compat\CMakeFiles\compat.dir\strtoul.obj.provides.build: src\compat\CMakeFiles\compat.dir\strtoul.obj

src\compat\CMakeFiles\compat.dir\uname.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\uname.obj: src\compat\uname.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_15)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/uname.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\uname.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\uname.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\uname.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/uname.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\uname.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\uname.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\uname.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/uname.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\uname.s /c C:\git\pvpgn\src\compat\uname.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\uname.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\uname.obj.requires

src\compat\CMakeFiles\compat.dir\uname.obj.provides: src\compat\CMakeFiles\compat.dir\uname.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\uname.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\uname.obj.provides

src\compat\CMakeFiles\compat.dir\uname.obj.provides.build: src\compat\CMakeFiles\compat.dir\uname.obj

src\compat\CMakeFiles\compat.dir\vsnprintf.obj: src\compat\CMakeFiles\compat.dir\flags.make
src\compat\CMakeFiles\compat.dir\vsnprintf.obj: src\compat\vsnprintf.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\git\pvpgn\CMakeFiles $(CMAKE_PROGRESS_16)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/compat/CMakeFiles/compat.dir/vsnprintf.obj"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\compat.dir\vsnprintf.obj /FdC:\git\pvpgn\src\compat\compat.pdb -c C:\git\pvpgn\src\compat\vsnprintf.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\vsnprintf.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/compat.dir/vsnprintf.i"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  > CMakeFiles\compat.dir\vsnprintf.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E C:\git\pvpgn\src\compat\vsnprintf.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\vsnprintf.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/compat.dir/vsnprintf.s"
	cd C:\git\pvpgn\src\compat
	c:\PROGRA~2\MICROS~3.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\compat.dir\vsnprintf.s /c C:\git\pvpgn\src\compat\vsnprintf.cpp
<<
	cd C:\git\pvpgn

src\compat\CMakeFiles\compat.dir\vsnprintf.obj.requires:
.PHONY : src\compat\CMakeFiles\compat.dir\vsnprintf.obj.requires

src\compat\CMakeFiles\compat.dir\vsnprintf.obj.provides: src\compat\CMakeFiles\compat.dir\vsnprintf.obj.requires
	$(MAKE) -f src\compat\CMakeFiles\compat.dir\build.make /nologo -$(MAKEFLAGS) src\compat\CMakeFiles\compat.dir\vsnprintf.obj.provides.build
.PHONY : src\compat\CMakeFiles\compat.dir\vsnprintf.obj.provides

src\compat\CMakeFiles\compat.dir\vsnprintf.obj.provides.build: src\compat\CMakeFiles\compat.dir\vsnprintf.obj

# Object files for target compat
compat_OBJECTS = \
"CMakeFiles\compat.dir\gettimeofday.obj" \
"CMakeFiles\compat.dir\inet_aton.obj" \
"CMakeFiles\compat.dir\inet_ntoa.obj" \
"CMakeFiles\compat.dir\mmap.obj" \
"CMakeFiles\compat.dir\pdir.obj" \
"CMakeFiles\compat.dir\pgetopt.obj" \
"CMakeFiles\compat.dir\psock.obj" \
"CMakeFiles\compat.dir\snprintf.obj" \
"CMakeFiles\compat.dir\strcasecmp.obj" \
"CMakeFiles\compat.dir\strdup.obj" \
"CMakeFiles\compat.dir\strerror.obj" \
"CMakeFiles\compat.dir\strncasecmp.obj" \
"CMakeFiles\compat.dir\strsep.obj" \
"CMakeFiles\compat.dir\strtoul.obj" \
"CMakeFiles\compat.dir\uname.obj" \
"CMakeFiles\compat.dir\vsnprintf.obj"

# External object files for target compat
compat_EXTERNAL_OBJECTS =

src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\gettimeofday.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\inet_aton.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\inet_ntoa.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\mmap.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\pdir.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\pgetopt.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\psock.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\snprintf.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\strcasecmp.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\strdup.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\strerror.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\strncasecmp.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\strsep.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\strtoul.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\uname.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\vsnprintf.obj
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\build.make
src\compat\compat.lib: src\compat\CMakeFiles\compat.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library compat.lib"
	cd C:\git\pvpgn\src\compat
	$(CMAKE_COMMAND) -P CMakeFiles\compat.dir\cmake_clean_target.cmake
	cd C:\git\pvpgn
	cd C:\git\pvpgn\src\compat
	c:\MinGW\bin\ld.exe /lib /nologo  /out:compat.lib @CMakeFiles\compat.dir\objects1.rsp 
	cd C:\git\pvpgn

# Rule to build all files generated by this target.
src\compat\CMakeFiles\compat.dir\build: src\compat\compat.lib
.PHONY : src\compat\CMakeFiles\compat.dir\build

src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\gettimeofday.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\inet_aton.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\inet_ntoa.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\mmap.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\pdir.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\pgetopt.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\psock.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\snprintf.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\strcasecmp.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\strdup.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\strerror.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\strncasecmp.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\strsep.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\strtoul.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\uname.obj.requires
src\compat\CMakeFiles\compat.dir\requires: src\compat\CMakeFiles\compat.dir\vsnprintf.obj.requires
.PHONY : src\compat\CMakeFiles\compat.dir\requires

src\compat\CMakeFiles\compat.dir\clean:
	cd C:\git\pvpgn\src\compat
	$(CMAKE_COMMAND) -P CMakeFiles\compat.dir\cmake_clean.cmake
	cd C:\git\pvpgn
.PHONY : src\compat\CMakeFiles\compat.dir\clean

src\compat\CMakeFiles\compat.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\git\pvpgn C:\git\pvpgn\src\compat C:\git\pvpgn C:\git\pvpgn\src\compat C:\git\pvpgn\src\compat\CMakeFiles\compat.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : src\compat\CMakeFiles\compat.dir\depend

