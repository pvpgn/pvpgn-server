/*
 * PvPGN bnet_hash + LphChecked replacement, packaged for code-cave injection
 * into Warcraft III 1.29.2.9231.
 *
 * Compile with:
 *   gcc -m32 -O2 -fno-pic -fno-stack-protector -ffreestanding -nostdlib \
 *       -fno-asynchronous-unwind-tables -fcf-protection=none \
 *       -fno-jump-tables -mpreferred-stack-boundary=2 \
 *       -c bnet_hash.c -o bnet_hash.o
 *
 * Then extract .text bytes (single contiguous blob) for injection. No external
 * symbols are referenced; internal calls are PC-relative and survive being
 * loaded at any address in the target process.
 *
 * Reference: src/common/bnethash.cpp in pvpgn-server (do_blizzard_hash variant
 * of hash_set_16 + the modified-SHA1 do_hash routine).
 */

typedef unsigned int u32;
typedef unsigned char u8;

static inline u32 ROTL32(u32 v, u32 n) { return (v << n) | (v >> (32 - n)); }

/* Run one 64-byte block through the modified SHA-1 used by classic Battle.net.
 * `state[0..4]` is the running hash, `block16` is the 16 dwords of input
 * (already byte-packed in PvPGN's "blizzard" little-endian order). */
static void do_hash_block(u32 *state, const u32 *block16)
{
    u32 tmp[64 + 16];
    int i;

    /* Copy initial 16 dwords */
    for (i = 0; i < 16; i++) tmp[i] = block16[i];

    /* Expansion - the "blizzard" variant: ROTL32(1, x ^ y ^ z ^ w)
     * (note: ROL of constant 1, not of the XOR result like SHA-1 proper) */
    for (i = 0; i < 64; i++) {
        u32 x = tmp[i] ^ tmp[i + 8] ^ tmp[i + 2] ^ tmp[i + 13];
        tmp[i + 16] = ROTL32(1, x & 31);
    }

    u32 a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
    u32 g = 0;

    for (i = 0; i < 20; i++) {
        g = tmp[i] + ROTL32(a, 5) + e + ((b & c) | (~b & d)) + 0x5a827999u;
        e = d; d = c; c = ROTL32(b, 30); b = a; a = g;
    }
    for (i = 20; i < 40; i++) {
        g = (d ^ c ^ b) + e + ROTL32(g, 5) + tmp[i] + 0x6ed9eba1u;
        e = d; d = c; c = ROTL32(b, 30); b = a; a = g;
    }
    for (i = 40; i < 60; i++) {
        g = tmp[i] + ROTL32(g, 5) + e + ((c & b) | (d & c) | (d & b)) - 0x70e44324u;
        e = d; d = c; c = ROTL32(b, 30); b = a; a = g;
    }
    for (i = 60; i < 80; i++) {
        g = (d ^ c ^ b) + e + ROTL32(g, 5) + tmp[i] - 0x359d3e2au;
        e = d; d = c; c = ROTL32(b, 30); b = a; a = g;
    }

    state[0] += g;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

/* PvPGN's hash_set_16 with do_blizzard_hash variant: pack `count` bytes from
 * `src` into 16 little-endian dwords, zero-pad the rest. */
static void hash_set_16(u32 *dst, const u8 *src, unsigned count)
{
    int i;
    unsigned pos = 0;
    for (i = 0; i < 16; i++) {
        u32 v = 0;
        if (pos < count) v |= ((u32)src[pos]);
        pos++;
        if (pos < count) v |= ((u32)src[pos]) << 8;
        pos++;
        if (pos < count) v |= ((u32)src[pos]) << 16;
        pos++;
        if (pos < count) v |= ((u32)src[pos]) << 24;
        pos++;
        dst[i] = v;
    }
}

/* PvPGN's bnet_hash: SHA-1-init state, hash 64-byte blocks of input until
 * exhausted (no length suffix or padding standard SHA-1 has). Output is 5
 * dwords in `out`. */
void bnet_hash(u32 *out, const u8 *data, unsigned size)
{
    u32 block[16];

    out[0] = 0x67452301u;
    out[1] = 0xefcdab89u;
    out[2] = 0x98badcfeu;
    out[3] = 0x10325476u;
    out[4] = 0xc3d2e1f0u;

    while (size > 0) {
        unsigned inc = (size > 64) ? 64 : size;
        hash_set_16(block, data, inc);
        do_hash_block(out, block);
        data += inc;
        size -= inc;
    }
}

/* LphChecked replacement.
 *
 * Original at VA 0x0083A590 in 1.29.2.9231:
 *     __cdecl int LphChecked(byte *out, char *ctx, int a3, int a4)
 *     - out = 20-byte output buffer for the password proof
 *     - ctx + 32 = ASCII null-terminated password
 *     - a3, a4 = ignored
 *     - returns 1 on success
 *
 * Per W3CE/PvPGNPatcher: lowercase the password, run bnet_hash, write the
 * 5-dword digest to `out`, return 1.
 *
 * Note: `__attribute__((cdecl))` is the default x86 calling convention,
 * but spelling it explicit so the compiled output is unambiguous.
 */
int __attribute__((cdecl)) lph_checked_hook(u8 *out, char *ctx, int a3, int a4)
{
    char buf[64];
    int n = 0;
    char *p = ctx + 32;

    /* Lowercase ASCII; stop at NUL or buffer limit. */
    while (n < (int)sizeof(buf) - 1) {
        char c = p[n];
        if (c == 0) break;
        if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
        buf[n++] = c;
    }

    u32 digest[5];
    bnet_hash(digest, (u8 *)buf, (unsigned)n);

    /* Copy 20 bytes (5 dwords). Write as dwords for compactness. */
    u32 *dout = (u32 *)out;
    dout[0] = digest[0];
    dout[1] = digest[1];
    dout[2] = digest[2];
    dout[3] = digest[3];
    dout[4] = digest[4];

    (void)a3; (void)a4;
    return 1;
}
