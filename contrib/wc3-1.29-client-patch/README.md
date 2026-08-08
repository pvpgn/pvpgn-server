# Warcraft III 1.29.2 client patcher for PvPGN

Patches a vanilla Warcraft III 1.29.2.9231 client binary so it can connect
to a PvPGN server and authenticate with a real password. Produces a
self-contained patched `Warcraft III.exe` — no DLLs, no loaders, no
runtime injection.

The patched EXE works against an upstream-style PvPGN with **no
server-side cryptographic bypass**. Just add the `0x1d` versioncheck row
(already done in this branch) and run.

## How it works

This mirrors what
[W3CE](https://github.com/Warcraft-III-Community-Edition/Launcher)'s
`PvPGNPatcher` does (analysed from `W3CE-v0.2.1-alpha`), but as a static
patch. We append a new PE section (`.pvpgnh`) to the EXE containing:

- A port of PvPGN's `bnet_hash()` from `src/common/bnethash.cpp`
  (`do_blizzard_hash` variant).
- An `LphChecked` replacement (replaces the SRP3 M1 computation; the
  client now sends `bnet_hash(lower(password))` as the 20-byte logon
  proof).
- A `CreateAccount` trampoline + wrapper (calls the original, then
  copies the plaintext password into the verifier slot — PvPGN reads
  this as a null-terminated plaintext string and stores
  `passhash1 = bnet_hash(plaintext)`).

Plus four byte patches in `.text`:

| Site | VA | What it does |
| :--- | :--- | :--- |
| `auth_req` | `0x00831424` | NOPs the JNE that fails the 128-byte server-signature compare. PvPGN sends an all-zero signature; this lets the client accept it. |
| M2 dispatch | `0x00830DDB` | `74 21 -> 75 20`. Flips a JZ to JNE so the client accepts an all-zero M2 reply (which is what PvPGN's legacy `hash_eq` success path emits). |
| `LphChecked` entry | `0x0083A590` | 5-byte JMP to the new `lph_checked_hook` in our cave. |
| `CreateAccount` entry | `0x00986A50` | 5-byte JMP to our wrapper. |

End-to-end: client sends `bnet_hash(password)` → PvPGN's existing
`passhash1` compare matches (because the create-account heuristic
extracted the plaintext we put in the verifier slot) → login succeeds.
Wrong passwords fail. Login then proceeds to channel join, MOTD,
etc. as normal.

## Building (Linux)

Requirements:

```bash
sudo apt install gcc-multilib python3-pip
python3 -m venv ~/venv
source ~/venv/bin/activate
pip install keystone-engine pefile capstone
```

Build:

```bash
cd contrib/wc3-1.29-client-patch
python3 build_payload.py /path/to/clean/Warcraft\ III.exe ./Warcraft\ III\ -\ PvPGN.exe
```

The script:
1. Verifies the input MD5 is `c453b4a0bde1a47f41cb2475573edb84` (vanilla
   1.29.2.9231).
2. Compiles `bnet_hash.c` to a flat 32-bit blob via `gcc -m32` + `ld
   --oformat=binary`.
3. Assembles the CreateAccount trampoline + wrapper via
   `keystone-engine`.
4. Strips the Authenticode signature (it'd be invalid after our patches
   anyway).
5. Appends a new PE section `.pvpgnh` with the cave contents.
6. Patches the four entry points in `.text` to redirect into the cave.
7. Writes the patched output, prints its MD5.

For 1.29.2.9231 the output is deterministic (same C, same keystone,
same input) — current reference MD5 `729ac6d7b8438d9579c9a1738ef2d42a`.

## Files

- `bnet_hash.c` — clean C reference for the modified-SHA1 hash PvPGN
  uses. Compiled with no libc, no external symbols. Also contains
  `lph_checked_hook` (the LphChecked replacement) so the build
  produces a single object file.
- `build_payload.py` — the main build orchestrator. Reads, patches,
  writes the EXE.

## Server-side: pointing the client at PvPGN

WC3 1.29 reads the BNCS gateway list from `<install>\BattleNet\bnserver-WAR3.ini`
(note the `BattleNet\` subfolder — different from 1.27/1.28, which used
the install root). Sample contents:

```ini
[Server List Version]
VER=1001

[Server Gateways]
1=192.168.1.223

[192.168.1.223]
ZONE=0
ENU=Local PvPGN
```

Replace the IP/hostname with wherever your PvPGN server is listening.

## Caveats

- **Authenticode signature is stripped.** Modifying any byte breaks
  Blizzard's signature; the build also clears the Security data
  directory entry. Windows SmartScreen may warn ("Unknown Publisher")
  on first launch — accept and continue.
- **ASCII passwords only.** The LphChecked hook's lowercasing logic
  handles ASCII A–Z; it's not Unicode-aware. PvPGN's own create-account
  path also lowercases ASCII-only via `strtolower`, so this matches
  upstream behaviour.
- **Only 1.29.2.9231 is supported.** The patcher is hardcoded against
  the byte offsets of that exact build (vanilla MD5
  `c453b4a0bde1a47f41cb2475573edb84`). Other 1.29.x sub-versions need
  the offsets re-derived; see `docs/wc3-1.29-pvpgn-notes.md` and
  `docs/wc3-1.29-real-auth-plan.md` for the methodology.
- **No support for Reforged (1.32+).** Reforged dropped classic BNCS
  entirely; PvPGN can't talk to Reforged at all.

## Background and references

- `docs/wc3-1.29-pvpgn-notes.md` — full binary-analysis writeup of
  what's in 1.29.2.9231 and how the patch sites were located.
- `docs/wc3-1.29-real-auth-plan.md` — design plan + W3CE analysis
  that produced this patcher.
- W3CE: <https://github.com/Warcraft-III-Community-Edition/Launcher>
- `src/common/bnethash.cpp` (in this repo): the canonical
  `bnet_hash` (`do_blizzard_hash` variant) implementation we ported.

## Reverting

```cmd
del "Warcraft III.exe"
ren "Warcraft III.exe.orig" "Warcraft III.exe"
```

Or just re-install from clean media. The patcher never modifies the
input file in place — it always writes a new copy.
