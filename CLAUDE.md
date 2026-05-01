# CLAUDE.md

Onboarding and quick-reference document for working on PvPGN-PRO. Read this
first; follow the links below for depth.

## Project summary

PvPGN-PRO is a cross-platform C++11 server that emulates Battle.net (Blizzard)
and Westwood Online (Westwood/EA) game-network protocols, allowing legacy
clients (Diablo, Diablo 2, StarCraft, WarCraft 2/3, Westwood Chat, Command &
Conquer, Nox, Dune 2000, Emperor: Battle for Dune) to connect to a private
server for chat, matchmaking, ladders, and game hosting. It is a community
fork of the original PvPGN project (whose upstream development stopped in
2011), which itself descends from the bnetd codebase -- hence the main daemon
is still called `bnetd` and its main config is `bnetd.conf`. The supported
client list, history, and licensing (GPL v2) are in `README.md`, `README.DEV`
section 2, and `LICENSE`.

## Quick orientation

| When you need... | Read |
| --- | --- |
| Build / install instructions for any OS | `README.md` |
| Coding style, RAII, exceptions, identifiers, namespaces, include order, valgrind recipe, glossary | `README.DEV` |
| Developer architecture overview (source layout, daemons, runtime model, modules, protocol handlers, build matrix) | `docs/architecture.md` |
| Index of operator/admin docs (ad banners, motd, ports, storage, fdwatch, versioncheck) | `docs/readme.md` |
| Per-release config-version migration notes | `UPDATE` |
| Per-release feature notes / changelog | `NEWS`, `version-history.txt` |
| Tested OS/compiler matrix | `docs/ports.md` |
| Storage backend selection (`storage_path` syntax) | `docs/storage.txt` |
| Manpages for the binaries | `man/*.1` |

## Build commands

CMake 3.1+ and a C++11 compiler (GCC 5.1+, MSVC 19.0 / VS 2015+, or Clang).
The build is enforced in the top-level `CMakeLists.txt`.

Linux / macOS / BSD:

    cmake -G "Unix Makefiles" -H./ -B./build
    cd build && make

With every optional subsystem enabled:

    cmake -G "Unix Makefiles" -H./ -B./build \
      -DWITH_LUA=ON \
      -DWITH_MYSQL=ON \
      -DWITH_SQLITE3=ON \
      -DWITH_PGSQL=ON \
      -DWITH_ODBC=ON

(Also accepted as `-D WITH_LUA=true` per `.travis.yml`.)

Windows: easiest path is the
[Magic Builder](https://github.com/pvpgn/pvpgn-magic-builder) helper (also
what AppVeyor uses). To drive CMake directly:

    cmake -G "Visual Studio 14 2015" -H./ -B./build

then open the generated `.sln`.

Install / uninstall / purge (run from the build dir):

    make install        # installs binaries, configs, scripts, files, manpages, lua
    make uninstall      # remove files installed by `install` (custom target)
    make purge          # uninstall plus wipe state directories (custom target)

Both `uninstall` and `purge` are custom targets defined at the bottom of the
top-level `CMakeLists.txt`.

## Test commands

CTest is enabled at the top level (`enable_testing()` in `CMakeLists.txt`).
After configuring the build:

    cd build
    ctest --output-on-failure

There are two unit tests today (see `src/test/CMakeLists.txt`):

- `bnetsrp3_test` -- exercises the SRP3 hash used by newer Battle.net auth.
- `bigint` -- exercises the arbitrary-precision integer used by SRP.

There is no broader integration / functional suite; coverage is thin. When
adding code that touches `common/`, prefer adding a test under `src/test/`
following the same `add_executable` + `add_test` pattern.

## How to run for development

Foreground / debugging:

    /path/to/bnetd -f                    # run in foreground (POSIX, gated by DO_DAEMONIZE)
    /path/to/bnetd -c /path/to/bnetd.conf
    /path/to/bnetd -D                    # debug mode (foreground + stdout logs)

