#!/usr/bin/env python3
"""
Build a self-contained patched Warcraft III.exe for 1.29.2.9231 that
implements real password verification against PvPGN, mirroring W3CE's
PvPGNPatcher approach but in pure native code with no DLL dependencies.

Output: a patched .exe with one extra PE section ('.pvpgnh') containing:
  - bnet_hash() (compiled C from bnet_hash.c)
  - lph_checked_hook() (compiled C, calls bnet_hash)
  - create_account trampoline (saved prologue + jmp back to original+5)
  - create_account wrapper (re-pushes args, calls trampoline,
    copies plaintext password into the verifier slot post-call)

PE entry-point patches in the original .text:
  - 0x00831424: auth_req JNE -> 6-byte multibyte NOP (already used)
  - 0x0083A590: LphChecked entry -> 5-byte JMP to our hook
  - 0x00986A50: CreateAccount entry -> 5-byte JMP to our wrapper

Reverts everything else from the bypass-mode patcher (no more lph_v_*
NOPs).
"""
import os, sys, struct, subprocess, hashlib, shutil
from pathlib import Path

import keystone

HERE = Path(__file__).parent

# Defaults for stand-alone use; override via argv[1] / argv[2] if needed.
SRC_EXE = sys.argv[1] if len(sys.argv) > 1 else str(HERE / "Warcraft III.exe")
DST_EXE = sys.argv[2] if len(sys.argv) > 2 else str(HERE / "Warcraft III - PvPGN.exe")

# Targets from W3CE PvPGNPatcher (verified against our binary)
VA_AUTH_REQ_JNE   = 0x00831424      # 6-byte JNE: 0F 85 B8 00 00 00
VA_M2_DISPATCH    = 0x00830DDB      # JZ in M2-handling code; flip to JNE
VA_LPH_CHECKED    = 0x0083A590      # __cdecl int LphChecked(out, ctx, a3, a4)
VA_CREATE_ACCT    = 0x00986A50      # __thiscall int CreateAccount(this, u, p, a4, a5)
VA_CREATE_ACCT_5  = VA_CREATE_ACCT + 5  # where trampoline returns control

# 1.29.2.9231 expected MD5 (vanilla)
EXPECTED_MD5 = bytes.fromhex("c453b4a0bde1a47f41cb2475573edb84")


# ---------------------------------------------------------------------------
# Step 1: compile bnet_hash.c into a flat binary blob
# ---------------------------------------------------------------------------
def build_bnet_blob():
    obj = HERE / "bnet_hash.o"
    bin_ = HERE / "bnet_hash.bin"
    src = HERE / "bnet_hash.c"
    subprocess.check_call([
        "gcc", "-m32", "-O2", "-fno-pic", "-fno-stack-protector",
        "-ffreestanding", "-nostdlib",
        "-fno-asynchronous-unwind-tables", "-fcf-protection=none",
        "-fno-jump-tables", "-mpreferred-stack-boundary=2",
        "-c", str(src), "-o", str(obj),
    ])
    subprocess.check_call([
        "ld", "-m", "elf_i386", "-Ttext=0", "--oformat=binary",
        "--no-dynamic-linker", "-nostdlib",
        "-o", str(bin_), str(obj),
    ], stderr=subprocess.DEVNULL)

    # Read symbol table from the .o to find lph_checked_hook offset
    nm_out = subprocess.check_output(["nm", str(obj)]).decode()
    syms = {}
    for line in nm_out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] == "T":
            syms[parts[2]] = int(parts[0], 16)
    blob = bin_.read_bytes()
    return blob, syms


