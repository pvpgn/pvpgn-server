# Add CLAUDE.md and developer architecture documentation

## Overview

Author a comprehensive `CLAUDE.md` at the repository root to serve as the primary onboarding/reference document for future development sessions, plus one supplementary architecture document in `docs/` that fills the documentation gap (the existing `docs/` folder covers config and porting, but not the codebase layout or runtime model). Update `docs/readme.md` to index the new file.

This is a documentation-only change. No source code, build files, or runtime behavior are modified. There are no automated tests to add; each task ends with a verification step that re-reads the cited source to confirm the doc's claims are accurate.

## Context

- Existing top-level docs: `README.md` (user-facing, build instructions), `README.DEV` (coding style, exceptions, RAII, identifier conventions, namespaces, PVPGN history), `UPDATE` (config-version migration notes), `NEWS`, `version-history.txt`, `CREDITS`
- Existing `docs/` folder: `readme.md` (index), `adbanners.md`, `bnmotd.md`, `compile.g++.md`, `compile.visualstudio2015.md`, `fdwatch.txt`, `ports.md`, `storage.txt`, `versioncheck.md`
- Build system: CMake 3.1+, C++11; options `WITH_BNETD/WITH_D2CS/WITH_D2DBS/WITH_LUA/WITH_MYSQL/WITH_SQLITE3/WITH_PGSQL/WITH_ODBC/WITH_WIN32_GUI`; root `CMakeLists.txt`; `ConfigureChecks.cmake`; `cmake/Modules`; `lib/fmt` vendored
- CI: `.travis.yml` (Linux gcc), `appveyor.yml` (Win VS2019, all storage backends), `.github/workflows/codeql-analysis.yml`
- Source tree (`src/`):
  - `bnetd`: main Battle.net server daemon — entry `main.cpp`, server loop in `server.cpp`, `connection.{cpp,h}`, `account.{cpp,h}`, channel, game, ladder, clan, team, realm, anongame, tournament, prefs, storage (file/sql), `handle_*` per protocol (bnet, bot, telnet, irc, wol, file, udp, d2cs, apireg, init), versioncheck, autoupdate, mail, news, ipban, helpfile, command, command_groups, alias_command, icons, topic, watch, output, message, support, runprog, timer, tick, tracker, userlog, i18n, lua* glue
  - `d2cs`: Diablo 2 character server (separate daemon) — `main.cpp`, `d2gs`, `d2ladder`, `gamequeue`, `server`, queue, `handle_bnetd/d2cs/d2gs`
  - `d2dbs`: Diablo 2 database server (separate daemon)
  - `common`: cross-daemon utilities — fdwatch (select/poll/epoll/kqueue backends), packet, network, eventlog (fmt-based templated logger), conf parser, hashtable, list, queue, xalloc, xstr/xstring, util, addr, `bnet_protocol.h` and other `*_protocol.h` headers, hashing (bnethash/bnetsrp3/wolhash/bigint), pugixml, peerchat, rcm, tag, bn_type, bnettime, give_up_root_privileges
  - `compat`: portability shims for POSIX/Win32 differences (psock, gettimeofday, strsep, strcasecmp, mkdir, mmap, pdir, pgetopt, uname, etc.)
  - `client`: side tools — bnchat, bnstat, bnftp, bnbot, udptest
  - `bniutils`, `bnpass`, `bnproxy`, `bntrackd`: small auxiliary programs (icon utils, password hash, proxy, tracker daemon)
  - `win32`: Windows entrypoints, service, GUI resources, dump
  - `json`: vendored nlohmann/json single header
  - `test`: bigint and bnetsrp3 unit tests (CTest)
- Lua subsystem (when `WITH_LUA=ON`, optional, Lua 5.1): `lua/main.lua` entrypoint, `handle_*.lua` hooks (channel/client/command/game/server/user), `config.lua`, `extend/` (account, channel, eventlog, game, message, account_wrap, enum), `include/` (bitwise, common, convert, file, math, string, table, timer), `command/` (custom slash commands: ping, redirect, stats, w3motd), `antihack/`, `ghost/`, `quiz/`. C++ glue lives in `src/bnetd/lua{interface,functions,objects,wrapper}.{cpp,h}`.
- Configuration: `conf/*.conf.in` templates, generated at CMake configure with paths substituted; `bnetd.conf` is the primary; `storage_path` string selects backend; `conf/i18n/` holds translations
- Coding conventions (from `README.DEV`): C++11, tabs (8-space) for indent, exceptions derived from `std::exception` caught by const-ref, RAII for resources, no static globals (use unnamed namespace), `pvpgn` root namespace with `pvpgn::bnetd / pvpgn::d2cs / pvpgn::d2dbs` sub-namespaces, header guards, includes start with `common/setup_before.h` and end with `common/setup_after.h`, no function bodies in `.h` except templates, fmt-style `{}` formatting in eventlog
- Runtime model: single-threaded event loop driven by fdwatch (select/poll/epoll/kqueue chosen at compile time per platform), connection objects added/removed from fdwatch; tick/timer modules drive periodic work; out-of-memory handling via reserved buffer + `std::set_new_handler`

