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

You may also find people familiar with PvPGN at these Discord servers:
- **#botdev** at [Discord - BNETDocs](https://discord.com/invite/u87WVeu)
- **#pvpgn-d2gs-support** at [Discord - DIABLO2.ORG](https://discord.com/invite/FkFJNBu)

## Development
Submit pull requests to contribute to this project. Utilize C++11 features and adhere to the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md) whenever possible.

## Building
See [docs/ports.md](https://github.com/pvpgn/pvpgn-server/blob/master/docs/ports.md) for operating systems and compilers that have been confirmed to work with PvPGN. Any operating system that supports WinAPI or POSIX, and any C++11 compliant compiler should be able to build PvPGN.

On Windows, [Magic Builder](https://github.com/pvpgn/pvpgn-magic-builder) is a convenient tool for building PvPGN.

### Obtain packages.

#### Ubuntu / Debian 10
```
apt install -y build-essential git cmake zlib1g-dev libcurl4-openssl-dev
apt install -y liblua5.1-0-dev # Lua support
apt install -y mysql-server mysql-client libmysqlclient-dev # MySQL support
apt install -y sqlite3 libsqlite3-dev # SQLite3 support
apt install -y postgresql libpq-dev # PostgreSQL support
```

#### CentOS 8
```
dnf -y install wget gcc-c++ make git cmake zlib-devel libcurl-devel
dnf -y install readline-devel && wget -c https://www.lua.org/ftp/lua-5.1.5.tar.gz -O - | tar -xz && cd lua-5.1.5 && make linux && make install # Lua 5.1 must be compiled and installed from source. Newer version not supported yet.
dnf -y install mysql-server mysql-devel
dnf -y install sqlite-devel
dnf -y install postgresql
```

#### CentOS 7
```
yum -y install epel-release centos-release-scl
yum -y install git zlib-devel cmake3 libcurl-devel devtoolset-7-gcc*
ln -s /usr/bin/cmake3 /usr/bin/cmake
scl enable devtoolset-7 bash
```

#### Fedora 32
```
dnf -y install gcc-c++ make git cmake zlib-devel libcurl-devel
dnf -y install community-mysql-server
```

#### FreeBSD 12
```
env ASSUME_ALWAYS_YES=yes pkg install git cmake curl
# Lua?
# MySQL?
# SQLite3?
# PostgreSQL?
```

### Generate a makefile or Visual Studio solution

```
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server && mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE="RelWithDebInfo" ../
```

- On Linux, you may want to specify a custom installation directory by passing `-DCMAKE_INSTALL_PREFIX=/usr/local/pvpgn` to CMake.
- Additional CMake options are:
  - `-DWITH_BNETD=true`
  - `-DWITH_D2CS=true`
  - `-DWITH_D2DBS=true`
  - `-DWITH_LUA=false`
  - `-DWITH_WIN32_GUI=true`
  - `-DWITH_MYSQL=false`
  - `-DWITH_SQLITE3=false`
  - `-DWITH_PGSQL=false`
  - `-DWITH_ODBC=false`

### Build and install

#### Linux
```
make && make install
```

#### Windows (Visual Studio)
First open *Developer PowerShell for VS ...* or *Developer Command Prompt for VS ...*
```
msbuild pvpgn.sln -target:ALL_BUILD;INSTALL /p:Configuration="RelWithDebInfo"
```
#### Docker

[Check this link](README_DOCKER.md)

## Hosting on LAN or VPS with private IP address
Some VPS providers do not assign your server a direct public IP. If that is the case or you host at home behind NAT you need to setup the route translation in `address_translation.conf`. The public address is pushed as the route server address to game clients when seeking games. Failure to push the correct address to game clients results in players not being able to match and join games (long game search and error).

If your network interface is directly bound to public IP, PvPGN can figure it out on it's own and this step is not necessary.

## License
PvPGN-PRO is licensed under the GNU GPL v2 (or later). See LICENSE file for more information.

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