# ---------------------------------------------------------------------------
# Step 2: assemble the CreateAccount trampoline + wrapper.
#
# We need to emit machine code at fixed (cave) virtual addresses that depend
# on the final layout. So we pass in `cave_va` and the offsets we want to
# place each fragment at.
# ---------------------------------------------------------------------------
def assemble_createaccount(cave_va, offset_tramp, offset_wrapper):
    ks = keystone.Ks(keystone.KS_ARCH_X86, keystone.KS_MODE_32)

    # Trampoline executes the saved bytes from CreateAccount's first 5 bytes:
    #   55          push ebp
    #   8B EC       mov ebp, esp
    #   53          push ebx
    #   56          push esi
    # then jumps to original + 5.
    tramp_va = cave_va + offset_tramp
    tramp_asm = f"""
        push ebp
        mov  ebp, esp
        push ebx
        push esi
        jmp  0x{VA_CREATE_ACCT_5:08x}
    """
    tramp_bytes, _ = ks.asm(tramp_asm, tramp_va)
    tramp_bytes = bytes(tramp_bytes)

    # Wrapper:
    #   on entry, ECX = this; stack = [caller_retaddr, username, password, a4, a5]
    #   - re-push args for trampoline call
    #   - call trampoline (which preserves __thiscall: ECX still this, ret 0x10)
    #   - save eax (result), copy 16 bytes from password to a4+32
    #   - return result with ret 0x10
    wrapper_va = cave_va + offset_wrapper
    wrapper_asm = f"""
        push ebp
        mov  ebp, esp
        push edi
        push esi
        push ebx
        ; re-push args (right-to-left for stdcall/thiscall)
        push dword ptr [ebp + 0x14]      ; a5
        push dword ptr [ebp + 0x10]      ; a4
        push dword ptr [ebp + 0x0c]      ; password
        push dword ptr [ebp + 0x08]      ; username
        ; ECX still this (we haven't touched it)
        call 0x{tramp_va:08x}
        ; eax = result; trampoline did ret 0x10 so 4 args popped already.
        push eax
        ; copy 16 bytes from password (ebp+0xc) to a4+32 (a4 = ebp+0x10)
        mov  esi, [ebp + 0x0c]
        mov  edi, [ebp + 0x10]
        add  edi, 32
        mov  ecx, 4
        cld
        rep  movsd
        pop  eax
        pop  ebx
        pop  esi
        pop  edi
        leave
        ret  0x10
    """
    wrapper_bytes, _ = ks.asm(wrapper_asm, wrapper_va)
    wrapper_bytes = bytes(wrapper_bytes)
    return tramp_bytes, wrapper_bytes


# ---------------------------------------------------------------------------
# Step 3: PE section append + entry-point patches
# ---------------------------------------------------------------------------

def align_up(n, a):
    return (n + a - 1) // a * a


