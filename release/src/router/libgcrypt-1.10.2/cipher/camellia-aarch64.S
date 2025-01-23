/* camellia-aarch64.S  -  ARMv8/AArch64 assembly implementation of Camellia
 *                        cipher
 *
 * Copyright (C) 2016 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "asm-common-aarch64.h"

#if defined(__AARCH64EL__)
#ifdef HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS

.text

/* struct camellia_ctx: */
#define key_table 0

/* register macros */
#define CTX x0
#define RDST x1
#define RSRC x2
#define RKEYBITS w3

#define RTAB1 x4
#define RTAB2 x5
#define RTAB3 x6
#define RTAB4 x7
#define RMASK w8

#define IL w9
#define IR w10

#define xIL x9
#define xIR x10

#define XL w11
#define XR w12
#define YL w13
#define YR w14

#define RT0 w15
#define RT1 w16
#define RT2 w17
#define RT3 w19

#define xRT0 x15
#define xRT1 x16
#define xRT2 x17
#define xRT3 x19

#ifdef __AARCH64EL__
  #define host_to_be(reg, rtmp) \
	  rev reg, reg;
  #define be_to_host(reg, rtmp) \
	  rev reg, reg;
#else
  /* nop on big-endian */
  #define host_to_be(reg, rtmp) /*_*/
  #define be_to_host(reg, rtmp) /*_*/
#endif

