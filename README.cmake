                      PvPGN CMake Build Instructions
                    ===================================



0. Getting "cmake"

   Before anything you need to make sure you have a cmake (at least version
2.4.3) installed on the system you wish to compile PvPGN. On Linux/*BSD systems 
this sould probably be provided by your package system one way or another or 
you could try to build cmake from source. Follow the "Readme.txt" from cmake 
sources if you are going over that path.

   Another source of "cmake" may be the binary packages provided by the 
cmake people on www.cmake.org. This is usually the way to get it on 
Windows or exotic Unices. For BeOS you could get it from bebits.com.

   Please note that "cmake" is not a build system by itself but rather a 
"meta" build system. Parsing the cmake input files, cmake will generate 
the build system files to use with your build system (which may be a 
classic Unix style "make" or an IDE such as Visual Studio). cmake calls 
the part of it that generates the project files a "generator" (and is 
configurable).


1. Running "cmake"

   Now that you have cmake (and also you have the pvpgn sources with the 
cmake support) you should try to run cmake to generate the project files 
(or makefiles, depending on your generator used with cmake). It is 
recommended that you do an "out of source" build, this means you should 
not try to build pvpgn inside the source directory but in a directory 
specially made for building pvpgn.

   Note: the instructions generally follow how to use the cmake command 
line version which is more portable (available everywhere cmake is 
available). For using various cmake GUIs one has to find out himself how 
they work.

   Create a build directory (preferably have it empty), change current 
directory there, run cmake as "cmake /path/to/pvpgn-src" where 
"/path/to/pvpgn-src" is obvioulsy the path to the pvpgn source 
directory. This will have cmake generate the type of project files that 
is default for your platform (which is "Unix Makefiles" on Unix/Linux 
systems or "Visual Studio Project files" on Windows) without any 
additional storage types (ie. no SQL support) and will install pvpgn on 
"make install" in the default location (which is "/usr/local" for Unix 
systems or "<System-Drive>:\\Program Files\\pvpgn" on Windows).

   All these defaults are probably not what you wanted so cmake offers a 
way to customize this by command line flags setting variables or 
selecting the project files generator. Here are some useful options:

   -G "Project Type" : tells cmake to use the "Project Type" project 
files generator; on Unix systems this is by default "Unix Makefiles" and 
on Windows "Visual Studio project files"; other useful values are 
"KDevelop3", "MinGW Makefiles" or "MSYS Makefiles" (the last 2 are 
obviously valid only on Windows)

   -D CMAKE_INSTALL_PREFIX="/path/to" : it will tell cmake to set 
accordingly values so that on "make install" it will install relative to 
"/path/to"

   -D WITH_MYSQL=true : it will tell cmake to check for MySQL 
headers/libraries and if present prepare project files able to compile 
PvPGN with MySQL

   -D WITH_PGSQL=true or -D WITH_SQLITE3=true are similar to WITH_MYSQL 
but for PostgreSQL/SQLite3 storage support

   -D CMAKE_BUILD_TYPE=Debug : build in debug mode (to get meaningfull 
stack traces) 

   WARNING: in between 2 cmake runs make sure you remove all build files 
or at least remove the CMakeCache.txt file otherwise you may experience 
strange problems.

2. Building PvPGN

    After successfully running cmake you are ready to build pvpgn. 
Depending on your project files generator used with cmake you do this in 
various ways. Example: when using "Unix Makefiles" you should just issue 
"make" and it should build your pvpgn binaries. When using some IDE 
project file types such as KDevelop3 or Visual Studio you should run the 
"Build" command of that IDE.

3. Installing PvPGN


   Again, depending on the type of project files generator you used you 
can install pvpgn to the CMAKE_INSTALL_DIR prefixed location. When using 
the "Unix Makefiles" generator you can use "make install" to do so.


4. Full examples usage

4.1 Using cmake on a Unix/Linux system installing pvpgn under "~pvpgn" 
with MySQL support

  $ cd pvpgn-build
  $ cmake -D CMAKE_INSTALL_PREFIX=/home/pvpgn -D WITH_MYSQL ../pvpgn-src
  $ make
  $ make install

4.2 Using cmake on a Windows (command line prompt) system with mingw32
  > cd pvpgn-build
  > cmake ..\pvpgn-src
  > mingw32-make
  > mingw32-make install

5. Problems


   cmake building in PvPGN is a young feature as such many issues may 
still arise. If you have problems with cmake please contact us at 
pvpgn-dev@berlios.de (after you subscribe to the mailing list at 
https://lists.berlios.de/mailman/listinfo/pvpgn-dev) or use the bug 
tracker at http://developer.berlios.de/bugs/?group_id=2291 .

   When reporting problems please provide all information such as pvpgn 
version, your platform name/version, exactly what you tried to do (the 
commands used) and exactly the errors you get.