def patch_pe(src_bytes, cave_blob, file_align, sect_align, image_base):
    """Append `cave_blob` as a new PE section called '.pvpgnh', return
    (new_bytes, cave_va). Caller is responsible for entry-point patches."""
    data = bytearray(src_bytes)

    # Locate PE headers
    e_lfanew = struct.unpack_from("<I", data, 0x3c)[0]
    file_hdr = e_lfanew + 4
    nsec_off = file_hdr + 2
    nsec = struct.unpack_from("<H", data, nsec_off)[0]
    opt_hdr_size = struct.unpack_from("<H", data, file_hdr + 16)[0]
    opt_hdr = file_hdr + 20
    sections_off = opt_hdr + opt_hdr_size

    # Find the highest VA + virtual size of existing sections, and the highest
    # raw end of file. New section starts after both.
    max_vaddr = 0
    max_raw_end = 0
    for i in range(nsec):
        s = sections_off + i * 40
        vsize = struct.unpack_from("<I", data, s + 8)[0]
        vaddr = struct.unpack_from("<I", data, s + 12)[0]
        rsize = struct.unpack_from("<I", data, s + 16)[0]
        raw   = struct.unpack_from("<I", data, s + 20)[0]
        max_vaddr = max(max_vaddr, vaddr + vsize)
        max_raw_end = max(max_raw_end, raw + rsize)

    # Strip the Authenticode signature if present. It lives in IMAGE_DIRECTORY_
    # ENTRY_SECURITY (index 4) of the optional header data directories, which
    # for PE32 starts at opt_hdr + 96 (after standard fields). It's stored as
    # raw data past the last section, NOT inside any section.
    # Directory entries are 8 bytes each (RVA, Size). For PE32:
    #   opt_hdr + 96 + 0..7 = entry 0 (Export)
    #   opt_hdr + 96 + 32..39 = entry 4 (Security)
    sec_dir_off = opt_hdr + 96 + 4 * 8
    sec_dir_va = struct.unpack_from("<I", data, sec_dir_off)[0]
    sec_dir_size = struct.unpack_from("<I", data, sec_dir_off + 4)[0]
    if sec_dir_size > 0:
        # Truncate the file at the start of the signature blob and zero
        # the Security data directory entry.
        if sec_dir_va < len(data):
            del data[sec_dir_va:sec_dir_va + sec_dir_size]
        struct.pack_into("<II", data, sec_dir_off, 0, 0)
        # The CheckSum field at opt_hdr + 64 should also be invalidated/zeroed
        # since the signature is gone.
        struct.pack_into("<I", data, opt_hdr + 64, 0)

    new_vaddr = align_up(max_vaddr, sect_align)
    new_vsize = len(cave_blob)
    new_raw_size = align_up(new_vsize, file_align)
    new_raw_off  = align_up(max(max_raw_end, len(data)), file_align)

    # Verify there's room for one more section header (40 bytes)
    size_of_headers = struct.unpack_from("<I", data, opt_hdr + 60)[0]
    used = sections_off + nsec * 40
    if used + 40 > size_of_headers:
        raise SystemExit(
            f"No room for new section header in headers (used={used:#x}, "
            f"sizeOfHeaders={size_of_headers:#x}). Would need to extend "
            f"SizeOfHeaders, not implemented."
        )

    # Write the new section header
    new_hdr_off = sections_off + nsec * 40
    name = b".pvpgnh\x00"  # exactly 8 bytes
    chars = 0x60000020  # MEM_EXECUTE | MEM_READ | CNT_CODE
    struct.pack_into("<8sIIIIIIHHI", data, new_hdr_off,
                     name, new_vsize, new_vaddr, new_raw_size, new_raw_off,
                     0, 0, 0, 0, chars)

    # Bump NumberOfSections
    struct.pack_into("<H", data, nsec_off, nsec + 1)

    # Update SizeOfImage = new_vaddr + aligned(new_vsize, sect_align)
    old_size_of_image = struct.unpack_from("<I", data, opt_hdr + 56)[0]
    new_size_of_image = align_up(new_vaddr + new_vsize, sect_align)
    if new_size_of_image > old_size_of_image:
        struct.pack_into("<I", data, opt_hdr + 56, new_size_of_image)

    # Pad file to new_raw_off, then append cave_blob padded to new_raw_size
    if len(data) < new_raw_off:
        data.extend(b"\x00" * (new_raw_off - len(data)))
    cave_padded = cave_blob + b"\x00" * (new_raw_size - new_vsize)
    data.extend(cave_padded)

    return bytes(data), image_base + new_vaddr


