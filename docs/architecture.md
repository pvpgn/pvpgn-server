# PvPGN-PRO Architecture

A developer-oriented overview of the PvPGN-PRO source tree, the daemons it
produces, the runtime model, the configuration system, the storage layer, the
protocol handlers, the optional Lua scripting subsystem, and the build-time
options that select between them. For build/install instructions see
`README.md`; for coding style and conventions see `README.DEV`.

## Repository layout

Top-level directories:

- `src/` -- C++ source for every binary plus shared `common`/`compat` static
  libraries and the bundled `nlohmann/json` single header.
- `conf/` -- configuration templates (`*.conf.in`, `*.json.in`, `*.txt.in`)
  processed by CMake's `configure_file` to substitute install paths, plus the
  `i18n/` translation directory.
- `lua/` -- Lua scripts that ship with the server (only installed when
  `WITH_LUA=ON` is configured); contains the entry point, hook files, helper
  modules, and bundled subsystems (antihack, ghost, quiz).
- `docs/` -- developer/operator documentation. `readme.md` is the index.
- `cmake/Modules/` -- custom CMake modules: `FindMySQL.cmake`,
  `FindODBC.cmake`, `FindPostgreSQL.cmake`, `FindSQLite3.cmake`,
  `CheckMkdirArgs.cmake`, `DefineInstallationPaths.cmake`, plus the templates
  for the `uninstall` and `purge` targets.
- `lib/fmt/` -- vendored copy of the `{fmt}` library, used by the templated
  `eventlog()` formatter and elsewhere.
- `man/` -- nroff manpages for the binaries.
- `scripts/` -- helper scripts (init.d/launch wrappers, log rotation, ladder
  conversion, password hash generators, etc.) installed alongside the server.
- `files/` -- runtime support files shipped with the server: BNI icon
  archives, `bnserver*.ini` overrides, autoupdate MPQs, ad smacker/png
  banners, and the `newbie.save` Diablo save template.
- Root files: `CMakeLists.txt`, `ConfigureChecks.cmake`, `config.h.cmake`,
  `README.md`, `README.DEV`, `UPDATE`, `NEWS`, `version-history.txt`,
  `CREDITS`, `LICENSE`, plus the CI configs `.travis.yml`, `appveyor.yml`,
  and `.github/workflows/codeql-analysis.yml`.

## Daemons and binaries

CMake builds the following targets when their guards are satisfied (see
`src/CMakeLists.txt`):

| Target | Built when | Role |
| --- | --- | --- |
| `bnetd` | `WITH_BNETD=ON` (default) | Main Battle.net server daemon. Implements chat, channels, games, ladders, clans, teams, autoupdate, news, ad banners, IRC/WOL gateways, etc. Sources live in `src/bnetd/`. |
| `d2cs` | `WITH_D2CS=ON` (default) | Diablo 2 character server. Validates and forwards character data between bnetd and the Diablo 2 game servers. Sources in `src/d2cs/`. |
| `d2dbs` | `WITH_D2DBS=ON` (default) | Diablo 2 database server. Stores closed-realm character files written through d2cs. Sources in `src/d2dbs/`. |
| `bntrackd` | always | Standalone tracking daemon that aggregates server status pings and writes a public listing. Sources in `src/bntrackd/`. |
| `bnpass` | always | CLI utility that prints the bnet/SHA1 password hash for a cleartext password. Sources in `src/bnpass/`. |
| `sha1hash` | always | Companion CLI to `bnpass` that emits a raw SHA-1 hash. Built from `src/bnpass/`. |
| `bnilist`, `bni2tga`, `bniextract`, `bnibuild`, `tgainfo` | always | Tools for inspecting and assembling Battle.net BNI icon archives. Sources in `src/bniutils/`. |
| `bnchat`, `bnftp`, `bnbot`, `bnstat` | always | Reference client utilities (chat client, file fetch, bot login, account stats query). Sources in `src/client/`. |
| Test executables `bnetsrp3_test`, `bigint` | when CTest is enabled | Unit tests for the SRP3 hash and arbitrary-precision integer code. See `src/test/CMakeLists.txt`. |