## Development Approach

- **Testing approach**: Documentation-only — no automated tests apply. Every task ends with a verification step: re-read the cited source/header/config files and confirm every concrete claim (file paths, function names, option flags, namespaces, defaults) matches the code in this snapshot. If a claim cannot be verified, remove or qualify it rather than guessing.
- Complete each task fully before moving to the next.
- Keep both new documents in sync: if `CLAUDE.md` and `docs/architecture.md` cover the same topic, the architecture doc holds the depth and `CLAUDE.md` links to it.
- Do not duplicate large sections that already live in `README.md` / `README.DEV` — link to them.
- No emoji. Plain markdown. Code/path references use backticks.

## Implementation Steps

### Task 1: Inventory pass and outline

**Files:**
- Read: `README.md`, `README.DEV`, `UPDATE`, `NEWS`, `CMakeLists.txt`, `ConfigureChecks.cmake`, `config.h.cmake`, `cmake/Modules/*`, `docs/*`, `src/CMakeLists.txt`, every `src/*/CMakeLists.txt`, `conf/CMakeLists.txt`, `lua/CMakeLists.txt`, `.travis.yml`, `appveyor.yml`, `.github/workflows/codeql-analysis.yml`
- Skim entry points: `src/bnetd/main.cpp`, `src/bnetd/server.cpp` (loop), `src/d2cs/main.cpp`, `src/d2dbs/main.cpp`
- Skim core modules to confirm responsibilities: `src/common/fdwatch.h`, `src/common/eventlog.h`, `src/common/setup_before.h`, `src/common/conf.h`, `src/common/packet.h`, `src/bnetd/connection.h`, `src/bnetd/storage.h`, `src/bnetd/prefs.h`, `src/bnetd/luainterface.h`

Steps:
- [x] Build a short outline (in working notes, not committed) listing every section the two new documents will contain
- [x] Verify the outline against the actual file inventory (no claims about modules that do not exist)

### Task 2: Write docs/architecture.md (developer architecture overview)

**Files:**
- Create: `docs/architecture.md`

Sections:
- [x] Repository layout: top-level directories (`src/`, `conf/`, `lua/`, `docs/`, `cmake/`, `lib/`, `man/`, `scripts/`, `files/`) and what lives in each
- [x] Daemons and binaries produced: bnetd (main), d2cs, d2dbs, bntrackd, bnpass, bniutils, bnproxy, client tools (bnchat, bnstat, bnftp, bnbot, udptest); when each is built and what role it plays
- [x] bnetd module map: group `src/bnetd/*` files by responsibility (entry/loop, connection/session, account/storage, channels/messaging, games/anongame/tournament/ladder, clan/team/friends, realm/d2cs bridge, protocol handlers `handle_*`, configuration/prefs, support files, lua bridge)
- [x] common/ module map: fdwatch and its backends, eventlog (templated fmt API), packet/network, conf parser, hashtable/list/queue, xalloc, hashing primitives (bnethash, bnetsrp3, wolhash, bigint), pugixml, `*_protocol.h` headers
- [x] compat/ purpose and how POSIX/Win32 divergence is handled (psock, gettimeofday, etc.)
- [x] Runtime model: single-threaded fdwatch event loop, conn add/remove lifecycle, tick/timer cadence (`BNETD_POLL_INTERVAL`, `BNETD_JIFFIES`), OOM safety buffer, daemonization (`DO_DAEMONIZE`), Win32 service mode
- [x] Configuration system: conf templates in `conf/*.in` substituted by CMake, runtime parsing via `prefs.cpp`, key files (`bnetd.conf`, `d2cs.conf`, `d2dbs.conf`, `channel.conf`, `realm.conf`, `versioncheck.json`, `ad.json`, `bnmaps.conf`, `command_groups.conf`, `supportfile.conf`, `address_translation.conf`, `sql_DB_layout.conf`, `i18n/`)
- [x] Storage layer: `storage_path` string format, file (plain/cdb) vs sql backends, sql sub-drivers (mysql/pgsql/sqlite3/odbc) gated by `WITH_*` options; reference `docs/storage.txt`
- [x] Protocol handlers: bnet/bot/telnet/irc/wol/wgameres/apireg/file/udp/d2cs/init/w3route — list with port defaults from `setup_before.h`
- [x] Lua scripting: when enabled, file layout under `lua/`, hook entry points (`handle_channel/client/command/game/server/user`), C++ glue files, registered C functions
- [x] Logging: `eventlog(level, __FUNCTION__, "fmt {}", args)` template API, levels (none/trace/debug/info/warn/error/fatal/gui), `loglevels` config
- [x] Build options matrix: `WITH_BNETD/D2CS/D2DBS/LUA/MYSQL/SQLITE3/PGSQL/ODBC/WIN32_GUI` and how they affect the outputs
- [x] Verify: re-read each cited file and confirm every concrete claim (file path, function name, option flag, port number, default constant) matches the source