def patch_entrypoints(data_bytes, lph_va, wrapper_va):
    """Apply auth_req + LphChecked-jmp + CreateAccount-jmp patches.

    Locations (from the original PE — file offsets correspond directly to
    VAs minus 0x401000 + 0x400 since the .text section is mapped that way).
    """
    data = bytearray(data_bytes)

    # File offset = VA - 0x401000 + 0x400
    def to_off(va): return va - 0x00401000 + 0x400

    # 1) auth_req: 0F 85 B8 00 00 00 -> 66 0F 1F 44 00 00
    off = to_off(VA_AUTH_REQ_JNE)
    expected = bytes.fromhex("0F85B8000000")
    cur = bytes(data[off:off+6])
    assert cur == expected, f"auth_req site bytes changed: {cur.hex()}"
    data[off:off+6] = bytes.fromhex("660F1F440000")

    # 1b) M2 dispatch: 74 21 -> 75 20 (JZ short -> JNE short, off-by-one
    #    is intentional - the original target lands on a CS-prefixed push,
    #    the new target lands on the prefix itself which is a no-op).
    #    Without this patch the legacy `hash_eq` server path sends an
    #    all-zero 20-byte M2; the client validates that and disconnects
    #    with "the specified server is invalid".
    off = to_off(VA_M2_DISPATCH)
    expected = bytes.fromhex("7421")
    cur = bytes(data[off:off+2])
    assert cur == expected, f"M2 dispatch site bytes changed: {cur.hex()}"
    data[off:off+2] = bytes.fromhex("7520")

    # 2) LphChecked entry: 5-byte JMP to lph_checked_hook
    off = to_off(VA_LPH_CHECKED)
    expected = bytes.fromhex("558BEC56FF")  # push ebp; mov ebp,esp; push esi; <next>
    cur = bytes(data[off:off+5])
    assert cur == expected, f"LphChecked site bytes changed: {cur.hex()}"
    rel = lph_va - (VA_LPH_CHECKED + 5)
    data[off:off+5] = b"\xE9" + struct.pack("<i", rel)

    # 3) CreateAccount entry: 5-byte JMP to wrapper
    off = to_off(VA_CREATE_ACCT)
    expected = bytes.fromhex("558BEC5356")  # push ebp; mov ebp,esp; push ebx; push esi
    cur = bytes(data[off:off+5])
    assert cur == expected, f"CreateAccount site bytes changed: {cur.hex()}"
    rel = wrapper_va - (VA_CREATE_ACCT + 5)
    data[off:off+5] = b"\xE9" + struct.pack("<i", rel)

    return bytes(data)