`src/bnproxy/` contains historical TCP proxy sources (`bnproxy.c`,
`virtconn.c`) but has no `CMakeLists.txt`, so the CMake build does not produce
a `bnproxy` binary in this snapshot. A `bnproxy.1` manpage still ships in
`man/`.

## bnetd module map

`src/bnetd/` is the largest subtree. Files group naturally by responsibility
(see `src/bnetd/CMakeLists.txt` for the canonical list):

- Entry, server loop, command-line: `main.cpp`, `server.cpp`/`server.h`,
  `cmdline.cpp`/`cmdline.h`.
- Connection / session: `connection.cpp`/`connection.h` (per-socket state
  with classes such as `conn_class_bnet`, `conn_class_irc`,
  `conn_class_wol`, `conn_class_telnet`, `conn_class_d2cs_bnetd`,
  `conn_class_w3route`, etc.), `quota.h`, `tick.cpp`/`tick.h`,
  `timer.cpp`/`timer.h`, `watch.cpp`/`watch.h`.
- Account/storage glue: `account.cpp`/`account.h`,
  `account_wrap.cpp`/`account_wrap.h`, `attr.h`,
  `attrgroup.cpp`/`attrgroup.h`, `attrlayer.cpp`/`attrlayer.h`,
  `storage.cpp`/`storage.h`, `storage_file.cpp`/`storage_file.h`,
  `file.cpp`/`file.h`, `file_plain.cpp`/`file_plain.h`,
  `storage_sql.cpp`/`storage_sql.h`, `sql_common.cpp`/`sql_common.h`,
  `sql_dbcreator.cpp`/`sql_dbcreator.h`, plus the per-driver back ends
  `sql_mysql.{cpp,h}`, `sql_pgsql.{cpp,h}`, `sql_sqlite3.{cpp,h}`,
  `sql_odbc.{cpp,h}`.
- Channels and messaging: `channel.cpp`/`channel.h`,
  `channel_conv.cpp`/`channel_conv.h`, `message.cpp`/`message.h`,
  `topic.cpp`/`topic.h`, `command.cpp`/`command.h`,
  `command_groups.cpp`/`command_groups.h`,
  `alias_command.cpp`/`alias_command.h`, `helpfile.cpp`/`helpfile.h`,
  `news.cpp`/`news.h`, `mail.cpp`/`mail.h`,
  `userlog.cpp`/`userlog.h`, `output.cpp`/`output.h`.
- Games / ranking: `game.cpp`/`game.h`,
  `game_conv.cpp`/`game_conv.h`, `anongame.cpp`/`anongame.h`,
  `anongame_gameresult.{cpp,h}`, `anongame_infos.{cpp,h}`,
  `anongame_maplists.{cpp,h}`, `anongame_wol.{cpp,h}`,
  `ladder.cpp`/`ladder.h`, `ladder_calc.cpp`/`ladder_calc.h`,
  `tournament.cpp`/`tournament.h`, `character.cpp`/`character.h`.
- Clans / teams / friends: `clan.cpp`/`clan.h`,
  `team.cpp`/`team.h`, `friends.cpp`/`friends.h`.
- Realm / d2cs bridge: `realm.cpp`/`realm.h`, `handle_d2cs.{cpp,h}`.
- Protocol handlers (`handle_*`): see the dedicated section below.
- Configuration and prefs: `prefs.cpp`/`prefs.h` (global accessors backed
  by a `t_conf_entry` table), `irc.cpp`/`irc.h`, `versioncheck.{cpp,h}`,
  `autoupdate.{cpp,h}`, `adbanner.{cpp,h}`, `ipban.{cpp,h}`,
  `i18n.{cpp,h}`, `icons.{cpp,h}`.
- Support / process management: `support.cpp`/`support.h`,
  `runprog.cpp`/`runprog.h`, `tracker.cpp`/`tracker.h`,
  `udptest_send.cpp`/`udptest_send.h`.