### Task 3: Write CLAUDE.md (root onboarding doc)

**Files:**
- Create: `CLAUDE.md`

Sections:
- [x] Project summary: one paragraph — what PvPGN-PRO is, what it serves, fork lineage
- [x] Quick orientation: pointer to `README.md` (build/install), `README.DEV` (coding style), `docs/architecture.md` (this PR's new file), `docs/readme.md` (other docs index), `UPDATE`/`NEWS` (release/migration notes)
- [x] Build commands: `cmake -G "Unix Makefiles" -H./ -B./build && cd build && make` for Linux; with options `-DWITH_LUA=ON -DWITH_MYSQL=ON -DWITH_SQLITE3=ON -DWITH_PGSQL=ON -DWITH_ODBC=ON`; Windows path via Magic Builder or `cmake -G "Visual Studio 14 2015"`; `make install` and `make uninstall`/`make purge` targets
- [x] Test commands: enable testing in CMake, then `ctest` from build dir; tests live in `src/test/` (currently bigint, bnetsrp3)
- [x] How to run for dev: `bnetd -f` (foreground), `-c <conf>` to point at a config, valgrind recipe from `README.DEV`
- [x] Codebase map at-a-glance (terse table of `src/` subdirs and their roles, with link to `docs/architecture.md` for depth)
- [x] Coding conventions (summarized from `README.DEV` with link): tabs for indent, RAII, exceptions derived from `std::exception`, const-ref catch, namespace `pvpgn::*`, no static — use unnamed namespace, `common/setup_before.h` first / `common/setup_after.h` last in includes, function definitions split type/name across lines, `getString`/`setString` accessors, no function bodies in headers (except templates), fmt-style `{}` placeholders in eventlog
- [x] Logging: how to log (`eventlog(eventlog_level_info, __FUNCTION__, "msg {}", arg);`), `ERROR0`/`WARN1`/etc. macros, log levels and where the file is written (`logfile` in `bnetd.conf`)
- [x] Configuration workflow when adding a new option: declare in `prefs.cpp`/`prefs.h`, add template in `conf/bnetd.conf.in`, document in `UPDATE`
- [x] Storage backends: short summary plus link to `docs/storage.txt`
- [x] Lua scripting: short summary plus pointers to `lua/` and the C++ bridge (`luainterface.cpp`, `luafunctions.cpp`, `luaobjects.cpp`, `luawrapper.cpp`)
- [x] Adding a new chat command: pointer to `command.cpp`, `command_groups.conf`, `bnalias.conf`, and Lua `handle_command.lua` for script-side commands
- [x] Adding a new protocol handler: pointer to `handle_*.cpp`/`.h` pattern and `handlers.h`
- [x] Common pitfalls / gotchas: header include order (`setup_before.h` must be first), raw `static` is disallowed by the project style, do not put bodies in `.h`, use `xalloc`/`xstrdup` rather than raw `malloc` where existing code does, fdwatch sockets must be removed in `conn_destroy` before close
- [x] CI: Travis (Linux) and AppVeyor (Windows + storage matrix) and CodeQL (cpp) — what each runs
- [x] Where to ask / report: GitHub issues on pvpgn/pvpgn-server (per `README.md`)
- [x] Verify: walk every code/path/option reference and confirm against the snapshot; any item that cannot be verified is removed or rewritten

### Task 4: Index the new architecture doc

**Files:**
- Modify: `docs/readme.md`

- [x] Add a row for `architecture.md` to the Index table with a one-line description
- [x] Verify: render the table mentally, ensure column alignment and that the file linked exists

### Task 5: Final verification pass

- [ ] Re-read `CLAUDE.md` end-to-end and grep the repository for every file path, function name, CMake option, namespace, port number, and default constant it cites; correct any drift
- [ ] Re-read `docs/architecture.md` and do the same grep-and-verify pass
- [ ] Confirm no claims are made about features/files that do not exist in the current snapshot (e.g., do not promise documentation for a system that has not been read)
- [ ] Confirm `CLAUDE.md` is concise enough to be useful as a quick reference (target ~300–500 lines), and that long-form depth lives in `docs/architecture.md`
- [ ] Confirm no edits were made to source code, CMake files, or configuration templates

### Task 6: Move plan to completed

- [ ] Move this plan from `docs/plans/` to `docs/plans/completed/`