#define ldr_input_aligned_be(rin, a, b, c, d, rtmp) \
	ldr a, [rin, #0]; \
	ldr b, [rin, #4]; \
	be_to_host(a, rtmp); \
	ldr c, [rin, #8]; \
	be_to_host(b, rtmp); \
	ldr d, [rin, #12]; \
	be_to_host(c, rtmp); \
	be_to_host(d, rtmp);

#define str_output_aligned_be(rout, a, b, c, d, rtmp) \
	be_to_host(a, rtmp); \
	be_to_host(b, rtmp); \
	str a, [rout, #0]; \
	be_to_host(c, rtmp); \
	str b, [rout, #4]; \
	be_to_host(d, rtmp); \
	str c, [rout, #8]; \
	str d, [rout, #12];

/* unaligned word reads/writes allowed */
#define ldr_input_be(rin, ra, rb, rc, rd, rtmp) \
	ldr_input_aligned_be(rin, ra, rb, rc, rd, rtmp)

#define str_output_be(rout, ra, rb, rc, rd, rtmp0, rtmp1) \
	str_output_aligned_be(rout, ra, rb, rc, rd, rtmp0)

/**********************************************************************
  1-way camellia
 **********************************************************************/
#define roundsm(xl, xr, kl, kr, yl, yr) \
	ldr RT2, [CTX, #(key_table + ((kl) * 4))]; \
	and  IR, RMASK, xr, lsl#(4);      /*sp1110*/ \
	ldr RT3, [CTX, #(key_table + ((kr) * 4))]; \
	and  IL, RMASK, xl, lsr#(24 - 4); /*sp1110*/ \
	and RT0, RMASK, xr, lsr#(16 - 4); /*sp3033*/ \
	ldr  IR, [RTAB1,  xIR]; \
	and RT1, RMASK, xl, lsr#(8 - 4);  /*sp3033*/ \
	eor yl, yl, RT2; \
	ldr  IL, [RTAB1,  xIL]; \
	eor yr, yr, RT3; \
	\
	ldr RT0, [RTAB3, xRT0]; \
	ldr RT1, [RTAB3, xRT1]; \
	\
	and RT2, RMASK, xr, lsr#(24 - 4); /*sp0222*/ \
	and RT3, RMASK, xl, lsr#(16 - 4); /*sp0222*/ \
	\
	eor IR, IR, RT0; \
	eor IL, IL, RT1; \
	\
	ldr RT2, [RTAB2, xRT2]; \
	and RT0, RMASK, xr, lsr#(8 - 4);  /*sp4404*/ \
	ldr RT3, [RTAB2, xRT3]; \
	and RT1, RMASK, xl, lsl#(4);      /*sp4404*/ \
	\
	ldr RT0, [RTAB4, xRT0]; \
	ldr RT1, [RTAB4, xRT1]; \
	\
	eor IR, IR, RT2; \
	eor IL, IL, RT3; \
	eor IR, IR, RT0; \
	eor IL, IL, RT1; \
	\
	eor IR, IR, IL; \
	eor yr, yr, IL, ror#8; \
	eor yl, yl, IR; \
	eor yr, yr, IR;

#define enc_rounds(n) \
	roundsm(XL, XR, ((n) + 2) * 2 + 0, ((n) + 2) * 2 + 1, YL, YR); \
	roundsm(YL, YR, ((n) + 3) * 2 + 0, ((n) + 3) * 2 + 1, XL, XR); \
	roundsm(XL, XR, ((n) + 4) * 2 + 0, ((n) + 4) * 2 + 1, YL, YR); \
	roundsm(YL, YR, ((n) + 5) * 2 + 0, ((n) + 5) * 2 + 1, XL, XR); \
	roundsm(XL, XR, ((n) + 6) * 2 + 0, ((n) + 6) * 2 + 1, YL, YR); \
	roundsm(YL, YR, ((n) + 7) * 2 + 0, ((n) + 7) * 2 + 1, XL, XR);

#define dec_rounds(n) \
	roundsm(XL, XR, ((n) + 7) * 2 + 0, ((n) + 7) * 2 + 1, YL, YR); \
	roundsm(YL, YR, ((n) + 6) * 2 + 0, ((n) + 6) * 2 + 1, XL, XR); \
	roundsm(XL, XR, ((n) + 5) * 2 + 0, ((n) + 5) * 2 + 1, YL, YR); \
	roundsm(YL, YR, ((n) + 4) * 2 + 0, ((n) + 4) * 2 + 1, XL, XR); \
	roundsm(XL, XR, ((n) + 3) * 2 + 0, ((n) + 3) * 2 + 1, YL, YR); \
	roundsm(YL, YR, ((n) + 2) * 2 + 0, ((n) + 2) * 2 + 1, XL, XR);

/* perform FL and FL⁻¹ */
#define fls(ll, lr, rl, rr, kll, klr, krl, krr) \
	ldr RT0, [CTX, #(key_table + ((kll) * 4))]; \
	ldr RT2, [CTX, #(key_table + ((krr) * 4))]; \
	and RT0, RT0, ll; \
	ldr RT3, [CTX, #(key_table + ((krl) * 4))]; \
	orr RT2, RT2, rr; \
	ldr RT1, [CTX, #(key_table + ((klr) * 4))]; \
	eor rl, rl, RT2; \
	eor lr, lr, RT0, ror#31; \
	and RT3, RT3, rl; \
	orr RT1, RT1, lr; \
	eor ll, ll, RT1; \
	eor rr, rr, RT3, ror#31;

#define enc_fls(n) \
	fls(XL, XR, YL, YR, \
	    (n) * 2 + 0, (n) * 2 + 1, \
	    (n) * 2 + 2, (n) * 2 + 3);

#define dec_fls(n) \
	fls(XL, XR, YL, YR, \
	    (n) * 2 + 2, (n) * 2 + 3, \
	    (n) * 2 + 0, (n) * 2 + 1);

#define inpack(n) \
	ldr_input_be(RSRC, XL, XR, YL, YR, RT0); \
	ldr RT0, [CTX, #(key_table + ((n) * 8) + 0)]; \
	ldr RT1, [CTX, #(key_table + ((n) * 8) + 4)]; \
	eor XL, XL, RT0; \
	eor XR, XR, RT1;

#define outunpack(n) \
	ldr RT0, [CTX, #(key_table + ((n) * 8) + 0)]; \
	ldr RT1, [CTX, #(key_table + ((n) * 8) + 4)]; \
	eor YL, YL, RT0; \
	eor YR, YR, RT1; \
	str_output_be(RDST, YL, YR, XL, XR, RT0, RT1);

.globl _gcry_camellia_arm_encrypt_block
ELF(.type   _gcry_camellia_arm_encrypt_block,@function;)

_gcry_camellia_arm_encrypt_block:
	CFI_STARTPROC()
	stp x19, x30, [sp, #-16]!
	CFI_ADJUST_CFA_OFFSET(16)
	CFI_REG_ON_STACK(19, 0)
	CFI_REG_ON_STACK(30, 8)

	/* input:
	 *	x0: keytable
	 *	x1: dst
	 *	x2: src
	 *	w3: keybitlen
	 */

	adr RTAB1,  _gcry_camellia_arm_tables;
	mov RMASK, #(0xff<<4); /* byte mask */
	add RTAB2, RTAB1, #(1 * 4);
	add RTAB3, RTAB1, #(2 * 4);
	add RTAB4, RTAB1, #(3 * 4);

	inpack(0);

	enc_rounds(0);
	enc_fls(8);
	enc_rounds(8);
	enc_fls(16);
	enc_rounds(16);

	cmp RKEYBITS, #(16 * 8);
	bne .Lenc_256;

	outunpack(24);

	CFI_REMEMBER_STATE()
	ldp x19, x30, [sp], #16
	CFI_ADJUST_CFA_OFFSET(-16)
	CFI_RESTORE(x19)
	CFI_RESTORE(x30)
	ret_spec_stop;
	CFI_RESTORE_STATE()
.ltorg

.Lenc_256:
	enc_fls(24);
	enc_rounds(24);

	outunpack(32);

	ldp x19, x30, [sp], #16
	CFI_ADJUST_CFA_OFFSET(-16)
	CFI_RESTORE(x19)
	CFI_RESTORE(x30)
	ret_spec_stop;
	CFI_ENDPROC()
.ltorg
ELF(.size _gcry_camellia_arm_encrypt_block,.-_gcry_camellia_arm_encrypt_block;)

.globl _gcry_camellia_arm_decrypt_block
ELF(.type   _gcry_camellia_arm_decrypt_block,@function;)

_gcry_camellia_arm_decrypt_block:
	CFI_STARTPROC()
	stp x19, x30, [sp, #-16]!
	CFI_ADJUST_CFA_OFFSET(16)
	CFI_REG_ON_STACK(19, 0)
	CFI_REG_ON_STACK(30, 8)

	/* input:
	 *	x0: keytable
	 *	x1: dst
	 *	x2: src
	 *	w3: keybitlen
	 */

	adr RTAB1,  _gcry_camellia_arm_tables;
	mov RMASK, #(0xff<<4); /* byte mask */
	add RTAB2, RTAB1, #(1 * 4);
	add RTAB3, RTAB1, #(2 * 4);
	add RTAB4, RTAB1, #(3 * 4);

	cmp RKEYBITS, #(16 * 8);
	bne .Ldec_256;

	inpack(24);

.Ldec_128:
	dec_rounds(16);
	dec_fls(16);
	dec_rounds(8);
	dec_fls(8);
	dec_rounds(0);

	outunpack(0);

	CFI_REMEMBER_STATE()
	ldp x19, x30, [sp], #16
	CFI_ADJUST_CFA_OFFSET(-16)
	CFI_RESTORE(x19)
	CFI_RESTORE(x30)
	ret_spec_stop;
	CFI_RESTORE_STATE()
.ltorg

.Ldec_256:
	inpack(32);
	dec_rounds(24);
	dec_fls(24);

	b .Ldec_128;
	CFI_ENDPROC()
.ltorg
ELF(.size _gcry_camellia_arm_decrypt_block,.-_gcry_camellia_arm_decrypt_block;)

/* Encryption/Decryption tables */
ELF(.type  _gcry_camellia_arm_tables,@object;)
.balign 32
_gcry_camellia_arm_tables:
.Lcamellia_sp1110:
.long 0x70707000
.Lcamellia_sp0222:
            .long 0x00e0e0e0
.Lcamellia_sp3033:
                        .long 0x38003838
.Lcamellia_sp4404:
                                    .long 0x70700070
.long 0x82828200, 0x00050505, 0x41004141, 0x2c2c002c
.long 0x2c2c2c00, 0x00585858, 0x16001616, 0xb3b300b3
.long 0xececec00, 0x00d9d9d9, 0x76007676, 0xc0c000c0
.long 0xb3b3b300, 0x00676767, 0xd900d9d9, 0xe4e400e4
.long 0x27272700, 0x004e4e4e, 0x93009393, 0x57570057
.long 0xc0c0c000, 0x00818181, 0x60006060, 0xeaea00ea
.long 0xe5e5e500, 0x00cbcbcb, 0xf200f2f2, 0xaeae00ae
.long 0xe4e4e400, 0x00c9c9c9, 0x72007272, 0x23230023
.long 0x85858500, 0x000b0b0b, 0xc200c2c2, 0x6b6b006b
.long 0x57575700, 0x00aeaeae, 0xab00abab, 0x45450045
.long 0x35353500, 0x006a6a6a, 0x9a009a9a, 0xa5a500a5
.long 0xeaeaea00, 0x00d5d5d5, 0x75007575, 0xeded00ed
.long 0x0c0c0c00, 0x00181818, 0x06000606, 0x4f4f004f
.long 0xaeaeae00, 0x005d5d5d, 0x57005757, 0x1d1d001d
.long 0x41414100, 0x00828282, 0xa000a0a0, 0x92920092
.long 0x23232300, 0x00464646, 0x91009191, 0x86860086
.long 0xefefef00, 0x00dfdfdf, 0xf700f7f7, 0xafaf00af
.long 0x6b6b6b00, 0x00d6d6d6, 0xb500b5b5, 0x7c7c007c
.long 0x93939300, 0x00272727, 0xc900c9c9, 0x1f1f001f
.long 0x45454500, 0x008a8a8a, 0xa200a2a2, 0x3e3e003e
.long 0x19191900, 0x00323232, 0x8c008c8c, 0xdcdc00dc
.long 0xa5a5a500, 0x004b4b4b, 0xd200d2d2, 0x5e5e005e
.long 0x21212100, 0x00424242, 0x90009090, 0x0b0b000b
.long 0xededed00, 0x00dbdbdb, 0xf600f6f6, 0xa6a600a6
.long 0x0e0e0e00, 0x001c1c1c, 0x07000707, 0x39390039
.long 0x4f4f4f00, 0x009e9e9e, 0xa700a7a7, 0xd5d500d5
.long 0x4e4e4e00, 0x009c9c9c, 0x27002727, 0x5d5d005d
.long 0x1d1d1d00, 0x003a3a3a, 0x8e008e8e, 0xd9d900d9
.long 0x65656500, 0x00cacaca, 0xb200b2b2, 0x5a5a005a
.long 0x92929200, 0x00252525, 0x49004949, 0x51510051
.long 0xbdbdbd00, 0x007b7b7b, 0xde00dede, 0x6c6c006c
.long 0x86868600, 0x000d0d0d, 0x43004343, 0x8b8b008b
.long 0xb8b8b800, 0x00717171, 0x5c005c5c, 0x9a9a009a
.long 0xafafaf00, 0x005f5f5f, 0xd700d7d7, 0xfbfb00fb
.long 0x8f8f8f00, 0x001f1f1f, 0xc700c7c7, 0xb0b000b0
.long 0x7c7c7c00, 0x00f8f8f8, 0x3e003e3e, 0x74740074
.long 0xebebeb00, 0x00d7d7d7, 0xf500f5f5, 0x2b2b002b
.long 0x1f1f1f00, 0x003e3e3e, 0x8f008f8f, 0xf0f000f0
.long 0xcecece00, 0x009d9d9d, 0x67006767, 0x84840084
.long 0x3e3e3e00, 0x007c7c7c, 0x1f001f1f, 0xdfdf00df
.long 0x30303000, 0x00606060, 0x18001818, 0xcbcb00cb
.long 0xdcdcdc00, 0x00b9b9b9, 0x6e006e6e, 0x34340034
.long 0x5f5f5f00, 0x00bebebe, 0xaf00afaf, 0x76760076
.long 0x5e5e5e00, 0x00bcbcbc, 0x2f002f2f, 0x6d6d006d
.long 0xc5c5c500, 0x008b8b8b, 0xe200e2e2, 0xa9a900a9
.long 0x0b0b0b00, 0x00161616, 0x85008585, 0xd1d100d1
.long 0x1a1a1a00, 0x00343434, 0x0d000d0d, 0x04040004
.long 0xa6a6a600, 0x004d4d4d, 0x53005353, 0x14140014
.long 0xe1e1e100, 0x00c3c3c3, 0xf000f0f0, 0x3a3a003a
.long 0x39393900, 0x00727272, 0x9c009c9c, 0xdede00de
.long 0xcacaca00, 0x00959595, 0x65006565, 0x11110011
.long 0xd5d5d500, 0x00ababab, 0xea00eaea, 0x32320032
.long 0x47474700, 0x008e8e8e, 0xa300a3a3, 0x9c9c009c
.long 0x5d5d5d00, 0x00bababa, 0xae00aeae, 0x53530053
.long 0x3d3d3d00, 0x007a7a7a, 0x9e009e9e, 0xf2f200f2
.long 0xd9d9d900, 0x00b3b3b3, 0xec00ecec, 0xfefe00fe
.long 0x01010100, 0x00020202, 0x80008080, 0xcfcf00cf
.long 0x5a5a5a00, 0x00b4b4b4, 0x2d002d2d, 0xc3c300c3
.long 0xd6d6d600, 0x00adadad, 0x6b006b6b, 0x7a7a007a
.long 0x51515100, 0x00a2a2a2, 0xa800a8a8, 0x24240024
.long 0x56565600, 0x00acacac, 0x2b002b2b, 0xe8e800e8
.long 0x6c6c6c00, 0x00d8d8d8, 0x36003636, 0x60600060
.long 0x4d4d4d00, 0x009a9a9a, 0xa600a6a6, 0x69690069
.long 0x8b8b8b00, 0x00171717, 0xc500c5c5, 0xaaaa00aa
.long 0x0d0d0d00, 0x001a1a1a, 0x86008686, 0xa0a000a0
.long 0x9a9a9a00, 0x00353535, 0x4d004d4d, 0xa1a100a1
.long 0x66666600, 0x00cccccc, 0x33003333, 0x62620062
.long 0xfbfbfb00, 0x00f7f7f7, 0xfd00fdfd, 0x54540054
.long 0xcccccc00, 0x00999999, 0x66006666, 0x1e1e001e
.long 0xb0b0b000, 0x00616161, 0x58005858, 0xe0e000e0
.long 0x2d2d2d00, 0x005a5a5a, 0x96009696, 0x64640064
.long 0x74747400, 0x00e8e8e8, 0x3a003a3a, 0x10100010
.long 0x12121200, 0x00242424, 0x09000909, 0x00000000
.long 0x2b2b2b00, 0x00565656, 0x95009595, 0xa3a300a3
.long 0x20202000, 0x00404040, 0x10001010, 0x75750075
.long 0xf0f0f000, 0x00e1e1e1, 0x78007878, 0x8a8a008a
.long 0xb1b1b100, 0x00636363, 0xd800d8d8, 0xe6e600e6
.long 0x84848400, 0x00090909, 0x42004242, 0x09090009
.long 0x99999900, 0x00333333, 0xcc00cccc, 0xdddd00dd
.long 0xdfdfdf00, 0x00bfbfbf, 0xef00efef, 0x87870087
.long 0x4c4c4c00, 0x00989898, 0x26002626, 0x83830083
.long 0xcbcbcb00, 0x00979797, 0xe500e5e5, 0xcdcd00cd
.long 0xc2c2c200, 0x00858585, 0x61006161, 0x90900090
.long 0x34343400, 0x00686868, 0x1a001a1a, 0x73730073
.long 0x7e7e7e00, 0x00fcfcfc, 0x3f003f3f, 0xf6f600f6
.long 0x76767600, 0x00ececec, 0x3b003b3b, 0x9d9d009d
.long 0x05050500, 0x000a0a0a, 0x82008282, 0xbfbf00bf
.long 0x6d6d6d00, 0x00dadada, 0xb600b6b6, 0x52520052
.long 0xb7b7b700, 0x006f6f6f, 0xdb00dbdb, 0xd8d800d8
.long 0xa9a9a900, 0x00535353, 0xd400d4d4, 0xc8c800c8
.long 0x31313100, 0x00626262, 0x98009898, 0xc6c600c6
.long 0xd1d1d100, 0x00a3a3a3, 0xe800e8e8, 0x81810081
.long 0x17171700, 0x002e2e2e, 0x8b008b8b, 0x6f6f006f
.long 0x04040400, 0x00080808, 0x02000202, 0x13130013
.long 0xd7d7d700, 0x00afafaf, 0xeb00ebeb, 0x63630063
.long 0x14141400, 0x00282828, 0x0a000a0a, 0xe9e900e9
.long 0x58585800, 0x00b0b0b0, 0x2c002c2c, 0xa7a700a7
.long 0x3a3a3a00, 0x00747474, 0x1d001d1d, 0x9f9f009f
.long 0x61616100, 0x00c2c2c2, 0xb000b0b0, 0xbcbc00bc
.long 0xdedede00, 0x00bdbdbd, 0x6f006f6f, 0x29290029
.long 0x1b1b1b00, 0x00363636, 0x8d008d8d, 0xf9f900f9
.long 0x11111100, 0x00222222, 0x88008888, 0x2f2f002f
.long 0x1c1c1c00, 0x00383838, 0x0e000e0e, 0xb4b400b4
.long 0x32323200, 0x00646464, 0x19001919, 0x78780078
.long 0x0f0f0f00, 0x001e1e1e, 0x87008787, 0x06060006
.long 0x9c9c9c00, 0x00393939, 0x4e004e4e, 0xe7e700e7
.long 0x16161600, 0x002c2c2c, 0x0b000b0b, 0x71710071
.long 0x53535300, 0x00a6a6a6, 0xa900a9a9, 0xd4d400d4
.long 0x18181800, 0x00303030, 0x0c000c0c, 0xabab00ab
.long 0xf2f2f200, 0x00e5e5e5, 0x79007979, 0x88880088
.long 0x22222200, 0x00444444, 0x11001111, 0x8d8d008d
.long 0xfefefe00, 0x00fdfdfd, 0x7f007f7f, 0x72720072
.long 0x44444400, 0x00888888, 0x22002222, 0xb9b900b9
.long 0xcfcfcf00, 0x009f9f9f, 0xe700e7e7, 0xf8f800f8
.long 0xb2b2b200, 0x00656565, 0x59005959, 0xacac00ac
.long 0xc3c3c300, 0x00878787, 0xe100e1e1, 0x36360036
.long 0xb5b5b500, 0x006b6b6b, 0xda00dada, 0x2a2a002a
.long 0x7a7a7a00, 0x00f4f4f4, 0x3d003d3d, 0x3c3c003c
.long 0x91919100, 0x00232323, 0xc800c8c8, 0xf1f100f1
.long 0x24242400, 0x00484848, 0x12001212, 0x40400040
.long 0x08080800, 0x00101010, 0x04000404, 0xd3d300d3
.long 0xe8e8e800, 0x00d1d1d1, 0x74007474, 0xbbbb00bb
.long 0xa8a8a800, 0x00515151, 0x54005454, 0x43430043
.long 0x60606000, 0x00c0c0c0, 0x30003030, 0x15150015
.long 0xfcfcfc00, 0x00f9f9f9, 0x7e007e7e, 0xadad00ad
.long 0x69696900, 0x00d2d2d2, 0xb400b4b4, 0x77770077
.long 0x50505000, 0x00a0a0a0, 0x28002828, 0x80800080
.long 0xaaaaaa00, 0x00555555, 0x55005555, 0x82820082
.long 0xd0d0d000, 0x00a1a1a1, 0x68006868, 0xecec00ec
.long 0xa0a0a000, 0x00414141, 0x50005050, 0x27270027
.long 0x7d7d7d00, 0x00fafafa, 0xbe00bebe, 0xe5e500e5
.long 0xa1a1a100, 0x00434343, 0xd000d0d0, 0x85850085
.long 0x89898900, 0x00131313, 0xc400c4c4, 0x35350035
.long 0x62626200, 0x00c4c4c4, 0x31003131, 0x0c0c000c
.long 0x97979700, 0x002f2f2f, 0xcb00cbcb, 0x41410041
.long 0x54545400, 0x00a8a8a8, 0x2a002a2a, 0xefef00ef
.long 0x5b5b5b00, 0x00b6b6b6, 0xad00adad, 0x93930093
.long 0x1e1e1e00, 0x003c3c3c, 0x0f000f0f, 0x19190019
.long 0x95959500, 0x002b2b2b, 0xca00caca, 0x21210021
.long 0xe0e0e000, 0x00c1c1c1, 0x70007070, 0x0e0e000e
.long 0xffffff00, 0x00ffffff, 0xff00ffff, 0x4e4e004e
.long 0x64646400, 0x00c8c8c8, 0x32003232, 0x65650065
.long 0xd2d2d200, 0x00a5a5a5, 0x69006969, 0xbdbd00bd
.long 0x10101000, 0x00202020, 0x08000808, 0xb8b800b8
.long 0xc4c4c400, 0x00898989, 0x62006262, 0x8f8f008f
.long 0x00000000, 0x00000000, 0x00000000, 0xebeb00eb
.long 0x48484800, 0x00909090, 0x24002424, 0xcece00ce
.long 0xa3a3a300, 0x00474747, 0xd100d1d1, 0x30300030
.long 0xf7f7f700, 0x00efefef, 0xfb00fbfb, 0x5f5f005f
.long 0x75757500, 0x00eaeaea, 0xba00baba, 0xc5c500c5
.long 0xdbdbdb00, 0x00b7b7b7, 0xed00eded, 0x1a1a001a
.long 0x8a8a8a00, 0x00151515, 0x45004545, 0xe1e100e1
.long 0x03030300, 0x00060606, 0x81008181, 0xcaca00ca
.long 0xe6e6e600, 0x00cdcdcd, 0x73007373, 0x47470047
.long 0xdadada00, 0x00b5b5b5, 0x6d006d6d, 0x3d3d003d
.long 0x09090900, 0x00121212, 0x84008484, 0x01010001
.long 0x3f3f3f00, 0x007e7e7e, 0x9f009f9f, 0xd6d600d6
.long 0xdddddd00, 0x00bbbbbb, 0xee00eeee, 0x56560056
.long 0x94949400, 0x00292929, 0x4a004a4a, 0x4d4d004d
.long 0x87878700, 0x000f0f0f, 0xc300c3c3, 0x0d0d000d
.long 0x5c5c5c00, 0x00b8b8b8, 0x2e002e2e, 0x66660066
.long 0x83838300, 0x00070707, 0xc100c1c1, 0xcccc00cc
.long 0x02020200, 0x00040404, 0x01000101, 0x2d2d002d
.long 0xcdcdcd00, 0x009b9b9b, 0xe600e6e6, 0x12120012
.long 0x4a4a4a00, 0x00949494, 0x25002525, 0x20200020
.long 0x90909000, 0x00212121, 0x48004848, 0xb1b100b1
.long 0x33333300, 0x00666666, 0x99009999, 0x99990099
.long 0x73737300, 0x00e6e6e6, 0xb900b9b9, 0x4c4c004c
.long 0x67676700, 0x00cecece, 0xb300b3b3, 0xc2c200c2
.long 0xf6f6f600, 0x00ededed, 0x7b007b7b, 0x7e7e007e
.long 0xf3f3f300, 0x00e7e7e7, 0xf900f9f9, 0x05050005
.long 0x9d9d9d00, 0x003b3b3b, 0xce00cece, 0xb7b700b7
.long 0x7f7f7f00, 0x00fefefe, 0xbf00bfbf, 0x31310031
.long 0xbfbfbf00, 0x007f7f7f, 0xdf00dfdf, 0x17170017
.long 0xe2e2e200, 0x00c5c5c5, 0x71007171, 0xd7d700d7
.long 0x52525200, 0x00a4a4a4, 0x29002929, 0x58580058
.long 0x9b9b9b00, 0x00373737, 0xcd00cdcd, 0x61610061
.long 0xd8d8d800, 0x00b1b1b1, 0x6c006c6c, 0x1b1b001b
.long 0x26262600, 0x004c4c4c, 0x13001313, 0x1c1c001c
.long 0xc8c8c800, 0x00919191, 0x64006464, 0x0f0f000f
.long 0x37373700, 0x006e6e6e, 0x9b009b9b, 0x16160016
.long 0xc6c6c600, 0x008d8d8d, 0x63006363, 0x18180018
.long 0x3b3b3b00, 0x00767676, 0x9d009d9d, 0x22220022
.long 0x81818100, 0x00030303, 0xc000c0c0, 0x44440044
.long 0x96969600, 0x002d2d2d, 0x4b004b4b, 0xb2b200b2
.long 0x6f6f6f00, 0x00dedede, 0xb700b7b7, 0xb5b500b5
.long 0x4b4b4b00, 0x00969696, 0xa500a5a5, 0x91910091
.long 0x13131300, 0x00262626, 0x89008989, 0x08080008
.long 0xbebebe00, 0x007d7d7d, 0x5f005f5f, 0xa8a800a8
.long 0x63636300, 0x00c6c6c6, 0xb100b1b1, 0xfcfc00fc
.long 0x2e2e2e00, 0x005c5c5c, 0x17001717, 0x50500050
.long 0xe9e9e900, 0x00d3d3d3, 0xf400f4f4, 0xd0d000d0
.long 0x79797900, 0x00f2f2f2, 0xbc00bcbc, 0x7d7d007d
.long 0xa7a7a700, 0x004f4f4f, 0xd300d3d3, 0x89890089
.long 0x8c8c8c00, 0x00191919, 0x46004646, 0x97970097
.long 0x9f9f9f00, 0x003f3f3f, 0xcf00cfcf, 0x5b5b005b
.long 0x6e6e6e00, 0x00dcdcdc, 0x37003737, 0x95950095
.long 0xbcbcbc00, 0x00797979, 0x5e005e5e, 0xffff00ff
.long 0x8e8e8e00, 0x001d1d1d, 0x47004747, 0xd2d200d2
.long 0x29292900, 0x00525252, 0x94009494, 0xc4c400c4
.long 0xf5f5f500, 0x00ebebeb, 0xfa00fafa, 0x48480048
.long 0xf9f9f900, 0x00f3f3f3, 0xfc00fcfc, 0xf7f700f7
.long 0xb6b6b600, 0x006d6d6d, 0x5b005b5b, 0xdbdb00db
.long 0x2f2f2f00, 0x005e5e5e, 0x97009797, 0x03030003
.long 0xfdfdfd00, 0x00fbfbfb, 0xfe00fefe, 0xdada00da
.long 0xb4b4b400, 0x00696969, 0x5a005a5a, 0x3f3f003f
.long 0x59595900, 0x00b2b2b2, 0xac00acac, 0x94940094
.long 0x78787800, 0x00f0f0f0, 0x3c003c3c, 0x5c5c005c
.long 0x98989800, 0x00313131, 0x4c004c4c, 0x02020002
.long 0x06060600, 0x000c0c0c, 0x03000303, 0x4a4a004a
.long 0x6a6a6a00, 0x00d4d4d4, 0x35003535, 0x33330033
.long 0xe7e7e700, 0x00cfcfcf, 0xf300f3f3, 0x67670067
.long 0x46464600, 0x008c8c8c, 0x23002323, 0xf3f300f3
.long 0x71717100, 0x00e2e2e2, 0xb800b8b8, 0x7f7f007f
.long 0xbababa00, 0x00757575, 0x5d005d5d, 0xe2e200e2
.long 0xd4d4d400, 0x00a9a9a9, 0x6a006a6a, 0x9b9b009b
.long 0x25252500, 0x004a4a4a, 0x92009292, 0x26260026
.long 0xababab00, 0x00575757, 0xd500d5d5, 0x37370037
.long 0x42424200, 0x00848484, 0x21002121, 0x3b3b003b
.long 0x88888800, 0x00111111, 0x44004444, 0x96960096
.long 0xa2a2a200, 0x00454545, 0x51005151, 0x4b4b004b
.long 0x8d8d8d00, 0x001b1b1b, 0xc600c6c6, 0xbebe00be
.long 0xfafafa00, 0x00f5f5f5, 0x7d007d7d, 0x2e2e002e
.long 0x72727200, 0x00e4e4e4, 0x39003939, 0x79790079
.long 0x07070700, 0x000e0e0e, 0x83008383, 0x8c8c008c
.long 0xb9b9b900, 0x00737373, 0xdc00dcdc, 0x6e6e006e
.long 0x55555500, 0x00aaaaaa, 0xaa00aaaa, 0x8e8e008e
.long 0xf8f8f800, 0x00f1f1f1, 0x7c007c7c, 0xf5f500f5
.long 0xeeeeee00, 0x00dddddd, 0x77007777, 0xb6b600b6
.long 0xacacac00, 0x00595959, 0x56005656, 0xfdfd00fd
.long 0x0a0a0a00, 0x00141414, 0x05000505, 0x59590059
.long 0x36363600, 0x006c6c6c, 0x1b001b1b, 0x98980098
.long 0x49494900, 0x00929292, 0xa400a4a4, 0x6a6a006a
.long 0x2a2a2a00, 0x00545454, 0x15001515, 0x46460046
.long 0x68686800, 0x00d0d0d0, 0x34003434, 0xbaba00ba
.long 0x3c3c3c00, 0x00787878, 0x1e001e1e, 0x25250025
.long 0x38383800, 0x00707070, 0x1c001c1c, 0x42420042
.long 0xf1f1f100, 0x00e3e3e3, 0xf800f8f8, 0xa2a200a2
.long 0xa4a4a400, 0x00494949, 0x52005252, 0xfafa00fa
.long 0x40404000, 0x00808080, 0x20002020, 0x07070007
.long 0x28282800, 0x00505050, 0x14001414, 0x55550055
.long 0xd3d3d300, 0x00a7a7a7, 0xe900e9e9, 0xeeee00ee
.long 0x7b7b7b00, 0x00f6f6f6, 0xbd00bdbd, 0x0a0a000a
.long 0xbbbbbb00, 0x00777777, 0xdd00dddd, 0x49490049
.long 0xc9c9c900, 0x00939393, 0xe400e4e4, 0x68680068
.long 0x43434300, 0x00868686, 0xa100a1a1, 0x38380038
.long 0xc1c1c100, 0x00838383, 0xe000e0e0, 0xa4a400a4
.long 0x15151500, 0x002a2a2a, 0x8a008a8a, 0x28280028
.long 0xe3e3e300, 0x00c7c7c7, 0xf100f1f1, 0x7b7b007b
.long 0xadadad00, 0x005b5b5b, 0xd600d6d6, 0xc9c900c9
.long 0xf4f4f400, 0x00e9e9e9, 0x7a007a7a, 0xc1c100c1
.long 0x77777700, 0x00eeeeee, 0xbb00bbbb, 0xe3e300e3
.long 0xc7c7c700, 0x008f8f8f, 0xe300e3e3, 0xf4f400f4
.long 0x80808000, 0x00010101, 0x40004040, 0xc7c700c7
.long 0x9e9e9e00, 0x003d3d3d, 0x4f004f4f, 0x9e9e009e
ELF(.size _gcry_camellia_arm_tables,.-_gcry_camellia_arm_tables;)

#endif /*HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS*/
#endif /*__AARCH64EL__*/