Default config path comes from `BNETD_DEFAULT_CONF_FILE` (set in
`ConfigureChecks.cmake` to `${SYSCONFDIR}/bnetd.conf` on install, falling
back to `conf/bnetd.conf` when running from a build tree). Other CLI flags
are listed in `src/bnetd/cmdline.cpp` (`--service`, `-s install/uninstall`
on Windows; `--gui`/`--console` when `WIN32_GUI` is built; `-d FILE` for a
hex packet dump).

Memory debugging (Linux), per `README.DEV` Appendix B:

    valgrind --tool=memcheck --num-callers=10 \
             --leak-check=yes --leak-resolution=high \
             /path/to/bnetd -f 2> valg.out

`d2cs` and `d2dbs` accept the same `-c`/`-f`/`-D` style flags via their own
`cmdline.cpp` files.

## Codebase map at a glance

| Path | Role |
| --- | --- |
| `src/bnetd/` | Main Battle.net server daemon (largest subtree) |
| `src/d2cs/` | Diablo 2 character server (separate daemon) |
| `src/d2dbs/` | Diablo 2 database server (separate daemon) |
| `src/common/` | Static lib linked into every daemon: fdwatch, eventlog, packet/network, conf parser, hashtable, hashing primitives, wire-protocol headers, vendored pugixml |
| `src/compat/` | Static lib of POSIX/Win32 portability shims (psock, gettimeofday, mkdir, pdir, pgetopt, mmap, etc.) |
| `src/client/` | Reference client utilities: `bnchat`, `bnstat`, `bnftp`, `bnbot` (the `udptest.cpp` source compiles into `bnchat` and `bnstat` — there is no standalone `udptest` binary) |
| `src/bniutils/` | BNI icon archive tools: `bnilist`, `bni2tga`, `bniextract`, `bnibuild`, `tgainfo` |
| `src/bnpass/` | Password hash CLI (`bnpass`, `sha1hash`) |
| `src/bntrackd/` | Standalone tracking daemon (`bntrackd`) |
| `src/bnproxy/` | Historical TCP proxy sources; **not built** in this snapshot (no `CMakeLists.txt`) |
| `src/win32/` | Windows entrypoints, service, GUI resources, crash dump |
| `src/json/` | Vendored `nlohmann/json` single header |
| `src/test/` | CTest unit tests (`bnetsrp3_test`, `bigint`) |
| `conf/` | `*.in` config templates substituted by CMake at configure time, plus `i18n/` |
| `lua/` | Lua scripts shipped when `WITH_LUA=ON` |
| `cmake/Modules/` | `Find{MySQL,ODBC,PostgreSQL,SQLite3}.cmake`, install-paths, uninstall/purge templates |
| `lib/fmt/` | Vendored `{fmt}` library used by `eventlog()` |
| `man/` | nroff manpages |
| `scripts/` | Init.d wrappers, log rotation, ladder conversion, password hashers |
| `files/` | BNI icon archives, `bnserver*.ini` overrides, autoupdate MPQs, ad banners, `newbie.save` template |
| `docs/` | Developer/operator docs |

For depth -- module-by-module breakdown of `src/bnetd/`, `src/common/`,
`src/compat/`, the runtime model, the configuration system, the storage
layer, the protocol handlers, the Lua subsystem, and the build option matrix
-- see `docs/architecture.md`.

## Coding conventions

Authoritative source: `README.DEV` section 5. Highlights:

- C++11 (`CMAKE_CXX_STANDARD 11`, `CMAKE_CXX_EXTENSIONS OFF`). Treat the
  C++ Core Guidelines as a reference (see `README.md` "Development").
- Indent with **tabs**, tab width = 8. Brace on the same column as the
  controlling keyword. `switch`, `namespace`, and access modifiers do not
  add a level.
- Function definitions split the return type onto its own line above the
  `Class::method(args)` line:
      const std::string&
      MyClass::getString()
      { ... }
- Accessors are `getX` / `setX`. Class names are `UpperCamelCase`; method
  names are `lowerCamelCase`.
- No function bodies in headers (templates and class templates are the
  exception).
