Player vs Player Gaming Network - PRO
=====
![](http://i.imgur.com/LfI3hXo.png)

PvPGN is a free and open source cross-platform server software that supports Battle.net and and Westwood Online game clients. PvPGN-PRO is a fork of the official PvPGN project, whose development stopped in 2011, and aims to provide continued maintenance and additional features for PvPGN.

[![License (GPL version 2)](https://img.shields.io/badge/license-GNU%20GPL%20version%202-blue.svg?style=flat-square)](http://opensource.org/licenses/GPL-2.0)
![Language (C++)](https://img.shields.io/badge/powered_by-C++-brightgreen.svg?style=flat-square)
[![Language (Lua)](https://img.shields.io/badge/powered_by-Lua-red.svg?style=flat-square)](https://lua.org)
[![Github Releases (by Release)](https://img.shields.io/github/downloads/pvpgn/pvpgn-server/1.99.7.2.1/total.svg?maxAge=2592000)]()

[![Compiler (Microsoft Visual C++)](https://img.shields.io/badge/compiled_with-Microsoft%20Visual%20C++-yellow.svg?style=flat-square)](https://msdn.microsoft.com/en-us/vstudio/hh386302.aspx)
[![Compiler (LLVM/Clang)](https://img.shields.io/badge/compiled_with-LLVM/Clang-lightgrey.svg?style=flat-square)](http://clang.llvm.org/)
[![Compiler (GCC)](https://img.shields.io/badge/compiled_with-GCC-yellowgreen.svg?style=flat-square)](https://gcc.gnu.org/)

[![Build Status](https://travis-ci.org/pvpgn/pvpgn-server.svg?branch=master)](https://travis-ci.org/pvpgn/pvpgn-server)
[![Build status](https://ci.appveyor.com/api/projects/status/dqoj9lkvhfwthmn6)](https://ci.appveyor.com/project/HarpyWar/pvpgn)

[Deleaker](http://www.deleaker.com/) helps us find memory leaks.

## Tracking
By default, tracking is enabled and is only used for the purpose of sending informational data (e.g. server description, homepage, uptime, amount of users) to tracking servers. To disable tracking, set ````track = 0```` in ````conf/bnetd.conf````.

## Supported Clients
- **WarCraft 2: Battle.net Edition**: 2.02a, 2.02b
- **WarCraft 3: Reign of Chaos**\*: 1.13a, 1.13b, 1.14a, 1.14b, 1.15a, 1.16a, 1.17a, 1.18a, 1.19a, 1.19b, 1.20a, 1.20b, 1.20c, 1.20d, 1.20e, 1.21a, 1.21b, 1.22a, 1.23a, 1.24a, 1.24b, 1.24c, 1.24d, 1.24e, 1.25b, 1.26a, 1.27a, 1.27b, 1.28, 1.28.1, 1.28.2, 1.28.4, 1.28.5
- **WarCraft 3: The Frozen Throne**\*: 1.13a, 1.13b, 1.14a, 1.14b, 1.15a, 1.16a, 1.17a, 1.18a, 1.19a, 1.19b, 1.20a, 1.20b, 1.20c, 1.20d, 1.20e, 1.21a, 1.21b, 1.22a, 1.23a, 1.24a, 1.24b, 1.24c, 1.24d, 1.24e, 1.25b, 1.26a, 1.27a, 1.27b, 1.28, 1.28.1, 1.28.2, 1.28.4, 1.28.5
- **StarCraft**: 1.08, 1.08b, 1.09, 1.09b, 1.10, 1.11, 1.11b, 1.12, 1.12b, 1.13, 1.13b, 1.13c, 1.13d, 1.13e, 1.13f, 1.14, 1.15, 1.15.1, 1.15.2, 1.15.3, 1.16, 1.16.1, 1.17.0, 1.18.0
- **StarCraft: Brood War**: 1.08, 1.08b, 1.09, 1.09b, 1.10, 1.11, 1.11b, 1.12, 1.12b, 1.13, 1.13b, 1.13c, 1.13d, 1.13e, 1.13f, 1.14, 1.15, 1.15.1, 1.15.2, 1.15.3, 1.16, 1.16.1, 1.17.0, 1.18.0
- **Diablo**: 1.09, 1.09b
- **Diablo 2**: 1.10, 1.11, 1.11b, 1.12a, 1.13c, 1.14a, 1.14b, 1.14c, 1.14d
- **Diablo 2: Lord of Destruction**: 1.10, 1.11, 1.11b, 1.12a, 1.13c, 1.14a, 1.14b, 1.14c, 1.14d
- **Westwood Chat Client**: 4.221
- **Command & Conquer**: Win95 1.04a (using Westwood Chat)
- **Command & Conquer: Red Alert**: Win95 2.00 (using Westwood Chat), Win95 3.03
- **Command & Conquer: Red Alert 2**: 1.006
- **Command & Conquer: Tiberian Sun**: 2.03 ST-10
- **Command & Conquer: Tiberian Sun Firestorm**: 2.03 ST-10
- **Command & Conquer: Yuri's Revenge**: 1.001
- **Command & Conquer: Renegade**: 1.037
- **Nox**: 1.02b
- **Nox Quest**: 1.02b
- **Dune 2000**: 1.06
- **Emperor: Battle for Dune**: 1.09

\* WarCraft 3 clients are unable to connect to PvPGN servers without a client-side modification, through tools such as [W3L](https://github.com/w3lh/w3l), to disable server signature verification.
\* StarCraft clients beginning with patch 1.18 will not be supported by PvPGN-PRO due to protocol changes. A 1.18.0 versioncheck entry is included for compatibility with bot software.

## Support
[Create an issue](https://github.com/pvpgn/pvpgn-server/issues) if you have any questions, suggestions, or anything else to say about PvPGN-PRO. Please note that D2GS is not part of the PvPGN project and is therefore unsupported here.
Set `loglevels = fatal,error,warn,info,debug,trace` in `bnetd.conf` before obtaining logs and posting them.

## Development
Submit pull requests to contribute to this project. Utilize C++11 features and adhere to the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md) whenever possible.

## Building
See [docs/ports.md](https://github.com/pvpgn/pvpgn-server/blob/master/docs/ports.md) for operating systems and compilers that have been confirmed to work with PvPGN. Any operating system that supports WinAPI or POSIX, and any C++11 compliant compiler should be able to build PvPGN. The CMake files have been hardcoded to reject compilers older than Visual Studio 2015 and GCC 5.1.

#### Windows
Use [Magic Builder](https://github.com/pvpgn/pvpgn-magic-builder).

Alternatively, use cmake to generate the .sln project and build it from Visual Studio.
```
cmake -g "Visual Studio 14 2015" -H./ -B./build
```
This will generate .sln in `build` directory.

#### Linux in general
Do not blindly run these commands. The main problem with older distributions is installing CMake 3.2.x and GCC 5, so external repositories are used in the examples.

```
apt-get install git install cmake make build-essential zlib1g-dev
apt-get install liblua5.1-0-dev #Lua support
apt-get install mysql-server mysql-client libmysqlclient-dev #MySQL support
cd /home
git clone https://github.com/pvpgn/pvpgn-server.git
cmake -D CMAKE_INSTALL_PREFIX=/usr/local/pvpgn -D WITH_MYSQL=true -D WITH_LUA=true ../
make
make install
```

#### Ubuntu 16.04, 18.04
```
sudo apt-get -y install build-essential git cmake zlib1g-dev
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server && cmake -G "Unix Makefiles" -H./ -B./build
cd build && make
```

#### Ubuntu 14.04
```
sudo apt-get -y install build-essential zlib1g-dev git
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get -y update
sudo apt-get -y install gcc-5 g++-5
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 60 --slave /usr/bin/g++ g++ /usr/bin/g++-5
sudo add-apt-repository -y ppa:george-edison55/cmake-3.x
sudo apt-get update
sudo apt-get -y install cmake
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server && cmake -G "Unix Makefiles" -H./ -B./build
cd build && make
```

#### Debian 8 with clang compiler
```
sudo apt-get -y install build-essential zlib1g-dev clang libc++-dev git
wget https://cmake.org/files/v3.7/cmake-3.7.1-Linux-x86_64.tar.gz
tar xvfz cmake-3.7.1-Linux-x86_64.tar.gz
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server && CC=/usr/bin/clang CXX=/usr/bin/clang++ ../cmake-3.7.1-Linux-x86_64/bin/cmake -G "Unix Makefiles" -H./ -B./build
cd build && make
```

#### CentOS 7
```
sudo yum -y install epel-release centos-release-scl
sudo yum -y install git zlib-devel cmake3 devtoolset-4-gcc*
sudo ln -s /usr/bin/cmake3 /usr/bin/cmake
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server
CC=/opt/rh/devtoolset-4/root/usr/bin/gcc CXX=/opt/rh/devtoolset-4/root/usr/bin/g++ cmake -G "Unix Makefiles" -H./ -B./build
cd build && make
```

#### Fedora 25
```
sudo dnf -y install gcc-c++ gcc make zlib-devel cmake git
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server
cmake -G "Unix Makefiles" -H./ -B./build
cd build && make
```

#### FreeBSD 11
```
sudo pkg install -y git cmake
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server
cmake -G "Unix Makefiles" -H./ -B./build
cd build && make
```

Full instructions: [Русский](http://harpywar.com/?a=articles&b=2&c=1&d=74) | [English](http://harpywar.com/?a=articles&b=2&c=1&d=74&lang=en)

#### Docker
There are some build arguments supported:
| Build Arg    | Original Option | Default Value |
|--------------|-----------------|---------------|
| with-bnetd   | WITH_BNETD      | true          |
| with-d2cs    | WITH_D2CS       | true          |
| with-d2dbs   | WITH_D2DBS      | true          |
| with-lua     | WITH_LUA        | false         |
| with-mysql   | WITH_MYSQL      | false         |
| with-sqlite3 | WITH_SQLITE3    | false         |
| with-pgsql   | WITH_PGSQL      | false         |
| with-odbc    | WITH_ODBC       | false         |
| march-native | MARCH_NATIVE    | false         |

Some note: If you're planning to build an image and run it somewhere else, you shouldn't enable `march-native` argument. The code might not work as you expected.

References:
* https://stackoverflow.com/questions/52653025/why-is-march-native-used-so-rarely
* https://lemire.me/blog/2018/07/25/it-is-more-complicated-than-i-thought-mtune-march-in-gcc/

How to build and run:

```bash
docker build . -t pvpgn-server --build-arg with-d2cs=false --build-arg with-d2dbs=false --build-arg with-mysql=true
docker run -d -v /your/config/file:/usr/local/etc/pvpgn/config-file.conf pvpgn-server
```

You will need to mount the configuration files to the container. The config files will be located at `/usr/local/etc/pvpgn`.

TODO: Add configurable enviroment variables

## Hosting on LAN or VPS with private IP address
Some VPS providers do not assign your server a direct public IP. If that is the case or you host at home behind NAT you need to setup the route translation in `address_translation.conf`. The public address is pushed as the route server address to game clients when seeking games. Failure to push the correct address to game clients results in players not being able to match and join games (long game search and error).

If your network interface is directly bound to public IP, PvPGN can figure it out on it's own and this step is not necessary.

## License

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