- Lua bridge (compiled only when `WITH_LUA` is defined):
  `luainterface.cpp`/`luainterface.h`, `luafunctions.{cpp,h}`,
  `luaobjects.{cpp,h}`, `luawrapper.{cpp,h}`.
- Bundled headers: `../win32/winmain.{cpp,h}` (see win32 section) and
  `../json/json.hpp` (nlohmann/json).

The `pvpgn::bnetd` namespace wraps every symbol declared in this directory,
nested inside the project root namespace `pvpgn`.

## common/ module map

`src/common/` holds code that is linked into every daemon as the static
library `common` (see `src/common/CMakeLists.txt`). Major modules:

- `fdwatch` -- the abstract socket event-notification API. Public interface
  lives in `fdwatch.h`; the per-platform back ends are
  `fdwatch_select.{cpp,h}`, `fdwatch_poll.{cpp,h}`,
  `fdwatch_epoll.{cpp,h}`, and `fdwatch_kqueue.{cpp,h}`. Selection happens
  via `HAVE_*` symbols computed in `ConfigureChecks.cmake`. Background and
  rationale are documented in `docs/fdwatch.txt`.
- `eventlog` -- variadic templated logger built on `{fmt}`. Levels are the
  bit-field enum `t_eventlog_level` (`none`, `trace`, `debug`, `info`, `warn`,
  `error`, `fatal`, plus `gui` only when `WIN32_GUI` is defined). Convenience
  macros `ERROR0..3`, `WARN0..3`, `INFO0..3`, `DEBUG0..3`, `TRACE0..3` wrap
  the template with `__FUNCTION__` as the module name.
- Networking primitives: `packet.{cpp,h}` (typed packet wrapper plus
  protocol-class enum -- bnet, file, raw, udp, init, d2game, d2gs, d2cs,
  d2cs_bnetd, w3route, wolgameres), `network.{cpp,h}`, `addr.{cpp,h}`,
  `tag.{cpp,h}`.
- Configuration parser: `conf.{cpp,h}` -- generic line/key/value parser
  driven by a `t_conf_entry` table (`set`/`get`/`setdef` callbacks). Used by
  every daemon's `prefs.cpp`.