- All code lives under `namespace pvpgn { ... }`. Daemon-specific code is
  nested further: `pvpgn::bnetd`, `pvpgn::d2cs`, `pvpgn::d2dbs`. Common
  code stays in plain `pvpgn` so every daemon sees it.
- **No `static` for file-local variables or functions.** Use an unnamed
  namespace instead. `README.DEV` 5.e is explicit about this.
- **Never** put `using` directives in headers.
- Include order in every `.cpp`: `common/setup_before.h` first; then the
  matching header for this `.cpp`; then standard C++ library; then C90
  via `<cstring>` etc.; then platform/POSIX/Win32 headers. End the file
  with `common/setup_after.h`.
- Resources (memory, files, sockets, etc.) acquired in a function must be
  released before that function returns. Use RAII wrappers so an exception
  cannot leak. Prefer `new`/`delete` over `malloc`/`free` (so OOM throws
  `std::bad_alloc`); for legacy callers use `xalloc`/`xstrdup` from
  `src/common/xalloc.{cpp,h}` to match surrounding code.
- All thrown objects must derive from `std::exception`. Catch by const
  reference: `} catch (const std::exception& ex) { ... }`. Define a custom
  exception class only when callers must catch it separately, and nest it
  inside the throwing class.
- `eventlog()` uses `{fmt}`-style `{}` placeholders, not `printf` `%s`.

## Logging

Logger is the variadic template `pvpgn::eventlog(...)` in
`src/common/eventlog.h`. Call it directly:

    eventlog(eventlog_level_info, __FUNCTION__, "loaded {} accounts", count);

Levels (bitfield) defined in the same header: `eventlog_level_none`,
`..._trace`, `..._debug`, `..._info`, `..._warn`, `..._error`,
`..._fatal`, plus `..._gui` only when `WIN32_GUI` is defined. The active
mask is set from the comma-separated `loglevels` config option (default
`BNETD_LOG_LEVELS = "warn,error"` from `setup_before.h`). To get verbose
logs for bug reports set `loglevels = fatal,error,warn,info,debug,trace`
in `bnetd.conf` (per `README.md` "Support").

Output goes to the file named by `logfile` in `bnetd.conf` (default
`BNETD_LOG_FILE = "logs/bnetd.log"`). Format is
`<timestamp> [<level>] <module>: <message>` with timestamps formatted by
`EVENT_TIME_FORMAT = "%b %d %H:%M:%S"`.

Legacy convenience macros (`ERROR0..3`, `WARN0..3`, `INFO0..3`,
`DEBUG0..3`, `TRACE0..3`) inject `__FUNCTION__` automatically and are
defined at the bottom of `eventlog.h`. They still appear throughout the
codebase, but new code should prefer calling `eventlog()` directly so the
`{fmt}` placeholders are explicit.

## Adding a configuration option

bnetd configuration is parsed at startup by `prefs_load()` in
`src/bnetd/prefs.cpp` using a `t_conf_entry` table -- each row binds a
directive name to set/get/setdef callbacks. To add a new option:

1. Add a `prefs_get_<name>()` accessor in `src/bnetd/prefs.h` and define
   it in `prefs.cpp`.
2. Add `conf_set_<name>` / `conf_get_<name>` / `conf_setdef_<name>`
   helpers in `prefs.cpp` (see existing entries near line 726 of that
   file).
3. Add a row to `conf_table[]` so `conf_load_file()` picks it up.
4. Add a default constant in `src/common/setup_before.h` if one is
   needed.
5. Add the option (with a comment) to `conf/bnetd.conf.in`. CMake will
   substitute install paths into the installed file via `configure_file`.
6. Add an entry to `UPDATE` describing the new directive so existing
   operators know to add it when migrating configs.

`d2cs` and `d2dbs` follow the same pattern in their own `prefs.cpp` files.

## Storage backends

Selection happens via the `storage_path` config string parsed by
`storage_init()` in `src/bnetd/storage.cpp`. Two top-level drivers:

