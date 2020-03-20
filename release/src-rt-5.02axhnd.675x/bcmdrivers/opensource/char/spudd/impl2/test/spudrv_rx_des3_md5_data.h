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
#ifndef __SPUDRV_RX_DES3_MD5_DATA_H__
#define __SPUDRV_RX_DES3_MD5_DATA_H__

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

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt0_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 283 words. 

/// BDA size     is 1128 (0x468) 

/// BDA id       is 0xd8ea 

    0x0468d8ea,// 3 BDA   1 

/// PAY Generic Data size   : 1128 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xa17f1bd3,// 4 PAY   1 

    0x4399c7f4,// 5 PAY   2 

    0x68f9e1d9,// 6 PAY   3 

    0xd0e3cd46,// 7 PAY   4 

    0xa2670f40,// 8 PAY   5 

    0x4a36bb60,// 9 PAY   6 

    0xdff6bf3d,// 10 PAY   7 

    0xcf6d6aca,// 11 PAY   8 

    0x27892a36,// 12 PAY   9 

    0xb3a64259,// 13 PAY  10 

    0x0b53f96d,// 14 PAY  11 

    0x381f4d3b,// 15 PAY  12 

    0x6fa9032e,// 16 PAY  13 

    0x69c3e0af,// 17 PAY  14 

    0xfc2af810,// 18 PAY  15 

    0xcf77de57,// 19 PAY  16 

    0x5894007c,// 20 PAY  17 

    0x207eadff,// 21 PAY  18 

    0x0e978c80,// 22 PAY  19 

    0x71a365f1,// 23 PAY  20 

    0x5b16cb2d,// 24 PAY  21 

    0x9b11bea8,// 25 PAY  22 

    0xea54c92c,// 26 PAY  23 

    0xb272d3c2,// 27 PAY  24 

    0xc5914f4d,// 28 PAY  25 

    0xdb753ad5,// 29 PAY  26 

    0x49cce16e,// 30 PAY  27 

    0xa0f45151,// 31 PAY  28 

    0x1e695404,// 32 PAY  29 

    0x18b87c09,// 33 PAY  30 

    0xd9f4cb93,// 34 PAY  31 

    0x22ae2f7e,// 35 PAY  32 

    0x880f4509,// 36 PAY  33 

    0xc98cdaf8,// 37 PAY  34 

    0x278f1144,// 38 PAY  35 

    0x83ea1d13,// 39 PAY  36 

    0x0ef7d560,// 40 PAY  37 

    0x849b2a23,// 41 PAY  38 

    0xa7c4604d,// 42 PAY  39 

    0xb3df193f,// 43 PAY  40 

    0xb97873d5,// 44 PAY  41 

    0x73362c47,// 45 PAY  42 

    0xf3bb48be,// 46 PAY  43 

    0xd600bf75,// 47 PAY  44 

    0x43bfa6bf,// 48 PAY  45 

    0xddaf8b1f,// 49 PAY  46 

    0xee76ab64,// 50 PAY  47 

    0xbac8e108,// 51 PAY  48 

    0xee60b96a,// 52 PAY  49 

    0x069ce083,// 53 PAY  50 

    0xd68e55ca,// 54 PAY  51 

    0x5e323945,// 55 PAY  52 

    0xfe08e249,// 56 PAY  53 

    0xca01da9c,// 57 PAY  54 

    0x78665a7a,// 58 PAY  55 

    0x4012a1f8,// 59 PAY  56 

    0xe879490e,// 60 PAY  57 

    0xc80f232a,// 61 PAY  58 

    0x812ffcca,// 62 PAY  59 

    0x31ea207e,// 63 PAY  60 

    0x564ec321,// 64 PAY  61 

    0xaa138997,// 65 PAY  62 

    0x7914df11,// 66 PAY  63 

    0x482c7dfd,// 67 PAY  64 

    0xc86e4a85,// 68 PAY  65 

    0x847e2857,// 69 PAY  66 

    0xe8febf8a,// 70 PAY  67 

    0xba2b27b4,// 71 PAY  68 

    0x44b13297,// 72 PAY  69 

    0x1315f597,// 73 PAY  70 

    0xa194e3fd,// 74 PAY  71 

    0x3323ea74,// 75 PAY  72 

    0x05a53cd1,// 76 PAY  73 

    0xd177e43e,// 77 PAY  74 

    0x1dec6656,// 78 PAY  75 

    0x79231f39,// 79 PAY  76 

    0x1459af3a,// 80 PAY  77 

    0xca9ed55b,// 81 PAY  78 

    0x0c01f5d3,// 82 PAY  79 

    0x54f879ea,// 83 PAY  80 

    0x071807cc,// 84 PAY  81 

    0x77156d5b,// 85 PAY  82 

    0xb49e7ef9,// 86 PAY  83 

    0x1c865391,// 87 PAY  84 

    0xbcee3839,// 88 PAY  85 

    0xe347e15a,// 89 PAY  86 

    0xa8e32dfd,// 90 PAY  87 

    0x03c6eced,// 91 PAY  88 

    0x8a6e91cd,// 92 PAY  89 

    0x229eeb56,// 93 PAY  90 

    0xe2cebc4b,// 94 PAY  91 

    0x06577737,// 95 PAY  92 

    0x6f0e0ad5,// 96 PAY  93 

    0xea60e597,// 97 PAY  94 

    0x5db0cef4,// 98 PAY  95 

    0x25669c80,// 99 PAY  96 

    0xed9fc222,// 100 PAY  97 

    0x4889146d,// 101 PAY  98 

    0xe6eb5cea,// 102 PAY  99 

    0xf8873af4,// 103 PAY 100 

    0x88a06277,// 104 PAY 101 

    0x99179638,// 105 PAY 102 

    0xd3c53b0a,// 106 PAY 103 

    0xac70495e,// 107 PAY 104 

    0xf84a330f,// 108 PAY 105 

    0x3d92c0f6,// 109 PAY 106 

    0x8c1725c7,// 110 PAY 107 

    0x698de8a1,// 111 PAY 108 

    0xbc761121,// 112 PAY 109 

    0x0f6f3967,// 113 PAY 110 

    0x3477e130,// 114 PAY 111 

    0xf1bc44f4,// 115 PAY 112 

    0xc9148a59,// 116 PAY 113 

    0xf2617826,// 117 PAY 114 

    0xdfa38cb6,// 118 PAY 115 

    0x8a3c8b1e,// 119 PAY 116 

    0x580540f8,// 120 PAY 117 

    0xa296659e,// 121 PAY 118 

    0x264b1ff4,// 122 PAY 119 

    0x830720cb,// 123 PAY 120 

    0x2988b1a5,// 124 PAY 121 

    0xcd6391fc,// 125 PAY 122 

    0xec900fa8,// 126 PAY 123 

    0x99678e0e,// 127 PAY 124 

    0x823b91e2,// 128 PAY 125 

    0x0a2528e1,// 129 PAY 126 

    0xb8651657,// 130 PAY 127 

    0x7c997a30,// 131 PAY 128 

    0x459c4d41,// 132 PAY 129 

    0xa39e5831,// 133 PAY 130 

    0x450b9209,// 134 PAY 131 

    0xb601ebdd,// 135 PAY 132 

    0x799b2cdc,// 136 PAY 133 

    0xe2f4212d,// 137 PAY 134 

    0xa149fc47,// 138 PAY 135 

    0x14724c13,// 139 PAY 136 

    0x5ec61ea2,// 140 PAY 137 

    0xb7cd6f59,// 141 PAY 138 

    0xed362934,// 142 PAY 139 

    0x53ef5147,// 143 PAY 140 

    0xed91c8bb,// 144 PAY 141 

    0x4a89eccf,// 145 PAY 142 

    0x8fa46e46,// 146 PAY 143 

    0x92cc1422,// 147 PAY 144 

    0x2b5f879a,// 148 PAY 145 

    0xdf985b86,// 149 PAY 146 

    0x4acb677e,// 150 PAY 147 

    0xb837d302,// 151 PAY 148 

    0x25127680,// 152 PAY 149 

    0x8bac75a7,// 153 PAY 150 

    0x1ea216ec,// 154 PAY 151 

    0x67e49cba,// 155 PAY 152 

    0xeb6e2c29,// 156 PAY 153 

    0xcde50a6a,// 157 PAY 154 

    0x2d861745,// 158 PAY 155 

    0x7a58ae35,// 159 PAY 156 

    0xc179e27c,// 160 PAY 157 

    0xfdb204a8,// 161 PAY 158 

    0x02e2eeb7,// 162 PAY 159 

    0xf31597aa,// 163 PAY 160 

    0x9da858f2,// 164 PAY 161 

    0x71ed2752,// 165 PAY 162 

    0x9debb5af,// 166 PAY 163 

    0x5fea2ace,// 167 PAY 164 

    0xc009df95,// 168 PAY 165 

    0xf0eefec5,// 169 PAY 166 

    0x364d5218,// 170 PAY 167 

    0xef5760b9,// 171 PAY 168 

    0xf744f4ff,// 172 PAY 169 

    0x0229764d,// 173 PAY 170 

    0x44497ef3,// 174 PAY 171 

    0xbce84b43,// 175 PAY 172 

    0x4b0e6c80,// 176 PAY 173 

    0xe725f99c,// 177 PAY 174 

    0x06277ee9,// 178 PAY 175 

    0x5ef0c8d1,// 179 PAY 176 

    0xf6a28914,// 180 PAY 177 

    0x92df3cc9,// 181 PAY 178 

    0xd2dd86aa,// 182 PAY 179 

    0xd98f3c9b,// 183 PAY 180 

    0x623d8115,// 184 PAY 181 

    0xd771105e,// 185 PAY 182 

    0x316b7c0d,// 186 PAY 183 

    0x5a723c9a,// 187 PAY 184 

    0xf809aaee,// 188 PAY 185 

    0xee37de7c,// 189 PAY 186 

    0xd7860e4d,// 190 PAY 187 

    0xf599c19b,// 191 PAY 188 

    0xfddd8193,// 192 PAY 189 

    0x63213a75,// 193 PAY 190 

    0x19a85740,// 194 PAY 191 

    0x912303da,// 195 PAY 192 

    0x37d10e60,// 196 PAY 193 

    0xbe95e8ba,// 197 PAY 194 

    0x99fac630,// 198 PAY 195 

    0xf062b1d0,// 199 PAY 196 

    0x962c99c7,// 200 PAY 197 

    0xe0046e49,// 201 PAY 198 

    0x5e6c3d17,// 202 PAY 199 

    0xb67cd48a,// 203 PAY 200 

    0x852ba1fa,// 204 PAY 201 

    0x5498f280,// 205 PAY 202 

    0x3b4aa112,// 206 PAY 203 

    0xd662f212,// 207 PAY 204 

    0x21295705,// 208 PAY 205 

    0x5f808ed1,// 209 PAY 206 

    0xb0b4866c,// 210 PAY 207 

    0x33502c78,// 211 PAY 208 

    0x5890b735,// 212 PAY 209 

    0x43c15269,// 213 PAY 210 

    0xcfe91f83,// 214 PAY 211 

    0xb2a72985,// 215 PAY 212 

    0x587695ba,// 216 PAY 213 

    0xc1613ab8,// 217 PAY 214 

    0xe012ffc7,// 218 PAY 215 

    0xc48ae52e,// 219 PAY 216 

    0x92c9dfd7,// 220 PAY 217 

    0x9d1c2b5f,// 221 PAY 218 

    0x3034ba0d,// 222 PAY 219 

    0x38ff5dc6,// 223 PAY 220 

    0xec918a5d,// 224 PAY 221 

    0x3d7e5e79,// 225 PAY 222 

    0x0f66aaad,// 226 PAY 223 

    0xb239cf4d,// 227 PAY 224 

    0x65222c33,// 228 PAY 225 

    0xf9179ffa,// 229 PAY 226 

    0x079727b0,// 230 PAY 227 

    0xd393b27a,// 231 PAY 228 

    0xc7a6da5b,// 232 PAY 229 

    0x8699b138,// 233 PAY 230 

    0xcfed8ed7,// 234 PAY 231 

    0xfe8ca173,// 235 PAY 232 

    0x552d3041,// 236 PAY 233 

    0x64470654,// 237 PAY 234 

    0x3a0304e6,// 238 PAY 235 

    0xdb3f9cb3,// 239 PAY 236 

    0x31c3f22f,// 240 PAY 237 

    0x7982e92e,// 241 PAY 238 

    0xdae2da84,// 242 PAY 239 

    0x2fdd2eb0,// 243 PAY 240 

    0x749ba6fb,// 244 PAY 241 

    0xdc7214e6,// 245 PAY 242 

    0xa9432337,// 246 PAY 243 

    0x7791f600,// 247 PAY 244 

    0x1646db5d,// 248 PAY 245 

    0x4ca7e085,// 249 PAY 246 

    0x0674aa7d,// 250 PAY 247 

    0x29924ee0,// 251 PAY 248 

    0xb50a76f7,// 252 PAY 249 

    0x12f32c04,// 253 PAY 250 

    0x2901780e,// 254 PAY 251 

    0x21f0a9d7,// 255 PAY 252 

    0x8b36b44d,// 256 PAY 253 

    0xab84147f,// 257 PAY 254 

    0xc8db7f30,// 258 PAY 255 

    0xe5ea70db,// 259 PAY 256 

    0xb1eb8d03,// 260 PAY 257 

    0x7b5ac303,// 261 PAY 258 

    0x62855011,// 262 PAY 259 

    0xac7097b4,// 263 PAY 260 

    0x9f8aacbd,// 264 PAY 261 

    0x1b250a5e,// 265 PAY 262 

    0x39dee7d4,// 266 PAY 263 

    0xe714325c,// 267 PAY 264 

    0xdffe2dfe,// 268 PAY 265 

    0x549c8da7,// 269 PAY 266 

    0xf049e54b,// 270 PAY 267 

    0xb19e1124,// 271 PAY 268 

    0x43408369,// 272 PAY 269 

    0x90478f20,// 273 PAY 270 

    0x80c9d880,// 274 PAY 271 

    0xeaea2d44,// 275 PAY 272 

    0xc2d0950d,// 276 PAY 273 

    0x2852e5db,// 277 PAY 274 

    0x731d6b7a,// 278 PAY 275 

    0xba32b5d0,// 279 PAY 276 

    0x704802b8,// 280 PAY 277 

    0x3dec0e38,// 281 PAY 278 

    0x3250893d,// 282 PAY 279 

    0x8aef595f,// 283 PAY 280 

    0xf34c4a86,// 284 PAY 281 

    0x352a350f,// 285 PAY 282 

/// HASH is  12 bytes 

    0x852ba1fa,// 286 HSH   1 

    0x5498f280,// 287 HSH   2 

    0x3b4aa112,// 288 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 99 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x93 

    0x02609363 // 289 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt1_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 513 words. 

/// BDA size     is 2048 (0x800) 

/// BDA id       is 0x3d82 

    0x08003d82,// 3 BDA   1 

/// PAY Generic Data size   : 2048 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x2dc061cf,// 4 PAY   1 

    0x161cc825,// 5 PAY   2 

    0x202fd253,// 6 PAY   3 

    0x13a06bf3,// 7 PAY   4 

    0xe08a9e7c,// 8 PAY   5 

    0xee61bf5d,// 9 PAY   6 

    0x48b5d489,// 10 PAY   7 

    0xafb39e1e,// 11 PAY   8 

    0x3ee1e750,// 12 PAY   9 

    0xefcdd2f3,// 13 PAY  10 

    0x4c2b2559,// 14 PAY  11 

    0xbd12689c,// 15 PAY  12 

    0xb133907a,// 16 PAY  13 

    0x2ed8fa4a,// 17 PAY  14 

    0x4c32d31a,// 18 PAY  15 

    0x5eacf37c,// 19 PAY  16 

    0x3ac8c7fe,// 20 PAY  17 

    0xd5e5fe1f,// 21 PAY  18 

    0x02de59f3,// 22 PAY  19 

    0xc2be1446,// 23 PAY  20 

    0x62a03e3e,// 24 PAY  21 

    0x5752b5d7,// 25 PAY  22 

    0xe776d48e,// 26 PAY  23 

    0x66101e00,// 27 PAY  24 

    0x6bf03436,// 28 PAY  25 

    0xe6153322,// 29 PAY  26 

    0xbc3fec5c,// 30 PAY  27 

    0x27acd3ca,// 31 PAY  28 

    0xdc5c10fd,// 32 PAY  29 

    0xd6f2f64b,// 33 PAY  30 

    0xaf7b945c,// 34 PAY  31 

    0xcb75a728,// 35 PAY  32 

    0x498b09cf,// 36 PAY  33 

    0xeb6091d3,// 37 PAY  34 

    0x01590053,// 38 PAY  35 

    0x2f243054,// 39 PAY  36 

    0x5e58f7a7,// 40 PAY  37 

    0x80d56156,// 41 PAY  38 

    0x0cda6e64,// 42 PAY  39 

    0xa08e6b08,// 43 PAY  40 

    0xeeb9b3d4,// 44 PAY  41 

    0x12aee6b9,// 45 PAY  42 

    0x8d6995c5,// 46 PAY  43 

    0x427b4a0b,// 47 PAY  44 

    0x8e7c6181,// 48 PAY  45 

    0xa19e15cd,// 49 PAY  46 

    0x8737ca7d,// 50 PAY  47 

    0xd3b29dd0,// 51 PAY  48 

    0x1a8d764d,// 52 PAY  49 

    0xe7d99b5d,// 53 PAY  50 

    0x25bcd779,// 54 PAY  51 

    0x2ef0987e,// 55 PAY  52 

    0xa2f522c0,// 56 PAY  53 

    0x58ed06c6,// 57 PAY  54 

    0x7f5b7823,// 58 PAY  55 

    0xc7d5e20f,// 59 PAY  56 

    0x73bc6a1f,// 60 PAY  57 

    0xc1624ca1,// 61 PAY  58 

    0xbd14b566,// 62 PAY  59 

    0xdd08f45b,// 63 PAY  60 

    0x09563f3a,// 64 PAY  61 

    0xdb126204,// 65 PAY  62 

    0xf2ce71a5,// 66 PAY  63 

    0x3acb73d3,// 67 PAY  64 

    0x27139c03,// 68 PAY  65 

    0x9020481c,// 69 PAY  66 

    0xb7be6853,// 70 PAY  67 

    0x3b8f9051,// 71 PAY  68 

    0x7f89957f,// 72 PAY  69 

    0xb36c5cc6,// 73 PAY  70 

    0x17abdf59,// 74 PAY  71 

    0x9380073a,// 75 PAY  72 

    0xf43e7c7c,// 76 PAY  73 

    0xebc9bca2,// 77 PAY  74 

    0xaec58776,// 78 PAY  75 

    0xe41f7da5,// 79 PAY  76 

    0xa9d092df,// 80 PAY  77 

    0x91dd007a,// 81 PAY  78 

    0x13b63592,// 82 PAY  79 

    0xddb00260,// 83 PAY  80 

    0x5f21fb04,// 84 PAY  81 

    0xb77c9bd6,// 85 PAY  82 

    0x89cd264e,// 86 PAY  83 

    0xe41a2708,// 87 PAY  84 

    0x3e58c5f9,// 88 PAY  85 

    0x6b0632d8,// 89 PAY  86 

    0x9ac72d54,// 90 PAY  87 

    0x5d3eedbe,// 91 PAY  88 

    0x1e23ef90,// 92 PAY  89 

    0x38685cee,// 93 PAY  90 

    0xb6e7a3f1,// 94 PAY  91 

    0x420dc260,// 95 PAY  92 

    0x146676f0,// 96 PAY  93 

    0x4677f295,// 97 PAY  94 

    0x635a0cbf,// 98 PAY  95 

    0x1df5cd00,// 99 PAY  96 

    0x7cc72a5e,// 100 PAY  97 

    0x6afc9c85,// 101 PAY  98 

    0xd5269420,// 102 PAY  99 

    0x87fb3b52,// 103 PAY 100 

    0x8656fa3d,// 104 PAY 101 

    0xf1b4f0fe,// 105 PAY 102 

    0xf845c876,// 106 PAY 103 

    0xbbe6bb88,// 107 PAY 104 

    0x079455c8,// 108 PAY 105 

    0xa94e2379,// 109 PAY 106 

    0x778e84ed,// 110 PAY 107 

    0x67d5e6e7,// 111 PAY 108 

    0xa1b6c4dc,// 112 PAY 109 

    0x8b4f790b,// 113 PAY 110 

    0xd60318e9,// 114 PAY 111 

    0x1ed4988d,// 115 PAY 112 

    0x2658be98,// 116 PAY 113 

    0x90fb3424,// 117 PAY 114 

    0x80f6139b,// 118 PAY 115 

    0xd570c21c,// 119 PAY 116 

    0x11075456,// 120 PAY 117 

    0x1ccf85b9,// 121 PAY 118 

    0xd6b2dbd0,// 122 PAY 119 

    0x636e3f2e,// 123 PAY 120 

    0x109999e5,// 124 PAY 121 

    0xa7c5cd77,// 125 PAY 122 

    0x367fa4c5,// 126 PAY 123 

    0x18610de9,// 127 PAY 124 

    0xa5156386,// 128 PAY 125 

    0x9bc9cb03,// 129 PAY 126 

    0xa74c0eb7,// 130 PAY 127 

    0x5dbf7b61,// 131 PAY 128 

    0x14eeb6e5,// 132 PAY 129 

    0x5220e71b,// 133 PAY 130 

    0xc046928f,// 134 PAY 131 

    0x0b8eec14,// 135 PAY 132 

    0x4d43656b,// 136 PAY 133 

    0x822bf32e,// 137 PAY 134 

    0xab9ffee4,// 138 PAY 135 

    0x22884cd9,// 139 PAY 136 

    0xf1175dc2,// 140 PAY 137 

    0xaa033508,// 141 PAY 138 

    0xa2e3adc0,// 142 PAY 139 

    0xf4eae98e,// 143 PAY 140 

    0xcedf0b6d,// 144 PAY 141 

    0x84f4ab70,// 145 PAY 142 

    0x759e319e,// 146 PAY 143 

    0x048a85d6,// 147 PAY 144 

    0x9d740559,// 148 PAY 145 

    0xb8bb10a2,// 149 PAY 146 

    0x345d2c8b,// 150 PAY 147 

    0xf45dbcf3,// 151 PAY 148 

    0xfe1b39ea,// 152 PAY 149 

    0x0aa150f1,// 153 PAY 150 

    0xef3a8237,// 154 PAY 151 

    0x139e94dd,// 155 PAY 152 

    0xf4adb0c4,// 156 PAY 153 

    0x43e5bf3d,// 157 PAY 154 

    0x16ad342d,// 158 PAY 155 

    0xc4020262,// 159 PAY 156 

    0x0a38dfc5,// 160 PAY 157 

    0x3331fdc1,// 161 PAY 158 

    0xc53fbeea,// 162 PAY 159 

    0x00449d3b,// 163 PAY 160 

    0x5cad81ee,// 164 PAY 161 

    0xa95967e2,// 165 PAY 162 

    0xca7af5cc,// 166 PAY 163 

    0xafe4425c,// 167 PAY 164 

    0x789be653,// 168 PAY 165 

    0xb7f967c4,// 169 PAY 166 

    0x87f5a618,// 170 PAY 167 

    0xccd19fa0,// 171 PAY 168 

    0xde4faa12,// 172 PAY 169 

    0xf69e0fbe,// 173 PAY 170 

    0x3ad6acaf,// 174 PAY 171 

    0x72d0895e,// 175 PAY 172 

    0x0680000d,// 176 PAY 173 

    0x3885f71e,// 177 PAY 174 

    0x8de0a0cb,// 178 PAY 175 

    0x7252ae8c,// 179 PAY 176 

    0x41cc4b24,// 180 PAY 177 

    0x7c014ce2,// 181 PAY 178 

    0x8e7ee4b0,// 182 PAY 179 

    0xc9a8c451,// 183 PAY 180 

    0x08963846,// 184 PAY 181 

    0xeb14dd13,// 185 PAY 182 

    0x913e0584,// 186 PAY 183 

    0x96e4949e,// 187 PAY 184 

    0x46e71c46,// 188 PAY 185 

    0x10978564,// 189 PAY 186 

    0xb071c4d0,// 190 PAY 187 

    0x72598d74,// 191 PAY 188 

    0x512c4d7b,// 192 PAY 189 

    0xeba8788c,// 193 PAY 190 

    0xaf29a03f,// 194 PAY 191 

    0x108a9be6,// 195 PAY 192 

    0xc79b8e15,// 196 PAY 193 

    0x85d4bb5a,// 197 PAY 194 

    0x96ee3bc5,// 198 PAY 195 

    0x8e77f3c5,// 199 PAY 196 

    0xa556738a,// 200 PAY 197 

    0xb0979aee,// 201 PAY 198 

    0x5ab97c60,// 202 PAY 199 

    0x891b853d,// 203 PAY 200 

    0x14bd8441,// 204 PAY 201 

    0x09264a05,// 205 PAY 202 

    0xf6130093,// 206 PAY 203 

    0x857c55dd,// 207 PAY 204 

    0x44bd731a,// 208 PAY 205 

    0xf6f43e93,// 209 PAY 206 

    0xf9503303,// 210 PAY 207 

    0xdf1874ce,// 211 PAY 208 

    0x3f383f4a,// 212 PAY 209 

    0xc8cc76c4,// 213 PAY 210 

    0x8822e936,// 214 PAY 211 

    0x8c75f220,// 215 PAY 212 

    0x755d9ba2,// 216 PAY 213 

    0x84e31e8f,// 217 PAY 214 

    0xebb0259a,// 218 PAY 215 

    0x6c1d8ecd,// 219 PAY 216 

    0xa5b5373d,// 220 PAY 217 

    0xea5332e9,// 221 PAY 218 

    0xae1c56ac,// 222 PAY 219 

    0x52a8cf31,// 223 PAY 220 

    0x1966b0aa,// 224 PAY 221 

    0xd3bae545,// 225 PAY 222 

    0x2a0bb58c,// 226 PAY 223 

    0x2d56cc30,// 227 PAY 224 

    0x3e2ce6a6,// 228 PAY 225 

    0xc2b162a1,// 229 PAY 226 

    0x35efa0ed,// 230 PAY 227 

    0x66c3522e,// 231 PAY 228 

    0x3e05e25c,// 232 PAY 229 

    0xe3cb2f74,// 233 PAY 230 

    0x8bd237ae,// 234 PAY 231 

    0x06909d12,// 235 PAY 232 

    0x55ef8bef,// 236 PAY 233 

    0xf62e6ad8,// 237 PAY 234 

    0x92cd1e45,// 238 PAY 235 

    0x9cdf74e9,// 239 PAY 236 

    0x0545d6c4,// 240 PAY 237 

    0xebd94ee7,// 241 PAY 238 

    0xcf9c67cc,// 242 PAY 239 

    0x94a7ad40,// 243 PAY 240 

    0x14b31e41,// 244 PAY 241 

    0x9fea8d01,// 245 PAY 242 

    0xeebf4661,// 246 PAY 243 

    0x5a4d4d43,// 247 PAY 244 

    0xd71755e1,// 248 PAY 245 

    0xa86bbd72,// 249 PAY 246 

    0x7c052899,// 250 PAY 247 

    0x7dd36638,// 251 PAY 248 

    0x65eb029f,// 252 PAY 249 

    0xeb984551,// 253 PAY 250 

    0xaa6e194c,// 254 PAY 251 

    0x2063d5ec,// 255 PAY 252 

    0x8ba1050f,// 256 PAY 253 

    0xaccdfad5,// 257 PAY 254 

    0xfde89d19,// 258 PAY 255 

    0x5c09333d,// 259 PAY 256 

    0x1bd5434c,// 260 PAY 257 

    0x67837735,// 261 PAY 258 

    0x9b7b3a77,// 262 PAY 259 

    0x43c8eab1,// 263 PAY 260 

    0xa016faee,// 264 PAY 261 

    0x1e839ed7,// 265 PAY 262 

    0xf0de6cec,// 266 PAY 263 

    0xb0790eda,// 267 PAY 264 

    0x7bab6864,// 268 PAY 265 

    0xbf6a1b96,// 269 PAY 266 

    0x9e384623,// 270 PAY 267 

    0x41ba9e34,// 271 PAY 268 

    0x5193daeb,// 272 PAY 269 

    0x34b7a914,// 273 PAY 270 

    0xfbf6fc45,// 274 PAY 271 

    0x7b2ddcae,// 275 PAY 272 

    0xc82a7049,// 276 PAY 273 

    0x08992e0b,// 277 PAY 274 

    0x255b6ec5,// 278 PAY 275 

    0xbf8dd036,// 279 PAY 276 

    0x97f4c2e9,// 280 PAY 277 

    0x39896092,// 281 PAY 278 

    0xae269984,// 282 PAY 279 

    0xf0fc2fa0,// 283 PAY 280 

    0x0e71d1e9,// 284 PAY 281 

    0xc83d1d7a,// 285 PAY 282 

    0x8d9f561d,// 286 PAY 283 

    0x63d322c8,// 287 PAY 284 

    0xb6046df4,// 288 PAY 285 

    0x76772e50,// 289 PAY 286 

    0xdd9bdda5,// 290 PAY 287 

    0xff8b649a,// 291 PAY 288 

    0x68a5c97d,// 292 PAY 289 

    0xca0b5aff,// 293 PAY 290 

    0x7f839f26,// 294 PAY 291 

    0x976a3076,// 295 PAY 292 

    0xdec78fef,// 296 PAY 293 

    0x599a968d,// 297 PAY 294 

    0xfdb1da9a,// 298 PAY 295 

    0xaf933999,// 299 PAY 296 

    0x6dfc7b0c,// 300 PAY 297 

    0x5cc13f32,// 301 PAY 298 

    0x532b6687,// 302 PAY 299 

    0xc8e2d140,// 303 PAY 300 

    0xf34cb575,// 304 PAY 301 

    0xed9cd5a8,// 305 PAY 302 

    0xc884c909,// 306 PAY 303 

    0xa88a68a3,// 307 PAY 304 

    0xd654a989,// 308 PAY 305 

    0x4a52d766,// 309 PAY 306 

    0xa30bb6f2,// 310 PAY 307 

    0x8bf251b9,// 311 PAY 308 

    0x676090e7,// 312 PAY 309 

    0xc55cad42,// 313 PAY 310 

    0x31192cd9,// 314 PAY 311 

    0xe26bb685,// 315 PAY 312 

    0x0d7661d6,// 316 PAY 313 

    0x20c16e7b,// 317 PAY 314 

    0x4edc5734,// 318 PAY 315 

    0x41724d04,// 319 PAY 316 

    0x78286bc0,// 320 PAY 317 

    0x98e7ecd5,// 321 PAY 318 

    0x2ab66444,// 322 PAY 319 

    0x2b51eb55,// 323 PAY 320 

    0x45c07699,// 324 PAY 321 

    0xa726dc81,// 325 PAY 322 

    0xede5aaa7,// 326 PAY 323 

    0xcb0bfee2,// 327 PAY 324 

    0xcf9b26bc,// 328 PAY 325 

    0x2837471d,// 329 PAY 326 

    0xfa1d36d2,// 330 PAY 327 

    0x2c0bdea5,// 331 PAY 328 

    0x0b30b016,// 332 PAY 329 

    0x6f2df8fb,// 333 PAY 330 

    0x0a8cbb41,// 334 PAY 331 

    0xfc45db02,// 335 PAY 332 

    0xf9666396,// 336 PAY 333 

    0xb7a7f77a,// 337 PAY 334 

    0x0c6538b7,// 338 PAY 335 

    0x7e0cac7d,// 339 PAY 336 

    0x2929e96f,// 340 PAY 337 

    0x7f0949eb,// 341 PAY 338 

    0xf04eade2,// 342 PAY 339 

    0x022086e8,// 343 PAY 340 

    0x90d85f1c,// 344 PAY 341 

    0x24374a50,// 345 PAY 342 

    0x356f5981,// 346 PAY 343 

    0xf554b8a7,// 347 PAY 344 

    0x46c96cfe,// 348 PAY 345 

    0x935007aa,// 349 PAY 346 

    0xd8ec7cc0,// 350 PAY 347 

    0x2c5fe93c,// 351 PAY 348 

    0x08674e12,// 352 PAY 349 

    0x2d56c13b,// 353 PAY 350 

    0x2c70e819,// 354 PAY 351 

    0xa59109d6,// 355 PAY 352 

    0x4f3881e2,// 356 PAY 353 

    0x58556003,// 357 PAY 354 

    0x8e256a1c,// 358 PAY 355 

    0x7ef2d537,// 359 PAY 356 

    0x189a2115,// 360 PAY 357 

    0xdda8eb55,// 361 PAY 358 

    0x2fcf3c6b,// 362 PAY 359 

    0xec36d027,// 363 PAY 360 

    0xb782f954,// 364 PAY 361 

    0x2110bef9,// 365 PAY 362 

    0x407a319c,// 366 PAY 363 

    0x892bd11b,// 367 PAY 364 

    0x148111b7,// 368 PAY 365 

    0x6615fed8,// 369 PAY 366 

    0x240a4d5c,// 370 PAY 367 

    0x6bd6f070,// 371 PAY 368 

    0xc82c9681,// 372 PAY 369 

    0xf0c02d24,// 373 PAY 370 

    0xcf2b043c,// 374 PAY 371 

    0x7c291bca,// 375 PAY 372 

    0x70902aa3,// 376 PAY 373 

    0x4bdfba8b,// 377 PAY 374 

    0xba4f3466,// 378 PAY 375 

    0x25e14198,// 379 PAY 376 

    0x08ac83b4,// 380 PAY 377 

    0x501919eb,// 381 PAY 378 

    0x43d316c1,// 382 PAY 379 

    0x46bc561b,// 383 PAY 380 

    0x342b0083,// 384 PAY 381 

    0x2e96fb8a,// 385 PAY 382 

    0x5175717e,// 386 PAY 383 

    0xf3dc18d1,// 387 PAY 384 

    0x1be50951,// 388 PAY 385 

    0x1229dade,// 389 PAY 386 

    0x1e7ca45d,// 390 PAY 387 

    0x3d545c1a,// 391 PAY 388 

    0x59b1a168,// 392 PAY 389 

    0x9cb4233b,// 393 PAY 390 

    0xf2d663ed,// 394 PAY 391 

    0xbf5bfba8,// 395 PAY 392 

    0x6602fea5,// 396 PAY 393 

    0x3956d462,// 397 PAY 394 

    0x3b8c8b08,// 398 PAY 395 

    0xb5a97740,// 399 PAY 396 

    0x6f93e51d,// 400 PAY 397 

    0x7edf1050,// 401 PAY 398 

    0xdb227230,// 402 PAY 399 

    0x4c8be2f1,// 403 PAY 400 

    0x10bbcb0b,// 404 PAY 401 

    0xe01493da,// 405 PAY 402 

    0xf9eab87b,// 406 PAY 403 

    0xfc5f723e,// 407 PAY 404 

    0x859b6b72,// 408 PAY 405 

    0x8bbc280a,// 409 PAY 406 

    0xde517c9a,// 410 PAY 407 

    0x52ec49a2,// 411 PAY 408 

    0x5a9e850f,// 412 PAY 409 

    0x33f12ded,// 413 PAY 410 

    0x73b5f9ab,// 414 PAY 411 

    0xedd58bf4,// 415 PAY 412 

    0x46cea42c,// 416 PAY 413 

    0x1fc125a8,// 417 PAY 414 

    0x49815bbb,// 418 PAY 415 

    0x94b743f2,// 419 PAY 416 

    0x8df38617,// 420 PAY 417 

    0xcae2779c,// 421 PAY 418 

    0x042c027c,// 422 PAY 419 

    0xe365429b,// 423 PAY 420 

    0xe1289c9e,// 424 PAY 421 

    0x49756fdc,// 425 PAY 422 

    0x8891575a,// 426 PAY 423 

    0x9b39e267,// 427 PAY 424 

    0xe582d449,// 428 PAY 425 

    0x2ee9a30a,// 429 PAY 426 

    0x1df41f1e,// 430 PAY 427 

    0xf35ebfa5,// 431 PAY 428 

    0x559e28e1,// 432 PAY 429 

    0x6bb624ce,// 433 PAY 430 

    0x19eeffd2,// 434 PAY 431 

    0x199007cb,// 435 PAY 432 

    0xd5ec429a,// 436 PAY 433 

    0x3cd48f6a,// 437 PAY 434 

    0x2decdf5b,// 438 PAY 435 

    0x685b87de,// 439 PAY 436 

    0x9fe182fe,// 440 PAY 437 

    0x3002001f,// 441 PAY 438 

    0xf3a5c718,// 442 PAY 439 

    0xb5df740d,// 443 PAY 440 

    0x2bc88c10,// 444 PAY 441 

    0xca4f8a4c,// 445 PAY 442 

    0xda41e16f,// 446 PAY 443 

    0xb23aab28,// 447 PAY 444 

    0x81d8df26,// 448 PAY 445 

    0xd1ceba6b,// 449 PAY 446 

    0x99f067d8,// 450 PAY 447 

    0x01764ffd,// 451 PAY 448 

    0x6f480884,// 452 PAY 449 

    0x4072e477,// 453 PAY 450 

    0xd365d1ce,// 454 PAY 451 

    0x729be759,// 455 PAY 452 

    0x4d13e588,// 456 PAY 453 

    0xb38bdb1d,// 457 PAY 454 

    0x51936a14,// 458 PAY 455 

    0x1d8ec9f0,// 459 PAY 456 

    0x7adaf360,// 460 PAY 457 

    0xff9b42b7,// 461 PAY 458 

    0xa8733dbe,// 462 PAY 459 

    0x91158038,// 463 PAY 460 

    0x8e3141ba,// 464 PAY 461 

    0x948ccdee,// 465 PAY 462 

    0xebcdf133,// 466 PAY 463 

    0xeab765d3,// 467 PAY 464 

    0xafec0890,// 468 PAY 465 

    0x4ca2e265,// 469 PAY 466 

    0x1d1f3c0b,// 470 PAY 467 

    0x330d7a5b,// 471 PAY 468 

    0xcc02efa7,// 472 PAY 469 

    0xa2eb13a5,// 473 PAY 470 

    0x184a4246,// 474 PAY 471 

    0x77f3d7ff,// 475 PAY 472 

    0x66be2e12,// 476 PAY 473 

    0x14d12842,// 477 PAY 474 

    0x4067c40a,// 478 PAY 475 

    0xd6864edb,// 479 PAY 476 

    0xc7feea40,// 480 PAY 477 

    0x444c3a30,// 481 PAY 478 

    0x6a50223c,// 482 PAY 479 

    0x5b650f53,// 483 PAY 480 

    0xca48b31c,// 484 PAY 481 

    0x3a0cf956,// 485 PAY 482 

    0xe4f7fc91,// 486 PAY 483 

    0x5dbe26b1,// 487 PAY 484 

    0x806d387c,// 488 PAY 485 

    0xe920be2c,// 489 PAY 486 

    0xbc0895d4,// 490 PAY 487 

    0xdf4151e3,// 491 PAY 488 

    0xe93f047e,// 492 PAY 489 

    0x7a32f3a7,// 493 PAY 490 

    0x02db7639,// 494 PAY 491 

    0x29079420,// 495 PAY 492 

    0x7bf7068e,// 496 PAY 493 

    0x5ab89bb3,// 497 PAY 494 

    0xaadde0ec,// 498 PAY 495 

    0xfd99d983,// 499 PAY 496 

    0xb8f2eefa,// 500 PAY 497 

    0x870769c4,// 501 PAY 498 

    0x67f1309e,// 502 PAY 499 

    0x9c3747e2,// 503 PAY 500 

    0x01e479b3,// 504 PAY 501 

    0x51926bb5,// 505 PAY 502 

    0x06a37492,// 506 PAY 503 

    0x0d33f14b,// 507 PAY 504 

    0x56789730,// 508 PAY 505 

    0xc04726ea,// 509 PAY 506 

    0x37fd10e4,// 510 PAY 507 

    0x82483aec,// 511 PAY 508 

    0x8ea946f3,// 512 PAY 509 

    0xcbc0bb9d,// 513 PAY 510 

    0x486944f1,// 514 PAY 511 

    0x3eca9793,// 515 PAY 512 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x018880b2 // 516 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt2_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 144 words. 

/// BDA size     is 571 (0x23b) 

/// BDA id       is 0xb49d 

    0x023bb49d,// 3 BDA   1 

/// PAY Generic Data size   : 571 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf39d08a2,// 4 PAY   1 

    0x6953ace5,// 5 PAY   2 

    0x4dcae1f0,// 6 PAY   3 

    0xb54a73ea,// 7 PAY   4 

    0xd946cbef,// 8 PAY   5 

    0x54df7262,// 9 PAY   6 

    0xe206e12f,// 10 PAY   7 

    0xb20c91d8,// 11 PAY   8 

    0xacf8677b,// 12 PAY   9 

    0xb0bc6f7b,// 13 PAY  10 

    0x9382900f,// 14 PAY  11 

    0x33c799b6,// 15 PAY  12 

    0x236287d2,// 16 PAY  13 

    0x3ce3bd8d,// 17 PAY  14 

    0xb423d69f,// 18 PAY  15 

    0x1267336d,// 19 PAY  16 

    0x46e2f1dc,// 20 PAY  17 

    0xabd0a91b,// 21 PAY  18 

    0x2590939a,// 22 PAY  19 

    0x71cad0eb,// 23 PAY  20 

    0xad383cd9,// 24 PAY  21 

    0x090ab014,// 25 PAY  22 

    0xec767be2,// 26 PAY  23 

    0xeeeb0a9d,// 27 PAY  24 

    0x5ac76ac5,// 28 PAY  25 

    0x21cd55d3,// 29 PAY  26 

    0x22833a4b,// 30 PAY  27 

    0x4b40debb,// 31 PAY  28 

    0xf578efe1,// 32 PAY  29 

    0xfdd23394,// 33 PAY  30 

    0x7259399f,// 34 PAY  31 

    0x2f6da4bb,// 35 PAY  32 

    0x034c409e,// 36 PAY  33 

    0x582d5428,// 37 PAY  34 

    0x6633f8c9,// 38 PAY  35 

    0xc9dd8e55,// 39 PAY  36 

    0x6ac16872,// 40 PAY  37 

    0x99084b0f,// 41 PAY  38 

    0x7546f737,// 42 PAY  39 

    0xa93c2f9b,// 43 PAY  40 

    0xea434906,// 44 PAY  41 

    0x94358f0d,// 45 PAY  42 

    0xdf6e9638,// 46 PAY  43 

    0x02e97b7f,// 47 PAY  44 

    0x20b1df26,// 48 PAY  45 

    0x222888f9,// 49 PAY  46 

    0x1c92a4c0,// 50 PAY  47 

    0x8af098a7,// 51 PAY  48 

    0xa726f426,// 52 PAY  49 

    0x4d81caa2,// 53 PAY  50 

    0x7897b32c,// 54 PAY  51 

    0x08aa347f,// 55 PAY  52 

    0xa621c4a1,// 56 PAY  53 

    0x087fde89,// 57 PAY  54 

    0x3e93e17e,// 58 PAY  55 

    0x34dd6a74,// 59 PAY  56 

    0xd5a1a999,// 60 PAY  57 

    0x87243bf5,// 61 PAY  58 

    0x399cd7cb,// 62 PAY  59 

    0x40610c1b,// 63 PAY  60 

    0x42ab9ffa,// 64 PAY  61 

    0xd3c639bc,// 65 PAY  62 

    0xa65842d9,// 66 PAY  63 

    0x76fb97a9,// 67 PAY  64 

    0xd5184464,// 68 PAY  65 

    0x39c6be8e,// 69 PAY  66 

    0x26186106,// 70 PAY  67 

    0xae693102,// 71 PAY  68 

    0xf08e0d79,// 72 PAY  69 

    0x1de7702d,// 73 PAY  70 

    0x96170c04,// 74 PAY  71 

    0xbc2f9644,// 75 PAY  72 

    0xe07819f7,// 76 PAY  73 

    0xd40428b7,// 77 PAY  74 

    0x286ab3b4,// 78 PAY  75 

    0x231f5937,// 79 PAY  76 

    0xda47bf40,// 80 PAY  77 

    0x64686114,// 81 PAY  78 

    0x2897d627,// 82 PAY  79 

    0x95d78651,// 83 PAY  80 

    0xa7ad8244,// 84 PAY  81 

    0x397e0e44,// 85 PAY  82 

    0x3f2f1bcc,// 86 PAY  83 

    0x8731fcad,// 87 PAY  84 

    0x9c7dcbb5,// 88 PAY  85 

    0x9a90c55c,// 89 PAY  86 

    0x7d40e723,// 90 PAY  87 

    0x84501466,// 91 PAY  88 

    0x15145f99,// 92 PAY  89 

    0xd96005b0,// 93 PAY  90 

    0xff6fe2ac,// 94 PAY  91 

    0x95d90456,// 95 PAY  92 

    0x783005c6,// 96 PAY  93 

    0xc7533985,// 97 PAY  94 

    0x191d543a,// 98 PAY  95 

    0x1b409860,// 99 PAY  96 

    0x5601a72e,// 100 PAY  97 

    0x4ebd7b2e,// 101 PAY  98 

    0x82f0044a,// 102 PAY  99 

    0x84176321,// 103 PAY 100 

    0x2aa7298f,// 104 PAY 101 

    0xcc6913d8,// 105 PAY 102 

    0xefc72c54,// 106 PAY 103 

    0x79af1a98,// 107 PAY 104 

    0x18bb4b1f,// 108 PAY 105 

    0x612bb8bb,// 109 PAY 106 

    0xfc31b5dc,// 110 PAY 107 

    0x39312b0f,// 111 PAY 108 

    0xfc06f893,// 112 PAY 109 

    0x63f93f69,// 113 PAY 110 

    0x62931922,// 114 PAY 111 

    0xeb057a7f,// 115 PAY 112 

    0x9bf40680,// 116 PAY 113 

    0xc4d98aad,// 117 PAY 114 

    0xc6b920ea,// 118 PAY 115 

    0x2c1f6cf6,// 119 PAY 116 

    0x7166525e,// 120 PAY 117 

    0xd1067da3,// 121 PAY 118 

    0xa3ca216d,// 122 PAY 119 

    0x4212dfd1,// 123 PAY 120 

    0x539ffe62,// 124 PAY 121 

    0xbde82535,// 125 PAY 122 

    0x9f8144e4,// 126 PAY 123 

    0x834ac4b0,// 127 PAY 124 

    0x65b9d5ce,// 128 PAY 125 

    0xf8c4c718,// 129 PAY 126 

    0xd044d0fe,// 130 PAY 127 

    0x62e48565,// 131 PAY 128 

    0x80e5d09e,// 132 PAY 129 

    0x4ba985c9,// 133 PAY 130 

    0x96ad0613,// 134 PAY 131 

    0x291c6b26,// 135 PAY 132 

    0x7866055b,// 136 PAY 133 

    0x7fb216ca,// 137 PAY 134 

    0xe9d07e3b,// 138 PAY 135 

    0x6fdc40b1,// 139 PAY 136 

    0xa55d67f3,// 140 PAY 137 

    0x0b9cc94e,// 141 PAY 138 

    0x424deecf,// 142 PAY 139 

    0xafd36b37,// 143 PAY 140 

    0xf2a582be,// 144 PAY 141 

    0xa0604b7c,// 145 PAY 142 

    0x567a4d00,// 146 PAY 143 

/// STA is 1 words. 

/// STA num_pkts       : 74 

/// STA pkt_idx        : 189 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe9 

    0x02f5e94a // 147 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt3_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 368 words. 

/// BDA size     is 1465 (0x5b9) 

/// BDA id       is 0x61e 

    0x05b9061e,// 3 BDA   1 

/// PAY Generic Data size   : 1465 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x3dcf6fa7,// 4 PAY   1 

    0x62ab18fe,// 5 PAY   2 

    0x80220751,// 6 PAY   3 

    0x5e07eee5,// 7 PAY   4 

    0x500508e9,// 8 PAY   5 

    0x91241e0c,// 9 PAY   6 

    0xb1dd6761,// 10 PAY   7 

    0xa71ba8ed,// 11 PAY   8 

    0xd2853a87,// 12 PAY   9 

    0xf225b47f,// 13 PAY  10 

    0x2911bbde,// 14 PAY  11 

    0xc53a2e43,// 15 PAY  12 

    0x5e329820,// 16 PAY  13 

    0x388b4f7c,// 17 PAY  14 

    0x3eb16c0f,// 18 PAY  15 

    0x0a2dd7ce,// 19 PAY  16 

    0xd58bc84e,// 20 PAY  17 

    0x94abdb09,// 21 PAY  18 

    0x240d9aca,// 22 PAY  19 

    0x9ef06fc3,// 23 PAY  20 

    0xdba1cbeb,// 24 PAY  21 

    0xdba3d296,// 25 PAY  22 

    0x78d375c1,// 26 PAY  23 

    0x188c7d0d,// 27 PAY  24 

    0xd42bad77,// 28 PAY  25 

    0x24474aac,// 29 PAY  26 

    0x1436c8e1,// 30 PAY  27 

    0x2980a233,// 31 PAY  28 

    0x05c683ce,// 32 PAY  29 

    0x86b6a3dc,// 33 PAY  30 

    0x5a14c054,// 34 PAY  31 

    0x54c9897f,// 35 PAY  32 

    0x13a55edd,// 36 PAY  33 

    0x874f90d4,// 37 PAY  34 

    0xddd7db5c,// 38 PAY  35 

    0xf9e9e0f9,// 39 PAY  36 

    0x13455c2c,// 40 PAY  37 

    0x43656b57,// 41 PAY  38 

    0x9c33a077,// 42 PAY  39 

    0xe3497662,// 43 PAY  40 

    0x3eb2dc68,// 44 PAY  41 

    0x7598208a,// 45 PAY  42 

    0xa6f6abfc,// 46 PAY  43 

    0xca151563,// 47 PAY  44 

    0x18387bc3,// 48 PAY  45 

    0x25e464e5,// 49 PAY  46 

    0x511942c3,// 50 PAY  47 

    0x1a734805,// 51 PAY  48 

    0x5e21ef4a,// 52 PAY  49 

    0x0a13095a,// 53 PAY  50 

    0x7c2df976,// 54 PAY  51 

    0xe5258e51,// 55 PAY  52 

    0x4423f165,// 56 PAY  53 

    0x9a7dd970,// 57 PAY  54 

    0xd346c855,// 58 PAY  55 

    0xb0d7a56f,// 59 PAY  56 

    0xf827f69c,// 60 PAY  57 

    0xc3ea28ac,// 61 PAY  58 

    0x2d1bc12d,// 62 PAY  59 

    0xb725a87e,// 63 PAY  60 

    0x20c55069,// 64 PAY  61 

    0xd1ab3607,// 65 PAY  62 

    0xe6345e47,// 66 PAY  63 

    0x130e20bf,// 67 PAY  64 

    0x76a80e35,// 68 PAY  65 

    0xcff34685,// 69 PAY  66 

    0x871029af,// 70 PAY  67 

    0xc3608935,// 71 PAY  68 

    0x82746d95,// 72 PAY  69 

    0xc261a3a0,// 73 PAY  70 

    0x85123bf2,// 74 PAY  71 

    0xe953c370,// 75 PAY  72 

    0xa1979407,// 76 PAY  73 

    0xa7666b02,// 77 PAY  74 

    0xc4c9bfff,// 78 PAY  75 

    0x3a0db88c,// 79 PAY  76 

    0xce456b2f,// 80 PAY  77 

    0x027094af,// 81 PAY  78 

    0x13ad8613,// 82 PAY  79 

    0xa184df19,// 83 PAY  80 

    0x1ddb8ee9,// 84 PAY  81 

    0xabc9e496,// 85 PAY  82 

    0x58412737,// 86 PAY  83 

    0xa1c59d75,// 87 PAY  84 

    0x7a984ba6,// 88 PAY  85 

    0x25d35471,// 89 PAY  86 

    0xa91da804,// 90 PAY  87 

    0x8634312c,// 91 PAY  88 

    0x69d68323,// 92 PAY  89 

    0x01bc0fab,// 93 PAY  90 

    0x1770a954,// 94 PAY  91 

    0xa886a10f,// 95 PAY  92 

    0x69500aa2,// 96 PAY  93 

    0xfc387129,// 97 PAY  94 

    0x6ae74e6b,// 98 PAY  95 

    0x7a1c05be,// 99 PAY  96 

    0x8ab1413d,// 100 PAY  97 

    0xef5924e3,// 101 PAY  98 

    0x493e5e4b,// 102 PAY  99 

    0x5cc6fe94,// 103 PAY 100 

    0x74d62d47,// 104 PAY 101 

    0xed206e13,// 105 PAY 102 

    0xecd0a2a1,// 106 PAY 103 

    0xa98002f3,// 107 PAY 104 

    0x7393cce8,// 108 PAY 105 

    0xce4fb080,// 109 PAY 106 

    0x1ae1f05e,// 110 PAY 107 

    0x16ec4186,// 111 PAY 108 

    0x1761dad0,// 112 PAY 109 

    0x7ce26159,// 113 PAY 110 

    0x85a07975,// 114 PAY 111 

    0x97cec0a3,// 115 PAY 112 

    0xbd305df9,// 116 PAY 113 

    0x3b4caa89,// 117 PAY 114 

    0xb8168ef9,// 118 PAY 115 

    0xc9a6626e,// 119 PAY 116 

    0x359b37d9,// 120 PAY 117 

    0x300cb76f,// 121 PAY 118 

    0xcea01f20,// 122 PAY 119 

    0x0622f4da,// 123 PAY 120 

    0xae0b233d,// 124 PAY 121 

    0xe523f60d,// 125 PAY 122 

    0x0950e533,// 126 PAY 123 

    0x36a1849e,// 127 PAY 124 

    0xf778e1e2,// 128 PAY 125 

    0x00750d48,// 129 PAY 126 

    0xca31f766,// 130 PAY 127 

    0x02ea14e9,// 131 PAY 128 

    0xe85bdeed,// 132 PAY 129 

    0x0a630409,// 133 PAY 130 

    0xa42be44e,// 134 PAY 131 

    0xa4055a37,// 135 PAY 132 

    0x3610843d,// 136 PAY 133 

    0x6dd9972b,// 137 PAY 134 

    0x88ce1b36,// 138 PAY 135 

    0x4dcc177c,// 139 PAY 136 

    0x5d8b2593,// 140 PAY 137 

    0x97869cd8,// 141 PAY 138 

    0x1af02d55,// 142 PAY 139 

    0x1a1fde42,// 143 PAY 140 

    0xe66dddbf,// 144 PAY 141 

    0x059a7124,// 145 PAY 142 

    0xd2e8f7ff,// 146 PAY 143 

    0x51bdab53,// 147 PAY 144 

    0x9b3e841c,// 148 PAY 145 

    0x2c8a1683,// 149 PAY 146 

    0xe412ad30,// 150 PAY 147 

    0x170b18db,// 151 PAY 148 

    0x182514f7,// 152 PAY 149 

    0xea775719,// 153 PAY 150 

    0x5e732298,// 154 PAY 151 

    0xdf5c2193,// 155 PAY 152 

    0x61baf9be,// 156 PAY 153 

    0x5518c6c3,// 157 PAY 154 

    0x4b77ffe9,// 158 PAY 155 

    0xe5a073bf,// 159 PAY 156 

    0x79aabf6f,// 160 PAY 157 

    0xd578986c,// 161 PAY 158 

    0xa3ab7be7,// 162 PAY 159 

    0x6ed1c469,// 163 PAY 160 

    0xf4cdc9d2,// 164 PAY 161 

    0x0701ed05,// 165 PAY 162 

    0x56716e73,// 166 PAY 163 

    0xbbb3eb0f,// 167 PAY 164 

    0x0a809d8a,// 168 PAY 165 

    0x48cf0ee1,// 169 PAY 166 

    0x9f76a68a,// 170 PAY 167 

    0x61d4cc62,// 171 PAY 168 

    0xc3b35c40,// 172 PAY 169 

    0x1e8f7098,// 173 PAY 170 

    0xea712371,// 174 PAY 171 

    0xdec9aaf3,// 175 PAY 172 

    0x4c0e0b48,// 176 PAY 173 

    0x72529453,// 177 PAY 174 

    0x9861866c,// 178 PAY 175 

    0xb5f62c4a,// 179 PAY 176 

    0x0d171464,// 180 PAY 177 

    0x8c34a1ac,// 181 PAY 178 

    0x74e21190,// 182 PAY 179 

    0x766a9f1b,// 183 PAY 180 

    0x8b35a38a,// 184 PAY 181 

    0x0609d668,// 185 PAY 182 

    0x36647635,// 186 PAY 183 

    0x349390b4,// 187 PAY 184 

    0x922eba96,// 188 PAY 185 

    0xaa416ee5,// 189 PAY 186 

    0x826cbff9,// 190 PAY 187 

    0xc2dc4e36,// 191 PAY 188 

    0x59298811,// 192 PAY 189 

    0xcc38f797,// 193 PAY 190 

    0x8a8e1516,// 194 PAY 191 

    0x08cce90d,// 195 PAY 192 

    0xbbd88e8a,// 196 PAY 193 

    0x532de01d,// 197 PAY 194 

    0x3dfa1a82,// 198 PAY 195 

    0xad0f6d47,// 199 PAY 196 

    0x3e76235d,// 200 PAY 197 

    0xf047eeaa,// 201 PAY 198 

    0x5c37a513,// 202 PAY 199 

    0xa7d3aa7e,// 203 PAY 200 

    0xb9f38fd3,// 204 PAY 201 

    0x46f66432,// 205 PAY 202 

    0x2297f6a0,// 206 PAY 203 

    0x7d6e3256,// 207 PAY 204 

    0xa22c7b1e,// 208 PAY 205 

    0xdafc38cd,// 209 PAY 206 

    0xcd0bb840,// 210 PAY 207 

    0x69ad876e,// 211 PAY 208 

    0xeb1746ec,// 212 PAY 209 

    0x8bb65def,// 213 PAY 210 

    0x48e42211,// 214 PAY 211 

    0xaff69937,// 215 PAY 212 

    0x890d7904,// 216 PAY 213 

    0x2645c30b,// 217 PAY 214 

    0x4d15039d,// 218 PAY 215 

    0x2da9edf5,// 219 PAY 216 

    0x239ed126,// 220 PAY 217 

    0x26b2bee3,// 221 PAY 218 

    0x44693ffb,// 222 PAY 219 

    0xd845bbf4,// 223 PAY 220 

    0x073a2193,// 224 PAY 221 

    0x64a32fd1,// 225 PAY 222 

    0xe67e3e9b,// 226 PAY 223 

    0xddb148b3,// 227 PAY 224 

    0x8c9cca9b,// 228 PAY 225 

    0x21e933da,// 229 PAY 226 

    0x08d07b6f,// 230 PAY 227 

    0x9f2814ac,// 231 PAY 228 

    0x28e3a119,// 232 PAY 229 

    0x38a4c30f,// 233 PAY 230 

    0xad7f328c,// 234 PAY 231 

    0xf65bc813,// 235 PAY 232 

    0xb37275d2,// 236 PAY 233 

    0x0407c360,// 237 PAY 234 

    0x42838d66,// 238 PAY 235 

    0x60fdd401,// 239 PAY 236 

    0xcf57abc0,// 240 PAY 237 

    0x8a1e658a,// 241 PAY 238 

    0x2609394c,// 242 PAY 239 

    0xa2c663d5,// 243 PAY 240 

    0x71cddf17,// 244 PAY 241 

    0xbe8fb0e7,// 245 PAY 242 

    0xc59c35f5,// 246 PAY 243 

    0xacc4fb12,// 247 PAY 244 

    0xf96b41fc,// 248 PAY 245 

    0x65da2988,// 249 PAY 246 

    0xa9dfe988,// 250 PAY 247 

    0x711fb787,// 251 PAY 248 

    0xcee88a88,// 252 PAY 249 

    0xf42ab9f1,// 253 PAY 250 

    0xb12370e7,// 254 PAY 251 

    0x2b59f368,// 255 PAY 252 

    0x95717141,// 256 PAY 253 

    0x43f4e93c,// 257 PAY 254 

    0xd9fe3631,// 258 PAY 255 

    0x417479f6,// 259 PAY 256 

    0xa0f884e0,// 260 PAY 257 

    0x1b1257b3,// 261 PAY 258 

    0x4b856b06,// 262 PAY 259 

    0xb43c3fa1,// 263 PAY 260 

    0xde3286db,// 264 PAY 261 

    0xbf09bc60,// 265 PAY 262 

    0x4d0ea9d9,// 266 PAY 263 

    0x595b6ba4,// 267 PAY 264 

    0xc9523860,// 268 PAY 265 

    0xfa19cda8,// 269 PAY 266 

    0x50065e84,// 270 PAY 267 

    0xfd25ac4a,// 271 PAY 268 

    0x676e0256,// 272 PAY 269 

    0xcc6287be,// 273 PAY 270 

    0x5963b0ae,// 274 PAY 271 

    0xee4bf4f5,// 275 PAY 272 

    0x57946741,// 276 PAY 273 

    0xda59ec12,// 277 PAY 274 

    0x2e52beb8,// 278 PAY 275 

    0xda859d3c,// 279 PAY 276 

    0xc1c61900,// 280 PAY 277 

    0xf5ddc70b,// 281 PAY 278 

    0x54faca7f,// 282 PAY 279 

    0xb1a00dca,// 283 PAY 280 

    0x36c0fe98,// 284 PAY 281 

    0x7f9e50c7,// 285 PAY 282 

    0x60e710e1,// 286 PAY 283 

    0x6f02385b,// 287 PAY 284 

    0x328b8937,// 288 PAY 285 

    0x42cc3475,// 289 PAY 286 

    0xfc7a220b,// 290 PAY 287 

    0x39545885,// 291 PAY 288 

    0x44dc382f,// 292 PAY 289 

    0x7bf17122,// 293 PAY 290 

    0xbb2b6eb0,// 294 PAY 291 

    0xebcbf550,// 295 PAY 292 

    0xb68810aa,// 296 PAY 293 

    0xa7333513,// 297 PAY 294 

    0x43b49ea7,// 298 PAY 295 

    0x47990243,// 299 PAY 296 

    0x4cedf958,// 300 PAY 297 

    0xf953d373,// 301 PAY 298 

    0xf4180637,// 302 PAY 299 

    0x9f2fccae,// 303 PAY 300 

    0xaa87aa8e,// 304 PAY 301 

    0x37e50075,// 305 PAY 302 

    0x4fc99ebe,// 306 PAY 303 

    0xbe6017be,// 307 PAY 304 

    0x17a86db6,// 308 PAY 305 

    0x861baa0e,// 309 PAY 306 

    0x4d5f68c8,// 310 PAY 307 

    0xd1449091,// 311 PAY 308 

    0x769e8b0b,// 312 PAY 309 

    0xe3ad286d,// 313 PAY 310 

    0x7fc0bd05,// 314 PAY 311 

    0xb755a713,// 315 PAY 312 

    0x5671e0ee,// 316 PAY 313 

    0xaa2d6f95,// 317 PAY 314 

    0x2c1735fc,// 318 PAY 315 

    0x54cbf907,// 319 PAY 316 

    0x2391f994,// 320 PAY 317 

    0xe6081dc4,// 321 PAY 318 

    0x20c988b1,// 322 PAY 319 

    0x66544887,// 323 PAY 320 

    0x7b61de34,// 324 PAY 321 

    0x31561d13,// 325 PAY 322 

    0x930043ef,// 326 PAY 323 

    0x0ad1ebe9,// 327 PAY 324 

    0xf8cac3a6,// 328 PAY 325 

    0xecfabfb2,// 329 PAY 326 

    0x72f0e74d,// 330 PAY 327 

    0xfbeb434a,// 331 PAY 328 

    0x430343e5,// 332 PAY 329 

    0x46afeb4a,// 333 PAY 330 

    0xca0e97de,// 334 PAY 331 

    0x3d73eb96,// 335 PAY 332 

    0x5ea15a03,// 336 PAY 333 

    0xed58a651,// 337 PAY 334 

    0xddffb565,// 338 PAY 335 

    0xd5664c4f,// 339 PAY 336 

    0x3c3448ca,// 340 PAY 337 

    0x1ac0ff70,// 341 PAY 338 

    0x09fe17b4,// 342 PAY 339 

    0x507d31c7,// 343 PAY 340 

    0x49334560,// 344 PAY 341 

    0x7303b13a,// 345 PAY 342 

    0x5cddf1fd,// 346 PAY 343 

    0x592b8a0f,// 347 PAY 344 

    0x144e7fe3,// 348 PAY 345 

    0x365cd26f,// 349 PAY 346 

    0x2f1d68ee,// 350 PAY 347 

    0xfd7a4061,// 351 PAY 348 

    0xad4a7438,// 352 PAY 349 

    0x504f8d54,// 353 PAY 350 

    0x81dadc6f,// 354 PAY 351 

    0x5b049b7e,// 355 PAY 352 

    0xa1d21a76,// 356 PAY 353 

    0x476bfe7e,// 357 PAY 354 

    0xa799e80d,// 358 PAY 355 

    0x2371051b,// 359 PAY 356 

    0x3c13b123,// 360 PAY 357 

    0xb89a46a0,// 361 PAY 358 

    0xe4f5bf21,// 362 PAY 359 

    0xe3f1ed2b,// 363 PAY 360 

    0x01599d7a,// 364 PAY 361 

    0x05f786d9,// 365 PAY 362 

    0xfc0b495b,// 366 PAY 363 

    0x9b7a6db0,// 367 PAY 364 

    0x473b73e1,// 368 PAY 365 

    0xd352d10a,// 369 PAY 366 

    0x06000000,// 370 PAY 367 

/// HASH is  8 bytes 

    0xaa87aa8e,// 371 HSH   1 

    0x37e50075,// 372 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 112 

/// STA pkt_idx        : 18 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb1 

    0x0049b170 // 373 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt4_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 413 words. 

/// BDA size     is 1646 (0x66e) 

/// BDA id       is 0xff9 

    0x066e0ff9,// 3 BDA   1 

/// PAY Generic Data size   : 1646 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xec57cc42,// 4 PAY   1 

    0x0b0e4f0b,// 5 PAY   2 

    0xcdea33e3,// 6 PAY   3 

    0x953d5d55,// 7 PAY   4 

    0x0d43395d,// 8 PAY   5 

    0xb758428c,// 9 PAY   6 

    0xbcaa4e60,// 10 PAY   7 

    0x68a02f4c,// 11 PAY   8 

    0x05e03f5f,// 12 PAY   9 

    0xa874aec0,// 13 PAY  10 

    0x807d4f49,// 14 PAY  11 

    0x66c263ab,// 15 PAY  12 

    0x750b4f5a,// 16 PAY  13 

    0xd2591025,// 17 PAY  14 

    0xea482b57,// 18 PAY  15 

    0x8b01d684,// 19 PAY  16 

    0xed220ab8,// 20 PAY  17 

    0xdf8430ba,// 21 PAY  18 

    0x45a8f56e,// 22 PAY  19 

    0x83e7fc1c,// 23 PAY  20 

    0x20abc50c,// 24 PAY  21 

    0x0a9cd7db,// 25 PAY  22 

    0x82ae2f8c,// 26 PAY  23 

    0xfc5fc230,// 27 PAY  24 

    0xc12081d3,// 28 PAY  25 

    0x0183c386,// 29 PAY  26 

    0xd8f9b79e,// 30 PAY  27 

    0x7a021382,// 31 PAY  28 

    0x4bfb8e34,// 32 PAY  29 

    0xd0a270b7,// 33 PAY  30 

    0xa004b407,// 34 PAY  31 

    0x80b65480,// 35 PAY  32 

    0xd04c17e5,// 36 PAY  33 

    0x7af956f3,// 37 PAY  34 

    0xce50f284,// 38 PAY  35 

    0xe7bed5f3,// 39 PAY  36 

    0x323ef08e,// 40 PAY  37 

    0x62d0e779,// 41 PAY  38 

    0x81f8f0ba,// 42 PAY  39 

    0x1cd9f79e,// 43 PAY  40 

    0xd43057e8,// 44 PAY  41 

    0x7ce9d632,// 45 PAY  42 

    0x594b464d,// 46 PAY  43 

    0x65141b2b,// 47 PAY  44 

    0xbeae74f3,// 48 PAY  45 

    0xf034c156,// 49 PAY  46 

    0x05af7156,// 50 PAY  47 

    0x2f6e49e2,// 51 PAY  48 

    0xeeb59942,// 52 PAY  49 

    0xa11b51f8,// 53 PAY  50 

    0x264a7311,// 54 PAY  51 

    0x3f640eba,// 55 PAY  52 

    0x582b163e,// 56 PAY  53 

    0xf0a4d3c0,// 57 PAY  54 

    0x1883ef56,// 58 PAY  55 

    0x3ae39f11,// 59 PAY  56 

    0xef23ef70,// 60 PAY  57 

    0x554a3cc2,// 61 PAY  58 

    0xc3874185,// 62 PAY  59 

    0x8ff4a023,// 63 PAY  60 

    0xe9144cf4,// 64 PAY  61 

    0xbec2531a,// 65 PAY  62 

    0xfef7ae99,// 66 PAY  63 

    0xcecd9f33,// 67 PAY  64 

    0x8e8467d3,// 68 PAY  65 

    0x691d51d0,// 69 PAY  66 

    0x053d5f45,// 70 PAY  67 

    0x69e48bf5,// 71 PAY  68 

    0xe428776b,// 72 PAY  69 

    0x323f382c,// 73 PAY  70 

    0x3ca1cec9,// 74 PAY  71 

    0x1e77bd66,// 75 PAY  72 

    0x6388ee59,// 76 PAY  73 

    0xc915c172,// 77 PAY  74 

    0x8c789ebc,// 78 PAY  75 

    0xc8ad18db,// 79 PAY  76 

    0xc2883ebc,// 80 PAY  77 

    0x4adc7a7d,// 81 PAY  78 

    0x8954472e,// 82 PAY  79 

    0xb82a20f0,// 83 PAY  80 

    0xe400a2c8,// 84 PAY  81 

    0xa150c4f1,// 85 PAY  82 

    0x1073df46,// 86 PAY  83 

    0x72549864,// 87 PAY  84 

    0x9afca132,// 88 PAY  85 

    0xffa41b44,// 89 PAY  86 

    0x105ca52e,// 90 PAY  87 

    0x2ba33c3e,// 91 PAY  88 

    0x41b01629,// 92 PAY  89 

    0xa33847ed,// 93 PAY  90 

    0x477181d2,// 94 PAY  91 

    0x8bcb059f,// 95 PAY  92 

    0x47ab2bd5,// 96 PAY  93 

    0x16c9c1c6,// 97 PAY  94 

    0x921b7340,// 98 PAY  95 

    0x6c6c16fd,// 99 PAY  96 

    0x19d8486a,// 100 PAY  97 

    0x1b573b25,// 101 PAY  98 

    0x7078289c,// 102 PAY  99 

    0x347ad75c,// 103 PAY 100 

    0xdeb884e7,// 104 PAY 101 

    0x0eb68a7e,// 105 PAY 102 

    0x8a69cae6,// 106 PAY 103 

    0x14ef66a6,// 107 PAY 104 

    0xba3cc7c0,// 108 PAY 105 

    0xe8647a51,// 109 PAY 106 

    0x20412613,// 110 PAY 107 

    0xda85949f,// 111 PAY 108 

    0xd244d135,// 112 PAY 109 

    0xa9545a07,// 113 PAY 110 

    0x1a48b401,// 114 PAY 111 

    0xfe3c6059,// 115 PAY 112 

    0x46c3ef67,// 116 PAY 113 

    0x4b5d4b15,// 117 PAY 114 

    0x783f7153,// 118 PAY 115 

    0x3a974454,// 119 PAY 116 

    0xff623323,// 120 PAY 117 

    0x56c5dd4c,// 121 PAY 118 

    0x62ea4ac5,// 122 PAY 119 

    0x8fcf859b,// 123 PAY 120 

    0xdb701b44,// 124 PAY 121 

    0xda2fcca7,// 125 PAY 122 

    0xe3a2781c,// 126 PAY 123 

    0x4a0fd1c5,// 127 PAY 124 

    0xb0f95e42,// 128 PAY 125 

    0x1ec90ffc,// 129 PAY 126 

    0x720184d5,// 130 PAY 127 

    0x1b292776,// 131 PAY 128 

    0x564e9d14,// 132 PAY 129 

    0x2aae6dfe,// 133 PAY 130 

    0x94860e0b,// 134 PAY 131 

    0xcb54ff3c,// 135 PAY 132 

    0x70e6514e,// 136 PAY 133 

    0xbae5598a,// 137 PAY 134 

    0x5d4dc8d6,// 138 PAY 135 

    0xfedf7321,// 139 PAY 136 

    0x49293cff,// 140 PAY 137 

    0xf3040369,// 141 PAY 138 

    0xc8bafd8f,// 142 PAY 139 

    0xc0bbd333,// 143 PAY 140 

    0xac1e3992,// 144 PAY 141 

    0xe9413114,// 145 PAY 142 

    0x39cd5886,// 146 PAY 143 

    0xa9f9aaa5,// 147 PAY 144 

    0xec2ff718,// 148 PAY 145 

    0x06d86028,// 149 PAY 146 

    0x1b3b93b7,// 150 PAY 147 

    0xc3ed43c3,// 151 PAY 148 

    0x921ce0d2,// 152 PAY 149 

    0x6ee2da95,// 153 PAY 150 

    0x95db2eeb,// 154 PAY 151 

    0xb757293a,// 155 PAY 152 

    0xd3dd244f,// 156 PAY 153 

    0x28eaf696,// 157 PAY 154 

    0x3a398fdf,// 158 PAY 155 

    0xccad862a,// 159 PAY 156 

    0x84cc2aca,// 160 PAY 157 

    0x9172410a,// 161 PAY 158 

    0x4cad7e69,// 162 PAY 159 

    0xafc44977,// 163 PAY 160 

    0x9508aa75,// 164 PAY 161 

    0x3d809c0a,// 165 PAY 162 

    0x2ee894d7,// 166 PAY 163 

    0x8ddfccb1,// 167 PAY 164 

    0xccbf6805,// 168 PAY 165 

    0x8d40b13f,// 169 PAY 166 

    0xaa40fd23,// 170 PAY 167 

    0x5483b58e,// 171 PAY 168 

    0x7c986ebd,// 172 PAY 169 

    0xe46dc773,// 173 PAY 170 

    0xe68c5697,// 174 PAY 171 

    0xd2df36e2,// 175 PAY 172 

    0xe9c186d3,// 176 PAY 173 

    0xbd3af3a6,// 177 PAY 174 

    0x1b8996dc,// 178 PAY 175 

    0x04907115,// 179 PAY 176 

    0x148b6a12,// 180 PAY 177 

    0xb14666c5,// 181 PAY 178 

    0x3c6e92db,// 182 PAY 179 

    0x6a64e33b,// 183 PAY 180 

    0x08861609,// 184 PAY 181 

    0x9a3b9c36,// 185 PAY 182 

    0xdd73023e,// 186 PAY 183 

    0x5bf1cdf6,// 187 PAY 184 

    0xe0adfea6,// 188 PAY 185 

    0x88a5b3aa,// 189 PAY 186 

    0xc77fe16f,// 190 PAY 187 

    0xc857ccbe,// 191 PAY 188 

    0xbb854fe9,// 192 PAY 189 

    0x61704a96,// 193 PAY 190 

    0x2de19050,// 194 PAY 191 

    0x6cef3f79,// 195 PAY 192 

    0x8d217784,// 196 PAY 193 

    0xbbde8cf6,// 197 PAY 194 

    0xbb8df29e,// 198 PAY 195 

    0x45458a13,// 199 PAY 196 

    0xe251ffda,// 200 PAY 197 

    0x1fc5b97c,// 201 PAY 198 

    0x4f842917,// 202 PAY 199 

    0x9a3f944e,// 203 PAY 200 

    0x6bc05cbb,// 204 PAY 201 

    0xe72e1447,// 205 PAY 202 

    0x3f462a38,// 206 PAY 203 

    0x092889a5,// 207 PAY 204 

    0x07598579,// 208 PAY 205 

    0xab0f0791,// 209 PAY 206 

    0x19531121,// 210 PAY 207 

    0xd11e23d7,// 211 PAY 208 

    0x062bc80c,// 212 PAY 209 

    0xce02506f,// 213 PAY 210 

    0x616f463e,// 214 PAY 211 

    0x3ac3f73b,// 215 PAY 212 

    0x31896526,// 216 PAY 213 

    0x3aa9ea22,// 217 PAY 214 

    0xd3ec24ba,// 218 PAY 215 

    0x946cb132,// 219 PAY 216 

    0xdf8244f1,// 220 PAY 217 

    0xf1fcc299,// 221 PAY 218 

    0x7c014a04,// 222 PAY 219 

    0x564610b2,// 223 PAY 220 

    0x1ac19a52,// 224 PAY 221 

    0x550953da,// 225 PAY 222 

    0xd7ca0016,// 226 PAY 223 

    0x0dd23a12,// 227 PAY 224 

    0xc010d7c6,// 228 PAY 225 

    0xacf93d95,// 229 PAY 226 

    0xc33b71c6,// 230 PAY 227 

    0x87bdc006,// 231 PAY 228 

    0x12a2546c,// 232 PAY 229 

    0x3b5ec29d,// 233 PAY 230 

    0x14481049,// 234 PAY 231 

    0x063f989a,// 235 PAY 232 

    0xc79b8dcf,// 236 PAY 233 

    0x7d38acae,// 237 PAY 234 

    0xcc3d19af,// 238 PAY 235 

    0x538b5843,// 239 PAY 236 

    0x55602636,// 240 PAY 237 

    0xed2d46bf,// 241 PAY 238 

    0x5c071b69,// 242 PAY 239 

    0x91715ce7,// 243 PAY 240 

    0x7a634502,// 244 PAY 241 

    0xa6cf9901,// 245 PAY 242 

    0x5c986634,// 246 PAY 243 

    0x7fdf6220,// 247 PAY 244 

    0x71769269,// 248 PAY 245 

    0x42a860b4,// 249 PAY 246 

    0xff39845b,// 250 PAY 247 

    0x112299f8,// 251 PAY 248 

    0x7b91f0ea,// 252 PAY 249 

    0x5e938aac,// 253 PAY 250 

    0x17d5cc18,// 254 PAY 251 

    0xc2e52025,// 255 PAY 252 

    0x4fac1fb6,// 256 PAY 253 

    0x9c763460,// 257 PAY 254 

    0xea614b56,// 258 PAY 255 

    0xdda8d7d3,// 259 PAY 256 

    0x52ab3fc4,// 260 PAY 257 

    0xb935bfaf,// 261 PAY 258 

    0x24195a12,// 262 PAY 259 

    0xede5f7d0,// 263 PAY 260 

    0x496d2430,// 264 PAY 261 

    0x1a6b4bd6,// 265 PAY 262 

    0x3197afe2,// 266 PAY 263 

    0x9e9e3b8d,// 267 PAY 264 

    0x161896f3,// 268 PAY 265 

    0xa574d79a,// 269 PAY 266 

    0x89574d46,// 270 PAY 267 

    0x63908fa4,// 271 PAY 268 

    0xb6187b02,// 272 PAY 269 

    0x8dd603d1,// 273 PAY 270 

    0x9e17b3cf,// 274 PAY 271 

    0x22c21190,// 275 PAY 272 

    0xdf1b0444,// 276 PAY 273 

    0xc1483cbd,// 277 PAY 274 

    0x0066d0bc,// 278 PAY 275 

    0x365987cf,// 279 PAY 276 

    0xd9cae268,// 280 PAY 277 

    0x1a706a8b,// 281 PAY 278 

    0x9dfd8079,// 282 PAY 279 

    0xca0e74cb,// 283 PAY 280 

    0x2f3ac9ec,// 284 PAY 281 

    0x53198637,// 285 PAY 282 

    0x74215705,// 286 PAY 283 

    0x8b19721f,// 287 PAY 284 

    0x75136567,// 288 PAY 285 

    0x0a79f9bb,// 289 PAY 286 

    0x96aab64c,// 290 PAY 287 

    0xe6d1535d,// 291 PAY 288 

    0x414d9c39,// 292 PAY 289 

    0x49da01da,// 293 PAY 290 

    0xc8729d9c,// 294 PAY 291 

    0x8d1ab53f,// 295 PAY 292 

    0x35c32306,// 296 PAY 293 

    0x8041832a,// 297 PAY 294 

    0xff31377c,// 298 PAY 295 

    0xec9fffa8,// 299 PAY 296 

    0xea989e54,// 300 PAY 297 

    0x8eea52ee,// 301 PAY 298 

    0x8a4131eb,// 302 PAY 299 

    0xa8a1f5bc,// 303 PAY 300 

    0xebd1656c,// 304 PAY 301 

    0x1c62b056,// 305 PAY 302 

    0x251138c1,// 306 PAY 303 

    0xec940eb4,// 307 PAY 304 

    0x7a0b0f93,// 308 PAY 305 

    0x8c95142c,// 309 PAY 306 

    0x8c913966,// 310 PAY 307 

    0x7f161322,// 311 PAY 308 

    0x63d87181,// 312 PAY 309 

    0x12dce921,// 313 PAY 310 

    0x60fc63be,// 314 PAY 311 

    0xca2c0ce9,// 315 PAY 312 

    0x0189f2f5,// 316 PAY 313 

    0x23be0521,// 317 PAY 314 

    0xf5af28c5,// 318 PAY 315 

    0xb33541a4,// 319 PAY 316 

    0x54b3f0a4,// 320 PAY 317 

    0xa0a892c3,// 321 PAY 318 

    0x2f7add54,// 322 PAY 319 

    0xffd48f48,// 323 PAY 320 

    0x332e5e30,// 324 PAY 321 

    0x9e3bab1d,// 325 PAY 322 

    0xdd2c213f,// 326 PAY 323 

    0x80199b26,// 327 PAY 324 

    0x5515f5a9,// 328 PAY 325 

    0x12c0c768,// 329 PAY 326 

    0x9eba46df,// 330 PAY 327 

    0xa4a2e735,// 331 PAY 328 

    0xb70b2cdb,// 332 PAY 329 

    0xb2969f02,// 333 PAY 330 

    0x0ff14b60,// 334 PAY 331 

    0xae5949dc,// 335 PAY 332 

    0xc81ff3ae,// 336 PAY 333 

    0x6dbd3f29,// 337 PAY 334 

    0x58012212,// 338 PAY 335 

    0x38952af9,// 339 PAY 336 

    0x1b114a7e,// 340 PAY 337 

    0xb9859cb1,// 341 PAY 338 

    0xc44185dc,// 342 PAY 339 

    0x14395fd1,// 343 PAY 340 

    0x67cb1b40,// 344 PAY 341 

    0x2a27ca7c,// 345 PAY 342 

    0x73f1c740,// 346 PAY 343 

    0xe0fdce20,// 347 PAY 344 

    0x42df1743,// 348 PAY 345 

    0x94a0ec48,// 349 PAY 346 

    0x6231b5fc,// 350 PAY 347 

    0xb45bd432,// 351 PAY 348 

    0x78a66043,// 352 PAY 349 

    0x01a66b40,// 353 PAY 350 

    0x149b28d5,// 354 PAY 351 

    0x5eb509c6,// 355 PAY 352 

    0x5999fec8,// 356 PAY 353 

    0x05f3d303,// 357 PAY 354 

    0x3dd0a538,// 358 PAY 355 

    0xf918eb79,// 359 PAY 356 

    0x66f53a89,// 360 PAY 357 

    0x1b6b702e,// 361 PAY 358 

    0xfb098168,// 362 PAY 359 

    0xc7244f94,// 363 PAY 360 

    0x15957908,// 364 PAY 361 

    0xf5f0f351,// 365 PAY 362 

    0x3a28b326,// 366 PAY 363 

    0xc0b06a6c,// 367 PAY 364 

    0x4d1170b7,// 368 PAY 365 

    0xb3d6bc2e,// 369 PAY 366 

    0x9f1d2506,// 370 PAY 367 

    0x2c6d887c,// 371 PAY 368 

    0xfa084936,// 372 PAY 369 

    0xae61a1a9,// 373 PAY 370 

    0xc71d5ecc,// 374 PAY 371 

    0x4fbc93db,// 375 PAY 372 

    0x76d2646b,// 376 PAY 373 

    0x08726d42,// 377 PAY 374 

    0xe156bd29,// 378 PAY 375 

    0xaed28808,// 379 PAY 376 

    0xb23e39ab,// 380 PAY 377 

    0xd1394bd2,// 381 PAY 378 

    0x6a78b792,// 382 PAY 379 

    0xf5e12253,// 383 PAY 380 

    0xd1a26b74,// 384 PAY 381 

    0xd9834e71,// 385 PAY 382 

    0x411ce544,// 386 PAY 383 

    0x03403e62,// 387 PAY 384 

    0x2e1309bf,// 388 PAY 385 

    0x1658b21a,// 389 PAY 386 

    0x08158030,// 390 PAY 387 

    0x0d6188b9,// 391 PAY 388 

    0xe24d2852,// 392 PAY 389 

    0x1ce1cd18,// 393 PAY 390 

    0x319f9c76,// 394 PAY 391 

    0xa0ce4a30,// 395 PAY 392 

    0x06cca2f2,// 396 PAY 393 

    0x6ebde4f8,// 397 PAY 394 

    0xe9d5650e,// 398 PAY 395 

    0x3fd5ddfe,// 399 PAY 396 

    0x95d7c6f5,// 400 PAY 397 

    0x57ac95ee,// 401 PAY 398 

    0xabef10aa,// 402 PAY 399 

    0x9f1a6a4d,// 403 PAY 400 

    0x0ebf9a21,// 404 PAY 401 

    0x2275be2b,// 405 PAY 402 

    0xc602a1e3,// 406 PAY 403 

    0xc1edb958,// 407 PAY 404 

    0x32e3d11c,// 408 PAY 405 

    0x419dc441,// 409 PAY 406 

    0x7b36e09e,// 410 PAY 407 

    0x91188066,// 411 PAY 408 

    0x08479f0b,// 412 PAY 409 

    0xb8dd2a57,// 413 PAY 410 

    0xc097c2ff,// 414 PAY 411 

    0x13670000,// 415 PAY 412 

/// HASH is  12 bytes 

    0xe0adfea6,// 416 HSH   1 

    0x88a5b3aa,// 417 HSH   2 

    0xc77fe16f,// 418 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 162 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0051cea2 // 419 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt5_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 496 words. 

/// BDA size     is 1978 (0x7ba) 

/// BDA id       is 0x93b0 

    0x07ba93b0,// 3 BDA   1 

/// PAY Generic Data size   : 1978 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x7681f907,// 4 PAY   1 

    0xce342827,// 5 PAY   2 

    0x087e23c4,// 6 PAY   3 

    0xc63a969b,// 7 PAY   4 

    0x7f6fbaad,// 8 PAY   5 

    0x6bab7bc1,// 9 PAY   6 

    0x95fdcc0d,// 10 PAY   7 

    0x8839d8a7,// 11 PAY   8 

    0x926fe04c,// 12 PAY   9 

    0xd56601ab,// 13 PAY  10 

    0x0e114077,// 14 PAY  11 

    0xe961d67a,// 15 PAY  12 

    0xeaffce4e,// 16 PAY  13 

    0x534b1d80,// 17 PAY  14 

    0xd9c1826c,// 18 PAY  15 

    0x27c05e97,// 19 PAY  16 

    0x6a6f33d5,// 20 PAY  17 

    0x9c33d5f7,// 21 PAY  18 

    0xa0e752a6,// 22 PAY  19 

    0x2da4dd64,// 23 PAY  20 

    0xe0197f70,// 24 PAY  21 

    0x50956054,// 25 PAY  22 

    0x4f0ce1bd,// 26 PAY  23 

    0xa549fcc4,// 27 PAY  24 

    0xa050aad7,// 28 PAY  25 

    0xc57a275c,// 29 PAY  26 

    0x100d58c4,// 30 PAY  27 

    0xd9f17d53,// 31 PAY  28 

    0xd8ebaf03,// 32 PAY  29 

    0x5cc7b719,// 33 PAY  30 

    0xb94e9277,// 34 PAY  31 

    0x4db318fd,// 35 PAY  32 

    0xc0a61f20,// 36 PAY  33 

    0xe49b958d,// 37 PAY  34 

    0x8c38b785,// 38 PAY  35 

    0x76719f40,// 39 PAY  36 

    0x3402f104,// 40 PAY  37 

    0xb5e7cf4f,// 41 PAY  38 

    0x944bfeb1,// 42 PAY  39 

    0xe4261750,// 43 PAY  40 

    0xaa9b84ae,// 44 PAY  41 

    0x022f87d0,// 45 PAY  42 

    0x456977b0,// 46 PAY  43 

    0xfd77ab7a,// 47 PAY  44 

    0x715674e8,// 48 PAY  45 

    0xf297cd84,// 49 PAY  46 

    0x6526cbc0,// 50 PAY  47 

    0x71e2f18a,// 51 PAY  48 

    0x5b0c581b,// 52 PAY  49 

    0x81169a51,// 53 PAY  50 

    0x1ff0822e,// 54 PAY  51 

    0x00a3d4f2,// 55 PAY  52 

    0x44abb8c7,// 56 PAY  53 

    0xdcebc9af,// 57 PAY  54 

    0x26ab6b3a,// 58 PAY  55 

    0x0910fff0,// 59 PAY  56 

    0x74286b88,// 60 PAY  57 

    0x8bea0963,// 61 PAY  58 

    0x970b7625,// 62 PAY  59 

    0xc51e22a1,// 63 PAY  60 

    0xfaba6d9f,// 64 PAY  61 

    0x93d5103f,// 65 PAY  62 

    0xd05b43f7,// 66 PAY  63 

    0x073999a3,// 67 PAY  64 

    0xe1b965b5,// 68 PAY  65 

    0x72a0be3d,// 69 PAY  66 

    0xdb86886f,// 70 PAY  67 

    0xcb767f67,// 71 PAY  68 

    0x145efabe,// 72 PAY  69 

    0xfd806240,// 73 PAY  70 

    0xd382aec0,// 74 PAY  71 

    0x60e9242b,// 75 PAY  72 

    0x16a2ca99,// 76 PAY  73 

    0x45e9d042,// 77 PAY  74 

    0xb5f3dfd3,// 78 PAY  75 

    0xb3e58e88,// 79 PAY  76 

    0x94b7d2eb,// 80 PAY  77 

    0x2937f30d,// 81 PAY  78 

    0xb6ae60fe,// 82 PAY  79 

    0xf4f9ca6f,// 83 PAY  80 

    0x34ec4687,// 84 PAY  81 

    0x4fa1411e,// 85 PAY  82 

    0x50a11221,// 86 PAY  83 

    0x87f56cff,// 87 PAY  84 

    0xb3519d95,// 88 PAY  85 

    0x350b20f6,// 89 PAY  86 

    0x634c9e71,// 90 PAY  87 

    0xf3e92070,// 91 PAY  88 

    0x6c5a8ae9,// 92 PAY  89 

    0x25f8469d,// 93 PAY  90 

    0x1d498039,// 94 PAY  91 

    0x921b1dd7,// 95 PAY  92 

    0x37259e65,// 96 PAY  93 

    0x64c0c588,// 97 PAY  94 

    0xcc3aca69,// 98 PAY  95 

    0x4a44a8ae,// 99 PAY  96 

    0x8df30cfd,// 100 PAY  97 

    0x390fb84b,// 101 PAY  98 

    0x9d3b38a4,// 102 PAY  99 

    0x40e0f1c8,// 103 PAY 100 

    0xe1d5cd68,// 104 PAY 101 

    0x8a799752,// 105 PAY 102 

    0x956f974a,// 106 PAY 103 

    0xc115bd5a,// 107 PAY 104 

    0xc2f51a70,// 108 PAY 105 

    0x66884643,// 109 PAY 106 

    0xe5d4c883,// 110 PAY 107 

    0xb4d2074c,// 111 PAY 108 

    0xc21236f9,// 112 PAY 109 

    0x82e2d664,// 113 PAY 110 

    0x7f5e1f9c,// 114 PAY 111 

    0x6cca6e77,// 115 PAY 112 

    0x99fb1dce,// 116 PAY 113 

    0xf89f5dd7,// 117 PAY 114 

    0x889140dc,// 118 PAY 115 

    0xefa07f38,// 119 PAY 116 

    0x8d76228c,// 120 PAY 117 

    0x11b72355,// 121 PAY 118 

    0x8f5839d6,// 122 PAY 119 

    0x0e96e6f0,// 123 PAY 120 

    0x13c9ad41,// 124 PAY 121 

    0x302c10ed,// 125 PAY 122 

    0xc5c1125f,// 126 PAY 123 

    0x0a810bdd,// 127 PAY 124 

    0x9474547d,// 128 PAY 125 

    0x5b771adb,// 129 PAY 126 

    0xc88449a0,// 130 PAY 127 

    0xf9ee1d00,// 131 PAY 128 

    0x1d26ea03,// 132 PAY 129 

    0xb931d1f2,// 133 PAY 130 

    0x9e94eeff,// 134 PAY 131 

    0x8e4cce2b,// 135 PAY 132 

    0xa9cca7b8,// 136 PAY 133 

    0xd139ac02,// 137 PAY 134 

    0xb254164a,// 138 PAY 135 

    0xa5ca2171,// 139 PAY 136 

    0xa86fc941,// 140 PAY 137 

    0x25ea8c83,// 141 PAY 138 

    0x6eca2ec0,// 142 PAY 139 

    0x21df7d8e,// 143 PAY 140 

    0x2f39cb04,// 144 PAY 141 

    0xf2176c2e,// 145 PAY 142 

    0xee91288d,// 146 PAY 143 

    0x2a805af8,// 147 PAY 144 

    0xd3e0ab30,// 148 PAY 145 

    0x459abab7,// 149 PAY 146 

    0xab154117,// 150 PAY 147 

    0x2519d51e,// 151 PAY 148 

    0x81c3eea0,// 152 PAY 149 

    0xa4a5eeb0,// 153 PAY 150 

    0xfad91757,// 154 PAY 151 

    0x22ef5d79,// 155 PAY 152 

    0x79c0a49e,// 156 PAY 153 

    0x3bc31dd9,// 157 PAY 154 

    0x0a8f75b4,// 158 PAY 155 

    0x3a911387,// 159 PAY 156 

    0xc6dd1961,// 160 PAY 157 

    0x6580891f,// 161 PAY 158 

    0x91c27be1,// 162 PAY 159 

    0x484be6cd,// 163 PAY 160 

    0x7f1afb86,// 164 PAY 161 

    0x7d2f5f19,// 165 PAY 162 

    0x06f6a856,// 166 PAY 163 

    0x82ae7fec,// 167 PAY 164 

    0xd0b98dfd,// 168 PAY 165 

    0x0db859a1,// 169 PAY 166 

    0xd15e61dc,// 170 PAY 167 

    0x5ad313b4,// 171 PAY 168 

    0x5b44e27b,// 172 PAY 169 

    0xe6698e99,// 173 PAY 170 

    0xe2eb9c28,// 174 PAY 171 

    0xa6c67960,// 175 PAY 172 

    0x00ceb51b,// 176 PAY 173 

    0x2a436959,// 177 PAY 174 

    0x0df09180,// 178 PAY 175 

    0xc76e2c74,// 179 PAY 176 

    0xaa2f4c8f,// 180 PAY 177 

    0x2fe16663,// 181 PAY 178 

    0x0f591eea,// 182 PAY 179 

    0x35289d4a,// 183 PAY 180 

    0x016e6d27,// 184 PAY 181 

    0xf68a5841,// 185 PAY 182 

    0x78196903,// 186 PAY 183 

    0x23f2c675,// 187 PAY 184 

    0x9fe61437,// 188 PAY 185 

    0x533c0c0a,// 189 PAY 186 

    0x2f38dd2a,// 190 PAY 187 

    0x46c13b4b,// 191 PAY 188 

    0x76081e60,// 192 PAY 189 

    0x94712d99,// 193 PAY 190 

    0x189e3c4e,// 194 PAY 191 

    0x332a397e,// 195 PAY 192 

    0x8abe0574,// 196 PAY 193 

    0x1ce4ca2a,// 197 PAY 194 

    0x3c7452a6,// 198 PAY 195 

    0xa0783dd5,// 199 PAY 196 

    0xd5758451,// 200 PAY 197 

    0x7d2c7f7a,// 201 PAY 198 

    0x9ff87980,// 202 PAY 199 

    0x6142d783,// 203 PAY 200 

    0xdb817157,// 204 PAY 201 

    0x6e2423ca,// 205 PAY 202 

    0x39910e9f,// 206 PAY 203 

    0x79960acf,// 207 PAY 204 

    0xd499ba46,// 208 PAY 205 

    0x81eef62e,// 209 PAY 206 

    0xa6ccc6f0,// 210 PAY 207 

    0x40e078e3,// 211 PAY 208 

    0x2dcb285c,// 212 PAY 209 

    0x3a4f3608,// 213 PAY 210 

    0x765e658a,// 214 PAY 211 

    0x7c48a3db,// 215 PAY 212 

    0xefacfb19,// 216 PAY 213 

    0x14d2b986,// 217 PAY 214 

    0x1d40c4e0,// 218 PAY 215 

    0xbd66d086,// 219 PAY 216 

    0xf094d2db,// 220 PAY 217 

    0x5d05711a,// 221 PAY 218 

    0xa58cc855,// 222 PAY 219 

    0x40e6500b,// 223 PAY 220 

    0x6a056baf,// 224 PAY 221 

    0xdf5f9cc5,// 225 PAY 222 

    0x53553fe5,// 226 PAY 223 

    0x4870ed6c,// 227 PAY 224 

    0x9f468bbd,// 228 PAY 225 

    0xa767233c,// 229 PAY 226 

    0x7b7d59d1,// 230 PAY 227 

    0xb36257c9,// 231 PAY 228 

    0x8d377a3e,// 232 PAY 229 

    0xfdd95d90,// 233 PAY 230 

    0x33732497,// 234 PAY 231 

    0x7305e5f3,// 235 PAY 232 

    0x3d44e229,// 236 PAY 233 

    0xde0821aa,// 237 PAY 234 

    0xc376b2fa,// 238 PAY 235 

    0xc023dd47,// 239 PAY 236 

    0xf1092d09,// 240 PAY 237 

    0x540c6fef,// 241 PAY 238 

    0x7a76c0f1,// 242 PAY 239 

    0xdbca7e3a,// 243 PAY 240 

    0x891d8354,// 244 PAY 241 

    0x1e8b70e3,// 245 PAY 242 

    0xe4099b5f,// 246 PAY 243 

    0xf111ff68,// 247 PAY 244 

    0x919bfc92,// 248 PAY 245 

    0x14db5fe1,// 249 PAY 246 

    0xb731034e,// 250 PAY 247 

    0x5b41de56,// 251 PAY 248 

    0x2dc48a73,// 252 PAY 249 

    0xe7db8c47,// 253 PAY 250 

    0x169efc1a,// 254 PAY 251 

    0xa1a92c8a,// 255 PAY 252 

    0x1fdc7704,// 256 PAY 253 

    0xbef344bf,// 257 PAY 254 

    0xf8ebaa92,// 258 PAY 255 

    0x1814ab84,// 259 PAY 256 

    0x7e90971c,// 260 PAY 257 

    0xed09d208,// 261 PAY 258 

    0xe989f44d,// 262 PAY 259 

    0x39dbf19e,// 263 PAY 260 

    0x8d82a3bc,// 264 PAY 261 

    0xce120aed,// 265 PAY 262 

    0x7bd1da77,// 266 PAY 263 

    0x313ae237,// 267 PAY 264 

    0x6b4427f4,// 268 PAY 265 

    0x2c9289cd,// 269 PAY 266 

    0xe1cad597,// 270 PAY 267 

    0x6ae1bc01,// 271 PAY 268 

    0xe94a1699,// 272 PAY 269 

    0x4d5c50e4,// 273 PAY 270 

    0xf41962ef,// 274 PAY 271 

    0xe5e1629f,// 275 PAY 272 

    0x2b700d2f,// 276 PAY 273 

    0x706cd74a,// 277 PAY 274 

    0x9f0abfc1,// 278 PAY 275 

    0x545e4f8e,// 279 PAY 276 

    0x809c2787,// 280 PAY 277 

    0xa00a3a1e,// 281 PAY 278 

    0xfd318b33,// 282 PAY 279 

    0xd09b44f1,// 283 PAY 280 

    0xa928b4c6,// 284 PAY 281 

    0xd43c4b2c,// 285 PAY 282 

    0x70863114,// 286 PAY 283 

    0xcb4fb3b6,// 287 PAY 284 

    0xed9865c1,// 288 PAY 285 

    0x6a6abfd2,// 289 PAY 286 

    0x3329faba,// 290 PAY 287 

    0x829015a9,// 291 PAY 288 

    0x16581954,// 292 PAY 289 

    0x5ebc682d,// 293 PAY 290 

    0xe205d2ba,// 294 PAY 291 

    0x7d850073,// 295 PAY 292 

    0x15365694,// 296 PAY 293 

    0x961c07ba,// 297 PAY 294 

    0x647aacc1,// 298 PAY 295 

    0x74924e19,// 299 PAY 296 

    0x5ec9b543,// 300 PAY 297 

    0xa5a3db26,// 301 PAY 298 

    0x30120dfc,// 302 PAY 299 

    0xba168120,// 303 PAY 300 

    0x6f237746,// 304 PAY 301 

    0x552ca354,// 305 PAY 302 

    0xd7aaa735,// 306 PAY 303 

    0xa3f566c6,// 307 PAY 304 

    0xd0f404c4,// 308 PAY 305 

    0xfe3dbc3f,// 309 PAY 306 

    0x84d57ac8,// 310 PAY 307 

    0x1727618d,// 311 PAY 308 

    0x7df06c4c,// 312 PAY 309 

    0x6496b658,// 313 PAY 310 

    0x31b3c80b,// 314 PAY 311 

    0x2d97e322,// 315 PAY 312 

    0x6c770c68,// 316 PAY 313 

    0x69756623,// 317 PAY 314 

    0x8639ebae,// 318 PAY 315 

    0xd7105f92,// 319 PAY 316 

    0x9bdb4b18,// 320 PAY 317 

    0x110d2d86,// 321 PAY 318 

    0x67ad4d3a,// 322 PAY 319 

    0xcf270391,// 323 PAY 320 

    0x732b424e,// 324 PAY 321 

    0xeed2405c,// 325 PAY 322 

    0xb24df920,// 326 PAY 323 

    0x4515a6b7,// 327 PAY 324 

    0x74282536,// 328 PAY 325 

    0x1eef2183,// 329 PAY 326 

    0x7dea220b,// 330 PAY 327 

    0xa498a567,// 331 PAY 328 

    0x57ca2d8f,// 332 PAY 329 

    0x45aca0d8,// 333 PAY 330 

    0xb69fe4dd,// 334 PAY 331 

    0x469fd455,// 335 PAY 332 

    0xdbfe1a08,// 336 PAY 333 

    0xce268376,// 337 PAY 334 

    0x9481d97e,// 338 PAY 335 

    0x19918f54,// 339 PAY 336 

    0x8ad5481e,// 340 PAY 337 

    0x0dbc6726,// 341 PAY 338 

    0xdf13a634,// 342 PAY 339 

    0x1b60fec6,// 343 PAY 340 

    0xb5a064fc,// 344 PAY 341 

    0x355ec299,// 345 PAY 342 

    0x57b733b8,// 346 PAY 343 

    0x8d9a7e82,// 347 PAY 344 

    0x806e1a5d,// 348 PAY 345 

    0xde5bac48,// 349 PAY 346 

    0x70867048,// 350 PAY 347 

    0xba437dfd,// 351 PAY 348 

    0xd403c90d,// 352 PAY 349 

    0x273e89f5,// 353 PAY 350 

    0x9d36b1f2,// 354 PAY 351 

    0x1401ea32,// 355 PAY 352 

    0x932325f9,// 356 PAY 353 

    0x3c23aba9,// 357 PAY 354 

    0x0f168df4,// 358 PAY 355 

    0x8c9b3a27,// 359 PAY 356 

    0x9d15d8bc,// 360 PAY 357 

    0x76f0d5ea,// 361 PAY 358 

    0xce1e5604,// 362 PAY 359 

    0x0ddec7e2,// 363 PAY 360 

    0x67f6281c,// 364 PAY 361 

    0x073739a8,// 365 PAY 362 

    0x205c56e4,// 366 PAY 363 

    0xef63ec94,// 367 PAY 364 

    0x6fd392dc,// 368 PAY 365 

    0x91890090,// 369 PAY 366 

    0x7085ed17,// 370 PAY 367 

    0xb073ceab,// 371 PAY 368 

    0x558ee07c,// 372 PAY 369 

    0xffeb1a97,// 373 PAY 370 

    0x1b55ac65,// 374 PAY 371 

    0x4b74a900,// 375 PAY 372 

    0x72fd231e,// 376 PAY 373 

    0x42f4c8d2,// 377 PAY 374 

    0x80b32f98,// 378 PAY 375 

    0x7dea680c,// 379 PAY 376 

    0x5e264605,// 380 PAY 377 

    0x83176bb5,// 381 PAY 378 

    0x3c818bb7,// 382 PAY 379 

    0x7b360e31,// 383 PAY 380 

    0x38180958,// 384 PAY 381 

    0x84fd235b,// 385 PAY 382 

    0x3e2c9cde,// 386 PAY 383 

    0x9b2a2e2e,// 387 PAY 384 

    0xb3c72944,// 388 PAY 385 

    0x21130f79,// 389 PAY 386 

    0x478f6fc6,// 390 PAY 387 

    0x912c0ddd,// 391 PAY 388 

    0x8e017960,// 392 PAY 389 

    0xa675b23f,// 393 PAY 390 

    0x8763efbe,// 394 PAY 391 

    0xada4111f,// 395 PAY 392 

    0x6a63048c,// 396 PAY 393 

    0x58246a80,// 397 PAY 394 

    0x61332966,// 398 PAY 395 

    0x9e833939,// 399 PAY 396 

    0xa3364738,// 400 PAY 397 

    0xf1d305bf,// 401 PAY 398 

    0xb21be527,// 402 PAY 399 

    0x55eab155,// 403 PAY 400 

    0xd2d4090b,// 404 PAY 401 

    0x2692240d,// 405 PAY 402 

    0x7587aa63,// 406 PAY 403 

    0xf8e2bf9f,// 407 PAY 404 

    0xb463c29f,// 408 PAY 405 

    0x54654c96,// 409 PAY 406 

    0xf7449ac8,// 410 PAY 407 

    0x39634c21,// 411 PAY 408 

    0x950f1bb7,// 412 PAY 409 

    0x9612a48f,// 413 PAY 410 

    0xacd446f5,// 414 PAY 411 

    0x0d145cc4,// 415 PAY 412 

    0x28a65a66,// 416 PAY 413 

    0xfbe0842f,// 417 PAY 414 

    0x3294ab32,// 418 PAY 415 

    0x0feeaeac,// 419 PAY 416 

    0x5cf4b51f,// 420 PAY 417 

    0x693847bb,// 421 PAY 418 

    0x99d884d3,// 422 PAY 419 

    0x3b3f38db,// 423 PAY 420 

    0xe5f9f758,// 424 PAY 421 

    0xcad5e693,// 425 PAY 422 

    0x179b4089,// 426 PAY 423 

    0x39a13708,// 427 PAY 424 

    0x8aad6886,// 428 PAY 425 

    0xf4fcb7db,// 429 PAY 426 

    0xf3aa539c,// 430 PAY 427 

    0xe4b4e5c3,// 431 PAY 428 

    0x197f8b4b,// 432 PAY 429 

    0x158730ea,// 433 PAY 430 

    0x8b143e0c,// 434 PAY 431 

    0x3e944077,// 435 PAY 432 

    0xcd1c9ca3,// 436 PAY 433 

    0xdce7c50a,// 437 PAY 434 

    0x2ef6d692,// 438 PAY 435 

    0x219282a0,// 439 PAY 436 

    0x6c662521,// 440 PAY 437 

    0x6ef53daf,// 441 PAY 438 

    0x8bfc3966,// 442 PAY 439 

    0x8c99e796,// 443 PAY 440 

    0xb2d5d83e,// 444 PAY 441 

    0xd91fd2d4,// 445 PAY 442 

    0x2f837995,// 446 PAY 443 

    0x89fbd64c,// 447 PAY 444 

    0x09bbbfa2,// 448 PAY 445 

    0xebd3ab08,// 449 PAY 446 

    0x170dc590,// 450 PAY 447 

    0xfb79d16e,// 451 PAY 448 

    0x6897e365,// 452 PAY 449 

    0xd1d71a67,// 453 PAY 450 

    0xf7057399,// 454 PAY 451 

    0x77cc301d,// 455 PAY 452 

    0x899cd900,// 456 PAY 453 

    0xed29288a,// 457 PAY 454 

    0x5f64bda1,// 458 PAY 455 

    0x80e996ed,// 459 PAY 456 

    0x50dd9ed5,// 460 PAY 457 

    0x43f08c77,// 461 PAY 458 

    0x8dd8dd67,// 462 PAY 459 

    0xea7ea887,// 463 PAY 460 

    0xf6191f55,// 464 PAY 461 

    0xf9a9518e,// 465 PAY 462 

    0xf4c3cb97,// 466 PAY 463 

    0x8c95c476,// 467 PAY 464 

    0x80a30913,// 468 PAY 465 

    0x0ec7972f,// 469 PAY 466 

    0x83d5abfa,// 470 PAY 467 

    0xd66b1c83,// 471 PAY 468 

    0x7fe9e79b,// 472 PAY 469 

    0x17a9975c,// 473 PAY 470 

    0x98142221,// 474 PAY 471 

    0x8f2ac304,// 475 PAY 472 

    0xe40beee1,// 476 PAY 473 

    0x7a05e4ff,// 477 PAY 474 

    0x9d49c0d2,// 478 PAY 475 

    0x0ca8e2ca,// 479 PAY 476 

    0xc35fa1cc,// 480 PAY 477 

    0xa8a2cc7d,// 481 PAY 478 

    0x6769299e,// 482 PAY 479 

    0xb80778a5,// 483 PAY 480 

    0x597ca6ef,// 484 PAY 481 

    0x2b5ac06d,// 485 PAY 482 

    0xced989b2,// 486 PAY 483 

    0x1f43d4bf,// 487 PAY 484 

    0x5993adb4,// 488 PAY 485 

    0x7145d909,// 489 PAY 486 

    0x2b51c145,// 490 PAY 487 

    0x24a987f0,// 491 PAY 488 

    0x25f09a29,// 492 PAY 489 

    0x5bd5904c,// 493 PAY 490 

    0x8ccd8d68,// 494 PAY 491 

    0x54eed878,// 495 PAY 492 

    0x93177dd5,// 496 PAY 493 

    0x4cef02b7,// 497 PAY 494 

    0xba9f0000,// 498 PAY 495 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x20 

    0x02492053 // 499 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt6_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 290 words. 

/// BDA size     is 1154 (0x482) 

/// BDA id       is 0x4456 

    0x04824456,// 3 BDA   1 

/// PAY Generic Data size   : 1154 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x3f6e31d6,// 4 PAY   1 

    0x3649216b,// 5 PAY   2 

    0x7ef0263d,// 6 PAY   3 

    0x24e4ec05,// 7 PAY   4 

    0xdc83de93,// 8 PAY   5 

    0x2c642bcd,// 9 PAY   6 

    0xef9ea399,// 10 PAY   7 

    0xca566d2d,// 11 PAY   8 

    0x173e2ee2,// 12 PAY   9 

    0x3013f7c2,// 13 PAY  10 

    0x912c645c,// 14 PAY  11 

    0x1716510a,// 15 PAY  12 

    0xbbed7b74,// 16 PAY  13 

    0x0fc007cd,// 17 PAY  14 

    0x872d37eb,// 18 PAY  15 

    0xb7060b94,// 19 PAY  16 

    0x3e17246f,// 20 PAY  17 

    0x73f61113,// 21 PAY  18 

    0x3f2c0874,// 22 PAY  19 

    0xa8b6ccac,// 23 PAY  20 

    0x69d09774,// 24 PAY  21 

    0x9535a97e,// 25 PAY  22 

    0xb3cbe1e3,// 26 PAY  23 

    0xadd59e3c,// 27 PAY  24 

    0xb55c2421,// 28 PAY  25 

    0xd8e79391,// 29 PAY  26 

    0xfa3a5d3b,// 30 PAY  27 

    0xd7c3c544,// 31 PAY  28 

    0x3a491263,// 32 PAY  29 

    0xc0c5fbd2,// 33 PAY  30 

    0x749b55af,// 34 PAY  31 

    0x85b82d4a,// 35 PAY  32 

    0xd433d1a0,// 36 PAY  33 

    0xadae45ce,// 37 PAY  34 

    0xb9c9b821,// 38 PAY  35 

    0xbeb450f1,// 39 PAY  36 

    0x054681fe,// 40 PAY  37 

    0xec6ee0f2,// 41 PAY  38 

    0xb3f9e3eb,// 42 PAY  39 

    0xf110e6a6,// 43 PAY  40 

    0x04a2a60d,// 44 PAY  41 

    0x81bb6ea3,// 45 PAY  42 

    0xd2240898,// 46 PAY  43 

    0x7351469c,// 47 PAY  44 

    0xac3a6b80,// 48 PAY  45 

    0xa72f43b8,// 49 PAY  46 

    0xd7f1ed8e,// 50 PAY  47 

    0x611b319d,// 51 PAY  48 

    0x892b7fe1,// 52 PAY  49 

    0x3f7273a9,// 53 PAY  50 

    0xa4a75d68,// 54 PAY  51 

    0x6f0b9c9b,// 55 PAY  52 

    0x427b8a43,// 56 PAY  53 

    0xb761d7c6,// 57 PAY  54 

    0x74bbd429,// 58 PAY  55 

    0xe85237c0,// 59 PAY  56 

    0xf9c233e8,// 60 PAY  57 

    0x9241d96d,// 61 PAY  58 

    0xe90ad5ed,// 62 PAY  59 

    0xa8470b67,// 63 PAY  60 

    0x223435e5,// 64 PAY  61 

    0x357e409c,// 65 PAY  62 

    0x05ef5f9f,// 66 PAY  63 

    0xb076551d,// 67 PAY  64 

    0xfdb57a5f,// 68 PAY  65 

    0x0588795f,// 69 PAY  66 

    0x37921da7,// 70 PAY  67 

    0xdcc3f4ef,// 71 PAY  68 

    0x661c7471,// 72 PAY  69 

    0x8e525acb,// 73 PAY  70 

    0xf42f6c3f,// 74 PAY  71 

    0xbe02651c,// 75 PAY  72 

    0x2a63298b,// 76 PAY  73 

    0x4e804464,// 77 PAY  74 

    0x2c96a087,// 78 PAY  75 

    0x395eb5f6,// 79 PAY  76 

    0x0fb5c376,// 80 PAY  77 

    0x92584128,// 81 PAY  78 

    0x73ede514,// 82 PAY  79 

    0x93eb0fa5,// 83 PAY  80 

    0x8aed0ce0,// 84 PAY  81 

    0x6bb36dd5,// 85 PAY  82 

    0x9e695d74,// 86 PAY  83 

    0xd1f9b7dc,// 87 PAY  84 

    0x17b52a31,// 88 PAY  85 

    0x03242c95,// 89 PAY  86 

    0xda977633,// 90 PAY  87 

    0xad711088,// 91 PAY  88 

    0xb62a1701,// 92 PAY  89 

    0x987dc8d6,// 93 PAY  90 

    0x75e4c107,// 94 PAY  91 

    0xd5e376aa,// 95 PAY  92 

    0xec28f537,// 96 PAY  93 

    0x52f0b369,// 97 PAY  94 

    0xbc8db67e,// 98 PAY  95 

    0xbdd5978c,// 99 PAY  96 

    0x814f7ef7,// 100 PAY  97 

    0xd659b9be,// 101 PAY  98 

    0xed46ed0b,// 102 PAY  99 

    0x704ec9a5,// 103 PAY 100 

    0xc8f69d24,// 104 PAY 101 

    0xc91bd808,// 105 PAY 102 

    0xfb309191,// 106 PAY 103 

    0xed390c59,// 107 PAY 104 

    0xc732cc27,// 108 PAY 105 

    0x042b4288,// 109 PAY 106 

    0xe6166e13,// 110 PAY 107 

    0xd5af35ba,// 111 PAY 108 

    0x4b496352,// 112 PAY 109 

    0x6a55188e,// 113 PAY 110 

    0x9dab2870,// 114 PAY 111 

    0x82dde083,// 115 PAY 112 

    0xc07b70ff,// 116 PAY 113 

    0xee1d2586,// 117 PAY 114 

    0xd6d12fd3,// 118 PAY 115 

    0x9fcb201b,// 119 PAY 116 

    0x05a45ae5,// 120 PAY 117 

    0x855ba40c,// 121 PAY 118 

    0x3fd84dac,// 122 PAY 119 

    0x5e9a9f37,// 123 PAY 120 

    0xcbe98fa8,// 124 PAY 121 

    0x6edb60c4,// 125 PAY 122 

    0x4509b44a,// 126 PAY 123 

    0x92ae32e1,// 127 PAY 124 

    0x270a08e8,// 128 PAY 125 

    0x0f58fd30,// 129 PAY 126 

    0xbf9db3f1,// 130 PAY 127 

    0xcef3d422,// 131 PAY 128 

    0xf4d1aea7,// 132 PAY 129 

    0x8ed5e0fc,// 133 PAY 130 

    0xb0143efc,// 134 PAY 131 

    0x9819d854,// 135 PAY 132 

    0x3b2d8d18,// 136 PAY 133 

    0x166120d9,// 137 PAY 134 

    0x74cb27b7,// 138 PAY 135 

    0x1c5a565c,// 139 PAY 136 

    0xc1d42ad5,// 140 PAY 137 

    0x3fba8dad,// 141 PAY 138 

    0xb7eae706,// 142 PAY 139 

    0x8d1d971f,// 143 PAY 140 

    0x3a5114e4,// 144 PAY 141 

    0x3006ba7b,// 145 PAY 142 

    0x3bec11f6,// 146 PAY 143 

    0x39d4f830,// 147 PAY 144 

    0xb1cdace3,// 148 PAY 145 

    0x614b00a6,// 149 PAY 146 

    0xd06b9940,// 150 PAY 147 

    0xd0193729,// 151 PAY 148 

    0x64d65777,// 152 PAY 149 

    0xe5caede0,// 153 PAY 150 

    0x71751923,// 154 PAY 151 

    0x92827cd8,// 155 PAY 152 

    0x16075e53,// 156 PAY 153 

    0xd6f83dfd,// 157 PAY 154 

    0x2a8ae527,// 158 PAY 155 

    0xed129f95,// 159 PAY 156 

    0x339c852e,// 160 PAY 157 

    0xa5a47ef0,// 161 PAY 158 

    0x1a3c0055,// 162 PAY 159 

    0x7f911842,// 163 PAY 160 

    0x95a6d589,// 164 PAY 161 

    0x5e79fcea,// 165 PAY 162 

    0x90ce7b17,// 166 PAY 163 

    0x1a393ace,// 167 PAY 164 

    0x4f1998ef,// 168 PAY 165 

    0xe5203c9d,// 169 PAY 166 

    0x1e0e6100,// 170 PAY 167 

    0xb7148755,// 171 PAY 168 

    0x331314e7,// 172 PAY 169 

    0x9af46a1f,// 173 PAY 170 

    0x3d90241a,// 174 PAY 171 

    0x003c4cce,// 175 PAY 172 

    0xe3d0bad9,// 176 PAY 173 

    0xe97424ad,// 177 PAY 174 

    0x06e774dc,// 178 PAY 175 

    0x1cb956f3,// 179 PAY 176 

    0x8c9868b8,// 180 PAY 177 

    0xd1bfcb2d,// 181 PAY 178 

    0x444f570b,// 182 PAY 179 

    0x13d7001d,// 183 PAY 180 

    0xb93204dc,// 184 PAY 181 

    0x1dcfa922,// 185 PAY 182 

    0xe3f60285,// 186 PAY 183 

    0xdd48f486,// 187 PAY 184 

    0x452ea71b,// 188 PAY 185 

    0x77eeebb7,// 189 PAY 186 

    0xbb5b867c,// 190 PAY 187 

    0xe6055b0f,// 191 PAY 188 

    0xe70c4b10,// 192 PAY 189 

    0x8a770b68,// 193 PAY 190 

    0xdef0bc12,// 194 PAY 191 

    0x78d898e1,// 195 PAY 192 

    0x58b05098,// 196 PAY 193 

    0x83d88cad,// 197 PAY 194 

    0xad07dd63,// 198 PAY 195 

    0x5db64b43,// 199 PAY 196 

    0x33b690ff,// 200 PAY 197 

    0x372dd3f3,// 201 PAY 198 

    0xce2e428f,// 202 PAY 199 

    0x9e36d12a,// 203 PAY 200 

    0x0b4ff12f,// 204 PAY 201 

    0xd24b584b,// 205 PAY 202 

    0x9236e117,// 206 PAY 203 

    0x1f03768b,// 207 PAY 204 

    0xba280e94,// 208 PAY 205 

    0x8bc3c8eb,// 209 PAY 206 

    0xd9281226,// 210 PAY 207 

    0xfe843495,// 211 PAY 208 

    0x5c8cc68d,// 212 PAY 209 

    0x59d5061d,// 213 PAY 210 

    0x85baef0d,// 214 PAY 211 

    0x44d39af0,// 215 PAY 212 

    0x38a8085a,// 216 PAY 213 

    0x45ea171d,// 217 PAY 214 

    0xc8c39169,// 218 PAY 215 

    0x99e2664c,// 219 PAY 216 

    0xd32da728,// 220 PAY 217 

    0x9383c272,// 221 PAY 218 

    0x50ca8970,// 222 PAY 219 

    0x6aea8f1b,// 223 PAY 220 

    0xac50e156,// 224 PAY 221 

    0x304ff069,// 225 PAY 222 

    0x56f12e15,// 226 PAY 223 

    0xd70d1dbb,// 227 PAY 224 

    0x8aea5566,// 228 PAY 225 

    0x037faeab,// 229 PAY 226 

    0xc3872241,// 230 PAY 227 

    0x4bd574d5,// 231 PAY 228 

    0x9170c07f,// 232 PAY 229 

    0x333b9da7,// 233 PAY 230 

    0x0497b524,// 234 PAY 231 

    0x8bca3df1,// 235 PAY 232 

    0x7f2aa056,// 236 PAY 233 

    0x291fb6ce,// 237 PAY 234 

    0xa9187093,// 238 PAY 235 

    0xd4956d4d,// 239 PAY 236 

    0x8409e367,// 240 PAY 237 

    0xca620d65,// 241 PAY 238 

    0x4b1bdec4,// 242 PAY 239 

    0xcef84d13,// 243 PAY 240 

    0xb2d1b703,// 244 PAY 241 

    0xfd1f049c,// 245 PAY 242 

    0xf71d8531,// 246 PAY 243 

    0x02b2baa2,// 247 PAY 244 

    0xeab23a5c,// 248 PAY 245 

    0xc5071fae,// 249 PAY 246 

    0x13f40167,// 250 PAY 247 

    0x2a64202c,// 251 PAY 248 

    0xa93146eb,// 252 PAY 249 

    0x59fcf599,// 253 PAY 250 

    0xc149112a,// 254 PAY 251 

    0x47293107,// 255 PAY 252 

    0xcce5a5c8,// 256 PAY 253 

    0x0f05e385,// 257 PAY 254 

    0xc4a973a2,// 258 PAY 255 

    0x3cdc22c1,// 259 PAY 256 

    0x40fbc2bf,// 260 PAY 257 

    0x62f4255b,// 261 PAY 258 

    0xd96823f2,// 262 PAY 259 

    0xe5ab45f0,// 263 PAY 260 

    0x9c2ac708,// 264 PAY 261 

    0x3fd08427,// 265 PAY 262 

    0x774f88a4,// 266 PAY 263 

    0x91c271e7,// 267 PAY 264 

    0x71110374,// 268 PAY 265 

    0x56aaa9ac,// 269 PAY 266 

    0x4d04ed41,// 270 PAY 267 

    0x98a83f20,// 271 PAY 268 

    0x4f2cc50f,// 272 PAY 269 

    0xf26befd7,// 273 PAY 270 

    0x5e4eee18,// 274 PAY 271 

    0xe63571b5,// 275 PAY 272 

    0xeba53e31,// 276 PAY 273 

    0x8567de45,// 277 PAY 274 

    0xd5eb2821,// 278 PAY 275 

    0x17e443eb,// 279 PAY 276 

    0x3474dbb2,// 280 PAY 277 

    0xcdb58b8f,// 281 PAY 278 

    0x96612f20,// 282 PAY 279 

    0xfbe3aabb,// 283 PAY 280 

    0x332ac90a,// 284 PAY 281 

    0xa49b37f9,// 285 PAY 282 

    0xdafab1e5,// 286 PAY 283 

    0x5db4cc9e,// 287 PAY 284 

    0x600a70dd,// 288 PAY 285 

    0x2a6bd1d2,// 289 PAY 286 

    0xd7f14ec6,// 290 PAY 287 

    0x7c34f5aa,// 291 PAY 288 

    0x166d0000,// 292 PAY 289 

/// HASH is  12 bytes 

    0x74bbd429,// 293 HSH   1 

    0xe85237c0,// 294 HSH   2 

    0xf9c233e8,// 295 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 34 

/// STA pkt_idx        : 231 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdf 

    0x039cdf22 // 296 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt7_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 381 words. 

/// BDA size     is 1520 (0x5f0) 

/// BDA id       is 0xd61 

    0x05f00d61,// 3 BDA   1 

/// PAY Generic Data size   : 1520 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x640524dd,// 4 PAY   1 

    0xa47b144c,// 5 PAY   2 

    0xab4b809b,// 6 PAY   3 

    0xa2da8e78,// 7 PAY   4 

    0xf560e23e,// 8 PAY   5 

    0xd0afa329,// 9 PAY   6 

    0x71ea0bcb,// 10 PAY   7 

    0x6fa17160,// 11 PAY   8 

    0x39f7fe3f,// 12 PAY   9 

    0xec32211a,// 13 PAY  10 

    0xc922d321,// 14 PAY  11 

    0x353bba67,// 15 PAY  12 

    0x8a26c74a,// 16 PAY  13 

    0x340afb63,// 17 PAY  14 

    0x1b670d8e,// 18 PAY  15 

    0x796b0f3f,// 19 PAY  16 

    0x1c358c30,// 20 PAY  17 

    0xba187d41,// 21 PAY  18 

    0x02163283,// 22 PAY  19 

    0xd0873f0b,// 23 PAY  20 

    0xd3325c42,// 24 PAY  21 

    0x87e3d1f4,// 25 PAY  22 

    0x905c78e9,// 26 PAY  23 

    0x92f13eda,// 27 PAY  24 

    0xbc2ed5f1,// 28 PAY  25 

    0x4f81a5e3,// 29 PAY  26 

    0x099380c9,// 30 PAY  27 

    0x80373502,// 31 PAY  28 

    0xffec6106,// 32 PAY  29 

    0x8449f286,// 33 PAY  30 

    0xb041ac21,// 34 PAY  31 

    0x181dc405,// 35 PAY  32 

    0xafc2050e,// 36 PAY  33 

    0x719a6f19,// 37 PAY  34 

    0xc8c500a0,// 38 PAY  35 

    0x5f832704,// 39 PAY  36 

    0xba1d399e,// 40 PAY  37 

    0xd1285f74,// 41 PAY  38 

    0xa05d227f,// 42 PAY  39 

    0xb08d84a3,// 43 PAY  40 

    0xfeef377d,// 44 PAY  41 

    0xf743651d,// 45 PAY  42 

    0xa6853224,// 46 PAY  43 

    0x9b2150c5,// 47 PAY  44 

    0x98672e55,// 48 PAY  45 

    0xc3bc58a2,// 49 PAY  46 

    0x18511dfe,// 50 PAY  47 

    0x2c3126ab,// 51 PAY  48 

    0x6eb4b08f,// 52 PAY  49 

    0x8209c915,// 53 PAY  50 

    0xe0e0a007,// 54 PAY  51 

    0x2546e69a,// 55 PAY  52 

    0xcccfe759,// 56 PAY  53 

    0x2f96e53b,// 57 PAY  54 

    0xca2a7307,// 58 PAY  55 

    0x368cccb8,// 59 PAY  56 

    0xbee69860,// 60 PAY  57 

    0xd730f563,// 61 PAY  58 

    0xffde4831,// 62 PAY  59 

    0xee20aefa,// 63 PAY  60 

    0x450a7e2c,// 64 PAY  61 

    0x4ae5357b,// 65 PAY  62 

    0x2f170e47,// 66 PAY  63 

    0x6945a266,// 67 PAY  64 

    0x861e731e,// 68 PAY  65 

    0x4cb1cff1,// 69 PAY  66 

    0x14c84cd9,// 70 PAY  67 

    0x5c9c129d,// 71 PAY  68 

    0x1446b137,// 72 PAY  69 

    0x0e3c3237,// 73 PAY  70 

    0x04f5f8de,// 74 PAY  71 

    0x3d31eae9,// 75 PAY  72 

    0xb3713c08,// 76 PAY  73 

    0x6e738978,// 77 PAY  74 

    0x1bbecb19,// 78 PAY  75 

    0xa7619a11,// 79 PAY  76 

    0x6659d4b0,// 80 PAY  77 

    0x8352a59c,// 81 PAY  78 

    0x21a1e8e6,// 82 PAY  79 

    0x4ea3de5c,// 83 PAY  80 

    0xf81f08e3,// 84 PAY  81 

    0xecd99a2b,// 85 PAY  82 

    0x892bf19a,// 86 PAY  83 

    0x21a7b859,// 87 PAY  84 

    0x55115a5d,// 88 PAY  85 

    0xc19dbdc4,// 89 PAY  86 

    0x4abe641b,// 90 PAY  87 

    0x4898112f,// 91 PAY  88 

    0x10b01015,// 92 PAY  89 

    0x77aba804,// 93 PAY  90 

    0x4c01b3a9,// 94 PAY  91 

    0xcca7197c,// 95 PAY  92 

    0x4ecc672b,// 96 PAY  93 

    0x16dd72f6,// 97 PAY  94 

    0xc366dba2,// 98 PAY  95 

    0xcea08746,// 99 PAY  96 

    0xc0d5097d,// 100 PAY  97 

    0x594ae5cc,// 101 PAY  98 

    0xf164baa5,// 102 PAY  99 

    0x85fc693f,// 103 PAY 100 

    0xfa98741b,// 104 PAY 101 

    0x8edadb4e,// 105 PAY 102 

    0xbe56a92c,// 106 PAY 103 

    0x2db9622a,// 107 PAY 104 

    0xb1617272,// 108 PAY 105 

    0x6479856e,// 109 PAY 106 

    0x6d23c252,// 110 PAY 107 

    0x9e650953,// 111 PAY 108 

    0x23e0dad8,// 112 PAY 109 

    0x7f61f3d5,// 113 PAY 110 

    0x5721e9b0,// 114 PAY 111 

    0xffc9b905,// 115 PAY 112 

    0x677a8ccc,// 116 PAY 113 

    0xa92c6a75,// 117 PAY 114 

    0xe83a465b,// 118 PAY 115 

    0x44915dc9,// 119 PAY 116 

    0xce0041ef,// 120 PAY 117 

    0xb524071c,// 121 PAY 118 

    0xa93bd230,// 122 PAY 119 

    0x28a96c36,// 123 PAY 120 

    0x476fd5d0,// 124 PAY 121 

    0xec20ee49,// 125 PAY 122 

    0xbfa47606,// 126 PAY 123 

    0xd303ad9d,// 127 PAY 124 

    0x1d823058,// 128 PAY 125 

    0x15d9812a,// 129 PAY 126 

    0xe21209e8,// 130 PAY 127 

    0x1b157f40,// 131 PAY 128 

    0x0eeec2b5,// 132 PAY 129 

    0x9303f661,// 133 PAY 130 

    0x414ac280,// 134 PAY 131 

    0x9e3090c9,// 135 PAY 132 

    0xd778e467,// 136 PAY 133 

    0x13297f1e,// 137 PAY 134 

    0x915d796c,// 138 PAY 135 

    0xb8811bd5,// 139 PAY 136 

    0x8991b911,// 140 PAY 137 

    0x3c12661c,// 141 PAY 138 

    0x2babafb8,// 142 PAY 139 

    0xdff13328,// 143 PAY 140 

    0x95dc9726,// 144 PAY 141 

    0x416fc34c,// 145 PAY 142 

    0xaf9830fc,// 146 PAY 143 

    0x245773f5,// 147 PAY 144 

    0xb9f680f8,// 148 PAY 145 

    0x80f925e0,// 149 PAY 146 

    0x46a2f5cc,// 150 PAY 147 

    0xfaa0424b,// 151 PAY 148 

    0x7dd13c72,// 152 PAY 149 

    0x2eb37079,// 153 PAY 150 

    0x862ea016,// 154 PAY 151 

    0x28eb4a8c,// 155 PAY 152 

    0x7623e6bc,// 156 PAY 153 

    0x295e27ec,// 157 PAY 154 

    0xd9494b38,// 158 PAY 155 

    0xf5367d73,// 159 PAY 156 

    0x541c2edf,// 160 PAY 157 

    0xb4272e70,// 161 PAY 158 

    0xc9875952,// 162 PAY 159 

    0x51f8b17a,// 163 PAY 160 

    0x80f4e993,// 164 PAY 161 

    0xf175f7af,// 165 PAY 162 

    0xf672c778,// 166 PAY 163 

    0x9db4517a,// 167 PAY 164 

    0x8b032fc4,// 168 PAY 165 

    0xaacd4432,// 169 PAY 166 

    0x08c90f87,// 170 PAY 167 

    0xe496f449,// 171 PAY 168 

    0x908ace88,// 172 PAY 169 

    0x4495366f,// 173 PAY 170 

    0xace16d57,// 174 PAY 171 

    0x93596542,// 175 PAY 172 

    0xbf5d1a04,// 176 PAY 173 

    0x19a0207a,// 177 PAY 174 

    0x3c75b6c5,// 178 PAY 175 

    0x6c4f3570,// 179 PAY 176 

    0xddec9f2c,// 180 PAY 177 

    0x537a689a,// 181 PAY 178 

    0xa935a890,// 182 PAY 179 

    0x4cd976a5,// 183 PAY 180 

    0xa6652ee9,// 184 PAY 181 

    0x8b7c5bd7,// 185 PAY 182 

    0x4f0f712a,// 186 PAY 183 

    0x3905b79c,// 187 PAY 184 

    0xc5f7bbe2,// 188 PAY 185 

    0xc5d3b0b6,// 189 PAY 186 

    0x5a854b18,// 190 PAY 187 

    0xc71515cf,// 191 PAY 188 

    0xe6d54fab,// 192 PAY 189 

    0x58636978,// 193 PAY 190 

    0xf16b9c90,// 194 PAY 191 

    0x77d65ff4,// 195 PAY 192 

    0x14eea08b,// 196 PAY 193 

    0xf3f94f0b,// 197 PAY 194 

    0xeb389787,// 198 PAY 195 

    0x63dd3c35,// 199 PAY 196 

    0x24c4f6b2,// 200 PAY 197 

    0xfcadaf29,// 201 PAY 198 

    0xe387dec8,// 202 PAY 199 

    0x7324dabc,// 203 PAY 200 

    0x268d15bb,// 204 PAY 201 

    0x605f1b88,// 205 PAY 202 

    0x177817f0,// 206 PAY 203 

    0xd0cbbdc1,// 207 PAY 204 

    0xae8bb19c,// 208 PAY 205 

    0x7c327b71,// 209 PAY 206 

    0xf1d42984,// 210 PAY 207 

    0x2eed6581,// 211 PAY 208 

    0x1a474ee4,// 212 PAY 209 

    0xfabcadde,// 213 PAY 210 

    0x18322082,// 214 PAY 211 

    0x9980999c,// 215 PAY 212 

    0x63fcdc99,// 216 PAY 213 

    0xbc02fc57,// 217 PAY 214 

    0x57aa0b8b,// 218 PAY 215 

    0xdf6e15d8,// 219 PAY 216 

    0xd2f9e15a,// 220 PAY 217 

    0x1479e7aa,// 221 PAY 218 

    0x90ae08ee,// 222 PAY 219 

    0x94619887,// 223 PAY 220 

    0x3b2148a0,// 224 PAY 221 

    0x3d945983,// 225 PAY 222 

    0x10a94ef7,// 226 PAY 223 

    0xb0023d88,// 227 PAY 224 

    0xff20342f,// 228 PAY 225 

    0xf87f4a1a,// 229 PAY 226 

    0x247df0b0,// 230 PAY 227 

    0x9c8692ce,// 231 PAY 228 

    0x69bf0a40,// 232 PAY 229 

    0xa1083d0b,// 233 PAY 230 

    0x2343d59d,// 234 PAY 231 

    0x0621fb4f,// 235 PAY 232 

    0x6faf4e10,// 236 PAY 233 

    0xf5709111,// 237 PAY 234 

    0x0629f254,// 238 PAY 235 

    0x3b8c7f8b,// 239 PAY 236 

    0x820962b7,// 240 PAY 237 

    0x99ecd7e4,// 241 PAY 238 

    0x5172d006,// 242 PAY 239 

    0x19acfad4,// 243 PAY 240 

    0xc023a5c3,// 244 PAY 241 

    0x0c0a4f40,// 245 PAY 242 

    0xb384106c,// 246 PAY 243 

    0x3a5ec74e,// 247 PAY 244 

    0x84598d52,// 248 PAY 245 

    0xd17085b9,// 249 PAY 246 

    0x6ad9d9c3,// 250 PAY 247 

    0x602f30bb,// 251 PAY 248 

    0x0afda75a,// 252 PAY 249 

    0x10ca7048,// 253 PAY 250 

    0x882ea1e3,// 254 PAY 251 

    0xaf75f43d,// 255 PAY 252 

    0x5ec1855e,// 256 PAY 253 

    0x81f94cdf,// 257 PAY 254 

    0x224b9c51,// 258 PAY 255 

    0x7b3fd896,// 259 PAY 256 

    0xa0f859ea,// 260 PAY 257 

    0x09a5f6f0,// 261 PAY 258 

    0xd70a86c0,// 262 PAY 259 

    0x97881d36,// 263 PAY 260 

    0x2962bfe1,// 264 PAY 261 

    0x8611f5d9,// 265 PAY 262 

    0xe0296378,// 266 PAY 263 

    0x459f5c9a,// 267 PAY 264 

    0x913f6173,// 268 PAY 265 

    0x6d812b58,// 269 PAY 266 

    0x09062425,// 270 PAY 267 

    0xeed78987,// 271 PAY 268 

    0x2ab65813,// 272 PAY 269 

    0x7ffdb1d0,// 273 PAY 270 

    0x03a7b380,// 274 PAY 271 

    0xa91cb202,// 275 PAY 272 

    0x20bf39b0,// 276 PAY 273 

    0x3a64aa58,// 277 PAY 274 

    0x9a88c6a7,// 278 PAY 275 

    0xe2f44729,// 279 PAY 276 

    0x679db8bf,// 280 PAY 277 

    0x43286cc9,// 281 PAY 278 

    0x7e959271,// 282 PAY 279 

    0x119c95ea,// 283 PAY 280 

    0xa2035c2e,// 284 PAY 281 

    0x29a60a51,// 285 PAY 282 

    0xc19574eb,// 286 PAY 283 

    0x3c4e208a,// 287 PAY 284 

    0xdde1ba63,// 288 PAY 285 

    0xb0db2859,// 289 PAY 286 

    0xb924ba32,// 290 PAY 287 

    0x923135d3,// 291 PAY 288 

    0x48de564d,// 292 PAY 289 

    0xb057a259,// 293 PAY 290 

    0x0888c9e4,// 294 PAY 291 

    0xd201a88a,// 295 PAY 292 

    0x219cb509,// 296 PAY 293 

    0x45165732,// 297 PAY 294 

    0x11fac70d,// 298 PAY 295 

    0x6640c108,// 299 PAY 296 

    0x424c6667,// 300 PAY 297 

    0x6195b203,// 301 PAY 298 

    0x7bbbc86f,// 302 PAY 299 

    0x96dbbfca,// 303 PAY 300 

    0x5b71e02d,// 304 PAY 301 

    0x8cddfd58,// 305 PAY 302 

    0x8c8a9cb7,// 306 PAY 303 

    0x6b6be30c,// 307 PAY 304 

    0x32d72bb4,// 308 PAY 305 

    0x0f057e75,// 309 PAY 306 

    0xe3300a72,// 310 PAY 307 

    0x189351ba,// 311 PAY 308 

    0x6eaa3cd1,// 312 PAY 309 

    0xaf8dfa2f,// 313 PAY 310 

    0xa0d04a9a,// 314 PAY 311 

    0xbc7e37dc,// 315 PAY 312 

    0xe4669d59,// 316 PAY 313 

    0x6c67b314,// 317 PAY 314 

    0xd9036999,// 318 PAY 315 

    0x765fb775,// 319 PAY 316 

    0xfe36cd53,// 320 PAY 317 

    0x602067ff,// 321 PAY 318 

    0x5b9ca6bb,// 322 PAY 319 

    0xda34c1e1,// 323 PAY 320 

    0x488519f2,// 324 PAY 321 

    0x9324ce9d,// 325 PAY 322 

    0x55d70463,// 326 PAY 323 

    0x1560ac8c,// 327 PAY 324 

    0x6a246241,// 328 PAY 325 

    0xbc2a0be0,// 329 PAY 326 

    0x61d516c4,// 330 PAY 327 

    0xca721d56,// 331 PAY 328 

    0x38cc4ec9,// 332 PAY 329 

    0x462dbc41,// 333 PAY 330 

    0xe2523f45,// 334 PAY 331 

    0xb515377f,// 335 PAY 332 

    0x1222576a,// 336 PAY 333 

    0x6a2cfdfb,// 337 PAY 334 

    0x40de1567,// 338 PAY 335 

    0x790c7592,// 339 PAY 336 

    0x0b28dbde,// 340 PAY 337 

    0x331fdde7,// 341 PAY 338 

    0x8f6c9f39,// 342 PAY 339 

    0xaa699adc,// 343 PAY 340 

    0x73770c76,// 344 PAY 341 

    0x703357f2,// 345 PAY 342 

    0x206910de,// 346 PAY 343 

    0xb86151f8,// 347 PAY 344 

    0xfa8c377d,// 348 PAY 345 

    0xa9e08b15,// 349 PAY 346 

    0x948fc4a7,// 350 PAY 347 

    0x5584ce30,// 351 PAY 348 

    0x1c559e4e,// 352 PAY 349 

    0x434305e1,// 353 PAY 350 

    0xefdc0bb3,// 354 PAY 351 

    0xafded7cb,// 355 PAY 352 

    0x01f1116e,// 356 PAY 353 

    0x963cda0e,// 357 PAY 354 

    0xf758e163,// 358 PAY 355 

    0x44f4e224,// 359 PAY 356 

    0xb89266f7,// 360 PAY 357 

    0xd7ea9570,// 361 PAY 358 

    0x26d6061d,// 362 PAY 359 

    0xce89a675,// 363 PAY 360 

    0xebb72fd4,// 364 PAY 361 

    0xe882eaec,// 365 PAY 362 

    0xf66d936d,// 366 PAY 363 

    0xc6dc7c83,// 367 PAY 364 

    0xb48292dc,// 368 PAY 365 

    0xa69f9d19,// 369 PAY 366 

    0x902df374,// 370 PAY 367 

    0x0af53e2d,// 371 PAY 368 

    0x3c4bd0c0,// 372 PAY 369 

    0xedd707a2,// 373 PAY 370 

    0x90d5beba,// 374 PAY 371 

    0xae21b402,// 375 PAY 372 

    0x50c8f1c6,// 376 PAY 373 

    0x357440fa,// 377 PAY 374 

    0xf6e7e66d,// 378 PAY 375 

    0x6e985181,// 379 PAY 376 

    0x8ab5e98e,// 380 PAY 377 

    0xe1ea2e25,// 381 PAY 378 

    0x05be7cf2,// 382 PAY 379 

    0x42b30650,// 383 PAY 380 

/// STA is 1 words. 

/// STA num_pkts       : 140 

/// STA pkt_idx        : 203 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xee 

    0x032dee8c // 384 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt8_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 125 words. 

/// BDA size     is 495 (0x1ef) 

/// BDA id       is 0x5dfa 

    0x01ef5dfa,// 3 BDA   1 

/// PAY Generic Data size   : 495 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xe064cfd2,// 4 PAY   1 

    0x3248517b,// 5 PAY   2 

    0xbb71f92e,// 6 PAY   3 

    0xa21afa33,// 7 PAY   4 

    0x3bae1b04,// 8 PAY   5 

    0x8fbb2e05,// 9 PAY   6 

    0xc42db805,// 10 PAY   7 

    0x74fae3f9,// 11 PAY   8 

    0x9a892364,// 12 PAY   9 

    0xdf8473bf,// 13 PAY  10 

    0x09069a54,// 14 PAY  11 

    0xaa474f20,// 15 PAY  12 

    0xef967f29,// 16 PAY  13 

    0xc2e38ab4,// 17 PAY  14 

    0x47292160,// 18 PAY  15 

    0x606780b1,// 19 PAY  16 

    0x4c2d2d36,// 20 PAY  17 

    0x6db53570,// 21 PAY  18 

    0xf9900f37,// 22 PAY  19 

    0xaa43da70,// 23 PAY  20 

    0xaa2df95b,// 24 PAY  21 

    0xdca2261e,// 25 PAY  22 

    0x8854bfa2,// 26 PAY  23 

    0x0c15285b,// 27 PAY  24 

    0xbdd7e976,// 28 PAY  25 

    0x7f7e2d40,// 29 PAY  26 

    0x3161ce1d,// 30 PAY  27 

    0x652c2773,// 31 PAY  28 

    0xa9738506,// 32 PAY  29 

    0x5efc323a,// 33 PAY  30 

    0x68a19b5e,// 34 PAY  31 

    0xf196c9de,// 35 PAY  32 

    0xa1a4791d,// 36 PAY  33 

    0x8cadd153,// 37 PAY  34 

    0x5bef0cf2,// 38 PAY  35 

    0xf81722b9,// 39 PAY  36 

    0x8eafbb5e,// 40 PAY  37 

    0x3384a9b5,// 41 PAY  38 

    0x999e613f,// 42 PAY  39 

    0xc6c9106f,// 43 PAY  40 

    0x1a4b2a21,// 44 PAY  41 

    0xdee880e0,// 45 PAY  42 

    0x4f318208,// 46 PAY  43 

    0x2bb1d561,// 47 PAY  44 

    0x24fbd1d3,// 48 PAY  45 

    0xdd71eff6,// 49 PAY  46 

    0x72830bca,// 50 PAY  47 

    0x626fc626,// 51 PAY  48 

    0xc3ffb8a0,// 52 PAY  49 

    0xe818712a,// 53 PAY  50 

    0x15493ef9,// 54 PAY  51 

    0x1c89df42,// 55 PAY  52 

    0xed27b144,// 56 PAY  53 

    0xb257e9d5,// 57 PAY  54 

    0x8b679b15,// 58 PAY  55 

    0xfe7e609a,// 59 PAY  56 

    0x5a4f1a68,// 60 PAY  57 

    0x0f15e8fa,// 61 PAY  58 

    0xa2b0d977,// 62 PAY  59 

    0x02e1dc9d,// 63 PAY  60 

    0x083c9ed1,// 64 PAY  61 

    0x06299604,// 65 PAY  62 

    0x2601733e,// 66 PAY  63 

    0x5e23e524,// 67 PAY  64 

    0x9b391ead,// 68 PAY  65 

    0xe7a289ec,// 69 PAY  66 

    0x203d52e0,// 70 PAY  67 

    0x60e60e65,// 71 PAY  68 

    0x45834db9,// 72 PAY  69 

    0x67dc8110,// 73 PAY  70 

    0x6875a141,// 74 PAY  71 

    0x1211303d,// 75 PAY  72 

    0x84876103,// 76 PAY  73 

    0x25030a8d,// 77 PAY  74 

    0x328852c3,// 78 PAY  75 

    0x9f8698e6,// 79 PAY  76 

    0xba262b4f,// 80 PAY  77 

    0x8acd9c22,// 81 PAY  78 

    0xe525e0be,// 82 PAY  79 

    0x139c13de,// 83 PAY  80 

    0xd9a2bc8f,// 84 PAY  81 

    0x30d70d86,// 85 PAY  82 

    0x251fa3e7,// 86 PAY  83 

    0x0b1a67f9,// 87 PAY  84 

    0x26310c97,// 88 PAY  85 

    0x9c09f9b2,// 89 PAY  86 

    0x99e4ce7f,// 90 PAY  87 

    0x6dd4e578,// 91 PAY  88 

    0x16a78c34,// 92 PAY  89 

    0xe448d91e,// 93 PAY  90 

    0x78ec0992,// 94 PAY  91 

    0xcdc170cb,// 95 PAY  92 

    0x31d710ab,// 96 PAY  93 

    0xcaae9450,// 97 PAY  94 

    0xe579c995,// 98 PAY  95 

    0xac45edf4,// 99 PAY  96 

    0xcfc8298f,// 100 PAY  97 

    0x0dd078f9,// 101 PAY  98 

    0x52f66b2f,// 102 PAY  99 

    0x97971657,// 103 PAY 100 

    0xede1bee9,// 104 PAY 101 

    0x8dddeb38,// 105 PAY 102 

    0x39e90942,// 106 PAY 103 

    0x4da69375,// 107 PAY 104 

    0x761a742c,// 108 PAY 105 

    0xe21b6358,// 109 PAY 106 

    0x4d07d6ec,// 110 PAY 107 

    0xc31ad0e8,// 111 PAY 108 

    0x28c046a8,// 112 PAY 109 

    0x39ad5c63,// 113 PAY 110 

    0xad6e0498,// 114 PAY 111 

    0xf4dc68db,// 115 PAY 112 

    0x064e6f81,// 116 PAY 113 

    0xf72289d3,// 117 PAY 114 

    0x9a62e59c,// 118 PAY 115 

    0x0321efdf,// 119 PAY 116 

    0x46666daa,// 120 PAY 117 

    0x9936c74d,// 121 PAY 118 

    0xb0d8be80,// 122 PAY 119 

    0xa8cb4c1c,// 123 PAY 120 

    0x9eb5d141,// 124 PAY 121 

    0xeb56dea8,// 125 PAY 122 

    0x461152c6,// 126 PAY 123 

    0x4ef84c00,// 127 PAY 124 

/// HASH is  4 bytes 

    0xb257e9d5,// 128 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 239 

/// STA pkt_idx        : 63 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x00fc1cef // 129 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt9_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 459 words. 

/// BDA size     is 1832 (0x728) 

/// BDA id       is 0xc580 

    0x0728c580,// 3 BDA   1 

/// PAY Generic Data size   : 1832 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x8c5a7abf,// 4 PAY   1 

    0x22bd8831,// 5 PAY   2 

    0x308a6d4c,// 6 PAY   3 

    0xac5c75bc,// 7 PAY   4 

    0xe0b39da9,// 8 PAY   5 

    0x1453f950,// 9 PAY   6 

    0x2a72fc0c,// 10 PAY   7 

    0xecf0b7f4,// 11 PAY   8 

    0xe057bda4,// 12 PAY   9 

    0x0449caf2,// 13 PAY  10 

    0x48080c76,// 14 PAY  11 

    0xf6d70477,// 15 PAY  12 

    0x1d18e976,// 16 PAY  13 

    0x03bdbc14,// 17 PAY  14 

    0xe92e1d33,// 18 PAY  15 

    0xbdd99d5f,// 19 PAY  16 

    0x3a84466e,// 20 PAY  17 

    0xc7739a4c,// 21 PAY  18 

    0xf19acc5f,// 22 PAY  19 

    0xad76071d,// 23 PAY  20 

    0xdf0ed2c5,// 24 PAY  21 

    0xa7b58288,// 25 PAY  22 

    0x2fc93d8b,// 26 PAY  23 

    0xfd35e404,// 27 PAY  24 

    0x8b99f156,// 28 PAY  25 

    0xfe59ed70,// 29 PAY  26 

    0x1facd5d3,// 30 PAY  27 

    0x08c352d5,// 31 PAY  28 

    0xd0ec7b28,// 32 PAY  29 

    0xcbeb6993,// 33 PAY  30 

    0x8487f088,// 34 PAY  31 

    0x84b1453d,// 35 PAY  32 

    0xc125b496,// 36 PAY  33 

    0x13381b74,// 37 PAY  34 

    0xb6d836b8,// 38 PAY  35 

    0x210636eb,// 39 PAY  36 

    0xe518fd95,// 40 PAY  37 

    0xb7c3688a,// 41 PAY  38 

    0xc473077f,// 42 PAY  39 

    0x55c82246,// 43 PAY  40 

    0x72ace3ad,// 44 PAY  41 

    0xb7671764,// 45 PAY  42 

    0x20127803,// 46 PAY  43 

    0x02a96c27,// 47 PAY  44 

    0x197dcb06,// 48 PAY  45 

    0xcc71039b,// 49 PAY  46 

    0x8d0c2edb,// 50 PAY  47 

    0xb13955df,// 51 PAY  48 

    0x1b16f4e5,// 52 PAY  49 

    0x77afe6bd,// 53 PAY  50 

    0x4160f129,// 54 PAY  51 

    0x957c645f,// 55 PAY  52 

    0xd989e5e4,// 56 PAY  53 

    0x3b4d9dc0,// 57 PAY  54 

    0x672cb700,// 58 PAY  55 

    0x83d814fd,// 59 PAY  56 

    0x3e0ec623,// 60 PAY  57 

    0x43c0c8eb,// 61 PAY  58 

    0x019f367c,// 62 PAY  59 

    0x1406b4fa,// 63 PAY  60 

    0x27fb70d9,// 64 PAY  61 

    0x266c6d1e,// 65 PAY  62 

    0x4b20c7dc,// 66 PAY  63 

    0x03b76282,// 67 PAY  64 

    0xd2a6e386,// 68 PAY  65 

    0xde51fe00,// 69 PAY  66 

    0x8ab65831,// 70 PAY  67 

    0x3bf032c6,// 71 PAY  68 

    0xc451f98c,// 72 PAY  69 

    0x7b5b38b5,// 73 PAY  70 

    0x41b0e0e6,// 74 PAY  71 

    0xa416747b,// 75 PAY  72 

    0x4b581098,// 76 PAY  73 

    0xc3091a7d,// 77 PAY  74 

    0xbb212062,// 78 PAY  75 

    0x862bfcb1,// 79 PAY  76 

    0x3090f6b1,// 80 PAY  77 

    0x6b7d2532,// 81 PAY  78 

    0xad952fb8,// 82 PAY  79 

    0x01f05f27,// 83 PAY  80 

    0xa3e203a3,// 84 PAY  81 

    0xcc3af054,// 85 PAY  82 

    0x503d3497,// 86 PAY  83 

    0xd388cf91,// 87 PAY  84 

    0x437f59aa,// 88 PAY  85 

    0x97e3caf8,// 89 PAY  86 

    0xa7393f66,// 90 PAY  87 

    0xb2ddefea,// 91 PAY  88 

    0x428aae8f,// 92 PAY  89 

    0x926fca55,// 93 PAY  90 

    0x2f323f63,// 94 PAY  91 

    0x9550ad7c,// 95 PAY  92 

    0x67c046da,// 96 PAY  93 

    0xbddd4237,// 97 PAY  94 

    0x1c0a5fcc,// 98 PAY  95 

    0xedc26e9c,// 99 PAY  96 

    0xc2cfd801,// 100 PAY  97 

    0x74fafe31,// 101 PAY  98 

    0x8a211edc,// 102 PAY  99 

    0x03381754,// 103 PAY 100 

    0x708f3ae7,// 104 PAY 101 

    0xc9dc5b17,// 105 PAY 102 

    0x876f1654,// 106 PAY 103 

    0x8ae89f9c,// 107 PAY 104 

    0xd3499088,// 108 PAY 105 

    0x9103fb25,// 109 PAY 106 

    0x8420d5b3,// 110 PAY 107 

    0x49fbde5f,// 111 PAY 108 

    0xab55ddfd,// 112 PAY 109 

    0x208945cf,// 113 PAY 110 

    0xf6242b1d,// 114 PAY 111 

    0x85669971,// 115 PAY 112 

    0xa42a1eeb,// 116 PAY 113 

    0x1b1843cf,// 117 PAY 114 

    0x3d05e0ad,// 118 PAY 115 

    0xbb6e4418,// 119 PAY 116 

    0x54828579,// 120 PAY 117 

    0x2dfe922c,// 121 PAY 118 

    0x3186ebc9,// 122 PAY 119 

    0x6304cf46,// 123 PAY 120 

    0x5fce9f5a,// 124 PAY 121 

    0x02e7fd92,// 125 PAY 122 

    0x260ad9ee,// 126 PAY 123 

    0x699f598e,// 127 PAY 124 

    0x839c4e66,// 128 PAY 125 

    0xcbef26fe,// 129 PAY 126 

    0xedaf32ca,// 130 PAY 127 

    0xad024103,// 131 PAY 128 

    0x0f9e1d59,// 132 PAY 129 

    0xe8f20c7c,// 133 PAY 130 

    0x3ba806c4,// 134 PAY 131 

    0x651a3886,// 135 PAY 132 

    0x40495a3f,// 136 PAY 133 

    0x0ba1b294,// 137 PAY 134 

    0x6e6e84db,// 138 PAY 135 

    0x7baed7c9,// 139 PAY 136 

    0x2247cf11,// 140 PAY 137 

    0xe923493f,// 141 PAY 138 

    0x5f69f419,// 142 PAY 139 

    0xc82749a1,// 143 PAY 140 

    0x875a4347,// 144 PAY 141 

    0xb7ddd65d,// 145 PAY 142 

    0x6a9210d4,// 146 PAY 143 

    0x60db1108,// 147 PAY 144 

    0x8f831386,// 148 PAY 145 

    0xf0183f69,// 149 PAY 146 

    0xe6881ed6,// 150 PAY 147 

    0xe46b653f,// 151 PAY 148 

    0x561a6f81,// 152 PAY 149 

    0x11125dd0,// 153 PAY 150 

    0xdfe674ea,// 154 PAY 151 

    0x8a9d5ed9,// 155 PAY 152 

    0xd1f270bf,// 156 PAY 153 

    0x60f86c1f,// 157 PAY 154 

    0x88432d99,// 158 PAY 155 

    0x4dcd2456,// 159 PAY 156 

    0x32ae973f,// 160 PAY 157 

    0xfbae3976,// 161 PAY 158 

    0xabd98802,// 162 PAY 159 

    0xbabb966d,// 163 PAY 160 

    0xb4f92e7a,// 164 PAY 161 

    0x1a9f3570,// 165 PAY 162 

    0x1cc3b816,// 166 PAY 163 

    0x266de4c9,// 167 PAY 164 

    0x56bd73a8,// 168 PAY 165 

    0x6a76eeda,// 169 PAY 166 

    0x10409a76,// 170 PAY 167 

    0xe62370c4,// 171 PAY 168 

    0x062cf065,// 172 PAY 169 

    0x69d676e7,// 173 PAY 170 

    0x332d09d7,// 174 PAY 171 

    0xdb58c621,// 175 PAY 172 

    0x9ece4892,// 176 PAY 173 

    0x1e388c8e,// 177 PAY 174 

    0xd7693718,// 178 PAY 175 

    0x9fba92e1,// 179 PAY 176 

    0x08b70ebf,// 180 PAY 177 

    0x58e262d0,// 181 PAY 178 

    0x0a16ae06,// 182 PAY 179 

    0x512bb4ab,// 183 PAY 180 

    0x74a45d36,// 184 PAY 181 

    0x980afd1a,// 185 PAY 182 

    0xa736c9b4,// 186 PAY 183 

    0xa9c15fc2,// 187 PAY 184 

    0xa1ea161c,// 188 PAY 185 

    0x2e96d2ab,// 189 PAY 186 

    0x0b42d3e3,// 190 PAY 187 

    0xe0df1c89,// 191 PAY 188 

    0x1a50666f,// 192 PAY 189 

    0xcab69eb9,// 193 PAY 190 

    0x7ec427b2,// 194 PAY 191 

    0x02d55436,// 195 PAY 192 

    0xa5c044a2,// 196 PAY 193 

    0xe66bc0e2,// 197 PAY 194 

    0x4f05ae00,// 198 PAY 195 

    0xa3d069cd,// 199 PAY 196 

    0x2a9c65f4,// 200 PAY 197 

    0x76fd03cb,// 201 PAY 198 

    0x96898c8f,// 202 PAY 199 

    0xd6e5094f,// 203 PAY 200 

    0xe90c25ba,// 204 PAY 201 

    0x47cb2c94,// 205 PAY 202 

    0xb57de792,// 206 PAY 203 

    0x7ca9aa21,// 207 PAY 204 

    0xa26014db,// 208 PAY 205 

    0xac313512,// 209 PAY 206 

    0x46304b64,// 210 PAY 207 

    0x9a472e34,// 211 PAY 208 

    0xb52ba30f,// 212 PAY 209 

    0xc328452a,// 213 PAY 210 

    0x159e10b5,// 214 PAY 211 

    0x9c182e6a,// 215 PAY 212 

    0x0a2e2b4d,// 216 PAY 213 

    0xb80c2ff5,// 217 PAY 214 

    0xd83eb23f,// 218 PAY 215 

    0x8e334f55,// 219 PAY 216 

    0x376c9f94,// 220 PAY 217 

    0x71e60461,// 221 PAY 218 

    0x15f0c0ab,// 222 PAY 219 

    0xa8fbfc4b,// 223 PAY 220 

    0xbdbf031c,// 224 PAY 221 

    0x157fa1f7,// 225 PAY 222 

    0x583e8bf1,// 226 PAY 223 

    0x06706fc5,// 227 PAY 224 

    0x65b245b6,// 228 PAY 225 

    0xb0baced5,// 229 PAY 226 

    0x7eb03e5d,// 230 PAY 227 

    0xa3f27233,// 231 PAY 228 

    0xf1d16883,// 232 PAY 229 

    0x03d5a4e6,// 233 PAY 230 

    0xe5917fb9,// 234 PAY 231 

    0x4adf4e97,// 235 PAY 232 

    0x8b6b32b2,// 236 PAY 233 

    0xc5b09709,// 237 PAY 234 

    0x87eaa832,// 238 PAY 235 

    0xc57f0c69,// 239 PAY 236 

    0xe39c320b,// 240 PAY 237 

    0x4260d5f5,// 241 PAY 238 

    0xb20b67f9,// 242 PAY 239 

    0x75e267ef,// 243 PAY 240 

    0xaf19c442,// 244 PAY 241 

    0x04febaa6,// 245 PAY 242 

    0xef9defc1,// 246 PAY 243 

    0xcb52550f,// 247 PAY 244 

    0x1a92ca85,// 248 PAY 245 

    0x66aef3b8,// 249 PAY 246 

    0x3aa7ae5e,// 250 PAY 247 

    0x070f8428,// 251 PAY 248 

    0xd68c4d23,// 252 PAY 249 

    0x04125529,// 253 PAY 250 

    0xa02c228b,// 254 PAY 251 

    0x74a640a0,// 255 PAY 252 

    0xf129ad4d,// 256 PAY 253 

    0x16dec9ad,// 257 PAY 254 

    0x19e6a43e,// 258 PAY 255 

    0x37b4a14e,// 259 PAY 256 

    0x1195ad37,// 260 PAY 257 

    0x41151898,// 261 PAY 258 

    0x414a043e,// 262 PAY 259 

    0x1540b472,// 263 PAY 260 

    0x619a2414,// 264 PAY 261 

    0x10915981,// 265 PAY 262 

    0x54c2f0fb,// 266 PAY 263 

    0x97dbc6b6,// 267 PAY 264 

    0xeef314e5,// 268 PAY 265 

    0xc67fdea9,// 269 PAY 266 

    0x01849bcc,// 270 PAY 267 

    0x52277cc3,// 271 PAY 268 

    0x72886395,// 272 PAY 269 

    0x61138f97,// 273 PAY 270 

    0x96650ba0,// 274 PAY 271 

    0x79efe11b,// 275 PAY 272 

    0xd4dc6148,// 276 PAY 273 

    0x75d6b56d,// 277 PAY 274 

    0x321d000e,// 278 PAY 275 

    0xb3af92b4,// 279 PAY 276 

    0xd66bc549,// 280 PAY 277 

    0x7ab71966,// 281 PAY 278 

    0x6d19ef96,// 282 PAY 279 

    0x18ec8aea,// 283 PAY 280 

    0x6870f3c7,// 284 PAY 281 

    0x3133984a,// 285 PAY 282 

    0xb414154d,// 286 PAY 283 

    0xc8a0b9f4,// 287 PAY 284 

    0xffca68d8,// 288 PAY 285 

    0x49aa31dd,// 289 PAY 286 

    0x0c4dfa42,// 290 PAY 287 

    0xd709761b,// 291 PAY 288 

    0x22236c82,// 292 PAY 289 

    0x8f1526a8,// 293 PAY 290 

    0x48a22d9e,// 294 PAY 291 

    0x1536ece5,// 295 PAY 292 

    0x9f19d8b8,// 296 PAY 293 

    0x6ec656bb,// 297 PAY 294 

    0xcc1e9aba,// 298 PAY 295 

    0x4320ea86,// 299 PAY 296 

    0x456402db,// 300 PAY 297 

    0xd0741ea3,// 301 PAY 298 

    0x5e79be86,// 302 PAY 299 

    0xda6aec87,// 303 PAY 300 

    0xa794c56a,// 304 PAY 301 

    0xd075e4b2,// 305 PAY 302 

    0xac27cc49,// 306 PAY 303 

    0xf87ab867,// 307 PAY 304 

    0x50c37d06,// 308 PAY 305 

    0x3d54f335,// 309 PAY 306 

    0x5a81f5c5,// 310 PAY 307 

    0x7df737a7,// 311 PAY 308 

    0x636f4298,// 312 PAY 309 

    0x549480c4,// 313 PAY 310 

    0xee2539bd,// 314 PAY 311 

    0x4c6b49d3,// 315 PAY 312 

    0x7c01b34a,// 316 PAY 313 

    0xd650e049,// 317 PAY 314 

    0x5d61bc1b,// 318 PAY 315 

    0xb1b14c0e,// 319 PAY 316 

    0x1c669b1d,// 320 PAY 317 

    0xbcc804c4,// 321 PAY 318 

    0xcbc3d10b,// 322 PAY 319 

    0x079f2f6a,// 323 PAY 320 

    0x0f65d245,// 324 PAY 321 

    0x7e5141d7,// 325 PAY 322 

    0x81f8b312,// 326 PAY 323 

    0x671f0cc5,// 327 PAY 324 

    0x126366b1,// 328 PAY 325 

    0x12acd4bf,// 329 PAY 326 

    0x6f639457,// 330 PAY 327 

    0x657d769c,// 331 PAY 328 

    0x645b6d9f,// 332 PAY 329 

    0x4750bdf6,// 333 PAY 330 

    0x54ecc2da,// 334 PAY 331 

    0x011410db,// 335 PAY 332 

    0x3b94845d,// 336 PAY 333 

    0xf4c7c82e,// 337 PAY 334 

    0x44c82d63,// 338 PAY 335 

    0x12bc3678,// 339 PAY 336 

    0x60f97e96,// 340 PAY 337 

    0x76bf0ba4,// 341 PAY 338 

    0x809959ae,// 342 PAY 339 

    0xc681d4db,// 343 PAY 340 

    0xc735e61e,// 344 PAY 341 

    0x31ccd584,// 345 PAY 342 

    0xa5c7b88e,// 346 PAY 343 

    0x5aa468ea,// 347 PAY 344 

    0xbc2241c3,// 348 PAY 345 

    0x711d42d9,// 349 PAY 346 

    0x3d88babd,// 350 PAY 347 

    0x01af02c0,// 351 PAY 348 

    0x4d5c96f2,// 352 PAY 349 

    0x7993801f,// 353 PAY 350 

    0xfb707741,// 354 PAY 351 

    0xc9174fca,// 355 PAY 352 

    0xca7e2fef,// 356 PAY 353 

    0x27cae18e,// 357 PAY 354 

    0x65d7310d,// 358 PAY 355 

    0x0c431b3c,// 359 PAY 356 

    0x1154fc05,// 360 PAY 357 

    0x4868df36,// 361 PAY 358 

    0xd875aaa6,// 362 PAY 359 

    0x2ad2aea2,// 363 PAY 360 

    0x386e5c11,// 364 PAY 361 

    0x1b1e7b72,// 365 PAY 362 

    0x8af1055e,// 366 PAY 363 

    0x4af454fa,// 367 PAY 364 

    0x0fde3c77,// 368 PAY 365 

    0x1c67663e,// 369 PAY 366 

    0x9d249404,// 370 PAY 367 

    0x6e6068a1,// 371 PAY 368 

    0xebbe1988,// 372 PAY 369 

    0xf5f10994,// 373 PAY 370 

    0x6a8684ee,// 374 PAY 371 

    0x85255bd2,// 375 PAY 372 

    0x9e5596b7,// 376 PAY 373 

    0x7f0c8aa1,// 377 PAY 374 

    0x847acec6,// 378 PAY 375 

    0xa32d6ece,// 379 PAY 376 

    0x8b8e35ab,// 380 PAY 377 

    0xd84e47a7,// 381 PAY 378 

    0x7f4f1f56,// 382 PAY 379 

    0x3502eba3,// 383 PAY 380 

    0x23f219ee,// 384 PAY 381 

    0x99c1166b,// 385 PAY 382 

    0xb37c7efc,// 386 PAY 383 

    0xecc5d9ac,// 387 PAY 384 

    0xd0721a15,// 388 PAY 385 

    0x8795235f,// 389 PAY 386 

    0x1c0e754f,// 390 PAY 387 

    0x6a7ac7f1,// 391 PAY 388 

    0x5c1b6edf,// 392 PAY 389 

    0x4676b35c,// 393 PAY 390 

    0x17e68557,// 394 PAY 391 

    0x3caadf90,// 395 PAY 392 

    0xfeca4f4e,// 396 PAY 393 

    0x61e2f92c,// 397 PAY 394 

    0xd9968cc4,// 398 PAY 395 

    0xd6165c03,// 399 PAY 396 

    0xe85e1c24,// 400 PAY 397 

    0x1b128abe,// 401 PAY 398 

    0x65e7ba07,// 402 PAY 399 

    0xac54791b,// 403 PAY 400 

    0x4cc718b8,// 404 PAY 401 

    0x968d2837,// 405 PAY 402 

    0xe5b2efcf,// 406 PAY 403 

    0xcfc92cbd,// 407 PAY 404 

    0xac601164,// 408 PAY 405 

    0x39eada58,// 409 PAY 406 

    0x76e383db,// 410 PAY 407 

    0x4a7134d2,// 411 PAY 408 

    0x678bf395,// 412 PAY 409 

    0x04045901,// 413 PAY 410 

    0x522111af,// 414 PAY 411 

    0x35846fc7,// 415 PAY 412 

    0xa5bb7eb7,// 416 PAY 413 

    0x71a7442f,// 417 PAY 414 

    0x9b8ba1ab,// 418 PAY 415 

    0xf4d4730f,// 419 PAY 416 

    0x32ec359a,// 420 PAY 417 

    0x8e741558,// 421 PAY 418 

    0xc17d4821,// 422 PAY 419 

    0x4075fc9c,// 423 PAY 420 

    0x54520906,// 424 PAY 421 

    0x7cd61f89,// 425 PAY 422 

    0x1df4061f,// 426 PAY 423 

    0x4eb023b4,// 427 PAY 424 

    0xa4d9cdd1,// 428 PAY 425 

    0x09849e41,// 429 PAY 426 

    0xe79290b5,// 430 PAY 427 

    0xb38d1a1a,// 431 PAY 428 

    0xc95cddc9,// 432 PAY 429 

    0x26076e98,// 433 PAY 430 

    0x8d26dbf3,// 434 PAY 431 

    0x181887eb,// 435 PAY 432 

    0x25d51417,// 436 PAY 433 

    0x7f2c9d78,// 437 PAY 434 

    0xd72a1675,// 438 PAY 435 

    0x820920fb,// 439 PAY 436 

    0x7f38fa8d,// 440 PAY 437 

    0x59bb5c84,// 441 PAY 438 

    0x93b5d46d,// 442 PAY 439 

    0x5c88af5e,// 443 PAY 440 

    0x555aca86,// 444 PAY 441 

    0x661ce86d,// 445 PAY 442 

    0x33d3271b,// 446 PAY 443 

    0xa221e79d,// 447 PAY 444 

    0xf9b8f25f,// 448 PAY 445 

    0x6440b4df,// 449 PAY 446 

    0x1dbab2fa,// 450 PAY 447 

    0xd567ec02,// 451 PAY 448 

    0xf6fb6dda,// 452 PAY 449 

    0x31291b60,// 453 PAY 450 

    0x61cf660f,// 454 PAY 451 

    0xdb70418c,// 455 PAY 452 

    0x303b3b4c,// 456 PAY 453 

    0x70379af8,// 457 PAY 454 

    0x50c73ff6,// 458 PAY 455 

    0x8bc1824e,// 459 PAY 456 

    0xc9d78aec,// 460 PAY 457 

    0x575912fb,// 461 PAY 458 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x71 

    0x01317149 // 462 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt10_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 444 words. 

/// BDA size     is 1771 (0x6eb) 

/// BDA id       is 0xdf39 

    0x06ebdf39,// 3 BDA   1 

/// PAY Generic Data size   : 1771 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xee1bf2e6,// 4 PAY   1 

    0xf452d10c,// 5 PAY   2 

    0x85914ff2,// 6 PAY   3 

    0xf46bb6ff,// 7 PAY   4 

    0xca3b02c8,// 8 PAY   5 

    0x179d3321,// 9 PAY   6 

    0x40bc7475,// 10 PAY   7 

    0x6939b2d6,// 11 PAY   8 

    0xfb346fc5,// 12 PAY   9 

    0x1d9dc3b7,// 13 PAY  10 

    0x5a276947,// 14 PAY  11 

    0xe12f0469,// 15 PAY  12 

    0xcdb142b0,// 16 PAY  13 

    0x582450bd,// 17 PAY  14 

    0x082dc4f0,// 18 PAY  15 

    0xdb61169d,// 19 PAY  16 

    0xd70d8214,// 20 PAY  17 

    0x71093e50,// 21 PAY  18 

    0xe6d46826,// 22 PAY  19 

    0x544547ce,// 23 PAY  20 

    0x4aebcdce,// 24 PAY  21 

    0xcfb5fc5f,// 25 PAY  22 

    0x7e78963d,// 26 PAY  23 

    0x3c098fab,// 27 PAY  24 

    0x99a04fbf,// 28 PAY  25 

    0x215bf313,// 29 PAY  26 

    0x648f4358,// 30 PAY  27 

    0x21dfa479,// 31 PAY  28 

    0x3ff61a7f,// 32 PAY  29 

    0x091c03e4,// 33 PAY  30 

    0xf6b59947,// 34 PAY  31 

    0x7ecf2943,// 35 PAY  32 

    0x5b75308f,// 36 PAY  33 

    0xbd93d5d9,// 37 PAY  34 

    0xc693ce4c,// 38 PAY  35 

    0x2250d1a3,// 39 PAY  36 

    0x215cd62e,// 40 PAY  37 

    0x203fb0e0,// 41 PAY  38 

    0x10d44d56,// 42 PAY  39 

    0x600b65a5,// 43 PAY  40 

    0x0ef18eee,// 44 PAY  41 

    0x897bf6a8,// 45 PAY  42 

    0xd1b72c47,// 46 PAY  43 

    0x5cbd95bc,// 47 PAY  44 

    0xbac4a43e,// 48 PAY  45 

    0xa5b39e40,// 49 PAY  46 

    0x20b7501f,// 50 PAY  47 

    0xb55fd748,// 51 PAY  48 

    0x7a17b832,// 52 PAY  49 

    0xb0f0bff3,// 53 PAY  50 

    0x37ae3c0a,// 54 PAY  51 

    0xfc5acb2e,// 55 PAY  52 

    0x623b85bf,// 56 PAY  53 

    0x79ca7d62,// 57 PAY  54 

    0x4797b17e,// 58 PAY  55 

    0xa1013847,// 59 PAY  56 

    0x90b68566,// 60 PAY  57 

    0x251dbe84,// 61 PAY  58 

    0xbb521501,// 62 PAY  59 

    0x254209d7,// 63 PAY  60 

    0xb481ee7d,// 64 PAY  61 

    0xcca6c4f2,// 65 PAY  62 

    0x73c77860,// 66 PAY  63 

    0xccccf6ab,// 67 PAY  64 

    0x7f30b95b,// 68 PAY  65 

    0xaff7a182,// 69 PAY  66 

    0x33e5eb96,// 70 PAY  67 

    0x7c13d8ec,// 71 PAY  68 

    0x05a60ea7,// 72 PAY  69 

    0xdaccf557,// 73 PAY  70 

    0xc971e42a,// 74 PAY  71 

    0x70092fbd,// 75 PAY  72 

    0x5a378c3e,// 76 PAY  73 

    0x417f4264,// 77 PAY  74 

    0xd8047429,// 78 PAY  75 

    0x3806fba2,// 79 PAY  76 

    0xc98a737e,// 80 PAY  77 

    0x43f41260,// 81 PAY  78 

    0xa3f9307e,// 82 PAY  79 

    0xfd2943ec,// 83 PAY  80 

    0x340e78d7,// 84 PAY  81 

    0xf379c5e8,// 85 PAY  82 

    0x51e58937,// 86 PAY  83 

    0xb46807da,// 87 PAY  84 

    0xa7a46b3b,// 88 PAY  85 

    0xad12e26c,// 89 PAY  86 

    0xcb90248e,// 90 PAY  87 

    0x97414d59,// 91 PAY  88 

    0x7c605e1d,// 92 PAY  89 

    0x55bf1166,// 93 PAY  90 

    0x392cc722,// 94 PAY  91 

    0x2893d899,// 95 PAY  92 

    0xb6fa206c,// 96 PAY  93 

    0x3dd1b205,// 97 PAY  94 

    0x26a67dd2,// 98 PAY  95 

    0x8a363609,// 99 PAY  96 

    0x581cec8c,// 100 PAY  97 

    0x282624f7,// 101 PAY  98 

    0xeaf6e7eb,// 102 PAY  99 

    0xa3fbfa90,// 103 PAY 100 

    0x505f5aac,// 104 PAY 101 

    0xaa4f9896,// 105 PAY 102 

    0xbb4f8dda,// 106 PAY 103 

    0x95a95f18,// 107 PAY 104 

    0x88a3f1a3,// 108 PAY 105 

    0xbdd667bf,// 109 PAY 106 

    0xac89a848,// 110 PAY 107 

    0x74db39bd,// 111 PAY 108 

    0x5dbc50fb,// 112 PAY 109 

    0x868aaf42,// 113 PAY 110 

    0x1d46926b,// 114 PAY 111 

    0x28998aa1,// 115 PAY 112 

    0xcc8a850d,// 116 PAY 113 

    0x75a3b33c,// 117 PAY 114 

    0x34370c38,// 118 PAY 115 

    0x1007d2d7,// 119 PAY 116 

    0x5e208c02,// 120 PAY 117 

    0x9d535bfc,// 121 PAY 118 

    0x1d32e921,// 122 PAY 119 

    0xd9e33271,// 123 PAY 120 

    0x7ac1141b,// 124 PAY 121 

    0x55281aad,// 125 PAY 122 

    0x8b8ef928,// 126 PAY 123 

    0x8ead79b1,// 127 PAY 124 

    0x8f3723da,// 128 PAY 125 

    0x7dc15b2a,// 129 PAY 126 

    0xdd465fd7,// 130 PAY 127 

    0xd177c562,// 131 PAY 128 

    0x80795d2c,// 132 PAY 129 

    0x0748e006,// 133 PAY 130 

    0x27334e67,// 134 PAY 131 

    0x9d1c40c5,// 135 PAY 132 

    0xfc9647e7,// 136 PAY 133 

    0x28256c28,// 137 PAY 134 

    0x242b5c04,// 138 PAY 135 

    0xb78c30b2,// 139 PAY 136 

    0x4d7b0e42,// 140 PAY 137 

    0x06e1588d,// 141 PAY 138 

    0xc73a2744,// 142 PAY 139 

    0x388810f2,// 143 PAY 140 

    0xae418c6b,// 144 PAY 141 

    0xcfcb4cc5,// 145 PAY 142 

    0x3a9f6e87,// 146 PAY 143 

    0x43852f53,// 147 PAY 144 

    0x6e856d8c,// 148 PAY 145 

    0xb09fadd4,// 149 PAY 146 

    0xdd7dadb2,// 150 PAY 147 

    0x729433ec,// 151 PAY 148 

    0xb59e2f80,// 152 PAY 149 

    0x97187eeb,// 153 PAY 150 

    0x259d7782,// 154 PAY 151 

    0x5f4b2f78,// 155 PAY 152 

    0x7c9f9459,// 156 PAY 153 

    0xe2b32441,// 157 PAY 154 

    0xebc6bdc0,// 158 PAY 155 

    0x806333b1,// 159 PAY 156 

    0x7c482afe,// 160 PAY 157 

    0xfe960f66,// 161 PAY 158 

    0x04fb96a0,// 162 PAY 159 

    0xddbb9ffb,// 163 PAY 160 

    0x8b982a3b,// 164 PAY 161 

    0xc2f5cca7,// 165 PAY 162 

    0x408144bf,// 166 PAY 163 

    0xe325347e,// 167 PAY 164 

    0xdb2b17a5,// 168 PAY 165 

    0xdbae6bcb,// 169 PAY 166 

    0x4821ec4e,// 170 PAY 167 

    0x9051edbc,// 171 PAY 168 

    0x61643ead,// 172 PAY 169 

    0x67d23f62,// 173 PAY 170 

    0xb4ff714e,// 174 PAY 171 

    0x6d406be7,// 175 PAY 172 

    0x18ec2796,// 176 PAY 173 

    0x110211af,// 177 PAY 174 

    0x75776ad5,// 178 PAY 175 

    0x879adc70,// 179 PAY 176 

    0x29ec0cbd,// 180 PAY 177 

    0xe5d9b18a,// 181 PAY 178 

    0x6ff39f74,// 182 PAY 179 

    0x8a21d946,// 183 PAY 180 

    0x9a804654,// 184 PAY 181 

    0x868bfdd9,// 185 PAY 182 

    0x7d42706e,// 186 PAY 183 

    0x30aa3a11,// 187 PAY 184 

    0x702b87ee,// 188 PAY 185 

    0x191397e1,// 189 PAY 186 

    0xe125f8c2,// 190 PAY 187 

    0xc339efbc,// 191 PAY 188 

    0xe44a287a,// 192 PAY 189 

    0x35d22e61,// 193 PAY 190 

    0xecb449ad,// 194 PAY 191 

    0x9db764c5,// 195 PAY 192 

    0x423dc51f,// 196 PAY 193 

    0x12b31864,// 197 PAY 194 

    0xa9090c9d,// 198 PAY 195 

    0xa8690700,// 199 PAY 196 

    0x1090116b,// 200 PAY 197 

    0x8c54df86,// 201 PAY 198 

    0x3aa14744,// 202 PAY 199 

    0x1e564b9e,// 203 PAY 200 

    0x144012f6,// 204 PAY 201 

    0xad3ff031,// 205 PAY 202 

    0x10b6df8a,// 206 PAY 203 

    0xfab6988c,// 207 PAY 204 

    0xb0f48a97,// 208 PAY 205 

    0xaa908c81,// 209 PAY 206 

    0x6b318090,// 210 PAY 207 

    0x1027b3ae,// 211 PAY 208 

    0xf6f82248,// 212 PAY 209 

    0x408bfdd5,// 213 PAY 210 

    0x05c1d5fe,// 214 PAY 211 

    0xb82dd639,// 215 PAY 212 

    0x7dd9b21b,// 216 PAY 213 

    0x87b60540,// 217 PAY 214 

    0x50f8752d,// 218 PAY 215 

    0xe8f0cbad,// 219 PAY 216 

    0x4284027f,// 220 PAY 217 

    0x3b8f6e08,// 221 PAY 218 

    0xd1b37864,// 222 PAY 219 

    0x1043b7b1,// 223 PAY 220 

    0x1db2205b,// 224 PAY 221 

    0x506db5f7,// 225 PAY 222 

    0x791c71d7,// 226 PAY 223 

    0xac73a605,// 227 PAY 224 

    0xad211e73,// 228 PAY 225 

    0x26e7bd45,// 229 PAY 226 

    0xba519b2e,// 230 PAY 227 

    0x7081f36a,// 231 PAY 228 

    0x3d0813c4,// 232 PAY 229 

    0x65ff7bf9,// 233 PAY 230 

    0x4f14436d,// 234 PAY 231 

    0xb5cb3cea,// 235 PAY 232 

    0x67e60c4e,// 236 PAY 233 

    0x3d939651,// 237 PAY 234 

    0x0e8e46ff,// 238 PAY 235 

    0x83a1c9a5,// 239 PAY 236 

    0xdf062ae9,// 240 PAY 237 

    0xf275883d,// 241 PAY 238 

    0xe9f3cfc1,// 242 PAY 239 

    0x1ba8f031,// 243 PAY 240 

    0xc5f8f6ce,// 244 PAY 241 

    0x27748d78,// 245 PAY 242 

    0xb1cb3b3b,// 246 PAY 243 

    0xefd16e93,// 247 PAY 244 

    0xb9dd327a,// 248 PAY 245 

    0x5b4ca43f,// 249 PAY 246 

    0x3a971a1c,// 250 PAY 247 

    0xaa8fa868,// 251 PAY 248 

    0x2e9c8905,// 252 PAY 249 

    0x52f87326,// 253 PAY 250 

    0x0d0354f9,// 254 PAY 251 

    0x79a090b0,// 255 PAY 252 

    0xdbafda81,// 256 PAY 253 

    0x0f0c2351,// 257 PAY 254 

    0x126055a7,// 258 PAY 255 

    0x424f5e6c,// 259 PAY 256 

    0x73b3384c,// 260 PAY 257 

    0xef422d63,// 261 PAY 258 

    0xd83e5f52,// 262 PAY 259 

    0x12a37b30,// 263 PAY 260 

    0x66696ab0,// 264 PAY 261 

    0xaf8dd456,// 265 PAY 262 

    0xc3c02fad,// 266 PAY 263 

    0x62797a37,// 267 PAY 264 

    0x4d220697,// 268 PAY 265 

    0x8533cb32,// 269 PAY 266 

    0x55bbdca5,// 270 PAY 267 

    0x31f4b3ff,// 271 PAY 268 

    0x11d0afa2,// 272 PAY 269 

    0x81e695b6,// 273 PAY 270 

    0x8e4ea732,// 274 PAY 271 

    0x2a7d2fb9,// 275 PAY 272 

    0x03153249,// 276 PAY 273 

    0x2be30d51,// 277 PAY 274 

    0x93c542e3,// 278 PAY 275 

    0x7ff2a896,// 279 PAY 276 

    0xef3c2eda,// 280 PAY 277 

    0x34e080c5,// 281 PAY 278 

    0x7db273a6,// 282 PAY 279 

    0x244097a5,// 283 PAY 280 

    0x9904da4b,// 284 PAY 281 

    0x990ac20c,// 285 PAY 282 

    0x764b0d03,// 286 PAY 283 

    0xf0bde167,// 287 PAY 284 

    0xa0d6ec2b,// 288 PAY 285 

    0x0e57c379,// 289 PAY 286 

    0xa17a3bac,// 290 PAY 287 

    0xbe897eaa,// 291 PAY 288 

    0x7adeaa10,// 292 PAY 289 

    0x2562f954,// 293 PAY 290 

    0x78dc6bcd,// 294 PAY 291 

    0x9ccd21ad,// 295 PAY 292 

    0xb1936c93,// 296 PAY 293 

    0xa565166f,// 297 PAY 294 

    0x7124ecd2,// 298 PAY 295 

    0x72c4e093,// 299 PAY 296 

    0x3108d688,// 300 PAY 297 

    0x0d1dc410,// 301 PAY 298 

    0x5c9b2774,// 302 PAY 299 

    0x32c2c30a,// 303 PAY 300 

    0x8b789ced,// 304 PAY 301 

    0x9b951446,// 305 PAY 302 

    0xd2a99560,// 306 PAY 303 

    0xc5c1b165,// 307 PAY 304 

    0x64696585,// 308 PAY 305 

    0xfc3a5a3d,// 309 PAY 306 

    0xfe46bd60,// 310 PAY 307 

    0xcd8c1a8a,// 311 PAY 308 

    0x0cfbe1a7,// 312 PAY 309 

    0x2da2416c,// 313 PAY 310 

    0x85142e93,// 314 PAY 311 

    0x2ee9cb5e,// 315 PAY 312 

    0xd8a0b38c,// 316 PAY 313 

    0xce61beb0,// 317 PAY 314 

    0xa70441b0,// 318 PAY 315 

    0xf53654ff,// 319 PAY 316 

    0x7bee950e,// 320 PAY 317 

    0x5313f503,// 321 PAY 318 

    0xb557dacb,// 322 PAY 319 

    0xa06c4a4c,// 323 PAY 320 

    0x4e0b35a8,// 324 PAY 321 

    0x41bdde50,// 325 PAY 322 

    0x84242fab,// 326 PAY 323 

    0x69847d6d,// 327 PAY 324 

    0xb8ee0e7d,// 328 PAY 325 

    0x04a454d3,// 329 PAY 326 

    0x042aa08c,// 330 PAY 327 

    0x504f7737,// 331 PAY 328 

    0x506a32fc,// 332 PAY 329 

    0xc2d9bdcd,// 333 PAY 330 

    0xcd1f3a29,// 334 PAY 331 

    0xeddac9c8,// 335 PAY 332 

    0x54185857,// 336 PAY 333 

    0x41b194b9,// 337 PAY 334 

    0xc742bb6c,// 338 PAY 335 

    0x2553aff1,// 339 PAY 336 

    0x2e292ae4,// 340 PAY 337 

    0x6f42a0ff,// 341 PAY 338 

    0x06df4343,// 342 PAY 339 

    0x994bdd13,// 343 PAY 340 

    0x14827ba9,// 344 PAY 341 

    0xe49873a5,// 345 PAY 342 

    0x1dbba3e8,// 346 PAY 343 

    0xa7b8a062,// 347 PAY 344 

    0xbe025381,// 348 PAY 345 

    0xda819ec0,// 349 PAY 346 

    0x4b8bca2b,// 350 PAY 347 

    0x3e2224b1,// 351 PAY 348 

    0x4d45f1e9,// 352 PAY 349 

    0xaa4472c8,// 353 PAY 350 

    0x069f10aa,// 354 PAY 351 

    0x87affa00,// 355 PAY 352 

    0xf9b6126c,// 356 PAY 353 

    0x6630187a,// 357 PAY 354 

    0xf8d8cb6e,// 358 PAY 355 

    0x5ad39403,// 359 PAY 356 

    0xe84b4b69,// 360 PAY 357 

    0x121e84c3,// 361 PAY 358 

    0x85f6ad02,// 362 PAY 359 

    0x77eeb4dd,// 363 PAY 360 

    0xd5ef8bba,// 364 PAY 361 

    0x353c3e7d,// 365 PAY 362 

    0xec84010b,// 366 PAY 363 

    0x0da9c808,// 367 PAY 364 

    0x805825a8,// 368 PAY 365 

    0x1c0bbda8,// 369 PAY 366 

    0x26c44e7c,// 370 PAY 367 

    0x21cc73f3,// 371 PAY 368 

    0xd61de739,// 372 PAY 369 

    0xae097791,// 373 PAY 370 

    0xaabbd9fc,// 374 PAY 371 

    0x40dd69a4,// 375 PAY 372 

    0xdbc988ff,// 376 PAY 373 

    0xfb9e6d8d,// 377 PAY 374 

    0x5609fd30,// 378 PAY 375 

    0x8f3810b1,// 379 PAY 376 

    0x911ec8c2,// 380 PAY 377 

    0x94d6c4bb,// 381 PAY 378 

    0xb7e5d967,// 382 PAY 379 

    0xc84988e7,// 383 PAY 380 

    0x50e4adbd,// 384 PAY 381 

    0x7f917efd,// 385 PAY 382 

    0x05dd866e,// 386 PAY 383 

    0xfb021239,// 387 PAY 384 

    0x1a15f55e,// 388 PAY 385 

    0xb464317e,// 389 PAY 386 

    0x0f29f25d,// 390 PAY 387 

    0x503771a8,// 391 PAY 388 

    0x33b61648,// 392 PAY 389 

    0xe696cb99,// 393 PAY 390 

    0x49a1e8f8,// 394 PAY 391 

    0x3439a06c,// 395 PAY 392 

    0xc0683b0c,// 396 PAY 393 

    0x095c6cc7,// 397 PAY 394 

    0x0313acab,// 398 PAY 395 

    0x14c6fd51,// 399 PAY 396 

    0xb6bf0875,// 400 PAY 397 

    0x4a4bd461,// 401 PAY 398 

    0x5f45998e,// 402 PAY 399 

    0xcf25c8d8,// 403 PAY 400 

    0xfd59ab0f,// 404 PAY 401 

    0xb83b4e8a,// 405 PAY 402 

    0xc1c696fe,// 406 PAY 403 

    0xe4d97d8f,// 407 PAY 404 

    0xc21e2262,// 408 PAY 405 

    0x526ff39f,// 409 PAY 406 

    0x384bb5b6,// 410 PAY 407 

    0x6c994c10,// 411 PAY 408 

    0x61f8b068,// 412 PAY 409 

    0xa0ee08fe,// 413 PAY 410 

    0xf841faf8,// 414 PAY 411 

    0xa575b813,// 415 PAY 412 

    0xde9cb361,// 416 PAY 413 

    0x8e06cb05,// 417 PAY 414 

    0xbda7f48e,// 418 PAY 415 

    0x760ee389,// 419 PAY 416 

    0xc79a8d53,// 420 PAY 417 

    0xbbf194f4,// 421 PAY 418 

    0x71eb9653,// 422 PAY 419 

    0x551d33de,// 423 PAY 420 

    0xc2b6d6c2,// 424 PAY 421 

    0xdfefd826,// 425 PAY 422 

    0xef6a2249,// 426 PAY 423 

    0xf93c8cfd,// 427 PAY 424 

    0xef87896a,// 428 PAY 425 

    0x96521116,// 429 PAY 426 

    0xaa464020,// 430 PAY 427 

    0xcbc99687,// 431 PAY 428 

    0xcbb19958,// 432 PAY 429 

    0xaabeadad,// 433 PAY 430 

    0x8167d71e,// 434 PAY 431 

    0x9aa89351,// 435 PAY 432 

    0x8a69f494,// 436 PAY 433 

    0x939c0237,// 437 PAY 434 

    0xe022edd1,// 438 PAY 435 

    0x4f9885c0,// 439 PAY 436 

    0x8de7e4e5,// 440 PAY 437 

    0x9b7de392,// 441 PAY 438 

    0x5297aa0a,// 442 PAY 439 

    0x91cd9128,// 443 PAY 440 

    0xa168e6ab,// 444 PAY 441 

    0xc90f86a6,// 445 PAY 442 

    0x052cb400,// 446 PAY 443 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0359e82c // 447 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt11_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 237 words. 

/// BDA size     is 944 (0x3b0) 

/// BDA id       is 0x906 

    0x03b00906,// 3 BDA   1 

/// PAY Generic Data size   : 944 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x42aae3b1,// 4 PAY   1 

    0x06b07671,// 5 PAY   2 

    0x395be0d0,// 6 PAY   3 

    0xd9c767ed,// 7 PAY   4 

    0xc5af405d,// 8 PAY   5 

    0x83cf9d39,// 9 PAY   6 

    0x42a05258,// 10 PAY   7 

    0x282a5173,// 11 PAY   8 

    0x2f05e363,// 12 PAY   9 

    0xd5d8b826,// 13 PAY  10 

    0xefa5b3a9,// 14 PAY  11 

    0x5439881f,// 15 PAY  12 

    0xa2a4a04d,// 16 PAY  13 

    0x2df15a8d,// 17 PAY  14 

    0xc412aded,// 18 PAY  15 

    0x396b1834,// 19 PAY  16 

    0xa98b8983,// 20 PAY  17 

    0x91a90c65,// 21 PAY  18 

    0x31deef7d,// 22 PAY  19 

    0xd1d83872,// 23 PAY  20 

    0x76fa618f,// 24 PAY  21 

    0x2c95bb84,// 25 PAY  22 

    0xc89aa69f,// 26 PAY  23 

    0x8a61714e,// 27 PAY  24 

    0x2fefcd71,// 28 PAY  25 

    0x9bf0f538,// 29 PAY  26 

    0x704d9448,// 30 PAY  27 

    0xa39588e0,// 31 PAY  28 

    0x6ce41ade,// 32 PAY  29 

    0x376a5df9,// 33 PAY  30 

    0x5e5d94bd,// 34 PAY  31 

    0x1ce27201,// 35 PAY  32 

    0xd5a397f4,// 36 PAY  33 

    0x96fc5b3d,// 37 PAY  34 

    0x9525ddf2,// 38 PAY  35 

    0xf8ac8976,// 39 PAY  36 

    0xc38eee16,// 40 PAY  37 

    0xa877d95d,// 41 PAY  38 

    0x774f793e,// 42 PAY  39 

    0xbd990600,// 43 PAY  40 

    0x857ee90c,// 44 PAY  41 

    0x96400d72,// 45 PAY  42 

    0x3895160f,// 46 PAY  43 

    0x5ceed0a3,// 47 PAY  44 

    0x599d7351,// 48 PAY  45 

    0x462a76b7,// 49 PAY  46 

    0xe29913c5,// 50 PAY  47 

    0xdd278eb1,// 51 PAY  48 

    0x1bf7c9ce,// 52 PAY  49 

    0x89332337,// 53 PAY  50 

    0xfb2b6289,// 54 PAY  51 

    0x6df67d91,// 55 PAY  52 

    0xe9ad4dad,// 56 PAY  53 

    0xf760c072,// 57 PAY  54 

    0x4ac22d10,// 58 PAY  55 

    0x2bb6db46,// 59 PAY  56 

    0xa07f1eaa,// 60 PAY  57 

    0x06f19169,// 61 PAY  58 

    0x2df1f7ea,// 62 PAY  59 

    0xb869c3b2,// 63 PAY  60 

    0x0574e19b,// 64 PAY  61 

    0xf4c4c15d,// 65 PAY  62 

    0xa5c324b7,// 66 PAY  63 

    0xf79df1d5,// 67 PAY  64 

    0xab72016d,// 68 PAY  65 

    0x89886373,// 69 PAY  66 

    0xc78be41a,// 70 PAY  67 

    0xd8a250c5,// 71 PAY  68 

    0x70a19c55,// 72 PAY  69 

    0x7c3e9082,// 73 PAY  70 

    0xc806bf25,// 74 PAY  71 

    0x879f5bd1,// 75 PAY  72 

    0xcbb8cbf8,// 76 PAY  73 

    0xbee0b3e2,// 77 PAY  74 

    0xb97110d3,// 78 PAY  75 

    0x85683dc9,// 79 PAY  76 

    0x47ba536f,// 80 PAY  77 

    0xc5d01ce7,// 81 PAY  78 

    0x201ee1cf,// 82 PAY  79 

    0x9e0d4a30,// 83 PAY  80 

    0xd65c4d19,// 84 PAY  81 

    0x9c2f4c62,// 85 PAY  82 

    0x3984aba9,// 86 PAY  83 

    0x6cccd8c6,// 87 PAY  84 

    0xac426f82,// 88 PAY  85 

    0x5c19a0ef,// 89 PAY  86 

    0x55881f32,// 90 PAY  87 

    0x4b303e48,// 91 PAY  88 

    0xbae0524f,// 92 PAY  89 

    0xbfb54870,// 93 PAY  90 

    0xfe17039e,// 94 PAY  91 

    0xbc447d99,// 95 PAY  92 

    0x08db73d0,// 96 PAY  93 

    0xb9258023,// 97 PAY  94 

    0x6510f808,// 98 PAY  95 

    0xc71a4c02,// 99 PAY  96 

    0xe8723eb7,// 100 PAY  97 

    0x1bb08d97,// 101 PAY  98 

    0x0c7a6041,// 102 PAY  99 

    0x1262009d,// 103 PAY 100 

    0x0204b019,// 104 PAY 101 

    0x451f6d51,// 105 PAY 102 

    0x377eda64,// 106 PAY 103 

    0x9e2f88d2,// 107 PAY 104 

    0xe59453ee,// 108 PAY 105 

    0xfeb0e7ef,// 109 PAY 106 

    0x110f8b09,// 110 PAY 107 

    0xa8b7bb16,// 111 PAY 108 

    0x11f63352,// 112 PAY 109 

    0xa9383954,// 113 PAY 110 

    0xc5ef4ffe,// 114 PAY 111 

    0xb2ae9827,// 115 PAY 112 

    0x79ed176b,// 116 PAY 113 

    0x023a5f24,// 117 PAY 114 

    0x552b9543,// 118 PAY 115 

    0x135b47a2,// 119 PAY 116 

    0x9bf5629a,// 120 PAY 117 

    0xb7b50596,// 121 PAY 118 

    0x0861ce5f,// 122 PAY 119 

    0xd20f2d67,// 123 PAY 120 

    0xbc6c9983,// 124 PAY 121 

    0x08e94ae3,// 125 PAY 122 

    0xec897e2e,// 126 PAY 123 

    0x0e427418,// 127 PAY 124 

    0x9848cb6e,// 128 PAY 125 

    0x2041b816,// 129 PAY 126 

    0xae5e9fad,// 130 PAY 127 

    0x92d05c92,// 131 PAY 128 

    0xbd671b19,// 132 PAY 129 

    0x2721eb93,// 133 PAY 130 

    0x652fc104,// 134 PAY 131 

    0xbcb7e9c9,// 135 PAY 132 

    0x975e2481,// 136 PAY 133 

    0xf8a0b110,// 137 PAY 134 

    0xd2449843,// 138 PAY 135 

    0xbe5e405e,// 139 PAY 136 

    0xb80d5385,// 140 PAY 137 

    0xc5b9bd46,// 141 PAY 138 

    0x6b20510a,// 142 PAY 139 

    0xf2102646,// 143 PAY 140 

    0x78130073,// 144 PAY 141 

    0x5a92905b,// 145 PAY 142 

    0x7edd514c,// 146 PAY 143 

    0x0e541810,// 147 PAY 144 

    0xe5951059,// 148 PAY 145 

    0x845a3b8f,// 149 PAY 146 

    0x2324ae6d,// 150 PAY 147 

    0x7ab3ce58,// 151 PAY 148 

    0xe1cef568,// 152 PAY 149 

    0x761e672b,// 153 PAY 150 

    0x4eec9242,// 154 PAY 151 

    0x6fcc4cbe,// 155 PAY 152 

    0xe6b78a91,// 156 PAY 153 

    0xaa148805,// 157 PAY 154 

    0xbd1c2152,// 158 PAY 155 

    0x79530c2e,// 159 PAY 156 

    0x18a7b469,// 160 PAY 157 

    0x112095b1,// 161 PAY 158 

    0x86ba7a8d,// 162 PAY 159 

    0xe434d2ac,// 163 PAY 160 

    0x05de5efe,// 164 PAY 161 

    0xbf5d29e4,// 165 PAY 162 

    0x9c37f80d,// 166 PAY 163 

    0xb2beb0f0,// 167 PAY 164 

    0x2b150e48,// 168 PAY 165 

    0x3d022be5,// 169 PAY 166 

    0x8b6da051,// 170 PAY 167 

    0x65addac5,// 171 PAY 168 

    0xbd2f3fc9,// 172 PAY 169 

    0x58234240,// 173 PAY 170 

    0xfbf07542,// 174 PAY 171 

    0x8b2ad535,// 175 PAY 172 

    0xf1e37216,// 176 PAY 173 

    0xc36ff7f8,// 177 PAY 174 

    0xb7b17df7,// 178 PAY 175 

    0x09b32011,// 179 PAY 176 

    0x5765cfcb,// 180 PAY 177 

    0x6f495cf1,// 181 PAY 178 

    0x216ed275,// 182 PAY 179 

    0x8d44e51e,// 183 PAY 180 

    0xd5d16cd9,// 184 PAY 181 

    0x8e4d501d,// 185 PAY 182 

    0xd550827a,// 186 PAY 183 

    0xe6b9efaa,// 187 PAY 184 

    0x96685eca,// 188 PAY 185 

    0xfdf3a3f4,// 189 PAY 186 

    0x4401056e,// 190 PAY 187 

    0x5e454022,// 191 PAY 188 

    0xa876e540,// 192 PAY 189 

    0xb7ed19ee,// 193 PAY 190 

    0x18470358,// 194 PAY 191 

    0x25caeb99,// 195 PAY 192 

    0x06934ef4,// 196 PAY 193 

    0xb8fadebb,// 197 PAY 194 

    0x27779292,// 198 PAY 195 

    0x866d9896,// 199 PAY 196 

    0x81c03aa5,// 200 PAY 197 

    0x09da4a78,// 201 PAY 198 

    0xb3427d9c,// 202 PAY 199 

    0xc6e5a3c2,// 203 PAY 200 

    0x743061ac,// 204 PAY 201 

    0x4ea492cb,// 205 PAY 202 

    0x76515bf1,// 206 PAY 203 

    0x2b8022d2,// 207 PAY 204 

    0x257034b2,// 208 PAY 205 

    0x422ac679,// 209 PAY 206 

    0x73c319ab,// 210 PAY 207 

    0x06b056c5,// 211 PAY 208 

    0xbfd1f217,// 212 PAY 209 

    0x4b83ca9b,// 213 PAY 210 

    0x3f8d53ae,// 214 PAY 211 

    0x881a5d53,// 215 PAY 212 

    0xd10c08ce,// 216 PAY 213 

    0x71ef1af1,// 217 PAY 214 

    0x07f9dbe8,// 218 PAY 215 

    0xaf6224de,// 219 PAY 216 

    0xdc23a71f,// 220 PAY 217 

    0xb7707bc2,// 221 PAY 218 

    0x331d27ec,// 222 PAY 219 

    0x71959409,// 223 PAY 220 

    0xb735aabc,// 224 PAY 221 

    0x677e2e93,// 225 PAY 222 

    0x5f5d0d6b,// 226 PAY 223 

    0xbe7d157c,// 227 PAY 224 

    0x79638599,// 228 PAY 225 

    0x8556dfcb,// 229 PAY 226 

    0xd9998262,// 230 PAY 227 

    0x10d0a3d4,// 231 PAY 228 

    0x157a6342,// 232 PAY 229 

    0xc8b81c36,// 233 PAY 230 

    0xbb31a9c2,// 234 PAY 231 

    0x5808d80a,// 235 PAY 232 

    0x529739a0,// 236 PAY 233 

    0x7a244752,// 237 PAY 234 

    0x04d480a5,// 238 PAY 235 

    0x55113da5,// 239 PAY 236 

/// STA is 1 words. 

/// STA num_pkts       : 10 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7c 

    0x03607c0a // 240 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt12_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 105 words. 

/// BDA size     is 416 (0x1a0) 

/// BDA id       is 0x79ee 

    0x01a079ee,// 3 BDA   1 

/// PAY Generic Data size   : 416 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xed947e23,// 4 PAY   1 

    0xb79a9962,// 5 PAY   2 

    0x0c5a032b,// 6 PAY   3 

    0x5baab447,// 7 PAY   4 

    0xc36590db,// 8 PAY   5 

    0x73113177,// 9 PAY   6 

    0xb10416fb,// 10 PAY   7 

    0xbf9fc6d0,// 11 PAY   8 

    0x808cf3aa,// 12 PAY   9 

    0x5550015a,// 13 PAY  10 

    0x026e279a,// 14 PAY  11 

    0xfd782964,// 15 PAY  12 

    0x600b2062,// 16 PAY  13 

    0x62029f3c,// 17 PAY  14 

    0x0606e129,// 18 PAY  15 

    0xeb2d2072,// 19 PAY  16 

    0x92e06a89,// 20 PAY  17 

    0xce8e3b85,// 21 PAY  18 

    0x7df85e0d,// 22 PAY  19 

    0x58669a7d,// 23 PAY  20 

    0xf2bfcef3,// 24 PAY  21 

    0xfaa90d8b,// 25 PAY  22 

    0x953730e1,// 26 PAY  23 

    0xda9a0bf4,// 27 PAY  24 

    0x61a90499,// 28 PAY  25 

    0xe119259f,// 29 PAY  26 

    0xb9cbb4be,// 30 PAY  27 

    0xf5be755f,// 31 PAY  28 

    0x5495d3c9,// 32 PAY  29 

    0xe7b6e5f1,// 33 PAY  30 

    0x7d40f1eb,// 34 PAY  31 

    0x66585086,// 35 PAY  32 

    0x8a2201fd,// 36 PAY  33 

    0xa41e4e08,// 37 PAY  34 

    0xde0e036c,// 38 PAY  35 

    0x1baf41dd,// 39 PAY  36 

    0x44d23c67,// 40 PAY  37 

    0xa8c95025,// 41 PAY  38 

    0x21c79168,// 42 PAY  39 

    0xbf1c3cf2,// 43 PAY  40 

    0x61441361,// 44 PAY  41 

    0xb4927c80,// 45 PAY  42 

    0xdc358011,// 46 PAY  43 

    0x0f868331,// 47 PAY  44 

    0xda77ee9a,// 48 PAY  45 

    0xb219e3d9,// 49 PAY  46 

    0x8ea79b13,// 50 PAY  47 

    0xe366d5c5,// 51 PAY  48 

    0x963aaffc,// 52 PAY  49 

    0xb31b4f72,// 53 PAY  50 

    0x74b7a0db,// 54 PAY  51 

    0xfe69f30d,// 55 PAY  52 

    0x0793ce4a,// 56 PAY  53 

    0x92f5def4,// 57 PAY  54 

    0xba482710,// 58 PAY  55 

    0x9ead9611,// 59 PAY  56 

    0x91e4c71d,// 60 PAY  57 

    0x0b70a1ef,// 61 PAY  58 

    0x4206dc5f,// 62 PAY  59 

    0xba0ee1ec,// 63 PAY  60 

    0xb8a7ae16,// 64 PAY  61 

    0x63b53395,// 65 PAY  62 

    0x9d47a002,// 66 PAY  63 

    0x0fce43cc,// 67 PAY  64 

    0xfad24334,// 68 PAY  65 

    0x72f9ae11,// 69 PAY  66 

    0x93bcf45d,// 70 PAY  67 

    0x6525f025,// 71 PAY  68 

    0x4d3a1b1b,// 72 PAY  69 

    0xdb8fbb7d,// 73 PAY  70 

    0xcde2cb8b,// 74 PAY  71 

    0x1c72cada,// 75 PAY  72 

    0x00e207ef,// 76 PAY  73 

    0x5cf690f8,// 77 PAY  74 

    0x19b5d421,// 78 PAY  75 

    0x7f8f2749,// 79 PAY  76 

    0xd2a6ffc8,// 80 PAY  77 

    0x3b2f9235,// 81 PAY  78 

    0xbcb58f5e,// 82 PAY  79 

    0x117217f5,// 83 PAY  80 

    0x2bb811ed,// 84 PAY  81 

    0x76b3f4f7,// 85 PAY  82 

    0x138d47ec,// 86 PAY  83 

    0x368332cf,// 87 PAY  84 

    0xb6cd391c,// 88 PAY  85 

    0xf2d47697,// 89 PAY  86 

    0x1a681e20,// 90 PAY  87 

    0x9e7e7ce5,// 91 PAY  88 

    0x4816fb2b,// 92 PAY  89 

    0x7c8ca375,// 93 PAY  90 

    0x515b15d6,// 94 PAY  91 

    0x788929c7,// 95 PAY  92 

    0xc3c436f3,// 96 PAY  93 

    0x241fb195,// 97 PAY  94 

    0xa69561e1,// 98 PAY  95 

    0x3b129a5d,// 99 PAY  96 

    0x3d41897e,// 100 PAY  97 

    0xc298d323,// 101 PAY  98 

    0x95f0521c,// 102 PAY  99 

    0x9f1ba543,// 103 PAY 100 

    0x30091cb4,// 104 PAY 101 

    0x17f21294,// 105 PAY 102 

    0xcfa75efe,// 106 PAY 103 

    0x931f1db6,// 107 PAY 104 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 70 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcf 

    0x0119cf74 // 108 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt13_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 343 words. 

/// BDA size     is 1367 (0x557) 

/// BDA id       is 0x91a 

    0x0557091a,// 3 BDA   1 

/// PAY Generic Data size   : 1367 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x844e346c,// 4 PAY   1 

    0x6f11c2d5,// 5 PAY   2 

    0x0b8f97b1,// 6 PAY   3 

    0xacaa12a8,// 7 PAY   4 

    0x37d22d51,// 8 PAY   5 

    0x48fa9b7c,// 9 PAY   6 

    0x69d304c3,// 10 PAY   7 

    0x8b927e26,// 11 PAY   8 

    0x74ac7dec,// 12 PAY   9 

    0xebd012b8,// 13 PAY  10 

    0x13ffe71a,// 14 PAY  11 

    0xacfddace,// 15 PAY  12 

    0x2528fe4e,// 16 PAY  13 

    0xc64828ef,// 17 PAY  14 

    0xbd93894d,// 18 PAY  15 

    0x47158067,// 19 PAY  16 

    0x0c8a3d35,// 20 PAY  17 

    0x9da1505c,// 21 PAY  18 

    0x2303e8f1,// 22 PAY  19 

    0x5ec1f636,// 23 PAY  20 

    0x16db887c,// 24 PAY  21 

    0xa7c6ea0b,// 25 PAY  22 

    0x99e84dfc,// 26 PAY  23 

    0x9e10daa0,// 27 PAY  24 

    0x6570f990,// 28 PAY  25 

    0xb1f97f28,// 29 PAY  26 

    0xa01f9d16,// 30 PAY  27 

    0x42a5742b,// 31 PAY  28 

    0xdd03139d,// 32 PAY  29 

    0x793d89f7,// 33 PAY  30 

    0x47c90fcf,// 34 PAY  31 

    0x5dc7367d,// 35 PAY  32 

    0x465f17f0,// 36 PAY  33 

    0x044bc444,// 37 PAY  34 

    0xf68cb8c8,// 38 PAY  35 

    0xc32cce52,// 39 PAY  36 

    0xa4815027,// 40 PAY  37 

    0x3edc1fcb,// 41 PAY  38 

    0xad8f5e8f,// 42 PAY  39 

    0xf0a63567,// 43 PAY  40 

    0xba288c68,// 44 PAY  41 

    0x88e44a6a,// 45 PAY  42 

    0xd06eed2c,// 46 PAY  43 

    0x566d7683,// 47 PAY  44 

    0x4dec8985,// 48 PAY  45 

    0xf9c9afe5,// 49 PAY  46 

    0xba334357,// 50 PAY  47 

    0xb0a0f709,// 51 PAY  48 

    0xda7b6898,// 52 PAY  49 

    0xe65ccdb4,// 53 PAY  50 

    0xaf0657fc,// 54 PAY  51 

    0xda92ebf2,// 55 PAY  52 

    0x19725407,// 56 PAY  53 

    0xcb0612df,// 57 PAY  54 

    0x2c8ba4fe,// 58 PAY  55 

    0xc5b510d2,// 59 PAY  56 

    0xc3a79d56,// 60 PAY  57 

    0x44ef94db,// 61 PAY  58 

    0x09bb792b,// 62 PAY  59 

    0x7f0b58ec,// 63 PAY  60 

    0x9e9b07e5,// 64 PAY  61 

    0x8ae5dead,// 65 PAY  62 

    0xfd22dce7,// 66 PAY  63 

    0x3b86f38b,// 67 PAY  64 

    0x405f696a,// 68 PAY  65 

    0x32ee9da7,// 69 PAY  66 

    0x63704d65,// 70 PAY  67 

    0xbcbbe77c,// 71 PAY  68 

    0xde02a755,// 72 PAY  69 

    0x5f8ff9fa,// 73 PAY  70 

    0xe6c43aba,// 74 PAY  71 

    0xd1ada1a3,// 75 PAY  72 

    0x06655e3c,// 76 PAY  73 

    0x1767c3cb,// 77 PAY  74 

    0xe5643cb4,// 78 PAY  75 

    0x432836bf,// 79 PAY  76 

    0x0fde2dd6,// 80 PAY  77 

    0xb67e81a3,// 81 PAY  78 

    0x8aef231b,// 82 PAY  79 

    0xc89327cd,// 83 PAY  80 

    0xba01588a,// 84 PAY  81 

    0x6e224f91,// 85 PAY  82 

    0xc11ca30d,// 86 PAY  83 

    0x2fc465d5,// 87 PAY  84 

    0x5abae0f2,// 88 PAY  85 

    0x39849d13,// 89 PAY  86 

    0x8bf6c9f8,// 90 PAY  87 

    0x72b3ebfa,// 91 PAY  88 

    0xf92b6c61,// 92 PAY  89 

    0xe483fb21,// 93 PAY  90 

    0xb7aad2f5,// 94 PAY  91 

    0xf7ce0cda,// 95 PAY  92 

    0xb4801488,// 96 PAY  93 

    0x0a40162e,// 97 PAY  94 

    0x5f89798e,// 98 PAY  95 

    0xcfb2cb18,// 99 PAY  96 

    0x754ff798,// 100 PAY  97 

    0x522089f7,// 101 PAY  98 

    0x69e9c062,// 102 PAY  99 

    0x3efbbde0,// 103 PAY 100 

    0x3d41cf3a,// 104 PAY 101 

    0x3dc714f9,// 105 PAY 102 

    0x569fa27b,// 106 PAY 103 

    0x4db31ead,// 107 PAY 104 

    0x972af9d1,// 108 PAY 105 

    0xafc5d8f8,// 109 PAY 106 

    0xaed71828,// 110 PAY 107 

    0xcacd5332,// 111 PAY 108 

    0x9e7dfe2f,// 112 PAY 109 

    0x5354420c,// 113 PAY 110 

    0xf69ec552,// 114 PAY 111 

    0x004cf47c,// 115 PAY 112 

    0xed60fc9e,// 116 PAY 113 

    0x4d6f1c90,// 117 PAY 114 

    0xd564a01c,// 118 PAY 115 

    0x1297c7ff,// 119 PAY 116 

    0x3af8bac3,// 120 PAY 117 

    0xecf2bb37,// 121 PAY 118 

    0xd29bd3ae,// 122 PAY 119 

    0x7bf82bb1,// 123 PAY 120 

    0xc68acb2a,// 124 PAY 121 

    0xb7f9fb2a,// 125 PAY 122 

    0x02c1dbc3,// 126 PAY 123 

    0xc1a17aaa,// 127 PAY 124 

    0x02cf69c0,// 128 PAY 125 

    0xd0cbf651,// 129 PAY 126 

    0x08a39516,// 130 PAY 127 

    0x4fa5ef39,// 131 PAY 128 

    0x9a0548fc,// 132 PAY 129 

    0xd8ccb66e,// 133 PAY 130 

    0x8f943043,// 134 PAY 131 

    0x4aa55e9e,// 135 PAY 132 

    0x4adc92fe,// 136 PAY 133 

    0xb4c01a90,// 137 PAY 134 

    0x04a6f9f1,// 138 PAY 135 

    0xd32517a2,// 139 PAY 136 

    0x2db90a73,// 140 PAY 137 

    0x86a2c254,// 141 PAY 138 

    0x0209f6ca,// 142 PAY 139 

    0x307a0e90,// 143 PAY 140 

    0xa0e6027b,// 144 PAY 141 

    0x61aa98f6,// 145 PAY 142 

    0xc9626522,// 146 PAY 143 

    0x6f7a3bb3,// 147 PAY 144 

    0x9a0c5b78,// 148 PAY 145 

    0x40392ce5,// 149 PAY 146 

    0x34e546ad,// 150 PAY 147 

    0x479c5475,// 151 PAY 148 

    0x54341425,// 152 PAY 149 

    0x5b50eb6e,// 153 PAY 150 

    0xef523500,// 154 PAY 151 

    0x08ab4d18,// 155 PAY 152 

    0x142cd672,// 156 PAY 153 

    0x86198e61,// 157 PAY 154 

    0x9ee447a0,// 158 PAY 155 

    0xb7d70b85,// 159 PAY 156 

    0x73b2e6ae,// 160 PAY 157 

    0x125311d3,// 161 PAY 158 

    0x8b28870e,// 162 PAY 159 

    0x03f60529,// 163 PAY 160 

    0xf0e3b7b1,// 164 PAY 161 

    0x3ece3c66,// 165 PAY 162 

    0xe4bee10b,// 166 PAY 163 

    0x142e46d8,// 167 PAY 164 

    0x37529c4f,// 168 PAY 165 

    0x050dd6e3,// 169 PAY 166 

    0x1735e3e2,// 170 PAY 167 

    0x10eb5f63,// 171 PAY 168 

    0xeb5e4832,// 172 PAY 169 

    0x241a9391,// 173 PAY 170 

    0xfb269ab6,// 174 PAY 171 

    0xf13b3ca4,// 175 PAY 172 

    0xfc483e17,// 176 PAY 173 

    0xbc71b6aa,// 177 PAY 174 

    0xe1460a7e,// 178 PAY 175 

    0xaa1d9792,// 179 PAY 176 

    0x73ec58ac,// 180 PAY 177 

    0x19a89c0e,// 181 PAY 178 

    0x6b4e1e50,// 182 PAY 179 

    0x39d39973,// 183 PAY 180 

    0xe208f312,// 184 PAY 181 

    0x9683188c,// 185 PAY 182 

    0x6f88ded0,// 186 PAY 183 

    0xd9018ee9,// 187 PAY 184 

    0xe5bbd3d0,// 188 PAY 185 

    0x840aaf9c,// 189 PAY 186 

    0x4b00e62e,// 190 PAY 187 

    0xa681a542,// 191 PAY 188 

    0x4d69bd88,// 192 PAY 189 

    0xe3e3034e,// 193 PAY 190 

    0x7a9b06f5,// 194 PAY 191 

    0x17bb2a09,// 195 PAY 192 

    0x20a8b5eb,// 196 PAY 193 

    0x6b0353b2,// 197 PAY 194 

    0xb36be11c,// 198 PAY 195 

    0x35cd81ab,// 199 PAY 196 

    0x94274cfb,// 200 PAY 197 

    0xe5f14ef5,// 201 PAY 198 

    0xbbbcba6d,// 202 PAY 199 

    0xbd64bccb,// 203 PAY 200 

    0x1e2e3138,// 204 PAY 201 

    0xa0346644,// 205 PAY 202 

    0xd2d08965,// 206 PAY 203 

    0x447379cb,// 207 PAY 204 

    0xd30d15e3,// 208 PAY 205 

    0x2c8cff8f,// 209 PAY 206 

    0x48bf6d73,// 210 PAY 207 

    0x538c4167,// 211 PAY 208 

    0x1a2ea4d7,// 212 PAY 209 

    0x262c299c,// 213 PAY 210 

    0x361ccf8e,// 214 PAY 211 

    0xe44ae1f0,// 215 PAY 212 

    0x718b4c0f,// 216 PAY 213 

    0xbb15b0ec,// 217 PAY 214 

    0x5e2891ee,// 218 PAY 215 

    0x192f4317,// 219 PAY 216 

    0xfe4e3af2,// 220 PAY 217 

    0x4aeac021,// 221 PAY 218 

    0x8034b89e,// 222 PAY 219 

    0x34980b59,// 223 PAY 220 

    0xcb20a473,// 224 PAY 221 

    0x06c2a34d,// 225 PAY 222 

    0x84718985,// 226 PAY 223 

    0xc3179ade,// 227 PAY 224 

    0x252273d9,// 228 PAY 225 

    0x0b7d8562,// 229 PAY 226 

    0xb706891d,// 230 PAY 227 

    0x935dd901,// 231 PAY 228 

    0x8a160c19,// 232 PAY 229 

    0xeadb2e4d,// 233 PAY 230 

    0xf3ecb769,// 234 PAY 231 

    0x9b418276,// 235 PAY 232 

    0xf05569f4,// 236 PAY 233 

    0xa0aa21da,// 237 PAY 234 

    0xfb33c0e8,// 238 PAY 235 

    0x64929e17,// 239 PAY 236 

    0x2f9a7a86,// 240 PAY 237 

    0x0d77ba15,// 241 PAY 238 

    0x523e5ab6,// 242 PAY 239 

    0x34131e4a,// 243 PAY 240 

    0xd7a06768,// 244 PAY 241 

    0x84d842a6,// 245 PAY 242 

    0x61a6d424,// 246 PAY 243 

    0x197fe65e,// 247 PAY 244 

    0x67a5d293,// 248 PAY 245 

    0x5102fb87,// 249 PAY 246 

    0x2641c8c2,// 250 PAY 247 

    0x3594851d,// 251 PAY 248 

    0xaa1f7580,// 252 PAY 249 

    0x8b8d6a20,// 253 PAY 250 

    0x330709e2,// 254 PAY 251 

    0x7ec66444,// 255 PAY 252 

    0xaec5c08c,// 256 PAY 253 

    0xeb94e212,// 257 PAY 254 

    0x0c3d4c5c,// 258 PAY 255 

    0x1b62a097,// 259 PAY 256 

    0x61231e36,// 260 PAY 257 

    0xd2792b13,// 261 PAY 258 

    0x29269a5a,// 262 PAY 259 

    0x958ec994,// 263 PAY 260 

    0xc4667990,// 264 PAY 261 

    0x98226a60,// 265 PAY 262 

    0xe9ed4c49,// 266 PAY 263 

    0x1282db3f,// 267 PAY 264 

    0x9e9954ed,// 268 PAY 265 

    0xc82c4eba,// 269 PAY 266 

    0xac16cc85,// 270 PAY 267 

    0xd225d49b,// 271 PAY 268 

    0xa18dd500,// 272 PAY 269 

    0xa44a1fc7,// 273 PAY 270 

    0xb7622fe4,// 274 PAY 271 

    0x6644188b,// 275 PAY 272 

    0x817dc966,// 276 PAY 273 

    0xdb06c600,// 277 PAY 274 

    0xe5d2a32d,// 278 PAY 275 

    0xc3e9ae63,// 279 PAY 276 

    0x8ef54ac4,// 280 PAY 277 

    0xe7f974a6,// 281 PAY 278 

    0x7a464466,// 282 PAY 279 

    0x4bd4160a,// 283 PAY 280 

    0x328de852,// 284 PAY 281 

    0xd85121ca,// 285 PAY 282 

    0x2df50739,// 286 PAY 283 

    0x92adabf4,// 287 PAY 284 

    0x54226027,// 288 PAY 285 

    0x2d7a38c6,// 289 PAY 286 

    0x80dcf50b,// 290 PAY 287 

    0xbb8714de,// 291 PAY 288 

    0x4e2da754,// 292 PAY 289 

    0x5acf03b5,// 293 PAY 290 

    0x678a0649,// 294 PAY 291 

    0x08268272,// 295 PAY 292 

    0x70164c02,// 296 PAY 293 

    0x981a5e5b,// 297 PAY 294 

    0x594f66f5,// 298 PAY 295 

    0xd7ea5165,// 299 PAY 296 

    0xa38f4f97,// 300 PAY 297 

    0x42661336,// 301 PAY 298 

    0x428f2cd6,// 302 PAY 299 

    0x4f23b7dd,// 303 PAY 300 

    0x30084e7a,// 304 PAY 301 

    0xdd85c86a,// 305 PAY 302 

    0xfb6e7ec8,// 306 PAY 303 

    0xbb52c2e4,// 307 PAY 304 

    0x3ef0c338,// 308 PAY 305 

    0xd83f0352,// 309 PAY 306 

    0x229996c4,// 310 PAY 307 

    0x70b571a4,// 311 PAY 308 

    0xdcd2cf0e,// 312 PAY 309 

    0xd51a5901,// 313 PAY 310 

    0xbbf8bc50,// 314 PAY 311 

    0x7f0cd6bb,// 315 PAY 312 

    0x5e85a35b,// 316 PAY 313 

    0x032dd1fd,// 317 PAY 314 

    0x25990961,// 318 PAY 315 

    0xa0b48781,// 319 PAY 316 

    0xa3e8885e,// 320 PAY 317 

    0xf8289dec,// 321 PAY 318 

    0x9650b538,// 322 PAY 319 

    0xe7a72fd0,// 323 PAY 320 

    0xfadb152e,// 324 PAY 321 

    0xbe694d87,// 325 PAY 322 

    0x1d188dd9,// 326 PAY 323 

    0x9a9914e1,// 327 PAY 324 

    0x117ef496,// 328 PAY 325 

    0x233b9c3c,// 329 PAY 326 

    0xae317eef,// 330 PAY 327 

    0x3dc9c92b,// 331 PAY 328 

    0xa0d83c48,// 332 PAY 329 

    0xe908c7c3,// 333 PAY 330 

    0x4d88425e,// 334 PAY 331 

    0xeba3a077,// 335 PAY 332 

    0x2a4e5492,// 336 PAY 333 

    0x5fbf28ba,// 337 PAY 334 

    0x54448224,// 338 PAY 335 

    0x09e17455,// 339 PAY 336 

    0x44299189,// 340 PAY 337 

    0x19575ca7,// 341 PAY 338 

    0xa8bc1dd4,// 342 PAY 339 

    0x43f896c5,// 343 PAY 340 

    0x1073ee4c,// 344 PAY 341 

    0x78bb2f00,// 345 PAY 342 

/// HASH is  16 bytes 

    0x8a160c19,// 346 HSH   1 

    0xeadb2e4d,// 347 HSH   2 

    0xf3ecb769,// 348 HSH   3 

    0x9b418276,// 349 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 168 

/// STA pkt_idx        : 182 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3e 

    0x02d93ea8 // 350 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt14_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 128 words. 

/// BDA size     is 505 (0x1f9) 

/// BDA id       is 0x241b 

    0x01f9241b,// 3 BDA   1 

/// PAY Generic Data size   : 505 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xbebc2e5a,// 4 PAY   1 

    0xcc046ae1,// 5 PAY   2 

    0x78e63a3d,// 6 PAY   3 

    0xf664e13d,// 7 PAY   4 

    0x0c59d8b2,// 8 PAY   5 

    0x018e0cfd,// 9 PAY   6 

    0xe799dbbe,// 10 PAY   7 

    0x7d217a20,// 11 PAY   8 

    0x3619da86,// 12 PAY   9 

    0xd627eb9e,// 13 PAY  10 

    0xfd81013a,// 14 PAY  11 

    0x66a1e464,// 15 PAY  12 

    0x99efac22,// 16 PAY  13 

    0x2d569df4,// 17 PAY  14 

    0x0cbce186,// 18 PAY  15 

    0xb2b863c0,// 19 PAY  16 

    0xe84a5912,// 20 PAY  17 

    0xaf9d41ff,// 21 PAY  18 

    0xc26e1a0d,// 22 PAY  19 

    0x426658a1,// 23 PAY  20 

    0x7a2be684,// 24 PAY  21 

    0xd98c9aa7,// 25 PAY  22 

    0xb72b5025,// 26 PAY  23 

    0xfa4781b0,// 27 PAY  24 

    0x9dc61ad8,// 28 PAY  25 

    0x2d7fc9fc,// 29 PAY  26 

    0xe28fe15e,// 30 PAY  27 

    0xf6f9a70e,// 31 PAY  28 

    0x5f1dc8e5,// 32 PAY  29 

    0x66116c35,// 33 PAY  30 

    0xba864d42,// 34 PAY  31 

    0x65553dee,// 35 PAY  32 

    0x1173d07e,// 36 PAY  33 

    0x44b9dae3,// 37 PAY  34 

    0x672f72a0,// 38 PAY  35 

    0x2c399fd9,// 39 PAY  36 

    0x5b4ebd20,// 40 PAY  37 

    0x3c754b79,// 41 PAY  38 

    0xe5f3d27e,// 42 PAY  39 

    0x93a98183,// 43 PAY  40 

    0x2c4be2ba,// 44 PAY  41 

    0x0e4f380f,// 45 PAY  42 

    0x6eaacadf,// 46 PAY  43 

    0xdec57d6a,// 47 PAY  44 

    0xecfd2f7e,// 48 PAY  45 

    0x4a4712d8,// 49 PAY  46 

    0x23f9880b,// 50 PAY  47 

    0x6f3c0313,// 51 PAY  48 

    0xaaabe1b4,// 52 PAY  49 

    0x9c9a6765,// 53 PAY  50 

    0x9b6ea08e,// 54 PAY  51 

    0x082c713c,// 55 PAY  52 

    0xb6ca2859,// 56 PAY  53 

    0xadc41232,// 57 PAY  54 

    0x832c8884,// 58 PAY  55 

    0x64d3dadc,// 59 PAY  56 

    0xb1f6c0e4,// 60 PAY  57 

    0xfc130bb3,// 61 PAY  58 

    0x85dd1cf7,// 62 PAY  59 

    0x72811996,// 63 PAY  60 

    0xb53c8f85,// 64 PAY  61 

    0xf32c3028,// 65 PAY  62 

    0x02b5bb41,// 66 PAY  63 

    0x9e9f249e,// 67 PAY  64 

    0xeb0764ba,// 68 PAY  65 

    0x7f22b5ef,// 69 PAY  66 

    0xb0801143,// 70 PAY  67 

    0x04a7f6dc,// 71 PAY  68 

    0xafeb6f98,// 72 PAY  69 

    0x3696e3b1,// 73 PAY  70 

    0xcb50e85f,// 74 PAY  71 

    0x56f06b2a,// 75 PAY  72 

    0x226b8691,// 76 PAY  73 

    0xa5c7b731,// 77 PAY  74 

    0xef86c773,// 78 PAY  75 

    0x222bb5f5,// 79 PAY  76 

    0x54a730ef,// 80 PAY  77 

    0xe446cda7,// 81 PAY  78 

    0x52c762b0,// 82 PAY  79 

    0x3c642779,// 83 PAY  80 

    0xcba79440,// 84 PAY  81 

    0x3a704336,// 85 PAY  82 

    0xc8b6c901,// 86 PAY  83 

    0xd9c21c3c,// 87 PAY  84 

    0x0857ec42,// 88 PAY  85 

    0xc629c9b1,// 89 PAY  86 

    0x494c58d4,// 90 PAY  87 

    0x27a3b743,// 91 PAY  88 

    0x59cef63d,// 92 PAY  89 

    0x8deacbf7,// 93 PAY  90 

    0x5dfad799,// 94 PAY  91 

    0xc2dfb7de,// 95 PAY  92 

    0xe07c65c7,// 96 PAY  93 

    0xc80cd6e7,// 97 PAY  94 

    0x13ae3ad5,// 98 PAY  95 

    0x28c2c728,// 99 PAY  96 

    0xeee04328,// 100 PAY  97 

    0x6501c1ce,// 101 PAY  98 

    0xc45cb21f,// 102 PAY  99 

    0x37587be7,// 103 PAY 100 

    0x8208d06f,// 104 PAY 101 

    0x0a8eab3f,// 105 PAY 102 

    0x4701e407,// 106 PAY 103 

    0x61cd5faf,// 107 PAY 104 

    0x9cb14d83,// 108 PAY 105 

    0x5cc7243f,// 109 PAY 106 

    0x471c2a37,// 110 PAY 107 

    0x19d82f90,// 111 PAY 108 

    0x3d5f1b92,// 112 PAY 109 

    0x7f0b1952,// 113 PAY 110 

    0x24917db8,// 114 PAY 111 

    0x43d3e209,// 115 PAY 112 

    0x286a2343,// 116 PAY 113 

    0x1f95f9d8,// 117 PAY 114 

    0xa4e08fe3,// 118 PAY 115 

    0x6d9df744,// 119 PAY 116 

    0x6f72c9fe,// 120 PAY 117 

    0x2731d023,// 121 PAY 118 

    0xae23b2a1,// 122 PAY 119 

    0x1d92846f,// 123 PAY 120 

    0x0e84f3e6,// 124 PAY 121 

    0xe47cb13b,// 125 PAY 122 

    0x6f5ef634,// 126 PAY 123 

    0x40836755,// 127 PAY 124 

    0xca0810b2,// 128 PAY 125 

    0x312f2e6f,// 129 PAY 126 

    0xa8000000,// 130 PAY 127 

/// HASH is  12 bytes 

    0xef86c773,// 131 HSH   1 

    0x222bb5f5,// 132 HSH   2 

    0x54a730ef,// 133 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 132 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x26 

    0x01f12684 // 134 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt15_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 354 words. 

/// BDA size     is 1410 (0x582) 

/// BDA id       is 0xa24 

    0x05820a24,// 3 BDA   1 

/// PAY Generic Data size   : 1410 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xd427529a,// 4 PAY   1 

    0xf83e0f0e,// 5 PAY   2 

    0x69d29a24,// 6 PAY   3 

    0x655cbdd7,// 7 PAY   4 

    0x3095ee69,// 8 PAY   5 

    0x48a392a7,// 9 PAY   6 

    0xcabaac65,// 10 PAY   7 

    0x64c343b8,// 11 PAY   8 

    0xfacf2976,// 12 PAY   9 

    0x0d4ee041,// 13 PAY  10 

    0xe59ffbe1,// 14 PAY  11 

    0x8d750442,// 15 PAY  12 

    0xe7c65a76,// 16 PAY  13 

    0xec71bcc3,// 17 PAY  14 

    0x6ebef965,// 18 PAY  15 

    0xcaf6d8f5,// 19 PAY  16 

    0xc3625f25,// 20 PAY  17 

    0x15c8c488,// 21 PAY  18 

    0xe90b662b,// 22 PAY  19 

    0x8ab4dfca,// 23 PAY  20 

    0x3cec9618,// 24 PAY  21 

    0x3f6f0af7,// 25 PAY  22 

    0x8fec12be,// 26 PAY  23 

    0xccc8bb0b,// 27 PAY  24 

    0x404ce64d,// 28 PAY  25 

    0x2b037e12,// 29 PAY  26 

    0x254ab6f6,// 30 PAY  27 

    0x46ea25e6,// 31 PAY  28 

    0xad64eb55,// 32 PAY  29 

    0x4a74e9fb,// 33 PAY  30 

    0x19216ce3,// 34 PAY  31 

    0x2a59280c,// 35 PAY  32 

    0x87a4d5b5,// 36 PAY  33 

    0x0c8602db,// 37 PAY  34 

    0x43f7a5b9,// 38 PAY  35 

    0xb1726816,// 39 PAY  36 

    0x4b8a5020,// 40 PAY  37 

    0x3ff75de2,// 41 PAY  38 

    0xd8c83726,// 42 PAY  39 

    0x49dda4c2,// 43 PAY  40 

    0x83e27fcf,// 44 PAY  41 

    0x2800dd1a,// 45 PAY  42 

    0x92ddf78b,// 46 PAY  43 

    0x0a944ca4,// 47 PAY  44 

    0x583db4ce,// 48 PAY  45 

    0xf61331b7,// 49 PAY  46 

    0x86fe966e,// 50 PAY  47 

    0x9cfc09a0,// 51 PAY  48 

    0xb05c4aa8,// 52 PAY  49 

    0x3d6ef679,// 53 PAY  50 

    0xf0ff747e,// 54 PAY  51 

    0xd886d34b,// 55 PAY  52 

    0x099906ac,// 56 PAY  53 

    0x979a2504,// 57 PAY  54 

    0x018419ca,// 58 PAY  55 

    0x5f0313be,// 59 PAY  56 

    0x60fa39ed,// 60 PAY  57 

    0x468654f0,// 61 PAY  58 

    0xd41ccf1a,// 62 PAY  59 

    0x275fc545,// 63 PAY  60 

    0xdaac7016,// 64 PAY  61 

    0x6543429d,// 65 PAY  62 

    0x527ab93a,// 66 PAY  63 

    0x841f867a,// 67 PAY  64 

    0x6ace82e9,// 68 PAY  65 

    0x71c97049,// 69 PAY  66 

    0xc2308aa1,// 70 PAY  67 

    0xdef2a769,// 71 PAY  68 

    0xf768e050,// 72 PAY  69 

    0x89b08e13,// 73 PAY  70 

    0x07897d61,// 74 PAY  71 

    0xf0d90582,// 75 PAY  72 

    0x09a8959c,// 76 PAY  73 

    0x48e6dcbe,// 77 PAY  74 

    0xf4bd2a4b,// 78 PAY  75 

    0xec882cbf,// 79 PAY  76 

    0x330711d7,// 80 PAY  77 

    0x93a29386,// 81 PAY  78 

    0x34944943,// 82 PAY  79 

    0xd3eb792d,// 83 PAY  80 

    0xec64e611,// 84 PAY  81 

    0x8fd856e7,// 85 PAY  82 

    0x6c0f280e,// 86 PAY  83 

    0x62e04de8,// 87 PAY  84 

    0x938b8971,// 88 PAY  85 

    0x1d973874,// 89 PAY  86 

    0x3e678ff8,// 90 PAY  87 

    0xd102c596,// 91 PAY  88 

    0xc34a82ed,// 92 PAY  89 

    0xf518cbb3,// 93 PAY  90 

    0xd7d01058,// 94 PAY  91 

    0x83f958c8,// 95 PAY  92 

    0x378599b6,// 96 PAY  93 

    0x4b3dd9f0,// 97 PAY  94 

    0x37a5719c,// 98 PAY  95 

    0x68f29a39,// 99 PAY  96 

    0xbbf1a8ce,// 100 PAY  97 

    0x61cfe5a9,// 101 PAY  98 

    0xb40cfffa,// 102 PAY  99 

    0xc5e5f269,// 103 PAY 100 

    0x10ed55c2,// 104 PAY 101 

    0xc19be824,// 105 PAY 102 

    0x9b45cf8a,// 106 PAY 103 

    0xce98d0b3,// 107 PAY 104 

    0x35a39f85,// 108 PAY 105 

    0x8b11e32e,// 109 PAY 106 

    0x680f7a61,// 110 PAY 107 

    0xd9520a00,// 111 PAY 108 

    0xba51bdf6,// 112 PAY 109 

    0x17e73c1b,// 113 PAY 110 

    0xa740235b,// 114 PAY 111 

    0x22a1eb51,// 115 PAY 112 

    0x83e98b8f,// 116 PAY 113 

    0x5686db59,// 117 PAY 114 

    0xbcc42a68,// 118 PAY 115 

    0x8d52e030,// 119 PAY 116 

    0x12599bad,// 120 PAY 117 

    0x64884aca,// 121 PAY 118 

    0x85f6a283,// 122 PAY 119 

    0x73401252,// 123 PAY 120 

    0x9313099c,// 124 PAY 121 

    0xb8e00dc8,// 125 PAY 122 

    0x330a9d41,// 126 PAY 123 

    0x381a1358,// 127 PAY 124 

    0x31e3a22e,// 128 PAY 125 

    0x86779920,// 129 PAY 126 

    0xb7a9157d,// 130 PAY 127 

    0x0466f501,// 131 PAY 128 

    0x41c46b52,// 132 PAY 129 

    0xb3c909c7,// 133 PAY 130 

    0x4048ac4a,// 134 PAY 131 

    0x2b06f7fc,// 135 PAY 132 

    0x59663460,// 136 PAY 133 

    0xbfd96d97,// 137 PAY 134 

    0x3825c1b2,// 138 PAY 135 

    0x197a6b4c,// 139 PAY 136 

    0x07ed6160,// 140 PAY 137 

    0xac72048e,// 141 PAY 138 

    0x414ad589,// 142 PAY 139 

    0xa29eea4c,// 143 PAY 140 

    0xfc098f45,// 144 PAY 141 

    0x9e3445af,// 145 PAY 142 

    0x5d994404,// 146 PAY 143 

    0x3de07f07,// 147 PAY 144 

    0xe13ea340,// 148 PAY 145 

    0xde6cce38,// 149 PAY 146 

    0x16a8c05f,// 150 PAY 147 

    0xdd565888,// 151 PAY 148 

    0x814f3160,// 152 PAY 149 

    0x3f5b0ded,// 153 PAY 150 

    0x6a97b879,// 154 PAY 151 

    0x3d2c4d75,// 155 PAY 152 

    0x672a82ba,// 156 PAY 153 

    0x1c2c50b5,// 157 PAY 154 

    0x03b13766,// 158 PAY 155 

    0xbb748652,// 159 PAY 156 

    0x1a8884a0,// 160 PAY 157 

    0x611a7d5c,// 161 PAY 158 

    0x7060f5f9,// 162 PAY 159 

    0xe05076fa,// 163 PAY 160 

    0x5c19ee12,// 164 PAY 161 

    0x3fe2ff59,// 165 PAY 162 

    0xf34ff17e,// 166 PAY 163 

    0x408c9e2e,// 167 PAY 164 

    0xa7d03290,// 168 PAY 165 

    0xe0959904,// 169 PAY 166 

    0xdc99d830,// 170 PAY 167 

    0x4f681c9f,// 171 PAY 168 

    0xcbe92e8f,// 172 PAY 169 

    0x4f0dd563,// 173 PAY 170 

    0x6ddf99e1,// 174 PAY 171 

    0x3e59ff73,// 175 PAY 172 

    0x0c666ca7,// 176 PAY 173 

    0x76b70094,// 177 PAY 174 

    0x970b5b0b,// 178 PAY 175 

    0x12376b0f,// 179 PAY 176 

    0xd7ec1056,// 180 PAY 177 

    0xe8b793b3,// 181 PAY 178 

    0xcb9a2a58,// 182 PAY 179 

    0x27c04dc6,// 183 PAY 180 

    0x03be4bec,// 184 PAY 181 

    0x55653bf2,// 185 PAY 182 

    0xa65bd021,// 186 PAY 183 

    0x8444fb5f,// 187 PAY 184 

    0x4b28b659,// 188 PAY 185 

    0x1da0922e,// 189 PAY 186 

    0xf26275e9,// 190 PAY 187 

    0xf0acca6f,// 191 PAY 188 

    0x0a1c4f6e,// 192 PAY 189 

    0xa0d3dee6,// 193 PAY 190 

    0x6eda49f9,// 194 PAY 191 

    0xb79eebae,// 195 PAY 192 

    0x19bf8fcb,// 196 PAY 193 

    0x7ec2439d,// 197 PAY 194 

    0x972237e5,// 198 PAY 195 

    0x85145c73,// 199 PAY 196 

    0xe96d8b98,// 200 PAY 197 

    0xe4c85e95,// 201 PAY 198 

    0x02eb8511,// 202 PAY 199 

    0xc5e5b2cd,// 203 PAY 200 

    0xa94419eb,// 204 PAY 201 

    0x87ccb473,// 205 PAY 202 

    0x5207a401,// 206 PAY 203 

    0x42a3f112,// 207 PAY 204 

    0xae01bb76,// 208 PAY 205 

    0x2c9a20d6,// 209 PAY 206 

    0xed756fe9,// 210 PAY 207 

    0x5012d7c7,// 211 PAY 208 

    0x88ab2f4c,// 212 PAY 209 

    0xd4ca0707,// 213 PAY 210 

    0x96f52fe8,// 214 PAY 211 

    0x596103d7,// 215 PAY 212 

    0xb2ee6cd5,// 216 PAY 213 

    0x67e5a0b9,// 217 PAY 214 

    0x85406ab4,// 218 PAY 215 

    0xb7275d88,// 219 PAY 216 

    0x3d76fe21,// 220 PAY 217 

    0x43e0b4e7,// 221 PAY 218 

    0x1717e371,// 222 PAY 219 

    0x992a7758,// 223 PAY 220 

    0xc8bff934,// 224 PAY 221 

    0x37c4a577,// 225 PAY 222 

    0x26812f9a,// 226 PAY 223 

    0xc84ce29a,// 227 PAY 224 

    0x9c70bc0e,// 228 PAY 225 

    0x4488baa7,// 229 PAY 226 

    0xdf9304a5,// 230 PAY 227 

    0x20a2112a,// 231 PAY 228 

    0x55808236,// 232 PAY 229 

    0xb1f8a586,// 233 PAY 230 

    0x1cbd7ff7,// 234 PAY 231 

    0x071ae678,// 235 PAY 232 

    0xda234ace,// 236 PAY 233 

    0x2022789b,// 237 PAY 234 

    0x736fa92f,// 238 PAY 235 

    0x8e36475f,// 239 PAY 236 

    0xe2a3a79e,// 240 PAY 237 

    0x9adbbfc3,// 241 PAY 238 

    0x1626de70,// 242 PAY 239 

    0xa480805e,// 243 PAY 240 

    0x2dca6680,// 244 PAY 241 

    0x934dc46d,// 245 PAY 242 

    0x92eaec8e,// 246 PAY 243 

    0x351bcfb3,// 247 PAY 244 

    0x4d68f9aa,// 248 PAY 245 

    0x95fca2df,// 249 PAY 246 

    0x2e2e9d19,// 250 PAY 247 

    0x2e92cae0,// 251 PAY 248 

    0xd908ddca,// 252 PAY 249 

    0x430ac730,// 253 PAY 250 

    0xd3332444,// 254 PAY 251 

    0x85282262,// 255 PAY 252 

    0xf4051d72,// 256 PAY 253 

    0x3e0872c4,// 257 PAY 254 

    0xfaf0d49f,// 258 PAY 255 

    0x67726062,// 259 PAY 256 

    0xeca482a8,// 260 PAY 257 

    0x47aef03a,// 261 PAY 258 

    0xd882a59e,// 262 PAY 259 

    0x5f5f9c8b,// 263 PAY 260 

    0x95f3d1a1,// 264 PAY 261 

    0xfce808a8,// 265 PAY 262 

    0x934f6e0d,// 266 PAY 263 

    0xdea22300,// 267 PAY 264 

    0x65f5f8d8,// 268 PAY 265 

    0x411fbe28,// 269 PAY 266 

    0xd514933d,// 270 PAY 267 

    0x61007c9e,// 271 PAY 268 

    0xfa4373dc,// 272 PAY 269 

    0x6712531b,// 273 PAY 270 

    0x2e1ab091,// 274 PAY 271 

    0xe342e03a,// 275 PAY 272 

    0xba8c3713,// 276 PAY 273 

    0x418c52d4,// 277 PAY 274 

    0xd60898d1,// 278 PAY 275 

    0x8182a67c,// 279 PAY 276 

    0x6c78a494,// 280 PAY 277 

    0x65bf36ff,// 281 PAY 278 

    0x48aefdfd,// 282 PAY 279 

    0x2146eddb,// 283 PAY 280 

    0xc4e65fb0,// 284 PAY 281 

    0xbcff692e,// 285 PAY 282 

    0x268788ab,// 286 PAY 283 

    0xeed82f2f,// 287 PAY 284 

    0x1a551e4d,// 288 PAY 285 

    0xabb84eb7,// 289 PAY 286 

    0xd0d11b3d,// 290 PAY 287 

    0xc5c57d96,// 291 PAY 288 

    0x080e3c74,// 292 PAY 289 

    0xcbc46fbb,// 293 PAY 290 

    0x485385e8,// 294 PAY 291 

    0x1a8f3bd2,// 295 PAY 292 

    0x2fe9f5ce,// 296 PAY 293 

    0xaa7cbf16,// 297 PAY 294 

    0x32efcc51,// 298 PAY 295 

    0x864b274b,// 299 PAY 296 

    0xbb4b808c,// 300 PAY 297 

    0xee77e78c,// 301 PAY 298 

    0xc016f331,// 302 PAY 299 

    0xccacaeac,// 303 PAY 300 

    0x38845d1e,// 304 PAY 301 

    0x50e608a7,// 305 PAY 302 

    0x5daef07b,// 306 PAY 303 

    0xc79b7880,// 307 PAY 304 

    0x83601c74,// 308 PAY 305 

    0xb8394a90,// 309 PAY 306 

    0xe440ed5a,// 310 PAY 307 

    0x8772bc53,// 311 PAY 308 

    0x6dbf306a,// 312 PAY 309 

    0xa08a3160,// 313 PAY 310 

    0x0f5bc0bd,// 314 PAY 311 

    0x69882ecf,// 315 PAY 312 

    0x8bc8930a,// 316 PAY 313 

    0x62e73e7c,// 317 PAY 314 

    0x51462a73,// 318 PAY 315 

    0xab0cf552,// 319 PAY 316 

    0x2a6ecfb9,// 320 PAY 317 

    0x12f556be,// 321 PAY 318 

    0xac222cb3,// 322 PAY 319 

    0x69ff1604,// 323 PAY 320 

    0xf1a7e87b,// 324 PAY 321 

    0x213220ea,// 325 PAY 322 

    0x17c62717,// 326 PAY 323 

    0xe44ff878,// 327 PAY 324 

    0xef1275b1,// 328 PAY 325 

    0x1e5025ef,// 329 PAY 326 

    0xaaa37cf9,// 330 PAY 327 

    0xea90ec4c,// 331 PAY 328 

    0x6b9632c5,// 332 PAY 329 

    0x72468854,// 333 PAY 330 

    0x6db111a0,// 334 PAY 331 

    0xafb09e15,// 335 PAY 332 

    0xadcc3878,// 336 PAY 333 

    0x0da3055a,// 337 PAY 334 

    0x3ca71a0e,// 338 PAY 335 

    0xe810b713,// 339 PAY 336 

    0x386a26bb,// 340 PAY 337 

    0x3c0ece6c,// 341 PAY 338 

    0x9091a6f2,// 342 PAY 339 

    0x63613821,// 343 PAY 340 

    0x2fe91aa4,// 344 PAY 341 

    0xfdf663fc,// 345 PAY 342 

    0x5e5f1d54,// 346 PAY 343 

    0x5d2c06d3,// 347 PAY 344 

    0x2502d9a4,// 348 PAY 345 

    0x384b1e10,// 349 PAY 346 

    0x1aa229aa,// 350 PAY 347 

    0xf6290650,// 351 PAY 348 

    0x5567fe1c,// 352 PAY 349 

    0xab1e73b8,// 353 PAY 350 

    0xa883c3b3,// 354 PAY 351 

    0x79ccc88b,// 355 PAY 352 

    0xbc560000,// 356 PAY 353 

/// STA is 1 words. 

/// STA num_pkts       : 92 

/// STA pkt_idx        : 100 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xde 

    0x0191de5c // 357 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt16_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 286 words. 

/// BDA size     is 1138 (0x472) 

/// BDA id       is 0x8a3d 

    0x04728a3d,// 3 BDA   1 

/// PAY Generic Data size   : 1138 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x14a592b1,// 4 PAY   1 

    0x1151c4f5,// 5 PAY   2 

    0xec86d0a4,// 6 PAY   3 

    0xe31a2eaa,// 7 PAY   4 

    0xe4769455,// 8 PAY   5 

    0xf01d083a,// 9 PAY   6 

    0xa7f3e069,// 10 PAY   7 

    0xe1284f5e,// 11 PAY   8 

    0xa0061f86,// 12 PAY   9 

    0x4fcf60dd,// 13 PAY  10 

    0x40696bc7,// 14 PAY  11 

    0x71ce182c,// 15 PAY  12 

    0x8a14d1da,// 16 PAY  13 

    0xcb79b2c6,// 17 PAY  14 

    0xf9695bbc,// 18 PAY  15 

    0xda092272,// 19 PAY  16 

    0x382f6db7,// 20 PAY  17 

    0x58a83c6d,// 21 PAY  18 

    0x6141f5eb,// 22 PAY  19 

    0x9c130718,// 23 PAY  20 

    0x24cdbae5,// 24 PAY  21 

    0x4ebf7281,// 25 PAY  22 

    0xb5e22747,// 26 PAY  23 

    0x92edc4b5,// 27 PAY  24 

    0xc19a4bbd,// 28 PAY  25 

    0x2361b47f,// 29 PAY  26 

    0xc7169c07,// 30 PAY  27 

    0x69361fbe,// 31 PAY  28 

    0xbd79a90b,// 32 PAY  29 

    0x0e841e51,// 33 PAY  30 

    0xdb66476a,// 34 PAY  31 

    0xdf2786df,// 35 PAY  32 

    0x435f719c,// 36 PAY  33 

    0x2938f58f,// 37 PAY  34 

    0x4d6603c2,// 38 PAY  35 

    0x29493f2b,// 39 PAY  36 

    0x6ec3258a,// 40 PAY  37 

    0xb9ab7588,// 41 PAY  38 

    0xe9bf9713,// 42 PAY  39 

    0x49cbd518,// 43 PAY  40 

    0xb50a1709,// 44 PAY  41 

    0x90b1cb0a,// 45 PAY  42 

    0x9b73bbd1,// 46 PAY  43 

    0x57ceee80,// 47 PAY  44 

    0xc62df69b,// 48 PAY  45 

    0x4fbfb668,// 49 PAY  46 

    0x8db814f7,// 50 PAY  47 

    0x3122cce9,// 51 PAY  48 

    0x5ba7d9a3,// 52 PAY  49 

    0x21b99f87,// 53 PAY  50 

    0x7cf3f8ed,// 54 PAY  51 

    0xbee8228b,// 55 PAY  52 

    0x5303aa81,// 56 PAY  53 

    0xc0feb368,// 57 PAY  54 

    0x922dd8a4,// 58 PAY  55 

    0xbd86e794,// 59 PAY  56 

    0xd5b7b8a8,// 60 PAY  57 

    0x13bcef17,// 61 PAY  58 

    0x80b5e900,// 62 PAY  59 

    0x886a212d,// 63 PAY  60 

    0x31b51011,// 64 PAY  61 

    0xeb6546a1,// 65 PAY  62 

    0xdebd9cc2,// 66 PAY  63 

    0x0f91759c,// 67 PAY  64 

    0x5ea9e83c,// 68 PAY  65 

    0xcdddc529,// 69 PAY  66 

    0x15cc9308,// 70 PAY  67 

    0xec13847d,// 71 PAY  68 

    0xc0ced4a5,// 72 PAY  69 

    0x276b7eff,// 73 PAY  70 

    0x7d22bf96,// 74 PAY  71 

    0x5bf7c067,// 75 PAY  72 

    0x0e5d5280,// 76 PAY  73 

    0xb671ade5,// 77 PAY  74 

    0x67d7a130,// 78 PAY  75 

    0xe63f51a3,// 79 PAY  76 

    0xfd28773b,// 80 PAY  77 

    0x5f28c63d,// 81 PAY  78 

    0xe75a1aa7,// 82 PAY  79 

    0xc382f564,// 83 PAY  80 

    0x17776950,// 84 PAY  81 

    0xd00b9849,// 85 PAY  82 

    0xd94c3b7f,// 86 PAY  83 

    0x963a3224,// 87 PAY  84 

    0xa363c0f4,// 88 PAY  85 

    0x835980b4,// 89 PAY  86 

    0xbfdb0a6e,// 90 PAY  87 

    0x2271fbab,// 91 PAY  88 

    0x37ab8c80,// 92 PAY  89 

    0x2f4a11b7,// 93 PAY  90 

    0x929757f1,// 94 PAY  91 

    0x1283927c,// 95 PAY  92 

    0x9ec79676,// 96 PAY  93 

    0xc5642db7,// 97 PAY  94 

    0x63918551,// 98 PAY  95 

    0x8125df92,// 99 PAY  96 

    0xd815eed8,// 100 PAY  97 

    0x7add9ee2,// 101 PAY  98 

    0x47aa6194,// 102 PAY  99 

    0xac5d0be4,// 103 PAY 100 

    0xd40d3f53,// 104 PAY 101 

    0xe24a569e,// 105 PAY 102 

    0x418e493c,// 106 PAY 103 

    0x0ab6f72c,// 107 PAY 104 

    0xdf5a11c8,// 108 PAY 105 

    0xe4e0ab53,// 109 PAY 106 

    0x388a4278,// 110 PAY 107 

    0x2ccf7699,// 111 PAY 108 

    0x38ad4102,// 112 PAY 109 

    0xfcdd290a,// 113 PAY 110 

    0x874f6d46,// 114 PAY 111 

    0x7ab94b28,// 115 PAY 112 

    0xced073f3,// 116 PAY 113 

    0x6f27b6f4,// 117 PAY 114 

    0xa54a17ae,// 118 PAY 115 

    0x287350ed,// 119 PAY 116 

    0x9195a2a9,// 120 PAY 117 

    0x2a2354fb,// 121 PAY 118 

    0x0c56e25c,// 122 PAY 119 

    0xaed29bc7,// 123 PAY 120 

    0xcd026b8c,// 124 PAY 121 

    0x3eeb7911,// 125 PAY 122 

    0x0be12220,// 126 PAY 123 

    0x29fa3f47,// 127 PAY 124 

    0xcce266c4,// 128 PAY 125 

    0xe2300172,// 129 PAY 126 

    0x05bec114,// 130 PAY 127 

    0x2bcc3875,// 131 PAY 128 

    0x94d5f33b,// 132 PAY 129 

    0x09d01443,// 133 PAY 130 

    0x9f2a561c,// 134 PAY 131 

    0xffc452ed,// 135 PAY 132 

    0x032a82cb,// 136 PAY 133 

    0x44d3ba43,// 137 PAY 134 

    0x42fc8b33,// 138 PAY 135 

    0xc30366a0,// 139 PAY 136 

    0xfb09f0ed,// 140 PAY 137 

    0x431d8963,// 141 PAY 138 

    0xa3ddb623,// 142 PAY 139 

    0x10e8b99d,// 143 PAY 140 

    0x53eb6e90,// 144 PAY 141 

    0x4060314c,// 145 PAY 142 

    0x6b48765a,// 146 PAY 143 

    0x2763cb98,// 147 PAY 144 

    0xb10511c7,// 148 PAY 145 

    0x7263c971,// 149 PAY 146 

    0x926184f4,// 150 PAY 147 

    0xe8842a54,// 151 PAY 148 

    0x98e7d032,// 152 PAY 149 

    0xc78d6ea3,// 153 PAY 150 

    0x728abd4c,// 154 PAY 151 

    0xda88b301,// 155 PAY 152 

    0x60171cff,// 156 PAY 153 

    0x148380df,// 157 PAY 154 

    0x93b94327,// 158 PAY 155 

    0xfa1310e9,// 159 PAY 156 

    0x97e09827,// 160 PAY 157 

    0x2f74e1dc,// 161 PAY 158 

    0x69a2dda2,// 162 PAY 159 

    0x4def17f3,// 163 PAY 160 

    0x0e7e2795,// 164 PAY 161 

    0x0709c709,// 165 PAY 162 

    0x22fe80c5,// 166 PAY 163 

    0xa8ac577a,// 167 PAY 164 

    0xb5770263,// 168 PAY 165 

    0x578183e9,// 169 PAY 166 

    0xc3644788,// 170 PAY 167 

    0x62737710,// 171 PAY 168 

    0xea8b1c70,// 172 PAY 169 

    0x797e35e2,// 173 PAY 170 

    0x455db781,// 174 PAY 171 

    0x0b6ee943,// 175 PAY 172 

    0xde098d31,// 176 PAY 173 

    0xde6fdc04,// 177 PAY 174 

    0xd49627c3,// 178 PAY 175 

    0x43ae10fc,// 179 PAY 176 

    0x307a682d,// 180 PAY 177 

    0x9813630e,// 181 PAY 178 

    0x5797bea1,// 182 PAY 179 

    0xf3548f37,// 183 PAY 180 

    0x24bbc422,// 184 PAY 181 

    0x637bdace,// 185 PAY 182 

    0x7f171781,// 186 PAY 183 

    0xf5d08ab4,// 187 PAY 184 

    0xfa043239,// 188 PAY 185 

    0x38787c03,// 189 PAY 186 

    0x82a18667,// 190 PAY 187 

    0xfa62ebb5,// 191 PAY 188 

    0xa1561f79,// 192 PAY 189 

    0x23eed3a4,// 193 PAY 190 

    0xf11e3a61,// 194 PAY 191 

    0xc0a935ad,// 195 PAY 192 

    0xdb2e19ee,// 196 PAY 193 

    0xebc88bb3,// 197 PAY 194 

    0x9bbb4c24,// 198 PAY 195 

    0x76fe69e5,// 199 PAY 196 

    0x27e87231,// 200 PAY 197 

    0x18762a89,// 201 PAY 198 

    0xcdef1e26,// 202 PAY 199 

    0xec42ab5b,// 203 PAY 200 

    0xe55de240,// 204 PAY 201 

    0x6154083d,// 205 PAY 202 

    0x20064b62,// 206 PAY 203 

    0x7f7f4c63,// 207 PAY 204 

    0xa693eeac,// 208 PAY 205 

    0x102d8e09,// 209 PAY 206 

    0x78b69c4b,// 210 PAY 207 

    0xafe1aa20,// 211 PAY 208 

    0xc66fff4c,// 212 PAY 209 

    0x8ce3f5df,// 213 PAY 210 

    0x04c08767,// 214 PAY 211 

    0x3677a8c7,// 215 PAY 212 

    0xf57f39b4,// 216 PAY 213 

    0xfc490016,// 217 PAY 214 

    0x14280703,// 218 PAY 215 

    0x9c71c744,// 219 PAY 216 

    0x839c2dd5,// 220 PAY 217 

    0x94214bc0,// 221 PAY 218 

    0x0bfdc141,// 222 PAY 219 

    0x34f64a1f,// 223 PAY 220 

    0x7048e85a,// 224 PAY 221 

    0x3c99f2fc,// 225 PAY 222 

    0x8d6dea05,// 226 PAY 223 

    0xf237719e,// 227 PAY 224 

    0x9572b3c7,// 228 PAY 225 

    0xdb662fbf,// 229 PAY 226 

    0xe98042a4,// 230 PAY 227 

    0x4acd94a8,// 231 PAY 228 

    0x9e98f2b9,// 232 PAY 229 

    0x7b762bcd,// 233 PAY 230 

    0xad43e742,// 234 PAY 231 

    0x6bc3772e,// 235 PAY 232 

    0x2bf20e12,// 236 PAY 233 

    0xd841a469,// 237 PAY 234 

    0x0f12c13d,// 238 PAY 235 

    0x83b1b096,// 239 PAY 236 

    0xef02fb04,// 240 PAY 237 

    0x84a4e878,// 241 PAY 238 

    0x2b141186,// 242 PAY 239 

    0x14a3dd2f,// 243 PAY 240 

    0x89ad8eba,// 244 PAY 241 

    0x930e961a,// 245 PAY 242 

    0x92c54f42,// 246 PAY 243 

    0x578ee017,// 247 PAY 244 

    0x005f3628,// 248 PAY 245 

    0x74971c97,// 249 PAY 246 

    0x48c154bb,// 250 PAY 247 

    0xd45685cf,// 251 PAY 248 

    0x4dc2b520,// 252 PAY 249 

    0x97eaad75,// 253 PAY 250 

    0x441f454a,// 254 PAY 251 

    0xfe68e806,// 255 PAY 252 

    0x2bc37eb9,// 256 PAY 253 

    0x044d274e,// 257 PAY 254 

    0xf03f437e,// 258 PAY 255 

    0xcaa80530,// 259 PAY 256 

    0x0ec008d5,// 260 PAY 257 

    0xe2de1f1a,// 261 PAY 258 

    0xf15388df,// 262 PAY 259 

    0x44da4ceb,// 263 PAY 260 

    0x72ff0f84,// 264 PAY 261 

    0x4d6fbb66,// 265 PAY 262 

    0xb6e2d550,// 266 PAY 263 

    0x3bb1f6c5,// 267 PAY 264 

    0x319a497e,// 268 PAY 265 

    0xa22b1e3b,// 269 PAY 266 

    0x796e6a21,// 270 PAY 267 

    0x78f5e7d8,// 271 PAY 268 

    0xf2590dde,// 272 PAY 269 

    0x3f605c30,// 273 PAY 270 

    0x12ba37e8,// 274 PAY 271 

    0x541d8dea,// 275 PAY 272 

    0x467812eb,// 276 PAY 273 

    0xbb46d039,// 277 PAY 274 

    0x803c48a5,// 278 PAY 275 

    0x1c46752e,// 279 PAY 276 

    0x469752ec,// 280 PAY 277 

    0xde21673e,// 281 PAY 278 

    0xe5d3d820,// 282 PAY 279 

    0x8bca9357,// 283 PAY 280 

    0x52a3d9fb,// 284 PAY 281 

    0xc3d96503,// 285 PAY 282 

    0xf912092a,// 286 PAY 283 

    0x9d4b6763,// 287 PAY 284 

    0x31c90000,// 288 PAY 285 

/// STA is 1 words. 

/// STA num_pkts       : 124 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc5 

    0x03dcc57c // 289 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt17_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 47 words. 

/// BDA size     is 182 (0xb6) 

/// BDA id       is 0xc3ba 

    0x00b6c3ba,// 3 BDA   1 

/// PAY Generic Data size   : 182 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x8e1c0abf,// 4 PAY   1 

    0x680b03cf,// 5 PAY   2 

    0xb58f5caa,// 6 PAY   3 

    0x48903301,// 7 PAY   4 

    0xd560125d,// 8 PAY   5 

    0xd16939a2,// 9 PAY   6 

    0x8495b648,// 10 PAY   7 

    0x62083b13,// 11 PAY   8 

    0xfe2db590,// 12 PAY   9 

    0x7cb399fd,// 13 PAY  10 

    0xdd277b25,// 14 PAY  11 

    0xd4fedf62,// 15 PAY  12 

    0x4dac0c91,// 16 PAY  13 

    0x5eb1bec0,// 17 PAY  14 

    0x76c38205,// 18 PAY  15 

    0x50b03b5a,// 19 PAY  16 

    0x879d3a22,// 20 PAY  17 

    0xf55619d2,// 21 PAY  18 

    0x2312b2f4,// 22 PAY  19 

    0x8b773bd2,// 23 PAY  20 

    0xabc23b37,// 24 PAY  21 

    0x7597fd38,// 25 PAY  22 

    0x303fcaaf,// 26 PAY  23 

    0xd3e39c74,// 27 PAY  24 

    0x5a973d9b,// 28 PAY  25 

    0xb8333ebe,// 29 PAY  26 

    0x0e5045dd,// 30 PAY  27 

    0x9ce92bce,// 31 PAY  28 

    0x43b78583,// 32 PAY  29 

    0x2073b36c,// 33 PAY  30 

    0x53b1231f,// 34 PAY  31 

    0xc48b9341,// 35 PAY  32 

    0x0ac600d0,// 36 PAY  33 

    0xffb08a88,// 37 PAY  34 

    0x5ec6dae5,// 38 PAY  35 

    0xac24fcf1,// 39 PAY  36 

    0xd4084c25,// 40 PAY  37 

    0xe426b64d,// 41 PAY  38 

    0xe8bd03c6,// 42 PAY  39 

    0x926d1a6d,// 43 PAY  40 

    0x1e705504,// 44 PAY  41 

    0xaebaf796,// 45 PAY  42 

    0x033e66af,// 46 PAY  43 

    0xf93d5d41,// 47 PAY  44 

    0x75f0a1ca,// 48 PAY  45 

    0x223e0000,// 49 PAY  46 

/// HASH is  16 bytes 

    0x53b1231f,// 50 HSH   1 

    0xc48b9341,// 51 HSH   2 

    0x0ac600d0,// 52 HSH   3 

    0xffb08a88,// 53 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 137 

/// STA pkt_idx        : 109 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc0 

    0x01b5c089 // 54 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt18_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 287 words. 

/// BDA size     is 1144 (0x478) 

/// BDA id       is 0xdbea 

    0x0478dbea,// 3 BDA   1 

/// PAY Generic Data size   : 1144 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x313219ab,// 4 PAY   1 

    0xa04f3bc1,// 5 PAY   2 

    0xf73fb9dd,// 6 PAY   3 

    0xf76c7e49,// 7 PAY   4 

    0xcc01be39,// 8 PAY   5 

    0xe3e18d30,// 9 PAY   6 

    0x000cc02e,// 10 PAY   7 

    0x2d1e6ba6,// 11 PAY   8 

    0xfd52f8ee,// 12 PAY   9 

    0xdd3097c9,// 13 PAY  10 

    0x42503f38,// 14 PAY  11 

    0xe5043679,// 15 PAY  12 

    0xca16e35c,// 16 PAY  13 

    0x2464c037,// 17 PAY  14 

    0xfe7392fc,// 18 PAY  15 

    0x4402643e,// 19 PAY  16 

    0x722a2666,// 20 PAY  17 

    0x37245b18,// 21 PAY  18 

    0xf71d9ee2,// 22 PAY  19 

    0x8188d981,// 23 PAY  20 

    0x09520c73,// 24 PAY  21 

    0x3d247d60,// 25 PAY  22 

    0x2fa06c30,// 26 PAY  23 

    0x7882c538,// 27 PAY  24 

    0xe31e75d0,// 28 PAY  25 

    0x93fa30c1,// 29 PAY  26 

    0xe5119aa4,// 30 PAY  27 

    0x95446dff,// 31 PAY  28 

    0x3c33fa6d,// 32 PAY  29 

    0x3f02270e,// 33 PAY  30 

    0x351a381b,// 34 PAY  31 

    0xb5d3041e,// 35 PAY  32 

    0xeae98d7c,// 36 PAY  33 

    0x8f98c650,// 37 PAY  34 

    0x8a87ed0d,// 38 PAY  35 

    0x25de9c2b,// 39 PAY  36 

    0x615b9142,// 40 PAY  37 

    0x7d7454f5,// 41 PAY  38 

    0x8ee83150,// 42 PAY  39 

    0xcd73239e,// 43 PAY  40 

    0xbf7dc450,// 44 PAY  41 

    0x47d80201,// 45 PAY  42 

    0xf19f1f81,// 46 PAY  43 

    0xe56e63d9,// 47 PAY  44 

    0x6974fa61,// 48 PAY  45 

    0x6563485d,// 49 PAY  46 

    0x9cbfdbd3,// 50 PAY  47 

    0x7cac7634,// 51 PAY  48 

    0xe02fde3a,// 52 PAY  49 

    0x7da24c43,// 53 PAY  50 

    0x38c1d29b,// 54 PAY  51 

    0xfa92c803,// 55 PAY  52 

    0xc1a5dac2,// 56 PAY  53 

    0x8efcc84c,// 57 PAY  54 

    0xe28298b1,// 58 PAY  55 

    0x895530d0,// 59 PAY  56 

    0xbaa1eff4,// 60 PAY  57 

    0xae3012f6,// 61 PAY  58 

    0xb871d61f,// 62 PAY  59 

    0x7487cd10,// 63 PAY  60 

    0xbd8a4366,// 64 PAY  61 

    0x9ab54827,// 65 PAY  62 

    0x9f0a8bee,// 66 PAY  63 

    0x17fe389b,// 67 PAY  64 

    0xe77501a9,// 68 PAY  65 

    0x2ebc46b1,// 69 PAY  66 

    0x48892657,// 70 PAY  67 

    0xbc8f3183,// 71 PAY  68 

    0xc01c4c7f,// 72 PAY  69 

    0x2da2abfd,// 73 PAY  70 

    0x45be9507,// 74 PAY  71 

    0x15062cbb,// 75 PAY  72 

    0xf587f0fa,// 76 PAY  73 

    0x8d167ab3,// 77 PAY  74 

    0x94c14b48,// 78 PAY  75 

    0x1a6ef857,// 79 PAY  76 

    0xcf3ac294,// 80 PAY  77 

    0xec96d44e,// 81 PAY  78 

    0x59d9a749,// 82 PAY  79 

    0x9c7d3d48,// 83 PAY  80 

    0x2592ca0e,// 84 PAY  81 

    0xb6002341,// 85 PAY  82 

    0x11c2b7f2,// 86 PAY  83 

    0x4de7b67a,// 87 PAY  84 

    0x63f01663,// 88 PAY  85 

    0xc4b99156,// 89 PAY  86 

    0x4f3d1ba2,// 90 PAY  87 

    0x90e430d0,// 91 PAY  88 

    0xa7ea9e64,// 92 PAY  89 

    0x413b2729,// 93 PAY  90 

    0x1489cbd0,// 94 PAY  91 

    0xac6881a5,// 95 PAY  92 

    0x7ac44728,// 96 PAY  93 

    0x402dea35,// 97 PAY  94 

    0xca7cf804,// 98 PAY  95 

    0x98a020d0,// 99 PAY  96 

    0x818cdb55,// 100 PAY  97 

    0xdb7f2739,// 101 PAY  98 

    0x8c2c91a3,// 102 PAY  99 

    0xca28639a,// 103 PAY 100 

    0x9e9f804a,// 104 PAY 101 

    0xca14bd2a,// 105 PAY 102 

    0x01823f41,// 106 PAY 103 

    0xa0bfae01,// 107 PAY 104 

    0x4dff26fb,// 108 PAY 105 

    0x4863b2e2,// 109 PAY 106 

    0x21c924a4,// 110 PAY 107 

    0x21330288,// 111 PAY 108 

    0x8bb4a971,// 112 PAY 109 

    0xba6e1ff1,// 113 PAY 110 

    0x27290be7,// 114 PAY 111 

    0x38e98c15,// 115 PAY 112 

    0x12ca6860,// 116 PAY 113 

    0x9f2a1ffc,// 117 PAY 114 

    0x28af3462,// 118 PAY 115 

    0xda8c0ac6,// 119 PAY 116 

    0x0164cece,// 120 PAY 117 

    0x64611691,// 121 PAY 118 

    0xf178be3a,// 122 PAY 119 

    0x7d3c3244,// 123 PAY 120 

    0xb6b4cbb7,// 124 PAY 121 

    0x68c34838,// 125 PAY 122 

    0xac63e28c,// 126 PAY 123 

    0x9dcff1a7,// 127 PAY 124 

    0x962d163e,// 128 PAY 125 

    0x393059b9,// 129 PAY 126 

    0x9d6f3375,// 130 PAY 127 

    0xcfba57a6,// 131 PAY 128 

    0x08f27c1b,// 132 PAY 129 

    0xee829178,// 133 PAY 130 

    0xb07281af,// 134 PAY 131 

    0x6b6b5b3b,// 135 PAY 132 

    0xee230c65,// 136 PAY 133 

    0xa871224a,// 137 PAY 134 

    0x56e43fac,// 138 PAY 135 

    0xbe09cdbb,// 139 PAY 136 

    0x6d2b39b0,// 140 PAY 137 

    0xe49c815b,// 141 PAY 138 

    0x64bf8578,// 142 PAY 139 

    0x92919ce6,// 143 PAY 140 

    0xf0a183ed,// 144 PAY 141 

    0x0a7f4ef4,// 145 PAY 142 

    0x15b869f6,// 146 PAY 143 

    0xb9da82bf,// 147 PAY 144 

    0xf0a3ae43,// 148 PAY 145 

    0x9fddc902,// 149 PAY 146 

    0x1c7bdb7f,// 150 PAY 147 

    0x81b2069e,// 151 PAY 148 

    0x7a3f1663,// 152 PAY 149 

    0xad22b17a,// 153 PAY 150 

    0x79bd595a,// 154 PAY 151 

    0xe63d5c91,// 155 PAY 152 

    0xd3969cda,// 156 PAY 153 

    0xa1b9db2e,// 157 PAY 154 

    0xaee94589,// 158 PAY 155 

    0x0267adfd,// 159 PAY 156 

    0xa7b69962,// 160 PAY 157 

    0x74c78801,// 161 PAY 158 

    0xb5bea988,// 162 PAY 159 

    0xae46f60c,// 163 PAY 160 

    0x0368fe2e,// 164 PAY 161 

    0x765513f8,// 165 PAY 162 

    0x8942a9e3,// 166 PAY 163 

    0xf69f6128,// 167 PAY 164 

    0x5de8fddb,// 168 PAY 165 

    0x9292bfb5,// 169 PAY 166 

    0x59555316,// 170 PAY 167 

    0xf052e6de,// 171 PAY 168 

    0x94873c80,// 172 PAY 169 

    0x198412dd,// 173 PAY 170 

    0x19bc1bf3,// 174 PAY 171 

    0x65b24377,// 175 PAY 172 

    0xa2d01bda,// 176 PAY 173 

    0xc452422f,// 177 PAY 174 

    0x788b870a,// 178 PAY 175 

    0xa717acfd,// 179 PAY 176 

    0x8df0847a,// 180 PAY 177 

    0x37f245be,// 181 PAY 178 

    0x2d781cac,// 182 PAY 179 

    0x1f319507,// 183 PAY 180 

    0xb999a4ad,// 184 PAY 181 

    0xc4e849a3,// 185 PAY 182 

    0x08073cc8,// 186 PAY 183 

    0x1bedf7bd,// 187 PAY 184 

    0x87083e9c,// 188 PAY 185 

    0x84f8d9e0,// 189 PAY 186 

    0xa02a097f,// 190 PAY 187 

    0x9c162f2f,// 191 PAY 188 

    0xa799bb6d,// 192 PAY 189 

    0x8ae374ab,// 193 PAY 190 

    0x75ecb96f,// 194 PAY 191 

    0x3882d39e,// 195 PAY 192 

    0xda1d7052,// 196 PAY 193 

    0x106fca32,// 197 PAY 194 

    0x3d60fd27,// 198 PAY 195 

    0x5ffed5dc,// 199 PAY 196 

    0x3c0fe7a0,// 200 PAY 197 

    0x624f9cdc,// 201 PAY 198 

    0x4a6be3f9,// 202 PAY 199 

    0x880c5129,// 203 PAY 200 

    0x7ee09287,// 204 PAY 201 

    0x06db1bc1,// 205 PAY 202 

    0xaa794952,// 206 PAY 203 

    0xc2db4100,// 207 PAY 204 

    0x5cb05a1e,// 208 PAY 205 

    0xbfe07c49,// 209 PAY 206 

    0x8b73127d,// 210 PAY 207 

    0xc9e5ca0b,// 211 PAY 208 

    0xd761a9ab,// 212 PAY 209 

    0x0902835a,// 213 PAY 210 

    0xb227d11b,// 214 PAY 211 

    0x13a8603a,// 215 PAY 212 

    0x6984e729,// 216 PAY 213 

    0x1a756e72,// 217 PAY 214 

    0x75d6912b,// 218 PAY 215 

    0xd26a3c9a,// 219 PAY 216 

    0x71eac7be,// 220 PAY 217 

    0xaa36782f,// 221 PAY 218 

    0x0d23d558,// 222 PAY 219 

    0x6957f122,// 223 PAY 220 

    0xc7400d69,// 224 PAY 221 

    0x73d23671,// 225 PAY 222 

    0xfbbdbd10,// 226 PAY 223 

    0x6fe0cb78,// 227 PAY 224 

    0x7cc07b22,// 228 PAY 225 

    0x83aeb5e6,// 229 PAY 226 

    0x451fe760,// 230 PAY 227 

    0x12cf177c,// 231 PAY 228 

    0x19a13fb7,// 232 PAY 229 

    0x666c6cc4,// 233 PAY 230 

    0xf0d0a554,// 234 PAY 231 

    0x428bafa7,// 235 PAY 232 

    0x29c16f1c,// 236 PAY 233 

    0x5d937706,// 237 PAY 234 

    0xbc879775,// 238 PAY 235 

    0x266e14af,// 239 PAY 236 

    0xf09b4c40,// 240 PAY 237 

    0xf945853c,// 241 PAY 238 

    0xe2258519,// 242 PAY 239 

    0xbfca556c,// 243 PAY 240 

    0x95444964,// 244 PAY 241 

    0x9a944986,// 245 PAY 242 

    0xaac172ce,// 246 PAY 243 

    0xdfb271f6,// 247 PAY 244 

    0x6b995f51,// 248 PAY 245 

    0x181f441d,// 249 PAY 246 

    0x44eb9e91,// 250 PAY 247 

    0x12b233a7,// 251 PAY 248 

    0xf217b53e,// 252 PAY 249 

    0xa8aa8661,// 253 PAY 250 

    0x1028f734,// 254 PAY 251 

    0xea5db38a,// 255 PAY 252 

    0x8cb526fe,// 256 PAY 253 

    0x7c683eea,// 257 PAY 254 

    0xc03d2950,// 258 PAY 255 

    0x65e877d4,// 259 PAY 256 

    0x1ca5a22e,// 260 PAY 257 

    0xe64caf61,// 261 PAY 258 

    0x71c3cada,// 262 PAY 259 

    0x88afd8b4,// 263 PAY 260 

    0xbc38ffa3,// 264 PAY 261 

    0x376b6c87,// 265 PAY 262 

    0xa5e5c1f9,// 266 PAY 263 

    0xfc2e25df,// 267 PAY 264 

    0x8659646d,// 268 PAY 265 

    0x30da1ace,// 269 PAY 266 

    0x77fde0f0,// 270 PAY 267 

    0xd6e6b012,// 271 PAY 268 

    0x92c050b0,// 272 PAY 269 

    0x428779d0,// 273 PAY 270 

    0xccfe1d48,// 274 PAY 271 

    0x07eb6190,// 275 PAY 272 

    0xe0821607,// 276 PAY 273 

    0xf844d3d1,// 277 PAY 274 

    0x06e54c6d,// 278 PAY 275 

    0xebe50335,// 279 PAY 276 

    0xc296c0a8,// 280 PAY 277 

    0xe5b5def8,// 281 PAY 278 

    0x3bed1c69,// 282 PAY 279 

    0xfe30934b,// 283 PAY 280 

    0xe7c8f679,// 284 PAY 281 

    0x651cda10,// 285 PAY 282 

    0x53207572,// 286 PAY 283 

    0x4f2802ba,// 287 PAY 284 

    0x313f13b0,// 288 PAY 285 

    0x3e595848,// 289 PAY 286 

/// HASH is  4 bytes 

    0xc9e5ca0b,// 290 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 212 

/// STA pkt_idx        : 86 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe6 

    0x0158e6d4 // 291 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt19_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 56 words. 

/// BDA size     is 218 (0xda) 

/// BDA id       is 0x426c 

    0x00da426c,// 3 BDA   1 

/// PAY Generic Data size   : 218 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x0beb7dbf,// 4 PAY   1 

    0x5ecadbb4,// 5 PAY   2 

    0x65071c52,// 6 PAY   3 

    0x2b83d669,// 7 PAY   4 

    0xa215162e,// 8 PAY   5 

    0x55e822d9,// 9 PAY   6 

    0x44b1b620,// 10 PAY   7 

    0xecffa843,// 11 PAY   8 

    0xeff6f4ef,// 12 PAY   9 

    0x3b794e8d,// 13 PAY  10 

    0x497945e1,// 14 PAY  11 

    0x0fab4181,// 15 PAY  12 

    0xc60cc56a,// 16 PAY  13 

    0x9cb36323,// 17 PAY  14 

    0xcecc69c4,// 18 PAY  15 

    0x23b6d830,// 19 PAY  16 

    0xed457ba4,// 20 PAY  17 

    0x0c6b1db0,// 21 PAY  18 

    0xf0976b90,// 22 PAY  19 

    0xf32369c3,// 23 PAY  20 

    0xdd19ecf4,// 24 PAY  21 

    0x4a9e5f82,// 25 PAY  22 

    0xb94bc9ee,// 26 PAY  23 

    0xfa711bab,// 27 PAY  24 

    0x12637d67,// 28 PAY  25 

    0xdf6782b2,// 29 PAY  26 

    0x4704f523,// 30 PAY  27 

    0x87145ff7,// 31 PAY  28 

    0xbbad2053,// 32 PAY  29 

    0x4e9006ba,// 33 PAY  30 

    0x086eb00c,// 34 PAY  31 

    0xb5556976,// 35 PAY  32 

    0x4f398873,// 36 PAY  33 

    0x410843c3,// 37 PAY  34 

    0x655da360,// 38 PAY  35 

    0xfa805911,// 39 PAY  36 

    0x7de045c7,// 40 PAY  37 

    0x38292758,// 41 PAY  38 

    0xaca509bf,// 42 PAY  39 

    0x2f489dec,// 43 PAY  40 

    0x4e0d167e,// 44 PAY  41 

    0x1c4e32a9,// 45 PAY  42 

    0xf6c0b554,// 46 PAY  43 

    0x515cb24b,// 47 PAY  44 

    0xaa8d00b8,// 48 PAY  45 

    0x37300051,// 49 PAY  46 

    0x921f7e30,// 50 PAY  47 

    0x1edaa9a4,// 51 PAY  48 

    0xe67ed09d,// 52 PAY  49 

    0x512a2f27,// 53 PAY  50 

    0xd69d7eca,// 54 PAY  51 

    0x9ff59006,// 55 PAY  52 

    0x214e632d,// 56 PAY  53 

    0xcbd5eec8,// 57 PAY  54 

    0xcc6a0000,// 58 PAY  55 

/// HASH is  16 bytes 

    0x4f398873,// 59 HSH   1 

    0x410843c3,// 60 HSH   2 

    0x655da360,// 61 HSH   3 

    0xfa805911,// 62 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 27 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02cc3c1b // 63 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt20_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 92 words. 

/// BDA size     is 364 (0x16c) 

/// BDA id       is 0x1692 

    0x016c1692,// 3 BDA   1 

/// PAY Generic Data size   : 364 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xb7bfe802,// 4 PAY   1 

    0x4f36d580,// 5 PAY   2 

    0xa2bdaa35,// 6 PAY   3 

    0xa504eed0,// 7 PAY   4 

    0xf2920c72,// 8 PAY   5 

    0x6b97e3da,// 9 PAY   6 

    0x8cb67451,// 10 PAY   7 

    0xbd11eab9,// 11 PAY   8 

    0x09197788,// 12 PAY   9 

    0x5c449632,// 13 PAY  10 

    0xc86b91d5,// 14 PAY  11 

    0x73ae6ae9,// 15 PAY  12 

    0x4b19d117,// 16 PAY  13 

    0xac5a9480,// 17 PAY  14 

    0x75893b29,// 18 PAY  15 

    0x7137fca9,// 19 PAY  16 

    0xc9402de5,// 20 PAY  17 

    0x1014d603,// 21 PAY  18 

    0x37123c8c,// 22 PAY  19 

    0xd54c59f9,// 23 PAY  20 

    0xbecbce21,// 24 PAY  21 

    0xb2c5250e,// 25 PAY  22 

    0x3f2e4d99,// 26 PAY  23 

    0x7816e514,// 27 PAY  24 

    0xf44c7ea3,// 28 PAY  25 

    0x3cc9810f,// 29 PAY  26 

    0xe434a574,// 30 PAY  27 

    0xfb302d56,// 31 PAY  28 

    0x92ba7fb3,// 32 PAY  29 

    0xa1e47385,// 33 PAY  30 

    0x03a6d4a5,// 34 PAY  31 

    0x512c8bcc,// 35 PAY  32 

    0xb08a6b70,// 36 PAY  33 

    0x417804d6,// 37 PAY  34 

    0x8e1a1782,// 38 PAY  35 

    0x4d41f650,// 39 PAY  36 

    0xad211852,// 40 PAY  37 

    0xb9c0e050,// 41 PAY  38 

    0x826db892,// 42 PAY  39 

    0x69d2f13e,// 43 PAY  40 

    0x027911c3,// 44 PAY  41 

    0x213061cd,// 45 PAY  42 

    0xfd4d096a,// 46 PAY  43 

    0x1583258d,// 47 PAY  44 

    0x45e141bf,// 48 PAY  45 

    0x2b58f540,// 49 PAY  46 

    0x273ba6a0,// 50 PAY  47 

    0x503724d0,// 51 PAY  48 

    0xbe8a6d93,// 52 PAY  49 

    0x80a69d79,// 53 PAY  50 

    0xe06ff9fd,// 54 PAY  51 

    0xecf31502,// 55 PAY  52 

    0xe2d22cda,// 56 PAY  53 

    0x81168baf,// 57 PAY  54 

    0x740cd847,// 58 PAY  55 

    0xcdf4e874,// 59 PAY  56 

    0x89263463,// 60 PAY  57 

    0xdb564a4e,// 61 PAY  58 

    0x66a45457,// 62 PAY  59 

    0xa7b388b6,// 63 PAY  60 

    0xc232a774,// 64 PAY  61 

    0x1fe96dda,// 65 PAY  62 

    0x63ec0bc9,// 66 PAY  63 

    0xf215849b,// 67 PAY  64 

    0xafe017f4,// 68 PAY  65 

    0x899e2cd1,// 69 PAY  66 

    0xbf4e54e2,// 70 PAY  67 

    0x92b454f7,// 71 PAY  68 

    0x6479d346,// 72 PAY  69 

    0x7e69db21,// 73 PAY  70 

    0xc50530b0,// 74 PAY  71 

    0x227ab299,// 75 PAY  72 

    0xefea54ad,// 76 PAY  73 

    0x31a0470d,// 77 PAY  74 

    0x7a0655ce,// 78 PAY  75 

    0xb814fe17,// 79 PAY  76 

    0x49976485,// 80 PAY  77 

    0x80f57f2c,// 81 PAY  78 

    0xd47bc2ce,// 82 PAY  79 

    0xb9e1ccb1,// 83 PAY  80 

    0x77f4e999,// 84 PAY  81 

    0xb969c159,// 85 PAY  82 

    0x2666ecaf,// 86 PAY  83 

    0x254e666f,// 87 PAY  84 

    0xea3b1934,// 88 PAY  85 

    0x012c8e73,// 89 PAY  86 

    0x1222040a,// 90 PAY  87 

    0x34232396,// 91 PAY  88 

    0x086b8f84,// 92 PAY  89 

    0x8f525ee4,// 93 PAY  90 

    0xf38976e9,// 94 PAY  91 

/// HASH is  12 bytes 

    0x012c8e73,// 95 HSH   1 

    0x1222040a,// 96 HSH   2 

    0x34232396,// 97 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 230 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x005c5be6 // 98 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt21_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 112 words. 

/// BDA size     is 442 (0x1ba) 

/// BDA id       is 0xb7c2 

    0x01bab7c2,// 3 BDA   1 

/// PAY Generic Data size   : 442 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xc3b1c1f0,// 4 PAY   1 

    0xa623d261,// 5 PAY   2 

    0xdfaffdbe,// 6 PAY   3 

    0x5c566f38,// 7 PAY   4 

    0xfb253181,// 8 PAY   5 

    0x4ecc12d4,// 9 PAY   6 

    0xcdd4abc5,// 10 PAY   7 

    0x0dc0dbd6,// 11 PAY   8 

    0x0896647e,// 12 PAY   9 

    0x86f9caaf,// 13 PAY  10 

    0x09d3d721,// 14 PAY  11 

    0xf60047f0,// 15 PAY  12 

    0xdb8d5bf3,// 16 PAY  13 

    0x4261605d,// 17 PAY  14 

    0x3c83d69a,// 18 PAY  15 

    0xbca4fe8f,// 19 PAY  16 

    0xd006ce1c,// 20 PAY  17 

    0x53f02c0d,// 21 PAY  18 

    0x65d864f4,// 22 PAY  19 

    0xf6ff76e4,// 23 PAY  20 

    0xabd769aa,// 24 PAY  21 

    0x8592bba8,// 25 PAY  22 

    0x6bca728b,// 26 PAY  23 

    0x8153856f,// 27 PAY  24 

    0xfbc075fd,// 28 PAY  25 

    0xc0d2eac4,// 29 PAY  26 

    0xd98178e1,// 30 PAY  27 

    0x17f71a62,// 31 PAY  28 

    0xd134f851,// 32 PAY  29 

    0xcc6ea311,// 33 PAY  30 

    0xd9c1ca12,// 34 PAY  31 

    0xb9e9f8b1,// 35 PAY  32 

    0xa1822f3c,// 36 PAY  33 

    0x6f6165d5,// 37 PAY  34 

    0x21c763df,// 38 PAY  35 

    0x63057320,// 39 PAY  36 

    0x3d8c66cf,// 40 PAY  37 

    0xa3cba1ff,// 41 PAY  38 

    0xaf0158d0,// 42 PAY  39 

    0x904a29d5,// 43 PAY  40 

    0x512c9d4a,// 44 PAY  41 

    0x1da12df3,// 45 PAY  42 

    0xe02fa5b4,// 46 PAY  43 

    0xd278ae22,// 47 PAY  44 

    0x1deaa853,// 48 PAY  45 

    0xb9923bb4,// 49 PAY  46 

    0xef1b1313,// 50 PAY  47 

    0xaa85c938,// 51 PAY  48 

    0x93c5dc27,// 52 PAY  49 

    0x5fb32893,// 53 PAY  50 

    0x28101560,// 54 PAY  51 

    0xe0970638,// 55 PAY  52 

    0x35e553a5,// 56 PAY  53 

    0x0ae66f2c,// 57 PAY  54 

    0xbf432d1b,// 58 PAY  55 

    0x8b6822bf,// 59 PAY  56 

    0x0521286d,// 60 PAY  57 

    0x4777e933,// 61 PAY  58 

    0xd0858db6,// 62 PAY  59 

    0xe8f37f63,// 63 PAY  60 

    0xc8e56613,// 64 PAY  61 

    0x33e4976b,// 65 PAY  62 

    0xfccaf747,// 66 PAY  63 

    0xdba64db8,// 67 PAY  64 

    0x2f1166b3,// 68 PAY  65 

    0xe49ccdc5,// 69 PAY  66 

    0x8fba5a12,// 70 PAY  67 

    0xf1c618bf,// 71 PAY  68 

    0x1bcc36cc,// 72 PAY  69 

    0x3dbf72bc,// 73 PAY  70 

    0x7752ed55,// 74 PAY  71 

    0xc08e1ab1,// 75 PAY  72 

    0x053688dd,// 76 PAY  73 

    0x13b61792,// 77 PAY  74 

    0x832607c9,// 78 PAY  75 

    0x8055db37,// 79 PAY  76 

    0xf6c59957,// 80 PAY  77 

    0x473c698a,// 81 PAY  78 

    0x25d20a0a,// 82 PAY  79 

    0x5a6fd67a,// 83 PAY  80 

    0x34824930,// 84 PAY  81 

    0xa45557f5,// 85 PAY  82 

    0xaa190428,// 86 PAY  83 

    0xad735a3b,// 87 PAY  84 

    0xa3671d47,// 88 PAY  85 

    0xa225254f,// 89 PAY  86 

    0x7be260a6,// 90 PAY  87 

    0x75a70932,// 91 PAY  88 

    0x4314e949,// 92 PAY  89 

    0x0bddcd86,// 93 PAY  90 

    0xf4ad681e,// 94 PAY  91 

    0xc2d0adfa,// 95 PAY  92 

    0x142f2d20,// 96 PAY  93 

    0xb928cfae,// 97 PAY  94 

    0x69b35a4a,// 98 PAY  95 

    0x74795b0c,// 99 PAY  96 

    0x26111565,// 100 PAY  97 

    0x8fb49b1a,// 101 PAY  98 

    0xac978841,// 102 PAY  99 

    0xc0391fb9,// 103 PAY 100 

    0x4c72967f,// 104 PAY 101 

    0x676f15c1,// 105 PAY 102 

    0xe3f04e55,// 106 PAY 103 

    0x6dd3dd83,// 107 PAY 104 

    0x5617e43b,// 108 PAY 105 

    0x167f49e8,// 109 PAY 106 

    0x640ad9cf,// 110 PAY 107 

    0x57071785,// 111 PAY 108 

    0xbe689f08,// 112 PAY 109 

    0x03e7a23c,// 113 PAY 110 

    0x43fb0000,// 114 PAY 111 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 59 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x14 

    0x00ec1485 // 115 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt22_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 414 words. 

/// BDA size     is 1652 (0x674) 

/// BDA id       is 0x393e 

    0x0674393e,// 3 BDA   1 

/// PAY Generic Data size   : 1652 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xe3af2ffd,// 4 PAY   1 

    0x489c6e5b,// 5 PAY   2 

    0xa1748dbc,// 6 PAY   3 

    0x916e4acd,// 7 PAY   4 

    0x687bea98,// 8 PAY   5 

    0x9e2d0265,// 9 PAY   6 

    0x0c6202a4,// 10 PAY   7 

    0x6c615cd7,// 11 PAY   8 

    0xb506d6b3,// 12 PAY   9 

    0xd156c133,// 13 PAY  10 

    0xb6b588a5,// 14 PAY  11 

    0x83060d7e,// 15 PAY  12 

    0x8d5a4df9,// 16 PAY  13 

    0x2ef1695c,// 17 PAY  14 

    0x3305a263,// 18 PAY  15 

    0xae9465cb,// 19 PAY  16 

    0xdbfe1cfc,// 20 PAY  17 

    0xbaf5ac32,// 21 PAY  18 

    0x4cd302ab,// 22 PAY  19 

    0x60171234,// 23 PAY  20 

    0x58c3d240,// 24 PAY  21 

    0xa84a1a70,// 25 PAY  22 

    0x6150ffcb,// 26 PAY  23 

    0x737ce8aa,// 27 PAY  24 

    0x824d8c3b,// 28 PAY  25 

    0xc5df7ee7,// 29 PAY  26 

    0x4647a120,// 30 PAY  27 

    0xdb3d93c1,// 31 PAY  28 

    0x32c81bda,// 32 PAY  29 

    0xd2dbdbb0,// 33 PAY  30 

    0x530fdc7f,// 34 PAY  31 

    0x8239d704,// 35 PAY  32 

    0xef46383d,// 36 PAY  33 

    0x0f21f153,// 37 PAY  34 

    0x0a66459b,// 38 PAY  35 

    0xc65612ab,// 39 PAY  36 

    0x688283e6,// 40 PAY  37 

    0x8c34f36e,// 41 PAY  38 

    0xad2c1789,// 42 PAY  39 

    0xe979aad8,// 43 PAY  40 

    0x3033591d,// 44 PAY  41 

    0x838351b0,// 45 PAY  42 

    0x19b23530,// 46 PAY  43 

    0xa378d9bc,// 47 PAY  44 

    0xa0424354,// 48 PAY  45 

    0x6c897288,// 49 PAY  46 

    0x3e5b3df9,// 50 PAY  47 

    0xbc6d3e62,// 51 PAY  48 

    0x106b3bab,// 52 PAY  49 

    0x668b9f31,// 53 PAY  50 

    0x6da3fdb9,// 54 PAY  51 

    0xf9c1d470,// 55 PAY  52 

    0x29aa7bc2,// 56 PAY  53 

    0x24b63403,// 57 PAY  54 

    0xf2049d91,// 58 PAY  55 

    0x3e19d1f3,// 59 PAY  56 

    0xcd33bba5,// 60 PAY  57 

    0x04154796,// 61 PAY  58 

    0x61f2a978,// 62 PAY  59 

    0x8e121b63,// 63 PAY  60 

    0xeaf42c49,// 64 PAY  61 

    0x6e0c5d65,// 65 PAY  62 

    0x2ac16ae5,// 66 PAY  63 

    0xb1cb6316,// 67 PAY  64 

    0x6c1f5479,// 68 PAY  65 

    0xd3bdd75f,// 69 PAY  66 

    0x6a09d672,// 70 PAY  67 

    0xa6fbf972,// 71 PAY  68 

    0x5a362ac7,// 72 PAY  69 

    0x3d3c8a13,// 73 PAY  70 

    0x2e095bce,// 74 PAY  71 

    0xa84101aa,// 75 PAY  72 

    0x70546116,// 76 PAY  73 

    0xd65613ee,// 77 PAY  74 

    0x6e97b9cb,// 78 PAY  75 

    0x38ee7556,// 79 PAY  76 

    0x0fa4533d,// 80 PAY  77 

    0x43aff3b7,// 81 PAY  78 

    0xd6d8a8fe,// 82 PAY  79 

    0xd6c84c56,// 83 PAY  80 

    0x435b8673,// 84 PAY  81 

    0xa81b21d4,// 85 PAY  82 

    0x78ddb9d8,// 86 PAY  83 

    0x08827483,// 87 PAY  84 

    0x624f6623,// 88 PAY  85 

    0x7fd20f00,// 89 PAY  86 

    0x78683270,// 90 PAY  87 

    0x05f7b962,// 91 PAY  88 

    0x9fed3d89,// 92 PAY  89 

    0x58bbc8a9,// 93 PAY  90 

    0x29bf9241,// 94 PAY  91 

    0xcb21b67b,// 95 PAY  92 

    0x02e64205,// 96 PAY  93 

    0x147dc6b3,// 97 PAY  94 

    0x2bc0d7bf,// 98 PAY  95 

    0xfb657218,// 99 PAY  96 

    0xf993871f,// 100 PAY  97 

    0x405168e5,// 101 PAY  98 

    0x97e41617,// 102 PAY  99 

    0xe05e06d6,// 103 PAY 100 

    0x503b19ad,// 104 PAY 101 

    0x1f0ec989,// 105 PAY 102 

    0x45058cd2,// 106 PAY 103 

    0xf4e1185f,// 107 PAY 104 

    0xc4ae5eff,// 108 PAY 105 

    0x080d5c11,// 109 PAY 106 

    0x6161f607,// 110 PAY 107 

    0x4817e38c,// 111 PAY 108 

    0x719d5669,// 112 PAY 109 

    0x60eb33b7,// 113 PAY 110 

    0x59230c9c,// 114 PAY 111 

    0x2617860f,// 115 PAY 112 

    0xdff05d10,// 116 PAY 113 

    0xafb54a32,// 117 PAY 114 

    0xde29b200,// 118 PAY 115 

    0x64baf386,// 119 PAY 116 

    0xbb4ddecd,// 120 PAY 117 

    0x822c3a2a,// 121 PAY 118 

    0x47c8b2c2,// 122 PAY 119 

    0xfde6ba05,// 123 PAY 120 

    0x65ac8446,// 124 PAY 121 

    0xcf89eb07,// 125 PAY 122 

    0x6d6b3946,// 126 PAY 123 

    0xd9192196,// 127 PAY 124 

    0x09f4c9d2,// 128 PAY 125 

    0x0cc08864,// 129 PAY 126 

    0x9a40435f,// 130 PAY 127 

    0xa74ed7b4,// 131 PAY 128 

    0x0245ac17,// 132 PAY 129 

    0x43ec83ac,// 133 PAY 130 

    0x49b8d962,// 134 PAY 131 

    0xb52e938d,// 135 PAY 132 

    0xe42e45d9,// 136 PAY 133 

    0x5a9e5e4c,// 137 PAY 134 

    0xb8142ffc,// 138 PAY 135 

    0x585fb562,// 139 PAY 136 

    0xfbca8810,// 140 PAY 137 

    0x1bc2d526,// 141 PAY 138 

    0x9696dbfd,// 142 PAY 139 

    0x0caa88a8,// 143 PAY 140 

    0xacdfd7b0,// 144 PAY 141 

    0x6c61857c,// 145 PAY 142 

    0xa23b6f30,// 146 PAY 143 

    0x750e55d8,// 147 PAY 144 

    0x0795c718,// 148 PAY 145 

    0xa6460fa4,// 149 PAY 146 

    0xb919857c,// 150 PAY 147 

    0x42e3c066,// 151 PAY 148 

    0xfe1a69db,// 152 PAY 149 

    0xd1394d0c,// 153 PAY 150 

    0xb22efe1e,// 154 PAY 151 

    0x0fb59192,// 155 PAY 152 

    0x411a39f5,// 156 PAY 153 

    0x600b3bac,// 157 PAY 154 

    0xbda712dc,// 158 PAY 155 

    0x033bbf79,// 159 PAY 156 

    0x8d720e73,// 160 PAY 157 

    0x90c52c11,// 161 PAY 158 

    0x7b074b5b,// 162 PAY 159 

    0x11635b4b,// 163 PAY 160 

    0xf61b2e56,// 164 PAY 161 

    0x1cd6f4c0,// 165 PAY 162 

    0x4eded6c8,// 166 PAY 163 

    0x81a54c4d,// 167 PAY 164 

    0x3a5329b0,// 168 PAY 165 

    0x9e408791,// 169 PAY 166 

    0x46f81498,// 170 PAY 167 

    0xce15539e,// 171 PAY 168 

    0xe5d0a4ce,// 172 PAY 169 

    0x7012cdb1,// 173 PAY 170 

    0x7147741f,// 174 PAY 171 

    0x9aa39799,// 175 PAY 172 

    0xf3dee3fe,// 176 PAY 173 

    0x30eebdf9,// 177 PAY 174 

    0x268b7c93,// 178 PAY 175 

    0x66aa2cff,// 179 PAY 176 

    0x9627cbb2,// 180 PAY 177 

    0x72435d4e,// 181 PAY 178 

    0xb75d6187,// 182 PAY 179 

    0xd7cef32f,// 183 PAY 180 

    0x2421b92d,// 184 PAY 181 

    0x691689c3,// 185 PAY 182 

    0xaba18801,// 186 PAY 183 

    0x2814a9cd,// 187 PAY 184 

    0x40b3c0dc,// 188 PAY 185 

    0xf22df363,// 189 PAY 186 

    0xf10488ad,// 190 PAY 187 

    0xb20bc1aa,// 191 PAY 188 

    0xb0d05aee,// 192 PAY 189 

    0x5a834f2f,// 193 PAY 190 

    0x2ca4165c,// 194 PAY 191 

    0xff6b4354,// 195 PAY 192 

    0xb363dbbf,// 196 PAY 193 

    0xb0dce2ce,// 197 PAY 194 

    0xeb891d86,// 198 PAY 195 

    0x8847964b,// 199 PAY 196 

    0x55e699f6,// 200 PAY 197 

    0x863aa4e8,// 201 PAY 198 

    0xd054d6ac,// 202 PAY 199 

    0x9b46a27b,// 203 PAY 200 

    0x8e8d22bd,// 204 PAY 201 

    0xdb1e684b,// 205 PAY 202 

    0xfaea1aa1,// 206 PAY 203 

    0x6efb5b18,// 207 PAY 204 

    0xc62f8187,// 208 PAY 205 

    0xb169283e,// 209 PAY 206 

    0xdea18f38,// 210 PAY 207 

    0x7c54ee1d,// 211 PAY 208 

    0x3deb6f49,// 212 PAY 209 

    0x913bf9c7,// 213 PAY 210 

    0xd71943a0,// 214 PAY 211 

    0x5236997c,// 215 PAY 212 

    0xefb4cbb7,// 216 PAY 213 

    0x8675a6fe,// 217 PAY 214 

    0x100e721f,// 218 PAY 215 

    0xe482b4fc,// 219 PAY 216 

    0x8a7aefb8,// 220 PAY 217 

    0x7fcea9b9,// 221 PAY 218 

    0x2d0a576b,// 222 PAY 219 

    0xbddd229c,// 223 PAY 220 

    0x631aef47,// 224 PAY 221 

    0xc6164e52,// 225 PAY 222 

    0x4b9ace60,// 226 PAY 223 

    0xd34e5ae3,// 227 PAY 224 

    0x7df6b48b,// 228 PAY 225 

    0xabfe1ec2,// 229 PAY 226 

    0x901b8241,// 230 PAY 227 

    0x8720ccdf,// 231 PAY 228 

    0x3a4dd39d,// 232 PAY 229 

    0x58e4ec93,// 233 PAY 230 

    0xb206fe74,// 234 PAY 231 

    0x367e0c49,// 235 PAY 232 

    0xe38403f5,// 236 PAY 233 

    0xdc31e3c5,// 237 PAY 234 

    0x5f58f2be,// 238 PAY 235 

    0x7827337a,// 239 PAY 236 

    0x8bfe249a,// 240 PAY 237 

    0x082994da,// 241 PAY 238 

    0xab8f694b,// 242 PAY 239 

    0xec0455d7,// 243 PAY 240 

    0x730a0ccf,// 244 PAY 241 

    0xa987cfbe,// 245 PAY 242 

    0x67409e6a,// 246 PAY 243 

    0xacbbb3a0,// 247 PAY 244 

    0x5e849e86,// 248 PAY 245 

    0x4f8c997a,// 249 PAY 246 

    0x1c288e4d,// 250 PAY 247 

    0xc43fb796,// 251 PAY 248 

    0xf67542fc,// 252 PAY 249 

    0xfac9db84,// 253 PAY 250 

    0x12403215,// 254 PAY 251 

    0x8ceb1388,// 255 PAY 252 

    0xa7b01db4,// 256 PAY 253 

    0xe89ffae8,// 257 PAY 254 

    0xb05372d6,// 258 PAY 255 

    0xa4947a88,// 259 PAY 256 

    0x8211d4f0,// 260 PAY 257 

    0x01ffb03b,// 261 PAY 258 

    0xb15bfcdc,// 262 PAY 259 

    0xa00dec10,// 263 PAY 260 

    0x0f07a8d1,// 264 PAY 261 

    0xab2ada92,// 265 PAY 262 

    0xd6a3dc94,// 266 PAY 263 

    0xfa65c286,// 267 PAY 264 

    0x23086f05,// 268 PAY 265 

    0xfe6f3069,// 269 PAY 266 

    0x3c3c693a,// 270 PAY 267 

    0x054784a0,// 271 PAY 268 

    0x31074d06,// 272 PAY 269 

    0xb5684b75,// 273 PAY 270 

    0x3da12273,// 274 PAY 271 

    0x9ed9e0ec,// 275 PAY 272 

    0x030bfa87,// 276 PAY 273 

    0xd8032d8d,// 277 PAY 274 

    0x57fa1918,// 278 PAY 275 

    0x1aad5a56,// 279 PAY 276 

    0x945b1333,// 280 PAY 277 

    0x8dd48205,// 281 PAY 278 

    0x27e7fc1f,// 282 PAY 279 

    0x28d83440,// 283 PAY 280 

    0x565f1ca1,// 284 PAY 281 

    0xecc970b7,// 285 PAY 282 

    0x4051b677,// 286 PAY 283 

    0x8314ccd8,// 287 PAY 284 

    0xc5eb6f6a,// 288 PAY 285 

    0x0ff07c96,// 289 PAY 286 

    0xadfc5516,// 290 PAY 287 

    0xcc4e41da,// 291 PAY 288 

    0x196b48cb,// 292 PAY 289 

    0xd08775aa,// 293 PAY 290 

    0x0f402a63,// 294 PAY 291 

    0x44916331,// 295 PAY 292 

    0x041f0520,// 296 PAY 293 

    0x6bffce90,// 297 PAY 294 

    0x0f7011a5,// 298 PAY 295 

    0xecfa8423,// 299 PAY 296 

    0x9f258aa5,// 300 PAY 297 

    0x281cbf17,// 301 PAY 298 

    0x9ba3f553,// 302 PAY 299 

    0xbc069319,// 303 PAY 300 

    0x61955b5a,// 304 PAY 301 

    0x6a92dddf,// 305 PAY 302 

    0xfa1589cb,// 306 PAY 303 

    0xae124ae5,// 307 PAY 304 

    0xe0b8618b,// 308 PAY 305 

    0x53117441,// 309 PAY 306 

    0x56b0ddc8,// 310 PAY 307 

    0x5f3865ad,// 311 PAY 308 

    0x7da0d06c,// 312 PAY 309 

    0xead315f0,// 313 PAY 310 

    0xbcdc841d,// 314 PAY 311 

    0x26acda4f,// 315 PAY 312 

    0x394bd7a0,// 316 PAY 313 

    0x8c8dbe70,// 317 PAY 314 

    0x5e018770,// 318 PAY 315 

    0xe9182a4a,// 319 PAY 316 

    0xca7250b3,// 320 PAY 317 

    0x4441250c,// 321 PAY 318 

    0xd3c15dfd,// 322 PAY 319 

    0x5fa097f0,// 323 PAY 320 

    0x93ed43cc,// 324 PAY 321 

    0x1ccc5d94,// 325 PAY 322 

    0x2ec5f415,// 326 PAY 323 

    0x723d2198,// 327 PAY 324 

    0xde1c9150,// 328 PAY 325 

    0x83cbf64d,// 329 PAY 326 

    0x7072ff52,// 330 PAY 327 

    0xaef836a0,// 331 PAY 328 

    0x79ba1ae1,// 332 PAY 329 

    0x69becff5,// 333 PAY 330 

    0x3a1c0ac0,// 334 PAY 331 

    0xa9998935,// 335 PAY 332 

    0x608f00db,// 336 PAY 333 

    0xbcd8b78f,// 337 PAY 334 

    0xd64f1d54,// 338 PAY 335 

    0xcffc5051,// 339 PAY 336 

    0x40a5e207,// 340 PAY 337 

    0xc0b4458c,// 341 PAY 338 

    0x08611fdc,// 342 PAY 339 

    0x72948bed,// 343 PAY 340 

    0x07233761,// 344 PAY 341 

    0x66350b8d,// 345 PAY 342 

    0x19c61606,// 346 PAY 343 

    0xd72eb0de,// 347 PAY 344 

    0x2850fada,// 348 PAY 345 

    0x5d349cba,// 349 PAY 346 

    0xcc9f9f6b,// 350 PAY 347 

    0xe015e97f,// 351 PAY 348 

    0x22c12daa,// 352 PAY 349 

    0xab9ad980,// 353 PAY 350 

    0x2642dde8,// 354 PAY 351 

    0xdb0f459c,// 355 PAY 352 

    0x16d0949f,// 356 PAY 353 

    0xede1a76d,// 357 PAY 354 

    0xb7ff6532,// 358 PAY 355 

    0x2624b5e8,// 359 PAY 356 

    0x76efd4fb,// 360 PAY 357 

    0x1f359db2,// 361 PAY 358 

    0x8f1b50e1,// 362 PAY 359 

    0x1dd446c5,// 363 PAY 360 

    0xef59c556,// 364 PAY 361 

    0x3eebbe29,// 365 PAY 362 

    0xf1b783fd,// 366 PAY 363 

    0x523575ae,// 367 PAY 364 

    0x0dd0ea5c,// 368 PAY 365 

    0xfdc99a37,// 369 PAY 366 

    0x00963b62,// 370 PAY 367 

    0x99713762,// 371 PAY 368 

    0xa0063407,// 372 PAY 369 

    0x89113364,// 373 PAY 370 

    0x086386b9,// 374 PAY 371 

    0xfbedf101,// 375 PAY 372 

    0xad423951,// 376 PAY 373 

    0xb858f229,// 377 PAY 374 

    0x4e1f4400,// 378 PAY 375 

    0x2da8a955,// 379 PAY 376 

    0x81f877a1,// 380 PAY 377 

    0x3cbccedc,// 381 PAY 378 

    0x51f9a3ff,// 382 PAY 379 

    0x783d7f74,// 383 PAY 380 

    0x6c4d2011,// 384 PAY 381 

    0x46696f0a,// 385 PAY 382 

    0xc42713dc,// 386 PAY 383 

    0x447f65d2,// 387 PAY 384 

    0x9eb80b7b,// 388 PAY 385 

    0xfa9f5fc7,// 389 PAY 386 

    0x195e9885,// 390 PAY 387 

    0xc22de9b3,// 391 PAY 388 

    0xb39c6805,// 392 PAY 389 

    0xf38fffbe,// 393 PAY 390 

    0xee53baa2,// 394 PAY 391 

    0x16c3e49e,// 395 PAY 392 

    0x4c31b1e4,// 396 PAY 393 

    0x1647e421,// 397 PAY 394 

    0x5ced6fe4,// 398 PAY 395 

    0xffc8ebe3,// 399 PAY 396 

    0x8200bbc0,// 400 PAY 397 

    0x577aa2bb,// 401 PAY 398 

    0x4a2808cd,// 402 PAY 399 

    0x06afcb35,// 403 PAY 400 

    0x14a0bb5b,// 404 PAY 401 

    0xb3ba0fc9,// 405 PAY 402 

    0xc5e90469,// 406 PAY 403 

    0x77a756e8,// 407 PAY 404 

    0xd48b61b4,// 408 PAY 405 

    0x0829650b,// 409 PAY 406 

    0x011084c1,// 410 PAY 407 

    0x8d73b55d,// 411 PAY 408 

    0xe86c1d0a,// 412 PAY 409 

    0x508f99d2,// 413 PAY 410 

    0xd81c644d,// 414 PAY 411 

    0xa964ab09,// 415 PAY 412 

    0xaf7f7cac,// 416 PAY 413 

/// HASH is  4 bytes 

    0x08827483,// 417 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 76 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x0160804c // 418 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt23_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 284 words. 

/// BDA size     is 1131 (0x46b) 

/// BDA id       is 0x6a15 

    0x046b6a15,// 3 BDA   1 

/// PAY Generic Data size   : 1131 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x9c72253c,// 4 PAY   1 

    0xe1c5b1b3,// 5 PAY   2 

    0x3eeca832,// 6 PAY   3 

    0xd9b6052a,// 7 PAY   4 

    0x686709a7,// 8 PAY   5 

    0xd3608b6f,// 9 PAY   6 

    0xfffbb6f0,// 10 PAY   7 

    0x6571bc8d,// 11 PAY   8 

    0x5e357df0,// 12 PAY   9 

    0x759a57f4,// 13 PAY  10 

    0x1d18a297,// 14 PAY  11 

    0xf3aea47f,// 15 PAY  12 

    0xabaf006d,// 16 PAY  13 

    0x0d4f1c68,// 17 PAY  14 

    0x6e60d984,// 18 PAY  15 

    0xdf4139b4,// 19 PAY  16 

    0x215f0c2a,// 20 PAY  17 

    0x3f128837,// 21 PAY  18 

    0xd7bf3830,// 22 PAY  19 

    0x918ac106,// 23 PAY  20 

    0x23e41595,// 24 PAY  21 

    0xd65601fb,// 25 PAY  22 

    0x21c2743c,// 26 PAY  23 

    0x91372caf,// 27 PAY  24 

    0x3d58574f,// 28 PAY  25 

    0xeb6f9d15,// 29 PAY  26 

    0xae5c513f,// 30 PAY  27 

    0x069edebe,// 31 PAY  28 

    0x20601538,// 32 PAY  29 

    0xea7a2f70,// 33 PAY  30 

    0xdd96bf5e,// 34 PAY  31 

    0xf67eb667,// 35 PAY  32 

    0x3c3a1daa,// 36 PAY  33 

    0xaf859d40,// 37 PAY  34 

    0x4f00dd1f,// 38 PAY  35 

    0xd1d0e712,// 39 PAY  36 

    0x4535aea6,// 40 PAY  37 

    0xd49111ce,// 41 PAY  38 

    0x00cfdb89,// 42 PAY  39 

    0x93a27741,// 43 PAY  40 

    0xd82d2464,// 44 PAY  41 

    0xba3b5c1c,// 45 PAY  42 

    0x1b35fcf0,// 46 PAY  43 

    0x3554f991,// 47 PAY  44 

    0x1c337275,// 48 PAY  45 

    0x609b12c4,// 49 PAY  46 

    0x5394dabe,// 50 PAY  47 

    0x5a77bb68,// 51 PAY  48 

    0xe1b125e8,// 52 PAY  49 

    0xca2ff296,// 53 PAY  50 

    0x9c984c61,// 54 PAY  51 

    0x7e896a54,// 55 PAY  52 

    0x53dd02d0,// 56 PAY  53 

    0x7eadb562,// 57 PAY  54 

    0x8fbd92e2,// 58 PAY  55 

    0x334ce669,// 59 PAY  56 

    0xb0f45eb0,// 60 PAY  57 

    0x23ec22ba,// 61 PAY  58 

    0xaac8a4b2,// 62 PAY  59 

    0x4bf1fa9b,// 63 PAY  60 

    0x07bbecb4,// 64 PAY  61 

    0xe71d5e67,// 65 PAY  62 

    0x7f9ae1a8,// 66 PAY  63 

    0xaaf1663d,// 67 PAY  64 

    0x7334742e,// 68 PAY  65 

    0x896a27c5,// 69 PAY  66 

    0x961ff471,// 70 PAY  67 

    0xeb172cb5,// 71 PAY  68 

    0x464e7ab1,// 72 PAY  69 

    0x46254c0b,// 73 PAY  70 

    0xe7167be1,// 74 PAY  71 

    0xca492531,// 75 PAY  72 

    0x09985603,// 76 PAY  73 

    0x7fc0507b,// 77 PAY  74 

    0x0c3de502,// 78 PAY  75 

    0x84fc822c,// 79 PAY  76 

    0xec87d0c0,// 80 PAY  77 

    0xa81cff77,// 81 PAY  78 

    0xbff8fa17,// 82 PAY  79 

    0xc5a9c3f0,// 83 PAY  80 

    0x8efc5f5d,// 84 PAY  81 

    0x630f2c00,// 85 PAY  82 

    0x1bf8afac,// 86 PAY  83 

    0x37e72f5e,// 87 PAY  84 

    0x5c92f060,// 88 PAY  85 

    0xc262ce29,// 89 PAY  86 

    0x9c856ade,// 90 PAY  87 

    0x751b00d7,// 91 PAY  88 

    0x81fa0c2d,// 92 PAY  89 

    0x2252387d,// 93 PAY  90 

    0x96953931,// 94 PAY  91 

    0x1f077641,// 95 PAY  92 

    0xcca1c566,// 96 PAY  93 

    0xb1f3b200,// 97 PAY  94 

    0x035f6c3f,// 98 PAY  95 

    0x40ae7d8e,// 99 PAY  96 

    0x5a288352,// 100 PAY  97 

    0xa72a687e,// 101 PAY  98 

    0x0448c21d,// 102 PAY  99 

    0xbb6f5487,// 103 PAY 100 

    0xb8f252df,// 104 PAY 101 

    0x74ccb5f0,// 105 PAY 102 

    0x1aece0ce,// 106 PAY 103 

    0x910edfea,// 107 PAY 104 

    0x7189e639,// 108 PAY 105 

    0xfd8e718a,// 109 PAY 106 

    0x68b29105,// 110 PAY 107 

    0x1f4be6b8,// 111 PAY 108 

    0x6e510058,// 112 PAY 109 

    0xee5a9d7f,// 113 PAY 110 

    0x5b3c13e7,// 114 PAY 111 

    0xac4ae5d3,// 115 PAY 112 

    0x0e998aa2,// 116 PAY 113 

    0x8bc26b4c,// 117 PAY 114 

    0x89273a69,// 118 PAY 115 

    0x7d779966,// 119 PAY 116 

    0xccb59f31,// 120 PAY 117 

    0x2f8fee43,// 121 PAY 118 

    0xe1416717,// 122 PAY 119 

    0x4c593f9f,// 123 PAY 120 

    0x6388fd72,// 124 PAY 121 

    0x51882379,// 125 PAY 122 

    0xb31a8e39,// 126 PAY 123 

    0x210a39dd,// 127 PAY 124 

    0x567df4b3,// 128 PAY 125 

    0xa723a060,// 129 PAY 126 

    0x682bae00,// 130 PAY 127 

    0x104eac15,// 131 PAY 128 

    0xb7fe6b27,// 132 PAY 129 

    0x017cfb34,// 133 PAY 130 

    0x4e1b6328,// 134 PAY 131 

    0x7f0ccc04,// 135 PAY 132 

    0x88daa6e3,// 136 PAY 133 

    0x6d59f5e7,// 137 PAY 134 

    0xf9e8d79d,// 138 PAY 135 

    0x0431fff6,// 139 PAY 136 

    0xeb883f40,// 140 PAY 137 

    0x5d090735,// 141 PAY 138 

    0xfa8d49d8,// 142 PAY 139 

    0x74672930,// 143 PAY 140 

    0xd8d99a54,// 144 PAY 141 

    0xba66c8fc,// 145 PAY 142 

    0x5a4767a0,// 146 PAY 143 

    0x6b2451bf,// 147 PAY 144 

    0x583abf34,// 148 PAY 145 

    0x5b27f07e,// 149 PAY 146 

    0x4a348499,// 150 PAY 147 

    0x8a76c64b,// 151 PAY 148 

    0xc848e899,// 152 PAY 149 

    0x1442dba6,// 153 PAY 150 

    0xbf383f4e,// 154 PAY 151 

    0x1805d380,// 155 PAY 152 

    0x35d3f012,// 156 PAY 153 

    0x542c818b,// 157 PAY 154 

    0xa1be5ed3,// 158 PAY 155 

    0x8dd7769b,// 159 PAY 156 

    0x762f676e,// 160 PAY 157 

    0x6c3a8867,// 161 PAY 158 

    0xc7cab5b6,// 162 PAY 159 

    0xfd387165,// 163 PAY 160 

    0x78814fb2,// 164 PAY 161 

    0x0a3ad9a0,// 165 PAY 162 

    0x5389794b,// 166 PAY 163 

    0xcae715cf,// 167 PAY 164 

    0x4206ffa9,// 168 PAY 165 

    0x650f4d0b,// 169 PAY 166 

    0xe14807b6,// 170 PAY 167 

    0x99add00c,// 171 PAY 168 

    0x1370f00d,// 172 PAY 169 

    0x657406f6,// 173 PAY 170 

    0x6d4d56d3,// 174 PAY 171 

    0x811d15b7,// 175 PAY 172 

    0x0bf9e5c9,// 176 PAY 173 

    0x4f762f39,// 177 PAY 174 

    0x47253758,// 178 PAY 175 

    0x10b2379d,// 179 PAY 176 

    0x2dc83fff,// 180 PAY 177 

    0x7239345e,// 181 PAY 178 

    0xd0a9f877,// 182 PAY 179 

    0x432a992a,// 183 PAY 180 

    0x9cd6a7dd,// 184 PAY 181 

    0xd5395ff3,// 185 PAY 182 

    0x49141133,// 186 PAY 183 

    0x0bf069a8,// 187 PAY 184 

    0x82fe5860,// 188 PAY 185 

    0x50c3415b,// 189 PAY 186 

    0x31081b9d,// 190 PAY 187 

    0xc60229d0,// 191 PAY 188 

    0xf74657f6,// 192 PAY 189 

    0x897922ef,// 193 PAY 190 

    0x38af1e60,// 194 PAY 191 

    0x6a2593da,// 195 PAY 192 

    0xc17db52a,// 196 PAY 193 

    0x6d1b3364,// 197 PAY 194 

    0x246ac585,// 198 PAY 195 

    0x09b21465,// 199 PAY 196 

    0x4a4e177f,// 200 PAY 197 

    0x8dae3465,// 201 PAY 198 

    0x958ff289,// 202 PAY 199 

    0x547358fe,// 203 PAY 200 

    0x09d8aad5,// 204 PAY 201 

    0x28a25697,// 205 PAY 202 

    0x18ff95e0,// 206 PAY 203 

    0xf9c12193,// 207 PAY 204 

    0xa6513a8c,// 208 PAY 205 

    0x8c215bfb,// 209 PAY 206 

    0xdee9106d,// 210 PAY 207 

    0x5122db8f,// 211 PAY 208 

    0x41a38880,// 212 PAY 209 

    0x96c2c08c,// 213 PAY 210 

    0x9a130361,// 214 PAY 211 

    0x70da903f,// 215 PAY 212 

    0x9f78cf48,// 216 PAY 213 

    0x6865e75a,// 217 PAY 214 

    0xdb27ddc7,// 218 PAY 215 

    0x9cf18611,// 219 PAY 216 

    0xcdfb8bad,// 220 PAY 217 

    0x661dd859,// 221 PAY 218 

    0x36fc3421,// 222 PAY 219 

    0xcaba4698,// 223 PAY 220 

    0x4d4d32f4,// 224 PAY 221 

    0xe58cee2c,// 225 PAY 222 

    0xa1245d21,// 226 PAY 223 

    0x12a6db22,// 227 PAY 224 

    0xd3515355,// 228 PAY 225 

    0xc565a8a6,// 229 PAY 226 

    0x83790d20,// 230 PAY 227 

    0xaa784fbe,// 231 PAY 228 

    0x4e042d6e,// 232 PAY 229 

    0x39826af5,// 233 PAY 230 

    0x9c3d85f1,// 234 PAY 231 

    0x1e4f9265,// 235 PAY 232 

    0x987f8158,// 236 PAY 233 

    0x1ed46684,// 237 PAY 234 

    0x22aff49e,// 238 PAY 235 

    0xf77b90d7,// 239 PAY 236 

    0x031968d9,// 240 PAY 237 

    0x311edd04,// 241 PAY 238 

    0x2e7a4a40,// 242 PAY 239 

    0x463ecece,// 243 PAY 240 

    0xf41032da,// 244 PAY 241 

    0x1635ecf2,// 245 PAY 242 

    0x4b889296,// 246 PAY 243 

    0x9d23944f,// 247 PAY 244 

    0x42805ddd,// 248 PAY 245 

    0x02cbce32,// 249 PAY 246 

    0x3b4de8c5,// 250 PAY 247 

    0xbfe15359,// 251 PAY 248 

    0x677f5319,// 252 PAY 249 

    0x71272ff8,// 253 PAY 250 

    0x88b2d154,// 254 PAY 251 

    0x9f0130bc,// 255 PAY 252 

    0x7bcc7d06,// 256 PAY 253 

    0xee1434b5,// 257 PAY 254 

    0x12cd0abd,// 258 PAY 255 

    0x4975fcc2,// 259 PAY 256 

    0xb9ab1582,// 260 PAY 257 

    0x1e90225b,// 261 PAY 258 

    0x1f400f70,// 262 PAY 259 

    0xc6893a07,// 263 PAY 260 

    0x3f314454,// 264 PAY 261 

    0xca4045fc,// 265 PAY 262 

    0xd56674c9,// 266 PAY 263 

    0xa789b3a3,// 267 PAY 264 

    0x7f4f3d1c,// 268 PAY 265 

    0x9e10bb0d,// 269 PAY 266 

    0x70ac7c14,// 270 PAY 267 

    0x8efef134,// 271 PAY 268 

    0x2ab19480,// 272 PAY 269 

    0x954eed78,// 273 PAY 270 

    0xed8a0244,// 274 PAY 271 

    0xa30a2e68,// 275 PAY 272 

    0xf5c8b771,// 276 PAY 273 

    0xf6f0cec2,// 277 PAY 274 

    0x4937cbbc,// 278 PAY 275 

    0x9954d862,// 279 PAY 276 

    0xb44e87b3,// 280 PAY 277 

    0x6d782326,// 281 PAY 278 

    0x10d34c02,// 282 PAY 279 

    0x601f684c,// 283 PAY 280 

    0xa8fdeba7,// 284 PAY 281 

    0x7d82fc7a,// 285 PAY 282 

    0x20d08f00,// 286 PAY 283 

/// STA is 1 words. 

/// STA num_pkts       : 23 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd8 

    0x03d9d817 // 287 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt24_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 366 words. 

/// BDA size     is 1457 (0x5b1) 

/// BDA id       is 0x2a9b 

    0x05b12a9b,// 3 BDA   1 

/// PAY Generic Data size   : 1457 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x2925892c,// 4 PAY   1 

    0x6e9e13a4,// 5 PAY   2 

    0x549ab820,// 6 PAY   3 

    0x63c6fec9,// 7 PAY   4 

    0x43707279,// 8 PAY   5 

    0xbf7a3c55,// 9 PAY   6 

    0x30696545,// 10 PAY   7 

    0xdc215c81,// 11 PAY   8 

    0x61ed6494,// 12 PAY   9 

    0x4e062b47,// 13 PAY  10 

    0x0b6d1f5f,// 14 PAY  11 

    0x657715d3,// 15 PAY  12 

    0xa2373b06,// 16 PAY  13 

    0xb183ef53,// 17 PAY  14 

    0x3c4e5545,// 18 PAY  15 

    0xbe2f2fd7,// 19 PAY  16 

    0xcd0f28ba,// 20 PAY  17 

    0x0ede6b6a,// 21 PAY  18 

    0x10fbddb8,// 22 PAY  19 

    0xc9391b86,// 23 PAY  20 

    0x97d6d358,// 24 PAY  21 

    0xe25449d8,// 25 PAY  22 

    0x1dff35db,// 26 PAY  23 

    0xcef21b91,// 27 PAY  24 

    0xcb502799,// 28 PAY  25 

    0xeb09a772,// 29 PAY  26 

    0xd28bdac0,// 30 PAY  27 

    0x1b35a946,// 31 PAY  28 

    0x6b5fb813,// 32 PAY  29 

    0x40233fd3,// 33 PAY  30 

    0x69f71886,// 34 PAY  31 

    0x54d8a8bd,// 35 PAY  32 

    0x8fab434e,// 36 PAY  33 

    0x0bb355be,// 37 PAY  34 

    0xa09dd0bb,// 38 PAY  35 

    0x30458666,// 39 PAY  36 

    0x489f19c4,// 40 PAY  37 

    0x24f656f0,// 41 PAY  38 

    0xcf3bc443,// 42 PAY  39 

    0x6067a823,// 43 PAY  40 

    0x9ba9a018,// 44 PAY  41 

    0x4fb7c7ca,// 45 PAY  42 

    0xfa2806f6,// 46 PAY  43 

    0x51463dbd,// 47 PAY  44 

    0x266a290a,// 48 PAY  45 

    0xdad4e60a,// 49 PAY  46 

    0x708604a3,// 50 PAY  47 

    0x88d84a7f,// 51 PAY  48 

    0x3e5ac9a9,// 52 PAY  49 

    0x235e3d9f,// 53 PAY  50 

    0x6447663f,// 54 PAY  51 

    0xc2f87018,// 55 PAY  52 

    0xb28e39b2,// 56 PAY  53 

    0x4085fe54,// 57 PAY  54 

    0xab9ec9c1,// 58 PAY  55 

    0xe42fcfa7,// 59 PAY  56 

    0xf5f78ff4,// 60 PAY  57 

    0xf68a204f,// 61 PAY  58 

    0xf5258b62,// 62 PAY  59 

    0xf30785fc,// 63 PAY  60 

    0xbc59fd91,// 64 PAY  61 

    0x94732e91,// 65 PAY  62 

    0x8d2312d4,// 66 PAY  63 

    0x071d254d,// 67 PAY  64 

    0xa1f5165f,// 68 PAY  65 

    0xda0fff3d,// 69 PAY  66 

    0x148ca7b7,// 70 PAY  67 

    0xaf6243ea,// 71 PAY  68 

    0x359f05e6,// 72 PAY  69 

    0xa7107044,// 73 PAY  70 

    0x23a654b7,// 74 PAY  71 

    0x4fe1fa57,// 75 PAY  72 

    0x10f97228,// 76 PAY  73 

    0x5060b8c1,// 77 PAY  74 

    0xf9cd9e16,// 78 PAY  75 

    0xfc114b16,// 79 PAY  76 

    0x4f942479,// 80 PAY  77 

    0x85c76346,// 81 PAY  78 

    0xbe21d250,// 82 PAY  79 

    0x50db7306,// 83 PAY  80 

    0xcf614709,// 84 PAY  81 

    0x84998627,// 85 PAY  82 

    0xb9e0834b,// 86 PAY  83 

    0xc76509be,// 87 PAY  84 

    0x5a9b0d1c,// 88 PAY  85 

    0x369eaaa4,// 89 PAY  86 

    0xdaf6790e,// 90 PAY  87 

    0xac445c77,// 91 PAY  88 

    0xb6ba6810,// 92 PAY  89 

    0xd3926b07,// 93 PAY  90 

    0x302b3d81,// 94 PAY  91 

    0x298184ed,// 95 PAY  92 

    0x67f93e47,// 96 PAY  93 

    0x01f122b2,// 97 PAY  94 

    0x7b78c098,// 98 PAY  95 

    0x6950f19a,// 99 PAY  96 

    0x1fdd3c9e,// 100 PAY  97 

    0xcc6b88c0,// 101 PAY  98 

    0x861ccab4,// 102 PAY  99 

    0x1d00b8ce,// 103 PAY 100 

    0x4d537828,// 104 PAY 101 

    0x474d3dba,// 105 PAY 102 

    0x2055ede8,// 106 PAY 103 

    0xd8757278,// 107 PAY 104 

    0xc9b23765,// 108 PAY 105 

    0x8e870fff,// 109 PAY 106 

    0x4c38ec13,// 110 PAY 107 

    0x9d72af98,// 111 PAY 108 

    0x788bfe87,// 112 PAY 109 

    0xe10df9a8,// 113 PAY 110 

    0xd1cbc666,// 114 PAY 111 

    0xa220a256,// 115 PAY 112 

    0x77a240ff,// 116 PAY 113 

    0x3646d7cf,// 117 PAY 114 

    0x0d916670,// 118 PAY 115 

    0x7caea41e,// 119 PAY 116 

    0x8c0253c1,// 120 PAY 117 

    0xbde8eba9,// 121 PAY 118 

    0x6ea54282,// 122 PAY 119 

    0x31719bcd,// 123 PAY 120 

    0x030d0fe7,// 124 PAY 121 

    0x95b0c7a6,// 125 PAY 122 

    0x2894f89a,// 126 PAY 123 

    0x0e8eb43d,// 127 PAY 124 

    0xbcb5efca,// 128 PAY 125 

    0xf348c518,// 129 PAY 126 

    0x6eab1f9c,// 130 PAY 127 

    0x912e8066,// 131 PAY 128 

    0x246597cf,// 132 PAY 129 

    0xef244605,// 133 PAY 130 

    0x4baf4282,// 134 PAY 131 

    0xffcebe5f,// 135 PAY 132 

    0x89a2662a,// 136 PAY 133 

    0x70f066d9,// 137 PAY 134 

    0xbcd42656,// 138 PAY 135 

    0x95f76f9d,// 139 PAY 136 

    0x6b51a52f,// 140 PAY 137 

    0x8436f1f7,// 141 PAY 138 

    0xc5c1183a,// 142 PAY 139 

    0x88f334c0,// 143 PAY 140 

    0x6997b0b8,// 144 PAY 141 

    0x44737bba,// 145 PAY 142 

    0x54da4072,// 146 PAY 143 

    0x09cdf752,// 147 PAY 144 

    0x2a1da16d,// 148 PAY 145 

    0xcefe4e10,// 149 PAY 146 

    0x3d0ef6ee,// 150 PAY 147 

    0x836d983d,// 151 PAY 148 

    0xf71394d6,// 152 PAY 149 

    0xbb97319b,// 153 PAY 150 

    0x3401ec65,// 154 PAY 151 

    0x2c83f2a3,// 155 PAY 152 

    0xae86afc0,// 156 PAY 153 

    0x494f58cb,// 157 PAY 154 

    0x4e46ae3d,// 158 PAY 155 

    0x2bcb5b61,// 159 PAY 156 

    0xddfb7e91,// 160 PAY 157 

    0x4a8750fc,// 161 PAY 158 

    0x1b0927ab,// 162 PAY 159 

    0x413d05a5,// 163 PAY 160 

    0x692d7c13,// 164 PAY 161 

    0x28421a61,// 165 PAY 162 

    0x03f60817,// 166 PAY 163 

    0x55a094fa,// 167 PAY 164 

    0xc159a35d,// 168 PAY 165 

    0x1f468196,// 169 PAY 166 

    0xec7a55d5,// 170 PAY 167 

    0x9b4236b3,// 171 PAY 168 

    0x4cb94f02,// 172 PAY 169 

    0x22aebf68,// 173 PAY 170 

    0xdddbec70,// 174 PAY 171 

    0x957eeb0b,// 175 PAY 172 

    0xcd980a45,// 176 PAY 173 

    0x6b90d376,// 177 PAY 174 

    0xcb425232,// 178 PAY 175 

    0xab523b04,// 179 PAY 176 

    0x38ef2afb,// 180 PAY 177 

    0xa6c89718,// 181 PAY 178 

    0x5f8d49c6,// 182 PAY 179 

    0x4091aeaf,// 183 PAY 180 

    0x969a6c93,// 184 PAY 181 

    0x094ae59c,// 185 PAY 182 

    0x01fa44b0,// 186 PAY 183 

    0x6e065ef2,// 187 PAY 184 

    0x40b4dd19,// 188 PAY 185 

    0x03398b74,// 189 PAY 186 

    0x95eee99f,// 190 PAY 187 

    0xd93f9154,// 191 PAY 188 

    0x44b281ac,// 192 PAY 189 

    0x3bf552a6,// 193 PAY 190 

    0x8e82e2b9,// 194 PAY 191 

    0x1799ef59,// 195 PAY 192 

    0x84d9b2b7,// 196 PAY 193 

    0xb33dca79,// 197 PAY 194 

    0x01106e4d,// 198 PAY 195 

    0x6b1c91d8,// 199 PAY 196 

    0xb6d2cb0d,// 200 PAY 197 

    0x05f6a94c,// 201 PAY 198 

    0x36301011,// 202 PAY 199 

    0x3fa25cd4,// 203 PAY 200 

    0xd1d9995a,// 204 PAY 201 

    0x383265cf,// 205 PAY 202 

    0xb745cb6a,// 206 PAY 203 

    0xbc4781e9,// 207 PAY 204 

    0xc4da061f,// 208 PAY 205 

    0x769c7142,// 209 PAY 206 

    0x0422d762,// 210 PAY 207 

    0x7ba5fa5c,// 211 PAY 208 

    0xb9024a7f,// 212 PAY 209 

    0x0b78f4c8,// 213 PAY 210 

    0x1fe4af63,// 214 PAY 211 

    0x3b81dbf0,// 215 PAY 212 

    0xf6043690,// 216 PAY 213 

    0xc0b9698d,// 217 PAY 214 

    0x91d3fe47,// 218 PAY 215 

    0x9dbccc99,// 219 PAY 216 

    0x345bc19a,// 220 PAY 217 

    0xefe25556,// 221 PAY 218 

    0x5aac0377,// 222 PAY 219 

    0xa4df94f1,// 223 PAY 220 

    0x99a50150,// 224 PAY 221 

    0xb93b7c4a,// 225 PAY 222 

    0xf8d430c7,// 226 PAY 223 

    0xbed7b2ab,// 227 PAY 224 

    0x513089ca,// 228 PAY 225 

    0xa542a31f,// 229 PAY 226 

    0xda2e6a87,// 230 PAY 227 

    0x85024d40,// 231 PAY 228 

    0x6644c0e3,// 232 PAY 229 

    0x71a348b0,// 233 PAY 230 

    0x9b329b00,// 234 PAY 231 

    0x979b37d6,// 235 PAY 232 

    0xb4ec20e9,// 236 PAY 233 

    0xd46a738d,// 237 PAY 234 

    0x1dac5041,// 238 PAY 235 

    0x61e7ed33,// 239 PAY 236 

    0x5f53265b,// 240 PAY 237 

    0x64ae5326,// 241 PAY 238 

    0xd1556110,// 242 PAY 239 

    0x60e74c10,// 243 PAY 240 

    0x0facacdf,// 244 PAY 241 

    0xa7b61a02,// 245 PAY 242 

    0x140df53f,// 246 PAY 243 

    0x55ba32ed,// 247 PAY 244 

    0x1b911dd9,// 248 PAY 245 

    0xd2c7bb7b,// 249 PAY 246 

    0x12793d36,// 250 PAY 247 

    0x04937993,// 251 PAY 248 

    0xee940935,// 252 PAY 249 

    0xd487b034,// 253 PAY 250 

    0xf804c2a2,// 254 PAY 251 

    0xa5fbddc1,// 255 PAY 252 

    0xa177e7ea,// 256 PAY 253 

    0xaeeec320,// 257 PAY 254 

    0xb804c3e2,// 258 PAY 255 

    0x7fef1071,// 259 PAY 256 

    0x7a2d41e4,// 260 PAY 257 

    0xfc0d21d3,// 261 PAY 258 

    0x0afed52f,// 262 PAY 259 

    0x98d8e13b,// 263 PAY 260 

    0x9fcbb082,// 264 PAY 261 

    0x7ae4168a,// 265 PAY 262 

    0xce249e5f,// 266 PAY 263 

    0xf0d8f580,// 267 PAY 264 

    0x743518e9,// 268 PAY 265 

    0x771fff12,// 269 PAY 266 

    0xb1a36e70,// 270 PAY 267 

    0x8cf39d8a,// 271 PAY 268 

    0x9a8478ed,// 272 PAY 269 

    0xe1979b72,// 273 PAY 270 

    0xfd0a721d,// 274 PAY 271 

    0xd733021c,// 275 PAY 272 

    0x823cfbb3,// 276 PAY 273 

    0x29cff339,// 277 PAY 274 

    0xe4b6817c,// 278 PAY 275 

    0x258ec3b4,// 279 PAY 276 

    0x0c3418de,// 280 PAY 277 

    0x15a4eb2d,// 281 PAY 278 

    0x85ed4d87,// 282 PAY 279 

    0xda2824a7,// 283 PAY 280 

    0xedb16750,// 284 PAY 281 

    0xd4c6bf97,// 285 PAY 282 

    0x7be8fc17,// 286 PAY 283 

    0xe9af424a,// 287 PAY 284 

    0x74227c7d,// 288 PAY 285 

    0xc794277d,// 289 PAY 286 

    0xfa3bb3f5,// 290 PAY 287 

    0xc66b78b9,// 291 PAY 288 

    0x0cb215cb,// 292 PAY 289 

    0x5cb59d70,// 293 PAY 290 

    0xd73031a9,// 294 PAY 291 

    0x28d40085,// 295 PAY 292 

    0xea5c4fb5,// 296 PAY 293 

    0x0de93448,// 297 PAY 294 

    0x3cfa7142,// 298 PAY 295 

    0x0851234e,// 299 PAY 296 

    0x6bb09621,// 300 PAY 297 

    0x44fb28ed,// 301 PAY 298 

    0xc014056a,// 302 PAY 299 

    0xbad7e500,// 303 PAY 300 

    0x7731078a,// 304 PAY 301 

    0x9963e54a,// 305 PAY 302 

    0xb13665db,// 306 PAY 303 

    0xf0aa34a9,// 307 PAY 304 

    0xad96da86,// 308 PAY 305 

    0xab8055b3,// 309 PAY 306 

    0x4b72c9c2,// 310 PAY 307 

    0x93092018,// 311 PAY 308 

    0x721a18c2,// 312 PAY 309 

    0xee55d621,// 313 PAY 310 

    0xc9fc6bf4,// 314 PAY 311 

    0x6f6fa126,// 315 PAY 312 

    0xbfeff0a7,// 316 PAY 313 

    0xfe9811a7,// 317 PAY 314 

    0xb63fcb22,// 318 PAY 315 

    0x389fbdaa,// 319 PAY 316 

    0xf5d6e886,// 320 PAY 317 

    0xc30b05a3,// 321 PAY 318 

    0x4e5ab0e9,// 322 PAY 319 

    0x9f54d24d,// 323 PAY 320 

    0x38fae6c1,// 324 PAY 321 

    0x67cb9d76,// 325 PAY 322 

    0xc78e5169,// 326 PAY 323 

    0x88f74f7e,// 327 PAY 324 

    0xcbcd211b,// 328 PAY 325 

    0xe02c5729,// 329 PAY 326 

    0xd7572720,// 330 PAY 327 

    0xd785241e,// 331 PAY 328 

    0x3617f120,// 332 PAY 329 

    0x7f931991,// 333 PAY 330 

    0x0facf5eb,// 334 PAY 331 

    0x08ec273d,// 335 PAY 332 

    0x39f0574a,// 336 PAY 333 

    0xaed03c4a,// 337 PAY 334 

    0x386218fd,// 338 PAY 335 

    0x7da04401,// 339 PAY 336 

    0xdb25b1f1,// 340 PAY 337 

    0xefb689a9,// 341 PAY 338 

    0xdb5e3fb7,// 342 PAY 339 

    0xc15d08c7,// 343 PAY 340 

    0x8dd20ce4,// 344 PAY 341 

    0x1ba21329,// 345 PAY 342 

    0x613b247a,// 346 PAY 343 

    0x3f45b009,// 347 PAY 344 

    0xd7f20566,// 348 PAY 345 

    0x54e4b6ff,// 349 PAY 346 

    0x038ef826,// 350 PAY 347 

    0xf0735369,// 351 PAY 348 

    0x176eb81a,// 352 PAY 349 

    0x81bcf51d,// 353 PAY 350 

    0x09dd71e2,// 354 PAY 351 

    0x3ca34708,// 355 PAY 352 

    0x9ed60315,// 356 PAY 353 

    0xf9ddaa4e,// 357 PAY 354 

    0xe8548d3a,// 358 PAY 355 

    0x1d33acfe,// 359 PAY 356 

    0xe9ce01b5,// 360 PAY 357 

    0xb84189e5,// 361 PAY 358 

    0x85391d98,// 362 PAY 359 

    0x6cece17d,// 363 PAY 360 

    0x761939b0,// 364 PAY 361 

    0x40ad74dc,// 365 PAY 362 

    0x2933a88e,// 366 PAY 363 

    0x774deb6d,// 367 PAY 364 

    0x46000000,// 368 PAY 365 

/// STA is 1 words. 

/// STA num_pkts       : 151 

/// STA pkt_idx        : 156 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02703c97 // 369 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt25_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 322 words. 

/// BDA size     is 1284 (0x504) 

/// BDA id       is 0xdca0 

    0x0504dca0,// 3 BDA   1 

/// PAY Generic Data size   : 1284 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x837034ef,// 4 PAY   1 

    0x951db558,// 5 PAY   2 

    0xb7483f5f,// 6 PAY   3 

    0x046abba2,// 7 PAY   4 

    0xbb1a4c20,// 8 PAY   5 

    0x01d1db32,// 9 PAY   6 

    0x8285c5d7,// 10 PAY   7 

    0x3b1f4cf2,// 11 PAY   8 

    0x40fe26b9,// 12 PAY   9 

    0x0fa95211,// 13 PAY  10 

    0x33378c03,// 14 PAY  11 

    0xc5985311,// 15 PAY  12 

    0x01c1f203,// 16 PAY  13 

    0x1bc5869a,// 17 PAY  14 

    0x3902e8e1,// 18 PAY  15 

    0xde58e18f,// 19 PAY  16 

    0x674bc1ad,// 20 PAY  17 

    0x77392798,// 21 PAY  18 

    0x12a1ba28,// 22 PAY  19 

    0x1bd6bfa3,// 23 PAY  20 

    0x71c20d5c,// 24 PAY  21 

    0x1b53a5f5,// 25 PAY  22 

    0xc98f5e47,// 26 PAY  23 

    0xe5a35761,// 27 PAY  24 

    0xf8be6a5e,// 28 PAY  25 

    0x50bce4ef,// 29 PAY  26 

    0x8b509a01,// 30 PAY  27 

    0xd15288a6,// 31 PAY  28 

    0x15f1e268,// 32 PAY  29 

    0x227f6439,// 33 PAY  30 

    0x4bf499d1,// 34 PAY  31 

    0x94d80585,// 35 PAY  32 

    0x33bc1c42,// 36 PAY  33 

    0x28b3cf09,// 37 PAY  34 

    0x8f465ea4,// 38 PAY  35 

    0x9c2433b2,// 39 PAY  36 

    0x283cffba,// 40 PAY  37 

    0xe27ad788,// 41 PAY  38 

    0x4843469a,// 42 PAY  39 

    0x0c97e396,// 43 PAY  40 

    0xa31e494d,// 44 PAY  41 

    0xf5a0f765,// 45 PAY  42 

    0x98391833,// 46 PAY  43 

    0xc56882f9,// 47 PAY  44 

    0x3d361e50,// 48 PAY  45 

    0x0d7da177,// 49 PAY  46 

    0x47948f1d,// 50 PAY  47 

    0x65df07c5,// 51 PAY  48 

    0x90f69fbc,// 52 PAY  49 

    0x1004c121,// 53 PAY  50 

    0xc18ef45e,// 54 PAY  51 

    0x503c0cd3,// 55 PAY  52 

    0x26cf12a3,// 56 PAY  53 

    0xf678906e,// 57 PAY  54 

    0x72965685,// 58 PAY  55 

    0x88b15060,// 59 PAY  56 

    0x16bcab79,// 60 PAY  57 

    0x4ef5cc97,// 61 PAY  58 

    0xb91aea3a,// 62 PAY  59 

    0x10b4f4e2,// 63 PAY  60 

    0xdae316d3,// 64 PAY  61 

    0x9aab3349,// 65 PAY  62 

    0x7a03f84c,// 66 PAY  63 

    0xdfb4f759,// 67 PAY  64 

    0x353f968d,// 68 PAY  65 

    0x5c3ecf3f,// 69 PAY  66 

    0xae3a35ed,// 70 PAY  67 

    0xc0710765,// 71 PAY  68 

    0xcf38c31e,// 72 PAY  69 

    0x3ab5dbc2,// 73 PAY  70 

    0x3bf41042,// 74 PAY  71 

    0x352653b4,// 75 PAY  72 

    0x61cddfe4,// 76 PAY  73 

    0x1662a8e2,// 77 PAY  74 

    0xb450ef10,// 78 PAY  75 

    0x22641b22,// 79 PAY  76 

    0x47b89470,// 80 PAY  77 

    0x6fe5264f,// 81 PAY  78 

    0x406cb73f,// 82 PAY  79 

    0x9318f16a,// 83 PAY  80 

    0xcd048bff,// 84 PAY  81 

    0x960593cd,// 85 PAY  82 

    0xf1876408,// 86 PAY  83 

    0x039bfba4,// 87 PAY  84 

    0x60696ffb,// 88 PAY  85 

    0x8c23e2cf,// 89 PAY  86 

    0x1ff5d0e8,// 90 PAY  87 

    0xf5faef3f,// 91 PAY  88 

    0x52a5010d,// 92 PAY  89 

    0xe071b426,// 93 PAY  90 

    0xac9d773f,// 94 PAY  91 

    0x44ab53eb,// 95 PAY  92 

    0x5b6f1003,// 96 PAY  93 

    0x918799af,// 97 PAY  94 

    0xe9d783cf,// 98 PAY  95 

    0x8cd12ff4,// 99 PAY  96 

    0xc20b1b46,// 100 PAY  97 

    0xce4b55b1,// 101 PAY  98 

    0x93bac5b6,// 102 PAY  99 

    0x2717f747,// 103 PAY 100 

    0x49e563d2,// 104 PAY 101 

    0x5e515cb7,// 105 PAY 102 

    0x916a014b,// 106 PAY 103 

    0x36a6de04,// 107 PAY 104 

    0x1fd86445,// 108 PAY 105 

    0xd734905d,// 109 PAY 106 

    0xe24be87f,// 110 PAY 107 

    0x9af954bd,// 111 PAY 108 

    0xab44e5ee,// 112 PAY 109 

    0x39e7cf70,// 113 PAY 110 

    0x98484b01,// 114 PAY 111 

    0x555bfac3,// 115 PAY 112 

    0x53e3f2d0,// 116 PAY 113 

    0x89fa40ae,// 117 PAY 114 

    0xb3b46c80,// 118 PAY 115 

    0x16e8c04b,// 119 PAY 116 

    0x74ec6712,// 120 PAY 117 

    0xbcac9892,// 121 PAY 118 

    0x78e393da,// 122 PAY 119 

    0x129394b9,// 123 PAY 120 

    0x573c138e,// 124 PAY 121 

    0xbc7df61b,// 125 PAY 122 

    0x4bce853c,// 126 PAY 123 

    0x23cf6aff,// 127 PAY 124 

    0xcc29b8cb,// 128 PAY 125 

    0xdab408ab,// 129 PAY 126 

    0x97c315ee,// 130 PAY 127 

    0x197b554b,// 131 PAY 128 

    0x33580911,// 132 PAY 129 

    0x64ef0953,// 133 PAY 130 

    0xd51e1547,// 134 PAY 131 

    0x32e6bab9,// 135 PAY 132 

    0x57a53b02,// 136 PAY 133 

    0xde3bb34a,// 137 PAY 134 

    0x1b6a9fd8,// 138 PAY 135 

    0x00b5ca31,// 139 PAY 136 

    0x5db82060,// 140 PAY 137 

    0x85ab2c0d,// 141 PAY 138 

    0x659379c9,// 142 PAY 139 

    0x2fcce736,// 143 PAY 140 

    0xdcf71303,// 144 PAY 141 

    0x3f5032e5,// 145 PAY 142 

    0xbad5cb70,// 146 PAY 143 

    0x8fb6cc3f,// 147 PAY 144 

    0x97cd8776,// 148 PAY 145 

    0x91676797,// 149 PAY 146 

    0xb0a7b15b,// 150 PAY 147 

    0x5efd490e,// 151 PAY 148 

    0xabe7bdf0,// 152 PAY 149 

    0xe28f9348,// 153 PAY 150 

    0x235287ae,// 154 PAY 151 

    0xd7bd47ab,// 155 PAY 152 

    0x4638ceb0,// 156 PAY 153 

    0x128278b4,// 157 PAY 154 

    0xe3f9048d,// 158 PAY 155 

    0x46789858,// 159 PAY 156 

    0xae62f628,// 160 PAY 157 

    0xe0c4da65,// 161 PAY 158 

    0x3e777ad4,// 162 PAY 159 

    0x19c0f945,// 163 PAY 160 

    0x0080a70f,// 164 PAY 161 

    0x76de0814,// 165 PAY 162 

    0xd02477ce,// 166 PAY 163 

    0x24404dd8,// 167 PAY 164 

    0x0348b361,// 168 PAY 165 

    0x5fcea812,// 169 PAY 166 

    0xed65708c,// 170 PAY 167 

    0xd149cd20,// 171 PAY 168 

    0x55679bcf,// 172 PAY 169 

    0x9443b042,// 173 PAY 170 

    0x395be7b7,// 174 PAY 171 

    0x2cff4347,// 175 PAY 172 

    0xb12f4413,// 176 PAY 173 

    0x1d62391b,// 177 PAY 174 

    0x023c884b,// 178 PAY 175 

    0x452d0a64,// 179 PAY 176 

    0x0e09985f,// 180 PAY 177 

    0x0fbd6c03,// 181 PAY 178 

    0x43dc6a9d,// 182 PAY 179 

    0x170ea0bb,// 183 PAY 180 

    0x88d3566a,// 184 PAY 181 

    0x0bfb1eb3,// 185 PAY 182 

    0x98eb758b,// 186 PAY 183 

    0xe03faa3e,// 187 PAY 184 

    0xcec43488,// 188 PAY 185 

    0xbc9e8de0,// 189 PAY 186 

    0xe85bbfb7,// 190 PAY 187 

    0x1c72b412,// 191 PAY 188 

    0x133040c4,// 192 PAY 189 

    0x7c66f642,// 193 PAY 190 

    0x75928b35,// 194 PAY 191 

    0xb8fa3d42,// 195 PAY 192 

    0x6d15c4e4,// 196 PAY 193 

    0x65136da3,// 197 PAY 194 

    0x342e0380,// 198 PAY 195 

    0xc97c56ec,// 199 PAY 196 

    0x98412435,// 200 PAY 197 

    0x7a77c477,// 201 PAY 198 

    0x22ca98f8,// 202 PAY 199 

    0x731d080e,// 203 PAY 200 

    0xb503fd55,// 204 PAY 201 

    0xf4563b6c,// 205 PAY 202 

    0x5a7659c1,// 206 PAY 203 

    0xa2fca8f9,// 207 PAY 204 

    0x0ffde924,// 208 PAY 205 

    0x390c755e,// 209 PAY 206 

    0xdd8d0946,// 210 PAY 207 

    0xf8fdf425,// 211 PAY 208 

    0x86e1276e,// 212 PAY 209 

    0x9621c6e3,// 213 PAY 210 

    0x7a62dc66,// 214 PAY 211 

    0xa4140d0e,// 215 PAY 212 

    0x1348e101,// 216 PAY 213 

    0xb6fce2d0,// 217 PAY 214 

    0xb0fcfc29,// 218 PAY 215 

    0xef1aa321,// 219 PAY 216 

    0x8286c34c,// 220 PAY 217 

    0x76af8167,// 221 PAY 218 

    0x4056f1f8,// 222 PAY 219 

    0x3950e849,// 223 PAY 220 

    0x0a03ec7f,// 224 PAY 221 

    0x089caba4,// 225 PAY 222 

    0x1b294921,// 226 PAY 223 

    0xe315a485,// 227 PAY 224 

    0x17905273,// 228 PAY 225 

    0x37cc4da8,// 229 PAY 226 

    0x3836f389,// 230 PAY 227 

    0xb7146700,// 231 PAY 228 

    0x44e7afc8,// 232 PAY 229 

    0x5df16b6f,// 233 PAY 230 

    0x1566e156,// 234 PAY 231 

    0x0309f79e,// 235 PAY 232 

    0xf567cefd,// 236 PAY 233 

    0xdcee7c8c,// 237 PAY 234 

    0x3189fef4,// 238 PAY 235 

    0x02faf92b,// 239 PAY 236 

    0x91a6d600,// 240 PAY 237 

    0x10dfed4e,// 241 PAY 238 

    0x230cdee3,// 242 PAY 239 

    0x0f9886d4,// 243 PAY 240 

    0x9486bdca,// 244 PAY 241 

    0x5720afb2,// 245 PAY 242 

    0xd9f4cb0f,// 246 PAY 243 

    0xfe5dd438,// 247 PAY 244 

    0x0ec7cbc5,// 248 PAY 245 

    0x053ee180,// 249 PAY 246 

    0x2f07496b,// 250 PAY 247 

    0xc812148d,// 251 PAY 248 

    0xe4ddbf7e,// 252 PAY 249 

    0x795ba4d2,// 253 PAY 250 

    0x753a2ca5,// 254 PAY 251 

    0xc862b71c,// 255 PAY 252 

    0x8fb0f614,// 256 PAY 253 

    0x558aaf9a,// 257 PAY 254 

    0x73c41ff3,// 258 PAY 255 

    0x6370fe7d,// 259 PAY 256 

    0xe949ef96,// 260 PAY 257 

    0x78705d82,// 261 PAY 258 

    0x93e5daa4,// 262 PAY 259 

    0xd0eda3ed,// 263 PAY 260 

    0xe4ed34d6,// 264 PAY 261 

    0xce94c044,// 265 PAY 262 

    0x5f5567db,// 266 PAY 263 

    0x0668f162,// 267 PAY 264 

    0x4832501c,// 268 PAY 265 

    0x505839c5,// 269 PAY 266 

    0xf79c1109,// 270 PAY 267 

    0xe0175a1e,// 271 PAY 268 

    0xc6ee369d,// 272 PAY 269 

    0x95ad2d49,// 273 PAY 270 

    0xbe405f9c,// 274 PAY 271 

    0x74edb043,// 275 PAY 272 

    0xc7f244d8,// 276 PAY 273 

    0x5e8f04ff,// 277 PAY 274 

    0x5791db54,// 278 PAY 275 

    0xdb4a8468,// 279 PAY 276 

    0x3829c2bf,// 280 PAY 277 

    0x853976a0,// 281 PAY 278 

    0x56e7a466,// 282 PAY 279 

    0x86dc4fdf,// 283 PAY 280 

    0x6530669d,// 284 PAY 281 

    0x73ff6d86,// 285 PAY 282 

    0x11c2e020,// 286 PAY 283 

    0x2df8edb8,// 287 PAY 284 

    0x3a2aab2e,// 288 PAY 285 

    0xd3ab356b,// 289 PAY 286 

    0xfc49e3e7,// 290 PAY 287 

    0x1992fa4b,// 291 PAY 288 

    0x0aee296b,// 292 PAY 289 

    0x43a03ed8,// 293 PAY 290 

    0x2008ad64,// 294 PAY 291 

    0xe42fde82,// 295 PAY 292 

    0x6f6789fe,// 296 PAY 293 

    0xeb5882ec,// 297 PAY 294 

    0x627a03af,// 298 PAY 295 

    0x211c0d99,// 299 PAY 296 

    0x2ce1e3a9,// 300 PAY 297 

    0x03f2cd9c,// 301 PAY 298 

    0xe7205e64,// 302 PAY 299 

    0x25336ac9,// 303 PAY 300 

    0x30dc372c,// 304 PAY 301 

    0x5833e5f7,// 305 PAY 302 

    0xe2c478e6,// 306 PAY 303 

    0x62f3b20a,// 307 PAY 304 

    0xc4bb28b6,// 308 PAY 305 

    0x71d09764,// 309 PAY 306 

    0xfebdab4e,// 310 PAY 307 

    0x3a7ebe23,// 311 PAY 308 

    0x09a49319,// 312 PAY 309 

    0x950d9067,// 313 PAY 310 

    0xa976e332,// 314 PAY 311 

    0xed3f0092,// 315 PAY 312 

    0x5daa3500,// 316 PAY 313 

    0xe2d84d4b,// 317 PAY 314 

    0x4bb47bca,// 318 PAY 315 

    0x1e2ef813,// 319 PAY 316 

    0x17f1228a,// 320 PAY 317 

    0x9dc02f2e,// 321 PAY 318 

    0x290f0c3f,// 322 PAY 319 

    0x04659c81,// 323 PAY 320 

    0x618576ef,// 324 PAY 321 

/// HASH is  12 bytes 

    0x5720afb2,// 325 HSH   1 

    0xd9f4cb0f,// 326 HSH   2 

    0xfe5dd438,// 327 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 145 

/// STA pkt_idx        : 230 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x03994d91 // 328 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt26_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 109 words. 

/// BDA size     is 431 (0x1af) 

/// BDA id       is 0x9ed4 

    0x01af9ed4,// 3 BDA   1 

/// PAY Generic Data size   : 431 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x91cb524d,// 4 PAY   1 

    0xcc58a94e,// 5 PAY   2 

    0x24cbf6cf,// 6 PAY   3 

    0x592e44eb,// 7 PAY   4 

    0xf82ed209,// 8 PAY   5 

    0xea49eb19,// 9 PAY   6 

    0x177841a5,// 10 PAY   7 

    0xa45bedfc,// 11 PAY   8 

    0xb22db768,// 12 PAY   9 

    0x3ef59a2d,// 13 PAY  10 

    0xd8af37d5,// 14 PAY  11 

    0xa52d0ce3,// 15 PAY  12 

    0x08527a93,// 16 PAY  13 

    0x2073b8c8,// 17 PAY  14 

    0x644f9464,// 18 PAY  15 

    0xe0f27c60,// 19 PAY  16 

    0xfff7b85b,// 20 PAY  17 

    0x63352dd4,// 21 PAY  18 

    0x01614a17,// 22 PAY  19 

    0xb3d4bfdf,// 23 PAY  20 

    0xe56fc92e,// 24 PAY  21 

    0x8eda538c,// 25 PAY  22 

    0x7d1425cd,// 26 PAY  23 

    0x9e69e8f6,// 27 PAY  24 

    0xcb7bbc0d,// 28 PAY  25 

    0x8e8c378b,// 29 PAY  26 

    0x8b671aa0,// 30 PAY  27 

    0xcb33880a,// 31 PAY  28 

    0xff4327f4,// 32 PAY  29 

    0x9d3df710,// 33 PAY  30 

    0x9354f1c1,// 34 PAY  31 

    0x382bf611,// 35 PAY  32 

    0x21dc8b2b,// 36 PAY  33 

    0x6a73bc09,// 37 PAY  34 

    0x3eae9d06,// 38 PAY  35 

    0x9316af64,// 39 PAY  36 

    0xc4e18e1a,// 40 PAY  37 

    0xf49aea38,// 41 PAY  38 

    0x0b9868d7,// 42 PAY  39 

    0x05a9a0d7,// 43 PAY  40 

    0xe3e1fb86,// 44 PAY  41 

    0xd8343454,// 45 PAY  42 

    0xa000e436,// 46 PAY  43 

    0x3b3bdf14,// 47 PAY  44 

    0xed05ead4,// 48 PAY  45 

    0x4b027c29,// 49 PAY  46 

    0x38ee210b,// 50 PAY  47 

    0x2e91cd5f,// 51 PAY  48 

    0xf43b74e4,// 52 PAY  49 

    0x696d8afd,// 53 PAY  50 

    0x4f4f6e5f,// 54 PAY  51 

    0x4396cbc1,// 55 PAY  52 

    0xef24f167,// 56 PAY  53 

    0x8991652e,// 57 PAY  54 

    0x65ceb360,// 58 PAY  55 

    0xa4a0770e,// 59 PAY  56 

    0xe50403f0,// 60 PAY  57 

    0xd4558315,// 61 PAY  58 

    0xc4583d3d,// 62 PAY  59 

    0x6ef38285,// 63 PAY  60 

    0xab2722b3,// 64 PAY  61 

    0xb29022ea,// 65 PAY  62 

    0xacebedbd,// 66 PAY  63 

    0x1aa7c2c1,// 67 PAY  64 

    0x8b4b55ee,// 68 PAY  65 

    0x4cfb8727,// 69 PAY  66 

    0x84da03cb,// 70 PAY  67 

    0xe5828565,// 71 PAY  68 

    0x831a7850,// 72 PAY  69 

    0x3b47a2a6,// 73 PAY  70 

    0x54766f96,// 74 PAY  71 

    0x5fd7898c,// 75 PAY  72 

    0x7ef58e24,// 76 PAY  73 

    0x5a2f1ed3,// 77 PAY  74 

    0x7386f35c,// 78 PAY  75 

    0x3dfcb22a,// 79 PAY  76 

    0x4986d726,// 80 PAY  77 

    0x92eafc60,// 81 PAY  78 

    0xdbafe7f4,// 82 PAY  79 

    0xd69c8066,// 83 PAY  80 

    0x67215cd5,// 84 PAY  81 

    0x798d6f58,// 85 PAY  82 

    0xbc472451,// 86 PAY  83 

    0x2c4a8bd6,// 87 PAY  84 

    0xc8d07247,// 88 PAY  85 

    0xe1d4acf8,// 89 PAY  86 

    0x53e41342,// 90 PAY  87 

    0x66918d03,// 91 PAY  88 

    0x9a3ed195,// 92 PAY  89 

    0x8e48faf8,// 93 PAY  90 

    0x69d4762a,// 94 PAY  91 

    0x422184da,// 95 PAY  92 

    0x8f1a3eb3,// 96 PAY  93 

    0x2722c683,// 97 PAY  94 

    0x61f01701,// 98 PAY  95 

    0x2100ba74,// 99 PAY  96 

    0x01f7ebfd,// 100 PAY  97 

    0x3e2f12af,// 101 PAY  98 

    0x7e048828,// 102 PAY  99 

    0x35b7935d,// 103 PAY 100 

    0x08b82bac,// 104 PAY 101 

    0xb49bb4d1,// 105 PAY 102 

    0x6379dfb8,// 106 PAY 103 

    0xfd55d600,// 107 PAY 104 

    0xc9d995cf,// 108 PAY 105 

    0x71d31730,// 109 PAY 106 

    0x7c848e1d,// 110 PAY 107 

    0x3b598b00,// 111 PAY 108 

/// STA is 1 words. 

/// STA num_pkts       : 14 

/// STA pkt_idx        : 249 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc8 

    0x03e4c80e // 112 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt27_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 390 words. 

/// BDA size     is 1554 (0x612) 

/// BDA id       is 0x601b 

    0x0612601b,// 3 BDA   1 

/// PAY Generic Data size   : 1554 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa57d737d,// 4 PAY   1 

    0x01302fe7,// 5 PAY   2 

    0xcc121e3b,// 6 PAY   3 

    0x87091841,// 7 PAY   4 

    0x7242d89f,// 8 PAY   5 

    0x5a288f0c,// 9 PAY   6 

    0xa47fdff5,// 10 PAY   7 

    0x56639dbe,// 11 PAY   8 

    0x1b357158,// 12 PAY   9 

    0x3ce07c3a,// 13 PAY  10 

    0x96e297f4,// 14 PAY  11 

    0xb9820cfd,// 15 PAY  12 

    0x3e01059d,// 16 PAY  13 

    0x53361fdc,// 17 PAY  14 

    0x67cb1a5f,// 18 PAY  15 

    0xb05a5d67,// 19 PAY  16 

    0x09405e9c,// 20 PAY  17 

    0x78192904,// 21 PAY  18 

    0x6b24762d,// 22 PAY  19 

    0x477ce8c9,// 23 PAY  20 

    0xe9dc6b6f,// 24 PAY  21 

    0xfbe18a6f,// 25 PAY  22 

    0xbaad6b80,// 26 PAY  23 

    0x185b715b,// 27 PAY  24 

    0x71807f30,// 28 PAY  25 

    0x89c42d3e,// 29 PAY  26 

    0xe3fb553a,// 30 PAY  27 

    0x7aca9c59,// 31 PAY  28 

    0xbc13885e,// 32 PAY  29 

    0x0dc7c515,// 33 PAY  30 

    0x330f1501,// 34 PAY  31 

    0x5a8f8dfc,// 35 PAY  32 

    0x0bfa3aed,// 36 PAY  33 

    0x2f311227,// 37 PAY  34 

    0x7d596f70,// 38 PAY  35 

    0xc110326c,// 39 PAY  36 

    0x589c933d,// 40 PAY  37 

    0xb41eb0a1,// 41 PAY  38 

    0x15a50847,// 42 PAY  39 

    0x7a6d776f,// 43 PAY  40 

    0xb9712836,// 44 PAY  41 

    0xc403ab5d,// 45 PAY  42 

    0x77310bbe,// 46 PAY  43 

    0x49257e3b,// 47 PAY  44 

    0xf80a5bc5,// 48 PAY  45 

    0xf5a8c967,// 49 PAY  46 

    0x8afcfd89,// 50 PAY  47 

    0xa4728d07,// 51 PAY  48 

    0x383d57f3,// 52 PAY  49 

    0x765946c3,// 53 PAY  50 

    0xbeb51b4f,// 54 PAY  51 

    0xc998ac49,// 55 PAY  52 

    0x172903b9,// 56 PAY  53 

    0x67dd3da8,// 57 PAY  54 

    0xa7542968,// 58 PAY  55 

    0x91aff946,// 59 PAY  56 

    0x88b0ae56,// 60 PAY  57 

    0x96ddaef9,// 61 PAY  58 

    0x9378d248,// 62 PAY  59 

    0xb91992a8,// 63 PAY  60 

    0x10b366b5,// 64 PAY  61 

    0xb4325802,// 65 PAY  62 

    0xe5348fa5,// 66 PAY  63 

    0x710ed7d0,// 67 PAY  64 

    0xb07ee624,// 68 PAY  65 

    0x4e46caf1,// 69 PAY  66 

    0x35452fdb,// 70 PAY  67 

    0xae2bc2dc,// 71 PAY  68 

    0xff4a493a,// 72 PAY  69 

    0xcb4678bb,// 73 PAY  70 

    0xf74e25fd,// 74 PAY  71 

    0x577b6633,// 75 PAY  72 

    0x5d405d3f,// 76 PAY  73 

    0xf47d5bb9,// 77 PAY  74 

    0x3152ef9a,// 78 PAY  75 

    0x768181b4,// 79 PAY  76 

    0x0bff2877,// 80 PAY  77 

    0x03478903,// 81 PAY  78 

    0xc75d15cf,// 82 PAY  79 

    0x24bad10c,// 83 PAY  80 

    0xd5d0ab45,// 84 PAY  81 

    0xb8ab37ff,// 85 PAY  82 

    0x49bd7ed0,// 86 PAY  83 

    0xb0f8e685,// 87 PAY  84 

    0x54099f91,// 88 PAY  85 

    0x65ada0d0,// 89 PAY  86 

    0x6b151460,// 90 PAY  87 

    0x88059661,// 91 PAY  88 

    0x733560d1,// 92 PAY  89 

    0xae67dc6f,// 93 PAY  90 

    0xf07c56a6,// 94 PAY  91 

    0xc08fc5a1,// 95 PAY  92 

    0x1417515e,// 96 PAY  93 

    0xb7feec1b,// 97 PAY  94 

    0xebbf2996,// 98 PAY  95 

    0x99c9ea6a,// 99 PAY  96 

    0x3188399c,// 100 PAY  97 

    0xc38d12dc,// 101 PAY  98 

    0xba485b91,// 102 PAY  99 

    0x0259d4a0,// 103 PAY 100 

    0x77eebae2,// 104 PAY 101 

    0xcb7b6316,// 105 PAY 102 

    0x2beac27e,// 106 PAY 103 

    0xad5d5ef6,// 107 PAY 104 

    0x59dc9588,// 108 PAY 105 

    0xd3206589,// 109 PAY 106 

    0x42a5ee23,// 110 PAY 107 

    0x8b2f5423,// 111 PAY 108 

    0x3d00d0d7,// 112 PAY 109 

    0x63bfcf39,// 113 PAY 110 

    0xb9d40d86,// 114 PAY 111 

    0x0b97dbe3,// 115 PAY 112 

    0x3775585d,// 116 PAY 113 

    0xd465d292,// 117 PAY 114 

    0x5fe82a12,// 118 PAY 115 

    0x057ece70,// 119 PAY 116 

    0x0f8ef99c,// 120 PAY 117 

    0x197ff913,// 121 PAY 118 

    0x48cdc151,// 122 PAY 119 

    0x88d01b9e,// 123 PAY 120 

    0x8a853f9e,// 124 PAY 121 

    0xe278d1ad,// 125 PAY 122 

    0xc2a7b8b4,// 126 PAY 123 

    0xb82d2b82,// 127 PAY 124 

    0x9e4b56e1,// 128 PAY 125 

    0x97841017,// 129 PAY 126 

    0x2081fe67,// 130 PAY 127 

    0xeac125a5,// 131 PAY 128 

    0xd102170a,// 132 PAY 129 

    0x79563095,// 133 PAY 130 

    0x7cd41db1,// 134 PAY 131 

    0x1f62e5e3,// 135 PAY 132 

    0xd272644d,// 136 PAY 133 

    0x3232000d,// 137 PAY 134 

    0xbd30db2b,// 138 PAY 135 

    0x89dbb70e,// 139 PAY 136 

    0x8950aa89,// 140 PAY 137 

    0xd564a8c9,// 141 PAY 138 

    0x3c01cc3b,// 142 PAY 139 

    0x5f271e3a,// 143 PAY 140 

    0x6a11cca9,// 144 PAY 141 

    0x2cd1fddc,// 145 PAY 142 

    0x2326774f,// 146 PAY 143 

    0x940b6a2a,// 147 PAY 144 

    0xbd848ee6,// 148 PAY 145 

    0x53f38aca,// 149 PAY 146 

    0x30e849ef,// 150 PAY 147 

    0x2541a435,// 151 PAY 148 

    0xd3b16b02,// 152 PAY 149 

    0xe66a512d,// 153 PAY 150 

    0x1150280a,// 154 PAY 151 

    0xb24a8bd0,// 155 PAY 152 

    0x552c9cea,// 156 PAY 153 

    0x0c15b879,// 157 PAY 154 

    0x744112bb,// 158 PAY 155 

    0x70dad1b9,// 159 PAY 156 

    0x8169dd62,// 160 PAY 157 

    0x9734f44a,// 161 PAY 158 

    0xc0b885de,// 162 PAY 159 

    0x89f7e2b8,// 163 PAY 160 

    0x14edc88d,// 164 PAY 161 

    0x4786ec3f,// 165 PAY 162 

    0xd2d8bb14,// 166 PAY 163 

    0xda1c82bc,// 167 PAY 164 

    0x3d16deb6,// 168 PAY 165 

    0xd177ae74,// 169 PAY 166 

    0x47db56c5,// 170 PAY 167 

    0x0ee470bc,// 171 PAY 168 

    0x4cf9a364,// 172 PAY 169 

    0xa8e39c34,// 173 PAY 170 

    0xea33485f,// 174 PAY 171 

    0x7ecd35b6,// 175 PAY 172 

    0xa6aaf191,// 176 PAY 173 

    0x05c6fba1,// 177 PAY 174 

    0x57bb6ce0,// 178 PAY 175 

    0x9a33be40,// 179 PAY 176 

    0xdd219c97,// 180 PAY 177 

    0x236308d8,// 181 PAY 178 

    0x09e5854b,// 182 PAY 179 

    0x81f895c4,// 183 PAY 180 

    0x46cf58c2,// 184 PAY 181 

    0x31a8440e,// 185 PAY 182 

    0x467be644,// 186 PAY 183 

    0xe0c4e5f9,// 187 PAY 184 

    0x725de6cd,// 188 PAY 185 

    0x570a6804,// 189 PAY 186 

    0x50c727ed,// 190 PAY 187 

    0xdf55edc3,// 191 PAY 188 

    0x5bc22aa2,// 192 PAY 189 

    0x46a7c7b8,// 193 PAY 190 

    0x9fc750e0,// 194 PAY 191 

    0xafac88d5,// 195 PAY 192 

    0x3bd6e73f,// 196 PAY 193 

    0xa6a171c9,// 197 PAY 194 

    0x69872811,// 198 PAY 195 

    0x8cdd8892,// 199 PAY 196 

    0xf878d851,// 200 PAY 197 

    0x777b0808,// 201 PAY 198 

    0x4719896e,// 202 PAY 199 

    0x5bda652b,// 203 PAY 200 

    0x30fd72cb,// 204 PAY 201 

    0xfc03431c,// 205 PAY 202 

    0xde6eafe3,// 206 PAY 203 

    0x3b914c6c,// 207 PAY 204 

    0xa3ffe7c3,// 208 PAY 205 

    0xfa74bf16,// 209 PAY 206 

    0x9a28d113,// 210 PAY 207 

    0xd8f72409,// 211 PAY 208 

    0x4e701c1b,// 212 PAY 209 

    0x895a330f,// 213 PAY 210 

    0xca135905,// 214 PAY 211 

    0x2e391d17,// 215 PAY 212 

    0x9ed43e10,// 216 PAY 213 

    0x90fe5a8e,// 217 PAY 214 

    0x90916b5b,// 218 PAY 215 

    0x27607a13,// 219 PAY 216 

    0x42b550c8,// 220 PAY 217 

    0x131276f6,// 221 PAY 218 

    0x915c7000,// 222 PAY 219 

    0xde716093,// 223 PAY 220 

    0x956c6486,// 224 PAY 221 

    0x7b5bb622,// 225 PAY 222 

    0x8dc8c867,// 226 PAY 223 

    0x622ca148,// 227 PAY 224 

    0x557a722f,// 228 PAY 225 

    0xf4ad73e4,// 229 PAY 226 

    0x2fbcc474,// 230 PAY 227 

    0x55cdf18a,// 231 PAY 228 

    0x40d3f621,// 232 PAY 229 

    0x83927759,// 233 PAY 230 

    0x84e34236,// 234 PAY 231 

    0xbbb44a59,// 235 PAY 232 

    0x34711730,// 236 PAY 233 

    0x1984e46f,// 237 PAY 234 

    0x875000fe,// 238 PAY 235 

    0x1f794ba6,// 239 PAY 236 

    0x9d9d8b4f,// 240 PAY 237 

    0x8f5b108b,// 241 PAY 238 

    0x2e186ae6,// 242 PAY 239 

    0x1f12a9c8,// 243 PAY 240 

    0x91a3304b,// 244 PAY 241 

    0xf1b89059,// 245 PAY 242 

    0x3b4576f2,// 246 PAY 243 

    0x16c60bb5,// 247 PAY 244 

    0xd7d9db3d,// 248 PAY 245 

    0x56cf73be,// 249 PAY 246 

    0x06056187,// 250 PAY 247 

    0x5d37b2ba,// 251 PAY 248 

    0xaa2093da,// 252 PAY 249 

    0xc61be865,// 253 PAY 250 

    0x1ffc1ffd,// 254 PAY 251 

    0x3c5b3490,// 255 PAY 252 

    0x2dd27c45,// 256 PAY 253 

    0x75a5a69a,// 257 PAY 254 

    0x3548a09f,// 258 PAY 255 

    0x4d6e6f1b,// 259 PAY 256 

    0x1bb5bfac,// 260 PAY 257 

    0xbeafa63d,// 261 PAY 258 

    0x08beddd1,// 262 PAY 259 

    0x329c5a65,// 263 PAY 260 

    0x802473f5,// 264 PAY 261 

    0x68490a6d,// 265 PAY 262 

    0xcfd04eac,// 266 PAY 263 

    0x19b85990,// 267 PAY 264 

    0x4f6d8089,// 268 PAY 265 

    0x80e288de,// 269 PAY 266 

    0xda0b3f9d,// 270 PAY 267 

    0xa08166e5,// 271 PAY 268 

    0x40ce50ac,// 272 PAY 269 

    0xb0c98a92,// 273 PAY 270 

    0xfc86fe57,// 274 PAY 271 

    0x6c5ae172,// 275 PAY 272 

    0xca639282,// 276 PAY 273 

    0xa2bdd619,// 277 PAY 274 

    0x9787d949,// 278 PAY 275 

    0x3dedde60,// 279 PAY 276 

    0x02da1947,// 280 PAY 277 

    0x4cf46865,// 281 PAY 278 

    0x6bc8c807,// 282 PAY 279 

    0xbbf4dedd,// 283 PAY 280 

    0x31046391,// 284 PAY 281 

    0x4b48e8d0,// 285 PAY 282 

    0x6588828a,// 286 PAY 283 

    0xdc565a15,// 287 PAY 284 

    0x92990dc5,// 288 PAY 285 

    0x3cb3f40b,// 289 PAY 286 

    0x41f66e05,// 290 PAY 287 

    0xe7171cc5,// 291 PAY 288 

    0x619a9d57,// 292 PAY 289 

    0x78fbe60d,// 293 PAY 290 

    0xecc5d524,// 294 PAY 291 

    0xd4f15497,// 295 PAY 292 

    0xdf5f980a,// 296 PAY 293 

    0x1c3096b2,// 297 PAY 294 

    0xab531fbc,// 298 PAY 295 

    0xdf44cf54,// 299 PAY 296 

    0x2f388f4d,// 300 PAY 297 

    0x041037a2,// 301 PAY 298 

    0xa3466ba3,// 302 PAY 299 

    0xc2ef23b3,// 303 PAY 300 

    0x7ef63f4f,// 304 PAY 301 

    0xca7635a3,// 305 PAY 302 

    0x1c2a59b6,// 306 PAY 303 

    0x774f573e,// 307 PAY 304 

    0xbe6dd4a5,// 308 PAY 305 

    0x3368d5ec,// 309 PAY 306 

    0xd2830ea1,// 310 PAY 307 

    0xd38c7df4,// 311 PAY 308 

    0xb163e1c4,// 312 PAY 309 

    0x5dcfe70a,// 313 PAY 310 

    0xc6e79183,// 314 PAY 311 

    0x9d075f81,// 315 PAY 312 

    0xd2a1783e,// 316 PAY 313 

    0x7f316c65,// 317 PAY 314 

    0x8c0ec406,// 318 PAY 315 

    0xc6cd1a91,// 319 PAY 316 

    0xb47f6b78,// 320 PAY 317 

    0xb8ebf3d7,// 321 PAY 318 

    0xaa7686f9,// 322 PAY 319 

    0x1cb518e7,// 323 PAY 320 

    0x89455af8,// 324 PAY 321 

    0x8aa746d6,// 325 PAY 322 

    0x725d680e,// 326 PAY 323 

    0x978116d2,// 327 PAY 324 

    0x277f33ce,// 328 PAY 325 

    0x8ab42368,// 329 PAY 326 

    0x2f7b3014,// 330 PAY 327 

    0xde08fc9f,// 331 PAY 328 

    0x244cd3b5,// 332 PAY 329 

    0x354c4f12,// 333 PAY 330 

    0x7a5aff0f,// 334 PAY 331 

    0x2eb3b45e,// 335 PAY 332 

    0xac89c311,// 336 PAY 333 

    0xee2607ec,// 337 PAY 334 

    0x4d17101c,// 338 PAY 335 

    0x40cab3b6,// 339 PAY 336 

    0xb91cdb5a,// 340 PAY 337 

    0x98e7d4e5,// 341 PAY 338 

    0xc62fe91b,// 342 PAY 339 

    0x5591d25b,// 343 PAY 340 

    0xdff0564c,// 344 PAY 341 

    0xe9660d0e,// 345 PAY 342 

    0x82381b3a,// 346 PAY 343 

    0xc0cf0237,// 347 PAY 344 

    0x07ec3a74,// 348 PAY 345 

    0x445feb55,// 349 PAY 346 

    0xe2a0eb1d,// 350 PAY 347 

    0xdbd3ccf2,// 351 PAY 348 

    0x037385ed,// 352 PAY 349 

    0x934ee0dc,// 353 PAY 350 

    0x6991a2ca,// 354 PAY 351 

    0x006cc4e8,// 355 PAY 352 

    0x832d4fa6,// 356 PAY 353 

    0xf4748cea,// 357 PAY 354 

    0x69d6dc2b,// 358 PAY 355 

    0x653b4862,// 359 PAY 356 

    0x4bf70f13,// 360 PAY 357 

    0xc924dbdb,// 361 PAY 358 

    0xdc0c25e6,// 362 PAY 359 

    0x2a55cbcc,// 363 PAY 360 

    0x03a7b78e,// 364 PAY 361 

    0x50e9d0be,// 365 PAY 362 

    0x1a027991,// 366 PAY 363 

    0x1d917b5f,// 367 PAY 364 

    0xcf3f741c,// 368 PAY 365 

    0xc1fc9fc5,// 369 PAY 366 

    0x0eafe0b0,// 370 PAY 367 

    0x3a9c11f8,// 371 PAY 368 

    0x1e4170b3,// 372 PAY 369 

    0xda49db9a,// 373 PAY 370 

    0x28ce0f61,// 374 PAY 371 

    0xc68353d0,// 375 PAY 372 

    0x5e0daba7,// 376 PAY 373 

    0xd31a43ee,// 377 PAY 374 

    0x261becf9,// 378 PAY 375 

    0x649fe8f6,// 379 PAY 376 

    0x140aa172,// 380 PAY 377 

    0xcbb926da,// 381 PAY 378 

    0xd97a1927,// 382 PAY 379 

    0xca305cd4,// 383 PAY 380 

    0x13ece921,// 384 PAY 381 

    0x3ba55c2a,// 385 PAY 382 

    0xf6b27d73,// 386 PAY 383 

    0xdc82a765,// 387 PAY 384 

    0xd1300ed7,// 388 PAY 385 

    0xbcc133e1,// 389 PAY 386 

    0xc1c7a7e4,// 390 PAY 387 

    0x15480f00,// 391 PAY 388 

    0x80ba0000,// 392 PAY 389 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x03f8c1d6 // 393 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt28_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 257 words. 

/// BDA size     is 1021 (0x3fd) 

/// BDA id       is 0x24a3 

    0x03fd24a3,// 3 BDA   1 

/// PAY Generic Data size   : 1021 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xc32ee435,// 4 PAY   1 

    0x8d2acb39,// 5 PAY   2 

    0x5692b5d9,// 6 PAY   3 

    0x8273505a,// 7 PAY   4 

    0x0aa4bf83,// 8 PAY   5 

    0x873264f7,// 9 PAY   6 

    0xeb1eecf2,// 10 PAY   7 

    0xcdb23c37,// 11 PAY   8 

    0xc2992c03,// 12 PAY   9 

    0x52037d28,// 13 PAY  10 

    0x25a07776,// 14 PAY  11 

    0xa028d155,// 15 PAY  12 

    0x1f82399d,// 16 PAY  13 

    0x982f342a,// 17 PAY  14 

    0xba992a9e,// 18 PAY  15 

    0x051cfec0,// 19 PAY  16 

    0xc878113b,// 20 PAY  17 

    0x49727d3b,// 21 PAY  18 

    0xe1785fa4,// 22 PAY  19 

    0x72e93368,// 23 PAY  20 

    0xbb7fa5f0,// 24 PAY  21 

    0x3f21ce41,// 25 PAY  22 

    0xa9a3dcab,// 26 PAY  23 

    0x506eb64a,// 27 PAY  24 

    0xa7aa0dd8,// 28 PAY  25 

    0xeab594bc,// 29 PAY  26 

    0x9160f567,// 30 PAY  27 

    0xfa8face6,// 31 PAY  28 

    0x1eea097b,// 32 PAY  29 

    0x7bbc1573,// 33 PAY  30 

    0xd64f9a75,// 34 PAY  31 

    0x191bb291,// 35 PAY  32 

    0xfd0bfb7b,// 36 PAY  33 

    0x17f556c9,// 37 PAY  34 

    0xc77013bd,// 38 PAY  35 

    0xb6ffa46c,// 39 PAY  36 

    0xcc5889d2,// 40 PAY  37 

    0x32d7ec71,// 41 PAY  38 

    0xdc7b166f,// 42 PAY  39 

    0x979eb0ea,// 43 PAY  40 

    0x6910db74,// 44 PAY  41 

    0xd2f3e2ed,// 45 PAY  42 

    0x53b1464e,// 46 PAY  43 

    0xa9856a6f,// 47 PAY  44 

    0xf746f743,// 48 PAY  45 

    0x3900880c,// 49 PAY  46 

    0x11415935,// 50 PAY  47 

    0xc0f746df,// 51 PAY  48 

    0x027bc149,// 52 PAY  49 

    0xcc82ef78,// 53 PAY  50 

    0x78b71084,// 54 PAY  51 

    0xc283fb8d,// 55 PAY  52 

    0x15b777aa,// 56 PAY  53 

    0xa02d9b64,// 57 PAY  54 

    0xd34fb155,// 58 PAY  55 

    0xa21d4b15,// 59 PAY  56 

    0x9ac9983f,// 60 PAY  57 

    0xa0b308e5,// 61 PAY  58 

    0xe5d1e804,// 62 PAY  59 

    0xc210eb77,// 63 PAY  60 

    0x0aeab351,// 64 PAY  61 

    0x6f8d9a6c,// 65 PAY  62 

    0x4ad30fcf,// 66 PAY  63 

    0x2805d07c,// 67 PAY  64 

    0x0319e8cb,// 68 PAY  65 

    0x1e5a9822,// 69 PAY  66 

    0x68768391,// 70 PAY  67 

    0x858bc038,// 71 PAY  68 

    0x037e8cfe,// 72 PAY  69 

    0x5e442c28,// 73 PAY  70 

    0x8d088833,// 74 PAY  71 

    0x89a71993,// 75 PAY  72 

    0xca98aa00,// 76 PAY  73 

    0xf6d5ab84,// 77 PAY  74 

    0xe5b47e12,// 78 PAY  75 

    0xa84674a7,// 79 PAY  76 

    0xa5f5afe8,// 80 PAY  77 

    0xb34646cb,// 81 PAY  78 

    0xb7662c7e,// 82 PAY  79 

    0xaa3c5e34,// 83 PAY  80 

    0x7f6b4b40,// 84 PAY  81 

    0xdaec2fd3,// 85 PAY  82 

    0x6456a198,// 86 PAY  83 

    0x8198852b,// 87 PAY  84 

    0x443b5b3f,// 88 PAY  85 

    0x38b37d67,// 89 PAY  86 

    0xa0f6b846,// 90 PAY  87 

    0xa376cf1a,// 91 PAY  88 

    0xc5dd71fa,// 92 PAY  89 

    0x9daee8d2,// 93 PAY  90 

    0x8bb3709b,// 94 PAY  91 

    0xf0da03d5,// 95 PAY  92 

    0xef138c40,// 96 PAY  93 

    0x4311b42e,// 97 PAY  94 

    0xcf813c61,// 98 PAY  95 

    0x832313e9,// 99 PAY  96 

    0x63afab6d,// 100 PAY  97 

    0xf4c7f629,// 101 PAY  98 

    0xc4862d82,// 102 PAY  99 

    0xa7b3a0b4,// 103 PAY 100 

    0x048017e0,// 104 PAY 101 

    0x9914fc4c,// 105 PAY 102 

    0x1999d726,// 106 PAY 103 

    0x7b3e9432,// 107 PAY 104 

    0x78ccf98f,// 108 PAY 105 

    0x5281b683,// 109 PAY 106 

    0xe5e018f0,// 110 PAY 107 

    0x96ea5ef9,// 111 PAY 108 

    0xc65edbf0,// 112 PAY 109 

    0x4b17321e,// 113 PAY 110 

    0x456a680f,// 114 PAY 111 

    0xeb82e43e,// 115 PAY 112 

    0xa1886725,// 116 PAY 113 

    0x0d63daf6,// 117 PAY 114 

    0x876bcee6,// 118 PAY 115 

    0x24bc23d3,// 119 PAY 116 

    0xf7b059b4,// 120 PAY 117 

    0x34c241c7,// 121 PAY 118 

    0x3cbdf39f,// 122 PAY 119 

    0x1b46098b,// 123 PAY 120 

    0x764726bf,// 124 PAY 121 

    0xbd129e65,// 125 PAY 122 

    0x4f455fb6,// 126 PAY 123 

    0x4833cf59,// 127 PAY 124 

    0xa242e543,// 128 PAY 125 

    0xd4012e9a,// 129 PAY 126 

    0xe414be0a,// 130 PAY 127 

    0x1b32b674,// 131 PAY 128 

    0x555bdb71,// 132 PAY 129 

    0x26c48856,// 133 PAY 130 

    0x435821da,// 134 PAY 131 

    0xa2ff2e67,// 135 PAY 132 

    0xe4ba417c,// 136 PAY 133 

    0x50eb72c5,// 137 PAY 134 

    0x5fde15c7,// 138 PAY 135 

    0x3d4b1279,// 139 PAY 136 

    0x93cedc79,// 140 PAY 137 

    0x36210f83,// 141 PAY 138 

    0xb236c79a,// 142 PAY 139 

    0x444e4606,// 143 PAY 140 

    0x7b361b78,// 144 PAY 141 

    0x66f16b0b,// 145 PAY 142 

    0x3fa0e109,// 146 PAY 143 

    0xf06a4c75,// 147 PAY 144 

    0x23d652b1,// 148 PAY 145 

    0x8cad38c9,// 149 PAY 146 

    0xaaa5ac2a,// 150 PAY 147 

    0x693c049f,// 151 PAY 148 

    0x6bb0998f,// 152 PAY 149 

    0x2416a403,// 153 PAY 150 

    0xcc45d594,// 154 PAY 151 

    0x8800da2c,// 155 PAY 152 

    0x5d292a4e,// 156 PAY 153 

    0xb4bbb3e4,// 157 PAY 154 

    0x7bfbd9d3,// 158 PAY 155 

    0x491e13f5,// 159 PAY 156 

    0xf000faa6,// 160 PAY 157 

    0x748db314,// 161 PAY 158 

    0xb78aa874,// 162 PAY 159 

    0x5d54588b,// 163 PAY 160 

    0x9680279f,// 164 PAY 161 

    0x477be0eb,// 165 PAY 162 

    0xb850c16e,// 166 PAY 163 

    0x9c7c9af5,// 167 PAY 164 

    0xbda129e4,// 168 PAY 165 

    0x50cd9771,// 169 PAY 166 

    0x0016764d,// 170 PAY 167 

    0xc2472122,// 171 PAY 168 

    0x89621328,// 172 PAY 169 

    0x8e24aed4,// 173 PAY 170 

    0xca7c324c,// 174 PAY 171 

    0x7f189795,// 175 PAY 172 

    0x5f58b822,// 176 PAY 173 

    0xca54a471,// 177 PAY 174 

    0xf30c16a7,// 178 PAY 175 

    0x2e2a264a,// 179 PAY 176 

    0x7043b09f,// 180 PAY 177 

    0x618f8560,// 181 PAY 178 

    0xc677f045,// 182 PAY 179 

    0x5c04a204,// 183 PAY 180 

    0x2540381f,// 184 PAY 181 

    0x90b552af,// 185 PAY 182 

    0xd3e56bc8,// 186 PAY 183 

    0x68275515,// 187 PAY 184 

    0x7ba22af0,// 188 PAY 185 

    0x32fa4c9e,// 189 PAY 186 

    0x77bcfd12,// 190 PAY 187 

    0xe3ac6f6b,// 191 PAY 188 

    0x2ca92b8c,// 192 PAY 189 

    0x4aba183b,// 193 PAY 190 

    0x68f99bc5,// 194 PAY 191 

    0x89b66a18,// 195 PAY 192 

    0xf4c6d27f,// 196 PAY 193 

    0x7a6554fb,// 197 PAY 194 

    0xb50d9d83,// 198 PAY 195 

    0xebec3013,// 199 PAY 196 

    0xaf9cb832,// 200 PAY 197 

    0x4d23dfed,// 201 PAY 198 

    0x195272a4,// 202 PAY 199 

    0x3a281290,// 203 PAY 200 

    0x6d5d2b95,// 204 PAY 201 

    0x7967be10,// 205 PAY 202 

    0x3c454e76,// 206 PAY 203 

    0x92527cca,// 207 PAY 204 

    0xec992af8,// 208 PAY 205 

    0x43075d32,// 209 PAY 206 

    0x7cc59548,// 210 PAY 207 

    0x18735ead,// 211 PAY 208 

    0x798bab01,// 212 PAY 209 

    0x3bff890b,// 213 PAY 210 

    0x2a766b17,// 214 PAY 211 

    0x7126a85c,// 215 PAY 212 

    0x58c27b06,// 216 PAY 213 

    0xe81c5fca,// 217 PAY 214 

    0x9274d929,// 218 PAY 215 

    0xc801213a,// 219 PAY 216 

    0x2dcf0c58,// 220 PAY 217 

    0xd0dff559,// 221 PAY 218 

    0x697f297f,// 222 PAY 219 

    0x0a5232ee,// 223 PAY 220 

    0x6bc4aa66,// 224 PAY 221 

    0x478667c3,// 225 PAY 222 

    0x4955ebe4,// 226 PAY 223 

    0xf06231d9,// 227 PAY 224 

    0x4c5c8589,// 228 PAY 225 

    0x8a5761e2,// 229 PAY 226 

    0x8fee4efb,// 230 PAY 227 

    0x0b506283,// 231 PAY 228 

    0x8b01ff65,// 232 PAY 229 

    0x819d838f,// 233 PAY 230 

    0x73abe9d9,// 234 PAY 231 

    0xda52b2a7,// 235 PAY 232 

    0x17ca0ae7,// 236 PAY 233 

    0x4865d07b,// 237 PAY 234 

    0x24b53aff,// 238 PAY 235 

    0x003e76ab,// 239 PAY 236 

    0x2b89a2df,// 240 PAY 237 

    0x3618b470,// 241 PAY 238 

    0x7d1741af,// 242 PAY 239 

    0x5508f324,// 243 PAY 240 

    0x4d93b217,// 244 PAY 241 

    0x03126563,// 245 PAY 242 

    0x63d53c5c,// 246 PAY 243 

    0xd69faa27,// 247 PAY 244 

    0x535ff50a,// 248 PAY 245 

    0x1d0f5b3f,// 249 PAY 246 

    0x024943f7,// 250 PAY 247 

    0x75c2aa81,// 251 PAY 248 

    0x2ec15bef,// 252 PAY 249 

    0x82574ce5,// 253 PAY 250 

    0xc8655fce,// 254 PAY 251 

    0x7be5553d,// 255 PAY 252 

    0xcbf1d357,// 256 PAY 253 

    0xc400021b,// 257 PAY 254 

    0x86f8a3f6,// 258 PAY 255 

    0x59000000,// 259 PAY 256 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe5 

    0x01e8e580 // 260 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt29_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 367 words. 

/// BDA size     is 1463 (0x5b7) 

/// BDA id       is 0x31c5 

    0x05b731c5,// 3 BDA   1 

/// PAY Generic Data size   : 1463 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x4f8b40e1,// 4 PAY   1 

    0x1d65da9e,// 5 PAY   2 

    0x5306fa45,// 6 PAY   3 

    0xd20c01d9,// 7 PAY   4 

    0x37b70c49,// 8 PAY   5 

    0xa6baf927,// 9 PAY   6 

    0xb74b0b15,// 10 PAY   7 

    0x7ed7f06a,// 11 PAY   8 

    0x104b4a9b,// 12 PAY   9 

    0xd02545ae,// 13 PAY  10 

    0xb3714988,// 14 PAY  11 

    0x39f0e5c9,// 15 PAY  12 

    0xdfe92a59,// 16 PAY  13 

    0x654af32a,// 17 PAY  14 

    0x9674f2f2,// 18 PAY  15 

    0x0393647c,// 19 PAY  16 

    0xb4523744,// 20 PAY  17 

    0x083183d5,// 21 PAY  18 

    0x34e423a4,// 22 PAY  19 

    0x933a3f1f,// 23 PAY  20 

    0xcc402871,// 24 PAY  21 

    0xb7b9c88a,// 25 PAY  22 

    0x3f614c14,// 26 PAY  23 

    0x8408fcb9,// 27 PAY  24 

    0xdb600bb3,// 28 PAY  25 

    0xd3c8bd16,// 29 PAY  26 

    0xe46be464,// 30 PAY  27 

    0xd6ea03d2,// 31 PAY  28 

    0x76b0ef48,// 32 PAY  29 

    0x6ddcc8e2,// 33 PAY  30 

    0x1d2461fe,// 34 PAY  31 

    0x30a97d17,// 35 PAY  32 

    0xfac16eff,// 36 PAY  33 

    0xa90527ff,// 37 PAY  34 

    0x33b7f059,// 38 PAY  35 

    0x11ca82ad,// 39 PAY  36 

    0x1eecf4b3,// 40 PAY  37 

    0x0f86f155,// 41 PAY  38 

    0x7f72a861,// 42 PAY  39 

    0x9a27e3f5,// 43 PAY  40 

    0xdae6e14e,// 44 PAY  41 

    0x4d8508da,// 45 PAY  42 

    0xe4ed2a7a,// 46 PAY  43 

    0x1cf55ec8,// 47 PAY  44 

    0x7fb2b258,// 48 PAY  45 

    0x11ae51e9,// 49 PAY  46 

    0xd0007996,// 50 PAY  47 

    0x02a7a5b9,// 51 PAY  48 

    0xc88e4207,// 52 PAY  49 

    0x3908bbc7,// 53 PAY  50 

    0xc4ca33d6,// 54 PAY  51 

    0x415d6933,// 55 PAY  52 

    0x47f9f655,// 56 PAY  53 

    0xb07bf970,// 57 PAY  54 

    0x4b1cc118,// 58 PAY  55 

    0x8f9d7488,// 59 PAY  56 

    0xc6885a50,// 60 PAY  57 

    0x71a0fc0d,// 61 PAY  58 

    0x4f155524,// 62 PAY  59 

    0x456ceff5,// 63 PAY  60 

    0x939cf0c3,// 64 PAY  61 

    0xaf30c985,// 65 PAY  62 

    0xc95eb642,// 66 PAY  63 

    0xde251884,// 67 PAY  64 

    0x92845f61,// 68 PAY  65 

    0xc35ca720,// 69 PAY  66 

    0x7c9ca7e8,// 70 PAY  67 

    0x1e34917a,// 71 PAY  68 

    0x4e77a3b7,// 72 PAY  69 

    0x62ee2278,// 73 PAY  70 

    0x096b8a69,// 74 PAY  71 

    0xcaa4e8f9,// 75 PAY  72 

    0xb2418186,// 76 PAY  73 

    0xd579b2f1,// 77 PAY  74 

    0x2c31851f,// 78 PAY  75 

    0x12d00c66,// 79 PAY  76 

    0x23319a97,// 80 PAY  77 

    0x7e432b03,// 81 PAY  78 

    0xbe604377,// 82 PAY  79 

    0x7ab510d7,// 83 PAY  80 

    0x3f74d11c,// 84 PAY  81 

    0x0097964c,// 85 PAY  82 

    0xd59da03f,// 86 PAY  83 

    0xe8e4bfa6,// 87 PAY  84 

    0x27b92579,// 88 PAY  85 

    0x018b1ea5,// 89 PAY  86 

    0x06916f8f,// 90 PAY  87 

    0xda537b42,// 91 PAY  88 

    0x28829413,// 92 PAY  89 

    0x33061b83,// 93 PAY  90 

    0xb0e6282a,// 94 PAY  91 

    0x41d0fc28,// 95 PAY  92 

    0x55ea3aff,// 96 PAY  93 

    0x469b0701,// 97 PAY  94 

    0x243b4370,// 98 PAY  95 

    0xbe811ff6,// 99 PAY  96 

    0x4d1d8a95,// 100 PAY  97 

    0x6a662ad9,// 101 PAY  98 

    0x93dd55eb,// 102 PAY  99 

    0x560107d5,// 103 PAY 100 

    0x20995800,// 104 PAY 101 

    0xe4b601fe,// 105 PAY 102 

    0x98175199,// 106 PAY 103 

    0xa4f209a4,// 107 PAY 104 

    0x614e847c,// 108 PAY 105 

    0x2294b9e2,// 109 PAY 106 

    0xb250dfd1,// 110 PAY 107 

    0x8fe2cd49,// 111 PAY 108 

    0xc63c4a90,// 112 PAY 109 

    0x9c5a89d7,// 113 PAY 110 

    0x994b7622,// 114 PAY 111 

    0xa63bac38,// 115 PAY 112 

    0xd9de9334,// 116 PAY 113 

    0xa904c0c3,// 117 PAY 114 

    0x2003ba24,// 118 PAY 115 

    0x72e452b2,// 119 PAY 116 

    0xc7ed71cd,// 120 PAY 117 

    0x3738017a,// 121 PAY 118 

    0x488c39eb,// 122 PAY 119 

    0x37c75334,// 123 PAY 120 

    0xab2f28d2,// 124 PAY 121 

    0x48fb48fb,// 125 PAY 122 

    0xe5e55632,// 126 PAY 123 

    0x82d23804,// 127 PAY 124 

    0xf6d9f558,// 128 PAY 125 

    0x4d02e0c8,// 129 PAY 126 

    0x1d469fd1,// 130 PAY 127 

    0xc472208c,// 131 PAY 128 

    0x132278ca,// 132 PAY 129 

    0xb05166ac,// 133 PAY 130 

    0x2ae0ecc1,// 134 PAY 131 

    0x7bf7ae33,// 135 PAY 132 

    0xa2cf8326,// 136 PAY 133 

    0xdcd7e261,// 137 PAY 134 

    0x242c2df0,// 138 PAY 135 

    0xa3c66e46,// 139 PAY 136 

    0x577e2f0c,// 140 PAY 137 

    0x2fc9c54e,// 141 PAY 138 

    0xf2f4925a,// 142 PAY 139 

    0x3dc8a830,// 143 PAY 140 

    0x16f8c985,// 144 PAY 141 

    0x12fb4d90,// 145 PAY 142 

    0x2adea9fd,// 146 PAY 143 

    0xe5c171d4,// 147 PAY 144 

    0x4ca933e6,// 148 PAY 145 

    0xbb8a729d,// 149 PAY 146 

    0x13346c49,// 150 PAY 147 

    0x16733b6a,// 151 PAY 148 

    0xfd9439c4,// 152 PAY 149 

    0x778cb88a,// 153 PAY 150 

    0x14623871,// 154 PAY 151 

    0x14c0daba,// 155 PAY 152 

    0xb72ef37a,// 156 PAY 153 

    0x24ae7de6,// 157 PAY 154 

    0x00382e34,// 158 PAY 155 

    0x6c475dbe,// 159 PAY 156 

    0xad267448,// 160 PAY 157 

    0x864f6e64,// 161 PAY 158 

    0x4938f51d,// 162 PAY 159 

    0x8951f9d9,// 163 PAY 160 

    0x08c36268,// 164 PAY 161 

    0x4d96ec72,// 165 PAY 162 

    0xa7fd8e54,// 166 PAY 163 

    0x38e0c1c7,// 167 PAY 164 

    0xa00af3a7,// 168 PAY 165 

    0x2254febf,// 169 PAY 166 

    0x72267066,// 170 PAY 167 

    0x53d29d78,// 171 PAY 168 

    0x3b228e73,// 172 PAY 169 

    0xdcccc8c5,// 173 PAY 170 

    0x9c3b3cd3,// 174 PAY 171 

    0xd0214db1,// 175 PAY 172 

    0x11994723,// 176 PAY 173 

    0x5bd3b5d6,// 177 PAY 174 

    0x3ec8b269,// 178 PAY 175 

    0xcbe16cda,// 179 PAY 176 

    0x4aa07623,// 180 PAY 177 

    0xcd4682fc,// 181 PAY 178 

    0x14721f55,// 182 PAY 179 

    0x61bbfb88,// 183 PAY 180 

    0x298c97e7,// 184 PAY 181 

    0xc2bb8a0b,// 185 PAY 182 

    0xb0986a58,// 186 PAY 183 

    0x422145f2,// 187 PAY 184 

    0x4433c358,// 188 PAY 185 

    0x1bcce3d5,// 189 PAY 186 

    0xefa74849,// 190 PAY 187 

    0xb323ed99,// 191 PAY 188 

    0x8d4ed436,// 192 PAY 189 

    0xfab34cd7,// 193 PAY 190 

    0x4e9e2bbe,// 194 PAY 191 

    0xbbe2c00e,// 195 PAY 192 

    0xc59874aa,// 196 PAY 193 

    0xb40aeccb,// 197 PAY 194 

    0x06601553,// 198 PAY 195 

    0x40b008aa,// 199 PAY 196 

    0x840ff390,// 200 PAY 197 

    0x05647ea4,// 201 PAY 198 

    0x8e2ab29e,// 202 PAY 199 

    0x6d795f62,// 203 PAY 200 

    0x4b5e26df,// 204 PAY 201 

    0x82b516fe,// 205 PAY 202 

    0xd52b87d3,// 206 PAY 203 

    0xa8f92792,// 207 PAY 204 

    0x4ac43252,// 208 PAY 205 

    0xcac7604f,// 209 PAY 206 

    0x82a6dbdb,// 210 PAY 207 

    0x9f370de8,// 211 PAY 208 

    0xbf071cd0,// 212 PAY 209 

    0x33f76e2b,// 213 PAY 210 

    0x7f74421f,// 214 PAY 211 

    0x0e30aa13,// 215 PAY 212 

    0x542bb53f,// 216 PAY 213 

    0x1f874acc,// 217 PAY 214 

    0x98089caf,// 218 PAY 215 

    0x8ae6f424,// 219 PAY 216 

    0x2b3ad38f,// 220 PAY 217 

    0x8fd07439,// 221 PAY 218 

    0x91a4b6b5,// 222 PAY 219 

    0x46490b9b,// 223 PAY 220 

    0x6710e2d1,// 224 PAY 221 

    0x2b1203a7,// 225 PAY 222 

    0xeecf633a,// 226 PAY 223 

    0xfac1311a,// 227 PAY 224 

    0x03e452a4,// 228 PAY 225 

    0x6311561b,// 229 PAY 226 

    0x3f3aa77e,// 230 PAY 227 

    0xc09be41d,// 231 PAY 228 

    0x4e77992a,// 232 PAY 229 

    0x8ecde911,// 233 PAY 230 

    0x60be04b1,// 234 PAY 231 

    0xe2bd31a0,// 235 PAY 232 

    0x1c87e278,// 236 PAY 233 

    0xcfa6e3b9,// 237 PAY 234 

    0x7bc8bc72,// 238 PAY 235 

    0x0e038cc9,// 239 PAY 236 

    0x9294ca67,// 240 PAY 237 

    0x25419164,// 241 PAY 238 

    0x89babc96,// 242 PAY 239 

    0x20e662bf,// 243 PAY 240 

    0xfd79d9ab,// 244 PAY 241 

    0x8628c252,// 245 PAY 242 

    0x20916bf9,// 246 PAY 243 

    0xfe274369,// 247 PAY 244 

    0x2ad1dd1f,// 248 PAY 245 

    0x6178d11f,// 249 PAY 246 

    0x0b6d479f,// 250 PAY 247 

    0x312e92f3,// 251 PAY 248 

    0x90124922,// 252 PAY 249 

    0xbaf82b70,// 253 PAY 250 

    0xbfb14f64,// 254 PAY 251 

    0xc3eaf093,// 255 PAY 252 

    0x2e399875,// 256 PAY 253 

    0xd876feab,// 257 PAY 254 

    0xce4c79bd,// 258 PAY 255 

    0x6f503e4d,// 259 PAY 256 

    0x564b7b24,// 260 PAY 257 

    0x0baa53ce,// 261 PAY 258 

    0xadad0818,// 262 PAY 259 

    0x3b01feeb,// 263 PAY 260 

    0x1d0192b5,// 264 PAY 261 

    0xa10c6f49,// 265 PAY 262 

    0x3a67d05f,// 266 PAY 263 

    0x65b62b43,// 267 PAY 264 

    0xdebf5738,// 268 PAY 265 

    0x9e5285c5,// 269 PAY 266 

    0x73eb350d,// 270 PAY 267 

    0x33d5ae71,// 271 PAY 268 

    0x62d489eb,// 272 PAY 269 

    0xe873bb5b,// 273 PAY 270 

    0x8d84ba27,// 274 PAY 271 

    0xe72beab2,// 275 PAY 272 

    0xfd3fe1a6,// 276 PAY 273 

    0x63ffab63,// 277 PAY 274 

    0xc9fbca15,// 278 PAY 275 

    0x4d248eec,// 279 PAY 276 

    0xa9319df3,// 280 PAY 277 

    0x26b6b359,// 281 PAY 278 

    0xdefe6e9f,// 282 PAY 279 

    0x0fc344c4,// 283 PAY 280 

    0xefcb1f17,// 284 PAY 281 

    0x37d976ae,// 285 PAY 282 

    0xbf1bd335,// 286 PAY 283 

    0x24fb2e1c,// 287 PAY 284 

    0xc420993c,// 288 PAY 285 

    0x73421364,// 289 PAY 286 

    0xe5c4cbe2,// 290 PAY 287 

    0x5d448a3f,// 291 PAY 288 

    0xea48a347,// 292 PAY 289 

    0xee5f7557,// 293 PAY 290 

    0x859dc015,// 294 PAY 291 

    0x3c3701e0,// 295 PAY 292 

    0xa75ee144,// 296 PAY 293 

    0xfec1666f,// 297 PAY 294 

    0x8c97e0cb,// 298 PAY 295 

    0x41f179f3,// 299 PAY 296 

    0x31a3f847,// 300 PAY 297 

    0x81ac6f55,// 301 PAY 298 

    0xf66936b5,// 302 PAY 299 

    0xc9f11a37,// 303 PAY 300 

    0xe476ff76,// 304 PAY 301 

    0x5de2fd1d,// 305 PAY 302 

    0x9200cfd4,// 306 PAY 303 

    0x470cdc1a,// 307 PAY 304 

    0x96cd4e84,// 308 PAY 305 

    0x8b0dba15,// 309 PAY 306 

    0x9dab3a58,// 310 PAY 307 

    0x10fd2c0d,// 311 PAY 308 

    0xb1af086e,// 312 PAY 309 

    0xb5ac9d08,// 313 PAY 310 

    0x338e5229,// 314 PAY 311 

    0xc8a95715,// 315 PAY 312 

    0xc70bb72e,// 316 PAY 313 

    0x6137a0e5,// 317 PAY 314 

    0xf1d76e3f,// 318 PAY 315 

    0x3ff384d1,// 319 PAY 316 

    0xc4282a86,// 320 PAY 317 

    0xfa97a66e,// 321 PAY 318 

    0xc9e4d52f,// 322 PAY 319 

    0xcde2c7d6,// 323 PAY 320 

    0x3825aee1,// 324 PAY 321 

    0x44d8653c,// 325 PAY 322 

    0xcaaffb51,// 326 PAY 323 

    0x6d22e808,// 327 PAY 324 

    0x9937a653,// 328 PAY 325 

    0x8f259508,// 329 PAY 326 

    0xe4fb6351,// 330 PAY 327 

    0xbf9e05da,// 331 PAY 328 

    0x0f56a1ea,// 332 PAY 329 

    0x519ecc08,// 333 PAY 330 

    0x7906adea,// 334 PAY 331 

    0x3d12bc70,// 335 PAY 332 

    0xca58886d,// 336 PAY 333 

    0x36bd466c,// 337 PAY 334 

    0xea6842c8,// 338 PAY 335 

    0x23abb37c,// 339 PAY 336 

    0x83710f02,// 340 PAY 337 

    0xde8f91b8,// 341 PAY 338 

    0xae1f2391,// 342 PAY 339 

    0x8cb028b8,// 343 PAY 340 

    0xcbcfaca3,// 344 PAY 341 

    0x5902577d,// 345 PAY 342 

    0x1c200432,// 346 PAY 343 

    0x5a35d67c,// 347 PAY 344 

    0x2de067fe,// 348 PAY 345 

    0x963efeb6,// 349 PAY 346 

    0x5a491443,// 350 PAY 347 

    0x4ee22746,// 351 PAY 348 

    0xf12563be,// 352 PAY 349 

    0x5f21d50e,// 353 PAY 350 

    0x1a83697a,// 354 PAY 351 

    0x71772c43,// 355 PAY 352 

    0x222dac4e,// 356 PAY 353 

    0x29245153,// 357 PAY 354 

    0xdb27cec1,// 358 PAY 355 

    0x9e62644b,// 359 PAY 356 

    0x5b410829,// 360 PAY 357 

    0x4470650e,// 361 PAY 358 

    0x031481f0,// 362 PAY 359 

    0x1952c600,// 363 PAY 360 

    0xa59181ba,// 364 PAY 361 

    0x7d280840,// 365 PAY 362 

    0x7396fef7,// 366 PAY 363 

    0xfaca1bc6,// 367 PAY 364 

    0x80ff6115,// 368 PAY 365 

    0xfb901800,// 369 PAY 366 

/// HASH is  8 bytes 

    0x9e62644b,// 370 HSH   1 

    0x5b410829,// 371 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 28 

/// STA pkt_idx        : 43 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x00ac6e1c // 372 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt30_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 137 words. 

/// BDA size     is 542 (0x21e) 

/// BDA id       is 0x15c 

    0x021e015c,// 3 BDA   1 

/// PAY Generic Data size   : 542 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xd2ceba4a,// 4 PAY   1 

    0xdd45d0a8,// 5 PAY   2 

    0x3ab5efa3,// 6 PAY   3 

    0x00b79393,// 7 PAY   4 

    0x2978ea9a,// 8 PAY   5 

    0xdb5d39e2,// 9 PAY   6 

    0x602e43bc,// 10 PAY   7 

    0x36e14a0e,// 11 PAY   8 

    0x3af18044,// 12 PAY   9 

    0x028efb78,// 13 PAY  10 

    0x1603eb35,// 14 PAY  11 

    0x0ceb9214,// 15 PAY  12 

    0x50705bad,// 16 PAY  13 

    0xd38244a4,// 17 PAY  14 

    0x12a1e0ef,// 18 PAY  15 

    0x84fa8a69,// 19 PAY  16 

    0xbd5af0a6,// 20 PAY  17 

    0x64535df4,// 21 PAY  18 

    0x8cae1410,// 22 PAY  19 

    0x8a8d5a7c,// 23 PAY  20 

    0x776df535,// 24 PAY  21 

    0xa610127b,// 25 PAY  22 

    0x92c9fd2c,// 26 PAY  23 

    0xcd1283b4,// 27 PAY  24 

    0x0e5c6c8b,// 28 PAY  25 

    0x314a1492,// 29 PAY  26 

    0x2c00f38a,// 30 PAY  27 

    0x16a96fbf,// 31 PAY  28 

    0xed418395,// 32 PAY  29 

    0x1211d079,// 33 PAY  30 

    0x5bb441db,// 34 PAY  31 

    0xbd3cd183,// 35 PAY  32 

    0x8c4be505,// 36 PAY  33 

    0x5da33c7f,// 37 PAY  34 

    0x220c7b9f,// 38 PAY  35 

    0xeab0587a,// 39 PAY  36 

    0x448178a3,// 40 PAY  37 

    0xddb1f7cc,// 41 PAY  38 

    0x953e51e5,// 42 PAY  39 

    0xb8813fe8,// 43 PAY  40 

    0x3069ad6c,// 44 PAY  41 

    0x32ac74fe,// 45 PAY  42 

    0x659c8d07,// 46 PAY  43 

    0x1fb86ff7,// 47 PAY  44 

    0x5bed8df3,// 48 PAY  45 

    0x9c869d15,// 49 PAY  46 

    0x3a2bcea4,// 50 PAY  47 

    0xb9bf262c,// 51 PAY  48 

    0x72d3626f,// 52 PAY  49 

    0x38acc4cc,// 53 PAY  50 

    0xe02ceb22,// 54 PAY  51 

    0xb4c356ca,// 55 PAY  52 

    0xe1bfa310,// 56 PAY  53 

    0x312665ac,// 57 PAY  54 

    0xcfd1cf40,// 58 PAY  55 

    0x8685231a,// 59 PAY  56 

    0x2dae3dd3,// 60 PAY  57 

    0x9dcda84f,// 61 PAY  58 

    0xbdc83a90,// 62 PAY  59 

    0x74a77dc2,// 63 PAY  60 

    0x69c51ae4,// 64 PAY  61 

    0xcbed9743,// 65 PAY  62 

    0x2ef1ad4d,// 66 PAY  63 

    0xbecd01c9,// 67 PAY  64 

    0x5b15d28c,// 68 PAY  65 

    0x437606aa,// 69 PAY  66 

    0xad52e28a,// 70 PAY  67 

    0x68fecd51,// 71 PAY  68 

    0xbebe1aee,// 72 PAY  69 

    0x75c0cef3,// 73 PAY  70 

    0xe264427c,// 74 PAY  71 

    0x46eed095,// 75 PAY  72 

    0x66db09f6,// 76 PAY  73 

    0x56385851,// 77 PAY  74 

    0xd22f9f61,// 78 PAY  75 

    0x8a4ac323,// 79 PAY  76 

    0x7665aded,// 80 PAY  77 

    0x9fbfd3aa,// 81 PAY  78 

    0x84a84f36,// 82 PAY  79 

    0x48fc195c,// 83 PAY  80 

    0x90411d14,// 84 PAY  81 

    0x06526bc2,// 85 PAY  82 

    0x6462a488,// 86 PAY  83 

    0x1b504883,// 87 PAY  84 

    0x613b2680,// 88 PAY  85 

    0x0d571876,// 89 PAY  86 

    0x3dce694e,// 90 PAY  87 

    0xd185b54e,// 91 PAY  88 

    0x61ecfcf2,// 92 PAY  89 

    0xa0b9ef5a,// 93 PAY  90 

    0xcbaee0b7,// 94 PAY  91 

    0x6345bf0b,// 95 PAY  92 

    0xc7d42e9b,// 96 PAY  93 

    0xfe687fba,// 97 PAY  94 

    0xc78eeae2,// 98 PAY  95 

    0x78a49853,// 99 PAY  96 

    0x7e152387,// 100 PAY  97 

    0x0e4947ed,// 101 PAY  98 

    0x26e2da49,// 102 PAY  99 

    0x730cfe45,// 103 PAY 100 

    0xc0ff756f,// 104 PAY 101 

    0xda5188b8,// 105 PAY 102 

    0xce1d16cf,// 106 PAY 103 

    0xbd4c5857,// 107 PAY 104 

    0x9e237104,// 108 PAY 105 

    0x78113c41,// 109 PAY 106 

    0xc08da435,// 110 PAY 107 

    0xca4c81d4,// 111 PAY 108 

    0x6969d434,// 112 PAY 109 

    0x74b4a269,// 113 PAY 110 

    0x82c76e4d,// 114 PAY 111 

    0xa85e776f,// 115 PAY 112 

    0xef04a495,// 116 PAY 113 

    0x8a77163f,// 117 PAY 114 

    0x2e0614ef,// 118 PAY 115 

    0x11698dcb,// 119 PAY 116 

    0x96babb57,// 120 PAY 117 

    0x5b007cdd,// 121 PAY 118 

    0x82e548d3,// 122 PAY 119 

    0xb6a2d438,// 123 PAY 120 

    0xe04b5cc7,// 124 PAY 121 

    0x9687cea4,// 125 PAY 122 

    0x184eff9e,// 126 PAY 123 

    0xb5e58791,// 127 PAY 124 

    0xac5f05b8,// 128 PAY 125 

    0x1d173a6c,// 129 PAY 126 

    0x8254f645,// 130 PAY 127 

    0x65c293c2,// 131 PAY 128 

    0x5d7c314b,// 132 PAY 129 

    0x3f437bea,// 133 PAY 130 

    0x6b0bbcc4,// 134 PAY 131 

    0x694155a7,// 135 PAY 132 

    0x5b8dc653,// 136 PAY 133 

    0x7966a7c0,// 137 PAY 134 

    0x863f1b43,// 138 PAY 135 

    0xb86f0000,// 139 PAY 136 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 105 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x53 

    0x01a55306 // 140 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt31_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 56 words. 

/// BDA size     is 217 (0xd9) 

/// BDA id       is 0x2b67 

    0x00d92b67,// 3 BDA   1 

/// PAY Generic Data size   : 217 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x7c6e9ecc,// 4 PAY   1 

    0x0975d10e,// 5 PAY   2 

    0x05f00124,// 6 PAY   3 

    0x3dce1cc1,// 7 PAY   4 

    0xfc80c47f,// 8 PAY   5 

    0x173b72f4,// 9 PAY   6 

    0x6b1bfe98,// 10 PAY   7 

    0xa33d18c7,// 11 PAY   8 

    0x57d2d0a4,// 12 PAY   9 

    0x6c03b5d7,// 13 PAY  10 

    0x4ab563a5,// 14 PAY  11 

    0x2499abfa,// 15 PAY  12 

    0xbac68007,// 16 PAY  13 

    0xcd56288e,// 17 PAY  14 

    0xb3d4ee17,// 18 PAY  15 

    0x5dd80258,// 19 PAY  16 

    0xe374e462,// 20 PAY  17 

    0xcf8c14b7,// 21 PAY  18 

    0x47aa4c82,// 22 PAY  19 

    0x2dad3a64,// 23 PAY  20 

    0x2f8073d6,// 24 PAY  21 

    0xb8b21c78,// 25 PAY  22 

    0x800bd08c,// 26 PAY  23 

    0x51e4f908,// 27 PAY  24 

    0x7765c2da,// 28 PAY  25 

    0x99b2b11a,// 29 PAY  26 

    0xd5cbb4b1,// 30 PAY  27 

    0x6c7e95cc,// 31 PAY  28 

    0x4e2c2c60,// 32 PAY  29 

    0xcf802244,// 33 PAY  30 

    0x5a817dbf,// 34 PAY  31 

    0xb0ee223f,// 35 PAY  32 

    0xcf53851b,// 36 PAY  33 

    0x00aaa4e6,// 37 PAY  34 

    0x6d332eb6,// 38 PAY  35 

    0x9f53cb6f,// 39 PAY  36 

    0x8817581c,// 40 PAY  37 

    0x13309474,// 41 PAY  38 

    0xc778e06b,// 42 PAY  39 

    0x47d71c26,// 43 PAY  40 

    0xb797ce4f,// 44 PAY  41 

    0x32c75569,// 45 PAY  42 

    0xc16eac66,// 46 PAY  43 

    0xccd825e6,// 47 PAY  44 

    0xcb435f09,// 48 PAY  45 

    0xf622e233,// 49 PAY  46 

    0x56a0f6cb,// 50 PAY  47 

    0x9db25123,// 51 PAY  48 

    0xf9925efd,// 52 PAY  49 

    0x03ceb460,// 53 PAY  50 

    0xf6a98542,// 54 PAY  51 

    0x5b6568eb,// 55 PAY  52 

    0xaca34ae7,// 56 PAY  53 

    0xefae466a,// 57 PAY  54 

    0xc8000000,// 58 PAY  55 

/// HASH is  4 bytes 

    0xf6a98542,// 59 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 179 

/// STA pkt_idx        : 154 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb8 

    0x0269b8b3 // 60 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt32_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 296 words. 

/// BDA size     is 1180 (0x49c) 

/// BDA id       is 0x1659 

    0x049c1659,// 3 BDA   1 

/// PAY Generic Data size   : 1180 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x0981550a,// 4 PAY   1 

    0x97d28bfb,// 5 PAY   2 

    0x5421b0ba,// 6 PAY   3 

    0x9a985b00,// 7 PAY   4 

    0x92b6f676,// 8 PAY   5 

    0x7d201e87,// 9 PAY   6 

    0x68d94a9c,// 10 PAY   7 

    0x6a482faf,// 11 PAY   8 

    0xe7a04a31,// 12 PAY   9 

    0x7f60d6ac,// 13 PAY  10 

    0x7e58a854,// 14 PAY  11 

    0xf47e6927,// 15 PAY  12 

    0x55850a40,// 16 PAY  13 

    0xf911e6b2,// 17 PAY  14 

    0x7ceda1f2,// 18 PAY  15 

    0x83df8bad,// 19 PAY  16 

    0xe09eaeac,// 20 PAY  17 

    0x14fcc4b9,// 21 PAY  18 

    0xdb5d68ed,// 22 PAY  19 

    0x457270a5,// 23 PAY  20 

    0x289543cc,// 24 PAY  21 

    0xf0d68512,// 25 PAY  22 

    0xf8d70f96,// 26 PAY  23 

    0xcc0f7d53,// 27 PAY  24 

    0x4c42c4e0,// 28 PAY  25 

    0x68709158,// 29 PAY  26 

    0xa7faaa6f,// 30 PAY  27 

    0xe3e11682,// 31 PAY  28 

    0x6ce6f122,// 32 PAY  29 

    0x5fe448df,// 33 PAY  30 

    0x2ed558f0,// 34 PAY  31 

    0x1c582ac9,// 35 PAY  32 

    0x9b26337d,// 36 PAY  33 

    0x7ca6877c,// 37 PAY  34 

    0xeb519d8a,// 38 PAY  35 

    0xa98a6dcd,// 39 PAY  36 

    0x44546879,// 40 PAY  37 

    0x2b853595,// 41 PAY  38 

    0x62363c2d,// 42 PAY  39 

    0x662997dc,// 43 PAY  40 

    0x5d04c0bb,// 44 PAY  41 

    0xd15e1838,// 45 PAY  42 

    0xac2c6067,// 46 PAY  43 

    0xc860719a,// 47 PAY  44 

    0x94436586,// 48 PAY  45 

    0xe11eab69,// 49 PAY  46 

    0xa1736c83,// 50 PAY  47 

    0x6f543f8e,// 51 PAY  48 

    0xfe3b06b7,// 52 PAY  49 

    0x2753ccc9,// 53 PAY  50 

    0x1ed6260b,// 54 PAY  51 

    0xf3a059dd,// 55 PAY  52 

    0xbc51a405,// 56 PAY  53 

    0x316affc2,// 57 PAY  54 

    0x2d777d49,// 58 PAY  55 

    0x20219c21,// 59 PAY  56 

    0x8e48a16e,// 60 PAY  57 

    0xb8a1f739,// 61 PAY  58 

    0x63cfd480,// 62 PAY  59 

    0x2afdb21b,// 63 PAY  60 

    0x52770be1,// 64 PAY  61 

    0x55fe5403,// 65 PAY  62 

    0xf790fda9,// 66 PAY  63 

    0xc74d3b2d,// 67 PAY  64 

    0x5c6f829a,// 68 PAY  65 

    0x64eb4c61,// 69 PAY  66 

    0x1d0f38b5,// 70 PAY  67 

    0x7ab0523c,// 71 PAY  68 

    0xd4143530,// 72 PAY  69 

    0x42f8899a,// 73 PAY  70 

    0x78b56e89,// 74 PAY  71 

    0xb139b0c1,// 75 PAY  72 

    0x70f1eb90,// 76 PAY  73 

    0x9956d31b,// 77 PAY  74 

    0x20e48e35,// 78 PAY  75 

    0xbb65d496,// 79 PAY  76 

    0x2f855c44,// 80 PAY  77 

    0x30f9e957,// 81 PAY  78 

    0x64c70319,// 82 PAY  79 

    0x9d309894,// 83 PAY  80 

    0x793fcd17,// 84 PAY  81 

    0x479b7857,// 85 PAY  82 

    0x9e6454e4,// 86 PAY  83 

    0x4bd08c98,// 87 PAY  84 

    0xee3d5558,// 88 PAY  85 

    0xc5e7058a,// 89 PAY  86 

    0x59d8425e,// 90 PAY  87 

    0x314ef0f5,// 91 PAY  88 

    0xa7542e38,// 92 PAY  89 

    0x71e98149,// 93 PAY  90 

    0x1158b5f6,// 94 PAY  91 

    0x9f4ad3f3,// 95 PAY  92 

    0x0592ad47,// 96 PAY  93 

    0x2540efee,// 97 PAY  94 

    0xbcb17a77,// 98 PAY  95 

    0x70fcc651,// 99 PAY  96 

    0x0946df42,// 100 PAY  97 

    0xb828a4d5,// 101 PAY  98 

    0xa94cc6b6,// 102 PAY  99 

    0xb5f2eaaa,// 103 PAY 100 

    0xeea47f98,// 104 PAY 101 

    0x1864447c,// 105 PAY 102 

    0x6c52202a,// 106 PAY 103 

    0xfa9774f1,// 107 PAY 104 

    0x306c6a6e,// 108 PAY 105 

    0x1d5c23c6,// 109 PAY 106 

    0x1b979186,// 110 PAY 107 

    0xf6af1ef0,// 111 PAY 108 

    0xfd839aa4,// 112 PAY 109 

    0xaf8c03b0,// 113 PAY 110 

    0xd66e3f74,// 114 PAY 111 

    0x59d96b20,// 115 PAY 112 

    0x8c815dae,// 116 PAY 113 

    0xb652bd56,// 117 PAY 114 

    0xace4fb2c,// 118 PAY 115 

    0xaf92bdab,// 119 PAY 116 

    0x92949cd8,// 120 PAY 117 

    0xffe5b3d6,// 121 PAY 118 

    0xc81fdba3,// 122 PAY 119 

    0xa72fef16,// 123 PAY 120 

    0x301a2011,// 124 PAY 121 

    0xdf157854,// 125 PAY 122 

    0x72f7c331,// 126 PAY 123 

    0xc220ff8a,// 127 PAY 124 

    0xc0f9d67e,// 128 PAY 125 

    0xd572ce5f,// 129 PAY 126 

    0x8b22933d,// 130 PAY 127 

    0x8c930082,// 131 PAY 128 

    0xbcaa9a51,// 132 PAY 129 

    0x31859a70,// 133 PAY 130 

    0x6eaf6b87,// 134 PAY 131 

    0x73ab0652,// 135 PAY 132 

    0xce0eb0ba,// 136 PAY 133 

    0x5b3f81fb,// 137 PAY 134 

    0x5b3d3c77,// 138 PAY 135 

    0xbe206c8a,// 139 PAY 136 

    0x9392fe18,// 140 PAY 137 

    0xbdd66538,// 141 PAY 138 

    0xb3a50421,// 142 PAY 139 

    0xadb34218,// 143 PAY 140 

    0x82947629,// 144 PAY 141 

    0xa2a58b03,// 145 PAY 142 

    0x9f86a174,// 146 PAY 143 

    0xff7e249d,// 147 PAY 144 

    0x5bca5076,// 148 PAY 145 

    0x8acc21ce,// 149 PAY 146 

    0xe246c692,// 150 PAY 147 

    0x3cb04808,// 151 PAY 148 

    0xb27b474e,// 152 PAY 149 

    0xd10e2c71,// 153 PAY 150 

    0x2b963a6f,// 154 PAY 151 

    0xbeb985e1,// 155 PAY 152 

    0x667b76ca,// 156 PAY 153 

    0x952a090c,// 157 PAY 154 

    0x88b44bcc,// 158 PAY 155 

    0x6a705571,// 159 PAY 156 

    0x0b2b27df,// 160 PAY 157 

    0xcacfad69,// 161 PAY 158 

    0xda9fb245,// 162 PAY 159 

    0x14fa8cd8,// 163 PAY 160 

    0x39927342,// 164 PAY 161 

    0x0f054867,// 165 PAY 162 

    0x79b9e119,// 166 PAY 163 

    0x4cd66fd6,// 167 PAY 164 

    0x11839461,// 168 PAY 165 

    0xe480b3c4,// 169 PAY 166 

    0x2f792723,// 170 PAY 167 

    0x9f478d4d,// 171 PAY 168 

    0x24770dff,// 172 PAY 169 

    0xeb37641b,// 173 PAY 170 

    0xca0326e9,// 174 PAY 171 

    0x096113ca,// 175 PAY 172 

    0x046aa317,// 176 PAY 173 

    0x16714986,// 177 PAY 174 

    0x9eb79df3,// 178 PAY 175 

    0x351a163a,// 179 PAY 176 

    0x4fca6f16,// 180 PAY 177 

    0x76381202,// 181 PAY 178 

    0xf59a1712,// 182 PAY 179 

    0x48184c41,// 183 PAY 180 

    0x7c8736a0,// 184 PAY 181 

    0x2122c969,// 185 PAY 182 

    0xe46132fd,// 186 PAY 183 

    0xa9f7b8b4,// 187 PAY 184 

    0xfffb6202,// 188 PAY 185 

    0x5b81096d,// 189 PAY 186 

    0x5e9aae75,// 190 PAY 187 

    0x904cfefb,// 191 PAY 188 

    0x4a85e84f,// 192 PAY 189 

    0xd90c31af,// 193 PAY 190 

    0x7d59eca0,// 194 PAY 191 

    0x1af9bb89,// 195 PAY 192 

    0xa4901c73,// 196 PAY 193 

    0x59838242,// 197 PAY 194 

    0xe984f97f,// 198 PAY 195 

    0x79c39adb,// 199 PAY 196 

    0x5ce6f846,// 200 PAY 197 

    0x2229b824,// 201 PAY 198 

    0xa34dab44,// 202 PAY 199 

    0x8bdd32e6,// 203 PAY 200 

    0x4083171a,// 204 PAY 201 

    0x8ce54246,// 205 PAY 202 

    0x39fc4077,// 206 PAY 203 

    0x4957fd95,// 207 PAY 204 

    0x54960afe,// 208 PAY 205 

    0x9c200f7a,// 209 PAY 206 

    0x00f04868,// 210 PAY 207 

    0x1c0ee7c8,// 211 PAY 208 

    0xa83efb9d,// 212 PAY 209 

    0xed8322d9,// 213 PAY 210 

    0x8b601365,// 214 PAY 211 

    0x282e85d9,// 215 PAY 212 

    0x9589278a,// 216 PAY 213 

    0x297701ec,// 217 PAY 214 

    0xcc25c0e4,// 218 PAY 215 

    0xbc7ab40d,// 219 PAY 216 

    0x8273a98e,// 220 PAY 217 

    0xa55f6ee4,// 221 PAY 218 

    0x9f9d3563,// 222 PAY 219 

    0x6fc4c702,// 223 PAY 220 

    0x11eb9d7b,// 224 PAY 221 

    0xfe7b8d90,// 225 PAY 222 

    0xe5ba815f,// 226 PAY 223 

    0x1e490633,// 227 PAY 224 

    0x19b7b733,// 228 PAY 225 

    0xd6c43348,// 229 PAY 226 

    0x8da1a1a9,// 230 PAY 227 

    0x17d55d71,// 231 PAY 228 

    0x1cf9753c,// 232 PAY 229 

    0x4ab2bba2,// 233 PAY 230 

    0xb9aecef0,// 234 PAY 231 

    0x089c6abe,// 235 PAY 232 

    0xfe07dfe1,// 236 PAY 233 

    0x3b9eb742,// 237 PAY 234 

    0xfb9ef00b,// 238 PAY 235 

    0x6b31548a,// 239 PAY 236 

    0x7e576bf3,// 240 PAY 237 

    0x190728bb,// 241 PAY 238 

    0x9ab4f73c,// 242 PAY 239 

    0x8a3a4aae,// 243 PAY 240 

    0x32519a85,// 244 PAY 241 

    0x5ec8d2d1,// 245 PAY 242 

    0x6e3cc4f9,// 246 PAY 243 

    0x9fc1c1b4,// 247 PAY 244 

    0xbf1e6964,// 248 PAY 245 

    0x46e629f5,// 249 PAY 246 

    0xe318d596,// 250 PAY 247 

    0xaafc62b3,// 251 PAY 248 

    0xb7ea6b44,// 252 PAY 249 

    0x107d8ea5,// 253 PAY 250 

    0x3ec3e886,// 254 PAY 251 

    0x7a81af13,// 255 PAY 252 

    0x76b195ea,// 256 PAY 253 

    0x39cdc935,// 257 PAY 254 

    0xda6b6144,// 258 PAY 255 

    0x0465f66a,// 259 PAY 256 

    0x4954bbd2,// 260 PAY 257 

    0x4a691c43,// 261 PAY 258 

    0xc1a65885,// 262 PAY 259 

    0xe0ffa591,// 263 PAY 260 

    0xfc96f0e2,// 264 PAY 261 

    0xef1a29f0,// 265 PAY 262 

    0x743ab6ee,// 266 PAY 263 

    0xfd84f9a7,// 267 PAY 264 

    0xb7cd35e4,// 268 PAY 265 

    0x0b0e91a6,// 269 PAY 266 

    0x00ac369f,// 270 PAY 267 

    0x3b28f1cf,// 271 PAY 268 

    0x3b47c8a8,// 272 PAY 269 

    0x889110e2,// 273 PAY 270 

    0xcbda37ab,// 274 PAY 271 

    0xf5b1f0d0,// 275 PAY 272 

    0x6badef20,// 276 PAY 273 

    0x2886f89a,// 277 PAY 274 

    0xa5221ecc,// 278 PAY 275 

    0xaed93499,// 279 PAY 276 

    0x8c380e4b,// 280 PAY 277 

    0x076e5b06,// 281 PAY 278 

    0xf4cba67d,// 282 PAY 279 

    0xb84cd1cc,// 283 PAY 280 

    0xa118ca56,// 284 PAY 281 

    0x7a5ac668,// 285 PAY 282 

    0x6087176e,// 286 PAY 283 

    0xb5d52a78,// 287 PAY 284 

    0x24fb1116,// 288 PAY 285 

    0x83edeb4a,// 289 PAY 286 

    0xf85eaa16,// 290 PAY 287 

    0x440224e4,// 291 PAY 288 

    0xef605a7d,// 292 PAY 289 

    0x61dfe84d,// 293 PAY 290 

    0xc0da5f83,// 294 PAY 291 

    0x7493ef05,// 295 PAY 292 

    0xb2e5da4f,// 296 PAY 293 

    0x0a3f8904,// 297 PAY 294 

    0xd05b1459,// 298 PAY 295 

/// HASH is  8 bytes 

    0xa7542e38,// 299 HSH   1 

    0x71e98149,// 300 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 75 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x28 

    0x0111284b // 301 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt33_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 407 words. 

/// BDA size     is 1621 (0x655) 

/// BDA id       is 0x8db2 

    0x06558db2,// 3 BDA   1 

/// PAY Generic Data size   : 1621 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xe90c381d,// 4 PAY   1 

    0x21253887,// 5 PAY   2 

    0x83521b44,// 6 PAY   3 

    0x83ba30c3,// 7 PAY   4 

    0x96ae08b5,// 8 PAY   5 

    0xf2ce9e74,// 9 PAY   6 

    0xf602cf3c,// 10 PAY   7 

    0xbc87a4aa,// 11 PAY   8 

    0xa0e4f1f7,// 12 PAY   9 

    0xaa48762c,// 13 PAY  10 

    0xb7b3900a,// 14 PAY  11 

    0x839854d1,// 15 PAY  12 

    0xbf9a27d2,// 16 PAY  13 

    0x4009b783,// 17 PAY  14 

    0xc90f14b0,// 18 PAY  15 

    0x4d7408b7,// 19 PAY  16 

    0x420eead1,// 20 PAY  17 

    0x6900c8dd,// 21 PAY  18 

    0x7b3d59bd,// 22 PAY  19 

    0x9ee15267,// 23 PAY  20 

    0xdc172585,// 24 PAY  21 

    0xc7613b08,// 25 PAY  22 

    0x61c31ca0,// 26 PAY  23 

    0x0e0a8a9f,// 27 PAY  24 

    0xd7172dc6,// 28 PAY  25 

    0x3b93cddb,// 29 PAY  26 

    0x1ef6ed4f,// 30 PAY  27 

    0x6e7c3a61,// 31 PAY  28 

    0x15349a13,// 32 PAY  29 

    0x613006ed,// 33 PAY  30 

    0xce29c284,// 34 PAY  31 

    0xf2c8a447,// 35 PAY  32 

    0xf6f06e9b,// 36 PAY  33 

    0xa26b1531,// 37 PAY  34 

    0x50537162,// 38 PAY  35 

    0x9ea65629,// 39 PAY  36 

    0xa8844f83,// 40 PAY  37 

    0x2aebe292,// 41 PAY  38 

    0xd3ee7c39,// 42 PAY  39 

    0x6b94f80d,// 43 PAY  40 

    0xbedfe334,// 44 PAY  41 

    0x278c36c8,// 45 PAY  42 

    0x963041b3,// 46 PAY  43 

    0x5e929bf7,// 47 PAY  44 

    0x9561fd77,// 48 PAY  45 

    0xcb388331,// 49 PAY  46 

    0x80e66b5a,// 50 PAY  47 

    0x70957cc3,// 51 PAY  48 

    0xd59a704b,// 52 PAY  49 

    0xc52b25cc,// 53 PAY  50 

    0x98b85508,// 54 PAY  51 

    0xfd00c1ec,// 55 PAY  52 

    0xe2820b1e,// 56 PAY  53 

    0x58920b9a,// 57 PAY  54 

    0x555ec779,// 58 PAY  55 

    0xddbcf506,// 59 PAY  56 

    0x12f849e0,// 60 PAY  57 

    0xc528711d,// 61 PAY  58 

    0x2c13cf87,// 62 PAY  59 

    0xd165b0d0,// 63 PAY  60 

    0x7b88587b,// 64 PAY  61 

    0xd519e896,// 65 PAY  62 

    0xa4638ea7,// 66 PAY  63 

    0x3e523fd6,// 67 PAY  64 

    0xa25693c0,// 68 PAY  65 

    0x5e97527c,// 69 PAY  66 

    0x78e2aa1b,// 70 PAY  67 

    0xcab75821,// 71 PAY  68 

    0x7e07a93d,// 72 PAY  69 

    0xd2ee6fbf,// 73 PAY  70 

    0xf81f72e0,// 74 PAY  71 

    0xa88e752f,// 75 PAY  72 

    0x25de4a22,// 76 PAY  73 

    0xb3a38224,// 77 PAY  74 

    0x718599a7,// 78 PAY  75 

    0x770fafd1,// 79 PAY  76 

    0x15d3cba9,// 80 PAY  77 

    0x04be3598,// 81 PAY  78 

    0x55ed6c06,// 82 PAY  79 

    0x61a6a77b,// 83 PAY  80 

    0xd4dce8a2,// 84 PAY  81 

    0x0169d013,// 85 PAY  82 

    0x309a3039,// 86 PAY  83 

    0x13f62cf5,// 87 PAY  84 

    0x6b694ef1,// 88 PAY  85 

    0x2e94b632,// 89 PAY  86 

    0xb09a2e78,// 90 PAY  87 

    0xc6e5243f,// 91 PAY  88 

    0x581f9cec,// 92 PAY  89 

    0x72e28cf8,// 93 PAY  90 

    0xbc3531e3,// 94 PAY  91 

    0xa3841a06,// 95 PAY  92 

    0xce355f7a,// 96 PAY  93 

    0x04585036,// 97 PAY  94 

    0x69433980,// 98 PAY  95 

    0x0f6f2ff7,// 99 PAY  96 

    0x98ff57cd,// 100 PAY  97 

    0x1a435129,// 101 PAY  98 

    0x7efff22e,// 102 PAY  99 

    0xf60640c9,// 103 PAY 100 

    0x1860c115,// 104 PAY 101 

    0x65c894a7,// 105 PAY 102 

    0xbd0feb8a,// 106 PAY 103 

    0x7684d49e,// 107 PAY 104 

    0x0e127fe6,// 108 PAY 105 

    0x1a7230b1,// 109 PAY 106 

    0x8fb6deda,// 110 PAY 107 

    0x937df72b,// 111 PAY 108 

    0x6f6b90a3,// 112 PAY 109 

    0xbad6632d,// 113 PAY 110 

    0x4751b736,// 114 PAY 111 

    0x0c1b7b87,// 115 PAY 112 

    0xfdce8b08,// 116 PAY 113 

    0x0157dafd,// 117 PAY 114 

    0x24cc714f,// 118 PAY 115 

    0xc96d64cd,// 119 PAY 116 

    0x8ae9eb60,// 120 PAY 117 

    0x7a178864,// 121 PAY 118 

    0xca2411ac,// 122 PAY 119 

    0x8ae76eb7,// 123 PAY 120 

    0x8c5f4653,// 124 PAY 121 

    0xdecba7b8,// 125 PAY 122 

    0xdb400265,// 126 PAY 123 

    0x961ea054,// 127 PAY 124 

    0xb1ab37a8,// 128 PAY 125 

    0x07f2565d,// 129 PAY 126 

    0xe49e6b04,// 130 PAY 127 

    0xf78c51ba,// 131 PAY 128 

    0x18ef07ae,// 132 PAY 129 

    0x36931b1b,// 133 PAY 130 

    0x47780ad6,// 134 PAY 131 

    0x1f3ca8e8,// 135 PAY 132 

    0x1ed896b1,// 136 PAY 133 

    0x0931f2e7,// 137 PAY 134 

    0xd3f1d907,// 138 PAY 135 

    0xb381a2ae,// 139 PAY 136 

    0x427716c7,// 140 PAY 137 

    0x4bceddc4,// 141 PAY 138 

    0x9bfdb80a,// 142 PAY 139 

    0x10689d23,// 143 PAY 140 

    0x144e3b02,// 144 PAY 141 

    0x66108c8f,// 145 PAY 142 

    0x85e58975,// 146 PAY 143 

    0x83d7ead5,// 147 PAY 144 

    0x8a2f73cf,// 148 PAY 145 

    0x1ba7ef5d,// 149 PAY 146 

    0x496f7266,// 150 PAY 147 

    0x2c558ad1,// 151 PAY 148 

    0xec00d638,// 152 PAY 149 

    0x2e5b410d,// 153 PAY 150 

    0x829ddd1a,// 154 PAY 151 

    0x4d4a8f62,// 155 PAY 152 

    0xdcf98780,// 156 PAY 153 

    0x1c29789f,// 157 PAY 154 

    0xcd0e9f6a,// 158 PAY 155 

    0x7a4c7717,// 159 PAY 156 

    0xcb6f7e7f,// 160 PAY 157 

    0xf3888f4f,// 161 PAY 158 

    0x554faecb,// 162 PAY 159 

    0x128705d0,// 163 PAY 160 

    0x13ba3ade,// 164 PAY 161 

    0xda232b3e,// 165 PAY 162 

    0xd1d6f636,// 166 PAY 163 

    0x14b433cb,// 167 PAY 164 

    0x1d9a9841,// 168 PAY 165 

    0x331ef720,// 169 PAY 166 

    0x4b3819bb,// 170 PAY 167 

    0x953dfb18,// 171 PAY 168 

    0x9d1aa90e,// 172 PAY 169 

    0xa7338743,// 173 PAY 170 

    0xc6a7fa6f,// 174 PAY 171 

    0x1f7e0aeb,// 175 PAY 172 

    0x651f4ac5,// 176 PAY 173 

    0x6b0d3f93,// 177 PAY 174 

    0xf790f411,// 178 PAY 175 

    0x1a45c3d4,// 179 PAY 176 

    0xed745e3e,// 180 PAY 177 

    0x694ed9be,// 181 PAY 178 

    0xae92141b,// 182 PAY 179 

    0xa4d4c325,// 183 PAY 180 

    0x77b561e3,// 184 PAY 181 

    0xc371b1e6,// 185 PAY 182 

    0x316af308,// 186 PAY 183 

    0x6c08a8a0,// 187 PAY 184 

    0x73bbd37a,// 188 PAY 185 

    0x0c477a45,// 189 PAY 186 

    0xdd784d0d,// 190 PAY 187 

    0xba7e5a55,// 191 PAY 188 

    0x85dea188,// 192 PAY 189 

    0x8a1f5ef7,// 193 PAY 190 

    0x99c25d80,// 194 PAY 191 

    0x324c4890,// 195 PAY 192 

    0xb6433015,// 196 PAY 193 

    0xcfa00e15,// 197 PAY 194 

    0x14eb1953,// 198 PAY 195 

    0xa42c4d76,// 199 PAY 196 

    0x4ce2a4d4,// 200 PAY 197 

    0x9b4cd1ae,// 201 PAY 198 

    0xad2c6dc0,// 202 PAY 199 

    0x81614f17,// 203 PAY 200 

    0x558ee608,// 204 PAY 201 

    0xf229a065,// 205 PAY 202 

    0x3c93a8f4,// 206 PAY 203 

    0x63caea84,// 207 PAY 204 

    0xe6a60939,// 208 PAY 205 

    0x724439f1,// 209 PAY 206 

    0x46d94ba7,// 210 PAY 207 

    0x3e125262,// 211 PAY 208 

    0x171bad54,// 212 PAY 209 

    0x56601712,// 213 PAY 210 

    0x4f7f1327,// 214 PAY 211 

    0xe08f9896,// 215 PAY 212 

    0xd9deaa61,// 216 PAY 213 

    0x9333b225,// 217 PAY 214 

    0x4f6bacd1,// 218 PAY 215 

    0xb34a2bbd,// 219 PAY 216 

    0xf5bf880d,// 220 PAY 217 

    0x8181d765,// 221 PAY 218 

    0xecb3b7de,// 222 PAY 219 

    0xb8499907,// 223 PAY 220 

    0xc96e28b6,// 224 PAY 221 

    0x219b31b0,// 225 PAY 222 

    0x2d596cb5,// 226 PAY 223 

    0xe825ca38,// 227 PAY 224 

    0x8bba4790,// 228 PAY 225 

    0x2beacfe7,// 229 PAY 226 

    0x27cc33ea,// 230 PAY 227 

    0x7b8f71d4,// 231 PAY 228 

    0xac42e979,// 232 PAY 229 

    0x85dc5db8,// 233 PAY 230 

    0xeac91e7d,// 234 PAY 231 

    0x0d54159c,// 235 PAY 232 

    0x120cb784,// 236 PAY 233 

    0xb85d2122,// 237 PAY 234 

    0xf3fa4659,// 238 PAY 235 

    0xb4e3473d,// 239 PAY 236 

    0x6f4ffb20,// 240 PAY 237 

    0xf06b0e4d,// 241 PAY 238 

    0x991ca390,// 242 PAY 239 

    0xa79a97f3,// 243 PAY 240 

    0xea2a36db,// 244 PAY 241 

    0x1c60908c,// 245 PAY 242 

    0x8b25f871,// 246 PAY 243 

    0x66e13c7c,// 247 PAY 244 

    0x7e06c6f6,// 248 PAY 245 

    0x72f10e16,// 249 PAY 246 

    0x5bc8eef4,// 250 PAY 247 

    0x7629a7f7,// 251 PAY 248 

    0x7865c839,// 252 PAY 249 

    0x4029f20f,// 253 PAY 250 

    0x29a4666d,// 254 PAY 251 

    0x2f60c4e4,// 255 PAY 252 

    0x26fd0bca,// 256 PAY 253 

    0x750a5e3c,// 257 PAY 254 

    0xc959bd49,// 258 PAY 255 

    0xdec68cba,// 259 PAY 256 

    0x13848c41,// 260 PAY 257 

    0xb726fa04,// 261 PAY 258 

    0x0929aea0,// 262 PAY 259 

    0x8deb2bc0,// 263 PAY 260 

    0x7e93a834,// 264 PAY 261 

    0x50f2dd78,// 265 PAY 262 

    0x8017f19f,// 266 PAY 263 

    0xee7539ec,// 267 PAY 264 

    0xbd91d663,// 268 PAY 265 

    0xbe04b1a6,// 269 PAY 266 

    0xe3f01bb2,// 270 PAY 267 

    0x2f5e1a6b,// 271 PAY 268 

    0xa1e27727,// 272 PAY 269 

    0x79549205,// 273 PAY 270 

    0x6cc0093a,// 274 PAY 271 

    0xbb083943,// 275 PAY 272 

    0xa1bf595a,// 276 PAY 273 

    0xed7b3a9f,// 277 PAY 274 

    0x732d7f59,// 278 PAY 275 

    0x7259a81c,// 279 PAY 276 

    0xadddfebd,// 280 PAY 277 

    0x3e954a97,// 281 PAY 278 

    0x54769ac1,// 282 PAY 279 

    0xa5c3777f,// 283 PAY 280 

    0x2bc7cd36,// 284 PAY 281 

    0x2dcb4226,// 285 PAY 282 

    0x963ff68d,// 286 PAY 283 

    0xfe924176,// 287 PAY 284 

    0x347f45db,// 288 PAY 285 

    0x0232b214,// 289 PAY 286 

    0xf9d9b90c,// 290 PAY 287 

    0xa3f82095,// 291 PAY 288 

    0xd0eb2cb3,// 292 PAY 289 

    0x3cccfa16,// 293 PAY 290 

    0x206c1497,// 294 PAY 291 

    0xb171c348,// 295 PAY 292 

    0x1ccb9e82,// 296 PAY 293 

    0x53ad6d24,// 297 PAY 294 

    0x84bc47bb,// 298 PAY 295 

    0xc99b2f9e,// 299 PAY 296 

    0x5c5e5c4d,// 300 PAY 297 

    0x9df5151a,// 301 PAY 298 

    0x2e3eaa04,// 302 PAY 299 

    0x0a00c275,// 303 PAY 300 

    0x00ed4b48,// 304 PAY 301 

    0xd4548018,// 305 PAY 302 

    0x39fb96c6,// 306 PAY 303 

    0xc923242f,// 307 PAY 304 

    0xc1bcd72c,// 308 PAY 305 

    0x38745d9e,// 309 PAY 306 

    0x80fdc648,// 310 PAY 307 

    0x8417df21,// 311 PAY 308 

    0xfc07279e,// 312 PAY 309 

    0xd91713e8,// 313 PAY 310 

    0x78e6cacd,// 314 PAY 311 

    0xc0faebbe,// 315 PAY 312 

    0xfbc5b8e0,// 316 PAY 313 

    0x89136ae3,// 317 PAY 314 

    0x434dd055,// 318 PAY 315 

    0xea99b5d3,// 319 PAY 316 

    0x230ad611,// 320 PAY 317 

    0xea53093e,// 321 PAY 318 

    0xd743d417,// 322 PAY 319 

    0x34c8a5f6,// 323 PAY 320 

    0xfec531d1,// 324 PAY 321 

    0x2bd1c66c,// 325 PAY 322 

    0xef9a1bd4,// 326 PAY 323 

    0x74c3e167,// 327 PAY 324 

    0xce6565d1,// 328 PAY 325 

    0x31f7bc01,// 329 PAY 326 

    0xa531187c,// 330 PAY 327 

    0x6667ff70,// 331 PAY 328 

    0xae480537,// 332 PAY 329 

    0x629d1f1b,// 333 PAY 330 

    0x4b11648b,// 334 PAY 331 

    0xd3daec35,// 335 PAY 332 

    0x09effa22,// 336 PAY 333 

    0x9ae0f85e,// 337 PAY 334 

    0x029a873a,// 338 PAY 335 

    0x80be1ad0,// 339 PAY 336 

    0x8ba248e1,// 340 PAY 337 

    0x4c62929a,// 341 PAY 338 

    0x2510a4ad,// 342 PAY 339 

    0x31b7b1e5,// 343 PAY 340 

    0x7d65e72f,// 344 PAY 341 

    0x4bcd178e,// 345 PAY 342 

    0x6f4866ed,// 346 PAY 343 

    0x535c4dc5,// 347 PAY 344 

    0x81dbbcd7,// 348 PAY 345 

    0x138db0ca,// 349 PAY 346 

    0x34e8183f,// 350 PAY 347 

    0x1c1b9a4c,// 351 PAY 348 

    0x3d5f9f61,// 352 PAY 349 

    0x7a3e58f7,// 353 PAY 350 

    0x6a9b4112,// 354 PAY 351 

    0x0e7dd904,// 355 PAY 352 

    0x82bd861b,// 356 PAY 353 

    0xf6eba995,// 357 PAY 354 

    0xa7eb7828,// 358 PAY 355 

    0x708cc61f,// 359 PAY 356 

    0xf2451b82,// 360 PAY 357 

    0x3e4fca35,// 361 PAY 358 

    0x2e5e70d4,// 362 PAY 359 

    0x8bb3655d,// 363 PAY 360 

    0xdb3528bf,// 364 PAY 361 

    0x4ef9ca4e,// 365 PAY 362 

    0x980e9360,// 366 PAY 363 

    0x472d7135,// 367 PAY 364 

    0xf45f6707,// 368 PAY 365 

    0x3b7ad946,// 369 PAY 366 

    0xde3075fc,// 370 PAY 367 

    0x39636fd6,// 371 PAY 368 

    0xf8c29558,// 372 PAY 369 

    0x1e690ba5,// 373 PAY 370 

    0xcc484d99,// 374 PAY 371 

    0x3608b1ba,// 375 PAY 372 

    0xd3653680,// 376 PAY 373 

    0xc4fb00d8,// 377 PAY 374 

    0x00ed547b,// 378 PAY 375 

    0x9d9a6e6f,// 379 PAY 376 

    0xfe96de8f,// 380 PAY 377 

    0xc168cb30,// 381 PAY 378 

    0x7847d2d9,// 382 PAY 379 

    0x4a95fa56,// 383 PAY 380 

    0xf5d636a1,// 384 PAY 381 

    0x9fa6b8b3,// 385 PAY 382 

    0x2e72b2e8,// 386 PAY 383 

    0x84c42893,// 387 PAY 384 

    0xfc9890df,// 388 PAY 385 

    0xa9bdc6d2,// 389 PAY 386 

    0x111f32ac,// 390 PAY 387 

    0x05dea20b,// 391 PAY 388 

    0x1dab9347,// 392 PAY 389 

    0xcdaba66d,// 393 PAY 390 

    0xf68b7782,// 394 PAY 391 

    0x5329bb33,// 395 PAY 392 

    0x64b73fc2,// 396 PAY 393 

    0xa3f0c605,// 397 PAY 394 

    0x437b78b6,// 398 PAY 395 

    0xc58b7f86,// 399 PAY 396 

    0x0ded6940,// 400 PAY 397 

    0xd2b13150,// 401 PAY 398 

    0x8b08e36f,// 402 PAY 399 

    0x18ea4785,// 403 PAY 400 

    0x20bc6b1b,// 404 PAY 401 

    0x68f4b9c6,// 405 PAY 402 

    0xa331d34c,// 406 PAY 403 

    0xae72b2f3,// 407 PAY 404 

    0x1deede18,// 408 PAY 405 

    0xbe000000,// 409 PAY 406 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 141 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb2 

    0x0234b237 // 410 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt34_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 439 words. 

/// BDA size     is 1751 (0x6d7) 

/// BDA id       is 0xaed0 

    0x06d7aed0,// 3 BDA   1 

/// PAY Generic Data size   : 1751 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xbf5d63f4,// 4 PAY   1 

    0xd8a26cea,// 5 PAY   2 

    0x224630bd,// 6 PAY   3 

    0xb9bc71c1,// 7 PAY   4 

    0x8b284107,// 8 PAY   5 

    0x90713570,// 9 PAY   6 

    0x969afc99,// 10 PAY   7 

    0x1dca7df7,// 11 PAY   8 

    0xc5204b46,// 12 PAY   9 

    0x285632af,// 13 PAY  10 

    0x1c8fbcf6,// 14 PAY  11 

    0xe36aea93,// 15 PAY  12 

    0x3b6b8eb6,// 16 PAY  13 

    0x27958877,// 17 PAY  14 

    0xb7ca9c97,// 18 PAY  15 

    0x630cc3e8,// 19 PAY  16 

    0xd467bb4a,// 20 PAY  17 

    0xd639a5f0,// 21 PAY  18 

    0xa5f2f909,// 22 PAY  19 

    0xf7d15f70,// 23 PAY  20 

    0x727d44f0,// 24 PAY  21 

    0xfb2b9e55,// 25 PAY  22 

    0xbb9b2d45,// 26 PAY  23 

    0x5ae8008b,// 27 PAY  24 

    0x0785d2b3,// 28 PAY  25 

    0x16452d8f,// 29 PAY  26 

    0xd8e67bb9,// 30 PAY  27 

    0x37314b27,// 31 PAY  28 

    0x96e1334a,// 32 PAY  29 

    0x45699698,// 33 PAY  30 

    0x9b8cbdf8,// 34 PAY  31 

    0xc63418c4,// 35 PAY  32 

    0xd92b1331,// 36 PAY  33 

    0x3cccb362,// 37 PAY  34 

    0x76688afa,// 38 PAY  35 

    0xa7eca16c,// 39 PAY  36 

    0x73b127af,// 40 PAY  37 

    0x38047244,// 41 PAY  38 

    0xe4c80ed9,// 42 PAY  39 

    0x33986ab5,// 43 PAY  40 

    0x363692d5,// 44 PAY  41 

    0xaf74a572,// 45 PAY  42 

    0x9c7a20f8,// 46 PAY  43 

    0x98eaa150,// 47 PAY  44 

    0x0262cb53,// 48 PAY  45 

    0xe409b369,// 49 PAY  46 

    0x03632373,// 50 PAY  47 

    0x9eeaef2c,// 51 PAY  48 

    0x12006d53,// 52 PAY  49 

    0x493742f8,// 53 PAY  50 

    0x07882d12,// 54 PAY  51 

    0x58458820,// 55 PAY  52 

    0xe9d4c55f,// 56 PAY  53 

    0x5a5778ea,// 57 PAY  54 

    0x7b065672,// 58 PAY  55 

    0x425b7288,// 59 PAY  56 

    0xfe5da59e,// 60 PAY  57 

    0xbf81c5b9,// 61 PAY  58 

    0x2cdd14d9,// 62 PAY  59 

    0x2e0d50d8,// 63 PAY  60 

    0x4338e6e0,// 64 PAY  61 

    0x5b771614,// 65 PAY  62 

    0x611fbf37,// 66 PAY  63 

    0x7e87142a,// 67 PAY  64 

    0x0547a45b,// 68 PAY  65 

    0x86431373,// 69 PAY  66 

    0x40e51aef,// 70 PAY  67 

    0xec5f839a,// 71 PAY  68 

    0x5cddd369,// 72 PAY  69 

    0x55d44b14,// 73 PAY  70 

    0x886abd97,// 74 PAY  71 

    0x3b4d1904,// 75 PAY  72 

    0xf127d206,// 76 PAY  73 

    0x462731e1,// 77 PAY  74 

    0xc5045ae9,// 78 PAY  75 

    0xcf1198ef,// 79 PAY  76 

    0x62403f0d,// 80 PAY  77 

    0x25007392,// 81 PAY  78 

    0x33f1e7ea,// 82 PAY  79 

    0x27869f27,// 83 PAY  80 

    0x8b9d6c2b,// 84 PAY  81 

    0x0885ce8e,// 85 PAY  82 

    0x30fd8788,// 86 PAY  83 

    0x640d05e3,// 87 PAY  84 

    0xdb2d701f,// 88 PAY  85 

    0xec94e48c,// 89 PAY  86 

    0xe3cfa226,// 90 PAY  87 

    0xe36c3633,// 91 PAY  88 

    0xbbbb9e69,// 92 PAY  89 

    0x8e8091b7,// 93 PAY  90 

    0xbc2491cb,// 94 PAY  91 

    0xc4566458,// 95 PAY  92 

    0xa2c61176,// 96 PAY  93 

    0x0f4d4b79,// 97 PAY  94 

    0xdb61fe45,// 98 PAY  95 

    0xfe26ee0c,// 99 PAY  96 

    0x0738dec7,// 100 PAY  97 

    0xe8317396,// 101 PAY  98 

    0xd1ba58e1,// 102 PAY  99 

    0x5245ec15,// 103 PAY 100 

    0xdaa0bed3,// 104 PAY 101 

    0xa2629e2b,// 105 PAY 102 

    0x3227d8aa,// 106 PAY 103 

    0x92f70ff2,// 107 PAY 104 

    0x8964c418,// 108 PAY 105 

    0x3f8e8932,// 109 PAY 106 

    0x4211f516,// 110 PAY 107 

    0xc6fde3a6,// 111 PAY 108 

    0x3e791c6a,// 112 PAY 109 

    0xd62724ae,// 113 PAY 110 

    0x076d3787,// 114 PAY 111 

    0xdc18dce3,// 115 PAY 112 

    0xc0f1fec2,// 116 PAY 113 

    0x28480a31,// 117 PAY 114 

    0x070a7449,// 118 PAY 115 

    0x9f2efb13,// 119 PAY 116 

    0x018b7a9b,// 120 PAY 117 

    0xbf2f2d42,// 121 PAY 118 

    0x40e4a6d2,// 122 PAY 119 

    0xee05fd1e,// 123 PAY 120 

    0xe60d5386,// 124 PAY 121 

    0x44b87038,// 125 PAY 122 

    0x174ac1e2,// 126 PAY 123 

    0x646bb304,// 127 PAY 124 

    0xa8390ea0,// 128 PAY 125 

    0xb66f97e5,// 129 PAY 126 

    0xea44a70c,// 130 PAY 127 

    0x637a84ba,// 131 PAY 128 

    0x43a64490,// 132 PAY 129 

    0x2244589b,// 133 PAY 130 

    0xdaee930b,// 134 PAY 131 

    0xbb708072,// 135 PAY 132 

    0xa95ba3af,// 136 PAY 133 

    0xc9ded651,// 137 PAY 134 

    0x4271f221,// 138 PAY 135 

    0xa7de8168,// 139 PAY 136 

    0x716072cb,// 140 PAY 137 

    0x4114987f,// 141 PAY 138 

    0x684ec04c,// 142 PAY 139 

    0xb93184ac,// 143 PAY 140 

    0x85eafe1e,// 144 PAY 141 

    0x5d75e417,// 145 PAY 142 

    0x79e2d91c,// 146 PAY 143 

    0xadfdea8a,// 147 PAY 144 

    0xedaf9584,// 148 PAY 145 

    0x8443e033,// 149 PAY 146 

    0xf000245d,// 150 PAY 147 

    0xdc5ec11a,// 151 PAY 148 

    0xd69318ae,// 152 PAY 149 

    0x9e74b65b,// 153 PAY 150 

    0x6630a630,// 154 PAY 151 

    0x3c334fb3,// 155 PAY 152 

    0x0223716c,// 156 PAY 153 

    0xb9db00f9,// 157 PAY 154 

    0xc60d2447,// 158 PAY 155 

    0x78847979,// 159 PAY 156 

    0x59854365,// 160 PAY 157 

    0x1d402266,// 161 PAY 158 

    0x36b057ba,// 162 PAY 159 

    0x228b9efc,// 163 PAY 160 

    0xfa7a7af2,// 164 PAY 161 

    0x4d391700,// 165 PAY 162 

    0x5205dc9e,// 166 PAY 163 

    0x199792e1,// 167 PAY 164 

    0x777e2a6d,// 168 PAY 165 

    0x0b8a6b71,// 169 PAY 166 

    0xecaa29ac,// 170 PAY 167 

    0x46e82704,// 171 PAY 168 

    0x7a3e7f99,// 172 PAY 169 

    0xe736a961,// 173 PAY 170 

    0x61356204,// 174 PAY 171 

    0xfbba9f5c,// 175 PAY 172 

    0xcebe95ad,// 176 PAY 173 

    0xb53e57b1,// 177 PAY 174 

    0xc0c4a3d2,// 178 PAY 175 

    0xffef571c,// 179 PAY 176 

    0x1f4d8558,// 180 PAY 177 

    0x3b30c0e9,// 181 PAY 178 

    0x9ca34205,// 182 PAY 179 

    0x7e5958ed,// 183 PAY 180 

    0xedd72295,// 184 PAY 181 

    0x7bc59782,// 185 PAY 182 

    0x0abecede,// 186 PAY 183 

    0x5b962dba,// 187 PAY 184 

    0xfdb7a062,// 188 PAY 185 

    0x69ebb806,// 189 PAY 186 

    0x35c90093,// 190 PAY 187 

    0xca3e3442,// 191 PAY 188 

    0x704d8928,// 192 PAY 189 

    0xe5062988,// 193 PAY 190 

    0xc85695a7,// 194 PAY 191 

    0x268f9cf4,// 195 PAY 192 

    0x6516aa45,// 196 PAY 193 

    0x4b092b04,// 197 PAY 194 

    0x713ae8f8,// 198 PAY 195 

    0xd8b15ff0,// 199 PAY 196 

    0x64511f90,// 200 PAY 197 

    0xe37a23e2,// 201 PAY 198 

    0x4c8c0526,// 202 PAY 199 

    0xb25b1b36,// 203 PAY 200 

    0x2e368e35,// 204 PAY 201 

    0x4cbd1c30,// 205 PAY 202 

    0x4966814a,// 206 PAY 203 

    0xb5c3984f,// 207 PAY 204 

    0x5c05895a,// 208 PAY 205 

    0xf5816f30,// 209 PAY 206 

    0x5c233c97,// 210 PAY 207 

    0x328aba75,// 211 PAY 208 

    0x2bbbb667,// 212 PAY 209 

    0x04a33793,// 213 PAY 210 

    0x91a9b728,// 214 PAY 211 

    0xa7afb07a,// 215 PAY 212 

    0x50ca47c0,// 216 PAY 213 

    0xd8b2434f,// 217 PAY 214 

    0x409ee1e2,// 218 PAY 215 

    0x498d0d13,// 219 PAY 216 

    0x824cf0ca,// 220 PAY 217 

    0xea73bf80,// 221 PAY 218 

    0x155fced0,// 222 PAY 219 

    0x16a0cfe6,// 223 PAY 220 

    0x8243fc51,// 224 PAY 221 

    0x4022fd2b,// 225 PAY 222 

    0x573cedfd,// 226 PAY 223 

    0xcb5ddbd1,// 227 PAY 224 

    0x785b0ed5,// 228 PAY 225 

    0x9dd0ac68,// 229 PAY 226 

    0xe836fbd4,// 230 PAY 227 

    0x4d1a0bdb,// 231 PAY 228 

    0x9f2dcff8,// 232 PAY 229 

    0x8b933536,// 233 PAY 230 

    0x7470cc67,// 234 PAY 231 

    0x62e77802,// 235 PAY 232 

    0xdc5e5832,// 236 PAY 233 

    0x476367aa,// 237 PAY 234 

    0xd802cbc4,// 238 PAY 235 

    0x08bf93f4,// 239 PAY 236 

    0xab9f5bd6,// 240 PAY 237 

    0x2108c7af,// 241 PAY 238 

    0x81a95649,// 242 PAY 239 

    0xff7bdcb1,// 243 PAY 240 

    0xc79c64eb,// 244 PAY 241 

    0xd7c393e6,// 245 PAY 242 

    0x28db3a6a,// 246 PAY 243 

    0x777fad5a,// 247 PAY 244 

    0x995bcf49,// 248 PAY 245 

    0x10021218,// 249 PAY 246 

    0x8a419795,// 250 PAY 247 

    0x04cf6c62,// 251 PAY 248 

    0x150bf2f1,// 252 PAY 249 

    0x4ea52386,// 253 PAY 250 

    0xf876e793,// 254 PAY 251 

    0xe1d5d29a,// 255 PAY 252 

    0xc9d67894,// 256 PAY 253 

    0x07427658,// 257 PAY 254 

    0xa9ea0d90,// 258 PAY 255 

    0xca014eba,// 259 PAY 256 

    0x12b15e2a,// 260 PAY 257 

    0x201f5fec,// 261 PAY 258 

    0xe3134bae,// 262 PAY 259 

    0x72130bf8,// 263 PAY 260 

    0x3cc924b4,// 264 PAY 261 

    0x76265101,// 265 PAY 262 

    0x0cfe5d03,// 266 PAY 263 

    0x651a6255,// 267 PAY 264 

    0x0629de60,// 268 PAY 265 

    0x6b053c20,// 269 PAY 266 

    0x93c2c846,// 270 PAY 267 

    0x3157ee89,// 271 PAY 268 

    0x522d2598,// 272 PAY 269 

    0xb3799ec3,// 273 PAY 270 

    0x980be56c,// 274 PAY 271 

    0x96f9f44a,// 275 PAY 272 

    0x80a92b71,// 276 PAY 273 

    0x5775e5e0,// 277 PAY 274 

    0xcef61f3c,// 278 PAY 275 

    0x4ddf05df,// 279 PAY 276 

    0xd04a4101,// 280 PAY 277 

    0xea59f074,// 281 PAY 278 

    0xcbdddeb6,// 282 PAY 279 

    0xe2714d19,// 283 PAY 280 

    0x8e3efa40,// 284 PAY 281 

    0x7236529d,// 285 PAY 282 

    0x6d74db25,// 286 PAY 283 

    0x2087dcb5,// 287 PAY 284 

    0xcbd87c15,// 288 PAY 285 

    0x30311473,// 289 PAY 286 

    0x49d723ce,// 290 PAY 287 

    0xddcc1723,// 291 PAY 288 

    0x391b5636,// 292 PAY 289 

    0xb2bdee3f,// 293 PAY 290 

    0x2d7fbc9d,// 294 PAY 291 

    0xae8c8fa1,// 295 PAY 292 

    0x8b353883,// 296 PAY 293 

    0x6ee0dc02,// 297 PAY 294 

    0x4b7f16e5,// 298 PAY 295 

    0x2d8f014c,// 299 PAY 296 

    0x609e3e79,// 300 PAY 297 

    0xf1a7405c,// 301 PAY 298 

    0x193b8f5f,// 302 PAY 299 

    0x64bd5ca0,// 303 PAY 300 

    0xe50938e2,// 304 PAY 301 

    0xcfc5b8e5,// 305 PAY 302 

    0xd8959317,// 306 PAY 303 

    0x2ed36dc3,// 307 PAY 304 

    0x7cfa32f3,// 308 PAY 305 

    0x60adc777,// 309 PAY 306 

    0x598eb1a3,// 310 PAY 307 

    0xaf49a61e,// 311 PAY 308 

    0x041193e7,// 312 PAY 309 

    0x11385dd0,// 313 PAY 310 

    0x4d6921b3,// 314 PAY 311 

    0xa0e68ddc,// 315 PAY 312 

    0x799e7c8f,// 316 PAY 313 

    0x2fd75c97,// 317 PAY 314 

    0x344253c6,// 318 PAY 315 

    0x45a9f4ae,// 319 PAY 316 

    0x8d669b85,// 320 PAY 317 

    0xac032eac,// 321 PAY 318 

    0x4482366b,// 322 PAY 319 

    0x6dfd82c6,// 323 PAY 320 

    0xbe39fdd8,// 324 PAY 321 

    0x8c5beff2,// 325 PAY 322 

    0x7ce14dc5,// 326 PAY 323 

    0x7b3b8f59,// 327 PAY 324 

    0x289e9d7e,// 328 PAY 325 

    0x6a5bf21e,// 329 PAY 326 

    0x5fcff832,// 330 PAY 327 

    0xa7af6f3a,// 331 PAY 328 

    0xf6be0584,// 332 PAY 329 

    0x456616a2,// 333 PAY 330 

    0x086744cb,// 334 PAY 331 

    0x7eeab153,// 335 PAY 332 

    0x39e32d65,// 336 PAY 333 

    0xd046cbcd,// 337 PAY 334 

    0xa7fed317,// 338 PAY 335 

    0x48f8101e,// 339 PAY 336 

    0x10bc5bb3,// 340 PAY 337 

    0xd3453575,// 341 PAY 338 

    0xae9e03c1,// 342 PAY 339 

    0x562d155b,// 343 PAY 340 

    0x85ce7503,// 344 PAY 341 

    0x9f2e65a9,// 345 PAY 342 

    0xf42d5c15,// 346 PAY 343 

    0x8b602445,// 347 PAY 344 

    0xa7cd6983,// 348 PAY 345 

    0x22c9988b,// 349 PAY 346 

    0x12765876,// 350 PAY 347 

    0x38784a36,// 351 PAY 348 

    0x89de0dfa,// 352 PAY 349 

    0x889fafe2,// 353 PAY 350 

    0x827188fb,// 354 PAY 351 

    0x3675cb2f,// 355 PAY 352 

    0x57f2af32,// 356 PAY 353 

    0x3cc0d9a1,// 357 PAY 354 

    0xa0d39adf,// 358 PAY 355 

    0x3b5fbbf3,// 359 PAY 356 

    0xd31c4544,// 360 PAY 357 

    0x0b236ed4,// 361 PAY 358 

    0xddeaea33,// 362 PAY 359 

    0x62f0fbff,// 363 PAY 360 

    0x5296a89b,// 364 PAY 361 

    0x9cb418f0,// 365 PAY 362 

    0xc4148163,// 366 PAY 363 

    0x2f105c27,// 367 PAY 364 

    0x660b7c07,// 368 PAY 365 

    0x01643565,// 369 PAY 366 

    0x9c9413d5,// 370 PAY 367 

    0xdc99ac87,// 371 PAY 368 

    0xb582ef6e,// 372 PAY 369 

    0xac083e08,// 373 PAY 370 

    0xed721e13,// 374 PAY 371 

    0xaad280a9,// 375 PAY 372 

    0xa90c1b96,// 376 PAY 373 

    0x36e107b3,// 377 PAY 374 

    0xac74e8cc,// 378 PAY 375 

    0x3c971f62,// 379 PAY 376 

    0x73bb0ff2,// 380 PAY 377 

    0xc368fe1a,// 381 PAY 378 

    0x89aacac6,// 382 PAY 379 

    0x818263ec,// 383 PAY 380 

    0xc770234f,// 384 PAY 381 

    0x8d15136c,// 385 PAY 382 

    0x37b995ff,// 386 PAY 383 

    0xd3f8f764,// 387 PAY 384 

    0x261ade06,// 388 PAY 385 

    0xf8868496,// 389 PAY 386 

    0x71e65046,// 390 PAY 387 

    0x6c1bd15c,// 391 PAY 388 

    0xd74da15c,// 392 PAY 389 

    0x3d4812ba,// 393 PAY 390 

    0x276a0159,// 394 PAY 391 

    0x4f676751,// 395 PAY 392 

    0x065e26da,// 396 PAY 393 

    0xd8260b87,// 397 PAY 394 

    0x89bd941f,// 398 PAY 395 

    0x50f97e4d,// 399 PAY 396 

    0x8d428774,// 400 PAY 397 

    0xbc55ea8a,// 401 PAY 398 

    0x94ccb5b2,// 402 PAY 399 

    0x1e82ad1a,// 403 PAY 400 

    0x26f42cad,// 404 PAY 401 

    0x16bb4f67,// 405 PAY 402 

    0x71bcb186,// 406 PAY 403 

    0x75e59f9a,// 407 PAY 404 

    0xd165d466,// 408 PAY 405 

    0x56b7e8f2,// 409 PAY 406 

    0x46838d6d,// 410 PAY 407 

    0xc17766d1,// 411 PAY 408 

    0xde366c32,// 412 PAY 409 

    0x74d6a101,// 413 PAY 410 

    0x11fc4b16,// 414 PAY 411 

    0xaec89687,// 415 PAY 412 

    0x2ae94e41,// 416 PAY 413 

    0x1144fb49,// 417 PAY 414 

    0xd90fb5a8,// 418 PAY 415 

    0x40374c84,// 419 PAY 416 

    0x023912bc,// 420 PAY 417 

    0xe07566b3,// 421 PAY 418 

    0x5ed3097b,// 422 PAY 419 

    0x5a442524,// 423 PAY 420 

    0x116836ae,// 424 PAY 421 

    0xd2dbedbd,// 425 PAY 422 

    0x4285933c,// 426 PAY 423 

    0x7c1662f0,// 427 PAY 424 

    0x3c8b1b40,// 428 PAY 425 

    0x95545f22,// 429 PAY 426 

    0x8480aa38,// 430 PAY 427 

    0xfd219bbd,// 431 PAY 428 

    0x05bc3d82,// 432 PAY 429 

    0xc5e04016,// 433 PAY 430 

    0x21260e1f,// 434 PAY 431 

    0x728828e6,// 435 PAY 432 

    0x573a5d94,// 436 PAY 433 

    0xd822bf2c,// 437 PAY 434 

    0xd8f3d7ff,// 438 PAY 435 

    0xb086f60f,// 439 PAY 436 

    0x4b452f37,// 440 PAY 437 

    0x47adec00,// 441 PAY 438 

/// HASH is  12 bytes 

    0xd822bf2c,// 442 HSH   1 

    0xd8f3d7ff,// 443 HSH   2 

    0xb086f60f,// 444 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 84 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3d 

    0x02b53d54 // 445 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt35_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 287 words. 

/// BDA size     is 1142 (0x476) 

/// BDA id       is 0x41dc 

    0x047641dc,// 3 BDA   1 

/// PAY Generic Data size   : 1142 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x12d3e180,// 4 PAY   1 

    0x4e27fea8,// 5 PAY   2 

    0x9c2a3f82,// 6 PAY   3 

    0x21665265,// 7 PAY   4 

    0xec1b3d84,// 8 PAY   5 

    0x409e2e81,// 9 PAY   6 

    0xc4924d39,// 10 PAY   7 

    0x6d10b6be,// 11 PAY   8 

    0xc3706033,// 12 PAY   9 

    0x7d6106e7,// 13 PAY  10 

    0x7e13c48f,// 14 PAY  11 

    0x6a86327c,// 15 PAY  12 

    0x672ac703,// 16 PAY  13 

    0xa2fe3488,// 17 PAY  14 

    0xf407aa95,// 18 PAY  15 

    0xc1dbfa38,// 19 PAY  16 

    0xadc0c315,// 20 PAY  17 

    0xa862de1d,// 21 PAY  18 

    0x822b27db,// 22 PAY  19 

    0x92d09a8f,// 23 PAY  20 

    0xb7cdc9c6,// 24 PAY  21 

    0xb95886a4,// 25 PAY  22 

    0x5e226bc8,// 26 PAY  23 

    0x344dc815,// 27 PAY  24 

    0xfd3d44a2,// 28 PAY  25 

    0x5cd7294c,// 29 PAY  26 

    0x91a5d416,// 30 PAY  27 

    0x2bfc53b7,// 31 PAY  28 

    0x195fb51e,// 32 PAY  29 

    0x615fd2df,// 33 PAY  30 

    0x75e63acf,// 34 PAY  31 

    0x60031149,// 35 PAY  32 

    0x39952cd2,// 36 PAY  33 

    0x1b65623f,// 37 PAY  34 

    0xe049387c,// 38 PAY  35 

    0x8b2e10f5,// 39 PAY  36 

    0x6049e618,// 40 PAY  37 

    0x6814296a,// 41 PAY  38 

    0x24ffc449,// 42 PAY  39 

    0x402b73c5,// 43 PAY  40 

    0x5340ed6f,// 44 PAY  41 

    0x0ee34b4d,// 45 PAY  42 

    0x29891d4a,// 46 PAY  43 

    0x05e8d25b,// 47 PAY  44 

    0x03e1693f,// 48 PAY  45 

    0xaf82688e,// 49 PAY  46 

    0xdf7b720a,// 50 PAY  47 

    0xdd7ce800,// 51 PAY  48 

    0xee213ab1,// 52 PAY  49 

    0x9435a69f,// 53 PAY  50 

    0x2859006e,// 54 PAY  51 

    0x247e3b91,// 55 PAY  52 

    0x7cea0222,// 56 PAY  53 

    0x158e89f8,// 57 PAY  54 

    0x2a52e8a0,// 58 PAY  55 

    0x53fd072c,// 59 PAY  56 

    0x16a15d56,// 60 PAY  57 

    0xaf15821e,// 61 PAY  58 

    0x5e319ef7,// 62 PAY  59 

    0xa70bf509,// 63 PAY  60 

    0xac55bcdc,// 64 PAY  61 

    0x2e4719e8,// 65 PAY  62 

    0x39f09d1e,// 66 PAY  63 

    0xf204d219,// 67 PAY  64 

    0x283c3659,// 68 PAY  65 

    0x456232e1,// 69 PAY  66 

    0x7291d62f,// 70 PAY  67 

    0x78c5d141,// 71 PAY  68 

    0xb98066ca,// 72 PAY  69 

    0xb2abd9fa,// 73 PAY  70 

    0xe650da46,// 74 PAY  71 

    0x0d6fae8f,// 75 PAY  72 

    0xb27665e1,// 76 PAY  73 

    0x6c691e42,// 77 PAY  74 

    0xb923d113,// 78 PAY  75 

    0x34d8eeff,// 79 PAY  76 

    0x41d971f1,// 80 PAY  77 

    0x86da9294,// 81 PAY  78 

    0xd02db701,// 82 PAY  79 

    0xc2a5e2b3,// 83 PAY  80 

    0xeb0d0690,// 84 PAY  81 

    0x7c6b8230,// 85 PAY  82 

    0x2d8e3c4f,// 86 PAY  83 

    0xab634c74,// 87 PAY  84 

    0x17d92658,// 88 PAY  85 

    0x50fa97a8,// 89 PAY  86 

    0x4cd94115,// 90 PAY  87 

    0xd20d4a8f,// 91 PAY  88 

    0xd5c403f1,// 92 PAY  89 

    0xd4613fe0,// 93 PAY  90 

    0x087423f8,// 94 PAY  91 

    0x10ee0277,// 95 PAY  92 

    0x2939f281,// 96 PAY  93 

    0x63a0d281,// 97 PAY  94 

    0x652d26ca,// 98 PAY  95 

    0xee564647,// 99 PAY  96 

    0xbe7c7e48,// 100 PAY  97 

    0xd8999ffe,// 101 PAY  98 

    0x818ef1ad,// 102 PAY  99 

    0x54758579,// 103 PAY 100 

    0x2f282e65,// 104 PAY 101 

    0x78aa780d,// 105 PAY 102 

    0xc5aeb73e,// 106 PAY 103 

    0x9be63c2c,// 107 PAY 104 

    0x7ea28529,// 108 PAY 105 

    0x21754c0d,// 109 PAY 106 

    0xe51da996,// 110 PAY 107 

    0xd186ab37,// 111 PAY 108 

    0x2122efd8,// 112 PAY 109 

    0xa7138d5a,// 113 PAY 110 

    0xdb3ced60,// 114 PAY 111 

    0x631de8ab,// 115 PAY 112 

    0x2eb553f2,// 116 PAY 113 

    0x79bd7633,// 117 PAY 114 

    0x504d6db0,// 118 PAY 115 

    0xbe43468c,// 119 PAY 116 

    0xe3a10c20,// 120 PAY 117 

    0x627e413d,// 121 PAY 118 

    0x3dfafce2,// 122 PAY 119 

    0xe5e078b1,// 123 PAY 120 

    0x3086cff4,// 124 PAY 121 

    0x181e5c0b,// 125 PAY 122 

    0xb034153c,// 126 PAY 123 

    0xa351e57f,// 127 PAY 124 

    0x88b19a8d,// 128 PAY 125 

    0xe6052ce4,// 129 PAY 126 

    0xf7eb5bd2,// 130 PAY 127 

    0x0c2eb0d7,// 131 PAY 128 

    0x5b358782,// 132 PAY 129 

    0x70356020,// 133 PAY 130 

    0x0d67189d,// 134 PAY 131 

    0xba77e307,// 135 PAY 132 

    0x3364b670,// 136 PAY 133 

    0xe29b4715,// 137 PAY 134 

    0xd7a164c5,// 138 PAY 135 

    0x62eedb86,// 139 PAY 136 

    0x4df96995,// 140 PAY 137 

    0xf71e28ee,// 141 PAY 138 

    0x0cf7e972,// 142 PAY 139 

    0x0378e741,// 143 PAY 140 

    0xf744cf38,// 144 PAY 141 

    0xd564f15b,// 145 PAY 142 

    0x9367033c,// 146 PAY 143 

    0xa801024b,// 147 PAY 144 

    0xc96d5c7a,// 148 PAY 145 

    0x9c372d55,// 149 PAY 146 

    0xfb4093d4,// 150 PAY 147 

    0x2c3b6208,// 151 PAY 148 

    0x81d9dae2,// 152 PAY 149 

    0x3778083d,// 153 PAY 150 

    0x47faf431,// 154 PAY 151 

    0x17d48cc5,// 155 PAY 152 

    0x97308ed6,// 156 PAY 153 

    0xe7cfdb89,// 157 PAY 154 

    0x18c7c712,// 158 PAY 155 

    0x0b01db2c,// 159 PAY 156 

    0xe2c17c57,// 160 PAY 157 

    0xa491e6b1,// 161 PAY 158 

    0x892cd6fa,// 162 PAY 159 

    0xba168a8b,// 163 PAY 160 

    0x8d07f44b,// 164 PAY 161 

    0xaee747b3,// 165 PAY 162 

    0x37439622,// 166 PAY 163 

    0x831def61,// 167 PAY 164 

    0xd23fe6a6,// 168 PAY 165 

    0x3587bf2e,// 169 PAY 166 

    0xea9f9089,// 170 PAY 167 

    0xd1bdfe66,// 171 PAY 168 

    0xa3efe0d2,// 172 PAY 169 

    0xe7fb9753,// 173 PAY 170 

    0xe0ba4ced,// 174 PAY 171 

    0x21bd5402,// 175 PAY 172 

    0x124b0beb,// 176 PAY 173 

    0xf9b76a41,// 177 PAY 174 

    0x08a406e7,// 178 PAY 175 

    0xa17c8b66,// 179 PAY 176 

    0xb2ad5c6a,// 180 PAY 177 

    0x99344f0d,// 181 PAY 178 

    0xa6272d92,// 182 PAY 179 

    0x7650bdb2,// 183 PAY 180 

    0x3acc497c,// 184 PAY 181 

    0xd366d8c2,// 185 PAY 182 

    0xea066389,// 186 PAY 183 

    0x4e4b234c,// 187 PAY 184 

    0x5d2ea1e6,// 188 PAY 185 

    0x304b897d,// 189 PAY 186 

    0x49bc5e99,// 190 PAY 187 

    0xe37faa8d,// 191 PAY 188 

    0x298dae57,// 192 PAY 189 

    0x9db8dbc9,// 193 PAY 190 

    0x52ff307b,// 194 PAY 191 

    0x1815c92b,// 195 PAY 192 

    0x41715335,// 196 PAY 193 

    0xbf4e1283,// 197 PAY 194 

    0x4bfdbc89,// 198 PAY 195 

    0x80ffe0e7,// 199 PAY 196 

    0x7304958c,// 200 PAY 197 

    0x225d06ea,// 201 PAY 198 

    0x9dbdb1d6,// 202 PAY 199 

    0x9144ee5c,// 203 PAY 200 

    0x30c961df,// 204 PAY 201 

    0xe8443bed,// 205 PAY 202 

    0x94811699,// 206 PAY 203 

    0xde74c2d0,// 207 PAY 204 

    0x48c9dfd8,// 208 PAY 205 

    0x9f6b5843,// 209 PAY 206 

    0xd050e4ba,// 210 PAY 207 

    0xead4f973,// 211 PAY 208 

    0xedb75cc0,// 212 PAY 209 

    0xfe658c61,// 213 PAY 210 

    0xa2da90e5,// 214 PAY 211 

    0xeb4fe64b,// 215 PAY 212 

    0x16a41172,// 216 PAY 213 

    0x537c5d2d,// 217 PAY 214 

    0x25577f96,// 218 PAY 215 

    0x4d2f45de,// 219 PAY 216 

    0x378ea79a,// 220 PAY 217 

    0x659aec35,// 221 PAY 218 

    0x62cd576d,// 222 PAY 219 

    0xa7dadaaf,// 223 PAY 220 

    0xbdd95317,// 224 PAY 221 

    0x6c0036db,// 225 PAY 222 

    0x83461faf,// 226 PAY 223 

    0x8aebd39d,// 227 PAY 224 

    0xb41d758b,// 228 PAY 225 

    0x22feb7b8,// 229 PAY 226 

    0x35cfb5d0,// 230 PAY 227 

    0x2e02a379,// 231 PAY 228 

    0x399ff76c,// 232 PAY 229 

    0xb7f79c05,// 233 PAY 230 

    0x151d00f6,// 234 PAY 231 

    0x1d748789,// 235 PAY 232 

    0xb47fb179,// 236 PAY 233 

    0xc165ee72,// 237 PAY 234 

    0x2c3a19c4,// 238 PAY 235 

    0xc39f36e6,// 239 PAY 236 

    0x283d77b7,// 240 PAY 237 

    0x9a6d6463,// 241 PAY 238 

    0x02d9002b,// 242 PAY 239 

    0x562d07a6,// 243 PAY 240 

    0x0b9cb7cf,// 244 PAY 241 

    0xbae2f6be,// 245 PAY 242 

    0x1e5eb07d,// 246 PAY 243 

    0x34b6a1ac,// 247 PAY 244 

    0x62ac86fd,// 248 PAY 245 

    0xc6b6ed34,// 249 PAY 246 

    0xa50751ff,// 250 PAY 247 

    0xc84142f3,// 251 PAY 248 

    0xa3a25d6a,// 252 PAY 249 

    0xf41280f1,// 253 PAY 250 

    0x23b94d5f,// 254 PAY 251 

    0x16358dc3,// 255 PAY 252 

    0x74cc3a77,// 256 PAY 253 

    0x42988ca4,// 257 PAY 254 

    0x36d3e27f,// 258 PAY 255 

    0x9a8cd1a3,// 259 PAY 256 

    0xe71d26cf,// 260 PAY 257 

    0x5109591d,// 261 PAY 258 

    0x7de66e6e,// 262 PAY 259 

    0xabdf2061,// 263 PAY 260 

    0x7b8999f4,// 264 PAY 261 

    0xa0e716e8,// 265 PAY 262 

    0x73b93e9f,// 266 PAY 263 

    0x68163a4b,// 267 PAY 264 

    0x732b3373,// 268 PAY 265 

    0x314e631c,// 269 PAY 266 

    0x0a704cc3,// 270 PAY 267 

    0xd4b94b3c,// 271 PAY 268 

    0xe58310d1,// 272 PAY 269 

    0xfc0e46b0,// 273 PAY 270 

    0xf5e0c104,// 274 PAY 271 

    0x2e7697a9,// 275 PAY 272 

    0xd63a495b,// 276 PAY 273 

    0xa8fd770f,// 277 PAY 274 

    0x3f780201,// 278 PAY 275 

    0xe0032b81,// 279 PAY 276 

    0x5c869332,// 280 PAY 277 

    0xf453fae4,// 281 PAY 278 

    0x83d6de1d,// 282 PAY 279 

    0x00c1ea8f,// 283 PAY 280 

    0xd1f6fc07,// 284 PAY 281 

    0x0852b756,// 285 PAY 282 

    0xe10e5a04,// 286 PAY 283 

    0x268f8199,// 287 PAY 284 

    0x64c5374a,// 288 PAY 285 

    0x08cf0000,// 289 PAY 286 

/// HASH is  8 bytes 

    0xe0032b81,// 290 HSH   1 

    0x5c869332,// 291 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 7 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xfa 

    0x001cfad6 // 292 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt36_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 506 words. 

/// BDA size     is 2018 (0x7e2) 

/// BDA id       is 0xda48 

    0x07e2da48,// 3 BDA   1 

/// PAY Generic Data size   : 2018 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa99a9fc3,// 4 PAY   1 

    0x2d793b24,// 5 PAY   2 

    0xcc4e0838,// 6 PAY   3 

    0xacfc823c,// 7 PAY   4 

    0xf85c8b99,// 8 PAY   5 

    0xc5c35769,// 9 PAY   6 

    0xfd49b5c9,// 10 PAY   7 

    0xa15161ed,// 11 PAY   8 

    0x60056d3d,// 12 PAY   9 

    0x2378f733,// 13 PAY  10 

    0x4b542ee2,// 14 PAY  11 

    0x26f81d2a,// 15 PAY  12 

    0x95778f69,// 16 PAY  13 

    0x1ace4d7d,// 17 PAY  14 

    0x780527be,// 18 PAY  15 

    0x79f979c4,// 19 PAY  16 

    0xc2c52ec6,// 20 PAY  17 

    0xe8b61cde,// 21 PAY  18 

    0x37f5daaf,// 22 PAY  19 

    0xc9efd3c5,// 23 PAY  20 

    0xdf0b4a6a,// 24 PAY  21 

    0x888dcd57,// 25 PAY  22 

    0x314a70b9,// 26 PAY  23 

    0x59f31e4b,// 27 PAY  24 

    0xad0cd829,// 28 PAY  25 

    0xb02a8a7a,// 29 PAY  26 

    0xc08633b1,// 30 PAY  27 

    0xd71bb318,// 31 PAY  28 

    0x47b7abf7,// 32 PAY  29 

    0xaee9cfe7,// 33 PAY  30 

    0x0581f751,// 34 PAY  31 

    0xd818bf56,// 35 PAY  32 

    0xf80f1026,// 36 PAY  33 

    0xeece9120,// 37 PAY  34 

    0xb82882db,// 38 PAY  35 

    0xf58f62c9,// 39 PAY  36 

    0x9140e1af,// 40 PAY  37 

    0x8612cd52,// 41 PAY  38 

    0x88e3db43,// 42 PAY  39 

    0x2e756e30,// 43 PAY  40 

    0x69ccc25f,// 44 PAY  41 

    0x65bc1f8e,// 45 PAY  42 

    0xc92b0462,// 46 PAY  43 

    0x6274fbba,// 47 PAY  44 

    0x5516ed4c,// 48 PAY  45 

    0x942479c1,// 49 PAY  46 

    0x80242847,// 50 PAY  47 

    0x1987d9a5,// 51 PAY  48 

    0x91a1051e,// 52 PAY  49 

    0x60b2802c,// 53 PAY  50 

    0xc445625d,// 54 PAY  51 

    0x9df1a89c,// 55 PAY  52 

    0xcfae6e2e,// 56 PAY  53 

    0x20963696,// 57 PAY  54 

    0xccb8f6e2,// 58 PAY  55 

    0x81d8696c,// 59 PAY  56 

    0x6ed1170e,// 60 PAY  57 

    0x6edc168c,// 61 PAY  58 

    0x2ebd92d5,// 62 PAY  59 

    0x1f5818c4,// 63 PAY  60 

    0x452bc949,// 64 PAY  61 

    0xc671e06a,// 65 PAY  62 

    0xba6f1b12,// 66 PAY  63 

    0xefe8d96f,// 67 PAY  64 

    0x5ce336f5,// 68 PAY  65 

    0x27c7a565,// 69 PAY  66 

    0x12b0f558,// 70 PAY  67 

    0xfaf01d21,// 71 PAY  68 

    0x2ce8136a,// 72 PAY  69 

    0x18f85541,// 73 PAY  70 

    0x2ab56dba,// 74 PAY  71 

    0x33e98c0e,// 75 PAY  72 

    0x5828696c,// 76 PAY  73 

    0xda8c7a30,// 77 PAY  74 

    0x8565add2,// 78 PAY  75 

    0x29816c1c,// 79 PAY  76 

    0xdecb925e,// 80 PAY  77 

    0x3f81c21d,// 81 PAY  78 

    0x340b656d,// 82 PAY  79 

    0xee13c9d8,// 83 PAY  80 

    0x015b72a4,// 84 PAY  81 

    0x75368370,// 85 PAY  82 

    0x06bd1442,// 86 PAY  83 

    0x04ad5142,// 87 PAY  84 

    0xef211270,// 88 PAY  85 

    0x5207aa28,// 89 PAY  86 

    0x37ed3e1d,// 90 PAY  87 

    0x4b5cf2b9,// 91 PAY  88 

    0x870b2af2,// 92 PAY  89 

    0x4a3c9a55,// 93 PAY  90 

    0x9acd9659,// 94 PAY  91 

    0xdf7c9829,// 95 PAY  92 

    0xc20c84fa,// 96 PAY  93 

    0x34a94eb8,// 97 PAY  94 

    0x8b19d8b2,// 98 PAY  95 

    0x59b454c8,// 99 PAY  96 

    0x9cc82386,// 100 PAY  97 

    0xf637c6b2,// 101 PAY  98 

    0xa4fb8512,// 102 PAY  99 

    0x12e9e44c,// 103 PAY 100 

    0xcaf02f19,// 104 PAY 101 

    0x8a8754b0,// 105 PAY 102 

    0x302c32c5,// 106 PAY 103 

    0x2d7e9efe,// 107 PAY 104 

    0x868d5c01,// 108 PAY 105 

    0xc6031c90,// 109 PAY 106 

    0x2749fed5,// 110 PAY 107 

    0xa404911c,// 111 PAY 108 

    0x2596d519,// 112 PAY 109 

    0xf9c37ba7,// 113 PAY 110 

    0x492430f1,// 114 PAY 111 

    0xe85075aa,// 115 PAY 112 

    0x4df9d25a,// 116 PAY 113 

    0xe7dbb4d2,// 117 PAY 114 

    0x2dc33677,// 118 PAY 115 

    0xd0778dc7,// 119 PAY 116 

    0x7e7ba748,// 120 PAY 117 

    0x66888604,// 121 PAY 118 

    0x44beba7c,// 122 PAY 119 

    0x1566b695,// 123 PAY 120 

    0x72c369c8,// 124 PAY 121 

    0x10c267d7,// 125 PAY 122 

    0x18adea9c,// 126 PAY 123 

    0x88420930,// 127 PAY 124 

    0x5bdc912c,// 128 PAY 125 

    0x0c8663da,// 129 PAY 126 

    0x02427f8b,// 130 PAY 127 

    0xa8ae4598,// 131 PAY 128 

    0x9fd77510,// 132 PAY 129 

    0xdc33d748,// 133 PAY 130 

    0x0b7cd491,// 134 PAY 131 

    0xddbd3201,// 135 PAY 132 

    0x349f76d1,// 136 PAY 133 

    0xa4bf0f54,// 137 PAY 134 

    0x86450c88,// 138 PAY 135 

    0x331135ee,// 139 PAY 136 

    0x092f0aaa,// 140 PAY 137 

    0xc08b19f3,// 141 PAY 138 

    0xe4ea32c3,// 142 PAY 139 

    0xdf0597ac,// 143 PAY 140 

    0x9332fa23,// 144 PAY 141 

    0x9f682c16,// 145 PAY 142 

    0x12e6bd2a,// 146 PAY 143 

    0x83eeeb7f,// 147 PAY 144 

    0x013d6cee,// 148 PAY 145 

    0xed077720,// 149 PAY 146 

    0x286fd9d1,// 150 PAY 147 

    0x373d3e18,// 151 PAY 148 

    0x05507c46,// 152 PAY 149 

    0xdee1eb01,// 153 PAY 150 

    0xe524699e,// 154 PAY 151 

    0xb9ed988a,// 155 PAY 152 

    0xecffe644,// 156 PAY 153 

    0xba382615,// 157 PAY 154 

    0x674a20b8,// 158 PAY 155 

    0x34b7e0ba,// 159 PAY 156 

    0x4a4238ac,// 160 PAY 157 

    0x80f7aad4,// 161 PAY 158 

    0x33ec42f2,// 162 PAY 159 

    0x246953fa,// 163 PAY 160 

    0x42dc9453,// 164 PAY 161 

    0xbd4593ba,// 165 PAY 162 

    0x2eafb682,// 166 PAY 163 

    0x994f83b0,// 167 PAY 164 

    0x68fca5b4,// 168 PAY 165 

    0x1e51dd88,// 169 PAY 166 

    0x7d158fff,// 170 PAY 167 

    0x3e3c66b4,// 171 PAY 168 

    0xb4e89601,// 172 PAY 169 

    0xe50a873a,// 173 PAY 170 

    0x56b51668,// 174 PAY 171 

    0x82a949b9,// 175 PAY 172 

    0xcf85cf6e,// 176 PAY 173 

    0x8483a78c,// 177 PAY 174 

    0xf77005ce,// 178 PAY 175 

    0x89453741,// 179 PAY 176 

    0x35a6212d,// 180 PAY 177 

    0x4188ae53,// 181 PAY 178 

    0x53109139,// 182 PAY 179 

    0x688b60de,// 183 PAY 180 

    0xb26d5207,// 184 PAY 181 

    0xd979cbe0,// 185 PAY 182 

    0x877ffa18,// 186 PAY 183 

    0x289aa95d,// 187 PAY 184 

    0xaa489736,// 188 PAY 185 

    0x457bd882,// 189 PAY 186 

    0x5f9e83ee,// 190 PAY 187 

    0xc0fbfe3f,// 191 PAY 188 

    0x5614cc74,// 192 PAY 189 

    0xc1954565,// 193 PAY 190 

    0x2dc693a9,// 194 PAY 191 

    0xf9057d40,// 195 PAY 192 

    0xdf5bda80,// 196 PAY 193 

    0x8b232273,// 197 PAY 194 

    0xa7e088d8,// 198 PAY 195 

    0x22b92393,// 199 PAY 196 

    0x90b118fe,// 200 PAY 197 

    0xfc8113de,// 201 PAY 198 

    0x9a557a81,// 202 PAY 199 

    0xd4591149,// 203 PAY 200 

    0xd76b35a1,// 204 PAY 201 

    0x14ffb0f3,// 205 PAY 202 

    0x8ded7388,// 206 PAY 203 

    0x43df54c5,// 207 PAY 204 

    0xa28a8787,// 208 PAY 205 

    0x5e191b76,// 209 PAY 206 

    0xb5e462b3,// 210 PAY 207 

    0xe66ddd28,// 211 PAY 208 

    0xae2489a1,// 212 PAY 209 

    0xa7366bd5,// 213 PAY 210 

    0xd0ab396e,// 214 PAY 211 

    0x83c12459,// 215 PAY 212 

    0x93514a95,// 216 PAY 213 

    0x53f441c4,// 217 PAY 214 

    0x4bbbab34,// 218 PAY 215 

    0x42c145c1,// 219 PAY 216 

    0xeb529e17,// 220 PAY 217 

    0x73ad364b,// 221 PAY 218 

    0xa4287dd9,// 222 PAY 219 

    0xff0b1a86,// 223 PAY 220 

    0xd6edc392,// 224 PAY 221 

    0xc523fa07,// 225 PAY 222 

    0x02c7d024,// 226 PAY 223 

    0xa2e98ab6,// 227 PAY 224 

    0x3404c55e,// 228 PAY 225 

    0xed8c3379,// 229 PAY 226 

    0x043bf689,// 230 PAY 227 

    0x8a2d9ed2,// 231 PAY 228 

    0xab52c933,// 232 PAY 229 

    0x3576fef7,// 233 PAY 230 

    0x3de63b26,// 234 PAY 231 

    0xc8907638,// 235 PAY 232 

    0x4d47370a,// 236 PAY 233 

    0xea8c6ef3,// 237 PAY 234 

    0xdd237b07,// 238 PAY 235 

    0xcc9e318d,// 239 PAY 236 

    0xd2e00c73,// 240 PAY 237 

    0x6d7d18ba,// 241 PAY 238 

    0x78ef0c89,// 242 PAY 239 

    0xe6a93253,// 243 PAY 240 

    0x742dc0d7,// 244 PAY 241 

    0x163e9cd9,// 245 PAY 242 

    0x85138daa,// 246 PAY 243 

    0x11c5a6c9,// 247 PAY 244 

    0x1ff947ba,// 248 PAY 245 

    0xe2cb008f,// 249 PAY 246 

    0x9ac3cbd3,// 250 PAY 247 

    0xcf96ff4e,// 251 PAY 248 

    0x5cde7d9e,// 252 PAY 249 

    0xa041242f,// 253 PAY 250 

    0xce91deb6,// 254 PAY 251 

    0xf7a075df,// 255 PAY 252 

    0x8bfc4dc5,// 256 PAY 253 

    0x09743f8b,// 257 PAY 254 

    0x9ca1f7af,// 258 PAY 255 

    0x07c95f32,// 259 PAY 256 

    0x941a742c,// 260 PAY 257 

    0x52f2c505,// 261 PAY 258 

    0x8c717513,// 262 PAY 259 

    0x7d114fd5,// 263 PAY 260 

    0x04193995,// 264 PAY 261 

    0x1bb58c9c,// 265 PAY 262 

    0xcfbeb79e,// 266 PAY 263 

    0xa3a777c7,// 267 PAY 264 

    0x37d1d3c5,// 268 PAY 265 

    0xf5ac4b3a,// 269 PAY 266 

    0x7fc9549e,// 270 PAY 267 

    0xc0028a41,// 271 PAY 268 

    0x318caebd,// 272 PAY 269 

    0xf33194de,// 273 PAY 270 

    0xf8070721,// 274 PAY 271 

    0xba9e9fb9,// 275 PAY 272 

    0x340e1c88,// 276 PAY 273 

    0xeefb0350,// 277 PAY 274 

    0x6de9555d,// 278 PAY 275 

    0x99dff289,// 279 PAY 276 

    0x5a65e188,// 280 PAY 277 

    0x81624a9d,// 281 PAY 278 

    0xd23d4dd0,// 282 PAY 279 

    0x7f27a065,// 283 PAY 280 

    0x3eb63a8f,// 284 PAY 281 

    0x5e47e6ff,// 285 PAY 282 

    0x8b6575ae,// 286 PAY 283 

    0x439428a8,// 287 PAY 284 

    0xdc3f413c,// 288 PAY 285 

    0xb02aa103,// 289 PAY 286 

    0xa87d40ea,// 290 PAY 287 

    0x7f665465,// 291 PAY 288 

    0xe4922c2c,// 292 PAY 289 

    0x009cdd3a,// 293 PAY 290 

    0x3c6d14bd,// 294 PAY 291 

    0x49bf5e78,// 295 PAY 292 

    0xcc53e25f,// 296 PAY 293 

    0xe4d9c1cf,// 297 PAY 294 

    0xef256d9f,// 298 PAY 295 

    0x981af0b0,// 299 PAY 296 

    0xc39104be,// 300 PAY 297 

    0xa3c705db,// 301 PAY 298 

    0x4639a7d1,// 302 PAY 299 

    0x8ea8355f,// 303 PAY 300 

    0x642f54bc,// 304 PAY 301 

    0x070c33ab,// 305 PAY 302 

    0xeac31928,// 306 PAY 303 

    0x742c1f25,// 307 PAY 304 

    0x0075466a,// 308 PAY 305 

    0xef8421cc,// 309 PAY 306 

    0x5837074b,// 310 PAY 307 

    0x8fd347a6,// 311 PAY 308 

    0xf847658e,// 312 PAY 309 

    0x24ff00b7,// 313 PAY 310 

    0x3a024f73,// 314 PAY 311 

    0x39dac282,// 315 PAY 312 

    0x454fb5b1,// 316 PAY 313 

    0xd5a7a97b,// 317 PAY 314 

    0xdb488102,// 318 PAY 315 

    0x2b257695,// 319 PAY 316 

    0x094dbb3f,// 320 PAY 317 

    0x111111de,// 321 PAY 318 

    0xb69bb39f,// 322 PAY 319 

    0x2c200f0b,// 323 PAY 320 

    0xc34b42e1,// 324 PAY 321 

    0xdb707342,// 325 PAY 322 

    0x202fe9e2,// 326 PAY 323 

    0xe6239ba8,// 327 PAY 324 

    0x48980e12,// 328 PAY 325 

    0xdff5b057,// 329 PAY 326 

    0xac345ced,// 330 PAY 327 

    0xd0c5d626,// 331 PAY 328 

    0xa9abb0ec,// 332 PAY 329 

    0x7aaffd01,// 333 PAY 330 

    0x78ff7fed,// 334 PAY 331 

    0x9abe8881,// 335 PAY 332 

    0xf4e2125d,// 336 PAY 333 

    0xd891f4de,// 337 PAY 334 

    0xd56a8174,// 338 PAY 335 

    0x0df4bb4d,// 339 PAY 336 

    0xb4d6a385,// 340 PAY 337 

    0xfed43e67,// 341 PAY 338 

    0x58587096,// 342 PAY 339 

    0x4f82db3f,// 343 PAY 340 

    0xb801c09c,// 344 PAY 341 

    0xc0b9b0aa,// 345 PAY 342 

    0xfa822306,// 346 PAY 343 

    0xa08906e2,// 347 PAY 344 

    0x8c8325df,// 348 PAY 345 

    0x08eeb756,// 349 PAY 346 

    0x7483e803,// 350 PAY 347 

    0x0736acef,// 351 PAY 348 

    0xcc2135bb,// 352 PAY 349 

    0x7d3f31ff,// 353 PAY 350 

    0xb37f9211,// 354 PAY 351 

    0xf1c88eeb,// 355 PAY 352 

    0xe800f012,// 356 PAY 353 

    0xb291de34,// 357 PAY 354 

    0x686734d7,// 358 PAY 355 

    0x83b8cc25,// 359 PAY 356 

    0x98f95149,// 360 PAY 357 

    0x2d775dee,// 361 PAY 358 

    0x3cde4ba5,// 362 PAY 359 

    0x9959d9fe,// 363 PAY 360 

    0xc1b711e6,// 364 PAY 361 

    0x2e02d654,// 365 PAY 362 

    0x01be395d,// 366 PAY 363 

    0xebbbf3c2,// 367 PAY 364 

    0x8506a3f5,// 368 PAY 365 

    0x3cfdd87e,// 369 PAY 366 

    0x510891a1,// 370 PAY 367 

    0x04d29623,// 371 PAY 368 

    0x97ec115f,// 372 PAY 369 

    0xd3649ae5,// 373 PAY 370 

    0x4fb7a429,// 374 PAY 371 

    0x28f4d97a,// 375 PAY 372 

    0x09a43eb5,// 376 PAY 373 

    0xac609d6c,// 377 PAY 374 

    0x31e37f16,// 378 PAY 375 

    0x1f86829d,// 379 PAY 376 

    0x0cc21463,// 380 PAY 377 

    0xe50d48a5,// 381 PAY 378 

    0x2503d36e,// 382 PAY 379 

    0x313d7d5b,// 383 PAY 380 

    0x485e1ae1,// 384 PAY 381 

    0xf560ab64,// 385 PAY 382 

    0xcedd1ca2,// 386 PAY 383 

    0x2df9702a,// 387 PAY 384 

    0x90c8f06d,// 388 PAY 385 

    0xc312828d,// 389 PAY 386 

    0x1597ec15,// 390 PAY 387 

    0x212013ba,// 391 PAY 388 

    0x4eb2dea2,// 392 PAY 389 

    0x707674e0,// 393 PAY 390 

    0xce3b5cf1,// 394 PAY 391 

    0x09400d9c,// 395 PAY 392 

    0xf96d329d,// 396 PAY 393 

    0xbd1dc68b,// 397 PAY 394 

    0x692d9f99,// 398 PAY 395 

    0x6da97b0e,// 399 PAY 396 

    0xfda48aca,// 400 PAY 397 

    0xe8a059a2,// 401 PAY 398 

    0x7f3d9221,// 402 PAY 399 

    0x92016f3d,// 403 PAY 400 

    0x20ef65a9,// 404 PAY 401 

    0xe7faf5fc,// 405 PAY 402 

    0x617cb335,// 406 PAY 403 

    0x0b9baa6b,// 407 PAY 404 

    0x78107619,// 408 PAY 405 

    0x27911ee8,// 409 PAY 406 

    0x51f8f823,// 410 PAY 407 

    0xa98e5aee,// 411 PAY 408 

    0x06121f9d,// 412 PAY 409 

    0x45e01e5c,// 413 PAY 410 

    0x3a7cac98,// 414 PAY 411 

    0x24cc67c8,// 415 PAY 412 

    0x7c53579d,// 416 PAY 413 

    0xb729f9a7,// 417 PAY 414 

    0xc7650f23,// 418 PAY 415 

    0x64d5a972,// 419 PAY 416 

    0xad3e53c6,// 420 PAY 417 

    0x6184caea,// 421 PAY 418 

    0x6ae7912b,// 422 PAY 419 

    0x88423ceb,// 423 PAY 420 

    0x969def02,// 424 PAY 421 

    0x810c71bd,// 425 PAY 422 

    0xfd7e6d8b,// 426 PAY 423 

    0x6e33e05e,// 427 PAY 424 

    0xd2a5827a,// 428 PAY 425 

    0xb8943cb7,// 429 PAY 426 

    0xd623c823,// 430 PAY 427 

    0x81f7aeaa,// 431 PAY 428 

    0x006fcb2a,// 432 PAY 429 

    0xca73b8d1,// 433 PAY 430 

    0x45258ad8,// 434 PAY 431 

    0x5d0242c4,// 435 PAY 432 

    0xd9180b63,// 436 PAY 433 

    0xe2ac5c9f,// 437 PAY 434 

    0x74f83b9a,// 438 PAY 435 

    0xcbd52057,// 439 PAY 436 

    0x77cb2dfc,// 440 PAY 437 

    0x0a1dabde,// 441 PAY 438 

    0x0330328a,// 442 PAY 439 

    0xb928330d,// 443 PAY 440 

    0x048f83f9,// 444 PAY 441 

    0xa90fc135,// 445 PAY 442 

    0x01139ee0,// 446 PAY 443 

    0x0463bede,// 447 PAY 444 

    0x3308d0a2,// 448 PAY 445 

    0x00cb7bb7,// 449 PAY 446 

    0xc2c1d325,// 450 PAY 447 

    0x70a56cdf,// 451 PAY 448 

    0x35a26485,// 452 PAY 449 

    0x3ed70dd2,// 453 PAY 450 

    0xa8830d11,// 454 PAY 451 

    0x60fc214c,// 455 PAY 452 

    0xe74a7ac2,// 456 PAY 453 

    0xba26baff,// 457 PAY 454 

    0x9629dcb9,// 458 PAY 455 

    0xeb943f2b,// 459 PAY 456 

    0x719f3e71,// 460 PAY 457 

    0x063df3fe,// 461 PAY 458 

    0xdc6920ad,// 462 PAY 459 

    0xbd096e5d,// 463 PAY 460 

    0x1837bda2,// 464 PAY 461 

    0x4d5c0771,// 465 PAY 462 

    0x450c0c15,// 466 PAY 463 

    0xad50db61,// 467 PAY 464 

    0xe30b1739,// 468 PAY 465 

    0x2aae2ab0,// 469 PAY 466 

    0x080cd3c0,// 470 PAY 467 

    0x50ccd98c,// 471 PAY 468 

    0xf59bcfc9,// 472 PAY 469 

    0xa6a29821,// 473 PAY 470 

    0xf72e28e3,// 474 PAY 471 

    0x12411e9b,// 475 PAY 472 

    0x67f63079,// 476 PAY 473 

    0x6ff2ee0c,// 477 PAY 474 

    0x338a9c54,// 478 PAY 475 

    0x2b9675c2,// 479 PAY 476 

    0xf449cf15,// 480 PAY 477 

    0xc8fd608f,// 481 PAY 478 

    0xc6799468,// 482 PAY 479 

    0x900ddcfb,// 483 PAY 480 

    0xd5be6c18,// 484 PAY 481 

    0x67069072,// 485 PAY 482 

    0xcdb8902a,// 486 PAY 483 

    0x7a9a32de,// 487 PAY 484 

    0x6dc0204d,// 488 PAY 485 

    0x2cb331a6,// 489 PAY 486 

    0x111a02cb,// 490 PAY 487 

    0xa000773c,// 491 PAY 488 

    0xea937fb6,// 492 PAY 489 

    0xdbffe4e1,// 493 PAY 490 

    0x85d49d20,// 494 PAY 491 

    0x3b7be657,// 495 PAY 492 

    0xc800a305,// 496 PAY 493 

    0x5343717f,// 497 PAY 494 

    0xbdebf321,// 498 PAY 495 

    0x761c3464,// 499 PAY 496 

    0x0b2b9626,// 500 PAY 497 

    0x3ffa9ddd,// 501 PAY 498 

    0x91ca9822,// 502 PAY 499 

    0x762e0cb6,// 503 PAY 500 

    0x965bfdc3,// 504 PAY 501 

    0x4d63d4a5,// 505 PAY 502 

    0x0227d28d,// 506 PAY 503 

    0x91f0ce97,// 507 PAY 504 

    0x58ed0000,// 508 PAY 505 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x32 

    0x03f932b2 // 509 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt37_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 500 words. 

/// BDA size     is 1994 (0x7ca) 

/// BDA id       is 0x1608 

    0x07ca1608,// 3 BDA   1 

/// PAY Generic Data size   : 1994 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa9049b54,// 4 PAY   1 

    0xf10eb311,// 5 PAY   2 

    0x258a24f4,// 6 PAY   3 

    0x01aca27a,// 7 PAY   4 

    0x91864d1a,// 8 PAY   5 

    0xba83aee2,// 9 PAY   6 

    0x6fc71baf,// 10 PAY   7 

    0x42e99a96,// 11 PAY   8 

    0x4726736a,// 12 PAY   9 

    0xbb2e5108,// 13 PAY  10 

    0xda13aaac,// 14 PAY  11 

    0x6fce0a3e,// 15 PAY  12 

    0x2886eb68,// 16 PAY  13 

    0x82da17f7,// 17 PAY  14 

    0x48beb3cd,// 18 PAY  15 

    0x5eea6033,// 19 PAY  16 

    0x365521f5,// 20 PAY  17 

    0x015dd40a,// 21 PAY  18 

    0x0eea7dc6,// 22 PAY  19 

    0x7fa3949d,// 23 PAY  20 

    0xde551d52,// 24 PAY  21 

    0x739c5ca4,// 25 PAY  22 

    0x7df26ef2,// 26 PAY  23 

    0x74ffa896,// 27 PAY  24 

    0x4331d89a,// 28 PAY  25 

    0x7dbc09a0,// 29 PAY  26 

    0xc28fa981,// 30 PAY  27 

    0x42e8f73d,// 31 PAY  28 

    0x43cb69b4,// 32 PAY  29 

    0xc0b7421b,// 33 PAY  30 

    0x2eb5fe59,// 34 PAY  31 

    0x56590717,// 35 PAY  32 

    0x0a7ef424,// 36 PAY  33 

    0xe01dfc8d,// 37 PAY  34 

    0x13d4e9f7,// 38 PAY  35 

    0xc7394764,// 39 PAY  36 

    0xed7c8f6a,// 40 PAY  37 

    0x0996a4cd,// 41 PAY  38 

    0x11f3b0f0,// 42 PAY  39 

    0x2a6720ed,// 43 PAY  40 

    0x6f3ce026,// 44 PAY  41 

    0xbeaca972,// 45 PAY  42 

    0xaa3bce02,// 46 PAY  43 

    0x00c54c9c,// 47 PAY  44 

    0xd7b39983,// 48 PAY  45 

    0xfeafb6f4,// 49 PAY  46 

    0xb7224df8,// 50 PAY  47 

    0xb2a01604,// 51 PAY  48 

    0x1abae8eb,// 52 PAY  49 

    0x8e6f4512,// 53 PAY  50 

    0xc576693b,// 54 PAY  51 

    0x028468a2,// 55 PAY  52 

    0x00733a41,// 56 PAY  53 

    0xe612a0b4,// 57 PAY  54 

    0x592bfa14,// 58 PAY  55 

    0x3f312c6a,// 59 PAY  56 

    0xb0819426,// 60 PAY  57 

    0x01551127,// 61 PAY  58 

    0xd88252b2,// 62 PAY  59 

    0xa89c2afd,// 63 PAY  60 

    0xf327ae9e,// 64 PAY  61 

    0x7e65454d,// 65 PAY  62 

    0x4c46849e,// 66 PAY  63 

    0x18a46198,// 67 PAY  64 

    0x81b771b7,// 68 PAY  65 

    0x5916668b,// 69 PAY  66 

    0x94794211,// 70 PAY  67 

    0x494af2f6,// 71 PAY  68 

    0xdded6dc6,// 72 PAY  69 

    0x8ddaad4a,// 73 PAY  70 

    0x8ea71550,// 74 PAY  71 

    0x301331bd,// 75 PAY  72 

    0x5431392f,// 76 PAY  73 

    0x80b2e19a,// 77 PAY  74 

    0xace18dee,// 78 PAY  75 

    0xf55d9475,// 79 PAY  76 

    0xd1e7b5c7,// 80 PAY  77 

    0xd0d7a0f0,// 81 PAY  78 

    0xc5672787,// 82 PAY  79 

    0x27aebde2,// 83 PAY  80 

    0xf224ee94,// 84 PAY  81 

    0x3ee8b048,// 85 PAY  82 

    0x42651852,// 86 PAY  83 

    0x5edd75bb,// 87 PAY  84 

    0x1622b050,// 88 PAY  85 

    0xfb4563c6,// 89 PAY  86 

    0x370eaa38,// 90 PAY  87 

    0xdb7c2fc9,// 91 PAY  88 

    0xa5479248,// 92 PAY  89 

    0xb244ebd4,// 93 PAY  90 

    0xfb9f183e,// 94 PAY  91 

    0x45bb07d7,// 95 PAY  92 

    0xbb457979,// 96 PAY  93 

    0xb75839fd,// 97 PAY  94 

    0x179a7327,// 98 PAY  95 

    0x498d22a9,// 99 PAY  96 

    0x11afc316,// 100 PAY  97 

    0x64f5fe3f,// 101 PAY  98 

    0x7357e44e,// 102 PAY  99 

    0xc227d28d,// 103 PAY 100 

    0x879d98f7,// 104 PAY 101 

    0x3d029a42,// 105 PAY 102 

    0x5c04eaff,// 106 PAY 103 

    0x9280271b,// 107 PAY 104 

    0x41cfa267,// 108 PAY 105 

    0x88380466,// 109 PAY 106 

    0xc22e2638,// 110 PAY 107 

    0x4616b4ab,// 111 PAY 108 

    0xecdd5af7,// 112 PAY 109 

    0xb03e9e7b,// 113 PAY 110 

    0xf6ba4094,// 114 PAY 111 

    0x7092eafa,// 115 PAY 112 

    0x63d79767,// 116 PAY 113 

    0xe657bff9,// 117 PAY 114 

    0x95415bad,// 118 PAY 115 

    0x44930999,// 119 PAY 116 

    0x60c8e62a,// 120 PAY 117 

    0xf1fb2ed1,// 121 PAY 118 

    0xcbc33e52,// 122 PAY 119 

    0x5a2109f6,// 123 PAY 120 

    0xb6290f48,// 124 PAY 121 

    0x086218ad,// 125 PAY 122 

    0xcc27e0b7,// 126 PAY 123 

    0xa7edee42,// 127 PAY 124 

    0x15b7de73,// 128 PAY 125 

    0x78ba9c5c,// 129 PAY 126 

    0x989dcb2f,// 130 PAY 127 

    0x104c5d7f,// 131 PAY 128 

    0xa132e5c0,// 132 PAY 129 

    0xba0188ce,// 133 PAY 130 

    0x3398ce07,// 134 PAY 131 

    0x83c63447,// 135 PAY 132 

    0x932cf248,// 136 PAY 133 

    0x2f550b7b,// 137 PAY 134 

    0x47780c7f,// 138 PAY 135 

    0x51e1f472,// 139 PAY 136 

    0x9b2633d1,// 140 PAY 137 

    0x87b5d4f5,// 141 PAY 138 

    0x7a189d74,// 142 PAY 139 

    0x5d27403d,// 143 PAY 140 

    0xed46234f,// 144 PAY 141 

    0xeb9fcb4a,// 145 PAY 142 

    0xd93ae09a,// 146 PAY 143 

    0x159b02c8,// 147 PAY 144 

    0x9cf3f7b9,// 148 PAY 145 

    0xc6fef43b,// 149 PAY 146 

    0x613f3f5b,// 150 PAY 147 

    0x772a5491,// 151 PAY 148 

    0x29787519,// 152 PAY 149 

    0x11df8459,// 153 PAY 150 

    0xc442d45e,// 154 PAY 151 

    0x47d64907,// 155 PAY 152 

    0x5384a819,// 156 PAY 153 

    0x234baaf8,// 157 PAY 154 

    0x3613f292,// 158 PAY 155 

    0xf3451e7f,// 159 PAY 156 

    0x79d20999,// 160 PAY 157 

    0x29350e92,// 161 PAY 158 

    0x66537e1b,// 162 PAY 159 

    0xc332378e,// 163 PAY 160 

    0x1d01dac9,// 164 PAY 161 

    0x72ce7e44,// 165 PAY 162 

    0x9d4d3dee,// 166 PAY 163 

    0x7d302367,// 167 PAY 164 

    0x3e5e6674,// 168 PAY 165 

    0xb01187d7,// 169 PAY 166 

    0x128cf07e,// 170 PAY 167 

    0x652ee64a,// 171 PAY 168 

    0x83a48ed5,// 172 PAY 169 

    0xfa584cf1,// 173 PAY 170 

    0x7c3e5a43,// 174 PAY 171 

    0x6f03bead,// 175 PAY 172 

    0xd24fb747,// 176 PAY 173 

    0x03cc0891,// 177 PAY 174 

    0x684bedf1,// 178 PAY 175 

    0xbb333a7e,// 179 PAY 176 

    0xac0f63ff,// 180 PAY 177 

    0x0e0abe9e,// 181 PAY 178 

    0xec80af7b,// 182 PAY 179 

    0x8d955525,// 183 PAY 180 

    0x7e24cc89,// 184 PAY 181 

    0xafb42c10,// 185 PAY 182 

    0x2dde2a41,// 186 PAY 183 

    0x14e3324b,// 187 PAY 184 

    0x5766242f,// 188 PAY 185 

    0xb7c0506a,// 189 PAY 186 

    0xf28db95b,// 190 PAY 187 

    0x4f0c3b0b,// 191 PAY 188 

    0x81718838,// 192 PAY 189 

    0xd0d8df57,// 193 PAY 190 

    0x3560b6cb,// 194 PAY 191 

    0x422060a0,// 195 PAY 192 

    0xe8bcc28a,// 196 PAY 193 

    0x99f6ad33,// 197 PAY 194 

    0xd997d9c4,// 198 PAY 195 

    0x81042c66,// 199 PAY 196 

    0x98e85096,// 200 PAY 197 

    0x02060793,// 201 PAY 198 

    0x96a620e7,// 202 PAY 199 

    0x90de9b56,// 203 PAY 200 

    0x37b63259,// 204 PAY 201 

    0x83082c1e,// 205 PAY 202 

    0xb62e30e5,// 206 PAY 203 

    0xf1c179f7,// 207 PAY 204 

    0xaf0a39ae,// 208 PAY 205 

    0x181fb87e,// 209 PAY 206 

    0x59d28387,// 210 PAY 207 

    0xf1ce621a,// 211 PAY 208 

    0x74cd1635,// 212 PAY 209 

    0x5f9cf5d1,// 213 PAY 210 

    0xbe7b8f33,// 214 PAY 211 

    0x20fe8ec2,// 215 PAY 212 

    0x7245d9d8,// 216 PAY 213 

    0xaa0ebaa0,// 217 PAY 214 

    0xd0d195ee,// 218 PAY 215 

    0x1fa3bc7a,// 219 PAY 216 

    0x8aa1db15,// 220 PAY 217 

    0xdf5880fa,// 221 PAY 218 

    0xc994dc6e,// 222 PAY 219 

    0xa9038f8c,// 223 PAY 220 

    0xeeab3907,// 224 PAY 221 

    0x1aa154c6,// 225 PAY 222 

    0x9075f366,// 226 PAY 223 

    0xf2e3c84e,// 227 PAY 224 

    0xf51fa50d,// 228 PAY 225 

    0x718378cf,// 229 PAY 226 

    0xbec60bf5,// 230 PAY 227 

    0xa35b0c17,// 231 PAY 228 

    0xfa818754,// 232 PAY 229 

    0x1b5b9345,// 233 PAY 230 

    0xe88be981,// 234 PAY 231 

    0xc77d1a71,// 235 PAY 232 

    0x344bb69c,// 236 PAY 233 

    0xd45da438,// 237 PAY 234 

    0x05038b05,// 238 PAY 235 

    0x2e8f84cf,// 239 PAY 236 

    0x4e0f5307,// 240 PAY 237 

    0xf4389820,// 241 PAY 238 

    0x323ddeb4,// 242 PAY 239 

    0x3b98577a,// 243 PAY 240 

    0x636b28db,// 244 PAY 241 

    0x9390a676,// 245 PAY 242 

    0xc129c2bc,// 246 PAY 243 

    0xeee3bb94,// 247 PAY 244 

    0xb7028755,// 248 PAY 245 

    0xd0970fae,// 249 PAY 246 

    0xbcd8ce84,// 250 PAY 247 

    0x18d6dd86,// 251 PAY 248 

    0x25ef5a31,// 252 PAY 249 

    0xc5cf17ed,// 253 PAY 250 

    0x951a0870,// 254 PAY 251 

    0x4da58195,// 255 PAY 252 

    0x76c1a655,// 256 PAY 253 

    0x65d2e5d3,// 257 PAY 254 

    0x2d9f6b80,// 258 PAY 255 

    0x8e8dfaeb,// 259 PAY 256 

    0x8e28b5a6,// 260 PAY 257 

    0x011388cb,// 261 PAY 258 

    0xef1ed77f,// 262 PAY 259 

    0xbfbcdf0d,// 263 PAY 260 

    0xae2ba105,// 264 PAY 261 

    0xd2bb1318,// 265 PAY 262 

    0x3d904004,// 266 PAY 263 

    0x229b5966,// 267 PAY 264 

    0x7c017897,// 268 PAY 265 

    0x590318bd,// 269 PAY 266 

    0x1d1ce7fc,// 270 PAY 267 

    0xec4db19c,// 271 PAY 268 

    0x9ac63c42,// 272 PAY 269 

    0xddca5079,// 273 PAY 270 

    0x222d1244,// 274 PAY 271 

    0xa9b47056,// 275 PAY 272 

    0xbe7ccad4,// 276 PAY 273 

    0xada77f1e,// 277 PAY 274 

    0xdbb7ece2,// 278 PAY 275 

    0xab521dd7,// 279 PAY 276 

    0x5461f40b,// 280 PAY 277 

    0x6f8d96cf,// 281 PAY 278 

    0x0c52d6b1,// 282 PAY 279 

    0xb42baaac,// 283 PAY 280 

    0xafbe952c,// 284 PAY 281 

    0xd636578f,// 285 PAY 282 

    0x995aa004,// 286 PAY 283 

    0xe42d9e61,// 287 PAY 284 

    0x6e8c451a,// 288 PAY 285 

    0xfeba4a2b,// 289 PAY 286 

    0x44bf2900,// 290 PAY 287 

    0x8e31345e,// 291 PAY 288 

    0x3963413b,// 292 PAY 289 

    0x54f0907a,// 293 PAY 290 

    0xab07bb3a,// 294 PAY 291 

    0xfced4da6,// 295 PAY 292 

    0x428b8d71,// 296 PAY 293 

    0xc7467cd0,// 297 PAY 294 

    0x670813e7,// 298 PAY 295 

    0xfcaa95d3,// 299 PAY 296 

    0x13ab6327,// 300 PAY 297 

    0x15f1ed6e,// 301 PAY 298 

    0xc3b1d6b8,// 302 PAY 299 

    0xf9c96831,// 303 PAY 300 

    0x71b91ece,// 304 PAY 301 

    0x3aa59136,// 305 PAY 302 

    0x6941647c,// 306 PAY 303 

    0x55e7a115,// 307 PAY 304 

    0x95ceb8eb,// 308 PAY 305 

    0xd1e9db8d,// 309 PAY 306 

    0x5949a5cd,// 310 PAY 307 

    0x723b2094,// 311 PAY 308 

    0xeff2abdf,// 312 PAY 309 

    0x65c04c8c,// 313 PAY 310 

    0x595e6b90,// 314 PAY 311 

    0xf9edab8c,// 315 PAY 312 

    0x0bf30441,// 316 PAY 313 

    0x3cb3ea35,// 317 PAY 314 

    0xcf4f59ed,// 318 PAY 315 

    0x5a9a0b75,// 319 PAY 316 

    0x4e3a5fe6,// 320 PAY 317 

    0x50a8309d,// 321 PAY 318 

    0x177b23ab,// 322 PAY 319 

    0xc25c3456,// 323 PAY 320 

    0xfe445a1d,// 324 PAY 321 

    0xecf49b39,// 325 PAY 322 

    0x46f7db3b,// 326 PAY 323 

    0xf3216f6d,// 327 PAY 324 

    0xfca6a228,// 328 PAY 325 

    0x37eb7a0c,// 329 PAY 326 

    0x87bbba57,// 330 PAY 327 

    0xb7ef432e,// 331 PAY 328 

    0x9d2fbd28,// 332 PAY 329 

    0x42571dee,// 333 PAY 330 

    0x0e3fa8a6,// 334 PAY 331 

    0xd056e47f,// 335 PAY 332 

    0x2b4d6f53,// 336 PAY 333 

    0xf4f9a600,// 337 PAY 334 

    0x7c9a71b0,// 338 PAY 335 

    0xf05f5c9d,// 339 PAY 336 

    0x986df09d,// 340 PAY 337 

    0x9612070a,// 341 PAY 338 

    0x846d9e03,// 342 PAY 339 

    0x3d3189d2,// 343 PAY 340 

    0xd69113c6,// 344 PAY 341 

    0x46e96321,// 345 PAY 342 

    0xcd77c506,// 346 PAY 343 

    0xeace73bb,// 347 PAY 344 

    0xc23db1a8,// 348 PAY 345 

    0x2d19c117,// 349 PAY 346 

    0xd2fbd9cf,// 350 PAY 347 

    0x7d002c60,// 351 PAY 348 

    0x5ef12e92,// 352 PAY 349 

    0x8f724ecb,// 353 PAY 350 

    0xd9cc35c3,// 354 PAY 351 

    0x98f028ed,// 355 PAY 352 

    0x416b4b00,// 356 PAY 353 

    0x7b657cd2,// 357 PAY 354 

    0x4eb42f55,// 358 PAY 355 

    0x42fd467f,// 359 PAY 356 

    0xf354fb0b,// 360 PAY 357 

    0x6f67871b,// 361 PAY 358 

    0x50fd03ca,// 362 PAY 359 

    0xf4c545fb,// 363 PAY 360 

    0x21f5ba41,// 364 PAY 361 

    0xb8912662,// 365 PAY 362 

    0x5c43b598,// 366 PAY 363 

    0x7db326d9,// 367 PAY 364 

    0xbd5d831d,// 368 PAY 365 

    0xbe47e475,// 369 PAY 366 

    0x9651b6a1,// 370 PAY 367 

    0x13d56bcb,// 371 PAY 368 

    0xb83c5db9,// 372 PAY 369 

    0xc4ba4013,// 373 PAY 370 

    0xbdf91fc5,// 374 PAY 371 

    0x04e975ae,// 375 PAY 372 

    0xa715f342,// 376 PAY 373 

    0x58fb5c59,// 377 PAY 374 

    0xb4a94ef4,// 378 PAY 375 

    0xb57808db,// 379 PAY 376 

    0xb9f0954f,// 380 PAY 377 

    0x7f0b1982,// 381 PAY 378 

    0xa51e715b,// 382 PAY 379 

    0x863b5a91,// 383 PAY 380 

    0x2a36e648,// 384 PAY 381 

    0xb8f3b5ae,// 385 PAY 382 

    0xd712b213,// 386 PAY 383 

    0x8897dbad,// 387 PAY 384 

    0xa6704e48,// 388 PAY 385 

    0xc180bca4,// 389 PAY 386 

    0xe42515e6,// 390 PAY 387 

    0x3b35331d,// 391 PAY 388 

    0x02ddc18d,// 392 PAY 389 

    0x72421405,// 393 PAY 390 

    0xaeeae95f,// 394 PAY 391 

    0x106a572a,// 395 PAY 392 

    0x1db52d42,// 396 PAY 393 

    0x9985c3e0,// 397 PAY 394 

    0xa1d7289f,// 398 PAY 395 

    0x9ff2ba5f,// 399 PAY 396 

    0x2e6b8845,// 400 PAY 397 

    0x402fa060,// 401 PAY 398 

    0x6539fdf1,// 402 PAY 399 

    0x6d371090,// 403 PAY 400 

    0xf62ca997,// 404 PAY 401 

    0xefbb6f81,// 405 PAY 402 

    0x1ac35c89,// 406 PAY 403 

    0xcbcff5c7,// 407 PAY 404 

    0xf8551b6e,// 408 PAY 405 

    0x01a54f62,// 409 PAY 406 

    0xa4df2dc3,// 410 PAY 407 

    0xa4ec7f89,// 411 PAY 408 

    0x128321e4,// 412 PAY 409 

    0x2f54cd4d,// 413 PAY 410 

    0xfba3329e,// 414 PAY 411 

    0x31ba9861,// 415 PAY 412 

    0xd2c08dd3,// 416 PAY 413 

    0x9a8633ff,// 417 PAY 414 

    0xc15ce407,// 418 PAY 415 

    0x751763c9,// 419 PAY 416 

    0xceefcd67,// 420 PAY 417 

    0xc1b030f9,// 421 PAY 418 

    0x671bb721,// 422 PAY 419 

    0x90461d18,// 423 PAY 420 

    0xd48b59e0,// 424 PAY 421 

    0xc982cf49,// 425 PAY 422 

    0x31a3af60,// 426 PAY 423 

    0xaf69cf47,// 427 PAY 424 

    0xbffd78a6,// 428 PAY 425 

    0x320646a9,// 429 PAY 426 

    0x90227be8,// 430 PAY 427 

    0xb985c8f7,// 431 PAY 428 

    0xe883fd4b,// 432 PAY 429 

    0x63946c94,// 433 PAY 430 

    0x3e375400,// 434 PAY 431 

    0xf47218ca,// 435 PAY 432 

    0x510daf7f,// 436 PAY 433 

    0x6842311b,// 437 PAY 434 

    0xb8e0fb23,// 438 PAY 435 

    0xa33f8bb9,// 439 PAY 436 

    0x52522ac6,// 440 PAY 437 

    0x78286c9c,// 441 PAY 438 

    0x0c30da94,// 442 PAY 439 

    0xc371708b,// 443 PAY 440 

    0xbee2106c,// 444 PAY 441 

    0x4b549321,// 445 PAY 442 

    0x65cc8007,// 446 PAY 443 

    0x27239179,// 447 PAY 444 

    0x941fe5bf,// 448 PAY 445 

    0xc54d0cfe,// 449 PAY 446 

    0x7c3b82cc,// 450 PAY 447 

    0xba01751d,// 451 PAY 448 

    0x845f9860,// 452 PAY 449 

    0x8aae209e,// 453 PAY 450 

    0x552c50f2,// 454 PAY 451 

    0x5f9fa005,// 455 PAY 452 

    0x89cb7401,// 456 PAY 453 

    0xa9259a39,// 457 PAY 454 

    0x4184ab5c,// 458 PAY 455 

    0xff55486b,// 459 PAY 456 

    0x647c26fd,// 460 PAY 457 

    0x82b51a97,// 461 PAY 458 

    0x626e9faf,// 462 PAY 459 

    0xa205824d,// 463 PAY 460 

    0xb36377ae,// 464 PAY 461 

    0x4dc38c26,// 465 PAY 462 

    0x841db16d,// 466 PAY 463 

    0xe1f3d655,// 467 PAY 464 

    0xe69224a0,// 468 PAY 465 

    0xa56a69c8,// 469 PAY 466 

    0x19b66163,// 470 PAY 467 

    0x99b430d0,// 471 PAY 468 

    0x10eedaa7,// 472 PAY 469 

    0x4a8378db,// 473 PAY 470 

    0xe2a5077b,// 474 PAY 471 

    0xd21d4b2a,// 475 PAY 472 

    0xd81e38d0,// 476 PAY 473 

    0xbc12a989,// 477 PAY 474 

    0xb9a868c3,// 478 PAY 475 

    0x635bb610,// 479 PAY 476 

    0x5b7dd459,// 480 PAY 477 

    0x134beba6,// 481 PAY 478 

    0xfbef10be,// 482 PAY 479 

    0x6792b7ce,// 483 PAY 480 

    0x457b9c67,// 484 PAY 481 

    0xb4eb98ea,// 485 PAY 482 

    0xd93ad498,// 486 PAY 483 

    0x0ed7aa63,// 487 PAY 484 

    0x378af3e3,// 488 PAY 485 

    0x246da7e8,// 489 PAY 486 

    0x60c3bf9c,// 490 PAY 487 

    0xed8fc041,// 491 PAY 488 

    0x1cb6eea2,// 492 PAY 489 

    0x8ff67d24,// 493 PAY 490 

    0xa749f975,// 494 PAY 491 

    0x7e1ab472,// 495 PAY 492 

    0x452f26a5,// 496 PAY 493 

    0x8580144f,// 497 PAY 494 

    0xa2f8fdd6,// 498 PAY 495 

    0xb4b9e9fa,// 499 PAY 496 

    0xe9f02e92,// 500 PAY 497 

    0x40ebe005,// 501 PAY 498 

    0x657f0000,// 502 PAY 499 

/// HASH is  4 bytes 

    0xff55486b,// 503 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 164 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x0161bea4 // 504 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt38_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 358 words. 

/// BDA size     is 1427 (0x593) 

/// BDA id       is 0x1528 

    0x05931528,// 3 BDA   1 

/// PAY Generic Data size   : 1427 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x9788ad68,// 4 PAY   1 

    0x2e1d5e26,// 5 PAY   2 

    0x1305b129,// 6 PAY   3 

    0x249200e2,// 7 PAY   4 

    0xb0c90a63,// 8 PAY   5 

    0xc8573ea7,// 9 PAY   6 

    0x3bea80b8,// 10 PAY   7 

    0xf39e3f07,// 11 PAY   8 

    0x70b8639e,// 12 PAY   9 

    0xbced92b5,// 13 PAY  10 

    0x07586b7b,// 14 PAY  11 

    0x984a84c1,// 15 PAY  12 

    0x36dc5b0d,// 16 PAY  13 

    0x49209399,// 17 PAY  14 

    0x8b496b13,// 18 PAY  15 

    0x6c6a3647,// 19 PAY  16 

    0x707414ec,// 20 PAY  17 

    0x1498f614,// 21 PAY  18 

    0xea09dc3d,// 22 PAY  19 

    0xaebdd35e,// 23 PAY  20 

    0x2434f17e,// 24 PAY  21 

    0x3b06fc1d,// 25 PAY  22 

    0xba83ea8f,// 26 PAY  23 

    0x80725474,// 27 PAY  24 

    0x52c8eb1e,// 28 PAY  25 

    0xec44336a,// 29 PAY  26 

    0x51de0b6a,// 30 PAY  27 

    0xd669d18f,// 31 PAY  28 

    0x8746c3fc,// 32 PAY  29 

    0xd0fb43da,// 33 PAY  30 

    0x8fe2cb8c,// 34 PAY  31 

    0xeb74c195,// 35 PAY  32 

    0xe2340d85,// 36 PAY  33 

    0x4ac72f3e,// 37 PAY  34 

    0xa1a8d21e,// 38 PAY  35 

    0xb77303ac,// 39 PAY  36 

    0x0a2393c3,// 40 PAY  37 

    0x76684404,// 41 PAY  38 

    0x6d465ff8,// 42 PAY  39 

    0x2f37ab40,// 43 PAY  40 

    0xb27d6656,// 44 PAY  41 

    0xeda8b298,// 45 PAY  42 

    0xd9408b6a,// 46 PAY  43 

    0x12c6f246,// 47 PAY  44 

    0x282fc62e,// 48 PAY  45 

    0x33a19277,// 49 PAY  46 

    0x8b13ebac,// 50 PAY  47 

    0xe5008528,// 51 PAY  48 

    0x4a88ef0a,// 52 PAY  49 

    0xc22bbbaf,// 53 PAY  50 

    0xce83b8cc,// 54 PAY  51 

    0x0eb2a9c3,// 55 PAY  52 

    0x1a2b1427,// 56 PAY  53 

    0xc4698244,// 57 PAY  54 

    0x43f95134,// 58 PAY  55 

    0x4995db6e,// 59 PAY  56 

    0xe713f84e,// 60 PAY  57 

    0xc2480380,// 61 PAY  58 

    0x948e5775,// 62 PAY  59 

    0xf9de4f3a,// 63 PAY  60 

    0x2deabb35,// 64 PAY  61 

    0x74b253c3,// 65 PAY  62 

    0x4fdd1fab,// 66 PAY  63 

    0x13e210b2,// 67 PAY  64 

    0x3e3255a8,// 68 PAY  65 

    0xa1a0b588,// 69 PAY  66 

    0x984394f0,// 70 PAY  67 

    0x52eac021,// 71 PAY  68 

    0x3afd3e26,// 72 PAY  69 

    0x32fad573,// 73 PAY  70 

    0x28ccc01b,// 74 PAY  71 

    0x7604b53b,// 75 PAY  72 

    0x4c75f5b7,// 76 PAY  73 

    0xb4b5a56f,// 77 PAY  74 

    0x5c1ca95e,// 78 PAY  75 

    0xa99544d2,// 79 PAY  76 

    0xf9d7f2d3,// 80 PAY  77 

    0xb8419a59,// 81 PAY  78 

    0x2798a1c4,// 82 PAY  79 

    0x0f9c03a5,// 83 PAY  80 

    0x82819284,// 84 PAY  81 

    0x1394d3c2,// 85 PAY  82 

    0x9ea415a9,// 86 PAY  83 

    0x0c2fbbf6,// 87 PAY  84 

    0x2694ec9d,// 88 PAY  85 

    0x4a73140e,// 89 PAY  86 

    0xf0363881,// 90 PAY  87 

    0x862f760b,// 91 PAY  88 

    0x03dbcd6d,// 92 PAY  89 

    0x38874249,// 93 PAY  90 

    0xf29d0e95,// 94 PAY  91 

    0x0065ee4e,// 95 PAY  92 

    0xfb8b3595,// 96 PAY  93 

    0x7840c152,// 97 PAY  94 

    0xa6339256,// 98 PAY  95 

    0x7f4568d0,// 99 PAY  96 

    0x0e25b3fd,// 100 PAY  97 

    0x256bf599,// 101 PAY  98 

    0xfb70fa50,// 102 PAY  99 

    0x4fe18938,// 103 PAY 100 

    0x20cd9e0e,// 104 PAY 101 

    0x9ef30fab,// 105 PAY 102 

    0x56af4ede,// 106 PAY 103 

    0x842b200f,// 107 PAY 104 

    0x1fd5c673,// 108 PAY 105 

    0x66546dcd,// 109 PAY 106 

    0x449dc8fc,// 110 PAY 107 

    0xfa1eceaf,// 111 PAY 108 

    0x9304b60a,// 112 PAY 109 

    0x968abe46,// 113 PAY 110 

    0xbe5c587a,// 114 PAY 111 

    0xaef73359,// 115 PAY 112 

    0xcf41b2a4,// 116 PAY 113 

    0x1ba2b45e,// 117 PAY 114 

    0xa64321a8,// 118 PAY 115 

    0xf5c15b1a,// 119 PAY 116 

    0x06f0528d,// 120 PAY 117 

    0x15b5b54b,// 121 PAY 118 

    0xcab8cb2c,// 122 PAY 119 

    0xbf9d5cac,// 123 PAY 120 

    0xbed72e91,// 124 PAY 121 

    0x759c1b99,// 125 PAY 122 

    0x7ef5f721,// 126 PAY 123 

    0x3d8f8094,// 127 PAY 124 

    0xde13b0db,// 128 PAY 125 

    0x4a4a8a6c,// 129 PAY 126 

    0x616fd00e,// 130 PAY 127 

    0xd7d3050f,// 131 PAY 128 

    0x12ad7171,// 132 PAY 129 

    0xf2254b3c,// 133 PAY 130 

    0x660f2e02,// 134 PAY 131 

    0x64508b5d,// 135 PAY 132 

    0x965468db,// 136 PAY 133 

    0x18d94eff,// 137 PAY 134 

    0xf9d704fe,// 138 PAY 135 

    0x68f775b4,// 139 PAY 136 

    0x42b30866,// 140 PAY 137 

    0x604f300e,// 141 PAY 138 

    0x700b6418,// 142 PAY 139 

    0x01185e4a,// 143 PAY 140 

    0xeee6d89c,// 144 PAY 141 

    0xc8a74d67,// 145 PAY 142 

    0xcc9fe2f1,// 146 PAY 143 

    0xc9e2f18e,// 147 PAY 144 

    0x1614a6c9,// 148 PAY 145 

    0x35ff60f4,// 149 PAY 146 

    0x4d32dc9e,// 150 PAY 147 

    0x3cccea25,// 151 PAY 148 

    0x583f80ab,// 152 PAY 149 

    0xc1b887db,// 153 PAY 150 

    0xe5ac689c,// 154 PAY 151 

    0xceea63be,// 155 PAY 152 

    0x28c94448,// 156 PAY 153 

    0x1e5b7086,// 157 PAY 154 

    0x290fdced,// 158 PAY 155 

    0x9aa7b247,// 159 PAY 156 

    0x77f35b72,// 160 PAY 157 

    0xd7f9fab1,// 161 PAY 158 

    0x08e54ad6,// 162 PAY 159 

    0x7c8eede5,// 163 PAY 160 

    0x68879360,// 164 PAY 161 

    0x13636bf4,// 165 PAY 162 

    0xcd1fdf42,// 166 PAY 163 

    0x9a18cd6b,// 167 PAY 164 

    0xf0837d3d,// 168 PAY 165 

    0xcf269b53,// 169 PAY 166 

    0xdf519cbc,// 170 PAY 167 

    0xc81d75ae,// 171 PAY 168 

    0x911d5911,// 172 PAY 169 

    0x3a9d10a6,// 173 PAY 170 

    0x8ac6f809,// 174 PAY 171 

    0x0b77ba97,// 175 PAY 172 

    0x87249662,// 176 PAY 173 

    0xe1d056d9,// 177 PAY 174 

    0x0733ebcc,// 178 PAY 175 

    0x021f1250,// 179 PAY 176 

    0x939a3c25,// 180 PAY 177 

    0x553010cf,// 181 PAY 178 

    0x8d880bee,// 182 PAY 179 

    0x8364d7fe,// 183 PAY 180 

    0x8cda57bd,// 184 PAY 181 

    0x2903399c,// 185 PAY 182 

    0x68616a36,// 186 PAY 183 

    0x1e5274d4,// 187 PAY 184 

    0x1cd8c648,// 188 PAY 185 

    0xff970ecf,// 189 PAY 186 

    0x1503c3e0,// 190 PAY 187 

    0x8909974a,// 191 PAY 188 

    0x18e57b67,// 192 PAY 189 

    0x38b6e932,// 193 PAY 190 

    0x9eb5ae25,// 194 PAY 191 

    0xa857920b,// 195 PAY 192 

    0x5877eca0,// 196 PAY 193 

    0x08034fd4,// 197 PAY 194 

    0x5ee0dae2,// 198 PAY 195 

    0x01f09b59,// 199 PAY 196 

    0x8a208e24,// 200 PAY 197 

    0x31012f8d,// 201 PAY 198 

    0xcc419c18,// 202 PAY 199 

    0xbe0ad11b,// 203 PAY 200 

    0x4fbcf60d,// 204 PAY 201 

    0x2b2ad5d7,// 205 PAY 202 

    0x7d863660,// 206 PAY 203 

    0x0ddf820c,// 207 PAY 204 

    0x1c2a4a5a,// 208 PAY 205 

    0x2e2e0c7e,// 209 PAY 206 

    0xb45d9a7a,// 210 PAY 207 

    0xb53075e1,// 211 PAY 208 

    0x68a877d2,// 212 PAY 209 

    0x49819a62,// 213 PAY 210 

    0xc4a3ae72,// 214 PAY 211 

    0x6473d0e4,// 215 PAY 212 

    0x87c41ce1,// 216 PAY 213 

    0x1a600b7e,// 217 PAY 214 

    0xfdf83f69,// 218 PAY 215 

    0x3d6cd45f,// 219 PAY 216 

    0xee4596ed,// 220 PAY 217 

    0x1ff2550c,// 221 PAY 218 

    0x1ef91081,// 222 PAY 219 

    0x786193b7,// 223 PAY 220 

    0xc4739a56,// 224 PAY 221 

    0x81ea406f,// 225 PAY 222 

    0xca3bda60,// 226 PAY 223 

    0x6442f4bd,// 227 PAY 224 

    0x82c5c169,// 228 PAY 225 

    0xd399a424,// 229 PAY 226 

    0x35d2fe2a,// 230 PAY 227 

    0xe6584487,// 231 PAY 228 

    0xfec28b8e,// 232 PAY 229 

    0x8f84ec58,// 233 PAY 230 

    0xc1ae6aad,// 234 PAY 231 

    0x5b757014,// 235 PAY 232 

    0x70f8f8fc,// 236 PAY 233 

    0x62aefdf8,// 237 PAY 234 

    0xf88932ed,// 238 PAY 235 

    0x17fbb662,// 239 PAY 236 

    0x170f202e,// 240 PAY 237 

    0xeb63a80f,// 241 PAY 238 

    0x7ccb85e5,// 242 PAY 239 

    0x3d7678bb,// 243 PAY 240 

    0x0d7fe475,// 244 PAY 241 

    0xfb2de9ba,// 245 PAY 242 

    0xa84cf98e,// 246 PAY 243 

    0x2a0e482a,// 247 PAY 244 

    0xebd23e3d,// 248 PAY 245 

    0x476af6dd,// 249 PAY 246 

    0xd0c8c36e,// 250 PAY 247 

    0x56891f95,// 251 PAY 248 

    0x575c721a,// 252 PAY 249 

    0x394e4823,// 253 PAY 250 

    0xa4db1fd5,// 254 PAY 251 

    0xb3ca050c,// 255 PAY 252 

    0x27e32e6d,// 256 PAY 253 

    0x91bebb05,// 257 PAY 254 

    0x9e763796,// 258 PAY 255 

    0x27aa399a,// 259 PAY 256 

    0x9fd5eb88,// 260 PAY 257 

    0xd3eacebe,// 261 PAY 258 

    0x475f0227,// 262 PAY 259 

    0xb3c6de82,// 263 PAY 260 

    0x1e51a928,// 264 PAY 261 

    0xe7b2011a,// 265 PAY 262 

    0x3043357e,// 266 PAY 263 

    0xbf4c1fd3,// 267 PAY 264 

    0x499f86a3,// 268 PAY 265 

    0x6e08c696,// 269 PAY 266 

    0xf30dfca0,// 270 PAY 267 

    0x781fe9d9,// 271 PAY 268 

    0x7cafb9b8,// 272 PAY 269 

    0x52ea5654,// 273 PAY 270 

    0xb52cd75c,// 274 PAY 271 

    0x52264780,// 275 PAY 272 

    0x989cef0b,// 276 PAY 273 

    0xe5c9fb83,// 277 PAY 274 

    0x32218a65,// 278 PAY 275 

    0x75117005,// 279 PAY 276 

    0x2ec9b5d8,// 280 PAY 277 

    0x3221f0ca,// 281 PAY 278 

    0xb13d4974,// 282 PAY 279 

    0xb4c2462f,// 283 PAY 280 

    0xea38adbf,// 284 PAY 281 

    0xaab0a148,// 285 PAY 282 

    0x52517f79,// 286 PAY 283 

    0xe9c17560,// 287 PAY 284 

    0xa5276903,// 288 PAY 285 

    0xc7015fe8,// 289 PAY 286 

    0xf972c97a,// 290 PAY 287 

    0x769451ab,// 291 PAY 288 

    0x85c36282,// 292 PAY 289 

    0x8c548149,// 293 PAY 290 

    0xe908e25c,// 294 PAY 291 

    0x6be75308,// 295 PAY 292 

    0xf9db1a77,// 296 PAY 293 

    0x6df91d9b,// 297 PAY 294 

    0x82f6442a,// 298 PAY 295 

    0x51273ac2,// 299 PAY 296 

    0xeb522d4d,// 300 PAY 297 

    0x13b3741d,// 301 PAY 298 

    0xe8f591ea,// 302 PAY 299 

    0x843250cd,// 303 PAY 300 

    0x22d65e94,// 304 PAY 301 

    0x78aee0fc,// 305 PAY 302 

    0x670dc729,// 306 PAY 303 

    0xb2142cc5,// 307 PAY 304 

    0x7104e452,// 308 PAY 305 

    0xf6fafdc4,// 309 PAY 306 

    0x587b8834,// 310 PAY 307 

    0x44da409e,// 311 PAY 308 

    0x8a18896d,// 312 PAY 309 

    0x172349c8,// 313 PAY 310 

    0x9371c64a,// 314 PAY 311 

    0xf740d9a0,// 315 PAY 312 

    0x9d7067f5,// 316 PAY 313 

    0x5b827ea6,// 317 PAY 314 

    0x7396d303,// 318 PAY 315 

    0xb8f68958,// 319 PAY 316 

    0xac744741,// 320 PAY 317 

    0x1e8840c8,// 321 PAY 318 

    0x91a63868,// 322 PAY 319 

    0x352ffcb3,// 323 PAY 320 

    0xfd54120f,// 324 PAY 321 

    0x4be509fc,// 325 PAY 322 

    0x2d704d5f,// 326 PAY 323 

    0xf84f1d73,// 327 PAY 324 

    0x535d3c9d,// 328 PAY 325 

    0x92ca4375,// 329 PAY 326 

    0xcf9c9b6c,// 330 PAY 327 

    0x6727275e,// 331 PAY 328 

    0x1d5a0615,// 332 PAY 329 

    0x523cd9b3,// 333 PAY 330 

    0x18916f1a,// 334 PAY 331 

    0x2eff32b1,// 335 PAY 332 

    0x43005e33,// 336 PAY 333 

    0x89140258,// 337 PAY 334 

    0xf8561bd0,// 338 PAY 335 

    0x4a5c5e5b,// 339 PAY 336 

    0x4ed4144c,// 340 PAY 337 

    0xb381a3da,// 341 PAY 338 

    0xdb4fc1e9,// 342 PAY 339 

    0x392d3367,// 343 PAY 340 

    0x2dd52ee5,// 344 PAY 341 

    0xf748af2d,// 345 PAY 342 

    0xd90b2ec8,// 346 PAY 343 

    0x38bff693,// 347 PAY 344 

    0x17accc2e,// 348 PAY 345 

    0xf95bf509,// 349 PAY 346 

    0x3b247a25,// 350 PAY 347 

    0x1628f6a3,// 351 PAY 348 

    0x6d3dd74b,// 352 PAY 349 

    0x52319573,// 353 PAY 350 

    0x5e52e979,// 354 PAY 351 

    0xff89ade6,// 355 PAY 352 

    0xb5e7322f,// 356 PAY 353 

    0x0beabe33,// 357 PAY 354 

    0x65942361,// 358 PAY 355 

    0xbed9d74a,// 359 PAY 356 

    0x0990e000,// 360 PAY 357 

/// HASH is  16 bytes 

    0xe8f591ea,// 361 HSH   1 

    0x843250cd,// 362 HSH   2 

    0x22d65e94,// 363 HSH   3 

    0x78aee0fc,// 364 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 229 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4f 

    0x01314fe5 // 365 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt39_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 244 words. 

/// BDA size     is 970 (0x3ca) 

/// BDA id       is 0xc25c 

    0x03cac25c,// 3 BDA   1 

/// PAY Generic Data size   : 970 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x0ac17073,// 4 PAY   1 

    0x8f344251,// 5 PAY   2 

    0x92ea17db,// 6 PAY   3 

    0x570626e7,// 7 PAY   4 

    0xe18f1349,// 8 PAY   5 

    0x97e9bdb5,// 9 PAY   6 

    0x4ed83ebe,// 10 PAY   7 

    0x9877b53b,// 11 PAY   8 

    0xc0398069,// 12 PAY   9 

    0xdac4ffda,// 13 PAY  10 

    0x70052735,// 14 PAY  11 

    0xed88514c,// 15 PAY  12 

    0xdb5f0dd2,// 16 PAY  13 

    0x20030dea,// 17 PAY  14 

    0x525b2ab3,// 18 PAY  15 

    0xb61f2587,// 19 PAY  16 

    0x53929c56,// 20 PAY  17 

    0x482f9898,// 21 PAY  18 

    0x195ef3b2,// 22 PAY  19 

    0xefd6b432,// 23 PAY  20 

    0xcf0272e3,// 24 PAY  21 

    0x12050d33,// 25 PAY  22 

    0x7eb2f17f,// 26 PAY  23 

    0x8fa5807d,// 27 PAY  24 

    0x5662fe9c,// 28 PAY  25 

    0xd260ed24,// 29 PAY  26 

    0x2ff5c54c,// 30 PAY  27 

    0x34bcb9b3,// 31 PAY  28 

    0x7fbf654a,// 32 PAY  29 

    0x86c39f2b,// 33 PAY  30 

    0x50e0d09f,// 34 PAY  31 

    0x59b91cf0,// 35 PAY  32 

    0xbb98dbca,// 36 PAY  33 

    0x10c78bfb,// 37 PAY  34 

    0x4f5f5ad7,// 38 PAY  35 

    0x1a630a41,// 39 PAY  36 

    0x9b436584,// 40 PAY  37 

    0x0b11baf0,// 41 PAY  38 

    0x89c0567b,// 42 PAY  39 

    0xc67ff12d,// 43 PAY  40 

    0x4deac532,// 44 PAY  41 

    0x3125f449,// 45 PAY  42 

    0xca802ed2,// 46 PAY  43 

    0x899370ad,// 47 PAY  44 

    0x7da62f5a,// 48 PAY  45 

    0xd2c32578,// 49 PAY  46 

    0xca9f298d,// 50 PAY  47 

    0x620b3482,// 51 PAY  48 

    0x0ac6fffd,// 52 PAY  49 

    0x8f49ac0f,// 53 PAY  50 

    0xe758d635,// 54 PAY  51 

    0x10143b95,// 55 PAY  52 

    0x4ca9bac1,// 56 PAY  53 

    0x48000f5f,// 57 PAY  54 

    0xffd5daae,// 58 PAY  55 

    0x8709c6ce,// 59 PAY  56 

    0x6eb21726,// 60 PAY  57 

    0x2e1b1cd0,// 61 PAY  58 

    0x255622b3,// 62 PAY  59 

    0xd5337d63,// 63 PAY  60 

    0xbc6b50c2,// 64 PAY  61 

    0xafd419e6,// 65 PAY  62 

    0x296263d0,// 66 PAY  63 

    0xb983a236,// 67 PAY  64 

    0x098d6ffb,// 68 PAY  65 

    0x117e7290,// 69 PAY  66 

    0x255aa2ee,// 70 PAY  67 

    0x12c933c6,// 71 PAY  68 

    0x17369bb1,// 72 PAY  69 

    0x907c2703,// 73 PAY  70 

    0xaf2c303c,// 74 PAY  71 

    0x7a054634,// 75 PAY  72 

    0x266527ce,// 76 PAY  73 

    0x2e9df8c0,// 77 PAY  74 

    0x32be48e1,// 78 PAY  75 

    0x6ab81d72,// 79 PAY  76 

    0xe57bc43e,// 80 PAY  77 

    0x6c9dc230,// 81 PAY  78 

    0x3d517666,// 82 PAY  79 

    0x2b86f9c9,// 83 PAY  80 

    0xd7003c82,// 84 PAY  81 

    0xaec7eb5a,// 85 PAY  82 

    0x7d10c7fc,// 86 PAY  83 

    0xd8a41b8f,// 87 PAY  84 

    0xb02e4a12,// 88 PAY  85 

    0x8729e90a,// 89 PAY  86 

    0xd1bbcc03,// 90 PAY  87 

    0x8d12cb2d,// 91 PAY  88 

    0x44c86448,// 92 PAY  89 

    0x1ceb5188,// 93 PAY  90 

    0xb46af00b,// 94 PAY  91 

    0xdac4d154,// 95 PAY  92 

    0xfe8807b1,// 96 PAY  93 

    0x5652d5cb,// 97 PAY  94 

    0x54653747,// 98 PAY  95 

    0xf28f1967,// 99 PAY  96 

    0x0d6695c1,// 100 PAY  97 

    0xdfc7356b,// 101 PAY  98 

    0x7bf49db2,// 102 PAY  99 

    0x2ce27eb5,// 103 PAY 100 

    0x33d98ce0,// 104 PAY 101 

    0x9d937e94,// 105 PAY 102 

    0x35c7e93a,// 106 PAY 103 

    0x5ee9c649,// 107 PAY 104 

    0xa9fde6c9,// 108 PAY 105 

    0xf631a9f7,// 109 PAY 106 

    0x159bfc6e,// 110 PAY 107 

    0x1f876e22,// 111 PAY 108 

    0xa8c285cc,// 112 PAY 109 

    0x55c51885,// 113 PAY 110 

    0x0e9696c5,// 114 PAY 111 

    0xee261ee8,// 115 PAY 112 

    0xa3ab7e0e,// 116 PAY 113 

    0xa81a6f5b,// 117 PAY 114 

    0x234c36dc,// 118 PAY 115 

    0x078f8b8d,// 119 PAY 116 

    0x4c43f3d0,// 120 PAY 117 

    0x6aa85151,// 121 PAY 118 

    0x7ba95c68,// 122 PAY 119 

    0xa111589e,// 123 PAY 120 

    0x92cf21dc,// 124 PAY 121 

    0xf095b5e0,// 125 PAY 122 

    0x0de62d31,// 126 PAY 123 

    0xbfb95a6e,// 127 PAY 124 

    0x3bc80bc0,// 128 PAY 125 

    0xef1de1c2,// 129 PAY 126 

    0x12fef8aa,// 130 PAY 127 

    0xd9846b10,// 131 PAY 128 

    0x6c0ac73d,// 132 PAY 129 

    0x3c741fe1,// 133 PAY 130 

    0xcd59a566,// 134 PAY 131 

    0x65fd391d,// 135 PAY 132 

    0x698f737c,// 136 PAY 133 

    0xfafe0d99,// 137 PAY 134 

    0x84a7ec14,// 138 PAY 135 

    0xbdda690c,// 139 PAY 136 

    0x897de9c5,// 140 PAY 137 

    0xf97ef816,// 141 PAY 138 

    0xd6a05aa5,// 142 PAY 139 

    0x251d90b6,// 143 PAY 140 

    0xf60b187b,// 144 PAY 141 

    0xf876c50f,// 145 PAY 142 

    0xf409f440,// 146 PAY 143 

    0x686cf081,// 147 PAY 144 

    0x6f28fa86,// 148 PAY 145 

    0x06480495,// 149 PAY 146 

    0x6f816019,// 150 PAY 147 

    0x74a4012d,// 151 PAY 148 

    0x10729cf3,// 152 PAY 149 

    0x9322beac,// 153 PAY 150 

    0x4a8e57bf,// 154 PAY 151 

    0x3d28e42e,// 155 PAY 152 

    0x694fce94,// 156 PAY 153 

    0xe040dcdf,// 157 PAY 154 

    0xa0d2ea77,// 158 PAY 155 

    0x5b701f92,// 159 PAY 156 

    0xcaa4e845,// 160 PAY 157 

    0x8786877c,// 161 PAY 158 

    0xa4d36a2c,// 162 PAY 159 

    0x966c6ec4,// 163 PAY 160 

    0x0b667895,// 164 PAY 161 

    0x026812da,// 165 PAY 162 

    0xc8ba5d5c,// 166 PAY 163 

    0x95b48993,// 167 PAY 164 

    0xd5407aef,// 168 PAY 165 

    0x410f120e,// 169 PAY 166 

    0xf40cb684,// 170 PAY 167 

    0x021fbe27,// 171 PAY 168 

    0x5046aa47,// 172 PAY 169 

    0x708193b1,// 173 PAY 170 

    0x218d466e,// 174 PAY 171 

    0xeb429e10,// 175 PAY 172 

    0x07f54f07,// 176 PAY 173 

    0x0c84063a,// 177 PAY 174 

    0xb327b89f,// 178 PAY 175 

    0xeb80e09e,// 179 PAY 176 

    0x02b7bae2,// 180 PAY 177 

    0xc89538fe,// 181 PAY 178 

    0x03403985,// 182 PAY 179 

    0xb39f18c9,// 183 PAY 180 

    0xd6be776b,// 184 PAY 181 

    0x92f385b3,// 185 PAY 182 

    0x6fc9d463,// 186 PAY 183 

    0xe6e51a0d,// 187 PAY 184 

    0x6ec53185,// 188 PAY 185 

    0xb1d9293a,// 189 PAY 186 

    0x85172c49,// 190 PAY 187 

    0x34a8c526,// 191 PAY 188 

    0x31795f2d,// 192 PAY 189 

    0x449c70e3,// 193 PAY 190 

    0x53e62ee3,// 194 PAY 191 

    0xcc215c08,// 195 PAY 192 

    0x4def5a52,// 196 PAY 193 

    0x9c26341f,// 197 PAY 194 

    0x69a7064a,// 198 PAY 195 

    0xc55add1c,// 199 PAY 196 

    0x5162f901,// 200 PAY 197 

    0xb5c3cf6f,// 201 PAY 198 

    0x431c5dc1,// 202 PAY 199 

    0x9100c190,// 203 PAY 200 

    0xd7be1437,// 204 PAY 201 

    0x3ef6bcdb,// 205 PAY 202 

    0xae8d61b5,// 206 PAY 203 

    0xbeb0f366,// 207 PAY 204 

    0xbf9b13d0,// 208 PAY 205 

    0x69ffdd24,// 209 PAY 206 

    0xf7f49327,// 210 PAY 207 

    0x8519a10b,// 211 PAY 208 

    0x7de29e30,// 212 PAY 209 

    0x08eaf029,// 213 PAY 210 

    0x47b25136,// 214 PAY 211 

    0x59ade212,// 215 PAY 212 

    0xf32cf4c4,// 216 PAY 213 

    0x188045dd,// 217 PAY 214 

    0x9c775ab1,// 218 PAY 215 

    0x06ea68f2,// 219 PAY 216 

    0xbe1c3e73,// 220 PAY 217 

    0x50e17235,// 221 PAY 218 

    0x1bd32b44,// 222 PAY 219 

    0x52391438,// 223 PAY 220 

    0x976fe308,// 224 PAY 221 

    0xc2966f20,// 225 PAY 222 

    0x9e81b877,// 226 PAY 223 

    0xd8816621,// 227 PAY 224 

    0x6239410b,// 228 PAY 225 

    0xcb92a969,// 229 PAY 226 

    0x79f10f0a,// 230 PAY 227 

    0xc51b5ea4,// 231 PAY 228 

    0xfcabb8ba,// 232 PAY 229 

    0xf5ff6fa0,// 233 PAY 230 

    0x4589ebb2,// 234 PAY 231 

    0xdf75fb10,// 235 PAY 232 

    0xe9a13d87,// 236 PAY 233 

    0xb174ebad,// 237 PAY 234 

    0xcf5d5445,// 238 PAY 235 

    0x25fec0cf,// 239 PAY 236 

    0xc622d8ea,// 240 PAY 237 

    0xf135c042,// 241 PAY 238 

    0xe2d2bd4a,// 242 PAY 239 

    0xd8973053,// 243 PAY 240 

    0xf90e8fa4,// 244 PAY 241 

    0x9dacedda,// 245 PAY 242 

    0xbe620000,// 246 PAY 243 

/// HASH is  8 bytes 

    0xf28f1967,// 247 HSH   1 

    0x0d6695c1,// 248 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 208 

/// STA pkt_idx        : 48 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5f 

    0x00c05fd0 // 249 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt40_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 341 words. 

/// BDA size     is 1358 (0x54e) 

/// BDA id       is 0xd94d 

    0x054ed94d,// 3 BDA   1 

/// PAY Generic Data size   : 1358 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x80a85c9f,// 4 PAY   1 

    0x276f5b84,// 5 PAY   2 

    0x878e20fc,// 6 PAY   3 

    0xf550e6a4,// 7 PAY   4 

    0xf5257d67,// 8 PAY   5 

    0xee3cb45c,// 9 PAY   6 

    0x45556dfe,// 10 PAY   7 

    0xeaab626f,// 11 PAY   8 

    0xe71d6952,// 12 PAY   9 

    0x64cd800f,// 13 PAY  10 

    0x6f073dd9,// 14 PAY  11 

    0x26bea21c,// 15 PAY  12 

    0x1d4f85f5,// 16 PAY  13 

    0x5cc9c378,// 17 PAY  14 

    0x361e6303,// 18 PAY  15 

    0x7b06e1b6,// 19 PAY  16 

    0x68b7a622,// 20 PAY  17 

    0xe8dd9874,// 21 PAY  18 

    0x0d48613f,// 22 PAY  19 

    0x9e66f609,// 23 PAY  20 

    0x85b97fbb,// 24 PAY  21 

    0x2480578c,// 25 PAY  22 

    0xf6400d34,// 26 PAY  23 

    0x2c907c46,// 27 PAY  24 

    0x5ae590a0,// 28 PAY  25 

    0x56f9d37c,// 29 PAY  26 

    0x44eab228,// 30 PAY  27 

    0x96146c02,// 31 PAY  28 

    0xd2d9bb7f,// 32 PAY  29 

    0xe4b7ff48,// 33 PAY  30 

    0x30a1fa43,// 34 PAY  31 

    0x68be9e23,// 35 PAY  32 

    0xf22c5f31,// 36 PAY  33 

    0xcbc23661,// 37 PAY  34 

    0xb0e71d8b,// 38 PAY  35 

    0x47badd97,// 39 PAY  36 

    0x1a14352b,// 40 PAY  37 

    0x1e3f706f,// 41 PAY  38 

    0x13cc8319,// 42 PAY  39 

    0x4bb124c4,// 43 PAY  40 

    0x61d86d24,// 44 PAY  41 

    0x2c4b4609,// 45 PAY  42 

    0x606cef7b,// 46 PAY  43 

    0xb7555e55,// 47 PAY  44 

    0x49d4960f,// 48 PAY  45 

    0xbc96e1e8,// 49 PAY  46 

    0x0e04a4af,// 50 PAY  47 

    0xc3ad5710,// 51 PAY  48 

    0x98ba79f2,// 52 PAY  49 

    0x2871d6ab,// 53 PAY  50 

    0x5f61e958,// 54 PAY  51 

    0x97123220,// 55 PAY  52 

    0xbc289ad2,// 56 PAY  53 

    0x9a7f319c,// 57 PAY  54 

    0xc1334e87,// 58 PAY  55 

    0xd9ac9db1,// 59 PAY  56 

    0xc92ff2fd,// 60 PAY  57 

    0xa83c6885,// 61 PAY  58 

    0x9ac3bfff,// 62 PAY  59 

    0x8e59430e,// 63 PAY  60 

    0xbf6408c0,// 64 PAY  61 

    0x6d72f92c,// 65 PAY  62 

    0x0258c877,// 66 PAY  63 

    0x3c86d01b,// 67 PAY  64 

    0x08a37ea7,// 68 PAY  65 

    0x139a9a8e,// 69 PAY  66 

    0x2cc8d53f,// 70 PAY  67 

    0xda5a8977,// 71 PAY  68 

    0xcc8d8541,// 72 PAY  69 

    0x3ce4f134,// 73 PAY  70 

    0x7bd61429,// 74 PAY  71 

    0xe8b196a3,// 75 PAY  72 

    0x05686c4b,// 76 PAY  73 

    0x2c001687,// 77 PAY  74 

    0xd0127aed,// 78 PAY  75 

    0xffc91b7d,// 79 PAY  76 

    0xdcbebf86,// 80 PAY  77 

    0x778bf474,// 81 PAY  78 

    0x77e20314,// 82 PAY  79 

    0x57a0682d,// 83 PAY  80 

    0x2c2a633b,// 84 PAY  81 

    0x0d724f8a,// 85 PAY  82 

    0x954138f9,// 86 PAY  83 

    0x93f3919e,// 87 PAY  84 

    0x17664908,// 88 PAY  85 

    0x50ade8fc,// 89 PAY  86 

    0xbeb58501,// 90 PAY  87 

    0x6bb2dd6b,// 91 PAY  88 

    0x95f0bf98,// 92 PAY  89 

    0x44a3fb4e,// 93 PAY  90 

    0x84743fc3,// 94 PAY  91 

    0x33ac4318,// 95 PAY  92 

    0xa412d80e,// 96 PAY  93 

    0x1eeb149f,// 97 PAY  94 

    0x23d08d65,// 98 PAY  95 

    0xf999a8da,// 99 PAY  96 

    0x2219b6f9,// 100 PAY  97 

    0x5bdf5f04,// 101 PAY  98 

    0xb142fc9e,// 102 PAY  99 

    0x2c2ab981,// 103 PAY 100 

    0xbb249404,// 104 PAY 101 

    0x9ce7e52e,// 105 PAY 102 

    0xe97d0d52,// 106 PAY 103 

    0x0d2898b6,// 107 PAY 104 

    0xecb8f9cd,// 108 PAY 105 

    0x0b663c3d,// 109 PAY 106 

    0x96914e2d,// 110 PAY 107 

    0xd99eb864,// 111 PAY 108 

    0x6130f5b9,// 112 PAY 109 

    0xa8acd5ba,// 113 PAY 110 

    0xe2494725,// 114 PAY 111 

    0xebdd7988,// 115 PAY 112 

    0x21e7c24f,// 116 PAY 113 

    0x9d33c5b5,// 117 PAY 114 

    0x7af37487,// 118 PAY 115 

    0x33fdf9b4,// 119 PAY 116 

    0x7bd71a48,// 120 PAY 117 

    0xa175e1f9,// 121 PAY 118 

    0x2f3361b0,// 122 PAY 119 

    0x93805458,// 123 PAY 120 

    0xcef1aefa,// 124 PAY 121 

    0x3fe6c7a5,// 125 PAY 122 

    0xd1bc3b43,// 126 PAY 123 

    0xb4f5f17b,// 127 PAY 124 

    0xac121215,// 128 PAY 125 

    0x455525d2,// 129 PAY 126 

    0x60e86639,// 130 PAY 127 

    0x85eb36fd,// 131 PAY 128 

    0x0d18e706,// 132 PAY 129 

    0xe5a275df,// 133 PAY 130 

    0xa17cb3e4,// 134 PAY 131 

    0xdfa50618,// 135 PAY 132 

    0x6874b355,// 136 PAY 133 

    0x50b7d0e6,// 137 PAY 134 

    0xb4eaabd0,// 138 PAY 135 

    0xc323d384,// 139 PAY 136 

    0x5fc83bce,// 140 PAY 137 

    0xd4601b12,// 141 PAY 138 

    0x0aeb64ce,// 142 PAY 139 

    0xb88eb5b7,// 143 PAY 140 

    0xa1783972,// 144 PAY 141 

    0x7d0d5e31,// 145 PAY 142 

    0x13aec0d5,// 146 PAY 143 

    0xc0c45f37,// 147 PAY 144 

    0x42fc91c1,// 148 PAY 145 

    0x9089f60e,// 149 PAY 146 

    0x9fc4c98e,// 150 PAY 147 

    0x15ad4bf6,// 151 PAY 148 

    0x8e387833,// 152 PAY 149 

    0x634524e3,// 153 PAY 150 

    0x77b63bc8,// 154 PAY 151 

    0xc9689410,// 155 PAY 152 

    0x0819ae99,// 156 PAY 153 

    0x61b3f705,// 157 PAY 154 

    0x5ead1dee,// 158 PAY 155 

    0xa81d655b,// 159 PAY 156 

    0x5ab07438,// 160 PAY 157 

    0x3bca28ec,// 161 PAY 158 

    0xefe62751,// 162 PAY 159 

    0x786ac5f1,// 163 PAY 160 

    0x35a4134c,// 164 PAY 161 

    0xc6d6885a,// 165 PAY 162 

    0x417b00eb,// 166 PAY 163 

    0x711d8e3b,// 167 PAY 164 

    0xda803046,// 168 PAY 165 

    0xd8d70f14,// 169 PAY 166 

    0x8944503e,// 170 PAY 167 

    0xedeb06d8,// 171 PAY 168 

    0x752fb065,// 172 PAY 169 

    0xf1604427,// 173 PAY 170 

    0xe07e9773,// 174 PAY 171 

    0xf6f4a6a3,// 175 PAY 172 

    0x42da0b66,// 176 PAY 173 

    0xe3eecccd,// 177 PAY 174 

    0x1d65b8fe,// 178 PAY 175 

    0xe531be64,// 179 PAY 176 

    0x894adbdc,// 180 PAY 177 

    0x4bf587d0,// 181 PAY 178 

    0xb47b96ce,// 182 PAY 179 

    0x3f15dc76,// 183 PAY 180 

    0x2a91d866,// 184 PAY 181 

    0x9718c26e,// 185 PAY 182 

    0xdd9e1597,// 186 PAY 183 

    0x794d43f4,// 187 PAY 184 

    0xbc255614,// 188 PAY 185 

    0x4b20fc5f,// 189 PAY 186 

    0x6e8bb1b0,// 190 PAY 187 

    0x2a54ee6e,// 191 PAY 188 

    0x8c856024,// 192 PAY 189 

    0x739dcfd4,// 193 PAY 190 

    0xb883fe63,// 194 PAY 191 

    0x5414d02c,// 195 PAY 192 

    0x28b6c3b5,// 196 PAY 193 

    0xccf4a145,// 197 PAY 194 

    0x8a807be1,// 198 PAY 195 

    0xe92e154d,// 199 PAY 196 

    0x31ca2c41,// 200 PAY 197 

    0x4290d6b9,// 201 PAY 198 

    0xbd114d2f,// 202 PAY 199 

    0x84c57073,// 203 PAY 200 

    0xc0169233,// 204 PAY 201 

    0x9b1e6a15,// 205 PAY 202 

    0x87e83ed3,// 206 PAY 203 

    0xc510b5aa,// 207 PAY 204 

    0x258cfa2b,// 208 PAY 205 

    0xe0ac7429,// 209 PAY 206 

    0xe582231f,// 210 PAY 207 

    0x21a8f96e,// 211 PAY 208 

    0xf710f9e0,// 212 PAY 209 

    0x2451fcda,// 213 PAY 210 

    0x6babaae4,// 214 PAY 211 

    0x3e17c61b,// 215 PAY 212 

    0xc7e6e73b,// 216 PAY 213 

    0x258050c5,// 217 PAY 214 

    0x3eba078b,// 218 PAY 215 

    0x442d29ac,// 219 PAY 216 

    0x40fd5aaa,// 220 PAY 217 

    0xc4ffd7d5,// 221 PAY 218 

    0x8eeb4283,// 222 PAY 219 

    0x6e6397f1,// 223 PAY 220 

    0x10a7eead,// 224 PAY 221 

    0x643b100f,// 225 PAY 222 

    0xbb0ca9de,// 226 PAY 223 

    0x26bf9408,// 227 PAY 224 

    0xc230a81f,// 228 PAY 225 

    0xaccb0c79,// 229 PAY 226 

    0x657c9da7,// 230 PAY 227 

    0x22063d5b,// 231 PAY 228 

    0x520e5883,// 232 PAY 229 

    0xad68e984,// 233 PAY 230 

    0x40addc26,// 234 PAY 231 

    0xb77e5eeb,// 235 PAY 232 

    0x5653b835,// 236 PAY 233 

    0x3236d9d1,// 237 PAY 234 

    0xf67c250f,// 238 PAY 235 

    0xc03e3174,// 239 PAY 236 

    0x7fe5682b,// 240 PAY 237 

    0x24aa60cf,// 241 PAY 238 

    0x16106015,// 242 PAY 239 

    0xf3d3aeb6,// 243 PAY 240 

    0x8296357c,// 244 PAY 241 

    0xa54a07cb,// 245 PAY 242 

    0xb48a67b1,// 246 PAY 243 

    0x592e8bf1,// 247 PAY 244 

    0x8de5591b,// 248 PAY 245 

    0xb89c45e9,// 249 PAY 246 

    0xfa2a37c1,// 250 PAY 247 

    0x046b441d,// 251 PAY 248 

    0xc91d51ba,// 252 PAY 249 

    0x4c6a3a32,// 253 PAY 250 

    0x8bf6432d,// 254 PAY 251 

    0xe29bdc69,// 255 PAY 252 

    0x9b562369,// 256 PAY 253 

    0x9d7d96c5,// 257 PAY 254 

    0x3a94451e,// 258 PAY 255 

    0xec0e535b,// 259 PAY 256 

    0x87d4b167,// 260 PAY 257 

    0xd4aae65e,// 261 PAY 258 

    0x94605967,// 262 PAY 259 

    0x04caf13b,// 263 PAY 260 

    0x778fef62,// 264 PAY 261 

    0xe3875647,// 265 PAY 262 

    0x601cb81a,// 266 PAY 263 

    0x0e008442,// 267 PAY 264 

    0x532174ef,// 268 PAY 265 

    0x25f68b42,// 269 PAY 266 

    0x15a5bb1d,// 270 PAY 267 

    0xde3806c2,// 271 PAY 268 

    0x006b702c,// 272 PAY 269 

    0x4b95939e,// 273 PAY 270 

    0x77306e7d,// 274 PAY 271 

    0x6048872a,// 275 PAY 272 

    0x7b3da8a2,// 276 PAY 273 

    0x0fd27c68,// 277 PAY 274 

    0xf39d5249,// 278 PAY 275 

    0xf9806140,// 279 PAY 276 

    0x7be36b32,// 280 PAY 277 

    0x77648582,// 281 PAY 278 

    0xf093f0a8,// 282 PAY 279 

    0xb0052b90,// 283 PAY 280 

    0x6eab97a4,// 284 PAY 281 

    0x57213c9f,// 285 PAY 282 

    0x2e1e806a,// 286 PAY 283 

    0xecc717ea,// 287 PAY 284 

    0x90298046,// 288 PAY 285 

    0x532ead0f,// 289 PAY 286 

    0x0fe6785d,// 290 PAY 287 

    0x4727adf3,// 291 PAY 288 

    0xf019705d,// 292 PAY 289 

    0x971497e3,// 293 PAY 290 

    0x26cf341e,// 294 PAY 291 

    0xbac109c5,// 295 PAY 292 

    0x161660bb,// 296 PAY 293 

    0xed38a1c0,// 297 PAY 294 

    0xb1c055aa,// 298 PAY 295 

    0x642697e6,// 299 PAY 296 

    0x9b554def,// 300 PAY 297 

    0x24b87d70,// 301 PAY 298 

    0xa5342618,// 302 PAY 299 

    0xa4ddd5dd,// 303 PAY 300 

    0xb03500eb,// 304 PAY 301 

    0xd56d8ace,// 305 PAY 302 

    0x5a6f5f31,// 306 PAY 303 

    0xf80398dc,// 307 PAY 304 

    0xf9bedcdb,// 308 PAY 305 

    0x9913ea38,// 309 PAY 306 

    0xb643aa99,// 310 PAY 307 

    0xe142ff5a,// 311 PAY 308 

    0x5e1327b1,// 312 PAY 309 

    0xc63d8793,// 313 PAY 310 

    0x19d5b5ec,// 314 PAY 311 

    0xe30ecdd2,// 315 PAY 312 

    0x33bff272,// 316 PAY 313 

    0xd8adc24c,// 317 PAY 314 

    0x9cc12443,// 318 PAY 315 

    0xbc6cd675,// 319 PAY 316 

    0x07367a13,// 320 PAY 317 

    0x33019e29,// 321 PAY 318 

    0x0e77280b,// 322 PAY 319 

    0xd5aae949,// 323 PAY 320 

    0x6f3f4dae,// 324 PAY 321 

    0x7e303100,// 325 PAY 322 

    0x2a210080,// 326 PAY 323 

    0x9e82dac0,// 327 PAY 324 

    0x6543a507,// 328 PAY 325 

    0x726e6ebe,// 329 PAY 326 

    0xf9367437,// 330 PAY 327 

    0x3e840883,// 331 PAY 328 

    0x059e5d36,// 332 PAY 329 

    0xe54f0633,// 333 PAY 330 

    0xe8ebc922,// 334 PAY 331 

    0x9687bbdd,// 335 PAY 332 

    0x68906bd0,// 336 PAY 333 

    0x5a008c76,// 337 PAY 334 

    0x642f0c3a,// 338 PAY 335 

    0x63e2837c,// 339 PAY 336 

    0xbf4cbc90,// 340 PAY 337 

    0x64a0b67c,// 341 PAY 338 

    0xebb7c844,// 342 PAY 339 

    0x604c0000,// 343 PAY 340 

/// HASH is  16 bytes 

    0xe92e154d,// 344 HSH   1 

    0x31ca2c41,// 345 HSH   2 

    0x4290d6b9,// 346 HSH   3 

    0xbd114d2f,// 347 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 120 

/// STA pkt_idx        : 243 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdb 

    0x03ccdb78 // 348 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt41_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 307 words. 

/// BDA size     is 1223 (0x4c7) 

/// BDA id       is 0x4f88 

    0x04c74f88,// 3 BDA   1 

/// PAY Generic Data size   : 1223 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x18dd9747,// 4 PAY   1 

    0xbbb55591,// 5 PAY   2 

    0xa877095b,// 6 PAY   3 

    0xd99efd4b,// 7 PAY   4 

    0xb723de79,// 8 PAY   5 

    0x6bbe2c38,// 9 PAY   6 

    0xa2795b6e,// 10 PAY   7 

    0x54268456,// 11 PAY   8 

    0x3bbeeec4,// 12 PAY   9 

    0x576dc187,// 13 PAY  10 

    0xbec1b33b,// 14 PAY  11 

    0xfc60deb5,// 15 PAY  12 

    0x10db9cea,// 16 PAY  13 

    0xb051ffb4,// 17 PAY  14 

    0xf92ad2ba,// 18 PAY  15 

    0x0ba322bc,// 19 PAY  16 

    0xc09c8efa,// 20 PAY  17 

    0x26dfdb4f,// 21 PAY  18 

    0x511ca646,// 22 PAY  19 

    0x02682a50,// 23 PAY  20 

    0xdf384225,// 24 PAY  21 

    0xe023ad83,// 25 PAY  22 

    0xc074e05a,// 26 PAY  23 

    0xe44ce629,// 27 PAY  24 

    0xec93c8c6,// 28 PAY  25 

    0x3652071d,// 29 PAY  26 

    0xc716787d,// 30 PAY  27 

    0xf23dc50c,// 31 PAY  28 

    0x689e6019,// 32 PAY  29 

    0xd72f5cb2,// 33 PAY  30 

    0xa50a6d26,// 34 PAY  31 

    0xee061c66,// 35 PAY  32 

    0x3c8ca92b,// 36 PAY  33 

    0x4d3dd43e,// 37 PAY  34 

    0x002986f7,// 38 PAY  35 

    0xa6b1aede,// 39 PAY  36 

    0xe5f6d7c2,// 40 PAY  37 

    0xea4f5462,// 41 PAY  38 

    0xcf72396b,// 42 PAY  39 

    0x37f93601,// 43 PAY  40 

    0x41b2403f,// 44 PAY  41 

    0xe0c46ca7,// 45 PAY  42 

    0x152dca13,// 46 PAY  43 

    0x923f5607,// 47 PAY  44 

    0xd0788c64,// 48 PAY  45 

    0xb7c1abd5,// 49 PAY  46 

    0x260a0120,// 50 PAY  47 

    0x456ea3bc,// 51 PAY  48 

    0x4a75e76c,// 52 PAY  49 

    0x0160d1ef,// 53 PAY  50 

    0xe1760fd6,// 54 PAY  51 

    0x438f2e77,// 55 PAY  52 

    0x7c4f0b9e,// 56 PAY  53 

    0x07f9b000,// 57 PAY  54 

    0xa9e900d9,// 58 PAY  55 

    0xff0adaa0,// 59 PAY  56 

    0xee2462bd,// 60 PAY  57 

    0x01670ba3,// 61 PAY  58 

    0xd808ddf0,// 62 PAY  59 

    0x8cc747b1,// 63 PAY  60 

    0xd62a5ad9,// 64 PAY  61 

    0x65f48e10,// 65 PAY  62 

    0x8cda0f69,// 66 PAY  63 

    0x8394438d,// 67 PAY  64 

    0x167e8748,// 68 PAY  65 

    0x9e143d38,// 69 PAY  66 

    0x44320874,// 70 PAY  67 

    0xc4989101,// 71 PAY  68 

    0x80407274,// 72 PAY  69 

    0xfa275d21,// 73 PAY  70 

    0x1d34adb4,// 74 PAY  71 

    0x5e3c1f5a,// 75 PAY  72 

    0x09a78aa2,// 76 PAY  73 

    0x8b2b94e6,// 77 PAY  74 

    0x782cf788,// 78 PAY  75 

    0xe19dc1ff,// 79 PAY  76 

    0xeed6ae23,// 80 PAY  77 

    0x1d9ee1c0,// 81 PAY  78 

    0xb43afe3f,// 82 PAY  79 

    0x113c3716,// 83 PAY  80 

    0xafc947a5,// 84 PAY  81 

    0x710d8838,// 85 PAY  82 

    0x64627509,// 86 PAY  83 

    0x182bc904,// 87 PAY  84 

    0xbcf7789a,// 88 PAY  85 

    0x1fcaa353,// 89 PAY  86 

    0xc7e99fa1,// 90 PAY  87 

    0x4682ae95,// 91 PAY  88 

    0xbddc1d83,// 92 PAY  89 

    0x59be55f8,// 93 PAY  90 

    0xbf660c88,// 94 PAY  91 

    0x8652171b,// 95 PAY  92 

    0x52786486,// 96 PAY  93 

    0x2a9560fa,// 97 PAY  94 

    0xa1434cdc,// 98 PAY  95 

    0x05bfae01,// 99 PAY  96 

    0xb3214906,// 100 PAY  97 

    0x8e693628,// 101 PAY  98 

    0xe81a03a2,// 102 PAY  99 

    0xc92664b0,// 103 PAY 100 

    0x676c52d2,// 104 PAY 101 

    0x8d49e115,// 105 PAY 102 

    0x66d34b27,// 106 PAY 103 

    0x850bfc93,// 107 PAY 104 

    0x22456a6f,// 108 PAY 105 

    0xbf08056c,// 109 PAY 106 

    0xcb5ce283,// 110 PAY 107 

    0xb558911d,// 111 PAY 108 

    0x544d3bc9,// 112 PAY 109 

    0x3798aebd,// 113 PAY 110 

    0x716ea3e0,// 114 PAY 111 

    0x6eb71ce6,// 115 PAY 112 

    0x36e2fab2,// 116 PAY 113 

    0x9c3df7c2,// 117 PAY 114 

    0x6b0f0c73,// 118 PAY 115 

    0xdb8482c1,// 119 PAY 116 

    0x92f0e42b,// 120 PAY 117 

    0x612b4140,// 121 PAY 118 

    0x53919512,// 122 PAY 119 

    0x4e9c7af2,// 123 PAY 120 

    0x9a2f0cd0,// 124 PAY 121 

    0x2526884f,// 125 PAY 122 

    0x9d81a662,// 126 PAY 123 

    0x47c27c75,// 127 PAY 124 

    0x3fc5d88a,// 128 PAY 125 

    0xa250a548,// 129 PAY 126 

    0xa787ed20,// 130 PAY 127 

    0x1165426f,// 131 PAY 128 

    0x4f5ba9a4,// 132 PAY 129 

    0xb3ee680b,// 133 PAY 130 

    0x2e0e75ac,// 134 PAY 131 

    0x5d25aa18,// 135 PAY 132 

    0xcac41a9b,// 136 PAY 133 

    0xff17bd83,// 137 PAY 134 

    0x192ab16c,// 138 PAY 135 

    0x8a8a2ab4,// 139 PAY 136 

    0xe6403783,// 140 PAY 137 

    0x307ed9da,// 141 PAY 138 

    0xb764bdff,// 142 PAY 139 

    0x1ec88503,// 143 PAY 140 

    0xc738c475,// 144 PAY 141 

    0x4844556e,// 145 PAY 142 

    0x267ec033,// 146 PAY 143 

    0xc71bde43,// 147 PAY 144 

    0x0792e9be,// 148 PAY 145 

    0x8d79096c,// 149 PAY 146 

    0xa1177137,// 150 PAY 147 

    0x51eb8046,// 151 PAY 148 

    0x2e799601,// 152 PAY 149 

    0x8a364460,// 153 PAY 150 

    0x1a0c46a7,// 154 PAY 151 

    0x865d48ae,// 155 PAY 152 

    0xca0c963f,// 156 PAY 153 

    0x33dd778a,// 157 PAY 154 

    0x56814bb0,// 158 PAY 155 

    0xe5d7d9fb,// 159 PAY 156 

    0x9e76a00d,// 160 PAY 157 

    0x296dbe70,// 161 PAY 158 

    0xd666750b,// 162 PAY 159 

    0x339d7dda,// 163 PAY 160 

    0x0c6d0388,// 164 PAY 161 

    0x2342bda5,// 165 PAY 162 

    0x84fdf05e,// 166 PAY 163 

    0x5af58c5e,// 167 PAY 164 

    0x527623c6,// 168 PAY 165 

    0xfa3ba4b3,// 169 PAY 166 

    0x1cb3ee1a,// 170 PAY 167 

    0x9427d11c,// 171 PAY 168 

    0x0369f18f,// 172 PAY 169 

    0xc11b13f1,// 173 PAY 170 

    0x11463383,// 174 PAY 171 

    0x7a8c1ec0,// 175 PAY 172 

    0x01845b48,// 176 PAY 173 

    0x41432a8c,// 177 PAY 174 

    0xca8ee35a,// 178 PAY 175 

    0x2b403e3d,// 179 PAY 176 

    0xc04cbad2,// 180 PAY 177 

    0xddf2f23a,// 181 PAY 178 

    0xc8c6b509,// 182 PAY 179 

    0x584cf5a9,// 183 PAY 180 

    0x62d5ff31,// 184 PAY 181 

    0x902b7107,// 185 PAY 182 

    0x725c00a8,// 186 PAY 183 

    0x2caebb34,// 187 PAY 184 

    0xba885d71,// 188 PAY 185 

    0xc518ab1e,// 189 PAY 186 

    0x5bfda206,// 190 PAY 187 

    0xac37a2f3,// 191 PAY 188 

    0x9e6bfdac,// 192 PAY 189 

    0xeede11e1,// 193 PAY 190 

    0x9ffbe096,// 194 PAY 191 

    0x7e341510,// 195 PAY 192 

    0x432d5683,// 196 PAY 193 

    0xb1412b20,// 197 PAY 194 

    0xeae47077,// 198 PAY 195 

    0x8d758ae6,// 199 PAY 196 

    0x621fe272,// 200 PAY 197 

    0x0130a2c6,// 201 PAY 198 

    0x960519f0,// 202 PAY 199 

    0x0d2ec61d,// 203 PAY 200 

    0x541f3c45,// 204 PAY 201 

    0x481bea88,// 205 PAY 202 

    0x3ce1d5e8,// 206 PAY 203 

    0x6eb1dac2,// 207 PAY 204 

    0x0fc8acdd,// 208 PAY 205 

    0x0173dfb4,// 209 PAY 206 

    0xbeb2626e,// 210 PAY 207 

    0x137d3321,// 211 PAY 208 

    0xaec03736,// 212 PAY 209 

    0xc36f94d9,// 213 PAY 210 

    0x38d6fa83,// 214 PAY 211 

    0x25edc17d,// 215 PAY 212 

    0x0c71472c,// 216 PAY 213 

    0x2c5b5995,// 217 PAY 214 

    0xd3a466a9,// 218 PAY 215 

    0x4f6f4f35,// 219 PAY 216 

    0xf407e4ff,// 220 PAY 217 

    0x74885182,// 221 PAY 218 

    0xf0f1551f,// 222 PAY 219 

    0x72623b4c,// 223 PAY 220 

    0xa78316bc,// 224 PAY 221 

    0xefe78f5d,// 225 PAY 222 

    0xf906c10b,// 226 PAY 223 

    0x3ff91405,// 227 PAY 224 

    0x53c1f32d,// 228 PAY 225 

    0x5545eec6,// 229 PAY 226 

    0x413356dd,// 230 PAY 227 

    0xc34df7d2,// 231 PAY 228 

    0x37549441,// 232 PAY 229 

    0xf2ca6dd4,// 233 PAY 230 

    0xcecf9a2e,// 234 PAY 231 

    0xb9ea6a9f,// 235 PAY 232 

    0x7baa87d0,// 236 PAY 233 

    0x1f4500ca,// 237 PAY 234 

    0xac825ab6,// 238 PAY 235 

    0xe3ce1d67,// 239 PAY 236 

    0x0d2de97b,// 240 PAY 237 

    0xecc03175,// 241 PAY 238 

    0xe8f23816,// 242 PAY 239 

    0x77df86e0,// 243 PAY 240 

    0x29a07199,// 244 PAY 241 

    0xcf901a4d,// 245 PAY 242 

    0x64c9ea17,// 246 PAY 243 

    0x3463a083,// 247 PAY 244 

    0x2cc21879,// 248 PAY 245 

    0x75e392c0,// 249 PAY 246 

    0x7bd3f041,// 250 PAY 247 

    0x8dae4966,// 251 PAY 248 

    0x525cfa05,// 252 PAY 249 

    0xdde32ba2,// 253 PAY 250 

    0x45ddfb82,// 254 PAY 251 

    0x43887a02,// 255 PAY 252 

    0xb7c1c724,// 256 PAY 253 

    0x06c00c5e,// 257 PAY 254 

    0xc64b1476,// 258 PAY 255 

    0xcd67882c,// 259 PAY 256 

    0xe8615459,// 260 PAY 257 

    0x9d02dfa6,// 261 PAY 258 

    0x74312cd3,// 262 PAY 259 

    0x5bf9beca,// 263 PAY 260 

    0x88f3eb1b,// 264 PAY 261 

    0x5f3715e1,// 265 PAY 262 

    0xd8ab66c5,// 266 PAY 263 

    0xed2a6673,// 267 PAY 264 

    0xe9675c73,// 268 PAY 265 

    0x4556907f,// 269 PAY 266 

    0xa7a3e3c5,// 270 PAY 267 

    0xa9972339,// 271 PAY 268 

    0xc27018cf,// 272 PAY 269 

    0x5833f310,// 273 PAY 270 

    0x20b38052,// 274 PAY 271 

    0xab6e0a03,// 275 PAY 272 

    0x693e52bb,// 276 PAY 273 

    0x5fc633df,// 277 PAY 274 

    0xfcb98661,// 278 PAY 275 

    0x7401c29a,// 279 PAY 276 

    0xfbd51c17,// 280 PAY 277 

    0x27afdda9,// 281 PAY 278 

    0x414f1e14,// 282 PAY 279 

    0xc604ab4c,// 283 PAY 280 

    0x2027d280,// 284 PAY 281 

    0x0d3a5ed2,// 285 PAY 282 

    0xf7b34487,// 286 PAY 283 

    0x245942f2,// 287 PAY 284 

    0x4bd1eb86,// 288 PAY 285 

    0x7ac9d62c,// 289 PAY 286 

    0x12690717,// 290 PAY 287 

    0x0e36cc46,// 291 PAY 288 

    0xdaeb51d0,// 292 PAY 289 

    0x4c7c4191,// 293 PAY 290 

    0xb5bd8564,// 294 PAY 291 

    0x15235b5b,// 295 PAY 292 

    0x609cea6f,// 296 PAY 293 

    0xf0d5142f,// 297 PAY 294 

    0x3c4b79fb,// 298 PAY 295 

    0x4154c94f,// 299 PAY 296 

    0x8d057954,// 300 PAY 297 

    0x620d3c16,// 301 PAY 298 

    0x2cd13d33,// 302 PAY 299 

    0x86c54522,// 303 PAY 300 

    0x00d1df9d,// 304 PAY 301 

    0x68fd9ca0,// 305 PAY 302 

    0x32c8461f,// 306 PAY 303 

    0xdde4e27c,// 307 PAY 304 

    0x42303e6e,// 308 PAY 305 

    0x4e16ab00,// 309 PAY 306 

/// STA is 1 words. 

/// STA num_pkts       : 154 

/// STA pkt_idx        : 101 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4a 

    0x01954a9a // 310 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt42_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 245 words. 

/// BDA size     is 975 (0x3cf) 

/// BDA id       is 0x80ec 

    0x03cf80ec,// 3 BDA   1 

/// PAY Generic Data size   : 975 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x17e87c65,// 4 PAY   1 

    0xca51282b,// 5 PAY   2 

    0x3361ddd2,// 6 PAY   3 

    0xb6d9d6a1,// 7 PAY   4 

    0x5af49c0c,// 8 PAY   5 

    0xe2085d19,// 9 PAY   6 

    0xb744a765,// 10 PAY   7 

    0xbb243209,// 11 PAY   8 

    0x17cd9707,// 12 PAY   9 

    0x881b1090,// 13 PAY  10 

    0x3d3f5fbf,// 14 PAY  11 

    0x6dfe0fcd,// 15 PAY  12 

    0xe186b723,// 16 PAY  13 

    0xd2629e1d,// 17 PAY  14 

    0x2c5b7403,// 18 PAY  15 

    0x1b36ab32,// 19 PAY  16 

    0x4b1ad987,// 20 PAY  17 

    0x61d56fdf,// 21 PAY  18 

    0xf1655abb,// 22 PAY  19 

    0x469f91c7,// 23 PAY  20 

    0x6c97f9f7,// 24 PAY  21 

    0x65a06773,// 25 PAY  22 

    0xa65bea29,// 26 PAY  23 

    0x5d0db645,// 27 PAY  24 

    0x710d01c1,// 28 PAY  25 

    0x34e0f726,// 29 PAY  26 

    0x5413c8a3,// 30 PAY  27 

    0x6373edf8,// 31 PAY  28 

    0x9652512b,// 32 PAY  29 

    0xb7a13f8c,// 33 PAY  30 

    0xd0ba224f,// 34 PAY  31 

    0x083f1b04,// 35 PAY  32 

    0xdff404bf,// 36 PAY  33 

    0x4a9e939f,// 37 PAY  34 

    0x5f433221,// 38 PAY  35 

    0x3c75908b,// 39 PAY  36 

    0x1fac658a,// 40 PAY  37 

    0x86a9eb14,// 41 PAY  38 

    0xdebec7d1,// 42 PAY  39 

    0x9f6544d9,// 43 PAY  40 

    0x1abd1a67,// 44 PAY  41 

    0xc44cc2b5,// 45 PAY  42 

    0xa4f43d7b,// 46 PAY  43 

    0x58796bb1,// 47 PAY  44 

    0x7867703e,// 48 PAY  45 

    0x7c9daa41,// 49 PAY  46 

    0x049e1416,// 50 PAY  47 

    0xf96519ba,// 51 PAY  48 

    0xc94e14a4,// 52 PAY  49 

    0xe576a265,// 53 PAY  50 

    0x09b2f528,// 54 PAY  51 

    0xceeb7491,// 55 PAY  52 

    0x4a3b7b65,// 56 PAY  53 

    0xb6d6d46c,// 57 PAY  54 

    0x7e1dc3df,// 58 PAY  55 

    0x25533576,// 59 PAY  56 

    0xafb062dd,// 60 PAY  57 

    0xbb3c19d7,// 61 PAY  58 

    0x2cff5cc1,// 62 PAY  59 

    0x202bde3e,// 63 PAY  60 

    0x40d73925,// 64 PAY  61 

    0xff283d53,// 65 PAY  62 

    0xdb243e67,// 66 PAY  63 

    0x9f06267c,// 67 PAY  64 

    0x4ccc3fc8,// 68 PAY  65 

    0x326ab19b,// 69 PAY  66 

    0x209bea7b,// 70 PAY  67 

    0x7bec463d,// 71 PAY  68 

    0x3227de10,// 72 PAY  69 

    0xef216294,// 73 PAY  70 

    0xd02d44c8,// 74 PAY  71 

    0x5d4cc0be,// 75 PAY  72 

    0x0e11e5e0,// 76 PAY  73 

    0xccedc806,// 77 PAY  74 

    0xa4d0fa38,// 78 PAY  75 

    0x72461ac1,// 79 PAY  76 

    0xede0952e,// 80 PAY  77 

    0x7c275b7a,// 81 PAY  78 

    0xbb90f259,// 82 PAY  79 

    0x4fd48fb6,// 83 PAY  80 

    0xd29c2766,// 84 PAY  81 

    0x9e96c62c,// 85 PAY  82 

    0x883f79fc,// 86 PAY  83 

    0xa1d23c3d,// 87 PAY  84 

    0x817cbfde,// 88 PAY  85 

    0x1c845d54,// 89 PAY  86 

    0xbecce8d0,// 90 PAY  87 

    0xdb5fbf8e,// 91 PAY  88 

    0x5f63e806,// 92 PAY  89 

    0xe49eb55e,// 93 PAY  90 

    0x2cc57413,// 94 PAY  91 

    0x10581d3e,// 95 PAY  92 

    0x3849b402,// 96 PAY  93 

    0x3af647c1,// 97 PAY  94 

    0x3c096653,// 98 PAY  95 

    0x16037a93,// 99 PAY  96 

    0xe149ad74,// 100 PAY  97 

    0xbad2f089,// 101 PAY  98 

    0x962fe0ad,// 102 PAY  99 

    0x914cf73c,// 103 PAY 100 

    0x073bf0fa,// 104 PAY 101 

    0x9780d676,// 105 PAY 102 

    0x0bf921e9,// 106 PAY 103 

    0xfac2a4e2,// 107 PAY 104 

    0x6a04471a,// 108 PAY 105 

    0xcbd9d2f0,// 109 PAY 106 

    0xcd80aea6,// 110 PAY 107 

    0x40a73a80,// 111 PAY 108 

    0xe3c7c2e3,// 112 PAY 109 

    0xc75437e0,// 113 PAY 110 

    0x10eb87a1,// 114 PAY 111 

    0x92e6c8f8,// 115 PAY 112 

    0xc5641d2f,// 116 PAY 113 

    0x7ec1e619,// 117 PAY 114 

    0xcd58dd43,// 118 PAY 115 

    0x6c4ecac0,// 119 PAY 116 

    0x45443ab8,// 120 PAY 117 

    0x7e442e69,// 121 PAY 118 

    0x8b7c0d29,// 122 PAY 119 

    0x5be789c0,// 123 PAY 120 

    0xb30594ee,// 124 PAY 121 

    0x20af2fe2,// 125 PAY 122 

    0xde8d91df,// 126 PAY 123 

    0xea6f91d3,// 127 PAY 124 

    0xbed7c7df,// 128 PAY 125 

    0x43866352,// 129 PAY 126 

    0xf167b441,// 130 PAY 127 

    0xa40beed0,// 131 PAY 128 

    0xfa188060,// 132 PAY 129 

    0x12a8836d,// 133 PAY 130 

    0x72b18ea7,// 134 PAY 131 

    0x63a1cc8c,// 135 PAY 132 

    0x44956b7b,// 136 PAY 133 

    0x7f61cfa2,// 137 PAY 134 

    0x6e55d117,// 138 PAY 135 

    0xa670e24e,// 139 PAY 136 

    0xb88f52e7,// 140 PAY 137 

    0xc64b5157,// 141 PAY 138 

    0x8a741c27,// 142 PAY 139 

    0x43e2ab2c,// 143 PAY 140 

    0x4a1e7c7b,// 144 PAY 141 

    0xbb975636,// 145 PAY 142 

    0xe6157ef9,// 146 PAY 143 

    0x52601b8b,// 147 PAY 144 

    0x3d3d2152,// 148 PAY 145 

    0xdbb36e79,// 149 PAY 146 

    0x9a15e2cf,// 150 PAY 147 

    0xa1924d24,// 151 PAY 148 

    0xbc38f421,// 152 PAY 149 

    0xe1bda536,// 153 PAY 150 

    0x37a4635b,// 154 PAY 151 

    0x9ac26209,// 155 PAY 152 

    0x0acfcd17,// 156 PAY 153 

    0x72b0e244,// 157 PAY 154 

    0x247bd9f8,// 158 PAY 155 

    0x02046fb1,// 159 PAY 156 

    0x3e7f4e3a,// 160 PAY 157 

    0x04b4ff1a,// 161 PAY 158 

    0xaec2ad58,// 162 PAY 159 

    0x019a6502,// 163 PAY 160 

    0x47da2f72,// 164 PAY 161 

    0xbe2f094a,// 165 PAY 162 

    0xe8884679,// 166 PAY 163 

    0xb1add602,// 167 PAY 164 

    0xe3d4e7f2,// 168 PAY 165 

    0xfad9c254,// 169 PAY 166 

    0xe694fa0a,// 170 PAY 167 

    0x9718f5f1,// 171 PAY 168 

    0xfa088205,// 172 PAY 169 

    0xe728b4a5,// 173 PAY 170 

    0xc2fa6277,// 174 PAY 171 

    0xe23a7801,// 175 PAY 172 

    0x5fd9c1d3,// 176 PAY 173 

    0xfec01d65,// 177 PAY 174 

    0x61ac968b,// 178 PAY 175 

    0xbd3b7124,// 179 PAY 176 

    0xbc3ca63b,// 180 PAY 177 

    0x1cde94a2,// 181 PAY 178 

    0x1e6cc1af,// 182 PAY 179 

    0xaa9f83ef,// 183 PAY 180 

    0x84870518,// 184 PAY 181 

    0xdad722ea,// 185 PAY 182 

    0xbdcb82ea,// 186 PAY 183 

    0x3e4e8520,// 187 PAY 184 

    0x0a745ccf,// 188 PAY 185 

    0xf99c8dc2,// 189 PAY 186 

    0x674ff198,// 190 PAY 187 

    0xd59988b2,// 191 PAY 188 

    0x25edd6d4,// 192 PAY 189 

    0x6838c614,// 193 PAY 190 

    0x46bcd362,// 194 PAY 191 

    0x93107eb4,// 195 PAY 192 

    0x7e4b4fb3,// 196 PAY 193 

    0x799b764f,// 197 PAY 194 

    0xb0f62e5c,// 198 PAY 195 

    0xff24a02a,// 199 PAY 196 

    0xf2cbdd45,// 200 PAY 197 

    0xdc045633,// 201 PAY 198 

    0x88cb5392,// 202 PAY 199 

    0x381966d8,// 203 PAY 200 

    0xc574bbfe,// 204 PAY 201 

    0x8316d412,// 205 PAY 202 

    0xfaa37800,// 206 PAY 203 

    0x6e13d476,// 207 PAY 204 

    0x42f1faba,// 208 PAY 205 

    0x300dd771,// 209 PAY 206 

    0xd6ed5a4a,// 210 PAY 207 

    0xa5264275,// 211 PAY 208 

    0x908772db,// 212 PAY 209 

    0x9f8a2c49,// 213 PAY 210 

    0x4c8ffbe3,// 214 PAY 211 

    0x04fc8531,// 215 PAY 212 

    0x7f98fa2c,// 216 PAY 213 

    0x4e6721ca,// 217 PAY 214 

    0x5177aa72,// 218 PAY 215 

    0x6d43639e,// 219 PAY 216 

    0x6d65d5b7,// 220 PAY 217 

    0x60f9d37d,// 221 PAY 218 

    0x5dffbb15,// 222 PAY 219 

    0x8f3ed038,// 223 PAY 220 

    0x2aecb1b8,// 224 PAY 221 

    0xc8491a07,// 225 PAY 222 

    0x9cd241cd,// 226 PAY 223 

    0xf511f535,// 227 PAY 224 

    0xe1296f44,// 228 PAY 225 

    0x05db55e2,// 229 PAY 226 

    0x7da70dcb,// 230 PAY 227 

    0x1ba2ec9d,// 231 PAY 228 

    0x09a013f9,// 232 PAY 229 

    0x542abb8a,// 233 PAY 230 

    0x214241fd,// 234 PAY 231 

    0x48693cd8,// 235 PAY 232 

    0x0b504da2,// 236 PAY 233 

    0xd21b4ec0,// 237 PAY 234 

    0xd0d50ae0,// 238 PAY 235 

    0xac183eba,// 239 PAY 236 

    0x54c014fc,// 240 PAY 237 

    0x3c6b374a,// 241 PAY 238 

    0x13fd6b32,// 242 PAY 239 

    0x9a38635c,// 243 PAY 240 

    0x095a7532,// 244 PAY 241 

    0x017b595d,// 245 PAY 242 

    0x3b4691ea,// 246 PAY 243 

    0x619a2800,// 247 PAY 244 

/// HASH is  12 bytes 

    0xf511f535,// 248 HSH   1 

    0xe1296f44,// 249 HSH   2 

    0x05db55e2,// 250 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 54 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8b 

    0x00d98b2d // 251 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt43_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 127 words. 

/// BDA size     is 501 (0x1f5) 

/// BDA id       is 0x6967 

    0x01f56967,// 3 BDA   1 

/// PAY Generic Data size   : 501 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x2d849fd4,// 4 PAY   1 

    0xc7e01521,// 5 PAY   2 

    0xd10239d9,// 6 PAY   3 

    0x3160419a,// 7 PAY   4 

    0x0297ddec,// 8 PAY   5 

    0xb87ba183,// 9 PAY   6 

    0x40a1465c,// 10 PAY   7 

    0x27ad28df,// 11 PAY   8 

    0x2ab27ecc,// 12 PAY   9 

    0xa408956f,// 13 PAY  10 

    0x428cd883,// 14 PAY  11 

    0x479e7175,// 15 PAY  12 

    0x35899e3d,// 16 PAY  13 

    0x37768a0e,// 17 PAY  14 

    0x7cb0e53b,// 18 PAY  15 

    0x12cc6ff5,// 19 PAY  16 

    0xe1bbd635,// 20 PAY  17 

    0xbd88de36,// 21 PAY  18 

    0x4f54d585,// 22 PAY  19 

    0x270d4dc7,// 23 PAY  20 

    0x0fb295dd,// 24 PAY  21 

    0x266ef1ab,// 25 PAY  22 

    0xb4eed800,// 26 PAY  23 

    0xcaadb61a,// 27 PAY  24 

    0x45be5225,// 28 PAY  25 

    0x99a73398,// 29 PAY  26 

    0x196ed03a,// 30 PAY  27 

    0xfd7f23b6,// 31 PAY  28 

    0x589a9b37,// 32 PAY  29 

    0x6c430a78,// 33 PAY  30 

    0x43061ea9,// 34 PAY  31 

    0x7b739cb2,// 35 PAY  32 

    0x96992abb,// 36 PAY  33 

    0x65002627,// 37 PAY  34 

    0xef2039f9,// 38 PAY  35 

    0x7703fbee,// 39 PAY  36 

    0x7fcfa321,// 40 PAY  37 

    0x978d8222,// 41 PAY  38 

    0x4399d677,// 42 PAY  39 

    0x64539cf9,// 43 PAY  40 

    0x654634cb,// 44 PAY  41 

    0x3f38ce25,// 45 PAY  42 

    0x95c877a5,// 46 PAY  43 

    0x59703ce9,// 47 PAY  44 

    0xf722668f,// 48 PAY  45 

    0xbab23149,// 49 PAY  46 

    0xe77a46d4,// 50 PAY  47 

    0xc4c13441,// 51 PAY  48 

    0x5e850648,// 52 PAY  49 

    0x05df9b12,// 53 PAY  50 

    0xb109823c,// 54 PAY  51 

    0x33def586,// 55 PAY  52 

    0x4af8f8e4,// 56 PAY  53 

    0xa4fabcc1,// 57 PAY  54 

    0xa968949c,// 58 PAY  55 

    0x8b8f8443,// 59 PAY  56 

    0x8a6b4933,// 60 PAY  57 

    0xa95d84f7,// 61 PAY  58 

    0x35abfb52,// 62 PAY  59 

    0xe4ed1f03,// 63 PAY  60 

    0xecb9dad4,// 64 PAY  61 

    0xce1db557,// 65 PAY  62 

    0xf6fff7d3,// 66 PAY  63 

    0xdbf8dc80,// 67 PAY  64 

    0xc6c7883f,// 68 PAY  65 

    0xe9e5ea18,// 69 PAY  66 

    0x14205f28,// 70 PAY  67 

    0x0d6e806c,// 71 PAY  68 

    0x29f4d671,// 72 PAY  69 

    0x6c97d8b9,// 73 PAY  70 

    0xc9043cfd,// 74 PAY  71 

    0xb2c63235,// 75 PAY  72 

    0xd882f1fb,// 76 PAY  73 

    0x43995437,// 77 PAY  74 

    0x7589fb0a,// 78 PAY  75 

    0x0c299c47,// 79 PAY  76 

    0xa9a98801,// 80 PAY  77 

    0x5daaa565,// 81 PAY  78 

    0x3269121f,// 82 PAY  79 

    0xc04b6afc,// 83 PAY  80 

    0x289e3273,// 84 PAY  81 

    0x90fd715e,// 85 PAY  82 

    0xcb16247d,// 86 PAY  83 

    0x789cb769,// 87 PAY  84 

    0x15d83b09,// 88 PAY  85 

    0x39d94c6c,// 89 PAY  86 

    0x0302fc82,// 90 PAY  87 

    0x295fc49f,// 91 PAY  88 

    0xef2d6b35,// 92 PAY  89 

    0x8a94f163,// 93 PAY  90 

    0xf3c4f1b7,// 94 PAY  91 

    0x1cfa812f,// 95 PAY  92 

    0x1c7caafb,// 96 PAY  93 

    0xd9c60e22,// 97 PAY  94 

    0xc53d5526,// 98 PAY  95 

    0x35a12ebd,// 99 PAY  96 

    0x98d6a0dc,// 100 PAY  97 

    0x7c26ad3a,// 101 PAY  98 

    0xf7a31d39,// 102 PAY  99 

    0x8ab07f0a,// 103 PAY 100 

    0xa8d38cb2,// 104 PAY 101 

    0xd44456e6,// 105 PAY 102 

    0x1478cb9f,// 106 PAY 103 

    0xf1b4e491,// 107 PAY 104 

    0xa170a484,// 108 PAY 105 

    0xd6a1bfb8,// 109 PAY 106 

    0xe9f1efe4,// 110 PAY 107 

    0x5ed7e966,// 111 PAY 108 

    0x06e51c2b,// 112 PAY 109 

    0xcb807729,// 113 PAY 110 

    0x9da68b95,// 114 PAY 111 

    0x5f28659b,// 115 PAY 112 

    0xca21740a,// 116 PAY 113 

    0xf9fc624d,// 117 PAY 114 

    0x74c470c2,// 118 PAY 115 

    0x0bcdbffb,// 119 PAY 116 

    0xb2c80166,// 120 PAY 117 

    0xc7a905fc,// 121 PAY 118 

    0x38196ee2,// 122 PAY 119 

    0x6c606c0f,// 123 PAY 120 

    0xe29136d6,// 124 PAY 121 

    0x18ced30c,// 125 PAY 122 

    0xb319beab,// 126 PAY 123 

    0xe9b48597,// 127 PAY 124 

    0x25638b81,// 128 PAY 125 

    0x76000000,// 129 PAY 126 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 232 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x79 

    0x03a07985 // 130 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt44_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 376 words. 

/// BDA size     is 1499 (0x5db) 

/// BDA id       is 0x4bfd 

    0x05db4bfd,// 3 BDA   1 

/// PAY Generic Data size   : 1499 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x21bb0df0,// 4 PAY   1 

    0xea141529,// 5 PAY   2 

    0x42f7284a,// 6 PAY   3 

    0x3ebcf7fd,// 7 PAY   4 

    0xe00bcbe2,// 8 PAY   5 

    0x302aea4b,// 9 PAY   6 

    0x4668000b,// 10 PAY   7 

    0xc1bbdad3,// 11 PAY   8 

    0xa01763a4,// 12 PAY   9 

    0x17cb745f,// 13 PAY  10 

    0x4599fefe,// 14 PAY  11 

    0xca472bc0,// 15 PAY  12 

    0x0df09fd4,// 16 PAY  13 

    0x9a20e16c,// 17 PAY  14 

    0xba5b95e1,// 18 PAY  15 

    0x38bb8733,// 19 PAY  16 

    0x5311f613,// 20 PAY  17 

    0x0723b45e,// 21 PAY  18 

    0x1814788d,// 22 PAY  19 

    0xb2ec8525,// 23 PAY  20 

    0xb0759549,// 24 PAY  21 

    0xc2f6a0d8,// 25 PAY  22 

    0x4e083d3e,// 26 PAY  23 

    0x26919160,// 27 PAY  24 

    0x750811d2,// 28 PAY  25 

    0x53b9c83f,// 29 PAY  26 

    0x742b1718,// 30 PAY  27 

    0xe38bef83,// 31 PAY  28 

    0x3287fe1f,// 32 PAY  29 

    0x681270bf,// 33 PAY  30 

    0x994d5a1a,// 34 PAY  31 

    0xfc258e25,// 35 PAY  32 

    0x48ba298a,// 36 PAY  33 

    0x63374049,// 37 PAY  34 

    0xa529e83c,// 38 PAY  35 

    0x381b1d05,// 39 PAY  36 

    0x5fe36c0d,// 40 PAY  37 

    0x8f8691fb,// 41 PAY  38 

    0x0b392e2a,// 42 PAY  39 

    0x136b45a1,// 43 PAY  40 

    0xec65e1a6,// 44 PAY  41 

    0xf587bce4,// 45 PAY  42 

    0x57215e56,// 46 PAY  43 

    0x90e577a6,// 47 PAY  44 

    0x7d53e58b,// 48 PAY  45 

    0x9e44ea38,// 49 PAY  46 

    0x4163ab41,// 50 PAY  47 

    0xd39507fe,// 51 PAY  48 

    0xb167b545,// 52 PAY  49 

    0x1eb7822f,// 53 PAY  50 

    0x08613286,// 54 PAY  51 

    0x62dbb3e2,// 55 PAY  52 

    0x9895f31b,// 56 PAY  53 

    0x5bff0f6a,// 57 PAY  54 

    0x3237822e,// 58 PAY  55 

    0xf6ad8234,// 59 PAY  56 

    0xf9390c0e,// 60 PAY  57 

    0x2e5929c7,// 61 PAY  58 

    0x0b132c26,// 62 PAY  59 

    0x15fa5976,// 63 PAY  60 

    0x5752610a,// 64 PAY  61 

    0xd2cd57fe,// 65 PAY  62 

    0xb738e330,// 66 PAY  63 

    0xe3f1c510,// 67 PAY  64 

    0xb09b8c53,// 68 PAY  65 

    0x6989f0c2,// 69 PAY  66 

    0x42ed2841,// 70 PAY  67 

    0x9513d725,// 71 PAY  68 

    0x8f079ebb,// 72 PAY  69 

    0x70c18484,// 73 PAY  70 

    0xe3a4f221,// 74 PAY  71 

    0xcc7b35fd,// 75 PAY  72 

    0xdfa4be44,// 76 PAY  73 

    0xf2e2ef8b,// 77 PAY  74 

    0xd01a7da5,// 78 PAY  75 

    0x9f863055,// 79 PAY  76 

    0x27974d79,// 80 PAY  77 

    0xe39d03ba,// 81 PAY  78 

    0xadd08424,// 82 PAY  79 

    0x3eb74235,// 83 PAY  80 

    0x94931db9,// 84 PAY  81 

    0xebe1fb9c,// 85 PAY  82 

    0x51564a33,// 86 PAY  83 

    0x4f1e8f10,// 87 PAY  84 

    0x9adb6729,// 88 PAY  85 

    0x2c1855a0,// 89 PAY  86 

    0x4dd16a2f,// 90 PAY  87 

    0xc0cf4c01,// 91 PAY  88 

    0x6402f952,// 92 PAY  89 

    0x2d340626,// 93 PAY  90 

    0x2be55ce9,// 94 PAY  91 

    0x1d5fc871,// 95 PAY  92 

    0x6726291a,// 96 PAY  93 

    0x35d575db,// 97 PAY  94 

    0xf21c329e,// 98 PAY  95 

    0x9eef0a64,// 99 PAY  96 

    0x97ad4b69,// 100 PAY  97 

    0xa1b98fad,// 101 PAY  98 

    0xf5df7cbf,// 102 PAY  99 

    0x5db329b8,// 103 PAY 100 

    0x8a747a86,// 104 PAY 101 

    0xb89bec9c,// 105 PAY 102 

    0x8a8bb98c,// 106 PAY 103 

    0xc125cfff,// 107 PAY 104 

    0x294a8136,// 108 PAY 105 

    0x7acdc088,// 109 PAY 106 

    0x8b9661e9,// 110 PAY 107 

    0x156417a2,// 111 PAY 108 

    0x40d0cd7c,// 112 PAY 109 

    0x8189e0fb,// 113 PAY 110 

    0xb1899ef9,// 114 PAY 111 

    0xa5932e72,// 115 PAY 112 

    0xe7752aff,// 116 PAY 113 

    0x25c40b05,// 117 PAY 114 

    0xa63ea391,// 118 PAY 115 

    0x1b3c79a0,// 119 PAY 116 

    0x932e3725,// 120 PAY 117 

    0x8685bf70,// 121 PAY 118 

    0x3731ebad,// 122 PAY 119 

    0x2936c00d,// 123 PAY 120 

    0x84b0093c,// 124 PAY 121 

    0xc8b1bc37,// 125 PAY 122 

    0x110f6eab,// 126 PAY 123 

    0x4d412888,// 127 PAY 124 

    0x5975b42f,// 128 PAY 125 

    0x1cdee017,// 129 PAY 126 

    0x1a5dcb44,// 130 PAY 127 

    0x2bd76c9c,// 131 PAY 128 

    0x495bdf76,// 132 PAY 129 

    0x0b149227,// 133 PAY 130 

    0xae77aa36,// 134 PAY 131 

    0x882c0131,// 135 PAY 132 

    0x126a121c,// 136 PAY 133 

    0xa70b694e,// 137 PAY 134 

    0x6301bca2,// 138 PAY 135 

    0xf6f9a14f,// 139 PAY 136 

    0x2e0ab08b,// 140 PAY 137 

    0x1b641bb8,// 141 PAY 138 

    0x01fbc37a,// 142 PAY 139 

    0xb33d3c71,// 143 PAY 140 

    0xd205f558,// 144 PAY 141 

    0xce209f72,// 145 PAY 142 

    0x147a4422,// 146 PAY 143 

    0xd1b39622,// 147 PAY 144 

    0xc2997742,// 148 PAY 145 

    0x7deaeaea,// 149 PAY 146 

    0x4fbdcad0,// 150 PAY 147 

    0xf24f8dd2,// 151 PAY 148 

    0x5f573eda,// 152 PAY 149 

    0x24c41e45,// 153 PAY 150 

    0x3343fdd1,// 154 PAY 151 

    0x43216072,// 155 PAY 152 

    0x7473ffa2,// 156 PAY 153 

    0x02f778f5,// 157 PAY 154 

    0x39668b08,// 158 PAY 155 

    0x129c82de,// 159 PAY 156 

    0x429d2373,// 160 PAY 157 

    0xd9b2a84d,// 161 PAY 158 

    0x30d3147d,// 162 PAY 159 

    0x3b4fbba4,// 163 PAY 160 

    0x08df8703,// 164 PAY 161 

    0xd41f69ff,// 165 PAY 162 

    0xe851f8f9,// 166 PAY 163 

    0x6324a3a6,// 167 PAY 164 

    0x10f4a04b,// 168 PAY 165 

    0xd362e932,// 169 PAY 166 

    0x3e701a4c,// 170 PAY 167 

    0x2e054279,// 171 PAY 168 

    0xbde66965,// 172 PAY 169 

    0xfe189f92,// 173 PAY 170 

    0x10374f3e,// 174 PAY 171 

    0xd37c0295,// 175 PAY 172 

    0xe8cde467,// 176 PAY 173 

    0xb2f16f0b,// 177 PAY 174 

    0x5c472bbd,// 178 PAY 175 

    0x75dab419,// 179 PAY 176 

    0x1f6be4db,// 180 PAY 177 

    0x4c4d435e,// 181 PAY 178 

    0xce797b2a,// 182 PAY 179 

    0xcf0835b6,// 183 PAY 180 

    0x408b4db0,// 184 PAY 181 

    0x65476bc0,// 185 PAY 182 

    0xca9617d4,// 186 PAY 183 

    0xb2fdf489,// 187 PAY 184 

    0x974b4e70,// 188 PAY 185 

    0x9f9871f6,// 189 PAY 186 

    0xe530fcf2,// 190 PAY 187 

    0x2b6d3af8,// 191 PAY 188 

    0x71615d53,// 192 PAY 189 

    0x3ebddce5,// 193 PAY 190 

    0x3db1272f,// 194 PAY 191 

    0x980e882a,// 195 PAY 192 

    0xce3e0a55,// 196 PAY 193 

    0xbbb8c9ab,// 197 PAY 194 

    0xe753984a,// 198 PAY 195 

    0x44caae51,// 199 PAY 196 

    0x1ce02ffd,// 200 PAY 197 

    0x276402d3,// 201 PAY 198 

    0x5c7602f6,// 202 PAY 199 

    0x4cac57a9,// 203 PAY 200 

    0x76e956d8,// 204 PAY 201 

    0xcebe5135,// 205 PAY 202 

    0xe6bc8ad9,// 206 PAY 203 

    0xddecfb90,// 207 PAY 204 

    0x4e277179,// 208 PAY 205 

    0x4eeb03e6,// 209 PAY 206 

    0xbebc5caa,// 210 PAY 207 

    0x2de4b81c,// 211 PAY 208 

    0xc04315c5,// 212 PAY 209 

    0x05354e84,// 213 PAY 210 

    0x4aaca03b,// 214 PAY 211 

    0xdda241af,// 215 PAY 212 

    0x51a8705c,// 216 PAY 213 

    0x2ffecc7b,// 217 PAY 214 

    0xf6fdcda1,// 218 PAY 215 

    0x7fe34360,// 219 PAY 216 

    0x056dda1b,// 220 PAY 217 

    0x716e3c4a,// 221 PAY 218 

    0x9bf03720,// 222 PAY 219 

    0xea017123,// 223 PAY 220 

    0x03f2040e,// 224 PAY 221 

    0x3783f20e,// 225 PAY 222 

    0x9366163d,// 226 PAY 223 

    0x04f01d41,// 227 PAY 224 

    0xfe8e08f0,// 228 PAY 225 

    0xc4119f23,// 229 PAY 226 

    0x9e2323e7,// 230 PAY 227 

    0x5e58fb55,// 231 PAY 228 

    0x7d4b75e9,// 232 PAY 229 

    0xa1251a3c,// 233 PAY 230 

    0x9dfee953,// 234 PAY 231 

    0x42a49201,// 235 PAY 232 

    0x04b23a44,// 236 PAY 233 

    0xac4a07e0,// 237 PAY 234 

    0xaa4f21af,// 238 PAY 235 

    0xf1f5097e,// 239 PAY 236 

    0xf1f19628,// 240 PAY 237 

    0xc82972a6,// 241 PAY 238 

    0x254e3d8a,// 242 PAY 239 

    0x4ae46028,// 243 PAY 240 

    0x0f207f4c,// 244 PAY 241 

    0xa83b89cc,// 245 PAY 242 

    0x29b95e5d,// 246 PAY 243 

    0x301794cb,// 247 PAY 244 

    0x84d54998,// 248 PAY 245 

    0x57c52853,// 249 PAY 246 

    0xd131dd7d,// 250 PAY 247 

    0x83ec2fb2,// 251 PAY 248 

    0x385766f3,// 252 PAY 249 

    0x02e666da,// 253 PAY 250 

    0x49731dc7,// 254 PAY 251 

    0x2e97864f,// 255 PAY 252 

    0x920ad66e,// 256 PAY 253 

    0x65642591,// 257 PAY 254 

    0xa8527b1a,// 258 PAY 255 

    0x47a25d76,// 259 PAY 256 

    0x07907655,// 260 PAY 257 

    0x855a23b6,// 261 PAY 258 

    0x50730fb7,// 262 PAY 259 

    0x1e8d1e96,// 263 PAY 260 

    0xf0dec46b,// 264 PAY 261 

    0xe294fd7b,// 265 PAY 262 

    0x39a2bf57,// 266 PAY 263 

    0x94de0d45,// 267 PAY 264 

    0xadb1152b,// 268 PAY 265 

    0x01481dae,// 269 PAY 266 

    0x3041ec42,// 270 PAY 267 

    0x7c3b4ce3,// 271 PAY 268 

    0xa7e4f484,// 272 PAY 269 

    0xcc36eaa0,// 273 PAY 270 

    0x8481b99d,// 274 PAY 271 

    0x731b74c2,// 275 PAY 272 

    0x1f2675e5,// 276 PAY 273 

    0x460c2e61,// 277 PAY 274 

    0xe9c16e51,// 278 PAY 275 

    0x7c730b32,// 279 PAY 276 

    0xd72621cf,// 280 PAY 277 

    0x0bc47414,// 281 PAY 278 

    0x60978440,// 282 PAY 279 

    0xb6e5a368,// 283 PAY 280 

    0x595232f8,// 284 PAY 281 

    0xd55b5e82,// 285 PAY 282 

    0x46667045,// 286 PAY 283 

    0x17151309,// 287 PAY 284 

    0x52f94d6f,// 288 PAY 285 

    0xf1728936,// 289 PAY 286 

    0x2f4d7943,// 290 PAY 287 

    0x4b5188d7,// 291 PAY 288 

    0x68b4eb9b,// 292 PAY 289 

    0x8a45e948,// 293 PAY 290 

    0xec2a7400,// 294 PAY 291 

    0x92d5aca9,// 295 PAY 292 

    0x7ca3b642,// 296 PAY 293 

    0xfcf8352a,// 297 PAY 294 

    0x432c1e37,// 298 PAY 295 

    0xb58358f2,// 299 PAY 296 

    0x17f35a00,// 300 PAY 297 

    0x0ded7f2e,// 301 PAY 298 

    0xb71a4020,// 302 PAY 299 

    0x5c393370,// 303 PAY 300 

    0xed06343d,// 304 PAY 301 

    0xd3741901,// 305 PAY 302 

    0x3cf8dd80,// 306 PAY 303 

    0x350f900c,// 307 PAY 304 

    0x321fb5c2,// 308 PAY 305 

    0xee83bb94,// 309 PAY 306 

    0x2685ebdc,// 310 PAY 307 

    0x31d7f6c7,// 311 PAY 308 

    0x8c2d5b2a,// 312 PAY 309 

    0x62f1a843,// 313 PAY 310 

    0x6ba8297a,// 314 PAY 311 

    0x2ed49060,// 315 PAY 312 

    0x4f38675b,// 316 PAY 313 

    0x8795a9fa,// 317 PAY 314 

    0x46b9c08b,// 318 PAY 315 

    0xa83a873e,// 319 PAY 316 

    0xd9acf3d3,// 320 PAY 317 

    0x400d34ec,// 321 PAY 318 

    0x2a059c1c,// 322 PAY 319 

    0x022d0f89,// 323 PAY 320 

    0xf6650390,// 324 PAY 321 

    0x9b2d98ec,// 325 PAY 322 

    0xcda10488,// 326 PAY 323 

    0xf22e50cd,// 327 PAY 324 

    0x19a7dd6b,// 328 PAY 325 

    0xc988a04f,// 329 PAY 326 

    0xd46db5d5,// 330 PAY 327 

    0x7b6a0882,// 331 PAY 328 

    0xa1c64a84,// 332 PAY 329 

    0x1c244e06,// 333 PAY 330 

    0x1e845345,// 334 PAY 331 

    0xefadf28d,// 335 PAY 332 

    0xc8b20def,// 336 PAY 333 

    0xc3affeef,// 337 PAY 334 

    0x0f36b879,// 338 PAY 335 

    0xf7275e36,// 339 PAY 336 

    0x6325b9c0,// 340 PAY 337 

    0x3986845a,// 341 PAY 338 

    0xd01f43f8,// 342 PAY 339 

    0x8a5ac832,// 343 PAY 340 

    0x78952ead,// 344 PAY 341 

    0x862b075d,// 345 PAY 342 

    0x4fb6941d,// 346 PAY 343 

    0xf5054872,// 347 PAY 344 

    0x742e8f9f,// 348 PAY 345 

    0xd30d52a7,// 349 PAY 346 

    0x3dfa95a6,// 350 PAY 347 

    0x5b1918d2,// 351 PAY 348 

    0x632d6b7f,// 352 PAY 349 

    0x1867ae34,// 353 PAY 350 

    0x62e0eb60,// 354 PAY 351 

    0x54832774,// 355 PAY 352 

    0x723f007a,// 356 PAY 353 

    0x4a291b7d,// 357 PAY 354 

    0xb18e3a0d,// 358 PAY 355 

    0x0e18628c,// 359 PAY 356 

    0x15efb77c,// 360 PAY 357 

    0xb6f850f1,// 361 PAY 358 

    0x1a494bb1,// 362 PAY 359 

    0x10285c1b,// 363 PAY 360 

    0x3b55d23f,// 364 PAY 361 

    0x77763c81,// 365 PAY 362 

    0xa94a418a,// 366 PAY 363 

    0x051087d8,// 367 PAY 364 

    0x791fd616,// 368 PAY 365 

    0xcc051808,// 369 PAY 366 

    0xfe2cf169,// 370 PAY 367 

    0x9a955c31,// 371 PAY 368 

    0x1b67283f,// 372 PAY 369 

    0xf15c78fe,// 373 PAY 370 

    0x92ebeb70,// 374 PAY 371 

    0x7480f49d,// 375 PAY 372 

    0x80370a37,// 376 PAY 373 

    0xae2b9984,// 377 PAY 374 

    0x010c8900,// 378 PAY 375 

/// STA is 1 words. 

/// STA num_pkts       : 13 

/// STA pkt_idx        : 80 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9a 

    0x01419a0d // 379 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt45_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 211 words. 

/// BDA size     is 838 (0x346) 

/// BDA id       is 0xae4b 

    0x0346ae4b,// 3 BDA   1 

/// PAY Generic Data size   : 838 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x288f7b31,// 4 PAY   1 

    0xdec5b5ab,// 5 PAY   2 

    0x70542d20,// 6 PAY   3 

    0x29d06881,// 7 PAY   4 

    0x1734f7c1,// 8 PAY   5 

    0xfaa08535,// 9 PAY   6 

    0xca9345e1,// 10 PAY   7 

    0xfff691a2,// 11 PAY   8 

    0xe4c78df8,// 12 PAY   9 

    0x1f447709,// 13 PAY  10 

    0x5e938c9c,// 14 PAY  11 

    0xf8107fef,// 15 PAY  12 

    0x9a6a0ac2,// 16 PAY  13 

    0x9fe0c207,// 17 PAY  14 

    0xd7b35a65,// 18 PAY  15 

    0xfd6301df,// 19 PAY  16 

    0xc0b32da3,// 20 PAY  17 

    0x60964545,// 21 PAY  18 

    0x1948dbda,// 22 PAY  19 

    0x7b716c47,// 23 PAY  20 

    0x32a3eb97,// 24 PAY  21 

    0x7f0f46f6,// 25 PAY  22 

    0xfb6e53e7,// 26 PAY  23 

    0xeb426bc1,// 27 PAY  24 

    0xde8ae597,// 28 PAY  25 

    0x05531453,// 29 PAY  26 

    0x41f609b8,// 30 PAY  27 

    0x04cea21c,// 31 PAY  28 

    0x19068d00,// 32 PAY  29 

    0xc917ccc7,// 33 PAY  30 

    0x3d291894,// 34 PAY  31 

    0x91bd823d,// 35 PAY  32 

    0xf58b968b,// 36 PAY  33 

    0xce978ab3,// 37 PAY  34 

    0xa8d77d05,// 38 PAY  35 

    0xb9c0a016,// 39 PAY  36 

    0xa6a5fd63,// 40 PAY  37 

    0x93f0057d,// 41 PAY  38 

    0xd846882e,// 42 PAY  39 

    0xc926adb5,// 43 PAY  40 

    0x10ca82ca,// 44 PAY  41 

    0x49061f28,// 45 PAY  42 

    0x68c75ae2,// 46 PAY  43 

    0x5441c969,// 47 PAY  44 

    0x1c649dc8,// 48 PAY  45 

    0x7d99a73b,// 49 PAY  46 

    0x30b379cc,// 50 PAY  47 

    0xdbf77a7c,// 51 PAY  48 

    0x6c6f3bf1,// 52 PAY  49 

    0x4809dcbc,// 53 PAY  50 

    0x4bca84ff,// 54 PAY  51 

    0xbf5d1685,// 55 PAY  52 

    0xc3baa784,// 56 PAY  53 

    0x9ce4da13,// 57 PAY  54 

    0xa42b7eac,// 58 PAY  55 

    0xf7aea91f,// 59 PAY  56 

    0xbb7f6d1c,// 60 PAY  57 

    0xfa1cb82e,// 61 PAY  58 

    0x747988b5,// 62 PAY  59 

    0x2e2c688d,// 63 PAY  60 

    0xcba33a85,// 64 PAY  61 

    0xa44a2f73,// 65 PAY  62 

    0xcd217896,// 66 PAY  63 

    0x86349007,// 67 PAY  64 

    0xc4c3e7b2,// 68 PAY  65 

    0x0fd85f98,// 69 PAY  66 

    0x615a65c1,// 70 PAY  67 

    0xdc7e12f6,// 71 PAY  68 

    0xa6e3c618,// 72 PAY  69 

    0x64a2f546,// 73 PAY  70 

    0xfe779dca,// 74 PAY  71 

    0x8641e2e2,// 75 PAY  72 

    0xbbb65b9c,// 76 PAY  73 

    0x1b573d7d,// 77 PAY  74 

    0xd4c24ac8,// 78 PAY  75 

    0x9747b2ef,// 79 PAY  76 

    0xf5a7371c,// 80 PAY  77 

    0x3ef77425,// 81 PAY  78 

    0x8f9f3dd4,// 82 PAY  79 

    0x7687aee4,// 83 PAY  80 

    0x1bdb27fe,// 84 PAY  81 

    0x06014166,// 85 PAY  82 

    0xadec45b4,// 86 PAY  83 

    0x6a3973c6,// 87 PAY  84 

    0x41df9d10,// 88 PAY  85 

    0xf2268e6e,// 89 PAY  86 

    0x784893b1,// 90 PAY  87 

    0x8242c4b3,// 91 PAY  88 

    0xb171fd22,// 92 PAY  89 

    0x38ee964f,// 93 PAY  90 

    0x838479e7,// 94 PAY  91 

    0xd65f0cf1,// 95 PAY  92 

    0x47c896ef,// 96 PAY  93 

    0x5e41fd91,// 97 PAY  94 

    0xea4cb478,// 98 PAY  95 

    0xf3924d2f,// 99 PAY  96 

    0xb63fe9ce,// 100 PAY  97 

    0xfdca432e,// 101 PAY  98 

    0x0f326439,// 102 PAY  99 

    0x1143c356,// 103 PAY 100 

    0xaae994a6,// 104 PAY 101 

    0x1d3447e0,// 105 PAY 102 

    0x0b1a9a9c,// 106 PAY 103 

    0x53781d9e,// 107 PAY 104 

    0xf59f7fe0,// 108 PAY 105 

    0x9042a27b,// 109 PAY 106 

    0xf0e0737f,// 110 PAY 107 

    0x62d6d57f,// 111 PAY 108 

    0x24c6930f,// 112 PAY 109 

    0x726c6bf4,// 113 PAY 110 

    0xc8586970,// 114 PAY 111 

    0xe1992a2c,// 115 PAY 112 

    0x6f31a3cf,// 116 PAY 113 

    0xa9f2831b,// 117 PAY 114 

    0x1c1b8a89,// 118 PAY 115 

    0xd059bf35,// 119 PAY 116 

    0x8b269a02,// 120 PAY 117 

    0xd987e778,// 121 PAY 118 

    0xaf4ae694,// 122 PAY 119 

    0x1db17da6,// 123 PAY 120 

    0xf1c4dc01,// 124 PAY 121 

    0xe581f391,// 125 PAY 122 

    0x7d5a4e26,// 126 PAY 123 

    0x5af036f5,// 127 PAY 124 

    0xede7022b,// 128 PAY 125 

    0x0297c64c,// 129 PAY 126 

    0x1c7d3e76,// 130 PAY 127 

    0x7520d912,// 131 PAY 128 

    0xdd094c07,// 132 PAY 129 

    0xfd01914e,// 133 PAY 130 

    0x6c0e65c2,// 134 PAY 131 

    0x6de7e500,// 135 PAY 132 

    0x16afdfac,// 136 PAY 133 

    0xe428d327,// 137 PAY 134 

    0x76514834,// 138 PAY 135 

    0x15dc13b1,// 139 PAY 136 

    0xba0a1b1f,// 140 PAY 137 

    0xa2b5b1e8,// 141 PAY 138 

    0x452e3a3a,// 142 PAY 139 

    0x20fa34b5,// 143 PAY 140 

    0x67be082e,// 144 PAY 141 

    0x87c530e4,// 145 PAY 142 

    0x7302ec7c,// 146 PAY 143 

    0xbc79ad67,// 147 PAY 144 

    0x7c097852,// 148 PAY 145 

    0xa608a0ca,// 149 PAY 146 

    0xf2543939,// 150 PAY 147 

    0x71e42b03,// 151 PAY 148 

    0xf6b82f2a,// 152 PAY 149 

    0x75651669,// 153 PAY 150 

    0x6dff3192,// 154 PAY 151 

    0xba146e65,// 155 PAY 152 

    0xa7bc81fa,// 156 PAY 153 

    0x999169fe,// 157 PAY 154 

    0xf86d28c9,// 158 PAY 155 

    0xb4de706a,// 159 PAY 156 

    0xbc6fe474,// 160 PAY 157 

    0xf2186d3f,// 161 PAY 158 

    0x184751af,// 162 PAY 159 

    0x87a50325,// 163 PAY 160 

    0x9396527d,// 164 PAY 161 

    0x20384bb7,// 165 PAY 162 

    0x758a955f,// 166 PAY 163 

    0x6f3a0678,// 167 PAY 164 

    0x843c0543,// 168 PAY 165 

    0xb4b4976f,// 169 PAY 166 

    0x1300db5f,// 170 PAY 167 

    0x8adea50e,// 171 PAY 168 

    0x35330ec4,// 172 PAY 169 

    0x252c05f0,// 173 PAY 170 

    0x5e7a3ae7,// 174 PAY 171 

    0x8ed78b73,// 175 PAY 172 

    0x35783b62,// 176 PAY 173 

    0x1532d9a1,// 177 PAY 174 

    0xd621620b,// 178 PAY 175 

    0x3512e33c,// 179 PAY 176 

    0xc31ec95f,// 180 PAY 177 

    0xc58e79a5,// 181 PAY 178 

    0x07d314cc,// 182 PAY 179 

    0xf0a4b6e9,// 183 PAY 180 

    0xd348ef19,// 184 PAY 181 

    0xca3defd4,// 185 PAY 182 

    0x4823e08e,// 186 PAY 183 

    0x569d45af,// 187 PAY 184 

    0x7ff48c28,// 188 PAY 185 

    0xb8c1d978,// 189 PAY 186 

    0x1e313802,// 190 PAY 187 

    0x19792c7a,// 191 PAY 188 

    0xc4bf66e0,// 192 PAY 189 

    0x9a63390f,// 193 PAY 190 

    0xfc83d88d,// 194 PAY 191 

    0x7c8a6b21,// 195 PAY 192 

    0x8d0b929e,// 196 PAY 193 

    0xebcaa76c,// 197 PAY 194 

    0x1b9002f4,// 198 PAY 195 

    0xc0c2716b,// 199 PAY 196 

    0x93ed1e7d,// 200 PAY 197 

    0xfb5cece0,// 201 PAY 198 

    0xb855c227,// 202 PAY 199 

    0x1bfade2f,// 203 PAY 200 

    0x588dd5c7,// 204 PAY 201 

    0x14f43e88,// 205 PAY 202 

    0x26a87efc,// 206 PAY 203 

    0x47a768a8,// 207 PAY 204 

    0xb16b2e0b,// 208 PAY 205 

    0x85fd3bf2,// 209 PAY 206 

    0xad86df72,// 210 PAY 207 

    0xb9754d6b,// 211 PAY 208 

    0x2d39465e,// 212 PAY 209 

    0x69030000,// 213 PAY 210 

/// STA is 1 words. 

/// STA num_pkts       : 173 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3f 

    0x02cd3fad // 214 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt46_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 53 words. 

/// BDA size     is 206 (0xce) 

/// BDA id       is 0x3c4b 

    0x00ce3c4b,// 3 BDA   1 

/// PAY Generic Data size   : 206 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x01e780a6,// 4 PAY   1 

    0xfaffb14a,// 5 PAY   2 

    0xdb37b73e,// 6 PAY   3 

    0xd488cdb3,// 7 PAY   4 

    0xa5110597,// 8 PAY   5 

    0x4c71b803,// 9 PAY   6 

    0x763c2a7f,// 10 PAY   7 

    0x0d0741ff,// 11 PAY   8 

    0x8257bf74,// 12 PAY   9 

    0x6e1ae91a,// 13 PAY  10 

    0xbff6d996,// 14 PAY  11 

    0x2bf45306,// 15 PAY  12 

    0x33250aae,// 16 PAY  13 

    0x6b30a438,// 17 PAY  14 

    0x161e08d9,// 18 PAY  15 

    0xffb43893,// 19 PAY  16 

    0x76a900c7,// 20 PAY  17 

    0x25e902a5,// 21 PAY  18 

    0xd6ba0444,// 22 PAY  19 

    0x46ad8c0f,// 23 PAY  20 

    0xf72db40d,// 24 PAY  21 

    0xf72171e4,// 25 PAY  22 

    0x317b5fb9,// 26 PAY  23 

    0x6a0a9472,// 27 PAY  24 

    0x4d7d2c2a,// 28 PAY  25 

    0xacf4b999,// 29 PAY  26 

    0xe18415e4,// 30 PAY  27 

    0x257347b2,// 31 PAY  28 

    0xc86440c2,// 32 PAY  29 

    0xf57ce579,// 33 PAY  30 

    0x46fbe708,// 34 PAY  31 

    0x509ffb2c,// 35 PAY  32 

    0xeedb660e,// 36 PAY  33 

    0x077245bd,// 37 PAY  34 

    0x377da5a3,// 38 PAY  35 

    0x1d31d942,// 39 PAY  36 

    0x1dd2adf2,// 40 PAY  37 

    0xfd2ca583,// 41 PAY  38 

    0xe9f21f34,// 42 PAY  39 

    0xfeed6db3,// 43 PAY  40 

    0x58c266cf,// 44 PAY  41 

    0xc1a8c9a3,// 45 PAY  42 

    0xcec93382,// 46 PAY  43 

    0x71f5ac9f,// 47 PAY  44 

    0xec050809,// 48 PAY  45 

    0xb24ec47e,// 49 PAY  46 

    0x44e7fbda,// 50 PAY  47 

    0x2e53ccbc,// 51 PAY  48 

    0xa12006b3,// 52 PAY  49 

    0x80978dba,// 53 PAY  50 

    0x584b0689,// 54 PAY  51 

    0x83fe0000,// 55 PAY  52 

/// STA is 1 words. 

/// STA num_pkts       : 100 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6f 

    0x02616f64 // 56 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt47_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 126 words. 

/// BDA size     is 500 (0x1f4) 

/// BDA id       is 0x4ac8 

    0x01f44ac8,// 3 BDA   1 

/// PAY Generic Data size   : 500 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x136ed125,// 4 PAY   1 

    0x56fcd3e0,// 5 PAY   2 

    0x31373b65,// 6 PAY   3 

    0x81158e9d,// 7 PAY   4 

    0x16b804e8,// 8 PAY   5 

    0x1e69b082,// 9 PAY   6 

    0x0219396b,// 10 PAY   7 

    0x4fbd6baa,// 11 PAY   8 

    0x3e73afcb,// 12 PAY   9 

    0xfac5232c,// 13 PAY  10 

    0x77f7475c,// 14 PAY  11 

    0xbc19e89d,// 15 PAY  12 

    0xf8ee272a,// 16 PAY  13 

    0x682055c7,// 17 PAY  14 

    0xe68176a2,// 18 PAY  15 

    0xf4d01bc1,// 19 PAY  16 

    0x4e5bfabe,// 20 PAY  17 

    0xe65d1d4f,// 21 PAY  18 

    0xd39e1228,// 22 PAY  19 

    0x331ab2cd,// 23 PAY  20 

    0xc026e07e,// 24 PAY  21 

    0x51421361,// 25 PAY  22 

    0x519c3f57,// 26 PAY  23 

    0x2156279a,// 27 PAY  24 

    0xfc36c453,// 28 PAY  25 

    0x117a9f7b,// 29 PAY  26 

    0xbdbadc8a,// 30 PAY  27 

    0xd25c7d68,// 31 PAY  28 

    0x8183e766,// 32 PAY  29 

    0x03498543,// 33 PAY  30 

    0x459625c4,// 34 PAY  31 

    0x2d3ca0f7,// 35 PAY  32 

    0x5c1cb74e,// 36 PAY  33 

    0x8619d470,// 37 PAY  34 

    0x183f44b1,// 38 PAY  35 

    0x2bb71447,// 39 PAY  36 

    0x508d796b,// 40 PAY  37 

    0x05eb898c,// 41 PAY  38 

    0xda8018bc,// 42 PAY  39 

    0x403069e5,// 43 PAY  40 

    0x1d204d59,// 44 PAY  41 

    0x7aea32df,// 45 PAY  42 

    0xdff04c9c,// 46 PAY  43 

    0x357f175b,// 47 PAY  44 

    0x8c5f49c4,// 48 PAY  45 

    0xbdec73c1,// 49 PAY  46 

    0x2c2287d8,// 50 PAY  47 

    0xafd20cc3,// 51 PAY  48 

    0x7825bcba,// 52 PAY  49 

    0x6d08ed48,// 53 PAY  50 

    0x15f2ffaf,// 54 PAY  51 

    0xc7ba385d,// 55 PAY  52 

    0x04d96c0b,// 56 PAY  53 

    0xf8a1326c,// 57 PAY  54 

    0x1397dd3e,// 58 PAY  55 

    0xf823964b,// 59 PAY  56 

    0xe5e7f1ec,// 60 PAY  57 

    0xda4ef7a5,// 61 PAY  58 

    0x9f682120,// 62 PAY  59 

    0xcd38a8be,// 63 PAY  60 

    0x8b0fd7fe,// 64 PAY  61 

    0x6476c0dd,// 65 PAY  62 

    0x664d8ebc,// 66 PAY  63 

    0xf7abeafe,// 67 PAY  64 

    0xc9d7ebd1,// 68 PAY  65 

    0x49f6aa34,// 69 PAY  66 

    0xa8ef3686,// 70 PAY  67 

    0xed33d6a2,// 71 PAY  68 

    0xb8908985,// 72 PAY  69 

    0x7fa08648,// 73 PAY  70 

    0xcaf0ef0f,// 74 PAY  71 

    0xf8b2e961,// 75 PAY  72 

    0xa3d34888,// 76 PAY  73 

    0xe893f427,// 77 PAY  74 

    0x2965f725,// 78 PAY  75 

    0x0e08ade4,// 79 PAY  76 

    0xaaa4b3e0,// 80 PAY  77 

    0xf0cacb90,// 81 PAY  78 

    0xf908edea,// 82 PAY  79 

    0xa29bd78f,// 83 PAY  80 

    0xa9927758,// 84 PAY  81 

    0xd0d11602,// 85 PAY  82 

    0x4a96bb51,// 86 PAY  83 

    0x94995db6,// 87 PAY  84 

    0xab4828aa,// 88 PAY  85 

    0xd91a2ed6,// 89 PAY  86 

    0xdf1fb2e6,// 90 PAY  87 

    0x5132436b,// 91 PAY  88 

    0x96e4a02a,// 92 PAY  89 

    0x84bd1c79,// 93 PAY  90 

    0xf0f3dae9,// 94 PAY  91 

    0xaa38be25,// 95 PAY  92 

    0x129775b5,// 96 PAY  93 

    0xf232c85f,// 97 PAY  94 

    0xc5e6708e,// 98 PAY  95 

    0x4bf5b509,// 99 PAY  96 

    0x97a054fa,// 100 PAY  97 

    0xc336c95c,// 101 PAY  98 

    0x423cb91e,// 102 PAY  99 

    0xf05b44f9,// 103 PAY 100 

    0x9310654b,// 104 PAY 101 

    0x9a9a6c52,// 105 PAY 102 

    0x70129684,// 106 PAY 103 

    0x16ee126c,// 107 PAY 104 

    0x236e2d3b,// 108 PAY 105 

    0x0a7b90da,// 109 PAY 106 

    0x20fcb7c1,// 110 PAY 107 

    0x0c2751e3,// 111 PAY 108 

    0x462c63da,// 112 PAY 109 

    0xcada1409,// 113 PAY 110 

    0x883a5535,// 114 PAY 111 

    0x3b895881,// 115 PAY 112 

    0x70e7a42e,// 116 PAY 113 

    0xfb5c698d,// 117 PAY 114 

    0xd792fcc0,// 118 PAY 115 

    0xe01d6a01,// 119 PAY 116 

    0xe7970597,// 120 PAY 117 

    0x3df78665,// 121 PAY 118 

    0x1f3833c7,// 122 PAY 119 

    0x3096515f,// 123 PAY 120 

    0xbf78ca5c,// 124 PAY 121 

    0x5cd89299,// 125 PAY 122 

    0xa8574a6a,// 126 PAY 123 

    0x61d97897,// 127 PAY 124 

    0x7eae8f48,// 128 PAY 125 

/// HASH is  8 bytes 

    0xd792fcc0,// 129 HSH   1 

    0xe01d6a01,// 130 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 35 

/// STA pkt_idx        : 128 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xac 

    0x0200ac23 // 131 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt48_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 181 words. 

/// BDA size     is 720 (0x2d0) 

/// BDA id       is 0x267c 

    0x02d0267c,// 3 BDA   1 

/// PAY Generic Data size   : 720 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x632b4d7a,// 4 PAY   1 

    0x167d721c,// 5 PAY   2 

    0x29a7505e,// 6 PAY   3 

    0x36dba929,// 7 PAY   4 

    0x87e8a29a,// 8 PAY   5 

    0xaded9bc1,// 9 PAY   6 

    0xffb9c6d7,// 10 PAY   7 

    0x42120664,// 11 PAY   8 

    0x6a105d33,// 12 PAY   9 

    0xcbaf4725,// 13 PAY  10 

    0xf34d1b74,// 14 PAY  11 

    0x8cae55eb,// 15 PAY  12 

    0xd89f5e64,// 16 PAY  13 

    0x6a197444,// 17 PAY  14 

    0x5a66c906,// 18 PAY  15 

    0xab7029a6,// 19 PAY  16 

    0xcdf6fb6c,// 20 PAY  17 

    0xf839d287,// 21 PAY  18 

    0x43c1f3b3,// 22 PAY  19 

    0x9c88fe14,// 23 PAY  20 

    0x8addc7fd,// 24 PAY  21 

    0x8a6d05b7,// 25 PAY  22 

    0xf7937f90,// 26 PAY  23 

    0x44353cbd,// 27 PAY  24 

    0x0063ee9b,// 28 PAY  25 

    0x77f7c902,// 29 PAY  26 

    0x691ac262,// 30 PAY  27 

    0x2059946c,// 31 PAY  28 

    0x9615c410,// 32 PAY  29 

    0x632f23eb,// 33 PAY  30 

    0xa6508cdd,// 34 PAY  31 

    0x0f2b7721,// 35 PAY  32 

    0x36d884cf,// 36 PAY  33 

    0x5474259c,// 37 PAY  34 

    0x51664c76,// 38 PAY  35 

    0x52fb6b06,// 39 PAY  36 

    0x463ea431,// 40 PAY  37 

    0x774c40e3,// 41 PAY  38 

    0xbd320673,// 42 PAY  39 

    0x1f357a9e,// 43 PAY  40 

    0x26b5d3ef,// 44 PAY  41 

    0xbdfec7a8,// 45 PAY  42 

    0x9d4c5a97,// 46 PAY  43 

    0x7a205d43,// 47 PAY  44 

    0xf4288d7e,// 48 PAY  45 

    0x3283aa2e,// 49 PAY  46 

    0x9dbf839b,// 50 PAY  47 

    0x254f1b38,// 51 PAY  48 

    0x3866425f,// 52 PAY  49 

    0x08fe9ebf,// 53 PAY  50 

    0xc1f3a6d1,// 54 PAY  51 

    0x1850248b,// 55 PAY  52 

    0xfaa05b3d,// 56 PAY  53 

    0xa33c954e,// 57 PAY  54 

    0xdebd12a5,// 58 PAY  55 

    0x30da2489,// 59 PAY  56 

    0x862e6bf6,// 60 PAY  57 

    0x13f69b24,// 61 PAY  58 

    0xdeab0636,// 62 PAY  59 

    0x1ed05ebd,// 63 PAY  60 

    0x59102003,// 64 PAY  61 

    0xa4fe2e45,// 65 PAY  62 

    0x07e98575,// 66 PAY  63 

    0x702a5d31,// 67 PAY  64 

    0x1d8d49d0,// 68 PAY  65 

    0x9d1a423a,// 69 PAY  66 

    0x7d83496c,// 70 PAY  67 

    0x6412f2f3,// 71 PAY  68 

    0x8c5ec5b0,// 72 PAY  69 

    0x88fdef2b,// 73 PAY  70 

    0x8d67e242,// 74 PAY  71 

    0x0e98b4d5,// 75 PAY  72 

    0x60d1834e,// 76 PAY  73 

    0x7739e47e,// 77 PAY  74 

    0x0a989367,// 78 PAY  75 

    0xba2a950c,// 79 PAY  76 

    0xf989d254,// 80 PAY  77 

    0x2f8ed635,// 81 PAY  78 

    0xc5e8b28d,// 82 PAY  79 

    0xb2bbf205,// 83 PAY  80 

    0x7f245f69,// 84 PAY  81 

    0x19252561,// 85 PAY  82 

    0x044a6799,// 86 PAY  83 

    0x90a72c5e,// 87 PAY  84 

    0x3d6bade3,// 88 PAY  85 

    0x34ebb6e6,// 89 PAY  86 

    0xe8db5e68,// 90 PAY  87 

    0x75240e86,// 91 PAY  88 

    0x55b28510,// 92 PAY  89 

    0x3909da18,// 93 PAY  90 

    0xa66106bf,// 94 PAY  91 

    0xa9f389a2,// 95 PAY  92 

    0x2aa0b900,// 96 PAY  93 

    0x7fdeb78d,// 97 PAY  94 

    0xe72f785c,// 98 PAY  95 

    0x4c4619fb,// 99 PAY  96 

    0xe3e0fa01,// 100 PAY  97 

    0x26c2b8b3,// 101 PAY  98 

    0xe39b7409,// 102 PAY  99 

    0x04992b44,// 103 PAY 100 

    0xc54f580e,// 104 PAY 101 

    0x0870a3c5,// 105 PAY 102 

    0x04360548,// 106 PAY 103 

    0x9fc72a4f,// 107 PAY 104 

    0xe1245961,// 108 PAY 105 

    0xb2a10b69,// 109 PAY 106 

    0xdd00cd11,// 110 PAY 107 

    0x73cf58ea,// 111 PAY 108 

    0xc9da33f3,// 112 PAY 109 

    0x7558d72c,// 113 PAY 110 

    0xa5f6e513,// 114 PAY 111 

    0x76a8f8cd,// 115 PAY 112 

    0xf9133e13,// 116 PAY 113 

    0x4360c566,// 117 PAY 114 

    0x105a3406,// 118 PAY 115 

    0xd6fafa42,// 119 PAY 116 

    0x07d9ed92,// 120 PAY 117 

    0xdb5f1dc8,// 121 PAY 118 

    0x7d3b483e,// 122 PAY 119 

    0x26501db5,// 123 PAY 120 

    0xbe4a1ebb,// 124 PAY 121 

    0xd9804ff9,// 125 PAY 122 

    0x2b461d90,// 126 PAY 123 

    0x9ba3253c,// 127 PAY 124 

    0x3c1759f8,// 128 PAY 125 

    0x25f4828d,// 129 PAY 126 

    0x461936c9,// 130 PAY 127 

    0x959efe8d,// 131 PAY 128 

    0x15949aef,// 132 PAY 129 

    0x6a90a4ab,// 133 PAY 130 

    0x0822f8c9,// 134 PAY 131 

    0x696fdef4,// 135 PAY 132 

    0x0e1d40aa,// 136 PAY 133 

    0x72935012,// 137 PAY 134 

    0x140b8866,// 138 PAY 135 

    0x4051f424,// 139 PAY 136 

    0x880281d9,// 140 PAY 137 

    0x023e990c,// 141 PAY 138 

    0x30139d7e,// 142 PAY 139 

    0xb2d9c125,// 143 PAY 140 

    0x7b7c0592,// 144 PAY 141 

    0x8f63ce5c,// 145 PAY 142 

    0x9d0533b4,// 146 PAY 143 

    0x830a2066,// 147 PAY 144 

    0x50b993fc,// 148 PAY 145 

    0x6bb893f3,// 149 PAY 146 

    0x02b9658d,// 150 PAY 147 

    0xc0a0dddc,// 151 PAY 148 

    0xba2c3b40,// 152 PAY 149 

    0xe697f0b1,// 153 PAY 150 

    0xb2c02809,// 154 PAY 151 

    0x8939cb4d,// 155 PAY 152 

    0x5685f3e4,// 156 PAY 153 

    0xf3cfb174,// 157 PAY 154 

    0x071fd2cd,// 158 PAY 155 

    0x664d61c9,// 159 PAY 156 

    0x19ea4afc,// 160 PAY 157 

    0x5d944686,// 161 PAY 158 

    0x8b99b4cc,// 162 PAY 159 

    0x07bcd8d7,// 163 PAY 160 

    0x6d2abd59,// 164 PAY 161 

    0xa045412c,// 165 PAY 162 

    0x13805e11,// 166 PAY 163 

    0xc0a8d771,// 167 PAY 164 

    0xf3cd3ab5,// 168 PAY 165 

    0x6fd72cb2,// 169 PAY 166 

    0x926990cd,// 170 PAY 167 

    0x4c5acbcb,// 171 PAY 168 

    0xf620fb3b,// 172 PAY 169 

    0x4cbd7f48,// 173 PAY 170 

    0xd728c5c7,// 174 PAY 171 

    0x6a5cd9ad,// 175 PAY 172 

    0x4f15226f,// 176 PAY 173 

    0xb67422d0,// 177 PAY 174 

    0xb719b89a,// 178 PAY 175 

    0x15fba93d,// 179 PAY 176 

    0xbfa61634,// 180 PAY 177 

    0x2f2e5b08,// 181 PAY 178 

    0x909f4c49,// 182 PAY 179 

    0x1db17aa4,// 183 PAY 180 

/// STA is 1 words. 

/// STA num_pkts       : 144 

/// STA pkt_idx        : 108 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x01b11c90 // 184 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt49_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 174 words. 

/// BDA size     is 692 (0x2b4) 

/// BDA id       is 0x5630 

    0x02b45630,// 3 BDA   1 

/// PAY Generic Data size   : 692 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xd92236da,// 4 PAY   1 

    0xce76df13,// 5 PAY   2 

    0x45b20dbf,// 6 PAY   3 

    0x5b3dee02,// 7 PAY   4 

    0xbab89e65,// 8 PAY   5 

    0xa2f70930,// 9 PAY   6 

    0x7173fb15,// 10 PAY   7 

    0xe4e06806,// 11 PAY   8 

    0x2d5f19aa,// 12 PAY   9 

    0x972d0dcf,// 13 PAY  10 

    0x41dcadfc,// 14 PAY  11 

    0x9fc33d68,// 15 PAY  12 

    0x57cfb668,// 16 PAY  13 

    0x0e9f0d29,// 17 PAY  14 

    0xa7feb6b0,// 18 PAY  15 

    0xaffdbaf1,// 19 PAY  16 

    0xe3494e5c,// 20 PAY  17 

    0x5313b99f,// 21 PAY  18 

    0x0b674eb6,// 22 PAY  19 

    0xb62b22eb,// 23 PAY  20 

    0xbb32f975,// 24 PAY  21 

    0xa1b61107,// 25 PAY  22 

    0x9e3c9cce,// 26 PAY  23 

    0xe6cfb1a9,// 27 PAY  24 

    0x896e89ee,// 28 PAY  25 

    0xcaa66de8,// 29 PAY  26 

    0x5ae1ff85,// 30 PAY  27 

    0x2da70649,// 31 PAY  28 

    0xb26d8462,// 32 PAY  29 

    0xf12c0b3b,// 33 PAY  30 

    0xd1deac02,// 34 PAY  31 

    0x3ef399a2,// 35 PAY  32 

    0x591db88a,// 36 PAY  33 

    0xea289ade,// 37 PAY  34 

    0xabb01c13,// 38 PAY  35 

    0x8f8db3d7,// 39 PAY  36 

    0x18af4d53,// 40 PAY  37 

    0x31e828c5,// 41 PAY  38 

    0xfa65a372,// 42 PAY  39 

    0xc3255a62,// 43 PAY  40 

    0x63ad0f99,// 44 PAY  41 

    0x163e6dc1,// 45 PAY  42 

    0x0def7639,// 46 PAY  43 

    0x87949b27,// 47 PAY  44 

    0x0b7d3f42,// 48 PAY  45 

    0xd68ed374,// 49 PAY  46 

    0x4ec78953,// 50 PAY  47 

    0x710a0e7b,// 51 PAY  48 

    0xfdaf65f2,// 52 PAY  49 

    0x01afb583,// 53 PAY  50 

    0xdec7be8a,// 54 PAY  51 

    0xa133785c,// 55 PAY  52 

    0x9cff372b,// 56 PAY  53 

    0x68713539,// 57 PAY  54 

    0xa61e4ce3,// 58 PAY  55 

    0x29ee9a5d,// 59 PAY  56 

    0xb3980e48,// 60 PAY  57 

    0xabe22456,// 61 PAY  58 

    0x87bece1f,// 62 PAY  59 

    0xf15dd089,// 63 PAY  60 

    0x546cb973,// 64 PAY  61 

    0x0f9e1b34,// 65 PAY  62 

    0x3d36c800,// 66 PAY  63 

    0xc15bd3ea,// 67 PAY  64 

    0x9de0fbd7,// 68 PAY  65 

    0xa42f8b75,// 69 PAY  66 

    0xfb41fb7c,// 70 PAY  67 

    0x8d90b78c,// 71 PAY  68 

    0x131de8b5,// 72 PAY  69 

    0x8efb3156,// 73 PAY  70 

    0x1b4cf41f,// 74 PAY  71 

    0xaff9971a,// 75 PAY  72 

    0xf74934bd,// 76 PAY  73 

    0xf6f8b23e,// 77 PAY  74 

    0xda512d55,// 78 PAY  75 

    0x6bc832c7,// 79 PAY  76 

    0xe3ff4ddd,// 80 PAY  77 

    0xb3a4993b,// 81 PAY  78 

    0x8680b63c,// 82 PAY  79 

    0x470931ce,// 83 PAY  80 

    0x7931b88d,// 84 PAY  81 

    0xa4cd98f4,// 85 PAY  82 

    0xef9b5dde,// 86 PAY  83 

    0xcf63e614,// 87 PAY  84 

    0x2c5386c6,// 88 PAY  85 

    0x32228271,// 89 PAY  86 

    0x32b57d4c,// 90 PAY  87 

    0x53cb6ba2,// 91 PAY  88 

    0x7c80c3b7,// 92 PAY  89 

    0x4cc8c930,// 93 PAY  90 

    0x82cbad94,// 94 PAY  91 

    0x61a10d43,// 95 PAY  92 

    0xe94dd699,// 96 PAY  93 

    0x332a8019,// 97 PAY  94 

    0x3bd32e2f,// 98 PAY  95 

    0xe3d8c8d2,// 99 PAY  96 

    0x33e9aeeb,// 100 PAY  97 

    0x88ec039a,// 101 PAY  98 

    0x0f4e1ec1,// 102 PAY  99 

    0x5cab9dd9,// 103 PAY 100 

    0xe432c94f,// 104 PAY 101 

    0x86740380,// 105 PAY 102 

    0x8618490a,// 106 PAY 103 

    0x20baffce,// 107 PAY 104 

    0xe243a9d8,// 108 PAY 105 

    0x15e0fe57,// 109 PAY 106 

    0xd871e182,// 110 PAY 107 

    0x6c82cbdf,// 111 PAY 108 

    0x6c4195fa,// 112 PAY 109 

    0xf05922f0,// 113 PAY 110 

    0x6ad5796d,// 114 PAY 111 

    0xba96cfff,// 115 PAY 112 

    0xbeb76c20,// 116 PAY 113 

    0x6e639bb7,// 117 PAY 114 

    0xfe607789,// 118 PAY 115 

    0xa6c4faea,// 119 PAY 116 

    0x8cb3e423,// 120 PAY 117 

    0x92cc5b87,// 121 PAY 118 

    0xe9b253ed,// 122 PAY 119 

    0xde08263f,// 123 PAY 120 

    0x41c3deaf,// 124 PAY 121 

    0xc8d926e5,// 125 PAY 122 

    0x26c0f063,// 126 PAY 123 

    0x924bd413,// 127 PAY 124 

    0x053fa3ad,// 128 PAY 125 

    0x8615eb62,// 129 PAY 126 

    0x8b5fbcac,// 130 PAY 127 

    0xf7fe7a90,// 131 PAY 128 

    0xba1ef38c,// 132 PAY 129 

    0x63a11908,// 133 PAY 130 

    0xc108a4b4,// 134 PAY 131 

    0x573ddc06,// 135 PAY 132 

    0x51d00126,// 136 PAY 133 

    0x7bcf4a41,// 137 PAY 134 

    0x6639cbdd,// 138 PAY 135 

    0x60a969cd,// 139 PAY 136 

    0xd288cd6f,// 140 PAY 137 

    0x637e66e9,// 141 PAY 138 

    0xd3291f85,// 142 PAY 139 

    0x979f2d29,// 143 PAY 140 

    0xd60bf4d5,// 144 PAY 141 

    0x387a9925,// 145 PAY 142 

    0xb0d3b1b4,// 146 PAY 143 

    0x8d5d9b27,// 147 PAY 144 

    0xfa1cb0a5,// 148 PAY 145 

    0xaa2fcec3,// 149 PAY 146 

    0x58211c9c,// 150 PAY 147 

    0x9fe9c254,// 151 PAY 148 

    0x2d7bbff4,// 152 PAY 149 

    0x1a63dbb4,// 153 PAY 150 

    0x64e1c2e2,// 154 PAY 151 

    0xf86159c0,// 155 PAY 152 

    0xe5d9b9c1,// 156 PAY 153 

    0xaf51992e,// 157 PAY 154 

    0xa003cfc9,// 158 PAY 155 

    0x9dbf2aef,// 159 PAY 156 

    0x186029f8,// 160 PAY 157 

    0xf5801d3a,// 161 PAY 158 

    0x4f0fd15a,// 162 PAY 159 

    0xdc6e26a0,// 163 PAY 160 

    0xe74e424a,// 164 PAY 161 

    0x2cfad088,// 165 PAY 162 

    0x901c7126,// 166 PAY 163 

    0x9859ff73,// 167 PAY 164 

    0x7a8d397d,// 168 PAY 165 

    0x04f8a52a,// 169 PAY 166 

    0xe0aab883,// 170 PAY 167 

    0xace41f25,// 171 PAY 168 

    0xe67e11b2,// 172 PAY 169 

    0x9527f0a2,// 173 PAY 170 

    0x76481bda,// 174 PAY 171 

    0xe6254cc5,// 175 PAY 172 

    0xe1731fc0,// 176 PAY 173 

/// HASH is  12 bytes 

    0xba1ef38c,// 177 HSH   1 

    0x63a11908,// 178 HSH   2 

    0xc108a4b4,// 179 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 101 

/// STA pkt_idx        : 196 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0311ce65 // 180 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt50_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 86 words. 

/// BDA size     is 340 (0x154) 

/// BDA id       is 0x3ee5 

    0x01543ee5,// 3 BDA   1 

/// PAY Generic Data size   : 340 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x9fbb0993,// 4 PAY   1 

    0x4bbe50f8,// 5 PAY   2 

    0x3c2163fa,// 6 PAY   3 

    0xa55cb87f,// 7 PAY   4 

    0x18eab902,// 8 PAY   5 

    0xdde9f511,// 9 PAY   6 

    0x9110d2e7,// 10 PAY   7 

    0xae1502fd,// 11 PAY   8 

    0x189d969f,// 12 PAY   9 

    0x8e412c06,// 13 PAY  10 

    0xb4c04d97,// 14 PAY  11 

    0x8814af3a,// 15 PAY  12 

    0x4ad40054,// 16 PAY  13 

    0x2f91f919,// 17 PAY  14 

    0xd10358fd,// 18 PAY  15 

    0x8c70f07c,// 19 PAY  16 

    0x4debc6bf,// 20 PAY  17 

    0x7a82287c,// 21 PAY  18 

    0xfa1091f9,// 22 PAY  19 

    0x23d0d34b,// 23 PAY  20 

    0x95d472c1,// 24 PAY  21 

    0x84d4be53,// 25 PAY  22 

    0x68d6ca95,// 26 PAY  23 

    0x70f545f4,// 27 PAY  24 

    0xfad9b6ef,// 28 PAY  25 

    0x452d0301,// 29 PAY  26 

    0xd27786cb,// 30 PAY  27 

    0xb39e3bed,// 31 PAY  28 

    0xcf967632,// 32 PAY  29 

    0xeb0d7d44,// 33 PAY  30 

    0x4bd52cf4,// 34 PAY  31 

    0x26eb5467,// 35 PAY  32 

    0x122cd02d,// 36 PAY  33 

    0xe535ec7c,// 37 PAY  34 

    0x70142e66,// 38 PAY  35 

    0xaf99b6ac,// 39 PAY  36 

    0xa6c3fb6f,// 40 PAY  37 

    0xbbd65777,// 41 PAY  38 

    0x0e6ba266,// 42 PAY  39 

    0x8b4e0b3e,// 43 PAY  40 

    0x13cc2c50,// 44 PAY  41 

    0x9034c7f1,// 45 PAY  42 

    0x0dc7d909,// 46 PAY  43 

    0xb7db8918,// 47 PAY  44 

    0xc5cf2b37,// 48 PAY  45 

    0x89364d1b,// 49 PAY  46 

    0xb8ee1293,// 50 PAY  47 

    0x171f374f,// 51 PAY  48 

    0x71653361,// 52 PAY  49 

    0x6fee3b29,// 53 PAY  50 

    0xac1a97a2,// 54 PAY  51 

    0xa7e41946,// 55 PAY  52 

    0x74725900,// 56 PAY  53 

    0x5e47541f,// 57 PAY  54 

    0x9ae36ee2,// 58 PAY  55 

    0x95576c18,// 59 PAY  56 

    0xe2df1927,// 60 PAY  57 

    0x615a0737,// 61 PAY  58 

    0x78998034,// 62 PAY  59 

    0x1f4e3808,// 63 PAY  60 

    0x210da937,// 64 PAY  61 

    0xc559d79a,// 65 PAY  62 

    0xc869bf8c,// 66 PAY  63 

    0x5ad68b14,// 67 PAY  64 

    0x700a2e61,// 68 PAY  65 

    0xe80d31e2,// 69 PAY  66 

    0x1aa0f2f4,// 70 PAY  67 

    0x39ef925f,// 71 PAY  68 

    0xb0d43c54,// 72 PAY  69 

    0x1feb8f51,// 73 PAY  70 

    0x24c42a2d,// 74 PAY  71 

    0x3453fdcc,// 75 PAY  72 

    0xf6eb4c21,// 76 PAY  73 

    0xbedbf6a2,// 77 PAY  74 

    0xbdebc1e7,// 78 PAY  75 

    0x59b7922a,// 79 PAY  76 

    0x108b024c,// 80 PAY  77 

    0x3a69fdb2,// 81 PAY  78 

    0x53c889ec,// 82 PAY  79 

    0x7ec1fa2b,// 83 PAY  80 

    0x608290c6,// 84 PAY  81 

    0x9d2d34f2,// 85 PAY  82 

    0xacb57b9e,// 86 PAY  83 

    0xa6fa7f46,// 87 PAY  84 

    0x88e65cc3,// 88 PAY  85 

/// STA is 1 words. 

/// STA num_pkts       : 196 

/// STA pkt_idx        : 51 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x52 

    0x00cc52c4 // 89 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt51_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 145 words. 

/// BDA size     is 576 (0x240) 

/// BDA id       is 0x30ad 

    0x024030ad,// 3 BDA   1 

/// PAY Generic Data size   : 576 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x0fcef435,// 4 PAY   1 

    0x54cf58fe,// 5 PAY   2 

    0xe9164c5d,// 6 PAY   3 

    0x7008bf82,// 7 PAY   4 

    0x4a221cb9,// 8 PAY   5 

    0xc2ed0dd5,// 9 PAY   6 

    0x8a81692f,// 10 PAY   7 

    0xf012513b,// 11 PAY   8 

    0xaaebf778,// 12 PAY   9 

    0xb76766a4,// 13 PAY  10 

    0x87fbfb3c,// 14 PAY  11 

    0x5564b8fa,// 15 PAY  12 

    0xc05f84a2,// 16 PAY  13 

    0x260d229d,// 17 PAY  14 

    0xad880507,// 18 PAY  15 

    0x8d990a56,// 19 PAY  16 

    0xab8555ba,// 20 PAY  17 

    0xf04cec25,// 21 PAY  18 

    0x5d5e55d1,// 22 PAY  19 

    0x19014806,// 23 PAY  20 

    0xff07264c,// 24 PAY  21 

    0xbec0bc74,// 25 PAY  22 

    0x251a55b4,// 26 PAY  23 

    0x801b76cf,// 27 PAY  24 

    0x99ac9c0e,// 28 PAY  25 

    0xc0bf617c,// 29 PAY  26 

    0x74b194f0,// 30 PAY  27 

    0x712be474,// 31 PAY  28 

    0x60dc58f2,// 32 PAY  29 

    0xa2f2820b,// 33 PAY  30 

    0x7ebc74d9,// 34 PAY  31 

    0x06e2de12,// 35 PAY  32 

    0x10a8c025,// 36 PAY  33 

    0x1594a347,// 37 PAY  34 

    0xb5712d84,// 38 PAY  35 

    0xc9a4d9a3,// 39 PAY  36 

    0x92cd1111,// 40 PAY  37 

    0x55c9c063,// 41 PAY  38 

    0x832f943e,// 42 PAY  39 

    0x77aa3f05,// 43 PAY  40 

    0x25945e05,// 44 PAY  41 

    0x7a5bac42,// 45 PAY  42 

    0xbc5f324d,// 46 PAY  43 

    0x7084449c,// 47 PAY  44 

    0xa7d376fc,// 48 PAY  45 

    0x9c5e4c54,// 49 PAY  46 

    0xe11609ea,// 50 PAY  47 

    0x6b90d38c,// 51 PAY  48 

    0xc6685087,// 52 PAY  49 

    0xa3257a64,// 53 PAY  50 

    0x0920d264,// 54 PAY  51 

    0xd9100643,// 55 PAY  52 

    0x29d60830,// 56 PAY  53 

    0xc88cae56,// 57 PAY  54 

    0x6630df2e,// 58 PAY  55 

    0x5d8d49f8,// 59 PAY  56 

    0x7ad6eb06,// 60 PAY  57 

    0x8f2d2303,// 61 PAY  58 

    0x9853c180,// 62 PAY  59 

    0x91636202,// 63 PAY  60 

    0xcf6eaa6d,// 64 PAY  61 

    0x3fbc5e6c,// 65 PAY  62 

    0x39b44015,// 66 PAY  63 

    0x47988620,// 67 PAY  64 

    0xb5435666,// 68 PAY  65 

    0xb9b411e1,// 69 PAY  66 

    0x5e40a4a8,// 70 PAY  67 

    0x30c45e9a,// 71 PAY  68 

    0x47ac555e,// 72 PAY  69 

    0x2c89cd33,// 73 PAY  70 

    0x43f1486e,// 74 PAY  71 

    0x3e84c989,// 75 PAY  72 

    0x80a24316,// 76 PAY  73 

    0xe5fdc6cb,// 77 PAY  74 

    0x944e9e99,// 78 PAY  75 

    0x35510ce3,// 79 PAY  76 

    0x9f1def68,// 80 PAY  77 

    0x6a2ba5ee,// 81 PAY  78 

    0x6b88f7fb,// 82 PAY  79 

    0x816c36a2,// 83 PAY  80 

    0xa7e901e1,// 84 PAY  81 

    0x5406d183,// 85 PAY  82 

    0x29a878ba,// 86 PAY  83 

    0xa1f1d0fe,// 87 PAY  84 

    0xc72dfcae,// 88 PAY  85 

    0x09787d76,// 89 PAY  86 

    0x516d8d95,// 90 PAY  87 

    0x422d17f7,// 91 PAY  88 

    0x1de57522,// 92 PAY  89 

    0x4a54d39b,// 93 PAY  90 

    0xfb6aee31,// 94 PAY  91 

    0xa0b83974,// 95 PAY  92 

    0x60125f3e,// 96 PAY  93 

    0xc2c68a76,// 97 PAY  94 

    0x24be1095,// 98 PAY  95 

    0x0b22f368,// 99 PAY  96 

    0x32049acc,// 100 PAY  97 

    0x281c4cff,// 101 PAY  98 

    0xa54210d5,// 102 PAY  99 

    0x185a63a1,// 103 PAY 100 

    0x5a446acf,// 104 PAY 101 

    0xa303447a,// 105 PAY 102 

    0xaf3677e0,// 106 PAY 103 

    0x0163c21f,// 107 PAY 104 

    0xae964806,// 108 PAY 105 

    0x4ca13552,// 109 PAY 106 

    0xb5f70cf5,// 110 PAY 107 

    0x0f6d49bd,// 111 PAY 108 

    0xbe1beca2,// 112 PAY 109 

    0xdd897920,// 113 PAY 110 

    0x83c610f9,// 114 PAY 111 

    0xd7bd2f08,// 115 PAY 112 

    0x5e2197c8,// 116 PAY 113 

    0xdfa9c04d,// 117 PAY 114 

    0xc484598b,// 118 PAY 115 

    0xa57f7cfd,// 119 PAY 116 

    0x587f72e6,// 120 PAY 117 

    0x3adadad5,// 121 PAY 118 

    0x318786dc,// 122 PAY 119 

    0x2f843933,// 123 PAY 120 

    0xff68ed61,// 124 PAY 121 

    0x564732b1,// 125 PAY 122 

    0xccf25fd6,// 126 PAY 123 

    0x5fbd8021,// 127 PAY 124 

    0x24af0c53,// 128 PAY 125 

    0x3cdd28e8,// 129 PAY 126 

    0x9ad1442a,// 130 PAY 127 

    0xdf5125bb,// 131 PAY 128 

    0x048563a0,// 132 PAY 129 

    0x6e2065c6,// 133 PAY 130 

    0x3ff96cf5,// 134 PAY 131 

    0x2b3434cf,// 135 PAY 132 

    0x64d46f28,// 136 PAY 133 

    0x985c0501,// 137 PAY 134 

    0xb318ba70,// 138 PAY 135 

    0xfded9546,// 139 PAY 136 

    0xe2f06790,// 140 PAY 137 

    0x097bf0eb,// 141 PAY 138 

    0x10c6bd6b,// 142 PAY 139 

    0xd9192ebd,// 143 PAY 140 

    0x1ab29a7f,// 144 PAY 141 

    0x3aefc132,// 145 PAY 142 

    0x22f3dc69,// 146 PAY 143 

    0x69d042a9,// 147 PAY 144 

/// HASH is  4 bytes 

    0xb318ba70,// 148 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 59 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb6 

    0x0050b63b // 149 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt52_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 205 words. 

/// BDA size     is 816 (0x330) 

/// BDA id       is 0xa607 

    0x0330a607,// 3 BDA   1 

/// PAY Generic Data size   : 816 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xdd46e1b1,// 4 PAY   1 

    0x30021120,// 5 PAY   2 

    0xf6159bd2,// 6 PAY   3 

    0xc739590e,// 7 PAY   4 

    0x15169c3c,// 8 PAY   5 

    0x013816ec,// 9 PAY   6 

    0x22ada436,// 10 PAY   7 

    0x46b0a2a1,// 11 PAY   8 

    0xac5f19ee,// 12 PAY   9 

    0x25d73452,// 13 PAY  10 

    0x6c82281f,// 14 PAY  11 

    0x88bb542a,// 15 PAY  12 

    0x79626efa,// 16 PAY  13 

    0xa93e0f92,// 17 PAY  14 

    0x09642e30,// 18 PAY  15 

    0xab237e2a,// 19 PAY  16 

    0x46bd66dd,// 20 PAY  17 

    0x46641c5f,// 21 PAY  18 

    0xfb6457cb,// 22 PAY  19 

    0x6dac559e,// 23 PAY  20 

    0xc4898bc5,// 24 PAY  21 

    0xda157dcc,// 25 PAY  22 

    0x2166fc0f,// 26 PAY  23 

    0x199e6853,// 27 PAY  24 

    0x4e1b1898,// 28 PAY  25 

    0x0aadf119,// 29 PAY  26 

    0xc1aae2ca,// 30 PAY  27 

    0xa5e3662e,// 31 PAY  28 

    0x0efffb24,// 32 PAY  29 

    0x2edac864,// 33 PAY  30 

    0x7ee171af,// 34 PAY  31 

    0xa75e36e3,// 35 PAY  32 

    0x04ab7bfe,// 36 PAY  33 

    0x25b9545d,// 37 PAY  34 

    0xe517065a,// 38 PAY  35 

    0x9e2d09c4,// 39 PAY  36 

    0x2e3b8017,// 40 PAY  37 

    0xc53859d2,// 41 PAY  38 

    0xad16d783,// 42 PAY  39 

    0x7b6ff46e,// 43 PAY  40 

    0x2d8012b1,// 44 PAY  41 

    0xc359096c,// 45 PAY  42 

    0x9d4c246a,// 46 PAY  43 

    0xec00d4dc,// 47 PAY  44 

    0xd2b55b4d,// 48 PAY  45 

    0x52562793,// 49 PAY  46 

    0xb3026eaa,// 50 PAY  47 

    0xee6deb43,// 51 PAY  48 

    0xa71c32d0,// 52 PAY  49 

    0x4d3b9435,// 53 PAY  50 

    0x67b004cc,// 54 PAY  51 

    0xaf81abe6,// 55 PAY  52 

    0x1085a78a,// 56 PAY  53 

    0x49097f6d,// 57 PAY  54 

    0xe5574a65,// 58 PAY  55 

    0x3efe2cb8,// 59 PAY  56 

    0xb31869fc,// 60 PAY  57 

    0x7748bab9,// 61 PAY  58 

    0xfdd1f192,// 62 PAY  59 

    0xe2655b35,// 63 PAY  60 

    0x1650668e,// 64 PAY  61 

    0x0c84a23b,// 65 PAY  62 

    0xff525c4f,// 66 PAY  63 

    0xe8453b78,// 67 PAY  64 

    0xc3167e4f,// 68 PAY  65 

    0xe279a113,// 69 PAY  66 

    0x865be110,// 70 PAY  67 

    0x8fae0670,// 71 PAY  68 

    0xf668c551,// 72 PAY  69 

    0xead9c557,// 73 PAY  70 

    0x389b80c2,// 74 PAY  71 

    0xe3862e41,// 75 PAY  72 

    0xd55169cd,// 76 PAY  73 

    0xb06b107f,// 77 PAY  74 

    0xc41f9252,// 78 PAY  75 

    0x20d11cd9,// 79 PAY  76 

    0x7ce8ca49,// 80 PAY  77 

    0x5f303df8,// 81 PAY  78 

    0x822b610d,// 82 PAY  79 

    0xe57df17b,// 83 PAY  80 

    0x7ab041a5,// 84 PAY  81 

    0x73779eb9,// 85 PAY  82 

    0xdc42a22f,// 86 PAY  83 

    0xbb96039a,// 87 PAY  84 

    0x7991a5fa,// 88 PAY  85 

    0xa8cb32a2,// 89 PAY  86 

    0x1901263d,// 90 PAY  87 

    0xc4a6d651,// 91 PAY  88 

    0x28f8cbb6,// 92 PAY  89 

    0xe5ad28ad,// 93 PAY  90 

    0x418f52ea,// 94 PAY  91 

    0xce4e5237,// 95 PAY  92 

    0xe6fdc1c4,// 96 PAY  93 

    0x0a461655,// 97 PAY  94 

    0xa5e48fec,// 98 PAY  95 

    0xfa8187bb,// 99 PAY  96 

    0xc502f93a,// 100 PAY  97 

    0x0882a61e,// 101 PAY  98 

    0xe735523b,// 102 PAY  99 

    0x3e2dace7,// 103 PAY 100 

    0xb84ad0c1,// 104 PAY 101 

    0x8e1a66be,// 105 PAY 102 

    0x7188d72b,// 106 PAY 103 

    0xe6e34ce1,// 107 PAY 104 

    0x37b628f5,// 108 PAY 105 

    0x4980440f,// 109 PAY 106 

    0x01c7b92a,// 110 PAY 107 

    0xe664f3a7,// 111 PAY 108 

    0xaecb6eb0,// 112 PAY 109 

    0xae2a9dff,// 113 PAY 110 

    0xc26cc930,// 114 PAY 111 

    0x02a9a0b0,// 115 PAY 112 

    0x158d5fb9,// 116 PAY 113 

    0xbe1411e8,// 117 PAY 114 

    0xa7b9d3a7,// 118 PAY 115 

    0xac7f5a8b,// 119 PAY 116 

    0x4a610f49,// 120 PAY 117 

    0x0fde8869,// 121 PAY 118 

    0x7be01434,// 122 PAY 119 

    0xd9426f70,// 123 PAY 120 

    0x8d730ed8,// 124 PAY 121 

    0xdd7d9c71,// 125 PAY 122 

    0xe98634e8,// 126 PAY 123 

    0x29533676,// 127 PAY 124 

    0xa90e75bd,// 128 PAY 125 

    0x81a7f98f,// 129 PAY 126 

    0x0f295f09,// 130 PAY 127 

    0x0947494c,// 131 PAY 128 

    0x49900d4c,// 132 PAY 129 

    0xab4de5d8,// 133 PAY 130 

    0x0a98ddde,// 134 PAY 131 

    0x5fc98489,// 135 PAY 132 

    0x6d15f65b,// 136 PAY 133 

    0xa040ffcf,// 137 PAY 134 

    0x5723b105,// 138 PAY 135 

    0x3b1722ad,// 139 PAY 136 

    0x3f1c32ac,// 140 PAY 137 

    0x627e63da,// 141 PAY 138 

    0xcc808fbb,// 142 PAY 139 

    0x060679c1,// 143 PAY 140 

    0x511e28da,// 144 PAY 141 

    0xca2b2cf4,// 145 PAY 142 

    0x6cd8237d,// 146 PAY 143 

    0xcba09d69,// 147 PAY 144 

    0x56afca8e,// 148 PAY 145 

    0xc5110496,// 149 PAY 146 

    0xa4a001c0,// 150 PAY 147 

    0x41b05bf1,// 151 PAY 148 

    0x73081d6c,// 152 PAY 149 

    0xd1aec8da,// 153 PAY 150 

    0xfb25a58e,// 154 PAY 151 

    0xac9e9c3f,// 155 PAY 152 

    0x6d6875f8,// 156 PAY 153 

    0xc4761dc5,// 157 PAY 154 

    0x573e0a3e,// 158 PAY 155 

    0x24f40a4a,// 159 PAY 156 

    0x3b1da451,// 160 PAY 157 

    0x78b30c7d,// 161 PAY 158 

    0xc974cfb8,// 162 PAY 159 

    0x48850a22,// 163 PAY 160 

    0x6b613a6c,// 164 PAY 161 

    0xb1496cb5,// 165 PAY 162 

    0x50ce8b9b,// 166 PAY 163 

    0xb204f2b2,// 167 PAY 164 

    0xfe1b95bc,// 168 PAY 165 

    0xe921d005,// 169 PAY 166 

    0x26e159e0,// 170 PAY 167 

    0x6494d338,// 171 PAY 168 

    0xd1487f80,// 172 PAY 169 

    0xd75f6ab3,// 173 PAY 170 

    0xc1bce716,// 174 PAY 171 

    0x29beb910,// 175 PAY 172 

    0x1829893e,// 176 PAY 173 

    0x11d6499a,// 177 PAY 174 

    0x03bd9395,// 178 PAY 175 

    0x6868d878,// 179 PAY 176 

    0xe23e0697,// 180 PAY 177 

    0xb14ad48c,// 181 PAY 178 

    0x9561ed21,// 182 PAY 179 

    0x6688c3a7,// 183 PAY 180 

    0x322057be,// 184 PAY 181 

    0x908ab2c2,// 185 PAY 182 

    0xc984d9c2,// 186 PAY 183 

    0x69e0769b,// 187 PAY 184 

    0x9e61fce5,// 188 PAY 185 

    0x3841d801,// 189 PAY 186 

    0x2024046a,// 190 PAY 187 

    0xc81979e7,// 191 PAY 188 

    0xd29f1479,// 192 PAY 189 

    0x5e67d05e,// 193 PAY 190 

    0xb7a000ce,// 194 PAY 191 

    0xc54cec2d,// 195 PAY 192 

    0xb5983353,// 196 PAY 193 

    0xadb8f212,// 197 PAY 194 

    0x1105bd4f,// 198 PAY 195 

    0x01b8c113,// 199 PAY 196 

    0x07bf3812,// 200 PAY 197 

    0xdd6858e0,// 201 PAY 198 

    0x7b9ae298,// 202 PAY 199 

    0xd669dd5d,// 203 PAY 200 

    0x504ba28e,// 204 PAY 201 

    0x2a601506,// 205 PAY 202 

    0x07969e37,// 206 PAY 203 

    0x69237dcc,// 207 PAY 204 

/// STA is 1 words. 

/// STA num_pkts       : 163 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x58 

    0x03d958a3 // 208 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt53_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 354 words. 

/// BDA size     is 1409 (0x581) 

/// BDA id       is 0x7e9d 

    0x05817e9d,// 3 BDA   1 

/// PAY Generic Data size   : 1409 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x19601f5d,// 4 PAY   1 

    0xce270785,// 5 PAY   2 

    0xe1386fb8,// 6 PAY   3 

    0xf83db524,// 7 PAY   4 

    0xc4eeddc2,// 8 PAY   5 

    0x4e043b89,// 9 PAY   6 

    0x0b80c2c2,// 10 PAY   7 

    0x4c5a7fd3,// 11 PAY   8 

    0x38ffb19b,// 12 PAY   9 

    0x708be665,// 13 PAY  10 

    0x7d631158,// 14 PAY  11 

    0xa008bacf,// 15 PAY  12 

    0xc931ad6f,// 16 PAY  13 

    0x2283e803,// 17 PAY  14 

    0xf9861538,// 18 PAY  15 

    0x3164f625,// 19 PAY  16 

    0x68bd3f34,// 20 PAY  17 

    0x516f6ef9,// 21 PAY  18 

    0x1f05a1b7,// 22 PAY  19 

    0x5ebbd93d,// 23 PAY  20 

    0xb0eccdd0,// 24 PAY  21 

    0xcf9581cb,// 25 PAY  22 

    0x96bb3f6a,// 26 PAY  23 

    0xc71469bd,// 27 PAY  24 

    0x1f1950ac,// 28 PAY  25 

    0x9a0bf046,// 29 PAY  26 

    0x5b773d81,// 30 PAY  27 

    0xce01c1e3,// 31 PAY  28 

    0x317a689a,// 32 PAY  29 

    0xce55d7b3,// 33 PAY  30 

    0x1fdb304e,// 34 PAY  31 

    0xa833e68f,// 35 PAY  32 

    0x2ca75080,// 36 PAY  33 

    0xf08ff185,// 37 PAY  34 

    0x64059044,// 38 PAY  35 

    0xe685a3eb,// 39 PAY  36 

    0x78e8c918,// 40 PAY  37 

    0x12a0638a,// 41 PAY  38 

    0xd762467a,// 42 PAY  39 

    0x3218e553,// 43 PAY  40 

    0x51c88913,// 44 PAY  41 

    0xd81ca7f5,// 45 PAY  42 

    0x9af25167,// 46 PAY  43 

    0x9b6030ba,// 47 PAY  44 

    0xe111ad6e,// 48 PAY  45 

    0x2adadee9,// 49 PAY  46 

    0xeaa4ec22,// 50 PAY  47 

    0x0c532d62,// 51 PAY  48 

    0xd2014e5d,// 52 PAY  49 

    0xfc0b57a5,// 53 PAY  50 

    0x061759d0,// 54 PAY  51 

    0xdf953b18,// 55 PAY  52 

    0xe8237242,// 56 PAY  53 

    0x4e025058,// 57 PAY  54 

    0x69ace428,// 58 PAY  55 

    0xede6749a,// 59 PAY  56 

    0xf7803ad0,// 60 PAY  57 

    0x23307823,// 61 PAY  58 

    0x48c18739,// 62 PAY  59 

    0x6b093367,// 63 PAY  60 

    0x3979742f,// 64 PAY  61 

    0xe24452bd,// 65 PAY  62 

    0x9e875625,// 66 PAY  63 

    0x53169fa3,// 67 PAY  64 

    0x3173a6ee,// 68 PAY  65 

    0x3b540622,// 69 PAY  66 

    0xa4b22514,// 70 PAY  67 

    0x01ca2e3f,// 71 PAY  68 

    0x1d032f24,// 72 PAY  69 

    0xc6393708,// 73 PAY  70 

    0xcc1de6d1,// 74 PAY  71 

    0xf82d18b8,// 75 PAY  72 

    0xf69cc327,// 76 PAY  73 

    0xb403bf30,// 77 PAY  74 

    0x48863c68,// 78 PAY  75 

    0x379107d5,// 79 PAY  76 

    0x2ddbe12a,// 80 PAY  77 

    0x0ccc25a7,// 81 PAY  78 

    0xa0beb03e,// 82 PAY  79 

    0x3236d002,// 83 PAY  80 

    0x071265dc,// 84 PAY  81 

    0x1009f2b5,// 85 PAY  82 

    0xde87c467,// 86 PAY  83 

    0x506a3691,// 87 PAY  84 

    0x0bf827d4,// 88 PAY  85 

    0x3052a5d7,// 89 PAY  86 

    0x40c1b5af,// 90 PAY  87 

    0x7896b2dd,// 91 PAY  88 

    0x3808cf8b,// 92 PAY  89 

    0x572f145b,// 93 PAY  90 

    0xe0747449,// 94 PAY  91 

    0x5fff8984,// 95 PAY  92 

    0x6accb74e,// 96 PAY  93 

    0x8f91bd36,// 97 PAY  94 

    0xe9723df8,// 98 PAY  95 

    0xd16821e2,// 99 PAY  96 

    0x9edefd66,// 100 PAY  97 

    0xaa6c38e3,// 101 PAY  98 

    0x3216285b,// 102 PAY  99 

    0x2ec91ac6,// 103 PAY 100 

    0x476a68f2,// 104 PAY 101 

    0x3c084220,// 105 PAY 102 

    0x0f8ebd3b,// 106 PAY 103 

    0xca15bfa6,// 107 PAY 104 

    0xc8326a7c,// 108 PAY 105 

    0x7d67ccc2,// 109 PAY 106 

    0x61a20a57,// 110 PAY 107 

    0x8c1222dc,// 111 PAY 108 

    0x20cb5ad4,// 112 PAY 109 

    0x7c533373,// 113 PAY 110 

    0xf823da58,// 114 PAY 111 

    0x7d68b442,// 115 PAY 112 

    0x8b959320,// 116 PAY 113 

    0x4702f6ca,// 117 PAY 114 

    0x10f1d532,// 118 PAY 115 

    0xcacaba03,// 119 PAY 116 

    0xaccceb16,// 120 PAY 117 

    0xf7eabfb0,// 121 PAY 118 

    0xa82a5ebb,// 122 PAY 119 

    0xe96a011d,// 123 PAY 120 

    0x491cfe03,// 124 PAY 121 

    0x1ccde768,// 125 PAY 122 

    0x17ea599a,// 126 PAY 123 

    0x641f73c8,// 127 PAY 124 

    0x72d7ad10,// 128 PAY 125 

    0x8d489b22,// 129 PAY 126 

    0x9f44bffe,// 130 PAY 127 

    0x933a1643,// 131 PAY 128 

    0x2c60fdf2,// 132 PAY 129 

    0x346c7219,// 133 PAY 130 

    0x0c637fa6,// 134 PAY 131 

    0xf6fb8cd5,// 135 PAY 132 

    0x0632edda,// 136 PAY 133 

    0xbe692ea1,// 137 PAY 134 

    0xa270d533,// 138 PAY 135 

    0x105f2e7f,// 139 PAY 136 

    0x19f84323,// 140 PAY 137 

    0xd8ce0413,// 141 PAY 138 

    0x1ce03311,// 142 PAY 139 

    0xf9b6685f,// 143 PAY 140 

    0x52d663d4,// 144 PAY 141 

    0x250daa0a,// 145 PAY 142 

    0xda0740b5,// 146 PAY 143 

    0x1f695d2f,// 147 PAY 144 

    0xc3715699,// 148 PAY 145 

    0x14a81fc0,// 149 PAY 146 

    0x66b56dda,// 150 PAY 147 

    0x9be7b69c,// 151 PAY 148 

    0x220504ab,// 152 PAY 149 

    0x1ae51322,// 153 PAY 150 

    0x8219484a,// 154 PAY 151 

    0xb47baafe,// 155 PAY 152 

    0x7d94aa78,// 156 PAY 153 

    0x1d163a5a,// 157 PAY 154 

    0xd532c895,// 158 PAY 155 

    0x0f4f0d75,// 159 PAY 156 

    0x2075a43a,// 160 PAY 157 

    0x319640a5,// 161 PAY 158 

    0xa6301a26,// 162 PAY 159 

    0x7611ae09,// 163 PAY 160 

    0x550b6e7d,// 164 PAY 161 

    0x5fdbeccf,// 165 PAY 162 

    0x492ce344,// 166 PAY 163 

    0x40d03a75,// 167 PAY 164 

    0x3ce970a9,// 168 PAY 165 

    0xa8b14edf,// 169 PAY 166 

    0xd70dc949,// 170 PAY 167 

    0x187fee71,// 171 PAY 168 

    0x70560a62,// 172 PAY 169 

    0xe1eb9a38,// 173 PAY 170 

    0x66419d37,// 174 PAY 171 

    0xf99a7994,// 175 PAY 172 

    0x790d9a8c,// 176 PAY 173 

    0x3b3132f2,// 177 PAY 174 

    0x9dd4d428,// 178 PAY 175 

    0x4f3e9f8f,// 179 PAY 176 

    0xf5dba66d,// 180 PAY 177 

    0x707fc661,// 181 PAY 178 

    0x5bbf2b8e,// 182 PAY 179 

    0x0660c5d2,// 183 PAY 180 

    0xa5bcc2b5,// 184 PAY 181 

    0xb16ebcfa,// 185 PAY 182 

    0x97ab51c4,// 186 PAY 183 

    0x78ac5423,// 187 PAY 184 

    0x51b7dca2,// 188 PAY 185 

    0xe41d08ab,// 189 PAY 186 

    0x1e831eda,// 190 PAY 187 

    0x19fb87d3,// 191 PAY 188 

    0x0fecb00c,// 192 PAY 189 

    0xd07fc0f5,// 193 PAY 190 

    0xec176276,// 194 PAY 191 

    0x23125f0b,// 195 PAY 192 

    0x8f91b42f,// 196 PAY 193 

    0xe58ae3f4,// 197 PAY 194 

    0x8471ccd9,// 198 PAY 195 

    0x23d90500,// 199 PAY 196 

    0x0891fe32,// 200 PAY 197 

    0xb17152dc,// 201 PAY 198 

    0xb3808d6c,// 202 PAY 199 

    0x6ba1bb19,// 203 PAY 200 

    0xe67cd7da,// 204 PAY 201 

    0x9cc6e583,// 205 PAY 202 

    0xab5bafcd,// 206 PAY 203 

    0xeb047b13,// 207 PAY 204 

    0xf0d4c70a,// 208 PAY 205 

    0x2cd3f1fd,// 209 PAY 206 

    0xdfc92ec6,// 210 PAY 207 

    0x552fa2bb,// 211 PAY 208 

    0x29a826a1,// 212 PAY 209 

    0x218fce20,// 213 PAY 210 

    0x43d5aa50,// 214 PAY 211 

    0x035c8a6f,// 215 PAY 212 

    0x4d6fd332,// 216 PAY 213 

    0x3e2cdb84,// 217 PAY 214 

    0x1cc7cbcc,// 218 PAY 215 

    0x90d070db,// 219 PAY 216 

    0xab11b495,// 220 PAY 217 

    0x8a726844,// 221 PAY 218 

    0xd1e36e84,// 222 PAY 219 

    0x1066c867,// 223 PAY 220 

    0x42ce24c0,// 224 PAY 221 

    0xe8a5a3ee,// 225 PAY 222 

    0x151feb9c,// 226 PAY 223 

    0x98c639b8,// 227 PAY 224 

    0xc6c77aae,// 228 PAY 225 

    0xe01503df,// 229 PAY 226 

    0xe00597fb,// 230 PAY 227 

    0x36fc855a,// 231 PAY 228 

    0x09845ae9,// 232 PAY 229 

    0x81528a47,// 233 PAY 230 

    0x7a848921,// 234 PAY 231 

    0x6ca9a76d,// 235 PAY 232 

    0x789eacdf,// 236 PAY 233 

    0x22741b2a,// 237 PAY 234 

    0xbaf1ac56,// 238 PAY 235 

    0xd4ee3554,// 239 PAY 236 

    0x31ceaf39,// 240 PAY 237 

    0xfac5f32d,// 241 PAY 238 

    0xb204a1bc,// 242 PAY 239 

    0x5312d6d3,// 243 PAY 240 

    0x06461ab5,// 244 PAY 241 

    0xc7ea5bd9,// 245 PAY 242 

    0xde1a995d,// 246 PAY 243 

    0x55c52802,// 247 PAY 244 

    0x42136e05,// 248 PAY 245 

    0x74955eb0,// 249 PAY 246 

    0x00b61469,// 250 PAY 247 

    0xb6e810ba,// 251 PAY 248 

    0x4d56e1eb,// 252 PAY 249 

    0x4ade8602,// 253 PAY 250 

    0x9695cc7d,// 254 PAY 251 

    0x47f5266b,// 255 PAY 252 

    0xf05d4354,// 256 PAY 253 

    0xadcf17ba,// 257 PAY 254 

    0xb39651b9,// 258 PAY 255 

    0x96a1e0d2,// 259 PAY 256 

    0xae14828f,// 260 PAY 257 

    0xd18df01e,// 261 PAY 258 

    0x34106858,// 262 PAY 259 

    0x6be25418,// 263 PAY 260 

    0xcd70a870,// 264 PAY 261 

    0xfe324e89,// 265 PAY 262 

    0x5eed4746,// 266 PAY 263 

    0x3a2bb202,// 267 PAY 264 

    0x9c246873,// 268 PAY 265 

    0xb0c49d6e,// 269 PAY 266 

    0x33ed7c68,// 270 PAY 267 

    0x2ec5e5cf,// 271 PAY 268 

    0xb7fd8d79,// 272 PAY 269 

    0xb18e3773,// 273 PAY 270 

    0x92831117,// 274 PAY 271 

    0x388abe82,// 275 PAY 272 

    0x1562c09d,// 276 PAY 273 

    0xf7abc442,// 277 PAY 274 

    0xd9a93577,// 278 PAY 275 

    0x8a8d6caf,// 279 PAY 276 

    0xa8925dd0,// 280 PAY 277 

    0x738f1fce,// 281 PAY 278 

    0xa99eac87,// 282 PAY 279 

    0x1c32617f,// 283 PAY 280 

    0x092595fd,// 284 PAY 281 

    0xfd658d83,// 285 PAY 282 

    0x44d06f5f,// 286 PAY 283 

    0xc3abb735,// 287 PAY 284 

    0xaddc7d59,// 288 PAY 285 

    0x148636ad,// 289 PAY 286 

    0x4e133cac,// 290 PAY 287 

    0x6449e8be,// 291 PAY 288 

    0x776df8b1,// 292 PAY 289 

    0xb990f0ee,// 293 PAY 290 

    0x8a2e3dc6,// 294 PAY 291 

    0x4627b294,// 295 PAY 292 

    0x846e3a33,// 296 PAY 293 

    0x58f984f4,// 297 PAY 294 

    0x2182984e,// 298 PAY 295 

    0x311b307a,// 299 PAY 296 

    0x2b40ef04,// 300 PAY 297 

    0x463aaec9,// 301 PAY 298 

    0xe8c269a4,// 302 PAY 299 

    0x6a312146,// 303 PAY 300 

    0x944f5e19,// 304 PAY 301 

    0xfe819aec,// 305 PAY 302 

    0xd090da5b,// 306 PAY 303 

    0xec25a455,// 307 PAY 304 

    0xf950028a,// 308 PAY 305 

    0xd7854dc4,// 309 PAY 306 

    0xa0d3479c,// 310 PAY 307 

    0x0166b1e6,// 311 PAY 308 

    0x1040b6ab,// 312 PAY 309 

    0x88c6dad6,// 313 PAY 310 

    0xa0d29a23,// 314 PAY 311 

    0x142aa939,// 315 PAY 312 

    0x976b9493,// 316 PAY 313 

    0x1f7fbd0a,// 317 PAY 314 

    0x33e32944,// 318 PAY 315 

    0x7c540825,// 319 PAY 316 

    0x32bb54a0,// 320 PAY 317 

    0x1846e7e6,// 321 PAY 318 

    0xab66ef16,// 322 PAY 319 

    0xc332dca5,// 323 PAY 320 

    0xc8736184,// 324 PAY 321 

    0x8fce97b8,// 325 PAY 322 

    0xaa35a01c,// 326 PAY 323 

    0xfd7febf6,// 327 PAY 324 

    0x524c769b,// 328 PAY 325 

    0xeb58543f,// 329 PAY 326 

    0x613d59f7,// 330 PAY 327 

    0xb811f535,// 331 PAY 328 

    0x99d32fa9,// 332 PAY 329 

    0x3f2e8bc2,// 333 PAY 330 

    0xce43b0b5,// 334 PAY 331 

    0x96b1e478,// 335 PAY 332 

    0x4018041d,// 336 PAY 333 

    0xc586d5d8,// 337 PAY 334 

    0x46dd7ce1,// 338 PAY 335 

    0x3be88ba5,// 339 PAY 336 

    0xfcabeb34,// 340 PAY 337 

    0x5232c3d1,// 341 PAY 338 

    0x9b0e49c7,// 342 PAY 339 

    0x628c7e58,// 343 PAY 340 

    0x74e764c7,// 344 PAY 341 

    0xe01ca325,// 345 PAY 342 

    0xbffc8be5,// 346 PAY 343 

    0x456acc37,// 347 PAY 344 

    0x921523eb,// 348 PAY 345 

    0x59a883b8,// 349 PAY 346 

    0xae060f1a,// 350 PAY 347 

    0xedccbadf,// 351 PAY 348 

    0x491617cd,// 352 PAY 349 

    0x805c9d55,// 353 PAY 350 

    0x2ab5026b,// 354 PAY 351 

    0x8c16706a,// 355 PAY 352 

    0x4b000000,// 356 PAY 353 

/// HASH is  12 bytes 

    0x44d06f5f,// 357 HSH   1 

    0xc3abb735,// 358 HSH   2 

    0xaddc7d59,// 359 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 44 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf8 

    0x00b1f806 // 360 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt54_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 37 words. 

/// BDA size     is 142 (0x8e) 

/// BDA id       is 0x5aee 

    0x008e5aee,// 3 BDA   1 

/// PAY Generic Data size   : 142 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x62719e97,// 4 PAY   1 

    0x80b9016b,// 5 PAY   2 

    0xdc939dde,// 6 PAY   3 

    0x8b25c222,// 7 PAY   4 

    0x8174cfb3,// 8 PAY   5 

    0xac94266d,// 9 PAY   6 

    0x0cfe7878,// 10 PAY   7 

    0x76d9e780,// 11 PAY   8 

    0x9a19e28c,// 12 PAY   9 

    0x4e4affa8,// 13 PAY  10 

    0xd518e76c,// 14 PAY  11 

    0x950ad194,// 15 PAY  12 

    0x75e2cb05,// 16 PAY  13 

    0xa472e3d4,// 17 PAY  14 

    0x755b0f0b,// 18 PAY  15 

    0xf1be880e,// 19 PAY  16 

    0xa9520bbe,// 20 PAY  17 

    0x776436cd,// 21 PAY  18 

    0xcc2d9894,// 22 PAY  19 

    0x1fb09284,// 23 PAY  20 

    0xdaf31eb5,// 24 PAY  21 

    0xbdddea39,// 25 PAY  22 

    0xca9bed7d,// 26 PAY  23 

    0xfa6c2331,// 27 PAY  24 

    0xcf20d58f,// 28 PAY  25 

    0x1e911418,// 29 PAY  26 

    0xd288b999,// 30 PAY  27 

    0x1659562e,// 31 PAY  28 

    0x49261d00,// 32 PAY  29 

    0xaa99fc4b,// 33 PAY  30 

    0x579cfb26,// 34 PAY  31 

    0x3012b63d,// 35 PAY  32 

    0x5c1aa200,// 36 PAY  33 

    0xecbb4ba4,// 37 PAY  34 

    0x8b16af66,// 38 PAY  35 

    0x9c760000,// 39 PAY  36 

/// HASH is  12 bytes 

    0xdaf31eb5,// 40 HSH   1 

    0xbdddea39,// 41 HSH   2 

    0xca9bed7d,// 42 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 33 

/// STA pkt_idx        : 109 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x19 

    0x01b41921 // 43 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt55_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 266 words. 

/// BDA size     is 1058 (0x422) 

/// BDA id       is 0x49d1 

    0x042249d1,// 3 BDA   1 

/// PAY Generic Data size   : 1058 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x0869385a,// 4 PAY   1 

    0x05f6f9b4,// 5 PAY   2 

    0x259c9bad,// 6 PAY   3 

    0x31094dcf,// 7 PAY   4 

    0x7995399b,// 8 PAY   5 

    0xebd5e52d,// 9 PAY   6 

    0x04d41b51,// 10 PAY   7 

    0x363a0ff9,// 11 PAY   8 

    0x2493ed93,// 12 PAY   9 

    0xc62baef2,// 13 PAY  10 

    0xc1c58960,// 14 PAY  11 

    0x9835ade0,// 15 PAY  12 

    0x04eca6d6,// 16 PAY  13 

    0x7bed45d0,// 17 PAY  14 

    0xcaa9cecf,// 18 PAY  15 

    0xc2242854,// 19 PAY  16 

    0xc413d807,// 20 PAY  17 

    0x67802036,// 21 PAY  18 

    0xb3727935,// 22 PAY  19 

    0x41c84e23,// 23 PAY  20 

    0x283abb81,// 24 PAY  21 

    0xe950bbb1,// 25 PAY  22 

    0x545b81a5,// 26 PAY  23 

    0xb24a35ed,// 27 PAY  24 

    0xfbaa33fe,// 28 PAY  25 

    0x1a7f4188,// 29 PAY  26 

    0x8c1ef479,// 30 PAY  27 

    0xfafc9284,// 31 PAY  28 

    0x7917324f,// 32 PAY  29 

    0x3b604cca,// 33 PAY  30 

    0x32f0aab3,// 34 PAY  31 

    0xb9cf25f7,// 35 PAY  32 

    0x5d23d7e3,// 36 PAY  33 

    0xf09c0b13,// 37 PAY  34 

    0x752c224a,// 38 PAY  35 

    0x120f9d4e,// 39 PAY  36 

    0xb3252bf1,// 40 PAY  37 

    0x7d190c67,// 41 PAY  38 

    0x59478bd8,// 42 PAY  39 

    0x3f6554a8,// 43 PAY  40 

    0xc2c6ba4a,// 44 PAY  41 

    0xcb9143b0,// 45 PAY  42 

    0x21590fa7,// 46 PAY  43 

    0xac0c61a3,// 47 PAY  44 

    0x6afd5130,// 48 PAY  45 

    0xfce586f8,// 49 PAY  46 

    0x7b61ca7f,// 50 PAY  47 

    0x67dbb848,// 51 PAY  48 

    0x25f0d6f9,// 52 PAY  49 

    0xc15b924b,// 53 PAY  50 

    0x955d8d83,// 54 PAY  51 

    0xed81f292,// 55 PAY  52 

    0xa53a85cb,// 56 PAY  53 

    0x53258650,// 57 PAY  54 

    0x7b7041fe,// 58 PAY  55 

    0x111aef7d,// 59 PAY  56 

    0x4028b971,// 60 PAY  57 

    0x60de240d,// 61 PAY  58 

    0xc9d3f58f,// 62 PAY  59 

    0xdd8f8a13,// 63 PAY  60 

    0x8fae1b05,// 64 PAY  61 

    0x579e81b0,// 65 PAY  62 

    0x31ed10ae,// 66 PAY  63 

    0x054342d8,// 67 PAY  64 

    0x087c263d,// 68 PAY  65 

    0xd099516e,// 69 PAY  66 

    0xb1d89deb,// 70 PAY  67 

    0x23766652,// 71 PAY  68 

    0x43a90b24,// 72 PAY  69 

    0xb82e85aa,// 73 PAY  70 

    0xc5c0a761,// 74 PAY  71 

    0x7c4f2d29,// 75 PAY  72 

    0x9fa37f77,// 76 PAY  73 

    0x453255f7,// 77 PAY  74 

    0xb3f8287f,// 78 PAY  75 

    0x729f9408,// 79 PAY  76 

    0x6adaf2e4,// 80 PAY  77 

    0x127c722e,// 81 PAY  78 

    0x7b8ad590,// 82 PAY  79 

    0x0bcfb195,// 83 PAY  80 

    0x2417d795,// 84 PAY  81 

    0xda305c6a,// 85 PAY  82 

    0x82e2ac26,// 86 PAY  83 

    0x8884c7a4,// 87 PAY  84 

    0xf669df3b,// 88 PAY  85 

    0xa2f80868,// 89 PAY  86 

    0xc5e41a51,// 90 PAY  87 

    0x6d0a7fa5,// 91 PAY  88 

    0xbb87202e,// 92 PAY  89 

    0xd8e7ab68,// 93 PAY  90 

    0x55b92b98,// 94 PAY  91 

    0xf0eea849,// 95 PAY  92 

    0xec9df685,// 96 PAY  93 

    0x442b7327,// 97 PAY  94 

    0x27d963d2,// 98 PAY  95 

    0x94083a88,// 99 PAY  96 

    0xa024f08e,// 100 PAY  97 

    0xf5144dd8,// 101 PAY  98 

    0xf103e264,// 102 PAY  99 

    0x42f7b65a,// 103 PAY 100 

    0x384eb0e8,// 104 PAY 101 

    0x0ec9b706,// 105 PAY 102 

    0xda3be2d5,// 106 PAY 103 

    0xf53da5dc,// 107 PAY 104 

    0xcd351952,// 108 PAY 105 

    0x4162da1d,// 109 PAY 106 

    0x1f3724dd,// 110 PAY 107 

    0x481c0fd3,// 111 PAY 108 

    0xa45c46ca,// 112 PAY 109 

    0xb5466971,// 113 PAY 110 

    0x59f03599,// 114 PAY 111 

    0x126524c6,// 115 PAY 112 

    0x0b3eb607,// 116 PAY 113 

    0x119e5168,// 117 PAY 114 

    0x26dac58f,// 118 PAY 115 

    0x94be1e38,// 119 PAY 116 

    0xb218b76d,// 120 PAY 117 

    0xe88dbb9c,// 121 PAY 118 

    0x94547a08,// 122 PAY 119 

    0x9d1988a2,// 123 PAY 120 

    0x1c8b0a46,// 124 PAY 121 

    0xb5150383,// 125 PAY 122 

    0x1bd5da4d,// 126 PAY 123 

    0xcd7299e8,// 127 PAY 124 

    0x335c1f15,// 128 PAY 125 

    0x5f47f1d8,// 129 PAY 126 

    0x1b16c3e0,// 130 PAY 127 

    0x49ea6f0f,// 131 PAY 128 

    0xc1f4d210,// 132 PAY 129 

    0xad12093e,// 133 PAY 130 

    0x0c1ea680,// 134 PAY 131 

    0x339bcabe,// 135 PAY 132 

    0x63d23729,// 136 PAY 133 

    0x1036a002,// 137 PAY 134 

    0x607235c8,// 138 PAY 135 

    0x34b84427,// 139 PAY 136 

    0xe94cc0a9,// 140 PAY 137 

    0x66b28d99,// 141 PAY 138 

    0xcc57f3ce,// 142 PAY 139 

    0x67aa5fb4,// 143 PAY 140 

    0xa97d6e10,// 144 PAY 141 

    0xe0756b7b,// 145 PAY 142 

    0x762e47b7,// 146 PAY 143 

    0x0c132e93,// 147 PAY 144 

    0x8d56c454,// 148 PAY 145 

    0x707b4b3b,// 149 PAY 146 

    0x8e71ed63,// 150 PAY 147 

    0x721776dc,// 151 PAY 148 

    0x5bc249a5,// 152 PAY 149 

    0x7d50e09c,// 153 PAY 150 

    0xdce3e026,// 154 PAY 151 

    0xd10e723a,// 155 PAY 152 

    0x81973e14,// 156 PAY 153 

    0xf69174b9,// 157 PAY 154 

    0xfc0c4aa2,// 158 PAY 155 

    0x8a7e99b2,// 159 PAY 156 

    0xecfda218,// 160 PAY 157 

    0xe7656a94,// 161 PAY 158 

    0xaeb1c5f7,// 162 PAY 159 

    0x24dd6292,// 163 PAY 160 

    0x50b7cbfd,// 164 PAY 161 

    0x0a9d4e76,// 165 PAY 162 

    0xb4ffe681,// 166 PAY 163 

    0xf87e5931,// 167 PAY 164 

    0xb0dd90c6,// 168 PAY 165 

    0x2ae5c94b,// 169 PAY 166 

    0xbeb28a88,// 170 PAY 167 

    0xb534bfec,// 171 PAY 168 

    0x765fec18,// 172 PAY 169 

    0xc9a205d8,// 173 PAY 170 

    0xe707b963,// 174 PAY 171 

    0xcb4476d5,// 175 PAY 172 

    0xcea86a0b,// 176 PAY 173 

    0x90646945,// 177 PAY 174 

    0x49d445e9,// 178 PAY 175 

    0x62031b1d,// 179 PAY 176 

    0xab833dc7,// 180 PAY 177 

    0xa185352d,// 181 PAY 178 

    0x534c4c9d,// 182 PAY 179 

    0xc04ab078,// 183 PAY 180 

    0x9cb33ee6,// 184 PAY 181 

    0x694bc334,// 185 PAY 182 

    0x86ca4bd5,// 186 PAY 183 

    0x4a43b19d,// 187 PAY 184 

    0x95f4ccf6,// 188 PAY 185 

    0x2c2e1a0b,// 189 PAY 186 

    0x761e9645,// 190 PAY 187 

    0x58c71249,// 191 PAY 188 

    0xf9d23ec7,// 192 PAY 189 

    0x8482a930,// 193 PAY 190 

    0xede6ba09,// 194 PAY 191 

    0xa01f4b47,// 195 PAY 192 

    0x5b0f7104,// 196 PAY 193 

    0x2ebbaa06,// 197 PAY 194 

    0xcaedc488,// 198 PAY 195 

    0x98438cbc,// 199 PAY 196 

    0x309fe813,// 200 PAY 197 

    0x16058508,// 201 PAY 198 

    0x689656b8,// 202 PAY 199 

    0xf045cd27,// 203 PAY 200 

    0xed319829,// 204 PAY 201 

    0xd85e52f2,// 205 PAY 202 

    0x446ade11,// 206 PAY 203 

    0xe3eda5ee,// 207 PAY 204 

    0xabd19163,// 208 PAY 205 

    0xd6dad582,// 209 PAY 206 

    0xec196071,// 210 PAY 207 

    0xe5381c25,// 211 PAY 208 

    0x09fed7ad,// 212 PAY 209 

    0xc725d3f3,// 213 PAY 210 

    0x9ed7538d,// 214 PAY 211 

    0x4e400485,// 215 PAY 212 

    0x6c8e9dc9,// 216 PAY 213 

    0x14a76f70,// 217 PAY 214 

    0x1335780f,// 218 PAY 215 

    0xf6509392,// 219 PAY 216 

    0x9a03b3eb,// 220 PAY 217 

    0x14063e31,// 221 PAY 218 

    0xfcd46151,// 222 PAY 219 

    0x851d12dd,// 223 PAY 220 

    0x49d5e4e8,// 224 PAY 221 

    0xd52e5f2a,// 225 PAY 222 

    0xa9d45a46,// 226 PAY 223 

    0xf924ecb6,// 227 PAY 224 

    0x9bb809be,// 228 PAY 225 

    0x756ad416,// 229 PAY 226 

    0xf33fa5fc,// 230 PAY 227 

    0x647f437d,// 231 PAY 228 

    0x6db0e3eb,// 232 PAY 229 

    0x0f01304d,// 233 PAY 230 

    0xa1852cbf,// 234 PAY 231 

    0xc9d6cf60,// 235 PAY 232 

    0x7f496a85,// 236 PAY 233 

    0xb4395ce0,// 237 PAY 234 

    0x936c5057,// 238 PAY 235 

    0xf3473fe0,// 239 PAY 236 

    0x19ceb2fd,// 240 PAY 237 

    0x89c23600,// 241 PAY 238 

    0xff0f105c,// 242 PAY 239 

    0xbc995889,// 243 PAY 240 

    0xf6714043,// 244 PAY 241 

    0x3c61c8c9,// 245 PAY 242 

    0x571530e6,// 246 PAY 243 

    0xc8a59859,// 247 PAY 244 

    0xa77d24f4,// 248 PAY 245 

    0x4c2700cd,// 249 PAY 246 

    0xfcdbc19c,// 250 PAY 247 

    0x11997ea2,// 251 PAY 248 

    0xaaf1a65f,// 252 PAY 249 

    0xc7e90057,// 253 PAY 250 

    0xa39e1fe6,// 254 PAY 251 

    0x29870d63,// 255 PAY 252 

    0xf335593e,// 256 PAY 253 

    0xff6dc319,// 257 PAY 254 

    0x84b4a7a1,// 258 PAY 255 

    0xae0ac7f9,// 259 PAY 256 

    0xc77b9eaf,// 260 PAY 257 

    0x1e7819e6,// 261 PAY 258 

    0xf5b17f2a,// 262 PAY 259 

    0x01a165bf,// 263 PAY 260 

    0xfc4823e4,// 264 PAY 261 

    0x23c42f63,// 265 PAY 262 

    0x7951d964,// 266 PAY 263 

    0xedfa52d5,// 267 PAY 264 

    0x34260000,// 268 PAY 265 

/// STA is 1 words. 

/// STA num_pkts       : 215 

/// STA pkt_idx        : 91 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc4 

    0x016cc4d7 // 269 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt56_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 237 words. 

/// BDA size     is 942 (0x3ae) 

/// BDA id       is 0x1b7b 

    0x03ae1b7b,// 3 BDA   1 

/// PAY Generic Data size   : 942 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xc795f10c,// 4 PAY   1 

    0x051a4197,// 5 PAY   2 

    0xdfea2f35,// 6 PAY   3 

    0x12c3a629,// 7 PAY   4 

    0x886a29ac,// 8 PAY   5 

    0x4124472c,// 9 PAY   6 

    0xb57582ab,// 10 PAY   7 

    0x8f6c6700,// 11 PAY   8 

    0x8d77351e,// 12 PAY   9 

    0xd91d2401,// 13 PAY  10 

    0x8caa96ad,// 14 PAY  11 

    0xa729da22,// 15 PAY  12 

    0x121223a3,// 16 PAY  13 

    0xf21f1a29,// 17 PAY  14 

    0x08943e93,// 18 PAY  15 

    0xffef8323,// 19 PAY  16 

    0x54d5c59a,// 20 PAY  17 

    0xab9863e8,// 21 PAY  18 

    0xf5346aaa,// 22 PAY  19 

    0xc5d7f2f9,// 23 PAY  20 

    0x14588d96,// 24 PAY  21 

    0xe7cd651c,// 25 PAY  22 

    0x62ab6d51,// 26 PAY  23 

    0x2444cce5,// 27 PAY  24 

    0x979c849d,// 28 PAY  25 

    0xa775b95d,// 29 PAY  26 

    0x55ebec01,// 30 PAY  27 

    0x79c964b7,// 31 PAY  28 

    0xc8748589,// 32 PAY  29 

    0x70a01c25,// 33 PAY  30 

    0xbee2341d,// 34 PAY  31 

    0x37819548,// 35 PAY  32 

    0xc4d8c738,// 36 PAY  33 

    0xc4f0270a,// 37 PAY  34 

    0x4077a65c,// 38 PAY  35 

    0x81c49666,// 39 PAY  36 

    0x6b177c6e,// 40 PAY  37 

    0x0b3639f8,// 41 PAY  38 

    0x37a4b143,// 42 PAY  39 

    0x9c5ad977,// 43 PAY  40 

    0x57c4318b,// 44 PAY  41 

    0x920b5aba,// 45 PAY  42 

    0xd8886258,// 46 PAY  43 

    0xdd9d7753,// 47 PAY  44 

    0xa323a810,// 48 PAY  45 

    0x00131494,// 49 PAY  46 

    0x3b07fcf9,// 50 PAY  47 

    0x4c16a3ba,// 51 PAY  48 

    0x9af9414b,// 52 PAY  49 

    0x01e8b89c,// 53 PAY  50 

    0x33809159,// 54 PAY  51 

    0x27c0765c,// 55 PAY  52 

    0xbb9235bb,// 56 PAY  53 

    0x8027230c,// 57 PAY  54 

    0xed202f82,// 58 PAY  55 

    0x9af49745,// 59 PAY  56 

    0x0a547460,// 60 PAY  57 

    0xef1607b4,// 61 PAY  58 

    0x08b3c281,// 62 PAY  59 

    0x2255244e,// 63 PAY  60 

    0x192c327d,// 64 PAY  61 

    0x3910f8a1,// 65 PAY  62 

    0x46d7fcc0,// 66 PAY  63 

    0xe5c4fb88,// 67 PAY  64 

    0x670b3600,// 68 PAY  65 

    0x5265df9f,// 69 PAY  66 

    0x9e915a4a,// 70 PAY  67 

    0xb589a55d,// 71 PAY  68 

    0x340d15a4,// 72 PAY  69 

    0x7640e7ca,// 73 PAY  70 

    0x6b1e282e,// 74 PAY  71 

    0xeb226398,// 75 PAY  72 

    0x4a88ea3d,// 76 PAY  73 

    0xee0b78f3,// 77 PAY  74 

    0x0a102052,// 78 PAY  75 

    0xfe35d828,// 79 PAY  76 

    0xfa142d8c,// 80 PAY  77 

    0x76293948,// 81 PAY  78 

    0x0446e56b,// 82 PAY  79 

    0x3cfbca72,// 83 PAY  80 

    0xf7d1d709,// 84 PAY  81 

    0x6e1013c1,// 85 PAY  82 

    0x055aec7a,// 86 PAY  83 

    0x285a836d,// 87 PAY  84 

    0xea3a772d,// 88 PAY  85 

    0x65eb5deb,// 89 PAY  86 

    0x013f08b8,// 90 PAY  87 

    0x59b1a2f4,// 91 PAY  88 

    0x84ee08eb,// 92 PAY  89 

    0xa95bf4cd,// 93 PAY  90 

    0x8fd2a538,// 94 PAY  91 

    0x2fa49aef,// 95 PAY  92 

    0x4aa7bb77,// 96 PAY  93 

    0x80584b41,// 97 PAY  94 

    0xc88c4446,// 98 PAY  95 

    0xf1808ac1,// 99 PAY  96 

    0x40227348,// 100 PAY  97 

    0x68418129,// 101 PAY  98 

    0x3da18e54,// 102 PAY  99 

    0xe2fdcbfd,// 103 PAY 100 

    0xfd77fb0c,// 104 PAY 101 

    0x97f16fce,// 105 PAY 102 

    0x704792fe,// 106 PAY 103 

    0xf02a9f52,// 107 PAY 104 

    0x7108cdf6,// 108 PAY 105 

    0xca38bd16,// 109 PAY 106 

    0xe3bd9b07,// 110 PAY 107 

    0xe0738a5d,// 111 PAY 108 

    0x7a0608f4,// 112 PAY 109 

    0x86e3cae6,// 113 PAY 110 

    0x9a198333,// 114 PAY 111 

    0x749ad119,// 115 PAY 112 

    0x567ddabe,// 116 PAY 113 

    0xd7b6b8e6,// 117 PAY 114 

    0x132c3b49,// 118 PAY 115 

    0xca34b149,// 119 PAY 116 

    0xf40893b4,// 120 PAY 117 

    0x53de108f,// 121 PAY 118 

    0x3bc77048,// 122 PAY 119 

    0xaecda5d8,// 123 PAY 120 

    0x4215625d,// 124 PAY 121 

    0x343c7991,// 125 PAY 122 

    0xfb8bc66e,// 126 PAY 123 

    0xf06cc44c,// 127 PAY 124 

    0x504caddf,// 128 PAY 125 

    0x4a3bb9c4,// 129 PAY 126 

    0x1734103e,// 130 PAY 127 

    0xc80718b6,// 131 PAY 128 

    0x4eb52127,// 132 PAY 129 

    0x1b37785e,// 133 PAY 130 

    0x812dd2e6,// 134 PAY 131 

    0xb654edfa,// 135 PAY 132 

    0x29601a49,// 136 PAY 133 

    0xa2503b97,// 137 PAY 134 

    0x2f4bf2f3,// 138 PAY 135 

    0x2d83e8b9,// 139 PAY 136 

    0x26f518f4,// 140 PAY 137 

    0x927c2598,// 141 PAY 138 

    0x50ba1a0c,// 142 PAY 139 

    0x66a2936e,// 143 PAY 140 

    0xe552e357,// 144 PAY 141 

    0xa67a1a9b,// 145 PAY 142 

    0xc1724348,// 146 PAY 143 

    0x00e9771c,// 147 PAY 144 

    0xebf04681,// 148 PAY 145 

    0xb236c1b3,// 149 PAY 146 

    0xf976d7d0,// 150 PAY 147 

    0x13f2251f,// 151 PAY 148 

    0x7a10df8a,// 152 PAY 149 

    0x5dac7f57,// 153 PAY 150 

    0x92df3950,// 154 PAY 151 

    0x0ed95ce0,// 155 PAY 152 

    0x1e961db0,// 156 PAY 153 

    0xf92b7c13,// 157 PAY 154 

    0xcfe951e5,// 158 PAY 155 

    0x1bad15c4,// 159 PAY 156 

    0xbb362fab,// 160 PAY 157 

    0x6058b178,// 161 PAY 158 

    0xed6f2b38,// 162 PAY 159 

    0x1cf12060,// 163 PAY 160 

    0x06565f19,// 164 PAY 161 

    0x299c2113,// 165 PAY 162 

    0x59bb85c0,// 166 PAY 163 

    0xa32d8899,// 167 PAY 164 

    0xc6d1620b,// 168 PAY 165 

    0x92cb1cbd,// 169 PAY 166 

    0x340a546a,// 170 PAY 167 

    0xdc7f329a,// 171 PAY 168 

    0x4865f5c3,// 172 PAY 169 

    0x45bbb475,// 173 PAY 170 

    0xecdbb7c6,// 174 PAY 171 

    0x6ea00951,// 175 PAY 172 

    0xed51abeb,// 176 PAY 173 

    0xe92ba2bf,// 177 PAY 174 

    0xf095d922,// 178 PAY 175 

    0x2a84a54c,// 179 PAY 176 

    0x20b4bbfd,// 180 PAY 177 

    0x8cfbb495,// 181 PAY 178 

    0xa9c85998,// 182 PAY 179 

    0x3d14abac,// 183 PAY 180 

    0x79570164,// 184 PAY 181 

    0xeebae1f1,// 185 PAY 182 

    0x4c1f5675,// 186 PAY 183 

    0x9c4f2f30,// 187 PAY 184 

    0x73fcdbea,// 188 PAY 185 

    0x45b9521a,// 189 PAY 186 

    0x34568bc9,// 190 PAY 187 

    0x7307170d,// 191 PAY 188 

    0x012b4187,// 192 PAY 189 

    0x799df8c4,// 193 PAY 190 

    0xf3746386,// 194 PAY 191 

    0x60e2390c,// 195 PAY 192 

    0xa0331e46,// 196 PAY 193 

    0x76e5c012,// 197 PAY 194 

    0x491c9ae5,// 198 PAY 195 

    0x11024312,// 199 PAY 196 

    0x4e372fab,// 200 PAY 197 

    0x29516361,// 201 PAY 198 

    0x47a69a0b,// 202 PAY 199 

    0xb2d8028d,// 203 PAY 200 

    0x9d3de030,// 204 PAY 201 

    0x04e44eff,// 205 PAY 202 

    0x9152dea5,// 206 PAY 203 

    0xf6a058c0,// 207 PAY 204 

    0x52db85f7,// 208 PAY 205 

    0x373091e7,// 209 PAY 206 

    0x0aee35e9,// 210 PAY 207 

    0xb6799ba3,// 211 PAY 208 

    0x75b24e31,// 212 PAY 209 

    0xf6ea66df,// 213 PAY 210 

    0x2bf568e2,// 214 PAY 211 

    0xd39e50b7,// 215 PAY 212 

    0xb0dbf8e9,// 216 PAY 213 

    0x07c10b44,// 217 PAY 214 

    0x5be5559f,// 218 PAY 215 

    0x0e14093c,// 219 PAY 216 

    0x5cb78e91,// 220 PAY 217 

    0x9e704065,// 221 PAY 218 

    0xa5e1664e,// 222 PAY 219 

    0xc02a18f6,// 223 PAY 220 

    0x7fed4f46,// 224 PAY 221 

    0x0d7c5cfb,// 225 PAY 222 

    0xbdd07cbe,// 226 PAY 223 

    0x76bc0149,// 227 PAY 224 

    0x9c4c8f69,// 228 PAY 225 

    0xf1846b01,// 229 PAY 226 

    0xa86564d3,// 230 PAY 227 

    0x3972a3a5,// 231 PAY 228 

    0x41ca9f83,// 232 PAY 229 

    0x16ed7811,// 233 PAY 230 

    0x5a31bc7e,// 234 PAY 231 

    0xb81b3e14,// 235 PAY 232 

    0x72f521e1,// 236 PAY 233 

    0x548d183a,// 237 PAY 234 

    0x2f60916a,// 238 PAY 235 

    0x25580000,// 239 PAY 236 

/// STA is 1 words. 

/// STA num_pkts       : 160 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf7 

    0x02d1f7a0 // 240 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt57_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 498 words. 

/// BDA size     is 1985 (0x7c1) 

/// BDA id       is 0xf427 

    0x07c1f427,// 3 BDA   1 

/// PAY Generic Data size   : 1985 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x73c6a354,// 4 PAY   1 

    0xc18225b9,// 5 PAY   2 

    0xdf2a927a,// 6 PAY   3 

    0xc74b093c,// 7 PAY   4 

    0x8a9530e1,// 8 PAY   5 

    0xa09a2c5b,// 9 PAY   6 

    0x487b41f8,// 10 PAY   7 

    0x6f40440d,// 11 PAY   8 

    0x9f5b1d6b,// 12 PAY   9 

    0xaa449443,// 13 PAY  10 

    0x395dd5a6,// 14 PAY  11 

    0xa0068b1d,// 15 PAY  12 

    0xd47d66be,// 16 PAY  13 

    0x5e5c26fc,// 17 PAY  14 

    0xd8828b31,// 18 PAY  15 

    0x7986bf9a,// 19 PAY  16 

    0x0c5b2422,// 20 PAY  17 

    0x6e197ea6,// 21 PAY  18 

    0x6bbeb78f,// 22 PAY  19 

    0x01ece4c5,// 23 PAY  20 

    0x97c7e0c6,// 24 PAY  21 

    0x7425aecb,// 25 PAY  22 

    0x40d1c701,// 26 PAY  23 

    0xa0aaad8a,// 27 PAY  24 

    0xa9cf5376,// 28 PAY  25 

    0x5bb3ead7,// 29 PAY  26 

    0xf775c862,// 30 PAY  27 

    0x064aaacb,// 31 PAY  28 

    0x161f29b7,// 32 PAY  29 

    0xbd96436e,// 33 PAY  30 

    0x1ec725ca,// 34 PAY  31 

    0xde8f372a,// 35 PAY  32 

    0x07d33d1b,// 36 PAY  33 

    0xd8430e36,// 37 PAY  34 

    0xca37d27b,// 38 PAY  35 

    0x6518762f,// 39 PAY  36 

    0x6b2ba5d2,// 40 PAY  37 

    0x26782a26,// 41 PAY  38 

    0x9adfe8fe,// 42 PAY  39 

    0x853adfaf,// 43 PAY  40 

    0x1f9acc47,// 44 PAY  41 

    0xddd394c5,// 45 PAY  42 

    0x62e4b258,// 46 PAY  43 

    0xdad4c6bc,// 47 PAY  44 

    0x47ef0bf2,// 48 PAY  45 

    0x133040ef,// 49 PAY  46 

    0xcb526a0c,// 50 PAY  47 

    0x1c9e4028,// 51 PAY  48 

    0xa955a211,// 52 PAY  49 

    0xb2342b4c,// 53 PAY  50 

    0xfbbd90b7,// 54 PAY  51 

    0xa51989a8,// 55 PAY  52 

    0x0d12765e,// 56 PAY  53 

    0x2b49300d,// 57 PAY  54 

    0x713c6634,// 58 PAY  55 

    0x39d1a0f4,// 59 PAY  56 

    0xe67dad86,// 60 PAY  57 

    0xc10f57bd,// 61 PAY  58 

    0xdc452694,// 62 PAY  59 

    0x6cf6eb23,// 63 PAY  60 

    0x201ed004,// 64 PAY  61 

    0x0ee2cc0e,// 65 PAY  62 

    0x81ef047d,// 66 PAY  63 

    0x57a53366,// 67 PAY  64 

    0x63a88cdc,// 68 PAY  65 

    0xc7feefee,// 69 PAY  66 

    0xfb18d79f,// 70 PAY  67 

    0x37413e04,// 71 PAY  68 

    0x67d3dcce,// 72 PAY  69 

    0xb00cf0bb,// 73 PAY  70 

    0x588efe28,// 74 PAY  71 

    0xb5a76829,// 75 PAY  72 

    0xefb81e5a,// 76 PAY  73 

    0xeb80f739,// 77 PAY  74 

    0xdddf6cab,// 78 PAY  75 

    0xdb76f13e,// 79 PAY  76 

    0xea9972f5,// 80 PAY  77 

    0x5e78059d,// 81 PAY  78 

    0xc7a6ef43,// 82 PAY  79 

    0x2c35b175,// 83 PAY  80 

    0xc67209e4,// 84 PAY  81 

    0x360a5f47,// 85 PAY  82 

    0xcc1b6116,// 86 PAY  83 

    0x66fac605,// 87 PAY  84 

    0x569415ba,// 88 PAY  85 

    0x1d21edee,// 89 PAY  86 

    0xfcc37d38,// 90 PAY  87 

    0x9f7d3919,// 91 PAY  88 

    0xe234f4a9,// 92 PAY  89 

    0x62f36a44,// 93 PAY  90 

    0x5ee7a039,// 94 PAY  91 

    0xa10da98e,// 95 PAY  92 

    0x23f4e7cf,// 96 PAY  93 

    0xe7e82455,// 97 PAY  94 

    0xc6e2f242,// 98 PAY  95 

    0x3e8ec17b,// 99 PAY  96 

    0xbf6e74b7,// 100 PAY  97 

    0xed6b8ce1,// 101 PAY  98 

    0xcbe51d27,// 102 PAY  99 

    0x6b958305,// 103 PAY 100 

    0x97543bea,// 104 PAY 101 

    0x12b5fdd8,// 105 PAY 102 

    0xc924537e,// 106 PAY 103 

    0x2a26c343,// 107 PAY 104 

    0x612f0d63,// 108 PAY 105 

    0x2e7c424e,// 109 PAY 106 

    0x9a2aad6c,// 110 PAY 107 

    0x552f4879,// 111 PAY 108 

    0x7e5193cf,// 112 PAY 109 

    0x7f2ba059,// 113 PAY 110 

    0x77e86f37,// 114 PAY 111 

    0x6e8876e0,// 115 PAY 112 

    0x9e46af98,// 116 PAY 113 

    0xc4fb5e03,// 117 PAY 114 

    0x54d110e1,// 118 PAY 115 

    0xc65b26e7,// 119 PAY 116 

    0x13a1c812,// 120 PAY 117 

    0x03f52ee4,// 121 PAY 118 

    0xb1ae2c4e,// 122 PAY 119 

    0x711008c5,// 123 PAY 120 

    0x0c828fb3,// 124 PAY 121 

    0x0aa4cddb,// 125 PAY 122 

    0x965325ca,// 126 PAY 123 

    0xa15d82ab,// 127 PAY 124 

    0x293cf36f,// 128 PAY 125 

    0x3ea02321,// 129 PAY 126 

    0x1fa23042,// 130 PAY 127 

    0xd4017b79,// 131 PAY 128 

    0x14947acc,// 132 PAY 129 

    0xd9a853ea,// 133 PAY 130 

    0x24bd2c63,// 134 PAY 131 

    0xbac64c5b,// 135 PAY 132 

    0x0e62a0df,// 136 PAY 133 

    0x6f4015c7,// 137 PAY 134 

    0xe5fc659c,// 138 PAY 135 

    0x5c6b2baf,// 139 PAY 136 

    0xa06f06d8,// 140 PAY 137 

    0x442a8ac7,// 141 PAY 138 

    0x7a331caf,// 142 PAY 139 

    0x788f29bf,// 143 PAY 140 

    0xd3b02ef0,// 144 PAY 141 

    0xa190eccd,// 145 PAY 142 

    0x1a8a1c8a,// 146 PAY 143 

    0xf9c1dd7b,// 147 PAY 144 

    0xcd572891,// 148 PAY 145 

    0x7edd2ad3,// 149 PAY 146 

    0x8c90d607,// 150 PAY 147 

    0xc0439932,// 151 PAY 148 

    0x6c50ddc7,// 152 PAY 149 

    0xefe64af2,// 153 PAY 150 

    0x322d677a,// 154 PAY 151 

    0x82ba589e,// 155 PAY 152 

    0xedefb73a,// 156 PAY 153 

    0xbb3b7bb9,// 157 PAY 154 

    0x5ffa8ab1,// 158 PAY 155 

    0xbd4b5bd2,// 159 PAY 156 

    0x5eefaf38,// 160 PAY 157 

    0x4d149d94,// 161 PAY 158 

    0x43dfb6a7,// 162 PAY 159 

    0xb11124dc,// 163 PAY 160 

    0xf8c9994a,// 164 PAY 161 

    0xd3fe4fdc,// 165 PAY 162 

    0xef7e39e7,// 166 PAY 163 

    0xb392f9a5,// 167 PAY 164 

    0x09cfd647,// 168 PAY 165 

    0xf1ff6796,// 169 PAY 166 

    0xbd2c0853,// 170 PAY 167 

    0xcdd406ac,// 171 PAY 168 

    0xf5e33965,// 172 PAY 169 

    0x050bc0cc,// 173 PAY 170 

    0xb065fc5a,// 174 PAY 171 

    0x6715dcb9,// 175 PAY 172 

    0xbd853a3a,// 176 PAY 173 

    0x355a6b73,// 177 PAY 174 

    0xaac9d4a2,// 178 PAY 175 

    0x0c86e351,// 179 PAY 176 

    0xc63c7689,// 180 PAY 177 

    0xacffcc2a,// 181 PAY 178 

    0xb1c20146,// 182 PAY 179 

    0xbf3dc3b3,// 183 PAY 180 

    0x438797a4,// 184 PAY 181 

    0x254edeed,// 185 PAY 182 

    0x97f4c20c,// 186 PAY 183 

    0x9f43a171,// 187 PAY 184 

    0x54d2912f,// 188 PAY 185 

    0xe102ac97,// 189 PAY 186 

    0xd1a22705,// 190 PAY 187 

    0x11cb6f63,// 191 PAY 188 

    0xa0d0ebc8,// 192 PAY 189 

    0xac04dd2e,// 193 PAY 190 

    0xe351c069,// 194 PAY 191 

    0xed2da9b9,// 195 PAY 192 

    0x803e068f,// 196 PAY 193 

    0xb93d7ff5,// 197 PAY 194 

    0x8a9f4d5c,// 198 PAY 195 

    0xa9110ca2,// 199 PAY 196 

    0x08cb634b,// 200 PAY 197 

    0xea88cf0d,// 201 PAY 198 

    0xfdc6d712,// 202 PAY 199 

    0xc82576d0,// 203 PAY 200 

    0xddaa9ab5,// 204 PAY 201 

    0x5f02dffe,// 205 PAY 202 

    0xbf99c28d,// 206 PAY 203 

    0x730ee436,// 207 PAY 204 

    0x979c65e1,// 208 PAY 205 

    0xc1126577,// 209 PAY 206 

    0x2785122c,// 210 PAY 207 

    0x6d8c5d79,// 211 PAY 208 

    0x3523733a,// 212 PAY 209 

    0x1b4dd617,// 213 PAY 210 

    0x6b84e86b,// 214 PAY 211 

    0x0335fad1,// 215 PAY 212 

    0x4dde3677,// 216 PAY 213 

    0x7a3f6277,// 217 PAY 214 

    0x49762f6d,// 218 PAY 215 

    0x3b34acbd,// 219 PAY 216 

    0x237d01d6,// 220 PAY 217 

    0xb76675e1,// 221 PAY 218 

    0xba9e0023,// 222 PAY 219 

    0x925064b2,// 223 PAY 220 

    0xdb7132ab,// 224 PAY 221 

    0x41647a50,// 225 PAY 222 

    0xd7aaaa4c,// 226 PAY 223 

    0x4289161e,// 227 PAY 224 

    0xfa2eda2e,// 228 PAY 225 

    0x8dbdf82f,// 229 PAY 226 

    0x215715bd,// 230 PAY 227 

    0xa06aa3b5,// 231 PAY 228 

    0x35c3752a,// 232 PAY 229 

    0x4ce14538,// 233 PAY 230 

    0xa5d8dd4e,// 234 PAY 231 

    0x1f7385c8,// 235 PAY 232 

    0x2fd0faa8,// 236 PAY 233 

    0x65d045dd,// 237 PAY 234 

    0xf5a65f3c,// 238 PAY 235 

    0xbe262d47,// 239 PAY 236 

    0xd674e6b1,// 240 PAY 237 

    0x6b3d8be3,// 241 PAY 238 

    0x3066788f,// 242 PAY 239 

    0x4636a808,// 243 PAY 240 

    0x51b840c7,// 244 PAY 241 

    0xd263dc2d,// 245 PAY 242 

    0xb7297652,// 246 PAY 243 

    0x69f198b6,// 247 PAY 244 

    0xe39aeccd,// 248 PAY 245 

    0xd1de5176,// 249 PAY 246 

    0x0d362221,// 250 PAY 247 

    0x0e8adf8c,// 251 PAY 248 

    0x24a92f28,// 252 PAY 249 

    0xd4fa1b6c,// 253 PAY 250 

    0xb558c848,// 254 PAY 251 

    0xf11cb0fb,// 255 PAY 252 

    0xef19715a,// 256 PAY 253 

    0xd0df19ba,// 257 PAY 254 

    0x9e99435b,// 258 PAY 255 

    0xa109c406,// 259 PAY 256 

    0xe7bf8d60,// 260 PAY 257 

    0x6ceafb30,// 261 PAY 258 

    0x74dac0e1,// 262 PAY 259 

    0xaaf2668a,// 263 PAY 260 

    0x9cd25f42,// 264 PAY 261 

    0x98124d7a,// 265 PAY 262 

    0x6f1e1c98,// 266 PAY 263 

    0xf45a2cce,// 267 PAY 264 

    0x5fd3b319,// 268 PAY 265 

    0xf5b8c5a1,// 269 PAY 266 

    0xaeff5096,// 270 PAY 267 

    0x90b9de17,// 271 PAY 268 

    0x99833920,// 272 PAY 269 

    0x680ba894,// 273 PAY 270 

    0x5cce47aa,// 274 PAY 271 

    0x8fb93669,// 275 PAY 272 

    0x1774fc93,// 276 PAY 273 

    0xd479a4db,// 277 PAY 274 

    0xc0774e39,// 278 PAY 275 

    0x5a017d87,// 279 PAY 276 

    0x34bed756,// 280 PAY 277 

    0x441aaa54,// 281 PAY 278 

    0xac1e24cb,// 282 PAY 279 

    0x5e4df1c6,// 283 PAY 280 

    0x8c10a119,// 284 PAY 281 

    0x8d72f006,// 285 PAY 282 

    0xa015aa3d,// 286 PAY 283 

    0xd3d0bd59,// 287 PAY 284 

    0x79f46d30,// 288 PAY 285 

    0x416269e2,// 289 PAY 286 

    0x83e094f5,// 290 PAY 287 

    0x7c48277a,// 291 PAY 288 

    0x8a19ee55,// 292 PAY 289 

    0x81538fcb,// 293 PAY 290 

    0xb7d28809,// 294 PAY 291 

    0x9a366d47,// 295 PAY 292 

    0x157acb15,// 296 PAY 293 

    0xcd715ae9,// 297 PAY 294 

    0x3a45ea01,// 298 PAY 295 

    0x2dfdbf16,// 299 PAY 296 

    0x031d0e3b,// 300 PAY 297 

    0x1bbfeb38,// 301 PAY 298 

    0xb04f39ec,// 302 PAY 299 

    0xbddb33b1,// 303 PAY 300 

    0xefc7c78d,// 304 PAY 301 

    0xff1b5ab9,// 305 PAY 302 

    0xb5ec7dcc,// 306 PAY 303 

    0x89ba7456,// 307 PAY 304 

    0xc9102d5b,// 308 PAY 305 

    0x279b41af,// 309 PAY 306 

    0xf1202e5e,// 310 PAY 307 

    0x9ba64600,// 311 PAY 308 

    0x426c41da,// 312 PAY 309 

    0xf8282815,// 313 PAY 310 

    0x1a14dd4f,// 314 PAY 311 

    0x88dd8dd3,// 315 PAY 312 

    0x927e9242,// 316 PAY 313 

    0x1c3f038c,// 317 PAY 314 

    0x566b5d1a,// 318 PAY 315 

    0x1fb76ddc,// 319 PAY 316 

    0x40453846,// 320 PAY 317 

    0x12627ab9,// 321 PAY 318 

    0xf94fb400,// 322 PAY 319 

    0x3ab51e35,// 323 PAY 320 

    0x803e2bd1,// 324 PAY 321 

    0x24e0503e,// 325 PAY 322 

    0xb66740c4,// 326 PAY 323 

    0x54d3698f,// 327 PAY 324 

    0xc5fbf6ef,// 328 PAY 325 

    0x5726b4b3,// 329 PAY 326 

    0x1e5b9055,// 330 PAY 327 

    0x96ef9831,// 331 PAY 328 

    0x9123fad8,// 332 PAY 329 

    0xab2e1058,// 333 PAY 330 

    0x98b7eda1,// 334 PAY 331 

    0x03f88184,// 335 PAY 332 

    0x953b5b30,// 336 PAY 333 

    0x5945517e,// 337 PAY 334 

    0x6c46afe2,// 338 PAY 335 

    0x9b3be127,// 339 PAY 336 

    0x20f78d0c,// 340 PAY 337 

    0x7a15a396,// 341 PAY 338 

    0xc7af928a,// 342 PAY 339 

    0x02e309f2,// 343 PAY 340 

    0x15a8c7bc,// 344 PAY 341 

    0xf2164f78,// 345 PAY 342 

    0x11aa1783,// 346 PAY 343 

    0x44677b20,// 347 PAY 344 

    0x7636922a,// 348 PAY 345 

    0xd5ab974d,// 349 PAY 346 

    0x7ab61ae0,// 350 PAY 347 

    0x739ecf18,// 351 PAY 348 

    0x6b5d3539,// 352 PAY 349 

    0xa9daa466,// 353 PAY 350 

    0x8b9ee041,// 354 PAY 351 

    0x61426517,// 355 PAY 352 

    0x3df684af,// 356 PAY 353 

    0x780208e5,// 357 PAY 354 

    0x4d8cfc68,// 358 PAY 355 

    0x09772031,// 359 PAY 356 

    0xb82bc750,// 360 PAY 357 

    0xd0c21f60,// 361 PAY 358 

    0x3cab2429,// 362 PAY 359 

    0xaa19a7fa,// 363 PAY 360 

    0x2134d2a9,// 364 PAY 361 

    0xa8120cbd,// 365 PAY 362 

    0x0cdd1c50,// 366 PAY 363 

    0x70387b03,// 367 PAY 364 

    0x628bf0c2,// 368 PAY 365 

    0x2c99bda6,// 369 PAY 366 

    0x093f665e,// 370 PAY 367 

    0xb2d9c58f,// 371 PAY 368 

    0x0d33a045,// 372 PAY 369 

    0xca1a99d6,// 373 PAY 370 

    0xe620d188,// 374 PAY 371 

    0x47cef0d3,// 375 PAY 372 

    0x8c14f6e1,// 376 PAY 373 

    0xa30dd418,// 377 PAY 374 

    0x5e2a18f4,// 378 PAY 375 

    0x28e12107,// 379 PAY 376 

    0x043011f8,// 380 PAY 377 

    0x5afd5eba,// 381 PAY 378 

    0xc0fcc7ba,// 382 PAY 379 

    0x878d02ce,// 383 PAY 380 

    0x0328213f,// 384 PAY 381 

    0x4346122b,// 385 PAY 382 

    0x5570e601,// 386 PAY 383 

    0x0a577b34,// 387 PAY 384 

    0x6e2537e1,// 388 PAY 385 

    0x52194740,// 389 PAY 386 

    0x45d012a7,// 390 PAY 387 

    0x9319c6c6,// 391 PAY 388 

    0xe811cc06,// 392 PAY 389 

    0x93529278,// 393 PAY 390 

    0x1627658c,// 394 PAY 391 

    0x7fd10c54,// 395 PAY 392 

    0xf60cb8d6,// 396 PAY 393 

    0x6247f5d1,// 397 PAY 394 

    0x0f9f42bb,// 398 PAY 395 

    0xf40f1c70,// 399 PAY 396 

    0xb5bd47cd,// 400 PAY 397 

    0x96aaaecf,// 401 PAY 398 

    0xb0e235b1,// 402 PAY 399 

    0xb6f8f727,// 403 PAY 400 

    0x29334d44,// 404 PAY 401 

    0x7ff4ba47,// 405 PAY 402 

    0x629c2234,// 406 PAY 403 

    0x90c0f6b0,// 407 PAY 404 

    0x57684e9f,// 408 PAY 405 

    0xa973df09,// 409 PAY 406 

    0x5662c40a,// 410 PAY 407 

    0x03320d45,// 411 PAY 408 

    0x3e0f5ce6,// 412 PAY 409 

    0x659cf530,// 413 PAY 410 

    0x700ab6a0,// 414 PAY 411 

    0xd26e86d0,// 415 PAY 412 

    0xeb64399b,// 416 PAY 413 

    0xc998f821,// 417 PAY 414 

    0x5d57134e,// 418 PAY 415 

    0x80992bbe,// 419 PAY 416 

    0x33c46712,// 420 PAY 417 

    0x2f173a23,// 421 PAY 418 

    0x82d77a0c,// 422 PAY 419 

    0x233c19f5,// 423 PAY 420 

    0xa87d0fe6,// 424 PAY 421 

    0xfcc54ace,// 425 PAY 422 

    0xd94d6ec3,// 426 PAY 423 

    0xc41e1d1f,// 427 PAY 424 

    0xbdf136f7,// 428 PAY 425 

    0xb1296949,// 429 PAY 426 

    0x9e3dfcea,// 430 PAY 427 

    0x2ba04f94,// 431 PAY 428 

    0xf478da38,// 432 PAY 429 

    0x53e6610c,// 433 PAY 430 

    0x2c074e98,// 434 PAY 431 

    0xc9ca6a38,// 435 PAY 432 

    0x4242dc99,// 436 PAY 433 

    0xfa057b86,// 437 PAY 434 

    0xcdae9c84,// 438 PAY 435 

    0xef761ed5,// 439 PAY 436 

    0xf5ec6cbd,// 440 PAY 437 

    0x361d057f,// 441 PAY 438 

    0x0308ac5a,// 442 PAY 439 

    0xee4be08c,// 443 PAY 440 

    0x2528d5f8,// 444 PAY 441 

    0x77d3327d,// 445 PAY 442 

    0x47768bdd,// 446 PAY 443 

    0xaa1507ea,// 447 PAY 444 

    0x368309ce,// 448 PAY 445 

    0xf9d38cc3,// 449 PAY 446 

    0xff7efb12,// 450 PAY 447 

    0xf09ab18b,// 451 PAY 448 

    0x3746ccd7,// 452 PAY 449 

    0x4698fbbe,// 453 PAY 450 

    0xc903621e,// 454 PAY 451 

    0xceb30c4b,// 455 PAY 452 

    0x8803c6d6,// 456 PAY 453 

    0xb38df4cc,// 457 PAY 454 

    0x49051138,// 458 PAY 455 

    0xfc88131e,// 459 PAY 456 

    0x7e046ee9,// 460 PAY 457 

    0x48b6dc94,// 461 PAY 458 

    0x13e89f73,// 462 PAY 459 

    0xca904f35,// 463 PAY 460 

    0x3f631956,// 464 PAY 461 

    0x7da961c8,// 465 PAY 462 

    0x56b2a197,// 466 PAY 463 

    0x9a1b5830,// 467 PAY 464 

    0x4d47f7c1,// 468 PAY 465 

    0x7ff4d4e6,// 469 PAY 466 

    0x2b404390,// 470 PAY 467 

    0x024511ea,// 471 PAY 468 

    0x62332a55,// 472 PAY 469 

    0xfb6b0210,// 473 PAY 470 

    0xde29090d,// 474 PAY 471 

    0xe7ab7476,// 475 PAY 472 

    0x26a6e1f1,// 476 PAY 473 

    0xe82a0ea8,// 477 PAY 474 

    0x40e22c4c,// 478 PAY 475 

    0x1351ed1f,// 479 PAY 476 

    0x1a04009b,// 480 PAY 477 

    0x3635d1f2,// 481 PAY 478 

    0x77509a00,// 482 PAY 479 

    0xd5d902e2,// 483 PAY 480 

    0x422e3fe1,// 484 PAY 481 

    0x50d163f2,// 485 PAY 482 

    0xedd7fe54,// 486 PAY 483 

    0xafe86b90,// 487 PAY 484 

    0x1027fe63,// 488 PAY 485 

    0x7c228a28,// 489 PAY 486 

    0x47d78a19,// 490 PAY 487 

    0xa759c998,// 491 PAY 488 

    0x0b03b796,// 492 PAY 489 

    0x4de64d76,// 493 PAY 490 

    0x6fb150b9,// 494 PAY 491 

    0x9e614538,// 495 PAY 492 

    0x74a60765,// 496 PAY 493 

    0x6fb13289,// 497 PAY 494 

    0x8b42e848,// 498 PAY 495 

    0x060dbbfa,// 499 PAY 496 

    0xec000000,// 500 PAY 497 

/// HASH is  12 bytes 

    0x9ba64600,// 501 HSH   1 

    0x426c41da,// 502 HSH   2 

    0xf8282815,// 503 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 231 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9 

    0x03dc09e7 // 504 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt58_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 402 words. 

/// BDA size     is 1601 (0x641) 

/// BDA id       is 0x5042 

    0x06415042,// 3 BDA   1 

/// PAY Generic Data size   : 1601 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xf1fa5d86,// 4 PAY   1 

    0x399c99e3,// 5 PAY   2 

    0xd86aaa81,// 6 PAY   3 

    0xcf7229c9,// 7 PAY   4 

    0x9dec40b1,// 8 PAY   5 

    0x4ec8267c,// 9 PAY   6 

    0x125e4b90,// 10 PAY   7 

    0x645a8869,// 11 PAY   8 

    0xd14eb79d,// 12 PAY   9 

    0x306a99b9,// 13 PAY  10 

    0x1f7aee20,// 14 PAY  11 

    0x55bb6bd1,// 15 PAY  12 

    0x2721bdac,// 16 PAY  13 

    0xc211027a,// 17 PAY  14 

    0x5409a660,// 18 PAY  15 

    0xb5403db1,// 19 PAY  16 

    0x7fd63903,// 20 PAY  17 

    0x2da14510,// 21 PAY  18 

    0x0c5f5e85,// 22 PAY  19 

    0x9a84413f,// 23 PAY  20 

    0x2be19c84,// 24 PAY  21 

    0x32cbc8d8,// 25 PAY  22 

    0xc2cac576,// 26 PAY  23 

    0x88ce2836,// 27 PAY  24 

    0xfacf8d96,// 28 PAY  25 

    0x41db8d35,// 29 PAY  26 

    0x76dbceee,// 30 PAY  27 

    0xae3c4f4c,// 31 PAY  28 

    0xf8bca1b6,// 32 PAY  29 

    0x440400d3,// 33 PAY  30 

    0x0f139480,// 34 PAY  31 

    0x416fff05,// 35 PAY  32 

    0x9544918f,// 36 PAY  33 

    0xcf4ad08b,// 37 PAY  34 

    0xdce65f82,// 38 PAY  35 

    0xd6a16c8d,// 39 PAY  36 

    0x1d70b17f,// 40 PAY  37 

    0xf2ca7161,// 41 PAY  38 

    0xed566f61,// 42 PAY  39 

    0xea316f3d,// 43 PAY  40 

    0xc9412477,// 44 PAY  41 

    0xa9fb7fa9,// 45 PAY  42 

    0x1e251d49,// 46 PAY  43 

    0x0e085400,// 47 PAY  44 

    0xaac94bd1,// 48 PAY  45 

    0x934c7670,// 49 PAY  46 

    0xdc64ac6e,// 50 PAY  47 

    0xc49e3d0c,// 51 PAY  48 

    0xbf2b7b7a,// 52 PAY  49 

    0x6506af88,// 53 PAY  50 

    0x74bc241c,// 54 PAY  51 

    0x9babf201,// 55 PAY  52 

    0x7f34dcc7,// 56 PAY  53 

    0x0ac28d5e,// 57 PAY  54 

    0xae686405,// 58 PAY  55 

    0xee5a657f,// 59 PAY  56 

    0xf53cf28b,// 60 PAY  57 

    0x30818345,// 61 PAY  58 

    0x122251ca,// 62 PAY  59 

    0x99695687,// 63 PAY  60 

    0xbbb6c5d4,// 64 PAY  61 

    0x75016613,// 65 PAY  62 

    0x4d4caabe,// 66 PAY  63 

    0x55861def,// 67 PAY  64 

    0x67b0131d,// 68 PAY  65 

    0x74fd5b7f,// 69 PAY  66 

    0x769afcee,// 70 PAY  67 

    0x9b6be906,// 71 PAY  68 

    0xed95a007,// 72 PAY  69 

    0x9c8aec83,// 73 PAY  70 

    0x1db9c045,// 74 PAY  71 

    0xb2ac0f88,// 75 PAY  72 

    0x08b07060,// 76 PAY  73 

    0xba8921ed,// 77 PAY  74 

    0xbaa98d40,// 78 PAY  75 

    0x45852b3c,// 79 PAY  76 

    0xb458339e,// 80 PAY  77 

    0x7402a89b,// 81 PAY  78 

    0x0fc8b7b9,// 82 PAY  79 

    0x9563cf22,// 83 PAY  80 

    0x6f9515aa,// 84 PAY  81 

    0x6347ab22,// 85 PAY  82 

    0x48597a81,// 86 PAY  83 

    0x1afe32fa,// 87 PAY  84 

    0xc936d896,// 88 PAY  85 

    0xeff0e7eb,// 89 PAY  86 

    0x72f01129,// 90 PAY  87 

    0x9871c12e,// 91 PAY  88 

    0x2ba509b1,// 92 PAY  89 

    0x802a2e8c,// 93 PAY  90 

    0xf5242930,// 94 PAY  91 

    0xe90fe0fb,// 95 PAY  92 

    0x1859c032,// 96 PAY  93 

    0xd43689a8,// 97 PAY  94 

    0xdfa8e53b,// 98 PAY  95 

    0x800c125b,// 99 PAY  96 

    0xe5667342,// 100 PAY  97 

    0xb799418a,// 101 PAY  98 

    0xb9957a6b,// 102 PAY  99 

    0x9a72b624,// 103 PAY 100 

    0x6f666bea,// 104 PAY 101 

    0xba65e8e2,// 105 PAY 102 

    0xc18acdba,// 106 PAY 103 

    0xf782a9ec,// 107 PAY 104 

    0x375954c8,// 108 PAY 105 

    0x84bfd531,// 109 PAY 106 

    0xf18ef6e9,// 110 PAY 107 

    0xda409491,// 111 PAY 108 

    0x7ae37729,// 112 PAY 109 

    0x66bc938c,// 113 PAY 110 

    0x5b6093ed,// 114 PAY 111 

    0xc33e7bf8,// 115 PAY 112 

    0x96ade1ba,// 116 PAY 113 

    0xa0f80b54,// 117 PAY 114 

    0x432d722f,// 118 PAY 115 

    0x856150af,// 119 PAY 116 

    0x691bae3d,// 120 PAY 117 

    0x5c8a3027,// 121 PAY 118 

    0x1a2160c7,// 122 PAY 119 

    0x79629c4a,// 123 PAY 120 

    0x2630b4ee,// 124 PAY 121 

    0x59ea5802,// 125 PAY 122 

    0x2777741f,// 126 PAY 123 

    0x6d40c1eb,// 127 PAY 124 

    0xe74e2e8d,// 128 PAY 125 

    0xa26d3c52,// 129 PAY 126 

    0x134ffca1,// 130 PAY 127 

    0x53f28cfe,// 131 PAY 128 

    0x2da4883f,// 132 PAY 129 

    0x3b04f30d,// 133 PAY 130 

    0xe9795da3,// 134 PAY 131 

    0xe74ece40,// 135 PAY 132 

    0xf2f84320,// 136 PAY 133 

    0xb094f228,// 137 PAY 134 

    0xea0d2f17,// 138 PAY 135 

    0x7a022012,// 139 PAY 136 

    0x1bbb5a5e,// 140 PAY 137 

    0x6d4220ab,// 141 PAY 138 

    0x77def4f1,// 142 PAY 139 

    0x1cb07d47,// 143 PAY 140 

    0xc696b580,// 144 PAY 141 

    0xfc430b08,// 145 PAY 142 

    0x62e3ca6f,// 146 PAY 143 

    0xa2097045,// 147 PAY 144 

    0xfeb2a760,// 148 PAY 145 

    0xfbb756be,// 149 PAY 146 

    0xfae33897,// 150 PAY 147 

    0x0faab7c9,// 151 PAY 148 

    0x57162137,// 152 PAY 149 

    0x0d153e80,// 153 PAY 150 

    0x90800f38,// 154 PAY 151 

    0x390d96b3,// 155 PAY 152 

    0x3504a721,// 156 PAY 153 

    0x16a314db,// 157 PAY 154 

    0x72ee30c1,// 158 PAY 155 

    0x1efa258e,// 159 PAY 156 

    0x83f2866d,// 160 PAY 157 

    0xfa87565c,// 161 PAY 158 

    0xc487c1c4,// 162 PAY 159 

    0x8660ad5a,// 163 PAY 160 

    0x4e7142bc,// 164 PAY 161 

    0x8cce6553,// 165 PAY 162 

    0x5fe5c2d5,// 166 PAY 163 

    0xb9fcf362,// 167 PAY 164 

    0x216b21ac,// 168 PAY 165 

    0xb37b5dca,// 169 PAY 166 

    0xf4e5899e,// 170 PAY 167 

    0xe5df0611,// 171 PAY 168 

    0xa572a5b0,// 172 PAY 169 

    0x34dae4bf,// 173 PAY 170 

    0xf858201f,// 174 PAY 171 

    0xb68a6c6c,// 175 PAY 172 

    0x71969a27,// 176 PAY 173 

    0xa788b181,// 177 PAY 174 

    0x56113af0,// 178 PAY 175 

    0x7cf10abd,// 179 PAY 176 

    0x43e7a022,// 180 PAY 177 

    0x69f17c4b,// 181 PAY 178 

    0x6db384b3,// 182 PAY 179 

    0x82d10f89,// 183 PAY 180 

    0xfd8be6f6,// 184 PAY 181 

    0x2de789d6,// 185 PAY 182 

    0x4f36b83d,// 186 PAY 183 

    0xf089fa2e,// 187 PAY 184 

    0x65d8de8d,// 188 PAY 185 

    0xbc9b7456,// 189 PAY 186 

    0xff30a335,// 190 PAY 187 

    0xe4298cbe,// 191 PAY 188 

    0x2b20e12d,// 192 PAY 189 

    0xf77c5f87,// 193 PAY 190 

    0xf3710784,// 194 PAY 191 

    0x500ff1e7,// 195 PAY 192 

    0x9cb00780,// 196 PAY 193 

    0xe8d90349,// 197 PAY 194 

    0x38233271,// 198 PAY 195 

    0xa69b83b7,// 199 PAY 196 

    0x2739cf11,// 200 PAY 197 

    0xc9a761a2,// 201 PAY 198 

    0xb80a4b43,// 202 PAY 199 

    0x9d4e32f9,// 203 PAY 200 

    0xe4f2e60c,// 204 PAY 201 

    0x3e278796,// 205 PAY 202 

    0xe7fae5d0,// 206 PAY 203 

    0xed97913e,// 207 PAY 204 

    0xe731d9cc,// 208 PAY 205 

    0xd4018388,// 209 PAY 206 

    0xc9f2bfd7,// 210 PAY 207 

    0x4f57b51e,// 211 PAY 208 

    0xfb1b9118,// 212 PAY 209 

    0xf822ec66,// 213 PAY 210 

    0x220db768,// 214 PAY 211 

    0x9a5bb51a,// 215 PAY 212 

    0x21f38cdf,// 216 PAY 213 

    0xcfb79292,// 217 PAY 214 

    0x1f0f59ab,// 218 PAY 215 

    0x18f4af23,// 219 PAY 216 

    0xa8dc4db3,// 220 PAY 217 

    0x3a1fdd22,// 221 PAY 218 

    0xb024c81b,// 222 PAY 219 

    0x750a171e,// 223 PAY 220 

    0x10fe5789,// 224 PAY 221 

    0xad824b91,// 225 PAY 222 

    0x61191ecc,// 226 PAY 223 

    0x4537f07a,// 227 PAY 224 

    0x36d2698d,// 228 PAY 225 

    0xfe55d967,// 229 PAY 226 

    0x9241ec95,// 230 PAY 227 

    0x771c2557,// 231 PAY 228 

    0xf629d164,// 232 PAY 229 

    0x3b475265,// 233 PAY 230 

    0x2769f0df,// 234 PAY 231 

    0xf5c98551,// 235 PAY 232 

    0x73c9f553,// 236 PAY 233 

    0x43dee9e4,// 237 PAY 234 

    0xf042e1e7,// 238 PAY 235 

    0xe95fc30d,// 239 PAY 236 

    0x7ddc6470,// 240 PAY 237 

    0xd3255d75,// 241 PAY 238 

    0x348f669c,// 242 PAY 239 

    0x96ac7849,// 243 PAY 240 

    0xe45026a5,// 244 PAY 241 

    0xf65bd72f,// 245 PAY 242 

    0x2e67a442,// 246 PAY 243 

    0xa8e45bcb,// 247 PAY 244 

    0x45c35880,// 248 PAY 245 

    0x3fbecd29,// 249 PAY 246 

    0x184ec008,// 250 PAY 247 

    0xa7c3a613,// 251 PAY 248 

    0x94c07027,// 252 PAY 249 

    0xd65ff9ba,// 253 PAY 250 

    0xec5444b7,// 254 PAY 251 

    0xb8c44b14,// 255 PAY 252 

    0x4deeeec9,// 256 PAY 253 

    0xabad1add,// 257 PAY 254 

    0xc80c0855,// 258 PAY 255 

    0x8e482a4c,// 259 PAY 256 

    0x95b9d499,// 260 PAY 257 

    0x55fee221,// 261 PAY 258 

    0x1b96f7e7,// 262 PAY 259 

    0x16a30c40,// 263 PAY 260 

    0x6dd45c71,// 264 PAY 261 

    0xfecadb8f,// 265 PAY 262 

    0x096a6aec,// 266 PAY 263 

    0xf37519be,// 267 PAY 264 

    0xde35db1f,// 268 PAY 265 

    0x2db32bea,// 269 PAY 266 

    0x1ce15533,// 270 PAY 267 

    0x6d50f188,// 271 PAY 268 

    0xfeeb5089,// 272 PAY 269 

    0xd1c1f63d,// 273 PAY 270 

    0x6c351a98,// 274 PAY 271 

    0xa999d314,// 275 PAY 272 

    0x61c6bb75,// 276 PAY 273 

    0x82cedaea,// 277 PAY 274 

    0x63cdb7ad,// 278 PAY 275 

    0x95f36341,// 279 PAY 276 

    0xaaf41ec9,// 280 PAY 277 

    0xbfd8dfc8,// 281 PAY 278 

    0x8bc6e8ce,// 282 PAY 279 

    0xe7dfba76,// 283 PAY 280 

    0x9d6cbdea,// 284 PAY 281 

    0x5c5b5b72,// 285 PAY 282 

    0xfec43f51,// 286 PAY 283 

    0x8a8858da,// 287 PAY 284 

    0x4331cafc,// 288 PAY 285 

    0x03ae64da,// 289 PAY 286 

    0xa7a5e9d1,// 290 PAY 287 

    0xd9ba699f,// 291 PAY 288 

    0x4f305df8,// 292 PAY 289 

    0xf6927f1e,// 293 PAY 290 

    0x99f2a38e,// 294 PAY 291 

    0xca576bfa,// 295 PAY 292 

    0xaa1d81ca,// 296 PAY 293 

    0x550323fd,// 297 PAY 294 

    0xfb31f0eb,// 298 PAY 295 

    0x5441f760,// 299 PAY 296 

    0xae6fa731,// 300 PAY 297 

    0xb28529bb,// 301 PAY 298 

    0x1e471600,// 302 PAY 299 

    0x23828f48,// 303 PAY 300 

    0xde3058d3,// 304 PAY 301 

    0x9c0a19c9,// 305 PAY 302 

    0x9e1545bf,// 306 PAY 303 

    0xccc9cbb7,// 307 PAY 304 

    0x3df53eea,// 308 PAY 305 

    0xae28dde8,// 309 PAY 306 

    0xccf430e1,// 310 PAY 307 

    0xf2c6db1d,// 311 PAY 308 

    0x86024426,// 312 PAY 309 

    0xb9050fb2,// 313 PAY 310 

    0x97555942,// 314 PAY 311 

    0xdf5e3840,// 315 PAY 312 

    0xec918333,// 316 PAY 313 

    0x6e14ef95,// 317 PAY 314 

    0xb08b1b50,// 318 PAY 315 

    0xda2ca7f9,// 319 PAY 316 

    0x205e13fb,// 320 PAY 317 

    0xab703ddc,// 321 PAY 318 

    0x6e861fb1,// 322 PAY 319 

    0x4e4bd062,// 323 PAY 320 

    0x6c7b9e2d,// 324 PAY 321 

    0xdae152bb,// 325 PAY 322 

    0xcec0d264,// 326 PAY 323 

    0x13c8877f,// 327 PAY 324 

    0x4711a1c3,// 328 PAY 325 

    0x0835f6b4,// 329 PAY 326 

    0xbc3c1dd9,// 330 PAY 327 

    0x5e00a281,// 331 PAY 328 

    0xebf86d06,// 332 PAY 329 

    0x9e15c021,// 333 PAY 330 

    0x7c42f34c,// 334 PAY 331 

    0x5bfe480b,// 335 PAY 332 

    0xefba4b3a,// 336 PAY 333 

    0xf3e47359,// 337 PAY 334 

    0x4308e290,// 338 PAY 335 

    0x0473b3e9,// 339 PAY 336 

    0x1b5bea42,// 340 PAY 337 

    0x347c52b4,// 341 PAY 338 

    0xc4583596,// 342 PAY 339 

    0x6abc2c5c,// 343 PAY 340 

    0x08f10543,// 344 PAY 341 

    0x06f154e9,// 345 PAY 342 

    0xdf75c36d,// 346 PAY 343 

    0xa1d4d984,// 347 PAY 344 

    0x58b25f32,// 348 PAY 345 

    0xf9cd300a,// 349 PAY 346 

    0x82e58222,// 350 PAY 347 

    0xe7c8f8fd,// 351 PAY 348 

    0x388eef51,// 352 PAY 349 

    0xf39408de,// 353 PAY 350 

    0xc8021c9a,// 354 PAY 351 

    0x299e9611,// 355 PAY 352 

    0x5b430e1f,// 356 PAY 353 

    0xe20ad1df,// 357 PAY 354 

    0xbdb0bd6e,// 358 PAY 355 

    0xb44224dc,// 359 PAY 356 

    0x8ccfbd91,// 360 PAY 357 

    0x85ca277e,// 361 PAY 358 

    0x7c5ba8c1,// 362 PAY 359 

    0x25f71be7,// 363 PAY 360 

    0x16d0c6df,// 364 PAY 361 

    0x6388e255,// 365 PAY 362 

    0x342274d5,// 366 PAY 363 

    0xb92cd530,// 367 PAY 364 

    0x0ec9b1a2,// 368 PAY 365 

    0x56117150,// 369 PAY 366 

    0xbc0f6043,// 370 PAY 367 

    0xb61af6e5,// 371 PAY 368 

    0x1b312e77,// 372 PAY 369 

    0x98c0fae3,// 373 PAY 370 

    0xc472f07f,// 374 PAY 371 

    0x657277c8,// 375 PAY 372 

    0x747411d0,// 376 PAY 373 

    0x032a878c,// 377 PAY 374 

    0x7fca338d,// 378 PAY 375 

    0xf1976b4f,// 379 PAY 376 

    0x7974c3be,// 380 PAY 377 

    0xf1476f65,// 381 PAY 378 

    0xb822672b,// 382 PAY 379 

    0xba30d41b,// 383 PAY 380 

    0x6c0f5e67,// 384 PAY 381 

    0xdf59a85e,// 385 PAY 382 

    0x4c3bcf1e,// 386 PAY 383 

    0x8c8cd596,// 387 PAY 384 

    0x3f22c842,// 388 PAY 385 

    0x50665def,// 389 PAY 386 

    0x0a1f3255,// 390 PAY 387 

    0xc48ca46c,// 391 PAY 388 

    0xe3178f78,// 392 PAY 389 

    0x30bd9d28,// 393 PAY 390 

    0xf1e6eb7f,// 394 PAY 391 

    0x7f6c1d30,// 395 PAY 392 

    0xdfcd766f,// 396 PAY 393 

    0x75bdb6ae,// 397 PAY 394 

    0xd156f4e8,// 398 PAY 395 

    0x6871e837,// 399 PAY 396 

    0xeea4d7a0,// 400 PAY 397 

    0x1bb9aea5,// 401 PAY 398 

    0x200f7cbf,// 402 PAY 399 

    0x1df823ed,// 403 PAY 400 

    0x02000000,// 404 PAY 401 

/// STA is 1 words. 

/// STA num_pkts       : 207 

/// STA pkt_idx        : 160 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1e 

    0x02811ecf // 405 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt59_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 504 words. 

/// BDA size     is 2011 (0x7db) 

/// BDA id       is 0x1f49 

    0x07db1f49,// 3 BDA   1 

/// PAY Generic Data size   : 2011 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x316ec9e6,// 4 PAY   1 

    0x57dbe114,// 5 PAY   2 

    0xef46b072,// 6 PAY   3 

    0x6ebb7964,// 7 PAY   4 

    0x81234c81,// 8 PAY   5 

    0xfb295eb6,// 9 PAY   6 

    0x0d212f70,// 10 PAY   7 

    0xf5066e35,// 11 PAY   8 

    0xdd8a0935,// 12 PAY   9 

    0x5a165920,// 13 PAY  10 

    0x3ce37a15,// 14 PAY  11 

    0x836020db,// 15 PAY  12 

    0xc81fd370,// 16 PAY  13 

    0xc6b5fca1,// 17 PAY  14 

    0x6e15a853,// 18 PAY  15 

    0xe39f7100,// 19 PAY  16 

    0x1e4a4155,// 20 PAY  17 

    0xfa11d53c,// 21 PAY  18 

    0x6e401cae,// 22 PAY  19 

    0x5a1121c1,// 23 PAY  20 

    0x6a3dbb83,// 24 PAY  21 

    0x86fc9a91,// 25 PAY  22 

    0xf4b7360a,// 26 PAY  23 

    0x7fd7d950,// 27 PAY  24 

    0x13da5d52,// 28 PAY  25 

    0x61fc7816,// 29 PAY  26 

    0xd9bb0fe9,// 30 PAY  27 

    0x474ee31e,// 31 PAY  28 

    0xc9dea47a,// 32 PAY  29 

    0xf6357b06,// 33 PAY  30 

    0x8515ce40,// 34 PAY  31 

    0x88d7594a,// 35 PAY  32 

    0xc40012a7,// 36 PAY  33 

    0x8f4858ca,// 37 PAY  34 

    0x8384ed79,// 38 PAY  35 

    0x984fb46d,// 39 PAY  36 

    0x54f989c0,// 40 PAY  37 

    0xfe368329,// 41 PAY  38 

    0xf5e04f6c,// 42 PAY  39 

    0xf447fb69,// 43 PAY  40 

    0x75d7861c,// 44 PAY  41 

    0x290b9429,// 45 PAY  42 

    0x7dd8dafa,// 46 PAY  43 

    0xc5990a3f,// 47 PAY  44 

    0x756ff8c3,// 48 PAY  45 

    0x873c79c0,// 49 PAY  46 

    0xfbd5f0b1,// 50 PAY  47 

    0x844ebec9,// 51 PAY  48 

    0x1419462c,// 52 PAY  49 

    0x9102be58,// 53 PAY  50 

    0xf4c32f94,// 54 PAY  51 

    0xa1e025fe,// 55 PAY  52 

    0x45ab017b,// 56 PAY  53 

    0x49d3c0ff,// 57 PAY  54 

    0xca9fa5f7,// 58 PAY  55 

    0xbb90d116,// 59 PAY  56 

    0x1887a1f3,// 60 PAY  57 

    0x67606d1e,// 61 PAY  58 

    0xffb87505,// 62 PAY  59 

    0xccd0db7b,// 63 PAY  60 

    0xf0ba833e,// 64 PAY  61 

    0xc09275e7,// 65 PAY  62 

    0xe03c9a9b,// 66 PAY  63 

    0x8e7064fe,// 67 PAY  64 

    0x6819e8d1,// 68 PAY  65 

    0x075e50b9,// 69 PAY  66 

    0x2495bf52,// 70 PAY  67 

    0x94435086,// 71 PAY  68 

    0x6e20284c,// 72 PAY  69 

    0x0f43af88,// 73 PAY  70 

    0x5403b65f,// 74 PAY  71 

    0xf466ec50,// 75 PAY  72 

    0x19c95873,// 76 PAY  73 

    0x98c00936,// 77 PAY  74 

    0x0b1941f7,// 78 PAY  75 

    0x4fc0737d,// 79 PAY  76 

    0x0c854e9b,// 80 PAY  77 

    0x3fc06b69,// 81 PAY  78 

    0xeaaa9a00,// 82 PAY  79 

    0xb680503e,// 83 PAY  80 

    0xa56df467,// 84 PAY  81 

    0x4635b96e,// 85 PAY  82 

    0x5b30ff34,// 86 PAY  83 

    0xc60f3b50,// 87 PAY  84 

    0x456d6b50,// 88 PAY  85 

    0xbd23dc37,// 89 PAY  86 

    0x83ce5e45,// 90 PAY  87 

    0x1b9cc8b8,// 91 PAY  88 

    0xbfa651ba,// 92 PAY  89 

    0xd82c3ca2,// 93 PAY  90 

    0x06fbed7f,// 94 PAY  91 

    0xac0d7103,// 95 PAY  92 

    0xde7e9938,// 96 PAY  93 

    0x93462194,// 97 PAY  94 

    0x2612a090,// 98 PAY  95 

    0xeac12fc5,// 99 PAY  96 

    0x1bc559b0,// 100 PAY  97 

    0x831df3d4,// 101 PAY  98 

    0x0c029dbc,// 102 PAY  99 

    0x5dbb856f,// 103 PAY 100 

    0xe37eb017,// 104 PAY 101 

    0x0ed068a1,// 105 PAY 102 

    0x2018021e,// 106 PAY 103 

    0x624565fd,// 107 PAY 104 

    0x801f4a07,// 108 PAY 105 

    0xd31050ca,// 109 PAY 106 

    0x8dcf067e,// 110 PAY 107 

    0x63eca92c,// 111 PAY 108 

    0x0c8ec5c6,// 112 PAY 109 

    0x955c9d1d,// 113 PAY 110 

    0x50df4510,// 114 PAY 111 

    0x3118655e,// 115 PAY 112 

    0x54fc5362,// 116 PAY 113 

    0x8a60a756,// 117 PAY 114 

    0x8f5cfe7b,// 118 PAY 115 

    0x21a0856c,// 119 PAY 116 

    0x6c431676,// 120 PAY 117 

    0xf3c99238,// 121 PAY 118 

    0xa94882d5,// 122 PAY 119 

    0x8acc209e,// 123 PAY 120 

    0x0c73f6ff,// 124 PAY 121 

    0xb87781c6,// 125 PAY 122 

    0x2643feb6,// 126 PAY 123 

    0xb51d1ecc,// 127 PAY 124 

    0xfa2bec9e,// 128 PAY 125 

    0xa607141d,// 129 PAY 126 

    0x44c0c188,// 130 PAY 127 

    0x5bf5a1c7,// 131 PAY 128 

    0x5e81d51b,// 132 PAY 129 

    0x25271685,// 133 PAY 130 

    0xe758c55e,// 134 PAY 131 

    0x90a66427,// 135 PAY 132 

    0xe0fd657f,// 136 PAY 133 

    0xeeb9c27f,// 137 PAY 134 

    0x938336e7,// 138 PAY 135 

    0xf76aced0,// 139 PAY 136 

    0x8d82cea5,// 140 PAY 137 

    0xa134b005,// 141 PAY 138 

    0x068e6b39,// 142 PAY 139 

    0xadcb7c32,// 143 PAY 140 

    0x26234010,// 144 PAY 141 

    0x0571b41a,// 145 PAY 142 

    0xbacd79d5,// 146 PAY 143 

    0xff1ccc4c,// 147 PAY 144 

    0x93ee7288,// 148 PAY 145 

    0xb061ee7c,// 149 PAY 146 

    0x382da947,// 150 PAY 147 

    0x56b4ecdc,// 151 PAY 148 

    0x381908f7,// 152 PAY 149 

    0xfa122954,// 153 PAY 150 

    0x89b6c578,// 154 PAY 151 

    0xb48b4896,// 155 PAY 152 

    0xb585fec6,// 156 PAY 153 

    0xc8c6c711,// 157 PAY 154 

    0xf8f4557d,// 158 PAY 155 

    0xe1fa1754,// 159 PAY 156 

    0x4e2fc837,// 160 PAY 157 

    0x193d9480,// 161 PAY 158 

    0xde6d7ffe,// 162 PAY 159 

    0xc8586a57,// 163 PAY 160 

    0xbbcc208c,// 164 PAY 161 

    0xa887bcdc,// 165 PAY 162 

    0xda88c94c,// 166 PAY 163 

    0x2fefbc73,// 167 PAY 164 

    0x02814a4e,// 168 PAY 165 

    0x8b0b407c,// 169 PAY 166 

    0x8398b4ef,// 170 PAY 167 

    0xb02862f7,// 171 PAY 168 

    0x55eff83a,// 172 PAY 169 

    0x4f6bf8f1,// 173 PAY 170 

    0x64c0c8d1,// 174 PAY 171 

    0xa1b0979e,// 175 PAY 172 

    0x19820a61,// 176 PAY 173 

    0x058c5388,// 177 PAY 174 

    0xe41bdbdb,// 178 PAY 175 

    0x47b4cc74,// 179 PAY 176 

    0x83f316aa,// 180 PAY 177 

    0xd7eea318,// 181 PAY 178 

    0x695c8c44,// 182 PAY 179 

    0x1a09af36,// 183 PAY 180 

    0x8242e242,// 184 PAY 181 

    0x3b40134e,// 185 PAY 182 

    0x83323c8b,// 186 PAY 183 

    0x40b60336,// 187 PAY 184 

    0xa32f039b,// 188 PAY 185 

    0x76a8d384,// 189 PAY 186 

    0xaa639027,// 190 PAY 187 

    0x7e445a3f,// 191 PAY 188 

    0x73d15bf9,// 192 PAY 189 

    0x33818135,// 193 PAY 190 

    0x97c3b878,// 194 PAY 191 

    0x69be5334,// 195 PAY 192 

    0xe6d4cdb0,// 196 PAY 193 

    0x6575447d,// 197 PAY 194 

    0x9643d737,// 198 PAY 195 

    0x0bea3d28,// 199 PAY 196 

    0xf08cc894,// 200 PAY 197 

    0xae5008df,// 201 PAY 198 

    0x6d35628e,// 202 PAY 199 

    0x2db27023,// 203 PAY 200 

    0x7770971b,// 204 PAY 201 

    0x2e68f9c4,// 205 PAY 202 

    0xadb49aac,// 206 PAY 203 

    0xafa10a09,// 207 PAY 204 

    0x3d28c0a0,// 208 PAY 205 

    0x2dfe4be7,// 209 PAY 206 

    0x88c94dd1,// 210 PAY 207 

    0x63b390d2,// 211 PAY 208 

    0x8614f6d3,// 212 PAY 209 

    0x830c55da,// 213 PAY 210 

    0x47238604,// 214 PAY 211 

    0x059e6abf,// 215 PAY 212 

    0x243fcf07,// 216 PAY 213 

    0x7564140b,// 217 PAY 214 

    0xa6e45bc3,// 218 PAY 215 

    0x930834b8,// 219 PAY 216 

    0xf36edff6,// 220 PAY 217 

    0xc32dbe56,// 221 PAY 218 

    0x923e9494,// 222 PAY 219 

    0xd8d75220,// 223 PAY 220 

    0x70d46eb5,// 224 PAY 221 

    0x9734272e,// 225 PAY 222 

    0x0b770fb1,// 226 PAY 223 

    0x2ff02f93,// 227 PAY 224 

    0xf1427484,// 228 PAY 225 

    0xf758a5ce,// 229 PAY 226 

    0xfecb0087,// 230 PAY 227 

    0xb4e82d1b,// 231 PAY 228 

    0x0054281c,// 232 PAY 229 

    0xe732978e,// 233 PAY 230 

    0xfa738049,// 234 PAY 231 

    0x748b50fe,// 235 PAY 232 

    0x78c01ade,// 236 PAY 233 

    0x222d0d4d,// 237 PAY 234 

    0xc44357d7,// 238 PAY 235 

    0xb806840c,// 239 PAY 236 

    0xb1820fd6,// 240 PAY 237 

    0x54cfdc3d,// 241 PAY 238 

    0x6fe4e13e,// 242 PAY 239 

    0x313311ba,// 243 PAY 240 

    0x56a4f897,// 244 PAY 241 

    0x2cd99103,// 245 PAY 242 

    0x2d46dc26,// 246 PAY 243 

    0x11ac836d,// 247 PAY 244 

    0x6d8dbf3a,// 248 PAY 245 

    0x9bfbc648,// 249 PAY 246 

    0x62dc818f,// 250 PAY 247 

    0x0bc64299,// 251 PAY 248 

    0x8c94e5e0,// 252 PAY 249 

    0xc9e9b548,// 253 PAY 250 

    0xeaf55cc8,// 254 PAY 251 

    0x78b64562,// 255 PAY 252 

    0x1bac6470,// 256 PAY 253 

    0xd85b30f1,// 257 PAY 254 

    0xe19aa58a,// 258 PAY 255 

    0xacf431c3,// 259 PAY 256 

    0xf236f9a1,// 260 PAY 257 

    0xcee21e9a,// 261 PAY 258 

    0xeb0b4a77,// 262 PAY 259 

    0xe8109a21,// 263 PAY 260 

    0xe8bd526d,// 264 PAY 261 

    0x4269cbc5,// 265 PAY 262 

    0xb4d05c11,// 266 PAY 263 

    0x989aa323,// 267 PAY 264 

    0x1aa71aaa,// 268 PAY 265 

    0x991ddc5d,// 269 PAY 266 

    0x8581260f,// 270 PAY 267 

    0x5e318407,// 271 PAY 268 

    0x94e9aab2,// 272 PAY 269 

    0xb13309a3,// 273 PAY 270 

    0x63017d13,// 274 PAY 271 

    0x0ccf4163,// 275 PAY 272 

    0xd37356e5,// 276 PAY 273 

    0xf48fe96e,// 277 PAY 274 

    0x2a24d82d,// 278 PAY 275 

    0x01a9bc94,// 279 PAY 276 

    0x0206dede,// 280 PAY 277 

    0xdd9f6d9e,// 281 PAY 278 

    0xc5055506,// 282 PAY 279 

    0xd3c20a84,// 283 PAY 280 

    0x69d27cc7,// 284 PAY 281 

    0x226afa11,// 285 PAY 282 

    0x49057bd8,// 286 PAY 283 

    0xd209f6b4,// 287 PAY 284 

    0xb46a2dd9,// 288 PAY 285 

    0x276abeb2,// 289 PAY 286 

    0x5a812d4a,// 290 PAY 287 

    0x26e08897,// 291 PAY 288 

    0xf2b06f1d,// 292 PAY 289 

    0x0580f69a,// 293 PAY 290 

    0x874e4dc4,// 294 PAY 291 

    0xf2345719,// 295 PAY 292 

    0x9cdc6bfe,// 296 PAY 293 

    0xbbab3918,// 297 PAY 294 

    0x6042be21,// 298 PAY 295 

    0x66dc88ce,// 299 PAY 296 

    0xc33723a9,// 300 PAY 297 

    0x52dc4290,// 301 PAY 298 

    0x70acf36a,// 302 PAY 299 

    0xbc3d27aa,// 303 PAY 300 

    0x1ed7df42,// 304 PAY 301 

    0x9d2869ff,// 305 PAY 302 

    0x64792bfe,// 306 PAY 303 

    0x2dd28080,// 307 PAY 304 

    0x8942aecc,// 308 PAY 305 

    0x3b6660b8,// 309 PAY 306 

    0x6ad20426,// 310 PAY 307 

    0x1904ed9a,// 311 PAY 308 

    0xb102b9db,// 312 PAY 309 

    0x1955c2da,// 313 PAY 310 

    0x281ccfa4,// 314 PAY 311 

    0x85234ca3,// 315 PAY 312 

    0xea2c8ddd,// 316 PAY 313 

    0xba337c40,// 317 PAY 314 

    0xcc3b25e5,// 318 PAY 315 

    0x9d836771,// 319 PAY 316 

    0x0dfe748e,// 320 PAY 317 

    0x3a46f1c7,// 321 PAY 318 

    0x98ed6579,// 322 PAY 319 

    0xe48fe0e8,// 323 PAY 320 

    0x010eef1c,// 324 PAY 321 

    0x2b3fc12a,// 325 PAY 322 

    0x3945f9ee,// 326 PAY 323 

    0x356bb640,// 327 PAY 324 

    0x51512f12,// 328 PAY 325 

    0x265ef7ae,// 329 PAY 326 

    0x595a9ce1,// 330 PAY 327 

    0x1b926409,// 331 PAY 328 

    0x4dd35230,// 332 PAY 329 

    0x1c188671,// 333 PAY 330 

    0x144eee76,// 334 PAY 331 

    0x4333dec8,// 335 PAY 332 

    0xf7bb7de2,// 336 PAY 333 

    0x89ec2acd,// 337 PAY 334 

    0x49088934,// 338 PAY 335 

    0xaa3bd1d6,// 339 PAY 336 

    0x1ebfaf8c,// 340 PAY 337 

    0xbe8096ab,// 341 PAY 338 

    0x63e84600,// 342 PAY 339 

    0x7e5d3e3d,// 343 PAY 340 

    0xe77c4003,// 344 PAY 341 

    0xfb421fd8,// 345 PAY 342 

    0x7df6c682,// 346 PAY 343 

    0x17de5fba,// 347 PAY 344 

    0x3bfe315a,// 348 PAY 345 

    0x3ed53bbf,// 349 PAY 346 

    0xa37224bc,// 350 PAY 347 

    0x6799e762,// 351 PAY 348 

    0x317a4c3b,// 352 PAY 349 

    0xb256d298,// 353 PAY 350 

    0x30bd039e,// 354 PAY 351 

    0xf5f81bb3,// 355 PAY 352 

    0xddf4559b,// 356 PAY 353 

    0xc3615f6e,// 357 PAY 354 

    0x99f7ede8,// 358 PAY 355 

    0x813b9d6c,// 359 PAY 356 

    0x24d85ad2,// 360 PAY 357 

    0x90b0f229,// 361 PAY 358 

    0x89bdd0ef,// 362 PAY 359 

    0xa55c077c,// 363 PAY 360 

    0xac752890,// 364 PAY 361 

    0xf8266fd8,// 365 PAY 362 

    0x88cfb298,// 366 PAY 363 

    0x16356390,// 367 PAY 364 

    0x2e16bc41,// 368 PAY 365 

    0xfbe99a09,// 369 PAY 366 

    0x2e91c8ad,// 370 PAY 367 

    0xcfd079d7,// 371 PAY 368 

    0x77e29276,// 372 PAY 369 

    0x7d336873,// 373 PAY 370 

    0x34602e75,// 374 PAY 371 

    0x8770d505,// 375 PAY 372 

    0x9bb96f8d,// 376 PAY 373 

    0x37bf307f,// 377 PAY 374 

    0xe42d1f41,// 378 PAY 375 

    0xba821e17,// 379 PAY 376 

    0xbd245575,// 380 PAY 377 

    0xa6c10db5,// 381 PAY 378 

    0x2de6e446,// 382 PAY 379 

    0xb5a02b80,// 383 PAY 380 

    0x70018e44,// 384 PAY 381 

    0x93a8194c,// 385 PAY 382 

    0x60fc5ed5,// 386 PAY 383 

    0xcea40a16,// 387 PAY 384 

    0x0f1e1d3c,// 388 PAY 385 

    0xae1ef164,// 389 PAY 386 

    0x63c7d7d1,// 390 PAY 387 

    0x3edd9313,// 391 PAY 388 

    0xe379355c,// 392 PAY 389 

    0xe742d5a4,// 393 PAY 390 

    0x645a792f,// 394 PAY 391 

    0xedc972df,// 395 PAY 392 

    0xcbe27842,// 396 PAY 393 

    0x2193a206,// 397 PAY 394 

    0xb1a270f7,// 398 PAY 395 

    0xc5413a7d,// 399 PAY 396 

    0x0784fd6c,// 400 PAY 397 

    0x39d336fb,// 401 PAY 398 

    0xc74a0f41,// 402 PAY 399 

    0x8b6594fe,// 403 PAY 400 

    0x66dcd55b,// 404 PAY 401 

    0xb989f1fc,// 405 PAY 402 

    0xe988e86c,// 406 PAY 403 

    0xe2ae6108,// 407 PAY 404 

    0xba85c8b1,// 408 PAY 405 

    0x3518d7db,// 409 PAY 406 

    0xe979a42e,// 410 PAY 407 

    0xc3ff24b5,// 411 PAY 408 

    0x35fdd523,// 412 PAY 409 

    0x6eab508c,// 413 PAY 410 

    0x1e9f3e99,// 414 PAY 411 

    0x3f8eab1e,// 415 PAY 412 

    0x32c3e8f3,// 416 PAY 413 

    0x7b5cd77e,// 417 PAY 414 

    0xd1878c7f,// 418 PAY 415 

    0x582d03e0,// 419 PAY 416 

    0x8389f62f,// 420 PAY 417 

    0xdbc07442,// 421 PAY 418 

    0x82079a64,// 422 PAY 419 

    0x9954c8e3,// 423 PAY 420 

    0x84a758c4,// 424 PAY 421 

    0x23564c9b,// 425 PAY 422 

    0x4715233a,// 426 PAY 423 

    0xbd30c208,// 427 PAY 424 

    0x0bd5a271,// 428 PAY 425 

    0xa2d6c257,// 429 PAY 426 

    0x9e07f740,// 430 PAY 427 

    0xcfaacf09,// 431 PAY 428 

    0x13b537dc,// 432 PAY 429 

    0x6cb86579,// 433 PAY 430 

    0x912af7dc,// 434 PAY 431 

    0xee671e50,// 435 PAY 432 

    0x516a8009,// 436 PAY 433 

    0x06c6cc23,// 437 PAY 434 

    0x819d4e59,// 438 PAY 435 

    0xf56ee244,// 439 PAY 436 

    0xad85a3d0,// 440 PAY 437 

    0xf89d49a2,// 441 PAY 438 

    0xc7b79690,// 442 PAY 439 

    0xf4d0c7e9,// 443 PAY 440 

    0xa513a7cb,// 444 PAY 441 

    0x0fef36e7,// 445 PAY 442 

    0xbbe7f192,// 446 PAY 443 

    0x5dd0142b,// 447 PAY 444 

    0xea7d8736,// 448 PAY 445 

    0x164c449f,// 449 PAY 446 

    0xef64360e,// 450 PAY 447 

    0x146ea541,// 451 PAY 448 

    0x3d5f8add,// 452 PAY 449 

    0xf925f206,// 453 PAY 450 

    0x74b9d0b0,// 454 PAY 451 

    0x36f26f72,// 455 PAY 452 

    0xceaebff5,// 456 PAY 453 

    0xf3dc85e3,// 457 PAY 454 

    0xeb941a7d,// 458 PAY 455 

    0x78d93de5,// 459 PAY 456 

    0xe4e6bcab,// 460 PAY 457 

    0x1160e408,// 461 PAY 458 

    0xb624948f,// 462 PAY 459 

    0xde6f2fe4,// 463 PAY 460 

    0xf09bccac,// 464 PAY 461 

    0x859d3277,// 465 PAY 462 

    0x11251992,// 466 PAY 463 

    0xb7278bf4,// 467 PAY 464 

    0x5e795bd0,// 468 PAY 465 

    0x8100ef6e,// 469 PAY 466 

    0xa3ceaccb,// 470 PAY 467 

    0xe0b8451a,// 471 PAY 468 

    0x88acedde,// 472 PAY 469 

    0xfe37c612,// 473 PAY 470 

    0x928b53a9,// 474 PAY 471 

    0x339f22ec,// 475 PAY 472 

    0x0ee37281,// 476 PAY 473 

    0xa1dd43e2,// 477 PAY 474 

    0xf2e43e7b,// 478 PAY 475 

    0xda2e75cc,// 479 PAY 476 

    0x6ae9371e,// 480 PAY 477 

    0x1dfa0f88,// 481 PAY 478 

    0xc747b41b,// 482 PAY 479 

    0x50cb46cb,// 483 PAY 480 

    0x93c1d803,// 484 PAY 481 

    0x4eaeab18,// 485 PAY 482 

    0x6cb0e54a,// 486 PAY 483 

    0xf2449d7b,// 487 PAY 484 

    0x32a37488,// 488 PAY 485 

    0xe1be2eed,// 489 PAY 486 

    0x55be92bf,// 490 PAY 487 

    0x6d26b208,// 491 PAY 488 

    0x2c73ad8d,// 492 PAY 489 

    0x914cc624,// 493 PAY 490 

    0x9964f42d,// 494 PAY 491 

    0x7cd441e5,// 495 PAY 492 

    0x78a82683,// 496 PAY 493 

    0xb170765e,// 497 PAY 494 

    0xa4cc694c,// 498 PAY 495 

    0x40322134,// 499 PAY 496 

    0xbc52fbce,// 500 PAY 497 

    0x2097bbcd,// 501 PAY 498 

    0x05829cd1,// 502 PAY 499 

    0xef8d3d69,// 503 PAY 500 

    0x38d6175b,// 504 PAY 501 

    0x9ef86122,// 505 PAY 502 

    0x4449e400,// 506 PAY 503 

/// HASH is  12 bytes 

    0xd37356e5,// 507 HSH   1 

    0xf48fe96e,// 508 HSH   2 

    0x2a24d82d,// 509 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 244 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7a 

    0x01107af4 // 510 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt60_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 271 words. 

/// BDA size     is 1077 (0x435) 

/// BDA id       is 0x6d6b 

    0x04356d6b,// 3 BDA   1 

/// PAY Generic Data size   : 1077 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0563b74f,// 4 PAY   1 

    0xa847150c,// 5 PAY   2 

    0xf0cc4df0,// 6 PAY   3 

    0x62d9a1ae,// 7 PAY   4 

    0xd6c9577e,// 8 PAY   5 

    0x220c924f,// 9 PAY   6 

    0x6e4d1e2b,// 10 PAY   7 

    0x5234528f,// 11 PAY   8 

    0x5d4d4525,// 12 PAY   9 

    0x952377ae,// 13 PAY  10 

    0x0b877731,// 14 PAY  11 

    0xf165d633,// 15 PAY  12 

    0x7afceb4f,// 16 PAY  13 

    0x2e881570,// 17 PAY  14 

    0x861d2630,// 18 PAY  15 

    0xfcac507f,// 19 PAY  16 

    0x3bf6577c,// 20 PAY  17 

    0x1bfa71cf,// 21 PAY  18 

    0xd2b6862e,// 22 PAY  19 

    0x5e11c2ab,// 23 PAY  20 

    0xe5ed1b88,// 24 PAY  21 

    0x494bd55c,// 25 PAY  22 

    0x9cd9b181,// 26 PAY  23 

    0x24f97611,// 27 PAY  24 

    0x1cdcaed7,// 28 PAY  25 

    0xda405a16,// 29 PAY  26 

    0x90abdecc,// 30 PAY  27 

    0xa368c666,// 31 PAY  28 

    0x92caddb4,// 32 PAY  29 

    0xc588fef7,// 33 PAY  30 

    0xaef0508b,// 34 PAY  31 

    0xd0c1c6b0,// 35 PAY  32 

    0xdaeb7337,// 36 PAY  33 

    0xf3ae41ff,// 37 PAY  34 

    0x39c8c638,// 38 PAY  35 

    0x52fbfb74,// 39 PAY  36 

    0x5a5c8e1b,// 40 PAY  37 

    0x5f42bbf0,// 41 PAY  38 

    0x1b1560b6,// 42 PAY  39 

    0xa450f3d1,// 43 PAY  40 

    0xe1731cda,// 44 PAY  41 

    0xc02f6c35,// 45 PAY  42 

    0xbd4e6c33,// 46 PAY  43 

    0x23868ca1,// 47 PAY  44 

    0xef2622bc,// 48 PAY  45 

    0x05f73fe4,// 49 PAY  46 

    0x53dfbd6e,// 50 PAY  47 

    0x4dae091b,// 51 PAY  48 

    0x133bbf22,// 52 PAY  49 

    0x7928748c,// 53 PAY  50 

    0x64d09b75,// 54 PAY  51 

    0xf0372281,// 55 PAY  52 

    0x5c00a13c,// 56 PAY  53 

    0xafd76a3b,// 57 PAY  54 

    0x4cb152b1,// 58 PAY  55 

    0x1aee81bc,// 59 PAY  56 

    0xae464700,// 60 PAY  57 

    0xce169f7f,// 61 PAY  58 

    0xca15fa4e,// 62 PAY  59 

    0x08094514,// 63 PAY  60 

    0xbefdfcb9,// 64 PAY  61 

    0xd3a0a132,// 65 PAY  62 

    0x0f5ff80d,// 66 PAY  63 

    0x7eba0a9f,// 67 PAY  64 

    0x4debb848,// 68 PAY  65 

    0x38c180dd,// 69 PAY  66 

    0xcbe5f48a,// 70 PAY  67 

    0x788b70c3,// 71 PAY  68 

    0x518b2b4b,// 72 PAY  69 

    0xb345232f,// 73 PAY  70 

    0x74fb1a17,// 74 PAY  71 

    0xd7078667,// 75 PAY  72 

    0x8cb4a22b,// 76 PAY  73 

    0xac5b0937,// 77 PAY  74 

    0xdb8e9d08,// 78 PAY  75 

    0x854545fa,// 79 PAY  76 

    0x0e3c100f,// 80 PAY  77 

    0x4c0a2689,// 81 PAY  78 

    0xf9e750e9,// 82 PAY  79 

    0x123edbad,// 83 PAY  80 

    0x68f6888b,// 84 PAY  81 

    0xb3eb7256,// 85 PAY  82 

    0xd5f9302f,// 86 PAY  83 

    0x100f8732,// 87 PAY  84 

    0x70caa38c,// 88 PAY  85 

    0x9a908355,// 89 PAY  86 

    0x9bbb1817,// 90 PAY  87 

    0x3e1c3377,// 91 PAY  88 

    0x8e49d671,// 92 PAY  89 

    0xe35a1fec,// 93 PAY  90 

    0x233f4c8e,// 94 PAY  91 

    0x63bee6fd,// 95 PAY  92 

    0x99ead7aa,// 96 PAY  93 

    0xb987615b,// 97 PAY  94 

    0x83e06833,// 98 PAY  95 

    0x25b4aefa,// 99 PAY  96 

    0x852d22ae,// 100 PAY  97 

    0x8a6eaf80,// 101 PAY  98 

    0xac7e6528,// 102 PAY  99 

    0x5db51095,// 103 PAY 100 

    0x82794e23,// 104 PAY 101 

    0x7344f0a7,// 105 PAY 102 

    0x0c88db72,// 106 PAY 103 

    0x73c2b790,// 107 PAY 104 

    0x64ffd0c8,// 108 PAY 105 

    0x620d591d,// 109 PAY 106 

    0x3e8c7cdf,// 110 PAY 107 

    0x12369bac,// 111 PAY 108 

    0xb4762d4e,// 112 PAY 109 

    0x47010767,// 113 PAY 110 

    0xfe6d5f1c,// 114 PAY 111 

    0xc21c05c7,// 115 PAY 112 

    0x3501223b,// 116 PAY 113 

    0xa027f453,// 117 PAY 114 

    0x8a79e95b,// 118 PAY 115 

    0xa09bbb46,// 119 PAY 116 

    0x732d382e,// 120 PAY 117 

    0x7b952202,// 121 PAY 118 

    0x976230c3,// 122 PAY 119 

    0xdb189bb6,// 123 PAY 120 

    0xdd211d83,// 124 PAY 121 

    0x8132b286,// 125 PAY 122 

    0x08c25a85,// 126 PAY 123 

    0x1cfa6397,// 127 PAY 124 

    0x83c76731,// 128 PAY 125 

    0x0742c10e,// 129 PAY 126 

    0xad892a86,// 130 PAY 127 

    0x46711935,// 131 PAY 128 

    0x1466c62e,// 132 PAY 129 

    0x9a775c02,// 133 PAY 130 

    0x7effe4e5,// 134 PAY 131 

    0xc698a934,// 135 PAY 132 

    0xcbe3e455,// 136 PAY 133 

    0x85d2b436,// 137 PAY 134 

    0x8962c1c9,// 138 PAY 135 

    0x84e19b02,// 139 PAY 136 

    0x5b88ddca,// 140 PAY 137 

    0xe5ce1e61,// 141 PAY 138 

    0x98e51ebe,// 142 PAY 139 

    0xa06b6235,// 143 PAY 140 

    0x879f7bc2,// 144 PAY 141 

    0xae66e25a,// 145 PAY 142 

    0x8bcc665b,// 146 PAY 143 

    0xdf655ad1,// 147 PAY 144 

    0x515d9ee8,// 148 PAY 145 

    0x4f011218,// 149 PAY 146 

    0x352da860,// 150 PAY 147 

    0xc5ff4d67,// 151 PAY 148 

    0xa7356eea,// 152 PAY 149 

    0xc0a83739,// 153 PAY 150 

    0x7fd30891,// 154 PAY 151 

    0xd040d325,// 155 PAY 152 

    0x0171ea95,// 156 PAY 153 

    0xf721cfcd,// 157 PAY 154 

    0x1a7e78bc,// 158 PAY 155 

    0x0955d93c,// 159 PAY 156 

    0xd2c9dfe6,// 160 PAY 157 

    0x1384c9dc,// 161 PAY 158 

    0x4b0428e2,// 162 PAY 159 

    0xc529a9dd,// 163 PAY 160 

    0x7c1aadc7,// 164 PAY 161 

    0xda01ac75,// 165 PAY 162 

    0xec621064,// 166 PAY 163 

    0x8a7a4d9b,// 167 PAY 164 

    0xb02a79d2,// 168 PAY 165 

    0xc0d65a8f,// 169 PAY 166 

    0x1c097008,// 170 PAY 167 

    0x5b0255a8,// 171 PAY 168 

    0x0520980b,// 172 PAY 169 

    0xb231b77c,// 173 PAY 170 

    0x753385f1,// 174 PAY 171 

    0x2666629a,// 175 PAY 172 

    0x3619109e,// 176 PAY 173 

    0x600a5bbc,// 177 PAY 174 

    0x352804d8,// 178 PAY 175 

    0xa3fe9455,// 179 PAY 176 

    0xbd704350,// 180 PAY 177 

    0x762bef74,// 181 PAY 178 

    0x17c57615,// 182 PAY 179 

    0xc41310cf,// 183 PAY 180 

    0x823351d8,// 184 PAY 181 

    0x6d99fdcc,// 185 PAY 182 

    0x41936330,// 186 PAY 183 

    0x3486dd17,// 187 PAY 184 

    0xf28c563a,// 188 PAY 185 

    0x8f0858bb,// 189 PAY 186 

    0x03a7ef78,// 190 PAY 187 

    0x795bc02c,// 191 PAY 188 

    0x9ac87d81,// 192 PAY 189 

    0x0f3b729a,// 193 PAY 190 

    0xbc4067c1,// 194 PAY 191 

    0x47d5c5fd,// 195 PAY 192 

    0x6a805656,// 196 PAY 193 

    0xa7da2cad,// 197 PAY 194 

    0x10557b6d,// 198 PAY 195 

    0x7d1f6837,// 199 PAY 196 

    0x80324980,// 200 PAY 197 

    0x17faa5b8,// 201 PAY 198 

    0x6aa12720,// 202 PAY 199 

    0x458b0e78,// 203 PAY 200 

    0xbda0409f,// 204 PAY 201 

    0x5af3cd12,// 205 PAY 202 

    0x25397088,// 206 PAY 203 

    0x5cca54e0,// 207 PAY 204 

    0x1892c8f4,// 208 PAY 205 

    0xbfd0b2fe,// 209 PAY 206 

    0xd29a4b38,// 210 PAY 207 

    0x951c043c,// 211 PAY 208 

    0xf062be56,// 212 PAY 209 

    0xe6a4997d,// 213 PAY 210 

    0xa73df7fc,// 214 PAY 211 

    0x1bb7d39e,// 215 PAY 212 

    0x6fabe8aa,// 216 PAY 213 

    0x4a1bbecb,// 217 PAY 214 

    0x6c9c1c28,// 218 PAY 215 

    0x5e2bb4da,// 219 PAY 216 

    0xeaa0b09b,// 220 PAY 217 

    0x798fc14a,// 221 PAY 218 

    0x6d14ecf7,// 222 PAY 219 

    0xe3d5cfc8,// 223 PAY 220 

    0x868022d5,// 224 PAY 221 

    0x4ef5832f,// 225 PAY 222 

    0xe632d969,// 226 PAY 223 

    0x89da5e94,// 227 PAY 224 

    0xae4a68fc,// 228 PAY 225 

    0xc894e55f,// 229 PAY 226 

    0xc605c8a0,// 230 PAY 227 

    0x20e88a44,// 231 PAY 228 

    0x8140cc41,// 232 PAY 229 

    0xa20ff1ce,// 233 PAY 230 

    0x5f35a165,// 234 PAY 231 

    0x24cb9fde,// 235 PAY 232 

    0x2605e6b6,// 236 PAY 233 

    0x6622a2a2,// 237 PAY 234 

    0xa1884d71,// 238 PAY 235 

    0x78fd88c3,// 239 PAY 236 

    0x6aa1fd1c,// 240 PAY 237 

    0xb6f99946,// 241 PAY 238 

    0x07a07f92,// 242 PAY 239 

    0x00b3d27d,// 243 PAY 240 

    0x8cc41be3,// 244 PAY 241 

    0x0f34e924,// 245 PAY 242 

    0xf5388c93,// 246 PAY 243 

    0xf8ec9370,// 247 PAY 244 

    0x8576012a,// 248 PAY 245 

    0x568216d6,// 249 PAY 246 

    0x3f3c6432,// 250 PAY 247 

    0x706171b5,// 251 PAY 248 

    0x2a9ffa3e,// 252 PAY 249 

    0xd8592cd2,// 253 PAY 250 

    0x7168dbfa,// 254 PAY 251 

    0x5b6a9470,// 255 PAY 252 

    0x2666dbcb,// 256 PAY 253 

    0x6b851531,// 257 PAY 254 

    0xd38beeba,// 258 PAY 255 

    0x2f5f1582,// 259 PAY 256 

    0xe4cf79ff,// 260 PAY 257 

    0x966fb515,// 261 PAY 258 

    0xd863dec9,// 262 PAY 259 

    0xfe7ffd8f,// 263 PAY 260 

    0xf91c89a3,// 264 PAY 261 

    0xb14d9afc,// 265 PAY 262 

    0x9bdac21e,// 266 PAY 263 

    0x39b7d318,// 267 PAY 264 

    0x51a7148c,// 268 PAY 265 

    0xff64fcd5,// 269 PAY 266 

    0x68f99d48,// 270 PAY 267 

    0xceffb823,// 271 PAY 268 

    0x4081abb6,// 272 PAY 269 

    0xac000000,// 273 PAY 270 

/// HASH is  12 bytes 

    0xb14d9afc,// 274 HSH   1 

    0x9bdac21e,// 275 HSH   2 

    0x39b7d318,// 276 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 18 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xda 

    0x0051da12 // 277 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt61_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 118 words. 

/// BDA size     is 465 (0x1d1) 

/// BDA id       is 0xb58b 

    0x01d1b58b,// 3 BDA   1 

/// PAY Generic Data size   : 465 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x934523e8,// 4 PAY   1 

    0x3215014e,// 5 PAY   2 

    0x58972a2c,// 6 PAY   3 

    0xf6d8edc5,// 7 PAY   4 

    0x2e962b82,// 8 PAY   5 

    0x485c99d8,// 9 PAY   6 

    0xd8ae936d,// 10 PAY   7 

    0x18ae98ad,// 11 PAY   8 

    0x13a2fd26,// 12 PAY   9 

    0xa12384cc,// 13 PAY  10 

    0x4da39508,// 14 PAY  11 

    0xebc2fb5f,// 15 PAY  12 

    0x3eddf7c1,// 16 PAY  13 

    0x180e4002,// 17 PAY  14 

    0xba26b276,// 18 PAY  15 

    0xb007af1c,// 19 PAY  16 

    0xd8949def,// 20 PAY  17 

    0x65ab970b,// 21 PAY  18 

    0x08f921a4,// 22 PAY  19 

    0x7e8b6c13,// 23 PAY  20 

    0x590a999f,// 24 PAY  21 

    0xa93a409e,// 25 PAY  22 

    0x70f111f0,// 26 PAY  23 

    0xdd46c662,// 27 PAY  24 

    0x081155ae,// 28 PAY  25 

    0x3bb1cf67,// 29 PAY  26 

    0x6e1260e4,// 30 PAY  27 

    0x0d5c75a9,// 31 PAY  28 

    0x06ab90e7,// 32 PAY  29 

    0x93711c5a,// 33 PAY  30 

    0x38bda3f0,// 34 PAY  31 

    0xfb7b0a3b,// 35 PAY  32 

    0x72b2e5cb,// 36 PAY  33 

    0x0e4a0d7a,// 37 PAY  34 

    0x5dd443d0,// 38 PAY  35 

    0x81782db8,// 39 PAY  36 

    0xd64ec731,// 40 PAY  37 

    0xdca0f562,// 41 PAY  38 

    0xc1bb70df,// 42 PAY  39 

    0x8093f14e,// 43 PAY  40 

    0x5421a821,// 44 PAY  41 

    0x9a3ad9fb,// 45 PAY  42 

    0xe4c38e2d,// 46 PAY  43 

    0xaa1951b5,// 47 PAY  44 

    0x15993a02,// 48 PAY  45 

    0x65ffc0c0,// 49 PAY  46 

    0xf71fccc2,// 50 PAY  47 

    0x5de33066,// 51 PAY  48 

    0xb7de89af,// 52 PAY  49 

    0x4e514fba,// 53 PAY  50 

    0x5be1a258,// 54 PAY  51 

    0x663add1f,// 55 PAY  52 

    0x4b113471,// 56 PAY  53 

    0x5530dc2c,// 57 PAY  54 

    0x82547ac0,// 58 PAY  55 

    0x6024d3ca,// 59 PAY  56 

    0x04ae6de5,// 60 PAY  57 

    0xc9750621,// 61 PAY  58 

    0x51e9ffad,// 62 PAY  59 

    0xc9af76f4,// 63 PAY  60 

    0xb27adb11,// 64 PAY  61 

    0x6972f129,// 65 PAY  62 

    0x5964ab03,// 66 PAY  63 

    0xc2a270b8,// 67 PAY  64 

    0x92723007,// 68 PAY  65 

    0x113c7a5b,// 69 PAY  66 

    0xd2b0fd61,// 70 PAY  67 

    0x430c0fac,// 71 PAY  68 

    0xc96660e3,// 72 PAY  69 

    0x94124a18,// 73 PAY  70 

    0x6fa4ca6b,// 74 PAY  71 

    0x52834ea6,// 75 PAY  72 

    0xcf57d3f8,// 76 PAY  73 

    0x97c981c1,// 77 PAY  74 

    0x5644f15e,// 78 PAY  75 

    0x98901130,// 79 PAY  76 

    0xe66ab173,// 80 PAY  77 

    0x8ccad062,// 81 PAY  78 

    0x2896f173,// 82 PAY  79 

    0xaff28f34,// 83 PAY  80 

    0xf1909c1b,// 84 PAY  81 

    0x58628b58,// 85 PAY  82 

    0x550e6299,// 86 PAY  83 

    0x4bdf023e,// 87 PAY  84 

    0x7d994681,// 88 PAY  85 

    0x9a530e5a,// 89 PAY  86 

    0xd5ff1940,// 90 PAY  87 

    0x9564b354,// 91 PAY  88 

    0x0957c73a,// 92 PAY  89 

    0x6d8914bc,// 93 PAY  90 

    0xfdd1be04,// 94 PAY  91 

    0xd062fc10,// 95 PAY  92 

    0x75eab4f0,// 96 PAY  93 

    0xf79ef619,// 97 PAY  94 

    0x9306e23a,// 98 PAY  95 

    0xe64582d9,// 99 PAY  96 

    0x7d8b3701,// 100 PAY  97 

    0x0bdba11d,// 101 PAY  98 

    0xb938fd22,// 102 PAY  99 

    0xe9700e15,// 103 PAY 100 

    0xe0c42885,// 104 PAY 101 

    0x0c347050,// 105 PAY 102 

    0x5611c18d,// 106 PAY 103 

    0xa583d082,// 107 PAY 104 

    0xdc584f05,// 108 PAY 105 

    0xad4864bc,// 109 PAY 106 

    0xdedab474,// 110 PAY 107 

    0x80fcc443,// 111 PAY 108 

    0x824c33f0,// 112 PAY 109 

    0x7feded7d,// 113 PAY 110 

    0xe9d1c315,// 114 PAY 111 

    0x1584578f,// 115 PAY 112 

    0xa83da200,// 116 PAY 113 

    0x19639b39,// 117 PAY 114 

    0x9dc32f6f,// 118 PAY 115 

    0xc91f09e3,// 119 PAY 116 

    0xfa000000,// 120 PAY 117 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x03586eb2 // 121 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt62_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 298 words. 

/// BDA size     is 1185 (0x4a1) 

/// BDA id       is 0x8eca 

    0x04a18eca,// 3 BDA   1 

/// PAY Generic Data size   : 1185 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0393938d,// 4 PAY   1 

    0xa122f2db,// 5 PAY   2 

    0x49858b4c,// 6 PAY   3 

    0xb78151b0,// 7 PAY   4 

    0x6a2e4037,// 8 PAY   5 

    0x7b2ddae7,// 9 PAY   6 

    0x3279bae0,// 10 PAY   7 

    0xb80ec4ed,// 11 PAY   8 

    0xd3e8c5bb,// 12 PAY   9 

    0xa27ba7d5,// 13 PAY  10 

    0xcba6384b,// 14 PAY  11 

    0x4ba7153f,// 15 PAY  12 

    0xa4412498,// 16 PAY  13 

    0x145efdbe,// 17 PAY  14 

    0xc1f39d3f,// 18 PAY  15 

    0x6ff5b67a,// 19 PAY  16 

    0xac694a2d,// 20 PAY  17 

    0x6609a13e,// 21 PAY  18 

    0xd34b0fb8,// 22 PAY  19 

    0x402f0853,// 23 PAY  20 

    0x86d30eb3,// 24 PAY  21 

    0x9400e4d5,// 25 PAY  22 

    0x88349a3e,// 26 PAY  23 

    0xbbf499a1,// 27 PAY  24 

    0x5a8222c4,// 28 PAY  25 

    0xca03dfa0,// 29 PAY  26 

    0x1764fdbc,// 30 PAY  27 

    0xaea2ff9b,// 31 PAY  28 

    0x8cf789d6,// 32 PAY  29 

    0x7866df99,// 33 PAY  30 

    0xb99639c6,// 34 PAY  31 

    0x654882ad,// 35 PAY  32 

    0x58897f3c,// 36 PAY  33 

    0x2dacadd4,// 37 PAY  34 

    0x6373352d,// 38 PAY  35 

    0x2269ccf3,// 39 PAY  36 

    0x8ca73f64,// 40 PAY  37 

    0x3e4b9986,// 41 PAY  38 

    0xa2debe5b,// 42 PAY  39 

    0x1fac72ea,// 43 PAY  40 

    0xf21a5427,// 44 PAY  41 

    0x3de13454,// 45 PAY  42 

    0x2c000232,// 46 PAY  43 

    0x91041352,// 47 PAY  44 

    0x70b1457b,// 48 PAY  45 

    0xa5e6d12e,// 49 PAY  46 

    0x56aba714,// 50 PAY  47 

    0x9493528a,// 51 PAY  48 

    0x88d87c00,// 52 PAY  49 

    0x02677010,// 53 PAY  50 

    0x25b31ba1,// 54 PAY  51 

    0x06c0c5be,// 55 PAY  52 

    0x54bffb0f,// 56 PAY  53 

    0x62daeec5,// 57 PAY  54 

    0x1d9295f2,// 58 PAY  55 

    0x20c7f86a,// 59 PAY  56 

    0x678bd828,// 60 PAY  57 

    0x26709867,// 61 PAY  58 

    0x7e7d8ebd,// 62 PAY  59 

    0x5ba17a7e,// 63 PAY  60 

    0x0edf6aef,// 64 PAY  61 

    0x095704af,// 65 PAY  62 

    0x3c5baa5d,// 66 PAY  63 

    0xa86dff65,// 67 PAY  64 

    0xec70f1cf,// 68 PAY  65 

    0xb22228cf,// 69 PAY  66 

    0x6b16748f,// 70 PAY  67 

    0x26d538e6,// 71 PAY  68 

    0x0ab5eb05,// 72 PAY  69 

    0x4c779145,// 73 PAY  70 

    0x6cfb1128,// 74 PAY  71 

    0x53616ada,// 75 PAY  72 

    0x3b69f7fa,// 76 PAY  73 

    0x18722e71,// 77 PAY  74 

    0xa5bd16ec,// 78 PAY  75 

    0xc467261e,// 79 PAY  76 

    0x8627c949,// 80 PAY  77 

    0xd688be47,// 81 PAY  78 

    0x6ed12a5c,// 82 PAY  79 

    0xf10fdd94,// 83 PAY  80 

    0x097a4c16,// 84 PAY  81 

    0xbbd4d07a,// 85 PAY  82 

    0xf33542fe,// 86 PAY  83 

    0xf4324f61,// 87 PAY  84 

    0x333efb2e,// 88 PAY  85 

    0x5bf91726,// 89 PAY  86 

    0x29acacb1,// 90 PAY  87 

    0x50ac9fdb,// 91 PAY  88 

    0xbfb86510,// 92 PAY  89 

    0x89460451,// 93 PAY  90 

    0x281d5c5c,// 94 PAY  91 

    0x7c9f070b,// 95 PAY  92 

    0xbd929316,// 96 PAY  93 

    0xe1cf6232,// 97 PAY  94 

    0xd224ff01,// 98 PAY  95 

    0x27973b9f,// 99 PAY  96 

    0x9b7a09f7,// 100 PAY  97 

    0xc5ff4be6,// 101 PAY  98 

    0x5838bc54,// 102 PAY  99 

    0x610e3a89,// 103 PAY 100 

    0x94fcd911,// 104 PAY 101 

    0x26314d66,// 105 PAY 102 

    0xf81f90fc,// 106 PAY 103 

    0xc96deab1,// 107 PAY 104 

    0x4837eddc,// 108 PAY 105 

    0x968ad125,// 109 PAY 106 

    0x489ff91a,// 110 PAY 107 

    0xe835a642,// 111 PAY 108 

    0xcb8dea08,// 112 PAY 109 

    0x7b991d53,// 113 PAY 110 

    0xe65ea6b7,// 114 PAY 111 

    0x6cf7e9dc,// 115 PAY 112 

    0xb0e8fa30,// 116 PAY 113 

    0xa5f7dc25,// 117 PAY 114 

    0x75930657,// 118 PAY 115 

    0x85ec7315,// 119 PAY 116 

    0x9e237082,// 120 PAY 117 

    0x725d69b9,// 121 PAY 118 

    0xcc510b93,// 122 PAY 119 

    0x4823a89e,// 123 PAY 120 

    0xe504af1e,// 124 PAY 121 

    0xdc8255db,// 125 PAY 122 

    0x7e20f22e,// 126 PAY 123 

    0x8f9f0916,// 127 PAY 124 

    0xde8b0926,// 128 PAY 125 

    0x82363779,// 129 PAY 126 

    0xed5d04ac,// 130 PAY 127 

    0xd33d4298,// 131 PAY 128 

    0x28074314,// 132 PAY 129 

    0xf34e231d,// 133 PAY 130 

    0xa3c9124a,// 134 PAY 131 

    0x841e088b,// 135 PAY 132 

    0xcd5f68ed,// 136 PAY 133 

    0xd0412a76,// 137 PAY 134 

    0x3e96a588,// 138 PAY 135 

    0x2a7889f9,// 139 PAY 136 

    0xdbbb6467,// 140 PAY 137 

    0x8e972452,// 141 PAY 138 

    0xef9bcee0,// 142 PAY 139 

    0xf83f45a9,// 143 PAY 140 

    0x85c146fa,// 144 PAY 141 

    0xa75a16c5,// 145 PAY 142 

    0x95c2ef82,// 146 PAY 143 

    0xf087f5f5,// 147 PAY 144 

    0xe157408b,// 148 PAY 145 

    0xda2bc7c3,// 149 PAY 146 

    0x0120140f,// 150 PAY 147 

    0x6dd83ccb,// 151 PAY 148 

    0xe9ab0b6e,// 152 PAY 149 

    0x66390bb3,// 153 PAY 150 

    0x92d028a3,// 154 PAY 151 

    0xd9678d02,// 155 PAY 152 

    0x0a4e21bd,// 156 PAY 153 

    0x980e50b9,// 157 PAY 154 

    0xc8aac6b8,// 158 PAY 155 

    0x6d530bab,// 159 PAY 156 

    0x81c64421,// 160 PAY 157 

    0x297432a5,// 161 PAY 158 

    0xca8ef9ad,// 162 PAY 159 

    0x7df6d760,// 163 PAY 160 

    0xa31bd6ed,// 164 PAY 161 

    0x9c61eb1c,// 165 PAY 162 

    0x366484cf,// 166 PAY 163 

    0xf80914ef,// 167 PAY 164 

    0x869c06c1,// 168 PAY 165 

    0xaf514267,// 169 PAY 166 

    0x0c9a5b42,// 170 PAY 167 

    0x5759570b,// 171 PAY 168 

    0xd4eaecb1,// 172 PAY 169 

    0x1dcae8ad,// 173 PAY 170 

    0xde7d2a3c,// 174 PAY 171 

    0x6b13f064,// 175 PAY 172 

    0x3417819d,// 176 PAY 173 

    0x7b72c98e,// 177 PAY 174 

    0x59b58a33,// 178 PAY 175 

    0x403142aa,// 179 PAY 176 

    0xab97bfe1,// 180 PAY 177 

    0x369b8db2,// 181 PAY 178 

    0xa655198f,// 182 PAY 179 

    0x685c8c58,// 183 PAY 180 

    0xeb490189,// 184 PAY 181 

    0xa82ac5fe,// 185 PAY 182 

    0x74d87287,// 186 PAY 183 

    0x6546db81,// 187 PAY 184 

    0xfe036fec,// 188 PAY 185 

    0xf0a24d3b,// 189 PAY 186 

    0xb90850e4,// 190 PAY 187 

    0x4fe81d9e,// 191 PAY 188 

    0x7a5b536b,// 192 PAY 189 

    0x363a8381,// 193 PAY 190 

    0xd65839a8,// 194 PAY 191 

    0x026694d4,// 195 PAY 192 

    0xe58a1b14,// 196 PAY 193 

    0xa752181a,// 197 PAY 194 

    0x4eab428b,// 198 PAY 195 

    0x63e9c48c,// 199 PAY 196 

    0x22ee4c85,// 200 PAY 197 

    0x1375b2ea,// 201 PAY 198 

    0xbf0f73bf,// 202 PAY 199 

    0x6258308c,// 203 PAY 200 

    0xb52d2f4d,// 204 PAY 201 

    0x56ff49f4,// 205 PAY 202 

    0xdcbe7f8a,// 206 PAY 203 

    0x5bcb4091,// 207 PAY 204 

    0xe1b7b957,// 208 PAY 205 

    0xa57a7780,// 209 PAY 206 

    0x9306eb7d,// 210 PAY 207 

    0x7eb05bf0,// 211 PAY 208 

    0x2d3e9e1b,// 212 PAY 209 

    0x644cfd6c,// 213 PAY 210 

    0x3fbdd083,// 214 PAY 211 

    0x8df0449f,// 215 PAY 212 

    0xffd2b5d0,// 216 PAY 213 

    0xef822892,// 217 PAY 214 

    0x635d5dde,// 218 PAY 215 

    0xb903fba7,// 219 PAY 216 

    0xaa4c1afd,// 220 PAY 217 

    0x2336e9eb,// 221 PAY 218 

    0x2bb83bae,// 222 PAY 219 

    0xad477af4,// 223 PAY 220 

    0xcec5410c,// 224 PAY 221 

    0x561435a3,// 225 PAY 222 

    0x46e9d742,// 226 PAY 223 

    0xf3730c81,// 227 PAY 224 

    0x156901bd,// 228 PAY 225 

    0x9e0c5bbc,// 229 PAY 226 

    0x0b7c9d19,// 230 PAY 227 

    0xe878753c,// 231 PAY 228 

    0x8b9dcd94,// 232 PAY 229 

    0x6ad01ec8,// 233 PAY 230 

    0x174c248d,// 234 PAY 231 

    0x52ca88d7,// 235 PAY 232 

    0xbf79f1eb,// 236 PAY 233 

    0xaeb6ebfc,// 237 PAY 234 

    0x0d9bf2bb,// 238 PAY 235 

    0x24a716b7,// 239 PAY 236 

    0x9b361d57,// 240 PAY 237 

    0x14cb4013,// 241 PAY 238 

    0x12b3a6ef,// 242 PAY 239 

    0x5a54966c,// 243 PAY 240 

    0x46cb4275,// 244 PAY 241 

    0xabdc3761,// 245 PAY 242 

    0x5fbf01b4,// 246 PAY 243 

    0x53ce466e,// 247 PAY 244 

    0xf5c9c574,// 248 PAY 245 

    0x69bae7b3,// 249 PAY 246 

    0x039008ef,// 250 PAY 247 

    0x35ba432d,// 251 PAY 248 

    0xfd819bb0,// 252 PAY 249 

    0x14ff2da7,// 253 PAY 250 

    0xa6895b12,// 254 PAY 251 

    0x40ca2dc2,// 255 PAY 252 

    0x438c030d,// 256 PAY 253 

    0x0f308b2e,// 257 PAY 254 

    0x8490bbea,// 258 PAY 255 

    0xf0dc3363,// 259 PAY 256 

    0x47f7bccd,// 260 PAY 257 

    0x924fce11,// 261 PAY 258 

    0x34238755,// 262 PAY 259 

    0xe15928b8,// 263 PAY 260 

    0x6ef8308d,// 264 PAY 261 

    0x0bf526bd,// 265 PAY 262 

    0xc3e285a7,// 266 PAY 263 

    0xac41188a,// 267 PAY 264 

    0x62e01b2e,// 268 PAY 265 

    0x729289c2,// 269 PAY 266 

    0xe5dbc081,// 270 PAY 267 

    0x117162cd,// 271 PAY 268 

    0xbede803a,// 272 PAY 269 

    0x4070d5a2,// 273 PAY 270 

    0x52153264,// 274 PAY 271 

    0x7491777d,// 275 PAY 272 

    0x57ad4ebf,// 276 PAY 273 

    0xfefa8612,// 277 PAY 274 

    0x7a7db524,// 278 PAY 275 

    0xdc8d947f,// 279 PAY 276 

    0x684b3418,// 280 PAY 277 

    0x2c34d20c,// 281 PAY 278 

    0xfea711bb,// 282 PAY 279 

    0xe9e334a0,// 283 PAY 280 

    0x173da67d,// 284 PAY 281 

    0x402f5921,// 285 PAY 282 

    0x31437496,// 286 PAY 283 

    0xfb7c7133,// 287 PAY 284 

    0x3c57f3db,// 288 PAY 285 

    0x6ef87994,// 289 PAY 286 

    0x6e3d46b1,// 290 PAY 287 

    0x486c4d0e,// 291 PAY 288 

    0xc1567d2a,// 292 PAY 289 

    0x5e37409e,// 293 PAY 290 

    0x7bc7b458,// 294 PAY 291 

    0x27303adf,// 295 PAY 292 

    0x5f7b6bea,// 296 PAY 293 

    0x60e4b11b,// 297 PAY 294 

    0xdd61a2f8,// 298 PAY 295 

    0xd7b82cd9,// 299 PAY 296 

    0x26000000,// 300 PAY 297 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 187 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdd 

    0x02ecdd80 // 301 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt63_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 284 words. 

/// BDA size     is 1131 (0x46b) 

/// BDA id       is 0xad27 

    0x046bad27,// 3 BDA   1 

/// PAY Generic Data size   : 1131 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x16161d62,// 4 PAY   1 

    0x8ae89c91,// 5 PAY   2 

    0x0c1930f4,// 6 PAY   3 

    0x7cd00b64,// 7 PAY   4 

    0x6e7d4269,// 8 PAY   5 

    0x608c225b,// 9 PAY   6 

    0xcb249f5f,// 10 PAY   7 

    0x56bc050c,// 11 PAY   8 

    0xef1621cb,// 12 PAY   9 

    0x9c5f6974,// 13 PAY  10 

    0xcf75103e,// 14 PAY  11 

    0x060eecbf,// 15 PAY  12 

    0x7bafecce,// 16 PAY  13 

    0x9630351d,// 17 PAY  14 

    0xc89815a2,// 18 PAY  15 

    0x23db69fd,// 19 PAY  16 

    0xb14fa3d1,// 20 PAY  17 

    0x7d03bf9a,// 21 PAY  18 

    0xd1868cd9,// 22 PAY  19 

    0xd7f6e83e,// 23 PAY  20 

    0x7f4f5308,// 24 PAY  21 

    0x0ab9c49c,// 25 PAY  22 

    0x1ec65f1d,// 26 PAY  23 

    0xf81dd021,// 27 PAY  24 

    0x488b257a,// 28 PAY  25 

    0xa401f341,// 29 PAY  26 

    0x99edf7c7,// 30 PAY  27 

    0x6eda83ca,// 31 PAY  28 

    0xac202118,// 32 PAY  29 

    0x491ec6f8,// 33 PAY  30 

    0x2e88605c,// 34 PAY  31 

    0x61e361ec,// 35 PAY  32 

    0xe80a3e80,// 36 PAY  33 

    0xf8675794,// 37 PAY  34 

    0xe5e3ae60,// 38 PAY  35 

    0x3a18deeb,// 39 PAY  36 

    0xa8fa862d,// 40 PAY  37 

    0x7b7f9472,// 41 PAY  38 

    0xec5a51af,// 42 PAY  39 

    0xe5180a1b,// 43 PAY  40 

    0x5b201f85,// 44 PAY  41 

    0xf5eeb548,// 45 PAY  42 

    0x4fe397b8,// 46 PAY  43 

    0x6ace7dbc,// 47 PAY  44 

    0xe2efd1c7,// 48 PAY  45 

    0x6b74004e,// 49 PAY  46 

    0x855a9339,// 50 PAY  47 

    0x7d4a8d18,// 51 PAY  48 

    0x4bbaf085,// 52 PAY  49 

    0x9cea3f3e,// 53 PAY  50 

    0x51150ef2,// 54 PAY  51 

    0xd373a00d,// 55 PAY  52 

    0x7bff4954,// 56 PAY  53 

    0x4ae36086,// 57 PAY  54 

    0xca030017,// 58 PAY  55 

    0x6c62167e,// 59 PAY  56 

    0xbf5f97be,// 60 PAY  57 

    0x52ea7c10,// 61 PAY  58 

    0xbd77a4bf,// 62 PAY  59 

    0xc1d8c6c4,// 63 PAY  60 

    0x807c3bdf,// 64 PAY  61 

    0x7b1a1cac,// 65 PAY  62 

    0x43fe605f,// 66 PAY  63 

    0x7670b1a8,// 67 PAY  64 

    0xa86bf6b6,// 68 PAY  65 

    0x8d1a386d,// 69 PAY  66 

    0x6e49d2bc,// 70 PAY  67 

    0xb1354a7f,// 71 PAY  68 

    0x9b6594a0,// 72 PAY  69 

    0x61177e37,// 73 PAY  70 

    0xb40f24df,// 74 PAY  71 

    0x63d955d0,// 75 PAY  72 

    0x51c9883d,// 76 PAY  73 

    0xad6adc18,// 77 PAY  74 

    0xc520b9fe,// 78 PAY  75 

    0xf314e386,// 79 PAY  76 

    0xf8900691,// 80 PAY  77 

    0xe75157d6,// 81 PAY  78 

    0xcaa422db,// 82 PAY  79 

    0x623041ba,// 83 PAY  80 

    0x110d2a22,// 84 PAY  81 

    0x59718515,// 85 PAY  82 

    0x9a172f50,// 86 PAY  83 

    0x2822d049,// 87 PAY  84 

    0xf0e042d4,// 88 PAY  85 

    0xc1dca1ec,// 89 PAY  86 

    0x3e47ddb0,// 90 PAY  87 

    0xf50e98b5,// 91 PAY  88 

    0xf57ba019,// 92 PAY  89 

    0xf23c2700,// 93 PAY  90 

    0x35396c69,// 94 PAY  91 

    0x6b6f4dc8,// 95 PAY  92 

    0x6018e57c,// 96 PAY  93 

    0x6bfb2c71,// 97 PAY  94 

    0x63d5b947,// 98 PAY  95 

    0x080bb83a,// 99 PAY  96 

    0xde1b6cd5,// 100 PAY  97 

    0xdee17687,// 101 PAY  98 

    0x7a49b54f,// 102 PAY  99 

    0x041eaf6a,// 103 PAY 100 

    0x0f73a720,// 104 PAY 101 

    0x07f95b74,// 105 PAY 102 

    0xb53fe580,// 106 PAY 103 

    0x8b4f1469,// 107 PAY 104 

    0x4f4a6161,// 108 PAY 105 

    0x663c302c,// 109 PAY 106 

    0xbdc23501,// 110 PAY 107 

    0x10523389,// 111 PAY 108 

    0x3bb1ee28,// 112 PAY 109 

    0xa3388be9,// 113 PAY 110 

    0x2d86df46,// 114 PAY 111 

    0x43e97765,// 115 PAY 112 

    0xf51b9292,// 116 PAY 113 

    0x7671b076,// 117 PAY 114 

    0xbeea4447,// 118 PAY 115 

    0x9e9ad999,// 119 PAY 116 

    0x737a3315,// 120 PAY 117 

    0x9bf9efaf,// 121 PAY 118 

    0x529134de,// 122 PAY 119 

    0x94b1b338,// 123 PAY 120 

    0x3efad291,// 124 PAY 121 

    0xf228f256,// 125 PAY 122 

    0xa1435be9,// 126 PAY 123 

    0x232e50de,// 127 PAY 124 

    0xac8ecaf8,// 128 PAY 125 

    0x27687d1f,// 129 PAY 126 

    0x21bb8dcd,// 130 PAY 127 

    0x983e6afe,// 131 PAY 128 

    0xf4ac1741,// 132 PAY 129 

    0xb9d8d64c,// 133 PAY 130 

    0xef6e76cd,// 134 PAY 131 

    0x06adff46,// 135 PAY 132 

    0xbab03714,// 136 PAY 133 

    0x074f20b4,// 137 PAY 134 

    0x34762637,// 138 PAY 135 

    0x06aad04c,// 139 PAY 136 

    0x6985bc58,// 140 PAY 137 

    0x06c0bdda,// 141 PAY 138 

    0x8ca5d252,// 142 PAY 139 

    0x161835cd,// 143 PAY 140 

    0x74ff1755,// 144 PAY 141 

    0xea5168c6,// 145 PAY 142 

    0xf3596954,// 146 PAY 143 

    0x1561f610,// 147 PAY 144 

    0xb06d749f,// 148 PAY 145 

    0x10825ec6,// 149 PAY 146 

    0x21daf2f2,// 150 PAY 147 

    0xb463d3dd,// 151 PAY 148 

    0xd3cf758e,// 152 PAY 149 

    0x13ae7ef4,// 153 PAY 150 

    0xa8f2c7ce,// 154 PAY 151 

    0x3bbeb267,// 155 PAY 152 

    0xea9f7d3f,// 156 PAY 153 

    0xa33ea139,// 157 PAY 154 

    0x107259aa,// 158 PAY 155 

    0xebfe4b2f,// 159 PAY 156 

    0xd91ee673,// 160 PAY 157 

    0x2c44de99,// 161 PAY 158 

    0xe00bf3fa,// 162 PAY 159 

    0x32329fa9,// 163 PAY 160 

    0x48f66bc4,// 164 PAY 161 

    0x65e80bad,// 165 PAY 162 

    0x9e3eb366,// 166 PAY 163 

    0xcc25b4af,// 167 PAY 164 

    0x185a2496,// 168 PAY 165 

    0xeae6da10,// 169 PAY 166 

    0x3a0b987f,// 170 PAY 167 

    0x8dc2ac1a,// 171 PAY 168 

    0xd43c1790,// 172 PAY 169 

    0x817c0aa8,// 173 PAY 170 

    0x6681747d,// 174 PAY 171 

    0x0108a8d8,// 175 PAY 172 

    0xa3d2b37d,// 176 PAY 173 

    0x47892837,// 177 PAY 174 

    0x473be2a0,// 178 PAY 175 

    0x852a01a7,// 179 PAY 176 

    0x38896ba8,// 180 PAY 177 

    0x67afbeca,// 181 PAY 178 

    0xcfae2c30,// 182 PAY 179 

    0xa9c2d74d,// 183 PAY 180 

    0x325b98cb,// 184 PAY 181 

    0xe0f8d709,// 185 PAY 182 

    0x43a88a55,// 186 PAY 183 

    0xfb9a1f59,// 187 PAY 184 

    0xfaa7c3c5,// 188 PAY 185 

    0x3fd219a6,// 189 PAY 186 

    0x791e2aef,// 190 PAY 187 

    0x067189c4,// 191 PAY 188 

    0xd29d1f08,// 192 PAY 189 

    0xfeeff204,// 193 PAY 190 

    0x26db8099,// 194 PAY 191 

    0x44aa384c,// 195 PAY 192 

    0x6c418548,// 196 PAY 193 

    0x23617ea6,// 197 PAY 194 

    0x40398f42,// 198 PAY 195 

    0x14bfa32f,// 199 PAY 196 

    0x71b2b16a,// 200 PAY 197 

    0x74b71ac2,// 201 PAY 198 

    0xd955e4c4,// 202 PAY 199 

    0x1b15bf20,// 203 PAY 200 

    0x631567b7,// 204 PAY 201 

    0x5a303711,// 205 PAY 202 

    0x4d0b09e6,// 206 PAY 203 

    0x08616f4d,// 207 PAY 204 

    0x2ada9cfb,// 208 PAY 205 

    0x360bde22,// 209 PAY 206 

    0x76d732f2,// 210 PAY 207 

    0x5ec72a8f,// 211 PAY 208 

    0xab0a19e7,// 212 PAY 209 

    0x9e5031cb,// 213 PAY 210 

    0x0b16b697,// 214 PAY 211 

    0x11e04af3,// 215 PAY 212 

    0xa057b6b1,// 216 PAY 213 

    0x7cf3d21c,// 217 PAY 214 

    0x04e71e51,// 218 PAY 215 

    0x6faf2219,// 219 PAY 216 

    0xeab06001,// 220 PAY 217 

    0xcec16d10,// 221 PAY 218 

    0x50feb8a2,// 222 PAY 219 

    0xa69d5fe5,// 223 PAY 220 

    0x5713003d,// 224 PAY 221 

    0x56a7da96,// 225 PAY 222 

    0x6167ff0e,// 226 PAY 223 

    0x4989c040,// 227 PAY 224 

    0x7baf1648,// 228 PAY 225 

    0x25915840,// 229 PAY 226 

    0xbc30f032,// 230 PAY 227 

    0xb01d15da,// 231 PAY 228 

    0xbe655c61,// 232 PAY 229 

    0x6ea150c3,// 233 PAY 230 

    0xe4b39025,// 234 PAY 231 

    0x2e807941,// 235 PAY 232 

    0x60d96bf2,// 236 PAY 233 

    0xc91a1872,// 237 PAY 234 

    0x9ba21227,// 238 PAY 235 

    0xb364007c,// 239 PAY 236 

    0x099a43db,// 240 PAY 237 

    0xc3ea619b,// 241 PAY 238 

    0xe68d7bc6,// 242 PAY 239 

    0xef945b97,// 243 PAY 240 

    0x2f4d8c65,// 244 PAY 241 

    0x23eaa281,// 245 PAY 242 

    0xfa72321f,// 246 PAY 243 

    0x6dae2b28,// 247 PAY 244 

    0xccb3c0c5,// 248 PAY 245 

    0x92fb3c1a,// 249 PAY 246 

    0xd0f552d1,// 250 PAY 247 

    0x33434983,// 251 PAY 248 

    0x09210cab,// 252 PAY 249 

    0xc9fa4c76,// 253 PAY 250 

    0x56b04235,// 254 PAY 251 

    0x0f10132c,// 255 PAY 252 

    0xaf906089,// 256 PAY 253 

    0xab1cdb67,// 257 PAY 254 

    0xb5dfbd2d,// 258 PAY 255 

    0x1b166a11,// 259 PAY 256 

    0xedbfa3f5,// 260 PAY 257 

    0x2d40a38b,// 261 PAY 258 

    0xb408c0e1,// 262 PAY 259 

    0xc195da8d,// 263 PAY 260 

    0x2011416c,// 264 PAY 261 

    0x7e81d4c3,// 265 PAY 262 

    0x066940c2,// 266 PAY 263 

    0x5965ff1a,// 267 PAY 264 

    0x25278b70,// 268 PAY 265 

    0x40208c33,// 269 PAY 266 

    0x6cf3c61f,// 270 PAY 267 

    0x25b05c15,// 271 PAY 268 

    0xc68a0996,// 272 PAY 269 

    0x822a57ea,// 273 PAY 270 

    0x29482860,// 274 PAY 271 

    0xa232b6e0,// 275 PAY 272 

    0xf3f549e2,// 276 PAY 273 

    0xbaea951c,// 277 PAY 274 

    0x23a6e1c0,// 278 PAY 275 

    0xe492a5cf,// 279 PAY 276 

    0x89a224c4,// 280 PAY 277 

    0x98f04fe3,// 281 PAY 278 

    0x1d1a5069,// 282 PAY 279 

    0xba5297a8,// 283 PAY 280 

    0x292dcdd0,// 284 PAY 281 

    0xdaf192ba,// 285 PAY 282 

    0x8a362e00,// 286 PAY 283 

/// HASH is  8 bytes 

    0xb408c0e1,// 287 HSH   1 

    0xc195da8d,// 288 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 218 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x25 

    0x03682553 // 289 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt64_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 284 words. 

/// BDA size     is 1130 (0x46a) 

/// BDA id       is 0x770a 

    0x046a770a,// 3 BDA   1 

/// PAY Generic Data size   : 1130 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xc40f0b6c,// 4 PAY   1 

    0xa3006216,// 5 PAY   2 

    0xec6a01b6,// 6 PAY   3 

    0x6905e27b,// 7 PAY   4 

    0x489d4415,// 8 PAY   5 

    0xf40ef0f7,// 9 PAY   6 

    0xf5dc2aab,// 10 PAY   7 

    0xabdfc28d,// 11 PAY   8 

    0xb44aaa78,// 12 PAY   9 

    0xf4cfc990,// 13 PAY  10 

    0x3f5363de,// 14 PAY  11 

    0x60ded868,// 15 PAY  12 

    0x30b8673a,// 16 PAY  13 

    0xc1f065fe,// 17 PAY  14 

    0x1466ef50,// 18 PAY  15 

    0x7cde037e,// 19 PAY  16 

    0x83dff4ac,// 20 PAY  17 

    0xb6d28305,// 21 PAY  18 

    0x99d75018,// 22 PAY  19 

    0xe1a997a1,// 23 PAY  20 

    0xdd5384c4,// 24 PAY  21 

    0xdb0a5dd3,// 25 PAY  22 

    0xdf98275b,// 26 PAY  23 

    0x0ed1cfac,// 27 PAY  24 

    0x36b815a3,// 28 PAY  25 

    0x95ac8aa5,// 29 PAY  26 

    0xb0fc60c3,// 30 PAY  27 

    0x0b0cc68f,// 31 PAY  28 

    0x2fd843e0,// 32 PAY  29 

    0x3dc47883,// 33 PAY  30 

    0x230139a4,// 34 PAY  31 

    0xd719d812,// 35 PAY  32 

    0xd95a9d20,// 36 PAY  33 

    0x74bfba43,// 37 PAY  34 

    0xae9ee675,// 38 PAY  35 

    0x0ef066af,// 39 PAY  36 

    0x2ae39fa7,// 40 PAY  37 

    0x06d85db2,// 41 PAY  38 

    0x70227f74,// 42 PAY  39 

    0x48eb4a0e,// 43 PAY  40 

    0xbf2ff54b,// 44 PAY  41 

    0x7c809465,// 45 PAY  42 

    0xa21a6b15,// 46 PAY  43 

    0xd3654e99,// 47 PAY  44 

    0x0211802e,// 48 PAY  45 

    0xe4f095df,// 49 PAY  46 

    0x2fef4e9f,// 50 PAY  47 

    0xcf0407f8,// 51 PAY  48 

    0x1c60f613,// 52 PAY  49 

    0x618706eb,// 53 PAY  50 

    0xd99b8f1c,// 54 PAY  51 

    0xca8ba323,// 55 PAY  52 

    0xa4add889,// 56 PAY  53 

    0x7cca7c72,// 57 PAY  54 

    0x98199cb3,// 58 PAY  55 

    0x2377a12f,// 59 PAY  56 

    0x48806707,// 60 PAY  57 

    0x9341aa0f,// 61 PAY  58 

    0xf4218e43,// 62 PAY  59 

    0x4d57f442,// 63 PAY  60 

    0xfefe8483,// 64 PAY  61 

    0x9bacc134,// 65 PAY  62 

    0xab628f94,// 66 PAY  63 

    0x11986c82,// 67 PAY  64 

    0x3e8e4d15,// 68 PAY  65 

    0x56d949d1,// 69 PAY  66 

    0x5bc2e8dc,// 70 PAY  67 

    0x4957f75e,// 71 PAY  68 

    0xc52f45b4,// 72 PAY  69 

    0xb54a1f2c,// 73 PAY  70 

    0x0748c899,// 74 PAY  71 

    0xc0c1a0dc,// 75 PAY  72 

    0xeb45bb27,// 76 PAY  73 

    0x33aeaa98,// 77 PAY  74 

    0xad3ee78b,// 78 PAY  75 

    0xfbb958f7,// 79 PAY  76 

    0xb871d5b0,// 80 PAY  77 

    0x57002305,// 81 PAY  78 

    0x506f329a,// 82 PAY  79 

    0x0ea1aace,// 83 PAY  80 

    0x071eafad,// 84 PAY  81 

    0xf710bb8c,// 85 PAY  82 

    0xd3c69fae,// 86 PAY  83 

    0xf8ba7d3a,// 87 PAY  84 

    0xf1d56cf7,// 88 PAY  85 

    0x8598bf37,// 89 PAY  86 

    0xd65432cb,// 90 PAY  87 

    0xf0bcd0d2,// 91 PAY  88 

    0x8e5fe562,// 92 PAY  89 

    0x8c0e7a23,// 93 PAY  90 

    0x92ccab53,// 94 PAY  91 

    0xfba7e22f,// 95 PAY  92 

    0x853e42b3,// 96 PAY  93 

    0x337629c8,// 97 PAY  94 

    0xa32c7b9c,// 98 PAY  95 

    0x35fe3f2d,// 99 PAY  96 

    0x75de84f7,// 100 PAY  97 

    0xb1a72434,// 101 PAY  98 

    0xb92d386f,// 102 PAY  99 

    0xfc1464e2,// 103 PAY 100 

    0xc9b65cd2,// 104 PAY 101 

    0x63f67ffe,// 105 PAY 102 

    0xd5d062f1,// 106 PAY 103 

    0xe1407637,// 107 PAY 104 

    0xd40c425d,// 108 PAY 105 

    0x1c4b4eb5,// 109 PAY 106 

    0xead67b1a,// 110 PAY 107 

    0xef894876,// 111 PAY 108 

    0xb21cd9b7,// 112 PAY 109 

    0xa866cf4f,// 113 PAY 110 

    0x865e492a,// 114 PAY 111 

    0x76f43217,// 115 PAY 112 

    0x95a166c7,// 116 PAY 113 

    0xbf2d0857,// 117 PAY 114 

    0x322dbde8,// 118 PAY 115 

    0xa6ed40fc,// 119 PAY 116 

    0x8d80ceba,// 120 PAY 117 

    0xe854054c,// 121 PAY 118 

    0x065006ef,// 122 PAY 119 

    0x41e2c8da,// 123 PAY 120 

    0x322e29a1,// 124 PAY 121 

    0x5e6d4023,// 125 PAY 122 

    0x0ec30138,// 126 PAY 123 

    0xc0ecc060,// 127 PAY 124 

    0x397b81d6,// 128 PAY 125 

    0x5dbb8d86,// 129 PAY 126 

    0xa8307f8d,// 130 PAY 127 

    0x2e43d5f2,// 131 PAY 128 

    0xe948ff56,// 132 PAY 129 

    0x3d25cfbc,// 133 PAY 130 

    0x2866ce4e,// 134 PAY 131 

    0x2c6c548a,// 135 PAY 132 

    0x5ed2f1f7,// 136 PAY 133 

    0x385b5ca4,// 137 PAY 134 

    0x87390876,// 138 PAY 135 

    0x3fe3a84e,// 139 PAY 136 

    0x428e9a0a,// 140 PAY 137 

    0x0d4a5a70,// 141 PAY 138 

    0x19a37de9,// 142 PAY 139 

    0x454a42ef,// 143 PAY 140 

    0x0fb731bc,// 144 PAY 141 

    0x1e62f615,// 145 PAY 142 

    0x93cb1566,// 146 PAY 143 

    0x244d479f,// 147 PAY 144 

    0x71de816a,// 148 PAY 145 

    0x81b06390,// 149 PAY 146 

    0xf71a120f,// 150 PAY 147 

    0x9b6b639e,// 151 PAY 148 

    0xff99173c,// 152 PAY 149 

    0x8327dee4,// 153 PAY 150 

    0xdc128d5f,// 154 PAY 151 

    0xbe09214f,// 155 PAY 152 

    0x510848ed,// 156 PAY 153 

    0x18ee20bf,// 157 PAY 154 

    0x1ec2bab0,// 158 PAY 155 

    0x95d27e9c,// 159 PAY 156 

    0x5d4df88d,// 160 PAY 157 

    0xc541c6b6,// 161 PAY 158 

    0x9c60811a,// 162 PAY 159 

    0x36fa9437,// 163 PAY 160 

    0xf7fce42f,// 164 PAY 161 

    0xe0cfeef2,// 165 PAY 162 

    0x352f3776,// 166 PAY 163 

    0x363170a0,// 167 PAY 164 

    0x661cad59,// 168 PAY 165 

    0x335573e4,// 169 PAY 166 

    0x07081d0d,// 170 PAY 167 

    0x77aa9b17,// 171 PAY 168 

    0xeade5aac,// 172 PAY 169 

    0x998893b0,// 173 PAY 170 

    0x73bdc1bf,// 174 PAY 171 

    0x0b7915d3,// 175 PAY 172 

    0xa9fbd295,// 176 PAY 173 

    0x8b266b4e,// 177 PAY 174 

    0xa19f8a5b,// 178 PAY 175 

    0x12fb3b38,// 179 PAY 176 

    0x2ea45ce2,// 180 PAY 177 

    0xefd7b3ae,// 181 PAY 178 

    0x393e88f0,// 182 PAY 179 

    0xd611d55c,// 183 PAY 180 

    0x58117c49,// 184 PAY 181 

    0x76c2e291,// 185 PAY 182 

    0x7ebf4a04,// 186 PAY 183 

    0x81f5e824,// 187 PAY 184 

    0xfa4c03d3,// 188 PAY 185 

    0x6d5b5c1a,// 189 PAY 186 

    0xbe0bb936,// 190 PAY 187 

    0x617332de,// 191 PAY 188 

    0x1a5bf546,// 192 PAY 189 

    0xc540c3af,// 193 PAY 190 

    0xa352547c,// 194 PAY 191 

    0x7b7e599f,// 195 PAY 192 

    0xd0e49f9e,// 196 PAY 193 

    0x6691aa75,// 197 PAY 194 

    0xff3c0f08,// 198 PAY 195 

    0x8ef11cce,// 199 PAY 196 

    0xc89de928,// 200 PAY 197 

    0x5556f9f5,// 201 PAY 198 

    0x2a6697aa,// 202 PAY 199 

    0xa92d91a4,// 203 PAY 200 

    0xf0bd919f,// 204 PAY 201 

    0xb46dda6a,// 205 PAY 202 

    0x3613ca0e,// 206 PAY 203 

    0x84d81a01,// 207 PAY 204 

    0xee3ad55e,// 208 PAY 205 

    0x59337953,// 209 PAY 206 

    0x382346e2,// 210 PAY 207 

    0x19112783,// 211 PAY 208 

    0xe9c57b04,// 212 PAY 209 

    0xfe0c603f,// 213 PAY 210 

    0x849b0bcf,// 214 PAY 211 

    0xc5fbde3c,// 215 PAY 212 

    0x550eb7b4,// 216 PAY 213 

    0x5e0e8faf,// 217 PAY 214 

    0x7b02df0e,// 218 PAY 215 

    0x491496a7,// 219 PAY 216 

    0xf0da445d,// 220 PAY 217 

    0xf7ba2066,// 221 PAY 218 

    0xe145a31d,// 222 PAY 219 

    0x02db19e0,// 223 PAY 220 

    0x7c316baa,// 224 PAY 221 

    0xbe14d815,// 225 PAY 222 

    0x61e5f84a,// 226 PAY 223 

    0x0deabcf7,// 227 PAY 224 

    0x2c5545df,// 228 PAY 225 

    0xf6fd771c,// 229 PAY 226 

    0x5ec99562,// 230 PAY 227 

    0x73b178f8,// 231 PAY 228 

    0xaf448ec5,// 232 PAY 229 

    0x6bf01503,// 233 PAY 230 

    0xf02c5462,// 234 PAY 231 

    0x19733cca,// 235 PAY 232 

    0xc2e84d71,// 236 PAY 233 

    0x4e015165,// 237 PAY 234 

    0xae608a64,// 238 PAY 235 

    0x046b6bdc,// 239 PAY 236 

    0x03eca4a0,// 240 PAY 237 

    0xbc01920e,// 241 PAY 238 

    0x51e696e8,// 242 PAY 239 

    0x07bcac02,// 243 PAY 240 

    0x313e4714,// 244 PAY 241 

    0x15a5a4f0,// 245 PAY 242 

    0x825991ce,// 246 PAY 243 

    0x52be061e,// 247 PAY 244 

    0x2d41272b,// 248 PAY 245 

    0xcd57b6a4,// 249 PAY 246 

    0x97094558,// 250 PAY 247 

    0xb9d6a156,// 251 PAY 248 

    0x60909b34,// 252 PAY 249 

    0xfac89954,// 253 PAY 250 

    0x9a807624,// 254 PAY 251 

    0x53041191,// 255 PAY 252 

    0xdf004a14,// 256 PAY 253 

    0x5f4b5fd6,// 257 PAY 254 

    0xfa3868dd,// 258 PAY 255 

    0xf707879c,// 259 PAY 256 

    0xdbce37f5,// 260 PAY 257 

    0xf890eb30,// 261 PAY 258 

    0x8e350bcd,// 262 PAY 259 

    0x970d0b9c,// 263 PAY 260 

    0xee16131e,// 264 PAY 261 

    0x159b5d66,// 265 PAY 262 

    0x963dd9ad,// 266 PAY 263 

    0x5c4c73af,// 267 PAY 264 

    0x8864f8ab,// 268 PAY 265 

    0xb1b3b2c7,// 269 PAY 266 

    0xc6c887f8,// 270 PAY 267 

    0x30a2c6ce,// 271 PAY 268 

    0x3c3c7bf6,// 272 PAY 269 

    0x22b60f10,// 273 PAY 270 

    0xe603f780,// 274 PAY 271 

    0x51863e3b,// 275 PAY 272 

    0xe8d19e1f,// 276 PAY 273 

    0x4a923399,// 277 PAY 274 

    0x8e141f08,// 278 PAY 275 

    0x97e10788,// 279 PAY 276 

    0x21e5aebd,// 280 PAY 277 

    0xc1bc9ae3,// 281 PAY 278 

    0x78f2c5b2,// 282 PAY 279 

    0x92aad564,// 283 PAY 280 

    0x1f41c1b4,// 284 PAY 281 

    0xa822231f,// 285 PAY 282 

    0x3f020000,// 286 PAY 283 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 131 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbb 

    0x020cbb33 // 287 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt65_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 65 words. 

/// BDA size     is 253 (0xfd) 

/// BDA id       is 0x5fa4 

    0x00fd5fa4,// 3 BDA   1 

/// PAY Generic Data size   : 253 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0d6abc56,// 4 PAY   1 

    0xfdd0431f,// 5 PAY   2 

    0xac8bb714,// 6 PAY   3 

    0x410302c2,// 7 PAY   4 

    0xa94fbed9,// 8 PAY   5 

    0xb37e64b8,// 9 PAY   6 

    0x7fa2ace2,// 10 PAY   7 

    0x909a9c51,// 11 PAY   8 

    0x005c422b,// 12 PAY   9 

    0x685b881e,// 13 PAY  10 

    0x1fc2619e,// 14 PAY  11 

    0x8f0a7702,// 15 PAY  12 

    0x396858fc,// 16 PAY  13 

    0xfbafd162,// 17 PAY  14 

    0xd68ec344,// 18 PAY  15 

    0x52bcce85,// 19 PAY  16 

    0xd877c1b7,// 20 PAY  17 

    0xcd0e9a4f,// 21 PAY  18 

    0x2128c3d7,// 22 PAY  19 

    0xf0df3b9c,// 23 PAY  20 

    0xfe5c3700,// 24 PAY  21 

    0x2962901b,// 25 PAY  22 

    0x66a05f35,// 26 PAY  23 

    0x8a7b5bc3,// 27 PAY  24 

    0x7bb1d192,// 28 PAY  25 

    0xa7f2ae48,// 29 PAY  26 

    0x1a23455c,// 30 PAY  27 

    0xce497072,// 31 PAY  28 

    0x8087c049,// 32 PAY  29 

    0x6f37bbeb,// 33 PAY  30 

    0xf6a79c74,// 34 PAY  31 

    0x3b018932,// 35 PAY  32 

    0x03638db1,// 36 PAY  33 

    0x09173fb2,// 37 PAY  34 

    0xde252965,// 38 PAY  35 

    0x38589d32,// 39 PAY  36 

    0x792dd826,// 40 PAY  37 

    0x1becdd88,// 41 PAY  38 

    0xb63b91ee,// 42 PAY  39 

    0x07d1f3bd,// 43 PAY  40 

    0x82769ac3,// 44 PAY  41 

    0xc36afdef,// 45 PAY  42 

    0x3c2efc44,// 46 PAY  43 

    0xda342a8f,// 47 PAY  44 

    0x39640b05,// 48 PAY  45 

    0x4a6833da,// 49 PAY  46 

    0x9555704c,// 50 PAY  47 

    0x161473e8,// 51 PAY  48 

    0x04cc4634,// 52 PAY  49 

    0xf7b9dc31,// 53 PAY  50 

    0x6b7868e5,// 54 PAY  51 

    0x9f8e1943,// 55 PAY  52 

    0xb944408c,// 56 PAY  53 

    0x5ace372a,// 57 PAY  54 

    0x2c4270a9,// 58 PAY  55 

    0xe340136f,// 59 PAY  56 

    0xf22d9acc,// 60 PAY  57 

    0xc8b33e6a,// 61 PAY  58 

    0x46d2eff1,// 62 PAY  59 

    0x7478a4c7,// 63 PAY  60 

    0x4f346bfe,// 64 PAY  61 

    0xb43dd430,// 65 PAY  62 

    0xbddf253a,// 66 PAY  63 

    0x4a000000,// 67 PAY  64 

/// HASH is  16 bytes 

    0x46d2eff1,// 68 HSH   1 

    0x7478a4c7,// 69 HSH   2 

    0x4f346bfe,// 70 HSH   3 

    0xb43dd430,// 71 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x45 

    0x00d44533 // 72 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt66_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 211 words. 

/// BDA size     is 838 (0x346) 

/// BDA id       is 0xb717 

    0x0346b717,// 3 BDA   1 

/// PAY Generic Data size   : 838 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe2bfd939,// 4 PAY   1 

    0x7f6d04d6,// 5 PAY   2 

    0xedee156d,// 6 PAY   3 

    0xb3c6b3d0,// 7 PAY   4 

    0x68f05865,// 8 PAY   5 

    0x30107d5c,// 9 PAY   6 

    0xa5350bed,// 10 PAY   7 

    0x4c9712c1,// 11 PAY   8 

    0x1df19339,// 12 PAY   9 

    0x2643d339,// 13 PAY  10 

    0x1c2874cd,// 14 PAY  11 

    0xae06fb1d,// 15 PAY  12 

    0x5e658dd6,// 16 PAY  13 

    0x6312f186,// 17 PAY  14 

    0x91e1e83b,// 18 PAY  15 

    0x96ac41b9,// 19 PAY  16 

    0x03a63422,// 20 PAY  17 

    0x9f9fc9ff,// 21 PAY  18 

    0x1b6ee412,// 22 PAY  19 

    0xc3d8ac86,// 23 PAY  20 

    0xfd136921,// 24 PAY  21 

    0x687f7865,// 25 PAY  22 

    0xe0d6bd6e,// 26 PAY  23 

    0x2a500ccb,// 27 PAY  24 

    0x23a2f5eb,// 28 PAY  25 

    0x389ce96c,// 29 PAY  26 

    0x9f36db1e,// 30 PAY  27 

    0x76301189,// 31 PAY  28 

    0xa23c6bcc,// 32 PAY  29 

    0x788da093,// 33 PAY  30 

    0x11d320df,// 34 PAY  31 

    0xdc1948ee,// 35 PAY  32 

    0x69d4e8c9,// 36 PAY  33 

    0x36ba918c,// 37 PAY  34 

    0xf6d83f23,// 38 PAY  35 

    0x6f720bbb,// 39 PAY  36 

    0x66c8f086,// 40 PAY  37 

    0x8640254c,// 41 PAY  38 

    0x85f60ed8,// 42 PAY  39 

    0xe58dafba,// 43 PAY  40 

    0xea056a0b,// 44 PAY  41 

    0x9ddb03fe,// 45 PAY  42 

    0xac8fc113,// 46 PAY  43 

    0xd4b5966d,// 47 PAY  44 

    0x3abdfdc0,// 48 PAY  45 

    0xfface19e,// 49 PAY  46 

    0xa05069d9,// 50 PAY  47 

    0xbbd05995,// 51 PAY  48 

    0xdb126eff,// 52 PAY  49 

    0x2d2f97c2,// 53 PAY  50 

    0x39752a1e,// 54 PAY  51 

    0xe3b1896c,// 55 PAY  52 

    0x5495fbd1,// 56 PAY  53 

    0xd389f6d4,// 57 PAY  54 

    0x935ab83e,// 58 PAY  55 

    0xec86f485,// 59 PAY  56 

    0x74f0fe39,// 60 PAY  57 

    0xad9c6c51,// 61 PAY  58 

    0x4b5960b4,// 62 PAY  59 

    0x2ec15ddc,// 63 PAY  60 

    0x6893d097,// 64 PAY  61 

    0xcc8c0a02,// 65 PAY  62 

    0xf899a209,// 66 PAY  63 

    0x1095f6f4,// 67 PAY  64 

    0x9a613c7f,// 68 PAY  65 

    0xcfd24368,// 69 PAY  66 

    0xf1d1b611,// 70 PAY  67 

    0x2f9dfcb0,// 71 PAY  68 

    0x27bc8519,// 72 PAY  69 

    0xbcf2f3ab,// 73 PAY  70 

    0x07fc908e,// 74 PAY  71 

    0xdfa472cf,// 75 PAY  72 

    0x5ceee498,// 76 PAY  73 

    0x59bfb8f8,// 77 PAY  74 

    0x65c0362c,// 78 PAY  75 

    0x5ec08c0e,// 79 PAY  76 

    0x908f738b,// 80 PAY  77 

    0x53fcdd7e,// 81 PAY  78 

    0xe88a3687,// 82 PAY  79 

    0x9b28de31,// 83 PAY  80 

    0xda1dbc9e,// 84 PAY  81 

    0xf86a88ba,// 85 PAY  82 

    0x7dff3929,// 86 PAY  83 

    0xa3f2b4a4,// 87 PAY  84 

    0xa439968a,// 88 PAY  85 

    0x66667c77,// 89 PAY  86 

    0x4ebce425,// 90 PAY  87 

    0xcf19ee2e,// 91 PAY  88 

    0x719d40b3,// 92 PAY  89 

    0x22285f68,// 93 PAY  90 

    0xa7a368a4,// 94 PAY  91 

    0x1171a4e6,// 95 PAY  92 

    0x64b1f4bc,// 96 PAY  93 

    0xb722a9bc,// 97 PAY  94 

    0xbaccc936,// 98 PAY  95 

    0x08a737aa,// 99 PAY  96 

    0x1d0562fb,// 100 PAY  97 

    0xbcb851cd,// 101 PAY  98 

    0x9394989f,// 102 PAY  99 

    0x55c1d0c7,// 103 PAY 100 

    0x2a841147,// 104 PAY 101 

    0x88a1faa5,// 105 PAY 102 

    0x675d812d,// 106 PAY 103 

    0x80ba78fb,// 107 PAY 104 

    0xb03e938c,// 108 PAY 105 

    0x2c62aa13,// 109 PAY 106 

    0x34da850d,// 110 PAY 107 

    0x93ab4880,// 111 PAY 108 

    0x2273e3e9,// 112 PAY 109 

    0x0e19d898,// 113 PAY 110 

    0xcacf94c5,// 114 PAY 111 

    0x5c9c7d7d,// 115 PAY 112 

    0x7b999830,// 116 PAY 113 

    0x7ffca893,// 117 PAY 114 

    0xcd575dc9,// 118 PAY 115 

    0xd6cd7264,// 119 PAY 116 

    0x5d0d1911,// 120 PAY 117 

    0x24f5ca1c,// 121 PAY 118 

    0xe3f996f2,// 122 PAY 119 

    0xd03370df,// 123 PAY 120 

    0x6df113b6,// 124 PAY 121 

    0x3eee4ad4,// 125 PAY 122 

    0xb88ac7ad,// 126 PAY 123 

    0x0750fff3,// 127 PAY 124 

    0x473cde91,// 128 PAY 125 

    0xecac10a7,// 129 PAY 126 

    0x02d73f3f,// 130 PAY 127 

    0x549b66fc,// 131 PAY 128 

    0x60a5da83,// 132 PAY 129 

    0xb958cdaa,// 133 PAY 130 

    0xb51b17cf,// 134 PAY 131 

    0xba6d6067,// 135 PAY 132 

    0xabbb2821,// 136 PAY 133 

    0x36f0187f,// 137 PAY 134 

    0x3f4b8db4,// 138 PAY 135 

    0xcb5e88c1,// 139 PAY 136 

    0xecfe6c3e,// 140 PAY 137 

    0xccaf3efe,// 141 PAY 138 

    0xf819d1a1,// 142 PAY 139 

    0xe18c8517,// 143 PAY 140 

    0x3ee4cd2c,// 144 PAY 141 

    0x8cebb746,// 145 PAY 142 

    0xe9bc2e7c,// 146 PAY 143 

    0xa9f4c7c1,// 147 PAY 144 

    0x235310d7,// 148 PAY 145 

    0x5cc299c9,// 149 PAY 146 

    0x109c7608,// 150 PAY 147 

    0x77862d88,// 151 PAY 148 

    0x17f3c50a,// 152 PAY 149 

    0x50ac2845,// 153 PAY 150 

    0xb45bb8c2,// 154 PAY 151 

    0x383bdfee,// 155 PAY 152 

    0x3f677a75,// 156 PAY 153 

    0xab78a8c9,// 157 PAY 154 

    0xb58de395,// 158 PAY 155 

    0x88a6a458,// 159 PAY 156 

    0x47ce3135,// 160 PAY 157 

    0x8a5662bf,// 161 PAY 158 

    0x6a5dff60,// 162 PAY 159 

    0xbf5e6c1d,// 163 PAY 160 

    0x730976b9,// 164 PAY 161 

    0xcb725b7d,// 165 PAY 162 

    0xa19edf44,// 166 PAY 163 

    0x073a183e,// 167 PAY 164 

    0xcecb4098,// 168 PAY 165 

    0xa9cdcd31,// 169 PAY 166 

    0x3711eb60,// 170 PAY 167 

    0x29f154bd,// 171 PAY 168 

    0x00f23392,// 172 PAY 169 

    0xee22c08c,// 173 PAY 170 

    0x3ccdc632,// 174 PAY 171 

    0x0f04bd86,// 175 PAY 172 

    0x8ab750ec,// 176 PAY 173 

    0xdb4b6086,// 177 PAY 174 

    0xbecc5e4f,// 178 PAY 175 

    0x73b2e9e0,// 179 PAY 176 

    0xab1755aa,// 180 PAY 177 

    0x9d0ac6ea,// 181 PAY 178 

    0x84444cf2,// 182 PAY 179 

    0x97a38179,// 183 PAY 180 

    0x81b8cd8e,// 184 PAY 181 

    0x4f719efe,// 185 PAY 182 

    0x6ca58cfe,// 186 PAY 183 

    0x9b04fd16,// 187 PAY 184 

    0xfeae28e0,// 188 PAY 185 

    0x6280d1b8,// 189 PAY 186 

    0xdc7f2e37,// 190 PAY 187 

    0x48e2850d,// 191 PAY 188 

    0x5814dd0e,// 192 PAY 189 

    0x3bedd783,// 193 PAY 190 

    0x7613200c,// 194 PAY 191 

    0x6c8fae6e,// 195 PAY 192 

    0x7e8cf7c6,// 196 PAY 193 

    0xd2c2b791,// 197 PAY 194 

    0x7b3def94,// 198 PAY 195 

    0xf5642924,// 199 PAY 196 

    0x2669308a,// 200 PAY 197 

    0x20dbf546,// 201 PAY 198 

    0x8a110606,// 202 PAY 199 

    0x98bd98a5,// 203 PAY 200 

    0x0432856b,// 204 PAY 201 

    0x2a825123,// 205 PAY 202 

    0x37631cd1,// 206 PAY 203 

    0x15a3829d,// 207 PAY 204 

    0x5f7210f4,// 208 PAY 205 

    0xeabbcd4b,// 209 PAY 206 

    0x7f921ec8,// 210 PAY 207 

    0x53ef599a,// 211 PAY 208 

    0x2ed8d32f,// 212 PAY 209 

    0xc75c0000,// 213 PAY 210 

/// HASH is  8 bytes 

    0x84444cf2,// 214 HSH   1 

    0x97a38179,// 215 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 91 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x83 

    0x01e8835b // 216 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt67_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 107 words. 

/// BDA size     is 422 (0x1a6) 

/// BDA id       is 0xa522 

    0x01a6a522,// 3 BDA   1 

/// PAY Generic Data size   : 422 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x7332dc0d,// 4 PAY   1 

    0xe57f2558,// 5 PAY   2 

    0x9b45e940,// 6 PAY   3 

    0x8766c538,// 7 PAY   4 

    0xb8f56439,// 8 PAY   5 

    0x68e3074d,// 9 PAY   6 

    0xd7febda0,// 10 PAY   7 

    0xd6059516,// 11 PAY   8 

    0x53e1acfe,// 12 PAY   9 

    0x42815040,// 13 PAY  10 

    0x95f8cc2b,// 14 PAY  11 

    0xf28e807a,// 15 PAY  12 

    0xd5d01f9a,// 16 PAY  13 

    0xc667c86d,// 17 PAY  14 

    0xce25546c,// 18 PAY  15 

    0xe476a4a0,// 19 PAY  16 

    0xe82fab0e,// 20 PAY  17 

    0xe756b183,// 21 PAY  18 

    0x5b2fce58,// 22 PAY  19 

    0xdc28f51a,// 23 PAY  20 

    0x04855f2c,// 24 PAY  21 

    0xfa9fe52a,// 25 PAY  22 

    0xcb638bba,// 26 PAY  23 

    0xb1dba47f,// 27 PAY  24 

    0x62e4d8e4,// 28 PAY  25 

    0x51be7a5d,// 29 PAY  26 

    0xc3cc21d0,// 30 PAY  27 

    0x10032bcc,// 31 PAY  28 

    0x46a2cdd1,// 32 PAY  29 

    0xe62e698c,// 33 PAY  30 

    0xe63fe12c,// 34 PAY  31 

    0x69169781,// 35 PAY  32 

    0x5667338d,// 36 PAY  33 

    0x115d5e39,// 37 PAY  34 

    0xe127f013,// 38 PAY  35 

    0x71654c92,// 39 PAY  36 

    0x7e3a6229,// 40 PAY  37 

    0x763b23e6,// 41 PAY  38 

    0x729bfc1b,// 42 PAY  39 

    0xaf540ec8,// 43 PAY  40 

    0x82e604dd,// 44 PAY  41 

    0x45e98a00,// 45 PAY  42 

    0xb89cb467,// 46 PAY  43 

    0xc55f97cc,// 47 PAY  44 

    0x827b6cb4,// 48 PAY  45 

    0x9af59420,// 49 PAY  46 

    0xddff4836,// 50 PAY  47 

    0x73d70292,// 51 PAY  48 

    0xca917ffe,// 52 PAY  49 

    0x22320da3,// 53 PAY  50 

    0xe6af44fd,// 54 PAY  51 

    0xd90ffe80,// 55 PAY  52 

    0xeda8f877,// 56 PAY  53 

    0x4d669b58,// 57 PAY  54 

    0xd42fe813,// 58 PAY  55 

    0xeaf99292,// 59 PAY  56 

    0x5096aa45,// 60 PAY  57 

    0x3f900d26,// 61 PAY  58 

    0xfa072831,// 62 PAY  59 

    0xace57fea,// 63 PAY  60 

    0xe9211956,// 64 PAY  61 

    0xd12f8b4e,// 65 PAY  62 

    0x5dc674a7,// 66 PAY  63 

    0xbb57ba36,// 67 PAY  64 

    0x4712441b,// 68 PAY  65 

    0xfbdbecb1,// 69 PAY  66 

    0x57d828f3,// 70 PAY  67 

    0xca5a6030,// 71 PAY  68 

    0x57fd3350,// 72 PAY  69 

    0x180e66bb,// 73 PAY  70 

    0x53d57c24,// 74 PAY  71 

    0xae5e809e,// 75 PAY  72 

    0x3794e119,// 76 PAY  73 

    0xfa65d03f,// 77 PAY  74 

    0xef41fde2,// 78 PAY  75 

    0x17afb227,// 79 PAY  76 

    0xefb5702d,// 80 PAY  77 

    0xe9fe2533,// 81 PAY  78 

    0xaa55dc6e,// 82 PAY  79 

    0x2666b9a5,// 83 PAY  80 

    0x911ab36e,// 84 PAY  81 

    0x2685d578,// 85 PAY  82 

    0x1efaed92,// 86 PAY  83 

    0x64db0dd6,// 87 PAY  84 

    0x15e4de48,// 88 PAY  85 

    0x7623bb2a,// 89 PAY  86 

    0x1fd7a29f,// 90 PAY  87 

    0x902ab616,// 91 PAY  88 

    0x66ac4095,// 92 PAY  89 

    0x8204a5f1,// 93 PAY  90 

    0x5d0151c7,// 94 PAY  91 

    0xa2cf4451,// 95 PAY  92 

    0xa5e481c2,// 96 PAY  93 

    0xc4a620b6,// 97 PAY  94 

    0x6d68b6fd,// 98 PAY  95 

    0x4ab9c5b1,// 99 PAY  96 

    0xdf317f59,// 100 PAY  97 

    0x7e6f958e,// 101 PAY  98 

    0xd55b8283,// 102 PAY  99 

    0x1f2b7150,// 103 PAY 100 

    0x07d7acc4,// 104 PAY 101 

    0xac7b5a10,// 105 PAY 102 

    0x2c1a2f31,// 106 PAY 103 

    0x7eca476e,// 107 PAY 104 

    0xbec1c4cd,// 108 PAY 105 

    0xeae40000,// 109 PAY 106 

/// HASH is  4 bytes 

    0x1f2b7150,// 110 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 221 

/// STA pkt_idx        : 35 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd4 

    0x008cd4dd // 111 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt68_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 317 words. 

/// BDA size     is 1261 (0x4ed) 

/// BDA id       is 0x7985 

    0x04ed7985,// 3 BDA   1 

/// PAY Generic Data size   : 1261 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xe9379e5b,// 4 PAY   1 

    0xa5744c2f,// 5 PAY   2 

    0x9e866062,// 6 PAY   3 

    0x5abb80f7,// 7 PAY   4 

    0xc95a1666,// 8 PAY   5 

    0x8984a06a,// 9 PAY   6 

    0x0df22017,// 10 PAY   7 

    0x4f2da423,// 11 PAY   8 

    0x0b3b7be8,// 12 PAY   9 

    0x75f6d42e,// 13 PAY  10 

    0x6f0cb94d,// 14 PAY  11 

    0xf9b04fff,// 15 PAY  12 

    0x09c357e5,// 16 PAY  13 

    0x42a357c6,// 17 PAY  14 

    0xd69e9cd1,// 18 PAY  15 

    0x10b58799,// 19 PAY  16 

    0x39fa8472,// 20 PAY  17 

    0xd8be9594,// 21 PAY  18 

    0x09c2311e,// 22 PAY  19 

    0xb5932950,// 23 PAY  20 

    0xebc1b23f,// 24 PAY  21 

    0x67c72715,// 25 PAY  22 

    0xebf30681,// 26 PAY  23 

    0x1a19440d,// 27 PAY  24 

    0xdffac3d0,// 28 PAY  25 

    0xb8c3fad9,// 29 PAY  26 

    0x203f3341,// 30 PAY  27 

    0x26c8abac,// 31 PAY  28 

    0xbc17b327,// 32 PAY  29 

    0x5bacc12b,// 33 PAY  30 

    0xb5138e72,// 34 PAY  31 

    0x0b2ebbb3,// 35 PAY  32 

    0x9d2a08e3,// 36 PAY  33 

    0x7889a8ac,// 37 PAY  34 

    0x178152cb,// 38 PAY  35 

    0xa8a454aa,// 39 PAY  36 

    0x6e06ea3b,// 40 PAY  37 

    0x9957bb6e,// 41 PAY  38 

    0x54741b19,// 42 PAY  39 

    0x668985d6,// 43 PAY  40 

    0xc8001791,// 44 PAY  41 

    0x794403c4,// 45 PAY  42 

    0x725b1e39,// 46 PAY  43 

    0x4c3b0322,// 47 PAY  44 

    0xdaa953b8,// 48 PAY  45 

    0x981fad1e,// 49 PAY  46 

    0xecb36528,// 50 PAY  47 

    0xf0f3e034,// 51 PAY  48 

    0x13652866,// 52 PAY  49 

    0xc1b33731,// 53 PAY  50 

    0xd6adb163,// 54 PAY  51 

    0x75eca1ba,// 55 PAY  52 

    0xc0a31fe9,// 56 PAY  53 

    0x5a25d85a,// 57 PAY  54 

    0xe149c162,// 58 PAY  55 

    0x6ebe48bf,// 59 PAY  56 

    0xc2a4c04d,// 60 PAY  57 

    0xe886f340,// 61 PAY  58 

    0x3fc7641d,// 62 PAY  59 

    0xf0dc0282,// 63 PAY  60 

    0x0d411be4,// 64 PAY  61 

    0xa43fa97b,// 65 PAY  62 

    0xe0d1eec0,// 66 PAY  63 

    0xf296717a,// 67 PAY  64 

    0x65f2f24c,// 68 PAY  65 

    0xf8c26950,// 69 PAY  66 

    0x04ae87db,// 70 PAY  67 

    0x047f75e3,// 71 PAY  68 

    0x9416cb0c,// 72 PAY  69 

    0xe1de851b,// 73 PAY  70 

    0xc2733fa8,// 74 PAY  71 

    0xb33c7819,// 75 PAY  72 

    0x186a7b16,// 76 PAY  73 

    0x0ad993cd,// 77 PAY  74 

    0x7e5a5310,// 78 PAY  75 

    0x860e25eb,// 79 PAY  76 

    0x784f108b,// 80 PAY  77 

    0x9ce717d8,// 81 PAY  78 

    0x2fb8bf22,// 82 PAY  79 

    0x7f3794d9,// 83 PAY  80 

    0xb0a02aec,// 84 PAY  81 

    0x98b5a107,// 85 PAY  82 

    0x17eede20,// 86 PAY  83 

    0x39556c13,// 87 PAY  84 

    0x6cf73a9c,// 88 PAY  85 

    0x1cb33009,// 89 PAY  86 

    0xb36b2337,// 90 PAY  87 

    0x633550f1,// 91 PAY  88 

    0xd2cdbf6b,// 92 PAY  89 

    0xb62cd6f1,// 93 PAY  90 

    0xb52bca94,// 94 PAY  91 

    0xc2fa7277,// 95 PAY  92 

    0x7d685118,// 96 PAY  93 

    0xfdc77db7,// 97 PAY  94 

    0x0d697842,// 98 PAY  95 

    0x666066f0,// 99 PAY  96 

    0x2b166ff9,// 100 PAY  97 

    0x792596ba,// 101 PAY  98 

    0xc941b66b,// 102 PAY  99 

    0x59586393,// 103 PAY 100 

    0x52c8b417,// 104 PAY 101 

    0x8ae6d2c1,// 105 PAY 102 

    0x44aa6d88,// 106 PAY 103 

    0x2a3745e0,// 107 PAY 104 

    0x0ab5f4e1,// 108 PAY 105 

    0x300040f1,// 109 PAY 106 

    0xe5a1e34f,// 110 PAY 107 

    0x3964a54b,// 111 PAY 108 

    0x282b630a,// 112 PAY 109 

    0xd39ddc97,// 113 PAY 110 

    0xcea0b2af,// 114 PAY 111 

    0xf4aec525,// 115 PAY 112 

    0x8ff12767,// 116 PAY 113 

    0x30ced89f,// 117 PAY 114 

    0x9648745e,// 118 PAY 115 

    0x240f0d56,// 119 PAY 116 

    0x92f50597,// 120 PAY 117 

    0x6ba2d1a5,// 121 PAY 118 

    0x973739ff,// 122 PAY 119 

    0xcad24cba,// 123 PAY 120 

    0xc6aac81a,// 124 PAY 121 

    0xfa651065,// 125 PAY 122 

    0x824008f2,// 126 PAY 123 

    0x5af4ba8a,// 127 PAY 124 

    0xf86ca7de,// 128 PAY 125 

    0xbfe2b849,// 129 PAY 126 

    0xebcba6c3,// 130 PAY 127 

    0x3f1c530c,// 131 PAY 128 

    0xe6d7b8ed,// 132 PAY 129 

    0x44820cdf,// 133 PAY 130 

    0x082419ba,// 134 PAY 131 

    0x71b08d7e,// 135 PAY 132 

    0xafc2478a,// 136 PAY 133 

    0x60ec8482,// 137 PAY 134 

    0xe3947f28,// 138 PAY 135 

    0x0933f155,// 139 PAY 136 

    0x9f3f0a7a,// 140 PAY 137 

    0x660f5226,// 141 PAY 138 

    0x6cd26d2b,// 142 PAY 139 

    0x3ff158a8,// 143 PAY 140 

    0xe134f01d,// 144 PAY 141 

    0x51fd036b,// 145 PAY 142 

    0x803364d1,// 146 PAY 143 

    0xa0381744,// 147 PAY 144 

    0x96594988,// 148 PAY 145 

    0x0cead9c0,// 149 PAY 146 

    0x9f8f7033,// 150 PAY 147 

    0xa871c9bc,// 151 PAY 148 

    0x5655fc46,// 152 PAY 149 

    0x55cdbf54,// 153 PAY 150 

    0x5b2193fb,// 154 PAY 151 

    0xda924f83,// 155 PAY 152 

    0xff5fe806,// 156 PAY 153 

    0x010ce3b8,// 157 PAY 154 

    0x2f4e0d90,// 158 PAY 155 

    0x3050b54c,// 159 PAY 156 

    0xe5b7414e,// 160 PAY 157 

    0x75a39ec6,// 161 PAY 158 

    0xccfd1028,// 162 PAY 159 

    0xe324fdd8,// 163 PAY 160 

    0xbe925b09,// 164 PAY 161 

    0xb3356859,// 165 PAY 162 

    0x2981b477,// 166 PAY 163 

    0x712a3aa4,// 167 PAY 164 

    0x07394cfc,// 168 PAY 165 

    0x91aff1b3,// 169 PAY 166 

    0x3f4888cb,// 170 PAY 167 

    0x56403f25,// 171 PAY 168 

    0x9e92171e,// 172 PAY 169 

    0xefa85b9b,// 173 PAY 170 

    0x326fe355,// 174 PAY 171 

    0x6ceb6e20,// 175 PAY 172 

    0x778cb3e7,// 176 PAY 173 

    0xd36b9772,// 177 PAY 174 

    0x602891c6,// 178 PAY 175 

    0xc207658d,// 179 PAY 176 

    0x3d212ead,// 180 PAY 177 

    0xa7d619f2,// 181 PAY 178 

    0x2616e394,// 182 PAY 179 

    0xbfe7ad24,// 183 PAY 180 

    0x8e6d0bac,// 184 PAY 181 

    0x5d00317d,// 185 PAY 182 

    0x6f48a1f7,// 186 PAY 183 

    0xe2ce266b,// 187 PAY 184 

    0x4ff4a92b,// 188 PAY 185 

    0x1cfd7fd4,// 189 PAY 186 

    0xcb2cba90,// 190 PAY 187 

    0x4251e762,// 191 PAY 188 

    0x6db075a9,// 192 PAY 189 

    0xd6bbc8af,// 193 PAY 190 

    0x177e284b,// 194 PAY 191 

    0x75d3d8bb,// 195 PAY 192 

    0x899938d6,// 196 PAY 193 

    0x1f374566,// 197 PAY 194 

    0xcd5d78ef,// 198 PAY 195 

    0x1ad7c31a,// 199 PAY 196 

    0xefcafd26,// 200 PAY 197 

    0x4c8b7e68,// 201 PAY 198 

    0xa2c38a6a,// 202 PAY 199 

    0xa9a9ba39,// 203 PAY 200 

    0x964d0e83,// 204 PAY 201 

    0x7281077b,// 205 PAY 202 

    0x33a7509b,// 206 PAY 203 

    0xc187ca2a,// 207 PAY 204 

    0x531e7512,// 208 PAY 205 

    0x9510a407,// 209 PAY 206 

    0x626a7b3b,// 210 PAY 207 

    0xa9069a01,// 211 PAY 208 

    0xf0f01534,// 212 PAY 209 

    0x19841239,// 213 PAY 210 

    0x55af6c9e,// 214 PAY 211 

    0x0b1c1f67,// 215 PAY 212 

    0xd3af9e49,// 216 PAY 213 

    0x52e5f12f,// 217 PAY 214 

    0xf3f3c25c,// 218 PAY 215 

    0xf315d661,// 219 PAY 216 

    0xd1f168eb,// 220 PAY 217 

    0x48e3c554,// 221 PAY 218 

    0x774c698d,// 222 PAY 219 

    0x5b576c5f,// 223 PAY 220 

    0x93f0c9a2,// 224 PAY 221 

    0x59ee121c,// 225 PAY 222 

    0xbafaf42f,// 226 PAY 223 

    0xc13a386f,// 227 PAY 224 

    0xcbf81839,// 228 PAY 225 

    0x1202e478,// 229 PAY 226 

    0xe52ed787,// 230 PAY 227 

    0x5cd180cf,// 231 PAY 228 

    0xe7f4ff41,// 232 PAY 229 

    0x4afb77e7,// 233 PAY 230 

    0xfa6c750a,// 234 PAY 231 

    0x85e86d16,// 235 PAY 232 

    0xc3075d99,// 236 PAY 233 

    0xcfe904bd,// 237 PAY 234 

    0x3b09a799,// 238 PAY 235 

    0x74365f0b,// 239 PAY 236 

    0x53cd3f46,// 240 PAY 237 

    0x171e7ea4,// 241 PAY 238 

    0x7bb99730,// 242 PAY 239 

    0x51d3bd5c,// 243 PAY 240 

    0x06709dc4,// 244 PAY 241 

    0x6f817624,// 245 PAY 242 

    0x92b0c597,// 246 PAY 243 

    0x23028c89,// 247 PAY 244 

    0x1ff4511d,// 248 PAY 245 

    0xccd13f00,// 249 PAY 246 

    0xeb2dd85c,// 250 PAY 247 

    0xe548e0b5,// 251 PAY 248 

    0x7fc4701f,// 252 PAY 249 

    0x5e45c00e,// 253 PAY 250 

    0x3bd5b440,// 254 PAY 251 

    0xf8c8c604,// 255 PAY 252 

    0x0b1128d0,// 256 PAY 253 

    0x8ceee55c,// 257 PAY 254 

    0xcb141d69,// 258 PAY 255 

    0x18058c48,// 259 PAY 256 

    0x9f9869e1,// 260 PAY 257 

    0x5f911163,// 261 PAY 258 

    0xd8780c8b,// 262 PAY 259 

    0x7fe659e6,// 263 PAY 260 

    0x14c8c60a,// 264 PAY 261 

    0xf6b29964,// 265 PAY 262 

    0x9e241d9c,// 266 PAY 263 

    0x9e9a56df,// 267 PAY 264 

    0xe47dc8ca,// 268 PAY 265 

    0xb1e5bfbe,// 269 PAY 266 

    0x376eca92,// 270 PAY 267 

    0xf3d1c871,// 271 PAY 268 

    0xf97929dd,// 272 PAY 269 

    0xdfee3c6d,// 273 PAY 270 

    0x4e0042dc,// 274 PAY 271 

    0x0c1f719a,// 275 PAY 272 

    0x679176bf,// 276 PAY 273 

    0x30fa37f5,// 277 PAY 274 

    0xfcdd8402,// 278 PAY 275 

    0xdf1408da,// 279 PAY 276 

    0xfbc7b1a3,// 280 PAY 277 

    0xe9aa9077,// 281 PAY 278 

    0xa96aa20e,// 282 PAY 279 

    0xa079a07d,// 283 PAY 280 

    0xeaa16f0e,// 284 PAY 281 

    0x1128946e,// 285 PAY 282 

    0x5f1e4f1c,// 286 PAY 283 

    0x7bb2a5a2,// 287 PAY 284 

    0x2080b388,// 288 PAY 285 

    0xec76033f,// 289 PAY 286 

    0x7ce6eff4,// 290 PAY 287 

    0xfca6f19a,// 291 PAY 288 

    0x435fc981,// 292 PAY 289 

    0x1a611e3c,// 293 PAY 290 

    0x73849c25,// 294 PAY 291 

    0xe8e5e7d0,// 295 PAY 292 

    0xf490c220,// 296 PAY 293 

    0x4c4745b9,// 297 PAY 294 

    0x24c61166,// 298 PAY 295 

    0xc800af45,// 299 PAY 296 

    0x6be0d618,// 300 PAY 297 

    0xb37f4457,// 301 PAY 298 

    0xd5e50f15,// 302 PAY 299 

    0xaec8922d,// 303 PAY 300 

    0x71afac5d,// 304 PAY 301 

    0x985ef063,// 305 PAY 302 

    0x7f569580,// 306 PAY 303 

    0xf89b913e,// 307 PAY 304 

    0xe1533ac1,// 308 PAY 305 

    0x3354933d,// 309 PAY 306 

    0xae243423,// 310 PAY 307 

    0xa8156c4e,// 311 PAY 308 

    0xc444f21a,// 312 PAY 309 

    0x1cf5bff3,// 313 PAY 310 

    0xce2b4a3b,// 314 PAY 311 

    0xee34abb3,// 315 PAY 312 

    0x20fa5b17,// 316 PAY 313 

    0x96ae18b6,// 317 PAY 314 

    0x98f1c547,// 318 PAY 315 

    0x6b000000,// 319 PAY 316 

/// HASH is  16 bytes 

    0x3354933d,// 320 HSH   1 

    0xae243423,// 321 HSH   2 

    0xa8156c4e,// 322 HSH   3 

    0xc444f21a,// 323 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 198 

/// STA pkt_idx        : 217 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9d 

    0x03659dc6 // 324 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt69_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 182 words. 

/// BDA size     is 723 (0x2d3) 

/// BDA id       is 0xe834 

    0x02d3e834,// 3 BDA   1 

/// PAY Generic Data size   : 723 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xe814653f,// 4 PAY   1 

    0xcdb3dc58,// 5 PAY   2 

    0xb79ac96c,// 6 PAY   3 

    0xe3629787,// 7 PAY   4 

    0xa7f66568,// 8 PAY   5 

    0xf0a83ed9,// 9 PAY   6 

    0x1445a929,// 10 PAY   7 

    0x5c8e768f,// 11 PAY   8 

    0x10497a47,// 12 PAY   9 

    0x3efe485a,// 13 PAY  10 

    0xfca0e50b,// 14 PAY  11 

    0xbd835a00,// 15 PAY  12 

    0xee4f0e86,// 16 PAY  13 

    0x4f99366b,// 17 PAY  14 

    0xdf4026a2,// 18 PAY  15 

    0x68c21b38,// 19 PAY  16 

    0x3a09d5ba,// 20 PAY  17 

    0x9e620913,// 21 PAY  18 

    0xb3bf05df,// 22 PAY  19 

    0xe275c8f7,// 23 PAY  20 

    0xea983d13,// 24 PAY  21 

    0x946e32a3,// 25 PAY  22 

    0x715d08ba,// 26 PAY  23 

    0x9a12e528,// 27 PAY  24 

    0xe39f881c,// 28 PAY  25 

    0x2aca0664,// 29 PAY  26 

    0xf81297bf,// 30 PAY  27 

    0x68b62d3e,// 31 PAY  28 

    0xb3b7db68,// 32 PAY  29 

    0x2776622e,// 33 PAY  30 

    0x6119861d,// 34 PAY  31 

    0xee8c2476,// 35 PAY  32 

    0xa4e92437,// 36 PAY  33 

    0x949d8699,// 37 PAY  34 

    0x6ce64f0a,// 38 PAY  35 

    0x2d0a4eb5,// 39 PAY  36 

    0xf6092333,// 40 PAY  37 

    0xb02f009c,// 41 PAY  38 

    0xe838460f,// 42 PAY  39 

    0x8b0879f9,// 43 PAY  40 

    0x703d68f0,// 44 PAY  41 

    0xdc0d3b02,// 45 PAY  42 

    0xa2558219,// 46 PAY  43 

    0x232dd04c,// 47 PAY  44 

    0x03c7047f,// 48 PAY  45 

    0xc25f4b98,// 49 PAY  46 

    0x583b1920,// 50 PAY  47 

    0xc326497a,// 51 PAY  48 

    0x2e368695,// 52 PAY  49 

    0x5a6b5401,// 53 PAY  50 

    0x6eeb395c,// 54 PAY  51 

    0x3882c30f,// 55 PAY  52 

    0xdfd2375f,// 56 PAY  53 

    0xfcf48292,// 57 PAY  54 

    0xe3f22d28,// 58 PAY  55 

    0x798b344c,// 59 PAY  56 

    0xbe4fd3c6,// 60 PAY  57 

    0x5ad17019,// 61 PAY  58 

    0x016fa806,// 62 PAY  59 

    0x43cff59b,// 63 PAY  60 

    0x85868754,// 64 PAY  61 

    0x80dec839,// 65 PAY  62 

    0x2996a268,// 66 PAY  63 

    0xd1decffb,// 67 PAY  64 

    0x293f0383,// 68 PAY  65 

    0xb8b5cd0f,// 69 PAY  66 

    0x788a4687,// 70 PAY  67 

    0x9c4670c4,// 71 PAY  68 

    0xd8943020,// 72 PAY  69 

    0xce6b6d49,// 73 PAY  70 

    0xff2e22d3,// 74 PAY  71 

    0x3768f079,// 75 PAY  72 

    0xe8346a86,// 76 PAY  73 

    0x2af72519,// 77 PAY  74 

    0xfa9f0403,// 78 PAY  75 

    0xfd0fba53,// 79 PAY  76 

    0x5edc84fb,// 80 PAY  77 

    0x404dd6e9,// 81 PAY  78 

    0x4104dc38,// 82 PAY  79 

    0x7a2a7f1d,// 83 PAY  80 

    0x26977258,// 84 PAY  81 

    0x93c1462b,// 85 PAY  82 

    0x0eceab12,// 86 PAY  83 

    0xd90f694a,// 87 PAY  84 

    0x31bbfdb9,// 88 PAY  85 

    0x2830ea99,// 89 PAY  86 

    0x238efaaa,// 90 PAY  87 

    0xb101fd90,// 91 PAY  88 

    0x757ca7d5,// 92 PAY  89 

    0x1527f9ee,// 93 PAY  90 

    0xf21c03a2,// 94 PAY  91 

    0xc37aae03,// 95 PAY  92 

    0x06eb2564,// 96 PAY  93 

    0xcb7003ae,// 97 PAY  94 

    0x1cfa6ef3,// 98 PAY  95 

    0x2654d61e,// 99 PAY  96 

    0x10068c4a,// 100 PAY  97 

    0x023a247e,// 101 PAY  98 

    0x40377e48,// 102 PAY  99 

    0x0c8a5e6c,// 103 PAY 100 

    0x6db03e75,// 104 PAY 101 

    0x8296c74b,// 105 PAY 102 

    0x4dd1e957,// 106 PAY 103 

    0xa5fbd8e4,// 107 PAY 104 

    0xa9fc2ae8,// 108 PAY 105 

    0xdd0bc079,// 109 PAY 106 

    0xda2472bb,// 110 PAY 107 

    0xfc8a82ed,// 111 PAY 108 

    0x06957b7a,// 112 PAY 109 

    0x8687f1c1,// 113 PAY 110 

    0xbaf9740e,// 114 PAY 111 

    0xae09f019,// 115 PAY 112 

    0xe21c4489,// 116 PAY 113 

    0xd3921dc9,// 117 PAY 114 

    0xb59555b0,// 118 PAY 115 

    0xac3dfda8,// 119 PAY 116 

    0x0d5f1ddc,// 120 PAY 117 

    0xdd4f8847,// 121 PAY 118 

    0x152e10f9,// 122 PAY 119 

    0x571b395c,// 123 PAY 120 

    0xc47a3f87,// 124 PAY 121 

    0xcdb7ed51,// 125 PAY 122 

    0x8d44197a,// 126 PAY 123 

    0x5f1183e6,// 127 PAY 124 

    0xd6583ab4,// 128 PAY 125 

    0x7ce69cbe,// 129 PAY 126 

    0x720138bc,// 130 PAY 127 

    0xef265c98,// 131 PAY 128 

    0x99ee63c8,// 132 PAY 129 

    0xcd3407d6,// 133 PAY 130 

    0xd45418b4,// 134 PAY 131 

    0x2967f053,// 135 PAY 132 

    0x46e90b4d,// 136 PAY 133 

    0x42c753e0,// 137 PAY 134 

    0x1eb9810d,// 138 PAY 135 

    0xdb581fb2,// 139 PAY 136 

    0x5112af13,// 140 PAY 137 

    0x50bd20f4,// 141 PAY 138 

    0xa1f2e880,// 142 PAY 139 

    0xe737506e,// 143 PAY 140 

    0xe5289709,// 144 PAY 141 

    0xf76bd1fa,// 145 PAY 142 

    0xcfec7309,// 146 PAY 143 

    0x6481e5c7,// 147 PAY 144 

    0x39ed5332,// 148 PAY 145 

    0xfa596287,// 149 PAY 146 

    0x682892f1,// 150 PAY 147 

    0xf3037963,// 151 PAY 148 

    0x7ae3e57a,// 152 PAY 149 

    0xefe3f2e9,// 153 PAY 150 

    0x0bfaf489,// 154 PAY 151 

    0x20d1f226,// 155 PAY 152 

    0x1e8e06ac,// 156 PAY 153 

    0x90fbf338,// 157 PAY 154 

    0x9ca42b49,// 158 PAY 155 

    0x3141e275,// 159 PAY 156 

    0x378f9f44,// 160 PAY 157 

    0xb688d204,// 161 PAY 158 

    0x62cbe52e,// 162 PAY 159 

    0xb83ea27e,// 163 PAY 160 

    0x106d800f,// 164 PAY 161 

    0xe20cdb41,// 165 PAY 162 

    0x89b6fe45,// 166 PAY 163 

    0x2be96f01,// 167 PAY 164 

    0xa4753cea,// 168 PAY 165 

    0x356f7233,// 169 PAY 166 

    0xcafa0ff0,// 170 PAY 167 

    0xb9c68f8e,// 171 PAY 168 

    0x60ab2471,// 172 PAY 169 

    0x4ba7dc06,// 173 PAY 170 

    0x97fe8380,// 174 PAY 171 

    0xbf92ba35,// 175 PAY 172 

    0x8085a4dd,// 176 PAY 173 

    0x7a1cc29a,// 177 PAY 174 

    0x9da70131,// 178 PAY 175 

    0x20859f18,// 179 PAY 176 

    0xaadf8f92,// 180 PAY 177 

    0x440681ab,// 181 PAY 178 

    0x79afca0e,// 182 PAY 179 

    0xfd892c35,// 183 PAY 180 

    0x68a97f00,// 184 PAY 181 

/// STA is 1 words. 

/// STA num_pkts       : 24 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xed 

    0x03dded18 // 185 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt70_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 320 words. 

/// BDA size     is 1275 (0x4fb) 

/// BDA id       is 0x8e57 

    0x04fb8e57,// 3 BDA   1 

/// PAY Generic Data size   : 1275 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xa731afbd,// 4 PAY   1 

    0x38656022,// 5 PAY   2 

    0x0cf67191,// 6 PAY   3 

    0xead8f177,// 7 PAY   4 

    0x8e435470,// 8 PAY   5 

    0x4fff8886,// 9 PAY   6 

    0xb4c68706,// 10 PAY   7 

    0x2bb01678,// 11 PAY   8 

    0x7071ee1f,// 12 PAY   9 

    0xefb9311f,// 13 PAY  10 

    0xde05699b,// 14 PAY  11 

    0xda3c7ebb,// 15 PAY  12 

    0x60aed44c,// 16 PAY  13 

    0x3cadb8a9,// 17 PAY  14 

    0xea69fd67,// 18 PAY  15 

    0xa068a89e,// 19 PAY  16 

    0x632525c0,// 20 PAY  17 

    0xaba87026,// 21 PAY  18 

    0xefd2b87c,// 22 PAY  19 

    0x8ef0adc4,// 23 PAY  20 

    0x1a9890b1,// 24 PAY  21 

    0xd040cfd1,// 25 PAY  22 

    0x56d92059,// 26 PAY  23 

    0x82228553,// 27 PAY  24 

    0xf289b70d,// 28 PAY  25 

    0x46d724fa,// 29 PAY  26 

    0xe6fdc91c,// 30 PAY  27 

    0x07f9d702,// 31 PAY  28 

    0x1beb154e,// 32 PAY  29 

    0x5022d167,// 33 PAY  30 

    0xa9c7a732,// 34 PAY  31 

    0x207492d5,// 35 PAY  32 

    0xd6a2aa9e,// 36 PAY  33 

    0x64fdc0b6,// 37 PAY  34 

    0x4a48e466,// 38 PAY  35 

    0x719a7568,// 39 PAY  36 

    0x733f2267,// 40 PAY  37 

    0x0696463d,// 41 PAY  38 

    0x9c6646e8,// 42 PAY  39 

    0x24accfaf,// 43 PAY  40 

    0x32cd5272,// 44 PAY  41 

    0x8a83da19,// 45 PAY  42 

    0x917ef8d7,// 46 PAY  43 

    0x82f71c9f,// 47 PAY  44 

    0x370dee2f,// 48 PAY  45 

    0x8891dc4d,// 49 PAY  46 

    0xfa2815d9,// 50 PAY  47 

    0xa557af77,// 51 PAY  48 

    0x47d0a8b8,// 52 PAY  49 

    0x54f6634a,// 53 PAY  50 

    0xceaccc4e,// 54 PAY  51 

    0x69718c79,// 55 PAY  52 

    0xc1f7122a,// 56 PAY  53 

    0x2bffa2d6,// 57 PAY  54 

    0xf5cda604,// 58 PAY  55 

    0x6036ab05,// 59 PAY  56 

    0x865d2b5d,// 60 PAY  57 

    0xc5f2d241,// 61 PAY  58 

    0x18aec89d,// 62 PAY  59 

    0x14089f88,// 63 PAY  60 

    0x9d83bfc9,// 64 PAY  61 

    0x5864a85a,// 65 PAY  62 

    0xd47e0051,// 66 PAY  63 

    0x9353f0f6,// 67 PAY  64 

    0x7131e898,// 68 PAY  65 

    0xf75284b2,// 69 PAY  66 

    0x914589e3,// 70 PAY  67 

    0x95fb12d1,// 71 PAY  68 

    0x2a403e3b,// 72 PAY  69 

    0x50451b81,// 73 PAY  70 

    0x9aa9dfdd,// 74 PAY  71 

    0x3fb2b038,// 75 PAY  72 

    0xe4e74af1,// 76 PAY  73 

    0xc2ce3eaa,// 77 PAY  74 

    0x54548d87,// 78 PAY  75 

    0xc8fb92c5,// 79 PAY  76 

    0x833362e1,// 80 PAY  77 

    0x66d8af5e,// 81 PAY  78 

    0x09d1ab98,// 82 PAY  79 

    0x0f010e54,// 83 PAY  80 

    0x98baf4dd,// 84 PAY  81 

    0x432582de,// 85 PAY  82 

    0x3936d2a2,// 86 PAY  83 

    0xcd203897,// 87 PAY  84 

    0x37ee013f,// 88 PAY  85 

    0x44e88f4c,// 89 PAY  86 

    0x7f2f9e9e,// 90 PAY  87 

    0x766990ee,// 91 PAY  88 

    0xf5d1911b,// 92 PAY  89 

    0xe171d13e,// 93 PAY  90 

    0x73d9b399,// 94 PAY  91 

    0x61b2f402,// 95 PAY  92 

    0x57f5a1fa,// 96 PAY  93 

    0x7760a275,// 97 PAY  94 

    0x93fe1392,// 98 PAY  95 

    0xfe836d70,// 99 PAY  96 

    0xa643c1f0,// 100 PAY  97 

    0x288c2f82,// 101 PAY  98 

    0xe4ce759d,// 102 PAY  99 

    0xf7dba10e,// 103 PAY 100 

    0xa940d30b,// 104 PAY 101 

    0x682f3cd2,// 105 PAY 102 

    0x675277c0,// 106 PAY 103 

    0xe9a2e7e9,// 107 PAY 104 

    0x9c88435d,// 108 PAY 105 

    0x67225229,// 109 PAY 106 

    0x4bf117ea,// 110 PAY 107 

    0xdfebf5e8,// 111 PAY 108 

    0x93e0d679,// 112 PAY 109 

    0xe0cbd582,// 113 PAY 110 

    0x899f358b,// 114 PAY 111 

    0x77287992,// 115 PAY 112 

    0x9a8be002,// 116 PAY 113 

    0x7ef280e4,// 117 PAY 114 

    0xff1926ab,// 118 PAY 115 

    0xd241b86e,// 119 PAY 116 

    0xb277cacc,// 120 PAY 117 

    0x23b7182c,// 121 PAY 118 

    0xab05adc2,// 122 PAY 119 

    0xf6af014a,// 123 PAY 120 

    0xe8773a15,// 124 PAY 121 

    0xa76917f3,// 125 PAY 122 

    0x370d0225,// 126 PAY 123 

    0x224b76f1,// 127 PAY 124 

    0xec70922d,// 128 PAY 125 

    0x03f03973,// 129 PAY 126 

    0x69e0c98e,// 130 PAY 127 

    0xa5b11733,// 131 PAY 128 

    0xfd325efb,// 132 PAY 129 

    0xd6103059,// 133 PAY 130 

    0x7265d6bf,// 134 PAY 131 

    0xf066d8ef,// 135 PAY 132 

    0x0e39ee4d,// 136 PAY 133 

    0x8190daff,// 137 PAY 134 

    0x21880f02,// 138 PAY 135 

    0xbeda5f6a,// 139 PAY 136 

    0x1e4bfade,// 140 PAY 137 

    0x5b34237f,// 141 PAY 138 

    0x77125da3,// 142 PAY 139 

    0x5bc867e5,// 143 PAY 140 

    0x8bc54158,// 144 PAY 141 

    0x5832f0e4,// 145 PAY 142 

    0x79d8b242,// 146 PAY 143 

    0x408f8288,// 147 PAY 144 

    0xbcc16df5,// 148 PAY 145 

    0x75ceb821,// 149 PAY 146 

    0x0b76d7c7,// 150 PAY 147 

    0x0affc07a,// 151 PAY 148 

    0x65358d8d,// 152 PAY 149 

    0xc46b0ac7,// 153 PAY 150 

    0x14462f43,// 154 PAY 151 

    0x836bc9b7,// 155 PAY 152 

    0x94067df0,// 156 PAY 153 

    0x53cf69e6,// 157 PAY 154 

    0x14cd8886,// 158 PAY 155 

    0xfe258da3,// 159 PAY 156 

    0xa3938772,// 160 PAY 157 

    0x6e5bfae0,// 161 PAY 158 

    0x7943758a,// 162 PAY 159 

    0x9143ef99,// 163 PAY 160 

    0x1bb70b28,// 164 PAY 161 

    0x69c23c78,// 165 PAY 162 

    0x13bde055,// 166 PAY 163 

    0x01b1b7d0,// 167 PAY 164 

    0xdd16643e,// 168 PAY 165 

    0x35e24993,// 169 PAY 166 

    0x9e2097a8,// 170 PAY 167 

    0x2aefc1bc,// 171 PAY 168 

    0xe83d4106,// 172 PAY 169 

    0x83ee2a62,// 173 PAY 170 

    0xef970178,// 174 PAY 171 

    0x8838ad4a,// 175 PAY 172 

    0x7c99fd4a,// 176 PAY 173 

    0x40e62d81,// 177 PAY 174 

    0xb74a9ad3,// 178 PAY 175 

    0x6b98bebc,// 179 PAY 176 

    0x54f1c59c,// 180 PAY 177 

    0xa4754fba,// 181 PAY 178 

    0x503c836c,// 182 PAY 179 

    0xd4677536,// 183 PAY 180 

    0xaf873c05,// 184 PAY 181 

    0x1258cdf5,// 185 PAY 182 

    0x0836b495,// 186 PAY 183 

    0x1c4c874b,// 187 PAY 184 

    0x9bb01696,// 188 PAY 185 

    0x74b4c2bd,// 189 PAY 186 

    0x3602ed03,// 190 PAY 187 

    0xb764d3a7,// 191 PAY 188 

    0x362bddbf,// 192 PAY 189 

    0x3c3f1961,// 193 PAY 190 

    0x10eec89d,// 194 PAY 191 

    0xff60380f,// 195 PAY 192 

    0x76a32d58,// 196 PAY 193 

    0x70717367,// 197 PAY 194 

    0xceb2ed54,// 198 PAY 195 

    0x318dc4c8,// 199 PAY 196 

    0xa23aa1f6,// 200 PAY 197 

    0x4fa683a3,// 201 PAY 198 

    0x504b1f01,// 202 PAY 199 

    0x91d454dd,// 203 PAY 200 

    0x413f2df0,// 204 PAY 201 

    0x57ec28c2,// 205 PAY 202 

    0x2b806b96,// 206 PAY 203 

    0x0a6b327a,// 207 PAY 204 

    0x264d75d3,// 208 PAY 205 

    0xad3ab59e,// 209 PAY 206 

    0x7fdc19d9,// 210 PAY 207 

    0x228c22b8,// 211 PAY 208 

    0xd9875fd2,// 212 PAY 209 

    0x42a9ada7,// 213 PAY 210 

    0x4d485edf,// 214 PAY 211 

    0x5f600df8,// 215 PAY 212 

    0x6ef97e41,// 216 PAY 213 

    0x32621879,// 217 PAY 214 

    0xd718f099,// 218 PAY 215 

    0x54c4e205,// 219 PAY 216 

    0xcc37bc67,// 220 PAY 217 

    0x51eb9a28,// 221 PAY 218 

    0x60052a84,// 222 PAY 219 

    0xf23b16c9,// 223 PAY 220 

    0x8181c92c,// 224 PAY 221 

    0x423c6460,// 225 PAY 222 

    0xc20e0cee,// 226 PAY 223 

    0xfa96538d,// 227 PAY 224 

    0x3bc45fe5,// 228 PAY 225 

    0x6c3a42ab,// 229 PAY 226 

    0xf8abd54c,// 230 PAY 227 

    0x2cf6eedb,// 231 PAY 228 

    0xd4f381e4,// 232 PAY 229 

    0x9e6f48ff,// 233 PAY 230 

    0xdce6af13,// 234 PAY 231 

    0x06df2757,// 235 PAY 232 

    0xa9e6ea34,// 236 PAY 233 

    0xa0dafffb,// 237 PAY 234 

    0xe901b5b2,// 238 PAY 235 

    0x6e9417a3,// 239 PAY 236 

    0xed3f6ddc,// 240 PAY 237 

    0xa7ceebe4,// 241 PAY 238 

    0xbc0abd11,// 242 PAY 239 

    0x43498bdf,// 243 PAY 240 

    0x2eee88c3,// 244 PAY 241 

    0x87089588,// 245 PAY 242 

    0x78dbab9c,// 246 PAY 243 

    0x76e46ee5,// 247 PAY 244 

    0xc5156c0f,// 248 PAY 245 

    0x189b3dd9,// 249 PAY 246 

    0x5da610ef,// 250 PAY 247 

    0xeb9ee7dc,// 251 PAY 248 

    0x87144ead,// 252 PAY 249 

    0x266aefe6,// 253 PAY 250 

    0xa91db10e,// 254 PAY 251 

    0x491ca2e7,// 255 PAY 252 

    0x7a0a3962,// 256 PAY 253 

    0x1cad827a,// 257 PAY 254 

    0x27b1c1d2,// 258 PAY 255 

    0xd890569d,// 259 PAY 256 

    0x287b2494,// 260 PAY 257 

    0x69414c28,// 261 PAY 258 

    0xdc92695b,// 262 PAY 259 

    0x2cb2d138,// 263 PAY 260 

    0x320312b0,// 264 PAY 261 

    0xd9a5fb13,// 265 PAY 262 

    0x5f1f7781,// 266 PAY 263 

    0x165a4c9a,// 267 PAY 264 

    0x01cc65ef,// 268 PAY 265 

    0x27b065fd,// 269 PAY 266 

    0xedc29901,// 270 PAY 267 

    0xf36c8678,// 271 PAY 268 

    0x64aa9eb6,// 272 PAY 269 

    0xfc14e015,// 273 PAY 270 

    0x698a93f2,// 274 PAY 271 

    0x981c1261,// 275 PAY 272 

    0x1ce87baa,// 276 PAY 273 

    0x46f848ad,// 277 PAY 274 

    0xaa4246be,// 278 PAY 275 

    0x91970ecb,// 279 PAY 276 

    0xbbeab6d0,// 280 PAY 277 

    0xd048bcd4,// 281 PAY 278 

    0x036763ca,// 282 PAY 279 

    0x73ead135,// 283 PAY 280 

    0x1e1acde0,// 284 PAY 281 

    0x5aca35d8,// 285 PAY 282 

    0x27f29aec,// 286 PAY 283 

    0x488a4e58,// 287 PAY 284 

    0x5cc51821,// 288 PAY 285 

    0x821767e8,// 289 PAY 286 

    0x9e3953e3,// 290 PAY 287 

    0x5f74da4d,// 291 PAY 288 

    0x76263571,// 292 PAY 289 

    0xbf462c2e,// 293 PAY 290 

    0xe7e6aeea,// 294 PAY 291 

    0x510f1d9a,// 295 PAY 292 

    0xa72d2cb8,// 296 PAY 293 

    0xb7c10be1,// 297 PAY 294 

    0xed91aa2b,// 298 PAY 295 

    0x0404c639,// 299 PAY 296 

    0x07d06c73,// 300 PAY 297 

    0x1cf306aa,// 301 PAY 298 

    0xf33b3c4b,// 302 PAY 299 

    0xce1851fe,// 303 PAY 300 

    0x98044f00,// 304 PAY 301 

    0x8f51c7a5,// 305 PAY 302 

    0xfdf461a8,// 306 PAY 303 

    0x818108f1,// 307 PAY 304 

    0xabf7dd2d,// 308 PAY 305 

    0xb66cbfd2,// 309 PAY 306 

    0x449a5c9d,// 310 PAY 307 

    0x20042617,// 311 PAY 308 

    0xac075975,// 312 PAY 309 

    0x53cdae0e,// 313 PAY 310 

    0xc4bb1c38,// 314 PAY 311 

    0x881e13a2,// 315 PAY 312 

    0xeb850728,// 316 PAY 313 

    0xce7a7577,// 317 PAY 314 

    0xff981773,// 318 PAY 315 

    0x3dd24e80,// 319 PAY 316 

    0x92df9fe7,// 320 PAY 317 

    0xab22d160,// 321 PAY 318 

    0xa4500900,// 322 PAY 319 

/// HASH is  12 bytes 

    0x77125da3,// 323 HSH   1 

    0x5bc867e5,// 324 HSH   2 

    0x8bc54158,// 325 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 20 

/// STA pkt_idx        : 132 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x13 

    0x02111314 // 326 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt71_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 307 words. 

/// BDA size     is 1223 (0x4c7) 

/// BDA id       is 0xc9c9 

    0x04c7c9c9,// 3 BDA   1 

/// PAY Generic Data size   : 1223 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x93535ef4,// 4 PAY   1 

    0x31b37ca3,// 5 PAY   2 

    0xaf8633d3,// 6 PAY   3 

    0x4fc3bc01,// 7 PAY   4 

    0x9ee538a3,// 8 PAY   5 

    0x28c621b9,// 9 PAY   6 

    0x09a89d27,// 10 PAY   7 

    0xde451940,// 11 PAY   8 

    0xf3e0e9db,// 12 PAY   9 

    0xeab308c7,// 13 PAY  10 

    0x3f454e35,// 14 PAY  11 

    0xf1e3d20d,// 15 PAY  12 

    0x803d2386,// 16 PAY  13 

    0x67d61b83,// 17 PAY  14 

    0x384bdbeb,// 18 PAY  15 

    0xf472a74b,// 19 PAY  16 

    0x325b01b5,// 20 PAY  17 

    0xf574cc3a,// 21 PAY  18 

    0x7e9a3c27,// 22 PAY  19 

    0xbece69ff,// 23 PAY  20 

    0x9502f31c,// 24 PAY  21 

    0x02c84a34,// 25 PAY  22 

    0x503b3557,// 26 PAY  23 

    0xc753a40b,// 27 PAY  24 

    0x77bdc6b6,// 28 PAY  25 

    0xe1979a55,// 29 PAY  26 

    0xfbfedba7,// 30 PAY  27 

    0x14dc3fec,// 31 PAY  28 

    0xf4d74a73,// 32 PAY  29 

    0xced3855b,// 33 PAY  30 

    0x1dc6c491,// 34 PAY  31 

    0xc9aa6366,// 35 PAY  32 

    0x459f3af9,// 36 PAY  33 

    0xb61c1b52,// 37 PAY  34 

    0x9fe39dee,// 38 PAY  35 

    0x2268b24e,// 39 PAY  36 

    0x04f85423,// 40 PAY  37 

    0x4a155942,// 41 PAY  38 

    0x4398f0f6,// 42 PAY  39 

    0x0eb7bdc8,// 43 PAY  40 

    0xa5101cbe,// 44 PAY  41 

    0xdd366a6e,// 45 PAY  42 

    0x0d7f6761,// 46 PAY  43 

    0xff415f35,// 47 PAY  44 

    0x740731d9,// 48 PAY  45 

    0x8b7e0607,// 49 PAY  46 

    0xcb12cdfc,// 50 PAY  47 

    0xdc7db7ba,// 51 PAY  48 

    0x3106094c,// 52 PAY  49 

    0xc94e0810,// 53 PAY  50 

    0x326ad469,// 54 PAY  51 

    0x55d22c98,// 55 PAY  52 

    0x545bfea1,// 56 PAY  53 

    0xd8fb8dd1,// 57 PAY  54 

    0x0c4f1cd8,// 58 PAY  55 

    0x6ae742eb,// 59 PAY  56 

    0x08e43677,// 60 PAY  57 

    0xb8d90ad6,// 61 PAY  58 

    0xd40d8ff9,// 62 PAY  59 

    0xf50df376,// 63 PAY  60 

    0x61bf5345,// 64 PAY  61 

    0xf6a16c12,// 65 PAY  62 

    0xba9f6d9b,// 66 PAY  63 

    0x0c721e6f,// 67 PAY  64 

    0xe4c77e4b,// 68 PAY  65 

    0xb7085da7,// 69 PAY  66 

    0x58f64a3a,// 70 PAY  67 

    0x488e3a26,// 71 PAY  68 

    0x21c9d2fe,// 72 PAY  69 

    0xd722a351,// 73 PAY  70 

    0x4fc2ac4b,// 74 PAY  71 

    0x404348bd,// 75 PAY  72 

    0x84fdda40,// 76 PAY  73 

    0x20bdb30f,// 77 PAY  74 

    0xb3951a39,// 78 PAY  75 

    0x9574977c,// 79 PAY  76 

    0x1f7d6fe5,// 80 PAY  77 

    0x80be8f39,// 81 PAY  78 

    0x299cc98e,// 82 PAY  79 

    0xa1eb7d34,// 83 PAY  80 

    0xa11204be,// 84 PAY  81 

    0xe80170fe,// 85 PAY  82 

    0x27f13fd0,// 86 PAY  83 

    0x1fef0790,// 87 PAY  84 

    0xe05e0c39,// 88 PAY  85 

    0x02ea676a,// 89 PAY  86 

    0x4738c840,// 90 PAY  87 

    0x02a0df77,// 91 PAY  88 

    0x0a3c4a26,// 92 PAY  89 

    0xabc613c8,// 93 PAY  90 

    0xbf288444,// 94 PAY  91 

    0xd2a14993,// 95 PAY  92 

    0x9dd8c97e,// 96 PAY  93 

    0x9961bab8,// 97 PAY  94 

    0x51388a07,// 98 PAY  95 

    0xb77b5603,// 99 PAY  96 

    0xaac47a40,// 100 PAY  97 

    0x62e3a73a,// 101 PAY  98 

    0xbf1567ce,// 102 PAY  99 

    0x8cb249fb,// 103 PAY 100 

    0x5cc3df0f,// 104 PAY 101 

    0x087bc034,// 105 PAY 102 

    0x60f3fe3b,// 106 PAY 103 

    0x6f014bbb,// 107 PAY 104 

    0x968d6818,// 108 PAY 105 

    0xf8865bb2,// 109 PAY 106 

    0x6ab26356,// 110 PAY 107 

    0xc1a51bd9,// 111 PAY 108 

    0xf2aabf73,// 112 PAY 109 

    0x51c0d950,// 113 PAY 110 

    0x425b3d1d,// 114 PAY 111 

    0x70458f55,// 115 PAY 112 

    0xfa0fc01e,// 116 PAY 113 

    0x8a2c022e,// 117 PAY 114 

    0x8f97d757,// 118 PAY 115 

    0xe94b50bd,// 119 PAY 116 

    0x56e33aa1,// 120 PAY 117 

    0x99b184d7,// 121 PAY 118 

    0xfb1224aa,// 122 PAY 119 

    0xdf4148e1,// 123 PAY 120 

    0x4b14150a,// 124 PAY 121 

    0x5ce722e9,// 125 PAY 122 

    0x570ee6a3,// 126 PAY 123 

    0x18633896,// 127 PAY 124 

    0xac943f29,// 128 PAY 125 

    0xc46f0abd,// 129 PAY 126 

    0x109b1ff3,// 130 PAY 127 

    0xbd7019d2,// 131 PAY 128 

    0xb13ed464,// 132 PAY 129 

    0x12e3340c,// 133 PAY 130 

    0xdb9b17a9,// 134 PAY 131 

    0x3f31927d,// 135 PAY 132 

    0x24a56c90,// 136 PAY 133 

    0x1de5f8d9,// 137 PAY 134 

    0x0108920b,// 138 PAY 135 

    0xfbfbcc6b,// 139 PAY 136 

    0xb847da45,// 140 PAY 137 

    0xc2612b49,// 141 PAY 138 

    0xdd6617a8,// 142 PAY 139 

    0xc2c5995c,// 143 PAY 140 

    0x63754d97,// 144 PAY 141 

    0x0acdfbe4,// 145 PAY 142 

    0x10c44c65,// 146 PAY 143 

    0x0481bf85,// 147 PAY 144 

    0xfdd3aaf0,// 148 PAY 145 

    0x2d3a31f7,// 149 PAY 146 

    0xd3cfaf91,// 150 PAY 147 

    0xc7e0466c,// 151 PAY 148 

    0xeee3d9bd,// 152 PAY 149 

    0x2ac0f10a,// 153 PAY 150 

    0x3cf2efe7,// 154 PAY 151 

    0xf4bb0e7e,// 155 PAY 152 

    0xdd558009,// 156 PAY 153 

    0x8f4dad83,// 157 PAY 154 

    0x8c61e215,// 158 PAY 155 

    0x9b98eed8,// 159 PAY 156 

    0x84308b33,// 160 PAY 157 

    0x9fe1639b,// 161 PAY 158 

    0x32ea71fe,// 162 PAY 159 

    0xe5e66d35,// 163 PAY 160 

    0xf90d342a,// 164 PAY 161 

    0xf5a51c8c,// 165 PAY 162 

    0x6d52c87d,// 166 PAY 163 

    0xd2398e8d,// 167 PAY 164 

    0x4bb12088,// 168 PAY 165 

    0x36b0b54d,// 169 PAY 166 

    0x7afe18d6,// 170 PAY 167 

    0x505303e9,// 171 PAY 168 

    0xaee69759,// 172 PAY 169 

    0x8eda9d70,// 173 PAY 170 

    0x2d6b7537,// 174 PAY 171 

    0x1f9043f7,// 175 PAY 172 

    0xa8fd84d9,// 176 PAY 173 

    0x4c409742,// 177 PAY 174 

    0xfbeababb,// 178 PAY 175 

    0x5725e4e8,// 179 PAY 176 

    0x0abef7f2,// 180 PAY 177 

    0xe8716edb,// 181 PAY 178 

    0xac50bba7,// 182 PAY 179 

    0xf86c846c,// 183 PAY 180 

    0x815a61d7,// 184 PAY 181 

    0x9912b7f9,// 185 PAY 182 

    0x8925d10a,// 186 PAY 183 

    0x8d64502e,// 187 PAY 184 

    0x7bf2bdb5,// 188 PAY 185 

    0x967fbaad,// 189 PAY 186 

    0xb147caa8,// 190 PAY 187 

    0x13493382,// 191 PAY 188 

    0xeb8410d1,// 192 PAY 189 

    0x4573a155,// 193 PAY 190 

    0x8c4290f0,// 194 PAY 191 

    0xe0a6b1b0,// 195 PAY 192 

    0x513fbd4e,// 196 PAY 193 

    0xf02beecb,// 197 PAY 194 

    0x15d653cc,// 198 PAY 195 

    0xcbb21213,// 199 PAY 196 

    0xde38b5d3,// 200 PAY 197 

    0xf8f49530,// 201 PAY 198 

    0x499ab83f,// 202 PAY 199 

    0x7e27bb52,// 203 PAY 200 

    0xd86e084b,// 204 PAY 201 

    0x02950e22,// 205 PAY 202 

    0x523e3fc7,// 206 PAY 203 

    0xf8dc55d3,// 207 PAY 204 

    0xa3de63f2,// 208 PAY 205 

    0x91163767,// 209 PAY 206 

    0xf3ac1fe2,// 210 PAY 207 

    0xd3d014f7,// 211 PAY 208 

    0xa05ab35d,// 212 PAY 209 

    0x1c4e90c0,// 213 PAY 210 

    0x4c7b550f,// 214 PAY 211 

    0xbac09ec9,// 215 PAY 212 

    0x83a6e934,// 216 PAY 213 

    0x1f9a48e5,// 217 PAY 214 

    0xfcbfc784,// 218 PAY 215 

    0x808d58ad,// 219 PAY 216 

    0x39bd5c37,// 220 PAY 217 

    0xa590c28b,// 221 PAY 218 

    0x6b3e8175,// 222 PAY 219 

    0xe897c723,// 223 PAY 220 

    0x30d2480c,// 224 PAY 221 

    0x3e2c7b56,// 225 PAY 222 

    0xe6085f0f,// 226 PAY 223 

    0x760a3e29,// 227 PAY 224 

    0xa1810b6e,// 228 PAY 225 

    0xa9c330d2,// 229 PAY 226 

    0xfc2c6339,// 230 PAY 227 

    0x2f724d86,// 231 PAY 228 

    0x8d747bfb,// 232 PAY 229 

    0xb4051c85,// 233 PAY 230 

    0xf184adf1,// 234 PAY 231 

    0x180d4dc0,// 235 PAY 232 

    0x99218fe3,// 236 PAY 233 

    0x515e3c97,// 237 PAY 234 

    0xb5cdfe7d,// 238 PAY 235 

    0xe79a4d2d,// 239 PAY 236 

    0xd0d4a626,// 240 PAY 237 

    0xbfa218a9,// 241 PAY 238 

    0x442bdb78,// 242 PAY 239 

    0x09d1db9f,// 243 PAY 240 

    0xb619fc79,// 244 PAY 241 

    0xc2f10d96,// 245 PAY 242 

    0x622f6926,// 246 PAY 243 

    0xcdf81522,// 247 PAY 244 

    0xf7fb8699,// 248 PAY 245 

    0x66efadb3,// 249 PAY 246 

    0xd94080da,// 250 PAY 247 

    0x57161937,// 251 PAY 248 

    0x167a62d6,// 252 PAY 249 

    0xe10ab55e,// 253 PAY 250 

    0x574abc09,// 254 PAY 251 

    0xc30c2a1d,// 255 PAY 252 

    0x079a3e06,// 256 PAY 253 

    0xe2aaa3e9,// 257 PAY 254 

    0x2ebf09f9,// 258 PAY 255 

    0xda0662c8,// 259 PAY 256 

    0xac4f5540,// 260 PAY 257 

    0xcd628647,// 261 PAY 258 

    0xdca5d8e1,// 262 PAY 259 

    0xe8e2186d,// 263 PAY 260 

    0x0ca7e676,// 264 PAY 261 

    0x62aaccab,// 265 PAY 262 

    0x7d6ff48b,// 266 PAY 263 

    0xf12bfbdc,// 267 PAY 264 

    0xa24d12b7,// 268 PAY 265 

    0xcdec1baf,// 269 PAY 266 

    0xddf23f46,// 270 PAY 267 

    0xde0be85a,// 271 PAY 268 

    0x3cb34812,// 272 PAY 269 

    0xe1c30fdb,// 273 PAY 270 

    0x476a27bc,// 274 PAY 271 

    0x447ba0e4,// 275 PAY 272 

    0x46dd3c7e,// 276 PAY 273 

    0x825916cb,// 277 PAY 274 

    0xed18db2b,// 278 PAY 275 

    0x171e175b,// 279 PAY 276 

    0x765dd56b,// 280 PAY 277 

    0x8f39822a,// 281 PAY 278 

    0xb482c2f5,// 282 PAY 279 

    0x8efbc7d1,// 283 PAY 280 

    0x1716626d,// 284 PAY 281 

    0x97396074,// 285 PAY 282 

    0x098ba85c,// 286 PAY 283 

    0x1758e231,// 287 PAY 284 

    0xa2942edc,// 288 PAY 285 

    0x088ffc00,// 289 PAY 286 

    0x68a64093,// 290 PAY 287 

    0x9411f84c,// 291 PAY 288 

    0x3f99a3af,// 292 PAY 289 

    0x96c46df2,// 293 PAY 290 

    0x42246c37,// 294 PAY 291 

    0xfed23de3,// 295 PAY 292 

    0x9fa1a7cb,// 296 PAY 293 

    0xa2199d34,// 297 PAY 294 

    0xbf76e2e8,// 298 PAY 295 

    0x428f1e69,// 299 PAY 296 

    0x91b2a809,// 300 PAY 297 

    0x2cf92a15,// 301 PAY 298 

    0xcbafe4ae,// 302 PAY 299 

    0xc8996674,// 303 PAY 300 

    0x34afc052,// 304 PAY 301 

    0xefef8a10,// 305 PAY 302 

    0xed3c405d,// 306 PAY 303 

    0x23eaca19,// 307 PAY 304 

    0xa5ef9c7e,// 308 PAY 305 

    0x9fe10d00,// 309 PAY 306 

/// HASH is  4 bytes 

    0x8f4dad83,// 310 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 85 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf5 

    0x02ccf555 // 311 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt72_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 28 words. 

/// BDA size     is 105 (0x69) 

/// BDA id       is 0xdad4 

    0x0069dad4,// 3 BDA   1 

/// PAY Generic Data size   : 105 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x2e4c8f44,// 4 PAY   1 

    0x845e6a32,// 5 PAY   2 

    0x23b36b2b,// 6 PAY   3 

    0x34978cb9,// 7 PAY   4 

    0x47ed8be4,// 8 PAY   5 

    0x0ca55f53,// 9 PAY   6 

    0x13460391,// 10 PAY   7 

    0xe257145d,// 11 PAY   8 

    0x3034a7e3,// 12 PAY   9 

    0xb42026b0,// 13 PAY  10 

    0x514ef3a6,// 14 PAY  11 

    0x3d6c22f1,// 15 PAY  12 

    0x8208e2ce,// 16 PAY  13 

    0x083595ec,// 17 PAY  14 

    0xcb2f7c82,// 18 PAY  15 

    0x364ff57f,// 19 PAY  16 

    0xe1078ddd,// 20 PAY  17 

    0xa0ec884d,// 21 PAY  18 

    0xcdfa79f9,// 22 PAY  19 

    0x38c2158a,// 23 PAY  20 

    0xc5e20b3d,// 24 PAY  21 

    0xc15a3818,// 25 PAY  22 

    0x98fcab8c,// 26 PAY  23 

    0x72c722e0,// 27 PAY  24 

    0xf06cefa6,// 28 PAY  25 

    0x8f50159c,// 29 PAY  26 

    0x46000000,// 30 PAY  27 

/// HASH is  16 bytes 

    0xa0ec884d,// 31 HSH   1 

    0xcdfa79f9,// 32 HSH   2 

    0x38c2158a,// 33 HSH   3 

    0xc5e20b3d,// 34 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 45 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x94 

    0x00b5942c // 35 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt73_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 109 words. 

/// BDA size     is 429 (0x1ad) 

/// BDA id       is 0xb041 

    0x01adb041,// 3 BDA   1 

/// PAY Generic Data size   : 429 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xf1149571,// 4 PAY   1 

    0x2a17d39f,// 5 PAY   2 

    0xa6b19785,// 6 PAY   3 

    0x537bb3d4,// 7 PAY   4 

    0x5582d983,// 8 PAY   5 

    0x52921999,// 9 PAY   6 

    0x705ca01f,// 10 PAY   7 

    0xc4df5b02,// 11 PAY   8 

    0x7eb02e24,// 12 PAY   9 

    0xcd76d848,// 13 PAY  10 

    0x63801ee4,// 14 PAY  11 

    0x35f2ed30,// 15 PAY  12 

    0x6bc4804e,// 16 PAY  13 

    0xd16a3c7c,// 17 PAY  14 

    0x3f01b1c6,// 18 PAY  15 

    0xd7c41b18,// 19 PAY  16 

    0xa79a2569,// 20 PAY  17 

    0x869eedba,// 21 PAY  18 

    0x49a2581b,// 22 PAY  19 

    0x928ccfa9,// 23 PAY  20 

    0xf29ebcf3,// 24 PAY  21 

    0x351f4ca4,// 25 PAY  22 

    0xb7e5d945,// 26 PAY  23 

    0x59066568,// 27 PAY  24 

    0x7bfe764e,// 28 PAY  25 

    0xdbdc9669,// 29 PAY  26 

    0xdc9bec84,// 30 PAY  27 

    0xaebde1f9,// 31 PAY  28 

    0x182a2923,// 32 PAY  29 

    0xc4aa871e,// 33 PAY  30 

    0xb7c19f41,// 34 PAY  31 

    0xf983cd05,// 35 PAY  32 

    0xafad39d5,// 36 PAY  33 

    0x762ba32e,// 37 PAY  34 

    0xb4cc6c2e,// 38 PAY  35 

    0x4714bd3c,// 39 PAY  36 

    0xba54b963,// 40 PAY  37 

    0x613a7520,// 41 PAY  38 

    0x3433113d,// 42 PAY  39 

    0xa8a762a9,// 43 PAY  40 

    0x40749acb,// 44 PAY  41 

    0xfd530b69,// 45 PAY  42 

    0x2a4ee7f3,// 46 PAY  43 

    0xac1874be,// 47 PAY  44 

    0x4beef707,// 48 PAY  45 

    0xc4d18805,// 49 PAY  46 

    0xc4704f37,// 50 PAY  47 

    0xc9be9f82,// 51 PAY  48 

    0x39a274de,// 52 PAY  49 

    0x69de2cf8,// 53 PAY  50 

    0x15c820db,// 54 PAY  51 

    0xb7ccd921,// 55 PAY  52 

    0x8a3479fe,// 56 PAY  53 

    0xe165c96c,// 57 PAY  54 

    0x537ee4f6,// 58 PAY  55 

    0x1c1aee5a,// 59 PAY  56 

    0x4340d233,// 60 PAY  57 

    0x9bec2138,// 61 PAY  58 

    0xab1e15d8,// 62 PAY  59 

    0x8abd0451,// 63 PAY  60 

    0xeaf35917,// 64 PAY  61 

    0x72b0af5e,// 65 PAY  62 

    0x50c444b9,// 66 PAY  63 

    0xb0e82c97,// 67 PAY  64 

    0x922640cb,// 68 PAY  65 

    0x835962fd,// 69 PAY  66 

    0xae522b80,// 70 PAY  67 

    0x8aa64481,// 71 PAY  68 

    0x7ea18dcc,// 72 PAY  69 

    0x98a937bd,// 73 PAY  70 

    0xecc8286f,// 74 PAY  71 

    0x31a4bad9,// 75 PAY  72 

    0x0edd2e6d,// 76 PAY  73 

    0xfd7fc345,// 77 PAY  74 

    0x592b25b9,// 78 PAY  75 

    0x87f23a29,// 79 PAY  76 

    0x3d6883e9,// 80 PAY  77 

    0xf655347c,// 81 PAY  78 

    0xecb9f0aa,// 82 PAY  79 

    0x4076a3fd,// 83 PAY  80 

    0x259078f8,// 84 PAY  81 

    0xbb309c1a,// 85 PAY  82 

    0xcba123c8,// 86 PAY  83 

    0x46e7684e,// 87 PAY  84 

    0x704b8164,// 88 PAY  85 

    0x639e6fb2,// 89 PAY  86 

    0x989a5ae0,// 90 PAY  87 

    0x0fa0d68f,// 91 PAY  88 

    0x8d8e4fb3,// 92 PAY  89 

    0xf92e6e23,// 93 PAY  90 

    0x2355b339,// 94 PAY  91 

    0x36da1cad,// 95 PAY  92 

    0xf46db92f,// 96 PAY  93 

    0x67d90d58,// 97 PAY  94 

    0xf2c75b48,// 98 PAY  95 

    0x1b019897,// 99 PAY  96 

    0x62ff8ee5,// 100 PAY  97 

    0x50f2b0e9,// 101 PAY  98 

    0x1581a56f,// 102 PAY  99 

    0x2c563c95,// 103 PAY 100 

    0x7b1b7473,// 104 PAY 101 

    0x89a4c04d,// 105 PAY 102 

    0x1797ade8,// 106 PAY 103 

    0x50e63144,// 107 PAY 104 

    0x4d716264,// 108 PAY 105 

    0x95443caf,// 109 PAY 106 

    0x2bbd243e,// 110 PAY 107 

    0x5c000000,// 111 PAY 108 

/// HASH is  8 bytes 

    0x89a4c04d,// 112 HSH   1 

    0x1797ade8,// 113 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 218 

/// STA pkt_idx        : 241 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x44 

    0x03c544da // 114 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt74_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 473 words. 

/// BDA size     is 1885 (0x75d) 

/// BDA id       is 0xa0be 

    0x075da0be,// 3 BDA   1 

/// PAY Generic Data size   : 1885 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x5cd118cb,// 4 PAY   1 

    0x16489fb9,// 5 PAY   2 

    0x0fb3080a,// 6 PAY   3 

    0x9a3de689,// 7 PAY   4 

    0x802e6100,// 8 PAY   5 

    0xce4393a4,// 9 PAY   6 

    0x7abe2e7f,// 10 PAY   7 

    0xe68ac8a4,// 11 PAY   8 

    0xbf4ca502,// 12 PAY   9 

    0x97b083be,// 13 PAY  10 

    0x43aaa914,// 14 PAY  11 

    0xbeab49bf,// 15 PAY  12 

    0xdeab60cd,// 16 PAY  13 

    0x44bf5494,// 17 PAY  14 

    0x0b9c32be,// 18 PAY  15 

    0x4ae85f49,// 19 PAY  16 

    0x0423034f,// 20 PAY  17 

    0xbcc4fc99,// 21 PAY  18 

    0xccdcc124,// 22 PAY  19 

    0xe4f08bb7,// 23 PAY  20 

    0xf0daccf4,// 24 PAY  21 

    0xfd431b65,// 25 PAY  22 

    0xf7a9a11a,// 26 PAY  23 

    0xbecbbf26,// 27 PAY  24 

    0x58fd3a58,// 28 PAY  25 

    0x8b2086da,// 29 PAY  26 

    0x69a05a21,// 30 PAY  27 

    0x93a14140,// 31 PAY  28 

    0x4cd700ae,// 32 PAY  29 

    0x2751f0c4,// 33 PAY  30 

    0x7be8b910,// 34 PAY  31 

    0xf75ae2f2,// 35 PAY  32 

    0x472ff78c,// 36 PAY  33 

    0xcf6716b7,// 37 PAY  34 

    0xca33a4a6,// 38 PAY  35 

    0xc25af151,// 39 PAY  36 

    0xf1619095,// 40 PAY  37 

    0x3ea3c726,// 41 PAY  38 

    0xedaaeb55,// 42 PAY  39 

    0xfa603639,// 43 PAY  40 

    0x1185d6a8,// 44 PAY  41 

    0x9a3eea05,// 45 PAY  42 

    0xa23b9d11,// 46 PAY  43 

    0xbc77c351,// 47 PAY  44 

    0xbc603d67,// 48 PAY  45 

    0xb9718142,// 49 PAY  46 

    0x72dda0e1,// 50 PAY  47 

    0xe0ce5244,// 51 PAY  48 

    0x4a7607a0,// 52 PAY  49 

    0xc2bfa806,// 53 PAY  50 

    0x26e34f55,// 54 PAY  51 

    0x8f7487cc,// 55 PAY  52 

    0x03ebc37d,// 56 PAY  53 

    0xaf7048fb,// 57 PAY  54 

    0x390981bb,// 58 PAY  55 

    0x2025f23f,// 59 PAY  56 

    0x1d96a86a,// 60 PAY  57 

    0xf1d75e93,// 61 PAY  58 

    0x70d693ea,// 62 PAY  59 

    0xa366e9fd,// 63 PAY  60 

    0x64a08016,// 64 PAY  61 

    0x063deba1,// 65 PAY  62 

    0x370fa7ef,// 66 PAY  63 

    0x736b7d55,// 67 PAY  64 

    0xf99b23a2,// 68 PAY  65 

    0x51915110,// 69 PAY  66 

    0x42e32910,// 70 PAY  67 

    0x6f0c1fb9,// 71 PAY  68 

    0x9d2997cb,// 72 PAY  69 

    0x4e0ff9e0,// 73 PAY  70 

    0x233b0bda,// 74 PAY  71 

    0xd38f2360,// 75 PAY  72 

    0xc613f025,// 76 PAY  73 

    0x436c56d1,// 77 PAY  74 

    0x400cb8e5,// 78 PAY  75 

    0x690b9f67,// 79 PAY  76 

    0x96f6c2b3,// 80 PAY  77 

    0xdc7cc3f8,// 81 PAY  78 

    0xde71f785,// 82 PAY  79 

    0x67640147,// 83 PAY  80 

    0xa782817f,// 84 PAY  81 

    0x8c3b1ea1,// 85 PAY  82 

    0x153c4dd3,// 86 PAY  83 

    0x98799afa,// 87 PAY  84 

    0xced99272,// 88 PAY  85 

    0x3fef0fd3,// 89 PAY  86 

    0x06067513,// 90 PAY  87 

    0x1e45d32c,// 91 PAY  88 

    0xd00bc1d0,// 92 PAY  89 

    0x77a37f85,// 93 PAY  90 

    0x41d138fa,// 94 PAY  91 

    0xcef437a6,// 95 PAY  92 

    0x19dbbc2c,// 96 PAY  93 

    0x98e4156a,// 97 PAY  94 

    0x2e4bb72b,// 98 PAY  95 

    0xcf4d4a70,// 99 PAY  96 

    0xaedbe741,// 100 PAY  97 

    0x544c6254,// 101 PAY  98 

    0x9c67e2f4,// 102 PAY  99 

    0x5535641d,// 103 PAY 100 

    0x2594b87d,// 104 PAY 101 

    0x5ce8cb2e,// 105 PAY 102 

    0x09eb1073,// 106 PAY 103 

    0x84179727,// 107 PAY 104 

    0xa8a05491,// 108 PAY 105 

    0x2e26c25d,// 109 PAY 106 

    0x30eb60b7,// 110 PAY 107 

    0xe1fe83e8,// 111 PAY 108 

    0xeea12ae5,// 112 PAY 109 

    0x4ba2f204,// 113 PAY 110 

    0xcf46af54,// 114 PAY 111 

    0x7b673d7f,// 115 PAY 112 

    0x34e5ca05,// 116 PAY 113 

    0xb218a4e7,// 117 PAY 114 

    0x96f2d1a6,// 118 PAY 115 

    0xc848785f,// 119 PAY 116 

    0x557749ac,// 120 PAY 117 

    0xa4f42d5c,// 121 PAY 118 

    0xff464575,// 122 PAY 119 

    0x34ceadcb,// 123 PAY 120 

    0x3f791e63,// 124 PAY 121 

    0xb7deac31,// 125 PAY 122 

    0x4dda39d9,// 126 PAY 123 

    0x68411e3c,// 127 PAY 124 

    0xf271b756,// 128 PAY 125 

    0x81eb8ca0,// 129 PAY 126 

    0xbfdd9c48,// 130 PAY 127 

    0x921e1c2f,// 131 PAY 128 

    0xa72ab4dd,// 132 PAY 129 

    0x5f0f0d35,// 133 PAY 130 

    0xf45d6678,// 134 PAY 131 

    0x38d25461,// 135 PAY 132 

    0x9c80b547,// 136 PAY 133 

    0x40652c6b,// 137 PAY 134 

    0x41975b31,// 138 PAY 135 

    0x3f86bafb,// 139 PAY 136 

    0x61785b62,// 140 PAY 137 

    0x90c5bc51,// 141 PAY 138 

    0x63cf25a3,// 142 PAY 139 

    0xb7c6a057,// 143 PAY 140 

    0x4c7598fc,// 144 PAY 141 

    0x257fdd9e,// 145 PAY 142 

    0x030f4e13,// 146 PAY 143 

    0x003581fb,// 147 PAY 144 

    0x49ac8389,// 148 PAY 145 

    0x55fa427d,// 149 PAY 146 

    0xb4ec7ba8,// 150 PAY 147 

    0xb7f6f4f5,// 151 PAY 148 

    0x4f21a5b0,// 152 PAY 149 

    0xe53b4a94,// 153 PAY 150 

    0x87ac8444,// 154 PAY 151 

    0xefce69f7,// 155 PAY 152 

    0xcb3edfed,// 156 PAY 153 

    0xab489494,// 157 PAY 154 

    0xd575dc07,// 158 PAY 155 

    0x60360898,// 159 PAY 156 

    0x197eea18,// 160 PAY 157 

    0xe49ce216,// 161 PAY 158 

    0xa3eec54f,// 162 PAY 159 

    0x3fe70785,// 163 PAY 160 

    0x8af5fd68,// 164 PAY 161 

    0x2dfff093,// 165 PAY 162 

    0x1886bb99,// 166 PAY 163 

    0xc5cbbba2,// 167 PAY 164 

    0xa92c68c5,// 168 PAY 165 

    0xc4436048,// 169 PAY 166 

    0xa4637da6,// 170 PAY 167 

    0x9e316e59,// 171 PAY 168 

    0x05997632,// 172 PAY 169 

    0x8fb7c7df,// 173 PAY 170 

    0x8299ef87,// 174 PAY 171 

    0x5760ea36,// 175 PAY 172 

    0x655fa70b,// 176 PAY 173 

    0x8ba83105,// 177 PAY 174 

    0x4f3a60a1,// 178 PAY 175 

    0xd4aab2eb,// 179 PAY 176 

    0x1bf553fc,// 180 PAY 177 

    0x757fc414,// 181 PAY 178 

    0x2f3cec51,// 182 PAY 179 

    0x0fecae9a,// 183 PAY 180 

    0x60b66947,// 184 PAY 181 

    0x2267959a,// 185 PAY 182 

    0x3eecf956,// 186 PAY 183 

    0xbc926fa1,// 187 PAY 184 

    0xe3d37da7,// 188 PAY 185 

    0x74573fdd,// 189 PAY 186 

    0xe5583c6c,// 190 PAY 187 

    0x7c3ff67a,// 191 PAY 188 

    0xd2aa53f8,// 192 PAY 189 

    0xdbfc7653,// 193 PAY 190 

    0x849eb8c2,// 194 PAY 191 

    0x9261c6c6,// 195 PAY 192 

    0x01c379dc,// 196 PAY 193 

    0xad3048b1,// 197 PAY 194 

    0x0d6a0fa4,// 198 PAY 195 

    0xeba60c10,// 199 PAY 196 

    0x764ebdba,// 200 PAY 197 

    0xec3cb7cc,// 201 PAY 198 

    0x5c896468,// 202 PAY 199 

    0x1ccd07b6,// 203 PAY 200 

    0xfa588fa8,// 204 PAY 201 

    0xb261a581,// 205 PAY 202 

    0xf4d1a90a,// 206 PAY 203 

    0xeece991f,// 207 PAY 204 

    0xf9c54678,// 208 PAY 205 

    0x87d239ee,// 209 PAY 206 

    0xa2430abe,// 210 PAY 207 

    0x22a195f1,// 211 PAY 208 

    0x5c2ddc53,// 212 PAY 209 

    0x597e09e5,// 213 PAY 210 

    0xa5bc0319,// 214 PAY 211 

    0x194cf783,// 215 PAY 212 

    0x09b1d463,// 216 PAY 213 

    0xca956950,// 217 PAY 214 

    0xfcc7bfc3,// 218 PAY 215 

    0x4eac2c02,// 219 PAY 216 

    0x549b2c1b,// 220 PAY 217 

    0x07c11659,// 221 PAY 218 

    0xacf809ce,// 222 PAY 219 

    0x6be8b89a,// 223 PAY 220 

    0x8b1e799e,// 224 PAY 221 

    0x3ae1f2f6,// 225 PAY 222 

    0x7e8f7e12,// 226 PAY 223 

    0x06d9baa2,// 227 PAY 224 

    0x86c914a9,// 228 PAY 225 

    0xb33678b1,// 229 PAY 226 

    0x59298f87,// 230 PAY 227 

    0xa1c56593,// 231 PAY 228 

    0x1bec1270,// 232 PAY 229 

    0x25c44001,// 233 PAY 230 

    0x6f8e81d2,// 234 PAY 231 

    0xce9e2a32,// 235 PAY 232 

    0x35fc45ce,// 236 PAY 233 

    0x43c89c5a,// 237 PAY 234 

    0x3a7c85ce,// 238 PAY 235 

    0xd3db6328,// 239 PAY 236 

    0x4e87e803,// 240 PAY 237 

    0x0ebdda92,// 241 PAY 238 

    0x03a33cfc,// 242 PAY 239 

    0xb832e011,// 243 PAY 240 

    0x0d4c1d7e,// 244 PAY 241 

    0x1e202416,// 245 PAY 242 

    0xc5a4951a,// 246 PAY 243 

    0x584b01a4,// 247 PAY 244 

    0x9c5d70fc,// 248 PAY 245 

    0x486750e5,// 249 PAY 246 

    0xed4ddd57,// 250 PAY 247 

    0x01ab6949,// 251 PAY 248 

    0x123b734b,// 252 PAY 249 

    0xd37a76f5,// 253 PAY 250 

    0x7699881f,// 254 PAY 251 

    0x9df0fde9,// 255 PAY 252 

    0x693a2ecf,// 256 PAY 253 

    0x6c36e477,// 257 PAY 254 

    0x66ff2416,// 258 PAY 255 

    0x2075354b,// 259 PAY 256 

    0x1f51569e,// 260 PAY 257 

    0x088ac978,// 261 PAY 258 

    0xfd508690,// 262 PAY 259 

    0x7b098068,// 263 PAY 260 

    0x53085c4e,// 264 PAY 261 

    0xc09a210a,// 265 PAY 262 

    0x3fd5cf21,// 266 PAY 263 

    0x94414940,// 267 PAY 264 

    0x084dfaed,// 268 PAY 265 

    0x7ade8c5f,// 269 PAY 266 

    0x47ae005f,// 270 PAY 267 

    0x8b08cd18,// 271 PAY 268 

    0xe7747e41,// 272 PAY 269 

    0xc434a1a7,// 273 PAY 270 

    0x4be3b573,// 274 PAY 271 

    0x178a4e8b,// 275 PAY 272 

    0xf04a717d,// 276 PAY 273 

    0x41e2c8a8,// 277 PAY 274 

    0x18782cab,// 278 PAY 275 

    0xc4d1fa5b,// 279 PAY 276 

    0x3c9ccfe6,// 280 PAY 277 

    0xf96be32c,// 281 PAY 278 

    0x94cb32c5,// 282 PAY 279 

    0x914d0a12,// 283 PAY 280 

    0x5e746b05,// 284 PAY 281 

    0xa188f8eb,// 285 PAY 282 

    0xed97b84c,// 286 PAY 283 

    0x7e191d23,// 287 PAY 284 

    0xe01e2df2,// 288 PAY 285 

    0xa1a77f68,// 289 PAY 286 

    0x0852fd48,// 290 PAY 287 

    0xf3523617,// 291 PAY 288 

    0x5108f46f,// 292 PAY 289 

    0x7c5e737e,// 293 PAY 290 

    0x59987233,// 294 PAY 291 

    0x777478a4,// 295 PAY 292 

    0x1034605a,// 296 PAY 293 

    0xb796e67e,// 297 PAY 294 

    0x11ab7e7a,// 298 PAY 295 

    0xb23bda64,// 299 PAY 296 

    0x0d81845d,// 300 PAY 297 

    0x0e54bf65,// 301 PAY 298 

    0x180eddc0,// 302 PAY 299 

    0x4357768e,// 303 PAY 300 

    0x13765849,// 304 PAY 301 

    0x26fd3949,// 305 PAY 302 

    0xd5d30d73,// 306 PAY 303 

    0xac3b2c22,// 307 PAY 304 

    0x253ee495,// 308 PAY 305 

    0xbb7689e4,// 309 PAY 306 

    0xfcfd595d,// 310 PAY 307 

    0xcd1055bc,// 311 PAY 308 

    0xed9bc390,// 312 PAY 309 

    0xf5ed6739,// 313 PAY 310 

    0x5cb92cf2,// 314 PAY 311 

    0x33520483,// 315 PAY 312 

    0x9dbbb7fb,// 316 PAY 313 

    0x985a13b9,// 317 PAY 314 

    0x50645dc6,// 318 PAY 315 

    0xe5752864,// 319 PAY 316 

    0x5da47299,// 320 PAY 317 

    0x113a2391,// 321 PAY 318 

    0xc3a565ff,// 322 PAY 319 

    0x046b34b3,// 323 PAY 320 

    0x2dc89e23,// 324 PAY 321 

    0xb5fb9ddb,// 325 PAY 322 

    0xc1ac1bc4,// 326 PAY 323 

    0x46a345ea,// 327 PAY 324 

    0x6f42c6ce,// 328 PAY 325 

    0x54ee0304,// 329 PAY 326 

    0x9c45443e,// 330 PAY 327 

    0xe24c4d24,// 331 PAY 328 

    0x9b588071,// 332 PAY 329 

    0x0b5aeaf5,// 333 PAY 330 

    0x6c056462,// 334 PAY 331 

    0x22abaf9f,// 335 PAY 332 

    0x38e66dbb,// 336 PAY 333 

    0x3167cd72,// 337 PAY 334 

    0x21094064,// 338 PAY 335 

    0xf3a46c21,// 339 PAY 336 

    0x968cbe51,// 340 PAY 337 

    0xa323ea3b,// 341 PAY 338 

    0xacc772f3,// 342 PAY 339 

    0x63bc447f,// 343 PAY 340 

    0x2bdcc64d,// 344 PAY 341 

    0xdc20f80f,// 345 PAY 342 

    0x611e0178,// 346 PAY 343 

    0x1f3aa337,// 347 PAY 344 

    0x2629bcc1,// 348 PAY 345 

    0x62d2fb14,// 349 PAY 346 

    0x384417c2,// 350 PAY 347 

    0x2f5b797c,// 351 PAY 348 

    0x614db4d1,// 352 PAY 349 

    0xb5b061be,// 353 PAY 350 

    0x38dfc035,// 354 PAY 351 

    0x15b05609,// 355 PAY 352 

    0x936a27e1,// 356 PAY 353 

    0x946fd56c,// 357 PAY 354 

    0xe8626c78,// 358 PAY 355 

    0x7c47244f,// 359 PAY 356 

    0x6fd290e0,// 360 PAY 357 

    0xd40df44f,// 361 PAY 358 

    0x41ca1b3b,// 362 PAY 359 

    0xe03d8b15,// 363 PAY 360 

    0x2097f288,// 364 PAY 361 

    0xfb49f753,// 365 PAY 362 

    0x2aa486d3,// 366 PAY 363 

    0xaf55b21f,// 367 PAY 364 

    0x34a39c1b,// 368 PAY 365 

    0xc9d7bedf,// 369 PAY 366 

    0xe443048f,// 370 PAY 367 

    0xa9489540,// 371 PAY 368 

    0x92192063,// 372 PAY 369 

    0x3a5b8a62,// 373 PAY 370 

    0xe60e380b,// 374 PAY 371 

    0x85250d9d,// 375 PAY 372 

    0x145927b4,// 376 PAY 373 

    0xc967e939,// 377 PAY 374 

    0x0b0fcbaa,// 378 PAY 375 

    0x0100daca,// 379 PAY 376 

    0xd86886d4,// 380 PAY 377 

    0x1bdb8650,// 381 PAY 378 

    0x5a7e3775,// 382 PAY 379 

    0xb2bda177,// 383 PAY 380 

    0x276db8c4,// 384 PAY 381 

    0x16c629cd,// 385 PAY 382 

    0x38bee713,// 386 PAY 383 

    0x2556ea74,// 387 PAY 384 

    0x3b48e94c,// 388 PAY 385 

    0x10c8316d,// 389 PAY 386 

    0x3fc92b4d,// 390 PAY 387 

    0x0c88b02e,// 391 PAY 388 

    0xe113bbbf,// 392 PAY 389 

    0x7b0c63b9,// 393 PAY 390 

    0x55f5438e,// 394 PAY 391 

    0x12191ac5,// 395 PAY 392 

    0xa36937d3,// 396 PAY 393 

    0xaa6c6a38,// 397 PAY 394 

    0xa241ce17,// 398 PAY 395 

    0x9e7545c6,// 399 PAY 396 

    0x3d9ced0f,// 400 PAY 397 

    0xab78f534,// 401 PAY 398 

    0xf62d7e8e,// 402 PAY 399 

    0x6457d20b,// 403 PAY 400 

    0x7e2d2322,// 404 PAY 401 

    0x5d3a965e,// 405 PAY 402 

    0x3fd9c0b1,// 406 PAY 403 

    0x12baea22,// 407 PAY 404 

    0x9008b121,// 408 PAY 405 

    0xc1115367,// 409 PAY 406 

    0x69407d77,// 410 PAY 407 

    0xee559f81,// 411 PAY 408 

    0xd32b7c56,// 412 PAY 409 

    0x73746090,// 413 PAY 410 

    0x9cb95a44,// 414 PAY 411 

    0xdd67b5de,// 415 PAY 412 

    0xa3d14182,// 416 PAY 413 

    0xb62945e4,// 417 PAY 414 

    0xb2c53f79,// 418 PAY 415 

    0x4277b122,// 419 PAY 416 

    0x8b043f88,// 420 PAY 417 

    0x90c43bac,// 421 PAY 418 

    0x7b2ea867,// 422 PAY 419 

    0xf27064b6,// 423 PAY 420 

    0x5a1e1a92,// 424 PAY 421 

    0x00dc00b1,// 425 PAY 422 

    0x8f216257,// 426 PAY 423 

    0xa0e3d751,// 427 PAY 424 

    0x5be66629,// 428 PAY 425 

    0xa81dd09a,// 429 PAY 426 

    0x7c5352f4,// 430 PAY 427 

    0xfe11c291,// 431 PAY 428 

    0x1ab0d8fc,// 432 PAY 429 

    0xbdbfac1c,// 433 PAY 430 

    0x24dfadea,// 434 PAY 431 

    0x1284b541,// 435 PAY 432 

    0xc8b4f65e,// 436 PAY 433 

    0x2eb70400,// 437 PAY 434 

    0x207c48d2,// 438 PAY 435 

    0x5d2875e1,// 439 PAY 436 

    0x6f283785,// 440 PAY 437 

    0x830de6f7,// 441 PAY 438 

    0xef0c99c4,// 442 PAY 439 

    0x43a05b6a,// 443 PAY 440 

    0x24aa0f2a,// 444 PAY 441 

    0x0ea68045,// 445 PAY 442 

    0xe8e5dce7,// 446 PAY 443 

    0xeb9fb12d,// 447 PAY 444 

    0xdc9d5720,// 448 PAY 445 

    0xfff8abe7,// 449 PAY 446 

    0xb753aee6,// 450 PAY 447 

    0x01207b90,// 451 PAY 448 

    0x84530973,// 452 PAY 449 

    0x072838b3,// 453 PAY 450 

    0x777a9d5e,// 454 PAY 451 

    0x6b46a34e,// 455 PAY 452 

    0xb4abaddf,// 456 PAY 453 

    0xd3e49609,// 457 PAY 454 

    0x12555f7f,// 458 PAY 455 

    0x572fb596,// 459 PAY 456 

    0x1a230aa1,// 460 PAY 457 

    0x8ee60f57,// 461 PAY 458 

    0xa1404a1e,// 462 PAY 459 

    0x4a53e79a,// 463 PAY 460 

    0x02d908a2,// 464 PAY 461 

    0x44210caf,// 465 PAY 462 

    0x934b7306,// 466 PAY 463 

    0x72c85721,// 467 PAY 464 

    0x7d0ed246,// 468 PAY 465 

    0x5b3f079d,// 469 PAY 466 

    0x1fb7c418,// 470 PAY 467 

    0x3bce26aa,// 471 PAY 468 

    0x39c86f69,// 472 PAY 469 

    0x6e01f1e7,// 473 PAY 470 

    0x1b820798,// 474 PAY 471 

    0x69000000,// 475 PAY 472 

/// HASH is  8 bytes 

    0x1fb7c418,// 476 HSH   1 

    0x3bce26aa,// 477 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 111 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x01f04d6f // 478 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt75_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 50 words. 

/// BDA size     is 195 (0xc3) 

/// BDA id       is 0xec5d 

    0x00c3ec5d,// 3 BDA   1 

/// PAY Generic Data size   : 195 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x94f648b0,// 4 PAY   1 

    0x6b7ad3d1,// 5 PAY   2 

    0x0396f6c2,// 6 PAY   3 

    0x9c2a26d3,// 7 PAY   4 

    0x36c20979,// 8 PAY   5 

    0x81df17a3,// 9 PAY   6 

    0x2dc338c8,// 10 PAY   7 

    0x28371ae6,// 11 PAY   8 

    0xf6d069de,// 12 PAY   9 

    0x543d4d07,// 13 PAY  10 

    0xeb09a2c9,// 14 PAY  11 

    0x76da0d60,// 15 PAY  12 

    0x83120e96,// 16 PAY  13 

    0x1ff63663,// 17 PAY  14 

    0xb84e0529,// 18 PAY  15 

    0x7ae8a415,// 19 PAY  16 

    0xeb9555a2,// 20 PAY  17 

    0x3651f616,// 21 PAY  18 

    0xa6ad2f01,// 22 PAY  19 

    0xee30f979,// 23 PAY  20 

    0xc810231d,// 24 PAY  21 

    0xaeaa55d3,// 25 PAY  22 

    0xb72b4958,// 26 PAY  23 

    0x415ccb89,// 27 PAY  24 

    0x7cc1c485,// 28 PAY  25 

    0x13c62cf7,// 29 PAY  26 

    0x21b8cd5b,// 30 PAY  27 

    0xc45d94bf,// 31 PAY  28 

    0xa6fe71e0,// 32 PAY  29 

    0x405abc36,// 33 PAY  30 

    0x0ad6e8ed,// 34 PAY  31 

    0x31ec6605,// 35 PAY  32 

    0x27f7be31,// 36 PAY  33 

    0x859d2bdb,// 37 PAY  34 

    0xdc92d336,// 38 PAY  35 

    0x6938d1c3,// 39 PAY  36 

    0xef76a541,// 40 PAY  37 

    0xd86df547,// 41 PAY  38 

    0xdea17f8c,// 42 PAY  39 

    0x6425cc81,// 43 PAY  40 

    0x84cdd12e,// 44 PAY  41 

    0xeeed8270,// 45 PAY  42 

    0xe56331f0,// 46 PAY  43 

    0xfea4f5d1,// 47 PAY  44 

    0xaf293ce6,// 48 PAY  45 

    0xe97c44de,// 49 PAY  46 

    0xcd7f4f08,// 50 PAY  47 

    0x2f64b546,// 51 PAY  48 

    0xf6aa6900,// 52 PAY  49 

/// STA is 1 words. 

/// STA num_pkts       : 147 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x96 

    0x02d09693 // 53 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt76_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 368 words. 

/// BDA size     is 1467 (0x5bb) 

/// BDA id       is 0xd76a 

    0x05bbd76a,// 3 BDA   1 

/// PAY Generic Data size   : 1467 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xdcb71168,// 4 PAY   1 

    0xbab8f1bb,// 5 PAY   2 

    0x57d5ee01,// 6 PAY   3 

    0x625ff6e4,// 7 PAY   4 

    0x310f9664,// 8 PAY   5 

    0x7447cbf7,// 9 PAY   6 

    0xa517698b,// 10 PAY   7 

    0x9956d525,// 11 PAY   8 

    0x76be2f85,// 12 PAY   9 

    0x125a84cd,// 13 PAY  10 

    0x1d3d77ec,// 14 PAY  11 

    0x6f9bc390,// 15 PAY  12 

    0xac71c85c,// 16 PAY  13 

    0xb8e80ca5,// 17 PAY  14 

    0x04f74fd6,// 18 PAY  15 

    0x5a3aa35f,// 19 PAY  16 

    0xc19adc5b,// 20 PAY  17 

    0x8f3bc89d,// 21 PAY  18 

    0x16336f0c,// 22 PAY  19 

    0x7a355bae,// 23 PAY  20 

    0x64af0a55,// 24 PAY  21 

    0xfdec35d4,// 25 PAY  22 

    0xab7f2e3e,// 26 PAY  23 

    0xa78cf70c,// 27 PAY  24 

    0xcc1e79f1,// 28 PAY  25 

    0x529c4624,// 29 PAY  26 

    0x30db3a95,// 30 PAY  27 

    0xfe053af0,// 31 PAY  28 

    0xd4f46da3,// 32 PAY  29 

    0xc152c14b,// 33 PAY  30 

    0x69eb9641,// 34 PAY  31 

    0x2c56ce8a,// 35 PAY  32 

    0x29d903f2,// 36 PAY  33 

    0xca58c779,// 37 PAY  34 

    0x5aa1f2f4,// 38 PAY  35 

    0xe83dad58,// 39 PAY  36 

    0xd3747f2d,// 40 PAY  37 

    0x75883634,// 41 PAY  38 

    0x54cfc709,// 42 PAY  39 

    0x465d5fc4,// 43 PAY  40 

    0x02b97bb9,// 44 PAY  41 

    0xe6d04766,// 45 PAY  42 

    0x514593e8,// 46 PAY  43 

    0x6d5a5001,// 47 PAY  44 

    0x9dd92b0f,// 48 PAY  45 

    0x5f839c8c,// 49 PAY  46 

    0xd7463b23,// 50 PAY  47 

    0x7ef7db7e,// 51 PAY  48 

    0xe634d8cd,// 52 PAY  49 

    0xcd48fad5,// 53 PAY  50 

    0xe45e598d,// 54 PAY  51 

    0x89416bce,// 55 PAY  52 

    0xc74e002f,// 56 PAY  53 

    0x4dfece60,// 57 PAY  54 

    0x25b661a2,// 58 PAY  55 

    0x35e2be80,// 59 PAY  56 

    0x506ada4a,// 60 PAY  57 

    0xa89d856e,// 61 PAY  58 

    0x3ed855fc,// 62 PAY  59 

    0xc3bf1397,// 63 PAY  60 

    0xb5dba28a,// 64 PAY  61 

    0x056a8e30,// 65 PAY  62 

    0x937d479e,// 66 PAY  63 

    0x152e9962,// 67 PAY  64 

    0x9b4b158d,// 68 PAY  65 

    0x0b9896b2,// 69 PAY  66 

    0x2da92449,// 70 PAY  67 

    0x7bfb15cf,// 71 PAY  68 

    0x157acbc4,// 72 PAY  69 

    0x44c6e716,// 73 PAY  70 

    0x9ea8e637,// 74 PAY  71 

    0x11c8cb31,// 75 PAY  72 

    0xe4a0b65c,// 76 PAY  73 

    0x115e351f,// 77 PAY  74 

    0x696ae8a6,// 78 PAY  75 

    0x879d449b,// 79 PAY  76 

    0x1b109e06,// 80 PAY  77 

    0xa2a2db90,// 81 PAY  78 

    0x462f6625,// 82 PAY  79 

    0x2a6d7556,// 83 PAY  80 

    0xad789c3e,// 84 PAY  81 

    0x4d790bd0,// 85 PAY  82 

    0xc981d5e9,// 86 PAY  83 

    0x4b719fdf,// 87 PAY  84 

    0xbdcd798e,// 88 PAY  85 

    0x5d1e3ca2,// 89 PAY  86 

    0x40f9a928,// 90 PAY  87 

    0xc390bcdd,// 91 PAY  88 

    0x28e09484,// 92 PAY  89 

    0x43e5ac60,// 93 PAY  90 

    0x4b3ae0de,// 94 PAY  91 

    0xf22eb863,// 95 PAY  92 

    0x3ea6700e,// 96 PAY  93 

    0x5a3b5439,// 97 PAY  94 

    0x7cc8b60f,// 98 PAY  95 

    0xc621085b,// 99 PAY  96 

    0xaf12bf5d,// 100 PAY  97 

    0xcade557b,// 101 PAY  98 

    0xc8d67b64,// 102 PAY  99 

    0x12cdafa1,// 103 PAY 100 

    0x3f687664,// 104 PAY 101 

    0x8053d026,// 105 PAY 102 

    0xc099092e,// 106 PAY 103 

    0x8fbd0cc5,// 107 PAY 104 

    0x5c4ffda1,// 108 PAY 105 

    0x6cad20b4,// 109 PAY 106 

    0xda78d895,// 110 PAY 107 

    0x6566ca53,// 111 PAY 108 

    0x2164dcef,// 112 PAY 109 

    0x7af3b123,// 113 PAY 110 

    0x7fae9c51,// 114 PAY 111 

    0x8591cd70,// 115 PAY 112 

    0x3820fb8e,// 116 PAY 113 

    0x78f1b4a3,// 117 PAY 114 

    0x079e2de7,// 118 PAY 115 

    0xca9431cb,// 119 PAY 116 

    0xb2866964,// 120 PAY 117 

    0xdcf8554b,// 121 PAY 118 

    0xa1d36996,// 122 PAY 119 

    0xa4c631a4,// 123 PAY 120 

    0xd1ecd319,// 124 PAY 121 

    0xff6d64a4,// 125 PAY 122 

    0x168d8bba,// 126 PAY 123 

    0x94fbb9b3,// 127 PAY 124 

    0xc23dfbc0,// 128 PAY 125 

    0xf715b97d,// 129 PAY 126 

    0xe7b29d9f,// 130 PAY 127 

    0x8b1d5ca0,// 131 PAY 128 

    0x3371df66,// 132 PAY 129 

    0xcdacaca4,// 133 PAY 130 

    0x27ba2077,// 134 PAY 131 

    0x1d99d0d5,// 135 PAY 132 

    0x8363f120,// 136 PAY 133 

    0xeffcd87f,// 137 PAY 134 

    0xea2679a9,// 138 PAY 135 

    0x52b8a574,// 139 PAY 136 

    0xc610e4e6,// 140 PAY 137 

    0x19eec8d5,// 141 PAY 138 

    0x201297c8,// 142 PAY 139 

    0xcadf3fb6,// 143 PAY 140 

    0x6276c27a,// 144 PAY 141 

    0x84544b12,// 145 PAY 142 

    0x8300bd45,// 146 PAY 143 

    0x4a369fc3,// 147 PAY 144 

    0x79a703f5,// 148 PAY 145 

    0x988fd9fe,// 149 PAY 146 

    0x1c8ede8a,// 150 PAY 147 

    0xd789560e,// 151 PAY 148 

    0x1d33b639,// 152 PAY 149 

    0xbdcc26c8,// 153 PAY 150 

    0x9300ccce,// 154 PAY 151 

    0x9495d212,// 155 PAY 152 

    0x82fa706d,// 156 PAY 153 

    0xcfc4073a,// 157 PAY 154 

    0x2287b4a8,// 158 PAY 155 

    0x26055e58,// 159 PAY 156 

    0x40165304,// 160 PAY 157 

    0x1e878f35,// 161 PAY 158 

    0x1168e676,// 162 PAY 159 

    0xf3d8c292,// 163 PAY 160 

    0x146ecb79,// 164 PAY 161 

    0xddcbbf52,// 165 PAY 162 

    0x4c5e5a74,// 166 PAY 163 

    0x0b529dbf,// 167 PAY 164 

    0x7ebc9f61,// 168 PAY 165 

    0xb7dee152,// 169 PAY 166 

    0x6c9e37f3,// 170 PAY 167 

    0x708b444d,// 171 PAY 168 

    0xc7ec48b5,// 172 PAY 169 

    0x01a1db0b,// 173 PAY 170 

    0x045e5bc9,// 174 PAY 171 

    0xa03714c9,// 175 PAY 172 

    0x2e01edfc,// 176 PAY 173 

    0x9cb882e7,// 177 PAY 174 

    0x0befb83b,// 178 PAY 175 

    0x4dee41fd,// 179 PAY 176 

    0x0a57dc0d,// 180 PAY 177 

    0x3c01a48e,// 181 PAY 178 

    0xf85a9c08,// 182 PAY 179 

    0x9a023edf,// 183 PAY 180 

    0x4eb7f442,// 184 PAY 181 

    0x3e97151d,// 185 PAY 182 

    0x6115f9f3,// 186 PAY 183 

    0x080ea43d,// 187 PAY 184 

    0x573a408a,// 188 PAY 185 

    0x08241f9d,// 189 PAY 186 

    0x0c1ba14a,// 190 PAY 187 

    0x4f18cd0d,// 191 PAY 188 

    0xbfeb67cc,// 192 PAY 189 

    0xe77664a3,// 193 PAY 190 

    0x534e9848,// 194 PAY 191 

    0xaaf72eb4,// 195 PAY 192 

    0x2f85cdc9,// 196 PAY 193 

    0x3192b929,// 197 PAY 194 

    0xbd14df0a,// 198 PAY 195 

    0x404a3993,// 199 PAY 196 

    0xafb80388,// 200 PAY 197 

    0x1c182eca,// 201 PAY 198 

    0x40b84ebb,// 202 PAY 199 

    0xdd907731,// 203 PAY 200 

    0x608a08c5,// 204 PAY 201 

    0x8dafd6ed,// 205 PAY 202 

    0x142f045c,// 206 PAY 203 

    0xcdcbed1e,// 207 PAY 204 

    0xedf6a461,// 208 PAY 205 

    0x65891603,// 209 PAY 206 

    0x454f1f0b,// 210 PAY 207 

    0x7f503409,// 211 PAY 208 

    0xfbde7e5f,// 212 PAY 209 

    0x935230fe,// 213 PAY 210 

    0x01277f0b,// 214 PAY 211 

    0x6efddc10,// 215 PAY 212 

    0x830a7469,// 216 PAY 213 

    0xb829f93a,// 217 PAY 214 

    0x28255fb1,// 218 PAY 215 

    0x7c887d1b,// 219 PAY 216 

    0x5c75c1cb,// 220 PAY 217 

    0xa335874d,// 221 PAY 218 

    0xd758d5c3,// 222 PAY 219 

    0xff1a2c86,// 223 PAY 220 

    0x92dd1480,// 224 PAY 221 

    0x81d412a6,// 225 PAY 222 

    0x2cc85308,// 226 PAY 223 

    0x812f2d65,// 227 PAY 224 

    0x80734e93,// 228 PAY 225 

    0x9b75e249,// 229 PAY 226 

    0xfe071e25,// 230 PAY 227 

    0xaf6646b9,// 231 PAY 228 

    0x61cf071a,// 232 PAY 229 

    0xb4e15e21,// 233 PAY 230 

    0xae119cc6,// 234 PAY 231 

    0x0ae46ecb,// 235 PAY 232 

    0xc59f929a,// 236 PAY 233 

    0x9e25402f,// 237 PAY 234 

    0x1247c380,// 238 PAY 235 

    0xeb34367e,// 239 PAY 236 

    0x7a22c6aa,// 240 PAY 237 

    0xfa47f032,// 241 PAY 238 

    0x02703359,// 242 PAY 239 

    0xc48349da,// 243 PAY 240 

    0xaf2e7bb4,// 244 PAY 241 

    0xaa3aaa60,// 245 PAY 242 

    0x2f74bac2,// 246 PAY 243 

    0x91ccac29,// 247 PAY 244 

    0xf54f44d5,// 248 PAY 245 

    0xbae23c5a,// 249 PAY 246 

    0x34cd7a28,// 250 PAY 247 

    0x8700f19d,// 251 PAY 248 

    0xbcd2f877,// 252 PAY 249 

    0x37175925,// 253 PAY 250 

    0x04cfdf73,// 254 PAY 251 

    0x0562638e,// 255 PAY 252 

    0x9b3e0207,// 256 PAY 253 

    0xe094e3c7,// 257 PAY 254 

    0xad67c4c5,// 258 PAY 255 

    0x2d8344b0,// 259 PAY 256 

    0xdfeabc5a,// 260 PAY 257 

    0x01ce7cf6,// 261 PAY 258 

    0x4934a073,// 262 PAY 259 

    0xb58a4b75,// 263 PAY 260 

    0xc74c15b6,// 264 PAY 261 

    0xee40a6c4,// 265 PAY 262 

    0x296e92bd,// 266 PAY 263 

    0xa5a8d7b3,// 267 PAY 264 

    0x579b7fa7,// 268 PAY 265 

    0xd4f44efd,// 269 PAY 266 

    0x4d5c1e44,// 270 PAY 267 

    0x7ef776ad,// 271 PAY 268 

    0x3dff29ea,// 272 PAY 269 

    0x7c7fae64,// 273 PAY 270 

    0xbae1540f,// 274 PAY 271 

    0x5dc9c42f,// 275 PAY 272 

    0x71443a6d,// 276 PAY 273 

    0x8d997fa8,// 277 PAY 274 

    0x1eda9222,// 278 PAY 275 

    0x5263bddb,// 279 PAY 276 

    0x21a9be3e,// 280 PAY 277 

    0xbdc6aea4,// 281 PAY 278 

    0xf97edb48,// 282 PAY 279 

    0xaa84707f,// 283 PAY 280 

    0x72d86587,// 284 PAY 281 

    0xd4f82320,// 285 PAY 282 

    0xb9c960a6,// 286 PAY 283 

    0xa9cb221b,// 287 PAY 284 

    0x651f0b2f,// 288 PAY 285 

    0x056dcf9f,// 289 PAY 286 

    0x4c47917e,// 290 PAY 287 

    0x5b3a0c5e,// 291 PAY 288 

    0x47ffab1f,// 292 PAY 289 

    0xd8175afb,// 293 PAY 290 

    0x04476140,// 294 PAY 291 

    0x85f9b020,// 295 PAY 292 

    0x1db35e27,// 296 PAY 293 

    0x72a9e3a8,// 297 PAY 294 

    0x27eb06bc,// 298 PAY 295 

    0x5ebfc441,// 299 PAY 296 

    0xecd048f9,// 300 PAY 297 

    0x037173c4,// 301 PAY 298 

    0x856c289c,// 302 PAY 299 

    0x27df5db0,// 303 PAY 300 

    0x8cee0092,// 304 PAY 301 

    0xe44b3ad0,// 305 PAY 302 

    0x365c1329,// 306 PAY 303 

    0xff382457,// 307 PAY 304 

    0x4afb0eee,// 308 PAY 305 

    0x41859ac5,// 309 PAY 306 

    0x2f3160f7,// 310 PAY 307 

    0x191305e8,// 311 PAY 308 

    0x4e00ea46,// 312 PAY 309 

    0xa2acbf15,// 313 PAY 310 

    0x5ecfdeec,// 314 PAY 311 

    0x00e83d82,// 315 PAY 312 

    0x1599638e,// 316 PAY 313 

    0xdfa9f1eb,// 317 PAY 314 

    0x3ec4f585,// 318 PAY 315 

    0x3e47b551,// 319 PAY 316 

    0x92f53c2b,// 320 PAY 317 

    0x96d2a57c,// 321 PAY 318 

    0x2b5b1bd7,// 322 PAY 319 

    0xdaf2bc51,// 323 PAY 320 

    0xbc935cc2,// 324 PAY 321 

    0xdcc967a6,// 325 PAY 322 

    0xd49c5cdc,// 326 PAY 323 

    0xd2b180b4,// 327 PAY 324 

    0x9127d59b,// 328 PAY 325 

    0x0031e187,// 329 PAY 326 

    0xe7a78cb3,// 330 PAY 327 

    0x33c3be7f,// 331 PAY 328 

    0xc920afdc,// 332 PAY 329 

    0x8169cd65,// 333 PAY 330 

    0xf26870f0,// 334 PAY 331 

    0x256245ee,// 335 PAY 332 

    0x6e87c2d3,// 336 PAY 333 

    0x0c7e4f1d,// 337 PAY 334 

    0xaee75cd0,// 338 PAY 335 

    0xcce57dc4,// 339 PAY 336 

    0x6a9b304e,// 340 PAY 337 

    0x6994b8dd,// 341 PAY 338 

    0xc616d8ff,// 342 PAY 339 

    0xfef3d0cc,// 343 PAY 340 

    0x3ae5304b,// 344 PAY 341 

    0xdb545a6b,// 345 PAY 342 

    0x171a6f04,// 346 PAY 343 

    0x32b5f5f0,// 347 PAY 344 

    0xf1466242,// 348 PAY 345 

    0x5f5ea4c6,// 349 PAY 346 

    0x94eaca01,// 350 PAY 347 

    0x1e100f3b,// 351 PAY 348 

    0xd3816c4a,// 352 PAY 349 

    0x0746d15c,// 353 PAY 350 

    0x1f53a79a,// 354 PAY 351 

    0xd79a854d,// 355 PAY 352 

    0x83cd4161,// 356 PAY 353 

    0xd57bbd2b,// 357 PAY 354 

    0x65368ca0,// 358 PAY 355 

    0xb0586a82,// 359 PAY 356 

    0xcd46b67c,// 360 PAY 357 

    0x6230717e,// 361 PAY 358 

    0x69bb35ae,// 362 PAY 359 

    0x75b59ae3,// 363 PAY 360 

    0x790465ab,// 364 PAY 361 

    0x1eea1cd9,// 365 PAY 362 

    0x26c7670f,// 366 PAY 363 

    0x9d34de4e,// 367 PAY 364 

    0xcbffd58f,// 368 PAY 365 

    0xf679d2f9,// 369 PAY 366 

    0x9be9a200,// 370 PAY 367 

/// STA is 1 words. 

/// STA num_pkts       : 235 

/// STA pkt_idx        : 178 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x02c81ceb // 371 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt77_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 38 words. 

/// BDA size     is 147 (0x93) 

/// BDA id       is 0xaa5 

    0x00930aa5,// 3 BDA   1 

/// PAY Generic Data size   : 147 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x98b0b8e7,// 4 PAY   1 

    0xf49fb0e7,// 5 PAY   2 

    0x6081bcaf,// 6 PAY   3 

    0x6df5f018,// 7 PAY   4 

    0x6e1971cb,// 8 PAY   5 

    0xb0cfbfa8,// 9 PAY   6 

    0x83506234,// 10 PAY   7 

    0xba898c05,// 11 PAY   8 

    0xb54983cb,// 12 PAY   9 

    0x2b81b82c,// 13 PAY  10 

    0x33e3f069,// 14 PAY  11 

    0x00317e1c,// 15 PAY  12 

    0xf4a79466,// 16 PAY  13 

    0x2705e3d6,// 17 PAY  14 

    0x49793e51,// 18 PAY  15 

    0xf66bfc85,// 19 PAY  16 

    0x52fcd4c0,// 20 PAY  17 

    0x8fb3de97,// 21 PAY  18 

    0x9a30fa54,// 22 PAY  19 

    0xda2581ef,// 23 PAY  20 

    0x1650eccc,// 24 PAY  21 

    0xa8c8b39c,// 25 PAY  22 

    0x38355013,// 26 PAY  23 

    0x3311889e,// 27 PAY  24 

    0xdb08c74a,// 28 PAY  25 

    0xa058fa12,// 29 PAY  26 

    0x96b80012,// 30 PAY  27 

    0xc1e412aa,// 31 PAY  28 

    0xe947de80,// 32 PAY  29 

    0x47141fc3,// 33 PAY  30 

    0x44268174,// 34 PAY  31 

    0xa5031aaf,// 35 PAY  32 

    0x98abbe7f,// 36 PAY  33 

    0x14f1743d,// 37 PAY  34 

    0xe9a4a329,// 38 PAY  35 

    0xb52a08e6,// 39 PAY  36 

    0x18eead00,// 40 PAY  37 

/// STA is 1 words. 

/// STA num_pkts       : 254 

/// STA pkt_idx        : 71 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x011dbefe // 41 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt78_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 426 words. 

/// BDA size     is 1698 (0x6a2) 

/// BDA id       is 0xdef3 

    0x06a2def3,// 3 BDA   1 

/// PAY Generic Data size   : 1698 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x66bbf8ae,// 4 PAY   1 

    0x6c9f2a31,// 5 PAY   2 

    0xb68419b5,// 6 PAY   3 

    0x72349853,// 7 PAY   4 

    0x3061dd4e,// 8 PAY   5 

    0xc9ed3787,// 9 PAY   6 

    0xec90d578,// 10 PAY   7 

    0x9aac210f,// 11 PAY   8 

    0x46abaaf7,// 12 PAY   9 

    0x4bd085d1,// 13 PAY  10 

    0x1338b232,// 14 PAY  11 

    0x111be187,// 15 PAY  12 

    0x154e5c58,// 16 PAY  13 

    0x3b2ac451,// 17 PAY  14 

    0x72b6cfce,// 18 PAY  15 

    0x8f286513,// 19 PAY  16 

    0x77f9d5a5,// 20 PAY  17 

    0x50efa950,// 21 PAY  18 

    0xfdeb8472,// 22 PAY  19 

    0x73c399d2,// 23 PAY  20 

    0xfe598f54,// 24 PAY  21 

    0xd1fc3ba4,// 25 PAY  22 

    0x8508972e,// 26 PAY  23 

    0xe34aea4f,// 27 PAY  24 

    0xceb023ca,// 28 PAY  25 

    0xaf93fb4d,// 29 PAY  26 

    0x5d7fe955,// 30 PAY  27 

    0x62058bc5,// 31 PAY  28 

    0x487785e6,// 32 PAY  29 

    0xf3e250d5,// 33 PAY  30 

    0x91ce1654,// 34 PAY  31 

    0x3faa2785,// 35 PAY  32 

    0x525289a3,// 36 PAY  33 

    0xee536ca3,// 37 PAY  34 

    0x87c301ef,// 38 PAY  35 

    0x738b4746,// 39 PAY  36 

    0x41758eb9,// 40 PAY  37 

    0x3453462e,// 41 PAY  38 

    0x10d6a591,// 42 PAY  39 

    0xe7353d38,// 43 PAY  40 

    0x00dbc516,// 44 PAY  41 

    0x43666421,// 45 PAY  42 

    0x0e02ca09,// 46 PAY  43 

    0x626b933d,// 47 PAY  44 

    0x01dbdeea,// 48 PAY  45 

    0xb75a5f3e,// 49 PAY  46 

    0x9f5ae494,// 50 PAY  47 

    0x1815f132,// 51 PAY  48 

    0xaaecf338,// 52 PAY  49 

    0xb9bfe2b9,// 53 PAY  50 

    0xdedf8d50,// 54 PAY  51 

    0xa98732f7,// 55 PAY  52 

    0xf7ee8896,// 56 PAY  53 

    0xa3ab4bf2,// 57 PAY  54 

    0x969641f4,// 58 PAY  55 

    0xe14fe560,// 59 PAY  56 

    0x2606912b,// 60 PAY  57 

    0x84ee38c1,// 61 PAY  58 

    0xaa121dd3,// 62 PAY  59 

    0xb0f532ec,// 63 PAY  60 

    0xf93b7e43,// 64 PAY  61 

    0x7b635909,// 65 PAY  62 

    0x60460fb4,// 66 PAY  63 

    0x6115df7b,// 67 PAY  64 

    0x73fd209a,// 68 PAY  65 

    0x81850ba7,// 69 PAY  66 

    0xd4835109,// 70 PAY  67 

    0x0e6da804,// 71 PAY  68 

    0x149ee413,// 72 PAY  69 

    0x68c31c4a,// 73 PAY  70 

    0x489dda12,// 74 PAY  71 

    0xf071e4ae,// 75 PAY  72 

    0x2c3743d7,// 76 PAY  73 

    0xce2b72a3,// 77 PAY  74 

    0xf2c3b7ff,// 78 PAY  75 

    0xd6cf88c8,// 79 PAY  76 

    0x1156fac5,// 80 PAY  77 

    0x139a93a6,// 81 PAY  78 

    0x35500adb,// 82 PAY  79 

    0x8815da01,// 83 PAY  80 

    0xc8d7ac74,// 84 PAY  81 

    0x048048c1,// 85 PAY  82 

    0x2fea9ab9,// 86 PAY  83 

    0x3c9e5a84,// 87 PAY  84 

    0x8fdf4803,// 88 PAY  85 

    0x3ee831f3,// 89 PAY  86 

    0x16f7d11b,// 90 PAY  87 

    0x4f84a6a0,// 91 PAY  88 

    0xe9f697c7,// 92 PAY  89 

    0xf6b567cb,// 93 PAY  90 

    0xdb346678,// 94 PAY  91 

    0xb306c7e8,// 95 PAY  92 

    0x2d37d2ff,// 96 PAY  93 

    0x3a693469,// 97 PAY  94 

    0xf16c491d,// 98 PAY  95 

    0x73230f9e,// 99 PAY  96 

    0xa291d707,// 100 PAY  97 

    0xc7ec22df,// 101 PAY  98 

    0xdc0c6ea0,// 102 PAY  99 

    0x55e028e6,// 103 PAY 100 

    0x28673027,// 104 PAY 101 

    0x3cc1e942,// 105 PAY 102 

    0x41c85cbb,// 106 PAY 103 

    0x090f390a,// 107 PAY 104 

    0x90372702,// 108 PAY 105 

    0xf28f5049,// 109 PAY 106 

    0x654a6999,// 110 PAY 107 

    0x1ecbe5cc,// 111 PAY 108 

    0xc0724e76,// 112 PAY 109 

    0x6e22a1dc,// 113 PAY 110 

    0xc1ed9981,// 114 PAY 111 

    0xa7efea52,// 115 PAY 112 

    0x96cf739c,// 116 PAY 113 

    0xc13b288c,// 117 PAY 114 

    0x04ea9d60,// 118 PAY 115 

    0x49be2106,// 119 PAY 116 

    0x28b9d47d,// 120 PAY 117 

    0x6a264957,// 121 PAY 118 

    0x6ac4ce7d,// 122 PAY 119 

    0x268f652a,// 123 PAY 120 

    0x11e3b015,// 124 PAY 121 

    0x6eb59a05,// 125 PAY 122 

    0xda577ad9,// 126 PAY 123 

    0x5936557a,// 127 PAY 124 

    0x8b8247dd,// 128 PAY 125 

    0x0d85fbd7,// 129 PAY 126 

    0x0447fbd8,// 130 PAY 127 

    0x9278ea9f,// 131 PAY 128 

    0x7ae760fa,// 132 PAY 129 

    0xc4046d67,// 133 PAY 130 

    0x98d58c79,// 134 PAY 131 

    0xf1af434e,// 135 PAY 132 

    0x5d13a655,// 136 PAY 133 

    0x13475404,// 137 PAY 134 

    0xe4692c87,// 138 PAY 135 

    0xa576fab3,// 139 PAY 136 

    0xae88ad46,// 140 PAY 137 

    0x684fde77,// 141 PAY 138 

    0xee3484a9,// 142 PAY 139 

    0x5396dc79,// 143 PAY 140 

    0xcfdbdac9,// 144 PAY 141 

    0x171cb9c0,// 145 PAY 142 

    0x6e3208b1,// 146 PAY 143 

    0x072a5221,// 147 PAY 144 

    0x1e95ed31,// 148 PAY 145 

    0x14258e3e,// 149 PAY 146 

    0x0d0c38fd,// 150 PAY 147 

    0xb9e7abb1,// 151 PAY 148 

    0x69108537,// 152 PAY 149 

    0xfb647963,// 153 PAY 150 

    0x7e8d3e0a,// 154 PAY 151 

    0x2d83a59a,// 155 PAY 152 

    0xb66010f9,// 156 PAY 153 

    0xb988ed9c,// 157 PAY 154 

    0x665b04f8,// 158 PAY 155 

    0xb6979960,// 159 PAY 156 

    0xc693cccc,// 160 PAY 157 

    0x8cbaf16a,// 161 PAY 158 

    0x2197bd22,// 162 PAY 159 

    0xee16e16f,// 163 PAY 160 

    0xf8ede563,// 164 PAY 161 

    0x60056ffc,// 165 PAY 162 

    0xe3c065ab,// 166 PAY 163 

    0x6a0261af,// 167 PAY 164 

    0x63f9f185,// 168 PAY 165 

    0x9b1fb201,// 169 PAY 166 

    0xdc02be0b,// 170 PAY 167 

    0xa268574d,// 171 PAY 168 

    0x6dcc727c,// 172 PAY 169 

    0x9b7dba99,// 173 PAY 170 

    0xaaa77974,// 174 PAY 171 

    0xe7056dcc,// 175 PAY 172 

    0xc142f433,// 176 PAY 173 

    0x5d3d3b25,// 177 PAY 174 

    0x477c1884,// 178 PAY 175 

    0x5d454950,// 179 PAY 176 

    0x58a77388,// 180 PAY 177 

    0xc811012c,// 181 PAY 178 

    0x377e5f6c,// 182 PAY 179 

    0x5a157db9,// 183 PAY 180 

    0xe8d83c36,// 184 PAY 181 

    0x35ccba16,// 185 PAY 182 

    0x299061bb,// 186 PAY 183 

    0x34579c10,// 187 PAY 184 

    0x2f6a1b08,// 188 PAY 185 

    0x63feee6c,// 189 PAY 186 

    0x611e8232,// 190 PAY 187 

    0x4144a780,// 191 PAY 188 

    0x42743aa4,// 192 PAY 189 

    0xca96ed08,// 193 PAY 190 

    0xc4943131,// 194 PAY 191 

    0x69a774fd,// 195 PAY 192 

    0xa078866e,// 196 PAY 193 

    0xd2d6ae85,// 197 PAY 194 

    0x130d2748,// 198 PAY 195 

    0xa1f0accf,// 199 PAY 196 

    0x3dc58ced,// 200 PAY 197 

    0x46fc5bb6,// 201 PAY 198 

    0x03259c43,// 202 PAY 199 

    0x89e559ed,// 203 PAY 200 

    0x7d6e53bb,// 204 PAY 201 

    0x222be8e8,// 205 PAY 202 

    0x0a3c6025,// 206 PAY 203 

    0xdd6bd3fa,// 207 PAY 204 

    0x996443dd,// 208 PAY 205 

    0x68b91afd,// 209 PAY 206 

    0x12653bb3,// 210 PAY 207 

    0xd4d4f4f7,// 211 PAY 208 

    0xf3b4cb51,// 212 PAY 209 

    0x863a67f9,// 213 PAY 210 

    0x5ecb324e,// 214 PAY 211 

    0x5292ff10,// 215 PAY 212 

    0x78a75cf4,// 216 PAY 213 

    0xb16e9cc7,// 217 PAY 214 

    0x61e40d71,// 218 PAY 215 

    0x8da36cd3,// 219 PAY 216 

    0x6d181471,// 220 PAY 217 

    0x44bde20b,// 221 PAY 218 

    0x2cb55d90,// 222 PAY 219 

    0x2b34dbcd,// 223 PAY 220 

    0x395b3470,// 224 PAY 221 

    0x60f384df,// 225 PAY 222 

    0xc1f9cee5,// 226 PAY 223 

    0x921e992a,// 227 PAY 224 

    0x9239a89b,// 228 PAY 225 

    0x4a78376f,// 229 PAY 226 

    0xe45c670c,// 230 PAY 227 

    0xa631c151,// 231 PAY 228 

    0x850fbee9,// 232 PAY 229 

    0x95449211,// 233 PAY 230 

    0xb06f85bc,// 234 PAY 231 

    0x3ef2f02b,// 235 PAY 232 

    0x6ecd2059,// 236 PAY 233 

    0xa0a2861e,// 237 PAY 234 

    0x9ec3b378,// 238 PAY 235 

    0xad21a664,// 239 PAY 236 

    0x34626ce9,// 240 PAY 237 

    0x82e30293,// 241 PAY 238 

    0xa8723299,// 242 PAY 239 

    0x5eecc034,// 243 PAY 240 

    0xa89ed554,// 244 PAY 241 

    0x2cc95da0,// 245 PAY 242 

    0x9039df63,// 246 PAY 243 

    0x4ac03516,// 247 PAY 244 

    0xab99c47c,// 248 PAY 245 

    0xbc23d964,// 249 PAY 246 

    0x6653c79f,// 250 PAY 247 

    0x8d2da086,// 251 PAY 248 

    0x85f3d840,// 252 PAY 249 

    0x8d0cfc16,// 253 PAY 250 

    0x4dfdc3cd,// 254 PAY 251 

    0xde7500fa,// 255 PAY 252 

    0xaf99ded6,// 256 PAY 253 

    0xc9192356,// 257 PAY 254 

    0xa4413ccf,// 258 PAY 255 

    0xc3406903,// 259 PAY 256 

    0xa6d17343,// 260 PAY 257 

    0xff130a95,// 261 PAY 258 

    0x46f30ef4,// 262 PAY 259 

    0x29e2d2f3,// 263 PAY 260 

    0xca73429f,// 264 PAY 261 

    0x789bf9fc,// 265 PAY 262 

    0xd9e583eb,// 266 PAY 263 

    0x714614fe,// 267 PAY 264 

    0x9aa0fdc0,// 268 PAY 265 

    0xe7ee1e86,// 269 PAY 266 

    0xfebbaf76,// 270 PAY 267 

    0x2605f03d,// 271 PAY 268 

    0xb7f61b18,// 272 PAY 269 

    0xfe7652d5,// 273 PAY 270 

    0x11bc0ee9,// 274 PAY 271 

    0x2347d481,// 275 PAY 272 

    0xb7ec0f89,// 276 PAY 273 

    0x52300b84,// 277 PAY 274 

    0x88a0c548,// 278 PAY 275 

    0xba0f5a79,// 279 PAY 276 

    0x87d5d7f4,// 280 PAY 277 

    0xe0f882cf,// 281 PAY 278 

    0xffe03441,// 282 PAY 279 

    0xb6e3a87f,// 283 PAY 280 

    0xc5ae6e58,// 284 PAY 281 

    0x41972784,// 285 PAY 282 

    0xc0cb4a1d,// 286 PAY 283 

    0x6000fc60,// 287 PAY 284 

    0xaa813884,// 288 PAY 285 

    0xd9758572,// 289 PAY 286 

    0xf701494e,// 290 PAY 287 

    0x5c5a00d1,// 291 PAY 288 

    0x0d5fe768,// 292 PAY 289 

    0x713a3f5d,// 293 PAY 290 

    0xc95f23ac,// 294 PAY 291 

    0xa171071b,// 295 PAY 292 

    0xaae811bd,// 296 PAY 293 

    0xd6f988a7,// 297 PAY 294 

    0xbc68e2d0,// 298 PAY 295 

    0xc47522ea,// 299 PAY 296 

    0xab716307,// 300 PAY 297 

    0x9f277ce3,// 301 PAY 298 

    0x2da17ad6,// 302 PAY 299 

    0x0baa2866,// 303 PAY 300 

    0x3f7ce41e,// 304 PAY 301 

    0x46030ffb,// 305 PAY 302 

    0xf7333779,// 306 PAY 303 

    0x93633c62,// 307 PAY 304 

    0x7bff5a9c,// 308 PAY 305 

    0x1be5606c,// 309 PAY 306 

    0x14b4dc54,// 310 PAY 307 

    0xbbd15fa1,// 311 PAY 308 

    0x8cdab726,// 312 PAY 309 

    0x7cd62fa4,// 313 PAY 310 

    0x6942be7c,// 314 PAY 311 

    0xd425c38a,// 315 PAY 312 

    0xc45d72a8,// 316 PAY 313 

    0xc5b12a16,// 317 PAY 314 

    0x5073df78,// 318 PAY 315 

    0x95da2774,// 319 PAY 316 

    0xc5689718,// 320 PAY 317 

    0xf5028929,// 321 PAY 318 

    0x0550939d,// 322 PAY 319 

    0xa5a4acb0,// 323 PAY 320 

    0x7082752b,// 324 PAY 321 

    0x8970e04e,// 325 PAY 322 

    0x26470073,// 326 PAY 323 

    0x37d99eb8,// 327 PAY 324 

    0xeddaa8cf,// 328 PAY 325 

    0x3c8e15b9,// 329 PAY 326 

    0x9ef9dfac,// 330 PAY 327 

    0x629c7040,// 331 PAY 328 

    0x633ce5f2,// 332 PAY 329 

    0xb7f80cb8,// 333 PAY 330 

    0x54bab774,// 334 PAY 331 

    0x11038156,// 335 PAY 332 

    0xc4c11628,// 336 PAY 333 

    0x672516d2,// 337 PAY 334 

    0x76d20d8d,// 338 PAY 335 

    0xbd196e4b,// 339 PAY 336 

    0xfaded000,// 340 PAY 337 

    0xdef49cb4,// 341 PAY 338 

    0xebbd80ff,// 342 PAY 339 

    0x3c745936,// 343 PAY 340 

    0xf5fe9562,// 344 PAY 341 

    0x435af1e2,// 345 PAY 342 

    0xe2515717,// 346 PAY 343 

    0x2034d27e,// 347 PAY 344 

    0x0edced5f,// 348 PAY 345 

    0xaf665941,// 349 PAY 346 

    0x2233895f,// 350 PAY 347 

    0x033623c5,// 351 PAY 348 

    0xc0aa31f8,// 352 PAY 349 

    0xdeec3b4f,// 353 PAY 350 

    0x6f75a30f,// 354 PAY 351 

    0x760e69ec,// 355 PAY 352 

    0x743082aa,// 356 PAY 353 

    0x397094d9,// 357 PAY 354 

    0x41c36e44,// 358 PAY 355 

    0x6ca14635,// 359 PAY 356 

    0xaf4cc738,// 360 PAY 357 

    0x1dc555da,// 361 PAY 358 

    0xbcf88082,// 362 PAY 359 

    0x040dd611,// 363 PAY 360 

    0xb4b910f5,// 364 PAY 361 

    0x3a5a8e3c,// 365 PAY 362 

    0x078c118e,// 366 PAY 363 

    0xbe6c4302,// 367 PAY 364 

    0xfbb658bd,// 368 PAY 365 

    0xb95197ee,// 369 PAY 366 

    0x2824354a,// 370 PAY 367 

    0x5099332b,// 371 PAY 368 

    0x2dab1b3b,// 372 PAY 369 

    0x6b133c95,// 373 PAY 370 

    0x2092cf93,// 374 PAY 371 

    0xb0d4c426,// 375 PAY 372 

    0xc46f2c0e,// 376 PAY 373 

    0x713b04cb,// 377 PAY 374 

    0x773a52e8,// 378 PAY 375 

    0x8a24da9e,// 379 PAY 376 

    0x41ff7065,// 380 PAY 377 

    0x33f0ccb1,// 381 PAY 378 

    0xc4308679,// 382 PAY 379 

    0x7484add5,// 383 PAY 380 

    0x17fe5e64,// 384 PAY 381 

    0x3583a083,// 385 PAY 382 

    0x4d81a68f,// 386 PAY 383 

    0xe74c29db,// 387 PAY 384 

    0xe5e2e467,// 388 PAY 385 

    0xe72da5c4,// 389 PAY 386 

    0x2125cb03,// 390 PAY 387 

    0xcb6e75cb,// 391 PAY 388 

    0x9a6624bb,// 392 PAY 389 

    0xe5dd3f05,// 393 PAY 390 

    0xd5e2fe60,// 394 PAY 391 

    0x17179d60,// 395 PAY 392 

    0x649b74f2,// 396 PAY 393 

    0x837982c9,// 397 PAY 394 

    0xdeaf13cc,// 398 PAY 395 

    0x5c822c41,// 399 PAY 396 

    0x144892c1,// 400 PAY 397 

    0x13d1ba63,// 401 PAY 398 

    0x2ebfe02f,// 402 PAY 399 

    0xf6587f3d,// 403 PAY 400 

    0x5b7d3a09,// 404 PAY 401 

    0xeec3cbf1,// 405 PAY 402 

    0xdeb2b49f,// 406 PAY 403 

    0x993a5ce3,// 407 PAY 404 

    0xd0ddb538,// 408 PAY 405 

    0x17c5f698,// 409 PAY 406 

    0xceb0c676,// 410 PAY 407 

    0x6113ddcd,// 411 PAY 408 

    0xab5badec,// 412 PAY 409 

    0x1e709647,// 413 PAY 410 

    0x9cdab431,// 414 PAY 411 

    0xaa22132a,// 415 PAY 412 

    0x544efb78,// 416 PAY 413 

    0xa1a822d8,// 417 PAY 414 

    0x2d4567d8,// 418 PAY 415 

    0xfdadcfa0,// 419 PAY 416 

    0x30b99a16,// 420 PAY 417 

    0xc3192f3a,// 421 PAY 418 

    0x54782374,// 422 PAY 419 

    0xaac38ab6,// 423 PAY 420 

    0x0de313d7,// 424 PAY 421 

    0x1835f86c,// 425 PAY 422 

    0x920af321,// 426 PAY 423 

    0x7782e009,// 427 PAY 424 

    0xe8390000,// 428 PAY 425 

/// HASH is  4 bytes 

    0x6653c79f,// 429 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 168 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf4 

    0x02a0f437 // 430 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt79_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 260 words. 

/// BDA size     is 1036 (0x40c) 

/// BDA id       is 0x4d3 

    0x040c04d3,// 3 BDA   1 

/// PAY Generic Data size   : 1036 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x47abbdde,// 4 PAY   1 

    0x42ca9f58,// 5 PAY   2 

    0x6cef5265,// 6 PAY   3 

    0x54a9aabe,// 7 PAY   4 

    0x9bb72479,// 8 PAY   5 

    0x64ed60da,// 9 PAY   6 

    0xddf3b35f,// 10 PAY   7 

    0x9480271c,// 11 PAY   8 

    0x6f8b9959,// 12 PAY   9 

    0x4f59ec9f,// 13 PAY  10 

    0xbc5145fe,// 14 PAY  11 

    0x8cb55043,// 15 PAY  12 

    0x89317868,// 16 PAY  13 

    0x836899a6,// 17 PAY  14 

    0xa891d6ae,// 18 PAY  15 

    0x0b86cce6,// 19 PAY  16 

    0xe9eb057b,// 20 PAY  17 

    0x6b0bffba,// 21 PAY  18 

    0xc9fc98de,// 22 PAY  19 

    0xa3de6a84,// 23 PAY  20 

    0x92a91bb6,// 24 PAY  21 

    0xc50a3d44,// 25 PAY  22 

    0x1997fdda,// 26 PAY  23 

    0x88272f7b,// 27 PAY  24 

    0xf6883a3b,// 28 PAY  25 

    0xd15af502,// 29 PAY  26 

    0x4a971ae3,// 30 PAY  27 

    0xb805b529,// 31 PAY  28 

    0xaa10e06b,// 32 PAY  29 

    0x710610f6,// 33 PAY  30 

    0x3436aec4,// 34 PAY  31 

    0xdd50ccf5,// 35 PAY  32 

    0x4ec43d8e,// 36 PAY  33 

    0x71d3ba3a,// 37 PAY  34 

    0x63e8e08e,// 38 PAY  35 

    0xa8f3a476,// 39 PAY  36 

    0x9f962bd8,// 40 PAY  37 

    0xfa233a8d,// 41 PAY  38 

    0xc2da2794,// 42 PAY  39 

    0x0cee2ec5,// 43 PAY  40 

    0xa1de9b66,// 44 PAY  41 

    0xe2c30d53,// 45 PAY  42 

    0x7614c910,// 46 PAY  43 

    0xe62497ee,// 47 PAY  44 

    0xb4995084,// 48 PAY  45 

    0x35c15f31,// 49 PAY  46 

    0x7017f65b,// 50 PAY  47 

    0xed7c2e18,// 51 PAY  48 

    0xd3dc05e4,// 52 PAY  49 

    0x5fb695ab,// 53 PAY  50 

    0xa8c7062e,// 54 PAY  51 

    0x8baa42bc,// 55 PAY  52 

    0x418d1931,// 56 PAY  53 

    0xfad492cc,// 57 PAY  54 

    0xeb9f3ebe,// 58 PAY  55 

    0x143f8810,// 59 PAY  56 

    0xb9d5270d,// 60 PAY  57 

    0xe0615aeb,// 61 PAY  58 

    0xeb30cc52,// 62 PAY  59 

    0x1486cf9a,// 63 PAY  60 

    0xf00c27fb,// 64 PAY  61 

    0x78c2d862,// 65 PAY  62 

    0x3585e71f,// 66 PAY  63 

    0x508a3576,// 67 PAY  64 

    0x730218d1,// 68 PAY  65 

    0x317ab156,// 69 PAY  66 

    0x7f10e41b,// 70 PAY  67 

    0x36c02d08,// 71 PAY  68 

    0xe016a3eb,// 72 PAY  69 

    0x7e44bf63,// 73 PAY  70 

    0x6e253d43,// 74 PAY  71 

    0x96da5ac1,// 75 PAY  72 

    0x81ea9a2d,// 76 PAY  73 

    0x3b57abd2,// 77 PAY  74 

    0x2a02f90a,// 78 PAY  75 

    0x8f224017,// 79 PAY  76 

    0xc18144c3,// 80 PAY  77 

    0xe0ada1ad,// 81 PAY  78 

    0x3c52aac9,// 82 PAY  79 

    0x39adc753,// 83 PAY  80 

    0xa7c5f334,// 84 PAY  81 

    0x3e3eac07,// 85 PAY  82 

    0xfa9ac5df,// 86 PAY  83 

    0x9d4abf1a,// 87 PAY  84 

    0xa063a1e8,// 88 PAY  85 

    0x8ed1e4c3,// 89 PAY  86 

    0x4f2b5333,// 90 PAY  87 

    0xd1018462,// 91 PAY  88 

    0x3cbec1d5,// 92 PAY  89 

    0xdbca9d94,// 93 PAY  90 

    0x24da9978,// 94 PAY  91 

    0x58b77315,// 95 PAY  92 

    0x097ee72f,// 96 PAY  93 

    0x8d8594b7,// 97 PAY  94 

    0x0bd23b88,// 98 PAY  95 

    0x602dbd28,// 99 PAY  96 

    0x5e8169f3,// 100 PAY  97 

    0xe9bd9bdf,// 101 PAY  98 

    0x7ca98766,// 102 PAY  99 

    0x45a5760f,// 103 PAY 100 

    0x7b6d9c58,// 104 PAY 101 

    0x6007e43b,// 105 PAY 102 

    0x1d1e59a0,// 106 PAY 103 

    0x31ad82d9,// 107 PAY 104 

    0xb4cc9780,// 108 PAY 105 

    0xa35ca08f,// 109 PAY 106 

    0x245fbac0,// 110 PAY 107 

    0xd4f5a7e6,// 111 PAY 108 

    0x07650e70,// 112 PAY 109 

    0x23d959ae,// 113 PAY 110 

    0x43e514dc,// 114 PAY 111 

    0x53d9b803,// 115 PAY 112 

    0xea3e8260,// 116 PAY 113 

    0xd4bccb99,// 117 PAY 114 

    0x622489eb,// 118 PAY 115 

    0x6c345899,// 119 PAY 116 

    0x72015947,// 120 PAY 117 

    0x24d6051d,// 121 PAY 118 

    0xc8b1d46e,// 122 PAY 119 

    0xbad712e4,// 123 PAY 120 

    0x2241c559,// 124 PAY 121 

    0x48f63325,// 125 PAY 122 

    0x265d3696,// 126 PAY 123 

    0x1964dd75,// 127 PAY 124 

    0x6f372d98,// 128 PAY 125 

    0x6d2bd9c0,// 129 PAY 126 

    0x937062ca,// 130 PAY 127 

    0x018cf098,// 131 PAY 128 

    0xa44638b5,// 132 PAY 129 

    0x9c0ac7b9,// 133 PAY 130 

    0x924983b0,// 134 PAY 131 

    0xc8b555a4,// 135 PAY 132 

    0xcc5f6e96,// 136 PAY 133 

    0xcb929aa9,// 137 PAY 134 

    0xf9f95c8d,// 138 PAY 135 

    0xaf86cf1a,// 139 PAY 136 

    0x7c20c8b1,// 140 PAY 137 

    0xb60d0c2b,// 141 PAY 138 

    0x554c8410,// 142 PAY 139 

    0xe312308b,// 143 PAY 140 

    0xa22332cb,// 144 PAY 141 

    0x46a0299d,// 145 PAY 142 

    0xbedcfcad,// 146 PAY 143 

    0xb4988ca4,// 147 PAY 144 

    0xe92bd7de,// 148 PAY 145 

    0xe07436d1,// 149 PAY 146 

    0x7fffcbe2,// 150 PAY 147 

    0xef573017,// 151 PAY 148 

    0x080d119e,// 152 PAY 149 

    0xf9ff2299,// 153 PAY 150 

    0x459a199d,// 154 PAY 151 

    0x48c20eb7,// 155 PAY 152 

    0x257d77a8,// 156 PAY 153 

    0xbaf3f9e7,// 157 PAY 154 

    0x62f1ba5a,// 158 PAY 155 

    0xffb52354,// 159 PAY 156 

    0xa2b300ee,// 160 PAY 157 

    0x7e71d928,// 161 PAY 158 

    0x57a653fc,// 162 PAY 159 

    0x7e6cdb11,// 163 PAY 160 

    0x8f31c6e8,// 164 PAY 161 

    0x8ea549a4,// 165 PAY 162 

    0x6a3709d6,// 166 PAY 163 

    0x50139ec4,// 167 PAY 164 

    0x1a7d2bb8,// 168 PAY 165 

    0x38011b5c,// 169 PAY 166 

    0xe2d84199,// 170 PAY 167 

    0x6c8b92d9,// 171 PAY 168 

    0x18350fe7,// 172 PAY 169 

    0x4866d127,// 173 PAY 170 

    0x6cf89005,// 174 PAY 171 

    0x90eb8da2,// 175 PAY 172 

    0xf86e69d4,// 176 PAY 173 

    0x5ef1e04c,// 177 PAY 174 

    0x2017db81,// 178 PAY 175 

    0x89c25ba0,// 179 PAY 176 

    0xbfbe7e29,// 180 PAY 177 

    0x548b55c6,// 181 PAY 178 

    0xeb2328ba,// 182 PAY 179 

    0xaa31d7ca,// 183 PAY 180 

    0x024a5347,// 184 PAY 181 

    0x5973ad06,// 185 PAY 182 

    0xc8fe7431,// 186 PAY 183 

    0x4507565c,// 187 PAY 184 

    0x7098f236,// 188 PAY 185 

    0x63d52ec1,// 189 PAY 186 

    0x522d50ad,// 190 PAY 187 

    0x5217bac8,// 191 PAY 188 

    0x755afa36,// 192 PAY 189 

    0x8c76222e,// 193 PAY 190 

    0x87a51bfa,// 194 PAY 191 

    0x97cbb919,// 195 PAY 192 

    0x86a6963c,// 196 PAY 193 

    0xd0a80200,// 197 PAY 194 

    0xd3ac23b2,// 198 PAY 195 

    0x58a197e2,// 199 PAY 196 

    0x9b1f2ff7,// 200 PAY 197 

    0x3a015d76,// 201 PAY 198 

    0x11c9e1a2,// 202 PAY 199 

    0x9d69b389,// 203 PAY 200 

    0x6a57ac3e,// 204 PAY 201 

    0xaee271f0,// 205 PAY 202 

    0x872f0268,// 206 PAY 203 

    0xda8af501,// 207 PAY 204 

    0x49de5e7a,// 208 PAY 205 

    0x4aa2722f,// 209 PAY 206 

    0xcf4cc5a3,// 210 PAY 207 

    0x2e6034f9,// 211 PAY 208 

    0x39e5705a,// 212 PAY 209 

    0xe9709a37,// 213 PAY 210 

    0x290729e2,// 214 PAY 211 

    0x7d615457,// 215 PAY 212 

    0x3dbb0e7f,// 216 PAY 213 

    0xe132d046,// 217 PAY 214 

    0x9e63180c,// 218 PAY 215 

    0xf2b0f9a7,// 219 PAY 216 

    0x2977d27d,// 220 PAY 217 

    0x37d0d4ac,// 221 PAY 218 

    0x9d1a77fb,// 222 PAY 219 

    0xbaabdb00,// 223 PAY 220 

    0xe75740ac,// 224 PAY 221 

    0x1a0f9da9,// 225 PAY 222 

    0x588fa845,// 226 PAY 223 

    0x5ca3538b,// 227 PAY 224 

    0x7fe34e16,// 228 PAY 225 

    0xed5286e7,// 229 PAY 226 

    0xb7edc7ab,// 230 PAY 227 

    0x0d6ec339,// 231 PAY 228 

    0x3b4d33c0,// 232 PAY 229 

    0xf2ed4944,// 233 PAY 230 

    0x59f6231c,// 234 PAY 231 

    0x41e7e449,// 235 PAY 232 

    0x232bc02f,// 236 PAY 233 

    0x774fc4c5,// 237 PAY 234 

    0x7af5da5a,// 238 PAY 235 

    0x64080f97,// 239 PAY 236 

    0x94249d76,// 240 PAY 237 

    0x59b79e8e,// 241 PAY 238 

    0xdfa118e3,// 242 PAY 239 

    0xa3039a5e,// 243 PAY 240 

    0x71a97935,// 244 PAY 241 

    0xe6b41db9,// 245 PAY 242 

    0xdb8b7600,// 246 PAY 243 

    0x22055660,// 247 PAY 244 

    0x1a3a3e40,// 248 PAY 245 

    0x6b2fcb9b,// 249 PAY 246 

    0xa5f5e50d,// 250 PAY 247 

    0x0b97dabc,// 251 PAY 248 

    0x40269ca6,// 252 PAY 249 

    0xbc5c691f,// 253 PAY 250 

    0x84b679f6,// 254 PAY 251 

    0x75352ea1,// 255 PAY 252 

    0x6c0b279d,// 256 PAY 253 

    0x2b06bf3a,// 257 PAY 254 

    0x801c45bf,// 258 PAY 255 

    0x0f16b10e,// 259 PAY 256 

    0x4de8bc8f,// 260 PAY 257 

    0x63df05bf,// 261 PAY 258 

    0x2ba1bdda,// 262 PAY 259 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf 

    0x01690f2d // 263 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt80_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 200 words. 

/// BDA size     is 796 (0x31c) 

/// BDA id       is 0xcf5d 

    0x031ccf5d,// 3 BDA   1 

/// PAY Generic Data size   : 796 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x6aec1cf3,// 4 PAY   1 

    0x1104cb59,// 5 PAY   2 

    0xafa848ba,// 6 PAY   3 

    0xafbe4e1c,// 7 PAY   4 

    0xa358398a,// 8 PAY   5 

    0x480fdd6b,// 9 PAY   6 

    0xd614d1c8,// 10 PAY   7 

    0x18d7a876,// 11 PAY   8 

    0x5f23aaf1,// 12 PAY   9 

    0x6b141924,// 13 PAY  10 

    0x0aa0ea66,// 14 PAY  11 

    0xc591c2f3,// 15 PAY  12 

    0x3cb01aae,// 16 PAY  13 

    0x809b39e4,// 17 PAY  14 

    0x7c36cec2,// 18 PAY  15 

    0x2f866458,// 19 PAY  16 

    0x1cb3ea1a,// 20 PAY  17 

    0x5844b91c,// 21 PAY  18 

    0xfb8ea100,// 22 PAY  19 

    0x5c4c4e36,// 23 PAY  20 

    0x621c063a,// 24 PAY  21 

    0x8cdd9a48,// 25 PAY  22 

    0x88205c4d,// 26 PAY  23 

    0xbd85dedf,// 27 PAY  24 

    0xc6f7ad66,// 28 PAY  25 

    0x1a33e21d,// 29 PAY  26 

    0xdd97a041,// 30 PAY  27 

    0xaa067ac8,// 31 PAY  28 

    0x703f4bda,// 32 PAY  29 

    0x2cc515e9,// 33 PAY  30 

    0x8f1396a7,// 34 PAY  31 

    0x15bc3ac6,// 35 PAY  32 

    0xe9dd7ea8,// 36 PAY  33 

    0xa8d2d7c1,// 37 PAY  34 

    0x4b90037c,// 38 PAY  35 

    0xcff06688,// 39 PAY  36 

    0xba51025a,// 40 PAY  37 

    0x28f24c2c,// 41 PAY  38 

    0xb6c1461b,// 42 PAY  39 

    0x90dd9497,// 43 PAY  40 

    0x21a497a0,// 44 PAY  41 

    0x8994f48e,// 45 PAY  42 

    0x479e2152,// 46 PAY  43 

    0x7ad6e914,// 47 PAY  44 

    0x37a4f89f,// 48 PAY  45 

    0x161fff15,// 49 PAY  46 

    0x4febdd0c,// 50 PAY  47 

    0x7a921b2a,// 51 PAY  48 

    0xf81906f1,// 52 PAY  49 

    0x9ce8318e,// 53 PAY  50 

    0xe3e93aa7,// 54 PAY  51 

    0xf6f4fe48,// 55 PAY  52 

    0xd50f9b7b,// 56 PAY  53 

    0x950f6b98,// 57 PAY  54 

    0x59f83167,// 58 PAY  55 

    0xf33a362e,// 59 PAY  56 

    0x2ad720ca,// 60 PAY  57 

    0xbdd3e805,// 61 PAY  58 

    0x2a12362c,// 62 PAY  59 

    0x332797c1,// 63 PAY  60 

    0xca0323a5,// 64 PAY  61 

    0x9869a22c,// 65 PAY  62 

    0x37949fce,// 66 PAY  63 

    0x747012f8,// 67 PAY  64 

    0xe2752454,// 68 PAY  65 

    0x7ca15ab7,// 69 PAY  66 

    0x3dd80786,// 70 PAY  67 

    0xdb4e701f,// 71 PAY  68 

    0x030495eb,// 72 PAY  69 

    0xf6e72453,// 73 PAY  70 

    0x16619c7f,// 74 PAY  71 

    0x13825497,// 75 PAY  72 

    0x4523868c,// 76 PAY  73 

    0x3e9896be,// 77 PAY  74 

    0xa438be40,// 78 PAY  75 

    0x792fdde9,// 79 PAY  76 

    0xa14f1635,// 80 PAY  77 

    0xf8108df2,// 81 PAY  78 

    0xc9a7cc97,// 82 PAY  79 

    0x7c7ef94b,// 83 PAY  80 

    0x7ee154a6,// 84 PAY  81 

    0x0ca1a1b5,// 85 PAY  82 

    0x18295151,// 86 PAY  83 

    0x653ee6b0,// 87 PAY  84 

    0x87a96946,// 88 PAY  85 

    0xf6ee4984,// 89 PAY  86 

    0xad570cb0,// 90 PAY  87 

    0x7cd3d0f9,// 91 PAY  88 

    0xdd2feb8a,// 92 PAY  89 

    0x6421ef9e,// 93 PAY  90 

    0x5467c010,// 94 PAY  91 

    0xec8c0cee,// 95 PAY  92 

    0xe0a8b0f2,// 96 PAY  93 

    0x2e8e058d,// 97 PAY  94 

    0xa70b342d,// 98 PAY  95 

    0x974a3dbe,// 99 PAY  96 

    0xe534dc77,// 100 PAY  97 

    0xa2956035,// 101 PAY  98 

    0x32d411b1,// 102 PAY  99 

    0x0792edff,// 103 PAY 100 

    0x7e1d2157,// 104 PAY 101 

    0x167ede4b,// 105 PAY 102 

    0x82c12579,// 106 PAY 103 

    0xe5f0e9b7,// 107 PAY 104 

    0xe5679844,// 108 PAY 105 

    0x71561f0b,// 109 PAY 106 

    0x96b0cf60,// 110 PAY 107 

    0x922cc734,// 111 PAY 108 

    0x3df9d972,// 112 PAY 109 

    0x9f2523fe,// 113 PAY 110 

    0xc0dc78db,// 114 PAY 111 

    0xb687f5f0,// 115 PAY 112 

    0xf495194e,// 116 PAY 113 

    0x1e14d156,// 117 PAY 114 

    0xe853253f,// 118 PAY 115 

    0x2e30bae3,// 119 PAY 116 

    0x75a7d642,// 120 PAY 117 

    0x4856bf40,// 121 PAY 118 

    0x4e828852,// 122 PAY 119 

    0xc00c2bb5,// 123 PAY 120 

    0x9f013d87,// 124 PAY 121 

    0xd386b89a,// 125 PAY 122 

    0x331e3f27,// 126 PAY 123 

    0x9d83da5c,// 127 PAY 124 

    0x4c3a2059,// 128 PAY 125 

    0x2bbdb279,// 129 PAY 126 

    0xea326bc7,// 130 PAY 127 

    0x0f9a702e,// 131 PAY 128 

    0xa98f1905,// 132 PAY 129 

    0x4e493381,// 133 PAY 130 

    0x29813f00,// 134 PAY 131 

    0xdf3477ea,// 135 PAY 132 

    0x94bc7e15,// 136 PAY 133 

    0xce0866c2,// 137 PAY 134 

    0x7f9392c7,// 138 PAY 135 

    0xb865ac53,// 139 PAY 136 

    0x6fc18f5c,// 140 PAY 137 

    0x2a00af5b,// 141 PAY 138 

    0x064d91d0,// 142 PAY 139 

    0xba474c0b,// 143 PAY 140 

    0xd1852ae6,// 144 PAY 141 

    0x3cae2e3b,// 145 PAY 142 

    0x9a207571,// 146 PAY 143 

    0xdfafa284,// 147 PAY 144 

    0x88bfd2fd,// 148 PAY 145 

    0x3cfda6d4,// 149 PAY 146 

    0x62bb7f2d,// 150 PAY 147 

    0xa2ef8256,// 151 PAY 148 

    0x9725e4ce,// 152 PAY 149 

    0xb625ebb3,// 153 PAY 150 

    0x855efbef,// 154 PAY 151 

    0xbd57465b,// 155 PAY 152 

    0xe8c62ecc,// 156 PAY 153 

    0xbbd8f474,// 157 PAY 154 

    0x0ede70b0,// 158 PAY 155 

    0x8037aa34,// 159 PAY 156 

    0xbfe763f7,// 160 PAY 157 

    0xd82c6041,// 161 PAY 158 

    0x9cb77aaf,// 162 PAY 159 

    0x6b07030f,// 163 PAY 160 

    0x71a9e0c1,// 164 PAY 161 

    0xfc16af52,// 165 PAY 162 

    0x90fad0e0,// 166 PAY 163 

    0x404d7117,// 167 PAY 164 

    0xfcad76ed,// 168 PAY 165 

    0x0e52ac3c,// 169 PAY 166 

    0xa39cfa71,// 170 PAY 167 

    0x1a06ba1d,// 171 PAY 168 

    0x61171d94,// 172 PAY 169 

    0x381136e9,// 173 PAY 170 

    0xe4c8bf67,// 174 PAY 171 

    0x2ac84ddb,// 175 PAY 172 

    0xf050aa7d,// 176 PAY 173 

    0xeae7fab5,// 177 PAY 174 

    0x72448868,// 178 PAY 175 

    0x8c566ade,// 179 PAY 176 

    0x3304b6b2,// 180 PAY 177 

    0x63f8bbf3,// 181 PAY 178 

    0xd7294ebf,// 182 PAY 179 

    0xed496946,// 183 PAY 180 

    0x76e17145,// 184 PAY 181 

    0xdd795993,// 185 PAY 182 

    0xcb109386,// 186 PAY 183 

    0x7cd5471a,// 187 PAY 184 

    0x81f0705c,// 188 PAY 185 

    0x8e999e2a,// 189 PAY 186 

    0x9e947300,// 190 PAY 187 

    0x33fb580d,// 191 PAY 188 

    0xd81fe3bf,// 192 PAY 189 

    0x6959841b,// 193 PAY 190 

    0x14c428db,// 194 PAY 191 

    0x12fca66c,// 195 PAY 192 

    0x249250c0,// 196 PAY 193 

    0x7a63746f,// 197 PAY 194 

    0xeb90817e,// 198 PAY 195 

    0xa6fd5a28,// 199 PAY 196 

    0x3f954111,// 200 PAY 197 

    0x666c5738,// 201 PAY 198 

    0xc89f41cc,// 202 PAY 199 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 255 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x03fc0485 // 203 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt81_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 293 words. 

/// BDA size     is 1168 (0x490) 

/// BDA id       is 0xe599 

    0x0490e599,// 3 BDA   1 

/// PAY Generic Data size   : 1168 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x42b5993a,// 4 PAY   1 

    0x1e6551e9,// 5 PAY   2 

    0xcba0a303,// 6 PAY   3 

    0x1b6a29a4,// 7 PAY   4 

    0x24814d27,// 8 PAY   5 

    0xff2ddce5,// 9 PAY   6 

    0x6b9c5b6e,// 10 PAY   7 

    0x42ce5930,// 11 PAY   8 

    0x903048a5,// 12 PAY   9 

    0xd4bac7d6,// 13 PAY  10 

    0x70a0b83d,// 14 PAY  11 

    0x96dc2828,// 15 PAY  12 

    0x78d1733c,// 16 PAY  13 

    0x0f4af923,// 17 PAY  14 

    0xe892aa0e,// 18 PAY  15 

    0xf708a57c,// 19 PAY  16 

    0x7787b579,// 20 PAY  17 

    0x36e4218d,// 21 PAY  18 

    0xa0aa2b15,// 22 PAY  19 

    0x6a7fd9ef,// 23 PAY  20 

    0x9a97b9ac,// 24 PAY  21 

    0x02a9b47c,// 25 PAY  22 

    0xbbbbac6b,// 26 PAY  23 

    0x5fe8ff30,// 27 PAY  24 

    0xd560f6bf,// 28 PAY  25 

    0x8b66d100,// 29 PAY  26 

    0xe96a19c0,// 30 PAY  27 

    0x51078bb5,// 31 PAY  28 

    0xea2651b0,// 32 PAY  29 

    0x0bd2c585,// 33 PAY  30 

    0x57e4e379,// 34 PAY  31 

    0x239c1167,// 35 PAY  32 

    0x157d14d5,// 36 PAY  33 

    0x17b08f07,// 37 PAY  34 

    0x00fd8b2d,// 38 PAY  35 

    0x9e483079,// 39 PAY  36 

    0x7c373aa8,// 40 PAY  37 

    0x17f447c1,// 41 PAY  38 

    0x4a0ee021,// 42 PAY  39 

    0x6858a7dd,// 43 PAY  40 

    0x5e73874f,// 44 PAY  41 

    0xe438b112,// 45 PAY  42 

    0x9080980d,// 46 PAY  43 

    0xab0b1fac,// 47 PAY  44 

    0x69094b5e,// 48 PAY  45 

    0x5f124273,// 49 PAY  46 

    0x9e78029f,// 50 PAY  47 

    0x6d634055,// 51 PAY  48 

    0xa2b21bdf,// 52 PAY  49 

    0x9297a65f,// 53 PAY  50 

    0xc2dcbf2b,// 54 PAY  51 

    0xce4c57c6,// 55 PAY  52 

    0x8ab2796f,// 56 PAY  53 

    0x3f19c5ea,// 57 PAY  54 

    0x39d7f4d1,// 58 PAY  55 

    0x65195c72,// 59 PAY  56 

    0x1db263fc,// 60 PAY  57 

    0x2a3e856e,// 61 PAY  58 

    0x31e5f23b,// 62 PAY  59 

    0x95eb6adb,// 63 PAY  60 

    0x0942e7a1,// 64 PAY  61 

    0xc534d88c,// 65 PAY  62 

    0xe8c34a88,// 66 PAY  63 

    0x35cd6497,// 67 PAY  64 

    0x57a40935,// 68 PAY  65 

    0x9d39e3f3,// 69 PAY  66 

    0xfa1fb6db,// 70 PAY  67 

    0x107dc17e,// 71 PAY  68 

    0xd348cb50,// 72 PAY  69 

    0x442e25e8,// 73 PAY  70 

    0xd736e9ac,// 74 PAY  71 

    0xd5020fdf,// 75 PAY  72 

    0x8cfa6e8f,// 76 PAY  73 

    0x30da5818,// 77 PAY  74 

    0x726309de,// 78 PAY  75 

    0xbf14593f,// 79 PAY  76 

    0x2defe2d0,// 80 PAY  77 

    0x2e399f9d,// 81 PAY  78 

    0x6d7e6650,// 82 PAY  79 

    0x491e2635,// 83 PAY  80 

    0xb79dab7e,// 84 PAY  81 

    0x90eb8e11,// 85 PAY  82 

    0x9e1d60c0,// 86 PAY  83 

    0x70c5454d,// 87 PAY  84 

    0x54f23690,// 88 PAY  85 

    0x00fe4b96,// 89 PAY  86 

    0xfddaf112,// 90 PAY  87 

    0x5a5130dc,// 91 PAY  88 

    0xf55bb9a2,// 92 PAY  89 

    0x07c4c8c9,// 93 PAY  90 

    0xe3d8fcf5,// 94 PAY  91 

    0xf07e5cbd,// 95 PAY  92 

    0xc94b4e5e,// 96 PAY  93 

    0x51f482b7,// 97 PAY  94 

    0x2e8431ec,// 98 PAY  95 

    0x8c8fa098,// 99 PAY  96 

    0xbe60155c,// 100 PAY  97 

    0xde0b7bc9,// 101 PAY  98 

    0xd511dfff,// 102 PAY  99 

    0xd7c3afa3,// 103 PAY 100 

    0xcf610365,// 104 PAY 101 

    0x27808b29,// 105 PAY 102 

    0x6bf32684,// 106 PAY 103 

    0x4df690f7,// 107 PAY 104 

    0x3ce938ad,// 108 PAY 105 

    0x34e7e8fe,// 109 PAY 106 

    0x61f4896f,// 110 PAY 107 

    0x79cb9fc9,// 111 PAY 108 

    0xe4a43220,// 112 PAY 109 

    0x5cad8f01,// 113 PAY 110 

    0x0d4204c4,// 114 PAY 111 

    0xa0e740e3,// 115 PAY 112 

    0x5df32d68,// 116 PAY 113 

    0x46a2b342,// 117 PAY 114 

    0x373a5ec7,// 118 PAY 115 

    0xf7beabea,// 119 PAY 116 

    0xddce95c5,// 120 PAY 117 

    0xcc85f1ab,// 121 PAY 118 

    0x8ef0c9bc,// 122 PAY 119 

    0x788b00ca,// 123 PAY 120 

    0x02b419e6,// 124 PAY 121 

    0x31f68080,// 125 PAY 122 

    0x39ed5859,// 126 PAY 123 

    0x7b2db9e6,// 127 PAY 124 

    0x3dece79b,// 128 PAY 125 

    0xa792193a,// 129 PAY 126 

    0x98e40185,// 130 PAY 127 

    0xe734ffb1,// 131 PAY 128 

    0xb5151117,// 132 PAY 129 

    0xe0d84119,// 133 PAY 130 

    0xeefee000,// 134 PAY 131 

    0xec93acdd,// 135 PAY 132 

    0xf1f16cdb,// 136 PAY 133 

    0x5a683189,// 137 PAY 134 

    0x54e427cb,// 138 PAY 135 

    0x669d7a51,// 139 PAY 136 

    0xaa2b9705,// 140 PAY 137 

    0xd3df2694,// 141 PAY 138 

    0xcd72a0fc,// 142 PAY 139 

    0xc600ccd9,// 143 PAY 140 

    0x4411dff2,// 144 PAY 141 

    0xe8e0ebc8,// 145 PAY 142 

    0x7e88e415,// 146 PAY 143 

    0xfeacf916,// 147 PAY 144 

    0x4741a800,// 148 PAY 145 

    0x90f38208,// 149 PAY 146 

    0x91771327,// 150 PAY 147 

    0xe04e8491,// 151 PAY 148 

    0x0e1c7a93,// 152 PAY 149 

    0x344c9291,// 153 PAY 150 

    0x9e9b6a4f,// 154 PAY 151 

    0xe7620d50,// 155 PAY 152 

    0x1787f7f0,// 156 PAY 153 

    0x0a736e27,// 157 PAY 154 

    0x0bc8e65f,// 158 PAY 155 

    0x869e4f2a,// 159 PAY 156 

    0xb92333c9,// 160 PAY 157 

    0x322aa4d9,// 161 PAY 158 

    0x69698dad,// 162 PAY 159 

    0x229be6ba,// 163 PAY 160 

    0xaf19c359,// 164 PAY 161 

    0x0822641e,// 165 PAY 162 

    0xcc0289f9,// 166 PAY 163 

    0xcc6031ef,// 167 PAY 164 

    0x92aeebc2,// 168 PAY 165 

    0x8077e8c3,// 169 PAY 166 

    0x6d0202af,// 170 PAY 167 

    0x50a40237,// 171 PAY 168 

    0x9e75ec88,// 172 PAY 169 

    0x2d29edfc,// 173 PAY 170 

    0x20d6193a,// 174 PAY 171 

    0x46b6e152,// 175 PAY 172 

    0xf48fb614,// 176 PAY 173 

    0xb718da96,// 177 PAY 174 

    0x321fff2c,// 178 PAY 175 

    0x06312dca,// 179 PAY 176 

    0x28494580,// 180 PAY 177 

    0x4a72e939,// 181 PAY 178 

    0xa65ab225,// 182 PAY 179 

    0xca096c90,// 183 PAY 180 

    0x64954055,// 184 PAY 181 

    0xe408bde3,// 185 PAY 182 

    0xb7a4b33d,// 186 PAY 183 

    0x552aaa56,// 187 PAY 184 

    0x404f62c9,// 188 PAY 185 

    0x02b87584,// 189 PAY 186 

    0x1cc291f1,// 190 PAY 187 

    0x18d9cd6a,// 191 PAY 188 

    0xc39bdc6b,// 192 PAY 189 

    0xfaa1a7e3,// 193 PAY 190 

    0xa5f167da,// 194 PAY 191 

    0x098f675e,// 195 PAY 192 

    0x03fc1325,// 196 PAY 193 

    0x67c89bb4,// 197 PAY 194 

    0xc358d625,// 198 PAY 195 

    0x6d53f4c2,// 199 PAY 196 

    0x79fbe6f8,// 200 PAY 197 

    0x814ce051,// 201 PAY 198 

    0xf52eb3a2,// 202 PAY 199 

    0xd05c20b6,// 203 PAY 200 

    0x25f3dd49,// 204 PAY 201 

    0x7ed5fe7d,// 205 PAY 202 

    0xe7bdbaab,// 206 PAY 203 

    0x08a209d8,// 207 PAY 204 

    0xd62b09cd,// 208 PAY 205 

    0xbacb1c94,// 209 PAY 206 

    0x80147eab,// 210 PAY 207 

    0xce81bdd3,// 211 PAY 208 

    0x46676d12,// 212 PAY 209 

    0xf0726e6a,// 213 PAY 210 

    0x3532ec2f,// 214 PAY 211 

    0x07df397d,// 215 PAY 212 

    0xb6f82621,// 216 PAY 213 

    0x4287e84c,// 217 PAY 214 

    0xa9598565,// 218 PAY 215 

    0x5109af92,// 219 PAY 216 

    0x6b400e45,// 220 PAY 217 

    0xacc4b7fe,// 221 PAY 218 

    0x7dfed360,// 222 PAY 219 

    0xbda9225e,// 223 PAY 220 

    0xfab7e432,// 224 PAY 221 

    0x998e3d34,// 225 PAY 222 

    0x39883c6f,// 226 PAY 223 

    0x0c6ca656,// 227 PAY 224 

    0x0a3cc909,// 228 PAY 225 

    0xe3bc8fe0,// 229 PAY 226 

    0x9e90a8bf,// 230 PAY 227 

    0x78699dcc,// 231 PAY 228 

    0x085612cd,// 232 PAY 229 

    0x75c6b123,// 233 PAY 230 

    0x0124bf35,// 234 PAY 231 

    0x0e94f33c,// 235 PAY 232 

    0x06ff0e99,// 236 PAY 233 

    0x2bbe6802,// 237 PAY 234 

    0x6cfabeb0,// 238 PAY 235 

    0x0c00d9d5,// 239 PAY 236 

    0xb4462514,// 240 PAY 237 

    0x95aee21e,// 241 PAY 238 

    0xbe65953f,// 242 PAY 239 

    0x1c051e8b,// 243 PAY 240 

    0x801897fe,// 244 PAY 241 

    0xa2602899,// 245 PAY 242 

    0xd8ab51b5,// 246 PAY 243 

    0x4a3f7156,// 247 PAY 244 

    0xa0d33201,// 248 PAY 245 

    0xecaca6fb,// 249 PAY 246 

    0xa62d7cc4,// 250 PAY 247 

    0xda2c11b3,// 251 PAY 248 

    0x39373a96,// 252 PAY 249 

    0x37ef946e,// 253 PAY 250 

    0x21db3408,// 254 PAY 251 

    0xc3c3f108,// 255 PAY 252 

    0x81d70531,// 256 PAY 253 

    0x5d62bc13,// 257 PAY 254 

    0x598c908b,// 258 PAY 255 

    0x54ca15d6,// 259 PAY 256 

    0x5ef1de5a,// 260 PAY 257 

    0xbfad2d2f,// 261 PAY 258 

    0xb85e81c7,// 262 PAY 259 

    0xb0e36874,// 263 PAY 260 

    0xa7cca493,// 264 PAY 261 

    0x54869c12,// 265 PAY 262 

    0x32b3cc1d,// 266 PAY 263 

    0x6b7d68b9,// 267 PAY 264 

    0xe530861a,// 268 PAY 265 

    0x881009d1,// 269 PAY 266 

    0x3a71a58b,// 270 PAY 267 

    0x299e8633,// 271 PAY 268 

    0x3f53d2fd,// 272 PAY 269 

    0x224ee610,// 273 PAY 270 

    0x54a4d05e,// 274 PAY 271 

    0xbe14449e,// 275 PAY 272 

    0x84756a47,// 276 PAY 273 

    0xd23ee597,// 277 PAY 274 

    0x0ad201b3,// 278 PAY 275 

    0x6c4f11bd,// 279 PAY 276 

    0xefb11912,// 280 PAY 277 

    0x93beda38,// 281 PAY 278 

    0x42030f1b,// 282 PAY 279 

    0x0b11bd76,// 283 PAY 280 

    0x50375203,// 284 PAY 281 

    0xf5504fa5,// 285 PAY 282 

    0xeaee5c66,// 286 PAY 283 

    0x493bb7ff,// 287 PAY 284 

    0x3a15e554,// 288 PAY 285 

    0xd93522ae,// 289 PAY 286 

    0xfd6e13b2,// 290 PAY 287 

    0xc7114be5,// 291 PAY 288 

    0xab40ace4,// 292 PAY 289 

    0xfca1a272,// 293 PAY 290 

    0x3b88b3da,// 294 PAY 291 

    0x3a8ea39e,// 295 PAY 292 

/// STA is 1 words. 

/// STA num_pkts       : 210 

/// STA pkt_idx        : 251 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1f 

    0x03ed1fd2 // 296 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt82_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 171 words. 

/// BDA size     is 678 (0x2a6) 

/// BDA id       is 0x8517 

    0x02a68517,// 3 BDA   1 

/// PAY Generic Data size   : 678 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x5f5d8290,// 4 PAY   1 

    0x26b8969c,// 5 PAY   2 

    0x1faf59f4,// 6 PAY   3 

    0x2ddf1acc,// 7 PAY   4 

    0x5a92d801,// 8 PAY   5 

    0x9c38897b,// 9 PAY   6 

    0x749a2521,// 10 PAY   7 

    0x4dbb4fc6,// 11 PAY   8 

    0xb04b5bfd,// 12 PAY   9 

    0xd9d7d590,// 13 PAY  10 

    0xea2e48bb,// 14 PAY  11 

    0x6885c108,// 15 PAY  12 

    0x65294ced,// 16 PAY  13 

    0x6c8be1b7,// 17 PAY  14 

    0x3007ac7e,// 18 PAY  15 

    0xacc53174,// 19 PAY  16 

    0xc44e2672,// 20 PAY  17 

    0x79aa7101,// 21 PAY  18 

    0x5e2b9a61,// 22 PAY  19 

    0x157873db,// 23 PAY  20 

    0xb5f9b1e0,// 24 PAY  21 

    0xebd9dfa9,// 25 PAY  22 

    0x74fd9937,// 26 PAY  23 

    0x0a1b2c33,// 27 PAY  24 

    0xfc932084,// 28 PAY  25 

    0x4c075bdf,// 29 PAY  26 

    0xc3558a20,// 30 PAY  27 

    0xcda700cd,// 31 PAY  28 

    0xf8a0f3ec,// 32 PAY  29 

    0xf1a6b214,// 33 PAY  30 

    0x722fbc6c,// 34 PAY  31 

    0xdbf37a00,// 35 PAY  32 

    0x09c2434e,// 36 PAY  33 

    0xc714f3cc,// 37 PAY  34 

    0xfa0fb82d,// 38 PAY  35 

    0xc7b7d619,// 39 PAY  36 

    0x33037434,// 40 PAY  37 

    0x92fc8467,// 41 PAY  38 

    0x1a342875,// 42 PAY  39 

    0xe85aafe7,// 43 PAY  40 

    0x398c50e6,// 44 PAY  41 

    0xd3024423,// 45 PAY  42 

    0x7b01b4eb,// 46 PAY  43 

    0x96b9a6d0,// 47 PAY  44 

    0x19782474,// 48 PAY  45 

    0xc3b32ffe,// 49 PAY  46 

    0x0e3883db,// 50 PAY  47 

    0x3f38026d,// 51 PAY  48 

    0xe4b8703f,// 52 PAY  49 

    0x750fe484,// 53 PAY  50 

    0xb550fd68,// 54 PAY  51 

    0x305b7778,// 55 PAY  52 

    0x0d5ffaf1,// 56 PAY  53 

    0x31f95267,// 57 PAY  54 

    0xcd837f05,// 58 PAY  55 

    0x8dd49460,// 59 PAY  56 

    0x2f6088b9,// 60 PAY  57 

    0x168c0702,// 61 PAY  58 

    0x9e7ce4d3,// 62 PAY  59 

    0x401cfc87,// 63 PAY  60 

    0x3d371a6a,// 64 PAY  61 

    0x06c6fc32,// 65 PAY  62 

    0xfce9edb3,// 66 PAY  63 

    0x6b393a89,// 67 PAY  64 

    0x681e1498,// 68 PAY  65 

    0x6af2f8ad,// 69 PAY  66 

    0x7f4a1811,// 70 PAY  67 

    0x1abeb7d7,// 71 PAY  68 

    0x5ec8f571,// 72 PAY  69 

    0x5a1d13d7,// 73 PAY  70 

    0xdce1c05c,// 74 PAY  71 

    0xf545212e,// 75 PAY  72 

    0x48327e24,// 76 PAY  73 

    0x7031fc78,// 77 PAY  74 

    0x812804f9,// 78 PAY  75 

    0xb128b209,// 79 PAY  76 

    0x8eca133e,// 80 PAY  77 

    0x8e391398,// 81 PAY  78 

    0x0642ae4a,// 82 PAY  79 

    0x2b03acfd,// 83 PAY  80 

    0xa3c88df9,// 84 PAY  81 

    0x2c0746ef,// 85 PAY  82 

    0x107aff09,// 86 PAY  83 

    0xcf2a31b3,// 87 PAY  84 

    0xd8901bff,// 88 PAY  85 

    0x4711f3ad,// 89 PAY  86 

    0x1d5f1e3c,// 90 PAY  87 

    0x7eb5691b,// 91 PAY  88 

    0xf8f5241e,// 92 PAY  89 

    0xf95318d2,// 93 PAY  90 

    0xefcd6568,// 94 PAY  91 

    0xcdac554c,// 95 PAY  92 

    0xffce896e,// 96 PAY  93 

    0x9da1a8af,// 97 PAY  94 

    0x9529482b,// 98 PAY  95 

    0xbb00810e,// 99 PAY  96 

    0x569c2792,// 100 PAY  97 

    0x9e59d3a1,// 101 PAY  98 

    0x6a9f26bf,// 102 PAY  99 

    0xfebfac20,// 103 PAY 100 

    0x4502417d,// 104 PAY 101 

    0xbaf8de61,// 105 PAY 102 

    0xd8009212,// 106 PAY 103 

    0x5885bca8,// 107 PAY 104 

    0xc9647a60,// 108 PAY 105 

    0xb4493643,// 109 PAY 106 

    0x243d5ecc,// 110 PAY 107 

    0xdebc3d76,// 111 PAY 108 

    0x6f24e230,// 112 PAY 109 

    0x63bd8933,// 113 PAY 110 

    0x0b946ec6,// 114 PAY 111 

    0x83ef235e,// 115 PAY 112 

    0xaede92cc,// 116 PAY 113 

    0x7510d96c,// 117 PAY 114 

    0x65ff2e16,// 118 PAY 115 

    0xd36fdfd5,// 119 PAY 116 

    0xcdc3b1c5,// 120 PAY 117 

    0xda2f3bd6,// 121 PAY 118 

    0x5595aacf,// 122 PAY 119 

    0x4a2fed41,// 123 PAY 120 

    0x4d3768a5,// 124 PAY 121 

    0xa55eaacb,// 125 PAY 122 

    0x4ec9f68a,// 126 PAY 123 

    0x19a2368e,// 127 PAY 124 

    0xc8801f43,// 128 PAY 125 

    0x0927de40,// 129 PAY 126 

    0xc4ab8bd8,// 130 PAY 127 

    0x60dfe6a8,// 131 PAY 128 

    0x0a9ba76d,// 132 PAY 129 

    0x3715475d,// 133 PAY 130 

    0xa44eb991,// 134 PAY 131 

    0xea21eb45,// 135 PAY 132 

    0x289ab71b,// 136 PAY 133 

    0xd0c141ce,// 137 PAY 134 

    0x52982cb3,// 138 PAY 135 

    0x3dd1f213,// 139 PAY 136 

    0xdfde137e,// 140 PAY 137 

    0x42a3a472,// 141 PAY 138 

    0x31410465,// 142 PAY 139 

    0x3491fbe0,// 143 PAY 140 

    0x1c7491d2,// 144 PAY 141 

    0x4ffcac54,// 145 PAY 142 

    0x5033e5e0,// 146 PAY 143 

    0x8d516ba4,// 147 PAY 144 

    0x432bc37d,// 148 PAY 145 

    0xc5149b1d,// 149 PAY 146 

    0x8da90d64,// 150 PAY 147 

    0x312d5991,// 151 PAY 148 

    0x1a09df1c,// 152 PAY 149 

    0x8020b641,// 153 PAY 150 

    0x33e0d04a,// 154 PAY 151 

    0x9a3b48c3,// 155 PAY 152 

    0x3dca9bc4,// 156 PAY 153 

    0x5ed9c417,// 157 PAY 154 

    0x8d9b3e3a,// 158 PAY 155 

    0xaa226e02,// 159 PAY 156 

    0x99196241,// 160 PAY 157 

    0x6fed8bc0,// 161 PAY 158 

    0xc7223205,// 162 PAY 159 

    0xbc2b1411,// 163 PAY 160 

    0xaa0e8489,// 164 PAY 161 

    0x0dba8ee2,// 165 PAY 162 

    0xc07cd2a4,// 166 PAY 163 

    0xe6f59b47,// 167 PAY 164 

    0x9f0ca412,// 168 PAY 165 

    0x58685b4e,// 169 PAY 166 

    0x46566cd4,// 170 PAY 167 

    0x342c09f6,// 171 PAY 168 

    0x17f03722,// 172 PAY 169 

    0xbcfc0000,// 173 PAY 170 

/// HASH is  12 bytes 

    0x4711f3ad,// 174 HSH   1 

    0x1d5f1e3c,// 175 HSH   2 

    0x7eb5691b,// 176 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 81 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdc 

    0x03dcdc51 // 177 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt83_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 265 words. 

/// BDA size     is 1056 (0x420) 

/// BDA id       is 0x8d6d 

    0x04208d6d,// 3 BDA   1 

/// PAY Generic Data size   : 1056 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x4f6bf28c,// 4 PAY   1 

    0x54e5c017,// 5 PAY   2 

    0xbc86ac8c,// 6 PAY   3 

    0x6db3e34b,// 7 PAY   4 

    0x1f8b8b0b,// 8 PAY   5 

    0xb9944e08,// 9 PAY   6 

    0x5c140254,// 10 PAY   7 

    0x87d7dc6d,// 11 PAY   8 

    0x82734e90,// 12 PAY   9 

    0xcda1b2d0,// 13 PAY  10 

    0x0a3bef63,// 14 PAY  11 

    0xd9efa2a6,// 15 PAY  12 

    0xb7e7116d,// 16 PAY  13 

    0xfab3442b,// 17 PAY  14 

    0xa5ce6fc9,// 18 PAY  15 

    0x5f3fdfcd,// 19 PAY  16 

    0xa654ec39,// 20 PAY  17 

    0xe0f41f3a,// 21 PAY  18 

    0x044e3c29,// 22 PAY  19 

    0x5cc25e1a,// 23 PAY  20 

    0x6f0962ab,// 24 PAY  21 

    0x9cab752e,// 25 PAY  22 

    0xbb70973d,// 26 PAY  23 

    0xcfd74dd6,// 27 PAY  24 

    0xbb449320,// 28 PAY  25 

    0x23f1d905,// 29 PAY  26 

    0xa13803f5,// 30 PAY  27 

    0x1ea95a2b,// 31 PAY  28 

    0xa36fed93,// 32 PAY  29 

    0x5d542d88,// 33 PAY  30 

    0x83da026a,// 34 PAY  31 

    0x27fd083c,// 35 PAY  32 

    0xa56a32aa,// 36 PAY  33 

    0x5c32bb9c,// 37 PAY  34 

    0x173f3a08,// 38 PAY  35 

    0xd19e2d4a,// 39 PAY  36 

    0xd8b0d123,// 40 PAY  37 

    0xa5eedcee,// 41 PAY  38 

    0x9e740923,// 42 PAY  39 

    0x36631632,// 43 PAY  40 

    0xc9dddbf3,// 44 PAY  41 

    0x677bc030,// 45 PAY  42 

    0xe13f2994,// 46 PAY  43 

    0x02a188ee,// 47 PAY  44 

    0xe51612af,// 48 PAY  45 

    0xb7632e5e,// 49 PAY  46 

    0x2a4d315e,// 50 PAY  47 

    0x33da3066,// 51 PAY  48 

    0x97c81f0d,// 52 PAY  49 

    0x7e155595,// 53 PAY  50 

    0xe7880ab7,// 54 PAY  51 

    0xd6d9312d,// 55 PAY  52 

    0x17093299,// 56 PAY  53 

    0x3af2917a,// 57 PAY  54 

    0x946bfea1,// 58 PAY  55 

    0x6f84a6a7,// 59 PAY  56 

    0x6973fb7a,// 60 PAY  57 

    0xa3df6ee8,// 61 PAY  58 

    0x10b230bb,// 62 PAY  59 

    0x06b735a6,// 63 PAY  60 

    0x3287b62d,// 64 PAY  61 

    0x07cf70a8,// 65 PAY  62 

    0x56882d87,// 66 PAY  63 

    0x8ffba09a,// 67 PAY  64 

    0x9126ea3f,// 68 PAY  65 

    0x10ef6e75,// 69 PAY  66 

    0x131e15f9,// 70 PAY  67 

    0x7e845747,// 71 PAY  68 

    0xe34b0f13,// 72 PAY  69 

    0x6fc20ee2,// 73 PAY  70 

    0xf535a79c,// 74 PAY  71 

    0x0f8d8822,// 75 PAY  72 

    0x274a2f08,// 76 PAY  73 

    0x90e860a7,// 77 PAY  74 

    0x9dd23bcb,// 78 PAY  75 

    0x7e2ecfe2,// 79 PAY  76 

    0x8aef6ea9,// 80 PAY  77 

    0xfd3e00f3,// 81 PAY  78 

    0x2d64c4aa,// 82 PAY  79 

    0xb1986218,// 83 PAY  80 

    0x1e4a87d2,// 84 PAY  81 

    0x1533151b,// 85 PAY  82 

    0xd1eab7a3,// 86 PAY  83 

    0x99992885,// 87 PAY  84 

    0x8b173a1f,// 88 PAY  85 

    0x90e03134,// 89 PAY  86 

    0x4b21ac2f,// 90 PAY  87 

    0x4fd92492,// 91 PAY  88 

    0x00aee959,// 92 PAY  89 

    0xbc5b57de,// 93 PAY  90 

    0x65021a58,// 94 PAY  91 

    0x612b9ce1,// 95 PAY  92 

    0xd9526e2f,// 96 PAY  93 

    0x4934eaf4,// 97 PAY  94 

    0xf784d333,// 98 PAY  95 

    0x315e2285,// 99 PAY  96 

    0xc2ec6e28,// 100 PAY  97 

    0x5551070a,// 101 PAY  98 

    0x71e3a827,// 102 PAY  99 

    0xf2e494f5,// 103 PAY 100 

    0xc5960fcc,// 104 PAY 101 

    0x2d95f0a5,// 105 PAY 102 

    0x1fe2fdd8,// 106 PAY 103 

    0x7304bdb5,// 107 PAY 104 

    0xc46c87f6,// 108 PAY 105 

    0xc2be0acc,// 109 PAY 106 

    0x324ed126,// 110 PAY 107 

    0x869d9b29,// 111 PAY 108 

    0xa924dc56,// 112 PAY 109 

    0xf79c9271,// 113 PAY 110 

    0xc2161add,// 114 PAY 111 

    0xdb6875eb,// 115 PAY 112 

    0x60adb534,// 116 PAY 113 

    0x49c59c04,// 117 PAY 114 

    0x67fa44a7,// 118 PAY 115 

    0x9b15507b,// 119 PAY 116 

    0x8ee55fda,// 120 PAY 117 

    0x2535f865,// 121 PAY 118 

    0x20aac7cb,// 122 PAY 119 

    0x596df16d,// 123 PAY 120 

    0x4c22e5d8,// 124 PAY 121 

    0x0ce3fa30,// 125 PAY 122 

    0xb09b60ca,// 126 PAY 123 

    0xcfb2b9ed,// 127 PAY 124 

    0x5b0c5945,// 128 PAY 125 

    0xafec8c22,// 129 PAY 126 

    0x7fa65812,// 130 PAY 127 

    0x43d9c939,// 131 PAY 128 

    0x1a2a9f16,// 132 PAY 129 

    0xd9ba4777,// 133 PAY 130 

    0x27e89b45,// 134 PAY 131 

    0x6e94c58f,// 135 PAY 132 

    0xcb500881,// 136 PAY 133 

    0xa7151ba1,// 137 PAY 134 

    0xa180653f,// 138 PAY 135 

    0xbd32f4a1,// 139 PAY 136 

    0x4aff7516,// 140 PAY 137 

    0x4d7e8f1b,// 141 PAY 138 

    0xb1b336df,// 142 PAY 139 

    0x1a14e000,// 143 PAY 140 

    0x4036ba76,// 144 PAY 141 

    0x3e9e099e,// 145 PAY 142 

    0x3825dcbe,// 146 PAY 143 

    0x96f7e76d,// 147 PAY 144 

    0xb32d12b3,// 148 PAY 145 

    0x724b129c,// 149 PAY 146 

    0x6fc1fe39,// 150 PAY 147 

    0xf2f85f51,// 151 PAY 148 

    0x6aaaf13b,// 152 PAY 149 

    0xd84c7f35,// 153 PAY 150 

    0x627b5866,// 154 PAY 151 

    0xda6db5fd,// 155 PAY 152 

    0x6fdaa956,// 156 PAY 153 

    0x6d09fcd6,// 157 PAY 154 

    0x91de973c,// 158 PAY 155 

    0x62c31f61,// 159 PAY 156 

    0xbfa746c0,// 160 PAY 157 

    0x5d517778,// 161 PAY 158 

    0x0777882e,// 162 PAY 159 

    0x0e05a6e7,// 163 PAY 160 

    0x465b26a2,// 164 PAY 161 

    0xac424019,// 165 PAY 162 

    0x1eec525a,// 166 PAY 163 

    0x0f491e70,// 167 PAY 164 

    0x806509e4,// 168 PAY 165 

    0xb85ba1a3,// 169 PAY 166 

    0x5abf9bf2,// 170 PAY 167 

    0xf55ec72d,// 171 PAY 168 

    0xf3f1e51d,// 172 PAY 169 

    0x8a9f46ec,// 173 PAY 170 

    0x4f7d57f8,// 174 PAY 171 

    0x852ac50a,// 175 PAY 172 

    0x9cb98fcb,// 176 PAY 173 

    0x1b6bae1c,// 177 PAY 174 

    0x3c67592b,// 178 PAY 175 

    0x0bede6cc,// 179 PAY 176 

    0x57fea48a,// 180 PAY 177 

    0x3c100861,// 181 PAY 178 

    0xbc2290c5,// 182 PAY 179 

    0x0fc32216,// 183 PAY 180 

    0xdbe8899d,// 184 PAY 181 

    0xf4feeb13,// 185 PAY 182 

    0x14657838,// 186 PAY 183 

    0x8ba55a31,// 187 PAY 184 

    0xe5dce82e,// 188 PAY 185 

    0x583c85c9,// 189 PAY 186 

    0xb14c481e,// 190 PAY 187 

    0xabfe908b,// 191 PAY 188 

    0xbaf33440,// 192 PAY 189 

    0x7089ceb2,// 193 PAY 190 

    0x40381ca4,// 194 PAY 191 

    0x62509af4,// 195 PAY 192 

    0x37c5c626,// 196 PAY 193 

    0x908ef23a,// 197 PAY 194 

    0x2e40c50c,// 198 PAY 195 

    0xa8780015,// 199 PAY 196 

    0xd9309369,// 200 PAY 197 

    0x1f2ab713,// 201 PAY 198 

    0xed2fd430,// 202 PAY 199 

    0x5f50a089,// 203 PAY 200 

    0xccc7b64e,// 204 PAY 201 

    0x0d785b88,// 205 PAY 202 

    0x0df01069,// 206 PAY 203 

    0xe16a809b,// 207 PAY 204 

    0xbc858f1f,// 208 PAY 205 

    0x4cd2fc49,// 209 PAY 206 

    0x8bea3464,// 210 PAY 207 

    0xd254bab7,// 211 PAY 208 

    0x0fecc96d,// 212 PAY 209 

    0xcb28cc26,// 213 PAY 210 

    0x42a8b206,// 214 PAY 211 

    0xd6efc660,// 215 PAY 212 

    0x69eff2d3,// 216 PAY 213 

    0x575b2f64,// 217 PAY 214 

    0x78bd8f64,// 218 PAY 215 

    0x0926ff4b,// 219 PAY 216 

    0xd0de100f,// 220 PAY 217 

    0xf5300a84,// 221 PAY 218 

    0x87601744,// 222 PAY 219 

    0x126d12e6,// 223 PAY 220 

    0xd2851cc4,// 224 PAY 221 

    0x27bc9817,// 225 PAY 222 

    0x16ea8017,// 226 PAY 223 

    0x46be7527,// 227 PAY 224 

    0x1ca9c4a7,// 228 PAY 225 

    0x6a7a0c0d,// 229 PAY 226 

    0xdbb3c718,// 230 PAY 227 

    0x1eb68c0b,// 231 PAY 228 

    0x3f09a9fe,// 232 PAY 229 

    0xcbaf63ed,// 233 PAY 230 

    0x760ee7e8,// 234 PAY 231 

    0x0f305afa,// 235 PAY 232 

    0x9013fd8d,// 236 PAY 233 

    0x4732d5cc,// 237 PAY 234 

    0xcdfe0ef4,// 238 PAY 235 

    0xb4cf9445,// 239 PAY 236 

    0xb41eb56e,// 240 PAY 237 

    0xc22e4aaf,// 241 PAY 238 

    0xb15c946c,// 242 PAY 239 

    0xa8530b88,// 243 PAY 240 

    0xa8e23b3f,// 244 PAY 241 

    0xe07c6510,// 245 PAY 242 

    0x8752761c,// 246 PAY 243 

    0x05bb1029,// 247 PAY 244 

    0xae52a23c,// 248 PAY 245 

    0x79c88b27,// 249 PAY 246 

    0x4910a0b7,// 250 PAY 247 

    0x627af672,// 251 PAY 248 

    0x37b6fdf9,// 252 PAY 249 

    0x5b8fd88c,// 253 PAY 250 

    0x1fbf0d37,// 254 PAY 251 

    0x4eca03a9,// 255 PAY 252 

    0x47a92f98,// 256 PAY 253 

    0xf33f088d,// 257 PAY 254 

    0x7803ad71,// 258 PAY 255 

    0x1ca66cc2,// 259 PAY 256 

    0x3de4ad17,// 260 PAY 257 

    0xd924dc9c,// 261 PAY 258 

    0xe3d29f33,// 262 PAY 259 

    0xb461950b,// 263 PAY 260 

    0x629d6742,// 264 PAY 261 

    0xb81b9fa1,// 265 PAY 262 

    0xcc8730c3,// 266 PAY 263 

    0x966d05ad,// 267 PAY 264 

/// HASH is  12 bytes 

    0x79c88b27,// 268 HSH   1 

    0x4910a0b7,// 269 HSH   2 

    0x627af672,// 270 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 206 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x16 

    0x03391649 // 271 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt84_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 227 words. 

/// BDA size     is 901 (0x385) 

/// BDA id       is 0x8ef 

    0x038508ef,// 3 BDA   1 

/// PAY Generic Data size   : 901 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x85100d80,// 4 PAY   1 

    0x63c54be0,// 5 PAY   2 

    0xc0340fdb,// 6 PAY   3 

    0x991d9a64,// 7 PAY   4 

    0x780acfd6,// 8 PAY   5 

    0xc24f033c,// 9 PAY   6 

    0x5bcf08bc,// 10 PAY   7 

    0xc024443a,// 11 PAY   8 

    0x2f05c192,// 12 PAY   9 

    0x08b99921,// 13 PAY  10 

    0x3359bd07,// 14 PAY  11 

    0x83124194,// 15 PAY  12 

    0x4a21c7fa,// 16 PAY  13 

    0xe7b84476,// 17 PAY  14 

    0x58ac643e,// 18 PAY  15 

    0x1bfa324e,// 19 PAY  16 

    0x4124c99f,// 20 PAY  17 

    0x79747ae8,// 21 PAY  18 

    0xcc500337,// 22 PAY  19 

    0xe98d61ba,// 23 PAY  20 

    0x634cda54,// 24 PAY  21 

    0x4797bc1c,// 25 PAY  22 

    0x97ff7c5f,// 26 PAY  23 

    0xd6a38b1e,// 27 PAY  24 

    0x548b5b14,// 28 PAY  25 

    0x20b75f17,// 29 PAY  26 

    0x30ba6400,// 30 PAY  27 

    0xddd43c1e,// 31 PAY  28 

    0x387e7571,// 32 PAY  29 

    0xabb171c2,// 33 PAY  30 

    0x950bb682,// 34 PAY  31 

    0xe986c476,// 35 PAY  32 

    0xb87f8eea,// 36 PAY  33 

    0x00184067,// 37 PAY  34 

    0x40c74f95,// 38 PAY  35 

    0x48bdcf13,// 39 PAY  36 

    0x660f8a5f,// 40 PAY  37 

    0xdb28890a,// 41 PAY  38 

    0xb3a4ec21,// 42 PAY  39 

    0x6813792d,// 43 PAY  40 

    0xb5b7f189,// 44 PAY  41 

    0xdff12800,// 45 PAY  42 

    0x622642c2,// 46 PAY  43 

    0x872f2844,// 47 PAY  44 

    0x81affe59,// 48 PAY  45 

    0x0fba94a1,// 49 PAY  46 

    0x33990d43,// 50 PAY  47 

    0x97e8913e,// 51 PAY  48 

    0x51734ad3,// 52 PAY  49 

    0xfb8ab4eb,// 53 PAY  50 

    0x83bdc52a,// 54 PAY  51 

    0xd38b8483,// 55 PAY  52 

    0x4d591778,// 56 PAY  53 

    0xf8c32e54,// 57 PAY  54 

    0x10fca49d,// 58 PAY  55 

    0xbcf4392b,// 59 PAY  56 

    0x11242553,// 60 PAY  57 

    0xd300fc03,// 61 PAY  58 

    0x05a7e4fd,// 62 PAY  59 

    0x922de5b0,// 63 PAY  60 

    0x379cdc02,// 64 PAY  61 

    0x21edaf16,// 65 PAY  62 

    0x3ab895fe,// 66 PAY  63 

    0x09559159,// 67 PAY  64 

    0x49b57881,// 68 PAY  65 

    0x4b586049,// 69 PAY  66 

    0x73c0052f,// 70 PAY  67 

    0xd3bdc4d4,// 71 PAY  68 

    0xf180a643,// 72 PAY  69 

    0x849264f0,// 73 PAY  70 

    0x07795755,// 74 PAY  71 

    0x1ac0b079,// 75 PAY  72 

    0xe2b8b4f0,// 76 PAY  73 

    0x44806f06,// 77 PAY  74 

    0xb6e4ef5c,// 78 PAY  75 

    0xa4b924ed,// 79 PAY  76 

    0xc366a765,// 80 PAY  77 

    0x48c3f211,// 81 PAY  78 

    0x8ec4b88c,// 82 PAY  79 

    0x6795990d,// 83 PAY  80 

    0x6ffc4a70,// 84 PAY  81 

    0xe0c69247,// 85 PAY  82 

    0x1cd5f59d,// 86 PAY  83 

    0x1f4571b2,// 87 PAY  84 

    0x1bb82823,// 88 PAY  85 

    0xed7bac26,// 89 PAY  86 

    0xdddf8b60,// 90 PAY  87 

    0xd2afa44f,// 91 PAY  88 

    0x112fb103,// 92 PAY  89 

    0xef887df9,// 93 PAY  90 

    0xe0ca6ffc,// 94 PAY  91 

    0x6855b46a,// 95 PAY  92 

    0x6a9070b0,// 96 PAY  93 

    0xa7fc897b,// 97 PAY  94 

    0x312a179b,// 98 PAY  95 

    0x476e8d63,// 99 PAY  96 

    0x5ef5fb7e,// 100 PAY  97 

    0x005764ba,// 101 PAY  98 

    0x4f58cdb3,// 102 PAY  99 

    0x4a3965b4,// 103 PAY 100 

    0xf6900a8d,// 104 PAY 101 

    0xd6423a49,// 105 PAY 102 

    0x8e7469be,// 106 PAY 103 

    0xc7443611,// 107 PAY 104 

    0x2f4d2943,// 108 PAY 105 

    0xc8f26b56,// 109 PAY 106 

    0x58bdcd04,// 110 PAY 107 

    0xd52db8dd,// 111 PAY 108 

    0xd4f0bb7e,// 112 PAY 109 

    0xb4c14021,// 113 PAY 110 

    0xf3e11b10,// 114 PAY 111 

    0xa3de78a4,// 115 PAY 112 

    0xd2fd495d,// 116 PAY 113 

    0xe48fb4cb,// 117 PAY 114 

    0x3696e254,// 118 PAY 115 

    0xe37cf27c,// 119 PAY 116 

    0xaec10fe6,// 120 PAY 117 

    0x81a2d3b2,// 121 PAY 118 

    0x8af770c8,// 122 PAY 119 

    0xe73cb39d,// 123 PAY 120 

    0x490dfbc9,// 124 PAY 121 

    0x835f77b1,// 125 PAY 122 

    0x35105e96,// 126 PAY 123 

    0x3f40b00a,// 127 PAY 124 

    0xdfd1e824,// 128 PAY 125 

    0x110ba00e,// 129 PAY 126 

    0xbbb908c2,// 130 PAY 127 

    0xa3adb4fd,// 131 PAY 128 

    0x2a4bc648,// 132 PAY 129 

    0x7a822d15,// 133 PAY 130 

    0xd4458b6d,// 134 PAY 131 

    0xa151dbcc,// 135 PAY 132 

    0xd536a8be,// 136 PAY 133 

    0xa0ed7077,// 137 PAY 134 

    0xd984cbfb,// 138 PAY 135 

    0xc4851837,// 139 PAY 136 

    0xa65100f7,// 140 PAY 137 

    0xf76427b2,// 141 PAY 138 

    0xabb432e4,// 142 PAY 139 

    0xb01db8fc,// 143 PAY 140 

    0xc5bc3a0c,// 144 PAY 141 

    0xa183e3f4,// 145 PAY 142 

    0x7344fe49,// 146 PAY 143 

    0x897c0588,// 147 PAY 144 

    0xada5b8f3,// 148 PAY 145 

    0x14181eae,// 149 PAY 146 

    0xf96f9905,// 150 PAY 147 

    0x56e75513,// 151 PAY 148 

    0xaef59fcf,// 152 PAY 149 

    0x52f6d1f5,// 153 PAY 150 

    0x864032e2,// 154 PAY 151 

    0x182e8803,// 155 PAY 152 

    0xea68f07a,// 156 PAY 153 

    0x1945b9c9,// 157 PAY 154 

    0xe1d9b0c8,// 158 PAY 155 

    0xd4f6262a,// 159 PAY 156 

    0x94a0f6ec,// 160 PAY 157 

    0xb4abb6a3,// 161 PAY 158 

    0x289bb28f,// 162 PAY 159 

    0x0e14c402,// 163 PAY 160 

    0xa3ef2e26,// 164 PAY 161 

    0xc3d52946,// 165 PAY 162 

    0xe42245b0,// 166 PAY 163 

    0x4ec3fbd5,// 167 PAY 164 

    0x3d44e06b,// 168 PAY 165 

    0x12ea2e9e,// 169 PAY 166 

    0x2164aba6,// 170 PAY 167 

    0x9cebfc8a,// 171 PAY 168 

    0xbc5de34b,// 172 PAY 169 

    0x5d54c7c9,// 173 PAY 170 

    0x1eeb1d8b,// 174 PAY 171 

    0x54adb716,// 175 PAY 172 

    0x91595072,// 176 PAY 173 

    0xbb416e7c,// 177 PAY 174 

    0x4a562c2a,// 178 PAY 175 

    0xebb2d9ef,// 179 PAY 176 

    0x38d4d8eb,// 180 PAY 177 

    0x48bcd044,// 181 PAY 178 

    0x0665d844,// 182 PAY 179 

    0x79189ee0,// 183 PAY 180 

    0x1def1323,// 184 PAY 181 

    0x44ffdc79,// 185 PAY 182 

    0xade654d7,// 186 PAY 183 

    0xb9256920,// 187 PAY 184 

    0x621f25d8,// 188 PAY 185 

    0x9b8ce5fc,// 189 PAY 186 

    0x4dd25f69,// 190 PAY 187 

    0x0812de14,// 191 PAY 188 

    0xd3d3e3f2,// 192 PAY 189 

    0x821bcf82,// 193 PAY 190 

    0xe15e6e90,// 194 PAY 191 

    0x4e6fa30d,// 195 PAY 192 

    0x2991ee29,// 196 PAY 193 

    0x0e1d47c2,// 197 PAY 194 

    0x8246fccc,// 198 PAY 195 

    0x7aef5ae3,// 199 PAY 196 

    0x17b4e94d,// 200 PAY 197 

    0xd933da6e,// 201 PAY 198 

    0x4fd95c06,// 202 PAY 199 

    0xa1623311,// 203 PAY 200 

    0x75601988,// 204 PAY 201 

    0xb225ade6,// 205 PAY 202 

    0x4ee01029,// 206 PAY 203 

    0x10003ca7,// 207 PAY 204 

    0xaae10b5a,// 208 PAY 205 

    0xe26bc0a3,// 209 PAY 206 

    0xd11f6e9c,// 210 PAY 207 

    0xf91533ed,// 211 PAY 208 

    0x060d7413,// 212 PAY 209 

    0x1d3dae7b,// 213 PAY 210 

    0x294a23d8,// 214 PAY 211 

    0x76ac0404,// 215 PAY 212 

    0xc9c5f018,// 216 PAY 213 

    0xb9b78485,// 217 PAY 214 

    0xa9756d9a,// 218 PAY 215 

    0xec6cd481,// 219 PAY 216 

    0x6cbbf150,// 220 PAY 217 

    0xc848a996,// 221 PAY 218 

    0xf30ca2b7,// 222 PAY 219 

    0x1d232061,// 223 PAY 220 

    0x57654e31,// 224 PAY 221 

    0x34479af6,// 225 PAY 222 

    0x3c3b7cbb,// 226 PAY 223 

    0xcf5730c3,// 227 PAY 224 

    0xd750d82a,// 228 PAY 225 

    0x4d000000,// 229 PAY 226 

/// STA is 1 words. 

/// STA num_pkts       : 53 

/// STA pkt_idx        : 49 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb4 

    0x00c4b435 // 230 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt85_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 501 words. 

/// BDA size     is 1997 (0x7cd) 

/// BDA id       is 0xca9 

    0x07cd0ca9,// 3 BDA   1 

/// PAY Generic Data size   : 1997 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x2c08faf8,// 4 PAY   1 

    0x39f740e4,// 5 PAY   2 

    0xc1e3d278,// 6 PAY   3 

    0xfc962da2,// 7 PAY   4 

    0x04033476,// 8 PAY   5 

    0xba39f6c4,// 9 PAY   6 

    0x1f307497,// 10 PAY   7 

    0x884574b6,// 11 PAY   8 

    0x635f18c7,// 12 PAY   9 

    0xde264c77,// 13 PAY  10 

    0x4732b862,// 14 PAY  11 

    0x3b91f0dd,// 15 PAY  12 

    0xc80312c2,// 16 PAY  13 

    0x0915cc14,// 17 PAY  14 

    0x35943c20,// 18 PAY  15 

    0x995f6c93,// 19 PAY  16 

    0x50594e51,// 20 PAY  17 

    0x4b5d8f2e,// 21 PAY  18 

    0x56fbc12d,// 22 PAY  19 

    0x944b7ebc,// 23 PAY  20 

    0x51c2d709,// 24 PAY  21 

    0xd9d95426,// 25 PAY  22 

    0xcba00c62,// 26 PAY  23 

    0x63c0e979,// 27 PAY  24 

    0x0a028882,// 28 PAY  25 

    0x59911d53,// 29 PAY  26 

    0x81217b4e,// 30 PAY  27 

    0x78c568c0,// 31 PAY  28 

    0xe96df748,// 32 PAY  29 

    0x1e6fd184,// 33 PAY  30 

    0xf5c0b2e6,// 34 PAY  31 

    0x6d967f18,// 35 PAY  32 

    0xd7edadc5,// 36 PAY  33 

    0x9a2ddec2,// 37 PAY  34 

    0xe515affa,// 38 PAY  35 

    0xe755b4eb,// 39 PAY  36 

    0x93656266,// 40 PAY  37 

    0x107336cc,// 41 PAY  38 

    0x0cc9ad16,// 42 PAY  39 

    0xff0bd07e,// 43 PAY  40 

    0x979a7b90,// 44 PAY  41 

    0x24853716,// 45 PAY  42 

    0x0362db59,// 46 PAY  43 

    0x7b3a7e30,// 47 PAY  44 

    0x8661bf6e,// 48 PAY  45 

    0x5ba02c50,// 49 PAY  46 

    0x44787274,// 50 PAY  47 

    0x2087cbf0,// 51 PAY  48 

    0xd08e9cd4,// 52 PAY  49 

    0x949251fc,// 53 PAY  50 

    0x02df3435,// 54 PAY  51 

    0xb8163cac,// 55 PAY  52 

    0x0bda4a97,// 56 PAY  53 

    0x3dcbedd2,// 57 PAY  54 

    0x00988fe3,// 58 PAY  55 

    0x770b4849,// 59 PAY  56 

    0xaf774cd3,// 60 PAY  57 

    0x2c40d3cc,// 61 PAY  58 

    0xff9a8e57,// 62 PAY  59 

    0x75f27f59,// 63 PAY  60 

    0x5015a05e,// 64 PAY  61 

    0xec399f06,// 65 PAY  62 

    0x15a33a46,// 66 PAY  63 

    0x5c9ecf67,// 67 PAY  64 

    0xc776eb66,// 68 PAY  65 

    0xcdee9cbd,// 69 PAY  66 

    0x6230f2e9,// 70 PAY  67 

    0x18b8d48a,// 71 PAY  68 

    0x57dbe6ba,// 72 PAY  69 

    0x68cbdffe,// 73 PAY  70 

    0x898c3b92,// 74 PAY  71 

    0x0360091a,// 75 PAY  72 

    0x1bceeb47,// 76 PAY  73 

    0x71ca1a0c,// 77 PAY  74 

    0x95a8a34a,// 78 PAY  75 

    0x9b9a04c6,// 79 PAY  76 

    0xf25e742a,// 80 PAY  77 

    0xba7017ee,// 81 PAY  78 

    0x22c16305,// 82 PAY  79 

    0x2ca4d41d,// 83 PAY  80 

    0xddee6527,// 84 PAY  81 

    0xba2b29ab,// 85 PAY  82 

    0x1ebc7565,// 86 PAY  83 

    0xdb094193,// 87 PAY  84 

    0x0f8012c2,// 88 PAY  85 

    0x6ddae70e,// 89 PAY  86 

    0x3e9dc1f3,// 90 PAY  87 

    0xe63acc4c,// 91 PAY  88 

    0x365c3be3,// 92 PAY  89 

    0x7a3abdaa,// 93 PAY  90 

    0x8464bfe7,// 94 PAY  91 

    0x09c0e809,// 95 PAY  92 

    0xe5307516,// 96 PAY  93 

    0xa8c84596,// 97 PAY  94 

    0x03246ea4,// 98 PAY  95 

    0xcbc7592a,// 99 PAY  96 

    0x36dc0f5c,// 100 PAY  97 

    0xe4c8b61b,// 101 PAY  98 

    0x31d91536,// 102 PAY  99 

    0xb016bbf4,// 103 PAY 100 

    0x90a5f955,// 104 PAY 101 

    0x88d4e711,// 105 PAY 102 

    0x9db17ab5,// 106 PAY 103 

    0xad449f9e,// 107 PAY 104 

    0x292e7b21,// 108 PAY 105 

    0x7486dcc0,// 109 PAY 106 

    0xe0fa86d8,// 110 PAY 107 

    0x9b72939b,// 111 PAY 108 

    0x99402dee,// 112 PAY 109 

    0x93804a5d,// 113 PAY 110 

    0xda3217e6,// 114 PAY 111 

    0x97d7c005,// 115 PAY 112 

    0xe3d6095a,// 116 PAY 113 

    0xcf884b0f,// 117 PAY 114 

    0xd0095a68,// 118 PAY 115 

    0xb528602a,// 119 PAY 116 

    0x59d290b7,// 120 PAY 117 

    0x490dcc6c,// 121 PAY 118 

    0x1eb91476,// 122 PAY 119 

    0x17f09b38,// 123 PAY 120 

    0x2a5b3e7f,// 124 PAY 121 

    0x8e342218,// 125 PAY 122 

    0xcab2b610,// 126 PAY 123 

    0x113c97cc,// 127 PAY 124 

    0x08ee2f74,// 128 PAY 125 

    0xb3b7f18b,// 129 PAY 126 

    0x64242487,// 130 PAY 127 

    0xbf23fc13,// 131 PAY 128 

    0x9d7b9bb6,// 132 PAY 129 

    0x684782d2,// 133 PAY 130 

    0xea416f87,// 134 PAY 131 

    0x3edc4314,// 135 PAY 132 

    0xa13040a7,// 136 PAY 133 

    0x372b5406,// 137 PAY 134 

    0x135757ff,// 138 PAY 135 

    0x52cbb5dc,// 139 PAY 136 

    0x83006b8f,// 140 PAY 137 

    0xe18c914c,// 141 PAY 138 

    0x13fc0b6c,// 142 PAY 139 

    0x1d401038,// 143 PAY 140 

    0xf232d9c4,// 144 PAY 141 

    0x2d251337,// 145 PAY 142 

    0xf1c7a9f1,// 146 PAY 143 

    0xb9c1fd46,// 147 PAY 144 

    0xf82ea63a,// 148 PAY 145 

    0xbe41a74c,// 149 PAY 146 

    0x007c799e,// 150 PAY 147 

    0x876ca738,// 151 PAY 148 

    0xca5862f5,// 152 PAY 149 

    0x6566ac5e,// 153 PAY 150 

    0xcb259acf,// 154 PAY 151 

    0xeeafdf22,// 155 PAY 152 

    0x3b39706d,// 156 PAY 153 

    0x23e5c72b,// 157 PAY 154 

    0xbd5edce5,// 158 PAY 155 

    0xda6488d8,// 159 PAY 156 

    0xaf8ed06d,// 160 PAY 157 

    0xfa992f35,// 161 PAY 158 

    0x6935044b,// 162 PAY 159 

    0xdcf6f4a9,// 163 PAY 160 

    0xf925f9d9,// 164 PAY 161 

    0xb6642cbe,// 165 PAY 162 

    0x1654f63f,// 166 PAY 163 

    0xdedb3b57,// 167 PAY 164 

    0x31dc8ee6,// 168 PAY 165 

    0x9e7e3505,// 169 PAY 166 

    0x23ad48a1,// 170 PAY 167 

    0x464cb102,// 171 PAY 168 

    0x4c82bd54,// 172 PAY 169 

    0x42484f9c,// 173 PAY 170 

    0xfead2285,// 174 PAY 171 

    0x11ffbd75,// 175 PAY 172 

    0x88c85ebb,// 176 PAY 173 

    0xce497c5b,// 177 PAY 174 

    0xc7649772,// 178 PAY 175 

    0x6dc01a2c,// 179 PAY 176 

    0x86dcebb5,// 180 PAY 177 

    0x941c5b30,// 181 PAY 178 

    0x7dc771bc,// 182 PAY 179 

    0x8324547b,// 183 PAY 180 

    0x318d0a90,// 184 PAY 181 

    0x662c14f7,// 185 PAY 182 

    0x769c77f8,// 186 PAY 183 

    0x11acf37d,// 187 PAY 184 

    0x5c7865cf,// 188 PAY 185 

    0xe1bf0fa5,// 189 PAY 186 

    0x0ec9ba25,// 190 PAY 187 

    0xe906f9da,// 191 PAY 188 

    0x7b8d2a24,// 192 PAY 189 

    0xfb741fe8,// 193 PAY 190 

    0xefa61b68,// 194 PAY 191 

    0x8df342b2,// 195 PAY 192 

    0x69c92aa1,// 196 PAY 193 

    0xb2f92f4e,// 197 PAY 194 

    0x4f3e3a3b,// 198 PAY 195 

    0xd67949c1,// 199 PAY 196 

    0x2a5f2927,// 200 PAY 197 

    0x16fdf7fc,// 201 PAY 198 

    0xc7e5a0c9,// 202 PAY 199 

    0xc8bcd42d,// 203 PAY 200 

    0xfab67935,// 204 PAY 201 

    0xf48c16ba,// 205 PAY 202 

    0xd988788a,// 206 PAY 203 

    0x88697738,// 207 PAY 204 

    0x96c0e8bf,// 208 PAY 205 

    0x18834df7,// 209 PAY 206 

    0xad5b163f,// 210 PAY 207 

    0xa20a9405,// 211 PAY 208 

    0xc4e3530b,// 212 PAY 209 

    0xa4815b32,// 213 PAY 210 

    0x06220567,// 214 PAY 211 

    0xc83b3e39,// 215 PAY 212 

    0xe4840966,// 216 PAY 213 

    0x7e18622e,// 217 PAY 214 

    0xddde866c,// 218 PAY 215 

    0x8e448dc9,// 219 PAY 216 

    0xfb8d51a3,// 220 PAY 217 

    0xf07b7470,// 221 PAY 218 

    0xee2e661b,// 222 PAY 219 

    0xc56bf53f,// 223 PAY 220 

    0x551aaf17,// 224 PAY 221 

    0x92327589,// 225 PAY 222 

    0xd4a59a11,// 226 PAY 223 

    0x1432fe7d,// 227 PAY 224 

    0x2ca9256a,// 228 PAY 225 

    0x12443a61,// 229 PAY 226 

    0x2e543867,// 230 PAY 227 

    0x38b0535a,// 231 PAY 228 

    0xefaf3f9e,// 232 PAY 229 

    0xba340ac8,// 233 PAY 230 

    0x2d488251,// 234 PAY 231 

    0x370da4a3,// 235 PAY 232 

    0xbd2f6847,// 236 PAY 233 

    0x688d5a54,// 237 PAY 234 

    0x3ee28453,// 238 PAY 235 

    0xbcf2d16d,// 239 PAY 236 

    0x308cdf6e,// 240 PAY 237 

    0x95cb0436,// 241 PAY 238 

    0xa1cc0897,// 242 PAY 239 

    0x578dce5f,// 243 PAY 240 

    0x9e14e78f,// 244 PAY 241 

    0x2c046c5f,// 245 PAY 242 

    0x3d9d21db,// 246 PAY 243 

    0x484309f9,// 247 PAY 244 

    0x1c54143c,// 248 PAY 245 

    0x3242af52,// 249 PAY 246 

    0x7ed3a3e7,// 250 PAY 247 

    0x182a9e19,// 251 PAY 248 

    0x3870b9e1,// 252 PAY 249 

    0xcac12dc2,// 253 PAY 250 

    0xc8a2d2c8,// 254 PAY 251 

    0x6a8bb317,// 255 PAY 252 

    0xe792e2f7,// 256 PAY 253 

    0x762bccff,// 257 PAY 254 

    0x3cc91118,// 258 PAY 255 

    0x47578617,// 259 PAY 256 

    0x8c742c85,// 260 PAY 257 

    0x2acbf70f,// 261 PAY 258 

    0xf5f4378b,// 262 PAY 259 

    0xdd4c9b92,// 263 PAY 260 

    0xf64d9c6b,// 264 PAY 261 

    0x1f0877c4,// 265 PAY 262 

    0x17316e4e,// 266 PAY 263 

    0xbd9f60ae,// 267 PAY 264 

    0xe168efe8,// 268 PAY 265 

    0x201e18ea,// 269 PAY 266 

    0xb6be696b,// 270 PAY 267 

    0x26d1bf9b,// 271 PAY 268 

    0x56c3bfa0,// 272 PAY 269 

    0xcf302eb4,// 273 PAY 270 

    0x0e30f959,// 274 PAY 271 

    0x44220191,// 275 PAY 272 

    0x72ffcca6,// 276 PAY 273 

    0x85e8e7e4,// 277 PAY 274 

    0x26a36b52,// 278 PAY 275 

    0x9d4660d7,// 279 PAY 276 

    0x7106584a,// 280 PAY 277 

    0xb7a77bf4,// 281 PAY 278 

    0xb03aec67,// 282 PAY 279 

    0x9583e68e,// 283 PAY 280 

    0x6c761c7c,// 284 PAY 281 

    0xa721fb50,// 285 PAY 282 

    0xd482ed86,// 286 PAY 283 

    0x4095d279,// 287 PAY 284 

    0x85e1000c,// 288 PAY 285 

    0xc04a1a5c,// 289 PAY 286 

    0xcbfb6eae,// 290 PAY 287 

    0x49e54a93,// 291 PAY 288 

    0x1cee031c,// 292 PAY 289 

    0x9f632453,// 293 PAY 290 

    0xf7e9470e,// 294 PAY 291 

    0x9c020c45,// 295 PAY 292 

    0x3227fc3d,// 296 PAY 293 

    0x9e8a0fe7,// 297 PAY 294 

    0x84c83bdc,// 298 PAY 295 

    0xa176eea1,// 299 PAY 296 

    0x8af0b079,// 300 PAY 297 

    0xe068ee4e,// 301 PAY 298 

    0x274fba59,// 302 PAY 299 

    0xa93e9ee2,// 303 PAY 300 

    0x71dcb54c,// 304 PAY 301 

    0x01980e08,// 305 PAY 302 

    0x66ab7e5a,// 306 PAY 303 

    0x2174e03c,// 307 PAY 304 

    0x6f7719ae,// 308 PAY 305 

    0x51d6f47f,// 309 PAY 306 

    0x48b6d299,// 310 PAY 307 

    0xe0117d0c,// 311 PAY 308 

    0xacb1c578,// 312 PAY 309 

    0x1be9aad3,// 313 PAY 310 

    0x54d806f7,// 314 PAY 311 

    0x9b01eac1,// 315 PAY 312 

    0x02edebd4,// 316 PAY 313 

    0x3ef34c60,// 317 PAY 314 

    0x9be1fbf7,// 318 PAY 315 

    0x482b3e85,// 319 PAY 316 

    0xa61b26ad,// 320 PAY 317 

    0xf79debf2,// 321 PAY 318 

    0x39abb782,// 322 PAY 319 

    0x49043f9b,// 323 PAY 320 

    0xa0ed345d,// 324 PAY 321 

    0xc19299e8,// 325 PAY 322 

    0xda82b3d5,// 326 PAY 323 

    0xd1798287,// 327 PAY 324 

    0xae7cae37,// 328 PAY 325 

    0x4b9c1051,// 329 PAY 326 

    0x3cecf81c,// 330 PAY 327 

    0x2a46f716,// 331 PAY 328 

    0x884b2474,// 332 PAY 329 

    0xa13a5776,// 333 PAY 330 

    0x799451c1,// 334 PAY 331 

    0x5ddb1714,// 335 PAY 332 

    0xd6328035,// 336 PAY 333 

    0x40ee3941,// 337 PAY 334 

    0x18f8c0be,// 338 PAY 335 

    0x609ffd5e,// 339 PAY 336 

    0xd465c598,// 340 PAY 337 

    0x8273755b,// 341 PAY 338 

    0x437afbc1,// 342 PAY 339 

    0x94d7f7f4,// 343 PAY 340 

    0xc73104ed,// 344 PAY 341 

    0xcc60bd6b,// 345 PAY 342 

    0x3217fed0,// 346 PAY 343 

    0x6c015a9d,// 347 PAY 344 

    0x691bed09,// 348 PAY 345 

    0xd487584e,// 349 PAY 346 

    0x36ce2fb7,// 350 PAY 347 

    0x960ddaac,// 351 PAY 348 

    0x1a38e754,// 352 PAY 349 

    0x4f85cd47,// 353 PAY 350 

    0x3b7769a3,// 354 PAY 351 

    0x3c292992,// 355 PAY 352 

    0x1c596e3e,// 356 PAY 353 

    0x5d72d9bb,// 357 PAY 354 

    0xe949f9de,// 358 PAY 355 

    0x7d9238ec,// 359 PAY 356 

    0x8d20709b,// 360 PAY 357 

    0xf16ba776,// 361 PAY 358 

    0x37ee35f7,// 362 PAY 359 

    0xfce72a3f,// 363 PAY 360 

    0x62c00fe9,// 364 PAY 361 

    0xd93f44a1,// 365 PAY 362 

    0x88a0675a,// 366 PAY 363 

    0x3cb21dc2,// 367 PAY 364 

    0x348db55e,// 368 PAY 365 

    0x538f88c5,// 369 PAY 366 

    0x894437d1,// 370 PAY 367 

    0xfb553156,// 371 PAY 368 

    0xf702b54c,// 372 PAY 369 

    0x197211a8,// 373 PAY 370 

    0xedd9af4a,// 374 PAY 371 

    0xbc28328d,// 375 PAY 372 

    0xd86f9873,// 376 PAY 373 

    0x773cbc0b,// 377 PAY 374 

    0x01f161b5,// 378 PAY 375 

    0x89614a0f,// 379 PAY 376 

    0x8fa6b2ad,// 380 PAY 377 

    0x1b7f8bf2,// 381 PAY 378 

    0x87ffd5f5,// 382 PAY 379 

    0xb88715cd,// 383 PAY 380 

    0x71a43545,// 384 PAY 381 

    0xcd6e2561,// 385 PAY 382 

    0x60223309,// 386 PAY 383 

    0x48c53c8a,// 387 PAY 384 

    0xac0c211b,// 388 PAY 385 

    0x624ee381,// 389 PAY 386 

    0xc0097bc3,// 390 PAY 387 

    0x3c1d74a8,// 391 PAY 388 

    0x6f89b449,// 392 PAY 389 

    0xec0018fd,// 393 PAY 390 

    0x977a52ef,// 394 PAY 391 

    0xe3bb7cfc,// 395 PAY 392 

    0x2e56a8d4,// 396 PAY 393 

    0xef5e05f8,// 397 PAY 394 

    0xfddd61e9,// 398 PAY 395 

    0x84322ed4,// 399 PAY 396 

    0x56e1fff9,// 400 PAY 397 

    0x634f1d73,// 401 PAY 398 

    0xb6f042f8,// 402 PAY 399 

    0x9c754e20,// 403 PAY 400 

    0xa527b7e4,// 404 PAY 401 

    0x64097978,// 405 PAY 402 

    0xf27caa08,// 406 PAY 403 

    0x7fb9ceba,// 407 PAY 404 

    0x762e4eb3,// 408 PAY 405 

    0x1dd1b64a,// 409 PAY 406 

    0x3f55444b,// 410 PAY 407 

    0xdb92f00a,// 411 PAY 408 

    0x1c973711,// 412 PAY 409 

    0x52652dff,// 413 PAY 410 

    0xe03fd1c8,// 414 PAY 411 

    0x5f9ccdc3,// 415 PAY 412 

    0x24c13386,// 416 PAY 413 

    0x9fd22ac0,// 417 PAY 414 

    0xa8b84ffc,// 418 PAY 415 

    0xb02e9a3b,// 419 PAY 416 

    0x2cc0ed1f,// 420 PAY 417 

    0x666ba837,// 421 PAY 418 

    0xd234d0d9,// 422 PAY 419 

    0xbe96a59c,// 423 PAY 420 

    0x70b3a743,// 424 PAY 421 

    0xccb088ff,// 425 PAY 422 

    0x56cd3919,// 426 PAY 423 

    0xa3946e78,// 427 PAY 424 

    0x9fe74185,// 428 PAY 425 

    0xd2e8070a,// 429 PAY 426 

    0x8d95a628,// 430 PAY 427 

    0x39edc4fd,// 431 PAY 428 

    0xc2578bd0,// 432 PAY 429 

    0xa521599c,// 433 PAY 430 

    0x320f5694,// 434 PAY 431 

    0x91ffe11f,// 435 PAY 432 

    0xeaf8a0fd,// 436 PAY 433 

    0x9792a92d,// 437 PAY 434 

    0x4185a8ee,// 438 PAY 435 

    0x7e4ce430,// 439 PAY 436 

    0x282d1760,// 440 PAY 437 

    0x0feb0843,// 441 PAY 438 

    0x4d725ce4,// 442 PAY 439 

    0x66e420e8,// 443 PAY 440 

    0x09096b85,// 444 PAY 441 

    0xac0bbd21,// 445 PAY 442 

    0xc0191a3d,// 446 PAY 443 

    0x34584775,// 447 PAY 444 

    0xcf24edbb,// 448 PAY 445 

    0xba02727f,// 449 PAY 446 

    0x21889932,// 450 PAY 447 

    0x3773d834,// 451 PAY 448 

    0x227fef74,// 452 PAY 449 

    0x2d53b5c1,// 453 PAY 450 

    0x7677f90c,// 454 PAY 451 

    0xeb32b454,// 455 PAY 452 

    0xcb7777eb,// 456 PAY 453 

    0x13bccb8d,// 457 PAY 454 

    0x257e353d,// 458 PAY 455 

    0x16a20b2a,// 459 PAY 456 

    0x8efe0509,// 460 PAY 457 

    0x567af12d,// 461 PAY 458 

    0x5ddb0157,// 462 PAY 459 

    0x702a3bd3,// 463 PAY 460 

    0xe6c18379,// 464 PAY 461 

    0xc4f58f20,// 465 PAY 462 

    0x3e4abd62,// 466 PAY 463 

    0xef77c193,// 467 PAY 464 

    0x57281e7f,// 468 PAY 465 

    0x0fb91165,// 469 PAY 466 

    0x9ae8fc9c,// 470 PAY 467 

    0x431cb11e,// 471 PAY 468 

    0x91a8d1b3,// 472 PAY 469 

    0xb344ca6b,// 473 PAY 470 

    0x8d17d051,// 474 PAY 471 

    0x06272cbd,// 475 PAY 472 

    0x9eb1a991,// 476 PAY 473 

    0x46347585,// 477 PAY 474 

    0xae7c11e3,// 478 PAY 475 

    0xe99b3791,// 479 PAY 476 

    0x08b5c582,// 480 PAY 477 

    0xb0eaa131,// 481 PAY 478 

    0x4ac907ae,// 482 PAY 479 

    0x06ac7f2b,// 483 PAY 480 

    0xbd51ed4c,// 484 PAY 481 

    0x2f74715a,// 485 PAY 482 

    0x2677abf5,// 486 PAY 483 

    0x968ae67d,// 487 PAY 484 

    0x5f67caa1,// 488 PAY 485 

    0x35c25061,// 489 PAY 486 

    0xa6781d07,// 490 PAY 487 

    0xb21f6f28,// 491 PAY 488 

    0xf25a8bd0,// 492 PAY 489 

    0xbf1b2863,// 493 PAY 490 

    0xa0dcc397,// 494 PAY 491 

    0x773b16f4,// 495 PAY 492 

    0xe2beffef,// 496 PAY 493 

    0x330c9d0f,// 497 PAY 494 

    0xd2a971be,// 498 PAY 495 

    0x1a9f7b18,// 499 PAY 496 

    0x7196bd64,// 500 PAY 497 

    0x10e874a6,// 501 PAY 498 

    0x5958fbf7,// 502 PAY 499 

    0x81000000,// 503 PAY 500 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5e 

    0x00d55e7e // 504 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt86_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 82 words. 

/// BDA size     is 323 (0x143) 

/// BDA id       is 0xf6c2 

    0x0143f6c2,// 3 BDA   1 

/// PAY Generic Data size   : 323 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xc59bbf0d,// 4 PAY   1 

    0x0225f841,// 5 PAY   2 

    0x502c9168,// 6 PAY   3 

    0xc611e4eb,// 7 PAY   4 

    0x6e942304,// 8 PAY   5 

    0x256d8bcc,// 9 PAY   6 

    0x388443b2,// 10 PAY   7 

    0x4b86da18,// 11 PAY   8 

    0xdf920289,// 12 PAY   9 

    0xe5a13042,// 13 PAY  10 

    0x95880083,// 14 PAY  11 

    0x015f8dd8,// 15 PAY  12 

    0x4a57ff18,// 16 PAY  13 

    0x7053c84c,// 17 PAY  14 

    0x1a225ddf,// 18 PAY  15 

    0x92f3575e,// 19 PAY  16 

    0x80d5506b,// 20 PAY  17 

    0xb79b498e,// 21 PAY  18 

    0x56aaf465,// 22 PAY  19 

    0x763f5164,// 23 PAY  20 

    0x3cc00cac,// 24 PAY  21 

    0xac93b60e,// 25 PAY  22 

    0x1ab596f2,// 26 PAY  23 

    0xe49bc0ec,// 27 PAY  24 

    0x5b55e49a,// 28 PAY  25 

    0xc39e2466,// 29 PAY  26 

    0x371d2fa9,// 30 PAY  27 

    0x65db5e99,// 31 PAY  28 

    0x7376412b,// 32 PAY  29 

    0xf2eb0b24,// 33 PAY  30 

    0x83f3f9b9,// 34 PAY  31 

    0x41ecebc5,// 35 PAY  32 

    0xd1a91e8c,// 36 PAY  33 

    0x3287a68a,// 37 PAY  34 

    0x1469c2a8,// 38 PAY  35 

    0x5e9fd187,// 39 PAY  36 

    0x1512760f,// 40 PAY  37 

    0xb4b5415d,// 41 PAY  38 

    0x2b4fce6c,// 42 PAY  39 

    0x94459f14,// 43 PAY  40 

    0x3d6f76a8,// 44 PAY  41 

    0x9f3a4ba3,// 45 PAY  42 

    0x485cf66e,// 46 PAY  43 

    0x7cf0c646,// 47 PAY  44 

    0xdc8dfd79,// 48 PAY  45 

    0xd453fd7d,// 49 PAY  46 

    0x0d46d79d,// 50 PAY  47 

    0x722a97eb,// 51 PAY  48 

    0xbad4ffef,// 52 PAY  49 

    0xd7009fab,// 53 PAY  50 

    0x533df91f,// 54 PAY  51 

    0xeefbde53,// 55 PAY  52 

    0x1f5c008d,// 56 PAY  53 

    0x2522f7a4,// 57 PAY  54 

    0xaa2c3f4d,// 58 PAY  55 

    0xe058b36a,// 59 PAY  56 

    0x15d0bdfe,// 60 PAY  57 

    0x1baa9cba,// 61 PAY  58 

    0x946dc64c,// 62 PAY  59 

    0xe5c24e0c,// 63 PAY  60 

    0x1ced85a9,// 64 PAY  61 

    0xead2b6cd,// 65 PAY  62 

    0x73799a5d,// 66 PAY  63 

    0x61e056ae,// 67 PAY  64 

    0xaa357f8a,// 68 PAY  65 

    0x31d7ec6a,// 69 PAY  66 

    0x8435b859,// 70 PAY  67 

    0x9bef4ef2,// 71 PAY  68 

    0x0e68677b,// 72 PAY  69 

    0x1079e7d1,// 73 PAY  70 

    0xf8958ce1,// 74 PAY  71 

    0xa764a0fe,// 75 PAY  72 

    0x32681d81,// 76 PAY  73 

    0xb6a9f3ab,// 77 PAY  74 

    0xda714def,// 78 PAY  75 

    0xc312e702,// 79 PAY  76 

    0x9aeddcda,// 80 PAY  77 

    0x25ad37dc,// 81 PAY  78 

    0xdc98a148,// 82 PAY  79 

    0x0b783d1b,// 83 PAY  80 

    0xc6123100,// 84 PAY  81 

/// HASH is  12 bytes 

    0x1baa9cba,// 85 HSH   1 

    0x946dc64c,// 86 HSH   2 

    0xe5c24e0c,// 87 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 233 

/// STA pkt_idx        : 239 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x03bc5be9 // 88 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt87_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 66 words. 

/// BDA size     is 258 (0x102) 

/// BDA id       is 0x316c 

    0x0102316c,// 3 BDA   1 

/// PAY Generic Data size   : 258 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xf992bb71,// 4 PAY   1 

    0xd30abd4e,// 5 PAY   2 

    0x07511a45,// 6 PAY   3 

    0xa1eafe6b,// 7 PAY   4 

    0xe178ba9c,// 8 PAY   5 

    0xcefbc0c7,// 9 PAY   6 

    0xbb8427a6,// 10 PAY   7 

    0x1327f589,// 11 PAY   8 

    0x2ce62661,// 12 PAY   9 

    0xb36f2d63,// 13 PAY  10 

    0xf449527b,// 14 PAY  11 

    0x96287b2b,// 15 PAY  12 

    0x74ab2ca7,// 16 PAY  13 

    0xc00938a6,// 17 PAY  14 

    0x45c18ff1,// 18 PAY  15 

    0xfd646c6c,// 19 PAY  16 

    0x8276b1d5,// 20 PAY  17 

    0x7ddb86a7,// 21 PAY  18 

    0x78992c6f,// 22 PAY  19 

    0xfe4f6ec9,// 23 PAY  20 

    0x1b2f045e,// 24 PAY  21 

    0xe6a32a56,// 25 PAY  22 

    0xfbda0ea2,// 26 PAY  23 

    0x39c51962,// 27 PAY  24 

    0x1cd48ab5,// 28 PAY  25 

    0x8366407d,// 29 PAY  26 

    0x869bcdc3,// 30 PAY  27 

    0xfee07a8b,// 31 PAY  28 

    0xdcc25534,// 32 PAY  29 

    0xf69c3851,// 33 PAY  30 

    0xb23491a0,// 34 PAY  31 

    0xcd44c768,// 35 PAY  32 

    0xff08f18b,// 36 PAY  33 

    0xc453c019,// 37 PAY  34 

    0xf04cab9e,// 38 PAY  35 

    0x7d37a884,// 39 PAY  36 

    0xf9a53e13,// 40 PAY  37 

    0x57420112,// 41 PAY  38 

    0x284ed7d0,// 42 PAY  39 

    0x8d2a964a,// 43 PAY  40 

    0xc898c630,// 44 PAY  41 

    0xb07bc2bc,// 45 PAY  42 

    0xa704cfc3,// 46 PAY  43 

    0x17fc1ce5,// 47 PAY  44 

    0xde6e036a,// 48 PAY  45 

    0x891a9ce7,// 49 PAY  46 

    0xe9ab26ed,// 50 PAY  47 

    0xeab29391,// 51 PAY  48 

    0x010c3a35,// 52 PAY  49 

    0x6939e17e,// 53 PAY  50 

    0xe84cc0b3,// 54 PAY  51 

    0xc5730547,// 55 PAY  52 

    0x3e8888d5,// 56 PAY  53 

    0xe3fc863b,// 57 PAY  54 

    0x9d0493c2,// 58 PAY  55 

    0xf1e8bebf,// 59 PAY  56 

    0x9b8954ba,// 60 PAY  57 

    0x02dd5d00,// 61 PAY  58 

    0xac59bb95,// 62 PAY  59 

    0x15547158,// 63 PAY  60 

    0x1e0dbe22,// 64 PAY  61 

    0x6bbe6d53,// 65 PAY  62 

    0x24ce6ab9,// 66 PAY  63 

    0x204ed858,// 67 PAY  64 

    0x0ede0000,// 68 PAY  65 

/// STA is 1 words. 

/// STA num_pkts       : 189 

/// STA pkt_idx        : 136 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcb 

    0x0220cbbd // 69 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt88_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 497 words. 

/// BDA size     is 1983 (0x7bf) 

/// BDA id       is 0x441 

    0x07bf0441,// 3 BDA   1 

/// PAY Generic Data size   : 1983 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xdb4bbc4c,// 4 PAY   1 

    0x035a6412,// 5 PAY   2 

    0x2b44f83e,// 6 PAY   3 

    0xb135c302,// 7 PAY   4 

    0x7e2249e9,// 8 PAY   5 

    0xf2df3f20,// 9 PAY   6 

    0xbe0f4a58,// 10 PAY   7 

    0x60f19455,// 11 PAY   8 

    0x8a0c66d4,// 12 PAY   9 

    0x8a0cc0b0,// 13 PAY  10 

    0xb2c9c9bd,// 14 PAY  11 

    0xec6f50fb,// 15 PAY  12 

    0xa84afffb,// 16 PAY  13 

    0x7de1c23b,// 17 PAY  14 

    0xb6da6bd7,// 18 PAY  15 

    0xe5bab197,// 19 PAY  16 

    0x80b2d08d,// 20 PAY  17 

    0x3fdeac9f,// 21 PAY  18 

    0x8aad85a2,// 22 PAY  19 

    0x36977b71,// 23 PAY  20 

    0x9f9e79de,// 24 PAY  21 

    0x39bcdee4,// 25 PAY  22 

    0x3a7e3af4,// 26 PAY  23 

    0xfd7e3238,// 27 PAY  24 

    0x76d059f6,// 28 PAY  25 

    0x42b88940,// 29 PAY  26 

    0xedb4b2cf,// 30 PAY  27 

    0xfda78aa0,// 31 PAY  28 

    0xa7882a0f,// 32 PAY  29 

    0xafc60964,// 33 PAY  30 

    0x2262a9b3,// 34 PAY  31 

    0xcfdf8061,// 35 PAY  32 

    0xae36b1f4,// 36 PAY  33 

    0xab69e80e,// 37 PAY  34 

    0xee852db2,// 38 PAY  35 

    0x6279d550,// 39 PAY  36 

    0xe4ca4f93,// 40 PAY  37 

    0x46777a9f,// 41 PAY  38 

    0x3c330961,// 42 PAY  39 

    0xd5a722fa,// 43 PAY  40 

    0xbbb03d76,// 44 PAY  41 

    0xdd7a13ea,// 45 PAY  42 

    0xe999a1fd,// 46 PAY  43 

    0x140e9d59,// 47 PAY  44 

    0x6bdb9de5,// 48 PAY  45 

    0x14c7c219,// 49 PAY  46 

    0xdd777de4,// 50 PAY  47 

    0x87ec4b01,// 51 PAY  48 

    0x94aee0f8,// 52 PAY  49 

    0x4e54601c,// 53 PAY  50 

    0xf868aff7,// 54 PAY  51 

    0xd7147456,// 55 PAY  52 

    0x10afdb20,// 56 PAY  53 

    0xddf83fbe,// 57 PAY  54 

    0xcfefefc9,// 58 PAY  55 

    0xaae30b20,// 59 PAY  56 

    0x2a3b0ef7,// 60 PAY  57 

    0xa38702a2,// 61 PAY  58 

    0xc0259c57,// 62 PAY  59 

    0xed21bf4b,// 63 PAY  60 

    0xf1aad939,// 64 PAY  61 

    0x93e1f34c,// 65 PAY  62 

    0xe29a6efe,// 66 PAY  63 

    0x0d8f6082,// 67 PAY  64 

    0x5ff14f72,// 68 PAY  65 

    0xbf4b3a11,// 69 PAY  66 

    0x4a0abb2c,// 70 PAY  67 

    0xd7dad8d3,// 71 PAY  68 

    0x1723dc9d,// 72 PAY  69 

    0xb7195a5e,// 73 PAY  70 

    0xae8e1a5e,// 74 PAY  71 

    0x9b9b6700,// 75 PAY  72 

    0x92600b6b,// 76 PAY  73 

    0x5ff5308a,// 77 PAY  74 

    0xa2dfe733,// 78 PAY  75 

    0xcc0ba393,// 79 PAY  76 

    0x335d39f9,// 80 PAY  77 

    0xacb386f1,// 81 PAY  78 

    0x311540b0,// 82 PAY  79 

    0x6392af58,// 83 PAY  80 

    0x51efff50,// 84 PAY  81 

    0x520ac208,// 85 PAY  82 

    0xaad551a7,// 86 PAY  83 

    0x6c536615,// 87 PAY  84 

    0xa92790c6,// 88 PAY  85 

    0x2b9b5a7b,// 89 PAY  86 

    0x2da50b72,// 90 PAY  87 

    0x23e1a2d3,// 91 PAY  88 

    0xa3a4ae1a,// 92 PAY  89 

    0x85715d1c,// 93 PAY  90 

    0x2f527f5a,// 94 PAY  91 

    0x14bdde67,// 95 PAY  92 

    0x5b664941,// 96 PAY  93 

    0x14908e95,// 97 PAY  94 

    0x7b90c7f9,// 98 PAY  95 

    0x572bf281,// 99 PAY  96 

    0x6d20cdba,// 100 PAY  97 

    0xe300eda6,// 101 PAY  98 

    0xe99619b3,// 102 PAY  99 

    0xd5f82c60,// 103 PAY 100 

    0xcd22178c,// 104 PAY 101 

    0x5c882a41,// 105 PAY 102 

    0xef8adbab,// 106 PAY 103 

    0x1d92e479,// 107 PAY 104 

    0x5e7bab02,// 108 PAY 105 

    0xf1b77e7b,// 109 PAY 106 

    0x5c340b2f,// 110 PAY 107 

    0x28060ae5,// 111 PAY 108 

    0xdeb3d968,// 112 PAY 109 

    0xcd5d8249,// 113 PAY 110 

    0xed15cd0d,// 114 PAY 111 

    0x6ff70ccb,// 115 PAY 112 

    0x11eed592,// 116 PAY 113 

    0x1420e20b,// 117 PAY 114 

    0x4ee68569,// 118 PAY 115 

    0x4a57d9e6,// 119 PAY 116 

    0xa2e4ce15,// 120 PAY 117 

    0xa66870f1,// 121 PAY 118 

    0xb087f892,// 122 PAY 119 

    0x6a33c843,// 123 PAY 120 

    0x928afdb5,// 124 PAY 121 

    0x4d09487e,// 125 PAY 122 

    0x56ffea3e,// 126 PAY 123 

    0x55cd7c86,// 127 PAY 124 

    0x3498968a,// 128 PAY 125 

    0x5c618c12,// 129 PAY 126 

    0xbadf01f9,// 130 PAY 127 

    0x854ee71f,// 131 PAY 128 

    0x7e3264d1,// 132 PAY 129 

    0xc5cac478,// 133 PAY 130 

    0x46092d9e,// 134 PAY 131 

    0xa2481faa,// 135 PAY 132 

    0x0ef62307,// 136 PAY 133 

    0x7a4a3ccd,// 137 PAY 134 

    0x6844ae72,// 138 PAY 135 

    0xfeee50ef,// 139 PAY 136 

    0x4a0bcee4,// 140 PAY 137 

    0x1669c85e,// 141 PAY 138 

    0x12c125ea,// 142 PAY 139 

    0x04245e88,// 143 PAY 140 

    0x43c09e2e,// 144 PAY 141 

    0xf5986b0f,// 145 PAY 142 

    0x6b84db9c,// 146 PAY 143 

    0x501297b4,// 147 PAY 144 

    0x21395d6c,// 148 PAY 145 

    0x5cbd2792,// 149 PAY 146 

    0x4f2cd0ac,// 150 PAY 147 

    0x9e1399f1,// 151 PAY 148 

    0xae9723f5,// 152 PAY 149 

    0xc7e3c21a,// 153 PAY 150 

    0xa78da103,// 154 PAY 151 

    0x1b7baef8,// 155 PAY 152 

    0x248243f1,// 156 PAY 153 

    0xb8e4279c,// 157 PAY 154 

    0x7ff6ee36,// 158 PAY 155 

    0x6fe9c5ce,// 159 PAY 156 

    0x5dd0d560,// 160 PAY 157 

    0xc14c0249,// 161 PAY 158 

    0x018a2d8d,// 162 PAY 159 

    0x447d00b9,// 163 PAY 160 

    0x43040e94,// 164 PAY 161 

    0xcd3ab1a7,// 165 PAY 162 

    0x270b2187,// 166 PAY 163 

    0xf4fd052d,// 167 PAY 164 

    0x1c4732be,// 168 PAY 165 

    0x9cd233ac,// 169 PAY 166 

    0xf0f68511,// 170 PAY 167 

    0x4f4fdbbe,// 171 PAY 168 

    0xd9dd1262,// 172 PAY 169 

    0x54173ba1,// 173 PAY 170 

    0x1e0218fd,// 174 PAY 171 

    0x13ee3fcd,// 175 PAY 172 

    0xfca0e32d,// 176 PAY 173 

    0x175f4b32,// 177 PAY 174 

    0x557f8214,// 178 PAY 175 

    0x0a073bfb,// 179 PAY 176 

    0x69926f2c,// 180 PAY 177 

    0xef1dc559,// 181 PAY 178 

    0x59f7daed,// 182 PAY 179 

    0x788a96bc,// 183 PAY 180 

    0x671972a9,// 184 PAY 181 

    0x5d9e4d4a,// 185 PAY 182 

    0x3e4da874,// 186 PAY 183 

    0xd11cc43f,// 187 PAY 184 

    0x31a35934,// 188 PAY 185 

    0x12284260,// 189 PAY 186 

    0xdcd293de,// 190 PAY 187 

    0xd0fc72aa,// 191 PAY 188 

    0x71e325df,// 192 PAY 189 

    0xc28521ee,// 193 PAY 190 

    0x6f8c8eb6,// 194 PAY 191 

    0xff633c45,// 195 PAY 192 

    0x56f84a2c,// 196 PAY 193 

    0x1f9705ba,// 197 PAY 194 

    0xd142f485,// 198 PAY 195 

    0x909c79e8,// 199 PAY 196 

    0x6bd046d6,// 200 PAY 197 

    0xd21d2fea,// 201 PAY 198 

    0xe521f8c3,// 202 PAY 199 

    0x88e5a38b,// 203 PAY 200 

    0xd9b92207,// 204 PAY 201 

    0x29eca6e0,// 205 PAY 202 

    0xe6e1a723,// 206 PAY 203 

    0x46e944e2,// 207 PAY 204 

    0xf6a9dfd1,// 208 PAY 205 

    0xfb150425,// 209 PAY 206 

    0x36a1c73d,// 210 PAY 207 

    0x1c12f23b,// 211 PAY 208 

    0x4a7d8386,// 212 PAY 209 

    0xb0444711,// 213 PAY 210 

    0x4a19588c,// 214 PAY 211 

    0x51c819cd,// 215 PAY 212 

    0xd18ca433,// 216 PAY 213 

    0xbaed1e5e,// 217 PAY 214 

    0x23170e0c,// 218 PAY 215 

    0x59f3172a,// 219 PAY 216 

    0xa2599579,// 220 PAY 217 

    0xc2359d5c,// 221 PAY 218 

    0x0a74fcf4,// 222 PAY 219 

    0x23ef5473,// 223 PAY 220 

    0x6e8dfe20,// 224 PAY 221 

    0x3872cea7,// 225 PAY 222 

    0x2c11878e,// 226 PAY 223 

    0xeae37ae2,// 227 PAY 224 

    0xdaa3772a,// 228 PAY 225 

    0x75f9efd1,// 229 PAY 226 

    0xdbe1d59a,// 230 PAY 227 

    0x3f651cdb,// 231 PAY 228 

    0xeba98aa9,// 232 PAY 229 

    0xf34f09ba,// 233 PAY 230 

    0xb02dd938,// 234 PAY 231 

    0x4ea74df8,// 235 PAY 232 

    0x0cf49eef,// 236 PAY 233 

    0x6e816aa8,// 237 PAY 234 

    0x0b32234f,// 238 PAY 235 

    0xee701258,// 239 PAY 236 

    0xc433fd04,// 240 PAY 237 

    0x5260b796,// 241 PAY 238 

    0x052ade5b,// 242 PAY 239 

    0x41a966ad,// 243 PAY 240 

    0x85105bc7,// 244 PAY 241 

    0x7597db59,// 245 PAY 242 

    0xbd960b34,// 246 PAY 243 

    0x9a86b8bd,// 247 PAY 244 

    0x938f18d5,// 248 PAY 245 

    0xa502a7ed,// 249 PAY 246 

    0x1c74eb5a,// 250 PAY 247 

    0x62f273d8,// 251 PAY 248 

    0x37fa9063,// 252 PAY 249 

    0xed9f424a,// 253 PAY 250 

    0xf5a970fe,// 254 PAY 251 

    0x5f165cc4,// 255 PAY 252 

    0xe16a270c,// 256 PAY 253 

    0x0b3c2733,// 257 PAY 254 

    0xd5745bb8,// 258 PAY 255 

    0x0ee00a28,// 259 PAY 256 

    0x06733a55,// 260 PAY 257 

    0x428b7432,// 261 PAY 258 

    0x74d23e01,// 262 PAY 259 

    0x2be56125,// 263 PAY 260 

    0x8a400792,// 264 PAY 261 

    0x028ae596,// 265 PAY 262 

    0xb20875b3,// 266 PAY 263 

    0xc00bf066,// 267 PAY 264 

    0x21477532,// 268 PAY 265 

    0x62261918,// 269 PAY 266 

    0xd33b9a9f,// 270 PAY 267 

    0xd0d3ab74,// 271 PAY 268 

    0xb1d22169,// 272 PAY 269 

    0x13dec11e,// 273 PAY 270 

    0x39318545,// 274 PAY 271 

    0xe6e4505b,// 275 PAY 272 

    0x941ff45e,// 276 PAY 273 

    0x66063f3a,// 277 PAY 274 

    0x150466df,// 278 PAY 275 

    0x181ce2cd,// 279 PAY 276 

    0x4a52a3a8,// 280 PAY 277 

    0x9c7f3a54,// 281 PAY 278 

    0xe52031d9,// 282 PAY 279 

    0xd5d053b7,// 283 PAY 280 

    0xfa09ad09,// 284 PAY 281 

    0x22ccfadc,// 285 PAY 282 

    0xda0417bf,// 286 PAY 283 

    0x791e92a1,// 287 PAY 284 

    0xf21c84b9,// 288 PAY 285 

    0x47343679,// 289 PAY 286 

    0xdc2991b8,// 290 PAY 287 

    0x98003b30,// 291 PAY 288 

    0x60f6bc5b,// 292 PAY 289 

    0x08c3a8b5,// 293 PAY 290 

    0x58926b5d,// 294 PAY 291 

    0xf41cacb8,// 295 PAY 292 

    0x72b746bd,// 296 PAY 293 

    0xd9a0e195,// 297 PAY 294 

    0x0c6769e9,// 298 PAY 295 

    0x9743a9c5,// 299 PAY 296 

    0x5ce5e368,// 300 PAY 297 

    0x81c83c23,// 301 PAY 298 

    0x8558720b,// 302 PAY 299 

    0xde035db5,// 303 PAY 300 

    0x578c71dc,// 304 PAY 301 

    0x0bbb207f,// 305 PAY 302 

    0x971f810c,// 306 PAY 303 

    0x51b19796,// 307 PAY 304 

    0x3451cf20,// 308 PAY 305 

    0xe181a70f,// 309 PAY 306 

    0xf79503ad,// 310 PAY 307 

    0xb3747ef8,// 311 PAY 308 

    0x40bb9587,// 312 PAY 309 

    0x72c92e6e,// 313 PAY 310 

    0xc2a4e70a,// 314 PAY 311 

    0xe75805b1,// 315 PAY 312 

    0x07366d59,// 316 PAY 313 

    0xd1364d5c,// 317 PAY 314 

    0xc948b566,// 318 PAY 315 

    0xaba2c65c,// 319 PAY 316 

    0x9b22ceb0,// 320 PAY 317 

    0x96122060,// 321 PAY 318 

    0x0395e378,// 322 PAY 319 

    0x70cce60e,// 323 PAY 320 

    0xb4817395,// 324 PAY 321 

    0xb4c3760b,// 325 PAY 322 

    0xe11f99cc,// 326 PAY 323 

    0xd6467866,// 327 PAY 324 

    0x72975ebf,// 328 PAY 325 

    0x21322092,// 329 PAY 326 

    0xbbea0325,// 330 PAY 327 

    0x167ed5df,// 331 PAY 328 

    0xd0ad3359,// 332 PAY 329 

    0xd6a5bf45,// 333 PAY 330 

    0x33a8421b,// 334 PAY 331 

    0x006dccbd,// 335 PAY 332 

    0x459835f2,// 336 PAY 333 

    0x67ca43db,// 337 PAY 334 

    0xa36cba42,// 338 PAY 335 

    0x723184f1,// 339 PAY 336 

    0xa482902c,// 340 PAY 337 

    0x1b2b20d1,// 341 PAY 338 

    0x8691c5de,// 342 PAY 339 

    0xce49e9c3,// 343 PAY 340 

    0x42268132,// 344 PAY 341 

    0x783261be,// 345 PAY 342 

    0xa3277dde,// 346 PAY 343 

    0xb6b2c76f,// 347 PAY 344 

    0x4c5e5568,// 348 PAY 345 

    0x951e0a48,// 349 PAY 346 

    0x39559662,// 350 PAY 347 

    0xaed2ddd3,// 351 PAY 348 

    0x78e3fa26,// 352 PAY 349 

    0x2bf77fa6,// 353 PAY 350 

    0xa941e93a,// 354 PAY 351 

    0xc3fad5ba,// 355 PAY 352 

    0xb35004a6,// 356 PAY 353 

    0x2102bb5e,// 357 PAY 354 

    0xd8556deb,// 358 PAY 355 

    0x6e7c3ef5,// 359 PAY 356 

    0x0246f9dd,// 360 PAY 357 

    0x6201407d,// 361 PAY 358 

    0xa0abf3ff,// 362 PAY 359 

    0xae296958,// 363 PAY 360 

    0xf5277b33,// 364 PAY 361 

    0xdf8f7de9,// 365 PAY 362 

    0x30ddfa26,// 366 PAY 363 

    0x9134daaa,// 367 PAY 364 

    0x33fda7f6,// 368 PAY 365 

    0x62d65833,// 369 PAY 366 

    0xc2bc7eea,// 370 PAY 367 

    0xc37dadf8,// 371 PAY 368 

    0x32b3958b,// 372 PAY 369 

    0x5bd124cc,// 373 PAY 370 

    0x40202be2,// 374 PAY 371 

    0x871ca3b1,// 375 PAY 372 

    0x52e10f42,// 376 PAY 373 

    0x532ae8f9,// 377 PAY 374 

    0x5d94e2c3,// 378 PAY 375 

    0xcbce1a58,// 379 PAY 376 

    0x581f66b7,// 380 PAY 377 

    0x0a78793d,// 381 PAY 378 

    0x2e96d97e,// 382 PAY 379 

    0x28ba7b18,// 383 PAY 380 

    0xc0bd634c,// 384 PAY 381 

    0xe04a2b40,// 385 PAY 382 

    0xd72aac08,// 386 PAY 383 

    0xdf59188c,// 387 PAY 384 

    0x0f193466,// 388 PAY 385 

    0x2558bf65,// 389 PAY 386 

    0xb235b584,// 390 PAY 387 

    0xa9d25c1a,// 391 PAY 388 

    0x338fec95,// 392 PAY 389 

    0x8d513bfc,// 393 PAY 390 

    0xe8cb4bc9,// 394 PAY 391 

    0xb5345f33,// 395 PAY 392 

    0x9aa018fd,// 396 PAY 393 

    0xcbb903aa,// 397 PAY 394 

    0x28bf8045,// 398 PAY 395 

    0xee8a344f,// 399 PAY 396 

    0x53e596b2,// 400 PAY 397 

    0x1a90ef55,// 401 PAY 398 

    0x084eb40a,// 402 PAY 399 

    0xe26da42f,// 403 PAY 400 

    0x726b9135,// 404 PAY 401 

    0x557df7f9,// 405 PAY 402 

    0xae74a4dd,// 406 PAY 403 

    0x2c547c38,// 407 PAY 404 

    0x907a554b,// 408 PAY 405 

    0x9a9574de,// 409 PAY 406 

    0x714c6744,// 410 PAY 407 

    0xf8447635,// 411 PAY 408 

    0x065121e7,// 412 PAY 409 

    0x74ed8e94,// 413 PAY 410 

    0x2014fb0b,// 414 PAY 411 

    0x7acd71f9,// 415 PAY 412 

    0x4ab88662,// 416 PAY 413 

    0x31b826cf,// 417 PAY 414 

    0x93237164,// 418 PAY 415 

    0xd5cc49bf,// 419 PAY 416 

    0x10a5c0ec,// 420 PAY 417 

    0x07323f93,// 421 PAY 418 

    0xeab13545,// 422 PAY 419 

    0x02d90285,// 423 PAY 420 

    0x38e8dbb3,// 424 PAY 421 

    0x44688343,// 425 PAY 422 

    0x6fbb2117,// 426 PAY 423 

    0xf983960e,// 427 PAY 424 

    0xc66fb6cf,// 428 PAY 425 

    0x41a431a2,// 429 PAY 426 

    0x5c527e83,// 430 PAY 427 

    0xffd6f94d,// 431 PAY 428 

    0x607e4a59,// 432 PAY 429 

    0x73d369b1,// 433 PAY 430 

    0xb3be0c3d,// 434 PAY 431 

    0x70dfde56,// 435 PAY 432 

    0x529b3b06,// 436 PAY 433 

    0x17606de1,// 437 PAY 434 

    0x3b7f4b90,// 438 PAY 435 

    0x0b8bdf45,// 439 PAY 436 

    0x2656bc66,// 440 PAY 437 

    0x65ec4d3f,// 441 PAY 438 

    0x1c6020e6,// 442 PAY 439 

    0x0091151d,// 443 PAY 440 

    0x0ad7afb1,// 444 PAY 441 

    0x5817367c,// 445 PAY 442 

    0x4c7cd746,// 446 PAY 443 

    0xd0f9bc1e,// 447 PAY 444 

    0x292d8e96,// 448 PAY 445 

    0x9492dc16,// 449 PAY 446 

    0xddd3263e,// 450 PAY 447 

    0x63503b3b,// 451 PAY 448 

    0xbceb53c9,// 452 PAY 449 

    0xa07e6ae9,// 453 PAY 450 

    0x70499c8b,// 454 PAY 451 

    0x987e8c3c,// 455 PAY 452 

    0x1434588e,// 456 PAY 453 

    0xfd7eee85,// 457 PAY 454 

    0x0db779e9,// 458 PAY 455 

    0x2ef1c865,// 459 PAY 456 

    0xcc8b00f7,// 460 PAY 457 

    0x1e061cd0,// 461 PAY 458 

    0x96234bc3,// 462 PAY 459 

    0x497aab9f,// 463 PAY 460 

    0x5013492f,// 464 PAY 461 

    0x83f6f445,// 465 PAY 462 

    0x9d74ffd6,// 466 PAY 463 

    0xc45f6878,// 467 PAY 464 

    0x8b644f79,// 468 PAY 465 

    0xc807ab11,// 469 PAY 466 

    0x94340e4d,// 470 PAY 467 

    0x4eed9afb,// 471 PAY 468 

    0xa0c22228,// 472 PAY 469 

    0x41112297,// 473 PAY 470 

    0x5796dd8c,// 474 PAY 471 

    0x64f39b12,// 475 PAY 472 

    0x43ddc05b,// 476 PAY 473 

    0x09b7f6c6,// 477 PAY 474 

    0xc7d6f922,// 478 PAY 475 

    0x04cbd794,// 479 PAY 476 

    0x5635697a,// 480 PAY 477 

    0xcebea8d5,// 481 PAY 478 

    0x514e3126,// 482 PAY 479 

    0x156985a8,// 483 PAY 480 

    0xd6e61eef,// 484 PAY 481 

    0x3722c455,// 485 PAY 482 

    0x0b5465dc,// 486 PAY 483 

    0xcf218a65,// 487 PAY 484 

    0x687369af,// 488 PAY 485 

    0x7a77cb6e,// 489 PAY 486 

    0xaeb821e1,// 490 PAY 487 

    0x41960077,// 491 PAY 488 

    0x81852054,// 492 PAY 489 

    0x589006be,// 493 PAY 490 

    0x85568572,// 494 PAY 491 

    0x174cea40,// 495 PAY 492 

    0x106c78a7,// 496 PAY 493 

    0x1d592062,// 497 PAY 494 

    0x19fa5dc3,// 498 PAY 495 

    0xff9a5200,// 499 PAY 496 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf1 

    0x0248f17e // 500 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt89_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 150 words. 

/// BDA size     is 594 (0x252) 

/// BDA id       is 0x8e30 

    0x02528e30,// 3 BDA   1 

/// PAY Generic Data size   : 594 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa1ab798c,// 4 PAY   1 

    0x61f767aa,// 5 PAY   2 

    0x5c9f8daa,// 6 PAY   3 

    0x28c41cb4,// 7 PAY   4 

    0x2b6b8df4,// 8 PAY   5 

    0xdfc616ed,// 9 PAY   6 

    0x272b10a9,// 10 PAY   7 

    0xd7cef4b4,// 11 PAY   8 

    0x4629ba26,// 12 PAY   9 

    0xfab90eeb,// 13 PAY  10 

    0x083f04a8,// 14 PAY  11 

    0x5f552d62,// 15 PAY  12 

    0xee66129f,// 16 PAY  13 

    0xcc02419e,// 17 PAY  14 

    0x0b47b957,// 18 PAY  15 

    0x83fa32af,// 19 PAY  16 

    0x51b14d7c,// 20 PAY  17 

    0x8188d762,// 21 PAY  18 

    0x7778fe4d,// 22 PAY  19 

    0x8e4a2346,// 23 PAY  20 

    0x2366aede,// 24 PAY  21 

    0x3a4241ad,// 25 PAY  22 

    0x8f84cf28,// 26 PAY  23 

    0x419ba7bd,// 27 PAY  24 

    0x90b3abbb,// 28 PAY  25 

    0xd31cb90b,// 29 PAY  26 

    0xd467394d,// 30 PAY  27 

    0x0e3d8f4c,// 31 PAY  28 

    0x04c2c97f,// 32 PAY  29 

    0x0f57b3f1,// 33 PAY  30 

    0xeba365d9,// 34 PAY  31 

    0xecc4c413,// 35 PAY  32 

    0x214ab605,// 36 PAY  33 

    0x3f52957e,// 37 PAY  34 

    0x0b526681,// 38 PAY  35 

    0x4b8af58d,// 39 PAY  36 

    0x99aea507,// 40 PAY  37 

    0x9d70e2b4,// 41 PAY  38 

    0xa47eece6,// 42 PAY  39 

    0xdcc29d49,// 43 PAY  40 

    0xa2f6e04c,// 44 PAY  41 

    0xecb7555d,// 45 PAY  42 

    0x24a6b27f,// 46 PAY  43 

    0xf63e555b,// 47 PAY  44 

    0x96ce1d05,// 48 PAY  45 

    0x644b364e,// 49 PAY  46 

    0x6178154c,// 50 PAY  47 

    0x8c74f748,// 51 PAY  48 

    0x6f0620c2,// 52 PAY  49 

    0xf1195cd3,// 53 PAY  50 

    0x8630a9b3,// 54 PAY  51 

    0x24c6f4d3,// 55 PAY  52 

    0x21de246a,// 56 PAY  53 

    0xf20b2c33,// 57 PAY  54 

    0xfb044540,// 58 PAY  55 

    0x90b8f949,// 59 PAY  56 

    0x66bd8f66,// 60 PAY  57 

    0xec60668c,// 61 PAY  58 

    0x813d9c4d,// 62 PAY  59 

    0xdc208dad,// 63 PAY  60 

    0xaddc71b7,// 64 PAY  61 

    0x4860682b,// 65 PAY  62 

    0x39b759a7,// 66 PAY  63 

    0x9d3f4e8a,// 67 PAY  64 

    0x4e6c6bfc,// 68 PAY  65 

    0xa783a986,// 69 PAY  66 

    0x68953716,// 70 PAY  67 

    0xa66ad17d,// 71 PAY  68 

    0xde61c72a,// 72 PAY  69 

    0xc6da347f,// 73 PAY  70 

    0xeded06ab,// 74 PAY  71 

    0xdb043257,// 75 PAY  72 

    0x6b9f2490,// 76 PAY  73 

    0xbda9e4f9,// 77 PAY  74 

    0x333192f6,// 78 PAY  75 

    0xd7a59b53,// 79 PAY  76 

    0xffceb69a,// 80 PAY  77 

    0x87aa0bd0,// 81 PAY  78 

    0xfdb4c8db,// 82 PAY  79 

    0xdbee72e5,// 83 PAY  80 

    0x743de8e7,// 84 PAY  81 

    0xdee7e5fc,// 85 PAY  82 

    0xaafc315a,// 86 PAY  83 

    0xbb91153f,// 87 PAY  84 

    0x374a800f,// 88 PAY  85 

    0x979448c0,// 89 PAY  86 

    0x60e38f65,// 90 PAY  87 

    0x0a55a70a,// 91 PAY  88 

    0xb1316089,// 92 PAY  89 

    0xac0f7394,// 93 PAY  90 

    0xa1d94a8a,// 94 PAY  91 

    0x2a29aa5b,// 95 PAY  92 

    0x5b9adce0,// 96 PAY  93 

    0xdf7b2316,// 97 PAY  94 

    0x4e29b2c1,// 98 PAY  95 

    0x97440b87,// 99 PAY  96 

    0x38efe35d,// 100 PAY  97 

    0x891ef0fe,// 101 PAY  98 

    0xc228f703,// 102 PAY  99 

    0x7dc16c6f,// 103 PAY 100 

    0x46223e77,// 104 PAY 101 

    0xcc75e613,// 105 PAY 102 

    0x77835261,// 106 PAY 103 

    0x806c5f95,// 107 PAY 104 

    0x22e43d85,// 108 PAY 105 

    0x70fc2e54,// 109 PAY 106 

    0xac05eb29,// 110 PAY 107 

    0x5848bd5d,// 111 PAY 108 

    0x1b5d5353,// 112 PAY 109 

    0x83bc6ef5,// 113 PAY 110 

    0xf9127a87,// 114 PAY 111 

    0xf484b714,// 115 PAY 112 

    0x895a8c40,// 116 PAY 113 

    0x14fcd826,// 117 PAY 114 

    0x10e0490d,// 118 PAY 115 

    0x835e92d4,// 119 PAY 116 

    0xe933d85f,// 120 PAY 117 

    0x493eb7d0,// 121 PAY 118 

    0x904639dd,// 122 PAY 119 

    0x1688936c,// 123 PAY 120 

    0x70ee9107,// 124 PAY 121 

    0x01072573,// 125 PAY 122 

    0xb7bcd260,// 126 PAY 123 

    0xe60f0a7e,// 127 PAY 124 

    0xdbaeb255,// 128 PAY 125 

    0xa4096421,// 129 PAY 126 

    0xac6029dd,// 130 PAY 127 

    0xf1ea51b2,// 131 PAY 128 

    0xc9188ffd,// 132 PAY 129 

    0x0c9f8727,// 133 PAY 130 

    0x00656b1a,// 134 PAY 131 

    0x43f0e599,// 135 PAY 132 

    0x6a607772,// 136 PAY 133 

    0x83fec90f,// 137 PAY 134 

    0xed462986,// 138 PAY 135 

    0x094ad044,// 139 PAY 136 

    0xf486f5f3,// 140 PAY 137 

    0xebfe45a7,// 141 PAY 138 

    0xdd46f8b0,// 142 PAY 139 

    0x65f92fb8,// 143 PAY 140 

    0x15294aba,// 144 PAY 141 

    0xa58bda9c,// 145 PAY 142 

    0x79e30939,// 146 PAY 143 

    0x8121e6df,// 147 PAY 144 

    0x0536d802,// 148 PAY 145 

    0xf566336d,// 149 PAY 146 

    0x66c3f1d7,// 150 PAY 147 

    0x6b3e033a,// 151 PAY 148 

    0x26300000,// 152 PAY 149 

/// HASH is  4 bytes 

    0x891ef0fe,// 153 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 49 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x0188c131 // 154 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt90_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 79 words. 

/// BDA size     is 311 (0x137) 

/// BDA id       is 0xa336 

    0x0137a336,// 3 BDA   1 

/// PAY Generic Data size   : 311 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x5c0a8de0,// 4 PAY   1 

    0x52b0f583,// 5 PAY   2 

    0x9f74ecd3,// 6 PAY   3 

    0xf9b9f110,// 7 PAY   4 

    0x7e729762,// 8 PAY   5 

    0x32abba1d,// 9 PAY   6 

    0x88c4fd28,// 10 PAY   7 

    0x443f83bb,// 11 PAY   8 

    0x36c41cfe,// 12 PAY   9 

    0xf77dcbe8,// 13 PAY  10 

    0x9a540a09,// 14 PAY  11 

    0x502cd8ff,// 15 PAY  12 

    0xdec74db7,// 16 PAY  13 

    0x9eb5fa3d,// 17 PAY  14 

    0xe919827b,// 18 PAY  15 

    0x6996c415,// 19 PAY  16 

    0x06820d66,// 20 PAY  17 

    0x7804a0a2,// 21 PAY  18 

    0x23c3a939,// 22 PAY  19 

    0x821253e6,// 23 PAY  20 

    0xedf15de9,// 24 PAY  21 

    0x0b3b432a,// 25 PAY  22 

    0xa431f51e,// 26 PAY  23 

    0xe4e7ee56,// 27 PAY  24 

    0xc5863294,// 28 PAY  25 

    0x4c50098d,// 29 PAY  26 

    0x224d9eea,// 30 PAY  27 

    0xc60175df,// 31 PAY  28 

    0x5ded2263,// 32 PAY  29 

    0x95e6cc5d,// 33 PAY  30 

    0x9de262a3,// 34 PAY  31 

    0x5d2a05f4,// 35 PAY  32 

    0x1491e04c,// 36 PAY  33 

    0xbe744de4,// 37 PAY  34 

    0xb8dd0714,// 38 PAY  35 

    0xc3ae680d,// 39 PAY  36 

    0xe08fc7b7,// 40 PAY  37 

    0x1675b4eb,// 41 PAY  38 

    0x9e501dc5,// 42 PAY  39 

    0xf1564a30,// 43 PAY  40 

    0xe5fb5cfd,// 44 PAY  41 

    0xa35cdb57,// 45 PAY  42 

    0xdd90001c,// 46 PAY  43 

    0x47324830,// 47 PAY  44 

    0x983da4ad,// 48 PAY  45 

    0xc3ec4b5b,// 49 PAY  46 

    0xb1d99d62,// 50 PAY  47 

    0x5f9099b8,// 51 PAY  48 

    0x499483d3,// 52 PAY  49 

    0x268180b4,// 53 PAY  50 

    0x5022c473,// 54 PAY  51 

    0x46e14663,// 55 PAY  52 

    0x5f89d2da,// 56 PAY  53 

    0x831f6dba,// 57 PAY  54 

    0x17e86c2b,// 58 PAY  55 

    0x0710c0fe,// 59 PAY  56 

    0x658b34e1,// 60 PAY  57 

    0xca0df221,// 61 PAY  58 

    0x9f586513,// 62 PAY  59 

    0xce305376,// 63 PAY  60 

    0x89ec072a,// 64 PAY  61 

    0xeb90b039,// 65 PAY  62 

    0xd5b3954b,// 66 PAY  63 

    0xfb8dd28e,// 67 PAY  64 

    0xc71ff7ce,// 68 PAY  65 

    0xd11ee788,// 69 PAY  66 

    0x8ad29f71,// 70 PAY  67 

    0x7a063f51,// 71 PAY  68 

    0x2ec0de42,// 72 PAY  69 

    0x18780dae,// 73 PAY  70 

    0x5a2450aa,// 74 PAY  71 

    0xdbebf2c2,// 75 PAY  72 

    0x8c499eeb,// 76 PAY  73 

    0x9168bf1f,// 77 PAY  74 

    0x2f97e8ef,// 78 PAY  75 

    0x3e7b31de,// 79 PAY  76 

    0x36a68e54,// 80 PAY  77 

    0xaa1e4300,// 81 PAY  78 

/// HASH is  8 bytes 

    0x9f586513,// 82 HSH   1 

    0xce305376,// 83 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 171 

/// STA pkt_idx        : 56 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc7 

    0x00e0c7ab // 84 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt91_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 223 words. 

/// BDA size     is 887 (0x377) 

/// BDA id       is 0xd7b7 

    0x0377d7b7,// 3 BDA   1 

/// PAY Generic Data size   : 887 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xafe8866f,// 4 PAY   1 

    0x5275a663,// 5 PAY   2 

    0x8c6d2eb5,// 6 PAY   3 

    0x966b12e1,// 7 PAY   4 

    0xe1334280,// 8 PAY   5 

    0xfc120ede,// 9 PAY   6 

    0xff21a7a5,// 10 PAY   7 

    0xd1134ee8,// 11 PAY   8 

    0x403e333d,// 12 PAY   9 

    0x850ba686,// 13 PAY  10 

    0x5fccbd01,// 14 PAY  11 

    0x5f97c777,// 15 PAY  12 

    0x1289e61c,// 16 PAY  13 

    0xaa8a978b,// 17 PAY  14 

    0x5e571825,// 18 PAY  15 

    0xaa197a42,// 19 PAY  16 

    0x40ef8517,// 20 PAY  17 

    0x5acc41ec,// 21 PAY  18 

    0x6e22bd47,// 22 PAY  19 

    0x8353555b,// 23 PAY  20 

    0x9847fcd2,// 24 PAY  21 

    0x4d9d3f06,// 25 PAY  22 

    0x3002dce6,// 26 PAY  23 

    0xd8dfdfde,// 27 PAY  24 

    0xf04166b5,// 28 PAY  25 

    0xa405a43c,// 29 PAY  26 

    0x5b7f64a1,// 30 PAY  27 

    0xd32ea620,// 31 PAY  28 

    0x5a3dfc15,// 32 PAY  29 

    0x6ff7fbba,// 33 PAY  30 

    0x5ed38d89,// 34 PAY  31 

    0x90d17981,// 35 PAY  32 

    0xe4a27b79,// 36 PAY  33 

    0x4d4676f9,// 37 PAY  34 

    0x1242cb9c,// 38 PAY  35 

    0x404772b2,// 39 PAY  36 

    0x7e8bfea9,// 40 PAY  37 

    0x1ef55f17,// 41 PAY  38 

    0x248b9bbe,// 42 PAY  39 

    0x968562c3,// 43 PAY  40 

    0x308262ef,// 44 PAY  41 

    0x5a91d84f,// 45 PAY  42 

    0x48be18e4,// 46 PAY  43 

    0x7de36f34,// 47 PAY  44 

    0x4ee65586,// 48 PAY  45 

    0x6bfb2fde,// 49 PAY  46 

    0x7562abcc,// 50 PAY  47 

    0x650da6ce,// 51 PAY  48 

    0xd8571998,// 52 PAY  49 

    0x9cb4e3c1,// 53 PAY  50 

    0x714433d5,// 54 PAY  51 

    0xe4258790,// 55 PAY  52 

    0xaa9b668e,// 56 PAY  53 

    0xc1ef2407,// 57 PAY  54 

    0xc4424cd6,// 58 PAY  55 

    0xbf1db1fe,// 59 PAY  56 

    0xb3820689,// 60 PAY  57 

    0xa26dcee2,// 61 PAY  58 

    0x81260c22,// 62 PAY  59 

    0x07759600,// 63 PAY  60 

    0x25a1777c,// 64 PAY  61 

    0xc2154392,// 65 PAY  62 

    0xd529458c,// 66 PAY  63 

    0xf783df57,// 67 PAY  64 

    0x6686a083,// 68 PAY  65 

    0xb6dfc697,// 69 PAY  66 

    0xb8903d06,// 70 PAY  67 

    0xe6689ae8,// 71 PAY  68 

    0xc3f59ba2,// 72 PAY  69 

    0x70d1df89,// 73 PAY  70 

    0x9e3b3ec4,// 74 PAY  71 

    0x4feb9f7a,// 75 PAY  72 

    0xc9119eb4,// 76 PAY  73 

    0xadbe3c9a,// 77 PAY  74 

    0xb6a330ca,// 78 PAY  75 

    0x22acd680,// 79 PAY  76 

    0x0e928234,// 80 PAY  77 

    0x21576165,// 81 PAY  78 

    0xbfb7ba98,// 82 PAY  79 

    0x53e2d8f5,// 83 PAY  80 

    0x21571e4c,// 84 PAY  81 

    0x4739e00c,// 85 PAY  82 

    0x39774f25,// 86 PAY  83 

    0x8bca81a2,// 87 PAY  84 

    0xc903f732,// 88 PAY  85 

    0x555b8cdf,// 89 PAY  86 

    0xba8c09a7,// 90 PAY  87 

    0x860a5f91,// 91 PAY  88 

    0xda7ed251,// 92 PAY  89 

    0xab73caa0,// 93 PAY  90 

    0xafb4030c,// 94 PAY  91 

    0x5a8c6728,// 95 PAY  92 

    0x2bafcfce,// 96 PAY  93 

    0xe906fa06,// 97 PAY  94 

    0xc812d544,// 98 PAY  95 

    0xdf4f1b11,// 99 PAY  96 

    0xe7c80490,// 100 PAY  97 

    0x43e79065,// 101 PAY  98 

    0x9b1fc86d,// 102 PAY  99 

    0x77ecb3d9,// 103 PAY 100 

    0x02a65fe9,// 104 PAY 101 

    0x306df8be,// 105 PAY 102 

    0x4645fe91,// 106 PAY 103 

    0xbde9cd61,// 107 PAY 104 

    0x119d745a,// 108 PAY 105 

    0x7732bf05,// 109 PAY 106 

    0x530eb7c3,// 110 PAY 107 

    0xd913134e,// 111 PAY 108 

    0x985cdfa0,// 112 PAY 109 

    0x52039ef2,// 113 PAY 110 

    0xb571a503,// 114 PAY 111 

    0x76a417a8,// 115 PAY 112 

    0x49e3e586,// 116 PAY 113 

    0xb097b3e5,// 117 PAY 114 

    0x20a15413,// 118 PAY 115 

    0x71a92f0e,// 119 PAY 116 

    0x5dbfeb11,// 120 PAY 117 

    0x8354db50,// 121 PAY 118 

    0x8cf97a3e,// 122 PAY 119 

    0x6e41b866,// 123 PAY 120 

    0xd4e170b0,// 124 PAY 121 

    0x178ea8cd,// 125 PAY 122 

    0x41b3a3a4,// 126 PAY 123 

    0x8ec31b5b,// 127 PAY 124 

    0xb460a486,// 128 PAY 125 

    0xdbe3abeb,// 129 PAY 126 

    0x8e014cdf,// 130 PAY 127 

    0x92147cb4,// 131 PAY 128 

    0x31dd6ac5,// 132 PAY 129 

    0xd41620da,// 133 PAY 130 

    0x8a320268,// 134 PAY 131 

    0x6682606a,// 135 PAY 132 

    0x2f9a86a0,// 136 PAY 133 

    0x2e0aee87,// 137 PAY 134 

    0x06125246,// 138 PAY 135 

    0x807b24a4,// 139 PAY 136 

    0x66fef024,// 140 PAY 137 

    0xf4774c58,// 141 PAY 138 

    0x83b5dcb5,// 142 PAY 139 

    0x0d9bd9d9,// 143 PAY 140 

    0xf7109d37,// 144 PAY 141 

    0x43f4c4f1,// 145 PAY 142 

    0x46e042fd,// 146 PAY 143 

    0x4d481813,// 147 PAY 144 

    0x23492d1c,// 148 PAY 145 

    0x7c387d2b,// 149 PAY 146 

    0x0275c669,// 150 PAY 147 

    0x5f787574,// 151 PAY 148 

    0x659e5b13,// 152 PAY 149 

    0x4fb5a507,// 153 PAY 150 

    0x5a6da792,// 154 PAY 151 

    0xbb8aeeea,// 155 PAY 152 

    0x73ef8c89,// 156 PAY 153 

    0x12d399f5,// 157 PAY 154 

    0x4b9788b6,// 158 PAY 155 

    0x92217964,// 159 PAY 156 

    0x1c30b3cd,// 160 PAY 157 

    0xd429ed8b,// 161 PAY 158 

    0x29e64db0,// 162 PAY 159 

    0xad21b871,// 163 PAY 160 

    0x5711f47e,// 164 PAY 161 

    0x82da0b5f,// 165 PAY 162 

    0xab40f67a,// 166 PAY 163 

    0x1dc9a2f2,// 167 PAY 164 

    0x410dd110,// 168 PAY 165 

    0x7106aed9,// 169 PAY 166 

    0xb4726d2d,// 170 PAY 167 

    0x03c47673,// 171 PAY 168 

    0xc9cc59a4,// 172 PAY 169 

    0xa773192f,// 173 PAY 170 

    0x54000da1,// 174 PAY 171 

    0x21c33560,// 175 PAY 172 

    0xbeaae612,// 176 PAY 173 

    0x241f2b64,// 177 PAY 174 

    0xbcd5b530,// 178 PAY 175 

    0x40d024af,// 179 PAY 176 

    0x82ef952c,// 180 PAY 177 

    0x876816a5,// 181 PAY 178 

    0x1b5035b8,// 182 PAY 179 

    0xb633f100,// 183 PAY 180 

    0x825750db,// 184 PAY 181 

    0x5c67c9e0,// 185 PAY 182 

    0xc6b58bae,// 186 PAY 183 

    0xe61e10d6,// 187 PAY 184 

    0xe4252504,// 188 PAY 185 

    0x08c9b451,// 189 PAY 186 

    0x9f9b0444,// 190 PAY 187 

    0xcbd0ca6e,// 191 PAY 188 

    0x8bae36a8,// 192 PAY 189 

    0x19d6103b,// 193 PAY 190 

    0x825d52e6,// 194 PAY 191 

    0x77777132,// 195 PAY 192 

    0x0373717a,// 196 PAY 193 

    0xf12ea05f,// 197 PAY 194 

    0xd676c5a6,// 198 PAY 195 

    0x933bff6f,// 199 PAY 196 

    0x372a5f96,// 200 PAY 197 

    0x4e2507a8,// 201 PAY 198 

    0x26b7d0e5,// 202 PAY 199 

    0x007bf4b0,// 203 PAY 200 

    0x88060984,// 204 PAY 201 

    0x1fe234c2,// 205 PAY 202 

    0xb54c8218,// 206 PAY 203 

    0x4817a3f3,// 207 PAY 204 

    0x4d7cf9d7,// 208 PAY 205 

    0xd19b505a,// 209 PAY 206 

    0x4e083226,// 210 PAY 207 

    0xc7ffbaee,// 211 PAY 208 

    0x427d9d42,// 212 PAY 209 

    0xa785aa3a,// 213 PAY 210 

    0xcbf919bd,// 214 PAY 211 

    0x6c363f17,// 215 PAY 212 

    0x839964b0,// 216 PAY 213 

    0x28043696,// 217 PAY 214 

    0x46fa283c,// 218 PAY 215 

    0x81ea01ac,// 219 PAY 216 

    0x82c9cf23,// 220 PAY 217 

    0xba0180a0,// 221 PAY 218 

    0x19cc0776,// 222 PAY 219 

    0x5efbbb59,// 223 PAY 220 

    0x2bad6b74,// 224 PAY 221 

    0x1b763500,// 225 PAY 222 

/// STA is 1 words. 

/// STA num_pkts       : 249 

/// STA pkt_idx        : 166 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x029904f9 // 226 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt92_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 304 words. 

/// BDA size     is 1210 (0x4ba) 

/// BDA id       is 0xb198 

    0x04bab198,// 3 BDA   1 

/// PAY Generic Data size   : 1210 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x9b0fbe37,// 4 PAY   1 

    0x42b9201b,// 5 PAY   2 

    0x2b1d911d,// 6 PAY   3 

    0xdc529861,// 7 PAY   4 

    0xaa06e1b8,// 8 PAY   5 

    0xf24c060a,// 9 PAY   6 

    0x6719698a,// 10 PAY   7 

    0xff407107,// 11 PAY   8 

    0x9dd8909d,// 12 PAY   9 

    0x446207bf,// 13 PAY  10 

    0x8d456eb2,// 14 PAY  11 

    0xd96a0d2e,// 15 PAY  12 

    0xf70639f2,// 16 PAY  13 

    0x47c62039,// 17 PAY  14 

    0x1992439d,// 18 PAY  15 

    0xb2593ef2,// 19 PAY  16 

    0x072afc7a,// 20 PAY  17 

    0x8eb6f6f6,// 21 PAY  18 

    0xbde34487,// 22 PAY  19 

    0xe3d778ad,// 23 PAY  20 

    0xaadc4182,// 24 PAY  21 

    0x64208ec0,// 25 PAY  22 

    0xe6288f63,// 26 PAY  23 

    0x752b92bd,// 27 PAY  24 

    0x27a83344,// 28 PAY  25 

    0xc50cea09,// 29 PAY  26 

    0xa8b1f0a5,// 30 PAY  27 

    0x7bc893d6,// 31 PAY  28 

    0x5ccd4f54,// 32 PAY  29 

    0xddfc1939,// 33 PAY  30 

    0x270df9f1,// 34 PAY  31 

    0x31cc05ec,// 35 PAY  32 

    0xee887989,// 36 PAY  33 

    0xdc36ab62,// 37 PAY  34 

    0x8662ec03,// 38 PAY  35 

    0x1dfac0ae,// 39 PAY  36 

    0x53103fd1,// 40 PAY  37 

    0x1c2d57d6,// 41 PAY  38 

    0xb5bc72e9,// 42 PAY  39 

    0xbe4e0b9f,// 43 PAY  40 

    0xb90f1550,// 44 PAY  41 

    0x542298bb,// 45 PAY  42 

    0x1c5a004f,// 46 PAY  43 

    0x18c14d7a,// 47 PAY  44 

    0xc76383a6,// 48 PAY  45 

    0x6c5d846d,// 49 PAY  46 

    0xd6cd5e2b,// 50 PAY  47 

    0xf52a36a9,// 51 PAY  48 

    0x605e185f,// 52 PAY  49 

    0x79db21eb,// 53 PAY  50 

    0x3ddab6ef,// 54 PAY  51 

    0xc0843ce0,// 55 PAY  52 

    0x87e53105,// 56 PAY  53 

    0x21624c40,// 57 PAY  54 

    0xf710041a,// 58 PAY  55 

    0x9177028c,// 59 PAY  56 

    0xe42a727b,// 60 PAY  57 

    0x83c0de50,// 61 PAY  58 

    0x91c6e614,// 62 PAY  59 

    0x2d92cbd6,// 63 PAY  60 

    0x1340e93c,// 64 PAY  61 

    0xebe66d7c,// 65 PAY  62 

    0x3c696f50,// 66 PAY  63 

    0x15a777ac,// 67 PAY  64 

    0xfd305f54,// 68 PAY  65 

    0x2372dba0,// 69 PAY  66 

    0xa8ca1363,// 70 PAY  67 

    0x2445dfe2,// 71 PAY  68 

    0x212179da,// 72 PAY  69 

    0x4d9b9323,// 73 PAY  70 

    0x4cd80ab3,// 74 PAY  71 

    0x1a4e57ef,// 75 PAY  72 

    0x82d759b7,// 76 PAY  73 

    0x506ca5f1,// 77 PAY  74 

    0x488cd545,// 78 PAY  75 

    0xabd8a715,// 79 PAY  76 

    0x71934622,// 80 PAY  77 

    0x2bf00e25,// 81 PAY  78 

    0x5ea1e37f,// 82 PAY  79 

    0x6522b4d0,// 83 PAY  80 

    0xba28c7ae,// 84 PAY  81 

    0x81167d26,// 85 PAY  82 

    0xc9bd9e1a,// 86 PAY  83 

    0x2ff1d0d3,// 87 PAY  84 

    0x4d662b6f,// 88 PAY  85 

    0xa2bcd1cd,// 89 PAY  86 

    0xd8daf1c6,// 90 PAY  87 

    0xf708f510,// 91 PAY  88 

    0xdea01ffa,// 92 PAY  89 

    0x8dd99a37,// 93 PAY  90 

    0xd6ae9ba3,// 94 PAY  91 

    0x726dfae7,// 95 PAY  92 

    0x76d9e8a5,// 96 PAY  93 

    0x5a182c82,// 97 PAY  94 

    0x51f8bcea,// 98 PAY  95 

    0x073ae0cf,// 99 PAY  96 

    0x4677865e,// 100 PAY  97 

    0x22447e24,// 101 PAY  98 

    0x536fe4a4,// 102 PAY  99 

    0x29381ce7,// 103 PAY 100 

    0x420b7168,// 104 PAY 101 

    0xcab54187,// 105 PAY 102 

    0x7b0940af,// 106 PAY 103 

    0x3e2939e3,// 107 PAY 104 

    0x171d9fef,// 108 PAY 105 

    0x0f495444,// 109 PAY 106 

    0xaafa61d7,// 110 PAY 107 

    0x383611e4,// 111 PAY 108 

    0xeae968f6,// 112 PAY 109 

    0xe65d1bce,// 113 PAY 110 

    0x4365b807,// 114 PAY 111 

    0x5259ba0d,// 115 PAY 112 

    0x9e1b46a2,// 116 PAY 113 

    0x70240690,// 117 PAY 114 

    0xa47cda81,// 118 PAY 115 

    0xb54f8198,// 119 PAY 116 

    0xd8c1d7eb,// 120 PAY 117 

    0x86962fb7,// 121 PAY 118 

    0xc868ba42,// 122 PAY 119 

    0x4015af9e,// 123 PAY 120 

    0xf57be33a,// 124 PAY 121 

    0x30efcf03,// 125 PAY 122 

    0x92979100,// 126 PAY 123 

    0xd5cf9aec,// 127 PAY 124 

    0xbb5f267d,// 128 PAY 125 

    0x849c0d66,// 129 PAY 126 

    0x244ac6e9,// 130 PAY 127 

    0x73260699,// 131 PAY 128 

    0x83b7222a,// 132 PAY 129 

    0x7b5f811a,// 133 PAY 130 

    0xab36c148,// 134 PAY 131 

    0xef43746c,// 135 PAY 132 

    0x410bc95d,// 136 PAY 133 

    0x785159b6,// 137 PAY 134 

    0xfcaeec28,// 138 PAY 135 

    0x74d6d293,// 139 PAY 136 

    0x88afae9c,// 140 PAY 137 

    0x06f89b14,// 141 PAY 138 

    0x36041e5f,// 142 PAY 139 

    0x3235466d,// 143 PAY 140 

    0xb26ffe7a,// 144 PAY 141 

    0x7d114257,// 145 PAY 142 

    0x3ad50155,// 146 PAY 143 

    0xc48c3e90,// 147 PAY 144 

    0xb201f0d7,// 148 PAY 145 

    0x1ced08df,// 149 PAY 146 

    0x57097bdf,// 150 PAY 147 

    0x03c75cc4,// 151 PAY 148 

    0x9582274e,// 152 PAY 149 

    0x7b0b24d1,// 153 PAY 150 

    0x591df92b,// 154 PAY 151 

    0x50cc85d0,// 155 PAY 152 

    0x4a8dee6f,// 156 PAY 153 

    0x551bfc31,// 157 PAY 154 

    0x2c22a25f,// 158 PAY 155 

    0x6487f7a9,// 159 PAY 156 

    0x5c2b27d6,// 160 PAY 157 

    0xa090d330,// 161 PAY 158 

    0x0336e948,// 162 PAY 159 

    0xccc1d4cd,// 163 PAY 160 

    0xc9cc4a3d,// 164 PAY 161 

    0x07a8bf38,// 165 PAY 162 

    0xce78f9fd,// 166 PAY 163 

    0xbde2c508,// 167 PAY 164 

    0xcc39b915,// 168 PAY 165 

    0xa784fe52,// 169 PAY 166 

    0xe241d1fe,// 170 PAY 167 

    0x25cf2110,// 171 PAY 168 

    0x6bf8ff95,// 172 PAY 169 

    0x45fe67c1,// 173 PAY 170 

    0x0e301046,// 174 PAY 171 

    0x21593869,// 175 PAY 172 

    0x70656f22,// 176 PAY 173 

    0x762de594,// 177 PAY 174 

    0xf4f58672,// 178 PAY 175 

    0xae24b9f4,// 179 PAY 176 

    0x59872a7d,// 180 PAY 177 

    0x315f923e,// 181 PAY 178 

    0xcff6588b,// 182 PAY 179 

    0x1ed2772c,// 183 PAY 180 

    0x9f28bf67,// 184 PAY 181 

    0x224d702b,// 185 PAY 182 

    0xa53ea84f,// 186 PAY 183 

    0xa7b2fe23,// 187 PAY 184 

    0x7065681f,// 188 PAY 185 

    0x115f8fd8,// 189 PAY 186 

    0x21314129,// 190 PAY 187 

    0x6970e7bb,// 191 PAY 188 

    0x26b2735c,// 192 PAY 189 

    0x7dccd3d7,// 193 PAY 190 

    0x07450564,// 194 PAY 191 

    0xbb8260fb,// 195 PAY 192 

    0x7b61a56d,// 196 PAY 193 

    0x18e71c4b,// 197 PAY 194 

    0xed136e65,// 198 PAY 195 

    0xfe6e899c,// 199 PAY 196 

    0x38b614ae,// 200 PAY 197 

    0xa91b831b,// 201 PAY 198 

    0x4e8354ca,// 202 PAY 199 

    0x562a9fb5,// 203 PAY 200 

    0xacaba58c,// 204 PAY 201 

    0xdfd1950c,// 205 PAY 202 

    0xb49b9f75,// 206 PAY 203 

    0x506a672f,// 207 PAY 204 

    0xa0c03e6f,// 208 PAY 205 

    0xdb5d15ec,// 209 PAY 206 

    0x14b5e3ad,// 210 PAY 207 

    0xfc8646f8,// 211 PAY 208 

    0xd72be61f,// 212 PAY 209 

    0xb604a6bd,// 213 PAY 210 

    0x09937785,// 214 PAY 211 

    0x70476f9e,// 215 PAY 212 

    0x41a4be1f,// 216 PAY 213 

    0x2456b66b,// 217 PAY 214 

    0xef465d39,// 218 PAY 215 

    0x02e3af14,// 219 PAY 216 

    0x6ac188a2,// 220 PAY 217 

    0x328085cc,// 221 PAY 218 

    0x717c828e,// 222 PAY 219 

    0x8ff33acc,// 223 PAY 220 

    0x2be9594a,// 224 PAY 221 

    0xa78862aa,// 225 PAY 222 

    0x65d0c74d,// 226 PAY 223 

    0xd3835914,// 227 PAY 224 

    0xd019f458,// 228 PAY 225 

    0xda49dbb2,// 229 PAY 226 

    0x493d5533,// 230 PAY 227 

    0xacfd6143,// 231 PAY 228 

    0x14a7aae5,// 232 PAY 229 

    0x078c3a6b,// 233 PAY 230 

    0x206c87c4,// 234 PAY 231 

    0x38eab481,// 235 PAY 232 

    0x4f7d5b7b,// 236 PAY 233 

    0xedd1277d,// 237 PAY 234 

    0x44491612,// 238 PAY 235 

    0xe88596e8,// 239 PAY 236 

    0xc44fefa2,// 240 PAY 237 

    0x9fe76015,// 241 PAY 238 

    0xa7a8388b,// 242 PAY 239 

    0x13da43c3,// 243 PAY 240 

    0x2eed36a5,// 244 PAY 241 

    0x73687f15,// 245 PAY 242 

    0x282701f7,// 246 PAY 243 

    0x53827262,// 247 PAY 244 

    0x0c838c4d,// 248 PAY 245 

    0xabcb7468,// 249 PAY 246 

    0x487a7ed6,// 250 PAY 247 

    0x74f18b9b,// 251 PAY 248 

    0x5417543c,// 252 PAY 249 

    0xe792bf64,// 253 PAY 250 

    0xb2b28b93,// 254 PAY 251 

    0xfbdd5e41,// 255 PAY 252 

    0x81a0f16d,// 256 PAY 253 

    0x79fea7c9,// 257 PAY 254 

    0xc2104daa,// 258 PAY 255 

    0xf00fbe37,// 259 PAY 256 

    0xd12b04db,// 260 PAY 257 

    0x9356b117,// 261 PAY 258 

    0xe4488570,// 262 PAY 259 

    0x33ba95bb,// 263 PAY 260 

    0x85e48a75,// 264 PAY 261 

    0x4b85ecc5,// 265 PAY 262 

    0x91729559,// 266 PAY 263 

    0x492a6003,// 267 PAY 264 

    0xc8f604b6,// 268 PAY 265 

    0x534c7f8b,// 269 PAY 266 

    0x7b1b4553,// 270 PAY 267 

    0xe009e959,// 271 PAY 268 

    0x66784d5c,// 272 PAY 269 

    0x8a3ffa16,// 273 PAY 270 

    0x0d4256be,// 274 PAY 271 

    0x04bb2c72,// 275 PAY 272 

    0x9ab0a9e1,// 276 PAY 273 

    0x921b9b95,// 277 PAY 274 

    0x55b97c3a,// 278 PAY 275 

    0x236f5cba,// 279 PAY 276 

    0x65dbe181,// 280 PAY 277 

    0x69d24e2f,// 281 PAY 278 

    0xa57f2792,// 282 PAY 279 

    0x1083a1f2,// 283 PAY 280 

    0x4cc514af,// 284 PAY 281 

    0xe767de3f,// 285 PAY 282 

    0xe1246844,// 286 PAY 283 

    0xd46a4e95,// 287 PAY 284 

    0x39ad57a8,// 288 PAY 285 

    0xdf6fe2aa,// 289 PAY 286 

    0x443d5ef4,// 290 PAY 287 

    0x2b81bfd0,// 291 PAY 288 

    0xe2e92176,// 292 PAY 289 

    0xaac30ddf,// 293 PAY 290 

    0xa43ed086,// 294 PAY 291 

    0x8dd017cd,// 295 PAY 292 

    0xab075fbb,// 296 PAY 293 

    0xf39e2167,// 297 PAY 294 

    0x0b99a7aa,// 298 PAY 295 

    0xad3fe724,// 299 PAY 296 

    0x2586a44b,// 300 PAY 297 

    0x1aec65ec,// 301 PAY 298 

    0x04bdd2e6,// 302 PAY 299 

    0xac8d3a89,// 303 PAY 300 

    0x3708a4b0,// 304 PAY 301 

    0x27597377,// 305 PAY 302 

    0xe65d0000,// 306 PAY 303 

/// HASH is  12 bytes 

    0x92979100,// 307 HSH   1 

    0xd5cf9aec,// 308 HSH   2 

    0xbb5f267d,// 309 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 86 

/// STA pkt_idx        : 224 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf3 

    0x0381f356 // 310 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt93_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 469 words. 

/// BDA size     is 1871 (0x74f) 

/// BDA id       is 0x3f09 

    0x074f3f09,// 3 BDA   1 

/// PAY Generic Data size   : 1871 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xaca9df8a,// 4 PAY   1 

    0x78e57576,// 5 PAY   2 

    0xcdc89b95,// 6 PAY   3 

    0x77ce468e,// 7 PAY   4 

    0xa2209ea7,// 8 PAY   5 

    0x0769dfba,// 9 PAY   6 

    0x29bcfbfb,// 10 PAY   7 

    0x2d6b9b18,// 11 PAY   8 

    0x1a568153,// 12 PAY   9 

    0xb1824732,// 13 PAY  10 

    0x2e48033c,// 14 PAY  11 

    0x14207de9,// 15 PAY  12 

    0xb50d3cde,// 16 PAY  13 

    0xc7c41c18,// 17 PAY  14 

    0x3dcdd00b,// 18 PAY  15 

    0x1494b88f,// 19 PAY  16 

    0x63556c9e,// 20 PAY  17 

    0xa9011af5,// 21 PAY  18 

    0x8475e9ea,// 22 PAY  19 

    0x0d9685e4,// 23 PAY  20 

    0xd52daf86,// 24 PAY  21 

    0x435627d4,// 25 PAY  22 

    0x8e74aa79,// 26 PAY  23 

    0x695e45c8,// 27 PAY  24 

    0xef14b318,// 28 PAY  25 

    0x2869ec84,// 29 PAY  26 

    0xa4c125d7,// 30 PAY  27 

    0x3c6eff08,// 31 PAY  28 

    0x05708f9c,// 32 PAY  29 

    0xbb01fdd0,// 33 PAY  30 

    0x4668c29e,// 34 PAY  31 

    0x4ccd525a,// 35 PAY  32 

    0xa1fc36f0,// 36 PAY  33 

    0x0d14e42f,// 37 PAY  34 

    0x9d1a2135,// 38 PAY  35 

    0x84c80c5a,// 39 PAY  36 

    0xf466fbb0,// 40 PAY  37 

    0x255735d8,// 41 PAY  38 

    0x6126c527,// 42 PAY  39 

    0x5c067c13,// 43 PAY  40 

    0xcb26af93,// 44 PAY  41 

    0x600a1ebd,// 45 PAY  42 

    0xacd6cfe4,// 46 PAY  43 

    0xeabf878e,// 47 PAY  44 

    0x75eeb2dc,// 48 PAY  45 

    0x00676182,// 49 PAY  46 

    0xd6d95643,// 50 PAY  47 

    0x10688dc5,// 51 PAY  48 

    0x35ad7dea,// 52 PAY  49 

    0x79777ad7,// 53 PAY  50 

    0x07fdbc16,// 54 PAY  51 

    0xdd2f204a,// 55 PAY  52 

    0xb82cec95,// 56 PAY  53 

    0xb24b2c32,// 57 PAY  54 

    0x77561913,// 58 PAY  55 

    0x41c4ea23,// 59 PAY  56 

    0x35094d42,// 60 PAY  57 

    0x719a2995,// 61 PAY  58 

    0x5398357a,// 62 PAY  59 

    0x2aba3459,// 63 PAY  60 

    0x78055bd1,// 64 PAY  61 

    0x279f72b4,// 65 PAY  62 

    0x84e61651,// 66 PAY  63 

    0x476bb934,// 67 PAY  64 

    0x3fa6c64d,// 68 PAY  65 

    0xb4ba5440,// 69 PAY  66 

    0xca9261f3,// 70 PAY  67 

    0x216826f3,// 71 PAY  68 

    0xed76a7d1,// 72 PAY  69 

    0x88e50076,// 73 PAY  70 

    0x0b727091,// 74 PAY  71 

    0x5cdbf375,// 75 PAY  72 

    0xe8f53b00,// 76 PAY  73 

    0x585024b4,// 77 PAY  74 

    0x892e47fd,// 78 PAY  75 

    0xbc506ffd,// 79 PAY  76 

    0x8376a318,// 80 PAY  77 

    0xee517d18,// 81 PAY  78 

    0xa6bfbb92,// 82 PAY  79 

    0x374e7f93,// 83 PAY  80 

    0xcb0672d3,// 84 PAY  81 

    0x32c8b946,// 85 PAY  82 

    0xdffff6aa,// 86 PAY  83 

    0xf37b54db,// 87 PAY  84 

    0x2098fe96,// 88 PAY  85 

    0x2646b789,// 89 PAY  86 

    0xb0803f2d,// 90 PAY  87 

    0x1fd052ff,// 91 PAY  88 

    0xe846a5a0,// 92 PAY  89 

    0xacff64f6,// 93 PAY  90 

    0x170c5293,// 94 PAY  91 

    0x73f5c7c6,// 95 PAY  92 

    0xb76d3263,// 96 PAY  93 

    0xe0d2c2a4,// 97 PAY  94 

    0x03d8377f,// 98 PAY  95 

    0x21af62e0,// 99 PAY  96 

    0x25dea708,// 100 PAY  97 

    0xffe4a665,// 101 PAY  98 

    0xa2b66b9d,// 102 PAY  99 

    0xf21a9054,// 103 PAY 100 

    0xe53dccdd,// 104 PAY 101 

    0x1e252a89,// 105 PAY 102 

    0x48206cac,// 106 PAY 103 

    0xb96ae96d,// 107 PAY 104 

    0x0d08ee91,// 108 PAY 105 

    0x9989a408,// 109 PAY 106 

    0xffb56338,// 110 PAY 107 

    0x7d5a32a2,// 111 PAY 108 

    0x9870e29e,// 112 PAY 109 

    0x85211bbe,// 113 PAY 110 

    0xa1cff962,// 114 PAY 111 

    0x6079586c,// 115 PAY 112 

    0x1deaf360,// 116 PAY 113 

    0x9e2d7589,// 117 PAY 114 

    0x2204901e,// 118 PAY 115 

    0x67e8b047,// 119 PAY 116 

    0x990be520,// 120 PAY 117 

    0x19b11d5d,// 121 PAY 118 

    0xe728bcf5,// 122 PAY 119 

    0x3da19dac,// 123 PAY 120 

    0x8a5c7c68,// 124 PAY 121 

    0x3bcc0a9b,// 125 PAY 122 

    0x66534dd5,// 126 PAY 123 

    0x928fb099,// 127 PAY 124 

    0x8578bc13,// 128 PAY 125 

    0x51874bd3,// 129 PAY 126 

    0xd2b4f71d,// 130 PAY 127 

    0xdff52a0c,// 131 PAY 128 

    0xb8f65cbf,// 132 PAY 129 

    0x432ca26e,// 133 PAY 130 

    0xa05cdc7a,// 134 PAY 131 

    0x136d13e1,// 135 PAY 132 

    0xd205ec85,// 136 PAY 133 

    0x46ad41a0,// 137 PAY 134 

    0x8a32ffbc,// 138 PAY 135 

    0x3c732e28,// 139 PAY 136 

    0xa8b0ee3b,// 140 PAY 137 

    0xd317eab0,// 141 PAY 138 

    0xeb77c4ae,// 142 PAY 139 

    0x1a43e424,// 143 PAY 140 

    0xff4808c2,// 144 PAY 141 

    0x7f26ae15,// 145 PAY 142 

    0x301bff00,// 146 PAY 143 

    0xe95419c9,// 147 PAY 144 

    0x3fc36f77,// 148 PAY 145 

    0xf31ed50e,// 149 PAY 146 

    0x28c178a9,// 150 PAY 147 

    0x162b798c,// 151 PAY 148 

    0xb02a59fe,// 152 PAY 149 

    0x3207e85c,// 153 PAY 150 

    0xa4bd6b96,// 154 PAY 151 

    0x1c858671,// 155 PAY 152 

    0xb79b733b,// 156 PAY 153 

    0xbf13bb42,// 157 PAY 154 

    0x208781f7,// 158 PAY 155 

    0xf5e3e78f,// 159 PAY 156 

    0xc341a45c,// 160 PAY 157 

    0xac37fcc0,// 161 PAY 158 

    0xbbbefae9,// 162 PAY 159 

    0x19c76d11,// 163 PAY 160 

    0x207feb59,// 164 PAY 161 

    0x10a057fa,// 165 PAY 162 

    0x6ad9db49,// 166 PAY 163 

    0x67466fc5,// 167 PAY 164 

    0xb600f871,// 168 PAY 165 

    0x88df1c78,// 169 PAY 166 

    0x74078cfe,// 170 PAY 167 

    0x38bc3723,// 171 PAY 168 

    0xdc3202a1,// 172 PAY 169 

    0x34a467e6,// 173 PAY 170 

    0x0e16662c,// 174 PAY 171 

    0xe0880f0d,// 175 PAY 172 

    0x27f45740,// 176 PAY 173 

    0x962efabc,// 177 PAY 174 

    0x8ec9631f,// 178 PAY 175 

    0xcea22354,// 179 PAY 176 

    0xeee6a5ab,// 180 PAY 177 

    0x66e8035e,// 181 PAY 178 

    0x04c2dad5,// 182 PAY 179 

    0x13dcc9c7,// 183 PAY 180 

    0xd7390be9,// 184 PAY 181 

    0xe8d5f7fb,// 185 PAY 182 

    0x869f1de1,// 186 PAY 183 

    0xba0e3366,// 187 PAY 184 

    0x6764d7d7,// 188 PAY 185 

    0x77311c27,// 189 PAY 186 

    0x5e472514,// 190 PAY 187 

    0x0db61c82,// 191 PAY 188 

    0x5bb07f67,// 192 PAY 189 

    0x655c5d12,// 193 PAY 190 

    0xd1df7122,// 194 PAY 191 

    0x9cd539eb,// 195 PAY 192 

    0x785346de,// 196 PAY 193 

    0x824f31b1,// 197 PAY 194 

    0xae9ad390,// 198 PAY 195 

    0x866d29d5,// 199 PAY 196 

    0x6d3c42d6,// 200 PAY 197 

    0xb8ff817f,// 201 PAY 198 

    0x03bcd02a,// 202 PAY 199 

    0xdea48367,// 203 PAY 200 

    0x0683be49,// 204 PAY 201 

    0x5768d7c4,// 205 PAY 202 

    0x89897720,// 206 PAY 203 

    0xc771a6c2,// 207 PAY 204 

    0x02cdf8f1,// 208 PAY 205 

    0x1499c7ad,// 209 PAY 206 

    0x0bce5b5e,// 210 PAY 207 

    0x0947f739,// 211 PAY 208 

    0xfb53e87b,// 212 PAY 209 

    0xae9a443b,// 213 PAY 210 

    0x9269054b,// 214 PAY 211 

    0x6a60bcf2,// 215 PAY 212 

    0xf4a3ae67,// 216 PAY 213 

    0x0e861ffa,// 217 PAY 214 

    0x18051714,// 218 PAY 215 

    0x41a7dab8,// 219 PAY 216 

    0x73db080f,// 220 PAY 217 

    0x167d8c1a,// 221 PAY 218 

    0x497a9988,// 222 PAY 219 

    0x7decfbdd,// 223 PAY 220 

    0x1c2ec965,// 224 PAY 221 

    0x037d37d8,// 225 PAY 222 

    0x9a02db95,// 226 PAY 223 

    0x2709bec8,// 227 PAY 224 

    0x62f27dde,// 228 PAY 225 

    0x39cee4a2,// 229 PAY 226 

    0xea42be00,// 230 PAY 227 

    0xbfe31cc7,// 231 PAY 228 

    0x7a9ca4f8,// 232 PAY 229 

    0xe7aaaae9,// 233 PAY 230 

    0x624d61b5,// 234 PAY 231 

    0x344075e6,// 235 PAY 232 

    0x20acdd29,// 236 PAY 233 

    0x4e19736f,// 237 PAY 234 

    0xbc18b00c,// 238 PAY 235 

    0x575c6950,// 239 PAY 236 

    0xaf2bbd68,// 240 PAY 237 

    0x6f766213,// 241 PAY 238 

    0xabebac90,// 242 PAY 239 

    0xffee4448,// 243 PAY 240 

    0xb35e1d9b,// 244 PAY 241 

    0xdba551f3,// 245 PAY 242 

    0x2789f297,// 246 PAY 243 

    0x70714f67,// 247 PAY 244 

    0x6c62bff5,// 248 PAY 245 

    0xceed0ba8,// 249 PAY 246 

    0x7c0068db,// 250 PAY 247 

    0xa6647323,// 251 PAY 248 

    0xde85498c,// 252 PAY 249 

    0x5146f530,// 253 PAY 250 

    0x5244384d,// 254 PAY 251 

    0xe018ce1e,// 255 PAY 252 

    0x3700e3a8,// 256 PAY 253 

    0x94397c00,// 257 PAY 254 

    0xab10698c,// 258 PAY 255 

    0xe6ccfe6a,// 259 PAY 256 

    0x0a52bbcf,// 260 PAY 257 

    0x22a6c7ef,// 261 PAY 258 

    0x3c179ae5,// 262 PAY 259 

    0xf4c4a547,// 263 PAY 260 

    0xb153284e,// 264 PAY 261 

    0xfad82185,// 265 PAY 262 

    0xf4fa1a90,// 266 PAY 263 

    0xf54ef4a7,// 267 PAY 264 

    0xd5634baf,// 268 PAY 265 

    0x8634c66c,// 269 PAY 266 

    0xb99361fe,// 270 PAY 267 

    0x8e93a2c4,// 271 PAY 268 

    0xbad6a7f0,// 272 PAY 269 

    0xe7ecb993,// 273 PAY 270 

    0x6cdec521,// 274 PAY 271 

    0xbb094212,// 275 PAY 272 

    0x7ac27d02,// 276 PAY 273 

    0xe742ab75,// 277 PAY 274 

    0x1b1dc710,// 278 PAY 275 

    0x4af223b4,// 279 PAY 276 

    0xaf4b016e,// 280 PAY 277 

    0xcff8be81,// 281 PAY 278 

    0x4cb1e14b,// 282 PAY 279 

    0x97e4b7e4,// 283 PAY 280 

    0xf16065a2,// 284 PAY 281 

    0x255da140,// 285 PAY 282 

    0xb7c56710,// 286 PAY 283 

    0x0efd3335,// 287 PAY 284 

    0x5502c978,// 288 PAY 285 

    0xe42432d3,// 289 PAY 286 

    0xef793def,// 290 PAY 287 

    0x0a9ac4ae,// 291 PAY 288 

    0xafca5038,// 292 PAY 289 

    0xde0ebef6,// 293 PAY 290 

    0x7490a9c4,// 294 PAY 291 

    0xc1c577c7,// 295 PAY 292 

    0xd65f1ebf,// 296 PAY 293 

    0x34227ab3,// 297 PAY 294 

    0xddbd45e3,// 298 PAY 295 

    0x8f97cd2c,// 299 PAY 296 

    0x34480ecf,// 300 PAY 297 

    0x8f3eefdf,// 301 PAY 298 

    0x139fc2c1,// 302 PAY 299 

    0x54535139,// 303 PAY 300 

    0x3faff849,// 304 PAY 301 

    0xc5571daa,// 305 PAY 302 

    0x4399ff50,// 306 PAY 303 

    0x0facbbdf,// 307 PAY 304 

    0x476180e6,// 308 PAY 305 

    0x4eeba33f,// 309 PAY 306 

    0x99ba0562,// 310 PAY 307 

    0x1137a285,// 311 PAY 308 

    0x7716bfa4,// 312 PAY 309 

    0x2b09da3f,// 313 PAY 310 

    0x393bd955,// 314 PAY 311 

    0xeeb8c7c7,// 315 PAY 312 

    0x53983e53,// 316 PAY 313 

    0x1ec60bdc,// 317 PAY 314 

    0xf0ed2c97,// 318 PAY 315 

    0x0514f54e,// 319 PAY 316 

    0x6a8d964f,// 320 PAY 317 

    0xa1180947,// 321 PAY 318 

    0x3523d983,// 322 PAY 319 

    0xc7c08aba,// 323 PAY 320 

    0xe0d38a4f,// 324 PAY 321 

    0x9448da58,// 325 PAY 322 

    0xa66d5f5e,// 326 PAY 323 

    0xe527c0a6,// 327 PAY 324 

    0xb0f0420a,// 328 PAY 325 

    0x70ca4ea0,// 329 PAY 326 

    0xd8458a7c,// 330 PAY 327 

    0xb2264b77,// 331 PAY 328 

    0x344c62b8,// 332 PAY 329 

    0x019e05f5,// 333 PAY 330 

    0xe448f72a,// 334 PAY 331 

    0xc80a9a8e,// 335 PAY 332 

    0x3e9cb21b,// 336 PAY 333 

    0x4e0608fb,// 337 PAY 334 

    0x02b3ca49,// 338 PAY 335 

    0x27c2ded7,// 339 PAY 336 

    0x480d8cdf,// 340 PAY 337 

    0x66da80f1,// 341 PAY 338 

    0xa793c9d2,// 342 PAY 339 

    0xe292a7ab,// 343 PAY 340 

    0xb2d534f5,// 344 PAY 341 

    0xb3dd48c1,// 345 PAY 342 

    0xea3deaf0,// 346 PAY 343 

    0xbaae643e,// 347 PAY 344 

    0x921344d0,// 348 PAY 345 

    0x63b2ae0e,// 349 PAY 346 

    0x7961055c,// 350 PAY 347 

    0xf12f46c1,// 351 PAY 348 

    0xaea11222,// 352 PAY 349 

    0x4d680ba3,// 353 PAY 350 

    0x68fa38a5,// 354 PAY 351 

    0xc663ff1a,// 355 PAY 352 

    0x9e293171,// 356 PAY 353 

    0x6c2859e9,// 357 PAY 354 

    0x057ff8c3,// 358 PAY 355 

    0xc0dcec4d,// 359 PAY 356 

    0xede3c0e0,// 360 PAY 357 

    0xe8160a2e,// 361 PAY 358 

    0x412a4b39,// 362 PAY 359 

    0xbfb4cb60,// 363 PAY 360 

    0x20a10f60,// 364 PAY 361 

    0x62ad52e5,// 365 PAY 362 

    0x7c05f4d6,// 366 PAY 363 

    0x337d16b8,// 367 PAY 364 

    0xa29f8cc5,// 368 PAY 365 

    0xae644b9c,// 369 PAY 366 

    0x7d3b9b25,// 370 PAY 367 

    0x9560f08d,// 371 PAY 368 

    0x20708367,// 372 PAY 369 

    0x463148d0,// 373 PAY 370 

    0x8880d2de,// 374 PAY 371 

    0xb65ad7ed,// 375 PAY 372 

    0xbc1486ed,// 376 PAY 373 

    0x0c8bc248,// 377 PAY 374 

    0xb9afd9a5,// 378 PAY 375 

    0x0d728098,// 379 PAY 376 

    0x48ea1d9a,// 380 PAY 377 

    0xefc2d27a,// 381 PAY 378 

    0x7e6543ec,// 382 PAY 379 

    0xf56c0c30,// 383 PAY 380 

    0x69e0b1f9,// 384 PAY 381 

    0xafb0c515,// 385 PAY 382 

    0xd66b62a1,// 386 PAY 383 

    0x69f83311,// 387 PAY 384 

    0x9c321abd,// 388 PAY 385 

    0x9e93cedb,// 389 PAY 386 

    0xe20c2324,// 390 PAY 387 

    0x63293ee2,// 391 PAY 388 

    0x4b6bcfce,// 392 PAY 389 

    0x2ad4118c,// 393 PAY 390 

    0x5934cf13,// 394 PAY 391 

    0x24d79dc6,// 395 PAY 392 

    0xc2f97386,// 396 PAY 393 

    0x2cca59e9,// 397 PAY 394 

    0x8959e0fd,// 398 PAY 395 

    0x1e71a849,// 399 PAY 396 

    0x3aabceb4,// 400 PAY 397 

    0x16beb393,// 401 PAY 398 

    0x86bbe201,// 402 PAY 399 

    0x0a0b8130,// 403 PAY 400 

    0x237ff078,// 404 PAY 401 

    0x397216c4,// 405 PAY 402 

    0xabbe6a6e,// 406 PAY 403 

    0xab6558a0,// 407 PAY 404 

    0x0818e7b9,// 408 PAY 405 

    0x987d197f,// 409 PAY 406 

    0x10394fa2,// 410 PAY 407 

    0x8608e0da,// 411 PAY 408 

    0x71a7da78,// 412 PAY 409 

    0xed3a53e3,// 413 PAY 410 

    0x0531caf7,// 414 PAY 411 

    0x0dbbc144,// 415 PAY 412 

    0xca7fd24c,// 416 PAY 413 

    0x841d3bdb,// 417 PAY 414 

    0xdf04e4f0,// 418 PAY 415 

    0x7555ba9e,// 419 PAY 416 

    0x115b7093,// 420 PAY 417 

    0xb894a5ac,// 421 PAY 418 

    0x2b6bbd7f,// 422 PAY 419 

    0x90ace6e3,// 423 PAY 420 

    0xbca47c2e,// 424 PAY 421 

    0x0e4e0263,// 425 PAY 422 

    0x0819af80,// 426 PAY 423 

    0x19ee2008,// 427 PAY 424 

    0x2c1fa4cb,// 428 PAY 425 

    0x932028db,// 429 PAY 426 

    0x63dd7330,// 430 PAY 427 

    0x6cdecf6a,// 431 PAY 428 

    0x1a02a5e2,// 432 PAY 429 

    0x9a8c6c37,// 433 PAY 430 

    0x2cb4777f,// 434 PAY 431 

    0x0e50a966,// 435 PAY 432 

    0x67a3bc9a,// 436 PAY 433 

    0x77d37326,// 437 PAY 434 

    0xfda11521,// 438 PAY 435 

    0xb9916278,// 439 PAY 436 

    0xcd0a7e14,// 440 PAY 437 

    0xfc555937,// 441 PAY 438 

    0xd72d7896,// 442 PAY 439 

    0xa2f54955,// 443 PAY 440 

    0x899458ca,// 444 PAY 441 

    0x1239edbc,// 445 PAY 442 

    0xd17b5a7f,// 446 PAY 443 

    0x1b4d04ab,// 447 PAY 444 

    0x3a89179c,// 448 PAY 445 

    0x6bdaacc7,// 449 PAY 446 

    0xa8a4b603,// 450 PAY 447 

    0xbc83831d,// 451 PAY 448 

    0x186496f8,// 452 PAY 449 

    0x4ce95788,// 453 PAY 450 

    0x20fd4ad1,// 454 PAY 451 

    0xee0d6c0e,// 455 PAY 452 

    0xade60935,// 456 PAY 453 

    0xd734d5a0,// 457 PAY 454 

    0x69b0c73e,// 458 PAY 455 

    0x345d9e76,// 459 PAY 456 

    0xfd174bf6,// 460 PAY 457 

    0x0e95c963,// 461 PAY 458 

    0x28d70a80,// 462 PAY 459 

    0xa148aaf7,// 463 PAY 460 

    0xbc31c22f,// 464 PAY 461 

    0xdf05ddef,// 465 PAY 462 

    0x3d8602af,// 466 PAY 463 

    0x35fabd23,// 467 PAY 464 

    0x9394e1db,// 468 PAY 465 

    0x63bf9056,// 469 PAY 466 

    0x7c7a391c,// 470 PAY 467 

    0xa87d0300,// 471 PAY 468 

/// HASH is  8 bytes 

    0x1a02a5e2,// 472 HSH   1 

    0x9a8c6c37,// 473 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 67 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd1 

    0x0168d143 // 474 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt94_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 161 words. 

/// BDA size     is 637 (0x27d) 

/// BDA id       is 0xda3e 

    0x027dda3e,// 3 BDA   1 

/// PAY Generic Data size   : 637 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xae362d8a,// 4 PAY   1 

    0xe553a634,// 5 PAY   2 

    0x6206a77f,// 6 PAY   3 

    0x037a0d55,// 7 PAY   4 

    0x41a33293,// 8 PAY   5 

    0x4d5dabff,// 9 PAY   6 

    0x8d044684,// 10 PAY   7 

    0x47a682c3,// 11 PAY   8 

    0xbd73eb25,// 12 PAY   9 

    0xa4218b94,// 13 PAY  10 

    0x6b1b9904,// 14 PAY  11 

    0x3a591b9b,// 15 PAY  12 

    0x8353a6f3,// 16 PAY  13 

    0x4bcb4ef0,// 17 PAY  14 

    0x283a278c,// 18 PAY  15 

    0x7d1b8645,// 19 PAY  16 

    0x5d423b44,// 20 PAY  17 

    0x91b89011,// 21 PAY  18 

    0x16e5cdfb,// 22 PAY  19 

    0xb7d53221,// 23 PAY  20 

    0x61ae476a,// 24 PAY  21 

    0x7933dd6a,// 25 PAY  22 

    0x68d18964,// 26 PAY  23 

    0xe2fc55c8,// 27 PAY  24 

    0xc087192b,// 28 PAY  25 

    0x04ae1f93,// 29 PAY  26 

    0x7c155350,// 30 PAY  27 

    0xba2a7256,// 31 PAY  28 

    0xee236594,// 32 PAY  29 

    0x368a2e22,// 33 PAY  30 

    0x56129c52,// 34 PAY  31 

    0xe0f41316,// 35 PAY  32 

    0x925e553f,// 36 PAY  33 

    0x440a4e27,// 37 PAY  34 

    0xc4656d86,// 38 PAY  35 

    0x874dddaf,// 39 PAY  36 

    0x0e1bb789,// 40 PAY  37 

    0x315f5512,// 41 PAY  38 

    0x7782f7df,// 42 PAY  39 

    0x928c29b6,// 43 PAY  40 

    0xc6f4993c,// 44 PAY  41 

    0x594cc51e,// 45 PAY  42 

    0xc4d60531,// 46 PAY  43 

    0xbe228a34,// 47 PAY  44 

    0x496b8175,// 48 PAY  45 

    0x48e02699,// 49 PAY  46 

    0xfe53775d,// 50 PAY  47 

    0x2e732fdc,// 51 PAY  48 

    0x43ffad92,// 52 PAY  49 

    0x0d935195,// 53 PAY  50 

    0x549734a4,// 54 PAY  51 

    0x560a0811,// 55 PAY  52 

    0x05056d88,// 56 PAY  53 

    0x139bd4ee,// 57 PAY  54 

    0xdc9d6397,// 58 PAY  55 

    0x8d3d4df7,// 59 PAY  56 

    0x7ddc6999,// 60 PAY  57 

    0x03cc7d33,// 61 PAY  58 

    0x0e10426c,// 62 PAY  59 

    0x6f114520,// 63 PAY  60 

    0x4a7e9c91,// 64 PAY  61 

    0x7d9207c3,// 65 PAY  62 

    0xfdebe533,// 66 PAY  63 

    0x17deac13,// 67 PAY  64 

    0x98bd2c5b,// 68 PAY  65 

    0xedc15531,// 69 PAY  66 

    0x01dfe88f,// 70 PAY  67 

    0x56d900a6,// 71 PAY  68 

    0xc7b5459c,// 72 PAY  69 

    0x6b115898,// 73 PAY  70 

    0x1f161c92,// 74 PAY  71 

    0x925d1942,// 75 PAY  72 

    0xfd52b005,// 76 PAY  73 

    0x86bd2288,// 77 PAY  74 

    0x9fa9dfc4,// 78 PAY  75 

    0x6659e748,// 79 PAY  76 

    0xcc596a70,// 80 PAY  77 

    0x5572711d,// 81 PAY  78 

    0xcd979aa9,// 82 PAY  79 

    0x8a180a80,// 83 PAY  80 

    0x9302df00,// 84 PAY  81 

    0x6c4a4949,// 85 PAY  82 

    0x2aded96b,// 86 PAY  83 

    0xc3fbfb34,// 87 PAY  84 

    0xcec8f0fb,// 88 PAY  85 

    0x47f0d4c0,// 89 PAY  86 

    0xd3d2049e,// 90 PAY  87 

    0x0af40b7f,// 91 PAY  88 

    0x69fe6f70,// 92 PAY  89 

    0x1c86aff5,// 93 PAY  90 

    0x4bbaca4b,// 94 PAY  91 

    0x2a4d18d1,// 95 PAY  92 

    0xc30ce69e,// 96 PAY  93 

    0xc023c752,// 97 PAY  94 

    0xd5768405,// 98 PAY  95 

    0x002f6508,// 99 PAY  96 

    0x744b509a,// 100 PAY  97 

    0xe0629f3a,// 101 PAY  98 

    0xa1c068ca,// 102 PAY  99 

    0x0624bb84,// 103 PAY 100 

    0x189994a4,// 104 PAY 101 

    0x02c1e174,// 105 PAY 102 

    0x3185d7a8,// 106 PAY 103 

    0x217aeef6,// 107 PAY 104 

    0x675eb064,// 108 PAY 105 

    0x9a5421c7,// 109 PAY 106 

    0xa7055645,// 110 PAY 107 

    0xf5930b1e,// 111 PAY 108 

    0x6b3d0902,// 112 PAY 109 

    0x486d58cc,// 113 PAY 110 

    0xbc7f2ee7,// 114 PAY 111 

    0x7ec91833,// 115 PAY 112 

    0x85faf154,// 116 PAY 113 

    0x5afd4d5c,// 117 PAY 114 

    0x140d2376,// 118 PAY 115 

    0xefcc0725,// 119 PAY 116 

    0x8617ce69,// 120 PAY 117 

    0x45ac7e64,// 121 PAY 118 

    0xe0551f3a,// 122 PAY 119 

    0x96b2d003,// 123 PAY 120 

    0xf0ea9deb,// 124 PAY 121 

    0x815eb931,// 125 PAY 122 

    0xcc043406,// 126 PAY 123 

    0x2b28afc2,// 127 PAY 124 

    0x13c01695,// 128 PAY 125 

    0x3276bcb5,// 129 PAY 126 

    0xd7792ae2,// 130 PAY 127 

    0x6cb8fd5e,// 131 PAY 128 

    0x11eff36a,// 132 PAY 129 

    0xb6a443ac,// 133 PAY 130 

    0xaf9977cb,// 134 PAY 131 

    0x2436eb72,// 135 PAY 132 

    0xefd4e94d,// 136 PAY 133 

    0x6b675c15,// 137 PAY 134 

    0x06adf3fe,// 138 PAY 135 

    0xea2c10e8,// 139 PAY 136 

    0x7daebc06,// 140 PAY 137 

    0xdf42a911,// 141 PAY 138 

    0xc176c3f5,// 142 PAY 139 

    0x314e11f3,// 143 PAY 140 

    0x51a06cb1,// 144 PAY 141 

    0x1786c198,// 145 PAY 142 

    0x5fc5dbeb,// 146 PAY 143 

    0x77668ca9,// 147 PAY 144 

    0x44246e06,// 148 PAY 145 

    0xded0ea4b,// 149 PAY 146 

    0xd16281f7,// 150 PAY 147 

    0x840779b3,// 151 PAY 148 

    0x2b3c5715,// 152 PAY 149 

    0x42b4c53d,// 153 PAY 150 

    0xccb1d773,// 154 PAY 151 

    0x58107ae4,// 155 PAY 152 

    0xabc3ee49,// 156 PAY 153 

    0x163fce12,// 157 PAY 154 

    0x4011008b,// 158 PAY 155 

    0xa74f4635,// 159 PAY 156 

    0xc2fb1fd9,// 160 PAY 157 

    0x74e9c80e,// 161 PAY 158 

    0xc84e0175,// 162 PAY 159 

    0xc0000000,// 163 PAY 160 

/// STA is 1 words. 

/// STA num_pkts       : 175 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc9 

    0x005dc9af // 164 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt95_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 17 words. 

/// BDA size     is 61 (0x3d) 

/// BDA id       is 0x6c9d 

    0x003d6c9d,// 3 BDA   1 

/// PAY Generic Data size   : 61 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xc90dd298,// 4 PAY   1 

    0x0e912aaa,// 5 PAY   2 

    0xdeea2bd3,// 6 PAY   3 

    0xd8db98d9,// 7 PAY   4 

    0x88eaae29,// 8 PAY   5 

    0x1296661b,// 9 PAY   6 

    0xb3b07b5a,// 10 PAY   7 

    0x5d166b8a,// 11 PAY   8 

    0x1ebb0620,// 12 PAY   9 

    0xeac0ba62,// 13 PAY  10 

    0x58980155,// 14 PAY  11 

    0x16401570,// 15 PAY  12 

    0x39aae24d,// 16 PAY  13 

    0x39b4b7ac,// 17 PAY  14 

    0x786f19dc,// 18 PAY  15 

    0xad000000,// 19 PAY  16 

/// STA is 1 words. 

/// STA num_pkts       : 93 

/// STA pkt_idx        : 87 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe2 

    0x015ce25d // 20 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt96_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 299 words. 

/// BDA size     is 1189 (0x4a5) 

/// BDA id       is 0xc09b 

    0x04a5c09b,// 3 BDA   1 

/// PAY Generic Data size   : 1189 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x3b5d0d9e,// 4 PAY   1 

    0x3457678d,// 5 PAY   2 

    0x20912b6b,// 6 PAY   3 

    0x8915c7f8,// 7 PAY   4 

    0x1682b80e,// 8 PAY   5 

    0x9bca88f6,// 9 PAY   6 

    0xada36a51,// 10 PAY   7 

    0x5ed9e197,// 11 PAY   8 

    0x0c5f9fa0,// 12 PAY   9 

    0x522bc859,// 13 PAY  10 

    0x26994aa4,// 14 PAY  11 

    0xf20e4b5d,// 15 PAY  12 

    0x8f3d43a3,// 16 PAY  13 

    0xb698d8e9,// 17 PAY  14 

    0x24599036,// 18 PAY  15 

    0x8a75bfbe,// 19 PAY  16 

    0xf7fd94c5,// 20 PAY  17 

    0xeaa5c2b1,// 21 PAY  18 

    0x22822120,// 22 PAY  19 

    0x86fcd16b,// 23 PAY  20 

    0xcbb53e32,// 24 PAY  21 

    0x7ca951d9,// 25 PAY  22 

    0xbfd62fcd,// 26 PAY  23 

    0x6f346ce5,// 27 PAY  24 

    0x8d0a451d,// 28 PAY  25 

    0xb6424432,// 29 PAY  26 

    0x2460b401,// 30 PAY  27 

    0xa3c59190,// 31 PAY  28 

    0x72ff5378,// 32 PAY  29 

    0xf8860b5b,// 33 PAY  30 

    0x7fcae1a5,// 34 PAY  31 

    0x9ad7f00b,// 35 PAY  32 

    0x270e463c,// 36 PAY  33 

    0x336904bb,// 37 PAY  34 

    0x3ade7187,// 38 PAY  35 

    0xbaf63588,// 39 PAY  36 

    0x7fd2f423,// 40 PAY  37 

    0x454e2314,// 41 PAY  38 

    0x7fa0df6e,// 42 PAY  39 

    0xfe01ec7e,// 43 PAY  40 

    0xda486914,// 44 PAY  41 

    0x33a48130,// 45 PAY  42 

    0xd741678c,// 46 PAY  43 

    0x33494328,// 47 PAY  44 

    0x4208a882,// 48 PAY  45 

    0xc942027a,// 49 PAY  46 

    0x22aefd89,// 50 PAY  47 

    0xde39b7e4,// 51 PAY  48 

    0xa1f4d2e8,// 52 PAY  49 

    0x6e3a8738,// 53 PAY  50 

    0xc5fdc301,// 54 PAY  51 

    0x6769ef3f,// 55 PAY  52 

    0xe84fd84a,// 56 PAY  53 

    0xdb219504,// 57 PAY  54 

    0x5127abf2,// 58 PAY  55 

    0x7f7e2924,// 59 PAY  56 

    0xf136ea50,// 60 PAY  57 

    0x970f1c57,// 61 PAY  58 

    0x7e37425d,// 62 PAY  59 

    0xaccbce3b,// 63 PAY  60 

    0x8704e9e8,// 64 PAY  61 

    0x8db1f003,// 65 PAY  62 

    0xb81ead53,// 66 PAY  63 

    0x423d5bc7,// 67 PAY  64 

    0xadb43b06,// 68 PAY  65 

    0xd6da9f65,// 69 PAY  66 

    0xe6a63704,// 70 PAY  67 

    0x109d97c5,// 71 PAY  68 

    0x1296b6de,// 72 PAY  69 

    0xee792a9b,// 73 PAY  70 

    0x093deba1,// 74 PAY  71 

    0xf5acd6ea,// 75 PAY  72 

    0x22ddff48,// 76 PAY  73 

    0x39bed082,// 77 PAY  74 

    0xc39f551d,// 78 PAY  75 

    0x1a6f473b,// 79 PAY  76 

    0x0dfcfd79,// 80 PAY  77 

    0x015aa6b5,// 81 PAY  78 

    0x986100ff,// 82 PAY  79 

    0x9f9f2630,// 83 PAY  80 

    0xaa723ccb,// 84 PAY  81 

    0xb72d0114,// 85 PAY  82 

    0x2205784c,// 86 PAY  83 

    0x53dcc453,// 87 PAY  84 

    0x02387e26,// 88 PAY  85 

    0x6db2913d,// 89 PAY  86 

    0x13f8c443,// 90 PAY  87 

    0xb0dcf045,// 91 PAY  88 

    0x66dfd6bd,// 92 PAY  89 

    0xa85ce99f,// 93 PAY  90 

    0xd40dadf4,// 94 PAY  91 

    0x08264254,// 95 PAY  92 

    0x2cf66ef1,// 96 PAY  93 

    0xd0705741,// 97 PAY  94 

    0x742b1b2c,// 98 PAY  95 

    0x0a3db824,// 99 PAY  96 

    0x8ce8c94c,// 100 PAY  97 

    0xe07962b8,// 101 PAY  98 

    0x351a19a6,// 102 PAY  99 

    0x1204035e,// 103 PAY 100 

    0x5b916bd4,// 104 PAY 101 

    0xc9da7f61,// 105 PAY 102 

    0x9e98c4e9,// 106 PAY 103 

    0xbe78ebb4,// 107 PAY 104 

    0xd87222a2,// 108 PAY 105 

    0xc7993e7c,// 109 PAY 106 

    0x6ee8439a,// 110 PAY 107 

    0xc8c364f8,// 111 PAY 108 

    0x6eb207b5,// 112 PAY 109 

    0x4bbb7463,// 113 PAY 110 

    0x14167367,// 114 PAY 111 

    0xf536e554,// 115 PAY 112 

    0xa2fb80b7,// 116 PAY 113 

    0xb0de2eb0,// 117 PAY 114 

    0xa8f44d2d,// 118 PAY 115 

    0x5ef5b2ab,// 119 PAY 116 

    0xbf302d7d,// 120 PAY 117 

    0x0703e0d2,// 121 PAY 118 

    0x4cb21dd7,// 122 PAY 119 

    0x3c5d0548,// 123 PAY 120 

    0x5137c430,// 124 PAY 121 

    0xb98045f7,// 125 PAY 122 

    0xfb77ad28,// 126 PAY 123 

    0x17f5d9d9,// 127 PAY 124 

    0xcbb43004,// 128 PAY 125 

    0x0cdd29eb,// 129 PAY 126 

    0x16cff02a,// 130 PAY 127 

    0xbbbaa28c,// 131 PAY 128 

    0x9d4c1e9a,// 132 PAY 129 

    0xe4313eb3,// 133 PAY 130 

    0xdede1d88,// 134 PAY 131 

    0xc4c1c340,// 135 PAY 132 

    0x8e9074cb,// 136 PAY 133 

    0x5a4dbabf,// 137 PAY 134 

    0x863ba62f,// 138 PAY 135 

    0x47c5c280,// 139 PAY 136 

    0xeb812100,// 140 PAY 137 

    0xe02833fa,// 141 PAY 138 

    0x6d1ff2e6,// 142 PAY 139 

    0xf5a968ae,// 143 PAY 140 

    0x80ca9ee0,// 144 PAY 141 

    0xa10ba397,// 145 PAY 142 

    0x1a8a5af8,// 146 PAY 143 

    0x2476f825,// 147 PAY 144 

    0x1037e715,// 148 PAY 145 

    0x50b3df46,// 149 PAY 146 

    0x1ec4aa43,// 150 PAY 147 

    0xe666957d,// 151 PAY 148 

    0x79dc6607,// 152 PAY 149 

    0xb27d72e5,// 153 PAY 150 

    0xfa6cd44d,// 154 PAY 151 

    0x3edab3ec,// 155 PAY 152 

    0xf737991d,// 156 PAY 153 

    0x9412aa63,// 157 PAY 154 

    0x8f1fc029,// 158 PAY 155 

    0x0f18a6e1,// 159 PAY 156 

    0x574b88d3,// 160 PAY 157 

    0x12f64404,// 161 PAY 158 

    0x1b51d2a2,// 162 PAY 159 

    0xf4b5590a,// 163 PAY 160 

    0xd52b0fe4,// 164 PAY 161 

    0x9ce658e6,// 165 PAY 162 

    0xefa75a14,// 166 PAY 163 

    0xba60ce52,// 167 PAY 164 

    0x37ea34fd,// 168 PAY 165 

    0x6d9ae3cb,// 169 PAY 166 

    0xfb18e0eb,// 170 PAY 167 

    0x41c2599e,// 171 PAY 168 

    0x2d5475dc,// 172 PAY 169 

    0x6af6826c,// 173 PAY 170 

    0x67da6bf6,// 174 PAY 171 

    0x1d12f200,// 175 PAY 172 

    0x173442b1,// 176 PAY 173 

    0xc29bf154,// 177 PAY 174 

    0xfdd0c00a,// 178 PAY 175 

    0x7b379c49,// 179 PAY 176 

    0xf0583cf3,// 180 PAY 177 

    0xc0aa4d76,// 181 PAY 178 

    0x07faea15,// 182 PAY 179 

    0x82d77572,// 183 PAY 180 

    0x5f3b2fe0,// 184 PAY 181 

    0x9295f5e8,// 185 PAY 182 

    0xc9f1fd68,// 186 PAY 183 

    0x9736f5d6,// 187 PAY 184 

    0xf6e84f4c,// 188 PAY 185 

    0xbfad8ec1,// 189 PAY 186 

    0x2151df9c,// 190 PAY 187 

    0xde60ca59,// 191 PAY 188 

    0xeb705b96,// 192 PAY 189 

    0x8e29b711,// 193 PAY 190 

    0xfeaa07d3,// 194 PAY 191 

    0x0ac3cfb6,// 195 PAY 192 

    0x9af487b3,// 196 PAY 193 

    0x6797d5b6,// 197 PAY 194 

    0x88ee8986,// 198 PAY 195 

    0x4a8de91e,// 199 PAY 196 

    0x3d90a3ca,// 200 PAY 197 

    0x43fbac14,// 201 PAY 198 

    0x9ac4c7dc,// 202 PAY 199 

    0x8d7693b8,// 203 PAY 200 

    0x6d8a2d4f,// 204 PAY 201 

    0xfb9b59c6,// 205 PAY 202 

    0xb1f72fd5,// 206 PAY 203 

    0xd57251b8,// 207 PAY 204 

    0x5382ccc6,// 208 PAY 205 

    0xfae58490,// 209 PAY 206 

    0x3b09ce83,// 210 PAY 207 

    0x87caac0b,// 211 PAY 208 

    0xce93d948,// 212 PAY 209 

    0x640b0c9d,// 213 PAY 210 

    0xbabead07,// 214 PAY 211 

    0xbf5cdc5a,// 215 PAY 212 

    0xb6d7de09,// 216 PAY 213 

    0x6a141576,// 217 PAY 214 

    0x6b400273,// 218 PAY 215 

    0xb9dd3f24,// 219 PAY 216 

    0xc9742345,// 220 PAY 217 

    0xdf30ecbc,// 221 PAY 218 

    0x6135fd42,// 222 PAY 219 

    0x913a7f0c,// 223 PAY 220 

    0xfa1cb680,// 224 PAY 221 

    0x4090d208,// 225 PAY 222 

    0xe100b3d7,// 226 PAY 223 

    0xc9327d06,// 227 PAY 224 

    0x7fc11fa8,// 228 PAY 225 

    0x1bbb10bd,// 229 PAY 226 

    0xc03ab09f,// 230 PAY 227 

    0x0967afb7,// 231 PAY 228 

    0x5c4eaa29,// 232 PAY 229 

    0xcca187a7,// 233 PAY 230 

    0x66d5c26a,// 234 PAY 231 

    0x7905651a,// 235 PAY 232 

    0x0843f2fb,// 236 PAY 233 

    0xf03480b4,// 237 PAY 234 

    0x2b6d180c,// 238 PAY 235 

    0x620f38df,// 239 PAY 236 

    0x75f88e51,// 240 PAY 237 

    0x93d51013,// 241 PAY 238 

    0xa0e35fa9,// 242 PAY 239 

    0xde38a0b8,// 243 PAY 240 

    0x04b8d66d,// 244 PAY 241 

    0xc13e6963,// 245 PAY 242 

    0xa5ce7404,// 246 PAY 243 

    0xfaa26664,// 247 PAY 244 

    0x94c4af3e,// 248 PAY 245 

    0x5c102c15,// 249 PAY 246 

    0x0ef37c76,// 250 PAY 247 

    0x6a360421,// 251 PAY 248 

    0x06e62524,// 252 PAY 249 

    0xcebe3b8b,// 253 PAY 250 

    0x3306c297,// 254 PAY 251 

    0x52c90e69,// 255 PAY 252 

    0xb21d0bff,// 256 PAY 253 

    0x77cb8b9b,// 257 PAY 254 

    0x995884b3,// 258 PAY 255 

    0x27c92d97,// 259 PAY 256 

    0x552a4a61,// 260 PAY 257 

    0xea6ad244,// 261 PAY 258 

    0x6017b67d,// 262 PAY 259 

    0x3bd219fe,// 263 PAY 260 

    0x1f0eeb3a,// 264 PAY 261 

    0xe20cea50,// 265 PAY 262 

    0x53dfc3ca,// 266 PAY 263 

    0xaedb644d,// 267 PAY 264 

    0xcc625304,// 268 PAY 265 

    0xf7f1b4ce,// 269 PAY 266 

    0xfe6c1269,// 270 PAY 267 

    0xdf7a3dc9,// 271 PAY 268 

    0x2c1bebfe,// 272 PAY 269 

    0x01062dd0,// 273 PAY 270 

    0x1cf35544,// 274 PAY 271 

    0xc5c382ec,// 275 PAY 272 

    0x90a14896,// 276 PAY 273 

    0x162cd877,// 277 PAY 274 

    0x4b88ace8,// 278 PAY 275 

    0xb8c98388,// 279 PAY 276 

    0x63d9d0de,// 280 PAY 277 

    0x58e52970,// 281 PAY 278 

    0x2e233bc3,// 282 PAY 279 

    0x30cfb941,// 283 PAY 280 

    0x2154f52c,// 284 PAY 281 

    0x441b427d,// 285 PAY 282 

    0x064aced4,// 286 PAY 283 

    0x464059af,// 287 PAY 284 

    0x79069456,// 288 PAY 285 

    0x8b575732,// 289 PAY 286 

    0xedbf5be7,// 290 PAY 287 

    0x4a04835d,// 291 PAY 288 

    0xd237df66,// 292 PAY 289 

    0xb77dd717,// 293 PAY 290 

    0x949cbd8a,// 294 PAY 291 

    0xa672cf66,// 295 PAY 292 

    0xa3db6d2d,// 296 PAY 293 

    0xffd9c5be,// 297 PAY 294 

    0x0bae85c3,// 298 PAY 295 

    0xb8eb25f1,// 299 PAY 296 

    0xbc1fbe99,// 300 PAY 297 

    0xd2000000,// 301 PAY 298 

/// HASH is  12 bytes 

    0x52c90e69,// 302 HSH   1 

    0xb21d0bff,// 303 HSH   2 

    0x77cb8b9b,// 304 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 201 

/// STA pkt_idx        : 14 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5 

    0x003905c9 // 305 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt97_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 222 words. 

/// BDA size     is 882 (0x372) 

/// BDA id       is 0x2790 

    0x03722790,// 3 BDA   1 

/// PAY Generic Data size   : 882 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x2f95b3d4,// 4 PAY   1 

    0x473b7103,// 5 PAY   2 

    0x093079d0,// 6 PAY   3 

    0x766f6094,// 7 PAY   4 

    0xf4790f27,// 8 PAY   5 

    0xcacabf01,// 9 PAY   6 

    0xec472a8b,// 10 PAY   7 

    0xb334bf3c,// 11 PAY   8 

    0x35502e9e,// 12 PAY   9 

    0x3402b5aa,// 13 PAY  10 

    0x31c11f89,// 14 PAY  11 

    0x89300aa5,// 15 PAY  12 

    0x4985b46c,// 16 PAY  13 

    0xcd3cf4c6,// 17 PAY  14 

    0x4004c4f7,// 18 PAY  15 

    0x6a054f3f,// 19 PAY  16 

    0x35834fa5,// 20 PAY  17 

    0x2dc7a057,// 21 PAY  18 

    0x4d621ce5,// 22 PAY  19 

    0x460433ba,// 23 PAY  20 

    0x289140ad,// 24 PAY  21 

    0x1bcfdb94,// 25 PAY  22 

    0x8ca185f6,// 26 PAY  23 

    0xf7e9f6e0,// 27 PAY  24 

    0x1ec7e018,// 28 PAY  25 

    0xf78f7fbf,// 29 PAY  26 

    0x60f37eb5,// 30 PAY  27 

    0xc8971f88,// 31 PAY  28 

    0x60882344,// 32 PAY  29 

    0x22b77518,// 33 PAY  30 

    0x0744c6a7,// 34 PAY  31 

    0x875756a4,// 35 PAY  32 

    0x0951d18a,// 36 PAY  33 

    0x6b453f98,// 37 PAY  34 

    0xda1ffb02,// 38 PAY  35 

    0xf8b0bf0d,// 39 PAY  36 

    0xc6a35fd5,// 40 PAY  37 

    0xf3a56447,// 41 PAY  38 

    0xcd6fee7b,// 42 PAY  39 

    0xfd746552,// 43 PAY  40 

    0x3dda39e0,// 44 PAY  41 

    0x7fb4c49d,// 45 PAY  42 

    0x0683d208,// 46 PAY  43 

    0xcb629963,// 47 PAY  44 

    0x54db0974,// 48 PAY  45 

    0x541d29a8,// 49 PAY  46 

    0xca16d9c2,// 50 PAY  47 

    0x925b347c,// 51 PAY  48 

    0x4f4b8d9d,// 52 PAY  49 

    0x3f542b81,// 53 PAY  50 

    0xf2a3420b,// 54 PAY  51 

    0xfd0b7d69,// 55 PAY  52 

    0xaf8e49c6,// 56 PAY  53 

    0x6b722093,// 57 PAY  54 

    0x6dab3b54,// 58 PAY  55 

    0xe6b49407,// 59 PAY  56 

    0x93c70f8c,// 60 PAY  57 

    0x3ad10393,// 61 PAY  58 

    0x23db3986,// 62 PAY  59 

    0x9ab7a1d0,// 63 PAY  60 

    0x3339240c,// 64 PAY  61 

    0xeeee3e0e,// 65 PAY  62 

    0x18897f08,// 66 PAY  63 

    0x94ec2c96,// 67 PAY  64 

    0x14f558a4,// 68 PAY  65 

    0x2e39978b,// 69 PAY  66 

    0x1837ea64,// 70 PAY  67 

    0xf795a602,// 71 PAY  68 

    0x063fe9f4,// 72 PAY  69 

    0x22b66927,// 73 PAY  70 

    0x68e9b3a7,// 74 PAY  71 

    0xdd296426,// 75 PAY  72 

    0x5ac210f2,// 76 PAY  73 

    0x974f6123,// 77 PAY  74 

    0xffdc6580,// 78 PAY  75 

    0x9154dc15,// 79 PAY  76 

    0x3d30fafb,// 80 PAY  77 

    0xde621c1f,// 81 PAY  78 

    0xca32228d,// 82 PAY  79 

    0x6f96df5b,// 83 PAY  80 

    0x4cc835ad,// 84 PAY  81 

    0x6a978ed0,// 85 PAY  82 

    0x6a53f675,// 86 PAY  83 

    0xb45d3d7b,// 87 PAY  84 

    0x46941c7b,// 88 PAY  85 

    0x14f27438,// 89 PAY  86 

    0x3cf6f53a,// 90 PAY  87 

    0x18e51029,// 91 PAY  88 

    0xe52cf42b,// 92 PAY  89 

    0xc4c89986,// 93 PAY  90 

    0x6a424614,// 94 PAY  91 

    0x22b74d53,// 95 PAY  92 

    0xf64dc69d,// 96 PAY  93 

    0x1459e87b,// 97 PAY  94 

    0x1bc72d7a,// 98 PAY  95 

    0xfc3d76c7,// 99 PAY  96 

    0x08e8f70c,// 100 PAY  97 

    0x7f3247af,// 101 PAY  98 

    0xffee02d1,// 102 PAY  99 

    0x001707c3,// 103 PAY 100 

    0x35a16832,// 104 PAY 101 

    0x8b57e16a,// 105 PAY 102 

    0xc863a4a6,// 106 PAY 103 

    0x86e6280f,// 107 PAY 104 

    0x19cbe3b8,// 108 PAY 105 

    0xee59dbde,// 109 PAY 106 

    0xd0fda965,// 110 PAY 107 

    0x29219140,// 111 PAY 108 

    0xa66339b3,// 112 PAY 109 

    0xd7818352,// 113 PAY 110 

    0x4141fd66,// 114 PAY 111 

    0xeccbc612,// 115 PAY 112 

    0xe71ea8c7,// 116 PAY 113 

    0x4a65e69d,// 117 PAY 114 

    0xc117737c,// 118 PAY 115 

    0x0021eb6c,// 119 PAY 116 

    0xd1e5ee18,// 120 PAY 117 

    0xa61ccd8d,// 121 PAY 118 

    0x6dfa57d1,// 122 PAY 119 

    0xd8803e78,// 123 PAY 120 

    0x86309f9f,// 124 PAY 121 

    0x5908f7d0,// 125 PAY 122 

    0x23037919,// 126 PAY 123 

    0x59ae0524,// 127 PAY 124 

    0x3d4f7064,// 128 PAY 125 

    0xbec04593,// 129 PAY 126 

    0xaca216ca,// 130 PAY 127 

    0x28dc3e5a,// 131 PAY 128 

    0x43aad52b,// 132 PAY 129 

    0xb0f07d56,// 133 PAY 130 

    0x30795180,// 134 PAY 131 

    0x34ef9e35,// 135 PAY 132 

    0xf2623ccb,// 136 PAY 133 

    0x9fa5c46a,// 137 PAY 134 

    0x8fd14981,// 138 PAY 135 

    0x025800ad,// 139 PAY 136 

    0xeaede4c0,// 140 PAY 137 

    0x27f6bb5a,// 141 PAY 138 

    0x4ff3523c,// 142 PAY 139 

    0x8040f667,// 143 PAY 140 

    0x12b9011a,// 144 PAY 141 

    0x9794f4ee,// 145 PAY 142 

    0xf5cbbded,// 146 PAY 143 

    0x8bc2f60d,// 147 PAY 144 

    0x84a4cea9,// 148 PAY 145 

    0x5ffeceea,// 149 PAY 146 

    0x0e55308c,// 150 PAY 147 

    0x69535c1c,// 151 PAY 148 

    0x63fdf839,// 152 PAY 149 

    0x91b12173,// 153 PAY 150 

    0xed388a2a,// 154 PAY 151 

    0x2d63e11d,// 155 PAY 152 

    0x11c19c89,// 156 PAY 153 

    0xbb7bcd25,// 157 PAY 154 

    0xf19784ea,// 158 PAY 155 

    0xa400b2da,// 159 PAY 156 

    0x6b48b449,// 160 PAY 157 

    0x8925b08e,// 161 PAY 158 

    0xd1b1cb14,// 162 PAY 159 

    0x1a435951,// 163 PAY 160 

    0x266661fe,// 164 PAY 161 

    0xffa78077,// 165 PAY 162 

    0x61615b2c,// 166 PAY 163 

    0x55f29be9,// 167 PAY 164 

    0x69b43a9c,// 168 PAY 165 

    0x8b1584aa,// 169 PAY 166 

    0xdd2324bb,// 170 PAY 167 

    0x3bc4fa5d,// 171 PAY 168 

    0x9f6c4f2a,// 172 PAY 169 

    0x130c69be,// 173 PAY 170 

    0x468d9ee2,// 174 PAY 171 

    0xa0e74101,// 175 PAY 172 

    0x7beffb68,// 176 PAY 173 

    0x3975f845,// 177 PAY 174 

    0x2771d777,// 178 PAY 175 

    0x0807f5bf,// 179 PAY 176 

    0x2c62eb9e,// 180 PAY 177 

    0x5f81e5d6,// 181 PAY 178 

    0xbb5a4f77,// 182 PAY 179 

    0x93ae6646,// 183 PAY 180 

    0xdcd3d63e,// 184 PAY 181 

    0xa5a40c64,// 185 PAY 182 

    0x96dda0af,// 186 PAY 183 

    0xde7251b2,// 187 PAY 184 

    0x4b572b03,// 188 PAY 185 

    0x23d22e3e,// 189 PAY 186 

    0x4c72183a,// 190 PAY 187 

    0xe40ff4c3,// 191 PAY 188 

    0x6a86a3a9,// 192 PAY 189 

    0x4ae5eda1,// 193 PAY 190 

    0x3975d5c0,// 194 PAY 191 

    0x053fdc6b,// 195 PAY 192 

    0x6424f42e,// 196 PAY 193 

    0x3fe66582,// 197 PAY 194 

    0xe866732d,// 198 PAY 195 

    0xa2030fcc,// 199 PAY 196 

    0x7e1d8c78,// 200 PAY 197 

    0x4335fb2f,// 201 PAY 198 

    0x4237ebcb,// 202 PAY 199 

    0xb3c6329b,// 203 PAY 200 

    0x78dcc626,// 204 PAY 201 

    0xa9926a9a,// 205 PAY 202 

    0x874dc714,// 206 PAY 203 

    0x9c5c79c1,// 207 PAY 204 

    0x75b14c58,// 208 PAY 205 

    0xbb5a8288,// 209 PAY 206 

    0x856fcb6c,// 210 PAY 207 

    0x079b00e4,// 211 PAY 208 

    0x0f26dff0,// 212 PAY 209 

    0xc9334ac7,// 213 PAY 210 

    0xf11a1105,// 214 PAY 211 

    0x1d7b3577,// 215 PAY 212 

    0x3e10f5f2,// 216 PAY 213 

    0x8a8e547b,// 217 PAY 214 

    0x5d407e61,// 218 PAY 215 

    0x6c90398b,// 219 PAY 216 

    0x078394fb,// 220 PAY 217 

    0xbb802e08,// 221 PAY 218 

    0x5fc46260,// 222 PAY 219 

    0xc24b4f91,// 223 PAY 220 

    0xf2db0000,// 224 PAY 221 

/// HASH is  8 bytes 

    0xfd746552,// 225 HSH   1 

    0x3dda39e0,// 226 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 248 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbf 

    0x03e1bf74 // 227 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt98_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 60 words. 

/// BDA size     is 234 (0xea) 

/// BDA id       is 0xf658 

    0x00eaf658,// 3 BDA   1 

/// PAY Generic Data size   : 234 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x8363a6e3,// 4 PAY   1 

    0xa985e7d1,// 5 PAY   2 

    0xcba972f4,// 6 PAY   3 

    0xea81817d,// 7 PAY   4 

    0x80f94fe1,// 8 PAY   5 

    0x640be5bc,// 9 PAY   6 

    0xf7db2d4b,// 10 PAY   7 

    0x5ccfb42f,// 11 PAY   8 

    0x789d3d6c,// 12 PAY   9 

    0x7e8705d3,// 13 PAY  10 

    0x9a247ff8,// 14 PAY  11 

    0x5c3f7fa8,// 15 PAY  12 

    0xc00864fa,// 16 PAY  13 

    0x9dbd9b28,// 17 PAY  14 

    0x399917d7,// 18 PAY  15 

    0x47c86d30,// 19 PAY  16 

    0x10cfbd64,// 20 PAY  17 

    0xef59cf25,// 21 PAY  18 

    0x84b8a6f7,// 22 PAY  19 

    0x1df1a5d6,// 23 PAY  20 

    0x2a275a99,// 24 PAY  21 

    0x0ce8d359,// 25 PAY  22 

    0x6f2e1211,// 26 PAY  23 

    0xa403c705,// 27 PAY  24 

    0x2fa66c2d,// 28 PAY  25 

    0x4faec264,// 29 PAY  26 

    0x15ce63b2,// 30 PAY  27 

    0x533f5278,// 31 PAY  28 

    0xec079bc2,// 32 PAY  29 

    0xdd95f0f7,// 33 PAY  30 

    0x10709c44,// 34 PAY  31 

    0x17817dc7,// 35 PAY  32 

    0xc72ccdd4,// 36 PAY  33 

    0xe09e6a30,// 37 PAY  34 

    0x1ba8563b,// 38 PAY  35 

    0xbd9621f4,// 39 PAY  36 

    0x80ea120a,// 40 PAY  37 

    0xe50e2fc9,// 41 PAY  38 

    0x2c918a6b,// 42 PAY  39 

    0xb559e893,// 43 PAY  40 

    0xda6c169d,// 44 PAY  41 

    0xd415f70c,// 45 PAY  42 

    0xc56b6739,// 46 PAY  43 

    0x2c20e8d4,// 47 PAY  44 

    0xe428296a,// 48 PAY  45 

    0xe72bfc9a,// 49 PAY  46 

    0x0a54e151,// 50 PAY  47 

    0x552cd65d,// 51 PAY  48 

    0x46362189,// 52 PAY  49 

    0xb30b2e21,// 53 PAY  50 

    0xb1f94e6c,// 54 PAY  51 

    0xae1b1ec7,// 55 PAY  52 

    0x98d22ca3,// 56 PAY  53 

    0x0ac62aee,// 57 PAY  54 

    0x1004ab49,// 58 PAY  55 

    0xe0c704ee,// 59 PAY  56 

    0xe9ebf5f7,// 60 PAY  57 

    0x0f3be7e7,// 61 PAY  58 

    0x0f050000,// 62 PAY  59 

/// STA is 1 words. 

/// STA num_pkts       : 65 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8 

    0x02b40841 // 63 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des3_md5_pkt99_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 347 words. 

/// BDA size     is 1381 (0x565) 

/// BDA id       is 0xb4f7 

    0x0565b4f7,// 3 BDA   1 

/// PAY Generic Data size   : 1381 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x2e53a752,// 4 PAY   1 

    0x2b5c5c34,// 5 PAY   2 

    0xac18672b,// 6 PAY   3 

    0x6edfb7dd,// 7 PAY   4 

    0x6bf8bb58,// 8 PAY   5 

    0x6d08dc82,// 9 PAY   6 

    0x2601b966,// 10 PAY   7 

    0x86e0342d,// 11 PAY   8 

    0xe05a9949,// 12 PAY   9 

    0x10b8958f,// 13 PAY  10 

    0x5f55a06d,// 14 PAY  11 

    0x6980ebb9,// 15 PAY  12 

    0x2a244a0d,// 16 PAY  13 

    0x4b09cd9f,// 17 PAY  14 

    0xe5856655,// 18 PAY  15 

    0x70c5e78b,// 19 PAY  16 

    0x8601711e,// 20 PAY  17 

    0x8d02c7a8,// 21 PAY  18 

    0xb1e19707,// 22 PAY  19 

    0xa93027fb,// 23 PAY  20 

    0xde6ca96d,// 24 PAY  21 

    0xf568f797,// 25 PAY  22 

    0xffef8852,// 26 PAY  23 

    0xc913b011,// 27 PAY  24 

    0xf62d9609,// 28 PAY  25 

    0x6288eed6,// 29 PAY  26 

    0x535deec4,// 30 PAY  27 

    0x9843462b,// 31 PAY  28 

    0x6aeff210,// 32 PAY  29 

    0x05c251b5,// 33 PAY  30 

    0xc14e72f3,// 34 PAY  31 

    0x4155c599,// 35 PAY  32 

    0x7649eeba,// 36 PAY  33 

    0x246d43bf,// 37 PAY  34 

    0x53a3633b,// 38 PAY  35 

    0x06cb2ab6,// 39 PAY  36 

    0x793aaebe,// 40 PAY  37 

    0x69d2294a,// 41 PAY  38 

    0xe86a7509,// 42 PAY  39 

    0x786b71da,// 43 PAY  40 

    0x6a5f820d,// 44 PAY  41 

    0xd7578efd,// 45 PAY  42 

    0x4157255e,// 46 PAY  43 

    0xc70250f7,// 47 PAY  44 

    0x0f6cd78d,// 48 PAY  45 

    0x6b4f5844,// 49 PAY  46 

    0xc2e3e97e,// 50 PAY  47 

    0x83781b35,// 51 PAY  48 

    0x75d99980,// 52 PAY  49 

    0xdad55131,// 53 PAY  50 

    0x4b69704e,// 54 PAY  51 

    0xae66c48e,// 55 PAY  52 

    0x6e6e3022,// 56 PAY  53 

    0x3e8119b2,// 57 PAY  54 

    0xa71fad92,// 58 PAY  55 

    0x83023526,// 59 PAY  56 

    0xdf597a5f,// 60 PAY  57 

    0x3ffa0477,// 61 PAY  58 

    0xc66c9f83,// 62 PAY  59 

    0xf8dfcd62,// 63 PAY  60 

    0xdc67262f,// 64 PAY  61 

    0x63cf59c6,// 65 PAY  62 

    0x907781ce,// 66 PAY  63 

    0x7b21e0cb,// 67 PAY  64 

    0xecd441d1,// 68 PAY  65 

    0x671d4d41,// 69 PAY  66 

    0x2e7887e8,// 70 PAY  67 

    0x48b328b8,// 71 PAY  68 

    0xdfda6874,// 72 PAY  69 

    0x092bbf5c,// 73 PAY  70 

    0x4b7278da,// 74 PAY  71 

    0x3893a650,// 75 PAY  72 

    0xf0b5dca8,// 76 PAY  73 

    0x1a4c1df8,// 77 PAY  74 

    0x0e214015,// 78 PAY  75 

    0x68af4082,// 79 PAY  76 

    0x2aa14c2e,// 80 PAY  77 

    0xb5b09073,// 81 PAY  78 

    0xf9c6002e,// 82 PAY  79 

    0xdeb817b7,// 83 PAY  80 

    0xc9e6d143,// 84 PAY  81 

    0x1a65bfdc,// 85 PAY  82 

    0xcc4c3d5c,// 86 PAY  83 

    0x2c04e196,// 87 PAY  84 

    0x4a3e7844,// 88 PAY  85 

    0x899f1579,// 89 PAY  86 

    0xad2d3643,// 90 PAY  87 

    0x47cbd32f,// 91 PAY  88 

    0x799e75f4,// 92 PAY  89 

    0xd7e451cf,// 93 PAY  90 

    0x23f8bc05,// 94 PAY  91 

    0x4ad6e55a,// 95 PAY  92 

    0xa3ad7703,// 96 PAY  93 

    0x8c1b351a,// 97 PAY  94 

    0x1c3910c4,// 98 PAY  95 

    0x754ad73b,// 99 PAY  96 

    0xf5e53350,// 100 PAY  97 

    0xbd0c755e,// 101 PAY  98 

    0x6959207e,// 102 PAY  99 

    0x4132e9de,// 103 PAY 100 

    0x34cf9cbf,// 104 PAY 101 

    0xf1c1796f,// 105 PAY 102 

    0xd0099278,// 106 PAY 103 

    0x3581a768,// 107 PAY 104 

    0x039b2da2,// 108 PAY 105 

    0x8d626d7b,// 109 PAY 106 

    0xf2622d2c,// 110 PAY 107 

    0xc291e1a9,// 111 PAY 108 

    0x2c22cd1e,// 112 PAY 109 

    0x52a33873,// 113 PAY 110 

    0x0e28409d,// 114 PAY 111 

    0xf094d07a,// 115 PAY 112 

    0xf7752a15,// 116 PAY 113 

    0x1ebca9bb,// 117 PAY 114 

    0xfb9dab58,// 118 PAY 115 

    0x2fa3aab2,// 119 PAY 116 

    0x3634b3e7,// 120 PAY 117 

    0xaf718ee0,// 121 PAY 118 

    0x2335e2a4,// 122 PAY 119 

    0xc73fa008,// 123 PAY 120 

    0x3326bf60,// 124 PAY 121 

    0x0511d190,// 125 PAY 122 

    0xd5ddea93,// 126 PAY 123 

    0x5a764be9,// 127 PAY 124 

    0x0a10fbde,// 128 PAY 125 

    0xfb43a483,// 129 PAY 126 

    0x5f08077e,// 130 PAY 127 

    0xec1ffd22,// 131 PAY 128 

    0xca9b2c35,// 132 PAY 129 

    0x80b6a534,// 133 PAY 130 

    0x93e01ab1,// 134 PAY 131 

    0x6ccc8a90,// 135 PAY 132 

    0x91fca771,// 136 PAY 133 

    0x530a2227,// 137 PAY 134 

    0x944278ce,// 138 PAY 135 

    0xf0e02282,// 139 PAY 136 

    0x7f48bc11,// 140 PAY 137 

    0x1823dfa3,// 141 PAY 138 

    0x507caf1f,// 142 PAY 139 

    0xc1ee0e58,// 143 PAY 140 

    0x0f77ae95,// 144 PAY 141 

    0x6dd2eedb,// 145 PAY 142 

    0xc5f26adc,// 146 PAY 143 

    0xf35c8890,// 147 PAY 144 

    0x3de01ac6,// 148 PAY 145 

    0x507fc514,// 149 PAY 146 

    0x504a1508,// 150 PAY 147 

    0xb5ff82be,// 151 PAY 148 

    0xad3c2a33,// 152 PAY 149 

    0x8fe89522,// 153 PAY 150 

    0xa48abe31,// 154 PAY 151 

    0x73b80478,// 155 PAY 152 

    0x3bcd18af,// 156 PAY 153 

    0xedd6059b,// 157 PAY 154 

    0x5173070f,// 158 PAY 155 

    0x5063b5ef,// 159 PAY 156 

    0x6382a21d,// 160 PAY 157 

    0x9199fb6f,// 161 PAY 158 

    0x68d327f8,// 162 PAY 159 

    0xc956f950,// 163 PAY 160 

    0xecb24467,// 164 PAY 161 

    0xe6492368,// 165 PAY 162 

    0x35e2d1e8,// 166 PAY 163 

    0xc4a06225,// 167 PAY 164 

    0xa38a3dd0,// 168 PAY 165 

    0x63e95406,// 169 PAY 166 

    0x3fd0ed8e,// 170 PAY 167 

    0xbed4544d,// 171 PAY 168 

    0x9aa94de1,// 172 PAY 169 

    0xdaef7a44,// 173 PAY 170 

    0x4b6a3016,// 174 PAY 171 

    0x2164543b,// 175 PAY 172 

    0x86637d54,// 176 PAY 173 

    0x15788cc4,// 177 PAY 174 

    0x515ce1b9,// 178 PAY 175 

    0xa9ac7ed7,// 179 PAY 176 

    0x6cd5dfc8,// 180 PAY 177 

    0x9b720664,// 181 PAY 178 

    0xa47ba4d4,// 182 PAY 179 

    0x5c2a95f8,// 183 PAY 180 

    0xa4d72e70,// 184 PAY 181 

    0x8427ba4f,// 185 PAY 182 

    0x3a5afc5e,// 186 PAY 183 

    0xe2926818,// 187 PAY 184 

    0xc3c5f5bc,// 188 PAY 185 

    0x6c445c55,// 189 PAY 186 

    0xcf4a6aa1,// 190 PAY 187 

    0xe508e137,// 191 PAY 188 

    0x02224e06,// 192 PAY 189 

    0xc1cb6ca9,// 193 PAY 190 

    0x3e48689b,// 194 PAY 191 

    0x0f0b65dc,// 195 PAY 192 

    0x4d00a8ab,// 196 PAY 193 

    0xbba38c78,// 197 PAY 194 

    0xa9f62989,// 198 PAY 195 

    0x91132f0a,// 199 PAY 196 

    0xfdf4ba7c,// 200 PAY 197 

    0xaf2096bb,// 201 PAY 198 

    0x1dd3e5e5,// 202 PAY 199 

    0x0bece3ca,// 203 PAY 200 

    0x4ab33578,// 204 PAY 201 

    0x5a6de512,// 205 PAY 202 

    0xe582d3dc,// 206 PAY 203 

    0x9bc2d971,// 207 PAY 204 

    0x32e1bfad,// 208 PAY 205 

    0x2f47b7be,// 209 PAY 206 

    0x17550d50,// 210 PAY 207 

    0xc4e5cc10,// 211 PAY 208 

    0x4ad1e963,// 212 PAY 209 

    0x483d49a7,// 213 PAY 210 

    0x2b842feb,// 214 PAY 211 

    0x95cf8751,// 215 PAY 212 

    0x82dd5d12,// 216 PAY 213 

    0x884026bb,// 217 PAY 214 

    0x6487a997,// 218 PAY 215 

    0xccb6b216,// 219 PAY 216 

    0x661a233c,// 220 PAY 217 

    0x1e15c245,// 221 PAY 218 

    0xfafc43a2,// 222 PAY 219 

    0x93db1f14,// 223 PAY 220 

    0xb4e0b29b,// 224 PAY 221 

    0x2557e828,// 225 PAY 222 

    0x2ece6813,// 226 PAY 223 

    0x760bf5b3,// 227 PAY 224 

    0x13524f79,// 228 PAY 225 

    0xd3f55cde,// 229 PAY 226 

    0x1880a5f6,// 230 PAY 227 

    0xe726b888,// 231 PAY 228 

    0x7b89b98e,// 232 PAY 229 

    0xe2c0f21a,// 233 PAY 230 

    0x4b459596,// 234 PAY 231 

    0x530197c6,// 235 PAY 232 

    0x6aba7751,// 236 PAY 233 

    0xfd56a64e,// 237 PAY 234 

    0x3856c500,// 238 PAY 235 

    0x5a5068a5,// 239 PAY 236 

    0xb2a02be3,// 240 PAY 237 

    0xd306e3aa,// 241 PAY 238 

    0x75df5f10,// 242 PAY 239 

    0x31382461,// 243 PAY 240 

    0x49e19395,// 244 PAY 241 

    0x6634ac2e,// 245 PAY 242 

    0x54f42ee6,// 246 PAY 243 

    0x2eb8c57e,// 247 PAY 244 

    0x182f69af,// 248 PAY 245 

    0xcf526c9f,// 249 PAY 246 

    0xcdcf039b,// 250 PAY 247 

    0xa0a6bbcf,// 251 PAY 248 

    0xfe9fa81f,// 252 PAY 249 

    0xaffa2422,// 253 PAY 250 

    0x2679c983,// 254 PAY 251 

    0xe6ea62d0,// 255 PAY 252 

    0x06fe62fb,// 256 PAY 253 

    0x2c0a7019,// 257 PAY 254 

    0x0ae10b80,// 258 PAY 255 

    0xdec88d22,// 259 PAY 256 

    0x1649b713,// 260 PAY 257 

    0x995c78e0,// 261 PAY 258 

    0x3a827736,// 262 PAY 259 

    0x951ac0f5,// 263 PAY 260 

    0x6d888275,// 264 PAY 261 

    0x75a8214b,// 265 PAY 262 

    0x80722957,// 266 PAY 263 

    0x3d5822b7,// 267 PAY 264 

    0xc428ae27,// 268 PAY 265 

    0xfd088b46,// 269 PAY 266 

    0xf9cf4537,// 270 PAY 267 

    0x5ae14d91,// 271 PAY 268 

    0x7a8550a9,// 272 PAY 269 

    0xdfd53289,// 273 PAY 270 

    0x700a4f4b,// 274 PAY 271 

    0x9a92ebdc,// 275 PAY 272 

    0x990bd140,// 276 PAY 273 

    0xd0fa294c,// 277 PAY 274 

    0xda16a05e,// 278 PAY 275 

    0xfa6dc481,// 279 PAY 276 

    0x3a35cb97,// 280 PAY 277 

    0xb144e733,// 281 PAY 278 

    0xf6e87fbf,// 282 PAY 279 

    0xaac2b11d,// 283 PAY 280 

    0xd5c8dfdc,// 284 PAY 281 

    0x7d000885,// 285 PAY 282 

    0xdf57e088,// 286 PAY 283 

    0x593ab70b,// 287 PAY 284 

    0xb527dbc3,// 288 PAY 285 

    0x99eb513a,// 289 PAY 286 

    0xd11a7cb9,// 290 PAY 287 

    0x525f8d57,// 291 PAY 288 

    0xc0f14704,// 292 PAY 289 

    0x5b10c77a,// 293 PAY 290 

    0x57a15ac7,// 294 PAY 291 

    0x9f67e20d,// 295 PAY 292 

    0x651604a2,// 296 PAY 293 

    0xc826e95a,// 297 PAY 294 

    0x6100d58f,// 298 PAY 295 

    0x36aa13d5,// 299 PAY 296 

    0x77f0444d,// 300 PAY 297 

    0x2a940e15,// 301 PAY 298 

    0x98768dde,// 302 PAY 299 

    0xaaa92fe3,// 303 PAY 300 

    0x6fc73a73,// 304 PAY 301 

    0x24568889,// 305 PAY 302 

    0xba5c42f0,// 306 PAY 303 

    0x48f22140,// 307 PAY 304 

    0x220a9200,// 308 PAY 305 

    0x826833f7,// 309 PAY 306 

    0xc5cf8969,// 310 PAY 307 

    0x62a35315,// 311 PAY 308 

    0xddc1d740,// 312 PAY 309 

    0xe12e23fd,// 313 PAY 310 

    0x85f5dbb8,// 314 PAY 311 

    0xa469303c,// 315 PAY 312 

    0xaca49367,// 316 PAY 313 

    0x9216ebe7,// 317 PAY 314 

    0xa94c6eb8,// 318 PAY 315 

    0x4fb0bd7f,// 319 PAY 316 

    0xc9f7987e,// 320 PAY 317 

    0xb5d93723,// 321 PAY 318 

    0xbc04803e,// 322 PAY 319 

    0xf03426a8,// 323 PAY 320 

    0x41540e89,// 324 PAY 321 

    0x1c50b9dc,// 325 PAY 322 

    0xe1f26dd7,// 326 PAY 323 

    0xfe4137e6,// 327 PAY 324 

    0x80f42034,// 328 PAY 325 

    0x63c54724,// 329 PAY 326 

    0xcb138dad,// 330 PAY 327 

    0xcceccc33,// 331 PAY 328 

    0x2e82df3e,// 332 PAY 329 

    0xe0ca8b7c,// 333 PAY 330 

    0xe7877bc7,// 334 PAY 331 

    0x84d6bf48,// 335 PAY 332 

    0x740d7449,// 336 PAY 333 

    0xb81ddab0,// 337 PAY 334 

    0x771f1695,// 338 PAY 335 

    0xda6bacd4,// 339 PAY 336 

    0xb7575a48,// 340 PAY 337 

    0xebc9501d,// 341 PAY 338 

    0xfdde8dea,// 342 PAY 339 

    0x918a0ba1,// 343 PAY 340 

    0x1f9e8b58,// 344 PAY 341 

    0xeaad3fb7,// 345 PAY 342 

    0x4e7adf36,// 346 PAY 343 

    0xeb4172da,// 347 PAY 344 

    0x5c50f2c1,// 348 PAY 345 

    0xc5000000,// 349 PAY 346 

/// STA is 1 words. 

/// STA num_pkts       : 62 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0361e83e // 350 STA   1 

};

//


uint16 rx_des3_md5_pkt_len[] = {
    sizeof(rx_des3_md5_pkt0_tmpl),
    sizeof(rx_des3_md5_pkt1_tmpl),
    sizeof(rx_des3_md5_pkt2_tmpl),
    sizeof(rx_des3_md5_pkt3_tmpl),
    sizeof(rx_des3_md5_pkt4_tmpl),
    sizeof(rx_des3_md5_pkt5_tmpl),
    sizeof(rx_des3_md5_pkt6_tmpl),
    sizeof(rx_des3_md5_pkt7_tmpl),
    sizeof(rx_des3_md5_pkt8_tmpl),
    sizeof(rx_des3_md5_pkt9_tmpl),
    sizeof(rx_des3_md5_pkt10_tmpl),
    sizeof(rx_des3_md5_pkt11_tmpl),
    sizeof(rx_des3_md5_pkt12_tmpl),
    sizeof(rx_des3_md5_pkt13_tmpl),
    sizeof(rx_des3_md5_pkt14_tmpl),
    sizeof(rx_des3_md5_pkt15_tmpl),
    sizeof(rx_des3_md5_pkt16_tmpl),
    sizeof(rx_des3_md5_pkt17_tmpl),
    sizeof(rx_des3_md5_pkt18_tmpl),
    sizeof(rx_des3_md5_pkt19_tmpl),
    sizeof(rx_des3_md5_pkt20_tmpl),
    sizeof(rx_des3_md5_pkt21_tmpl),
    sizeof(rx_des3_md5_pkt22_tmpl),
    sizeof(rx_des3_md5_pkt23_tmpl),
    sizeof(rx_des3_md5_pkt24_tmpl),
    sizeof(rx_des3_md5_pkt25_tmpl),
    sizeof(rx_des3_md5_pkt26_tmpl),
    sizeof(rx_des3_md5_pkt27_tmpl),
    sizeof(rx_des3_md5_pkt28_tmpl),
    sizeof(rx_des3_md5_pkt29_tmpl),
    sizeof(rx_des3_md5_pkt30_tmpl),
    sizeof(rx_des3_md5_pkt31_tmpl),
    sizeof(rx_des3_md5_pkt32_tmpl),
    sizeof(rx_des3_md5_pkt33_tmpl),
    sizeof(rx_des3_md5_pkt34_tmpl),
    sizeof(rx_des3_md5_pkt35_tmpl),
    sizeof(rx_des3_md5_pkt36_tmpl),
    sizeof(rx_des3_md5_pkt37_tmpl),
    sizeof(rx_des3_md5_pkt38_tmpl),
    sizeof(rx_des3_md5_pkt39_tmpl),
    sizeof(rx_des3_md5_pkt40_tmpl),
    sizeof(rx_des3_md5_pkt41_tmpl),
    sizeof(rx_des3_md5_pkt42_tmpl),
    sizeof(rx_des3_md5_pkt43_tmpl),
    sizeof(rx_des3_md5_pkt44_tmpl),
    sizeof(rx_des3_md5_pkt45_tmpl),
    sizeof(rx_des3_md5_pkt46_tmpl),
    sizeof(rx_des3_md5_pkt47_tmpl),
    sizeof(rx_des3_md5_pkt48_tmpl),
    sizeof(rx_des3_md5_pkt49_tmpl),
    sizeof(rx_des3_md5_pkt50_tmpl),
    sizeof(rx_des3_md5_pkt51_tmpl),
    sizeof(rx_des3_md5_pkt52_tmpl),
    sizeof(rx_des3_md5_pkt53_tmpl),
    sizeof(rx_des3_md5_pkt54_tmpl),
    sizeof(rx_des3_md5_pkt55_tmpl),
    sizeof(rx_des3_md5_pkt56_tmpl),
    sizeof(rx_des3_md5_pkt57_tmpl),
    sizeof(rx_des3_md5_pkt58_tmpl),
    sizeof(rx_des3_md5_pkt59_tmpl),
    sizeof(rx_des3_md5_pkt60_tmpl),
    sizeof(rx_des3_md5_pkt61_tmpl),
    sizeof(rx_des3_md5_pkt62_tmpl),
    sizeof(rx_des3_md5_pkt63_tmpl),
    sizeof(rx_des3_md5_pkt64_tmpl),
    sizeof(rx_des3_md5_pkt65_tmpl),
    sizeof(rx_des3_md5_pkt66_tmpl),
    sizeof(rx_des3_md5_pkt67_tmpl),
    sizeof(rx_des3_md5_pkt68_tmpl),
    sizeof(rx_des3_md5_pkt69_tmpl),
    sizeof(rx_des3_md5_pkt70_tmpl),
    sizeof(rx_des3_md5_pkt71_tmpl),
    sizeof(rx_des3_md5_pkt72_tmpl),
    sizeof(rx_des3_md5_pkt73_tmpl),
    sizeof(rx_des3_md5_pkt74_tmpl),
    sizeof(rx_des3_md5_pkt75_tmpl),
    sizeof(rx_des3_md5_pkt76_tmpl),
    sizeof(rx_des3_md5_pkt77_tmpl),
    sizeof(rx_des3_md5_pkt78_tmpl),
    sizeof(rx_des3_md5_pkt79_tmpl),
    sizeof(rx_des3_md5_pkt80_tmpl),
    sizeof(rx_des3_md5_pkt81_tmpl),
    sizeof(rx_des3_md5_pkt82_tmpl),
    sizeof(rx_des3_md5_pkt83_tmpl),
    sizeof(rx_des3_md5_pkt84_tmpl),
    sizeof(rx_des3_md5_pkt85_tmpl),
    sizeof(rx_des3_md5_pkt86_tmpl),
    sizeof(rx_des3_md5_pkt87_tmpl),
    sizeof(rx_des3_md5_pkt88_tmpl),
    sizeof(rx_des3_md5_pkt89_tmpl),
    sizeof(rx_des3_md5_pkt90_tmpl),
    sizeof(rx_des3_md5_pkt91_tmpl),
    sizeof(rx_des3_md5_pkt92_tmpl),
    sizeof(rx_des3_md5_pkt93_tmpl),
    sizeof(rx_des3_md5_pkt94_tmpl),
    sizeof(rx_des3_md5_pkt95_tmpl),
    sizeof(rx_des3_md5_pkt96_tmpl),
    sizeof(rx_des3_md5_pkt97_tmpl),
    sizeof(rx_des3_md5_pkt98_tmpl),
    sizeof(rx_des3_md5_pkt99_tmpl)
};

unsigned char *rx_des3_md5_test_pkts[] = {
    (unsigned char *)&rx_des3_md5_pkt0_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt1_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt2_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt3_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt4_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt5_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt6_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt7_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt8_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt9_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt10_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt11_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt12_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt13_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt14_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt15_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt16_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt17_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt18_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt19_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt20_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt21_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt22_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt23_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt24_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt25_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt26_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt27_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt28_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt29_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt30_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt31_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt32_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt33_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt34_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt35_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt36_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt37_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt38_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt39_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt40_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt41_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt42_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt43_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt44_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt45_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt46_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt47_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt48_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt49_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt50_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt51_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt52_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt53_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt54_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt55_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt56_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt57_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt58_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt59_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt60_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt61_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt62_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt63_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt64_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt65_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt66_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt67_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt68_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt69_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt70_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt71_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt72_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt73_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt74_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt75_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt76_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt77_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt78_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt79_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt80_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt81_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt82_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt83_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt84_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt85_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt86_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt87_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt88_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt89_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt90_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt91_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt92_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt93_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt94_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt95_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt96_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt97_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt98_tmpl[0],
    (unsigned char *)&rx_des3_md5_pkt99_tmpl[0]
};
#endif /* CONFIG_BCM_SPU_TEST */

#endif /* __SPUDRV_RX_DES3_MD5_DATA_H__*/