- Containers / memory: `hashtable.{cpp,h}`, `hash_tuple.hpp`, `list.{cpp,h}`,
  `elist.h`, `queue.{cpp,h}`, `xalloc.{cpp,h}` (xalloc OOM hook used by
  bnetd's safety buffer), `xstr.{cpp,h}`, `xstring.{cpp,h}`,
  `scoped_ptr.h`, `scoped_array.h`, `introtate.h`.
- Hashing primitives: `bnethash.{cpp,h}` (Battle.net hash and SHA-1 helpers),
  `bnethashconv.{cpp,h}`, `bnetsrp3.{cpp,h}` (SRP3 used for newer Battle.net
  auth), `wolhash.{cpp,h}` (Westwood Online), `bigint.{cpp,h}` (the
  arbitrary-precision integer used by SRP and exercised by the unit test).
- Misc: `bn_type.{cpp,h}`, `bnettime.{cpp,h}`, `field_sizes.h`, `flags.h`,
  `proginfo.{cpp,h}`, `rcm.{cpp,h}`, `rlimit.{cpp,h}`, `systemerror.{cpp,h}`,
  `token.{cpp,h}`, `trans.{cpp,h}`, `util.{cpp,h}`,
  `give_up_root_privileges.{cpp,h}`, `gui_printf.{cpp,h}`,
  `hexdump.{cpp,h}`, `lstr.h`, `peerchat.{cpp,h}`,
  `pugiconfig.h` + `pugixml.{cpp,h}` (vendored pugixml).
- Sentinel headers: `setup_before.h` and `setup_after.h` (see Coding model).
- Wire-protocol structure headers: `bnet_protocol.h`,
  `anongame_protocol.h`, `bot_protocol.h`, `file_protocol.h`,
  `init_protocol.h`, `irc_protocol.h`, `udp_protocol.h`,
  `d2game_protocol.h`, `d2cs_protocol.h`, `d2cs_d2gs_protocol.h`,
  `d2cs_d2dbs_ladder.h`, `d2cs_d2gs_character.h`, `d2cs_bnetd_protocol.h`,
  `wol_gameres_protocol.h`, plus the Diablo character file definitions in
  `d2char_checksum.{cpp,h}` and `d2char_file.h`.

## compat/ module map

`src/compat/` is the static `compat` library. Each header/source provides a
portability shim so the rest of the code can call a single API on POSIX,
Windows, and other platforms. Notable shims (see
`src/compat/CMakeLists.txt`): `psock.{cpp,h}` (BSD/WinSock socket API
unification), `gettimeofday.{cpp,h}`, `pdir.{cpp,h}` (directory iteration),
`pgetopt.{cpp,h}`, `mmap.{cpp,h}`, `mkdir.h` (POSIX vs `_mkdir` and
`MKDIR_TAKES_ONE_ARG` resolved by `CheckMkdirArgs.cmake`),
`strcasecmp.{cpp,h}`, `strncasecmp.{cpp,h}`, `strdup.{cpp,h}`,
`strerror.{cpp,h}`, `strsep.{cpp,h}`, `uname.{cpp,h}`, plus header-only
shims for `access`, `gethostname`, `netinet_in`, `pgetpid`, `read`, `recv`,
`rename`, `runtime_libs`, `send`, `socket`, `statmacros`, `stdfileno`,
`termios`. The compatibility decisions are driven by `HAVE_*` macros emitted
into `config.h` from `ConfigureChecks.cmake` and consumed inside each shim.

## Runtime model

bnetd is a single-threaded, event-driven server. Its lifecycle:

1. `main()` (in `src/bnetd/main.cpp`) parses command-line args via
   `cmdline_load()`, optionally daemonizes via `fork_bnetd()` (gated by
   `DO_DAEMONIZE` -- defined when `HAVE_FORK`, `HAVE_CHDIR`, and either
   `HAVE_SETPGID` or `HAVE_SETPGRP` are present), loads the preferences from
   the file given by `cmdline_get_preffile()`, opens the eventlog, drops root
   via `give_up_root_privileges()`, writes a pidfile, and calls
   `pre_server_startup()`.
2. `pre_server_startup()` sets up the OOM safety buffer (1 MB
   `oom_buffer` plus a 1 MiB `emergency_mem` allocated for
   `std::set_new_handler`), initializes the storage backend
   (`storage_init` parses `storage_path` and selects file or sql), the
   socket layer (`psock_init`), the support files, anongame map and match
   lists, and finally `fdwatch_init(prefs_get_max_connections())`. It then
   builds the in-memory account/clan/team/realm/channel/etc. lists, loads
   help, IP bans, ad banners, autoupdate, version-check, news, watch lists,
   the WAR3 XP tables, characters, command groups, alias commands,
   translation tables, tournament, customicons, anongame infos,
   wol matchlists, and (when `WITH_LUA` is defined) the Lua scripts.
3. `server_process()` (in `server.cpp`) runs the main loop. It opens the
   listening sockets defined in `bnetd.conf` for each `t_laddr_type` (bnet,
   w3route, irc, wolv1, wolv2, apireg, wgameres, telnet) and adds them to
   the fdwatch pool. The loop repeatedly calls `fdwatch(timeout)` with a
   `BNETD_POLL_INTERVAL` of 20 ms (see `BNETD_POLL_INTERVAL` and
   `BNETD_JIFFIES = 50` in `src/common/setup_before.h`) and dispatches via
   per-fd handler callbacks (`handle_accept`, `handle_tcp`, `handle_udp`).
4. New TCP connections are wrapped in a `t_connection`, classified per the
   listening address type, and pushed into `connlist` plus fdwatch via
   `conn_add_fdwatch`. Read/write availability flips trigger
   `sd_tcpinput`/`sd_tcpoutput`; the input handler chooses a packet class
   based on `conn_get_class` and dispatches to the appropriate
   `handle_<protocol>_packet` function.
5. Periodic work runs from the timer module (`timer.cpp`,
   `timerlist_add_timer`) at jiffy granularity, plus the `tick.cpp` driver.
   Stale init connections are killed via `prefs_get_initkill_timer`.
   Operator signals (`SIGTERM`, `SIGHUP`, `SIGUSR1`) drive
   `server_quit_wraper`, `server_restart_wraper`, and `server_save_wraper`
   (all guarded by `DO_POSIXSIG`).
6. On shutdown `post_server_shutdown(status)` walks a fall-through `switch`
   to unwind whichever subsystems were initialized, ending with
   `oom_free()` and `delete[] emergency_mem`.

The `BNETD_MAX_OUTBURST = 16384` constant in `setup_before.h` caps a single
`sd_tcpoutput` write. `BNETD_MAX_SOCKETS = 1000` is the default fdwatch
pool size (overridable by `prefs_get_max_connections`). On Windows
`FD_SETSIZE` is redefined to `BNETD_MAX_SOCKVAL = 8192` so `select()` can
handle more sockets.

Daemonization paths exist for both POSIX (`fork_bnetd`) and Windows
(`win32/service.cpp`, `winmain.cpp`, `app_main` versus `main` selected by
`WIN32_GUI`). The Windows GUI build sets a `g_ServiceStatus` global and
installs an `unhandled_handler` crash dump callback. d2cs and d2dbs follow a
similar but smaller pattern in `src/d2cs/main.cpp` and `src/d2dbs/main.cpp`.

## Configuration system

Configuration files ship as `*.in` templates in `conf/`. `conf/CMakeLists.txt`
calls `configure_file(... @ONLY)` to substitute install-path variables
(`@LOCALSTATEDIR@`, `@SYSCONFDIR@`, etc.) into every output. The substituted
files are installed under `${SYSCONFDIR}` for the daemons.

bnetd's configuration is parsed at startup by `prefs_load()` in
`src/bnetd/prefs.cpp`, which uses `conf_load_file` from `src/common/conf.cpp`
and a `t_conf_entry` array binding each option name to a setter, getter, and
default. Defaults live in `src/common/setup_before.h` (e.g.
`BNETD_SERV_PORT = 6112`, `BNETD_TELNET_PORT = 23`,
`BNETD_IRC_PORT = 6667`, `BNETD_W3ROUTE_PORT = 6200`,
`BNETD_REALM_PORT = 6113`, `BNETD_APIREG_PORT = 5400`,
`BNETD_WOLV1_PORT = 4000`, `BNETD_WOLV2_PORT = 4005`,
`BNETD_WGAMERES_PORT = 4807`, `BNETD_TRACK_PORT = 6114`).

Key configuration files (paths under `${SYSCONFDIR}` after install):

- `bnetd.conf` -- main bnetd configuration. Default file is
  `BNETD_DEFAULT_CONF_FILE` (set in `ConfigureChecks.cmake`).
- `d2cs.conf`, `d2dbs.conf` -- per-daemon configuration.
- `channel.conf`, `realm.conf`, `bnban.conf`, `bnalias.conf`,
  `bnmaps.conf`, `bnxpcalc.conf`, `bnxplevel.conf`, `topics.conf`,
  `tournament.conf`, `command_groups.conf`, `supportfile.conf`,
  `address_translation.conf`, `sql_DB_layout.conf`, `icons.conf`,
  `anongame_infos.conf`, `autoupdate.conf` -- domain-specific tables.
- `versioncheck.json`, `ad.json` -- JSON tables (parsed via
  `nlohmann/json` -- see `src/json/json.hpp`).
- `bnetd_default_user.plain` -- template for the default account.
- `bnissue.txt` -- pre-login banner.
- `i18n/` -- per-locale translation XML loaded by `i18n_load()` and
  driven by `BNETD_I18N_DIR` and `BNETD_LOCALIZE_FILE`.

Extra Windows-specific overrides ship as `bnetd.conf.win32`,
`d2cs.conf.win32`, `d2dbs.conf.win32`.

## Storage layer

bnetd's `storage_init(spath)` (in `src/bnetd/storage.cpp`) parses the
`storage_path` config string, splitting on `:` to pick a driver and passing
the remainder to that driver's `init`. Two drivers are present:

- `file` (always compiled) -- backs accounts, clans and teams with directory
  trees of plain-text files. Implementation in
  `src/bnetd/storage_file.cpp`/`.h` plus `file.cpp`/`.h` and
  `file_plain.cpp`/`.h`. Only the `plain` mode is implemented in this
  snapshot; the `cdb` mode discussed in `docs/storage.txt` is documentation
  carried over from earlier versions and is not present as a code path here.
- `sql` (compiled when `WITH_SQL` is defined, i.e. when any of
  `WITH_SQL_MYSQL`, `WITH_SQL_PGSQL`, `WITH_SQL_SQLITE3`,
  `WITH_SQL_ODBC` are set). Common code is in `storage_sql.cpp` and
  `sql_common.cpp`. The string syntax is
  `sql:mode=<driver>;host=...;name=...;user=...;pass=...;default=<uid>;prefix=<p>`.
  Per-driver back ends compile only if their compile-time macro is set:
  `sql_mysql.cpp`, `sql_pgsql.cpp`, `sql_sqlite3.cpp`, `sql_odbc.cpp`.
  In this snapshot `src/CMakeLists.txt` adds `-DWITH_SQL_MYSQL`,
  `-DWITH_SQL_SQLITE3`, and `-DWITH_SQL_PGSQL` based on the corresponding
  `*_FOUND` results, but does not add `-DWITH_SQL_ODBC`; the
  `WITH_ODBC` CMake option still triggers ODBC discovery and links the
  libraries into bnetd, but the ODBC code paths in `sql_common.cpp` and
  `sql_odbc.cpp` will not compile until that define is added.

The dispatch interface is the function-pointer table `t_storage` declared in
`src/bnetd/storage.h`; the file driver populates it as `storage_file` and the
sql driver as `storage_sql`. Schema layout is described by
`conf/sql_DB_layout.conf.in`. For end-user storage configuration see
`docs/storage.txt`.

## Protocol handlers

bnetd dispatches per protocol via the `handle_*` files in `src/bnetd/`. Each
handler corresponds to one or more connection classes and to a listening
address type defined in `setup_before.h`. The set of handlers and their
default ports:

| Source files | Connection class(es) | Listener / port default |
| --- | --- | --- |
| `handle_init.{cpp,h}` | `conn_class_init` | every TCP socket starts here until the first byte tags it as bnet, file, bot, or telnet |
| `handle_bnet.{cpp,h}` | `conn_class_bnet` | `BNETD_SERV_PORT = 6112` (`servaddrs`) |
| `handle_file.{cpp,h}` | `conn_class_file` | shares the bnet listener |
| `handle_bot.{cpp,h}` | `conn_class_bot` | shares the bnet listener |
| `handle_telnet.{cpp,h}` | `conn_class_telnet` | `BNETD_TELNET_PORT = 23` (`telnetaddrs`) |
| `handle_irc.{cpp,h}`, `handle_irc_common.{cpp,h}` | `conn_class_irc`, `conn_class_ircinit` | `BNETD_IRC_PORT = 6667` (`ircaddrs`) |
| `handle_wol.{cpp,h}`, `handle_wol_gameres.{cpp,h}`, `handle_wserv.{cpp,h}` | `conn_class_wol`, `conn_class_wserv`, `conn_class_wgameres`, `conn_class_wladder` | `BNETD_WOLV1_PORT = 4000`, `BNETD_WOLV2_PORT = 4005`, `BNETD_WGAMERES_PORT = 4807` |
| `handle_apireg.{cpp,h}` | `conn_class_apireg` | `BNETD_APIREG_PORT = 5400` (`apiregaddrs`) |
| `handle_d2cs.{cpp,h}` | `conn_class_d2cs_bnetd` | bnetd opens the d2cs realm connection out to `BNETD_REALM_PORT = 6113` (the d2cs listening port; see `realm.cpp`) |
| `handle_anongame.{cpp,h}` | invoked from bnet path | (no listener -- used for matchmaking) |
| `handle_udp.{cpp,h}` | UDP packet path | UDP listener uses `BNETD_DEF_TEST_PORT = 6112` (configurable via `udptest_port`); the `BNETD_MIN_TEST_PORT`..`BNETD_MAX_TEST_PORT` 6112-6500 range is the *client*-side bind-search range used by `src/client/udptest.cpp`, not a server listener range |
| `handle_w3route_packet` (declared in `anongame.h`, dispatched from `server.cpp`) | `conn_class_w3route` | `BNETD_W3ROUTE_PORT = 6200`, default address `BNETD_W3ROUTE_ADDR = 0.0.0.0` |

`handlers.h` defines the shared `t_handler` callback signature
`int(*)(t_connection *, t_packet const * const)` and the `t_htable_row` row
used to wire packet types to handler functions inside each `handle_*.cpp`.

## Lua scripting

When CMake is configured with `WITH_LUA=ON`, `find_package(Lua REQUIRED)`
discovers the Lua development headers and library (CI installs
`liblua5.1`), the `lua/` subdirectory is added to the build (so the scripts
are installed under `${LOCALSTATEDIR}/lua` -- see `lua/CMakeLists.txt`),
and the bnetd build defines `WITH_LUA` so the bridge files compile.

Layout under `lua/`:

- `main.lua` -- entry point. Initializes the antihack and ghost subsystems
  if their flags are set in `config.lua`.
- `config.lua` -- declarative feature toggles consumed by `main.lua` and the
  hook handlers.
- `handle_channel.lua`, `handle_client.lua`, `handle_command.lua`,
  `handle_game.lua`, `handle_server.lua`, `handle_user.lua` -- hook
  dispatchers, one per `t_luaevent_type` group declared in
  `src/bnetd/luainterface.h` (`luaevent_command*`,
  `luaevent_game_*`, `luaevent_channel_*`, `luaevent_user_*`,
  `luaevent_server_*`).
- `extend/` -- helpers for native objects: `account.lua`,
  `account_wrap.lua`, `channel.lua`, `eventlog.lua`, `game.lua`,
  `message.lua`, plus the `enum/` directory.
- `include/` -- general-purpose Lua utilities: `bitwise.lua`, `common.lua`,
  `convert.lua`, `file.lua`, `math.lua`, `string.lua`, `table.lua`,
  `timer.lua`.
- `command/` -- example custom slash commands wired into
  `handle_command.lua`: `ping.lua`, `redirect.lua`, `stats.lua`,
  `w3motd.lua`.
- `antihack/`, `ghost/`, `quiz/` -- bundled subsystems toggled in
  `config.lua`.

The C++ side of the bridge lives in `src/bnetd/luainterface.{cpp,h}`,
`luafunctions.{cpp,h}`, `luaobjects.{cpp,h}`, and `luawrapper.{cpp,h}`.
`lua_load(scriptdir)` is invoked from `pre_server_startup()` (in
`src/bnetd/main.cpp`). `lua_unload()` is only called from the SIGHUP
rehash path in `server_process()` when `restart_mode_lua` (or
`restart_mode_all`) is requested; `post_server_shutdown()` does not
call it -- the Lua state is reclaimed at process exit. The per-event
hooks are `lua_handle_command`, `lua_handle_game`, `lua_handle_channel`,
`lua_handle_user`, `lua_handle_user_icon`, `lua_handle_server`,
`lua_handle_client_readmemory`, and `lua_handle_client_extrawork`.

## Logging

`pvpgn::eventlog` is a function template defined inline in
`src/common/eventlog.h`. The signature is
`eventlog(t_eventlog_level level, const char* module, fmt::string_view fmt, const Args&... args)`.
Levels (a power-of-two bitfield) are `eventlog_level_none`, `..._trace`,
`..._debug`, `..._info`, `..._warn`, `..._error`, `..._fatal`, plus
`..._gui` only when `WIN32_GUI` is defined. The active mask is the
`currlevel` global, populated at startup from the comma-separated
`loglevels` setting (default `BNETD_LOG_LEVELS = "warn,error"`).

Log output is written to the `FILE*` registered via `eventlog_set` /
`eventlog_open`. The format is `<timestamp> [<level>] <module>: <message>\n`
with timestamps formatted via `EVENT_TIME_FORMAT = "%b %d %H:%M:%S"` from
`setup_before.h`. Convenience macros `ERROR0..3`, `WARN0..3`, `INFO0..3`,
`DEBUG0..3`, `TRACE0..3` (defined at the bottom of `eventlog.h`) wrap the
template and inject `__FUNCTION__` as the module argument; new code is
expected to call `eventlog()` directly.

## Build option matrix

Top-level CMake options (see `CMakeLists.txt`):

| Option | Default | Effect |
| --- | --- | --- |
| `WITH_BNETD` | ON | Build the `bnetd` daemon and pull in `WITH_BNETD`-gated code such as ZLIB linkage. |
| `WITH_D2CS` | ON | Build the `d2cs` daemon. |
| `WITH_D2DBS` | ON | Build the `d2dbs` daemon. |
| `WITH_LUA` | OFF | Adds `-DWITH_LUA`, calls `find_package(Lua REQUIRED)` (CI uses `liblua5.1`), compiles the Lua bridge inside bnetd, and installs the `lua/` script tree. |
| `WITH_MYSQL` | OFF | Defines `WITH_SQL_MYSQL`, requires `find_package(MySQL REQUIRED)`, links MySQL into bnetd. |
| `WITH_SQLITE3` | OFF | Defines `WITH_SQL_SQLITE3`, requires SQLite3, links SQLite into bnetd. |
| `WITH_PGSQL` | OFF | Defines `WITH_SQL_PGSQL`, requires PostgreSQL, links libpq into bnetd. |
| `WITH_ODBC` | OFF | Requires ODBC via `find_package(ODBC REQUIRED)` and links the discovered ODBC libraries into bnetd. Note: in this snapshot `src/CMakeLists.txt` does not currently add the `-DWITH_SQL_ODBC` compile define, so the ODBC code in `sql_common.cpp` / `sql_odbc.cpp` is excluded by the preprocessor even when the option is on. |
| `WITH_WIN32_GUI` | ON (Windows only) | Adds `-DWIN32_GUI`, switches the executable target to `WIN32` (no console), and links the GUI resource files instead of the console resources. |

Any of `WITH_SQL_MYSQL`, `WITH_SQL_PGSQL`, `WITH_SQL_SQLITE3`,
`WITH_SQL_ODBC` enables the umbrella `WITH_SQL` macro inside
`setup_before.h`, which is what `storage.cpp` and `storage_sql.cpp` actually
key off of.

Compile requirements (from the top-level `CMakeLists.txt`):

- CMake 3.1+, C++11, `CMAKE_CXX_EXTENSIONS OFF`.
- GCC 5.1 or higher, MSVC 19.0 (Visual Studio 2015) or higher, or Clang.

## CI

- `.travis.yml` -- Linux/amd64 (focal) gcc build with MySQL and Lua. Runs
  `cmake -D WITH_MYSQL=true -D WITH_LUA=true ../`, `make`, then
  `make install` and `make uninstall` as a sanity check.
- `appveyor.yml` -- Visual Studio 2019 (x86) builds via the Magic Builder
  helper (`pvpgn-magic-builder`). Matrix covers all storage drivers
  (`plain`, `mysql`, `pgsql`, `sqlite`, `odbc`) and produces both GUI and
  console builds for each.
- `.github/workflows/codeql-analysis.yml` -- CodeQL `cpp` analysis, runs on
  push/PR against `master` and `develop`.