- `file:...` -- always compiled. Plain-text files for accounts, clans,
  and teams. (The `cdb` mode mentioned in older docs is not present in
  this snapshot's code.)
- `sql:mode=<driver>;host=...;name=...;user=...;pass=...;default=<uid>;prefix=<p>`
  -- compiled when `WITH_SQL` is enabled. Per-driver back ends compile
  only if their compile-time macro is set: `WITH_SQL_MYSQL`,
  `WITH_SQL_PGSQL`, `WITH_SQL_SQLITE3`, `WITH_SQL_ODBC`. Note: in this
  snapshot `src/CMakeLists.txt` does not currently emit
  `-DWITH_SQL_ODBC` even when `WITH_ODBC=ON`, so the ODBC code paths are
  excluded; see `docs/architecture.md` "Storage layer" for the gory
  details.

Schema layout for the SQL drivers is described by
`conf/sql_DB_layout.conf.in`. End-user configuration syntax and examples
live in `docs/storage.txt`.

## Lua scripting

Optional, requires `WITH_LUA=ON` (then `find_package(Lua REQUIRED)`; CI
installs `liblua5.1-0-dev`). Scripts ship under `lua/` and install to
`${LOCALSTATEDIR}/lua` -- see `lua/CMakeLists.txt`.

Layout summary:

- `lua/main.lua` -- entry point; initializes antihack and ghost based on
  `lua/config.lua`.
- `lua/handle_channel.lua`, `handle_client.lua`, `handle_command.lua`,
  `handle_game.lua`, `handle_server.lua`, `handle_user.lua` -- one
  dispatcher per `t_luaevent_type` group declared in
  `src/bnetd/luainterface.h`.
- `lua/extend/` -- helpers wrapping native objects (account, channel,
  game, message, eventlog).
- `lua/include/` -- general-purpose Lua utilities.
- `lua/command/` -- example custom slash commands (`ping`, `redirect`,
  `stats`, `w3motd`).
- `lua/antihack/`, `lua/ghost/`, `lua/quiz/` -- bundled subsystems gated
  by `config.lua`.

The C++ bridge is in `src/bnetd/luainterface.{cpp,h}`,
`luafunctions.{cpp,h}`, `luaobjects.{cpp,h}`, `luawrapper.{cpp,h}`.
`lua_load(scriptdir)` runs from `pre_server_startup()` (in
`src/bnetd/main.cpp`); `lua_unload()` is invoked from the SIGHUP rehash
path in `server_process()` when `restart_mode_lua` (or `restart_mode_all`)
is requested -- it is not part of `post_server_shutdown()`, which lets
process exit reclaim the Lua state. Hook entry points exposed to the C++
side include `lua_handle_command`, `lua_handle_game`, `lua_handle_channel`,
`lua_handle_user`, `lua_handle_user_icon`, `lua_handle_server`,
`lua_handle_client_readmemory`, `lua_handle_client_extrawork`.

See `docs/architecture.md` "Lua scripting" for depth.

## Adding a chat command

Slash commands handled in C++ live in
`src/bnetd/command.cpp`. The dispatcher is `handle_command()`; it walks
the `standard_command_table[]` array of `{ "/name", _handler }` rows
(near line 425). To add a new built-in command:

1. Write `int _handle_<name>_command(t_connection* c, char const* text)`
   in `command.cpp`. The existing handlers all use a `static` storage
   class -- legacy that predates the project's "unnamed namespace
   instead of `static`" rule (`README.DEV` 5.e). Match the surrounding
   style: keep `static` to stay consistent with the rest of the file
   even though new translation units should prefer an unnamed namespace.
2. Add a row to `standard_command_table[]`.
3. If the command should be permission-gated, add the corresponding
   privilege bit to `command_groups.conf` and check it inside the
   handler.
4. Aliases for existing commands can be configured at runtime via
   `bnalias.conf` (no code change required).
5. For script-side commands, add a Lua handler under `lua/command/` and
   wire it into `lua/handle_command.lua` -- this avoids a recompile.

## Adding a protocol handler

Each protocol/connection-class lives in `src/bnetd/handle_<name>.cpp`
plus `handle_<name>.h`. The shared signature is in `handlers.h`:

    typedef int(*t_handler)(t_connection *, t_packet const * const);

Inside each `handle_<name>.cpp` a `t_htable_row` table maps packet
type IDs to handler functions; `handle_<name>_packet()` is the entry
point dispatched from the input path in `server.cpp`. To add a new
protocol:

1. Add a connection class to the enum in `connection.h`
   (`conn_class_*`).
2. If the protocol needs its own listening port, add a `t_laddr_type`
   and threading through `server.cpp` (see how `irc`, `wol`,
   `apireg`, `telnet`, `w3route`, `wgameres` are wired). Default ports
   are constants in `src/common/setup_before.h`.
3. Create `handle_<name>.{cpp,h}` -- add an entry handler, the
   per-packet-type `t_htable_row` table, and the conn-class teardown.
4. Wire the handler into `server.cpp`'s dispatch.
5. Add tests under `src/test/` if the parsing/encoding lives in
   `src/common/`.

For the inventory of existing handlers and their default ports see
`docs/architecture.md` "Protocol handlers".

## Common pitfalls

- `common/setup_before.h` MUST be the first include in every `.cpp`.
  It pulls in the CMake-generated `config.h` and platform macros
  (`HAVE_*`, `WITH_*`) that downstream headers depend on. End the file
  with `common/setup_after.h`. Adding a new `.cpp` without this pair
  will produce confusing platform-specific build failures.
- Raw `static` for translation-unit-local symbols is disallowed by
  project style (`README.DEV` 5.e). Use an unnamed namespace.
- Function bodies do not belong in `.h` files (except for templates).
- Match the surrounding allocation idiom: most code uses `new`/`delete`
  (so `std::bad_alloc` is thrown on OOM and caught by the safety
  buffer), but legacy paths still use `xalloc`/`xstrdup`/`xfree` from
  `src/common/xalloc.{cpp,h}`. Don't mix raw `malloc` with `xfree` or
  vice versa.
- Sockets registered with fdwatch must be removed before close.
  `conn_destroy()` in `src/bnetd/connection.cpp` already calls
  `fdwatch_del_fd(c->socket.fdw_idx)` -- if you add a code path that
  closes a connection's socket directly, route it through the same
  helper or you will leak the fdwatch slot.
- `eventlog()` uses `{fmt}` `{}` placeholders. `printf`-style `%s`
  inside a format string will silently format wrong.
- Be careful with `using namespace` in headers: it is forbidden by
  `README.DEV` and will leak into every translation unit that includes
  the header.
- When adding a new SQL driver branch, gate it on the corresponding
  `WITH_SQL_*` define and add the `-D` flag in `src/CMakeLists.txt`
  (see the existing MySQL/PostgreSQL/SQLite3 branches; ODBC is the
  cautionary example).
- Do not commit Windows line endings; `README.DEV` 5.a requires
  Unix-style line endings throughout.

## CI

| Pipeline | What it runs |
| --- | --- |
| Travis (`.travis.yml`) | Linux/amd64 (Ubuntu focal) gcc build with MySQL and Lua. Runs `cmake -D WITH_MYSQL=true -D WITH_LUA=true ../`, `make`, then `make install` and `make uninstall` as a sanity check. Branches: `master`, `develop`. |
| AppVeyor (`appveyor.yml`) | Visual Studio 2019, x86, Release. Build matrix covers all storage drivers (`plain`, `mysql`, `pgsql`, `sqlite`, `odbc`) and produces both GUI and console binaries for each. Driven via the `pvpgn-magic-builder` helper. Branches: `master`, `develop`. |
| GitHub Actions CodeQL (`.github/workflows/codeql-analysis.yml`) | CodeQL `cpp` static analysis on push/PR against `master`/`develop`. |

PRs must pass all three. There is no separate test job today (CTest is
not invoked in CI).

## Where to ask / report

GitHub issues at <https://github.com/pvpgn/pvpgn-server/issues>
(per `README.md` "Support"). Before posting logs set
`loglevels = fatal,error,warn,info,debug,trace` in `bnetd.conf`.

D2GS is not part of PvPGN-PRO and is unsupported here.