def main():
    src = Path(SRC_EXE).read_bytes()
    if hashlib.md5(src).digest() != EXPECTED_MD5:
        raise SystemExit(
            f"Input MD5 mismatch. Expected {EXPECTED_MD5.hex()}; got "
            f"{hashlib.md5(src).hexdigest()}."
        )

    # Compile bnet_hash + lph_checked_hook
    bnet_blob, syms = build_bnet_blob()
    print(f"bnet_hash blob: {len(bnet_blob)} bytes; symbols: {syms}")

    # Read PE headers to determine alignment + image base + where the new
    # section will land.
    e_lfanew = struct.unpack_from("<I", src, 0x3c)[0]
    opt_hdr = e_lfanew + 4 + 20
    image_base = struct.unpack_from("<I", src, opt_hdr + 28)[0]
    sect_align = struct.unpack_from("<I", src, opt_hdr + 32)[0]
    file_align = struct.unpack_from("<I", src, opt_hdr + 36)[0]

    # Compute new section's VA so we can produce relocation-correct asm
    # for the trampoline + wrapper before the actual append.
    nsec = struct.unpack_from("<H", src, e_lfanew + 4 + 2)[0]
    sections_off = opt_hdr + struct.unpack_from("<H", src, e_lfanew + 4 + 16)[0]
    max_vaddr = 0
    for i in range(nsec):
        s = sections_off + i * 40
        vs = struct.unpack_from("<I", src, s + 8)[0]
        va = struct.unpack_from("<I", src, s + 12)[0]
        max_vaddr = max(max_vaddr, va + vs)
    new_section_va = image_base + ((max_vaddr + sect_align - 1) // sect_align * sect_align)

    # Layout the cave:
    #   0x000              bnet_hash   (from .bin)
    #   syms[lph_checked]  lph_checked_hook
    #   align(blob, 16)    trampoline   (24 bytes incl jmp)
    #   tramp + 24, align  wrapper
    blob = bytearray(bnet_blob)
    bnet_offset = 0
    lph_offset = syms["lph_checked_hook"]

    # Pad to 16-byte alignment for the asm fragments
    while len(blob) % 16:
        blob.append(0xCC)
    tramp_offset = len(blob)

    # Pre-assemble to know sizes (we need wrapper to know trampoline VA, so
    # do a placement pass: trampoline first at tramp_offset; wrapper after
    # trampoline at trampoline_offset + ceil(tramp_size, 16))
    tramp_va = new_section_va + tramp_offset

    # First, assemble trampoline (no forward refs)
    ks = keystone.Ks(keystone.KS_ARCH_X86, keystone.KS_MODE_32)
    tramp_asm = f"""
        push ebp
        mov  ebp, esp
        push ebx
        push esi
        jmp  0x{VA_CREATE_ACCT_5:08x}
    """
    tramp_bytes, _ = ks.asm(tramp_asm, tramp_va)
    tramp_bytes = bytes(tramp_bytes)

    # Place wrapper after trampoline (16-byte aligned)
    wrapper_offset = tramp_offset + len(tramp_bytes)
    while wrapper_offset % 16:
        wrapper_offset += 1
    wrapper_va = new_section_va + wrapper_offset

    # Now assemble wrapper with the known trampoline VA
    # Note: keystone doesn't accept 'rep movsd' in this build, so we
    # unroll the 16-byte (4-dword) copy as 4 explicit mov pairs.
    wrapper_asm = f"""
        push ebp
        mov  ebp, esp
        push edi
        push esi
        push ebx
        push dword ptr [ebp + 0x14]
        push dword ptr [ebp + 0x10]
        push dword ptr [ebp + 0x0c]
        push dword ptr [ebp + 0x08]
        call 0x{tramp_va:08x}
        push eax
        mov  esi, [ebp + 0x0c]
        mov  edi, [ebp + 0x10]
        mov  eax, [esi]
        mov  [edi + 0x20], eax
        mov  eax, [esi + 4]
        mov  [edi + 0x24], eax
        mov  eax, [esi + 8]
        mov  [edi + 0x28], eax
        mov  eax, [esi + 12]
        mov  [edi + 0x2c], eax
        pop  eax
        pop  ebx
        pop  esi
        pop  edi
        leave
        ret  0x10
    """
    wrapper_bytes, _ = ks.asm(wrapper_asm, wrapper_va)
    wrapper_bytes = bytes(wrapper_bytes)

    # Assemble final cave
    cave = bytearray(bnet_blob)
    while len(cave) < tramp_offset:
        cave.append(0xCC)
    cave.extend(tramp_bytes)
    while len(cave) < wrapper_offset:
        cave.append(0xCC)
    cave.extend(wrapper_bytes)

    print(f"Cave: {len(cave)} bytes")
    print(f"  bnet_hash             @ +0x{bnet_offset:04x}")
    print(f"  lph_checked_hook      @ +0x{lph_offset:04x}  VA 0x{new_section_va + lph_offset:08x}")
    print(f"  create_account_tramp  @ +0x{tramp_offset:04x}  VA 0x{tramp_va:08x}")
    print(f"  create_account_wrap   @ +0x{wrapper_offset:04x}  VA 0x{wrapper_va:08x}")
    print(f"  new section base       VA 0x{new_section_va:08x}")

    # Append section
    patched, cave_va = patch_pe(src, bytes(cave), file_align, sect_align, image_base)
    assert cave_va == new_section_va, f"VA mismatch {cave_va:08x} != {new_section_va:08x}"

    # Patch entry points
    lph_va = new_section_va + lph_offset
    final = patch_entrypoints(patched, lph_va, wrapper_va)

    Path(DST_EXE).write_bytes(final)
    md5 = hashlib.md5(final).hexdigest()
    print(f"\nWrote: {DST_EXE}")
    print(f"  md5: {md5}")
    print(f"  size: {len(final)} bytes")


if __name__ == "__main__":
    main()
