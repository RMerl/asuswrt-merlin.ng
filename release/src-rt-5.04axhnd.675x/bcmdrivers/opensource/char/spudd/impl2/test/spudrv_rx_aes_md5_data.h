/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/***************************************************************************
 * File Name  : 
 *
 * Description: 
 *              
 ***************************************************************************/

#ifndef __SPUDRV_RX_AES_MD5_DATA_H__
#define __SPUDRV_RX_AES_MD5_DATA_H__

#if defined(CONFIG_BCM_SPU_TEST)
//

// Automatically generated vector file.

//

// Vector format is: {EOP, SOP, PKT_DATA}.

//

// Example vectors:

//     101234567 // EOP = 0, SOP = 1, PKT_DATA = 01234567

//     089abcdef // EOP = 0, SOP = 0, PKT_DATA = 89abcdef

//     213572468 // EOP = 1, SOP = 0, PKT_DATA = 13572468

//

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt0_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 207 words. 

/// BDA size     is 824 (0x338) 

/// BDA id       is 0xd8ea 

    0x0338d8ea,// 3 BDA   1 

/// PAY Generic Data size   : 824 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x5a0cbf1d,// 4 PAY   1 

    0xf5cd007c,// 5 PAY   2 

    0xc72a5e6a,// 6 PAY   3 

    0xfc9c85d5,// 7 PAY   4 

    0x26942087,// 8 PAY   5 

    0xfb3b5286,// 9 PAY   6 

    0x56fa3df1,// 10 PAY   7 

    0xb4f0fef8,// 11 PAY   8 

    0x45c876bb,// 12 PAY   9 

    0xe6bb8807,// 13 PAY  10 

    0x9455c8a9,// 14 PAY  11 

    0x4e237977,// 15 PAY  12 

    0x8e84ed67,// 16 PAY  13 

    0xd5e6e7a1,// 17 PAY  14 

    0xb6c4dc8b,// 18 PAY  15 

    0x4f790bd6,// 19 PAY  16 

    0x0318e91e,// 20 PAY  17 

    0xd4988d26,// 21 PAY  18 

    0x58be9890,// 22 PAY  19 

    0xfb342480,// 23 PAY  20 

    0xf6139bd5,// 24 PAY  21 

    0x70c21c11,// 25 PAY  22 

    0x0754561c,// 26 PAY  23 

    0xcf85b9d6,// 27 PAY  24 

    0xb2dbd063,// 28 PAY  25 

    0x6e3f2e10,// 29 PAY  26 

    0x9999e5a7,// 30 PAY  27 

    0xc5cd7736,// 31 PAY  28 

    0x7fa4c518,// 32 PAY  29 

    0x610de9a5,// 33 PAY  30 

    0x1563869b,// 34 PAY  31 

    0xc9cb03a7,// 35 PAY  32 

    0x4c0eb75d,// 36 PAY  33 

    0xbf7b6114,// 37 PAY  34 

    0xeeb6e552,// 38 PAY  35 

    0x20e71bc0,// 39 PAY  36 

    0x46928f0b,// 40 PAY  37 

    0x8eec144d,// 41 PAY  38 

    0x43656b82,// 42 PAY  39 

    0x2bf32eab,// 43 PAY  40 

    0x9ffee422,// 44 PAY  41 

    0x884cd9f1,// 45 PAY  42 

    0x175dc2aa,// 46 PAY  43 

    0x033508a2,// 47 PAY  44 

    0xe3adc0f4,// 48 PAY  45 

    0xeae98ece,// 49 PAY  46 

    0xdf0b6d84,// 50 PAY  47 

    0xf4ab7075,// 51 PAY  48 

    0x9e319e04,// 52 PAY  49 

    0x8a85d69d,// 53 PAY  50 

    0x740559b8,// 54 PAY  51 

    0xbb10a234,// 55 PAY  52 

    0x5d2c8bf4,// 56 PAY  53 

    0x5dbcf3fe,// 57 PAY  54 

    0x1b39ea0a,// 58 PAY  55 

    0xa150f1ef,// 59 PAY  56 

    0x3a823713,// 60 PAY  57 

    0x9e94ddf4,// 61 PAY  58 

    0xadb0c443,// 62 PAY  59 

    0xe5bf3d16,// 63 PAY  60 

    0xad342dc4,// 64 PAY  61 

    0x0202620a,// 65 PAY  62 

    0x38dfc533,// 66 PAY  63 

    0x31fdc1c5,// 67 PAY  64 

    0x3fbeea00,// 68 PAY  65 

    0x449d3b5c,// 69 PAY  66 

    0xad81eea9,// 70 PAY  67 

    0x5967e2ca,// 71 PAY  68 

    0x7af5ccaf,// 72 PAY  69 

    0xe4425c78,// 73 PAY  70 

    0x9be653b7,// 74 PAY  71 

    0xf967c487,// 75 PAY  72 

    0xf5a618cc,// 76 PAY  73 

    0xd19fa0de,// 77 PAY  74 

    0x4faa12f6,// 78 PAY  75 

    0x9e0fbe3a,// 79 PAY  76 

    0xd6acaf72,// 80 PAY  77 

    0xd0895e06,// 81 PAY  78 

    0x80000d38,// 82 PAY  79 

    0x85f71e8d,// 83 PAY  80 

    0xe0a0cb72,// 84 PAY  81 

    0x52ae8c41,// 85 PAY  82 

    0xcc4b247c,// 86 PAY  83 

    0x014ce28e,// 87 PAY  84 

    0x7ee4b0c9,// 88 PAY  85 

    0xa8c45108,// 89 PAY  86 

    0x963846eb,// 90 PAY  87 

    0x14dd1391,// 91 PAY  88 

    0x3e058496,// 92 PAY  89 

    0xe4949e46,// 93 PAY  90 

    0xe71c4610,// 94 PAY  91 

    0x978564b0,// 95 PAY  92 

    0x71c4d072,// 96 PAY  93 

    0x598d7451,// 97 PAY  94 

    0x2c4d7beb,// 98 PAY  95 

    0xa8788caf,// 99 PAY  96 

    0x29a03f10,// 100 PAY  97 

    0x8a9be6c7,// 101 PAY  98 

    0x9b8e1585,// 102 PAY  99 

    0xd4bb5a96,// 103 PAY 100 

    0xee3bc58e,// 104 PAY 101 

    0x77f3c5a5,// 105 PAY 102 

    0x56738ab0,// 106 PAY 103 

    0x979aee5a,// 107 PAY 104 

    0xb97c6089,// 108 PAY 105 

    0x1b853d14,// 109 PAY 106 

    0xbd844109,// 110 PAY 107 

    0x264a05f6,// 111 PAY 108 

    0x13009385,// 112 PAY 109 

    0x7c55dd44,// 113 PAY 110 

    0xbd731af6,// 114 PAY 111 

    0xf43e93f9,// 115 PAY 112 

    0x503303df,// 116 PAY 113 

    0x1874ce3f,// 117 PAY 114 

    0x383f4ac8,// 118 PAY 115 

    0xcc76c488,// 119 PAY 116 

    0x22e9368c,// 120 PAY 117 

    0x75f22075,// 121 PAY 118 

    0x5d9ba284,// 122 PAY 119 

    0xe31e8feb,// 123 PAY 120 

    0xb0259a6c,// 124 PAY 121 

    0x1d8ecda5,// 125 PAY 122 

    0xb5373dea,// 126 PAY 123 

    0x5332e9ae,// 127 PAY 124 

    0x1c56ac52,// 128 PAY 125 

    0xa8cf3119,// 129 PAY 126 

    0x66b0aad3,// 130 PAY 127 

    0xbae5452a,// 131 PAY 128 

    0x0bb58c2d,// 132 PAY 129 

    0x56cc303e,// 133 PAY 130 

    0x2ce6a6c2,// 134 PAY 131 

    0xb162a135,// 135 PAY 132 

    0xefa0ed66,// 136 PAY 133 

    0xc3522e3e,// 137 PAY 134 

    0x05e25ce3,// 138 PAY 135 

    0xcb2f748b,// 139 PAY 136 

    0xd237ae06,// 140 PAY 137 

    0x909d1255,// 141 PAY 138 

    0xef8beff6,// 142 PAY 139 

    0x2e6ad892,// 143 PAY 140 

    0xcd1e459c,// 144 PAY 141 

    0xdf74e905,// 145 PAY 142 

    0x45d6c4eb,// 146 PAY 143 

    0xd94ee7cf,// 147 PAY 144 

    0x9c67cc94,// 148 PAY 145 

    0xa7ad4014,// 149 PAY 146 

    0xb31e419f,// 150 PAY 147 

    0x63062a8f,// 151 PAY 148 

    0x941b27d6,// 152 PAY 149 

    0x6b8ebe10,// 153 PAY 150 

    0x1755e1a8,// 154 PAY 151 

    0x6bbd727c,// 155 PAY 152 

    0x0528997d,// 156 PAY 153 

    0xd3663865,// 157 PAY 154 

    0xeb029feb,// 158 PAY 155 

    0x984551aa,// 159 PAY 156 

    0x6e194c20,// 160 PAY 157 

    0x63d5ec8b,// 161 PAY 158 

    0xa1050fac,// 162 PAY 159 

    0xcdfad5fd,// 163 PAY 160 

    0xe89d195c,// 164 PAY 161 

    0x09333d1b,// 165 PAY 162 

    0xd5434c67,// 166 PAY 163 

    0x8377359b,// 167 PAY 164 

    0x7b3a7743,// 168 PAY 165 

    0xc8eab1a0,// 169 PAY 166 

    0x16faee1e,// 170 PAY 167 

    0x839ed7f0,// 171 PAY 168 

    0xde6cecb0,// 172 PAY 169 

    0x790eda7b,// 173 PAY 170 

    0xab6864bf,// 174 PAY 171 

    0x6a1b969e,// 175 PAY 172 

    0x38462341,// 176 PAY 173 

    0xba9e3451,// 177 PAY 174 

    0x93daeb34,// 178 PAY 175 

    0xb7a914fb,// 179 PAY 176 

    0xf6fc457b,// 180 PAY 177 

    0x2ddcaec8,// 181 PAY 178 

    0x2a704908,// 182 PAY 179 

    0x992e0b25,// 183 PAY 180 

    0x5b6ec5bf,// 184 PAY 181 

    0x8dd03697,// 185 PAY 182 

    0xf4c2e939,// 186 PAY 183 

    0x896092ae,// 187 PAY 184 

    0x269984f0,// 188 PAY 185 

    0xfc2fa00e,// 189 PAY 186 

    0x71d1e9c8,// 190 PAY 187 

    0x3d1d7a8d,// 191 PAY 188 

    0x9f561d63,// 192 PAY 189 

    0xd322c8b6,// 193 PAY 190 

    0x046df476,// 194 PAY 191 

    0x772e50dd,// 195 PAY 192 

    0x9bdda5ff,// 196 PAY 193 

    0x8b649a68,// 197 PAY 194 

    0xa5c97dca,// 198 PAY 195 

    0x0b5aff7f,// 199 PAY 196 

    0x839f2697,// 200 PAY 197 

    0x6a3076de,// 201 PAY 198 

    0xc78fef59,// 202 PAY 199 

    0x9a968dfd,// 203 PAY 200 

    0xb1da9aaf,// 204 PAY 201 

    0x9339996d,// 205 PAY 202 

    0xfc7b0c5c,// 206 PAY 203 

    0xc13f3253,// 207 PAY 204 

    0x2b6687c8,// 208 PAY 205 

    0xe2d140f3,// 209 PAY 206 

/// HASH is  12 bytes 

    0x63062a8f,// 210 HSH   1 

    0x941b27d6,// 211 HSH   2 

    0x6b8ebe10,// 212 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 99 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x93 

    0x02609363 // 213 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt1_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 164 words. 

/// BDA size     is 649 (0x289) 

/// BDA id       is 0x3d82 

    0x02893d82,// 3 BDA   1 

/// PAY Generic Data size   : 649 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xb575ed9c,// 4 PAY   1 

    0xd5a8c884,// 5 PAY   2 

    0xc909a88a,// 6 PAY   3 

    0x68a3d654,// 7 PAY   4 

    0xa9894a52,// 8 PAY   5 

    0xd766a30b,// 9 PAY   6 

    0xb6f28bf2,// 10 PAY   7 

    0x51b96760,// 11 PAY   8 

    0x90e7c55c,// 12 PAY   9 

    0xad423119,// 13 PAY  10 

    0x2cd9e26b,// 14 PAY  11 

    0xb6850d76,// 15 PAY  12 

    0x61d620c1,// 16 PAY  13 

    0x6e7b4edc,// 17 PAY  14 

    0x57344172,// 18 PAY  15 

    0x4d047828,// 19 PAY  16 

    0x6bc098e7,// 20 PAY  17 

    0xecd52ab6,// 21 PAY  18 

    0x64442b51,// 22 PAY  19 

    0xeb5545c0,// 23 PAY  20 

    0x7699a726,// 24 PAY  21 

    0xdc81ede5,// 25 PAY  22 

    0xaaa7cb0b,// 26 PAY  23 

    0xfee2cf9b,// 27 PAY  24 

    0x26bc2837,// 28 PAY  25 

    0x471dfa1d,// 29 PAY  26 

    0x36d22c0b,// 30 PAY  27 

    0xdea50b30,// 31 PAY  28 

    0xb0166f2d,// 32 PAY  29 

    0xf8fb0a8c,// 33 PAY  30 

    0xbb41fc45,// 34 PAY  31 

    0xdb02f966,// 35 PAY  32 

    0x6396b7a7,// 36 PAY  33 

    0xf77a0c65,// 37 PAY  34 

    0x38b77e0c,// 38 PAY  35 

    0xac7d2929,// 39 PAY  36 

    0xe96f7f09,// 40 PAY  37 

    0x49ebf04e,// 41 PAY  38 

    0xade20220,// 42 PAY  39 

    0x86e890d8,// 43 PAY  40 

    0x5f1c2437,// 44 PAY  41 

    0x4a50356f,// 45 PAY  42 

    0x5981f554,// 46 PAY  43 

    0xb8a746c9,// 47 PAY  44 

    0x6cfe9350,// 48 PAY  45 

    0x07aad8ec,// 49 PAY  46 

    0x7cc02c5f,// 50 PAY  47 

    0xe93c0867,// 51 PAY  48 

    0x4e122d56,// 52 PAY  49 

    0xc13b2c70,// 53 PAY  50 

    0xe819a591,// 54 PAY  51 

    0x09d64f38,// 55 PAY  52 

    0x81e25855,// 56 PAY  53 

    0x60038e25,// 57 PAY  54 

    0x6a1c7ef2,// 58 PAY  55 

    0xd537189a,// 59 PAY  56 

    0x2115dda8,// 60 PAY  57 

    0xeb552fcf,// 61 PAY  58 

    0x3c6bec36,// 62 PAY  59 

    0xd027b782,// 63 PAY  60 

    0xf9542110,// 64 PAY  61 

    0xbef9407a,// 65 PAY  62 

    0x319c892b,// 66 PAY  63 

    0xd11b1481,// 67 PAY  64 

    0x11b76615,// 68 PAY  65 

    0xfed8240a,// 69 PAY  66 

    0x4d5c6bd6,// 70 PAY  67 

    0xf070c82c,// 71 PAY  68 

    0x9681f0c0,// 72 PAY  69 

    0x2d24cf2b,// 73 PAY  70 

    0x043c7c29,// 74 PAY  71 

    0x1bca7090,// 75 PAY  72 

    0x2aa34bdf,// 76 PAY  73 

    0xba8bba4f,// 77 PAY  74 

    0x346625e1,// 78 PAY  75 

    0x419808ac,// 79 PAY  76 

    0x83b45019,// 80 PAY  77 

    0x19eb43d3,// 81 PAY  78 

    0x16c146bc,// 82 PAY  79 

    0x561b342b,// 83 PAY  80 

    0x00832e96,// 84 PAY  81 

    0xfb8a5175,// 85 PAY  82 

    0x717ef3dc,// 86 PAY  83 

    0x18d11be5,// 87 PAY  84 

    0x09511229,// 88 PAY  85 

    0xdade1e7c,// 89 PAY  86 

    0xa45d3d54,// 90 PAY  87 

    0x5c1a59b1,// 91 PAY  88 

    0xa1689cb4,// 92 PAY  89 

    0x233bf2d6,// 93 PAY  90 

    0x63edbf5b,// 94 PAY  91 

    0xfba86602,// 95 PAY  92 

    0xfea53956,// 96 PAY  93 

    0xd4623b8c,// 97 PAY  94 

    0x8b08b5a9,// 98 PAY  95 

    0x77406f93,// 99 PAY  96 

    0xe51d7edf,// 100 PAY  97 

    0x1050db22,// 101 PAY  98 

    0x72304c8b,// 102 PAY  99 

    0xe2f110bb,// 103 PAY 100 

    0xcb0be014,// 104 PAY 101 

    0x93daf9ea,// 105 PAY 102 

    0xb87bfc5f,// 106 PAY 103 

    0x723e859b,// 107 PAY 104 

    0x6b728bbc,// 108 PAY 105 

    0x280ade51,// 109 PAY 106 

    0x7c9a52ec,// 110 PAY 107 

    0x49a25a9e,// 111 PAY 108 

    0x850f33f1,// 112 PAY 109 

    0x2ded73b5,// 113 PAY 110 

    0xf9abedd5,// 114 PAY 111 

    0x8bf446ce,// 115 PAY 112 

    0xa42c1fc1,// 116 PAY 113 

    0x25a84981,// 117 PAY 114 

    0x5bbb94b7,// 118 PAY 115 

    0x43f28df3,// 119 PAY 116 

    0x8617cae2,// 120 PAY 117 

    0x779c042c,// 121 PAY 118 

    0x027ce365,// 122 PAY 119 

    0x429be128,// 123 PAY 120 

    0x9c9e4975,// 124 PAY 121 

    0x6fdc8891,// 125 PAY 122 

    0x575a9b39,// 126 PAY 123 

    0xe267e582,// 127 PAY 124 

    0xd4492ee9,// 128 PAY 125 

    0xa30a1df4,// 129 PAY 126 

    0x1f1ef35e,// 130 PAY 127 

    0xbfa5559e,// 131 PAY 128 

    0x28e16bb6,// 132 PAY 129 

    0x24ce19ee,// 133 PAY 130 

    0xffd21990,// 134 PAY 131 

    0x07cbd5ec,// 135 PAY 132 

    0x429a3cd4,// 136 PAY 133 

    0x8f6a2dec,// 137 PAY 134 

    0xdf5b685b,// 138 PAY 135 

    0x87de9fe1,// 139 PAY 136 

    0x82fe3002,// 140 PAY 137 

    0x001ff3a5,// 141 PAY 138 

    0xc718b5df,// 142 PAY 139 

    0x740d2bc8,// 143 PAY 140 

    0x8c10ca4f,// 144 PAY 141 

    0x8a4cda41,// 145 PAY 142 

    0xe16fb23a,// 146 PAY 143 

    0xab2881d8,// 147 PAY 144 

    0xdf26d1ce,// 148 PAY 145 

    0xba6b99f0,// 149 PAY 146 

    0x67d80176,// 150 PAY 147 

    0x4ffd6f48,// 151 PAY 148 

    0x08844072,// 152 PAY 149 

    0xe477d365,// 153 PAY 150 

    0xd1ce729b,// 154 PAY 151 

    0xe7594d13,// 155 PAY 152 

    0xe588b38b,// 156 PAY 153 

    0xdb1d5193,// 157 PAY 154 

    0x6a141d8e,// 158 PAY 155 

    0xc9f07ada,// 159 PAY 156 

    0xc5e00d0a,// 160 PAY 157 

    0xdbe3fb25,// 161 PAY 158 

    0x3dbe9115,// 162 PAY 159 

    0x80388e31,// 163 PAY 160 

    0x41ba948c,// 164 PAY 161 

    0xcdeeebcd,// 165 PAY 162 

    0xf1000000,// 166 PAY 163 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x018880b2 // 167 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt2_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 114 words. 

/// BDA size     is 452 (0x1c4) 

/// BDA id       is 0xb49d 

    0x01c4b49d,// 3 BDA   1 

/// PAY Generic Data size   : 452 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xeab765d3,// 4 PAY   1 

    0xafec0890,// 5 PAY   2 

    0x4ca2e265,// 6 PAY   3 

    0x1d1f3c0b,// 7 PAY   4 

    0x330d7a5b,// 8 PAY   5 

    0xcc02efa7,// 9 PAY   6 

    0xa2eb13a5,// 10 PAY   7 

    0x184a4246,// 11 PAY   8 

    0x77f3d7ff,// 12 PAY   9 

    0x66be2e12,// 13 PAY  10 

    0x14d12842,// 14 PAY  11 

    0x4067c40a,// 15 PAY  12 

    0xd6864edb,// 16 PAY  13 

    0xc7feea40,// 17 PAY  14 

    0x444c3a30,// 18 PAY  15 

    0x6a50223c,// 19 PAY  16 

    0x5b650f53,// 20 PAY  17 

    0xca48b31c,// 21 PAY  18 

    0x3a0cf956,// 22 PAY  19 

    0xe4f7fc91,// 23 PAY  20 

    0x5dbe26b1,// 24 PAY  21 

    0x806d387c,// 25 PAY  22 

    0xe920be2c,// 26 PAY  23 

    0xbc0895d4,// 27 PAY  24 

    0xdf4151e3,// 28 PAY  25 

    0xe93f047e,// 29 PAY  26 

    0x7a32f3a7,// 30 PAY  27 

    0x02db7639,// 31 PAY  28 

    0x29079420,// 32 PAY  29 

    0x7bf7068e,// 33 PAY  30 

    0x5ab89bb3,// 34 PAY  31 

    0xaadde0ec,// 35 PAY  32 

    0xfd99d983,// 36 PAY  33 

    0xb8f2eefa,// 37 PAY  34 

    0x870769c4,// 38 PAY  35 

    0x67f1309e,// 39 PAY  36 

    0x9c3747e2,// 40 PAY  37 

    0x01e479b3,// 41 PAY  38 

    0x81517b25,// 42 PAY  39 

    0x15bb4ab3,// 43 PAY  40 

    0x0d33f14b,// 44 PAY  41 

    0x56789730,// 45 PAY  42 

    0xc04726ea,// 46 PAY  43 

    0x37fd10e4,// 47 PAY  44 

    0x82483aec,// 48 PAY  45 

    0x8ea946f3,// 49 PAY  46 

    0xcbc0bb9d,// 50 PAY  47 

    0x486944f1,// 51 PAY  48 

    0x3eca9793,// 52 PAY  49 

    0x44f39d08,// 53 PAY  50 

    0xa26953ac,// 54 PAY  51 

    0xe54dcae1,// 55 PAY  52 

    0xf0b54a73,// 56 PAY  53 

    0xead946cb,// 57 PAY  54 

    0xef54df72,// 58 PAY  55 

    0x62e206e1,// 59 PAY  56 

    0x2fb20c91,// 60 PAY  57 

    0xd8acf867,// 61 PAY  58 

    0x7bb0bc6f,// 62 PAY  59 

    0x7b938290,// 63 PAY  60 

    0x0f33c799,// 64 PAY  61 

    0xb6236287,// 65 PAY  62 

    0xd23ce3bd,// 66 PAY  63 

    0x8db423d6,// 67 PAY  64 

    0x9f126733,// 68 PAY  65 

    0x6d46e2f1,// 69 PAY  66 

    0xdcabd0a9,// 70 PAY  67 

    0x1b259093,// 71 PAY  68 

    0x9a71cad0,// 72 PAY  69 

    0xebad383c,// 73 PAY  70 

    0xd9090ab0,// 74 PAY  71 

    0x14ec767b,// 75 PAY  72 

    0xe2eeeb0a,// 76 PAY  73 

    0x9d5ac76a,// 77 PAY  74 

    0xc521cd55,// 78 PAY  75 

    0xd322833a,// 79 PAY  76 

    0x4b4b40de,// 80 PAY  77 

    0xbbf578ef,// 81 PAY  78 

    0xe1fdd233,// 82 PAY  79 

    0x94725939,// 83 PAY  80 

    0x9f2f6da4,// 84 PAY  81 

    0xbb034c40,// 85 PAY  82 

    0x9e582d54,// 86 PAY  83 

    0x286633f8,// 87 PAY  84 

    0xc9c9dd8e,// 88 PAY  85 

    0x556ac168,// 89 PAY  86 

    0x7299084b,// 90 PAY  87 

    0x0f7546f7,// 91 PAY  88 

    0x37a93c2f,// 92 PAY  89 

    0x9bea4349,// 93 PAY  90 

    0x0694358f,// 94 PAY  91 

    0x0ddf6e96,// 95 PAY  92 

    0x3802e97b,// 96 PAY  93 

    0x7f20b1df,// 97 PAY  94 

    0x26222888,// 98 PAY  95 

    0xf91c92a4,// 99 PAY  96 

    0x33e49f58,// 100 PAY  97 

    0x11013264,// 101 PAY  98 

    0x264d81ca,// 102 PAY  99 

    0xa27897b3,// 103 PAY 100 

    0x2c08aa34,// 104 PAY 101 

    0x7fa621c4,// 105 PAY 102 

    0xa1087fde,// 106 PAY 103 

    0x893e93e1,// 107 PAY 104 

    0x7e34dd6a,// 108 PAY 105 

    0x74d5a1a9,// 109 PAY 106 

    0x9987243b,// 110 PAY 107 

    0xf5399cd7,// 111 PAY 108 

    0xcb40610c,// 112 PAY 109 

    0x1b42ab9f,// 113 PAY 110 

    0xfad3c639,// 114 PAY 111 

    0xbca65842,// 115 PAY 112 

    0xd976fb97,// 116 PAY 113 

/// STA is 1 words. 

/// STA num_pkts       : 74 

/// STA pkt_idx        : 189 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe9 

    0x02f5e94a // 117 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt3_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 344 words. 

/// BDA size     is 1370 (0x55a) 

/// BDA id       is 0x61e 

    0x055a061e,// 3 BDA   1 

/// PAY Generic Data size   : 1370 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xd5184464,// 4 PAY   1 

    0x39c6be8e,// 5 PAY   2 

    0x26186106,// 6 PAY   3 

    0xae693102,// 7 PAY   4 

    0xf08e0d79,// 8 PAY   5 

    0x1de7702d,// 9 PAY   6 

    0x96170c04,// 10 PAY   7 

    0xbc2f9644,// 11 PAY   8 

    0xe07819f7,// 12 PAY   9 

    0xd40428b7,// 13 PAY  10 

    0x286ab3b4,// 14 PAY  11 

    0x231f5937,// 15 PAY  12 

    0xda47bf40,// 16 PAY  13 

    0x64686114,// 17 PAY  14 

    0x2897d627,// 18 PAY  15 

    0x95d78651,// 19 PAY  16 

    0xa7ad8244,// 20 PAY  17 

    0x397e0e44,// 21 PAY  18 

    0x3f2f1bcc,// 22 PAY  19 

    0x8731fcad,// 23 PAY  20 

    0x9c7dcbb5,// 24 PAY  21 

    0x9a90c55c,// 25 PAY  22 

    0x7d40e723,// 26 PAY  23 

    0x84501466,// 27 PAY  24 

    0x15145f99,// 28 PAY  25 

    0xd96005b0,// 29 PAY  26 

    0xff6fe2ac,// 30 PAY  27 

    0x95d90456,// 31 PAY  28 

    0x783005c6,// 32 PAY  29 

    0xc7533985,// 33 PAY  30 

    0x191d543a,// 34 PAY  31 

    0x1b409860,// 35 PAY  32 

    0x5601a72e,// 36 PAY  33 

    0x4ebd7b2e,// 37 PAY  34 

    0x82f0044a,// 38 PAY  35 

    0x84176321,// 39 PAY  36 

    0x2aa7298f,// 40 PAY  37 

    0xcc6913d8,// 41 PAY  38 

    0xefc72c54,// 42 PAY  39 

    0x79af1a98,// 43 PAY  40 

    0x18bb4b1f,// 44 PAY  41 

    0x612bb8bb,// 45 PAY  42 

    0xfc31b5dc,// 46 PAY  43 

    0x39312b0f,// 47 PAY  44 

    0xfc06f893,// 48 PAY  45 

    0x63f93f69,// 49 PAY  46 

    0x62931922,// 50 PAY  47 

    0xeb057a7f,// 51 PAY  48 

    0x9bf40680,// 52 PAY  49 

    0xc4d98aad,// 53 PAY  50 

    0xc6b920ea,// 54 PAY  51 

    0x2c1f6cf6,// 55 PAY  52 

    0x7166525e,// 56 PAY  53 

    0xd1067da3,// 57 PAY  54 

    0xa3ca216d,// 58 PAY  55 

    0x4212dfd1,// 59 PAY  56 

    0x539ffe62,// 60 PAY  57 

    0x5d3c4213,// 61 PAY  58 

    0xbf1bd7f8,// 62 PAY  59 

    0x834ac4b0,// 63 PAY  60 

    0x65b9d5ce,// 64 PAY  61 

    0xf8c4c718,// 65 PAY  62 

    0xd044d0fe,// 66 PAY  63 

    0x62e48565,// 67 PAY  64 

    0x80e5d09e,// 68 PAY  65 

    0x4ba985c9,// 69 PAY  66 

    0x96ad0613,// 70 PAY  67 

    0x291c6b26,// 71 PAY  68 

    0x7866055b,// 72 PAY  69 

    0x7fb216ca,// 73 PAY  70 

    0xe9d07e3b,// 74 PAY  71 

    0x6fdc40b1,// 75 PAY  72 

    0xa55d67f3,// 76 PAY  73 

    0x0b9cc94e,// 77 PAY  74 

    0x424deecf,// 78 PAY  75 

    0xafd36b37,// 79 PAY  76 

    0xf2a582be,// 80 PAY  77 

    0xa0604b7c,// 81 PAY  78 

    0x567a4db5,// 82 PAY  79 

    0x3dcf6fa7,// 83 PAY  80 

    0x62ab18fe,// 84 PAY  81 

    0x80220751,// 85 PAY  82 

    0x5e07eee5,// 86 PAY  83 

    0x500508e9,// 87 PAY  84 

    0x91241e0c,// 88 PAY  85 

    0xb1dd6761,// 89 PAY  86 

    0xa71ba8ed,// 90 PAY  87 

    0xd2853a87,// 91 PAY  88 

    0xf225b47f,// 92 PAY  89 

    0x2911bbde,// 93 PAY  90 

    0xc53a2e43,// 94 PAY  91 

    0x5e329820,// 95 PAY  92 

    0x388b4f7c,// 96 PAY  93 

    0x3eb16c0f,// 97 PAY  94 

    0x0a2dd7ce,// 98 PAY  95 

    0xd58bc84e,// 99 PAY  96 

    0x94abdb09,// 100 PAY  97 

    0x240d9aca,// 101 PAY  98 

    0x9ef06fc3,// 102 PAY  99 

    0xdba1cbeb,// 103 PAY 100 

    0xdba3d296,// 104 PAY 101 

    0x78d375c1,// 105 PAY 102 

    0x188c7d0d,// 106 PAY 103 

    0xd42bad77,// 107 PAY 104 

    0x24474aac,// 108 PAY 105 

    0x1436c8e1,// 109 PAY 106 

    0x2980a233,// 110 PAY 107 

    0x05c683ce,// 111 PAY 108 

    0x86b6a3dc,// 112 PAY 109 

    0x5a14c054,// 113 PAY 110 

    0x54c9897f,// 114 PAY 111 

    0x13a55edd,// 115 PAY 112 

    0x874f90d4,// 116 PAY 113 

    0xddd7db5c,// 117 PAY 114 

    0xf9e9e0f9,// 118 PAY 115 

    0x13455c2c,// 119 PAY 116 

    0x43656b57,// 120 PAY 117 

    0x9c33a077,// 121 PAY 118 

    0xe3497662,// 122 PAY 119 

    0x3eb2dc68,// 123 PAY 120 

    0x7598208a,// 124 PAY 121 

    0xa6f6abfc,// 125 PAY 122 

    0xca151563,// 126 PAY 123 

    0x18387bc3,// 127 PAY 124 

    0x25e464e5,// 128 PAY 125 

    0x511942c3,// 129 PAY 126 

    0x1a734805,// 130 PAY 127 

    0x5e21ef4a,// 131 PAY 128 

    0x0a13095a,// 132 PAY 129 

    0x7c2df976,// 133 PAY 130 

    0xe5258e51,// 134 PAY 131 

    0x4423f165,// 135 PAY 132 

    0x9a7dd970,// 136 PAY 133 

    0xd346c855,// 137 PAY 134 

    0xb0d7a56f,// 138 PAY 135 

    0xf827f69c,// 139 PAY 136 

    0xc3ea28ac,// 140 PAY 137 

    0x2d1bc12d,// 141 PAY 138 

    0xb725a87e,// 142 PAY 139 

    0x20c55069,// 143 PAY 140 

    0xd1ab3607,// 144 PAY 141 

    0xe6345e47,// 145 PAY 142 

    0x130e20bf,// 146 PAY 143 

    0x76a80e35,// 147 PAY 144 

    0xcff34685,// 148 PAY 145 

    0x871029af,// 149 PAY 146 

    0xc3608935,// 150 PAY 147 

    0x82746d95,// 151 PAY 148 

    0xc261a3a0,// 152 PAY 149 

    0x85123bf2,// 153 PAY 150 

    0xe953c370,// 154 PAY 151 

    0xa1979407,// 155 PAY 152 

    0xa7666b02,// 156 PAY 153 

    0xc4c9bfff,// 157 PAY 154 

    0x3a0db88c,// 158 PAY 155 

    0xce456b2f,// 159 PAY 156 

    0x027094af,// 160 PAY 157 

    0x13ad8613,// 161 PAY 158 

    0xa184df19,// 162 PAY 159 

    0x1ddb8ee9,// 163 PAY 160 

    0xabc9e496,// 164 PAY 161 

    0x58412737,// 165 PAY 162 

    0xa1c59d75,// 166 PAY 163 

    0x7a984ba6,// 167 PAY 164 

    0x25d35471,// 168 PAY 165 

    0xa91da804,// 169 PAY 166 

    0x8634312c,// 170 PAY 167 

    0x69d68323,// 171 PAY 168 

    0x01bc0fab,// 172 PAY 169 

    0x1770a954,// 173 PAY 170 

    0xa886a10f,// 174 PAY 171 

    0x69500aa2,// 175 PAY 172 

    0xfc387129,// 176 PAY 173 

    0x6ae74e6b,// 177 PAY 174 

    0x7a1c05be,// 178 PAY 175 

    0x8ab1413d,// 179 PAY 176 

    0xef5924e3,// 180 PAY 177 

    0x493e5e4b,// 181 PAY 178 

    0x5cc6fe94,// 182 PAY 179 

    0x74d62d47,// 183 PAY 180 

    0xed206e13,// 184 PAY 181 

    0xecd0a2a1,// 185 PAY 182 

    0xa98002f3,// 186 PAY 183 

    0x7393cce8,// 187 PAY 184 

    0xce4fb080,// 188 PAY 185 

    0x1ae1f05e,// 189 PAY 186 

    0x16ec4186,// 190 PAY 187 

    0x1761dad0,// 191 PAY 188 

    0x7ce26159,// 192 PAY 189 

    0x85a07975,// 193 PAY 190 

    0x97cec0a3,// 194 PAY 191 

    0xbd305df9,// 195 PAY 192 

    0x3b4caa89,// 196 PAY 193 

    0xb8168ef9,// 197 PAY 194 

    0xc9a6626e,// 198 PAY 195 

    0x359b37d9,// 199 PAY 196 

    0x300cb76f,// 200 PAY 197 

    0xcea01f20,// 201 PAY 198 

    0x0622f4da,// 202 PAY 199 

    0xae0b233d,// 203 PAY 200 

    0xe523f60d,// 204 PAY 201 

    0x0950e533,// 205 PAY 202 

    0x36a1849e,// 206 PAY 203 

    0xf778e1e2,// 207 PAY 204 

    0x00750d48,// 208 PAY 205 

    0xca31f766,// 209 PAY 206 

    0x02ea14e9,// 210 PAY 207 

    0xe85bdeed,// 211 PAY 208 

    0x0a630409,// 212 PAY 209 

    0xa42be44e,// 213 PAY 210 

    0xa4055a37,// 214 PAY 211 

    0x3610843d,// 215 PAY 212 

    0x6dd9972b,// 216 PAY 213 

    0x88ce1b36,// 217 PAY 214 

    0x4dcc177c,// 218 PAY 215 

    0x5d8b2593,// 219 PAY 216 

    0x97869cd8,// 220 PAY 217 

    0x1af02d55,// 221 PAY 218 

    0x1a1fde42,// 222 PAY 219 

    0xe66dddbf,// 223 PAY 220 

    0x059a7124,// 224 PAY 221 

    0xd2e8f7ff,// 225 PAY 222 

    0x51bdab53,// 226 PAY 223 

    0x9b3e841c,// 227 PAY 224 

    0x2c8a1683,// 228 PAY 225 

    0xe412ad30,// 229 PAY 226 

    0x170b18db,// 230 PAY 227 

    0x182514f7,// 231 PAY 228 

    0xea775719,// 232 PAY 229 

    0x5e732298,// 233 PAY 230 

    0xdf5c2193,// 234 PAY 231 

    0x61baf9be,// 235 PAY 232 

    0x5518c6c3,// 236 PAY 233 

    0x4b77ffe9,// 237 PAY 234 

    0xe5a073bf,// 238 PAY 235 

    0x79aabf6f,// 239 PAY 236 

    0xd578986c,// 240 PAY 237 

    0xa3ab7be7,// 241 PAY 238 

    0x6ed1c469,// 242 PAY 239 

    0xf4cdc9d2,// 243 PAY 240 

    0x0701ed05,// 244 PAY 241 

    0x56716e73,// 245 PAY 242 

    0xbbb3eb0f,// 246 PAY 243 

    0x0a809d8a,// 247 PAY 244 

    0x48cf0ee1,// 248 PAY 245 

    0x9f76a68a,// 249 PAY 246 

    0x61d4cc62,// 250 PAY 247 

    0xc3b35c40,// 251 PAY 248 

    0x1e8f7098,// 252 PAY 249 

    0xea712371,// 253 PAY 250 

    0xdec9aaf3,// 254 PAY 251 

    0x4c0e0b48,// 255 PAY 252 

    0x72529453,// 256 PAY 253 

    0x9861866c,// 257 PAY 254 

    0xb5f62c4a,// 258 PAY 255 

    0x0d171464,// 259 PAY 256 

    0x8c34a1ac,// 260 PAY 257 

    0x74e21190,// 261 PAY 258 

    0x766a9f1b,// 262 PAY 259 

    0x8b35a38a,// 263 PAY 260 

    0x0609d668,// 264 PAY 261 

    0x36647635,// 265 PAY 262 

    0x349390b4,// 266 PAY 263 

    0x922eba96,// 267 PAY 264 

    0xaa416ee5,// 268 PAY 265 

    0x826cbff9,// 269 PAY 266 

    0xc2dc4e36,// 270 PAY 267 

    0x59298811,// 271 PAY 268 

    0xcc38f797,// 272 PAY 269 

    0x8a8e1516,// 273 PAY 270 

    0x08cce90d,// 274 PAY 271 

    0xbbd88e8a,// 275 PAY 272 

    0x532de01d,// 276 PAY 273 

    0x3dfa1a82,// 277 PAY 274 

    0xad0f6d47,// 278 PAY 275 

    0x3e76235d,// 279 PAY 276 

    0xf047eeaa,// 280 PAY 277 

    0x5c37a513,// 281 PAY 278 

    0xa7d3aa7e,// 282 PAY 279 

    0xb9f38fd3,// 283 PAY 280 

    0x46f66432,// 284 PAY 281 

    0x1b745376,// 285 PAY 282 

    0xc9a0c318,// 286 PAY 283 

    0xa22c7b1e,// 287 PAY 284 

    0xdafc38cd,// 288 PAY 285 

    0xcd0bb840,// 289 PAY 286 

    0x69ad876e,// 290 PAY 287 

    0xeb1746ec,// 291 PAY 288 

    0x8bb65def,// 292 PAY 289 

    0x48e42211,// 293 PAY 290 

    0xaff69937,// 294 PAY 291 

    0x890d7904,// 295 PAY 292 

    0x2645c30b,// 296 PAY 293 

    0x4d15039d,// 297 PAY 294 

    0x2da9edf5,// 298 PAY 295 

    0x239ed126,// 299 PAY 296 

    0x26b2bee3,// 300 PAY 297 

    0x44693ffb,// 301 PAY 298 

    0xd845bbf4,// 302 PAY 299 

    0x073a2193,// 303 PAY 300 

    0x64a32fd1,// 304 PAY 301 

    0xe67e3e9b,// 305 PAY 302 

    0xddb148b3,// 306 PAY 303 

    0x8c9cca9b,// 307 PAY 304 

    0x21e933da,// 308 PAY 305 

    0x08d07b6f,// 309 PAY 306 

    0x9f2814ac,// 310 PAY 307 

    0x28e3a119,// 311 PAY 308 

    0x38a4c30f,// 312 PAY 309 

    0xad7f328c,// 313 PAY 310 

    0xf65bc813,// 314 PAY 311 

    0xb37275d2,// 315 PAY 312 

    0x0407c360,// 316 PAY 313 

    0x42838d66,// 317 PAY 314 

    0x60fdd401,// 318 PAY 315 

    0xcf57abc0,// 319 PAY 316 

    0x8a1e658a,// 320 PAY 317 

    0x2609394c,// 321 PAY 318 

    0xa2c663d5,// 322 PAY 319 

    0x71cddf17,// 323 PAY 320 

    0xbe8fb0e7,// 324 PAY 321 

    0xc59c35f5,// 325 PAY 322 

    0xacc4fb12,// 326 PAY 323 

    0xf96b41fc,// 327 PAY 324 

    0x65da2988,// 328 PAY 325 

    0xa9dfe988,// 329 PAY 326 

    0x711fb787,// 330 PAY 327 

    0xcee88a88,// 331 PAY 328 

    0xf42ab9f1,// 332 PAY 329 

    0xb12370e7,// 333 PAY 330 

    0x2b59f368,// 334 PAY 331 

    0x95717141,// 335 PAY 332 

    0x43f4e93c,// 336 PAY 333 

    0xd9fe3631,// 337 PAY 334 

    0x417479f6,// 338 PAY 335 

    0xa0f884e0,// 339 PAY 336 

    0x1b1257b3,// 340 PAY 337 

    0x4b856b06,// 341 PAY 338 

    0xb43c3fa1,// 342 PAY 339 

    0xde3286db,// 343 PAY 340 

    0xbf09bc60,// 344 PAY 341 

    0x4d0ea9d9,// 345 PAY 342 

    0x595b0000,// 346 PAY 343 

/// HASH is  8 bytes 

    0x1b745376,// 347 HSH   1 

    0xc9a0c318,// 348 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 112 

/// STA pkt_idx        : 18 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb1 

    0x0049b170 // 349 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt4_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 223 words. 

/// BDA size     is 888 (0x378) 

/// BDA id       is 0xff9 

    0x03780ff9,// 3 BDA   1 

/// PAY Generic Data size   : 888 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xa4c95238,// 4 PAY   1 

    0x60fa19cd,// 5 PAY   2 

    0xa850065e,// 6 PAY   3 

    0x84fd25ac,// 7 PAY   4 

    0x4a676e02,// 8 PAY   5 

    0x56cc6287,// 9 PAY   6 

    0xbe5963b0,// 10 PAY   7 

    0xaeee4bf4,// 11 PAY   8 

    0xf5579467,// 12 PAY   9 

    0x41da59ec,// 13 PAY  10 

    0x122e52be,// 14 PAY  11 

    0xb8da859d,// 15 PAY  12 

    0x3cc1c619,// 16 PAY  13 

    0x00f5ddc7,// 17 PAY  14 

    0x0b54faca,// 18 PAY  15 

    0x7fb1a00d,// 19 PAY  16 

    0xca36c0fe,// 20 PAY  17 

    0x987f9e50,// 21 PAY  18 

    0xc760e710,// 22 PAY  19 

    0xe16f0238,// 23 PAY  20 

    0x5b328b89,// 24 PAY  21 

    0x3742cc34,// 25 PAY  22 

    0x75fc7a22,// 26 PAY  23 

    0x0b395458,// 27 PAY  24 

    0x8544dc38,// 28 PAY  25 

    0x2f7bf171,// 29 PAY  26 

    0x22bb2b6e,// 30 PAY  27 

    0xb0ebcbf5,// 31 PAY  28 

    0x50b68810,// 32 PAY  29 

    0xaaa73335,// 33 PAY  30 

    0x1343b49e,// 34 PAY  31 

    0xa7479902,// 35 PAY  32 

    0x434cedf9,// 36 PAY  33 

    0x58f953d3,// 37 PAY  34 

    0x73f41806,// 38 PAY  35 

    0x379f2fcc,// 39 PAY  36 

    0xaebdc9aa,// 40 PAY  37 

    0xc4f5d18f,// 41 PAY  38 

    0xf04fc99e,// 42 PAY  39 

    0xbebe6017,// 43 PAY  40 

    0xbe17a86d,// 44 PAY  41 

    0xb6861baa,// 45 PAY  42 

    0x0e4d5f68,// 46 PAY  43 

    0xc8d14490,// 47 PAY  44 

    0x91769e8b,// 48 PAY  45 

    0x0be3ad28,// 49 PAY  46 

    0x6d7fc0bd,// 50 PAY  47 

    0x05b755a7,// 51 PAY  48 

    0x135671e0,// 52 PAY  49 

    0xeeaa2d6f,// 53 PAY  50 

    0x952c1735,// 54 PAY  51 

    0xfc54cbf9,// 55 PAY  52 

    0x072391f9,// 56 PAY  53 

    0x94e6081d,// 57 PAY  54 

    0xc420c988,// 58 PAY  55 

    0xb1665448,// 59 PAY  56 

    0x877b61de,// 60 PAY  57 

    0x3431561d,// 61 PAY  58 

    0x13930043,// 62 PAY  59 

    0xef0ad1eb,// 63 PAY  60 

    0xe9f8cac3,// 64 PAY  61 

    0xa6ecfabf,// 65 PAY  62 

    0xb272f0e7,// 66 PAY  63 

    0x4dfbeb43,// 67 PAY  64 

    0x4a430343,// 68 PAY  65 

    0xe546afeb,// 69 PAY  66 

    0x4aca0e97,// 70 PAY  67 

    0xde3d73eb,// 71 PAY  68 

    0x965ea15a,// 72 PAY  69 

    0x03ed58a6,// 73 PAY  70 

    0x51ddffb5,// 74 PAY  71 

    0x65d5664c,// 75 PAY  72 

    0x4f3c3448,// 76 PAY  73 

    0xca1ac0ff,// 77 PAY  74 

    0x7009fe17,// 78 PAY  75 

    0xb4507d31,// 79 PAY  76 

    0xc7493345,// 80 PAY  77 

    0x607303b1,// 81 PAY  78 

    0x3a5cddf1,// 82 PAY  79 

    0xfd592b8a,// 83 PAY  80 

    0x0f144e7f,// 84 PAY  81 

    0xe3365cd2,// 85 PAY  82 

    0x6f2f1d68,// 86 PAY  83 

    0xeefd7a40,// 87 PAY  84 

    0x61ad4a74,// 88 PAY  85 

    0x38504f8d,// 89 PAY  86 

    0x5481dadc,// 90 PAY  87 

    0x6f5b049b,// 91 PAY  88 

    0x7ea1d21a,// 92 PAY  89 

    0x76476bfe,// 93 PAY  90 

    0x7ea799e8,// 94 PAY  91 

    0x0d237105,// 95 PAY  92 

    0x1b3c13b1,// 96 PAY  93 

    0x23b89a46,// 97 PAY  94 

    0xa0e4f5bf,// 98 PAY  95 

    0x21e3f1ed,// 99 PAY  96 

    0x2b01599d,// 100 PAY  97 

    0x7a05f786,// 101 PAY  98 

    0xd9fc0b49,// 102 PAY  99 

    0x5b9b7a6d,// 103 PAY 100 

    0xb0473b73,// 104 PAY 101 

    0xe1d352d1,// 105 PAY 102 

    0x86501747,// 106 PAY 103 

    0xcde073ba,// 107 PAY 104 

    0xce434bb5,// 108 PAY 105 

    0xea33e395,// 109 PAY 106 

    0x3d5d550d,// 110 PAY 107 

    0x43395db7,// 111 PAY 108 

    0x58428cbc,// 112 PAY 109 

    0xaa4e6068,// 113 PAY 110 

    0xa02f4c05,// 114 PAY 111 

    0xe03f5fa8,// 115 PAY 112 

    0x74aec080,// 116 PAY 113 

    0x7d4f4966,// 117 PAY 114 

    0xc263ab75,// 118 PAY 115 

    0x0b4f5ad2,// 119 PAY 116 

    0x591025ea,// 120 PAY 117 

    0x482b578b,// 121 PAY 118 

    0x01d684ed,// 122 PAY 119 

    0x220ab8df,// 123 PAY 120 

    0x8430ba45,// 124 PAY 121 

    0xa8f56e83,// 125 PAY 122 

    0xe7fc1c20,// 126 PAY 123 

    0xabc50c0a,// 127 PAY 124 

    0x9cd7db82,// 128 PAY 125 

    0xae2f8cfc,// 129 PAY 126 

    0x5fc230c1,// 130 PAY 127 

    0x2081d301,// 131 PAY 128 

    0x83c386d8,// 132 PAY 129 

    0xf9b79e7a,// 133 PAY 130 

    0x0213824b,// 134 PAY 131 

    0xfb8e34d0,// 135 PAY 132 

    0xa270b7a0,// 136 PAY 133 

    0x04b40780,// 137 PAY 134 

    0xb65480d0,// 138 PAY 135 

    0x4c17e57a,// 139 PAY 136 

    0xf956f3ce,// 140 PAY 137 

    0x50f284e7,// 141 PAY 138 

    0xbed5f332,// 142 PAY 139 

    0x3ef08e62,// 143 PAY 140 

    0xd0e77981,// 144 PAY 141 

    0xf8f0ba1c,// 145 PAY 142 

    0xd9f79ed4,// 146 PAY 143 

    0x3057e87c,// 147 PAY 144 

    0xe9d63259,// 148 PAY 145 

    0x4b464d65,// 149 PAY 146 

    0x141b2bbe,// 150 PAY 147 

    0xae74f3f0,// 151 PAY 148 

    0x34c15605,// 152 PAY 149 

    0xaf71562f,// 153 PAY 150 

    0x6e49e2ee,// 154 PAY 151 

    0xb59942a1,// 155 PAY 152 

    0x1b51f826,// 156 PAY 153 

    0x4a73113f,// 157 PAY 154 

    0x640eba58,// 158 PAY 155 

    0x2b163ef0,// 159 PAY 156 

    0xa4d3c018,// 160 PAY 157 

    0x83ef563a,// 161 PAY 158 

    0xe39f11ef,// 162 PAY 159 

    0x23ef7055,// 163 PAY 160 

    0x4a3cc2c3,// 164 PAY 161 

    0x8741858f,// 165 PAY 162 

    0xf4a023e9,// 166 PAY 163 

    0x144cf4be,// 167 PAY 164 

    0xc2531afe,// 168 PAY 165 

    0xf7ae99ce,// 169 PAY 166 

    0xcd9f338e,// 170 PAY 167 

    0x8467d369,// 171 PAY 168 

    0x1d51d005,// 172 PAY 169 

    0x3d5f4569,// 173 PAY 170 

    0xe48bf5e4,// 174 PAY 171 

    0x28776b32,// 175 PAY 172 

    0x3f382c3c,// 176 PAY 173 

    0xa1cec91e,// 177 PAY 174 

    0x77bd6663,// 178 PAY 175 

    0x88ee59c9,// 179 PAY 176 

    0x15c1728c,// 180 PAY 177 

    0x789ebcc8,// 181 PAY 178 

    0xad18dbc2,// 182 PAY 179 

    0x883ebc4a,// 183 PAY 180 

    0xdc7a7d89,// 184 PAY 181 

    0x54472eb8,// 185 PAY 182 

    0x2a20f0e4,// 186 PAY 183 

    0x00a2c8a1,// 187 PAY 184 

    0x50c4f110,// 188 PAY 185 

    0x73df4672,// 189 PAY 186 

    0x5498649a,// 190 PAY 187 

    0xfca132ff,// 191 PAY 188 

    0xa41b4410,// 192 PAY 189 

    0x5ca52e2b,// 193 PAY 190 

    0xa33c3e41,// 194 PAY 191 

    0xb01629a3,// 195 PAY 192 

    0x3847ed47,// 196 PAY 193 

    0x7181d28b,// 197 PAY 194 

    0xcb059f47,// 198 PAY 195 

    0xab2bd516,// 199 PAY 196 

    0xc9c1c692,// 200 PAY 197 

    0x1b73406c,// 201 PAY 198 

    0x6c16fd19,// 202 PAY 199 

    0xd8486a1b,// 203 PAY 200 

    0x573b2570,// 204 PAY 201 

    0x78289c34,// 205 PAY 202 

    0x7ad75cde,// 206 PAY 203 

    0xb884e70e,// 207 PAY 204 

    0xb68a7e8a,// 208 PAY 205 

    0x69cae614,// 209 PAY 206 

    0xef66a6ba,// 210 PAY 207 

    0x3cc7c0e8,// 211 PAY 208 

    0x647a5120,// 212 PAY 209 

    0x412613da,// 213 PAY 210 

    0x85949fd2,// 214 PAY 211 

    0x44d135a9,// 215 PAY 212 

    0x545a071a,// 216 PAY 213 

    0x48b401fe,// 217 PAY 214 

    0x3c605946,// 218 PAY 215 

    0xc3ef674b,// 219 PAY 216 

    0x5d4b1578,// 220 PAY 217 

    0x3f71533a,// 221 PAY 218 

    0x974454ff,// 222 PAY 219 

    0x62332356,// 223 PAY 220 

    0xc5dd4c62,// 224 PAY 221 

    0xea4ac58f,// 225 PAY 222 

/// HASH is  12 bytes 

    0x86501747,// 226 HSH   1 

    0xcde073ba,// 227 HSH   2 

    0xce434bb5,// 228 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 162 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0051cea2 // 229 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt5_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 418 words. 

/// BDA size     is 1668 (0x684) 

/// BDA id       is 0x93b0 

    0x068493b0,// 3 BDA   1 

/// PAY Generic Data size   : 1668 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x859bdb70,// 4 PAY   1 

    0x1b44da2f,// 5 PAY   2 

    0xcca7e3a2,// 6 PAY   3 

    0x781c4a0f,// 7 PAY   4 

    0xd1c5b0f9,// 8 PAY   5 

    0x5e421ec9,// 9 PAY   6 

    0x0ffc7201,// 10 PAY   7 

    0x84d51b29,// 11 PAY   8 

    0x2776564e,// 12 PAY   9 

    0x9d142aae,// 13 PAY  10 

    0x6dfe9486,// 14 PAY  11 

    0x0e0bcb54,// 15 PAY  12 

    0xff3c70e6,// 16 PAY  13 

    0x514ebae5,// 17 PAY  14 

    0x598a5d4d,// 18 PAY  15 

    0xc8d6fedf,// 19 PAY  16 

    0x73214929,// 20 PAY  17 

    0x3cfff304,// 21 PAY  18 

    0x0369c8ba,// 22 PAY  19 

    0xfd8fc0bb,// 23 PAY  20 

    0xd333ac1e,// 24 PAY  21 

    0x3992e941,// 25 PAY  22 

    0x311439cd,// 26 PAY  23 

    0x5886a9f9,// 27 PAY  24 

    0xaaa5ec2f,// 28 PAY  25 

    0xf71806d8,// 29 PAY  26 

    0x60281b3b,// 30 PAY  27 

    0x93b7c3ed,// 31 PAY  28 

    0x43c3921c,// 32 PAY  29 

    0xe0d26ee2,// 33 PAY  30 

    0xda9595db,// 34 PAY  31 

    0x2eebb757,// 35 PAY  32 

    0x293ad3dd,// 36 PAY  33 

    0x244f28ea,// 37 PAY  34 

    0xf6963a39,// 38 PAY  35 

    0x8fdfccad,// 39 PAY  36 

    0x862a84cc,// 40 PAY  37 

    0x2aca9172,// 41 PAY  38 

    0x410a4cad,// 42 PAY  39 

    0x7e69afc4,// 43 PAY  40 

    0x49779508,// 44 PAY  41 

    0xaa753d80,// 45 PAY  42 

    0x9c0a2ee8,// 46 PAY  43 

    0x94d78ddf,// 47 PAY  44 

    0xccb1ccbf,// 48 PAY  45 

    0x68058d40,// 49 PAY  46 

    0xb13faa40,// 50 PAY  47 

    0xfd235483,// 51 PAY  48 

    0xb58e7c98,// 52 PAY  49 

    0x6ebde46d,// 53 PAY  50 

    0xc773e68c,// 54 PAY  51 

    0x5697d2df,// 55 PAY  52 

    0x36e2e9c1,// 56 PAY  53 

    0x86d3bd3a,// 57 PAY  54 

    0xf3a61b89,// 58 PAY  55 

    0x96dc0490,// 59 PAY  56 

    0x7115148b,// 60 PAY  57 

    0x6a12b146,// 61 PAY  58 

    0x66c53c6e,// 62 PAY  59 

    0x92db6a64,// 63 PAY  60 

    0xe33b0886,// 64 PAY  61 

    0x16099a3b,// 65 PAY  62 

    0x9c36dd73,// 66 PAY  63 

    0x023e5bf1,// 67 PAY  64 

    0xcdf6d58c,// 68 PAY  65 

    0x18c31709,// 69 PAY  66 

    0x557b5cfc,// 70 PAY  67 

    0x8303c857,// 71 PAY  68 

    0xccbebb85,// 72 PAY  69 

    0x4fe96170,// 73 PAY  70 

    0x4a962de1,// 74 PAY  71 

    0x90506cef,// 75 PAY  72 

    0x3f798d21,// 76 PAY  73 

    0x7784bbde,// 77 PAY  74 

    0x8cf6bb8d,// 78 PAY  75 

    0xf29e4545,// 79 PAY  76 

    0x8a13e251,// 80 PAY  77 

    0xffda1fc5,// 81 PAY  78 

    0xb97c4f84,// 82 PAY  79 

    0x29179a3f,// 83 PAY  80 

    0x944e6bc0,// 84 PAY  81 

    0x5cbbe72e,// 85 PAY  82 

    0x14473f46,// 86 PAY  83 

    0x2a380928,// 87 PAY  84 

    0x89a50759,// 88 PAY  85 

    0x8579ab0f,// 89 PAY  86 

    0x07911953,// 90 PAY  87 

    0x1121d11e,// 91 PAY  88 

    0x23d7062b,// 92 PAY  89 

    0xc80cce02,// 93 PAY  90 

    0x506f616f,// 94 PAY  91 

    0x463e3ac3,// 95 PAY  92 

    0xf73b3189,// 96 PAY  93 

    0x65263aa9,// 97 PAY  94 

    0xea22d3ec,// 98 PAY  95 

    0x24ba946c,// 99 PAY  96 

    0xb132df82,// 100 PAY  97 

    0x44f1f1fc,// 101 PAY  98 

    0xc2997c01,// 102 PAY  99 

    0x4a045646,// 103 PAY 100 

    0x10b21ac1,// 104 PAY 101 

    0x9a525509,// 105 PAY 102 

    0x53dad7ca,// 106 PAY 103 

    0x00160dd2,// 107 PAY 104 

    0x3a12c010,// 108 PAY 105 

    0xd7c6acf9,// 109 PAY 106 

    0x3d95c33b,// 110 PAY 107 

    0x71c687bd,// 111 PAY 108 

    0xc00612a2,// 112 PAY 109 

    0x546c3b5e,// 113 PAY 110 

    0xc29d1448,// 114 PAY 111 

    0x1049063f,// 115 PAY 112 

    0x989ac79b,// 116 PAY 113 

    0x8dcf7d38,// 117 PAY 114 

    0xacaecc3d,// 118 PAY 115 

    0x19af538b,// 119 PAY 116 

    0x58435560,// 120 PAY 117 

    0x2636ed2d,// 121 PAY 118 

    0x46bf5c07,// 122 PAY 119 

    0x1b699171,// 123 PAY 120 

    0x5ce77a63,// 124 PAY 121 

    0x4502a6cf,// 125 PAY 122 

    0x99015c98,// 126 PAY 123 

    0x66347fdf,// 127 PAY 124 

    0x62207176,// 128 PAY 125 

    0x926942a8,// 129 PAY 126 

    0x60b4ff39,// 130 PAY 127 

    0x845b1122,// 131 PAY 128 

    0x99f87b91,// 132 PAY 129 

    0xf0ea5e93,// 133 PAY 130 

    0x8aac17d5,// 134 PAY 131 

    0xcc18c2e5,// 135 PAY 132 

    0x20254fac,// 136 PAY 133 

    0x1fb69c76,// 137 PAY 134 

    0x3460ea61,// 138 PAY 135 

    0x4b56dda8,// 139 PAY 136 

    0xd7d352ab,// 140 PAY 137 

    0x3fc4b935,// 141 PAY 138 

    0xbfaf2419,// 142 PAY 139 

    0x5a12ede5,// 143 PAY 140 

    0xf7d0496d,// 144 PAY 141 

    0x24301a6b,// 145 PAY 142 

    0x4bd63197,// 146 PAY 143 

    0xafe29e9e,// 147 PAY 144 

    0x3b8d1618,// 148 PAY 145 

    0x96f3a574,// 149 PAY 146 

    0xd79a8957,// 150 PAY 147 

    0x4d466390,// 151 PAY 148 

    0x8fa4b618,// 152 PAY 149 

    0x7b028dd6,// 153 PAY 150 

    0x03d19e17,// 154 PAY 151 

    0xb3cf22c2,// 155 PAY 152 

    0x1190df1b,// 156 PAY 153 

    0x0444c148,// 157 PAY 154 

    0x3cbd0066,// 158 PAY 155 

    0xd0bc3659,// 159 PAY 156 

    0x87cfd9ca,// 160 PAY 157 

    0xe2681a70,// 161 PAY 158 

    0x6a8b9dfd,// 162 PAY 159 

    0x8079ca0e,// 163 PAY 160 

    0x74cb2f3a,// 164 PAY 161 

    0xc9ec5319,// 165 PAY 162 

    0x86377421,// 166 PAY 163 

    0x57058b19,// 167 PAY 164 

    0x721f7513,// 168 PAY 165 

    0x65670a79,// 169 PAY 166 

    0xf9bb96aa,// 170 PAY 167 

    0xb64ce6d1,// 171 PAY 168 

    0x535d414d,// 172 PAY 169 

    0x9c3949da,// 173 PAY 170 

    0x01dac872,// 174 PAY 171 

    0x9d9c8d1a,// 175 PAY 172 

    0xb53f35c3,// 176 PAY 173 

    0x23068041,// 177 PAY 174 

    0x832aff31,// 178 PAY 175 

    0x377cec9f,// 179 PAY 176 

    0xffa8ea98,// 180 PAY 177 

    0x9e548eea,// 181 PAY 178 

    0x52ee8a41,// 182 PAY 179 

    0x31eba8a1,// 183 PAY 180 

    0xf5bcebd1,// 184 PAY 181 

    0x656c1c62,// 185 PAY 182 

    0xb0562511,// 186 PAY 183 

    0x38c1ec94,// 187 PAY 184 

    0x0eb47a0b,// 188 PAY 185 

    0x0f938c95,// 189 PAY 186 

    0x142c8c91,// 190 PAY 187 

    0x39667f16,// 191 PAY 188 

    0xbe5211c3,// 192 PAY 189 

    0x40f3eab8,// 193 PAY 190 

    0x3ec05799,// 194 PAY 191 

    0x63beca2c,// 195 PAY 192 

    0x0ce90189,// 196 PAY 193 

    0xf2f523be,// 197 PAY 194 

    0x0521f5af,// 198 PAY 195 

    0x28c5b335,// 199 PAY 196 

    0x41a454b3,// 200 PAY 197 

    0xf0a4a0a8,// 201 PAY 198 

    0x92c32f7a,// 202 PAY 199 

    0xdd54ffd4,// 203 PAY 200 

    0x8f48332e,// 204 PAY 201 

    0x5e309e3b,// 205 PAY 202 

    0xab1ddd2c,// 206 PAY 203 

    0x213f8019,// 207 PAY 204 

    0x9b265515,// 208 PAY 205 

    0xf5a912c0,// 209 PAY 206 

    0xc7689eba,// 210 PAY 207 

    0x46dfa4a2,// 211 PAY 208 

    0xe735b70b,// 212 PAY 209 

    0x2cdbb296,// 213 PAY 210 

    0x9f020ff1,// 214 PAY 211 

    0x4b60ae59,// 215 PAY 212 

    0x49dcc81f,// 216 PAY 213 

    0xf3ae6dbd,// 217 PAY 214 

    0x3f295801,// 218 PAY 215 

    0x22123895,// 219 PAY 216 

    0x2af91b11,// 220 PAY 217 

    0x4a7eb985,// 221 PAY 218 

    0x9cb1c441,// 222 PAY 219 

    0x85dc1439,// 223 PAY 220 

    0x5fd167cb,// 224 PAY 221 

    0x1b402a27,// 225 PAY 222 

    0xca7c73f1,// 226 PAY 223 

    0xc740e0fd,// 227 PAY 224 

    0xce2042df,// 228 PAY 225 

    0x174394a0,// 229 PAY 226 

    0xec486231,// 230 PAY 227 

    0xb5fcb45b,// 231 PAY 228 

    0xd43278a6,// 232 PAY 229 

    0x604301a6,// 233 PAY 230 

    0x6b40149b,// 234 PAY 231 

    0x28d55eb5,// 235 PAY 232 

    0x09c65999,// 236 PAY 233 

    0xfec805f3,// 237 PAY 234 

    0xd3033dd0,// 238 PAY 235 

    0xa538f918,// 239 PAY 236 

    0xeb7966f5,// 240 PAY 237 

    0x3a891b6b,// 241 PAY 238 

    0x702efb09,// 242 PAY 239 

    0x8168c724,// 243 PAY 240 

    0x4f941595,// 244 PAY 241 

    0x7908f5f0,// 245 PAY 242 

    0xf3513a28,// 246 PAY 243 

    0xb326c0b0,// 247 PAY 244 

    0x6a6c4d11,// 248 PAY 245 

    0x70b7b3d6,// 249 PAY 246 

    0xbc2e9f1d,// 250 PAY 247 

    0x25062c6d,// 251 PAY 248 

    0x887cfa08,// 252 PAY 249 

    0x4936ae61,// 253 PAY 250 

    0xa1a9c71d,// 254 PAY 251 

    0x5ecc4fbc,// 255 PAY 252 

    0x93db76d2,// 256 PAY 253 

    0x646b0872,// 257 PAY 254 

    0x6d42e156,// 258 PAY 255 

    0xbd29aed2,// 259 PAY 256 

    0x8808b23e,// 260 PAY 257 

    0x39abd139,// 261 PAY 258 

    0x4bd26a78,// 262 PAY 259 

    0xb792f5e1,// 263 PAY 260 

    0x2253d1a2,// 264 PAY 261 

    0x6b74d983,// 265 PAY 262 

    0x4e71411c,// 266 PAY 263 

    0xe5440340,// 267 PAY 264 

    0x3e622e13,// 268 PAY 265 

    0x09bf1658,// 269 PAY 266 

    0xb21a0815,// 270 PAY 267 

    0x80300d61,// 271 PAY 268 

    0x88b9e24d,// 272 PAY 269 

    0x28521ce1,// 273 PAY 270 

    0xcd18319f,// 274 PAY 271 

    0x9c76a0ce,// 275 PAY 272 

    0x4a3006cc,// 276 PAY 273 

    0xa2f26ebd,// 277 PAY 274 

    0xe4f8e9d5,// 278 PAY 275 

    0x650e3fd5,// 279 PAY 276 

    0xddfe95d7,// 280 PAY 277 

    0xc6f557ac,// 281 PAY 278 

    0x95eeabef,// 282 PAY 279 

    0x10aa9f1a,// 283 PAY 280 

    0x6a4d0ebf,// 284 PAY 281 

    0x9a212275,// 285 PAY 282 

    0xbe2bc602,// 286 PAY 283 

    0xa1e3c1ed,// 287 PAY 284 

    0xb95832e3,// 288 PAY 285 

    0xd11c419d,// 289 PAY 286 

    0xc4417b36,// 290 PAY 287 

    0xe09e9118,// 291 PAY 288 

    0x80660847,// 292 PAY 289 

    0x9f0bb8dd,// 293 PAY 290 

    0x2a57c097,// 294 PAY 291 

    0xc2ff1367,// 295 PAY 292 

    0xf77681f9,// 296 PAY 293 

    0x07ce3428,// 297 PAY 294 

    0x27087e23,// 298 PAY 295 

    0xc4c63a96,// 299 PAY 296 

    0x9b7f6fba,// 300 PAY 297 

    0xad6bab7b,// 301 PAY 298 

    0xc195fdcc,// 302 PAY 299 

    0x0d8839d8,// 303 PAY 300 

    0xa7926fe0,// 304 PAY 301 

    0x4cd56601,// 305 PAY 302 

    0xab0e1140,// 306 PAY 303 

    0x77e961d6,// 307 PAY 304 

    0x7aeaffce,// 308 PAY 305 

    0x4e534b1d,// 309 PAY 306 

    0x80d9c182,// 310 PAY 307 

    0x6c27c05e,// 311 PAY 308 

    0x976a6f33,// 312 PAY 309 

    0xd59c33d5,// 313 PAY 310 

    0xf7a0e752,// 314 PAY 311 

    0xa62da4dd,// 315 PAY 312 

    0x64e0197f,// 316 PAY 313 

    0x70509560,// 317 PAY 314 

    0x544f0ce1,// 318 PAY 315 

    0xbda549fc,// 319 PAY 316 

    0xc4a050aa,// 320 PAY 317 

    0xd7c57a27,// 321 PAY 318 

    0x5c100d58,// 322 PAY 319 

    0xc4d9f17d,// 323 PAY 320 

    0x53d8ebaf,// 324 PAY 321 

    0x035cc7b7,// 325 PAY 322 

    0x19b94e92,// 326 PAY 323 

    0x774db318,// 327 PAY 324 

    0xfdc0a61f,// 328 PAY 325 

    0x20e49b95,// 329 PAY 326 

    0x8d8c38b7,// 330 PAY 327 

    0x8576719f,// 331 PAY 328 

    0x403402f1,// 332 PAY 329 

    0x04b5e7cf,// 333 PAY 330 

    0x4f944bfe,// 334 PAY 331 

    0xb1e42617,// 335 PAY 332 

    0x50aa9b84,// 336 PAY 333 

    0xae022f87,// 337 PAY 334 

    0xd0456977,// 338 PAY 335 

    0xb0fd77ab,// 339 PAY 336 

    0x7a715674,// 340 PAY 337 

    0xe8f297cd,// 341 PAY 338 

    0x846526cb,// 342 PAY 339 

    0xc071e2f1,// 343 PAY 340 

    0x8a5b0c58,// 344 PAY 341 

    0x1b81169a,// 345 PAY 342 

    0x511ff082,// 346 PAY 343 

    0x2e00a3d4,// 347 PAY 344 

    0xf244abb8,// 348 PAY 345 

    0xc7dcebc9,// 349 PAY 346 

    0xaf26ab6b,// 350 PAY 347 

    0x3a0910ff,// 351 PAY 348 

    0xf074286b,// 352 PAY 349 

    0x888bea09,// 353 PAY 350 

    0x63970b76,// 354 PAY 351 

    0x25c51e22,// 355 PAY 352 

    0xa1faba6d,// 356 PAY 353 

    0x9f93d510,// 357 PAY 354 

    0x3fd05b43,// 358 PAY 355 

    0xf7073999,// 359 PAY 356 

    0xa3e1b965,// 360 PAY 357 

    0xb572a0be,// 361 PAY 358 

    0x3ddb8688,// 362 PAY 359 

    0x6fcb767f,// 363 PAY 360 

    0x67145efa,// 364 PAY 361 

    0xbefd8062,// 365 PAY 362 

    0x40d382ae,// 366 PAY 363 

    0xc060e924,// 367 PAY 364 

    0x2b16a2ca,// 368 PAY 365 

    0x9945e9d0,// 369 PAY 366 

    0x42b5f3df,// 370 PAY 367 

    0xd3b3e58e,// 371 PAY 368 

    0x8894b7d2,// 372 PAY 369 

    0xeb2937f3,// 373 PAY 370 

    0x0db6ae60,// 374 PAY 371 

    0xfef4f9ca,// 375 PAY 372 

    0x6f34ec46,// 376 PAY 373 

    0x874fa141,// 377 PAY 374 

    0x1e50a112,// 378 PAY 375 

    0x2187f56c,// 379 PAY 376 

    0xffb3519d,// 380 PAY 377 

    0x95350b20,// 381 PAY 378 

    0xf6634c9e,// 382 PAY 379 

    0x71f3e920,// 383 PAY 380 

    0x706c5a8a,// 384 PAY 381 

    0xe925f846,// 385 PAY 382 

    0x9d1d4980,// 386 PAY 383 

    0x39921b1d,// 387 PAY 384 

    0xd737259e,// 388 PAY 385 

    0x6564c0c5,// 389 PAY 386 

    0x88cc3aca,// 390 PAY 387 

    0x694a44a8,// 391 PAY 388 

    0xae8df30c,// 392 PAY 389 

    0xfd390fb8,// 393 PAY 390 

    0x4b9d3b38,// 394 PAY 391 

    0xa440e0f1,// 395 PAY 392 

    0xc8e1d5cd,// 396 PAY 393 

    0x688a7997,// 397 PAY 394 

    0x52956f97,// 398 PAY 395 

    0x4ac115bd,// 399 PAY 396 

    0x5ac2f51a,// 400 PAY 397 

    0x70668846,// 401 PAY 398 

    0x43e5d4c8,// 402 PAY 399 

    0x83b4d207,// 403 PAY 400 

    0x4cc21236,// 404 PAY 401 

    0xf982e2d6,// 405 PAY 402 

    0x647f5e1f,// 406 PAY 403 

    0x9c6cca6e,// 407 PAY 404 

    0x7799fb1d,// 408 PAY 405 

    0xcef89f5d,// 409 PAY 406 

    0xd7889140,// 410 PAY 407 

    0xdcefa07f,// 411 PAY 408 

    0x388d7622,// 412 PAY 409 

    0x8c11b723,// 413 PAY 410 

    0x558f5839,// 414 PAY 411 

    0xd60e96e6,// 415 PAY 412 

    0xf013c9ad,// 416 PAY 413 

    0x41302c10,// 417 PAY 414 

    0xedc5c112,// 418 PAY 415 

    0x5f0a810b,// 419 PAY 416 

    0xdd947454,// 420 PAY 417 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x20 

    0x02492053 // 421 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt6_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 256 words. 

/// BDA size     is 1020 (0x3fc) 

/// BDA id       is 0x4456 

    0x03fc4456,// 3 BDA   1 

/// PAY Generic Data size   : 1020 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x5b771adb,// 4 PAY   1 

    0xc88449a0,// 5 PAY   2 

    0xf9ee1d00,// 6 PAY   3 

    0x1d26ea03,// 7 PAY   4 

    0xb931d1f2,// 8 PAY   5 

    0x9e94eeff,// 9 PAY   6 

    0x8e4cce2b,// 10 PAY   7 

    0xa9cca7b8,// 11 PAY   8 

    0xd139ac02,// 12 PAY   9 

    0xb254164a,// 13 PAY  10 

    0xa5ca2171,// 14 PAY  11 

    0xa86fc941,// 15 PAY  12 

    0x25ea8c83,// 16 PAY  13 

    0x6eca2ec0,// 17 PAY  14 

    0x21df7d8e,// 18 PAY  15 

    0x2f39cb04,// 19 PAY  16 

    0xf2176c2e,// 20 PAY  17 

    0xee91288d,// 21 PAY  18 

    0x2a805af8,// 22 PAY  19 

    0xd3e0ab30,// 23 PAY  20 

    0x459abab7,// 24 PAY  21 

    0xab154117,// 25 PAY  22 

    0x2519d51e,// 26 PAY  23 

    0x81c3eea0,// 27 PAY  24 

    0xa4a5eeb0,// 28 PAY  25 

    0xfad91757,// 29 PAY  26 

    0x22ef5d79,// 30 PAY  27 

    0x79c0a49e,// 31 PAY  28 

    0x3bc31dd9,// 32 PAY  29 

    0x0a8f75b4,// 33 PAY  30 

    0x3a911387,// 34 PAY  31 

    0xc6dd1961,// 35 PAY  32 

    0x6580891f,// 36 PAY  33 

    0x91c27be1,// 37 PAY  34 

    0x484be6cd,// 38 PAY  35 

    0x7f1afb86,// 39 PAY  36 

    0x7d2f5f19,// 40 PAY  37 

    0x06f6a856,// 41 PAY  38 

    0x82ae7fec,// 42 PAY  39 

    0xd0b98dfd,// 43 PAY  40 

    0x0db859a1,// 44 PAY  41 

    0xd15e61dc,// 45 PAY  42 

    0x5ad313b4,// 46 PAY  43 

    0x5b44e27b,// 47 PAY  44 

    0xe6698e99,// 48 PAY  45 

    0xe2eb9c28,// 49 PAY  46 

    0xa6c67960,// 50 PAY  47 

    0x00ceb51b,// 51 PAY  48 

    0x2a436959,// 52 PAY  49 

    0x9266a1dc,// 53 PAY  50 

    0xf6c532ad,// 54 PAY  51 

    0x6b12d76a,// 55 PAY  52 

    0x2fe16663,// 56 PAY  53 

    0x0f591eea,// 57 PAY  54 

    0x35289d4a,// 58 PAY  55 

    0x016e6d27,// 59 PAY  56 

    0xf68a5841,// 60 PAY  57 

    0x78196903,// 61 PAY  58 

    0x23f2c675,// 62 PAY  59 

    0x9fe61437,// 63 PAY  60 

    0x533c0c0a,// 64 PAY  61 

    0x2f38dd2a,// 65 PAY  62 

    0x46c13b4b,// 66 PAY  63 

    0x76081e60,// 67 PAY  64 

    0x94712d99,// 68 PAY  65 

    0x189e3c4e,// 69 PAY  66 

    0x332a397e,// 70 PAY  67 

    0x8abe0574,// 71 PAY  68 

    0x1ce4ca2a,// 72 PAY  69 

    0x3c7452a6,// 73 PAY  70 

    0xa0783dd5,// 74 PAY  71 

    0xd5758451,// 75 PAY  72 

    0x7d2c7f7a,// 76 PAY  73 

    0x9ff87980,// 77 PAY  74 

    0x6142d783,// 78 PAY  75 

    0xdb817157,// 79 PAY  76 

    0x6e2423ca,// 80 PAY  77 

    0x39910e9f,// 81 PAY  78 

    0x79960acf,// 82 PAY  79 

    0xd499ba46,// 83 PAY  80 

    0x81eef62e,// 84 PAY  81 

    0xa6ccc6f0,// 85 PAY  82 

    0x40e078e3,// 86 PAY  83 

    0x2dcb285c,// 87 PAY  84 

    0x3a4f3608,// 88 PAY  85 

    0x765e658a,// 89 PAY  86 

    0x7c48a3db,// 90 PAY  87 

    0xefacfb19,// 91 PAY  88 

    0x14d2b986,// 92 PAY  89 

    0x1d40c4e0,// 93 PAY  90 

    0xbd66d086,// 94 PAY  91 

    0xf094d2db,// 95 PAY  92 

    0x5d05711a,// 96 PAY  93 

    0xa58cc855,// 97 PAY  94 

    0x40e6500b,// 98 PAY  95 

    0x6a056baf,// 99 PAY  96 

    0xdf5f9cc5,// 100 PAY  97 

    0x38064a44,// 101 PAY  98 

    0x2e3232ea,// 102 PAY  99 

    0xe9ca61ca,// 103 PAY 100 

    0xa767233c,// 104 PAY 101 

    0x7b7d59d1,// 105 PAY 102 

    0xb36257c9,// 106 PAY 103 

    0x8d377a3e,// 107 PAY 104 

    0xfdd95d90,// 108 PAY 105 

    0x33732497,// 109 PAY 106 

    0x7305e5f3,// 110 PAY 107 

    0x3d44e229,// 111 PAY 108 

    0xde0821aa,// 112 PAY 109 

    0xc376b2fa,// 113 PAY 110 

    0xc023dd47,// 114 PAY 111 

    0xf1092d09,// 115 PAY 112 

    0x540c6fef,// 116 PAY 113 

    0x7a76c0f1,// 117 PAY 114 

    0xdbca7e3a,// 118 PAY 115 

    0x891d8354,// 119 PAY 116 

    0x1e8b70e3,// 120 PAY 117 

    0xe4099b5f,// 121 PAY 118 

    0xf111ff68,// 122 PAY 119 

    0x919bfc92,// 123 PAY 120 

    0x14db5fe1,// 124 PAY 121 

    0xb731034e,// 125 PAY 122 

    0x5b41de56,// 126 PAY 123 

    0x2dc48a73,// 127 PAY 124 

    0xe7db8c47,// 128 PAY 125 

    0x169efc1a,// 129 PAY 126 

    0xa1a92c8a,// 130 PAY 127 

    0x1fdc7704,// 131 PAY 128 

    0xbef344bf,// 132 PAY 129 

    0xf8ebaa92,// 133 PAY 130 

    0x1814ab84,// 134 PAY 131 

    0x7e90971c,// 135 PAY 132 

    0xed09d208,// 136 PAY 133 

    0xe989f44d,// 137 PAY 134 

    0x39dbf19e,// 138 PAY 135 

    0x8d82a3bc,// 139 PAY 136 

    0xce120aed,// 140 PAY 137 

    0x7bd1da77,// 141 PAY 138 

    0x313ae237,// 142 PAY 139 

    0x6b4427f4,// 143 PAY 140 

    0x2c9289cd,// 144 PAY 141 

    0xe1cad597,// 145 PAY 142 

    0x6ae1bc01,// 146 PAY 143 

    0xe94a1699,// 147 PAY 144 

    0x4d5c50e4,// 148 PAY 145 

    0xf41962ef,// 149 PAY 146 

    0xe5e1629f,// 150 PAY 147 

    0x2b700d2f,// 151 PAY 148 

    0x706cd74a,// 152 PAY 149 

    0x9f0abfc1,// 153 PAY 150 

    0x545e4f8e,// 154 PAY 151 

    0x809c2787,// 155 PAY 152 

    0xa00a3a1e,// 156 PAY 153 

    0xfd318b33,// 157 PAY 154 

    0xd09b44f1,// 158 PAY 155 

    0xa928b4c6,// 159 PAY 156 

    0xd43c4b2c,// 160 PAY 157 

    0x70863114,// 161 PAY 158 

    0xcb4fb3b6,// 162 PAY 159 

    0xed9865c1,// 163 PAY 160 

    0x6a6abfd2,// 164 PAY 161 

    0x3329faba,// 165 PAY 162 

    0x829015a9,// 166 PAY 163 

    0x16581954,// 167 PAY 164 

    0x5ebc682d,// 168 PAY 165 

    0xe205d2ba,// 169 PAY 166 

    0x7d850073,// 170 PAY 167 

    0x15365694,// 171 PAY 168 

    0x961c07ba,// 172 PAY 169 

    0x647aacc1,// 173 PAY 170 

    0x74924e19,// 174 PAY 171 

    0x5ec9b543,// 175 PAY 172 

    0xa5a3db26,// 176 PAY 173 

    0x30120dfc,// 177 PAY 174 

    0xba168120,// 178 PAY 175 

    0x6f237746,// 179 PAY 176 

    0x552ca354,// 180 PAY 177 

    0xd7aaa735,// 181 PAY 178 

    0xa3f566c6,// 182 PAY 179 

    0xd0f404c4,// 183 PAY 180 

    0xfe3dbc3f,// 184 PAY 181 

    0x84d57ac8,// 185 PAY 182 

    0x1727618d,// 186 PAY 183 

    0x7df06c4c,// 187 PAY 184 

    0x6496b658,// 188 PAY 185 

    0x31b3c80b,// 189 PAY 186 

    0x2d97e322,// 190 PAY 187 

    0x6c770c68,// 191 PAY 188 

    0x69756623,// 192 PAY 189 

    0x8639ebae,// 193 PAY 190 

    0xd7105f92,// 194 PAY 191 

    0x9bdb4b18,// 195 PAY 192 

    0x110d2d86,// 196 PAY 193 

    0x67ad4d3a,// 197 PAY 194 

    0xcf270391,// 198 PAY 195 

    0x732b424e,// 199 PAY 196 

    0xeed2405c,// 200 PAY 197 

    0xb24df920,// 201 PAY 198 

    0x4515a6b7,// 202 PAY 199 

    0x74282536,// 203 PAY 200 

    0x1eef2183,// 204 PAY 201 

    0x7dea220b,// 205 PAY 202 

    0xa498a567,// 206 PAY 203 

    0x57ca2d8f,// 207 PAY 204 

    0x45aca0d8,// 208 PAY 205 

    0xb69fe4dd,// 209 PAY 206 

    0x469fd455,// 210 PAY 207 

    0xdbfe1a08,// 211 PAY 208 

    0xce268376,// 212 PAY 209 

    0x9481d97e,// 213 PAY 210 

    0x19918f54,// 214 PAY 211 

    0x8ad5481e,// 215 PAY 212 

    0x0dbc6726,// 216 PAY 213 

    0xdf13a634,// 217 PAY 214 

    0x1b60fec6,// 218 PAY 215 

    0xb5a064fc,// 219 PAY 216 

    0x355ec299,// 220 PAY 217 

    0x57b733b8,// 221 PAY 218 

    0x8d9a7e82,// 222 PAY 219 

    0x806e1a5d,// 223 PAY 220 

    0xde5bac48,// 224 PAY 221 

    0x70867048,// 225 PAY 222 

    0xba437dfd,// 226 PAY 223 

    0xd403c90d,// 227 PAY 224 

    0x273e89f5,// 228 PAY 225 

    0x9d36b1f2,// 229 PAY 226 

    0x1401ea32,// 230 PAY 227 

    0x932325f9,// 231 PAY 228 

    0x3c23aba9,// 232 PAY 229 

    0x0f168df4,// 233 PAY 230 

    0x8c9b3a27,// 234 PAY 231 

    0x9d15d8bc,// 235 PAY 232 

    0x76f0d5ea,// 236 PAY 233 

    0xce1e5604,// 237 PAY 234 

    0x0ddec7e2,// 238 PAY 235 

    0x67f6281c,// 239 PAY 236 

    0x073739a8,// 240 PAY 237 

    0x205c56e4,// 241 PAY 238 

    0xef63ec94,// 242 PAY 239 

    0x6fd392dc,// 243 PAY 240 

    0x91890090,// 244 PAY 241 

    0x7085ed17,// 245 PAY 242 

    0xb073ceab,// 246 PAY 243 

    0x558ee07c,// 247 PAY 244 

    0xffeb1a97,// 248 PAY 245 

    0x1b55ac65,// 249 PAY 246 

    0x4b74a900,// 250 PAY 247 

    0x72fd231e,// 251 PAY 248 

    0x42f4c8d2,// 252 PAY 249 

    0x80b32f98,// 253 PAY 250 

    0x7dea680c,// 254 PAY 251 

    0x5e264605,// 255 PAY 252 

    0x83176bb5,// 256 PAY 253 

    0x3c818bb7,// 257 PAY 254 

    0x7b360e31,// 258 PAY 255 

/// HASH is  12 bytes 

    0x9266a1dc,// 259 HSH   1 

    0xf6c532ad,// 260 HSH   2 

    0x6b12d76a,// 261 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 34 

/// STA pkt_idx        : 231 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdf 

    0x039cdf22 // 262 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt7_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 124 words. 

/// BDA size     is 490 (0x1ea) 

/// BDA id       is 0xd61 

    0x01ea0d61,// 3 BDA   1 

/// PAY Generic Data size   : 490 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x18095884,// 4 PAY   1 

    0xfd235b3e,// 5 PAY   2 

    0x2c9cde9b,// 6 PAY   3 

    0x2a2e2eb3,// 7 PAY   4 

    0xc7294421,// 8 PAY   5 

    0x130f7947,// 9 PAY   6 

    0x8f6fc691,// 10 PAY   7 

    0x2c0ddd8e,// 11 PAY   8 

    0x017960a6,// 12 PAY   9 

    0x75b23f87,// 13 PAY  10 

    0x63efbead,// 14 PAY  11 

    0xa4111f6a,// 15 PAY  12 

    0x63048c58,// 16 PAY  13 

    0x246a8061,// 17 PAY  14 

    0x3329669e,// 18 PAY  15 

    0x833939a3,// 19 PAY  16 

    0x364738f1,// 20 PAY  17 

    0xd305bfb2,// 21 PAY  18 

    0x1be52755,// 22 PAY  19 

    0xeab155d2,// 23 PAY  20 

    0xd4090b26,// 24 PAY  21 

    0x92240d75,// 25 PAY  22 

    0x87aa63f8,// 26 PAY  23 

    0xe2bf9fb4,// 27 PAY  24 

    0x63c29f54,// 28 PAY  25 

    0x654c96f7,// 29 PAY  26 

    0x449ac839,// 30 PAY  27 

    0x634c2195,// 31 PAY  28 

    0x0f1bb796,// 32 PAY  29 

    0x12a48fac,// 33 PAY  30 

    0xd446f50d,// 34 PAY  31 

    0x145cc428,// 35 PAY  32 

    0xa65a66fb,// 36 PAY  33 

    0xe0842f32,// 37 PAY  34 

    0x94ab320f,// 38 PAY  35 

    0xeeaeac5c,// 39 PAY  36 

    0xf4b51f69,// 40 PAY  37 

    0x3847bb99,// 41 PAY  38 

    0xd884d33b,// 42 PAY  39 

    0x3f38dbe5,// 43 PAY  40 

    0xf9f758ca,// 44 PAY  41 

    0xd5e69317,// 45 PAY  42 

    0x9b408939,// 46 PAY  43 

    0xa137088a,// 47 PAY  44 

    0xad6886f4,// 48 PAY  45 

    0xfcb7dbf3,// 49 PAY  46 

    0xaa539ce4,// 50 PAY  47 

    0xb4e5c319,// 51 PAY  48 

    0x7f8b4b15,// 52 PAY  49 

    0x8730ea8b,// 53 PAY  50 

    0x143e0c3e,// 54 PAY  51 

    0x944077cd,// 55 PAY  52 

    0x1c9ca3dc,// 56 PAY  53 

    0xe7c50a2e,// 57 PAY  54 

    0xf6d69221,// 58 PAY  55 

    0x9282a06c,// 59 PAY  56 

    0x6625216e,// 60 PAY  57 

    0xf53daf8b,// 61 PAY  58 

    0xfc39668c,// 62 PAY  59 

    0x99e796b2,// 63 PAY  60 

    0xd5d83ed9,// 64 PAY  61 

    0x1fd2d42f,// 65 PAY  62 

    0x83799589,// 66 PAY  63 

    0xfbd64c09,// 67 PAY  64 

    0xbbbfa2eb,// 68 PAY  65 

    0xd3ab0817,// 69 PAY  66 

    0x0dc590fb,// 70 PAY  67 

    0x79d16e68,// 71 PAY  68 

    0x97e365d1,// 72 PAY  69 

    0xd71a67f7,// 73 PAY  70 

    0x05739977,// 74 PAY  71 

    0xcc301d89,// 75 PAY  72 

    0x9cd900ed,// 76 PAY  73 

    0x29288a5f,// 77 PAY  74 

    0x64bda180,// 78 PAY  75 

    0xe996ed50,// 79 PAY  76 

    0xdd9ed543,// 80 PAY  77 

    0xf08c778d,// 81 PAY  78 

    0xd8dd67ea,// 82 PAY  79 

    0x7ea887f6,// 83 PAY  80 

    0x191f55f9,// 84 PAY  81 

    0xa9518ef4,// 85 PAY  82 

    0xc3cb978c,// 86 PAY  83 

    0x95c47680,// 87 PAY  84 

    0xa309130e,// 88 PAY  85 

    0xc7972f83,// 89 PAY  86 

    0xd5abfad6,// 90 PAY  87 

    0x6b1c837f,// 91 PAY  88 

    0xe9e79b17,// 92 PAY  89 

    0xa9975c98,// 93 PAY  90 

    0x1422218f,// 94 PAY  91 

    0x2ac304e4,// 95 PAY  92 

    0x0beee17a,// 96 PAY  93 

    0x05e4ff9d,// 97 PAY  94 

    0x49c0d20c,// 98 PAY  95 

    0xa8e2cac3,// 99 PAY  96 

    0x5fa1cca8,// 100 PAY  97 

    0xa2cc7d67,// 101 PAY  98 

    0x69299eb8,// 102 PAY  99 

    0x0778a559,// 103 PAY 100 

    0x7ca6ef2b,// 104 PAY 101 

    0x5ac06dce,// 105 PAY 102 

    0xd989b21f,// 106 PAY 103 

    0x43d4bf59,// 107 PAY 104 

    0x93adb471,// 108 PAY 105 

    0x45d9092b,// 109 PAY 106 

    0x51c14524,// 110 PAY 107 

    0xa987f025,// 111 PAY 108 

    0xf09a295b,// 112 PAY 109 

    0xd5904c8c,// 113 PAY 110 

    0xcd8d6854,// 114 PAY 111 

    0xeed87893,// 115 PAY 112 

    0x570adf93,// 116 PAY 113 

    0x9174e1df,// 117 PAY 114 

    0xca5debca,// 118 PAY 115 

    0x31d63649,// 119 PAY 116 

    0x216b7ef0,// 120 PAY 117 

    0x263d24e4,// 121 PAY 118 

    0xec05dc83,// 122 PAY 119 

    0xde932c64,// 123 PAY 120 

    0x2bcdef9e,// 124 PAY 121 

    0xa399ca56,// 125 PAY 122 

    0x6d2d0000,// 126 PAY 123 

/// STA is 1 words. 

/// STA num_pkts       : 140 

/// STA pkt_idx        : 203 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xee 

    0x032dee8c // 127 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt8_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 59 words. 

/// BDA size     is 229 (0xe5) 

/// BDA id       is 0x5dfa 

    0x00e55dfa,// 3 BDA   1 

/// PAY Generic Data size   : 229 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x3e2ee230,// 4 PAY   1 

    0x13f7c291,// 5 PAY   2 

    0x2c645c17,// 6 PAY   3 

    0x16510abb,// 7 PAY   4 

    0xed7b740f,// 8 PAY   5 

    0xc007cd87,// 9 PAY   6 

    0x2d37ebb7,// 10 PAY   7 

    0x060b943e,// 11 PAY   8 

    0x17246f73,// 12 PAY   9 

    0xf611133f,// 13 PAY  10 

    0x2c0874a8,// 14 PAY  11 

    0xb6ccac69,// 15 PAY  12 

    0xd0977495,// 16 PAY  13 

    0x35a97eb3,// 17 PAY  14 

    0xcbe1e3ad,// 18 PAY  15 

    0xd59e3cb5,// 19 PAY  16 

    0x5c2421d8,// 20 PAY  17 

    0xe79391fa,// 21 PAY  18 

    0x3a5d3bd7,// 22 PAY  19 

    0xc3c5443a,// 23 PAY  20 

    0x491263c0,// 24 PAY  21 

    0xc5fbd274,// 25 PAY  22 

    0x9b55af85,// 26 PAY  23 

    0xb82d4ad4,// 27 PAY  24 

    0x33d1a0ad,// 28 PAY  25 

    0xae45ceb9,// 29 PAY  26 

    0xc9b821be,// 30 PAY  27 

    0x46194333,// 31 PAY  28 

    0x4681feec,// 32 PAY  29 

    0x6ee0f2b3,// 33 PAY  30 

    0xf9e3ebf1,// 34 PAY  31 

    0x10e6a604,// 35 PAY  32 

    0xa2a60d81,// 36 PAY  33 

    0xbb6ea3d2,// 37 PAY  34 

    0x24089873,// 38 PAY  35 

    0x51469cac,// 39 PAY  36 

    0x3a6b80a7,// 40 PAY  37 

    0x2f43b8d7,// 41 PAY  38 

    0xf1ed8e61,// 42 PAY  39 

    0x1b319d89,// 43 PAY  40 

    0x2b7fe13f,// 44 PAY  41 

    0x7273a9a4,// 45 PAY  42 

    0xa75d686f,// 46 PAY  43 

    0x0b9c9b42,// 47 PAY  44 

    0x7b8a43b7,// 48 PAY  45 

    0x61d7c6ad,// 49 PAY  46 

    0xe13024cd,// 50 PAY  47 

    0x7bfb789c,// 51 PAY  48 

    0xb3816e92,// 52 PAY  49 

    0x41d96de9,// 53 PAY  50 

    0x0ad5eda8,// 54 PAY  51 

    0x470b6722,// 55 PAY  52 

    0x3435e535,// 56 PAY  53 

    0x7e409c05,// 57 PAY  54 

    0xef5f9fb0,// 58 PAY  55 

    0x76551dfd,// 59 PAY  56 

    0xb57a5f05,// 60 PAY  57 

    0x88000000,// 61 PAY  58 

/// HASH is  4 bytes 

    0x46194333,// 62 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 239 

/// STA pkt_idx        : 63 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x00fc1cef // 63 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt9_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 251 words. 

/// BDA size     is 997 (0x3e5) 

/// BDA id       is 0xc580 

    0x03e5c580,// 3 BDA   1 

/// PAY Generic Data size   : 997 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x5f37921d,// 4 PAY   1 

    0xa7dcc3f4,// 5 PAY   2 

    0xef661c74,// 6 PAY   3 

    0x718e525a,// 7 PAY   4 

    0xcbf42f6c,// 8 PAY   5 

    0x3fbe0265,// 9 PAY   6 

    0x1c2a6329,// 10 PAY   7 

    0x8b4e8044,// 11 PAY   8 

    0x642c96a0,// 12 PAY   9 

    0x87395eb5,// 13 PAY  10 

    0xf60fb5c3,// 14 PAY  11 

    0x76925841,// 15 PAY  12 

    0x2873ede5,// 16 PAY  13 

    0x1493eb0f,// 17 PAY  14 

    0xa58aed0c,// 18 PAY  15 

    0xe06bb36d,// 19 PAY  16 

    0xd59e695d,// 20 PAY  17 

    0x74d1f9b7,// 21 PAY  18 

    0xdc17b52a,// 22 PAY  19 

    0x3103242c,// 23 PAY  20 

    0x95da9776,// 24 PAY  21 

    0x33ad7110,// 25 PAY  22 

    0x88b62a17,// 26 PAY  23 

    0x01987dc8,// 27 PAY  24 

    0xd675e4c1,// 28 PAY  25 

    0x07d5e376,// 29 PAY  26 

    0xaaec28f5,// 30 PAY  27 

    0x3752f0b3,// 31 PAY  28 

    0x69bc8db6,// 32 PAY  29 

    0x7ebdd597,// 33 PAY  30 

    0x8c814f7e,// 34 PAY  31 

    0xf7d659b9,// 35 PAY  32 

    0xbeed46ed,// 36 PAY  33 

    0x0b704ec9,// 37 PAY  34 

    0xa5c8f69d,// 38 PAY  35 

    0x24c91bd8,// 39 PAY  36 

    0x08fb3091,// 40 PAY  37 

    0x91ed390c,// 41 PAY  38 

    0x59c732cc,// 42 PAY  39 

    0x27042b42,// 43 PAY  40 

    0x88e6166e,// 44 PAY  41 

    0x13d5af35,// 45 PAY  42 

    0xba4b4963,// 46 PAY  43 

    0x526a5518,// 47 PAY  44 

    0x8e9dab28,// 48 PAY  45 

    0x7082dde0,// 49 PAY  46 

    0x83c07b70,// 50 PAY  47 

    0xffee1d25,// 51 PAY  48 

    0x86d6d12f,// 52 PAY  49 

    0xd39fcb20,// 53 PAY  50 

    0x1b05a45a,// 54 PAY  51 

    0xe5855ba4,// 55 PAY  52 

    0x0c3fd84d,// 56 PAY  53 

    0xac5e9a9f,// 57 PAY  54 

    0x37cbe98f,// 58 PAY  55 

    0xa86edb60,// 59 PAY  56 

    0xc44509b4,// 60 PAY  57 

    0x4a92ae32,// 61 PAY  58 

    0xe1270a08,// 62 PAY  59 

    0xe80f58fd,// 63 PAY  60 

    0x30bf9db3,// 64 PAY  61 

    0xf1cef3d4,// 65 PAY  62 

    0x22f4d1ae,// 66 PAY  63 

    0xa78ed5e0,// 67 PAY  64 

    0xfcb0143e,// 68 PAY  65 

    0xfc9819d8,// 69 PAY  66 

    0x543b2d8d,// 70 PAY  67 

    0x18166120,// 71 PAY  68 

    0xd974cb27,// 72 PAY  69 

    0xb71c5a56,// 73 PAY  70 

    0x5cc1d42a,// 74 PAY  71 

    0xd53fba8d,// 75 PAY  72 

    0xadb7eae7,// 76 PAY  73 

    0x068d1d97,// 77 PAY  74 

    0x1f3a5114,// 78 PAY  75 

    0xe43006ba,// 79 PAY  76 

    0x7b3bec11,// 80 PAY  77 

    0xf639d4f8,// 81 PAY  78 

    0x30b1cdac,// 82 PAY  79 

    0xe3614b00,// 83 PAY  80 

    0xa6d06b99,// 84 PAY  81 

    0x40d01937,// 85 PAY  82 

    0x2964d657,// 86 PAY  83 

    0x77e5caed,// 87 PAY  84 

    0xe0717519,// 88 PAY  85 

    0x2392827c,// 89 PAY  86 

    0xd816075e,// 90 PAY  87 

    0x53d6f83d,// 91 PAY  88 

    0xfd2a8ae5,// 92 PAY  89 

    0x27ed129f,// 93 PAY  90 

    0x95339c85,// 94 PAY  91 

    0x2ea5a47e,// 95 PAY  92 

    0xf01a3c00,// 96 PAY  93 

    0x557f9118,// 97 PAY  94 

    0x4295a6d5,// 98 PAY  95 

    0x895e79fc,// 99 PAY  96 

    0xea90ce7b,// 100 PAY  97 

    0x171a393a,// 101 PAY  98 

    0xce4f1998,// 102 PAY  99 

    0xefe5203c,// 103 PAY 100 

    0x9d1e0e61,// 104 PAY 101 

    0x00b71487,// 105 PAY 102 

    0x55331314,// 106 PAY 103 

    0xe79af46a,// 107 PAY 104 

    0x1f3d9024,// 108 PAY 105 

    0x1a003c4c,// 109 PAY 106 

    0xcee3d0ba,// 110 PAY 107 

    0xd9e97424,// 111 PAY 108 

    0xad06e774,// 112 PAY 109 

    0xdc1cb956,// 113 PAY 110 

    0xf38c9868,// 114 PAY 111 

    0xb8d1bfcb,// 115 PAY 112 

    0x2d444f57,// 116 PAY 113 

    0x0b13d700,// 117 PAY 114 

    0x1db93204,// 118 PAY 115 

    0xdc1dcfa9,// 119 PAY 116 

    0x22e3f602,// 120 PAY 117 

    0x85dd48f4,// 121 PAY 118 

    0x86452ea7,// 122 PAY 119 

    0x1b77eeeb,// 123 PAY 120 

    0xb7bb5b86,// 124 PAY 121 

    0x7ce6055b,// 125 PAY 122 

    0x0fe70c4b,// 126 PAY 123 

    0x108a770b,// 127 PAY 124 

    0x68def0bc,// 128 PAY 125 

    0x1278d898,// 129 PAY 126 

    0xe158b050,// 130 PAY 127 

    0x9883d88c,// 131 PAY 128 

    0xadad07dd,// 132 PAY 129 

    0x635db64b,// 133 PAY 130 

    0x4333b690,// 134 PAY 131 

    0xff372dd3,// 135 PAY 132 

    0xf3ce2e42,// 136 PAY 133 

    0x8f9e36d1,// 137 PAY 134 

    0x2a0b4ff1,// 138 PAY 135 

    0x2fd24b58,// 139 PAY 136 

    0x4b9236e1,// 140 PAY 137 

    0x171f0376,// 141 PAY 138 

    0x8bba280e,// 142 PAY 139 

    0x948bc3c8,// 143 PAY 140 

    0xebd92812,// 144 PAY 141 

    0x26fe8434,// 145 PAY 142 

    0x955c8cc6,// 146 PAY 143 

    0x8d59d506,// 147 PAY 144 

    0x1d85baef,// 148 PAY 145 

    0x0d44d39a,// 149 PAY 146 

    0xf038a808,// 150 PAY 147 

    0x5a45ea17,// 151 PAY 148 

    0x1dc8c391,// 152 PAY 149 

    0x6999e266,// 153 PAY 150 

    0x4cd32da7,// 154 PAY 151 

    0x289383c2,// 155 PAY 152 

    0x7250ca89,// 156 PAY 153 

    0x706aea8f,// 157 PAY 154 

    0x1bac50e1,// 158 PAY 155 

    0x56304ff0,// 159 PAY 156 

    0x6956f12e,// 160 PAY 157 

    0x15d70d1d,// 161 PAY 158 

    0xbb8aea55,// 162 PAY 159 

    0x66037fae,// 163 PAY 160 

    0xabc38722,// 164 PAY 161 

    0x414bd574,// 165 PAY 162 

    0xd59170c0,// 166 PAY 163 

    0x7f333b9d,// 167 PAY 164 

    0xa70497b5,// 168 PAY 165 

    0x248bca3d,// 169 PAY 166 

    0xf17f2aa0,// 170 PAY 167 

    0x56291fb6,// 171 PAY 168 

    0xcea91870,// 172 PAY 169 

    0x93d4956d,// 173 PAY 170 

    0x4d8409e3,// 174 PAY 171 

    0x67ca620d,// 175 PAY 172 

    0x654b1bde,// 176 PAY 173 

    0xc4cef84d,// 177 PAY 174 

    0x13b2d1b7,// 178 PAY 175 

    0x03fd1f04,// 179 PAY 176 

    0x9cf71d85,// 180 PAY 177 

    0x3102b2ba,// 181 PAY 178 

    0xa2eab23a,// 182 PAY 179 

    0x5cc5071f,// 183 PAY 180 

    0xae13f401,// 184 PAY 181 

    0x672a6420,// 185 PAY 182 

    0x2ca93146,// 186 PAY 183 

    0xeb59fcf5,// 187 PAY 184 

    0x99c14911,// 188 PAY 185 

    0x2a472931,// 189 PAY 186 

    0x07cce5a5,// 190 PAY 187 

    0xc80f05e3,// 191 PAY 188 

    0x85c4a973,// 192 PAY 189 

    0xa23cdc22,// 193 PAY 190 

    0xc140fbc2,// 194 PAY 191 

    0xbf62f425,// 195 PAY 192 

    0x5bd96823,// 196 PAY 193 

    0xf2e5ab45,// 197 PAY 194 

    0xf09c2ac7,// 198 PAY 195 

    0x083fd084,// 199 PAY 196 

    0x27774f88,// 200 PAY 197 

    0xa491c271,// 201 PAY 198 

    0xe7711103,// 202 PAY 199 

    0x7456aaa9,// 203 PAY 200 

    0xac4d04ed,// 204 PAY 201 

    0x4198a83f,// 205 PAY 202 

    0x204f2cc5,// 206 PAY 203 

    0x0ff26bef,// 207 PAY 204 

    0xd75e4eee,// 208 PAY 205 

    0x18e63571,// 209 PAY 206 

    0xb5eba53e,// 210 PAY 207 

    0x318567de,// 211 PAY 208 

    0x45d5eb28,// 212 PAY 209 

    0x2117e443,// 213 PAY 210 

    0xeb3474db,// 214 PAY 211 

    0xb2cdb58b,// 215 PAY 212 

    0x8f96612f,// 216 PAY 213 

    0x20fbe3aa,// 217 PAY 214 

    0xbb332ac9,// 218 PAY 215 

    0x0aa49b37,// 219 PAY 216 

    0xf9dafab1,// 220 PAY 217 

    0xe55db4cc,// 221 PAY 218 

    0x9e600a70,// 222 PAY 219 

    0xdd2a6bd1,// 223 PAY 220 

    0xd2d7f14e,// 224 PAY 221 

    0xc67c34f5,// 225 PAY 222 

    0xaa166dbc,// 226 PAY 223 

    0x640524dd,// 227 PAY 224 

    0xa47b144c,// 228 PAY 225 

    0xab4b809b,// 229 PAY 226 

    0xa2da8e78,// 230 PAY 227 

    0xf560e23e,// 231 PAY 228 

    0xd0afa329,// 232 PAY 229 

    0x71ea0bcb,// 233 PAY 230 

    0x6fa17160,// 234 PAY 231 

    0x39f7fe3f,// 235 PAY 232 

    0xec32211a,// 236 PAY 233 

    0xc922d321,// 237 PAY 234 

    0x353bba67,// 238 PAY 235 

    0x8a26c74a,// 239 PAY 236 

    0x340afb63,// 240 PAY 237 

    0x1b670d8e,// 241 PAY 238 

    0x796b0f3f,// 242 PAY 239 

    0x1c358c30,// 243 PAY 240 

    0xba187d41,// 244 PAY 241 

    0x02163283,// 245 PAY 242 

    0xd0873f0b,// 246 PAY 243 

    0xd3325c42,// 247 PAY 244 

    0x87e3d1f4,// 248 PAY 245 

    0x46db7403,// 249 PAY 246 

    0x92f13eda,// 250 PAY 247 

    0xbc2ed5f1,// 251 PAY 248 

    0x4f81a5e3,// 252 PAY 249 

    0x09000000,// 253 PAY 250 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x71 

    0x01317149 // 254 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt10_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 301 words. 

/// BDA size     is 1199 (0x4af) 

/// BDA id       is 0xdf39 

    0x04afdf39,// 3 BDA   1 

/// PAY Generic Data size   : 1199 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x80c98037,// 4 PAY   1 

    0x3502ffec,// 5 PAY   2 

    0x61068449,// 6 PAY   3 

    0xf286b041,// 7 PAY   4 

    0xac21181d,// 8 PAY   5 

    0xc405afc2,// 9 PAY   6 

    0x050e719a,// 10 PAY   7 

    0x6f19c8c5,// 11 PAY   8 

    0x00a05f83,// 12 PAY   9 

    0x2704ba1d,// 13 PAY  10 

    0x399ed128,// 14 PAY  11 

    0x5f74a05d,// 15 PAY  12 

    0x227fb08d,// 16 PAY  13 

    0x84a3feef,// 17 PAY  14 

    0x377df743,// 18 PAY  15 

    0x651da685,// 19 PAY  16 

    0x32249b21,// 20 PAY  17 

    0x50c59867,// 21 PAY  18 

    0x2e55c3bc,// 22 PAY  19 

    0x58a21851,// 23 PAY  20 

    0x1dfe2c31,// 24 PAY  21 

    0x26ab6eb4,// 25 PAY  22 

    0xb08f8209,// 26 PAY  23 

    0xc915e0e0,// 27 PAY  24 

    0xa0072546,// 28 PAY  25 

    0xe69acccf,// 29 PAY  26 

    0xe7592f96,// 30 PAY  27 

    0xe53bca2a,// 31 PAY  28 

    0x7307368c,// 32 PAY  29 

    0xccb8bee6,// 33 PAY  30 

    0x9860d730,// 34 PAY  31 

    0xf563ffde,// 35 PAY  32 

    0x4831ee20,// 36 PAY  33 

    0xaefa450a,// 37 PAY  34 

    0x7e2c4ae5,// 38 PAY  35 

    0x357b2f17,// 39 PAY  36 

    0x0e476945,// 40 PAY  37 

    0xa266861e,// 41 PAY  38 

    0x731e4cb1,// 42 PAY  39 

    0xcff114c8,// 43 PAY  40 

    0x4cd95c9c,// 44 PAY  41 

    0x129d1446,// 45 PAY  42 

    0xb1370e3c,// 46 PAY  43 

    0x323704f5,// 47 PAY  44 

    0xf8de3d31,// 48 PAY  45 

    0xeae9b371,// 49 PAY  46 

    0x3c086e73,// 50 PAY  47 

    0x89781bbe,// 51 PAY  48 

    0xcb19a761,// 52 PAY  49 

    0x9a116659,// 53 PAY  50 

    0xd4b08352,// 54 PAY  51 

    0xa59c21a1,// 55 PAY  52 

    0xe8e64ea3,// 56 PAY  53 

    0xde5cf81f,// 57 PAY  54 

    0x08e3ecd9,// 58 PAY  55 

    0x9a2b892b,// 59 PAY  56 

    0xf19a21a7,// 60 PAY  57 

    0xb8595511,// 61 PAY  58 

    0x5a5dc19d,// 62 PAY  59 

    0xbdc44abe,// 63 PAY  60 

    0x641b4898,// 64 PAY  61 

    0x112f10b0,// 65 PAY  62 

    0x101577ab,// 66 PAY  63 

    0xa8044c01,// 67 PAY  64 

    0xb3a9cca7,// 68 PAY  65 

    0x197c4ecc,// 69 PAY  66 

    0x672b16dd,// 70 PAY  67 

    0x72f6c366,// 71 PAY  68 

    0xdba2cea0,// 72 PAY  69 

    0x8746c0d5,// 73 PAY  70 

    0x097d594a,// 74 PAY  71 

    0xe5ccf164,// 75 PAY  72 

    0xbaa585fc,// 76 PAY  73 

    0x693ffa98,// 77 PAY  74 

    0x741b8eda,// 78 PAY  75 

    0xdb4ebe56,// 79 PAY  76 

    0xa92c2db9,// 80 PAY  77 

    0x622ab161,// 81 PAY  78 

    0x72726479,// 82 PAY  79 

    0x856e6d23,// 83 PAY  80 

    0xc2529e65,// 84 PAY  81 

    0x095323e0,// 85 PAY  82 

    0xdad87f61,// 86 PAY  83 

    0xf3d55721,// 87 PAY  84 

    0xe9b0ffc9,// 88 PAY  85 

    0xb905677a,// 89 PAY  86 

    0x8ccca92c,// 90 PAY  87 

    0x6a75e83a,// 91 PAY  88 

    0x465b4491,// 92 PAY  89 

    0x5dc9ce00,// 93 PAY  90 

    0x41efb524,// 94 PAY  91 

    0x071ca93b,// 95 PAY  92 

    0xd23028a9,// 96 PAY  93 

    0x6c36476f,// 97 PAY  94 

    0xd5d0ec20,// 98 PAY  95 

    0xee49bfa4,// 99 PAY  96 

    0x7606d303,// 100 PAY  97 

    0xad9d1d82,// 101 PAY  98 

    0x305815d9,// 102 PAY  99 

    0x812ae212,// 103 PAY 100 

    0x09e81b15,// 104 PAY 101 

    0x7f400eee,// 105 PAY 102 

    0xc2b59303,// 106 PAY 103 

    0xf661414a,// 107 PAY 104 

    0xc2809e30,// 108 PAY 105 

    0x90c9d778,// 109 PAY 106 

    0xe4671329,// 110 PAY 107 

    0x7f1e915d,// 111 PAY 108 

    0x796cb881,// 112 PAY 109 

    0x1bd58991,// 113 PAY 110 

    0xb9113c12,// 114 PAY 111 

    0x661c2bab,// 115 PAY 112 

    0xafb8dff1,// 116 PAY 113 

    0x332895dc,// 117 PAY 114 

    0x9726416f,// 118 PAY 115 

    0xc34caf98,// 119 PAY 116 

    0x30fc2457,// 120 PAY 117 

    0x73f5b9f6,// 121 PAY 118 

    0x80f880f9,// 122 PAY 119 

    0x25e046a2,// 123 PAY 120 

    0xf5ccfaa0,// 124 PAY 121 

    0x424b7dd1,// 125 PAY 122 

    0x3c722eb3,// 126 PAY 123 

    0x7079862e,// 127 PAY 124 

    0xa01628eb,// 128 PAY 125 

    0x4a8c7623,// 129 PAY 126 

    0xe6bc295e,// 130 PAY 127 

    0x27ecd949,// 131 PAY 128 

    0x4b38f536,// 132 PAY 129 

    0x7d73541c,// 133 PAY 130 

    0x2edfb427,// 134 PAY 131 

    0x2e70c987,// 135 PAY 132 

    0x595251f8,// 136 PAY 133 

    0xb17a80f4,// 137 PAY 134 

    0xe993f175,// 138 PAY 135 

    0xf7aff672,// 139 PAY 136 

    0xc7789db4,// 140 PAY 137 

    0x517a8b03,// 141 PAY 138 

    0x2fc4aacd,// 142 PAY 139 

    0x443208c9,// 143 PAY 140 

    0x0f87e496,// 144 PAY 141 

    0xf449908a,// 145 PAY 142 

    0xce884495,// 146 PAY 143 

    0x366face1,// 147 PAY 144 

    0x6d579359,// 148 PAY 145 

    0x6542bf5d,// 149 PAY 146 

    0x1a0419a0,// 150 PAY 147 

    0x207a3c75,// 151 PAY 148 

    0xb6c56c4f,// 152 PAY 149 

    0x3570ddec,// 153 PAY 150 

    0x9f2c537a,// 154 PAY 151 

    0x689aa935,// 155 PAY 152 

    0xa8904cd9,// 156 PAY 153 

    0x76a5a665,// 157 PAY 154 

    0x2ee98b7c,// 158 PAY 155 

    0x5bd74f0f,// 159 PAY 156 

    0x712a3905,// 160 PAY 157 

    0xb79cc5f7,// 161 PAY 158 

    0xbbe2c5d3,// 162 PAY 159 

    0xb0b65a85,// 163 PAY 160 

    0x4b18c715,// 164 PAY 161 

    0x15cfe6d5,// 165 PAY 162 

    0x4fab5863,// 166 PAY 163 

    0x6978f16b,// 167 PAY 164 

    0x9c9077d6,// 168 PAY 165 

    0x5ff414ee,// 169 PAY 166 

    0xa08bf3f9,// 170 PAY 167 

    0x4f0beb38,// 171 PAY 168 

    0x978763dd,// 172 PAY 169 

    0x3c3524c4,// 173 PAY 170 

    0xf6b2fcad,// 174 PAY 171 

    0xaf29e387,// 175 PAY 172 

    0xdec87324,// 176 PAY 173 

    0xdabc268d,// 177 PAY 174 

    0x15bb605f,// 178 PAY 175 

    0x1b881778,// 179 PAY 176 

    0x17f0d0cb,// 180 PAY 177 

    0xbdc1ae8b,// 181 PAY 178 

    0xb19c7c32,// 182 PAY 179 

    0x7b71f1d4,// 183 PAY 180 

    0x29842eed,// 184 PAY 181 

    0x65811a47,// 185 PAY 182 

    0x4ee4fabc,// 186 PAY 183 

    0xadde1832,// 187 PAY 184 

    0x20829980,// 188 PAY 185 

    0x999c63fc,// 189 PAY 186 

    0xdc99bc02,// 190 PAY 187 

    0xfc5757aa,// 191 PAY 188 

    0x0b8bdf6e,// 192 PAY 189 

    0x15d8d2f9,// 193 PAY 190 

    0xe15a1479,// 194 PAY 191 

    0xe7aa90ae,// 195 PAY 192 

    0x08ee9461,// 196 PAY 193 

    0x98873b21,// 197 PAY 194 

    0x48a03d94,// 198 PAY 195 

    0x598310a9,// 199 PAY 196 

    0x4ef7b002,// 200 PAY 197 

    0x3d88ff20,// 201 PAY 198 

    0x342ff87f,// 202 PAY 199 

    0x4a1a247d,// 203 PAY 200 

    0xf0b09c86,// 204 PAY 201 

    0x92ce69bf,// 205 PAY 202 

    0x0a40a108,// 206 PAY 203 

    0x3d0b2343,// 207 PAY 204 

    0xd59d0621,// 208 PAY 205 

    0xfb4f6faf,// 209 PAY 206 

    0x4e10f570,// 210 PAY 207 

    0x91110629,// 211 PAY 208 

    0xf2543b8c,// 212 PAY 209 

    0x7f8b8209,// 213 PAY 210 

    0x62b799ec,// 214 PAY 211 

    0xd7e45172,// 215 PAY 212 

    0xd00619ac,// 216 PAY 213 

    0xfad4c023,// 217 PAY 214 

    0xa5c30c0a,// 218 PAY 215 

    0x4f40b384,// 219 PAY 216 

    0x106c3a5e,// 220 PAY 217 

    0xc74e8459,// 221 PAY 218 

    0x8d52d170,// 222 PAY 219 

    0x85b96ad9,// 223 PAY 220 

    0xd9c3602f,// 224 PAY 221 

    0x30bb0afd,// 225 PAY 222 

    0xa75a10ca,// 226 PAY 223 

    0x7048882e,// 227 PAY 224 

    0xa1e3af75,// 228 PAY 225 

    0xf43d5ec1,// 229 PAY 226 

    0x855e81f9,// 230 PAY 227 

    0x4cdf224b,// 231 PAY 228 

    0x9c517b3f,// 232 PAY 229 

    0xd896a0f8,// 233 PAY 230 

    0x59ea09a5,// 234 PAY 231 

    0xf6f0d70a,// 235 PAY 232 

    0x86c09788,// 236 PAY 233 

    0x1d362962,// 237 PAY 234 

    0xbfe18611,// 238 PAY 235 

    0xf5d9e029,// 239 PAY 236 

    0x6378459f,// 240 PAY 237 

    0x5c9a913f,// 241 PAY 238 

    0x61736d81,// 242 PAY 239 

    0x2b580906,// 243 PAY 240 

    0x2425eed7,// 244 PAY 241 

    0x89872ab6,// 245 PAY 242 

    0x58137ffd,// 246 PAY 243 

    0xb1d003a7,// 247 PAY 244 

    0xb380a91c,// 248 PAY 245 

    0xb20220bf,// 249 PAY 246 

    0x39b03a64,// 250 PAY 247 

    0xaa589a88,// 251 PAY 248 

    0xc6a7e2f4,// 252 PAY 249 

    0x4729679d,// 253 PAY 250 

    0xb8bf4328,// 254 PAY 251 

    0x6cc97e95,// 255 PAY 252 

    0x9271119c,// 256 PAY 253 

    0x95eaa203,// 257 PAY 254 

    0x5c2e29a6,// 258 PAY 255 

    0x0a51c195,// 259 PAY 256 

    0x74eb3c4e,// 260 PAY 257 

    0x208adde1,// 261 PAY 258 

    0xba63b0db,// 262 PAY 259 

    0x2859b924,// 263 PAY 260 

    0xba329231,// 264 PAY 261 

    0x35d348de,// 265 PAY 262 

    0x564db057,// 266 PAY 263 

    0xa2590888,// 267 PAY 264 

    0xc9e4d201,// 268 PAY 265 

    0xa88a219c,// 269 PAY 266 

    0xb5094516,// 270 PAY 267 

    0x573211fa,// 271 PAY 268 

    0xc70d6640,// 272 PAY 269 

    0xc108424c,// 273 PAY 270 

    0x66676195,// 274 PAY 271 

    0xb2037bbb,// 275 PAY 272 

    0xc86f96db,// 276 PAY 273 

    0xbfca5b71,// 277 PAY 274 

    0x56ccc9f3,// 278 PAY 275 

    0xfd588c8a,// 279 PAY 276 

    0x9cb76b6b,// 280 PAY 277 

    0xe30c32d7,// 281 PAY 278 

    0x2bb40f05,// 282 PAY 279 

    0x7e75e330,// 283 PAY 280 

    0x0a721893,// 284 PAY 281 

    0x51ba6eaa,// 285 PAY 282 

    0x3cd1af8d,// 286 PAY 283 

    0xfa2fa0d0,// 287 PAY 284 

    0x4a9abc7e,// 288 PAY 285 

    0x37dce466,// 289 PAY 286 

    0x9d596c67,// 290 PAY 287 

    0xb314d903,// 291 PAY 288 

    0x6999765f,// 292 PAY 289 

    0xb775fe36,// 293 PAY 290 

    0xcd536020,// 294 PAY 291 

    0x67ff5b9c,// 295 PAY 292 

    0xa6bbda34,// 296 PAY 293 

    0xc1e14885,// 297 PAY 294 

    0x19f29324,// 298 PAY 295 

    0xce9d55d7,// 299 PAY 296 

    0x04631560,// 300 PAY 297 

    0xac8c6a24,// 301 PAY 298 

    0x6241bc2a,// 302 PAY 299 

    0x0be06100,// 303 PAY 300 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0359e82c // 304 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt11_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 431 words. 

/// BDA size     is 1719 (0x6b7) 

/// BDA id       is 0x906 

    0x06b70906,// 3 BDA   1 

/// PAY Generic Data size   : 1719 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x16c4ca72,// 4 PAY   1 

    0x1d5638cc,// 5 PAY   2 

    0x4ec9462d,// 6 PAY   3 

    0xbc41e252,// 7 PAY   4 

    0x3f45b515,// 8 PAY   5 

    0x377f1222,// 9 PAY   6 

    0x576a6a2c,// 10 PAY   7 

    0xfdfb40de,// 11 PAY   8 

    0x1567790c,// 12 PAY   9 

    0x75920b28,// 13 PAY  10 

    0xdbde331f,// 14 PAY  11 

    0xdde78f6c,// 15 PAY  12 

    0x9f39aa69,// 16 PAY  13 

    0x9adc7377,// 17 PAY  14 

    0x0c767033,// 18 PAY  15 

    0x57f22069,// 19 PAY  16 

    0x10deb861,// 20 PAY  17 

    0x51f8fa8c,// 21 PAY  18 

    0x377da9e0,// 22 PAY  19 

    0x8b15948f,// 23 PAY  20 

    0xc4a75584,// 24 PAY  21 

    0xce301c55,// 25 PAY  22 

    0x9e4e4343,// 26 PAY  23 

    0x05e1efdc,// 27 PAY  24 

    0x0bb3afde,// 28 PAY  25 

    0xd7cb01f1,// 29 PAY  26 

    0x116e963c,// 30 PAY  27 

    0xda0ef758,// 31 PAY  28 

    0xe163e7e7,// 32 PAY  29 

    0x8135dd80,// 33 PAY  30 

    0xab219028,// 34 PAY  31 

    0xb46226d6,// 35 PAY  32 

    0x061dce89,// 36 PAY  33 

    0xa675ebb7,// 37 PAY  34 

    0x2fd4e882,// 38 PAY  35 

    0xeaecf66d,// 39 PAY  36 

    0x936dc6dc,// 40 PAY  37 

    0x7c83b482,// 41 PAY  38 

    0x92dca69f,// 42 PAY  39 

    0x9d19902d,// 43 PAY  40 

    0xf3740af5,// 44 PAY  41 

    0x3e2d3c4b,// 45 PAY  42 

    0xd0c0edd7,// 46 PAY  43 

    0x07a290d5,// 47 PAY  44 

    0xbebaae21,// 48 PAY  45 

    0xb40250c8,// 49 PAY  46 

    0xf1c63574,// 50 PAY  47 

    0x40faf6e7,// 51 PAY  48 

    0xe66d6e98,// 52 PAY  49 

    0x51818ab5,// 53 PAY  50 

    0xe98ee1ea,// 54 PAY  51 

    0x2e2505be,// 55 PAY  52 

    0x7cf242b3,// 56 PAY  53 

    0x06503ae0,// 57 PAY  54 

    0x64cfd232,// 58 PAY  55 

    0x48517bbb,// 59 PAY  56 

    0x71f92ea2,// 60 PAY  57 

    0x1afa333b,// 61 PAY  58 

    0xae1b048f,// 62 PAY  59 

    0xbb2e05c4,// 63 PAY  60 

    0x2db80574,// 64 PAY  61 

    0xfae3f99a,// 65 PAY  62 

    0x892364df,// 66 PAY  63 

    0x8473bf09,// 67 PAY  64 

    0x069a54aa,// 68 PAY  65 

    0x474f20ef,// 69 PAY  66 

    0x967f29c2,// 70 PAY  67 

    0xe38ab447,// 71 PAY  68 

    0x29216060,// 72 PAY  69 

    0x6780b14c,// 73 PAY  70 

    0x2d2d366d,// 74 PAY  71 

    0xb53570f9,// 75 PAY  72 

    0x900f37aa,// 76 PAY  73 

    0x43da70aa,// 77 PAY  74 

    0x2df95bdc,// 78 PAY  75 

    0xa2261e88,// 79 PAY  76 

    0x54bfa20c,// 80 PAY  77 

    0x15285bbd,// 81 PAY  78 

    0xd7e9767f,// 82 PAY  79 

    0x7e2d4031,// 83 PAY  80 

    0x61ce1d65,// 84 PAY  81 

    0x2c2773a9,// 85 PAY  82 

    0x7385065e,// 86 PAY  83 

    0xfc323a68,// 87 PAY  84 

    0xa19b5ef1,// 88 PAY  85 

    0x96c9dea1,// 89 PAY  86 

    0xa4791d8c,// 90 PAY  87 

    0xadd1535b,// 91 PAY  88 

    0xef0cf2f8,// 92 PAY  89 

    0x1722b98e,// 93 PAY  90 

    0xafbb5e33,// 94 PAY  91 

    0x84a9b599,// 95 PAY  92 

    0x9e613fc6,// 96 PAY  93 

    0xc9106f1a,// 97 PAY  94 

    0x4b2a21de,// 98 PAY  95 

    0xe880e04f,// 99 PAY  96 

    0x3182082b,// 100 PAY  97 

    0xb1d56124,// 101 PAY  98 

    0xfbd1d3dd,// 102 PAY  99 

    0x71eff672,// 103 PAY 100 

    0x830bca62,// 104 PAY 101 

    0x6fc626c3,// 105 PAY 102 

    0xffb8a0e8,// 106 PAY 103 

    0x18712a15,// 107 PAY 104 

    0x493ef91c,// 108 PAY 105 

    0x89df42ed,// 109 PAY 106 

    0x27b14441,// 110 PAY 107 

    0x021b028b,// 111 PAY 108 

    0x679b15fe,// 112 PAY 109 

    0x7e609a5a,// 113 PAY 110 

    0x4f1a680f,// 114 PAY 111 

    0x15e8faa2,// 115 PAY 112 

    0xb0d97702,// 116 PAY 113 

    0xe1dc9d08,// 117 PAY 114 

    0x3c9ed106,// 118 PAY 115 

    0x29960426,// 119 PAY 116 

    0x01733e5e,// 120 PAY 117 

    0x23e5249b,// 121 PAY 118 

    0x391eade7,// 122 PAY 119 

    0xa289ec20,// 123 PAY 120 

    0x3d52e060,// 124 PAY 121 

    0xe60e6545,// 125 PAY 122 

    0x834db967,// 126 PAY 123 

    0xdc811068,// 127 PAY 124 

    0x75a14112,// 128 PAY 125 

    0x11303d84,// 129 PAY 126 

    0x87610325,// 130 PAY 127 

    0x030a8d32,// 131 PAY 128 

    0x8852c39f,// 132 PAY 129 

    0x8698e6ba,// 133 PAY 130 

    0x262b4f8a,// 134 PAY 131 

    0xcd9c22e5,// 135 PAY 132 

    0x25e0be13,// 136 PAY 133 

    0x9c13ded9,// 137 PAY 134 

    0xa2bc8f30,// 138 PAY 135 

    0xd70d8625,// 139 PAY 136 

    0x1fa3e70b,// 140 PAY 137 

    0x1a67f926,// 141 PAY 138 

    0x310c979c,// 142 PAY 139 

    0x09f9b299,// 143 PAY 140 

    0xe4ce7f6d,// 144 PAY 141 

    0xd4e57816,// 145 PAY 142 

    0xa78c34e4,// 146 PAY 143 

    0x48d91e78,// 147 PAY 144 

    0xec0992cd,// 148 PAY 145 

    0xc170cb31,// 149 PAY 146 

    0xd710abca,// 150 PAY 147 

    0xae9450e5,// 151 PAY 148 

    0x79c995ac,// 152 PAY 149 

    0x45edf4cf,// 153 PAY 150 

    0xc8298f0d,// 154 PAY 151 

    0xd078f952,// 155 PAY 152 

    0xf66b2f97,// 156 PAY 153 

    0x971657ed,// 157 PAY 154 

    0xe1bee98d,// 158 PAY 155 

    0xddeb3839,// 159 PAY 156 

    0xe909424d,// 160 PAY 157 

    0xa6937576,// 161 PAY 158 

    0x1a742ce2,// 162 PAY 159 

    0x1b63584d,// 163 PAY 160 

    0x07d6ecc3,// 164 PAY 161 

    0x1ad0e828,// 165 PAY 162 

    0xc046a839,// 166 PAY 163 

    0xad5c63ad,// 167 PAY 164 

    0x6e0498f4,// 168 PAY 165 

    0xdc68db06,// 169 PAY 166 

    0x4e6f81f7,// 170 PAY 167 

    0x2289d39a,// 171 PAY 168 

    0x62e59c03,// 172 PAY 169 

    0x21efdf46,// 173 PAY 170 

    0x666daa99,// 174 PAY 171 

    0x36c74db0,// 175 PAY 172 

    0xd8be80a8,// 176 PAY 173 

    0xcb4c1c9e,// 177 PAY 174 

    0xb5d141eb,// 178 PAY 175 

    0x56dea846,// 179 PAY 176 

    0x1152c64e,// 180 PAY 177 

    0xf84ce48c,// 181 PAY 178 

    0x5a7abf22,// 182 PAY 179 

    0xbd883130,// 183 PAY 180 

    0x8a6d4cac,// 184 PAY 181 

    0x5c75bce0,// 185 PAY 182 

    0xb39da914,// 186 PAY 183 

    0x53f9502a,// 187 PAY 184 

    0x72fc0cec,// 188 PAY 185 

    0xf0b7f4e0,// 189 PAY 186 

    0x57bda404,// 190 PAY 187 

    0x49caf248,// 191 PAY 188 

    0x080c76f6,// 192 PAY 189 

    0xd704771d,// 193 PAY 190 

    0x18e97603,// 194 PAY 191 

    0xbdbc14e9,// 195 PAY 192 

    0x2e1d33bd,// 196 PAY 193 

    0xd99d5f3a,// 197 PAY 194 

    0x84466ec7,// 198 PAY 195 

    0x739a4cf1,// 199 PAY 196 

    0x9acc5fad,// 200 PAY 197 

    0x76071ddf,// 201 PAY 198 

    0x0ed2c5a7,// 202 PAY 199 

    0xb582882f,// 203 PAY 200 

    0xc93d8bfd,// 204 PAY 201 

    0x35e4048b,// 205 PAY 202 

    0x99f156fe,// 206 PAY 203 

    0x59ed701f,// 207 PAY 204 

    0xacd5d308,// 208 PAY 205 

    0xc352d5d0,// 209 PAY 206 

    0xec7b28cb,// 210 PAY 207 

    0xeb699384,// 211 PAY 208 

    0x87f08884,// 212 PAY 209 

    0xb1453dc1,// 213 PAY 210 

    0x25b49613,// 214 PAY 211 

    0x381b74b6,// 215 PAY 212 

    0xd836b821,// 216 PAY 213 

    0x0636ebe5,// 217 PAY 214 

    0x18fd95b7,// 218 PAY 215 

    0xc3688ac4,// 219 PAY 216 

    0x73077f55,// 220 PAY 217 

    0xc8224672,// 221 PAY 218 

    0xace3adb7,// 222 PAY 219 

    0x67176420,// 223 PAY 220 

    0x12780302,// 224 PAY 221 

    0xa96c2719,// 225 PAY 222 

    0x7dcb06cc,// 226 PAY 223 

    0x71039b8d,// 227 PAY 224 

    0x0c2edbb1,// 228 PAY 225 

    0x3955df1b,// 229 PAY 226 

    0x16f4e577,// 230 PAY 227 

    0xafe6bd41,// 231 PAY 228 

    0x60f12995,// 232 PAY 229 

    0x7c645fd9,// 233 PAY 230 

    0x89e5e43b,// 234 PAY 231 

    0x4d9dc067,// 235 PAY 232 

    0x2cb70083,// 236 PAY 233 

    0xd814fd3e,// 237 PAY 234 

    0x0ec62343,// 238 PAY 235 

    0xc0c8eb01,// 239 PAY 236 

    0x9f367c14,// 240 PAY 237 

    0x06b4fa27,// 241 PAY 238 

    0xfb70d926,// 242 PAY 239 

    0x6c6d1e4b,// 243 PAY 240 

    0x20c7dc03,// 244 PAY 241 

    0xb76282d2,// 245 PAY 242 

    0xa6e386de,// 246 PAY 243 

    0x51fe008a,// 247 PAY 244 

    0xb658313b,// 248 PAY 245 

    0xf032c6c4,// 249 PAY 246 

    0x51f98c7b,// 250 PAY 247 

    0x5b38b541,// 251 PAY 248 

    0xb0e0e6a4,// 252 PAY 249 

    0x16747b4b,// 253 PAY 250 

    0x581098c3,// 254 PAY 251 

    0x091a7dbb,// 255 PAY 252 

    0x21206286,// 256 PAY 253 

    0x2bfcb130,// 257 PAY 254 

    0x90f6b16b,// 258 PAY 255 

    0x7d2532ad,// 259 PAY 256 

    0x952fb801,// 260 PAY 257 

    0xf05f27a3,// 261 PAY 258 

    0xe203a3cc,// 262 PAY 259 

    0x3af05450,// 263 PAY 260 

    0x3d3497d3,// 264 PAY 261 

    0x88cf9143,// 265 PAY 262 

    0x7f59aa97,// 266 PAY 263 

    0xe3caf8a7,// 267 PAY 264 

    0x393f66b2,// 268 PAY 265 

    0xddefea42,// 269 PAY 266 

    0x8aae8f92,// 270 PAY 267 

    0x6fca552f,// 271 PAY 268 

    0x323f6395,// 272 PAY 269 

    0x50ad7c67,// 273 PAY 270 

    0xc046dabd,// 274 PAY 271 

    0xdd42371c,// 275 PAY 272 

    0x0a5fcced,// 276 PAY 273 

    0xc26e9cc2,// 277 PAY 274 

    0xcfd80174,// 278 PAY 275 

    0xfafe318a,// 279 PAY 276 

    0x211edc03,// 280 PAY 277 

    0x38175470,// 281 PAY 278 

    0x8f3ae7c9,// 282 PAY 279 

    0xdc5b1787,// 283 PAY 280 

    0x6f16548a,// 284 PAY 281 

    0xe89f9cd3,// 285 PAY 282 

    0x49908891,// 286 PAY 283 

    0x03fb2584,// 287 PAY 284 

    0x20d5b349,// 288 PAY 285 

    0xfbde5fab,// 289 PAY 286 

    0x281a7968,// 290 PAY 287 

    0x8945cff6,// 291 PAY 288 

    0x242b1d85,// 292 PAY 289 

    0x669971a4,// 293 PAY 290 

    0x2a1eeb1b,// 294 PAY 291 

    0x1843cf3d,// 295 PAY 292 

    0x05e0adbb,// 296 PAY 293 

    0x6e441854,// 297 PAY 294 

    0x8285792d,// 298 PAY 295 

    0xfe922c31,// 299 PAY 296 

    0x86ebc963,// 300 PAY 297 

    0x04cf465f,// 301 PAY 298 

    0xce9f5a02,// 302 PAY 299 

    0xe7fd9226,// 303 PAY 300 

    0x0ad9ee69,// 304 PAY 301 

    0x9f598e83,// 305 PAY 302 

    0x9c4e66cb,// 306 PAY 303 

    0xef26feed,// 307 PAY 304 

    0xaf32caad,// 308 PAY 305 

    0x0241030f,// 309 PAY 306 

    0x9e1d59e8,// 310 PAY 307 

    0xf20c7c3b,// 311 PAY 308 

    0xa806c465,// 312 PAY 309 

    0x1a388640,// 313 PAY 310 

    0x495a3f0b,// 314 PAY 311 

    0xa1b2946e,// 315 PAY 312 

    0x6e84db7b,// 316 PAY 313 

    0xaed7c922,// 317 PAY 314 

    0x47cf11e9,// 318 PAY 315 

    0x23493f5f,// 319 PAY 316 

    0x69f419c8,// 320 PAY 317 

    0x2749a187,// 321 PAY 318 

    0x5a4347b7,// 322 PAY 319 

    0xddd65d6a,// 323 PAY 320 

    0x9210d460,// 324 PAY 321 

    0xdb11088f,// 325 PAY 322 

    0x831386f0,// 326 PAY 323 

    0x183f69e6,// 327 PAY 324 

    0x881ed6e4,// 328 PAY 325 

    0x6b653f56,// 329 PAY 326 

    0x1a6f8111,// 330 PAY 327 

    0x125dd0df,// 331 PAY 328 

    0xe674ea8a,// 332 PAY 329 

    0x9d5ed9d1,// 333 PAY 330 

    0xf270bf60,// 334 PAY 331 

    0xf86c1f88,// 335 PAY 332 

    0x432d994d,// 336 PAY 333 

    0xcd245632,// 337 PAY 334 

    0xae973ffb,// 338 PAY 335 

    0xae3976ab,// 339 PAY 336 

    0xd98802ba,// 340 PAY 337 

    0xbb966db4,// 341 PAY 338 

    0xf92e7a1a,// 342 PAY 339 

    0x9f35701c,// 343 PAY 340 

    0xc3b81626,// 344 PAY 341 

    0x6de4c956,// 345 PAY 342 

    0xbd73a86a,// 346 PAY 343 

    0x76eeda10,// 347 PAY 344 

    0x409a76e6,// 348 PAY 345 

    0x2370c406,// 349 PAY 346 

    0x2cf06569,// 350 PAY 347 

    0xd676e733,// 351 PAY 348 

    0x2d09d7db,// 352 PAY 349 

    0x58c6219e,// 353 PAY 350 

    0xce48921e,// 354 PAY 351 

    0x388c8ed7,// 355 PAY 352 

    0x6937189f,// 356 PAY 353 

    0xba92e108,// 357 PAY 354 

    0xb70ebf58,// 358 PAY 355 

    0xe262d00a,// 359 PAY 356 

    0x16ae0651,// 360 PAY 357 

    0x2bb4ab74,// 361 PAY 358 

    0xa45d3698,// 362 PAY 359 

    0x0afd1aa7,// 363 PAY 360 

    0x36c9b4a9,// 364 PAY 361 

    0xc15fc2a1,// 365 PAY 362 

    0xea161c2e,// 366 PAY 363 

    0x96d2ab0b,// 367 PAY 364 

    0x42d3e3e0,// 368 PAY 365 

    0xdf1c891a,// 369 PAY 366 

    0x50666fca,// 370 PAY 367 

    0xb69eb97e,// 371 PAY 368 

    0xc427b202,// 372 PAY 369 

    0xd55436a5,// 373 PAY 370 

    0xc044a2e6,// 374 PAY 371 

    0x6bc0e24f,// 375 PAY 372 

    0x05ae00a3,// 376 PAY 373 

    0xd069cd2a,// 377 PAY 374 

    0x9c65f476,// 378 PAY 375 

    0xfd03cb96,// 379 PAY 376 

    0x898c8fd6,// 380 PAY 377 

    0xe5094fe9,// 381 PAY 378 

    0x0c25ba47,// 382 PAY 379 

    0xcb2c94b5,// 383 PAY 380 

    0x7de7927c,// 384 PAY 381 

    0xa9aa21a2,// 385 PAY 382 

    0x6014dbac,// 386 PAY 383 

    0x31351246,// 387 PAY 384 

    0x304b649a,// 388 PAY 385 

    0x472e34b5,// 389 PAY 386 

    0x2ba30fc3,// 390 PAY 387 

    0x28452a15,// 391 PAY 388 

    0x9e10b59c,// 392 PAY 389 

    0x182e6a0a,// 393 PAY 390 

    0x2e2b4db8,// 394 PAY 391 

    0x0c2ff5d8,// 395 PAY 392 

    0x3eb23f8e,// 396 PAY 393 

    0x334f5537,// 397 PAY 394 

    0x6c9f9471,// 398 PAY 395 

    0xe6046115,// 399 PAY 396 

    0xf0c0aba8,// 400 PAY 397 

    0xfbfc4bbd,// 401 PAY 398 

    0xbf031c15,// 402 PAY 399 

    0x7fa1f758,// 403 PAY 400 

    0x3e8bf106,// 404 PAY 401 

    0x706fc565,// 405 PAY 402 

    0xb245b6b0,// 406 PAY 403 

    0xbaced57e,// 407 PAY 404 

    0xb03e5da3,// 408 PAY 405 

    0xf27233f1,// 409 PAY 406 

    0xd1688303,// 410 PAY 407 

    0xd5a4e6e5,// 411 PAY 408 

    0x917fb94a,// 412 PAY 409 

    0xdf4e978b,// 413 PAY 410 

    0x6b32b2c5,// 414 PAY 411 

    0xb0970987,// 415 PAY 412 

    0xeaa832c5,// 416 PAY 413 

    0x7f0c69e3,// 417 PAY 414 

    0x9c320b42,// 418 PAY 415 

    0x60d5f5b2,// 419 PAY 416 

    0x0b67f975,// 420 PAY 417 

    0xe267efaf,// 421 PAY 418 

    0x19c44204,// 422 PAY 419 

    0xfebaa6ef,// 423 PAY 420 

    0x9defc1cb,// 424 PAY 421 

    0x52550f1a,// 425 PAY 422 

    0x92ca8566,// 426 PAY 423 

    0xaef3b83a,// 427 PAY 424 

    0xa7ae5e07,// 428 PAY 425 

    0x0f8428d6,// 429 PAY 426 

    0x8c4d2304,// 430 PAY 427 

    0x125529a0,// 431 PAY 428 

    0x2c228b74,// 432 PAY 429 

    0xa640a000,// 433 PAY 430 

/// STA is 1 words. 

/// STA num_pkts       : 10 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7c 

    0x03607c0a // 434 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt12_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 486 words. 

/// BDA size     is 1937 (0x791) 

/// BDA id       is 0x79ee 

    0x079179ee,// 3 BDA   1 

/// PAY Generic Data size   : 1937 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x29ad4d16,// 4 PAY   1 

    0xdec9ad19,// 5 PAY   2 

    0xe6a43e37,// 6 PAY   3 

    0xb4a14e11,// 7 PAY   4 

    0x95ad3741,// 8 PAY   5 

    0x15189841,// 9 PAY   6 

    0x4a043e15,// 10 PAY   7 

    0x40b47261,// 11 PAY   8 

    0x9a241410,// 12 PAY   9 

    0x91598154,// 13 PAY  10 

    0xc2f0fb97,// 14 PAY  11 

    0xdbc6b6ee,// 15 PAY  12 

    0xf314e5c6,// 16 PAY  13 

    0x7fdea901,// 17 PAY  14 

    0x849bcc52,// 18 PAY  15 

    0x277cc372,// 19 PAY  16 

    0x88639561,// 20 PAY  17 

    0x138f9796,// 21 PAY  18 

    0x650ba079,// 22 PAY  19 

    0xefe11bd4,// 23 PAY  20 

    0xdc614875,// 24 PAY  21 

    0xd6b56d32,// 25 PAY  22 

    0x1d000eb3,// 26 PAY  23 

    0xaf92b4d6,// 27 PAY  24 

    0x6bc5497a,// 28 PAY  25 

    0xb719666d,// 29 PAY  26 

    0x19ef9618,// 30 PAY  27 

    0xec8aea68,// 31 PAY  28 

    0x70f3c731,// 32 PAY  29 

    0x33984ab4,// 33 PAY  30 

    0x14154dc8,// 34 PAY  31 

    0xa0b9f4ff,// 35 PAY  32 

    0xca68d849,// 36 PAY  33 

    0xaa31dd0c,// 37 PAY  34 

    0x4dfa42d7,// 38 PAY  35 

    0x09761b22,// 39 PAY  36 

    0x236c828f,// 40 PAY  37 

    0x1526a848,// 41 PAY  38 

    0xa22d9e15,// 42 PAY  39 

    0x36ece59f,// 43 PAY  40 

    0x19d8b86e,// 44 PAY  41 

    0xc656bbcc,// 45 PAY  42 

    0x1e9aba43,// 46 PAY  43 

    0x20ea8645,// 47 PAY  44 

    0x6402dbd0,// 48 PAY  45 

    0x741ea35e,// 49 PAY  46 

    0x79be86da,// 50 PAY  47 

    0x6aec87a7,// 51 PAY  48 

    0x94c56ad0,// 52 PAY  49 

    0x75e4b2ac,// 53 PAY  50 

    0x27cc49f8,// 54 PAY  51 

    0x7ab86750,// 55 PAY  52 

    0xc37d063d,// 56 PAY  53 

    0x54f3355a,// 57 PAY  54 

    0x81f5c57d,// 58 PAY  55 

    0xf737a763,// 59 PAY  56 

    0x6f429854,// 60 PAY  57 

    0x9480c4ee,// 61 PAY  58 

    0x2539bd4c,// 62 PAY  59 

    0x6b49d37c,// 63 PAY  60 

    0x01b34ad6,// 64 PAY  61 

    0x50e0495d,// 65 PAY  62 

    0x61bc1bb1,// 66 PAY  63 

    0xb14c0e1c,// 67 PAY  64 

    0x669b1dbc,// 68 PAY  65 

    0xc804c4cb,// 69 PAY  66 

    0xc3d10b07,// 70 PAY  67 

    0x9f2f6a0f,// 71 PAY  68 

    0x65d2457e,// 72 PAY  69 

    0x5141d781,// 73 PAY  70 

    0xf8b31267,// 74 PAY  71 

    0x1f0cc512,// 75 PAY  72 

    0x6366b112,// 76 PAY  73 

    0xacd4bf6f,// 77 PAY  74 

    0x63945765,// 78 PAY  75 

    0x7d769c64,// 79 PAY  76 

    0x5b6d9f47,// 80 PAY  77 

    0x50bdf654,// 81 PAY  78 

    0xecc2da01,// 82 PAY  79 

    0x1410db3b,// 83 PAY  80 

    0x94845df4,// 84 PAY  81 

    0xc7c82e44,// 85 PAY  82 

    0xc82d6312,// 86 PAY  83 

    0xbc367860,// 87 PAY  84 

    0xf97e9676,// 88 PAY  85 

    0xbf0ba480,// 89 PAY  86 

    0x9959aec6,// 90 PAY  87 

    0x81d4dbc7,// 91 PAY  88 

    0x35e61e31,// 92 PAY  89 

    0xccd584a5,// 93 PAY  90 

    0xc7b88e5a,// 94 PAY  91 

    0xa468eabc,// 95 PAY  92 

    0x2241c371,// 96 PAY  93 

    0x1d42d93d,// 97 PAY  94 

    0x88babd01,// 98 PAY  95 

    0xaf02c04d,// 99 PAY  96 

    0x5c96f279,// 100 PAY  97 

    0x93801ffb,// 101 PAY  98 

    0x707741c9,// 102 PAY  99 

    0x174fcaca,// 103 PAY 100 

    0x7e2fef27,// 104 PAY 101 

    0xcae18e65,// 105 PAY 102 

    0xd7310d0c,// 106 PAY 103 

    0x431b3c11,// 107 PAY 104 

    0x54fc0548,// 108 PAY 105 

    0x68df36d8,// 109 PAY 106 

    0x75aaa62a,// 110 PAY 107 

    0xd2aea238,// 111 PAY 108 

    0x6e5c111b,// 112 PAY 109 

    0x1e7b728a,// 113 PAY 110 

    0xf1055e4a,// 114 PAY 111 

    0xf454fa0f,// 115 PAY 112 

    0xde3c771c,// 116 PAY 113 

    0x67663e9d,// 117 PAY 114 

    0x2494046e,// 118 PAY 115 

    0x6068a1eb,// 119 PAY 116 

    0xbe1988f5,// 120 PAY 117 

    0xf109946a,// 121 PAY 118 

    0x8684ee85,// 122 PAY 119 

    0x255bd29e,// 123 PAY 120 

    0x5596b77f,// 124 PAY 121 

    0x0c8aa184,// 125 PAY 122 

    0x7acec6a3,// 126 PAY 123 

    0x2d6ece8b,// 127 PAY 124 

    0x8e35abd8,// 128 PAY 125 

    0x4e47a77f,// 129 PAY 126 

    0x4f1f5635,// 130 PAY 127 

    0x02eba323,// 131 PAY 128 

    0xf219ee99,// 132 PAY 129 

    0xc1166bb3,// 133 PAY 130 

    0x7c7efcec,// 134 PAY 131 

    0xc5d9acd0,// 135 PAY 132 

    0x721a1587,// 136 PAY 133 

    0x95235f1c,// 137 PAY 134 

    0x0e754f6a,// 138 PAY 135 

    0x7ac7f15c,// 139 PAY 136 

    0x1b6edf46,// 140 PAY 137 

    0x093ac08f,// 141 PAY 138 

    0xe685573c,// 142 PAY 139 

    0xaadf90fe,// 143 PAY 140 

    0xca4f4e61,// 144 PAY 141 

    0xe2f92cd9,// 145 PAY 142 

    0x968cc4d6,// 146 PAY 143 

    0x165c03e8,// 147 PAY 144 

    0x5e1c241b,// 148 PAY 145 

    0x128abe65,// 149 PAY 146 

    0xe7ba07ac,// 150 PAY 147 

    0x54791b4c,// 151 PAY 148 

    0xc718b896,// 152 PAY 149 

    0x8d2837e5,// 153 PAY 150 

    0xb2efcfcf,// 154 PAY 151 

    0xc92cbdac,// 155 PAY 152 

    0x60116439,// 156 PAY 153 

    0xeada5876,// 157 PAY 154 

    0xe383db4a,// 158 PAY 155 

    0x7134d267,// 159 PAY 156 

    0x8bf39504,// 160 PAY 157 

    0x04590152,// 161 PAY 158 

    0x2111af35,// 162 PAY 159 

    0x846fc7a5,// 163 PAY 160 

    0xbb7eb771,// 164 PAY 161 

    0xa7442f9b,// 165 PAY 162 

    0x8ba1abf4,// 166 PAY 163 

    0xd4730f32,// 167 PAY 164 

    0xec359a8e,// 168 PAY 165 

    0x741558c1,// 169 PAY 166 

    0x7d482140,// 170 PAY 167 

    0x75fc9c54,// 171 PAY 168 

    0x5209067c,// 172 PAY 169 

    0xd61f891d,// 173 PAY 170 

    0xf4061f4e,// 174 PAY 171 

    0xb023b4a4,// 175 PAY 172 

    0xd9cdd109,// 176 PAY 173 

    0x849e41e7,// 177 PAY 174 

    0x9290b5b3,// 178 PAY 175 

    0x8d1a1ac9,// 179 PAY 176 

    0x5cddc926,// 180 PAY 177 

    0x076e988d,// 181 PAY 178 

    0x26dbf318,// 182 PAY 179 

    0x1887eb25,// 183 PAY 180 

    0xd514177f,// 184 PAY 181 

    0x2c9d78d7,// 185 PAY 182 

    0x2a167582,// 186 PAY 183 

    0x0920fb7f,// 187 PAY 184 

    0x38fa8d59,// 188 PAY 185 

    0xbb5c8493,// 189 PAY 186 

    0xb5d46d5c,// 190 PAY 187 

    0x88af5e55,// 191 PAY 188 

    0x5aca8666,// 192 PAY 189 

    0x1ce86d33,// 193 PAY 190 

    0xd3271ba2,// 194 PAY 191 

    0x21e79df9,// 195 PAY 192 

    0xb8f25f64,// 196 PAY 193 

    0x40b4df1d,// 197 PAY 194 

    0xbab2fad5,// 198 PAY 195 

    0x67ec02f6,// 199 PAY 196 

    0xfb6dda31,// 200 PAY 197 

    0x291b6061,// 201 PAY 198 

    0xcf660fdb,// 202 PAY 199 

    0x70418c30,// 203 PAY 200 

    0x3b3b4cfa,// 204 PAY 201 

    0xc817ae50,// 205 PAY 202 

    0xc73ff68b,// 206 PAY 203 

    0xc1824ec9,// 207 PAY 204 

    0xd78aec57,// 208 PAY 205 

    0x5912fbdc,// 209 PAY 206 

    0xee1bf2e6,// 210 PAY 207 

    0xf452d10c,// 211 PAY 208 

    0x85914ff2,// 212 PAY 209 

    0xf46bb6ff,// 213 PAY 210 

    0xca3b02c8,// 214 PAY 211 

    0x179d3321,// 215 PAY 212 

    0x40bc7475,// 216 PAY 213 

    0x6939b2d6,// 217 PAY 214 

    0xfb346fc5,// 218 PAY 215 

    0x1d9dc3b7,// 219 PAY 216 

    0x5a276947,// 220 PAY 217 

    0xe12f0469,// 221 PAY 218 

    0xcdb142b0,// 222 PAY 219 

    0x582450bd,// 223 PAY 220 

    0x082dc4f0,// 224 PAY 221 

    0xdb61169d,// 225 PAY 222 

    0xd70d8214,// 226 PAY 223 

    0x71093e50,// 227 PAY 224 

    0xe6d46826,// 228 PAY 225 

    0x544547ce,// 229 PAY 226 

    0x4aebcdce,// 230 PAY 227 

    0xcfb5fc5f,// 231 PAY 228 

    0x7e78963d,// 232 PAY 229 

    0x3c098fab,// 233 PAY 230 

    0x99a04fbf,// 234 PAY 231 

    0x215bf313,// 235 PAY 232 

    0x648f4358,// 236 PAY 233 

    0x21dfa479,// 237 PAY 234 

    0x3ff61a7f,// 238 PAY 235 

    0x091c03e4,// 239 PAY 236 

    0xf6b59947,// 240 PAY 237 

    0x7ecf2943,// 241 PAY 238 

    0x5b75308f,// 242 PAY 239 

    0xbd93d5d9,// 243 PAY 240 

    0xc693ce4c,// 244 PAY 241 

    0x2250d1a3,// 245 PAY 242 

    0x215cd62e,// 246 PAY 243 

    0x203fb0e0,// 247 PAY 244 

    0x10d44d56,// 248 PAY 245 

    0x600b65a5,// 249 PAY 246 

    0x0ef18eee,// 250 PAY 247 

    0x897bf6a8,// 251 PAY 248 

    0xd1b72c47,// 252 PAY 249 

    0x5cbd95bc,// 253 PAY 250 

    0xbac4a43e,// 254 PAY 251 

    0xa5b39e40,// 255 PAY 252 

    0x20b7501f,// 256 PAY 253 

    0xb55fd748,// 257 PAY 254 

    0x7a17b832,// 258 PAY 255 

    0xb0f0bff3,// 259 PAY 256 

    0x37ae3c0a,// 260 PAY 257 

    0xfc5acb2e,// 261 PAY 258 

    0x623b85bf,// 262 PAY 259 

    0x79ca7d62,// 263 PAY 260 

    0x4797b17e,// 264 PAY 261 

    0xa1013847,// 265 PAY 262 

    0x90b68566,// 266 PAY 263 

    0x251dbe84,// 267 PAY 264 

    0xbb521501,// 268 PAY 265 

    0x254209d7,// 269 PAY 266 

    0xb481ee7d,// 270 PAY 267 

    0xcca6c4f2,// 271 PAY 268 

    0x73c77860,// 272 PAY 269 

    0xccccf6ab,// 273 PAY 270 

    0x7f30b95b,// 274 PAY 271 

    0xaff7a182,// 275 PAY 272 

    0x33e5eb96,// 276 PAY 273 

    0x7c13d8ec,// 277 PAY 274 

    0x05a60ea7,// 278 PAY 275 

    0xdaccf557,// 279 PAY 276 

    0xc971e42a,// 280 PAY 277 

    0x70092fbd,// 281 PAY 278 

    0x5a378c3e,// 282 PAY 279 

    0x417f4264,// 283 PAY 280 

    0xd8047429,// 284 PAY 281 

    0x3806fba2,// 285 PAY 282 

    0xc98a737e,// 286 PAY 283 

    0x43f41260,// 287 PAY 284 

    0xa3f9307e,// 288 PAY 285 

    0xfd2943ec,// 289 PAY 286 

    0x340e78d7,// 290 PAY 287 

    0xf379c5e8,// 291 PAY 288 

    0x51e58937,// 292 PAY 289 

    0xb46807da,// 293 PAY 290 

    0xa7a46b3b,// 294 PAY 291 

    0xad12e26c,// 295 PAY 292 

    0xcb90248e,// 296 PAY 293 

    0x97414d59,// 297 PAY 294 

    0x7c605e1d,// 298 PAY 295 

    0x55bf1166,// 299 PAY 296 

    0x392cc722,// 300 PAY 297 

    0x2893d899,// 301 PAY 298 

    0xb6fa206c,// 302 PAY 299 

    0x3dd1b205,// 303 PAY 300 

    0x26a67dd2,// 304 PAY 301 

    0x8a363609,// 305 PAY 302 

    0x581cec8c,// 306 PAY 303 

    0x282624f7,// 307 PAY 304 

    0xeaf6e7eb,// 308 PAY 305 

    0xa3fbfa90,// 309 PAY 306 

    0x505f5aac,// 310 PAY 307 

    0xaa4f9896,// 311 PAY 308 

    0xbb4f8dda,// 312 PAY 309 

    0x95a95f18,// 313 PAY 310 

    0x88a3f1a3,// 314 PAY 311 

    0xbdd667bf,// 315 PAY 312 

    0xac89a848,// 316 PAY 313 

    0x74db39bd,// 317 PAY 314 

    0x5dbc50fb,// 318 PAY 315 

    0x868aaf42,// 319 PAY 316 

    0x1d46926b,// 320 PAY 317 

    0x28998aa1,// 321 PAY 318 

    0xcc8a850d,// 322 PAY 319 

    0x75a3b33c,// 323 PAY 320 

    0x34370c38,// 324 PAY 321 

    0x1007d2d7,// 325 PAY 322 

    0x5e208c02,// 326 PAY 323 

    0x9d535bfc,// 327 PAY 324 

    0x1d32e921,// 328 PAY 325 

    0xd9e33271,// 329 PAY 326 

    0x7ac1141b,// 330 PAY 327 

    0x55281aad,// 331 PAY 328 

    0x8b8ef928,// 332 PAY 329 

    0x8ead79b1,// 333 PAY 330 

    0x8f3723da,// 334 PAY 331 

    0x7dc15b2a,// 335 PAY 332 

    0xdd465fd7,// 336 PAY 333 

    0xd177c562,// 337 PAY 334 

    0x80795d2c,// 338 PAY 335 

    0x0748e006,// 339 PAY 336 

    0x27334e67,// 340 PAY 337 

    0x9d1c40c5,// 341 PAY 338 

    0xfc9647e7,// 342 PAY 339 

    0x28256c28,// 343 PAY 340 

    0x242b5c04,// 344 PAY 341 

    0xb78c30b2,// 345 PAY 342 

    0x4d7b0e42,// 346 PAY 343 

    0x06e1588d,// 347 PAY 344 

    0xc73a2744,// 348 PAY 345 

    0x388810f2,// 349 PAY 346 

    0xae418c6b,// 350 PAY 347 

    0xcfcb4cc5,// 351 PAY 348 

    0x3a9f6e87,// 352 PAY 349 

    0x43852f53,// 353 PAY 350 

    0x6e856d8c,// 354 PAY 351 

    0xb09fadd4,// 355 PAY 352 

    0xdd7dadb2,// 356 PAY 353 

    0x729433ec,// 357 PAY 354 

    0xb59e2f80,// 358 PAY 355 

    0x97187eeb,// 359 PAY 356 

    0x259d7782,// 360 PAY 357 

    0x5f4b2f78,// 361 PAY 358 

    0x7c9f9459,// 362 PAY 359 

    0xe2b32441,// 363 PAY 360 

    0xebc6bdc0,// 364 PAY 361 

    0x806333b1,// 365 PAY 362 

    0x7c482afe,// 366 PAY 363 

    0xfe960f66,// 367 PAY 364 

    0x04fb96a0,// 368 PAY 365 

    0xddbb9ffb,// 369 PAY 366 

    0x8b982a3b,// 370 PAY 367 

    0xc2f5cca7,// 371 PAY 368 

    0x408144bf,// 372 PAY 369 

    0xe325347e,// 373 PAY 370 

    0xdb2b17a5,// 374 PAY 371 

    0xdbae6bcb,// 375 PAY 372 

    0x4821ec4e,// 376 PAY 373 

    0x9051edbc,// 377 PAY 374 

    0x61643ead,// 378 PAY 375 

    0x67d23f62,// 379 PAY 376 

    0xb4ff714e,// 380 PAY 377 

    0x6d406be7,// 381 PAY 378 

    0x18ec2796,// 382 PAY 379 

    0x110211af,// 383 PAY 380 

    0x75776ad5,// 384 PAY 381 

    0x879adc70,// 385 PAY 382 

    0x29ec0cbd,// 386 PAY 383 

    0xe5d9b18a,// 387 PAY 384 

    0x6ff39f74,// 388 PAY 385 

    0x8a21d946,// 389 PAY 386 

    0x9a804654,// 390 PAY 387 

    0x868bfdd9,// 391 PAY 388 

    0x7d42706e,// 392 PAY 389 

    0x30aa3a11,// 393 PAY 390 

    0x702b87ee,// 394 PAY 391 

    0x191397e1,// 395 PAY 392 

    0xe125f8c2,// 396 PAY 393 

    0xc339efbc,// 397 PAY 394 

    0xe44a287a,// 398 PAY 395 

    0x35d22e61,// 399 PAY 396 

    0xecb449ad,// 400 PAY 397 

    0x9db764c5,// 401 PAY 398 

    0x423dc51f,// 402 PAY 399 

    0x12b31864,// 403 PAY 400 

    0xa9090c9d,// 404 PAY 401 

    0xa8690700,// 405 PAY 402 

    0x1090116b,// 406 PAY 403 

    0x8c54df86,// 407 PAY 404 

    0x3aa14744,// 408 PAY 405 

    0x1e564b9e,// 409 PAY 406 

    0x144012f6,// 410 PAY 407 

    0xad3ff031,// 411 PAY 408 

    0x10b6df8a,// 412 PAY 409 

    0xfab6988c,// 413 PAY 410 

    0xb0f48a97,// 414 PAY 411 

    0xaa908c81,// 415 PAY 412 

    0x6b318090,// 416 PAY 413 

    0x1027b3ae,// 417 PAY 414 

    0xf6f82248,// 418 PAY 415 

    0x408bfdd5,// 419 PAY 416 

    0x05c1d5fe,// 420 PAY 417 

    0xb82dd639,// 421 PAY 418 

    0x7dd9b21b,// 422 PAY 419 

    0x87b60540,// 423 PAY 420 

    0x50f8752d,// 424 PAY 421 

    0xe8f0cbad,// 425 PAY 422 

    0x4284027f,// 426 PAY 423 

    0x3b8f6e08,// 427 PAY 424 

    0xd1b37864,// 428 PAY 425 

    0x1043b7b1,// 429 PAY 426 

    0x1db2205b,// 430 PAY 427 

    0x506db5f7,// 431 PAY 428 

    0x791c71d7,// 432 PAY 429 

    0xac73a605,// 433 PAY 430 

    0xad211e73,// 434 PAY 431 

    0x26e7bd45,// 435 PAY 432 

    0xba519b2e,// 436 PAY 433 

    0x7081f36a,// 437 PAY 434 

    0x3d0813c4,// 438 PAY 435 

    0x65ff7bf9,// 439 PAY 436 

    0x4f14436d,// 440 PAY 437 

    0xb5cb3cea,// 441 PAY 438 

    0x67e60c4e,// 442 PAY 439 

    0x3d939651,// 443 PAY 440 

    0x0e8e46ff,// 444 PAY 441 

    0x83a1c9a5,// 445 PAY 442 

    0xdf062ae9,// 446 PAY 443 

    0xf275883d,// 447 PAY 444 

    0xe9f3cfc1,// 448 PAY 445 

    0x1ba8f031,// 449 PAY 446 

    0xc5f8f6ce,// 450 PAY 447 

    0x27748d78,// 451 PAY 448 

    0xb1cb3b3b,// 452 PAY 449 

    0xefd16e93,// 453 PAY 450 

    0xb9dd327a,// 454 PAY 451 

    0x5b4ca43f,// 455 PAY 452 

    0x3a971a1c,// 456 PAY 453 

    0xaa8fa868,// 457 PAY 454 

    0x2e9c8905,// 458 PAY 455 

    0x52f87326,// 459 PAY 456 

    0x0d0354f9,// 460 PAY 457 

    0x79a090b0,// 461 PAY 458 

    0xdbafda81,// 462 PAY 459 

    0x0f0c2351,// 463 PAY 460 

    0x126055a7,// 464 PAY 461 

    0x424f5e6c,// 465 PAY 462 

    0x73b3384c,// 466 PAY 463 

    0xef422d63,// 467 PAY 464 

    0xd83e5f52,// 468 PAY 465 

    0x12a37b30,// 469 PAY 466 

    0x66696ab0,// 470 PAY 467 

    0xaf8dd456,// 471 PAY 468 

    0xc3c02fad,// 472 PAY 469 

    0x62797a37,// 473 PAY 470 

    0x4d220697,// 474 PAY 471 

    0x8533cb32,// 475 PAY 472 

    0x55bbdca5,// 476 PAY 473 

    0x31f4b3ff,// 477 PAY 474 

    0x11d0afa2,// 478 PAY 475 

    0x81e695b6,// 479 PAY 476 

    0x8e4ea732,// 480 PAY 477 

    0x2a7d2fb9,// 481 PAY 478 

    0x03153249,// 482 PAY 479 

    0x2be30d51,// 483 PAY 480 

    0x93c542e3,// 484 PAY 481 

    0x7ff2a896,// 485 PAY 482 

    0xef3c2eda,// 486 PAY 483 

    0x34e080c5,// 487 PAY 484 

    0x7d000000,// 488 PAY 485 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 70 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcf 

    0x0119cf74 // 489 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt13_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 362 words. 

/// BDA size     is 1441 (0x5a1) 

/// BDA id       is 0x91a 

    0x05a1091a,// 3 BDA   1 

/// PAY Generic Data size   : 1441 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x73a62440,// 4 PAY   1 

    0x97a59904,// 5 PAY   2 

    0xda4b990a,// 6 PAY   3 

    0xc20c764b,// 7 PAY   4 

    0x0d03f0bd,// 8 PAY   5 

    0xe167a0d6,// 9 PAY   6 

    0xec2b0e57,// 10 PAY   7 

    0xc379a17a,// 11 PAY   8 

    0x3bacbe89,// 12 PAY   9 

    0x7eaa7ade,// 13 PAY  10 

    0xaa102562,// 14 PAY  11 

    0xf95478dc,// 15 PAY  12 

    0x6bcd9ccd,// 16 PAY  13 

    0x21adb193,// 17 PAY  14 

    0x6c93a565,// 18 PAY  15 

    0x166f7124,// 19 PAY  16 

    0xecd272c4,// 20 PAY  17 

    0xe0933108,// 21 PAY  18 

    0xd6880d1d,// 22 PAY  19 

    0xc4105c9b,// 23 PAY  20 

    0x277432c2,// 24 PAY  21 

    0xc30a8b78,// 25 PAY  22 

    0x9ced9b95,// 26 PAY  23 

    0x1446d2a9,// 27 PAY  24 

    0x9560c5c1,// 28 PAY  25 

    0xb1656469,// 29 PAY  26 

    0x6585fc3a,// 30 PAY  27 

    0x5a3dfe46,// 31 PAY  28 

    0xbd60cd8c,// 32 PAY  29 

    0x1a8a0cfb,// 33 PAY  30 

    0xe1a72da2,// 34 PAY  31 

    0x416c8514,// 35 PAY  32 

    0x2e932ee9,// 36 PAY  33 

    0xcb5ed8a0,// 37 PAY  34 

    0xb38cce61,// 38 PAY  35 

    0xbeb0a704,// 39 PAY  36 

    0x41b0f536,// 40 PAY  37 

    0x54ff7bee,// 41 PAY  38 

    0x950e5313,// 42 PAY  39 

    0xf503b557,// 43 PAY  40 

    0xdacba06c,// 44 PAY  41 

    0x4a4c4e0b,// 45 PAY  42 

    0x35a841bd,// 46 PAY  43 

    0xde508424,// 47 PAY  44 

    0x2fab6984,// 48 PAY  45 

    0x7d6db8ee,// 49 PAY  46 

    0x0e7d04a4,// 50 PAY  47 

    0x54d3042a,// 51 PAY  48 

    0xa08c504f,// 52 PAY  49 

    0x7737506a,// 53 PAY  50 

    0x32fcc2d9,// 54 PAY  51 

    0xbdcdcd1f,// 55 PAY  52 

    0x3a29edda,// 56 PAY  53 

    0xc9c85418,// 57 PAY  54 

    0x585741b1,// 58 PAY  55 

    0x94b9c742,// 59 PAY  56 

    0xbb6c2553,// 60 PAY  57 

    0xaff12e29,// 61 PAY  58 

    0x2ae46f42,// 62 PAY  59 

    0xa0ff06df,// 63 PAY  60 

    0x4343994b,// 64 PAY  61 

    0xdd131482,// 65 PAY  62 

    0x7ba9e498,// 66 PAY  63 

    0x73a51dbb,// 67 PAY  64 

    0xa3e8a7b8,// 68 PAY  65 

    0xa062be02,// 69 PAY  66 

    0x5381da81,// 70 PAY  67 

    0x9ec04b8b,// 71 PAY  68 

    0xca2b3e22,// 72 PAY  69 

    0x24b14d45,// 73 PAY  70 

    0xf1e9aa44,// 74 PAY  71 

    0x72c8069f,// 75 PAY  72 

    0x10aa87af,// 76 PAY  73 

    0xfa00f9b6,// 77 PAY  74 

    0x126c6630,// 78 PAY  75 

    0x187af8d8,// 79 PAY  76 

    0xcb6e5ad3,// 80 PAY  77 

    0x9403e84b,// 81 PAY  78 

    0x4b69121e,// 82 PAY  79 

    0x84c385f6,// 83 PAY  80 

    0xad0277ee,// 84 PAY  81 

    0xb4ddd5ef,// 85 PAY  82 

    0x8bba353c,// 86 PAY  83 

    0x3e7dec84,// 87 PAY  84 

    0x010b0da9,// 88 PAY  85 

    0xc8088058,// 89 PAY  86 

    0x25a81c0b,// 90 PAY  87 

    0xbda826c4,// 91 PAY  88 

    0x4e7c21cc,// 92 PAY  89 

    0x73f3d61d,// 93 PAY  90 

    0xe739ae09,// 94 PAY  91 

    0x7791aabb,// 95 PAY  92 

    0xd9fc40dd,// 96 PAY  93 

    0x69a4dbc9,// 97 PAY  94 

    0x88fffb9e,// 98 PAY  95 

    0x6d8d5609,// 99 PAY  96 

    0xfd308f38,// 100 PAY  97 

    0x10b1911e,// 101 PAY  98 

    0xc8c294d6,// 102 PAY  99 

    0xc4bbb7e5,// 103 PAY 100 

    0xd967c849,// 104 PAY 101 

    0x88e750e4,// 105 PAY 102 

    0xadbd7f91,// 106 PAY 103 

    0x7efd05dd,// 107 PAY 104 

    0x866efb02,// 108 PAY 105 

    0x12391a15,// 109 PAY 106 

    0xf55eb464,// 110 PAY 107 

    0x317e0f29,// 111 PAY 108 

    0xf25d5037,// 112 PAY 109 

    0x71a833b6,// 113 PAY 110 

    0x1648e696,// 114 PAY 111 

    0xcb9949a1,// 115 PAY 112 

    0xe8f83439,// 116 PAY 113 

    0xa06cc068,// 117 PAY 114 

    0x3b0c095c,// 118 PAY 115 

    0x6cc70313,// 119 PAY 116 

    0xacab14c6,// 120 PAY 117 

    0xfd51b6bf,// 121 PAY 118 

    0x08754a4b,// 122 PAY 119 

    0xd4615f45,// 123 PAY 120 

    0x998ecf25,// 124 PAY 121 

    0xc8d8fd59,// 125 PAY 122 

    0xab0fb83b,// 126 PAY 123 

    0x4e8ac1c6,// 127 PAY 124 

    0x96fee4d9,// 128 PAY 125 

    0x7d8fc21e,// 129 PAY 126 

    0x22627f0e,// 130 PAY 127 

    0xcd90384b,// 131 PAY 128 

    0xb5b66c99,// 132 PAY 129 

    0x4c1061f8,// 133 PAY 130 

    0xb068a0ee,// 134 PAY 131 

    0x08fef841,// 135 PAY 132 

    0xfaf8a575,// 136 PAY 133 

    0xb813de9c,// 137 PAY 134 

    0xb3618e06,// 138 PAY 135 

    0xcb05bda7,// 139 PAY 136 

    0xf48e760e,// 140 PAY 137 

    0xe389c79a,// 141 PAY 138 

    0x8d53bbf1,// 142 PAY 139 

    0x94f471eb,// 143 PAY 140 

    0x9653551d,// 144 PAY 141 

    0x33dec2b6,// 145 PAY 142 

    0xd6c2dfef,// 146 PAY 143 

    0xd826ef6a,// 147 PAY 144 

    0x2249f93c,// 148 PAY 145 

    0x8cfdef87,// 149 PAY 146 

    0x896a9652,// 150 PAY 147 

    0x1116aa46,// 151 PAY 148 

    0x4020cbc9,// 152 PAY 149 

    0x9687cbb1,// 153 PAY 150 

    0x9958aabe,// 154 PAY 151 

    0xadad8167,// 155 PAY 152 

    0xd71e9aa8,// 156 PAY 153 

    0x93518a69,// 157 PAY 154 

    0xf494939c,// 158 PAY 155 

    0x0237e022,// 159 PAY 156 

    0xedd14f98,// 160 PAY 157 

    0x85c08de7,// 161 PAY 158 

    0xe4e59b7d,// 162 PAY 159 

    0xe3925297,// 163 PAY 160 

    0xaa0a91cd,// 164 PAY 161 

    0x9128a168,// 165 PAY 162 

    0xe6abc90f,// 166 PAY 163 

    0x86a6052c,// 167 PAY 164 

    0xb47342aa,// 168 PAY 165 

    0xe3b106b0,// 169 PAY 166 

    0x7671395b,// 170 PAY 167 

    0xe0d0d9c7,// 171 PAY 168 

    0x67edc5af,// 172 PAY 169 

    0x405d83cf,// 173 PAY 170 

    0x9d3942a0,// 174 PAY 171 

    0x5258282a,// 175 PAY 172 

    0x51732f05,// 176 PAY 173 

    0xe363d5d8,// 177 PAY 174 

    0xb826efa5,// 178 PAY 175 

    0xb3a95439,// 179 PAY 176 

    0x881fa2a4,// 180 PAY 177 

    0xa04d2df1,// 181 PAY 178 

    0x5a8dc412,// 182 PAY 179 

    0xaded396b,// 183 PAY 180 

    0x1834a98b,// 184 PAY 181 

    0x898391a9,// 185 PAY 182 

    0x0c6531de,// 186 PAY 183 

    0xef7dd1d8,// 187 PAY 184 

    0x387276fa,// 188 PAY 185 

    0x618f2c95,// 189 PAY 186 

    0xbb84c89a,// 190 PAY 187 

    0xa69f8a61,// 191 PAY 188 

    0x714e2fef,// 192 PAY 189 

    0xcd719bf0,// 193 PAY 190 

    0xf538704d,// 194 PAY 191 

    0x9448a395,// 195 PAY 192 

    0x88e06ce4,// 196 PAY 193 

    0x1ade376a,// 197 PAY 194 

    0x5df95e5d,// 198 PAY 195 

    0x94bd1ce2,// 199 PAY 196 

    0x7201d5a3,// 200 PAY 197 

    0x97f496fc,// 201 PAY 198 

    0x5b3d9525,// 202 PAY 199 

    0xddf2f8ac,// 203 PAY 200 

    0x8976c38e,// 204 PAY 201 

    0xee16a877,// 205 PAY 202 

    0xd95d774f,// 206 PAY 203 

    0x793ebd99,// 207 PAY 204 

    0x0600857e,// 208 PAY 205 

    0xe90c9640,// 209 PAY 206 

    0x0d723895,// 210 PAY 207 

    0x160f5cee,// 211 PAY 208 

    0xd0a3599d,// 212 PAY 209 

    0x7351462a,// 213 PAY 210 

    0x76b7e299,// 214 PAY 211 

    0x13c5dd27,// 215 PAY 212 

    0x8eb11bf7,// 216 PAY 213 

    0xc9ce8933,// 217 PAY 214 

    0x2337fb2b,// 218 PAY 215 

    0x62896df6,// 219 PAY 216 

    0x7d91e9ad,// 220 PAY 217 

    0x4dadf760,// 221 PAY 218 

    0xc0724ac2,// 222 PAY 219 

    0x2d102bb6,// 223 PAY 220 

    0xdb46a07f,// 224 PAY 221 

    0x1eaa06f1,// 225 PAY 222 

    0x91692df1,// 226 PAY 223 

    0xf7eab869,// 227 PAY 224 

    0xc3b20574,// 228 PAY 225 

    0xe19bf4c4,// 229 PAY 226 

    0xc15da5c3,// 230 PAY 227 

    0x24b7f79d,// 231 PAY 228 

    0xf1d5ab72,// 232 PAY 229 

    0x016d8988,// 233 PAY 230 

    0x6373c78b,// 234 PAY 231 

    0xe41ad8a2,// 235 PAY 232 

    0x50c570a1,// 236 PAY 233 

    0x9c557c3e,// 237 PAY 234 

    0x9082c806,// 238 PAY 235 

    0xbf25879f,// 239 PAY 236 

    0x5bd1cbb8,// 240 PAY 237 

    0xcbf8bee0,// 241 PAY 238 

    0xb3e2b971,// 242 PAY 239 

    0x10d38568,// 243 PAY 240 

    0x3dc947ba,// 244 PAY 241 

    0x7c6ac4f2,// 245 PAY 242 

    0xdb8abeec,// 246 PAY 243 

    0x9be7658d,// 247 PAY 244 

    0x123659ad,// 248 PAY 245 

    0x4d199c2f,// 249 PAY 246 

    0x4c623984,// 250 PAY 247 

    0xaba96ccc,// 251 PAY 248 

    0xd8c6ac42,// 252 PAY 249 

    0x6f825c19,// 253 PAY 250 

    0xa0ef5588,// 254 PAY 251 

    0x1f324b30,// 255 PAY 252 

    0x3e48bae0,// 256 PAY 253 

    0x524fbfb5,// 257 PAY 254 

    0x4870fe17,// 258 PAY 255 

    0x039ebc44,// 259 PAY 256 

    0x7d9908db,// 260 PAY 257 

    0x73d0b925,// 261 PAY 258 

    0x80236510,// 262 PAY 259 

    0xf808c71a,// 263 PAY 260 

    0x4c02e872,// 264 PAY 261 

    0x3eb71bb0,// 265 PAY 262 

    0x8d970c7a,// 266 PAY 263 

    0x60411262,// 267 PAY 264 

    0x009d0204,// 268 PAY 265 

    0xb019451f,// 269 PAY 266 

    0x6d51377e,// 270 PAY 267 

    0xda649e2f,// 271 PAY 268 

    0x88d2e594,// 272 PAY 269 

    0x53eefeb0,// 273 PAY 270 

    0xe7ef110f,// 274 PAY 271 

    0x8b09a8b7,// 275 PAY 272 

    0xbb1611f6,// 276 PAY 273 

    0x3352a938,// 277 PAY 274 

    0x3954c5ef,// 278 PAY 275 

    0x4ffeb2ae,// 279 PAY 276 

    0x982779ed,// 280 PAY 277 

    0x176b023a,// 281 PAY 278 

    0x5f24552b,// 282 PAY 279 

    0x9543135b,// 283 PAY 280 

    0x47a29bf5,// 284 PAY 281 

    0x629ab7b5,// 285 PAY 282 

    0x05960861,// 286 PAY 283 

    0xce5fd20f,// 287 PAY 284 

    0x2d67bc6c,// 288 PAY 285 

    0x998308e9,// 289 PAY 286 

    0x4ae3ec89,// 290 PAY 287 

    0x7e2e0e42,// 291 PAY 288 

    0x74189848,// 292 PAY 289 

    0xcb6e2041,// 293 PAY 290 

    0xb816ae5e,// 294 PAY 291 

    0x9fad92d0,// 295 PAY 292 

    0x5c92bd67,// 296 PAY 293 

    0x1b192721,// 297 PAY 294 

    0xeb93652f,// 298 PAY 295 

    0xc104bcb7,// 299 PAY 296 

    0xe9c9975e,// 300 PAY 297 

    0x2481f8a0,// 301 PAY 298 

    0xb110d244,// 302 PAY 299 

    0x9843be5e,// 303 PAY 300 

    0x405eb80d,// 304 PAY 301 

    0x5385c5b9,// 305 PAY 302 

    0xbd466b20,// 306 PAY 303 

    0x510af210,// 307 PAY 304 

    0x26467813,// 308 PAY 305 

    0x00735a92,// 309 PAY 306 

    0x905b7edd,// 310 PAY 307 

    0x514c0e54,// 311 PAY 308 

    0x1810e595,// 312 PAY 309 

    0x1059845a,// 313 PAY 310 

    0x3b8f2324,// 314 PAY 311 

    0xae6d7ab3,// 315 PAY 312 

    0xce58e1ce,// 316 PAY 313 

    0xf568761e,// 317 PAY 314 

    0x672b4eec,// 318 PAY 315 

    0x92426fcc,// 319 PAY 316 

    0x4cbee6b7,// 320 PAY 317 

    0x8a91aa14,// 321 PAY 318 

    0x8805bd1c,// 322 PAY 319 

    0x21527953,// 323 PAY 320 

    0x0c2edbd8,// 324 PAY 321 

    0x40d41120,// 325 PAY 322 

    0x95b186ba,// 326 PAY 323 

    0x7a8de434,// 327 PAY 324 

    0xd2ac05de,// 328 PAY 325 

    0x5efebf5d,// 329 PAY 326 

    0x29e49c37,// 330 PAY 327 

    0xf80db2be,// 331 PAY 328 

    0xb0f02b15,// 332 PAY 329 

    0x0e483d02,// 333 PAY 330 

    0x2be58b6d,// 334 PAY 331 

    0xa05165ad,// 335 PAY 332 

    0xdac5bd2f,// 336 PAY 333 

    0x3fc95823,// 337 PAY 334 

    0x4240fbf0,// 338 PAY 335 

    0x75428b2a,// 339 PAY 336 

    0xd535f1e3,// 340 PAY 337 

    0x7216c36f,// 341 PAY 338 

    0xf7f8b7b1,// 342 PAY 339 

    0x7df709b3,// 343 PAY 340 

    0x20115765,// 344 PAY 341 

    0xcfcb6f49,// 345 PAY 342 

    0x5cf1216e,// 346 PAY 343 

    0xd2758d44,// 347 PAY 344 

    0xe51ed5d1,// 348 PAY 345 

    0x6cd98e4d,// 349 PAY 346 

    0x501dd550,// 350 PAY 347 

    0x827ae6b9,// 351 PAY 348 

    0xefaa9668,// 352 PAY 349 

    0x5ecafdf3,// 353 PAY 350 

    0xa3f44401,// 354 PAY 351 

    0x056e5e45,// 355 PAY 352 

    0x4022a876,// 356 PAY 353 

    0xe540b7ed,// 357 PAY 354 

    0x19ee1847,// 358 PAY 355 

    0x035825ca,// 359 PAY 356 

    0xeb990693,// 360 PAY 357 

    0x4ef4b8fa,// 361 PAY 358 

    0xdebb2777,// 362 PAY 359 

    0x9292866d,// 363 PAY 360 

    0x98000000,// 364 PAY 361 

/// HASH is  16 bytes 

    0x7c6ac4f2,// 365 HSH   1 

    0xdb8abeec,// 366 HSH   2 

    0x9be7658d,// 367 HSH   3 

    0x123659ad,// 368 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 168 

/// STA pkt_idx        : 182 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3e 

    0x02d93ea8 // 369 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt14_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 307 words. 

/// BDA size     is 1223 (0x4c7) 

/// BDA id       is 0x241b 

    0x04c7241b,// 3 BDA   1 

/// PAY Generic Data size   : 1223 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x81c03aa5,// 4 PAY   1 

    0x09da4a78,// 5 PAY   2 

    0xb3427d9c,// 6 PAY   3 

    0xc6e5a3c2,// 7 PAY   4 

    0x743061ac,// 8 PAY   5 

    0x4ea492cb,// 9 PAY   6 

    0x76515bf1,// 10 PAY   7 

    0x2b8022d2,// 11 PAY   8 

    0x257034b2,// 12 PAY   9 

    0x422ac679,// 13 PAY  10 

    0x73c319ab,// 14 PAY  11 

    0x06b056c5,// 15 PAY  12 

    0xbfd1f217,// 16 PAY  13 

    0x4b83ca9b,// 17 PAY  14 

    0x3f8d53ae,// 18 PAY  15 

    0x881a5d53,// 19 PAY  16 

    0xd10c08ce,// 20 PAY  17 

    0x71ef1af1,// 21 PAY  18 

    0x07f9dbe8,// 22 PAY  19 

    0xaf6224de,// 23 PAY  20 

    0xdc23a71f,// 24 PAY  21 

    0xb7707bc2,// 25 PAY  22 

    0x331d27ec,// 26 PAY  23 

    0x71959409,// 27 PAY  24 

    0xb735aabc,// 28 PAY  25 

    0x677e2e93,// 29 PAY  26 

    0x5f5d0d6b,// 30 PAY  27 

    0xbe7d157c,// 31 PAY  28 

    0x79638599,// 32 PAY  29 

    0x8556dfcb,// 33 PAY  30 

    0xd9998262,// 34 PAY  31 

    0x10d0a3d4,// 35 PAY  32 

    0x157a6342,// 36 PAY  33 

    0xc8b81c36,// 37 PAY  34 

    0xbb31a9c2,// 38 PAY  35 

    0x5808d80a,// 39 PAY  36 

    0x529739a0,// 40 PAY  37 

    0x7a244752,// 41 PAY  38 

    0x04d480a5,// 42 PAY  39 

    0x55113da5,// 43 PAY  40 

    0x30ed947e,// 44 PAY  41 

    0x23b79a99,// 45 PAY  42 

    0x620c5a03,// 46 PAY  43 

    0x2b5baab4,// 47 PAY  44 

    0x47c36590,// 48 PAY  45 

    0xdb731131,// 49 PAY  46 

    0x77b10416,// 50 PAY  47 

    0xfbbf9fc6,// 51 PAY  48 

    0xd0808cf3,// 52 PAY  49 

    0xaa555001,// 53 PAY  50 

    0x5a026e27,// 54 PAY  51 

    0x9afd7829,// 55 PAY  52 

    0x64600b20,// 56 PAY  53 

    0x6262029f,// 57 PAY  54 

    0x3c0606e1,// 58 PAY  55 

    0x29eb2d20,// 59 PAY  56 

    0x7292e06a,// 60 PAY  57 

    0x89ce8e3b,// 61 PAY  58 

    0x857df85e,// 62 PAY  59 

    0x0d58669a,// 63 PAY  60 

    0x7df2bfce,// 64 PAY  61 

    0xf3faa90d,// 65 PAY  62 

    0x8b953730,// 66 PAY  63 

    0xe1da9a0b,// 67 PAY  64 

    0xf461a904,// 68 PAY  65 

    0x99e11925,// 69 PAY  66 

    0x9fb9cbb4,// 70 PAY  67 

    0xbef5be75,// 71 PAY  68 

    0x5f5495d3,// 72 PAY  69 

    0xc9e7b6e5,// 73 PAY  70 

    0xf17410ff,// 74 PAY  71 

    0x2a665850,// 75 PAY  72 

    0x868a2201,// 76 PAY  73 

    0xfda41e4e,// 77 PAY  74 

    0x08de0e03,// 78 PAY  75 

    0x6c1baf41,// 79 PAY  76 

    0xdd44d23c,// 80 PAY  77 

    0x67a8c950,// 81 PAY  78 

    0x2521c791,// 82 PAY  79 

    0x68bf1c3c,// 83 PAY  80 

    0xf2614413,// 84 PAY  81 

    0x61b4927c,// 85 PAY  82 

    0x80dc3580,// 86 PAY  83 

    0x110f8683,// 87 PAY  84 

    0x31da77ee,// 88 PAY  85 

    0x9ab219e3,// 89 PAY  86 

    0xd98ea79b,// 90 PAY  87 

    0x13e366d5,// 91 PAY  88 

    0xc5963aaf,// 92 PAY  89 

    0xfcb31b4f,// 93 PAY  90 

    0x7274b7a0,// 94 PAY  91 

    0xdbfe69f3,// 95 PAY  92 

    0x0d0793ce,// 96 PAY  93 

    0x4a92f5de,// 97 PAY  94 

    0xf4ba4827,// 98 PAY  95 

    0x109ead96,// 99 PAY  96 

    0x1191e4c7,// 100 PAY  97 

    0x1d0b70a1,// 101 PAY  98 

    0xef4206dc,// 102 PAY  99 

    0x5fba0ee1,// 103 PAY 100 

    0xecb8a7ae,// 104 PAY 101 

    0x1663b533,// 105 PAY 102 

    0x959d47a0,// 106 PAY 103 

    0x020fce43,// 107 PAY 104 

    0xccfad243,// 108 PAY 105 

    0x3472f9ae,// 109 PAY 106 

    0x1193bcf4,// 110 PAY 107 

    0x5d6525f0,// 111 PAY 108 

    0x254d3a1b,// 112 PAY 109 

    0x1bdb8fbb,// 113 PAY 110 

    0x7dcde2cb,// 114 PAY 111 

    0x8b1c72ca,// 115 PAY 112 

    0xda00e207,// 116 PAY 113 

    0xef5cf690,// 117 PAY 114 

    0xf819b5d4,// 118 PAY 115 

    0x217f8f27,// 119 PAY 116 

    0x49d2a6ff,// 120 PAY 117 

    0xc83b2f92,// 121 PAY 118 

    0x35bcb58f,// 122 PAY 119 

    0x5e117217,// 123 PAY 120 

    0xf52bb811,// 124 PAY 121 

    0xed76b3f4,// 125 PAY 122 

    0xf7138d47,// 126 PAY 123 

    0xec368332,// 127 PAY 124 

    0xcfb6cd39,// 128 PAY 125 

    0x1cf2d476,// 129 PAY 126 

    0x971a681e,// 130 PAY 127 

    0x209e7e7c,// 131 PAY 128 

    0xe54816fb,// 132 PAY 129 

    0x2b7c8ca3,// 133 PAY 130 

    0x75515b15,// 134 PAY 131 

    0xd6788929,// 135 PAY 132 

    0xc7c3c436,// 136 PAY 133 

    0xf3241fb1,// 137 PAY 134 

    0x95a69561,// 138 PAY 135 

    0xe13b129a,// 139 PAY 136 

    0x5d3d4189,// 140 PAY 137 

    0x7ec298d3,// 141 PAY 138 

    0x2395f052,// 142 PAY 139 

    0x1c9f1ba5,// 143 PAY 140 

    0x4330091c,// 144 PAY 141 

    0xb417f212,// 145 PAY 142 

    0x94cfa75e,// 146 PAY 143 

    0xfe931f1d,// 147 PAY 144 

    0xb6a9844e,// 148 PAY 145 

    0x346c6f11,// 149 PAY 146 

    0xc2d50b8f,// 150 PAY 147 

    0x97b1acaa,// 151 PAY 148 

    0x12a837d2,// 152 PAY 149 

    0x2d5148fa,// 153 PAY 150 

    0x9b7c69d3,// 154 PAY 151 

    0x04c38b92,// 155 PAY 152 

    0x7e2674ac,// 156 PAY 153 

    0x7decebd0,// 157 PAY 154 

    0x12b813ff,// 158 PAY 155 

    0xe71aacfd,// 159 PAY 156 

    0xdace2528,// 160 PAY 157 

    0xfe4ec648,// 161 PAY 158 

    0x28efbd93,// 162 PAY 159 

    0x894d4715,// 163 PAY 160 

    0x80670c8a,// 164 PAY 161 

    0x3d359da1,// 165 PAY 162 

    0x505c2303,// 166 PAY 163 

    0xe8f15ec1,// 167 PAY 164 

    0xf63616db,// 168 PAY 165 

    0x887ca7c6,// 169 PAY 166 

    0xea0b99e8,// 170 PAY 167 

    0x4dfc9e10,// 171 PAY 168 

    0xdaa06570,// 172 PAY 169 

    0xf990b1f9,// 173 PAY 170 

    0x7f28a01f,// 174 PAY 171 

    0x9d1642a5,// 175 PAY 172 

    0x742bdd03,// 176 PAY 173 

    0x139d793d,// 177 PAY 174 

    0x89f747c9,// 178 PAY 175 

    0x0fcf5dc7,// 179 PAY 176 

    0x367d465f,// 180 PAY 177 

    0x17f0044b,// 181 PAY 178 

    0xc444f68c,// 182 PAY 179 

    0x86232840,// 183 PAY 180 

    0xe9d38b1b,// 184 PAY 181 

    0xaff53d6c,// 185 PAY 182 

    0x1fcbad8f,// 186 PAY 183 

    0x5e8ff0a6,// 187 PAY 184 

    0x3567ba28,// 188 PAY 185 

    0x8c6888e4,// 189 PAY 186 

    0x4a6ad06e,// 190 PAY 187 

    0xed2c566d,// 191 PAY 188 

    0x76834dec,// 192 PAY 189 

    0x8985f9c9,// 193 PAY 190 

    0xafe5ba33,// 194 PAY 191 

    0x4357b0a0,// 195 PAY 192 

    0xf709da7b,// 196 PAY 193 

    0x6898e65c,// 197 PAY 194 

    0xcdb4af06,// 198 PAY 195 

    0x57fcda92,// 199 PAY 196 

    0xebf21972,// 200 PAY 197 

    0x5407cb06,// 201 PAY 198 

    0x12df2c8b,// 202 PAY 199 

    0xa4fec5b5,// 203 PAY 200 

    0x10d2c3a7,// 204 PAY 201 

    0x9d5644ef,// 205 PAY 202 

    0x94db09bb,// 206 PAY 203 

    0x792b7f0b,// 207 PAY 204 

    0x58ec9e9b,// 208 PAY 205 

    0x07e58ae5,// 209 PAY 206 

    0xdeadfd22,// 210 PAY 207 

    0xdce73b86,// 211 PAY 208 

    0xf38b405f,// 212 PAY 209 

    0x696a32ee,// 213 PAY 210 

    0x9da76370,// 214 PAY 211 

    0x4d65bcbb,// 215 PAY 212 

    0xe77cde02,// 216 PAY 213 

    0xa7555f8f,// 217 PAY 214 

    0xf9fae6c4,// 218 PAY 215 

    0x3abad1ad,// 219 PAY 216 

    0xa1a30665,// 220 PAY 217 

    0x5e3c1767,// 221 PAY 218 

    0xc3cbe564,// 222 PAY 219 

    0x3cb44328,// 223 PAY 220 

    0x36bf0fde,// 224 PAY 221 

    0x2dd6b67e,// 225 PAY 222 

    0x81a38aef,// 226 PAY 223 

    0x231bc893,// 227 PAY 224 

    0x27cdba01,// 228 PAY 225 

    0x588a6e22,// 229 PAY 226 

    0x4f91c11c,// 230 PAY 227 

    0xa30d2fc4,// 231 PAY 228 

    0x65d55aba,// 232 PAY 229 

    0xe0f23984,// 233 PAY 230 

    0x9d138bf6,// 234 PAY 231 

    0xc9f872b3,// 235 PAY 232 

    0xebfaf92b,// 236 PAY 233 

    0x6c61e483,// 237 PAY 234 

    0xfb21b7aa,// 238 PAY 235 

    0xd2f5f7ce,// 239 PAY 236 

    0x0cdab480,// 240 PAY 237 

    0x14880a40,// 241 PAY 238 

    0x162e5f89,// 242 PAY 239 

    0x798ecfb2,// 243 PAY 240 

    0xcb18754f,// 244 PAY 241 

    0xf7985220,// 245 PAY 242 

    0x89f769e9,// 246 PAY 243 

    0xc0623efb,// 247 PAY 244 

    0xbde03d41,// 248 PAY 245 

    0xcf3a3dc7,// 249 PAY 246 

    0x14f9569f,// 250 PAY 247 

    0xa27b4db3,// 251 PAY 248 

    0x1ead972a,// 252 PAY 249 

    0xf9d1afc5,// 253 PAY 250 

    0xd8f8aed7,// 254 PAY 251 

    0x1828cacd,// 255 PAY 252 

    0x53329e7d,// 256 PAY 253 

    0xfe2f5354,// 257 PAY 254 

    0x420cf69e,// 258 PAY 255 

    0xc552004c,// 259 PAY 256 

    0xf47ced60,// 260 PAY 257 

    0xfc9e4d6f,// 261 PAY 258 

    0x1c90d564,// 262 PAY 259 

    0xa01c1297,// 263 PAY 260 

    0xc7ff3af8,// 264 PAY 261 

    0xbac3ecf2,// 265 PAY 262 

    0xbb37d29b,// 266 PAY 263 

    0xd3ae7bf8,// 267 PAY 264 

    0x2bb1c68a,// 268 PAY 265 

    0xcb2ab7f9,// 269 PAY 266 

    0xfb2a02c1,// 270 PAY 267 

    0xdbc3c1a1,// 271 PAY 268 

    0x7aaa02cf,// 272 PAY 269 

    0x69c0d0cb,// 273 PAY 270 

    0xf65108a3,// 274 PAY 271 

    0x95164fa5,// 275 PAY 272 

    0xef399a05,// 276 PAY 273 

    0x48fcd8cc,// 277 PAY 274 

    0xb66e8f94,// 278 PAY 275 

    0x30434aa5,// 279 PAY 276 

    0x5e9e4adc,// 280 PAY 277 

    0x92feb4c0,// 281 PAY 278 

    0x1a9004a6,// 282 PAY 279 

    0xf9f1d325,// 283 PAY 280 

    0x17a22db9,// 284 PAY 281 

    0x0a7386a2,// 285 PAY 282 

    0xc2540209,// 286 PAY 283 

    0xf6ca307a,// 287 PAY 284 

    0x0e90a0e6,// 288 PAY 285 

    0x027b61aa,// 289 PAY 286 

    0x98f6c962,// 290 PAY 287 

    0x65226f7a,// 291 PAY 288 

    0x3bb39a0c,// 292 PAY 289 

    0x5b784039,// 293 PAY 290 

    0x2ce534e5,// 294 PAY 291 

    0x46ad479c,// 295 PAY 292 

    0x54755434,// 296 PAY 293 

    0x14255b50,// 297 PAY 294 

    0xeb6eef52,// 298 PAY 295 

    0x350008ab,// 299 PAY 296 

    0x4d18142c,// 300 PAY 297 

    0xd6728619,// 301 PAY 298 

    0x8e619ee4,// 302 PAY 299 

    0x47a0b7d7,// 303 PAY 300 

    0x0b8573b2,// 304 PAY 301 

    0xe6ae1253,// 305 PAY 302 

    0x11d38b28,// 306 PAY 303 

    0x870e03f6,// 307 PAY 304 

    0x0529f0e3,// 308 PAY 305 

    0xb7b13e00,// 309 PAY 306 

/// HASH is  12 bytes 

    0x86232840,// 310 HSH   1 

    0xe9d38b1b,// 311 HSH   2 

    0xaff53d6c,// 312 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 132 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x26 

    0x01f12684 // 313 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt15_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 417 words. 

/// BDA size     is 1662 (0x67e) 

/// BDA id       is 0xa24 

    0x067e0a24,// 3 BDA   1 

/// PAY Generic Data size   : 1662 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x3c66e4be,// 4 PAY   1 

    0xe10b142e,// 5 PAY   2 

    0x46d83752,// 6 PAY   3 

    0x9c4f050d,// 7 PAY   4 

    0xd6e31735,// 8 PAY   5 

    0xe3e210eb,// 9 PAY   6 

    0x5f63eb5e,// 10 PAY   7 

    0x4832241a,// 11 PAY   8 

    0x9391fb26,// 12 PAY   9 

    0x9ab6f13b,// 13 PAY  10 

    0x3ca4fc48,// 14 PAY  11 

    0x3e17bc71,// 15 PAY  12 

    0xb6aae146,// 16 PAY  13 

    0x0a7eaa1d,// 17 PAY  14 

    0x979273ec,// 18 PAY  15 

    0x58ac19a8,// 19 PAY  16 

    0x9c0e6b4e,// 20 PAY  17 

    0x1e5039d3,// 21 PAY  18 

    0x9973e208,// 22 PAY  19 

    0xf3129683,// 23 PAY  20 

    0x188c6f88,// 24 PAY  21 

    0xded0d901,// 25 PAY  22 

    0x8ee9e5bb,// 26 PAY  23 

    0xd3d0840a,// 27 PAY  24 

    0xaf9c4b00,// 28 PAY  25 

    0xe62ea681,// 29 PAY  26 

    0xa5424d69,// 30 PAY  27 

    0xbd88e3e3,// 31 PAY  28 

    0x034e7a9b,// 32 PAY  29 

    0x06f517bb,// 33 PAY  30 

    0x2a0920a8,// 34 PAY  31 

    0xb5eb6b03,// 35 PAY  32 

    0x53b2b36b,// 36 PAY  33 

    0xe11c35cd,// 37 PAY  34 

    0x81ab9427,// 38 PAY  35 

    0x4cfbe5f1,// 39 PAY  36 

    0x4ef5bbbc,// 40 PAY  37 

    0xba6dbd64,// 41 PAY  38 

    0xbccb1e2e,// 42 PAY  39 

    0x3138a034,// 43 PAY  40 

    0x6644d2d0,// 44 PAY  41 

    0x89654473,// 45 PAY  42 

    0x79cbd30d,// 46 PAY  43 

    0x15e32c8c,// 47 PAY  44 

    0xff8f48bf,// 48 PAY  45 

    0x6d73538c,// 49 PAY  46 

    0x41671a2e,// 50 PAY  47 

    0xa4d7262c,// 51 PAY  48 

    0x299c361c,// 52 PAY  49 

    0xcf8ee44a,// 53 PAY  50 

    0xe1f0718b,// 54 PAY  51 

    0x4c0fbb15,// 55 PAY  52 

    0xb0ec5e28,// 56 PAY  53 

    0x91ee192f,// 57 PAY  54 

    0x4317fe4e,// 58 PAY  55 

    0x3af24aea,// 59 PAY  56 

    0xc0218034,// 60 PAY  57 

    0xb89e3498,// 61 PAY  58 

    0x0b59cb20,// 62 PAY  59 

    0xa47306c2,// 63 PAY  60 

    0xa34d8471,// 64 PAY  61 

    0x8985c317,// 65 PAY  62 

    0x9ade2522,// 66 PAY  63 

    0x73d90b7d,// 67 PAY  64 

    0x8562b706,// 68 PAY  65 

    0x891d935d,// 69 PAY  66 

    0xd90169ed,// 70 PAY  67 

    0xf3dc8f77,// 71 PAY  68 

    0xc00eb1df,// 72 PAY  69 

    0xd6167769,// 73 PAY  70 

    0x4a80f055,// 74 PAY  71 

    0x69f4a0aa,// 75 PAY  72 

    0x21dafb33,// 76 PAY  73 

    0xc0e86492,// 77 PAY  74 

    0x9e172f9a,// 78 PAY  75 

    0x7a860d77,// 79 PAY  76 

    0xba15523e,// 80 PAY  77 

    0x5ab63413,// 81 PAY  78 

    0x1e4ad7a0,// 82 PAY  79 

    0x676884d8,// 83 PAY  80 

    0x42a661a6,// 84 PAY  81 

    0xd424197f,// 85 PAY  82 

    0xe65e67a5,// 86 PAY  83 

    0xd2935102,// 87 PAY  84 

    0xfb872641,// 88 PAY  85 

    0xc8c23594,// 89 PAY  86 

    0x851daa1f,// 90 PAY  87 

    0x75808b8d,// 91 PAY  88 

    0x6a203307,// 92 PAY  89 

    0x09e27ec6,// 93 PAY  90 

    0x6444aec5,// 94 PAY  91 

    0xc08ceb94,// 95 PAY  92 

    0xe2120c3d,// 96 PAY  93 

    0x4c5c1b62,// 97 PAY  94 

    0xa0976123,// 98 PAY  95 

    0x1e36d279,// 99 PAY  96 

    0x2b132926,// 100 PAY  97 

    0x9a5a958e,// 101 PAY  98 

    0xc994c466,// 102 PAY  99 

    0x79909822,// 103 PAY 100 

    0x6a60e9ed,// 104 PAY 101 

    0x4c491282,// 105 PAY 102 

    0xdb3f9e99,// 106 PAY 103 

    0x54edc82c,// 107 PAY 104 

    0x4ebaac16,// 108 PAY 105 

    0xcc85d225,// 109 PAY 106 

    0xd49ba18d,// 110 PAY 107 

    0xd500a44a,// 111 PAY 108 

    0x1fc7b762,// 112 PAY 109 

    0x2fe46644,// 113 PAY 110 

    0x188b817d,// 114 PAY 111 

    0xc966db06,// 115 PAY 112 

    0xc600e5d2,// 116 PAY 113 

    0xa32dc3e9,// 117 PAY 114 

    0xae638ef5,// 118 PAY 115 

    0x4ac4e7f9,// 119 PAY 116 

    0x74a67a46,// 120 PAY 117 

    0x44664bd4,// 121 PAY 118 

    0x160a328d,// 122 PAY 119 

    0xe852d851,// 123 PAY 120 

    0x21ca2df5,// 124 PAY 121 

    0x073992ad,// 125 PAY 122 

    0xabf45422,// 126 PAY 123 

    0x60272d7a,// 127 PAY 124 

    0x38c680dc,// 128 PAY 125 

    0xf50bbb87,// 129 PAY 126 

    0x14de4e2d,// 130 PAY 127 

    0xa7545acf,// 131 PAY 128 

    0x03b5678a,// 132 PAY 129 

    0x06490826,// 133 PAY 130 

    0x82727016,// 134 PAY 131 

    0x4c02981a,// 135 PAY 132 

    0x5e5b594f,// 136 PAY 133 

    0x66f5d7ea,// 137 PAY 134 

    0x5165a38f,// 138 PAY 135 

    0x4f974266,// 139 PAY 136 

    0x1336428f,// 140 PAY 137 

    0x2cd64f23,// 141 PAY 138 

    0xb7dd3008,// 142 PAY 139 

    0x4e7add85,// 143 PAY 140 

    0xc86afb6e,// 144 PAY 141 

    0x7ec8bb52,// 145 PAY 142 

    0xc2e43ef0,// 146 PAY 143 

    0xc338d83f,// 147 PAY 144 

    0x03522299,// 148 PAY 145 

    0x96c470b5,// 149 PAY 146 

    0x71a4dcd2,// 150 PAY 147 

    0xcf0ed51a,// 151 PAY 148 

    0x5901bbf8,// 152 PAY 149 

    0xbc507f0c,// 153 PAY 150 

    0xd6bb5e85,// 154 PAY 151 

    0xa35b032d,// 155 PAY 152 

    0xd1fd2599,// 156 PAY 153 

    0x0961a0b4,// 157 PAY 154 

    0x8781a3e8,// 158 PAY 155 

    0x885ef828,// 159 PAY 156 

    0x9dec9650,// 160 PAY 157 

    0xb538e7a7,// 161 PAY 158 

    0x2fd0fadb,// 162 PAY 159 

    0x152ebe69,// 163 PAY 160 

    0x4d871d18,// 164 PAY 161 

    0x8dd99a99,// 165 PAY 162 

    0x14e1117e,// 166 PAY 163 

    0xf496233b,// 167 PAY 164 

    0x9c3cae31,// 168 PAY 165 

    0x7eef3dc9,// 169 PAY 166 

    0xc92ba0d8,// 170 PAY 167 

    0x3c48e908,// 171 PAY 168 

    0xc7c34d88,// 172 PAY 169 

    0x425eeba3,// 173 PAY 170 

    0xa0772a4e,// 174 PAY 171 

    0x54925fbf,// 175 PAY 172 

    0x28ba5444,// 176 PAY 173 

    0x822409e1,// 177 PAY 174 

    0x74554429,// 178 PAY 175 

    0x91891957,// 179 PAY 176 

    0x5ca7a8bc,// 180 PAY 177 

    0x1dd443f8,// 181 PAY 178 

    0x96c51073,// 182 PAY 179 

    0xee4c78bb,// 183 PAY 180 

    0x2f3cbebc,// 184 PAY 181 

    0x2e5acc04,// 185 PAY 182 

    0x6ae178e6,// 186 PAY 183 

    0x3a3df664,// 187 PAY 184 

    0xe13d0c59,// 188 PAY 185 

    0xd8b2018e,// 189 PAY 186 

    0x0cfde799,// 190 PAY 187 

    0xdbbe7d21,// 191 PAY 188 

    0x7a203619,// 192 PAY 189 

    0xda86d627,// 193 PAY 190 

    0xeb9efd81,// 194 PAY 191 

    0x013a66a1,// 195 PAY 192 

    0xe46499ef,// 196 PAY 193 

    0xac222d56,// 197 PAY 194 

    0x9df40cbc,// 198 PAY 195 

    0xe186b2b8,// 199 PAY 196 

    0x63c0e84a,// 200 PAY 197 

    0x5912af9d,// 201 PAY 198 

    0x41ffc26e,// 202 PAY 199 

    0x1a0d4266,// 203 PAY 200 

    0x58a17a2b,// 204 PAY 201 

    0xe684d98c,// 205 PAY 202 

    0x9aa7b72b,// 206 PAY 203 

    0x5025fa47,// 207 PAY 204 

    0x81b09dc6,// 208 PAY 205 

    0x1ad82d7f,// 209 PAY 206 

    0xc9fce28f,// 210 PAY 207 

    0xe15ef6f9,// 211 PAY 208 

    0xa70e5f1d,// 212 PAY 209 

    0xc8e56611,// 213 PAY 210 

    0x6c35ba86,// 214 PAY 211 

    0x4d426555,// 215 PAY 212 

    0x3dee1173,// 216 PAY 213 

    0xd07e44b9,// 217 PAY 214 

    0xdae3672f,// 218 PAY 215 

    0x72a02c39,// 219 PAY 216 

    0x9fd95b4e,// 220 PAY 217 

    0xbd203c75,// 221 PAY 218 

    0x4b79e5f3,// 222 PAY 219 

    0xd27e93a9,// 223 PAY 220 

    0x81832c4b,// 224 PAY 221 

    0xe2ba0e4f,// 225 PAY 222 

    0x380f6eaa,// 226 PAY 223 

    0xcadfdec5,// 227 PAY 224 

    0x7d6aecfd,// 228 PAY 225 

    0x2f7e4a47,// 229 PAY 226 

    0x12d823f9,// 230 PAY 227 

    0x880b6f3c,// 231 PAY 228 

    0x0313aaab,// 232 PAY 229 

    0xe1b49c9a,// 233 PAY 230 

    0x67659b6e,// 234 PAY 231 

    0xa08e082c,// 235 PAY 232 

    0x713cb6ca,// 236 PAY 233 

    0x2859adc4,// 237 PAY 234 

    0x1232832c,// 238 PAY 235 

    0x888464d3,// 239 PAY 236 

    0xdadcb1f6,// 240 PAY 237 

    0xc0e4fc13,// 241 PAY 238 

    0x0bb385dd,// 242 PAY 239 

    0x1cf77281,// 243 PAY 240 

    0x1996b53c,// 244 PAY 241 

    0x8f85f32c,// 245 PAY 242 

    0x302802b5,// 246 PAY 243 

    0xbb419e9f,// 247 PAY 244 

    0x249eeb07,// 248 PAY 245 

    0x64ba7f22,// 249 PAY 246 

    0xb5efb080,// 250 PAY 247 

    0x114304a7,// 251 PAY 248 

    0xf6dcafeb,// 252 PAY 249 

    0x6f983696,// 253 PAY 250 

    0xe3b1cb50,// 254 PAY 251 

    0xe85f56f0,// 255 PAY 252 

    0x6b2a226b,// 256 PAY 253 

    0x8691a5c7,// 257 PAY 254 

    0xb7314df5,// 258 PAY 255 

    0xb22045ac,// 259 PAY 256 

    0x14082c1a,// 260 PAY 257 

    0x5458e446,// 261 PAY 258 

    0xcda752c7,// 262 PAY 259 

    0x62b03c64,// 263 PAY 260 

    0x2779cba7,// 264 PAY 261 

    0x94403a70,// 265 PAY 262 

    0x4336c8b6,// 266 PAY 263 

    0xc901d9c2,// 267 PAY 264 

    0x1c3c0857,// 268 PAY 265 

    0xec42c629,// 269 PAY 266 

    0xc9b1494c,// 270 PAY 267 

    0x58d427a3,// 271 PAY 268 

    0xb74359ce,// 272 PAY 269 

    0xf63d8dea,// 273 PAY 270 

    0xcbf75dfa,// 274 PAY 271 

    0xd799c2df,// 275 PAY 272 

    0xb7dee07c,// 276 PAY 273 

    0x65c7c80c,// 277 PAY 274 

    0xd6e713ae,// 278 PAY 275 

    0x3ad528c2,// 279 PAY 276 

    0xc728eee0,// 280 PAY 277 

    0x43286501,// 281 PAY 278 

    0xc1cec45c,// 282 PAY 279 

    0xb21f3758,// 283 PAY 280 

    0x7be78208,// 284 PAY 281 

    0xd06f0a8e,// 285 PAY 282 

    0xab3f4701,// 286 PAY 283 

    0xe40761cd,// 287 PAY 284 

    0x5faf9cb1,// 288 PAY 285 

    0x4d835cc7,// 289 PAY 286 

    0x243f471c,// 290 PAY 287 

    0x2a3719d8,// 291 PAY 288 

    0x2f903d5f,// 292 PAY 289 

    0x1b927f0b,// 293 PAY 290 

    0x19522491,// 294 PAY 291 

    0x7db843d3,// 295 PAY 292 

    0xe209286a,// 296 PAY 293 

    0x23431f95,// 297 PAY 294 

    0xf9d8a4e0,// 298 PAY 295 

    0x8fe36d9d,// 299 PAY 296 

    0xf7446f72,// 300 PAY 297 

    0xc9fe2731,// 301 PAY 298 

    0xd023ae23,// 302 PAY 299 

    0xb2a11d92,// 303 PAY 300 

    0x846f0e84,// 304 PAY 301 

    0xf3e6e47c,// 305 PAY 302 

    0xb13b6f5e,// 306 PAY 303 

    0xf6344083,// 307 PAY 304 

    0x6755ca08,// 308 PAY 305 

    0x10b2312f,// 309 PAY 306 

    0x2e6fa8ae,// 310 PAY 307 

    0xd427529a,// 311 PAY 308 

    0xf83e0f0e,// 312 PAY 309 

    0x69d29a24,// 313 PAY 310 

    0x655cbdd7,// 314 PAY 311 

    0x3095ee69,// 315 PAY 312 

    0x48a392a7,// 316 PAY 313 

    0xcabaac65,// 317 PAY 314 

    0x64c343b8,// 318 PAY 315 

    0xfacf2976,// 319 PAY 316 

    0x0d4ee041,// 320 PAY 317 

    0xe59ffbe1,// 321 PAY 318 

    0x8d750442,// 322 PAY 319 

    0xe7c65a76,// 323 PAY 320 

    0xec71bcc3,// 324 PAY 321 

    0x6ebef965,// 325 PAY 322 

    0xcaf6d8f5,// 326 PAY 323 

    0xc3625f25,// 327 PAY 324 

    0x15c8c488,// 328 PAY 325 

    0xe90b662b,// 329 PAY 326 

    0x8ab4dfca,// 330 PAY 327 

    0x3cec9618,// 331 PAY 328 

    0x3f6f0af7,// 332 PAY 329 

    0x8fec12be,// 333 PAY 330 

    0xccc8bb0b,// 334 PAY 331 

    0x404ce64d,// 335 PAY 332 

    0x2b037e12,// 336 PAY 333 

    0x254ab6f6,// 337 PAY 334 

    0x46ea25e6,// 338 PAY 335 

    0xad64eb55,// 339 PAY 336 

    0x4a74e9fb,// 340 PAY 337 

    0x19216ce3,// 341 PAY 338 

    0x2a59280c,// 342 PAY 339 

    0x87a4d5b5,// 343 PAY 340 

    0x0c8602db,// 344 PAY 341 

    0x43f7a5b9,// 345 PAY 342 

    0xb1726816,// 346 PAY 343 

    0x4b8a5020,// 347 PAY 344 

    0x3ff75de2,// 348 PAY 345 

    0xd8c83726,// 349 PAY 346 

    0x49dda4c2,// 350 PAY 347 

    0x83e27fcf,// 351 PAY 348 

    0x2800dd1a,// 352 PAY 349 

    0x92ddf78b,// 353 PAY 350 

    0x0a944ca4,// 354 PAY 351 

    0x583db4ce,// 355 PAY 352 

    0xf61331b7,// 356 PAY 353 

    0x86fe966e,// 357 PAY 354 

    0x9cfc09a0,// 358 PAY 355 

    0xb05c4aa8,// 359 PAY 356 

    0x3d6ef679,// 360 PAY 357 

    0xf0ff747e,// 361 PAY 358 

    0xd886d34b,// 362 PAY 359 

    0x099906ac,// 363 PAY 360 

    0x979a2504,// 364 PAY 361 

    0x018419ca,// 365 PAY 362 

    0x5f0313be,// 366 PAY 363 

    0x60fa39ed,// 367 PAY 364 

    0x468654f0,// 368 PAY 365 

    0xd41ccf1a,// 369 PAY 366 

    0x275fc545,// 370 PAY 367 

    0xdaac7016,// 371 PAY 368 

    0x6543429d,// 372 PAY 369 

    0x527ab93a,// 373 PAY 370 

    0x841f867a,// 374 PAY 371 

    0x6ace82e9,// 375 PAY 372 

    0x71c97049,// 376 PAY 373 

    0xc2308aa1,// 377 PAY 374 

    0xdef2a769,// 378 PAY 375 

    0xf768e050,// 379 PAY 376 

    0x89b08e13,// 380 PAY 377 

    0x07897d61,// 381 PAY 378 

    0xf0d90582,// 382 PAY 379 

    0x09a8959c,// 383 PAY 380 

    0x48e6dcbe,// 384 PAY 381 

    0xf4bd2a4b,// 385 PAY 382 

    0xec882cbf,// 386 PAY 383 

    0x330711d7,// 387 PAY 384 

    0x93a29386,// 388 PAY 385 

    0x34944943,// 389 PAY 386 

    0xd3eb792d,// 390 PAY 387 

    0xec64e611,// 391 PAY 388 

    0x8fd856e7,// 392 PAY 389 

    0x6c0f280e,// 393 PAY 390 

    0x62e04de8,// 394 PAY 391 

    0x938b8971,// 395 PAY 392 

    0x1d973874,// 396 PAY 393 

    0x3e678ff8,// 397 PAY 394 

    0xd102c596,// 398 PAY 395 

    0xc34a82ed,// 399 PAY 396 

    0xf518cbb3,// 400 PAY 397 

    0xd7d01058,// 401 PAY 398 

    0x83f958c8,// 402 PAY 399 

    0x378599b6,// 403 PAY 400 

    0x4b3dd9f0,// 404 PAY 401 

    0x37a5719c,// 405 PAY 402 

    0x68f29a39,// 406 PAY 403 

    0xbbf1a8ce,// 407 PAY 404 

    0x61cfe5a9,// 408 PAY 405 

    0xb40cfffa,// 409 PAY 406 

    0xbba63383,// 410 PAY 407 

    0x10ed55c2,// 411 PAY 408 

    0xc19be824,// 412 PAY 409 

    0x9b45cf8a,// 413 PAY 410 

    0xce98d0b3,// 414 PAY 411 

    0x35a39f85,// 415 PAY 412 

    0x8b11e32e,// 416 PAY 413 

    0x680f7a61,// 417 PAY 414 

    0xd9520a00,// 418 PAY 415 

    0xba510000,// 419 PAY 416 

/// STA is 1 words. 

/// STA num_pkts       : 92 

/// STA pkt_idx        : 100 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xde 

    0x0191de5c // 420 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt16_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 384 words. 

/// BDA size     is 1531 (0x5fb) 

/// BDA id       is 0x8a3d 

    0x05fb8a3d,// 3 BDA   1 

/// PAY Generic Data size   : 1531 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf617e73c,// 4 PAY   1 

    0x1ba74023,// 5 PAY   2 

    0x5b22a1eb,// 6 PAY   3 

    0x5183e98b,// 7 PAY   4 

    0x8f5686db,// 8 PAY   5 

    0x59bcc42a,// 9 PAY   6 

    0x688d52e0,// 10 PAY   7 

    0x3012599b,// 11 PAY   8 

    0xad64884a,// 12 PAY   9 

    0xca85f6a2,// 13 PAY  10 

    0x83734012,// 14 PAY  11 

    0x52931309,// 15 PAY  12 

    0x9cb8e00d,// 16 PAY  13 

    0xc8330a9d,// 17 PAY  14 

    0x41381a13,// 18 PAY  15 

    0x5831e3a2,// 19 PAY  16 

    0x2e867799,// 20 PAY  17 

    0x20b7a915,// 21 PAY  18 

    0x7d0466f5,// 22 PAY  19 

    0x0141c46b,// 23 PAY  20 

    0x52b3c909,// 24 PAY  21 

    0xc74048ac,// 25 PAY  22 

    0x4a2b06f7,// 26 PAY  23 

    0xfc596634,// 27 PAY  24 

    0x60bfd96d,// 28 PAY  25 

    0x973825c1,// 29 PAY  26 

    0xb2197a6b,// 30 PAY  27 

    0x4c07ed61,// 31 PAY  28 

    0x60ac7204,// 32 PAY  29 

    0x8e414ad5,// 33 PAY  30 

    0x89a29eea,// 34 PAY  31 

    0x4cfc098f,// 35 PAY  32 

    0x459e3445,// 36 PAY  33 

    0xaf5d9944,// 37 PAY  34 

    0x043de07f,// 38 PAY  35 

    0x07e13ea3,// 39 PAY  36 

    0x40de6cce,// 40 PAY  37 

    0x3816a8c0,// 41 PAY  38 

    0x5fdd5658,// 42 PAY  39 

    0x88814f31,// 43 PAY  40 

    0x603f5b0d,// 44 PAY  41 

    0xed6a97b8,// 45 PAY  42 

    0x793d2c4d,// 46 PAY  43 

    0x75672a82,// 47 PAY  44 

    0xba1c2c50,// 48 PAY  45 

    0xb503b137,// 49 PAY  46 

    0x66bb7486,// 50 PAY  47 

    0x521a8884,// 51 PAY  48 

    0xa0611a7d,// 52 PAY  49 

    0x5c7060f5,// 53 PAY  50 

    0xf9e05076,// 54 PAY  51 

    0xfa5c19ee,// 55 PAY  52 

    0x123fe2ff,// 56 PAY  53 

    0x59f34ff1,// 57 PAY  54 

    0x7e408c9e,// 58 PAY  55 

    0x2ea7d032,// 59 PAY  56 

    0x90e09599,// 60 PAY  57 

    0x04dc99d8,// 61 PAY  58 

    0x304f681c,// 62 PAY  59 

    0x9fcbe92e,// 63 PAY  60 

    0x8f4f0dd5,// 64 PAY  61 

    0x636ddf99,// 65 PAY  62 

    0xe13e59ff,// 66 PAY  63 

    0x730c666c,// 67 PAY  64 

    0xa776b700,// 68 PAY  65 

    0x94970b5b,// 69 PAY  66 

    0x0b12376b,// 70 PAY  67 

    0x0fd7ec10,// 71 PAY  68 

    0x56e8b793,// 72 PAY  69 

    0xb3cb9a2a,// 73 PAY  70 

    0x5827c04d,// 74 PAY  71 

    0xc603be4b,// 75 PAY  72 

    0xec55653b,// 76 PAY  73 

    0xf2a65bd0,// 77 PAY  74 

    0x218444fb,// 78 PAY  75 

    0x5f4b28b6,// 79 PAY  76 

    0x591da092,// 80 PAY  77 

    0x2ef26275,// 81 PAY  78 

    0xe9f0acca,// 82 PAY  79 

    0x6f0a1c4f,// 83 PAY  80 

    0x6ea0d3de,// 84 PAY  81 

    0xe66eda49,// 85 PAY  82 

    0xf9b79eeb,// 86 PAY  83 

    0xae19bf8f,// 87 PAY  84 

    0xcb7ec243,// 88 PAY  85 

    0x9d972237,// 89 PAY  86 

    0xe585145c,// 90 PAY  87 

    0x73e96d8b,// 91 PAY  88 

    0x98e4c85e,// 92 PAY  89 

    0x9502eb85,// 93 PAY  90 

    0x11c5e5b2,// 94 PAY  91 

    0xcda94419,// 95 PAY  92 

    0xeb87ccb4,// 96 PAY  93 

    0x735207a4,// 97 PAY  94 

    0x0142a3f1,// 98 PAY  95 

    0x12ae01bb,// 99 PAY  96 

    0x762c9a20,// 100 PAY  97 

    0xd6ed756f,// 101 PAY  98 

    0xe95012d7,// 102 PAY  99 

    0xc788ab2f,// 103 PAY 100 

    0x4cd4ca07,// 104 PAY 101 

    0x0796f52f,// 105 PAY 102 

    0xe8596103,// 106 PAY 103 

    0xd7b2ee6c,// 107 PAY 104 

    0xd567e5a0,// 108 PAY 105 

    0xb985406a,// 109 PAY 106 

    0xb4b7275d,// 110 PAY 107 

    0x883d76fe,// 111 PAY 108 

    0x2143e0b4,// 112 PAY 109 

    0xe71717e3,// 113 PAY 110 

    0x71992a77,// 114 PAY 111 

    0x58c8bff9,// 115 PAY 112 

    0x3437c4a5,// 116 PAY 113 

    0x7726812f,// 117 PAY 114 

    0x9ac84ce2,// 118 PAY 115 

    0x9a9c70bc,// 119 PAY 116 

    0x0e4488ba,// 120 PAY 117 

    0xa7df9304,// 121 PAY 118 

    0xa520a211,// 122 PAY 119 

    0x2a558082,// 123 PAY 120 

    0x36b1f8a5,// 124 PAY 121 

    0x861cbd7f,// 125 PAY 122 

    0xf7071ae6,// 126 PAY 123 

    0x78da234a,// 127 PAY 124 

    0xce202278,// 128 PAY 125 

    0x9b736fa9,// 129 PAY 126 

    0x2f8e3647,// 130 PAY 127 

    0x5fe2a3a7,// 131 PAY 128 

    0x9e9adbbf,// 132 PAY 129 

    0xc31626de,// 133 PAY 130 

    0x70a48080,// 134 PAY 131 

    0x5e2dca66,// 135 PAY 132 

    0x80934dc4,// 136 PAY 133 

    0x6d92eaec,// 137 PAY 134 

    0x8e351bcf,// 138 PAY 135 

    0xb34d68f9,// 139 PAY 136 

    0xaa95fca2,// 140 PAY 137 

    0xdf2e2e9d,// 141 PAY 138 

    0x192e92ca,// 142 PAY 139 

    0xe0d908dd,// 143 PAY 140 

    0xca430ac7,// 144 PAY 141 

    0x30d33324,// 145 PAY 142 

    0x44852822,// 146 PAY 143 

    0x62f4051d,// 147 PAY 144 

    0x723e0872,// 148 PAY 145 

    0xc4faf0d4,// 149 PAY 146 

    0x9f677260,// 150 PAY 147 

    0x62eca482,// 151 PAY 148 

    0xa847aef0,// 152 PAY 149 

    0x3ad882a5,// 153 PAY 150 

    0x9e5f5f9c,// 154 PAY 151 

    0x8b95f3d1,// 155 PAY 152 

    0xa1fce808,// 156 PAY 153 

    0xa8934f6e,// 157 PAY 154 

    0x0ddea223,// 158 PAY 155 

    0x0065f5f8,// 159 PAY 156 

    0xd8411fbe,// 160 PAY 157 

    0x28d51493,// 161 PAY 158 

    0x3d61007c,// 162 PAY 159 

    0x9efa4373,// 163 PAY 160 

    0xdc671253,// 164 PAY 161 

    0x1b2e1ab0,// 165 PAY 162 

    0x91e342e0,// 166 PAY 163 

    0x3aba8c37,// 167 PAY 164 

    0x13418c52,// 168 PAY 165 

    0xd4d60898,// 169 PAY 166 

    0xd18182a6,// 170 PAY 167 

    0x7c6c78a4,// 171 PAY 168 

    0x9465bf36,// 172 PAY 169 

    0xff48aefd,// 173 PAY 170 

    0xfd2146ed,// 174 PAY 171 

    0xdbc4e65f,// 175 PAY 172 

    0xb0bcff69,// 176 PAY 173 

    0x2e268788,// 177 PAY 174 

    0xabeed82f,// 178 PAY 175 

    0x2f1a551e,// 179 PAY 176 

    0x4dabb84e,// 180 PAY 177 

    0xb7d0d11b,// 181 PAY 178 

    0x3dc5c57d,// 182 PAY 179 

    0x96080e3c,// 183 PAY 180 

    0x74cbc46f,// 184 PAY 181 

    0xbb485385,// 185 PAY 182 

    0xe81a8f3b,// 186 PAY 183 

    0xd22fe9f5,// 187 PAY 184 

    0xceaa7cbf,// 188 PAY 185 

    0x1632efcc,// 189 PAY 186 

    0x51864b27,// 190 PAY 187 

    0x4bbb4b80,// 191 PAY 188 

    0x8cee77e7,// 192 PAY 189 

    0x8cc016f3,// 193 PAY 190 

    0x31ccacae,// 194 PAY 191 

    0xac38845d,// 195 PAY 192 

    0x1e50e608,// 196 PAY 193 

    0xa75daef0,// 197 PAY 194 

    0x7bc79b78,// 198 PAY 195 

    0x8083601c,// 199 PAY 196 

    0x74b8394a,// 200 PAY 197 

    0x90e440ed,// 201 PAY 198 

    0x5a8772bc,// 202 PAY 199 

    0x536dbf30,// 203 PAY 200 

    0x6aa08a31,// 204 PAY 201 

    0x600f5bc0,// 205 PAY 202 

    0xbd69882e,// 206 PAY 203 

    0xcf8bc893,// 207 PAY 204 

    0x0a62e73e,// 208 PAY 205 

    0x7c51462a,// 209 PAY 206 

    0x73ab0cf5,// 210 PAY 207 

    0x522a6ecf,// 211 PAY 208 

    0xb912f556,// 212 PAY 209 

    0xbeac222c,// 213 PAY 210 

    0xb369ff16,// 214 PAY 211 

    0x04f1a7e8,// 215 PAY 212 

    0x7b213220,// 216 PAY 213 

    0xea17c627,// 217 PAY 214 

    0x17e44ff8,// 218 PAY 215 

    0x78ef1275,// 219 PAY 216 

    0xb11e5025,// 220 PAY 217 

    0xefaaa37c,// 221 PAY 218 

    0xf9ea90ec,// 222 PAY 219 

    0x4c6b9632,// 223 PAY 220 

    0xc5724688,// 224 PAY 221 

    0x546db111,// 225 PAY 222 

    0xa0afb09e,// 226 PAY 223 

    0x15adcc38,// 227 PAY 224 

    0x780da305,// 228 PAY 225 

    0x5a3ca71a,// 229 PAY 226 

    0x0ee810b7,// 230 PAY 227 

    0x13386a26,// 231 PAY 228 

    0xbb3c0ece,// 232 PAY 229 

    0x6c9091a6,// 233 PAY 230 

    0xf2636138,// 234 PAY 231 

    0x212fe91a,// 235 PAY 232 

    0xa4fdf663,// 236 PAY 233 

    0xfc5e5f1d,// 237 PAY 234 

    0x545d2c06,// 238 PAY 235 

    0xd337a22f,// 239 PAY 236 

    0x2e384b1e,// 240 PAY 237 

    0x101aa229,// 241 PAY 238 

    0xaaf62906,// 242 PAY 239 

    0x505567fe,// 243 PAY 240 

    0x1cab1e73,// 244 PAY 241 

    0xb8a883c3,// 245 PAY 242 

    0xb379ccc8,// 246 PAY 243 

    0x8bbc568c,// 247 PAY 244 

    0x14a592b1,// 248 PAY 245 

    0x1151c4f5,// 249 PAY 246 

    0xec86d0a4,// 250 PAY 247 

    0xe31a2eaa,// 251 PAY 248 

    0xe4769455,// 252 PAY 249 

    0xf01d083a,// 253 PAY 250 

    0xa7f3e069,// 254 PAY 251 

    0xe1284f5e,// 255 PAY 252 

    0xa0061f86,// 256 PAY 253 

    0x4fcf60dd,// 257 PAY 254 

    0x40696bc7,// 258 PAY 255 

    0x71ce182c,// 259 PAY 256 

    0x8a14d1da,// 260 PAY 257 

    0xcb79b2c6,// 261 PAY 258 

    0xf9695bbc,// 262 PAY 259 

    0xda092272,// 263 PAY 260 

    0x382f6db7,// 264 PAY 261 

    0x58a83c6d,// 265 PAY 262 

    0x6141f5eb,// 266 PAY 263 

    0x9c130718,// 267 PAY 264 

    0x24cdbae5,// 268 PAY 265 

    0x4ebf7281,// 269 PAY 266 

    0xb5e22747,// 270 PAY 267 

    0x92edc4b5,// 271 PAY 268 

    0xc19a4bbd,// 272 PAY 269 

    0x2361b47f,// 273 PAY 270 

    0xc7169c07,// 274 PAY 271 

    0x69361fbe,// 275 PAY 272 

    0xbd79a90b,// 276 PAY 273 

    0x0e841e51,// 277 PAY 274 

    0xdb66476a,// 278 PAY 275 

    0xdf2786df,// 279 PAY 276 

    0x435f719c,// 280 PAY 277 

    0x2938f58f,// 281 PAY 278 

    0x4d6603c2,// 282 PAY 279 

    0x29493f2b,// 283 PAY 280 

    0x6ec3258a,// 284 PAY 281 

    0xb9ab7588,// 285 PAY 282 

    0xe9bf9713,// 286 PAY 283 

    0x49cbd518,// 287 PAY 284 

    0xb50a1709,// 288 PAY 285 

    0x90b1cb0a,// 289 PAY 286 

    0x9b73bbd1,// 290 PAY 287 

    0x57ceee80,// 291 PAY 288 

    0xc62df69b,// 292 PAY 289 

    0x4fbfb668,// 293 PAY 290 

    0x8db814f7,// 294 PAY 291 

    0x3122cce9,// 295 PAY 292 

    0x5ba7d9a3,// 296 PAY 293 

    0x21b99f87,// 297 PAY 294 

    0x7cf3f8ed,// 298 PAY 295 

    0xbee8228b,// 299 PAY 296 

    0x5303aa81,// 300 PAY 297 

    0xc0feb368,// 301 PAY 298 

    0x922dd8a4,// 302 PAY 299 

    0xbd86e794,// 303 PAY 300 

    0xd5b7b8a8,// 304 PAY 301 

    0x13bcef17,// 305 PAY 302 

    0x80b5e900,// 306 PAY 303 

    0x886a212d,// 307 PAY 304 

    0x31b51011,// 308 PAY 305 

    0xeb6546a1,// 309 PAY 306 

    0xdebd9cc2,// 310 PAY 307 

    0x0f91759c,// 311 PAY 308 

    0x5ea9e83c,// 312 PAY 309 

    0xcdddc529,// 313 PAY 310 

    0x15cc9308,// 314 PAY 311 

    0xec13847d,// 315 PAY 312 

    0xc0ced4a5,// 316 PAY 313 

    0x276b7eff,// 317 PAY 314 

    0x7d22bf96,// 318 PAY 315 

    0x5bf7c067,// 319 PAY 316 

    0x0e5d5280,// 320 PAY 317 

    0xb671ade5,// 321 PAY 318 

    0x67d7a130,// 322 PAY 319 

    0xe63f51a3,// 323 PAY 320 

    0xfd28773b,// 324 PAY 321 

    0x5f28c63d,// 325 PAY 322 

    0xe75a1aa7,// 326 PAY 323 

    0xc382f564,// 327 PAY 324 

    0x17776950,// 328 PAY 325 

    0xd00b9849,// 329 PAY 326 

    0xd94c3b7f,// 330 PAY 327 

    0x963a3224,// 331 PAY 328 

    0xa363c0f4,// 332 PAY 329 

    0x835980b4,// 333 PAY 330 

    0xbfdb0a6e,// 334 PAY 331 

    0x2271fbab,// 335 PAY 332 

    0xf4da84cf,// 336 PAY 333 

    0xfbdabd02,// 337 PAY 334 

    0xb9740215,// 338 PAY 335 

    0xaf57bf0b,// 339 PAY 336 

    0x9ec79676,// 340 PAY 337 

    0xc5642db7,// 341 PAY 338 

    0x63918551,// 342 PAY 339 

    0x8125df92,// 343 PAY 340 

    0xd815eed8,// 344 PAY 341 

    0x7add9ee2,// 345 PAY 342 

    0x47aa6194,// 346 PAY 343 

    0xac5d0be4,// 347 PAY 344 

    0xd40d3f53,// 348 PAY 345 

    0xe24a569e,// 349 PAY 346 

    0x418e493c,// 350 PAY 347 

    0x0ab6f72c,// 351 PAY 348 

    0xdf5a11c8,// 352 PAY 349 

    0xe4e0ab53,// 353 PAY 350 

    0x388a4278,// 354 PAY 351 

    0x2ccf7699,// 355 PAY 352 

    0x38ad4102,// 356 PAY 353 

    0xfcdd290a,// 357 PAY 354 

    0x874f6d46,// 358 PAY 355 

    0x7ab94b28,// 359 PAY 356 

    0xced073f3,// 360 PAY 357 

    0x6f27b6f4,// 361 PAY 358 

    0xa54a17ae,// 362 PAY 359 

    0x287350ed,// 363 PAY 360 

    0x9195a2a9,// 364 PAY 361 

    0x2a2354fb,// 365 PAY 362 

    0x0c56e25c,// 366 PAY 363 

    0xaed29bc7,// 367 PAY 364 

    0xcd026b8c,// 368 PAY 365 

    0x3eeb7911,// 369 PAY 366 

    0x0be12220,// 370 PAY 367 

    0x29fa3f47,// 371 PAY 368 

    0xcce266c4,// 372 PAY 369 

    0xe2300172,// 373 PAY 370 

    0x05bec114,// 374 PAY 371 

    0x2bcc3875,// 375 PAY 372 

    0x94d5f33b,// 376 PAY 373 

    0x09d01443,// 377 PAY 374 

    0x9f2a561c,// 378 PAY 375 

    0xffc452ed,// 379 PAY 376 

    0x032a82cb,// 380 PAY 377 

    0x44d3ba43,// 381 PAY 378 

    0x42fc8b33,// 382 PAY 379 

    0xc30366a0,// 383 PAY 380 

    0xfb09f0ed,// 384 PAY 381 

    0x431d8963,// 385 PAY 382 

    0xa3ddb600,// 386 PAY 383 

/// STA is 1 words. 

/// STA num_pkts       : 124 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc5 

    0x03dcc57c // 387 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt17_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 82 words. 

/// BDA size     is 321 (0x141) 

/// BDA id       is 0xc3ba 

    0x0141c3ba,// 3 BDA   1 

/// PAY Generic Data size   : 321 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x10e8b99d,// 4 PAY   1 

    0x53eb6e90,// 5 PAY   2 

    0x4060314c,// 6 PAY   3 

    0x6b48765a,// 7 PAY   4 

    0x2763cb98,// 8 PAY   5 

    0xb10511c7,// 9 PAY   6 

    0x7263c971,// 10 PAY   7 

    0x926184f4,// 11 PAY   8 

    0xe8842a54,// 12 PAY   9 

    0x98e7d032,// 13 PAY  10 

    0xc78d6ea3,// 14 PAY  11 

    0x728abd4c,// 15 PAY  12 

    0xda88b301,// 16 PAY  13 

    0x60171cff,// 17 PAY  14 

    0x148380df,// 18 PAY  15 

    0x93b94327,// 19 PAY  16 

    0xfa1310e9,// 20 PAY  17 

    0x97e09827,// 21 PAY  18 

    0x2f74e1dc,// 22 PAY  19 

    0x69a2dda2,// 23 PAY  20 

    0x4def17f3,// 24 PAY  21 

    0x0e7e2795,// 25 PAY  22 

    0x0709c709,// 26 PAY  23 

    0x22fe80c5,// 27 PAY  24 

    0xa8ac577a,// 28 PAY  25 

    0xb5770263,// 29 PAY  26 

    0x578183e9,// 30 PAY  27 

    0xc3644788,// 31 PAY  28 

    0x62737710,// 32 PAY  29 

    0xea8b1c70,// 33 PAY  30 

    0x797e35e2,// 34 PAY  31 

    0x455db781,// 35 PAY  32 

    0x0b6ee943,// 36 PAY  33 

    0xde098d31,// 37 PAY  34 

    0xde6fdc04,// 38 PAY  35 

    0xd49627c3,// 39 PAY  36 

    0x43ae10fc,// 40 PAY  37 

    0x307a682d,// 41 PAY  38 

    0x9813630e,// 42 PAY  39 

    0x5797bea1,// 43 PAY  40 

    0xf3548f37,// 44 PAY  41 

    0x24bbc422,// 45 PAY  42 

    0x637bdace,// 46 PAY  43 

    0x7f171781,// 47 PAY  44 

    0xf5d08ab4,// 48 PAY  45 

    0xfa043239,// 49 PAY  46 

    0x38787c03,// 50 PAY  47 

    0x82a18667,// 51 PAY  48 

    0xfa62ebb5,// 52 PAY  49 

    0xa1561f79,// 53 PAY  50 

    0x23eed3a4,// 54 PAY  51 

    0xf11e3a61,// 55 PAY  52 

    0xc0a935ad,// 56 PAY  53 

    0xdb2e19ee,// 57 PAY  54 

    0xebc88bb3,// 58 PAY  55 

    0x9bbb4c24,// 59 PAY  56 

    0x76fe69e5,// 60 PAY  57 

    0x05f7331e,// 61 PAY  58 

    0x98d40f84,// 62 PAY  59 

    0x998d7ba5,// 63 PAY  60 

    0x95805105,// 64 PAY  61 

    0xe55de240,// 65 PAY  62 

    0x6154083d,// 66 PAY  63 

    0x20064b62,// 67 PAY  64 

    0x7f7f4c63,// 68 PAY  65 

    0xa693eeac,// 69 PAY  66 

    0x102d8e09,// 70 PAY  67 

    0x78b69c4b,// 71 PAY  68 

    0xafe1aa20,// 72 PAY  69 

    0xc66fff4c,// 73 PAY  70 

    0x8ce3f5df,// 74 PAY  71 

    0x04c08767,// 75 PAY  72 

    0x3677a8c7,// 76 PAY  73 

    0xf57f39b4,// 77 PAY  74 

    0xfc490016,// 78 PAY  75 

    0x14280703,// 79 PAY  76 

    0x9c71c744,// 80 PAY  77 

    0x839c2dd5,// 81 PAY  78 

    0x94214bc0,// 82 PAY  79 

    0x0bfdc141,// 83 PAY  80 

    0x34000000,// 84 PAY  81 

/// HASH is  16 bytes 

    0x05f7331e,// 85 HSH   1 

    0x98d40f84,// 86 HSH   2 

    0x998d7ba5,// 87 HSH   3 

    0x95805105,// 88 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 137 

/// STA pkt_idx        : 109 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc0 

    0x01b5c089 // 89 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt18_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 494 words. 

/// BDA size     is 1970 (0x7b2) 

/// BDA id       is 0xdbea 

    0x07b2dbea,// 3 BDA   1 

/// PAY Generic Data size   : 1970 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x4a1f7048,// 4 PAY   1 

    0xe85a3c99,// 5 PAY   2 

    0xf2fc8d6d,// 6 PAY   3 

    0xea05f237,// 7 PAY   4 

    0x719e9572,// 8 PAY   5 

    0xb3c7db66,// 9 PAY   6 

    0x2fbfe980,// 10 PAY   7 

    0x42a44acd,// 11 PAY   8 

    0x94a89e98,// 12 PAY   9 

    0xf2b97b76,// 13 PAY  10 

    0x2bcdad43,// 14 PAY  11 

    0xe7426bc3,// 15 PAY  12 

    0x772e2bf2,// 16 PAY  13 

    0x0e12d841,// 17 PAY  14 

    0xa4690f12,// 18 PAY  15 

    0xc13d83b1,// 19 PAY  16 

    0xb096ef02,// 20 PAY  17 

    0xfb0484a4,// 21 PAY  18 

    0xe8782b14,// 22 PAY  19 

    0x118614a3,// 23 PAY  20 

    0xdd2f89ad,// 24 PAY  21 

    0x8eba930e,// 25 PAY  22 

    0x961a92c5,// 26 PAY  23 

    0x4f42578e,// 27 PAY  24 

    0xe017005f,// 28 PAY  25 

    0x3628699a,// 29 PAY  26 

    0xd368c698,// 30 PAY  27 

    0x3a788571,// 31 PAY  28 

    0xdca892d9,// 32 PAY  29 

    0x828c97ea,// 33 PAY  30 

    0xad75441f,// 34 PAY  31 

    0x454afe68,// 35 PAY  32 

    0xe8062bc3,// 36 PAY  33 

    0x7eb9044d,// 37 PAY  34 

    0x274ef03f,// 38 PAY  35 

    0x437ecaa8,// 39 PAY  36 

    0x05300ec0,// 40 PAY  37 

    0x08d5e2de,// 41 PAY  38 

    0x1f1af153,// 42 PAY  39 

    0x88df44da,// 43 PAY  40 

    0x4ceb72ff,// 44 PAY  41 

    0x0f844d6f,// 45 PAY  42 

    0xbb66b6e2,// 46 PAY  43 

    0xd5503bb1,// 47 PAY  44 

    0xf6c5319a,// 48 PAY  45 

    0x497ea22b,// 49 PAY  46 

    0x1e3b796e,// 50 PAY  47 

    0x6a2178f5,// 51 PAY  48 

    0xe7d8f259,// 52 PAY  49 

    0x0dde3f60,// 53 PAY  50 

    0x5c3012ba,// 54 PAY  51 

    0x37e8541d,// 55 PAY  52 

    0x8dea4678,// 56 PAY  53 

    0x12ebbb46,// 57 PAY  54 

    0xd039803c,// 58 PAY  55 

    0x48a51c46,// 59 PAY  56 

    0x752e4697,// 60 PAY  57 

    0x52ecde21,// 61 PAY  58 

    0x673ee5d3,// 62 PAY  59 

    0xd8208bca,// 63 PAY  60 

    0x935752a3,// 64 PAY  61 

    0xd9fbc3d9,// 65 PAY  62 

    0x6503f912,// 66 PAY  63 

    0x092a9d4b,// 67 PAY  64 

    0x676331c9,// 68 PAY  65 

    0x138e1c0a,// 69 PAY  66 

    0xbf680b03,// 70 PAY  67 

    0xcfb58f5c,// 71 PAY  68 

    0xaa489033,// 72 PAY  69 

    0x01d56012,// 73 PAY  70 

    0x5dd16939,// 74 PAY  71 

    0xa28495b6,// 75 PAY  72 

    0x4862083b,// 76 PAY  73 

    0x13fe2db5,// 77 PAY  74 

    0x907cb399,// 78 PAY  75 

    0xfddd277b,// 79 PAY  76 

    0x25d4fedf,// 80 PAY  77 

    0x624dac0c,// 81 PAY  78 

    0x915eb1be,// 82 PAY  79 

    0xc076c382,// 83 PAY  80 

    0x0550b03b,// 84 PAY  81 

    0x5a879d3a,// 85 PAY  82 

    0x22f55619,// 86 PAY  83 

    0xd22312b2,// 87 PAY  84 

    0xf48b773b,// 88 PAY  85 

    0xd2abc23b,// 89 PAY  86 

    0x377597fd,// 90 PAY  87 

    0x38303fca,// 91 PAY  88 

    0xafd3e39c,// 92 PAY  89 

    0x745a973d,// 93 PAY  90 

    0x9bb8333e,// 94 PAY  91 

    0xbe0e5045,// 95 PAY  92 

    0xdd9ce92b,// 96 PAY  93 

    0xce43b785,// 97 PAY  94 

    0x832073b3,// 98 PAY  95 

    0x6c1a9039,// 99 PAY  96 

    0x87178f52,// 100 PAY  97 

    0xba194ba1,// 101 PAY  98 

    0x72e49d41,// 102 PAY  99 

    0x9a5ec6da,// 103 PAY 100 

    0xe5ac24fc,// 104 PAY 101 

    0xf1d4084c,// 105 PAY 102 

    0x25e426b6,// 106 PAY 103 

    0x4de8bd03,// 107 PAY 104 

    0xc6926d1a,// 108 PAY 105 

    0x6d1e7055,// 109 PAY 106 

    0x04aebaf7,// 110 PAY 107 

    0x96033e66,// 111 PAY 108 

    0xaff93d5d,// 112 PAY 109 

    0x4175f0a1,// 113 PAY 110 

    0xca223e8d,// 114 PAY 111 

    0x313219ab,// 115 PAY 112 

    0xa04f3bc1,// 116 PAY 113 

    0xf73fb9dd,// 117 PAY 114 

    0xf76c7e49,// 118 PAY 115 

    0xcc01be39,// 119 PAY 116 

    0xe3e18d30,// 120 PAY 117 

    0x000cc02e,// 121 PAY 118 

    0x2d1e6ba6,// 122 PAY 119 

    0xfd52f8ee,// 123 PAY 120 

    0xdd3097c9,// 124 PAY 121 

    0x42503f38,// 125 PAY 122 

    0xe5043679,// 126 PAY 123 

    0xca16e35c,// 127 PAY 124 

    0x2464c037,// 128 PAY 125 

    0xfe7392fc,// 129 PAY 126 

    0x4402643e,// 130 PAY 127 

    0x722a2666,// 131 PAY 128 

    0x37245b18,// 132 PAY 129 

    0xf71d9ee2,// 133 PAY 130 

    0x8188d981,// 134 PAY 131 

    0x09520c73,// 135 PAY 132 

    0x3d247d60,// 136 PAY 133 

    0x2fa06c30,// 137 PAY 134 

    0x7882c538,// 138 PAY 135 

    0xe31e75d0,// 139 PAY 136 

    0x93fa30c1,// 140 PAY 137 

    0xe5119aa4,// 141 PAY 138 

    0x95446dff,// 142 PAY 139 

    0x3c33fa6d,// 143 PAY 140 

    0x3f02270e,// 144 PAY 141 

    0x351a381b,// 145 PAY 142 

    0xb5d3041e,// 146 PAY 143 

    0xeae98d7c,// 147 PAY 144 

    0x8f98c650,// 148 PAY 145 

    0x8a87ed0d,// 149 PAY 146 

    0x25de9c2b,// 150 PAY 147 

    0x615b9142,// 151 PAY 148 

    0x7d7454f5,// 152 PAY 149 

    0x8ee83150,// 153 PAY 150 

    0xcd73239e,// 154 PAY 151 

    0xbf7dc450,// 155 PAY 152 

    0x47d80201,// 156 PAY 153 

    0xf19f1f81,// 157 PAY 154 

    0xe56e63d9,// 158 PAY 155 

    0x6974fa61,// 159 PAY 156 

    0x6563485d,// 160 PAY 157 

    0x9cbfdbd3,// 161 PAY 158 

    0x7cac7634,// 162 PAY 159 

    0xe02fde3a,// 163 PAY 160 

    0x7da24c43,// 164 PAY 161 

    0x38c1d29b,// 165 PAY 162 

    0xfa92c803,// 166 PAY 163 

    0xc1a5dac2,// 167 PAY 164 

    0x8efcc84c,// 168 PAY 165 

    0xe28298b1,// 169 PAY 166 

    0x895530d0,// 170 PAY 167 

    0xbaa1eff4,// 171 PAY 168 

    0xae3012f6,// 172 PAY 169 

    0xb871d61f,// 173 PAY 170 

    0x7487cd10,// 174 PAY 171 

    0xbd8a4366,// 175 PAY 172 

    0x9ab54827,// 176 PAY 173 

    0x9f0a8bee,// 177 PAY 174 

    0x17fe389b,// 178 PAY 175 

    0xe77501a9,// 179 PAY 176 

    0x2ebc46b1,// 180 PAY 177 

    0x48892657,// 181 PAY 178 

    0xbc8f3183,// 182 PAY 179 

    0xc01c4c7f,// 183 PAY 180 

    0x2da2abfd,// 184 PAY 181 

    0x45be9507,// 185 PAY 182 

    0x15062cbb,// 186 PAY 183 

    0xf587f0fa,// 187 PAY 184 

    0x8d167ab3,// 188 PAY 185 

    0x94c14b48,// 189 PAY 186 

    0x1a6ef857,// 190 PAY 187 

    0xcf3ac294,// 191 PAY 188 

    0xec96d44e,// 192 PAY 189 

    0x59d9a749,// 193 PAY 190 

    0x9c7d3d48,// 194 PAY 191 

    0x2592ca0e,// 195 PAY 192 

    0xb6002341,// 196 PAY 193 

    0x11c2b7f2,// 197 PAY 194 

    0x4de7b67a,// 198 PAY 195 

    0x63f01663,// 199 PAY 196 

    0xc4b99156,// 200 PAY 197 

    0x4f3d1ba2,// 201 PAY 198 

    0x90e430d0,// 202 PAY 199 

    0xa7ea9e64,// 203 PAY 200 

    0x413b2729,// 204 PAY 201 

    0x1489cbd0,// 205 PAY 202 

    0xac6881a5,// 206 PAY 203 

    0x7ac44728,// 207 PAY 204 

    0x402dea35,// 208 PAY 205 

    0xca7cf804,// 209 PAY 206 

    0x98a020d0,// 210 PAY 207 

    0x818cdb55,// 211 PAY 208 

    0xdb7f2739,// 212 PAY 209 

    0x8c2c91a3,// 213 PAY 210 

    0xca28639a,// 214 PAY 211 

    0x9e9f804a,// 215 PAY 212 

    0xca14bd2a,// 216 PAY 213 

    0x01823f41,// 217 PAY 214 

    0xa0bfae01,// 218 PAY 215 

    0x4dff26fb,// 219 PAY 216 

    0x4863b2e2,// 220 PAY 217 

    0x21c924a4,// 221 PAY 218 

    0x21330288,// 222 PAY 219 

    0x8bb4a971,// 223 PAY 220 

    0xba6e1ff1,// 224 PAY 221 

    0x27290be7,// 225 PAY 222 

    0x38e98c15,// 226 PAY 223 

    0x12ca6860,// 227 PAY 224 

    0x9f2a1ffc,// 228 PAY 225 

    0x28af3462,// 229 PAY 226 

    0xda8c0ac6,// 230 PAY 227 

    0x0164cece,// 231 PAY 228 

    0x64611691,// 232 PAY 229 

    0xf178be3a,// 233 PAY 230 

    0x7d3c3244,// 234 PAY 231 

    0xb6b4cbb7,// 235 PAY 232 

    0x68c34838,// 236 PAY 233 

    0xac63e28c,// 237 PAY 234 

    0x9dcff1a7,// 238 PAY 235 

    0x962d163e,// 239 PAY 236 

    0x393059b9,// 240 PAY 237 

    0x9d6f3375,// 241 PAY 238 

    0xcfba57a6,// 242 PAY 239 

    0x08f27c1b,// 243 PAY 240 

    0xee829178,// 244 PAY 241 

    0xb07281af,// 245 PAY 242 

    0x6b6b5b3b,// 246 PAY 243 

    0xee230c65,// 247 PAY 244 

    0xa871224a,// 248 PAY 245 

    0x56e43fac,// 249 PAY 246 

    0xbe09cdbb,// 250 PAY 247 

    0x6d2b39b0,// 251 PAY 248 

    0xe49c815b,// 252 PAY 249 

    0x64bf8578,// 253 PAY 250 

    0x92919ce6,// 254 PAY 251 

    0xf0a183ed,// 255 PAY 252 

    0x0a7f4ef4,// 256 PAY 253 

    0x15b869f6,// 257 PAY 254 

    0xb9da82bf,// 258 PAY 255 

    0xf0a3ae43,// 259 PAY 256 

    0x9fddc902,// 260 PAY 257 

    0x1c7bdb7f,// 261 PAY 258 

    0x81b2069e,// 262 PAY 259 

    0x7a3f1663,// 263 PAY 260 

    0xad22b17a,// 264 PAY 261 

    0x79bd595a,// 265 PAY 262 

    0xe63d5c91,// 266 PAY 263 

    0xd3969cda,// 267 PAY 264 

    0xa1b9db2e,// 268 PAY 265 

    0xaee94589,// 269 PAY 266 

    0x0267adfd,// 270 PAY 267 

    0xa7b69962,// 271 PAY 268 

    0x74c78801,// 272 PAY 269 

    0xb5bea988,// 273 PAY 270 

    0xae46f60c,// 274 PAY 271 

    0x0368fe2e,// 275 PAY 272 

    0x765513f8,// 276 PAY 273 

    0x8942a9e3,// 277 PAY 274 

    0xf69f6128,// 278 PAY 275 

    0x5de8fddb,// 279 PAY 276 

    0x9292bfb5,// 280 PAY 277 

    0x59555316,// 281 PAY 278 

    0xf052e6de,// 282 PAY 279 

    0x94873c80,// 283 PAY 280 

    0x198412dd,// 284 PAY 281 

    0x19bc1bf3,// 285 PAY 282 

    0x65b24377,// 286 PAY 283 

    0xa2d01bda,// 287 PAY 284 

    0xc452422f,// 288 PAY 285 

    0x788b870a,// 289 PAY 286 

    0xa717acfd,// 290 PAY 287 

    0x8df0847a,// 291 PAY 288 

    0x37f245be,// 292 PAY 289 

    0x2d781cac,// 293 PAY 290 

    0x1f319507,// 294 PAY 291 

    0xb999a4ad,// 295 PAY 292 

    0xc4e849a3,// 296 PAY 293 

    0x08073cc8,// 297 PAY 294 

    0x1bedf7bd,// 298 PAY 295 

    0x87083e9c,// 299 PAY 296 

    0x84f8d9e0,// 300 PAY 297 

    0xa02a097f,// 301 PAY 298 

    0x9c162f2f,// 302 PAY 299 

    0xa799bb6d,// 303 PAY 300 

    0x8ae374ab,// 304 PAY 301 

    0x75ecb96f,// 305 PAY 302 

    0x3882d39e,// 306 PAY 303 

    0xda1d7052,// 307 PAY 304 

    0x106fca32,// 308 PAY 305 

    0x3d60fd27,// 309 PAY 306 

    0x5ffed5dc,// 310 PAY 307 

    0x3c0fe7a0,// 311 PAY 308 

    0x624f9cdc,// 312 PAY 309 

    0x4a6be3f9,// 313 PAY 310 

    0x880c5129,// 314 PAY 311 

    0x7ee09287,// 315 PAY 312 

    0x06db1bc1,// 316 PAY 313 

    0xaa794952,// 317 PAY 314 

    0xc2db4100,// 318 PAY 315 

    0x5cb05a1e,// 319 PAY 316 

    0xbfe07c49,// 320 PAY 317 

    0x8b73127d,// 321 PAY 318 

    0xbd3a8b0e,// 322 PAY 319 

    0xd761a9ab,// 323 PAY 320 

    0x0902835a,// 324 PAY 321 

    0xb227d11b,// 325 PAY 322 

    0x13a8603a,// 326 PAY 323 

    0x6984e729,// 327 PAY 324 

    0x1a756e72,// 328 PAY 325 

    0x75d6912b,// 329 PAY 326 

    0xd26a3c9a,// 330 PAY 327 

    0x71eac7be,// 331 PAY 328 

    0xaa36782f,// 332 PAY 329 

    0x0d23d558,// 333 PAY 330 

    0x6957f122,// 334 PAY 331 

    0xc7400d69,// 335 PAY 332 

    0x73d23671,// 336 PAY 333 

    0xfbbdbd10,// 337 PAY 334 

    0x6fe0cb78,// 338 PAY 335 

    0x7cc07b22,// 339 PAY 336 

    0x83aeb5e6,// 340 PAY 337 

    0x451fe760,// 341 PAY 338 

    0x12cf177c,// 342 PAY 339 

    0x19a13fb7,// 343 PAY 340 

    0x666c6cc4,// 344 PAY 341 

    0xf0d0a554,// 345 PAY 342 

    0x428bafa7,// 346 PAY 343 

    0x29c16f1c,// 347 PAY 344 

    0x5d937706,// 348 PAY 345 

    0xbc879775,// 349 PAY 346 

    0x266e14af,// 350 PAY 347 

    0xf09b4c40,// 351 PAY 348 

    0xf945853c,// 352 PAY 349 

    0xe2258519,// 353 PAY 350 

    0xbfca556c,// 354 PAY 351 

    0x95444964,// 355 PAY 352 

    0x9a944986,// 356 PAY 353 

    0xaac172ce,// 357 PAY 354 

    0xdfb271f6,// 358 PAY 355 

    0x6b995f51,// 359 PAY 356 

    0x181f441d,// 360 PAY 357 

    0x44eb9e91,// 361 PAY 358 

    0xb6a2c2b8,// 362 PAY 359 

    0xf217b53e,// 363 PAY 360 

    0xa8aa8661,// 364 PAY 361 

    0x1028f734,// 365 PAY 362 

    0xea5db38a,// 366 PAY 363 

    0x8cb526fe,// 367 PAY 364 

    0x7c683eea,// 368 PAY 365 

    0xc03d2950,// 369 PAY 366 

    0x65e877d4,// 370 PAY 367 

    0x1ca5a22e,// 371 PAY 368 

    0xe64caf61,// 372 PAY 369 

    0x71c3cada,// 373 PAY 370 

    0x88afd8b4,// 374 PAY 371 

    0xbc38ffa3,// 375 PAY 372 

    0x376b6c87,// 376 PAY 373 

    0xa5e5c1f9,// 377 PAY 374 

    0xfc2e25df,// 378 PAY 375 

    0x8659646d,// 379 PAY 376 

    0x30da1ace,// 380 PAY 377 

    0x77fde0f0,// 381 PAY 378 

    0xd6e6b012,// 382 PAY 379 

    0x92c050b0,// 383 PAY 380 

    0x428779d0,// 384 PAY 381 

    0xccfe1d48,// 385 PAY 382 

    0x07eb6190,// 386 PAY 383 

    0xe0821607,// 387 PAY 384 

    0xf844d3d1,// 388 PAY 385 

    0x06e54c6d,// 389 PAY 386 

    0xebe50335,// 390 PAY 387 

    0xc296c0a8,// 391 PAY 388 

    0xe5b5def8,// 392 PAY 389 

    0x3bed1c69,// 393 PAY 390 

    0xfe30934b,// 394 PAY 391 

    0xe7c8f679,// 395 PAY 392 

    0x651cda10,// 396 PAY 393 

    0x53207572,// 397 PAY 394 

    0x4f2802ba,// 398 PAY 395 

    0x313f13b0,// 399 PAY 396 

    0x3e595848,// 400 PAY 397 

    0x170beb7d,// 401 PAY 398 

    0xbf5ecadb,// 402 PAY 399 

    0xb465071c,// 403 PAY 400 

    0x522b83d6,// 404 PAY 401 

    0x69a21516,// 405 PAY 402 

    0x2e55e822,// 406 PAY 403 

    0xd944b1b6,// 407 PAY 404 

    0x20ecffa8,// 408 PAY 405 

    0x43eff6f4,// 409 PAY 406 

    0xef3b794e,// 410 PAY 407 

    0x8d497945,// 411 PAY 408 

    0xe10fab41,// 412 PAY 409 

    0x81c60cc5,// 413 PAY 410 

    0x6a9cb363,// 414 PAY 411 

    0x23cecc69,// 415 PAY 412 

    0xc423b6d8,// 416 PAY 413 

    0x30ed457b,// 417 PAY 414 

    0xa40c6b1d,// 418 PAY 415 

    0xb0f0976b,// 419 PAY 416 

    0x90f32369,// 420 PAY 417 

    0xc3dd19ec,// 421 PAY 418 

    0xf44a9e5f,// 422 PAY 419 

    0x82b94bc9,// 423 PAY 420 

    0xeefa711b,// 424 PAY 421 

    0xab12637d,// 425 PAY 422 

    0x67df6782,// 426 PAY 423 

    0xb24704f5,// 427 PAY 424 

    0x2387145f,// 428 PAY 425 

    0xf7bbad20,// 429 PAY 426 

    0x534e9006,// 430 PAY 427 

    0xba086eb0,// 431 PAY 428 

    0x0cb55569,// 432 PAY 429 

    0x76c4de60,// 433 PAY 430 

    0x1892f32f,// 434 PAY 431 

    0x7f4fa946,// 435 PAY 432 

    0xadee3f2c,// 436 PAY 433 

    0xe27de045,// 437 PAY 434 

    0xc7382927,// 438 PAY 435 

    0x58aca509,// 439 PAY 436 

    0xbf2f489d,// 440 PAY 437 

    0xec4e0d16,// 441 PAY 438 

    0x7e1c4e32,// 442 PAY 439 

    0xa9f6c0b5,// 443 PAY 440 

    0x54515cb2,// 444 PAY 441 

    0x4baa8d00,// 445 PAY 442 

    0xb8373000,// 446 PAY 443 

    0x51921f7e,// 447 PAY 444 

    0x301edaa9,// 448 PAY 445 

    0xa4e67ed0,// 449 PAY 446 

    0x9d512a2f,// 450 PAY 447 

    0x27d69d7e,// 451 PAY 448 

    0xca9ff590,// 452 PAY 449 

    0x06214e63,// 453 PAY 450 

    0x2dcbd5ee,// 454 PAY 451 

    0xc8cc6a2a,// 455 PAY 452 

    0xb7bfe802,// 456 PAY 453 

    0x4f36d580,// 457 PAY 454 

    0xa2bdaa35,// 458 PAY 455 

    0xa504eed0,// 459 PAY 456 

    0xf2920c72,// 460 PAY 457 

    0x6b97e3da,// 461 PAY 458 

    0x8cb67451,// 462 PAY 459 

    0xbd11eab9,// 463 PAY 460 

    0x09197788,// 464 PAY 461 

    0x5c449632,// 465 PAY 462 

    0xc86b91d5,// 466 PAY 463 

    0x73ae6ae9,// 467 PAY 464 

    0x4b19d117,// 468 PAY 465 

    0xac5a9480,// 469 PAY 466 

    0x75893b29,// 470 PAY 467 

    0x7137fca9,// 471 PAY 468 

    0xc9402de5,// 472 PAY 469 

    0x1014d603,// 473 PAY 470 

    0x37123c8c,// 474 PAY 471 

    0xd54c59f9,// 475 PAY 472 

    0xbecbce21,// 476 PAY 473 

    0xb2c5250e,// 477 PAY 474 

    0x3f2e4d99,// 478 PAY 475 

    0x7816e514,// 479 PAY 476 

    0xf44c7ea3,// 480 PAY 477 

    0x3cc9810f,// 481 PAY 478 

    0xe434a574,// 482 PAY 479 

    0xfb302d56,// 483 PAY 480 

    0x92ba7fb3,// 484 PAY 481 

    0xa1e47385,// 485 PAY 482 

    0x03a6d4a5,// 486 PAY 483 

    0x512c8bcc,// 487 PAY 484 

    0xb08a6b70,// 488 PAY 485 

    0x417804d6,// 489 PAY 486 

    0x8e1a1782,// 490 PAY 487 

    0x4d41f650,// 491 PAY 488 

    0xad211852,// 492 PAY 489 

    0xb9c0e050,// 493 PAY 490 

    0x826db892,// 494 PAY 491 

    0x69d2f13e,// 495 PAY 492 

    0x02790000,// 496 PAY 493 

/// HASH is  4 bytes 

    0xb6a2c2b8,// 497 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 212 

/// STA pkt_idx        : 86 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe6 

    0x0158e6d4 // 498 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt19_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 48 words. 

/// BDA size     is 186 (0xba) 

/// BDA id       is 0x426c 

    0x00ba426c,// 3 BDA   1 

/// PAY Generic Data size   : 186 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xc3213061,// 4 PAY   1 

    0xcdfd4d09,// 5 PAY   2 

    0x6a158325,// 6 PAY   3 

    0x8d45e141,// 7 PAY   4 

    0xbf2b58f5,// 8 PAY   5 

    0x40273ba6,// 9 PAY   6 

    0xa0503724,// 10 PAY   7 

    0xd0be8a6d,// 11 PAY   8 

    0x9380a69d,// 12 PAY   9 

    0x79e06ff9,// 13 PAY  10 

    0xfdecf315,// 14 PAY  11 

    0x02e2d22c,// 15 PAY  12 

    0xda81168b,// 16 PAY  13 

    0xaf740cd8,// 17 PAY  14 

    0x47cdf4e8,// 18 PAY  15 

    0x74892634,// 19 PAY  16 

    0x63db564a,// 20 PAY  17 

    0x4e66a454,// 21 PAY  18 

    0x57a7b388,// 22 PAY  19 

    0xb6c232a7,// 23 PAY  20 

    0x741fe96d,// 24 PAY  21 

    0xda63ec0b,// 25 PAY  22 

    0xc9f21584,// 26 PAY  23 

    0x9bafe017,// 27 PAY  24 

    0xf4899e2c,// 28 PAY  25 

    0xd1bf4e54,// 29 PAY  26 

    0xe292b454,// 30 PAY  27 

    0xf76479d3,// 31 PAY  28 

    0x467e69db,// 32 PAY  29 

    0xb28d5ce4,// 33 PAY  30 

    0x9c395ea7,// 34 PAY  31 

    0x4387288c,// 35 PAY  32 

    0x9f8ad9cc,// 36 PAY  33 

    0x0d7a0655,// 37 PAY  34 

    0xceb814fe,// 38 PAY  35 

    0x17499764,// 39 PAY  36 

    0x8580f57f,// 40 PAY  37 

    0x2cd47bc2,// 41 PAY  38 

    0xceb9e1cc,// 42 PAY  39 

    0xb177f4e9,// 43 PAY  40 

    0x99b969c1,// 44 PAY  41 

    0x592666ec,// 45 PAY  42 

    0xaf254e66,// 46 PAY  43 

    0x6fea3b19,// 47 PAY  44 

    0x348eaa15,// 48 PAY  45 

    0xe9f39a6c,// 49 PAY  46 

    0x45eb0000,// 50 PAY  47 

/// HASH is  16 bytes 

    0xb28d5ce4,// 51 HSH   1 

    0x9c395ea7,// 52 HSH   2 

    0x4387288c,// 53 HSH   3 

    0x9f8ad9cc,// 54 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 27 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02cc3c1b // 55 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt20_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 344 words. 

/// BDA size     is 1371 (0x55b) 

/// BDA id       is 0x1692 

    0x055b1692,// 3 BDA   1 

/// PAY Generic Data size   : 1371 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xeaa0086b,// 4 PAY   1 

    0x8f848f52,// 5 PAY   2 

    0x5ee4f389,// 6 PAY   3 

    0x76e934c3,// 7 PAY   4 

    0xb1c1f0a6,// 8 PAY   5 

    0x23d261df,// 9 PAY   6 

    0xaffdbe5c,// 10 PAY   7 

    0x566f38fb,// 11 PAY   8 

    0x2531814e,// 12 PAY   9 

    0xcc12d4cd,// 13 PAY  10 

    0xd4abc50d,// 14 PAY  11 

    0xc0dbd608,// 15 PAY  12 

    0x96647e86,// 16 PAY  13 

    0xf9caaf09,// 17 PAY  14 

    0xd3d721f6,// 18 PAY  15 

    0x0047f0db,// 19 PAY  16 

    0x8d5bf342,// 20 PAY  17 

    0x61605d3c,// 21 PAY  18 

    0x83d69abc,// 22 PAY  19 

    0xa4fe8fd0,// 23 PAY  20 

    0x06ce1c53,// 24 PAY  21 

    0xf02c0d65,// 25 PAY  22 

    0xd864f4f6,// 26 PAY  23 

    0xff76e4ab,// 27 PAY  24 

    0xd769aa85,// 28 PAY  25 

    0x92bba86b,// 29 PAY  26 

    0xca728b81,// 30 PAY  27 

    0x53856ffb,// 31 PAY  28 

    0xc075fdc0,// 32 PAY  29 

    0xd2eac4d9,// 33 PAY  30 

    0x8178e117,// 34 PAY  31 

    0xf71a62d1,// 35 PAY  32 

    0x34f851cc,// 36 PAY  33 

    0x6ea311d9,// 37 PAY  34 

    0xc1ca12b9,// 38 PAY  35 

    0xe9f8b1a1,// 39 PAY  36 

    0x822f3c6f,// 40 PAY  37 

    0x6165d521,// 41 PAY  38 

    0xc763df63,// 42 PAY  39 

    0x0573203d,// 43 PAY  40 

    0x8c66cfa3,// 44 PAY  41 

    0xcba1ffaf,// 45 PAY  42 

    0x0158d090,// 46 PAY  43 

    0x4a29d551,// 47 PAY  44 

    0x2c9d4a1d,// 48 PAY  45 

    0xa12df3e0,// 49 PAY  46 

    0x2fa5b4d2,// 50 PAY  47 

    0x78ae221d,// 51 PAY  48 

    0xeaa853b9,// 52 PAY  49 

    0x923bb4ef,// 53 PAY  50 

    0x1b1313aa,// 54 PAY  51 

    0x85c93893,// 55 PAY  52 

    0xc5dc275f,// 56 PAY  53 

    0xb3289328,// 57 PAY  54 

    0x101560e0,// 58 PAY  55 

    0x97063835,// 59 PAY  56 

    0xe553a50a,// 60 PAY  57 

    0xe66f2cbf,// 61 PAY  58 

    0x432d1b8b,// 62 PAY  59 

    0x6822bf05,// 63 PAY  60 

    0x21286d47,// 64 PAY  61 

    0x77e933d0,// 65 PAY  62 

    0x858db6e8,// 66 PAY  63 

    0xf37f63c8,// 67 PAY  64 

    0xe5661333,// 68 PAY  65 

    0xe4976bfc,// 69 PAY  66 

    0xcaf747db,// 70 PAY  67 

    0xa64db82f,// 71 PAY  68 

    0x1166b3e4,// 72 PAY  69 

    0x9ccdc58f,// 73 PAY  70 

    0xba5a12f1,// 74 PAY  71 

    0xc618bf1b,// 75 PAY  72 

    0xcc36cc3d,// 76 PAY  73 

    0xbf72bc77,// 77 PAY  74 

    0x52ed55c0,// 78 PAY  75 

    0x8e1ab105,// 79 PAY  76 

    0x3688dd13,// 80 PAY  77 

    0xb6179283,// 81 PAY  78 

    0x2607c980,// 82 PAY  79 

    0x55db37f6,// 83 PAY  80 

    0xc5995747,// 84 PAY  81 

    0x3c698a25,// 85 PAY  82 

    0xd20a0a5a,// 86 PAY  83 

    0x6fd67a34,// 87 PAY  84 

    0x824930a4,// 88 PAY  85 

    0x5557f5aa,// 89 PAY  86 

    0x190428ad,// 90 PAY  87 

    0x735a3ba3,// 91 PAY  88 

    0x671d47a2,// 92 PAY  89 

    0x25254f7b,// 93 PAY  90 

    0xe260a675,// 94 PAY  91 

    0xa7093243,// 95 PAY  92 

    0x14e9490b,// 96 PAY  93 

    0xddcd86f4,// 97 PAY  94 

    0xad681ec2,// 98 PAY  95 

    0xd0adfa14,// 99 PAY  96 

    0x2f2d20b9,// 100 PAY  97 

    0x28cfae69,// 101 PAY  98 

    0xb35a4a74,// 102 PAY  99 

    0x795b0c26,// 103 PAY 100 

    0x1115658f,// 104 PAY 101 

    0xb49b1aac,// 105 PAY 102 

    0x978841c0,// 106 PAY 103 

    0x391fb94c,// 107 PAY 104 

    0x72967f67,// 108 PAY 105 

    0x6f15c1e3,// 109 PAY 106 

    0xf04e556d,// 110 PAY 107 

    0xd3dd8314,// 111 PAY 108 

    0x8f3fe05f,// 112 PAY 109 

    0xb63df964,// 113 PAY 110 

    0x0ad9cf57,// 114 PAY 111 

    0x071785be,// 115 PAY 112 

    0x689f0803,// 116 PAY 113 

    0xe7a23c43,// 117 PAY 114 

    0xfbcde3af,// 118 PAY 115 

    0x2ffd489c,// 119 PAY 116 

    0x6e5ba174,// 120 PAY 117 

    0x8dbc916e,// 121 PAY 118 

    0x4acd687b,// 122 PAY 119 

    0xea989e2d,// 123 PAY 120 

    0x02650c62,// 124 PAY 121 

    0x02a46c61,// 125 PAY 122 

    0x5cd7b506,// 126 PAY 123 

    0xd6b3d156,// 127 PAY 124 

    0xc133b6b5,// 128 PAY 125 

    0x88a58306,// 129 PAY 126 

    0x0d7e8d5a,// 130 PAY 127 

    0x4df92ef1,// 131 PAY 128 

    0x695c3305,// 132 PAY 129 

    0xa263ae94,// 133 PAY 130 

    0x65cbdbfe,// 134 PAY 131 

    0x1cfcbaf5,// 135 PAY 132 

    0xac324cd3,// 136 PAY 133 

    0x02ab6017,// 137 PAY 134 

    0x123458c3,// 138 PAY 135 

    0xd240a84a,// 139 PAY 136 

    0x1a706150,// 140 PAY 137 

    0xffcb737c,// 141 PAY 138 

    0xe8aa824d,// 142 PAY 139 

    0x8c3bc5df,// 143 PAY 140 

    0x7ee74647,// 144 PAY 141 

    0xa120db3d,// 145 PAY 142 

    0x93c132c8,// 146 PAY 143 

    0x1bdad2db,// 147 PAY 144 

    0xdbb0530f,// 148 PAY 145 

    0xdc7f8239,// 149 PAY 146 

    0xd704ef46,// 150 PAY 147 

    0x383d0f21,// 151 PAY 148 

    0xf1530a66,// 152 PAY 149 

    0x459bc656,// 153 PAY 150 

    0x12ab6882,// 154 PAY 151 

    0x83e68c34,// 155 PAY 152 

    0xf36ead2c,// 156 PAY 153 

    0x1789e979,// 157 PAY 154 

    0xaad83033,// 158 PAY 155 

    0x591d8383,// 159 PAY 156 

    0x51b019b2,// 160 PAY 157 

    0x3530a378,// 161 PAY 158 

    0xd9bca042,// 162 PAY 159 

    0x43546c89,// 163 PAY 160 

    0x72883e5b,// 164 PAY 161 

    0x3df9bc6d,// 165 PAY 162 

    0x3e62106b,// 166 PAY 163 

    0x3bab668b,// 167 PAY 164 

    0x9f316da3,// 168 PAY 165 

    0xfdb9f9c1,// 169 PAY 166 

    0xd47029aa,// 170 PAY 167 

    0x7bc224b6,// 171 PAY 168 

    0x3403f204,// 172 PAY 169 

    0x9d913e19,// 173 PAY 170 

    0xd1f3cd33,// 174 PAY 171 

    0xbba50415,// 175 PAY 172 

    0x479661f2,// 176 PAY 173 

    0xa9788e12,// 177 PAY 174 

    0x1b63eaf4,// 178 PAY 175 

    0x2c496e0c,// 179 PAY 176 

    0x5d652ac1,// 180 PAY 177 

    0x6ae5b1cb,// 181 PAY 178 

    0x63166c1f,// 182 PAY 179 

    0x5479d3bd,// 183 PAY 180 

    0xd75f6a09,// 184 PAY 181 

    0xd672a6fb,// 185 PAY 182 

    0xf9725a36,// 186 PAY 183 

    0x2ac73d3c,// 187 PAY 184 

    0x8a132e09,// 188 PAY 185 

    0x5bcea841,// 189 PAY 186 

    0x01aa7054,// 190 PAY 187 

    0x6116d656,// 191 PAY 188 

    0x13ee6e97,// 192 PAY 189 

    0xb9cb38ee,// 193 PAY 190 

    0x75560fa4,// 194 PAY 191 

    0x533d43af,// 195 PAY 192 

    0xf3b7d6d8,// 196 PAY 193 

    0xa8fed6c8,// 197 PAY 194 

    0x4c56435b,// 198 PAY 195 

    0x8673a81b,// 199 PAY 196 

    0x21d478dd,// 200 PAY 197 

    0xb9d8f177,// 201 PAY 198 

    0xe7f6624f,// 202 PAY 199 

    0x66237fd2,// 203 PAY 200 

    0x0f007868,// 204 PAY 201 

    0x327005f7,// 205 PAY 202 

    0xb9629fed,// 206 PAY 203 

    0x3d8958bb,// 207 PAY 204 

    0xc8a929bf,// 208 PAY 205 

    0x9241cb21,// 209 PAY 206 

    0xb67b02e6,// 210 PAY 207 

    0x4205147d,// 211 PAY 208 

    0xc6b32bc0,// 212 PAY 209 

    0xd7bffb65,// 213 PAY 210 

    0x7218f993,// 214 PAY 211 

    0x871f4051,// 215 PAY 212 

    0x68e597e4,// 216 PAY 213 

    0x1617e05e,// 217 PAY 214 

    0x06d6503b,// 218 PAY 215 

    0x19ad1f0e,// 219 PAY 216 

    0xc9894505,// 220 PAY 217 

    0x8cd2f4e1,// 221 PAY 218 

    0x185fc4ae,// 222 PAY 219 

    0x5eff080d,// 223 PAY 220 

    0x5c116161,// 224 PAY 221 

    0xf6074817,// 225 PAY 222 

    0xe38c719d,// 226 PAY 223 

    0x566960eb,// 227 PAY 224 

    0x33b75923,// 228 PAY 225 

    0x0c9c2617,// 229 PAY 226 

    0x860fdff0,// 230 PAY 227 

    0x5d10afb5,// 231 PAY 228 

    0x4a32de29,// 232 PAY 229 

    0xb20064ba,// 233 PAY 230 

    0xf386bb4d,// 234 PAY 231 

    0xdecd822c,// 235 PAY 232 

    0x3a2a47c8,// 236 PAY 233 

    0xb2c2fde6,// 237 PAY 234 

    0xba0565ac,// 238 PAY 235 

    0x8446cf89,// 239 PAY 236 

    0xeb076d6b,// 240 PAY 237 

    0x3946d919,// 241 PAY 238 

    0x219609f4,// 242 PAY 239 

    0xc9d20cc0,// 243 PAY 240 

    0x88649a40,// 244 PAY 241 

    0x435fa74e,// 245 PAY 242 

    0xd7b40245,// 246 PAY 243 

    0xac1743ec,// 247 PAY 244 

    0x83ac49b8,// 248 PAY 245 

    0xd962b52e,// 249 PAY 246 

    0x938de42e,// 250 PAY 247 

    0x45d95a9e,// 251 PAY 248 

    0x5e4cb814,// 252 PAY 249 

    0x2ffc585f,// 253 PAY 250 

    0xb562fbca,// 254 PAY 251 

    0x88101bc2,// 255 PAY 252 

    0xd5269696,// 256 PAY 253 

    0xdbfd0caa,// 257 PAY 254 

    0x88a8acdf,// 258 PAY 255 

    0xd7b06c61,// 259 PAY 256 

    0x857ca23b,// 260 PAY 257 

    0x6f30750e,// 261 PAY 258 

    0x55d80795,// 262 PAY 259 

    0xc718a646,// 263 PAY 260 

    0x0fa4b919,// 264 PAY 261 

    0x857c42e3,// 265 PAY 262 

    0xc066fe1a,// 266 PAY 263 

    0x69dbd139,// 267 PAY 264 

    0x4d0cb22e,// 268 PAY 265 

    0xfe1e0fb5,// 269 PAY 266 

    0x9192411a,// 270 PAY 267 

    0x39f5600b,// 271 PAY 268 

    0x3bacbda7,// 272 PAY 269 

    0x12dc033b,// 273 PAY 270 

    0xbf798d72,// 274 PAY 271 

    0x0e7390c5,// 275 PAY 272 

    0x2c117b07,// 276 PAY 273 

    0x4b5b1163,// 277 PAY 274 

    0x5b4bf61b,// 278 PAY 275 

    0x2e561cd6,// 279 PAY 276 

    0xf4c04ede,// 280 PAY 277 

    0xd6c881a5,// 281 PAY 278 

    0x4c4d3a53,// 282 PAY 279 

    0x29b09e40,// 283 PAY 280 

    0x879146f8,// 284 PAY 281 

    0x1498ce15,// 285 PAY 282 

    0x539ee5d0,// 286 PAY 283 

    0xa4ce7012,// 287 PAY 284 

    0xcdb17147,// 288 PAY 285 

    0x741f9aa3,// 289 PAY 286 

    0x9799f3de,// 290 PAY 287 

    0xe3fe30ee,// 291 PAY 288 

    0xbdf9268b,// 292 PAY 289 

    0x7c9366aa,// 293 PAY 290 

    0x2cff9627,// 294 PAY 291 

    0xcbb27243,// 295 PAY 292 

    0x5d4eb75d,// 296 PAY 293 

    0x6187d7ce,// 297 PAY 294 

    0xf32f2421,// 298 PAY 295 

    0xb92d6916,// 299 PAY 296 

    0x89c3aba1,// 300 PAY 297 

    0x88012814,// 301 PAY 298 

    0xa9cd40b3,// 302 PAY 299 

    0xc0dcf22d,// 303 PAY 300 

    0xf363f104,// 304 PAY 301 

    0x88adb20b,// 305 PAY 302 

    0xc1aab0d0,// 306 PAY 303 

    0x5aee5a83,// 307 PAY 304 

    0x4f2f2ca4,// 308 PAY 305 

    0x165cff6b,// 309 PAY 306 

    0x4354b363,// 310 PAY 307 

    0xdbbfb0dc,// 311 PAY 308 

    0xe2ceeb89,// 312 PAY 309 

    0x1d868847,// 313 PAY 310 

    0x964b55e6,// 314 PAY 311 

    0x99f6863a,// 315 PAY 312 

    0xa4e8d054,// 316 PAY 313 

    0xd6ac9b46,// 317 PAY 314 

    0xa27b8e8d,// 318 PAY 315 

    0x22bddb1e,// 319 PAY 316 

    0x684bfaea,// 320 PAY 317 

    0x1aa16efb,// 321 PAY 318 

    0x5b18c62f,// 322 PAY 319 

    0x8187b169,// 323 PAY 320 

    0x283edea1,// 324 PAY 321 

    0x8f387c54,// 325 PAY 322 

    0xee1d3deb,// 326 PAY 323 

    0x6f49913b,// 327 PAY 324 

    0xf9c7d719,// 328 PAY 325 

    0x43a05236,// 329 PAY 326 

    0x997cefb4,// 330 PAY 327 

    0xcbb78675,// 331 PAY 328 

    0xa6fe100e,// 332 PAY 329 

    0x721fe482,// 333 PAY 330 

    0x2dc238a1,// 334 PAY 331 

    0xd6fc5c88,// 335 PAY 332 

    0xe60548e3,// 336 PAY 333 

    0x576bbddd,// 337 PAY 334 

    0x229c631a,// 338 PAY 335 

    0xef47c616,// 339 PAY 336 

    0x4e524b9a,// 340 PAY 337 

    0xce60d34e,// 341 PAY 338 

    0x5ae37df6,// 342 PAY 339 

    0xb48babfe,// 343 PAY 340 

    0x1ec2901b,// 344 PAY 341 

    0x82418720,// 345 PAY 342 

    0xccdf3a00,// 346 PAY 343 

/// HASH is  12 bytes 

    0x2dc238a1,// 347 HSH   1 

    0xd6fc5c88,// 348 HSH   2 

    0xe60548e3,// 349 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 230 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x005c5be6 // 350 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt21_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 166 words. 

/// BDA size     is 657 (0x291) 

/// BDA id       is 0xb7c2 

    0x0291b7c2,// 3 BDA   1 

/// PAY Generic Data size   : 657 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xd39d58e4,// 4 PAY   1 

    0xec93b206,// 5 PAY   2 

    0xfe74367e,// 6 PAY   3 

    0x0c49e384,// 7 PAY   4 

    0x03f5dc31,// 8 PAY   5 

    0xe3c55f58,// 9 PAY   6 

    0xf2be7827,// 10 PAY   7 

    0x337a8bfe,// 11 PAY   8 

    0x249a0829,// 12 PAY   9 

    0x94daab8f,// 13 PAY  10 

    0x694bec04,// 14 PAY  11 

    0x55d7730a,// 15 PAY  12 

    0x0ccfa987,// 16 PAY  13 

    0xcfbe6740,// 17 PAY  14 

    0x9e6aacbb,// 18 PAY  15 

    0xb3a05e84,// 19 PAY  16 

    0x9e864f8c,// 20 PAY  17 

    0x997a1c28,// 21 PAY  18 

    0x8e4dc43f,// 22 PAY  19 

    0xb796f675,// 23 PAY  20 

    0x42fcfac9,// 24 PAY  21 

    0xdb841240,// 25 PAY  22 

    0x32158ceb,// 26 PAY  23 

    0x1388a7b0,// 27 PAY  24 

    0x1db4e89f,// 28 PAY  25 

    0xfae8b053,// 29 PAY  26 

    0x72d6a494,// 30 PAY  27 

    0x7a888211,// 31 PAY  28 

    0xd4f001ff,// 32 PAY  29 

    0xb03bb15b,// 33 PAY  30 

    0xfcdca00d,// 34 PAY  31 

    0xec100f07,// 35 PAY  32 

    0xa8d1ab2a,// 36 PAY  33 

    0xda92d6a3,// 37 PAY  34 

    0xdc94fa65,// 38 PAY  35 

    0xc2862308,// 39 PAY  36 

    0x6f05fe6f,// 40 PAY  37 

    0x30693c3c,// 41 PAY  38 

    0x693a0547,// 42 PAY  39 

    0x84a03107,// 43 PAY  40 

    0x4d06b568,// 44 PAY  41 

    0x4b753da1,// 45 PAY  42 

    0x22739ed9,// 46 PAY  43 

    0xe0ec030b,// 47 PAY  44 

    0xfa87d803,// 48 PAY  45 

    0x2d8d57fa,// 49 PAY  46 

    0x19181aad,// 50 PAY  47 

    0x5a56945b,// 51 PAY  48 

    0x13338dd4,// 52 PAY  49 

    0x820527e7,// 53 PAY  50 

    0xfc1f28d8,// 54 PAY  51 

    0x3440565f,// 55 PAY  52 

    0x1ca1ecc9,// 56 PAY  53 

    0x70b74051,// 57 PAY  54 

    0xb6778314,// 58 PAY  55 

    0xccd8c5eb,// 59 PAY  56 

    0x6f6a0ff0,// 60 PAY  57 

    0x7c96adfc,// 61 PAY  58 

    0x5516cc4e,// 62 PAY  59 

    0x41da196b,// 63 PAY  60 

    0x48cbd087,// 64 PAY  61 

    0x75aa0f40,// 65 PAY  62 

    0x2a634491,// 66 PAY  63 

    0x6331041f,// 67 PAY  64 

    0x05206bff,// 68 PAY  65 

    0xce900f70,// 69 PAY  66 

    0x11a5ecfa,// 70 PAY  67 

    0x84239f25,// 71 PAY  68 

    0x8aa5281c,// 72 PAY  69 

    0xbf179ba3,// 73 PAY  70 

    0xf553bc06,// 74 PAY  71 

    0x93196195,// 75 PAY  72 

    0x5b5a6a92,// 76 PAY  73 

    0xdddffa15,// 77 PAY  74 

    0x89cbae12,// 78 PAY  75 

    0x4ae5e0b8,// 79 PAY  76 

    0x618b5311,// 80 PAY  77 

    0x744156b0,// 81 PAY  78 

    0xddc85f38,// 82 PAY  79 

    0x65ad7da0,// 83 PAY  80 

    0xd06cead3,// 84 PAY  81 

    0x15f0bcdc,// 85 PAY  82 

    0x841d26ac,// 86 PAY  83 

    0xda4f394b,// 87 PAY  84 

    0xd7a08c8d,// 88 PAY  85 

    0xbe705e01,// 89 PAY  86 

    0x8770e918,// 90 PAY  87 

    0x2a4aca72,// 91 PAY  88 

    0x50b34441,// 92 PAY  89 

    0x250cd3c1,// 93 PAY  90 

    0x5dfd5fa0,// 94 PAY  91 

    0x97f093ed,// 95 PAY  92 

    0x43cc1ccc,// 96 PAY  93 

    0x5d942ec5,// 97 PAY  94 

    0xf415723d,// 98 PAY  95 

    0x2198de1c,// 99 PAY  96 

    0x915083cb,// 100 PAY  97 

    0xf64d7072,// 101 PAY  98 

    0xff52aef8,// 102 PAY  99 

    0x36a079ba,// 103 PAY 100 

    0x1ae169be,// 104 PAY 101 

    0xcff53a1c,// 105 PAY 102 

    0x0ac0a999,// 106 PAY 103 

    0x8935608f,// 107 PAY 104 

    0x00dbbcd8,// 108 PAY 105 

    0xb78fd64f,// 109 PAY 106 

    0x1d54cffc,// 110 PAY 107 

    0x505140a5,// 111 PAY 108 

    0xe207c0b4,// 112 PAY 109 

    0x458c0861,// 113 PAY 110 

    0x1fdc7294,// 114 PAY 111 

    0x8bed0723,// 115 PAY 112 

    0x37616635,// 116 PAY 113 

    0x0b8d19c6,// 117 PAY 114 

    0x1606d72e,// 118 PAY 115 

    0xb0de2850,// 119 PAY 116 

    0xfada5d34,// 120 PAY 117 

    0x9cbacc9f,// 121 PAY 118 

    0x9f6be015,// 122 PAY 119 

    0xe97f22c1,// 123 PAY 120 

    0x2daaab9a,// 124 PAY 121 

    0xd9802642,// 125 PAY 122 

    0xdde8db0f,// 126 PAY 123 

    0x459c16d0,// 127 PAY 124 

    0x949fede1,// 128 PAY 125 

    0xa76db7ff,// 129 PAY 126 

    0x65322624,// 130 PAY 127 

    0xb5e876ef,// 131 PAY 128 

    0xd4fb1f35,// 132 PAY 129 

    0x9db28f1b,// 133 PAY 130 

    0x50e11dd4,// 134 PAY 131 

    0x46c5ef59,// 135 PAY 132 

    0xc5563eeb,// 136 PAY 133 

    0xbe29f1b7,// 137 PAY 134 

    0x83fd5235,// 138 PAY 135 

    0x75ae0dd0,// 139 PAY 136 

    0xea5cfdc9,// 140 PAY 137 

    0x9a370096,// 141 PAY 138 

    0x3b629971,// 142 PAY 139 

    0x3762a006,// 143 PAY 140 

    0x34078911,// 144 PAY 141 

    0x33640863,// 145 PAY 142 

    0x86b9fbed,// 146 PAY 143 

    0xf101ad42,// 147 PAY 144 

    0x3951b858,// 148 PAY 145 

    0xf2294e1f,// 149 PAY 146 

    0x44002da8,// 150 PAY 147 

    0xa95581f8,// 151 PAY 148 

    0x77a13cbc,// 152 PAY 149 

    0xcedc51f9,// 153 PAY 150 

    0xa3ff783d,// 154 PAY 151 

    0x7f746c4d,// 155 PAY 152 

    0x20114669,// 156 PAY 153 

    0x6f0ac427,// 157 PAY 154 

    0x13dc447f,// 158 PAY 155 

    0x65d29eb8,// 159 PAY 156 

    0x0b7bfa9f,// 160 PAY 157 

    0xd7830d63,// 161 PAY 158 

    0x8eb837b6,// 162 PAY 159 

    0xe9b3b39c,// 163 PAY 160 

    0x6805f38f,// 164 PAY 161 

    0xffbeee53,// 165 PAY 162 

    0xbaa216c3,// 166 PAY 163 

    0xe49e4c31,// 167 PAY 164 

    0xb1000000,// 168 PAY 165 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 59 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x14 

    0x00ec1485 // 169 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt22_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 461 words. 

/// BDA size     is 1837 (0x72d) 

/// BDA id       is 0x393e 

    0x072d393e,// 3 BDA   1 

/// PAY Generic Data size   : 1837 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x1647e421,// 4 PAY   1 

    0x5ced6fe4,// 5 PAY   2 

    0xffc8ebe3,// 6 PAY   3 

    0x8200bbc0,// 7 PAY   4 

    0x577aa2bb,// 8 PAY   5 

    0x4a2808cd,// 9 PAY   6 

    0x06afcb35,// 10 PAY   7 

    0x14a0bb5b,// 11 PAY   8 

    0xb3ba0fc9,// 12 PAY   9 

    0xc5e90469,// 13 PAY  10 

    0x77a756e8,// 14 PAY  11 

    0xd48b61b4,// 15 PAY  12 

    0x0829650b,// 16 PAY  13 

    0x011084c1,// 17 PAY  14 

    0x8d73b55d,// 18 PAY  15 

    0xe86c1d0a,// 19 PAY  16 

    0x508f99d2,// 20 PAY  17 

    0xd81c644d,// 21 PAY  18 

    0xa964ab09,// 22 PAY  19 

    0xaf7f7cac,// 23 PAY  20 

    0x8b9c7225,// 24 PAY  21 

    0x3ce1c5b1,// 25 PAY  22 

    0xb33eeca8,// 26 PAY  23 

    0x32d9b605,// 27 PAY  24 

    0x2a686709,// 28 PAY  25 

    0xa7d3608b,// 29 PAY  26 

    0x6ffffbb6,// 30 PAY  27 

    0xf06571bc,// 31 PAY  28 

    0x8d5e357d,// 32 PAY  29 

    0xf0759a57,// 33 PAY  30 

    0xf41d18a2,// 34 PAY  31 

    0x97f3aea4,// 35 PAY  32 

    0x7fabaf00,// 36 PAY  33 

    0x6d0d4f1c,// 37 PAY  34 

    0x686e60d9,// 38 PAY  35 

    0x84df4139,// 39 PAY  36 

    0xb4215f0c,// 40 PAY  37 

    0x2a3f1288,// 41 PAY  38 

    0x37d7bf38,// 42 PAY  39 

    0x30918ac1,// 43 PAY  40 

    0x0623e415,// 44 PAY  41 

    0x95d65601,// 45 PAY  42 

    0xfb21c274,// 46 PAY  43 

    0x3c91372c,// 47 PAY  44 

    0xaf3d5857,// 48 PAY  45 

    0x4feb6f9d,// 49 PAY  46 

    0x15ae5c51,// 50 PAY  47 

    0x3f069ede,// 51 PAY  48 

    0xbe206015,// 52 PAY  49 

    0x38ea7a2f,// 53 PAY  50 

    0x70dd96bf,// 54 PAY  51 

    0x5ef67eb6,// 55 PAY  52 

    0x673c3a1d,// 56 PAY  53 

    0xaaaf859d,// 57 PAY  54 

    0x404f00dd,// 58 PAY  55 

    0x1fd1d0e7,// 59 PAY  56 

    0x124535ae,// 60 PAY  57 

    0xa6d49111,// 61 PAY  58 

    0xce00cfdb,// 62 PAY  59 

    0x8993a277,// 63 PAY  60 

    0x41d82d24,// 64 PAY  61 

    0x64ba3b5c,// 65 PAY  62 

    0x1c1b35fc,// 66 PAY  63 

    0xf03554f9,// 67 PAY  64 

    0x911c3372,// 68 PAY  65 

    0x75609b12,// 69 PAY  66 

    0xc45394da,// 70 PAY  67 

    0xbe5a77bb,// 71 PAY  68 

    0x68e1b125,// 72 PAY  69 

    0xe8ca2ff2,// 73 PAY  70 

    0x969c984c,// 74 PAY  71 

    0x617e896a,// 75 PAY  72 

    0x5453dd02,// 76 PAY  73 

    0xd07eadb5,// 77 PAY  74 

    0x628fbd92,// 78 PAY  75 

    0xe2334ce6,// 79 PAY  76 

    0x69b0f45e,// 80 PAY  77 

    0xb023ec22,// 81 PAY  78 

    0xbaaac8a4,// 82 PAY  79 

    0xb24bf1fa,// 83 PAY  80 

    0x9b07bbec,// 84 PAY  81 

    0xb4e71d5e,// 85 PAY  82 

    0x677f9ae1,// 86 PAY  83 

    0xa8aaf166,// 87 PAY  84 

    0x3d733474,// 88 PAY  85 

    0x2e896a27,// 89 PAY  86 

    0xc5961ff4,// 90 PAY  87 

    0x71eb172c,// 91 PAY  88 

    0xb5464e7a,// 92 PAY  89 

    0xb146254c,// 93 PAY  90 

    0x0be7167b,// 94 PAY  91 

    0xe1ca4925,// 95 PAY  92 

    0x31099856,// 96 PAY  93 

    0x037fc050,// 97 PAY  94 

    0x7b0c3de5,// 98 PAY  95 

    0x2d7cc408,// 99 PAY  96 

    0x2cec87d0,// 100 PAY  97 

    0xc0a81cff,// 101 PAY  98 

    0x77bff8fa,// 102 PAY  99 

    0x17c5a9c3,// 103 PAY 100 

    0xf08efc5f,// 104 PAY 101 

    0x5d630f2c,// 105 PAY 102 

    0x001bf8af,// 106 PAY 103 

    0xac37e72f,// 107 PAY 104 

    0x5e5c92f0,// 108 PAY 105 

    0x60c262ce,// 109 PAY 106 

    0x299c856a,// 110 PAY 107 

    0xde751b00,// 111 PAY 108 

    0xd781fa0c,// 112 PAY 109 

    0x2d225238,// 113 PAY 110 

    0x7d969539,// 114 PAY 111 

    0x311f0776,// 115 PAY 112 

    0x41cca1c5,// 116 PAY 113 

    0x66b1f3b2,// 117 PAY 114 

    0x00035f6c,// 118 PAY 115 

    0x3f40ae7d,// 119 PAY 116 

    0x8e5a2883,// 120 PAY 117 

    0x52a72a68,// 121 PAY 118 

    0x7e0448c2,// 122 PAY 119 

    0x1dbb6f54,// 123 PAY 120 

    0x87b8f252,// 124 PAY 121 

    0xdf74ccb5,// 125 PAY 122 

    0xf01aece0,// 126 PAY 123 

    0xce910edf,// 127 PAY 124 

    0xea7189e6,// 128 PAY 125 

    0x39fd8e71,// 129 PAY 126 

    0x8a68b291,// 130 PAY 127 

    0x051f4be6,// 131 PAY 128 

    0xb86e5100,// 132 PAY 129 

    0x58ee5a9d,// 133 PAY 130 

    0x7f5b3c13,// 134 PAY 131 

    0xe7ac4ae5,// 135 PAY 132 

    0xd30e998a,// 136 PAY 133 

    0xa28bc26b,// 137 PAY 134 

    0x4c89273a,// 138 PAY 135 

    0x697d7799,// 139 PAY 136 

    0x66ccb59f,// 140 PAY 137 

    0x312f8fee,// 141 PAY 138 

    0x43e14167,// 142 PAY 139 

    0x174c593f,// 143 PAY 140 

    0x9f6388fd,// 144 PAY 141 

    0x72518823,// 145 PAY 142 

    0x79b31a8e,// 146 PAY 143 

    0x39210a39,// 147 PAY 144 

    0xdd567df4,// 148 PAY 145 

    0xb3a723a0,// 149 PAY 146 

    0x60682bae,// 150 PAY 147 

    0x00104eac,// 151 PAY 148 

    0x15b7fe6b,// 152 PAY 149 

    0x27017cfb,// 153 PAY 150 

    0x344e1b63,// 154 PAY 151 

    0x287f0ccc,// 155 PAY 152 

    0x0488daa6,// 156 PAY 153 

    0xe36d59f5,// 157 PAY 154 

    0xe7f9e8d7,// 158 PAY 155 

    0x9d0431ff,// 159 PAY 156 

    0xf6eb883f,// 160 PAY 157 

    0x405d0907,// 161 PAY 158 

    0x35fa8d49,// 162 PAY 159 

    0xd8746729,// 163 PAY 160 

    0x30d8d99a,// 164 PAY 161 

    0x54ba66c8,// 165 PAY 162 

    0xfc5a4767,// 166 PAY 163 

    0xa06b2451,// 167 PAY 164 

    0xbf583abf,// 168 PAY 165 

    0x345b27f0,// 169 PAY 166 

    0x7e4a3484,// 170 PAY 167 

    0x998a76c6,// 171 PAY 168 

    0x4bc848e8,// 172 PAY 169 

    0x991442db,// 173 PAY 170 

    0xa6bf383f,// 174 PAY 171 

    0x4e1805d3,// 175 PAY 172 

    0x8035d3f0,// 176 PAY 173 

    0x12542c81,// 177 PAY 174 

    0x8ba1be5e,// 178 PAY 175 

    0xd38dd776,// 179 PAY 176 

    0x9b762f67,// 180 PAY 177 

    0x6e6c3a88,// 181 PAY 178 

    0x67c7cab5,// 182 PAY 179 

    0xb6fd3871,// 183 PAY 180 

    0x6578814f,// 184 PAY 181 

    0xb20a3ad9,// 185 PAY 182 

    0xa0538979,// 186 PAY 183 

    0x4bcae715,// 187 PAY 184 

    0xcf4206ff,// 188 PAY 185 

    0xa9650f4d,// 189 PAY 186 

    0x0be14807,// 190 PAY 187 

    0xb699add0,// 191 PAY 188 

    0x0c1370f0,// 192 PAY 189 

    0x0d657406,// 193 PAY 190 

    0xf66d4d56,// 194 PAY 191 

    0xd3811d15,// 195 PAY 192 

    0xb70bf9e5,// 196 PAY 193 

    0xc94f762f,// 197 PAY 194 

    0x39472537,// 198 PAY 195 

    0x5810b237,// 199 PAY 196 

    0x9d2dc83f,// 200 PAY 197 

    0xff723934,// 201 PAY 198 

    0x5ed0a9f8,// 202 PAY 199 

    0x77432a99,// 203 PAY 200 

    0x2a9cd6a7,// 204 PAY 201 

    0xddd5395f,// 205 PAY 202 

    0xf3491411,// 206 PAY 203 

    0x330bf069,// 207 PAY 204 

    0xa882fe58,// 208 PAY 205 

    0x6050c341,// 209 PAY 206 

    0x5b31081b,// 210 PAY 207 

    0x9dc60229,// 211 PAY 208 

    0xd0f74657,// 212 PAY 209 

    0xf6897922,// 213 PAY 210 

    0xef38af1e,// 214 PAY 211 

    0x606a2593,// 215 PAY 212 

    0xdac17db5,// 216 PAY 213 

    0x2a6d1b33,// 217 PAY 214 

    0x64246ac5,// 218 PAY 215 

    0x8509b214,// 219 PAY 216 

    0x654a4e17,// 220 PAY 217 

    0x7f8dae34,// 221 PAY 218 

    0x65958ff2,// 222 PAY 219 

    0x89547358,// 223 PAY 220 

    0xfe09d8aa,// 224 PAY 221 

    0xd528a256,// 225 PAY 222 

    0x9718ff95,// 226 PAY 223 

    0xe0f9c121,// 227 PAY 224 

    0x93a6513a,// 228 PAY 225 

    0x8c8c215b,// 229 PAY 226 

    0xfbdee910,// 230 PAY 227 

    0x6d5122db,// 231 PAY 228 

    0x8f41a388,// 232 PAY 229 

    0x8096c2c0,// 233 PAY 230 

    0x8c9a1303,// 234 PAY 231 

    0x6170da90,// 235 PAY 232 

    0x3f9f78cf,// 236 PAY 233 

    0x486865e7,// 237 PAY 234 

    0x5adb27dd,// 238 PAY 235 

    0xc79cf186,// 239 PAY 236 

    0x11cdfb8b,// 240 PAY 237 

    0xad661dd8,// 241 PAY 238 

    0x5936fc34,// 242 PAY 239 

    0x21caba46,// 243 PAY 240 

    0x984d4d32,// 244 PAY 241 

    0xf4e58cee,// 245 PAY 242 

    0x2ca1245d,// 246 PAY 243 

    0x2112a6db,// 247 PAY 244 

    0x22d35153,// 248 PAY 245 

    0x55c565a8,// 249 PAY 246 

    0xa683790d,// 250 PAY 247 

    0x20aa784f,// 251 PAY 248 

    0xbe4e042d,// 252 PAY 249 

    0x6e39826a,// 253 PAY 250 

    0xf59c3d85,// 254 PAY 251 

    0xf11e4f92,// 255 PAY 252 

    0x65987f81,// 256 PAY 253 

    0x581ed466,// 257 PAY 254 

    0x8422aff4,// 258 PAY 255 

    0x9ef77b90,// 259 PAY 256 

    0xd7031968,// 260 PAY 257 

    0xd9311edd,// 261 PAY 258 

    0x042e7a4a,// 262 PAY 259 

    0x40463ece,// 263 PAY 260 

    0xcef41032,// 264 PAY 261 

    0xda1635ec,// 265 PAY 262 

    0xf24b8892,// 266 PAY 263 

    0x969d2394,// 267 PAY 264 

    0x4f42805d,// 268 PAY 265 

    0xdd02cbce,// 269 PAY 266 

    0x323b4de8,// 270 PAY 267 

    0xc5bfe153,// 271 PAY 268 

    0x59677f53,// 272 PAY 269 

    0x1971272f,// 273 PAY 270 

    0xf888b2d1,// 274 PAY 271 

    0x549f0130,// 275 PAY 272 

    0xbc7bcc7d,// 276 PAY 273 

    0x06ee1434,// 277 PAY 274 

    0xb512cd0a,// 278 PAY 275 

    0xbd4975fc,// 279 PAY 276 

    0xc2b9ab15,// 280 PAY 277 

    0x821e9022,// 281 PAY 278 

    0x5b1f400f,// 282 PAY 279 

    0x70c6893a,// 283 PAY 280 

    0x073f3144,// 284 PAY 281 

    0x54ca4045,// 285 PAY 282 

    0xfcd56674,// 286 PAY 283 

    0xc9a789b3,// 287 PAY 284 

    0xa37f4f3d,// 288 PAY 285 

    0x1c9e10bb,// 289 PAY 286 

    0x0d70ac7c,// 290 PAY 287 

    0x148efef1,// 291 PAY 288 

    0x342ab194,// 292 PAY 289 

    0x80954eed,// 293 PAY 290 

    0x78ed8a02,// 294 PAY 291 

    0x44a30a2e,// 295 PAY 292 

    0x68f5c8b7,// 296 PAY 293 

    0x71f6f0ce,// 297 PAY 294 

    0xc2d3ef51,// 298 PAY 295 

    0x2ba69f54,// 299 PAY 296 

    0x7d0ffd8b,// 300 PAY 297 

    0x4e6d7823,// 301 PAY 298 

    0x2610d34c,// 302 PAY 299 

    0x02601f68,// 303 PAY 300 

    0x4ca8fdeb,// 304 PAY 301 

    0xa77d82fc,// 305 PAY 302 

    0x7a20d08f,// 306 PAY 303 

    0xb4292589,// 307 PAY 304 

    0x2c6e9e13,// 308 PAY 305 

    0xa4549ab8,// 309 PAY 306 

    0x2063c6fe,// 310 PAY 307 

    0xc9437072,// 311 PAY 308 

    0x79bf7a3c,// 312 PAY 309 

    0x55306965,// 313 PAY 310 

    0x45dc215c,// 314 PAY 311 

    0x8161ed64,// 315 PAY 312 

    0x944e062b,// 316 PAY 313 

    0x470b6d1f,// 317 PAY 314 

    0x5f657715,// 318 PAY 315 

    0xd3a2373b,// 319 PAY 316 

    0x06b183ef,// 320 PAY 317 

    0x533c4e55,// 321 PAY 318 

    0x45be2f2f,// 322 PAY 319 

    0xd7cd0f28,// 323 PAY 320 

    0xba0ede6b,// 324 PAY 321 

    0x6a10fbdd,// 325 PAY 322 

    0xb8c9391b,// 326 PAY 323 

    0x8697d6d3,// 327 PAY 324 

    0x58e25449,// 328 PAY 325 

    0xd81dff35,// 329 PAY 326 

    0xdbcef21b,// 330 PAY 327 

    0x91cb5027,// 331 PAY 328 

    0x99eb09a7,// 332 PAY 329 

    0x72d28bda,// 333 PAY 330 

    0xc01b35a9,// 334 PAY 331 

    0x466b5fb8,// 335 PAY 332 

    0x1340233f,// 336 PAY 333 

    0xd369f718,// 337 PAY 334 

    0x8654d8a8,// 338 PAY 335 

    0xbd8fab43,// 339 PAY 336 

    0x4e0bb355,// 340 PAY 337 

    0xbea09dd0,// 341 PAY 338 

    0xbb304586,// 342 PAY 339 

    0x66489f19,// 343 PAY 340 

    0xc424f656,// 344 PAY 341 

    0xf0cf3bc4,// 345 PAY 342 

    0x436067a8,// 346 PAY 343 

    0x239ba9a0,// 347 PAY 344 

    0x184fb7c7,// 348 PAY 345 

    0xcafa2806,// 349 PAY 346 

    0xf651463d,// 350 PAY 347 

    0xbd266a29,// 351 PAY 348 

    0x0adad4e6,// 352 PAY 349 

    0x0a708604,// 353 PAY 350 

    0xa388d84a,// 354 PAY 351 

    0x7f3e5ac9,// 355 PAY 352 

    0xa9235e3d,// 356 PAY 353 

    0x9f644766,// 357 PAY 354 

    0x3fc2f870,// 358 PAY 355 

    0x18b28e39,// 359 PAY 356 

    0xb24085fe,// 360 PAY 357 

    0x54ab9ec9,// 361 PAY 358 

    0xc1e42fcf,// 362 PAY 359 

    0xa7f5f78f,// 363 PAY 360 

    0xf4f68a20,// 364 PAY 361 

    0x4ff5258b,// 365 PAY 362 

    0x62f30785,// 366 PAY 363 

    0xfcbc59fd,// 367 PAY 364 

    0x9194732e,// 368 PAY 365 

    0x918d2312,// 369 PAY 366 

    0xd4071d25,// 370 PAY 367 

    0x4da1f516,// 371 PAY 368 

    0x5fda0fff,// 372 PAY 369 

    0x3d148ca7,// 373 PAY 370 

    0xb7af6243,// 374 PAY 371 

    0xea359f05,// 375 PAY 372 

    0xe6a71070,// 376 PAY 373 

    0x4423a654,// 377 PAY 374 

    0xb74fe1fa,// 378 PAY 375 

    0x5710f972,// 379 PAY 376 

    0x285060b8,// 380 PAY 377 

    0xc1f9cd9e,// 381 PAY 378 

    0x16fc114b,// 382 PAY 379 

    0x164f9424,// 383 PAY 380 

    0x7985c763,// 384 PAY 381 

    0x46be21d2,// 385 PAY 382 

    0x5050db73,// 386 PAY 383 

    0x06cf6147,// 387 PAY 384 

    0x09849986,// 388 PAY 385 

    0x27b9e083,// 389 PAY 386 

    0x4bc76509,// 390 PAY 387 

    0xbe5a9b0d,// 391 PAY 388 

    0x1c369eaa,// 392 PAY 389 

    0xa4daf679,// 393 PAY 390 

    0x0eac445c,// 394 PAY 391 

    0x77b6ba68,// 395 PAY 392 

    0x10d3926b,// 396 PAY 393 

    0x07302b3d,// 397 PAY 394 

    0x81298184,// 398 PAY 395 

    0xed67f93e,// 399 PAY 396 

    0x4701f122,// 400 PAY 397 

    0xb27b78c0,// 401 PAY 398 

    0x986950f1,// 402 PAY 399 

    0x9a1fdd3c,// 403 PAY 400 

    0x9ecc6b88,// 404 PAY 401 

    0xc0861cca,// 405 PAY 402 

    0xb41d00b8,// 406 PAY 403 

    0xce4d5378,// 407 PAY 404 

    0x28474d3d,// 408 PAY 405 

    0xba2055ed,// 409 PAY 406 

    0xe8d87572,// 410 PAY 407 

    0x78c9b237,// 411 PAY 408 

    0x658e870f,// 412 PAY 409 

    0xff4c38ec,// 413 PAY 410 

    0x139d72af,// 414 PAY 411 

    0x98788bfe,// 415 PAY 412 

    0x87e10df9,// 416 PAY 413 

    0xa8d1cbc6,// 417 PAY 414 

    0x66a220a2,// 418 PAY 415 

    0x5677a240,// 419 PAY 416 

    0xff3646d7,// 420 PAY 417 

    0xcf0d9166,// 421 PAY 418 

    0x707caea4,// 422 PAY 419 

    0x1e8c0253,// 423 PAY 420 

    0xc1bde8eb,// 424 PAY 421 

    0xa96ea542,// 425 PAY 422 

    0x8231719b,// 426 PAY 423 

    0xcd030d0f,// 427 PAY 424 

    0xe795b0c7,// 428 PAY 425 

    0xa6075789,// 429 PAY 426 

    0x66beb8ae,// 430 PAY 427 

    0x47bcb5ef,// 431 PAY 428 

    0xcaf348c5,// 432 PAY 429 

    0x186eab1f,// 433 PAY 430 

    0x9c912e80,// 434 PAY 431 

    0x66246597,// 435 PAY 432 

    0xcfef2446,// 436 PAY 433 

    0x054baf42,// 437 PAY 434 

    0x82ffcebe,// 438 PAY 435 

    0x5f89a266,// 439 PAY 436 

    0x2a70f066,// 440 PAY 437 

    0xd9bcd426,// 441 PAY 438 

    0x5695f76f,// 442 PAY 439 

    0x9d6b51a5,// 443 PAY 440 

    0x2f8436f1,// 444 PAY 441 

    0xf7c5c118,// 445 PAY 442 

    0x3a88f334,// 446 PAY 443 

    0xc06997b0,// 447 PAY 444 

    0xb844737b,// 448 PAY 445 

    0xba54da40,// 449 PAY 446 

    0x7209cdf7,// 450 PAY 447 

    0x522a1da1,// 451 PAY 448 

    0x6dcefe4e,// 452 PAY 449 

    0x103d0ef6,// 453 PAY 450 

    0xee836d98,// 454 PAY 451 

    0x3df71394,// 455 PAY 452 

    0xd6bb9731,// 456 PAY 453 

    0x9b3401ec,// 457 PAY 454 

    0x652c83f2,// 458 PAY 455 

    0xa3ae86af,// 459 PAY 456 

    0xc0494f58,// 460 PAY 457 

    0xcb4e46ae,// 461 PAY 458 

    0x3d2bcb5b,// 462 PAY 459 

    0x61000000,// 463 PAY 460 

/// HASH is  4 bytes 

    0x2d7cc408,// 464 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 76 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x0160804c // 465 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt23_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 445 words. 

/// BDA size     is 1774 (0x6ee) 

/// BDA id       is 0x6a15 

    0x06ee6a15,// 3 BDA   1 

/// PAY Generic Data size   : 1774 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xfb7e914a,// 4 PAY   1 

    0x8750fc1b,// 5 PAY   2 

    0x0927ab41,// 6 PAY   3 

    0x3d05a569,// 7 PAY   4 

    0x2d7c1328,// 8 PAY   5 

    0x421a6103,// 9 PAY   6 

    0xf6081755,// 10 PAY   7 

    0xa094fac1,// 11 PAY   8 

    0x59a35d1f,// 12 PAY   9 

    0x468196ec,// 13 PAY  10 

    0x7a55d59b,// 14 PAY  11 

    0x4236b34c,// 15 PAY  12 

    0xb94f0222,// 16 PAY  13 

    0xaebf68dd,// 17 PAY  14 

    0xdbec7095,// 18 PAY  15 

    0x7eeb0bcd,// 19 PAY  16 

    0x980a456b,// 20 PAY  17 

    0x90d376cb,// 21 PAY  18 

    0x425232ab,// 22 PAY  19 

    0x523b0438,// 23 PAY  20 

    0xef2afba6,// 24 PAY  21 

    0xc897185f,// 25 PAY  22 

    0x8d49c640,// 26 PAY  23 

    0x91aeaf96,// 27 PAY  24 

    0x9a6c9309,// 28 PAY  25 

    0x4ae59c01,// 29 PAY  26 

    0xfa44b06e,// 30 PAY  27 

    0x065ef240,// 31 PAY  28 

    0xb4dd1903,// 32 PAY  29 

    0x398b7495,// 33 PAY  30 

    0xeee99fd9,// 34 PAY  31 

    0x3f915444,// 35 PAY  32 

    0xb281ac3b,// 36 PAY  33 

    0xf552a68e,// 37 PAY  34 

    0x82e2b917,// 38 PAY  35 

    0x99ef5984,// 39 PAY  36 

    0xd9b2b7b3,// 40 PAY  37 

    0x3dca7901,// 41 PAY  38 

    0x106e4d6b,// 42 PAY  39 

    0x1c91d8b6,// 43 PAY  40 

    0xd2cb0d05,// 44 PAY  41 

    0xf6a94c36,// 45 PAY  42 

    0x3010113f,// 46 PAY  43 

    0xa25cd4d1,// 47 PAY  44 

    0xd9995a38,// 48 PAY  45 

    0x3265cfb7,// 49 PAY  46 

    0x45cb6abc,// 50 PAY  47 

    0x4781e9c4,// 51 PAY  48 

    0xda061f76,// 52 PAY  49 

    0x9c714204,// 53 PAY  50 

    0x22d7627b,// 54 PAY  51 

    0xa5fa5cb9,// 55 PAY  52 

    0x024a7f0b,// 56 PAY  53 

    0x78f4c81f,// 57 PAY  54 

    0xe4af633b,// 58 PAY  55 

    0x81dbf0f6,// 59 PAY  56 

    0x043690c0,// 60 PAY  57 

    0xb9698d91,// 61 PAY  58 

    0xd3fe479d,// 62 PAY  59 

    0xbccc9934,// 63 PAY  60 

    0x5bc19aef,// 64 PAY  61 

    0xe255565a,// 65 PAY  62 

    0xac0377a4,// 66 PAY  63 

    0xdf94f199,// 67 PAY  64 

    0xa50150b9,// 68 PAY  65 

    0x3b7c4af8,// 69 PAY  66 

    0xd430c7be,// 70 PAY  67 

    0xd7b2ab51,// 71 PAY  68 

    0x3089caa5,// 72 PAY  69 

    0x42a31fda,// 73 PAY  70 

    0x2e6a8785,// 74 PAY  71 

    0x024d4066,// 75 PAY  72 

    0x44c0e371,// 76 PAY  73 

    0xa348b09b,// 77 PAY  74 

    0x329b0097,// 78 PAY  75 

    0x9b37d6b4,// 79 PAY  76 

    0xec20e9d4,// 80 PAY  77 

    0x6a738d1d,// 81 PAY  78 

    0xac504161,// 82 PAY  79 

    0xe7ed335f,// 83 PAY  80 

    0x53265b64,// 84 PAY  81 

    0xae5326d1,// 85 PAY  82 

    0x55611060,// 86 PAY  83 

    0xe74c100f,// 87 PAY  84 

    0xacacdfa7,// 88 PAY  85 

    0xb61a0214,// 89 PAY  86 

    0x0df53f55,// 90 PAY  87 

    0xba32ed1b,// 91 PAY  88 

    0x911dd9d2,// 92 PAY  89 

    0xc7bb7b12,// 93 PAY  90 

    0x793d3604,// 94 PAY  91 

    0x937993ee,// 95 PAY  92 

    0x940935d4,// 96 PAY  93 

    0x87b034f8,// 97 PAY  94 

    0x04c2a2a5,// 98 PAY  95 

    0xfbddc1a1,// 99 PAY  96 

    0x77e7eaae,// 100 PAY  97 

    0xeec320b8,// 101 PAY  98 

    0x04c3e27f,// 102 PAY  99 

    0xef10717a,// 103 PAY 100 

    0x2d41e4fc,// 104 PAY 101 

    0x0d21d30a,// 105 PAY 102 

    0xfed52f98,// 106 PAY 103 

    0xd8e13b9f,// 107 PAY 104 

    0xcbb0827a,// 108 PAY 105 

    0xe4168ace,// 109 PAY 106 

    0x249e5ff0,// 110 PAY 107 

    0xd8f58074,// 111 PAY 108 

    0x3518e977,// 112 PAY 109 

    0x1fff12b1,// 113 PAY 110 

    0xa36e708c,// 114 PAY 111 

    0xf39d8a9a,// 115 PAY 112 

    0x8478ede1,// 116 PAY 113 

    0x979b72fd,// 117 PAY 114 

    0x0a721dd7,// 118 PAY 115 

    0x33021c82,// 119 PAY 116 

    0x3cfbb329,// 120 PAY 117 

    0xcff339e4,// 121 PAY 118 

    0xb6817c25,// 122 PAY 119 

    0x8ec3b40c,// 123 PAY 120 

    0x3418de15,// 124 PAY 121 

    0xa4eb2d85,// 125 PAY 122 

    0xed4d87da,// 126 PAY 123 

    0x2824a7ed,// 127 PAY 124 

    0xb16750d4,// 128 PAY 125 

    0xc6bf977b,// 129 PAY 126 

    0xe8fc17e9,// 130 PAY 127 

    0xaf424a74,// 131 PAY 128 

    0x227c7dc7,// 132 PAY 129 

    0x94277dfa,// 133 PAY 130 

    0x3bb3f5c6,// 134 PAY 131 

    0x6b78b90c,// 135 PAY 132 

    0xb215cb5c,// 136 PAY 133 

    0xb59d70d7,// 137 PAY 134 

    0x3031a928,// 138 PAY 135 

    0xd40085ea,// 139 PAY 136 

    0x5c4fb50d,// 140 PAY 137 

    0xe934483c,// 141 PAY 138 

    0xfa714208,// 142 PAY 139 

    0x51234e6b,// 143 PAY 140 

    0xb0962144,// 144 PAY 141 

    0xfb28edc0,// 145 PAY 142 

    0x14056aba,// 146 PAY 143 

    0xd7e50077,// 147 PAY 144 

    0x31078a99,// 148 PAY 145 

    0x63e54ab1,// 149 PAY 146 

    0x3665dbf0,// 150 PAY 147 

    0xaa34a9ad,// 151 PAY 148 

    0x96da86ab,// 152 PAY 149 

    0x8055b34b,// 153 PAY 150 

    0x72c9c293,// 154 PAY 151 

    0x09201872,// 155 PAY 152 

    0x1a18c2ee,// 156 PAY 153 

    0x55d621c9,// 157 PAY 154 

    0xfc6bf46f,// 158 PAY 155 

    0x6fa126bf,// 159 PAY 156 

    0xeff0a7fe,// 160 PAY 157 

    0x9811a7b6,// 161 PAY 158 

    0x3fcb2238,// 162 PAY 159 

    0x9fbdaaf5,// 163 PAY 160 

    0xd6e886c3,// 164 PAY 161 

    0x0b05a34e,// 165 PAY 162 

    0x5ab0e99f,// 166 PAY 163 

    0x54d24d38,// 167 PAY 164 

    0xfae6c167,// 168 PAY 165 

    0xcb9d76c7,// 169 PAY 166 

    0x8e516988,// 170 PAY 167 

    0xf74f7ecb,// 171 PAY 168 

    0xcd211be0,// 172 PAY 169 

    0x2c5729d7,// 173 PAY 170 

    0x572720d7,// 174 PAY 171 

    0x85241e36,// 175 PAY 172 

    0x17f1207f,// 176 PAY 173 

    0x9319910f,// 177 PAY 174 

    0xacf5eb08,// 178 PAY 175 

    0xec273d39,// 179 PAY 176 

    0xf0574aae,// 180 PAY 177 

    0xd03c4a38,// 181 PAY 178 

    0x6218fd7d,// 182 PAY 179 

    0xa04401db,// 183 PAY 180 

    0x25b1f1ef,// 184 PAY 181 

    0xb689a9db,// 185 PAY 182 

    0x5e3fb7c1,// 186 PAY 183 

    0x5d08c78d,// 187 PAY 184 

    0xd20ce41b,// 188 PAY 185 

    0xa2132961,// 189 PAY 186 

    0x3b247a3f,// 190 PAY 187 

    0x45b009d7,// 191 PAY 188 

    0xf2056654,// 192 PAY 189 

    0xe4b6ff03,// 193 PAY 190 

    0x8ef826f0,// 194 PAY 191 

    0x73536917,// 195 PAY 192 

    0x6eb81a81,// 196 PAY 193 

    0xbcf51d09,// 197 PAY 194 

    0xdd71e23c,// 198 PAY 195 

    0xa347089e,// 199 PAY 196 

    0xd60315f9,// 200 PAY 197 

    0xddaa4ee8,// 201 PAY 198 

    0x548d3a1d,// 202 PAY 199 

    0x33acfee9,// 203 PAY 200 

    0xce01b5b8,// 204 PAY 201 

    0x4189e585,// 205 PAY 202 

    0x391d986c,// 206 PAY 203 

    0xece17d76,// 207 PAY 204 

    0x1939b040,// 208 PAY 205 

    0xad74dc29,// 209 PAY 206 

    0x33a88e77,// 210 PAY 207 

    0x4deb6d46,// 211 PAY 208 

    0x9e837034,// 212 PAY 209 

    0xef951db5,// 213 PAY 210 

    0x58b7483f,// 214 PAY 211 

    0x5f046abb,// 215 PAY 212 

    0xa2bb1a4c,// 216 PAY 213 

    0x2001d1db,// 217 PAY 214 

    0x328285c5,// 218 PAY 215 

    0xd73b1f4c,// 219 PAY 216 

    0xf240fe26,// 220 PAY 217 

    0xb90fa952,// 221 PAY 218 

    0x1133378c,// 222 PAY 219 

    0x03c59853,// 223 PAY 220 

    0x1101c1f2,// 224 PAY 221 

    0x031bc586,// 225 PAY 222 

    0x9a3902e8,// 226 PAY 223 

    0xe1de58e1,// 227 PAY 224 

    0x8f674bc1,// 228 PAY 225 

    0xad773927,// 229 PAY 226 

    0x9812a1ba,// 230 PAY 227 

    0x281bd6bf,// 231 PAY 228 

    0xa371c20d,// 232 PAY 229 

    0x5c1b53a5,// 233 PAY 230 

    0xf5c98f5e,// 234 PAY 231 

    0x47e5a357,// 235 PAY 232 

    0x61f8be6a,// 236 PAY 233 

    0x5e50bce4,// 237 PAY 234 

    0xef8b509a,// 238 PAY 235 

    0x01d15288,// 239 PAY 236 

    0xa615f1e2,// 240 PAY 237 

    0x68227f64,// 241 PAY 238 

    0x394bf499,// 242 PAY 239 

    0xd194d805,// 243 PAY 240 

    0x8533bc1c,// 244 PAY 241 

    0x4228b3cf,// 245 PAY 242 

    0x098f465e,// 246 PAY 243 

    0xa49c2433,// 247 PAY 244 

    0xb2283cff,// 248 PAY 245 

    0xbae27ad7,// 249 PAY 246 

    0x88484346,// 250 PAY 247 

    0x9a0c97e3,// 251 PAY 248 

    0x96a31e49,// 252 PAY 249 

    0x4df5a0f7,// 253 PAY 250 

    0x65983918,// 254 PAY 251 

    0x33c56882,// 255 PAY 252 

    0xf93d361e,// 256 PAY 253 

    0x500d7da1,// 257 PAY 254 

    0x7747948f,// 258 PAY 255 

    0x1d65df07,// 259 PAY 256 

    0xc590f69f,// 260 PAY 257 

    0xbc1004c1,// 261 PAY 258 

    0x21c18ef4,// 262 PAY 259 

    0x5e503c0c,// 263 PAY 260 

    0xd326cf12,// 264 PAY 261 

    0xa3f67890,// 265 PAY 262 

    0x6e729656,// 266 PAY 263 

    0x8588b150,// 267 PAY 264 

    0x6016bcab,// 268 PAY 265 

    0x794ef5cc,// 269 PAY 266 

    0x97b91aea,// 270 PAY 267 

    0x3a10b4f4,// 271 PAY 268 

    0xe2dae316,// 272 PAY 269 

    0xd39aab33,// 273 PAY 270 

    0x497a03f8,// 274 PAY 271 

    0x4cdfb4f7,// 275 PAY 272 

    0x59353f96,// 276 PAY 273 

    0x8d5c3ecf,// 277 PAY 274 

    0x3fae3a35,// 278 PAY 275 

    0xedc07107,// 279 PAY 276 

    0x65cf38c3,// 280 PAY 277 

    0x1e3ab5db,// 281 PAY 278 

    0xc23bf410,// 282 PAY 279 

    0x42352653,// 283 PAY 280 

    0xb461cddf,// 284 PAY 281 

    0xe41662a8,// 285 PAY 282 

    0xe2b450ef,// 286 PAY 283 

    0x1022641b,// 287 PAY 284 

    0x2247b894,// 288 PAY 285 

    0x706fe526,// 289 PAY 286 

    0x4f406cb7,// 290 PAY 287 

    0x3f9318f1,// 291 PAY 288 

    0x6acd048b,// 292 PAY 289 

    0xff960593,// 293 PAY 290 

    0xcdf18764,// 294 PAY 291 

    0x08039bfb,// 295 PAY 292 

    0xa460696f,// 296 PAY 293 

    0xfb8c23e2,// 297 PAY 294 

    0xcf1ff5d0,// 298 PAY 295 

    0xe8f5faef,// 299 PAY 296 

    0x3f52a501,// 300 PAY 297 

    0x0de071b4,// 301 PAY 298 

    0x26ac9d77,// 302 PAY 299 

    0x3f44ab53,// 303 PAY 300 

    0xeb5b6f10,// 304 PAY 301 

    0x03918799,// 305 PAY 302 

    0xafe9d783,// 306 PAY 303 

    0xcf8cd12f,// 307 PAY 304 

    0xf4c20b1b,// 308 PAY 305 

    0x46ce4b55,// 309 PAY 306 

    0xb193bac5,// 310 PAY 307 

    0xb62717f7,// 311 PAY 308 

    0x4749e563,// 312 PAY 309 

    0xd25e515c,// 313 PAY 310 

    0xb7916a01,// 314 PAY 311 

    0x4b36a6de,// 315 PAY 312 

    0x041fd864,// 316 PAY 313 

    0x45d73490,// 317 PAY 314 

    0x5de24be8,// 318 PAY 315 

    0x7f9af954,// 319 PAY 316 

    0xbdab44e5,// 320 PAY 317 

    0xee39e7cf,// 321 PAY 318 

    0x7098484b,// 322 PAY 319 

    0x01555bfa,// 323 PAY 320 

    0xc353e3f2,// 324 PAY 321 

    0xd089fa40,// 325 PAY 322 

    0xaeb3b46c,// 326 PAY 323 

    0x8016e8c0,// 327 PAY 324 

    0x4b74ec67,// 328 PAY 325 

    0x12bcac98,// 329 PAY 326 

    0x9278e393,// 330 PAY 327 

    0xda129394,// 331 PAY 328 

    0xb9573c13,// 332 PAY 329 

    0x8ebc7df6,// 333 PAY 330 

    0x1b4bce85,// 334 PAY 331 

    0x3c23cf6a,// 335 PAY 332 

    0xffcc29b8,// 336 PAY 333 

    0xcbdab408,// 337 PAY 334 

    0xab97c315,// 338 PAY 335 

    0xee197b55,// 339 PAY 336 

    0x4b335809,// 340 PAY 337 

    0x1164ef09,// 341 PAY 338 

    0x53d51e15,// 342 PAY 339 

    0x4732e6ba,// 343 PAY 340 

    0xb957a53b,// 344 PAY 341 

    0x02de3bb3,// 345 PAY 342 

    0x4a1b6a9f,// 346 PAY 343 

    0xd800b5ca,// 347 PAY 344 

    0x315db820,// 348 PAY 345 

    0x6085ab2c,// 349 PAY 346 

    0x0d659379,// 350 PAY 347 

    0xc92fcce7,// 351 PAY 348 

    0x36dcf713,// 352 PAY 349 

    0x033f5032,// 353 PAY 350 

    0xe5bad5cb,// 354 PAY 351 

    0x708fb6cc,// 355 PAY 352 

    0x3f97cd87,// 356 PAY 353 

    0x76916767,// 357 PAY 354 

    0x97b0a7b1,// 358 PAY 355 

    0x5b5efd49,// 359 PAY 356 

    0x0eabe7bd,// 360 PAY 357 

    0xf0e28f93,// 361 PAY 358 

    0x48235287,// 362 PAY 359 

    0xaed7bd47,// 363 PAY 360 

    0xab4638ce,// 364 PAY 361 

    0xb0128278,// 365 PAY 362 

    0xb4e3f904,// 366 PAY 363 

    0x8d467898,// 367 PAY 364 

    0x58ae62f6,// 368 PAY 365 

    0x28e0c4da,// 369 PAY 366 

    0x653e777a,// 370 PAY 367 

    0xd419c0f9,// 371 PAY 368 

    0x450080a7,// 372 PAY 369 

    0x0f76de08,// 373 PAY 370 

    0x14d02477,// 374 PAY 371 

    0xce24404d,// 375 PAY 372 

    0xd80348b3,// 376 PAY 373 

    0x615fcea8,// 377 PAY 374 

    0x12ed6570,// 378 PAY 375 

    0x8cd149cd,// 379 PAY 376 

    0x2055679b,// 380 PAY 377 

    0xcf9443b0,// 381 PAY 378 

    0x42395be7,// 382 PAY 379 

    0xb72cff43,// 383 PAY 380 

    0x47b12f44,// 384 PAY 381 

    0x131d6239,// 385 PAY 382 

    0x1b023c88,// 386 PAY 383 

    0x4b452d0a,// 387 PAY 384 

    0x640e0998,// 388 PAY 385 

    0x5f0fbd6c,// 389 PAY 386 

    0x0343dc6a,// 390 PAY 387 

    0x9d170ea0,// 391 PAY 388 

    0xbb88d356,// 392 PAY 389 

    0x6a0bfb1e,// 393 PAY 390 

    0xb398eb75,// 394 PAY 391 

    0x8be03faa,// 395 PAY 392 

    0x3ecec434,// 396 PAY 393 

    0x88bc9e8d,// 397 PAY 394 

    0xe0e85bbf,// 398 PAY 395 

    0xb71c72b4,// 399 PAY 396 

    0x12133040,// 400 PAY 397 

    0xc47c66f6,// 401 PAY 398 

    0x4275928b,// 402 PAY 399 

    0x35b8fa3d,// 403 PAY 400 

    0x426d15c4,// 404 PAY 401 

    0xe465136d,// 405 PAY 402 

    0xa3342e03,// 406 PAY 403 

    0x80c97c56,// 407 PAY 404 

    0xec984124,// 408 PAY 405 

    0x357a77c4,// 409 PAY 406 

    0x7722ca98,// 410 PAY 407 

    0xf8731d08,// 411 PAY 408 

    0x0eb503fd,// 412 PAY 409 

    0x55f4563b,// 413 PAY 410 

    0x6c5a7659,// 414 PAY 411 

    0xc1a2fca8,// 415 PAY 412 

    0xf90ffde9,// 416 PAY 413 

    0x24390c75,// 417 PAY 414 

    0x5edd8d09,// 418 PAY 415 

    0x46f8fdf4,// 419 PAY 416 

    0x2586e127,// 420 PAY 417 

    0x6e9621c6,// 421 PAY 418 

    0xe37a62dc,// 422 PAY 419 

    0x66a4140d,// 423 PAY 420 

    0x0e1348e1,// 424 PAY 421 

    0x01b6fce2,// 425 PAY 422 

    0xd0b0fcfc,// 426 PAY 423 

    0x29ef1aa3,// 427 PAY 424 

    0x218286c3,// 428 PAY 425 

    0x4c76af81,// 429 PAY 426 

    0x674056f1,// 430 PAY 427 

    0xf83950e8,// 431 PAY 428 

    0x490a03ec,// 432 PAY 429 

    0x7f089cab,// 433 PAY 430 

    0xa41b2949,// 434 PAY 431 

    0x21e315a4,// 435 PAY 432 

    0x85179052,// 436 PAY 433 

    0x25b96bba,// 437 PAY 434 

    0x8e35cd31,// 438 PAY 435 

    0x289b1c23,// 439 PAY 436 

    0x0044e7af,// 440 PAY 437 

    0xc85df16b,// 441 PAY 438 

    0x6f1566e1,// 442 PAY 439 

    0x560309f7,// 443 PAY 440 

    0x9ef567ce,// 444 PAY 441 

    0xfddcee7c,// 445 PAY 442 

    0x8c3189fe,// 446 PAY 443 

    0xf4020000,// 447 PAY 444 

/// STA is 1 words. 

/// STA num_pkts       : 23 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd8 

    0x03d9d817 // 448 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt24_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 502 words. 

/// BDA size     is 2004 (0x7d4) 

/// BDA id       is 0x2a9b 

    0x07d42a9b,// 3 BDA   1 

/// PAY Generic Data size   : 2004 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xf92b91a6,// 4 PAY   1 

    0xd60010df,// 5 PAY   2 

    0xed4e230c,// 6 PAY   3 

    0xdee30f98,// 7 PAY   4 

    0x86d49486,// 8 PAY   5 

    0xbdcae232,// 9 PAY   6 

    0xbc77608f,// 10 PAY   7 

    0x58d52ab9,// 11 PAY   8 

    0xd7370ec7,// 12 PAY   9 

    0xcbc5053e,// 13 PAY  10 

    0xe1802f07,// 14 PAY  11 

    0x496bc812,// 15 PAY  12 

    0x148de4dd,// 16 PAY  13 

    0xbf7e795b,// 17 PAY  14 

    0xa4d2753a,// 18 PAY  15 

    0x2ca5c862,// 19 PAY  16 

    0xb71c8fb0,// 20 PAY  17 

    0xf614558a,// 21 PAY  18 

    0xaf9a73c4,// 22 PAY  19 

    0x1ff36370,// 23 PAY  20 

    0xfe7de949,// 24 PAY  21 

    0xef967870,// 25 PAY  22 

    0x5d8293e5,// 26 PAY  23 

    0xdaa4d0ed,// 27 PAY  24 

    0xa3ede4ed,// 28 PAY  25 

    0x34d6ce94,// 29 PAY  26 

    0xc0445f55,// 30 PAY  27 

    0x67db0668,// 31 PAY  28 

    0xf1624832,// 32 PAY  29 

    0x501c5058,// 33 PAY  30 

    0x39c5f79c,// 34 PAY  31 

    0x1109e017,// 35 PAY  32 

    0x5a1ec6ee,// 36 PAY  33 

    0x369d95ad,// 37 PAY  34 

    0x2d49be40,// 38 PAY  35 

    0x5f9c74ed,// 39 PAY  36 

    0xb043c7f2,// 40 PAY  37 

    0x44d85e8f,// 41 PAY  38 

    0x04ff5791,// 42 PAY  39 

    0xdb54db4a,// 43 PAY  40 

    0x84683829,// 44 PAY  41 

    0xc2bf8539,// 45 PAY  42 

    0x76a056e7,// 46 PAY  43 

    0xa46686dc,// 47 PAY  44 

    0x4fdf6530,// 48 PAY  45 

    0x669d73ff,// 49 PAY  46 

    0x6d8611c2,// 50 PAY  47 

    0xe0202df8,// 51 PAY  48 

    0xedb83a2a,// 52 PAY  49 

    0xab2ed3ab,// 53 PAY  50 

    0x356bfc49,// 54 PAY  51 

    0xe3e71992,// 55 PAY  52 

    0xfa4b0aee,// 56 PAY  53 

    0x296b43a0,// 57 PAY  54 

    0x3ed82008,// 58 PAY  55 

    0xad64e42f,// 59 PAY  56 

    0xde826f67,// 60 PAY  57 

    0x89feeb58,// 61 PAY  58 

    0x82ec627a,// 62 PAY  59 

    0x03af211c,// 63 PAY  60 

    0x0d992ce1,// 64 PAY  61 

    0xe3a903f2,// 65 PAY  62 

    0xcd9ce720,// 66 PAY  63 

    0x5e642533,// 67 PAY  64 

    0x6ac930dc,// 68 PAY  65 

    0x372c5833,// 69 PAY  66 

    0xe5f7e2c4,// 70 PAY  67 

    0x78e662f3,// 71 PAY  68 

    0xb20ac4bb,// 72 PAY  69 

    0x28b671d0,// 73 PAY  70 

    0x9764febd,// 74 PAY  71 

    0xab4e3a7e,// 75 PAY  72 

    0xbe2309a4,// 76 PAY  73 

    0x9319950d,// 77 PAY  74 

    0x9067a976,// 78 PAY  75 

    0xe332ed3f,// 79 PAY  76 

    0x00925daa,// 80 PAY  77 

    0x3500e2d8,// 81 PAY  78 

    0x4d4b4bb4,// 82 PAY  79 

    0x7bca1e2e,// 83 PAY  80 

    0xf81317f1,// 84 PAY  81 

    0x228a9dc0,// 85 PAY  82 

    0x2f2e290f,// 86 PAY  83 

    0x0c3f0465,// 87 PAY  84 

    0x9c816185,// 88 PAY  85 

    0x76ef3291,// 89 PAY  86 

    0xcb524dcc,// 90 PAY  87 

    0x58a94e24,// 91 PAY  88 

    0xcbf6cf59,// 92 PAY  89 

    0x2e44ebf8,// 93 PAY  90 

    0x2ed209ea,// 94 PAY  91 

    0x49eb1917,// 95 PAY  92 

    0x7841a5a4,// 96 PAY  93 

    0x5bedfcb2,// 97 PAY  94 

    0x2db7683e,// 98 PAY  95 

    0xf59a2dd8,// 99 PAY  96 

    0xaf37d5a5,// 100 PAY  97 

    0x2d0ce308,// 101 PAY  98 

    0x527a9320,// 102 PAY  99 

    0x73b8c864,// 103 PAY 100 

    0x4f9464e0,// 104 PAY 101 

    0xf27c60ff,// 105 PAY 102 

    0xf7b85b63,// 106 PAY 103 

    0x352dd401,// 107 PAY 104 

    0x614a17b3,// 108 PAY 105 

    0xd4bfdfe5,// 109 PAY 106 

    0x6fc92e8e,// 110 PAY 107 

    0xda538c7d,// 111 PAY 108 

    0x1425cd9e,// 112 PAY 109 

    0x69e8f6cb,// 113 PAY 110 

    0x7bbc0d8e,// 114 PAY 111 

    0x8c378b8b,// 115 PAY 112 

    0x671aa0cb,// 116 PAY 113 

    0x33880aff,// 117 PAY 114 

    0x4327f49d,// 118 PAY 115 

    0x3df71093,// 119 PAY 116 

    0x54f1c138,// 120 PAY 117 

    0x2bf61121,// 121 PAY 118 

    0xdc8b2b6a,// 122 PAY 119 

    0x73bc093e,// 123 PAY 120 

    0xae9d0693,// 124 PAY 121 

    0x16af64c4,// 125 PAY 122 

    0xe18e1af4,// 126 PAY 123 

    0x9aea380b,// 127 PAY 124 

    0x9868d705,// 128 PAY 125 

    0xa9a0d7e3,// 129 PAY 126 

    0xe1fb86d8,// 130 PAY 127 

    0x343454a0,// 131 PAY 128 

    0x00e4363b,// 132 PAY 129 

    0x3bdf14ed,// 133 PAY 130 

    0x05ead44b,// 134 PAY 131 

    0x027c2938,// 135 PAY 132 

    0xee210b2e,// 136 PAY 133 

    0x91cd5ff4,// 137 PAY 134 

    0x3b74e469,// 138 PAY 135 

    0x6d8afd4f,// 139 PAY 136 

    0x4f6e5f43,// 140 PAY 137 

    0x96cbc1ef,// 141 PAY 138 

    0x24f16789,// 142 PAY 139 

    0x91652e65,// 143 PAY 140 

    0xceb360a4,// 144 PAY 141 

    0xa0770ee5,// 145 PAY 142 

    0x0403f0d4,// 146 PAY 143 

    0x558315c4,// 147 PAY 144 

    0x583d3d6e,// 148 PAY 145 

    0xf38285ab,// 149 PAY 146 

    0x2722b3b2,// 150 PAY 147 

    0x9022eaac,// 151 PAY 148 

    0xebedbd1a,// 152 PAY 149 

    0xa7c2c18b,// 153 PAY 150 

    0x4b55ee4c,// 154 PAY 151 

    0xfb872784,// 155 PAY 152 

    0xda03cbe5,// 156 PAY 153 

    0x82856583,// 157 PAY 154 

    0x1a78503b,// 158 PAY 155 

    0x47a2a654,// 159 PAY 156 

    0x766f965f,// 160 PAY 157 

    0xd7898c7e,// 161 PAY 158 

    0xf58e245a,// 162 PAY 159 

    0x2f1ed373,// 163 PAY 160 

    0x86f35c3d,// 164 PAY 161 

    0xfcb22a49,// 165 PAY 162 

    0x86d72692,// 166 PAY 163 

    0xeafc60db,// 167 PAY 164 

    0xafe7f4d6,// 168 PAY 165 

    0x9c806667,// 169 PAY 166 

    0x215cd579,// 170 PAY 167 

    0x8d6f58bc,// 171 PAY 168 

    0x4724512c,// 172 PAY 169 

    0x4a8bd6c8,// 173 PAY 170 

    0xd07247e1,// 174 PAY 171 

    0x64b616fe,// 175 PAY 172 

    0x2f10d4af,// 176 PAY 173 

    0x6ad629d3,// 177 PAY 174 

    0xbc3d305c,// 178 PAY 175 

    0x98829e69,// 179 PAY 176 

    0xd4762a42,// 180 PAY 177 

    0x2184da8f,// 181 PAY 178 

    0x1a3eb327,// 182 PAY 179 

    0x22c68361,// 183 PAY 180 

    0xf0170121,// 184 PAY 181 

    0x00ba7401,// 185 PAY 182 

    0xf7ebfd3e,// 186 PAY 183 

    0x2f12af7e,// 187 PAY 184 

    0x04882835,// 188 PAY 185 

    0xb7935d08,// 189 PAY 186 

    0xb82bacb4,// 190 PAY 187 

    0x9bb4d163,// 191 PAY 188 

    0x79dfb8fd,// 192 PAY 189 

    0x55d600c9,// 193 PAY 190 

    0xd995cf71,// 194 PAY 191 

    0xd317307c,// 195 PAY 192 

    0x848e1d3b,// 196 PAY 193 

    0x598bc1a5,// 197 PAY 194 

    0x7d737d01,// 198 PAY 195 

    0x302fe7cc,// 199 PAY 196 

    0x121e3b87,// 200 PAY 197 

    0x09184172,// 201 PAY 198 

    0x42d89f5a,// 202 PAY 199 

    0x288f0ca4,// 203 PAY 200 

    0x7fdff556,// 204 PAY 201 

    0x639dbe1b,// 205 PAY 202 

    0x3571583c,// 206 PAY 203 

    0xe07c3a96,// 207 PAY 204 

    0xe297f4b9,// 208 PAY 205 

    0x820cfd3e,// 209 PAY 206 

    0x01059d53,// 210 PAY 207 

    0x361fdc67,// 211 PAY 208 

    0xcb1a5fb0,// 212 PAY 209 

    0x5a5d6709,// 213 PAY 210 

    0x405e9c78,// 214 PAY 211 

    0x1929046b,// 215 PAY 212 

    0x24762d47,// 216 PAY 213 

    0x7ce8c9e9,// 217 PAY 214 

    0xdc6b6ffb,// 218 PAY 215 

    0xe18a6fba,// 219 PAY 216 

    0xad6b8018,// 220 PAY 217 

    0x5b715b71,// 221 PAY 218 

    0x807f3089,// 222 PAY 219 

    0xc42d3ee3,// 223 PAY 220 

    0xfb553a7a,// 224 PAY 221 

    0xca9c59bc,// 225 PAY 222 

    0x13885e0d,// 226 PAY 223 

    0xc7c51533,// 227 PAY 224 

    0x0f15015a,// 228 PAY 225 

    0x8f8dfc0b,// 229 PAY 226 

    0xfa3aed2f,// 230 PAY 227 

    0x3112277d,// 231 PAY 228 

    0x596f70c1,// 232 PAY 229 

    0x10326c58,// 233 PAY 230 

    0x9c933db4,// 234 PAY 231 

    0x1eb0a115,// 235 PAY 232 

    0xa508477a,// 236 PAY 233 

    0x6d776fb9,// 237 PAY 234 

    0x712836c4,// 238 PAY 235 

    0x03ab5d77,// 239 PAY 236 

    0x310bbe49,// 240 PAY 237 

    0x257e3bf8,// 241 PAY 238 

    0x0a5bc5f5,// 242 PAY 239 

    0xa8c9678a,// 243 PAY 240 

    0xfcfd89a4,// 244 PAY 241 

    0x728d0738,// 245 PAY 242 

    0x3d57f376,// 246 PAY 243 

    0x5946c3be,// 247 PAY 244 

    0xb51b4fc9,// 248 PAY 245 

    0x98ac4917,// 249 PAY 246 

    0x2903b967,// 250 PAY 247 

    0xdd3da8a7,// 251 PAY 248 

    0x54296891,// 252 PAY 249 

    0xaff94688,// 253 PAY 250 

    0xb0ae5696,// 254 PAY 251 

    0xddaef993,// 255 PAY 252 

    0x78d248b9,// 256 PAY 253 

    0x1992a810,// 257 PAY 254 

    0xb366b5b4,// 258 PAY 255 

    0x325802e5,// 259 PAY 256 

    0x348fa571,// 260 PAY 257 

    0x0ed7d0b0,// 261 PAY 258 

    0x7ee6244e,// 262 PAY 259 

    0x46caf135,// 263 PAY 260 

    0x452fdbae,// 264 PAY 261 

    0x2bc2dcff,// 265 PAY 262 

    0x4a493acb,// 266 PAY 263 

    0x4678bbf7,// 267 PAY 264 

    0x4e25fd57,// 268 PAY 265 

    0x7b66335d,// 269 PAY 266 

    0x405d3ff4,// 270 PAY 267 

    0x7d5bb931,// 271 PAY 268 

    0x52ef9a76,// 272 PAY 269 

    0x8181b40b,// 273 PAY 270 

    0xff287703,// 274 PAY 271 

    0x478903c7,// 275 PAY 272 

    0x5d15cf24,// 276 PAY 273 

    0xbad10cd5,// 277 PAY 274 

    0xd0ab45b8,// 278 PAY 275 

    0xab37ff49,// 279 PAY 276 

    0xbd7ed0b0,// 280 PAY 277 

    0xf8e68554,// 281 PAY 278 

    0x099f9165,// 282 PAY 279 

    0xada0d06b,// 283 PAY 280 

    0x15146088,// 284 PAY 281 

    0x05966173,// 285 PAY 282 

    0x3560d1ae,// 286 PAY 283 

    0x67dc6ff0,// 287 PAY 284 

    0x7c56a6c0,// 288 PAY 285 

    0x8fc5a114,// 289 PAY 286 

    0x17515eb7,// 290 PAY 287 

    0xfeec1beb,// 291 PAY 288 

    0xbf299699,// 292 PAY 289 

    0xc9ea6a31,// 293 PAY 290 

    0x88399cc3,// 294 PAY 291 

    0x8d12dcba,// 295 PAY 292 

    0x485b9102,// 296 PAY 293 

    0x59d4a077,// 297 PAY 294 

    0xeebae2cb,// 298 PAY 295 

    0x7b63162b,// 299 PAY 296 

    0xeac27ead,// 300 PAY 297 

    0x5d5ef659,// 301 PAY 298 

    0xdc9588d3,// 302 PAY 299 

    0x20658942,// 303 PAY 300 

    0xa5ee238b,// 304 PAY 301 

    0x2f54233d,// 305 PAY 302 

    0x00d0d763,// 306 PAY 303 

    0xbfcf39b9,// 307 PAY 304 

    0xd40d860b,// 308 PAY 305 

    0x97dbe337,// 309 PAY 306 

    0x75585dd4,// 310 PAY 307 

    0x65d2925f,// 311 PAY 308 

    0xe82a1205,// 312 PAY 309 

    0x7ece700f,// 313 PAY 310 

    0x8ef99c19,// 314 PAY 311 

    0x7ff91348,// 315 PAY 312 

    0xcdc15188,// 316 PAY 313 

    0xd01b9e8a,// 317 PAY 314 

    0x853f9ee2,// 318 PAY 315 

    0x78d1adc2,// 319 PAY 316 

    0xa7b8b4b8,// 320 PAY 317 

    0x2d2b829e,// 321 PAY 318 

    0x4b56e197,// 322 PAY 319 

    0x84101720,// 323 PAY 320 

    0x81fe67ea,// 324 PAY 321 

    0xc125a5d1,// 325 PAY 322 

    0x02170a79,// 326 PAY 323 

    0x5630957c,// 327 PAY 324 

    0xd41db11f,// 328 PAY 325 

    0x62e5e3d2,// 329 PAY 326 

    0x72644d32,// 330 PAY 327 

    0x32000dbd,// 331 PAY 328 

    0x30db2b89,// 332 PAY 329 

    0xdbb70e89,// 333 PAY 330 

    0x50aa89d5,// 334 PAY 331 

    0x64a8c93c,// 335 PAY 332 

    0x01cc3b5f,// 336 PAY 333 

    0x271e3a6a,// 337 PAY 334 

    0x11cca92c,// 338 PAY 335 

    0xd1fddc23,// 339 PAY 336 

    0x26774f94,// 340 PAY 337 

    0x0b6a2abd,// 341 PAY 338 

    0x848ee653,// 342 PAY 339 

    0xf38aca30,// 343 PAY 340 

    0xe849ef25,// 344 PAY 341 

    0x41a435d3,// 345 PAY 342 

    0xb16b02e6,// 346 PAY 343 

    0x6a512d11,// 347 PAY 344 

    0x50280ab2,// 348 PAY 345 

    0x4a8bd055,// 349 PAY 346 

    0x2c9cea0c,// 350 PAY 347 

    0x15b87974,// 351 PAY 348 

    0x4112bb70,// 352 PAY 349 

    0xdad1b981,// 353 PAY 350 

    0x69dd6297,// 354 PAY 351 

    0x34f44ac0,// 355 PAY 352 

    0xb885de89,// 356 PAY 353 

    0xf7e2b814,// 357 PAY 354 

    0xedc88d47,// 358 PAY 355 

    0x86ec3fd2,// 359 PAY 356 

    0xd8bb14da,// 360 PAY 357 

    0x1c82bc3d,// 361 PAY 358 

    0x16deb6d1,// 362 PAY 359 

    0x77ae7447,// 363 PAY 360 

    0xdb56c50e,// 364 PAY 361 

    0xe470bc4c,// 365 PAY 362 

    0xf9a364a8,// 366 PAY 363 

    0xe39c34ea,// 367 PAY 364 

    0x33485f7e,// 368 PAY 365 

    0xcd35b6a6,// 369 PAY 366 

    0xaaf19105,// 370 PAY 367 

    0xc6fba157,// 371 PAY 368 

    0xbb6ce09a,// 372 PAY 369 

    0x33be40dd,// 373 PAY 370 

    0x219c9723,// 374 PAY 371 

    0x6308d809,// 375 PAY 372 

    0xe5854b81,// 376 PAY 373 

    0xf895c446,// 377 PAY 374 

    0xcf58c231,// 378 PAY 375 

    0xa8440e46,// 379 PAY 376 

    0x7be644e0,// 380 PAY 377 

    0xc4e5f972,// 381 PAY 378 

    0x5de6cd57,// 382 PAY 379 

    0x0a680450,// 383 PAY 380 

    0xc727eddf,// 384 PAY 381 

    0x55edc35b,// 385 PAY 382 

    0xc22aa246,// 386 PAY 383 

    0xa7c7b89f,// 387 PAY 384 

    0xc750e0af,// 388 PAY 385 

    0xac88d53b,// 389 PAY 386 

    0xd6e73fa6,// 390 PAY 387 

    0xa171c969,// 391 PAY 388 

    0x8728118c,// 392 PAY 389 

    0xdd8892f8,// 393 PAY 390 

    0x78d85177,// 394 PAY 391 

    0x7b080847,// 395 PAY 392 

    0x19896e5b,// 396 PAY 393 

    0xda652b30,// 397 PAY 394 

    0xfd72cbfc,// 398 PAY 395 

    0x03431cde,// 399 PAY 396 

    0x6eafe33b,// 400 PAY 397 

    0x914c6ca3,// 401 PAY 398 

    0xffe7c3fa,// 402 PAY 399 

    0x74bf169a,// 403 PAY 400 

    0x28d113d8,// 404 PAY 401 

    0xf724094e,// 405 PAY 402 

    0x701c1b89,// 406 PAY 403 

    0x5a330fca,// 407 PAY 404 

    0x1359052e,// 408 PAY 405 

    0x391d179e,// 409 PAY 406 

    0xd43e1090,// 410 PAY 407 

    0xfe5a8e90,// 411 PAY 408 

    0x916b5b27,// 412 PAY 409 

    0x607a1342,// 413 PAY 410 

    0xb550c813,// 414 PAY 411 

    0x1276f691,// 415 PAY 412 

    0x5c7000de,// 416 PAY 413 

    0x71609395,// 417 PAY 414 

    0x6c64867b,// 418 PAY 415 

    0x5bb6228d,// 419 PAY 416 

    0xc8c86762,// 420 PAY 417 

    0x2ca14855,// 421 PAY 418 

    0x7a722ff4,// 422 PAY 419 

    0xad73e42f,// 423 PAY 420 

    0xbcc47455,// 424 PAY 421 

    0xcdf18a40,// 425 PAY 422 

    0xd3f62183,// 426 PAY 423 

    0x92775984,// 427 PAY 424 

    0xe34236bb,// 428 PAY 425 

    0xb44a5934,// 429 PAY 426 

    0x71173019,// 430 PAY 427 

    0x84e46f87,// 431 PAY 428 

    0x5000fe1f,// 432 PAY 429 

    0x794ba69d,// 433 PAY 430 

    0x9d8b4f8f,// 434 PAY 431 

    0x5b108b2e,// 435 PAY 432 

    0x186ae61f,// 436 PAY 433 

    0x12a9c891,// 437 PAY 434 

    0xa3304bf1,// 438 PAY 435 

    0xb890593b,// 439 PAY 436 

    0x4576f216,// 440 PAY 437 

    0xc60bb5d7,// 441 PAY 438 

    0xd9db3d56,// 442 PAY 439 

    0xcf73be06,// 443 PAY 440 

    0x0561875d,// 444 PAY 441 

    0x37b2baaa,// 445 PAY 442 

    0x2093dac6,// 446 PAY 443 

    0x1be8651f,// 447 PAY 444 

    0xfc1ffd3c,// 448 PAY 445 

    0x5b34902d,// 449 PAY 446 

    0xd27c4575,// 450 PAY 447 

    0xa5a69a35,// 451 PAY 448 

    0x48a09f4d,// 452 PAY 449 

    0x6e6f1b1b,// 453 PAY 450 

    0xb5bfacbe,// 454 PAY 451 

    0xafa63d08,// 455 PAY 452 

    0xbeddd132,// 456 PAY 453 

    0x9c5a6580,// 457 PAY 454 

    0x2473f568,// 458 PAY 455 

    0x490a6dcf,// 459 PAY 456 

    0xd04eac19,// 460 PAY 457 

    0xb859904f,// 461 PAY 458 

    0x6d808980,// 462 PAY 459 

    0xe288deda,// 463 PAY 460 

    0x0b3f9da0,// 464 PAY 461 

    0x8166e540,// 465 PAY 462 

    0xce50acb0,// 466 PAY 463 

    0xc98a92fc,// 467 PAY 464 

    0x86fe576c,// 468 PAY 465 

    0x5ae172ca,// 469 PAY 466 

    0x639282a2,// 470 PAY 467 

    0xbdd61997,// 471 PAY 468 

    0x87d9493d,// 472 PAY 469 

    0xedde6002,// 473 PAY 470 

    0xda19474c,// 474 PAY 471 

    0xf468656b,// 475 PAY 472 

    0xc8c807bb,// 476 PAY 473 

    0xf4dedd31,// 477 PAY 474 

    0x0463914b,// 478 PAY 475 

    0x48e8d065,// 479 PAY 476 

    0x88828adc,// 480 PAY 477 

    0x565a1592,// 481 PAY 478 

    0x990dc53c,// 482 PAY 479 

    0xb3f40b41,// 483 PAY 480 

    0xf66e05e7,// 484 PAY 481 

    0x171cc561,// 485 PAY 482 

    0x9a9d5778,// 486 PAY 483 

    0xfbe60dec,// 487 PAY 484 

    0xc5d524d4,// 488 PAY 485 

    0xf15497df,// 489 PAY 486 

    0x5f980a1c,// 490 PAY 487 

    0x3096b2ab,// 491 PAY 488 

    0x531fbcdf,// 492 PAY 489 

    0x44cf542f,// 493 PAY 490 

    0x388f4d04,// 494 PAY 491 

    0x1037a2a3,// 495 PAY 492 

    0x466ba3c2,// 496 PAY 493 

    0xef23b37e,// 497 PAY 494 

    0xf63f4fca,// 498 PAY 495 

    0x7635a31c,// 499 PAY 496 

    0x2a59b677,// 500 PAY 497 

    0x4f573ebe,// 501 PAY 498 

    0x6dd4a533,// 502 PAY 499 

    0x68d5ecd2,// 503 PAY 500 

    0x830ea1d3,// 504 PAY 501 

/// STA is 1 words. 

/// STA num_pkts       : 151 

/// STA pkt_idx        : 156 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02703c97 // 505 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt25_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 288 words. 

/// BDA size     is 1147 (0x47b) 

/// BDA id       is 0xdca0 

    0x047bdca0,// 3 BDA   1 

/// PAY Generic Data size   : 1147 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x7df4b163,// 4 PAY   1 

    0xe1c45dcf,// 5 PAY   2 

    0xe70ac6e7,// 6 PAY   3 

    0x91839d07,// 7 PAY   4 

    0x5f81d2a1,// 8 PAY   5 

    0x783e7f31,// 9 PAY   6 

    0x6c658c0e,// 10 PAY   7 

    0xc406c6cd,// 11 PAY   8 

    0x1a91b47f,// 12 PAY   9 

    0x6b78b8eb,// 13 PAY  10 

    0xf3d7aa76,// 14 PAY  11 

    0x86f91cb5,// 15 PAY  12 

    0x18e78945,// 16 PAY  13 

    0x5af88aa7,// 17 PAY  14 

    0x46d6725d,// 18 PAY  15 

    0x680e9781,// 19 PAY  16 

    0x16d2277f,// 20 PAY  17 

    0x33ce8ab4,// 21 PAY  18 

    0x23682f7b,// 22 PAY  19 

    0x3014de08,// 23 PAY  20 

    0xfc9f244c,// 24 PAY  21 

    0xd3b5354c,// 25 PAY  22 

    0x4f127a5a,// 26 PAY  23 

    0xff0f2eb3,// 27 PAY  24 

    0xb45eac89,// 28 PAY  25 

    0xc311ee26,// 29 PAY  26 

    0x07ec4d17,// 30 PAY  27 

    0x101c40ca,// 31 PAY  28 

    0xb3b6b91c,// 32 PAY  29 

    0xdb5a98e7,// 33 PAY  30 

    0xd4e5c62f,// 34 PAY  31 

    0xe91b5591,// 35 PAY  32 

    0xd25bdff0,// 36 PAY  33 

    0x564ce966,// 37 PAY  34 

    0x0d0e8238,// 38 PAY  35 

    0x1b3ac0cf,// 39 PAY  36 

    0x023707ec,// 40 PAY  37 

    0x3a74445f,// 41 PAY  38 

    0xeb55e2a0,// 42 PAY  39 

    0xeb1ddbd3,// 43 PAY  40 

    0xccf20373,// 44 PAY  41 

    0x85ed934e,// 45 PAY  42 

    0xe0dc6991,// 46 PAY  43 

    0xa2ca006c,// 47 PAY  44 

    0xc4e8832d,// 48 PAY  45 

    0x4fa6f474,// 49 PAY  46 

    0x8cea69d6,// 50 PAY  47 

    0xdc2b653b,// 51 PAY  48 

    0x48624bf7,// 52 PAY  49 

    0x0f13c924,// 53 PAY  50 

    0xdbdbdc0c,// 54 PAY  51 

    0x25e62a55,// 55 PAY  52 

    0xcbcc03a7,// 56 PAY  53 

    0xb78e50e9,// 57 PAY  54 

    0xd0be1a02,// 58 PAY  55 

    0x79911d91,// 59 PAY  56 

    0x7b5fcf3f,// 60 PAY  57 

    0x741cc1fc,// 61 PAY  58 

    0x9fc50eaf,// 62 PAY  59 

    0xe0b03a9c,// 63 PAY  60 

    0x11f81e41,// 64 PAY  61 

    0x70b3da49,// 65 PAY  62 

    0xdb9a28ce,// 66 PAY  63 

    0x0f61c683,// 67 PAY  64 

    0x53d0b85f,// 68 PAY  65 

    0x6b87fb55,// 69 PAY  66 

    0x4511fcf9,// 70 PAY  67 

    0x32d7649f,// 71 PAY  68 

    0xe8f6140a,// 72 PAY  69 

    0xa172cbb9,// 73 PAY  70 

    0x26dad97a,// 74 PAY  71 

    0x1927ca30,// 75 PAY  72 

    0x5cd413ec,// 76 PAY  73 

    0xe9213ba5,// 77 PAY  74 

    0x5c2af6b2,// 78 PAY  75 

    0x7d73dc82,// 79 PAY  76 

    0xa765d130,// 80 PAY  77 

    0x0ed7bcc1,// 81 PAY  78 

    0x33e1c1c7,// 82 PAY  79 

    0xa7e41548,// 83 PAY  80 

    0x0f0080ba,// 84 PAY  81 

    0x7dc32ee4,// 85 PAY  82 

    0x358d2acb,// 86 PAY  83 

    0x395692b5,// 87 PAY  84 

    0xd9827350,// 88 PAY  85 

    0x5a0aa4bf,// 89 PAY  86 

    0x83873264,// 90 PAY  87 

    0xf7eb1eec,// 91 PAY  88 

    0xf2cdb23c,// 92 PAY  89 

    0x37c2992c,// 93 PAY  90 

    0x0352037d,// 94 PAY  91 

    0x2825a077,// 95 PAY  92 

    0x76a028d1,// 96 PAY  93 

    0x551f8239,// 97 PAY  94 

    0x9d982f34,// 98 PAY  95 

    0x2aba992a,// 99 PAY  96 

    0x9e051cfe,// 100 PAY  97 

    0xc0c87811,// 101 PAY  98 

    0x3b49727d,// 102 PAY  99 

    0x3be1785f,// 103 PAY 100 

    0xa472e933,// 104 PAY 101 

    0x68bb7fa5,// 105 PAY 102 

    0xf03f21ce,// 106 PAY 103 

    0x41a9a3dc,// 107 PAY 104 

    0xab506eb6,// 108 PAY 105 

    0x4aa7aa0d,// 109 PAY 106 

    0xd8eab594,// 110 PAY 107 

    0xbc9160f5,// 111 PAY 108 

    0x67fa8fac,// 112 PAY 109 

    0xe61eea09,// 113 PAY 110 

    0x7b7bbc15,// 114 PAY 111 

    0x73d64f9a,// 115 PAY 112 

    0x75191bb2,// 116 PAY 113 

    0x91fd0bfb,// 117 PAY 114 

    0x7b17f556,// 118 PAY 115 

    0xc9c77013,// 119 PAY 116 

    0xbdb6ffa4,// 120 PAY 117 

    0x6ccc5889,// 121 PAY 118 

    0xd232d7ec,// 122 PAY 119 

    0x71dc7b16,// 123 PAY 120 

    0x6f979eb0,// 124 PAY 121 

    0xea6910db,// 125 PAY 122 

    0x74d2f3e2,// 126 PAY 123 

    0xed53b146,// 127 PAY 124 

    0x4ea9856a,// 128 PAY 125 

    0x6ff746f7,// 129 PAY 126 

    0x43390088,// 130 PAY 127 

    0x0c114159,// 131 PAY 128 

    0x35c0f746,// 132 PAY 129 

    0xdf027bc1,// 133 PAY 130 

    0x49cc82ef,// 134 PAY 131 

    0x7878b710,// 135 PAY 132 

    0x84c283fb,// 136 PAY 133 

    0x8d15b777,// 137 PAY 134 

    0xaaa02d9b,// 138 PAY 135 

    0x64d34fb1,// 139 PAY 136 

    0x55a21d4b,// 140 PAY 137 

    0x159ac998,// 141 PAY 138 

    0x3fa0b308,// 142 PAY 139 

    0xe5e5d1e8,// 143 PAY 140 

    0x04c210eb,// 144 PAY 141 

    0x770aeab3,// 145 PAY 142 

    0x516f8d9a,// 146 PAY 143 

    0x6c4ad30f,// 147 PAY 144 

    0xcf2805d0,// 148 PAY 145 

    0x7c0319e8,// 149 PAY 146 

    0xcb1e5a98,// 150 PAY 147 

    0x22687683,// 151 PAY 148 

    0x91858bc0,// 152 PAY 149 

    0x38037e8c,// 153 PAY 150 

    0xfe5e442c,// 154 PAY 151 

    0x288d0888,// 155 PAY 152 

    0x3389a719,// 156 PAY 153 

    0x93ca98aa,// 157 PAY 154 

    0x00f6d5ab,// 158 PAY 155 

    0x84e5b47e,// 159 PAY 156 

    0x12a84674,// 160 PAY 157 

    0xa7a5f5af,// 161 PAY 158 

    0xe8b34646,// 162 PAY 159 

    0xcbb7662c,// 163 PAY 160 

    0x7eaa3c5e,// 164 PAY 161 

    0x347f6b4b,// 165 PAY 162 

    0x40daec2f,// 166 PAY 163 

    0xd36456a1,// 167 PAY 164 

    0x98819885,// 168 PAY 165 

    0x2b443b5b,// 169 PAY 166 

    0x3f38b37d,// 170 PAY 167 

    0x67a0f6b8,// 171 PAY 168 

    0x46a376cf,// 172 PAY 169 

    0x1ac5dd71,// 173 PAY 170 

    0xfa9daee8,// 174 PAY 171 

    0xd28bb370,// 175 PAY 172 

    0x9bf0da03,// 176 PAY 173 

    0xd5ef138c,// 177 PAY 174 

    0x404311b4,// 178 PAY 175 

    0x2ecf813c,// 179 PAY 176 

    0x61832313,// 180 PAY 177 

    0xe963afab,// 181 PAY 178 

    0x6df4c7f6,// 182 PAY 179 

    0x29c4862d,// 183 PAY 180 

    0x82a7b3a0,// 184 PAY 181 

    0xb4048017,// 185 PAY 182 

    0xe09914fc,// 186 PAY 183 

    0x4c1999d7,// 187 PAY 184 

    0x267b3e94,// 188 PAY 185 

    0x3278ccf9,// 189 PAY 186 

    0x8f5281b6,// 190 PAY 187 

    0x83e5e018,// 191 PAY 188 

    0xf096ea5e,// 192 PAY 189 

    0xf9c65edb,// 193 PAY 190 

    0xf04b1732,// 194 PAY 191 

    0x1e456a68,// 195 PAY 192 

    0x0feb82e4,// 196 PAY 193 

    0x3ea18867,// 197 PAY 194 

    0x250d63da,// 198 PAY 195 

    0xf6876bce,// 199 PAY 196 

    0xe624bc23,// 200 PAY 197 

    0xd3f7b059,// 201 PAY 198 

    0xb434c241,// 202 PAY 199 

    0xc73cbdf3,// 203 PAY 200 

    0x9f1b4609,// 204 PAY 201 

    0x8b764726,// 205 PAY 202 

    0xbfbd129e,// 206 PAY 203 

    0x654f455f,// 207 PAY 204 

    0xb64833cf,// 208 PAY 205 

    0x59a242e5,// 209 PAY 206 

    0x43d4012e,// 210 PAY 207 

    0x9ae414be,// 211 PAY 208 

    0x0a1b32b6,// 212 PAY 209 

    0x74555bdb,// 213 PAY 210 

    0x7126c488,// 214 PAY 211 

    0x56435821,// 215 PAY 212 

    0xdaa2ff2e,// 216 PAY 213 

    0x67e4ba41,// 217 PAY 214 

    0x7c50eb72,// 218 PAY 215 

    0x25e96fe8,// 219 PAY 216 

    0x0ae90f32,// 220 PAY 217 

    0x27ed41bd,// 221 PAY 218 

    0x7936210f,// 222 PAY 219 

    0x83b236c7,// 223 PAY 220 

    0x9a444e46,// 224 PAY 221 

    0x067b361b,// 225 PAY 222 

    0x7866f16b,// 226 PAY 223 

    0x0b3fa0e1,// 227 PAY 224 

    0x09f06a4c,// 228 PAY 225 

    0x7523d652,// 229 PAY 226 

    0xb18cad38,// 230 PAY 227 

    0xc9aaa5ac,// 231 PAY 228 

    0x2a693c04,// 232 PAY 229 

    0x9f6bb099,// 233 PAY 230 

    0x8f2416a4,// 234 PAY 231 

    0x03cc45d5,// 235 PAY 232 

    0x948800da,// 236 PAY 233 

    0x2c5d292a,// 237 PAY 234 

    0x4eb4bbb3,// 238 PAY 235 

    0xe47bfbd9,// 239 PAY 236 

    0xd3491e13,// 240 PAY 237 

    0xf5f000fa,// 241 PAY 238 

    0xa6748db3,// 242 PAY 239 

    0x14b78aa8,// 243 PAY 240 

    0x745d5458,// 244 PAY 241 

    0x8b968027,// 245 PAY 242 

    0x9f477be0,// 246 PAY 243 

    0xebb850c1,// 247 PAY 244 

    0x6e9c7c9a,// 248 PAY 245 

    0xf5bda129,// 249 PAY 246 

    0xe450cd97,// 250 PAY 247 

    0x71001676,// 251 PAY 248 

    0x4dc24721,// 252 PAY 249 

    0x22896213,// 253 PAY 250 

    0x288e24ae,// 254 PAY 251 

    0xd4ca7c32,// 255 PAY 252 

    0x4c7f1897,// 256 PAY 253 

    0x955f58b8,// 257 PAY 254 

    0x22ca54a4,// 258 PAY 255 

    0x71f30c16,// 259 PAY 256 

    0xa72e2a26,// 260 PAY 257 

    0x4a7043b0,// 261 PAY 258 

    0x9f618f85,// 262 PAY 259 

    0x60c677f0,// 263 PAY 260 

    0x455c04a2,// 264 PAY 261 

    0x04254038,// 265 PAY 262 

    0x1f90b552,// 266 PAY 263 

    0xafd3e56b,// 267 PAY 264 

    0xc8682755,// 268 PAY 265 

    0x157ba22a,// 269 PAY 266 

    0xf032fa4c,// 270 PAY 267 

    0x9e77bcfd,// 271 PAY 268 

    0x12e3ac6f,// 272 PAY 269 

    0x6b2ca92b,// 273 PAY 270 

    0x8c4aba18,// 274 PAY 271 

    0x3b68f99b,// 275 PAY 272 

    0xc589b66a,// 276 PAY 273 

    0x18f4c6d2,// 277 PAY 274 

    0x7f7a6554,// 278 PAY 275 

    0xfbb50d9d,// 279 PAY 276 

    0x83ebec30,// 280 PAY 277 

    0x13af9cb8,// 281 PAY 278 

    0x324d23df,// 282 PAY 279 

    0xed195272,// 283 PAY 280 

    0xa43a2812,// 284 PAY 281 

    0x906d5d2b,// 285 PAY 282 

    0x957967be,// 286 PAY 283 

    0x103c454e,// 287 PAY 284 

    0x7692527c,// 288 PAY 285 

    0xcaec992a,// 289 PAY 286 

    0xf8430700,// 290 PAY 287 

/// HASH is  12 bytes 

    0x25e96fe8,// 291 HSH   1 

    0x0ae90f32,// 292 HSH   2 

    0x27ed41bd,// 293 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 145 

/// STA pkt_idx        : 230 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x03994d91 // 294 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt26_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 197 words. 

/// BDA size     is 782 (0x30e) 

/// BDA id       is 0x9ed4 

    0x030e9ed4,// 3 BDA   1 

/// PAY Generic Data size   : 782 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x327cc595,// 4 PAY   1 

    0x4818735e,// 5 PAY   2 

    0xad798bab,// 6 PAY   3 

    0x013bff89,// 7 PAY   4 

    0x0b2a766b,// 8 PAY   5 

    0x177126a8,// 9 PAY   6 

    0x5ca3f0cc,// 10 PAY   7 

    0x0524e82a,// 11 PAY   8 

    0x2ff3bffe,// 12 PAY   9 

    0x0e95af3d,// 13 PAY  10 

    0xe12dcf0c,// 14 PAY  11 

    0x58d0dff5,// 15 PAY  12 

    0x59697f29,// 16 PAY  13 

    0x7f0a5232,// 17 PAY  14 

    0xee6bc4aa,// 18 PAY  15 

    0x66478667,// 19 PAY  16 

    0xc34955eb,// 20 PAY  17 

    0xe4f06231,// 21 PAY  18 

    0xd94c5c85,// 22 PAY  19 

    0x898a5761,// 23 PAY  20 

    0xe28fee4e,// 24 PAY  21 

    0xfb0b5062,// 25 PAY  22 

    0x838b01ff,// 26 PAY  23 

    0x65819d83,// 27 PAY  24 

    0x8f73abe9,// 28 PAY  25 

    0xd9da52b2,// 29 PAY  26 

    0xa717ca0a,// 30 PAY  27 

    0xe74865d0,// 31 PAY  28 

    0x7b24b53a,// 32 PAY  29 

    0xff003e76,// 33 PAY  30 

    0xab2b89a2,// 34 PAY  31 

    0xdf3618b4,// 35 PAY  32 

    0x707d1741,// 36 PAY  33 

    0xaf5508f3,// 37 PAY  34 

    0x244d93b2,// 38 PAY  35 

    0x17031265,// 39 PAY  36 

    0x6363d53c,// 40 PAY  37 

    0x5cd69faa,// 41 PAY  38 

    0x27535ff5,// 42 PAY  39 

    0x0a1d0f5b,// 43 PAY  40 

    0x3f024943,// 44 PAY  41 

    0xf775c2aa,// 45 PAY  42 

    0x812ec15b,// 46 PAY  43 

    0xef82574c,// 47 PAY  44 

    0xe5c8655f,// 48 PAY  45 

    0xce7be555,// 49 PAY  46 

    0x3dcbf1d3,// 50 PAY  47 

    0x57c40002,// 51 PAY  48 

    0x1b86f8a3,// 52 PAY  49 

    0xf659b54f,// 53 PAY  50 

    0x8b40e11d,// 54 PAY  51 

    0x65da9e53,// 55 PAY  52 

    0x06fa45d2,// 56 PAY  53 

    0x0c01d937,// 57 PAY  54 

    0xb70c49a6,// 58 PAY  55 

    0xbaf927b7,// 59 PAY  56 

    0x4b0b157e,// 60 PAY  57 

    0xd7f06a10,// 61 PAY  58 

    0x4b4a9bd0,// 62 PAY  59 

    0x2545aeb3,// 63 PAY  60 

    0x71498839,// 64 PAY  61 

    0xf0e5c9df,// 65 PAY  62 

    0xe92a5965,// 66 PAY  63 

    0x4af32a96,// 67 PAY  64 

    0x74f2f203,// 68 PAY  65 

    0x93647cb4,// 69 PAY  66 

    0x52374408,// 70 PAY  67 

    0x3183d534,// 71 PAY  68 

    0xe423a493,// 72 PAY  69 

    0x3a3f1fcc,// 73 PAY  70 

    0x402871b7,// 74 PAY  71 

    0xb9c88a3f,// 75 PAY  72 

    0x614c1484,// 76 PAY  73 

    0x08fcb9db,// 77 PAY  74 

    0x600bb3d3,// 78 PAY  75 

    0xc8bd16e4,// 79 PAY  76 

    0x6be464d6,// 80 PAY  77 

    0xea03d276,// 81 PAY  78 

    0xb0ef486d,// 82 PAY  79 

    0xdcc8e21d,// 83 PAY  80 

    0x2461fe30,// 84 PAY  81 

    0xa97d17fa,// 85 PAY  82 

    0xc16effa9,// 86 PAY  83 

    0x0527ff33,// 87 PAY  84 

    0xb7f05911,// 88 PAY  85 

    0xca82ad1e,// 89 PAY  86 

    0xecf4b30f,// 90 PAY  87 

    0x86f1557f,// 91 PAY  88 

    0x72a8619a,// 92 PAY  89 

    0x27e3f5da,// 93 PAY  90 

    0xe6e14e4d,// 94 PAY  91 

    0x8508dae4,// 95 PAY  92 

    0xed2a7a1c,// 96 PAY  93 

    0xf55ec87f,// 97 PAY  94 

    0xb2b25811,// 98 PAY  95 

    0xae51e9d0,// 99 PAY  96 

    0x00799602,// 100 PAY  97 

    0xa7a5b9c8,// 101 PAY  98 

    0x8e420739,// 102 PAY  99 

    0x08bbc7c4,// 103 PAY 100 

    0xca33d641,// 104 PAY 101 

    0x5d693347,// 105 PAY 102 

    0xf9f655b0,// 106 PAY 103 

    0x7bf9704b,// 107 PAY 104 

    0x1cc1188f,// 108 PAY 105 

    0x9d7488c6,// 109 PAY 106 

    0x885a5071,// 110 PAY 107 

    0xa0fc0d4f,// 111 PAY 108 

    0x15552445,// 112 PAY 109 

    0x6ceff593,// 113 PAY 110 

    0x9cf0c3af,// 114 PAY 111 

    0x30c985c9,// 115 PAY 112 

    0x5eb642de,// 116 PAY 113 

    0x25188492,// 117 PAY 114 

    0x845f61c3,// 118 PAY 115 

    0x5ca7207c,// 119 PAY 116 

    0x9ca7e81e,// 120 PAY 117 

    0x34917a4e,// 121 PAY 118 

    0x77a3b762,// 122 PAY 119 

    0xee227809,// 123 PAY 120 

    0x6b8a69ca,// 124 PAY 121 

    0xa4e8f9b2,// 125 PAY 122 

    0x418186d5,// 126 PAY 123 

    0x79b2f12c,// 127 PAY 124 

    0x31851f12,// 128 PAY 125 

    0xd00c6623,// 129 PAY 126 

    0x319a977e,// 130 PAY 127 

    0x432b03be,// 131 PAY 128 

    0x6043777a,// 132 PAY 129 

    0xb510d73f,// 133 PAY 130 

    0x74d11c00,// 134 PAY 131 

    0x97964cd5,// 135 PAY 132 

    0x9da03fe8,// 136 PAY 133 

    0xe4bfa627,// 137 PAY 134 

    0xb9257901,// 138 PAY 135 

    0x8b1ea506,// 139 PAY 136 

    0x916f8fda,// 140 PAY 137 

    0x537b4228,// 141 PAY 138 

    0x82941333,// 142 PAY 139 

    0x061b83b0,// 143 PAY 140 

    0xe6282a41,// 144 PAY 141 

    0xd0fc2855,// 145 PAY 142 

    0xea3aff46,// 146 PAY 143 

    0x9b070124,// 147 PAY 144 

    0x3b4370be,// 148 PAY 145 

    0x811ff64d,// 149 PAY 146 

    0x1d8a956a,// 150 PAY 147 

    0x662ad993,// 151 PAY 148 

    0xdd55eb56,// 152 PAY 149 

    0x0107d520,// 153 PAY 150 

    0x995800e4,// 154 PAY 151 

    0xb601fe98,// 155 PAY 152 

    0x175199a4,// 156 PAY 153 

    0xf209a461,// 157 PAY 154 

    0x4e847c22,// 158 PAY 155 

    0x94b9e2b2,// 159 PAY 156 

    0x50dfd18f,// 160 PAY 157 

    0xe2cd49c6,// 161 PAY 158 

    0x1f10064b,// 162 PAY 159 

    0x03475ad6,// 163 PAY 160 

    0x0e32e62b,// 164 PAY 161 

    0xa0dbe20a,// 165 PAY 162 

    0xde9334a9,// 166 PAY 163 

    0x04c0c320,// 167 PAY 164 

    0x03ba2472,// 168 PAY 165 

    0xe452b2c7,// 169 PAY 166 

    0xed71cd37,// 170 PAY 167 

    0x38017a48,// 171 PAY 168 

    0x8c39eb37,// 172 PAY 169 

    0xc75334ab,// 173 PAY 170 

    0x2f28d248,// 174 PAY 171 

    0xfb48fbe5,// 175 PAY 172 

    0xe5563282,// 176 PAY 173 

    0xd23804f6,// 177 PAY 174 

    0xd9f5584d,// 178 PAY 175 

    0x02e0c81d,// 179 PAY 176 

    0x469fd1c4,// 180 PAY 177 

    0x72208c13,// 181 PAY 178 

    0x2278cab0,// 182 PAY 179 

    0x5166ac2a,// 183 PAY 180 

    0xe0ecc17b,// 184 PAY 181 

    0xf7ae33a2,// 185 PAY 182 

    0xcf8326dc,// 186 PAY 183 

    0xd7e26124,// 187 PAY 184 

    0x2c2df0a3,// 188 PAY 185 

    0xc66e4657,// 189 PAY 186 

    0x7e2f0c2f,// 190 PAY 187 

    0xc9c54ef2,// 191 PAY 188 

    0xf4925a3d,// 192 PAY 189 

    0xc8a83016,// 193 PAY 190 

    0xf8c98512,// 194 PAY 191 

    0xfb4d902a,// 195 PAY 192 

    0xdea9fde5,// 196 PAY 193 

    0xc171d44c,// 197 PAY 194 

    0xa933e6bb,// 198 PAY 195 

    0x8a720000,// 199 PAY 196 

/// STA is 1 words. 

/// STA num_pkts       : 14 

/// STA pkt_idx        : 249 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc8 

    0x03e4c80e // 200 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt27_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 320 words. 

/// BDA size     is 1273 (0x4f9) 

/// BDA id       is 0x601b 

    0x04f9601b,// 3 BDA   1 

/// PAY Generic Data size   : 1273 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x13346c49,// 4 PAY   1 

    0x16733b6a,// 5 PAY   2 

    0xfd9439c4,// 6 PAY   3 

    0x778cb88a,// 7 PAY   4 

    0x14623871,// 8 PAY   5 

    0x14c0daba,// 9 PAY   6 

    0xb72ef37a,// 10 PAY   7 

    0x24ae7de6,// 11 PAY   8 

    0x00382e34,// 12 PAY   9 

    0x6c475dbe,// 13 PAY  10 

    0xad267448,// 14 PAY  11 

    0x864f6e64,// 15 PAY  12 

    0x4938f51d,// 16 PAY  13 

    0x8951f9d9,// 17 PAY  14 

    0x08c36268,// 18 PAY  15 

    0x4d96ec72,// 19 PAY  16 

    0xa7fd8e54,// 20 PAY  17 

    0x38e0c1c7,// 21 PAY  18 

    0xa00af3a7,// 22 PAY  19 

    0x2254febf,// 23 PAY  20 

    0x72267066,// 24 PAY  21 

    0x53d29d78,// 25 PAY  22 

    0x3b228e73,// 26 PAY  23 

    0xdcccc8c5,// 27 PAY  24 

    0x9c3b3cd3,// 28 PAY  25 

    0xd0214db1,// 29 PAY  26 

    0x11994723,// 30 PAY  27 

    0x5bd3b5d6,// 31 PAY  28 

    0x3ec8b269,// 32 PAY  29 

    0xcbe16cda,// 33 PAY  30 

    0x4aa07623,// 34 PAY  31 

    0xcd4682fc,// 35 PAY  32 

    0x14721f55,// 36 PAY  33 

    0x61bbfb88,// 37 PAY  34 

    0x298c97e7,// 38 PAY  35 

    0xc2bb8a0b,// 39 PAY  36 

    0xb0986a58,// 40 PAY  37 

    0x422145f2,// 41 PAY  38 

    0x4433c358,// 42 PAY  39 

    0x1bcce3d5,// 43 PAY  40 

    0xefa74849,// 44 PAY  41 

    0xb323ed99,// 45 PAY  42 

    0x8d4ed436,// 46 PAY  43 

    0xfab34cd7,// 47 PAY  44 

    0x4e9e2bbe,// 48 PAY  45 

    0xbbe2c00e,// 49 PAY  46 

    0xc59874aa,// 50 PAY  47 

    0xb40aeccb,// 51 PAY  48 

    0x06601553,// 52 PAY  49 

    0x40b008aa,// 53 PAY  50 

    0x840ff390,// 54 PAY  51 

    0x05647ea4,// 55 PAY  52 

    0x8e2ab29e,// 56 PAY  53 

    0x6d795f62,// 57 PAY  54 

    0x4b5e26df,// 58 PAY  55 

    0x82b516fe,// 59 PAY  56 

    0xd52b87d3,// 60 PAY  57 

    0xa8f92792,// 61 PAY  58 

    0x4ac43252,// 62 PAY  59 

    0xcac7604f,// 63 PAY  60 

    0x82a6dbdb,// 64 PAY  61 

    0x9f370de8,// 65 PAY  62 

    0xbf071cd0,// 66 PAY  63 

    0x33f76e2b,// 67 PAY  64 

    0x7f74421f,// 68 PAY  65 

    0x0e30aa13,// 69 PAY  66 

    0x542bb53f,// 70 PAY  67 

    0x1f874acc,// 71 PAY  68 

    0x98089caf,// 72 PAY  69 

    0x8ae6f424,// 73 PAY  70 

    0x2b3ad38f,// 74 PAY  71 

    0x8fd07439,// 75 PAY  72 

    0x91a4b6b5,// 76 PAY  73 

    0x46490b9b,// 77 PAY  74 

    0x6710e2d1,// 78 PAY  75 

    0x2b1203a7,// 79 PAY  76 

    0xeecf633a,// 80 PAY  77 

    0xfac1311a,// 81 PAY  78 

    0x03e452a4,// 82 PAY  79 

    0x6311561b,// 83 PAY  80 

    0x3f3aa77e,// 84 PAY  81 

    0xc09be41d,// 85 PAY  82 

    0x4e77992a,// 86 PAY  83 

    0x8ecde911,// 87 PAY  84 

    0x60be04b1,// 88 PAY  85 

    0xe2bd31a0,// 89 PAY  86 

    0x1c87e278,// 90 PAY  87 

    0xcfa6e3b9,// 91 PAY  88 

    0x7bc8bc72,// 92 PAY  89 

    0x0e038cc9,// 93 PAY  90 

    0x9294ca67,// 94 PAY  91 

    0x25419164,// 95 PAY  92 

    0x89babc96,// 96 PAY  93 

    0x20e662bf,// 97 PAY  94 

    0xfd79d9ab,// 98 PAY  95 

    0x8628c252,// 99 PAY  96 

    0x20916bf9,// 100 PAY  97 

    0xfe274369,// 101 PAY  98 

    0x2ad1dd1f,// 102 PAY  99 

    0x6178d11f,// 103 PAY 100 

    0x0b6d479f,// 104 PAY 101 

    0x312e92f3,// 105 PAY 102 

    0x90124922,// 106 PAY 103 

    0xbaf82b70,// 107 PAY 104 

    0xbfb14f64,// 108 PAY 105 

    0xc3eaf093,// 109 PAY 106 

    0x2e399875,// 110 PAY 107 

    0xd876feab,// 111 PAY 108 

    0xce4c79bd,// 112 PAY 109 

    0x6f503e4d,// 113 PAY 110 

    0x564b7b24,// 114 PAY 111 

    0x0baa53ce,// 115 PAY 112 

    0xadad0818,// 116 PAY 113 

    0x3b01feeb,// 117 PAY 114 

    0x1d0192b5,// 118 PAY 115 

    0xa10c6f49,// 119 PAY 116 

    0x3a67d05f,// 120 PAY 117 

    0x65b62b43,// 121 PAY 118 

    0xdebf5738,// 122 PAY 119 

    0x9e5285c5,// 123 PAY 120 

    0x73eb350d,// 124 PAY 121 

    0x33d5ae71,// 125 PAY 122 

    0x62d489eb,// 126 PAY 123 

    0xe873bb5b,// 127 PAY 124 

    0x8d84ba27,// 128 PAY 125 

    0xe72beab2,// 129 PAY 126 

    0xfd3fe1a6,// 130 PAY 127 

    0x63ffab63,// 131 PAY 128 

    0xc9fbca15,// 132 PAY 129 

    0x4d248eec,// 133 PAY 130 

    0xa9319df3,// 134 PAY 131 

    0x26b6b359,// 135 PAY 132 

    0xdefe6e9f,// 136 PAY 133 

    0x0fc344c4,// 137 PAY 134 

    0xefcb1f17,// 138 PAY 135 

    0x37d976ae,// 139 PAY 136 

    0xbf1bd335,// 140 PAY 137 

    0x24fb2e1c,// 141 PAY 138 

    0xc420993c,// 142 PAY 139 

    0x73421364,// 143 PAY 140 

    0xe5c4cbe2,// 144 PAY 141 

    0x5d448a3f,// 145 PAY 142 

    0xea48a347,// 146 PAY 143 

    0xee5f7557,// 147 PAY 144 

    0x859dc015,// 148 PAY 145 

    0x3c3701e0,// 149 PAY 146 

    0xa75ee144,// 150 PAY 147 

    0xfec1666f,// 151 PAY 148 

    0x8c97e0cb,// 152 PAY 149 

    0x41f179f3,// 153 PAY 150 

    0x31a3f847,// 154 PAY 151 

    0x81ac6f55,// 155 PAY 152 

    0xf66936b5,// 156 PAY 153 

    0xc9f11a37,// 157 PAY 154 

    0xe476ff76,// 158 PAY 155 

    0x5de2fd1d,// 159 PAY 156 

    0x9200cfd4,// 160 PAY 157 

    0x470cdc1a,// 161 PAY 158 

    0x96cd4e84,// 162 PAY 159 

    0x8b0dba15,// 163 PAY 160 

    0x9dab3a58,// 164 PAY 161 

    0x10fd2c0d,// 165 PAY 162 

    0xb1af086e,// 166 PAY 163 

    0xb5ac9d08,// 167 PAY 164 

    0x338e5229,// 168 PAY 165 

    0xc8a95715,// 169 PAY 166 

    0xc70bb72e,// 170 PAY 167 

    0x6137a0e5,// 171 PAY 168 

    0xf1d76e3f,// 172 PAY 169 

    0x3ff384d1,// 173 PAY 170 

    0xc4282a86,// 174 PAY 171 

    0xfa97a66e,// 175 PAY 172 

    0xc9e4d52f,// 176 PAY 173 

    0xcde2c7d6,// 177 PAY 174 

    0x3825aee1,// 178 PAY 175 

    0x44d8653c,// 179 PAY 176 

    0xcaaffb51,// 180 PAY 177 

    0x6d22e808,// 181 PAY 178 

    0x9937a653,// 182 PAY 179 

    0x8f259508,// 183 PAY 180 

    0xe4fb6351,// 184 PAY 181 

    0xbf9e05da,// 185 PAY 182 

    0x0f56a1ea,// 186 PAY 183 

    0x519ecc08,// 187 PAY 184 

    0x7906adea,// 188 PAY 185 

    0x3d12bc70,// 189 PAY 186 

    0xca58886d,// 190 PAY 187 

    0x36bd466c,// 191 PAY 188 

    0xea6842c8,// 192 PAY 189 

    0x23abb37c,// 193 PAY 190 

    0x83710f02,// 194 PAY 191 

    0xde8f91b8,// 195 PAY 192 

    0xae1f2391,// 196 PAY 193 

    0x8cb028b8,// 197 PAY 194 

    0xcbcfaca3,// 198 PAY 195 

    0x5902577d,// 199 PAY 196 

    0x1c200432,// 200 PAY 197 

    0x5a35d67c,// 201 PAY 198 

    0x2de067fe,// 202 PAY 199 

    0x963efeb6,// 203 PAY 200 

    0x5a491443,// 204 PAY 201 

    0x4ee22746,// 205 PAY 202 

    0xf12563be,// 206 PAY 203 

    0x5f21d50e,// 207 PAY 204 

    0x1a83697a,// 208 PAY 205 

    0x71772c43,// 209 PAY 206 

    0x222dac4e,// 210 PAY 207 

    0x29245153,// 211 PAY 208 

    0xdb27cec1,// 212 PAY 209 

    0x6e03d163,// 213 PAY 210 

    0xbc79c897,// 214 PAY 211 

    0x4470650e,// 215 PAY 212 

    0x031481f0,// 216 PAY 213 

    0x1952c600,// 217 PAY 214 

    0xa59181ba,// 218 PAY 215 

    0x7d280840,// 219 PAY 216 

    0x7396fef7,// 220 PAY 217 

    0xfaca1bc6,// 221 PAY 218 

    0x80ff6115,// 222 PAY 219 

    0xfb901840,// 223 PAY 220 

    0xd2ceba4a,// 224 PAY 221 

    0xdd45d0a8,// 225 PAY 222 

    0x3ab5efa3,// 226 PAY 223 

    0x00b79393,// 227 PAY 224 

    0x2978ea9a,// 228 PAY 225 

    0xdb5d39e2,// 229 PAY 226 

    0x602e43bc,// 230 PAY 227 

    0x36e14a0e,// 231 PAY 228 

    0x3af18044,// 232 PAY 229 

    0x028efb78,// 233 PAY 230 

    0x1603eb35,// 234 PAY 231 

    0x0ceb9214,// 235 PAY 232 

    0x50705bad,// 236 PAY 233 

    0xd38244a4,// 237 PAY 234 

    0x12a1e0ef,// 238 PAY 235 

    0x84fa8a69,// 239 PAY 236 

    0xbd5af0a6,// 240 PAY 237 

    0x64535df4,// 241 PAY 238 

    0x8cae1410,// 242 PAY 239 

    0x8a8d5a7c,// 243 PAY 240 

    0x776df535,// 244 PAY 241 

    0xa610127b,// 245 PAY 242 

    0x92c9fd2c,// 246 PAY 243 

    0xcd1283b4,// 247 PAY 244 

    0x0e5c6c8b,// 248 PAY 245 

    0x314a1492,// 249 PAY 246 

    0x2c00f38a,// 250 PAY 247 

    0x16a96fbf,// 251 PAY 248 

    0xed418395,// 252 PAY 249 

    0x1211d079,// 253 PAY 250 

    0x5bb441db,// 254 PAY 251 

    0xbd3cd183,// 255 PAY 252 

    0x8c4be505,// 256 PAY 253 

    0x5da33c7f,// 257 PAY 254 

    0x220c7b9f,// 258 PAY 255 

    0xeab0587a,// 259 PAY 256 

    0x448178a3,// 260 PAY 257 

    0xddb1f7cc,// 261 PAY 258 

    0x953e51e5,// 262 PAY 259 

    0xb8813fe8,// 263 PAY 260 

    0x3069ad6c,// 264 PAY 261 

    0x32ac74fe,// 265 PAY 262 

    0x659c8d07,// 266 PAY 263 

    0x1fb86ff7,// 267 PAY 264 

    0x5bed8df3,// 268 PAY 265 

    0x9c869d15,// 269 PAY 266 

    0x3a2bcea4,// 270 PAY 267 

    0xb9bf262c,// 271 PAY 268 

    0x72d3626f,// 272 PAY 269 

    0x38acc4cc,// 273 PAY 270 

    0xe02ceb22,// 274 PAY 271 

    0xb4c356ca,// 275 PAY 272 

    0xe1bfa310,// 276 PAY 273 

    0x312665ac,// 277 PAY 274 

    0xcfd1cf40,// 278 PAY 275 

    0x8685231a,// 279 PAY 276 

    0x2dae3dd3,// 280 PAY 277 

    0x9dcda84f,// 281 PAY 278 

    0xbdc83a90,// 282 PAY 279 

    0x74a77dc2,// 283 PAY 280 

    0x69c51ae4,// 284 PAY 281 

    0xcbed9743,// 285 PAY 282 

    0x2ef1ad4d,// 286 PAY 283 

    0xbecd01c9,// 287 PAY 284 

    0x5b15d28c,// 288 PAY 285 

    0x437606aa,// 289 PAY 286 

    0xad52e28a,// 290 PAY 287 

    0x68fecd51,// 291 PAY 288 

    0xbebe1aee,// 292 PAY 289 

    0x75c0cef3,// 293 PAY 290 

    0xe264427c,// 294 PAY 291 

    0x46eed095,// 295 PAY 292 

    0x66db09f6,// 296 PAY 293 

    0x56385851,// 297 PAY 294 

    0xd22f9f61,// 298 PAY 295 

    0x8a4ac323,// 299 PAY 296 

    0x7665aded,// 300 PAY 297 

    0x9fbfd3aa,// 301 PAY 298 

    0x84a84f36,// 302 PAY 299 

    0x48fc195c,// 303 PAY 300 

    0x90411d14,// 304 PAY 301 

    0x06526bc2,// 305 PAY 302 

    0x6462a488,// 306 PAY 303 

    0x1b504883,// 307 PAY 304 

    0x613b2680,// 308 PAY 305 

    0xb54731dd,// 309 PAY 306 

    0x7f1916bd,// 310 PAY 307 

    0xe0fd9b30,// 311 PAY 308 

    0x61ecfcf2,// 312 PAY 309 

    0xa0b9ef5a,// 313 PAY 310 

    0xcbaee0b7,// 314 PAY 311 

    0x6345bf0b,// 315 PAY 312 

    0xc7d42e9b,// 316 PAY 313 

    0xfe687fba,// 317 PAY 314 

    0xc78eeae2,// 318 PAY 315 

    0x78a49853,// 319 PAY 316 

    0x7e152387,// 320 PAY 317 

    0x0e4947ed,// 321 PAY 318 

    0x26000000,// 322 PAY 319 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x03f8c1d6 // 323 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt28_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 456 words. 

/// BDA size     is 1817 (0x719) 

/// BDA id       is 0x24a3 

    0x071924a3,// 3 BDA   1 

/// PAY Generic Data size   : 1817 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xda49730c,// 4 PAY   1 

    0xfe45c0ff,// 5 PAY   2 

    0x756fda51,// 6 PAY   3 

    0x88b8ce1d,// 7 PAY   4 

    0x16cfbd4c,// 8 PAY   5 

    0x58579e23,// 9 PAY   6 

    0x71047811,// 10 PAY   7 

    0x3c41c08d,// 11 PAY   8 

    0xa435ca4c,// 12 PAY   9 

    0x81d46969,// 13 PAY  10 

    0xd43474b4,// 14 PAY  11 

    0xa26982c7,// 15 PAY  12 

    0x6e4da85e,// 16 PAY  13 

    0x776fef04,// 17 PAY  14 

    0xa4958a77,// 18 PAY  15 

    0x163f2e06,// 19 PAY  16 

    0x14efc315,// 20 PAY  17 

    0x58be96ba,// 21 PAY  18 

    0xbb575b00,// 22 PAY  19 

    0x7cdd82e5,// 23 PAY  20 

    0x48d3b6a2,// 24 PAY  21 

    0xd438e04b,// 25 PAY  22 

    0x5cc79687,// 26 PAY  23 

    0xcea4184e,// 27 PAY  24 

    0xff9eb5e5,// 28 PAY  25 

    0x8791ac5f,// 29 PAY  26 

    0x05b81d17,// 30 PAY  27 

    0x3a6c8254,// 31 PAY  28 

    0xf64565c2,// 32 PAY  29 

    0x93c25d7c,// 33 PAY  30 

    0x314b3f43,// 34 PAY  31 

    0x7bea6b0b,// 35 PAY  32 

    0xbcc46941,// 36 PAY  33 

    0x55a75b8d,// 37 PAY  34 

    0xc6537966,// 38 PAY  35 

    0xa7c0863f,// 39 PAY  36 

    0x1b43b86f,// 40 PAY  37 

    0x177c6e9e,// 41 PAY  38 

    0xcc0975d1,// 42 PAY  39 

    0x0e05f001,// 43 PAY  40 

    0x243dce1c,// 44 PAY  41 

    0xc1fc80c4,// 45 PAY  42 

    0x7f173b72,// 46 PAY  43 

    0xf46b1bfe,// 47 PAY  44 

    0x98a33d18,// 48 PAY  45 

    0xc757d2d0,// 49 PAY  46 

    0xa46c03b5,// 50 PAY  47 

    0xd74ab563,// 51 PAY  48 

    0xa52499ab,// 52 PAY  49 

    0xfabac680,// 53 PAY  50 

    0x07cd5628,// 54 PAY  51 

    0x8eb3d4ee,// 55 PAY  52 

    0x175dd802,// 56 PAY  53 

    0x58e374e4,// 57 PAY  54 

    0x62cf8c14,// 58 PAY  55 

    0xb747aa4c,// 59 PAY  56 

    0x822dad3a,// 60 PAY  57 

    0x642f8073,// 61 PAY  58 

    0xd6b8b21c,// 62 PAY  59 

    0x78800bd0,// 63 PAY  60 

    0x8c51e4f9,// 64 PAY  61 

    0x087765c2,// 65 PAY  62 

    0xda99b2b1,// 66 PAY  63 

    0x1ad5cbb4,// 67 PAY  64 

    0xb16c7e95,// 68 PAY  65 

    0xcc4e2c2c,// 69 PAY  66 

    0x60cf8022,// 70 PAY  67 

    0x445a817d,// 71 PAY  68 

    0xbfb0ee22,// 72 PAY  69 

    0x3fcf5385,// 73 PAY  70 

    0x1b00aaa4,// 74 PAY  71 

    0xe66d332e,// 75 PAY  72 

    0xb69f53cb,// 76 PAY  73 

    0x6f881758,// 77 PAY  74 

    0x1c133094,// 78 PAY  75 

    0x74c778e0,// 79 PAY  76 

    0x6b47d71c,// 80 PAY  77 

    0x26b797ce,// 81 PAY  78 

    0x4f32c755,// 82 PAY  79 

    0x69c16eac,// 83 PAY  80 

    0x66ccd825,// 84 PAY  81 

    0xe6cb435f,// 85 PAY  82 

    0x09f622e2,// 86 PAY  83 

    0x3356a0f6,// 87 PAY  84 

    0xcb9db251,// 88 PAY  85 

    0x23f9925e,// 89 PAY  86 

    0xfd03ceb4,// 90 PAY  87 

    0x6072225b,// 91 PAY  88 

    0x915b6568,// 92 PAY  89 

    0xebaca34a,// 93 PAY  90 

    0xe7efae46,// 94 PAY  91 

    0x6ac89109,// 95 PAY  92 

    0x81550a97,// 96 PAY  93 

    0xd28bfb54,// 97 PAY  94 

    0x21b0ba9a,// 98 PAY  95 

    0x985b0092,// 99 PAY  96 

    0xb6f6767d,// 100 PAY  97 

    0x201e8768,// 101 PAY  98 

    0xd94a9c6a,// 102 PAY  99 

    0x482fafe7,// 103 PAY 100 

    0xa04a317f,// 104 PAY 101 

    0x60d6ac7e,// 105 PAY 102 

    0x58a854f4,// 106 PAY 103 

    0x7e692755,// 107 PAY 104 

    0x850a40f9,// 108 PAY 105 

    0x11e6b27c,// 109 PAY 106 

    0xeda1f283,// 110 PAY 107 

    0xdf8bade0,// 111 PAY 108 

    0x9eaeac14,// 112 PAY 109 

    0xfcc4b9db,// 113 PAY 110 

    0x5d68ed45,// 114 PAY 111 

    0x7270a528,// 115 PAY 112 

    0x9543ccf0,// 116 PAY 113 

    0xd68512f8,// 117 PAY 114 

    0xd70f96cc,// 118 PAY 115 

    0x0f7d534c,// 119 PAY 116 

    0x42c4e068,// 120 PAY 117 

    0x709158a7,// 121 PAY 118 

    0xfaaa6fe3,// 122 PAY 119 

    0xe116826c,// 123 PAY 120 

    0xe6f1225f,// 124 PAY 121 

    0xe448df2e,// 125 PAY 122 

    0xd558f01c,// 126 PAY 123 

    0x582ac99b,// 127 PAY 124 

    0x26337d7c,// 128 PAY 125 

    0xa6877ceb,// 129 PAY 126 

    0x519d8aa9,// 130 PAY 127 

    0x8a6dcd44,// 131 PAY 128 

    0x5468792b,// 132 PAY 129 

    0x85359562,// 133 PAY 130 

    0x363c2d66,// 134 PAY 131 

    0x2997dc5d,// 135 PAY 132 

    0x04c0bbd1,// 136 PAY 133 

    0x5e1838ac,// 137 PAY 134 

    0x2c6067c8,// 138 PAY 135 

    0x60719a94,// 139 PAY 136 

    0x436586e1,// 140 PAY 137 

    0x1eab69a1,// 141 PAY 138 

    0x736c836f,// 142 PAY 139 

    0x543f8efe,// 143 PAY 140 

    0x3b06b727,// 144 PAY 141 

    0x53ccc91e,// 145 PAY 142 

    0xd6260bf3,// 146 PAY 143 

    0xa059ddbc,// 147 PAY 144 

    0x51a40531,// 148 PAY 145 

    0x6affc22d,// 149 PAY 146 

    0x777d4920,// 150 PAY 147 

    0x219c218e,// 151 PAY 148 

    0x48a16eb8,// 152 PAY 149 

    0xa1f73963,// 153 PAY 150 

    0xcfd4802a,// 154 PAY 151 

    0xfdb21b52,// 155 PAY 152 

    0x770be155,// 156 PAY 153 

    0xfe5403f7,// 157 PAY 154 

    0x90fda9c7,// 158 PAY 155 

    0x4d3b2d5c,// 159 PAY 156 

    0x6f829a64,// 160 PAY 157 

    0xeb4c611d,// 161 PAY 158 

    0x0f38b57a,// 162 PAY 159 

    0xb0523cd4,// 163 PAY 160 

    0x14353042,// 164 PAY 161 

    0xf8899a78,// 165 PAY 162 

    0xb56e89b1,// 166 PAY 163 

    0x39b0c170,// 167 PAY 164 

    0xf1eb9099,// 168 PAY 165 

    0x56d31b20,// 169 PAY 166 

    0xe48e35bb,// 170 PAY 167 

    0x65d4962f,// 171 PAY 168 

    0x855c4430,// 172 PAY 169 

    0xf9e95764,// 173 PAY 170 

    0xc703199d,// 174 PAY 171 

    0x30989479,// 175 PAY 172 

    0x3fcd1747,// 176 PAY 173 

    0x9b78579e,// 177 PAY 174 

    0x6454e44b,// 178 PAY 175 

    0xd08c98ee,// 179 PAY 176 

    0x3d5558c5,// 180 PAY 177 

    0xe7058a59,// 181 PAY 178 

    0xd8425e31,// 182 PAY 179 

    0x4ef0f5d8,// 183 PAY 180 

    0x655f737a,// 184 PAY 181 

    0x91b4e611,// 185 PAY 182 

    0x58b5f69f,// 186 PAY 183 

    0x4ad3f305,// 187 PAY 184 

    0x92ad4725,// 188 PAY 185 

    0x40efeebc,// 189 PAY 186 

    0xb17a7770,// 190 PAY 187 

    0xfcc65109,// 191 PAY 188 

    0x46df42b8,// 192 PAY 189 

    0x28a4d5a9,// 193 PAY 190 

    0x4cc6b6b5,// 194 PAY 191 

    0xf2eaaaee,// 195 PAY 192 

    0xa47f9818,// 196 PAY 193 

    0x64447c6c,// 197 PAY 194 

    0x52202afa,// 198 PAY 195 

    0x9774f130,// 199 PAY 196 

    0x6c6a6e1d,// 200 PAY 197 

    0x5c23c61b,// 201 PAY 198 

    0x979186f6,// 202 PAY 199 

    0xaf1ef0fd,// 203 PAY 200 

    0x839aa4af,// 204 PAY 201 

    0x8c03b0d6,// 205 PAY 202 

    0x6e3f7459,// 206 PAY 203 

    0xd96b208c,// 207 PAY 204 

    0x815daeb6,// 208 PAY 205 

    0x52bd56ac,// 209 PAY 206 

    0xe4fb2caf,// 210 PAY 207 

    0x92bdab92,// 211 PAY 208 

    0x949cd8ff,// 212 PAY 209 

    0xe5b3d6c8,// 213 PAY 210 

    0x1fdba3a7,// 214 PAY 211 

    0x2fef1630,// 215 PAY 212 

    0x1a2011df,// 216 PAY 213 

    0x15785472,// 217 PAY 214 

    0xf7c331c2,// 218 PAY 215 

    0x20ff8ac0,// 219 PAY 216 

    0xf9d67ed5,// 220 PAY 217 

    0x72ce5f8b,// 221 PAY 218 

    0x22933d8c,// 222 PAY 219 

    0x930082bc,// 223 PAY 220 

    0xaa9a5131,// 224 PAY 221 

    0x859a706e,// 225 PAY 222 

    0xaf6b8773,// 226 PAY 223 

    0xab0652ce,// 227 PAY 224 

    0x0eb0ba5b,// 228 PAY 225 

    0x3f81fb5b,// 229 PAY 226 

    0x3d3c77be,// 230 PAY 227 

    0x206c8a93,// 231 PAY 228 

    0x92fe18bd,// 232 PAY 229 

    0xd66538b3,// 233 PAY 230 

    0xa50421ad,// 234 PAY 231 

    0xb3421882,// 235 PAY 232 

    0x947629a2,// 236 PAY 233 

    0xa58b039f,// 237 PAY 234 

    0x86a174ff,// 238 PAY 235 

    0x7e249d5b,// 239 PAY 236 

    0xca50768a,// 240 PAY 237 

    0xcc21cee2,// 241 PAY 238 

    0x46c6923c,// 242 PAY 239 

    0xb04808b2,// 243 PAY 240 

    0x7b474ed1,// 244 PAY 241 

    0x0e2c712b,// 245 PAY 242 

    0x963a6fbe,// 246 PAY 243 

    0xb985e166,// 247 PAY 244 

    0x7b76ca95,// 248 PAY 245 

    0x2a090c88,// 249 PAY 246 

    0xb44bcc6a,// 250 PAY 247 

    0x7055710b,// 251 PAY 248 

    0x2b27dfca,// 252 PAY 249 

    0xcfad69da,// 253 PAY 250 

    0x9fb24514,// 254 PAY 251 

    0xfa8cd839,// 255 PAY 252 

    0x9273420f,// 256 PAY 253 

    0x05486779,// 257 PAY 254 

    0xb9e1194c,// 258 PAY 255 

    0xd66fd611,// 259 PAY 256 

    0x839461e4,// 260 PAY 257 

    0x80b3c42f,// 261 PAY 258 

    0x7927239f,// 262 PAY 259 

    0x478d4d24,// 263 PAY 260 

    0x770dffeb,// 264 PAY 261 

    0x37641bca,// 265 PAY 262 

    0x0326e909,// 266 PAY 263 

    0x6113ca04,// 267 PAY 264 

    0x6aa31716,// 268 PAY 265 

    0x7149869e,// 269 PAY 266 

    0xb79df335,// 270 PAY 267 

    0x1a163a4f,// 271 PAY 268 

    0xca6f1676,// 272 PAY 269 

    0x381202f5,// 273 PAY 270 

    0x9a171248,// 274 PAY 271 

    0x184c417c,// 275 PAY 272 

    0x8736a021,// 276 PAY 273 

    0x22c969e4,// 277 PAY 274 

    0x6132fda9,// 278 PAY 275 

    0xf7b8b4ff,// 279 PAY 276 

    0xfb62025b,// 280 PAY 277 

    0x81096d5e,// 281 PAY 278 

    0x9aae7590,// 282 PAY 279 

    0x4cfefb4a,// 283 PAY 280 

    0x85e84fd9,// 284 PAY 281 

    0x0c31af7d,// 285 PAY 282 

    0x59eca01a,// 286 PAY 283 

    0xf9bb89a4,// 287 PAY 284 

    0x901c7359,// 288 PAY 285 

    0x838242e9,// 289 PAY 286 

    0x84f97f79,// 290 PAY 287 

    0xc39adb5c,// 291 PAY 288 

    0xe6f84622,// 292 PAY 289 

    0x29b824a3,// 293 PAY 290 

    0x4dab448b,// 294 PAY 291 

    0xdd32e640,// 295 PAY 292 

    0x83171a8c,// 296 PAY 293 

    0xe5424639,// 297 PAY 294 

    0xfc407749,// 298 PAY 295 

    0x57fd9554,// 299 PAY 296 

    0x960afe9c,// 300 PAY 297 

    0x200f7a00,// 301 PAY 298 

    0xf048681c,// 302 PAY 299 

    0x0ee7c8a8,// 303 PAY 300 

    0x3efb9ded,// 304 PAY 301 

    0x8322d98b,// 305 PAY 302 

    0x60136528,// 306 PAY 303 

    0x2e85d995,// 307 PAY 304 

    0x89278a29,// 308 PAY 305 

    0x7701eccc,// 309 PAY 306 

    0x25c0e4bc,// 310 PAY 307 

    0x7ab40d82,// 311 PAY 308 

    0x73a98ea5,// 312 PAY 309 

    0x5f6ee49f,// 313 PAY 310 

    0x9d35636f,// 314 PAY 311 

    0xc4c70211,// 315 PAY 312 

    0xeb9d7bfe,// 316 PAY 313 

    0x7b8d90e5,// 317 PAY 314 

    0xba815f1e,// 318 PAY 315 

    0x49063319,// 319 PAY 316 

    0xb7b733d6,// 320 PAY 317 

    0xc433488d,// 321 PAY 318 

    0xa1a1a917,// 322 PAY 319 

    0xd55d711c,// 323 PAY 320 

    0xf9753c4a,// 324 PAY 321 

    0xb2bba2b9,// 325 PAY 322 

    0xaecef008,// 326 PAY 323 

    0x9c6abefe,// 327 PAY 324 

    0x07dfe13b,// 328 PAY 325 

    0x9eb742fb,// 329 PAY 326 

    0x9ef00b6b,// 330 PAY 327 

    0x31548a7e,// 331 PAY 328 

    0x576bf319,// 332 PAY 329 

    0x0728bb9a,// 333 PAY 330 

    0xb4f73c8a,// 334 PAY 331 

    0x3a4aae32,// 335 PAY 332 

    0x519a855e,// 336 PAY 333 

    0xc8d2d16e,// 337 PAY 334 

    0x3cc4f99f,// 338 PAY 335 

    0xc1c1b4bf,// 339 PAY 336 

    0x1e696446,// 340 PAY 337 

    0xe629f5e3,// 341 PAY 338 

    0x18d596aa,// 342 PAY 339 

    0xfc62b3b7,// 343 PAY 340 

    0xea6b4410,// 344 PAY 341 

    0x7d8ea53e,// 345 PAY 342 

    0xc3e8867a,// 346 PAY 343 

    0x81af1376,// 347 PAY 344 

    0xb195ea39,// 348 PAY 345 

    0xcdc935da,// 349 PAY 346 

    0x6b614404,// 350 PAY 347 

    0x65f66a49,// 351 PAY 348 

    0x54bbd24a,// 352 PAY 349 

    0x691c43c1,// 353 PAY 350 

    0xa65885e0,// 354 PAY 351 

    0xffa591fc,// 355 PAY 352 

    0x96f0e2ef,// 356 PAY 353 

    0x1a29f074,// 357 PAY 354 

    0x3ab6eefd,// 358 PAY 355 

    0x84f9a7b7,// 359 PAY 356 

    0xcd35e40b,// 360 PAY 357 

    0x0e91a600,// 361 PAY 358 

    0xac369f3b,// 362 PAY 359 

    0x28f1cf3b,// 363 PAY 360 

    0x47c8a888,// 364 PAY 361 

    0x9110e2cb,// 365 PAY 362 

    0xda37abf5,// 366 PAY 363 

    0xb1f0d06b,// 367 PAY 364 

    0xadef2028,// 368 PAY 365 

    0x86f89aa5,// 369 PAY 366 

    0x221eccae,// 370 PAY 367 

    0xd934998c,// 371 PAY 368 

    0x380e4b07,// 372 PAY 369 

    0x6e5b06f4,// 373 PAY 370 

    0xcba67db8,// 374 PAY 371 

    0x4cd1cca1,// 375 PAY 372 

    0x18ca567a,// 376 PAY 373 

    0x5ac66860,// 377 PAY 374 

    0x87176eb5,// 378 PAY 375 

    0xd52a7824,// 379 PAY 376 

    0xfb111683,// 380 PAY 377 

    0xedeb4af8,// 381 PAY 378 

    0x5eaa1644,// 382 PAY 379 

    0x0224e4ef,// 383 PAY 380 

    0xf0ac2d25,// 384 PAY 381 

    0xf9a0187d,// 385 PAY 382 

    0x090d48f0,// 386 PAY 383 

    0x3c0f61e5,// 387 PAY 384 

    0xe5da4f0a,// 388 PAY 385 

    0x3f8904d0,// 389 PAY 386 

    0x5b1459c9,// 390 PAY 387 

    0xe90c381d,// 391 PAY 388 

    0x21253887,// 392 PAY 389 

    0x83521b44,// 393 PAY 390 

    0x83ba30c3,// 394 PAY 391 

    0x96ae08b5,// 395 PAY 392 

    0xf2ce9e74,// 396 PAY 393 

    0xf602cf3c,// 397 PAY 394 

    0xbc87a4aa,// 398 PAY 395 

    0xa0e4f1f7,// 399 PAY 396 

    0xaa48762c,// 400 PAY 397 

    0xb7b3900a,// 401 PAY 398 

    0x839854d1,// 402 PAY 399 

    0xbf9a27d2,// 403 PAY 400 

    0x4009b783,// 404 PAY 401 

    0xc90f14b0,// 405 PAY 402 

    0x4d7408b7,// 406 PAY 403 

    0x420eead1,// 407 PAY 404 

    0x6900c8dd,// 408 PAY 405 

    0x7b3d59bd,// 409 PAY 406 

    0x9ee15267,// 410 PAY 407 

    0xdc172585,// 411 PAY 408 

    0xc7613b08,// 412 PAY 409 

    0x61c31ca0,// 413 PAY 410 

    0x0e0a8a9f,// 414 PAY 411 

    0xd7172dc6,// 415 PAY 412 

    0x3b93cddb,// 416 PAY 413 

    0x1ef6ed4f,// 417 PAY 414 

    0x6e7c3a61,// 418 PAY 415 

    0x15349a13,// 419 PAY 416 

    0x613006ed,// 420 PAY 417 

    0xce29c284,// 421 PAY 418 

    0xf2c8a447,// 422 PAY 419 

    0xf6f06e9b,// 423 PAY 420 

    0xa26b1531,// 424 PAY 421 

    0x50537162,// 425 PAY 422 

    0x9ea65629,// 426 PAY 423 

    0xa8844f83,// 427 PAY 424 

    0x2aebe292,// 428 PAY 425 

    0xd3ee7c39,// 429 PAY 426 

    0x6b94f80d,// 430 PAY 427 

    0xbedfe334,// 431 PAY 428 

    0x278c36c8,// 432 PAY 429 

    0x963041b3,// 433 PAY 430 

    0x5e929bf7,// 434 PAY 431 

    0x9561fd77,// 435 PAY 432 

    0xcb388331,// 436 PAY 433 

    0x80e66b5a,// 437 PAY 434 

    0x70957cc3,// 438 PAY 435 

    0xd59a704b,// 439 PAY 436 

    0xc52b25cc,// 440 PAY 437 

    0x98b85508,// 441 PAY 438 

    0xfd00c1ec,// 442 PAY 439 

    0xe2820b1e,// 443 PAY 440 

    0x58920b9a,// 444 PAY 441 

    0x555ec779,// 445 PAY 442 

    0xddbcf506,// 446 PAY 443 

    0x12f849e0,// 447 PAY 444 

    0xc528711d,// 448 PAY 445 

    0x2c13cf87,// 449 PAY 446 

    0xd165b0d0,// 450 PAY 447 

    0x7b88587b,// 451 PAY 448 

    0xd519e896,// 452 PAY 449 

    0xa4638ea7,// 453 PAY 450 

    0x3e523fd6,// 454 PAY 451 

    0xa25693c0,// 455 PAY 452 

    0x5e97527c,// 456 PAY 453 

    0x78e2aa1b,// 457 PAY 454 

    0xca000000,// 458 PAY 455 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe5 

    0x01e8e580 // 459 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt29_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 371 words. 

/// BDA size     is 1477 (0x5c5) 

/// BDA id       is 0x31c5 

    0x05c531c5,// 3 BDA   1 

/// PAY Generic Data size   : 1477 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x58217e07,// 4 PAY   1 

    0xa93dd2ee,// 5 PAY   2 

    0x6fbff81f,// 6 PAY   3 

    0x72e0a88e,// 7 PAY   4 

    0x752f25de,// 8 PAY   5 

    0x4a22b3a3,// 9 PAY   6 

    0x82247185,// 10 PAY   7 

    0x99a7770f,// 11 PAY   8 

    0xafd115d3,// 12 PAY   9 

    0xcba904be,// 13 PAY  10 

    0x359855ed,// 14 PAY  11 

    0x6c0661a6,// 15 PAY  12 

    0xa77bd4dc,// 16 PAY  13 

    0xe8a20169,// 17 PAY  14 

    0xd013309a,// 18 PAY  15 

    0x303913f6,// 19 PAY  16 

    0x2cf56b69,// 20 PAY  17 

    0x4ef12e94,// 21 PAY  18 

    0xb632b09a,// 22 PAY  19 

    0x2e78c6e5,// 23 PAY  20 

    0x243f581f,// 24 PAY  21 

    0x9cec72e2,// 25 PAY  22 

    0x8cf8bc35,// 26 PAY  23 

    0x31e3a384,// 27 PAY  24 

    0x1a06ce35,// 28 PAY  25 

    0x5f7a0458,// 29 PAY  26 

    0x50366943,// 30 PAY  27 

    0x39800f6f,// 31 PAY  28 

    0x2ff798ff,// 32 PAY  29 

    0x57cd1a43,// 33 PAY  30 

    0x51297eff,// 34 PAY  31 

    0xf22ef606,// 35 PAY  32 

    0x40c91860,// 36 PAY  33 

    0xc11565c8,// 37 PAY  34 

    0x94a7bd0f,// 38 PAY  35 

    0xeb8a7684,// 39 PAY  36 

    0xd49e0e12,// 40 PAY  37 

    0x7fe61a72,// 41 PAY  38 

    0x30b18fb6,// 42 PAY  39 

    0xdeda937d,// 43 PAY  40 

    0xf72b6f6b,// 44 PAY  41 

    0x90a3bad6,// 45 PAY  42 

    0x632d4751,// 46 PAY  43 

    0xb7360c1b,// 47 PAY  44 

    0x7b87fdce,// 48 PAY  45 

    0x8b080157,// 49 PAY  46 

    0xdafd24cc,// 50 PAY  47 

    0x714fc96d,// 51 PAY  48 

    0x64cd8ae9,// 52 PAY  49 

    0xeb607a17,// 53 PAY  50 

    0x8864ca24,// 54 PAY  51 

    0x11ac8ae7,// 55 PAY  52 

    0x6eb78c5f,// 56 PAY  53 

    0x4653decb,// 57 PAY  54 

    0xa7b8db40,// 58 PAY  55 

    0x0265961e,// 59 PAY  56 

    0xa054b1ab,// 60 PAY  57 

    0x37a807f2,// 61 PAY  58 

    0x565de49e,// 62 PAY  59 

    0x6b04f78c,// 63 PAY  60 

    0x51ba18ef,// 64 PAY  61 

    0x07ae3693,// 65 PAY  62 

    0x1b1b4778,// 66 PAY  63 

    0x0ad61f3c,// 67 PAY  64 

    0xa8e81ed8,// 68 PAY  65 

    0x96b10931,// 69 PAY  66 

    0xf2e7d3f1,// 70 PAY  67 

    0xd907b381,// 71 PAY  68 

    0xa2ae4277,// 72 PAY  69 

    0x16c74bce,// 73 PAY  70 

    0xddc49bfd,// 74 PAY  71 

    0xb80a1068,// 75 PAY  72 

    0x9d23144e,// 76 PAY  73 

    0x3b026610,// 77 PAY  74 

    0x8c8f85e5,// 78 PAY  75 

    0x897583d7,// 79 PAY  76 

    0xead58a2f,// 80 PAY  77 

    0x73cf1ba7,// 81 PAY  78 

    0xef5d496f,// 82 PAY  79 

    0x72662c55,// 83 PAY  80 

    0x8ad1ec00,// 84 PAY  81 

    0xd6382e5b,// 85 PAY  82 

    0x410d829d,// 86 PAY  83 

    0xdd1a4d4a,// 87 PAY  84 

    0x8f62dcf9,// 88 PAY  85 

    0x87801c29,// 89 PAY  86 

    0x789fcd0e,// 90 PAY  87 

    0x9f6a7a4c,// 91 PAY  88 

    0x7717cb6f,// 92 PAY  89 

    0x7e7ff388,// 93 PAY  90 

    0x8f4f554f,// 94 PAY  91 

    0xaecb1287,// 95 PAY  92 

    0x05d013ba,// 96 PAY  93 

    0x3adeda23,// 97 PAY  94 

    0x2b3ed1d6,// 98 PAY  95 

    0xf63614b4,// 99 PAY  96 

    0x33cb1d9a,// 100 PAY  97 

    0x9841331e,// 101 PAY  98 

    0xf7204b38,// 102 PAY  99 

    0x19bb953d,// 103 PAY 100 

    0xfb189d1a,// 104 PAY 101 

    0xa90ea733,// 105 PAY 102 

    0x8743c6a7,// 106 PAY 103 

    0xfa6f1f7e,// 107 PAY 104 

    0x0aeb651f,// 108 PAY 105 

    0x4ac56b0d,// 109 PAY 106 

    0x3f93f790,// 110 PAY 107 

    0xf4111a45,// 111 PAY 108 

    0xc3d4ed74,// 112 PAY 109 

    0x5e3e694e,// 113 PAY 110 

    0xd9beae92,// 114 PAY 111 

    0x141ba4d4,// 115 PAY 112 

    0xc32577b5,// 116 PAY 113 

    0x61e3c371,// 117 PAY 114 

    0xb1e6316a,// 118 PAY 115 

    0xf3086c08,// 119 PAY 116 

    0xa8a073bb,// 120 PAY 117 

    0xd37a0c47,// 121 PAY 118 

    0x7a45dd78,// 122 PAY 119 

    0x4d0dba7e,// 123 PAY 120 

    0x5a5585de,// 124 PAY 121 

    0xa1888a1f,// 125 PAY 122 

    0x5ef799c2,// 126 PAY 123 

    0x5d80324c,// 127 PAY 124 

    0x4890b643,// 128 PAY 125 

    0x3015cfa0,// 129 PAY 126 

    0x0e1514eb,// 130 PAY 127 

    0x1953a42c,// 131 PAY 128 

    0x4d764ce2,// 132 PAY 129 

    0xa4d49b4c,// 133 PAY 130 

    0xd1aead2c,// 134 PAY 131 

    0x6dc08161,// 135 PAY 132 

    0x4f17558e,// 136 PAY 133 

    0xe608f229,// 137 PAY 134 

    0xa0653c93,// 138 PAY 135 

    0xa8f463ca,// 139 PAY 136 

    0xea84e6a6,// 140 PAY 137 

    0x09397244,// 141 PAY 138 

    0x39f146d9,// 142 PAY 139 

    0x4ba73e12,// 143 PAY 140 

    0x5262171b,// 144 PAY 141 

    0xad545660,// 145 PAY 142 

    0x17124f7f,// 146 PAY 143 

    0x1327e08f,// 147 PAY 144 

    0x9896d9de,// 148 PAY 145 

    0xaa619333,// 149 PAY 146 

    0xb2254f6b,// 150 PAY 147 

    0xacd1b34a,// 151 PAY 148 

    0x2bbdf5bf,// 152 PAY 149 

    0x880d8181,// 153 PAY 150 

    0xd765ecb3,// 154 PAY 151 

    0xb7deb849,// 155 PAY 152 

    0x9907c96e,// 156 PAY 153 

    0x28b6219b,// 157 PAY 154 

    0x31b02d59,// 158 PAY 155 

    0x6cb5e825,// 159 PAY 156 

    0xca388bba,// 160 PAY 157 

    0x47902bea,// 161 PAY 158 

    0xcfe727cc,// 162 PAY 159 

    0x33ea7b8f,// 163 PAY 160 

    0x71d4ac42,// 164 PAY 161 

    0xe97985dc,// 165 PAY 162 

    0x5db8eac9,// 166 PAY 163 

    0x1e7d0d54,// 167 PAY 164 

    0x159c120c,// 168 PAY 165 

    0xb784b85d,// 169 PAY 166 

    0x2122f3fa,// 170 PAY 167 

    0x4659b4e3,// 171 PAY 168 

    0x473d6f4f,// 172 PAY 169 

    0xfb20f06b,// 173 PAY 170 

    0x0e4d991c,// 174 PAY 171 

    0xa390a79a,// 175 PAY 172 

    0x97f3ea2a,// 176 PAY 173 

    0x36db1c60,// 177 PAY 174 

    0x908c8b25,// 178 PAY 175 

    0xf87166e1,// 179 PAY 176 

    0x3c7c7e06,// 180 PAY 177 

    0xc6f672f1,// 181 PAY 178 

    0x0e165bc8,// 182 PAY 179 

    0xeef47629,// 183 PAY 180 

    0xa7f77865,// 184 PAY 181 

    0xc8394029,// 185 PAY 182 

    0xf20f29a4,// 186 PAY 183 

    0x666d2f60,// 187 PAY 184 

    0xc4e426fd,// 188 PAY 185 

    0x0bca750a,// 189 PAY 186 

    0x5e3cc959,// 190 PAY 187 

    0xbd49dec6,// 191 PAY 188 

    0x8cba1384,// 192 PAY 189 

    0x8c41b726,// 193 PAY 190 

    0xfa040929,// 194 PAY 191 

    0xaea08deb,// 195 PAY 192 

    0x2bc07e93,// 196 PAY 193 

    0xa83450f2,// 197 PAY 194 

    0xdd788017,// 198 PAY 195 

    0xf19fee75,// 199 PAY 196 

    0x39ecbd91,// 200 PAY 197 

    0xd663be04,// 201 PAY 198 

    0xb1a6e3f0,// 202 PAY 199 

    0x1bb22f5e,// 203 PAY 200 

    0x1a6ba1e2,// 204 PAY 201 

    0x77277954,// 205 PAY 202 

    0x92056cc0,// 206 PAY 203 

    0x093abb08,// 207 PAY 204 

    0x3943a1bf,// 208 PAY 205 

    0x595aed7b,// 209 PAY 206 

    0x3a9f732d,// 210 PAY 207 

    0x7f597259,// 211 PAY 208 

    0xa81caddd,// 212 PAY 209 

    0xfebd3e95,// 213 PAY 210 

    0x4a975476,// 214 PAY 211 

    0x9ac1a5c3,// 215 PAY 212 

    0x777f2bc7,// 216 PAY 213 

    0xcd362dcb,// 217 PAY 214 

    0x4226963f,// 218 PAY 215 

    0xf68dfe92,// 219 PAY 216 

    0x4176347f,// 220 PAY 217 

    0x45db0232,// 221 PAY 218 

    0xb214f9d9,// 222 PAY 219 

    0xb90ca3f8,// 223 PAY 220 

    0x2095d0eb,// 224 PAY 221 

    0x2cb33ccc,// 225 PAY 222 

    0xfa16206c,// 226 PAY 223 

    0x1497b171,// 227 PAY 224 

    0xc3481ccb,// 228 PAY 225 

    0x9e8253ad,// 229 PAY 226 

    0x6d2484bc,// 230 PAY 227 

    0x47bbc99b,// 231 PAY 228 

    0x2f9e5c5e,// 232 PAY 229 

    0x5c4d9df5,// 233 PAY 230 

    0x151a2e3e,// 234 PAY 231 

    0xaa040a00,// 235 PAY 232 

    0xc27500ed,// 236 PAY 233 

    0x4b48d454,// 237 PAY 234 

    0x801839fb,// 238 PAY 235 

    0x96c6c923,// 239 PAY 236 

    0x242fc1bc,// 240 PAY 237 

    0xd72c3874,// 241 PAY 238 

    0x5d9e80fd,// 242 PAY 239 

    0xc6488417,// 243 PAY 240 

    0xdf21fc07,// 244 PAY 241 

    0x279ed917,// 245 PAY 242 

    0x13e878e6,// 246 PAY 243 

    0xcacdc0fa,// 247 PAY 244 

    0xebbefbc5,// 248 PAY 245 

    0xb8e08913,// 249 PAY 246 

    0x6ae3434d,// 250 PAY 247 

    0xd055ea99,// 251 PAY 248 

    0xb5d3230a,// 252 PAY 249 

    0xd611ea53,// 253 PAY 250 

    0x093ed743,// 254 PAY 251 

    0xd41734c8,// 255 PAY 252 

    0xa5f6fec5,// 256 PAY 253 

    0x31d12bd1,// 257 PAY 254 

    0xc66cef9a,// 258 PAY 255 

    0x1bd474c3,// 259 PAY 256 

    0xe167ce65,// 260 PAY 257 

    0x65d131f7,// 261 PAY 258 

    0xbc01a531,// 262 PAY 259 

    0x187c6667,// 263 PAY 260 

    0xff70ae48,// 264 PAY 261 

    0x0537629d,// 265 PAY 262 

    0x1f1b4b11,// 266 PAY 263 

    0x648bd3da,// 267 PAY 264 

    0xec3509ef,// 268 PAY 265 

    0xfa229ae0,// 269 PAY 266 

    0xf85e029a,// 270 PAY 267 

    0x873a80be,// 271 PAY 268 

    0x1ad08ba2,// 272 PAY 269 

    0x48e14c62,// 273 PAY 270 

    0x929a2510,// 274 PAY 271 

    0xa4ad31b7,// 275 PAY 272 

    0xb1e57d65,// 276 PAY 273 

    0xe72f4bcd,// 277 PAY 274 

    0x178e6f48,// 278 PAY 275 

    0x66ed535c,// 279 PAY 276 

    0x4dc581db,// 280 PAY 277 

    0xbcd7138d,// 281 PAY 278 

    0xb0ca34e8,// 282 PAY 279 

    0x183f1c1b,// 283 PAY 280 

    0x9a4c3d5f,// 284 PAY 281 

    0x9f617a3e,// 285 PAY 282 

    0x58f76a9b,// 286 PAY 283 

    0x41120e7d,// 287 PAY 284 

    0xd90482bd,// 288 PAY 285 

    0x861bf6eb,// 289 PAY 286 

    0xa995a7eb,// 290 PAY 287 

    0x7828708c,// 291 PAY 288 

    0xc61ff245,// 292 PAY 289 

    0x1b823e4f,// 293 PAY 290 

    0xca352e5e,// 294 PAY 291 

    0x70d48bb3,// 295 PAY 292 

    0x655ddb35,// 296 PAY 293 

    0x28bf4ef9,// 297 PAY 294 

    0xca4e980e,// 298 PAY 295 

    0x9360472d,// 299 PAY 296 

    0x7135f45f,// 300 PAY 297 

    0x67073b7a,// 301 PAY 298 

    0xd946de30,// 302 PAY 299 

    0x75fc3963,// 303 PAY 300 

    0x6fd6f8c2,// 304 PAY 301 

    0x95581e69,// 305 PAY 302 

    0x0ba5cc48,// 306 PAY 303 

    0x4d993608,// 307 PAY 304 

    0xb1bad365,// 308 PAY 305 

    0x3680c4fb,// 309 PAY 306 

    0x00d800ed,// 310 PAY 307 

    0x547b9d9a,// 311 PAY 308 

    0x6e6ffe96,// 312 PAY 309 

    0xde8fc168,// 313 PAY 310 

    0xcb307847,// 314 PAY 311 

    0xd2d94a95,// 315 PAY 312 

    0xfa56f5d6,// 316 PAY 313 

    0x36a19fa6,// 317 PAY 314 

    0xb8b32e72,// 318 PAY 315 

    0xb2e884c4,// 319 PAY 316 

    0x2893fc98,// 320 PAY 317 

    0x90dfa9bd,// 321 PAY 318 

    0xc6d2111f,// 322 PAY 319 

    0x32ac05de,// 323 PAY 320 

    0xa20b1dab,// 324 PAY 321 

    0x9347234e,// 325 PAY 322 

    0xadda82f5,// 326 PAY 323 

    0x9337ded1,// 327 PAY 324 

    0xc1927c13,// 328 PAY 325 

    0x38d7a3f0,// 329 PAY 326 

    0xc605437b,// 330 PAY 327 

    0x78b6c58b,// 331 PAY 328 

    0x7f860ded,// 332 PAY 329 

    0x6940d2b1,// 333 PAY 330 

    0x31508b08,// 334 PAY 331 

    0xe36f18ea,// 335 PAY 332 

    0x478520bc,// 336 PAY 333 

    0x6b1b68f4,// 337 PAY 334 

    0xb9c6a331,// 338 PAY 335 

    0xd34cae72,// 339 PAY 336 

    0xb2f31dee,// 340 PAY 337 

    0xde18beda,// 341 PAY 338 

    0xbf5d63f4,// 342 PAY 339 

    0xd8a26cea,// 343 PAY 340 

    0x224630bd,// 344 PAY 341 

    0xb9bc71c1,// 345 PAY 342 

    0x8b284107,// 346 PAY 343 

    0x90713570,// 347 PAY 344 

    0x969afc99,// 348 PAY 345 

    0x1dca7df7,// 349 PAY 346 

    0xc5204b46,// 350 PAY 347 

    0x285632af,// 351 PAY 348 

    0x1c8fbcf6,// 352 PAY 349 

    0xe36aea93,// 353 PAY 350 

    0x3b6b8eb6,// 354 PAY 351 

    0x27958877,// 355 PAY 352 

    0xb7ca9c97,// 356 PAY 353 

    0x630cc3e8,// 357 PAY 354 

    0xd467bb4a,// 358 PAY 355 

    0xd639a5f0,// 359 PAY 356 

    0xa5f2f909,// 360 PAY 357 

    0xf7d15f70,// 361 PAY 358 

    0x727d44f0,// 362 PAY 359 

    0xcd67dbcc,// 363 PAY 360 

    0xa75f06fa,// 364 PAY 361 

    0x5ae8008b,// 365 PAY 362 

    0x0785d2b3,// 366 PAY 363 

    0x16452d8f,// 367 PAY 364 

    0xd8e67bb9,// 368 PAY 365 

    0x37314b27,// 369 PAY 366 

    0x96e1334a,// 370 PAY 367 

    0x45699698,// 371 PAY 368 

    0x9b8cbdf8,// 372 PAY 369 

    0xc6000000,// 373 PAY 370 

/// HASH is  8 bytes 

    0xcd67dbcc,// 374 HSH   1 

    0xa75f06fa,// 375 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 28 

/// STA pkt_idx        : 43 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x00ac6e1c // 376 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt30_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 116 words. 

/// BDA size     is 457 (0x1c9) 

/// BDA id       is 0x15c 

    0x01c9015c,// 3 BDA   1 

/// PAY Generic Data size   : 457 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x18c4d92b,// 4 PAY   1 

    0x13313ccc,// 5 PAY   2 

    0xb3627668,// 6 PAY   3 

    0x8afaa7ec,// 7 PAY   4 

    0xa16c73b1,// 8 PAY   5 

    0x27af3804,// 9 PAY   6 

    0x7244e4c8,// 10 PAY   7 

    0x0ed93398,// 11 PAY   8 

    0x6ab53636,// 12 PAY   9 

    0x92d5af74,// 13 PAY  10 

    0xa5729c7a,// 14 PAY  11 

    0x20f898ea,// 15 PAY  12 

    0xa1500262,// 16 PAY  13 

    0xcb53e409,// 17 PAY  14 

    0xb3690363,// 18 PAY  15 

    0x23739eea,// 19 PAY  16 

    0xef2c1200,// 20 PAY  17 

    0x6d534937,// 21 PAY  18 

    0x42f80788,// 22 PAY  19 

    0x2d125845,// 23 PAY  20 

    0x8820e9d4,// 24 PAY  21 

    0xc55f5a57,// 25 PAY  22 

    0x78ea7b06,// 26 PAY  23 

    0x5672425b,// 27 PAY  24 

    0x7288fe5d,// 28 PAY  25 

    0xa59ebf81,// 29 PAY  26 

    0xc5b92cdd,// 30 PAY  27 

    0x14d92e0d,// 31 PAY  28 

    0x50d84338,// 32 PAY  29 

    0xe6e05b77,// 33 PAY  30 

    0x1614611f,// 34 PAY  31 

    0xbf377e87,// 35 PAY  32 

    0x142a0547,// 36 PAY  33 

    0xa45b8643,// 37 PAY  34 

    0x137340e5,// 38 PAY  35 

    0x1aefec5f,// 39 PAY  36 

    0x839a5cdd,// 40 PAY  37 

    0xd36955d4,// 41 PAY  38 

    0x4b14886a,// 42 PAY  39 

    0xbd973b4d,// 43 PAY  40 

    0x1904f127,// 44 PAY  41 

    0xd2064627,// 45 PAY  42 

    0x31e1c504,// 46 PAY  43 

    0x5ae9cf11,// 47 PAY  44 

    0x98ef6240,// 48 PAY  45 

    0x3f0d2500,// 49 PAY  46 

    0x739233f1,// 50 PAY  47 

    0xe7ea2786,// 51 PAY  48 

    0x9f278b9d,// 52 PAY  49 

    0x6c2b0885,// 53 PAY  50 

    0xce8e30fd,// 54 PAY  51 

    0x8788640d,// 55 PAY  52 

    0x05e3db2d,// 56 PAY  53 

    0x701fec94,// 57 PAY  54 

    0xe48ce3cf,// 58 PAY  55 

    0xa226e36c,// 59 PAY  56 

    0x3633bbbb,// 60 PAY  57 

    0x9e698e80,// 61 PAY  58 

    0x91b7bc24,// 62 PAY  59 

    0x91cbc456,// 63 PAY  60 

    0x6458a2c6,// 64 PAY  61 

    0x11760f4d,// 65 PAY  62 

    0x4b79db61,// 66 PAY  63 

    0xfe45fe26,// 67 PAY  64 

    0xee0c0738,// 68 PAY  65 

    0xdec7e831,// 69 PAY  66 

    0x7396d1ba,// 70 PAY  67 

    0x58e15245,// 71 PAY  68 

    0xec15daa0,// 72 PAY  69 

    0xbed3a262,// 73 PAY  70 

    0x9e2b3227,// 74 PAY  71 

    0xd8aa92f7,// 75 PAY  72 

    0x0ff28964,// 76 PAY  73 

    0xc4183f8e,// 77 PAY  74 

    0x89324211,// 78 PAY  75 

    0xf516c6fd,// 79 PAY  76 

    0xe3a63e79,// 80 PAY  77 

    0x1c6ad627,// 81 PAY  78 

    0x24ae076d,// 82 PAY  79 

    0x3787dc18,// 83 PAY  80 

    0xdce3c0f1,// 84 PAY  81 

    0xfec22848,// 85 PAY  82 

    0x0a31070a,// 86 PAY  83 

    0x74499f2e,// 87 PAY  84 

    0xfb13018b,// 88 PAY  85 

    0x7a9bbf2f,// 89 PAY  86 

    0x2d4240e4,// 90 PAY  87 

    0xa6d2ee05,// 91 PAY  88 

    0xfd1ee60d,// 92 PAY  89 

    0x538644b8,// 93 PAY  90 

    0x7038174a,// 94 PAY  91 

    0xc1e2646b,// 95 PAY  92 

    0xb304a839,// 96 PAY  93 

    0x0ea0b66f,// 97 PAY  94 

    0x97e5ea44,// 98 PAY  95 

    0xa70c637a,// 99 PAY  96 

    0x84ba43a6,// 100 PAY  97 

    0x356802b4,// 101 PAY  98 

    0x589bdaee,// 102 PAY  99 

    0x930bbb70,// 103 PAY 100 

    0x8072a95b,// 104 PAY 101 

    0xa3afc9de,// 105 PAY 102 

    0xd6514271,// 106 PAY 103 

    0xf221a7de,// 107 PAY 104 

    0x81687160,// 108 PAY 105 

    0x72cb4114,// 109 PAY 106 

    0x987f684e,// 110 PAY 107 

    0xc04cb931,// 111 PAY 108 

    0x84ac85ea,// 112 PAY 109 

    0xfe1e5d75,// 113 PAY 110 

    0xe41779e2,// 114 PAY 111 

    0xd91cadfd,// 115 PAY 112 

    0xea8aedaf,// 116 PAY 113 

    0x95848443,// 117 PAY 114 

    0xe0000000,// 118 PAY 115 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 105 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x53 

    0x01a55306 // 119 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt31_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 114 words. 

/// BDA size     is 452 (0x1c4) 

/// BDA id       is 0x2b67 

    0x01c42b67,// 3 BDA   1 

/// PAY Generic Data size   : 452 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xf000245d,// 4 PAY   1 

    0xdc5ec11a,// 5 PAY   2 

    0xd69318ae,// 6 PAY   3 

    0x9e74b65b,// 7 PAY   4 

    0x6630a630,// 8 PAY   5 

    0x3c334fb3,// 9 PAY   6 

    0x0223716c,// 10 PAY   7 

    0xb9db00f9,// 11 PAY   8 

    0xc60d2447,// 12 PAY   9 

    0x78847979,// 13 PAY  10 

    0x59854365,// 14 PAY  11 

    0x1d402266,// 15 PAY  12 

    0x36b057ba,// 16 PAY  13 

    0x228b9efc,// 17 PAY  14 

    0xfa7a7af2,// 18 PAY  15 

    0x4d391700,// 19 PAY  16 

    0x5205dc9e,// 20 PAY  17 

    0x199792e1,// 21 PAY  18 

    0x777e2a6d,// 22 PAY  19 

    0x0b8a6b71,// 23 PAY  20 

    0xecaa29ac,// 24 PAY  21 

    0x46e82704,// 25 PAY  22 

    0x7a3e7f99,// 26 PAY  23 

    0xe736a961,// 27 PAY  24 

    0x61356204,// 28 PAY  25 

    0xfbba9f5c,// 29 PAY  26 

    0xcebe95ad,// 30 PAY  27 

    0xb53e57b1,// 31 PAY  28 

    0xc0c4a3d2,// 32 PAY  29 

    0xffef571c,// 33 PAY  30 

    0x1f4d8558,// 34 PAY  31 

    0x3b30c0e9,// 35 PAY  32 

    0x9ca34205,// 36 PAY  33 

    0x7e5958ed,// 37 PAY  34 

    0xedd72295,// 38 PAY  35 

    0x7bc59782,// 39 PAY  36 

    0x0abecede,// 40 PAY  37 

    0x5b962dba,// 41 PAY  38 

    0xfdb7a062,// 42 PAY  39 

    0x69ebb806,// 43 PAY  40 

    0x35c90093,// 44 PAY  41 

    0xca3e3442,// 45 PAY  42 

    0x704d8928,// 46 PAY  43 

    0xe5062988,// 47 PAY  44 

    0xc85695a7,// 48 PAY  45 

    0x268f9cf4,// 49 PAY  46 

    0x6516aa45,// 50 PAY  47 

    0x4b092b04,// 51 PAY  48 

    0x713ae8f8,// 52 PAY  49 

    0xd8b15ff0,// 53 PAY  50 

    0x64511f90,// 54 PAY  51 

    0xe37a23e2,// 55 PAY  52 

    0x4c8c0526,// 56 PAY  53 

    0xb25b1b36,// 57 PAY  54 

    0x2e368e35,// 58 PAY  55 

    0x4cbd1c30,// 59 PAY  56 

    0x4966814a,// 60 PAY  57 

    0xb5c3984f,// 61 PAY  58 

    0x5c05895a,// 62 PAY  59 

    0xf5816f30,// 63 PAY  60 

    0x5c233c97,// 64 PAY  61 

    0x328aba75,// 65 PAY  62 

    0x2bbbb667,// 66 PAY  63 

    0x04a33793,// 67 PAY  64 

    0x91a9b728,// 68 PAY  65 

    0xa7afb07a,// 69 PAY  66 

    0x50ca47c0,// 70 PAY  67 

    0xd8b2434f,// 71 PAY  68 

    0x409ee1e2,// 72 PAY  69 

    0x498d0d13,// 73 PAY  70 

    0x824cf0ca,// 74 PAY  71 

    0xea73bf80,// 75 PAY  72 

    0x155fced0,// 76 PAY  73 

    0x16a0cfe6,// 77 PAY  74 

    0x8243fc51,// 78 PAY  75 

    0x4022fd2b,// 79 PAY  76 

    0x573cedfd,// 80 PAY  77 

    0xcb5ddbd1,// 81 PAY  78 

    0x785b0ed5,// 82 PAY  79 

    0x9dd0ac68,// 83 PAY  80 

    0xe836fbd4,// 84 PAY  81 

    0x4d1a0bdb,// 85 PAY  82 

    0x9f2dcff8,// 86 PAY  83 

    0x8b933536,// 87 PAY  84 

    0x7470cc67,// 88 PAY  85 

    0x62e77802,// 89 PAY  86 

    0xdc5e5832,// 90 PAY  87 

    0x476367aa,// 91 PAY  88 

    0xd802cbc4,// 92 PAY  89 

    0x08bf93f4,// 93 PAY  90 

    0xab9f5bd6,// 94 PAY  91 

    0x2108c7af,// 95 PAY  92 

    0x81a95649,// 96 PAY  93 

    0xff7bdcb1,// 97 PAY  94 

    0xc79c64eb,// 98 PAY  95 

    0xd7c393e6,// 99 PAY  96 

    0x28db3a6a,// 100 PAY  97 

    0x777fad5a,// 101 PAY  98 

    0x995bcf49,// 102 PAY  99 

    0x10021218,// 103 PAY 100 

    0x8a419795,// 104 PAY 101 

    0x04cf6c62,// 105 PAY 102 

    0x150bf2f1,// 106 PAY 103 

    0x4ea52386,// 107 PAY 104 

    0xf876e793,// 108 PAY 105 

    0xe1d5d29a,// 109 PAY 106 

    0xc9d67894,// 110 PAY 107 

    0x07427658,// 111 PAY 108 

    0xa9ea0d90,// 112 PAY 109 

    0x9365490b,// 113 PAY 110 

    0x12b15e2a,// 114 PAY 111 

    0x201f5fec,// 115 PAY 112 

    0xe3134bae,// 116 PAY 113 

/// HASH is  4 bytes 

    0x9365490b,// 117 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 179 

/// STA pkt_idx        : 154 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb8 

    0x0269b8b3 // 118 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt32_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 236 words. 

/// BDA size     is 937 (0x3a9) 

/// BDA id       is 0x1659 

    0x03a91659,// 3 BDA   1 

/// PAY Generic Data size   : 937 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x130bf83c,// 4 PAY   1 

    0xc924b476,// 5 PAY   2 

    0x2651010c,// 6 PAY   3 

    0xfe5d0365,// 7 PAY   4 

    0x1a625506,// 8 PAY   5 

    0x29de606b,// 9 PAY   6 

    0x053c2093,// 10 PAY   7 

    0xc2c84631,// 11 PAY   8 

    0x57ee8952,// 12 PAY   9 

    0x2d2598b3,// 13 PAY  10 

    0x799ec398,// 14 PAY  11 

    0x0be56c96,// 15 PAY  12 

    0xf9f44a80,// 16 PAY  13 

    0xa92b7157,// 17 PAY  14 

    0x75e5e0ce,// 18 PAY  15 

    0xf61f3c4d,// 19 PAY  16 

    0xdf05dfd0,// 20 PAY  17 

    0x4a4101ea,// 21 PAY  18 

    0x59f074cb,// 22 PAY  19 

    0xdddeb6e2,// 23 PAY  20 

    0x714d198e,// 24 PAY  21 

    0x3efa4072,// 25 PAY  22 

    0x36529d6d,// 26 PAY  23 

    0x74db2520,// 27 PAY  24 

    0x87dcb5cb,// 28 PAY  25 

    0xd87c1530,// 29 PAY  26 

    0x31147349,// 30 PAY  27 

    0xd723cedd,// 31 PAY  28 

    0xcc172339,// 32 PAY  29 

    0x1b5636b2,// 33 PAY  30 

    0xbdee3f2d,// 34 PAY  31 

    0x7fbc9dae,// 35 PAY  32 

    0x8c8fa18b,// 36 PAY  33 

    0x3538836e,// 37 PAY  34 

    0xe0dc024b,// 38 PAY  35 

    0x7f16e52d,// 39 PAY  36 

    0x8f014c60,// 40 PAY  37 

    0x9e3e79f1,// 41 PAY  38 

    0xa7405c19,// 42 PAY  39 

    0x3b8f5f64,// 43 PAY  40 

    0xbd5ca0e5,// 44 PAY  41 

    0x0938e2cf,// 45 PAY  42 

    0xc5b8e5d8,// 46 PAY  43 

    0x9593172e,// 47 PAY  44 

    0xd36dc37c,// 48 PAY  45 

    0xfa32f360,// 49 PAY  46 

    0xadc77759,// 50 PAY  47 

    0x8eb1a3af,// 51 PAY  48 

    0x49a61e04,// 52 PAY  49 

    0x1193e711,// 53 PAY  50 

    0x385dd04d,// 54 PAY  51 

    0x6921b3a0,// 55 PAY  52 

    0xe68ddc79,// 56 PAY  53 

    0x9e7c8f2f,// 57 PAY  54 

    0xd75c9734,// 58 PAY  55 

    0x4253c645,// 59 PAY  56 

    0xa9f4ae8d,// 60 PAY  57 

    0x669b85ac,// 61 PAY  58 

    0x032eac44,// 62 PAY  59 

    0x82366b6d,// 63 PAY  60 

    0xfd82c6be,// 64 PAY  61 

    0x39fdd88c,// 65 PAY  62 

    0x5beff27c,// 66 PAY  63 

    0xe14dc57b,// 67 PAY  64 

    0x3b8f5928,// 68 PAY  65 

    0x9e9d7e6a,// 69 PAY  66 

    0x5bf21e5f,// 70 PAY  67 

    0xcff832a7,// 71 PAY  68 

    0xaf6f3af6,// 72 PAY  69 

    0xbe058445,// 73 PAY  70 

    0x6616a208,// 74 PAY  71 

    0x6744cb7e,// 75 PAY  72 

    0x7dd91422,// 76 PAY  73 

    0x645af8f7,// 77 PAY  74 

    0x46cbcda7,// 78 PAY  75 

    0xfed31748,// 79 PAY  76 

    0xf8101e10,// 80 PAY  77 

    0xbc5bb3d3,// 81 PAY  78 

    0x453575ae,// 82 PAY  79 

    0x9e03c156,// 83 PAY  80 

    0x2d155b85,// 84 PAY  81 

    0xce75039f,// 85 PAY  82 

    0x2e65a9f4,// 86 PAY  83 

    0x2d5c158b,// 87 PAY  84 

    0x602445a7,// 88 PAY  85 

    0xcd698322,// 89 PAY  86 

    0xc9988b12,// 90 PAY  87 

    0x76587638,// 91 PAY  88 

    0x784a3689,// 92 PAY  89 

    0xde0dfa88,// 93 PAY  90 

    0x9fafe282,// 94 PAY  91 

    0x7188fb36,// 95 PAY  92 

    0x75cb2f57,// 96 PAY  93 

    0xf2af323c,// 97 PAY  94 

    0xc0d9a1a0,// 98 PAY  95 

    0xd39adf3b,// 99 PAY  96 

    0x5fbbf3d3,// 100 PAY  97 

    0x1c45440b,// 101 PAY  98 

    0x236ed4dd,// 102 PAY  99 

    0xeaea3362,// 103 PAY 100 

    0xf0fbff52,// 104 PAY 101 

    0x96a89b9c,// 105 PAY 102 

    0xb418f0c4,// 106 PAY 103 

    0x1481632f,// 107 PAY 104 

    0x105c2766,// 108 PAY 105 

    0x0b7c0701,// 109 PAY 106 

    0x6435659c,// 110 PAY 107 

    0x9413d5dc,// 111 PAY 108 

    0x99ac87b5,// 112 PAY 109 

    0x82ef6eac,// 113 PAY 110 

    0x083e08ed,// 114 PAY 111 

    0x721e13aa,// 115 PAY 112 

    0xd280a9a9,// 116 PAY 113 

    0x0c1b9636,// 117 PAY 114 

    0xe107b3ac,// 118 PAY 115 

    0x74e8cc3c,// 119 PAY 116 

    0x971f6273,// 120 PAY 117 

    0xbb0ff2c3,// 121 PAY 118 

    0x68fe1a89,// 122 PAY 119 

    0xaacac681,// 123 PAY 120 

    0x8263ecc7,// 124 PAY 121 

    0x70234f8d,// 125 PAY 122 

    0x15136c37,// 126 PAY 123 

    0xb995ffd3,// 127 PAY 124 

    0xf8f76426,// 128 PAY 125 

    0x1ade06f8,// 129 PAY 126 

    0x86849671,// 130 PAY 127 

    0xe650466c,// 131 PAY 128 

    0x1bd15cd7,// 132 PAY 129 

    0x4da15c3d,// 133 PAY 130 

    0x4812ba27,// 134 PAY 131 

    0x6a01594f,// 135 PAY 132 

    0x67675106,// 136 PAY 133 

    0x5e26dad8,// 137 PAY 134 

    0x260b8789,// 138 PAY 135 

    0xbd941f50,// 139 PAY 136 

    0xf97e4d8d,// 140 PAY 137 

    0x428774bc,// 141 PAY 138 

    0x55ea8a94,// 142 PAY 139 

    0xccb5b21e,// 143 PAY 140 

    0x82ad1a26,// 144 PAY 141 

    0xf42cad16,// 145 PAY 142 

    0xbb4f6771,// 146 PAY 143 

    0xbcb18675,// 147 PAY 144 

    0xe59f9ad1,// 148 PAY 145 

    0x65d46656,// 149 PAY 146 

    0xb7e8f246,// 150 PAY 147 

    0x838d6dc1,// 151 PAY 148 

    0x7766d1de,// 152 PAY 149 

    0x366c3274,// 153 PAY 150 

    0xd6a10111,// 154 PAY 151 

    0xfc4b16ae,// 155 PAY 152 

    0xc896872a,// 156 PAY 153 

    0xe94e4111,// 157 PAY 154 

    0x44fb49d9,// 158 PAY 155 

    0x0fb5a840,// 159 PAY 156 

    0x374c8402,// 160 PAY 157 

    0x3912bce0,// 161 PAY 158 

    0x7566b35e,// 162 PAY 159 

    0xd3097b5a,// 163 PAY 160 

    0x44252411,// 164 PAY 161 

    0x6836aed2,// 165 PAY 162 

    0xdbedbd42,// 166 PAY 163 

    0x85933c7c,// 167 PAY 164 

    0x1662f03c,// 168 PAY 165 

    0x8b1b4095,// 169 PAY 166 

    0x545f2284,// 170 PAY 167 

    0x80aa38fd,// 171 PAY 168 

    0x219bbd05,// 172 PAY 169 

    0xbc3d82c5,// 173 PAY 170 

    0xe0401621,// 174 PAY 171 

    0x260e1f72,// 175 PAY 172 

    0x8828e657,// 176 PAY 173 

    0x3a5d9405,// 177 PAY 174 

    0xfab92df5,// 178 PAY 175 

    0x416dcb96,// 179 PAY 176 

    0x84dcfb4b,// 180 PAY 177 

    0x452f3747,// 181 PAY 178 

    0xadec8c12,// 182 PAY 179 

    0xd3e1804e,// 183 PAY 180 

    0x27fea89c,// 184 PAY 181 

    0x2a3f8221,// 185 PAY 182 

    0x665265ec,// 186 PAY 183 

    0x1b3d8440,// 187 PAY 184 

    0x9e2e81c4,// 188 PAY 185 

    0x924d396d,// 189 PAY 186 

    0x10b6bec3,// 190 PAY 187 

    0x7060337d,// 191 PAY 188 

    0x6106e77e,// 192 PAY 189 

    0x13c48f6a,// 193 PAY 190 

    0x86327c67,// 194 PAY 191 

    0x2ac703a2,// 195 PAY 192 

    0xfe3488f4,// 196 PAY 193 

    0x07aa95c1,// 197 PAY 194 

    0xdbfa38ad,// 198 PAY 195 

    0xc0c315a8,// 199 PAY 196 

    0x62de1d82,// 200 PAY 197 

    0x2b27db92,// 201 PAY 198 

    0xd09a8fb7,// 202 PAY 199 

    0xcdc9c6b9,// 203 PAY 200 

    0x5886a45e,// 204 PAY 201 

    0x226bc834,// 205 PAY 202 

    0x4dc815fd,// 206 PAY 203 

    0x3d44a25c,// 207 PAY 204 

    0xd7294c91,// 208 PAY 205 

    0xa5d4162b,// 209 PAY 206 

    0xfc53b719,// 210 PAY 207 

    0x5fb51e61,// 211 PAY 208 

    0x5fd2df75,// 212 PAY 209 

    0xe63acf60,// 213 PAY 210 

    0x03114939,// 214 PAY 211 

    0x952cd21b,// 215 PAY 212 

    0x65623fe0,// 216 PAY 213 

    0x49387c8b,// 217 PAY 214 

    0x2e10f560,// 218 PAY 215 

    0x49e61868,// 219 PAY 216 

    0x14296a24,// 220 PAY 217 

    0xffc44940,// 221 PAY 218 

    0x2b73c553,// 222 PAY 219 

    0x40ed6f0e,// 223 PAY 220 

    0xe34b4d29,// 224 PAY 221 

    0x891d4a05,// 225 PAY 222 

    0xe8d25b03,// 226 PAY 223 

    0xe1693faf,// 227 PAY 224 

    0x82688edf,// 228 PAY 225 

    0x7b720add,// 229 PAY 226 

    0x7ce800ee,// 230 PAY 227 

    0x213ab194,// 231 PAY 228 

    0x35a69f28,// 232 PAY 229 

    0x59006e24,// 233 PAY 230 

    0x7e3b917c,// 234 PAY 231 

    0xea022215,// 235 PAY 232 

    0x8e89f82a,// 236 PAY 233 

    0x52e8a053,// 237 PAY 234 

    0xfd000000,// 238 PAY 235 

/// HASH is  8 bytes 

    0x7dd91422,// 239 HSH   1 

    0x645af8f7,// 240 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 75 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x28 

    0x0111284b // 241 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt33_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 28 words. 

/// BDA size     is 106 (0x6a) 

/// BDA id       is 0x8db2 

    0x006a8db2,// 3 BDA   1 

/// PAY Generic Data size   : 106 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x2c16a15d,// 4 PAY   1 

    0x56af1582,// 5 PAY   2 

    0x1e5e319e,// 6 PAY   3 

    0xf7a70bf5,// 7 PAY   4 

    0x09ac55bc,// 8 PAY   5 

    0xdc2e4719,// 9 PAY   6 

    0xe839f09d,// 10 PAY   7 

    0x1ef204d2,// 11 PAY   8 

    0x19283c36,// 12 PAY   9 

    0x59456232,// 13 PAY  10 

    0xe17291d6,// 14 PAY  11 

    0x2f78c5d1,// 15 PAY  12 

    0x41b98066,// 16 PAY  13 

    0xcab2abd9,// 17 PAY  14 

    0xfae650da,// 18 PAY  15 

    0x460d6fae,// 19 PAY  16 

    0x8fb27665,// 20 PAY  17 

    0xe16c691e,// 21 PAY  18 

    0x42b923d1,// 22 PAY  19 

    0x1334d8ee,// 23 PAY  20 

    0xff41d971,// 24 PAY  21 

    0xf186da92,// 25 PAY  22 

    0xf7f9a24d,// 26 PAY  23 

    0x7a860f85,// 27 PAY  24 

    0xc9c75d11,// 28 PAY  25 

    0x4653d6f1,// 29 PAY  26 

    0x302d0000,// 30 PAY  27 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 141 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb2 

    0x0234b237 // 31 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt34_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 291 words. 

/// BDA size     is 1160 (0x488) 

/// BDA id       is 0xaed0 

    0x0488aed0,// 3 BDA   1 

/// PAY Generic Data size   : 1160 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x3c4fab63,// 4 PAY   1 

    0x4c7417d9,// 5 PAY   2 

    0x265850fa,// 6 PAY   3 

    0x97a84cd9,// 7 PAY   4 

    0x4115d20d,// 8 PAY   5 

    0x4a8fd5c4,// 9 PAY   6 

    0x03f1d461,// 10 PAY   7 

    0x3fe00874,// 11 PAY   8 

    0x23f810ee,// 12 PAY   9 

    0x02772939,// 13 PAY  10 

    0xf28163a0,// 14 PAY  11 

    0xd281652d,// 15 PAY  12 

    0x26caee56,// 16 PAY  13 

    0x4647be7c,// 17 PAY  14 

    0x7e48d899,// 18 PAY  15 

    0x9ffe818e,// 19 PAY  16 

    0xf1ad5475,// 20 PAY  17 

    0x85792f28,// 21 PAY  18 

    0x2e6578aa,// 22 PAY  19 

    0x780dc5ae,// 23 PAY  20 

    0xb73e9be6,// 24 PAY  21 

    0x3c2c7ea2,// 25 PAY  22 

    0x85292175,// 26 PAY  23 

    0x4c0de51d,// 27 PAY  24 

    0xa996d186,// 28 PAY  25 

    0xab372122,// 29 PAY  26 

    0xefd8a713,// 30 PAY  27 

    0x8d5adb3c,// 31 PAY  28 

    0xed60631d,// 32 PAY  29 

    0xe8ab2eb5,// 33 PAY  30 

    0x53f279bd,// 34 PAY  31 

    0x7633504d,// 35 PAY  32 

    0x6db0be43,// 36 PAY  33 

    0x468ce3a1,// 37 PAY  34 

    0x0c20627e,// 38 PAY  35 

    0x413d3dfa,// 39 PAY  36 

    0xfce2e5e0,// 40 PAY  37 

    0x78b13086,// 41 PAY  38 

    0xcff4181e,// 42 PAY  39 

    0x5c0bb034,// 43 PAY  40 

    0x153ca351,// 44 PAY  41 

    0xe57f88b1,// 45 PAY  42 

    0x9a8de605,// 46 PAY  43 

    0x2ce4f7eb,// 47 PAY  44 

    0x5bd20c2e,// 48 PAY  45 

    0xb0d75b35,// 49 PAY  46 

    0x87827035,// 50 PAY  47 

    0x60200d67,// 51 PAY  48 

    0x189dba77,// 52 PAY  49 

    0xe3073364,// 53 PAY  50 

    0xb670e29b,// 54 PAY  51 

    0x4715d7a1,// 55 PAY  52 

    0x64c562ee,// 56 PAY  53 

    0xdb864df9,// 57 PAY  54 

    0x6995f71e,// 58 PAY  55 

    0x28ee0cf7,// 59 PAY  56 

    0xe9720378,// 60 PAY  57 

    0xe741f744,// 61 PAY  58 

    0xcf38d564,// 62 PAY  59 

    0xf15b9367,// 63 PAY  60 

    0x033ca801,// 64 PAY  61 

    0x024bc96d,// 65 PAY  62 

    0x5c7a9c37,// 66 PAY  63 

    0x2d55fb40,// 67 PAY  64 

    0x93d42c3b,// 68 PAY  65 

    0x620881d9,// 69 PAY  66 

    0xdae23778,// 70 PAY  67 

    0x083d47fa,// 71 PAY  68 

    0xf43117d4,// 72 PAY  69 

    0x8cc59730,// 73 PAY  70 

    0x8ed6e7cf,// 74 PAY  71 

    0xdb8918c7,// 75 PAY  72 

    0xc7120b01,// 76 PAY  73 

    0xdb2ce2c1,// 77 PAY  74 

    0x7c57a491,// 78 PAY  75 

    0xe6b1892c,// 79 PAY  76 

    0xd6faba16,// 80 PAY  77 

    0x8a8b8d07,// 81 PAY  78 

    0xf44baee7,// 82 PAY  79 

    0x47b33743,// 83 PAY  80 

    0x9622831d,// 84 PAY  81 

    0xef61d23f,// 85 PAY  82 

    0xe6a63587,// 86 PAY  83 

    0xbf2eea9f,// 87 PAY  84 

    0x9089d1bd,// 88 PAY  85 

    0xfe66a3ef,// 89 PAY  86 

    0xe0d2e7fb,// 90 PAY  87 

    0x9753e0ba,// 91 PAY  88 

    0x4ced21bd,// 92 PAY  89 

    0x5402124b,// 93 PAY  90 

    0x0bebf9b7,// 94 PAY  91 

    0x6a4108a4,// 95 PAY  92 

    0x06e7a17c,// 96 PAY  93 

    0x8b66b2ad,// 97 PAY  94 

    0x5c6a9934,// 98 PAY  95 

    0x4f0da627,// 99 PAY  96 

    0x2d927650,// 100 PAY  97 

    0xbdb23acc,// 101 PAY  98 

    0x497cd366,// 102 PAY  99 

    0xd8c2ea06,// 103 PAY 100 

    0x63894e4b,// 104 PAY 101 

    0x234c5d2e,// 105 PAY 102 

    0xa1e6304b,// 106 PAY 103 

    0x897d49bc,// 107 PAY 104 

    0x5e99e37f,// 108 PAY 105 

    0xaa8d298d,// 109 PAY 106 

    0xae579db8,// 110 PAY 107 

    0xdbc952ff,// 111 PAY 108 

    0x307b1815,// 112 PAY 109 

    0xc92b4171,// 113 PAY 110 

    0x5335bf4e,// 114 PAY 111 

    0x12834bfd,// 115 PAY 112 

    0xbc8980ff,// 116 PAY 113 

    0xe0e77304,// 117 PAY 114 

    0x958c225d,// 118 PAY 115 

    0x06ea9dbd,// 119 PAY 116 

    0xb1d69144,// 120 PAY 117 

    0xee5c30c9,// 121 PAY 118 

    0x61dfe844,// 122 PAY 119 

    0x3bed9481,// 123 PAY 120 

    0x1699de74,// 124 PAY 121 

    0xc2d048c9,// 125 PAY 122 

    0xdfd89f6b,// 126 PAY 123 

    0x5843d050,// 127 PAY 124 

    0xe4baead4,// 128 PAY 125 

    0xf973edb7,// 129 PAY 126 

    0x5cc0fe65,// 130 PAY 127 

    0x8c61a2da,// 131 PAY 128 

    0x90e5eb4f,// 132 PAY 129 

    0xe64b16a4,// 133 PAY 130 

    0x1172537c,// 134 PAY 131 

    0x5d2d2557,// 135 PAY 132 

    0x7f964d2f,// 136 PAY 133 

    0x45de378e,// 137 PAY 134 

    0xa79a659a,// 138 PAY 135 

    0xec3562cd,// 139 PAY 136 

    0x576da7da,// 140 PAY 137 

    0xdaafbdd9,// 141 PAY 138 

    0x53176c00,// 142 PAY 139 

    0x36db8346,// 143 PAY 140 

    0x1faf8aeb,// 144 PAY 141 

    0xd39db41d,// 145 PAY 142 

    0x758b22fe,// 146 PAY 143 

    0xb7b835cf,// 147 PAY 144 

    0xb5d02e02,// 148 PAY 145 

    0xa379399f,// 149 PAY 146 

    0xf76cb7f7,// 150 PAY 147 

    0x9c05151d,// 151 PAY 148 

    0x00f61d74,// 152 PAY 149 

    0x8789b47f,// 153 PAY 150 

    0xb179c165,// 154 PAY 151 

    0xee722c3a,// 155 PAY 152 

    0x19c4c39f,// 156 PAY 153 

    0x36e6283d,// 157 PAY 154 

    0x77b79a6d,// 158 PAY 155 

    0x646302d9,// 159 PAY 156 

    0x002b562d,// 160 PAY 157 

    0x07a60b9c,// 161 PAY 158 

    0xb7cfbae2,// 162 PAY 159 

    0xf6be1e5e,// 163 PAY 160 

    0xb07d34b6,// 164 PAY 161 

    0xa1ac62ac,// 165 PAY 162 

    0x86fdc6b6,// 166 PAY 163 

    0xed34a507,// 167 PAY 164 

    0x51ffc841,// 168 PAY 165 

    0x42f3a3a2,// 169 PAY 166 

    0x5d6af412,// 170 PAY 167 

    0x80f123b9,// 171 PAY 168 

    0x4d5f1635,// 172 PAY 169 

    0x8dc374cc,// 173 PAY 170 

    0x3a774298,// 174 PAY 171 

    0x8ca436d3,// 175 PAY 172 

    0xe27f9a8c,// 176 PAY 173 

    0xd1a3e71d,// 177 PAY 174 

    0x26cf5109,// 178 PAY 175 

    0x591d7de6,// 179 PAY 176 

    0x6e6eabdf,// 180 PAY 177 

    0x20617b89,// 181 PAY 178 

    0x99f4a0e7,// 182 PAY 179 

    0x16e873b9,// 183 PAY 180 

    0x3e9f6816,// 184 PAY 181 

    0x3a4b732b,// 185 PAY 182 

    0x3373314e,// 186 PAY 183 

    0x631c0a70,// 187 PAY 184 

    0x4cc3d4b9,// 188 PAY 185 

    0x4b3ce583,// 189 PAY 186 

    0x10d1fc0e,// 190 PAY 187 

    0x46b0f5e0,// 191 PAY 188 

    0xc1042e76,// 192 PAY 189 

    0x97a9d63a,// 193 PAY 190 

    0x495ba8fd,// 194 PAY 191 

    0x770f3f78,// 195 PAY 192 

    0x02014a83,// 196 PAY 193 

    0x87f9b7d2,// 197 PAY 194 

    0x646bf453,// 198 PAY 195 

    0xfae483d6,// 199 PAY 196 

    0xde1d00c1,// 200 PAY 197 

    0xea8fd1f6,// 201 PAY 198 

    0xfc070852,// 202 PAY 199 

    0xb756e10e,// 203 PAY 200 

    0x5a04268f,// 204 PAY 201 

    0x819964c5,// 205 PAY 202 

    0x374a08cf,// 206 PAY 203 

    0xfca99a9f,// 207 PAY 204 

    0xc32d793b,// 208 PAY 205 

    0x24cc4e08,// 209 PAY 206 

    0x38acfc82,// 210 PAY 207 

    0x3cf85c8b,// 211 PAY 208 

    0x99c5c357,// 212 PAY 209 

    0x69fd49b5,// 213 PAY 210 

    0xc9a15161,// 214 PAY 211 

    0xed60056d,// 215 PAY 212 

    0x3d2378f7,// 216 PAY 213 

    0x334b542e,// 217 PAY 214 

    0xe226f81d,// 218 PAY 215 

    0x2a95778f,// 219 PAY 216 

    0x691ace4d,// 220 PAY 217 

    0x7d780527,// 221 PAY 218 

    0xbe79f979,// 222 PAY 219 

    0xc4c2c52e,// 223 PAY 220 

    0xc6e8b61c,// 224 PAY 221 

    0xde37f5da,// 225 PAY 222 

    0xafc9efd3,// 226 PAY 223 

    0xc5df0b4a,// 227 PAY 224 

    0x6a888dcd,// 228 PAY 225 

    0x57314a70,// 229 PAY 226 

    0xb959f31e,// 230 PAY 227 

    0x4bad0cd8,// 231 PAY 228 

    0x29b02a8a,// 232 PAY 229 

    0x7ac08633,// 233 PAY 230 

    0xb1d71bb3,// 234 PAY 231 

    0x1847b7ab,// 235 PAY 232 

    0xf7aee9cf,// 236 PAY 233 

    0xe70581f7,// 237 PAY 234 

    0x51d818bf,// 238 PAY 235 

    0x56f80f10,// 239 PAY 236 

    0x26eece91,// 240 PAY 237 

    0x20b82882,// 241 PAY 238 

    0xdbf58f62,// 242 PAY 239 

    0xc99140e1,// 243 PAY 240 

    0xaf8612cd,// 244 PAY 241 

    0x5288e3db,// 245 PAY 242 

    0x432e756e,// 246 PAY 243 

    0x3069ccc2,// 247 PAY 244 

    0x5f65bc1f,// 248 PAY 245 

    0x8ec92b04,// 249 PAY 246 

    0x626274fb,// 250 PAY 247 

    0xba5516ed,// 251 PAY 248 

    0x4c942479,// 252 PAY 249 

    0xc1802428,// 253 PAY 250 

    0x471987d9,// 254 PAY 251 

    0xa591a105,// 255 PAY 252 

    0x1e60b280,// 256 PAY 253 

    0x2cc44562,// 257 PAY 254 

    0x5d9df1a8,// 258 PAY 255 

    0x9ccfae6e,// 259 PAY 256 

    0x2e209636,// 260 PAY 257 

    0x96ccb8f6,// 261 PAY 258 

    0xe281d869,// 262 PAY 259 

    0x6c6ed117,// 263 PAY 260 

    0x0e6edc16,// 264 PAY 261 

    0x8c2ebd92,// 265 PAY 262 

    0xd51f5818,// 266 PAY 263 

    0xc4452bc9,// 267 PAY 264 

    0x49c671e0,// 268 PAY 265 

    0x6aba6f1b,// 269 PAY 266 

    0x12efe8d9,// 270 PAY 267 

    0x6f5ce336,// 271 PAY 268 

    0xf527c7a5,// 272 PAY 269 

    0x6512b0f5,// 273 PAY 270 

    0x58faf01d,// 274 PAY 271 

    0x212ce813,// 275 PAY 272 

    0x6a18f855,// 276 PAY 273 

    0x412ab56d,// 277 PAY 274 

    0xba33e98c,// 278 PAY 275 

    0x0e582869,// 279 PAY 276 

    0x6cda8c7a,// 280 PAY 277 

    0x308565ad,// 281 PAY 278 

    0xd229816c,// 282 PAY 279 

    0x1cdecb92,// 283 PAY 280 

    0x5e3f81c2,// 284 PAY 281 

    0x1d340b65,// 285 PAY 282 

    0x6dee13c9,// 286 PAY 283 

    0xd8015b72,// 287 PAY 284 

    0xa4753683,// 288 PAY 285 

    0x7006bd14,// 289 PAY 286 

    0x5a6eda60,// 290 PAY 287 

    0x17e7b1ea,// 291 PAY 288 

    0x1020f116,// 292 PAY 289 

    0x2837ed3e,// 293 PAY 290 

/// HASH is  12 bytes 

    0x5a6eda60,// 294 HSH   1 

    0x17e7b1ea,// 295 HSH   2 

    0x1020f116,// 296 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 84 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3d 

    0x02b53d54 // 297 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt35_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 71 words. 

/// BDA size     is 279 (0x117) 

/// BDA id       is 0x41dc 

    0x011741dc,// 3 BDA   1 

/// PAY Generic Data size   : 279 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x4b5cf2b9,// 4 PAY   1 

    0x870b2af2,// 5 PAY   2 

    0x4a3c9a55,// 6 PAY   3 

    0x9acd9659,// 7 PAY   4 

    0xdf7c9829,// 8 PAY   5 

    0xc20c84fa,// 9 PAY   6 

    0x34a94eb8,// 10 PAY   7 

    0x8b19d8b2,// 11 PAY   8 

    0x59b454c8,// 12 PAY   9 

    0x9cc82386,// 13 PAY  10 

    0xf637c6b2,// 14 PAY  11 

    0xa4fb8512,// 15 PAY  12 

    0x12e9e44c,// 16 PAY  13 

    0xcaf02f19,// 17 PAY  14 

    0x8a8754b0,// 18 PAY  15 

    0x302c32c5,// 19 PAY  16 

    0x2d7e9efe,// 20 PAY  17 

    0x868d5c01,// 21 PAY  18 

    0xc6031c90,// 22 PAY  19 

    0x2749fed5,// 23 PAY  20 

    0xa404911c,// 24 PAY  21 

    0x2596d519,// 25 PAY  22 

    0xf9c37ba7,// 26 PAY  23 

    0x492430f1,// 27 PAY  24 

    0xe85075aa,// 28 PAY  25 

    0x4df9d25a,// 29 PAY  26 

    0xe7dbb4d2,// 30 PAY  27 

    0x2dc33677,// 31 PAY  28 

    0xd0778dc7,// 32 PAY  29 

    0x7e7ba748,// 33 PAY  30 

    0x66888604,// 34 PAY  31 

    0x44beba7c,// 35 PAY  32 

    0x1566b695,// 36 PAY  33 

    0x72c369c8,// 37 PAY  34 

    0x10c267d7,// 38 PAY  35 

    0x18adea9c,// 39 PAY  36 

    0x88420930,// 40 PAY  37 

    0x5bdc912c,// 41 PAY  38 

    0x0c8663da,// 42 PAY  39 

    0x02427f8b,// 43 PAY  40 

    0xa8ae4598,// 44 PAY  41 

    0x9fd77510,// 45 PAY  42 

    0xdc33d748,// 46 PAY  43 

    0x0b7cd491,// 47 PAY  44 

    0xddbd3201,// 48 PAY  45 

    0x349f76d1,// 49 PAY  46 

    0xa4bf0f54,// 50 PAY  47 

    0x86450c88,// 51 PAY  48 

    0x331135ee,// 52 PAY  49 

    0x092f0aaa,// 53 PAY  50 

    0xc08b19f3,// 54 PAY  51 

    0xe4ea32c3,// 55 PAY  52 

    0xdf0597ac,// 56 PAY  53 

    0x9332fa23,// 57 PAY  54 

    0x9f682c16,// 58 PAY  55 

    0x12e6bd2a,// 59 PAY  56 

    0x83eeeb7f,// 60 PAY  57 

    0x013d6cee,// 61 PAY  58 

    0xed077720,// 62 PAY  59 

    0x286fd9d1,// 63 PAY  60 

    0x373d3e18,// 64 PAY  61 

    0x05507c46,// 65 PAY  62 

    0xdee1eb01,// 66 PAY  63 

    0xe524699e,// 67 PAY  64 

    0x479a6ba5,// 68 PAY  65 

    0x3ed4344e,// 69 PAY  66 

    0xba382615,// 70 PAY  67 

    0x674a20b8,// 71 PAY  68 

    0x34b7e0ba,// 72 PAY  69 

    0x4a423800,// 73 PAY  70 

/// HASH is  8 bytes 

    0x479a6ba5,// 74 HSH   1 

    0x3ed4344e,// 75 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 7 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xfa 

    0x001cfad6 // 76 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt36_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 349 words. 

/// BDA size     is 1392 (0x570) 

/// BDA id       is 0xda48 

    0x0570da48,// 3 BDA   1 

/// PAY Generic Data size   : 1392 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x80f7aad4,// 4 PAY   1 

    0x33ec42f2,// 5 PAY   2 

    0x246953fa,// 6 PAY   3 

    0x42dc9453,// 7 PAY   4 

    0xbd4593ba,// 8 PAY   5 

    0x2eafb682,// 9 PAY   6 

    0x994f83b0,// 10 PAY   7 

    0x68fca5b4,// 11 PAY   8 

    0x1e51dd88,// 12 PAY   9 

    0x7d158fff,// 13 PAY  10 

    0x3e3c66b4,// 14 PAY  11 

    0xb4e89601,// 15 PAY  12 

    0xe50a873a,// 16 PAY  13 

    0x56b51668,// 17 PAY  14 

    0x82a949b9,// 18 PAY  15 

    0xcf85cf6e,// 19 PAY  16 

    0x8483a78c,// 20 PAY  17 

    0xf77005ce,// 21 PAY  18 

    0x89453741,// 22 PAY  19 

    0x35a6212d,// 23 PAY  20 

    0x4188ae53,// 24 PAY  21 

    0x53109139,// 25 PAY  22 

    0x688b60de,// 26 PAY  23 

    0xb26d5207,// 27 PAY  24 

    0xd979cbe0,// 28 PAY  25 

    0x877ffa18,// 29 PAY  26 

    0x289aa95d,// 30 PAY  27 

    0xaa489736,// 31 PAY  28 

    0x457bd882,// 32 PAY  29 

    0x5f9e83ee,// 33 PAY  30 

    0xc0fbfe3f,// 34 PAY  31 

    0x5614cc74,// 35 PAY  32 

    0xc1954565,// 36 PAY  33 

    0x2dc693a9,// 37 PAY  34 

    0xf9057d40,// 38 PAY  35 

    0xdf5bda80,// 39 PAY  36 

    0x8b232273,// 40 PAY  37 

    0xa7e088d8,// 41 PAY  38 

    0x22b92393,// 42 PAY  39 

    0x90b118fe,// 43 PAY  40 

    0xfc8113de,// 44 PAY  41 

    0x9a557a81,// 45 PAY  42 

    0xd4591149,// 46 PAY  43 

    0xd76b35a1,// 47 PAY  44 

    0x14ffb0f3,// 48 PAY  45 

    0x8ded7388,// 49 PAY  46 

    0x43df54c5,// 50 PAY  47 

    0xa28a8787,// 51 PAY  48 

    0x5e191b76,// 52 PAY  49 

    0xb5e462b3,// 53 PAY  50 

    0xe66ddd28,// 54 PAY  51 

    0xae2489a1,// 55 PAY  52 

    0xa7366bd5,// 56 PAY  53 

    0xd0ab396e,// 57 PAY  54 

    0x83c12459,// 58 PAY  55 

    0x93514a95,// 59 PAY  56 

    0x53f441c4,// 60 PAY  57 

    0x4bbbab34,// 61 PAY  58 

    0x42c145c1,// 62 PAY  59 

    0xeb529e17,// 63 PAY  60 

    0x73ad364b,// 64 PAY  61 

    0xa4287dd9,// 65 PAY  62 

    0xff0b1a86,// 66 PAY  63 

    0xd6edc392,// 67 PAY  64 

    0xc523fa07,// 68 PAY  65 

    0x02c7d024,// 69 PAY  66 

    0xa2e98ab6,// 70 PAY  67 

    0x3404c55e,// 71 PAY  68 

    0xed8c3379,// 72 PAY  69 

    0x043bf689,// 73 PAY  70 

    0x8a2d9ed2,// 74 PAY  71 

    0xab52c933,// 75 PAY  72 

    0x3576fef7,// 76 PAY  73 

    0x3de63b26,// 77 PAY  74 

    0xc8907638,// 78 PAY  75 

    0x4d47370a,// 79 PAY  76 

    0xea8c6ef3,// 80 PAY  77 

    0xdd237b07,// 81 PAY  78 

    0xcc9e318d,// 82 PAY  79 

    0xd2e00c73,// 83 PAY  80 

    0x6d7d18ba,// 84 PAY  81 

    0x78ef0c89,// 85 PAY  82 

    0xe6a93253,// 86 PAY  83 

    0x742dc0d7,// 87 PAY  84 

    0x163e9cd9,// 88 PAY  85 

    0x85138daa,// 89 PAY  86 

    0x11c5a6c9,// 90 PAY  87 

    0x1ff947ba,// 91 PAY  88 

    0xe2cb008f,// 92 PAY  89 

    0x9ac3cbd3,// 93 PAY  90 

    0xcf96ff4e,// 94 PAY  91 

    0x5cde7d9e,// 95 PAY  92 

    0xa041242f,// 96 PAY  93 

    0xce91deb6,// 97 PAY  94 

    0xf7a075df,// 98 PAY  95 

    0x8bfc4dc5,// 99 PAY  96 

    0x09743f8b,// 100 PAY  97 

    0x9ca1f7af,// 101 PAY  98 

    0x07c95f32,// 102 PAY  99 

    0x941a742c,// 103 PAY 100 

    0x52f2c505,// 104 PAY 101 

    0x8c717513,// 105 PAY 102 

    0x7d114fd5,// 106 PAY 103 

    0x04193995,// 107 PAY 104 

    0x1bb58c9c,// 108 PAY 105 

    0xcfbeb79e,// 109 PAY 106 

    0xa3a777c7,// 110 PAY 107 

    0x37d1d3c5,// 111 PAY 108 

    0xf5ac4b3a,// 112 PAY 109 

    0x7fc9549e,// 113 PAY 110 

    0xc0028a41,// 114 PAY 111 

    0x318caebd,// 115 PAY 112 

    0xf33194de,// 116 PAY 113 

    0xf8070721,// 117 PAY 114 

    0xba9e9fb9,// 118 PAY 115 

    0x340e1c88,// 119 PAY 116 

    0xeefb0350,// 120 PAY 117 

    0x6de9555d,// 121 PAY 118 

    0x99dff289,// 122 PAY 119 

    0x5a65e188,// 123 PAY 120 

    0x81624a9d,// 124 PAY 121 

    0xd23d4dd0,// 125 PAY 122 

    0x7f27a065,// 126 PAY 123 

    0x3eb63a8f,// 127 PAY 124 

    0x5e47e6ff,// 128 PAY 125 

    0x8b6575ae,// 129 PAY 126 

    0x439428a8,// 130 PAY 127 

    0xdc3f413c,// 131 PAY 128 

    0xb02aa103,// 132 PAY 129 

    0xa87d40ea,// 133 PAY 130 

    0x7f665465,// 134 PAY 131 

    0xe4922c2c,// 135 PAY 132 

    0x009cdd3a,// 136 PAY 133 

    0x3c6d14bd,// 137 PAY 134 

    0x49bf5e78,// 138 PAY 135 

    0xcc53e25f,// 139 PAY 136 

    0xe4d9c1cf,// 140 PAY 137 

    0xef256d9f,// 141 PAY 138 

    0x981af0b0,// 142 PAY 139 

    0xc39104be,// 143 PAY 140 

    0xa3c705db,// 144 PAY 141 

    0x4639a7d1,// 145 PAY 142 

    0x8ea8355f,// 146 PAY 143 

    0x642f54bc,// 147 PAY 144 

    0x070c33ab,// 148 PAY 145 

    0xeac31928,// 149 PAY 146 

    0x742c1f25,// 150 PAY 147 

    0x0075466a,// 151 PAY 148 

    0xef8421cc,// 152 PAY 149 

    0x5837074b,// 153 PAY 150 

    0x8fd347a6,// 154 PAY 151 

    0xf847658e,// 155 PAY 152 

    0x24ff00b7,// 156 PAY 153 

    0x3a024f73,// 157 PAY 154 

    0x39dac282,// 158 PAY 155 

    0x454fb5b1,// 159 PAY 156 

    0xd5a7a97b,// 160 PAY 157 

    0xdb488102,// 161 PAY 158 

    0x2b257695,// 162 PAY 159 

    0x094dbb3f,// 163 PAY 160 

    0x111111de,// 164 PAY 161 

    0xb69bb39f,// 165 PAY 162 

    0x2c200f0b,// 166 PAY 163 

    0xc34b42e1,// 167 PAY 164 

    0xdb707342,// 168 PAY 165 

    0x202fe9e2,// 169 PAY 166 

    0xe6239ba8,// 170 PAY 167 

    0x48980e12,// 171 PAY 168 

    0xdff5b057,// 172 PAY 169 

    0xac345ced,// 173 PAY 170 

    0xd0c5d626,// 174 PAY 171 

    0xa9abb0ec,// 175 PAY 172 

    0x7aaffd01,// 176 PAY 173 

    0x78ff7fed,// 177 PAY 174 

    0x9abe8881,// 178 PAY 175 

    0xf4e2125d,// 179 PAY 176 

    0xd891f4de,// 180 PAY 177 

    0xd56a8174,// 181 PAY 178 

    0x0df4bb4d,// 182 PAY 179 

    0xb4d6a385,// 183 PAY 180 

    0xfed43e67,// 184 PAY 181 

    0x58587096,// 185 PAY 182 

    0x4f82db3f,// 186 PAY 183 

    0xb801c09c,// 187 PAY 184 

    0xc0b9b0aa,// 188 PAY 185 

    0xfa822306,// 189 PAY 186 

    0xa08906e2,// 190 PAY 187 

    0x8c8325df,// 191 PAY 188 

    0x08eeb756,// 192 PAY 189 

    0x7483e803,// 193 PAY 190 

    0x0736acef,// 194 PAY 191 

    0xcc2135bb,// 195 PAY 192 

    0x7d3f31ff,// 196 PAY 193 

    0xb37f9211,// 197 PAY 194 

    0xf1c88eeb,// 198 PAY 195 

    0xe800f012,// 199 PAY 196 

    0xb291de34,// 200 PAY 197 

    0x686734d7,// 201 PAY 198 

    0x83b8cc25,// 202 PAY 199 

    0x98f95149,// 203 PAY 200 

    0x2d775dee,// 204 PAY 201 

    0x3cde4ba5,// 205 PAY 202 

    0x9959d9fe,// 206 PAY 203 

    0xc1b711e6,// 207 PAY 204 

    0x2e02d654,// 208 PAY 205 

    0x01be395d,// 209 PAY 206 

    0xebbbf3c2,// 210 PAY 207 

    0x8506a3f5,// 211 PAY 208 

    0x3cfdd87e,// 212 PAY 209 

    0x510891a1,// 213 PAY 210 

    0x04d29623,// 214 PAY 211 

    0x97ec115f,// 215 PAY 212 

    0xd3649ae5,// 216 PAY 213 

    0x4fb7a429,// 217 PAY 214 

    0x28f4d97a,// 218 PAY 215 

    0x09a43eb5,// 219 PAY 216 

    0xac609d6c,// 220 PAY 217 

    0x31e37f16,// 221 PAY 218 

    0x1f86829d,// 222 PAY 219 

    0x0cc21463,// 223 PAY 220 

    0xe50d48a5,// 224 PAY 221 

    0x2503d36e,// 225 PAY 222 

    0x313d7d5b,// 226 PAY 223 

    0x485e1ae1,// 227 PAY 224 

    0xf560ab64,// 228 PAY 225 

    0xcedd1ca2,// 229 PAY 226 

    0x2df9702a,// 230 PAY 227 

    0x90c8f06d,// 231 PAY 228 

    0xc312828d,// 232 PAY 229 

    0x1597ec15,// 233 PAY 230 

    0x212013ba,// 234 PAY 231 

    0x4eb2dea2,// 235 PAY 232 

    0x707674e0,// 236 PAY 233 

    0xce3b5cf1,// 237 PAY 234 

    0x09400d9c,// 238 PAY 235 

    0xf96d329d,// 239 PAY 236 

    0xbd1dc68b,// 240 PAY 237 

    0x692d9f99,// 241 PAY 238 

    0x6da97b0e,// 242 PAY 239 

    0xfda48aca,// 243 PAY 240 

    0xe8a059a2,// 244 PAY 241 

    0x7f3d9221,// 245 PAY 242 

    0x92016f3d,// 246 PAY 243 

    0x20ef65a9,// 247 PAY 244 

    0xe7faf5fc,// 248 PAY 245 

    0x617cb335,// 249 PAY 246 

    0x0b9baa6b,// 250 PAY 247 

    0x78107619,// 251 PAY 248 

    0x27911ee8,// 252 PAY 249 

    0x51f8f823,// 253 PAY 250 

    0xa98e5aee,// 254 PAY 251 

    0x06121f9d,// 255 PAY 252 

    0x45e01e5c,// 256 PAY 253 

    0x3a7cac98,// 257 PAY 254 

    0x24cc67c8,// 258 PAY 255 

    0x7c53579d,// 259 PAY 256 

    0xb729f9a7,// 260 PAY 257 

    0xc7650f23,// 261 PAY 258 

    0x64d5a972,// 262 PAY 259 

    0xad3e53c6,// 263 PAY 260 

    0x6184caea,// 264 PAY 261 

    0x6ae7912b,// 265 PAY 262 

    0x88423ceb,// 266 PAY 263 

    0x969def02,// 267 PAY 264 

    0x810c71bd,// 268 PAY 265 

    0xfd7e6d8b,// 269 PAY 266 

    0x6e33e05e,// 270 PAY 267 

    0xd2a5827a,// 271 PAY 268 

    0xb8943cb7,// 272 PAY 269 

    0xd623c823,// 273 PAY 270 

    0x81f7aeaa,// 274 PAY 271 

    0x006fcb2a,// 275 PAY 272 

    0xca73b8d1,// 276 PAY 273 

    0x45258ad8,// 277 PAY 274 

    0x5d0242c4,// 278 PAY 275 

    0xd9180b63,// 279 PAY 276 

    0xe2ac5c9f,// 280 PAY 277 

    0x74f83b9a,// 281 PAY 278 

    0xcbd52057,// 282 PAY 279 

    0x77cb2dfc,// 283 PAY 280 

    0x0a1dabde,// 284 PAY 281 

    0x0330328a,// 285 PAY 282 

    0xb928330d,// 286 PAY 283 

    0x048f83f9,// 287 PAY 284 

    0xa90fc135,// 288 PAY 285 

    0x01139ee0,// 289 PAY 286 

    0x0463bede,// 290 PAY 287 

    0x3308d0a2,// 291 PAY 288 

    0x00cb7bb7,// 292 PAY 289 

    0xc2c1d325,// 293 PAY 290 

    0x70a56cdf,// 294 PAY 291 

    0x35a26485,// 295 PAY 292 

    0x3ed70dd2,// 296 PAY 293 

    0xa8830d11,// 297 PAY 294 

    0x60fc214c,// 298 PAY 295 

    0xe74a7ac2,// 299 PAY 296 

    0xba26baff,// 300 PAY 297 

    0x9629dcb9,// 301 PAY 298 

    0xeb943f2b,// 302 PAY 299 

    0x719f3e71,// 303 PAY 300 

    0x063df3fe,// 304 PAY 301 

    0xdc6920ad,// 305 PAY 302 

    0xbd096e5d,// 306 PAY 303 

    0x1837bda2,// 307 PAY 304 

    0x4d5c0771,// 308 PAY 305 

    0x450c0c15,// 309 PAY 306 

    0xad50db61,// 310 PAY 307 

    0xe30b1739,// 311 PAY 308 

    0x2aae2ab0,// 312 PAY 309 

    0x080cd3c0,// 313 PAY 310 

    0x50ccd98c,// 314 PAY 311 

    0xf59bcfc9,// 315 PAY 312 

    0xa6a29821,// 316 PAY 313 

    0xf72e28e3,// 317 PAY 314 

    0x12411e9b,// 318 PAY 315 

    0x67f63079,// 319 PAY 316 

    0x6ff2ee0c,// 320 PAY 317 

    0x338a9c54,// 321 PAY 318 

    0x2b9675c2,// 322 PAY 319 

    0xf449cf15,// 323 PAY 320 

    0x0b61d374,// 324 PAY 321 

    0x4b9cd0df,// 325 PAY 322 

    0x64042c24,// 326 PAY 323 

    0xd5be6c18,// 327 PAY 324 

    0x67069072,// 328 PAY 325 

    0xcdb8902a,// 329 PAY 326 

    0x7a9a32de,// 330 PAY 327 

    0x6dc0204d,// 331 PAY 328 

    0x2cb331a6,// 332 PAY 329 

    0xbeeb437e,// 333 PAY 330 

    0x39742dc0,// 334 PAY 331 

    0xaf51e654,// 335 PAY 332 

    0xdbffe4e1,// 336 PAY 333 

    0x85d49d20,// 337 PAY 334 

    0x3b7be657,// 338 PAY 335 

    0xc800a305,// 339 PAY 336 

    0x5343717f,// 340 PAY 337 

    0xbdebf321,// 341 PAY 338 

    0x761c3464,// 342 PAY 339 

    0x0b2b9626,// 343 PAY 340 

    0x3ffa9ddd,// 344 PAY 341 

    0x91ca9822,// 345 PAY 342 

    0x762e0cb6,// 346 PAY 343 

    0x965bfdc3,// 347 PAY 344 

    0x4d63d4a5,// 348 PAY 345 

    0x0227d28d,// 349 PAY 346 

    0x91f0ce97,// 350 PAY 347 

    0x58edf9a9,// 351 PAY 348 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x32 

    0x03f932b2 // 352 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt37_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 23 words. 

/// BDA size     is 86 (0x56) 

/// BDA id       is 0x1608 

    0x00561608,// 3 BDA   1 

/// PAY Generic Data size   : 86 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x9b54f10e,// 4 PAY   1 

    0xb311258a,// 5 PAY   2 

    0x24f401ac,// 6 PAY   3 

    0xa27a9186,// 7 PAY   4 

    0x4d1aba83,// 8 PAY   5 

    0xaee26fc7,// 9 PAY   6 

    0x1baf42e9,// 10 PAY   7 

    0x9a964726,// 11 PAY   8 

    0x736abb2e,// 12 PAY   9 

    0x5108da13,// 13 PAY  10 

    0xaaac6fce,// 14 PAY  11 

    0x0a3e2886,// 15 PAY  12 

    0xeb6882da,// 16 PAY  13 

    0x17f748be,// 17 PAY  14 

    0xb3cd5eea,// 18 PAY  15 

    0x60333655,// 19 PAY  16 

    0xdf522200,// 20 PAY  17 

    0xd40a0eea,// 21 PAY  18 

    0x7dc67fa3,// 22 PAY  19 

    0x949dde55,// 23 PAY  20 

    0x1d52739c,// 24 PAY  21 

    0x5ca40000,// 25 PAY  22 

/// HASH is  4 bytes 

    0xdf522200,// 26 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 164 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x0161bea4 // 27 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt38_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 259 words. 

/// BDA size     is 1031 (0x407) 

/// BDA id       is 0x1528 

    0x04071528,// 3 BDA   1 

/// PAY Generic Data size   : 1031 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf26ef274,// 4 PAY   1 

    0xffa89643,// 5 PAY   2 

    0x31d89a7d,// 6 PAY   3 

    0xbc09a0c2,// 7 PAY   4 

    0x8fa98142,// 8 PAY   5 

    0xe8f73d43,// 9 PAY   6 

    0xcb69b4c0,// 10 PAY   7 

    0xb7421b2e,// 11 PAY   8 

    0xb5fe5956,// 12 PAY   9 

    0x5907170a,// 13 PAY  10 

    0x7ef424e0,// 14 PAY  11 

    0x1dfc8d13,// 15 PAY  12 

    0xd4e9f7c7,// 16 PAY  13 

    0x394764ed,// 17 PAY  14 

    0x7c8f6a09,// 18 PAY  15 

    0x96a4cd11,// 19 PAY  16 

    0xf3b0f02a,// 20 PAY  17 

    0x6720ed6f,// 21 PAY  18 

    0x3ce026be,// 22 PAY  19 

    0xaca972aa,// 23 PAY  20 

    0x3bce0200,// 24 PAY  21 

    0xc54c9cd7,// 25 PAY  22 

    0xb39983fe,// 26 PAY  23 

    0xafb6f4b7,// 27 PAY  24 

    0x224df8b2,// 28 PAY  25 

    0xa016041a,// 29 PAY  26 

    0xbae8eb8e,// 30 PAY  27 

    0x6f4512c5,// 31 PAY  28 

    0x76693b02,// 32 PAY  29 

    0x8468a200,// 33 PAY  30 

    0x733a41e6,// 34 PAY  31 

    0x12a0b459,// 35 PAY  32 

    0x2bfa143f,// 36 PAY  33 

    0x312c6ab0,// 37 PAY  34 

    0x81942601,// 38 PAY  35 

    0x551127d8,// 39 PAY  36 

    0x8252b2a8,// 40 PAY  37 

    0x9c2afdf3,// 41 PAY  38 

    0x27ae9e7e,// 42 PAY  39 

    0x65454d4c,// 43 PAY  40 

    0x46849e18,// 44 PAY  41 

    0xa4619881,// 45 PAY  42 

    0xb771b759,// 46 PAY  43 

    0x16668b94,// 47 PAY  44 

    0x79421149,// 48 PAY  45 

    0x4af2f6dd,// 49 PAY  46 

    0xed6dc68d,// 50 PAY  47 

    0xdaad4a8e,// 51 PAY  48 

    0xa7155030,// 52 PAY  49 

    0x1331bd54,// 53 PAY  50 

    0x31392f80,// 54 PAY  51 

    0xb2e19aac,// 55 PAY  52 

    0xe18deef5,// 56 PAY  53 

    0x5d9475d1,// 57 PAY  54 

    0xe7b5c7d0,// 58 PAY  55 

    0xd7a0f0c5,// 59 PAY  56 

    0x67278727,// 60 PAY  57 

    0xaebde2f2,// 61 PAY  58 

    0x24ee943e,// 62 PAY  59 

    0xe8b04842,// 63 PAY  60 

    0x6518525e,// 64 PAY  61 

    0xdd75bb16,// 65 PAY  62 

    0x22b050fb,// 66 PAY  63 

    0x4563c637,// 67 PAY  64 

    0x0eaa38db,// 68 PAY  65 

    0x7c2fc9a5,// 69 PAY  66 

    0x479248b2,// 70 PAY  67 

    0x44ebd4fb,// 71 PAY  68 

    0x9f183e45,// 72 PAY  69 

    0xbb07d7bb,// 73 PAY  70 

    0x457979b7,// 74 PAY  71 

    0x5839fd17,// 75 PAY  72 

    0x9a732749,// 76 PAY  73 

    0x8d22a911,// 77 PAY  74 

    0xafc31664,// 78 PAY  75 

    0xf5fe3f73,// 79 PAY  76 

    0x57e44ec2,// 80 PAY  77 

    0x27d28d87,// 81 PAY  78 

    0x9d98f73d,// 82 PAY  79 

    0x029a425c,// 83 PAY  80 

    0x04eaff92,// 84 PAY  81 

    0x80271b41,// 85 PAY  82 

    0xcfa26788,// 86 PAY  83 

    0x380466c2,// 87 PAY  84 

    0x2e263846,// 88 PAY  85 

    0x16b4abec,// 89 PAY  86 

    0xdd5af7b0,// 90 PAY  87 

    0x3e9e7bf6,// 91 PAY  88 

    0xba409470,// 92 PAY  89 

    0x92eafa63,// 93 PAY  90 

    0xd79767e6,// 94 PAY  91 

    0x57bff995,// 95 PAY  92 

    0x415bad44,// 96 PAY  93 

    0x93099960,// 97 PAY  94 

    0xc8e62af1,// 98 PAY  95 

    0xfb2ed1cb,// 99 PAY  96 

    0xc33e525a,// 100 PAY  97 

    0x2109f6b6,// 101 PAY  98 

    0x290f4808,// 102 PAY  99 

    0x6218adcc,// 103 PAY 100 

    0x27e0b7a7,// 104 PAY 101 

    0xedee4215,// 105 PAY 102 

    0xb7de7378,// 106 PAY 103 

    0xba9c5c98,// 107 PAY 104 

    0x9dcb2f10,// 108 PAY 105 

    0x4c5d7fa1,// 109 PAY 106 

    0x32e5c0ba,// 110 PAY 107 

    0x0188ce33,// 111 PAY 108 

    0x98ce0783,// 112 PAY 109 

    0xc6344793,// 113 PAY 110 

    0x2cf2482f,// 114 PAY 111 

    0x550b7b47,// 115 PAY 112 

    0x780c7f51,// 116 PAY 113 

    0xe1f4729b,// 117 PAY 114 

    0x2633d187,// 118 PAY 115 

    0xb5d4f57a,// 119 PAY 116 

    0x189d745d,// 120 PAY 117 

    0x27403ded,// 121 PAY 118 

    0x46234feb,// 122 PAY 119 

    0x9fcb4ad9,// 123 PAY 120 

    0x3ae09a15,// 124 PAY 121 

    0x9b02c89c,// 125 PAY 122 

    0xf3f7b9c6,// 126 PAY 123 

    0xfef43b61,// 127 PAY 124 

    0x3f3f5b77,// 128 PAY 125 

    0x2a549129,// 129 PAY 126 

    0x78751911,// 130 PAY 127 

    0xdf8459c4,// 131 PAY 128 

    0x42d45e47,// 132 PAY 129 

    0xd6490753,// 133 PAY 130 

    0x84a81923,// 134 PAY 131 

    0x4baaf836,// 135 PAY 132 

    0x13f292f3,// 136 PAY 133 

    0x451e7f79,// 137 PAY 134 

    0xd2099929,// 138 PAY 135 

    0x350e9266,// 139 PAY 136 

    0x537e1bc3,// 140 PAY 137 

    0x32378e1d,// 141 PAY 138 

    0x01dac972,// 142 PAY 139 

    0xce7e449d,// 143 PAY 140 

    0x4d3dee7d,// 144 PAY 141 

    0x3023673e,// 145 PAY 142 

    0x5e6674b0,// 146 PAY 143 

    0x1187d712,// 147 PAY 144 

    0x8cf07e65,// 148 PAY 145 

    0x2ee64a83,// 149 PAY 146 

    0xa48ed5fa,// 150 PAY 147 

    0x584cf17c,// 151 PAY 148 

    0x3e5a436f,// 152 PAY 149 

    0x03beadd2,// 153 PAY 150 

    0x4fb74703,// 154 PAY 151 

    0xcc089168,// 155 PAY 152 

    0x4bedf1bb,// 156 PAY 153 

    0x333a7eac,// 157 PAY 154 

    0x0f63ff0e,// 158 PAY 155 

    0x0abe9eec,// 159 PAY 156 

    0x80af7b8d,// 160 PAY 157 

    0x9555257e,// 161 PAY 158 

    0x24cc89af,// 162 PAY 159 

    0xb42c102d,// 163 PAY 160 

    0xde2a4114,// 164 PAY 161 

    0xe3324b57,// 165 PAY 162 

    0x66242fb7,// 166 PAY 163 

    0xc0506af2,// 167 PAY 164 

    0x8db95b4f,// 168 PAY 165 

    0x0c3b0b81,// 169 PAY 166 

    0x718838d0,// 170 PAY 167 

    0xd8df5735,// 171 PAY 168 

    0x60b6cb42,// 172 PAY 169 

    0x2060a0e8,// 173 PAY 170 

    0xbcc28a99,// 174 PAY 171 

    0xf6ad33d9,// 175 PAY 172 

    0x97d9c481,// 176 PAY 173 

    0x042c6698,// 177 PAY 174 

    0xe8509602,// 178 PAY 175 

    0x06079396,// 179 PAY 176 

    0xa620e790,// 180 PAY 177 

    0xde9b5637,// 181 PAY 178 

    0xb6325983,// 182 PAY 179 

    0x082c1eb6,// 183 PAY 180 

    0x2e30e5f1,// 184 PAY 181 

    0xc179f7af,// 185 PAY 182 

    0x0a39ae18,// 186 PAY 183 

    0x1fb87e59,// 187 PAY 184 

    0xd28387f1,// 188 PAY 185 

    0xce621a74,// 189 PAY 186 

    0xcd16355f,// 190 PAY 187 

    0x9cf5d1be,// 191 PAY 188 

    0x7b8f3320,// 192 PAY 189 

    0xfe8ec272,// 193 PAY 190 

    0x45d9d8aa,// 194 PAY 191 

    0x0ebaa0d0,// 195 PAY 192 

    0xd195ee1f,// 196 PAY 193 

    0xa3bc7a8a,// 197 PAY 194 

    0xa1db15df,// 198 PAY 195 

    0x5880fac9,// 199 PAY 196 

    0x94dc6ea9,// 200 PAY 197 

    0x038f8cee,// 201 PAY 198 

    0xab39071a,// 202 PAY 199 

    0xa154c690,// 203 PAY 200 

    0x75f366f2,// 204 PAY 201 

    0xe3c84ef5,// 205 PAY 202 

    0x1fa50d71,// 206 PAY 203 

    0x8378cfbe,// 207 PAY 204 

    0xc60bf5a3,// 208 PAY 205 

    0x5b0c17fa,// 209 PAY 206 

    0x8187541b,// 210 PAY 207 

    0x5b9345e8,// 211 PAY 208 

    0x8be981c7,// 212 PAY 209 

    0x7d1a7134,// 213 PAY 210 

    0x4bb69cd4,// 214 PAY 211 

    0x5da43805,// 215 PAY 212 

    0x038b052e,// 216 PAY 213 

    0x8f84cf4e,// 217 PAY 214 

    0x0f5307f4,// 218 PAY 215 

    0x14e0059d,// 219 PAY 216 

    0x279b8cb0,// 220 PAY 217 

    0xcaa062b2,// 221 PAY 218 

    0x0079e80d,// 222 PAY 219 

    0x90a676c1,// 223 PAY 220 

    0x29c2bcee,// 224 PAY 221 

    0xe3bb94b7,// 225 PAY 222 

    0x028755d0,// 226 PAY 223 

    0x970faebc,// 227 PAY 224 

    0xd8ce8418,// 228 PAY 225 

    0xd6dd8625,// 229 PAY 226 

    0xef5a31c5,// 230 PAY 227 

    0xcf17ed95,// 231 PAY 228 

    0x1a08704d,// 232 PAY 229 

    0xa5819576,// 233 PAY 230 

    0xc1a65565,// 234 PAY 231 

    0xd2e5d32d,// 235 PAY 232 

    0x9f6b808e,// 236 PAY 233 

    0x8dfaeb8e,// 237 PAY 234 

    0x28b5a601,// 238 PAY 235 

    0x1388cbef,// 239 PAY 236 

    0x1ed77fbf,// 240 PAY 237 

    0xbcdf0dae,// 241 PAY 238 

    0x2ba105d2,// 242 PAY 239 

    0xbb13183d,// 243 PAY 240 

    0x90400422,// 244 PAY 241 

    0x9b59667c,// 245 PAY 242 

    0x01789759,// 246 PAY 243 

    0x0318bd1d,// 247 PAY 244 

    0x1ce7fcec,// 248 PAY 245 

    0x4db19c9a,// 249 PAY 246 

    0xc63c42dd,// 250 PAY 247 

    0xca507922,// 251 PAY 248 

    0x2d1244a9,// 252 PAY 249 

    0xb47056be,// 253 PAY 250 

    0x7ccad4ad,// 254 PAY 251 

    0xa77f1edb,// 255 PAY 252 

    0xb7ece2ab,// 256 PAY 253 

    0x521dd754,// 257 PAY 254 

    0x61f40b6f,// 258 PAY 255 

    0x8d96cf0c,// 259 PAY 256 

    0x52d6b1b4,// 260 PAY 257 

    0x2baaac00,// 261 PAY 258 

/// HASH is  16 bytes 

    0x14e0059d,// 262 HSH   1 

    0x279b8cb0,// 263 HSH   2 

    0xcaa062b2,// 264 HSH   3 

    0x0079e80d,// 265 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 229 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4f 

    0x01314fe5 // 266 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt39_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 357 words. 

/// BDA size     is 1422 (0x58e) 

/// BDA id       is 0xc25c 

    0x058ec25c,// 3 BDA   1 

/// PAY Generic Data size   : 1422 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xbe952cd6,// 4 PAY   1 

    0x36578f99,// 5 PAY   2 

    0x5aa004e4,// 6 PAY   3 

    0x2d9e616e,// 7 PAY   4 

    0x8c451afe,// 8 PAY   5 

    0xba4a2b44,// 9 PAY   6 

    0xbf29008e,// 10 PAY   7 

    0x31345e39,// 11 PAY   8 

    0x63413b54,// 12 PAY   9 

    0xf0907aab,// 13 PAY  10 

    0x07bb3afc,// 14 PAY  11 

    0xed4da642,// 15 PAY  12 

    0x8b8d71c7,// 16 PAY  13 

    0x467cd067,// 17 PAY  14 

    0x0813e7fc,// 18 PAY  15 

    0xaa95d313,// 19 PAY  16 

    0xab632715,// 20 PAY  17 

    0xf1ed6ec3,// 21 PAY  18 

    0xb1d6b8f9,// 22 PAY  19 

    0xc9683171,// 23 PAY  20 

    0xb91ece3a,// 24 PAY  21 

    0xa5913669,// 25 PAY  22 

    0x41647c55,// 26 PAY  23 

    0xe7a11595,// 27 PAY  24 

    0xceb8ebd1,// 28 PAY  25 

    0xe9db8d59,// 29 PAY  26 

    0x49a5cd72,// 30 PAY  27 

    0x3b2094ef,// 31 PAY  28 

    0xf2abdf65,// 32 PAY  29 

    0xc04c8c59,// 33 PAY  30 

    0x5e6b90f9,// 34 PAY  31 

    0xedab8c0b,// 35 PAY  32 

    0xf304413c,// 36 PAY  33 

    0xb3ea35cf,// 37 PAY  34 

    0x4f59ed5a,// 38 PAY  35 

    0x9a0b754e,// 39 PAY  36 

    0x3a5fe650,// 40 PAY  37 

    0xa8309d17,// 41 PAY  38 

    0x7b23abc2,// 42 PAY  39 

    0x5c3456fe,// 43 PAY  40 

    0x445a1dec,// 44 PAY  41 

    0xf49b3946,// 45 PAY  42 

    0xf7db3bf3,// 46 PAY  43 

    0x216f6dfc,// 47 PAY  44 

    0xa6a22837,// 48 PAY  45 

    0xeb7a0c87,// 49 PAY  46 

    0xbbba57b7,// 50 PAY  47 

    0xef432e9d,// 51 PAY  48 

    0x2fbd2842,// 52 PAY  49 

    0x571dee0e,// 53 PAY  50 

    0x3fa8a6d0,// 54 PAY  51 

    0x56e47f2b,// 55 PAY  52 

    0x4d6f53f4,// 56 PAY  53 

    0xf9a6007c,// 57 PAY  54 

    0x9a71b0f0,// 58 PAY  55 

    0x5f5c9d98,// 59 PAY  56 

    0x6df09d96,// 60 PAY  57 

    0x12070a84,// 61 PAY  58 

    0x6d9e033d,// 62 PAY  59 

    0x3189d2d6,// 63 PAY  60 

    0x9113c646,// 64 PAY  61 

    0xe96321cd,// 65 PAY  62 

    0x77c506ea,// 66 PAY  63 

    0xce73bbc2,// 67 PAY  64 

    0x3db1a82d,// 68 PAY  65 

    0x19c117d2,// 69 PAY  66 

    0xfbd9cf7d,// 70 PAY  67 

    0x002c605e,// 71 PAY  68 

    0xf12e928f,// 72 PAY  69 

    0x724ecbd9,// 73 PAY  70 

    0xcc35c398,// 74 PAY  71 

    0xf028ed41,// 75 PAY  72 

    0x6b4b007b,// 76 PAY  73 

    0x657cd24e,// 77 PAY  74 

    0xb42f5542,// 78 PAY  75 

    0xfd467ff3,// 79 PAY  76 

    0x54fb0b6f,// 80 PAY  77 

    0x67871b50,// 81 PAY  78 

    0xfd03caf4,// 82 PAY  79 

    0xc545fb21,// 83 PAY  80 

    0xf5ba41b8,// 84 PAY  81 

    0x9126625c,// 85 PAY  82 

    0x43b5987d,// 86 PAY  83 

    0xb326d9bd,// 87 PAY  84 

    0x5d831dbe,// 88 PAY  85 

    0x47e47596,// 89 PAY  86 

    0x51b6a113,// 90 PAY  87 

    0xd56bcbb8,// 91 PAY  88 

    0x3c5db9c4,// 92 PAY  89 

    0xba4013bd,// 93 PAY  90 

    0xf91fc504,// 94 PAY  91 

    0xe975aea7,// 95 PAY  92 

    0x15f34258,// 96 PAY  93 

    0xfb5c59b4,// 97 PAY  94 

    0xa94ef4b5,// 98 PAY  95 

    0x7808dbb9,// 99 PAY  96 

    0xf0954f7f,// 100 PAY  97 

    0x0b1982a5,// 101 PAY  98 

    0x1e715b86,// 102 PAY  99 

    0x3b5a912a,// 103 PAY 100 

    0x36e648b8,// 104 PAY 101 

    0xf3b5aed7,// 105 PAY 102 

    0x12b21388,// 106 PAY 103 

    0x97dbada6,// 107 PAY 104 

    0x704e48c1,// 108 PAY 105 

    0x80bca4e4,// 109 PAY 106 

    0x2515e63b,// 110 PAY 107 

    0x35331d02,// 111 PAY 108 

    0xddc18d72,// 112 PAY 109 

    0x421405ae,// 113 PAY 110 

    0xeae95f10,// 114 PAY 111 

    0x6a572a1d,// 115 PAY 112 

    0xb52d4299,// 116 PAY 113 

    0x85c3e0a1,// 117 PAY 114 

    0xd7289f9f,// 118 PAY 115 

    0xf2ba5f2e,// 119 PAY 116 

    0x6b884540,// 120 PAY 117 

    0x2fa06065,// 121 PAY 118 

    0x39fdf16d,// 122 PAY 119 

    0x371090f6,// 123 PAY 120 

    0x2ca997ef,// 124 PAY 121 

    0xbb6f811a,// 125 PAY 122 

    0xc35c89cb,// 126 PAY 123 

    0xcff5c7f8,// 127 PAY 124 

    0x551b6e01,// 128 PAY 125 

    0xa54f62a4,// 129 PAY 126 

    0xdf2dc3a4,// 130 PAY 127 

    0xec7f8912,// 131 PAY 128 

    0x8321e42f,// 132 PAY 129 

    0x54cd4dfb,// 133 PAY 130 

    0xa3329e31,// 134 PAY 131 

    0xba9861d2,// 135 PAY 132 

    0xc08dd39a,// 136 PAY 133 

    0x8633ffc1,// 137 PAY 134 

    0x5ce40775,// 138 PAY 135 

    0x1763c9ce,// 139 PAY 136 

    0xefcd67c1,// 140 PAY 137 

    0xb030f967,// 141 PAY 138 

    0x1bb72190,// 142 PAY 139 

    0x461d18d4,// 143 PAY 140 

    0x8b59e0c9,// 144 PAY 141 

    0x69a117c7,// 145 PAY 142 

    0x3a925b1a,// 146 PAY 143 

    0x69cf47bf,// 147 PAY 144 

    0xfd78a632,// 148 PAY 145 

    0x0646a990,// 149 PAY 146 

    0x227be8b9,// 150 PAY 147 

    0x85c8f7e8,// 151 PAY 148 

    0x83fd4b63,// 152 PAY 149 

    0x946c943e,// 153 PAY 150 

    0x375400f4,// 154 PAY 151 

    0x7218ca51,// 155 PAY 152 

    0x0daf7f68,// 156 PAY 153 

    0x42311bb8,// 157 PAY 154 

    0xe0fb23a3,// 158 PAY 155 

    0x3f8bb952,// 159 PAY 156 

    0x522ac678,// 160 PAY 157 

    0x286c9c0c,// 161 PAY 158 

    0x30da94c3,// 162 PAY 159 

    0x71708bbe,// 163 PAY 160 

    0xe2106c4b,// 164 PAY 161 

    0x54932165,// 165 PAY 162 

    0xcc800727,// 166 PAY 163 

    0x23917994,// 167 PAY 164 

    0x1fe5bfc5,// 168 PAY 165 

    0x4d0cfe7c,// 169 PAY 166 

    0x3b82ccba,// 170 PAY 167 

    0x01751d84,// 171 PAY 168 

    0x5f98608a,// 172 PAY 169 

    0xae209e55,// 173 PAY 170 

    0x2c50f25f,// 174 PAY 171 

    0x9fa00589,// 175 PAY 172 

    0xcb7401a9,// 176 PAY 173 

    0x259a3941,// 177 PAY 174 

    0x84ab5c28,// 178 PAY 175 

    0x0cfe8364,// 179 PAY 176 

    0x7c26fd82,// 180 PAY 177 

    0xb51a9762,// 181 PAY 178 

    0x6e9fafa2,// 182 PAY 179 

    0x05824db3,// 183 PAY 180 

    0x6377ae4d,// 184 PAY 181 

    0xc38c2684,// 185 PAY 182 

    0x1db16de1,// 186 PAY 183 

    0xf3d655e6,// 187 PAY 184 

    0x9224a0a5,// 188 PAY 185 

    0x6a69c819,// 189 PAY 186 

    0xb6616399,// 190 PAY 187 

    0xb430d010,// 191 PAY 188 

    0xeedaa74a,// 192 PAY 189 

    0x8378dbe2,// 193 PAY 190 

    0xa5077bd2,// 194 PAY 191 

    0x1d4b2ad8,// 195 PAY 192 

    0x1e38d0bc,// 196 PAY 193 

    0x12a989b9,// 197 PAY 194 

    0xa868c363,// 198 PAY 195 

    0x5bb6105b,// 199 PAY 196 

    0x7dd45913,// 200 PAY 197 

    0x4beba6fb,// 201 PAY 198 

    0xef10be67,// 202 PAY 199 

    0x92b7ce45,// 203 PAY 200 

    0x7b9c67b4,// 204 PAY 201 

    0xeb98ead9,// 205 PAY 202 

    0x3ad4980e,// 206 PAY 203 

    0xd7aa6337,// 207 PAY 204 

    0x8af3e324,// 208 PAY 205 

    0x6da7e860,// 209 PAY 206 

    0xc3bf9ced,// 210 PAY 207 

    0x8fc0411c,// 211 PAY 208 

    0xb6eea28f,// 212 PAY 209 

    0xf67d24a7,// 213 PAY 210 

    0x49f9757e,// 214 PAY 211 

    0x1ab47245,// 215 PAY 212 

    0x2f26a585,// 216 PAY 213 

    0x80144fa2,// 217 PAY 214 

    0xf8fdd6b4,// 218 PAY 215 

    0xb9e9fae9,// 219 PAY 216 

    0xf02e9240,// 220 PAY 217 

    0xebe00565,// 221 PAY 218 

    0x7fb19788,// 222 PAY 219 

    0xad682e1d,// 223 PAY 220 

    0x5e261305,// 224 PAY 221 

    0xb1292492,// 225 PAY 222 

    0x00e2b0c9,// 226 PAY 223 

    0x0a63c857,// 227 PAY 224 

    0x3ea73bea,// 228 PAY 225 

    0x80b8f39e,// 229 PAY 226 

    0x3f0770b8,// 230 PAY 227 

    0x639ebced,// 231 PAY 228 

    0x92b50758,// 232 PAY 229 

    0x6b7b984a,// 233 PAY 230 

    0x84c136dc,// 234 PAY 231 

    0x5b0d4920,// 235 PAY 232 

    0x93998b49,// 236 PAY 233 

    0x6b136c6a,// 237 PAY 234 

    0x36477074,// 238 PAY 235 

    0x14ec1498,// 239 PAY 236 

    0xf614ea09,// 240 PAY 237 

    0xdc3daebd,// 241 PAY 238 

    0xd35e2434,// 242 PAY 239 

    0xf17e3b06,// 243 PAY 240 

    0xfc1dba83,// 244 PAY 241 

    0xea8f8072,// 245 PAY 242 

    0x547452c8,// 246 PAY 243 

    0xeb1eec44,// 247 PAY 244 

    0x336a51de,// 248 PAY 245 

    0x0b6ad669,// 249 PAY 246 

    0xd18f8746,// 250 PAY 247 

    0xc3fcd0fb,// 251 PAY 248 

    0x43da8fe2,// 252 PAY 249 

    0xcb8ceb74,// 253 PAY 250 

    0xc195e234,// 254 PAY 251 

    0x0d854ac7,// 255 PAY 252 

    0x2f3ea1a8,// 256 PAY 253 

    0xd21eb773,// 257 PAY 254 

    0x03ac0a23,// 258 PAY 255 

    0x93c37668,// 259 PAY 256 

    0x44046d46,// 260 PAY 257 

    0x5ff82f37,// 261 PAY 258 

    0xab40b27d,// 262 PAY 259 

    0x6656eda8,// 263 PAY 260 

    0xb298d940,// 264 PAY 261 

    0x8b6a12c6,// 265 PAY 262 

    0xf246282f,// 266 PAY 263 

    0xc62e33a1,// 267 PAY 264 

    0x92778b13,// 268 PAY 265 

    0xebace500,// 269 PAY 266 

    0x85284a88,// 270 PAY 267 

    0xef0ac22b,// 271 PAY 268 

    0xbbafce83,// 272 PAY 269 

    0xb8cc0eb2,// 273 PAY 270 

    0xa9c31a2b,// 274 PAY 271 

    0x1427c469,// 275 PAY 272 

    0x824443f9,// 276 PAY 273 

    0x51344995,// 277 PAY 274 

    0xdb6ee713,// 278 PAY 275 

    0xf84ec248,// 279 PAY 276 

    0x0380948e,// 280 PAY 277 

    0x5775f9de,// 281 PAY 278 

    0x4f3a2dea,// 282 PAY 279 

    0xbb3574b2,// 283 PAY 280 

    0x53c34fdd,// 284 PAY 281 

    0x1fab13e2,// 285 PAY 282 

    0x10b23e32,// 286 PAY 283 

    0x55a8a1a0,// 287 PAY 284 

    0xb5889843,// 288 PAY 285 

    0x94f052ea,// 289 PAY 286 

    0xc0213afd,// 290 PAY 287 

    0x3e2632fa,// 291 PAY 288 

    0xd57328cc,// 292 PAY 289 

    0xc01b7604,// 293 PAY 290 

    0xb53b4c75,// 294 PAY 291 

    0xf5b7b4b5,// 295 PAY 292 

    0xa56f5c1c,// 296 PAY 293 

    0xa95ea995,// 297 PAY 294 

    0x44d2f9d7,// 298 PAY 295 

    0xf2d3b841,// 299 PAY 296 

    0x9a592798,// 300 PAY 297 

    0xa1c40f9c,// 301 PAY 298 

    0x03a58281,// 302 PAY 299 

    0x92841394,// 303 PAY 300 

    0xd3c29ea4,// 304 PAY 301 

    0x15a90c2f,// 305 PAY 302 

    0xbbf62694,// 306 PAY 303 

    0xec9d4a73,// 307 PAY 304 

    0x140ef036,// 308 PAY 305 

    0x3881862f,// 309 PAY 306 

    0x760b03db,// 310 PAY 307 

    0xcd6d3887,// 311 PAY 308 

    0x4249f29d,// 312 PAY 309 

    0x0e950065,// 313 PAY 310 

    0xee4efb8b,// 314 PAY 311 

    0x35957840,// 315 PAY 312 

    0xc152a633,// 316 PAY 313 

    0x92567f45,// 317 PAY 314 

    0x68d00e25,// 318 PAY 315 

    0xb3fd256b,// 319 PAY 316 

    0xf599fb70,// 320 PAY 317 

    0xfa504fe1,// 321 PAY 318 

    0x893820cd,// 322 PAY 319 

    0x9e0e9ef3,// 323 PAY 320 

    0x0fab56af,// 324 PAY 321 

    0x4ede842b,// 325 PAY 322 

    0x200f1fd5,// 326 PAY 323 

    0xc6736654,// 327 PAY 324 

    0x6dcd449d,// 328 PAY 325 

    0xc8fcfa1e,// 329 PAY 326 

    0xceaf9304,// 330 PAY 327 

    0xb60a968a,// 331 PAY 328 

    0xbe46be5c,// 332 PAY 329 

    0x587aaef7,// 333 PAY 330 

    0x3359cf41,// 334 PAY 331 

    0xb2a41ba2,// 335 PAY 332 

    0xb45ea643,// 336 PAY 333 

    0x21a8f5c1,// 337 PAY 334 

    0x5b1a06f0,// 338 PAY 335 

    0x528d15b5,// 339 PAY 336 

    0xb54bcab8,// 340 PAY 337 

    0xcb2cbf9d,// 341 PAY 338 

    0x5cacbed7,// 342 PAY 339 

    0x2e91759c,// 343 PAY 340 

    0x1b997ef5,// 344 PAY 341 

    0xf7213d8f,// 345 PAY 342 

    0x8094de13,// 346 PAY 343 

    0xb0db4a4a,// 347 PAY 344 

    0x8a6c616f,// 348 PAY 345 

    0xd00ed7d3,// 349 PAY 346 

    0x050f12ad,// 350 PAY 347 

    0x7171f225,// 351 PAY 348 

    0x4b3c660f,// 352 PAY 349 

    0x2e026450,// 353 PAY 350 

    0x8b5d9654,// 354 PAY 351 

    0x68db18d9,// 355 PAY 352 

    0x4efff9d7,// 356 PAY 353 

    0x04fe68f7,// 357 PAY 354 

    0x75b442b3,// 358 PAY 355 

    0x08660000,// 359 PAY 356 

/// HASH is  8 bytes 

    0x69a117c7,// 360 HSH   1 

    0x3a925b1a,// 361 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 208 

/// STA pkt_idx        : 48 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5f 

    0x00c05fd0 // 362 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt40_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 200 words. 

/// BDA size     is 796 (0x31c) 

/// BDA id       is 0xd94d 

    0x031cd94d,// 3 BDA   1 

/// PAY Generic Data size   : 796 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x4f300e70,// 4 PAY   1 

    0x0b641801,// 5 PAY   2 

    0x185e4aee,// 6 PAY   3 

    0xe6d89cc8,// 7 PAY   4 

    0xa74d67cc,// 8 PAY   5 

    0x9fe2f1c9,// 9 PAY   6 

    0xe2f18e16,// 10 PAY   7 

    0x14a6c935,// 11 PAY   8 

    0xff60f44d,// 12 PAY   9 

    0x32dc9e3c,// 13 PAY  10 

    0xccea2558,// 14 PAY  11 

    0x3f80abc1,// 15 PAY  12 

    0xb887dbe5,// 16 PAY  13 

    0xac689cce,// 17 PAY  14 

    0xea63be28,// 18 PAY  15 

    0xc944481e,// 19 PAY  16 

    0x5b708629,// 20 PAY  17 

    0x0fdced9a,// 21 PAY  18 

    0xa7b24777,// 22 PAY  19 

    0xf35b72d7,// 23 PAY  20 

    0xf9fab108,// 24 PAY  21 

    0xe54ad67c,// 25 PAY  22 

    0x8eede568,// 26 PAY  23 

    0x87936013,// 27 PAY  24 

    0x636bf4cd,// 28 PAY  25 

    0x1fdf429a,// 29 PAY  26 

    0x18cd6bf0,// 30 PAY  27 

    0x837d3dcf,// 31 PAY  28 

    0x269b53df,// 32 PAY  29 

    0x519cbcc8,// 33 PAY  30 

    0x1d75ae91,// 34 PAY  31 

    0x1d59113a,// 35 PAY  32 

    0x9d10a68a,// 36 PAY  33 

    0xc6f8090b,// 37 PAY  34 

    0x77ba9787,// 38 PAY  35 

    0x249662e1,// 39 PAY  36 

    0xd056d907,// 40 PAY  37 

    0x33ebcc02,// 41 PAY  38 

    0x1f125093,// 42 PAY  39 

    0x9a3c2555,// 43 PAY  40 

    0x3010cf8d,// 44 PAY  41 

    0x880bee83,// 45 PAY  42 

    0x64d7fe8c,// 46 PAY  43 

    0xda57bd29,// 47 PAY  44 

    0x03399c68,// 48 PAY  45 

    0x616a361e,// 49 PAY  46 

    0x5274d41c,// 50 PAY  47 

    0xd8c648ff,// 51 PAY  48 

    0x970ecf15,// 52 PAY  49 

    0x03c3e089,// 53 PAY  50 

    0x09974a18,// 54 PAY  51 

    0xe57b6738,// 55 PAY  52 

    0xb6e9329e,// 56 PAY  53 

    0xb5ae25a8,// 57 PAY  54 

    0x57920b58,// 58 PAY  55 

    0x77eca008,// 59 PAY  56 

    0x034fd45e,// 60 PAY  57 

    0xe0dae201,// 61 PAY  58 

    0xf09b598a,// 62 PAY  59 

    0x208e2431,// 63 PAY  60 

    0x012f8dcc,// 64 PAY  61 

    0x419c18be,// 65 PAY  62 

    0x0ad11b4f,// 66 PAY  63 

    0xbcf60d2b,// 67 PAY  64 

    0x2ad5d77d,// 68 PAY  65 

    0x8636600d,// 69 PAY  66 

    0xdf820c1c,// 70 PAY  67 

    0x2a4a5a2e,// 71 PAY  68 

    0x2e0c7eb4,// 72 PAY  69 

    0x5d9a7ab5,// 73 PAY  70 

    0x3075e168,// 74 PAY  71 

    0xa877d249,// 75 PAY  72 

    0x819a62c4,// 76 PAY  73 

    0xa3ae7264,// 77 PAY  74 

    0x73d0e487,// 78 PAY  75 

    0xc41ce11a,// 79 PAY  76 

    0x600b7efd,// 80 PAY  77 

    0xf83f693d,// 81 PAY  78 

    0x6cd45fee,// 82 PAY  79 

    0x4596ed1f,// 83 PAY  80 

    0xf2550c1e,// 84 PAY  81 

    0xf9108178,// 85 PAY  82 

    0x6193b7c4,// 86 PAY  83 

    0x739a5681,// 87 PAY  84 

    0xea406fca,// 88 PAY  85 

    0x3bda6064,// 89 PAY  86 

    0x42f4bd82,// 90 PAY  87 

    0xc5c169d3,// 91 PAY  88 

    0x99a42435,// 92 PAY  89 

    0xd2fe2ae6,// 93 PAY  90 

    0x584487fe,// 94 PAY  91 

    0xc28b8e8f,// 95 PAY  92 

    0x84ec58c1,// 96 PAY  93 

    0xae6aad5b,// 97 PAY  94 

    0x75701470,// 98 PAY  95 

    0xf8f8fc62,// 99 PAY  96 

    0xaefdf8f8,// 100 PAY  97 

    0x8932ed17,// 101 PAY  98 

    0xfbb66217,// 102 PAY  99 

    0x0f202eeb,// 103 PAY 100 

    0x63a80f7c,// 104 PAY 101 

    0xcb85e53d,// 105 PAY 102 

    0x7678bb0d,// 106 PAY 103 

    0x7fe475fb,// 107 PAY 104 

    0x2de9baa8,// 108 PAY 105 

    0x4cf98e2a,// 109 PAY 106 

    0x0e482aeb,// 110 PAY 107 

    0xd23e3d47,// 111 PAY 108 

    0x6af6ddd0,// 112 PAY 109 

    0xc8c36e56,// 113 PAY 110 

    0x891f9557,// 114 PAY 111 

    0x5c721a39,// 115 PAY 112 

    0x4e4823a4,// 116 PAY 113 

    0xdb1fd5b3,// 117 PAY 114 

    0x956c5378,// 118 PAY 115 

    0x161413f1,// 119 PAY 116 

    0x64863172,// 120 PAY 117 

    0x35a0a58a,// 121 PAY 118 

    0xaa399a9f,// 122 PAY 119 

    0xd5eb88d3,// 123 PAY 120 

    0xeacebe47,// 124 PAY 121 

    0x5f0227b3,// 125 PAY 122 

    0xc6de821e,// 126 PAY 123 

    0x51a928e7,// 127 PAY 124 

    0xb2011a30,// 128 PAY 125 

    0x43357ebf,// 129 PAY 126 

    0x4c1fd349,// 130 PAY 127 

    0x9f86a36e,// 131 PAY 128 

    0x08c696f3,// 132 PAY 129 

    0x0dfca078,// 133 PAY 130 

    0x1fe9d97c,// 134 PAY 131 

    0xafb9b852,// 135 PAY 132 

    0xea5654b5,// 136 PAY 133 

    0x2cd75c52,// 137 PAY 134 

    0x26478098,// 138 PAY 135 

    0x9cef0be5,// 139 PAY 136 

    0xc9fb8332,// 140 PAY 137 

    0x218a6575,// 141 PAY 138 

    0x1170052e,// 142 PAY 139 

    0xc9b5d832,// 143 PAY 140 

    0x21f0cab1,// 144 PAY 141 

    0x3d4974b4,// 145 PAY 142 

    0xc2462fea,// 146 PAY 143 

    0x38adbfaa,// 147 PAY 144 

    0xb0a14852,// 148 PAY 145 

    0x517f79e9,// 149 PAY 146 

    0xc17560a5,// 150 PAY 147 

    0x276903c7,// 151 PAY 148 

    0x015fe8f9,// 152 PAY 149 

    0x72c97a76,// 153 PAY 150 

    0x9451ab85,// 154 PAY 151 

    0xc362828c,// 155 PAY 152 

    0x548149e9,// 156 PAY 153 

    0x08e25c6b,// 157 PAY 154 

    0xe75308f9,// 158 PAY 155 

    0xdb1a776d,// 159 PAY 156 

    0xf91d9b82,// 160 PAY 157 

    0xf6442a51,// 161 PAY 158 

    0x273ac2eb,// 162 PAY 159 

    0x522d4d13,// 163 PAY 160 

    0xb3741da0,// 164 PAY 161 

    0x2d85a7eb,// 165 PAY 162 

    0x9fc31816,// 166 PAY 163 

    0x4c29d09a,// 167 PAY 164 

    0xb380d867,// 168 PAY 165 

    0x0dc729b2,// 169 PAY 166 

    0x142cc571,// 170 PAY 167 

    0x04e452f6,// 171 PAY 168 

    0xfafdc458,// 172 PAY 169 

    0x7b883444,// 173 PAY 170 

    0xda409e8a,// 174 PAY 171 

    0x18896d17,// 175 PAY 172 

    0x2349c893,// 176 PAY 173 

    0x71c64af7,// 177 PAY 174 

    0x40d9a09d,// 178 PAY 175 

    0x7067f55b,// 179 PAY 176 

    0x827ea673,// 180 PAY 177 

    0x96d303b8,// 181 PAY 178 

    0xf68958ac,// 182 PAY 179 

    0x7447411e,// 183 PAY 180 

    0x8840c891,// 184 PAY 181 

    0xa6386835,// 185 PAY 182 

    0x2ffcb3fd,// 186 PAY 183 

    0x54120f4b,// 187 PAY 184 

    0xe509fc2d,// 188 PAY 185 

    0x704d5ff8,// 189 PAY 186 

    0x4f1d7353,// 190 PAY 187 

    0x5d3c9d92,// 191 PAY 188 

    0xca4375cf,// 192 PAY 189 

    0x9c9b6c67,// 193 PAY 190 

    0x27275e1d,// 194 PAY 191 

    0x5a061552,// 195 PAY 192 

    0x3cd9b318,// 196 PAY 193 

    0x916f1a2e,// 197 PAY 194 

    0xff32b143,// 198 PAY 195 

    0x005e3389,// 199 PAY 196 

    0x140258f8,// 200 PAY 197 

    0x561bd04a,// 201 PAY 198 

    0x5c5e5b4e,// 202 PAY 199 

/// HASH is  16 bytes 

    0x956c5378,// 203 HSH   1 

    0x161413f1,// 204 HSH   2 

    0x64863172,// 205 HSH   3 

    0x35a0a58a,// 206 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 120 

/// STA pkt_idx        : 243 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdb 

    0x03ccdb78 // 207 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt41_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 429 words. 

/// BDA size     is 1710 (0x6ae) 

/// BDA id       is 0x4f88 

    0x06ae4f88,// 3 BDA   1 

/// PAY Generic Data size   : 1710 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x144cb381,// 4 PAY   1 

    0xa3dadb4f,// 5 PAY   2 

    0xc1e9392d,// 6 PAY   3 

    0x33672dd5,// 7 PAY   4 

    0x2ee5f748,// 8 PAY   5 

    0xaf2dd90b,// 9 PAY   6 

    0x2ec838bf,// 10 PAY   7 

    0xf69317ac,// 11 PAY   8 

    0xcc2ef95b,// 12 PAY   9 

    0xf5093b24,// 13 PAY  10 

    0x7a251628,// 14 PAY  11 

    0xf6a36d3d,// 15 PAY  12 

    0xd74b5231,// 16 PAY  13 

    0x95735e52,// 17 PAY  14 

    0xe979ff89,// 18 PAY  15 

    0xade6b5e7,// 19 PAY  16 

    0x322f0bea,// 20 PAY  17 

    0xbe336594,// 21 PAY  18 

    0x2361bed9,// 22 PAY  19 

    0xd74a0990,// 23 PAY  20 

    0xe0770ac1,// 24 PAY  21 

    0x70738f34,// 25 PAY  22 

    0x425192ea,// 26 PAY  23 

    0x17db5706,// 27 PAY  24 

    0x26e7e18f,// 28 PAY  25 

    0x134997e9,// 29 PAY  26 

    0xbdb54ed8,// 30 PAY  27 

    0x3ebe9877,// 31 PAY  28 

    0xb53bc039,// 32 PAY  29 

    0x8069dac4,// 33 PAY  30 

    0xffda7005,// 34 PAY  31 

    0x2735ed88,// 35 PAY  32 

    0x514cdb5f,// 36 PAY  33 

    0x0dd22003,// 37 PAY  34 

    0x0dea525b,// 38 PAY  35 

    0x2ab3b61f,// 39 PAY  36 

    0x25875392,// 40 PAY  37 

    0x9c56482f,// 41 PAY  38 

    0x9898195e,// 42 PAY  39 

    0xf3b2efd6,// 43 PAY  40 

    0xb432cf02,// 44 PAY  41 

    0x72e31205,// 45 PAY  42 

    0x0d337eb2,// 46 PAY  43 

    0xf17f8fa5,// 47 PAY  44 

    0x807d5662,// 48 PAY  45 

    0xfe9cd260,// 49 PAY  46 

    0xed242ff5,// 50 PAY  47 

    0xc54c34bc,// 51 PAY  48 

    0xb9b37fbf,// 52 PAY  49 

    0x654a86c3,// 53 PAY  50 

    0x9f2b50e0,// 54 PAY  51 

    0xd09f59b9,// 55 PAY  52 

    0x1cf0bb98,// 56 PAY  53 

    0xdbca10c7,// 57 PAY  54 

    0x8bfb4f5f,// 58 PAY  55 

    0x5ad71a63,// 59 PAY  56 

    0x0a419b43,// 60 PAY  57 

    0x65840b11,// 61 PAY  58 

    0xbaf089c0,// 62 PAY  59 

    0x567bc67f,// 63 PAY  60 

    0xf12d4dea,// 64 PAY  61 

    0xc5323125,// 65 PAY  62 

    0xf449ca80,// 66 PAY  63 

    0x2ed28993,// 67 PAY  64 

    0x70ad7da6,// 68 PAY  65 

    0x2f5ad2c3,// 69 PAY  66 

    0x2578ca9f,// 70 PAY  67 

    0x298d620b,// 71 PAY  68 

    0x34820ac6,// 72 PAY  69 

    0xfffd8f49,// 73 PAY  70 

    0xac0fe758,// 74 PAY  71 

    0xd6351014,// 75 PAY  72 

    0x3b954ca9,// 76 PAY  73 

    0xbac14800,// 77 PAY  74 

    0x0f5fffd5,// 78 PAY  75 

    0xdaae8709,// 79 PAY  76 

    0xc6ce6eb2,// 80 PAY  77 

    0x17262e1b,// 81 PAY  78 

    0x1cd02556,// 82 PAY  79 

    0x22b3d533,// 83 PAY  80 

    0x7d63bc6b,// 84 PAY  81 

    0x50c2afd4,// 85 PAY  82 

    0x19e62962,// 86 PAY  83 

    0x63d0b983,// 87 PAY  84 

    0xa236098d,// 88 PAY  85 

    0x6ffb117e,// 89 PAY  86 

    0x7290255a,// 90 PAY  87 

    0xa2ee12c9,// 91 PAY  88 

    0x33c61736,// 92 PAY  89 

    0x9bb1907c,// 93 PAY  90 

    0x2703af2c,// 94 PAY  91 

    0x303c7a05,// 95 PAY  92 

    0x46342665,// 96 PAY  93 

    0x27ce2e9d,// 97 PAY  94 

    0xf8c032be,// 98 PAY  95 

    0x48e16ab8,// 99 PAY  96 

    0x1d72e57b,// 100 PAY  97 

    0xc43e6c9d,// 101 PAY  98 

    0xc2303d51,// 102 PAY  99 

    0x76662b86,// 103 PAY 100 

    0xf9c9d700,// 104 PAY 101 

    0x3c82aec7,// 105 PAY 102 

    0xeb5a7d10,// 106 PAY 103 

    0xc7fcd8a4,// 107 PAY 104 

    0x1b8fb02e,// 108 PAY 105 

    0x4a128729,// 109 PAY 106 

    0xe90ad1bb,// 110 PAY 107 

    0xcc038d12,// 111 PAY 108 

    0xcb2d44c8,// 112 PAY 109 

    0x64481ceb,// 113 PAY 110 

    0x5188b46a,// 114 PAY 111 

    0xf00bdac4,// 115 PAY 112 

    0xd154fe88,// 116 PAY 113 

    0x07b15652,// 117 PAY 114 

    0xd5cb5465,// 118 PAY 115 

    0x37475502,// 119 PAY 116 

    0x86d5c3f6,// 120 PAY 117 

    0x584bdfc7,// 121 PAY 118 

    0x356b7bf4,// 122 PAY 119 

    0x9db22ce2,// 123 PAY 120 

    0x7eb533d9,// 124 PAY 121 

    0x8ce09d93,// 125 PAY 122 

    0x7e9435c7,// 126 PAY 123 

    0xe93a5ee9,// 127 PAY 124 

    0xc649a9fd,// 128 PAY 125 

    0xe6c9f631,// 129 PAY 126 

    0xa9f7159b,// 130 PAY 127 

    0xfc6e1f87,// 131 PAY 128 

    0x6e22a8c2,// 132 PAY 129 

    0x85cc55c5,// 133 PAY 130 

    0x18850e96,// 134 PAY 131 

    0x96c5ee26,// 135 PAY 132 

    0x1ee8a3ab,// 136 PAY 133 

    0x7e0ea81a,// 137 PAY 134 

    0x6f5b234c,// 138 PAY 135 

    0x36dc078f,// 139 PAY 136 

    0x8b8d4c43,// 140 PAY 137 

    0xf3d06aa8,// 141 PAY 138 

    0x51517ba9,// 142 PAY 139 

    0x5c68a111,// 143 PAY 140 

    0x589e92cf,// 144 PAY 141 

    0x21dcf095,// 145 PAY 142 

    0xb5e00de6,// 146 PAY 143 

    0x2d31bfb9,// 147 PAY 144 

    0x5a6e3bc8,// 148 PAY 145 

    0x0bc0ef1d,// 149 PAY 146 

    0xe1c212fe,// 150 PAY 147 

    0xf8aad984,// 151 PAY 148 

    0x6b106c0a,// 152 PAY 149 

    0xc73d3c74,// 153 PAY 150 

    0x1fe1cd59,// 154 PAY 151 

    0xa56665fd,// 155 PAY 152 

    0x391d698f,// 156 PAY 153 

    0x737cfafe,// 157 PAY 154 

    0x0d9984a7,// 158 PAY 155 

    0xec14bdda,// 159 PAY 156 

    0x690c897d,// 160 PAY 157 

    0xe9c5f97e,// 161 PAY 158 

    0xf816d6a0,// 162 PAY 159 

    0x5aa5251d,// 163 PAY 160 

    0x90b6f60b,// 164 PAY 161 

    0x187bf876,// 165 PAY 162 

    0xc50ff409,// 166 PAY 163 

    0xf440686c,// 167 PAY 164 

    0xf0816f28,// 168 PAY 165 

    0xfa860648,// 169 PAY 166 

    0x04956f81,// 170 PAY 167 

    0x601974a4,// 171 PAY 168 

    0x012d1072,// 172 PAY 169 

    0x9cf39322,// 173 PAY 170 

    0xbeac4a8e,// 174 PAY 171 

    0x57bf3d28,// 175 PAY 172 

    0xe42e694f,// 176 PAY 173 

    0xce94e040,// 177 PAY 174 

    0xdcdfa0d2,// 178 PAY 175 

    0xea775b70,// 179 PAY 176 

    0x1f92caa4,// 180 PAY 177 

    0xe8458786,// 181 PAY 178 

    0x877ca4d3,// 182 PAY 179 

    0x6a2c966c,// 183 PAY 180 

    0x6ec40b66,// 184 PAY 181 

    0x78950268,// 185 PAY 182 

    0x12dac8ba,// 186 PAY 183 

    0x5d5c95b4,// 187 PAY 184 

    0x8993d540,// 188 PAY 185 

    0x7aef410f,// 189 PAY 186 

    0x120ef40c,// 190 PAY 187 

    0xb684021f,// 191 PAY 188 

    0xbe275046,// 192 PAY 189 

    0xaa477081,// 193 PAY 190 

    0x93b1218d,// 194 PAY 191 

    0x466eeb42,// 195 PAY 192 

    0x9e1007f5,// 196 PAY 193 

    0x4f070c84,// 197 PAY 194 

    0x063ab327,// 198 PAY 195 

    0xb89feb80,// 199 PAY 196 

    0xe09e02b7,// 200 PAY 197 

    0xbae2c895,// 201 PAY 198 

    0x38fe0340,// 202 PAY 199 

    0x3985b39f,// 203 PAY 200 

    0x18c9d6be,// 204 PAY 201 

    0x776b92f3,// 205 PAY 202 

    0x85b36fc9,// 206 PAY 203 

    0xd463e6e5,// 207 PAY 204 

    0x1a0d6ec5,// 208 PAY 205 

    0x3185b1d9,// 209 PAY 206 

    0x293a8517,// 210 PAY 207 

    0x2c4934a8,// 211 PAY 208 

    0xc5263179,// 212 PAY 209 

    0x5f2d449c,// 213 PAY 210 

    0x70e353e6,// 214 PAY 211 

    0x2ee3cc21,// 215 PAY 212 

    0x5c084def,// 216 PAY 213 

    0x5a529c26,// 217 PAY 214 

    0x341f69a7,// 218 PAY 215 

    0x064ac55a,// 219 PAY 216 

    0xdd1c5162,// 220 PAY 217 

    0xf901b5c3,// 221 PAY 218 

    0xcf6f431c,// 222 PAY 219 

    0x5dc19100,// 223 PAY 220 

    0xc190d7be,// 224 PAY 221 

    0x14373ef6,// 225 PAY 222 

    0xbcdbae8d,// 226 PAY 223 

    0x61b5beb0,// 227 PAY 224 

    0xf366bf9b,// 228 PAY 225 

    0x13d069ff,// 229 PAY 226 

    0xdd24f7f4,// 230 PAY 227 

    0x93278519,// 231 PAY 228 

    0xa10b7de2,// 232 PAY 229 

    0x9e3008ea,// 233 PAY 230 

    0xf02947b2,// 234 PAY 231 

    0x513659ad,// 235 PAY 232 

    0xe212f32c,// 236 PAY 233 

    0xf4c41880,// 237 PAY 234 

    0x45dd9c77,// 238 PAY 235 

    0x5ab106ea,// 239 PAY 236 

    0x68f2be1c,// 240 PAY 237 

    0x3e7350e1,// 241 PAY 238 

    0x72351bd3,// 242 PAY 239 

    0x2b445239,// 243 PAY 240 

    0x1438976f,// 244 PAY 241 

    0xe308c296,// 245 PAY 242 

    0x6f209e81,// 246 PAY 243 

    0xb877d881,// 247 PAY 244 

    0x66216239,// 248 PAY 245 

    0x410bcb92,// 249 PAY 246 

    0xa96979f1,// 250 PAY 247 

    0x0f0ac51b,// 251 PAY 248 

    0x5ea4fcab,// 252 PAY 249 

    0xb8baf5ff,// 253 PAY 250 

    0x6fa04589,// 254 PAY 251 

    0xebb2df75,// 255 PAY 252 

    0xfb10e9a1,// 256 PAY 253 

    0x3d87b174,// 257 PAY 254 

    0xebadcf5d,// 258 PAY 255 

    0x544525fe,// 259 PAY 256 

    0xc0cfc622,// 260 PAY 257 

    0xd8eaf135,// 261 PAY 258 

    0xc042e2d2,// 262 PAY 259 

    0xbd4ad897,// 263 PAY 260 

    0x3053f90e,// 264 PAY 261 

    0x8fa49dac,// 265 PAY 262 

    0xeddabe62,// 266 PAY 263 

    0xa880a85c,// 267 PAY 264 

    0x9f276f5b,// 268 PAY 265 

    0x84878e20,// 269 PAY 266 

    0xfcf550e6,// 270 PAY 267 

    0xa4f5257d,// 271 PAY 268 

    0x67ee3cb4,// 272 PAY 269 

    0x5c45556d,// 273 PAY 270 

    0xfeeaab62,// 274 PAY 271 

    0x6fe71d69,// 275 PAY 272 

    0x5264cd80,// 276 PAY 273 

    0x0f6f073d,// 277 PAY 274 

    0xd926bea2,// 278 PAY 275 

    0x1c1d4f85,// 279 PAY 276 

    0xf55cc9c3,// 280 PAY 277 

    0x78361e63,// 281 PAY 278 

    0x037b06e1,// 282 PAY 279 

    0xb668b7a6,// 283 PAY 280 

    0x22e8dd98,// 284 PAY 281 

    0x740d4861,// 285 PAY 282 

    0x3f9e66f6,// 286 PAY 283 

    0x0985b97f,// 287 PAY 284 

    0xbb248057,// 288 PAY 285 

    0x8cf6400d,// 289 PAY 286 

    0x342c907c,// 290 PAY 287 

    0x465ae590,// 291 PAY 288 

    0xa056f9d3,// 292 PAY 289 

    0x7c44eab2,// 293 PAY 290 

    0x2896146c,// 294 PAY 291 

    0x02d2d9bb,// 295 PAY 292 

    0x7fe4b7ff,// 296 PAY 293 

    0x4830a1fa,// 297 PAY 294 

    0x4368be9e,// 298 PAY 295 

    0x23f22c5f,// 299 PAY 296 

    0x31cbc236,// 300 PAY 297 

    0x61b0e71d,// 301 PAY 298 

    0x8b47badd,// 302 PAY 299 

    0x971a1435,// 303 PAY 300 

    0x2b1e3f70,// 304 PAY 301 

    0x6f13cc83,// 305 PAY 302 

    0x194bb124,// 306 PAY 303 

    0xc461d86d,// 307 PAY 304 

    0x242c4b46,// 308 PAY 305 

    0x09606cef,// 309 PAY 306 

    0x7bb7555e,// 310 PAY 307 

    0x5549d496,// 311 PAY 308 

    0x0fbc96e1,// 312 PAY 309 

    0xe80e04a4,// 313 PAY 310 

    0x819245be,// 314 PAY 311 

    0xba7484dd,// 315 PAY 312 

    0x9a099a64,// 316 PAY 313 

    0xab5f61e9,// 317 PAY 314 

    0x58971232,// 318 PAY 315 

    0x20bc289a,// 319 PAY 316 

    0xd29a7f31,// 320 PAY 317 

    0x9cc1334e,// 321 PAY 318 

    0x87d9ac9d,// 322 PAY 319 

    0xb1c92ff2,// 323 PAY 320 

    0xfda83c68,// 324 PAY 321 

    0x859ac3bf,// 325 PAY 322 

    0xff8e5943,// 326 PAY 323 

    0x0ebf6408,// 327 PAY 324 

    0xc06d72f9,// 328 PAY 325 

    0x2c0258c8,// 329 PAY 326 

    0x773c86d0,// 330 PAY 327 

    0x1b08a37e,// 331 PAY 328 

    0xa7139a9a,// 332 PAY 329 

    0x8e2cc8d5,// 333 PAY 330 

    0x3fda5a89,// 334 PAY 331 

    0x77cc8d85,// 335 PAY 332 

    0x413ce4f1,// 336 PAY 333 

    0x347bd614,// 337 PAY 334 

    0x29e8b196,// 338 PAY 335 

    0xa305686c,// 339 PAY 336 

    0x4b2c0016,// 340 PAY 337 

    0x87d0127a,// 341 PAY 338 

    0xedffc91b,// 342 PAY 339 

    0x7ddcbebf,// 343 PAY 340 

    0x86778bf4,// 344 PAY 341 

    0x7477e203,// 345 PAY 342 

    0x1457a068,// 346 PAY 343 

    0x2d2c2a63,// 347 PAY 344 

    0x3b0d724f,// 348 PAY 345 

    0x8a954138,// 349 PAY 346 

    0xf993f391,// 350 PAY 347 

    0x9e176649,// 351 PAY 348 

    0x0850ade8,// 352 PAY 349 

    0xfcbeb585,// 353 PAY 350 

    0x016bb2dd,// 354 PAY 351 

    0x6b95f0bf,// 355 PAY 352 

    0x9844a3fb,// 356 PAY 353 

    0x4e84743f,// 357 PAY 354 

    0xc333ac43,// 358 PAY 355 

    0x18a412d8,// 359 PAY 356 

    0x0e1eeb14,// 360 PAY 357 

    0x9f23d08d,// 361 PAY 358 

    0x65f999a8,// 362 PAY 359 

    0xda2219b6,// 363 PAY 360 

    0xf95bdf5f,// 364 PAY 361 

    0x04b142fc,// 365 PAY 362 

    0x9e2c2ab9,// 366 PAY 363 

    0x81bb2494,// 367 PAY 364 

    0x049ce7e5,// 368 PAY 365 

    0x2ee97d0d,// 369 PAY 366 

    0x520d2898,// 370 PAY 367 

    0xb6ecb8f9,// 371 PAY 368 

    0xcd0b663c,// 372 PAY 369 

    0x3d96914e,// 373 PAY 370 

    0x2dd99eb8,// 374 PAY 371 

    0x646130f5,// 375 PAY 372 

    0xb9a8acd5,// 376 PAY 373 

    0xbae24947,// 377 PAY 374 

    0x25ebdd79,// 378 PAY 375 

    0x8821e7c2,// 379 PAY 376 

    0x4f9d33c5,// 380 PAY 377 

    0xb57af374,// 381 PAY 378 

    0x8733fdf9,// 382 PAY 379 

    0xb47bd71a,// 383 PAY 380 

    0x48a175e1,// 384 PAY 381 

    0xf92f3361,// 385 PAY 382 

    0xb0938054,// 386 PAY 383 

    0x58cef1ae,// 387 PAY 384 

    0xfa3fe6c7,// 388 PAY 385 

    0xa5d1bc3b,// 389 PAY 386 

    0x43b4f5f1,// 390 PAY 387 

    0x7bac1212,// 391 PAY 388 

    0x15455525,// 392 PAY 389 

    0xd260e866,// 393 PAY 390 

    0x3985eb36,// 394 PAY 391 

    0xfd0d18e7,// 395 PAY 392 

    0x06e5a275,// 396 PAY 393 

    0xdfa17cb3,// 397 PAY 394 

    0xe4dfa506,// 398 PAY 395 

    0x186874b3,// 399 PAY 396 

    0x5550b7d0,// 400 PAY 397 

    0xe6b4eaab,// 401 PAY 398 

    0xd0c323d3,// 402 PAY 399 

    0x845fc83b,// 403 PAY 400 

    0xced4601b,// 404 PAY 401 

    0x120aeb64,// 405 PAY 402 

    0xceb88eb5,// 406 PAY 403 

    0xb7a17839,// 407 PAY 404 

    0x727d0d5e,// 408 PAY 405 

    0x3113aec0,// 409 PAY 406 

    0xd5c0c45f,// 410 PAY 407 

    0x3742fc91,// 411 PAY 408 

    0xc19089f6,// 412 PAY 409 

    0x0e9fc4c9,// 413 PAY 410 

    0x8e15ad4b,// 414 PAY 411 

    0xf68e3878,// 415 PAY 412 

    0x33634524,// 416 PAY 413 

    0xe377b63b,// 417 PAY 414 

    0xc8c96894,// 418 PAY 415 

    0x100819ae,// 419 PAY 416 

    0x9961b3f7,// 420 PAY 417 

    0x055ead1d,// 421 PAY 418 

    0xeea81d65,// 422 PAY 419 

    0x5b5ab074,// 423 PAY 420 

    0x383bca28,// 424 PAY 421 

    0xecefe627,// 425 PAY 422 

    0x51786ac5,// 426 PAY 423 

    0xf135a413,// 427 PAY 424 

    0x4cc6d688,// 428 PAY 425 

    0x5a417b00,// 429 PAY 426 

    0xeb711d8e,// 430 PAY 427 

    0x3bda0000,// 431 PAY 428 

/// STA is 1 words. 

/// STA num_pkts       : 154 

/// STA pkt_idx        : 101 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4a 

    0x01954a9a // 432 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt42_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 265 words. 

/// BDA size     is 1053 (0x41d) 

/// BDA id       is 0x80ec 

    0x041d80ec,// 3 BDA   1 

/// PAY Generic Data size   : 1053 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x3046d8d7,// 4 PAY   1 

    0x0f148944,// 5 PAY   2 

    0x503eedeb,// 6 PAY   3 

    0x06d8752f,// 7 PAY   4 

    0xb065f160,// 8 PAY   5 

    0x4427e07e,// 9 PAY   6 

    0x9773f6f4,// 10 PAY   7 

    0xa6a342da,// 11 PAY   8 

    0x0b66e3ee,// 12 PAY   9 

    0xcccd1d65,// 13 PAY  10 

    0xb8fee531,// 14 PAY  11 

    0xbe64894a,// 15 PAY  12 

    0xdbdc4bf5,// 16 PAY  13 

    0x87d0b47b,// 17 PAY  14 

    0x96ce3f15,// 18 PAY  15 

    0xdc762a91,// 19 PAY  16 

    0xd8669718,// 20 PAY  17 

    0xc26edd9e,// 21 PAY  18 

    0x1597794d,// 22 PAY  19 

    0x43f4bc25,// 23 PAY  20 

    0x56144b20,// 24 PAY  21 

    0xfc5f6e8b,// 25 PAY  22 

    0xb1b02a54,// 26 PAY  23 

    0xee6e8c85,// 27 PAY  24 

    0x6024739d,// 28 PAY  25 

    0xcfd4b883,// 29 PAY  26 

    0xfe635414,// 30 PAY  27 

    0xd02c28b6,// 31 PAY  28 

    0xc3b5ccf4,// 32 PAY  29 

    0xa1458a80,// 33 PAY  30 

    0x7be1b713,// 34 PAY  31 

    0x78a08153,// 35 PAY  32 

    0x32648c39,// 36 PAY  33 

    0x99b4ffdd,// 37 PAY  34 

    0x2cf784c5,// 38 PAY  35 

    0x7073c016,// 39 PAY  36 

    0x92339b1e,// 40 PAY  37 

    0x6a1587e8,// 41 PAY  38 

    0x3ed3c510,// 42 PAY  39 

    0xb5aa258c,// 43 PAY  40 

    0xfa2be0ac,// 44 PAY  41 

    0x7429e582,// 45 PAY  42 

    0x231f21a8,// 46 PAY  43 

    0xf96ef710,// 47 PAY  44 

    0xf9e02451,// 48 PAY  45 

    0xfcda6bab,// 49 PAY  46 

    0xaae43e17,// 50 PAY  47 

    0xc61bc7e6,// 51 PAY  48 

    0xe73b2580,// 52 PAY  49 

    0x50c53eba,// 53 PAY  50 

    0x078b442d,// 54 PAY  51 

    0x29ac40fd,// 55 PAY  52 

    0x5aaac4ff,// 56 PAY  53 

    0xd7d58eeb,// 57 PAY  54 

    0x42836e63,// 58 PAY  55 

    0x97f110a7,// 59 PAY  56 

    0xeead643b,// 60 PAY  57 

    0x100fbb0c,// 61 PAY  58 

    0xa9de26bf,// 62 PAY  59 

    0x9408c230,// 63 PAY  60 

    0xa81faccb,// 64 PAY  61 

    0x0c79657c,// 65 PAY  62 

    0x9da72206,// 66 PAY  63 

    0x3d5b520e,// 67 PAY  64 

    0x5883ad68,// 68 PAY  65 

    0xe98440ad,// 69 PAY  66 

    0xdc26b77e,// 70 PAY  67 

    0x5eeb5653,// 71 PAY  68 

    0xb8353236,// 72 PAY  69 

    0xd9d1f67c,// 73 PAY  70 

    0x250fc03e,// 74 PAY  71 

    0x31747fe5,// 75 PAY  72 

    0x682b24aa,// 76 PAY  73 

    0x60cf1610,// 77 PAY  74 

    0x6015f3d3,// 78 PAY  75 

    0xaeb68296,// 79 PAY  76 

    0x357ca54a,// 80 PAY  77 

    0x07cbb48a,// 81 PAY  78 

    0x67b1592e,// 82 PAY  79 

    0x8bf18de5,// 83 PAY  80 

    0x591bb89c,// 84 PAY  81 

    0x45e9fa2a,// 85 PAY  82 

    0x37c1046b,// 86 PAY  83 

    0x441dc91d,// 87 PAY  84 

    0x51ba4c6a,// 88 PAY  85 

    0x3a328bf6,// 89 PAY  86 

    0x432de29b,// 90 PAY  87 

    0xdc699b56,// 91 PAY  88 

    0x23699d7d,// 92 PAY  89 

    0x96c53a94,// 93 PAY  90 

    0x451eec0e,// 94 PAY  91 

    0x535b87d4,// 95 PAY  92 

    0xb167d4aa,// 96 PAY  93 

    0xe65e9460,// 97 PAY  94 

    0x596704ca,// 98 PAY  95 

    0xf13b778f,// 99 PAY  96 

    0xef62e387,// 100 PAY  97 

    0x5647601c,// 101 PAY  98 

    0xb81a0e00,// 102 PAY  99 

    0x84425321,// 103 PAY 100 

    0x74ef25f6,// 104 PAY 101 

    0x8b4215a5,// 105 PAY 102 

    0xbb1dde38,// 106 PAY 103 

    0x06c2006b,// 107 PAY 104 

    0x702c4b95,// 108 PAY 105 

    0x939e7730,// 109 PAY 106 

    0x6e7d6048,// 110 PAY 107 

    0x872a7b3d,// 111 PAY 108 

    0xa8a20fd2,// 112 PAY 109 

    0x7c68f39d,// 113 PAY 110 

    0x5249f980,// 114 PAY 111 

    0x61407be3,// 115 PAY 112 

    0x6b327764,// 116 PAY 113 

    0x8582f093,// 117 PAY 114 

    0xf0a8b005,// 118 PAY 115 

    0x2b906eab,// 119 PAY 116 

    0x97a45721,// 120 PAY 117 

    0x3c9f2e1e,// 121 PAY 118 

    0x806aecc7,// 122 PAY 119 

    0x17ea9029,// 123 PAY 120 

    0x8046532e,// 124 PAY 121 

    0xad0f0fe6,// 125 PAY 122 

    0x785d4727,// 126 PAY 123 

    0xadf3f019,// 127 PAY 124 

    0x705d9714,// 128 PAY 125 

    0x97e326cf,// 129 PAY 126 

    0x341ebac1,// 130 PAY 127 

    0x09c51616,// 131 PAY 128 

    0x60bbed38,// 132 PAY 129 

    0xa1c0b1c0,// 133 PAY 130 

    0x55aa6426,// 134 PAY 131 

    0x97e69b55,// 135 PAY 132 

    0x4def24b8,// 136 PAY 133 

    0x7d70a534,// 137 PAY 134 

    0x2618a4dd,// 138 PAY 135 

    0xd5ddb035,// 139 PAY 136 

    0x00ebd56d,// 140 PAY 137 

    0x8ace5a6f,// 141 PAY 138 

    0x5f31f803,// 142 PAY 139 

    0x98dcf9be,// 143 PAY 140 

    0xdcdb9913,// 144 PAY 141 

    0xea38b643,// 145 PAY 142 

    0xaa99e142,// 146 PAY 143 

    0xff5a5e13,// 147 PAY 144 

    0x27b1c63d,// 148 PAY 145 

    0x879319d5,// 149 PAY 146 

    0xb5ece30e,// 150 PAY 147 

    0xcdd233bf,// 151 PAY 148 

    0xf272d8ad,// 152 PAY 149 

    0xc24c9cc1,// 153 PAY 150 

    0x2443bc6c,// 154 PAY 151 

    0xd6750736,// 155 PAY 152 

    0x7a133301,// 156 PAY 153 

    0x9e290e77,// 157 PAY 154 

    0x280bd5aa,// 158 PAY 155 

    0xe9496f3f,// 159 PAY 156 

    0x4dae7e30,// 160 PAY 157 

    0x31002a21,// 161 PAY 158 

    0x00809e82,// 162 PAY 159 

    0xdac06543,// 163 PAY 160 

    0xa507726e,// 164 PAY 161 

    0x6ebef936,// 165 PAY 162 

    0x74373e84,// 166 PAY 163 

    0x0883059e,// 167 PAY 164 

    0x5d36e54f,// 168 PAY 165 

    0x0633e8eb,// 169 PAY 166 

    0xc9229687,// 170 PAY 167 

    0xbbdd6890,// 171 PAY 168 

    0x6bd05a00,// 172 PAY 169 

    0x8c76642f,// 173 PAY 170 

    0x0c3a63e2,// 174 PAY 171 

    0x837cbf4c,// 175 PAY 172 

    0xbc9064a0,// 176 PAY 173 

    0xb67cebb7,// 177 PAY 174 

    0xc844604c,// 178 PAY 175 

    0x9718dd97,// 179 PAY 176 

    0x47bbb555,// 180 PAY 177 

    0x91a87709,// 181 PAY 178 

    0x5bd99efd,// 182 PAY 179 

    0x4bb723de,// 183 PAY 180 

    0x796bbe2c,// 184 PAY 181 

    0x38a2795b,// 185 PAY 182 

    0x6e542684,// 186 PAY 183 

    0x563bbeee,// 187 PAY 184 

    0xc4576dc1,// 188 PAY 185 

    0x87bec1b3,// 189 PAY 186 

    0x3bfc60de,// 190 PAY 187 

    0xb510db9c,// 191 PAY 188 

    0xeab051ff,// 192 PAY 189 

    0xb4f92ad2,// 193 PAY 190 

    0xba0ba322,// 194 PAY 191 

    0xbcc09c8e,// 195 PAY 192 

    0xfa26dfdb,// 196 PAY 193 

    0x4f511ca6,// 197 PAY 194 

    0x4602682a,// 198 PAY 195 

    0x50df3842,// 199 PAY 196 

    0x25e023ad,// 200 PAY 197 

    0x83c074e0,// 201 PAY 198 

    0x5ae44ce6,// 202 PAY 199 

    0x29ec93c8,// 203 PAY 200 

    0xc6365207,// 204 PAY 201 

    0x1dc71678,// 205 PAY 202 

    0x7df23dc5,// 206 PAY 203 

    0x0c689e60,// 207 PAY 204 

    0x19d72f5c,// 208 PAY 205 

    0xb2a50a6d,// 209 PAY 206 

    0x26ee061c,// 210 PAY 207 

    0x663c8ca9,// 211 PAY 208 

    0x2b4d3dd4,// 212 PAY 209 

    0x3e002986,// 213 PAY 210 

    0xf7a6b1ae,// 214 PAY 211 

    0xdee5f6d7,// 215 PAY 212 

    0xc2ea4f54,// 216 PAY 213 

    0x62cf7239,// 217 PAY 214 

    0x6b37f936,// 218 PAY 215 

    0x0141b240,// 219 PAY 216 

    0x3fe0c46c,// 220 PAY 217 

    0xa7152dca,// 221 PAY 218 

    0x13923f56,// 222 PAY 219 

    0x07d0788c,// 223 PAY 220 

    0x64b7c1ab,// 224 PAY 221 

    0xd5260a01,// 225 PAY 222 

    0x20456ea3,// 226 PAY 223 

    0xbc4a75e7,// 227 PAY 224 

    0x6c0160d1,// 228 PAY 225 

    0xefe1760f,// 229 PAY 226 

    0xd6438f2e,// 230 PAY 227 

    0x777c4f0b,// 231 PAY 228 

    0x9e07f9b0,// 232 PAY 229 

    0x00a9e900,// 233 PAY 230 

    0xd9ff0ada,// 234 PAY 231 

    0xa0ee2462,// 235 PAY 232 

    0xbd01670b,// 236 PAY 233 

    0xa3d808dd,// 237 PAY 234 

    0xf08cc747,// 238 PAY 235 

    0xb1d62a5a,// 239 PAY 236 

    0xd965f48e,// 240 PAY 237 

    0x108cda0f,// 241 PAY 238 

    0x69839443,// 242 PAY 239 

    0x8d167e87,// 243 PAY 240 

    0x489e143d,// 244 PAY 241 

    0x38443208,// 245 PAY 242 

    0x92423f1b,// 246 PAY 243 

    0xd586d36a,// 247 PAY 244 

    0x49d3da6e,// 248 PAY 245 

    0x211d34ad,// 249 PAY 246 

    0xb45e3c1f,// 250 PAY 247 

    0x5a09a78a,// 251 PAY 248 

    0xa28b2b94,// 252 PAY 249 

    0xe6782cf7,// 253 PAY 250 

    0x88e19dc1,// 254 PAY 251 

    0xffeed6ae,// 255 PAY 252 

    0x231d9ee1,// 256 PAY 253 

    0xc0b43afe,// 257 PAY 254 

    0x3f113c37,// 258 PAY 255 

    0x16afc947,// 259 PAY 256 

    0xa5710d88,// 260 PAY 257 

    0x38646275,// 261 PAY 258 

    0x09182bc9,// 262 PAY 259 

    0x04bcf778,// 263 PAY 260 

    0x9a1fcaa3,// 264 PAY 261 

    0x53c7e99f,// 265 PAY 262 

    0xa14682ae,// 266 PAY 263 

    0x95000000,// 267 PAY 264 

/// HASH is  12 bytes 

    0x92423f1b,// 268 HSH   1 

    0xd586d36a,// 269 HSH   2 

    0x49d3da6e,// 270 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 54 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8b 

    0x00d98b2d // 271 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt43_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 383 words. 

/// BDA size     is 1528 (0x5f8) 

/// BDA id       is 0x6967 

    0x05f86967,// 3 BDA   1 

/// PAY Generic Data size   : 1528 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xdc1d8359,// 4 PAY   1 

    0xbe55f8bf,// 5 PAY   2 

    0x660c8886,// 6 PAY   3 

    0x52171b52,// 7 PAY   4 

    0x7864862a,// 8 PAY   5 

    0x9560faa1,// 9 PAY   6 

    0x434cdc05,// 10 PAY   7 

    0xbfae01b3,// 11 PAY   8 

    0x2149068e,// 12 PAY   9 

    0x693628e8,// 13 PAY  10 

    0x1a03a2c9,// 14 PAY  11 

    0x2664b067,// 15 PAY  12 

    0x6c52d28d,// 16 PAY  13 

    0x49e11566,// 17 PAY  14 

    0xd34b2785,// 18 PAY  15 

    0x0bfc9322,// 19 PAY  16 

    0x456a6fbf,// 20 PAY  17 

    0x08056ccb,// 21 PAY  18 

    0x5ce283b5,// 22 PAY  19 

    0x58911d54,// 23 PAY  20 

    0x4d3bc937,// 24 PAY  21 

    0x98aebd71,// 25 PAY  22 

    0x6ea3e06e,// 26 PAY  23 

    0xb71ce636,// 27 PAY  24 

    0xe2fab29c,// 28 PAY  25 

    0x3df7c26b,// 29 PAY  26 

    0x0f0c73db,// 30 PAY  27 

    0x8482c192,// 31 PAY  28 

    0xf0e42b61,// 32 PAY  29 

    0x2b414053,// 33 PAY  30 

    0x9195124e,// 34 PAY  31 

    0x9c7af29a,// 35 PAY  32 

    0x2f0cd025,// 36 PAY  33 

    0x26884f9d,// 37 PAY  34 

    0x81a66247,// 38 PAY  35 

    0xc27c753f,// 39 PAY  36 

    0xc5d88aa2,// 40 PAY  37 

    0x50a548a7,// 41 PAY  38 

    0x87ed2011,// 42 PAY  39 

    0x65426f4f,// 43 PAY  40 

    0x5ba9a4b3,// 44 PAY  41 

    0xee680b2e,// 45 PAY  42 

    0x0e75ac5d,// 46 PAY  43 

    0x25aa18ca,// 47 PAY  44 

    0xc41a9bff,// 48 PAY  45 

    0x17bd8319,// 49 PAY  46 

    0x2ab16c8a,// 50 PAY  47 

    0x8a2ab4e6,// 51 PAY  48 

    0x40378330,// 52 PAY  49 

    0x7ed9dab7,// 53 PAY  50 

    0x64bdff1e,// 54 PAY  51 

    0xc88503c7,// 55 PAY  52 

    0x38c47548,// 56 PAY  53 

    0x44556e26,// 57 PAY  54 

    0x7ec033c7,// 58 PAY  55 

    0x1bde4307,// 59 PAY  56 

    0x92e9be8d,// 60 PAY  57 

    0x79096ca1,// 61 PAY  58 

    0x17713751,// 62 PAY  59 

    0xeb80462e,// 63 PAY  60 

    0x7996018a,// 64 PAY  61 

    0x3644601a,// 65 PAY  62 

    0x0c46a786,// 66 PAY  63 

    0x5d48aeca,// 67 PAY  64 

    0x0c963f33,// 68 PAY  65 

    0xdd778a56,// 69 PAY  66 

    0x814bb0e5,// 70 PAY  67 

    0xd7d9fb9e,// 71 PAY  68 

    0x76a00d29,// 72 PAY  69 

    0x6dbe70d6,// 73 PAY  70 

    0x66750b33,// 74 PAY  71 

    0x9d7dda0c,// 75 PAY  72 

    0x6d038823,// 76 PAY  73 

    0x42bda584,// 77 PAY  74 

    0xfdf05e5a,// 78 PAY  75 

    0xf58c5e52,// 79 PAY  76 

    0x7623c6fa,// 80 PAY  77 

    0x3ba4b31c,// 81 PAY  78 

    0xb3ee1a94,// 82 PAY  79 

    0x27d11c03,// 83 PAY  80 

    0x69f18fc1,// 84 PAY  81 

    0x1b13f111,// 85 PAY  82 

    0x4633837a,// 86 PAY  83 

    0x8c1ec001,// 87 PAY  84 

    0x845b4841,// 88 PAY  85 

    0x432a8cca,// 89 PAY  86 

    0x8ee35a2b,// 90 PAY  87 

    0x403e3dc0,// 91 PAY  88 

    0x4cbad2dd,// 92 PAY  89 

    0xf2f23ac8,// 93 PAY  90 

    0xc6b50958,// 94 PAY  91 

    0x4cf5a962,// 95 PAY  92 

    0xd5ff3190,// 96 PAY  93 

    0x2b710772,// 97 PAY  94 

    0x5c00a82c,// 98 PAY  95 

    0xaebb34ba,// 99 PAY  96 

    0x885d71c5,// 100 PAY  97 

    0x18ab1e5b,// 101 PAY  98 

    0xfda206ac,// 102 PAY  99 

    0x37a2f39e,// 103 PAY 100 

    0x6bfdacee,// 104 PAY 101 

    0xde11e19f,// 105 PAY 102 

    0xfbe0967e,// 106 PAY 103 

    0x34151043,// 107 PAY 104 

    0x2d5683b1,// 108 PAY 105 

    0x412b20ea,// 109 PAY 106 

    0xe470778d,// 110 PAY 107 

    0x758ae662,// 111 PAY 108 

    0x1fe27201,// 112 PAY 109 

    0x30a2c696,// 113 PAY 110 

    0x0519f00d,// 114 PAY 111 

    0x2ec61d54,// 115 PAY 112 

    0x1f3c4548,// 116 PAY 113 

    0x1bea883c,// 117 PAY 114 

    0xe1d5e86e,// 118 PAY 115 

    0xb1dac20f,// 119 PAY 116 

    0xc8acdd01,// 120 PAY 117 

    0x73dfb4be,// 121 PAY 118 

    0xb2626e13,// 122 PAY 119 

    0x7d3321ae,// 123 PAY 120 

    0xc03736c3,// 124 PAY 121 

    0x6f94d938,// 125 PAY 122 

    0xd6fa8325,// 126 PAY 123 

    0xedc17d0c,// 127 PAY 124 

    0x71472c2c,// 128 PAY 125 

    0x5b5995d3,// 129 PAY 126 

    0xa466a94f,// 130 PAY 127 

    0x6f4f35f4,// 131 PAY 128 

    0x07e4ff74,// 132 PAY 129 

    0x885182f0,// 133 PAY 130 

    0xf1551f72,// 134 PAY 131 

    0x623b4ca7,// 135 PAY 132 

    0x8316bcbd,// 136 PAY 133 

    0x6178c0d3,// 137 PAY 134 

    0xd571805e,// 138 PAY 135 

    0x76c63853,// 139 PAY 136 

    0xc1f32d55,// 140 PAY 137 

    0x45eec641,// 141 PAY 138 

    0x3356ddc3,// 142 PAY 139 

    0x4df7d237,// 143 PAY 140 

    0x549441f2,// 144 PAY 141 

    0xca6dd4ce,// 145 PAY 142 

    0xcf9a2eb9,// 146 PAY 143 

    0xea6a9f7b,// 147 PAY 144 

    0xaa87d01f,// 148 PAY 145 

    0x4500caac,// 149 PAY 146 

    0x825ab6e3,// 150 PAY 147 

    0xce1d670d,// 151 PAY 148 

    0x2de97bec,// 152 PAY 149 

    0xc03175e8,// 153 PAY 150 

    0xf2381677,// 154 PAY 151 

    0xdf86e029,// 155 PAY 152 

    0xa07199cf,// 156 PAY 153 

    0x901a4d64,// 157 PAY 154 

    0xc9ea1734,// 158 PAY 155 

    0x63a0832c,// 159 PAY 156 

    0xc2187975,// 160 PAY 157 

    0xe392c07b,// 161 PAY 158 

    0xd3f0418d,// 162 PAY 159 

    0xae496652,// 163 PAY 160 

    0x5cfa05dd,// 164 PAY 161 

    0xe32ba245,// 165 PAY 162 

    0xddfb8243,// 166 PAY 163 

    0x887a02b7,// 167 PAY 164 

    0xc1c72406,// 168 PAY 165 

    0xc00c5ec6,// 169 PAY 166 

    0x4b1476cd,// 170 PAY 167 

    0x67882ce8,// 171 PAY 168 

    0x6154599d,// 172 PAY 169 

    0x02dfa674,// 173 PAY 170 

    0x312cd35b,// 174 PAY 171 

    0xf9beca88,// 175 PAY 172 

    0xf3eb1b5f,// 176 PAY 173 

    0x3715e1d8,// 177 PAY 174 

    0xab66c5ed,// 178 PAY 175 

    0x2a6673e9,// 179 PAY 176 

    0x675c7345,// 180 PAY 177 

    0x56907fa7,// 181 PAY 178 

    0xa3e3c5a9,// 182 PAY 179 

    0x972339c2,// 183 PAY 180 

    0x7018cf58,// 184 PAY 181 

    0x33f31020,// 185 PAY 182 

    0xb38052ab,// 186 PAY 183 

    0x6e0a0369,// 187 PAY 184 

    0x3e52bb5f,// 188 PAY 185 

    0xc633dffc,// 189 PAY 186 

    0xb9866174,// 190 PAY 187 

    0x01c29afb,// 191 PAY 188 

    0xd51c1727,// 192 PAY 189 

    0xafdda941,// 193 PAY 190 

    0x4f1e14c6,// 194 PAY 191 

    0x04ab4c20,// 195 PAY 192 

    0x27d2800d,// 196 PAY 193 

    0x3a5ed2f7,// 197 PAY 194 

    0xb3448724,// 198 PAY 195 

    0x5942f24b,// 199 PAY 196 

    0xd1eb867a,// 200 PAY 197 

    0xc9d62c12,// 201 PAY 198 

    0x6907170e,// 202 PAY 199 

    0x36cc46da,// 203 PAY 200 

    0xeb51d04c,// 204 PAY 201 

    0x7c4191b5,// 205 PAY 202 

    0xbd856415,// 206 PAY 203 

    0x235b5b60,// 207 PAY 204 

    0x9cea6ff0,// 208 PAY 205 

    0xd5142f3c,// 209 PAY 206 

    0x4b79fb41,// 210 PAY 207 

    0x54c94f8d,// 211 PAY 208 

    0x05795462,// 212 PAY 209 

    0x0d3c162c,// 213 PAY 210 

    0xd13d3386,// 214 PAY 211 

    0xc5452200,// 215 PAY 212 

    0xd1df9d68,// 216 PAY 213 

    0xfd9ca032,// 217 PAY 214 

    0xc8461fdd,// 218 PAY 215 

    0xe4e27c42,// 219 PAY 216 

    0x303e6e4e,// 220 PAY 217 

    0x16ab7717,// 221 PAY 218 

    0xe87c65ca,// 222 PAY 219 

    0x51282b33,// 223 PAY 220 

    0x61ddd2b6,// 224 PAY 221 

    0xd9d6a15a,// 225 PAY 222 

    0xf49c0ce2,// 226 PAY 223 

    0x085d19b7,// 227 PAY 224 

    0x44a765bb,// 228 PAY 225 

    0x24320917,// 229 PAY 226 

    0xcd970788,// 230 PAY 227 

    0x1b10903d,// 231 PAY 228 

    0x3f5fbf6d,// 232 PAY 229 

    0xfe0fcde1,// 233 PAY 230 

    0x86b723d2,// 234 PAY 231 

    0x629e1d2c,// 235 PAY 232 

    0x5b74031b,// 236 PAY 233 

    0x36ab324b,// 237 PAY 234 

    0x1ad98761,// 238 PAY 235 

    0xd56fdff1,// 239 PAY 236 

    0x655abb46,// 240 PAY 237 

    0x9f91c76c,// 241 PAY 238 

    0x97f9f765,// 242 PAY 239 

    0xa06773a6,// 243 PAY 240 

    0x5bea295d,// 244 PAY 241 

    0x0db64571,// 245 PAY 242 

    0x0d01c134,// 246 PAY 243 

    0xe0f72654,// 247 PAY 244 

    0x13c8a363,// 248 PAY 245 

    0x73edf896,// 249 PAY 246 

    0x52512bb7,// 250 PAY 247 

    0xa13f8cd0,// 251 PAY 248 

    0xba224f08,// 252 PAY 249 

    0x3f1b04df,// 253 PAY 250 

    0xf404bf4a,// 254 PAY 251 

    0x9e939f5f,// 255 PAY 252 

    0x4332213c,// 256 PAY 253 

    0x75908b1f,// 257 PAY 254 

    0xac658a86,// 258 PAY 255 

    0xa9eb14de,// 259 PAY 256 

    0xbec7d19f,// 260 PAY 257 

    0x6544d91a,// 261 PAY 258 

    0xbd1a67c4,// 262 PAY 259 

    0x4cc2b5a4,// 263 PAY 260 

    0xf43d7b58,// 264 PAY 261 

    0x796bb178,// 265 PAY 262 

    0x67703e7c,// 266 PAY 263 

    0x9daa4104,// 267 PAY 264 

    0x9e1416f9,// 268 PAY 265 

    0x6519bac9,// 269 PAY 266 

    0x4e14a4e5,// 270 PAY 267 

    0x76a26509,// 271 PAY 268 

    0xb2f528ce,// 272 PAY 269 

    0xeb74914a,// 273 PAY 270 

    0x3b7b65b6,// 274 PAY 271 

    0xd6d46c7e,// 275 PAY 272 

    0x1dc3df25,// 276 PAY 273 

    0x533576af,// 277 PAY 274 

    0xb062ddbb,// 278 PAY 275 

    0x3c19d72c,// 279 PAY 276 

    0xff5cc120,// 280 PAY 277 

    0x2bde3e40,// 281 PAY 278 

    0xd73925ff,// 282 PAY 279 

    0x283d53db,// 283 PAY 280 

    0x243e679f,// 284 PAY 281 

    0x06267c4c,// 285 PAY 282 

    0xcc3fc832,// 286 PAY 283 

    0x6ab19b20,// 287 PAY 284 

    0x9bea7b7b,// 288 PAY 285 

    0xec463d32,// 289 PAY 286 

    0x27de10ef,// 290 PAY 287 

    0x216294d0,// 291 PAY 288 

    0x2d44c85d,// 292 PAY 289 

    0x4cc0be0e,// 293 PAY 290 

    0x11e5e0cc,// 294 PAY 291 

    0xedc806a4,// 295 PAY 292 

    0xd0fa3872,// 296 PAY 293 

    0x461ac1ed,// 297 PAY 294 

    0xe0952e7c,// 298 PAY 295 

    0x275b7abb,// 299 PAY 296 

    0x90f2594f,// 300 PAY 297 

    0xd48fb6d2,// 301 PAY 298 

    0x9c27669e,// 302 PAY 299 

    0x96c62c88,// 303 PAY 300 

    0x3f79fca1,// 304 PAY 301 

    0xd23c3d81,// 305 PAY 302 

    0x7cbfde1c,// 306 PAY 303 

    0x845d54be,// 307 PAY 304 

    0xcce8d0db,// 308 PAY 305 

    0x5fbf8e5f,// 309 PAY 306 

    0x63e806e4,// 310 PAY 307 

    0x9eb55e2c,// 311 PAY 308 

    0xc5741310,// 312 PAY 309 

    0x581d3e38,// 313 PAY 310 

    0x49b4023a,// 314 PAY 311 

    0xf647c13c,// 315 PAY 312 

    0x09665316,// 316 PAY 313 

    0x037a93e1,// 317 PAY 314 

    0x49ad74ba,// 318 PAY 315 

    0xd2f08996,// 319 PAY 316 

    0x2fe0ad91,// 320 PAY 317 

    0x4cf73c07,// 321 PAY 318 

    0x3bf0fa97,// 322 PAY 319 

    0x80d6760b,// 323 PAY 320 

    0xf921e9fa,// 324 PAY 321 

    0xc2a4e26a,// 325 PAY 322 

    0x04471acb,// 326 PAY 323 

    0xd9d2f0cd,// 327 PAY 324 

    0x80aea640,// 328 PAY 325 

    0xa73a80e3,// 329 PAY 326 

    0xc7c2e3c7,// 330 PAY 327 

    0x5437e010,// 331 PAY 328 

    0xeb87a192,// 332 PAY 329 

    0xe6c8f8c5,// 333 PAY 330 

    0x641d2f7e,// 334 PAY 331 

    0xc1e619cd,// 335 PAY 332 

    0x58dd436c,// 336 PAY 333 

    0x4ecac045,// 337 PAY 334 

    0x443ab87e,// 338 PAY 335 

    0x442e698b,// 339 PAY 336 

    0x7c0d295b,// 340 PAY 337 

    0xe789c0b3,// 341 PAY 338 

    0x0594ee20,// 342 PAY 339 

    0xaf2fe2de,// 343 PAY 340 

    0x8d91dfea,// 344 PAY 341 

    0x6f91d3be,// 345 PAY 342 

    0xd7c7df43,// 346 PAY 343 

    0x866352f1,// 347 PAY 344 

    0x67b441a4,// 348 PAY 345 

    0x0beed0fa,// 349 PAY 346 

    0x18806012,// 350 PAY 347 

    0xa8836d72,// 351 PAY 348 

    0xb18ea763,// 352 PAY 349 

    0xa1cc8c44,// 353 PAY 350 

    0x956b7b7f,// 354 PAY 351 

    0x61cfa26e,// 355 PAY 352 

    0x55d117a6,// 356 PAY 353 

    0x70e24eb8,// 357 PAY 354 

    0x8f52e7c6,// 358 PAY 355 

    0xef1bec13,// 359 PAY 356 

    0x7b210ca9,// 360 PAY 357 

    0xe2ab2c4a,// 361 PAY 358 

    0x1e7c7bbb,// 362 PAY 359 

    0x975636e6,// 363 PAY 360 

    0x157ef952,// 364 PAY 361 

    0x601b8b3d,// 365 PAY 362 

    0x3d2152db,// 366 PAY 363 

    0xb36e799a,// 367 PAY 364 

    0x15e2cfa1,// 368 PAY 365 

    0x924d24bc,// 369 PAY 366 

    0x38f421e1,// 370 PAY 367 

    0xbda53637,// 371 PAY 368 

    0xa4635b9a,// 372 PAY 369 

    0xc262090a,// 373 PAY 370 

    0xcfcd1772,// 374 PAY 371 

    0xb0e24424,// 375 PAY 372 

    0x7bd9f802,// 376 PAY 373 

    0x046fb13e,// 377 PAY 374 

    0x7f4e3a04,// 378 PAY 375 

    0xb4ff1aae,// 379 PAY 376 

    0xc2ad5801,// 380 PAY 377 

    0x9a650247,// 381 PAY 378 

    0xda2f72be,// 382 PAY 379 

    0x2f094ae8,// 383 PAY 380 

    0x884679b1,// 384 PAY 381 

    0xadd602e3,// 385 PAY 382 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 232 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x79 

    0x03a07985 // 386 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt44_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 429 words. 

/// BDA size     is 1710 (0x6ae) 

/// BDA id       is 0x4bfd 

    0x06ae4bfd,// 3 BDA   1 

/// PAY Generic Data size   : 1710 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe7f2fad9,// 4 PAY   1 

    0xc254e694,// 5 PAY   2 

    0xfa0a9718,// 6 PAY   3 

    0xf5f1fa08,// 7 PAY   4 

    0x8205e728,// 8 PAY   5 

    0xb4a5c2fa,// 9 PAY   6 

    0x6277e23a,// 10 PAY   7 

    0x78015fd9,// 11 PAY   8 

    0xc1d3fec0,// 12 PAY   9 

    0x1d6561ac,// 13 PAY  10 

    0x968bbd3b,// 14 PAY  11 

    0x7124bc3c,// 15 PAY  12 

    0xa63b1cde,// 16 PAY  13 

    0x94a21e6c,// 17 PAY  14 

    0xc1afaa9f,// 18 PAY  15 

    0x83ef8487,// 19 PAY  16 

    0x0518dad7,// 20 PAY  17 

    0x22eabdcb,// 21 PAY  18 

    0x82ea3e4e,// 22 PAY  19 

    0x85200a74,// 23 PAY  20 

    0x5ccff99c,// 24 PAY  21 

    0x8dc2674f,// 25 PAY  22 

    0xf198d599,// 26 PAY  23 

    0x88b225ed,// 27 PAY  24 

    0xd6d46838,// 28 PAY  25 

    0xc61446bc,// 29 PAY  26 

    0xd3629310,// 30 PAY  27 

    0x7eb47e4b,// 31 PAY  28 

    0x4fb3799b,// 32 PAY  29 

    0x764fb0f6,// 33 PAY  30 

    0x2e5cff24,// 34 PAY  31 

    0xa02af2cb,// 35 PAY  32 

    0xdd45dc04,// 36 PAY  33 

    0x563388cb,// 37 PAY  34 

    0x53923819,// 38 PAY  35 

    0x66d8c574,// 39 PAY  36 

    0xbbfe8316,// 40 PAY  37 

    0xd412faa3,// 41 PAY  38 

    0x78006e13,// 42 PAY  39 

    0xd47642f1,// 43 PAY  40 

    0xfaba300d,// 44 PAY  41 

    0xd771d6ed,// 45 PAY  42 

    0x5a4aa526,// 46 PAY  43 

    0x42759087,// 47 PAY  44 

    0x72db9f8a,// 48 PAY  45 

    0x2c494c8f,// 49 PAY  46 

    0xfbe304fc,// 50 PAY  47 

    0x85317f98,// 51 PAY  48 

    0xfa2c4e67,// 52 PAY  49 

    0x21ca5177,// 53 PAY  50 

    0xaa726d43,// 54 PAY  51 

    0x639e6d65,// 55 PAY  52 

    0xd5b760f9,// 56 PAY  53 

    0xd37d5dff,// 57 PAY  54 

    0xbb158f3e,// 58 PAY  55 

    0xd0382aec,// 59 PAY  56 

    0xb1b8c849,// 60 PAY  57 

    0x1a079cd2,// 61 PAY  58 

    0x41cd4c3f,// 62 PAY  59 

    0xd1b9b46b,// 63 PAY  60 

    0x119dd9cc,// 64 PAY  61 

    0x987f7da7,// 65 PAY  62 

    0x0dcb1ba2,// 66 PAY  63 

    0xec9d09a0,// 67 PAY  64 

    0x13f9542a,// 68 PAY  65 

    0xbb8a2142,// 69 PAY  66 

    0x41fd4869,// 70 PAY  67 

    0x3cd80b50,// 71 PAY  68 

    0x4da2d21b,// 72 PAY  69 

    0x4ec0d0d5,// 73 PAY  70 

    0x0ae0ac18,// 74 PAY  71 

    0x3eba54c0,// 75 PAY  72 

    0x14fc3c6b,// 76 PAY  73 

    0x374a13fd,// 77 PAY  74 

    0x6b329a38,// 78 PAY  75 

    0x635c095a,// 79 PAY  76 

    0x7532017b,// 80 PAY  77 

    0x595d3b46,// 81 PAY  78 

    0x91ea619a,// 82 PAY  79 

    0x283b2d84,// 83 PAY  80 

    0x9fd4c7e0,// 84 PAY  81 

    0x1521d102,// 85 PAY  82 

    0x39d93160,// 86 PAY  83 

    0x419a0297,// 87 PAY  84 

    0xddecb87b,// 88 PAY  85 

    0xa18340a1,// 89 PAY  86 

    0x465c27ad,// 90 PAY  87 

    0x28df2ab2,// 91 PAY  88 

    0x7ecca408,// 92 PAY  89 

    0x956f428c,// 93 PAY  90 

    0xd883479e,// 94 PAY  91 

    0x71753589,// 95 PAY  92 

    0x9e3d3776,// 96 PAY  93 

    0x8a0e7cb0,// 97 PAY  94 

    0xe53b12cc,// 98 PAY  95 

    0x6ff5e1bb,// 99 PAY  96 

    0xd635bd88,// 100 PAY  97 

    0xde364f54,// 101 PAY  98 

    0xd585270d,// 102 PAY  99 

    0x4dc70fb2,// 103 PAY 100 

    0x95dd266e,// 104 PAY 101 

    0xf1abb4ee,// 105 PAY 102 

    0xd800caad,// 106 PAY 103 

    0xb61a45be,// 107 PAY 104 

    0x522599a7,// 108 PAY 105 

    0x3398196e,// 109 PAY 106 

    0xd03afd7f,// 110 PAY 107 

    0x23b6589a,// 111 PAY 108 

    0x9b376c43,// 112 PAY 109 

    0x0a784306,// 113 PAY 110 

    0x1ea97b73,// 114 PAY 111 

    0x9cb29699,// 115 PAY 112 

    0x2abb6500,// 116 PAY 113 

    0x2627ef20,// 117 PAY 114 

    0x39f97703,// 118 PAY 115 

    0xfbee7fcf,// 119 PAY 116 

    0xa321978d,// 120 PAY 117 

    0x82224399,// 121 PAY 118 

    0xd6776453,// 122 PAY 119 

    0x9cf96546,// 123 PAY 120 

    0x34cb3f38,// 124 PAY 121 

    0xce2595c8,// 125 PAY 122 

    0x77a55970,// 126 PAY 123 

    0x3ce9f722,// 127 PAY 124 

    0x668fbab2,// 128 PAY 125 

    0x3149e77a,// 129 PAY 126 

    0x46d4c4c1,// 130 PAY 127 

    0x34415e85,// 131 PAY 128 

    0x064805df,// 132 PAY 129 

    0x9b12b109,// 133 PAY 130 

    0x823c33de,// 134 PAY 131 

    0xf5864af8,// 135 PAY 132 

    0xf8e4a4fa,// 136 PAY 133 

    0xbcc1a968,// 137 PAY 134 

    0x949c8b8f,// 138 PAY 135 

    0x84438a6b,// 139 PAY 136 

    0x4933a95d,// 140 PAY 137 

    0x84f735ab,// 141 PAY 138 

    0xfb52e4ed,// 142 PAY 139 

    0x1f03ecb9,// 143 PAY 140 

    0xdad4ce1d,// 144 PAY 141 

    0xb557f6ff,// 145 PAY 142 

    0xf7d3dbf8,// 146 PAY 143 

    0xdc80c6c7,// 147 PAY 144 

    0x883fe9e5,// 148 PAY 145 

    0xea181420,// 149 PAY 146 

    0x5f280d6e,// 150 PAY 147 

    0x806c29f4,// 151 PAY 148 

    0xd6716c97,// 152 PAY 149 

    0xd8b9c904,// 153 PAY 150 

    0x3cfdb2c6,// 154 PAY 151 

    0x3235d882,// 155 PAY 152 

    0xf1fb4399,// 156 PAY 153 

    0x54377589,// 157 PAY 154 

    0xfb0a0c29,// 158 PAY 155 

    0x9c47a9a9,// 159 PAY 156 

    0x88015daa,// 160 PAY 157 

    0xa5653269,// 161 PAY 158 

    0x121fc04b,// 162 PAY 159 

    0x6afc289e,// 163 PAY 160 

    0x327390fd,// 164 PAY 161 

    0x715ecb16,// 165 PAY 162 

    0x247d789c,// 166 PAY 163 

    0x96078be9,// 167 PAY 164 

    0x3b0939d9,// 168 PAY 165 

    0x4c6c0302,// 169 PAY 166 

    0xfc82295f,// 170 PAY 167 

    0xc49fef2d,// 171 PAY 168 

    0x6b358a94,// 172 PAY 169 

    0xf163f3c4,// 173 PAY 170 

    0xf1b71cfa,// 174 PAY 171 

    0x812f1c7c,// 175 PAY 172 

    0xaafbd9c6,// 176 PAY 173 

    0x0e22c53d,// 177 PAY 174 

    0x552635a1,// 178 PAY 175 

    0x2ebd98d6,// 179 PAY 176 

    0xa0dc7c26,// 180 PAY 177 

    0xad3af7a3,// 181 PAY 178 

    0x1d398ab0,// 182 PAY 179 

    0x7f0aa8d3,// 183 PAY 180 

    0x8cb2d444,// 184 PAY 181 

    0x56e61478,// 185 PAY 182 

    0xcb9ff1b4,// 186 PAY 183 

    0xe491a170,// 187 PAY 184 

    0xa484d6a1,// 188 PAY 185 

    0xbfb8e9f1,// 189 PAY 186 

    0xefe45ed7,// 190 PAY 187 

    0xe96606e5,// 191 PAY 188 

    0x1c2bcb80,// 192 PAY 189 

    0x77299da6,// 193 PAY 190 

    0x8b955f28,// 194 PAY 191 

    0x659bca21,// 195 PAY 192 

    0x740af9fc,// 196 PAY 193 

    0x624df3cb,// 197 PAY 194 

    0xb9f4e3cd,// 198 PAY 195 

    0xbbdbb2c8,// 199 PAY 196 

    0x0166c7a9,// 200 PAY 197 

    0x05fc3819,// 201 PAY 198 

    0x6ee26c60,// 202 PAY 199 

    0x6c0fe291,// 203 PAY 200 

    0x36d618ce,// 204 PAY 201 

    0xd30cb319,// 205 PAY 202 

    0xbeabe9b4,// 206 PAY 203 

    0x85972563,// 207 PAY 204 

    0x8b8176ba,// 208 PAY 205 

    0x21bb0df0,// 209 PAY 206 

    0xea141529,// 210 PAY 207 

    0x42f7284a,// 211 PAY 208 

    0x3ebcf7fd,// 212 PAY 209 

    0xe00bcbe2,// 213 PAY 210 

    0x302aea4b,// 214 PAY 211 

    0x4668000b,// 215 PAY 212 

    0xc1bbdad3,// 216 PAY 213 

    0xa01763a4,// 217 PAY 214 

    0x17cb745f,// 218 PAY 215 

    0x4599fefe,// 219 PAY 216 

    0xca472bc0,// 220 PAY 217 

    0x0df09fd4,// 221 PAY 218 

    0x9a20e16c,// 222 PAY 219 

    0xba5b95e1,// 223 PAY 220 

    0x38bb8733,// 224 PAY 221 

    0x5311f613,// 225 PAY 222 

    0x0723b45e,// 226 PAY 223 

    0x1814788d,// 227 PAY 224 

    0xb2ec8525,// 228 PAY 225 

    0xb0759549,// 229 PAY 226 

    0xc2f6a0d8,// 230 PAY 227 

    0x4e083d3e,// 231 PAY 228 

    0x26919160,// 232 PAY 229 

    0x750811d2,// 233 PAY 230 

    0x53b9c83f,// 234 PAY 231 

    0x742b1718,// 235 PAY 232 

    0xe38bef83,// 236 PAY 233 

    0x3287fe1f,// 237 PAY 234 

    0x681270bf,// 238 PAY 235 

    0x994d5a1a,// 239 PAY 236 

    0xfc258e25,// 240 PAY 237 

    0x48ba298a,// 241 PAY 238 

    0x63374049,// 242 PAY 239 

    0xa529e83c,// 243 PAY 240 

    0x381b1d05,// 244 PAY 241 

    0x5fe36c0d,// 245 PAY 242 

    0x8f8691fb,// 246 PAY 243 

    0x0b392e2a,// 247 PAY 244 

    0x136b45a1,// 248 PAY 245 

    0xec65e1a6,// 249 PAY 246 

    0xf587bce4,// 250 PAY 247 

    0x57215e56,// 251 PAY 248 

    0x90e577a6,// 252 PAY 249 

    0x7d53e58b,// 253 PAY 250 

    0x9e44ea38,// 254 PAY 251 

    0x4163ab41,// 255 PAY 252 

    0xd39507fe,// 256 PAY 253 

    0xb167b545,// 257 PAY 254 

    0x1eb7822f,// 258 PAY 255 

    0x08613286,// 259 PAY 256 

    0x62dbb3e2,// 260 PAY 257 

    0x9895f31b,// 261 PAY 258 

    0x5bff0f6a,// 262 PAY 259 

    0x3237822e,// 263 PAY 260 

    0xf6ad8234,// 264 PAY 261 

    0xf9390c0e,// 265 PAY 262 

    0x2e5929c7,// 266 PAY 263 

    0x0b132c26,// 267 PAY 264 

    0x15fa5976,// 268 PAY 265 

    0x5752610a,// 269 PAY 266 

    0xd2cd57fe,// 270 PAY 267 

    0xb738e330,// 271 PAY 268 

    0xe3f1c510,// 272 PAY 269 

    0xb09b8c53,// 273 PAY 270 

    0x6989f0c2,// 274 PAY 271 

    0x42ed2841,// 275 PAY 272 

    0x9513d725,// 276 PAY 273 

    0x8f079ebb,// 277 PAY 274 

    0x70c18484,// 278 PAY 275 

    0xe3a4f221,// 279 PAY 276 

    0xcc7b35fd,// 280 PAY 277 

    0xdfa4be44,// 281 PAY 278 

    0xf2e2ef8b,// 282 PAY 279 

    0xd01a7da5,// 283 PAY 280 

    0x9f863055,// 284 PAY 281 

    0x27974d79,// 285 PAY 282 

    0xe39d03ba,// 286 PAY 283 

    0xadd08424,// 287 PAY 284 

    0x3eb74235,// 288 PAY 285 

    0x94931db9,// 289 PAY 286 

    0xebe1fb9c,// 290 PAY 287 

    0x51564a33,// 291 PAY 288 

    0x4f1e8f10,// 292 PAY 289 

    0x9adb6729,// 293 PAY 290 

    0x2c1855a0,// 294 PAY 291 

    0x4dd16a2f,// 295 PAY 292 

    0xc0cf4c01,// 296 PAY 293 

    0x6402f952,// 297 PAY 294 

    0x2d340626,// 298 PAY 295 

    0x2be55ce9,// 299 PAY 296 

    0x1d5fc871,// 300 PAY 297 

    0x6726291a,// 301 PAY 298 

    0x35d575db,// 302 PAY 299 

    0xf21c329e,// 303 PAY 300 

    0x9eef0a64,// 304 PAY 301 

    0x97ad4b69,// 305 PAY 302 

    0xa1b98fad,// 306 PAY 303 

    0xf5df7cbf,// 307 PAY 304 

    0x5db329b8,// 308 PAY 305 

    0x8a747a86,// 309 PAY 306 

    0xb89bec9c,// 310 PAY 307 

    0x8a8bb98c,// 311 PAY 308 

    0xc125cfff,// 312 PAY 309 

    0x294a8136,// 313 PAY 310 

    0x7acdc088,// 314 PAY 311 

    0x8b9661e9,// 315 PAY 312 

    0x156417a2,// 316 PAY 313 

    0x40d0cd7c,// 317 PAY 314 

    0x8189e0fb,// 318 PAY 315 

    0xb1899ef9,// 319 PAY 316 

    0xa5932e72,// 320 PAY 317 

    0xe7752aff,// 321 PAY 318 

    0x25c40b05,// 322 PAY 319 

    0xa63ea391,// 323 PAY 320 

    0x1b3c79a0,// 324 PAY 321 

    0x932e3725,// 325 PAY 322 

    0x8685bf70,// 326 PAY 323 

    0x3731ebad,// 327 PAY 324 

    0x2936c00d,// 328 PAY 325 

    0x84b0093c,// 329 PAY 326 

    0xc8b1bc37,// 330 PAY 327 

    0x110f6eab,// 331 PAY 328 

    0x4d412888,// 332 PAY 329 

    0x5975b42f,// 333 PAY 330 

    0x1cdee017,// 334 PAY 331 

    0x1a5dcb44,// 335 PAY 332 

    0x2bd76c9c,// 336 PAY 333 

    0x495bdf76,// 337 PAY 334 

    0x0b149227,// 338 PAY 335 

    0xae77aa36,// 339 PAY 336 

    0x882c0131,// 340 PAY 337 

    0x126a121c,// 341 PAY 338 

    0xa70b694e,// 342 PAY 339 

    0x6301bca2,// 343 PAY 340 

    0xf6f9a14f,// 344 PAY 341 

    0x2e0ab08b,// 345 PAY 342 

    0x1b641bb8,// 346 PAY 343 

    0x01fbc37a,// 347 PAY 344 

    0xb33d3c71,// 348 PAY 345 

    0x766d7078,// 349 PAY 346 

    0xce209f72,// 350 PAY 347 

    0x147a4422,// 351 PAY 348 

    0xd1b39622,// 352 PAY 349 

    0xc2997742,// 353 PAY 350 

    0x7deaeaea,// 354 PAY 351 

    0x4fbdcad0,// 355 PAY 352 

    0xf24f8dd2,// 356 PAY 353 

    0x5f573eda,// 357 PAY 354 

    0x24c41e45,// 358 PAY 355 

    0x3343fdd1,// 359 PAY 356 

    0x43216072,// 360 PAY 357 

    0x7473ffa2,// 361 PAY 358 

    0x02f778f5,// 362 PAY 359 

    0x39668b08,// 363 PAY 360 

    0x129c82de,// 364 PAY 361 

    0x429d2373,// 365 PAY 362 

    0xd9b2a84d,// 366 PAY 363 

    0x30d3147d,// 367 PAY 364 

    0x3b4fbba4,// 368 PAY 365 

    0x08df8703,// 369 PAY 366 

    0xd41f69ff,// 370 PAY 367 

    0xe851f8f9,// 371 PAY 368 

    0x6324a3a6,// 372 PAY 369 

    0x10f4a04b,// 373 PAY 370 

    0xd362e932,// 374 PAY 371 

    0x3e701a4c,// 375 PAY 372 

    0x2e054279,// 376 PAY 373 

    0xbde66965,// 377 PAY 374 

    0xfe189f92,// 378 PAY 375 

    0x10374f3e,// 379 PAY 376 

    0xd37c0295,// 380 PAY 377 

    0xe8cde467,// 381 PAY 378 

    0xb2f16f0b,// 382 PAY 379 

    0x5c472bbd,// 383 PAY 380 

    0x75dab419,// 384 PAY 381 

    0x1f6be4db,// 385 PAY 382 

    0x4c4d435e,// 386 PAY 383 

    0xce797b2a,// 387 PAY 384 

    0xcf0835b6,// 388 PAY 385 

    0x408b4db0,// 389 PAY 386 

    0x65476bc0,// 390 PAY 387 

    0xca9617d4,// 391 PAY 388 

    0xb2fdf489,// 392 PAY 389 

    0x974b4e70,// 393 PAY 390 

    0x9f9871f6,// 394 PAY 391 

    0xe530fcf2,// 395 PAY 392 

    0x2b6d3af8,// 396 PAY 393 

    0x71615d53,// 397 PAY 394 

    0x3ebddce5,// 398 PAY 395 

    0x3db1272f,// 399 PAY 396 

    0x980e882a,// 400 PAY 397 

    0xce3e0a55,// 401 PAY 398 

    0xbbb8c9ab,// 402 PAY 399 

    0xe753984a,// 403 PAY 400 

    0x44caae51,// 404 PAY 401 

    0x1ce02ffd,// 405 PAY 402 

    0x276402d3,// 406 PAY 403 

    0x5c7602f6,// 407 PAY 404 

    0x4cac57a9,// 408 PAY 405 

    0x76e956d8,// 409 PAY 406 

    0xcebe5135,// 410 PAY 407 

    0xe6bc8ad9,// 411 PAY 408 

    0xddecfb90,// 412 PAY 409 

    0x4e277179,// 413 PAY 410 

    0x4eeb03e6,// 414 PAY 411 

    0xbebc5caa,// 415 PAY 412 

    0x2de4b81c,// 416 PAY 413 

    0xc04315c5,// 417 PAY 414 

    0x05354e84,// 418 PAY 415 

    0x4aaca03b,// 419 PAY 416 

    0xdda241af,// 420 PAY 417 

    0x51a8705c,// 421 PAY 418 

    0x2ffecc7b,// 422 PAY 419 

    0xf6fdcda1,// 423 PAY 420 

    0x7fe34360,// 424 PAY 421 

    0x056dda1b,// 425 PAY 422 

    0x716e3c4a,// 426 PAY 423 

    0x9bf03720,// 427 PAY 424 

    0xea017123,// 428 PAY 425 

    0x03f2040e,// 429 PAY 426 

    0x3783f20e,// 430 PAY 427 

    0x93660000,// 431 PAY 428 

/// STA is 1 words. 

/// STA num_pkts       : 13 

/// STA pkt_idx        : 80 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9a 

    0x01419a0d // 432 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt45_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 54 words. 

/// BDA size     is 210 (0xd2) 

/// BDA id       is 0xae4b 

    0x00d2ae4b,// 3 BDA   1 

/// PAY Generic Data size   : 210 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x3d04f01d,// 4 PAY   1 

    0x41fe8e08,// 5 PAY   2 

    0xf0c4119f,// 6 PAY   3 

    0x239e2323,// 7 PAY   4 

    0xe75e58fb,// 8 PAY   5 

    0x557d4b75,// 9 PAY   6 

    0xe9a1251a,// 10 PAY   7 

    0x3c9dfee9,// 11 PAY   8 

    0x5342a492,// 12 PAY   9 

    0x0104b23a,// 13 PAY  10 

    0x44ac4a07,// 14 PAY  11 

    0xe0aa4f21,// 15 PAY  12 

    0xaff1f509,// 16 PAY  13 

    0x7ef1f196,// 17 PAY  14 

    0x28c82972,// 18 PAY  15 

    0xa6254e3d,// 19 PAY  16 

    0x8a4ae460,// 20 PAY  17 

    0x280f207f,// 21 PAY  18 

    0x4ca83b89,// 22 PAY  19 

    0xcc29b95e,// 23 PAY  20 

    0x5d301794,// 24 PAY  21 

    0xcb84d549,// 25 PAY  22 

    0x9857c528,// 26 PAY  23 

    0x53d131dd,// 27 PAY  24 

    0x7d83ec2f,// 28 PAY  25 

    0xb2385766,// 29 PAY  26 

    0xf302e666,// 30 PAY  27 

    0xda49731d,// 31 PAY  28 

    0xc72e9786,// 32 PAY  29 

    0x4f920ad6,// 33 PAY  30 

    0x6e656425,// 34 PAY  31 

    0x91a8527b,// 35 PAY  32 

    0x1a47a25d,// 36 PAY  33 

    0x76079076,// 37 PAY  34 

    0x55855a23,// 38 PAY  35 

    0xb650730f,// 39 PAY  36 

    0xb71e8d1e,// 40 PAY  37 

    0x96f0dec4,// 41 PAY  38 

    0x6be294fd,// 42 PAY  39 

    0x7b39a2bf,// 43 PAY  40 

    0x5794de0d,// 44 PAY  41 

    0x45adb115,// 45 PAY  42 

    0x2b01481d,// 46 PAY  43 

    0x3b9aca3c,// 47 PAY  44 

    0x10147afd,// 48 PAY  45 

    0x9a1f7dad,// 49 PAY  46 

    0x84cc36ea,// 50 PAY  47 

    0xa08481b9,// 51 PAY  48 

    0x9d731b74,// 52 PAY  49 

    0xc21f2675,// 53 PAY  50 

    0xe5460c2e,// 54 PAY  51 

    0x61e9c16e,// 55 PAY  52 

    0x517c0000,// 56 PAY  53 

/// STA is 1 words. 

/// STA num_pkts       : 173 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3f 

    0x02cd3fad // 57 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt46_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 237 words. 

/// BDA size     is 941 (0x3ad) 

/// BDA id       is 0x3c4b 

    0x03ad3c4b,// 3 BDA   1 

/// PAY Generic Data size   : 941 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0b32d726,// 4 PAY   1 

    0x21cf0bc4,// 5 PAY   2 

    0x74146097,// 6 PAY   3 

    0x8440b6e5,// 7 PAY   4 

    0xa3685952,// 8 PAY   5 

    0x32f8d55b,// 9 PAY   6 

    0x5e824666,// 10 PAY   7 

    0x70451715,// 11 PAY   8 

    0x130952f9,// 12 PAY   9 

    0x4d6ff172,// 13 PAY  10 

    0x89362f4d,// 14 PAY  11 

    0x79434b51,// 15 PAY  12 

    0x88d768b4,// 16 PAY  13 

    0xeb9b8a45,// 17 PAY  14 

    0xe948ec2a,// 18 PAY  15 

    0x740092d5,// 19 PAY  16 

    0xaca97ca3,// 20 PAY  17 

    0xb642fcf8,// 21 PAY  18 

    0x352a432c,// 22 PAY  19 

    0x1e37b583,// 23 PAY  20 

    0x58f217f3,// 24 PAY  21 

    0x5a000ded,// 25 PAY  22 

    0x7f2eb71a,// 26 PAY  23 

    0x40205c39,// 27 PAY  24 

    0x3370ed06,// 28 PAY  25 

    0x343dd374,// 29 PAY  26 

    0x19013cf8,// 30 PAY  27 

    0xdd80350f,// 31 PAY  28 

    0x900c321f,// 32 PAY  29 

    0xb5c2ee83,// 33 PAY  30 

    0xbb942685,// 34 PAY  31 

    0xebdc31d7,// 35 PAY  32 

    0xf6c78c2d,// 36 PAY  33 

    0x5b2a62f1,// 37 PAY  34 

    0xa8436ba8,// 38 PAY  35 

    0x297a2ed4,// 39 PAY  36 

    0x90604f38,// 40 PAY  37 

    0x675b8795,// 41 PAY  38 

    0xa9fa46b9,// 42 PAY  39 

    0xc08ba83a,// 43 PAY  40 

    0x873ed9ac,// 44 PAY  41 

    0xf3d3400d,// 45 PAY  42 

    0x34ec2a05,// 46 PAY  43 

    0x9c1c022d,// 47 PAY  44 

    0x0f89f665,// 48 PAY  45 

    0x03909b2d,// 49 PAY  46 

    0x98eccda1,// 50 PAY  47 

    0x0488f22e,// 51 PAY  48 

    0x50cd19a7,// 52 PAY  49 

    0xdd6bc988,// 53 PAY  50 

    0xa04fd46d,// 54 PAY  51 

    0xb5d57b6a,// 55 PAY  52 

    0x0882a1c6,// 56 PAY  53 

    0x4a841c24,// 57 PAY  54 

    0x4e061e84,// 58 PAY  55 

    0x5345efad,// 59 PAY  56 

    0xf28dc8b2,// 60 PAY  57 

    0x0defc3af,// 61 PAY  58 

    0xfeef0f36,// 62 PAY  59 

    0xb879f727,// 63 PAY  60 

    0x5e366325,// 64 PAY  61 

    0xb9c03986,// 65 PAY  62 

    0x845ad01f,// 66 PAY  63 

    0x43f88a5a,// 67 PAY  64 

    0xc8327895,// 68 PAY  65 

    0x2ead862b,// 69 PAY  66 

    0x075d4fb6,// 70 PAY  67 

    0x941df505,// 71 PAY  68 

    0x4872742e,// 72 PAY  69 

    0x8f9fd30d,// 73 PAY  70 

    0x52a73dfa,// 74 PAY  71 

    0x95a65b19,// 75 PAY  72 

    0x18d2632d,// 76 PAY  73 

    0x6b7f1867,// 77 PAY  74 

    0xae3462e0,// 78 PAY  75 

    0xeb605483,// 79 PAY  76 

    0x2774723f,// 80 PAY  77 

    0x007a4a29,// 81 PAY  78 

    0x1b7db18e,// 82 PAY  79 

    0x3a0d0e18,// 83 PAY  80 

    0x628c15ef,// 84 PAY  81 

    0xb77cb6f8,// 85 PAY  82 

    0x50f11a49,// 86 PAY  83 

    0x4bb11028,// 87 PAY  84 

    0x5c1b3b55,// 88 PAY  85 

    0xd23f7776,// 89 PAY  86 

    0x3c81a94a,// 90 PAY  87 

    0x418a0510,// 91 PAY  88 

    0x87d8791f,// 92 PAY  89 

    0xd616cc05,// 93 PAY  90 

    0x1808fe2c,// 94 PAY  91 

    0xf1699a95,// 95 PAY  92 

    0x5c311b67,// 96 PAY  93 

    0x283ff15c,// 97 PAY  94 

    0x78fe92eb,// 98 PAY  95 

    0xeb707480,// 99 PAY  96 

    0xf49d8037,// 100 PAY  97 

    0x0a37ae2b,// 101 PAY  98 

    0x9984010c,// 102 PAY  99 

    0x8966288f,// 103 PAY 100 

    0x7b31dec5,// 104 PAY 101 

    0xb5ab7054,// 105 PAY 102 

    0x2d2029d0,// 106 PAY 103 

    0x68811734,// 107 PAY 104 

    0xf7c1faa0,// 108 PAY 105 

    0x8535ca93,// 109 PAY 106 

    0x45e1fff6,// 110 PAY 107 

    0x91a2e4c7,// 111 PAY 108 

    0x8df81f44,// 112 PAY 109 

    0x77095e93,// 113 PAY 110 

    0x8c9cf810,// 114 PAY 111 

    0x7fef9a6a,// 115 PAY 112 

    0x0ac29fe0,// 116 PAY 113 

    0xc207d7b3,// 117 PAY 114 

    0x5a65fd63,// 118 PAY 115 

    0x01dfc0b3,// 119 PAY 116 

    0x2da36096,// 120 PAY 117 

    0x45451948,// 121 PAY 118 

    0xdbda7b71,// 122 PAY 119 

    0x6c4732a3,// 123 PAY 120 

    0xeb977f0f,// 124 PAY 121 

    0x46f6fb6e,// 125 PAY 122 

    0x53e7eb42,// 126 PAY 123 

    0x6bc1de8a,// 127 PAY 124 

    0xe5970553,// 128 PAY 125 

    0x145341f6,// 129 PAY 126 

    0x09b804ce,// 130 PAY 127 

    0xa21c1906,// 131 PAY 128 

    0x8d00c917,// 132 PAY 129 

    0xccc73d29,// 133 PAY 130 

    0x189491bd,// 134 PAY 131 

    0x823df58b,// 135 PAY 132 

    0x968bce97,// 136 PAY 133 

    0x8ab3a8d7,// 137 PAY 134 

    0x7d05b9c0,// 138 PAY 135 

    0xa016a6a5,// 139 PAY 136 

    0xfd6393f0,// 140 PAY 137 

    0x057dd846,// 141 PAY 138 

    0x882ec926,// 142 PAY 139 

    0xadb510ca,// 143 PAY 140 

    0x82ca4906,// 144 PAY 141 

    0x1f2868c7,// 145 PAY 142 

    0x5ae25441,// 146 PAY 143 

    0xc9691c64,// 147 PAY 144 

    0x9dc87d99,// 148 PAY 145 

    0xa73b30b3,// 149 PAY 146 

    0x79ccdbf7,// 150 PAY 147 

    0x7a7c6c6f,// 151 PAY 148 

    0x3bf14809,// 152 PAY 149 

    0xdcbc4bca,// 153 PAY 150 

    0x84ffbf5d,// 154 PAY 151 

    0x1685c3ba,// 155 PAY 152 

    0xa7849ce4,// 156 PAY 153 

    0xda13a42b,// 157 PAY 154 

    0x7eacf7ae,// 158 PAY 155 

    0xa91fbb7f,// 159 PAY 156 

    0x6d1cfa1c,// 160 PAY 157 

    0xb82e7479,// 161 PAY 158 

    0x88b52e2c,// 162 PAY 159 

    0x688dcba3,// 163 PAY 160 

    0x3a85a44a,// 164 PAY 161 

    0x2f73cd21,// 165 PAY 162 

    0x78968634,// 166 PAY 163 

    0x9007c4c3,// 167 PAY 164 

    0xe7b20fd8,// 168 PAY 165 

    0x5f98615a,// 169 PAY 166 

    0x65c1dc7e,// 170 PAY 167 

    0x12f6a6e3,// 171 PAY 168 

    0xc61864a2,// 172 PAY 169 

    0xf546fe77,// 173 PAY 170 

    0x9dca8641,// 174 PAY 171 

    0xe2e2bbb6,// 175 PAY 172 

    0x5b9c1b57,// 176 PAY 173 

    0x3d7dd4c2,// 177 PAY 174 

    0x4ac89747,// 178 PAY 175 

    0xb2eff5a7,// 179 PAY 176 

    0x371c3ef7,// 180 PAY 177 

    0x74258f9f,// 181 PAY 178 

    0x3dd47687,// 182 PAY 179 

    0xaee41bdb,// 183 PAY 180 

    0x27fe0601,// 184 PAY 181 

    0x4166adec,// 185 PAY 182 

    0x45b46a39,// 186 PAY 183 

    0x73c641df,// 187 PAY 184 

    0x9d10f226,// 188 PAY 185 

    0x8e6e7848,// 189 PAY 186 

    0x93b18242,// 190 PAY 187 

    0xc4b3b171,// 191 PAY 188 

    0xfd2238ee,// 192 PAY 189 

    0x964f8384,// 193 PAY 190 

    0x79e7d65f,// 194 PAY 191 

    0x0cf147c8,// 195 PAY 192 

    0x96ef5e41,// 196 PAY 193 

    0xfd91ea4c,// 197 PAY 194 

    0xb478f392,// 198 PAY 195 

    0x4d2fb63f,// 199 PAY 196 

    0xe9cefdca,// 200 PAY 197 

    0x432e0f32,// 201 PAY 198 

    0x64391143,// 202 PAY 199 

    0xc356aae9,// 203 PAY 200 

    0x94a61d34,// 204 PAY 201 

    0x47e00b1a,// 205 PAY 202 

    0x9a9c5378,// 206 PAY 203 

    0x1d9ef59f,// 207 PAY 204 

    0x7fe09042,// 208 PAY 205 

    0xa27bf0e0,// 209 PAY 206 

    0x737f62d6,// 210 PAY 207 

    0xd57f24c6,// 211 PAY 208 

    0x930f726c,// 212 PAY 209 

    0x6bf4c858,// 213 PAY 210 

    0x6970e199,// 214 PAY 211 

    0x2a2c6f31,// 215 PAY 212 

    0xa3cfa9f2,// 216 PAY 213 

    0x831b1c1b,// 217 PAY 214 

    0x8a89d059,// 218 PAY 215 

    0xbf358b26,// 219 PAY 216 

    0x9a02d987,// 220 PAY 217 

    0xe778af4a,// 221 PAY 218 

    0xe6941db1,// 222 PAY 219 

    0x7da6f1c4,// 223 PAY 220 

    0xdc01e581,// 224 PAY 221 

    0xf3917d5a,// 225 PAY 222 

    0x4e265af0,// 226 PAY 223 

    0x36f5ede7,// 227 PAY 224 

    0xd4d8f79e,// 228 PAY 225 

    0x34e2a606,// 229 PAY 226 

    0x624a3b9e,// 230 PAY 227 

    0xcd3d0eae,// 231 PAY 228 

    0x4c07fd01,// 232 PAY 229 

    0x914e6c0e,// 233 PAY 230 

    0x65c26de7,// 234 PAY 231 

    0xe50016af,// 235 PAY 232 

    0xdface428,// 236 PAY 233 

    0xd3277651,// 237 PAY 234 

    0x483415dc,// 238 PAY 235 

    0x13000000,// 239 PAY 236 

/// STA is 1 words. 

/// STA num_pkts       : 100 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6f 

    0x02616f64 // 240 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt47_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 359 words. 

/// BDA size     is 1432 (0x598) 

/// BDA id       is 0x4ac8 

    0x05984ac8,// 3 BDA   1 

/// PAY Generic Data size   : 1432 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xba0a1b1f,// 4 PAY   1 

    0xa2b5b1e8,// 5 PAY   2 

    0x452e3a3a,// 6 PAY   3 

    0x20fa34b5,// 7 PAY   4 

    0x67be082e,// 8 PAY   5 

    0x87c530e4,// 9 PAY   6 

    0x7302ec7c,// 10 PAY   7 

    0xbc79ad67,// 11 PAY   8 

    0x7c097852,// 12 PAY   9 

    0xa608a0ca,// 13 PAY  10 

    0xf2543939,// 14 PAY  11 

    0x71e42b03,// 15 PAY  12 

    0xf6b82f2a,// 16 PAY  13 

    0x75651669,// 17 PAY  14 

    0x6dff3192,// 18 PAY  15 

    0xba146e65,// 19 PAY  16 

    0xa7bc81fa,// 20 PAY  17 

    0x999169fe,// 21 PAY  18 

    0xf86d28c9,// 22 PAY  19 

    0xb4de706a,// 23 PAY  20 

    0xbc6fe474,// 24 PAY  21 

    0xf2186d3f,// 25 PAY  22 

    0x184751af,// 26 PAY  23 

    0x87a50325,// 27 PAY  24 

    0x9396527d,// 28 PAY  25 

    0x20384bb7,// 29 PAY  26 

    0x758a955f,// 30 PAY  27 

    0x6f3a0678,// 31 PAY  28 

    0x843c0543,// 32 PAY  29 

    0xb4b4976f,// 33 PAY  30 

    0x1300db5f,// 34 PAY  31 

    0x8adea50e,// 35 PAY  32 

    0x35330ec4,// 36 PAY  33 

    0x252c05f0,// 37 PAY  34 

    0x5e7a3ae7,// 38 PAY  35 

    0x8ed78b73,// 39 PAY  36 

    0x35783b62,// 40 PAY  37 

    0x1532d9a1,// 41 PAY  38 

    0xd621620b,// 42 PAY  39 

    0x3512e33c,// 43 PAY  40 

    0xc31ec95f,// 44 PAY  41 

    0xc58e79a5,// 45 PAY  42 

    0x07d314cc,// 46 PAY  43 

    0xf0a4b6e9,// 47 PAY  44 

    0xd348ef19,// 48 PAY  45 

    0x154c4f45,// 49 PAY  46 

    0x241a08da,// 50 PAY  47 

    0x657c4707,// 51 PAY  48 

    0x7ff48c28,// 52 PAY  49 

    0xb8c1d978,// 53 PAY  50 

    0x1e313802,// 54 PAY  51 

    0x19792c7a,// 55 PAY  52 

    0xc4bf66e0,// 56 PAY  53 

    0x9a63390f,// 57 PAY  54 

    0xfc83d88d,// 58 PAY  55 

    0x7c8a6b21,// 59 PAY  56 

    0x8d0b929e,// 60 PAY  57 

    0xebcaa76c,// 61 PAY  58 

    0x1b9002f4,// 62 PAY  59 

    0xc0c2716b,// 63 PAY  60 

    0x93ed1e7d,// 64 PAY  61 

    0xfb5cece0,// 65 PAY  62 

    0xb855c227,// 66 PAY  63 

    0x1bfade2f,// 67 PAY  64 

    0x588dd5c7,// 68 PAY  65 

    0x14f43e88,// 69 PAY  66 

    0x26a87efc,// 70 PAY  67 

    0x47a768a8,// 71 PAY  68 

    0xb16b2e0b,// 72 PAY  69 

    0x85fd3bf2,// 73 PAY  70 

    0xad86df72,// 74 PAY  71 

    0xb9754d6b,// 75 PAY  72 

    0x2d39465e,// 76 PAY  73 

    0x69031601,// 77 PAY  74 

    0xe780a6fa,// 78 PAY  75 

    0xffb14adb,// 79 PAY  76 

    0x37b73ed4,// 80 PAY  77 

    0x88cdb3a5,// 81 PAY  78 

    0x1105974c,// 82 PAY  79 

    0x71b80376,// 83 PAY  80 

    0x3c2a7f0d,// 84 PAY  81 

    0x0741ff82,// 85 PAY  82 

    0x57bf746e,// 86 PAY  83 

    0x1ae91abf,// 87 PAY  84 

    0xf6d9962b,// 88 PAY  85 

    0xf4530633,// 89 PAY  86 

    0x250aae6b,// 90 PAY  87 

    0x30a43816,// 91 PAY  88 

    0x1e08d9ff,// 92 PAY  89 

    0xb4389376,// 93 PAY  90 

    0xa900c725,// 94 PAY  91 

    0xe902a5d6,// 95 PAY  92 

    0xba044446,// 96 PAY  93 

    0xad8c0ff7,// 97 PAY  94 

    0x2db40df7,// 98 PAY  95 

    0x2171e431,// 99 PAY  96 

    0x7b5fb96a,// 100 PAY  97 

    0x0a94724d,// 101 PAY  98 

    0x7d2c2aac,// 102 PAY  99 

    0xf4b999e1,// 103 PAY 100 

    0x8415e425,// 104 PAY 101 

    0x7347b2c8,// 105 PAY 102 

    0x6440c2f5,// 106 PAY 103 

    0x7ce57946,// 107 PAY 104 

    0xfbe70850,// 108 PAY 105 

    0x9ffb2cee,// 109 PAY 106 

    0xdb660e07,// 110 PAY 107 

    0x7245bd37,// 111 PAY 108 

    0x7da5a31d,// 112 PAY 109 

    0x31d9421d,// 113 PAY 110 

    0xd2adf2fd,// 114 PAY 111 

    0x2ca583e9,// 115 PAY 112 

    0xf21f34fe,// 116 PAY 113 

    0xed6db358,// 117 PAY 114 

    0xc266cfc1,// 118 PAY 115 

    0xa8c9a3ce,// 119 PAY 116 

    0xc9338271,// 120 PAY 117 

    0xf5ac9fec,// 121 PAY 118 

    0x050809b2,// 122 PAY 119 

    0x4ec47e52,// 123 PAY 120 

    0x7a1dcc48,// 124 PAY 121 

    0x23c028e6,// 125 PAY 122 

    0x04fc51d2,// 126 PAY 123 

    0xfaa03c58,// 127 PAY 124 

    0x4b068983,// 128 PAY 125 

    0xfe3b136e,// 129 PAY 126 

    0xd12556fc,// 130 PAY 127 

    0xd3e03137,// 131 PAY 128 

    0x3b658115,// 132 PAY 129 

    0x8e9d16b8,// 133 PAY 130 

    0x04e81e69,// 134 PAY 131 

    0xb0820219,// 135 PAY 132 

    0x396b4fbd,// 136 PAY 133 

    0x6baa3e73,// 137 PAY 134 

    0xafcbfac5,// 138 PAY 135 

    0x232c77f7,// 139 PAY 136 

    0x475cbc19,// 140 PAY 137 

    0xe89df8ee,// 141 PAY 138 

    0x272a6820,// 142 PAY 139 

    0x55c7e681,// 143 PAY 140 

    0x76a2f4d0,// 144 PAY 141 

    0x1bc14e5b,// 145 PAY 142 

    0xfabee65d,// 146 PAY 143 

    0x1d4fd39e,// 147 PAY 144 

    0x1228331a,// 148 PAY 145 

    0xb2cdc026,// 149 PAY 146 

    0xe07e5142,// 150 PAY 147 

    0x1361519c,// 151 PAY 148 

    0x3f572156,// 152 PAY 149 

    0x279afc36,// 153 PAY 150 

    0xc453117a,// 154 PAY 151 

    0x9f7bbdba,// 155 PAY 152 

    0xdc8ad25c,// 156 PAY 153 

    0x7d688183,// 157 PAY 154 

    0xe7660349,// 158 PAY 155 

    0x85434596,// 159 PAY 156 

    0x25c42d3c,// 160 PAY 157 

    0xa0f75c1c,// 161 PAY 158 

    0xb74e8619,// 162 PAY 159 

    0xd470183f,// 163 PAY 160 

    0x44b12bb7,// 164 PAY 161 

    0x1447508d,// 165 PAY 162 

    0x796b05eb,// 166 PAY 163 

    0x898cda80,// 167 PAY 164 

    0x18bc4030,// 168 PAY 165 

    0x69e51d20,// 169 PAY 166 

    0x4d597aea,// 170 PAY 167 

    0x32dfdff0,// 171 PAY 168 

    0x4c9c357f,// 172 PAY 169 

    0x175b8c5f,// 173 PAY 170 

    0x49c4bdec,// 174 PAY 171 

    0x73c12c22,// 175 PAY 172 

    0x87d8afd2,// 176 PAY 173 

    0x0cc37825,// 177 PAY 174 

    0xbcba6d08,// 178 PAY 175 

    0xed4815f2,// 179 PAY 176 

    0xffafc7ba,// 180 PAY 177 

    0x385d04d9,// 181 PAY 178 

    0x6c0bf8a1,// 182 PAY 179 

    0x326c1397,// 183 PAY 180 

    0xdd3ef823,// 184 PAY 181 

    0x964be5e7,// 185 PAY 182 

    0xf1ecda4e,// 186 PAY 183 

    0xf7a59f68,// 187 PAY 184 

    0x2120cd38,// 188 PAY 185 

    0xa8be8b0f,// 189 PAY 186 

    0xd7fe6476,// 190 PAY 187 

    0xc0dd664d,// 191 PAY 188 

    0x8ebcf7ab,// 192 PAY 189 

    0xeafec9d7,// 193 PAY 190 

    0xebd149f6,// 194 PAY 191 

    0xaa34a8ef,// 195 PAY 192 

    0x3686ed33,// 196 PAY 193 

    0xd6a2b890,// 197 PAY 194 

    0x89857fa0,// 198 PAY 195 

    0x8648caf0,// 199 PAY 196 

    0xef0ff8b2,// 200 PAY 197 

    0xe961a3d3,// 201 PAY 198 

    0x4888e893,// 202 PAY 199 

    0xf4272965,// 203 PAY 200 

    0xf7250e08,// 204 PAY 201 

    0xade4aaa4,// 205 PAY 202 

    0xb3e0f0ca,// 206 PAY 203 

    0xcb90f908,// 207 PAY 204 

    0xedeaa29b,// 208 PAY 205 

    0xd78fa992,// 209 PAY 206 

    0x7758d0d1,// 210 PAY 207 

    0x16024a96,// 211 PAY 208 

    0xbb519499,// 212 PAY 209 

    0x5db6ab48,// 213 PAY 210 

    0x28aad91a,// 214 PAY 211 

    0x2ed6df1f,// 215 PAY 212 

    0xb2e65132,// 216 PAY 213 

    0x436b96e4,// 217 PAY 214 

    0xa02a84bd,// 218 PAY 215 

    0x1c79f0f3,// 219 PAY 216 

    0xdae9aa38,// 220 PAY 217 

    0xbe251297,// 221 PAY 218 

    0x75b5f232,// 222 PAY 219 

    0xc85fc5e6,// 223 PAY 220 

    0x708e4bf5,// 224 PAY 221 

    0xb50997a0,// 225 PAY 222 

    0x54fac336,// 226 PAY 223 

    0xc95c423c,// 227 PAY 224 

    0xb91ef05b,// 228 PAY 225 

    0x44f99310,// 229 PAY 226 

    0x654b9a9a,// 230 PAY 227 

    0x6c527012,// 231 PAY 228 

    0x968416ee,// 232 PAY 229 

    0x126c236e,// 233 PAY 230 

    0x2d3b0a7b,// 234 PAY 231 

    0x90da20fc,// 235 PAY 232 

    0xb7c10c27,// 236 PAY 233 

    0x51e3462c,// 237 PAY 234 

    0x63dacada,// 238 PAY 235 

    0x1409883a,// 239 PAY 236 

    0x55353b89,// 240 PAY 237 

    0x588170e7,// 241 PAY 238 

    0xa42efb5c,// 242 PAY 239 

    0x698d4823,// 243 PAY 240 

    0xa02e5de6,// 244 PAY 241 

    0x8fb2e797,// 245 PAY 242 

    0x05973df7,// 246 PAY 243 

    0x86651f38,// 247 PAY 244 

    0x33c73096,// 248 PAY 245 

    0x515fbf78,// 249 PAY 246 

    0xca5c5cd8,// 250 PAY 247 

    0x9299a857,// 251 PAY 248 

    0x4a6a61d9,// 252 PAY 249 

    0x78977eae,// 253 PAY 250 

    0x8f485763,// 254 PAY 251 

    0x2b4d7a16,// 255 PAY 252 

    0x7d721c29,// 256 PAY 253 

    0xa7505e36,// 257 PAY 254 

    0xdba92987,// 258 PAY 255 

    0xe8a29aad,// 259 PAY 256 

    0xed9bc1ff,// 260 PAY 257 

    0xb9c6d742,// 261 PAY 258 

    0x1206646a,// 262 PAY 259 

    0x105d33cb,// 263 PAY 260 

    0xaf4725f3,// 264 PAY 261 

    0x4d1b748c,// 265 PAY 262 

    0xae55ebd8,// 266 PAY 263 

    0x9f5e646a,// 267 PAY 264 

    0x1974445a,// 268 PAY 265 

    0x66c906ab,// 269 PAY 266 

    0x7029a6cd,// 270 PAY 267 

    0xf6fb6cf8,// 271 PAY 268 

    0x39d28743,// 272 PAY 269 

    0xc1f3b39c,// 273 PAY 270 

    0x88fe148a,// 274 PAY 271 

    0xddc7fd8a,// 275 PAY 272 

    0x6d05b7f7,// 276 PAY 273 

    0x937f9044,// 277 PAY 274 

    0x353cbd00,// 278 PAY 275 

    0x63ee9b77,// 279 PAY 276 

    0xf7c90269,// 280 PAY 277 

    0x1ac26220,// 281 PAY 278 

    0x59946c96,// 282 PAY 279 

    0x15c41063,// 283 PAY 280 

    0x2f23eba6,// 284 PAY 281 

    0x508cdd0f,// 285 PAY 282 

    0x2b772136,// 286 PAY 283 

    0xd884cf54,// 287 PAY 284 

    0x74259c51,// 288 PAY 285 

    0x664c7652,// 289 PAY 286 

    0xfb6b0646,// 290 PAY 287 

    0x3ea43177,// 291 PAY 288 

    0x4c40e3bd,// 292 PAY 289 

    0x3206731f,// 293 PAY 290 

    0x357a9e26,// 294 PAY 291 

    0xb5d3efbd,// 295 PAY 292 

    0xfec7a89d,// 296 PAY 293 

    0x4c5a977a,// 297 PAY 294 

    0x205d43f4,// 298 PAY 295 

    0x288d7e32,// 299 PAY 296 

    0x83aa2e9d,// 300 PAY 297 

    0xbf839b25,// 301 PAY 298 

    0x4f1b3838,// 302 PAY 299 

    0x66425f08,// 303 PAY 300 

    0xfe9ebfc1,// 304 PAY 301 

    0xf3a6d118,// 305 PAY 302 

    0x50248bfa,// 306 PAY 303 

    0xa05b3da3,// 307 PAY 304 

    0x3c954ede,// 308 PAY 305 

    0xbd12a530,// 309 PAY 306 

    0xda248986,// 310 PAY 307 

    0x2e6bf613,// 311 PAY 308 

    0xf69b24de,// 312 PAY 309 

    0xab06361e,// 313 PAY 310 

    0xd05ebd59,// 314 PAY 311 

    0x102003a4,// 315 PAY 312 

    0xfe2e4507,// 316 PAY 313 

    0xe9857570,// 317 PAY 314 

    0x2a5d311d,// 318 PAY 315 

    0x8d49d09d,// 319 PAY 316 

    0x1a423a7d,// 320 PAY 317 

    0x83496c64,// 321 PAY 318 

    0x12f2f38c,// 322 PAY 319 

    0x5ec5b088,// 323 PAY 320 

    0xfdef2b8d,// 324 PAY 321 

    0x67e2420e,// 325 PAY 322 

    0x98b4d560,// 326 PAY 323 

    0xd1834e77,// 327 PAY 324 

    0x39e47e0a,// 328 PAY 325 

    0x989367ba,// 329 PAY 326 

    0x2a950cf9,// 330 PAY 327 

    0x89d2542f,// 331 PAY 328 

    0x8ed635c5,// 332 PAY 329 

    0xe8b28db2,// 333 PAY 330 

    0xbbf2057f,// 334 PAY 331 

    0x82570569,// 335 PAY 332 

    0xee1e3d1c,// 336 PAY 333 

    0x4a679990,// 337 PAY 334 

    0xa72c5e3d,// 338 PAY 335 

    0x6bade334,// 339 PAY 336 

    0xebb6e6e8,// 340 PAY 337 

    0xdb5e6875,// 341 PAY 338 

    0x240e8655,// 342 PAY 339 

    0xb2851039,// 343 PAY 340 

    0x09da18a6,// 344 PAY 341 

    0x6106bfa9,// 345 PAY 342 

    0xf389a22a,// 346 PAY 343 

    0xa0b9007f,// 347 PAY 344 

    0xdeb78de7,// 348 PAY 345 

    0x2f785c4c,// 349 PAY 346 

    0x4619fbe3,// 350 PAY 347 

    0xe0fa0126,// 351 PAY 348 

    0xc2b8b3e3,// 352 PAY 349 

    0x9b740904,// 353 PAY 350 

    0x992b44c5,// 354 PAY 351 

    0x4f580e08,// 355 PAY 352 

    0x70a3c504,// 356 PAY 353 

    0x3605489f,// 357 PAY 354 

    0xc72a4fe1,// 358 PAY 355 

    0x245961b2,// 359 PAY 356 

    0xa10b69dd,// 360 PAY 357 

    0x00cd1173,// 361 PAY 358 

/// HASH is  8 bytes 

    0x82570569,// 362 HSH   1 

    0xee1e3d1c,// 363 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 35 

/// STA pkt_idx        : 128 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xac 

    0x0200ac23 // 364 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt48_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 418 words. 

/// BDA size     is 1666 (0x682) 

/// BDA id       is 0x267c 

    0x0682267c,// 3 BDA   1 

/// PAY Generic Data size   : 1666 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x58eac9da,// 4 PAY   1 

    0x33f37558,// 5 PAY   2 

    0xd72ca5f6,// 6 PAY   3 

    0xe51376a8,// 7 PAY   4 

    0xf8cdf913,// 8 PAY   5 

    0x3e134360,// 9 PAY   6 

    0xc566105a,// 10 PAY   7 

    0x3406d6fa,// 11 PAY   8 

    0xfa4207d9,// 12 PAY   9 

    0xed92db5f,// 13 PAY  10 

    0x1dc87d3b,// 14 PAY  11 

    0x483e2650,// 15 PAY  12 

    0x1db5be4a,// 16 PAY  13 

    0x1ebbd980,// 17 PAY  14 

    0x4ff92b46,// 18 PAY  15 

    0x1d909ba3,// 19 PAY  16 

    0x253c3c17,// 20 PAY  17 

    0x59f825f4,// 21 PAY  18 

    0x828d4619,// 22 PAY  19 

    0x36c9959e,// 23 PAY  20 

    0xfe8d1594,// 24 PAY  21 

    0x9aef6a90,// 25 PAY  22 

    0xa4ab0822,// 26 PAY  23 

    0xf8c9696f,// 27 PAY  24 

    0xdef40e1d,// 28 PAY  25 

    0x40aa7293,// 29 PAY  26 

    0x5012140b,// 30 PAY  27 

    0x88664051,// 31 PAY  28 

    0xf4248802,// 32 PAY  29 

    0x81d9023e,// 33 PAY  30 

    0x990c3013,// 34 PAY  31 

    0x9d7eb2d9,// 35 PAY  32 

    0xc1257b7c,// 36 PAY  33 

    0x05928f63,// 37 PAY  34 

    0xce5c9d05,// 38 PAY  35 

    0x33b4830a,// 39 PAY  36 

    0x206650b9,// 40 PAY  37 

    0x93fc6bb8,// 41 PAY  38 

    0x93f302b9,// 42 PAY  39 

    0x658dc0a0,// 43 PAY  40 

    0xdddcba2c,// 44 PAY  41 

    0x3b40e697,// 45 PAY  42 

    0xf0b1b2c0,// 46 PAY  43 

    0x28098939,// 47 PAY  44 

    0xcb4d5685,// 48 PAY  45 

    0xf3e4f3cf,// 49 PAY  46 

    0xb174071f,// 50 PAY  47 

    0xd2cd664d,// 51 PAY  48 

    0x61c919ea,// 52 PAY  49 

    0x4afc5d94,// 53 PAY  50 

    0x46868b99,// 54 PAY  51 

    0xb4cc07bc,// 55 PAY  52 

    0xd8d76d2a,// 56 PAY  53 

    0xbd59a045,// 57 PAY  54 

    0x412c1380,// 58 PAY  55 

    0x5e11c0a8,// 59 PAY  56 

    0xd771f3cd,// 60 PAY  57 

    0x3ab56fd7,// 61 PAY  58 

    0x2cb29269,// 62 PAY  59 

    0x90cd4c5a,// 63 PAY  60 

    0xcbcbf620,// 64 PAY  61 

    0xfb3b4cbd,// 65 PAY  62 

    0x7f48d728,// 66 PAY  63 

    0xc5c76a5c,// 67 PAY  64 

    0xd9ad4f15,// 68 PAY  65 

    0x226fb674,// 69 PAY  66 

    0x22d0b719,// 70 PAY  67 

    0xb89a15fb,// 71 PAY  68 

    0xa93d6ca2,// 72 PAY  69 

    0x74218fc1,// 73 PAY  70 

    0x1230a7d8,// 74 PAY  71 

    0x3a7e1db1,// 75 PAY  72 

    0x7aa453d9,// 76 PAY  73 

    0x2236dace,// 77 PAY  74 

    0x76df1345,// 78 PAY  75 

    0xb20dbf5b,// 79 PAY  76 

    0x3dee02ba,// 80 PAY  77 

    0xb89e65a2,// 81 PAY  78 

    0xf7093071,// 82 PAY  79 

    0x73fb15e4,// 83 PAY  80 

    0xe068062d,// 84 PAY  81 

    0x5f19aa97,// 85 PAY  82 

    0x2d0dcf41,// 86 PAY  83 

    0xdcadfc9f,// 87 PAY  84 

    0xc33d6857,// 88 PAY  85 

    0xcfb6680e,// 89 PAY  86 

    0x9f0d29a7,// 90 PAY  87 

    0xfeb6b0af,// 91 PAY  88 

    0xfdbaf1e3,// 92 PAY  89 

    0x494e5c53,// 93 PAY  90 

    0x13b99f0b,// 94 PAY  91 

    0x674eb6b6,// 95 PAY  92 

    0x2b22ebbb,// 96 PAY  93 

    0x32f975a1,// 97 PAY  94 

    0xb611079e,// 98 PAY  95 

    0x3c9ccee6,// 99 PAY  96 

    0xcfb1a989,// 100 PAY  97 

    0x6e89eeca,// 101 PAY  98 

    0xa66de85a,// 102 PAY  99 

    0xe1ff852d,// 103 PAY 100 

    0xa70649b2,// 104 PAY 101 

    0x6d8462f1,// 105 PAY 102 

    0x2c0b3bd1,// 106 PAY 103 

    0xdeac023e,// 107 PAY 104 

    0xf399a259,// 108 PAY 105 

    0x1db88aea,// 109 PAY 106 

    0x289adeab,// 110 PAY 107 

    0xb01c138f,// 111 PAY 108 

    0x8db3d718,// 112 PAY 109 

    0xaf4d5331,// 113 PAY 110 

    0xe828c5fa,// 114 PAY 111 

    0x65a372c3,// 115 PAY 112 

    0x255a6263,// 116 PAY 113 

    0xad0f9916,// 117 PAY 114 

    0x3e6dc10d,// 118 PAY 115 

    0xef763987,// 119 PAY 116 

    0x949b270b,// 120 PAY 117 

    0x7d3f42d6,// 121 PAY 118 

    0x8ed3744e,// 122 PAY 119 

    0xc7895371,// 123 PAY 120 

    0x0a0e7bfd,// 124 PAY 121 

    0xaf65f201,// 125 PAY 122 

    0xafb583de,// 126 PAY 123 

    0xc7be8aa1,// 127 PAY 124 

    0x33785c9c,// 128 PAY 125 

    0xff372b68,// 129 PAY 126 

    0x713539a6,// 130 PAY 127 

    0x1e4ce329,// 131 PAY 128 

    0xee9a5db3,// 132 PAY 129 

    0x980e48ab,// 133 PAY 130 

    0xe2245687,// 134 PAY 131 

    0xbece1ff1,// 135 PAY 132 

    0x5dd08954,// 136 PAY 133 

    0x6cb9730f,// 137 PAY 134 

    0x9e1b343d,// 138 PAY 135 

    0x36c800c1,// 139 PAY 136 

    0x5bd3ea9d,// 140 PAY 137 

    0xe0fbd7a4,// 141 PAY 138 

    0x2f8b75fb,// 142 PAY 139 

    0x41fb7c8d,// 143 PAY 140 

    0x90b78c13,// 144 PAY 141 

    0x1de8b58e,// 145 PAY 142 

    0xfb31561b,// 146 PAY 143 

    0x4cf41faf,// 147 PAY 144 

    0xf9971af7,// 148 PAY 145 

    0x4934bdf6,// 149 PAY 146 

    0xf8b23eda,// 150 PAY 147 

    0x512d556b,// 151 PAY 148 

    0xc832c7e3,// 152 PAY 149 

    0xff4dddb3,// 153 PAY 150 

    0xa4993b86,// 154 PAY 151 

    0x80b63c47,// 155 PAY 152 

    0x0931ce79,// 156 PAY 153 

    0x31b88da4,// 157 PAY 154 

    0xcd98f4ef,// 158 PAY 155 

    0x9b5ddecf,// 159 PAY 156 

    0x63e6142c,// 160 PAY 157 

    0x5386c632,// 161 PAY 158 

    0x22827132,// 162 PAY 159 

    0xb57d4c53,// 163 PAY 160 

    0xcb6ba27c,// 164 PAY 161 

    0x80c3b74c,// 165 PAY 162 

    0xc8c93082,// 166 PAY 163 

    0xcbad9461,// 167 PAY 164 

    0xa10d43e9,// 168 PAY 165 

    0x4dd69933,// 169 PAY 166 

    0x2a80193b,// 170 PAY 167 

    0xd32e2fe3,// 171 PAY 168 

    0xd8c8d233,// 172 PAY 169 

    0xe9aeeb88,// 173 PAY 170 

    0xec039a0f,// 174 PAY 171 

    0x4e1ec15c,// 175 PAY 172 

    0xab9dd9e4,// 176 PAY 173 

    0x32c94f86,// 177 PAY 174 

    0x74038086,// 178 PAY 175 

    0x18490a20,// 179 PAY 176 

    0xbaffcee2,// 180 PAY 177 

    0x43a9d815,// 181 PAY 178 

    0xe0fe57d8,// 182 PAY 179 

    0x71e1826c,// 183 PAY 180 

    0x82cbdf6c,// 184 PAY 181 

    0x4195faf0,// 185 PAY 182 

    0x5922f06a,// 186 PAY 183 

    0xd5796dba,// 187 PAY 184 

    0x96cfffbe,// 188 PAY 185 

    0xb76c206e,// 189 PAY 186 

    0x639bb7fe,// 190 PAY 187 

    0x607789a6,// 191 PAY 188 

    0xc4faea8c,// 192 PAY 189 

    0xb3e42392,// 193 PAY 190 

    0xcc5b87e9,// 194 PAY 191 

    0xb253edde,// 195 PAY 192 

    0x08263f41,// 196 PAY 193 

    0xc3deafc8,// 197 PAY 194 

    0xd926e526,// 198 PAY 195 

    0xc0f06392,// 199 PAY 196 

    0x4bd41305,// 200 PAY 197 

    0x3fa3ad86,// 201 PAY 198 

    0x15eb628b,// 202 PAY 199 

    0x5fbcacf7,// 203 PAY 200 

    0xfe7a90b9,// 204 PAY 201 

    0xb293b2bd,// 205 PAY 202 

    0x9a312963,// 206 PAY 203 

    0xdb1e4657,// 207 PAY 204 

    0x3ddc0651,// 208 PAY 205 

    0xd001267b,// 209 PAY 206 

    0xcf4a4166,// 210 PAY 207 

    0x39cbdd60,// 211 PAY 208 

    0xa969cdd2,// 212 PAY 209 

    0x88cd6f63,// 213 PAY 210 

    0x7e66e9d3,// 214 PAY 211 

    0x291f8597,// 215 PAY 212 

    0x9f2d29d6,// 216 PAY 213 

    0x0bf4d538,// 217 PAY 214 

    0x7a9925b0,// 218 PAY 215 

    0xd3b1b48d,// 219 PAY 216 

    0x5d9b27fa,// 220 PAY 217 

    0x1cb0a5aa,// 221 PAY 218 

    0x2fcec358,// 222 PAY 219 

    0x211c9c9f,// 223 PAY 220 

    0xe9c2542d,// 224 PAY 221 

    0x7bbff41a,// 225 PAY 222 

    0x63dbb464,// 226 PAY 223 

    0xe1c2e2f8,// 227 PAY 224 

    0x6159c0e5,// 228 PAY 225 

    0xd9b9c1af,// 229 PAY 226 

    0x51992ea0,// 230 PAY 227 

    0x03cfc99d,// 231 PAY 228 

    0xbf2aef18,// 232 PAY 229 

    0x6029f8f5,// 233 PAY 230 

    0x801d3a4f,// 234 PAY 231 

    0x0fd15adc,// 235 PAY 232 

    0x6e26a0e7,// 236 PAY 233 

    0x4e424a2c,// 237 PAY 234 

    0xfad08890,// 238 PAY 235 

    0x1c712698,// 239 PAY 236 

    0x59ff737a,// 240 PAY 237 

    0x8d397d04,// 241 PAY 238 

    0xf8a52ae0,// 242 PAY 239 

    0xaab883ac,// 243 PAY 240 

    0xe41f25e6,// 244 PAY 241 

    0x7e11b295,// 245 PAY 242 

    0x27f0a276,// 246 PAY 243 

    0x481bdae6,// 247 PAY 244 

    0x254cc5e1,// 248 PAY 245 

    0x731fc027,// 249 PAY 246 

    0x9fbb0993,// 250 PAY 247 

    0x4bbe50f8,// 251 PAY 248 

    0x3c2163fa,// 252 PAY 249 

    0xa55cb87f,// 253 PAY 250 

    0x18eab902,// 254 PAY 251 

    0xdde9f511,// 255 PAY 252 

    0x9110d2e7,// 256 PAY 253 

    0xae1502fd,// 257 PAY 254 

    0x189d969f,// 258 PAY 255 

    0x8e412c06,// 259 PAY 256 

    0xb4c04d97,// 260 PAY 257 

    0x8814af3a,// 261 PAY 258 

    0x4ad40054,// 262 PAY 259 

    0x2f91f919,// 263 PAY 260 

    0xd10358fd,// 264 PAY 261 

    0x8c70f07c,// 265 PAY 262 

    0x4debc6bf,// 266 PAY 263 

    0x7a82287c,// 267 PAY 264 

    0xfa1091f9,// 268 PAY 265 

    0x23d0d34b,// 269 PAY 266 

    0x95d472c1,// 270 PAY 267 

    0x84d4be53,// 271 PAY 268 

    0x68d6ca95,// 272 PAY 269 

    0x70f545f4,// 273 PAY 270 

    0xfad9b6ef,// 274 PAY 271 

    0x452d0301,// 275 PAY 272 

    0xd27786cb,// 276 PAY 273 

    0xb39e3bed,// 277 PAY 274 

    0xcf967632,// 278 PAY 275 

    0xeb0d7d44,// 279 PAY 276 

    0x4bd52cf4,// 280 PAY 277 

    0x26eb5467,// 281 PAY 278 

    0x122cd02d,// 282 PAY 279 

    0xe535ec7c,// 283 PAY 280 

    0x70142e66,// 284 PAY 281 

    0xaf99b6ac,// 285 PAY 282 

    0xa6c3fb6f,// 286 PAY 283 

    0xbbd65777,// 287 PAY 284 

    0x0e6ba266,// 288 PAY 285 

    0x8b4e0b3e,// 289 PAY 286 

    0x13cc2c50,// 290 PAY 287 

    0x9034c7f1,// 291 PAY 288 

    0x0dc7d909,// 292 PAY 289 

    0xb7db8918,// 293 PAY 290 

    0xc5cf2b37,// 294 PAY 291 

    0x89364d1b,// 295 PAY 292 

    0xb8ee1293,// 296 PAY 293 

    0x171f374f,// 297 PAY 294 

    0x71653361,// 298 PAY 295 

    0x6fee3b29,// 299 PAY 296 

    0xac1a97a2,// 300 PAY 297 

    0xa7e41946,// 301 PAY 298 

    0x74725900,// 302 PAY 299 

    0x5e47541f,// 303 PAY 300 

    0x9ae36ee2,// 304 PAY 301 

    0x95576c18,// 305 PAY 302 

    0xe2df1927,// 306 PAY 303 

    0x615a0737,// 307 PAY 304 

    0x78998034,// 308 PAY 305 

    0x1f4e3808,// 309 PAY 306 

    0x210da937,// 310 PAY 307 

    0xc559d79a,// 311 PAY 308 

    0xc869bf8c,// 312 PAY 309 

    0x5ad68b14,// 313 PAY 310 

    0x700a2e61,// 314 PAY 311 

    0xe80d31e2,// 315 PAY 312 

    0x1aa0f2f4,// 316 PAY 313 

    0x39ef925f,// 317 PAY 314 

    0xb0d43c54,// 318 PAY 315 

    0x1feb8f51,// 319 PAY 316 

    0x24c42a2d,// 320 PAY 317 

    0x3453fdcc,// 321 PAY 318 

    0xf6eb4c21,// 322 PAY 319 

    0xbedbf6a2,// 323 PAY 320 

    0xbdebc1e7,// 324 PAY 321 

    0xf551bfda,// 325 PAY 322 

    0x5af62660,// 326 PAY 323 

    0x3a69fdb2,// 327 PAY 324 

    0x53c889ec,// 328 PAY 325 

    0x7ec1fa2b,// 329 PAY 326 

    0x608290c6,// 330 PAY 327 

    0x9d2d34f2,// 331 PAY 328 

    0xacb57b9e,// 332 PAY 329 

    0xa6fa7f46,// 333 PAY 330 

    0x88e65cc3,// 334 PAY 331 

    0x450fcef4,// 335 PAY 332 

    0x3554cf58,// 336 PAY 333 

    0xfee9164c,// 337 PAY 334 

    0x5d7008bf,// 338 PAY 335 

    0x824a221c,// 339 PAY 336 

    0xb9c2ed0d,// 340 PAY 337 

    0xd58a8169,// 341 PAY 338 

    0x2ff01251,// 342 PAY 339 

    0x3baaebf7,// 343 PAY 340 

    0x78b76766,// 344 PAY 341 

    0xa487fbfb,// 345 PAY 342 

    0x3c5564b8,// 346 PAY 343 

    0xfac05f84,// 347 PAY 344 

    0xa2260d22,// 348 PAY 345 

    0x9dad8805,// 349 PAY 346 

    0x078d990a,// 350 PAY 347 

    0x56ab8555,// 351 PAY 348 

    0xbaf04cec,// 352 PAY 349 

    0x255d5e55,// 353 PAY 350 

    0xd1190148,// 354 PAY 351 

    0x06ff0726,// 355 PAY 352 

    0x4cbec0bc,// 356 PAY 353 

    0x74251a55,// 357 PAY 354 

    0xb4801b76,// 358 PAY 355 

    0xcf99ac9c,// 359 PAY 356 

    0x0ec0bf61,// 360 PAY 357 

    0x7c74b194,// 361 PAY 358 

    0xf0712be4,// 362 PAY 359 

    0x7460dc58,// 363 PAY 360 

    0xf2a2f282,// 364 PAY 361 

    0x0b7ebc74,// 365 PAY 362 

    0xd906e2de,// 366 PAY 363 

    0x1210a8c0,// 367 PAY 364 

    0x251594a3,// 368 PAY 365 

    0x47b5712d,// 369 PAY 366 

    0x84c9a4d9,// 370 PAY 367 

    0xa392cd11,// 371 PAY 368 

    0x1155c9c0,// 372 PAY 369 

    0x63832f94,// 373 PAY 370 

    0x3e77aa3f,// 374 PAY 371 

    0x0525945e,// 375 PAY 372 

    0x057a5bac,// 376 PAY 373 

    0x42bc5f32,// 377 PAY 374 

    0x4d708444,// 378 PAY 375 

    0x9ca7d376,// 379 PAY 376 

    0xfc9c5e4c,// 380 PAY 377 

    0x54e11609,// 381 PAY 378 

    0xea6b90d3,// 382 PAY 379 

    0x8cc66850,// 383 PAY 380 

    0x87a3257a,// 384 PAY 381 

    0x640920d2,// 385 PAY 382 

    0x64d91006,// 386 PAY 383 

    0x4329d608,// 387 PAY 384 

    0x30c88cae,// 388 PAY 385 

    0x566630df,// 389 PAY 386 

    0x2e5d8d49,// 390 PAY 387 

    0xf87ad6eb,// 391 PAY 388 

    0x068f2d23,// 392 PAY 389 

    0x039853c1,// 393 PAY 390 

    0x80916362,// 394 PAY 391 

    0x02cf6eaa,// 395 PAY 392 

    0x6d3fbc5e,// 396 PAY 393 

    0x6c39b440,// 397 PAY 394 

    0x15479886,// 398 PAY 395 

    0x20b54356,// 399 PAY 396 

    0x66b9b411,// 400 PAY 397 

    0xe15e40a4,// 401 PAY 398 

    0xa830c45e,// 402 PAY 399 

    0x9a47ac55,// 403 PAY 400 

    0x5e2c89cd,// 404 PAY 401 

    0x3343f148,// 405 PAY 402 

    0x6e3e84c9,// 406 PAY 403 

    0x8980a243,// 407 PAY 404 

    0x16e5fdc6,// 408 PAY 405 

    0xcb944e9e,// 409 PAY 406 

    0x9935510c,// 410 PAY 407 

    0xe39f1def,// 411 PAY 408 

    0x686a2ba5,// 412 PAY 409 

    0xee6b88f7,// 413 PAY 410 

    0xfb816c36,// 414 PAY 411 

    0xa2a7e901,// 415 PAY 412 

    0xf79f961a,// 416 PAY 413 

    0x409b6674,// 417 PAY 414 

    0x4058c941,// 418 PAY 415 

    0xfec72dfc,// 419 PAY 416 

    0xae090000,// 420 PAY 417 

/// STA is 1 words. 

/// STA num_pkts       : 144 

/// STA pkt_idx        : 108 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x01b11c90 // 421 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt49_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 249 words. 

/// BDA size     is 989 (0x3dd) 

/// BDA id       is 0x5630 

    0x03dd5630,// 3 BDA   1 

/// PAY Generic Data size   : 989 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x7d76516d,// 4 PAY   1 

    0x8d95422d,// 5 PAY   2 

    0x17f71de5,// 6 PAY   3 

    0x75224a54,// 7 PAY   4 

    0xd39bfb6a,// 8 PAY   5 

    0xee31a0b8,// 9 PAY   6 

    0x39746012,// 10 PAY   7 

    0x5f3ec2c6,// 11 PAY   8 

    0x8a7624be,// 12 PAY   9 

    0x10950b22,// 13 PAY  10 

    0xf3683204,// 14 PAY  11 

    0x9acc281c,// 15 PAY  12 

    0x4cffa542,// 16 PAY  13 

    0x10d5185a,// 17 PAY  14 

    0x63a15a44,// 18 PAY  15 

    0x6acfa303,// 19 PAY  16 

    0x447aaf36,// 20 PAY  17 

    0x77e00163,// 21 PAY  18 

    0xc21fae96,// 22 PAY  19 

    0x48064ca1,// 23 PAY  20 

    0x3552b5f7,// 24 PAY  21 

    0x0cf50f6d,// 25 PAY  22 

    0x49bdbe1b,// 26 PAY  23 

    0xeca2dd89,// 27 PAY  24 

    0x792083c6,// 28 PAY  25 

    0x10f9d7bd,// 29 PAY  26 

    0x2f085e21,// 30 PAY  27 

    0x97c8dfa9,// 31 PAY  28 

    0xc04dc484,// 32 PAY  29 

    0x598ba57f,// 33 PAY  30 

    0x7cfd587f,// 34 PAY  31 

    0x72e63ada,// 35 PAY  32 

    0xdad53187,// 36 PAY  33 

    0x86dc2f84,// 37 PAY  34 

    0x3933ff68,// 38 PAY  35 

    0xed615647,// 39 PAY  36 

    0x32b1ccf2,// 40 PAY  37 

    0x5fd65fbd,// 41 PAY  38 

    0x802124af,// 42 PAY  39 

    0x0c533cdd,// 43 PAY  40 

    0x28e89ad1,// 44 PAY  41 

    0x442adf51,// 45 PAY  42 

    0x25bb0485,// 46 PAY  43 

    0x63a06e20,// 47 PAY  44 

    0x65c63ff9,// 48 PAY  45 

    0x6cf52b34,// 49 PAY  46 

    0x34cf64d4,// 50 PAY  47 

    0x6f28985c,// 51 PAY  48 

    0x0501e96b,// 52 PAY  49 

    0xe8edfded,// 53 PAY  50 

    0x9546e2f0,// 54 PAY  51 

    0x6790097b,// 55 PAY  52 

    0xf0eb10c6,// 56 PAY  53 

    0xbd6bd919,// 57 PAY  54 

    0x2ebd1ab2,// 58 PAY  55 

    0x9a7f3aef,// 59 PAY  56 

    0xc13222f3,// 60 PAY  57 

    0xdc6969d0,// 61 PAY  58 

    0x42a963dd,// 62 PAY  59 

    0x46e1b130,// 63 PAY  60 

    0x021120f6,// 64 PAY  61 

    0x159bd2c7,// 65 PAY  62 

    0x39590e15,// 66 PAY  63 

    0x169c3c01,// 67 PAY  64 

    0x3816ec22,// 68 PAY  65 

    0xada43646,// 69 PAY  66 

    0xb0a2a1ac,// 70 PAY  67 

    0x5f19ee25,// 71 PAY  68 

    0xd734526c,// 72 PAY  69 

    0x82281f88,// 73 PAY  70 

    0xbb542a79,// 74 PAY  71 

    0x626efaa9,// 75 PAY  72 

    0x3e0f9209,// 76 PAY  73 

    0x642e30ab,// 77 PAY  74 

    0x237e2a46,// 78 PAY  75 

    0xbd66dd46,// 79 PAY  76 

    0x641c5ffb,// 80 PAY  77 

    0x6457cb6d,// 81 PAY  78 

    0xac559ec4,// 82 PAY  79 

    0x898bc5da,// 83 PAY  80 

    0x157dcc21,// 84 PAY  81 

    0x66fc0f19,// 85 PAY  82 

    0x9e68534e,// 86 PAY  83 

    0x1b18980a,// 87 PAY  84 

    0xadf119c1,// 88 PAY  85 

    0xaae2caa5,// 89 PAY  86 

    0xe3662e0e,// 90 PAY  87 

    0xfffb242e,// 91 PAY  88 

    0xdac8647e,// 92 PAY  89 

    0xe171afa7,// 93 PAY  90 

    0x5e36e304,// 94 PAY  91 

    0xab7bfe25,// 95 PAY  92 

    0xb9545de5,// 96 PAY  93 

    0x17065a9e,// 97 PAY  94 

    0x2d09c42e,// 98 PAY  95 

    0x3b8017c5,// 99 PAY  96 

    0x3859d2ad,// 100 PAY  97 

    0x16d7837b,// 101 PAY  98 

    0x6ff46e2d,// 102 PAY  99 

    0x8012b1c3,// 103 PAY 100 

    0x59096c9d,// 104 PAY 101 

    0x4c246aec,// 105 PAY 102 

    0x00d4dcd2,// 106 PAY 103 

    0xb55b4d52,// 107 PAY 104 

    0x562793b3,// 108 PAY 105 

    0x026eaaee,// 109 PAY 106 

    0x6deb43a7,// 110 PAY 107 

    0x1c32d04d,// 111 PAY 108 

    0x3b943567,// 112 PAY 109 

    0xb004ccaf,// 113 PAY 110 

    0x81abe610,// 114 PAY 111 

    0x85a78a49,// 115 PAY 112 

    0x097f6de5,// 116 PAY 113 

    0x574a653e,// 117 PAY 114 

    0xfe2cb8b3,// 118 PAY 115 

    0x1869fc77,// 119 PAY 116 

    0x48bab9fd,// 120 PAY 117 

    0xd1f192e2,// 121 PAY 118 

    0x655b3516,// 122 PAY 119 

    0x50668e0c,// 123 PAY 120 

    0x84a23bff,// 124 PAY 121 

    0x525c4fe8,// 125 PAY 122 

    0x453b78c3,// 126 PAY 123 

    0x167e4fe2,// 127 PAY 124 

    0x79a11386,// 128 PAY 125 

    0x5be1108f,// 129 PAY 126 

    0xae0670f6,// 130 PAY 127 

    0x68c551ea,// 131 PAY 128 

    0xd9c55738,// 132 PAY 129 

    0x9b80c2e3,// 133 PAY 130 

    0x862e41d5,// 134 PAY 131 

    0x5169cdb0,// 135 PAY 132 

    0x6b107fc4,// 136 PAY 133 

    0x1f925220,// 137 PAY 134 

    0xd11cd97c,// 138 PAY 135 

    0xe8ca495f,// 139 PAY 136 

    0x303df882,// 140 PAY 137 

    0x2b610de5,// 141 PAY 138 

    0x7df17b7a,// 142 PAY 139 

    0xb041a573,// 143 PAY 140 

    0x779eb9dc,// 144 PAY 141 

    0x42a22fbb,// 145 PAY 142 

    0x96039a79,// 146 PAY 143 

    0x91a5faa8,// 147 PAY 144 

    0xcb32a219,// 148 PAY 145 

    0x01263dc4,// 149 PAY 146 

    0xa6d65128,// 150 PAY 147 

    0xf8cbb6e5,// 151 PAY 148 

    0xad28ad41,// 152 PAY 149 

    0x8f52eace,// 153 PAY 150 

    0x4e5237e6,// 154 PAY 151 

    0xfdc1c40a,// 155 PAY 152 

    0x461655a5,// 156 PAY 153 

    0xe48fecfa,// 157 PAY 154 

    0x8187bbc5,// 158 PAY 155 

    0x02f93a08,// 159 PAY 156 

    0x82a61ee7,// 160 PAY 157 

    0x35523b3e,// 161 PAY 158 

    0x2dace7b8,// 162 PAY 159 

    0x4ad0c18e,// 163 PAY 160 

    0x1a66be71,// 164 PAY 161 

    0x88d72be6,// 165 PAY 162 

    0xe34ce137,// 166 PAY 163 

    0xb628f549,// 167 PAY 164 

    0x80440f01,// 168 PAY 165 

    0xc7b92ae6,// 169 PAY 166 

    0x64f3a7ae,// 170 PAY 167 

    0xcb6eb0ae,// 171 PAY 168 

    0x2a9dffc2,// 172 PAY 169 

    0x6cc93002,// 173 PAY 170 

    0xa9a0b015,// 174 PAY 171 

    0x8d5fb9be,// 175 PAY 172 

    0x1411e8a7,// 176 PAY 173 

    0xb9d3a7ac,// 177 PAY 174 

    0x7f5a8b4a,// 178 PAY 175 

    0x610f490f,// 179 PAY 176 

    0xde88697b,// 180 PAY 177 

    0xe01434d9,// 181 PAY 178 

    0x426f708d,// 182 PAY 179 

    0x730ed8dd,// 183 PAY 180 

    0x7d9c71e9,// 184 PAY 181 

    0x8634e829,// 185 PAY 182 

    0x533676a9,// 186 PAY 183 

    0x0e75bd81,// 187 PAY 184 

    0xa7f98f0f,// 188 PAY 185 

    0xb332a486,// 189 PAY 186 

    0xbe41f7e0,// 190 PAY 187 

    0xb56a1163,// 191 PAY 188 

    0x4de5d80a,// 192 PAY 189 

    0x98ddde5f,// 193 PAY 190 

    0xc984896d,// 194 PAY 191 

    0x15f65ba0,// 195 PAY 192 

    0x40ffcf57,// 196 PAY 193 

    0x23b1053b,// 197 PAY 194 

    0x1722ad3f,// 198 PAY 195 

    0x1c32ac62,// 199 PAY 196 

    0x7e63dacc,// 200 PAY 197 

    0x808fbb06,// 201 PAY 198 

    0x0679c151,// 202 PAY 199 

    0x1e28daca,// 203 PAY 200 

    0x2b2cf46c,// 204 PAY 201 

    0xd8237d7b,// 205 PAY 202 

    0x1a6e89b1,// 206 PAY 203 

    0xeacd7b94,// 207 PAY 204 

    0x07ed36a4,// 208 PAY 205 

    0xa001c041,// 209 PAY 206 

    0xb05bf173,// 210 PAY 207 

    0x081d6cd1,// 211 PAY 208 

    0xaec8dafb,// 212 PAY 209 

    0x25a58eac,// 213 PAY 210 

    0x9e9c3f6d,// 214 PAY 211 

    0x6875f8c4,// 215 PAY 212 

    0x761dc557,// 216 PAY 213 

    0x3e0a3e24,// 217 PAY 214 

    0xf40a4a3b,// 218 PAY 215 

    0x1da45178,// 219 PAY 216 

    0xb30c7dc9,// 220 PAY 217 

    0x74cfb848,// 221 PAY 218 

    0x850a226b,// 222 PAY 219 

    0x613a6cb1,// 223 PAY 220 

    0x496cb550,// 224 PAY 221 

    0xce8b9bb2,// 225 PAY 222 

    0x04f2b2fe,// 226 PAY 223 

    0x1b95bce9,// 227 PAY 224 

    0x21d00526,// 228 PAY 225 

    0xe159e064,// 229 PAY 226 

    0x94d338d1,// 230 PAY 227 

    0x487f80d7,// 231 PAY 228 

    0x5f6ab3c1,// 232 PAY 229 

    0xbce71629,// 233 PAY 230 

    0xbeb91018,// 234 PAY 231 

    0x29893e11,// 235 PAY 232 

    0xd6499a03,// 236 PAY 233 

    0xbd939568,// 237 PAY 234 

    0x68d878e2,// 238 PAY 235 

    0x3e0697b1,// 239 PAY 236 

    0x4ad48c95,// 240 PAY 237 

    0x61ed2166,// 241 PAY 238 

    0x88c3a732,// 242 PAY 239 

    0x2057be90,// 243 PAY 240 

    0x8ab2c2c9,// 244 PAY 241 

    0x84d9c269,// 245 PAY 242 

    0xe0769b9e,// 246 PAY 243 

    0x61fce538,// 247 PAY 244 

    0x41d80120,// 248 PAY 245 

    0x24046ac8,// 249 PAY 246 

    0x1979e7d2,// 250 PAY 247 

    0x9f000000,// 251 PAY 248 

/// HASH is  12 bytes 

    0xb332a486,// 252 HSH   1 

    0xbe41f7e0,// 253 HSH   2 

    0xb56a1163,// 254 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 101 

/// STA pkt_idx        : 196 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0311ce65 // 255 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt50_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 53 words. 

/// BDA size     is 206 (0xce) 

/// BDA id       is 0x3ee5 

    0x00ce3ee5,// 3 BDA   1 

/// PAY Generic Data size   : 206 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x795e67d0,// 4 PAY   1 

    0x5eb7a000,// 5 PAY   2 

    0xcec54cec,// 6 PAY   3 

    0x2db59833,// 7 PAY   4 

    0x53adb8f2,// 8 PAY   5 

    0x121105bd,// 9 PAY   6 

    0x4f01b8c1,// 10 PAY   7 

    0x1307bf38,// 11 PAY   8 

    0x12dd6858,// 12 PAY   9 

    0xe07b9ae2,// 13 PAY  10 

    0x98d669dd,// 14 PAY  11 

    0x5d504ba2,// 15 PAY  12 

    0x8e2a6015,// 16 PAY  13 

    0x0607969e,// 17 PAY  14 

    0x3769237d,// 18 PAY  15 

    0xccae1960,// 19 PAY  16 

    0x1f5dce27,// 20 PAY  17 

    0x0785e138,// 21 PAY  18 

    0x6fb8f83d,// 22 PAY  19 

    0xb524c4ee,// 23 PAY  20 

    0xddc24e04,// 24 PAY  21 

    0x3b890b80,// 25 PAY  22 

    0xc2c24c5a,// 26 PAY  23 

    0x7fd338ff,// 27 PAY  24 

    0xb19b708b,// 28 PAY  25 

    0xe6657d63,// 29 PAY  26 

    0x1158a008,// 30 PAY  27 

    0xbacfc931,// 31 PAY  28 

    0xad6f2283,// 32 PAY  29 

    0xe803f986,// 33 PAY  30 

    0x15383164,// 34 PAY  31 

    0xf62568bd,// 35 PAY  32 

    0x3f34516f,// 36 PAY  33 

    0x6ef91f05,// 37 PAY  34 

    0xa1b75ebb,// 38 PAY  35 

    0xd93db0ec,// 39 PAY  36 

    0xcdd0cf95,// 40 PAY  37 

    0x81cb96bb,// 41 PAY  38 

    0x3f6ac714,// 42 PAY  39 

    0x69bd1f19,// 43 PAY  40 

    0x50ac9a0b,// 44 PAY  41 

    0xf0465b77,// 45 PAY  42 

    0x3d81ce01,// 46 PAY  43 

    0xc1e3317a,// 47 PAY  44 

    0x6b4ce937,// 48 PAY  45 

    0xaa16abb3,// 49 PAY  46 

    0x304ea833,// 50 PAY  47 

    0xe68f2ca7,// 51 PAY  48 

    0x5080f08f,// 52 PAY  49 

    0xf1856405,// 53 PAY  50 

    0x9044e685,// 54 PAY  51 

    0xa3eb0000,// 55 PAY  52 

/// STA is 1 words. 

/// STA num_pkts       : 196 

/// STA pkt_idx        : 51 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x52 

    0x00cc52c4 // 56 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt51_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 248 words. 

/// BDA size     is 986 (0x3da) 

/// BDA id       is 0x30ad 

    0x03da30ad,// 3 BDA   1 

/// PAY Generic Data size   : 986 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe8c91812,// 4 PAY   1 

    0xa0638ad7,// 5 PAY   2 

    0x62467a32,// 6 PAY   3 

    0x18e55351,// 7 PAY   4 

    0xc88913d8,// 8 PAY   5 

    0x1ca7f59a,// 9 PAY   6 

    0xf251679b,// 10 PAY   7 

    0x6030bae1,// 11 PAY   8 

    0x11ad6e2a,// 12 PAY   9 

    0xdadee9ea,// 13 PAY  10 

    0xa4ec220c,// 14 PAY  11 

    0x532d62d2,// 15 PAY  12 

    0x014e5dfc,// 16 PAY  13 

    0x0b57a506,// 17 PAY  14 

    0x1759d0df,// 18 PAY  15 

    0x953b18e8,// 19 PAY  16 

    0x2372424e,// 20 PAY  17 

    0x02505869,// 21 PAY  18 

    0xace428ed,// 22 PAY  19 

    0xe6749af7,// 23 PAY  20 

    0x803ad023,// 24 PAY  21 

    0x30782348,// 25 PAY  22 

    0xc187396b,// 26 PAY  23 

    0x09336739,// 27 PAY  24 

    0x79742fe2,// 28 PAY  25 

    0x4452bd9e,// 29 PAY  26 

    0x87562553,// 30 PAY  27 

    0x169fa331,// 31 PAY  28 

    0x73a6ee3b,// 32 PAY  29 

    0x540622a4,// 33 PAY  30 

    0xb2251401,// 34 PAY  31 

    0xca2e3f1d,// 35 PAY  32 

    0x032f24c6,// 36 PAY  33 

    0x393708cc,// 37 PAY  34 

    0x1de6d1f8,// 38 PAY  35 

    0x2d18b8f6,// 39 PAY  36 

    0x9cc327b4,// 40 PAY  37 

    0x03bf3048,// 41 PAY  38 

    0x863c6837,// 42 PAY  39 

    0x9107d52d,// 43 PAY  40 

    0xdbe12a0c,// 44 PAY  41 

    0xcc25a7a0,// 45 PAY  42 

    0xbeb03e32,// 46 PAY  43 

    0x36d00207,// 47 PAY  44 

    0x1265dc10,// 48 PAY  45 

    0x09f2b5de,// 49 PAY  46 

    0x87c46750,// 50 PAY  47 

    0x6a36910b,// 51 PAY  48 

    0xf827d430,// 52 PAY  49 

    0x52a5d740,// 53 PAY  50 

    0xc1b5af78,// 54 PAY  51 

    0x96b2dd38,// 55 PAY  52 

    0x08cf8b57,// 56 PAY  53 

    0x2f145be0,// 57 PAY  54 

    0x7474495f,// 58 PAY  55 

    0xff89846a,// 59 PAY  56 

    0xccb74e8f,// 60 PAY  57 

    0x91bd36e9,// 61 PAY  58 

    0x723df8d1,// 62 PAY  59 

    0x6821e29e,// 63 PAY  60 

    0xdefd66aa,// 64 PAY  61 

    0x6c38e332,// 65 PAY  62 

    0x16285b2e,// 66 PAY  63 

    0xc91ac647,// 67 PAY  64 

    0x6a68f23c,// 68 PAY  65 

    0x0842200f,// 69 PAY  66 

    0x8ebd3bca,// 70 PAY  67 

    0x15bfa6c8,// 71 PAY  68 

    0x326a7c7d,// 72 PAY  69 

    0x67ccc261,// 73 PAY  70 

    0xa20a578c,// 74 PAY  71 

    0x1222dc20,// 75 PAY  72 

    0xcb5ad47c,// 76 PAY  73 

    0x533373f8,// 77 PAY  74 

    0x23da587d,// 78 PAY  75 

    0x68b4428b,// 79 PAY  76 

    0x95932047,// 80 PAY  77 

    0x02f6ca10,// 81 PAY  78 

    0xf1d532ca,// 82 PAY  79 

    0xcaba03ac,// 83 PAY  80 

    0xcceb16f7,// 84 PAY  81 

    0xeabfb0a8,// 85 PAY  82 

    0x2a5ebbe9,// 86 PAY  83 

    0x6a011d49,// 87 PAY  84 

    0x1cfe031c,// 88 PAY  85 

    0xcde76817,// 89 PAY  86 

    0xea599a64,// 90 PAY  87 

    0x1f73c872,// 91 PAY  88 

    0xd7ad108d,// 92 PAY  89 

    0x489b229f,// 93 PAY  90 

    0x44bffe93,// 94 PAY  91 

    0x3a16432c,// 95 PAY  92 

    0x60fdf234,// 96 PAY  93 

    0x6c72190c,// 97 PAY  94 

    0x637fa6f6,// 98 PAY  95 

    0xfb8cd506,// 99 PAY  96 

    0x32eddabe,// 100 PAY  97 

    0x692ea1a2,// 101 PAY  98 

    0x70d53310,// 102 PAY  99 

    0x5f2e7f19,// 103 PAY 100 

    0xf84323d8,// 104 PAY 101 

    0xce04131c,// 105 PAY 102 

    0xe03311f9,// 106 PAY 103 

    0xb6685f52,// 107 PAY 104 

    0xd663d425,// 108 PAY 105 

    0x0daa0ada,// 109 PAY 106 

    0x0740b51f,// 110 PAY 107 

    0x695d2fc3,// 111 PAY 108 

    0x71569914,// 112 PAY 109 

    0xa81fc066,// 113 PAY 110 

    0xb56dda9b,// 114 PAY 111 

    0xe7b69c22,// 115 PAY 112 

    0x0504ab1a,// 116 PAY 113 

    0xe5132282,// 117 PAY 114 

    0x19484ab4,// 118 PAY 115 

    0x7baafe7d,// 119 PAY 116 

    0x94aa781d,// 120 PAY 117 

    0x163a5ad5,// 121 PAY 118 

    0x32c8950f,// 122 PAY 119 

    0x4f0d7520,// 123 PAY 120 

    0x75a43a31,// 124 PAY 121 

    0x9640a5a6,// 125 PAY 122 

    0x301a2676,// 126 PAY 123 

    0x11ae0955,// 127 PAY 124 

    0x0b6e7d5f,// 128 PAY 125 

    0xdbeccf49,// 129 PAY 126 

    0x2ce34440,// 130 PAY 127 

    0xd03a753c,// 131 PAY 128 

    0xe970a9a8,// 132 PAY 129 

    0xb14edfd7,// 133 PAY 130 

    0x0dc94918,// 134 PAY 131 

    0x7fee7170,// 135 PAY 132 

    0x560a62e1,// 136 PAY 133 

    0xeb9a3866,// 137 PAY 134 

    0x419d37f9,// 138 PAY 135 

    0x9a799479,// 139 PAY 136 

    0x0d9a8c3b,// 140 PAY 137 

    0x3132f29d,// 141 PAY 138 

    0xd4d4284f,// 142 PAY 139 

    0x3e9f8ff5,// 143 PAY 140 

    0xdba66d70,// 144 PAY 141 

    0x7fc6615b,// 145 PAY 142 

    0xbf2b8e06,// 146 PAY 143 

    0x60c5d2a5,// 147 PAY 144 

    0xbcc2b5b1,// 148 PAY 145 

    0x6ebcfa97,// 149 PAY 146 

    0xab51c478,// 150 PAY 147 

    0xac542351,// 151 PAY 148 

    0xb7dca2e4,// 152 PAY 149 

    0x1d08ab1e,// 153 PAY 150 

    0x831eda19,// 154 PAY 151 

    0xfb87d30f,// 155 PAY 152 

    0xecb00cd0,// 156 PAY 153 

    0x7fc0f5ec,// 157 PAY 154 

    0x17627623,// 158 PAY 155 

    0x125f0b8f,// 159 PAY 156 

    0x91b42fe5,// 160 PAY 157 

    0x8ae3f484,// 161 PAY 158 

    0x71ccd923,// 162 PAY 159 

    0xd9050008,// 163 PAY 160 

    0x91fe32b1,// 164 PAY 161 

    0x7152dcb3,// 165 PAY 162 

    0x808d6c6b,// 166 PAY 163 

    0xa1bb19e6,// 167 PAY 164 

    0x7cd7da9c,// 168 PAY 165 

    0xc6e583ab,// 169 PAY 166 

    0x5bafcdeb,// 170 PAY 167 

    0x047b13f0,// 171 PAY 168 

    0xd4c70a2c,// 172 PAY 169 

    0xd3f1fddf,// 173 PAY 170 

    0xc92ec655,// 174 PAY 171 

    0x2fa2bb29,// 175 PAY 172 

    0xa826a121,// 176 PAY 173 

    0x8fce2043,// 177 PAY 174 

    0xd5aa5003,// 178 PAY 175 

    0x5c8a6f4d,// 179 PAY 176 

    0x6fd3323e,// 180 PAY 177 

    0x2cdb841c,// 181 PAY 178 

    0xc7cbcc90,// 182 PAY 179 

    0xd070dbab,// 183 PAY 180 

    0x11b4958a,// 184 PAY 181 

    0x726844d1,// 185 PAY 182 

    0xe36e8410,// 186 PAY 183 

    0x66c86742,// 187 PAY 184 

    0xce24c0e8,// 188 PAY 185 

    0xa5a3ee15,// 189 PAY 186 

    0x1feb9c98,// 190 PAY 187 

    0xc639b8c6,// 191 PAY 188 

    0xc77aaee0,// 192 PAY 189 

    0x1503dfe0,// 193 PAY 190 

    0x0597fb36,// 194 PAY 191 

    0xfc855a09,// 195 PAY 192 

    0x845ae981,// 196 PAY 193 

    0x528a477a,// 197 PAY 194 

    0x8489216c,// 198 PAY 195 

    0xa9a76d78,// 199 PAY 196 

    0x9eacdf22,// 200 PAY 197 

    0x741b2aba,// 201 PAY 198 

    0xf1ac56d4,// 202 PAY 199 

    0xee355431,// 203 PAY 200 

    0xceaf39fa,// 204 PAY 201 

    0xc5f32db2,// 205 PAY 202 

    0x04a1bc53,// 206 PAY 203 

    0x12d6d306,// 207 PAY 204 

    0x461ab5c7,// 208 PAY 205 

    0xea5bd9de,// 209 PAY 206 

    0x1a995d55,// 210 PAY 207 

    0xc5280242,// 211 PAY 208 

    0x136e0574,// 212 PAY 209 

    0x955eb000,// 213 PAY 210 

    0xb61469b6,// 214 PAY 211 

    0xe810ba4d,// 215 PAY 212 

    0x56e1eb4a,// 216 PAY 213 

    0xde860296,// 217 PAY 214 

    0x95cc7d47,// 218 PAY 215 

    0xf5266bf0,// 219 PAY 216 

    0x5d4354ad,// 220 PAY 217 

    0xcf17bab3,// 221 PAY 218 

    0x9651b996,// 222 PAY 219 

    0xa1e0d2ae,// 223 PAY 220 

    0x14828fd1,// 224 PAY 221 

    0x8df01e34,// 225 PAY 222 

    0x1068586b,// 226 PAY 223 

    0xe25418cd,// 227 PAY 224 

    0x70a870fe,// 228 PAY 225 

    0x324e895e,// 229 PAY 226 

    0xed47463a,// 230 PAY 227 

    0x2bb2029c,// 231 PAY 228 

    0x246873b0,// 232 PAY 229 

    0xc49d6e33,// 233 PAY 230 

    0xed7c682e,// 234 PAY 231 

    0x69ea0c4e,// 235 PAY 232 

    0xfd8d79b1,// 236 PAY 233 

    0x8e377392,// 237 PAY 234 

    0x83111738,// 238 PAY 235 

    0x8abe8215,// 239 PAY 236 

    0x62c09df7,// 240 PAY 237 

    0xabc442d9,// 241 PAY 238 

    0xa935778a,// 242 PAY 239 

    0x8d6cafa8,// 243 PAY 240 

    0x925dd073,// 244 PAY 241 

    0x8f1fcea9,// 245 PAY 242 

    0x9eac871c,// 246 PAY 243 

    0x32617f09,// 247 PAY 244 

    0x2595fdfd,// 248 PAY 245 

    0x658d83d4,// 249 PAY 246 

    0x2b410000,// 250 PAY 247 

/// HASH is  4 bytes 

    0x69ea0c4e,// 251 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 59 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb6 

    0x0050b63b // 252 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt52_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 15 words. 

/// BDA size     is 55 (0x37) 

/// BDA id       is 0xa607 

    0x0037a607,// 3 BDA   1 

/// PAY Generic Data size   : 55 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x3cc2368f,// 4 PAY   1 

    0x88f7f10e,// 5 PAY   2 

    0x148636ad,// 6 PAY   3 

    0x4e133cac,// 7 PAY   4 

    0x6449e8be,// 8 PAY   5 

    0x776df8b1,// 9 PAY   6 

    0xb990f0ee,// 10 PAY   7 

    0x8a2e3dc6,// 11 PAY   8 

    0x47f897f8,// 12 PAY   9 

    0x35bd8853,// 13 PAY  10 

    0x5cd0c8fb,// 14 PAY  11 

    0x2182984e,// 15 PAY  12 

    0x311b307a,// 16 PAY  13 

    0x2b40ef00,// 17 PAY  14 

/// STA is 1 words. 

/// STA num_pkts       : 163 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x58 

    0x03d958a3 // 18 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt53_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 22 words. 

/// BDA size     is 83 (0x53) 

/// BDA id       is 0x7e9d 

    0x00537e9d,// 3 BDA   1 

/// PAY Generic Data size   : 83 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x463aaec9,// 4 PAY   1 

    0xe8c269a4,// 5 PAY   2 

    0x6a312146,// 6 PAY   3 

    0x944f5e19,// 7 PAY   4 

    0xfe819aec,// 8 PAY   5 

    0xd090da5b,// 9 PAY   6 

    0xec25a455,// 10 PAY   7 

    0xf950028a,// 11 PAY   8 

    0xd7854dc4,// 12 PAY   9 

    0xa0d3479c,// 13 PAY  10 

    0x0166b1e6,// 14 PAY  11 

    0x1040b6ab,// 15 PAY  12 

    0x88c6dad6,// 16 PAY  13 

    0xa0d29a23,// 17 PAY  14 

    0x142aa939,// 18 PAY  15 

    0x3aff10e6,// 19 PAY  16 

    0x2c158463,// 20 PAY  17 

    0xa51efc35,// 21 PAY  18 

    0x7c540825,// 22 PAY  19 

    0x32bb54a0,// 23 PAY  20 

    0x1846e700,// 24 PAY  21 

/// HASH is  12 bytes 

    0x3aff10e6,// 25 HSH   1 

    0x2c158463,// 26 HSH   2 

    0xa51efc35,// 27 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 44 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf8 

    0x00b1f806 // 28 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt54_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 464 words. 

/// BDA size     is 1852 (0x73c) 

/// BDA id       is 0x5aee 

    0x073c5aee,// 3 BDA   1 

/// PAY Generic Data size   : 1852 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xab66ef16,// 4 PAY   1 

    0xc332dca5,// 5 PAY   2 

    0xc8736184,// 6 PAY   3 

    0x8fce97b8,// 7 PAY   4 

    0xaa35a01c,// 8 PAY   5 

    0xfd7febf6,// 9 PAY   6 

    0x524c769b,// 10 PAY   7 

    0xeb58543f,// 11 PAY   8 

    0x613d59f7,// 12 PAY   9 

    0xb811f535,// 13 PAY  10 

    0x99d32fa9,// 14 PAY  11 

    0x3f2e8bc2,// 15 PAY  12 

    0xce43b0b5,// 16 PAY  13 

    0x96b1e478,// 17 PAY  14 

    0x4018041d,// 18 PAY  15 

    0xc586d5d8,// 19 PAY  16 

    0x46dd7ce1,// 20 PAY  17 

    0x3be88ba5,// 21 PAY  18 

    0xfcabeb34,// 22 PAY  19 

    0x5232c3d1,// 23 PAY  20 

    0x9b0e49c7,// 24 PAY  21 

    0x628c7e58,// 25 PAY  22 

    0x74e764c7,// 26 PAY  23 

    0xe01ca325,// 27 PAY  24 

    0xbffc8be5,// 28 PAY  25 

    0x456acc37,// 29 PAY  26 

    0x921523eb,// 30 PAY  27 

    0x59a883b8,// 31 PAY  28 

    0xae060f1a,// 32 PAY  29 

    0xedccbadf,// 33 PAY  30 

    0x491617cd,// 34 PAY  31 

    0x805c9d55,// 35 PAY  32 

    0x2ab5026b,// 36 PAY  33 

    0x8c16706a,// 37 PAY  34 

    0x4b0d6271,// 38 PAY  35 

    0x9e9780b9,// 39 PAY  36 

    0x016bdc93,// 40 PAY  37 

    0x9dde8b25,// 41 PAY  38 

    0xc2228174,// 42 PAY  39 

    0xcfb3ac94,// 43 PAY  40 

    0x266d0cfe,// 44 PAY  41 

    0x787876d9,// 45 PAY  42 

    0xe7809a19,// 46 PAY  43 

    0xe28c4e4a,// 47 PAY  44 

    0xffa8d518,// 48 PAY  45 

    0xe76c950a,// 49 PAY  46 

    0xd19475e2,// 50 PAY  47 

    0xcb05a472,// 51 PAY  48 

    0xe3d4755b,// 52 PAY  49 

    0x0f0bf1be,// 53 PAY  50 

    0x880ea952,// 54 PAY  51 

    0x0bbe7764,// 55 PAY  52 

    0x36cdcc2d,// 56 PAY  53 

    0x98941fb0,// 57 PAY  54 

    0x9284cebb,// 58 PAY  55 

    0x78c75c13,// 59 PAY  56 

    0xe8210d7d,// 60 PAY  57 

    0x3662fa6c,// 61 PAY  58 

    0x2331cf20,// 62 PAY  59 

    0xd58f1e91,// 63 PAY  60 

    0x1418d288,// 64 PAY  61 

    0xb9991659,// 65 PAY  62 

    0x562e4926,// 66 PAY  63 

    0x1d00aa99,// 67 PAY  64 

    0xfc4b579c,// 68 PAY  65 

    0xfb263012,// 69 PAY  66 

    0xb63d5c1a,// 70 PAY  67 

    0xa200ecbb,// 71 PAY  68 

    0x4ba48b16,// 72 PAY  69 

    0xaf669c76,// 73 PAY  70 

    0x82086938,// 74 PAY  71 

    0x5a05f6f9,// 75 PAY  72 

    0xb4259c9b,// 76 PAY  73 

    0xad31094d,// 77 PAY  74 

    0xcf799539,// 78 PAY  75 

    0x9bebd5e5,// 79 PAY  76 

    0x2d04d41b,// 80 PAY  77 

    0x51363a0f,// 81 PAY  78 

    0xf92493ed,// 82 PAY  79 

    0x93c62bae,// 83 PAY  80 

    0xf2c1c589,// 84 PAY  81 

    0x609835ad,// 85 PAY  82 

    0xe004eca6,// 86 PAY  83 

    0xd67bed45,// 87 PAY  84 

    0xd0caa9ce,// 88 PAY  85 

    0xcfc22428,// 89 PAY  86 

    0x54c413d8,// 90 PAY  87 

    0x07678020,// 91 PAY  88 

    0x36b37279,// 92 PAY  89 

    0x3541c84e,// 93 PAY  90 

    0x23283abb,// 94 PAY  91 

    0x81e950bb,// 95 PAY  92 

    0xb1545b81,// 96 PAY  93 

    0xa5b24a35,// 97 PAY  94 

    0xedfbaa33,// 98 PAY  95 

    0xfe1a7f41,// 99 PAY  96 

    0x888c1ef4,// 100 PAY  97 

    0x79fafc92,// 101 PAY  98 

    0x84791732,// 102 PAY  99 

    0x4f3b604c,// 103 PAY 100 

    0xca32f0aa,// 104 PAY 101 

    0xb3b9cf25,// 105 PAY 102 

    0xf75d23d7,// 106 PAY 103 

    0xe3f09c0b,// 107 PAY 104 

    0x13752c22,// 108 PAY 105 

    0x4a120f9d,// 109 PAY 106 

    0x4eb3252b,// 110 PAY 107 

    0xf17d190c,// 111 PAY 108 

    0x6759478b,// 112 PAY 109 

    0xd83f6554,// 113 PAY 110 

    0xa8c2c6ba,// 114 PAY 111 

    0x4acb9143,// 115 PAY 112 

    0xb021590f,// 116 PAY 113 

    0xa7ac0c61,// 117 PAY 114 

    0xa36afd51,// 118 PAY 115 

    0x30fce586,// 119 PAY 116 

    0xf87b61ca,// 120 PAY 117 

    0x7f67dbb8,// 121 PAY 118 

    0x4825f0d6,// 122 PAY 119 

    0xf9c15b92,// 123 PAY 120 

    0x4b955d8d,// 124 PAY 121 

    0x83ed81f2,// 125 PAY 122 

    0x92a53a85,// 126 PAY 123 

    0xcb532586,// 127 PAY 124 

    0x507b7041,// 128 PAY 125 

    0xfe111aef,// 129 PAY 126 

    0x7d4028b9,// 130 PAY 127 

    0x7160de24,// 131 PAY 128 

    0x0dc9d3f5,// 132 PAY 129 

    0x8fdd8f8a,// 133 PAY 130 

    0x138fae1b,// 134 PAY 131 

    0x05579e81,// 135 PAY 132 

    0xb031ed10,// 136 PAY 133 

    0xae054342,// 137 PAY 134 

    0xd8087c26,// 138 PAY 135 

    0x3dd09951,// 139 PAY 136 

    0x6eb1d89d,// 140 PAY 137 

    0xeb237666,// 141 PAY 138 

    0x5243a90b,// 142 PAY 139 

    0x24b82e85,// 143 PAY 140 

    0xaac5c0a7,// 144 PAY 141 

    0x617c4f2d,// 145 PAY 142 

    0x299fa37f,// 146 PAY 143 

    0x77453255,// 147 PAY 144 

    0xf7b3f828,// 148 PAY 145 

    0x7f729f94,// 149 PAY 146 

    0x086adaf2,// 150 PAY 147 

    0xe4127c72,// 151 PAY 148 

    0x2e7b8ad5,// 152 PAY 149 

    0x900bcfb1,// 153 PAY 150 

    0x952417d7,// 154 PAY 151 

    0x95da305c,// 155 PAY 152 

    0x6a82e2ac,// 156 PAY 153 

    0x268884c7,// 157 PAY 154 

    0xa4f669df,// 158 PAY 155 

    0x3ba2f808,// 159 PAY 156 

    0x68c5e41a,// 160 PAY 157 

    0x516d0a7f,// 161 PAY 158 

    0xa5bb8720,// 162 PAY 159 

    0x2ed8e7ab,// 163 PAY 160 

    0x6855b92b,// 164 PAY 161 

    0x98f0eea8,// 165 PAY 162 

    0x49ec9df6,// 166 PAY 163 

    0x85442b73,// 167 PAY 164 

    0x2727d963,// 168 PAY 165 

    0xd294083a,// 169 PAY 166 

    0x88a024f0,// 170 PAY 167 

    0x8ef5144d,// 171 PAY 168 

    0xd8f103e2,// 172 PAY 169 

    0x6442f7b6,// 173 PAY 170 

    0x5a384eb0,// 174 PAY 171 

    0xe80ec9b7,// 175 PAY 172 

    0x06da3be2,// 176 PAY 173 

    0xd5f53da5,// 177 PAY 174 

    0xdccd3519,// 178 PAY 175 

    0x524162da,// 179 PAY 176 

    0x1d1f3724,// 180 PAY 177 

    0xdd481c0f,// 181 PAY 178 

    0xd3a45c46,// 182 PAY 179 

    0xcab54669,// 183 PAY 180 

    0x7159f035,// 184 PAY 181 

    0x99126524,// 185 PAY 182 

    0xc60b3eb6,// 186 PAY 183 

    0x07119e51,// 187 PAY 184 

    0x6826dac5,// 188 PAY 185 

    0x8f94be1e,// 189 PAY 186 

    0x38b218b7,// 190 PAY 187 

    0x6de88dbb,// 191 PAY 188 

    0x9c94547a,// 192 PAY 189 

    0x089d1988,// 193 PAY 190 

    0xa21c8b0a,// 194 PAY 191 

    0x46b51503,// 195 PAY 192 

    0x831bd5da,// 196 PAY 193 

    0x4dcd7299,// 197 PAY 194 

    0xe8335c1f,// 198 PAY 195 

    0x155f47f1,// 199 PAY 196 

    0xd81b16c3,// 200 PAY 197 

    0xe049ea6f,// 201 PAY 198 

    0x0fc1f4d2,// 202 PAY 199 

    0x10ad1209,// 203 PAY 200 

    0x3e0c1ea6,// 204 PAY 201 

    0x80339bca,// 205 PAY 202 

    0xbe63d237,// 206 PAY 203 

    0x291036a0,// 207 PAY 204 

    0x02607235,// 208 PAY 205 

    0xc834b844,// 209 PAY 206 

    0x27e94cc0,// 210 PAY 207 

    0xa966b28d,// 211 PAY 208 

    0x99cc57f3,// 212 PAY 209 

    0xce67aa5f,// 213 PAY 210 

    0xb4a97d6e,// 214 PAY 211 

    0x10e0756b,// 215 PAY 212 

    0x7b762e47,// 216 PAY 213 

    0xb70c132e,// 217 PAY 214 

    0x938d56c4,// 218 PAY 215 

    0x54707b4b,// 219 PAY 216 

    0x3b8e71ed,// 220 PAY 217 

    0x63721776,// 221 PAY 218 

    0xdc5bc249,// 222 PAY 219 

    0xa57d50e0,// 223 PAY 220 

    0x9cdce3e0,// 224 PAY 221 

    0x26d10e72,// 225 PAY 222 

    0x3a81973e,// 226 PAY 223 

    0x14f69174,// 227 PAY 224 

    0xb9fc0c4a,// 228 PAY 225 

    0xa28a7e99,// 229 PAY 226 

    0xb2ecfda2,// 230 PAY 227 

    0x18e7656a,// 231 PAY 228 

    0x94aeb1c5,// 232 PAY 229 

    0xf724dd62,// 233 PAY 230 

    0x9250b7cb,// 234 PAY 231 

    0xfd0a9d4e,// 235 PAY 232 

    0x76b4ffe6,// 236 PAY 233 

    0x81f87e59,// 237 PAY 234 

    0x31b0dd90,// 238 PAY 235 

    0xc62ae5c9,// 239 PAY 236 

    0x4bbeb28a,// 240 PAY 237 

    0x88b534bf,// 241 PAY 238 

    0xec765fec,// 242 PAY 239 

    0x18c9a205,// 243 PAY 240 

    0xd8e707b9,// 244 PAY 241 

    0x63cb4476,// 245 PAY 242 

    0xd5cea86a,// 246 PAY 243 

    0x0b906469,// 247 PAY 244 

    0x4549d445,// 248 PAY 245 

    0xe962031b,// 249 PAY 246 

    0x1dab833d,// 250 PAY 247 

    0xc7a18535,// 251 PAY 248 

    0x2d534c4c,// 252 PAY 249 

    0x9dc04ab0,// 253 PAY 250 

    0x789cb33e,// 254 PAY 251 

    0xe6694bc3,// 255 PAY 252 

    0x3486ca4b,// 256 PAY 253 

    0xd54a43b1,// 257 PAY 254 

    0x9d95f4cc,// 258 PAY 255 

    0xf62c2e1a,// 259 PAY 256 

    0x0b761e96,// 260 PAY 257 

    0x4558c712,// 261 PAY 258 

    0x49f9d23e,// 262 PAY 259 

    0xc78482a9,// 263 PAY 260 

    0x30ede6ba,// 264 PAY 261 

    0x09a01f4b,// 265 PAY 262 

    0x475b0f71,// 266 PAY 263 

    0x042ebbaa,// 267 PAY 264 

    0x06caedc4,// 268 PAY 265 

    0x8898438c,// 269 PAY 266 

    0xbc309fe8,// 270 PAY 267 

    0x13160585,// 271 PAY 268 

    0x08689656,// 272 PAY 269 

    0xb8f045cd,// 273 PAY 270 

    0x27ed3198,// 274 PAY 271 

    0x29d85e52,// 275 PAY 272 

    0xf2446ade,// 276 PAY 273 

    0x11e3eda5,// 277 PAY 274 

    0xeeabd191,// 278 PAY 275 

    0x63d6dad5,// 279 PAY 276 

    0x82ec1960,// 280 PAY 277 

    0x71e5381c,// 281 PAY 278 

    0x2509fed7,// 282 PAY 279 

    0xf1a2aaa5,// 283 PAY 280 

    0xb927ea9c,// 284 PAY 281 

    0x228de553,// 285 PAY 282 

    0x856c8e9d,// 286 PAY 283 

    0xc914a76f,// 287 PAY 284 

    0x70133578,// 288 PAY 285 

    0x0ff65093,// 289 PAY 286 

    0x929a03b3,// 290 PAY 287 

    0xeb14063e,// 291 PAY 288 

    0x31fcd461,// 292 PAY 289 

    0x51851d12,// 293 PAY 290 

    0xdd49d5e4,// 294 PAY 291 

    0xe8d52e5f,// 295 PAY 292 

    0x2aa9d45a,// 296 PAY 293 

    0x46f924ec,// 297 PAY 294 

    0xb69bb809,// 298 PAY 295 

    0xbe756ad4,// 299 PAY 296 

    0x16f33fa5,// 300 PAY 297 

    0xfc647f43,// 301 PAY 298 

    0x7d6db0e3,// 302 PAY 299 

    0xeb0f0130,// 303 PAY 300 

    0x4da1852c,// 304 PAY 301 

    0xbfc9d6cf,// 305 PAY 302 

    0x607f496a,// 306 PAY 303 

    0x85b4395c,// 307 PAY 304 

    0xe0936c50,// 308 PAY 305 

    0x57f3473f,// 309 PAY 306 

    0xe019ceb2,// 310 PAY 307 

    0xfd89c236,// 311 PAY 308 

    0x00ff0f10,// 312 PAY 309 

    0x5cbc9958,// 313 PAY 310 

    0x89f67140,// 314 PAY 311 

    0x433c61c8,// 315 PAY 312 

    0xc9571530,// 316 PAY 313 

    0xe6a92fd0,// 317 PAY 314 

    0xa33a3290,// 318 PAY 315 

    0xfcf97e9f,// 319 PAY 316 

    0xc35bec09,// 320 PAY 317 

    0x6d11997e,// 321 PAY 318 

    0xa2aaf1a6,// 322 PAY 319 

    0x5fc7e900,// 323 PAY 320 

    0x57a39e1f,// 324 PAY 321 

    0xe629870d,// 325 PAY 322 

    0x63f33559,// 326 PAY 323 

    0x3eff6dc3,// 327 PAY 324 

    0x1984b4a7,// 328 PAY 325 

    0xa1ae0ac7,// 329 PAY 326 

    0xf9c77b9e,// 330 PAY 327 

    0xaf1e7819,// 331 PAY 328 

    0xe6f5b17f,// 332 PAY 329 

    0x2a01a165,// 333 PAY 330 

    0xbffc4823,// 334 PAY 331 

    0xe423c42f,// 335 PAY 332 

    0x637951d9,// 336 PAY 333 

    0x64edfa52,// 337 PAY 334 

    0xd5342673,// 338 PAY 335 

    0xc795f10c,// 339 PAY 336 

    0x051a4197,// 340 PAY 337 

    0xdfea2f35,// 341 PAY 338 

    0x12c3a629,// 342 PAY 339 

    0x886a29ac,// 343 PAY 340 

    0x4124472c,// 344 PAY 341 

    0xb57582ab,// 345 PAY 342 

    0x8f6c6700,// 346 PAY 343 

    0x8d77351e,// 347 PAY 344 

    0xd91d2401,// 348 PAY 345 

    0x8caa96ad,// 349 PAY 346 

    0xa729da22,// 350 PAY 347 

    0x121223a3,// 351 PAY 348 

    0xf21f1a29,// 352 PAY 349 

    0x08943e93,// 353 PAY 350 

    0xffef8323,// 354 PAY 351 

    0x54d5c59a,// 355 PAY 352 

    0xab9863e8,// 356 PAY 353 

    0xf5346aaa,// 357 PAY 354 

    0xc5d7f2f9,// 358 PAY 355 

    0x14588d96,// 359 PAY 356 

    0xe7cd651c,// 360 PAY 357 

    0x62ab6d51,// 361 PAY 358 

    0x2444cce5,// 362 PAY 359 

    0x979c849d,// 363 PAY 360 

    0xa775b95d,// 364 PAY 361 

    0x55ebec01,// 365 PAY 362 

    0x79c964b7,// 366 PAY 363 

    0xc8748589,// 367 PAY 364 

    0x70a01c25,// 368 PAY 365 

    0xbee2341d,// 369 PAY 366 

    0x37819548,// 370 PAY 367 

    0xc4d8c738,// 371 PAY 368 

    0xc4f0270a,// 372 PAY 369 

    0x4077a65c,// 373 PAY 370 

    0x81c49666,// 374 PAY 371 

    0x6b177c6e,// 375 PAY 372 

    0x0b3639f8,// 376 PAY 373 

    0x37a4b143,// 377 PAY 374 

    0x9c5ad977,// 378 PAY 375 

    0x57c4318b,// 379 PAY 376 

    0x920b5aba,// 380 PAY 377 

    0xd8886258,// 381 PAY 378 

    0xdd9d7753,// 382 PAY 379 

    0xa323a810,// 383 PAY 380 

    0x00131494,// 384 PAY 381 

    0x3b07fcf9,// 385 PAY 382 

    0x4c16a3ba,// 386 PAY 383 

    0x9af9414b,// 387 PAY 384 

    0x01e8b89c,// 388 PAY 385 

    0x33809159,// 389 PAY 386 

    0x27c0765c,// 390 PAY 387 

    0xbb9235bb,// 391 PAY 388 

    0x8027230c,// 392 PAY 389 

    0xed202f82,// 393 PAY 390 

    0x9af49745,// 394 PAY 391 

    0x0a547460,// 395 PAY 392 

    0xef1607b4,// 396 PAY 393 

    0x08b3c281,// 397 PAY 394 

    0x2255244e,// 398 PAY 395 

    0x192c327d,// 399 PAY 396 

    0x3910f8a1,// 400 PAY 397 

    0x46d7fcc0,// 401 PAY 398 

    0xe5c4fb88,// 402 PAY 399 

    0x670b3600,// 403 PAY 400 

    0x5265df9f,// 404 PAY 401 

    0x9e915a4a,// 405 PAY 402 

    0xb589a55d,// 406 PAY 403 

    0x340d15a4,// 407 PAY 404 

    0x7640e7ca,// 408 PAY 405 

    0x6b1e282e,// 409 PAY 406 

    0xeb226398,// 410 PAY 407 

    0x4a88ea3d,// 411 PAY 408 

    0xee0b78f3,// 412 PAY 409 

    0x0a102052,// 413 PAY 410 

    0xfe35d828,// 414 PAY 411 

    0xfa142d8c,// 415 PAY 412 

    0x76293948,// 416 PAY 413 

    0x0446e56b,// 417 PAY 414 

    0x3cfbca72,// 418 PAY 415 

    0xf7d1d709,// 419 PAY 416 

    0x6e1013c1,// 420 PAY 417 

    0x055aec7a,// 421 PAY 418 

    0x285a836d,// 422 PAY 419 

    0xea3a772d,// 423 PAY 420 

    0x65eb5deb,// 424 PAY 421 

    0x013f08b8,// 425 PAY 422 

    0x59b1a2f4,// 426 PAY 423 

    0x84ee08eb,// 427 PAY 424 

    0xa95bf4cd,// 428 PAY 425 

    0x8fd2a538,// 429 PAY 426 

    0x2fa49aef,// 430 PAY 427 

    0x4aa7bb77,// 431 PAY 428 

    0x80584b41,// 432 PAY 429 

    0xc88c4446,// 433 PAY 430 

    0xf1808ac1,// 434 PAY 431 

    0x40227348,// 435 PAY 432 

    0x68418129,// 436 PAY 433 

    0x3da18e54,// 437 PAY 434 

    0xe2fdcbfd,// 438 PAY 435 

    0xfd77fb0c,// 439 PAY 436 

    0x97f16fce,// 440 PAY 437 

    0x704792fe,// 441 PAY 438 

    0xf02a9f52,// 442 PAY 439 

    0x7108cdf6,// 443 PAY 440 

    0xca38bd16,// 444 PAY 441 

    0xe3bd9b07,// 445 PAY 442 

    0xe0738a5d,// 446 PAY 443 

    0x7a0608f4,// 447 PAY 444 

    0x86e3cae6,// 448 PAY 445 

    0x9a198333,// 449 PAY 446 

    0x749ad119,// 450 PAY 447 

    0x567ddabe,// 451 PAY 448 

    0xd7b6b8e6,// 452 PAY 449 

    0x132c3b49,// 453 PAY 450 

    0xca34b149,// 454 PAY 451 

    0xf40893b4,// 455 PAY 452 

    0x53de108f,// 456 PAY 453 

    0x3bc77048,// 457 PAY 454 

    0xaecda5d8,// 458 PAY 455 

    0x4215625d,// 459 PAY 456 

    0x343c7991,// 460 PAY 457 

    0xfb8bc66e,// 461 PAY 458 

    0xf06cc44c,// 462 PAY 459 

    0x504caddf,// 463 PAY 460 

    0x4a3bb9c4,// 464 PAY 461 

    0x1734103e,// 465 PAY 462 

    0xc80718b6,// 466 PAY 463 

/// HASH is  12 bytes 

    0xf1a2aaa5,// 467 HSH   1 

    0xb927ea9c,// 468 HSH   2 

    0x228de553,// 469 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 33 

/// STA pkt_idx        : 109 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x19 

    0x01b41921 // 470 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt55_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 167 words. 

/// BDA size     is 662 (0x296) 

/// BDA id       is 0x49d1 

    0x029649d1,// 3 BDA   1 

/// PAY Generic Data size   : 662 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xb521271b,// 4 PAY   1 

    0x37785e81,// 5 PAY   2 

    0x2dd2e6b6,// 6 PAY   3 

    0x54edfa29,// 7 PAY   4 

    0x601a49a2,// 8 PAY   5 

    0x503b972f,// 9 PAY   6 

    0x4bf2f32d,// 10 PAY   7 

    0x83e8b926,// 11 PAY   8 

    0xf518f492,// 12 PAY   9 

    0x7c259850,// 13 PAY  10 

    0xba1a0c66,// 14 PAY  11 

    0xa2936ee5,// 15 PAY  12 

    0x52e357a6,// 16 PAY  13 

    0x7a1a9bc1,// 17 PAY  14 

    0x72434800,// 18 PAY  15 

    0xe9771ceb,// 19 PAY  16 

    0xf04681b2,// 20 PAY  17 

    0x36c1b3f9,// 21 PAY  18 

    0x76d7d013,// 22 PAY  19 

    0xf2251f7a,// 23 PAY  20 

    0x10df8a5d,// 24 PAY  21 

    0xac7f5792,// 25 PAY  22 

    0xdf39500e,// 26 PAY  23 

    0xd95ce01e,// 27 PAY  24 

    0x961db0f9,// 28 PAY  25 

    0x2b7c13cf,// 29 PAY  26 

    0xe951e51b,// 30 PAY  27 

    0xad15c4bb,// 31 PAY  28 

    0x362fab60,// 32 PAY  29 

    0x58b178ed,// 33 PAY  30 

    0x6f2b381c,// 34 PAY  31 

    0xf1206006,// 35 PAY  32 

    0x565f1929,// 36 PAY  33 

    0x9c211359,// 37 PAY  34 

    0xbb85c0a3,// 38 PAY  35 

    0x2d8899c6,// 39 PAY  36 

    0xd1620b92,// 40 PAY  37 

    0xcb1cbd34,// 41 PAY  38 

    0x0a546adc,// 42 PAY  39 

    0x7f329a48,// 43 PAY  40 

    0x65f5c345,// 44 PAY  41 

    0xbbb475ec,// 45 PAY  42 

    0xdbb7c66e,// 46 PAY  43 

    0xa00951ed,// 47 PAY  44 

    0x51abebe9,// 48 PAY  45 

    0x2ba2bff0,// 49 PAY  46 

    0x95d9222a,// 50 PAY  47 

    0x84a54c20,// 51 PAY  48 

    0xb4bbfd8c,// 52 PAY  49 

    0xfbb495a9,// 53 PAY  50 

    0xc859983d,// 54 PAY  51 

    0x14abac79,// 55 PAY  52 

    0x570164ee,// 56 PAY  53 

    0xbae1f14c,// 57 PAY  54 

    0x1f56759c,// 58 PAY  55 

    0x4f2f3073,// 59 PAY  56 

    0xfcdbea45,// 60 PAY  57 

    0xb9521a34,// 61 PAY  58 

    0x568bc973,// 62 PAY  59 

    0x07170d01,// 63 PAY  60 

    0x2b418779,// 64 PAY  61 

    0x9df8c4f3,// 65 PAY  62 

    0x74638660,// 66 PAY  63 

    0xe2390ca0,// 67 PAY  64 

    0x331e4676,// 68 PAY  65 

    0xe5c01249,// 69 PAY  66 

    0x1c9ae511,// 70 PAY  67 

    0x0243124e,// 71 PAY  68 

    0x372fab29,// 72 PAY  69 

    0x51636147,// 73 PAY  70 

    0xa69a0bb2,// 74 PAY  71 

    0xd8028d9d,// 75 PAY  72 

    0x3de03004,// 76 PAY  73 

    0xe44eff91,// 77 PAY  74 

    0x52dea5f6,// 78 PAY  75 

    0xa058c052,// 79 PAY  76 

    0xdb85f737,// 80 PAY  77 

    0x3091e70a,// 81 PAY  78 

    0xee35e9b6,// 82 PAY  79 

    0x799ba375,// 83 PAY  80 

    0xb24e31f6,// 84 PAY  81 

    0xea66df9b,// 85 PAY  82 

    0x6b2caf27,// 86 PAY  83 

    0x3a3dccde,// 87 PAY  84 

    0xc50356f3,// 88 PAY  85 

    0x2902115b,// 89 PAY  86 

    0xe5559f0e,// 90 PAY  87 

    0x14093c5c,// 91 PAY  88 

    0xb78e919e,// 92 PAY  89 

    0x704065a5,// 93 PAY  90 

    0xe1664ec0,// 94 PAY  91 

    0x2a18f67f,// 95 PAY  92 

    0xed4f460d,// 96 PAY  93 

    0x7c5cfbbd,// 97 PAY  94 

    0xd07cbe76,// 98 PAY  95 

    0xbc01499c,// 99 PAY  96 

    0x4c8f69f1,// 100 PAY  97 

    0x846b01a8,// 101 PAY  98 

    0x6564d339,// 102 PAY  99 

    0x72a3a541,// 103 PAY 100 

    0xca9f8316,// 104 PAY 101 

    0xed78115a,// 105 PAY 102 

    0x31bc7eb8,// 106 PAY 103 

    0x1b3e1472,// 107 PAY 104 

    0xf521e154,// 108 PAY 105 

    0x8d183a2f,// 109 PAY 106 

    0x60916a25,// 110 PAY 107 

    0x58f773c6,// 111 PAY 108 

    0xa354c182,// 112 PAY 109 

    0x25b9df2a,// 113 PAY 110 

    0x927ac74b,// 114 PAY 111 

    0x093c8a95,// 115 PAY 112 

    0x30e1a09a,// 116 PAY 113 

    0x2c5b487b,// 117 PAY 114 

    0x41f86f40,// 118 PAY 115 

    0x440d9f5b,// 119 PAY 116 

    0x1d6baa44,// 120 PAY 117 

    0x9443395d,// 121 PAY 118 

    0xd5a6a006,// 122 PAY 119 

    0x8b1dd47d,// 123 PAY 120 

    0x66be5e5c,// 124 PAY 121 

    0x26fcd882,// 125 PAY 122 

    0x8b317986,// 126 PAY 123 

    0xbf9a0c5b,// 127 PAY 124 

    0x24226e19,// 128 PAY 125 

    0x7ea66bbe,// 129 PAY 126 

    0xb78f01ec,// 130 PAY 127 

    0xe4c597c7,// 131 PAY 128 

    0xe0c67425,// 132 PAY 129 

    0xaecb40d1,// 133 PAY 130 

    0xc701a0aa,// 134 PAY 131 

    0xad8aa9cf,// 135 PAY 132 

    0x53765bb3,// 136 PAY 133 

    0xead7f775,// 137 PAY 134 

    0xc862064a,// 138 PAY 135 

    0xaacb161f,// 139 PAY 136 

    0x29b7bd96,// 140 PAY 137 

    0x436e1ec7,// 141 PAY 138 

    0x25cade8f,// 142 PAY 139 

    0x372a07d3,// 143 PAY 140 

    0x3d1bd843,// 144 PAY 141 

    0x0e36ca37,// 145 PAY 142 

    0xd27b6518,// 146 PAY 143 

    0x762f6b2b,// 147 PAY 144 

    0xa5d22678,// 148 PAY 145 

    0x2a269adf,// 149 PAY 146 

    0xe8fe853a,// 150 PAY 147 

    0xdfaf1f9a,// 151 PAY 148 

    0xcc47ddd3,// 152 PAY 149 

    0x94c562e4,// 153 PAY 150 

    0xb258dad4,// 154 PAY 151 

    0x386ad73b,// 155 PAY 152 

    0xb42df9e2,// 156 PAY 153 

    0x933f904f,// 157 PAY 154 

    0xea18b336,// 158 PAY 155 

    0x4028a955,// 159 PAY 156 

    0xa211b234,// 160 PAY 157 

    0x2b4cfbbd,// 161 PAY 158 

    0x90b7a519,// 162 PAY 159 

    0x89a80d12,// 163 PAY 160 

    0x765e2b49,// 164 PAY 161 

    0x300d713c,// 165 PAY 162 

    0x663439d1,// 166 PAY 163 

    0xa0f4e67d,// 167 PAY 164 

    0xad86c10f,// 168 PAY 165 

    0x57bd0000,// 169 PAY 166 

/// STA is 1 words. 

/// STA num_pkts       : 215 

/// STA pkt_idx        : 91 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc4 

    0x016cc4d7 // 170 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt56_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 445 words. 

/// BDA size     is 1775 (0x6ef) 

/// BDA id       is 0x1b7b 

    0x06ef1b7b,// 3 BDA   1 

/// PAY Generic Data size   : 1775 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x4526946c,// 4 PAY   1 

    0xf6eb2320,// 5 PAY   2 

    0x1ed0040e,// 6 PAY   3 

    0xe2cc0e81,// 7 PAY   4 

    0xef047d57,// 8 PAY   5 

    0xa5336663,// 9 PAY   6 

    0xa88cdcc7,// 10 PAY   7 

    0xfeefeefb,// 11 PAY   8 

    0x18d79f37,// 12 PAY   9 

    0x413e0467,// 13 PAY  10 

    0xd3dcceb0,// 14 PAY  11 

    0x0cf0bb58,// 15 PAY  12 

    0x8efe28b5,// 16 PAY  13 

    0xa76829ef,// 17 PAY  14 

    0xb81e5aeb,// 18 PAY  15 

    0x80f739dd,// 19 PAY  16 

    0xdf6cabdb,// 20 PAY  17 

    0x76f13eea,// 21 PAY  18 

    0x9972f55e,// 22 PAY  19 

    0x78059dc7,// 23 PAY  20 

    0xa6ef432c,// 24 PAY  21 

    0x35b175c6,// 25 PAY  22 

    0x7209e436,// 26 PAY  23 

    0x0a5f47cc,// 27 PAY  24 

    0x1b611666,// 28 PAY  25 

    0xfac60556,// 29 PAY  26 

    0x9415ba1d,// 30 PAY  27 

    0x21edeefc,// 31 PAY  28 

    0xc37d389f,// 32 PAY  29 

    0x7d3919e2,// 33 PAY  30 

    0x34f4a962,// 34 PAY  31 

    0xf36a445e,// 35 PAY  32 

    0xe7a039a1,// 36 PAY  33 

    0x0da98e23,// 37 PAY  34 

    0xf4e7cfe7,// 38 PAY  35 

    0xe82455c6,// 39 PAY  36 

    0xe2f2423e,// 40 PAY  37 

    0x8ec17bbf,// 41 PAY  38 

    0x6e74b7ed,// 42 PAY  39 

    0x6b8ce1cb,// 43 PAY  40 

    0xe51d276b,// 44 PAY  41 

    0x95830597,// 45 PAY  42 

    0x543bea12,// 46 PAY  43 

    0xb5fdd8c9,// 47 PAY  44 

    0x24537e2a,// 48 PAY  45 

    0x26c34361,// 49 PAY  46 

    0x2f0d632e,// 50 PAY  47 

    0x7c424e9a,// 51 PAY  48 

    0x2aad6c55,// 52 PAY  49 

    0x2f48797e,// 53 PAY  50 

    0x5193cf7f,// 54 PAY  51 

    0x2ba05977,// 55 PAY  52 

    0xe86f376e,// 56 PAY  53 

    0x8876e09e,// 57 PAY  54 

    0x46af98c4,// 58 PAY  55 

    0xfb5e0354,// 59 PAY  56 

    0xd110e1c6,// 60 PAY  57 

    0x5b26e713,// 61 PAY  58 

    0xa1c81203,// 62 PAY  59 

    0xf52ee4b1,// 63 PAY  60 

    0xae2c4e71,// 64 PAY  61 

    0x1008c50c,// 65 PAY  62 

    0x828fb30a,// 66 PAY  63 

    0xa4cddb96,// 67 PAY  64 

    0x5325caa1,// 68 PAY  65 

    0x5d82ab29,// 69 PAY  66 

    0x3cf36f3e,// 70 PAY  67 

    0xa023211f,// 71 PAY  68 

    0xa23042d4,// 72 PAY  69 

    0x017b7914,// 73 PAY  70 

    0x947accd9,// 74 PAY  71 

    0xa853ea24,// 75 PAY  72 

    0xbd2c63ba,// 76 PAY  73 

    0xc64c5b0e,// 77 PAY  74 

    0x62a0df6f,// 78 PAY  75 

    0x4015c7e5,// 79 PAY  76 

    0xfc659c5c,// 80 PAY  77 

    0x6b2bafa0,// 81 PAY  78 

    0x6f06d844,// 82 PAY  79 

    0x2a8ac77a,// 83 PAY  80 

    0x331caf78,// 84 PAY  81 

    0x8f29bfd3,// 85 PAY  82 

    0xb02ef0a1,// 86 PAY  83 

    0x90eccd1a,// 87 PAY  84 

    0x8a1c8af9,// 88 PAY  85 

    0xc1dd7bcd,// 89 PAY  86 

    0x5728917e,// 90 PAY  87 

    0xdd2ad38c,// 91 PAY  88 

    0x90d607c0,// 92 PAY  89 

    0x4399326c,// 93 PAY  90 

    0x50ddc7ef,// 94 PAY  91 

    0xe64af232,// 95 PAY  92 

    0x2d677a82,// 96 PAY  93 

    0xba589eed,// 97 PAY  94 

    0xefb73abb,// 98 PAY  95 

    0x3b7bb95f,// 99 PAY  96 

    0xfa8ab1bd,// 100 PAY  97 

    0x4b5bd25e,// 101 PAY  98 

    0xefaf384d,// 102 PAY  99 

    0x149d9443,// 103 PAY 100 

    0xdfb6a7b1,// 104 PAY 101 

    0x1124dcf8,// 105 PAY 102 

    0xc9994ad3,// 106 PAY 103 

    0xfe4fdcef,// 107 PAY 104 

    0x7e39e7b3,// 108 PAY 105 

    0x92f9a509,// 109 PAY 106 

    0xcfd647f1,// 110 PAY 107 

    0xff6796bd,// 111 PAY 108 

    0x2c0853cd,// 112 PAY 109 

    0xd406acf5,// 113 PAY 110 

    0xe3396505,// 114 PAY 111 

    0x0bc0ccb0,// 115 PAY 112 

    0x65fc5a67,// 116 PAY 113 

    0x15dcb9bd,// 117 PAY 114 

    0x853a3a35,// 118 PAY 115 

    0x5a6b73aa,// 119 PAY 116 

    0xc9d4a20c,// 120 PAY 117 

    0x86e351c6,// 121 PAY 118 

    0x3c7689ac,// 122 PAY 119 

    0xffcc2ab1,// 123 PAY 120 

    0xc20146bf,// 124 PAY 121 

    0x3dc3b343,// 125 PAY 122 

    0x8797a425,// 126 PAY 123 

    0x4edeed97,// 127 PAY 124 

    0xf4c20c9f,// 128 PAY 125 

    0x43a17154,// 129 PAY 126 

    0xd2912fe1,// 130 PAY 127 

    0x02ac97d1,// 131 PAY 128 

    0xa2270511,// 132 PAY 129 

    0xcb6f63a0,// 133 PAY 130 

    0xd0ebc8ac,// 134 PAY 131 

    0x04dd2ee3,// 135 PAY 132 

    0x51c069ed,// 136 PAY 133 

    0x2da9b980,// 137 PAY 134 

    0x3e068fb9,// 138 PAY 135 

    0x3d7ff58a,// 139 PAY 136 

    0x9f4d5ca9,// 140 PAY 137 

    0x110ca208,// 141 PAY 138 

    0xcb634bea,// 142 PAY 139 

    0x88cf0dfd,// 143 PAY 140 

    0xc6d712c8,// 144 PAY 141 

    0x2576d0dd,// 145 PAY 142 

    0xaa9ab55f,// 146 PAY 143 

    0x02dffebf,// 147 PAY 144 

    0x99c28d73,// 148 PAY 145 

    0x0ee43697,// 149 PAY 146 

    0x9c65e1c1,// 150 PAY 147 

    0x12657727,// 151 PAY 148 

    0x85122c6d,// 152 PAY 149 

    0x8c5d7935,// 153 PAY 150 

    0x23733a1b,// 154 PAY 151 

    0x4dd6176b,// 155 PAY 152 

    0x84e86b03,// 156 PAY 153 

    0x35fad14d,// 157 PAY 154 

    0xde36777a,// 158 PAY 155 

    0x3f627749,// 159 PAY 156 

    0x762f6d3b,// 160 PAY 157 

    0x34acbd23,// 161 PAY 158 

    0x7d01d6b7,// 162 PAY 159 

    0x6675e1ba,// 163 PAY 160 

    0x9e002392,// 164 PAY 161 

    0x5064b2db,// 165 PAY 162 

    0x7132ab41,// 166 PAY 163 

    0x647a50d7,// 167 PAY 164 

    0xaaaa4c42,// 168 PAY 165 

    0x89161efa,// 169 PAY 166 

    0x2eda2e8d,// 170 PAY 167 

    0xbdf82f21,// 171 PAY 168 

    0x5715bda0,// 172 PAY 169 

    0x6aa3b535,// 173 PAY 170 

    0xc3752a4c,// 174 PAY 171 

    0xe14538a5,// 175 PAY 172 

    0xd8dd4e1f,// 176 PAY 173 

    0x7385c82f,// 177 PAY 174 

    0xd0faa865,// 178 PAY 175 

    0xd045ddf5,// 179 PAY 176 

    0xa65f3cbe,// 180 PAY 177 

    0x262d47d6,// 181 PAY 178 

    0x74e6b16b,// 182 PAY 179 

    0x3d8be330,// 183 PAY 180 

    0x66788f46,// 184 PAY 181 

    0x36a80851,// 185 PAY 182 

    0xb840c7d2,// 186 PAY 183 

    0x63dc2db7,// 187 PAY 184 

    0x29765269,// 188 PAY 185 

    0xf198b6e3,// 189 PAY 186 

    0x9aeccdd1,// 190 PAY 187 

    0xde51760d,// 191 PAY 188 

    0x3622210e,// 192 PAY 189 

    0x8adf8c24,// 193 PAY 190 

    0xa92f28d4,// 194 PAY 191 

    0xfa1b6cb5,// 195 PAY 192 

    0x58c848f1,// 196 PAY 193 

    0x1cb0fbef,// 197 PAY 194 

    0x19715ad0,// 198 PAY 195 

    0xdf19ba9e,// 199 PAY 196 

    0x99435ba1,// 200 PAY 197 

    0x09c406e7,// 201 PAY 198 

    0xbf8d606c,// 202 PAY 199 

    0xeafb3074,// 203 PAY 200 

    0xdac0e1aa,// 204 PAY 201 

    0xf2668a9c,// 205 PAY 202 

    0xd25f4298,// 206 PAY 203 

    0x124d7a6f,// 207 PAY 204 

    0x1e1c98f4,// 208 PAY 205 

    0x5a2cce5f,// 209 PAY 206 

    0xd3b319f5,// 210 PAY 207 

    0xb8c5a1ae,// 211 PAY 208 

    0xff509690,// 212 PAY 209 

    0xb9de1799,// 213 PAY 210 

    0x83392068,// 214 PAY 211 

    0x0ba8945c,// 215 PAY 212 

    0xce47aa8f,// 216 PAY 213 

    0xb9366917,// 217 PAY 214 

    0x74fc93d4,// 218 PAY 215 

    0x79a4dbc0,// 219 PAY 216 

    0x774e395a,// 220 PAY 217 

    0x017d8734,// 221 PAY 218 

    0xbed75644,// 222 PAY 219 

    0x1aaa54ac,// 223 PAY 220 

    0x1e24cb5e,// 224 PAY 221 

    0x4df1c68c,// 225 PAY 222 

    0x10a1198d,// 226 PAY 223 

    0x72f006a0,// 227 PAY 224 

    0x15aa3dd3,// 228 PAY 225 

    0xd0bd5979,// 229 PAY 226 

    0xf46d3041,// 230 PAY 227 

    0x6269e283,// 231 PAY 228 

    0xe094f57c,// 232 PAY 229 

    0x48277a8a,// 233 PAY 230 

    0x19ee5581,// 234 PAY 231 

    0x538fcbb7,// 235 PAY 232 

    0xd288099a,// 236 PAY 233 

    0x366d4715,// 237 PAY 234 

    0x7acb15cd,// 238 PAY 235 

    0x715ae93a,// 239 PAY 236 

    0x45ea012d,// 240 PAY 237 

    0xfdbf1603,// 241 PAY 238 

    0x1d0e3b1b,// 242 PAY 239 

    0xbfeb38b0,// 243 PAY 240 

    0x4f39ecbd,// 244 PAY 241 

    0xdb33b1ef,// 245 PAY 242 

    0xc7c78dff,// 246 PAY 243 

    0x1b5ab9b5,// 247 PAY 244 

    0xec7dcc89,// 248 PAY 245 

    0xba7456c9,// 249 PAY 246 

    0x102d5b27,// 250 PAY 247 

    0x9b41aff1,// 251 PAY 248 

    0x202e5e52,// 252 PAY 249 

    0x494368fd,// 253 PAY 250 

    0xe98c3d65,// 254 PAY 251 

    0xc3c6711a,// 255 PAY 252 

    0x14dd4f88,// 256 PAY 253 

    0xdd8dd392,// 257 PAY 254 

    0x7e92421c,// 258 PAY 255 

    0x3f038c56,// 259 PAY 256 

    0x6b5d1a1f,// 260 PAY 257 

    0xb76ddc40,// 261 PAY 258 

    0x45384612,// 262 PAY 259 

    0x627ab9f9,// 263 PAY 260 

    0x4fb4003a,// 264 PAY 261 

    0xb51e3580,// 265 PAY 262 

    0x3e2bd124,// 266 PAY 263 

    0xe0503eb6,// 267 PAY 264 

    0x6740c454,// 268 PAY 265 

    0xd3698fc5,// 269 PAY 266 

    0xfbf6ef57,// 270 PAY 267 

    0x26b4b31e,// 271 PAY 268 

    0x5b905596,// 272 PAY 269 

    0xef983191,// 273 PAY 270 

    0x23fad8ab,// 274 PAY 271 

    0x2e105898,// 275 PAY 272 

    0xb7eda103,// 276 PAY 273 

    0xf8818495,// 277 PAY 274 

    0x3b5b3059,// 278 PAY 275 

    0x45517e6c,// 279 PAY 276 

    0x46afe29b,// 280 PAY 277 

    0x3be12720,// 281 PAY 278 

    0xf78d0c7a,// 282 PAY 279 

    0x15a396c7,// 283 PAY 280 

    0xaf928a02,// 284 PAY 281 

    0xe309f215,// 285 PAY 282 

    0xa8c7bcf2,// 286 PAY 283 

    0x164f7811,// 287 PAY 284 

    0xaa178344,// 288 PAY 285 

    0x677b2076,// 289 PAY 286 

    0x36922ad5,// 290 PAY 287 

    0xab974d7a,// 291 PAY 288 

    0xb61ae073,// 292 PAY 289 

    0x9ecf186b,// 293 PAY 290 

    0x5d3539a9,// 294 PAY 291 

    0xdaa4668b,// 295 PAY 292 

    0x9ee04161,// 296 PAY 293 

    0x4265173d,// 297 PAY 294 

    0xf684af78,// 298 PAY 295 

    0x0208e54d,// 299 PAY 296 

    0x8cfc6809,// 300 PAY 297 

    0x772031b8,// 301 PAY 298 

    0x2bc750d0,// 302 PAY 299 

    0xc21f603c,// 303 PAY 300 

    0xab2429aa,// 304 PAY 301 

    0x19a7fa21,// 305 PAY 302 

    0x34d2a9a8,// 306 PAY 303 

    0x120cbd0c,// 307 PAY 304 

    0xdd1c5070,// 308 PAY 305 

    0x387b0362,// 309 PAY 306 

    0x8bf0c22c,// 310 PAY 307 

    0x99bda609,// 311 PAY 308 

    0x3f665eb2,// 312 PAY 309 

    0xd9c58f0d,// 313 PAY 310 

    0x33a045ca,// 314 PAY 311 

    0x1a99d6e6,// 315 PAY 312 

    0x20d18847,// 316 PAY 313 

    0xcef0d38c,// 317 PAY 314 

    0x14f6e1a3,// 318 PAY 315 

    0x0dd4185e,// 319 PAY 316 

    0x2a18f428,// 320 PAY 317 

    0xe1210704,// 321 PAY 318 

    0x3011f85a,// 322 PAY 319 

    0xfd5ebac0,// 323 PAY 320 

    0xfcc7ba87,// 324 PAY 321 

    0x8d02ce03,// 325 PAY 322 

    0x28213f43,// 326 PAY 323 

    0x46122b55,// 327 PAY 324 

    0x70e6010a,// 328 PAY 325 

    0x577b346e,// 329 PAY 326 

    0x2537e152,// 330 PAY 327 

    0x19474045,// 331 PAY 328 

    0xd012a793,// 332 PAY 329 

    0x19c6c6e8,// 333 PAY 330 

    0x11cc0693,// 334 PAY 331 

    0x52927816,// 335 PAY 332 

    0x27658c7f,// 336 PAY 333 

    0xd10c54f6,// 337 PAY 334 

    0x0cb8d662,// 338 PAY 335 

    0x47f5d10f,// 339 PAY 336 

    0x9f42bbf4,// 340 PAY 337 

    0x0f1c70b5,// 341 PAY 338 

    0xbd47cd96,// 342 PAY 339 

    0xaaaecfb0,// 343 PAY 340 

    0xe235b1b6,// 344 PAY 341 

    0xf8f72729,// 345 PAY 342 

    0x334d447f,// 346 PAY 343 

    0xf4ba4762,// 347 PAY 344 

    0x9c223490,// 348 PAY 345 

    0xc0f6b057,// 349 PAY 346 

    0x684e9fa9,// 350 PAY 347 

    0x73df0956,// 351 PAY 348 

    0x62c40a03,// 352 PAY 349 

    0x320d453e,// 353 PAY 350 

    0x0f5ce665,// 354 PAY 351 

    0x9cf53070,// 355 PAY 352 

    0x0ab6a0d2,// 356 PAY 353 

    0x6e86d0eb,// 357 PAY 354 

    0x64399bc9,// 358 PAY 355 

    0x98f8215d,// 359 PAY 356 

    0x57134e80,// 360 PAY 357 

    0x992bbe33,// 361 PAY 358 

    0xc467122f,// 362 PAY 359 

    0x173a2382,// 363 PAY 360 

    0xd77a0c23,// 364 PAY 361 

    0x3c19f5a8,// 365 PAY 362 

    0x7d0fe6fc,// 366 PAY 363 

    0xc54aced9,// 367 PAY 364 

    0x4d6ec3c4,// 368 PAY 365 

    0x1e1d1fbd,// 369 PAY 366 

    0xf136f7b1,// 370 PAY 367 

    0x2969499e,// 371 PAY 368 

    0x3dfcea2b,// 372 PAY 369 

    0xa04f94f4,// 373 PAY 370 

    0x78da3853,// 374 PAY 371 

    0xe6610c2c,// 375 PAY 372 

    0x074e98c9,// 376 PAY 373 

    0xca6a3842,// 377 PAY 374 

    0x42dc99fa,// 378 PAY 375 

    0x057b86cd,// 379 PAY 376 

    0xae9c84ef,// 380 PAY 377 

    0x761ed5f5,// 381 PAY 378 

    0xec6cbd36,// 382 PAY 379 

    0x1d057f03,// 383 PAY 380 

    0x08ac5aee,// 384 PAY 381 

    0x4be08c25,// 385 PAY 382 

    0x28d5f877,// 386 PAY 383 

    0xd3327d47,// 387 PAY 384 

    0x768bddaa,// 388 PAY 385 

    0x1507ea36,// 389 PAY 386 

    0x8309cef9,// 390 PAY 387 

    0xd38cc3ff,// 391 PAY 388 

    0x7efb12f0,// 392 PAY 389 

    0x9ab18b37,// 393 PAY 390 

    0x46ccd746,// 394 PAY 391 

    0x98fbbec9,// 395 PAY 392 

    0x03621ece,// 396 PAY 393 

    0xb30c4b88,// 397 PAY 394 

    0x03c6d6b3,// 398 PAY 395 

    0x8df4cc49,// 399 PAY 396 

    0x051138fc,// 400 PAY 397 

    0x88131e7e,// 401 PAY 398 

    0x046ee948,// 402 PAY 399 

    0xb6dc9413,// 403 PAY 400 

    0x76455851,// 404 PAY 401 

    0xf77424bb,// 405 PAY 402 

    0xf6d907de,// 406 PAY 403 

    0x17efbbad,// 407 PAY 404 

    0xb2a1979a,// 408 PAY 405 

    0x1b58304d,// 409 PAY 406 

    0x47f7c17f,// 410 PAY 407 

    0xf4d4e62b,// 411 PAY 408 

    0x40439002,// 412 PAY 409 

    0x4511ea62,// 413 PAY 410 

    0x332a55fb,// 414 PAY 411 

    0x6b0210de,// 415 PAY 412 

    0x29090de7,// 416 PAY 413 

    0xab747626,// 417 PAY 414 

    0xa6e1f1e8,// 418 PAY 415 

    0x2a0ea840,// 419 PAY 416 

    0xe22c4c13,// 420 PAY 417 

    0x51ed1f1a,// 421 PAY 418 

    0x04009b36,// 422 PAY 419 

    0x35d1f277,// 423 PAY 420 

    0x509a00d5,// 424 PAY 421 

    0xd902e242,// 425 PAY 422 

    0x2e3fe150,// 426 PAY 423 

    0xd163f2ed,// 427 PAY 424 

    0xd7fe54af,// 428 PAY 425 

    0xe86b9010,// 429 PAY 426 

    0x27fe637c,// 430 PAY 427 

    0x228a2847,// 431 PAY 428 

    0xd78a19a7,// 432 PAY 429 

    0x59c9980b,// 433 PAY 430 

    0x03b7964d,// 434 PAY 431 

    0xe64d766f,// 435 PAY 432 

    0xb150b99e,// 436 PAY 433 

    0x61453874,// 437 PAY 434 

    0xa607656f,// 438 PAY 435 

    0xb132898b,// 439 PAY 436 

    0x42e84806,// 440 PAY 437 

    0x0dbbfaec,// 441 PAY 438 

    0xc7f1fa5d,// 442 PAY 439 

    0x86399c99,// 443 PAY 440 

    0xe3d86aaa,// 444 PAY 441 

    0x81cf7229,// 445 PAY 442 

    0xc99dec40,// 446 PAY 443 

    0xb14ec800,// 447 PAY 444 

/// STA is 1 words. 

/// STA num_pkts       : 160 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf7 

    0x02d1f7a0 // 448 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt57_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 88 words. 

/// BDA size     is 348 (0x15c) 

/// BDA id       is 0xf427 

    0x015cf427,// 3 BDA   1 

/// PAY Generic Data size   : 348 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x7c125e4b,// 4 PAY   1 

    0x90645a88,// 5 PAY   2 

    0x69d14eb7,// 6 PAY   3 

    0x9d306a99,// 7 PAY   4 

    0xb91f7aee,// 8 PAY   5 

    0x2055bb6b,// 9 PAY   6 

    0xd12721bd,// 10 PAY   7 

    0xacc21102,// 11 PAY   8 

    0x7a5409a6,// 12 PAY   9 

    0x60b5403d,// 13 PAY  10 

    0xb17fd639,// 14 PAY  11 

    0x032da145,// 15 PAY  12 

    0x100c5f5e,// 16 PAY  13 

    0x859a8441,// 17 PAY  14 

    0x3f2be19c,// 18 PAY  15 

    0x8432cbc8,// 19 PAY  16 

    0xd8c2cac5,// 20 PAY  17 

    0x7688ce28,// 21 PAY  18 

    0x36facf8d,// 22 PAY  19 

    0x9641db8d,// 23 PAY  20 

    0x3576dbce,// 24 PAY  21 

    0xeeae3c4f,// 25 PAY  22 

    0x4cf8bca1,// 26 PAY  23 

    0xb6440400,// 27 PAY  24 

    0xd30f1394,// 28 PAY  25 

    0x80416fff,// 29 PAY  26 

    0x05954491,// 30 PAY  27 

    0x8fcf4ad0,// 31 PAY  28 

    0x8bdce65f,// 32 PAY  29 

    0x82d6a16c,// 33 PAY  30 

    0x8d1d70b1,// 34 PAY  31 

    0x7ff2ca71,// 35 PAY  32 

    0x61ed566f,// 36 PAY  33 

    0x61ea316f,// 37 PAY  34 

    0x3dc94124,// 38 PAY  35 

    0x77a9fb7f,// 39 PAY  36 

    0xa91e251d,// 40 PAY  37 

    0x490e0854,// 41 PAY  38 

    0x00aac94b,// 42 PAY  39 

    0xd1934c76,// 43 PAY  40 

    0x70dc64ac,// 44 PAY  41 

    0x6ec49e3d,// 45 PAY  42 

    0x0cbf2b7b,// 46 PAY  43 

    0x7a6506af,// 47 PAY  44 

    0x8874bc24,// 48 PAY  45 

    0x1c9babf2,// 49 PAY  46 

    0x017f34dc,// 50 PAY  47 

    0xc70ac28d,// 51 PAY  48 

    0x5eae6864,// 52 PAY  49 

    0x05ee5a65,// 53 PAY  50 

    0x7ff53cf2,// 54 PAY  51 

    0x8b308183,// 55 PAY  52 

    0x45122251,// 56 PAY  53 

    0xca996956,// 57 PAY  54 

    0x87bbb6c5,// 58 PAY  55 

    0x295ef63e,// 59 PAY  56 

    0xd99b7912,// 60 PAY  57 

    0xf85f245f,// 61 PAY  58 

    0xef67b013,// 62 PAY  59 

    0x1d74fd5b,// 63 PAY  60 

    0x7f769afc,// 64 PAY  61 

    0xee9b6be9,// 65 PAY  62 

    0x06ed95a0,// 66 PAY  63 

    0x079c8aec,// 67 PAY  64 

    0x831db9c0,// 68 PAY  65 

    0x45b2ac0f,// 69 PAY  66 

    0x8808b070,// 70 PAY  67 

    0x60ba8921,// 71 PAY  68 

    0xedbaa98d,// 72 PAY  69 

    0x4045852b,// 73 PAY  70 

    0x3cb45833,// 74 PAY  71 

    0x9e7402a8,// 75 PAY  72 

    0x9b0fc8b7,// 76 PAY  73 

    0xb99563cf,// 77 PAY  74 

    0x226f9515,// 78 PAY  75 

    0xaa6347ab,// 79 PAY  76 

    0x2248597a,// 80 PAY  77 

    0x811afe32,// 81 PAY  78 

    0xfac936d8,// 82 PAY  79 

    0x96eff0e7,// 83 PAY  80 

    0xeb72f011,// 84 PAY  81 

    0x299871c1,// 85 PAY  82 

    0x2e2ba509,// 86 PAY  83 

    0xb1802a2e,// 87 PAY  84 

    0x8cf52429,// 88 PAY  85 

    0x30e90fe0,// 89 PAY  86 

    0xfb1859c0,// 90 PAY  87 

/// HASH is  12 bytes 

    0x295ef63e,// 91 HSH   1 

    0xd99b7912,// 92 HSH   2 

    0xf85f245f,// 93 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 231 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9 

    0x03dc09e7 // 94 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt58_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 113 words. 

/// BDA size     is 445 (0x1bd) 

/// BDA id       is 0x5042 

    0x01bd5042,// 3 BDA   1 

/// PAY Generic Data size   : 445 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xd43689a8,// 4 PAY   1 

    0xdfa8e53b,// 5 PAY   2 

    0x800c125b,// 6 PAY   3 

    0xe5667342,// 7 PAY   4 

    0xb799418a,// 8 PAY   5 

    0xb9957a6b,// 9 PAY   6 

    0x9a72b624,// 10 PAY   7 

    0x6f666bea,// 11 PAY   8 

    0xba65e8e2,// 12 PAY   9 

    0xc18acdba,// 13 PAY  10 

    0xf782a9ec,// 14 PAY  11 

    0x375954c8,// 15 PAY  12 

    0x84bfd531,// 16 PAY  13 

    0xf18ef6e9,// 17 PAY  14 

    0xda409491,// 18 PAY  15 

    0x7ae37729,// 19 PAY  16 

    0x66bc938c,// 20 PAY  17 

    0x5b6093ed,// 21 PAY  18 

    0xc33e7bf8,// 22 PAY  19 

    0x96ade1ba,// 23 PAY  20 

    0xa0f80b54,// 24 PAY  21 

    0x432d722f,// 25 PAY  22 

    0x856150af,// 26 PAY  23 

    0x691bae3d,// 27 PAY  24 

    0x5c8a3027,// 28 PAY  25 

    0x1a2160c7,// 29 PAY  26 

    0x79629c4a,// 30 PAY  27 

    0x2630b4ee,// 31 PAY  28 

    0x59ea5802,// 32 PAY  29 

    0x2777741f,// 33 PAY  30 

    0x6d40c1eb,// 34 PAY  31 

    0xe74e2e8d,// 35 PAY  32 

    0xa26d3c52,// 36 PAY  33 

    0x134ffca1,// 37 PAY  34 

    0x53f28cfe,// 38 PAY  35 

    0x2da4883f,// 39 PAY  36 

    0x3b04f30d,// 40 PAY  37 

    0xe9795da3,// 41 PAY  38 

    0xe74ece40,// 42 PAY  39 

    0xf2f84320,// 43 PAY  40 

    0xb094f228,// 44 PAY  41 

    0xea0d2f17,// 45 PAY  42 

    0x7a022012,// 46 PAY  43 

    0x1bbb5a5e,// 47 PAY  44 

    0x6d4220ab,// 48 PAY  45 

    0x77def4f1,// 49 PAY  46 

    0x1cb07d47,// 50 PAY  47 

    0xc696b580,// 51 PAY  48 

    0xfc430b08,// 52 PAY  49 

    0x62e3ca6f,// 53 PAY  50 

    0xa2097045,// 54 PAY  51 

    0xfeb2a760,// 55 PAY  52 

    0xfbb756be,// 56 PAY  53 

    0xfae33897,// 57 PAY  54 

    0x0faab7c9,// 58 PAY  55 

    0x57162137,// 59 PAY  56 

    0x0d153e80,// 60 PAY  57 

    0x90800f38,// 61 PAY  58 

    0x390d96b3,// 62 PAY  59 

    0x3504a721,// 63 PAY  60 

    0x16a314db,// 64 PAY  61 

    0x72ee30c1,// 65 PAY  62 

    0x1efa258e,// 66 PAY  63 

    0x83f2866d,// 67 PAY  64 

    0xfa87565c,// 68 PAY  65 

    0xc487c1c4,// 69 PAY  66 

    0x8660ad5a,// 70 PAY  67 

    0x4e7142bc,// 71 PAY  68 

    0x8cce6553,// 72 PAY  69 

    0x5fe5c2d5,// 73 PAY  70 

    0xb9fcf362,// 74 PAY  71 

    0x216b21ac,// 75 PAY  72 

    0xb37b5dca,// 76 PAY  73 

    0xf4e5899e,// 77 PAY  74 

    0xe5df0611,// 78 PAY  75 

    0xa572a5b0,// 79 PAY  76 

    0x34dae4bf,// 80 PAY  77 

    0xf858201f,// 81 PAY  78 

    0xb68a6c6c,// 82 PAY  79 

    0x71969a27,// 83 PAY  80 

    0xa788b181,// 84 PAY  81 

    0x56113af0,// 85 PAY  82 

    0x7cf10abd,// 86 PAY  83 

    0x43e7a022,// 87 PAY  84 

    0x69f17c4b,// 88 PAY  85 

    0x6db384b3,// 89 PAY  86 

    0x82d10f89,// 90 PAY  87 

    0xfd8be6f6,// 91 PAY  88 

    0x2de789d6,// 92 PAY  89 

    0x4f36b83d,// 93 PAY  90 

    0xf089fa2e,// 94 PAY  91 

    0x65d8de8d,// 95 PAY  92 

    0xbc9b7456,// 96 PAY  93 

    0xff30a335,// 97 PAY  94 

    0xe4298cbe,// 98 PAY  95 

    0x2b20e12d,// 99 PAY  96 

    0xf77c5f87,// 100 PAY  97 

    0xf3710784,// 101 PAY  98 

    0x500ff1e7,// 102 PAY  99 

    0x9cb00780,// 103 PAY 100 

    0xe8d90349,// 104 PAY 101 

    0x38233271,// 105 PAY 102 

    0x06b60277,// 106 PAY 103 

    0x82e5384c,// 107 PAY 104 

    0xc9a761a2,// 108 PAY 105 

    0xb80a4b43,// 109 PAY 106 

    0x9d4e32f9,// 110 PAY 107 

    0xe4f2e60c,// 111 PAY 108 

    0x3e278796,// 112 PAY 109 

    0xe7fae5d0,// 113 PAY 110 

    0xed97913e,// 114 PAY 111 

    0xe7000000,// 115 PAY 112 

/// STA is 1 words. 

/// STA num_pkts       : 207 

/// STA pkt_idx        : 160 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1e 

    0x02811ecf // 116 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt59_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 108 words. 

/// BDA size     is 426 (0x1aa) 

/// BDA id       is 0x1f49 

    0x01aa1f49,// 3 BDA   1 

/// PAY Generic Data size   : 426 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xd9ccd401,// 4 PAY   1 

    0x8388c9f2,// 5 PAY   2 

    0xbfd74f57,// 6 PAY   3 

    0xb51efb1b,// 7 PAY   4 

    0x9118f822,// 8 PAY   5 

    0xec66220d,// 9 PAY   6 

    0xb7689a5b,// 10 PAY   7 

    0xb51a21f3,// 11 PAY   8 

    0x8cdfcfb7,// 12 PAY   9 

    0x92921f0f,// 13 PAY  10 

    0x59ab18f4,// 14 PAY  11 

    0xaf23a8dc,// 15 PAY  12 

    0x4db33a1f,// 16 PAY  13 

    0xdd22b024,// 17 PAY  14 

    0xc81b750a,// 18 PAY  15 

    0x171e10fe,// 19 PAY  16 

    0x5789ad82,// 20 PAY  17 

    0x4b916119,// 21 PAY  18 

    0x1ecc4537,// 22 PAY  19 

    0xf07a36d2,// 23 PAY  20 

    0x698dfe55,// 24 PAY  21 

    0xd9679241,// 25 PAY  22 

    0xec95771c,// 26 PAY  23 

    0x2557f629,// 27 PAY  24 

    0xd1643b47,// 28 PAY  25 

    0x52652769,// 29 PAY  26 

    0xf0dff5c9,// 30 PAY  27 

    0x855173c9,// 31 PAY  28 

    0xf55343de,// 32 PAY  29 

    0xe9e4f042,// 33 PAY  30 

    0xe1e7e95f,// 34 PAY  31 

    0xc30d7ddc,// 35 PAY  32 

    0x6470d325,// 36 PAY  33 

    0x5d75348f,// 37 PAY  34 

    0x669c96ac,// 38 PAY  35 

    0x7849e450,// 39 PAY  36 

    0x26a5f65b,// 40 PAY  37 

    0xd72f2e67,// 41 PAY  38 

    0xa442a8e4,// 42 PAY  39 

    0x5bcb45c3,// 43 PAY  40 

    0x58803fbe,// 44 PAY  41 

    0xcd29184e,// 45 PAY  42 

    0xc008a7c3,// 46 PAY  43 

    0xa61394c0,// 47 PAY  44 

    0x7027d65f,// 48 PAY  45 

    0xf9baec54,// 49 PAY  46 

    0x44b7b8c4,// 50 PAY  47 

    0x4b144dee,// 51 PAY  48 

    0xeec9abad,// 52 PAY  49 

    0x1addc80c,// 53 PAY  50 

    0x08558e48,// 54 PAY  51 

    0x2a4c95b9,// 55 PAY  52 

    0xd49955fe,// 56 PAY  53 

    0xe2211b96,// 57 PAY  54 

    0xf7e716a3,// 58 PAY  55 

    0x0c406dd4,// 59 PAY  56 

    0x5c71feca,// 60 PAY  57 

    0xdb8f096a,// 61 PAY  58 

    0x5ec9e386,// 62 PAY  59 

    0xd5a54e97,// 63 PAY  60 

    0xa208ea1e,// 64 PAY  61 

    0x2bea1ce1,// 65 PAY  62 

    0x55336d50,// 66 PAY  63 

    0xf188feeb,// 67 PAY  64 

    0x5089d1c1,// 68 PAY  65 

    0xf63d6c35,// 69 PAY  66 

    0x1a98a999,// 70 PAY  67 

    0xd31461c6,// 71 PAY  68 

    0xbb7582ce,// 72 PAY  69 

    0xdaea63cd,// 73 PAY  70 

    0xb7ad95f3,// 74 PAY  71 

    0x6341aaf4,// 75 PAY  72 

    0x1ec9bfd8,// 76 PAY  73 

    0xdfc88bc6,// 77 PAY  74 

    0xe8cee7df,// 78 PAY  75 

    0xba769d6c,// 79 PAY  76 

    0xbdea5c5b,// 80 PAY  77 

    0x5b72fec4,// 81 PAY  78 

    0x3f518a88,// 82 PAY  79 

    0x58da4331,// 83 PAY  80 

    0xcafc03ae,// 84 PAY  81 

    0x64daa7a5,// 85 PAY  82 

    0xe9d1d9ba,// 86 PAY  83 

    0x699f4f30,// 87 PAY  84 

    0x5df8f692,// 88 PAY  85 

    0x7f1e99f2,// 89 PAY  86 

    0xa38eca57,// 90 PAY  87 

    0x6bfaaa1d,// 91 PAY  88 

    0x81ca5503,// 92 PAY  89 

    0x23fdfb31,// 93 PAY  90 

    0xf0eb5441,// 94 PAY  91 

    0xf760ae6f,// 95 PAY  92 

    0xa731b285,// 96 PAY  93 

    0x29bb1e47,// 97 PAY  94 

    0x16002382,// 98 PAY  95 

    0x8f48de30,// 99 PAY  96 

    0x58d39c0a,// 100 PAY  97 

    0x19c99e15,// 101 PAY  98 

    0x45bfccc9,// 102 PAY  99 

    0xcbb73df5,// 103 PAY 100 

    0x3eeaae28,// 104 PAY 101 

    0xdde8ccf4,// 105 PAY 102 

    0x30e1f2c6,// 106 PAY 103 

    0xdb1d8602,// 107 PAY 104 

    0x4426b905,// 108 PAY 105 

    0x0fb29755,// 109 PAY 106 

    0x59420000,// 110 PAY 107 

/// HASH is  12 bytes 

    0x5ec9e386,// 111 HSH   1 

    0xd5a54e97,// 112 HSH   2 

    0xa208ea1e,// 113 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 244 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7a 

    0x01107af4 // 114 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt60_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 449 words. 

/// BDA size     is 1792 (0x700) 

/// BDA id       is 0x6d6b 

    0x07006d6b,// 3 BDA   1 

/// PAY Generic Data size   : 1792 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x5e3840ec,// 4 PAY   1 

    0x9183336e,// 5 PAY   2 

    0x14ef95b0,// 6 PAY   3 

    0x8b1b50da,// 7 PAY   4 

    0x2ca7f920,// 8 PAY   5 

    0x5e13fbab,// 9 PAY   6 

    0x703ddc6e,// 10 PAY   7 

    0x861fb14e,// 11 PAY   8 

    0x4bd0626c,// 12 PAY   9 

    0x7b9e2dda,// 13 PAY  10 

    0xe152bbce,// 14 PAY  11 

    0xc0d26413,// 15 PAY  12 

    0xc8877f47,// 16 PAY  13 

    0x11a1c308,// 17 PAY  14 

    0x35f6b4bc,// 18 PAY  15 

    0x3c1dd95e,// 19 PAY  16 

    0x00a281eb,// 20 PAY  17 

    0xf86d069e,// 21 PAY  18 

    0x15c0217c,// 22 PAY  19 

    0x42f34c5b,// 23 PAY  20 

    0xfe480bef,// 24 PAY  21 

    0xba4b3af3,// 25 PAY  22 

    0xe4735943,// 26 PAY  23 

    0x08e29004,// 27 PAY  24 

    0x73b3e91b,// 28 PAY  25 

    0x5bea4234,// 29 PAY  26 

    0x7c52b4c4,// 30 PAY  27 

    0x5835966a,// 31 PAY  28 

    0xbc2c5c08,// 32 PAY  29 

    0xf1054306,// 33 PAY  30 

    0xf154e9df,// 34 PAY  31 

    0x75c36da1,// 35 PAY  32 

    0xd4d98458,// 36 PAY  33 

    0xb25f32f9,// 37 PAY  34 

    0xcd300a82,// 38 PAY  35 

    0xe58222e7,// 39 PAY  36 

    0xc8f8fd38,// 40 PAY  37 

    0x8eef51f3,// 41 PAY  38 

    0x9408dec8,// 42 PAY  39 

    0x021c9a29,// 43 PAY  40 

    0x9e96115b,// 44 PAY  41 

    0x430e1fe2,// 45 PAY  42 

    0x0ad1dfbd,// 46 PAY  43 

    0xb0bd6eb4,// 47 PAY  44 

    0x4224dc8c,// 48 PAY  45 

    0xcfbd9185,// 49 PAY  46 

    0xca277e7c,// 50 PAY  47 

    0x5ba8c125,// 51 PAY  48 

    0xf71be716,// 52 PAY  49 

    0xd0c6df63,// 53 PAY  50 

    0x88e25534,// 54 PAY  51 

    0x2274d5b9,// 55 PAY  52 

    0x2cd5300e,// 56 PAY  53 

    0xc9b1a256,// 57 PAY  54 

    0x117150bc,// 58 PAY  55 

    0x0f6043b6,// 59 PAY  56 

    0x1af6e51b,// 60 PAY  57 

    0x312e7798,// 61 PAY  58 

    0xc0fae3c4,// 62 PAY  59 

    0x72f07f65,// 63 PAY  60 

    0x7277c874,// 64 PAY  61 

    0x7411d034,// 65 PAY  62 

    0xa01d056c,// 66 PAY  63 

    0xfa7b03f1,// 67 PAY  64 

    0x976b4f79,// 68 PAY  65 

    0x74c3bef1,// 69 PAY  66 

    0x476f65b8,// 70 PAY  67 

    0x22672bba,// 71 PAY  68 

    0x30d41b6c,// 72 PAY  69 

    0x0f5e67df,// 73 PAY  70 

    0x59a85e4c,// 74 PAY  71 

    0x3bcf1e8c,// 75 PAY  72 

    0x8cd5963f,// 76 PAY  73 

    0x22c84250,// 77 PAY  74 

    0x665def0a,// 78 PAY  75 

    0x1f3255c4,// 79 PAY  76 

    0x8ca46ce3,// 80 PAY  77 

    0x178f7830,// 81 PAY  78 

    0xbd9d28f1,// 82 PAY  79 

    0xe6eb7f7f,// 83 PAY  80 

    0x6c1d30df,// 84 PAY  81 

    0xcd766f75,// 85 PAY  82 

    0xbdb6aed1,// 86 PAY  83 

    0x56f4e868,// 87 PAY  84 

    0x71e837ee,// 88 PAY  85 

    0xa4d7a01b,// 89 PAY  86 

    0xb9aea520,// 90 PAY  87 

    0x0f7cbf1d,// 91 PAY  88 

    0xf823ed02,// 92 PAY  89 

    0xfb316ec9,// 93 PAY  90 

    0xe657dbe1,// 94 PAY  91 

    0x14ef46b0,// 95 PAY  92 

    0x726ebb79,// 96 PAY  93 

    0x6481234c,// 97 PAY  94 

    0x81fb295e,// 98 PAY  95 

    0xb60d212f,// 99 PAY  96 

    0x70f5066e,// 100 PAY  97 

    0x35dd8a09,// 101 PAY  98 

    0x355a1659,// 102 PAY  99 

    0x203ce37a,// 103 PAY 100 

    0x15836020,// 104 PAY 101 

    0xdbc81fd3,// 105 PAY 102 

    0x70c6b5fc,// 106 PAY 103 

    0xa16e15a8,// 107 PAY 104 

    0x53e39f71,// 108 PAY 105 

    0x001e4a41,// 109 PAY 106 

    0x55fa11d5,// 110 PAY 107 

    0x3c6e401c,// 111 PAY 108 

    0xae5a1121,// 112 PAY 109 

    0xc16a3dbb,// 113 PAY 110 

    0x8386fc9a,// 114 PAY 111 

    0x91f4b736,// 115 PAY 112 

    0x0a7fd7d9,// 116 PAY 113 

    0x5013da5d,// 117 PAY 114 

    0x5261fc78,// 118 PAY 115 

    0x16d9bb0f,// 119 PAY 116 

    0xe9474ee3,// 120 PAY 117 

    0x1ec9dea4,// 121 PAY 118 

    0x7af6357b,// 122 PAY 119 

    0x068515ce,// 123 PAY 120 

    0x4088d759,// 124 PAY 121 

    0x4ac40012,// 125 PAY 122 

    0xa78f4858,// 126 PAY 123 

    0xca8384ed,// 127 PAY 124 

    0x79984fb4,// 128 PAY 125 

    0x6d54f989,// 129 PAY 126 

    0xc0fe3683,// 130 PAY 127 

    0x29f5e04f,// 131 PAY 128 

    0x6cf447fb,// 132 PAY 129 

    0x6975d786,// 133 PAY 130 

    0x1c290b94,// 134 PAY 131 

    0x297dd8da,// 135 PAY 132 

    0xfac5990a,// 136 PAY 133 

    0x3f756ff8,// 137 PAY 134 

    0xc3873c79,// 138 PAY 135 

    0xc0fbd5f0,// 139 PAY 136 

    0xb1844ebe,// 140 PAY 137 

    0xc9141946,// 141 PAY 138 

    0x2c9102be,// 142 PAY 139 

    0x58f4c32f,// 143 PAY 140 

    0x94a1e025,// 144 PAY 141 

    0xfe45ab01,// 145 PAY 142 

    0x7b49d3c0,// 146 PAY 143 

    0xffca9fa5,// 147 PAY 144 

    0xf7bb90d1,// 148 PAY 145 

    0x161887a1,// 149 PAY 146 

    0xf367606d,// 150 PAY 147 

    0x1effb875,// 151 PAY 148 

    0x05ccd0db,// 152 PAY 149 

    0x7bf0ba83,// 153 PAY 150 

    0x3ec09275,// 154 PAY 151 

    0xe7e03c9a,// 155 PAY 152 

    0x9b8e7064,// 156 PAY 153 

    0xfe6819e8,// 157 PAY 154 

    0xd1075e50,// 158 PAY 155 

    0xb92495bf,// 159 PAY 156 

    0x52944350,// 160 PAY 157 

    0x866e2028,// 161 PAY 158 

    0x4c0f43af,// 162 PAY 159 

    0x885403b6,// 163 PAY 160 

    0x5ff466ec,// 164 PAY 161 

    0x5019c958,// 165 PAY 162 

    0x7398c009,// 166 PAY 163 

    0x360b1941,// 167 PAY 164 

    0xf74fc073,// 168 PAY 165 

    0x7d0c854e,// 169 PAY 166 

    0x9b3fc06b,// 170 PAY 167 

    0x69eaaa9a,// 171 PAY 168 

    0x00b68050,// 172 PAY 169 

    0x3ea56df4,// 173 PAY 170 

    0x674635b9,// 174 PAY 171 

    0x6e5b30ff,// 175 PAY 172 

    0x34c60f3b,// 176 PAY 173 

    0x50456d6b,// 177 PAY 174 

    0x50bd23dc,// 178 PAY 175 

    0x3783ce5e,// 179 PAY 176 

    0x451b9cc8,// 180 PAY 177 

    0xb8bfa651,// 181 PAY 178 

    0xbad82c3c,// 182 PAY 179 

    0xa206fbed,// 183 PAY 180 

    0x7fac0d71,// 184 PAY 181 

    0x03de7e99,// 185 PAY 182 

    0x38934621,// 186 PAY 183 

    0x942612a0,// 187 PAY 184 

    0x90eac12f,// 188 PAY 185 

    0xc51bc559,// 189 PAY 186 

    0xb0831df3,// 190 PAY 187 

    0xd40c029d,// 191 PAY 188 

    0xbc5dbb85,// 192 PAY 189 

    0x6fe37eb0,// 193 PAY 190 

    0x170ed068,// 194 PAY 191 

    0xa1201802,// 195 PAY 192 

    0x1e624565,// 196 PAY 193 

    0xfd801f4a,// 197 PAY 194 

    0x07d31050,// 198 PAY 195 

    0xca8dcf06,// 199 PAY 196 

    0x7e63eca9,// 200 PAY 197 

    0x2c0c8ec5,// 201 PAY 198 

    0xc6955c9d,// 202 PAY 199 

    0x1d50df45,// 203 PAY 200 

    0x10311865,// 204 PAY 201 

    0x5e54fc53,// 205 PAY 202 

    0x628a60a7,// 206 PAY 203 

    0x568f5cfe,// 207 PAY 204 

    0x7b21a085,// 208 PAY 205 

    0x6c6c4316,// 209 PAY 206 

    0x76f3c992,// 210 PAY 207 

    0x38a94882,// 211 PAY 208 

    0xd58acc20,// 212 PAY 209 

    0x9e0c73f6,// 213 PAY 210 

    0xffb87781,// 214 PAY 211 

    0xc62643fe,// 215 PAY 212 

    0xb6b51d1e,// 216 PAY 213 

    0xccfa2bec,// 217 PAY 214 

    0x9ea60714,// 218 PAY 215 

    0x1d44c0c1,// 219 PAY 216 

    0x885bf5a1,// 220 PAY 217 

    0xc75e81d5,// 221 PAY 218 

    0x1b252716,// 222 PAY 219 

    0x85e758c5,// 223 PAY 220 

    0x5e90a664,// 224 PAY 221 

    0x27e0fd65,// 225 PAY 222 

    0x7feeb9c2,// 226 PAY 223 

    0x7f938336,// 227 PAY 224 

    0xe7f76ace,// 228 PAY 225 

    0xd08d82ce,// 229 PAY 226 

    0xa5a134b0,// 230 PAY 227 

    0x05068e6b,// 231 PAY 228 

    0x39adcb7c,// 232 PAY 229 

    0x32262340,// 233 PAY 230 

    0x100571b4,// 234 PAY 231 

    0x1abacd79,// 235 PAY 232 

    0xd5ff1ccc,// 236 PAY 233 

    0x4c93ee72,// 237 PAY 234 

    0x88b061ee,// 238 PAY 235 

    0x7c382da9,// 239 PAY 236 

    0x4756b4ec,// 240 PAY 237 

    0xdc381908,// 241 PAY 238 

    0xf7fa1229,// 242 PAY 239 

    0x5489b6c5,// 243 PAY 240 

    0x78b48b48,// 244 PAY 241 

    0x96b585fe,// 245 PAY 242 

    0xc6c8c6c7,// 246 PAY 243 

    0x11f8f455,// 247 PAY 244 

    0x7de1fa17,// 248 PAY 245 

    0x544e2fc8,// 249 PAY 246 

    0x37193d94,// 250 PAY 247 

    0x80de6d7f,// 251 PAY 248 

    0xfec8586a,// 252 PAY 249 

    0x57bbcc20,// 253 PAY 250 

    0x8ca887bc,// 254 PAY 251 

    0xdcda88c9,// 255 PAY 252 

    0x4c2fefbc,// 256 PAY 253 

    0x7302814a,// 257 PAY 254 

    0x4e8b0b40,// 258 PAY 255 

    0x7c8398b4,// 259 PAY 256 

    0xefb02862,// 260 PAY 257 

    0xf755eff8,// 261 PAY 258 

    0x3a4f6bf8,// 262 PAY 259 

    0xf164c0c8,// 263 PAY 260 

    0xd1a1b097,// 264 PAY 261 

    0x9e19820a,// 265 PAY 262 

    0x61058c53,// 266 PAY 263 

    0x88e41bdb,// 267 PAY 264 

    0xdb47b4cc,// 268 PAY 265 

    0x7483f316,// 269 PAY 266 

    0xaad7eea3,// 270 PAY 267 

    0x18695c8c,// 271 PAY 268 

    0x441a09af,// 272 PAY 269 

    0x368242e2,// 273 PAY 270 

    0x423b4013,// 274 PAY 271 

    0x4e83323c,// 275 PAY 272 

    0x8b40b603,// 276 PAY 273 

    0x36a32f03,// 277 PAY 274 

    0x9b76a8d3,// 278 PAY 275 

    0x84aa6390,// 279 PAY 276 

    0x277e445a,// 280 PAY 277 

    0x3f73d15b,// 281 PAY 278 

    0xf9338181,// 282 PAY 279 

    0x3597c3b8,// 283 PAY 280 

    0x7869be53,// 284 PAY 281 

    0x34e6d4cd,// 285 PAY 282 

    0xb0657544,// 286 PAY 283 

    0x7d9643d7,// 287 PAY 284 

    0x370bea3d,// 288 PAY 285 

    0x28f08cc8,// 289 PAY 286 

    0x94ae5008,// 290 PAY 287 

    0xdf6d3562,// 291 PAY 288 

    0x8e2db270,// 292 PAY 289 

    0x23777097,// 293 PAY 290 

    0x1b2e68f9,// 294 PAY 291 

    0xc4adb49a,// 295 PAY 292 

    0xacafa10a,// 296 PAY 293 

    0x093d28c0,// 297 PAY 294 

    0xa02dfe4b,// 298 PAY 295 

    0xe788c94d,// 299 PAY 296 

    0xd163b390,// 300 PAY 297 

    0xd28614f6,// 301 PAY 298 

    0xd3830c55,// 302 PAY 299 

    0xda472386,// 303 PAY 300 

    0x04059e6a,// 304 PAY 301 

    0xbf243fcf,// 305 PAY 302 

    0x07756414,// 306 PAY 303 

    0x0ba6e45b,// 307 PAY 304 

    0xc3930834,// 308 PAY 305 

    0xb8f36edf,// 309 PAY 306 

    0xf6c32dbe,// 310 PAY 307 

    0x56923e94,// 311 PAY 308 

    0x94d8d752,// 312 PAY 309 

    0x2070d46e,// 313 PAY 310 

    0xb5973427,// 314 PAY 311 

    0x2e0b770f,// 315 PAY 312 

    0xb12ff02f,// 316 PAY 313 

    0x93f14274,// 317 PAY 314 

    0x84f758a5,// 318 PAY 315 

    0xcefecb00,// 319 PAY 316 

    0x87b4e82d,// 320 PAY 317 

    0x1b005428,// 321 PAY 318 

    0x1ce73297,// 322 PAY 319 

    0x8efa7380,// 323 PAY 320 

    0x49748b50,// 324 PAY 321 

    0xfe78c01a,// 325 PAY 322 

    0xde222d0d,// 326 PAY 323 

    0x4dc44357,// 327 PAY 324 

    0xd7b80684,// 328 PAY 325 

    0x0cb1820f,// 329 PAY 326 

    0xd654cfdc,// 330 PAY 327 

    0x3d6fe4e1,// 331 PAY 328 

    0x3e313311,// 332 PAY 329 

    0xba56a4f8,// 333 PAY 330 

    0x972cd991,// 334 PAY 331 

    0x032d46dc,// 335 PAY 332 

    0x2611ac83,// 336 PAY 333 

    0x6d6d8dbf,// 337 PAY 334 

    0x3a9bfbc6,// 338 PAY 335 

    0x4862dc81,// 339 PAY 336 

    0x8f0bc642,// 340 PAY 337 

    0x998c94e5,// 341 PAY 338 

    0xe0c9e9b5,// 342 PAY 339 

    0x48eaf55c,// 343 PAY 340 

    0xc878b645,// 344 PAY 341 

    0x621bac64,// 345 PAY 342 

    0x70d85b30,// 346 PAY 343 

    0xf1e19aa5,// 347 PAY 344 

    0x8aacf431,// 348 PAY 345 

    0xc3f236f9,// 349 PAY 346 

    0xa1cee21e,// 350 PAY 347 

    0x9aeb0b4a,// 351 PAY 348 

    0x77e8109a,// 352 PAY 349 

    0x21e8bd52,// 353 PAY 350 

    0x6d4269cb,// 354 PAY 351 

    0xc5b4d05c,// 355 PAY 352 

    0x11989aa3,// 356 PAY 353 

    0x231aa71a,// 357 PAY 354 

    0xaa991ddc,// 358 PAY 355 

    0x5d858126,// 359 PAY 356 

    0x0f5e3184,// 360 PAY 357 

    0x0794e9aa,// 361 PAY 358 

    0xb2b13309,// 362 PAY 359 

    0xa363017d,// 363 PAY 360 

    0x130ccf41,// 364 PAY 361 

    0x632e0a70,// 365 PAY 362 

    0xbcbc45f2,// 366 PAY 363 

    0xb58fd149,// 367 PAY 364 

    0x5e01a9bc,// 368 PAY 365 

    0x940206de,// 369 PAY 366 

    0xdedd9f6d,// 370 PAY 367 

    0x9ec50555,// 371 PAY 368 

    0x06d3c20a,// 372 PAY 369 

    0x8469d27c,// 373 PAY 370 

    0xc7226afa,// 374 PAY 371 

    0x1149057b,// 375 PAY 372 

    0xd8d209f6,// 376 PAY 373 

    0xb4b46a2d,// 377 PAY 374 

    0xd9276abe,// 378 PAY 375 

    0xb25a812d,// 379 PAY 376 

    0x4a26e088,// 380 PAY 377 

    0x97f2b06f,// 381 PAY 378 

    0x1d0580f6,// 382 PAY 379 

    0x9a874e4d,// 383 PAY 380 

    0xc4f23457,// 384 PAY 381 

    0x199cdc6b,// 385 PAY 382 

    0xfebbab39,// 386 PAY 383 

    0x186042be,// 387 PAY 384 

    0x2166dc88,// 388 PAY 385 

    0xcec33723,// 389 PAY 386 

    0xa952dc42,// 390 PAY 387 

    0x9070acf3,// 391 PAY 388 

    0x6abc3d27,// 392 PAY 389 

    0xaa1ed7df,// 393 PAY 390 

    0x429d2869,// 394 PAY 391 

    0xff64792b,// 395 PAY 392 

    0xfe2dd280,// 396 PAY 393 

    0x808942ae,// 397 PAY 394 

    0xcc3b6660,// 398 PAY 395 

    0xb86ad204,// 399 PAY 396 

    0x261904ed,// 400 PAY 397 

    0x9ab102b9,// 401 PAY 398 

    0xdb1955c2,// 402 PAY 399 

    0xda281ccf,// 403 PAY 400 

    0xa485234c,// 404 PAY 401 

    0xa3ea2c8d,// 405 PAY 402 

    0xddba337c,// 406 PAY 403 

    0x40cc3b25,// 407 PAY 404 

    0xe59d8367,// 408 PAY 405 

    0x710dfe74,// 409 PAY 406 

    0x8e3a46f1,// 410 PAY 407 

    0xc798ed65,// 411 PAY 408 

    0x79e48fe0,// 412 PAY 409 

    0xe8010eef,// 413 PAY 410 

    0x1c2b3fc1,// 414 PAY 411 

    0x2a3945f9,// 415 PAY 412 

    0xee356bb6,// 416 PAY 413 

    0x4051512f,// 417 PAY 414 

    0x12265ef7,// 418 PAY 415 

    0xae595a9c,// 419 PAY 416 

    0xe11b9264,// 420 PAY 417 

    0x094dd352,// 421 PAY 418 

    0x301c1886,// 422 PAY 419 

    0x71144eee,// 423 PAY 420 

    0x764333de,// 424 PAY 421 

    0xc8f7bb7d,// 425 PAY 422 

    0xe289ec2a,// 426 PAY 423 

    0xcd490889,// 427 PAY 424 

    0x34aa3bd1,// 428 PAY 425 

    0xd61ebfaf,// 429 PAY 426 

    0x8cbe8096,// 430 PAY 427 

    0xab63e846,// 431 PAY 428 

    0x007e5d3e,// 432 PAY 429 

    0x3de77c40,// 433 PAY 430 

    0x03fb421f,// 434 PAY 431 

    0xd87df6c6,// 435 PAY 432 

    0x8217de5f,// 436 PAY 433 

    0xba3bfe31,// 437 PAY 434 

    0x5a3ed53b,// 438 PAY 435 

    0xbfa37224,// 439 PAY 436 

    0xbc6799e7,// 440 PAY 437 

    0x62317a4c,// 441 PAY 438 

    0xdc02626c,// 442 PAY 439 

    0x44da1c91,// 443 PAY 440 

    0x37061b8e,// 444 PAY 441 

    0xb3ddf455,// 445 PAY 442 

    0x9bc3615f,// 446 PAY 443 

    0x6e99f7ed,// 447 PAY 444 

    0xe8813b9d,// 448 PAY 445 

    0x6c24d85a,// 449 PAY 446 

    0xd290b0f2,// 450 PAY 447 

    0x2989bdd0,// 451 PAY 448 

/// HASH is  12 bytes 

    0xdc02626c,// 452 HSH   1 

    0x44da1c91,// 453 HSH   2 

    0x37061b8e,// 454 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 18 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xda 

    0x0051da12 // 455 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt61_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 481 words. 

/// BDA size     is 1920 (0x780) 

/// BDA id       is 0xb58b 

    0x0780b58b,// 3 BDA   1 

/// PAY Generic Data size   : 1920 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xa55c077c,// 4 PAY   1 

    0xac752890,// 5 PAY   2 

    0xf8266fd8,// 6 PAY   3 

    0x88cfb298,// 7 PAY   4 

    0x16356390,// 8 PAY   5 

    0x2e16bc41,// 9 PAY   6 

    0xfbe99a09,// 10 PAY   7 

    0x2e91c8ad,// 11 PAY   8 

    0xcfd079d7,// 12 PAY   9 

    0x77e29276,// 13 PAY  10 

    0x7d336873,// 14 PAY  11 

    0x34602e75,// 15 PAY  12 

    0x8770d505,// 16 PAY  13 

    0x9bb96f8d,// 17 PAY  14 

    0x37bf307f,// 18 PAY  15 

    0xe42d1f41,// 19 PAY  16 

    0xba821e17,// 20 PAY  17 

    0xbd245575,// 21 PAY  18 

    0xa6c10db5,// 22 PAY  19 

    0x2de6e446,// 23 PAY  20 

    0xb5a02b80,// 24 PAY  21 

    0x70018e44,// 25 PAY  22 

    0x93a8194c,// 26 PAY  23 

    0x60fc5ed5,// 27 PAY  24 

    0xcea40a16,// 28 PAY  25 

    0x0f1e1d3c,// 29 PAY  26 

    0xae1ef164,// 30 PAY  27 

    0x63c7d7d1,// 31 PAY  28 

    0x3edd9313,// 32 PAY  29 

    0xe379355c,// 33 PAY  30 

    0xe742d5a4,// 34 PAY  31 

    0x645a792f,// 35 PAY  32 

    0xedc972df,// 36 PAY  33 

    0xcbe27842,// 37 PAY  34 

    0x2193a206,// 38 PAY  35 

    0xb1a270f7,// 39 PAY  36 

    0xc5413a7d,// 40 PAY  37 

    0x0784fd6c,// 41 PAY  38 

    0x39d336fb,// 42 PAY  39 

    0xc74a0f41,// 43 PAY  40 

    0x8b6594fe,// 44 PAY  41 

    0x66dcd55b,// 45 PAY  42 

    0xb989f1fc,// 46 PAY  43 

    0xe988e86c,// 47 PAY  44 

    0xe2ae6108,// 48 PAY  45 

    0xba85c8b1,// 49 PAY  46 

    0x3518d7db,// 50 PAY  47 

    0xe979a42e,// 51 PAY  48 

    0xc3ff24b5,// 52 PAY  49 

    0x35fdd523,// 53 PAY  50 

    0x6eab508c,// 54 PAY  51 

    0x1e9f3e99,// 55 PAY  52 

    0x3f8eab1e,// 56 PAY  53 

    0x32c3e8f3,// 57 PAY  54 

    0x7b5cd77e,// 58 PAY  55 

    0xd1878c7f,// 59 PAY  56 

    0x582d03e0,// 60 PAY  57 

    0x8389f62f,// 61 PAY  58 

    0xdbc07442,// 62 PAY  59 

    0x82079a64,// 63 PAY  60 

    0x9954c8e3,// 64 PAY  61 

    0x84a758c4,// 65 PAY  62 

    0x23564c9b,// 66 PAY  63 

    0x4715233a,// 67 PAY  64 

    0xbd30c208,// 68 PAY  65 

    0x0bd5a271,// 69 PAY  66 

    0xa2d6c257,// 70 PAY  67 

    0x9e07f740,// 71 PAY  68 

    0xcfaacf09,// 72 PAY  69 

    0x13b537dc,// 73 PAY  70 

    0x6cb86579,// 74 PAY  71 

    0x912af7dc,// 75 PAY  72 

    0xee671e50,// 76 PAY  73 

    0x516a8009,// 77 PAY  74 

    0x06c6cc23,// 78 PAY  75 

    0x819d4e59,// 79 PAY  76 

    0xf56ee244,// 80 PAY  77 

    0xad85a3d0,// 81 PAY  78 

    0xf89d49a2,// 82 PAY  79 

    0xc7b79690,// 83 PAY  80 

    0xf4d0c7e9,// 84 PAY  81 

    0xa513a7cb,// 85 PAY  82 

    0x0fef36e7,// 86 PAY  83 

    0xbbe7f192,// 87 PAY  84 

    0x5dd0142b,// 88 PAY  85 

    0xea7d8736,// 89 PAY  86 

    0x164c449f,// 90 PAY  87 

    0xef64360e,// 91 PAY  88 

    0x146ea541,// 92 PAY  89 

    0x3d5f8add,// 93 PAY  90 

    0xf925f206,// 94 PAY  91 

    0x74b9d0b0,// 95 PAY  92 

    0x36f26f72,// 96 PAY  93 

    0xceaebff5,// 97 PAY  94 

    0xf3dc85e3,// 98 PAY  95 

    0xeb941a7d,// 99 PAY  96 

    0x78d93de5,// 100 PAY  97 

    0xe4e6bcab,// 101 PAY  98 

    0x1160e408,// 102 PAY  99 

    0xb624948f,// 103 PAY 100 

    0xde6f2fe4,// 104 PAY 101 

    0xf09bccac,// 105 PAY 102 

    0x859d3277,// 106 PAY 103 

    0x11251992,// 107 PAY 104 

    0xb7278bf4,// 108 PAY 105 

    0x5e795bd0,// 109 PAY 106 

    0x8100ef6e,// 110 PAY 107 

    0xa3ceaccb,// 111 PAY 108 

    0xe0b8451a,// 112 PAY 109 

    0x88acedde,// 113 PAY 110 

    0xfe37c612,// 114 PAY 111 

    0x928b53a9,// 115 PAY 112 

    0x339f22ec,// 116 PAY 113 

    0x0ee37281,// 117 PAY 114 

    0xa1dd43e2,// 118 PAY 115 

    0xf2e43e7b,// 119 PAY 116 

    0xda2e75cc,// 120 PAY 117 

    0x6ae9371e,// 121 PAY 118 

    0x1dfa0f88,// 122 PAY 119 

    0xc747b41b,// 123 PAY 120 

    0x50cb46cb,// 124 PAY 121 

    0x93c1d803,// 125 PAY 122 

    0x4eaeab18,// 126 PAY 123 

    0x6cb0e54a,// 127 PAY 124 

    0xf2449d7b,// 128 PAY 125 

    0x32a37488,// 129 PAY 126 

    0xe1be2eed,// 130 PAY 127 

    0x55be92bf,// 131 PAY 128 

    0x6d26b208,// 132 PAY 129 

    0x2c73ad8d,// 133 PAY 130 

    0x914cc624,// 134 PAY 131 

    0x9964f42d,// 135 PAY 132 

    0x7cd441e5,// 136 PAY 133 

    0x78a82683,// 137 PAY 134 

    0xb170765e,// 138 PAY 135 

    0xa4cc694c,// 139 PAY 136 

    0x40322134,// 140 PAY 137 

    0xbc52fbce,// 141 PAY 138 

    0x2097bbcd,// 142 PAY 139 

    0x05829cd1,// 143 PAY 140 

    0xef8d3d69,// 144 PAY 141 

    0x38d6175b,// 145 PAY 142 

    0x9ef86122,// 146 PAY 143 

    0x4449e484,// 147 PAY 144 

    0x0563b74f,// 148 PAY 145 

    0xa847150c,// 149 PAY 146 

    0xf0cc4df0,// 150 PAY 147 

    0x62d9a1ae,// 151 PAY 148 

    0xd6c9577e,// 152 PAY 149 

    0x220c924f,// 153 PAY 150 

    0x6e4d1e2b,// 154 PAY 151 

    0x5234528f,// 155 PAY 152 

    0x5d4d4525,// 156 PAY 153 

    0x952377ae,// 157 PAY 154 

    0x0b877731,// 158 PAY 155 

    0xf165d633,// 159 PAY 156 

    0x7afceb4f,// 160 PAY 157 

    0x2e881570,// 161 PAY 158 

    0x861d2630,// 162 PAY 159 

    0xfcac507f,// 163 PAY 160 

    0x3bf6577c,// 164 PAY 161 

    0x1bfa71cf,// 165 PAY 162 

    0xd2b6862e,// 166 PAY 163 

    0x5e11c2ab,// 167 PAY 164 

    0xe5ed1b88,// 168 PAY 165 

    0x494bd55c,// 169 PAY 166 

    0x9cd9b181,// 170 PAY 167 

    0x24f97611,// 171 PAY 168 

    0x1cdcaed7,// 172 PAY 169 

    0xda405a16,// 173 PAY 170 

    0x90abdecc,// 174 PAY 171 

    0xa368c666,// 175 PAY 172 

    0x92caddb4,// 176 PAY 173 

    0xc588fef7,// 177 PAY 174 

    0xaef0508b,// 178 PAY 175 

    0xd0c1c6b0,// 179 PAY 176 

    0xdaeb7337,// 180 PAY 177 

    0xf3ae41ff,// 181 PAY 178 

    0x39c8c638,// 182 PAY 179 

    0x52fbfb74,// 183 PAY 180 

    0x5a5c8e1b,// 184 PAY 181 

    0x5f42bbf0,// 185 PAY 182 

    0x1b1560b6,// 186 PAY 183 

    0xa450f3d1,// 187 PAY 184 

    0xe1731cda,// 188 PAY 185 

    0xc02f6c35,// 189 PAY 186 

    0xbd4e6c33,// 190 PAY 187 

    0x23868ca1,// 191 PAY 188 

    0xef2622bc,// 192 PAY 189 

    0x05f73fe4,// 193 PAY 190 

    0x53dfbd6e,// 194 PAY 191 

    0x4dae091b,// 195 PAY 192 

    0x133bbf22,// 196 PAY 193 

    0x7928748c,// 197 PAY 194 

    0x64d09b75,// 198 PAY 195 

    0xf0372281,// 199 PAY 196 

    0x5c00a13c,// 200 PAY 197 

    0xafd76a3b,// 201 PAY 198 

    0x4cb152b1,// 202 PAY 199 

    0x1aee81bc,// 203 PAY 200 

    0xae464700,// 204 PAY 201 

    0xce169f7f,// 205 PAY 202 

    0xca15fa4e,// 206 PAY 203 

    0x08094514,// 207 PAY 204 

    0xbefdfcb9,// 208 PAY 205 

    0xd3a0a132,// 209 PAY 206 

    0x0f5ff80d,// 210 PAY 207 

    0x7eba0a9f,// 211 PAY 208 

    0x4debb848,// 212 PAY 209 

    0x38c180dd,// 213 PAY 210 

    0xcbe5f48a,// 214 PAY 211 

    0x788b70c3,// 215 PAY 212 

    0x518b2b4b,// 216 PAY 213 

    0xb345232f,// 217 PAY 214 

    0x74fb1a17,// 218 PAY 215 

    0xd7078667,// 219 PAY 216 

    0x8cb4a22b,// 220 PAY 217 

    0xac5b0937,// 221 PAY 218 

    0xdb8e9d08,// 222 PAY 219 

    0x854545fa,// 223 PAY 220 

    0x0e3c100f,// 224 PAY 221 

    0x4c0a2689,// 225 PAY 222 

    0xf9e750e9,// 226 PAY 223 

    0x123edbad,// 227 PAY 224 

    0x68f6888b,// 228 PAY 225 

    0xb3eb7256,// 229 PAY 226 

    0xd5f9302f,// 230 PAY 227 

    0x100f8732,// 231 PAY 228 

    0x70caa38c,// 232 PAY 229 

    0x9a908355,// 233 PAY 230 

    0x9bbb1817,// 234 PAY 231 

    0x3e1c3377,// 235 PAY 232 

    0x8e49d671,// 236 PAY 233 

    0xe35a1fec,// 237 PAY 234 

    0x233f4c8e,// 238 PAY 235 

    0x63bee6fd,// 239 PAY 236 

    0x99ead7aa,// 240 PAY 237 

    0xb987615b,// 241 PAY 238 

    0x83e06833,// 242 PAY 239 

    0x25b4aefa,// 243 PAY 240 

    0x852d22ae,// 244 PAY 241 

    0x8a6eaf80,// 245 PAY 242 

    0xac7e6528,// 246 PAY 243 

    0x5db51095,// 247 PAY 244 

    0x82794e23,// 248 PAY 245 

    0x7344f0a7,// 249 PAY 246 

    0x0c88db72,// 250 PAY 247 

    0x73c2b790,// 251 PAY 248 

    0x64ffd0c8,// 252 PAY 249 

    0x620d591d,// 253 PAY 250 

    0x3e8c7cdf,// 254 PAY 251 

    0x12369bac,// 255 PAY 252 

    0xb4762d4e,// 256 PAY 253 

    0x47010767,// 257 PAY 254 

    0xfe6d5f1c,// 258 PAY 255 

    0xc21c05c7,// 259 PAY 256 

    0x3501223b,// 260 PAY 257 

    0xa027f453,// 261 PAY 258 

    0x8a79e95b,// 262 PAY 259 

    0xa09bbb46,// 263 PAY 260 

    0x732d382e,// 264 PAY 261 

    0x7b952202,// 265 PAY 262 

    0x976230c3,// 266 PAY 263 

    0xdb189bb6,// 267 PAY 264 

    0xdd211d83,// 268 PAY 265 

    0x8132b286,// 269 PAY 266 

    0x08c25a85,// 270 PAY 267 

    0x1cfa6397,// 271 PAY 268 

    0x83c76731,// 272 PAY 269 

    0x0742c10e,// 273 PAY 270 

    0xad892a86,// 274 PAY 271 

    0x46711935,// 275 PAY 272 

    0x1466c62e,// 276 PAY 273 

    0x9a775c02,// 277 PAY 274 

    0x7effe4e5,// 278 PAY 275 

    0xc698a934,// 279 PAY 276 

    0xcbe3e455,// 280 PAY 277 

    0x85d2b436,// 281 PAY 278 

    0x8962c1c9,// 282 PAY 279 

    0x84e19b02,// 283 PAY 280 

    0x5b88ddca,// 284 PAY 281 

    0xe5ce1e61,// 285 PAY 282 

    0x98e51ebe,// 286 PAY 283 

    0xa06b6235,// 287 PAY 284 

    0x879f7bc2,// 288 PAY 285 

    0xae66e25a,// 289 PAY 286 

    0x8bcc665b,// 290 PAY 287 

    0xdf655ad1,// 291 PAY 288 

    0x515d9ee8,// 292 PAY 289 

    0x4f011218,// 293 PAY 290 

    0x352da860,// 294 PAY 291 

    0xc5ff4d67,// 295 PAY 292 

    0xa7356eea,// 296 PAY 293 

    0xc0a83739,// 297 PAY 294 

    0x7fd30891,// 298 PAY 295 

    0xd040d325,// 299 PAY 296 

    0x0171ea95,// 300 PAY 297 

    0xf721cfcd,// 301 PAY 298 

    0x1a7e78bc,// 302 PAY 299 

    0x0955d93c,// 303 PAY 300 

    0xd2c9dfe6,// 304 PAY 301 

    0x1384c9dc,// 305 PAY 302 

    0x4b0428e2,// 306 PAY 303 

    0xc529a9dd,// 307 PAY 304 

    0x7c1aadc7,// 308 PAY 305 

    0xda01ac75,// 309 PAY 306 

    0xec621064,// 310 PAY 307 

    0x8a7a4d9b,// 311 PAY 308 

    0xb02a79d2,// 312 PAY 309 

    0xc0d65a8f,// 313 PAY 310 

    0x1c097008,// 314 PAY 311 

    0x5b0255a8,// 315 PAY 312 

    0x0520980b,// 316 PAY 313 

    0xb231b77c,// 317 PAY 314 

    0x753385f1,// 318 PAY 315 

    0x2666629a,// 319 PAY 316 

    0x3619109e,// 320 PAY 317 

    0x600a5bbc,// 321 PAY 318 

    0x352804d8,// 322 PAY 319 

    0xa3fe9455,// 323 PAY 320 

    0xbd704350,// 324 PAY 321 

    0x762bef74,// 325 PAY 322 

    0x17c57615,// 326 PAY 323 

    0xc41310cf,// 327 PAY 324 

    0x823351d8,// 328 PAY 325 

    0x6d99fdcc,// 329 PAY 326 

    0x41936330,// 330 PAY 327 

    0x3486dd17,// 331 PAY 328 

    0xf28c563a,// 332 PAY 329 

    0x8f0858bb,// 333 PAY 330 

    0x03a7ef78,// 334 PAY 331 

    0x795bc02c,// 335 PAY 332 

    0x9ac87d81,// 336 PAY 333 

    0x0f3b729a,// 337 PAY 334 

    0xbc4067c1,// 338 PAY 335 

    0x47d5c5fd,// 339 PAY 336 

    0x6a805656,// 340 PAY 337 

    0xa7da2cad,// 341 PAY 338 

    0x10557b6d,// 342 PAY 339 

    0x7d1f6837,// 343 PAY 340 

    0x80324980,// 344 PAY 341 

    0x17faa5b8,// 345 PAY 342 

    0x6aa12720,// 346 PAY 343 

    0x458b0e78,// 347 PAY 344 

    0xbda0409f,// 348 PAY 345 

    0x5af3cd12,// 349 PAY 346 

    0x25397088,// 350 PAY 347 

    0x5cca54e0,// 351 PAY 348 

    0x1892c8f4,// 352 PAY 349 

    0xbfd0b2fe,// 353 PAY 350 

    0xd29a4b38,// 354 PAY 351 

    0x951c043c,// 355 PAY 352 

    0xf062be56,// 356 PAY 353 

    0xe6a4997d,// 357 PAY 354 

    0xa73df7fc,// 358 PAY 355 

    0x1bb7d39e,// 359 PAY 356 

    0x6fabe8aa,// 360 PAY 357 

    0x4a1bbecb,// 361 PAY 358 

    0x6c9c1c28,// 362 PAY 359 

    0x5e2bb4da,// 363 PAY 360 

    0xeaa0b09b,// 364 PAY 361 

    0x798fc14a,// 365 PAY 362 

    0x6d14ecf7,// 366 PAY 363 

    0xe3d5cfc8,// 367 PAY 364 

    0x868022d5,// 368 PAY 365 

    0x4ef5832f,// 369 PAY 366 

    0xe632d969,// 370 PAY 367 

    0x89da5e94,// 371 PAY 368 

    0xae4a68fc,// 372 PAY 369 

    0xc894e55f,// 373 PAY 370 

    0xc605c8a0,// 374 PAY 371 

    0x20e88a44,// 375 PAY 372 

    0x8140cc41,// 376 PAY 373 

    0xa20ff1ce,// 377 PAY 374 

    0x5f35a165,// 378 PAY 375 

    0x24cb9fde,// 379 PAY 376 

    0x2605e6b6,// 380 PAY 377 

    0x6622a2a2,// 381 PAY 378 

    0xa1884d71,// 382 PAY 379 

    0x78fd88c3,// 383 PAY 380 

    0x6aa1fd1c,// 384 PAY 381 

    0xb6f99946,// 385 PAY 382 

    0x07a07f92,// 386 PAY 383 

    0x00b3d27d,// 387 PAY 384 

    0x8cc41be3,// 388 PAY 385 

    0x0f34e924,// 389 PAY 386 

    0xf5388c93,// 390 PAY 387 

    0xf8ec9370,// 391 PAY 388 

    0x8576012a,// 392 PAY 389 

    0x568216d6,// 393 PAY 390 

    0x3f3c6432,// 394 PAY 391 

    0x706171b5,// 395 PAY 392 

    0x2a9ffa3e,// 396 PAY 393 

    0xd8592cd2,// 397 PAY 394 

    0x7168dbfa,// 398 PAY 395 

    0x5b6a9470,// 399 PAY 396 

    0x2666dbcb,// 400 PAY 397 

    0x6b851531,// 401 PAY 398 

    0xd38beeba,// 402 PAY 399 

    0x2f5f1582,// 403 PAY 400 

    0xe4cf79ff,// 404 PAY 401 

    0x966fb515,// 405 PAY 402 

    0xd863dec9,// 406 PAY 403 

    0xfe7ffd8f,// 407 PAY 404 

    0xf91c89a3,// 408 PAY 405 

    0x90a41af4,// 409 PAY 406 

    0x95150e94,// 410 PAY 407 

    0xdc042bc0,// 411 PAY 408 

    0x51a7148c,// 412 PAY 409 

    0xff64fcd5,// 413 PAY 410 

    0x68f99d48,// 414 PAY 411 

    0xceffb823,// 415 PAY 412 

    0x4081abb6,// 416 PAY 413 

    0xac379345,// 417 PAY 414 

    0x23e83215,// 418 PAY 415 

    0x014e5897,// 419 PAY 416 

    0x2a2cf6d8,// 420 PAY 417 

    0xedc52e96,// 421 PAY 418 

    0x2b82485c,// 422 PAY 419 

    0x99d8d8ae,// 423 PAY 420 

    0x936d18ae,// 424 PAY 421 

    0x98ad13a2,// 425 PAY 422 

    0xfd26a123,// 426 PAY 423 

    0x84cc4da3,// 427 PAY 424 

    0x9508ebc2,// 428 PAY 425 

    0xfb5f3edd,// 429 PAY 426 

    0xf7c1180e,// 430 PAY 427 

    0x4002ba26,// 431 PAY 428 

    0xb276b007,// 432 PAY 429 

    0xaf1cd894,// 433 PAY 430 

    0x9def65ab,// 434 PAY 431 

    0x970b08f9,// 435 PAY 432 

    0x21a47e8b,// 436 PAY 433 

    0x6c13590a,// 437 PAY 434 

    0x999fa93a,// 438 PAY 435 

    0x409e70f1,// 439 PAY 436 

    0x11f0dd46,// 440 PAY 437 

    0xc6620811,// 441 PAY 438 

    0x55ae3bb1,// 442 PAY 439 

    0xcf676e12,// 443 PAY 440 

    0x60e40d5c,// 444 PAY 441 

    0x75a906ab,// 445 PAY 442 

    0x90e79371,// 446 PAY 443 

    0x1c5a38bd,// 447 PAY 444 

    0xa3f0fb7b,// 448 PAY 445 

    0x50112c7e,// 449 PAY 446 

    0x0cbe515c,// 450 PAY 447 

    0xf7b165bf,// 451 PAY 448 

    0x43d08178,// 452 PAY 449 

    0x2db8d64e,// 453 PAY 450 

    0xc731dca0,// 454 PAY 451 

    0xf562c1bb,// 455 PAY 452 

    0x70df8093,// 456 PAY 453 

    0xf14e5421,// 457 PAY 454 

    0xa8219a3a,// 458 PAY 455 

    0xd9fbe4c3,// 459 PAY 456 

    0x8e2daa19,// 460 PAY 457 

    0x51b51599,// 461 PAY 458 

    0x3a0265ff,// 462 PAY 459 

    0xc0c0f71f,// 463 PAY 460 

    0xccc25de3,// 464 PAY 461 

    0x3066b7de,// 465 PAY 462 

    0x89af4e51,// 466 PAY 463 

    0x4fba5be1,// 467 PAY 464 

    0xa258663a,// 468 PAY 465 

    0xdd1f4b11,// 469 PAY 466 

    0x34715530,// 470 PAY 467 

    0xdc2c8254,// 471 PAY 468 

    0x7ac06024,// 472 PAY 469 

    0xd3ca04ae,// 473 PAY 470 

    0x6de5c975,// 474 PAY 471 

    0x062151e9,// 475 PAY 472 

    0xffadc9af,// 476 PAY 473 

    0x76f4b27a,// 477 PAY 474 

    0xdb116972,// 478 PAY 475 

    0xf1295964,// 479 PAY 476 

    0xab03c2a2,// 480 PAY 477 

    0x70b89272,// 481 PAY 478 

    0x3007113c,// 482 PAY 479 

    0x7a5bd2b0,// 483 PAY 480 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x03586eb2 // 484 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt62_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 508 words. 

/// BDA size     is 2028 (0x7ec) 

/// BDA id       is 0x8eca 

    0x07ec8eca,// 3 BDA   1 

/// PAY Generic Data size   : 2028 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x61430c0f,// 4 PAY   1 

    0xacc96660,// 5 PAY   2 

    0xe394124a,// 6 PAY   3 

    0x186fa4ca,// 7 PAY   4 

    0x6b52834e,// 8 PAY   5 

    0xa6cf57d3,// 9 PAY   6 

    0xf897c981,// 10 PAY   7 

    0xc15644f1,// 11 PAY   8 

    0x5e989011,// 12 PAY   9 

    0x30e66ab1,// 13 PAY  10 

    0x738ccad0,// 14 PAY  11 

    0x622896f1,// 15 PAY  12 

    0x73aff28f,// 16 PAY  13 

    0x34f1909c,// 17 PAY  14 

    0x1b58628b,// 18 PAY  15 

    0x58550e62,// 19 PAY  16 

    0x994bdf02,// 20 PAY  17 

    0x3e7d9946,// 21 PAY  18 

    0x819a530e,// 22 PAY  19 

    0x5ad5ff19,// 23 PAY  20 

    0x409564b3,// 24 PAY  21 

    0x540957c7,// 25 PAY  22 

    0x3a6d8914,// 26 PAY  23 

    0xbcfdd1be,// 27 PAY  24 

    0x04d062fc,// 28 PAY  25 

    0x1075eab4,// 29 PAY  26 

    0xf0f79ef6,// 30 PAY  27 

    0x199306e2,// 31 PAY  28 

    0x3ae64582,// 32 PAY  29 

    0xd97d8b37,// 33 PAY  30 

    0x010bdba1,// 34 PAY  31 

    0x1db938fd,// 35 PAY  32 

    0x22e9700e,// 36 PAY  33 

    0x15e0c428,// 37 PAY  34 

    0x850c3470,// 38 PAY  35 

    0x505611c1,// 39 PAY  36 

    0x8da583d0,// 40 PAY  37 

    0x82dc584f,// 41 PAY  38 

    0x053472bd,// 42 PAY  39 

    0xbb5850d1,// 43 PAY  40 

    0x3ae5b370,// 44 PAY  41 

    0x7d824c33,// 45 PAY  42 

    0xf07feded,// 46 PAY  43 

    0x7de9d1c3,// 47 PAY  44 

    0x15158457,// 48 PAY  45 

    0x8fa83da2,// 49 PAY  46 

    0x0019639b,// 50 PAY  47 

    0x399dc32f,// 51 PAY  48 

    0x6fc91f09,// 52 PAY  49 

    0xe3fa9203,// 53 PAY  50 

    0x93938da1,// 54 PAY  51 

    0x22f2db49,// 55 PAY  52 

    0x858b4cb7,// 56 PAY  53 

    0x8151b06a,// 57 PAY  54 

    0x2e40377b,// 58 PAY  55 

    0x2ddae732,// 59 PAY  56 

    0x79bae0b8,// 60 PAY  57 

    0x0ec4edd3,// 61 PAY  58 

    0xe8c5bba2,// 62 PAY  59 

    0x7ba7d5cb,// 63 PAY  60 

    0xa6384b4b,// 64 PAY  61 

    0xa7153fa4,// 65 PAY  62 

    0x41249814,// 66 PAY  63 

    0x5efdbec1,// 67 PAY  64 

    0xf39d3f6f,// 68 PAY  65 

    0xf5b67aac,// 69 PAY  66 

    0x694a2d66,// 70 PAY  67 

    0x09a13ed3,// 71 PAY  68 

    0x4b0fb840,// 72 PAY  69 

    0x2f085386,// 73 PAY  70 

    0xd30eb394,// 74 PAY  71 

    0x00e4d588,// 75 PAY  72 

    0x349a3ebb,// 76 PAY  73 

    0xf499a15a,// 77 PAY  74 

    0x8222c4ca,// 78 PAY  75 

    0x03dfa017,// 79 PAY  76 

    0x64fdbcae,// 80 PAY  77 

    0xa2ff9b8c,// 81 PAY  78 

    0xf789d678,// 82 PAY  79 

    0x66df99b9,// 83 PAY  80 

    0x9639c665,// 84 PAY  81 

    0x4882ad58,// 85 PAY  82 

    0x897f3c2d,// 86 PAY  83 

    0xacadd463,// 87 PAY  84 

    0x73352d22,// 88 PAY  85 

    0x69ccf38c,// 89 PAY  86 

    0xa73f643e,// 90 PAY  87 

    0x4b9986a2,// 91 PAY  88 

    0xdebe5b1f,// 92 PAY  89 

    0xac72eaf2,// 93 PAY  90 

    0x1a54273d,// 94 PAY  91 

    0xe134542c,// 95 PAY  92 

    0x00023291,// 96 PAY  93 

    0x04135270,// 97 PAY  94 

    0xb1457ba5,// 98 PAY  95 

    0xe6d12e56,// 99 PAY  96 

    0xaba71494,// 100 PAY  97 

    0x93528a88,// 101 PAY  98 

    0xd87c0002,// 102 PAY  99 

    0x67701025,// 103 PAY 100 

    0xb31ba106,// 104 PAY 101 

    0xc0c5be54,// 105 PAY 102 

    0xbffb0f62,// 106 PAY 103 

    0xdaeec51d,// 107 PAY 104 

    0x9295f220,// 108 PAY 105 

    0xc7f86a67,// 109 PAY 106 

    0x8bd82826,// 110 PAY 107 

    0x7098677e,// 111 PAY 108 

    0x7d8ebd5b,// 112 PAY 109 

    0xa17a7e0e,// 113 PAY 110 

    0xdf6aef09,// 114 PAY 111 

    0x5704af3c,// 115 PAY 112 

    0x5baa5da8,// 116 PAY 113 

    0x6dff65ec,// 117 PAY 114 

    0x70f1cfb2,// 118 PAY 115 

    0x2228cf6b,// 119 PAY 116 

    0x16748f26,// 120 PAY 117 

    0xd538e60a,// 121 PAY 118 

    0xb5eb054c,// 122 PAY 119 

    0x7791456c,// 123 PAY 120 

    0xfb112853,// 124 PAY 121 

    0x616ada3b,// 125 PAY 122 

    0x69f7fa18,// 126 PAY 123 

    0x722e71a5,// 127 PAY 124 

    0xbd16ecc4,// 128 PAY 125 

    0x67261e86,// 129 PAY 126 

    0x27c949d6,// 130 PAY 127 

    0x88be476e,// 131 PAY 128 

    0xd12a5cf1,// 132 PAY 129 

    0x0fdd9409,// 133 PAY 130 

    0x7a4c16bb,// 134 PAY 131 

    0xd4d07af3,// 135 PAY 132 

    0x3542fef4,// 136 PAY 133 

    0x324f6133,// 137 PAY 134 

    0x3efb2e5b,// 138 PAY 135 

    0xf9172629,// 139 PAY 136 

    0xacacb150,// 140 PAY 137 

    0xac9fdbbf,// 141 PAY 138 

    0xb8651089,// 142 PAY 139 

    0x46045128,// 143 PAY 140 

    0x1d5c5c7c,// 144 PAY 141 

    0x9f070bbd,// 145 PAY 142 

    0x929316e1,// 146 PAY 143 

    0xcf6232d2,// 147 PAY 144 

    0x24ff0127,// 148 PAY 145 

    0x973b9f9b,// 149 PAY 146 

    0x7a09f7c5,// 150 PAY 147 

    0xff4be658,// 151 PAY 148 

    0x38bc5461,// 152 PAY 149 

    0x0e3a8994,// 153 PAY 150 

    0xfcd91126,// 154 PAY 151 

    0x314d66f8,// 155 PAY 152 

    0x1f90fcc9,// 156 PAY 153 

    0x6deab148,// 157 PAY 154 

    0x37eddc96,// 158 PAY 155 

    0x8ad12548,// 159 PAY 156 

    0x9ff91ae8,// 160 PAY 157 

    0x35a642cb,// 161 PAY 158 

    0x8dea087b,// 162 PAY 159 

    0x991d53e6,// 163 PAY 160 

    0x5ea6b76c,// 164 PAY 161 

    0xf7e9dcb0,// 165 PAY 162 

    0xe8fa30a5,// 166 PAY 163 

    0xf7dc2575,// 167 PAY 164 

    0x93065785,// 168 PAY 165 

    0xec73159e,// 169 PAY 166 

    0x23708272,// 170 PAY 167 

    0x5d69b9cc,// 171 PAY 168 

    0x510b9348,// 172 PAY 169 

    0x23a89ee5,// 173 PAY 170 

    0x04af1edc,// 174 PAY 171 

    0x8255db7e,// 175 PAY 172 

    0x20f22e8f,// 176 PAY 173 

    0x9f0916de,// 177 PAY 174 

    0x8b092682,// 178 PAY 175 

    0x363779ed,// 179 PAY 176 

    0x5d04acd3,// 180 PAY 177 

    0x3d429828,// 181 PAY 178 

    0x074314f3,// 182 PAY 179 

    0x4e231da3,// 183 PAY 180 

    0xc9124a84,// 184 PAY 181 

    0x1e088bcd,// 185 PAY 182 

    0x5f68edd0,// 186 PAY 183 

    0x412a763e,// 187 PAY 184 

    0x96a5882a,// 188 PAY 185 

    0x7889f9db,// 189 PAY 186 

    0xbb64678e,// 190 PAY 187 

    0x972452ef,// 191 PAY 188 

    0x9bcee0f8,// 192 PAY 189 

    0x3f45a985,// 193 PAY 190 

    0xc146faa7,// 194 PAY 191 

    0x5a16c595,// 195 PAY 192 

    0xc2ef82f0,// 196 PAY 193 

    0x87f5f5e1,// 197 PAY 194 

    0x57408bda,// 198 PAY 195 

    0x2bc7c301,// 199 PAY 196 

    0x20140f6d,// 200 PAY 197 

    0xd83ccbe9,// 201 PAY 198 

    0xab0b6e66,// 202 PAY 199 

    0x390bb392,// 203 PAY 200 

    0xd028a3d9,// 204 PAY 201 

    0x678d020a,// 205 PAY 202 

    0x4e21bd98,// 206 PAY 203 

    0x0e50b9c8,// 207 PAY 204 

    0xaac6b86d,// 208 PAY 205 

    0x530bab81,// 209 PAY 206 

    0xc6442129,// 210 PAY 207 

    0x7432a5ca,// 211 PAY 208 

    0x8ef9ad7d,// 212 PAY 209 

    0xf6d760a3,// 213 PAY 210 

    0x1bd6ed9c,// 214 PAY 211 

    0x61eb1c36,// 215 PAY 212 

    0x6484cff8,// 216 PAY 213 

    0x0914ef86,// 217 PAY 214 

    0x9c06c1af,// 218 PAY 215 

    0x5142670c,// 219 PAY 216 

    0x9a5b4257,// 220 PAY 217 

    0x59570bd4,// 221 PAY 218 

    0xeaecb11d,// 222 PAY 219 

    0xcae8adde,// 223 PAY 220 

    0x7d2a3c6b,// 224 PAY 221 

    0x13f06434,// 225 PAY 222 

    0x17819d7b,// 226 PAY 223 

    0x72c98e59,// 227 PAY 224 

    0xb58a3340,// 228 PAY 225 

    0x3142aaab,// 229 PAY 226 

    0x97bfe136,// 230 PAY 227 

    0x9b8db2a6,// 231 PAY 228 

    0x55198f68,// 232 PAY 229 

    0x5c8c58eb,// 233 PAY 230 

    0x490189a8,// 234 PAY 231 

    0x2ac5fe74,// 235 PAY 232 

    0xd8728765,// 236 PAY 233 

    0x46db81fe,// 237 PAY 234 

    0x036fecf0,// 238 PAY 235 

    0xa24d3bb9,// 239 PAY 236 

    0x0850e44f,// 240 PAY 237 

    0xe81d9e7a,// 241 PAY 238 

    0x5b536b36,// 242 PAY 239 

    0x3a8381d6,// 243 PAY 240 

    0x5839a802,// 244 PAY 241 

    0x6694d4e5,// 245 PAY 242 

    0x8a1b14a7,// 246 PAY 243 

    0x52181a4e,// 247 PAY 244 

    0xab428b63,// 248 PAY 245 

    0xe9c48c22,// 249 PAY 246 

    0xee4c8513,// 250 PAY 247 

    0x75b2eabf,// 251 PAY 248 

    0x0f73bf62,// 252 PAY 249 

    0x58308cb5,// 253 PAY 250 

    0x2d2f4d56,// 254 PAY 251 

    0xff49f4dc,// 255 PAY 252 

    0xbe7f8a5b,// 256 PAY 253 

    0xcb4091e1,// 257 PAY 254 

    0xb7b957a5,// 258 PAY 255 

    0x7a778093,// 259 PAY 256 

    0x06eb7d7e,// 260 PAY 257 

    0xb05bf02d,// 261 PAY 258 

    0x3e9e1b64,// 262 PAY 259 

    0x4cfd6c3f,// 263 PAY 260 

    0xbdd0838d,// 264 PAY 261 

    0xf0449fff,// 265 PAY 262 

    0xd2b5d0ef,// 266 PAY 263 

    0x82289263,// 267 PAY 264 

    0x5d5ddeb9,// 268 PAY 265 

    0x03fba7aa,// 269 PAY 266 

    0x4c1afd23,// 270 PAY 267 

    0x36e9eb2b,// 271 PAY 268 

    0xb83baead,// 272 PAY 269 

    0x477af4ce,// 273 PAY 270 

    0xc5410c56,// 274 PAY 271 

    0x1435a346,// 275 PAY 272 

    0xe9d742f3,// 276 PAY 273 

    0x730c8115,// 277 PAY 274 

    0x6901bd9e,// 278 PAY 275 

    0x0c5bbc0b,// 279 PAY 276 

    0x7c9d19e8,// 280 PAY 277 

    0x78753c8b,// 281 PAY 278 

    0x9dcd946a,// 282 PAY 279 

    0xd01ec817,// 283 PAY 280 

    0x4c248d52,// 284 PAY 281 

    0xca88d7bf,// 285 PAY 282 

    0x79f1ebae,// 286 PAY 283 

    0xb6ebfc69,// 287 PAY 284 

    0x193d50d4,// 288 PAY 285 

    0x255ef8c1,// 289 PAY 286 

    0x1df00cdd,// 290 PAY 287 

    0xb64f1512,// 291 PAY 288 

    0xb3a6ef5a,// 292 PAY 289 

    0x54966c46,// 293 PAY 290 

    0xcb4275ab,// 294 PAY 291 

    0xdc37615f,// 295 PAY 292 

    0xbf01b453,// 296 PAY 293 

    0xce466ef5,// 297 PAY 294 

    0xc9c57469,// 298 PAY 295 

    0xbae7b303,// 299 PAY 296 

    0x9008ef35,// 300 PAY 297 

    0xba432dfd,// 301 PAY 298 

    0x819bb014,// 302 PAY 299 

    0xff2da7a6,// 303 PAY 300 

    0x895b1240,// 304 PAY 301 

    0xca2dc243,// 305 PAY 302 

    0x8c030d0f,// 306 PAY 303 

    0x308b2e84,// 307 PAY 304 

    0x90bbeaf0,// 308 PAY 305 

    0xdc336347,// 309 PAY 306 

    0xf7bccd92,// 310 PAY 307 

    0x4fce1134,// 311 PAY 308 

    0x238755e1,// 312 PAY 309 

    0x5928b86e,// 313 PAY 310 

    0xf8308d0b,// 314 PAY 311 

    0xf526bdc3,// 315 PAY 312 

    0xe285a7ac,// 316 PAY 313 

    0x41188a62,// 317 PAY 314 

    0xe01b2e72,// 318 PAY 315 

    0x9289c2e5,// 319 PAY 316 

    0xdbc08111,// 320 PAY 317 

    0x7162cdbe,// 321 PAY 318 

    0xde803a40,// 322 PAY 319 

    0x70d5a252,// 323 PAY 320 

    0x15326474,// 324 PAY 321 

    0x91777d57,// 325 PAY 322 

    0xad4ebffe,// 326 PAY 323 

    0xfa86127a,// 327 PAY 324 

    0x7db524dc,// 328 PAY 325 

    0x8d947f68,// 329 PAY 326 

    0x4b34182c,// 330 PAY 327 

    0x34d20cfe,// 331 PAY 328 

    0xa711bbe9,// 332 PAY 329 

    0xe334a017,// 333 PAY 330 

    0x3da67d40,// 334 PAY 331 

    0x2f592131,// 335 PAY 332 

    0x437496fb,// 336 PAY 333 

    0x7c71333c,// 337 PAY 334 

    0x57f3db6e,// 338 PAY 335 

    0xf879946e,// 339 PAY 336 

    0x3d46b148,// 340 PAY 337 

    0x6c4d0ec1,// 341 PAY 338 

    0x567d2a5e,// 342 PAY 339 

    0x37409e7b,// 343 PAY 340 

    0xc7b45827,// 344 PAY 341 

    0x303adf5f,// 345 PAY 342 

    0x7b6bea60,// 346 PAY 343 

    0xe4b11bdd,// 347 PAY 344 

    0x61a2f8d7,// 348 PAY 345 

    0xb82cd926,// 349 PAY 346 

    0x8b16161d,// 350 PAY 347 

    0x628ae89c,// 351 PAY 348 

    0x910c1930,// 352 PAY 349 

    0xf47cd00b,// 353 PAY 350 

    0x646e7d42,// 354 PAY 351 

    0x69608c22,// 355 PAY 352 

    0x5bcb249f,// 356 PAY 353 

    0x5f56bc05,// 357 PAY 354 

    0x0cef1621,// 358 PAY 355 

    0xcb9c5f69,// 359 PAY 356 

    0x74cf7510,// 360 PAY 357 

    0x3e060eec,// 361 PAY 358 

    0xbf7bafec,// 362 PAY 359 

    0xce963035,// 363 PAY 360 

    0x1dc89815,// 364 PAY 361 

    0xa223db69,// 365 PAY 362 

    0xfdb14fa3,// 366 PAY 363 

    0xd17d03bf,// 367 PAY 364 

    0x9ad1868c,// 368 PAY 365 

    0xd9d7f6e8,// 369 PAY 366 

    0x3e7f4f53,// 370 PAY 367 

    0x080ab9c4,// 371 PAY 368 

    0x9c1ec65f,// 372 PAY 369 

    0x1df81dd0,// 373 PAY 370 

    0x21488b25,// 374 PAY 371 

    0x7aa401f3,// 375 PAY 372 

    0x4199edf7,// 376 PAY 373 

    0xc76eda83,// 377 PAY 374 

    0xcaac2021,// 378 PAY 375 

    0x18491ec6,// 379 PAY 376 

    0xf82e8860,// 380 PAY 377 

    0x5c61e361,// 381 PAY 378 

    0xece80a3e,// 382 PAY 379 

    0x80f86757,// 383 PAY 380 

    0x94e5e3ae,// 384 PAY 381 

    0x603a18de,// 385 PAY 382 

    0xeba8fa86,// 386 PAY 383 

    0x2d7b7f94,// 387 PAY 384 

    0x72ec5a51,// 388 PAY 385 

    0xafe5180a,// 389 PAY 386 

    0x1b5b201f,// 390 PAY 387 

    0x85f5eeb5,// 391 PAY 388 

    0x484fe397,// 392 PAY 389 

    0xb86ace7d,// 393 PAY 390 

    0xbce2efd1,// 394 PAY 391 

    0xc76b7400,// 395 PAY 392 

    0x4e855a93,// 396 PAY 393 

    0x397d4a8d,// 397 PAY 394 

    0x184bbaf0,// 398 PAY 395 

    0x859cea3f,// 399 PAY 396 

    0x3e51150e,// 400 PAY 397 

    0xf2d373a0,// 401 PAY 398 

    0x0d7bff49,// 402 PAY 399 

    0x544ae360,// 403 PAY 400 

    0x86ca0300,// 404 PAY 401 

    0x176c6216,// 405 PAY 402 

    0x7ebf5f97,// 406 PAY 403 

    0xe1b7fee8,// 407 PAY 404 

    0x649624c4,// 408 PAY 405 

    0xb94a7dcb,// 409 PAY 406 

    0x2f4cb0d8,// 410 PAY 407 

    0xdf7b1a1c,// 411 PAY 408 

    0xac43fe60,// 412 PAY 409 

    0x5f7670b1,// 413 PAY 410 

    0xa8a86bf6,// 414 PAY 411 

    0xb68d1a38,// 415 PAY 412 

    0x6d6e49d2,// 416 PAY 413 

    0xbcb1354a,// 417 PAY 414 

    0x7f9b6594,// 418 PAY 415 

    0xa061177e,// 419 PAY 416 

    0x37b40f24,// 420 PAY 417 

    0xdf63d955,// 421 PAY 418 

    0xd051c988,// 422 PAY 419 

    0x3dad6adc,// 423 PAY 420 

    0x18c520b9,// 424 PAY 421 

    0xfef314e3,// 425 PAY 422 

    0x86f89006,// 426 PAY 423 

    0x91e75157,// 427 PAY 424 

    0xd6caa422,// 428 PAY 425 

    0xdb623041,// 429 PAY 426 

    0xba110d2a,// 430 PAY 427 

    0x22597185,// 431 PAY 428 

    0x159a172f,// 432 PAY 429 

    0x502822d0,// 433 PAY 430 

    0x49f0e042,// 434 PAY 431 

    0xd4c1dca1,// 435 PAY 432 

    0xec3e47dd,// 436 PAY 433 

    0xb0f50e98,// 437 PAY 434 

    0xb5f57ba0,// 438 PAY 435 

    0x19f23c27,// 439 PAY 436 

    0x0035396c,// 440 PAY 437 

    0x696b6f4d,// 441 PAY 438 

    0xc86018e5,// 442 PAY 439 

    0x7c6bfb2c,// 443 PAY 440 

    0x7163d5b9,// 444 PAY 441 

    0x47080bb8,// 445 PAY 442 

    0x3ade1b6c,// 446 PAY 443 

    0xd5dee176,// 447 PAY 444 

    0x877a49b5,// 448 PAY 445 

    0x4f041eaf,// 449 PAY 446 

    0x6a0f73a7,// 450 PAY 447 

    0x2007f95b,// 451 PAY 448 

    0x74b53fe5,// 452 PAY 449 

    0x808b4f14,// 453 PAY 450 

    0x694f4a61,// 454 PAY 451 

    0x61663c30,// 455 PAY 452 

    0x2cbdc235,// 456 PAY 453 

    0x01105233,// 457 PAY 454 

    0x893bb1ee,// 458 PAY 455 

    0x28a3388b,// 459 PAY 456 

    0xe92d86df,// 460 PAY 457 

    0x4643e977,// 461 PAY 458 

    0x65f51b92,// 462 PAY 459 

    0x927671b0,// 463 PAY 460 

    0x76beea44,// 464 PAY 461 

    0x479e9ad9,// 465 PAY 462 

    0x99737a33,// 466 PAY 463 

    0x159bf9ef,// 467 PAY 464 

    0xaf529134,// 468 PAY 465 

    0xde94b1b3,// 469 PAY 466 

    0x383efad2,// 470 PAY 467 

    0x91f228f2,// 471 PAY 468 

    0x56a1435b,// 472 PAY 469 

    0xe9232e50,// 473 PAY 470 

    0xdeac8eca,// 474 PAY 471 

    0xf827687d,// 475 PAY 472 

    0x1f21bb8d,// 476 PAY 473 

    0xcd983e6a,// 477 PAY 474 

    0xfef4ac17,// 478 PAY 475 

    0x41b9d8d6,// 479 PAY 476 

    0x4cef6e76,// 480 PAY 477 

    0xcd06adff,// 481 PAY 478 

    0x46bab037,// 482 PAY 479 

    0x14074f20,// 483 PAY 480 

    0xb4347626,// 484 PAY 481 

    0x3706aad0,// 485 PAY 482 

    0x4c6985bc,// 486 PAY 483 

    0x5806c0bd,// 487 PAY 484 

    0xda8ca5d2,// 488 PAY 485 

    0x52161835,// 489 PAY 486 

    0xcd74ff17,// 490 PAY 487 

    0x55ea5168,// 491 PAY 488 

    0xc6f35969,// 492 PAY 489 

    0x541561f6,// 493 PAY 490 

    0x10b06d74,// 494 PAY 491 

    0x9f10825e,// 495 PAY 492 

    0xc621daf2,// 496 PAY 493 

    0xf2b463d3,// 497 PAY 494 

    0xddd3cf75,// 498 PAY 495 

    0x8e13ae7e,// 499 PAY 496 

    0xf4a8f2c7,// 500 PAY 497 

    0xce3bbeb2,// 501 PAY 498 

    0x67ea9f7d,// 502 PAY 499 

    0x3fa33ea1,// 503 PAY 500 

    0x39107259,// 504 PAY 501 

    0xaaebfe4b,// 505 PAY 502 

    0x2fd91ee6,// 506 PAY 503 

    0x732c44de,// 507 PAY 504 

    0x99e00bf3,// 508 PAY 505 

    0xfa32329f,// 509 PAY 506 

    0xa948f66b,// 510 PAY 507 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 187 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdd 

    0x02ecdd80 // 511 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt63_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 397 words. 

/// BDA size     is 1581 (0x62d) 

/// BDA id       is 0xad27 

    0x062dad27,// 3 BDA   1 

/// PAY Generic Data size   : 1581 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x65e80bad,// 4 PAY   1 

    0x9e3eb366,// 5 PAY   2 

    0xcc25b4af,// 6 PAY   3 

    0x185a2496,// 7 PAY   4 

    0xeae6da10,// 8 PAY   5 

    0x3a0b987f,// 9 PAY   6 

    0x8dc2ac1a,// 10 PAY   7 

    0xd43c1790,// 11 PAY   8 

    0x817c0aa8,// 12 PAY   9 

    0x6681747d,// 13 PAY  10 

    0x0108a8d8,// 14 PAY  11 

    0xa3d2b37d,// 15 PAY  12 

    0x47892837,// 16 PAY  13 

    0x473be2a0,// 17 PAY  14 

    0x852a01a7,// 18 PAY  15 

    0x38896ba8,// 19 PAY  16 

    0x67afbeca,// 20 PAY  17 

    0xcfae2c30,// 21 PAY  18 

    0xa9c2d74d,// 22 PAY  19 

    0x325b98cb,// 23 PAY  20 

    0xe0f8d709,// 24 PAY  21 

    0x43a88a55,// 25 PAY  22 

    0xfb9a1f59,// 26 PAY  23 

    0xfaa7c3c5,// 27 PAY  24 

    0x3fd219a6,// 28 PAY  25 

    0x791e2aef,// 29 PAY  26 

    0x067189c4,// 30 PAY  27 

    0xd29d1f08,// 31 PAY  28 

    0xfeeff204,// 32 PAY  29 

    0x26db8099,// 33 PAY  30 

    0x44aa384c,// 34 PAY  31 

    0x6c418548,// 35 PAY  32 

    0x23617ea6,// 36 PAY  33 

    0x40398f42,// 37 PAY  34 

    0x14bfa32f,// 38 PAY  35 

    0x71b2b16a,// 39 PAY  36 

    0x74b71ac2,// 40 PAY  37 

    0xd955e4c4,// 41 PAY  38 

    0x1b15bf20,// 42 PAY  39 

    0x631567b7,// 43 PAY  40 

    0x5a303711,// 44 PAY  41 

    0x4d0b09e6,// 45 PAY  42 

    0x08616f4d,// 46 PAY  43 

    0x2ada9cfb,// 47 PAY  44 

    0x360bde22,// 48 PAY  45 

    0x76d732f2,// 49 PAY  46 

    0x5ec72a8f,// 50 PAY  47 

    0xab0a19e7,// 51 PAY  48 

    0x9e5031cb,// 52 PAY  49 

    0x0b16b697,// 53 PAY  50 

    0x11e04af3,// 54 PAY  51 

    0xa057b6b1,// 55 PAY  52 

    0x7cf3d21c,// 56 PAY  53 

    0x04e71e51,// 57 PAY  54 

    0x6faf2219,// 58 PAY  55 

    0xeab06001,// 59 PAY  56 

    0xcec16d10,// 60 PAY  57 

    0x50feb8a2,// 61 PAY  58 

    0xa69d5fe5,// 62 PAY  59 

    0x5713003d,// 63 PAY  60 

    0x56a7da96,// 64 PAY  61 

    0x6167ff0e,// 65 PAY  62 

    0x4989c040,// 66 PAY  63 

    0x7baf1648,// 67 PAY  64 

    0x25915840,// 68 PAY  65 

    0xbc30f032,// 69 PAY  66 

    0xb01d15da,// 70 PAY  67 

    0xbe655c61,// 71 PAY  68 

    0x6ea150c3,// 72 PAY  69 

    0xe4b39025,// 73 PAY  70 

    0x2e807941,// 74 PAY  71 

    0x60d96bf2,// 75 PAY  72 

    0xc91a1872,// 76 PAY  73 

    0x9ba21227,// 77 PAY  74 

    0xb364007c,// 78 PAY  75 

    0x099a43db,// 79 PAY  76 

    0xc3ea619b,// 80 PAY  77 

    0xe68d7bc6,// 81 PAY  78 

    0xef945b97,// 82 PAY  79 

    0x2f4d8c65,// 83 PAY  80 

    0x23eaa281,// 84 PAY  81 

    0xfa72321f,// 85 PAY  82 

    0x6dae2b28,// 86 PAY  83 

    0xccb3c0c5,// 87 PAY  84 

    0x92fb3c1a,// 88 PAY  85 

    0xd0f552d1,// 89 PAY  86 

    0x33434983,// 90 PAY  87 

    0x09210cab,// 91 PAY  88 

    0xc9fa4c76,// 92 PAY  89 

    0x56b04235,// 93 PAY  90 

    0x0f10132c,// 94 PAY  91 

    0xaf906089,// 95 PAY  92 

    0xab1cdb67,// 96 PAY  93 

    0xb5dfbd2d,// 97 PAY  94 

    0x1b166a11,// 98 PAY  95 

    0xedbfa3f5,// 99 PAY  96 

    0x2d40a38b,// 100 PAY  97 

    0x81bbcf2b,// 101 PAY  98 

    0x0e1b0396,// 102 PAY  99 

    0x2011416c,// 103 PAY 100 

    0x7e81d4c3,// 104 PAY 101 

    0x066940c2,// 105 PAY 102 

    0x5965ff1a,// 106 PAY 103 

    0x25278b70,// 107 PAY 104 

    0x40208c33,// 108 PAY 105 

    0x6cf3c61f,// 109 PAY 106 

    0x25b05c15,// 110 PAY 107 

    0xc68a0996,// 111 PAY 108 

    0x822a57ea,// 112 PAY 109 

    0x29482860,// 113 PAY 110 

    0xa232b6e0,// 114 PAY 111 

    0xf3f549e2,// 115 PAY 112 

    0xbaea951c,// 116 PAY 113 

    0x23a6e1c0,// 117 PAY 114 

    0xe492a5cf,// 118 PAY 115 

    0x89a224c4,// 119 PAY 116 

    0x98f04fe3,// 120 PAY 117 

    0x1d1a5069,// 121 PAY 118 

    0xba5297a8,// 122 PAY 119 

    0x292dcdd0,// 123 PAY 120 

    0xdaf192ba,// 124 PAY 121 

    0x8a362e8b,// 125 PAY 122 

    0xc40f0b6c,// 126 PAY 123 

    0xa3006216,// 127 PAY 124 

    0xec6a01b6,// 128 PAY 125 

    0x6905e27b,// 129 PAY 126 

    0x489d4415,// 130 PAY 127 

    0xf40ef0f7,// 131 PAY 128 

    0xf5dc2aab,// 132 PAY 129 

    0xabdfc28d,// 133 PAY 130 

    0xb44aaa78,// 134 PAY 131 

    0xf4cfc990,// 135 PAY 132 

    0x3f5363de,// 136 PAY 133 

    0x60ded868,// 137 PAY 134 

    0x30b8673a,// 138 PAY 135 

    0xc1f065fe,// 139 PAY 136 

    0x1466ef50,// 140 PAY 137 

    0x7cde037e,// 141 PAY 138 

    0x83dff4ac,// 142 PAY 139 

    0xb6d28305,// 143 PAY 140 

    0x99d75018,// 144 PAY 141 

    0xe1a997a1,// 145 PAY 142 

    0xdd5384c4,// 146 PAY 143 

    0xdb0a5dd3,// 147 PAY 144 

    0xdf98275b,// 148 PAY 145 

    0x0ed1cfac,// 149 PAY 146 

    0x36b815a3,// 150 PAY 147 

    0x95ac8aa5,// 151 PAY 148 

    0xb0fc60c3,// 152 PAY 149 

    0x0b0cc68f,// 153 PAY 150 

    0x2fd843e0,// 154 PAY 151 

    0x3dc47883,// 155 PAY 152 

    0x230139a4,// 156 PAY 153 

    0xd719d812,// 157 PAY 154 

    0xd95a9d20,// 158 PAY 155 

    0x74bfba43,// 159 PAY 156 

    0xae9ee675,// 160 PAY 157 

    0x0ef066af,// 161 PAY 158 

    0x2ae39fa7,// 162 PAY 159 

    0x06d85db2,// 163 PAY 160 

    0x70227f74,// 164 PAY 161 

    0x48eb4a0e,// 165 PAY 162 

    0xbf2ff54b,// 166 PAY 163 

    0x7c809465,// 167 PAY 164 

    0xa21a6b15,// 168 PAY 165 

    0xd3654e99,// 169 PAY 166 

    0x0211802e,// 170 PAY 167 

    0xe4f095df,// 171 PAY 168 

    0x2fef4e9f,// 172 PAY 169 

    0xcf0407f8,// 173 PAY 170 

    0x1c60f613,// 174 PAY 171 

    0x618706eb,// 175 PAY 172 

    0xd99b8f1c,// 176 PAY 173 

    0xca8ba323,// 177 PAY 174 

    0xa4add889,// 178 PAY 175 

    0x7cca7c72,// 179 PAY 176 

    0x98199cb3,// 180 PAY 177 

    0x2377a12f,// 181 PAY 178 

    0x48806707,// 182 PAY 179 

    0x9341aa0f,// 183 PAY 180 

    0xf4218e43,// 184 PAY 181 

    0x4d57f442,// 185 PAY 182 

    0xfefe8483,// 186 PAY 183 

    0x9bacc134,// 187 PAY 184 

    0xab628f94,// 188 PAY 185 

    0x11986c82,// 189 PAY 186 

    0x3e8e4d15,// 190 PAY 187 

    0x56d949d1,// 191 PAY 188 

    0x5bc2e8dc,// 192 PAY 189 

    0x4957f75e,// 193 PAY 190 

    0xc52f45b4,// 194 PAY 191 

    0xb54a1f2c,// 195 PAY 192 

    0x0748c899,// 196 PAY 193 

    0xc0c1a0dc,// 197 PAY 194 

    0xeb45bb27,// 198 PAY 195 

    0x33aeaa98,// 199 PAY 196 

    0xad3ee78b,// 200 PAY 197 

    0xfbb958f7,// 201 PAY 198 

    0xb871d5b0,// 202 PAY 199 

    0x57002305,// 203 PAY 200 

    0x506f329a,// 204 PAY 201 

    0x0ea1aace,// 205 PAY 202 

    0x071eafad,// 206 PAY 203 

    0xf710bb8c,// 207 PAY 204 

    0xd3c69fae,// 208 PAY 205 

    0xf8ba7d3a,// 209 PAY 206 

    0xf1d56cf7,// 210 PAY 207 

    0x8598bf37,// 211 PAY 208 

    0xd65432cb,// 212 PAY 209 

    0xf0bcd0d2,// 213 PAY 210 

    0x8e5fe562,// 214 PAY 211 

    0x8c0e7a23,// 215 PAY 212 

    0x92ccab53,// 216 PAY 213 

    0xfba7e22f,// 217 PAY 214 

    0x853e42b3,// 218 PAY 215 

    0x337629c8,// 219 PAY 216 

    0xa32c7b9c,// 220 PAY 217 

    0x35fe3f2d,// 221 PAY 218 

    0x75de84f7,// 222 PAY 219 

    0xb1a72434,// 223 PAY 220 

    0xb92d386f,// 224 PAY 221 

    0xfc1464e2,// 225 PAY 222 

    0xc9b65cd2,// 226 PAY 223 

    0x63f67ffe,// 227 PAY 224 

    0xd5d062f1,// 228 PAY 225 

    0xe1407637,// 229 PAY 226 

    0xd40c425d,// 230 PAY 227 

    0x1c4b4eb5,// 231 PAY 228 

    0xead67b1a,// 232 PAY 229 

    0xef894876,// 233 PAY 230 

    0xb21cd9b7,// 234 PAY 231 

    0xa866cf4f,// 235 PAY 232 

    0x865e492a,// 236 PAY 233 

    0x76f43217,// 237 PAY 234 

    0x95a166c7,// 238 PAY 235 

    0xbf2d0857,// 239 PAY 236 

    0x322dbde8,// 240 PAY 237 

    0xa6ed40fc,// 241 PAY 238 

    0x8d80ceba,// 242 PAY 239 

    0xe854054c,// 243 PAY 240 

    0x065006ef,// 244 PAY 241 

    0x41e2c8da,// 245 PAY 242 

    0x322e29a1,// 246 PAY 243 

    0x5e6d4023,// 247 PAY 244 

    0x0ec30138,// 248 PAY 245 

    0xc0ecc060,// 249 PAY 246 

    0x397b81d6,// 250 PAY 247 

    0x5dbb8d86,// 251 PAY 248 

    0xa8307f8d,// 252 PAY 249 

    0x2e43d5f2,// 253 PAY 250 

    0xe948ff56,// 254 PAY 251 

    0x3d25cfbc,// 255 PAY 252 

    0x2866ce4e,// 256 PAY 253 

    0x2c6c548a,// 257 PAY 254 

    0x5ed2f1f7,// 258 PAY 255 

    0x385b5ca4,// 259 PAY 256 

    0x87390876,// 260 PAY 257 

    0x3fe3a84e,// 261 PAY 258 

    0x428e9a0a,// 262 PAY 259 

    0x0d4a5a70,// 263 PAY 260 

    0x19a37de9,// 264 PAY 261 

    0x454a42ef,// 265 PAY 262 

    0x0fb731bc,// 266 PAY 263 

    0x1e62f615,// 267 PAY 264 

    0x93cb1566,// 268 PAY 265 

    0x244d479f,// 269 PAY 266 

    0x71de816a,// 270 PAY 267 

    0x81b06390,// 271 PAY 268 

    0xf71a120f,// 272 PAY 269 

    0x9b6b639e,// 273 PAY 270 

    0xff99173c,// 274 PAY 271 

    0x8327dee4,// 275 PAY 272 

    0xdc128d5f,// 276 PAY 273 

    0xbe09214f,// 277 PAY 274 

    0x510848ed,// 278 PAY 275 

    0x18ee20bf,// 279 PAY 276 

    0x1ec2bab0,// 280 PAY 277 

    0x95d27e9c,// 281 PAY 278 

    0x5d4df88d,// 282 PAY 279 

    0xc541c6b6,// 283 PAY 280 

    0x9c60811a,// 284 PAY 281 

    0x36fa9437,// 285 PAY 282 

    0xf7fce42f,// 286 PAY 283 

    0xe0cfeef2,// 287 PAY 284 

    0x352f3776,// 288 PAY 285 

    0x363170a0,// 289 PAY 286 

    0x661cad59,// 290 PAY 287 

    0x335573e4,// 291 PAY 288 

    0x07081d0d,// 292 PAY 289 

    0x77aa9b17,// 293 PAY 290 

    0xeade5aac,// 294 PAY 291 

    0x998893b0,// 295 PAY 292 

    0x73bdc1bf,// 296 PAY 293 

    0x0b7915d3,// 297 PAY 294 

    0xa9fbd295,// 298 PAY 295 

    0x8b266b4e,// 299 PAY 296 

    0xa19f8a5b,// 300 PAY 297 

    0x12fb3b38,// 301 PAY 298 

    0x2ea45ce2,// 302 PAY 299 

    0xefd7b3ae,// 303 PAY 300 

    0x393e88f0,// 304 PAY 301 

    0xd611d55c,// 305 PAY 302 

    0x58117c49,// 306 PAY 303 

    0x76c2e291,// 307 PAY 304 

    0x7ebf4a04,// 308 PAY 305 

    0x81f5e824,// 309 PAY 306 

    0xfa4c03d3,// 310 PAY 307 

    0x6d5b5c1a,// 311 PAY 308 

    0xbe0bb936,// 312 PAY 309 

    0x617332de,// 313 PAY 310 

    0x1a5bf546,// 314 PAY 311 

    0xc540c3af,// 315 PAY 312 

    0xa352547c,// 316 PAY 313 

    0x7b7e599f,// 317 PAY 314 

    0xd0e49f9e,// 318 PAY 315 

    0x6691aa75,// 319 PAY 316 

    0xff3c0f08,// 320 PAY 317 

    0x8ef11cce,// 321 PAY 318 

    0xc89de928,// 322 PAY 319 

    0x5556f9f5,// 323 PAY 320 

    0x2a6697aa,// 324 PAY 321 

    0xa92d91a4,// 325 PAY 322 

    0xf0bd919f,// 326 PAY 323 

    0xb46dda6a,// 327 PAY 324 

    0x3613ca0e,// 328 PAY 325 

    0x84d81a01,// 329 PAY 326 

    0xee3ad55e,// 330 PAY 327 

    0x59337953,// 331 PAY 328 

    0x382346e2,// 332 PAY 329 

    0x19112783,// 333 PAY 330 

    0xe9c57b04,// 334 PAY 331 

    0xfe0c603f,// 335 PAY 332 

    0x849b0bcf,// 336 PAY 333 

    0xc5fbde3c,// 337 PAY 334 

    0x550eb7b4,// 338 PAY 335 

    0x5e0e8faf,// 339 PAY 336 

    0x7b02df0e,// 340 PAY 337 

    0x491496a7,// 341 PAY 338 

    0xf0da445d,// 342 PAY 339 

    0xf7ba2066,// 343 PAY 340 

    0xe145a31d,// 344 PAY 341 

    0x02db19e0,// 345 PAY 342 

    0x7c316baa,// 346 PAY 343 

    0xbe14d815,// 347 PAY 344 

    0x61e5f84a,// 348 PAY 345 

    0x0deabcf7,// 349 PAY 346 

    0x2c5545df,// 350 PAY 347 

    0xf6fd771c,// 351 PAY 348 

    0x5ec99562,// 352 PAY 349 

    0x73b178f8,// 353 PAY 350 

    0xaf448ec5,// 354 PAY 351 

    0x6bf01503,// 355 PAY 352 

    0xf02c5462,// 356 PAY 353 

    0x19733cca,// 357 PAY 354 

    0xc2e84d71,// 358 PAY 355 

    0x4e015165,// 359 PAY 356 

    0xae608a64,// 360 PAY 357 

    0x046b6bdc,// 361 PAY 358 

    0x03eca4a0,// 362 PAY 359 

    0xbc01920e,// 363 PAY 360 

    0x51e696e8,// 364 PAY 361 

    0x07bcac02,// 365 PAY 362 

    0x313e4714,// 366 PAY 363 

    0xfbd824aa,// 367 PAY 364 

    0x32b14adf,// 368 PAY 365 

    0x52be061e,// 369 PAY 366 

    0x2d41272b,// 370 PAY 367 

    0xcd57b6a4,// 371 PAY 368 

    0x97094558,// 372 PAY 369 

    0xb9d6a156,// 373 PAY 370 

    0x60909b34,// 374 PAY 371 

    0xfac89954,// 375 PAY 372 

    0x9a807624,// 376 PAY 373 

    0x53041191,// 377 PAY 374 

    0xdf004a14,// 378 PAY 375 

    0x5f4b5fd6,// 379 PAY 376 

    0xfa3868dd,// 380 PAY 377 

    0xf707879c,// 381 PAY 378 

    0xdbce37f5,// 382 PAY 379 

    0x75c67f28,// 383 PAY 380 

    0x1adb3ceb,// 384 PAY 381 

    0xa399d93c,// 385 PAY 382 

    0xee16131e,// 386 PAY 383 

    0x159b5d66,// 387 PAY 384 

    0x963dd9ad,// 388 PAY 385 

    0x5c4c73af,// 389 PAY 386 

    0x8864f8ab,// 390 PAY 387 

    0xb1b3b2c7,// 391 PAY 388 

    0xc6c887f8,// 392 PAY 389 

    0x30a2c6ce,// 393 PAY 390 

    0x3c3c7bf6,// 394 PAY 391 

    0x22b60f10,// 395 PAY 392 

    0xe603f780,// 396 PAY 393 

    0x51863e3b,// 397 PAY 394 

    0xe8d19e1f,// 398 PAY 395 

    0x4a000000,// 399 PAY 396 

/// HASH is  8 bytes 

    0xfbd824aa,// 400 HSH   1 

    0x32b14adf,// 401 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 218 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x25 

    0x03682553 // 402 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt64_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 299 words. 

/// BDA size     is 1192 (0x4a8) 

/// BDA id       is 0x770a 

    0x04a8770a,// 3 BDA   1 

/// PAY Generic Data size   : 1192 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x33998e14,// 4 PAY   1 

    0x1f0897e1,// 5 PAY   2 

    0x078821e5,// 6 PAY   3 

    0xaebdc1bc,// 7 PAY   4 

    0x9ae378f2,// 8 PAY   5 

    0xc5b292aa,// 9 PAY   6 

    0xd5641f41,// 10 PAY   7 

    0xc1b4a822,// 11 PAY   8 

    0x231f3f02,// 12 PAY   9 

    0x1c0d6abc,// 13 PAY  10 

    0x56fdd043,// 14 PAY  11 

    0x1fac8bb7,// 15 PAY  12 

    0x14410302,// 16 PAY  13 

    0xc2a94fbe,// 17 PAY  14 

    0xd9b37e64,// 18 PAY  15 

    0xb87fa2ac,// 19 PAY  16 

    0xe2909a9c,// 20 PAY  17 

    0x51005c42,// 21 PAY  18 

    0x2b685b88,// 22 PAY  19 

    0x1e1fc261,// 23 PAY  20 

    0x9e8f0a77,// 24 PAY  21 

    0x02396858,// 25 PAY  22 

    0xfcfbafd1,// 26 PAY  23 

    0x62d68ec3,// 27 PAY  24 

    0x4452bcce,// 28 PAY  25 

    0x85d877c1,// 29 PAY  26 

    0xb7cd0e9a,// 30 PAY  27 

    0x4f2128c3,// 31 PAY  28 

    0xd7f0df3b,// 32 PAY  29 

    0x9cfe5c37,// 33 PAY  30 

    0x00296290,// 34 PAY  31 

    0x1b66a05f,// 35 PAY  32 

    0x358a7b5b,// 36 PAY  33 

    0xc37bb1d1,// 37 PAY  34 

    0x92a7f2ae,// 38 PAY  35 

    0x481a2345,// 39 PAY  36 

    0x5cce4970,// 40 PAY  37 

    0x728087c0,// 41 PAY  38 

    0x496f37bb,// 42 PAY  39 

    0xebf6a79c,// 43 PAY  40 

    0x743b0189,// 44 PAY  41 

    0x3203638d,// 45 PAY  42 

    0xb109173f,// 46 PAY  43 

    0xb2de2529,// 47 PAY  44 

    0x6538589d,// 48 PAY  45 

    0x32792dd8,// 49 PAY  46 

    0x261becdd,// 50 PAY  47 

    0x88b63b91,// 51 PAY  48 

    0xee07d1f3,// 52 PAY  49 

    0xbd82769a,// 53 PAY  50 

    0xc3c36afd,// 54 PAY  51 

    0xef3c2efc,// 55 PAY  52 

    0x44da342a,// 56 PAY  53 

    0x8f39640b,// 57 PAY  54 

    0x054a6833,// 58 PAY  55 

    0xda955570,// 59 PAY  56 

    0x4c161473,// 60 PAY  57 

    0xe804cc46,// 61 PAY  58 

    0x34f7b9dc,// 62 PAY  59 

    0x316b7868,// 63 PAY  60 

    0xe59f8e19,// 64 PAY  61 

    0x43b94440,// 65 PAY  62 

    0x8c5ace37,// 66 PAY  63 

    0x2a2c4270,// 67 PAY  64 

    0xa9e34013,// 68 PAY  65 

    0x6ff22d9a,// 69 PAY  66 

    0xccc8b33e,// 70 PAY  67 

    0x6af312d7,// 71 PAY  68 

    0x69f5c223,// 72 PAY  69 

    0xe9673f96,// 73 PAY  70 

    0xc765a5dc,// 74 PAY  71 

    0xfdbddf25,// 75 PAY  72 

    0x3a4a66e2,// 76 PAY  73 

    0xbfd9397f,// 77 PAY  74 

    0x6d04d6ed,// 78 PAY  75 

    0xee156db3,// 79 PAY  76 

    0xc6b3d068,// 80 PAY  77 

    0xf0586530,// 81 PAY  78 

    0x107d5ca5,// 82 PAY  79 

    0x350bed4c,// 83 PAY  80 

    0x9712c11d,// 84 PAY  81 

    0xf1933926,// 85 PAY  82 

    0x43d3391c,// 86 PAY  83 

    0x2874cdae,// 87 PAY  84 

    0x06fb1d5e,// 88 PAY  85 

    0x658dd663,// 89 PAY  86 

    0x12f18691,// 90 PAY  87 

    0xe1e83b96,// 91 PAY  88 

    0xac41b903,// 92 PAY  89 

    0xa634229f,// 93 PAY  90 

    0x9fc9ff1b,// 94 PAY  91 

    0x6ee412c3,// 95 PAY  92 

    0xd8ac86fd,// 96 PAY  93 

    0x13692168,// 97 PAY  94 

    0x7f7865e0,// 98 PAY  95 

    0xd6bd6e2a,// 99 PAY  96 

    0x500ccb23,// 100 PAY  97 

    0xa2f5eb38,// 101 PAY  98 

    0x9ce96c9f,// 102 PAY  99 

    0x36db1e76,// 103 PAY 100 

    0x301189a2,// 104 PAY 101 

    0x3c6bcc78,// 105 PAY 102 

    0x8da09311,// 106 PAY 103 

    0xd320dfdc,// 107 PAY 104 

    0x1948ee69,// 108 PAY 105 

    0xd4e8c936,// 109 PAY 106 

    0xba918cf6,// 110 PAY 107 

    0xd83f236f,// 111 PAY 108 

    0x720bbb66,// 112 PAY 109 

    0xc8f08686,// 113 PAY 110 

    0x40254c85,// 114 PAY 111 

    0xf60ed8e5,// 115 PAY 112 

    0x8dafbaea,// 116 PAY 113 

    0x056a0b9d,// 117 PAY 114 

    0xdb03feac,// 118 PAY 115 

    0x8fc113d4,// 119 PAY 116 

    0xb5966d3a,// 120 PAY 117 

    0xbdfdc0ff,// 121 PAY 118 

    0xace19ea0,// 122 PAY 119 

    0x5069d9bb,// 123 PAY 120 

    0xd05995db,// 124 PAY 121 

    0x126eff2d,// 125 PAY 122 

    0x2f97c239,// 126 PAY 123 

    0x752a1ee3,// 127 PAY 124 

    0xb1896c54,// 128 PAY 125 

    0x95fbd1d3,// 129 PAY 126 

    0x89f6d493,// 130 PAY 127 

    0x5ab83eec,// 131 PAY 128 

    0x86f48574,// 132 PAY 129 

    0xf0fe39ad,// 133 PAY 130 

    0x9c6c514b,// 134 PAY 131 

    0x5960b42e,// 135 PAY 132 

    0xc15ddc68,// 136 PAY 133 

    0x93d097cc,// 137 PAY 134 

    0x8c0a02f8,// 138 PAY 135 

    0x99a20910,// 139 PAY 136 

    0x95f6f49a,// 140 PAY 137 

    0x613c7fcf,// 141 PAY 138 

    0xd24368f1,// 142 PAY 139 

    0xd1b6112f,// 143 PAY 140 

    0x9dfcb027,// 144 PAY 141 

    0xbc8519bc,// 145 PAY 142 

    0xf2f3ab07,// 146 PAY 143 

    0xfc908edf,// 147 PAY 144 

    0xa472cf5c,// 148 PAY 145 

    0xeee49859,// 149 PAY 146 

    0xbfb8f865,// 150 PAY 147 

    0xc0362c5e,// 151 PAY 148 

    0xc08c0e90,// 152 PAY 149 

    0x8f738b53,// 153 PAY 150 

    0xfcdd7ee8,// 154 PAY 151 

    0x8a36879b,// 155 PAY 152 

    0x28de31da,// 156 PAY 153 

    0x1dbc9ef8,// 157 PAY 154 

    0x6a88ba7d,// 158 PAY 155 

    0xff3929a3,// 159 PAY 156 

    0xf2b4a4a4,// 160 PAY 157 

    0x39968a66,// 161 PAY 158 

    0x667c774e,// 162 PAY 159 

    0xbce425cf,// 163 PAY 160 

    0x19ee2e71,// 164 PAY 161 

    0x9d40b322,// 165 PAY 162 

    0x285f68a7,// 166 PAY 163 

    0xa368a411,// 167 PAY 164 

    0x71a4e664,// 168 PAY 165 

    0xb1f4bcb7,// 169 PAY 166 

    0x22a9bcba,// 170 PAY 167 

    0xccc93608,// 171 PAY 168 

    0xa737aa1d,// 172 PAY 169 

    0x0562fbbc,// 173 PAY 170 

    0xb851cd93,// 174 PAY 171 

    0x94989f55,// 175 PAY 172 

    0xc1d0c72a,// 176 PAY 173 

    0x84114788,// 177 PAY 174 

    0xa1faa567,// 178 PAY 175 

    0x5d812d80,// 179 PAY 176 

    0xba78fbb0,// 180 PAY 177 

    0x3e938c2c,// 181 PAY 178 

    0x62aa1334,// 182 PAY 179 

    0xda850d93,// 183 PAY 180 

    0xab488022,// 184 PAY 181 

    0x73e3e90e,// 185 PAY 182 

    0x19d898ca,// 186 PAY 183 

    0xcf94c55c,// 187 PAY 184 

    0x9c7d7d7b,// 188 PAY 185 

    0x9998307f,// 189 PAY 186 

    0xfca893cd,// 190 PAY 187 

    0x575dc9d6,// 191 PAY 188 

    0xcd72645d,// 192 PAY 189 

    0x0d191124,// 193 PAY 190 

    0xf5ca1ce3,// 194 PAY 191 

    0xf996f2d0,// 195 PAY 192 

    0x3370df6d,// 196 PAY 193 

    0xf113b63e,// 197 PAY 194 

    0xee4ad4b8,// 198 PAY 195 

    0x8ac7ad07,// 199 PAY 196 

    0x50fff347,// 200 PAY 197 

    0x3cde91ec,// 201 PAY 198 

    0xac10a702,// 202 PAY 199 

    0xd73f3f54,// 203 PAY 200 

    0x9b66fc60,// 204 PAY 201 

    0xa5da83b9,// 205 PAY 202 

    0x58cdaab5,// 206 PAY 203 

    0x1b17cfba,// 207 PAY 204 

    0x6d6067ab,// 208 PAY 205 

    0xbb282136,// 209 PAY 206 

    0xf0187f3f,// 210 PAY 207 

    0x4b8db4cb,// 211 PAY 208 

    0x5e88c1ec,// 212 PAY 209 

    0xfe6c3ecc,// 213 PAY 210 

    0xaf3efef8,// 214 PAY 211 

    0x19d1a1e1,// 215 PAY 212 

    0x8c85173e,// 216 PAY 213 

    0xe4cd2c8c,// 217 PAY 214 

    0xebb746e9,// 218 PAY 215 

    0xbc2e7ca9,// 219 PAY 216 

    0xf4c7c123,// 220 PAY 217 

    0x5310d75c,// 221 PAY 218 

    0xc299c910,// 222 PAY 219 

    0x9c760877,// 223 PAY 220 

    0x862d8817,// 224 PAY 221 

    0xf3c50a50,// 225 PAY 222 

    0xac2845b4,// 226 PAY 223 

    0x5bb8c238,// 227 PAY 224 

    0x3bdfee3f,// 228 PAY 225 

    0x677a75ab,// 229 PAY 226 

    0x78a8c9b5,// 230 PAY 227 

    0x8de39588,// 231 PAY 228 

    0xa6a45847,// 232 PAY 229 

    0xce31358a,// 233 PAY 230 

    0x5662bf6a,// 234 PAY 231 

    0x5dff60bf,// 235 PAY 232 

    0x5e6c1d73,// 236 PAY 233 

    0x0976b9cb,// 237 PAY 234 

    0x725b7da1,// 238 PAY 235 

    0x9edf4407,// 239 PAY 236 

    0x3a183ece,// 240 PAY 237 

    0xcb4098a9,// 241 PAY 238 

    0xcdcd3137,// 242 PAY 239 

    0x11eb6029,// 243 PAY 240 

    0xf154bd00,// 244 PAY 241 

    0xf23392ee,// 245 PAY 242 

    0x22c08c3c,// 246 PAY 243 

    0xcdc6320f,// 247 PAY 244 

    0x04bd868a,// 248 PAY 245 

    0xb750ecdb,// 249 PAY 246 

    0x4b6086be,// 250 PAY 247 

    0xcc5e4f73,// 251 PAY 248 

    0xb2e9e0ab,// 252 PAY 249 

    0x1755aa9d,// 253 PAY 250 

    0x0ac6eaeb,// 254 PAY 251 

    0x94c3770b,// 255 PAY 252 

    0x86cb6e81,// 256 PAY 253 

    0xb8cd8e4f,// 257 PAY 254 

    0x719efe6c,// 258 PAY 255 

    0xa58cfe9b,// 259 PAY 256 

    0x04fd16fe,// 260 PAY 257 

    0xae28e062,// 261 PAY 258 

    0x80d1b8dc,// 262 PAY 259 

    0x7f2e3748,// 263 PAY 260 

    0xe2850d58,// 264 PAY 261 

    0x14dd0e3b,// 265 PAY 262 

    0xedd78376,// 266 PAY 263 

    0x13200c6c,// 267 PAY 264 

    0x8fae6e7e,// 268 PAY 265 

    0x8cf7c6d2,// 269 PAY 266 

    0xc2b7917b,// 270 PAY 267 

    0x3def94f5,// 271 PAY 268 

    0x64292426,// 272 PAY 269 

    0x69308a20,// 273 PAY 270 

    0xdbf5468a,// 274 PAY 271 

    0x11060698,// 275 PAY 272 

    0xbd98a504,// 276 PAY 273 

    0x1c5fd32a,// 277 PAY 274 

    0xf44d2bf7,// 278 PAY 275 

    0x5846b559,// 279 PAY 276 

    0xa3829d5f,// 280 PAY 277 

    0x7210f4ea,// 281 PAY 278 

    0xbbcd4b7f,// 282 PAY 279 

    0x921ec853,// 283 PAY 280 

    0xef599a2e,// 284 PAY 281 

    0xd8d32fc7,// 285 PAY 282 

    0x5c317332,// 286 PAY 283 

    0xdc0de57f,// 287 PAY 284 

    0x25589b45,// 288 PAY 285 

    0xe9408766,// 289 PAY 286 

    0xc538b8f5,// 290 PAY 287 

    0x643968e3,// 291 PAY 288 

    0x074dd7fe,// 292 PAY 289 

    0xbda0d605,// 293 PAY 290 

    0x951653e1,// 294 PAY 291 

    0xacfe4281,// 295 PAY 292 

    0x504095f8,// 296 PAY 293 

    0xcc2bf28e,// 297 PAY 294 

    0x807ad5d0,// 298 PAY 295 

    0x1f9ac667,// 299 PAY 296 

    0xc86dce25,// 300 PAY 297 

    0x546ce476,// 301 PAY 298 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 131 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbb 

    0x020cbb33 // 302 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt65_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 335 words. 

/// BDA size     is 1334 (0x536) 

/// BDA id       is 0x5fa4 

    0x05365fa4,// 3 BDA   1 

/// PAY Generic Data size   : 1334 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa0e82fab,// 4 PAY   1 

    0x0ee756b1,// 5 PAY   2 

    0x835b2fce,// 6 PAY   3 

    0x58dc28f5,// 7 PAY   4 

    0x1a04855f,// 8 PAY   5 

    0x2cfa9fe5,// 9 PAY   6 

    0x2acb638b,// 10 PAY   7 

    0xbab1dba4,// 11 PAY   8 

    0x7f62e4d8,// 12 PAY   9 

    0xe451be7a,// 13 PAY  10 

    0x5dc3cc21,// 14 PAY  11 

    0xd010032b,// 15 PAY  12 

    0xcc46a2cd,// 16 PAY  13 

    0xd1e62e69,// 17 PAY  14 

    0x8ce63fe1,// 18 PAY  15 

    0x2c691697,// 19 PAY  16 

    0x81566733,// 20 PAY  17 

    0x8d115d5e,// 21 PAY  18 

    0x39e127f0,// 22 PAY  19 

    0x1371654c,// 23 PAY  20 

    0x927e3a62,// 24 PAY  21 

    0x29763b23,// 25 PAY  22 

    0xe6729bfc,// 26 PAY  23 

    0x1baf540e,// 27 PAY  24 

    0xc882e604,// 28 PAY  25 

    0xdd45e98a,// 29 PAY  26 

    0x00b89cb4,// 30 PAY  27 

    0x67c55f97,// 31 PAY  28 

    0xcc827b6c,// 32 PAY  29 

    0xb49af594,// 33 PAY  30 

    0x20ddff48,// 34 PAY  31 

    0x3673d702,// 35 PAY  32 

    0x92ca917f,// 36 PAY  33 

    0xfe22320d,// 37 PAY  34 

    0xa3e6af44,// 38 PAY  35 

    0xfdd90ffe,// 39 PAY  36 

    0x80eda8f8,// 40 PAY  37 

    0x774d669b,// 41 PAY  38 

    0x58d42fe8,// 42 PAY  39 

    0x13eaf992,// 43 PAY  40 

    0x925096aa,// 44 PAY  41 

    0x453f900d,// 45 PAY  42 

    0x26fa0728,// 46 PAY  43 

    0x31ace57f,// 47 PAY  44 

    0xeae92119,// 48 PAY  45 

    0x56d12f8b,// 49 PAY  46 

    0x4e5dc674,// 50 PAY  47 

    0xa7bb57ba,// 51 PAY  48 

    0x36471244,// 52 PAY  49 

    0x1bfbdbec,// 53 PAY  50 

    0xb157d828,// 54 PAY  51 

    0xf3ca5a60,// 55 PAY  52 

    0x3057fd33,// 56 PAY  53 

    0x50180e66,// 57 PAY  54 

    0xbb53d57c,// 58 PAY  55 

    0x24ae5e80,// 59 PAY  56 

    0x9e3794e1,// 60 PAY  57 

    0x19fa65d0,// 61 PAY  58 

    0x3fef41fd,// 62 PAY  59 

    0xe217afb2,// 63 PAY  60 

    0x27efb570,// 64 PAY  61 

    0x2de9fe25,// 65 PAY  62 

    0x33aa55dc,// 66 PAY  63 

    0x6e2666b9,// 67 PAY  64 

    0xa5911ab3,// 68 PAY  65 

    0x6e2685d5,// 69 PAY  66 

    0x781efaed,// 70 PAY  67 

    0x9264db0d,// 71 PAY  68 

    0xd615e4de,// 72 PAY  69 

    0x487623bb,// 73 PAY  70 

    0x2a1fd7a2,// 74 PAY  71 

    0x9f902ab6,// 75 PAY  72 

    0x1666ac40,// 76 PAY  73 

    0x958204a5,// 77 PAY  74 

    0xf15d0151,// 78 PAY  75 

    0xc7a2cf44,// 79 PAY  76 

    0x51a5e481,// 80 PAY  77 

    0xc2c4a620,// 81 PAY  78 

    0xb66d68b6,// 82 PAY  79 

    0xfd4ab9c5,// 83 PAY  80 

    0xb1df317f,// 84 PAY  81 

    0x597e6f95,// 85 PAY  82 

    0x8ed55b82,// 86 PAY  83 

    0x83bf0530,// 87 PAY  84 

    0x5b07d7ac,// 88 PAY  85 

    0xc4ac7b5a,// 89 PAY  86 

    0x102c1a2f,// 90 PAY  87 

    0x317eca47,// 91 PAY  88 

    0x6ebec1c4,// 92 PAY  89 

    0xcdeae49c,// 93 PAY  90 

    0xe9379e5b,// 94 PAY  91 

    0xa5744c2f,// 95 PAY  92 

    0x9e866062,// 96 PAY  93 

    0x5abb80f7,// 97 PAY  94 

    0xc95a1666,// 98 PAY  95 

    0x8984a06a,// 99 PAY  96 

    0x0df22017,// 100 PAY  97 

    0x4f2da423,// 101 PAY  98 

    0x0b3b7be8,// 102 PAY  99 

    0x75f6d42e,// 103 PAY 100 

    0x6f0cb94d,// 104 PAY 101 

    0xf9b04fff,// 105 PAY 102 

    0x09c357e5,// 106 PAY 103 

    0x42a357c6,// 107 PAY 104 

    0xd69e9cd1,// 108 PAY 105 

    0x10b58799,// 109 PAY 106 

    0x39fa8472,// 110 PAY 107 

    0xd8be9594,// 111 PAY 108 

    0x09c2311e,// 112 PAY 109 

    0xb5932950,// 113 PAY 110 

    0xebc1b23f,// 114 PAY 111 

    0x67c72715,// 115 PAY 112 

    0xebf30681,// 116 PAY 113 

    0x1a19440d,// 117 PAY 114 

    0xdffac3d0,// 118 PAY 115 

    0xb8c3fad9,// 119 PAY 116 

    0x203f3341,// 120 PAY 117 

    0x26c8abac,// 121 PAY 118 

    0xbc17b327,// 122 PAY 119 

    0x5bacc12b,// 123 PAY 120 

    0xb5138e72,// 124 PAY 121 

    0x0b2ebbb3,// 125 PAY 122 

    0x9d2a08e3,// 126 PAY 123 

    0x7889a8ac,// 127 PAY 124 

    0x178152cb,// 128 PAY 125 

    0xa8a454aa,// 129 PAY 126 

    0x6e06ea3b,// 130 PAY 127 

    0x9957bb6e,// 131 PAY 128 

    0x54741b19,// 132 PAY 129 

    0x668985d6,// 133 PAY 130 

    0xc8001791,// 134 PAY 131 

    0x794403c4,// 135 PAY 132 

    0x725b1e39,// 136 PAY 133 

    0x4c3b0322,// 137 PAY 134 

    0xdaa953b8,// 138 PAY 135 

    0x981fad1e,// 139 PAY 136 

    0xecb36528,// 140 PAY 137 

    0xf0f3e034,// 141 PAY 138 

    0x13652866,// 142 PAY 139 

    0xc1b33731,// 143 PAY 140 

    0xd6adb163,// 144 PAY 141 

    0x75eca1ba,// 145 PAY 142 

    0xc0a31fe9,// 146 PAY 143 

    0x5a25d85a,// 147 PAY 144 

    0xe149c162,// 148 PAY 145 

    0x6ebe48bf,// 149 PAY 146 

    0xc2a4c04d,// 150 PAY 147 

    0xe886f340,// 151 PAY 148 

    0x3fc7641d,// 152 PAY 149 

    0xf0dc0282,// 153 PAY 150 

    0x0d411be4,// 154 PAY 151 

    0xa43fa97b,// 155 PAY 152 

    0xe0d1eec0,// 156 PAY 153 

    0xf296717a,// 157 PAY 154 

    0x65f2f24c,// 158 PAY 155 

    0xf8c26950,// 159 PAY 156 

    0x04ae87db,// 160 PAY 157 

    0x047f75e3,// 161 PAY 158 

    0x9416cb0c,// 162 PAY 159 

    0xe1de851b,// 163 PAY 160 

    0xc2733fa8,// 164 PAY 161 

    0xb33c7819,// 165 PAY 162 

    0x186a7b16,// 166 PAY 163 

    0x0ad993cd,// 167 PAY 164 

    0x7e5a5310,// 168 PAY 165 

    0x860e25eb,// 169 PAY 166 

    0x784f108b,// 170 PAY 167 

    0x9ce717d8,// 171 PAY 168 

    0x2fb8bf22,// 172 PAY 169 

    0x7f3794d9,// 173 PAY 170 

    0xb0a02aec,// 174 PAY 171 

    0x98b5a107,// 175 PAY 172 

    0x17eede20,// 176 PAY 173 

    0x39556c13,// 177 PAY 174 

    0x6cf73a9c,// 178 PAY 175 

    0x1cb33009,// 179 PAY 176 

    0xb36b2337,// 180 PAY 177 

    0x633550f1,// 181 PAY 178 

    0xd2cdbf6b,// 182 PAY 179 

    0xb62cd6f1,// 183 PAY 180 

    0xb52bca94,// 184 PAY 181 

    0xc2fa7277,// 185 PAY 182 

    0x7d685118,// 186 PAY 183 

    0xfdc77db7,// 187 PAY 184 

    0x0d697842,// 188 PAY 185 

    0x666066f0,// 189 PAY 186 

    0x2b166ff9,// 190 PAY 187 

    0x792596ba,// 191 PAY 188 

    0xc941b66b,// 192 PAY 189 

    0x59586393,// 193 PAY 190 

    0x52c8b417,// 194 PAY 191 

    0x8ae6d2c1,// 195 PAY 192 

    0x44aa6d88,// 196 PAY 193 

    0x2a3745e0,// 197 PAY 194 

    0x0ab5f4e1,// 198 PAY 195 

    0x300040f1,// 199 PAY 196 

    0xe5a1e34f,// 200 PAY 197 

    0x3964a54b,// 201 PAY 198 

    0x282b630a,// 202 PAY 199 

    0xd39ddc97,// 203 PAY 200 

    0xcea0b2af,// 204 PAY 201 

    0xf4aec525,// 205 PAY 202 

    0x8ff12767,// 206 PAY 203 

    0x30ced89f,// 207 PAY 204 

    0x9648745e,// 208 PAY 205 

    0x240f0d56,// 209 PAY 206 

    0x92f50597,// 210 PAY 207 

    0x6ba2d1a5,// 211 PAY 208 

    0x973739ff,// 212 PAY 209 

    0xcad24cba,// 213 PAY 210 

    0xc6aac81a,// 214 PAY 211 

    0xfa651065,// 215 PAY 212 

    0x824008f2,// 216 PAY 213 

    0x5af4ba8a,// 217 PAY 214 

    0xf86ca7de,// 218 PAY 215 

    0xbfe2b849,// 219 PAY 216 

    0xebcba6c3,// 220 PAY 217 

    0x3f1c530c,// 221 PAY 218 

    0xe6d7b8ed,// 222 PAY 219 

    0x44820cdf,// 223 PAY 220 

    0x082419ba,// 224 PAY 221 

    0x71b08d7e,// 225 PAY 222 

    0xafc2478a,// 226 PAY 223 

    0x60ec8482,// 227 PAY 224 

    0xe3947f28,// 228 PAY 225 

    0x0933f155,// 229 PAY 226 

    0x9f3f0a7a,// 230 PAY 227 

    0x660f5226,// 231 PAY 228 

    0x6cd26d2b,// 232 PAY 229 

    0x3ff158a8,// 233 PAY 230 

    0xe134f01d,// 234 PAY 231 

    0x51fd036b,// 235 PAY 232 

    0x803364d1,// 236 PAY 233 

    0xa0381744,// 237 PAY 234 

    0x96594988,// 238 PAY 235 

    0x0cead9c0,// 239 PAY 236 

    0x9f8f7033,// 240 PAY 237 

    0xa871c9bc,// 241 PAY 238 

    0x5655fc46,// 242 PAY 239 

    0x55cdbf54,// 243 PAY 240 

    0x5b2193fb,// 244 PAY 241 

    0xda924f83,// 245 PAY 242 

    0xff5fe806,// 246 PAY 243 

    0x010ce3b8,// 247 PAY 244 

    0x2f4e0d90,// 248 PAY 245 

    0x3050b54c,// 249 PAY 246 

    0xe5b7414e,// 250 PAY 247 

    0x75a39ec6,// 251 PAY 248 

    0xccfd1028,// 252 PAY 249 

    0xe324fdd8,// 253 PAY 250 

    0xbe925b09,// 254 PAY 251 

    0xb3356859,// 255 PAY 252 

    0x2981b477,// 256 PAY 253 

    0x712a3aa4,// 257 PAY 254 

    0x07394cfc,// 258 PAY 255 

    0x91aff1b3,// 259 PAY 256 

    0x3f4888cb,// 260 PAY 257 

    0x56403f25,// 261 PAY 258 

    0x9e92171e,// 262 PAY 259 

    0xefa85b9b,// 263 PAY 260 

    0x326fe355,// 264 PAY 261 

    0x6ceb6e20,// 265 PAY 262 

    0x778cb3e7,// 266 PAY 263 

    0xd36b9772,// 267 PAY 264 

    0x602891c6,// 268 PAY 265 

    0xc207658d,// 269 PAY 266 

    0x3d212ead,// 270 PAY 267 

    0xa7d619f2,// 271 PAY 268 

    0x2616e394,// 272 PAY 269 

    0xbfe7ad24,// 273 PAY 270 

    0x8e6d0bac,// 274 PAY 271 

    0x5d00317d,// 275 PAY 272 

    0x6f48a1f7,// 276 PAY 273 

    0xe2ce266b,// 277 PAY 274 

    0x4ff4a92b,// 278 PAY 275 

    0x1cfd7fd4,// 279 PAY 276 

    0xcb2cba90,// 280 PAY 277 

    0x4251e762,// 281 PAY 278 

    0x6db075a9,// 282 PAY 279 

    0xd6bbc8af,// 283 PAY 280 

    0x177e284b,// 284 PAY 281 

    0x75d3d8bb,// 285 PAY 282 

    0x899938d6,// 286 PAY 283 

    0x1f374566,// 287 PAY 284 

    0xcd5d78ef,// 288 PAY 285 

    0x1ad7c31a,// 289 PAY 286 

    0xefcafd26,// 290 PAY 287 

    0x4c8b7e68,// 291 PAY 288 

    0xa2c38a6a,// 292 PAY 289 

    0xa9a9ba39,// 293 PAY 290 

    0x964d0e83,// 294 PAY 291 

    0x7281077b,// 295 PAY 292 

    0x33a7509b,// 296 PAY 293 

    0xc187ca2a,// 297 PAY 294 

    0x531e7512,// 298 PAY 295 

    0x9510a407,// 299 PAY 296 

    0x626a7b3b,// 300 PAY 297 

    0xa9069a01,// 301 PAY 298 

    0xf0f01534,// 302 PAY 299 

    0x19841239,// 303 PAY 300 

    0x55af6c9e,// 304 PAY 301 

    0x0b1c1f67,// 305 PAY 302 

    0xd3af9e49,// 306 PAY 303 

    0x52e5f12f,// 307 PAY 304 

    0xf3f3c25c,// 308 PAY 305 

    0xf315d661,// 309 PAY 306 

    0xd1f168eb,// 310 PAY 307 

    0x48e3c554,// 311 PAY 308 

    0x774c698d,// 312 PAY 309 

    0x5b576c5f,// 313 PAY 310 

    0x93f0c9a2,// 314 PAY 311 

    0x59ee121c,// 315 PAY 312 

    0xbafaf42f,// 316 PAY 313 

    0xc13a386f,// 317 PAY 314 

    0xcbf81839,// 318 PAY 315 

    0x1202e478,// 319 PAY 316 

    0xe52ed787,// 320 PAY 317 

    0x5cd180cf,// 321 PAY 318 

    0xe7f4ff41,// 322 PAY 319 

    0x4afb77e7,// 323 PAY 320 

    0xfa6c750a,// 324 PAY 321 

    0x85e86d16,// 325 PAY 322 

    0xc3075d99,// 326 PAY 323 

    0xcfe904bd,// 327 PAY 324 

    0xff84d7c2,// 328 PAY 325 

    0xf9ef90a6,// 329 PAY 326 

    0x40c4edfc,// 330 PAY 327 

    0x184a7758,// 331 PAY 328 

    0x7bb99730,// 332 PAY 329 

    0x51d3bd5c,// 333 PAY 330 

    0x06709dc4,// 334 PAY 331 

    0x6f817624,// 335 PAY 332 

    0x92b0c597,// 336 PAY 333 

    0x23020000,// 337 PAY 334 

/// HASH is  16 bytes 

    0xff84d7c2,// 338 HSH   1 

    0xf9ef90a6,// 339 HSH   2 

    0x40c4edfc,// 340 HSH   3 

    0x184a7758,// 341 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x45 

    0x00d44533 // 342 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt66_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 287 words. 

/// BDA size     is 1143 (0x477) 

/// BDA id       is 0xb717 

    0x0477b717,// 3 BDA   1 

/// PAY Generic Data size   : 1143 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x891ff451,// 4 PAY   1 

    0x1dccd13f,// 5 PAY   2 

    0x00eb2dd8,// 6 PAY   3 

    0x5ce548e0,// 7 PAY   4 

    0xb57fc470,// 8 PAY   5 

    0x1f5e45c0,// 9 PAY   6 

    0x0e3bd5b4,// 10 PAY   7 

    0x40f8c8c6,// 11 PAY   8 

    0x040b1128,// 12 PAY   9 

    0xd08ceee5,// 13 PAY  10 

    0x5ccb141d,// 14 PAY  11 

    0x6918058c,// 15 PAY  12 

    0x489f9869,// 16 PAY  13 

    0xe15f9111,// 17 PAY  14 

    0x63d8780c,// 18 PAY  15 

    0x8b7fe659,// 19 PAY  16 

    0xe614c8c6,// 20 PAY  17 

    0x0af6b299,// 21 PAY  18 

    0x649e241d,// 22 PAY  19 

    0x9c9e9a56,// 23 PAY  20 

    0xdfe47dc8,// 24 PAY  21 

    0xcab1e5bf,// 25 PAY  22 

    0xbe376eca,// 26 PAY  23 

    0x92f3d1c8,// 27 PAY  24 

    0x71f97929,// 28 PAY  25 

    0xdddfee3c,// 29 PAY  26 

    0x6d4e0042,// 30 PAY  27 

    0xdc0c1f71,// 31 PAY  28 

    0x9a679176,// 32 PAY  29 

    0xbf30fa37,// 33 PAY  30 

    0xf5fcdd84,// 34 PAY  31 

    0x02df1408,// 35 PAY  32 

    0xdafbc7b1,// 36 PAY  33 

    0xa3e9aa90,// 37 PAY  34 

    0x77a96aa2,// 38 PAY  35 

    0x0ea079a0,// 39 PAY  36 

    0x7deaa16f,// 40 PAY  37 

    0x0e112894,// 41 PAY  38 

    0x6e5f1e4f,// 42 PAY  39 

    0x1c7bb2a5,// 43 PAY  40 

    0xa22080b3,// 44 PAY  41 

    0x88ec7603,// 45 PAY  42 

    0x3f7ce6ef,// 46 PAY  43 

    0xf4fca6f1,// 47 PAY  44 

    0x9a435fc9,// 48 PAY  45 

    0x811a611e,// 49 PAY  46 

    0x3c73849c,// 50 PAY  47 

    0x25e8e5e7,// 51 PAY  48 

    0xd0f490c2,// 52 PAY  49 

    0x204c4745,// 53 PAY  50 

    0xb924c611,// 54 PAY  51 

    0x66c800af,// 55 PAY  52 

    0x456be0d6,// 56 PAY  53 

    0x18b37f44,// 57 PAY  54 

    0x57d5e50f,// 58 PAY  55 

    0x15aec892,// 59 PAY  56 

    0x2d71afac,// 60 PAY  57 

    0x5d985ef0,// 61 PAY  58 

    0x637f5695,// 62 PAY  59 

    0x80f89b91,// 63 PAY  60 

    0x3ee1533a,// 64 PAY  61 

    0xc19854d3,// 65 PAY  62 

    0xd0769645,// 66 PAY  63 

    0x2da4b5db,// 67 PAY  64 

    0xbee631e3,// 68 PAY  65 

    0xd91cf5bf,// 69 PAY  66 

    0xf3ce2b4a,// 70 PAY  67 

    0x3bee34ab,// 71 PAY  68 

    0xb320fa5b,// 72 PAY  69 

    0x1796ae18,// 73 PAY  70 

    0xb698f1c5,// 74 PAY  71 

    0x476b57e8,// 75 PAY  72 

    0x14653fcd,// 76 PAY  73 

    0xb3dc58b7,// 77 PAY  74 

    0x9ac96ce3,// 78 PAY  75 

    0x629787a7,// 79 PAY  76 

    0xf66568f0,// 80 PAY  77 

    0xa83ed914,// 81 PAY  78 

    0x45a9295c,// 82 PAY  79 

    0x8e768f10,// 83 PAY  80 

    0x497a473e,// 84 PAY  81 

    0xfe485afc,// 85 PAY  82 

    0xa0e50bbd,// 86 PAY  83 

    0x835a00ee,// 87 PAY  84 

    0x4f0e864f,// 88 PAY  85 

    0x99366bdf,// 89 PAY  86 

    0x4026a268,// 90 PAY  87 

    0xc21b383a,// 91 PAY  88 

    0x09d5ba9e,// 92 PAY  89 

    0x620913b3,// 93 PAY  90 

    0xbf05dfe2,// 94 PAY  91 

    0x75c8f7ea,// 95 PAY  92 

    0x983d1394,// 96 PAY  93 

    0x6e32a371,// 97 PAY  94 

    0x5d08ba9a,// 98 PAY  95 

    0x12e528e3,// 99 PAY  96 

    0x9f881c2a,// 100 PAY  97 

    0xca0664f8,// 101 PAY  98 

    0x1297bf68,// 102 PAY  99 

    0xb62d3eb3,// 103 PAY 100 

    0xb7db6827,// 104 PAY 101 

    0x76622e61,// 105 PAY 102 

    0x19861dee,// 106 PAY 103 

    0x8c2476a4,// 107 PAY 104 

    0xe9243794,// 108 PAY 105 

    0x9d86996c,// 109 PAY 106 

    0xe64f0a2d,// 110 PAY 107 

    0x0a4eb5f6,// 111 PAY 108 

    0x092333b0,// 112 PAY 109 

    0x2f009ce8,// 113 PAY 110 

    0x38460f8b,// 114 PAY 111 

    0x0879f970,// 115 PAY 112 

    0x3d68f0dc,// 116 PAY 113 

    0x0d3b02a2,// 117 PAY 114 

    0x55821923,// 118 PAY 115 

    0x2dd04c03,// 119 PAY 116 

    0xc7047fc2,// 120 PAY 117 

    0x5f4b9858,// 121 PAY 118 

    0x3b1920c3,// 122 PAY 119 

    0x26497a2e,// 123 PAY 120 

    0x3686955a,// 124 PAY 121 

    0x6b54016e,// 125 PAY 122 

    0xeb395c38,// 126 PAY 123 

    0x82c30fdf,// 127 PAY 124 

    0xd2375ffc,// 128 PAY 125 

    0xf48292e3,// 129 PAY 126 

    0xf22d2879,// 130 PAY 127 

    0x8b344cbe,// 131 PAY 128 

    0x4fd3c65a,// 132 PAY 129 

    0xd1701901,// 133 PAY 130 

    0x6fa80643,// 134 PAY 131 

    0xcff59b85,// 135 PAY 132 

    0x86875480,// 136 PAY 133 

    0xdec83929,// 137 PAY 134 

    0x96a268d1,// 138 PAY 135 

    0xdecffb29,// 139 PAY 136 

    0x3f0383b8,// 140 PAY 137 

    0xb5cd0f78,// 141 PAY 138 

    0x8a46879c,// 142 PAY 139 

    0x4670c4d8,// 143 PAY 140 

    0x943020ce,// 144 PAY 141 

    0x6b6d49ff,// 145 PAY 142 

    0x2e22d337,// 146 PAY 143 

    0x68f079e8,// 147 PAY 144 

    0x346a862a,// 148 PAY 145 

    0xf72519fa,// 149 PAY 146 

    0x9f0403fd,// 150 PAY 147 

    0x0fba535e,// 151 PAY 148 

    0xdc84fb40,// 152 PAY 149 

    0x4dd6e941,// 153 PAY 150 

    0x04dc387a,// 154 PAY 151 

    0x2a7f1d26,// 155 PAY 152 

    0x97725893,// 156 PAY 153 

    0xc1462b0e,// 157 PAY 154 

    0xceab12d9,// 158 PAY 155 

    0x0f694a31,// 159 PAY 156 

    0xbbfdb928,// 160 PAY 157 

    0x30ea9923,// 161 PAY 158 

    0x8efaaab1,// 162 PAY 159 

    0x01fd9075,// 163 PAY 160 

    0x7ca7d515,// 164 PAY 161 

    0x27f9eef2,// 165 PAY 162 

    0x1c03a2c3,// 166 PAY 163 

    0x7aae0306,// 167 PAY 164 

    0xeb2564cb,// 168 PAY 165 

    0x7003ae1c,// 169 PAY 166 

    0xfa6ef326,// 170 PAY 167 

    0x54d61e10,// 171 PAY 168 

    0x068c4a02,// 172 PAY 169 

    0x3a247e40,// 173 PAY 170 

    0x377e480c,// 174 PAY 171 

    0x8a5e6c6d,// 175 PAY 172 

    0xb03e7582,// 176 PAY 173 

    0x96c74b4d,// 177 PAY 174 

    0xd1e957a5,// 178 PAY 175 

    0xfbd8e4a9,// 179 PAY 176 

    0xfc2ae8dd,// 180 PAY 177 

    0x0bc079da,// 181 PAY 178 

    0x2472bbfc,// 182 PAY 179 

    0x8a82ed06,// 183 PAY 180 

    0x957b7a86,// 184 PAY 181 

    0x87f1c1ba,// 185 PAY 182 

    0xf9740eae,// 186 PAY 183 

    0x09f019e2,// 187 PAY 184 

    0x1c4489d3,// 188 PAY 185 

    0x921dc9b5,// 189 PAY 186 

    0x9555b0ac,// 190 PAY 187 

    0x3dfda80d,// 191 PAY 188 

    0x5f1ddcdd,// 192 PAY 189 

    0x4f884715,// 193 PAY 190 

    0x2e10f957,// 194 PAY 191 

    0x1b395cc4,// 195 PAY 192 

    0x7a3f87cd,// 196 PAY 193 

    0xb7ed518d,// 197 PAY 194 

    0x44197a5f,// 198 PAY 195 

    0x1183e6d6,// 199 PAY 196 

    0x583ab47c,// 200 PAY 197 

    0xe69cbe72,// 201 PAY 198 

    0x0138bcef,// 202 PAY 199 

    0x265c9899,// 203 PAY 200 

    0xee63c8cd,// 204 PAY 201 

    0x3407d6d4,// 205 PAY 202 

    0x5418b4fb,// 206 PAY 203 

    0xd26e54f1,// 207 PAY 204 

    0xac78b100,// 208 PAY 205 

    0x039c1bfe,// 209 PAY 206 

    0x922220db,// 210 PAY 207 

    0x581fb251,// 211 PAY 208 

    0x12af1350,// 212 PAY 209 

    0xbd20f4a1,// 213 PAY 210 

    0xf2e880e7,// 214 PAY 211 

    0x37506ee5,// 215 PAY 212 

    0x289709f7,// 216 PAY 213 

    0x6bd1facf,// 217 PAY 214 

    0xec730964,// 218 PAY 215 

    0x81e5c739,// 219 PAY 216 

    0xed5332fa,// 220 PAY 217 

    0x59628768,// 221 PAY 218 

    0x2892f1f3,// 222 PAY 219 

    0x0379637a,// 223 PAY 220 

    0xe3e57aef,// 224 PAY 221 

    0xe3f2e90b,// 225 PAY 222 

    0xfaf48920,// 226 PAY 223 

    0xd1f2261e,// 227 PAY 224 

    0x8e06ac90,// 228 PAY 225 

    0xfbf3389c,// 229 PAY 226 

    0xa42b4931,// 230 PAY 227 

    0x41e27537,// 231 PAY 228 

    0x8f9f44b6,// 232 PAY 229 

    0x88d20462,// 233 PAY 230 

    0xcbe52eb8,// 234 PAY 231 

    0x3ea27e10,// 235 PAY 232 

    0x6d800fe2,// 236 PAY 233 

    0x0cdb4189,// 237 PAY 234 

    0xb6fe452b,// 238 PAY 235 

    0xe96f01a4,// 239 PAY 236 

    0x753cea35,// 240 PAY 237 

    0x6f7233ca,// 241 PAY 238 

    0xfa0ff0b9,// 242 PAY 239 

    0xc68f8e60,// 243 PAY 240 

    0xab24714b,// 244 PAY 241 

    0xa7dc0697,// 245 PAY 242 

    0xfe8380bf,// 246 PAY 243 

    0x92ba3580,// 247 PAY 244 

    0x85a4dd7a,// 248 PAY 245 

    0xe034bf47,// 249 PAY 246 

    0x53b38429,// 250 PAY 247 

    0x859f18aa,// 251 PAY 248 

    0xdf8f9244,// 252 PAY 249 

    0x0681ab79,// 253 PAY 250 

    0xafca0efd,// 254 PAY 251 

    0x892c3568,// 255 PAY 252 

    0xa97f9da7,// 256 PAY 253 

    0x31afbd38,// 257 PAY 254 

    0x6560220c,// 258 PAY 255 

    0xf67191ea,// 259 PAY 256 

    0xd8f1778e,// 260 PAY 257 

    0x4354704f,// 261 PAY 258 

    0xff8886b4,// 262 PAY 259 

    0xc687062b,// 263 PAY 260 

    0xb0167870,// 264 PAY 261 

    0x71ee1fef,// 265 PAY 262 

    0xb9311fde,// 266 PAY 263 

    0x05699bda,// 267 PAY 264 

    0x3c7ebb60,// 268 PAY 265 

    0xaed44c3c,// 269 PAY 266 

    0xadb8a9ea,// 270 PAY 267 

    0x69fd67a0,// 271 PAY 268 

    0x68a89e63,// 272 PAY 269 

    0x2525c0ab,// 273 PAY 270 

    0xa87026ef,// 274 PAY 271 

    0xd2b87c8e,// 275 PAY 272 

    0xf0adc41a,// 276 PAY 273 

    0x9890b1d0,// 277 PAY 274 

    0x40cfd156,// 278 PAY 275 

    0xd9205982,// 279 PAY 276 

    0x228553f2,// 280 PAY 277 

    0x89b70d46,// 281 PAY 278 

    0xd724fae6,// 282 PAY 279 

    0xfdc91c07,// 283 PAY 280 

    0xf9d7021b,// 284 PAY 281 

    0xeb154e50,// 285 PAY 282 

    0x22d167a9,// 286 PAY 283 

    0xc7a73220,// 287 PAY 284 

    0x7492d5d6,// 288 PAY 285 

    0xa2aa9e00,// 289 PAY 286 

/// HASH is  8 bytes 

    0xe034bf47,// 290 HSH   1 

    0x53b38429,// 291 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 91 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x83 

    0x01e8835b // 292 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt67_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 210 words. 

/// BDA size     is 836 (0x344) 

/// BDA id       is 0xa522 

    0x0344a522,// 3 BDA   1 

/// PAY Generic Data size   : 836 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xfdc0b64a,// 4 PAY   1 

    0x48e46671,// 5 PAY   2 

    0x9a756873,// 6 PAY   3 

    0x3f226706,// 7 PAY   4 

    0x96463d9c,// 8 PAY   5 

    0x6646e824,// 9 PAY   6 

    0xaccfaf32,// 10 PAY   7 

    0xcd52728a,// 11 PAY   8 

    0x83da1991,// 12 PAY   9 

    0x7ef8d782,// 13 PAY  10 

    0xf71c9f37,// 14 PAY  11 

    0x0dee2f88,// 15 PAY  12 

    0x91dc4dfa,// 16 PAY  13 

    0x2815d9a5,// 17 PAY  14 

    0x57af7747,// 18 PAY  15 

    0xd0a8b854,// 19 PAY  16 

    0xf6634ace,// 20 PAY  17 

    0xaccc4e69,// 21 PAY  18 

    0x718c79c1,// 22 PAY  19 

    0xf7122a2b,// 23 PAY  20 

    0xffa2d6f5,// 24 PAY  21 

    0xcda60460,// 25 PAY  22 

    0x36ab0586,// 26 PAY  23 

    0x5d2b5dc5,// 27 PAY  24 

    0xf2d24118,// 28 PAY  25 

    0xaec89d14,// 29 PAY  26 

    0x089f889d,// 30 PAY  27 

    0x83bfc958,// 31 PAY  28 

    0x64a85ad4,// 32 PAY  29 

    0x7e005193,// 33 PAY  30 

    0x53f0f671,// 34 PAY  31 

    0x31e898f7,// 35 PAY  32 

    0x5284b291,// 36 PAY  33 

    0x4589e395,// 37 PAY  34 

    0xfb12d12a,// 38 PAY  35 

    0x403e3b50,// 39 PAY  36 

    0x451b819a,// 40 PAY  37 

    0xa9dfdd3f,// 41 PAY  38 

    0xb2b038e4,// 42 PAY  39 

    0xe74af1c2,// 43 PAY  40 

    0xce3eaa54,// 44 PAY  41 

    0x548d87c8,// 45 PAY  42 

    0xfb92c583,// 46 PAY  43 

    0x3362e166,// 47 PAY  44 

    0xd8af5e09,// 48 PAY  45 

    0xd1ab980f,// 49 PAY  46 

    0x010e5498,// 50 PAY  47 

    0xbaf4dd43,// 51 PAY  48 

    0x2582de39,// 52 PAY  49 

    0x36d2a2cd,// 53 PAY  50 

    0x20389737,// 54 PAY  51 

    0xee013f44,// 55 PAY  52 

    0xe88f4c7f,// 56 PAY  53 

    0x2f9e9e76,// 57 PAY  54 

    0x6990eef5,// 58 PAY  55 

    0xd1911be1,// 59 PAY  56 

    0x71d13e73,// 60 PAY  57 

    0xd9b39961,// 61 PAY  58 

    0xb2f40257,// 62 PAY  59 

    0xf5a1fa77,// 63 PAY  60 

    0x60a27593,// 64 PAY  61 

    0xfe1392fe,// 65 PAY  62 

    0x836d70a6,// 66 PAY  63 

    0x43c1f028,// 67 PAY  64 

    0x8c2f82e4,// 68 PAY  65 

    0xce759df7,// 69 PAY  66 

    0xdba10ea9,// 70 PAY  67 

    0x40d30b68,// 71 PAY  68 

    0x2f3cd267,// 72 PAY  69 

    0x5277c0e9,// 73 PAY  70 

    0xa2e7e99c,// 74 PAY  71 

    0x88435d67,// 75 PAY  72 

    0x2252294b,// 76 PAY  73 

    0xf117eadf,// 77 PAY  74 

    0xebf5e893,// 78 PAY  75 

    0xe0d679e0,// 79 PAY  76 

    0xcbd58289,// 80 PAY  77 

    0x9f358b77,// 81 PAY  78 

    0x2879929a,// 82 PAY  79 

    0x8be0027e,// 83 PAY  80 

    0xf280e4ff,// 84 PAY  81 

    0x1926abd2,// 85 PAY  82 

    0x41b86eb2,// 86 PAY  83 

    0x77cacc23,// 87 PAY  84 

    0xb7182cab,// 88 PAY  85 

    0x05adc2f6,// 89 PAY  86 

    0xaf014ae8,// 90 PAY  87 

    0x773a15a7,// 91 PAY  88 

    0x6917f337,// 92 PAY  89 

    0x0d022522,// 93 PAY  90 

    0x4b76f1ec,// 94 PAY  91 

    0x70922d03,// 95 PAY  92 

    0xf0397369,// 96 PAY  93 

    0xe0c98ea5,// 97 PAY  94 

    0xb11733fd,// 98 PAY  95 

    0x325efbd6,// 99 PAY  96 

    0x10305972,// 100 PAY  97 

    0x65d6bff0,// 101 PAY  98 

    0x66d8ef0e,// 102 PAY  99 

    0x39ee4d81,// 103 PAY 100 

    0x90daff21,// 104 PAY 101 

    0x880f02be,// 105 PAY 102 

    0xda5f6a1e,// 106 PAY 103 

    0x4bfade5b,// 107 PAY 104 

    0x34237ff9,// 108 PAY 105 

    0x68142971,// 109 PAY 106 

    0xe8155772,// 110 PAY 107 

    0x239c3558,// 111 PAY 108 

    0x32f0e479,// 112 PAY 109 

    0xd8b24240,// 113 PAY 110 

    0x8f8288bc,// 114 PAY 111 

    0xc16df575,// 115 PAY 112 

    0xceb8210b,// 116 PAY 113 

    0x76d7c70a,// 117 PAY 114 

    0xffc07a65,// 118 PAY 115 

    0x358d8dc4,// 119 PAY 116 

    0x6b0ac714,// 120 PAY 117 

    0x462f4383,// 121 PAY 118 

    0x6bc9b794,// 122 PAY 119 

    0x067df053,// 123 PAY 120 

    0xcf69e614,// 124 PAY 121 

    0xcd8886fe,// 125 PAY 122 

    0x258da3a3,// 126 PAY 123 

    0x9387726e,// 127 PAY 124 

    0x5bfae079,// 128 PAY 125 

    0x43758a91,// 129 PAY 126 

    0x43ef991b,// 130 PAY 127 

    0xb70b2869,// 131 PAY 128 

    0xc23c7813,// 132 PAY 129 

    0xbde05501,// 133 PAY 130 

    0xb1b7d0dd,// 134 PAY 131 

    0x16643e35,// 135 PAY 132 

    0xe249939e,// 136 PAY 133 

    0x2097a82a,// 137 PAY 134 

    0xefc1bce8,// 138 PAY 135 

    0x3d410683,// 139 PAY 136 

    0xee2a62ef,// 140 PAY 137 

    0x97017888,// 141 PAY 138 

    0x38ad4a7c,// 142 PAY 139 

    0x99fd4a40,// 143 PAY 140 

    0xe62d81b7,// 144 PAY 141 

    0x4a9ad36b,// 145 PAY 142 

    0x98bebc54,// 146 PAY 143 

    0xf1c59ca4,// 147 PAY 144 

    0x754fba50,// 148 PAY 145 

    0x3c836cd4,// 149 PAY 146 

    0x677536af,// 150 PAY 147 

    0x873c0512,// 151 PAY 148 

    0x58cdf508,// 152 PAY 149 

    0x36b4951c,// 153 PAY 150 

    0x4c874b9b,// 154 PAY 151 

    0xb0169674,// 155 PAY 152 

    0xb4c2bd36,// 156 PAY 153 

    0x02ed03b7,// 157 PAY 154 

    0x64d3a736,// 158 PAY 155 

    0x2bddbf3c,// 159 PAY 156 

    0x3f196110,// 160 PAY 157 

    0xeec89dff,// 161 PAY 158 

    0x60380f76,// 162 PAY 159 

    0xa32d5870,// 163 PAY 160 

    0x717367ce,// 164 PAY 161 

    0xb2ed5431,// 165 PAY 162 

    0x8dc4c8a2,// 166 PAY 163 

    0x3aa1f64f,// 167 PAY 164 

    0xa683a350,// 168 PAY 165 

    0x4b1f0191,// 169 PAY 166 

    0xd454dd41,// 170 PAY 167 

    0x3f2df057,// 171 PAY 168 

    0xec28c22b,// 172 PAY 169 

    0x806b960a,// 173 PAY 170 

    0x6b327a26,// 174 PAY 171 

    0x4d75d3ad,// 175 PAY 172 

    0x3ab59e7f,// 176 PAY 173 

    0xdc19d922,// 177 PAY 174 

    0x8c22b8d9,// 178 PAY 175 

    0x875fd242,// 179 PAY 176 

    0xa9ada74d,// 180 PAY 177 

    0x485edf5f,// 181 PAY 178 

    0x600df86e,// 182 PAY 179 

    0xf97e4132,// 183 PAY 180 

    0x621879d7,// 184 PAY 181 

    0x18f09954,// 185 PAY 182 

    0xc4e205cc,// 186 PAY 183 

    0x37bc6751,// 187 PAY 184 

    0xeb9a2860,// 188 PAY 185 

    0x052a84f2,// 189 PAY 186 

    0x3b16c981,// 190 PAY 187 

    0x81c92c42,// 191 PAY 188 

    0x3c6460c2,// 192 PAY 189 

    0x0e0ceefa,// 193 PAY 190 

    0x96538d3b,// 194 PAY 191 

    0xc45fe56c,// 195 PAY 192 

    0x3a42abf8,// 196 PAY 193 

    0xabd54c2c,// 197 PAY 194 

    0xf6eedbd4,// 198 PAY 195 

    0xf381e49e,// 199 PAY 196 

    0x6f48ffdc,// 200 PAY 197 

    0xe6af1306,// 201 PAY 198 

    0xdf2757a9,// 202 PAY 199 

    0xe6ea34a0,// 203 PAY 200 

    0x547811bc,// 204 PAY 201 

    0x01b5b26e,// 205 PAY 202 

    0x9417a3ed,// 206 PAY 203 

    0x3f6ddca7,// 207 PAY 204 

    0xceebe4bc,// 208 PAY 205 

    0x0abd1143,// 209 PAY 206 

    0x498bdf2e,// 210 PAY 207 

    0xee88c387,// 211 PAY 208 

    0x08958878,// 212 PAY 209 

/// HASH is  4 bytes 

    0x547811bc,// 213 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 221 

/// STA pkt_idx        : 35 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd4 

    0x008cd4dd // 214 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt68_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 442 words. 

/// BDA size     is 1764 (0x6e4) 

/// BDA id       is 0x7985 

    0x06e47985,// 3 BDA   1 

/// PAY Generic Data size   : 1764 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xab9c76e4,// 4 PAY   1 

    0x6ee5c515,// 5 PAY   2 

    0x6c0f189b,// 6 PAY   3 

    0x3dd95da6,// 7 PAY   4 

    0x10efeb9e,// 8 PAY   5 

    0xe7dc8714,// 9 PAY   6 

    0x4ead266a,// 10 PAY   7 

    0xefe6a91d,// 11 PAY   8 

    0xb10e491c,// 12 PAY   9 

    0xa2e77a0a,// 13 PAY  10 

    0x39621cad,// 14 PAY  11 

    0x827a27b1,// 15 PAY  12 

    0xc1d2d890,// 16 PAY  13 

    0x569d287b,// 17 PAY  14 

    0x24946941,// 18 PAY  15 

    0x4c28dc92,// 19 PAY  16 

    0x695b2cb2,// 20 PAY  17 

    0xd1383203,// 21 PAY  18 

    0x12b0d9a5,// 22 PAY  19 

    0xfb135f1f,// 23 PAY  20 

    0x7781165a,// 24 PAY  21 

    0x4c9a01cc,// 25 PAY  22 

    0x65ef27b0,// 26 PAY  23 

    0x65fdedc2,// 27 PAY  24 

    0x9901f36c,// 28 PAY  25 

    0x867864aa,// 29 PAY  26 

    0x9eb6fc14,// 30 PAY  27 

    0xe015698a,// 31 PAY  28 

    0x93f2981c,// 32 PAY  29 

    0x12611ce8,// 33 PAY  30 

    0x7baa46f8,// 34 PAY  31 

    0x48adaa42,// 35 PAY  32 

    0x46be9197,// 36 PAY  33 

    0x0ecbbbea,// 37 PAY  34 

    0xb6d0d048,// 38 PAY  35 

    0xbcd40367,// 39 PAY  36 

    0x63ca73ea,// 40 PAY  37 

    0xd1351e1a,// 41 PAY  38 

    0xcde05aca,// 42 PAY  39 

    0x35d827f2,// 43 PAY  40 

    0x9aec488a,// 44 PAY  41 

    0x4e585cc5,// 45 PAY  42 

    0x18218217,// 46 PAY  43 

    0x67e89e39,// 47 PAY  44 

    0x53e35f74,// 48 PAY  45 

    0xda4d7626,// 49 PAY  46 

    0x3571bf46,// 50 PAY  47 

    0x2c2ee7e6,// 51 PAY  48 

    0xaeea510f,// 52 PAY  49 

    0x1d9aa72d,// 53 PAY  50 

    0x2cb8b7c1,// 54 PAY  51 

    0x0be1ed91,// 55 PAY  52 

    0xaa2b0404,// 56 PAY  53 

    0xc63907d0,// 57 PAY  54 

    0x6c731cf3,// 58 PAY  55 

    0x06aaf33b,// 59 PAY  56 

    0x3c4bce18,// 60 PAY  57 

    0x51fe9804,// 61 PAY  58 

    0x4f008f51,// 62 PAY  59 

    0xc7a5fdf4,// 63 PAY  60 

    0x61a88181,// 64 PAY  61 

    0x08f1abf7,// 65 PAY  62 

    0xdd2db66c,// 66 PAY  63 

    0xbfd2449a,// 67 PAY  64 

    0x5c9d2004,// 68 PAY  65 

    0x2617ac07,// 69 PAY  66 

    0x597553cd,// 70 PAY  67 

    0xae0ec4bb,// 71 PAY  68 

    0x1c38881e,// 72 PAY  69 

    0x13a2eb85,// 73 PAY  70 

    0x0728ce7a,// 74 PAY  71 

    0x7577ff98,// 75 PAY  72 

    0x17733dd2,// 76 PAY  73 

    0x4e8092df,// 77 PAY  74 

    0x9fe7ab22,// 78 PAY  75 

    0xd160a450,// 79 PAY  76 

    0x09979353,// 80 PAY  77 

    0x5ef431b3,// 81 PAY  78 

    0x7ca3af86,// 82 PAY  79 

    0x33d34fc3,// 83 PAY  80 

    0xbc019ee5,// 84 PAY  81 

    0x38a328c6,// 85 PAY  82 

    0x21b909a8,// 86 PAY  83 

    0x9d27de45,// 87 PAY  84 

    0x1940f3e0,// 88 PAY  85 

    0xe9dbeab3,// 89 PAY  86 

    0x08c73f45,// 90 PAY  87 

    0x4e35f1e3,// 91 PAY  88 

    0xd20d803d,// 92 PAY  89 

    0x238667d6,// 93 PAY  90 

    0x1b83384b,// 94 PAY  91 

    0xdbebf472,// 95 PAY  92 

    0xa74b325b,// 96 PAY  93 

    0x01b5f574,// 97 PAY  94 

    0xcc3a7e9a,// 98 PAY  95 

    0x3c27bece,// 99 PAY  96 

    0x69ff9502,// 100 PAY  97 

    0xf31c02c8,// 101 PAY  98 

    0x4a34503b,// 102 PAY  99 

    0x3557c753,// 103 PAY 100 

    0xa40b77bd,// 104 PAY 101 

    0xc6b6e197,// 105 PAY 102 

    0x9a55fbfe,// 106 PAY 103 

    0xdba714dc,// 107 PAY 104 

    0x3fecf4d7,// 108 PAY 105 

    0x4a73ced3,// 109 PAY 106 

    0x855b1dc6,// 110 PAY 107 

    0xc491c9aa,// 111 PAY 108 

    0x6366459f,// 112 PAY 109 

    0x3af9b61c,// 113 PAY 110 

    0x1b529fe3,// 114 PAY 111 

    0x9dee2268,// 115 PAY 112 

    0xb24e04f8,// 116 PAY 113 

    0x54234a15,// 117 PAY 114 

    0x59424398,// 118 PAY 115 

    0xf0f60eb7,// 119 PAY 116 

    0xbdc8a510,// 120 PAY 117 

    0x1cbedd36,// 121 PAY 118 

    0x6a6e0d7f,// 122 PAY 119 

    0x6761ff41,// 123 PAY 120 

    0x5f357407,// 124 PAY 121 

    0x31d98b7e,// 125 PAY 122 

    0x0607cb12,// 126 PAY 123 

    0xcdfcdc7d,// 127 PAY 124 

    0xb7ba3106,// 128 PAY 125 

    0x094cc94e,// 129 PAY 126 

    0x0810326a,// 130 PAY 127 

    0xd46955d2,// 131 PAY 128 

    0x2c98545b,// 132 PAY 129 

    0xfea1d8fb,// 133 PAY 130 

    0x8dd10c4f,// 134 PAY 131 

    0x1cd86ae7,// 135 PAY 132 

    0x42eb08e4,// 136 PAY 133 

    0x3677b8d9,// 137 PAY 134 

    0x0ad6d40d,// 138 PAY 135 

    0x8ff9f50d,// 139 PAY 136 

    0xf37661bf,// 140 PAY 137 

    0x5345f6a1,// 141 PAY 138 

    0x6c12ba9f,// 142 PAY 139 

    0x6d9b0c72,// 143 PAY 140 

    0x1e6fe4c7,// 144 PAY 141 

    0x7e4bb708,// 145 PAY 142 

    0x5da758f6,// 146 PAY 143 

    0x4a3a488e,// 147 PAY 144 

    0x3a2621c9,// 148 PAY 145 

    0xd2fed722,// 149 PAY 146 

    0xa3514fc2,// 150 PAY 147 

    0xac4b4043,// 151 PAY 148 

    0x48bd84fd,// 152 PAY 149 

    0xda4020bd,// 153 PAY 150 

    0xb30fb395,// 154 PAY 151 

    0x1a399574,// 155 PAY 152 

    0x977c1f7d,// 156 PAY 153 

    0x6fe580be,// 157 PAY 154 

    0x8f39299c,// 158 PAY 155 

    0xc98ea1eb,// 159 PAY 156 

    0x7d34a112,// 160 PAY 157 

    0x04bee801,// 161 PAY 158 

    0x70fe27f1,// 162 PAY 159 

    0x3fd01fef,// 163 PAY 160 

    0x0790e05e,// 164 PAY 161 

    0x0c3902ea,// 165 PAY 162 

    0x676a4738,// 166 PAY 163 

    0xc84002a0,// 167 PAY 164 

    0xdf770a3c,// 168 PAY 165 

    0x4a26abc6,// 169 PAY 166 

    0x13c8bf28,// 170 PAY 167 

    0x8444d2a1,// 171 PAY 168 

    0x49939dd8,// 172 PAY 169 

    0xc97e9961,// 173 PAY 170 

    0xbab85138,// 174 PAY 171 

    0x8a07b77b,// 175 PAY 172 

    0x5603aac4,// 176 PAY 173 

    0x7a4062e3,// 177 PAY 174 

    0xa73abf15,// 178 PAY 175 

    0x67ce8cb2,// 179 PAY 176 

    0x49fb5cc3,// 180 PAY 177 

    0xdf0f087b,// 181 PAY 178 

    0xc03460f3,// 182 PAY 179 

    0xfe3b6f01,// 183 PAY 180 

    0x4bbb968d,// 184 PAY 181 

    0x6818f886,// 185 PAY 182 

    0x5bb26ab2,// 186 PAY 183 

    0x6356c1a5,// 187 PAY 184 

    0x1bd9f2aa,// 188 PAY 185 

    0xbf7351c0,// 189 PAY 186 

    0xd950425b,// 190 PAY 187 

    0x3d1d7045,// 191 PAY 188 

    0x8f55fa0f,// 192 PAY 189 

    0xc01e8a2c,// 193 PAY 190 

    0x022e8f97,// 194 PAY 191 

    0xd757e94b,// 195 PAY 192 

    0x50bd56e3,// 196 PAY 193 

    0x3aa199b1,// 197 PAY 194 

    0x84d7fb12,// 198 PAY 195 

    0x24aadf41,// 199 PAY 196 

    0x48e14b14,// 200 PAY 197 

    0x150a5ce7,// 201 PAY 198 

    0x22e9570e,// 202 PAY 199 

    0xe6a31863,// 203 PAY 200 

    0x3896ac94,// 204 PAY 201 

    0x3f29c46f,// 205 PAY 202 

    0x0abd109b,// 206 PAY 203 

    0x1ff3bd70,// 207 PAY 204 

    0x19d2b13e,// 208 PAY 205 

    0xd46412e3,// 209 PAY 206 

    0x340cdb9b,// 210 PAY 207 

    0x17a93f31,// 211 PAY 208 

    0x927d24a5,// 212 PAY 209 

    0x6c901de5,// 213 PAY 210 

    0xf8d90108,// 214 PAY 211 

    0x920bfbfb,// 215 PAY 212 

    0xcc6bb847,// 216 PAY 213 

    0xda45c261,// 217 PAY 214 

    0x2b49dd66,// 218 PAY 215 

    0x17a8c2c5,// 219 PAY 216 

    0x995c6375,// 220 PAY 217 

    0x4d970acd,// 221 PAY 218 

    0xfbe410c4,// 222 PAY 219 

    0x4c650481,// 223 PAY 220 

    0xbf85fdd3,// 224 PAY 221 

    0xaaf02d3a,// 225 PAY 222 

    0x31f7d3cf,// 226 PAY 223 

    0xaf91c7e0,// 227 PAY 224 

    0x466ceee3,// 228 PAY 225 

    0xd9bd2ac0,// 229 PAY 226 

    0xf10a3cf2,// 230 PAY 227 

    0xefe7f4bb,// 231 PAY 228 

    0x0e7edd55,// 232 PAY 229 

    0x8009d8ed,// 233 PAY 230 

    0x9b088c61,// 234 PAY 231 

    0xe2159b98,// 235 PAY 232 

    0xeed88430,// 236 PAY 233 

    0x8b339fe1,// 237 PAY 234 

    0x639b32ea,// 238 PAY 235 

    0x71fee5e6,// 239 PAY 236 

    0x6d35f90d,// 240 PAY 237 

    0x342af5a5,// 241 PAY 238 

    0x1c8c6d52,// 242 PAY 239 

    0xc87dd239,// 243 PAY 240 

    0x8e8d4bb1,// 244 PAY 241 

    0x208836b0,// 245 PAY 242 

    0xb54d7afe,// 246 PAY 243 

    0x18d65053,// 247 PAY 244 

    0x03e9aee6,// 248 PAY 245 

    0x97598eda,// 249 PAY 246 

    0x9d702d6b,// 250 PAY 247 

    0x75371f90,// 251 PAY 248 

    0x43f7a8fd,// 252 PAY 249 

    0x84d94c40,// 253 PAY 250 

    0x9742fbea,// 254 PAY 251 

    0xbabb5725,// 255 PAY 252 

    0xe4e80abe,// 256 PAY 253 

    0xf7f2e871,// 257 PAY 254 

    0x6edbac50,// 258 PAY 255 

    0xbba7f86c,// 259 PAY 256 

    0x846c815a,// 260 PAY 257 

    0x61d79912,// 261 PAY 258 

    0xb7f98925,// 262 PAY 259 

    0xd10a8d64,// 263 PAY 260 

    0x502e7bf2,// 264 PAY 261 

    0xbdb5967f,// 265 PAY 262 

    0xbaadb147,// 266 PAY 263 

    0xcaa81349,// 267 PAY 264 

    0x3382eb84,// 268 PAY 265 

    0x10d14573,// 269 PAY 266 

    0xa1558c42,// 270 PAY 267 

    0x90f0e0a6,// 271 PAY 268 

    0xb1b0513f,// 272 PAY 269 

    0xbd4ef02b,// 273 PAY 270 

    0xeecb15d6,// 274 PAY 271 

    0x53cccbb2,// 275 PAY 272 

    0x1213de38,// 276 PAY 273 

    0xb5d3f8f4,// 277 PAY 274 

    0x9530499a,// 278 PAY 275 

    0xb83f7e27,// 279 PAY 276 

    0xbb52d86e,// 280 PAY 277 

    0x084b0295,// 281 PAY 278 

    0x0e22523e,// 282 PAY 279 

    0x3fc7f8dc,// 283 PAY 280 

    0x55d3a3de,// 284 PAY 281 

    0x63f29116,// 285 PAY 282 

    0x3767f3ac,// 286 PAY 283 

    0x1fe2d3d0,// 287 PAY 284 

    0x14f7a05a,// 288 PAY 285 

    0xb35d1c4e,// 289 PAY 286 

    0x90c04c7b,// 290 PAY 287 

    0x550fbac0,// 291 PAY 288 

    0x9ec983a6,// 292 PAY 289 

    0xe9341f9a,// 293 PAY 290 

    0x48e5fcbf,// 294 PAY 291 

    0xc784808d,// 295 PAY 292 

    0x58ad39bd,// 296 PAY 293 

    0x5c37a590,// 297 PAY 294 

    0xc28b6b3e,// 298 PAY 295 

    0x8175e897,// 299 PAY 296 

    0xc72330d2,// 300 PAY 297 

    0x480c3e2c,// 301 PAY 298 

    0x7b56e608,// 302 PAY 299 

    0x5f0f760a,// 303 PAY 300 

    0x3e29a181,// 304 PAY 301 

    0x0b6ea9c3,// 305 PAY 302 

    0x30d2fc2c,// 306 PAY 303 

    0x63392f72,// 307 PAY 304 

    0x4d868d74,// 308 PAY 305 

    0x7bfbb405,// 309 PAY 306 

    0x1c85f184,// 310 PAY 307 

    0xadf1180d,// 311 PAY 308 

    0x4dc09921,// 312 PAY 309 

    0x8fe3515e,// 313 PAY 310 

    0x3c97b5cd,// 314 PAY 311 

    0xfe7de79a,// 315 PAY 312 

    0x4d2dd0d4,// 316 PAY 313 

    0xa626bfa2,// 317 PAY 314 

    0x18a9442b,// 318 PAY 315 

    0xdb7809d1,// 319 PAY 316 

    0xdb9fb619,// 320 PAY 317 

    0xfc79c2f1,// 321 PAY 318 

    0x0d96622f,// 322 PAY 319 

    0x6926cdf8,// 323 PAY 320 

    0x1522f7fb,// 324 PAY 321 

    0x869966ef,// 325 PAY 322 

    0xadb3d940,// 326 PAY 323 

    0x80da5716,// 327 PAY 324 

    0x1937167a,// 328 PAY 325 

    0x62d6e10a,// 329 PAY 326 

    0xb55e574a,// 330 PAY 327 

    0xbc09c30c,// 331 PAY 328 

    0x2a1d079a,// 332 PAY 329 

    0x3e06e2aa,// 333 PAY 330 

    0xa3e92ebf,// 334 PAY 331 

    0x09f9da06,// 335 PAY 332 

    0x62c8ac4f,// 336 PAY 333 

    0x5540cd62,// 337 PAY 334 

    0x8647dca5,// 338 PAY 335 

    0xd8e1e8e2,// 339 PAY 336 

    0x186d0ca7,// 340 PAY 337 

    0xe67662aa,// 341 PAY 338 

    0xccab7d6f,// 342 PAY 339 

    0xf48bf12b,// 343 PAY 340 

    0xfbdca24d,// 344 PAY 341 

    0x12b7cdec,// 345 PAY 342 

    0x1bafddf2,// 346 PAY 343 

    0x3f46de0b,// 347 PAY 344 

    0xe85a3cb3,// 348 PAY 345 

    0x4812e1c3,// 349 PAY 346 

    0x0fdb476a,// 350 PAY 347 

    0x27bc447b,// 351 PAY 348 

    0xa0e446dd,// 352 PAY 349 

    0x3c7e8259,// 353 PAY 350 

    0x16cbed18,// 354 PAY 351 

    0xdb2b171e,// 355 PAY 352 

    0x175b765d,// 356 PAY 353 

    0xd56b8f39,// 357 PAY 354 

    0x822ab482,// 358 PAY 355 

    0xc2f58efb,// 359 PAY 356 

    0xc7d11716,// 360 PAY 357 

    0x626d9739,// 361 PAY 358 

    0x6074098b,// 362 PAY 359 

    0xa85c1758,// 363 PAY 360 

    0xe231a294,// 364 PAY 361 

    0x2edc088f,// 365 PAY 362 

    0xfc0068a6,// 366 PAY 363 

    0x40939411,// 367 PAY 364 

    0xf84c3f99,// 368 PAY 365 

    0xa3af96c4,// 369 PAY 366 

    0x6df24224,// 370 PAY 367 

    0x6c37fed2,// 371 PAY 368 

    0x3de39fa1,// 372 PAY 369 

    0xa7cba219,// 373 PAY 370 

    0x9d34bf76,// 374 PAY 371 

    0xe2e8428f,// 375 PAY 372 

    0x1e6991b2,// 376 PAY 373 

    0xa8092cf9,// 377 PAY 374 

    0x2a15cbaf,// 378 PAY 375 

    0xe4aec899,// 379 PAY 376 

    0x667434af,// 380 PAY 377 

    0xc052efef,// 381 PAY 378 

    0x8a10ed3c,// 382 PAY 379 

    0x405d23ea,// 383 PAY 380 

    0xca19a5ef,// 384 PAY 381 

    0x9c7e9fe1,// 385 PAY 382 

    0x0d092e4c,// 386 PAY 383 

    0x8f44845e,// 387 PAY 384 

    0x6a3223b3,// 388 PAY 385 

    0x6b2b3497,// 389 PAY 386 

    0x8cb947ed,// 390 PAY 387 

    0x8be40ca5,// 391 PAY 388 

    0x5f531346,// 392 PAY 389 

    0x0391e257,// 393 PAY 390 

    0x145d3034,// 394 PAY 391 

    0xa7e3b420,// 395 PAY 392 

    0x26b0514e,// 396 PAY 393 

    0xf3a63d6c,// 397 PAY 394 

    0x22f18208,// 398 PAY 395 

    0xe2ce0835,// 399 PAY 396 

    0x95eccb2f,// 400 PAY 397 

    0x7c82364f,// 401 PAY 398 

    0xf57fe107,// 402 PAY 399 

    0x8ddd1f35,// 403 PAY 400 

    0x9af74a17,// 404 PAY 401 

    0x40d98854,// 405 PAY 402 

    0x93baa022,// 406 PAY 403 

    0xd889c15a,// 407 PAY 404 

    0x381898fc,// 408 PAY 405 

    0xab8c72c7,// 409 PAY 406 

    0x22e0f06c,// 410 PAY 407 

    0xefa68f50,// 411 PAY 408 

    0x159c4632,// 412 PAY 409 

    0xf1149571,// 413 PAY 410 

    0x2a17d39f,// 414 PAY 411 

    0xa6b19785,// 415 PAY 412 

    0x537bb3d4,// 416 PAY 413 

    0x5582d983,// 417 PAY 414 

    0x52921999,// 418 PAY 415 

    0x705ca01f,// 419 PAY 416 

    0xc4df5b02,// 420 PAY 417 

    0x7eb02e24,// 421 PAY 418 

    0xcd76d848,// 422 PAY 419 

    0x63801ee4,// 423 PAY 420 

    0x35f2ed30,// 424 PAY 421 

    0x6bc4804e,// 425 PAY 422 

    0xd16a3c7c,// 426 PAY 423 

    0x3f01b1c6,// 427 PAY 424 

    0xd7c41b18,// 428 PAY 425 

    0xa79a2569,// 429 PAY 426 

    0x869eedba,// 430 PAY 427 

    0x49a2581b,// 431 PAY 428 

    0xfcf7cbec,// 432 PAY 429 

    0x60f11111,// 433 PAY 430 

    0x94279226,// 434 PAY 431 

    0x7c24dc2b,// 435 PAY 432 

    0x59066568,// 436 PAY 433 

    0x7bfe764e,// 437 PAY 434 

    0xdbdc9669,// 438 PAY 435 

    0xdc9bec84,// 439 PAY 436 

    0xaebde1f9,// 440 PAY 437 

    0x182a2923,// 441 PAY 438 

    0xc4aa871e,// 442 PAY 439 

    0xb7c19f41,// 443 PAY 440 

    0xf983cd05,// 444 PAY 441 

/// HASH is  16 bytes 

    0xfcf7cbec,// 445 HSH   1 

    0x60f11111,// 446 HSH   2 

    0x94279226,// 447 HSH   3 

    0x7c24dc2b,// 448 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 198 

/// STA pkt_idx        : 217 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9d 

    0x03659dc6 // 449 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt69_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 356 words. 

/// BDA size     is 1418 (0x58a) 

/// BDA id       is 0xe834 

    0x058ae834,// 3 BDA   1 

/// PAY Generic Data size   : 1418 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xad39d576,// 4 PAY   1 

    0x2ba32eb4,// 5 PAY   2 

    0xcc6c2e47,// 6 PAY   3 

    0x14bd3cba,// 7 PAY   4 

    0x54b96361,// 8 PAY   5 

    0x3a752034,// 9 PAY   6 

    0x33113da8,// 10 PAY   7 

    0xa762a940,// 11 PAY   8 

    0x749acbfd,// 12 PAY   9 

    0x530b692a,// 13 PAY  10 

    0x4ee7f3ac,// 14 PAY  11 

    0x1874be4b,// 15 PAY  12 

    0xeef707c4,// 16 PAY  13 

    0xd18805c4,// 17 PAY  14 

    0x704f37c9,// 18 PAY  15 

    0xbe9f8239,// 19 PAY  16 

    0xa274de69,// 20 PAY  17 

    0xde2cf815,// 21 PAY  18 

    0xc820dbb7,// 22 PAY  19 

    0xccd9218a,// 23 PAY  20 

    0x3479fee1,// 24 PAY  21 

    0x65c96c53,// 25 PAY  22 

    0x7ee4f61c,// 26 PAY  23 

    0x1aee5a43,// 27 PAY  24 

    0x40d2339b,// 28 PAY  25 

    0xec2138ab,// 29 PAY  26 

    0x1e15d88a,// 30 PAY  27 

    0xbd0451ea,// 31 PAY  28 

    0xf3591772,// 32 PAY  29 

    0xb0af5e50,// 33 PAY  30 

    0xc444b9b0,// 34 PAY  31 

    0xe82c9792,// 35 PAY  32 

    0x2640cb83,// 36 PAY  33 

    0x5962fdae,// 37 PAY  34 

    0x522b808a,// 38 PAY  35 

    0xa644817e,// 39 PAY  36 

    0xa18dcc98,// 40 PAY  37 

    0xa937bdec,// 41 PAY  38 

    0xc8286f31,// 42 PAY  39 

    0xa4bad90e,// 43 PAY  40 

    0xdd2e6dfd,// 44 PAY  41 

    0x7fc34559,// 45 PAY  42 

    0x2b25b987,// 46 PAY  43 

    0xf23a293d,// 47 PAY  44 

    0x6883e9f6,// 48 PAY  45 

    0x55347cec,// 49 PAY  46 

    0xb9f0aa40,// 50 PAY  47 

    0x76a3fd25,// 51 PAY  48 

    0x9078f8bb,// 52 PAY  49 

    0x309c1acb,// 53 PAY  50 

    0xa123c846,// 54 PAY  51 

    0xe7684e70,// 55 PAY  52 

    0x4b816463,// 56 PAY  53 

    0x9e6fb298,// 57 PAY  54 

    0x9a5ae00f,// 58 PAY  55 

    0xa0d68f8d,// 59 PAY  56 

    0x8e4fb3f9,// 60 PAY  57 

    0x2e6e2323,// 61 PAY  58 

    0x55b33936,// 62 PAY  59 

    0xda1cadf4,// 63 PAY  60 

    0x6db92f67,// 64 PAY  61 

    0xd90d58f2,// 65 PAY  62 

    0xc75b481b,// 66 PAY  63 

    0x01989762,// 67 PAY  64 

    0xff8ee550,// 68 PAY  65 

    0xf2b0e915,// 69 PAY  66 

    0x81a56f2c,// 70 PAY  67 

    0x563c957b,// 71 PAY  68 

    0x1b7473d6,// 72 PAY  69 

    0xae261eaf,// 73 PAY  70 

    0x06d34850,// 74 PAY  71 

    0xe631444d,// 75 PAY  72 

    0x71626495,// 76 PAY  73 

    0x443caf2b,// 77 PAY  74 

    0xbd243e5c,// 78 PAY  75 

    0xeb5cd118,// 79 PAY  76 

    0xcb16489f,// 80 PAY  77 

    0xb90fb308,// 81 PAY  78 

    0x0a9a3de6,// 82 PAY  79 

    0x89802e61,// 83 PAY  80 

    0x00ce4393,// 84 PAY  81 

    0xa47abe2e,// 85 PAY  82 

    0x7fe68ac8,// 86 PAY  83 

    0xa4bf4ca5,// 87 PAY  84 

    0x0297b083,// 88 PAY  85 

    0xbe43aaa9,// 89 PAY  86 

    0x14beab49,// 90 PAY  87 

    0xbfdeab60,// 91 PAY  88 

    0xcd44bf54,// 92 PAY  89 

    0x940b9c32,// 93 PAY  90 

    0xbe4ae85f,// 94 PAY  91 

    0x49042303,// 95 PAY  92 

    0x4fbcc4fc,// 96 PAY  93 

    0x99ccdcc1,// 97 PAY  94 

    0x24e4f08b,// 98 PAY  95 

    0xb7f0dacc,// 99 PAY  96 

    0xf4fd431b,// 100 PAY  97 

    0x65f7a9a1,// 101 PAY  98 

    0x1abecbbf,// 102 PAY  99 

    0x2658fd3a,// 103 PAY 100 

    0x588b2086,// 104 PAY 101 

    0xda69a05a,// 105 PAY 102 

    0x2193a141,// 106 PAY 103 

    0x404cd700,// 107 PAY 104 

    0xae2751f0,// 108 PAY 105 

    0xc47be8b9,// 109 PAY 106 

    0x10f75ae2,// 110 PAY 107 

    0xf2472ff7,// 111 PAY 108 

    0x8ccf6716,// 112 PAY 109 

    0xb7ca33a4,// 113 PAY 110 

    0xa6c25af1,// 114 PAY 111 

    0x51f16190,// 115 PAY 112 

    0x953ea3c7,// 116 PAY 113 

    0x26edaaeb,// 117 PAY 114 

    0x55fa6036,// 118 PAY 115 

    0x391185d6,// 119 PAY 116 

    0xa89a3eea,// 120 PAY 117 

    0x05a23b9d,// 121 PAY 118 

    0x11bc77c3,// 122 PAY 119 

    0x51bc603d,// 123 PAY 120 

    0x67b97181,// 124 PAY 121 

    0x4272dda0,// 125 PAY 122 

    0xe1e0ce52,// 126 PAY 123 

    0x444a7607,// 127 PAY 124 

    0xa0c2bfa8,// 128 PAY 125 

    0x0626e34f,// 129 PAY 126 

    0x558f7487,// 130 PAY 127 

    0xcc03ebc3,// 131 PAY 128 

    0x7daf7048,// 132 PAY 129 

    0xfb390981,// 133 PAY 130 

    0xbb2025f2,// 134 PAY 131 

    0x3f1d96a8,// 135 PAY 132 

    0x6af1d75e,// 136 PAY 133 

    0x9370d693,// 137 PAY 134 

    0xeaa366e9,// 138 PAY 135 

    0xfd64a080,// 139 PAY 136 

    0x16063deb,// 140 PAY 137 

    0xa1370fa7,// 141 PAY 138 

    0xef736b7d,// 142 PAY 139 

    0x55f99b23,// 143 PAY 140 

    0xa2519151,// 144 PAY 141 

    0x1042e329,// 145 PAY 142 

    0x106f0c1f,// 146 PAY 143 

    0xb99d2997,// 147 PAY 144 

    0xcb4e0ff9,// 148 PAY 145 

    0xe0233b0b,// 149 PAY 146 

    0xdad38f23,// 150 PAY 147 

    0x60c613f0,// 151 PAY 148 

    0x25436c56,// 152 PAY 149 

    0xd1400cb8,// 153 PAY 150 

    0xe5690b9f,// 154 PAY 151 

    0x6796f6c2,// 155 PAY 152 

    0xb3dc7cc3,// 156 PAY 153 

    0xf8de71f7,// 157 PAY 154 

    0x85676401,// 158 PAY 155 

    0x47a78281,// 159 PAY 156 

    0x7f8c3b1e,// 160 PAY 157 

    0xa1153c4d,// 161 PAY 158 

    0xd398799a,// 162 PAY 159 

    0xfaced992,// 163 PAY 160 

    0x723fef0f,// 164 PAY 161 

    0xd3060675,// 165 PAY 162 

    0x131e45d3,// 166 PAY 163 

    0x2cd00bc1,// 167 PAY 164 

    0xd077a37f,// 168 PAY 165 

    0x8541d138,// 169 PAY 166 

    0xfacef437,// 170 PAY 167 

    0xa619dbbc,// 171 PAY 168 

    0x2c98e415,// 172 PAY 169 

    0x6a2e4bb7,// 173 PAY 170 

    0x2bcf4d4a,// 174 PAY 171 

    0x70aedbe7,// 175 PAY 172 

    0x41544c62,// 176 PAY 173 

    0x549c67e2,// 177 PAY 174 

    0xf4553564,// 178 PAY 175 

    0x1d2594b8,// 179 PAY 176 

    0x7d5ce8cb,// 180 PAY 177 

    0x2e09eb10,// 181 PAY 178 

    0x73841797,// 182 PAY 179 

    0x27a8a054,// 183 PAY 180 

    0x912e26c2,// 184 PAY 181 

    0x5d30eb60,// 185 PAY 182 

    0xb7e1fe83,// 186 PAY 183 

    0xe8eea12a,// 187 PAY 184 

    0xe54ba2f2,// 188 PAY 185 

    0x04cf46af,// 189 PAY 186 

    0x547b673d,// 190 PAY 187 

    0x7f34e5ca,// 191 PAY 188 

    0x05b218a4,// 192 PAY 189 

    0xe796f2d1,// 193 PAY 190 

    0xa6c84878,// 194 PAY 191 

    0x5f557749,// 195 PAY 192 

    0xaca4f42d,// 196 PAY 193 

    0x5cff4645,// 197 PAY 194 

    0x7534cead,// 198 PAY 195 

    0xcb3f791e,// 199 PAY 196 

    0x63b7deac,// 200 PAY 197 

    0x314dda39,// 201 PAY 198 

    0xd968411e,// 202 PAY 199 

    0x3cf271b7,// 203 PAY 200 

    0x5681eb8c,// 204 PAY 201 

    0xa0bfdd9c,// 205 PAY 202 

    0x48921e1c,// 206 PAY 203 

    0x2fa72ab4,// 207 PAY 204 

    0xdd5f0f0d,// 208 PAY 205 

    0x35f45d66,// 209 PAY 206 

    0x7838d254,// 210 PAY 207 

    0x619c80b5,// 211 PAY 208 

    0x4740652c,// 212 PAY 209 

    0x6b41975b,// 213 PAY 210 

    0x313f86ba,// 214 PAY 211 

    0xfb61785b,// 215 PAY 212 

    0x6290c5bc,// 216 PAY 213 

    0x5163cf25,// 217 PAY 214 

    0xa3b7c6a0,// 218 PAY 215 

    0x574c7598,// 219 PAY 216 

    0xfc257fdd,// 220 PAY 217 

    0x9e030f4e,// 221 PAY 218 

    0x13003581,// 222 PAY 219 

    0xfb49ac83,// 223 PAY 220 

    0x8955fa42,// 224 PAY 221 

    0x7db4ec7b,// 225 PAY 222 

    0xa8b7f6f4,// 226 PAY 223 

    0xf54f21a5,// 227 PAY 224 

    0xb0e53b4a,// 228 PAY 225 

    0x9487ac84,// 229 PAY 226 

    0x44efce69,// 230 PAY 227 

    0xf7cb3edf,// 231 PAY 228 

    0xedab4894,// 232 PAY 229 

    0x94d575dc,// 233 PAY 230 

    0x07603608,// 234 PAY 231 

    0x98197eea,// 235 PAY 232 

    0x18e49ce2,// 236 PAY 233 

    0x16a3eec5,// 237 PAY 234 

    0x4f3fe707,// 238 PAY 235 

    0x858af5fd,// 239 PAY 236 

    0x682dfff0,// 240 PAY 237 

    0x931886bb,// 241 PAY 238 

    0x99c5cbbb,// 242 PAY 239 

    0xa2a92c68,// 243 PAY 240 

    0xc5c44360,// 244 PAY 241 

    0x48a4637d,// 245 PAY 242 

    0xa69e316e,// 246 PAY 243 

    0x59059976,// 247 PAY 244 

    0x328fb7c7,// 248 PAY 245 

    0xdf8299ef,// 249 PAY 246 

    0x875760ea,// 250 PAY 247 

    0x36655fa7,// 251 PAY 248 

    0x0b8ba831,// 252 PAY 249 

    0x054f3a60,// 253 PAY 250 

    0xa1d4aab2,// 254 PAY 251 

    0xeb1bf553,// 255 PAY 252 

    0xfc757fc4,// 256 PAY 253 

    0x142f3cec,// 257 PAY 254 

    0x510fecae,// 258 PAY 255 

    0x9a60b669,// 259 PAY 256 

    0x47226795,// 260 PAY 257 

    0x9a3eecf9,// 261 PAY 258 

    0x56bc926f,// 262 PAY 259 

    0xa1e3d37d,// 263 PAY 260 

    0x67312fe5,// 264 PAY 261 

    0x81a7f7b6,// 265 PAY 262 

    0x822b4232,// 266 PAY 263 

    0xf5f54bd6,// 267 PAY 264 

    0xf8dbfc76,// 268 PAY 265 

    0x53849eb8,// 269 PAY 266 

    0xc29261c6,// 270 PAY 267 

    0xc601c379,// 271 PAY 268 

    0xdcad3048,// 272 PAY 269 

    0xb10d6a0f,// 273 PAY 270 

    0xa4eba60c,// 274 PAY 271 

    0x10764ebd,// 275 PAY 272 

    0xbaec3cb7,// 276 PAY 273 

    0xcc5c8964,// 277 PAY 274 

    0x681ccd07,// 278 PAY 275 

    0xb6fa588f,// 279 PAY 276 

    0xa8b261a5,// 280 PAY 277 

    0x81f4d1a9,// 281 PAY 278 

    0x0aeece99,// 282 PAY 279 

    0x1ff9c546,// 283 PAY 280 

    0x7887d239,// 284 PAY 281 

    0xeea2430a,// 285 PAY 282 

    0xbe22a195,// 286 PAY 283 

    0xf15c2ddc,// 287 PAY 284 

    0x53597e09,// 288 PAY 285 

    0xe5a5bc03,// 289 PAY 286 

    0x19194cf7,// 290 PAY 287 

    0x8309b1d4,// 291 PAY 288 

    0x63ca9569,// 292 PAY 289 

    0x50fcc7bf,// 293 PAY 290 

    0xc34eac2c,// 294 PAY 291 

    0x02549b2c,// 295 PAY 292 

    0x1b07c116,// 296 PAY 293 

    0x59acf809,// 297 PAY 294 

    0xce6be8b8,// 298 PAY 295 

    0x9a8b1e79,// 299 PAY 296 

    0x9e3ae1f2,// 300 PAY 297 

    0xf67e8f7e,// 301 PAY 298 

    0x1206d9ba,// 302 PAY 299 

    0xa286c914,// 303 PAY 300 

    0xa9b33678,// 304 PAY 301 

    0xb159298f,// 305 PAY 302 

    0x87a1c565,// 306 PAY 303 

    0x931bec12,// 307 PAY 304 

    0x7025c440,// 308 PAY 305 

    0x016f8e81,// 309 PAY 306 

    0xd2ce9e2a,// 310 PAY 307 

    0x3235fc45,// 311 PAY 308 

    0xce43c89c,// 312 PAY 309 

    0x5a3a7c85,// 313 PAY 310 

    0xced3db63,// 314 PAY 311 

    0x284e87e8,// 315 PAY 312 

    0x030ebdda,// 316 PAY 313 

    0x9203a33c,// 317 PAY 314 

    0xfcb832e0,// 318 PAY 315 

    0x110d4c1d,// 319 PAY 316 

    0x7e1e2024,// 320 PAY 317 

    0x16c5a495,// 321 PAY 318 

    0x1a584b01,// 322 PAY 319 

    0xa49c5d70,// 323 PAY 320 

    0xfc486750,// 324 PAY 321 

    0xe5ed4ddd,// 325 PAY 322 

    0x5701ab69,// 326 PAY 323 

    0x49123b73,// 327 PAY 324 

    0x4bd37a76,// 328 PAY 325 

    0xf5769988,// 329 PAY 326 

    0x1f9df0fd,// 330 PAY 327 

    0xe9693a2e,// 331 PAY 328 

    0xcf6c36e4,// 332 PAY 329 

    0x7766ff24,// 333 PAY 330 

    0x16207535,// 334 PAY 331 

    0x4b1f5156,// 335 PAY 332 

    0x9e088ac9,// 336 PAY 333 

    0x78fd5086,// 337 PAY 334 

    0x907b0980,// 338 PAY 335 

    0x6853085c,// 339 PAY 336 

    0x4ec09a21,// 340 PAY 337 

    0x0a3fd5cf,// 341 PAY 338 

    0x21944149,// 342 PAY 339 

    0x40084dfa,// 343 PAY 340 

    0xed7ade8c,// 344 PAY 341 

    0x5f47ae00,// 345 PAY 342 

    0x5f8b08cd,// 346 PAY 343 

    0x18e7747e,// 347 PAY 344 

    0x41c434a1,// 348 PAY 345 

    0xa74be3b5,// 349 PAY 346 

    0x73178a4e,// 350 PAY 347 

    0x8bf04a71,// 351 PAY 348 

    0x7d41e2c8,// 352 PAY 349 

    0xa818782c,// 353 PAY 350 

    0xabc4d1fa,// 354 PAY 351 

    0x5b3c9ccf,// 355 PAY 352 

    0xe6f96be3,// 356 PAY 353 

    0x2c94cb32,// 357 PAY 354 

    0xc5910000,// 358 PAY 355 

/// STA is 1 words. 

/// STA num_pkts       : 24 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xed 

    0x03dded18 // 359 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt70_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 162 words. 

/// BDA size     is 643 (0x283) 

/// BDA id       is 0x8e57 

    0x02838e57,// 3 BDA   1 

/// PAY Generic Data size   : 643 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x0a125e74,// 4 PAY   1 

    0x6b05a188,// 5 PAY   2 

    0xf8ebed97,// 6 PAY   3 

    0xb84c7e19,// 7 PAY   4 

    0x1d23e01e,// 8 PAY   5 

    0x2df2a1a7,// 9 PAY   6 

    0x7f680852,// 10 PAY   7 

    0xfd48f352,// 11 PAY   8 

    0x36175108,// 12 PAY   9 

    0xf46f7c5e,// 13 PAY  10 

    0x737e5998,// 14 PAY  11 

    0x72337774,// 15 PAY  12 

    0x78a41034,// 16 PAY  13 

    0x605ab796,// 17 PAY  14 

    0xe67e11ab,// 18 PAY  15 

    0x7e7ab23b,// 19 PAY  16 

    0xda640d81,// 20 PAY  17 

    0x845d0e54,// 21 PAY  18 

    0xbf65180e,// 22 PAY  19 

    0xddc04357,// 23 PAY  20 

    0x768e1376,// 24 PAY  21 

    0x584926fd,// 25 PAY  22 

    0x3949d5d3,// 26 PAY  23 

    0x0d73ac3b,// 27 PAY  24 

    0x2c22253e,// 28 PAY  25 

    0xe495bb76,// 29 PAY  26 

    0x89e4fcfd,// 30 PAY  27 

    0x595dcd10,// 31 PAY  28 

    0x55bced9b,// 32 PAY  29 

    0xc390f5ed,// 33 PAY  30 

    0x67395cb9,// 34 PAY  31 

    0x2cf23352,// 35 PAY  32 

    0x04839dbb,// 36 PAY  33 

    0xb7fb985a,// 37 PAY  34 

    0x13b95064,// 38 PAY  35 

    0x5dc6e575,// 39 PAY  36 

    0x28645da4,// 40 PAY  37 

    0x7299113a,// 41 PAY  38 

    0x2391c3a5,// 42 PAY  39 

    0x65ff046b,// 43 PAY  40 

    0x34b32dc8,// 44 PAY  41 

    0x9e23b5fb,// 45 PAY  42 

    0x9ddbc1ac,// 46 PAY  43 

    0x1bc446a3,// 47 PAY  44 

    0x45ea6f42,// 48 PAY  45 

    0xc6ce54ee,// 49 PAY  46 

    0x03049c45,// 50 PAY  47 

    0x443ee24c,// 51 PAY  48 

    0x4d249b58,// 52 PAY  49 

    0x80710b5a,// 53 PAY  50 

    0xeaf56c05,// 54 PAY  51 

    0x646222ab,// 55 PAY  52 

    0xaf9f38e6,// 56 PAY  53 

    0x6dbb3167,// 57 PAY  54 

    0xcd722109,// 58 PAY  55 

    0x4064f3a4,// 59 PAY  56 

    0x6c21968c,// 60 PAY  57 

    0xbe51a323,// 61 PAY  58 

    0xea3bacc7,// 62 PAY  59 

    0x72f363bc,// 63 PAY  60 

    0x447f2bdc,// 64 PAY  61 

    0xc64ddc20,// 65 PAY  62 

    0xf80f611e,// 66 PAY  63 

    0x01781f3a,// 67 PAY  64 

    0xa3372629,// 68 PAY  65 

    0xbcc162d2,// 69 PAY  66 

    0xfb143844,// 70 PAY  67 

    0x17c22f5b,// 71 PAY  68 

    0x797c614d,// 72 PAY  69 

    0xb4d1b5b0,// 73 PAY  70 

    0x286e57a1,// 74 PAY  71 

    0x8560f2d2,// 75 PAY  72 

    0xd24346d9,// 76 PAY  73 

    0x27e1946f,// 77 PAY  74 

    0xd56ce862,// 78 PAY  75 

    0x6c787c47,// 79 PAY  76 

    0x244f6fd2,// 80 PAY  77 

    0x90e0d40d,// 81 PAY  78 

    0xf44f41ca,// 82 PAY  79 

    0x1b3be03d,// 83 PAY  80 

    0x8b152097,// 84 PAY  81 

    0xf288fb49,// 85 PAY  82 

    0xf7532aa4,// 86 PAY  83 

    0x86d3af55,// 87 PAY  84 

    0xb21f34a3,// 88 PAY  85 

    0x9c1bc9d7,// 89 PAY  86 

    0xbedfe443,// 90 PAY  87 

    0x048fa948,// 91 PAY  88 

    0x95409219,// 92 PAY  89 

    0x20633a5b,// 93 PAY  90 

    0x8a62e60e,// 94 PAY  91 

    0x380b8525,// 95 PAY  92 

    0x0d9d1459,// 96 PAY  93 

    0x27b4c967,// 97 PAY  94 

    0xe9390b0f,// 98 PAY  95 

    0xcbaa0100,// 99 PAY  96 

    0xdacad868,// 100 PAY  97 

    0x86d41bdb,// 101 PAY  98 

    0x86505a7e,// 102 PAY  99 

    0x3775b2bd,// 103 PAY 100 

    0xa177276d,// 104 PAY 101 

    0xb8c416c6,// 105 PAY 102 

    0x29cd38be,// 106 PAY 103 

    0xe7132556,// 107 PAY 104 

    0xea743b48,// 108 PAY 105 

    0xe94c10c8,// 109 PAY 106 

    0x316d3fc9,// 110 PAY 107 

    0x2b4d0c88,// 111 PAY 108 

    0xb02ee113,// 112 PAY 109 

    0xbbbf7b0c,// 113 PAY 110 

    0x63b955f5,// 114 PAY 111 

    0x438e1219,// 115 PAY 112 

    0x1ac5a369,// 116 PAY 113 

    0x37d3aa6c,// 117 PAY 114 

    0x6a38a241,// 118 PAY 115 

    0xce179e75,// 119 PAY 116 

    0x45c63d9c,// 120 PAY 117 

    0xed0fab78,// 121 PAY 118 

    0xf534f62d,// 122 PAY 119 

    0x7e8e6457,// 123 PAY 120 

    0xd20b7e2d,// 124 PAY 121 

    0x23225d3a,// 125 PAY 122 

    0x965e3fd9,// 126 PAY 123 

    0xc0b112ba,// 127 PAY 124 

    0xea229008,// 128 PAY 125 

    0xb121c111,// 129 PAY 126 

    0x53676940,// 130 PAY 127 

    0x7d77ee55,// 131 PAY 128 

    0x9f81d32b,// 132 PAY 129 

    0x7c567374,// 133 PAY 130 

    0x60909cb9,// 134 PAY 131 

    0x5a44dd67,// 135 PAY 132 

    0xb5dea3d1,// 136 PAY 133 

    0x4182b629,// 137 PAY 134 

    0x45e4b2c5,// 138 PAY 135 

    0x3f794277,// 139 PAY 136 

    0xb1228b04,// 140 PAY 137 

    0x3f8890c4,// 141 PAY 138 

    0x3bac7b2e,// 142 PAY 139 

    0xa867f270,// 143 PAY 140 

    0x64b65a1e,// 144 PAY 141 

    0x1a9200dc,// 145 PAY 142 

    0x00b18f21,// 146 PAY 143 

    0x6257a0e3,// 147 PAY 144 

    0xd7515be6,// 148 PAY 145 

    0x6629a81d,// 149 PAY 146 

    0xd09a7c53,// 150 PAY 147 

    0x52f4fe11,// 151 PAY 148 

    0xc2911ab0,// 152 PAY 149 

    0xd8fcbdbf,// 153 PAY 150 

    0xac1c24df,// 154 PAY 151 

    0xadea1284,// 155 PAY 152 

    0xb541c8b4,// 156 PAY 153 

    0xf65e2eb7,// 157 PAY 154 

    0x0400207c,// 158 PAY 155 

    0x48d25d28,// 159 PAY 156 

    0x75e16f28,// 160 PAY 157 

    0x3785830d,// 161 PAY 158 

    0xe6f7ef0c,// 162 PAY 159 

    0x99c443a0,// 163 PAY 160 

    0x5b6a2400,// 164 PAY 161 

/// HASH is  12 bytes 

    0x286e57a1,// 165 HSH   1 

    0x8560f2d2,// 166 HSH   2 

    0xd24346d9,// 167 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 20 

/// STA pkt_idx        : 132 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x13 

    0x02111314 // 168 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt71_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 346 words. 

/// BDA size     is 1377 (0x561) 

/// BDA id       is 0xc9c9 

    0x0561c9c9,// 3 BDA   1 

/// PAY Generic Data size   : 1377 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0f2a0ea6,// 4 PAY   1 

    0x8045e8e5,// 5 PAY   2 

    0xdce7eb9f,// 6 PAY   3 

    0xb12ddc9d,// 7 PAY   4 

    0x5720fff8,// 8 PAY   5 

    0xabe7b753,// 9 PAY   6 

    0xaee60120,// 10 PAY   7 

    0x7b908453,// 11 PAY   8 

    0x09730728,// 12 PAY   9 

    0x38b3777a,// 13 PAY  10 

    0x9d5e6b46,// 14 PAY  11 

    0xa34eb4ab,// 15 PAY  12 

    0xaddfd3e4,// 16 PAY  13 

    0x96091255,// 17 PAY  14 

    0x5f7f572f,// 18 PAY  15 

    0xb5961a23,// 19 PAY  16 

    0x0aa18ee6,// 20 PAY  17 

    0x0f57a140,// 21 PAY  18 

    0x4a1e4a53,// 22 PAY  19 

    0xe79a02d9,// 23 PAY  20 

    0x08a24421,// 24 PAY  21 

    0x0caf934b,// 25 PAY  22 

    0x730672c8,// 26 PAY  23 

    0x57217d0e,// 27 PAY  24 

    0xd2465b3f,// 28 PAY  25 

    0x079dcf2a,// 29 PAY  26 

    0x29bac8a0,// 30 PAY  27 

    0x585639c8,// 31 PAY  28 

    0x6f696e01,// 32 PAY  29 

    0xf1e71b82,// 33 PAY  30 

    0x07986914,// 34 PAY  31 

    0x94f648b0,// 35 PAY  32 

    0x6b7ad3d1,// 36 PAY  33 

    0x0396f6c2,// 37 PAY  34 

    0x9c2a26d3,// 38 PAY  35 

    0x36c20979,// 39 PAY  36 

    0x81df17a3,// 40 PAY  37 

    0x2dc338c8,// 41 PAY  38 

    0x28371ae6,// 42 PAY  39 

    0xf6d069de,// 43 PAY  40 

    0x543d4d07,// 44 PAY  41 

    0xeb09a2c9,// 45 PAY  42 

    0x76da0d60,// 46 PAY  43 

    0x83120e96,// 47 PAY  44 

    0x1ff63663,// 48 PAY  45 

    0xb84e0529,// 49 PAY  46 

    0x7ae8a415,// 50 PAY  47 

    0xeb9555a2,// 51 PAY  48 

    0x3651f616,// 52 PAY  49 

    0xa6ad2f01,// 53 PAY  50 

    0xee30f979,// 54 PAY  51 

    0xc810231d,// 55 PAY  52 

    0xaeaa55d3,// 56 PAY  53 

    0xb72b4958,// 57 PAY  54 

    0x415ccb89,// 58 PAY  55 

    0xb432f7c1,// 59 PAY  56 

    0x1c81fa97,// 60 PAY  57 

    0xc7fae273,// 61 PAY  58 

    0xdf962fd9,// 62 PAY  59 

    0xa6fe71e0,// 63 PAY  60 

    0x405abc36,// 64 PAY  61 

    0x0ad6e8ed,// 65 PAY  62 

    0x31ec6605,// 66 PAY  63 

    0x27f7be31,// 67 PAY  64 

    0x859d2bdb,// 68 PAY  65 

    0xdc92d336,// 69 PAY  66 

    0x6938d1c3,// 70 PAY  67 

    0xef76a541,// 71 PAY  68 

    0xd86df547,// 72 PAY  69 

    0xdea17f8c,// 73 PAY  70 

    0x6425cc81,// 74 PAY  71 

    0x84cdd12e,// 75 PAY  72 

    0xeeed8270,// 76 PAY  73 

    0xe56331f0,// 77 PAY  74 

    0xfea4f5d1,// 78 PAY  75 

    0xaf293ce6,// 79 PAY  76 

    0xe97c44de,// 80 PAY  77 

    0xcd7f4f08,// 81 PAY  78 

    0x2f64b546,// 82 PAY  79 

    0xf6aa69b6,// 83 PAY  80 

    0xdcb71168,// 84 PAY  81 

    0xbab8f1bb,// 85 PAY  82 

    0x57d5ee01,// 86 PAY  83 

    0x625ff6e4,// 87 PAY  84 

    0x310f9664,// 88 PAY  85 

    0x7447cbf7,// 89 PAY  86 

    0xa517698b,// 90 PAY  87 

    0x9956d525,// 91 PAY  88 

    0x76be2f85,// 92 PAY  89 

    0x125a84cd,// 93 PAY  90 

    0x1d3d77ec,// 94 PAY  91 

    0x6f9bc390,// 95 PAY  92 

    0xac71c85c,// 96 PAY  93 

    0xb8e80ca5,// 97 PAY  94 

    0x04f74fd6,// 98 PAY  95 

    0x5a3aa35f,// 99 PAY  96 

    0xc19adc5b,// 100 PAY  97 

    0x8f3bc89d,// 101 PAY  98 

    0x16336f0c,// 102 PAY  99 

    0x7a355bae,// 103 PAY 100 

    0x64af0a55,// 104 PAY 101 

    0xfdec35d4,// 105 PAY 102 

    0xab7f2e3e,// 106 PAY 103 

    0xa78cf70c,// 107 PAY 104 

    0xcc1e79f1,// 108 PAY 105 

    0x529c4624,// 109 PAY 106 

    0x30db3a95,// 110 PAY 107 

    0xfe053af0,// 111 PAY 108 

    0xd4f46da3,// 112 PAY 109 

    0xc152c14b,// 113 PAY 110 

    0x69eb9641,// 114 PAY 111 

    0x2c56ce8a,// 115 PAY 112 

    0x29d903f2,// 116 PAY 113 

    0xca58c779,// 117 PAY 114 

    0x5aa1f2f4,// 118 PAY 115 

    0xe83dad58,// 119 PAY 116 

    0xd3747f2d,// 120 PAY 117 

    0x75883634,// 121 PAY 118 

    0x54cfc709,// 122 PAY 119 

    0x465d5fc4,// 123 PAY 120 

    0x02b97bb9,// 124 PAY 121 

    0xe6d04766,// 125 PAY 122 

    0x514593e8,// 126 PAY 123 

    0x6d5a5001,// 127 PAY 124 

    0x9dd92b0f,// 128 PAY 125 

    0x5f839c8c,// 129 PAY 126 

    0xd7463b23,// 130 PAY 127 

    0x7ef7db7e,// 131 PAY 128 

    0xe634d8cd,// 132 PAY 129 

    0xcd48fad5,// 133 PAY 130 

    0xe45e598d,// 134 PAY 131 

    0x89416bce,// 135 PAY 132 

    0xc74e002f,// 136 PAY 133 

    0x4dfece60,// 137 PAY 134 

    0x25b661a2,// 138 PAY 135 

    0x35e2be80,// 139 PAY 136 

    0x506ada4a,// 140 PAY 137 

    0xa89d856e,// 141 PAY 138 

    0x3ed855fc,// 142 PAY 139 

    0xc3bf1397,// 143 PAY 140 

    0xb5dba28a,// 144 PAY 141 

    0x056a8e30,// 145 PAY 142 

    0x937d479e,// 146 PAY 143 

    0x152e9962,// 147 PAY 144 

    0x9b4b158d,// 148 PAY 145 

    0x0b9896b2,// 149 PAY 146 

    0x2da92449,// 150 PAY 147 

    0x7bfb15cf,// 151 PAY 148 

    0x157acbc4,// 152 PAY 149 

    0x44c6e716,// 153 PAY 150 

    0x9ea8e637,// 154 PAY 151 

    0x11c8cb31,// 155 PAY 152 

    0xe4a0b65c,// 156 PAY 153 

    0x115e351f,// 157 PAY 154 

    0x696ae8a6,// 158 PAY 155 

    0x879d449b,// 159 PAY 156 

    0x1b109e06,// 160 PAY 157 

    0xa2a2db90,// 161 PAY 158 

    0x462f6625,// 162 PAY 159 

    0x2a6d7556,// 163 PAY 160 

    0xad789c3e,// 164 PAY 161 

    0x4d790bd0,// 165 PAY 162 

    0xc981d5e9,// 166 PAY 163 

    0x4b719fdf,// 167 PAY 164 

    0xbdcd798e,// 168 PAY 165 

    0x5d1e3ca2,// 169 PAY 166 

    0x40f9a928,// 170 PAY 167 

    0xc390bcdd,// 171 PAY 168 

    0x28e09484,// 172 PAY 169 

    0x43e5ac60,// 173 PAY 170 

    0x4b3ae0de,// 174 PAY 171 

    0xf22eb863,// 175 PAY 172 

    0x3ea6700e,// 176 PAY 173 

    0x5a3b5439,// 177 PAY 174 

    0xd28fb70a,// 178 PAY 175 

    0xc621085b,// 179 PAY 176 

    0xaf12bf5d,// 180 PAY 177 

    0xcade557b,// 181 PAY 178 

    0xc8d67b64,// 182 PAY 179 

    0x12cdafa1,// 183 PAY 180 

    0x3f687664,// 184 PAY 181 

    0x8053d026,// 185 PAY 182 

    0xc099092e,// 186 PAY 183 

    0x8fbd0cc5,// 187 PAY 184 

    0x5c4ffda1,// 188 PAY 185 

    0x6cad20b4,// 189 PAY 186 

    0xda78d895,// 190 PAY 187 

    0x6566ca53,// 191 PAY 188 

    0x2164dcef,// 192 PAY 189 

    0x7af3b123,// 193 PAY 190 

    0x7fae9c51,// 194 PAY 191 

    0x8591cd70,// 195 PAY 192 

    0x3820fb8e,// 196 PAY 193 

    0x78f1b4a3,// 197 PAY 194 

    0x079e2de7,// 198 PAY 195 

    0xca9431cb,// 199 PAY 196 

    0xb2866964,// 200 PAY 197 

    0xdcf8554b,// 201 PAY 198 

    0xa1d36996,// 202 PAY 199 

    0xa4c631a4,// 203 PAY 200 

    0xd1ecd319,// 204 PAY 201 

    0xff6d64a4,// 205 PAY 202 

    0x168d8bba,// 206 PAY 203 

    0x94fbb9b3,// 207 PAY 204 

    0xc23dfbc0,// 208 PAY 205 

    0xf715b97d,// 209 PAY 206 

    0xe7b29d9f,// 210 PAY 207 

    0x8b1d5ca0,// 211 PAY 208 

    0x3371df66,// 212 PAY 209 

    0xcdacaca4,// 213 PAY 210 

    0x27ba2077,// 214 PAY 211 

    0x1d99d0d5,// 215 PAY 212 

    0x8363f120,// 216 PAY 213 

    0xeffcd87f,// 217 PAY 214 

    0xea2679a9,// 218 PAY 215 

    0x52b8a574,// 219 PAY 216 

    0xc610e4e6,// 220 PAY 217 

    0x19eec8d5,// 221 PAY 218 

    0x201297c8,// 222 PAY 219 

    0xcadf3fb6,// 223 PAY 220 

    0x6276c27a,// 224 PAY 221 

    0x84544b12,// 225 PAY 222 

    0x8300bd45,// 226 PAY 223 

    0x4a369fc3,// 227 PAY 224 

    0x79a703f5,// 228 PAY 225 

    0x988fd9fe,// 229 PAY 226 

    0x1c8ede8a,// 230 PAY 227 

    0xd789560e,// 231 PAY 228 

    0x1d33b639,// 232 PAY 229 

    0xbdcc26c8,// 233 PAY 230 

    0x9300ccce,// 234 PAY 231 

    0x9495d212,// 235 PAY 232 

    0x82fa706d,// 236 PAY 233 

    0x9c07624f,// 237 PAY 234 

    0x33ad4c96,// 238 PAY 235 

    0xeb5fdabf,// 239 PAY 236 

    0xda444c9d,// 240 PAY 237 

    0x1e878f35,// 241 PAY 238 

    0x1168e676,// 242 PAY 239 

    0xf3d8c292,// 243 PAY 240 

    0x146ecb79,// 244 PAY 241 

    0xddcbbf52,// 245 PAY 242 

    0x4c5e5a74,// 246 PAY 243 

    0x0b529dbf,// 247 PAY 244 

    0x7ebc9f61,// 248 PAY 245 

    0xb7dee152,// 249 PAY 246 

    0x6c9e37f3,// 250 PAY 247 

    0x708b444d,// 251 PAY 248 

    0xc7ec48b5,// 252 PAY 249 

    0x01a1db0b,// 253 PAY 250 

    0x045e5bc9,// 254 PAY 251 

    0xa03714c9,// 255 PAY 252 

    0x2e01edfc,// 256 PAY 253 

    0x9cb882e7,// 257 PAY 254 

    0x0befb83b,// 258 PAY 255 

    0x4dee41fd,// 259 PAY 256 

    0x0a57dc0d,// 260 PAY 257 

    0x3c01a48e,// 261 PAY 258 

    0xf85a9c08,// 262 PAY 259 

    0x9a023edf,// 263 PAY 260 

    0x4eb7f442,// 264 PAY 261 

    0x3e97151d,// 265 PAY 262 

    0x6115f9f3,// 266 PAY 263 

    0x080ea43d,// 267 PAY 264 

    0x573a408a,// 268 PAY 265 

    0x08241f9d,// 269 PAY 266 

    0x0c1ba14a,// 270 PAY 267 

    0x4f18cd0d,// 271 PAY 268 

    0xbfeb67cc,// 272 PAY 269 

    0xe77664a3,// 273 PAY 270 

    0x534e9848,// 274 PAY 271 

    0xaaf72eb4,// 275 PAY 272 

    0x2f85cdc9,// 276 PAY 273 

    0x3192b929,// 277 PAY 274 

    0xbd14df0a,// 278 PAY 275 

    0x404a3993,// 279 PAY 276 

    0xafb80388,// 280 PAY 277 

    0x1c182eca,// 281 PAY 278 

    0x40b84ebb,// 282 PAY 279 

    0xdd907731,// 283 PAY 280 

    0x608a08c5,// 284 PAY 281 

    0x8dafd6ed,// 285 PAY 282 

    0x142f045c,// 286 PAY 283 

    0xcdcbed1e,// 287 PAY 284 

    0xedf6a461,// 288 PAY 285 

    0x65891603,// 289 PAY 286 

    0x454f1f0b,// 290 PAY 287 

    0x7f503409,// 291 PAY 288 

    0xfbde7e5f,// 292 PAY 289 

    0x935230fe,// 293 PAY 290 

    0x01277f0b,// 294 PAY 291 

    0x6efddc10,// 295 PAY 292 

    0x830a7469,// 296 PAY 293 

    0xb829f93a,// 297 PAY 294 

    0x28255fb1,// 298 PAY 295 

    0x7c887d1b,// 299 PAY 296 

    0x5c75c1cb,// 300 PAY 297 

    0xa335874d,// 301 PAY 298 

    0xd758d5c3,// 302 PAY 299 

    0xff1a2c86,// 303 PAY 300 

    0x92dd1480,// 304 PAY 301 

    0x81d412a6,// 305 PAY 302 

    0x2cc85308,// 306 PAY 303 

    0x812f2d65,// 307 PAY 304 

    0x80734e93,// 308 PAY 305 

    0x9b75e249,// 309 PAY 306 

    0xfe071e25,// 310 PAY 307 

    0xaf6646b9,// 311 PAY 308 

    0x61cf071a,// 312 PAY 309 

    0xb4e15e21,// 313 PAY 310 

    0xae119cc6,// 314 PAY 311 

    0x0ae46ecb,// 315 PAY 312 

    0xc59f929a,// 316 PAY 313 

    0x9e25402f,// 317 PAY 314 

    0x1247c380,// 318 PAY 315 

    0xeb34367e,// 319 PAY 316 

    0x7a22c6aa,// 320 PAY 317 

    0xfa47f032,// 321 PAY 318 

    0x02703359,// 322 PAY 319 

    0xc48349da,// 323 PAY 320 

    0xaf2e7bb4,// 324 PAY 321 

    0xaa3aaa60,// 325 PAY 322 

    0x2f74bac2,// 326 PAY 323 

    0x91ccac29,// 327 PAY 324 

    0xf54f44d5,// 328 PAY 325 

    0xbae23c5a,// 329 PAY 326 

    0x34cd7a28,// 330 PAY 327 

    0x8700f19d,// 331 PAY 328 

    0xbcd2f877,// 332 PAY 329 

    0x37175925,// 333 PAY 330 

    0x04cfdf73,// 334 PAY 331 

    0x0562638e,// 335 PAY 332 

    0x9b3e0207,// 336 PAY 333 

    0xe094e3c7,// 337 PAY 334 

    0xad67c4c5,// 338 PAY 335 

    0x2d8344b0,// 339 PAY 336 

    0xdfeabc5a,// 340 PAY 337 

    0x01ce7cf6,// 341 PAY 338 

    0x4934a073,// 342 PAY 339 

    0xb58a4b75,// 343 PAY 340 

    0xc74c15b6,// 344 PAY 341 

    0xee40a6c4,// 345 PAY 342 

    0x296e92bd,// 346 PAY 343 

    0xa5a8d7b3,// 347 PAY 344 

    0x57000000,// 348 PAY 345 

/// HASH is  4 bytes 

    0xd28fb70a,// 349 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 85 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf5 

    0x02ccf555 // 350 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt72_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 317 words. 

/// BDA size     is 1263 (0x4ef) 

/// BDA id       is 0xdad4 

    0x04efdad4,// 3 BDA   1 

/// PAY Generic Data size   : 1263 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x7fa7d4f4,// 4 PAY   1 

    0x4efd4d5c,// 5 PAY   2 

    0x1e447ef7,// 6 PAY   3 

    0x76ad3dff,// 7 PAY   4 

    0x29ea7c7f,// 8 PAY   5 

    0xae64bae1,// 9 PAY   6 

    0x540f5dc9,// 10 PAY   7 

    0xc42f7144,// 11 PAY   8 

    0x3a6d8d99,// 12 PAY   9 

    0x7fa81eda,// 13 PAY  10 

    0x92225263,// 14 PAY  11 

    0xbddb21a9,// 15 PAY  12 

    0xbe3ebdc6,// 16 PAY  13 

    0xaea4f97e,// 17 PAY  14 

    0xdb48aa84,// 18 PAY  15 

    0x707f72d8,// 19 PAY  16 

    0x6587d4f8,// 20 PAY  17 

    0x2320b9c9,// 21 PAY  18 

    0x60a6a9cb,// 22 PAY  19 

    0x221b651f,// 23 PAY  20 

    0x0b2f056d,// 24 PAY  21 

    0xcf9f4c47,// 25 PAY  22 

    0x917e5b3a,// 26 PAY  23 

    0x0c5e47ff,// 27 PAY  24 

    0xab1fd817,// 28 PAY  25 

    0x5afb0447,// 29 PAY  26 

    0x614085f9,// 30 PAY  27 

    0xb0201db3,// 31 PAY  28 

    0x5e2772a9,// 32 PAY  29 

    0xe3a827eb,// 33 PAY  30 

    0x06bc5ebf,// 34 PAY  31 

    0xc441ecd0,// 35 PAY  32 

    0x48f90371,// 36 PAY  33 

    0x73c4856c,// 37 PAY  34 

    0x289c27df,// 38 PAY  35 

    0x5db08cee,// 39 PAY  36 

    0x0092e44b,// 40 PAY  37 

    0x3ad0365c,// 41 PAY  38 

    0x1329ff38,// 42 PAY  39 

    0x24574afb,// 43 PAY  40 

    0x0eee4185,// 44 PAY  41 

    0x9ac52f31,// 45 PAY  42 

    0x60f71913,// 46 PAY  43 

    0x05e84e00,// 47 PAY  44 

    0xea46a2ac,// 48 PAY  45 

    0xbf155ecf,// 49 PAY  46 

    0xdeec00e8,// 50 PAY  47 

    0x3d821599,// 51 PAY  48 

    0x638edfa9,// 52 PAY  49 

    0xf1eb3ec4,// 53 PAY  50 

    0xf5853e47,// 54 PAY  51 

    0xb55192f5,// 55 PAY  52 

    0x3c2b96d2,// 56 PAY  53 

    0xa57c2b5b,// 57 PAY  54 

    0x1bd7daf2,// 58 PAY  55 

    0xbc51bc93,// 59 PAY  56 

    0x5cc2dcc9,// 60 PAY  57 

    0x67a6d49c,// 61 PAY  58 

    0x5cdcd2b1,// 62 PAY  59 

    0x80b49127,// 63 PAY  60 

    0xd59b0031,// 64 PAY  61 

    0xe187e7a7,// 65 PAY  62 

    0x8cb333c3,// 66 PAY  63 

    0xbe7fc920,// 67 PAY  64 

    0xafdc8169,// 68 PAY  65 

    0xcd65f268,// 69 PAY  66 

    0x70f02562,// 70 PAY  67 

    0x45ee6e87,// 71 PAY  68 

    0xc2d30c7e,// 72 PAY  69 

    0x4f1daee7,// 73 PAY  70 

    0x5cd0cce5,// 74 PAY  71 

    0x7dc46a9b,// 75 PAY  72 

    0x304e6994,// 76 PAY  73 

    0xb8ddc616,// 77 PAY  74 

    0xd8fffef3,// 78 PAY  75 

    0xd0cc3ae5,// 79 PAY  76 

    0x304bdb54,// 80 PAY  77 

    0x5a6b171a,// 81 PAY  78 

    0x6f0432b5,// 82 PAY  79 

    0xf5f0f146,// 83 PAY  80 

    0x62425f5e,// 84 PAY  81 

    0xa4c694ea,// 85 PAY  82 

    0xca011e10,// 86 PAY  83 

    0x0f3bd381,// 87 PAY  84 

    0x6c4a0746,// 88 PAY  85 

    0xd15c1f53,// 89 PAY  86 

    0xa79ad79a,// 90 PAY  87 

    0x854d83cd,// 91 PAY  88 

    0x4161d57b,// 92 PAY  89 

    0xbd2b6536,// 93 PAY  90 

    0x8ca0b058,// 94 PAY  91 

    0x6a82cd46,// 95 PAY  92 

    0xb67c6230,// 96 PAY  93 

    0x717e69bb,// 97 PAY  94 

    0x35ae75b5,// 98 PAY  95 

    0x9ae37904,// 99 PAY  96 

    0x65ab1eea,// 100 PAY  97 

    0x1cd926c7,// 101 PAY  98 

    0x670f9d34,// 102 PAY  99 

    0xde4ecbff,// 103 PAY 100 

    0xd58ff679,// 104 PAY 101 

    0xd2f99be9,// 105 PAY 102 

    0xa20e98b0,// 106 PAY 103 

    0xb8e7f49f,// 107 PAY 104 

    0xb0e76081,// 108 PAY 105 

    0xbcaf6df5,// 109 PAY 106 

    0xf0186e19,// 110 PAY 107 

    0x71cbb0cf,// 111 PAY 108 

    0xbfa88350,// 112 PAY 109 

    0x6234ba89,// 113 PAY 110 

    0x8c05b549,// 114 PAY 111 

    0x83cb2b81,// 115 PAY 112 

    0xb82c33e3,// 116 PAY 113 

    0xf0690031,// 117 PAY 114 

    0x7e1cf4a7,// 118 PAY 115 

    0x94662705,// 119 PAY 116 

    0xe3d64979,// 120 PAY 117 

    0x3e51f66b,// 121 PAY 118 

    0xfc8552fc,// 122 PAY 119 

    0xd4c08fb3,// 123 PAY 120 

    0xde979a30,// 124 PAY 121 

    0xfa54da25,// 125 PAY 122 

    0x81ef1650,// 126 PAY 123 

    0xeccca8c8,// 127 PAY 124 

    0xb39c3835,// 128 PAY 125 

    0x50133311,// 129 PAY 126 

    0x889edb08,// 130 PAY 127 

    0xc74aa058,// 131 PAY 128 

    0xfa1296b8,// 132 PAY 129 

    0x0012c1e4,// 133 PAY 130 

    0x12aae947,// 134 PAY 131 

    0xde804714,// 135 PAY 132 

    0x1fc34426,// 136 PAY 133 

    0x8174c32c,// 137 PAY 134 

    0xc64798ab,// 138 PAY 135 

    0xbe7f14f1,// 139 PAY 136 

    0x743de9a4,// 140 PAY 137 

    0xa329b52a,// 141 PAY 138 

    0x08e618ee,// 142 PAY 139 

    0xadd366bb,// 143 PAY 140 

    0xf8ae6c9f,// 144 PAY 141 

    0x2a31b684,// 145 PAY 142 

    0x19b57234,// 146 PAY 143 

    0x98533061,// 147 PAY 144 

    0xdd4ec9ed,// 148 PAY 145 

    0x3787ec90,// 149 PAY 146 

    0xd5789aac,// 150 PAY 147 

    0x210f46ab,// 151 PAY 148 

    0xaaf74bd0,// 152 PAY 149 

    0x85d11338,// 153 PAY 150 

    0xb232111b,// 154 PAY 151 

    0xe187154e,// 155 PAY 152 

    0x5c583b2a,// 156 PAY 153 

    0xc45172b6,// 157 PAY 154 

    0xcfce8f28,// 158 PAY 155 

    0x651377f9,// 159 PAY 156 

    0xd5a550ef,// 160 PAY 157 

    0xa950fdeb,// 161 PAY 158 

    0x847273c3,// 162 PAY 159 

    0x99d2fe59,// 163 PAY 160 

    0x8f54d1fc,// 164 PAY 161 

    0x3ba48508,// 165 PAY 162 

    0x972ee34a,// 166 PAY 163 

    0xea4fceb0,// 167 PAY 164 

    0x23caaf93,// 168 PAY 165 

    0xfb4d5d7f,// 169 PAY 166 

    0xe9556205,// 170 PAY 167 

    0x8bc54877,// 171 PAY 168 

    0x85e6f3e2,// 172 PAY 169 

    0x50d591ce,// 173 PAY 170 

    0x16543faa,// 174 PAY 171 

    0x27855252,// 175 PAY 172 

    0x89a3ee53,// 176 PAY 173 

    0x6ca387c3,// 177 PAY 174 

    0x01ef738b,// 178 PAY 175 

    0x47464175,// 179 PAY 176 

    0x8eb93453,// 180 PAY 177 

    0x462e10d6,// 181 PAY 178 

    0xa591e735,// 182 PAY 179 

    0x3d3800db,// 183 PAY 180 

    0xc5164366,// 184 PAY 181 

    0x64210e02,// 185 PAY 182 

    0xca09626b,// 186 PAY 183 

    0x933d01db,// 187 PAY 184 

    0xdeeab75a,// 188 PAY 185 

    0x5f3e9f5a,// 189 PAY 186 

    0xe4941815,// 190 PAY 187 

    0xf132aaec,// 191 PAY 188 

    0xf338b9bf,// 192 PAY 189 

    0xe2b9dedf,// 193 PAY 190 

    0x8d50a987,// 194 PAY 191 

    0x32f7f7ee,// 195 PAY 192 

    0x8896a3ab,// 196 PAY 193 

    0x4bf29696,// 197 PAY 194 

    0x41f4e14f,// 198 PAY 195 

    0xe5602606,// 199 PAY 196 

    0x912b84ee,// 200 PAY 197 

    0x38c1aa12,// 201 PAY 198 

    0x1dd3b0f5,// 202 PAY 199 

    0x32ecf93b,// 203 PAY 200 

    0x7e437b63,// 204 PAY 201 

    0x59096046,// 205 PAY 202 

    0x0fb46115,// 206 PAY 203 

    0xdf7b73fd,// 207 PAY 204 

    0x209a8185,// 208 PAY 205 

    0x0ba7d483,// 209 PAY 206 

    0x51090e6d,// 210 PAY 207 

    0xa804149e,// 211 PAY 208 

    0xe41368c3,// 212 PAY 209 

    0x1c4a489d,// 213 PAY 210 

    0xda12f071,// 214 PAY 211 

    0xe4ae2c37,// 215 PAY 212 

    0x43d7ce2b,// 216 PAY 213 

    0x72a3f2c3,// 217 PAY 214 

    0xb7ffd6cf,// 218 PAY 215 

    0x4ed6b4f7,// 219 PAY 216 

    0xba1aaf99,// 220 PAY 217 

    0xdfb90e2e,// 221 PAY 218 

    0x1ffc5f1e,// 222 PAY 219 

    0xda01c8d7,// 223 PAY 220 

    0xac740480,// 224 PAY 221 

    0x48c12fea,// 225 PAY 222 

    0x9ab93c9e,// 226 PAY 223 

    0x5a848fdf,// 227 PAY 224 

    0x48033ee8,// 228 PAY 225 

    0x31f316f7,// 229 PAY 226 

    0xd11b4f84,// 230 PAY 227 

    0xa6a0e9f6,// 231 PAY 228 

    0x97c7f6b5,// 232 PAY 229 

    0x67cbdb34,// 233 PAY 230 

    0x6678b306,// 234 PAY 231 

    0xc7e82d37,// 235 PAY 232 

    0xd2ff3a69,// 236 PAY 233 

    0x3469f16c,// 237 PAY 234 

    0x491d7323,// 238 PAY 235 

    0x0f9ea291,// 239 PAY 236 

    0xd707c7ec,// 240 PAY 237 

    0x22dfdc0c,// 241 PAY 238 

    0x6ea055e0,// 242 PAY 239 

    0x28e62867,// 243 PAY 240 

    0x30273cc1,// 244 PAY 241 

    0xe94241c8,// 245 PAY 242 

    0x5cbb090f,// 246 PAY 243 

    0x390a9037,// 247 PAY 244 

    0x2702f28f,// 248 PAY 245 

    0x5049654a,// 249 PAY 246 

    0x69991ecb,// 250 PAY 247 

    0xe5ccc072,// 251 PAY 248 

    0x4e766e22,// 252 PAY 249 

    0xa1dcc1ed,// 253 PAY 250 

    0x9981a7ef,// 254 PAY 251 

    0xea5296cf,// 255 PAY 252 

    0x739cc13b,// 256 PAY 253 

    0x288c04ea,// 257 PAY 254 

    0x9d6049be,// 258 PAY 255 

    0x210628b9,// 259 PAY 256 

    0xd47d6a26,// 260 PAY 257 

    0x49576ac4,// 261 PAY 258 

    0xce7d268f,// 262 PAY 259 

    0x652a11e3,// 263 PAY 260 

    0xb0156eb5,// 264 PAY 261 

    0x9a05da57,// 265 PAY 262 

    0x7ad95936,// 266 PAY 263 

    0x557a8b82,// 267 PAY 264 

    0x47dd0d85,// 268 PAY 265 

    0xfbd70447,// 269 PAY 266 

    0xfbd89278,// 270 PAY 267 

    0xea9f7ae7,// 271 PAY 268 

    0x60fac404,// 272 PAY 269 

    0x6d6798d5,// 273 PAY 270 

    0x8c79f1af,// 274 PAY 271 

    0x434e5d13,// 275 PAY 272 

    0xa6551347,// 276 PAY 273 

    0x5404e469,// 277 PAY 274 

    0x2c87a576,// 278 PAY 275 

    0xfab3ae88,// 279 PAY 276 

    0xad46684f,// 280 PAY 277 

    0xde77ee34,// 281 PAY 278 

    0x84a95396,// 282 PAY 279 

    0xdc79cfdb,// 283 PAY 280 

    0xdac9171c,// 284 PAY 281 

    0xb9c06e32,// 285 PAY 282 

    0x08b1072a,// 286 PAY 283 

    0x52211e95,// 287 PAY 284 

    0xed311425,// 288 PAY 285 

    0x8e3e0d0c,// 289 PAY 286 

    0x38fdb9e7,// 290 PAY 287 

    0xabb16910,// 291 PAY 288 

    0x8537fb64,// 292 PAY 289 

    0x79637e8d,// 293 PAY 290 

    0x3e0a2d83,// 294 PAY 291 

    0xa59ab660,// 295 PAY 292 

    0x10f9b988,// 296 PAY 293 

    0xed9c665b,// 297 PAY 294 

    0x04f8b697,// 298 PAY 295 

    0x9960c693,// 299 PAY 296 

    0xcccc8cba,// 300 PAY 297 

    0xf16a2197,// 301 PAY 298 

    0xbd22ee16,// 302 PAY 299 

    0xe16ff8ed,// 303 PAY 300 

    0xe5636005,// 304 PAY 301 

    0x6ffce3c0,// 305 PAY 302 

    0x65ab6a02,// 306 PAY 303 

    0x61af63f9,// 307 PAY 304 

    0xf1859b1f,// 308 PAY 305 

    0xb201dc02,// 309 PAY 306 

    0xbe0ba268,// 310 PAY 307 

    0x574d6dcc,// 311 PAY 308 

    0x727c9b7d,// 312 PAY 309 

    0xba99aaa7,// 313 PAY 310 

    0x7974e705,// 314 PAY 311 

    0x6dccc142,// 315 PAY 312 

    0xf4335d3d,// 316 PAY 313 

    0x3b25477c,// 317 PAY 314 

    0x18845d45,// 318 PAY 315 

    0x49505800,// 319 PAY 316 

/// HASH is  16 bytes 

    0x4ed6b4f7,// 320 HSH   1 

    0xba1aaf99,// 321 HSH   2 

    0xdfb90e2e,// 322 HSH   3 

    0x1ffc5f1e,// 323 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 45 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x94 

    0x00b5942c // 324 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt73_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 341 words. 

/// BDA size     is 1358 (0x54e) 

/// BDA id       is 0xb041 

    0x054eb041,// 3 BDA   1 

/// PAY Generic Data size   : 1358 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x7388c811,// 4 PAY   1 

    0x012c377e,// 5 PAY   2 

    0x5f6c5a15,// 6 PAY   3 

    0x7db9e8d8,// 7 PAY   4 

    0x3c3635cc,// 8 PAY   5 

    0xba162990,// 9 PAY   6 

    0x61bb3457,// 10 PAY   7 

    0x9c102f6a,// 11 PAY   8 

    0x1b0863fe,// 12 PAY   9 

    0xee6c611e,// 13 PAY  10 

    0x82324144,// 14 PAY  11 

    0xa7804274,// 15 PAY  12 

    0x3aa4ca96,// 16 PAY  13 

    0xed08c494,// 17 PAY  14 

    0x313169a7,// 18 PAY  15 

    0x74fda078,// 19 PAY  16 

    0x866ed2d6,// 20 PAY  17 

    0xae85130d,// 21 PAY  18 

    0x2748a1f0,// 22 PAY  19 

    0xaccf3dc5,// 23 PAY  20 

    0x8ced46fc,// 24 PAY  21 

    0x5bb60325,// 25 PAY  22 

    0x9c4389e5,// 26 PAY  23 

    0x59ed7d6e,// 27 PAY  24 

    0x53bb222b,// 28 PAY  25 

    0xe8e80a3c,// 29 PAY  26 

    0x6025dd6b,// 30 PAY  27 

    0xd3fa9964,// 31 PAY  28 

    0x43dd68b9,// 32 PAY  29 

    0x1afd1265,// 33 PAY  30 

    0x3bb3d4d4,// 34 PAY  31 

    0xf4f7f3b4,// 35 PAY  32 

    0xcb51863a,// 36 PAY  33 

    0x67f95ecb,// 37 PAY  34 

    0x324e5292,// 38 PAY  35 

    0xff1078a7,// 39 PAY  36 

    0x5cf4b16e,// 40 PAY  37 

    0x9cc761e4,// 41 PAY  38 

    0x0d718da3,// 42 PAY  39 

    0x6cd36d18,// 43 PAY  40 

    0x147144bd,// 44 PAY  41 

    0xe20b2cb5,// 45 PAY  42 

    0x5d902b34,// 46 PAY  43 

    0xdbcd395b,// 47 PAY  44 

    0x347060f3,// 48 PAY  45 

    0x84dfc1f9,// 49 PAY  46 

    0xcee5921e,// 50 PAY  47 

    0x992a9239,// 51 PAY  48 

    0xa89b4a78,// 52 PAY  49 

    0x376fe45c,// 53 PAY  50 

    0x670ca631,// 54 PAY  51 

    0xc151850f,// 55 PAY  52 

    0xbee99544,// 56 PAY  53 

    0x9211b06f,// 57 PAY  54 

    0x85bc3ef2,// 58 PAY  55 

    0xf02b6ecd,// 59 PAY  56 

    0x2059a0a2,// 60 PAY  57 

    0x861e9ec3,// 61 PAY  58 

    0xb378ad21,// 62 PAY  59 

    0xa6643462,// 63 PAY  60 

    0x6ce982e3,// 64 PAY  61 

    0x0293a872,// 65 PAY  62 

    0x32995eec,// 66 PAY  63 

    0xc034a89e,// 67 PAY  64 

    0xd5542cc9,// 68 PAY  65 

    0x5da09039,// 69 PAY  66 

    0xdf634ac0,// 70 PAY  67 

    0x3516ab99,// 71 PAY  68 

    0xc47cbc23,// 72 PAY  69 

    0xd9649782,// 73 PAY  70 

    0x32028d2d,// 74 PAY  71 

    0xa08685f3,// 75 PAY  72 

    0xd8408d0c,// 76 PAY  73 

    0xfc164dfd,// 77 PAY  74 

    0xc3cdde75,// 78 PAY  75 

    0x00faaf99,// 79 PAY  76 

    0xded6c919,// 80 PAY  77 

    0x2356a441,// 81 PAY  78 

    0x3ccfc340,// 82 PAY  79 

    0x6903a6d1,// 83 PAY  80 

    0x7343ff13,// 84 PAY  81 

    0x0a9546f3,// 85 PAY  82 

    0x0ef429e2,// 86 PAY  83 

    0xd2f3ca73,// 87 PAY  84 

    0x429f789b,// 88 PAY  85 

    0xf9fcd9e5,// 89 PAY  86 

    0x83eb7146,// 90 PAY  87 

    0x14fe9aa0,// 91 PAY  88 

    0xfdc0e7ee,// 92 PAY  89 

    0x1e86febb,// 93 PAY  90 

    0xaf762605,// 94 PAY  91 

    0xf03db7f6,// 95 PAY  92 

    0x1b18fe76,// 96 PAY  93 

    0x52d511bc,// 97 PAY  94 

    0x0ee92347,// 98 PAY  95 

    0xd481b7ec,// 99 PAY  96 

    0x0f895230,// 100 PAY  97 

    0x0b8488a0,// 101 PAY  98 

    0xc548ba0f,// 102 PAY  99 

    0x5a7987d5,// 103 PAY 100 

    0xd7f4e0f8,// 104 PAY 101 

    0x82cfffe0,// 105 PAY 102 

    0x3441b6e3,// 106 PAY 103 

    0xa87fc5ae,// 107 PAY 104 

    0x6e584197,// 108 PAY 105 

    0x2784c0cb,// 109 PAY 106 

    0x4a1d6000,// 110 PAY 107 

    0xfc60aa81,// 111 PAY 108 

    0x3884d975,// 112 PAY 109 

    0x8572f701,// 113 PAY 110 

    0x494e5c5a,// 114 PAY 111 

    0x00d10d5f,// 115 PAY 112 

    0xe768713a,// 116 PAY 113 

    0x3f5dc95f,// 117 PAY 114 

    0x23aca171,// 118 PAY 115 

    0x071baae8,// 119 PAY 116 

    0x11bdd6f9,// 120 PAY 117 

    0x88a7bc68,// 121 PAY 118 

    0xe2d0c475,// 122 PAY 119 

    0x22eaab71,// 123 PAY 120 

    0x63079f27,// 124 PAY 121 

    0x7ce32da1,// 125 PAY 122 

    0x7ad60baa,// 126 PAY 123 

    0x28663f7c,// 127 PAY 124 

    0xe41e4603,// 128 PAY 125 

    0x0ffbf733,// 129 PAY 126 

    0x37799363,// 130 PAY 127 

    0x3c627bff,// 131 PAY 128 

    0x5a9c1be5,// 132 PAY 129 

    0x606c14b4,// 133 PAY 130 

    0xdc54bbd1,// 134 PAY 131 

    0x5fa18cda,// 135 PAY 132 

    0xb7267cd6,// 136 PAY 133 

    0x2fa46942,// 137 PAY 134 

    0xbe7cd425,// 138 PAY 135 

    0xc38ac45d,// 139 PAY 136 

    0x72a8c5b1,// 140 PAY 137 

    0x2a165073,// 141 PAY 138 

    0xdf7895da,// 142 PAY 139 

    0x2774c568,// 143 PAY 140 

    0x9718f502,// 144 PAY 141 

    0x89290550,// 145 PAY 142 

    0x939da5a4,// 146 PAY 143 

    0xacb07082,// 147 PAY 144 

    0x752b8970,// 148 PAY 145 

    0xe04e2647,// 149 PAY 146 

    0x007337d9,// 150 PAY 147 

    0x9eb8edda,// 151 PAY 148 

    0xa8cf3c8e,// 152 PAY 149 

    0x15b99ef9,// 153 PAY 150 

    0xdfac629c,// 154 PAY 151 

    0x7040633c,// 155 PAY 152 

    0xe5f2b7f8,// 156 PAY 153 

    0x0cb854ba,// 157 PAY 154 

    0xb7741103,// 158 PAY 155 

    0x8156c4c1,// 159 PAY 156 

    0x16286725,// 160 PAY 157 

    0x16d276d2,// 161 PAY 158 

    0x0d8dbd19,// 162 PAY 159 

    0x6e4bfade,// 163 PAY 160 

    0xd000def4,// 164 PAY 161 

    0x9cb4ebbd,// 165 PAY 162 

    0x80ff3c74,// 166 PAY 163 

    0x5936f5fe,// 167 PAY 164 

    0x9562435a,// 168 PAY 165 

    0xf1e2e251,// 169 PAY 166 

    0x57172034,// 170 PAY 167 

    0xd27e0edc,// 171 PAY 168 

    0xed5faf66,// 172 PAY 169 

    0x59412233,// 173 PAY 170 

    0x895f0336,// 174 PAY 171 

    0x23c5c0aa,// 175 PAY 172 

    0x31f8deec,// 176 PAY 173 

    0x3b4f6f75,// 177 PAY 174 

    0xa30f760e,// 178 PAY 175 

    0x69ec7430,// 179 PAY 176 

    0x82aa3970,// 180 PAY 177 

    0x94d941c3,// 181 PAY 178 

    0x6e446ca1,// 182 PAY 179 

    0x4635af4c,// 183 PAY 180 

    0xc7381dc5,// 184 PAY 181 

    0x55dabcf8,// 185 PAY 182 

    0x8082040d,// 186 PAY 183 

    0xd611b4b9,// 187 PAY 184 

    0x10f53a5a,// 188 PAY 185 

    0x8e3c078c,// 189 PAY 186 

    0x118ebe6c,// 190 PAY 187 

    0x4302fbb6,// 191 PAY 188 

    0x58bdb951,// 192 PAY 189 

    0x97ee2824,// 193 PAY 190 

    0x354a5099,// 194 PAY 191 

    0x332b2dab,// 195 PAY 192 

    0x1b3b6b13,// 196 PAY 193 

    0x3c952092,// 197 PAY 194 

    0xcf93b0d4,// 198 PAY 195 

    0xc426c46f,// 199 PAY 196 

    0x2c0e713b,// 200 PAY 197 

    0x04cb773a,// 201 PAY 198 

    0x52e88a24,// 202 PAY 199 

    0xda9e41ff,// 203 PAY 200 

    0x706533f0,// 204 PAY 201 

    0xccb1c430,// 205 PAY 202 

    0x86797484,// 206 PAY 203 

    0xadd517fe,// 207 PAY 204 

    0x5e643583,// 208 PAY 205 

    0xa0834d81,// 209 PAY 206 

    0xa68fe74c,// 210 PAY 207 

    0x29dbe5e2,// 211 PAY 208 

    0xe467e72d,// 212 PAY 209 

    0xa5c42125,// 213 PAY 210 

    0xcb03cb6e,// 214 PAY 211 

    0x75cb9a66,// 215 PAY 212 

    0x24bbe5dd,// 216 PAY 213 

    0x3f05d5e2,// 217 PAY 214 

    0xfe601717,// 218 PAY 215 

    0x9d60649b,// 219 PAY 216 

    0x74f28379,// 220 PAY 217 

    0x82c9deaf,// 221 PAY 218 

    0x13cc5c82,// 222 PAY 219 

    0x2c411448,// 223 PAY 220 

    0x92c113d1,// 224 PAY 221 

    0xba632ebf,// 225 PAY 222 

    0xe02ff658,// 226 PAY 223 

    0x7f3d5b7d,// 227 PAY 224 

    0x3a09eec3,// 228 PAY 225 

    0xcbf1deb2,// 229 PAY 226 

    0xb49f993a,// 230 PAY 227 

    0x5ce3d0dd,// 231 PAY 228 

    0xb53817c5,// 232 PAY 229 

    0xf698ceb0,// 233 PAY 230 

    0xc6766113,// 234 PAY 231 

    0xddcdab5b,// 235 PAY 232 

    0xadec1e70,// 236 PAY 233 

    0x96479cda,// 237 PAY 234 

    0xb431aa22,// 238 PAY 235 

    0x132a544e,// 239 PAY 236 

    0xfb78a1a8,// 240 PAY 237 

    0x22d82d45,// 241 PAY 238 

    0x67d8fdad,// 242 PAY 239 

    0xcfa030b9,// 243 PAY 240 

    0x9a16c319,// 244 PAY 241 

    0x2f3a5478,// 245 PAY 242 

    0x2374aac3,// 246 PAY 243 

    0x8ab60de3,// 247 PAY 244 

    0x13d71835,// 248 PAY 245 

    0xf86c920a,// 249 PAY 246 

    0xf3217782,// 250 PAY 247 

    0xe009e839,// 251 PAY 248 

    0x7f47abbd,// 252 PAY 249 

    0xde42ca9f,// 253 PAY 250 

    0x586cef52,// 254 PAY 251 

    0x6554a9aa,// 255 PAY 252 

    0xbe9bb724,// 256 PAY 253 

    0x7964ed60,// 257 PAY 254 

    0xdaddf3b3,// 258 PAY 255 

    0x5f948027,// 259 PAY 256 

    0x1c6f8b99,// 260 PAY 257 

    0x594f59ec,// 261 PAY 258 

    0x9fbc5145,// 262 PAY 259 

    0xfe8cb550,// 263 PAY 260 

    0x43893178,// 264 PAY 261 

    0x68836899,// 265 PAY 262 

    0xa6a891d6,// 266 PAY 263 

    0xae0b86cc,// 267 PAY 264 

    0xe6e9eb05,// 268 PAY 265 

    0x7b6b0bff,// 269 PAY 266 

    0xbac9fc98,// 270 PAY 267 

    0xdea3de6a,// 271 PAY 268 

    0x8492a91b,// 272 PAY 269 

    0xb6c50a3d,// 273 PAY 270 

    0x441997fd,// 274 PAY 271 

    0xda88272f,// 275 PAY 272 

    0x7bf6883a,// 276 PAY 273 

    0x3bd15af5,// 277 PAY 274 

    0x024a971a,// 278 PAY 275 

    0xe3b805b5,// 279 PAY 276 

    0x29aa10e0,// 280 PAY 277 

    0x6b710610,// 281 PAY 278 

    0xf63436ae,// 282 PAY 279 

    0xc4dd50cc,// 283 PAY 280 

    0xf54ec43d,// 284 PAY 281 

    0x8e71d3ba,// 285 PAY 282 

    0x3a63e8e0,// 286 PAY 283 

    0x8ea8f3a4,// 287 PAY 284 

    0x769f962b,// 288 PAY 285 

    0xd8fa233a,// 289 PAY 286 

    0x8dc2da27,// 290 PAY 287 

    0x940cee2e,// 291 PAY 288 

    0xc5a1de9b,// 292 PAY 289 

    0x66e2c30d,// 293 PAY 290 

    0x537614c9,// 294 PAY 291 

    0x10e62497,// 295 PAY 292 

    0xeeb49950,// 296 PAY 293 

    0x8435c15f,// 297 PAY 294 

    0x317017f6,// 298 PAY 295 

    0x5bed7c2e,// 299 PAY 296 

    0x18d3dc05,// 300 PAY 297 

    0xe45fb695,// 301 PAY 298 

    0xaba8c706,// 302 PAY 299 

    0x2e8baa42,// 303 PAY 300 

    0xbc418d19,// 304 PAY 301 

    0x31fad492,// 305 PAY 302 

    0xcceb9f3e,// 306 PAY 303 

    0xbe143f88,// 307 PAY 304 

    0x10b9d527,// 308 PAY 305 

    0x0de0615a,// 309 PAY 306 

    0xebeb30cc,// 310 PAY 307 

    0x521486cf,// 311 PAY 308 

    0x9af00c27,// 312 PAY 309 

    0xfb78c2d8,// 313 PAY 310 

    0x623585e7,// 314 PAY 311 

    0x1f508a35,// 315 PAY 312 

    0x76730218,// 316 PAY 313 

    0xd1317ab1,// 317 PAY 314 

    0x567f10e4,// 318 PAY 315 

    0x1b36c02d,// 319 PAY 316 

    0x08e016a3,// 320 PAY 317 

    0xeb7e44bf,// 321 PAY 318 

    0x636e253d,// 322 PAY 319 

    0x4396da5a,// 323 PAY 320 

    0xc181ea9a,// 324 PAY 321 

    0x2d3b57ab,// 325 PAY 322 

    0xd22a02f9,// 326 PAY 323 

    0x0a8f2240,// 327 PAY 324 

    0x17c18144,// 328 PAY 325 

    0xc3e0ada1,// 329 PAY 326 

    0xad3c52aa,// 330 PAY 327 

    0x7082fa44,// 331 PAY 328 

    0xc6a41065,// 332 PAY 329 

    0x343e3eac,// 333 PAY 330 

    0x07fa9ac5,// 334 PAY 331 

    0xdf9d4abf,// 335 PAY 332 

    0x1aa063a1,// 336 PAY 333 

    0xe88ed1e4,// 337 PAY 334 

    0xc34f2b53,// 338 PAY 335 

    0x33d10184,// 339 PAY 336 

    0x623cbec1,// 340 PAY 337 

    0xd5dbca9d,// 341 PAY 338 

    0x9424da99,// 342 PAY 339 

    0x78580000,// 343 PAY 340 

/// HASH is  8 bytes 

    0x7082fa44,// 344 HSH   1 

    0xc6a41065,// 345 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 218 

/// STA pkt_idx        : 241 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x44 

    0x03c544da // 346 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt74_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 372 words. 

/// BDA size     is 1481 (0x5c9) 

/// BDA id       is 0xa0be 

    0x05c9a0be,// 3 BDA   1 

/// PAY Generic Data size   : 1481 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x7315097e,// 4 PAY   1 

    0xe72f8d85,// 5 PAY   2 

    0x94b70bd2,// 6 PAY   3 

    0x3b88602d,// 7 PAY   4 

    0xbd285e81,// 8 PAY   5 

    0x69f3e9bd,// 9 PAY   6 

    0x9bdf9754,// 10 PAY   7 

    0xd2b9db87,// 11 PAY   8 

    0xa2860402,// 12 PAY   9 

    0x17d16007,// 13 PAY  10 

    0xe43b1d1e,// 14 PAY  11 

    0x59a031ad,// 15 PAY  12 

    0x82d9b4cc,// 16 PAY  13 

    0x9780a35c,// 17 PAY  14 

    0xa08f245f,// 18 PAY  15 

    0xbac0d4f5,// 19 PAY  16 

    0xa7e60765,// 20 PAY  17 

    0x0e7023d9,// 21 PAY  18 

    0x59ae43e5,// 22 PAY  19 

    0x14dc53d9,// 23 PAY  20 

    0xb803ea3e,// 24 PAY  21 

    0x8260d4bc,// 25 PAY  22 

    0xcb996224,// 26 PAY  23 

    0x89eb6c34,// 27 PAY  24 

    0x58997201,// 28 PAY  25 

    0x594724d6,// 29 PAY  26 

    0x051dc8b1,// 30 PAY  27 

    0xd46ebad7,// 31 PAY  28 

    0x12e42241,// 32 PAY  29 

    0xc55948f6,// 33 PAY  30 

    0x3325265d,// 34 PAY  31 

    0x36961964,// 35 PAY  32 

    0xdd756f37,// 36 PAY  33 

    0x2d986d2b,// 37 PAY  34 

    0xd9c09370,// 38 PAY  35 

    0x62ca018c,// 39 PAY  36 

    0xf098a446,// 40 PAY  37 

    0x38b59c0a,// 41 PAY  38 

    0xc7b99249,// 42 PAY  39 

    0x83b0c8b5,// 43 PAY  40 

    0x55a4cc5f,// 44 PAY  41 

    0x6e96cb92,// 45 PAY  42 

    0x9aa9f9f9,// 46 PAY  43 

    0x5c8daf86,// 47 PAY  44 

    0xcf1a7c20,// 48 PAY  45 

    0xc8b1b60d,// 49 PAY  46 

    0x0c2b554c,// 50 PAY  47 

    0x8410e312,// 51 PAY  48 

    0x308ba223,// 52 PAY  49 

    0x32cb46a0,// 53 PAY  50 

    0x299dbedc,// 54 PAY  51 

    0xfcadb498,// 55 PAY  52 

    0x8ca4e92b,// 56 PAY  53 

    0xd7dee074,// 57 PAY  54 

    0x36d17fff,// 58 PAY  55 

    0xcbe2ef57,// 59 PAY  56 

    0x3017080d,// 60 PAY  57 

    0x119ef9ff,// 61 PAY  58 

    0x2299459a,// 62 PAY  59 

    0x199d48c2,// 63 PAY  60 

    0x0eb7257d,// 64 PAY  61 

    0x77a8baf3,// 65 PAY  62 

    0xf9e762f1,// 66 PAY  63 

    0xba5affb5,// 67 PAY  64 

    0x2354a2b3,// 68 PAY  65 

    0x00ee7e71,// 69 PAY  66 

    0xd92857a6,// 70 PAY  67 

    0x53fc7e6c,// 71 PAY  68 

    0xdb118f31,// 72 PAY  69 

    0xc6e88ea5,// 73 PAY  70 

    0x49a46a37,// 74 PAY  71 

    0x09d65013,// 75 PAY  72 

    0x9ec41a7d,// 76 PAY  73 

    0x2bb83801,// 77 PAY  74 

    0x1b5ce2d8,// 78 PAY  75 

    0x41996c8b,// 79 PAY  76 

    0x92d91835,// 80 PAY  77 

    0x0fe74866,// 81 PAY  78 

    0xd1276cf8,// 82 PAY  79 

    0x900590eb,// 83 PAY  80 

    0x8da2f86e,// 84 PAY  81 

    0x69d45ef1,// 85 PAY  82 

    0xe04c2017,// 86 PAY  83 

    0xdb8189c2,// 87 PAY  84 

    0x5ba0bfbe,// 88 PAY  85 

    0x7e29548b,// 89 PAY  86 

    0x55c6eb23,// 90 PAY  87 

    0x28baaa31,// 91 PAY  88 

    0xd7ca024a,// 92 PAY  89 

    0x53475973,// 93 PAY  90 

    0xad06c8fe,// 94 PAY  91 

    0x74314507,// 95 PAY  92 

    0x565c7098,// 96 PAY  93 

    0xf23663d5,// 97 PAY  94 

    0x2ec1522d,// 98 PAY  95 

    0x50ad5217,// 99 PAY  96 

    0xbac8755a,// 100 PAY  97 

    0xfa368c76,// 101 PAY  98 

    0x222e87a5,// 102 PAY  99 

    0x1bfa97cb,// 103 PAY 100 

    0xb91986a6,// 104 PAY 101 

    0x963cd0a8,// 105 PAY 102 

    0x0200d3ac,// 106 PAY 103 

    0x23b258a1,// 107 PAY 104 

    0x97e29b1f,// 108 PAY 105 

    0x2ff73a01,// 109 PAY 106 

    0x5d7611c9,// 110 PAY 107 

    0xe1a29d69,// 111 PAY 108 

    0xb3896a57,// 112 PAY 109 

    0xac3eaee2,// 113 PAY 110 

    0x71f0872f,// 114 PAY 111 

    0x0268da8a,// 115 PAY 112 

    0xf50149de,// 116 PAY 113 

    0x5e7a4aa2,// 117 PAY 114 

    0x722fcf4c,// 118 PAY 115 

    0xc5a32e60,// 119 PAY 116 

    0x34f939e5,// 120 PAY 117 

    0x705ae970,// 121 PAY 118 

    0x9a372907,// 122 PAY 119 

    0x29e27d61,// 123 PAY 120 

    0x54573dbb,// 124 PAY 121 

    0x0e7fe132,// 125 PAY 122 

    0xd0469e63,// 126 PAY 123 

    0x180cf2b0,// 127 PAY 124 

    0xf9a72977,// 128 PAY 125 

    0xd27d37d0,// 129 PAY 126 

    0xd4ac9d1a,// 130 PAY 127 

    0x77fbbaab,// 131 PAY 128 

    0xdb00e757,// 132 PAY 129 

    0x40ac1a0f,// 133 PAY 130 

    0x9da9588f,// 134 PAY 131 

    0xa8455ca3,// 135 PAY 132 

    0x538b7fe3,// 136 PAY 133 

    0x4e16ed52,// 137 PAY 134 

    0x86e7b7ed,// 138 PAY 135 

    0xc7ab0d6e,// 139 PAY 136 

    0xc3393b4d,// 140 PAY 137 

    0x33c0f2ed,// 141 PAY 138 

    0x494459f6,// 142 PAY 139 

    0x231c41e7,// 143 PAY 140 

    0xe449232b,// 144 PAY 141 

    0xc02f774f,// 145 PAY 142 

    0xc4c57af5,// 146 PAY 143 

    0xda5a6408,// 147 PAY 144 

    0x0f979424,// 148 PAY 145 

    0x9d7659b7,// 149 PAY 146 

    0x9e8edfa1,// 150 PAY 147 

    0x18e3a303,// 151 PAY 148 

    0x9a5e71a9,// 152 PAY 149 

    0x7935e6b4,// 153 PAY 150 

    0x1db9db8b,// 154 PAY 151 

    0x76002205,// 155 PAY 152 

    0x56601a3a,// 156 PAY 153 

    0x3e406b2f,// 157 PAY 154 

    0xcb9ba5f5,// 158 PAY 155 

    0xe50d0b97,// 159 PAY 156 

    0xdabc4026,// 160 PAY 157 

    0x9ca6bc5c,// 161 PAY 158 

    0x691f84b6,// 162 PAY 159 

    0x79f67535,// 163 PAY 160 

    0x2ea16c0b,// 164 PAY 161 

    0x279d2b06,// 165 PAY 162 

    0xbf3a801c,// 166 PAY 163 

    0x45bf0f16,// 167 PAY 164 

    0xb10e4de8,// 168 PAY 165 

    0xbc8f63df,// 169 PAY 166 

    0x05bf2ba1,// 170 PAY 167 

    0xbdda616a,// 171 PAY 168 

    0xec1cf311,// 172 PAY 169 

    0x04cb59af,// 173 PAY 170 

    0xa848baaf,// 174 PAY 171 

    0xbe4e1ca3,// 175 PAY 172 

    0x58398a48,// 176 PAY 173 

    0x0fdd6bd6,// 177 PAY 174 

    0x14d1c818,// 178 PAY 175 

    0xd7a8765f,// 179 PAY 176 

    0x23aaf16b,// 180 PAY 177 

    0x1419240a,// 181 PAY 178 

    0xa0ea66c5,// 182 PAY 179 

    0x91c2f33c,// 183 PAY 180 

    0xb01aae80,// 184 PAY 181 

    0x9b39e47c,// 185 PAY 182 

    0x36cec22f,// 186 PAY 183 

    0x8664581c,// 187 PAY 184 

    0xb3ea1a58,// 188 PAY 185 

    0x44b91cfb,// 189 PAY 186 

    0x8ea1005c,// 190 PAY 187 

    0x4c4e3662,// 191 PAY 188 

    0x1c063a8c,// 192 PAY 189 

    0xdd9a4888,// 193 PAY 190 

    0x205c4dbd,// 194 PAY 191 

    0x85dedfc6,// 195 PAY 192 

    0xf7ad661a,// 196 PAY 193 

    0x33e21ddd,// 197 PAY 194 

    0x97a041aa,// 198 PAY 195 

    0x067ac870,// 199 PAY 196 

    0x3f4bda2c,// 200 PAY 197 

    0xc515e98f,// 201 PAY 198 

    0x1396a715,// 202 PAY 199 

    0xbc3ac6e9,// 203 PAY 200 

    0xdd7ea8a8,// 204 PAY 201 

    0xd2d7c14b,// 205 PAY 202 

    0x90037ccf,// 206 PAY 203 

    0xf06688ba,// 207 PAY 204 

    0x51025a28,// 208 PAY 205 

    0xf24c2cb6,// 209 PAY 206 

    0xc1461b90,// 210 PAY 207 

    0xdd949721,// 211 PAY 208 

    0xa497a089,// 212 PAY 209 

    0x94f48e47,// 213 PAY 210 

    0x9e21527a,// 214 PAY 211 

    0xd6e91437,// 215 PAY 212 

    0xa4f89f16,// 216 PAY 213 

    0x1fff154f,// 217 PAY 214 

    0xebdd0c7a,// 218 PAY 215 

    0x921b2af8,// 219 PAY 216 

    0x1906f19c,// 220 PAY 217 

    0xe8318ee3,// 221 PAY 218 

    0xe93aa7f6,// 222 PAY 219 

    0xf4fe48d5,// 223 PAY 220 

    0x0f9b7b95,// 224 PAY 221 

    0x0f6b9859,// 225 PAY 222 

    0xf83167f3,// 226 PAY 223 

    0x3a362e2a,// 227 PAY 224 

    0xd720cabd,// 228 PAY 225 

    0xd3e8052a,// 229 PAY 226 

    0x12362c33,// 230 PAY 227 

    0x2797c1ca,// 231 PAY 228 

    0x0323a598,// 232 PAY 229 

    0x69a22c37,// 233 PAY 230 

    0x949fce74,// 234 PAY 231 

    0x7012f8e2,// 235 PAY 232 

    0x7524547c,// 236 PAY 233 

    0xa15ab73d,// 237 PAY 234 

    0xd80786db,// 238 PAY 235 

    0x4e701f03,// 239 PAY 236 

    0x0495ebf6,// 240 PAY 237 

    0xe7245316,// 241 PAY 238 

    0x619c7f13,// 242 PAY 239 

    0x82549745,// 243 PAY 240 

    0x23868c3e,// 244 PAY 241 

    0x9896bea4,// 245 PAY 242 

    0x38be4079,// 246 PAY 243 

    0x2fdde9a1,// 247 PAY 244 

    0x4f1635f8,// 248 PAY 245 

    0x108df2c9,// 249 PAY 246 

    0xa7cc977c,// 250 PAY 247 

    0x7ef94b7e,// 251 PAY 248 

    0xe154a60c,// 252 PAY 249 

    0xa1a1b518,// 253 PAY 250 

    0x29515165,// 254 PAY 251 

    0x3ee6b087,// 255 PAY 252 

    0xa96946f6,// 256 PAY 253 

    0xee4984ad,// 257 PAY 254 

    0x570cb07c,// 258 PAY 255 

    0xd3d0f9dd,// 259 PAY 256 

    0x2feb8a64,// 260 PAY 257 

    0x21ef9e54,// 261 PAY 258 

    0x67c010ec,// 262 PAY 259 

    0x8c0ceee0,// 263 PAY 260 

    0xa8b0f22e,// 264 PAY 261 

    0x8e058da7,// 265 PAY 262 

    0x0b342d97,// 266 PAY 263 

    0x4a3dbee5,// 267 PAY 264 

    0x34dc77a2,// 268 PAY 265 

    0x95603532,// 269 PAY 266 

    0xd411b107,// 270 PAY 267 

    0x92edff7e,// 271 PAY 268 

    0x1d215716,// 272 PAY 269 

    0x7ede4b82,// 273 PAY 270 

    0xc12579e5,// 274 PAY 271 

    0xf0e9b7e5,// 275 PAY 272 

    0x67984471,// 276 PAY 273 

    0x561f0b96,// 277 PAY 274 

    0xb0cf6092,// 278 PAY 275 

    0x2cc7343d,// 279 PAY 276 

    0xf9d9729f,// 280 PAY 277 

    0x2523fec0,// 281 PAY 278 

    0xdc78dbb6,// 282 PAY 279 

    0x87f5f0f4,// 283 PAY 280 

    0x95194e1e,// 284 PAY 281 

    0x14d156e8,// 285 PAY 282 

    0x53253f2e,// 286 PAY 283 

    0x30bae375,// 287 PAY 284 

    0xa7d64248,// 288 PAY 285 

    0x56bf404e,// 289 PAY 286 

    0x828852c0,// 290 PAY 287 

    0x0c2bb59f,// 291 PAY 288 

    0x013d87d3,// 292 PAY 289 

    0x86b89a33,// 293 PAY 290 

    0x1e3f279d,// 294 PAY 291 

    0x83da5c4c,// 295 PAY 292 

    0x3a205958,// 296 PAY 293 

    0x381c40f4,// 297 PAY 294 

    0x92519897,// 298 PAY 295 

    0x757d53a9,// 299 PAY 296 

    0x8f19054e,// 300 PAY 297 

    0x49338129,// 301 PAY 298 

    0x813f00df,// 302 PAY 299 

    0x3477ea94,// 303 PAY 300 

    0xbc7e15ce,// 304 PAY 301 

    0x0866c27f,// 305 PAY 302 

    0x9392c7b8,// 306 PAY 303 

    0x65ac536f,// 307 PAY 304 

    0xc18f5c2a,// 308 PAY 305 

    0x00af5b06,// 309 PAY 306 

    0x4d91d0ba,// 310 PAY 307 

    0x474c0bd1,// 311 PAY 308 

    0x852ae63c,// 312 PAY 309 

    0xae2e3b9a,// 313 PAY 310 

    0x207571df,// 314 PAY 311 

    0xafa28488,// 315 PAY 312 

    0xbfd2fd3c,// 316 PAY 313 

    0xfda6d462,// 317 PAY 314 

    0xbb7f2da2,// 318 PAY 315 

    0xef825697,// 319 PAY 316 

    0x25e4ceb6,// 320 PAY 317 

    0x25ebb385,// 321 PAY 318 

    0x5efbefbd,// 322 PAY 319 

    0x57465be8,// 323 PAY 320 

    0xc62eccbb,// 324 PAY 321 

    0xd8f4740e,// 325 PAY 322 

    0xde70b080,// 326 PAY 323 

    0x37aa34bf,// 327 PAY 324 

    0xe763f7d8,// 328 PAY 325 

    0x2c60419c,// 329 PAY 326 

    0xb77aaf6b,// 330 PAY 327 

    0x07030f71,// 331 PAY 328 

    0xa9e0c1fc,// 332 PAY 329 

    0x16af5290,// 333 PAY 330 

    0xfad0e040,// 334 PAY 331 

    0x4d7117fc,// 335 PAY 332 

    0xad76ed0e,// 336 PAY 333 

    0x52ac3ca3,// 337 PAY 334 

    0x9cfa711a,// 338 PAY 335 

    0x06ba1d61,// 339 PAY 336 

    0x171d9438,// 340 PAY 337 

    0x1136e9e4,// 341 PAY 338 

    0xc8bf672a,// 342 PAY 339 

    0xc84ddbf0,// 343 PAY 340 

    0x50aa7dea,// 344 PAY 341 

    0xe7fab572,// 345 PAY 342 

    0x4488688c,// 346 PAY 343 

    0x566ade33,// 347 PAY 344 

    0x04b6b263,// 348 PAY 345 

    0xf8bbf3d7,// 349 PAY 346 

    0x294ebfed,// 350 PAY 347 

    0x49694676,// 351 PAY 348 

    0xe17145dd,// 352 PAY 349 

    0x795993cb,// 353 PAY 350 

    0x1093867c,// 354 PAY 351 

    0xd5471a81,// 355 PAY 352 

    0xf0705c8e,// 356 PAY 353 

    0x999e2a9e,// 357 PAY 354 

    0x94730033,// 358 PAY 355 

    0xfb580dd8,// 359 PAY 356 

    0x1fe3bf69,// 360 PAY 357 

    0x59841b14,// 361 PAY 358 

    0xc428db12,// 362 PAY 359 

    0xfca66c24,// 363 PAY 360 

    0x9250c07a,// 364 PAY 361 

    0x63746feb,// 365 PAY 362 

    0x90817ea6,// 366 PAY 363 

    0xfd5a283f,// 367 PAY 364 

    0x95411166,// 368 PAY 365 

    0x50ead699,// 369 PAY 366 

    0x4122cb6e,// 370 PAY 367 

    0x42b5993a,// 371 PAY 368 

    0x1e6551e9,// 372 PAY 369 

    0xcba0a303,// 373 PAY 370 

    0x1b000000,// 374 PAY 371 

/// HASH is  8 bytes 

    0x50ead699,// 375 HSH   1 

    0x4122cb6e,// 376 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 111 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x01f04d6f // 377 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt75_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 219 words. 

/// BDA size     is 870 (0x366) 

/// BDA id       is 0xec5d 

    0x0366ec5d,// 3 BDA   1 

/// PAY Generic Data size   : 870 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x29a42481,// 4 PAY   1 

    0x4d27ff2d,// 5 PAY   2 

    0xdce56b9c,// 6 PAY   3 

    0x5b6e42ce,// 7 PAY   4 

    0x59309030,// 8 PAY   5 

    0x48a5d4ba,// 9 PAY   6 

    0xc7d670a0,// 10 PAY   7 

    0xb83d96dc,// 11 PAY   8 

    0x282878d1,// 12 PAY   9 

    0x733c0f4a,// 13 PAY  10 

    0xf923e892,// 14 PAY  11 

    0xaa0ef708,// 15 PAY  12 

    0xa57c7787,// 16 PAY  13 

    0xb57936e4,// 17 PAY  14 

    0x218da0aa,// 18 PAY  15 

    0x2b156a7f,// 19 PAY  16 

    0xd9ef9a97,// 20 PAY  17 

    0xb9ac02a9,// 21 PAY  18 

    0xb47cbbbb,// 22 PAY  19 

    0xac6b5fe8,// 23 PAY  20 

    0xff30d560,// 24 PAY  21 

    0xf6bf8b66,// 25 PAY  22 

    0xd100e96a,// 26 PAY  23 

    0x19c05107,// 27 PAY  24 

    0x8bb5ea26,// 28 PAY  25 

    0x51b00bd2,// 29 PAY  26 

    0xc58557e4,// 30 PAY  27 

    0xe379239c,// 31 PAY  28 

    0x1167157d,// 32 PAY  29 

    0x14d517b0,// 33 PAY  30 

    0x8f0700fd,// 34 PAY  31 

    0x8b2d9e48,// 35 PAY  32 

    0x30797c37,// 36 PAY  33 

    0x3aa817f4,// 37 PAY  34 

    0x47c14a0e,// 38 PAY  35 

    0xe0216858,// 39 PAY  36 

    0xa7dd5e73,// 40 PAY  37 

    0x874fe438,// 41 PAY  38 

    0xb1129080,// 42 PAY  39 

    0x980dab0b,// 43 PAY  40 

    0x1fac6909,// 44 PAY  41 

    0x4b5e5f12,// 45 PAY  42 

    0x42739e78,// 46 PAY  43 

    0x029f6d63,// 47 PAY  44 

    0x4055a2b2,// 48 PAY  45 

    0x1bdf9297,// 49 PAY  46 

    0xa65fc2dc,// 50 PAY  47 

    0xbf2bce4c,// 51 PAY  48 

    0x57c68ab2,// 52 PAY  49 

    0x796f3f19,// 53 PAY  50 

    0xc5ea39d7,// 54 PAY  51 

    0xf4d16519,// 55 PAY  52 

    0x5c721db2,// 56 PAY  53 

    0x63fc2a3e,// 57 PAY  54 

    0x856e31e5,// 58 PAY  55 

    0xf23b95eb,// 59 PAY  56 

    0x6adb0942,// 60 PAY  57 

    0xe7a1c534,// 61 PAY  58 

    0xd88ce8c3,// 62 PAY  59 

    0x4a8835cd,// 63 PAY  60 

    0x649757a4,// 64 PAY  61 

    0x09359d39,// 65 PAY  62 

    0xe3f3fa1f,// 66 PAY  63 

    0xb6db107d,// 67 PAY  64 

    0xc17ed348,// 68 PAY  65 

    0xcb50442e,// 69 PAY  66 

    0x25e8d736,// 70 PAY  67 

    0xe9acd502,// 71 PAY  68 

    0x0fdf8cfa,// 72 PAY  69 

    0x6e8f30da,// 73 PAY  70 

    0x58187263,// 74 PAY  71 

    0x09debf14,// 75 PAY  72 

    0x593f2def,// 76 PAY  73 

    0xe2d02e39,// 77 PAY  74 

    0x9f9d6d7e,// 78 PAY  75 

    0x6650491e,// 79 PAY  76 

    0x2635b79d,// 80 PAY  77 

    0xab7e90eb,// 81 PAY  78 

    0x8e119e1d,// 82 PAY  79 

    0x60c070c5,// 83 PAY  80 

    0x454d54f2,// 84 PAY  81 

    0x369000fe,// 85 PAY  82 

    0x4b96fdda,// 86 PAY  83 

    0xf1125a51,// 87 PAY  84 

    0x30dcf55b,// 88 PAY  85 

    0xb9a207c4,// 89 PAY  86 

    0xc8c9e3d8,// 90 PAY  87 

    0xfcf5f07e,// 91 PAY  88 

    0x5cbdc94b,// 92 PAY  89 

    0x4e5e51f4,// 93 PAY  90 

    0x82b72e84,// 94 PAY  91 

    0x31ec8c8f,// 95 PAY  92 

    0xa098be60,// 96 PAY  93 

    0x155cde0b,// 97 PAY  94 

    0x7bc9d511,// 98 PAY  95 

    0xdfffd7c3,// 99 PAY  96 

    0xafa3cf61,// 100 PAY  97 

    0x03652780,// 101 PAY  98 

    0x8b296bf3,// 102 PAY  99 

    0x26844df6,// 103 PAY 100 

    0x90f73ce9,// 104 PAY 101 

    0x38ad34e7,// 105 PAY 102 

    0xe8fe61f4,// 106 PAY 103 

    0x896f79cb,// 107 PAY 104 

    0x9fc9e4a4,// 108 PAY 105 

    0x32205cad,// 109 PAY 106 

    0x8f010d42,// 110 PAY 107 

    0x04c4a0e7,// 111 PAY 108 

    0x40e35df3,// 112 PAY 109 

    0x6770cfaa,// 113 PAY 110 

    0x7284998a,// 114 PAY 111 

    0x32aeec8a,// 115 PAY 112 

    0xd8aaad02,// 116 PAY 113 

    0x95c5cc85,// 117 PAY 114 

    0xf1ab8ef0,// 118 PAY 115 

    0xc9bc788b,// 119 PAY 116 

    0x00ca02b4,// 120 PAY 117 

    0x19e631f6,// 121 PAY 118 

    0x808039ed,// 122 PAY 119 

    0x58597b2d,// 123 PAY 120 

    0xb9e63dec,// 124 PAY 121 

    0xe79ba792,// 125 PAY 122 

    0x193a98e4,// 126 PAY 123 

    0x0185e734,// 127 PAY 124 

    0xffb1b515,// 128 PAY 125 

    0x1117e0d8,// 129 PAY 126 

    0x4119eefe,// 130 PAY 127 

    0xe000ec93,// 131 PAY 128 

    0xacddf1f1,// 132 PAY 129 

    0x6cdb5a68,// 133 PAY 130 

    0x318954e4,// 134 PAY 131 

    0x27cb669d,// 135 PAY 132 

    0x7a51aa2b,// 136 PAY 133 

    0x9705d3df,// 137 PAY 134 

    0x2694cd72,// 138 PAY 135 

    0xa0fcc600,// 139 PAY 136 

    0xccd94411,// 140 PAY 137 

    0xdff2e8e0,// 141 PAY 138 

    0xebc87e88,// 142 PAY 139 

    0xe415feac,// 143 PAY 140 

    0xf9164741,// 144 PAY 141 

    0xa80090f3,// 145 PAY 142 

    0x82089177,// 146 PAY 143 

    0x1327e04e,// 147 PAY 144 

    0x84910e1c,// 148 PAY 145 

    0x7a93344c,// 149 PAY 146 

    0x92919e9b,// 150 PAY 147 

    0x6a4fe762,// 151 PAY 148 

    0x0d501787,// 152 PAY 149 

    0xf7f00a73,// 153 PAY 150 

    0x6e270bc8,// 154 PAY 151 

    0xe65f869e,// 155 PAY 152 

    0x4f2ab923,// 156 PAY 153 

    0x33c9322a,// 157 PAY 154 

    0xa4d96969,// 158 PAY 155 

    0x8dad229b,// 159 PAY 156 

    0xe6baaf19,// 160 PAY 157 

    0xc3590822,// 161 PAY 158 

    0x641ecc02,// 162 PAY 159 

    0x89f9cc60,// 163 PAY 160 

    0x31ef92ae,// 164 PAY 161 

    0xebc28077,// 165 PAY 162 

    0xe8c36d02,// 166 PAY 163 

    0x02af50a4,// 167 PAY 164 

    0x02379e75,// 168 PAY 165 

    0xec882d29,// 169 PAY 166 

    0xedfc20d6,// 170 PAY 167 

    0x193a46b6,// 171 PAY 168 

    0xe152f48f,// 172 PAY 169 

    0xb614b718,// 173 PAY 170 

    0xda96321f,// 174 PAY 171 

    0xff2c0631,// 175 PAY 172 

    0x2dca2849,// 176 PAY 173 

    0x45804a72,// 177 PAY 174 

    0xe939a65a,// 178 PAY 175 

    0xb225ca09,// 179 PAY 176 

    0x6c906495,// 180 PAY 177 

    0x4055e408,// 181 PAY 178 

    0xbde3b7a4,// 182 PAY 179 

    0xb33d552a,// 183 PAY 180 

    0xaa56404f,// 184 PAY 181 

    0x62c902b8,// 185 PAY 182 

    0x75841cc2,// 186 PAY 183 

    0x91f118d9,// 187 PAY 184 

    0xcd6ac39b,// 188 PAY 185 

    0xdc6bfaa1,// 189 PAY 186 

    0xa7e3a5f1,// 190 PAY 187 

    0x67da098f,// 191 PAY 188 

    0x675e03fc,// 192 PAY 189 

    0x132567c8,// 193 PAY 190 

    0x9bb4c358,// 194 PAY 191 

    0xd6256d53,// 195 PAY 192 

    0xf4c279fb,// 196 PAY 193 

    0xe6f8814c,// 197 PAY 194 

    0xe051f52e,// 198 PAY 195 

    0xb3a2d05c,// 199 PAY 196 

    0x20b625f3,// 200 PAY 197 

    0xdd497ed5,// 201 PAY 198 

    0xfe7de7bd,// 202 PAY 199 

    0xbaab08a2,// 203 PAY 200 

    0x09d8d62b,// 204 PAY 201 

    0x09cdbacb,// 205 PAY 202 

    0x1c948014,// 206 PAY 203 

    0x7eabce81,// 207 PAY 204 

    0xbdd34667,// 208 PAY 205 

    0x6d128835,// 209 PAY 206 

    0x34933532,// 210 PAY 207 

    0xec2f07df,// 211 PAY 208 

    0x397db6f8,// 212 PAY 209 

    0x26214287,// 213 PAY 210 

    0xe84ca959,// 214 PAY 211 

    0x85655109,// 215 PAY 212 

    0xaf926b40,// 216 PAY 213 

    0x0e45acc4,// 217 PAY 214 

    0xb7fe7dfe,// 218 PAY 215 

    0xd360bda9,// 219 PAY 216 

    0x225efab7,// 220 PAY 217 

    0xe4320000,// 221 PAY 218 

/// STA is 1 words. 

/// STA num_pkts       : 147 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x96 

    0x02d09693 // 222 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt76_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 313 words. 

/// BDA size     is 1247 (0x4df) 

/// BDA id       is 0xd76a 

    0x04dfd76a,// 3 BDA   1 

/// PAY Generic Data size   : 1247 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x8e3d3439,// 4 PAY   1 

    0x883c6f0c,// 5 PAY   2 

    0x6ca6560a,// 6 PAY   3 

    0x3cc909e3,// 7 PAY   4 

    0xbc8fe09e,// 8 PAY   5 

    0x90a8bf78,// 9 PAY   6 

    0x699dcc08,// 10 PAY   7 

    0x5612cd75,// 11 PAY   8 

    0xc6b12301,// 12 PAY   9 

    0x24bf350e,// 13 PAY  10 

    0x94f33c06,// 14 PAY  11 

    0xff0e992b,// 15 PAY  12 

    0xbe68026c,// 16 PAY  13 

    0xfabeb00c,// 17 PAY  14 

    0x00d9d5b4,// 18 PAY  15 

    0x46251495,// 19 PAY  16 

    0xaee21ebe,// 20 PAY  17 

    0x65953f1c,// 21 PAY  18 

    0x051e8b80,// 22 PAY  19 

    0x1897fea2,// 23 PAY  20 

    0x602899d8,// 24 PAY  21 

    0xab51b54a,// 25 PAY  22 

    0x3f7156a0,// 26 PAY  23 

    0xd33201ec,// 27 PAY  24 

    0xaca6fba6,// 28 PAY  25 

    0x2d7cc4da,// 29 PAY  26 

    0x2c11b339,// 30 PAY  27 

    0x373a9637,// 31 PAY  28 

    0xef946e21,// 32 PAY  29 

    0xdb3408c3,// 33 PAY  30 

    0xc3f10881,// 34 PAY  31 

    0xd705315d,// 35 PAY  32 

    0x62bc1359,// 36 PAY  33 

    0x8c908b54,// 37 PAY  34 

    0xca15d65e,// 38 PAY  35 

    0xf1de5abf,// 39 PAY  36 

    0xad2d2fb8,// 40 PAY  37 

    0x5e81c7b0,// 41 PAY  38 

    0xe36874a7,// 42 PAY  39 

    0xcca49354,// 43 PAY  40 

    0x869c1232,// 44 PAY  41 

    0xb3cc1d6b,// 45 PAY  42 

    0x7d68b9e5,// 46 PAY  43 

    0x30861a88,// 47 PAY  44 

    0x1009d13a,// 48 PAY  45 

    0x71a58b29,// 49 PAY  46 

    0x9e86333f,// 50 PAY  47 

    0x53d2fd22,// 51 PAY  48 

    0x4ee61054,// 52 PAY  49 

    0xa4d05ebe,// 53 PAY  50 

    0x14449e84,// 54 PAY  51 

    0x756a47d2,// 55 PAY  52 

    0x3ee5970a,// 56 PAY  53 

    0xd201b36c,// 57 PAY  54 

    0x4f11bdef,// 58 PAY  55 

    0xb1191293,// 59 PAY  56 

    0xbeda3842,// 60 PAY  57 

    0x030f1b0b,// 61 PAY  58 

    0x11bd7650,// 62 PAY  59 

    0x375203f5,// 63 PAY  60 

    0x504fa5ea,// 64 PAY  61 

    0xee5c6649,// 65 PAY  62 

    0x3bb7ff3a,// 66 PAY  63 

    0x15e554d9,// 67 PAY  64 

    0x3522aefd,// 68 PAY  65 

    0x6e13b2c7,// 69 PAY  66 

    0x114be5ab,// 70 PAY  67 

    0x40ace4fc,// 71 PAY  68 

    0xa1a2723b,// 72 PAY  69 

    0x88b3da3a,// 73 PAY  70 

    0x8ea39e52,// 74 PAY  71 

    0x5f5d8290,// 75 PAY  72 

    0x26b8969c,// 76 PAY  73 

    0x1faf59f4,// 77 PAY  74 

    0x2ddf1acc,// 78 PAY  75 

    0x5a92d801,// 79 PAY  76 

    0x9c38897b,// 80 PAY  77 

    0x749a2521,// 81 PAY  78 

    0x4dbb4fc6,// 82 PAY  79 

    0xb04b5bfd,// 83 PAY  80 

    0xd9d7d590,// 84 PAY  81 

    0xea2e48bb,// 85 PAY  82 

    0x6885c108,// 86 PAY  83 

    0x65294ced,// 87 PAY  84 

    0x6c8be1b7,// 88 PAY  85 

    0x3007ac7e,// 89 PAY  86 

    0xacc53174,// 90 PAY  87 

    0xc44e2672,// 91 PAY  88 

    0x79aa7101,// 92 PAY  89 

    0x5e2b9a61,// 93 PAY  90 

    0x157873db,// 94 PAY  91 

    0xb5f9b1e0,// 95 PAY  92 

    0xebd9dfa9,// 96 PAY  93 

    0x74fd9937,// 97 PAY  94 

    0x0a1b2c33,// 98 PAY  95 

    0xfc932084,// 99 PAY  96 

    0x4c075bdf,// 100 PAY  97 

    0xc3558a20,// 101 PAY  98 

    0xcda700cd,// 102 PAY  99 

    0xf8a0f3ec,// 103 PAY 100 

    0xf1a6b214,// 104 PAY 101 

    0x722fbc6c,// 105 PAY 102 

    0xdbf37a00,// 106 PAY 103 

    0x09c2434e,// 107 PAY 104 

    0xc714f3cc,// 108 PAY 105 

    0xfa0fb82d,// 109 PAY 106 

    0xc7b7d619,// 110 PAY 107 

    0x33037434,// 111 PAY 108 

    0x92fc8467,// 112 PAY 109 

    0x1a342875,// 113 PAY 110 

    0xe85aafe7,// 114 PAY 111 

    0x398c50e6,// 115 PAY 112 

    0xd3024423,// 116 PAY 113 

    0x7b01b4eb,// 117 PAY 114 

    0x96b9a6d0,// 118 PAY 115 

    0x19782474,// 119 PAY 116 

    0xc3b32ffe,// 120 PAY 117 

    0x0e3883db,// 121 PAY 118 

    0x3f38026d,// 122 PAY 119 

    0xe4b8703f,// 123 PAY 120 

    0x750fe484,// 124 PAY 121 

    0xb550fd68,// 125 PAY 122 

    0x305b7778,// 126 PAY 123 

    0x0d5ffaf1,// 127 PAY 124 

    0x31f95267,// 128 PAY 125 

    0xcd837f05,// 129 PAY 126 

    0x8dd49460,// 130 PAY 127 

    0x2f6088b9,// 131 PAY 128 

    0x168c0702,// 132 PAY 129 

    0x9e7ce4d3,// 133 PAY 130 

    0x401cfc87,// 134 PAY 131 

    0x3d371a6a,// 135 PAY 132 

    0x06c6fc32,// 136 PAY 133 

    0x0a024201,// 137 PAY 134 

    0x39dde6aa,// 138 PAY 135 

    0xec658750,// 139 PAY 136 

    0x5f67234c,// 140 PAY 137 

    0x7f4a1811,// 141 PAY 138 

    0x1abeb7d7,// 142 PAY 139 

    0x5ec8f571,// 143 PAY 140 

    0x5a1d13d7,// 144 PAY 141 

    0xdce1c05c,// 145 PAY 142 

    0xf545212e,// 146 PAY 143 

    0x48327e24,// 147 PAY 144 

    0x7031fc78,// 148 PAY 145 

    0x812804f9,// 149 PAY 146 

    0xb128b209,// 150 PAY 147 

    0x8eca133e,// 151 PAY 148 

    0x8e391398,// 152 PAY 149 

    0x0642ae4a,// 153 PAY 150 

    0x2b03acfd,// 154 PAY 151 

    0xa3c88df9,// 155 PAY 152 

    0x2c0746ef,// 156 PAY 153 

    0x107aff09,// 157 PAY 154 

    0xcf2a31b3,// 158 PAY 155 

    0xd8901bff,// 159 PAY 156 

    0x3ae098f8,// 160 PAY 157 

    0x1b5d5fe9,// 161 PAY 158 

    0xad1e6b76,// 162 PAY 159 

    0xf8f5241e,// 163 PAY 160 

    0xf95318d2,// 164 PAY 161 

    0xefcd6568,// 165 PAY 162 

    0xcdac554c,// 166 PAY 163 

    0xffce896e,// 167 PAY 164 

    0x9da1a8af,// 168 PAY 165 

    0x9529482b,// 169 PAY 166 

    0xbb00810e,// 170 PAY 167 

    0x569c2792,// 171 PAY 168 

    0x9e59d3a1,// 172 PAY 169 

    0x6a9f26bf,// 173 PAY 170 

    0xfebfac20,// 174 PAY 171 

    0x4502417d,// 175 PAY 172 

    0xbaf8de61,// 176 PAY 173 

    0xd8009212,// 177 PAY 174 

    0x5885bca8,// 178 PAY 175 

    0xc9647a60,// 179 PAY 176 

    0xb4493643,// 180 PAY 177 

    0x243d5ecc,// 181 PAY 178 

    0xdebc3d76,// 182 PAY 179 

    0x6f24e230,// 183 PAY 180 

    0x63bd8933,// 184 PAY 181 

    0x0b946ec6,// 185 PAY 182 

    0x83ef235e,// 186 PAY 183 

    0xaede92cc,// 187 PAY 184 

    0x7510d96c,// 188 PAY 185 

    0x65ff2e16,// 189 PAY 186 

    0xd36fdfd5,// 190 PAY 187 

    0xcdc3b1c5,// 191 PAY 188 

    0xda2f3bd6,// 192 PAY 189 

    0x5595aacf,// 193 PAY 190 

    0x4a2fed41,// 194 PAY 191 

    0x4d3768a5,// 195 PAY 192 

    0xa55eaacb,// 196 PAY 193 

    0x4ec9f68a,// 197 PAY 194 

    0x19a2368e,// 198 PAY 195 

    0xc8801f43,// 199 PAY 196 

    0x0927de40,// 200 PAY 197 

    0xc4ab8bd8,// 201 PAY 198 

    0x60dfe6a8,// 202 PAY 199 

    0x0a9ba76d,// 203 PAY 200 

    0x3715475d,// 204 PAY 201 

    0xa44eb991,// 205 PAY 202 

    0xea21eb45,// 206 PAY 203 

    0x289ab71b,// 207 PAY 204 

    0xd0c141ce,// 208 PAY 205 

    0x52982cb3,// 209 PAY 206 

    0x3dd1f213,// 210 PAY 207 

    0xdfde137e,// 211 PAY 208 

    0x42a3a472,// 212 PAY 209 

    0x31410465,// 213 PAY 210 

    0x3491fbe0,// 214 PAY 211 

    0x1c7491d2,// 215 PAY 212 

    0x4ffcac54,// 216 PAY 213 

    0x5033e5e0,// 217 PAY 214 

    0x8d516ba4,// 218 PAY 215 

    0x432bc37d,// 219 PAY 216 

    0xc5149b1d,// 220 PAY 217 

    0x8da90d64,// 221 PAY 218 

    0x312d5991,// 222 PAY 219 

    0x1a09df1c,// 223 PAY 220 

    0x8020b641,// 224 PAY 221 

    0x33e0d04a,// 225 PAY 222 

    0x9a3b48c3,// 226 PAY 223 

    0x3dca9bc4,// 227 PAY 224 

    0x5ed9c417,// 228 PAY 225 

    0x8d9b3e3a,// 229 PAY 226 

    0xaa226e02,// 230 PAY 227 

    0x99196241,// 231 PAY 228 

    0x6fed8bc0,// 232 PAY 229 

    0xc7223205,// 233 PAY 230 

    0xbc2b1411,// 234 PAY 231 

    0xaa0e8489,// 235 PAY 232 

    0x0dba8ee2,// 236 PAY 233 

    0xc07cd2a4,// 237 PAY 234 

    0xe6f59b47,// 238 PAY 235 

    0x9f0ca412,// 239 PAY 236 

    0x58685b4e,// 240 PAY 237 

    0x46566cd4,// 241 PAY 238 

    0x342c09f6,// 242 PAY 239 

    0x17f03722,// 243 PAY 240 

    0xbcfc824f,// 244 PAY 241 

    0x6bf28c54,// 245 PAY 242 

    0xe5c017bc,// 246 PAY 243 

    0x86ac8c6d,// 247 PAY 244 

    0xb3e34b1f,// 248 PAY 245 

    0x8b8b0bb9,// 249 PAY 246 

    0x944e085c,// 250 PAY 247 

    0x14025487,// 251 PAY 248 

    0xd7dc6d82,// 252 PAY 249 

    0x734e90cd,// 253 PAY 250 

    0xa1b2d00a,// 254 PAY 251 

    0x3bef63d9,// 255 PAY 252 

    0xefa2a6b7,// 256 PAY 253 

    0xe7116dfa,// 257 PAY 254 

    0xb3442ba5,// 258 PAY 255 

    0xce6fc95f,// 259 PAY 256 

    0x3fdfcda6,// 260 PAY 257 

    0x54ec39e0,// 261 PAY 258 

    0xf41f3a04,// 262 PAY 259 

    0x4e3c295c,// 263 PAY 260 

    0xc25e1a6f,// 264 PAY 261 

    0x0962ab9c,// 265 PAY 262 

    0xab752ebb,// 266 PAY 263 

    0x70973dcf,// 267 PAY 264 

    0xd74dd6bb,// 268 PAY 265 

    0x44932023,// 269 PAY 266 

    0xf1d905a1,// 270 PAY 267 

    0x3803f51e,// 271 PAY 268 

    0xa95a2ba3,// 272 PAY 269 

    0x6fed935d,// 273 PAY 270 

    0x542d8883,// 274 PAY 271 

    0xda026a27,// 275 PAY 272 

    0xfd083ca5,// 276 PAY 273 

    0x6a32aa5c,// 277 PAY 274 

    0x32bb9c17,// 278 PAY 275 

    0x3f3a08d1,// 279 PAY 276 

    0x9e2d4ad8,// 280 PAY 277 

    0xb0d123a5,// 281 PAY 278 

    0xeedcee9e,// 282 PAY 279 

    0x74092336,// 283 PAY 280 

    0x631632c9,// 284 PAY 281 

    0xdddbf367,// 285 PAY 282 

    0x7bc030e1,// 286 PAY 283 

    0x3f299402,// 287 PAY 284 

    0xa188eee5,// 288 PAY 285 

    0x1612afb7,// 289 PAY 286 

    0x632e5e2a,// 290 PAY 287 

    0x4d315e33,// 291 PAY 288 

    0xda306697,// 292 PAY 289 

    0xc81f0d7e,// 293 PAY 290 

    0x155595e7,// 294 PAY 291 

    0x880ab7d6,// 295 PAY 292 

    0xd9312d17,// 296 PAY 293 

    0x0932993a,// 297 PAY 294 

    0xf2917a94,// 298 PAY 295 

    0x6bfea16f,// 299 PAY 296 

    0x84a6a769,// 300 PAY 297 

    0x73fb7aa3,// 301 PAY 298 

    0xdf6ee810,// 302 PAY 299 

    0xb230bb06,// 303 PAY 300 

    0xb735a632,// 304 PAY 301 

    0x87b62d07,// 305 PAY 302 

    0xcf70a856,// 306 PAY 303 

    0x882d878f,// 307 PAY 304 

    0xfba09a91,// 308 PAY 305 

    0x26ea3f10,// 309 PAY 306 

    0xef6e7513,// 310 PAY 307 

    0x1e15f97e,// 311 PAY 308 

    0x845747e3,// 312 PAY 309 

    0x4b0f136f,// 313 PAY 310 

    0xc20ee2f5,// 314 PAY 311 

    0x35a79c00,// 315 PAY 312 

/// STA is 1 words. 

/// STA num_pkts       : 235 

/// STA pkt_idx        : 178 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x02c81ceb // 316 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt77_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 42 words. 

/// BDA size     is 161 (0xa1) 

/// BDA id       is 0xaa5 

    0x00a10aa5,// 3 BDA   1 

/// PAY Generic Data size   : 161 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x8d882227,// 4 PAY   1 

    0x4a2f0890,// 5 PAY   2 

    0xe860a79d,// 6 PAY   3 

    0xd23bcb7e,// 7 PAY   4 

    0x2ecfe28a,// 8 PAY   5 

    0xef6ea9fd,// 9 PAY   6 

    0x3e00f32d,// 10 PAY   7 

    0x64c4aab1,// 11 PAY   8 

    0x9862181e,// 12 PAY   9 

    0x4a87d215,// 13 PAY  10 

    0x33151bd1,// 14 PAY  11 

    0xeab7a399,// 15 PAY  12 

    0x9928858b,// 16 PAY  13 

    0x173a1f90,// 17 PAY  14 

    0xe031344b,// 18 PAY  15 

    0x21ac2f4f,// 19 PAY  16 

    0xd9249200,// 20 PAY  17 

    0xaee959bc,// 21 PAY  18 

    0x5b57de65,// 22 PAY  19 

    0x021a5861,// 23 PAY  20 

    0x2b9ce1d9,// 24 PAY  21 

    0x526e2f49,// 25 PAY  22 

    0x34eaf4f7,// 26 PAY  23 

    0x84d33331,// 27 PAY  24 

    0x5e2285c2,// 28 PAY  25 

    0xec6e2855,// 29 PAY  26 

    0x51070a71,// 30 PAY  27 

    0xe3a827f2,// 31 PAY  28 

    0xe494f5c5,// 32 PAY  29 

    0x960fcc2d,// 33 PAY  30 

    0x95f0a51f,// 34 PAY  31 

    0xe2fdd873,// 35 PAY  32 

    0x04bdb5c4,// 36 PAY  33 

    0x6c87f6c2,// 37 PAY  34 

    0x4a01a30d,// 38 PAY  35 

    0x4ed12686,// 39 PAY  36 

    0x9d9b29a9,// 40 PAY  37 

    0x24dc56f7,// 41 PAY  38 

    0x9c9271c2,// 42 PAY  39 

    0x161adddb,// 43 PAY  40 

    0x68000000,// 44 PAY  41 

/// STA is 1 words. 

/// STA num_pkts       : 254 

/// STA pkt_idx        : 71 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x011dbefe // 45 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt78_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 241 words. 

/// BDA size     is 959 (0x3bf) 

/// BDA id       is 0xdef3 

    0x03bfdef3,// 3 BDA   1 

/// PAY Generic Data size   : 959 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xeb60adb5,// 4 PAY   1 

    0x3449c59c,// 5 PAY   2 

    0x0467fa44,// 6 PAY   3 

    0xa79b1550,// 7 PAY   4 

    0x7b8ee55f,// 8 PAY   5 

    0xda2535f8,// 9 PAY   6 

    0x6520aac7,// 10 PAY   7 

    0xcb596df1,// 11 PAY   8 

    0x6d4c22e5,// 12 PAY   9 

    0xd80ce3fa,// 13 PAY  10 

    0x30b09b60,// 14 PAY  11 

    0xcacfb2b9,// 15 PAY  12 

    0xed5b0c59,// 16 PAY  13 

    0x45afec8c,// 17 PAY  14 

    0x227fa658,// 18 PAY  15 

    0x1243d9c9,// 19 PAY  16 

    0x391a2a9f,// 20 PAY  17 

    0x16d9ba47,// 21 PAY  18 

    0x7727e89b,// 22 PAY  19 

    0x456e94c5,// 23 PAY  20 

    0x8fcb5008,// 24 PAY  21 

    0x81a7151b,// 25 PAY  22 

    0xa1a18065,// 26 PAY  23 

    0x3fbd32f4,// 27 PAY  24 

    0xa14aff75,// 28 PAY  25 

    0x164d7e8f,// 29 PAY  26 

    0x1bb1b336,// 30 PAY  27 

    0xdf1a14e0,// 31 PAY  28 

    0x004036ba,// 32 PAY  29 

    0x763e9e09,// 33 PAY  30 

    0x9e3825dc,// 34 PAY  31 

    0xbe96f7e7,// 35 PAY  32 

    0x6db32d12,// 36 PAY  33 

    0xb3724b12,// 37 PAY  34 

    0x9c6fc1fe,// 38 PAY  35 

    0x39f2f85f,// 39 PAY  36 

    0x516aaaf1,// 40 PAY  37 

    0x3bd84c7f,// 41 PAY  38 

    0x35627b58,// 42 PAY  39 

    0x66da6db5,// 43 PAY  40 

    0xfd6fdaa9,// 44 PAY  41 

    0x566d09fc,// 45 PAY  42 

    0xd691de97,// 46 PAY  43 

    0x3c62c31f,// 47 PAY  44 

    0x61bfa746,// 48 PAY  45 

    0xc05d5177,// 49 PAY  46 

    0x78077788,// 50 PAY  47 

    0x2e0e05a6,// 51 PAY  48 

    0xe7465b26,// 52 PAY  49 

    0xa2ac4240,// 53 PAY  50 

    0x191eec52,// 54 PAY  51 

    0x5a0f491e,// 55 PAY  52 

    0x70806509,// 56 PAY  53 

    0xe4b85ba1,// 57 PAY  54 

    0xa35abf9b,// 58 PAY  55 

    0xf2f55ec7,// 59 PAY  56 

    0x2df3f1e5,// 60 PAY  57 

    0x1d8a9f46,// 61 PAY  58 

    0xec4f7d57,// 62 PAY  59 

    0xf8852ac5,// 63 PAY  60 

    0x0a9cb98f,// 64 PAY  61 

    0xcb1b6bae,// 65 PAY  62 

    0x1c3c6759,// 66 PAY  63 

    0x2b0bede6,// 67 PAY  64 

    0xcc57fea4,// 68 PAY  65 

    0x8a3c1008,// 69 PAY  66 

    0x61bc2290,// 70 PAY  67 

    0xc50fc322,// 71 PAY  68 

    0x16dbe889,// 72 PAY  69 

    0x9df4feeb,// 73 PAY  70 

    0x13146578,// 74 PAY  71 

    0x388ba55a,// 75 PAY  72 

    0x31e5dce8,// 76 PAY  73 

    0x2e583c85,// 77 PAY  74 

    0xc9b14c48,// 78 PAY  75 

    0x1eabfe90,// 79 PAY  76 

    0x8bbaf334,// 80 PAY  77 

    0x407089ce,// 81 PAY  78 

    0xb240381c,// 82 PAY  79 

    0xa462509a,// 83 PAY  80 

    0xf437c5c6,// 84 PAY  81 

    0x26908ef2,// 85 PAY  82 

    0x3a2e40c5,// 86 PAY  83 

    0x0ca87800,// 87 PAY  84 

    0x15d93093,// 88 PAY  85 

    0x691f2ab7,// 89 PAY  86 

    0x13ed2fd4,// 90 PAY  87 

    0x305f50a0,// 91 PAY  88 

    0x89ccc7b6,// 92 PAY  89 

    0x4e0d785b,// 93 PAY  90 

    0x880df010,// 94 PAY  91 

    0x69e16a80,// 95 PAY  92 

    0x9bbc858f,// 96 PAY  93 

    0x1f4cd2fc,// 97 PAY  94 

    0x498bea34,// 98 PAY  95 

    0x64d254ba,// 99 PAY  96 

    0xb70fecc9,// 100 PAY  97 

    0x6dcb28cc,// 101 PAY  98 

    0x2642a8b2,// 102 PAY  99 

    0x06d6efc6,// 103 PAY 100 

    0x6069eff2,// 104 PAY 101 

    0xd3575b2f,// 105 PAY 102 

    0x6478bd8f,// 106 PAY 103 

    0x640926ff,// 107 PAY 104 

    0x4bd0de10,// 108 PAY 105 

    0x0ff5300a,// 109 PAY 106 

    0x84876017,// 110 PAY 107 

    0x44126d12,// 111 PAY 108 

    0xe6d2851c,// 112 PAY 109 

    0xc427bc98,// 113 PAY 110 

    0x1716ea80,// 114 PAY 111 

    0x1746be75,// 115 PAY 112 

    0x271ca9c4,// 116 PAY 113 

    0xa76a7a0c,// 117 PAY 114 

    0x0ddbb3c7,// 118 PAY 115 

    0x181eb68c,// 119 PAY 116 

    0x0b3f09a9,// 120 PAY 117 

    0xfecbaf63,// 121 PAY 118 

    0xed760ee7,// 122 PAY 119 

    0xe80f305a,// 123 PAY 120 

    0xfa9013fd,// 124 PAY 121 

    0x8d4732d5,// 125 PAY 122 

    0xcccdfe0e,// 126 PAY 123 

    0xf4b4cf94,// 127 PAY 124 

    0x45b41eb5,// 128 PAY 125 

    0x6ec22e4a,// 129 PAY 126 

    0xafb15c94,// 130 PAY 127 

    0x6ca8530b,// 131 PAY 128 

    0x88a8e23b,// 132 PAY 129 

    0x3fe07c65,// 133 PAY 130 

    0x10875276,// 134 PAY 131 

    0x1c05bb10,// 135 PAY 132 

    0x29ae52a2,// 136 PAY 133 

    0x3cf52bde,// 137 PAY 134 

    0xfa23ef33,// 138 PAY 135 

    0xfd79d597,// 139 PAY 136 

    0x9137b6fd,// 140 PAY 137 

    0xf95b8fd8,// 141 PAY 138 

    0x8c1fbf0d,// 142 PAY 139 

    0x0e7a129b,// 143 PAY 140 

    0xa947a92f,// 144 PAY 141 

    0x98f33f08,// 145 PAY 142 

    0x8d7803ad,// 146 PAY 143 

    0x711ca66c,// 147 PAY 144 

    0xc23de4ad,// 148 PAY 145 

    0x17d924dc,// 149 PAY 146 

    0x9ce3d29f,// 150 PAY 147 

    0x33b46195,// 151 PAY 148 

    0x0b629d67,// 152 PAY 149 

    0x42b81b9f,// 153 PAY 150 

    0xa1cc8730,// 154 PAY 151 

    0xc3966d05,// 155 PAY 152 

    0xad6e8510,// 156 PAY 153 

    0x0d8063c5,// 157 PAY 154 

    0x4be0c034,// 158 PAY 155 

    0x0fdb991d,// 159 PAY 156 

    0x9a64780a,// 160 PAY 157 

    0xcfd6c24f,// 161 PAY 158 

    0x033c5bcf,// 162 PAY 159 

    0x08bcc024,// 163 PAY 160 

    0x443a2f05,// 164 PAY 161 

    0xc19208b9,// 165 PAY 162 

    0x99213359,// 166 PAY 163 

    0xbd078312,// 167 PAY 164 

    0x41944a21,// 168 PAY 165 

    0xc7fae7b8,// 169 PAY 166 

    0x447658ac,// 170 PAY 167 

    0x643e1bfa,// 171 PAY 168 

    0x324e4124,// 172 PAY 169 

    0xc99f7974,// 173 PAY 170 

    0x7ae8cc50,// 174 PAY 171 

    0x0337e98d,// 175 PAY 172 

    0x61ba634c,// 176 PAY 173 

    0xda544797,// 177 PAY 174 

    0xbc1c97ff,// 178 PAY 175 

    0x7c5fd6a3,// 179 PAY 176 

    0x8b1e548b,// 180 PAY 177 

    0x5b1420b7,// 181 PAY 178 

    0x5f1730ba,// 182 PAY 179 

    0x6400ddd4,// 183 PAY 180 

    0x3c1e387e,// 184 PAY 181 

    0x7571abb1,// 185 PAY 182 

    0x71c2950b,// 186 PAY 183 

    0xb682e986,// 187 PAY 184 

    0xc476b87f,// 188 PAY 185 

    0x8eea0018,// 189 PAY 186 

    0x406740c7,// 190 PAY 187 

    0x4f9548bd,// 191 PAY 188 

    0xcf13660f,// 192 PAY 189 

    0x8a5fdb28,// 193 PAY 190 

    0x890ab3a4,// 194 PAY 191 

    0xec216813,// 195 PAY 192 

    0x792db5b7,// 196 PAY 193 

    0xf189dff1,// 197 PAY 194 

    0x28006226,// 198 PAY 195 

    0x42c2872f,// 199 PAY 196 

    0x284481af,// 200 PAY 197 

    0xfe590fba,// 201 PAY 198 

    0x94a13399,// 202 PAY 199 

    0x0d4397e8,// 203 PAY 200 

    0x913e5173,// 204 PAY 201 

    0x4ad3fb8a,// 205 PAY 202 

    0xb4eb83bd,// 206 PAY 203 

    0xc52ad38b,// 207 PAY 204 

    0x84834d59,// 208 PAY 205 

    0x1778f8c3,// 209 PAY 206 

    0x2e5410fc,// 210 PAY 207 

    0xa49dbcf4,// 211 PAY 208 

    0x392b1124,// 212 PAY 209 

    0x2553d300,// 213 PAY 210 

    0xfc0305a7,// 214 PAY 211 

    0xe4fd922d,// 215 PAY 212 

    0xe5b0379c,// 216 PAY 213 

    0xdc0221ed,// 217 PAY 214 

    0xaf163ab8,// 218 PAY 215 

    0x95fe0955,// 219 PAY 216 

    0x915949b5,// 220 PAY 217 

    0x78814b58,// 221 PAY 218 

    0x604973c0,// 222 PAY 219 

    0x052fd3bd,// 223 PAY 220 

    0xc4d4f180,// 224 PAY 221 

    0xa6438492,// 225 PAY 222 

    0x64f00779,// 226 PAY 223 

    0x57551ac0,// 227 PAY 224 

    0xb079e2b8,// 228 PAY 225 

    0xb4f04480,// 229 PAY 226 

    0x6f06b6e4,// 230 PAY 227 

    0xef5ca4b9,// 231 PAY 228 

    0x24edc366,// 232 PAY 229 

    0xa76548c3,// 233 PAY 230 

    0xf2118ec4,// 234 PAY 231 

    0xb88c6795,// 235 PAY 232 

    0x990d6ffc,// 236 PAY 233 

    0x4a70e0c6,// 237 PAY 234 

    0x92471cd5,// 238 PAY 235 

    0xf59d1f45,// 239 PAY 236 

    0x71b21bb8,// 240 PAY 237 

    0x2823ed7b,// 241 PAY 238 

    0xac26dddf,// 242 PAY 239 

    0x8b60d200,// 243 PAY 240 

/// HASH is  4 bytes 

    0x0e7a129b,// 244 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 168 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf4 

    0x02a0f437 // 245 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt79_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 355 words. 

/// BDA size     is 1414 (0x586) 

/// BDA id       is 0x4d3 

    0x058604d3,// 3 BDA   1 

/// PAY Generic Data size   : 1414 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa44f112f,// 4 PAY   1 

    0xb103ef88,// 5 PAY   2 

    0x7df9e0ca,// 6 PAY   3 

    0x6ffc6855,// 7 PAY   4 

    0xb46a6a90,// 8 PAY   5 

    0x70b0a7fc,// 9 PAY   6 

    0x897b312a,// 10 PAY   7 

    0x179b476e,// 11 PAY   8 

    0x8d635ef5,// 12 PAY   9 

    0xfb7e0057,// 13 PAY  10 

    0x64ba4f58,// 14 PAY  11 

    0xcdb34a39,// 15 PAY  12 

    0x65b4f690,// 16 PAY  13 

    0x0a8dd642,// 17 PAY  14 

    0x3a498e74,// 18 PAY  15 

    0x69bec744,// 19 PAY  16 

    0x36112f4d,// 20 PAY  17 

    0x2943c8f2,// 21 PAY  18 

    0x6b5658bd,// 22 PAY  19 

    0xcd04d52d,// 23 PAY  20 

    0xb8ddd4f0,// 24 PAY  21 

    0xbb7eb4c1,// 25 PAY  22 

    0x4021f3e1,// 26 PAY  23 

    0x1b10a3de,// 27 PAY  24 

    0x78a4d2fd,// 28 PAY  25 

    0x495de48f,// 29 PAY  26 

    0xb4cb3696,// 30 PAY  27 

    0xe254e37c,// 31 PAY  28 

    0xf27caec1,// 32 PAY  29 

    0x0fe681a2,// 33 PAY  30 

    0xd3b28af7,// 34 PAY  31 

    0x70c8e73c,// 35 PAY  32 

    0xb39d490d,// 36 PAY  33 

    0xfbc9835f,// 37 PAY  34 

    0x77b13510,// 38 PAY  35 

    0x5e963f40,// 39 PAY  36 

    0xb00adfd1,// 40 PAY  37 

    0xe824110b,// 41 PAY  38 

    0xa00ebbb9,// 42 PAY  39 

    0x08c2a3ad,// 43 PAY  40 

    0xb4fd2a4b,// 44 PAY  41 

    0xc6487a82,// 45 PAY  42 

    0x2d15d445,// 46 PAY  43 

    0x8b6da151,// 47 PAY  44 

    0xdbccd536,// 48 PAY  45 

    0xa8bea0ed,// 49 PAY  46 

    0x7077d984,// 50 PAY  47 

    0xcbfbc485,// 51 PAY  48 

    0x1837a651,// 52 PAY  49 

    0x00f7f764,// 53 PAY  50 

    0x27b2abb4,// 54 PAY  51 

    0x32e4b01d,// 55 PAY  52 

    0xb8fcc5bc,// 56 PAY  53 

    0x3a0ca183,// 57 PAY  54 

    0xe3f47344,// 58 PAY  55 

    0xfe49897c,// 59 PAY  56 

    0x0588ada5,// 60 PAY  57 

    0xb8f31418,// 61 PAY  58 

    0x1eaef96f,// 62 PAY  59 

    0x990556e7,// 63 PAY  60 

    0x5513aef5,// 64 PAY  61 

    0x9fcf52f6,// 65 PAY  62 

    0xd1f58640,// 66 PAY  63 

    0x32e2182e,// 67 PAY  64 

    0x8803ea68,// 68 PAY  65 

    0xf07a1945,// 69 PAY  66 

    0xb9c9e1d9,// 70 PAY  67 

    0xb0c8d4f6,// 71 PAY  68 

    0x262a94a0,// 72 PAY  69 

    0xf6ecb4ab,// 73 PAY  70 

    0xb6a3289b,// 74 PAY  71 

    0xb28f0e14,// 75 PAY  72 

    0xc402a3ef,// 76 PAY  73 

    0x2e26c3d5,// 77 PAY  74 

    0x2946e422,// 78 PAY  75 

    0x45b04ec3,// 79 PAY  76 

    0xfbd53d44,// 80 PAY  77 

    0xe06b12ea,// 81 PAY  78 

    0x2e9e2164,// 82 PAY  79 

    0xaba69ceb,// 83 PAY  80 

    0xfc8abc5d,// 84 PAY  81 

    0xe34b5d54,// 85 PAY  82 

    0xc7c91eeb,// 86 PAY  83 

    0x1d8b54ad,// 87 PAY  84 

    0xb7169159,// 88 PAY  85 

    0x5072bb41,// 89 PAY  86 

    0x6e7c4a56,// 90 PAY  87 

    0x2c2aebb2,// 91 PAY  88 

    0xd9ef38d4,// 92 PAY  89 

    0xd8eb48bc,// 93 PAY  90 

    0xd0440665,// 94 PAY  91 

    0xd8447918,// 95 PAY  92 

    0x9ee01def,// 96 PAY  93 

    0x132344ff,// 97 PAY  94 

    0xdc79ade6,// 98 PAY  95 

    0x54d79efc,// 99 PAY  96 

    0x6494f8c7,// 100 PAY  97 

    0x7b6866bb,// 101 PAY  98 

    0x7e0c4dd2,// 102 PAY  99 

    0x5f690812,// 103 PAY 100 

    0xde14d3d3,// 104 PAY 101 

    0xe3f2821b,// 105 PAY 102 

    0xcf82e15e,// 106 PAY 103 

    0x6e904e6f,// 107 PAY 104 

    0xa30d2991,// 108 PAY 105 

    0xee290e1d,// 109 PAY 106 

    0x47c28246,// 110 PAY 107 

    0xfccc7aef,// 111 PAY 108 

    0x5ae317b4,// 112 PAY 109 

    0xe94dd933,// 113 PAY 110 

    0xda6e4fd9,// 114 PAY 111 

    0x5c06a162,// 115 PAY 112 

    0x33117560,// 116 PAY 113 

    0x1988b225,// 117 PAY 114 

    0xade64ee0,// 118 PAY 115 

    0x10291000,// 119 PAY 116 

    0x3ca7aae1,// 120 PAY 117 

    0x0b5ae26b,// 121 PAY 118 

    0xc0a3d11f,// 122 PAY 119 

    0x6e9cf915,// 123 PAY 120 

    0x33ed060d,// 124 PAY 121 

    0x74131d3d,// 125 PAY 122 

    0xae7b294a,// 126 PAY 123 

    0x23d876ac,// 127 PAY 124 

    0x0404c9c5,// 128 PAY 125 

    0xf018b9b7,// 129 PAY 126 

    0x8485a975,// 130 PAY 127 

    0x6d9aec6c,// 131 PAY 128 

    0xd4816cbb,// 132 PAY 129 

    0xf150c848,// 133 PAY 130 

    0xa996f30c,// 134 PAY 131 

    0xa2b71d23,// 135 PAY 132 

    0x20615765,// 136 PAY 133 

    0x9edb48da,// 137 PAY 134 

    0xed11a5da,// 138 PAY 135 

    0xa5b01817,// 139 PAY 136 

    0x30c3d750,// 140 PAY 137 

    0xd82a4df9,// 141 PAY 138 

    0x2c08faf8,// 142 PAY 139 

    0x39f740e4,// 143 PAY 140 

    0xc1e3d278,// 144 PAY 141 

    0xfc962da2,// 145 PAY 142 

    0x04033476,// 146 PAY 143 

    0xba39f6c4,// 147 PAY 144 

    0x1f307497,// 148 PAY 145 

    0x884574b6,// 149 PAY 146 

    0x635f18c7,// 150 PAY 147 

    0xde264c77,// 151 PAY 148 

    0x4732b862,// 152 PAY 149 

    0x3b91f0dd,// 153 PAY 150 

    0xc80312c2,// 154 PAY 151 

    0x0915cc14,// 155 PAY 152 

    0x35943c20,// 156 PAY 153 

    0x995f6c93,// 157 PAY 154 

    0x50594e51,// 158 PAY 155 

    0x4b5d8f2e,// 159 PAY 156 

    0x56fbc12d,// 160 PAY 157 

    0x944b7ebc,// 161 PAY 158 

    0x51c2d709,// 162 PAY 159 

    0xd9d95426,// 163 PAY 160 

    0xcba00c62,// 164 PAY 161 

    0x63c0e979,// 165 PAY 162 

    0x0a028882,// 166 PAY 163 

    0x59911d53,// 167 PAY 164 

    0x81217b4e,// 168 PAY 165 

    0x78c568c0,// 169 PAY 166 

    0xe96df748,// 170 PAY 167 

    0x1e6fd184,// 171 PAY 168 

    0xf5c0b2e6,// 172 PAY 169 

    0x6d967f18,// 173 PAY 170 

    0xd7edadc5,// 174 PAY 171 

    0x9a2ddec2,// 175 PAY 172 

    0xe515affa,// 176 PAY 173 

    0xe755b4eb,// 177 PAY 174 

    0x93656266,// 178 PAY 175 

    0x107336cc,// 179 PAY 176 

    0x0cc9ad16,// 180 PAY 177 

    0xff0bd07e,// 181 PAY 178 

    0x979a7b90,// 182 PAY 179 

    0x24853716,// 183 PAY 180 

    0x0362db59,// 184 PAY 181 

    0x7b3a7e30,// 185 PAY 182 

    0x8661bf6e,// 186 PAY 183 

    0x5ba02c50,// 187 PAY 184 

    0x44787274,// 188 PAY 185 

    0x2087cbf0,// 189 PAY 186 

    0xd08e9cd4,// 190 PAY 187 

    0x949251fc,// 191 PAY 188 

    0x02df3435,// 192 PAY 189 

    0xb8163cac,// 193 PAY 190 

    0x0bda4a97,// 194 PAY 191 

    0x3dcbedd2,// 195 PAY 192 

    0x00988fe3,// 196 PAY 193 

    0x770b4849,// 197 PAY 194 

    0xaf774cd3,// 198 PAY 195 

    0x2c40d3cc,// 199 PAY 196 

    0xff9a8e57,// 200 PAY 197 

    0x75f27f59,// 201 PAY 198 

    0x5015a05e,// 202 PAY 199 

    0xec399f06,// 203 PAY 200 

    0x15a33a46,// 204 PAY 201 

    0x5c9ecf67,// 205 PAY 202 

    0xc776eb66,// 206 PAY 203 

    0xcdee9cbd,// 207 PAY 204 

    0x6230f2e9,// 208 PAY 205 

    0x18b8d48a,// 209 PAY 206 

    0x57dbe6ba,// 210 PAY 207 

    0x68cbdffe,// 211 PAY 208 

    0x898c3b92,// 212 PAY 209 

    0x0360091a,// 213 PAY 210 

    0x1bceeb47,// 214 PAY 211 

    0x71ca1a0c,// 215 PAY 212 

    0x95a8a34a,// 216 PAY 213 

    0x9b9a04c6,// 217 PAY 214 

    0xf25e742a,// 218 PAY 215 

    0xba7017ee,// 219 PAY 216 

    0x22c16305,// 220 PAY 217 

    0x2ca4d41d,// 221 PAY 218 

    0xddee6527,// 222 PAY 219 

    0xba2b29ab,// 223 PAY 220 

    0x1ebc7565,// 224 PAY 221 

    0xdb094193,// 225 PAY 222 

    0x0f8012c2,// 226 PAY 223 

    0x6ddae70e,// 227 PAY 224 

    0x3e9dc1f3,// 228 PAY 225 

    0xe63acc4c,// 229 PAY 226 

    0x365c3be3,// 230 PAY 227 

    0x7a3abdaa,// 231 PAY 228 

    0x8464bfe7,// 232 PAY 229 

    0x09c0e809,// 233 PAY 230 

    0xe5307516,// 234 PAY 231 

    0xa8c84596,// 235 PAY 232 

    0x03246ea4,// 236 PAY 233 

    0xcbc7592a,// 237 PAY 234 

    0x36dc0f5c,// 238 PAY 235 

    0xe4c8b61b,// 239 PAY 236 

    0x31d91536,// 240 PAY 237 

    0xb016bbf4,// 241 PAY 238 

    0x90a5f955,// 242 PAY 239 

    0x88d4e711,// 243 PAY 240 

    0x9db17ab5,// 244 PAY 241 

    0xad449f9e,// 245 PAY 242 

    0x292e7b21,// 246 PAY 243 

    0x7486dcc0,// 247 PAY 244 

    0xe0fa86d8,// 248 PAY 245 

    0x9b72939b,// 249 PAY 246 

    0x99402dee,// 250 PAY 247 

    0x93804a5d,// 251 PAY 248 

    0xda3217e6,// 252 PAY 249 

    0x97d7c005,// 253 PAY 250 

    0xe3d6095a,// 254 PAY 251 

    0xcf884b0f,// 255 PAY 252 

    0xd0095a68,// 256 PAY 253 

    0xb528602a,// 257 PAY 254 

    0x59d290b7,// 258 PAY 255 

    0x490dcc6c,// 259 PAY 256 

    0x1eb91476,// 260 PAY 257 

    0x17f09b38,// 261 PAY 258 

    0x2a5b3e7f,// 262 PAY 259 

    0x8e342218,// 263 PAY 260 

    0xcab2b610,// 264 PAY 261 

    0x113c97cc,// 265 PAY 262 

    0x08ee2f74,// 266 PAY 263 

    0xb3b7f18b,// 267 PAY 264 

    0x64242487,// 268 PAY 265 

    0xbf23fc13,// 269 PAY 266 

    0x9d7b9bb6,// 270 PAY 267 

    0x684782d2,// 271 PAY 268 

    0xea416f87,// 272 PAY 269 

    0x3edc4314,// 273 PAY 270 

    0xa13040a7,// 274 PAY 271 

    0x372b5406,// 275 PAY 272 

    0x135757ff,// 276 PAY 273 

    0x52cbb5dc,// 277 PAY 274 

    0x83006b8f,// 278 PAY 275 

    0xe18c914c,// 279 PAY 276 

    0x13fc0b6c,// 280 PAY 277 

    0x1d401038,// 281 PAY 278 

    0xf232d9c4,// 282 PAY 279 

    0x2d251337,// 283 PAY 280 

    0xf1c7a9f1,// 284 PAY 281 

    0xb9c1fd46,// 285 PAY 282 

    0xf82ea63a,// 286 PAY 283 

    0xbe41a74c,// 287 PAY 284 

    0x007c799e,// 288 PAY 285 

    0x876ca738,// 289 PAY 286 

    0xca5862f5,// 290 PAY 287 

    0x6566ac5e,// 291 PAY 288 

    0xcb259acf,// 292 PAY 289 

    0xeeafdf22,// 293 PAY 290 

    0x3b39706d,// 294 PAY 291 

    0x23e5c72b,// 295 PAY 292 

    0xbd5edce5,// 296 PAY 293 

    0xda6488d8,// 297 PAY 294 

    0xaf8ed06d,// 298 PAY 295 

    0xfa992f35,// 299 PAY 296 

    0x6935044b,// 300 PAY 297 

    0xdcf6f4a9,// 301 PAY 298 

    0xf925f9d9,// 302 PAY 299 

    0xb6642cbe,// 303 PAY 300 

    0x1654f63f,// 304 PAY 301 

    0xdedb3b57,// 305 PAY 302 

    0x31dc8ee6,// 306 PAY 303 

    0x9e7e3505,// 307 PAY 304 

    0x23ad48a1,// 308 PAY 305 

    0x464cb102,// 309 PAY 306 

    0x4c82bd54,// 310 PAY 307 

    0x42484f9c,// 311 PAY 308 

    0xfead2285,// 312 PAY 309 

    0x11ffbd75,// 313 PAY 310 

    0x88c85ebb,// 314 PAY 311 

    0xce497c5b,// 315 PAY 312 

    0xc7649772,// 316 PAY 313 

    0x6dc01a2c,// 317 PAY 314 

    0x86dcebb5,// 318 PAY 315 

    0x941c5b30,// 319 PAY 316 

    0x7dc771bc,// 320 PAY 317 

    0x8324547b,// 321 PAY 318 

    0x318d0a90,// 322 PAY 319 

    0x662c14f7,// 323 PAY 320 

    0x769c77f8,// 324 PAY 321 

    0x11acf37d,// 325 PAY 322 

    0x5c7865cf,// 326 PAY 323 

    0xe1bf0fa5,// 327 PAY 324 

    0x0ec9ba25,// 328 PAY 325 

    0xe906f9da,// 329 PAY 326 

    0x7b8d2a24,// 330 PAY 327 

    0xfb741fe8,// 331 PAY 328 

    0xefa61b68,// 332 PAY 329 

    0x8df342b2,// 333 PAY 330 

    0x69c92aa1,// 334 PAY 331 

    0xb2f92f4e,// 335 PAY 332 

    0x4f3e3a3b,// 336 PAY 333 

    0xd67949c1,// 337 PAY 334 

    0x2a5f2927,// 338 PAY 335 

    0x16fdf7fc,// 339 PAY 336 

    0xc7e5a0c9,// 340 PAY 337 

    0xc8bcd42d,// 341 PAY 338 

    0xfab67935,// 342 PAY 339 

    0xf48c16ba,// 343 PAY 340 

    0xd988788a,// 344 PAY 341 

    0x88697738,// 345 PAY 342 

    0x96c0e8bf,// 346 PAY 343 

    0x18834df7,// 347 PAY 344 

    0xad5b163f,// 348 PAY 345 

    0xa20a9405,// 349 PAY 346 

    0xc4e3530b,// 350 PAY 347 

    0xa4815b32,// 351 PAY 348 

    0x06220567,// 352 PAY 349 

    0xc83b3e39,// 353 PAY 350 

    0xe4840966,// 354 PAY 351 

    0x7e18622e,// 355 PAY 352 

    0xddde866c,// 356 PAY 353 

    0x8e440000,// 357 PAY 354 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf 

    0x01690f2d // 358 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt80_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 289 words. 

/// BDA size     is 1151 (0x47f) 

/// BDA id       is 0xcf5d 

    0x047fcf5d,// 3 BDA   1 

/// PAY Generic Data size   : 1151 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xc9fb8d51,// 4 PAY   1 

    0xa3f07b74,// 5 PAY   2 

    0x70ee2e66,// 6 PAY   3 

    0x1bc56bf5,// 7 PAY   4 

    0x3f551aaf,// 8 PAY   5 

    0x17923275,// 9 PAY   6 

    0x89d4a59a,// 10 PAY   7 

    0x111432fe,// 11 PAY   8 

    0x7d2ca925,// 12 PAY   9 

    0x6a12443a,// 13 PAY  10 

    0x612e5438,// 14 PAY  11 

    0x6738b053,// 15 PAY  12 

    0x5aefaf3f,// 16 PAY  13 

    0x9eba340a,// 17 PAY  14 

    0xc82d4882,// 18 PAY  15 

    0x51370da4,// 19 PAY  16 

    0xa3bd2f68,// 20 PAY  17 

    0x47688d5a,// 21 PAY  18 

    0x543ee284,// 22 PAY  19 

    0x53bcf2d1,// 23 PAY  20 

    0x6d308cdf,// 24 PAY  21 

    0x6e95cb04,// 25 PAY  22 

    0x36a1cc08,// 26 PAY  23 

    0x97578dce,// 27 PAY  24 

    0x5f9e14e7,// 28 PAY  25 

    0x8f2c046c,// 29 PAY  26 

    0x5f3d9d21,// 30 PAY  27 

    0xdb484309,// 31 PAY  28 

    0xf91c5414,// 32 PAY  29 

    0x3c3242af,// 33 PAY  30 

    0x527ed3a3,// 34 PAY  31 

    0xe7182a9e,// 35 PAY  32 

    0x193870b9,// 36 PAY  33 

    0xe1cac12d,// 37 PAY  34 

    0xc2c8a2d2,// 38 PAY  35 

    0xc86a8bb3,// 39 PAY  36 

    0x17e792e2,// 40 PAY  37 

    0xf7762bcc,// 41 PAY  38 

    0xff3cc911,// 42 PAY  39 

    0x18475786,// 43 PAY  40 

    0x178c742c,// 44 PAY  41 

    0x852acbf7,// 45 PAY  42 

    0x0ff5f437,// 46 PAY  43 

    0x8bdd4c9b,// 47 PAY  44 

    0x92f64d9c,// 48 PAY  45 

    0x6b1f0877,// 49 PAY  46 

    0xc417316e,// 50 PAY  47 

    0x4ebd9f60,// 51 PAY  48 

    0xaee168ef,// 52 PAY  49 

    0xe8201e18,// 53 PAY  50 

    0xeab6be69,// 54 PAY  51 

    0x6b26d1bf,// 55 PAY  52 

    0x9b56c3bf,// 56 PAY  53 

    0xa0cf302e,// 57 PAY  54 

    0xb40e30f9,// 58 PAY  55 

    0x59442201,// 59 PAY  56 

    0x9172ffcc,// 60 PAY  57 

    0xa685e8e7,// 61 PAY  58 

    0xe426a36b,// 62 PAY  59 

    0x529d4660,// 63 PAY  60 

    0xd7710658,// 64 PAY  61 

    0x4ab7a77b,// 65 PAY  62 

    0xf4b03aec,// 66 PAY  63 

    0x679583e6,// 67 PAY  64 

    0x8e6c761c,// 68 PAY  65 

    0x7ca721fb,// 69 PAY  66 

    0x50d482ed,// 70 PAY  67 

    0x864095d2,// 71 PAY  68 

    0x7985e100,// 72 PAY  69 

    0x0cc04a1a,// 73 PAY  70 

    0x5ccbfb6e,// 74 PAY  71 

    0xae49e54a,// 75 PAY  72 

    0x931cee03,// 76 PAY  73 

    0x1c9f6324,// 77 PAY  74 

    0x53f7e947,// 78 PAY  75 

    0x0e9c020c,// 79 PAY  76 

    0x453227fc,// 80 PAY  77 

    0x3d9e8a0f,// 81 PAY  78 

    0xe784c83b,// 82 PAY  79 

    0xdca176ee,// 83 PAY  80 

    0xa18af0b0,// 84 PAY  81 

    0x79e068ee,// 85 PAY  82 

    0x4e274fba,// 86 PAY  83 

    0x59a93e9e,// 87 PAY  84 

    0xe271dcb5,// 88 PAY  85 

    0x4c01980e,// 89 PAY  86 

    0x0866ab7e,// 90 PAY  87 

    0x5a2174e0,// 91 PAY  88 

    0x3c6f7719,// 92 PAY  89 

    0xae51d6f4,// 93 PAY  90 

    0x7f48b6d2,// 94 PAY  91 

    0x99e0117d,// 95 PAY  92 

    0x0cacb1c5,// 96 PAY  93 

    0x781be9aa,// 97 PAY  94 

    0xd354d806,// 98 PAY  95 

    0xf79b01ea,// 99 PAY  96 

    0xc102edeb,// 100 PAY  97 

    0xd43ef34c,// 101 PAY  98 

    0x609be1fb,// 102 PAY  99 

    0xf7482b3e,// 103 PAY 100 

    0x85a61b26,// 104 PAY 101 

    0xadf79deb,// 105 PAY 102 

    0xf239abb7,// 106 PAY 103 

    0x8249043f,// 107 PAY 104 

    0x9ba0ed34,// 108 PAY 105 

    0x5dc19299,// 109 PAY 106 

    0xe8da82b3,// 110 PAY 107 

    0xd5d17982,// 111 PAY 108 

    0x87ae7cae,// 112 PAY 109 

    0x374b9c10,// 113 PAY 110 

    0x513cecf8,// 114 PAY 111 

    0x1c2a46f7,// 115 PAY 112 

    0x16884b24,// 116 PAY 113 

    0x74a13a57,// 117 PAY 114 

    0x76799451,// 118 PAY 115 

    0xc15ddb17,// 119 PAY 116 

    0x14d63280,// 120 PAY 117 

    0x3540ee39,// 121 PAY 118 

    0x4118f8c0,// 122 PAY 119 

    0xbe609ffd,// 123 PAY 120 

    0x5ed465c5,// 124 PAY 121 

    0x98827375,// 125 PAY 122 

    0x5b437afb,// 126 PAY 123 

    0xc194d7f7,// 127 PAY 124 

    0xf4c73104,// 128 PAY 125 

    0xedcc60bd,// 129 PAY 126 

    0x6b3217fe,// 130 PAY 127 

    0xd06c015a,// 131 PAY 128 

    0x9d691bed,// 132 PAY 129 

    0x09d48758,// 133 PAY 130 

    0x4e36ce2f,// 134 PAY 131 

    0xb7960dda,// 135 PAY 132 

    0xac1a38e7,// 136 PAY 133 

    0x544f85cd,// 137 PAY 134 

    0x473b7769,// 138 PAY 135 

    0xa33c2929,// 139 PAY 136 

    0x921c596e,// 140 PAY 137 

    0x3e5d72d9,// 141 PAY 138 

    0xbbe949f9,// 142 PAY 139 

    0xde7d9238,// 143 PAY 140 

    0xec8d2070,// 144 PAY 141 

    0x9bf16ba7,// 145 PAY 142 

    0x7637ee35,// 146 PAY 143 

    0xf7fce72a,// 147 PAY 144 

    0x3f62c00f,// 148 PAY 145 

    0xe9d93f44,// 149 PAY 146 

    0xa188a067,// 150 PAY 147 

    0x5a3cb21d,// 151 PAY 148 

    0xc2348db5,// 152 PAY 149 

    0x5e538f88,// 153 PAY 150 

    0xc5894437,// 154 PAY 151 

    0xd1fb5531,// 155 PAY 152 

    0x56f702b5,// 156 PAY 153 

    0x4c197211,// 157 PAY 154 

    0xa8edd9af,// 158 PAY 155 

    0x4abc2832,// 159 PAY 156 

    0x8dd86f98,// 160 PAY 157 

    0x73773cbc,// 161 PAY 158 

    0x0b01f161,// 162 PAY 159 

    0xb589614a,// 163 PAY 160 

    0x0f8fa6b2,// 164 PAY 161 

    0xad1b7f8b,// 165 PAY 162 

    0xf287ffd5,// 166 PAY 163 

    0xf5b88715,// 167 PAY 164 

    0xcd71a435,// 168 PAY 165 

    0x45cd6e25,// 169 PAY 166 

    0x61602233,// 170 PAY 167 

    0x0948c53c,// 171 PAY 168 

    0x8aac0c21,// 172 PAY 169 

    0x1b624ee3,// 173 PAY 170 

    0x81c0097b,// 174 PAY 171 

    0xc33c1d74,// 175 PAY 172 

    0xa86f89b4,// 176 PAY 173 

    0x49ec0018,// 177 PAY 174 

    0xfd977a52,// 178 PAY 175 

    0xefe3bb7c,// 179 PAY 176 

    0xfc2e56a8,// 180 PAY 177 

    0xd4ef5e05,// 181 PAY 178 

    0xf8fddd61,// 182 PAY 179 

    0xe984322e,// 183 PAY 180 

    0xd456e1ff,// 184 PAY 181 

    0xe3f71309,// 185 PAY 182 

    0x13345b1b,// 186 PAY 183 

    0x9802427b,// 187 PAY 184 

    0x20a527b7,// 188 PAY 185 

    0xe4640979,// 189 PAY 186 

    0x78f27caa,// 190 PAY 187 

    0x087fb9ce,// 191 PAY 188 

    0xba762e4e,// 192 PAY 189 

    0xb31dd1b6,// 193 PAY 190 

    0x4a3f5544,// 194 PAY 191 

    0x4bdb92f0,// 195 PAY 192 

    0x0a1c9737,// 196 PAY 193 

    0x1152652d,// 197 PAY 194 

    0xffe03fd1,// 198 PAY 195 

    0xc85f9ccd,// 199 PAY 196 

    0xc324c133,// 200 PAY 197 

    0x869fd22a,// 201 PAY 198 

    0xc0a8b84f,// 202 PAY 199 

    0xfcb02e9a,// 203 PAY 200 

    0x3b2cc0ed,// 204 PAY 201 

    0x1f666ba8,// 205 PAY 202 

    0x37d234d0,// 206 PAY 203 

    0xd9be96a5,// 207 PAY 204 

    0x9c70b3a7,// 208 PAY 205 

    0x43ccb088,// 209 PAY 206 

    0xff56cd39,// 210 PAY 207 

    0x19a3946e,// 211 PAY 208 

    0x789fe741,// 212 PAY 209 

    0x85d2e807,// 213 PAY 210 

    0x0a8d95a6,// 214 PAY 211 

    0x2839edc4,// 215 PAY 212 

    0xfdc2578b,// 216 PAY 213 

    0xd0a52159,// 217 PAY 214 

    0x9c320f56,// 218 PAY 215 

    0x9491ffe1,// 219 PAY 216 

    0x1feaf8a0,// 220 PAY 217 

    0xfd9792a9,// 221 PAY 218 

    0x2d68d157,// 222 PAY 219 

    0xfb95633d,// 223 PAY 220 

    0x51354e53,// 224 PAY 221 

    0xb8a4211b,// 225 PAY 222 

    0xa74d725c,// 226 PAY 223 

    0xe466e420,// 227 PAY 224 

    0xe809096b,// 228 PAY 225 

    0x85ac0bbd,// 229 PAY 226 

    0x21c0191a,// 230 PAY 227 

    0x3d345847,// 231 PAY 228 

    0x75cf24ed,// 232 PAY 229 

    0xbbba0272,// 233 PAY 230 

    0x7f218899,// 234 PAY 231 

    0x323773d8,// 235 PAY 232 

    0x34227fef,// 236 PAY 233 

    0x742d53b5,// 237 PAY 234 

    0xc17677f9,// 238 PAY 235 

    0x0ceb32b4,// 239 PAY 236 

    0x54cb7777,// 240 PAY 237 

    0xeb13bccb,// 241 PAY 238 

    0x8d257e35,// 242 PAY 239 

    0x3d16a20b,// 243 PAY 240 

    0x2a8efe05,// 244 PAY 241 

    0x09567af1,// 245 PAY 242 

    0x2d5ddb01,// 246 PAY 243 

    0x57702a3b,// 247 PAY 244 

    0xd3e6c183,// 248 PAY 245 

    0x79c4f58f,// 249 PAY 246 

    0x203e4abd,// 250 PAY 247 

    0x62ef77c1,// 251 PAY 248 

    0x9357281e,// 252 PAY 249 

    0x7f0fb911,// 253 PAY 250 

    0x659ae8fc,// 254 PAY 251 

    0x9c431cb1,// 255 PAY 252 

    0x1e91a8d1,// 256 PAY 253 

    0xb3b344ca,// 257 PAY 254 

    0x6b8d17d0,// 258 PAY 255 

    0x5106272c,// 259 PAY 256 

    0xbd9eb1a9,// 260 PAY 257 

    0x91463475,// 261 PAY 258 

    0x85ae7c11,// 262 PAY 259 

    0xe3e99b37,// 263 PAY 260 

    0x9108b5c5,// 264 PAY 261 

    0x82b0eaa1,// 265 PAY 262 

    0x314ac907,// 266 PAY 263 

    0xae06ac7f,// 267 PAY 264 

    0x2bbd51ed,// 268 PAY 265 

    0x4c2f7471,// 269 PAY 266 

    0x5a2677ab,// 270 PAY 267 

    0xf5968ae6,// 271 PAY 268 

    0x7d5f67ca,// 272 PAY 269 

    0xa135c250,// 273 PAY 270 

    0x61a6781d,// 274 PAY 271 

    0x07b21f6f,// 275 PAY 272 

    0x28f25a8b,// 276 PAY 273 

    0xd0bf1b28,// 277 PAY 274 

    0x63a0dcc3,// 278 PAY 275 

    0x97773b16,// 279 PAY 276 

    0xf4e2beff,// 280 PAY 277 

    0xef330c9d,// 281 PAY 278 

    0x0fd2a971,// 282 PAY 279 

    0xbe1a9f7b,// 283 PAY 280 

    0x187196bd,// 284 PAY 281 

    0x6410e874,// 285 PAY 282 

    0xa65958fb,// 286 PAY 283 

    0xf78124c5,// 287 PAY 284 

    0x9bbf0d02,// 288 PAY 285 

    0x25f84150,// 289 PAY 286 

    0x2c9168c6,// 290 PAY 287 

    0x11e4eb00,// 291 PAY 288 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 255 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x03fc0485 // 292 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt81_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 230 words. 

/// BDA size     is 914 (0x392) 

/// BDA id       is 0xe599 

    0x0392e599,// 3 BDA   1 

/// PAY Generic Data size   : 914 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x94230425,// 4 PAY   1 

    0x6d8bcc38,// 5 PAY   2 

    0x8443b24b,// 6 PAY   3 

    0x86da18df,// 7 PAY   4 

    0x920289e5,// 8 PAY   5 

    0xa1304295,// 9 PAY   6 

    0x88008301,// 10 PAY   7 

    0x5f8dd84a,// 11 PAY   8 

    0x57ff1870,// 12 PAY   9 

    0x53c84c1a,// 13 PAY  10 

    0x225ddf92,// 14 PAY  11 

    0xf3575e80,// 15 PAY  12 

    0xd5506bb7,// 16 PAY  13 

    0x9b498e56,// 17 PAY  14 

    0xaaf46576,// 18 PAY  15 

    0x3f51643c,// 19 PAY  16 

    0xc00cacac,// 20 PAY  17 

    0x93b60e1a,// 21 PAY  18 

    0xb596f2e4,// 22 PAY  19 

    0x9bc0ec5b,// 23 PAY  20 

    0x55e49ac3,// 24 PAY  21 

    0x9e246637,// 25 PAY  22 

    0x1d2fa965,// 26 PAY  23 

    0xdb5e9973,// 27 PAY  24 

    0x76412bf2,// 28 PAY  25 

    0xeb0b2483,// 29 PAY  26 

    0xf3f9b941,// 30 PAY  27 

    0xecebc5d1,// 31 PAY  28 

    0xa91e8c32,// 32 PAY  29 

    0x87a68a14,// 33 PAY  30 

    0x69c2a85e,// 34 PAY  31 

    0x9fd18715,// 35 PAY  32 

    0x12760fb4,// 36 PAY  33 

    0xb5415d2b,// 37 PAY  34 

    0x4fce6c94,// 38 PAY  35 

    0x459f143d,// 39 PAY  36 

    0x6f76a89f,// 40 PAY  37 

    0x3a4ba348,// 41 PAY  38 

    0x5cf66e7c,// 42 PAY  39 

    0xf0c646dc,// 43 PAY  40 

    0x8dfd79d4,// 44 PAY  41 

    0x53fd7d0d,// 45 PAY  42 

    0x46d79d72,// 46 PAY  43 

    0x2a97ebba,// 47 PAY  44 

    0xd4ffefd7,// 48 PAY  45 

    0x009fab53,// 49 PAY  46 

    0x3df91fee,// 50 PAY  47 

    0xfbde531f,// 51 PAY  48 

    0x5c008d25,// 52 PAY  49 

    0x22f7a4aa,// 53 PAY  50 

    0x2c3f4de0,// 54 PAY  51 

    0x58b36a15,// 55 PAY  52 

    0xd0bdfef8,// 56 PAY  53 

    0xa6abb190,// 57 PAY  54 

    0xeb27e11e,// 58 PAY  55 

    0x59fc4f1c,// 59 PAY  56 

    0xed85a9ea,// 60 PAY  57 

    0xd2b6cd73,// 61 PAY  58 

    0x799a5d61,// 62 PAY  59 

    0xe056aeaa,// 63 PAY  60 

    0x357f8a31,// 64 PAY  61 

    0xd7ec6a84,// 65 PAY  62 

    0x35b8599b,// 66 PAY  63 

    0xef4ef20e,// 67 PAY  64 

    0x68677b10,// 68 PAY  65 

    0x79e7d1f8,// 69 PAY  66 

    0x958ce1a7,// 70 PAY  67 

    0x64a0fe32,// 71 PAY  68 

    0x681d81b6,// 72 PAY  69 

    0xa9f3abda,// 73 PAY  70 

    0x714defc3,// 74 PAY  71 

    0x12e7029a,// 75 PAY  72 

    0xeddcda25,// 76 PAY  73 

    0xad37dcdc,// 77 PAY  74 

    0x98a1480b,// 78 PAY  75 

    0x783d1bc6,// 79 PAY  76 

    0x12311cf9,// 80 PAY  77 

    0x92bb71d3,// 81 PAY  78 

    0x0abd4e07,// 82 PAY  79 

    0x511a45a1,// 83 PAY  80 

    0xeafe6be1,// 84 PAY  81 

    0x78ba9cce,// 85 PAY  82 

    0xfbc0c7bb,// 86 PAY  83 

    0x8427a613,// 87 PAY  84 

    0x27f5892c,// 88 PAY  85 

    0xe62661b3,// 89 PAY  86 

    0x6f2d63f4,// 90 PAY  87 

    0x49527b96,// 91 PAY  88 

    0x287b2b74,// 92 PAY  89 

    0xab2ca7c0,// 93 PAY  90 

    0x0938a645,// 94 PAY  91 

    0xc18ff1fd,// 95 PAY  92 

    0x646c6c82,// 96 PAY  93 

    0x76b1d57d,// 97 PAY  94 

    0xdb86a778,// 98 PAY  95 

    0x992c6ffe,// 99 PAY  96 

    0x4f6ec91b,// 100 PAY  97 

    0x2f045ee6,// 101 PAY  98 

    0xa32a56fb,// 102 PAY  99 

    0xda0ea239,// 103 PAY 100 

    0xc519621c,// 104 PAY 101 

    0xd48ab583,// 105 PAY 102 

    0x66407d86,// 106 PAY 103 

    0x9bcdc3fe,// 107 PAY 104 

    0xe07a8bdc,// 108 PAY 105 

    0xc25534f6,// 109 PAY 106 

    0x9c3851b2,// 110 PAY 107 

    0x3491a0cd,// 111 PAY 108 

    0x44c768ff,// 112 PAY 109 

    0x08f18bc4,// 113 PAY 110 

    0x53c019f0,// 114 PAY 111 

    0x4cab9e7d,// 115 PAY 112 

    0x37a884f9,// 116 PAY 113 

    0xa53e1357,// 117 PAY 114 

    0x42011228,// 118 PAY 115 

    0x4ed7d08d,// 119 PAY 116 

    0x2a964ac8,// 120 PAY 117 

    0x98c630b0,// 121 PAY 118 

    0x7bc2bca7,// 122 PAY 119 

    0x04cfc317,// 123 PAY 120 

    0xfc1ce5de,// 124 PAY 121 

    0x6e036a89,// 125 PAY 122 

    0x1a9ce7e9,// 126 PAY 123 

    0xab26edea,// 127 PAY 124 

    0xb2939101,// 128 PAY 125 

    0x0c3a3523,// 129 PAY 126 

    0x440e1b4e,// 130 PAY 127 

    0x633d6f85,// 131 PAY 128 

    0x67b98601,// 132 PAY 129 

    0xa2ccbee3,// 133 PAY 130 

    0xfc863b9d,// 134 PAY 131 

    0x0493c2f1,// 135 PAY 132 

    0xe8bebf9b,// 136 PAY 133 

    0x8954ba02,// 137 PAY 134 

    0xdd5d00ac,// 138 PAY 135 

    0x59bb9515,// 139 PAY 136 

    0x5471581e,// 140 PAY 137 

    0x0dbe226b,// 141 PAY 138 

    0xbe6d5324,// 142 PAY 139 

    0xce6ab920,// 143 PAY 140 

    0x4ed8580e,// 144 PAY 141 

    0xdef7db4b,// 145 PAY 142 

    0xbc4c035a,// 146 PAY 143 

    0x64122b44,// 147 PAY 144 

    0xf83eb135,// 148 PAY 145 

    0xc3027e22,// 149 PAY 146 

    0x49e9f2df,// 150 PAY 147 

    0x3f20be0f,// 151 PAY 148 

    0x4a5860f1,// 152 PAY 149 

    0x94558a0c,// 153 PAY 150 

    0x66d48a0c,// 154 PAY 151 

    0xc0b0b2c9,// 155 PAY 152 

    0xc9bdec6f,// 156 PAY 153 

    0x50fba84a,// 157 PAY 154 

    0xfffb7de1,// 158 PAY 155 

    0xc23bb6da,// 159 PAY 156 

    0x6bd7e5ba,// 160 PAY 157 

    0xb19780b2,// 161 PAY 158 

    0xd08d3fde,// 162 PAY 159 

    0xac9f8aad,// 163 PAY 160 

    0x85a23697,// 164 PAY 161 

    0x7b719f9e,// 165 PAY 162 

    0x79de39bc,// 166 PAY 163 

    0xdee43a7e,// 167 PAY 164 

    0x728b47b7,// 168 PAY 165 

    0x323876d0,// 169 PAY 166 

    0x59f642b8,// 170 PAY 167 

    0x8940edb4,// 171 PAY 168 

    0xb2cffda7,// 172 PAY 169 

    0x8aa0a788,// 173 PAY 170 

    0x2a0fafc6,// 174 PAY 171 

    0x09642262,// 175 PAY 172 

    0xa9b3cfdf,// 176 PAY 173 

    0x8061ae36,// 177 PAY 174 

    0xb1f4ab69,// 178 PAY 175 

    0xe80eee85,// 179 PAY 176 

    0x2db26279,// 180 PAY 177 

    0xd550e4ca,// 181 PAY 178 

    0x4f934677,// 182 PAY 179 

    0x7a9f3c33,// 183 PAY 180 

    0x0961d5a7,// 184 PAY 181 

    0x22fabbb0,// 185 PAY 182 

    0x3d76dd7a,// 186 PAY 183 

    0x13eae999,// 187 PAY 184 

    0xa1fd140e,// 188 PAY 185 

    0x9d596bdb,// 189 PAY 186 

    0x9de514c7,// 190 PAY 187 

    0xc219dd77,// 191 PAY 188 

    0x7de487ec,// 192 PAY 189 

    0x4b0194ae,// 193 PAY 190 

    0xe0f84e54,// 194 PAY 191 

    0x601cf868,// 195 PAY 192 

    0xaff7d714,// 196 PAY 193 

    0x745610af,// 197 PAY 194 

    0xdb20ddf8,// 198 PAY 195 

    0x3fbecfef,// 199 PAY 196 

    0xefc9aae3,// 200 PAY 197 

    0x0b202a3b,// 201 PAY 198 

    0x0ef7a387,// 202 PAY 199 

    0x02a2c025,// 203 PAY 200 

    0x9c57ed21,// 204 PAY 201 

    0xbf4bf1aa,// 205 PAY 202 

    0xd93993e1,// 206 PAY 203 

    0xf34ce29a,// 207 PAY 204 

    0x6efe0d8f,// 208 PAY 205 

    0x60825ff1,// 209 PAY 206 

    0x4f72bf4b,// 210 PAY 207 

    0x3a114a0a,// 211 PAY 208 

    0xbb2cd7da,// 212 PAY 209 

    0xd8d31723,// 213 PAY 210 

    0xdc9db719,// 214 PAY 211 

    0x5a5eae8e,// 215 PAY 212 

    0x1a5e9b9b,// 216 PAY 213 

    0x67009260,// 217 PAY 214 

    0x0b6b5ff5,// 218 PAY 215 

    0x308aa2df,// 219 PAY 216 

    0xe733cc0b,// 220 PAY 217 

    0xa393335d,// 221 PAY 218 

    0x39f9acb3,// 222 PAY 219 

    0x86f13115,// 223 PAY 220 

    0x40b06392,// 224 PAY 221 

    0xaf5851ef,// 225 PAY 222 

    0xff50520a,// 226 PAY 223 

    0xc208aad5,// 227 PAY 224 

    0x51a76c53,// 228 PAY 225 

    0x6615a927,// 229 PAY 226 

    0x90c62b9b,// 230 PAY 227 

    0x5a7b2da5,// 231 PAY 228 

    0x0b720000,// 232 PAY 229 

/// STA is 1 words. 

/// STA num_pkts       : 210 

/// STA pkt_idx        : 251 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1f 

    0x03ed1fd2 // 233 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt82_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 83 words. 

/// BDA size     is 326 (0x146) 

/// BDA id       is 0x8517 

    0x01468517,// 3 BDA   1 

/// PAY Generic Data size   : 326 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe1a2d3a3,// 4 PAY   1 

    0xa4ae1a85,// 5 PAY   2 

    0x715d1c2f,// 6 PAY   3 

    0x527f5a14,// 7 PAY   4 

    0xbdde675b,// 8 PAY   5 

    0x66494114,// 9 PAY   6 

    0x908e957b,// 10 PAY   7 

    0x90c7f957,// 11 PAY   8 

    0x2bf2816d,// 12 PAY   9 

    0x20cdbae3,// 13 PAY  10 

    0x00eda6e9,// 14 PAY  11 

    0x9619b3d5,// 15 PAY  12 

    0xf82c60cd,// 16 PAY  13 

    0x22178c5c,// 17 PAY  14 

    0x882a41ef,// 18 PAY  15 

    0x8adbab1d,// 19 PAY  16 

    0x92e4795e,// 20 PAY  17 

    0x7bab02f1,// 21 PAY  18 

    0xb77e7b5c,// 22 PAY  19 

    0x340b2f28,// 23 PAY  20 

    0x060ae5de,// 24 PAY  21 

    0xb3d968cd,// 25 PAY  22 

    0x5d8249ed,// 26 PAY  23 

    0x15cd0d6f,// 27 PAY  24 

    0xf70ccb11,// 28 PAY  25 

    0xeed59214,// 29 PAY  26 

    0x20e20b4e,// 30 PAY  27 

    0xe685694a,// 31 PAY  28 

    0x57d9e6a2,// 32 PAY  29 

    0xe4ce15a6,// 33 PAY  30 

    0x6870f1b0,// 34 PAY  31 

    0x87f8926a,// 35 PAY  32 

    0x33c84392,// 36 PAY  33 

    0x8afdb54d,// 37 PAY  34 

    0x09487e56,// 38 PAY  35 

    0xffea3e55,// 39 PAY  36 

    0xcd7c8634,// 40 PAY  37 

    0x98968a5c,// 41 PAY  38 

    0x618c12ba,// 42 PAY  39 

    0xdf01f985,// 43 PAY  40 

    0x4ee71f7e,// 44 PAY  41 

    0x3264d1c5,// 45 PAY  42 

    0x4294a60d,// 46 PAY  43 

    0x57639a0f,// 47 PAY  44 

    0x3a42e6ae,// 48 PAY  45 

    0xf623077a,// 49 PAY  46 

    0x4a3ccd68,// 50 PAY  47 

    0x44ae72fe,// 51 PAY  48 

    0xee50ef4a,// 52 PAY  49 

    0x0bcee416,// 53 PAY  50 

    0x69c85e12,// 54 PAY  51 

    0xc125ea04,// 55 PAY  52 

    0x245e8843,// 56 PAY  53 

    0xc09e2ef5,// 57 PAY  54 

    0x986b0f6b,// 58 PAY  55 

    0x84db9c50,// 59 PAY  56 

    0x1297b421,// 60 PAY  57 

    0x395d6c5c,// 61 PAY  58 

    0xbd27924f,// 62 PAY  59 

    0x2cd0ac9e,// 63 PAY  60 

    0x1399f1ae,// 64 PAY  61 

    0x9723f5c7,// 65 PAY  62 

    0xe3c21aa7,// 66 PAY  63 

    0x8da1031b,// 67 PAY  64 

    0x7baef824,// 68 PAY  65 

    0x8243f1b8,// 69 PAY  66 

    0xe4279c7f,// 70 PAY  67 

    0xf6ee366f,// 71 PAY  68 

    0xe9c5ce5d,// 72 PAY  69 

    0xd0d560c1,// 73 PAY  70 

    0x4c024901,// 74 PAY  71 

    0x8a2d8d44,// 75 PAY  72 

    0x7d00b943,// 76 PAY  73 

    0x040e94cd,// 77 PAY  74 

    0x3ab1a727,// 78 PAY  75 

    0x0b2187f4,// 79 PAY  76 

    0xfd052d1c,// 80 PAY  77 

    0x4732be9c,// 81 PAY  78 

    0xd233acf0,// 82 PAY  79 

    0xf685114f,// 83 PAY  80 

    0x4fdbbed9,// 84 PAY  81 

    0xdd120000,// 85 PAY  82 

/// HASH is  12 bytes 

    0x4294a60d,// 86 HSH   1 

    0x57639a0f,// 87 HSH   2 

    0x3a42e6ae,// 88 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 81 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdc 

    0x03dcdc51 // 89 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt83_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 204 words. 

/// BDA size     is 812 (0x32c) 

/// BDA id       is 0x8d6d 

    0x032c8d6d,// 3 BDA   1 

/// PAY Generic Data size   : 812 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x54173ba1,// 4 PAY   1 

    0x1e0218fd,// 5 PAY   2 

    0x13ee3fcd,// 6 PAY   3 

    0xfca0e32d,// 7 PAY   4 

    0x175f4b32,// 8 PAY   5 

    0x557f8214,// 9 PAY   6 

    0x0a073bfb,// 10 PAY   7 

    0x69926f2c,// 11 PAY   8 

    0xef1dc559,// 12 PAY   9 

    0x59f7daed,// 13 PAY  10 

    0x788a96bc,// 14 PAY  11 

    0x671972a9,// 15 PAY  12 

    0x5d9e4d4a,// 16 PAY  13 

    0x3e4da874,// 17 PAY  14 

    0xd11cc43f,// 18 PAY  15 

    0x31a35934,// 19 PAY  16 

    0x12284260,// 20 PAY  17 

    0xdcd293de,// 21 PAY  18 

    0xd0fc72aa,// 22 PAY  19 

    0x71e325df,// 23 PAY  20 

    0xc28521ee,// 24 PAY  21 

    0x6f8c8eb6,// 25 PAY  22 

    0xff633c45,// 26 PAY  23 

    0x56f84a2c,// 27 PAY  24 

    0x1f9705ba,// 28 PAY  25 

    0xd142f485,// 29 PAY  26 

    0x909c79e8,// 30 PAY  27 

    0x6bd046d6,// 31 PAY  28 

    0xd21d2fea,// 32 PAY  29 

    0xe521f8c3,// 33 PAY  30 

    0x88e5a38b,// 34 PAY  31 

    0xd9b92207,// 35 PAY  32 

    0x29eca6e0,// 36 PAY  33 

    0xe6e1a723,// 37 PAY  34 

    0x46e944e2,// 38 PAY  35 

    0xf6a9dfd1,// 39 PAY  36 

    0xfb150425,// 40 PAY  37 

    0x36a1c73d,// 41 PAY  38 

    0x1c12f23b,// 42 PAY  39 

    0x4a7d8386,// 43 PAY  40 

    0xb0444711,// 44 PAY  41 

    0x4a19588c,// 45 PAY  42 

    0x51c819cd,// 46 PAY  43 

    0xd18ca433,// 47 PAY  44 

    0xbaed1e5e,// 48 PAY  45 

    0x23170e0c,// 49 PAY  46 

    0x59f3172a,// 50 PAY  47 

    0xa2599579,// 51 PAY  48 

    0xc2359d5c,// 52 PAY  49 

    0x0a74fcf4,// 53 PAY  50 

    0x23ef5473,// 54 PAY  51 

    0x6e8dfe20,// 55 PAY  52 

    0x3872cea7,// 56 PAY  53 

    0x2c11878e,// 57 PAY  54 

    0xeae37ae2,// 58 PAY  55 

    0xdaa3772a,// 59 PAY  56 

    0x75f9efd1,// 60 PAY  57 

    0xdbe1d59a,// 61 PAY  58 

    0x3f651cdb,// 62 PAY  59 

    0xeba98aa9,// 63 PAY  60 

    0xf34f09ba,// 64 PAY  61 

    0xb02dd938,// 65 PAY  62 

    0x4ea74df8,// 66 PAY  63 

    0x0cf49eef,// 67 PAY  64 

    0x6e816aa8,// 68 PAY  65 

    0x0b32234f,// 69 PAY  66 

    0xee701258,// 70 PAY  67 

    0xc433fd04,// 71 PAY  68 

    0x5260b796,// 72 PAY  69 

    0x052ade5b,// 73 PAY  70 

    0x41a966ad,// 74 PAY  71 

    0x85105bc7,// 75 PAY  72 

    0x7597db59,// 76 PAY  73 

    0xbd960b34,// 77 PAY  74 

    0x9a86b8bd,// 78 PAY  75 

    0x938f18d5,// 79 PAY  76 

    0xa502a7ed,// 80 PAY  77 

    0x1c74eb5a,// 81 PAY  78 

    0x62f273d8,// 82 PAY  79 

    0x37fa9063,// 83 PAY  80 

    0xed9f424a,// 84 PAY  81 

    0xf5a970fe,// 85 PAY  82 

    0x5f165cc4,// 86 PAY  83 

    0xe16a270c,// 87 PAY  84 

    0x0b3c2733,// 88 PAY  85 

    0xd5745bb8,// 89 PAY  86 

    0x0ee00a28,// 90 PAY  87 

    0x06733a55,// 91 PAY  88 

    0x428b7432,// 92 PAY  89 

    0x74d23e01,// 93 PAY  90 

    0x2be56125,// 94 PAY  91 

    0x8a400792,// 95 PAY  92 

    0x028ae596,// 96 PAY  93 

    0xb20875b3,// 97 PAY  94 

    0xc00bf066,// 98 PAY  95 

    0x21477532,// 99 PAY  96 

    0x62261918,// 100 PAY  97 

    0xd33b9a9f,// 101 PAY  98 

    0xd0d3ab74,// 102 PAY  99 

    0xb1d22169,// 103 PAY 100 

    0x13dec11e,// 104 PAY 101 

    0x39318545,// 105 PAY 102 

    0xe6e4505b,// 106 PAY 103 

    0x941ff45e,// 107 PAY 104 

    0x66063f3a,// 108 PAY 105 

    0x150466df,// 109 PAY 106 

    0x181ce2cd,// 110 PAY 107 

    0x4a52a3a8,// 111 PAY 108 

    0x9c7f3a54,// 112 PAY 109 

    0xe52031d9,// 113 PAY 110 

    0xd5d053b7,// 114 PAY 111 

    0xfa09ad09,// 115 PAY 112 

    0x22ccfadc,// 116 PAY 113 

    0xda0417bf,// 117 PAY 114 

    0x791e92a1,// 118 PAY 115 

    0xf21c84b9,// 119 PAY 116 

    0x47343679,// 120 PAY 117 

    0xdc2991b8,// 121 PAY 118 

    0x98003b30,// 122 PAY 119 

    0x60f6bc5b,// 123 PAY 120 

    0x08c3a8b5,// 124 PAY 121 

    0x58926b5d,// 125 PAY 122 

    0xf41cacb8,// 126 PAY 123 

    0x72b746bd,// 127 PAY 124 

    0xd9a0e195,// 128 PAY 125 

    0x0c6769e9,// 129 PAY 126 

    0x9743a9c5,// 130 PAY 127 

    0x5ce5e368,// 131 PAY 128 

    0x81c83c23,// 132 PAY 129 

    0x8558720b,// 133 PAY 130 

    0xde035db5,// 134 PAY 131 

    0x578c71dc,// 135 PAY 132 

    0x0bbb207f,// 136 PAY 133 

    0x971f810c,// 137 PAY 134 

    0x51b19796,// 138 PAY 135 

    0x3451cf20,// 139 PAY 136 

    0xe181a70f,// 140 PAY 137 

    0xf79503ad,// 141 PAY 138 

    0xb3747ef8,// 142 PAY 139 

    0x40bb9587,// 143 PAY 140 

    0x72c92e6e,// 144 PAY 141 

    0xc2a4e70a,// 145 PAY 142 

    0xe75805b1,// 146 PAY 143 

    0x07366d59,// 147 PAY 144 

    0xd1364d5c,// 148 PAY 145 

    0xc948b566,// 149 PAY 146 

    0xaba2c65c,// 150 PAY 147 

    0x9b22ceb0,// 151 PAY 148 

    0x96122060,// 152 PAY 149 

    0x0395e378,// 153 PAY 150 

    0x70cce60e,// 154 PAY 151 

    0xb4817395,// 155 PAY 152 

    0xb4c3760b,// 156 PAY 153 

    0xe11f99cc,// 157 PAY 154 

    0xd6467866,// 158 PAY 155 

    0x72975ebf,// 159 PAY 156 

    0x21322092,// 160 PAY 157 

    0xbbea0325,// 161 PAY 158 

    0x167ed5df,// 162 PAY 159 

    0xd0ad3359,// 163 PAY 160 

    0xd6a5bf45,// 164 PAY 161 

    0x33a8421b,// 165 PAY 162 

    0x006dccbd,// 166 PAY 163 

    0x459835f2,// 167 PAY 164 

    0x67ca43db,// 168 PAY 165 

    0xa36cba42,// 169 PAY 166 

    0x723184f1,// 170 PAY 167 

    0xa482902c,// 171 PAY 168 

    0x1b2b20d1,// 172 PAY 169 

    0x8691c5de,// 173 PAY 170 

    0xce49e9c3,// 174 PAY 171 

    0x42268132,// 175 PAY 172 

    0x783261be,// 176 PAY 173 

    0xa3277dde,// 177 PAY 174 

    0xb6b2c76f,// 178 PAY 175 

    0x4c5e5568,// 179 PAY 176 

    0x951e0a48,// 180 PAY 177 

    0x39559662,// 181 PAY 178 

    0xaed2ddd3,// 182 PAY 179 

    0x78e3fa26,// 183 PAY 180 

    0x2bf77fa6,// 184 PAY 181 

    0xa941e93a,// 185 PAY 182 

    0xc3fad5ba,// 186 PAY 183 

    0xb35004a6,// 187 PAY 184 

    0x2102bb5e,// 188 PAY 185 

    0xd8556deb,// 189 PAY 186 

    0x6e7c3ef5,// 190 PAY 187 

    0x0246f9dd,// 191 PAY 188 

    0x5fdc88b7,// 192 PAY 189 

    0x16be45b1,// 193 PAY 190 

    0xbad4c285,// 194 PAY 191 

    0xf5277b33,// 195 PAY 192 

    0xdf8f7de9,// 196 PAY 193 

    0x30ddfa26,// 197 PAY 194 

    0x9134daaa,// 198 PAY 195 

    0x33fda7f6,// 199 PAY 196 

    0x62d65833,// 200 PAY 197 

    0xc2bc7eea,// 201 PAY 198 

    0xc37dadf8,// 202 PAY 199 

    0x32b3958b,// 203 PAY 200 

    0x5bd124cc,// 204 PAY 201 

    0x40202be2,// 205 PAY 202 

    0x871ca3b1,// 206 PAY 203 

/// HASH is  12 bytes 

    0x5fdc88b7,// 207 HSH   1 

    0x16be45b1,// 208 HSH   2 

    0xbad4c285,// 209 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 206 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x16 

    0x03391649 // 210 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt84_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 175 words. 

/// BDA size     is 694 (0x2b6) 

/// BDA id       is 0x8ef 

    0x02b608ef,// 3 BDA   1 

/// PAY Generic Data size   : 694 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe10f4253,// 4 PAY   1 

    0x2ae8f95d,// 5 PAY   2 

    0x94e2c3cb,// 6 PAY   3 

    0xce1a5858,// 7 PAY   4 

    0x1f66b70a,// 8 PAY   5 

    0x78793d2e,// 9 PAY   6 

    0x96d97e28,// 10 PAY   7 

    0xba7b18c0,// 11 PAY   8 

    0xbd634ce0,// 12 PAY   9 

    0x4a2b40d7,// 13 PAY  10 

    0x2aac08df,// 14 PAY  11 

    0x59188c0f,// 15 PAY  12 

    0x19346625,// 16 PAY  13 

    0x58bf65b2,// 17 PAY  14 

    0x35b584a9,// 18 PAY  15 

    0xd25c1a33,// 19 PAY  16 

    0x8fec958d,// 20 PAY  17 

    0x513bfce8,// 21 PAY  18 

    0xcb4bc9b5,// 22 PAY  19 

    0x345f339a,// 23 PAY  20 

    0xa018fdcb,// 24 PAY  21 

    0xb903aa28,// 25 PAY  22 

    0xbf8045ee,// 26 PAY  23 

    0x8a344f53,// 27 PAY  24 

    0xe596b21a,// 28 PAY  25 

    0x90ef5508,// 29 PAY  26 

    0x4eb40ae2,// 30 PAY  27 

    0x6da42f72,// 31 PAY  28 

    0x6b913555,// 32 PAY  29 

    0x7df7f9ae,// 33 PAY  30 

    0x74a4dd2c,// 34 PAY  31 

    0x547c3890,// 35 PAY  32 

    0x7a554b9a,// 36 PAY  33 

    0x9574de71,// 37 PAY  34 

    0x4c6744f8,// 38 PAY  35 

    0x44763556,// 39 PAY  36 

    0xe3b96274,// 40 PAY  37 

    0xed8e9420,// 41 PAY  38 

    0x14fb0b7a,// 42 PAY  39 

    0xcd71f94a,// 43 PAY  40 

    0xb8866231,// 44 PAY  41 

    0xb826cf93,// 45 PAY  42 

    0x237164d5,// 46 PAY  43 

    0xcc49bf10,// 47 PAY  44 

    0xa5c0ec07,// 48 PAY  45 

    0x323f93ea,// 49 PAY  46 

    0xb1354502,// 50 PAY  47 

    0xd9028538,// 51 PAY  48 

    0xe8dbb344,// 52 PAY  49 

    0x6883436f,// 53 PAY  50 

    0xbb2117f9,// 54 PAY  51 

    0x83960ec6,// 55 PAY  52 

    0x6fb6cf41,// 56 PAY  53 

    0xa431a25c,// 57 PAY  54 

    0x527e83ff,// 58 PAY  55 

    0xd6f94d60,// 59 PAY  56 

    0x7e4a5973,// 60 PAY  57 

    0xd369b1b3,// 61 PAY  58 

    0xbe0c3d70,// 62 PAY  59 

    0xdfde5652,// 63 PAY  60 

    0x9b3b0617,// 64 PAY  61 

    0x606de13b,// 65 PAY  62 

    0x7f4b900b,// 66 PAY  63 

    0x8bdf4526,// 67 PAY  64 

    0x56bc6665,// 68 PAY  65 

    0xec4d3f1c,// 69 PAY  66 

    0x6020e600,// 70 PAY  67 

    0x91151d0a,// 71 PAY  68 

    0xd7afb158,// 72 PAY  69 

    0x17367c4c,// 73 PAY  70 

    0x7cd746d0,// 74 PAY  71 

    0xf9bc1e29,// 75 PAY  72 

    0x2d8e9694,// 76 PAY  73 

    0x92dc16dd,// 77 PAY  74 

    0xd3263e63,// 78 PAY  75 

    0x503b3bbc,// 79 PAY  76 

    0xeb53c9a0,// 80 PAY  77 

    0x7e6ae970,// 81 PAY  78 

    0x499c8b98,// 82 PAY  79 

    0x7e8c3c14,// 83 PAY  80 

    0x34588efd,// 84 PAY  81 

    0x7eee850d,// 85 PAY  82 

    0xb779e92e,// 86 PAY  83 

    0xf1c865cc,// 87 PAY  84 

    0x8b00f71e,// 88 PAY  85 

    0x061cd096,// 89 PAY  86 

    0x234bc349,// 90 PAY  87 

    0x7aab9f50,// 91 PAY  88 

    0x13492f83,// 92 PAY  89 

    0xf6f4459d,// 93 PAY  90 

    0x74ffd6c4,// 94 PAY  91 

    0x5f68788b,// 95 PAY  92 

    0x644f79c8,// 96 PAY  93 

    0x07ab1194,// 97 PAY  94 

    0x340e4d4e,// 98 PAY  95 

    0xed9afba0,// 99 PAY  96 

    0xc2222841,// 100 PAY  97 

    0x11229757,// 101 PAY  98 

    0x96dd8c64,// 102 PAY  99 

    0xf39b1243,// 103 PAY 100 

    0xddc05b09,// 104 PAY 101 

    0xb7f6c6c7,// 105 PAY 102 

    0xd6f92204,// 106 PAY 103 

    0xcbd79456,// 107 PAY 104 

    0x35697ace,// 108 PAY 105 

    0xbea8d551,// 109 PAY 106 

    0x4e312615,// 110 PAY 107 

    0x6985a8d6,// 111 PAY 108 

    0xe61eef37,// 112 PAY 109 

    0x22c4550b,// 113 PAY 110 

    0x5465dccf,// 114 PAY 111 

    0x218a6568,// 115 PAY 112 

    0x7369af7a,// 116 PAY 113 

    0x77cb6eae,// 117 PAY 114 

    0xb821e141,// 118 PAY 115 

    0x96007781,// 119 PAY 116 

    0x85205458,// 120 PAY 117 

    0x9006be85,// 121 PAY 118 

    0x56857217,// 122 PAY 119 

    0x4cea4010,// 123 PAY 120 

    0x6c78a71d,// 124 PAY 121 

    0x59206219,// 125 PAY 122 

    0xfa5dc3ff,// 126 PAY 123 

    0x9a5247a1,// 127 PAY 124 

    0xab798c61,// 128 PAY 125 

    0xf767aa5c,// 129 PAY 126 

    0x9f8daa28,// 130 PAY 127 

    0xc41cb42b,// 131 PAY 128 

    0x6b8df4df,// 132 PAY 129 

    0xc616ed27,// 133 PAY 130 

    0x2b10a9d7,// 134 PAY 131 

    0xcef4b446,// 135 PAY 132 

    0x29ba26fa,// 136 PAY 133 

    0xb90eeb08,// 137 PAY 134 

    0x3f04a85f,// 138 PAY 135 

    0x552d62ee,// 139 PAY 136 

    0x66129fcc,// 140 PAY 137 

    0x02419e0b,// 141 PAY 138 

    0x47b95783,// 142 PAY 139 

    0xfa32af51,// 143 PAY 140 

    0xb14d7c81,// 144 PAY 141 

    0x61687170,// 145 PAY 142 

    0x4a9e0853,// 146 PAY 143 

    0x94ede532,// 147 PAY 144 

    0x66aede3a,// 148 PAY 145 

    0x4241ad8f,// 149 PAY 146 

    0x84cf2841,// 150 PAY 147 

    0x9ba7bd90,// 151 PAY 148 

    0xb3abbbd3,// 152 PAY 149 

    0x1cb90bd4,// 153 PAY 150 

    0x67394d0e,// 154 PAY 151 

    0x3d8f4c04,// 155 PAY 152 

    0xc2c97f0f,// 156 PAY 153 

    0x57b3f1eb,// 157 PAY 154 

    0xa365d9ec,// 158 PAY 155 

    0xc4c41321,// 159 PAY 156 

    0x4ab6053f,// 160 PAY 157 

    0x52957e0b,// 161 PAY 158 

    0x5266814b,// 162 PAY 159 

    0x8af58d99,// 163 PAY 160 

    0xaea5079d,// 164 PAY 161 

    0x70e2b4a4,// 165 PAY 162 

    0x7eece6dc,// 166 PAY 163 

    0xc29d49a2,// 167 PAY 164 

    0xf6e04cec,// 168 PAY 165 

    0xb7555d24,// 169 PAY 166 

    0xa6b27ff6,// 170 PAY 167 

    0x3e555b96,// 171 PAY 168 

    0xce1d0564,// 172 PAY 169 

    0x4b364e61,// 173 PAY 170 

    0x78154c8c,// 174 PAY 171 

    0x74f7486f,// 175 PAY 172 

    0x0620c2f1,// 176 PAY 173 

    0x195c0000,// 177 PAY 174 

/// STA is 1 words. 

/// STA num_pkts       : 53 

/// STA pkt_idx        : 49 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb4 

    0x00c4b435 // 178 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt85_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 426 words. 

/// BDA size     is 1697 (0x6a1) 

/// BDA id       is 0xca9 

    0x06a10ca9,// 3 BDA   1 

/// PAY Generic Data size   : 1697 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x8630a9b3,// 4 PAY   1 

    0x24c6f4d3,// 5 PAY   2 

    0x21de246a,// 6 PAY   3 

    0xf20b2c33,// 7 PAY   4 

    0xfb044540,// 8 PAY   5 

    0x90b8f949,// 9 PAY   6 

    0x66bd8f66,// 10 PAY   7 

    0xec60668c,// 11 PAY   8 

    0x813d9c4d,// 12 PAY   9 

    0xdc208dad,// 13 PAY  10 

    0xaddc71b7,// 14 PAY  11 

    0x4860682b,// 15 PAY  12 

    0x39b759a7,// 16 PAY  13 

    0x9d3f4e8a,// 17 PAY  14 

    0x4e6c6bfc,// 18 PAY  15 

    0xa783a986,// 19 PAY  16 

    0x68953716,// 20 PAY  17 

    0xa66ad17d,// 21 PAY  18 

    0xde61c72a,// 22 PAY  19 

    0xc6da347f,// 23 PAY  20 

    0xeded06ab,// 24 PAY  21 

    0xdb043257,// 25 PAY  22 

    0x6b9f2490,// 26 PAY  23 

    0xbda9e4f9,// 27 PAY  24 

    0x333192f6,// 28 PAY  25 

    0xd7a59b53,// 29 PAY  26 

    0xffceb69a,// 30 PAY  27 

    0x87aa0bd0,// 31 PAY  28 

    0xfdb4c8db,// 32 PAY  29 

    0xdbee72e5,// 33 PAY  30 

    0x743de8e7,// 34 PAY  31 

    0xdee7e5fc,// 35 PAY  32 

    0xaafc315a,// 36 PAY  33 

    0xbb91153f,// 37 PAY  34 

    0x374a800f,// 38 PAY  35 

    0x979448c0,// 39 PAY  36 

    0x60e38f65,// 40 PAY  37 

    0x0a55a70a,// 41 PAY  38 

    0xb1316089,// 42 PAY  39 

    0xac0f7394,// 43 PAY  40 

    0xa1d94a8a,// 44 PAY  41 

    0x2a29aa5b,// 45 PAY  42 

    0x5b9adce0,// 46 PAY  43 

    0xdf7b2316,// 47 PAY  44 

    0x4e29b2c1,// 48 PAY  45 

    0x97440b87,// 49 PAY  46 

    0x38efe35d,// 50 PAY  47 

    0x92070a0d,// 51 PAY  48 

    0xc228f703,// 52 PAY  49 

    0x7dc16c6f,// 53 PAY  50 

    0x46223e77,// 54 PAY  51 

    0xcc75e613,// 55 PAY  52 

    0x77835261,// 56 PAY  53 

    0x806c5f95,// 57 PAY  54 

    0x22e43d85,// 58 PAY  55 

    0x70fc2e54,// 59 PAY  56 

    0xac05eb29,// 60 PAY  57 

    0x5848bd5d,// 61 PAY  58 

    0x1b5d5353,// 62 PAY  59 

    0x83bc6ef5,// 63 PAY  60 

    0xf9127a87,// 64 PAY  61 

    0xf484b714,// 65 PAY  62 

    0x895a8c40,// 66 PAY  63 

    0x14fcd826,// 67 PAY  64 

    0x10e0490d,// 68 PAY  65 

    0x835e92d4,// 69 PAY  66 

    0xe933d85f,// 70 PAY  67 

    0x493eb7d0,// 71 PAY  68 

    0x904639dd,// 72 PAY  69 

    0x1688936c,// 73 PAY  70 

    0x70ee9107,// 74 PAY  71 

    0x01072573,// 75 PAY  72 

    0xb7bcd260,// 76 PAY  73 

    0xe60f0a7e,// 77 PAY  74 

    0xdbaeb255,// 78 PAY  75 

    0xa4096421,// 79 PAY  76 

    0xac6029dd,// 80 PAY  77 

    0xf1ea51b2,// 81 PAY  78 

    0xc9188ffd,// 82 PAY  79 

    0x0c9f8727,// 83 PAY  80 

    0x00656b1a,// 84 PAY  81 

    0x43f0e599,// 85 PAY  82 

    0x6a607772,// 86 PAY  83 

    0x83fec90f,// 87 PAY  84 

    0xed462986,// 88 PAY  85 

    0x094ad044,// 89 PAY  86 

    0xf486f5f3,// 90 PAY  87 

    0xebfe45a7,// 91 PAY  88 

    0xdd46f8b0,// 92 PAY  89 

    0x65f92fb8,// 93 PAY  90 

    0x15294aba,// 94 PAY  91 

    0xa58bda9c,// 95 PAY  92 

    0x79e30939,// 96 PAY  93 

    0x8121e6df,// 97 PAY  94 

    0x0536d802,// 98 PAY  95 

    0xf566336d,// 99 PAY  96 

    0x66c3f1d7,// 100 PAY  97 

    0x6b3e033a,// 101 PAY  98 

    0x2630235c,// 102 PAY  99 

    0x0a8de052,// 103 PAY 100 

    0xb0f5839f,// 104 PAY 101 

    0x74ecd3f9,// 105 PAY 102 

    0xb9f1107e,// 106 PAY 103 

    0x72976232,// 107 PAY 104 

    0xabba1d88,// 108 PAY 105 

    0xc4fd2844,// 109 PAY 106 

    0x3f83bb36,// 110 PAY 107 

    0xc41cfef7,// 111 PAY 108 

    0x7dcbe89a,// 112 PAY 109 

    0x540a0950,// 113 PAY 110 

    0x2cd8ffde,// 114 PAY 111 

    0xc74db79e,// 115 PAY 112 

    0xb5fa3de9,// 116 PAY 113 

    0x19827b69,// 117 PAY 114 

    0x96c41506,// 118 PAY 115 

    0x820d6678,// 119 PAY 116 

    0x04a0a223,// 120 PAY 117 

    0xc3a93982,// 121 PAY 118 

    0x1253e6ed,// 122 PAY 119 

    0xf15de90b,// 123 PAY 120 

    0x3b432aa4,// 124 PAY 121 

    0x31f51ee4,// 125 PAY 122 

    0xe7ee56c5,// 126 PAY 123 

    0x8632944c,// 127 PAY 124 

    0x50098d22,// 128 PAY 125 

    0x4d9eeac6,// 129 PAY 126 

    0x0175df5d,// 130 PAY 127 

    0xed226395,// 131 PAY 128 

    0xe6cc5d9d,// 132 PAY 129 

    0xe262a35d,// 133 PAY 130 

    0x2a05f414,// 134 PAY 131 

    0x91e04cbe,// 135 PAY 132 

    0x744de4b8,// 136 PAY 133 

    0xdd0714c3,// 137 PAY 134 

    0xae680de0,// 138 PAY 135 

    0x8fc7b716,// 139 PAY 136 

    0x75b4eb9e,// 140 PAY 137 

    0x501dc5f1,// 141 PAY 138 

    0x564a30e5,// 142 PAY 139 

    0xfb5cfda3,// 143 PAY 140 

    0x5cdb57dd,// 144 PAY 141 

    0x90001c47,// 145 PAY 142 

    0x32483098,// 146 PAY 143 

    0x3da4adc3,// 147 PAY 144 

    0xec4b5bb1,// 148 PAY 145 

    0xd99d625f,// 149 PAY 146 

    0x9099b849,// 150 PAY 147 

    0x9483d326,// 151 PAY 148 

    0x8180b450,// 152 PAY 149 

    0x22c47346,// 153 PAY 150 

    0xe146635f,// 154 PAY 151 

    0x89d2da83,// 155 PAY 152 

    0x1f6dba17,// 156 PAY 153 

    0xe86c2b07,// 157 PAY 154 

    0x10c0fe65,// 158 PAY 155 

    0x8b34e1ca,// 159 PAY 156 

    0x0df221e4,// 160 PAY 157 

    0x33fb03d5,// 161 PAY 158 

    0x1b51fc89,// 162 PAY 159 

    0xec072aeb,// 163 PAY 160 

    0x90b039d5,// 164 PAY 161 

    0xb3954bfb,// 165 PAY 162 

    0x8dd28ec7,// 166 PAY 163 

    0x1ff7ced1,// 167 PAY 164 

    0x1ee7888a,// 168 PAY 165 

    0xd29f717a,// 169 PAY 166 

    0x063f512e,// 170 PAY 167 

    0xc0de4218,// 171 PAY 168 

    0x780dae5a,// 172 PAY 169 

    0x2450aadb,// 173 PAY 170 

    0xebf2c28c,// 174 PAY 171 

    0x499eeb91,// 175 PAY 172 

    0x68bf1f2f,// 176 PAY 173 

    0x97e8ef3e,// 177 PAY 174 

    0x7b31de36,// 178 PAY 175 

    0xa68e54aa,// 179 PAY 176 

    0x1e436caf,// 180 PAY 177 

    0xe8866f52,// 181 PAY 178 

    0x75a6638c,// 182 PAY 179 

    0x6d2eb596,// 183 PAY 180 

    0x6b12e1e1,// 184 PAY 181 

    0x334280fc,// 185 PAY 182 

    0x120edeff,// 186 PAY 183 

    0x21a7a5d1,// 187 PAY 184 

    0x134ee840,// 188 PAY 185 

    0x3e333d85,// 189 PAY 186 

    0x0ba6865f,// 190 PAY 187 

    0xccbd015f,// 191 PAY 188 

    0x97c77712,// 192 PAY 189 

    0x89e61caa,// 193 PAY 190 

    0x8a978b5e,// 194 PAY 191 

    0x571825aa,// 195 PAY 192 

    0x197a4240,// 196 PAY 193 

    0xef85175a,// 197 PAY 194 

    0xcc41ec6e,// 198 PAY 195 

    0x22bd4783,// 199 PAY 196 

    0x53555b98,// 200 PAY 197 

    0x47fcd24d,// 201 PAY 198 

    0x9d3f0630,// 202 PAY 199 

    0x02dce6d8,// 203 PAY 200 

    0xdfdfdef0,// 204 PAY 201 

    0x4166b5a4,// 205 PAY 202 

    0x05a43c5b,// 206 PAY 203 

    0x7f64a1d3,// 207 PAY 204 

    0x2ea6205a,// 208 PAY 205 

    0x3dfc156f,// 209 PAY 206 

    0xf7fbba5e,// 210 PAY 207 

    0xd38d8990,// 211 PAY 208 

    0xd17981e4,// 212 PAY 209 

    0xa27b794d,// 213 PAY 210 

    0x4676f912,// 214 PAY 211 

    0x42cb9c40,// 215 PAY 212 

    0x4772b27e,// 216 PAY 213 

    0x8bfea91e,// 217 PAY 214 

    0xf55f1724,// 218 PAY 215 

    0x8b9bbe96,// 219 PAY 216 

    0x8562c330,// 220 PAY 217 

    0x8262ef5a,// 221 PAY 218 

    0x91d84f48,// 222 PAY 219 

    0xbe18e47d,// 223 PAY 220 

    0xe36f344e,// 224 PAY 221 

    0xe655866b,// 225 PAY 222 

    0xfb2fde75,// 226 PAY 223 

    0x62abcc65,// 227 PAY 224 

    0x0da6ced8,// 228 PAY 225 

    0x571998c7,// 229 PAY 226 

    0xa6267136,// 230 PAY 227 

    0x426a1ee4,// 231 PAY 228 

    0x258790aa,// 232 PAY 229 

    0x9b668ec1,// 233 PAY 230 

    0xef2407c4,// 234 PAY 231 

    0x424cd6bf,// 235 PAY 232 

    0x1db1feb3,// 236 PAY 233 

    0x820689a2,// 237 PAY 234 

    0x6dcee281,// 238 PAY 235 

    0x260c2207,// 239 PAY 236 

    0x75960025,// 240 PAY 237 

    0xa1777cc2,// 241 PAY 238 

    0x154392d5,// 242 PAY 239 

    0x29458cf7,// 243 PAY 240 

    0x83df5766,// 244 PAY 241 

    0x86a083b6,// 245 PAY 242 

    0xdfc697b8,// 246 PAY 243 

    0x903d06e6,// 247 PAY 244 

    0x689ae8c3,// 248 PAY 245 

    0xf59ba270,// 249 PAY 246 

    0xd1df899e,// 250 PAY 247 

    0x3b3ec44f,// 251 PAY 248 

    0xeb9f7ac9,// 252 PAY 249 

    0x119eb4ad,// 253 PAY 250 

    0xbe3c9ab6,// 254 PAY 251 

    0xa330ca22,// 255 PAY 252 

    0xacd6800e,// 256 PAY 253 

    0x92823421,// 257 PAY 254 

    0x576165bf,// 258 PAY 255 

    0xb7ba9853,// 259 PAY 256 

    0xe2d8f521,// 260 PAY 257 

    0x571e4c47,// 261 PAY 258 

    0x39e00c39,// 262 PAY 259 

    0x774f258b,// 263 PAY 260 

    0xca81a2c9,// 264 PAY 261 

    0x03f73255,// 265 PAY 262 

    0x5b8cdfba,// 266 PAY 263 

    0x8c09a786,// 267 PAY 264 

    0x0a5f91da,// 268 PAY 265 

    0x7ed251ab,// 269 PAY 266 

    0x73caa0af,// 270 PAY 267 

    0xb4030c5a,// 271 PAY 268 

    0x8c67282b,// 272 PAY 269 

    0xafcfcee9,// 273 PAY 270 

    0x06fa06c8,// 274 PAY 271 

    0x12d544df,// 275 PAY 272 

    0x4f1b11e7,// 276 PAY 273 

    0xc8049043,// 277 PAY 274 

    0xe790659b,// 278 PAY 275 

    0x1fc86d77,// 279 PAY 276 

    0xecb3d902,// 280 PAY 277 

    0xa65fe930,// 281 PAY 278 

    0x6df8be46,// 282 PAY 279 

    0x45fe91bd,// 283 PAY 280 

    0xe9cd6111,// 284 PAY 281 

    0x9d745a77,// 285 PAY 282 

    0x32bf0553,// 286 PAY 283 

    0x0eb7c3d9,// 287 PAY 284 

    0x13134e98,// 288 PAY 285 

    0x5cdfa052,// 289 PAY 286 

    0x039ef2b5,// 290 PAY 287 

    0x71a50376,// 291 PAY 288 

    0xa417a849,// 292 PAY 289 

    0xe3e586b0,// 293 PAY 290 

    0x97b3e520,// 294 PAY 291 

    0xa1541371,// 295 PAY 292 

    0xa92f0e5d,// 296 PAY 293 

    0xbfeb1183,// 297 PAY 294 

    0x54db508c,// 298 PAY 295 

    0xf97a3e6e,// 299 PAY 296 

    0x41b866d4,// 300 PAY 297 

    0xe170b017,// 301 PAY 298 

    0x8ea8cd41,// 302 PAY 299 

    0xb3a3a48e,// 303 PAY 300 

    0xc31b5bb4,// 304 PAY 301 

    0x60a486db,// 305 PAY 302 

    0xe3abeb8e,// 306 PAY 303 

    0x014cdf92,// 307 PAY 304 

    0x147cb431,// 308 PAY 305 

    0xdd6ac5d4,// 309 PAY 306 

    0x1620da8a,// 310 PAY 307 

    0x32026866,// 311 PAY 308 

    0x82606a2f,// 312 PAY 309 

    0x9a86a02e,// 313 PAY 310 

    0x0aee8706,// 314 PAY 311 

    0x12524680,// 315 PAY 312 

    0x7b24a466,// 316 PAY 313 

    0xfef024f4,// 317 PAY 314 

    0x774c5883,// 318 PAY 315 

    0xb5dcb50d,// 319 PAY 316 

    0x9bd9d9f7,// 320 PAY 317 

    0x109d3743,// 321 PAY 318 

    0xf4c4f146,// 322 PAY 319 

    0xe042fd4d,// 323 PAY 320 

    0x48181323,// 324 PAY 321 

    0x492d1c7c,// 325 PAY 322 

    0x387d2b02,// 326 PAY 323 

    0x75c6695f,// 327 PAY 324 

    0x78757465,// 328 PAY 325 

    0x9e5b134f,// 329 PAY 326 

    0xb5a5075a,// 330 PAY 327 

    0x6da792bb,// 331 PAY 328 

    0x8aeeea73,// 332 PAY 329 

    0xef8c8912,// 333 PAY 330 

    0xd399f54b,// 334 PAY 331 

    0x9788b692,// 335 PAY 332 

    0x2179641c,// 336 PAY 333 

    0x30b3cdd4,// 337 PAY 334 

    0x29ed8b29,// 338 PAY 335 

    0xe64db0ad,// 339 PAY 336 

    0x21b87157,// 340 PAY 337 

    0x11f47e82,// 341 PAY 338 

    0xda0b5fab,// 342 PAY 339 

    0x40f67a1d,// 343 PAY 340 

    0xc9a2f241,// 344 PAY 341 

    0x0dd11071,// 345 PAY 342 

    0x06aed9b4,// 346 PAY 343 

    0x726d2d03,// 347 PAY 344 

    0xc47673c9,// 348 PAY 345 

    0xcc59a4a7,// 349 PAY 346 

    0x73192f54,// 350 PAY 347 

    0x000da121,// 351 PAY 348 

    0xc33560be,// 352 PAY 349 

    0xaae61224,// 353 PAY 350 

    0x1f2b64bc,// 354 PAY 351 

    0xd5b53040,// 355 PAY 352 

    0xd024af82,// 356 PAY 353 

    0xef952c87,// 357 PAY 354 

    0x6816a51b,// 358 PAY 355 

    0x5035b8b6,// 359 PAY 356 

    0x33f10082,// 360 PAY 357 

    0x5750db5c,// 361 PAY 358 

    0x67c9e0c6,// 362 PAY 359 

    0xb58baee6,// 363 PAY 360 

    0x1e10d6e4,// 364 PAY 361 

    0x25250408,// 365 PAY 362 

    0xc9b4519f,// 366 PAY 363 

    0x9b0444cb,// 367 PAY 364 

    0xd0ca6e8b,// 368 PAY 365 

    0xae36a819,// 369 PAY 366 

    0xd6103b82,// 370 PAY 367 

    0x5d52e677,// 371 PAY 368 

    0xc0a452a7,// 372 PAY 369 

    0xc0d19747,// 373 PAY 370 

    0xfa57e876,// 374 PAY 371 

    0xa277d43b,// 375 PAY 372 

    0x3bff6f37,// 376 PAY 373 

    0x2a5f964e,// 377 PAY 374 

    0x2507a826,// 378 PAY 375 

    0xb7d0e500,// 379 PAY 376 

    0x7bf4b088,// 380 PAY 377 

    0x0609841f,// 381 PAY 378 

    0xe234c2b5,// 382 PAY 379 

    0x4c821848,// 383 PAY 380 

    0x17a3f34d,// 384 PAY 381 

    0x7cf9d7d1,// 385 PAY 382 

    0x9b505a4e,// 386 PAY 383 

    0x083226c7,// 387 PAY 384 

    0xffbaee42,// 388 PAY 385 

    0x7d9d42a7,// 389 PAY 386 

    0x85aa3acb,// 390 PAY 387 

    0xf919bd6c,// 391 PAY 388 

    0x363f1783,// 392 PAY 389 

    0x9964b028,// 393 PAY 390 

    0x04369646,// 394 PAY 391 

    0xfa283c81,// 395 PAY 392 

    0xea01ac82,// 396 PAY 393 

    0xc9cf23ba,// 397 PAY 394 

    0x0180a019,// 398 PAY 395 

    0xcc07765e,// 399 PAY 396 

    0xfbbb592b,// 400 PAY 397 

    0xad6b741b,// 401 PAY 398 

    0x7635959b,// 402 PAY 399 

    0x0fbe3742,// 403 PAY 400 

    0xb9201b2b,// 404 PAY 401 

    0x1d911ddc,// 405 PAY 402 

    0x529861aa,// 406 PAY 403 

    0x06e1b8f2,// 407 PAY 404 

    0x4c060a67,// 408 PAY 405 

    0x19698aff,// 409 PAY 406 

    0x4071079d,// 410 PAY 407 

    0xd8909d44,// 411 PAY 408 

    0x6207bf8d,// 412 PAY 409 

    0x456eb2d9,// 413 PAY 410 

    0x6a0d2ef7,// 414 PAY 411 

    0x0639f247,// 415 PAY 412 

    0xc6203919,// 416 PAY 413 

    0x92439db2,// 417 PAY 414 

    0x593ef207,// 418 PAY 415 

    0x2afc7a8e,// 419 PAY 416 

    0xb6f6f6bd,// 420 PAY 417 

    0xe34487e3,// 421 PAY 418 

    0xd778adaa,// 422 PAY 419 

    0xdc418264,// 423 PAY 420 

    0x208ec0e6,// 424 PAY 421 

    0x288f6375,// 425 PAY 422 

    0x2b92bd27,// 426 PAY 423 

    0xa83344c5,// 427 PAY 424 

    0x0c000000,// 428 PAY 425 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5e 

    0x00d55e7e // 429 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt86_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 471 words. 

/// BDA size     is 1878 (0x756) 

/// BDA id       is 0xf6c2 

    0x0756f6c2,// 3 BDA   1 

/// PAY Generic Data size   : 1878 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x09a8b1f0,// 4 PAY   1 

    0xa57bc893,// 5 PAY   2 

    0xd65ccd4f,// 6 PAY   3 

    0x54ddfc19,// 7 PAY   4 

    0x39270df9,// 8 PAY   5 

    0xf131cc05,// 9 PAY   6 

    0xecee8879,// 10 PAY   7 

    0x89dc36ab,// 11 PAY   8 

    0x628662ec,// 12 PAY   9 

    0x031dfac0,// 13 PAY  10 

    0xae53103f,// 14 PAY  11 

    0xd11c2d57,// 15 PAY  12 

    0xd6b5bc72,// 16 PAY  13 

    0xe9be4e0b,// 17 PAY  14 

    0x9fb90f15,// 18 PAY  15 

    0x50542298,// 19 PAY  16 

    0xbb1c5a00,// 20 PAY  17 

    0x4f18c14d,// 21 PAY  18 

    0x7ac76383,// 22 PAY  19 

    0xa66c5d84,// 23 PAY  20 

    0x6dd6cd5e,// 24 PAY  21 

    0x2bf52a36,// 25 PAY  22 

    0xa9605e18,// 26 PAY  23 

    0x5f79db21,// 27 PAY  24 

    0xeb3ddab6,// 28 PAY  25 

    0xefc0843c,// 29 PAY  26 

    0xe087e531,// 30 PAY  27 

    0x0521624c,// 31 PAY  28 

    0x40f71004,// 32 PAY  29 

    0x1a917702,// 33 PAY  30 

    0x8ce42a72,// 34 PAY  31 

    0x7b83c0de,// 35 PAY  32 

    0x5091c6e6,// 36 PAY  33 

    0x142d92cb,// 37 PAY  34 

    0xd61340e9,// 38 PAY  35 

    0x3cebe66d,// 39 PAY  36 

    0x7c3c696f,// 40 PAY  37 

    0x5015a777,// 41 PAY  38 

    0xacfd305f,// 42 PAY  39 

    0x542372db,// 43 PAY  40 

    0xa0a8ca13,// 44 PAY  41 

    0x632445df,// 45 PAY  42 

    0xe2212179,// 46 PAY  43 

    0xda4d9b93,// 47 PAY  44 

    0x234cd80a,// 48 PAY  45 

    0xb31a4e57,// 49 PAY  46 

    0xef82d759,// 50 PAY  47 

    0xb7506ca5,// 51 PAY  48 

    0xf1488cd5,// 52 PAY  49 

    0x45abd8a7,// 53 PAY  50 

    0x15719346,// 54 PAY  51 

    0x222bf00e,// 55 PAY  52 

    0x255ea1e3,// 56 PAY  53 

    0x7f6522b4,// 57 PAY  54 

    0xd0ba28c7,// 58 PAY  55 

    0xae81167d,// 59 PAY  56 

    0x26c9bd9e,// 60 PAY  57 

    0x1a2ff1d0,// 61 PAY  58 

    0xd34d662b,// 62 PAY  59 

    0x6fa2bcd1,// 63 PAY  60 

    0xcdd8daf1,// 64 PAY  61 

    0xc6f708f5,// 65 PAY  62 

    0x10dea01f,// 66 PAY  63 

    0xfa8dd99a,// 67 PAY  64 

    0x37d6ae9b,// 68 PAY  65 

    0xa3726dfa,// 69 PAY  66 

    0xe776d9e8,// 70 PAY  67 

    0xa55a182c,// 71 PAY  68 

    0x8251f8bc,// 72 PAY  69 

    0xea073ae0,// 73 PAY  70 

    0xcf467786,// 74 PAY  71 

    0x5e22447e,// 75 PAY  72 

    0x24536fe4,// 76 PAY  73 

    0xa429381c,// 77 PAY  74 

    0xe7420b71,// 78 PAY  75 

    0x68cab541,// 79 PAY  76 

    0x877b0940,// 80 PAY  77 

    0xaf3e2939,// 81 PAY  78 

    0xe3171d9f,// 82 PAY  79 

    0xef0f4954,// 83 PAY  80 

    0x44aafa61,// 84 PAY  81 

    0xd7383611,// 85 PAY  82 

    0xe4eae968,// 86 PAY  83 

    0xf6e65d1b,// 87 PAY  84 

    0xce4365b8,// 88 PAY  85 

    0x075259ba,// 89 PAY  86 

    0x0d9e1b46,// 90 PAY  87 

    0xa2702406,// 91 PAY  88 

    0x90a47cda,// 92 PAY  89 

    0x81b54f81,// 93 PAY  90 

    0x98d8c1d7,// 94 PAY  91 

    0xeb86962f,// 95 PAY  92 

    0xb7c868ba,// 96 PAY  93 

    0x424015af,// 97 PAY  94 

    0x9ef57be3,// 98 PAY  95 

    0x3a30efcf,// 99 PAY  96 

    0x03a36867,// 100 PAY  97 

    0xd7c677b1,// 101 PAY  98 

    0x618ad9ef,// 102 PAY  99 

    0x90849c0d,// 103 PAY 100 

    0x66244ac6,// 104 PAY 101 

    0xe9732606,// 105 PAY 102 

    0x9983b722,// 106 PAY 103 

    0x2a7b5f81,// 107 PAY 104 

    0x1aab36c1,// 108 PAY 105 

    0x48ef4374,// 109 PAY 106 

    0x6c410bc9,// 110 PAY 107 

    0x5d785159,// 111 PAY 108 

    0xb6fcaeec,// 112 PAY 109 

    0x2874d6d2,// 113 PAY 110 

    0x9388afae,// 114 PAY 111 

    0x9c06f89b,// 115 PAY 112 

    0x1436041e,// 116 PAY 113 

    0x5f323546,// 117 PAY 114 

    0x6db26ffe,// 118 PAY 115 

    0x7a7d1142,// 119 PAY 116 

    0x573ad501,// 120 PAY 117 

    0x55c48c3e,// 121 PAY 118 

    0x90b201f0,// 122 PAY 119 

    0xd71ced08,// 123 PAY 120 

    0xdf57097b,// 124 PAY 121 

    0xdf03c75c,// 125 PAY 122 

    0xc4958227,// 126 PAY 123 

    0x4e7b0b24,// 127 PAY 124 

    0xd1591df9,// 128 PAY 125 

    0x2b50cc85,// 129 PAY 126 

    0xd04a8dee,// 130 PAY 127 

    0x6f551bfc,// 131 PAY 128 

    0x312c22a2,// 132 PAY 129 

    0x5f6487f7,// 133 PAY 130 

    0xa95c2b27,// 134 PAY 131 

    0xd6a090d3,// 135 PAY 132 

    0x300336e9,// 136 PAY 133 

    0x48ccc1d4,// 137 PAY 134 

    0xcdc9cc4a,// 138 PAY 135 

    0x3d07a8bf,// 139 PAY 136 

    0x38ce78f9,// 140 PAY 137 

    0xfdbde2c5,// 141 PAY 138 

    0x08cc39b9,// 142 PAY 139 

    0x15a784fe,// 143 PAY 140 

    0x52e241d1,// 144 PAY 141 

    0xfe25cf21,// 145 PAY 142 

    0x106bf8ff,// 146 PAY 143 

    0x9545fe67,// 147 PAY 144 

    0xc10e3010,// 148 PAY 145 

    0x46215938,// 149 PAY 146 

    0x6970656f,// 150 PAY 147 

    0x22762de5,// 151 PAY 148 

    0x94f4f586,// 152 PAY 149 

    0x72ae24b9,// 153 PAY 150 

    0xf459872a,// 154 PAY 151 

    0x7d315f92,// 155 PAY 152 

    0x3ecff658,// 156 PAY 153 

    0x8b1ed277,// 157 PAY 154 

    0x2c9f28bf,// 158 PAY 155 

    0x67224d70,// 159 PAY 156 

    0x2ba53ea8,// 160 PAY 157 

    0x4fa7b2fe,// 161 PAY 158 

    0x23706568,// 162 PAY 159 

    0x1f115f8f,// 163 PAY 160 

    0xd8213141,// 164 PAY 161 

    0x296970e7,// 165 PAY 162 

    0xbb26b273,// 166 PAY 163 

    0x5c7dccd3,// 167 PAY 164 

    0xd7074505,// 168 PAY 165 

    0x64bb8260,// 169 PAY 166 

    0xfb7b61a5,// 170 PAY 167 

    0x6d18e71c,// 171 PAY 168 

    0x4bed136e,// 172 PAY 169 

    0x65fe6e89,// 173 PAY 170 

    0x9c38b614,// 174 PAY 171 

    0xaea91b83,// 175 PAY 172 

    0x1b4e8354,// 176 PAY 173 

    0xca562a9f,// 177 PAY 174 

    0xb5acaba5,// 178 PAY 175 

    0x8cdfd195,// 179 PAY 176 

    0x0cb49b9f,// 180 PAY 177 

    0x75506a67,// 181 PAY 178 

    0x2fa0c03e,// 182 PAY 179 

    0x6fdb5d15,// 183 PAY 180 

    0xec14b5e3,// 184 PAY 181 

    0xadfc8646,// 185 PAY 182 

    0xf8d72be6,// 186 PAY 183 

    0x1fb604a6,// 187 PAY 184 

    0xbd099377,// 188 PAY 185 

    0x8570476f,// 189 PAY 186 

    0x9e41a4be,// 190 PAY 187 

    0x1f2456b6,// 191 PAY 188 

    0x6bef465d,// 192 PAY 189 

    0x3902e3af,// 193 PAY 190 

    0x146ac188,// 194 PAY 191 

    0xa2328085,// 195 PAY 192 

    0xcc717c82,// 196 PAY 193 

    0x8e8ff33a,// 197 PAY 194 

    0xcc2be959,// 198 PAY 195 

    0x4aa78862,// 199 PAY 196 

    0xaa65d0c7,// 200 PAY 197 

    0x4dd38359,// 201 PAY 198 

    0x14d019f4,// 202 PAY 199 

    0x58da49db,// 203 PAY 200 

    0xb2493d55,// 204 PAY 201 

    0x33acfd61,// 205 PAY 202 

    0x4314a7aa,// 206 PAY 203 

    0xe5078c3a,// 207 PAY 204 

    0x6b206c87,// 208 PAY 205 

    0xc438eab4,// 209 PAY 206 

    0x814f7d5b,// 210 PAY 207 

    0x7bedd127,// 211 PAY 208 

    0x7d444916,// 212 PAY 209 

    0x12e88596,// 213 PAY 210 

    0xe8c44fef,// 214 PAY 211 

    0xa29fe760,// 215 PAY 212 

    0x15a7a838,// 216 PAY 213 

    0x8b13da43,// 217 PAY 214 

    0xc32eed36,// 218 PAY 215 

    0xa573687f,// 219 PAY 216 

    0x15282701,// 220 PAY 217 

    0xf7538272,// 221 PAY 218 

    0x620c838c,// 222 PAY 219 

    0x4dabcb74,// 223 PAY 220 

    0x68487a7e,// 224 PAY 221 

    0xd674f18b,// 225 PAY 222 

    0x9b541754,// 226 PAY 223 

    0x3ce792bf,// 227 PAY 224 

    0x64b2b28b,// 228 PAY 225 

    0x93fbdd5e,// 229 PAY 226 

    0x4181a0f1,// 230 PAY 227 

    0x6d79fea7,// 231 PAY 228 

    0xc9c2104d,// 232 PAY 229 

    0xaaf00fbe,// 233 PAY 230 

    0x37d12b04,// 234 PAY 231 

    0xdb9356b1,// 235 PAY 232 

    0x17e44885,// 236 PAY 233 

    0x7033ba95,// 237 PAY 234 

    0xbb85e48a,// 238 PAY 235 

    0x754b85ec,// 239 PAY 236 

    0xc5917295,// 240 PAY 237 

    0x59492a60,// 241 PAY 238 

    0x03c8f604,// 242 PAY 239 

    0xb6534c7f,// 243 PAY 240 

    0x8b7b1b45,// 244 PAY 241 

    0x53e009e9,// 245 PAY 242 

    0x5966784d,// 246 PAY 243 

    0x5c8a3ffa,// 247 PAY 244 

    0x160d4256,// 248 PAY 245 

    0xbe04bb2c,// 249 PAY 246 

    0x729ab0a9,// 250 PAY 247 

    0xe1921b9b,// 251 PAY 248 

    0x9555b97c,// 252 PAY 249 

    0x3a236f5c,// 253 PAY 250 

    0xba65dbe1,// 254 PAY 251 

    0x8169d24e,// 255 PAY 252 

    0x2fa57f27,// 256 PAY 253 

    0x921083a1,// 257 PAY 254 

    0xf24cc514,// 258 PAY 255 

    0xafe767de,// 259 PAY 256 

    0x3fe12468,// 260 PAY 257 

    0x44d46a4e,// 261 PAY 258 

    0x9539ad57,// 262 PAY 259 

    0xa8df6fe2,// 263 PAY 260 

    0xaa443d5e,// 264 PAY 261 

    0xf42b81bf,// 265 PAY 262 

    0xd0e2e921,// 266 PAY 263 

    0x76aac30d,// 267 PAY 264 

    0xdfa43ed0,// 268 PAY 265 

    0x868dd017,// 269 PAY 266 

    0xcdab075f,// 270 PAY 267 

    0xbbf39e21,// 271 PAY 268 

    0x670b99a7,// 272 PAY 269 

    0xaaad3fe7,// 273 PAY 270 

    0x242586a4,// 274 PAY 271 

    0x4b1aec65,// 275 PAY 272 

    0xec04bdd2,// 276 PAY 273 

    0xe6ac8d3a,// 277 PAY 274 

    0x893708a4,// 278 PAY 275 

    0xb0275973,// 279 PAY 276 

    0x77e65de9,// 280 PAY 277 

    0xaca9df8a,// 281 PAY 278 

    0x78e57576,// 282 PAY 279 

    0xcdc89b95,// 283 PAY 280 

    0x77ce468e,// 284 PAY 281 

    0xa2209ea7,// 285 PAY 282 

    0x0769dfba,// 286 PAY 283 

    0x29bcfbfb,// 287 PAY 284 

    0x2d6b9b18,// 288 PAY 285 

    0x1a568153,// 289 PAY 286 

    0xb1824732,// 290 PAY 287 

    0x2e48033c,// 291 PAY 288 

    0x14207de9,// 292 PAY 289 

    0xb50d3cde,// 293 PAY 290 

    0xc7c41c18,// 294 PAY 291 

    0x3dcdd00b,// 295 PAY 292 

    0x1494b88f,// 296 PAY 293 

    0x63556c9e,// 297 PAY 294 

    0xa9011af5,// 298 PAY 295 

    0x8475e9ea,// 299 PAY 296 

    0x0d9685e4,// 300 PAY 297 

    0xd52daf86,// 301 PAY 298 

    0x435627d4,// 302 PAY 299 

    0x8e74aa79,// 303 PAY 300 

    0x695e45c8,// 304 PAY 301 

    0xef14b318,// 305 PAY 302 

    0x2869ec84,// 306 PAY 303 

    0xa4c125d7,// 307 PAY 304 

    0x3c6eff08,// 308 PAY 305 

    0x05708f9c,// 309 PAY 306 

    0xbb01fdd0,// 310 PAY 307 

    0x4668c29e,// 311 PAY 308 

    0x4ccd525a,// 312 PAY 309 

    0xa1fc36f0,// 313 PAY 310 

    0x0d14e42f,// 314 PAY 311 

    0x9d1a2135,// 315 PAY 312 

    0x84c80c5a,// 316 PAY 313 

    0xf466fbb0,// 317 PAY 314 

    0x255735d8,// 318 PAY 315 

    0x6126c527,// 319 PAY 316 

    0x5c067c13,// 320 PAY 317 

    0xcb26af93,// 321 PAY 318 

    0x600a1ebd,// 322 PAY 319 

    0xacd6cfe4,// 323 PAY 320 

    0xeabf878e,// 324 PAY 321 

    0x75eeb2dc,// 325 PAY 322 

    0x00676182,// 326 PAY 323 

    0xd6d95643,// 327 PAY 324 

    0x10688dc5,// 328 PAY 325 

    0x35ad7dea,// 329 PAY 326 

    0x79777ad7,// 330 PAY 327 

    0x07fdbc16,// 331 PAY 328 

    0xdd2f204a,// 332 PAY 329 

    0xb82cec95,// 333 PAY 330 

    0xb24b2c32,// 334 PAY 331 

    0x77561913,// 335 PAY 332 

    0x41c4ea23,// 336 PAY 333 

    0x35094d42,// 337 PAY 334 

    0x719a2995,// 338 PAY 335 

    0x5398357a,// 339 PAY 336 

    0x2aba3459,// 340 PAY 337 

    0x78055bd1,// 341 PAY 338 

    0x279f72b4,// 342 PAY 339 

    0x84e61651,// 343 PAY 340 

    0x476bb934,// 344 PAY 341 

    0x3fa6c64d,// 345 PAY 342 

    0xb4ba5440,// 346 PAY 343 

    0xca9261f3,// 347 PAY 344 

    0x216826f3,// 348 PAY 345 

    0xece875a3,// 349 PAY 346 

    0x88a3d45f,// 350 PAY 347 

    0x1a8b2182,// 351 PAY 348 

    0x5cdbf375,// 352 PAY 349 

    0xe8f53b00,// 353 PAY 350 

    0x585024b4,// 354 PAY 351 

    0x892e47fd,// 355 PAY 352 

    0xbc506ffd,// 356 PAY 353 

    0x8376a318,// 357 PAY 354 

    0xee517d18,// 358 PAY 355 

    0xa6bfbb92,// 359 PAY 356 

    0x374e7f93,// 360 PAY 357 

    0xcb0672d3,// 361 PAY 358 

    0x32c8b946,// 362 PAY 359 

    0xdffff6aa,// 363 PAY 360 

    0xf37b54db,// 364 PAY 361 

    0x2098fe96,// 365 PAY 362 

    0x2646b789,// 366 PAY 363 

    0xb0803f2d,// 367 PAY 364 

    0x1fd052ff,// 368 PAY 365 

    0xe846a5a0,// 369 PAY 366 

    0xacff64f6,// 370 PAY 367 

    0x170c5293,// 371 PAY 368 

    0x73f5c7c6,// 372 PAY 369 

    0xb76d3263,// 373 PAY 370 

    0xe0d2c2a4,// 374 PAY 371 

    0x03d8377f,// 375 PAY 372 

    0x21af62e0,// 376 PAY 373 

    0x25dea708,// 377 PAY 374 

    0xffe4a665,// 378 PAY 375 

    0xa2b66b9d,// 379 PAY 376 

    0xf21a9054,// 380 PAY 377 

    0xe53dccdd,// 381 PAY 378 

    0x1e252a89,// 382 PAY 379 

    0x48206cac,// 383 PAY 380 

    0xb96ae96d,// 384 PAY 381 

    0x0d08ee91,// 385 PAY 382 

    0x9989a408,// 386 PAY 383 

    0xffb56338,// 387 PAY 384 

    0x7d5a32a2,// 388 PAY 385 

    0x9870e29e,// 389 PAY 386 

    0x85211bbe,// 390 PAY 387 

    0xa1cff962,// 391 PAY 388 

    0x6079586c,// 392 PAY 389 

    0x1deaf360,// 393 PAY 390 

    0x9e2d7589,// 394 PAY 391 

    0x2204901e,// 395 PAY 392 

    0x67e8b047,// 396 PAY 393 

    0x990be520,// 397 PAY 394 

    0x19b11d5d,// 398 PAY 395 

    0xe728bcf5,// 399 PAY 396 

    0x3da19dac,// 400 PAY 397 

    0x8a5c7c68,// 401 PAY 398 

    0x3bcc0a9b,// 402 PAY 399 

    0x66534dd5,// 403 PAY 400 

    0x928fb099,// 404 PAY 401 

    0x8578bc13,// 405 PAY 402 

    0x51874bd3,// 406 PAY 403 

    0xd2b4f71d,// 407 PAY 404 

    0xdff52a0c,// 408 PAY 405 

    0xb8f65cbf,// 409 PAY 406 

    0x432ca26e,// 410 PAY 407 

    0xa05cdc7a,// 411 PAY 408 

    0x136d13e1,// 412 PAY 409 

    0xd205ec85,// 413 PAY 410 

    0x46ad41a0,// 414 PAY 411 

    0x8a32ffbc,// 415 PAY 412 

    0x3c732e28,// 416 PAY 413 

    0xa8b0ee3b,// 417 PAY 414 

    0xd317eab0,// 418 PAY 415 

    0xeb77c4ae,// 419 PAY 416 

    0x1a43e424,// 420 PAY 417 

    0xff4808c2,// 421 PAY 418 

    0x7f26ae15,// 422 PAY 419 

    0x301bff00,// 423 PAY 420 

    0xe95419c9,// 424 PAY 421 

    0x3fc36f77,// 425 PAY 422 

    0xf31ed50e,// 426 PAY 423 

    0x28c178a9,// 427 PAY 424 

    0x162b798c,// 428 PAY 425 

    0xb02a59fe,// 429 PAY 426 

    0x3207e85c,// 430 PAY 427 

    0xa4bd6b96,// 431 PAY 428 

    0x1c858671,// 432 PAY 429 

    0xb79b733b,// 433 PAY 430 

    0xbf13bb42,// 434 PAY 431 

    0x208781f7,// 435 PAY 432 

    0xf5e3e78f,// 436 PAY 433 

    0xc341a45c,// 437 PAY 434 

    0xac37fcc0,// 438 PAY 435 

    0xbbbefae9,// 439 PAY 436 

    0x19c76d11,// 440 PAY 437 

    0x207feb59,// 441 PAY 438 

    0x10a057fa,// 442 PAY 439 

    0x6ad9db49,// 443 PAY 440 

    0x67466fc5,// 444 PAY 441 

    0xb600f871,// 445 PAY 442 

    0x88df1c78,// 446 PAY 443 

    0x74078cfe,// 447 PAY 444 

    0x38bc3723,// 448 PAY 445 

    0xdc3202a1,// 449 PAY 446 

    0x34a467e6,// 450 PAY 447 

    0x0e16662c,// 451 PAY 448 

    0xe0880f0d,// 452 PAY 449 

    0x27f45740,// 453 PAY 450 

    0x962efabc,// 454 PAY 451 

    0x8ec9631f,// 455 PAY 452 

    0xcea22354,// 456 PAY 453 

    0xeee6a5ab,// 457 PAY 454 

    0x66e8035e,// 458 PAY 455 

    0x04c2dad5,// 459 PAY 456 

    0x13dcc9c7,// 460 PAY 457 

    0xd7390be9,// 461 PAY 458 

    0xe8d5f7fb,// 462 PAY 459 

    0x869f1de1,// 463 PAY 460 

    0xba0e3366,// 464 PAY 461 

    0x6764d7d7,// 465 PAY 462 

    0x77311c27,// 466 PAY 463 

    0x5e472514,// 467 PAY 464 

    0x0db61c82,// 468 PAY 465 

    0x5bb07f67,// 469 PAY 466 

    0x655c5d12,// 470 PAY 467 

    0xd1df7122,// 471 PAY 468 

    0x9cd539eb,// 472 PAY 469 

    0x78530000,// 473 PAY 470 

/// HASH is  12 bytes 

    0xece875a3,// 474 HSH   1 

    0x88a3d45f,// 475 HSH   2 

    0x1a8b2182,// 476 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 233 

/// STA pkt_idx        : 239 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x03bc5be9 // 477 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt87_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 149 words. 

/// BDA size     is 590 (0x24e) 

/// BDA id       is 0x316c 

    0x024e316c,// 3 BDA   1 

/// PAY Generic Data size   : 590 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xde824f31,// 4 PAY   1 

    0xb1ae9ad3,// 5 PAY   2 

    0x90866d29,// 6 PAY   3 

    0xd56d3c42,// 7 PAY   4 

    0xd6b8ff81,// 8 PAY   5 

    0x7f03bcd0,// 9 PAY   6 

    0x2adea483,// 10 PAY   7 

    0x670683be,// 11 PAY   8 

    0x495768d7,// 12 PAY   9 

    0xc4898977,// 13 PAY  10 

    0x20c771a6,// 14 PAY  11 

    0xc202cdf8,// 15 PAY  12 

    0xf11499c7,// 16 PAY  13 

    0xad0bce5b,// 17 PAY  14 

    0x5e0947f7,// 18 PAY  15 

    0x39fb53e8,// 19 PAY  16 

    0x7bae9a44,// 20 PAY  17 

    0x3b926905,// 21 PAY  18 

    0x4b6a60bc,// 22 PAY  19 

    0xf2f4a3ae,// 23 PAY  20 

    0x670e861f,// 24 PAY  21 

    0xfa180517,// 25 PAY  22 

    0x1441a7da,// 26 PAY  23 

    0xb873db08,// 27 PAY  24 

    0x0f167d8c,// 28 PAY  25 

    0x1a497a99,// 29 PAY  26 

    0x887decfb,// 30 PAY  27 

    0xdd1c2ec9,// 31 PAY  28 

    0x65037d37,// 32 PAY  29 

    0xd89a02db,// 33 PAY  30 

    0x952709be,// 34 PAY  31 

    0xc862f27d,// 35 PAY  32 

    0xde39cee4,// 36 PAY  33 

    0xa2ea42be,// 37 PAY  34 

    0x00bfe31c,// 38 PAY  35 

    0xc77a9ca4,// 39 PAY  36 

    0xf8e7aaaa,// 40 PAY  37 

    0xe9624d61,// 41 PAY  38 

    0xb5344075,// 42 PAY  39 

    0xe620acdd,// 43 PAY  40 

    0x294e1973,// 44 PAY  41 

    0x6fbc18b0,// 45 PAY  42 

    0x0c575c69,// 46 PAY  43 

    0x50af2bbd,// 47 PAY  44 

    0x686f7662,// 48 PAY  45 

    0x13abebac,// 49 PAY  46 

    0x90ffee44,// 50 PAY  47 

    0x48b35e1d,// 51 PAY  48 

    0x9bdba551,// 52 PAY  49 

    0xf32789f2,// 53 PAY  50 

    0x9770714f,// 54 PAY  51 

    0x676c62bf,// 55 PAY  52 

    0xf5ceed0b,// 56 PAY  53 

    0xa87c0068,// 57 PAY  54 

    0xdba66473,// 58 PAY  55 

    0x23de8549,// 59 PAY  56 

    0x8c5146f5,// 60 PAY  57 

    0x30524438,// 61 PAY  58 

    0x4de018ce,// 62 PAY  59 

    0x1e3700e3,// 63 PAY  60 

    0xa894397c,// 64 PAY  61 

    0x00ab1069,// 65 PAY  62 

    0x8ce6ccfe,// 66 PAY  63 

    0x6a0a52bb,// 67 PAY  64 

    0xcf22a6c7,// 68 PAY  65 

    0xef3c179a,// 69 PAY  66 

    0xe5f4c4a5,// 70 PAY  67 

    0x47b15328,// 71 PAY  68 

    0x4efad821,// 72 PAY  69 

    0x85f4fa1a,// 73 PAY  70 

    0x90f54ef4,// 74 PAY  71 

    0xa7d5634b,// 75 PAY  72 

    0xaf8634c6,// 76 PAY  73 

    0x6cb99361,// 77 PAY  74 

    0xfe8e93a2,// 78 PAY  75 

    0xc4bad6a7,// 79 PAY  76 

    0xf0e7ecb9,// 80 PAY  77 

    0x936cdec5,// 81 PAY  78 

    0x21bb0942,// 82 PAY  79 

    0x127ac27d,// 83 PAY  80 

    0x02e742ab,// 84 PAY  81 

    0x751b1dc7,// 85 PAY  82 

    0x104af223,// 86 PAY  83 

    0xb4af4b01,// 87 PAY  84 

    0x6ecff8be,// 88 PAY  85 

    0x814cb1e1,// 89 PAY  86 

    0x4b97e4b7,// 90 PAY  87 

    0xe4f16065,// 91 PAY  88 

    0xa2255da1,// 92 PAY  89 

    0x40b7c567,// 93 PAY  90 

    0x100efd33,// 94 PAY  91 

    0x355502c9,// 95 PAY  92 

    0x78e42432,// 96 PAY  93 

    0xd3ef793d,// 97 PAY  94 

    0xef0a9ac4,// 98 PAY  95 

    0xaeafca50,// 99 PAY  96 

    0x38de0ebe,// 100 PAY  97 

    0xf67490a9,// 101 PAY  98 

    0xc4c1c577,// 102 PAY  99 

    0xc7d65f1e,// 103 PAY 100 

    0xbf34227a,// 104 PAY 101 

    0xb3ddbd45,// 105 PAY 102 

    0xe38f97cd,// 106 PAY 103 

    0x2c34480e,// 107 PAY 104 

    0xcf8f3eef,// 108 PAY 105 

    0xdf139fc2,// 109 PAY 106 

    0xc1545351,// 110 PAY 107 

    0x393faff8,// 111 PAY 108 

    0x49c5571d,// 112 PAY 109 

    0xaa4399ff,// 113 PAY 110 

    0x500facbb,// 114 PAY 111 

    0xdf476180,// 115 PAY 112 

    0xe64eeba3,// 116 PAY 113 

    0x3f99ba05,// 117 PAY 114 

    0x621137a2,// 118 PAY 115 

    0x13cbecad,// 119 PAY 116 

    0xe8885555,// 120 PAY 117 

    0x22ce1782,// 121 PAY 118 

    0x613472c7,// 122 PAY 119 

    0xc753983e,// 123 PAY 120 

    0x531ec60b,// 124 PAY 121 

    0xdcf0ed2c,// 125 PAY 122 

    0x970514f5,// 126 PAY 123 

    0x4e6a8d96,// 127 PAY 124 

    0x4fa11809,// 128 PAY 125 

    0x473523d9,// 129 PAY 126 

    0x83c7c08a,// 130 PAY 127 

    0xbae0d38a,// 131 PAY 128 

    0x4f9448da,// 132 PAY 129 

    0x58a66d5f,// 133 PAY 130 

    0x5ee527c0,// 134 PAY 131 

    0xa6b0f042,// 135 PAY 132 

    0x0a70ca4e,// 136 PAY 133 

    0xa0d8458a,// 137 PAY 134 

    0x7cb2264b,// 138 PAY 135 

    0x77344c62,// 139 PAY 136 

    0xb8019e05,// 140 PAY 137 

    0xf5e448f7,// 141 PAY 138 

    0x2ac80a9a,// 142 PAY 139 

    0x8e3e9cb2,// 143 PAY 140 

    0x1b4e0608,// 144 PAY 141 

    0xfb02b3ca,// 145 PAY 142 

    0x4927c2de,// 146 PAY 143 

    0xd7480d8c,// 147 PAY 144 

    0xdf66da80,// 148 PAY 145 

    0xf1a793c9,// 149 PAY 146 

    0xd2e292a7,// 150 PAY 147 

    0xabb20000,// 151 PAY 148 

/// STA is 1 words. 

/// STA num_pkts       : 189 

/// STA pkt_idx        : 136 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcb 

    0x0220cbbd // 152 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt88_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 430 words. 

/// BDA size     is 1714 (0x6b2) 

/// BDA id       is 0x441 

    0x06b20441,// 3 BDA   1 

/// PAY Generic Data size   : 1714 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x34f5b3dd,// 4 PAY   1 

    0x48c1ea3d,// 5 PAY   2 

    0xeaf0baae,// 6 PAY   3 

    0x643e9213,// 7 PAY   4 

    0x44d063b2,// 8 PAY   5 

    0xae0e7961,// 9 PAY   6 

    0x055cf12f,// 10 PAY   7 

    0x46c1aea1,// 11 PAY   8 

    0x12224d68,// 12 PAY   9 

    0x0ba368fa,// 13 PAY  10 

    0x38a5c663,// 14 PAY  11 

    0xff1a9e29,// 15 PAY  12 

    0x31716c28,// 16 PAY  13 

    0x59e9057f,// 17 PAY  14 

    0xf8c3c0dc,// 18 PAY  15 

    0xec4dede3,// 19 PAY  16 

    0xc0e0e816,// 20 PAY  17 

    0x0a2e412a,// 21 PAY  18 

    0x4b39bfb4,// 22 PAY  19 

    0xcb6020a1,// 23 PAY  20 

    0x0f6062ad,// 24 PAY  21 

    0x52e57c05,// 25 PAY  22 

    0xf4d6337d,// 26 PAY  23 

    0x16b8a29f,// 27 PAY  24 

    0x8cc5ae64,// 28 PAY  25 

    0x4b9c7d3b,// 29 PAY  26 

    0x9b259560,// 30 PAY  27 

    0xf08d2070,// 31 PAY  28 

    0x83674631,// 32 PAY  29 

    0x48d08880,// 33 PAY  30 

    0xd2deb65a,// 34 PAY  31 

    0xd7edbc14,// 35 PAY  32 

    0x86ed0c8b,// 36 PAY  33 

    0xc248b9af,// 37 PAY  34 

    0xd9a50d72,// 38 PAY  35 

    0x809848ea,// 39 PAY  36 

    0x1d9aefc2,// 40 PAY  37 

    0xd27a7e65,// 41 PAY  38 

    0x43ecf56c,// 42 PAY  39 

    0x0c3069e0,// 43 PAY  40 

    0xb1f9afb0,// 44 PAY  41 

    0xc515d66b,// 45 PAY  42 

    0x62a169f8,// 46 PAY  43 

    0x33119c32,// 47 PAY  44 

    0x1abd9e93,// 48 PAY  45 

    0xcedbe20c,// 49 PAY  46 

    0x23246329,// 50 PAY  47 

    0x3ee24b6b,// 51 PAY  48 

    0xcfce2ad4,// 52 PAY  49 

    0x118c5934,// 53 PAY  50 

    0xcf1324d7,// 54 PAY  51 

    0x9dc6c2f9,// 55 PAY  52 

    0x73862cca,// 56 PAY  53 

    0x59e98959,// 57 PAY  54 

    0xe0fd1e71,// 58 PAY  55 

    0xa8493aab,// 59 PAY  56 

    0xceb416be,// 60 PAY  57 

    0xb39386bb,// 61 PAY  58 

    0xe2010a0b,// 62 PAY  59 

    0x8130237f,// 63 PAY  60 

    0xf0783972,// 64 PAY  61 

    0x16c4abbe,// 65 PAY  62 

    0x6a6eab65,// 66 PAY  63 

    0x58a00818,// 67 PAY  64 

    0xe7b9987d,// 68 PAY  65 

    0x197f1039,// 69 PAY  66 

    0x4fa28608,// 70 PAY  67 

    0xe0da71a7,// 71 PAY  68 

    0xda78ed3a,// 72 PAY  69 

    0x53e30531,// 73 PAY  70 

    0xcaf70dbb,// 74 PAY  71 

    0xc144ca7f,// 75 PAY  72 

    0xd24c841d,// 76 PAY  73 

    0x3bdbdf04,// 77 PAY  74 

    0xe4f07555,// 78 PAY  75 

    0xba9e115b,// 79 PAY  76 

    0x7093b894,// 80 PAY  77 

    0xa5ac2b6b,// 81 PAY  78 

    0xbd7f90ac,// 82 PAY  79 

    0xe6e3bca4,// 83 PAY  80 

    0x7c2e0e4e,// 84 PAY  81 

    0x02630819,// 85 PAY  82 

    0xaf8019ee,// 86 PAY  83 

    0x20082c1f,// 87 PAY  84 

    0xa4cb9320,// 88 PAY  85 

    0x28db63dd,// 89 PAY  86 

    0x73306cde,// 90 PAY  87 

    0xcf6a4832,// 91 PAY  88 

    0x1b6c4fd5,// 92 PAY  89 

    0x1d152cb4,// 93 PAY  90 

    0x777f0e50,// 94 PAY  91 

    0xa96667a3,// 95 PAY  92 

    0xbc9a77d3,// 96 PAY  93 

    0x7326fda1,// 97 PAY  94 

    0x1521b991,// 98 PAY  95 

    0x6278cd0a,// 99 PAY  96 

    0x7e14fc55,// 100 PAY  97 

    0x5937d72d,// 101 PAY  98 

    0x7896a2f5,// 102 PAY  99 

    0x49558994,// 103 PAY 100 

    0x58ca1239,// 104 PAY 101 

    0xedbcd17b,// 105 PAY 102 

    0x5a7f1b4d,// 106 PAY 103 

    0x04ab3a89,// 107 PAY 104 

    0x179c6bda,// 108 PAY 105 

    0xacc7a8a4,// 109 PAY 106 

    0xb603bc83,// 110 PAY 107 

    0x831d1864,// 111 PAY 108 

    0x96f84ce9,// 112 PAY 109 

    0x578820fd,// 113 PAY 110 

    0x4ad1ee0d,// 114 PAY 111 

    0x6c0eade6,// 115 PAY 112 

    0x0935d734,// 116 PAY 113 

    0xd5a069b0,// 117 PAY 114 

    0xc73e345d,// 118 PAY 115 

    0x9e76fd17,// 119 PAY 116 

    0x4bf60e95,// 120 PAY 117 

    0xc96328d7,// 121 PAY 118 

    0x0a80a148,// 122 PAY 119 

    0xaaf7bc31,// 123 PAY 120 

    0xc22fdf05,// 124 PAY 121 

    0xddef3d86,// 125 PAY 122 

    0x02af35fa,// 126 PAY 123 

    0xbd239394,// 127 PAY 124 

    0xe1db63bf,// 128 PAY 125 

    0x90567c7a,// 129 PAY 126 

    0x391ca87d,// 130 PAY 127 

    0x034cae36,// 131 PAY 128 

    0x2d8ae553,// 132 PAY 129 

    0xa6346206,// 133 PAY 130 

    0xa77f037a,// 134 PAY 131 

    0x0d5541a3,// 135 PAY 132 

    0x32934d5d,// 136 PAY 133 

    0xabff8d04,// 137 PAY 134 

    0x468447a6,// 138 PAY 135 

    0x82c3bd73,// 139 PAY 136 

    0xeb25a421,// 140 PAY 137 

    0x8b946b1b,// 141 PAY 138 

    0x99043a59,// 142 PAY 139 

    0x1b9b8353,// 143 PAY 140 

    0xa6f34bcb,// 144 PAY 141 

    0x4ef0283a,// 145 PAY 142 

    0x278c7d1b,// 146 PAY 143 

    0x86455d42,// 147 PAY 144 

    0x3b4491b8,// 148 PAY 145 

    0x901116e5,// 149 PAY 146 

    0xcdfbb7d5,// 150 PAY 147 

    0x322161ae,// 151 PAY 148 

    0x476a7933,// 152 PAY 149 

    0xdd6a68d1,// 153 PAY 150 

    0x8964e2fc,// 154 PAY 151 

    0x55c8c087,// 155 PAY 152 

    0x192b04ae,// 156 PAY 153 

    0x1f937c15,// 157 PAY 154 

    0x5350ba2a,// 158 PAY 155 

    0x7256ee23,// 159 PAY 156 

    0x6594368a,// 160 PAY 157 

    0x2e225612,// 161 PAY 158 

    0x9c52e0f4,// 162 PAY 159 

    0x1316925e,// 163 PAY 160 

    0x553f440a,// 164 PAY 161 

    0x4e27c465,// 165 PAY 162 

    0x6d86874d,// 166 PAY 163 

    0xddaf0e1b,// 167 PAY 164 

    0xb789315f,// 168 PAY 165 

    0x55127782,// 169 PAY 166 

    0xf7df928c,// 170 PAY 167 

    0x29b6c6f4,// 171 PAY 168 

    0x993c594c,// 172 PAY 169 

    0xc51ec4d6,// 173 PAY 170 

    0x0531be22,// 174 PAY 171 

    0x8a34496b,// 175 PAY 172 

    0x817548e0,// 176 PAY 173 

    0x2699fe53,// 177 PAY 174 

    0x775d2e73,// 178 PAY 175 

    0x2fdc43ff,// 179 PAY 176 

    0xad920d93,// 180 PAY 177 

    0x51955497,// 181 PAY 178 

    0x34a4560a,// 182 PAY 179 

    0x08110505,// 183 PAY 180 

    0x6d88139b,// 184 PAY 181 

    0xd4eedc9d,// 185 PAY 182 

    0x63978d3d,// 186 PAY 183 

    0x4df77ddc,// 187 PAY 184 

    0x699903cc,// 188 PAY 185 

    0x7d330e10,// 189 PAY 186 

    0x426c6f11,// 190 PAY 187 

    0x45204a7e,// 191 PAY 188 

    0x9c917d92,// 192 PAY 189 

    0x07c3fdeb,// 193 PAY 190 

    0xe53317de,// 194 PAY 191 

    0xac1398bd,// 195 PAY 192 

    0x2c5bedc1,// 196 PAY 193 

    0x553101df,// 197 PAY 194 

    0xe88f56d9,// 198 PAY 195 

    0x00a6c7b5,// 199 PAY 196 

    0x459c6b11,// 200 PAY 197 

    0x58981f16,// 201 PAY 198 

    0x1c92925d,// 202 PAY 199 

    0x1942fd52,// 203 PAY 200 

    0xb00586bd,// 204 PAY 201 

    0x22889fa9,// 205 PAY 202 

    0xdfc46659,// 206 PAY 203 

    0xe748cc59,// 207 PAY 204 

    0x6a705572,// 208 PAY 205 

    0x711dcd97,// 209 PAY 206 

    0x9aa98a18,// 210 PAY 207 

    0x0a809302,// 211 PAY 208 

    0xdf006c4a,// 212 PAY 209 

    0x49492ade,// 213 PAY 210 

    0xd96bc3fb,// 214 PAY 211 

    0xfb34cec8,// 215 PAY 212 

    0xf0fb47f0,// 216 PAY 213 

    0xd4c0d3d2,// 217 PAY 214 

    0x049e0af4,// 218 PAY 215 

    0x0b7f69fe,// 219 PAY 216 

    0x6f701c86,// 220 PAY 217 

    0xaff54bba,// 221 PAY 218 

    0xca4b2a4d,// 222 PAY 219 

    0x18d1c30c,// 223 PAY 220 

    0xe69ec023,// 224 PAY 221 

    0xc752d576,// 225 PAY 222 

    0x8405002f,// 226 PAY 223 

    0x6508744b,// 227 PAY 224 

    0x509ae062,// 228 PAY 225 

    0x9f3aa1c0,// 229 PAY 226 

    0x68ca0624,// 230 PAY 227 

    0xbb841899,// 231 PAY 228 

    0x94a402c1,// 232 PAY 229 

    0xe1743185,// 233 PAY 230 

    0xd7a8217a,// 234 PAY 231 

    0xeef6675e,// 235 PAY 232 

    0xb0649a54,// 236 PAY 233 

    0x21c7a705,// 237 PAY 234 

    0x5645f593,// 238 PAY 235 

    0x0b1e6b3d,// 239 PAY 236 

    0x0902486d,// 240 PAY 237 

    0x58ccbc7f,// 241 PAY 238 

    0x2ee79f4f,// 242 PAY 239 

    0x3366c22c,// 243 PAY 240 

    0x76315afd,// 244 PAY 241 

    0x4d5c140d,// 245 PAY 242 

    0x2376efcc,// 246 PAY 243 

    0x07258617,// 247 PAY 244 

    0xce6945ac,// 248 PAY 245 

    0x7e64e055,// 249 PAY 246 

    0x1f3a96b2,// 250 PAY 247 

    0xd003f0ea,// 251 PAY 248 

    0x9deb815e,// 252 PAY 249 

    0xb931cc04,// 253 PAY 250 

    0x34062b28,// 254 PAY 251 

    0xafc213c0,// 255 PAY 252 

    0x16953276,// 256 PAY 253 

    0xbcb5d779,// 257 PAY 254 

    0x2ae26cb8,// 258 PAY 255 

    0xfd5e11ef,// 259 PAY 256 

    0xf36ab6a4,// 260 PAY 257 

    0x43acaf99,// 261 PAY 258 

    0x77cb2436,// 262 PAY 259 

    0xeb72efd4,// 263 PAY 260 

    0xe94d6b67,// 264 PAY 261 

    0x5c1506ad,// 265 PAY 262 

    0xf3feea2c,// 266 PAY 263 

    0x10e87dae,// 267 PAY 264 

    0xbc06df42,// 268 PAY 265 

    0xa911c176,// 269 PAY 266 

    0xc3f5314e,// 270 PAY 267 

    0x11f351a0,// 271 PAY 268 

    0x6cb11786,// 272 PAY 269 

    0xc1985fc5,// 273 PAY 270 

    0xdbeb7766,// 274 PAY 271 

    0x8ca94424,// 275 PAY 272 

    0x6e06ded0,// 276 PAY 273 

    0xea4bd162,// 277 PAY 274 

    0x81f78407,// 278 PAY 275 

    0x79b32b3c,// 279 PAY 276 

    0x571542b4,// 280 PAY 277 

    0xc53dccb1,// 281 PAY 278 

    0xd7735810,// 282 PAY 279 

    0x7ae4abc3,// 283 PAY 280 

    0xee49163f,// 284 PAY 281 

    0xce124011,// 285 PAY 282 

    0x008ba74f,// 286 PAY 283 

    0x4635c2fb,// 287 PAY 284 

    0x1fd974e9,// 288 PAY 285 

    0xc80ec84e,// 289 PAY 286 

    0x0175c003,// 290 PAY 287 

    0xc90dd298,// 291 PAY 288 

    0x0e912aaa,// 292 PAY 289 

    0xdeea2bd3,// 293 PAY 290 

    0xd8db98d9,// 294 PAY 291 

    0x88eaae29,// 295 PAY 292 

    0x1296661b,// 296 PAY 293 

    0xb3b07b5a,// 297 PAY 294 

    0x5d166b8a,// 298 PAY 295 

    0x1ebb0620,// 299 PAY 296 

    0xeac0ba62,// 300 PAY 297 

    0x838f1f9b,// 301 PAY 298 

    0x16401570,// 302 PAY 299 

    0x39aae24d,// 303 PAY 300 

    0x39b4b7ac,// 304 PAY 301 

    0x786f19dc,// 305 PAY 302 

    0xad923b5d,// 306 PAY 303 

    0x0d9e3457,// 307 PAY 304 

    0x678d2091,// 308 PAY 305 

    0x2b6b8915,// 309 PAY 306 

    0xc7f81682,// 310 PAY 307 

    0xb80e9bca,// 311 PAY 308 

    0x88f6ada3,// 312 PAY 309 

    0x6a515ed9,// 313 PAY 310 

    0xe1970c5f,// 314 PAY 311 

    0x9fa0522b,// 315 PAY 312 

    0xc8592699,// 316 PAY 313 

    0x4aa4f20e,// 317 PAY 314 

    0x4b5d8f3d,// 318 PAY 315 

    0x43a3b698,// 319 PAY 316 

    0xd8e92459,// 320 PAY 317 

    0x90368a75,// 321 PAY 318 

    0xbfbef7fd,// 322 PAY 319 

    0x94c5eaa5,// 323 PAY 320 

    0xc2b12282,// 324 PAY 321 

    0x212086fc,// 325 PAY 322 

    0xd16bcbb5,// 326 PAY 323 

    0x3e327ca9,// 327 PAY 324 

    0x51d9bfd6,// 328 PAY 325 

    0x2fcd6f34,// 329 PAY 326 

    0x6ce58d0a,// 330 PAY 327 

    0x451db642,// 331 PAY 328 

    0x44322460,// 332 PAY 329 

    0xb401a3c5,// 333 PAY 330 

    0x919072ff,// 334 PAY 331 

    0x5378f886,// 335 PAY 332 

    0x0b5b7fca,// 336 PAY 333 

    0xe1a59ad7,// 337 PAY 334 

    0xf00b270e,// 338 PAY 335 

    0x463c3369,// 339 PAY 336 

    0x04bb3ade,// 340 PAY 337 

    0x7187baf6,// 341 PAY 338 

    0x35887fd2,// 342 PAY 339 

    0xf423454e,// 343 PAY 340 

    0x23147fa0,// 344 PAY 341 

    0xdf6efe01,// 345 PAY 342 

    0xec7eda48,// 346 PAY 343 

    0x691433a4,// 347 PAY 344 

    0x8130d741,// 348 PAY 345 

    0x678c3349,// 349 PAY 346 

    0x43284208,// 350 PAY 347 

    0xa882c942,// 351 PAY 348 

    0x027a22ae,// 352 PAY 349 

    0xfd89de39,// 353 PAY 350 

    0xb7e4a1f4,// 354 PAY 351 

    0xd2e86e3a,// 355 PAY 352 

    0x8738c5fd,// 356 PAY 353 

    0xa74dc4ab,// 357 PAY 354 

    0xef3fe84f,// 358 PAY 355 

    0xd84adb21,// 359 PAY 356 

    0x95045127,// 360 PAY 357 

    0xabf27f7e,// 361 PAY 358 

    0x2924f136,// 362 PAY 359 

    0xea50970f,// 363 PAY 360 

    0x1c577e37,// 364 PAY 361 

    0x425daccb,// 365 PAY 362 

    0xce3b8704,// 366 PAY 363 

    0xe9e88db1,// 367 PAY 364 

    0xf003b81e,// 368 PAY 365 

    0xad53423d,// 369 PAY 366 

    0x5bc7adb4,// 370 PAY 367 

    0x3b06d6da,// 371 PAY 368 

    0x9f65e6a6,// 372 PAY 369 

    0x3704109d,// 373 PAY 370 

    0x97c51296,// 374 PAY 371 

    0xb6deee79,// 375 PAY 372 

    0x2a9b093d,// 376 PAY 373 

    0xeba1f5ac,// 377 PAY 374 

    0xd6ea22dd,// 378 PAY 375 

    0xff4839be,// 379 PAY 376 

    0xd082c39f,// 380 PAY 377 

    0x551d1a6f,// 381 PAY 378 

    0x473b0dfc,// 382 PAY 379 

    0xfd79015a,// 383 PAY 380 

    0xa6b59861,// 384 PAY 381 

    0x00ff9f9f,// 385 PAY 382 

    0x2630aa72,// 386 PAY 383 

    0x3ccbb72d,// 387 PAY 384 

    0x01142205,// 388 PAY 385 

    0x784c53dc,// 389 PAY 386 

    0xc4530238,// 390 PAY 387 

    0x7e266db2,// 391 PAY 388 

    0x913d13f8,// 392 PAY 389 

    0xc443b0dc,// 393 PAY 390 

    0xf04566df,// 394 PAY 391 

    0xd6bda85c,// 395 PAY 392 

    0xe99fd40d,// 396 PAY 393 

    0xadf40826,// 397 PAY 394 

    0x42542cf6,// 398 PAY 395 

    0x6ef1d070,// 399 PAY 396 

    0x5741742b,// 400 PAY 397 

    0x1b2c0a3d,// 401 PAY 398 

    0xb8248ce8,// 402 PAY 399 

    0xc94ce079,// 403 PAY 400 

    0x62b8351a,// 404 PAY 401 

    0x19a61204,// 405 PAY 402 

    0x035e5b91,// 406 PAY 403 

    0x6bd4c9da,// 407 PAY 404 

    0x7f619e98,// 408 PAY 405 

    0xc4e9be78,// 409 PAY 406 

    0xebb4d872,// 410 PAY 407 

    0x22a2c799,// 411 PAY 408 

    0x3e7c6ee8,// 412 PAY 409 

    0x439ac8c3,// 413 PAY 410 

    0x64f86eb2,// 414 PAY 411 

    0x07b54bbb,// 415 PAY 412 

    0x74631416,// 416 PAY 413 

    0x7367f536,// 417 PAY 414 

    0xe554a2fb,// 418 PAY 415 

    0x80b7b0de,// 419 PAY 416 

    0x2eb0a8f4,// 420 PAY 417 

    0x4d2d5ef5,// 421 PAY 418 

    0xb2abbf30,// 422 PAY 419 

    0x2d7d0703,// 423 PAY 420 

    0xe0d24cb2,// 424 PAY 421 

    0x1dd73c5d,// 425 PAY 422 

    0x05485137,// 426 PAY 423 

    0xc430b980,// 427 PAY 424 

    0x45f7fb77,// 428 PAY 425 

    0xad2817f5,// 429 PAY 426 

    0xd9d9cbb4,// 430 PAY 427 

    0x30040cdd,// 431 PAY 428 

    0x29eb0000,// 432 PAY 429 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf1 

    0x0248f17e // 433 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt89_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 56 words. 

/// BDA size     is 220 (0xdc) 

/// BDA id       is 0x8e30 

    0x00dc8e30,// 3 BDA   1 

/// PAY Generic Data size   : 220 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xcff02abb,// 4 PAY   1 

    0xbaa28c9d,// 5 PAY   2 

    0x4c1e9ae4,// 6 PAY   3 

    0x313eb3de,// 7 PAY   4 

    0xde1d88c4,// 8 PAY   5 

    0xc1c3408e,// 9 PAY   6 

    0x9074cb5a,// 10 PAY   7 

    0x4dbabf86,// 11 PAY   8 

    0x3ba62f47,// 12 PAY   9 

    0xc5c280eb,// 13 PAY  10 

    0x812100e0,// 14 PAY  11 

    0x2833fa6d,// 15 PAY  12 

    0x1ff2e6f5,// 16 PAY  13 

    0xa968ae80,// 17 PAY  14 

    0xca9ee0a1,// 18 PAY  15 

    0x0ba3971a,// 19 PAY  16 

    0x8a5af824,// 20 PAY  17 

    0x76f82510,// 21 PAY  18 

    0x37e71550,// 22 PAY  19 

    0xb3df461e,// 23 PAY  20 

    0xc4aa43e6,// 24 PAY  21 

    0x66957d79,// 25 PAY  22 

    0xdc6607b2,// 26 PAY  23 

    0x7d72e5fa,// 27 PAY  24 

    0x6cd44d3e,// 28 PAY  25 

    0xdab3ecf7,// 29 PAY  26 

    0x37991d94,// 30 PAY  27 

    0x12aa638f,// 31 PAY  28 

    0x1fc0290f,// 32 PAY  29 

    0x18a6e157,// 33 PAY  30 

    0x4b88d312,// 34 PAY  31 

    0xf644041b,// 35 PAY  32 

    0x51d2a2f4,// 36 PAY  33 

    0xb5590ad5,// 37 PAY  34 

    0x2b0fe49c,// 38 PAY  35 

    0xe658e6ef,// 39 PAY  36 

    0xa75a14ba,// 40 PAY  37 

    0x41e4f9ff,// 41 PAY  38 

    0xea34fd6d,// 42 PAY  39 

    0x9ae3cbfb,// 43 PAY  40 

    0x18e0eb41,// 44 PAY  41 

    0xc2599e2d,// 45 PAY  42 

    0x5475dc6a,// 46 PAY  43 

    0xf6826c67,// 47 PAY  44 

    0xda6bf61d,// 48 PAY  45 

    0x12f20017,// 49 PAY  46 

    0x3442b1c2,// 50 PAY  47 

    0x9bf154fd,// 51 PAY  48 

    0xd0c00a7b,// 52 PAY  49 

    0x379c49f0,// 53 PAY  50 

    0x583cf3c0,// 54 PAY  51 

    0xaa4d7607,// 55 PAY  52 

    0xfaea1582,// 56 PAY  53 

    0xd775725f,// 57 PAY  54 

    0x3b2fe092,// 58 PAY  55 

/// HASH is  4 bytes 

    0x41e4f9ff,// 59 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 49 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x0188c131 // 60 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt90_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 306 words. 

/// BDA size     is 1218 (0x4c2) 

/// BDA id       is 0xa336 

    0x04c2a336,// 3 BDA   1 

/// PAY Generic Data size   : 1218 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xf5e8c9f1,// 4 PAY   1 

    0xfd689736,// 5 PAY   2 

    0xf5d6f6e8,// 6 PAY   3 

    0x4f4cbfad,// 7 PAY   4 

    0x8ec12151,// 8 PAY   5 

    0xdf9cde60,// 9 PAY   6 

    0xca59eb70,// 10 PAY   7 

    0x5b968e29,// 11 PAY   8 

    0xb711feaa,// 12 PAY   9 

    0x07d30ac3,// 13 PAY  10 

    0xcfb69af4,// 14 PAY  11 

    0x87b36797,// 15 PAY  12 

    0xd5b688ee,// 16 PAY  13 

    0x89864a8d,// 17 PAY  14 

    0xe91e3d90,// 18 PAY  15 

    0xa3ca43fb,// 19 PAY  16 

    0xac149ac4,// 20 PAY  17 

    0xc7dc8d76,// 21 PAY  18 

    0x93b86d8a,// 22 PAY  19 

    0x2d4ffb9b,// 23 PAY  20 

    0x59c6b1f7,// 24 PAY  21 

    0x2fd5d572,// 25 PAY  22 

    0x51b85382,// 26 PAY  23 

    0xccc6fae5,// 27 PAY  24 

    0x84903b09,// 28 PAY  25 

    0xce8387ca,// 29 PAY  26 

    0xac0bce93,// 30 PAY  27 

    0xd948640b,// 31 PAY  28 

    0x0c9dbabe,// 32 PAY  29 

    0xad07bf5c,// 33 PAY  30 

    0xdc5ab6d7,// 34 PAY  31 

    0xde096a14,// 35 PAY  32 

    0x15766b40,// 36 PAY  33 

    0x0273b9dd,// 37 PAY  34 

    0x3f24c974,// 38 PAY  35 

    0x2345df30,// 39 PAY  36 

    0xecbc6135,// 40 PAY  37 

    0xfd42913a,// 41 PAY  38 

    0x7f0cfa1c,// 42 PAY  39 

    0xb6804090,// 43 PAY  40 

    0xd208e100,// 44 PAY  41 

    0xb3d7c932,// 45 PAY  42 

    0x7d067fc1,// 46 PAY  43 

    0x1fa81bbb,// 47 PAY  44 

    0x10bdc03a,// 48 PAY  45 

    0xb09f0967,// 49 PAY  46 

    0xafb75c4e,// 50 PAY  47 

    0xaa29cca1,// 51 PAY  48 

    0x87a766d5,// 52 PAY  49 

    0xc26a7905,// 53 PAY  50 

    0x651a0843,// 54 PAY  51 

    0xf2fbf034,// 55 PAY  52 

    0x80b42b6d,// 56 PAY  53 

    0x180c620f,// 57 PAY  54 

    0x38df75f8,// 58 PAY  55 

    0x8e5193d5,// 59 PAY  56 

    0x1013a0e3,// 60 PAY  57 

    0x5fa9de38,// 61 PAY  58 

    0xa0b804b8,// 62 PAY  59 

    0xd66dc13e,// 63 PAY  60 

    0x6963a5ce,// 64 PAY  61 

    0x7404faa2,// 65 PAY  62 

    0x666494c4,// 66 PAY  63 

    0xaf3e5c10,// 67 PAY  64 

    0x2c150ef3,// 68 PAY  65 

    0x7c766a36,// 69 PAY  66 

    0x042106e6,// 70 PAY  67 

    0x2524cebe,// 71 PAY  68 

    0x3b8b3306,// 72 PAY  69 

    0xc2974a9b,// 73 PAY  70 

    0x4dc65e23,// 74 PAY  71 

    0x6d93cfb6,// 75 PAY  72 

    0xea669958,// 76 PAY  73 

    0x84b327c9,// 77 PAY  74 

    0x2d97552a,// 78 PAY  75 

    0x4a61ea6a,// 79 PAY  76 

    0xd2446017,// 80 PAY  77 

    0xb67d3bd2,// 81 PAY  78 

    0x19fe1f0e,// 82 PAY  79 

    0xeb3ae20c,// 83 PAY  80 

    0xea5053df,// 84 PAY  81 

    0xc3caaedb,// 85 PAY  82 

    0x644dcc62,// 86 PAY  83 

    0x5304f7f1,// 87 PAY  84 

    0xb4cefe6c,// 88 PAY  85 

    0x1269df7a,// 89 PAY  86 

    0x3dc92c1b,// 90 PAY  87 

    0xebfe0106,// 91 PAY  88 

    0x2dd01cf3,// 92 PAY  89 

    0x5544c5c3,// 93 PAY  90 

    0x82ec90a1,// 94 PAY  91 

    0x4896162c,// 95 PAY  92 

    0xd8774b88,// 96 PAY  93 

    0xace8b8c9,// 97 PAY  94 

    0x838863d9,// 98 PAY  95 

    0xd0de58e5,// 99 PAY  96 

    0x29702e23,// 100 PAY  97 

    0x3bc330cf,// 101 PAY  98 

    0xb9412154,// 102 PAY  99 

    0xf52c441b,// 103 PAY 100 

    0x427d064a,// 104 PAY 101 

    0xced44640,// 105 PAY 102 

    0x59af7906,// 106 PAY 103 

    0x94568b57,// 107 PAY 104 

    0x5732edbf,// 108 PAY 105 

    0x5be74a04,// 109 PAY 106 

    0x835dd237,// 110 PAY 107 

    0xdf66b77d,// 111 PAY 108 

    0xd717949c,// 112 PAY 109 

    0xbd8aa672,// 113 PAY 110 

    0xcf66a3db,// 114 PAY 111 

    0x6d2dffd9,// 115 PAY 112 

    0xc5be0bae,// 116 PAY 113 

    0x85c3b8eb,// 117 PAY 114 

    0x25f1bc1f,// 118 PAY 115 

    0xbe99d26c,// 119 PAY 116 

    0x2f95b3d4,// 120 PAY 117 

    0x473b7103,// 121 PAY 118 

    0x093079d0,// 122 PAY 119 

    0x766f6094,// 123 PAY 120 

    0xf4790f27,// 124 PAY 121 

    0xcacabf01,// 125 PAY 122 

    0xec472a8b,// 126 PAY 123 

    0xb334bf3c,// 127 PAY 124 

    0x35502e9e,// 128 PAY 125 

    0x3402b5aa,// 129 PAY 126 

    0x31c11f89,// 130 PAY 127 

    0x89300aa5,// 131 PAY 128 

    0x4985b46c,// 132 PAY 129 

    0xcd3cf4c6,// 133 PAY 130 

    0x4004c4f7,// 134 PAY 131 

    0x6a054f3f,// 135 PAY 132 

    0x35834fa5,// 136 PAY 133 

    0x2dc7a057,// 137 PAY 134 

    0x4d621ce5,// 138 PAY 135 

    0x460433ba,// 139 PAY 136 

    0x289140ad,// 140 PAY 137 

    0x1bcfdb94,// 141 PAY 138 

    0x8ca185f6,// 142 PAY 139 

    0xf7e9f6e0,// 143 PAY 140 

    0x1ec7e018,// 144 PAY 141 

    0xf78f7fbf,// 145 PAY 142 

    0x60f37eb5,// 146 PAY 143 

    0xc8971f88,// 147 PAY 144 

    0x60882344,// 148 PAY 145 

    0x22b77518,// 149 PAY 146 

    0x0744c6a7,// 150 PAY 147 

    0x875756a4,// 151 PAY 148 

    0x0951d18a,// 152 PAY 149 

    0x6b453f98,// 153 PAY 150 

    0xda1ffb02,// 154 PAY 151 

    0xf8b0bf0d,// 155 PAY 152 

    0xc6a35fd5,// 156 PAY 153 

    0xf3a56447,// 157 PAY 154 

    0xcd6fee7b,// 158 PAY 155 

    0xa071f5ad,// 159 PAY 156 

    0xce2ed66f,// 160 PAY 157 

    0x7fb4c49d,// 161 PAY 158 

    0x0683d208,// 162 PAY 159 

    0xcb629963,// 163 PAY 160 

    0x54db0974,// 164 PAY 161 

    0x541d29a8,// 165 PAY 162 

    0xca16d9c2,// 166 PAY 163 

    0x925b347c,// 167 PAY 164 

    0x4f4b8d9d,// 168 PAY 165 

    0x3f542b81,// 169 PAY 166 

    0xf2a3420b,// 170 PAY 167 

    0xfd0b7d69,// 171 PAY 168 

    0xaf8e49c6,// 172 PAY 169 

    0x6b722093,// 173 PAY 170 

    0x6dab3b54,// 174 PAY 171 

    0xe6b49407,// 175 PAY 172 

    0x93c70f8c,// 176 PAY 173 

    0x3ad10393,// 177 PAY 174 

    0x23db3986,// 178 PAY 175 

    0x9ab7a1d0,// 179 PAY 176 

    0x3339240c,// 180 PAY 177 

    0xeeee3e0e,// 181 PAY 178 

    0x18897f08,// 182 PAY 179 

    0x94ec2c96,// 183 PAY 180 

    0x14f558a4,// 184 PAY 181 

    0x2e39978b,// 185 PAY 182 

    0x1837ea64,// 186 PAY 183 

    0xf795a602,// 187 PAY 184 

    0x063fe9f4,// 188 PAY 185 

    0x22b66927,// 189 PAY 186 

    0x68e9b3a7,// 190 PAY 187 

    0xdd296426,// 191 PAY 188 

    0x5ac210f2,// 192 PAY 189 

    0x974f6123,// 193 PAY 190 

    0xffdc6580,// 194 PAY 191 

    0x9154dc15,// 195 PAY 192 

    0x3d30fafb,// 196 PAY 193 

    0xde621c1f,// 197 PAY 194 

    0xca32228d,// 198 PAY 195 

    0x6f96df5b,// 199 PAY 196 

    0x4cc835ad,// 200 PAY 197 

    0x6a978ed0,// 201 PAY 198 

    0x6a53f675,// 202 PAY 199 

    0xb45d3d7b,// 203 PAY 200 

    0x46941c7b,// 204 PAY 201 

    0x14f27438,// 205 PAY 202 

    0x3cf6f53a,// 206 PAY 203 

    0x18e51029,// 207 PAY 204 

    0xe52cf42b,// 208 PAY 205 

    0xc4c89986,// 209 PAY 206 

    0x6a424614,// 210 PAY 207 

    0x22b74d53,// 211 PAY 208 

    0xf64dc69d,// 212 PAY 209 

    0x1459e87b,// 213 PAY 210 

    0x1bc72d7a,// 214 PAY 211 

    0xfc3d76c7,// 215 PAY 212 

    0x08e8f70c,// 216 PAY 213 

    0x7f3247af,// 217 PAY 214 

    0xffee02d1,// 218 PAY 215 

    0x001707c3,// 219 PAY 216 

    0x35a16832,// 220 PAY 217 

    0x8b57e16a,// 221 PAY 218 

    0xc863a4a6,// 222 PAY 219 

    0x86e6280f,// 223 PAY 220 

    0x19cbe3b8,// 224 PAY 221 

    0xee59dbde,// 225 PAY 222 

    0xd0fda965,// 226 PAY 223 

    0x29219140,// 227 PAY 224 

    0xa66339b3,// 228 PAY 225 

    0xd7818352,// 229 PAY 226 

    0x4141fd66,// 230 PAY 227 

    0xeccbc612,// 231 PAY 228 

    0xe71ea8c7,// 232 PAY 229 

    0x4a65e69d,// 233 PAY 230 

    0xc117737c,// 234 PAY 231 

    0x0021eb6c,// 235 PAY 232 

    0xd1e5ee18,// 236 PAY 233 

    0xa61ccd8d,// 237 PAY 234 

    0x6b83a3ba,// 238 PAY 235 

    0x60dcec72,// 239 PAY 236 

    0x86309f9f,// 240 PAY 237 

    0x5908f7d0,// 241 PAY 238 

    0x23037919,// 242 PAY 239 

    0x59ae0524,// 243 PAY 240 

    0x3d4f7064,// 244 PAY 241 

    0xbec04593,// 245 PAY 242 

    0xaca216ca,// 246 PAY 243 

    0x28dc3e5a,// 247 PAY 244 

    0x43aad52b,// 248 PAY 245 

    0xb0f07d56,// 249 PAY 246 

    0x30795180,// 250 PAY 247 

    0x34ef9e35,// 251 PAY 248 

    0xf2623ccb,// 252 PAY 249 

    0x9fa5c46a,// 253 PAY 250 

    0x8fd14981,// 254 PAY 251 

    0x025800ad,// 255 PAY 252 

    0xeaede4c0,// 256 PAY 253 

    0x27f6bb5a,// 257 PAY 254 

    0x4ff3523c,// 258 PAY 255 

    0x8040f667,// 259 PAY 256 

    0x12b9011a,// 260 PAY 257 

    0x9794f4ee,// 261 PAY 258 

    0xf5cbbded,// 262 PAY 259 

    0x8bc2f60d,// 263 PAY 260 

    0x84a4cea9,// 264 PAY 261 

    0x5ffeceea,// 265 PAY 262 

    0x0e55308c,// 266 PAY 263 

    0x69535c1c,// 267 PAY 264 

    0x63fdf839,// 268 PAY 265 

    0x91b12173,// 269 PAY 266 

    0xed388a2a,// 270 PAY 267 

    0x2d63e11d,// 271 PAY 268 

    0x11c19c89,// 272 PAY 269 

    0xbb7bcd25,// 273 PAY 270 

    0xf19784ea,// 274 PAY 271 

    0xa400b2da,// 275 PAY 272 

    0x6b48b449,// 276 PAY 273 

    0x8925b08e,// 277 PAY 274 

    0xd1b1cb14,// 278 PAY 275 

    0x1a435951,// 279 PAY 276 

    0x266661fe,// 280 PAY 277 

    0xffa78077,// 281 PAY 278 

    0x61615b2c,// 282 PAY 279 

    0x55f29be9,// 283 PAY 280 

    0x69b43a9c,// 284 PAY 281 

    0x8b1584aa,// 285 PAY 282 

    0xdd2324bb,// 286 PAY 283 

    0x3bc4fa5d,// 287 PAY 284 

    0x9f6c4f2a,// 288 PAY 285 

    0x130c69be,// 289 PAY 286 

    0x468d9ee2,// 290 PAY 287 

    0xa0e74101,// 291 PAY 288 

    0x7beffb68,// 292 PAY 289 

    0x3975f845,// 293 PAY 290 

    0x2771d777,// 294 PAY 291 

    0x0807f5bf,// 295 PAY 292 

    0x2c62eb9e,// 296 PAY 293 

    0x5f81e5d6,// 297 PAY 294 

    0xbb5a4f77,// 298 PAY 295 

    0x93ae6646,// 299 PAY 296 

    0xdcd3d63e,// 300 PAY 297 

    0xa5a40c64,// 301 PAY 298 

    0x96dda0af,// 302 PAY 299 

    0xde7251b2,// 303 PAY 300 

    0x4b572b03,// 304 PAY 301 

    0x23d22e3e,// 305 PAY 302 

    0x4c72183a,// 306 PAY 303 

    0xe40ff4c3,// 307 PAY 304 

    0x6a860000,// 308 PAY 305 

/// HASH is  8 bytes 

    0x6b83a3ba,// 309 HSH   1 

    0x60dcec72,// 310 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 171 

/// STA pkt_idx        : 56 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc7 

    0x00e0c7ab // 311 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt91_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 332 words. 

/// BDA size     is 1324 (0x52c) 

/// BDA id       is 0xd7b7 

    0x052cd7b7,// 3 BDA   1 

/// PAY Generic Data size   : 1324 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xa94ae5ed,// 4 PAY   1 

    0xa13975d5,// 5 PAY   2 

    0xc0053fdc,// 6 PAY   3 

    0x6b6424f4,// 7 PAY   4 

    0x2e3fe665,// 8 PAY   5 

    0x82e86673,// 9 PAY   6 

    0x2da2030f,// 10 PAY   7 

    0xcc7e1d8c,// 11 PAY   8 

    0x784335fb,// 12 PAY   9 

    0x2f4237eb,// 13 PAY  10 

    0xcbb3c632,// 14 PAY  11 

    0x9b78dcc6,// 15 PAY  12 

    0x26a9926a,// 16 PAY  13 

    0x9a874dc7,// 17 PAY  14 

    0x149c5c79,// 18 PAY  15 

    0xc175b14c,// 19 PAY  16 

    0x58bb5a82,// 20 PAY  17 

    0x88856fcb,// 21 PAY  18 

    0x6c079b00,// 22 PAY  19 

    0xe40f26df,// 23 PAY  20 

    0xf0c9334a,// 24 PAY  21 

    0xc7f11a11,// 25 PAY  22 

    0x051d7b35,// 26 PAY  23 

    0x773e10f5,// 27 PAY  24 

    0xf28a8e54,// 28 PAY  25 

    0x7b5d407e,// 29 PAY  26 

    0x616c9039,// 30 PAY  27 

    0x8b078394,// 31 PAY  28 

    0xfbbb802e,// 32 PAY  29 

    0x085fc462,// 33 PAY  30 

    0x60c24b4f,// 34 PAY  31 

    0x91f2db19,// 35 PAY  32 

    0x8363a6e3,// 36 PAY  33 

    0xa985e7d1,// 37 PAY  34 

    0xcba972f4,// 38 PAY  35 

    0xea81817d,// 39 PAY  36 

    0x80f94fe1,// 40 PAY  37 

    0x640be5bc,// 41 PAY  38 

    0xf7db2d4b,// 42 PAY  39 

    0x5ccfb42f,// 43 PAY  40 

    0x789d3d6c,// 44 PAY  41 

    0x7e8705d3,// 45 PAY  42 

    0x9a247ff8,// 46 PAY  43 

    0x5c3f7fa8,// 47 PAY  44 

    0xc00864fa,// 48 PAY  45 

    0x9dbd9b28,// 49 PAY  46 

    0x399917d7,// 50 PAY  47 

    0x47c86d30,// 51 PAY  48 

    0x10cfbd64,// 52 PAY  49 

    0xef59cf25,// 53 PAY  50 

    0x84b8a6f7,// 54 PAY  51 

    0x1df1a5d6,// 55 PAY  52 

    0x2a275a99,// 56 PAY  53 

    0x0ce8d359,// 57 PAY  54 

    0x6f2e1211,// 58 PAY  55 

    0xa403c705,// 59 PAY  56 

    0x2fa66c2d,// 60 PAY  57 

    0x4faec264,// 61 PAY  58 

    0x15ce63b2,// 62 PAY  59 

    0x533f5278,// 63 PAY  60 

    0xec079bc2,// 64 PAY  61 

    0xdd95f0f7,// 65 PAY  62 

    0x10709c44,// 66 PAY  63 

    0x17817dc7,// 67 PAY  64 

    0xc72ccdd4,// 68 PAY  65 

    0xe09e6a30,// 69 PAY  66 

    0x1ba8563b,// 70 PAY  67 

    0xbd9621f4,// 71 PAY  68 

    0x80ea120a,// 72 PAY  69 

    0xe50e2fc9,// 73 PAY  70 

    0x2c918a6b,// 74 PAY  71 

    0xb559e893,// 75 PAY  72 

    0xda6c169d,// 76 PAY  73 

    0x7a513c15,// 77 PAY  74 

    0x13285803,// 78 PAY  75 

    0x0d0b263d,// 79 PAY  76 

    0xe428296a,// 80 PAY  77 

    0xe72bfc9a,// 81 PAY  78 

    0x0a54e151,// 82 PAY  79 

    0x552cd65d,// 83 PAY  80 

    0x46362189,// 84 PAY  81 

    0xb30b2e21,// 85 PAY  82 

    0xb1f94e6c,// 86 PAY  83 

    0xae1b1ec7,// 87 PAY  84 

    0x98d22ca3,// 88 PAY  85 

    0x0ac62aee,// 89 PAY  86 

    0x1004ab49,// 90 PAY  87 

    0xe0c704ee,// 91 PAY  88 

    0xe9ebf5f7,// 92 PAY  89 

    0x0f3be7e7,// 93 PAY  90 

    0x0f05ab2e,// 94 PAY  91 

    0x53a7522b,// 95 PAY  92 

    0x5c5c34ac,// 96 PAY  93 

    0x18672b6e,// 97 PAY  94 

    0xdfb7dd6b,// 98 PAY  95 

    0xf8bb586d,// 99 PAY  96 

    0x08dc8226,// 100 PAY  97 

    0x01b96686,// 101 PAY  98 

    0xe0342de0,// 102 PAY  99 

    0x5a994910,// 103 PAY 100 

    0xb8958f5f,// 104 PAY 101 

    0x55a06d69,// 105 PAY 102 

    0x80ebb92a,// 106 PAY 103 

    0x244a0d4b,// 107 PAY 104 

    0x09cd9fe5,// 108 PAY 105 

    0x85665570,// 109 PAY 106 

    0xc5e78b86,// 110 PAY 107 

    0x01711e8d,// 111 PAY 108 

    0x02c7a8b1,// 112 PAY 109 

    0xe19707a9,// 113 PAY 110 

    0x3027fbde,// 114 PAY 111 

    0x6ca96df5,// 115 PAY 112 

    0x68f797ff,// 116 PAY 113 

    0xef8852c9,// 117 PAY 114 

    0x13b011f6,// 118 PAY 115 

    0x2d960962,// 119 PAY 116 

    0x88eed653,// 120 PAY 117 

    0x5deec498,// 121 PAY 118 

    0x43462b6a,// 122 PAY 119 

    0xeff21005,// 123 PAY 120 

    0xc251b5c1,// 124 PAY 121 

    0x4e72f341,// 125 PAY 122 

    0x55c59976,// 126 PAY 123 

    0x49eeba24,// 127 PAY 124 

    0x6d43bf53,// 128 PAY 125 

    0xa3633b06,// 129 PAY 126 

    0xcb2ab679,// 130 PAY 127 

    0x3aaebe69,// 131 PAY 128 

    0xd2294ae8,// 132 PAY 129 

    0x6a750978,// 133 PAY 130 

    0x6b71da6a,// 134 PAY 131 

    0x5f820dd7,// 135 PAY 132 

    0x578efd41,// 136 PAY 133 

    0x57255ec7,// 137 PAY 134 

    0x0250f70f,// 138 PAY 135 

    0x6cd78d6b,// 139 PAY 136 

    0x4f5844c2,// 140 PAY 137 

    0xe3e97e83,// 141 PAY 138 

    0x781b3575,// 142 PAY 139 

    0xd99980da,// 143 PAY 140 

    0xd551314b,// 144 PAY 141 

    0x69704eae,// 145 PAY 142 

    0x66c48e6e,// 146 PAY 143 

    0x6e30223e,// 147 PAY 144 

    0x8119b2a7,// 148 PAY 145 

    0x1fad9283,// 149 PAY 146 

    0x023526df,// 150 PAY 147 

    0x597a5f3f,// 151 PAY 148 

    0xfa0477c6,// 152 PAY 149 

    0x6c9f83f8,// 153 PAY 150 

    0xdfcd62dc,// 154 PAY 151 

    0x67262f63,// 155 PAY 152 

    0xcf59c690,// 156 PAY 153 

    0x7781ce7b,// 157 PAY 154 

    0x21e0cbec,// 158 PAY 155 

    0xd441d167,// 159 PAY 156 

    0x1d4d412e,// 160 PAY 157 

    0x7887e848,// 161 PAY 158 

    0xb328b8df,// 162 PAY 159 

    0xda687409,// 163 PAY 160 

    0x2bbf5c4b,// 164 PAY 161 

    0x7278da38,// 165 PAY 162 

    0x93a650f0,// 166 PAY 163 

    0xb5dca81a,// 167 PAY 164 

    0x4c1df80e,// 168 PAY 165 

    0x21401568,// 169 PAY 166 

    0xaf40822a,// 170 PAY 167 

    0xa14c2eb5,// 171 PAY 168 

    0xb09073f9,// 172 PAY 169 

    0xc6002ede,// 173 PAY 170 

    0xb817b7c9,// 174 PAY 171 

    0xe6d1431a,// 175 PAY 172 

    0x65bfdccc,// 176 PAY 173 

    0x4c3d5c2c,// 177 PAY 174 

    0x04e1964a,// 178 PAY 175 

    0x3e784489,// 179 PAY 176 

    0x9f1579ad,// 180 PAY 177 

    0x2d364347,// 181 PAY 178 

    0xcbd32f79,// 182 PAY 179 

    0x9e75f4d7,// 183 PAY 180 

    0xe451cf23,// 184 PAY 181 

    0xf8bc054a,// 185 PAY 182 

    0xd6e55aa3,// 186 PAY 183 

    0xad77038c,// 187 PAY 184 

    0x1b351a1c,// 188 PAY 185 

    0x3910c475,// 189 PAY 186 

    0x4ad73bf5,// 190 PAY 187 

    0xe53350bd,// 191 PAY 188 

    0x0c755e69,// 192 PAY 189 

    0x59207e41,// 193 PAY 190 

    0x32e9de34,// 194 PAY 191 

    0xcf9cbff1,// 195 PAY 192 

    0xc1796fd0,// 196 PAY 193 

    0x09927835,// 197 PAY 194 

    0x81a76803,// 198 PAY 195 

    0x9b2da28d,// 199 PAY 196 

    0x626d7bf2,// 200 PAY 197 

    0x622d2cc2,// 201 PAY 198 

    0x91e1a92c,// 202 PAY 199 

    0x22cd1e52,// 203 PAY 200 

    0xa338730e,// 204 PAY 201 

    0x28409df0,// 205 PAY 202 

    0x94d07af7,// 206 PAY 203 

    0x752a151e,// 207 PAY 204 

    0xbca9bbfb,// 208 PAY 205 

    0x9dab582f,// 209 PAY 206 

    0xa3aab236,// 210 PAY 207 

    0x34b3e7af,// 211 PAY 208 

    0x718ee023,// 212 PAY 209 

    0x35e2a4c7,// 213 PAY 210 

    0x3fa00833,// 214 PAY 211 

    0x26bf6005,// 215 PAY 212 

    0x11d190d5,// 216 PAY 213 

    0xddea935a,// 217 PAY 214 

    0x764be90a,// 218 PAY 215 

    0x10fbdefb,// 219 PAY 216 

    0x43a4835f,// 220 PAY 217 

    0x08077eec,// 221 PAY 218 

    0x1ffd22ca,// 222 PAY 219 

    0x9b2c3580,// 223 PAY 220 

    0xb6a53493,// 224 PAY 221 

    0xe01ab16c,// 225 PAY 222 

    0xcc8a9091,// 226 PAY 223 

    0xfca77153,// 227 PAY 224 

    0x0a222794,// 228 PAY 225 

    0x4278cef0,// 229 PAY 226 

    0xe022827f,// 230 PAY 227 

    0x48bc1118,// 231 PAY 228 

    0x23dfa350,// 232 PAY 229 

    0x7caf1fc1,// 233 PAY 230 

    0xee0e580f,// 234 PAY 231 

    0x77ae956d,// 235 PAY 232 

    0xd2eedbc5,// 236 PAY 233 

    0xf26adcf3,// 237 PAY 234 

    0x5c88903d,// 238 PAY 235 

    0xe01ac650,// 239 PAY 236 

    0x7fc51450,// 240 PAY 237 

    0x4a1508b5,// 241 PAY 238 

    0xff82bead,// 242 PAY 239 

    0x3c2a338f,// 243 PAY 240 

    0xe89522a4,// 244 PAY 241 

    0x8abe3173,// 245 PAY 242 

    0xb804783b,// 246 PAY 243 

    0xcd18afed,// 247 PAY 244 

    0xd6059b51,// 248 PAY 245 

    0x73070f50,// 249 PAY 246 

    0x63b5ef63,// 250 PAY 247 

    0x82a21d91,// 251 PAY 248 

    0x99fb6f68,// 252 PAY 249 

    0xd327f8c9,// 253 PAY 250 

    0x56f950ec,// 254 PAY 251 

    0xb24467e6,// 255 PAY 252 

    0x49236835,// 256 PAY 253 

    0xe2d1e8c4,// 257 PAY 254 

    0xa06225a3,// 258 PAY 255 

    0x8a3dd063,// 259 PAY 256 

    0xe954063f,// 260 PAY 257 

    0xd0ed8ebe,// 261 PAY 258 

    0xd4544d9a,// 262 PAY 259 

    0xa94de1da,// 263 PAY 260 

    0xef7a444b,// 264 PAY 261 

    0x6a301621,// 265 PAY 262 

    0x64543b86,// 266 PAY 263 

    0x637d5415,// 267 PAY 264 

    0x788cc451,// 268 PAY 265 

    0x5ce1b9a9,// 269 PAY 266 

    0xac7ed76c,// 270 PAY 267 

    0xd5dfc89b,// 271 PAY 268 

    0x720664a4,// 272 PAY 269 

    0x7ba4d45c,// 273 PAY 270 

    0x2a95f8a4,// 274 PAY 271 

    0xd72e7084,// 275 PAY 272 

    0x27ba4f3a,// 276 PAY 273 

    0x5afc5ee2,// 277 PAY 274 

    0x926818c3,// 278 PAY 275 

    0xc5f5bc6c,// 279 PAY 276 

    0x445c55cf,// 280 PAY 277 

    0x4a6aa1e5,// 281 PAY 278 

    0x08e13702,// 282 PAY 279 

    0x224e06c1,// 283 PAY 280 

    0xcb6ca93e,// 284 PAY 281 

    0x48689b0f,// 285 PAY 282 

    0x0b65dc4d,// 286 PAY 283 

    0x00a8abbb,// 287 PAY 284 

    0xa38c78a9,// 288 PAY 285 

    0xf6298991,// 289 PAY 286 

    0x132f0afd,// 290 PAY 287 

    0xf4ba7caf,// 291 PAY 288 

    0x2096bb1d,// 292 PAY 289 

    0xd3e5e50b,// 293 PAY 290 

    0xece3ca4a,// 294 PAY 291 

    0xb335785a,// 295 PAY 292 

    0x6de512e5,// 296 PAY 293 

    0x82d3dc9b,// 297 PAY 294 

    0xc2d97132,// 298 PAY 295 

    0xe1bfad2f,// 299 PAY 296 

    0x47b7be17,// 300 PAY 297 

    0x550d50c4,// 301 PAY 298 

    0xe5cc104a,// 302 PAY 299 

    0xd1e96348,// 303 PAY 300 

    0x3d49a72b,// 304 PAY 301 

    0x842feb95,// 305 PAY 302 

    0xcf875182,// 306 PAY 303 

    0xdd5d1288,// 307 PAY 304 

    0x4026bb64,// 308 PAY 305 

    0x87a997cc,// 309 PAY 306 

    0xb6b21666,// 310 PAY 307 

    0x1a233c1e,// 311 PAY 308 

    0x15c245fa,// 312 PAY 309 

    0xfc43a293,// 313 PAY 310 

    0xdb1f14b4,// 314 PAY 311 

    0xe0b29b25,// 315 PAY 312 

    0x57e8282e,// 316 PAY 313 

    0xce681376,// 317 PAY 314 

    0x0bf5b313,// 318 PAY 315 

    0x524f79d3,// 319 PAY 316 

    0xf55cde18,// 320 PAY 317 

    0x80a5f6e7,// 321 PAY 318 

    0x26b8887b,// 322 PAY 319 

    0x89b98ee2,// 323 PAY 320 

    0xc0f21a4b,// 324 PAY 321 

    0x45959653,// 325 PAY 322 

    0x0197c66a,// 326 PAY 323 

    0xba7751fd,// 327 PAY 324 

    0x56a64e38,// 328 PAY 325 

    0x56c5005a,// 329 PAY 326 

    0x5068a5b2,// 330 PAY 327 

    0xa02be3d3,// 331 PAY 328 

    0x06e3aa75,// 332 PAY 329 

    0xdf5f1031,// 333 PAY 330 

    0x38246149,// 334 PAY 331 

/// STA is 1 words. 

/// STA num_pkts       : 249 

/// STA pkt_idx        : 166 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x029904f9 // 335 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt92_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 454 words. 

/// BDA size     is 1812 (0x714) 

/// BDA id       is 0xb198 

    0x0714b198,// 3 BDA   1 

/// PAY Generic Data size   : 1812 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x93956634,// 4 PAY   1 

    0xac2e54f4,// 5 PAY   2 

    0x2ee62eb8,// 6 PAY   3 

    0xc57e182f,// 7 PAY   4 

    0x69afcf52,// 8 PAY   5 

    0x6c9fcdcf,// 9 PAY   6 

    0x039ba0a6,// 10 PAY   7 

    0xbbcffe9f,// 11 PAY   8 

    0xa81faffa,// 12 PAY   9 

    0x24222679,// 13 PAY  10 

    0xc983e6ea,// 14 PAY  11 

    0x62d006fe,// 15 PAY  12 

    0x62fb2c0a,// 16 PAY  13 

    0x70190ae1,// 17 PAY  14 

    0x0b80dec8,// 18 PAY  15 

    0x8d221649,// 19 PAY  16 

    0xb713995c,// 20 PAY  17 

    0x78e03a82,// 21 PAY  18 

    0x7736951a,// 22 PAY  19 

    0xc0f56d88,// 23 PAY  20 

    0x827575a8,// 24 PAY  21 

    0x214b8072,// 25 PAY  22 

    0x29573d58,// 26 PAY  23 

    0x22b7c428,// 27 PAY  24 

    0xae27fd08,// 28 PAY  25 

    0x8b46f9cf,// 29 PAY  26 

    0x45375ae1,// 30 PAY  27 

    0x4d917a85,// 31 PAY  28 

    0x50a9dfd5,// 32 PAY  29 

    0x3289700a,// 33 PAY  30 

    0x4f4b9a92,// 34 PAY  31 

    0xebdc990b,// 35 PAY  32 

    0xd140d0fa,// 36 PAY  33 

    0x294cda16,// 37 PAY  34 

    0xa05efa6d,// 38 PAY  35 

    0xc4813a35,// 39 PAY  36 

    0xcb97b144,// 40 PAY  37 

    0xe733f6e8,// 41 PAY  38 

    0x7fbfaac2,// 42 PAY  39 

    0xb11dd5c8,// 43 PAY  40 

    0xdfdc7d00,// 44 PAY  41 

    0x0885df57,// 45 PAY  42 

    0xe088593a,// 46 PAY  43 

    0xb70bb527,// 47 PAY  44 

    0xdbc399eb,// 48 PAY  45 

    0x513ad11a,// 49 PAY  46 

    0x7cb9525f,// 50 PAY  47 

    0x8d57c0f1,// 51 PAY  48 

    0x47045b10,// 52 PAY  49 

    0xc77a57a1,// 53 PAY  50 

    0x5ac79f67,// 54 PAY  51 

    0xe20d6516,// 55 PAY  52 

    0x04a2c826,// 56 PAY  53 

    0xe95a6100,// 57 PAY  54 

    0xd58f36aa,// 58 PAY  55 

    0x13d577f0,// 59 PAY  56 

    0x444d2a94,// 60 PAY  57 

    0x0e159876,// 61 PAY  58 

    0x8ddeaaa9,// 62 PAY  59 

    0x2fe36fc7,// 63 PAY  60 

    0x3a732456,// 64 PAY  61 

    0x8889ba5c,// 65 PAY  62 

    0x42f048f2,// 66 PAY  63 

    0x2140220a,// 67 PAY  64 

    0x92008268,// 68 PAY  65 

    0x33f7c5cf,// 69 PAY  66 

    0x896962a3,// 70 PAY  67 

    0x5315ddc1,// 71 PAY  68 

    0xd740e12e,// 72 PAY  69 

    0x23fd8d28,// 73 PAY  70 

    0x7aaaa469,// 74 PAY  71 

    0x303caca4,// 75 PAY  72 

    0x93679216,// 76 PAY  73 

    0xebe7a94c,// 77 PAY  74 

    0x6eb84fb0,// 78 PAY  75 

    0xbd7fc9f7,// 79 PAY  76 

    0x987eb5d9,// 80 PAY  77 

    0x3723bc04,// 81 PAY  78 

    0x803ef034,// 82 PAY  79 

    0x26a84154,// 83 PAY  80 

    0x0e891c50,// 84 PAY  81 

    0xb9dce1f2,// 85 PAY  82 

    0x6dd7fe41,// 86 PAY  83 

    0x37e680f4,// 87 PAY  84 

    0x203463c5,// 88 PAY  85 

    0x4724cb13,// 89 PAY  86 

    0x8dadccec,// 90 PAY  87 

    0xcc332e82,// 91 PAY  88 

    0xdf3ee0ca,// 92 PAY  89 

    0x8b7ce787,// 93 PAY  90 

    0x7bc784d6,// 94 PAY  91 

    0xbf48740d,// 95 PAY  92 

    0x7449b81d,// 96 PAY  93 

    0xdab0771f,// 97 PAY  94 

    0x1695da6b,// 98 PAY  95 

    0xacd4b757,// 99 PAY  96 

    0x5a48ebc9,// 100 PAY  97 

    0x501dfdde,// 101 PAY  98 

    0x8dea918a,// 102 PAY  99 

    0x0ba11f9e,// 103 PAY 100 

    0x8b58eaad,// 104 PAY 101 

    0x3fb74e7a,// 105 PAY 102 

    0xdf36eb41,// 106 PAY 103 

    0x72da5c50,// 107 PAY 104 

    0xf2c1c5a8,// 108 PAY 105 

    0xca2975e5,// 109 PAY 106 

    0x7c8560fb,// 110 PAY 107 

    0x8b091dd4,// 111 PAY 108 

    0x58300830,// 112 PAY 109 

    0xcba9c167,// 113 PAY 110 

    0xd116edb0,// 114 PAY 111 

    0x2fa1d3a8,// 115 PAY 112 

    0x657d4e74,// 116 PAY 113 

    0x8cb8f435,// 117 PAY 114 

    0x2c9415e6,// 118 PAY 115 

    0xa0d6f634,// 119 PAY 116 

    0xec8a9eff,// 120 PAY 117 

    0x28f034a4,// 121 PAY 118 

    0x005a9b81,// 122 PAY 119 

    0xc3a4a397,// 123 PAY 120 

    0xebd006d0,// 124 PAY 121 

    0x9008a1de,// 125 PAY 122 

    0xd7120e7e,// 126 PAY 123 

    0x8c8e295e,// 127 PAY 124 

    0x2887e449,// 128 PAY 125 

    0xae161490,// 129 PAY 126 

    0xd6ed08da,// 130 PAY 127 

    0x8f170914,// 131 PAY 128 

    0x2060e542,// 132 PAY 129 

    0x2f8febbb,// 133 PAY 130 

    0x652e1e43,// 134 PAY 131 

    0x844b46fc,// 135 PAY 132 

    0x34a0c01f,// 136 PAY 133 

    0x420ac7b6,// 137 PAY 134 

    0xbf9f7fb3,// 138 PAY 135 

    0x427c57d2,// 139 PAY 136 

    0xc7981ae9,// 140 PAY 137 

    0x7aacc441,// 141 PAY 138 

    0x18a76c4e,// 142 PAY 139 

    0x0f75a0b5,// 143 PAY 140 

    0xf9b46484,// 144 PAY 141 

    0x181d623d,// 145 PAY 142 

    0x24e1d346,// 146 PAY 143 

    0xd6b102dc,// 147 PAY 144 

    0x336ef6c2,// 148 PAY 145 

    0xc50d32ba,// 149 PAY 146 

    0xd52e5543,// 150 PAY 147 

    0x48252475,// 151 PAY 148 

    0xe6a7daf0,// 152 PAY 149 

    0x94bc26b0,// 153 PAY 150 

    0xbafbd858,// 154 PAY 151 

    0x14c6c98a,// 155 PAY 152 

    0x8d496657,// 156 PAY 153 

    0xad689321,// 157 PAY 154 

    0xc99cc1f2,// 158 PAY 155 

    0xa55dc075,// 159 PAY 156 

    0xde5ffde8,// 160 PAY 157 

    0x56a2f78e,// 161 PAY 158 

    0x582df864,// 162 PAY 159 

    0x9c3cd0f9,// 163 PAY 160 

    0x7100efa2,// 164 PAY 161 

    0x45db29b9,// 165 PAY 162 

    0xca7ac23c,// 166 PAY 163 

    0xe71d1e65,// 167 PAY 164 

    0x8f8a8036,// 168 PAY 165 

    0x93ae6783,// 169 PAY 166 

    0xbac9708b,// 170 PAY 167 

    0xe0f463db,// 171 PAY 168 

    0x6a3a5f12,// 172 PAY 169 

    0xf46ad3d9,// 173 PAY 170 

    0x9861c50b,// 174 PAY 171 

    0xa6919063,// 175 PAY 172 

    0x79300952,// 176 PAY 173 

    0x4060c496,// 177 PAY 174 

    0xe941d5a6,// 178 PAY 175 

    0xdc88d0e0,// 179 PAY 176 

    0xe07580cd,// 180 PAY 177 

    0x232e104d,// 181 PAY 178 

    0x1bdb2ad2,// 182 PAY 179 

    0xfc3f4ea4,// 183 PAY 180 

    0x4af6f384,// 184 PAY 181 

    0xd1751c0c,// 185 PAY 182 

    0x594b123e,// 186 PAY 183 

    0xb8e65c82,// 187 PAY 184 

    0xd35ee49c,// 188 PAY 185 

    0x2d2d2921,// 189 PAY 186 

    0xa0aebfd1,// 190 PAY 187 

    0x20e940f4,// 191 PAY 188 

    0x9f40acab,// 192 PAY 189 

    0xa1c7d9d7,// 193 PAY 190 

    0x96a9e3fe,// 194 PAY 191 

    0xe1fa3c2d,// 195 PAY 192 

    0x1c6011e5,// 196 PAY 193 

    0x659470bd,// 197 PAY 194 

    0xb6f87b80,// 198 PAY 195 

    0xe1790307,// 199 PAY 196 

    0x7ccf19a6,// 200 PAY 197 

    0x5743b32d,// 201 PAY 198 

    0x80c23295,// 202 PAY 199 

    0xff373de8,// 203 PAY 200 

    0x0cf54768,// 204 PAY 201 

    0x9e396a23,// 205 PAY 202 

    0xfecd78a1,// 206 PAY 203 

    0x05825990,// 207 PAY 204 

    0xbc80efe6,// 208 PAY 205 

    0x47e02a57,// 209 PAY 206 

    0x55915c5a,// 210 PAY 207 

    0x463c781f,// 211 PAY 208 

    0x7bbc768a,// 212 PAY 209 

    0x12189641,// 213 PAY 210 

    0xdbccfb53,// 214 PAY 211 

    0x375faff1,// 215 PAY 212 

    0x2c0ced3c,// 216 PAY 213 

    0x5743600b,// 217 PAY 214 

    0xbd702f91,// 218 PAY 215 

    0x3400fe69,// 219 PAY 216 

    0xe0dbff30,// 220 PAY 217 

    0x017404f4,// 221 PAY 218 

    0x3e2d6ee4,// 222 PAY 219 

    0xc537192b,// 223 PAY 220 

    0x2c389351,// 224 PAY 221 

    0x58af5629,// 225 PAY 222 

    0x5e75023b,// 226 PAY 223 

    0x055fe7f7,// 227 PAY 224 

    0x7e3c4f06,// 228 PAY 225 

    0xf2b46803,// 229 PAY 226 

    0x32eb10ae,// 230 PAY 227 

    0xf56b7036,// 231 PAY 228 

    0x821767e9,// 232 PAY 229 

    0xd5f2eecb,// 233 PAY 230 

    0x25fbe6f7,// 234 PAY 231 

    0x1be7d8cc,// 235 PAY 232 

    0x52a62b61,// 236 PAY 233 

    0x01902fcb,// 237 PAY 234 

    0xe9a035e1,// 238 PAY 235 

    0x55cd16ec,// 239 PAY 236 

    0xe9e1bbe2,// 240 PAY 237 

    0xf54fe160,// 241 PAY 238 

    0x4c0eefe8,// 242 PAY 239 

    0xf7ad6b00,// 243 PAY 240 

    0x77e7c852,// 244 PAY 241 

    0x7057f34b,// 245 PAY 242 

    0x38ca12d5,// 246 PAY 243 

    0x01f84f8b,// 247 PAY 244 

    0x91551e0b,// 248 PAY 245 

    0x8afbe30a,// 249 PAY 246 

    0x2bd3abfd,// 250 PAY 247 

    0x847935ce,// 251 PAY 248 

    0x48d1227e,// 252 PAY 249 

    0x36fdd874,// 253 PAY 250 

    0x0e5873b3,// 254 PAY 251 

    0xfc9c7f90,// 255 PAY 252 

    0xcd4a9e93,// 256 PAY 253 

    0x63f73e72,// 257 PAY 254 

    0x27daa075,// 258 PAY 255 

    0xafadc7a5,// 259 PAY 256 

    0xf15dbb5d,// 260 PAY 257 

    0x9983bdbd,// 261 PAY 258 

    0xb2d6ae97,// 262 PAY 259 

    0x3498e5f3,// 263 PAY 260 

    0x37c2d4ac,// 264 PAY 261 

    0xfc069c43,// 265 PAY 262 

    0x60bd7938,// 266 PAY 263 

    0x07eb1c4c,// 267 PAY 264 

    0x1d7388b8,// 268 PAY 265 

    0x591b3114,// 269 PAY 266 

    0xbf7a6cb7,// 270 PAY 267 

    0x47f80510,// 271 PAY 268 

    0x5a096594,// 272 PAY 269 

    0xe6dde498,// 273 PAY 270 

    0x60d57deb,// 274 PAY 271 

    0xb85321a8,// 275 PAY 272 

    0xa5b64887,// 276 PAY 273 

    0x8eb8af85,// 277 PAY 274 

    0x46793f7b,// 278 PAY 275 

    0x93296c73,// 279 PAY 276 

    0xe59e29d2,// 280 PAY 277 

    0x20dbd0b6,// 281 PAY 278 

    0xd4d1d885,// 282 PAY 279 

    0x9429b792,// 283 PAY 280 

    0x39a8335f,// 284 PAY 281 

    0xd7c51a2c,// 285 PAY 282 

    0x3236365f,// 286 PAY 283 

    0x62f77ec5,// 287 PAY 284 

    0xf53b0664,// 288 PAY 285 

    0xd5a30a01,// 289 PAY 286 

    0x84e02c90,// 290 PAY 287 

    0xdbc747a7,// 291 PAY 288 

    0xdf4a5c36,// 292 PAY 289 

    0xcc0499af,// 293 PAY 290 

    0x4053fb89,// 294 PAY 291 

    0xa94f82f8,// 295 PAY 292 

    0x31158568,// 296 PAY 293 

    0x217f22f9,// 297 PAY 294 

    0x42c7df93,// 298 PAY 295 

    0x353bd80b,// 299 PAY 296 

    0xb9578333,// 300 PAY 297 

    0x329352e2,// 301 PAY 298 

    0x96114ebb,// 302 PAY 299 

    0xc3524810,// 303 PAY 300 

    0xc6b54ae4,// 304 PAY 301 

    0xa1e6d568,// 305 PAY 302 

    0xfbc065c6,// 306 PAY 303 

    0x67d407ac,// 307 PAY 304 

    0x26705672,// 308 PAY 305 

    0x73ae8cbe,// 309 PAY 306 

    0x48d81547,// 310 PAY 307 

    0x087ad977,// 311 PAY 308 

    0x0a3e205c,// 312 PAY 309 

    0x5ff13863,// 313 PAY 310 

    0x4e47e22a,// 314 PAY 311 

    0xdcd67823,// 315 PAY 312 

    0xd088a340,// 316 PAY 313 

    0x8b065c8f,// 317 PAY 314 

    0x940ad39f,// 318 PAY 315 

    0xcb1452e1,// 319 PAY 316 

    0x632bb761,// 320 PAY 317 

    0xf9fc0f39,// 321 PAY 318 

    0x02b81e42,// 322 PAY 319 

    0x95604614,// 323 PAY 320 

    0x6acf0936,// 324 PAY 321 

    0xc6c8944d,// 325 PAY 322 

    0x245423d2,// 326 PAY 323 

    0x43d37d8e,// 327 PAY 324 

    0x3155dc5e,// 328 PAY 325 

    0xfe4dafbc,// 329 PAY 326 

    0x32356392,// 330 PAY 327 

    0x1ebef623,// 331 PAY 328 

    0xc9a6a159,// 332 PAY 329 

    0x4fee8d9e,// 333 PAY 330 

    0x7b4db603,// 334 PAY 331 

    0x66983080,// 335 PAY 332 

    0x50454382,// 336 PAY 333 

    0xd7d5f2ef,// 337 PAY 334 

    0x41fbf298,// 338 PAY 335 

    0x8d368486,// 339 PAY 336 

    0x30e135ce,// 340 PAY 337 

    0x0e51f29d,// 341 PAY 338 

    0xe90ce1bc,// 342 PAY 339 

    0x090e906a,// 343 PAY 340 

    0x870e06d2,// 344 PAY 341 

    0xd3c5f206,// 345 PAY 342 

    0x9674a1f7,// 346 PAY 343 

    0x5e1eb12a,// 347 PAY 344 

    0x443bfa7d,// 348 PAY 345 

    0xe563e7e9,// 349 PAY 346 

    0xbb353677,// 350 PAY 347 

    0x5dc6bf96,// 351 PAY 348 

    0xd8b50bb8,// 352 PAY 349 

    0xcf9dfca1,// 353 PAY 350 

    0xfa745902,// 354 PAY 351 

    0xae739955,// 355 PAY 352 

    0xb408b41c,// 356 PAY 353 

    0xf911f258,// 357 PAY 354 

    0x7eccdeda,// 358 PAY 355 

    0xe7b46e8b,// 359 PAY 356 

    0xd75d3d29,// 360 PAY 357 

    0x9b994967,// 361 PAY 358 

    0x67193ca5,// 362 PAY 359 

    0x5fea921d,// 363 PAY 360 

    0x75b419ef,// 364 PAY 361 

    0x07ddacef,// 365 PAY 362 

    0x728682cf,// 366 PAY 363 

    0x20752c66,// 367 PAY 364 

    0xceb48ce5,// 368 PAY 365 

    0xb97d0c12,// 369 PAY 366 

    0x9920cc73,// 370 PAY 367 

    0x0f66ac1a,// 371 PAY 368 

    0xf8c6da12,// 372 PAY 369 

    0xfe732fdb,// 373 PAY 370 

    0x0126c643,// 374 PAY 371 

    0x3cc20a70,// 375 PAY 372 

    0x4d208d0e,// 376 PAY 373 

    0xe44d9b1e,// 377 PAY 374 

    0x048ba3cc,// 378 PAY 375 

    0x41dbf064,// 379 PAY 376 

    0x7fb5ffb7,// 380 PAY 377 

    0x80c0f619,// 381 PAY 378 

    0x4bb2c9fe,// 382 PAY 379 

    0xe6e1f482,// 383 PAY 380 

    0xaf2361c6,// 384 PAY 381 

    0x411931d5,// 385 PAY 382 

    0xe0ce3d44,// 386 PAY 383 

    0x6adc0c74,// 387 PAY 384 

    0x24632373,// 388 PAY 385 

    0x051e2239,// 389 PAY 386 

    0xdfbe407d,// 390 PAY 387 

    0xdf44a947,// 391 PAY 388 

    0xbcd591a5,// 392 PAY 389 

    0xb01545a0,// 393 PAY 390 

    0x1bba424e,// 394 PAY 391 

    0x531e521c,// 395 PAY 392 

    0xd506c0cc,// 396 PAY 393 

    0xefab7fae,// 397 PAY 394 

    0xb15fdd73,// 398 PAY 395 

    0xa1666d87,// 399 PAY 396 

    0xb10c6e44,// 400 PAY 397 

    0xf9183a0f,// 401 PAY 398 

    0x8d7e3d7d,// 402 PAY 399 

    0x07c06d08,// 403 PAY 400 

    0x155204ef,// 404 PAY 401 

    0x0b9c5266,// 405 PAY 402 

    0xba4a7769,// 406 PAY 403 

    0x2e84a2fa,// 407 PAY 404 

    0x9d1c4013,// 408 PAY 405 

    0x737fb8da,// 409 PAY 406 

    0x501848cc,// 410 PAY 407 

    0x57921784,// 411 PAY 408 

    0x80ef1618,// 412 PAY 409 

    0x27ac61dd,// 413 PAY 410 

    0x14206765,// 414 PAY 411 

    0xefc58bcd,// 415 PAY 412 

    0xd68834b5,// 416 PAY 413 

    0xaa225669,// 417 PAY 414 

    0xdd1efe7f,// 418 PAY 415 

    0x4f37ae3d,// 419 PAY 416 

    0xab03fbc7,// 420 PAY 417 

    0x9084a19f,// 421 PAY 418 

    0xd734d7fb,// 422 PAY 419 

    0x5b43474c,// 423 PAY 420 

    0x1d309394,// 424 PAY 421 

    0xd8729d94,// 425 PAY 422 

    0xf04664f1,// 426 PAY 423 

    0xd2ba8841,// 427 PAY 424 

    0xd0c32553,// 428 PAY 425 

    0x84e0e28a,// 429 PAY 426 

    0xcdbf8926,// 430 PAY 427 

    0x01ea261b,// 431 PAY 428 

    0x3b68772f,// 432 PAY 429 

    0xe64941c3,// 433 PAY 430 

    0x7f23ee96,// 434 PAY 431 

    0x381f99eb,// 435 PAY 432 

    0xf46bc424,// 436 PAY 433 

    0xe772f55a,// 437 PAY 434 

    0x51749e4f,// 438 PAY 435 

    0x5441db8d,// 439 PAY 436 

    0xf76024a2,// 440 PAY 437 

    0x3825ec45,// 441 PAY 438 

    0xfdaacbb0,// 442 PAY 439 

    0x016c9e97,// 443 PAY 440 

    0xc4f7d734,// 444 PAY 441 

    0xf2d4a1ca,// 445 PAY 442 

    0xd734e87b,// 446 PAY 443 

    0x3746315c,// 447 PAY 444 

    0x6c066415,// 448 PAY 445 

    0xb601c09a,// 449 PAY 446 

    0x7983cd02,// 450 PAY 447 

    0x101f765b,// 451 PAY 448 

    0xe5913bac,// 452 PAY 449 

    0xdd4f03d5,// 453 PAY 450 

    0x07f7d05f,// 454 PAY 451 

    0x1b30c32f,// 455 PAY 452 

    0x9971041b,// 456 PAY 453 

/// HASH is  12 bytes 

    0xd35ee49c,// 457 HSH   1 

    0x2d2d2921,// 458 HSH   2 

    0xa0aebfd1,// 459 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 86 

/// STA pkt_idx        : 224 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf3 

    0x0381f356 // 460 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt93_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 182 words. 

/// BDA size     is 722 (0x2d2) 

/// BDA id       is 0x3f09 

    0x02d23f09,// 3 BDA   1 

/// PAY Generic Data size   : 722 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x99e139da,// 4 PAY   1 

    0x1d863067,// 5 PAY   2 

    0x0bf4f12d,// 6 PAY   3 

    0xdc6d221e,// 7 PAY   4 

    0x8ac07240,// 8 PAY   5 

    0x11700772,// 9 PAY   6 

    0xa3d5ced6,// 10 PAY   7 

    0x3b8e5a5e,// 11 PAY   8 

    0x1348a628,// 12 PAY   9 

    0x8f6f3350,// 13 PAY  10 

    0x501052af,// 14 PAY  11 

    0xcc2442d2,// 15 PAY  12 

    0x4545d300,// 16 PAY  13 

    0xa2a5dace,// 17 PAY  14 

    0x40bc4f30,// 18 PAY  15 

    0x6afc421d,// 19 PAY  16 

    0xda55f633,// 20 PAY  17 

    0xc19a37b1,// 21 PAY  18 

    0x3eb4b0d7,// 22 PAY  19 

    0xe3b7271b,// 23 PAY  20 

    0x98007093,// 24 PAY  21 

    0x7ed22aba,// 25 PAY  22 

    0x064e4ce6,// 26 PAY  23 

    0x7e144191,// 27 PAY  24 

    0x20c43de4,// 28 PAY  25 

    0x1183a50f,// 29 PAY  26 

    0x6b9d03dd,// 30 PAY  27 

    0xd84702e7,// 31 PAY  28 

    0xbe69325c,// 32 PAY  29 

    0xd977bb79,// 33 PAY  30 

    0xd4a49648,// 34 PAY  31 

    0x93600324,// 35 PAY  32 

    0x76ac4f30,// 36 PAY  33 

    0x17ab6630,// 37 PAY  34 

    0x705c3283,// 38 PAY  35 

    0xcd093c1f,// 39 PAY  36 

    0x43817742,// 40 PAY  37 

    0x9563f15c,// 41 PAY  38 

    0x86b6c8e7,// 42 PAY  39 

    0x41342e2e,// 43 PAY  40 

    0x5c5d3a1f,// 44 PAY  41 

    0xe417fe91,// 45 PAY  42 

    0x3f81ea85,// 46 PAY  43 

    0x2cb2f8d6,// 47 PAY  44 

    0x3e35d9b2,// 48 PAY  45 

    0xd0145a57,// 49 PAY  46 

    0x4b868dc9,// 50 PAY  47 

    0x198ee44d,// 51 PAY  48 

    0xcf952679,// 52 PAY  49 

    0x77fc4606,// 53 PAY  50 

    0x8993a398,// 54 PAY  51 

    0xdcc10d03,// 55 PAY  52 

    0xea6db115,// 56 PAY  53 

    0xc9bc8b1f,// 57 PAY  54 

    0xab8490b8,// 58 PAY  55 

    0x8ef674d8,// 59 PAY  56 

    0x8a460d4a,// 60 PAY  57 

    0x99dbb44c,// 61 PAY  58 

    0x26b7adc6,// 62 PAY  59 

    0x25530840,// 63 PAY  60 

    0xbf0368ae,// 64 PAY  61 

    0x751f0ac4,// 65 PAY  62 

    0x82d62175,// 66 PAY  63 

    0x67912e13,// 67 PAY  64 

    0x7ed64456,// 68 PAY  65 

    0xc879e0ab,// 69 PAY  66 

    0x36cc84a9,// 70 PAY  67 

    0x9e66f23a,// 71 PAY  68 

    0x7a3cbb2b,// 72 PAY  69 

    0x3d1deb2a,// 73 PAY  70 

    0xcfca9899,// 74 PAY  71 

    0x4744eb32,// 75 PAY  72 

    0xf3d8b8f3,// 76 PAY  73 

    0x79dca149,// 77 PAY  74 

    0x3002f0cb,// 78 PAY  75 

    0x93a2100e,// 79 PAY  76 

    0x808aeffd,// 80 PAY  77 

    0xbb3f6195,// 81 PAY  78 

    0xf9baab17,// 82 PAY  79 

    0x43456ded,// 83 PAY  80 

    0x4022cc47,// 84 PAY  81 

    0x4a318f66,// 85 PAY  82 

    0x10a5540f,// 86 PAY  83 

    0xf8795a87,// 87 PAY  84 

    0x0a2f9b6c,// 88 PAY  85 

    0x1e911884,// 89 PAY  86 

    0x5a8cf1c9,// 90 PAY  87 

    0x020fce64,// 91 PAY  88 

    0x84c53709,// 92 PAY  89 

    0x42fdf1d0,// 93 PAY  90 

    0xd16a9874,// 94 PAY  91 

    0x2948a849,// 95 PAY  92 

    0x42365921,// 96 PAY  93 

    0xe0a9df55,// 97 PAY  94 

    0xcbaf07e8,// 98 PAY  95 

    0xd84941c8,// 99 PAY  96 

    0x283c94bd,// 100 PAY  97 

    0xa462ade3,// 101 PAY  98 

    0x082f7a11,// 102 PAY  99 

    0xe571230a,// 103 PAY 100 

    0x6f04f030,// 104 PAY 101 

    0x8ceef6ae,// 105 PAY 102 

    0x3de98026,// 106 PAY 103 

    0xad9235c4,// 107 PAY 104 

    0xdbdfcc9d,// 108 PAY 105 

    0x4db45f65,// 109 PAY 106 

    0xa54bb6fb,// 110 PAY 107 

    0xe29a56e6,// 111 PAY 108 

    0xe2c53a7a,// 112 PAY 109 

    0x54582d69,// 113 PAY 110 

    0x23bcd840,// 114 PAY 111 

    0xb95e2755,// 115 PAY 112 

    0x11a8284c,// 116 PAY 113 

    0x7b2fe345,// 117 PAY 114 

    0x1acf6bcf,// 118 PAY 115 

    0x11dd138f,// 119 PAY 116 

    0x6e5ea555,// 120 PAY 117 

    0x36d86a78,// 121 PAY 118 

    0x1afffcd5,// 122 PAY 119 

    0xc808d2c3,// 123 PAY 120 

    0x064cd8d4,// 124 PAY 121 

    0xe51f6a10,// 125 PAY 122 

    0xbed1f10b,// 126 PAY 123 

    0x913e0938,// 127 PAY 124 

    0x12cb3c40,// 128 PAY 125 

    0x1beea3fa,// 129 PAY 126 

    0x66e2f58a,// 130 PAY 127 

    0x833c3983,// 131 PAY 128 

    0x13181d9e,// 132 PAY 129 

    0x7f469962,// 133 PAY 130 

    0x8061ea0b,// 134 PAY 131 

    0x23634c37,// 135 PAY 132 

    0x2012893f,// 136 PAY 133 

    0x0d2c01da,// 137 PAY 134 

    0x22310f71,// 138 PAY 135 

    0x86176bfd,// 139 PAY 136 

    0x23ca0d6f,// 140 PAY 137 

    0xe1e8cdb0,// 141 PAY 138 

    0x6a7411aa,// 142 PAY 139 

    0x13b526bf,// 143 PAY 140 

    0x46555353,// 144 PAY 141 

    0xe4f31aa5,// 145 PAY 142 

    0x165df573,// 146 PAY 143 

    0x6afd8c5f,// 147 PAY 144 

    0xee6ef06e,// 148 PAY 145 

    0x22530d45,// 149 PAY 146 

    0xf193a76f,// 150 PAY 147 

    0xf9e0d669,// 151 PAY 148 

    0xa33f69dc,// 152 PAY 149 

    0x0ae8ad87,// 153 PAY 150 

    0x6e4d77a1,// 154 PAY 151 

    0x14915816,// 155 PAY 152 

    0x5a71ccb8,// 156 PAY 153 

    0xfc612a26,// 157 PAY 154 

    0x3b6b7b2a,// 158 PAY 155 

    0x2b8800ca,// 159 PAY 156 

    0x13997fd0,// 160 PAY 157 

    0x47ff49b8,// 161 PAY 158 

    0x3ae23fba,// 162 PAY 159 

    0x8a0c6a11,// 163 PAY 160 

    0xd66f3598,// 164 PAY 161 

    0xbbefbe7f,// 165 PAY 162 

    0x8f204e81,// 166 PAY 163 

    0xe8657f50,// 167 PAY 164 

    0x4d0ecf01,// 168 PAY 165 

    0x6af53178,// 169 PAY 166 

    0xd125a9b8,// 170 PAY 167 

    0x3d0dd20f,// 171 PAY 168 

    0x6e97ed83,// 172 PAY 169 

    0x8a491a23,// 173 PAY 170 

    0xb11d1e74,// 174 PAY 171 

    0x7f1b3da0,// 175 PAY 172 

    0xeb120767,// 176 PAY 173 

    0xa3600ea8,// 177 PAY 174 

    0xd6fb2cf9,// 178 PAY 175 

    0x6206162f,// 179 PAY 176 

    0x22b56171,// 180 PAY 177 

    0x08264082,// 181 PAY 178 

    0xc621e773,// 182 PAY 179 

    0xcdcc6fd6,// 183 PAY 180 

    0x36630000,// 184 PAY 181 

/// HASH is  8 bytes 

    0xe8657f50,// 185 HSH   1 

    0x4d0ecf01,// 186 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 67 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd1 

    0x0168d143 // 187 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt94_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 263 words. 

/// BDA size     is 1048 (0x418) 

/// BDA id       is 0xda3e 

    0x0418da3e,// 3 BDA   1 

/// PAY Generic Data size   : 1048 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xdaee88b1,// 4 PAY   1 

    0xdb9175c7,// 5 PAY   2 

    0xb27339af,// 6 PAY   3 

    0x295ebe4a,// 7 PAY   4 

    0x04e234ae,// 8 PAY   5 

    0xaf94b315,// 9 PAY   6 

    0x0a6fe468,// 10 PAY   7 

    0xc93e48b8,// 11 PAY   8 

    0x9723fa49,// 12 PAY   9 

    0x9490d894,// 13 PAY  10 

    0xa20871e9,// 14 PAY  11 

    0x67a6f9ff,// 15 PAY  12 

    0x5877fb44,// 16 PAY  13 

    0xbe95b93d,// 17 PAY  14 

    0xe6fd1b43,// 18 PAY  15 

    0x565bb61f,// 19 PAY  16 

    0xd73347ad,// 20 PAY  17 

    0x52595979,// 21 PAY  18 

    0x33164b4d,// 22 PAY  19 

    0xe9f470db,// 23 PAY  20 

    0xa0820ca2,// 24 PAY  21 

    0xd3182938,// 25 PAY  22 

    0xdfc3f3ac,// 26 PAY  23 

    0x03ba508c,// 27 PAY  24 

    0xd50e3825,// 28 PAY  25 

    0x498becf0,// 29 PAY  26 

    0x24fee6d5,// 30 PAY  27 

    0x93d21bc1,// 31 PAY  28 

    0x46d131f3,// 32 PAY  29 

    0xd26fcece,// 33 PAY  30 

    0xec715276,// 34 PAY  31 

    0x325fe957,// 35 PAY  32 

    0xc4cced41,// 36 PAY  33 

    0x0ba9ac1d,// 37 PAY  34 

    0x4592b97d,// 38 PAY  35 

    0xa213b635,// 39 PAY  36 

    0x7612c32d,// 40 PAY  37 

    0xf28f3ec4,// 41 PAY  38 

    0x401dfd75,// 42 PAY  39 

    0x46555470,// 43 PAY  40 

    0x59be70e0,// 44 PAY  41 

    0x0821f3a8,// 45 PAY  42 

    0xb0913683,// 46 PAY  43 

    0xef30dce4,// 47 PAY  44 

    0x01c932c7,// 48 PAY  45 

    0xe10afb34,// 49 PAY  46 

    0xc6489287,// 50 PAY  47 

    0x72572b32,// 51 PAY  48 

    0x53fd25bf,// 52 PAY  49 

    0x7b7fd112,// 53 PAY  50 

    0xf4578e75,// 54 PAY  51 

    0x63aaa640,// 55 PAY  52 

    0x938bd77c,// 56 PAY  53 

    0xd605b871,// 57 PAY  54 

    0x5ebc48e3,// 58 PAY  55 

    0x39b62812,// 59 PAY  56 

    0x12016b33,// 60 PAY  57 

    0x8a2f2660,// 61 PAY  58 

    0x57e55953,// 62 PAY  59 

    0x9a538b2e,// 63 PAY  60 

    0xf88edc93,// 64 PAY  61 

    0xc1b87c73,// 65 PAY  62 

    0x96d74c16,// 66 PAY  63 

    0x9d9d92a6,// 67 PAY  64 

    0x25028f3e,// 68 PAY  65 

    0xe8b0bfd2,// 69 PAY  66 

    0x9578c21e,// 70 PAY  67 

    0x11117221,// 71 PAY  68 

    0x4fc90b0e,// 72 PAY  69 

    0xf9023f0c,// 73 PAY  70 

    0x5ecdc298,// 74 PAY  71 

    0xd84a1051,// 75 PAY  72 

    0x1ab844ad,// 76 PAY  73 

    0x19f7f9e9,// 77 PAY  74 

    0xfa5f1046,// 78 PAY  75 

    0x0d9c9373,// 79 PAY  76 

    0x6bf49bac,// 80 PAY  77 

    0x8bb87e88,// 81 PAY  78 

    0xd8aeb657,// 82 PAY  79 

    0x5ba54577,// 83 PAY  80 

    0x3768dd9c,// 84 PAY  81 

    0x3a60b1ee,// 85 PAY  82 

    0x7c4e398b,// 86 PAY  83 

    0xf4e83df8,// 87 PAY  84 

    0x586c3b16,// 88 PAY  85 

    0xbbf7a330,// 89 PAY  86 

    0xa9c648ea,// 90 PAY  87 

    0x1352555f,// 91 PAY  88 

    0x4df3e966,// 92 PAY  89 

    0xadda0650,// 93 PAY  90 

    0xe310a67d,// 94 PAY  91 

    0x8ae12db6,// 95 PAY  92 

    0xa8224427,// 96 PAY  93 

    0x7510671b,// 97 PAY  94 

    0xa824afbe,// 98 PAY  95 

    0xe21a1323,// 99 PAY  96 

    0x949345d6,// 100 PAY  97 

    0x8cd0f086,// 101 PAY  98 

    0x4c46dfc0,// 102 PAY  99 

    0x3d42cc05,// 103 PAY 100 

    0xae8358a8,// 104 PAY 101 

    0xe59435cd,// 105 PAY 102 

    0xff9af874,// 106 PAY 103 

    0x6e30c600,// 107 PAY 104 

    0x14a8128b,// 108 PAY 105 

    0x492f7a8a,// 109 PAY 106 

    0x810552e0,// 110 PAY 107 

    0x7eaf322a,// 111 PAY 108 

    0xa5ac4908,// 112 PAY 109 

    0xe924384b,// 113 PAY 110 

    0x6de355e3,// 114 PAY 111 

    0x50183764,// 115 PAY 112 

    0x0bef9b97,// 116 PAY 113 

    0x0b168ec5,// 117 PAY 114 

    0xe521c177,// 118 PAY 115 

    0xabf2731c,// 119 PAY 116 

    0xdd884c36,// 120 PAY 117 

    0x5362d886,// 121 PAY 118 

    0xb4e72c5f,// 122 PAY 119 

    0xf93769f6,// 123 PAY 120 

    0xdda42cfa,// 124 PAY 121 

    0x17dff7a9,// 125 PAY 122 

    0x950f6586,// 126 PAY 123 

    0x5ced8594,// 127 PAY 124 

    0x57fc1058,// 128 PAY 125 

    0xafa76bc1,// 129 PAY 126 

    0xa0f8a371,// 130 PAY 127 

    0x64a2f677,// 131 PAY 128 

    0xa8724448,// 132 PAY 129 

    0xeaba686d,// 133 PAY 130 

    0x6c39d4ed,// 134 PAY 131 

    0xf1a1f1ae,// 135 PAY 132 

    0x2889f3b1,// 136 PAY 133 

    0x3b0a761a,// 137 PAY 134 

    0xe3a865d9,// 138 PAY 135 

    0xd13ca932,// 139 PAY 136 

    0x188a5263,// 140 PAY 137 

    0xcda7be6c,// 141 PAY 138 

    0x9b42292b,// 142 PAY 139 

    0x01e1cd87,// 143 PAY 140 

    0xa607efa7,// 144 PAY 141 

    0x0595bf8f,// 145 PAY 142 

    0x39aa6914,// 146 PAY 143 

    0x39afbeb6,// 147 PAY 144 

    0x29e5cd82,// 148 PAY 145 

    0xd9472a01,// 149 PAY 146 

    0x1b91b218,// 150 PAY 147 

    0x8d9d5ccb,// 151 PAY 148 

    0x86487f5c,// 152 PAY 149 

    0x396e695c,// 153 PAY 150 

    0xb950278d,// 154 PAY 151 

    0x7f6bcbb7,// 155 PAY 152 

    0xc99b764f,// 156 PAY 153 

    0x9a25ca80,// 157 PAY 154 

    0xa6ba33ab,// 158 PAY 155 

    0xf48ebd58,// 159 PAY 156 

    0xa91b817d,// 160 PAY 157 

    0x51f63f0a,// 161 PAY 158 

    0x8108b315,// 162 PAY 159 

    0xa025e8a8,// 163 PAY 160 

    0x1724f719,// 164 PAY 161 

    0x1613d0ce,// 165 PAY 162 

    0x3864768b,// 166 PAY 163 

    0xe11445fb,// 167 PAY 164 

    0x1f82f586,// 168 PAY 165 

    0xa7331dc3,// 169 PAY 166 

    0x5d329e9a,// 170 PAY 167 

    0xbb7bd24b,// 171 PAY 168 

    0xecf8e912,// 172 PAY 169 

    0x43314a82,// 173 PAY 170 

    0x4808bd19,// 174 PAY 171 

    0x51cb8977,// 175 PAY 172 

    0x3a4a5569,// 176 PAY 173 

    0x3aa1ef72,// 177 PAY 174 

    0xb6037de0,// 178 PAY 175 

    0xbd2618c1,// 179 PAY 176 

    0x8a9201bb,// 180 PAY 177 

    0x8df5ed56,// 181 PAY 178 

    0xc531ce6e,// 182 PAY 179 

    0x98718e98,// 183 PAY 180 

    0x42fa1d35,// 184 PAY 181 

    0xdc1983a9,// 185 PAY 182 

    0x337a2504,// 186 PAY 183 

    0xdfa47ad9,// 187 PAY 184 

    0xe9e71318,// 188 PAY 185 

    0x7b3a972d,// 189 PAY 186 

    0x2d801acf,// 190 PAY 187 

    0x1013d8fd,// 191 PAY 188 

    0x074a91ef,// 192 PAY 189 

    0x07b4f472,// 193 PAY 190 

    0x72ab65ff,// 194 PAY 191 

    0x38200cb7,// 195 PAY 192 

    0xd7e9693d,// 196 PAY 193 

    0x5003df70,// 197 PAY 194 

    0x06d247ef,// 198 PAY 195 

    0x2507e86f,// 199 PAY 196 

    0x1149a12a,// 200 PAY 197 

    0x54c66235,// 201 PAY 198 

    0x9a0f6f78,// 202 PAY 199 

    0x3c50ef78,// 203 PAY 200 

    0x5d96c3ee,// 204 PAY 201 

    0x6c876274,// 205 PAY 202 

    0x1c06a517,// 206 PAY 203 

    0x351f86ef,// 207 PAY 204 

    0x2aed6daa,// 208 PAY 205 

    0x3e31be6d,// 209 PAY 206 

    0xae1f514e,// 210 PAY 207 

    0x789050ec,// 211 PAY 208 

    0xb4481dea,// 212 PAY 209 

    0xba52ab15,// 213 PAY 210 

    0x539f0d72,// 214 PAY 211 

    0x431b67ed,// 215 PAY 212 

    0x2db6a2b0,// 216 PAY 213 

    0xea8db6f0,// 217 PAY 214 

    0xb9e440da,// 218 PAY 215 

    0xb52c1429,// 219 PAY 216 

    0x69498bb5,// 220 PAY 217 

    0xe1008abc,// 221 PAY 218 

    0x8100c6cc,// 222 PAY 219 

    0xbd956a55,// 223 PAY 220 

    0x1fc78feb,// 224 PAY 221 

    0x4d677ad0,// 225 PAY 222 

    0x33fed351,// 226 PAY 223 

    0xccf77d8d,// 227 PAY 224 

    0xc01cffcd,// 228 PAY 225 

    0xcb2fc375,// 229 PAY 226 

    0x447fa96d,// 230 PAY 227 

    0x600d758c,// 231 PAY 228 

    0x6da9a479,// 232 PAY 229 

    0xfff2799f,// 233 PAY 230 

    0x9155c2e2,// 234 PAY 231 

    0x27d9376d,// 235 PAY 232 

    0x707f16bb,// 236 PAY 233 

    0x0f0444f2,// 237 PAY 234 

    0x35fb817b,// 238 PAY 235 

    0xfaf49b06,// 239 PAY 236 

    0x0dcbfcb4,// 240 PAY 237 

    0x4e715774,// 241 PAY 238 

    0x6858363a,// 242 PAY 239 

    0xefcd3f84,// 243 PAY 240 

    0xa50de1a6,// 244 PAY 241 

    0xe9690406,// 245 PAY 242 

    0x01a92a6c,// 246 PAY 243 

    0xa14d9e11,// 247 PAY 244 

    0xad81195b,// 248 PAY 245 

    0x2ac5f264,// 249 PAY 246 

    0x0c01dc2a,// 250 PAY 247 

    0xffeb27a6,// 251 PAY 248 

    0x79882d74,// 252 PAY 249 

    0xcb65723c,// 253 PAY 250 

    0xfe40a107,// 254 PAY 251 

    0x6e8e0b1b,// 255 PAY 252 

    0x507566a0,// 256 PAY 253 

    0x16e3f34e,// 257 PAY 254 

    0xaf11dba3,// 258 PAY 255 

    0x4c75c7d1,// 259 PAY 256 

    0x4c9d6fe9,// 260 PAY 257 

    0x8577cae8,// 261 PAY 258 

    0x3954b374,// 262 PAY 259 

    0x1126154c,// 263 PAY 260 

    0x776feed7,// 264 PAY 261 

    0xd88a1021,// 265 PAY 262 

/// STA is 1 words. 

/// STA num_pkts       : 175 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc9 

    0x005dc9af // 266 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt95_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 440 words. 

/// BDA size     is 1756 (0x6dc) 

/// BDA id       is 0x6c9d 

    0x06dc6c9d,// 3 BDA   1 

/// PAY Generic Data size   : 1756 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x554c975d,// 4 PAY   1 

    0xcaea734f,// 5 PAY   2 

    0xfadb728b,// 6 PAY   3 

    0xc4a9fec2,// 7 PAY   4 

    0x80642ebe,// 8 PAY   5 

    0xa9c9d599,// 9 PAY   6 

    0x224ffcff,// 10 PAY   7 

    0x2dd03bf5,// 11 PAY   8 

    0xd5007055,// 12 PAY   9 

    0x6fd3cbed,// 13 PAY  10 

    0xc99e9652,// 14 PAY  11 

    0xa18aef0c,// 15 PAY  12 

    0xdec0b4b1,// 16 PAY  13 

    0x6ce0c815,// 17 PAY  14 

    0x50237f04,// 18 PAY  15 

    0x8997b0d3,// 19 PAY  16 

    0xdb4e9ba2,// 20 PAY  17 

    0xc94a3d52,// 21 PAY  18 

    0x9a199094,// 22 PAY  19 

    0x721cce01,// 23 PAY  20 

    0x47f5bb53,// 24 PAY  21 

    0x61e3d797,// 25 PAY  22 

    0x9b2386ad,// 26 PAY  23 

    0x48e9c43a,// 27 PAY  24 

    0xf197a873,// 28 PAY  25 

    0x4436ffbb,// 29 PAY  26 

    0xb1e8c412,// 30 PAY  27 

    0xd1759e58,// 31 PAY  28 

    0x2fcc6e12,// 32 PAY  29 

    0xb046cff4,// 33 PAY  30 

    0x79a4ab65,// 34 PAY  31 

    0x27046e67,// 35 PAY  32 

    0xd10b01f5,// 36 PAY  33 

    0xbe2677fa,// 37 PAY  34 

    0xee52d223,// 38 PAY  35 

    0x7ccc9c2f,// 39 PAY  36 

    0xf1a86e67,// 40 PAY  37 

    0x8753a273,// 41 PAY  38 

    0x49a8c535,// 42 PAY  39 

    0x0858f7c9,// 43 PAY  40 

    0x66ffc45f,// 44 PAY  41 

    0x7c57e087,// 45 PAY  42 

    0xbcd5b531,// 46 PAY  43 

    0x9fde7612,// 47 PAY  44 

    0xf8368ea4,// 48 PAY  45 

    0x557f82fd,// 49 PAY  46 

    0x65290f97,// 50 PAY  47 

    0x944d8410,// 51 PAY  48 

    0x46902018,// 52 PAY  49 

    0x54cd6569,// 53 PAY  50 

    0xc1abec6e,// 54 PAY  51 

    0xdfe81158,// 55 PAY  52 

    0xff150212,// 56 PAY  53 

    0x6c026094,// 57 PAY  54 

    0xa7846e5d,// 58 PAY  55 

    0xc26ccc1c,// 59 PAY  56 

    0xc907c663,// 60 PAY  57 

    0x7ac39163,// 61 PAY  58 

    0x96be0bf5,// 62 PAY  59 

    0x2da6d5d3,// 63 PAY  60 

    0x2418d381,// 64 PAY  61 

    0x9e6f6d5d,// 65 PAY  62 

    0x49a8b4e8,// 66 PAY  63 

    0xdd6dbf9a,// 67 PAY  64 

    0xc20797f6,// 68 PAY  65 

    0x93f657e9,// 69 PAY  66 

    0xe509b570,// 70 PAY  67 

    0x7c20bf75,// 71 PAY  68 

    0xdc0753c0,// 72 PAY  69 

    0x406c2d10,// 73 PAY  70 

    0xed1d083a,// 74 PAY  71 

    0x6208f53a,// 75 PAY  72 

    0x0e008214,// 76 PAY  73 

    0x0a086000,// 77 PAY  74 

    0x930a439f,// 78 PAY  75 

    0x9e22d23d,// 79 PAY  76 

    0x3e4d2462,// 80 PAY  77 

    0x5e3066b7,// 81 PAY  78 

    0x6ceffb6e,// 82 PAY  79 

    0x7cfb7cb0,// 83 PAY  80 

    0xbb166121,// 84 PAY  81 

    0x5618361a,// 85 PAY  82 

    0x171f4895,// 86 PAY  83 

    0xb92dcbdd,// 87 PAY  84 

    0x782c3d4e,// 88 PAY  85 

    0x6aeafaa1,// 89 PAY  86 

    0xce2263b9,// 90 PAY  87 

    0x6bdfb8e2,// 91 PAY  88 

    0x9bea3625,// 92 PAY  89 

    0xd68284c0,// 93 PAY  90 

    0x66b85433,// 94 PAY  91 

    0x80646cc7,// 95 PAY  92 

    0x26b339df,// 96 PAY  93 

    0x80540115,// 97 PAY  94 

    0x427d38a9,// 98 PAY  95 

    0x9cf55e10,// 99 PAY  96 

    0x85cf33e7,// 100 PAY  97 

    0x7d3bca44,// 101 PAY  98 

    0xcec65e77,// 102 PAY  99 

    0x667fe911,// 103 PAY 100 

    0xe383456d,// 104 PAY 101 

    0x69cce210,// 105 PAY 102 

    0xca872d6b,// 106 PAY 103 

    0xfa24e6f5,// 107 PAY 104 

    0x71bf62a2,// 108 PAY 105 

    0xd747e777,// 109 PAY 106 

    0x3f0aa805,// 110 PAY 107 

    0x5f693eb2,// 111 PAY 108 

    0xe8823dbe,// 112 PAY 109 

    0xa4675216,// 113 PAY 110 

    0x92608970,// 114 PAY 111 

    0x5780bacb,// 115 PAY 112 

    0x181f0a4a,// 116 PAY 113 

    0x0870218b,// 117 PAY 114 

    0x909683a7,// 118 PAY 115 

    0xe017ccee,// 119 PAY 116 

    0xeb0bf602,// 120 PAY 117 

    0x2186ec14,// 121 PAY 118 

    0xff0cf1cc,// 122 PAY 119 

    0x0eca7a22,// 123 PAY 120 

    0xdfdc341e,// 124 PAY 121 

    0x374de201,// 125 PAY 122 

    0x7ffef299,// 126 PAY 123 

    0x91de948e,// 127 PAY 124 

    0xb50ccf17,// 128 PAY 125 

    0xfb1e51bf,// 129 PAY 126 

    0x1e00a043,// 130 PAY 127 

    0x00d3837b,// 131 PAY 128 

    0x656433aa,// 132 PAY 129 

    0xbe3c9875,// 133 PAY 130 

    0x53c2c73c,// 134 PAY 131 

    0xccec635f,// 135 PAY 132 

    0x4dfa52e3,// 136 PAY 133 

    0xd6be0b80,// 137 PAY 134 

    0x638595c6,// 138 PAY 135 

    0x4f14385f,// 139 PAY 136 

    0xad21ec5a,// 140 PAY 137 

    0xbd2bb4a8,// 141 PAY 138 

    0x617415a5,// 142 PAY 139 

    0x29af1aab,// 143 PAY 140 

    0xecb645f1,// 144 PAY 141 

    0x0c59a194,// 145 PAY 142 

    0x150b971b,// 146 PAY 143 

    0x78e5b5be,// 147 PAY 144 

    0xdf3feb95,// 148 PAY 145 

    0x355238d3,// 149 PAY 146 

    0x2b7c2fc0,// 150 PAY 147 

    0x7ecca00c,// 151 PAY 148 

    0xe953634a,// 152 PAY 149 

    0x1789273f,// 153 PAY 150 

    0x26ccb036,// 154 PAY 151 

    0xd29e12dc,// 155 PAY 152 

    0xdf9e354d,// 156 PAY 153 

    0x6f9ebdf5,// 157 PAY 154 

    0x7847367a,// 158 PAY 155 

    0xe2bb80db,// 159 PAY 156 

    0xd3b60878,// 160 PAY 157 

    0x98b883c6,// 161 PAY 158 

    0x479e6767,// 162 PAY 159 

    0x41f078ad,// 163 PAY 160 

    0x520d034c,// 164 PAY 161 

    0x408b3755,// 165 PAY 162 

    0x6a46505d,// 166 PAY 163 

    0x032d4546,// 167 PAY 164 

    0x4c8f1b9d,// 168 PAY 165 

    0xd81d1b31,// 169 PAY 166 

    0x68904714,// 170 PAY 167 

    0x0ebe1d6e,// 171 PAY 168 

    0x47f6549a,// 172 PAY 169 

    0x4ba049bf,// 173 PAY 170 

    0x7ed41a9b,// 174 PAY 171 

    0x85e81aaa,// 175 PAY 172 

    0x7012f2c1,// 176 PAY 173 

    0x8a95cbdc,// 177 PAY 174 

    0x7b607ad7,// 178 PAY 175 

    0x4832297f,// 179 PAY 176 

    0x2e064479,// 180 PAY 177 

    0xb572ee84,// 181 PAY 178 

    0x5df95f2a,// 182 PAY 179 

    0x676b6a86,// 183 PAY 180 

    0xe37e09b1,// 184 PAY 181 

    0x6e36da1b,// 185 PAY 182 

    0x014ac1a6,// 186 PAY 183 

    0xaa5952c5,// 187 PAY 184 

    0x56862783,// 188 PAY 185 

    0x54a45179,// 189 PAY 186 

    0xf06d7352,// 190 PAY 187 

    0x23fad05c,// 191 PAY 188 

    0x239cfe93,// 192 PAY 189 

    0x5799a019,// 193 PAY 190 

    0x6b618db7,// 194 PAY 191 

    0xd6c03cd2,// 195 PAY 192 

    0xaa438ada,// 196 PAY 193 

    0x964d3a7e,// 197 PAY 194 

    0x4f425cbc,// 198 PAY 195 

    0xfd93b1a4,// 199 PAY 196 

    0x6f7fb6a8,// 200 PAY 197 

    0x31d4781d,// 201 PAY 198 

    0x21926ea2,// 202 PAY 199 

    0xb4e88186,// 203 PAY 200 

    0x93023f66,// 204 PAY 201 

    0xe123fe10,// 205 PAY 202 

    0x7ce0d62b,// 206 PAY 203 

    0x3f9695b8,// 207 PAY 204 

    0x52314efe,// 208 PAY 205 

    0x0ada661b,// 209 PAY 206 

    0x2b2fca90,// 210 PAY 207 

    0x1b9e2a5d,// 211 PAY 208 

    0x2809f63b,// 212 PAY 209 

    0xcb15bc87,// 213 PAY 210 

    0x416e18f7,// 214 PAY 211 

    0x77dacf35,// 215 PAY 212 

    0x65fe377c,// 216 PAY 213 

    0xe6923800,// 217 PAY 214 

    0x434c0891,// 218 PAY 215 

    0xdd0799bf,// 219 PAY 216 

    0x9d34cec6,// 220 PAY 217 

    0xc73d6919,// 221 PAY 218 

    0x20a64d0c,// 222 PAY 219 

    0xd53774fd,// 223 PAY 220 

    0x7f971fff,// 224 PAY 221 

    0x38fbcf36,// 225 PAY 222 

    0xb9b57acf,// 226 PAY 223 

    0x0d091a0d,// 227 PAY 224 

    0x6f825054,// 228 PAY 225 

    0xa3da1c31,// 229 PAY 226 

    0xdab91978,// 230 PAY 227 

    0xa3441b03,// 231 PAY 228 

    0x1f06b8d6,// 232 PAY 229 

    0x1833302f,// 233 PAY 230 

    0x116ea11f,// 234 PAY 231 

    0x219c5181,// 235 PAY 232 

    0xa0841fc7,// 236 PAY 233 

    0x212e097f,// 237 PAY 234 

    0x2f8b9b04,// 238 PAY 235 

    0x73b385d4,// 239 PAY 236 

    0xaf32846d,// 240 PAY 237 

    0xaa3f4d24,// 241 PAY 238 

    0x5721d19a,// 242 PAY 239 

    0xd3229383,// 243 PAY 240 

    0x59262668,// 244 PAY 241 

    0x16bb496a,// 245 PAY 242 

    0xc21d2ede,// 246 PAY 243 

    0xb6388860,// 247 PAY 244 

    0xad56ddd9,// 248 PAY 245 

    0xd78eed5b,// 249 PAY 246 

    0x1b57ea87,// 250 PAY 247 

    0x2e413442,// 251 PAY 248 

    0x40945765,// 252 PAY 249 

    0xb4935a53,// 253 PAY 250 

    0x5b73ba68,// 254 PAY 251 

    0x0cd93bb2,// 255 PAY 252 

    0x561b423e,// 256 PAY 253 

    0x794e1055,// 257 PAY 254 

    0x908f1405,// 258 PAY 255 

    0x00ed265a,// 259 PAY 256 

    0x8c365429,// 260 PAY 257 

    0x820a31e4,// 261 PAY 258 

    0x1884fda5,// 262 PAY 259 

    0xff700cc8,// 263 PAY 260 

    0xf43b60ae,// 264 PAY 261 

    0xfb2bab55,// 265 PAY 262 

    0xa362ad5d,// 266 PAY 263 

    0xab062fe8,// 267 PAY 264 

    0xa70bb02b,// 268 PAY 265 

    0xdf6abb32,// 269 PAY 266 

    0xd9f5c507,// 270 PAY 267 

    0x1ee3168c,// 271 PAY 268 

    0x88e1cbaa,// 272 PAY 269 

    0x4442e74a,// 273 PAY 270 

    0x399de1e4,// 274 PAY 271 

    0x581f10b9,// 275 PAY 272 

    0xa39371e7,// 276 PAY 273 

    0xa947324c,// 277 PAY 274 

    0xe00e07dc,// 278 PAY 275 

    0x26ff70c8,// 279 PAY 276 

    0x75c1e81c,// 280 PAY 277 

    0x8dac3980,// 281 PAY 278 

    0x4a04f5db,// 282 PAY 279 

    0x8837ae07,// 283 PAY 280 

    0x3755aac6,// 284 PAY 281 

    0x2782575e,// 285 PAY 282 

    0x671b82aa,// 286 PAY 283 

    0xc8285b00,// 287 PAY 284 

    0xd91a67c1,// 288 PAY 285 

    0xb03c5996,// 289 PAY 286 

    0x08be8e0d,// 290 PAY 287 

    0x8dc8ab17,// 291 PAY 288 

    0x010f2bc1,// 292 PAY 289 

    0x7b7b1f84,// 293 PAY 290 

    0xf1bc029c,// 294 PAY 291 

    0x0a669a82,// 295 PAY 292 

    0x57feb74e,// 296 PAY 293 

    0xcbe8065b,// 297 PAY 294 

    0xbfad95e9,// 298 PAY 295 

    0xdb3a08ed,// 299 PAY 296 

    0xe56f31a2,// 300 PAY 297 

    0xc97b2d24,// 301 PAY 298 

    0x39b029eb,// 302 PAY 299 

    0xb659020d,// 303 PAY 300 

    0xd035368d,// 304 PAY 301 

    0xf9ddbd97,// 305 PAY 302 

    0xd454be37,// 306 PAY 303 

    0x1a4ae135,// 307 PAY 304 

    0xd534c64f,// 308 PAY 305 

    0x9e418501,// 309 PAY 306 

    0xb2e0a8bc,// 310 PAY 307 

    0x7cc50480,// 311 PAY 308 

    0xc0eeea75,// 312 PAY 309 

    0xb4fae887,// 313 PAY 310 

    0x782d331e,// 314 PAY 311 

    0xd80723fd,// 315 PAY 312 

    0x43ac97d2,// 316 PAY 313 

    0x29d239ee,// 317 PAY 314 

    0x1e10b904,// 318 PAY 315 

    0xdcafd7da,// 319 PAY 316 

    0x470896cc,// 320 PAY 317 

    0x42c8286c,// 321 PAY 318 

    0xbdd384c7,// 322 PAY 319 

    0xd0de4746,// 323 PAY 320 

    0x7cd2c68e,// 324 PAY 321 

    0x2dde9468,// 325 PAY 322 

    0xdb3a046d,// 326 PAY 323 

    0x0a6a8203,// 327 PAY 324 

    0xb9dfb6da,// 328 PAY 325 

    0x29985d8c,// 329 PAY 326 

    0xc89e521d,// 330 PAY 327 

    0x00ec8765,// 331 PAY 328 

    0xe08fdc0b,// 332 PAY 329 

    0xe687e24b,// 333 PAY 330 

    0xb1794037,// 334 PAY 331 

    0xd16b8b35,// 335 PAY 332 

    0xe5a9c454,// 336 PAY 333 

    0x69f95973,// 337 PAY 334 

    0xfe69aca2,// 338 PAY 335 

    0x40c693f5,// 339 PAY 336 

    0x0d0af164,// 340 PAY 337 

    0xdc203bf1,// 341 PAY 338 

    0x5caa9869,// 342 PAY 339 

    0x16e9276e,// 343 PAY 340 

    0x6cf6f81a,// 344 PAY 341 

    0xaa5cf9c4,// 345 PAY 342 

    0xfa8b3007,// 346 PAY 343 

    0xf6870b12,// 347 PAY 344 

    0x1a6605a7,// 348 PAY 345 

    0xe7b44803,// 349 PAY 346 

    0xfca7a541,// 350 PAY 347 

    0xf1a26b55,// 351 PAY 348 

    0x04a78b87,// 352 PAY 349 

    0xc9f7c332,// 353 PAY 350 

    0x3660c8cc,// 354 PAY 351 

    0xb64508ee,// 355 PAY 352 

    0x9f683b50,// 356 PAY 353 

    0xfd28074e,// 357 PAY 354 

    0x4f9544b8,// 358 PAY 355 

    0x6ea96ed7,// 359 PAY 356 

    0xd4d2bc89,// 360 PAY 357 

    0x1474e3f1,// 361 PAY 358 

    0x73a29a5c,// 362 PAY 359 

    0x829b00d9,// 363 PAY 360 

    0x04f52e6a,// 364 PAY 361 

    0x909b3eac,// 365 PAY 362 

    0x7867c5e9,// 366 PAY 363 

    0x694e1c43,// 367 PAY 364 

    0x28e2b8e1,// 368 PAY 365 

    0x596aea2a,// 369 PAY 366 

    0x20ba6a59,// 370 PAY 367 

    0x70aeb864,// 371 PAY 368 

    0x7d7b1a5f,// 372 PAY 369 

    0x17c89749,// 373 PAY 370 

    0xeb7b38d3,// 374 PAY 371 

    0x35f44630,// 375 PAY 372 

    0x91dcc0b2,// 376 PAY 373 

    0x8250216c,// 377 PAY 374 

    0x3a01ba53,// 378 PAY 375 

    0x03f671c7,// 379 PAY 376 

    0xbada8c16,// 380 PAY 377 

    0xc814ef7c,// 381 PAY 378 

    0xbc99abff,// 382 PAY 379 

    0xee944038,// 383 PAY 380 

    0x4dc652c2,// 384 PAY 381 

    0xd4238c20,// 385 PAY 382 

    0x748108f6,// 386 PAY 383 

    0xfc5bb603,// 387 PAY 384 

    0xf3182049,// 388 PAY 385 

    0x3dcc060d,// 389 PAY 386 

    0xd8d09050,// 390 PAY 387 

    0xe605c1b7,// 391 PAY 388 

    0xc80fc248,// 392 PAY 389 

    0x86d532dc,// 393 PAY 390 

    0x6507f624,// 394 PAY 391 

    0xa1b02190,// 395 PAY 392 

    0x6992fe74,// 396 PAY 393 

    0xb85be378,// 397 PAY 394 

    0x8d540610,// 398 PAY 395 

    0xcfc52cd9,// 399 PAY 396 

    0xab9019dc,// 400 PAY 397 

    0x10b3a10b,// 401 PAY 398 

    0x1f3a1abf,// 402 PAY 399 

    0xe842c831,// 403 PAY 400 

    0xa18fab17,// 404 PAY 401 

    0xfad2c5a7,// 405 PAY 402 

    0xf0f46c86,// 406 PAY 403 

    0x4ccecff7,// 407 PAY 404 

    0xeeb618d4,// 408 PAY 405 

    0xfbb1dd22,// 409 PAY 406 

    0x7e8eacb3,// 410 PAY 407 

    0x6e5d1967,// 411 PAY 408 

    0xde0fa3d5,// 412 PAY 409 

    0x6b84ea17,// 413 PAY 410 

    0x3b6498bd,// 414 PAY 411 

    0x45c03ce5,// 415 PAY 412 

    0xc69b0f99,// 416 PAY 413 

    0x87736e1a,// 417 PAY 414 

    0xa2907240,// 418 PAY 415 

    0xb0471b34,// 419 PAY 416 

    0xd511ec94,// 420 PAY 417 

    0x8cca8406,// 421 PAY 418 

    0x70c3770d,// 422 PAY 419 

    0x983a2c80,// 423 PAY 420 

    0x885e52dc,// 424 PAY 421 

    0xf6825fbd,// 425 PAY 422 

    0xc52c7a6a,// 426 PAY 423 

    0x101797fe,// 427 PAY 424 

    0xd9aa90d9,// 428 PAY 425 

    0xcbccc8c1,// 429 PAY 426 

    0x2cce151d,// 430 PAY 427 

    0x7351f959,// 431 PAY 428 

    0xf9ab2e9e,// 432 PAY 429 

    0x117450c0,// 433 PAY 430 

    0xe69d0ca3,// 434 PAY 431 

    0x94cde229,// 435 PAY 432 

    0x04261cca,// 436 PAY 433 

    0xf44b14e2,// 437 PAY 434 

    0xa8585595,// 438 PAY 435 

    0xea232860,// 439 PAY 436 

    0x99feeb79,// 440 PAY 437 

    0xe9f89ab7,// 441 PAY 438 

    0x96aeee68,// 442 PAY 439 

/// STA is 1 words. 

/// STA num_pkts       : 93 

/// STA pkt_idx        : 87 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe2 

    0x015ce25d // 443 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt96_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 270 words. 

/// BDA size     is 1075 (0x433) 

/// BDA id       is 0xc09b 

    0x0433c09b,// 3 BDA   1 

/// PAY Generic Data size   : 1075 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x70a7df38,// 4 PAY   1 

    0xbb6c3c04,// 5 PAY   2 

    0x3b5d8651,// 6 PAY   3 

    0xcd48bbe3,// 7 PAY   4 

    0x407e01bd,// 8 PAY   5 

    0x5830824d,// 9 PAY   6 

    0x576b2f07,// 10 PAY   7 

    0xf568c900,// 11 PAY   8 

    0x9527de85,// 12 PAY   9 

    0x3960673d,// 13 PAY  10 

    0xdea3a3d3,// 14 PAY  11 

    0xd01b8368,// 15 PAY  12 

    0xcfa79acc,// 16 PAY  13 

    0xd6c118b3,// 17 PAY  14 

    0x0ba9a149,// 18 PAY  15 

    0x27533168,// 19 PAY  16 

    0xb65dd3e0,// 20 PAY  17 

    0xd20a3e58,// 21 PAY  18 

    0x8d04b09c,// 22 PAY  19 

    0x6e82f034,// 23 PAY  20 

    0x6995b97a,// 24 PAY  21 

    0xe099069a,// 25 PAY  22 

    0x2dd60270,// 26 PAY  23 

    0x663aa4a9,// 27 PAY  24 

    0x3272ff39,// 28 PAY  25 

    0xd3f7b12f,// 29 PAY  26 

    0x9fe4510e,// 30 PAY  27 

    0x6a96a7b9,// 31 PAY  28 

    0x16e2f02e,// 32 PAY  29 

    0xe79f0270,// 33 PAY  30 

    0x3dc17fe8,// 34 PAY  31 

    0x1f54d362,// 35 PAY  32 

    0xaf2dd25e,// 36 PAY  33 

    0xa03aee25,// 37 PAY  34 

    0x74bd4fdd,// 38 PAY  35 

    0x2db48903,// 39 PAY  36 

    0x2737af1e,// 40 PAY  37 

    0x06140106,// 41 PAY  38 

    0xcfbed8cc,// 42 PAY  39 

    0x4ee87392,// 43 PAY  40 

    0x179acdd5,// 44 PAY  41 

    0x7d17a18f,// 45 PAY  42 

    0x6ebace44,// 46 PAY  43 

    0x909c4e6f,// 47 PAY  44 

    0x07c166ae,// 48 PAY  45 

    0x1c8dbef4,// 49 PAY  46 

    0xf883799e,// 50 PAY  47 

    0x67282b85,// 51 PAY  48 

    0x11342c7f,// 52 PAY  49 

    0x07b90eb1,// 53 PAY  50 

    0x516aa8be,// 54 PAY  51 

    0xef19c93c,// 55 PAY  52 

    0xffcea049,// 56 PAY  53 

    0x4b0043d5,// 57 PAY  54 

    0x9c342ca3,// 58 PAY  55 

    0x37513b1d,// 59 PAY  56 

    0xc5b3dbfa,// 60 PAY  57 

    0x4a2c2b87,// 61 PAY  58 

    0xeb66dc67,// 62 PAY  59 

    0x04ee78a9,// 63 PAY  60 

    0x71dff068,// 64 PAY  61 

    0x59daac5a,// 65 PAY  62 

    0xeb512a0c,// 66 PAY  63 

    0x3be7bff6,// 67 PAY  64 

    0xe3b080f5,// 68 PAY  65 

    0x1db88290,// 69 PAY  66 

    0xfcd0dbe4,// 70 PAY  67 

    0x91455840,// 71 PAY  68 

    0x89c91eb4,// 72 PAY  69 

    0x11d2681b,// 73 PAY  70 

    0x02c06ad7,// 74 PAY  71 

    0x02425ac2,// 75 PAY  72 

    0xa0225cd8,// 76 PAY  73 

    0x1a7f1b08,// 77 PAY  74 

    0xc91ee664,// 78 PAY  75 

    0xe0fce425,// 79 PAY  76 

    0xc8fec819,// 80 PAY  77 

    0x45bfb075,// 81 PAY  78 

    0xffdddb15,// 82 PAY  79 

    0xc573ce70,// 83 PAY  80 

    0x0cdcabe8,// 84 PAY  81 

    0x69ab1783,// 85 PAY  82 

    0xbc6a2e43,// 86 PAY  83 

    0xcb054385,// 87 PAY  84 

    0x94ad046a,// 88 PAY  85 

    0x0998e6a3,// 89 PAY  86 

    0xb881ac2c,// 90 PAY  87 

    0x9dd91007,// 91 PAY  88 

    0x54a8ad8c,// 92 PAY  89 

    0x0b060af6,// 93 PAY  90 

    0x1a11dbb6,// 94 PAY  91 

    0x5c9332f8,// 95 PAY  92 

    0x720e57dd,// 96 PAY  93 

    0x65dfee50,// 97 PAY  94 

    0xefad3250,// 98 PAY  95 

    0x1f73be7c,// 99 PAY  96 

    0xdd78fcf0,// 100 PAY  97 

    0x86cc4c2d,// 101 PAY  98 

    0xf91a1ec9,// 102 PAY  99 

    0xeb1225d0,// 103 PAY 100 

    0xbd984b0f,// 104 PAY 101 

    0x2d7809f9,// 105 PAY 102 

    0x3131a4cd,// 106 PAY 103 

    0x969a7c04,// 107 PAY 104 

    0x4d1d32a3,// 108 PAY 105 

    0x3b7057a2,// 109 PAY 106 

    0x9ab58091,// 110 PAY 107 

    0x6509c523,// 111 PAY 108 

    0xe2f5093b,// 112 PAY 109 

    0x25fc12ee,// 113 PAY 110 

    0x1a802c58,// 114 PAY 111 

    0x5c766c8a,// 115 PAY 112 

    0x11625462,// 116 PAY 113 

    0xde291e97,// 117 PAY 114 

    0x114f7a83,// 118 PAY 115 

    0x387ff3bd,// 119 PAY 116 

    0xf3758567,// 120 PAY 117 

    0xbbf31fb8,// 121 PAY 118 

    0x54cd1abc,// 122 PAY 119 

    0xe470d1c0,// 123 PAY 120 

    0x2e8ed2cf,// 124 PAY 121 

    0x6ed847e6,// 125 PAY 122 

    0x7f85de6c,// 126 PAY 123 

    0xd9d861e2,// 127 PAY 124 

    0x952b3caa,// 128 PAY 125 

    0x5a355a42,// 129 PAY 126 

    0x05dabd92,// 130 PAY 127 

    0xe54eb246,// 131 PAY 128 

    0xbd8449cd,// 132 PAY 129 

    0xc83a04a0,// 133 PAY 130 

    0x049ac8ad,// 134 PAY 131 

    0x9c9e8162,// 135 PAY 132 

    0xa37b41b7,// 136 PAY 133 

    0xaf1f174b,// 137 PAY 134 

    0xb686b7cf,// 138 PAY 135 

    0xd66c251d,// 139 PAY 136 

    0x7f4da938,// 140 PAY 137 

    0x79d536bd,// 141 PAY 138 

    0xd2eb34bb,// 142 PAY 139 

    0xc71ab544,// 143 PAY 140 

    0xab1785bf,// 144 PAY 141 

    0xa7145577,// 145 PAY 142 

    0x9134a196,// 146 PAY 143 

    0x70504580,// 147 PAY 144 

    0x08196be7,// 148 PAY 145 

    0x47f9c9a8,// 149 PAY 146 

    0x1ed13e6d,// 150 PAY 147 

    0x0726a8a8,// 151 PAY 148 

    0x0aab3b8e,// 152 PAY 149 

    0x4bca2ee4,// 153 PAY 150 

    0xcee40494,// 154 PAY 151 

    0x5d501a7c,// 155 PAY 152 

    0x9ec81976,// 156 PAY 153 

    0xedd90430,// 157 PAY 154 

    0x3d8adc5a,// 158 PAY 155 

    0x704f5ec4,// 159 PAY 156 

    0xea56f15a,// 160 PAY 157 

    0xa0f0204a,// 161 PAY 158 

    0x87a85d04,// 162 PAY 159 

    0x264ac7e3,// 163 PAY 160 

    0xc4e242eb,// 164 PAY 161 

    0xdbb1cba5,// 165 PAY 162 

    0xfa00fd93,// 166 PAY 163 

    0x871445c0,// 167 PAY 164 

    0xeb48557f,// 168 PAY 165 

    0xe3b61d77,// 169 PAY 166 

    0x59b4a6b5,// 170 PAY 167 

    0x008252da,// 171 PAY 168 

    0x6c7343d3,// 172 PAY 169 

    0x690cd58c,// 173 PAY 170 

    0xd20ce8c3,// 174 PAY 171 

    0xd90cdcfd,// 175 PAY 172 

    0xe5506a2d,// 176 PAY 173 

    0xeb9c8653,// 177 PAY 174 

    0xe68c5753,// 178 PAY 175 

    0x4fcd8b9a,// 179 PAY 176 

    0x693e0d98,// 180 PAY 177 

    0x62402509,// 181 PAY 178 

    0x3ce242d6,// 182 PAY 179 

    0xf5415868,// 183 PAY 180 

    0x27cdbad0,// 184 PAY 181 

    0x42421e39,// 185 PAY 182 

    0xa9e0ed78,// 186 PAY 183 

    0xdb25eabc,// 187 PAY 184 

    0x7a1619ca,// 188 PAY 185 

    0xf7351055,// 189 PAY 186 

    0x95ace44f,// 190 PAY 187 

    0xb702db15,// 191 PAY 188 

    0x90997c78,// 192 PAY 189 

    0x7ba177eb,// 193 PAY 190 

    0x31b982cb,// 194 PAY 191 

    0xcdaa88ed,// 195 PAY 192 

    0xe131cced,// 196 PAY 193 

    0xb2feb727,// 197 PAY 194 

    0xeafb8c7a,// 198 PAY 195 

    0x13e54d8f,// 199 PAY 196 

    0xb0a53b87,// 200 PAY 197 

    0x4d52705d,// 201 PAY 198 

    0x1845787f,// 202 PAY 199 

    0x40ef3a1f,// 203 PAY 200 

    0xb45df44b,// 204 PAY 201 

    0x04fdcf4b,// 205 PAY 202 

    0x75a181f8,// 206 PAY 203 

    0x0da6fce8,// 207 PAY 204 

    0x1f22b734,// 208 PAY 205 

    0xad256680,// 209 PAY 206 

    0xc0e00714,// 210 PAY 207 

    0x72d29cdf,// 211 PAY 208 

    0x0829cd00,// 212 PAY 209 

    0x622ac2d1,// 213 PAY 210 

    0x9d69eeba,// 214 PAY 211 

    0x5f4e0750,// 215 PAY 212 

    0xdb8580c5,// 216 PAY 213 

    0x7d8cba06,// 217 PAY 214 

    0x0ed707fe,// 218 PAY 215 

    0x847e0cb3,// 219 PAY 216 

    0x90f06b6c,// 220 PAY 217 

    0x4f84b750,// 221 PAY 218 

    0xfa934b27,// 222 PAY 219 

    0xd57d72c5,// 223 PAY 220 

    0x1f312b64,// 224 PAY 221 

    0xf231f09f,// 225 PAY 222 

    0xfe17a75d,// 226 PAY 223 

    0x2b8ef163,// 227 PAY 224 

    0x2f89c786,// 228 PAY 225 

    0x872c2705,// 229 PAY 226 

    0x400dd94b,// 230 PAY 227 

    0xd25e1bb1,// 231 PAY 228 

    0xbc4358ff,// 232 PAY 229 

    0xb36cbe31,// 233 PAY 230 

    0x93604ba3,// 234 PAY 231 

    0xaee006ce,// 235 PAY 232 

    0x6f5b004e,// 236 PAY 233 

    0x559e4c01,// 237 PAY 234 

    0xb6a66071,// 238 PAY 235 

    0x8cc75d6b,// 239 PAY 236 

    0xa1f12729,// 240 PAY 237 

    0x9d6d26b8,// 241 PAY 238 

    0x205bf471,// 242 PAY 239 

    0x84d5b74a,// 243 PAY 240 

    0xc038b342,// 244 PAY 241 

    0x4269473d,// 245 PAY 242 

    0xb98ee5fb,// 246 PAY 243 

    0xee49f643,// 247 PAY 244 

    0x89280864,// 248 PAY 245 

    0x6104aa23,// 249 PAY 246 

    0x40768f09,// 250 PAY 247 

    0x9b4d52f6,// 251 PAY 248 

    0x26319398,// 252 PAY 249 

    0xf32c7be9,// 253 PAY 250 

    0x8f1d73e6,// 254 PAY 251 

    0x9491ba8b,// 255 PAY 252 

    0xaf0444c2,// 256 PAY 253 

    0x180a5162,// 257 PAY 254 

    0x440d626d,// 258 PAY 255 

    0x5863f5fe,// 259 PAY 256 

    0xf27d2a7a,// 260 PAY 257 

    0x69e1e3c4,// 261 PAY 258 

    0x5766e5b3,// 262 PAY 259 

    0x35fad33e,// 263 PAY 260 

    0x97b21743,// 264 PAY 261 

    0xbcc961f1,// 265 PAY 262 

    0xd8e7f5b8,// 266 PAY 263 

    0x178640a8,// 267 PAY 264 

    0x12cba33a,// 268 PAY 265 

    0x1b90133a,// 269 PAY 266 

    0xa6ec30a0,// 270 PAY 267 

    0x72a9c0bd,// 271 PAY 268 

    0x6bb87700,// 272 PAY 269 

/// HASH is  12 bytes 

    0x400dd94b,// 273 HSH   1 

    0xd25e1bb1,// 274 HSH   2 

    0xbc4358ff,// 275 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 201 

/// STA pkt_idx        : 14 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5 

    0x003905c9 // 276 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt97_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 471 words. 

/// BDA size     is 1877 (0x755) 

/// BDA id       is 0x2790 

    0x07552790,// 3 BDA   1 

/// PAY Generic Data size   : 1877 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xc63c9cf6,// 4 PAY   1 

    0x04a571de,// 5 PAY   2 

    0xb110deb6,// 6 PAY   3 

    0xf3f431bd,// 7 PAY   4 

    0x6695e23f,// 8 PAY   5 

    0xc995fc39,// 9 PAY   6 

    0x7ff5b6a1,// 10 PAY   7 

    0xf4d22e3b,// 11 PAY   8 

    0x170717e1,// 12 PAY   9 

    0x0877efca,// 13 PAY  10 

    0xdca6edda,// 14 PAY  11 

    0xad830147,// 15 PAY  12 

    0x069b80e0,// 16 PAY  13 

    0x50205f30,// 17 PAY  14 

    0x07751193,// 18 PAY  15 

    0x743fe236,// 19 PAY  16 

    0xb2acf913,// 20 PAY  17 

    0x57aabb8d,// 21 PAY  18 

    0x6c323cfb,// 22 PAY  19 

    0xd6d27a20,// 23 PAY  20 

    0x6dbf52cc,// 24 PAY  21 

    0xd0b5f5ba,// 25 PAY  22 

    0xa9e459d8,// 26 PAY  23 

    0x56ecb655,// 27 PAY  24 

    0x5beec166,// 28 PAY  25 

    0x7ce36151,// 29 PAY  26 

    0x0c9a361c,// 30 PAY  27 

    0xc424f76f,// 31 PAY  28 

    0x2023af5e,// 32 PAY  29 

    0x3860f43f,// 33 PAY  30 

    0xfd7de8be,// 34 PAY  31 

    0xde15ffd1,// 35 PAY  32 

    0xa2a885e4,// 36 PAY  33 

    0xb0aeb669,// 37 PAY  34 

    0xe772554f,// 38 PAY  35 

    0x13f3263d,// 39 PAY  36 

    0x725595ad,// 40 PAY  37 

    0x1e8874e1,// 41 PAY  38 

    0xbe8bbe16,// 42 PAY  39 

    0xed7d9f49,// 43 PAY  40 

    0x088f11af,// 44 PAY  41 

    0xdf79999b,// 45 PAY  42 

    0x02cd13f7,// 46 PAY  43 

    0xf1563c81,// 47 PAY  44 

    0xfda03c99,// 48 PAY  45 

    0x3ecf574f,// 49 PAY  46 

    0xdc058e1b,// 50 PAY  47 

    0x925c2008,// 51 PAY  48 

    0x7cd2613a,// 52 PAY  49 

    0xf5cf24f1,// 53 PAY  50 

    0xeea7e56c,// 54 PAY  51 

    0x96de7702,// 55 PAY  52 

    0x85b30cee,// 56 PAY  53 

    0x08b56653,// 57 PAY  54 

    0x6d40ad30,// 58 PAY  55 

    0x6a4748f9,// 59 PAY  56 

    0x6f6feece,// 60 PAY  57 

    0xc9e8169e,// 61 PAY  58 

    0xde68176c,// 62 PAY  59 

    0x87685a11,// 63 PAY  60 

    0xfc55b3f1,// 64 PAY  61 

    0x7944a9ed,// 65 PAY  62 

    0x319d5ead,// 66 PAY  63 

    0x236654bf,// 67 PAY  64 

    0x98591c73,// 68 PAY  65 

    0xfd178054,// 69 PAY  66 

    0x9c0eaacb,// 70 PAY  67 

    0x1cc267e0,// 71 PAY  68 

    0xac060cf6,// 72 PAY  69 

    0xf1a6be6b,// 73 PAY  70 

    0xe00564b3,// 74 PAY  71 

    0x8ad88c6f,// 75 PAY  72 

    0x54cf669c,// 76 PAY  73 

    0x6ccd6f89,// 77 PAY  74 

    0xea7f1291,// 78 PAY  75 

    0xea7f3198,// 79 PAY  76 

    0xcbf28b72,// 80 PAY  77 

    0x8dfa2245,// 81 PAY  78 

    0x007931da,// 82 PAY  79 

    0xf11985d5,// 83 PAY  80 

    0xbdfce886,// 84 PAY  81 

    0x29cce472,// 85 PAY  82 

    0xf39a04ba,// 86 PAY  83 

    0x77ad0e9e,// 87 PAY  84 

    0x83ea5237,// 88 PAY  85 

    0xf7cfe022,// 89 PAY  86 

    0x2271f197,// 90 PAY  87 

    0x48a23c4b,// 91 PAY  88 

    0x462c30e5,// 92 PAY  89 

    0x074045be,// 93 PAY  90 

    0x7306df8e,// 94 PAY  91 

    0xbba159ce,// 95 PAY  92 

    0xc13d3ef4,// 96 PAY  93 

    0x02ccc974,// 97 PAY  94 

    0xe20481fc,// 98 PAY  95 

    0x0f130fdb,// 99 PAY  96 

    0xb841ca7c,// 100 PAY  97 

    0x20d60b26,// 101 PAY  98 

    0xd9cd1c45,// 102 PAY  99 

    0xd604a1b7,// 103 PAY 100 

    0xa3ce9237,// 104 PAY 101 

    0x119a5591,// 105 PAY 102 

    0xf98e28b6,// 106 PAY 103 

    0x5bf63f94,// 107 PAY 104 

    0x04b2372d,// 108 PAY 105 

    0x0c378c43,// 109 PAY 106 

    0x825530d1,// 110 PAY 107 

    0xb28725cb,// 111 PAY 108 

    0x343d0f4c,// 112 PAY 109 

    0xe5af4a54,// 113 PAY 110 

    0x4214da81,// 114 PAY 111 

    0x16bb9a35,// 115 PAY 112 

    0xa7b9ad5e,// 116 PAY 113 

    0x16b5eb49,// 117 PAY 114 

    0x1c4c709a,// 118 PAY 115 

    0x8fa136e8,// 119 PAY 116 

    0xe601ee60,// 120 PAY 117 

    0xe30ffd74,// 121 PAY 118 

    0x13aba9ff,// 122 PAY 119 

    0x29814c38,// 123 PAY 120 

    0x8f5ca58a,// 124 PAY 121 

    0x74c53d7f,// 125 PAY 122 

    0xd5cfafcb,// 126 PAY 123 

    0x7815ccff,// 127 PAY 124 

    0xa0982ccc,// 128 PAY 125 

    0xce85226f,// 129 PAY 126 

    0x3cc857a1,// 130 PAY 127 

    0x3fce31c3,// 131 PAY 128 

    0xeb7cbbde,// 132 PAY 129 

    0xab305c06,// 133 PAY 130 

    0xed53d6ef,// 134 PAY 131 

    0xb4054c5a,// 135 PAY 132 

    0x477878e2,// 136 PAY 133 

    0xb60f5310,// 137 PAY 134 

    0x10b6fdb6,// 138 PAY 135 

    0xab6dfb20,// 139 PAY 136 

    0x61b8fa01,// 140 PAY 137 

    0xe96bc45c,// 141 PAY 138 

    0x003232ec,// 142 PAY 139 

    0xd5cc0ec1,// 143 PAY 140 

    0x021e2b35,// 144 PAY 141 

    0x134d9fb4,// 145 PAY 142 

    0xa6f0fa3f,// 146 PAY 143 

    0xf0a40a32,// 147 PAY 144 

    0x3c874f63,// 148 PAY 145 

    0x5b13183d,// 149 PAY 146 

    0x9593323f,// 150 PAY 147 

    0x7e698a67,// 151 PAY 148 

    0xa4528198,// 152 PAY 149 

    0x24af893e,// 153 PAY 150 

    0x59009334,// 154 PAY 151 

    0xb4ab7955,// 155 PAY 152 

    0x5b6c6a67,// 156 PAY 153 

    0xd19fb2bf,// 157 PAY 154 

    0x979a9718,// 158 PAY 155 

    0x29d3852c,// 159 PAY 156 

    0x731b3381,// 160 PAY 157 

    0x6fa293aa,// 161 PAY 158 

    0xb916a974,// 162 PAY 159 

    0x52923006,// 163 PAY 160 

    0xe5c23882,// 164 PAY 161 

    0x2021062f,// 165 PAY 162 

    0xa3423d35,// 166 PAY 163 

    0xa06b2f4d,// 167 PAY 164 

    0x56851aec,// 168 PAY 165 

    0xbcc422d1,// 169 PAY 166 

    0x532eb3f5,// 170 PAY 167 

    0xf09fac1d,// 171 PAY 168 

    0x683132c4,// 172 PAY 169 

    0x27fcd3a0,// 173 PAY 170 

    0x476b8dd4,// 174 PAY 171 

    0x3d072b19,// 175 PAY 172 

    0x02944d21,// 176 PAY 173 

    0xbcd0955b,// 177 PAY 174 

    0x4df7927b,// 178 PAY 175 

    0xff615277,// 179 PAY 176 

    0xf49b720e,// 180 PAY 177 

    0x98489a2f,// 181 PAY 178 

    0x1de5fb07,// 182 PAY 179 

    0xaa66a867,// 183 PAY 180 

    0x57492c0d,// 184 PAY 181 

    0x05428b17,// 185 PAY 182 

    0x011ec902,// 186 PAY 183 

    0xa80068bb,// 187 PAY 184 

    0x7f81aa36,// 188 PAY 185 

    0x7623226c,// 189 PAY 186 

    0xa5c5fd4d,// 190 PAY 187 

    0xd5ce623b,// 191 PAY 188 

    0x70e1bd8b,// 192 PAY 189 

    0xdb2f9a80,// 193 PAY 190 

    0x4c9a6df2,// 194 PAY 191 

    0xa191d230,// 195 PAY 192 

    0xee0d387f,// 196 PAY 193 

    0x4992ff98,// 197 PAY 194 

    0xfca8f6e6,// 198 PAY 195 

    0xbdcfd88d,// 199 PAY 196 

    0x6dad345d,// 200 PAY 197 

    0xa6b4a190,// 201 PAY 198 

    0x8af4f514,// 202 PAY 199 

    0x72ec6674,// 203 PAY 200 

    0x306cad6a,// 204 PAY 201 

    0x8bf18532,// 205 PAY 202 

    0xa632a11d,// 206 PAY 203 

    0x7b2496c2,// 207 PAY 204 

    0xac4b5d8c,// 208 PAY 205 

    0x8f0317d7,// 209 PAY 206 

    0xfa0ff488,// 210 PAY 207 

    0xe5c8f182,// 211 PAY 208 

    0x524922cc,// 212 PAY 209 

    0x00bbaa5f,// 213 PAY 210 

    0xa573da76,// 214 PAY 211 

    0x6cff6a7e,// 215 PAY 212 

    0x7a940275,// 216 PAY 213 

    0xcd682fc8,// 217 PAY 214 

    0xe1a1b24f,// 218 PAY 215 

    0x53a424d7,// 219 PAY 216 

    0xd91e58ad,// 220 PAY 217 

    0xabdb3edc,// 221 PAY 218 

    0x98a41c61,// 222 PAY 219 

    0xea508f19,// 223 PAY 220 

    0xbf4d1bb0,// 224 PAY 221 

    0x7e06fe70,// 225 PAY 222 

    0x74fbb5c8,// 226 PAY 223 

    0xf7eef451,// 227 PAY 224 

    0x29112b23,// 228 PAY 225 

    0x9199dee5,// 229 PAY 226 

    0x04008a6d,// 230 PAY 227 

    0x574b459b,// 231 PAY 228 

    0x645ebd52,// 232 PAY 229 

    0x70a3f62a,// 233 PAY 230 

    0xe4cde2a3,// 234 PAY 231 

    0xb02a72b6,// 235 PAY 232 

    0xf55dfe4c,// 236 PAY 233 

    0x1e211359,// 237 PAY 234 

    0xa2c89da3,// 238 PAY 235 

    0x66f33915,// 239 PAY 236 

    0x03d6584e,// 240 PAY 237 

    0x01e19e62,// 241 PAY 238 

    0x1536493a,// 242 PAY 239 

    0x026e482e,// 243 PAY 240 

    0x0ad9acdc,// 244 PAY 241 

    0x60bb6637,// 245 PAY 242 

    0x0e04bf6d,// 246 PAY 243 

    0xa4f8e2bc,// 247 PAY 244 

    0xec89f65b,// 248 PAY 245 

    0x4839e516,// 249 PAY 246 

    0x448d48a5,// 250 PAY 247 

    0x16302c3d,// 251 PAY 248 

    0xd8b9168d,// 252 PAY 249 

    0x1af422ea,// 253 PAY 250 

    0xbcd69d59,// 254 PAY 251 

    0xd6956a55,// 255 PAY 252 

    0xc1003556,// 256 PAY 253 

    0x1b21d2d9,// 257 PAY 254 

    0xab66654d,// 258 PAY 255 

    0x3bf8091e,// 259 PAY 256 

    0xde471115,// 260 PAY 257 

    0x0005c86f,// 261 PAY 258 

    0xce352665,// 262 PAY 259 

    0xc0cbf3d2,// 263 PAY 260 

    0x2697ee7b,// 264 PAY 261 

    0xc4a3d051,// 265 PAY 262 

    0x378e3cd6,// 266 PAY 263 

    0xc73b7ab7,// 267 PAY 264 

    0x51c52d0a,// 268 PAY 265 

    0x359242b5,// 269 PAY 266 

    0x92b0ec6c,// 270 PAY 267 

    0x069a2557,// 271 PAY 268 

    0x5b1869de,// 272 PAY 269 

    0x5cfc2992,// 273 PAY 270 

    0x60e6f8e6,// 274 PAY 271 

    0x0ef890c5,// 275 PAY 272 

    0x085e24fe,// 276 PAY 273 

    0x236ccec8,// 277 PAY 274 

    0x9a6219d3,// 278 PAY 275 

    0x849b5c74,// 279 PAY 276 

    0x38298aa1,// 280 PAY 277 

    0xda2761bf,// 281 PAY 278 

    0xeb15a2e7,// 282 PAY 279 

    0xf2983856,// 283 PAY 280 

    0xaa4cd4e8,// 284 PAY 281 

    0x8d5725fb,// 285 PAY 282 

    0x132e6cf0,// 286 PAY 283 

    0x1ea1369c,// 287 PAY 284 

    0x85fc902d,// 288 PAY 285 

    0x875d7a14,// 289 PAY 286 

    0x8fa306f2,// 290 PAY 287 

    0xb9e1c01f,// 291 PAY 288 

    0x74184dfe,// 292 PAY 289 

    0xeee259df,// 293 PAY 290 

    0xb628b421,// 294 PAY 291 

    0x361c6c87,// 295 PAY 292 

    0x4c97506a,// 296 PAY 293 

    0xb22c6c6d,// 297 PAY 294 

    0x770718ef,// 298 PAY 295 

    0x437e1236,// 299 PAY 296 

    0x9b591cbc,// 300 PAY 297 

    0x496d1b0b,// 301 PAY 298 

    0xc46ac646,// 302 PAY 299 

    0x428a682b,// 303 PAY 300 

    0x5f71c6e8,// 304 PAY 301 

    0x907d38e6,// 305 PAY 302 

    0x0bb586e5,// 306 PAY 303 

    0xb9b15f15,// 307 PAY 304 

    0x4c91c67f,// 308 PAY 305 

    0x5c8748ac,// 309 PAY 306 

    0x3d2e099f,// 310 PAY 307 

    0xff9a325d,// 311 PAY 308 

    0x68df2a34,// 312 PAY 309 

    0xaa71fb05,// 313 PAY 310 

    0x340d7dac,// 314 PAY 311 

    0xc13332b4,// 315 PAY 312 

    0x7cfa6e6e,// 316 PAY 313 

    0x559954bc,// 317 PAY 314 

    0x5f1f70f1,// 318 PAY 315 

    0x2c47fbc4,// 319 PAY 316 

    0xda1ef2a1,// 320 PAY 317 

    0x509b0843,// 321 PAY 318 

    0x5e699ce9,// 322 PAY 319 

    0x01a92f35,// 323 PAY 320 

    0x9b256127,// 324 PAY 321 

    0x91a07896,// 325 PAY 322 

    0xab4edcac,// 326 PAY 323 

    0x3ee96b83,// 327 PAY 324 

    0x8a328129,// 328 PAY 325 

    0x0db6834a,// 329 PAY 326 

    0x024f18bc,// 330 PAY 327 

    0xe415b0cc,// 331 PAY 328 

    0x9a76d8dd,// 332 PAY 329 

    0x306e062f,// 333 PAY 330 

    0x45827b25,// 334 PAY 331 

    0x87a04682,// 335 PAY 332 

    0xe80219b4,// 336 PAY 333 

    0xee2a9484,// 337 PAY 334 

    0xf3f02aa8,// 338 PAY 335 

    0x671319fa,// 339 PAY 336 

    0xf125a6bf,// 340 PAY 337 

    0x1c9a0a56,// 341 PAY 338 

    0x4780f916,// 342 PAY 339 

    0x5bb92f3a,// 343 PAY 340 

    0x195cf9fb,// 344 PAY 341 

    0xba739581,// 345 PAY 342 

    0xcc55f2a0,// 346 PAY 343 

    0x2ab9040b,// 347 PAY 344 

    0x0212ea7d,// 348 PAY 345 

    0x64b6f245,// 349 PAY 346 

    0x17a12a13,// 350 PAY 347 

    0x59e892ec,// 351 PAY 348 

    0x40a76c66,// 352 PAY 349 

    0x1a8d8ca1,// 353 PAY 350 

    0x6caf1bf2,// 354 PAY 351 

    0x0e35daf6,// 355 PAY 352 

    0x4e191d22,// 356 PAY 353 

    0xfcb9eb5b,// 357 PAY 354 

    0x91e2d617,// 358 PAY 355 

    0xef62274d,// 359 PAY 356 

    0xa9966f1a,// 360 PAY 357 

    0x587a9fee,// 361 PAY 358 

    0x5b385429,// 362 PAY 359 

    0xf534870d,// 363 PAY 360 

    0x8041894f,// 364 PAY 361 

    0x234db282,// 365 PAY 362 

    0x85298b91,// 366 PAY 363 

    0xeba6c5c1,// 367 PAY 364 

    0xe5cf863c,// 368 PAY 365 

    0xc968f6f6,// 369 PAY 366 

    0x4e24e6ed,// 370 PAY 367 

    0x30ed9685,// 371 PAY 368 

    0x72f8d9ee,// 372 PAY 369 

    0x86df7306,// 373 PAY 370 

    0x302be814,// 374 PAY 371 

    0x560af2cd,// 375 PAY 372 

    0xd2058dac,// 376 PAY 373 

    0xce447cdd,// 377 PAY 374 

    0x197899f1,// 378 PAY 375 

    0xbb7d4d99,// 379 PAY 376 

    0xa6f2c5aa,// 380 PAY 377 

    0x55954a74,// 381 PAY 378 

    0xf264aec4,// 382 PAY 379 

    0x5562a097,// 383 PAY 380 

    0x0c202f14,// 384 PAY 381 

    0x7011ca9f,// 385 PAY 382 

    0xbd4ee60b,// 386 PAY 383 

    0x83333fc9,// 387 PAY 384 

    0xed8fdf0c,// 388 PAY 385 

    0x72ab6664,// 389 PAY 386 

    0xdd832485,// 390 PAY 387 

    0xabd336f5,// 391 PAY 388 

    0x5df03629,// 392 PAY 389 

    0x9572f065,// 393 PAY 390 

    0x171a7353,// 394 PAY 391 

    0x35fe9d36,// 395 PAY 392 

    0x2e27ce3c,// 396 PAY 393 

    0xe4372f4c,// 397 PAY 394 

    0x6d4b9bd6,// 398 PAY 395 

    0xdb4a1235,// 399 PAY 396 

    0x8d753ad2,// 400 PAY 397 

    0x8e1bb63a,// 401 PAY 398 

    0xf2b1b5f6,// 402 PAY 399 

    0xb632801c,// 403 PAY 400 

    0x8a19bdaa,// 404 PAY 401 

    0x0fc72f86,// 405 PAY 402 

    0x62a3bf1a,// 406 PAY 403 

    0x80b2bbff,// 407 PAY 404 

    0x84c8d367,// 408 PAY 405 

    0x3fdbfce3,// 409 PAY 406 

    0xab83cb1d,// 410 PAY 407 

    0xb88856a4,// 411 PAY 408 

    0xa279be18,// 412 PAY 409 

    0x0b21a859,// 413 PAY 410 

    0x34cde65a,// 414 PAY 411 

    0x326fc0b3,// 415 PAY 412 

    0xbbdf7636,// 416 PAY 413 

    0xe50beb06,// 417 PAY 414 

    0x6941c025,// 418 PAY 415 

    0xd44ee9d9,// 419 PAY 416 

    0xad424f2b,// 420 PAY 417 

    0x571a008f,// 421 PAY 418 

    0x2399f4f9,// 422 PAY 419 

    0x6923bb9a,// 423 PAY 420 

    0xf8ab4b43,// 424 PAY 421 

    0xb57c66ad,// 425 PAY 422 

    0x15113296,// 426 PAY 423 

    0xe076b01f,// 427 PAY 424 

    0xb806ec34,// 428 PAY 425 

    0x9c81faae,// 429 PAY 426 

    0x79584d78,// 430 PAY 427 

    0xf18bd7a0,// 431 PAY 428 

    0xbf5e9586,// 432 PAY 429 

    0x732ae987,// 433 PAY 430 

    0x27ec5a6b,// 434 PAY 431 

    0x757ab98b,// 435 PAY 432 

    0x9ede208f,// 436 PAY 433 

    0x824132c3,// 437 PAY 434 

    0x55d199df,// 438 PAY 435 

    0xbce6c72b,// 439 PAY 436 

    0xaf7ef141,// 440 PAY 437 

    0x3d6aa3b7,// 441 PAY 438 

    0xecf54d6a,// 442 PAY 439 

    0x4270129e,// 443 PAY 440 

    0x4cc99827,// 444 PAY 441 

    0x3bd37541,// 445 PAY 442 

    0x49c0a80a,// 446 PAY 443 

    0x23a8919b,// 447 PAY 444 

    0x9d2cf074,// 448 PAY 445 

    0xd9958cf4,// 449 PAY 446 

    0x7cf956cc,// 450 PAY 447 

    0x24b5446e,// 451 PAY 448 

    0xfc331338,// 452 PAY 449 

    0x412f7096,// 453 PAY 450 

    0x7db88436,// 454 PAY 451 

    0x6903fd13,// 455 PAY 452 

    0x9f4f25c6,// 456 PAY 453 

    0x2823e931,// 457 PAY 454 

    0x98cd6418,// 458 PAY 455 

    0xf006b46b,// 459 PAY 456 

    0xfdae42d8,// 460 PAY 457 

    0x564e788b,// 461 PAY 458 

    0x3560a910,// 462 PAY 459 

    0x3a3cb5ff,// 463 PAY 460 

    0x693e5487,// 464 PAY 461 

    0x160748d6,// 465 PAY 462 

    0xa087ec2f,// 466 PAY 463 

    0x964b933b,// 467 PAY 464 

    0x36f5287f,// 468 PAY 465 

    0x5f884f59,// 469 PAY 466 

    0x927065d5,// 470 PAY 467 

    0x93682183,// 471 PAY 468 

    0xc25dfef6,// 472 PAY 469 

    0x43000000,// 473 PAY 470 

/// HASH is  8 bytes 

    0x83ea5237,// 474 HSH   1 

    0xf7cfe022,// 475 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 248 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbf 

    0x03e1bf74 // 476 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt98_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 286 words. 

/// BDA size     is 1138 (0x472) 

/// BDA id       is 0xf658 

    0x0472f658,// 3 BDA   1 

/// PAY Generic Data size   : 1138 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xf1517780,// 4 PAY   1 

    0x84820ff9,// 5 PAY   2 

    0x889eeea7,// 6 PAY   3 

    0xc3f0f57c,// 7 PAY   4 

    0x84f6b8d7,// 8 PAY   5 

    0x1d3d4d79,// 9 PAY   6 

    0x3f81c9e8,// 10 PAY   7 

    0x22cc582e,// 11 PAY   8 

    0x6283c884,// 12 PAY   9 

    0x4df89d68,// 13 PAY  10 

    0xbd07e875,// 14 PAY  11 

    0x8e3922ab,// 15 PAY  12 

    0x6771906b,// 16 PAY  13 

    0xf2aad605,// 17 PAY  14 

    0xdddb0379,// 18 PAY  15 

    0x0e461363,// 19 PAY  16 

    0x3f4e32fe,// 20 PAY  17 

    0xd581deb1,// 21 PAY  18 

    0x44d5f71f,// 22 PAY  19 

    0x17de5d15,// 23 PAY  20 

    0x16e9b2b7,// 24 PAY  21 

    0x5654228e,// 25 PAY  22 

    0x8654272d,// 26 PAY  23 

    0xf29aff98,// 27 PAY  24 

    0xc9d831f7,// 28 PAY  25 

    0x587b5a59,// 29 PAY  26 

    0x2eb5ab64,// 30 PAY  27 

    0x1f5cfbb1,// 31 PAY  28 

    0x8105a444,// 32 PAY  29 

    0x6bc1608b,// 33 PAY  30 

    0x80a7bf2f,// 34 PAY  31 

    0xca2acdb1,// 35 PAY  32 

    0xcfd0c5b0,// 36 PAY  33 

    0xc513b78e,// 37 PAY  34 

    0x8eaecb62,// 38 PAY  35 

    0x57be620d,// 39 PAY  36 

    0x2f7f9107,// 40 PAY  37 

    0x0714b9e6,// 41 PAY  38 

    0x3ef77909,// 42 PAY  39 

    0xe9090c6d,// 43 PAY  40 

    0x10df8f76,// 44 PAY  41 

    0x30272967,// 45 PAY  42 

    0xd120f908,// 46 PAY  43 

    0xa0e71cf5,// 47 PAY  44 

    0x6e39c08c,// 48 PAY  45 

    0x7152189f,// 49 PAY  46 

    0x4e0a62ba,// 50 PAY  47 

    0x4ed8463e,// 51 PAY  48 

    0x5447556b,// 52 PAY  49 

    0x7528aa63,// 53 PAY  50 

    0x905094e0,// 54 PAY  51 

    0xa1c491dc,// 55 PAY  52 

    0xa9f84792,// 56 PAY  53 

    0xc6c4b9c1,// 57 PAY  54 

    0xb40c41d2,// 58 PAY  55 

    0x65511422,// 59 PAY  56 

    0x30536b06,// 60 PAY  57 

    0xd83aeccb,// 61 PAY  58 

    0x6a30826c,// 62 PAY  59 

    0x57da06fa,// 63 PAY  60 

    0x6af97277,// 64 PAY  61 

    0xbd68a096,// 65 PAY  62 

    0x3697a48d,// 66 PAY  63 

    0xcc28c6a8,// 67 PAY  64 

    0xfb5b5d0c,// 68 PAY  65 

    0x3242f117,// 69 PAY  66 

    0x07fe852c,// 70 PAY  67 

    0xec982d06,// 71 PAY  68 

    0xb9a01979,// 72 PAY  69 

    0xb100327b,// 73 PAY  70 

    0xab2ebf27,// 74 PAY  71 

    0xd285d4e7,// 75 PAY  72 

    0x31d4cd1b,// 76 PAY  73 

    0xf76f4695,// 77 PAY  74 

    0x1cfd8dc9,// 78 PAY  75 

    0x92378d00,// 79 PAY  76 

    0xef750f1e,// 80 PAY  77 

    0x5c01b66b,// 81 PAY  78 

    0x9a7132d0,// 82 PAY  79 

    0x340a9055,// 83 PAY  80 

    0x657f4e58,// 84 PAY  81 

    0x25b28e20,// 85 PAY  82 

    0x44e70be6,// 86 PAY  83 

    0x2d958077,// 87 PAY  84 

    0x138a20b7,// 88 PAY  85 

    0x91417178,// 89 PAY  86 

    0xba646011,// 90 PAY  87 

    0xd11880c1,// 91 PAY  88 

    0x86110f52,// 92 PAY  89 

    0x89d09950,// 93 PAY  90 

    0x6be46fb5,// 94 PAY  91 

    0x448e504f,// 95 PAY  92 

    0xc1c8451f,// 96 PAY  93 

    0x95843307,// 97 PAY  94 

    0x2cfb3789,// 98 PAY  95 

    0x71a7a864,// 99 PAY  96 

    0x2d2e29c2,// 100 PAY  97 

    0x43e0c023,// 101 PAY  98 

    0x02a6c2b1,// 102 PAY  99 

    0x8f2cd457,// 103 PAY 100 

    0x886d5059,// 104 PAY 101 

    0xa7502f1e,// 105 PAY 102 

    0x59045e7a,// 106 PAY 103 

    0x660e1d41,// 107 PAY 104 

    0x3135d67f,// 108 PAY 105 

    0xba7038b8,// 109 PAY 106 

    0x9b246582,// 110 PAY 107 

    0x86866414,// 111 PAY 108 

    0x6640c954,// 112 PAY 109 

    0x8b981e48,// 113 PAY 110 

    0x0cf3ba07,// 114 PAY 111 

    0x33131c25,// 115 PAY 112 

    0x48f4e19b,// 116 PAY 113 

    0x8100fdda,// 117 PAY 114 

    0x975af90a,// 118 PAY 115 

    0xaa8747e4,// 119 PAY 116 

    0x72ab269e,// 120 PAY 117 

    0x4f120bd2,// 121 PAY 118 

    0xba3a4bbc,// 122 PAY 119 

    0x27557ada,// 123 PAY 120 

    0x4857dd83,// 124 PAY 121 

    0xb2c2a6e5,// 125 PAY 122 

    0x9fc0edbf,// 126 PAY 123 

    0x934a9aa9,// 127 PAY 124 

    0x9eb9f752,// 128 PAY 125 

    0xf12c324f,// 129 PAY 126 

    0x3a196a6a,// 130 PAY 127 

    0x9e4eea8b,// 131 PAY 128 

    0x0e45953f,// 132 PAY 129 

    0x1fdc51ce,// 133 PAY 130 

    0x77c2eb64,// 134 PAY 131 

    0x4858ab9d,// 135 PAY 132 

    0xcac1a27c,// 136 PAY 133 

    0x63de9c7b,// 137 PAY 134 

    0x850e9c79,// 138 PAY 135 

    0xe0ebf95b,// 139 PAY 136 

    0xfa4006e5,// 140 PAY 137 

    0x6915dbb7,// 141 PAY 138 

    0x2c485a7b,// 142 PAY 139 

    0x1a3e92a0,// 143 PAY 140 

    0xef2db4ae,// 144 PAY 141 

    0x8ed13b9f,// 145 PAY 142 

    0x4ce98254,// 146 PAY 143 

    0xa53b9f47,// 147 PAY 144 

    0xfaa8914f,// 148 PAY 145 

    0x64855381,// 149 PAY 146 

    0x4067f643,// 150 PAY 147 

    0x25293435,// 151 PAY 148 

    0x18118dda,// 152 PAY 149 

    0x5e159449,// 153 PAY 150 

    0x2a161e49,// 154 PAY 151 

    0x9c0dcf03,// 155 PAY 152 

    0x3fc1f027,// 156 PAY 153 

    0xcf3b62ee,// 157 PAY 154 

    0xf242292b,// 158 PAY 155 

    0x4daf902f,// 159 PAY 156 

    0xcb052d89,// 160 PAY 157 

    0xf4b0365f,// 161 PAY 158 

    0xc71f8303,// 162 PAY 159 

    0x7309a9ab,// 163 PAY 160 

    0x8d727423,// 164 PAY 161 

    0x624d4378,// 165 PAY 162 

    0xa9aed522,// 166 PAY 163 

    0xdb36727b,// 167 PAY 164 

    0x5853f632,// 168 PAY 165 

    0x7c504217,// 169 PAY 166 

    0x2088a1da,// 170 PAY 167 

    0x2a2444d4,// 171 PAY 168 

    0x97df6136,// 172 PAY 169 

    0xeea6be26,// 173 PAY 170 

    0x7c04dee7,// 174 PAY 171 

    0x56e17633,// 175 PAY 172 

    0x21f25470,// 176 PAY 173 

    0xdaed3a45,// 177 PAY 174 

    0x433a539d,// 178 PAY 175 

    0xaae5caa9,// 179 PAY 176 

    0x2986bd13,// 180 PAY 177 

    0x1b73a600,// 181 PAY 178 

    0x5256bafa,// 182 PAY 179 

    0xe5c15725,// 183 PAY 180 

    0xe79b9e48,// 184 PAY 181 

    0xb2cdf505,// 185 PAY 182 

    0x6f8ffd28,// 186 PAY 183 

    0xbaa8d8d3,// 187 PAY 184 

    0x81070333,// 188 PAY 185 

    0x4d73512c,// 189 PAY 186 

    0x0f22cede,// 190 PAY 187 

    0x900020f5,// 191 PAY 188 

    0x380d4caa,// 192 PAY 189 

    0x922f8fb8,// 193 PAY 190 

    0x1cd3d5e9,// 194 PAY 191 

    0x0862b03f,// 195 PAY 192 

    0xe9dd89d2,// 196 PAY 193 

    0xec4e8eae,// 197 PAY 194 

    0x06a6ac69,// 198 PAY 195 

    0xd47d362e,// 199 PAY 196 

    0xf3f1bdcb,// 200 PAY 197 

    0x6c5f0838,// 201 PAY 198 

    0x5ac57596,// 202 PAY 199 

    0x0b9f3309,// 203 PAY 200 

    0x38936c0d,// 204 PAY 201 

    0xb485809e,// 205 PAY 202 

    0xf4b5cefa,// 206 PAY 203 

    0x4afed784,// 207 PAY 204 

    0xe6fa899f,// 208 PAY 205 

    0xccc03607,// 209 PAY 206 

    0x48421e09,// 210 PAY 207 

    0x1cb263d2,// 211 PAY 208 

    0x7d8f68f6,// 212 PAY 209 

    0xcbd8db99,// 213 PAY 210 

    0x5838024f,// 214 PAY 211 

    0x4ae6c6a4,// 215 PAY 212 

    0x6bdaad9d,// 216 PAY 213 

    0x29bd5939,// 217 PAY 214 

    0x63424744,// 218 PAY 215 

    0xef06a45a,// 219 PAY 216 

    0xf8397dc0,// 220 PAY 217 

    0x14a43e1b,// 221 PAY 218 

    0x8bbcaabd,// 222 PAY 219 

    0x5e0176ef,// 223 PAY 220 

    0xa456ef54,// 224 PAY 221 

    0x3ca83568,// 225 PAY 222 

    0xebb74e82,// 226 PAY 223 

    0xeca5f6ce,// 227 PAY 224 

    0x2a9bc015,// 228 PAY 225 

    0x7dc53165,// 229 PAY 226 

    0x268b3c0c,// 230 PAY 227 

    0x5421f823,// 231 PAY 228 

    0x99196b7c,// 232 PAY 229 

    0x75889f24,// 233 PAY 230 

    0x4eda58ad,// 234 PAY 231 

    0x75c865d5,// 235 PAY 232 

    0x8daab533,// 236 PAY 233 

    0x4e84fc56,// 237 PAY 234 

    0x29846ce3,// 238 PAY 235 

    0x4f93ce2f,// 239 PAY 236 

    0x0c5e6227,// 240 PAY 237 

    0x3e158460,// 241 PAY 238 

    0x7b1eebb9,// 242 PAY 239 

    0xf173f4f4,// 243 PAY 240 

    0x574fa1bf,// 244 PAY 241 

    0xf10129af,// 245 PAY 242 

    0xf746ce9f,// 246 PAY 243 

    0xf55a74ac,// 247 PAY 244 

    0x668aa9a5,// 248 PAY 245 

    0xcd9d4996,// 249 PAY 246 

    0x5a925662,// 250 PAY 247 

    0x6b318cb0,// 251 PAY 248 

    0xec66269e,// 252 PAY 249 

    0x99c6ead6,// 253 PAY 250 

    0x2cc4baf2,// 254 PAY 251 

    0xb3de3987,// 255 PAY 252 

    0xf1b4f16b,// 256 PAY 253 

    0x44cb7fa3,// 257 PAY 254 

    0x12b4efe6,// 258 PAY 255 

    0x11214f1d,// 259 PAY 256 

    0x389160aa,// 260 PAY 257 

    0xd731de81,// 261 PAY 258 

    0xa05bf1d7,// 262 PAY 259 

    0x72d34a74,// 263 PAY 260 

    0xc85377aa,// 264 PAY 261 

    0x843283f7,// 265 PAY 262 

    0x9273d1b7,// 266 PAY 263 

    0x50c20a3d,// 267 PAY 264 

    0x7c125b57,// 268 PAY 265 

    0xb2c5b337,// 269 PAY 266 

    0xed61f92a,// 270 PAY 267 

    0x64a57c3c,// 271 PAY 268 

    0x281045fa,// 272 PAY 269 

    0xf117252d,// 273 PAY 270 

    0xb48904b9,// 274 PAY 271 

    0x89ae39da,// 275 PAY 272 

    0xe6c8e64f,// 276 PAY 273 

    0x01c1ad29,// 277 PAY 274 

    0xd5408aaf,// 278 PAY 275 

    0x0b12ea24,// 279 PAY 276 

    0x38a2e299,// 280 PAY 277 

    0x076b6808,// 281 PAY 278 

    0x59d0fe85,// 282 PAY 279 

    0xbcff44db,// 283 PAY 280 

    0x16577072,// 284 PAY 281 

    0xc71c5d90,// 285 PAY 282 

    0x941ab6c4,// 286 PAY 283 

    0x1443c3b2,// 287 PAY 284 

    0xb3000000,// 288 PAY 285 

/// STA is 1 words. 

/// STA num_pkts       : 65 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8 

    0x02b40841 // 289 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_md5_pkt99_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 73 words. 

/// BDA size     is 285 (0x11d) 

/// BDA id       is 0xb4f7 

    0x011db4f7,// 3 BDA   1 

/// PAY Generic Data size   : 285 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xa949a0ba,// 4 PAY   1 

    0x893ef29c,// 5 PAY   2 

    0x0fae9ac3,// 6 PAY   3 

    0x2bbb2d90,// 7 PAY   4 

    0x5fffe985,// 8 PAY   5 

    0xfa8ebbbe,// 9 PAY   6 

    0x753bb6ca,// 10 PAY   7 

    0x03a144af,// 11 PAY   8 

    0x7b37334d,// 12 PAY   9 

    0x3f837a62,// 13 PAY  10 

    0x47daf4a0,// 14 PAY  11 

    0x2fd1c725,// 15 PAY  12 

    0x278549f3,// 16 PAY  13 

    0x0547c4c8,// 17 PAY  14 

    0x3b2665f0,// 18 PAY  15 

    0x0aa3e042,// 19 PAY  16 

    0x0483947c,// 20 PAY  17 

    0x6ad9b179,// 21 PAY  18 

    0xc1662820,// 22 PAY  19 

    0x7d2bee92,// 23 PAY  20 

    0x9f097648,// 24 PAY  21 

    0xadd32b27,// 25 PAY  22 

    0xc41443c6,// 26 PAY  23 

    0x01dc7b6e,// 27 PAY  24 

    0xb00571a7,// 28 PAY  25 

    0xaa2bb869,// 29 PAY  26 

    0xe70f4de4,// 30 PAY  27 

    0x80cb3be8,// 31 PAY  28 

    0x0f0ee973,// 32 PAY  29 

    0x0b386b4a,// 33 PAY  30 

    0xe6cfcb12,// 34 PAY  31 

    0xd793445a,// 35 PAY  32 

    0xc7c42ecd,// 36 PAY  33 

    0x2c3df5f0,// 37 PAY  34 

    0xb8154da9,// 38 PAY  35 

    0x917831e5,// 39 PAY  36 

    0xe0d78fcc,// 40 PAY  37 

    0xfc8b8efd,// 41 PAY  38 

    0xc7f9870c,// 42 PAY  39 

    0x09e76648,// 43 PAY  40 

    0xe3ecefae,// 44 PAY  41 

    0x30174034,// 45 PAY  42 

    0x067b99f0,// 46 PAY  43 

    0xe46b0451,// 47 PAY  44 

    0x206bccb0,// 48 PAY  45 

    0xe40d48fa,// 49 PAY  46 

    0x6eaf6f4f,// 50 PAY  47 

    0x25b9abf5,// 51 PAY  48 

    0x345a01d7,// 52 PAY  49 

    0x1870d2d5,// 53 PAY  50 

    0x49ccef81,// 54 PAY  51 

    0x7667bc36,// 55 PAY  52 

    0xc469e7b3,// 56 PAY  53 

    0xc4d12d20,// 57 PAY  54 

    0x2617b3f5,// 58 PAY  55 

    0x4658d1dc,// 59 PAY  56 

    0x292c33a2,// 60 PAY  57 

    0x2c2120c3,// 61 PAY  58 

    0x137cdfed,// 62 PAY  59 

    0x3065a8a0,// 63 PAY  60 

    0x1159efbc,// 64 PAY  61 

    0xced51a08,// 65 PAY  62 

    0x72dacfe7,// 66 PAY  63 

    0x189274da,// 67 PAY  64 

    0x346e09c6,// 68 PAY  65 

    0x0d19735f,// 69 PAY  66 

    0x785d64d3,// 70 PAY  67 

    0x95063351,// 71 PAY  68 

    0xd01f5b1f,// 72 PAY  69 

    0x76619c1c,// 73 PAY  70 

    0x176b6efc,// 74 PAY  71 

    0x41000000,// 75 PAY  72 

/// STA is 1 words. 

/// STA num_pkts       : 62 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0361e83e // 76 STA   1 

};

//


uint16 rx_aes_md5_pkt_len[] = {
    sizeof(rx_aes_md5_pkt0_tmpl),
    sizeof(rx_aes_md5_pkt1_tmpl),
    sizeof(rx_aes_md5_pkt2_tmpl),
    sizeof(rx_aes_md5_pkt3_tmpl),
    sizeof(rx_aes_md5_pkt4_tmpl),
    sizeof(rx_aes_md5_pkt5_tmpl),
    sizeof(rx_aes_md5_pkt6_tmpl),
    sizeof(rx_aes_md5_pkt7_tmpl),
    sizeof(rx_aes_md5_pkt8_tmpl),
    sizeof(rx_aes_md5_pkt9_tmpl),
    sizeof(rx_aes_md5_pkt10_tmpl),
    sizeof(rx_aes_md5_pkt11_tmpl),
    sizeof(rx_aes_md5_pkt12_tmpl),
    sizeof(rx_aes_md5_pkt13_tmpl),
    sizeof(rx_aes_md5_pkt14_tmpl),
    sizeof(rx_aes_md5_pkt15_tmpl),
    sizeof(rx_aes_md5_pkt16_tmpl),
    sizeof(rx_aes_md5_pkt17_tmpl),
    sizeof(rx_aes_md5_pkt18_tmpl),
    sizeof(rx_aes_md5_pkt19_tmpl),
    sizeof(rx_aes_md5_pkt20_tmpl),
    sizeof(rx_aes_md5_pkt21_tmpl),
    sizeof(rx_aes_md5_pkt22_tmpl),
    sizeof(rx_aes_md5_pkt23_tmpl),
    sizeof(rx_aes_md5_pkt24_tmpl),
    sizeof(rx_aes_md5_pkt25_tmpl),
    sizeof(rx_aes_md5_pkt26_tmpl),
    sizeof(rx_aes_md5_pkt27_tmpl),
    sizeof(rx_aes_md5_pkt28_tmpl),
    sizeof(rx_aes_md5_pkt29_tmpl),
    sizeof(rx_aes_md5_pkt30_tmpl),
    sizeof(rx_aes_md5_pkt31_tmpl),
    sizeof(rx_aes_md5_pkt32_tmpl),
    sizeof(rx_aes_md5_pkt33_tmpl),
    sizeof(rx_aes_md5_pkt34_tmpl),
    sizeof(rx_aes_md5_pkt35_tmpl),
    sizeof(rx_aes_md5_pkt36_tmpl),
    sizeof(rx_aes_md5_pkt37_tmpl),
    sizeof(rx_aes_md5_pkt38_tmpl),
    sizeof(rx_aes_md5_pkt39_tmpl),
    sizeof(rx_aes_md5_pkt40_tmpl),
    sizeof(rx_aes_md5_pkt41_tmpl),
    sizeof(rx_aes_md5_pkt42_tmpl),
    sizeof(rx_aes_md5_pkt43_tmpl),
    sizeof(rx_aes_md5_pkt44_tmpl),
    sizeof(rx_aes_md5_pkt45_tmpl),
    sizeof(rx_aes_md5_pkt46_tmpl),
    sizeof(rx_aes_md5_pkt47_tmpl),
    sizeof(rx_aes_md5_pkt48_tmpl),
    sizeof(rx_aes_md5_pkt49_tmpl),
    sizeof(rx_aes_md5_pkt50_tmpl),
    sizeof(rx_aes_md5_pkt51_tmpl),
    sizeof(rx_aes_md5_pkt52_tmpl),
    sizeof(rx_aes_md5_pkt53_tmpl),
    sizeof(rx_aes_md5_pkt54_tmpl),
    sizeof(rx_aes_md5_pkt55_tmpl),
    sizeof(rx_aes_md5_pkt56_tmpl),
    sizeof(rx_aes_md5_pkt57_tmpl),
    sizeof(rx_aes_md5_pkt58_tmpl),
    sizeof(rx_aes_md5_pkt59_tmpl),
    sizeof(rx_aes_md5_pkt60_tmpl),
    sizeof(rx_aes_md5_pkt61_tmpl),
    sizeof(rx_aes_md5_pkt62_tmpl),
    sizeof(rx_aes_md5_pkt63_tmpl),
    sizeof(rx_aes_md5_pkt64_tmpl),
    sizeof(rx_aes_md5_pkt65_tmpl),
    sizeof(rx_aes_md5_pkt66_tmpl),
    sizeof(rx_aes_md5_pkt67_tmpl),
    sizeof(rx_aes_md5_pkt68_tmpl),
    sizeof(rx_aes_md5_pkt69_tmpl),
    sizeof(rx_aes_md5_pkt70_tmpl),
    sizeof(rx_aes_md5_pkt71_tmpl),
    sizeof(rx_aes_md5_pkt72_tmpl),
    sizeof(rx_aes_md5_pkt73_tmpl),
    sizeof(rx_aes_md5_pkt74_tmpl),
    sizeof(rx_aes_md5_pkt75_tmpl),
    sizeof(rx_aes_md5_pkt76_tmpl),
    sizeof(rx_aes_md5_pkt77_tmpl),
    sizeof(rx_aes_md5_pkt78_tmpl),
    sizeof(rx_aes_md5_pkt79_tmpl),
    sizeof(rx_aes_md5_pkt80_tmpl),
    sizeof(rx_aes_md5_pkt81_tmpl),
    sizeof(rx_aes_md5_pkt82_tmpl),
    sizeof(rx_aes_md5_pkt83_tmpl),
    sizeof(rx_aes_md5_pkt84_tmpl),
    sizeof(rx_aes_md5_pkt85_tmpl),
    sizeof(rx_aes_md5_pkt86_tmpl),
    sizeof(rx_aes_md5_pkt87_tmpl),
    sizeof(rx_aes_md5_pkt88_tmpl),
    sizeof(rx_aes_md5_pkt89_tmpl),
    sizeof(rx_aes_md5_pkt90_tmpl),
    sizeof(rx_aes_md5_pkt91_tmpl),
    sizeof(rx_aes_md5_pkt92_tmpl),
    sizeof(rx_aes_md5_pkt93_tmpl),
    sizeof(rx_aes_md5_pkt94_tmpl),
    sizeof(rx_aes_md5_pkt95_tmpl),
    sizeof(rx_aes_md5_pkt96_tmpl),
    sizeof(rx_aes_md5_pkt97_tmpl),
    sizeof(rx_aes_md5_pkt98_tmpl),
    sizeof(rx_aes_md5_pkt99_tmpl)
};

unsigned char *rx_aes_md5_test_pkts[] = {
    (unsigned char *)&rx_aes_md5_pkt0_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt1_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt2_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt3_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt4_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt5_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt6_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt7_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt8_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt9_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt10_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt11_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt12_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt13_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt14_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt15_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt16_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt17_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt18_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt19_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt20_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt21_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt22_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt23_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt24_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt25_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt26_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt27_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt28_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt29_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt30_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt31_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt32_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt33_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt34_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt35_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt36_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt37_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt38_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt39_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt40_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt41_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt42_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt43_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt44_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt45_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt46_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt47_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt48_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt49_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt50_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt51_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt52_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt53_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt54_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt55_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt56_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt57_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt58_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt59_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt60_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt61_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt62_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt63_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt64_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt65_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt66_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt67_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt68_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt69_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt70_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt71_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt72_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt73_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt74_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt75_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt76_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt77_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt78_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt79_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt80_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt81_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt82_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt83_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt84_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt85_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt86_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt87_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt88_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt89_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt90_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt91_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt92_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt93_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt94_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt95_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt96_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt97_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt98_tmpl[0],
    (unsigned char *)&rx_aes_md5_pkt99_tmpl[0]
};
#endif /* CONFIG_BCM_SPU_TEST */

#endif /*__SPUDRV_RX_AES_MD5_DATA_H__*/
