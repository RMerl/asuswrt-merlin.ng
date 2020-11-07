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

#ifndef __SPUDRV_RX_AES_SHA1_DATA_H__
#define __SPUDRV_RX_AES_SHA1_DATA_H__

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

uint32 rx_aes_sha1_pkt0_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 409 words. 

/// BDA size     is 1630 (0x65e) 

/// BDA id       is 0xd8ea 

    0x065ed8ea,// 3 BDA   1 

/// PAY Generic Data size   : 1630 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x01da9c78,// 4 PAY   1 

    0x665a7a40,// 5 PAY   2 

    0x12a1f8e8,// 6 PAY   3 

    0x79490ec8,// 7 PAY   4 

    0x0f232a81,// 8 PAY   5 

    0x2ffcca31,// 9 PAY   6 

    0xea207e56,// 10 PAY   7 

    0x4ec321aa,// 11 PAY   8 

    0x13899779,// 12 PAY   9 

    0x14df1148,// 13 PAY  10 

    0x2c7dfdc8,// 14 PAY  11 

    0x6e4a8584,// 15 PAY  12 

    0x7e2857e8,// 16 PAY  13 

    0xfebf8aba,// 17 PAY  14 

    0x2b27b444,// 18 PAY  15 

    0xb1329713,// 19 PAY  16 

    0x15f597a1,// 20 PAY  17 

    0x94e3fd33,// 21 PAY  18 

    0x23ea7405,// 22 PAY  19 

    0xa53cd1d1,// 23 PAY  20 

    0x77e43e1d,// 24 PAY  21 

    0xec665679,// 25 PAY  22 

    0x231f3914,// 26 PAY  23 

    0x59af3aca,// 27 PAY  24 

    0x9ed55b0c,// 28 PAY  25 

    0x01f5d354,// 29 PAY  26 

    0xf879ea07,// 30 PAY  27 

    0x1807cc77,// 31 PAY  28 

    0x156d5bb4,// 32 PAY  29 

    0x9e7ef91c,// 33 PAY  30 

    0x865391bc,// 34 PAY  31 

    0xee3839e3,// 35 PAY  32 

    0x47e15aa8,// 36 PAY  33 

    0xe32dfd03,// 37 PAY  34 

    0xc6eced8a,// 38 PAY  35 

    0x6e91cd22,// 39 PAY  36 

    0x9eeb56e2,// 40 PAY  37 

    0xcebc4b06,// 41 PAY  38 

    0x5777376f,// 42 PAY  39 

    0x0e0ad5ea,// 43 PAY  40 

    0x60e5975d,// 44 PAY  41 

    0xb0cef425,// 45 PAY  42 

    0x669c80ed,// 46 PAY  43 

    0x9fc22248,// 47 PAY  44 

    0x89146de6,// 48 PAY  45 

    0xeb5ceaf8,// 49 PAY  46 

    0x873af488,// 50 PAY  47 

    0xa0627799,// 51 PAY  48 

    0x179638d3,// 52 PAY  49 

    0xc53b0aac,// 53 PAY  50 

    0x70495ef8,// 54 PAY  51 

    0x4a330f3d,// 55 PAY  52 

    0x92c0f68c,// 56 PAY  53 

    0x1725c769,// 57 PAY  54 

    0x8de8a1bc,// 58 PAY  55 

    0x7611210f,// 59 PAY  56 

    0x6f396734,// 60 PAY  57 

    0x77e130f1,// 61 PAY  58 

    0xbc44f4c9,// 62 PAY  59 

    0x148a59f2,// 63 PAY  60 

    0x617826df,// 64 PAY  61 

    0xa38cb68a,// 65 PAY  62 

    0x3c8b1e58,// 66 PAY  63 

    0x0540f8a2,// 67 PAY  64 

    0x96659e26,// 68 PAY  65 

    0x4b1ff483,// 69 PAY  66 

    0x0720cb29,// 70 PAY  67 

    0x88b1a5cd,// 71 PAY  68 

    0x6391fcec,// 72 PAY  69 

    0x900fa899,// 73 PAY  70 

    0x678e0e82,// 74 PAY  71 

    0x3b91e20a,// 75 PAY  72 

    0x2528e1b8,// 76 PAY  73 

    0x6516577c,// 77 PAY  74 

    0x997a3045,// 78 PAY  75 

    0x9c4d41a3,// 79 PAY  76 

    0x9e583145,// 80 PAY  77 

    0x0b9209b6,// 81 PAY  78 

    0x01ebdd79,// 82 PAY  79 

    0x9b2cdce2,// 83 PAY  80 

    0xf4212da1,// 84 PAY  81 

    0x49fc4714,// 85 PAY  82 

    0x724c135e,// 86 PAY  83 

    0xc61ea2b7,// 87 PAY  84 

    0xcd6f59ed,// 88 PAY  85 

    0x36293453,// 89 PAY  86 

    0xef5147ed,// 90 PAY  87 

    0x91c8bb4a,// 91 PAY  88 

    0x89eccf8f,// 92 PAY  89 

    0xa46e4692,// 93 PAY  90 

    0xcc14222b,// 94 PAY  91 

    0x5f879adf,// 95 PAY  92 

    0x985b864a,// 96 PAY  93 

    0xcb677eb8,// 97 PAY  94 

    0x37d30225,// 98 PAY  95 

    0x1276808b,// 99 PAY  96 

    0xac75a71e,// 100 PAY  97 

    0xa216ec67,// 101 PAY  98 

    0xe49cbaeb,// 102 PAY  99 

    0x6e2c29cd,// 103 PAY 100 

    0xe50a6a2d,// 104 PAY 101 

    0x8617457a,// 105 PAY 102 

    0x58ae35c1,// 106 PAY 103 

    0x79e27cfd,// 107 PAY 104 

    0xb204a802,// 108 PAY 105 

    0xe2eeb7f3,// 109 PAY 106 

    0x1597aa9d,// 110 PAY 107 

    0xa858f271,// 111 PAY 108 

    0xed27529d,// 112 PAY 109 

    0xebb5af5f,// 113 PAY 110 

    0xea2acec0,// 114 PAY 111 

    0x09df95f0,// 115 PAY 112 

    0xeefec536,// 116 PAY 113 

    0x4d5218ef,// 117 PAY 114 

    0x5760b9f7,// 118 PAY 115 

    0x44f4ff02,// 119 PAY 116 

    0x29764d44,// 120 PAY 117 

    0x497ef3bc,// 121 PAY 118 

    0xe84b434b,// 122 PAY 119 

    0x0e6c80e7,// 123 PAY 120 

    0x25f99c06,// 124 PAY 121 

    0x277ee95e,// 125 PAY 122 

    0xf0c8d1f6,// 126 PAY 123 

    0xa2891492,// 127 PAY 124 

    0xdf3cc9d2,// 128 PAY 125 

    0xdd86aad9,// 129 PAY 126 

    0x8f3c9b62,// 130 PAY 127 

    0x3d8115d7,// 131 PAY 128 

    0x71105e31,// 132 PAY 129 

    0x6b7c0d5a,// 133 PAY 130 

    0x723c9af8,// 134 PAY 131 

    0x09aaeeee,// 135 PAY 132 

    0x37de7cd7,// 136 PAY 133 

    0x860e4df5,// 137 PAY 134 

    0x99c19bfd,// 138 PAY 135 

    0xdd819363,// 139 PAY 136 

    0x213a7519,// 140 PAY 137 

    0xa8574091,// 141 PAY 138 

    0x2303da37,// 142 PAY 139 

    0xd10e60be,// 143 PAY 140 

    0x95e8ba99,// 144 PAY 141 

    0xfac630f0,// 145 PAY 142 

    0x62b1d096,// 146 PAY 143 

    0x2c99c7e0,// 147 PAY 144 

    0x046e495e,// 148 PAY 145 

    0x6c3d17b6,// 149 PAY 146 

    0x7cd48aaf,// 150 PAY 147 

    0x484202a3,// 151 PAY 148 

    0x38cc4098,// 152 PAY 149 

    0x390062d6,// 153 PAY 150 

    0x62f21221,// 154 PAY 151 

    0x2957055f,// 155 PAY 152 

    0x808ed1b0,// 156 PAY 153 

    0xb4866c33,// 157 PAY 154 

    0x502c7858,// 158 PAY 155 

    0x90b73543,// 159 PAY 156 

    0xc15269cf,// 160 PAY 157 

    0xe91f83b2,// 161 PAY 158 

    0xa7298558,// 162 PAY 159 

    0x7695bac1,// 163 PAY 160 

    0x613ab8e0,// 164 PAY 161 

    0x12ffc7c4,// 165 PAY 162 

    0x8ae52e92,// 166 PAY 163 

    0xc9dfd79d,// 167 PAY 164 

    0x1c2b5f30,// 168 PAY 165 

    0x34ba0d38,// 169 PAY 166 

    0xff5dc6ec,// 170 PAY 167 

    0x918a5d3d,// 171 PAY 168 

    0x7e5e790f,// 172 PAY 169 

    0x66aaadb2,// 173 PAY 170 

    0x39cf4d65,// 174 PAY 171 

    0x222c33f9,// 175 PAY 172 

    0x179ffa07,// 176 PAY 173 

    0x9727b0d3,// 177 PAY 174 

    0x93b27ac7,// 178 PAY 175 

    0xa6da5b86,// 179 PAY 176 

    0x99b138cf,// 180 PAY 177 

    0xed8ed7fe,// 181 PAY 178 

    0x8ca17355,// 182 PAY 179 

    0x2d304164,// 183 PAY 180 

    0x4706543a,// 184 PAY 181 

    0x0304e6db,// 185 PAY 182 

    0x3f9cb331,// 186 PAY 183 

    0xc3f22f79,// 187 PAY 184 

    0x82e92eda,// 188 PAY 185 

    0xe2da842f,// 189 PAY 186 

    0xdd2eb074,// 190 PAY 187 

    0x9ba6fbdc,// 191 PAY 188 

    0x7214e6a9,// 192 PAY 189 

    0x43233777,// 193 PAY 190 

    0x91f60016,// 194 PAY 191 

    0x46db5d4c,// 195 PAY 192 

    0xa7e08506,// 196 PAY 193 

    0x74aa7d29,// 197 PAY 194 

    0x924ee0b5,// 198 PAY 195 

    0x0a76f712,// 199 PAY 196 

    0xf32c0429,// 200 PAY 197 

    0x01780e21,// 201 PAY 198 

    0xf0a9d78b,// 202 PAY 199 

    0x36b44dab,// 203 PAY 200 

    0x84147fc8,// 204 PAY 201 

    0xdb7f30e5,// 205 PAY 202 

    0xea70dbb1,// 206 PAY 203 

    0xeb8d037b,// 207 PAY 204 

    0x5ac30362,// 208 PAY 205 

    0x855011ac,// 209 PAY 206 

    0x7097b49f,// 210 PAY 207 

    0x8aacbd1b,// 211 PAY 208 

    0x250a5e39,// 212 PAY 209 

    0xdee7d4e7,// 213 PAY 210 

    0x14325cdf,// 214 PAY 211 

    0xfe2dfe54,// 215 PAY 212 

    0x9c8da7f0,// 216 PAY 213 

    0x49e54bb1,// 217 PAY 214 

    0x9e112443,// 218 PAY 215 

    0x40836990,// 219 PAY 216 

    0x478f2080,// 220 PAY 217 

    0xc9d880ea,// 221 PAY 218 

    0xea2d44c2,// 222 PAY 219 

    0xd0950d28,// 223 PAY 220 

    0x52e5db73,// 224 PAY 221 

    0x1d6b7aba,// 225 PAY 222 

    0x32b5d070,// 226 PAY 223 

    0x4802b83d,// 227 PAY 224 

    0xec0e3832,// 228 PAY 225 

    0x50893d8a,// 229 PAY 226 

    0xef595ff3,// 230 PAY 227 

    0x4c4a8635,// 231 PAY 228 

    0x2a350fff,// 232 PAY 229 

    0x2dc061cf,// 233 PAY 230 

    0x161cc825,// 234 PAY 231 

    0x202fd253,// 235 PAY 232 

    0x13a06bf3,// 236 PAY 233 

    0xe08a9e7c,// 237 PAY 234 

    0xee61bf5d,// 238 PAY 235 

    0x48b5d489,// 239 PAY 236 

    0xafb39e1e,// 240 PAY 237 

    0x3ee1e750,// 241 PAY 238 

    0xefcdd2f3,// 242 PAY 239 

    0x4c2b2559,// 243 PAY 240 

    0xbd12689c,// 244 PAY 241 

    0xb133907a,// 245 PAY 242 

    0x2ed8fa4a,// 246 PAY 243 

    0x4c32d31a,// 247 PAY 244 

    0x5eacf37c,// 248 PAY 245 

    0x3ac8c7fe,// 249 PAY 246 

    0xd5e5fe1f,// 250 PAY 247 

    0x02de59f3,// 251 PAY 248 

    0xc2be1446,// 252 PAY 249 

    0x62a03e3e,// 253 PAY 250 

    0x5752b5d7,// 254 PAY 251 

    0xe776d48e,// 255 PAY 252 

    0x66101e00,// 256 PAY 253 

    0x6bf03436,// 257 PAY 254 

    0xe6153322,// 258 PAY 255 

    0xbc3fec5c,// 259 PAY 256 

    0x27acd3ca,// 260 PAY 257 

    0xdc5c10fd,// 261 PAY 258 

    0xd6f2f64b,// 262 PAY 259 

    0xaf7b945c,// 263 PAY 260 

    0xcb75a728,// 264 PAY 261 

    0x498b09cf,// 265 PAY 262 

    0xeb6091d3,// 266 PAY 263 

    0x01590053,// 267 PAY 264 

    0x2f243054,// 268 PAY 265 

    0x5e58f7a7,// 269 PAY 266 

    0x80d56156,// 270 PAY 267 

    0x0cda6e64,// 271 PAY 268 

    0xa08e6b08,// 272 PAY 269 

    0xeeb9b3d4,// 273 PAY 270 

    0x12aee6b9,// 274 PAY 271 

    0x8d6995c5,// 275 PAY 272 

    0x427b4a0b,// 276 PAY 273 

    0x8e7c6181,// 277 PAY 274 

    0xa19e15cd,// 278 PAY 275 

    0x8737ca7d,// 279 PAY 276 

    0xd3b29dd0,// 280 PAY 277 

    0x1a8d764d,// 281 PAY 278 

    0xe7d99b5d,// 282 PAY 279 

    0x25bcd779,// 283 PAY 280 

    0x2ef0987e,// 284 PAY 281 

    0xa2f522c0,// 285 PAY 282 

    0x58ed06c6,// 286 PAY 283 

    0x7f5b7823,// 287 PAY 284 

    0xc7d5e20f,// 288 PAY 285 

    0x73bc6a1f,// 289 PAY 286 

    0xc1624ca1,// 290 PAY 287 

    0xbd14b566,// 291 PAY 288 

    0xdd08f45b,// 292 PAY 289 

    0x09563f3a,// 293 PAY 290 

    0x7ddded00,// 294 PAY 291 

    0xf6de7768,// 295 PAY 292 

    0x2b741984,// 296 PAY 293 

    0x83bc76a0,// 297 PAY 294 

    0x9020481c,// 298 PAY 295 

    0xb7be6853,// 299 PAY 296 

    0x3b8f9051,// 300 PAY 297 

    0x7f89957f,// 301 PAY 298 

    0xb36c5cc6,// 302 PAY 299 

    0x17abdf59,// 303 PAY 300 

    0x9380073a,// 304 PAY 301 

    0xf43e7c7c,// 305 PAY 302 

    0xebc9bca2,// 306 PAY 303 

    0xaec58776,// 307 PAY 304 

    0xe41f7da5,// 308 PAY 305 

    0xa9d092df,// 309 PAY 306 

    0x91dd007a,// 310 PAY 307 

    0x13b63592,// 311 PAY 308 

    0xddb00260,// 312 PAY 309 

    0x5f21fb04,// 313 PAY 310 

    0xb77c9bd6,// 314 PAY 311 

    0x89cd264e,// 315 PAY 312 

    0xe41a2708,// 316 PAY 313 

    0x3e58c5f9,// 317 PAY 314 

    0x6b0632d8,// 318 PAY 315 

    0x9ac72d54,// 319 PAY 316 

    0x5d3eedbe,// 320 PAY 317 

    0x1e23ef90,// 321 PAY 318 

    0x38685cee,// 322 PAY 319 

    0xb6e7a3f1,// 323 PAY 320 

    0x420dc260,// 324 PAY 321 

    0x146676f0,// 325 PAY 322 

    0x4677f295,// 326 PAY 323 

    0x635a0cbf,// 327 PAY 324 

    0x1df5cd00,// 328 PAY 325 

    0x7cc72a5e,// 329 PAY 326 

    0x6afc9c85,// 330 PAY 327 

    0xd5269420,// 331 PAY 328 

    0x87fb3b52,// 332 PAY 329 

    0x8656fa3d,// 333 PAY 330 

    0xf1b4f0fe,// 334 PAY 331 

    0xf845c876,// 335 PAY 332 

    0xbbe6bb88,// 336 PAY 333 

    0x079455c8,// 337 PAY 334 

    0xa94e2379,// 338 PAY 335 

    0x778e84ed,// 339 PAY 336 

    0x67d5e6e7,// 340 PAY 337 

    0xa1b6c4dc,// 341 PAY 338 

    0x8b4f790b,// 342 PAY 339 

    0xd60318e9,// 343 PAY 340 

    0x1ed4988d,// 344 PAY 341 

    0x2658be98,// 345 PAY 342 

    0x90fb3424,// 346 PAY 343 

    0x80f6139b,// 347 PAY 344 

    0xd570c21c,// 348 PAY 345 

    0x11075456,// 349 PAY 346 

    0x1ccf85b9,// 350 PAY 347 

    0xd6b2dbd0,// 351 PAY 348 

    0x636e3f2e,// 352 PAY 349 

    0x109999e5,// 353 PAY 350 

    0xa7c5cd77,// 354 PAY 351 

    0x367fa4c5,// 355 PAY 352 

    0x18610de9,// 356 PAY 353 

    0xa5156386,// 357 PAY 354 

    0x9bc9cb03,// 358 PAY 355 

    0xa74c0eb7,// 359 PAY 356 

    0x5dbf7b61,// 360 PAY 357 

    0x14eeb6e5,// 361 PAY 358 

    0x5220e71b,// 362 PAY 359 

    0xc046928f,// 363 PAY 360 

    0x0b8eec14,// 364 PAY 361 

    0x4d43656b,// 365 PAY 362 

    0x822bf32e,// 366 PAY 363 

    0xab9ffee4,// 367 PAY 364 

    0x22884cd9,// 368 PAY 365 

    0xf1175dc2,// 369 PAY 366 

    0xaa033508,// 370 PAY 367 

    0xa2e3adc0,// 371 PAY 368 

    0xf4eae98e,// 372 PAY 369 

    0xcedf0b6d,// 373 PAY 370 

    0x84f4ab70,// 374 PAY 371 

    0x759e319e,// 375 PAY 372 

    0x048a85d6,// 376 PAY 373 

    0x9d740559,// 377 PAY 374 

    0xb8bb10a2,// 378 PAY 375 

    0x345d2c8b,// 379 PAY 376 

    0xf45dbcf3,// 380 PAY 377 

    0xfe1b39ea,// 381 PAY 378 

    0x0aa150f1,// 382 PAY 379 

    0xef3a8237,// 383 PAY 380 

    0x139e94dd,// 384 PAY 381 

    0xf4adb0c4,// 385 PAY 382 

    0x43e5bf3d,// 386 PAY 383 

    0x16ad342d,// 387 PAY 384 

    0xc4020262,// 388 PAY 385 

    0x0a38dfc5,// 389 PAY 386 

    0x3331fdc1,// 390 PAY 387 

    0xc53fbeea,// 391 PAY 388 

    0x00449d3b,// 392 PAY 389 

    0x5cad81ee,// 393 PAY 390 

    0xa95967e2,// 394 PAY 391 

    0xca7af5cc,// 395 PAY 392 

    0xafe4425c,// 396 PAY 393 

    0x789be653,// 397 PAY 394 

    0xb7f967c4,// 398 PAY 395 

    0x87f5a618,// 399 PAY 396 

    0xccd19fa0,// 400 PAY 397 

    0xde4faa12,// 401 PAY 398 

    0xf69e0fbe,// 402 PAY 399 

    0x3ad6acaf,// 403 PAY 400 

    0x72d0895e,// 404 PAY 401 

    0x0680000d,// 405 PAY 402 

    0x3885f71e,// 406 PAY 403 

    0x8de0a0cb,// 407 PAY 404 

    0x7252ae8c,// 408 PAY 405 

    0x41cc4b24,// 409 PAY 406 

    0x7c014ce2,// 410 PAY 407 

    0x8e7e0000,// 411 PAY 408 

/// HASH is  16 bytes 

    0x7ddded00,// 412 HSH   1 

    0xf6de7768,// 413 HSH   2 

    0x2b741984,// 414 HSH   3 

    0x83bc76a0,// 415 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 99 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x93 

    0x02609363 // 416 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt1_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 460 words. 

/// BDA size     is 1833 (0x729) 

/// BDA id       is 0x3d82 

    0x07293d82,// 3 BDA   1 

/// PAY Generic Data size   : 1833 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xb0c9a8c4,// 4 PAY   1 

    0x51089638,// 5 PAY   2 

    0x46eb14dd,// 6 PAY   3 

    0x13913e05,// 7 PAY   4 

    0x8496e494,// 8 PAY   5 

    0x9e46e71c,// 9 PAY   6 

    0x46109785,// 10 PAY   7 

    0x64b071c4,// 11 PAY   8 

    0xd072598d,// 12 PAY   9 

    0x74512c4d,// 13 PAY  10 

    0x7beba878,// 14 PAY  11 

    0x8caf29a0,// 15 PAY  12 

    0x3f108a9b,// 16 PAY  13 

    0xe6c79b8e,// 17 PAY  14 

    0x1585d4bb,// 18 PAY  15 

    0x5a96ee3b,// 19 PAY  16 

    0xc58e77f3,// 20 PAY  17 

    0xc5a55673,// 21 PAY  18 

    0x8ab0979a,// 22 PAY  19 

    0xee5ab97c,// 23 PAY  20 

    0x60891b85,// 24 PAY  21 

    0x3d14bd84,// 25 PAY  22 

    0x4109264a,// 26 PAY  23 

    0x05f61300,// 27 PAY  24 

    0x93857c55,// 28 PAY  25 

    0xdd44bd73,// 29 PAY  26 

    0x1af6f43e,// 30 PAY  27 

    0x93f95033,// 31 PAY  28 

    0x03df1874,// 32 PAY  29 

    0xce3f383f,// 33 PAY  30 

    0x4ac8cc76,// 34 PAY  31 

    0xc48822e9,// 35 PAY  32 

    0x368c75f2,// 36 PAY  33 

    0x20755d9b,// 37 PAY  34 

    0xa284e31e,// 38 PAY  35 

    0x8febb025,// 39 PAY  36 

    0x9a6c1d8e,// 40 PAY  37 

    0xcda5b537,// 41 PAY  38 

    0x3dea5332,// 42 PAY  39 

    0xe9ae1c56,// 43 PAY  40 

    0xac52a8cf,// 44 PAY  41 

    0x311966b0,// 45 PAY  42 

    0xaad3bae5,// 46 PAY  43 

    0x452a0bb5,// 47 PAY  44 

    0x8c2d56cc,// 48 PAY  45 

    0x303e2ce6,// 49 PAY  46 

    0xa6c2b162,// 50 PAY  47 

    0xa135efa0,// 51 PAY  48 

    0xed66c352,// 52 PAY  49 

    0x2e3e05e2,// 53 PAY  50 

    0x5ce3cb2f,// 54 PAY  51 

    0x748bd237,// 55 PAY  52 

    0xae06909d,// 56 PAY  53 

    0x1255ef8b,// 57 PAY  54 

    0xeff62e6a,// 58 PAY  55 

    0xd892cd1e,// 59 PAY  56 

    0x459cdf74,// 60 PAY  57 

    0xe90545d6,// 61 PAY  58 

    0xc4ebd94e,// 62 PAY  59 

    0xe7cf9c67,// 63 PAY  60 

    0xcc94a7ad,// 64 PAY  61 

    0x4014b31e,// 65 PAY  62 

    0x419fea8d,// 66 PAY  63 

    0x01eebf46,// 67 PAY  64 

    0x615a4d4d,// 68 PAY  65 

    0x43d71755,// 69 PAY  66 

    0xe1a86bbd,// 70 PAY  67 

    0x727c0528,// 71 PAY  68 

    0x997dd366,// 72 PAY  69 

    0x3865eb02,// 73 PAY  70 

    0x9feb9845,// 74 PAY  71 

    0x51aa6e19,// 75 PAY  72 

    0x4c2063d5,// 76 PAY  73 

    0xec8ba105,// 77 PAY  74 

    0x0faccdfa,// 78 PAY  75 

    0xd5fde89d,// 79 PAY  76 

    0x195c0933,// 80 PAY  77 

    0x3d1bd543,// 81 PAY  78 

    0x4c678377,// 82 PAY  79 

    0x359b7b3a,// 83 PAY  80 

    0x7743c8ea,// 84 PAY  81 

    0xb1a016fa,// 85 PAY  82 

    0xee1e839e,// 86 PAY  83 

    0xd7f0de6c,// 87 PAY  84 

    0xecb0790e,// 88 PAY  85 

    0xda7bab68,// 89 PAY  86 

    0x64bf6a1b,// 90 PAY  87 

    0x969e3846,// 91 PAY  88 

    0x2341ba9e,// 92 PAY  89 

    0x345193da,// 93 PAY  90 

    0xeb34b7a9,// 94 PAY  91 

    0x14fbf6fc,// 95 PAY  92 

    0x457b2ddc,// 96 PAY  93 

    0xaec82a70,// 97 PAY  94 

    0x4908992e,// 98 PAY  95 

    0x0b255b6e,// 99 PAY  96 

    0xc5bf8dd0,// 100 PAY  97 

    0x3697f4c2,// 101 PAY  98 

    0xe9398960,// 102 PAY  99 

    0x92ae2699,// 103 PAY 100 

    0x84f0fc2f,// 104 PAY 101 

    0xa00e71d1,// 105 PAY 102 

    0xe9c83d1d,// 106 PAY 103 

    0x7a8d9f56,// 107 PAY 104 

    0x1d63d322,// 108 PAY 105 

    0xc8b6046d,// 109 PAY 106 

    0xf476772e,// 110 PAY 107 

    0x50dd9bdd,// 111 PAY 108 

    0xa5ff8b64,// 112 PAY 109 

    0x9a68a5c9,// 113 PAY 110 

    0x7dca0b5a,// 114 PAY 111 

    0xff7f839f,// 115 PAY 112 

    0x26976a30,// 116 PAY 113 

    0x76dec78f,// 117 PAY 114 

    0xef599a96,// 118 PAY 115 

    0x8dfdb1da,// 119 PAY 116 

    0x9aaf9339,// 120 PAY 117 

    0x996dfc7b,// 121 PAY 118 

    0x0c5cc13f,// 122 PAY 119 

    0x32532b66,// 123 PAY 120 

    0x87c8e2d1,// 124 PAY 121 

    0x40f34cb5,// 125 PAY 122 

    0x75ed9cd5,// 126 PAY 123 

    0xa8c884c9,// 127 PAY 124 

    0x09a88a68,// 128 PAY 125 

    0xa3d654a9,// 129 PAY 126 

    0x894a52d7,// 130 PAY 127 

    0x66a30bb6,// 131 PAY 128 

    0xf28bf251,// 132 PAY 129 

    0xb9676090,// 133 PAY 130 

    0xe7c55cad,// 134 PAY 131 

    0x4231192c,// 135 PAY 132 

    0xd9e26bb6,// 136 PAY 133 

    0x850d7661,// 137 PAY 134 

    0xd620c16e,// 138 PAY 135 

    0x7b4edc57,// 139 PAY 136 

    0x3441724d,// 140 PAY 137 

    0x0478286b,// 141 PAY 138 

    0xc098e7ec,// 142 PAY 139 

    0xd52ab664,// 143 PAY 140 

    0x442b51eb,// 144 PAY 141 

    0x5545c076,// 145 PAY 142 

    0x99a726dc,// 146 PAY 143 

    0x81ede5aa,// 147 PAY 144 

    0xa7cb0bfe,// 148 PAY 145 

    0xe2cf9b26,// 149 PAY 146 

    0xbc283747,// 150 PAY 147 

    0x1dfa1d36,// 151 PAY 148 

    0xd22c0bde,// 152 PAY 149 

    0xa50b30b0,// 153 PAY 150 

    0x166f2df8,// 154 PAY 151 

    0xfb0a8cbb,// 155 PAY 152 

    0x41fc45db,// 156 PAY 153 

    0x02f96663,// 157 PAY 154 

    0x96b7a7f7,// 158 PAY 155 

    0x7a0c6538,// 159 PAY 156 

    0xb77e0cac,// 160 PAY 157 

    0x7d2929e9,// 161 PAY 158 

    0x6f7f0949,// 162 PAY 159 

    0xebf04ead,// 163 PAY 160 

    0xe2022086,// 164 PAY 161 

    0xe890d85f,// 165 PAY 162 

    0x1c24374a,// 166 PAY 163 

    0x50356f59,// 167 PAY 164 

    0x81f554b8,// 168 PAY 165 

    0xa746c96c,// 169 PAY 166 

    0xfe935007,// 170 PAY 167 

    0xaad8ec7c,// 171 PAY 168 

    0xc02c5fe9,// 172 PAY 169 

    0x3c08674e,// 173 PAY 170 

    0x122d56c1,// 174 PAY 171 

    0x3b2c70e8,// 175 PAY 172 

    0x19a59109,// 176 PAY 173 

    0xd64f3881,// 177 PAY 174 

    0xe2585560,// 178 PAY 175 

    0x038e256a,// 179 PAY 176 

    0x1c7ef2d5,// 180 PAY 177 

    0x37189a21,// 181 PAY 178 

    0x15dda8eb,// 182 PAY 179 

    0x552fcf3c,// 183 PAY 180 

    0x6bec36d0,// 184 PAY 181 

    0x27b782f9,// 185 PAY 182 

    0x542110be,// 186 PAY 183 

    0xf9407a31,// 187 PAY 184 

    0x9c892bd1,// 188 PAY 185 

    0x1b148111,// 189 PAY 186 

    0xb76615fe,// 190 PAY 187 

    0xd8240a4d,// 191 PAY 188 

    0x5c6bd6f0,// 192 PAY 189 

    0x70c82c96,// 193 PAY 190 

    0x81f0c02d,// 194 PAY 191 

    0x24cf2b04,// 195 PAY 192 

    0x3c7c291b,// 196 PAY 193 

    0xca70902a,// 197 PAY 194 

    0xa34bdfba,// 198 PAY 195 

    0x8bba4f34,// 199 PAY 196 

    0x6625e141,// 200 PAY 197 

    0x9808ac83,// 201 PAY 198 

    0xb4501919,// 202 PAY 199 

    0xeb43d316,// 203 PAY 200 

    0xc146bc56,// 204 PAY 201 

    0x1b342b00,// 205 PAY 202 

    0x832e96fb,// 206 PAY 203 

    0x8a517571,// 207 PAY 204 

    0x7ef3dc18,// 208 PAY 205 

    0xd11be509,// 209 PAY 206 

    0x511229da,// 210 PAY 207 

    0xde1e7ca4,// 211 PAY 208 

    0x5d3d545c,// 212 PAY 209 

    0x1a59b1a1,// 213 PAY 210 

    0x689cb423,// 214 PAY 211 

    0x3bf2d663,// 215 PAY 212 

    0xedbf5bfb,// 216 PAY 213 

    0xa86602fe,// 217 PAY 214 

    0xa53956d4,// 218 PAY 215 

    0x623b8c8b,// 219 PAY 216 

    0x08b5a977,// 220 PAY 217 

    0x406f93e5,// 221 PAY 218 

    0x1d7edf10,// 222 PAY 219 

    0x50db2272,// 223 PAY 220 

    0x304c8be2,// 224 PAY 221 

    0xf110bbcb,// 225 PAY 222 

    0x0be01493,// 226 PAY 223 

    0xdaf9eab8,// 227 PAY 224 

    0x7bfc5f72,// 228 PAY 225 

    0x3e859b6b,// 229 PAY 226 

    0x728bbc28,// 230 PAY 227 

    0x0ade517c,// 231 PAY 228 

    0x9a52ec49,// 232 PAY 229 

    0xa25a9e85,// 233 PAY 230 

    0x0f33f12d,// 234 PAY 231 

    0xed73b5f9,// 235 PAY 232 

    0xabedd58b,// 236 PAY 233 

    0xf446cea4,// 237 PAY 234 

    0x2c1fc125,// 238 PAY 235 

    0xa849815b,// 239 PAY 236 

    0xbb94b743,// 240 PAY 237 

    0xf28df386,// 241 PAY 238 

    0x17cae277,// 242 PAY 239 

    0x9c042c02,// 243 PAY 240 

    0x7ce36542,// 244 PAY 241 

    0x9be1289c,// 245 PAY 242 

    0x9e49756f,// 246 PAY 243 

    0xdc889157,// 247 PAY 244 

    0x5a9b39e2,// 248 PAY 245 

    0x67e582d4,// 249 PAY 246 

    0x492ee9a3,// 250 PAY 247 

    0x0a1df41f,// 251 PAY 248 

    0x1ef35ebf,// 252 PAY 249 

    0xa5559e28,// 253 PAY 250 

    0xe16bb624,// 254 PAY 251 

    0xce19eeff,// 255 PAY 252 

    0xd2199007,// 256 PAY 253 

    0xcbd5ec42,// 257 PAY 254 

    0x9a3cd48f,// 258 PAY 255 

    0x6a2decdf,// 259 PAY 256 

    0x5b685b87,// 260 PAY 257 

    0xde9fe182,// 261 PAY 258 

    0xfe300200,// 262 PAY 259 

    0x1ff3a5c7,// 263 PAY 260 

    0x18b5df74,// 264 PAY 261 

    0x0d2bc88c,// 265 PAY 262 

    0x10ca4f8a,// 266 PAY 263 

    0x4cda41e1,// 267 PAY 264 

    0x6fb23aab,// 268 PAY 265 

    0x2881d8df,// 269 PAY 266 

    0x26d1ceba,// 270 PAY 267 

    0x6b99f067,// 271 PAY 268 

    0xd801764f,// 272 PAY 269 

    0xfd6f4808,// 273 PAY 270 

    0x844072e4,// 274 PAY 271 

    0x77d365d1,// 275 PAY 272 

    0xce729be7,// 276 PAY 273 

    0x594d13e5,// 277 PAY 274 

    0x88b38bdb,// 278 PAY 275 

    0x1d51936a,// 279 PAY 276 

    0x141d8ec9,// 280 PAY 277 

    0xf07adaf3,// 281 PAY 278 

    0x60ff9b42,// 282 PAY 279 

    0xb7a8733d,// 283 PAY 280 

    0xbe911580,// 284 PAY 281 

    0x388e3141,// 285 PAY 282 

    0xba948ccd,// 286 PAY 283 

    0xeeebcdf1,// 287 PAY 284 

    0x33eab765,// 288 PAY 285 

    0xd3afec08,// 289 PAY 286 

    0x904ca2e2,// 290 PAY 287 

    0x651d1f3c,// 291 PAY 288 

    0x0b330d7a,// 292 PAY 289 

    0x5bcc02ef,// 293 PAY 290 

    0xa7a2eb13,// 294 PAY 291 

    0xa5184a42,// 295 PAY 292 

    0x4677f3d7,// 296 PAY 293 

    0xff66be2e,// 297 PAY 294 

    0x1214d128,// 298 PAY 295 

    0x424067c4,// 299 PAY 296 

    0x0ad6864e,// 300 PAY 297 

    0xdbc7feea,// 301 PAY 298 

    0x40444c3a,// 302 PAY 299 

    0x306a5022,// 303 PAY 300 

    0x3c5b650f,// 304 PAY 301 

    0x53ca48b3,// 305 PAY 302 

    0x1c3a0cf9,// 306 PAY 303 

    0x56e4f7fc,// 307 PAY 304 

    0x915dbe26,// 308 PAY 305 

    0xb1806d38,// 309 PAY 306 

    0x7ce920be,// 310 PAY 307 

    0x2cbc0895,// 311 PAY 308 

    0xd4df4151,// 312 PAY 309 

    0xe3e93f04,// 313 PAY 310 

    0x7e7a32f3,// 314 PAY 311 

    0xa702db76,// 315 PAY 312 

    0x39290794,// 316 PAY 313 

    0x207bf706,// 317 PAY 314 

    0x8e5ab89b,// 318 PAY 315 

    0xb3aadde0,// 319 PAY 316 

    0xecfd99d9,// 320 PAY 317 

    0x83b8f2ee,// 321 PAY 318 

    0xfa870769,// 322 PAY 319 

    0xc467f130,// 323 PAY 320 

    0x9e9c3747,// 324 PAY 321 

    0xe201e479,// 325 PAY 322 

    0xb381517b,// 326 PAY 323 

    0x2515bb4a,// 327 PAY 324 

    0xb30d33f1,// 328 PAY 325 

    0x4b567897,// 329 PAY 326 

    0x30c04726,// 330 PAY 327 

    0xea37fd10,// 331 PAY 328 

    0xe482483a,// 332 PAY 329 

    0xec8ea946,// 333 PAY 330 

    0xf3cbc0bb,// 334 PAY 331 

    0x9d486944,// 335 PAY 332 

    0xf13eca97,// 336 PAY 333 

    0x9344f39d,// 337 PAY 334 

    0x08a26953,// 338 PAY 335 

    0xace54dca,// 339 PAY 336 

    0xe1f0b54a,// 340 PAY 337 

    0x73ead946,// 341 PAY 338 

    0xcbef54df,// 342 PAY 339 

    0x7262e206,// 343 PAY 340 

    0xe12fb20c,// 344 PAY 341 

    0x91d8acf8,// 345 PAY 342 

    0x677bb0bc,// 346 PAY 343 

    0x6f7b9382,// 347 PAY 344 

    0x900f33c7,// 348 PAY 345 

    0x99b62362,// 349 PAY 346 

    0x87d23ce3,// 350 PAY 347 

    0xbd8db423,// 351 PAY 348 

    0xd69f1267,// 352 PAY 349 

    0x336d46e2,// 353 PAY 350 

    0xf1dcabd0,// 354 PAY 351 

    0xa91b2590,// 355 PAY 352 

    0x939a71ca,// 356 PAY 353 

    0xd0ebad38,// 357 PAY 354 

    0x3cd9090a,// 358 PAY 355 

    0xb014ec76,// 359 PAY 356 

    0x7be2eeeb,// 360 PAY 357 

    0x0a9d5ac7,// 361 PAY 358 

    0x6ac521cd,// 362 PAY 359 

    0x55d32283,// 363 PAY 360 

    0x3a4b4b40,// 364 PAY 361 

    0xdebbf578,// 365 PAY 362 

    0xefe1fdd2,// 366 PAY 363 

    0x33947259,// 367 PAY 364 

    0x399f2f6d,// 368 PAY 365 

    0xa4bb034c,// 369 PAY 366 

    0x409e582d,// 370 PAY 367 

    0x54286633,// 371 PAY 368 

    0xf8c9c9dd,// 372 PAY 369 

    0x8e556ac1,// 373 PAY 370 

    0x68729908,// 374 PAY 371 

    0x4b0f7546,// 375 PAY 372 

    0xf737a93c,// 376 PAY 373 

    0x2f9bea43,// 377 PAY 374 

    0x49069435,// 378 PAY 375 

    0x8f0ddf6e,// 379 PAY 376 

    0x963802e9,// 380 PAY 377 

    0x7b7f20b1,// 381 PAY 378 

    0xdf262228,// 382 PAY 379 

    0x88f91c92,// 383 PAY 380 

    0xa4c08af0,// 384 PAY 381 

    0x98a7a726,// 385 PAY 382 

    0xf4264d81,// 386 PAY 383 

    0xcaa27897,// 387 PAY 384 

    0xb32c08aa,// 388 PAY 385 

    0x347fa621,// 389 PAY 386 

    0xc4a1087f,// 390 PAY 387 

    0xde893e93,// 391 PAY 388 

    0xe17e34dd,// 392 PAY 389 

    0x6a74d5a1,// 393 PAY 390 

    0xa9998724,// 394 PAY 391 

    0x3bf5399c,// 395 PAY 392 

    0xd7cb4061,// 396 PAY 393 

    0x0c1b42ab,// 397 PAY 394 

    0x9ffad3c6,// 398 PAY 395 

    0x39bca658,// 399 PAY 396 

    0x42d976fb,// 400 PAY 397 

    0x97a9d518,// 401 PAY 398 

    0x446439c6,// 402 PAY 399 

    0xbe8e2618,// 403 PAY 400 

    0x6106ae69,// 404 PAY 401 

    0x3102f08e,// 405 PAY 402 

    0x0d791de7,// 406 PAY 403 

    0x702d9617,// 407 PAY 404 

    0x0c04bc2f,// 408 PAY 405 

    0x9644e078,// 409 PAY 406 

    0x19f7d404,// 410 PAY 407 

    0x28b7286a,// 411 PAY 408 

    0xb3b4231f,// 412 PAY 409 

    0x5937da47,// 413 PAY 410 

    0xbf406468,// 414 PAY 411 

    0x61142897,// 415 PAY 412 

    0xd62795d7,// 416 PAY 413 

    0x8651a7ad,// 417 PAY 414 

    0x8244397e,// 418 PAY 415 

    0x0e443f2f,// 419 PAY 416 

    0x1bcc8731,// 420 PAY 417 

    0xfcad9c7d,// 421 PAY 418 

    0xcbb59a90,// 422 PAY 419 

    0xc55c7d40,// 423 PAY 420 

    0xe7238450,// 424 PAY 421 

    0x14661514,// 425 PAY 422 

    0x5f99d960,// 426 PAY 423 

    0x05b0ff6f,// 427 PAY 424 

    0xe2ac95d9,// 428 PAY 425 

    0x04567830,// 429 PAY 426 

    0x05c6c753,// 430 PAY 427 

    0x3985191d,// 431 PAY 428 

    0x543a1b40,// 432 PAY 429 

    0x98605601,// 433 PAY 430 

    0xa72e4ebd,// 434 PAY 431 

    0x7b2e82f0,// 435 PAY 432 

    0x044a8417,// 436 PAY 433 

    0x63212aa7,// 437 PAY 434 

    0x298fcc69,// 438 PAY 435 

    0x13d8efc7,// 439 PAY 436 

    0x2c5479af,// 440 PAY 437 

    0x1a9818bb,// 441 PAY 438 

    0x4b1f612b,// 442 PAY 439 

    0xb8bbfc31,// 443 PAY 440 

    0xb5dc3931,// 444 PAY 441 

    0x2b0ffc06,// 445 PAY 442 

    0xf89363f9,// 446 PAY 443 

    0x3f696293,// 447 PAY 444 

    0x1922eb05,// 448 PAY 445 

    0x7a7f9bf4,// 449 PAY 446 

    0x0680c4d9,// 450 PAY 447 

    0x126c8953,// 451 PAY 448 

    0x06897856,// 452 PAY 449 

    0x6cf67166,// 453 PAY 450 

    0x525ed106,// 454 PAY 451 

    0x7da3a3ca,// 455 PAY 452 

    0x216d4212,// 456 PAY 453 

    0xdfd1539f,// 457 PAY 454 

    0xfe625d3c,// 458 PAY 455 

    0x4213bf1b,// 459 PAY 456 

    0xd7f8834a,// 460 PAY 457 

    0xc4b065b9,// 461 PAY 458 

    0xd5000000,// 462 PAY 459 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x018880b2 // 463 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt2_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 418 words. 

/// BDA size     is 1665 (0x681) 

/// BDA id       is 0xb49d 

    0x0681b49d,// 3 BDA   1 

/// PAY Generic Data size   : 1665 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xf8c4c718,// 4 PAY   1 

    0xd044d0fe,// 5 PAY   2 

    0x62e48565,// 6 PAY   3 

    0x80e5d09e,// 7 PAY   4 

    0x4ba985c9,// 8 PAY   5 

    0x96ad0613,// 9 PAY   6 

    0x291c6b26,// 10 PAY   7 

    0x7866055b,// 11 PAY   8 

    0x7fb216ca,// 12 PAY   9 

    0xe9d07e3b,// 13 PAY  10 

    0x6fdc40b1,// 14 PAY  11 

    0xa55d67f3,// 15 PAY  12 

    0x0b9cc94e,// 16 PAY  13 

    0x424deecf,// 17 PAY  14 

    0xafd36b37,// 18 PAY  15 

    0xf2a582be,// 19 PAY  16 

    0xa0604b7c,// 20 PAY  17 

    0x567a4db5,// 21 PAY  18 

    0x3dcf6fa7,// 22 PAY  19 

    0x62ab18fe,// 23 PAY  20 

    0x80220751,// 24 PAY  21 

    0x5e07eee5,// 25 PAY  22 

    0x500508e9,// 26 PAY  23 

    0x91241e0c,// 27 PAY  24 

    0xb1dd6761,// 28 PAY  25 

    0xa71ba8ed,// 29 PAY  26 

    0xd2853a87,// 30 PAY  27 

    0xf225b47f,// 31 PAY  28 

    0x2911bbde,// 32 PAY  29 

    0xc53a2e43,// 33 PAY  30 

    0x5e329820,// 34 PAY  31 

    0x388b4f7c,// 35 PAY  32 

    0x3eb16c0f,// 36 PAY  33 

    0x0a2dd7ce,// 37 PAY  34 

    0xd58bc84e,// 38 PAY  35 

    0x94abdb09,// 39 PAY  36 

    0x240d9aca,// 40 PAY  37 

    0x9ef06fc3,// 41 PAY  38 

    0xdba1cbeb,// 42 PAY  39 

    0xdba3d296,// 43 PAY  40 

    0x78d375c1,// 44 PAY  41 

    0x188c7d0d,// 45 PAY  42 

    0xd42bad77,// 46 PAY  43 

    0x24474aac,// 47 PAY  44 

    0x1436c8e1,// 48 PAY  45 

    0x2980a233,// 49 PAY  46 

    0x05c683ce,// 50 PAY  47 

    0x86b6a3dc,// 51 PAY  48 

    0x5a14c054,// 52 PAY  49 

    0x54c9897f,// 53 PAY  50 

    0x13a55edd,// 54 PAY  51 

    0x874f90d4,// 55 PAY  52 

    0xddd7db5c,// 56 PAY  53 

    0xf9e9e0f9,// 57 PAY  54 

    0x13455c2c,// 58 PAY  55 

    0x43656b57,// 59 PAY  56 

    0x9c33a077,// 60 PAY  57 

    0xe3497662,// 61 PAY  58 

    0x3eb2dc68,// 62 PAY  59 

    0x7598208a,// 63 PAY  60 

    0xa6f6abfc,// 64 PAY  61 

    0xca151563,// 65 PAY  62 

    0x18387bc3,// 66 PAY  63 

    0x25e464e5,// 67 PAY  64 

    0x511942c3,// 68 PAY  65 

    0x1a734805,// 69 PAY  66 

    0x5e21ef4a,// 70 PAY  67 

    0x0a13095a,// 71 PAY  68 

    0x7c2df976,// 72 PAY  69 

    0xe5258e51,// 73 PAY  70 

    0x4423f165,// 74 PAY  71 

    0x9a7dd970,// 75 PAY  72 

    0xd346c855,// 76 PAY  73 

    0xb0d7a56f,// 77 PAY  74 

    0xf827f69c,// 78 PAY  75 

    0xc3ea28ac,// 79 PAY  76 

    0x2d1bc12d,// 80 PAY  77 

    0xb725a87e,// 81 PAY  78 

    0x20c55069,// 82 PAY  79 

    0xd1ab3607,// 83 PAY  80 

    0xe6345e47,// 84 PAY  81 

    0x130e20bf,// 85 PAY  82 

    0x76a80e35,// 86 PAY  83 

    0xcff34685,// 87 PAY  84 

    0x871029af,// 88 PAY  85 

    0xc3608935,// 89 PAY  86 

    0x82746d95,// 90 PAY  87 

    0xc261a3a0,// 91 PAY  88 

    0x85123bf2,// 92 PAY  89 

    0xe953c370,// 93 PAY  90 

    0xa1979407,// 94 PAY  91 

    0xa7666b02,// 95 PAY  92 

    0xc4c9bfff,// 96 PAY  93 

    0x3a0db88c,// 97 PAY  94 

    0xce456b2f,// 98 PAY  95 

    0x027094af,// 99 PAY  96 

    0x13ad8613,// 100 PAY  97 

    0xa184df19,// 101 PAY  98 

    0x1ddb8ee9,// 102 PAY  99 

    0xabc9e496,// 103 PAY 100 

    0x58412737,// 104 PAY 101 

    0xa1c59d75,// 105 PAY 102 

    0x7a984ba6,// 106 PAY 103 

    0x25d35471,// 107 PAY 104 

    0xa91da804,// 108 PAY 105 

    0x8634312c,// 109 PAY 106 

    0x69d68323,// 110 PAY 107 

    0x01bc0fab,// 111 PAY 108 

    0x1770a954,// 112 PAY 109 

    0xa886a10f,// 113 PAY 110 

    0x69500aa2,// 114 PAY 111 

    0xfc387129,// 115 PAY 112 

    0x6ae74e6b,// 116 PAY 113 

    0x7a1c05be,// 117 PAY 114 

    0x8ab1413d,// 118 PAY 115 

    0xef5924e3,// 119 PAY 116 

    0x493e5e4b,// 120 PAY 117 

    0x5cc6fe94,// 121 PAY 118 

    0x74d62d47,// 122 PAY 119 

    0xed206e13,// 123 PAY 120 

    0xecd0a2a1,// 124 PAY 121 

    0xa98002f3,// 125 PAY 122 

    0x7393cce8,// 126 PAY 123 

    0xce4fb080,// 127 PAY 124 

    0x1ae1f05e,// 128 PAY 125 

    0x16ec4186,// 129 PAY 126 

    0x1761dad0,// 130 PAY 127 

    0x7ce26159,// 131 PAY 128 

    0x85a07975,// 132 PAY 129 

    0x97cec0a3,// 133 PAY 130 

    0xbd305df9,// 134 PAY 131 

    0x3b4caa89,// 135 PAY 132 

    0xb8168ef9,// 136 PAY 133 

    0xc9a6626e,// 137 PAY 134 

    0x359b37d9,// 138 PAY 135 

    0x300cb76f,// 139 PAY 136 

    0xcea01f20,// 140 PAY 137 

    0x0622f4da,// 141 PAY 138 

    0xae0b233d,// 142 PAY 139 

    0xe523f60d,// 143 PAY 140 

    0x0950e533,// 144 PAY 141 

    0x36a1849e,// 145 PAY 142 

    0xf778e1e2,// 146 PAY 143 

    0x00750d48,// 147 PAY 144 

    0xca31f766,// 148 PAY 145 

    0x02ea14e9,// 149 PAY 146 

    0xe85bdeed,// 150 PAY 147 

    0x0a630409,// 151 PAY 148 

    0xa42be44e,// 152 PAY 149 

    0xa4055a37,// 153 PAY 150 

    0x3610843d,// 154 PAY 151 

    0x6dd9972b,// 155 PAY 152 

    0x88ce1b36,// 156 PAY 153 

    0x4dcc177c,// 157 PAY 154 

    0x5d8b2593,// 158 PAY 155 

    0x97869cd8,// 159 PAY 156 

    0x1af02d55,// 160 PAY 157 

    0x1a1fde42,// 161 PAY 158 

    0xe66dddbf,// 162 PAY 159 

    0x059a7124,// 163 PAY 160 

    0xd2e8f7ff,// 164 PAY 161 

    0x51bdab53,// 165 PAY 162 

    0x9b3e841c,// 166 PAY 163 

    0x2c8a1683,// 167 PAY 164 

    0xe412ad30,// 168 PAY 165 

    0x170b18db,// 169 PAY 166 

    0x182514f7,// 170 PAY 167 

    0xea775719,// 171 PAY 168 

    0x5e732298,// 172 PAY 169 

    0xdf5c2193,// 173 PAY 170 

    0x61baf9be,// 174 PAY 171 

    0x5518c6c3,// 175 PAY 172 

    0x4b77ffe9,// 176 PAY 173 

    0xe5a073bf,// 177 PAY 174 

    0x79aabf6f,// 178 PAY 175 

    0xd578986c,// 179 PAY 176 

    0xa3ab7be7,// 180 PAY 177 

    0x6ed1c469,// 181 PAY 178 

    0xf4cdc9d2,// 182 PAY 179 

    0x0701ed05,// 183 PAY 180 

    0x56716e73,// 184 PAY 181 

    0xbbb3eb0f,// 185 PAY 182 

    0x0a809d8a,// 186 PAY 183 

    0x48cf0ee1,// 187 PAY 184 

    0x9f76a68a,// 188 PAY 185 

    0x61d4cc62,// 189 PAY 186 

    0xc3b35c40,// 190 PAY 187 

    0x1e8f7098,// 191 PAY 188 

    0xea712371,// 192 PAY 189 

    0xdec9aaf3,// 193 PAY 190 

    0x4c0e0b48,// 194 PAY 191 

    0x72529453,// 195 PAY 192 

    0x9861866c,// 196 PAY 193 

    0xb5f62c4a,// 197 PAY 194 

    0x0d171464,// 198 PAY 195 

    0x8c34a1ac,// 199 PAY 196 

    0x74e21190,// 200 PAY 197 

    0x766a9f1b,// 201 PAY 198 

    0x8b35a38a,// 202 PAY 199 

    0x0609d668,// 203 PAY 200 

    0x36647635,// 204 PAY 201 

    0x349390b4,// 205 PAY 202 

    0x922eba96,// 206 PAY 203 

    0xaa416ee5,// 207 PAY 204 

    0x826cbff9,// 208 PAY 205 

    0xc2dc4e36,// 209 PAY 206 

    0x59298811,// 210 PAY 207 

    0xcc38f797,// 211 PAY 208 

    0x8a8e1516,// 212 PAY 209 

    0x08cce90d,// 213 PAY 210 

    0xbbd88e8a,// 214 PAY 211 

    0x532de01d,// 215 PAY 212 

    0x3dfa1a82,// 216 PAY 213 

    0xad0f6d47,// 217 PAY 214 

    0x3e76235d,// 218 PAY 215 

    0xf047eeaa,// 219 PAY 216 

    0x5c37a513,// 220 PAY 217 

    0xa7d3aa7e,// 221 PAY 218 

    0xb9f38fd3,// 222 PAY 219 

    0x46f66432,// 223 PAY 220 

    0x2297f6a0,// 224 PAY 221 

    0x7d6e3256,// 225 PAY 222 

    0xa22c7b1e,// 226 PAY 223 

    0xdafc38cd,// 227 PAY 224 

    0xcd0bb840,// 228 PAY 225 

    0x69ad876e,// 229 PAY 226 

    0xeb1746ec,// 230 PAY 227 

    0x8bb65def,// 231 PAY 228 

    0x48e42211,// 232 PAY 229 

    0xaff69937,// 233 PAY 230 

    0x890d7904,// 234 PAY 231 

    0x2645c30b,// 235 PAY 232 

    0x4d15039d,// 236 PAY 233 

    0x2da9edf5,// 237 PAY 234 

    0x239ed126,// 238 PAY 235 

    0x26b2bee3,// 239 PAY 236 

    0x44693ffb,// 240 PAY 237 

    0xd845bbf4,// 241 PAY 238 

    0x073a2193,// 242 PAY 239 

    0x64a32fd1,// 243 PAY 240 

    0xe67e3e9b,// 244 PAY 241 

    0xddb148b3,// 245 PAY 242 

    0x8c9cca9b,// 246 PAY 243 

    0x21e933da,// 247 PAY 244 

    0x08d07b6f,// 248 PAY 245 

    0x9f2814ac,// 249 PAY 246 

    0x28e3a119,// 250 PAY 247 

    0x38a4c30f,// 251 PAY 248 

    0xad7f328c,// 252 PAY 249 

    0xf65bc813,// 253 PAY 250 

    0xb37275d2,// 254 PAY 251 

    0x0407c360,// 255 PAY 252 

    0x42838d66,// 256 PAY 253 

    0x60fdd401,// 257 PAY 254 

    0xcf57abc0,// 258 PAY 255 

    0x8a1e658a,// 259 PAY 256 

    0x2609394c,// 260 PAY 257 

    0xa2c663d5,// 261 PAY 258 

    0x71cddf17,// 262 PAY 259 

    0xbe8fb0e7,// 263 PAY 260 

    0xc59c35f5,// 264 PAY 261 

    0xacc4fb12,// 265 PAY 262 

    0xf96b41fc,// 266 PAY 263 

    0x65da2988,// 267 PAY 264 

    0xa9dfe988,// 268 PAY 265 

    0x711fb787,// 269 PAY 266 

    0xcee88a88,// 270 PAY 267 

    0xf42ab9f1,// 271 PAY 268 

    0xb12370e7,// 272 PAY 269 

    0x2b59f368,// 273 PAY 270 

    0x95717141,// 274 PAY 271 

    0x43f4e93c,// 275 PAY 272 

    0xd9fe3631,// 276 PAY 273 

    0x417479f6,// 277 PAY 274 

    0xa0f884e0,// 278 PAY 275 

    0x1b1257b3,// 279 PAY 276 

    0x4b856b06,// 280 PAY 277 

    0xb43c3fa1,// 281 PAY 278 

    0xde3286db,// 282 PAY 279 

    0xbf09bc60,// 283 PAY 280 

    0x4d0ea9d9,// 284 PAY 281 

    0x595b6ba4,// 285 PAY 282 

    0xc9523860,// 286 PAY 283 

    0xfa19cda8,// 287 PAY 284 

    0x50065e84,// 288 PAY 285 

    0xfd25ac4a,// 289 PAY 286 

    0x676e0256,// 290 PAY 287 

    0xcc6287be,// 291 PAY 288 

    0x5963b0ae,// 292 PAY 289 

    0xee4bf4f5,// 293 PAY 290 

    0x57946741,// 294 PAY 291 

    0xda59ec12,// 295 PAY 292 

    0x2e52beb8,// 296 PAY 293 

    0xda859d3c,// 297 PAY 294 

    0xc1c61900,// 298 PAY 295 

    0xf5ddc70b,// 299 PAY 296 

    0x54faca7f,// 300 PAY 297 

    0xb1a00dca,// 301 PAY 298 

    0x36c0fe98,// 302 PAY 299 

    0x7f9e50c7,// 303 PAY 300 

    0x60e710e1,// 304 PAY 301 

    0x6f02385b,// 305 PAY 302 

    0x328b8937,// 306 PAY 303 

    0x42cc3475,// 307 PAY 304 

    0xfc7a220b,// 308 PAY 305 

    0x39545885,// 309 PAY 306 

    0x44dc382f,// 310 PAY 307 

    0x7bf17122,// 311 PAY 308 

    0xbb2b6eb0,// 312 PAY 309 

    0xebcbf550,// 313 PAY 310 

    0xb68810aa,// 314 PAY 311 

    0xa7333513,// 315 PAY 312 

    0x43b49ea7,// 316 PAY 313 

    0x47990243,// 317 PAY 314 

    0x4cedf958,// 318 PAY 315 

    0xf953d373,// 319 PAY 316 

    0xf4180637,// 320 PAY 317 

    0x9f2fccae,// 321 PAY 318 

    0xbdc9aac4,// 322 PAY 319 

    0xf5d18ff0,// 323 PAY 320 

    0x4fc99ebe,// 324 PAY 321 

    0xbe6017be,// 325 PAY 322 

    0x17a86db6,// 326 PAY 323 

    0x861baa0e,// 327 PAY 324 

    0x4d5f68c8,// 328 PAY 325 

    0xd1449091,// 329 PAY 326 

    0x769e8b0b,// 330 PAY 327 

    0xe3ad286d,// 331 PAY 328 

    0x7fc0bd05,// 332 PAY 329 

    0xb755a713,// 333 PAY 330 

    0x5671e0ee,// 334 PAY 331 

    0xaa2d6f95,// 335 PAY 332 

    0x2c1735fc,// 336 PAY 333 

    0x54cbf907,// 337 PAY 334 

    0x2391f994,// 338 PAY 335 

    0xe6081dc4,// 339 PAY 336 

    0x20c988b1,// 340 PAY 337 

    0x66544887,// 341 PAY 338 

    0x7b61de34,// 342 PAY 339 

    0x31561d13,// 343 PAY 340 

    0x930043ef,// 344 PAY 341 

    0x0ad1ebe9,// 345 PAY 342 

    0xf8cac3a6,// 346 PAY 343 

    0xecfabfb2,// 347 PAY 344 

    0x72f0e74d,// 348 PAY 345 

    0xfbeb434a,// 349 PAY 346 

    0x430343e5,// 350 PAY 347 

    0x46afeb4a,// 351 PAY 348 

    0xca0e97de,// 352 PAY 349 

    0x3d73eb96,// 353 PAY 350 

    0x5ea15a03,// 354 PAY 351 

    0xed58a651,// 355 PAY 352 

    0xddffb565,// 356 PAY 353 

    0xd5664c4f,// 357 PAY 354 

    0x3c3448ca,// 358 PAY 355 

    0x1ac0ff70,// 359 PAY 356 

    0x09fe17b4,// 360 PAY 357 

    0x507d31c7,// 361 PAY 358 

    0x49334560,// 362 PAY 359 

    0x7303b13a,// 363 PAY 360 

    0x8d19d014,// 364 PAY 361 

    0x19a1cdc6,// 365 PAY 362 

    0xd9f97847,// 366 PAY 363 

    0x365cd26f,// 367 PAY 364 

    0x2f1d68ee,// 368 PAY 365 

    0xfd7a4061,// 369 PAY 366 

    0xad4a7438,// 370 PAY 367 

    0x504f8d54,// 371 PAY 368 

    0x81dadc6f,// 372 PAY 369 

    0x5b049b7e,// 373 PAY 370 

    0xa1d21a76,// 374 PAY 371 

    0x476bfe7e,// 375 PAY 372 

    0xa799e80d,// 376 PAY 373 

    0x2371051b,// 377 PAY 374 

    0x3c13b123,// 378 PAY 375 

    0xb89a46a0,// 379 PAY 376 

    0xe4f5bf21,// 380 PAY 377 

    0xe3f1ed2b,// 381 PAY 378 

    0x01599d7a,// 382 PAY 379 

    0x05f786d9,// 383 PAY 380 

    0xfc0b495b,// 384 PAY 381 

    0x9b7a6db0,// 385 PAY 382 

    0x473b73e1,// 386 PAY 383 

    0xd352d10a,// 387 PAY 384 

    0x06ccec57,// 388 PAY 385 

    0xcc420b0e,// 389 PAY 386 

    0x4f0bcdea,// 390 PAY 387 

    0x33e3953d,// 391 PAY 388 

    0x5d550d43,// 392 PAY 389 

    0x395db758,// 393 PAY 390 

    0x428cbcaa,// 394 PAY 391 

    0x4e6068a0,// 395 PAY 392 

    0x2f4c05e0,// 396 PAY 393 

    0x3f5fa874,// 397 PAY 394 

    0xaec0807d,// 398 PAY 395 

    0x4f4966c2,// 399 PAY 396 

    0x63ab750b,// 400 PAY 397 

    0x4f5ad259,// 401 PAY 398 

    0x1025ea48,// 402 PAY 399 

    0x2b578b01,// 403 PAY 400 

    0xd684ed22,// 404 PAY 401 

    0x0ab8df84,// 405 PAY 402 

    0x30ba45a8,// 406 PAY 403 

    0xf56e83e7,// 407 PAY 404 

    0xfc1c20ab,// 408 PAY 405 

    0xc50c0a9c,// 409 PAY 406 

    0xd7db82ae,// 410 PAY 407 

    0x2f8cfc5f,// 411 PAY 408 

    0xc230c120,// 412 PAY 409 

    0x81d30183,// 413 PAY 410 

    0xc386d8f9,// 414 PAY 411 

    0xb79e7a02,// 415 PAY 412 

    0x13824bfb,// 416 PAY 413 

    0x8e34d0a2,// 417 PAY 414 

    0x70b7a004,// 418 PAY 415 

    0xb40780b6,// 419 PAY 416 

    0x54000000,// 420 PAY 417 

/// STA is 1 words. 

/// STA num_pkts       : 74 

/// STA pkt_idx        : 189 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe9 

    0x02f5e94a // 421 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt3_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 265 words. 

/// BDA size     is 1055 (0x41f) 

/// BDA id       is 0x61e 

    0x041f061e,// 3 BDA   1 

/// PAY Generic Data size   : 1055 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xd04c17e5,// 4 PAY   1 

    0x7af956f3,// 5 PAY   2 

    0xce50f284,// 6 PAY   3 

    0xe7bed5f3,// 7 PAY   4 

    0x323ef08e,// 8 PAY   5 

    0x62d0e779,// 9 PAY   6 

    0x81f8f0ba,// 10 PAY   7 

    0x1cd9f79e,// 11 PAY   8 

    0xd43057e8,// 12 PAY   9 

    0x7ce9d632,// 13 PAY  10 

    0x594b464d,// 14 PAY  11 

    0x65141b2b,// 15 PAY  12 

    0xbeae74f3,// 16 PAY  13 

    0xf034c156,// 17 PAY  14 

    0x05af7156,// 18 PAY  15 

    0x2f6e49e2,// 19 PAY  16 

    0xeeb59942,// 20 PAY  17 

    0xa11b51f8,// 21 PAY  18 

    0x264a7311,// 22 PAY  19 

    0x3f640eba,// 23 PAY  20 

    0x582b163e,// 24 PAY  21 

    0xf0a4d3c0,// 25 PAY  22 

    0x1883ef56,// 26 PAY  23 

    0x3ae39f11,// 27 PAY  24 

    0xef23ef70,// 28 PAY  25 

    0x554a3cc2,// 29 PAY  26 

    0xc3874185,// 30 PAY  27 

    0x8ff4a023,// 31 PAY  28 

    0xe9144cf4,// 32 PAY  29 

    0xbec2531a,// 33 PAY  30 

    0xfef7ae99,// 34 PAY  31 

    0xcecd9f33,// 35 PAY  32 

    0x8e8467d3,// 36 PAY  33 

    0x691d51d0,// 37 PAY  34 

    0x053d5f45,// 38 PAY  35 

    0x69e48bf5,// 39 PAY  36 

    0xe428776b,// 40 PAY  37 

    0x323f382c,// 41 PAY  38 

    0x3ca1cec9,// 42 PAY  39 

    0x1e77bd66,// 43 PAY  40 

    0x6388ee59,// 44 PAY  41 

    0xc915c172,// 45 PAY  42 

    0x8c789ebc,// 46 PAY  43 

    0xc8ad18db,// 47 PAY  44 

    0xc2883ebc,// 48 PAY  45 

    0x4adc7a7d,// 49 PAY  46 

    0x8954472e,// 50 PAY  47 

    0xb82a20f0,// 51 PAY  48 

    0xe400a2c8,// 52 PAY  49 

    0xa150c4f1,// 53 PAY  50 

    0x1073df46,// 54 PAY  51 

    0x72549864,// 55 PAY  52 

    0x9afca132,// 56 PAY  53 

    0xffa41b44,// 57 PAY  54 

    0x105ca52e,// 58 PAY  55 

    0x2ba33c3e,// 59 PAY  56 

    0x41b01629,// 60 PAY  57 

    0xa33847ed,// 61 PAY  58 

    0x477181d2,// 62 PAY  59 

    0x8bcb059f,// 63 PAY  60 

    0x47ab2bd5,// 64 PAY  61 

    0x16c9c1c6,// 65 PAY  62 

    0x921b7340,// 66 PAY  63 

    0x6c6c16fd,// 67 PAY  64 

    0x19d8486a,// 68 PAY  65 

    0x1b573b25,// 69 PAY  66 

    0x7078289c,// 70 PAY  67 

    0x347ad75c,// 71 PAY  68 

    0xdeb884e7,// 72 PAY  69 

    0x0eb68a7e,// 73 PAY  70 

    0x8a69cae6,// 74 PAY  71 

    0x14ef66a6,// 75 PAY  72 

    0xba3cc7c0,// 76 PAY  73 

    0xe8647a51,// 77 PAY  74 

    0x20412613,// 78 PAY  75 

    0xda85949f,// 79 PAY  76 

    0xd244d135,// 80 PAY  77 

    0xa9545a07,// 81 PAY  78 

    0x1a48b401,// 82 PAY  79 

    0xfe3c6059,// 83 PAY  80 

    0x46c3ef67,// 84 PAY  81 

    0x4b5d4b15,// 85 PAY  82 

    0x783f7153,// 86 PAY  83 

    0x3a974454,// 87 PAY  84 

    0xff623323,// 88 PAY  85 

    0x56c5dd4c,// 89 PAY  86 

    0x62ea4ac5,// 90 PAY  87 

    0x8fcf859b,// 91 PAY  88 

    0xdb701b44,// 92 PAY  89 

    0xda2fcca7,// 93 PAY  90 

    0xe3a2781c,// 94 PAY  91 

    0x4a0fd1c5,// 95 PAY  92 

    0xb0f95e42,// 96 PAY  93 

    0x1ec90ffc,// 97 PAY  94 

    0x720184d5,// 98 PAY  95 

    0x1b292776,// 99 PAY  96 

    0x564e9d14,// 100 PAY  97 

    0x2aae6dfe,// 101 PAY  98 

    0x94860e0b,// 102 PAY  99 

    0xcb54ff3c,// 103 PAY 100 

    0x70e6514e,// 104 PAY 101 

    0xbae5598a,// 105 PAY 102 

    0x5d4dc8d6,// 106 PAY 103 

    0xfedf7321,// 107 PAY 104 

    0x49293cff,// 108 PAY 105 

    0xf3040369,// 109 PAY 106 

    0xc8bafd8f,// 110 PAY 107 

    0xc0bbd333,// 111 PAY 108 

    0xac1e3992,// 112 PAY 109 

    0xe9413114,// 113 PAY 110 

    0x39cd5886,// 114 PAY 111 

    0xa9f9aaa5,// 115 PAY 112 

    0xec2ff718,// 116 PAY 113 

    0x06d86028,// 117 PAY 114 

    0x1b3b93b7,// 118 PAY 115 

    0xc3ed43c3,// 119 PAY 116 

    0x921ce0d2,// 120 PAY 117 

    0x6ee2da95,// 121 PAY 118 

    0x95db2eeb,// 122 PAY 119 

    0xb757293a,// 123 PAY 120 

    0xd3dd244f,// 124 PAY 121 

    0x28eaf696,// 125 PAY 122 

    0x3a398fdf,// 126 PAY 123 

    0xccad862a,// 127 PAY 124 

    0x84cc2aca,// 128 PAY 125 

    0x9172410a,// 129 PAY 126 

    0x4cad7e69,// 130 PAY 127 

    0xafc44977,// 131 PAY 128 

    0x9508aa75,// 132 PAY 129 

    0x3d809c0a,// 133 PAY 130 

    0x2ee894d7,// 134 PAY 131 

    0x8ddfccb1,// 135 PAY 132 

    0xccbf6805,// 136 PAY 133 

    0x8d40b13f,// 137 PAY 134 

    0xaa40fd23,// 138 PAY 135 

    0x5483b58e,// 139 PAY 136 

    0x7c986ebd,// 140 PAY 137 

    0xe46dc773,// 141 PAY 138 

    0xe68c5697,// 142 PAY 139 

    0xd2df36e2,// 143 PAY 140 

    0xe9c186d3,// 144 PAY 141 

    0xbd3af3a6,// 145 PAY 142 

    0x1b8996dc,// 146 PAY 143 

    0x04907115,// 147 PAY 144 

    0x148b6a12,// 148 PAY 145 

    0xb14666c5,// 149 PAY 146 

    0x3c6e92db,// 150 PAY 147 

    0x6a64e33b,// 151 PAY 148 

    0x08861609,// 152 PAY 149 

    0x9a3b9c36,// 153 PAY 150 

    0xdd73023e,// 154 PAY 151 

    0x5bf1cdf6,// 155 PAY 152 

    0xd58c18c3,// 156 PAY 153 

    0x1709557b,// 157 PAY 154 

    0x5cfc8303,// 158 PAY 155 

    0xc857ccbe,// 159 PAY 156 

    0xbb854fe9,// 160 PAY 157 

    0x61704a96,// 161 PAY 158 

    0x2de19050,// 162 PAY 159 

    0x6cef3f79,// 163 PAY 160 

    0x8d217784,// 164 PAY 161 

    0xbbde8cf6,// 165 PAY 162 

    0xbb8df29e,// 166 PAY 163 

    0x45458a13,// 167 PAY 164 

    0xe251ffda,// 168 PAY 165 

    0x1fc5b97c,// 169 PAY 166 

    0x4f842917,// 170 PAY 167 

    0x9a3f944e,// 171 PAY 168 

    0x6bc05cbb,// 172 PAY 169 

    0xe72e1447,// 173 PAY 170 

    0x3f462a38,// 174 PAY 171 

    0x092889a5,// 175 PAY 172 

    0x07598579,// 176 PAY 173 

    0xab0f0791,// 177 PAY 174 

    0x19531121,// 178 PAY 175 

    0xd11e23d7,// 179 PAY 176 

    0x062bc80c,// 180 PAY 177 

    0xce02506f,// 181 PAY 178 

    0x616f463e,// 182 PAY 179 

    0x3ac3f73b,// 183 PAY 180 

    0x31896526,// 184 PAY 181 

    0x3aa9ea22,// 185 PAY 182 

    0xd3ec24ba,// 186 PAY 183 

    0x946cb132,// 187 PAY 184 

    0xdf8244f1,// 188 PAY 185 

    0xf1fcc299,// 189 PAY 186 

    0x7c014a04,// 190 PAY 187 

    0x564610b2,// 191 PAY 188 

    0x1ac19a52,// 192 PAY 189 

    0x550953da,// 193 PAY 190 

    0xd7ca0016,// 194 PAY 191 

    0x0dd23a12,// 195 PAY 192 

    0xc010d7c6,// 196 PAY 193 

    0xacf93d95,// 197 PAY 194 

    0xc33b71c6,// 198 PAY 195 

    0x87bdc006,// 199 PAY 196 

    0x12a2546c,// 200 PAY 197 

    0x3b5ec29d,// 201 PAY 198 

    0x14481049,// 202 PAY 199 

    0x063f989a,// 203 PAY 200 

    0xc79b8dcf,// 204 PAY 201 

    0x7d38acae,// 205 PAY 202 

    0xcc3d19af,// 206 PAY 203 

    0x538b5843,// 207 PAY 204 

    0x55602636,// 208 PAY 205 

    0xed2d46bf,// 209 PAY 206 

    0x5c071b69,// 210 PAY 207 

    0x91715ce7,// 211 PAY 208 

    0x7a634502,// 212 PAY 209 

    0xa6cf9901,// 213 PAY 210 

    0x5c986634,// 214 PAY 211 

    0x7fdf6220,// 215 PAY 212 

    0x71769269,// 216 PAY 213 

    0x42a860b4,// 217 PAY 214 

    0xff39845b,// 218 PAY 215 

    0x82b915c8,// 219 PAY 216 

    0xf40a34b3,// 220 PAY 217 

    0x5e938aac,// 221 PAY 218 

    0x17d5cc18,// 222 PAY 219 

    0xc2e52025,// 223 PAY 220 

    0x4fac1fb6,// 224 PAY 221 

    0x9c763460,// 225 PAY 222 

    0xea614b56,// 226 PAY 223 

    0xdda8d7d3,// 227 PAY 224 

    0x52ab3fc4,// 228 PAY 225 

    0xb935bfaf,// 229 PAY 226 

    0x24195a12,// 230 PAY 227 

    0xede5f7d0,// 231 PAY 228 

    0x496d2430,// 232 PAY 229 

    0x1a6b4bd6,// 233 PAY 230 

    0x3197afe2,// 234 PAY 231 

    0x9e9e3b8d,// 235 PAY 232 

    0x161896f3,// 236 PAY 233 

    0xa574d79a,// 237 PAY 234 

    0x89574d46,// 238 PAY 235 

    0x63908fa4,// 239 PAY 236 

    0xb6187b02,// 240 PAY 237 

    0x8dd603d1,// 241 PAY 238 

    0x9e17b3cf,// 242 PAY 239 

    0x22c21190,// 243 PAY 240 

    0xdf1b0444,// 244 PAY 241 

    0xc1483cbd,// 245 PAY 242 

    0x0066d0bc,// 246 PAY 243 

    0x365987cf,// 247 PAY 244 

    0xd9cae268,// 248 PAY 245 

    0x1a706a8b,// 249 PAY 246 

    0x9dfd8079,// 250 PAY 247 

    0xca0e74cb,// 251 PAY 248 

    0x2f3ac9ec,// 252 PAY 249 

    0x53198637,// 253 PAY 250 

    0x74215705,// 254 PAY 251 

    0x8b19721f,// 255 PAY 252 

    0x75136567,// 256 PAY 253 

    0x0a79f9bb,// 257 PAY 254 

    0x96aab64c,// 258 PAY 255 

    0xe6d1535d,// 259 PAY 256 

    0x414d9c39,// 260 PAY 257 

    0x49da01da,// 261 PAY 258 

    0xc8729d9c,// 262 PAY 259 

    0x8d1ab53f,// 263 PAY 260 

    0x35c32306,// 264 PAY 261 

    0x8041832a,// 265 PAY 262 

    0xff31377c,// 266 PAY 263 

    0xec9fff00,// 267 PAY 264 

/// HASH is  8 bytes 

    0x82b915c8,// 268 HSH   1 

    0xf40a34b3,// 269 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 112 

/// STA pkt_idx        : 18 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb1 

    0x0049b170 // 270 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt4_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 343 words. 

/// BDA size     is 1368 (0x558) 

/// BDA id       is 0xff9 

    0x05580ff9,// 3 BDA   1 

/// PAY Generic Data size   : 1368 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xea989e54,// 4 PAY   1 

    0x8eea52ee,// 5 PAY   2 

    0x8a4131eb,// 6 PAY   3 

    0xa8a1f5bc,// 7 PAY   4 

    0xebd1656c,// 8 PAY   5 

    0x1c62b056,// 9 PAY   6 

    0x251138c1,// 10 PAY   7 

    0xec940eb4,// 11 PAY   8 

    0x7a0b0f93,// 12 PAY   9 

    0x8c95142c,// 13 PAY  10 

    0x8c913966,// 14 PAY  11 

    0x7f161322,// 15 PAY  12 

    0x63d87181,// 16 PAY  13 

    0x12dce921,// 17 PAY  14 

    0x60fc63be,// 18 PAY  15 

    0xca2c0ce9,// 19 PAY  16 

    0x0189f2f5,// 20 PAY  17 

    0x23be0521,// 21 PAY  18 

    0xf5af28c5,// 22 PAY  19 

    0xb33541a4,// 23 PAY  20 

    0x54b3f0a4,// 24 PAY  21 

    0xa0a892c3,// 25 PAY  22 

    0x2f7add54,// 26 PAY  23 

    0xffd48f48,// 27 PAY  24 

    0x332e5e30,// 28 PAY  25 

    0x9e3bab1d,// 29 PAY  26 

    0xdd2c213f,// 30 PAY  27 

    0x80199b26,// 31 PAY  28 

    0x5515f5a9,// 32 PAY  29 

    0x12c0c768,// 33 PAY  30 

    0x9eba46df,// 34 PAY  31 

    0xa4a2e735,// 35 PAY  32 

    0xb70b2cdb,// 36 PAY  33 

    0xb2969f02,// 37 PAY  34 

    0x0ff14b60,// 38 PAY  35 

    0xae5949dc,// 39 PAY  36 

    0xc81ff3ae,// 40 PAY  37 

    0x6dbd3f29,// 41 PAY  38 

    0x58012212,// 42 PAY  39 

    0x38952af9,// 43 PAY  40 

    0x1b114a7e,// 44 PAY  41 

    0xb9859cb1,// 45 PAY  42 

    0xc44185dc,// 46 PAY  43 

    0x14395fd1,// 47 PAY  44 

    0x67cb1b40,// 48 PAY  45 

    0x2a27ca7c,// 49 PAY  46 

    0x73f1c740,// 50 PAY  47 

    0xe0fdce20,// 51 PAY  48 

    0x42df1743,// 52 PAY  49 

    0x94a0ec48,// 53 PAY  50 

    0x6231b5fc,// 54 PAY  51 

    0xb45bd432,// 55 PAY  52 

    0x78a66043,// 56 PAY  53 

    0x01a66b40,// 57 PAY  54 

    0x149b28d5,// 58 PAY  55 

    0x5eb509c6,// 59 PAY  56 

    0x5999fec8,// 60 PAY  57 

    0x05f3d303,// 61 PAY  58 

    0x3dd0a538,// 62 PAY  59 

    0xf918eb79,// 63 PAY  60 

    0x66f53a89,// 64 PAY  61 

    0x1b6b702e,// 65 PAY  62 

    0xfb098168,// 66 PAY  63 

    0xc7244f94,// 67 PAY  64 

    0x15957908,// 68 PAY  65 

    0xf5f0f351,// 69 PAY  66 

    0x3a28b326,// 70 PAY  67 

    0xc0b06a6c,// 71 PAY  68 

    0x4d1170b7,// 72 PAY  69 

    0xb3d6bc2e,// 73 PAY  70 

    0x9f1d2506,// 74 PAY  71 

    0x2c6d887c,// 75 PAY  72 

    0xfa084936,// 76 PAY  73 

    0xae61a1a9,// 77 PAY  74 

    0xc71d5ecc,// 78 PAY  75 

    0x4fbc93db,// 79 PAY  76 

    0x76d2646b,// 80 PAY  77 

    0x08726d42,// 81 PAY  78 

    0xe156bd29,// 82 PAY  79 

    0xaed28808,// 83 PAY  80 

    0xb23e39ab,// 84 PAY  81 

    0xd1394bd2,// 85 PAY  82 

    0x6a78b792,// 86 PAY  83 

    0xf5e12253,// 87 PAY  84 

    0xd1a26b74,// 88 PAY  85 

    0xd9834e71,// 89 PAY  86 

    0x411ce544,// 90 PAY  87 

    0x03403e62,// 91 PAY  88 

    0x2e1309bf,// 92 PAY  89 

    0x1658b21a,// 93 PAY  90 

    0x08158030,// 94 PAY  91 

    0x0d6188b9,// 95 PAY  92 

    0xe24d2852,// 96 PAY  93 

    0x1ce1cd18,// 97 PAY  94 

    0x319f9c76,// 98 PAY  95 

    0xa0ce4a30,// 99 PAY  96 

    0x06cca2f2,// 100 PAY  97 

    0x6ebde4f8,// 101 PAY  98 

    0xe9d5650e,// 102 PAY  99 

    0x3fd5ddfe,// 103 PAY 100 

    0x95d7c6f5,// 104 PAY 101 

    0x57ac95ee,// 105 PAY 102 

    0xabef10aa,// 106 PAY 103 

    0x9f1a6a4d,// 107 PAY 104 

    0x0ebf9a21,// 108 PAY 105 

    0x2275be2b,// 109 PAY 106 

    0xc602a1e3,// 110 PAY 107 

    0xc1edb958,// 111 PAY 108 

    0x32e3d11c,// 112 PAY 109 

    0x419dc441,// 113 PAY 110 

    0x7b36e09e,// 114 PAY 111 

    0x91188066,// 115 PAY 112 

    0x08479f0b,// 116 PAY 113 

    0xb8dd2a57,// 117 PAY 114 

    0xc097c2ff,// 118 PAY 115 

    0x1367f776,// 119 PAY 116 

    0x81f907ce,// 120 PAY 117 

    0x34282708,// 121 PAY 118 

    0x7e23c4c6,// 122 PAY 119 

    0x3a969b7f,// 123 PAY 120 

    0x6fbaad6b,// 124 PAY 121 

    0xab7bc195,// 125 PAY 122 

    0xfdcc0d88,// 126 PAY 123 

    0x39d8a792,// 127 PAY 124 

    0x6fe04cd5,// 128 PAY 125 

    0x6601ab0e,// 129 PAY 126 

    0x114077e9,// 130 PAY 127 

    0x61d67aea,// 131 PAY 128 

    0xffce4e53,// 132 PAY 129 

    0x4b1d80d9,// 133 PAY 130 

    0xc1826c27,// 134 PAY 131 

    0xc05e976a,// 135 PAY 132 

    0x6f33d59c,// 136 PAY 133 

    0x33d5f7a0,// 137 PAY 134 

    0xe752a62d,// 138 PAY 135 

    0xa4dd64e0,// 139 PAY 136 

    0x197f7050,// 140 PAY 137 

    0x9560544f,// 141 PAY 138 

    0x0ce1bda5,// 142 PAY 139 

    0x49fcc4a0,// 143 PAY 140 

    0x50aad7c5,// 144 PAY 141 

    0x7a275c10,// 145 PAY 142 

    0x0d58c4d9,// 146 PAY 143 

    0xf17d53d8,// 147 PAY 144 

    0xebaf035c,// 148 PAY 145 

    0xc7b719b9,// 149 PAY 146 

    0x4e92774d,// 150 PAY 147 

    0xb318fdc0,// 151 PAY 148 

    0xa61f20e4,// 152 PAY 149 

    0x9b958d8c,// 153 PAY 150 

    0x38b78576,// 154 PAY 151 

    0x719f4034,// 155 PAY 152 

    0x02f104b5,// 156 PAY 153 

    0xe7cf4f94,// 157 PAY 154 

    0x4bfeb1e4,// 158 PAY 155 

    0x261750aa,// 159 PAY 156 

    0xa3b43eaf,// 160 PAY 157 

    0xfa12511d,// 161 PAY 158 

    0x3550aa5f,// 162 PAY 159 

    0xc087d6a9,// 163 PAY 160 

    0x5674e8f2,// 164 PAY 161 

    0x97cd8465,// 165 PAY 162 

    0x26cbc071,// 166 PAY 163 

    0xe2f18a5b,// 167 PAY 164 

    0x0c581b81,// 168 PAY 165 

    0x169a511f,// 169 PAY 166 

    0xf0822e00,// 170 PAY 167 

    0xa3d4f244,// 171 PAY 168 

    0xabb8c7dc,// 172 PAY 169 

    0xebc9af26,// 173 PAY 170 

    0xab6b3a09,// 174 PAY 171 

    0x10fff074,// 175 PAY 172 

    0x286b888b,// 176 PAY 173 

    0xea096397,// 177 PAY 174 

    0x0b7625c5,// 178 PAY 175 

    0x1e22a1fa,// 179 PAY 176 

    0xba6d9f93,// 180 PAY 177 

    0xd5103fd0,// 181 PAY 178 

    0x5b43f707,// 182 PAY 179 

    0x3999a3e1,// 183 PAY 180 

    0xb965b572,// 184 PAY 181 

    0xa0be3ddb,// 185 PAY 182 

    0x86886fcb,// 186 PAY 183 

    0x767f6714,// 187 PAY 184 

    0x5efabefd,// 188 PAY 185 

    0x806240d3,// 189 PAY 186 

    0x82aec060,// 190 PAY 187 

    0xe9242b16,// 191 PAY 188 

    0xa2ca9945,// 192 PAY 189 

    0xe9d042b5,// 193 PAY 190 

    0xf3dfd3b3,// 194 PAY 191 

    0xe58e8894,// 195 PAY 192 

    0xb7d2eb29,// 196 PAY 193 

    0x37f30db6,// 197 PAY 194 

    0xae60fef4,// 198 PAY 195 

    0xf9ca6f34,// 199 PAY 196 

    0xec46874f,// 200 PAY 197 

    0xa1411e50,// 201 PAY 198 

    0xa1122187,// 202 PAY 199 

    0xf56cffb3,// 203 PAY 200 

    0x519d9535,// 204 PAY 201 

    0x0b20f663,// 205 PAY 202 

    0x4c9e71f3,// 206 PAY 203 

    0xe920706c,// 207 PAY 204 

    0x5a8ae925,// 208 PAY 205 

    0xf8469d1d,// 209 PAY 206 

    0x49803992,// 210 PAY 207 

    0x1b1dd737,// 211 PAY 208 

    0x259e6564,// 212 PAY 209 

    0xc0c588cc,// 213 PAY 210 

    0x3aca694a,// 214 PAY 211 

    0x44a8ae8d,// 215 PAY 212 

    0xf30cfd39,// 216 PAY 213 

    0x0fb84b9d,// 217 PAY 214 

    0x3b38a440,// 218 PAY 215 

    0xe0f1c8e1,// 219 PAY 216 

    0xd5cd688a,// 220 PAY 217 

    0x79975295,// 221 PAY 218 

    0x6f974ac1,// 222 PAY 219 

    0x15bd5ac2,// 223 PAY 220 

    0xf51a7066,// 224 PAY 221 

    0x884643e5,// 225 PAY 222 

    0xd4c883b4,// 226 PAY 223 

    0xd2074cc2,// 227 PAY 224 

    0x1236f982,// 228 PAY 225 

    0xe2d6647f,// 229 PAY 226 

    0x5e1f9c6c,// 230 PAY 227 

    0xca6e7799,// 231 PAY 228 

    0xfb1dcef8,// 232 PAY 229 

    0x9f5dd788,// 233 PAY 230 

    0x9140dcef,// 234 PAY 231 

    0xa07f388d,// 235 PAY 232 

    0x76228c11,// 236 PAY 233 

    0xb723558f,// 237 PAY 234 

    0x5839d60e,// 238 PAY 235 

    0x96e6f013,// 239 PAY 236 

    0xc9ad4130,// 240 PAY 237 

    0x2c10edc5,// 241 PAY 238 

    0xc1125f0a,// 242 PAY 239 

    0x810bdd94,// 243 PAY 240 

    0x74547d5b,// 244 PAY 241 

    0x771adbc8,// 245 PAY 242 

    0x8449a0f9,// 246 PAY 243 

    0xee1d001d,// 247 PAY 244 

    0x26ea03b9,// 248 PAY 245 

    0x31d1f29e,// 249 PAY 246 

    0x94eeff8e,// 250 PAY 247 

    0x4cce2ba9,// 251 PAY 248 

    0xcca7b8d1,// 252 PAY 249 

    0x39ac02b2,// 253 PAY 250 

    0x54164aa5,// 254 PAY 251 

    0xca2171a8,// 255 PAY 252 

    0x6fc94125,// 256 PAY 253 

    0xea8c836e,// 257 PAY 254 

    0xca2ec021,// 258 PAY 255 

    0xdf7d8e2f,// 259 PAY 256 

    0x39cb04f2,// 260 PAY 257 

    0x176c2eee,// 261 PAY 258 

    0x91288d2a,// 262 PAY 259 

    0x805af8d3,// 263 PAY 260 

    0xe0ab3045,// 264 PAY 261 

    0x9abab7ab,// 265 PAY 262 

    0x15411725,// 266 PAY 263 

    0x19d51e81,// 267 PAY 264 

    0xc3eea0a4,// 268 PAY 265 

    0xa5eeb0fa,// 269 PAY 266 

    0xd9175722,// 270 PAY 267 

    0xef5d7979,// 271 PAY 268 

    0xc0a49e3b,// 272 PAY 269 

    0xc31dd90a,// 273 PAY 270 

    0x8f75b43a,// 274 PAY 271 

    0x911387c6,// 275 PAY 272 

    0xdd196165,// 276 PAY 273 

    0x80891f91,// 277 PAY 274 

    0xc27be148,// 278 PAY 275 

    0x4be6cd7f,// 279 PAY 276 

    0x1afb867d,// 280 PAY 277 

    0x2f5f1906,// 281 PAY 278 

    0xf6a85682,// 282 PAY 279 

    0xae7fecd0,// 283 PAY 280 

    0xb98dfd0d,// 284 PAY 281 

    0xb859a1d1,// 285 PAY 282 

    0x5e61dc5a,// 286 PAY 283 

    0xd313b45b,// 287 PAY 284 

    0x44e27be6,// 288 PAY 285 

    0x698e99e2,// 289 PAY 286 

    0xeb9c28a6,// 290 PAY 287 

    0xc6796000,// 291 PAY 288 

    0xceb51b2a,// 292 PAY 289 

    0x4369590d,// 293 PAY 290 

    0xf09180c7,// 294 PAY 291 

    0x6e2c74aa,// 295 PAY 292 

    0x2f4c8f2f,// 296 PAY 293 

    0xe166630f,// 297 PAY 294 

    0x591eea35,// 298 PAY 295 

    0x289d4a01,// 299 PAY 296 

    0x6e6d27f6,// 300 PAY 297 

    0x8a584178,// 301 PAY 298 

    0x19690323,// 302 PAY 299 

    0xf2c6759f,// 303 PAY 300 

    0xe6143753,// 304 PAY 301 

    0x3c0c0a2f,// 305 PAY 302 

    0x38dd2a46,// 306 PAY 303 

    0xc13b4b76,// 307 PAY 304 

    0x081e6094,// 308 PAY 305 

    0x712d9918,// 309 PAY 306 

    0x9e3c4e33,// 310 PAY 307 

    0x2a397e8a,// 311 PAY 308 

    0xbe05741c,// 312 PAY 309 

    0xe4ca2a3c,// 313 PAY 310 

    0x7452a6a0,// 314 PAY 311 

    0x783dd5d5,// 315 PAY 312 

    0x7584517d,// 316 PAY 313 

    0x2c7f7a9f,// 317 PAY 314 

    0xf8798061,// 318 PAY 315 

    0x42d783db,// 319 PAY 316 

    0x8171576e,// 320 PAY 317 

    0x2423ca39,// 321 PAY 318 

    0x910e9f79,// 322 PAY 319 

    0x960acfd4,// 323 PAY 320 

    0x99ba4681,// 324 PAY 321 

    0xeef62ea6,// 325 PAY 322 

    0xccc6f040,// 326 PAY 323 

    0xe078e32d,// 327 PAY 324 

    0xcb285c3a,// 328 PAY 325 

    0x4f360876,// 329 PAY 326 

    0x5e658a7c,// 330 PAY 327 

    0x48a3dbef,// 331 PAY 328 

    0xacfb1914,// 332 PAY 329 

    0xd2b9861d,// 333 PAY 330 

    0x40c4e0bd,// 334 PAY 331 

    0x66d086f0,// 335 PAY 332 

    0x94d2db5d,// 336 PAY 333 

    0x05711aa5,// 337 PAY 334 

    0x8cc85540,// 338 PAY 335 

    0xe6500b6a,// 339 PAY 336 

    0x056bafdf,// 340 PAY 337 

    0x5f9cc538,// 341 PAY 338 

    0x064a442e,// 342 PAY 339 

    0x3232eae9,// 343 PAY 340 

    0xca61caa7,// 344 PAY 341 

    0x67233c7b,// 345 PAY 342 

/// HASH is  16 bytes 

    0xa3b43eaf,// 346 HSH   1 

    0xfa12511d,// 347 HSH   2 

    0x3550aa5f,// 348 HSH   3 

    0xc087d6a9,// 349 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 162 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0051cea2 // 350 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt5_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 257 words. 

/// BDA size     is 1023 (0x3ff) 

/// BDA id       is 0x93b0 

    0x03ff93b0,// 3 BDA   1 

/// PAY Generic Data size   : 1023 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x59d1b362,// 4 PAY   1 

    0x57c98d37,// 5 PAY   2 

    0x7a3efdd9,// 6 PAY   3 

    0x5d903373,// 7 PAY   4 

    0x24977305,// 8 PAY   5 

    0xe5f33d44,// 9 PAY   6 

    0xe229de08,// 10 PAY   7 

    0x21aac376,// 11 PAY   8 

    0xb2fac023,// 12 PAY   9 

    0xdd47f109,// 13 PAY  10 

    0x2d09540c,// 14 PAY  11 

    0x6fef7a76,// 15 PAY  12 

    0xc0f1dbca,// 16 PAY  13 

    0x7e3a891d,// 17 PAY  14 

    0x83541e8b,// 18 PAY  15 

    0x70e3e409,// 19 PAY  16 

    0x9b5ff111,// 20 PAY  17 

    0xff68919b,// 21 PAY  18 

    0xfc9214db,// 22 PAY  19 

    0x5fe1b731,// 23 PAY  20 

    0x034e5b41,// 24 PAY  21 

    0xde562dc4,// 25 PAY  22 

    0x8a73e7db,// 26 PAY  23 

    0x8c47169e,// 27 PAY  24 

    0xfc1aa1a9,// 28 PAY  25 

    0x2c8a1fdc,// 29 PAY  26 

    0x7704bef3,// 30 PAY  27 

    0x44bff8eb,// 31 PAY  28 

    0xaa921814,// 32 PAY  29 

    0xab847e90,// 33 PAY  30 

    0x971ced09,// 34 PAY  31 

    0xd208e989,// 35 PAY  32 

    0xf44d39db,// 36 PAY  33 

    0xf19e8d82,// 37 PAY  34 

    0xa3bcce12,// 38 PAY  35 

    0x0aed7bd1,// 39 PAY  36 

    0xda77313a,// 40 PAY  37 

    0xe2376b44,// 41 PAY  38 

    0x27f42c92,// 42 PAY  39 

    0x89cde1ca,// 43 PAY  40 

    0xd5976ae1,// 44 PAY  41 

    0xbc01e94a,// 45 PAY  42 

    0x16994d5c,// 46 PAY  43 

    0x50e4f419,// 47 PAY  44 

    0x62efe5e1,// 48 PAY  45 

    0x629f2b70,// 49 PAY  46 

    0x0d2f706c,// 50 PAY  47 

    0xd74a9f0a,// 51 PAY  48 

    0xbfc1545e,// 52 PAY  49 

    0x4f8e809c,// 53 PAY  50 

    0x2787a00a,// 54 PAY  51 

    0x3a1efd31,// 55 PAY  52 

    0x8b33d09b,// 56 PAY  53 

    0x44f1a928,// 57 PAY  54 

    0xb4c6d43c,// 58 PAY  55 

    0x4b2c7086,// 59 PAY  56 

    0x3114cb4f,// 60 PAY  57 

    0xb3b6ed98,// 61 PAY  58 

    0x65c16a6a,// 62 PAY  59 

    0xbfd23329,// 63 PAY  60 

    0xfaba8290,// 64 PAY  61 

    0x15a91658,// 65 PAY  62 

    0x19545ebc,// 66 PAY  63 

    0x682de205,// 67 PAY  64 

    0xd2ba7d85,// 68 PAY  65 

    0x00731536,// 69 PAY  66 

    0x5694961c,// 70 PAY  67 

    0x07ba647a,// 71 PAY  68 

    0xacc17492,// 72 PAY  69 

    0x4e195ec9,// 73 PAY  70 

    0xb543a5a3,// 74 PAY  71 

    0xdb263012,// 75 PAY  72 

    0x0dfcba16,// 76 PAY  73 

    0x81206f23,// 77 PAY  74 

    0x7746552c,// 78 PAY  75 

    0xa354d7aa,// 79 PAY  76 

    0xa735a3f5,// 80 PAY  77 

    0x66c6d0f4,// 81 PAY  78 

    0x04c4fe3d,// 82 PAY  79 

    0xbc3f84d5,// 83 PAY  80 

    0x7ac81727,// 84 PAY  81 

    0x618d7df0,// 85 PAY  82 

    0x6c4c6496,// 86 PAY  83 

    0xb65831b3,// 87 PAY  84 

    0xc80b2d97,// 88 PAY  85 

    0xe3226c77,// 89 PAY  86 

    0x0c686975,// 90 PAY  87 

    0x66238639,// 91 PAY  88 

    0xebaed710,// 92 PAY  89 

    0x5f929bdb,// 93 PAY  90 

    0x4b18110d,// 94 PAY  91 

    0x2d8667ad,// 95 PAY  92 

    0x4d3acf27,// 96 PAY  93 

    0x0391732b,// 97 PAY  94 

    0x424eeed2,// 98 PAY  95 

    0x405cb24d,// 99 PAY  96 

    0xf9204515,// 100 PAY  97 

    0xa6b77428,// 101 PAY  98 

    0x25361eef,// 102 PAY  99 

    0x21837dea,// 103 PAY 100 

    0x220ba498,// 104 PAY 101 

    0xa56757ca,// 105 PAY 102 

    0x2d8f45ac,// 106 PAY 103 

    0xa0d8b69f,// 107 PAY 104 

    0xe4dd469f,// 108 PAY 105 

    0xd455dbfe,// 109 PAY 106 

    0x1a08ce26,// 110 PAY 107 

    0x83769481,// 111 PAY 108 

    0xd97e1991,// 112 PAY 109 

    0x8f548ad5,// 113 PAY 110 

    0x481e0dbc,// 114 PAY 111 

    0x6726df13,// 115 PAY 112 

    0xa6341b60,// 116 PAY 113 

    0xfec6b5a0,// 117 PAY 114 

    0x64fc355e,// 118 PAY 115 

    0x54c2b099,// 119 PAY 116 

    0x99aa2a45,// 120 PAY 117 

    0xd6ba65c7,// 121 PAY 118 

    0x1a5dde5b,// 122 PAY 119 

    0xac487086,// 123 PAY 120 

    0x7048ba43,// 124 PAY 121 

    0x7dfdd403,// 125 PAY 122 

    0xc90d273e,// 126 PAY 123 

    0x89f59d36,// 127 PAY 124 

    0xb1f21401,// 128 PAY 125 

    0xea329323,// 129 PAY 126 

    0x25f93c23,// 130 PAY 127 

    0xaba90f16,// 131 PAY 128 

    0x8df48c9b,// 132 PAY 129 

    0x3a279d15,// 133 PAY 130 

    0xd8bc76f0,// 134 PAY 131 

    0xd5eace1e,// 135 PAY 132 

    0x56040dde,// 136 PAY 133 

    0xc7e267f6,// 137 PAY 134 

    0x281c0737,// 138 PAY 135 

    0x39a8205c,// 139 PAY 136 

    0x56e4ef63,// 140 PAY 137 

    0xec946fd3,// 141 PAY 138 

    0x92dc9189,// 142 PAY 139 

    0x00907085,// 143 PAY 140 

    0xed17b073,// 144 PAY 141 

    0xceab558e,// 145 PAY 142 

    0xe07cffeb,// 146 PAY 143 

    0x1a971b55,// 147 PAY 144 

    0xac654b74,// 148 PAY 145 

    0xa90072fd,// 149 PAY 146 

    0x231e42f4,// 150 PAY 147 

    0xc8d280b3,// 151 PAY 148 

    0x2f987dea,// 152 PAY 149 

    0x680c5e26,// 153 PAY 150 

    0x46058317,// 154 PAY 151 

    0x6bb53c81,// 155 PAY 152 

    0x8bb77b36,// 156 PAY 153 

    0x0e313818,// 157 PAY 154 

    0x095884fd,// 158 PAY 155 

    0x235b3e2c,// 159 PAY 156 

    0x9cde9b2a,// 160 PAY 157 

    0x2e2eb3c7,// 161 PAY 158 

    0x29442113,// 162 PAY 159 

    0x0f79478f,// 163 PAY 160 

    0x6fc6912c,// 164 PAY 161 

    0x0ddd8e01,// 165 PAY 162 

    0x7960a675,// 166 PAY 163 

    0xb23f8763,// 167 PAY 164 

    0xefbeada4,// 168 PAY 165 

    0x111f6a63,// 169 PAY 166 

    0x048c5824,// 170 PAY 167 

    0x6a806133,// 171 PAY 168 

    0x29669e83,// 172 PAY 169 

    0x3939a336,// 173 PAY 170 

    0x4738f1d3,// 174 PAY 171 

    0x05bfb21b,// 175 PAY 172 

    0xe52755ea,// 176 PAY 173 

    0xb155d2d4,// 177 PAY 174 

    0x090b2692,// 178 PAY 175 

    0x240d7587,// 179 PAY 176 

    0xaa63f8e2,// 180 PAY 177 

    0xbf9fb463,// 181 PAY 178 

    0xc29f5465,// 182 PAY 179 

    0x4c96f744,// 183 PAY 180 

    0x9ac83963,// 184 PAY 181 

    0x4c21950f,// 185 PAY 182 

    0x1bb79612,// 186 PAY 183 

    0xa48facd4,// 187 PAY 184 

    0x46f50d14,// 188 PAY 185 

    0x5cc428a6,// 189 PAY 186 

    0x5a66fbe0,// 190 PAY 187 

    0x842f3294,// 191 PAY 188 

    0xab320fee,// 192 PAY 189 

    0xaeac5cf4,// 193 PAY 190 

    0xb51f6938,// 194 PAY 191 

    0x47bb99d8,// 195 PAY 192 

    0x84d33b3f,// 196 PAY 193 

    0x38dbe5f9,// 197 PAY 194 

    0xf758cad5,// 198 PAY 195 

    0xe693179b,// 199 PAY 196 

    0x408939a1,// 200 PAY 197 

    0x37088aad,// 201 PAY 198 

    0x6886f4fc,// 202 PAY 199 

    0xb7dbf3aa,// 203 PAY 200 

    0x539ce4b4,// 204 PAY 201 

    0xe5c3197f,// 205 PAY 202 

    0x8b4b1587,// 206 PAY 203 

    0x30ea8b14,// 207 PAY 204 

    0x3e0c3e94,// 208 PAY 205 

    0x4077cd1c,// 209 PAY 206 

    0x9ca3dce7,// 210 PAY 207 

    0xc50a2ef6,// 211 PAY 208 

    0xd6922192,// 212 PAY 209 

    0x82a06c66,// 213 PAY 210 

    0x25216ef5,// 214 PAY 211 

    0x3daf8bfc,// 215 PAY 212 

    0x39668c99,// 216 PAY 213 

    0xe796b2d5,// 217 PAY 214 

    0xd83ed91f,// 218 PAY 215 

    0xd2d42f83,// 219 PAY 216 

    0x799589fb,// 220 PAY 217 

    0xd64c09bb,// 221 PAY 218 

    0xbfa2ebd3,// 222 PAY 219 

    0xab08170d,// 223 PAY 220 

    0xc590fb79,// 224 PAY 221 

    0xd16e6897,// 225 PAY 222 

    0xe365d1d7,// 226 PAY 223 

    0x1a67f705,// 227 PAY 224 

    0x739977cc,// 228 PAY 225 

    0x301d899c,// 229 PAY 226 

    0xd900ed29,// 230 PAY 227 

    0x288a5f64,// 231 PAY 228 

    0xbda180e9,// 232 PAY 229 

    0x96ed50dd,// 233 PAY 230 

    0x9ed543f0,// 234 PAY 231 

    0x8c778dd8,// 235 PAY 232 

    0xdd67ea7e,// 236 PAY 233 

    0xa887f619,// 237 PAY 234 

    0x1f55f9a9,// 238 PAY 235 

    0x518ef4c3,// 239 PAY 236 

    0xcb978c95,// 240 PAY 237 

    0xc47680a3,// 241 PAY 238 

    0x09130ec7,// 242 PAY 239 

    0x972f83d5,// 243 PAY 240 

    0xabfad66b,// 244 PAY 241 

    0x1c837fe9,// 245 PAY 242 

    0xe79b17a9,// 246 PAY 243 

    0x975c9814,// 247 PAY 244 

    0x22218f2a,// 248 PAY 245 

    0xc304e40b,// 249 PAY 246 

    0xeee17a05,// 250 PAY 247 

    0xe4ff9d49,// 251 PAY 248 

    0xc0d20ca8,// 252 PAY 249 

    0xe2cac35f,// 253 PAY 250 

    0xa1cca8a2,// 254 PAY 251 

    0xcc7d6769,// 255 PAY 252 

    0x299eb807,// 256 PAY 253 

    0x78a5597c,// 257 PAY 254 

    0xa6ef2b5a,// 258 PAY 255 

    0xc06dce00,// 259 PAY 256 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x20 

    0x02492053 // 260 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt6_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 438 words. 

/// BDA size     is 1748 (0x6d4) 

/// BDA id       is 0x4456 

    0x06d44456,// 3 BDA   1 

/// PAY Generic Data size   : 1748 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x89b21f43,// 4 PAY   1 

    0xd4bf5993,// 5 PAY   2 

    0xadb47145,// 6 PAY   3 

    0xd9092b51,// 7 PAY   4 

    0xc14524a9,// 8 PAY   5 

    0x87f025f0,// 9 PAY   6 

    0x9a295bd5,// 10 PAY   7 

    0x904c8ccd,// 11 PAY   8 

    0x8d6854ee,// 12 PAY   9 

    0xd8789317,// 13 PAY  10 

    0x7dd54cef,// 14 PAY  11 

    0x02b7ba9f,// 15 PAY  12 

    0x8e3f6e31,// 16 PAY  13 

    0xd6364921,// 17 PAY  14 

    0x6b7ef026,// 18 PAY  15 

    0x3d24e4ec,// 19 PAY  16 

    0x05dc83de,// 20 PAY  17 

    0x932c642b,// 21 PAY  18 

    0xcdef9ea3,// 22 PAY  19 

    0x99ca566d,// 23 PAY  20 

    0x2d173e2e,// 24 PAY  21 

    0xe23013f7,// 25 PAY  22 

    0xc2912c64,// 26 PAY  23 

    0x5c171651,// 27 PAY  24 

    0x0abbed7b,// 28 PAY  25 

    0x740fc007,// 29 PAY  26 

    0xcd872d37,// 30 PAY  27 

    0xebb7060b,// 31 PAY  28 

    0x943e1724,// 32 PAY  29 

    0x6f73f611,// 33 PAY  30 

    0x133f2c08,// 34 PAY  31 

    0x74a8b6cc,// 35 PAY  32 

    0xac69d097,// 36 PAY  33 

    0x749535a9,// 37 PAY  34 

    0x7eb3cbe1,// 38 PAY  35 

    0xe3add59e,// 39 PAY  36 

    0x3cb55c24,// 40 PAY  37 

    0x21d8e793,// 41 PAY  38 

    0x91fa3a5d,// 42 PAY  39 

    0x3bd7c3c5,// 43 PAY  40 

    0x443a4912,// 44 PAY  41 

    0x63c0c5fb,// 45 PAY  42 

    0xd2749b55,// 46 PAY  43 

    0xaf85b82d,// 47 PAY  44 

    0x4ad433d1,// 48 PAY  45 

    0xa0adae45,// 49 PAY  46 

    0xceb9c9b8,// 50 PAY  47 

    0x21beb450,// 51 PAY  48 

    0xf1054681,// 52 PAY  49 

    0xfeec6ee0,// 53 PAY  50 

    0xf2b3f9e3,// 54 PAY  51 

    0xebf110e6,// 55 PAY  52 

    0xa604a2a6,// 56 PAY  53 

    0x0d81bb6e,// 57 PAY  54 

    0xa3d22408,// 58 PAY  55 

    0x98735146,// 59 PAY  56 

    0x9cac3a6b,// 60 PAY  57 

    0x80a72f43,// 61 PAY  58 

    0xb8d7f1ed,// 62 PAY  59 

    0x8e611b31,// 63 PAY  60 

    0x9d892b7f,// 64 PAY  61 

    0xe13f7273,// 65 PAY  62 

    0xa9a4a75d,// 66 PAY  63 

    0x686f0b9c,// 67 PAY  64 

    0x9b427b8a,// 68 PAY  65 

    0x43b761d7,// 69 PAY  66 

    0xc6ade130,// 70 PAY  67 

    0x24cd7bfb,// 71 PAY  68 

    0x789cb381,// 72 PAY  69 

    0x6e9241d9,// 73 PAY  70 

    0x6de90ad5,// 74 PAY  71 

    0xeda8470b,// 75 PAY  72 

    0x67223435,// 76 PAY  73 

    0xe5357e40,// 77 PAY  74 

    0x9c05ef5f,// 78 PAY  75 

    0x9fb07655,// 79 PAY  76 

    0x1dfdb57a,// 80 PAY  77 

    0x5f058879,// 81 PAY  78 

    0x5f37921d,// 82 PAY  79 

    0xa7dcc3f4,// 83 PAY  80 

    0xef661c74,// 84 PAY  81 

    0x46ebf169,// 85 PAY  82 

    0xd8099092,// 86 PAY  83 

    0x08d0bf55,// 87 PAY  84 

    0x1c2a6329,// 88 PAY  85 

    0x8b4e8044,// 89 PAY  86 

    0x642c96a0,// 90 PAY  87 

    0x87395eb5,// 91 PAY  88 

    0xf60fb5c3,// 92 PAY  89 

    0x76925841,// 93 PAY  90 

    0x2873ede5,// 94 PAY  91 

    0x1493eb0f,// 95 PAY  92 

    0xa58aed0c,// 96 PAY  93 

    0xe06bb36d,// 97 PAY  94 

    0xd59e695d,// 98 PAY  95 

    0x74d1f9b7,// 99 PAY  96 

    0xdc17b52a,// 100 PAY  97 

    0x3103242c,// 101 PAY  98 

    0x95da9776,// 102 PAY  99 

    0x33ad7110,// 103 PAY 100 

    0x88b62a17,// 104 PAY 101 

    0x01987dc8,// 105 PAY 102 

    0xd675e4c1,// 106 PAY 103 

    0x07d5e376,// 107 PAY 104 

    0xaaec28f5,// 108 PAY 105 

    0x3752f0b3,// 109 PAY 106 

    0x69bc8db6,// 110 PAY 107 

    0x7ebdd597,// 111 PAY 108 

    0x8c814f7e,// 112 PAY 109 

    0xf7d659b9,// 113 PAY 110 

    0xbeed46ed,// 114 PAY 111 

    0x0b704ec9,// 115 PAY 112 

    0xa5c8f69d,// 116 PAY 113 

    0x24c91bd8,// 117 PAY 114 

    0x08fb3091,// 118 PAY 115 

    0x91ed390c,// 119 PAY 116 

    0x59c732cc,// 120 PAY 117 

    0x27042b42,// 121 PAY 118 

    0x88e6166e,// 122 PAY 119 

    0x13d5af35,// 123 PAY 120 

    0xba4b4963,// 124 PAY 121 

    0x526a5518,// 125 PAY 122 

    0x8e9dab28,// 126 PAY 123 

    0x7082dde0,// 127 PAY 124 

    0x83c07b70,// 128 PAY 125 

    0xffee1d25,// 129 PAY 126 

    0x86d6d12f,// 130 PAY 127 

    0xd39fcb20,// 131 PAY 128 

    0x1b05a45a,// 132 PAY 129 

    0xe5855ba4,// 133 PAY 130 

    0x0c3fd84d,// 134 PAY 131 

    0xac5e9a9f,// 135 PAY 132 

    0x37cbe98f,// 136 PAY 133 

    0xa86edb60,// 137 PAY 134 

    0xc44509b4,// 138 PAY 135 

    0x4a92ae32,// 139 PAY 136 

    0xe1270a08,// 140 PAY 137 

    0xe80f58fd,// 141 PAY 138 

    0x30bf9db3,// 142 PAY 139 

    0xf1cef3d4,// 143 PAY 140 

    0x22f4d1ae,// 144 PAY 141 

    0xa78ed5e0,// 145 PAY 142 

    0xfcb0143e,// 146 PAY 143 

    0xfc9819d8,// 147 PAY 144 

    0x543b2d8d,// 148 PAY 145 

    0x18166120,// 149 PAY 146 

    0xd974cb27,// 150 PAY 147 

    0xb71c5a56,// 151 PAY 148 

    0x5cc1d42a,// 152 PAY 149 

    0xd53fba8d,// 153 PAY 150 

    0xadb7eae7,// 154 PAY 151 

    0x068d1d97,// 155 PAY 152 

    0x1f3a5114,// 156 PAY 153 

    0xe43006ba,// 157 PAY 154 

    0x7b3bec11,// 158 PAY 155 

    0xf639d4f8,// 159 PAY 156 

    0x30b1cdac,// 160 PAY 157 

    0xe3614b00,// 161 PAY 158 

    0xa6d06b99,// 162 PAY 159 

    0x40d01937,// 163 PAY 160 

    0x2964d657,// 164 PAY 161 

    0x77e5caed,// 165 PAY 162 

    0xe0717519,// 166 PAY 163 

    0x2392827c,// 167 PAY 164 

    0xd816075e,// 168 PAY 165 

    0x53d6f83d,// 169 PAY 166 

    0xfd2a8ae5,// 170 PAY 167 

    0x27ed129f,// 171 PAY 168 

    0x95339c85,// 172 PAY 169 

    0x2ea5a47e,// 173 PAY 170 

    0xf01a3c00,// 174 PAY 171 

    0x557f9118,// 175 PAY 172 

    0x4295a6d5,// 176 PAY 173 

    0x895e79fc,// 177 PAY 174 

    0xea90ce7b,// 178 PAY 175 

    0x171a393a,// 179 PAY 176 

    0xce4f1998,// 180 PAY 177 

    0xefe5203c,// 181 PAY 178 

    0x9d1e0e61,// 182 PAY 179 

    0x00b71487,// 183 PAY 180 

    0x55331314,// 184 PAY 181 

    0xe79af46a,// 185 PAY 182 

    0x1f3d9024,// 186 PAY 183 

    0x1a003c4c,// 187 PAY 184 

    0xcee3d0ba,// 188 PAY 185 

    0xd9e97424,// 189 PAY 186 

    0xad06e774,// 190 PAY 187 

    0xdc1cb956,// 191 PAY 188 

    0xf38c9868,// 192 PAY 189 

    0xb8d1bfcb,// 193 PAY 190 

    0x2d444f57,// 194 PAY 191 

    0x0b13d700,// 195 PAY 192 

    0x1db93204,// 196 PAY 193 

    0xdc1dcfa9,// 197 PAY 194 

    0x22e3f602,// 198 PAY 195 

    0x85dd48f4,// 199 PAY 196 

    0x86452ea7,// 200 PAY 197 

    0x1b77eeeb,// 201 PAY 198 

    0xb7bb5b86,// 202 PAY 199 

    0x7ce6055b,// 203 PAY 200 

    0x0fe70c4b,// 204 PAY 201 

    0x108a770b,// 205 PAY 202 

    0x68def0bc,// 206 PAY 203 

    0x1278d898,// 207 PAY 204 

    0xe158b050,// 208 PAY 205 

    0x9883d88c,// 209 PAY 206 

    0xadad07dd,// 210 PAY 207 

    0x635db64b,// 211 PAY 208 

    0x4333b690,// 212 PAY 209 

    0xff372dd3,// 213 PAY 210 

    0xf3ce2e42,// 214 PAY 211 

    0x8f9e36d1,// 215 PAY 212 

    0x2a0b4ff1,// 216 PAY 213 

    0x2fd24b58,// 217 PAY 214 

    0x4b9236e1,// 218 PAY 215 

    0x171f0376,// 219 PAY 216 

    0x8bba280e,// 220 PAY 217 

    0x948bc3c8,// 221 PAY 218 

    0xebd92812,// 222 PAY 219 

    0x26fe8434,// 223 PAY 220 

    0x955c8cc6,// 224 PAY 221 

    0x8d59d506,// 225 PAY 222 

    0x1d85baef,// 226 PAY 223 

    0x0d44d39a,// 227 PAY 224 

    0xf038a808,// 228 PAY 225 

    0x5a45ea17,// 229 PAY 226 

    0x1dc8c391,// 230 PAY 227 

    0x6999e266,// 231 PAY 228 

    0x4cd32da7,// 232 PAY 229 

    0x289383c2,// 233 PAY 230 

    0x7250ca89,// 234 PAY 231 

    0x706aea8f,// 235 PAY 232 

    0x1bac50e1,// 236 PAY 233 

    0x56304ff0,// 237 PAY 234 

    0x6956f12e,// 238 PAY 235 

    0x15d70d1d,// 239 PAY 236 

    0xbb8aea55,// 240 PAY 237 

    0x66037fae,// 241 PAY 238 

    0xabc38722,// 242 PAY 239 

    0x414bd574,// 243 PAY 240 

    0xd59170c0,// 244 PAY 241 

    0x7f333b9d,// 245 PAY 242 

    0xa70497b5,// 246 PAY 243 

    0x248bca3d,// 247 PAY 244 

    0xf17f2aa0,// 248 PAY 245 

    0x56291fb6,// 249 PAY 246 

    0xcea91870,// 250 PAY 247 

    0x93d4956d,// 251 PAY 248 

    0x4d8409e3,// 252 PAY 249 

    0x67ca620d,// 253 PAY 250 

    0x654b1bde,// 254 PAY 251 

    0xc4cef84d,// 255 PAY 252 

    0x13b2d1b7,// 256 PAY 253 

    0x03fd1f04,// 257 PAY 254 

    0x9cf71d85,// 258 PAY 255 

    0x3102b2ba,// 259 PAY 256 

    0xa2eab23a,// 260 PAY 257 

    0x5cc5071f,// 261 PAY 258 

    0xae13f401,// 262 PAY 259 

    0x672a6420,// 263 PAY 260 

    0x2ca93146,// 264 PAY 261 

    0xeb59fcf5,// 265 PAY 262 

    0x99c14911,// 266 PAY 263 

    0x2a472931,// 267 PAY 264 

    0x07cce5a5,// 268 PAY 265 

    0xc80f05e3,// 269 PAY 266 

    0x85c4a973,// 270 PAY 267 

    0xa23cdc22,// 271 PAY 268 

    0xc140fbc2,// 272 PAY 269 

    0xbf62f425,// 273 PAY 270 

    0x5bd96823,// 274 PAY 271 

    0xf2e5ab45,// 275 PAY 272 

    0xf09c2ac7,// 276 PAY 273 

    0x083fd084,// 277 PAY 274 

    0x27774f88,// 278 PAY 275 

    0xa491c271,// 279 PAY 276 

    0xe7711103,// 280 PAY 277 

    0x7456aaa9,// 281 PAY 278 

    0xac4d04ed,// 282 PAY 279 

    0x4198a83f,// 283 PAY 280 

    0x204f2cc5,// 284 PAY 281 

    0x0ff26bef,// 285 PAY 282 

    0xd75e4eee,// 286 PAY 283 

    0x18e63571,// 287 PAY 284 

    0xb5eba53e,// 288 PAY 285 

    0x318567de,// 289 PAY 286 

    0x45d5eb28,// 290 PAY 287 

    0x2117e443,// 291 PAY 288 

    0xeb3474db,// 292 PAY 289 

    0xb2cdb58b,// 293 PAY 290 

    0x8f96612f,// 294 PAY 291 

    0x20fbe3aa,// 295 PAY 292 

    0xbb332ac9,// 296 PAY 293 

    0x0aa49b37,// 297 PAY 294 

    0xf9dafab1,// 298 PAY 295 

    0xe55db4cc,// 299 PAY 296 

    0x9e600a70,// 300 PAY 297 

    0xdd2a6bd1,// 301 PAY 298 

    0xd2d7f14e,// 302 PAY 299 

    0xc67c34f5,// 303 PAY 300 

    0xaa166dbc,// 304 PAY 301 

    0x640524dd,// 305 PAY 302 

    0xa47b144c,// 306 PAY 303 

    0xab4b809b,// 307 PAY 304 

    0xa2da8e78,// 308 PAY 305 

    0xf560e23e,// 309 PAY 306 

    0xd0afa329,// 310 PAY 307 

    0x71ea0bcb,// 311 PAY 308 

    0x6fa17160,// 312 PAY 309 

    0x39f7fe3f,// 313 PAY 310 

    0xec32211a,// 314 PAY 311 

    0xc922d321,// 315 PAY 312 

    0x353bba67,// 316 PAY 313 

    0x8a26c74a,// 317 PAY 314 

    0x340afb63,// 318 PAY 315 

    0x1b670d8e,// 319 PAY 316 

    0x796b0f3f,// 320 PAY 317 

    0x1c358c30,// 321 PAY 318 

    0xba187d41,// 322 PAY 319 

    0x02163283,// 323 PAY 320 

    0xd0873f0b,// 324 PAY 321 

    0xd3325c42,// 325 PAY 322 

    0x87e3d1f4,// 326 PAY 323 

    0x905c78e9,// 327 PAY 324 

    0x92f13eda,// 328 PAY 325 

    0xbc2ed5f1,// 329 PAY 326 

    0x4f81a5e3,// 330 PAY 327 

    0x099380c9,// 331 PAY 328 

    0x80373502,// 332 PAY 329 

    0xffec6106,// 333 PAY 330 

    0x8449f286,// 334 PAY 331 

    0xb041ac21,// 335 PAY 332 

    0x181dc405,// 336 PAY 333 

    0xafc2050e,// 337 PAY 334 

    0x719a6f19,// 338 PAY 335 

    0xc8c500a0,// 339 PAY 336 

    0x5f832704,// 340 PAY 337 

    0xba1d399e,// 341 PAY 338 

    0xd1285f74,// 342 PAY 339 

    0xa05d227f,// 343 PAY 340 

    0xb08d84a3,// 344 PAY 341 

    0xfeef377d,// 345 PAY 342 

    0xf743651d,// 346 PAY 343 

    0xa6853224,// 347 PAY 344 

    0x9b2150c5,// 348 PAY 345 

    0x98672e55,// 349 PAY 346 

    0xc3bc58a2,// 350 PAY 347 

    0x18511dfe,// 351 PAY 348 

    0x2c3126ab,// 352 PAY 349 

    0x6eb4b08f,// 353 PAY 350 

    0x8209c915,// 354 PAY 351 

    0xe0e0a007,// 355 PAY 352 

    0x2546e69a,// 356 PAY 353 

    0xcccfe759,// 357 PAY 354 

    0x2f96e53b,// 358 PAY 355 

    0xca2a7307,// 359 PAY 356 

    0x368cccb8,// 360 PAY 357 

    0xbee69860,// 361 PAY 358 

    0xd730f563,// 362 PAY 359 

    0xffde4831,// 363 PAY 360 

    0xee20aefa,// 364 PAY 361 

    0x450a7e2c,// 365 PAY 362 

    0x4ae5357b,// 366 PAY 363 

    0x2f170e47,// 367 PAY 364 

    0x6945a266,// 368 PAY 365 

    0x861e731e,// 369 PAY 366 

    0x4cb1cff1,// 370 PAY 367 

    0x14c84cd9,// 371 PAY 368 

    0x5c9c129d,// 372 PAY 369 

    0x1446b137,// 373 PAY 370 

    0x0e3c3237,// 374 PAY 371 

    0x04f5f8de,// 375 PAY 372 

    0x3d31eae9,// 376 PAY 373 

    0xb3713c08,// 377 PAY 374 

    0x6e738978,// 378 PAY 375 

    0x1bbecb19,// 379 PAY 376 

    0xa7619a11,// 380 PAY 377 

    0x6659d4b0,// 381 PAY 378 

    0x8352a59c,// 382 PAY 379 

    0x21a1e8e6,// 383 PAY 380 

    0x4ea3de5c,// 384 PAY 381 

    0xf81f08e3,// 385 PAY 382 

    0xecd99a2b,// 386 PAY 383 

    0x892bf19a,// 387 PAY 384 

    0x21a7b859,// 388 PAY 385 

    0x55115a5d,// 389 PAY 386 

    0xc19dbdc4,// 390 PAY 387 

    0x4abe641b,// 391 PAY 388 

    0x4898112f,// 392 PAY 389 

    0x10b01015,// 393 PAY 390 

    0x77aba804,// 394 PAY 391 

    0x4c01b3a9,// 395 PAY 392 

    0xcca7197c,// 396 PAY 393 

    0x4ecc672b,// 397 PAY 394 

    0x16dd72f6,// 398 PAY 395 

    0xc366dba2,// 399 PAY 396 

    0xcea08746,// 400 PAY 397 

    0xc0d5097d,// 401 PAY 398 

    0x594ae5cc,// 402 PAY 399 

    0xf164baa5,// 403 PAY 400 

    0x85fc693f,// 404 PAY 401 

    0xfa98741b,// 405 PAY 402 

    0x8edadb4e,// 406 PAY 403 

    0xbe56a92c,// 407 PAY 404 

    0x2db9622a,// 408 PAY 405 

    0xb1617272,// 409 PAY 406 

    0x6479856e,// 410 PAY 407 

    0x6d23c252,// 411 PAY 408 

    0x9e650953,// 412 PAY 409 

    0x23e0dad8,// 413 PAY 410 

    0x7f61f3d5,// 414 PAY 411 

    0x5721e9b0,// 415 PAY 412 

    0xffc9b905,// 416 PAY 413 

    0x677a8ccc,// 417 PAY 414 

    0xa92c6a75,// 418 PAY 415 

    0xe83a465b,// 419 PAY 416 

    0x44915dc9,// 420 PAY 417 

    0xce0041ef,// 421 PAY 418 

    0xb524071c,// 422 PAY 419 

    0xa93bd230,// 423 PAY 420 

    0x28a96c36,// 424 PAY 421 

    0x476fd5d0,// 425 PAY 422 

    0xec20ee49,// 426 PAY 423 

    0xbfa47606,// 427 PAY 424 

    0xd303ad9d,// 428 PAY 425 

    0x1d823058,// 429 PAY 426 

    0x15d9812a,// 430 PAY 427 

    0xe21209e8,// 431 PAY 428 

    0x1b157f40,// 432 PAY 429 

    0x0eeec2b5,// 433 PAY 430 

    0x9303f661,// 434 PAY 431 

    0x414ac280,// 435 PAY 432 

    0x9e3090c9,// 436 PAY 433 

    0xd778e467,// 437 PAY 434 

    0x13297f1e,// 438 PAY 435 

    0x915d796c,// 439 PAY 436 

    0xb8811bd5,// 440 PAY 437 

/// HASH is  12 bytes 

    0x46ebf169,// 441 HSH   1 

    0xd8099092,// 442 HSH   2 

    0x08d0bf55,// 443 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 34 

/// STA pkt_idx        : 231 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdf 

    0x039cdf22 // 444 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt7_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 283 words. 

/// BDA size     is 1128 (0x468) 

/// BDA id       is 0xd61 

    0x04680d61,// 3 BDA   1 

/// PAY Generic Data size   : 1128 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x91b9113c,// 4 PAY   1 

    0x12661c2b,// 5 PAY   2 

    0xabafb8df,// 6 PAY   3 

    0xf1332895,// 7 PAY   4 

    0xdc972641,// 8 PAY   5 

    0x6fc34caf,// 9 PAY   6 

    0x9830fc24,// 10 PAY   7 

    0x5773f5b9,// 11 PAY   8 

    0xf680f880,// 12 PAY   9 

    0xf925e046,// 13 PAY  10 

    0xa2f5ccfa,// 14 PAY  11 

    0xa0424b7d,// 15 PAY  12 

    0xd13c722e,// 16 PAY  13 

    0xb3707986,// 17 PAY  14 

    0x2ea01628,// 18 PAY  15 

    0xeb4a8c76,// 19 PAY  16 

    0x23e6bc29,// 20 PAY  17 

    0x5e27ecd9,// 21 PAY  18 

    0x494b38f5,// 22 PAY  19 

    0x367d7354,// 23 PAY  20 

    0x1c2edfb4,// 24 PAY  21 

    0x272e70c9,// 25 PAY  22 

    0x87595251,// 26 PAY  23 

    0xf8b17a80,// 27 PAY  24 

    0xf4e993f1,// 28 PAY  25 

    0x75f7aff6,// 29 PAY  26 

    0x72c7789d,// 30 PAY  27 

    0xb4517a8b,// 31 PAY  28 

    0x032fc4aa,// 32 PAY  29 

    0xcd443208,// 33 PAY  30 

    0xc90f87e4,// 34 PAY  31 

    0x96f44990,// 35 PAY  32 

    0x8ace8844,// 36 PAY  33 

    0x95366fac,// 37 PAY  34 

    0xe16d5793,// 38 PAY  35 

    0x596542bf,// 39 PAY  36 

    0x5d1a0419,// 40 PAY  37 

    0xa0207a3c,// 41 PAY  38 

    0x75b6c56c,// 42 PAY  39 

    0x4f3570dd,// 43 PAY  40 

    0xec9f2c53,// 44 PAY  41 

    0x7a689aa9,// 45 PAY  42 

    0x35a8904c,// 46 PAY  43 

    0xd976a5a6,// 47 PAY  44 

    0x652ee98b,// 48 PAY  45 

    0x7c5bd74f,// 49 PAY  46 

    0x0f712a39,// 50 PAY  47 

    0x05b79cc5,// 51 PAY  48 

    0xf7bbe2c5,// 52 PAY  49 

    0xd3b0b65a,// 53 PAY  50 

    0x854b18c7,// 54 PAY  51 

    0x1515cfe6,// 55 PAY  52 

    0xd54fab58,// 56 PAY  53 

    0x636978f1,// 57 PAY  54 

    0x6b9c9077,// 58 PAY  55 

    0xd65ff414,// 59 PAY  56 

    0xeea08bf3,// 60 PAY  57 

    0xf94f0beb,// 61 PAY  58 

    0x38978763,// 62 PAY  59 

    0xdd3c3524,// 63 PAY  60 

    0xc4f6b2fc,// 64 PAY  61 

    0xadaf29e3,// 65 PAY  62 

    0x87dec873,// 66 PAY  63 

    0x24dabc26,// 67 PAY  64 

    0x8d15bb60,// 68 PAY  65 

    0x5f1b8817,// 69 PAY  66 

    0x7817f0d0,// 70 PAY  67 

    0xcbbdc1ae,// 71 PAY  68 

    0x8bb19c7c,// 72 PAY  69 

    0x327b71f1,// 73 PAY  70 

    0xd429842e,// 74 PAY  71 

    0xed65811a,// 75 PAY  72 

    0x474ee4fa,// 76 PAY  73 

    0xbcadde18,// 77 PAY  74 

    0x32208299,// 78 PAY  75 

    0x80999c63,// 79 PAY  76 

    0xfcdc99bc,// 80 PAY  77 

    0x02fc5757,// 81 PAY  78 

    0xaa0b8bdf,// 82 PAY  79 

    0x6e15d8d2,// 83 PAY  80 

    0xf9e15a14,// 84 PAY  81 

    0x79e7aa90,// 85 PAY  82 

    0xae08ee94,// 86 PAY  83 

    0x6198873b,// 87 PAY  84 

    0x2148a03d,// 88 PAY  85 

    0x94598310,// 89 PAY  86 

    0xa94ef7b0,// 90 PAY  87 

    0x023d88ff,// 91 PAY  88 

    0x20342ff8,// 92 PAY  89 

    0x7f4a1a24,// 93 PAY  90 

    0x7df0b09c,// 94 PAY  91 

    0x8692ce69,// 95 PAY  92 

    0xbf0a40a1,// 96 PAY  93 

    0x083d0b23,// 97 PAY  94 

    0x43d59d06,// 98 PAY  95 

    0x21fb4f6f,// 99 PAY  96 

    0xaf4e10f5,// 100 PAY  97 

    0x70911106,// 101 PAY  98 

    0x29f2543b,// 102 PAY  99 

    0x8c7f8b82,// 103 PAY 100 

    0x0962b799,// 104 PAY 101 

    0xecd7e451,// 105 PAY 102 

    0x72d00619,// 106 PAY 103 

    0xacfad4c0,// 107 PAY 104 

    0x23a5c30c,// 108 PAY 105 

    0x0a4f40b3,// 109 PAY 106 

    0x84106c3a,// 110 PAY 107 

    0x5ec74e84,// 111 PAY 108 

    0x598d52d1,// 112 PAY 109 

    0x7085b96a,// 113 PAY 110 

    0xd9d9c360,// 114 PAY 111 

    0x2f30bb0a,// 115 PAY 112 

    0xfda75a10,// 116 PAY 113 

    0xca704888,// 117 PAY 114 

    0x2ea1e3af,// 118 PAY 115 

    0x75f43d5e,// 119 PAY 116 

    0xc1855e81,// 120 PAY 117 

    0xf94cdf22,// 121 PAY 118 

    0x4b9c517b,// 122 PAY 119 

    0x3fd896a0,// 123 PAY 120 

    0xf859ea09,// 124 PAY 121 

    0xa5f6f0d7,// 125 PAY 122 

    0x0a86c097,// 126 PAY 123 

    0x881d3629,// 127 PAY 124 

    0x62bfe186,// 128 PAY 125 

    0x11f5d9e0,// 129 PAY 126 

    0x29637845,// 130 PAY 127 

    0x9f5c9a91,// 131 PAY 128 

    0x3f61736d,// 132 PAY 129 

    0x812b5809,// 133 PAY 130 

    0x062425ee,// 134 PAY 131 

    0xd789872a,// 135 PAY 132 

    0xb658137f,// 136 PAY 133 

    0xfdb1d003,// 137 PAY 134 

    0xa7b380a9,// 138 PAY 135 

    0x1cb20220,// 139 PAY 136 

    0xbf39b03a,// 140 PAY 137 

    0x64aa589a,// 141 PAY 138 

    0x88c6a7e2,// 142 PAY 139 

    0xf4472967,// 143 PAY 140 

    0x9db8bf43,// 144 PAY 141 

    0x286cc97e,// 145 PAY 142 

    0x95927111,// 146 PAY 143 

    0x9c95eaa2,// 147 PAY 144 

    0x035c2e29,// 148 PAY 145 

    0xa60a51c1,// 149 PAY 146 

    0x9574eb3c,// 150 PAY 147 

    0x4e208add,// 151 PAY 148 

    0xe1ba63b0,// 152 PAY 149 

    0xdb2859b9,// 153 PAY 150 

    0x24ba3292,// 154 PAY 151 

    0x3135d348,// 155 PAY 152 

    0xde564db0,// 156 PAY 153 

    0x57a25908,// 157 PAY 154 

    0x88c9e4d2,// 158 PAY 155 

    0x01a88a21,// 159 PAY 156 

    0x9cb50945,// 160 PAY 157 

    0x16573211,// 161 PAY 158 

    0xfac70d66,// 162 PAY 159 

    0x40c10842,// 163 PAY 160 

    0x4c666761,// 164 PAY 161 

    0x95b2037b,// 165 PAY 162 

    0xbbc86f96,// 166 PAY 163 

    0xdbbfca5b,// 167 PAY 164 

    0x71e02d8c,// 168 PAY 165 

    0xddfd588c,// 169 PAY 166 

    0x8a9cb76b,// 170 PAY 167 

    0x6be30c32,// 171 PAY 168 

    0xd72bb40f,// 172 PAY 169 

    0x057e75e3,// 173 PAY 170 

    0x300a7218,// 174 PAY 171 

    0x9351ba6e,// 175 PAY 172 

    0xaa3cd1af,// 176 PAY 173 

    0x8dfa2fa0,// 177 PAY 174 

    0xd04a9abc,// 178 PAY 175 

    0x7e37dce4,// 179 PAY 176 

    0x669d596c,// 180 PAY 177 

    0x67b314d9,// 181 PAY 178 

    0x03699976,// 182 PAY 179 

    0x5fb775fe,// 183 PAY 180 

    0x36cd5360,// 184 PAY 181 

    0x2067ff5b,// 185 PAY 182 

    0x9ca6bbda,// 186 PAY 183 

    0x34c1e148,// 187 PAY 184 

    0x8519f293,// 188 PAY 185 

    0x24ce9d55,// 189 PAY 186 

    0xd7046315,// 190 PAY 187 

    0x60ac8c6a,// 191 PAY 188 

    0x246241bc,// 192 PAY 189 

    0x2a0be061,// 193 PAY 190 

    0xd516c4ca,// 194 PAY 191 

    0x721d5638,// 195 PAY 192 

    0xcc4ec946,// 196 PAY 193 

    0x2dbc41e2,// 197 PAY 194 

    0x523f45b5,// 198 PAY 195 

    0x15377f12,// 199 PAY 196 

    0x22576a6a,// 200 PAY 197 

    0x2cfdfb40,// 201 PAY 198 

    0xde156779,// 202 PAY 199 

    0x0c75920b,// 203 PAY 200 

    0x28dbde33,// 204 PAY 201 

    0x1fdde78f,// 205 PAY 202 

    0x6c9f39aa,// 206 PAY 203 

    0x699adc73,// 207 PAY 204 

    0x770c7670,// 208 PAY 205 

    0x3357f220,// 209 PAY 206 

    0x6910deb8,// 210 PAY 207 

    0x6151f8fa,// 211 PAY 208 

    0x8c377da9,// 212 PAY 209 

    0xe08b1594,// 213 PAY 210 

    0x8fc4a755,// 214 PAY 211 

    0x84ce301c,// 215 PAY 212 

    0x559e4e43,// 216 PAY 213 

    0x4305e1ef,// 217 PAY 214 

    0xdc0bb3af,// 218 PAY 215 

    0xded7cb01,// 219 PAY 216 

    0xf1116e96,// 220 PAY 217 

    0x3cda0ef7,// 221 PAY 218 

    0x58e163e7,// 222 PAY 219 

    0xe78135dd,// 223 PAY 220 

    0x80ab2190,// 224 PAY 221 

    0x28b46226,// 225 PAY 222 

    0xd6061dce,// 226 PAY 223 

    0x89a675eb,// 227 PAY 224 

    0xb72fd4e8,// 228 PAY 225 

    0x82eaecf6,// 229 PAY 226 

    0x6d936dc6,// 230 PAY 227 

    0xdc7c83b4,// 231 PAY 228 

    0x8292dca6,// 232 PAY 229 

    0x9f9d1990,// 233 PAY 230 

    0x2df3740a,// 234 PAY 231 

    0xf53e2d3c,// 235 PAY 232 

    0x4bd0c0ed,// 236 PAY 233 

    0xd707a290,// 237 PAY 234 

    0xd5bebaae,// 238 PAY 235 

    0x21b40250,// 239 PAY 236 

    0xc8f1c635,// 240 PAY 237 

    0x7440faf6,// 241 PAY 238 

    0xe7e66d6e,// 242 PAY 239 

    0x9851818a,// 243 PAY 240 

    0xb5e98ee1,// 244 PAY 241 

    0xea2e2505,// 245 PAY 242 

    0xbe7cf242,// 246 PAY 243 

    0xb306503a,// 247 PAY 244 

    0xe064cfd2,// 248 PAY 245 

    0x3248517b,// 249 PAY 246 

    0xbb71f92e,// 250 PAY 247 

    0xa21afa33,// 251 PAY 248 

    0x3bae1b04,// 252 PAY 249 

    0x8fbb2e05,// 253 PAY 250 

    0xc42db805,// 254 PAY 251 

    0x74fae3f9,// 255 PAY 252 

    0x9a892364,// 256 PAY 253 

    0xdf8473bf,// 257 PAY 254 

    0x09069a54,// 258 PAY 255 

    0xaa474f20,// 259 PAY 256 

    0xef967f29,// 260 PAY 257 

    0xc2e38ab4,// 261 PAY 258 

    0x47292160,// 262 PAY 259 

    0x606780b1,// 263 PAY 260 

    0x4c2d2d36,// 264 PAY 261 

    0x6db53570,// 265 PAY 262 

    0xce09532d,// 266 PAY 263 

    0x8a88644b,// 267 PAY 264 

    0xac929318,// 268 PAY 265 

    0x5e276cfc,// 269 PAY 266 

    0x8854bfa2,// 270 PAY 267 

    0x0c15285b,// 271 PAY 268 

    0xbdd7e976,// 272 PAY 269 

    0x7f7e2d40,// 273 PAY 270 

    0x3161ce1d,// 274 PAY 271 

    0x652c2773,// 275 PAY 272 

    0xa9738506,// 276 PAY 273 

    0x5efc323a,// 277 PAY 274 

    0x68a19b5e,// 278 PAY 275 

    0xf196c9de,// 279 PAY 276 

    0xa1a4791d,// 280 PAY 277 

    0x8cadd153,// 281 PAY 278 

    0x5bef0cf2,// 282 PAY 279 

    0xf81722b9,// 283 PAY 280 

    0x8eafbb5e,// 284 PAY 281 

    0x3384a9b5,// 285 PAY 282 

/// STA is 1 words. 

/// STA num_pkts       : 140 

/// STA pkt_idx        : 203 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xee 

    0x032dee8c // 286 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt8_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 313 words. 

/// BDA size     is 1247 (0x4df) 

/// BDA id       is 0x5dfa 

    0x04df5dfa,// 3 BDA   1 

/// PAY Generic Data size   : 1247 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x9e613fc6,// 4 PAY   1 

    0xc9106f1a,// 5 PAY   2 

    0x4b2a21de,// 6 PAY   3 

    0xe880e04f,// 7 PAY   4 

    0x3182082b,// 8 PAY   5 

    0xb1d56124,// 9 PAY   6 

    0xfbd1d3dd,// 10 PAY   7 

    0x71eff672,// 11 PAY   8 

    0x830bca62,// 12 PAY   9 

    0x6fc626c3,// 13 PAY  10 

    0xffb8a0e8,// 14 PAY  11 

    0x18712a15,// 15 PAY  12 

    0x493ef91c,// 16 PAY  13 

    0x89df42ed,// 17 PAY  14 

    0x27b14441,// 18 PAY  15 

    0x021b028b,// 19 PAY  16 

    0x679b15fe,// 20 PAY  17 

    0x7e609a5a,// 21 PAY  18 

    0x4f1a680f,// 22 PAY  19 

    0x15e8faa2,// 23 PAY  20 

    0xb0d97702,// 24 PAY  21 

    0xe1dc9d08,// 25 PAY  22 

    0x3c9ed106,// 26 PAY  23 

    0x29960426,// 27 PAY  24 

    0x01733e5e,// 28 PAY  25 

    0x23e5249b,// 29 PAY  26 

    0x391eade7,// 30 PAY  27 

    0xa289ec20,// 31 PAY  28 

    0x3d52e060,// 32 PAY  29 

    0xe60e6545,// 33 PAY  30 

    0x834db967,// 34 PAY  31 

    0xdc811068,// 35 PAY  32 

    0x75a14112,// 36 PAY  33 

    0x11303d84,// 37 PAY  34 

    0x87610325,// 38 PAY  35 

    0x030a8d32,// 39 PAY  36 

    0x8852c39f,// 40 PAY  37 

    0x8698e6ba,// 41 PAY  38 

    0x262b4f8a,// 42 PAY  39 

    0xcd9c22e5,// 43 PAY  40 

    0x25e0be13,// 44 PAY  41 

    0x9c13ded9,// 45 PAY  42 

    0xa2bc8f30,// 46 PAY  43 

    0xd70d8625,// 47 PAY  44 

    0x1fa3e70b,// 48 PAY  45 

    0x1a67f926,// 49 PAY  46 

    0x310c979c,// 50 PAY  47 

    0x09f9b299,// 51 PAY  48 

    0xe4ce7f6d,// 52 PAY  49 

    0xd4e57816,// 53 PAY  50 

    0xa78c34e4,// 54 PAY  51 

    0x48d91e78,// 55 PAY  52 

    0xec0992cd,// 56 PAY  53 

    0xc170cb31,// 57 PAY  54 

    0xd710abca,// 58 PAY  55 

    0xae9450e5,// 59 PAY  56 

    0x79c995ac,// 60 PAY  57 

    0x45edf4cf,// 61 PAY  58 

    0xc8298f0d,// 62 PAY  59 

    0xd078f952,// 63 PAY  60 

    0xf66b2f97,// 64 PAY  61 

    0x971657ed,// 65 PAY  62 

    0xe1bee98d,// 66 PAY  63 

    0xddeb3839,// 67 PAY  64 

    0xe909424d,// 68 PAY  65 

    0xa6937576,// 69 PAY  66 

    0x1a742ce2,// 70 PAY  67 

    0x1b63584d,// 71 PAY  68 

    0x07d6ecc3,// 72 PAY  69 

    0x1ad0e828,// 73 PAY  70 

    0xc046a839,// 74 PAY  71 

    0xad5c63ad,// 75 PAY  72 

    0x6e0498f4,// 76 PAY  73 

    0xdc68db06,// 77 PAY  74 

    0x4e6f81f7,// 78 PAY  75 

    0x2289d39a,// 79 PAY  76 

    0x62e59c03,// 80 PAY  77 

    0x21efdf46,// 81 PAY  78 

    0x666daa99,// 82 PAY  79 

    0x36c74db0,// 83 PAY  80 

    0xd8be80a8,// 84 PAY  81 

    0xcb4c1c9e,// 85 PAY  82 

    0xb5d141eb,// 86 PAY  83 

    0x56dea846,// 87 PAY  84 

    0x1152c64e,// 88 PAY  85 

    0xf84ce48c,// 89 PAY  86 

    0x5a7abf22,// 90 PAY  87 

    0xbd883130,// 91 PAY  88 

    0x8a6d4cac,// 92 PAY  89 

    0x5c75bce0,// 93 PAY  90 

    0xb39da914,// 94 PAY  91 

    0x53f9502a,// 95 PAY  92 

    0x72fc0cec,// 96 PAY  93 

    0xf0b7f4e0,// 97 PAY  94 

    0x57bda404,// 98 PAY  95 

    0x49caf248,// 99 PAY  96 

    0x080c76f6,// 100 PAY  97 

    0xd704771d,// 101 PAY  98 

    0x18e97603,// 102 PAY  99 

    0xbdbc14e9,// 103 PAY 100 

    0x2e1d33bd,// 104 PAY 101 

    0xd99d5f3a,// 105 PAY 102 

    0x84466ec7,// 106 PAY 103 

    0x739a4cf1,// 107 PAY 104 

    0x9acc5fad,// 108 PAY 105 

    0x76071ddf,// 109 PAY 106 

    0x0ed2c5a7,// 110 PAY 107 

    0xb582882f,// 111 PAY 108 

    0xc93d8bfd,// 112 PAY 109 

    0x35e4048b,// 113 PAY 110 

    0x99f156fe,// 114 PAY 111 

    0x59ed701f,// 115 PAY 112 

    0xacd5d308,// 116 PAY 113 

    0xc352d5d0,// 117 PAY 114 

    0xec7b28cb,// 118 PAY 115 

    0xeb699384,// 119 PAY 116 

    0x87f08884,// 120 PAY 117 

    0xb1453dc1,// 121 PAY 118 

    0x25b49613,// 122 PAY 119 

    0x381b74b6,// 123 PAY 120 

    0xd836b821,// 124 PAY 121 

    0x0636ebe5,// 125 PAY 122 

    0x18fd95b7,// 126 PAY 123 

    0xc3688ac4,// 127 PAY 124 

    0x73077f55,// 128 PAY 125 

    0xc8224672,// 129 PAY 126 

    0xace3adb7,// 130 PAY 127 

    0x67176420,// 131 PAY 128 

    0x12780302,// 132 PAY 129 

    0xa96c2719,// 133 PAY 130 

    0x7dcb06cc,// 134 PAY 131 

    0x71039b8d,// 135 PAY 132 

    0x0c2edbb1,// 136 PAY 133 

    0x3955df1b,// 137 PAY 134 

    0x16f4e577,// 138 PAY 135 

    0xafe6bd41,// 139 PAY 136 

    0x60f12995,// 140 PAY 137 

    0xe7e4f8ff,// 141 PAY 138 

    0x89e5e43b,// 142 PAY 139 

    0x4d9dc067,// 143 PAY 140 

    0x2cb70083,// 144 PAY 141 

    0xd814fd3e,// 145 PAY 142 

    0x0ec62343,// 146 PAY 143 

    0xc0c8eb01,// 147 PAY 144 

    0x9f367c14,// 148 PAY 145 

    0x06b4fa27,// 149 PAY 146 

    0xfb70d926,// 150 PAY 147 

    0x6c6d1e4b,// 151 PAY 148 

    0x20c7dc03,// 152 PAY 149 

    0xb76282d2,// 153 PAY 150 

    0xa6e386de,// 154 PAY 151 

    0x51fe008a,// 155 PAY 152 

    0xb658313b,// 156 PAY 153 

    0xf032c6c4,// 157 PAY 154 

    0x51f98c7b,// 158 PAY 155 

    0x5b38b541,// 159 PAY 156 

    0xb0e0e6a4,// 160 PAY 157 

    0x16747b4b,// 161 PAY 158 

    0x581098c3,// 162 PAY 159 

    0x091a7dbb,// 163 PAY 160 

    0x21206286,// 164 PAY 161 

    0x2bfcb130,// 165 PAY 162 

    0x90f6b16b,// 166 PAY 163 

    0x7d2532ad,// 167 PAY 164 

    0x952fb801,// 168 PAY 165 

    0xf05f27a3,// 169 PAY 166 

    0xe203a3cc,// 170 PAY 167 

    0x3af05450,// 171 PAY 168 

    0x3d3497d3,// 172 PAY 169 

    0x88cf9143,// 173 PAY 170 

    0x7f59aa97,// 174 PAY 171 

    0xe3caf8a7,// 175 PAY 172 

    0x393f66b2,// 176 PAY 173 

    0xddefea42,// 177 PAY 174 

    0x8aae8f92,// 178 PAY 175 

    0x6fca552f,// 179 PAY 176 

    0x323f6395,// 180 PAY 177 

    0x50ad7c67,// 181 PAY 178 

    0xc046dabd,// 182 PAY 179 

    0xdd42371c,// 183 PAY 180 

    0x0a5fcced,// 184 PAY 181 

    0xc26e9cc2,// 185 PAY 182 

    0xcfd80174,// 186 PAY 183 

    0xfafe318a,// 187 PAY 184 

    0x211edc03,// 188 PAY 185 

    0x38175470,// 189 PAY 186 

    0x8f3ae7c9,// 190 PAY 187 

    0xdc5b1787,// 191 PAY 188 

    0x6f16548a,// 192 PAY 189 

    0xe89f9cd3,// 193 PAY 190 

    0x49908891,// 194 PAY 191 

    0x03fb2584,// 195 PAY 192 

    0x20d5b349,// 196 PAY 193 

    0xfbde5fab,// 197 PAY 194 

    0x55ddfd20,// 198 PAY 195 

    0x8945cff6,// 199 PAY 196 

    0x242b1d85,// 200 PAY 197 

    0x669971a4,// 201 PAY 198 

    0x2a1eeb1b,// 202 PAY 199 

    0x1843cf3d,// 203 PAY 200 

    0x05e0adbb,// 204 PAY 201 

    0x6e441854,// 205 PAY 202 

    0x8285792d,// 206 PAY 203 

    0xfe922c31,// 207 PAY 204 

    0x86ebc963,// 208 PAY 205 

    0x04cf465f,// 209 PAY 206 

    0xce9f5a02,// 210 PAY 207 

    0xe7fd9226,// 211 PAY 208 

    0x0ad9ee69,// 212 PAY 209 

    0x9f598e83,// 213 PAY 210 

    0x9c4e66cb,// 214 PAY 211 

    0xef26feed,// 215 PAY 212 

    0xaf32caad,// 216 PAY 213 

    0x0241030f,// 217 PAY 214 

    0x9e1d59e8,// 218 PAY 215 

    0xf20c7c3b,// 219 PAY 216 

    0xa806c465,// 220 PAY 217 

    0x1a388640,// 221 PAY 218 

    0x495a3f0b,// 222 PAY 219 

    0xa1b2946e,// 223 PAY 220 

    0x6e84db7b,// 224 PAY 221 

    0xaed7c922,// 225 PAY 222 

    0x47cf11e9,// 226 PAY 223 

    0x23493f5f,// 227 PAY 224 

    0x69f419c8,// 228 PAY 225 

    0x2749a187,// 229 PAY 226 

    0x5a4347b7,// 230 PAY 227 

    0xddd65d6a,// 231 PAY 228 

    0x9210d460,// 232 PAY 229 

    0xdb11088f,// 233 PAY 230 

    0x831386f0,// 234 PAY 231 

    0x183f69e6,// 235 PAY 232 

    0x881ed6e4,// 236 PAY 233 

    0x6b653f56,// 237 PAY 234 

    0x1a6f8111,// 238 PAY 235 

    0x125dd0df,// 239 PAY 236 

    0xe674ea8a,// 240 PAY 237 

    0x9d5ed9d1,// 241 PAY 238 

    0xf270bf60,// 242 PAY 239 

    0xf86c1f88,// 243 PAY 240 

    0x432d994d,// 244 PAY 241 

    0xcd245632,// 245 PAY 242 

    0xae973ffb,// 246 PAY 243 

    0xae3976ab,// 247 PAY 244 

    0xd98802ba,// 248 PAY 245 

    0xbb966db4,// 249 PAY 246 

    0xf92e7a1a,// 250 PAY 247 

    0x9f35701c,// 251 PAY 248 

    0xc3b81626,// 252 PAY 249 

    0x6de4c956,// 253 PAY 250 

    0xbd73a86a,// 254 PAY 251 

    0x76eeda10,// 255 PAY 252 

    0x409a76e6,// 256 PAY 253 

    0x2370c406,// 257 PAY 254 

    0x2cf06569,// 258 PAY 255 

    0xd676e733,// 259 PAY 256 

    0x2d09d7db,// 260 PAY 257 

    0x58c6219e,// 261 PAY 258 

    0xce48921e,// 262 PAY 259 

    0x388c8ed7,// 263 PAY 260 

    0x6937189f,// 264 PAY 261 

    0xba92e108,// 265 PAY 262 

    0xb70ebf58,// 266 PAY 263 

    0xe262d00a,// 267 PAY 264 

    0x16ae0651,// 268 PAY 265 

    0x2bb4ab74,// 269 PAY 266 

    0xa45d3698,// 270 PAY 267 

    0x0afd1aa7,// 271 PAY 268 

    0x36c9b4a9,// 272 PAY 269 

    0xc15fc2a1,// 273 PAY 270 

    0xea161c2e,// 274 PAY 271 

    0x96d2ab0b,// 275 PAY 272 

    0x42d3e3e0,// 276 PAY 273 

    0xdf1c891a,// 277 PAY 274 

    0x50666fca,// 278 PAY 275 

    0xb69eb97e,// 279 PAY 276 

    0xc427b202,// 280 PAY 277 

    0xd55436a5,// 281 PAY 278 

    0xc044a2e6,// 282 PAY 279 

    0x6bc0e24f,// 283 PAY 280 

    0x05ae00a3,// 284 PAY 281 

    0xd069cd2a,// 285 PAY 282 

    0x9c65f476,// 286 PAY 283 

    0xfd03cb96,// 287 PAY 284 

    0x898c8fd6,// 288 PAY 285 

    0xe5094fe9,// 289 PAY 286 

    0x0c25ba47,// 290 PAY 287 

    0xcb2c94b5,// 291 PAY 288 

    0x7de7927c,// 292 PAY 289 

    0xa9aa21a2,// 293 PAY 290 

    0x6014dbac,// 294 PAY 291 

    0x31351246,// 295 PAY 292 

    0x304b649a,// 296 PAY 293 

    0x472e34b5,// 297 PAY 294 

    0x2ba30fc3,// 298 PAY 295 

    0x28452a15,// 299 PAY 296 

    0x9e10b59c,// 300 PAY 297 

    0x182e6a0a,// 301 PAY 298 

    0x2e2b4db8,// 302 PAY 299 

    0x0c2ff5d8,// 303 PAY 300 

    0x3eb23f8e,// 304 PAY 301 

    0x334f5537,// 305 PAY 302 

    0x6c9f9471,// 306 PAY 303 

    0xe6046115,// 307 PAY 304 

    0xf0c0aba8,// 308 PAY 305 

    0xfbfc4bbd,// 309 PAY 306 

    0xbf031c15,// 310 PAY 307 

    0x7fa1f758,// 311 PAY 308 

    0x3e8bf106,// 312 PAY 309 

    0x706fc565,// 313 PAY 310 

    0xb245b6b0,// 314 PAY 311 

    0xbaced500,// 315 PAY 312 

/// HASH is  4 bytes 

    0xe7e4f8ff,// 316 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 239 

/// STA pkt_idx        : 63 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x00fc1cef // 317 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt9_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 260 words. 

/// BDA size     is 1035 (0x40b) 

/// BDA id       is 0xc580 

    0x040bc580,// 3 BDA   1 

/// PAY Generic Data size   : 1035 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xb03e5da3,// 4 PAY   1 

    0xf27233f1,// 5 PAY   2 

    0xd1688303,// 6 PAY   3 

    0xd5a4e6e5,// 7 PAY   4 

    0x917fb94a,// 8 PAY   5 

    0xdf4e978b,// 9 PAY   6 

    0x6b32b2c5,// 10 PAY   7 

    0xb0970987,// 11 PAY   8 

    0xeaa832c5,// 12 PAY   9 

    0x7f0c69e3,// 13 PAY  10 

    0x9c320b42,// 14 PAY  11 

    0x60d5f5b2,// 15 PAY  12 

    0x0b67f975,// 16 PAY  13 

    0xe267efaf,// 17 PAY  14 

    0x19c44204,// 18 PAY  15 

    0xfebaa6ef,// 19 PAY  16 

    0x9defc1cb,// 20 PAY  17 

    0x52550f1a,// 21 PAY  18 

    0x92ca8566,// 22 PAY  19 

    0xaef3b83a,// 23 PAY  20 

    0xa7ae5e07,// 24 PAY  21 

    0x0f8428d6,// 25 PAY  22 

    0x8c4d2304,// 26 PAY  23 

    0x125529a0,// 27 PAY  24 

    0x2c228b74,// 28 PAY  25 

    0xa640a0f1,// 29 PAY  26 

    0x29ad4d16,// 30 PAY  27 

    0xdec9ad19,// 31 PAY  28 

    0xe6a43e37,// 32 PAY  29 

    0xb4a14e11,// 33 PAY  30 

    0x95ad3741,// 34 PAY  31 

    0x15189841,// 35 PAY  32 

    0x4a043e15,// 36 PAY  33 

    0x40b47261,// 37 PAY  34 

    0x9a241410,// 38 PAY  35 

    0x91598154,// 39 PAY  36 

    0xc2f0fb97,// 40 PAY  37 

    0xdbc6b6ee,// 41 PAY  38 

    0xf314e5c6,// 42 PAY  39 

    0x7fdea901,// 43 PAY  40 

    0x849bcc52,// 44 PAY  41 

    0x277cc372,// 45 PAY  42 

    0x88639561,// 46 PAY  43 

    0x138f9796,// 47 PAY  44 

    0x650ba079,// 48 PAY  45 

    0xefe11bd4,// 49 PAY  46 

    0xdc614875,// 50 PAY  47 

    0xd6b56d32,// 51 PAY  48 

    0x1d000eb3,// 52 PAY  49 

    0xaf92b4d6,// 53 PAY  50 

    0x6bc5497a,// 54 PAY  51 

    0xb719666d,// 55 PAY  52 

    0x19ef9618,// 56 PAY  53 

    0xec8aea68,// 57 PAY  54 

    0x70f3c731,// 58 PAY  55 

    0x33984ab4,// 59 PAY  56 

    0x14154dc8,// 60 PAY  57 

    0xa0b9f4ff,// 61 PAY  58 

    0xca68d849,// 62 PAY  59 

    0xaa31dd0c,// 63 PAY  60 

    0x4dfa42d7,// 64 PAY  61 

    0x09761b22,// 65 PAY  62 

    0x236c828f,// 66 PAY  63 

    0x1526a848,// 67 PAY  64 

    0xa22d9e15,// 68 PAY  65 

    0x36ece59f,// 69 PAY  66 

    0x19d8b86e,// 70 PAY  67 

    0xc656bbcc,// 71 PAY  68 

    0x1e9aba43,// 72 PAY  69 

    0x20ea8645,// 73 PAY  70 

    0x6402dbd0,// 74 PAY  71 

    0x741ea35e,// 75 PAY  72 

    0x79be86da,// 76 PAY  73 

    0x6aec87a7,// 77 PAY  74 

    0x94c56ad0,// 78 PAY  75 

    0x75e4b2ac,// 79 PAY  76 

    0x27cc49f8,// 80 PAY  77 

    0x7ab86750,// 81 PAY  78 

    0xc37d063d,// 82 PAY  79 

    0x54f3355a,// 83 PAY  80 

    0x81f5c57d,// 84 PAY  81 

    0xf737a763,// 85 PAY  82 

    0x6f429854,// 86 PAY  83 

    0x9480c4ee,// 87 PAY  84 

    0x2539bd4c,// 88 PAY  85 

    0x6b49d37c,// 89 PAY  86 

    0x01b34ad6,// 90 PAY  87 

    0x50e0495d,// 91 PAY  88 

    0x61bc1bb1,// 92 PAY  89 

    0xb14c0e1c,// 93 PAY  90 

    0x669b1dbc,// 94 PAY  91 

    0xc804c4cb,// 95 PAY  92 

    0xc3d10b07,// 96 PAY  93 

    0x9f2f6a0f,// 97 PAY  94 

    0x65d2457e,// 98 PAY  95 

    0x5141d781,// 99 PAY  96 

    0xf8b31267,// 100 PAY  97 

    0x1f0cc512,// 101 PAY  98 

    0x6366b112,// 102 PAY  99 

    0xacd4bf6f,// 103 PAY 100 

    0x63945765,// 104 PAY 101 

    0x7d769c64,// 105 PAY 102 

    0x5b6d9f47,// 106 PAY 103 

    0x50bdf654,// 107 PAY 104 

    0xecc2da01,// 108 PAY 105 

    0x1410db3b,// 109 PAY 106 

    0x94845df4,// 110 PAY 107 

    0xc7c82e44,// 111 PAY 108 

    0xc82d6312,// 112 PAY 109 

    0xbc367860,// 113 PAY 110 

    0xf97e9676,// 114 PAY 111 

    0xbf0ba480,// 115 PAY 112 

    0x9959aec6,// 116 PAY 113 

    0x81d4dbc7,// 117 PAY 114 

    0x35e61e31,// 118 PAY 115 

    0xccd584a5,// 119 PAY 116 

    0xc7b88e5a,// 120 PAY 117 

    0xa468eabc,// 121 PAY 118 

    0x2241c371,// 122 PAY 119 

    0x1d42d93d,// 123 PAY 120 

    0x88babd01,// 124 PAY 121 

    0xaf02c04d,// 125 PAY 122 

    0x5c96f279,// 126 PAY 123 

    0x93801ffb,// 127 PAY 124 

    0x707741c9,// 128 PAY 125 

    0x174fcaca,// 129 PAY 126 

    0x7e2fef27,// 130 PAY 127 

    0xcae18e65,// 131 PAY 128 

    0xd7310d0c,// 132 PAY 129 

    0x431b3c11,// 133 PAY 130 

    0x54fc0548,// 134 PAY 131 

    0x68df36d8,// 135 PAY 132 

    0x75aaa62a,// 136 PAY 133 

    0xd2aea238,// 137 PAY 134 

    0x6e5c111b,// 138 PAY 135 

    0x1e7b728a,// 139 PAY 136 

    0xf1055e4a,// 140 PAY 137 

    0xf454fa0f,// 141 PAY 138 

    0xde3c771c,// 142 PAY 139 

    0x67663e9d,// 143 PAY 140 

    0x2494046e,// 144 PAY 141 

    0x6068a1eb,// 145 PAY 142 

    0xbe1988f5,// 146 PAY 143 

    0xf109946a,// 147 PAY 144 

    0x8684ee85,// 148 PAY 145 

    0x255bd29e,// 149 PAY 146 

    0x5596b77f,// 150 PAY 147 

    0x0c8aa184,// 151 PAY 148 

    0x7acec6a3,// 152 PAY 149 

    0x2d6ece8b,// 153 PAY 150 

    0x8e35abd8,// 154 PAY 151 

    0x4e47a77f,// 155 PAY 152 

    0x4f1f5635,// 156 PAY 153 

    0x02eba323,// 157 PAY 154 

    0xf219ee99,// 158 PAY 155 

    0xc1166bb3,// 159 PAY 156 

    0x7c7efcec,// 160 PAY 157 

    0xc5d9acd0,// 161 PAY 158 

    0x721a1587,// 162 PAY 159 

    0x95235f1c,// 163 PAY 160 

    0x0e754f6a,// 164 PAY 161 

    0x7ac7f15c,// 165 PAY 162 

    0x1b6edf46,// 166 PAY 163 

    0x76b35c17,// 167 PAY 164 

    0xe685573c,// 168 PAY 165 

    0xaadf90fe,// 169 PAY 166 

    0xca4f4e61,// 170 PAY 167 

    0xe2f92cd9,// 171 PAY 168 

    0x968cc4d6,// 172 PAY 169 

    0x165c03e8,// 173 PAY 170 

    0x5e1c241b,// 174 PAY 171 

    0x128abe65,// 175 PAY 172 

    0xe7ba07ac,// 176 PAY 173 

    0x54791b4c,// 177 PAY 174 

    0xc718b896,// 178 PAY 175 

    0x8d2837e5,// 179 PAY 176 

    0xb2efcfcf,// 180 PAY 177 

    0xc92cbdac,// 181 PAY 178 

    0x60116439,// 182 PAY 179 

    0xeada5876,// 183 PAY 180 

    0xe383db4a,// 184 PAY 181 

    0x7134d267,// 185 PAY 182 

    0x8bf39504,// 186 PAY 183 

    0x04590152,// 187 PAY 184 

    0x2111af35,// 188 PAY 185 

    0x846fc7a5,// 189 PAY 186 

    0xbb7eb771,// 190 PAY 187 

    0xa7442f9b,// 191 PAY 188 

    0x8ba1abf4,// 192 PAY 189 

    0xd4730f32,// 193 PAY 190 

    0xec359a8e,// 194 PAY 191 

    0x741558c1,// 195 PAY 192 

    0x7d482140,// 196 PAY 193 

    0x75fc9c54,// 197 PAY 194 

    0x5209067c,// 198 PAY 195 

    0xd61f891d,// 199 PAY 196 

    0xf4061f4e,// 200 PAY 197 

    0xb023b4a4,// 201 PAY 198 

    0xd9cdd109,// 202 PAY 199 

    0x849e41e7,// 203 PAY 200 

    0x9290b5b3,// 204 PAY 201 

    0x8d1a1ac9,// 205 PAY 202 

    0x5cddc926,// 206 PAY 203 

    0x076e988d,// 207 PAY 204 

    0x26dbf318,// 208 PAY 205 

    0x1887eb25,// 209 PAY 206 

    0xd514177f,// 210 PAY 207 

    0x2c9d78d7,// 211 PAY 208 

    0x2a167582,// 212 PAY 209 

    0x0920fb7f,// 213 PAY 210 

    0x38fa8d59,// 214 PAY 211 

    0xbb5c8493,// 215 PAY 212 

    0xb5d46d5c,// 216 PAY 213 

    0x88af5e55,// 217 PAY 214 

    0x5aca8666,// 218 PAY 215 

    0x1ce86d33,// 219 PAY 216 

    0xd3271ba2,// 220 PAY 217 

    0x21e79df9,// 221 PAY 218 

    0xb8f25f64,// 222 PAY 219 

    0x40b4df1d,// 223 PAY 220 

    0xbab2fad5,// 224 PAY 221 

    0x67ec02f6,// 225 PAY 222 

    0xfb6dda31,// 226 PAY 223 

    0x291b6061,// 227 PAY 224 

    0xcf660fdb,// 228 PAY 225 

    0x70418c30,// 229 PAY 226 

    0x3b3b4cfa,// 230 PAY 227 

    0xc817ae50,// 231 PAY 228 

    0xc73ff68b,// 232 PAY 229 

    0xc1824ec9,// 233 PAY 230 

    0xd78aec57,// 234 PAY 231 

    0x5912fbdc,// 235 PAY 232 

    0xee1bf2e6,// 236 PAY 233 

    0xf452d10c,// 237 PAY 234 

    0x85914ff2,// 238 PAY 235 

    0xf46bb6ff,// 239 PAY 236 

    0xca3b02c8,// 240 PAY 237 

    0x179d3321,// 241 PAY 238 

    0x40bc7475,// 242 PAY 239 

    0x6939b2d6,// 243 PAY 240 

    0xfb346fc5,// 244 PAY 241 

    0x1d9dc3b7,// 245 PAY 242 

    0x5a276947,// 246 PAY 243 

    0xe12f0469,// 247 PAY 244 

    0xcdb142b0,// 248 PAY 245 

    0x582450bd,// 249 PAY 246 

    0x082dc4f0,// 250 PAY 247 

    0xdb61169d,// 251 PAY 248 

    0xd70d8214,// 252 PAY 249 

    0x71093e50,// 253 PAY 250 

    0xe6d46826,// 254 PAY 251 

    0x544547ce,// 255 PAY 252 

    0x4aebcdce,// 256 PAY 253 

    0xd00e4329,// 257 PAY 254 

    0x92155074,// 258 PAY 255 

    0x3c098fab,// 259 PAY 256 

    0x99a04fbf,// 260 PAY 257 

    0x215bf313,// 261 PAY 258 

    0x648f4300,// 262 PAY 259 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x71 

    0x01317149 // 263 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt10_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 188 words. 

/// BDA size     is 745 (0x2e9) 

/// BDA id       is 0xdf39 

    0x02e9df39,// 3 BDA   1 

/// PAY Generic Data size   : 745 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x21dfa479,// 4 PAY   1 

    0x3ff61a7f,// 5 PAY   2 

    0x091c03e4,// 6 PAY   3 

    0xf6b59947,// 7 PAY   4 

    0x7ecf2943,// 8 PAY   5 

    0x5b75308f,// 9 PAY   6 

    0xbd93d5d9,// 10 PAY   7 

    0xc693ce4c,// 11 PAY   8 

    0x2250d1a3,// 12 PAY   9 

    0x215cd62e,// 13 PAY  10 

    0x203fb0e0,// 14 PAY  11 

    0x10d44d56,// 15 PAY  12 

    0x600b65a5,// 16 PAY  13 

    0x0ef18eee,// 17 PAY  14 

    0x897bf6a8,// 18 PAY  15 

    0xd1b72c47,// 19 PAY  16 

    0x5cbd95bc,// 20 PAY  17 

    0xbac4a43e,// 21 PAY  18 

    0xa5b39e40,// 22 PAY  19 

    0x20b7501f,// 23 PAY  20 

    0xb55fd748,// 24 PAY  21 

    0x7a17b832,// 25 PAY  22 

    0xb0f0bff3,// 26 PAY  23 

    0x37ae3c0a,// 27 PAY  24 

    0xfc5acb2e,// 28 PAY  25 

    0x623b85bf,// 29 PAY  26 

    0x79ca7d62,// 30 PAY  27 

    0x4797b17e,// 31 PAY  28 

    0xa1013847,// 32 PAY  29 

    0x90b68566,// 33 PAY  30 

    0x251dbe84,// 34 PAY  31 

    0xbb521501,// 35 PAY  32 

    0x254209d7,// 36 PAY  33 

    0xb481ee7d,// 37 PAY  34 

    0xcca6c4f2,// 38 PAY  35 

    0x73c77860,// 39 PAY  36 

    0xccccf6ab,// 40 PAY  37 

    0x7f30b95b,// 41 PAY  38 

    0xaff7a182,// 42 PAY  39 

    0x33e5eb96,// 43 PAY  40 

    0x7c13d8ec,// 44 PAY  41 

    0x05a60ea7,// 45 PAY  42 

    0xdaccf557,// 46 PAY  43 

    0xc971e42a,// 47 PAY  44 

    0x70092fbd,// 48 PAY  45 

    0x5a378c3e,// 49 PAY  46 

    0x417f4264,// 50 PAY  47 

    0xd8047429,// 51 PAY  48 

    0x3806fba2,// 52 PAY  49 

    0xc98a737e,// 53 PAY  50 

    0x43f41260,// 54 PAY  51 

    0xa3f9307e,// 55 PAY  52 

    0xfd2943ec,// 56 PAY  53 

    0x340e78d7,// 57 PAY  54 

    0xf379c5e8,// 58 PAY  55 

    0x51e58937,// 59 PAY  56 

    0xb46807da,// 60 PAY  57 

    0xa7a46b3b,// 61 PAY  58 

    0xad12e26c,// 62 PAY  59 

    0xcb90248e,// 63 PAY  60 

    0x97414d59,// 64 PAY  61 

    0x7c605e1d,// 65 PAY  62 

    0x55bf1166,// 66 PAY  63 

    0x392cc722,// 67 PAY  64 

    0x2893d899,// 68 PAY  65 

    0xb6fa206c,// 69 PAY  66 

    0x3dd1b205,// 70 PAY  67 

    0x26a67dd2,// 71 PAY  68 

    0x8a363609,// 72 PAY  69 

    0x581cec8c,// 73 PAY  70 

    0x282624f7,// 74 PAY  71 

    0xeaf6e7eb,// 75 PAY  72 

    0xa3fbfa90,// 76 PAY  73 

    0x505f5aac,// 77 PAY  74 

    0xaa4f9896,// 78 PAY  75 

    0xbb4f8dda,// 79 PAY  76 

    0x95a95f18,// 80 PAY  77 

    0x88a3f1a3,// 81 PAY  78 

    0xbdd667bf,// 82 PAY  79 

    0xac89a848,// 83 PAY  80 

    0x74db39bd,// 84 PAY  81 

    0x5dbc50fb,// 85 PAY  82 

    0x868aaf42,// 86 PAY  83 

    0x1d46926b,// 87 PAY  84 

    0x28998aa1,// 88 PAY  85 

    0xcc8a850d,// 89 PAY  86 

    0x75a3b33c,// 90 PAY  87 

    0x34370c38,// 91 PAY  88 

    0x1007d2d7,// 92 PAY  89 

    0x5e208c02,// 93 PAY  90 

    0x9d535bfc,// 94 PAY  91 

    0x1d32e921,// 95 PAY  92 

    0xd9e33271,// 96 PAY  93 

    0x7ac1141b,// 97 PAY  94 

    0x55281aad,// 98 PAY  95 

    0x8b8ef928,// 99 PAY  96 

    0x8ead79b1,// 100 PAY  97 

    0x8f3723da,// 101 PAY  98 

    0x7dc15b2a,// 102 PAY  99 

    0xdd465fd7,// 103 PAY 100 

    0xd177c562,// 104 PAY 101 

    0x80795d2c,// 105 PAY 102 

    0x0748e006,// 106 PAY 103 

    0x27334e67,// 107 PAY 104 

    0x9d1c40c5,// 108 PAY 105 

    0xfc9647e7,// 109 PAY 106 

    0x28256c28,// 110 PAY 107 

    0x242b5c04,// 111 PAY 108 

    0xb78c30b2,// 112 PAY 109 

    0x4d7b0e42,// 113 PAY 110 

    0x06e1588d,// 114 PAY 111 

    0xc73a2744,// 115 PAY 112 

    0x388810f2,// 116 PAY 113 

    0xae418c6b,// 117 PAY 114 

    0xcfcb4cc5,// 118 PAY 115 

    0x3a9f6e87,// 119 PAY 116 

    0x43852f53,// 120 PAY 117 

    0x6e856d8c,// 121 PAY 118 

    0xb09fadd4,// 122 PAY 119 

    0xdd7dadb2,// 123 PAY 120 

    0x729433ec,// 124 PAY 121 

    0xb59e2f80,// 125 PAY 122 

    0x97187eeb,// 126 PAY 123 

    0x259d7782,// 127 PAY 124 

    0x5f4b2f78,// 128 PAY 125 

    0x7c9f9459,// 129 PAY 126 

    0xe2b32441,// 130 PAY 127 

    0xebc6bdc0,// 131 PAY 128 

    0x806333b1,// 132 PAY 129 

    0x7c482afe,// 133 PAY 130 

    0xfe960f66,// 134 PAY 131 

    0x04fb96a0,// 135 PAY 132 

    0xddbb9ffb,// 136 PAY 133 

    0x8b982a3b,// 137 PAY 134 

    0xc2f5cca7,// 138 PAY 135 

    0x408144bf,// 139 PAY 136 

    0xe325347e,// 140 PAY 137 

    0xdb2b17a5,// 141 PAY 138 

    0xdbae6bcb,// 142 PAY 139 

    0x4821ec4e,// 143 PAY 140 

    0x9051edbc,// 144 PAY 141 

    0x61643ead,// 145 PAY 142 

    0x67d23f62,// 146 PAY 143 

    0xb4ff714e,// 147 PAY 144 

    0x6d406be7,// 148 PAY 145 

    0x18ec2796,// 149 PAY 146 

    0x110211af,// 150 PAY 147 

    0x75776ad5,// 151 PAY 148 

    0x879adc70,// 152 PAY 149 

    0x29ec0cbd,// 153 PAY 150 

    0xe5d9b18a,// 154 PAY 151 

    0x6ff39f74,// 155 PAY 152 

    0x8a21d946,// 156 PAY 153 

    0x9a804654,// 157 PAY 154 

    0x868bfdd9,// 158 PAY 155 

    0x7d42706e,// 159 PAY 156 

    0x30aa3a11,// 160 PAY 157 

    0x702b87ee,// 161 PAY 158 

    0x191397e1,// 162 PAY 159 

    0xe125f8c2,// 163 PAY 160 

    0xc339efbc,// 164 PAY 161 

    0xe44a287a,// 165 PAY 162 

    0x35d22e61,// 166 PAY 163 

    0xecb449ad,// 167 PAY 164 

    0x9db764c5,// 168 PAY 165 

    0x423dc51f,// 169 PAY 166 

    0x12b31864,// 170 PAY 167 

    0xa9090c9d,// 171 PAY 168 

    0xeba6857d,// 172 PAY 169 

    0x1090116b,// 173 PAY 170 

    0x8c54df86,// 174 PAY 171 

    0x3aa14744,// 175 PAY 172 

    0x1e564b9e,// 176 PAY 173 

    0x144012f6,// 177 PAY 174 

    0xad3ff031,// 178 PAY 175 

    0x10b6df8a,// 179 PAY 176 

    0xfab6988c,// 180 PAY 177 

    0xb0f48a97,// 181 PAY 178 

    0xaa908c81,// 182 PAY 179 

    0x6b318090,// 183 PAY 180 

    0x1027b3ae,// 184 PAY 181 

    0xf6f82248,// 185 PAY 182 

    0x408bfdd5,// 186 PAY 183 

    0x05c1d5fe,// 187 PAY 184 

    0xb82dd639,// 188 PAY 185 

    0x7dd9b21b,// 189 PAY 186 

    0x87000000,// 190 PAY 187 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0359e82c // 191 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt11_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 369 words. 

/// BDA size     is 1472 (0x5c0) 

/// BDA id       is 0x906 

    0x05c00906,// 3 BDA   1 

/// PAY Generic Data size   : 1472 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x054050f8,// 4 PAY   1 

    0x752de8f0,// 5 PAY   2 

    0xcbad4284,// 6 PAY   3 

    0x027f3b8f,// 7 PAY   4 

    0x6e08d1b3,// 8 PAY   5 

    0x78641043,// 9 PAY   6 

    0xb7b11db2,// 10 PAY   7 

    0x205b506d,// 11 PAY   8 

    0xb5f7791c,// 12 PAY   9 

    0x71d7ac73,// 13 PAY  10 

    0xa605ad21,// 14 PAY  11 

    0x1e7326e7,// 15 PAY  12 

    0xbd45ba51,// 16 PAY  13 

    0x9b2e7081,// 17 PAY  14 

    0xf36a3d08,// 18 PAY  15 

    0x13c465ff,// 19 PAY  16 

    0x7bf94f14,// 20 PAY  17 

    0x436db5cb,// 21 PAY  18 

    0x3cea67e6,// 22 PAY  19 

    0x0c4e3d93,// 23 PAY  20 

    0x96510e8e,// 24 PAY  21 

    0x46ff83a1,// 25 PAY  22 

    0xc9a5df06,// 26 PAY  23 

    0x2ae9f275,// 27 PAY  24 

    0x883de9f3,// 28 PAY  25 

    0xcfc11ba8,// 29 PAY  26 

    0xf031c5f8,// 30 PAY  27 

    0xf6ce2774,// 31 PAY  28 

    0x8d78b1cb,// 32 PAY  29 

    0x3b3befd1,// 33 PAY  30 

    0x6e93b9dd,// 34 PAY  31 

    0x327a5b4c,// 35 PAY  32 

    0xa43f3a97,// 36 PAY  33 

    0x1a1caa8f,// 37 PAY  34 

    0xa8682e9c,// 38 PAY  35 

    0x890552f8,// 39 PAY  36 

    0x73260d03,// 40 PAY  37 

    0x54f979a0,// 41 PAY  38 

    0x90b0dbaf,// 42 PAY  39 

    0xda810f0c,// 43 PAY  40 

    0x23511260,// 44 PAY  41 

    0x55a7424f,// 45 PAY  42 

    0x5e6c73b3,// 46 PAY  43 

    0x384cef42,// 47 PAY  44 

    0x2d63d83e,// 48 PAY  45 

    0x5f5212a3,// 49 PAY  46 

    0x7b306669,// 50 PAY  47 

    0x6ab0af8d,// 51 PAY  48 

    0xd456c3c0,// 52 PAY  49 

    0x2fad6279,// 53 PAY  50 

    0x7a374d22,// 54 PAY  51 

    0x06978533,// 55 PAY  52 

    0xcb3255bb,// 56 PAY  53 

    0xdca531f4,// 57 PAY  54 

    0xb3ff11d0,// 58 PAY  55 

    0xafa281e6,// 59 PAY  56 

    0x95b68e4e,// 60 PAY  57 

    0xa7322a7d,// 61 PAY  58 

    0x2fb90315,// 62 PAY  59 

    0x32492be3,// 63 PAY  60 

    0x0d5193c5,// 64 PAY  61 

    0x42e37ff2,// 65 PAY  62 

    0xa896ef3c,// 66 PAY  63 

    0x2eda34e0,// 67 PAY  64 

    0x80c57db2,// 68 PAY  65 

    0x73a62440,// 69 PAY  66 

    0x97a59904,// 70 PAY  67 

    0xda4b990a,// 71 PAY  68 

    0xc20c764b,// 72 PAY  69 

    0x0d03f0bd,// 73 PAY  70 

    0xe167a0d6,// 74 PAY  71 

    0xec2b0e57,// 75 PAY  72 

    0xc379a17a,// 76 PAY  73 

    0x3bacbe89,// 77 PAY  74 

    0x7eaa7ade,// 78 PAY  75 

    0xaa102562,// 79 PAY  76 

    0xf95478dc,// 80 PAY  77 

    0x6bcd9ccd,// 81 PAY  78 

    0x21adb193,// 82 PAY  79 

    0x6c93a565,// 83 PAY  80 

    0x166f7124,// 84 PAY  81 

    0xecd272c4,// 85 PAY  82 

    0xe0933108,// 86 PAY  83 

    0xd6880d1d,// 87 PAY  84 

    0xc4105c9b,// 88 PAY  85 

    0x277432c2,// 89 PAY  86 

    0xc30a8b78,// 90 PAY  87 

    0x9ced9b95,// 91 PAY  88 

    0x1446d2a9,// 92 PAY  89 

    0x9560c5c1,// 93 PAY  90 

    0xb1656469,// 94 PAY  91 

    0x6585fc3a,// 95 PAY  92 

    0x5a3dfe46,// 96 PAY  93 

    0xbd60cd8c,// 97 PAY  94 

    0x1a8a0cfb,// 98 PAY  95 

    0xe1a72da2,// 99 PAY  96 

    0x416c8514,// 100 PAY  97 

    0x2e932ee9,// 101 PAY  98 

    0xcb5ed8a0,// 102 PAY  99 

    0xb38cce61,// 103 PAY 100 

    0xbeb0a704,// 104 PAY 101 

    0x41b0f536,// 105 PAY 102 

    0x54ff7bee,// 106 PAY 103 

    0x950e5313,// 107 PAY 104 

    0xf503b557,// 108 PAY 105 

    0xdacba06c,// 109 PAY 106 

    0x4a4c4e0b,// 110 PAY 107 

    0x35a841bd,// 111 PAY 108 

    0xde508424,// 112 PAY 109 

    0x2fab6984,// 113 PAY 110 

    0x7d6db8ee,// 114 PAY 111 

    0x0e7d04a4,// 115 PAY 112 

    0x54d3042a,// 116 PAY 113 

    0xa08c504f,// 117 PAY 114 

    0x7737506a,// 118 PAY 115 

    0x32fcc2d9,// 119 PAY 116 

    0xbdcdcd1f,// 120 PAY 117 

    0x3a29edda,// 121 PAY 118 

    0xc9c85418,// 122 PAY 119 

    0x585741b1,// 123 PAY 120 

    0x94b9c742,// 124 PAY 121 

    0xbb6c2553,// 125 PAY 122 

    0xaff12e29,// 126 PAY 123 

    0x2ae46f42,// 127 PAY 124 

    0xa0ff06df,// 128 PAY 125 

    0x4343994b,// 129 PAY 126 

    0xdd131482,// 130 PAY 127 

    0x7ba9e498,// 131 PAY 128 

    0x73a51dbb,// 132 PAY 129 

    0xa3e8a7b8,// 133 PAY 130 

    0xa062be02,// 134 PAY 131 

    0x5381da81,// 135 PAY 132 

    0x9ec04b8b,// 136 PAY 133 

    0xca2b3e22,// 137 PAY 134 

    0x24b14d45,// 138 PAY 135 

    0xf1e9aa44,// 139 PAY 136 

    0x72c8069f,// 140 PAY 137 

    0x10aa87af,// 141 PAY 138 

    0xfa00f9b6,// 142 PAY 139 

    0x126c6630,// 143 PAY 140 

    0x187af8d8,// 144 PAY 141 

    0xcb6e5ad3,// 145 PAY 142 

    0x9403e84b,// 146 PAY 143 

    0x4b69121e,// 147 PAY 144 

    0x84c385f6,// 148 PAY 145 

    0xad0277ee,// 149 PAY 146 

    0xb4ddd5ef,// 150 PAY 147 

    0x8bba353c,// 151 PAY 148 

    0x3e7dec84,// 152 PAY 149 

    0x010b0da9,// 153 PAY 150 

    0xc8088058,// 154 PAY 151 

    0x25a81c0b,// 155 PAY 152 

    0xbda826c4,// 156 PAY 153 

    0x4e7c21cc,// 157 PAY 154 

    0x73f3d61d,// 158 PAY 155 

    0xe739ae09,// 159 PAY 156 

    0x7791aabb,// 160 PAY 157 

    0xd9fc40dd,// 161 PAY 158 

    0x69a4dbc9,// 162 PAY 159 

    0x88fffb9e,// 163 PAY 160 

    0x6d8d5609,// 164 PAY 161 

    0xfd308f38,// 165 PAY 162 

    0x10b1911e,// 166 PAY 163 

    0xc8c294d6,// 167 PAY 164 

    0xc4bbb7e5,// 168 PAY 165 

    0xd967c849,// 169 PAY 166 

    0x88e750e4,// 170 PAY 167 

    0xadbd7f91,// 171 PAY 168 

    0x7efd05dd,// 172 PAY 169 

    0x866efb02,// 173 PAY 170 

    0x12391a15,// 174 PAY 171 

    0xf55eb464,// 175 PAY 172 

    0x317e0f29,// 176 PAY 173 

    0xf25d5037,// 177 PAY 174 

    0x71a833b6,// 178 PAY 175 

    0x1648e696,// 179 PAY 176 

    0xcb9949a1,// 180 PAY 177 

    0xe8f83439,// 181 PAY 178 

    0xa06cc068,// 182 PAY 179 

    0x3b0c095c,// 183 PAY 180 

    0x6cc70313,// 184 PAY 181 

    0xacab14c6,// 185 PAY 182 

    0xfd51b6bf,// 186 PAY 183 

    0x08754a4b,// 187 PAY 184 

    0xd4615f45,// 188 PAY 185 

    0x998ecf25,// 189 PAY 186 

    0xc8d8fd59,// 190 PAY 187 

    0xab0fb83b,// 191 PAY 188 

    0x4e8ac1c6,// 192 PAY 189 

    0x96fee4d9,// 193 PAY 190 

    0x7d8fc21e,// 194 PAY 191 

    0x22627f0e,// 195 PAY 192 

    0xcd90384b,// 196 PAY 193 

    0xb5b66c99,// 197 PAY 194 

    0x4c1061f8,// 198 PAY 195 

    0xb068a0ee,// 199 PAY 196 

    0x08fef841,// 200 PAY 197 

    0xfaf8a575,// 201 PAY 198 

    0xb813de9c,// 202 PAY 199 

    0xb3618e06,// 203 PAY 200 

    0xcb05bda7,// 204 PAY 201 

    0xf48e760e,// 205 PAY 202 

    0xe389c79a,// 206 PAY 203 

    0x8d53bbf1,// 207 PAY 204 

    0x94f471eb,// 208 PAY 205 

    0x9653551d,// 209 PAY 206 

    0x33dec2b6,// 210 PAY 207 

    0xd6c2dfef,// 211 PAY 208 

    0xd826ef6a,// 212 PAY 209 

    0x2249f93c,// 213 PAY 210 

    0x8cfdef87,// 214 PAY 211 

    0x896a9652,// 215 PAY 212 

    0x1116aa46,// 216 PAY 213 

    0x4020cbc9,// 217 PAY 214 

    0x9687cbb1,// 218 PAY 215 

    0x9958aabe,// 219 PAY 216 

    0xadad8167,// 220 PAY 217 

    0xd71e9aa8,// 221 PAY 218 

    0x93518a69,// 222 PAY 219 

    0xf494939c,// 223 PAY 220 

    0x0237e022,// 224 PAY 221 

    0xedd14f98,// 225 PAY 222 

    0x85c08de7,// 226 PAY 223 

    0xe4e59b7d,// 227 PAY 224 

    0xe3925297,// 228 PAY 225 

    0xaa0a91cd,// 229 PAY 226 

    0x9128a168,// 230 PAY 227 

    0xe6abc90f,// 231 PAY 228 

    0x86a6052c,// 232 PAY 229 

    0xb47342aa,// 233 PAY 230 

    0xe3b106b0,// 234 PAY 231 

    0x7671395b,// 235 PAY 232 

    0xe0d0d9c7,// 236 PAY 233 

    0x67edc5af,// 237 PAY 234 

    0x405d83cf,// 238 PAY 235 

    0x9d3942a0,// 239 PAY 236 

    0x5258282a,// 240 PAY 237 

    0x51732f05,// 241 PAY 238 

    0xe363d5d8,// 242 PAY 239 

    0xb826efa5,// 243 PAY 240 

    0xb3a95439,// 244 PAY 241 

    0x881fa2a4,// 245 PAY 242 

    0xa04d2df1,// 246 PAY 243 

    0x5a8dc412,// 247 PAY 244 

    0xaded396b,// 248 PAY 245 

    0xc22ce556,// 249 PAY 246 

    0xd6644481,// 250 PAY 247 

    0x0c6531de,// 251 PAY 248 

    0xef7dd1d8,// 252 PAY 249 

    0x387276fa,// 253 PAY 250 

    0x618f2c95,// 254 PAY 251 

    0xbb84c89a,// 255 PAY 252 

    0xa69f8a61,// 256 PAY 253 

    0x714e2fef,// 257 PAY 254 

    0xcd719bf0,// 258 PAY 255 

    0xf538704d,// 259 PAY 256 

    0x9448a395,// 260 PAY 257 

    0x88e06ce4,// 261 PAY 258 

    0x1ade376a,// 262 PAY 259 

    0x5df95e5d,// 263 PAY 260 

    0x94bd1ce2,// 264 PAY 261 

    0x7201d5a3,// 265 PAY 262 

    0x97f496fc,// 266 PAY 263 

    0x5b3d9525,// 267 PAY 264 

    0xddf2f8ac,// 268 PAY 265 

    0x8976c38e,// 269 PAY 266 

    0xee16a877,// 270 PAY 267 

    0xd95d774f,// 271 PAY 268 

    0x793ebd99,// 272 PAY 269 

    0x0600857e,// 273 PAY 270 

    0xe90c9640,// 274 PAY 271 

    0x0d723895,// 275 PAY 272 

    0x160f5cee,// 276 PAY 273 

    0xd0a3599d,// 277 PAY 274 

    0x7351462a,// 278 PAY 275 

    0x76b7e299,// 279 PAY 276 

    0x13c5dd27,// 280 PAY 277 

    0x8eb11bf7,// 281 PAY 278 

    0xc9ce8933,// 282 PAY 279 

    0x2337fb2b,// 283 PAY 280 

    0x62896df6,// 284 PAY 281 

    0x7d91e9ad,// 285 PAY 282 

    0x4dadf760,// 286 PAY 283 

    0xc0724ac2,// 287 PAY 284 

    0x2d102bb6,// 288 PAY 285 

    0xdb46a07f,// 289 PAY 286 

    0x1eaa06f1,// 290 PAY 287 

    0x91692df1,// 291 PAY 288 

    0xf7eab869,// 292 PAY 289 

    0xc3b20574,// 293 PAY 290 

    0xe19bf4c4,// 294 PAY 291 

    0xc15da5c3,// 295 PAY 292 

    0x24b7f79d,// 296 PAY 293 

    0xf1d5ab72,// 297 PAY 294 

    0x016d8988,// 298 PAY 295 

    0x6373c78b,// 299 PAY 296 

    0xe41ad8a2,// 300 PAY 297 

    0x50c570a1,// 301 PAY 298 

    0x9c557c3e,// 302 PAY 299 

    0x9082c806,// 303 PAY 300 

    0xbf25879f,// 304 PAY 301 

    0x5bd1cbb8,// 305 PAY 302 

    0xcbf8bee0,// 306 PAY 303 

    0xb3e2b971,// 307 PAY 304 

    0x10d38568,// 308 PAY 305 

    0x3dc947ba,// 309 PAY 306 

    0x536fc5d0,// 310 PAY 307 

    0x1ce7201e,// 311 PAY 308 

    0xe1cf9e0d,// 312 PAY 309 

    0x4a30d65c,// 313 PAY 310 

    0x4d199c2f,// 314 PAY 311 

    0x4c623984,// 315 PAY 312 

    0xaba96ccc,// 316 PAY 313 

    0xd8c6ac42,// 317 PAY 314 

    0x6f825c19,// 318 PAY 315 

    0xa0ef5588,// 319 PAY 316 

    0x1f324b30,// 320 PAY 317 

    0x3e48bae0,// 321 PAY 318 

    0x524fbfb5,// 322 PAY 319 

    0x4870fe17,// 323 PAY 320 

    0x039ebc44,// 324 PAY 321 

    0x7d9908db,// 325 PAY 322 

    0x73d0b925,// 326 PAY 323 

    0x80236510,// 327 PAY 324 

    0xf808c71a,// 328 PAY 325 

    0x4c02e872,// 329 PAY 326 

    0x3eb71bb0,// 330 PAY 327 

    0x8d970c7a,// 331 PAY 328 

    0x60411262,// 332 PAY 329 

    0x009d0204,// 333 PAY 330 

    0xb019451f,// 334 PAY 331 

    0x6d51377e,// 335 PAY 332 

    0xda649e2f,// 336 PAY 333 

    0x88d2e594,// 337 PAY 334 

    0x53eefeb0,// 338 PAY 335 

    0xe7ef110f,// 339 PAY 336 

    0x8b09a8b7,// 340 PAY 337 

    0xbb1611f6,// 341 PAY 338 

    0x3352a938,// 342 PAY 339 

    0x3954c5ef,// 343 PAY 340 

    0x4ffeb2ae,// 344 PAY 341 

    0x982779ed,// 345 PAY 342 

    0x176b023a,// 346 PAY 343 

    0x5f24552b,// 347 PAY 344 

    0x9543135b,// 348 PAY 345 

    0x47a29bf5,// 349 PAY 346 

    0x629ab7b5,// 350 PAY 347 

    0x05960861,// 351 PAY 348 

    0xce5fd20f,// 352 PAY 349 

    0x2d67bc6c,// 353 PAY 350 

    0x998308e9,// 354 PAY 351 

    0x4ae3ec89,// 355 PAY 352 

    0x7e2e0e42,// 356 PAY 353 

    0x74189848,// 357 PAY 354 

    0xcb6e2041,// 358 PAY 355 

    0xb816ae5e,// 359 PAY 356 

    0x9fad92d0,// 360 PAY 357 

    0x5c92bd67,// 361 PAY 358 

    0x1b192721,// 362 PAY 359 

    0xeb93652f,// 363 PAY 360 

    0xc104bcb7,// 364 PAY 361 

    0xe9c9975e,// 365 PAY 362 

    0x2481f8a0,// 366 PAY 363 

    0xb110d244,// 367 PAY 364 

    0x9843be5e,// 368 PAY 365 

    0x405eb80d,// 369 PAY 366 

    0x5385c5b9,// 370 PAY 367 

    0xbd466b20,// 371 PAY 368 

/// STA is 1 words. 

/// STA num_pkts       : 10 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7c 

    0x03607c0a // 372 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt12_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 173 words. 

/// BDA size     is 688 (0x2b0) 

/// BDA id       is 0x79ee 

    0x02b079ee,// 3 BDA   1 

/// PAY Generic Data size   : 688 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x0af21026,// 4 PAY   1 

    0x46781300,// 5 PAY   2 

    0x735a9290,// 6 PAY   3 

    0x5b7edd51,// 7 PAY   4 

    0x4c0e5418,// 8 PAY   5 

    0x10e59510,// 9 PAY   6 

    0x59845a3b,// 10 PAY   7 

    0x8f2324ae,// 11 PAY   8 

    0x6d7ab3ce,// 12 PAY   9 

    0x58e1cef5,// 13 PAY  10 

    0x68761e67,// 14 PAY  11 

    0x2b4eec92,// 15 PAY  12 

    0x426fcc4c,// 16 PAY  13 

    0xbee6b78a,// 17 PAY  14 

    0x91aa1488,// 18 PAY  15 

    0x05bd1c21,// 19 PAY  16 

    0x5279530c,// 20 PAY  17 

    0x2edbd840,// 21 PAY  18 

    0xd4112095,// 22 PAY  19 

    0xb186ba7a,// 23 PAY  20 

    0x8de434d2,// 24 PAY  21 

    0xac05de5e,// 25 PAY  22 

    0xfebf5d29,// 26 PAY  23 

    0xe49c37f8,// 27 PAY  24 

    0x0db2beb0,// 28 PAY  25 

    0xf02b150e,// 29 PAY  26 

    0x483d022b,// 30 PAY  27 

    0xe58b6da0,// 31 PAY  28 

    0x5165adda,// 32 PAY  29 

    0xc5bd2f3f,// 33 PAY  30 

    0xc9582342,// 34 PAY  31 

    0x40fbf075,// 35 PAY  32 

    0x428b2ad5,// 36 PAY  33 

    0x35f1e372,// 37 PAY  34 

    0x16c36ff7,// 38 PAY  35 

    0xf8b7b17d,// 39 PAY  36 

    0xf709b320,// 40 PAY  37 

    0x115765cf,// 41 PAY  38 

    0xcb6f495c,// 42 PAY  39 

    0xf1216ed2,// 43 PAY  40 

    0x758d44e5,// 44 PAY  41 

    0x1ed5d16c,// 45 PAY  42 

    0xd98e4d50,// 46 PAY  43 

    0x1dd55082,// 47 PAY  44 

    0x7ae6b9ef,// 48 PAY  45 

    0xaa96685e,// 49 PAY  46 

    0xcafdf3a3,// 50 PAY  47 

    0xf4440105,// 51 PAY  48 

    0x6e5e4540,// 52 PAY  49 

    0x22a876e5,// 53 PAY  50 

    0x40b7ed19,// 54 PAY  51 

    0xa93125e3,// 55 PAY  52 

    0x5825caeb,// 56 PAY  53 

    0x9906934e,// 57 PAY  54 

    0xf4b8fade,// 58 PAY  55 

    0xbb277792,// 59 PAY  56 

    0x92866d98,// 60 PAY  57 

    0x9681c03a,// 61 PAY  58 

    0xa509da4a,// 62 PAY  59 

    0x78b3427d,// 63 PAY  60 

    0x9cc6e5a3,// 64 PAY  61 

    0xc2743061,// 65 PAY  62 

    0xac4ea492,// 66 PAY  63 

    0xcb76515b,// 67 PAY  64 

    0xf12b8022,// 68 PAY  65 

    0xd2257034,// 69 PAY  66 

    0xb2422ac6,// 70 PAY  67 

    0x7973c319,// 71 PAY  68 

    0xab06b056,// 72 PAY  69 

    0xc5bfd1f2,// 73 PAY  70 

    0x174b83ca,// 74 PAY  71 

    0x9b3f8d53,// 75 PAY  72 

    0xae881a5d,// 76 PAY  73 

    0x53d10c08,// 77 PAY  74 

    0xce71ef1a,// 78 PAY  75 

    0xf107f9db,// 79 PAY  76 

    0xe8af6224,// 80 PAY  77 

    0xdedc23a7,// 81 PAY  78 

    0x1fb7707b,// 82 PAY  79 

    0xc2331d27,// 83 PAY  80 

    0xec719594,// 84 PAY  81 

    0x09b735aa,// 85 PAY  82 

    0xbc677e2e,// 86 PAY  83 

    0x935f5d0d,// 87 PAY  84 

    0x6bbe7d15,// 88 PAY  85 

    0x7c796385,// 89 PAY  86 

    0x998556df,// 90 PAY  87 

    0xcbd99982,// 91 PAY  88 

    0x6210d0a3,// 92 PAY  89 

    0xd4157a63,// 93 PAY  90 

    0x42c8b81c,// 94 PAY  91 

    0x36bb31a9,// 95 PAY  92 

    0xc25808d8,// 96 PAY  93 

    0x0a529739,// 97 PAY  94 

    0xa07a2447,// 98 PAY  95 

    0x5204d480,// 99 PAY  96 

    0xa555113d,// 100 PAY  97 

    0xa530ed94,// 101 PAY  98 

    0x7e23b79a,// 102 PAY  99 

    0x99620c5a,// 103 PAY 100 

    0x032b5baa,// 104 PAY 101 

    0xb447c365,// 105 PAY 102 

    0x90db7311,// 106 PAY 103 

    0x3177b104,// 107 PAY 104 

    0x16fbbf9f,// 108 PAY 105 

    0xc6d0808c,// 109 PAY 106 

    0xf3aa5550,// 110 PAY 107 

    0x015a026e,// 111 PAY 108 

    0x279afd78,// 112 PAY 109 

    0x2964600b,// 113 PAY 110 

    0x20626202,// 114 PAY 111 

    0x9f3c0606,// 115 PAY 112 

    0xe129eb2d,// 116 PAY 113 

    0x207292e0,// 117 PAY 114 

    0x6a89ce8e,// 118 PAY 115 

    0x3b857df8,// 119 PAY 116 

    0x5e0d5866,// 120 PAY 117 

    0x9a7df2bf,// 121 PAY 118 

    0xcef3faa9,// 122 PAY 119 

    0x0d8b9537,// 123 PAY 120 

    0x30e1da9a,// 124 PAY 121 

    0x0bf461a9,// 125 PAY 122 

    0x0499e119,// 126 PAY 123 

    0x259fb9cb,// 127 PAY 124 

    0xb4bef5be,// 128 PAY 125 

    0x755f5495,// 129 PAY 126 

    0xd3c9e7b6,// 130 PAY 127 

    0xe5f17410,// 131 PAY 128 

    0xff2a6658,// 132 PAY 129 

    0x50868a22,// 133 PAY 130 

    0x01fda41e,// 134 PAY 131 

    0x4e08de0e,// 135 PAY 132 

    0x036c1baf,// 136 PAY 133 

    0x41dd44d2,// 137 PAY 134 

    0x3c67a8c9,// 138 PAY 135 

    0x502521c7,// 139 PAY 136 

    0x9168bf1c,// 140 PAY 137 

    0x3cf26144,// 141 PAY 138 

    0x1361b492,// 142 PAY 139 

    0x7c80dc35,// 143 PAY 140 

    0x80110f86,// 144 PAY 141 

    0x8331da77,// 145 PAY 142 

    0xee9ab219,// 146 PAY 143 

    0xe3d98ea7,// 147 PAY 144 

    0x9b13e366,// 148 PAY 145 

    0xd5c5963a,// 149 PAY 146 

    0xaffcb31b,// 150 PAY 147 

    0x4f7274b7,// 151 PAY 148 

    0xa0dbfe69,// 152 PAY 149 

    0xf30d0793,// 153 PAY 150 

    0xce4a92f5,// 154 PAY 151 

    0xdef4ba48,// 155 PAY 152 

    0x27109ead,// 156 PAY 153 

    0x961191e4,// 157 PAY 154 

    0xc71d0b70,// 158 PAY 155 

    0xa1ef4206,// 159 PAY 156 

    0xdc5fba0e,// 160 PAY 157 

    0xe1ecb8a7,// 161 PAY 158 

    0xae1663b5,// 162 PAY 159 

    0x33959d47,// 163 PAY 160 

    0xa0020fce,// 164 PAY 161 

    0x43ccfad2,// 165 PAY 162 

    0x433472f9,// 166 PAY 163 

    0xae1193bc,// 167 PAY 164 

    0xf45d6525,// 168 PAY 165 

    0xf0254d3a,// 169 PAY 166 

    0x1b1bdb8f,// 170 PAY 167 

    0xbb7dcde2,// 171 PAY 168 

    0xcb8b1c72,// 172 PAY 169 

    0xcada00e2,// 173 PAY 170 

    0x07ef5cf6,// 174 PAY 171 

    0x90f819b5,// 175 PAY 172 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 70 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcf 

    0x0119cf74 // 176 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt13_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 429 words. 

/// BDA size     is 1709 (0x6ad) 

/// BDA id       is 0x91a 

    0x06ad091a,// 3 BDA   1 

/// PAY Generic Data size   : 1709 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x217f8f27,// 4 PAY   1 

    0x49d2a6ff,// 5 PAY   2 

    0xc83b2f92,// 6 PAY   3 

    0x35bcb58f,// 7 PAY   4 

    0x5e117217,// 8 PAY   5 

    0xf52bb811,// 9 PAY   6 

    0xed76b3f4,// 10 PAY   7 

    0xf7138d47,// 11 PAY   8 

    0xec368332,// 12 PAY   9 

    0xcfb6cd39,// 13 PAY  10 

    0x1cf2d476,// 14 PAY  11 

    0x971a681e,// 15 PAY  12 

    0x209e7e7c,// 16 PAY  13 

    0xe54816fb,// 17 PAY  14 

    0x2b7c8ca3,// 18 PAY  15 

    0x75515b15,// 19 PAY  16 

    0xd6788929,// 20 PAY  17 

    0xc7c3c436,// 21 PAY  18 

    0xf3241fb1,// 22 PAY  19 

    0x95a69561,// 23 PAY  20 

    0xe13b129a,// 24 PAY  21 

    0x5d3d4189,// 25 PAY  22 

    0x7ec298d3,// 26 PAY  23 

    0x2395f052,// 27 PAY  24 

    0x1c9f1ba5,// 28 PAY  25 

    0x4330091c,// 29 PAY  26 

    0xb417f212,// 30 PAY  27 

    0x94cfa75e,// 31 PAY  28 

    0xfe931f1d,// 32 PAY  29 

    0xb6a9844e,// 33 PAY  30 

    0x346c6f11,// 34 PAY  31 

    0xc2d50b8f,// 35 PAY  32 

    0x97b1acaa,// 36 PAY  33 

    0x12a837d2,// 37 PAY  34 

    0x2d5148fa,// 38 PAY  35 

    0x9b7c69d3,// 39 PAY  36 

    0x04c38b92,// 40 PAY  37 

    0x7e2674ac,// 41 PAY  38 

    0x7decebd0,// 42 PAY  39 

    0x12b813ff,// 43 PAY  40 

    0xe71aacfd,// 44 PAY  41 

    0xdace2528,// 45 PAY  42 

    0xfe4ec648,// 46 PAY  43 

    0x28efbd93,// 47 PAY  44 

    0x894d4715,// 48 PAY  45 

    0x80670c8a,// 49 PAY  46 

    0x3d359da1,// 50 PAY  47 

    0x505c2303,// 51 PAY  48 

    0xe8f15ec1,// 52 PAY  49 

    0xf63616db,// 53 PAY  50 

    0x887ca7c6,// 54 PAY  51 

    0xea0b99e8,// 55 PAY  52 

    0x4dfc9e10,// 56 PAY  53 

    0xdaa06570,// 57 PAY  54 

    0xf990b1f9,// 58 PAY  55 

    0x7f28a01f,// 59 PAY  56 

    0x9d1642a5,// 60 PAY  57 

    0x742bdd03,// 61 PAY  58 

    0x139d793d,// 62 PAY  59 

    0x89f747c9,// 63 PAY  60 

    0x0fcf5dc7,// 64 PAY  61 

    0x367d465f,// 65 PAY  62 

    0x17f0044b,// 66 PAY  63 

    0xc444f68c,// 67 PAY  64 

    0xb8c8c32c,// 68 PAY  65 

    0xce52a481,// 69 PAY  66 

    0x50273edc,// 70 PAY  67 

    0x1fcbad8f,// 71 PAY  68 

    0x5e8ff0a6,// 72 PAY  69 

    0x3567ba28,// 73 PAY  70 

    0x8c6888e4,// 74 PAY  71 

    0x4a6ad06e,// 75 PAY  72 

    0xed2c566d,// 76 PAY  73 

    0x76834dec,// 77 PAY  74 

    0x8985f9c9,// 78 PAY  75 

    0xafe5ba33,// 79 PAY  76 

    0x4357b0a0,// 80 PAY  77 

    0xf709da7b,// 81 PAY  78 

    0x6898e65c,// 82 PAY  79 

    0xcdb4af06,// 83 PAY  80 

    0x57fcda92,// 84 PAY  81 

    0xebf21972,// 85 PAY  82 

    0x5407cb06,// 86 PAY  83 

    0x12df2c8b,// 87 PAY  84 

    0xa4fec5b5,// 88 PAY  85 

    0x10d2c3a7,// 89 PAY  86 

    0x9d5644ef,// 90 PAY  87 

    0x94db09bb,// 91 PAY  88 

    0x792b7f0b,// 92 PAY  89 

    0x58ec9e9b,// 93 PAY  90 

    0x07e58ae5,// 94 PAY  91 

    0xdeadfd22,// 95 PAY  92 

    0xdce73b86,// 96 PAY  93 

    0xf38b405f,// 97 PAY  94 

    0x696a32ee,// 98 PAY  95 

    0x9da76370,// 99 PAY  96 

    0x4d65bcbb,// 100 PAY  97 

    0xe77cde02,// 101 PAY  98 

    0xa7555f8f,// 102 PAY  99 

    0xf9fae6c4,// 103 PAY 100 

    0x3abad1ad,// 104 PAY 101 

    0xa1a30665,// 105 PAY 102 

    0x5e3c1767,// 106 PAY 103 

    0xc3cbe564,// 107 PAY 104 

    0x3cb44328,// 108 PAY 105 

    0x36bf0fde,// 109 PAY 106 

    0x2dd6b67e,// 110 PAY 107 

    0x81a38aef,// 111 PAY 108 

    0x231bc893,// 112 PAY 109 

    0x27cdba01,// 113 PAY 110 

    0x588a6e22,// 114 PAY 111 

    0x4f91c11c,// 115 PAY 112 

    0xa30d2fc4,// 116 PAY 113 

    0x65d55aba,// 117 PAY 114 

    0xe0f23984,// 118 PAY 115 

    0x9d138bf6,// 119 PAY 116 

    0xc9f872b3,// 120 PAY 117 

    0xebfaf92b,// 121 PAY 118 

    0x6c61e483,// 122 PAY 119 

    0xfb21b7aa,// 123 PAY 120 

    0xd2f5f7ce,// 124 PAY 121 

    0x0cdab480,// 125 PAY 122 

    0x14880a40,// 126 PAY 123 

    0x162e5f89,// 127 PAY 124 

    0x798ecfb2,// 128 PAY 125 

    0xcb18754f,// 129 PAY 126 

    0xf7985220,// 130 PAY 127 

    0x89f769e9,// 131 PAY 128 

    0xc0623efb,// 132 PAY 129 

    0xbde03d41,// 133 PAY 130 

    0xcf3a3dc7,// 134 PAY 131 

    0x14f9569f,// 135 PAY 132 

    0xa27b4db3,// 136 PAY 133 

    0x1ead972a,// 137 PAY 134 

    0xf9d1afc5,// 138 PAY 135 

    0xd8f8aed7,// 139 PAY 136 

    0x1828cacd,// 140 PAY 137 

    0x53329e7d,// 141 PAY 138 

    0xfe2f5354,// 142 PAY 139 

    0x420cf69e,// 143 PAY 140 

    0xc552004c,// 144 PAY 141 

    0xf47ced60,// 145 PAY 142 

    0xfc9e4d6f,// 146 PAY 143 

    0x1c90d564,// 147 PAY 144 

    0xa01c1297,// 148 PAY 145 

    0xc7ff3af8,// 149 PAY 146 

    0xbac3ecf2,// 150 PAY 147 

    0xbb37d29b,// 151 PAY 148 

    0xd3ae7bf8,// 152 PAY 149 

    0x2bb1c68a,// 153 PAY 150 

    0xcb2ab7f9,// 154 PAY 151 

    0xfb2a02c1,// 155 PAY 152 

    0xdbc3c1a1,// 156 PAY 153 

    0x7aaa02cf,// 157 PAY 154 

    0x69c0d0cb,// 158 PAY 155 

    0xf65108a3,// 159 PAY 156 

    0x95164fa5,// 160 PAY 157 

    0xef399a05,// 161 PAY 158 

    0x48fcd8cc,// 162 PAY 159 

    0xb66e8f94,// 163 PAY 160 

    0x30434aa5,// 164 PAY 161 

    0x5e9e4adc,// 165 PAY 162 

    0x92feb4c0,// 166 PAY 163 

    0x1a9004a6,// 167 PAY 164 

    0xf9f1d325,// 168 PAY 165 

    0x17a22db9,// 169 PAY 166 

    0x0a7386a2,// 170 PAY 167 

    0xc2540209,// 171 PAY 168 

    0xf6ca307a,// 172 PAY 169 

    0x0e90a0e6,// 173 PAY 170 

    0x027b61aa,// 174 PAY 171 

    0x98f6c962,// 175 PAY 172 

    0x65226f7a,// 176 PAY 173 

    0x3bb39a0c,// 177 PAY 174 

    0x5b784039,// 178 PAY 175 

    0x2ce534e5,// 179 PAY 176 

    0x46ad479c,// 180 PAY 177 

    0x54755434,// 181 PAY 178 

    0x14255b50,// 182 PAY 179 

    0xeb6eef52,// 183 PAY 180 

    0x350008ab,// 184 PAY 181 

    0x4d18142c,// 185 PAY 182 

    0xd6728619,// 186 PAY 183 

    0x8e619ee4,// 187 PAY 184 

    0x47a0b7d7,// 188 PAY 185 

    0x0b8573b2,// 189 PAY 186 

    0xe6ae1253,// 190 PAY 187 

    0x11d38b28,// 191 PAY 188 

    0x870e03f6,// 192 PAY 189 

    0x0529f0e3,// 193 PAY 190 

    0xb7b13ece,// 194 PAY 191 

    0x3c66e4be,// 195 PAY 192 

    0xe10b142e,// 196 PAY 193 

    0x46d83752,// 197 PAY 194 

    0x9c4f050d,// 198 PAY 195 

    0xd6e31735,// 199 PAY 196 

    0xe3e210eb,// 200 PAY 197 

    0x5f63eb5e,// 201 PAY 198 

    0x4832241a,// 202 PAY 199 

    0x9391fb26,// 203 PAY 200 

    0x9ab6f13b,// 204 PAY 201 

    0x3ca4fc48,// 205 PAY 202 

    0x3e17bc71,// 206 PAY 203 

    0xb6aae146,// 207 PAY 204 

    0x0a7eaa1d,// 208 PAY 205 

    0x979273ec,// 209 PAY 206 

    0x58ac19a8,// 210 PAY 207 

    0x9c0e6b4e,// 211 PAY 208 

    0x1e5039d3,// 212 PAY 209 

    0x9973e208,// 213 PAY 210 

    0xf3129683,// 214 PAY 211 

    0x188c6f88,// 215 PAY 212 

    0xded0d901,// 216 PAY 213 

    0x8ee9e5bb,// 217 PAY 214 

    0xd3d0840a,// 218 PAY 215 

    0xaf9c4b00,// 219 PAY 216 

    0xe62ea681,// 220 PAY 217 

    0xa5424d69,// 221 PAY 218 

    0xbd88e3e3,// 222 PAY 219 

    0x034e7a9b,// 223 PAY 220 

    0x06f517bb,// 224 PAY 221 

    0x2a0920a8,// 225 PAY 222 

    0xb5eb6b03,// 226 PAY 223 

    0x53b2b36b,// 227 PAY 224 

    0xe11c35cd,// 228 PAY 225 

    0x81ab9427,// 229 PAY 226 

    0x4cfbe5f1,// 230 PAY 227 

    0x4ef5bbbc,// 231 PAY 228 

    0xba6dbd64,// 232 PAY 229 

    0xbccb1e2e,// 233 PAY 230 

    0x3138a034,// 234 PAY 231 

    0x6644d2d0,// 235 PAY 232 

    0x89654473,// 236 PAY 233 

    0x79cbd30d,// 237 PAY 234 

    0x15e32c8c,// 238 PAY 235 

    0xff8f48bf,// 239 PAY 236 

    0x6d73538c,// 240 PAY 237 

    0x41671a2e,// 241 PAY 238 

    0xa4d7262c,// 242 PAY 239 

    0x299c361c,// 243 PAY 240 

    0xcf8ee44a,// 244 PAY 241 

    0xe1f0718b,// 245 PAY 242 

    0x4c0fbb15,// 246 PAY 243 

    0xb0ec5e28,// 247 PAY 244 

    0x91ee192f,// 248 PAY 245 

    0x4317fe4e,// 249 PAY 246 

    0x3af24aea,// 250 PAY 247 

    0xc0218034,// 251 PAY 248 

    0xb89e3498,// 252 PAY 249 

    0x0b59cb20,// 253 PAY 250 

    0xa47306c2,// 254 PAY 251 

    0xa34d8471,// 255 PAY 252 

    0x8985c317,// 256 PAY 253 

    0x9ade2522,// 257 PAY 254 

    0x73d90b7d,// 258 PAY 255 

    0x8562b706,// 259 PAY 256 

    0x891d935d,// 260 PAY 257 

    0xd90169ed,// 261 PAY 258 

    0xf3dc8f77,// 262 PAY 259 

    0xc00eb1df,// 263 PAY 260 

    0xd6167769,// 264 PAY 261 

    0x4a80f055,// 265 PAY 262 

    0x69f4a0aa,// 266 PAY 263 

    0x21dafb33,// 267 PAY 264 

    0xc0e86492,// 268 PAY 265 

    0x9e172f9a,// 269 PAY 266 

    0x7a860d77,// 270 PAY 267 

    0xba15523e,// 271 PAY 268 

    0x5ab63413,// 272 PAY 269 

    0x1e4ad7a0,// 273 PAY 270 

    0x676884d8,// 274 PAY 271 

    0x42a661a6,// 275 PAY 272 

    0xd424197f,// 276 PAY 273 

    0xe65e67a5,// 277 PAY 274 

    0xd2935102,// 278 PAY 275 

    0xfb872641,// 279 PAY 276 

    0xc8c23594,// 280 PAY 277 

    0x851daa1f,// 281 PAY 278 

    0x75808b8d,// 282 PAY 279 

    0x6a203307,// 283 PAY 280 

    0x09e27ec6,// 284 PAY 281 

    0x6444aec5,// 285 PAY 282 

    0xc08ceb94,// 286 PAY 283 

    0xe2120c3d,// 287 PAY 284 

    0x4c5c1b62,// 288 PAY 285 

    0xa0976123,// 289 PAY 286 

    0xef719b91,// 290 PAY 287 

    0x7a6499c0,// 291 PAY 288 

    0xc712d621,// 292 PAY 289 

    0x96ce3ecd,// 293 PAY 290 

    0x8274ecbc,// 294 PAY 291 

    0x6a60e9ed,// 295 PAY 292 

    0x4c491282,// 296 PAY 293 

    0xdb3f9e99,// 297 PAY 294 

    0x54edc82c,// 298 PAY 295 

    0x4ebaac16,// 299 PAY 296 

    0xcc85d225,// 300 PAY 297 

    0xd49ba18d,// 301 PAY 298 

    0xd500a44a,// 302 PAY 299 

    0x1fc7b762,// 303 PAY 300 

    0x2fe46644,// 304 PAY 301 

    0x188b817d,// 305 PAY 302 

    0xc966db06,// 306 PAY 303 

    0xc600e5d2,// 307 PAY 304 

    0xa32dc3e9,// 308 PAY 305 

    0xae638ef5,// 309 PAY 306 

    0x4ac4e7f9,// 310 PAY 307 

    0x74a67a46,// 311 PAY 308 

    0x44664bd4,// 312 PAY 309 

    0x160a328d,// 313 PAY 310 

    0xe852d851,// 314 PAY 311 

    0x21ca2df5,// 315 PAY 312 

    0x073992ad,// 316 PAY 313 

    0xabf45422,// 317 PAY 314 

    0x60272d7a,// 318 PAY 315 

    0x38c680dc,// 319 PAY 316 

    0xf50bbb87,// 320 PAY 317 

    0x14de4e2d,// 321 PAY 318 

    0xa7545acf,// 322 PAY 319 

    0x03b5678a,// 323 PAY 320 

    0x06490826,// 324 PAY 321 

    0x82727016,// 325 PAY 322 

    0x4c02981a,// 326 PAY 323 

    0x5e5b594f,// 327 PAY 324 

    0x66f5d7ea,// 328 PAY 325 

    0x5165a38f,// 329 PAY 326 

    0x4f974266,// 330 PAY 327 

    0x1336428f,// 331 PAY 328 

    0x2cd64f23,// 332 PAY 329 

    0xb7dd3008,// 333 PAY 330 

    0x4e7add85,// 334 PAY 331 

    0xc86afb6e,// 335 PAY 332 

    0x7ec8bb52,// 336 PAY 333 

    0xc2e43ef0,// 337 PAY 334 

    0xc338d83f,// 338 PAY 335 

    0x03522299,// 339 PAY 336 

    0x96c470b5,// 340 PAY 337 

    0x71a4dcd2,// 341 PAY 338 

    0xcf0ed51a,// 342 PAY 339 

    0x5901bbf8,// 343 PAY 340 

    0xbc507f0c,// 344 PAY 341 

    0xd6bb5e85,// 345 PAY 342 

    0xa35b032d,// 346 PAY 343 

    0xd1fd2599,// 347 PAY 344 

    0x0961a0b4,// 348 PAY 345 

    0x8781a3e8,// 349 PAY 346 

    0x885ef828,// 350 PAY 347 

    0x9dec9650,// 351 PAY 348 

    0xb538e7a7,// 352 PAY 349 

    0x2fd0fadb,// 353 PAY 350 

    0x152ebe69,// 354 PAY 351 

    0x4d871d18,// 355 PAY 352 

    0x8dd99a99,// 356 PAY 353 

    0x14e1117e,// 357 PAY 354 

    0xf496233b,// 358 PAY 355 

    0x9c3cae31,// 359 PAY 356 

    0x7eef3dc9,// 360 PAY 357 

    0xc92ba0d8,// 361 PAY 358 

    0x3c48e908,// 362 PAY 359 

    0xc7c34d88,// 363 PAY 360 

    0x425eeba3,// 364 PAY 361 

    0xa0772a4e,// 365 PAY 362 

    0x54925fbf,// 366 PAY 363 

    0x28ba5444,// 367 PAY 364 

    0x822409e1,// 368 PAY 365 

    0x74554429,// 369 PAY 366 

    0x91891957,// 370 PAY 367 

    0x5ca7a8bc,// 371 PAY 368 

    0x1dd443f8,// 372 PAY 369 

    0x96c51073,// 373 PAY 370 

    0xee4c78bb,// 374 PAY 371 

    0x2f3cbebc,// 375 PAY 372 

    0x2e5acc04,// 376 PAY 373 

    0x6ae178e6,// 377 PAY 374 

    0x3a3df664,// 378 PAY 375 

    0xe13d0c59,// 379 PAY 376 

    0xd8b2018e,// 380 PAY 377 

    0x0cfde799,// 381 PAY 378 

    0xdbbe7d21,// 382 PAY 379 

    0x7a203619,// 383 PAY 380 

    0xda86d627,// 384 PAY 381 

    0xeb9efd81,// 385 PAY 382 

    0x013a66a1,// 386 PAY 383 

    0xe46499ef,// 387 PAY 384 

    0xac222d56,// 388 PAY 385 

    0x9df40cbc,// 389 PAY 386 

    0xe186b2b8,// 390 PAY 387 

    0x63c0e84a,// 391 PAY 388 

    0x5912af9d,// 392 PAY 389 

    0x41ffc26e,// 393 PAY 390 

    0x1a0d4266,// 394 PAY 391 

    0x58a17a2b,// 395 PAY 392 

    0xe684d98c,// 396 PAY 393 

    0x9aa7b72b,// 397 PAY 394 

    0x5025fa47,// 398 PAY 395 

    0x81b09dc6,// 399 PAY 396 

    0x1ad82d7f,// 400 PAY 397 

    0xc9fce28f,// 401 PAY 398 

    0xe15ef6f9,// 402 PAY 399 

    0xa70e5f1d,// 403 PAY 400 

    0xc8e56611,// 404 PAY 401 

    0x6c35ba86,// 405 PAY 402 

    0x4d426555,// 406 PAY 403 

    0x3dee1173,// 407 PAY 404 

    0xd07e44b9,// 408 PAY 405 

    0xdae3672f,// 409 PAY 406 

    0x72a02c39,// 410 PAY 407 

    0x9fd95b4e,// 411 PAY 408 

    0xbd203c75,// 412 PAY 409 

    0x4b79e5f3,// 413 PAY 410 

    0xd27e93a9,// 414 PAY 411 

    0x81832c4b,// 415 PAY 412 

    0xe2ba0e4f,// 416 PAY 413 

    0x380f6eaa,// 417 PAY 414 

    0xcadfdec5,// 418 PAY 415 

    0x7d6aecfd,// 419 PAY 416 

    0x2f7e4a47,// 420 PAY 417 

    0x12d823f9,// 421 PAY 418 

    0x880b6f3c,// 422 PAY 419 

    0x0313aaab,// 423 PAY 420 

    0xe1b49c9a,// 424 PAY 421 

    0x67659b6e,// 425 PAY 422 

    0xa08e082c,// 426 PAY 423 

    0x713cb6ca,// 427 PAY 424 

    0x2859adc4,// 428 PAY 425 

    0x1232832c,// 429 PAY 426 

    0x888464d3,// 430 PAY 427 

    0xda000000,// 431 PAY 428 

/// HASH is  20 bytes 

    0xef719b91,// 432 HSH   1 

    0x7a6499c0,// 433 HSH   2 

    0xc712d621,// 434 HSH   3 

    0x96ce3ecd,// 435 HSH   4 

    0x8274ecbc,// 436 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 168 

/// STA pkt_idx        : 182 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3e 

    0x02d93ea8 // 437 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt14_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 444 words. 

/// BDA size     is 1772 (0x6ec) 

/// BDA id       is 0x241b 

    0x06ec241b,// 3 BDA   1 

/// PAY Generic Data size   : 1772 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xb1f6c0e4,// 4 PAY   1 

    0xfc130bb3,// 5 PAY   2 

    0x85dd1cf7,// 6 PAY   3 

    0x72811996,// 7 PAY   4 

    0xb53c8f85,// 8 PAY   5 

    0xf32c3028,// 9 PAY   6 

    0x02b5bb41,// 10 PAY   7 

    0x9e9f249e,// 11 PAY   8 

    0xeb0764ba,// 12 PAY   9 

    0x7f22b5ef,// 13 PAY  10 

    0xb0801143,// 14 PAY  11 

    0x04a7f6dc,// 15 PAY  12 

    0xafeb6f98,// 16 PAY  13 

    0x3696e3b1,// 17 PAY  14 

    0xcb50e85f,// 18 PAY  15 

    0x56f06b2a,// 19 PAY  16 

    0x226b8691,// 20 PAY  17 

    0xa5c7b731,// 21 PAY  18 

    0x4df5b220,// 22 PAY  19 

    0x45ac1408,// 23 PAY  20 

    0x2c1a5458,// 24 PAY  21 

    0xe446cda7,// 25 PAY  22 

    0x52c762b0,// 26 PAY  23 

    0x3c642779,// 27 PAY  24 

    0xcba79440,// 28 PAY  25 

    0x3a704336,// 29 PAY  26 

    0xc8b6c901,// 30 PAY  27 

    0xd9c21c3c,// 31 PAY  28 

    0x0857ec42,// 32 PAY  29 

    0xc629c9b1,// 33 PAY  30 

    0x494c58d4,// 34 PAY  31 

    0x27a3b743,// 35 PAY  32 

    0x59cef63d,// 36 PAY  33 

    0x8deacbf7,// 37 PAY  34 

    0x5dfad799,// 38 PAY  35 

    0xc2dfb7de,// 39 PAY  36 

    0xe07c65c7,// 40 PAY  37 

    0xc80cd6e7,// 41 PAY  38 

    0x13ae3ad5,// 42 PAY  39 

    0x28c2c728,// 43 PAY  40 

    0xeee04328,// 44 PAY  41 

    0x6501c1ce,// 45 PAY  42 

    0xc45cb21f,// 46 PAY  43 

    0x37587be7,// 47 PAY  44 

    0x8208d06f,// 48 PAY  45 

    0x0a8eab3f,// 49 PAY  46 

    0x4701e407,// 50 PAY  47 

    0x61cd5faf,// 51 PAY  48 

    0x9cb14d83,// 52 PAY  49 

    0x5cc7243f,// 53 PAY  50 

    0x471c2a37,// 54 PAY  51 

    0x19d82f90,// 55 PAY  52 

    0x3d5f1b92,// 56 PAY  53 

    0x7f0b1952,// 57 PAY  54 

    0x24917db8,// 58 PAY  55 

    0x43d3e209,// 59 PAY  56 

    0x286a2343,// 60 PAY  57 

    0x1f95f9d8,// 61 PAY  58 

    0xa4e08fe3,// 62 PAY  59 

    0x6d9df744,// 63 PAY  60 

    0x6f72c9fe,// 64 PAY  61 

    0x2731d023,// 65 PAY  62 

    0xae23b2a1,// 66 PAY  63 

    0x1d92846f,// 67 PAY  64 

    0x0e84f3e6,// 68 PAY  65 

    0xe47cb13b,// 69 PAY  66 

    0x6f5ef634,// 70 PAY  67 

    0x40836755,// 71 PAY  68 

    0xca0810b2,// 72 PAY  69 

    0x312f2e6f,// 73 PAY  70 

    0xa8aed427,// 74 PAY  71 

    0x529af83e,// 75 PAY  72 

    0x0f0e69d2,// 76 PAY  73 

    0x9a24655c,// 77 PAY  74 

    0xbdd73095,// 78 PAY  75 

    0xee6948a3,// 79 PAY  76 

    0x92a7caba,// 80 PAY  77 

    0xac6564c3,// 81 PAY  78 

    0x43b8facf,// 82 PAY  79 

    0x29760d4e,// 83 PAY  80 

    0xe041e59f,// 84 PAY  81 

    0xfbe18d75,// 85 PAY  82 

    0x0442e7c6,// 86 PAY  83 

    0x5a76ec71,// 87 PAY  84 

    0xbcc36ebe,// 88 PAY  85 

    0xf965caf6,// 89 PAY  86 

    0xd8f5c362,// 90 PAY  87 

    0x5f2515c8,// 91 PAY  88 

    0xc488e90b,// 92 PAY  89 

    0x662b8ab4,// 93 PAY  90 

    0xdfca3cec,// 94 PAY  91 

    0x96183f6f,// 95 PAY  92 

    0x0af78fec,// 96 PAY  93 

    0x12beccc8,// 97 PAY  94 

    0xbb0b404c,// 98 PAY  95 

    0xe64d2b03,// 99 PAY  96 

    0x7e12254a,// 100 PAY  97 

    0xb6f646ea,// 101 PAY  98 

    0x25e6ad64,// 102 PAY  99 

    0xeb554a74,// 103 PAY 100 

    0xe9fb1921,// 104 PAY 101 

    0x6ce32a59,// 105 PAY 102 

    0x280c87a4,// 106 PAY 103 

    0xd5b50c86,// 107 PAY 104 

    0x02db43f7,// 108 PAY 105 

    0xa5b9b172,// 109 PAY 106 

    0x68164b8a,// 110 PAY 107 

    0x50203ff7,// 111 PAY 108 

    0x5de2d8c8,// 112 PAY 109 

    0x372649dd,// 113 PAY 110 

    0xa4c283e2,// 114 PAY 111 

    0x7fcf2800,// 115 PAY 112 

    0xdd1a92dd,// 116 PAY 113 

    0xf78b0a94,// 117 PAY 114 

    0x4ca4583d,// 118 PAY 115 

    0xb4cef613,// 119 PAY 116 

    0x31b786fe,// 120 PAY 117 

    0x966e9cfc,// 121 PAY 118 

    0x09a0b05c,// 122 PAY 119 

    0x4aa83d6e,// 123 PAY 120 

    0xf679f0ff,// 124 PAY 121 

    0x747ed886,// 125 PAY 122 

    0xd34b0999,// 126 PAY 123 

    0x06ac979a,// 127 PAY 124 

    0x25040184,// 128 PAY 125 

    0x19ca5f03,// 129 PAY 126 

    0x13be60fa,// 130 PAY 127 

    0x39ed4686,// 131 PAY 128 

    0x54f0d41c,// 132 PAY 129 

    0xcf1a275f,// 133 PAY 130 

    0xc545daac,// 134 PAY 131 

    0x70166543,// 135 PAY 132 

    0x429d527a,// 136 PAY 133 

    0xb93a841f,// 137 PAY 134 

    0x867a6ace,// 138 PAY 135 

    0x82e971c9,// 139 PAY 136 

    0x7049c230,// 140 PAY 137 

    0x8aa1def2,// 141 PAY 138 

    0xa769f768,// 142 PAY 139 

    0xe05089b0,// 143 PAY 140 

    0x8e130789,// 144 PAY 141 

    0x7d61f0d9,// 145 PAY 142 

    0x058209a8,// 146 PAY 143 

    0x959c48e6,// 147 PAY 144 

    0xdcbef4bd,// 148 PAY 145 

    0x2a4bec88,// 149 PAY 146 

    0x2cbf3307,// 150 PAY 147 

    0x11d793a2,// 151 PAY 148 

    0x93863494,// 152 PAY 149 

    0x4943d3eb,// 153 PAY 150 

    0x792dec64,// 154 PAY 151 

    0xe6118fd8,// 155 PAY 152 

    0x56e76c0f,// 156 PAY 153 

    0x280e62e0,// 157 PAY 154 

    0x4de8938b,// 158 PAY 155 

    0x89711d97,// 159 PAY 156 

    0x38743e67,// 160 PAY 157 

    0x8ff8d102,// 161 PAY 158 

    0xc596c34a,// 162 PAY 159 

    0x82edf518,// 163 PAY 160 

    0xcbb3d7d0,// 164 PAY 161 

    0x105883f9,// 165 PAY 162 

    0x58c83785,// 166 PAY 163 

    0x99b64b3d,// 167 PAY 164 

    0xd9f037a5,// 168 PAY 165 

    0x719c68f2,// 169 PAY 166 

    0x9a39bbf1,// 170 PAY 167 

    0xa8ce61cf,// 171 PAY 168 

    0xe5a9b40c,// 172 PAY 169 

    0xfffac5e5,// 173 PAY 170 

    0xf26910ed,// 174 PAY 171 

    0x55c2c19b,// 175 PAY 172 

    0xe8249b45,// 176 PAY 173 

    0xcf8ace98,// 177 PAY 174 

    0xd0b335a3,// 178 PAY 175 

    0x9f858b11,// 179 PAY 176 

    0xe32e680f,// 180 PAY 177 

    0x7a61d952,// 181 PAY 178 

    0x0a00ba51,// 182 PAY 179 

    0xbdf617e7,// 183 PAY 180 

    0x3c1ba740,// 184 PAY 181 

    0x235b22a1,// 185 PAY 182 

    0xeb5183e9,// 186 PAY 183 

    0x8b8f5686,// 187 PAY 184 

    0xdb59bcc4,// 188 PAY 185 

    0x2a688d52,// 189 PAY 186 

    0xe0301259,// 190 PAY 187 

    0x9bad6488,// 191 PAY 188 

    0x4aca85f6,// 192 PAY 189 

    0xa2837340,// 193 PAY 190 

    0x12529313,// 194 PAY 191 

    0x099cb8e0,// 195 PAY 192 

    0x0dc8330a,// 196 PAY 193 

    0x9d41381a,// 197 PAY 194 

    0x135831e3,// 198 PAY 195 

    0xa22e8677,// 199 PAY 196 

    0x9920b7a9,// 200 PAY 197 

    0x157d0466,// 201 PAY 198 

    0xf50141c4,// 202 PAY 199 

    0x6b52b3c9,// 203 PAY 200 

    0x09c74048,// 204 PAY 201 

    0xac4a2b06,// 205 PAY 202 

    0xf7fc5966,// 206 PAY 203 

    0x3460bfd9,// 207 PAY 204 

    0x6d973825,// 208 PAY 205 

    0xc1b2197a,// 209 PAY 206 

    0x6b4c07ed,// 210 PAY 207 

    0x6160ac72,// 211 PAY 208 

    0x048e414a,// 212 PAY 209 

    0xd589a29e,// 213 PAY 210 

    0xea4cfc09,// 214 PAY 211 

    0x8f459e34,// 215 PAY 212 

    0x45af5d99,// 216 PAY 213 

    0x44043de0,// 217 PAY 214 

    0x7f07e13e,// 218 PAY 215 

    0xa340de6c,// 219 PAY 216 

    0xce3816a8,// 220 PAY 217 

    0xc05fdd56,// 221 PAY 218 

    0x5888814f,// 222 PAY 219 

    0x31603f5b,// 223 PAY 220 

    0x0ded6a97,// 224 PAY 221 

    0xb8793d2c,// 225 PAY 222 

    0x4d75672a,// 226 PAY 223 

    0x82ba1c2c,// 227 PAY 224 

    0x50b503b1,// 228 PAY 225 

    0x3766bb74,// 229 PAY 226 

    0x86521a88,// 230 PAY 227 

    0x84a0611a,// 231 PAY 228 

    0x7d5c7060,// 232 PAY 229 

    0xf5f9e050,// 233 PAY 230 

    0x76fa5c19,// 234 PAY 231 

    0xee123fe2,// 235 PAY 232 

    0xff59f34f,// 236 PAY 233 

    0xf17e408c,// 237 PAY 234 

    0x9e2ea7d0,// 238 PAY 235 

    0x3290e095,// 239 PAY 236 

    0x9904dc99,// 240 PAY 237 

    0xd8304f68,// 241 PAY 238 

    0x1c9fcbe9,// 242 PAY 239 

    0x2e8f4f0d,// 243 PAY 240 

    0xd5636ddf,// 244 PAY 241 

    0x99e13e59,// 245 PAY 242 

    0xff730c66,// 246 PAY 243 

    0x6ca776b7,// 247 PAY 244 

    0x0094970b,// 248 PAY 245 

    0x5b0b1237,// 249 PAY 246 

    0x6b0fd7ec,// 250 PAY 247 

    0x1056e8b7,// 251 PAY 248 

    0x93b3cb9a,// 252 PAY 249 

    0x2a5827c0,// 253 PAY 250 

    0x4dc603be,// 254 PAY 251 

    0x4bec5565,// 255 PAY 252 

    0x3bf2a65b,// 256 PAY 253 

    0xd0218444,// 257 PAY 254 

    0xfb5f4b28,// 258 PAY 255 

    0xb6591da0,// 259 PAY 256 

    0x922ef262,// 260 PAY 257 

    0x75e9f0ac,// 261 PAY 258 

    0xca6f0a1c,// 262 PAY 259 

    0x4f6ea0d3,// 263 PAY 260 

    0x81129b31,// 264 PAY 261 

    0x823eaaaf,// 265 PAY 262 

    0x2300d6d0,// 266 PAY 263 

    0xa8515d00,// 267 PAY 264 

    0x439d9722,// 268 PAY 265 

    0x37e58514,// 269 PAY 266 

    0x5c73e96d,// 270 PAY 267 

    0x8b98e4c8,// 271 PAY 268 

    0x5e9502eb,// 272 PAY 269 

    0x8511c5e5,// 273 PAY 270 

    0xb2cda944,// 274 PAY 271 

    0x19eb87cc,// 275 PAY 272 

    0xb4735207,// 276 PAY 273 

    0xa40142a3,// 277 PAY 274 

    0xf112ae01,// 278 PAY 275 

    0xbb762c9a,// 279 PAY 276 

    0x20d6ed75,// 280 PAY 277 

    0x6fe95012,// 281 PAY 278 

    0xd7c788ab,// 282 PAY 279 

    0x2f4cd4ca,// 283 PAY 280 

    0x070796f5,// 284 PAY 281 

    0x2fe85961,// 285 PAY 282 

    0x03d7b2ee,// 286 PAY 283 

    0x6cd567e5,// 287 PAY 284 

    0xa0b98540,// 288 PAY 285 

    0x6ab4b727,// 289 PAY 286 

    0x5d883d76,// 290 PAY 287 

    0xfe2143e0,// 291 PAY 288 

    0xb4e71717,// 292 PAY 289 

    0xe371992a,// 293 PAY 290 

    0x7758c8bf,// 294 PAY 291 

    0xf93437c4,// 295 PAY 292 

    0xa5772681,// 296 PAY 293 

    0x2f9ac84c,// 297 PAY 294 

    0xe29a9c70,// 298 PAY 295 

    0xbc0e4488,// 299 PAY 296 

    0xbaa7df93,// 300 PAY 297 

    0x04a520a2,// 301 PAY 298 

    0x112a5580,// 302 PAY 299 

    0x8236b1f8,// 303 PAY 300 

    0xa5861cbd,// 304 PAY 301 

    0x7ff7071a,// 305 PAY 302 

    0xe678da23,// 306 PAY 303 

    0x4ace2022,// 307 PAY 304 

    0x789b736f,// 308 PAY 305 

    0xa92f8e36,// 309 PAY 306 

    0x475fe2a3,// 310 PAY 307 

    0xa79e9adb,// 311 PAY 308 

    0xbfc31626,// 312 PAY 309 

    0xde70a480,// 313 PAY 310 

    0x805e2dca,// 314 PAY 311 

    0x6680934d,// 315 PAY 312 

    0xc46d92ea,// 316 PAY 313 

    0xec8e351b,// 317 PAY 314 

    0xcfb34d68,// 318 PAY 315 

    0xf9aa95fc,// 319 PAY 316 

    0xa2df2e2e,// 320 PAY 317 

    0x9d192e92,// 321 PAY 318 

    0xcae0d908,// 322 PAY 319 

    0xddca430a,// 323 PAY 320 

    0xc730d333,// 324 PAY 321 

    0x24448528,// 325 PAY 322 

    0x2262f405,// 326 PAY 323 

    0x1d723e08,// 327 PAY 324 

    0x72c4faf0,// 328 PAY 325 

    0xd49f6772,// 329 PAY 326 

    0x6062eca4,// 330 PAY 327 

    0x82a847ae,// 331 PAY 328 

    0xf03ad882,// 332 PAY 329 

    0xa59e5f5f,// 333 PAY 330 

    0x9c8b95f3,// 334 PAY 331 

    0xd1a1fce8,// 335 PAY 332 

    0x08a8934f,// 336 PAY 333 

    0x6e0ddea2,// 337 PAY 334 

    0x230065f5,// 338 PAY 335 

    0xf8d8411f,// 339 PAY 336 

    0xbe28d514,// 340 PAY 337 

    0x933d6100,// 341 PAY 338 

    0x7c9efa43,// 342 PAY 339 

    0x73dc6712,// 343 PAY 340 

    0x531b2e1a,// 344 PAY 341 

    0xb091e342,// 345 PAY 342 

    0xe03aba8c,// 346 PAY 343 

    0x3713418c,// 347 PAY 344 

    0x52d4d608,// 348 PAY 345 

    0x98d18182,// 349 PAY 346 

    0xa67c6c78,// 350 PAY 347 

    0xa49465bf,// 351 PAY 348 

    0x36ff48ae,// 352 PAY 349 

    0xfdfd2146,// 353 PAY 350 

    0xeddbc4e6,// 354 PAY 351 

    0x5fb0bcff,// 355 PAY 352 

    0x692e2687,// 356 PAY 353 

    0x88abeed8,// 357 PAY 354 

    0x2f2f1a55,// 358 PAY 355 

    0x1e4dabb8,// 359 PAY 356 

    0x4eb7d0d1,// 360 PAY 357 

    0x1b3dc5c5,// 361 PAY 358 

    0x7d96080e,// 362 PAY 359 

    0x3c74cbc4,// 363 PAY 360 

    0x6fbb4853,// 364 PAY 361 

    0x85e81a8f,// 365 PAY 362 

    0x3bd22fe9,// 366 PAY 363 

    0xf5ceaa7c,// 367 PAY 364 

    0xbf1632ef,// 368 PAY 365 

    0xcc51864b,// 369 PAY 366 

    0x274bbb4b,// 370 PAY 367 

    0x808cee77,// 371 PAY 368 

    0xe78cc016,// 372 PAY 369 

    0xf331ccac,// 373 PAY 370 

    0xaeac3884,// 374 PAY 371 

    0x5d1e50e6,// 375 PAY 372 

    0x08a75dae,// 376 PAY 373 

    0xf07bc79b,// 377 PAY 374 

    0x78808360,// 378 PAY 375 

    0x1c74b839,// 379 PAY 376 

    0x4a90e440,// 380 PAY 377 

    0xed5a8772,// 381 PAY 378 

    0xbc536dbf,// 382 PAY 379 

    0x306aa08a,// 383 PAY 380 

    0x31600f5b,// 384 PAY 381 

    0xc0bd6988,// 385 PAY 382 

    0x2ecf8bc8,// 386 PAY 383 

    0x930a62e7,// 387 PAY 384 

    0x3e7c5146,// 388 PAY 385 

    0x2a73ab0c,// 389 PAY 386 

    0xf5522a6e,// 390 PAY 387 

    0xcfb912f5,// 391 PAY 388 

    0x56beac22,// 392 PAY 389 

    0x2cb369ff,// 393 PAY 390 

    0x1604f1a7,// 394 PAY 391 

    0xe87b2132,// 395 PAY 392 

    0x20ea17c6,// 396 PAY 393 

    0x2717e44f,// 397 PAY 394 

    0xf878ef12,// 398 PAY 395 

    0x75b11e50,// 399 PAY 396 

    0x25efaaa3,// 400 PAY 397 

    0x7cf9ea90,// 401 PAY 398 

    0xec4c6b96,// 402 PAY 399 

    0x32c57246,// 403 PAY 400 

    0x88546db1,// 404 PAY 401 

    0x11a0afb0,// 405 PAY 402 

    0x9e15adcc,// 406 PAY 403 

    0x38780da3,// 407 PAY 404 

    0x055a3ca7,// 408 PAY 405 

    0x1a0ee810,// 409 PAY 406 

    0xb713386a,// 410 PAY 407 

    0x26bb3c0e,// 411 PAY 408 

    0xce6c9091,// 412 PAY 409 

    0xa6f26361,// 413 PAY 410 

    0x38212fe9,// 414 PAY 411 

    0x1aa4fdf6,// 415 PAY 412 

    0x63fc5e5f,// 416 PAY 413 

    0x1d545d2c,// 417 PAY 414 

    0x06d337a2,// 418 PAY 415 

    0x2f2e384b,// 419 PAY 416 

    0x1e101aa2,// 420 PAY 417 

    0x29aaf629,// 421 PAY 418 

    0x06505567,// 422 PAY 419 

    0xfe1cab1e,// 423 PAY 420 

    0x73b8a883,// 424 PAY 421 

    0xc3b379cc,// 425 PAY 422 

    0xc88bbc56,// 426 PAY 423 

    0x8c14a592,// 427 PAY 424 

    0xb11151c4,// 428 PAY 425 

    0xf5ec86d0,// 429 PAY 426 

    0xa4e31a2e,// 430 PAY 427 

    0xaae47694,// 431 PAY 428 

    0x55f01d08,// 432 PAY 429 

    0x3aa7f3e0,// 433 PAY 430 

    0x69e1284f,// 434 PAY 431 

    0x5ea0061f,// 435 PAY 432 

    0x864fcf60,// 436 PAY 433 

    0xdd40696b,// 437 PAY 434 

    0xc771ce18,// 438 PAY 435 

    0x2c8a14d1,// 439 PAY 436 

    0xdacb79b2,// 440 PAY 437 

    0xc6f9695b,// 441 PAY 438 

    0xbcda0922,// 442 PAY 439 

    0x72382f6d,// 443 PAY 440 

    0xb758a83c,// 444 PAY 441 

    0x6d6141f5,// 445 PAY 442 

    0xeb9c1307,// 446 PAY 443 

/// HASH is  16 bytes 

    0x81129b31,// 447 HSH   1 

    0x823eaaaf,// 448 HSH   2 

    0x2300d6d0,// 449 HSH   3 

    0xa8515d00,// 450 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 132 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x26 

    0x01f12684 // 451 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt15_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 61 words. 

/// BDA size     is 240 (0xf0) 

/// BDA id       is 0xa24 

    0x00f00a24,// 3 BDA   1 

/// PAY Generic Data size   : 240 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x24cdbae5,// 4 PAY   1 

    0x4ebf7281,// 5 PAY   2 

    0xb5e22747,// 6 PAY   3 

    0x92edc4b5,// 7 PAY   4 

    0xc19a4bbd,// 8 PAY   5 

    0x2361b47f,// 9 PAY   6 

    0xc7169c07,// 10 PAY   7 

    0x69361fbe,// 11 PAY   8 

    0xbd79a90b,// 12 PAY   9 

    0x0e841e51,// 13 PAY  10 

    0xdb66476a,// 14 PAY  11 

    0xdf2786df,// 15 PAY  12 

    0x435f719c,// 16 PAY  13 

    0x2938f58f,// 17 PAY  14 

    0x4d6603c2,// 18 PAY  15 

    0x29493f2b,// 19 PAY  16 

    0x6ec3258a,// 20 PAY  17 

    0xb9ab7588,// 21 PAY  18 

    0xe9bf9713,// 22 PAY  19 

    0x49cbd518,// 23 PAY  20 

    0xb50a1709,// 24 PAY  21 

    0x90b1cb0a,// 25 PAY  22 

    0x9b73bbd1,// 26 PAY  23 

    0x57ceee80,// 27 PAY  24 

    0xc62df69b,// 28 PAY  25 

    0x4fbfb668,// 29 PAY  26 

    0x8db814f7,// 30 PAY  27 

    0x3122cce9,// 31 PAY  28 

    0x5ba7d9a3,// 32 PAY  29 

    0x21b99f87,// 33 PAY  30 

    0x7cf3f8ed,// 34 PAY  31 

    0xbee8228b,// 35 PAY  32 

    0x5303aa81,// 36 PAY  33 

    0xc0feb368,// 37 PAY  34 

    0x922dd8a4,// 38 PAY  35 

    0xbd86e794,// 39 PAY  36 

    0xd5b7b8a8,// 40 PAY  37 

    0x13bcef17,// 41 PAY  38 

    0x80b5e900,// 42 PAY  39 

    0x886a212d,// 43 PAY  40 

    0x31b51011,// 44 PAY  41 

    0xeb6546a1,// 45 PAY  42 

    0xdebd9cc2,// 46 PAY  43 

    0x0f91759c,// 47 PAY  44 

    0x5ea9e83c,// 48 PAY  45 

    0xcdddc529,// 49 PAY  46 

    0x15cc9308,// 50 PAY  47 

    0xec13847d,// 51 PAY  48 

    0xc0ced4a5,// 52 PAY  49 

    0x276b7eff,// 53 PAY  50 

    0x7d22bf96,// 54 PAY  51 

    0x5bf7c067,// 55 PAY  52 

    0x0e5d5280,// 56 PAY  53 

    0xb671ade5,// 57 PAY  54 

    0x67d7a130,// 58 PAY  55 

    0xc3c2f8c2,// 59 PAY  56 

    0xfd28773b,// 60 PAY  57 

    0x5f28c63d,// 61 PAY  58 

    0xe75a1aa7,// 62 PAY  59 

    0xc382f564,// 63 PAY  60 

/// STA is 1 words. 

/// STA num_pkts       : 92 

/// STA pkt_idx        : 100 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xde 

    0x0191de5c // 64 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt16_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 60 words. 

/// BDA size     is 235 (0xeb) 

/// BDA id       is 0x8a3d 

    0x00eb8a3d,// 3 BDA   1 

/// PAY Generic Data size   : 235 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x776950d0,// 4 PAY   1 

    0x0b9849d9,// 5 PAY   2 

    0x4c3b7f96,// 6 PAY   3 

    0x3a3224a3,// 7 PAY   4 

    0x63c0f483,// 8 PAY   5 

    0x5980b4bf,// 9 PAY   6 

    0xdb0a6e22,// 10 PAY   7 

    0x71fbab37,// 11 PAY   8 

    0xab8c802f,// 12 PAY   9 

    0x4a11b792,// 13 PAY  10 

    0x9757f112,// 14 PAY  11 

    0x83927c9e,// 15 PAY  12 

    0xc79676c5,// 16 PAY  13 

    0x642db763,// 17 PAY  14 

    0x91855181,// 18 PAY  15 

    0x25df92d8,// 19 PAY  16 

    0x15eed87a,// 20 PAY  17 

    0xdd9ee247,// 21 PAY  18 

    0xaa6194ac,// 22 PAY  19 

    0x5d0be4d4,// 23 PAY  20 

    0x0d3f53e2,// 24 PAY  21 

    0x4a569e41,// 25 PAY  22 

    0x8e493c0a,// 26 PAY  23 

    0xb6f72cdf,// 27 PAY  24 

    0x5a11c8e4,// 28 PAY  25 

    0xe0ab5338,// 29 PAY  26 

    0x8a42782c,// 30 PAY  27 

    0xcf769938,// 31 PAY  28 

    0xad4102fc,// 32 PAY  29 

    0xdd290a87,// 33 PAY  30 

    0x4f6d467a,// 34 PAY  31 

    0xb94b28ce,// 35 PAY  32 

    0xd073f36f,// 36 PAY  33 

    0x27b6f4a5,// 37 PAY  34 

    0x4a17ae28,// 38 PAY  35 

    0x7350ed91,// 39 PAY  36 

    0x95a2a92a,// 40 PAY  37 

    0x2354fb0c,// 41 PAY  38 

    0x56e25cae,// 42 PAY  39 

    0xd29bc7cd,// 43 PAY  40 

    0x026b8c3e,// 44 PAY  41 

    0xeb79110b,// 45 PAY  42 

    0xe1222029,// 46 PAY  43 

    0xfa3f47cc,// 47 PAY  44 

    0xe266c4e2,// 48 PAY  45 

    0x30017205,// 49 PAY  46 

    0xbec1142b,// 50 PAY  47 

    0xcc387594,// 51 PAY  48 

    0x71d9fb2f,// 52 PAY  49 

    0x4bc34b70,// 53 PAY  50 

    0xa0c48a65,// 54 PAY  51 

    0xee19ed5c,// 55 PAY  52 

    0xe110140f,// 56 PAY  53 

    0xd3ba4342,// 57 PAY  54 

    0xfc8b33c3,// 58 PAY  55 

    0x0366a0fb,// 59 PAY  56 

    0x09f0ed43,// 60 PAY  57 

    0x1d8963a3,// 61 PAY  58 

    0xddb62300,// 62 PAY  59 

/// STA is 1 words. 

/// STA num_pkts       : 124 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc5 

    0x03dcc57c // 63 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt17_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 46 words. 

/// BDA size     is 177 (0xb1) 

/// BDA id       is 0xc3ba 

    0x00b1c3ba,// 3 BDA   1 

/// PAY Generic Data size   : 177 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xe8b99d53,// 4 PAY   1 

    0xeb6e9040,// 5 PAY   2 

    0x60314c6b,// 6 PAY   3 

    0x48765a27,// 7 PAY   4 

    0x63cb98b1,// 8 PAY   5 

    0x0511c772,// 9 PAY   6 

    0x63c97192,// 10 PAY   7 

    0x6184f4e8,// 11 PAY   8 

    0x842a5498,// 12 PAY   9 

    0xe7d032c7,// 13 PAY  10 

    0x8d6ea372,// 14 PAY  11 

    0x8abd4cda,// 15 PAY  12 

    0x88b30160,// 16 PAY  13 

    0x171cff14,// 17 PAY  14 

    0x8380df93,// 18 PAY  15 

    0xb94327fa,// 19 PAY  16 

    0x1310e997,// 20 PAY  17 

    0xe098272f,// 21 PAY  18 

    0x74e1dc69,// 22 PAY  19 

    0xa2dda24d,// 23 PAY  20 

    0xef17f30e,// 24 PAY  21 

    0x7e279507,// 25 PAY  22 

    0x09c70922,// 26 PAY  23 

    0xfe80c5a8,// 27 PAY  24 

    0xac577ab5,// 28 PAY  25 

    0x77026357,// 29 PAY  26 

    0x8183e9c3,// 30 PAY  27 

    0x64478862,// 31 PAY  28 

    0x737710ea,// 32 PAY  29 

    0x8b1c7079,// 33 PAY  30 

    0x535a78a5,// 34 PAY  31 

    0xd111c5db,// 35 PAY  32 

    0x7046b97e,// 36 PAY  33 

    0x772d5100,// 37 PAY  34 

    0x10005503,// 38 PAY  35 

    0x9627c343,// 39 PAY  36 

    0xae10fc30,// 40 PAY  37 

    0x7a682d98,// 41 PAY  38 

    0x13630e57,// 42 PAY  39 

    0x97bea1f3,// 43 PAY  40 

    0x548f3724,// 44 PAY  41 

    0xbbc42263,// 45 PAY  42 

    0x7bdace7f,// 46 PAY  43 

    0x171781f5,// 47 PAY  44 

    0xd0000000,// 48 PAY  45 

/// HASH is  20 bytes 

    0x535a78a5,// 49 HSH   1 

    0xd111c5db,// 50 HSH   2 

    0x7046b97e,// 51 HSH   3 

    0x772d5100,// 52 HSH   4 

    0x10005503,// 53 HSH   5 

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

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt18_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 282 words. 

/// BDA size     is 1124 (0x464) 

/// BDA id       is 0xdbea 

    0x0464dbea,// 3 BDA   1 

/// PAY Generic Data size   : 1124 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xb4fa0432,// 4 PAY   1 

    0x3938787c,// 5 PAY   2 

    0x0382a186,// 6 PAY   3 

    0x67fa62eb,// 7 PAY   4 

    0xb5a1561f,// 8 PAY   5 

    0x7923eed3,// 9 PAY   6 

    0xa4f11e3a,// 10 PAY   7 

    0x61c0a935,// 11 PAY   8 

    0xaddb2e19,// 12 PAY   9 

    0xeeebc88b,// 13 PAY  10 

    0xb39bbb4c,// 14 PAY  11 

    0x2476fe69,// 15 PAY  12 

    0xe527e872,// 16 PAY  13 

    0x3118762a,// 17 PAY  14 

    0x89cdef1e,// 18 PAY  15 

    0x26ec42ab,// 19 PAY  16 

    0x5be55de2,// 20 PAY  17 

    0x40615408,// 21 PAY  18 

    0x3d20064b,// 22 PAY  19 

    0x627f7f4c,// 23 PAY  20 

    0x63a693ee,// 24 PAY  21 

    0xac102d8e,// 25 PAY  22 

    0x0978b69c,// 26 PAY  23 

    0x4bafe1aa,// 27 PAY  24 

    0x20c66fff,// 28 PAY  25 

    0x4c8ce3f5,// 29 PAY  26 

    0xdf04c087,// 30 PAY  27 

    0x673677a8,// 31 PAY  28 

    0xc7f57f39,// 32 PAY  29 

    0xb4fc4900,// 33 PAY  30 

    0x16142807,// 34 PAY  31 

    0x039c71c7,// 35 PAY  32 

    0x44839c2d,// 36 PAY  33 

    0xd594214b,// 37 PAY  34 

    0xc00bfdc1,// 38 PAY  35 

    0x4134f64a,// 39 PAY  36 

    0x1f7048e8,// 40 PAY  37 

    0x5a3c99f2,// 41 PAY  38 

    0xfc8d6dea,// 42 PAY  39 

    0x05f23771,// 43 PAY  40 

    0x9e9572b3,// 44 PAY  41 

    0xc7db662f,// 45 PAY  42 

    0xbfe98042,// 46 PAY  43 

    0xa44acd94,// 47 PAY  44 

    0xa89e98f2,// 48 PAY  45 

    0xb97b762b,// 49 PAY  46 

    0xcdad43e7,// 50 PAY  47 

    0x426bc377,// 51 PAY  48 

    0x2e2bf20e,// 52 PAY  49 

    0x12d841a4,// 53 PAY  50 

    0x690f12c1,// 54 PAY  51 

    0x3d83b1b0,// 55 PAY  52 

    0x96ef02fb,// 56 PAY  53 

    0x0484a4e8,// 57 PAY  54 

    0x782b1411,// 58 PAY  55 

    0x8614a3dd,// 59 PAY  56 

    0x2f89ad8e,// 60 PAY  57 

    0xba930e96,// 61 PAY  58 

    0x1a92c54f,// 62 PAY  59 

    0x42578ee0,// 63 PAY  60 

    0x17005f36,// 64 PAY  61 

    0x28699ad3,// 65 PAY  62 

    0x68c6983a,// 66 PAY  63 

    0x788571dc,// 67 PAY  64 

    0xa892d982,// 68 PAY  65 

    0x8c97eaad,// 69 PAY  66 

    0x75441f45,// 70 PAY  67 

    0x4afe68e8,// 71 PAY  68 

    0x062bc37e,// 72 PAY  69 

    0xb9044d27,// 73 PAY  70 

    0x4ef03f43,// 74 PAY  71 

    0x7ecaa805,// 75 PAY  72 

    0x300ec008,// 76 PAY  73 

    0xd5e2de1f,// 77 PAY  74 

    0x1af15388,// 78 PAY  75 

    0xdf44da4c,// 79 PAY  76 

    0xeb72ff0f,// 80 PAY  77 

    0x844d6fbb,// 81 PAY  78 

    0x66b6e2d5,// 82 PAY  79 

    0x503bb1f6,// 83 PAY  80 

    0xc5319a49,// 84 PAY  81 

    0x7ea22b1e,// 85 PAY  82 

    0x3b796e6a,// 86 PAY  83 

    0x2178f5e7,// 87 PAY  84 

    0xd8f2590d,// 88 PAY  85 

    0xde3f605c,// 89 PAY  86 

    0x3012ba37,// 90 PAY  87 

    0xe8541d8d,// 91 PAY  88 

    0xea467812,// 92 PAY  89 

    0xebbb46d0,// 93 PAY  90 

    0x39803c48,// 94 PAY  91 

    0xa51c4675,// 95 PAY  92 

    0x2e469752,// 96 PAY  93 

    0xecde2167,// 97 PAY  94 

    0x3ee5d3d8,// 98 PAY  95 

    0x208bca93,// 99 PAY  96 

    0x5752a3d9,// 100 PAY  97 

    0xfbc3d965,// 101 PAY  98 

    0x03f91209,// 102 PAY  99 

    0x2a9d4b67,// 103 PAY 100 

    0x6331c913,// 104 PAY 101 

    0x8e1c0abf,// 105 PAY 102 

    0x680b03cf,// 106 PAY 103 

    0xb58f5caa,// 107 PAY 104 

    0x48903301,// 108 PAY 105 

    0xd560125d,// 109 PAY 106 

    0xd16939a2,// 110 PAY 107 

    0x8495b648,// 111 PAY 108 

    0x62083b13,// 112 PAY 109 

    0xfe2db590,// 113 PAY 110 

    0x7cb399fd,// 114 PAY 111 

    0xdd277b25,// 115 PAY 112 

    0xd4fedf62,// 116 PAY 113 

    0x4dac0c91,// 117 PAY 114 

    0x5eb1bec0,// 118 PAY 115 

    0x76c38205,// 119 PAY 116 

    0x50b03b5a,// 120 PAY 117 

    0x879d3a22,// 121 PAY 118 

    0xf55619d2,// 122 PAY 119 

    0x2312b2f4,// 123 PAY 120 

    0x8b773bd2,// 124 PAY 121 

    0xabc23b37,// 125 PAY 122 

    0x7597fd38,// 126 PAY 123 

    0x303fcaaf,// 127 PAY 124 

    0xd3e39c74,// 128 PAY 125 

    0x5a973d9b,// 129 PAY 126 

    0xb8333ebe,// 130 PAY 127 

    0x0e5045dd,// 131 PAY 128 

    0x9ce92bce,// 132 PAY 129 

    0x43b78583,// 133 PAY 130 

    0x2073b36c,// 134 PAY 131 

    0x1a903987,// 135 PAY 132 

    0x178f52ba,// 136 PAY 133 

    0x194ba172,// 137 PAY 134 

    0xe49d419a,// 138 PAY 135 

    0x5ec6dae5,// 139 PAY 136 

    0xac24fcf1,// 140 PAY 137 

    0xd4084c25,// 141 PAY 138 

    0xe426b64d,// 142 PAY 139 

    0xe8bd03c6,// 143 PAY 140 

    0x926d1a6d,// 144 PAY 141 

    0x1e705504,// 145 PAY 142 

    0xaebaf796,// 146 PAY 143 

    0x033e66af,// 147 PAY 144 

    0xf93d5d41,// 148 PAY 145 

    0x75f0a1ca,// 149 PAY 146 

    0x223e8d31,// 150 PAY 147 

    0x3219aba0,// 151 PAY 148 

    0x4f3bc1f7,// 152 PAY 149 

    0x3fb9ddf7,// 153 PAY 150 

    0x6c7e49cc,// 154 PAY 151 

    0x01be39e3,// 155 PAY 152 

    0xe18d3000,// 156 PAY 153 

    0x0cc02e2d,// 157 PAY 154 

    0x1e6ba6fd,// 158 PAY 155 

    0x52f8eedd,// 159 PAY 156 

    0x3097c942,// 160 PAY 157 

    0x503f38e5,// 161 PAY 158 

    0x043679ca,// 162 PAY 159 

    0x16e35c24,// 163 PAY 160 

    0x64c037fe,// 164 PAY 161 

    0x7392fc44,// 165 PAY 162 

    0x02643e72,// 166 PAY 163 

    0x2a266637,// 167 PAY 164 

    0x245b18f7,// 168 PAY 165 

    0x1d9ee281,// 169 PAY 166 

    0x88d98109,// 170 PAY 167 

    0x520c733d,// 171 PAY 168 

    0x247d602f,// 172 PAY 169 

    0xa06c3078,// 173 PAY 170 

    0x82c538e3,// 174 PAY 171 

    0x1e75d093,// 175 PAY 172 

    0xfa30c1e5,// 176 PAY 173 

    0x119aa495,// 177 PAY 174 

    0x446dff3c,// 178 PAY 175 

    0x33fa6d3f,// 179 PAY 176 

    0x02270e35,// 180 PAY 177 

    0x1a381bb5,// 181 PAY 178 

    0xd3041eea,// 182 PAY 179 

    0xe98d7c8f,// 183 PAY 180 

    0x98c6508a,// 184 PAY 181 

    0x87ed0d25,// 185 PAY 182 

    0xde9c2b61,// 186 PAY 183 

    0x5b91427d,// 187 PAY 184 

    0x7454f58e,// 188 PAY 185 

    0xe83150cd,// 189 PAY 186 

    0x73239ebf,// 190 PAY 187 

    0x7dc45047,// 191 PAY 188 

    0xd80201f1,// 192 PAY 189 

    0x9f1f81e5,// 193 PAY 190 

    0x6e63d969,// 194 PAY 191 

    0x74fa6165,// 195 PAY 192 

    0x63485d9c,// 196 PAY 193 

    0xbfdbd37c,// 197 PAY 194 

    0xac7634e0,// 198 PAY 195 

    0x2fde3a7d,// 199 PAY 196 

    0xa24c4338,// 200 PAY 197 

    0xc1d29bfa,// 201 PAY 198 

    0x92c803c1,// 202 PAY 199 

    0xa5dac28e,// 203 PAY 200 

    0xfcc84ce2,// 204 PAY 201 

    0x8298b189,// 205 PAY 202 

    0x5530d0ba,// 206 PAY 203 

    0xfb33fa25,// 207 PAY 204 

    0x3012f6b8,// 208 PAY 205 

    0x71d61f74,// 209 PAY 206 

    0x87cd10bd,// 210 PAY 207 

    0x8a43669a,// 211 PAY 208 

    0xb548279f,// 212 PAY 209 

    0x0a8bee17,// 213 PAY 210 

    0xfe389be7,// 214 PAY 211 

    0x7501a92e,// 215 PAY 212 

    0xbc46b148,// 216 PAY 213 

    0x892657bc,// 217 PAY 214 

    0x8f3183c0,// 218 PAY 215 

    0x1c4c7f2d,// 219 PAY 216 

    0xa2abfd45,// 220 PAY 217 

    0xbe950715,// 221 PAY 218 

    0x062cbbf5,// 222 PAY 219 

    0x87f0fa8d,// 223 PAY 220 

    0x167ab394,// 224 PAY 221 

    0xc14b481a,// 225 PAY 222 

    0x6ef857cf,// 226 PAY 223 

    0x3ac294ec,// 227 PAY 224 

    0x96d44e59,// 228 PAY 225 

    0xd9a7499c,// 229 PAY 226 

    0x7d3d4825,// 230 PAY 227 

    0x92ca0eb6,// 231 PAY 228 

    0x00234111,// 232 PAY 229 

    0xc2b7f24d,// 233 PAY 230 

    0xe7b67a63,// 234 PAY 231 

    0xf01663c4,// 235 PAY 232 

    0xb991564f,// 236 PAY 233 

    0x3d1ba290,// 237 PAY 234 

    0xe430d0a7,// 238 PAY 235 

    0xea9e6441,// 239 PAY 236 

    0x3b272914,// 240 PAY 237 

    0x89cbd0ac,// 241 PAY 238 

    0x6881a57a,// 242 PAY 239 

    0xc4472840,// 243 PAY 240 

    0x2dea35ca,// 244 PAY 241 

    0x7cf80498,// 245 PAY 242 

    0xa020d081,// 246 PAY 243 

    0x8cdb55db,// 247 PAY 244 

    0x7f27398c,// 248 PAY 245 

    0x2c91a3ca,// 249 PAY 246 

    0x28639a9e,// 250 PAY 247 

    0x9f804aca,// 251 PAY 248 

    0x14bd2a01,// 252 PAY 249 

    0x823f41a0,// 253 PAY 250 

    0xbfae014d,// 254 PAY 251 

    0xff26fb48,// 255 PAY 252 

    0x63b2e221,// 256 PAY 253 

    0xc924a421,// 257 PAY 254 

    0x3302888b,// 258 PAY 255 

    0xb4a971ba,// 259 PAY 256 

    0x6e1ff127,// 260 PAY 257 

    0x290be738,// 261 PAY 258 

    0xe98c1512,// 262 PAY 259 

    0xca68609f,// 263 PAY 260 

    0x2a1ffc28,// 264 PAY 261 

    0xaf3462da,// 265 PAY 262 

    0x8c0ac601,// 266 PAY 263 

    0x64cece64,// 267 PAY 264 

    0x611691f1,// 268 PAY 265 

    0x78be3a7d,// 269 PAY 266 

    0x3c3244b6,// 270 PAY 267 

    0xb4cbb768,// 271 PAY 268 

    0xc34838ac,// 272 PAY 269 

    0x63e28c9d,// 273 PAY 270 

    0xcff1a796,// 274 PAY 271 

    0x2d163e39,// 275 PAY 272 

    0x3059b99d,// 276 PAY 273 

    0x6f3375cf,// 277 PAY 274 

    0xba57a608,// 278 PAY 275 

    0xf27c1bee,// 279 PAY 276 

    0x829178b0,// 280 PAY 277 

    0x7281af6b,// 281 PAY 278 

    0x6b5b3bee,// 282 PAY 279 

    0x230c65a8,// 283 PAY 280 

    0x71224a56,// 284 PAY 281 

/// HASH is  4 bytes 

    0xfb33fa25,// 285 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 212 

/// STA pkt_idx        : 86 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe6 

    0x0158e6d4 // 286 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt19_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 461 words. 

/// BDA size     is 1837 (0x72d) 

/// BDA id       is 0x426c 

    0x072d426c,// 3 BDA   1 

/// PAY Generic Data size   : 1837 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x3facbe09,// 4 PAY   1 

    0xcdbb6d2b,// 5 PAY   2 

    0x39b0e49c,// 6 PAY   3 

    0x815b64bf,// 7 PAY   4 

    0x85789291,// 8 PAY   5 

    0x9ce6f0a1,// 9 PAY   6 

    0x83ed0a7f,// 10 PAY   7 

    0x4ef415b8,// 11 PAY   8 

    0x69f6b9da,// 12 PAY   9 

    0x82bff0a3,// 13 PAY  10 

    0xae439fdd,// 14 PAY  11 

    0xc9021c7b,// 15 PAY  12 

    0xdb7f81b2,// 16 PAY  13 

    0x069e7a3f,// 17 PAY  14 

    0x1663ad22,// 18 PAY  15 

    0xb17a79bd,// 19 PAY  16 

    0x595ae63d,// 20 PAY  17 

    0x5c91d396,// 21 PAY  18 

    0x9cdaa1b9,// 22 PAY  19 

    0xdb2eaee9,// 23 PAY  20 

    0x45890267,// 24 PAY  21 

    0xadfda7b6,// 25 PAY  22 

    0x996274c7,// 26 PAY  23 

    0x8801b5be,// 27 PAY  24 

    0xa988ae46,// 28 PAY  25 

    0xf60c0368,// 29 PAY  26 

    0xfe2e7655,// 30 PAY  27 

    0x13f88942,// 31 PAY  28 

    0xa9e3f69f,// 32 PAY  29 

    0x61285de8,// 33 PAY  30 

    0xfddb9292,// 34 PAY  31 

    0xbfb55955,// 35 PAY  32 

    0x5316f052,// 36 PAY  33 

    0xe6de9487,// 37 PAY  34 

    0x3c801984,// 38 PAY  35 

    0x12dd19bc,// 39 PAY  36 

    0x1bf365b2,// 40 PAY  37 

    0x4377a2d0,// 41 PAY  38 

    0x1bdac452,// 42 PAY  39 

    0x422f788b,// 43 PAY  40 

    0x870aa717,// 44 PAY  41 

    0xacfd8df0,// 45 PAY  42 

    0x847a37f2,// 46 PAY  43 

    0x45be2d78,// 47 PAY  44 

    0x1cac1f31,// 48 PAY  45 

    0x9507b999,// 49 PAY  46 

    0xa4adc4e8,// 50 PAY  47 

    0x49a30807,// 51 PAY  48 

    0x3cc81bed,// 52 PAY  49 

    0xf7bd8708,// 53 PAY  50 

    0x3e9c84f8,// 54 PAY  51 

    0xd9e0a02a,// 55 PAY  52 

    0x097f9c16,// 56 PAY  53 

    0x2f2fa799,// 57 PAY  54 

    0xbb6d8ae3,// 58 PAY  55 

    0x74ab75ec,// 59 PAY  56 

    0xb96f3882,// 60 PAY  57 

    0xd39eda1d,// 61 PAY  58 

    0x7052106f,// 62 PAY  59 

    0xca323d60,// 63 PAY  60 

    0xfd275ffe,// 64 PAY  61 

    0xd5dc3c0f,// 65 PAY  62 

    0xe7a0624f,// 66 PAY  63 

    0x9cdc4a6b,// 67 PAY  64 

    0xe3f9880c,// 68 PAY  65 

    0x51297ee0,// 69 PAY  66 

    0x928706db,// 70 PAY  67 

    0x1bc1aa79,// 71 PAY  68 

    0x4952c2db,// 72 PAY  69 

    0x41005cb0,// 73 PAY  70 

    0x5a1ebfe0,// 74 PAY  71 

    0x7c498b73,// 75 PAY  72 

    0x127dbd3a,// 76 PAY  73 

    0x8b0ed761,// 77 PAY  74 

    0xa9ab0902,// 78 PAY  75 

    0x835ab227,// 79 PAY  76 

    0xd11b13a8,// 80 PAY  77 

    0x603a6984,// 81 PAY  78 

    0xe7291a75,// 82 PAY  79 

    0x6e7275d6,// 83 PAY  80 

    0x912bd26a,// 84 PAY  81 

    0x3c9a71ea,// 85 PAY  82 

    0xc7beaa36,// 86 PAY  83 

    0x782f0d23,// 87 PAY  84 

    0xd5586957,// 88 PAY  85 

    0xf122c740,// 89 PAY  86 

    0x0d6973d2,// 90 PAY  87 

    0x3671fbbd,// 91 PAY  88 

    0xbd106fe0,// 92 PAY  89 

    0xcb787cc0,// 93 PAY  90 

    0x7b2283ae,// 94 PAY  91 

    0xb5e6451f,// 95 PAY  92 

    0xe76012cf,// 96 PAY  93 

    0x177c19a1,// 97 PAY  94 

    0x3fb7666c,// 98 PAY  95 

    0x6cc4f0d0,// 99 PAY  96 

    0xa554428b,// 100 PAY  97 

    0xafa729c1,// 101 PAY  98 

    0x6f1c5d93,// 102 PAY  99 

    0x7706bc87,// 103 PAY 100 

    0x9775266e,// 104 PAY 101 

    0x14aff09b,// 105 PAY 102 

    0x4c40f945,// 106 PAY 103 

    0x853ce225,// 107 PAY 104 

    0x8519bfca,// 108 PAY 105 

    0x556c9544,// 109 PAY 106 

    0x49649a94,// 110 PAY 107 

    0x4986aac1,// 111 PAY 108 

    0x72cedfb2,// 112 PAY 109 

    0x71f66b99,// 113 PAY 110 

    0x5f51181f,// 114 PAY 111 

    0x441d44eb,// 115 PAY 112 

    0x9e9112b2,// 116 PAY 113 

    0x33a7f217,// 117 PAY 114 

    0xb53ea8aa,// 118 PAY 115 

    0x86611028,// 119 PAY 116 

    0xf734ea5d,// 120 PAY 117 

    0xb38a8cb5,// 121 PAY 118 

    0x26fe7c68,// 122 PAY 119 

    0x3eeac03d,// 123 PAY 120 

    0x295065e8,// 124 PAY 121 

    0x77d41ca5,// 125 PAY 122 

    0xa22ee64c,// 126 PAY 123 

    0xaf6171c3,// 127 PAY 124 

    0xcada88af,// 128 PAY 125 

    0xd8b4bc38,// 129 PAY 126 

    0xffa3376b,// 130 PAY 127 

    0x6c87a5e5,// 131 PAY 128 

    0xc1f9fc2e,// 132 PAY 129 

    0x25df8659,// 133 PAY 130 

    0x646d30da,// 134 PAY 131 

    0x1ace77fd,// 135 PAY 132 

    0xe0f0d6e6,// 136 PAY 133 

    0xb01292c0,// 137 PAY 134 

    0x50b04287,// 138 PAY 135 

    0x79d0ccfe,// 139 PAY 136 

    0x1d4807eb,// 140 PAY 137 

    0x6190e082,// 141 PAY 138 

    0x1607f844,// 142 PAY 139 

    0xd3d106e5,// 143 PAY 140 

    0x4c6debe5,// 144 PAY 141 

    0x0335c296,// 145 PAY 142 

    0xc0a8e5b5,// 146 PAY 143 

    0xdef83bed,// 147 PAY 144 

    0x1c69fe30,// 148 PAY 145 

    0x934be7c8,// 149 PAY 146 

    0xf679651c,// 150 PAY 147 

    0xda105320,// 151 PAY 148 

    0x75724f28,// 152 PAY 149 

    0x02ba313f,// 153 PAY 150 

    0x13b03e59,// 154 PAY 151 

    0x5848170b,// 155 PAY 152 

    0xeb7dbf5e,// 156 PAY 153 

    0xcadbb465,// 157 PAY 154 

    0x071c522b,// 158 PAY 155 

    0x83d669a2,// 159 PAY 156 

    0x15162e55,// 160 PAY 157 

    0xe822d944,// 161 PAY 158 

    0xb1b620ec,// 162 PAY 159 

    0xffa843ef,// 163 PAY 160 

    0xf6f4ef3b,// 164 PAY 161 

    0x794e8d49,// 165 PAY 162 

    0x7945e10f,// 166 PAY 163 

    0xab4181c6,// 167 PAY 164 

    0x0cc56a9c,// 168 PAY 165 

    0xb36323ce,// 169 PAY 166 

    0xcc69c423,// 170 PAY 167 

    0xb6d830ed,// 171 PAY 168 

    0x457ba40c,// 172 PAY 169 

    0x6b1db0f0,// 173 PAY 170 

    0x976b90f3,// 174 PAY 171 

    0x2369c3dd,// 175 PAY 172 

    0x19ecf44a,// 176 PAY 173 

    0x9e5f82b9,// 177 PAY 174 

    0x4bc9eefa,// 178 PAY 175 

    0x711bab12,// 179 PAY 176 

    0x637d67df,// 180 PAY 177 

    0x6782b247,// 181 PAY 178 

    0x04f52387,// 182 PAY 179 

    0x145ff7bb,// 183 PAY 180 

    0xad20534e,// 184 PAY 181 

    0x9006ba08,// 185 PAY 182 

    0x6eb00cb5,// 186 PAY 183 

    0x556976c4,// 187 PAY 184 

    0xde601892,// 188 PAY 185 

    0xf32f7f4f,// 189 PAY 186 

    0xa946adee,// 190 PAY 187 

    0x3f2ce27d,// 191 PAY 188 

    0xe045c738,// 192 PAY 189 

    0x292758ac,// 193 PAY 190 

    0xa509bf2f,// 194 PAY 191 

    0x489dec4e,// 195 PAY 192 

    0x0d167e1c,// 196 PAY 193 

    0x4e32a9f6,// 197 PAY 194 

    0xc0b55451,// 198 PAY 195 

    0x5cb24baa,// 199 PAY 196 

    0x8d00b837,// 200 PAY 197 

    0x30005192,// 201 PAY 198 

    0x1f7e301e,// 202 PAY 199 

    0xdaa9a4e6,// 203 PAY 200 

    0x7ed09d51,// 204 PAY 201 

    0x2a2f27d6,// 205 PAY 202 

    0x9d7eca9f,// 206 PAY 203 

    0xf5900621,// 207 PAY 204 

    0x4e632dcb,// 208 PAY 205 

    0xd5eec8cc,// 209 PAY 206 

    0x6a2ab7bf,// 210 PAY 207 

    0xe8024f36,// 211 PAY 208 

    0xd580a2bd,// 212 PAY 209 

    0xaa35a504,// 213 PAY 210 

    0xeed0f292,// 214 PAY 211 

    0x0c726b97,// 215 PAY 212 

    0xe3da8cb6,// 216 PAY 213 

    0x7451bd11,// 217 PAY 214 

    0xeab90919,// 218 PAY 215 

    0x77885c44,// 219 PAY 216 

    0x9632c86b,// 220 PAY 217 

    0x91d573ae,// 221 PAY 218 

    0x6ae94b19,// 222 PAY 219 

    0xd117ac5a,// 223 PAY 220 

    0x94807589,// 224 PAY 221 

    0x3b297137,// 225 PAY 222 

    0xfca9c940,// 226 PAY 223 

    0x2de51014,// 227 PAY 224 

    0xd6033712,// 228 PAY 225 

    0x3c8cd54c,// 229 PAY 226 

    0x59f9becb,// 230 PAY 227 

    0xce21b2c5,// 231 PAY 228 

    0x250e3f2e,// 232 PAY 229 

    0x4d997816,// 233 PAY 230 

    0xe514f44c,// 234 PAY 231 

    0x7ea33cc9,// 235 PAY 232 

    0x810fe434,// 236 PAY 233 

    0xa574fb30,// 237 PAY 234 

    0x2d5692ba,// 238 PAY 235 

    0x7fb3a1e4,// 239 PAY 236 

    0x738503a6,// 240 PAY 237 

    0xd4a5512c,// 241 PAY 238 

    0x8bccb08a,// 242 PAY 239 

    0x6b704178,// 243 PAY 240 

    0x04d68e1a,// 244 PAY 241 

    0x17824d41,// 245 PAY 242 

    0xf650ad21,// 246 PAY 243 

    0x1852b9c0,// 247 PAY 244 

    0xe050826d,// 248 PAY 245 

    0xb89269d2,// 249 PAY 246 

    0xf13e0279,// 250 PAY 247 

    0x11c32130,// 251 PAY 248 

    0x61cdfd4d,// 252 PAY 249 

    0x096a1583,// 253 PAY 250 

    0x258d45e1,// 254 PAY 251 

    0x41bf2b58,// 255 PAY 252 

    0xf540273b,// 256 PAY 253 

    0xa6a05037,// 257 PAY 254 

    0x24d0be8a,// 258 PAY 255 

    0x6d9380a6,// 259 PAY 256 

    0x9d79e06f,// 260 PAY 257 

    0xf9fdecf3,// 261 PAY 258 

    0x1502e2d2,// 262 PAY 259 

    0x2cda8116,// 263 PAY 260 

    0x8baf740c,// 264 PAY 261 

    0xd847cdf4,// 265 PAY 262 

    0xe8748926,// 266 PAY 263 

    0x3463db56,// 267 PAY 264 

    0x4a4e66a4,// 268 PAY 265 

    0x5457a7b3,// 269 PAY 266 

    0x88b6c232,// 270 PAY 267 

    0xa7741fe9,// 271 PAY 268 

    0x6dda63ec,// 272 PAY 269 

    0x0bc9f215,// 273 PAY 270 

    0x849bafe0,// 274 PAY 271 

    0x17f4899e,// 275 PAY 272 

    0x2cd1bf4e,// 276 PAY 273 

    0x54e292b4,// 277 PAY 274 

    0x54f76479,// 278 PAY 275 

    0xd3467e69,// 279 PAY 276 

    0xdb21c505,// 280 PAY 277 

    0x30b0227a,// 281 PAY 278 

    0xb299efea,// 282 PAY 279 

    0x4fb3a965,// 283 PAY 280 

    0x97934319,// 284 PAY 281 

    0x2b5832f7,// 285 PAY 282 

    0xb82167d4,// 286 PAY 283 

    0x612fe265,// 287 PAY 284 

    0x7f2cd47b,// 288 PAY 285 

    0xc2ceb9e1,// 289 PAY 286 

    0xccb177f4,// 290 PAY 287 

    0xe999b969,// 291 PAY 288 

    0xc1592666,// 292 PAY 289 

    0xecaf254e,// 293 PAY 290 

    0x666fea3b,// 294 PAY 291 

    0x19348eaa,// 295 PAY 292 

    0x15e9f39a,// 296 PAY 293 

    0x6c45eba9,// 297 PAY 294 

    0xeaa0086b,// 298 PAY 295 

    0x8f848f52,// 299 PAY 296 

    0x5ee4f389,// 300 PAY 297 

    0x76e934c3,// 301 PAY 298 

    0xb1c1f0a6,// 302 PAY 299 

    0x23d261df,// 303 PAY 300 

    0xaffdbe5c,// 304 PAY 301 

    0x566f38fb,// 305 PAY 302 

    0x2531814e,// 306 PAY 303 

    0xcc12d4cd,// 307 PAY 304 

    0xd4abc50d,// 308 PAY 305 

    0xc0dbd608,// 309 PAY 306 

    0x96647e86,// 310 PAY 307 

    0xf9caaf09,// 311 PAY 308 

    0xd3d721f6,// 312 PAY 309 

    0x0047f0db,// 313 PAY 310 

    0x8d5bf342,// 314 PAY 311 

    0x61605d3c,// 315 PAY 312 

    0x83d69abc,// 316 PAY 313 

    0xa4fe8fd0,// 317 PAY 314 

    0x06ce1c53,// 318 PAY 315 

    0xf02c0d65,// 319 PAY 316 

    0xd864f4f6,// 320 PAY 317 

    0xff76e4ab,// 321 PAY 318 

    0xd769aa85,// 322 PAY 319 

    0x92bba86b,// 323 PAY 320 

    0xca728b81,// 324 PAY 321 

    0x53856ffb,// 325 PAY 322 

    0xc075fdc0,// 326 PAY 323 

    0xd2eac4d9,// 327 PAY 324 

    0x8178e117,// 328 PAY 325 

    0xf71a62d1,// 329 PAY 326 

    0x34f851cc,// 330 PAY 327 

    0x6ea311d9,// 331 PAY 328 

    0xc1ca12b9,// 332 PAY 329 

    0xe9f8b1a1,// 333 PAY 330 

    0x822f3c6f,// 334 PAY 331 

    0x6165d521,// 335 PAY 332 

    0xc763df63,// 336 PAY 333 

    0x0573203d,// 337 PAY 334 

    0x8c66cfa3,// 338 PAY 335 

    0xcba1ffaf,// 339 PAY 336 

    0x0158d090,// 340 PAY 337 

    0x4a29d551,// 341 PAY 338 

    0x2c9d4a1d,// 342 PAY 339 

    0xa12df3e0,// 343 PAY 340 

    0x2fa5b4d2,// 344 PAY 341 

    0x78ae221d,// 345 PAY 342 

    0xeaa853b9,// 346 PAY 343 

    0x923bb4ef,// 347 PAY 344 

    0x1b1313aa,// 348 PAY 345 

    0x85c93893,// 349 PAY 346 

    0xc5dc275f,// 350 PAY 347 

    0xb3289328,// 351 PAY 348 

    0x101560e0,// 352 PAY 349 

    0x97063835,// 353 PAY 350 

    0xe553a50a,// 354 PAY 351 

    0xe66f2cbf,// 355 PAY 352 

    0x432d1b8b,// 356 PAY 353 

    0x6822bf05,// 357 PAY 354 

    0x21286d47,// 358 PAY 355 

    0x77e933d0,// 359 PAY 356 

    0x858db6e8,// 360 PAY 357 

    0xf37f63c8,// 361 PAY 358 

    0xe5661333,// 362 PAY 359 

    0xe4976bfc,// 363 PAY 360 

    0xcaf747db,// 364 PAY 361 

    0xa64db82f,// 365 PAY 362 

    0x1166b3e4,// 366 PAY 363 

    0x9ccdc58f,// 367 PAY 364 

    0xba5a12f1,// 368 PAY 365 

    0xc618bf1b,// 369 PAY 366 

    0xcc36cc3d,// 370 PAY 367 

    0xbf72bc77,// 371 PAY 368 

    0x52ed55c0,// 372 PAY 369 

    0x8e1ab105,// 373 PAY 370 

    0x3688dd13,// 374 PAY 371 

    0xb6179283,// 375 PAY 372 

    0x2607c980,// 376 PAY 373 

    0x55db37f6,// 377 PAY 374 

    0xc5995747,// 378 PAY 375 

    0x3c698a25,// 379 PAY 376 

    0xd20a0a5a,// 380 PAY 377 

    0x6fd67a34,// 381 PAY 378 

    0x824930a4,// 382 PAY 379 

    0x5557f5aa,// 383 PAY 380 

    0x190428ad,// 384 PAY 381 

    0x735a3ba3,// 385 PAY 382 

    0x671d47a2,// 386 PAY 383 

    0x25254f7b,// 387 PAY 384 

    0xe260a675,// 388 PAY 385 

    0xa7093243,// 389 PAY 386 

    0x14e9490b,// 390 PAY 387 

    0xddcd86f4,// 391 PAY 388 

    0xad681ec2,// 392 PAY 389 

    0xd0adfa14,// 393 PAY 390 

    0x2f2d20b9,// 394 PAY 391 

    0x28cfae69,// 395 PAY 392 

    0xb35a4a74,// 396 PAY 393 

    0x795b0c26,// 397 PAY 394 

    0x1115658f,// 398 PAY 395 

    0xb49b1aac,// 399 PAY 396 

    0x978841c0,// 400 PAY 397 

    0x391fb94c,// 401 PAY 398 

    0x72967f67,// 402 PAY 399 

    0x6f15c1e3,// 403 PAY 400 

    0xf04e556d,// 404 PAY 401 

    0xd3dd8314,// 405 PAY 402 

    0x8f3fe05f,// 406 PAY 403 

    0xb63df964,// 407 PAY 404 

    0x0ad9cf57,// 408 PAY 405 

    0x071785be,// 409 PAY 406 

    0x689f0803,// 410 PAY 407 

    0xe7a23c43,// 411 PAY 408 

    0xfbcde3af,// 412 PAY 409 

    0x2ffd489c,// 413 PAY 410 

    0x6e5ba174,// 414 PAY 411 

    0x8dbc916e,// 415 PAY 412 

    0x4acd687b,// 416 PAY 413 

    0xea989e2d,// 417 PAY 414 

    0x02650c62,// 418 PAY 415 

    0x02a46c61,// 419 PAY 416 

    0x5cd7b506,// 420 PAY 417 

    0xd6b3d156,// 421 PAY 418 

    0xc133b6b5,// 422 PAY 419 

    0x88a58306,// 423 PAY 420 

    0x0d7e8d5a,// 424 PAY 421 

    0x4df92ef1,// 425 PAY 422 

    0x695c3305,// 426 PAY 423 

    0xa263ae94,// 427 PAY 424 

    0x65cbdbfe,// 428 PAY 425 

    0x1cfcbaf5,// 429 PAY 426 

    0xac324cd3,// 430 PAY 427 

    0x02ab6017,// 431 PAY 428 

    0x123458c3,// 432 PAY 429 

    0xd240a84a,// 433 PAY 430 

    0x1a706150,// 434 PAY 431 

    0xffcb737c,// 435 PAY 432 

    0xe8aa824d,// 436 PAY 433 

    0x8c3bc5df,// 437 PAY 434 

    0x7ee74647,// 438 PAY 435 

    0xa120db3d,// 439 PAY 436 

    0x93c132c8,// 440 PAY 437 

    0x1bdad2db,// 441 PAY 438 

    0xdbb0530f,// 442 PAY 439 

    0xdc7f8239,// 443 PAY 440 

    0xd704ef46,// 444 PAY 441 

    0x383d0f21,// 445 PAY 442 

    0xf1530a66,// 446 PAY 443 

    0x459bc656,// 447 PAY 444 

    0x12ab6882,// 448 PAY 445 

    0x83e68c34,// 449 PAY 446 

    0xf36ead2c,// 450 PAY 447 

    0x1789e979,// 451 PAY 448 

    0xaad83033,// 452 PAY 449 

    0x591d8383,// 453 PAY 450 

    0x51b019b2,// 454 PAY 451 

    0x3530a378,// 455 PAY 452 

    0xd9bca042,// 456 PAY 453 

    0x43546c89,// 457 PAY 454 

    0x72883e5b,// 458 PAY 455 

    0x3df9bc6d,// 459 PAY 456 

    0x3e62106b,// 460 PAY 457 

    0x3bab668b,// 461 PAY 458 

    0x9f316da3,// 462 PAY 459 

    0xfd000000,// 463 PAY 460 

/// HASH is  20 bytes 

    0x4fb3a965,// 464 HSH   1 

    0x97934319,// 465 HSH   2 

    0x2b5832f7,// 466 HSH   3 

    0xb82167d4,// 467 HSH   4 

    0x612fe265,// 468 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 27 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02cc3c1b // 469 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt20_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 376 words. 

/// BDA size     is 1500 (0x5dc) 

/// BDA id       is 0x1692 

    0x05dc1692,// 3 BDA   1 

/// PAY Generic Data size   : 1500 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xf9c1d470,// 4 PAY   1 

    0x29aa7bc2,// 5 PAY   2 

    0x24b63403,// 6 PAY   3 

    0xf2049d91,// 7 PAY   4 

    0x3e19d1f3,// 8 PAY   5 

    0xcd33bba5,// 9 PAY   6 

    0x04154796,// 10 PAY   7 

    0x61f2a978,// 11 PAY   8 

    0x8e121b63,// 12 PAY   9 

    0xeaf42c49,// 13 PAY  10 

    0x6e0c5d65,// 14 PAY  11 

    0x2ac16ae5,// 15 PAY  12 

    0xb1cb6316,// 16 PAY  13 

    0x6c1f5479,// 17 PAY  14 

    0xd3bdd75f,// 18 PAY  15 

    0x6a09d672,// 19 PAY  16 

    0xa6fbf972,// 20 PAY  17 

    0x5a362ac7,// 21 PAY  18 

    0x3d3c8a13,// 22 PAY  19 

    0x2e095bce,// 23 PAY  20 

    0xa84101aa,// 24 PAY  21 

    0x70546116,// 25 PAY  22 

    0xd65613ee,// 26 PAY  23 

    0x6e97b9cb,// 27 PAY  24 

    0x38ee7556,// 28 PAY  25 

    0x0fa4533d,// 29 PAY  26 

    0x43aff3b7,// 30 PAY  27 

    0xd6d8a8fe,// 31 PAY  28 

    0xd6c84c56,// 32 PAY  29 

    0x435b8673,// 33 PAY  30 

    0xa81b21d4,// 34 PAY  31 

    0x78ddb9d8,// 35 PAY  32 

    0xf177e7f6,// 36 PAY  33 

    0x624f6623,// 37 PAY  34 

    0x7fd20f00,// 38 PAY  35 

    0x78683270,// 39 PAY  36 

    0x05f7b962,// 40 PAY  37 

    0x9fed3d89,// 41 PAY  38 

    0x58bbc8a9,// 42 PAY  39 

    0x29bf9241,// 43 PAY  40 

    0xcb21b67b,// 44 PAY  41 

    0x02e64205,// 45 PAY  42 

    0x147dc6b3,// 46 PAY  43 

    0x2bc0d7bf,// 47 PAY  44 

    0xfb657218,// 48 PAY  45 

    0xf993871f,// 49 PAY  46 

    0x405168e5,// 50 PAY  47 

    0x97e41617,// 51 PAY  48 

    0xe05e06d6,// 52 PAY  49 

    0x503b19ad,// 53 PAY  50 

    0x1f0ec989,// 54 PAY  51 

    0x45058cd2,// 55 PAY  52 

    0xf4e1185f,// 56 PAY  53 

    0xc4ae5eff,// 57 PAY  54 

    0x080d5c11,// 58 PAY  55 

    0x6161f607,// 59 PAY  56 

    0x4817e38c,// 60 PAY  57 

    0x719d5669,// 61 PAY  58 

    0x60eb33b7,// 62 PAY  59 

    0x59230c9c,// 63 PAY  60 

    0x2617860f,// 64 PAY  61 

    0xdff05d10,// 65 PAY  62 

    0xafb54a32,// 66 PAY  63 

    0xde29b200,// 67 PAY  64 

    0x64baf386,// 68 PAY  65 

    0xbb4ddecd,// 69 PAY  66 

    0x822c3a2a,// 70 PAY  67 

    0x47c8b2c2,// 71 PAY  68 

    0xfde6ba05,// 72 PAY  69 

    0x65ac8446,// 73 PAY  70 

    0xcf89eb07,// 74 PAY  71 

    0x6d6b3946,// 75 PAY  72 

    0xd9192196,// 76 PAY  73 

    0x09f4c9d2,// 77 PAY  74 

    0x0cc08864,// 78 PAY  75 

    0x9a40435f,// 79 PAY  76 

    0xa74ed7b4,// 80 PAY  77 

    0x0245ac17,// 81 PAY  78 

    0x43ec83ac,// 82 PAY  79 

    0x49b8d962,// 83 PAY  80 

    0xb52e938d,// 84 PAY  81 

    0xe42e45d9,// 85 PAY  82 

    0x5a9e5e4c,// 86 PAY  83 

    0xb8142ffc,// 87 PAY  84 

    0x585fb562,// 88 PAY  85 

    0xfbca8810,// 89 PAY  86 

    0x1bc2d526,// 90 PAY  87 

    0x9696dbfd,// 91 PAY  88 

    0x0caa88a8,// 92 PAY  89 

    0xacdfd7b0,// 93 PAY  90 

    0x6c61857c,// 94 PAY  91 

    0xa23b6f30,// 95 PAY  92 

    0x750e55d8,// 96 PAY  93 

    0x0795c718,// 97 PAY  94 

    0xa6460fa4,// 98 PAY  95 

    0xb919857c,// 99 PAY  96 

    0x42e3c066,// 100 PAY  97 

    0xfe1a69db,// 101 PAY  98 

    0xd1394d0c,// 102 PAY  99 

    0xb22efe1e,// 103 PAY 100 

    0x0fb59192,// 104 PAY 101 

    0x411a39f5,// 105 PAY 102 

    0x600b3bac,// 106 PAY 103 

    0xbda712dc,// 107 PAY 104 

    0x033bbf79,// 108 PAY 105 

    0x8d720e73,// 109 PAY 106 

    0x90c52c11,// 110 PAY 107 

    0x7b074b5b,// 111 PAY 108 

    0x11635b4b,// 112 PAY 109 

    0xf61b2e56,// 113 PAY 110 

    0x1cd6f4c0,// 114 PAY 111 

    0x4eded6c8,// 115 PAY 112 

    0x81a54c4d,// 116 PAY 113 

    0x3a5329b0,// 117 PAY 114 

    0x9e408791,// 118 PAY 115 

    0x46f81498,// 119 PAY 116 

    0xce15539e,// 120 PAY 117 

    0xe5d0a4ce,// 121 PAY 118 

    0x7012cdb1,// 122 PAY 119 

    0x7147741f,// 123 PAY 120 

    0x9aa39799,// 124 PAY 121 

    0xf3dee3fe,// 125 PAY 122 

    0x30eebdf9,// 126 PAY 123 

    0x268b7c93,// 127 PAY 124 

    0x66aa2cff,// 128 PAY 125 

    0x9627cbb2,// 129 PAY 126 

    0x72435d4e,// 130 PAY 127 

    0xb75d6187,// 131 PAY 128 

    0xd7cef32f,// 132 PAY 129 

    0x2421b92d,// 133 PAY 130 

    0x691689c3,// 134 PAY 131 

    0xaba18801,// 135 PAY 132 

    0x2814a9cd,// 136 PAY 133 

    0x40b3c0dc,// 137 PAY 134 

    0xf22df363,// 138 PAY 135 

    0xf10488ad,// 139 PAY 136 

    0xb20bc1aa,// 140 PAY 137 

    0xb0d05aee,// 141 PAY 138 

    0x5a834f2f,// 142 PAY 139 

    0x2ca4165c,// 143 PAY 140 

    0xff6b4354,// 144 PAY 141 

    0xb363dbbf,// 145 PAY 142 

    0xb0dce2ce,// 146 PAY 143 

    0xeb891d86,// 147 PAY 144 

    0x8847964b,// 148 PAY 145 

    0x55e699f6,// 149 PAY 146 

    0x863aa4e8,// 150 PAY 147 

    0xd054d6ac,// 151 PAY 148 

    0x9b46a27b,// 152 PAY 149 

    0x8e8d22bd,// 153 PAY 150 

    0xdb1e684b,// 154 PAY 151 

    0xfaea1aa1,// 155 PAY 152 

    0x6efb5b18,// 156 PAY 153 

    0xc62f8187,// 157 PAY 154 

    0xb169283e,// 158 PAY 155 

    0xdea18f38,// 159 PAY 156 

    0x7c54ee1d,// 160 PAY 157 

    0x3deb6f49,// 161 PAY 158 

    0x913bf9c7,// 162 PAY 159 

    0xd71943a0,// 163 PAY 160 

    0x5236997c,// 164 PAY 161 

    0xefb4cbb7,// 165 PAY 162 

    0x8675a6fe,// 166 PAY 163 

    0x100e721f,// 167 PAY 164 

    0xe482b4fc,// 168 PAY 165 

    0x8a7aefb8,// 169 PAY 166 

    0x7fcea9b9,// 170 PAY 167 

    0x2d0a576b,// 171 PAY 168 

    0xbddd229c,// 172 PAY 169 

    0x631aef47,// 173 PAY 170 

    0xc6164e52,// 174 PAY 171 

    0x4b9ace60,// 175 PAY 172 

    0xd34e5ae3,// 176 PAY 173 

    0x7df6b48b,// 177 PAY 174 

    0xabfe1ec2,// 178 PAY 175 

    0x901b8241,// 179 PAY 176 

    0x8720ccdf,// 180 PAY 177 

    0x3a4dd39d,// 181 PAY 178 

    0x58e4ec93,// 182 PAY 179 

    0xb206fe74,// 183 PAY 180 

    0x367e0c49,// 184 PAY 181 

    0xe38403f5,// 185 PAY 182 

    0xdc31e3c5,// 186 PAY 183 

    0x5f58f2be,// 187 PAY 184 

    0x7827337a,// 188 PAY 185 

    0x8bfe249a,// 189 PAY 186 

    0x082994da,// 190 PAY 187 

    0xab8f694b,// 191 PAY 188 

    0xec0455d7,// 192 PAY 189 

    0x730a0ccf,// 193 PAY 190 

    0xa987cfbe,// 194 PAY 191 

    0x67409e6a,// 195 PAY 192 

    0xacbbb3a0,// 196 PAY 193 

    0x5e849e86,// 197 PAY 194 

    0x4f8c997a,// 198 PAY 195 

    0x1c288e4d,// 199 PAY 196 

    0xc43fb796,// 200 PAY 197 

    0xf67542fc,// 201 PAY 198 

    0xfac9db84,// 202 PAY 199 

    0x12403215,// 203 PAY 200 

    0x8ceb1388,// 204 PAY 201 

    0xa7b01db4,// 205 PAY 202 

    0xe89ffae8,// 206 PAY 203 

    0xb05372d6,// 207 PAY 204 

    0xa4947a88,// 208 PAY 205 

    0x8211d4f0,// 209 PAY 206 

    0x01ffb03b,// 210 PAY 207 

    0xb15bfcdc,// 211 PAY 208 

    0xa00dec10,// 212 PAY 209 

    0x0f07a8d1,// 213 PAY 210 

    0xab2ada92,// 214 PAY 211 

    0xd6a3dc94,// 215 PAY 212 

    0xfa65c286,// 216 PAY 213 

    0x23086f05,// 217 PAY 214 

    0xfe6f3069,// 218 PAY 215 

    0x3c3c693a,// 219 PAY 216 

    0x054784a0,// 220 PAY 217 

    0x31074d06,// 221 PAY 218 

    0xb5684b75,// 222 PAY 219 

    0x3da12273,// 223 PAY 220 

    0x9ed9e0ec,// 224 PAY 221 

    0x030bfa87,// 225 PAY 222 

    0xd8032d8d,// 226 PAY 223 

    0x57fa1918,// 227 PAY 224 

    0x1aad5a56,// 228 PAY 225 

    0x945b1333,// 229 PAY 226 

    0x8dd48205,// 230 PAY 227 

    0x27e7fc1f,// 231 PAY 228 

    0x28d83440,// 232 PAY 229 

    0x565f1ca1,// 233 PAY 230 

    0xecc970b7,// 234 PAY 231 

    0x4051b677,// 235 PAY 232 

    0x8314ccd8,// 236 PAY 233 

    0xc5eb6f6a,// 237 PAY 234 

    0x0ff07c96,// 238 PAY 235 

    0xadfc5516,// 239 PAY 236 

    0xcc4e41da,// 240 PAY 237 

    0x196b48cb,// 241 PAY 238 

    0xd08775aa,// 242 PAY 239 

    0x0f402a63,// 243 PAY 240 

    0x44916331,// 244 PAY 241 

    0x041f0520,// 245 PAY 242 

    0x6bffce90,// 246 PAY 243 

    0x0f7011a5,// 247 PAY 244 

    0xecfa8423,// 248 PAY 245 

    0x9f258aa5,// 249 PAY 246 

    0x281cbf17,// 250 PAY 247 

    0x9ba3f553,// 251 PAY 248 

    0xbc069319,// 252 PAY 249 

    0x61955b5a,// 253 PAY 250 

    0x6a92dddf,// 254 PAY 251 

    0xfa1589cb,// 255 PAY 252 

    0xae124ae5,// 256 PAY 253 

    0xe0b8618b,// 257 PAY 254 

    0x53117441,// 258 PAY 255 

    0x56b0ddc8,// 259 PAY 256 

    0x5f3865ad,// 260 PAY 257 

    0x7da0d06c,// 261 PAY 258 

    0xead315f0,// 262 PAY 259 

    0xbcdc841d,// 263 PAY 260 

    0x26acda4f,// 264 PAY 261 

    0x394bd7a0,// 265 PAY 262 

    0x8c8dbe70,// 266 PAY 263 

    0x5e018770,// 267 PAY 264 

    0xe9182a4a,// 268 PAY 265 

    0xca7250b3,// 269 PAY 266 

    0x4441250c,// 270 PAY 267 

    0xd3c15dfd,// 271 PAY 268 

    0x5fa097f0,// 272 PAY 269 

    0x93ed43cc,// 273 PAY 270 

    0x1ccc5d94,// 274 PAY 271 

    0x2ec5f415,// 275 PAY 272 

    0x723d2198,// 276 PAY 273 

    0xde1c9150,// 277 PAY 274 

    0x83cbf64d,// 278 PAY 275 

    0x7072ff52,// 279 PAY 276 

    0xaef836a0,// 280 PAY 277 

    0x79ba1ae1,// 281 PAY 278 

    0x69becff5,// 282 PAY 279 

    0x3a1c0ac0,// 283 PAY 280 

    0xa9998935,// 284 PAY 281 

    0x608f00db,// 285 PAY 282 

    0xbcd8b78f,// 286 PAY 283 

    0xd64f1d54,// 287 PAY 284 

    0xcffc5051,// 288 PAY 285 

    0x40a5e207,// 289 PAY 286 

    0xc0b4458c,// 290 PAY 287 

    0x08611fdc,// 291 PAY 288 

    0x72948bed,// 292 PAY 289 

    0x07233761,// 293 PAY 290 

    0x66350b8d,// 294 PAY 291 

    0x19c61606,// 295 PAY 292 

    0xd72eb0de,// 296 PAY 293 

    0x2850fada,// 297 PAY 294 

    0x5d349cba,// 298 PAY 295 

    0xcc9f9f6b,// 299 PAY 296 

    0xe015e97f,// 300 PAY 297 

    0x22c12daa,// 301 PAY 298 

    0xab9ad980,// 302 PAY 299 

    0x2642dde8,// 303 PAY 300 

    0xdb0f459c,// 304 PAY 301 

    0x16d0949f,// 305 PAY 302 

    0xede1a76d,// 306 PAY 303 

    0xb7ff6532,// 307 PAY 304 

    0x2624b5e8,// 308 PAY 305 

    0x76efd4fb,// 309 PAY 306 

    0x1f359db2,// 310 PAY 307 

    0x8f1b50e1,// 311 PAY 308 

    0x1dd446c5,// 312 PAY 309 

    0xef59c556,// 313 PAY 310 

    0x3eebbe29,// 314 PAY 311 

    0xf1b783fd,// 315 PAY 312 

    0x523575ae,// 316 PAY 313 

    0x0dd0ea5c,// 317 PAY 314 

    0xfdc99a37,// 318 PAY 315 

    0x00963b62,// 319 PAY 316 

    0x99713762,// 320 PAY 317 

    0xa0063407,// 321 PAY 318 

    0x89113364,// 322 PAY 319 

    0x086386b9,// 323 PAY 320 

    0xfbedf101,// 324 PAY 321 

    0xad423951,// 325 PAY 322 

    0xb858f229,// 326 PAY 323 

    0x4e1f4400,// 327 PAY 324 

    0x2da8a955,// 328 PAY 325 

    0x81f877a1,// 329 PAY 326 

    0x3cbccedc,// 330 PAY 327 

    0x51f9a3ff,// 331 PAY 328 

    0x783d7f74,// 332 PAY 329 

    0x6c4d2011,// 333 PAY 330 

    0x46696f0a,// 334 PAY 331 

    0xc42713dc,// 335 PAY 332 

    0x447f65d2,// 336 PAY 333 

    0x9eb80b7b,// 337 PAY 334 

    0xfa9f5fc7,// 338 PAY 335 

    0x195e9885,// 339 PAY 336 

    0xc22de9b3,// 340 PAY 337 

    0xb39c6805,// 341 PAY 338 

    0xf38fffbe,// 342 PAY 339 

    0xee53baa2,// 343 PAY 340 

    0x16c3e49e,// 344 PAY 341 

    0x4c31b1e4,// 345 PAY 342 

    0x1647e421,// 346 PAY 343 

    0x5ced6fe4,// 347 PAY 344 

    0xffc8ebe3,// 348 PAY 345 

    0x8200bbc0,// 349 PAY 346 

    0x577aa2bb,// 350 PAY 347 

    0x4a2808cd,// 351 PAY 348 

    0x06afcb35,// 352 PAY 349 

    0x14a0bb5b,// 353 PAY 350 

    0xb3ba0fc9,// 354 PAY 351 

    0xc5e90469,// 355 PAY 352 

    0x77a756e8,// 356 PAY 353 

    0xd48b61b4,// 357 PAY 354 

    0x0829650b,// 358 PAY 355 

    0x011084c1,// 359 PAY 356 

    0x8d73b55d,// 360 PAY 357 

    0xe86c1d0a,// 361 PAY 358 

    0x508f99d2,// 362 PAY 359 

    0xd81c644d,// 363 PAY 360 

    0xa964ab09,// 364 PAY 361 

    0xa6707e79,// 365 PAY 362 

    0x86d139ae,// 366 PAY 363 

    0x98259168,// 367 PAY 364 

    0x34ada48d,// 368 PAY 365 

    0x32d9b605,// 369 PAY 366 

    0x2a686709,// 370 PAY 367 

    0xa7d3608b,// 371 PAY 368 

    0x6ffffbb6,// 372 PAY 369 

    0xf06571bc,// 373 PAY 370 

    0x8d5e357d,// 374 PAY 371 

    0xf0759a57,// 375 PAY 372 

    0xf41d18a2,// 376 PAY 373 

    0x97f3aea4,// 377 PAY 374 

    0x7fabaf00,// 378 PAY 375 

/// HASH is  16 bytes 

    0xa6707e79,// 379 HSH   1 

    0x86d139ae,// 380 HSH   2 

    0x98259168,// 381 HSH   3 

    0x34ada48d,// 382 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 230 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x005c5be6 // 383 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt21_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 228 words. 

/// BDA size     is 905 (0x389) 

/// BDA id       is 0xb7c2 

    0x0389b7c2,// 3 BDA   1 

/// PAY Generic Data size   : 905 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0d4f1c68,// 4 PAY   1 

    0x6e60d984,// 5 PAY   2 

    0xdf4139b4,// 6 PAY   3 

    0x215f0c2a,// 7 PAY   4 

    0x3f128837,// 8 PAY   5 

    0xd7bf3830,// 9 PAY   6 

    0x918ac106,// 10 PAY   7 

    0x23e41595,// 11 PAY   8 

    0xd65601fb,// 12 PAY   9 

    0x21c2743c,// 13 PAY  10 

    0x91372caf,// 14 PAY  11 

    0x3d58574f,// 15 PAY  12 

    0xeb6f9d15,// 16 PAY  13 

    0xae5c513f,// 17 PAY  14 

    0x069edebe,// 18 PAY  15 

    0x20601538,// 19 PAY  16 

    0xea7a2f70,// 20 PAY  17 

    0xdd96bf5e,// 21 PAY  18 

    0xf67eb667,// 22 PAY  19 

    0x3c3a1daa,// 23 PAY  20 

    0xaf859d40,// 24 PAY  21 

    0x4f00dd1f,// 25 PAY  22 

    0xd1d0e712,// 26 PAY  23 

    0x4535aea6,// 27 PAY  24 

    0xd49111ce,// 28 PAY  25 

    0x00cfdb89,// 29 PAY  26 

    0x93a27741,// 30 PAY  27 

    0xd82d2464,// 31 PAY  28 

    0xba3b5c1c,// 32 PAY  29 

    0x1b35fcf0,// 33 PAY  30 

    0x3554f991,// 34 PAY  31 

    0x1c337275,// 35 PAY  32 

    0x609b12c4,// 36 PAY  33 

    0x5394dabe,// 37 PAY  34 

    0x5a77bb68,// 38 PAY  35 

    0xe1b125e8,// 39 PAY  36 

    0xca2ff296,// 40 PAY  37 

    0x9c984c61,// 41 PAY  38 

    0x7e896a54,// 42 PAY  39 

    0x53dd02d0,// 43 PAY  40 

    0x7eadb562,// 44 PAY  41 

    0x8fbd92e2,// 45 PAY  42 

    0x334ce669,// 46 PAY  43 

    0xb0f45eb0,// 47 PAY  44 

    0x23ec22ba,// 48 PAY  45 

    0xaac8a4b2,// 49 PAY  46 

    0x4bf1fa9b,// 50 PAY  47 

    0x07bbecb4,// 51 PAY  48 

    0xe71d5e67,// 52 PAY  49 

    0x7f9ae1a8,// 53 PAY  50 

    0xaaf1663d,// 54 PAY  51 

    0x7334742e,// 55 PAY  52 

    0x896a27c5,// 56 PAY  53 

    0x961ff471,// 57 PAY  54 

    0xeb172cb5,// 58 PAY  55 

    0x464e7ab1,// 59 PAY  56 

    0x46254c0b,// 60 PAY  57 

    0xe7167be1,// 61 PAY  58 

    0xca492531,// 62 PAY  59 

    0x09985603,// 63 PAY  60 

    0x7fc0507b,// 64 PAY  61 

    0x0c3de502,// 65 PAY  62 

    0x84fc822c,// 66 PAY  63 

    0xec87d0c0,// 67 PAY  64 

    0xa81cff77,// 68 PAY  65 

    0xbff8fa17,// 69 PAY  66 

    0xc5a9c3f0,// 70 PAY  67 

    0x8efc5f5d,// 71 PAY  68 

    0x630f2c00,// 72 PAY  69 

    0x1bf8afac,// 73 PAY  70 

    0x37e72f5e,// 74 PAY  71 

    0x5c92f060,// 75 PAY  72 

    0xc262ce29,// 76 PAY  73 

    0x9c856ade,// 77 PAY  74 

    0x751b00d7,// 78 PAY  75 

    0x81fa0c2d,// 79 PAY  76 

    0x2252387d,// 80 PAY  77 

    0x96953931,// 81 PAY  78 

    0x1f077641,// 82 PAY  79 

    0xcca1c566,// 83 PAY  80 

    0xb1f3b200,// 84 PAY  81 

    0x035f6c3f,// 85 PAY  82 

    0x40ae7d8e,// 86 PAY  83 

    0x5a288352,// 87 PAY  84 

    0xa72a687e,// 88 PAY  85 

    0x0448c21d,// 89 PAY  86 

    0xbb6f5487,// 90 PAY  87 

    0xb8f252df,// 91 PAY  88 

    0x74ccb5f0,// 92 PAY  89 

    0x1aece0ce,// 93 PAY  90 

    0x910edfea,// 94 PAY  91 

    0x7189e639,// 95 PAY  92 

    0xfd8e718a,// 96 PAY  93 

    0x68b29105,// 97 PAY  94 

    0x1f4be6b8,// 98 PAY  95 

    0x6e510058,// 99 PAY  96 

    0xee5a9d7f,// 100 PAY  97 

    0x5b3c13e7,// 101 PAY  98 

    0xac4ae5d3,// 102 PAY  99 

    0x0e998aa2,// 103 PAY 100 

    0x8bc26b4c,// 104 PAY 101 

    0x89273a69,// 105 PAY 102 

    0x7d779966,// 106 PAY 103 

    0xccb59f31,// 107 PAY 104 

    0x2f8fee43,// 108 PAY 105 

    0xe1416717,// 109 PAY 106 

    0x4c593f9f,// 110 PAY 107 

    0x6388fd72,// 111 PAY 108 

    0x51882379,// 112 PAY 109 

    0xb31a8e39,// 113 PAY 110 

    0x210a39dd,// 114 PAY 111 

    0x567df4b3,// 115 PAY 112 

    0xa723a060,// 116 PAY 113 

    0x682bae00,// 117 PAY 114 

    0x104eac15,// 118 PAY 115 

    0xb7fe6b27,// 119 PAY 116 

    0x017cfb34,// 120 PAY 117 

    0x4e1b6328,// 121 PAY 118 

    0x7f0ccc04,// 122 PAY 119 

    0x88daa6e3,// 123 PAY 120 

    0x6d59f5e7,// 124 PAY 121 

    0xf9e8d79d,// 125 PAY 122 

    0x0431fff6,// 126 PAY 123 

    0xeb883f40,// 127 PAY 124 

    0x5d090735,// 128 PAY 125 

    0xfa8d49d8,// 129 PAY 126 

    0x74672930,// 130 PAY 127 

    0xd8d99a54,// 131 PAY 128 

    0xba66c8fc,// 132 PAY 129 

    0x5a4767a0,// 133 PAY 130 

    0x6b2451bf,// 134 PAY 131 

    0x583abf34,// 135 PAY 132 

    0x5b27f07e,// 136 PAY 133 

    0x4a348499,// 137 PAY 134 

    0x8a76c64b,// 138 PAY 135 

    0xc848e899,// 139 PAY 136 

    0x1442dba6,// 140 PAY 137 

    0xbf383f4e,// 141 PAY 138 

    0x1805d380,// 142 PAY 139 

    0x35d3f012,// 143 PAY 140 

    0x542c818b,// 144 PAY 141 

    0xa1be5ed3,// 145 PAY 142 

    0x8dd7769b,// 146 PAY 143 

    0x762f676e,// 147 PAY 144 

    0x6c3a8867,// 148 PAY 145 

    0xc7cab5b6,// 149 PAY 146 

    0xfd387165,// 150 PAY 147 

    0x78814fb2,// 151 PAY 148 

    0x0a3ad9a0,// 152 PAY 149 

    0x5389794b,// 153 PAY 150 

    0xcae715cf,// 154 PAY 151 

    0x4206ffa9,// 155 PAY 152 

    0x650f4d0b,// 156 PAY 153 

    0xe14807b6,// 157 PAY 154 

    0x99add00c,// 158 PAY 155 

    0x1370f00d,// 159 PAY 156 

    0x657406f6,// 160 PAY 157 

    0x6d4d56d3,// 161 PAY 158 

    0x811d15b7,// 162 PAY 159 

    0x0bf9e5c9,// 163 PAY 160 

    0x4f762f39,// 164 PAY 161 

    0x47253758,// 165 PAY 162 

    0x10b2379d,// 166 PAY 163 

    0x2dc83fff,// 167 PAY 164 

    0x7239345e,// 168 PAY 165 

    0xd0a9f877,// 169 PAY 166 

    0x432a992a,// 170 PAY 167 

    0x9cd6a7dd,// 171 PAY 168 

    0xd5395ff3,// 172 PAY 169 

    0x49141133,// 173 PAY 170 

    0x0bf069a8,// 174 PAY 171 

    0x82fe5860,// 175 PAY 172 

    0x50c3415b,// 176 PAY 173 

    0x31081b9d,// 177 PAY 174 

    0xc60229d0,// 178 PAY 175 

    0xf74657f6,// 179 PAY 176 

    0x897922ef,// 180 PAY 177 

    0x38af1e60,// 181 PAY 178 

    0x6a2593da,// 182 PAY 179 

    0xc17db52a,// 183 PAY 180 

    0x6d1b3364,// 184 PAY 181 

    0x246ac585,// 185 PAY 182 

    0x09b21465,// 186 PAY 183 

    0x4a4e177f,// 187 PAY 184 

    0x8dae3465,// 188 PAY 185 

    0x958ff289,// 189 PAY 186 

    0x547358fe,// 190 PAY 187 

    0x09d8aad5,// 191 PAY 188 

    0x28a25697,// 192 PAY 189 

    0x18ff95e0,// 193 PAY 190 

    0xf9c12193,// 194 PAY 191 

    0xa6513a8c,// 195 PAY 192 

    0x8c215bfb,// 196 PAY 193 

    0xdee9106d,// 197 PAY 194 

    0x5122db8f,// 198 PAY 195 

    0x41a38880,// 199 PAY 196 

    0x96c2c08c,// 200 PAY 197 

    0x9a130361,// 201 PAY 198 

    0x70da903f,// 202 PAY 199 

    0x9f78cf48,// 203 PAY 200 

    0x6865e75a,// 204 PAY 201 

    0xdb27ddc7,// 205 PAY 202 

    0x9cf18611,// 206 PAY 203 

    0xcdfb8bad,// 207 PAY 204 

    0x661dd859,// 208 PAY 205 

    0x36fc3421,// 209 PAY 206 

    0xcaba4698,// 210 PAY 207 

    0x4d4d32f4,// 211 PAY 208 

    0xe58cee2c,// 212 PAY 209 

    0xa1245d21,// 213 PAY 210 

    0x12a6db22,// 214 PAY 211 

    0xd3515355,// 215 PAY 212 

    0xc565a8a6,// 216 PAY 213 

    0x83790d20,// 217 PAY 214 

    0xaa784fbe,// 218 PAY 215 

    0x4e042d6e,// 219 PAY 216 

    0x4c296bfb,// 220 PAY 217 

    0x57d6d66e,// 221 PAY 218 

    0x1e4f9265,// 222 PAY 219 

    0x987f8158,// 223 PAY 220 

    0x1ed46684,// 224 PAY 221 

    0x22aff49e,// 225 PAY 222 

    0xf77b90d7,// 226 PAY 223 

    0x031968d9,// 227 PAY 224 

    0x311edd04,// 228 PAY 225 

    0x2e7a4a40,// 229 PAY 226 

    0x46000000,// 230 PAY 227 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 59 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x14 

    0x00ec1485 // 231 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt22_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 136 words. 

/// BDA size     is 539 (0x21b) 

/// BDA id       is 0x393e 

    0x021b393e,// 3 BDA   1 

/// PAY Generic Data size   : 539 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xcecef410,// 4 PAY   1 

    0x32da1635,// 5 PAY   2 

    0xecf24b88,// 6 PAY   3 

    0x92969d23,// 7 PAY   4 

    0x944f4280,// 8 PAY   5 

    0x5ddd02cb,// 9 PAY   6 

    0xce323b4d,// 10 PAY   7 

    0xe8c5bfe1,// 11 PAY   8 

    0x5359677f,// 12 PAY   9 

    0x53197127,// 13 PAY  10 

    0x2ff888b2,// 14 PAY  11 

    0xd1549f01,// 15 PAY  12 

    0x30bc7bcc,// 16 PAY  13 

    0x7d06ee14,// 17 PAY  14 

    0x34b512cd,// 18 PAY  15 

    0x0abd4975,// 19 PAY  16 

    0xfcc2b9ab,// 20 PAY  17 

    0x15821e90,// 21 PAY  18 

    0x225b1f40,// 22 PAY  19 

    0x0f70c689,// 23 PAY  20 

    0x3a073f31,// 24 PAY  21 

    0x4454ca40,// 25 PAY  22 

    0x45fcd566,// 26 PAY  23 

    0x74c9a789,// 27 PAY  24 

    0xb3a37f4f,// 28 PAY  25 

    0x3d1c9e10,// 29 PAY  26 

    0xbb0d70ac,// 30 PAY  27 

    0x7c148efe,// 31 PAY  28 

    0xf1342ab1,// 32 PAY  29 

    0x9480954e,// 33 PAY  30 

    0xed78ed8a,// 34 PAY  31 

    0x0244a30a,// 35 PAY  32 

    0xcb690389,// 36 PAY  33 

    0xb771f6f0,// 37 PAY  34 

    0xcec2d3ef,// 38 PAY  35 

    0x512ba69f,// 39 PAY  36 

    0x547d0ffd,// 40 PAY  37 

    0x8b4e6d78,// 41 PAY  38 

    0x232610d3,// 42 PAY  39 

    0x4c02601f,// 43 PAY  40 

    0x684ca8fd,// 44 PAY  41 

    0xeba77d82,// 45 PAY  42 

    0xfc7a20d0,// 46 PAY  43 

    0x8fb42925,// 47 PAY  44 

    0x892c6e9e,// 48 PAY  45 

    0x13a4549a,// 49 PAY  46 

    0xb82063c6,// 50 PAY  47 

    0xfec94370,// 51 PAY  48 

    0x7279bf7a,// 52 PAY  49 

    0x3c553069,// 53 PAY  50 

    0x6545dc21,// 54 PAY  51 

    0x5c8161ed,// 55 PAY  52 

    0x64944e06,// 56 PAY  53 

    0x2b470b6d,// 57 PAY  54 

    0x1f5f6577,// 58 PAY  55 

    0x15d3a237,// 59 PAY  56 

    0x3b06b183,// 60 PAY  57 

    0xef533c4e,// 61 PAY  58 

    0x5545be2f,// 62 PAY  59 

    0x2fd7cd0f,// 63 PAY  60 

    0x28ba0ede,// 64 PAY  61 

    0x6b6a10fb,// 65 PAY  62 

    0xddb8c939,// 66 PAY  63 

    0x1b8697d6,// 67 PAY  64 

    0xd358e254,// 68 PAY  65 

    0x49d81dff,// 69 PAY  66 

    0x35dbcef2,// 70 PAY  67 

    0x1b91cb50,// 71 PAY  68 

    0x2799eb09,// 72 PAY  69 

    0xa772d28b,// 73 PAY  70 

    0xdac01b35,// 74 PAY  71 

    0xa9466b5f,// 75 PAY  72 

    0xb8134023,// 76 PAY  73 

    0x3fd369f7,// 77 PAY  74 

    0x188654d8,// 78 PAY  75 

    0xa8bd8fab,// 79 PAY  76 

    0x434e0bb3,// 80 PAY  77 

    0x55bea09d,// 81 PAY  78 

    0xd0bb3045,// 82 PAY  79 

    0x8666489f,// 83 PAY  80 

    0x19c424f6,// 84 PAY  81 

    0x56f0cf3b,// 85 PAY  82 

    0xc4436067,// 86 PAY  83 

    0xa8239ba9,// 87 PAY  84 

    0xa0184fb7,// 88 PAY  85 

    0xc7cafa28,// 89 PAY  86 

    0x06f65146,// 90 PAY  87 

    0x3dbd266a,// 91 PAY  88 

    0x290adad4,// 92 PAY  89 

    0xe60a7086,// 93 PAY  90 

    0x04a388d8,// 94 PAY  91 

    0x4a7f3e5a,// 95 PAY  92 

    0xc9a9235e,// 96 PAY  93 

    0x3d9f6447,// 97 PAY  94 

    0x663fc2f8,// 98 PAY  95 

    0x7018b28e,// 99 PAY  96 

    0x39b24085,// 100 PAY  97 

    0xfe54ab9e,// 101 PAY  98 

    0xc9c1e42f,// 102 PAY  99 

    0xcfa7f5f7,// 103 PAY 100 

    0x8ff4f68a,// 104 PAY 101 

    0x204ff525,// 105 PAY 102 

    0x8b62f307,// 106 PAY 103 

    0x85fcbc59,// 107 PAY 104 

    0xfd919473,// 108 PAY 105 

    0x2e918d23,// 109 PAY 106 

    0x12d4071d,// 110 PAY 107 

    0x254da1f5,// 111 PAY 108 

    0x165fda0f,// 112 PAY 109 

    0xff3d148c,// 113 PAY 110 

    0xa7b7af62,// 114 PAY 111 

    0x43ea359f,// 115 PAY 112 

    0x05e6a710,// 116 PAY 113 

    0x704423a6,// 117 PAY 114 

    0x54b74fe1,// 118 PAY 115 

    0xfa5710f9,// 119 PAY 116 

    0x72285060,// 120 PAY 117 

    0xb8c1f9cd,// 121 PAY 118 

    0x9e16fc11,// 122 PAY 119 

    0x4b164f94,// 123 PAY 120 

    0x247985c7,// 124 PAY 121 

    0x6346be21,// 125 PAY 122 

    0xd25050db,// 126 PAY 123 

    0x7306cf61,// 127 PAY 124 

    0x47098499,// 128 PAY 125 

    0x8627b9e0,// 129 PAY 126 

    0x834bc765,// 130 PAY 127 

    0x09be5a9b,// 131 PAY 128 

    0x0d1c369e,// 132 PAY 129 

    0xaaa4daf6,// 133 PAY 130 

    0x790eac44,// 134 PAY 131 

    0x5c77b6ba,// 135 PAY 132 

    0x6810d392,// 136 PAY 133 

    0x6b07302b,// 137 PAY 134 

    0x3d812900,// 138 PAY 135 

/// HASH is  4 bytes 

    0xcb690389,// 139 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 76 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x0160804c // 140 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt23_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 266 words. 

/// BDA size     is 1057 (0x421) 

/// BDA id       is 0x6a15 

    0x04216a15,// 3 BDA   1 

/// PAY Generic Data size   : 1057 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x84ed67f9,// 4 PAY   1 

    0x3e4701f1,// 5 PAY   2 

    0x22b27b78,// 6 PAY   3 

    0xc0986950,// 7 PAY   4 

    0xf19a1fdd,// 8 PAY   5 

    0x3c9ecc6b,// 9 PAY   6 

    0x88c0861c,// 10 PAY   7 

    0xcab41d00,// 11 PAY   8 

    0xb8ce4d53,// 12 PAY   9 

    0x7828474d,// 13 PAY  10 

    0x3dba2055,// 14 PAY  11 

    0xede8d875,// 15 PAY  12 

    0x7278c9b2,// 16 PAY  13 

    0x37658e87,// 17 PAY  14 

    0x0fff4c38,// 18 PAY  15 

    0xec139d72,// 19 PAY  16 

    0xaf98788b,// 20 PAY  17 

    0xfe87e10d,// 21 PAY  18 

    0xf9a8d1cb,// 22 PAY  19 

    0xc666a220,// 23 PAY  20 

    0xa25677a2,// 24 PAY  21 

    0x40ff3646,// 25 PAY  22 

    0xd7cf0d91,// 26 PAY  23 

    0x66707cae,// 27 PAY  24 

    0xa41e8c02,// 28 PAY  25 

    0x53c1bde8,// 29 PAY  26 

    0xeba96ea5,// 30 PAY  27 

    0x42823171,// 31 PAY  28 

    0x9bcd030d,// 32 PAY  29 

    0x0fe795b0,// 33 PAY  30 

    0xc7a60757,// 34 PAY  31 

    0x8966beb8,// 35 PAY  32 

    0xae47bcb5,// 36 PAY  33 

    0xefcaf348,// 37 PAY  34 

    0xc5186eab,// 38 PAY  35 

    0x1f9c912e,// 39 PAY  36 

    0x80662465,// 40 PAY  37 

    0x97cfef24,// 41 PAY  38 

    0x46054baf,// 42 PAY  39 

    0x4282ffce,// 43 PAY  40 

    0xbe5f89a2,// 44 PAY  41 

    0x662a70f0,// 45 PAY  42 

    0x66d9bcd4,// 46 PAY  43 

    0x265695f7,// 47 PAY  44 

    0x6f9d6b51,// 48 PAY  45 

    0xa52f8436,// 49 PAY  46 

    0xf1f7c5c1,// 50 PAY  47 

    0x183a88f3,// 51 PAY  48 

    0x34c06997,// 52 PAY  49 

    0xb0b84473,// 53 PAY  50 

    0x7bba54da,// 54 PAY  51 

    0x407209cd,// 55 PAY  52 

    0xf7522a1d,// 56 PAY  53 

    0xa16dcefe,// 57 PAY  54 

    0x4e103d0e,// 58 PAY  55 

    0xf6ee836d,// 59 PAY  56 

    0x983df713,// 60 PAY  57 

    0x94d6bb97,// 61 PAY  58 

    0x319b3401,// 62 PAY  59 

    0xec652c83,// 63 PAY  60 

    0xf2a3ae86,// 64 PAY  61 

    0xafc0494f,// 65 PAY  62 

    0x58cb4e46,// 66 PAY  63 

    0xae3d2bcb,// 67 PAY  64 

    0x5b61ddfb,// 68 PAY  65 

    0x7e914a87,// 69 PAY  66 

    0x50fc1b09,// 70 PAY  67 

    0x27ab413d,// 71 PAY  68 

    0x05a5692d,// 72 PAY  69 

    0x7c132842,// 73 PAY  70 

    0x1a6103f6,// 74 PAY  71 

    0x081755a0,// 75 PAY  72 

    0x94fac159,// 76 PAY  73 

    0xa35d1f46,// 77 PAY  74 

    0x8196ec7a,// 78 PAY  75 

    0x55d59b42,// 79 PAY  76 

    0x36b34cb9,// 80 PAY  77 

    0x4f0222ae,// 81 PAY  78 

    0xbf68dddb,// 82 PAY  79 

    0xec70957e,// 83 PAY  80 

    0xeb0bcd98,// 84 PAY  81 

    0x0a456b90,// 85 PAY  82 

    0xd376cb42,// 86 PAY  83 

    0x5232ab52,// 87 PAY  84 

    0x3b0438ef,// 88 PAY  85 

    0x2afba6c8,// 89 PAY  86 

    0x97185f8d,// 90 PAY  87 

    0x49c64091,// 91 PAY  88 

    0xaeaf969a,// 92 PAY  89 

    0x6c93094a,// 93 PAY  90 

    0xe59c01fa,// 94 PAY  91 

    0x44b06e06,// 95 PAY  92 

    0x5ef240b4,// 96 PAY  93 

    0xdd190339,// 97 PAY  94 

    0x8b7495ee,// 98 PAY  95 

    0xe99fd93f,// 99 PAY  96 

    0x915444b2,// 100 PAY  97 

    0x81ac3bf5,// 101 PAY  98 

    0x52a68e82,// 102 PAY  99 

    0xe2b91799,// 103 PAY 100 

    0xef5984d9,// 104 PAY 101 

    0xb2b7b33d,// 105 PAY 102 

    0xca790110,// 106 PAY 103 

    0x6e4d6b1c,// 107 PAY 104 

    0x91d8b6d2,// 108 PAY 105 

    0xcb0d05f6,// 109 PAY 106 

    0xa94c3630,// 110 PAY 107 

    0x10113fa2,// 111 PAY 108 

    0x5cd4d1d9,// 112 PAY 109 

    0x995a3832,// 113 PAY 110 

    0x65cfb745,// 114 PAY 111 

    0xcb6abc47,// 115 PAY 112 

    0x81e9c4da,// 116 PAY 113 

    0x061f769c,// 117 PAY 114 

    0x71420422,// 118 PAY 115 

    0xd7627ba5,// 119 PAY 116 

    0xfa5cb902,// 120 PAY 117 

    0x4a7f0b78,// 121 PAY 118 

    0xf4c81fe4,// 122 PAY 119 

    0xaf633b81,// 123 PAY 120 

    0xdbf0f604,// 124 PAY 121 

    0x3690c0b9,// 125 PAY 122 

    0x698d91d3,// 126 PAY 123 

    0xfe479dbc,// 127 PAY 124 

    0xcc99345b,// 128 PAY 125 

    0xc19aefe2,// 129 PAY 126 

    0x55565aac,// 130 PAY 127 

    0x0377a4df,// 131 PAY 128 

    0x94f199a5,// 132 PAY 129 

    0x0150b93b,// 133 PAY 130 

    0x7c4af8d4,// 134 PAY 131 

    0x30c7bed7,// 135 PAY 132 

    0xb2ab5130,// 136 PAY 133 

    0x89caa542,// 137 PAY 134 

    0xa31fda2e,// 138 PAY 135 

    0x6a878502,// 139 PAY 136 

    0x4d406644,// 140 PAY 137 

    0xc0e371a3,// 141 PAY 138 

    0x48b09b32,// 142 PAY 139 

    0x9b00979b,// 143 PAY 140 

    0x37d6b4ec,// 144 PAY 141 

    0x20e9d46a,// 145 PAY 142 

    0x738d1dac,// 146 PAY 143 

    0x504161e7,// 147 PAY 144 

    0xed335f53,// 148 PAY 145 

    0x265b64ae,// 149 PAY 146 

    0x5326d155,// 150 PAY 147 

    0x611060e7,// 151 PAY 148 

    0x4c100fac,// 152 PAY 149 

    0xacdfa7b6,// 153 PAY 150 

    0x1a02140d,// 154 PAY 151 

    0xf53f55ba,// 155 PAY 152 

    0x32ed1b91,// 156 PAY 153 

    0x1dd9d2c7,// 157 PAY 154 

    0xbb7b1279,// 158 PAY 155 

    0x3d360493,// 159 PAY 156 

    0x7993ee94,// 160 PAY 157 

    0x0935d487,// 161 PAY 158 

    0xb034f804,// 162 PAY 159 

    0xc2a2a5fb,// 163 PAY 160 

    0xddc1a177,// 164 PAY 161 

    0xe7eaaeee,// 165 PAY 162 

    0xc320b804,// 166 PAY 163 

    0xc3e27fef,// 167 PAY 164 

    0x10717a2d,// 168 PAY 165 

    0x41e4fc0d,// 169 PAY 166 

    0x21d30afe,// 170 PAY 167 

    0xd52f98d8,// 171 PAY 168 

    0xe13b9fcb,// 172 PAY 169 

    0xb0827ae4,// 173 PAY 170 

    0x168ace24,// 174 PAY 171 

    0x9e5ff0d8,// 175 PAY 172 

    0xf5807435,// 176 PAY 173 

    0x18e9771f,// 177 PAY 174 

    0xff12b1a3,// 178 PAY 175 

    0x6e708cf3,// 179 PAY 176 

    0x9d8a9a84,// 180 PAY 177 

    0x78ede197,// 181 PAY 178 

    0x9b72fd0a,// 182 PAY 179 

    0x721dd733,// 183 PAY 180 

    0x021c823c,// 184 PAY 181 

    0xfbb329cf,// 185 PAY 182 

    0xf339e4b6,// 186 PAY 183 

    0x817c258e,// 187 PAY 184 

    0xc3b40c34,// 188 PAY 185 

    0x18de15a4,// 189 PAY 186 

    0xeb2d85ed,// 190 PAY 187 

    0x4d87da28,// 191 PAY 188 

    0x24a7edb1,// 192 PAY 189 

    0x6750d4c6,// 193 PAY 190 

    0xbf977be8,// 194 PAY 191 

    0xfc17e9af,// 195 PAY 192 

    0x424a7422,// 196 PAY 193 

    0x7c7dc794,// 197 PAY 194 

    0x277dfa3b,// 198 PAY 195 

    0xb3f5c66b,// 199 PAY 196 

    0x78b90cb2,// 200 PAY 197 

    0x15cb5cb5,// 201 PAY 198 

    0x9d70d730,// 202 PAY 199 

    0x31a928d4,// 203 PAY 200 

    0x0085ea5c,// 204 PAY 201 

    0x4fb50de9,// 205 PAY 202 

    0x34483cfa,// 206 PAY 203 

    0x71420851,// 207 PAY 204 

    0x234e6bb0,// 208 PAY 205 

    0x962144fb,// 209 PAY 206 

    0x28edc014,// 210 PAY 207 

    0x056abad7,// 211 PAY 208 

    0xe5007731,// 212 PAY 209 

    0x078a9963,// 213 PAY 210 

    0xe54ab136,// 214 PAY 211 

    0x65dbf0aa,// 215 PAY 212 

    0x34a9ad96,// 216 PAY 213 

    0xda86ab80,// 217 PAY 214 

    0x55b34b72,// 218 PAY 215 

    0xc9c29309,// 219 PAY 216 

    0x2018721a,// 220 PAY 217 

    0x18c2ee55,// 221 PAY 218 

    0xd621c9fc,// 222 PAY 219 

    0x6bf46f6f,// 223 PAY 220 

    0xa126bfef,// 224 PAY 221 

    0xf0a7fe98,// 225 PAY 222 

    0x11a7b63f,// 226 PAY 223 

    0xcb22389f,// 227 PAY 224 

    0xbdaaf5d6,// 228 PAY 225 

    0xe886c30b,// 229 PAY 226 

    0x05a34e5a,// 230 PAY 227 

    0xb0e99f54,// 231 PAY 228 

    0xd24d38fa,// 232 PAY 229 

    0xe6c167cb,// 233 PAY 230 

    0x9d76c78e,// 234 PAY 231 

    0x516988f7,// 235 PAY 232 

    0x4f7ecbcd,// 236 PAY 233 

    0x211be02c,// 237 PAY 234 

    0x5729d757,// 238 PAY 235 

    0x2720d785,// 239 PAY 236 

    0x241e3617,// 240 PAY 237 

    0xf1207f93,// 241 PAY 238 

    0x19910fac,// 242 PAY 239 

    0xf5eb08ec,// 243 PAY 240 

    0x273d39f0,// 244 PAY 241 

    0x574aaed0,// 245 PAY 242 

    0x3c4a3862,// 246 PAY 243 

    0x18fd7da0,// 247 PAY 244 

    0x4401db25,// 248 PAY 245 

    0xb1f1efb6,// 249 PAY 246 

    0x89a9db5e,// 250 PAY 247 

    0x3fb7c15d,// 251 PAY 248 

    0x08c78dd2,// 252 PAY 249 

    0x0ce41ba2,// 253 PAY 250 

    0x1329613b,// 254 PAY 251 

    0x247a3f45,// 255 PAY 252 

    0xb009d7f2,// 256 PAY 253 

    0x056654e4,// 257 PAY 254 

    0xb6ff038e,// 258 PAY 255 

    0xf826f073,// 259 PAY 256 

    0x2fd593be,// 260 PAY 257 

    0x9ad91161,// 261 PAY 258 

    0xb2a8687f,// 262 PAY 259 

    0x032fccd0,// 263 PAY 260 

    0x47089ed6,// 264 PAY 261 

    0x0315f9dd,// 265 PAY 262 

    0xaa4ee854,// 266 PAY 263 

    0x8d3a1d33,// 267 PAY 264 

    0xac000000,// 268 PAY 265 

/// STA is 1 words. 

/// STA num_pkts       : 23 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd8 

    0x03d9d817 // 269 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt24_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 511 words. 

/// BDA size     is 2038 (0x7f6) 

/// BDA id       is 0x2a9b 

    0x07f62a9b,// 3 BDA   1 

/// PAY Generic Data size   : 2038 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe9ce01b5,// 4 PAY   1 

    0xb84189e5,// 5 PAY   2 

    0x85391d98,// 6 PAY   3 

    0x6cece17d,// 7 PAY   4 

    0x761939b0,// 8 PAY   5 

    0x40ad74dc,// 9 PAY   6 

    0x2933a88e,// 10 PAY   7 

    0x774deb6d,// 11 PAY   8 

    0x469e8370,// 12 PAY   9 

    0x34ef951d,// 13 PAY  10 

    0xb558b748,// 14 PAY  11 

    0x3f5f046a,// 15 PAY  12 

    0xbba2bb1a,// 16 PAY  13 

    0x4c2001d1,// 17 PAY  14 

    0xdb328285,// 18 PAY  15 

    0xc5d73b1f,// 19 PAY  16 

    0x4cf240fe,// 20 PAY  17 

    0x26b90fa9,// 21 PAY  18 

    0x52113337,// 22 PAY  19 

    0x8c03c598,// 23 PAY  20 

    0x531101c1,// 24 PAY  21 

    0xf2031bc5,// 25 PAY  22 

    0x869a3902,// 26 PAY  23 

    0xe8e1de58,// 27 PAY  24 

    0xe18f674b,// 28 PAY  25 

    0xc1ad7739,// 29 PAY  26 

    0x279812a1,// 30 PAY  27 

    0xba281bd6,// 31 PAY  28 

    0xbfa371c2,// 32 PAY  29 

    0x0d5c1b53,// 33 PAY  30 

    0xa5f5c98f,// 34 PAY  31 

    0x5e47e5a3,// 35 PAY  32 

    0x5761f8be,// 36 PAY  33 

    0x6a5e50bc,// 37 PAY  34 

    0xe4ef8b50,// 38 PAY  35 

    0x9a01d152,// 39 PAY  36 

    0x88a615f1,// 40 PAY  37 

    0xe268227f,// 41 PAY  38 

    0x64394bf4,// 42 PAY  39 

    0x99d194d8,// 43 PAY  40 

    0x058533bc,// 44 PAY  41 

    0x1c4228b3,// 45 PAY  42 

    0xcf098f46,// 46 PAY  43 

    0x5ea49c24,// 47 PAY  44 

    0x33b2283c,// 48 PAY  45 

    0xffbae27a,// 49 PAY  46 

    0xd7884843,// 50 PAY  47 

    0x469a0c97,// 51 PAY  48 

    0xe396a31e,// 52 PAY  49 

    0x494df5a0,// 53 PAY  50 

    0xf7659839,// 54 PAY  51 

    0x1833c568,// 55 PAY  52 

    0x82f93d36,// 56 PAY  53 

    0x1e500d7d,// 57 PAY  54 

    0xa1774794,// 58 PAY  55 

    0x8f1d65df,// 59 PAY  56 

    0x07c590f6,// 60 PAY  57 

    0x9fbc1004,// 61 PAY  58 

    0xc121c18e,// 62 PAY  59 

    0xf45e503c,// 63 PAY  60 

    0x0cd326cf,// 64 PAY  61 

    0x12a3f678,// 65 PAY  62 

    0x906e7296,// 66 PAY  63 

    0x568588b1,// 67 PAY  64 

    0x506016bc,// 68 PAY  65 

    0xab794ef5,// 69 PAY  66 

    0xcc97b91a,// 70 PAY  67 

    0xea3a10b4,// 71 PAY  68 

    0xf4e2dae3,// 72 PAY  69 

    0x16d39aab,// 73 PAY  70 

    0x33497a03,// 74 PAY  71 

    0xf84cdfb4,// 75 PAY  72 

    0xf759353f,// 76 PAY  73 

    0x968d5c3e,// 77 PAY  74 

    0xcf3fae3a,// 78 PAY  75 

    0x35edc071,// 79 PAY  76 

    0x0765cf38,// 80 PAY  77 

    0xc31e3ab5,// 81 PAY  78 

    0xdbc23bf4,// 82 PAY  79 

    0x10423526,// 83 PAY  80 

    0x53b461cd,// 84 PAY  81 

    0xdfe41662,// 85 PAY  82 

    0xa8e2b450,// 86 PAY  83 

    0xef102264,// 87 PAY  84 

    0x1b2247b8,// 88 PAY  85 

    0x94706fe5,// 89 PAY  86 

    0x264f406c,// 90 PAY  87 

    0xb73f9318,// 91 PAY  88 

    0xf16acd04,// 92 PAY  89 

    0x8bff9605,// 93 PAY  90 

    0x93cdf187,// 94 PAY  91 

    0x6408039b,// 95 PAY  92 

    0xfba46069,// 96 PAY  93 

    0x6ffb8c23,// 97 PAY  94 

    0xe2cf1ff5,// 98 PAY  95 

    0xd0e8f5fa,// 99 PAY  96 

    0xef3f52a5,// 100 PAY  97 

    0x010de071,// 101 PAY  98 

    0xb426ac9d,// 102 PAY  99 

    0x773f44ab,// 103 PAY 100 

    0x53eb5b6f,// 104 PAY 101 

    0x10039187,// 105 PAY 102 

    0x99afe9d7,// 106 PAY 103 

    0x83cf8cd1,// 107 PAY 104 

    0x2ff4c20b,// 108 PAY 105 

    0x1b46ce4b,// 109 PAY 106 

    0x55b193ba,// 110 PAY 107 

    0xc5b62717,// 111 PAY 108 

    0xf74749e5,// 112 PAY 109 

    0x63d25e51,// 113 PAY 110 

    0x5cb7916a,// 114 PAY 111 

    0x014b36a6,// 115 PAY 112 

    0xde041fd8,// 116 PAY 113 

    0x6445d734,// 117 PAY 114 

    0x905de24b,// 118 PAY 115 

    0xe87f9af9,// 119 PAY 116 

    0x54bdab44,// 120 PAY 117 

    0xe5ee39e7,// 121 PAY 118 

    0xcf709848,// 122 PAY 119 

    0x4b01555b,// 123 PAY 120 

    0xfac353e3,// 124 PAY 121 

    0xf2d089fa,// 125 PAY 122 

    0x40aeb3b4,// 126 PAY 123 

    0x6c8016e8,// 127 PAY 124 

    0xc04b74ec,// 128 PAY 125 

    0x6712bcac,// 129 PAY 126 

    0x989278e3,// 130 PAY 127 

    0x93da1293,// 131 PAY 128 

    0x94b9573c,// 132 PAY 129 

    0x138ebc7d,// 133 PAY 130 

    0xf61b4bce,// 134 PAY 131 

    0x853c23cf,// 135 PAY 132 

    0x6affcc29,// 136 PAY 133 

    0xb8cbdab4,// 137 PAY 134 

    0x08ab97c3,// 138 PAY 135 

    0x15ee197b,// 139 PAY 136 

    0x554b3358,// 140 PAY 137 

    0x091164ef,// 141 PAY 138 

    0x0953d51e,// 142 PAY 139 

    0x154732e6,// 143 PAY 140 

    0xbab957a5,// 144 PAY 141 

    0x3b02de3b,// 145 PAY 142 

    0xb34a1b6a,// 146 PAY 143 

    0x9fd800b5,// 147 PAY 144 

    0xca315db8,// 148 PAY 145 

    0x206085ab,// 149 PAY 146 

    0x2c0d6593,// 150 PAY 147 

    0x79c92fcc,// 151 PAY 148 

    0xe736dcf7,// 152 PAY 149 

    0x13033f50,// 153 PAY 150 

    0x32e5bad5,// 154 PAY 151 

    0xcb708fb6,// 155 PAY 152 

    0xcc3f97cd,// 156 PAY 153 

    0x87769167,// 157 PAY 154 

    0x6797b0a7,// 158 PAY 155 

    0xb15b5efd,// 159 PAY 156 

    0x490eabe7,// 160 PAY 157 

    0xbdf0e28f,// 161 PAY 158 

    0x93482352,// 162 PAY 159 

    0x87aed7bd,// 163 PAY 160 

    0x47ab4638,// 164 PAY 161 

    0xceb01282,// 165 PAY 162 

    0x78b4e3f9,// 166 PAY 163 

    0x048d4678,// 167 PAY 164 

    0x9858ae62,// 168 PAY 165 

    0xf628e0c4,// 169 PAY 166 

    0xda653e77,// 170 PAY 167 

    0x7ad419c0,// 171 PAY 168 

    0xf9450080,// 172 PAY 169 

    0xa70f76de,// 173 PAY 170 

    0x0814d024,// 174 PAY 171 

    0x77ce2440,// 175 PAY 172 

    0x4dd80348,// 176 PAY 173 

    0xc1160801,// 177 PAY 174 

    0x1d04d9c2,// 178 PAY 175 

    0x708cd149,// 179 PAY 176 

    0xcd205567,// 180 PAY 177 

    0x9bcf9443,// 181 PAY 178 

    0xb042395b,// 182 PAY 179 

    0xe7b72cff,// 183 PAY 180 

    0x4347b12f,// 184 PAY 181 

    0x44131d62,// 185 PAY 182 

    0x391b023c,// 186 PAY 183 

    0x884b452d,// 187 PAY 184 

    0x0a640e09,// 188 PAY 185 

    0x985f0fbd,// 189 PAY 186 

    0x6c0343dc,// 190 PAY 187 

    0x6a9d170e,// 191 PAY 188 

    0xa0bb88d3,// 192 PAY 189 

    0x566a0bfb,// 193 PAY 190 

    0x1eb398eb,// 194 PAY 191 

    0x758be03f,// 195 PAY 192 

    0xaa3ecec4,// 196 PAY 193 

    0x3488bc9e,// 197 PAY 194 

    0x8de0e85b,// 198 PAY 195 

    0xbfb71c72,// 199 PAY 196 

    0xb4121330,// 200 PAY 197 

    0x40c47c66,// 201 PAY 198 

    0xf6427592,// 202 PAY 199 

    0x8b35b8fa,// 203 PAY 200 

    0x3d426d15,// 204 PAY 201 

    0xc4e46513,// 205 PAY 202 

    0x6da3342e,// 206 PAY 203 

    0x0380c97c,// 207 PAY 204 

    0x56ec9841,// 208 PAY 205 

    0x24357a77,// 209 PAY 206 

    0xc47722ca,// 210 PAY 207 

    0x98f8731d,// 211 PAY 208 

    0x080eb503,// 212 PAY 209 

    0xfd55f456,// 213 PAY 210 

    0x3b6c5a76,// 214 PAY 211 

    0x59c1a2fc,// 215 PAY 212 

    0xa8f90ffd,// 216 PAY 213 

    0xe924390c,// 217 PAY 214 

    0x755edd8d,// 218 PAY 215 

    0x0946f8fd,// 219 PAY 216 

    0xf42586e1,// 220 PAY 217 

    0x276e9621,// 221 PAY 218 

    0xc6e37a62,// 222 PAY 219 

    0xdc66a414,// 223 PAY 220 

    0x0d0e1348,// 224 PAY 221 

    0xe101b6fc,// 225 PAY 222 

    0xe2d0b0fc,// 226 PAY 223 

    0xfc29ef1a,// 227 PAY 224 

    0xa3218286,// 228 PAY 225 

    0xc34c76af,// 229 PAY 226 

    0x81674056,// 230 PAY 227 

    0xf1f83950,// 231 PAY 228 

    0xe8490a03,// 232 PAY 229 

    0xec7f089c,// 233 PAY 230 

    0xaba41b29,// 234 PAY 231 

    0x4921e315,// 235 PAY 232 

    0xa4851790,// 236 PAY 233 

    0x527337cc,// 237 PAY 234 

    0x4da83836,// 238 PAY 235 

    0xf389b714,// 239 PAY 236 

    0x670044e7,// 240 PAY 237 

    0xafc85df1,// 241 PAY 238 

    0x6b6f1566,// 242 PAY 239 

    0xe1560309,// 243 PAY 240 

    0xf79ef567,// 244 PAY 241 

    0xcefddcee,// 245 PAY 242 

    0x7c8c3189,// 246 PAY 243 

    0xfef402fa,// 247 PAY 244 

    0xf92b91a6,// 248 PAY 245 

    0xd60010df,// 249 PAY 246 

    0xed4e230c,// 250 PAY 247 

    0xdee30f98,// 251 PAY 248 

    0x86d49486,// 252 PAY 249 

    0xbdcae232,// 253 PAY 250 

    0xbc77608f,// 254 PAY 251 

    0x58d52ab9,// 255 PAY 252 

    0xd7370ec7,// 256 PAY 253 

    0xcbc5053e,// 257 PAY 254 

    0xe1802f07,// 258 PAY 255 

    0x496bc812,// 259 PAY 256 

    0x148de4dd,// 260 PAY 257 

    0xbf7e795b,// 261 PAY 258 

    0xa4d2753a,// 262 PAY 259 

    0x2ca5c862,// 263 PAY 260 

    0xb71c8fb0,// 264 PAY 261 

    0xf614558a,// 265 PAY 262 

    0xaf9a73c4,// 266 PAY 263 

    0x1ff36370,// 267 PAY 264 

    0xfe7de949,// 268 PAY 265 

    0xef967870,// 269 PAY 266 

    0x5d8293e5,// 270 PAY 267 

    0xdaa4d0ed,// 271 PAY 268 

    0xa3ede4ed,// 272 PAY 269 

    0x34d6ce94,// 273 PAY 270 

    0xc0445f55,// 274 PAY 271 

    0x67db0668,// 275 PAY 272 

    0xf1624832,// 276 PAY 273 

    0x501c5058,// 277 PAY 274 

    0x39c5f79c,// 278 PAY 275 

    0x1109e017,// 279 PAY 276 

    0x5a1ec6ee,// 280 PAY 277 

    0x369d95ad,// 281 PAY 278 

    0x2d49be40,// 282 PAY 279 

    0x5f9c74ed,// 283 PAY 280 

    0xb043c7f2,// 284 PAY 281 

    0x44d85e8f,// 285 PAY 282 

    0x04ff5791,// 286 PAY 283 

    0xdb54db4a,// 287 PAY 284 

    0x84683829,// 288 PAY 285 

    0xc2bf8539,// 289 PAY 286 

    0x76a056e7,// 290 PAY 287 

    0xa46686dc,// 291 PAY 288 

    0x4fdf6530,// 292 PAY 289 

    0x669d73ff,// 293 PAY 290 

    0x6d8611c2,// 294 PAY 291 

    0xe0202df8,// 295 PAY 292 

    0xedb83a2a,// 296 PAY 293 

    0xab2ed3ab,// 297 PAY 294 

    0x356bfc49,// 298 PAY 295 

    0xe3e71992,// 299 PAY 296 

    0xfa4b0aee,// 300 PAY 297 

    0x296b43a0,// 301 PAY 298 

    0x3ed82008,// 302 PAY 299 

    0xad64e42f,// 303 PAY 300 

    0xde826f67,// 304 PAY 301 

    0x89feeb58,// 305 PAY 302 

    0x82ec627a,// 306 PAY 303 

    0x03af211c,// 307 PAY 304 

    0x0d992ce1,// 308 PAY 305 

    0xe3a903f2,// 309 PAY 306 

    0xcd9ce720,// 310 PAY 307 

    0x5e642533,// 311 PAY 308 

    0x6ac930dc,// 312 PAY 309 

    0x372c5833,// 313 PAY 310 

    0xe5f7e2c4,// 314 PAY 311 

    0x78e662f3,// 315 PAY 312 

    0xb20ac4bb,// 316 PAY 313 

    0x28b671d0,// 317 PAY 314 

    0x9764febd,// 318 PAY 315 

    0xab4e3a7e,// 319 PAY 316 

    0xbe2309a4,// 320 PAY 317 

    0x9319950d,// 321 PAY 318 

    0x9067a976,// 322 PAY 319 

    0xe332ed3f,// 323 PAY 320 

    0x00925daa,// 324 PAY 321 

    0x3500e2d8,// 325 PAY 322 

    0x4d4b4bb4,// 326 PAY 323 

    0x7bca1e2e,// 327 PAY 324 

    0xf81317f1,// 328 PAY 325 

    0x228a9dc0,// 329 PAY 326 

    0x2f2e290f,// 330 PAY 327 

    0x0c3f0465,// 331 PAY 328 

    0x9c816185,// 332 PAY 329 

    0x76ef3291,// 333 PAY 330 

    0xcb524dcc,// 334 PAY 331 

    0x58a94e24,// 335 PAY 332 

    0xcbf6cf59,// 336 PAY 333 

    0x2e44ebf8,// 337 PAY 334 

    0x2ed209ea,// 338 PAY 335 

    0x49eb1917,// 339 PAY 336 

    0x7841a5a4,// 340 PAY 337 

    0x5bedfcb2,// 341 PAY 338 

    0x2db7683e,// 342 PAY 339 

    0xf59a2dd8,// 343 PAY 340 

    0xaf37d5a5,// 344 PAY 341 

    0x2d0ce308,// 345 PAY 342 

    0x527a9320,// 346 PAY 343 

    0x73b8c864,// 347 PAY 344 

    0x4f9464e0,// 348 PAY 345 

    0xf27c60ff,// 349 PAY 346 

    0xf7b85b63,// 350 PAY 347 

    0x352dd401,// 351 PAY 348 

    0x614a17b3,// 352 PAY 349 

    0xd4bfdfe5,// 353 PAY 350 

    0x6fc92e8e,// 354 PAY 351 

    0xda538c7d,// 355 PAY 352 

    0x1425cd9e,// 356 PAY 353 

    0x69e8f6cb,// 357 PAY 354 

    0x7bbc0d8e,// 358 PAY 355 

    0x8c378b8b,// 359 PAY 356 

    0x671aa0cb,// 360 PAY 357 

    0x33880aff,// 361 PAY 358 

    0x4327f49d,// 362 PAY 359 

    0x3df71093,// 363 PAY 360 

    0x54f1c138,// 364 PAY 361 

    0x2bf61121,// 365 PAY 362 

    0xdc8b2b6a,// 366 PAY 363 

    0x73bc093e,// 367 PAY 364 

    0xae9d0693,// 368 PAY 365 

    0x16af64c4,// 369 PAY 366 

    0xe18e1af4,// 370 PAY 367 

    0x9aea380b,// 371 PAY 368 

    0x9868d705,// 372 PAY 369 

    0xa9a0d7e3,// 373 PAY 370 

    0xe1fb86d8,// 374 PAY 371 

    0x343454a0,// 375 PAY 372 

    0x00e4363b,// 376 PAY 373 

    0x3bdf14ed,// 377 PAY 374 

    0x05ead44b,// 378 PAY 375 

    0x027c2938,// 379 PAY 376 

    0xee210b2e,// 380 PAY 377 

    0x91cd5ff4,// 381 PAY 378 

    0x3b74e469,// 382 PAY 379 

    0x6d8afd4f,// 383 PAY 380 

    0x4f6e5f43,// 384 PAY 381 

    0x96cbc1ef,// 385 PAY 382 

    0x24f16789,// 386 PAY 383 

    0x91652e65,// 387 PAY 384 

    0xceb360a4,// 388 PAY 385 

    0xa0770ee5,// 389 PAY 386 

    0x0403f0d4,// 390 PAY 387 

    0x558315c4,// 391 PAY 388 

    0x583d3d6e,// 392 PAY 389 

    0xf38285ab,// 393 PAY 390 

    0x2722b3b2,// 394 PAY 391 

    0x9022eaac,// 395 PAY 392 

    0xebedbd1a,// 396 PAY 393 

    0xa7c2c18b,// 397 PAY 394 

    0x4b55ee4c,// 398 PAY 395 

    0xfb872784,// 399 PAY 396 

    0xda03cbe5,// 400 PAY 397 

    0x82856583,// 401 PAY 398 

    0x1a78503b,// 402 PAY 399 

    0x47a2a654,// 403 PAY 400 

    0x766f965f,// 404 PAY 401 

    0xd7898c7e,// 405 PAY 402 

    0xf58e245a,// 406 PAY 403 

    0x2f1ed373,// 407 PAY 404 

    0x86f35c3d,// 408 PAY 405 

    0xfcb22a49,// 409 PAY 406 

    0x86d72692,// 410 PAY 407 

    0xeafc60db,// 411 PAY 408 

    0xafe7f4d6,// 412 PAY 409 

    0x9c806667,// 413 PAY 410 

    0x215cd579,// 414 PAY 411 

    0x8d6f58bc,// 415 PAY 412 

    0x4724512c,// 416 PAY 413 

    0x4a8bd6c8,// 417 PAY 414 

    0xd07247e1,// 418 PAY 415 

    0xd4acf821,// 419 PAY 416 

    0xc3bf11ba,// 420 PAY 417 

    0x6ad629d3,// 421 PAY 418 

    0xbc3d305c,// 422 PAY 419 

    0x98829e69,// 423 PAY 420 

    0xd4762a42,// 424 PAY 421 

    0x2184da8f,// 425 PAY 422 

    0x1a3eb327,// 426 PAY 423 

    0x22c68361,// 427 PAY 424 

    0xf0170121,// 428 PAY 425 

    0x00ba7401,// 429 PAY 426 

    0xf7ebfd3e,// 430 PAY 427 

    0x2f12af7e,// 431 PAY 428 

    0x04882835,// 432 PAY 429 

    0xb7935d08,// 433 PAY 430 

    0xb82bacb4,// 434 PAY 431 

    0x9bb4d163,// 435 PAY 432 

    0x79dfb8fd,// 436 PAY 433 

    0x55d600c9,// 437 PAY 434 

    0xd995cf71,// 438 PAY 435 

    0xd317307c,// 439 PAY 436 

    0x848e1d3b,// 440 PAY 437 

    0x598bc1a5,// 441 PAY 438 

    0x7d737d01,// 442 PAY 439 

    0x302fe7cc,// 443 PAY 440 

    0x121e3b87,// 444 PAY 441 

    0x09184172,// 445 PAY 442 

    0x42d89f5a,// 446 PAY 443 

    0x288f0ca4,// 447 PAY 444 

    0x7fdff556,// 448 PAY 445 

    0x639dbe1b,// 449 PAY 446 

    0x3571583c,// 450 PAY 447 

    0xe07c3a96,// 451 PAY 448 

    0xe297f4b9,// 452 PAY 449 

    0x820cfd3e,// 453 PAY 450 

    0x01059d53,// 454 PAY 451 

    0x361fdc67,// 455 PAY 452 

    0xcb1a5fb0,// 456 PAY 453 

    0x5a5d6709,// 457 PAY 454 

    0x405e9c78,// 458 PAY 455 

    0x1929046b,// 459 PAY 456 

    0x24762d47,// 460 PAY 457 

    0x7ce8c9e9,// 461 PAY 458 

    0xdc6b6ffb,// 462 PAY 459 

    0xe18a6fba,// 463 PAY 460 

    0xad6b8018,// 464 PAY 461 

    0x5b715b71,// 465 PAY 462 

    0x807f3089,// 466 PAY 463 

    0xc42d3ee3,// 467 PAY 464 

    0xfb553a7a,// 468 PAY 465 

    0xca9c59bc,// 469 PAY 466 

    0x13885e0d,// 470 PAY 467 

    0xc7c51533,// 471 PAY 468 

    0x0f15015a,// 472 PAY 469 

    0x8f8dfc0b,// 473 PAY 470 

    0xfa3aed2f,// 474 PAY 471 

    0x3112277d,// 475 PAY 472 

    0x596f70c1,// 476 PAY 473 

    0x10326c58,// 477 PAY 474 

    0x9c933db4,// 478 PAY 475 

    0x1eb0a115,// 479 PAY 476 

    0xa508477a,// 480 PAY 477 

    0x6d776fb9,// 481 PAY 478 

    0x712836c4,// 482 PAY 479 

    0x03ab5d77,// 483 PAY 480 

    0x310bbe49,// 484 PAY 481 

    0x257e3bf8,// 485 PAY 482 

    0x0a5bc5f5,// 486 PAY 483 

    0xa8c9678a,// 487 PAY 484 

    0xfcfd89a4,// 488 PAY 485 

    0x728d0738,// 489 PAY 486 

    0x3d57f376,// 490 PAY 487 

    0x5946c3be,// 491 PAY 488 

    0xb51b4fc9,// 492 PAY 489 

    0x98ac4917,// 493 PAY 490 

    0x2903b967,// 494 PAY 491 

    0xdd3da8a7,// 495 PAY 492 

    0x54296891,// 496 PAY 493 

    0xaff94688,// 497 PAY 494 

    0xb0ae5696,// 498 PAY 495 

    0xddaef993,// 499 PAY 496 

    0x78d248b9,// 500 PAY 497 

    0x1992a810,// 501 PAY 498 

    0xb366b5b4,// 502 PAY 499 

    0x325802e5,// 503 PAY 500 

    0x348fa571,// 504 PAY 501 

    0x0ed7d0b0,// 505 PAY 502 

    0x7ee6244e,// 506 PAY 503 

    0x46caf135,// 507 PAY 504 

    0x452fdbae,// 508 PAY 505 

    0x2bc2dcff,// 509 PAY 506 

    0x4a493acb,// 510 PAY 507 

    0x4678bbf7,// 511 PAY 508 

    0x4e25fd57,// 512 PAY 509 

    0x7b660000,// 513 PAY 510 

/// STA is 1 words. 

/// STA num_pkts       : 151 

/// STA pkt_idx        : 156 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02703c97 // 514 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt25_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 115 words. 

/// BDA size     is 453 (0x1c5) 

/// BDA id       is 0xdca0 

    0x01c5dca0,// 3 BDA   1 

/// PAY Generic Data size   : 453 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x5d405d3f,// 4 PAY   1 

    0xf47d5bb9,// 5 PAY   2 

    0x3152ef9a,// 6 PAY   3 

    0x768181b4,// 7 PAY   4 

    0x0bff2877,// 8 PAY   5 

    0x03478903,// 9 PAY   6 

    0xc75d15cf,// 10 PAY   7 

    0x24bad10c,// 11 PAY   8 

    0xd5d0ab45,// 12 PAY   9 

    0xb8ab37ff,// 13 PAY  10 

    0x49bd7ed0,// 14 PAY  11 

    0xb0f8e685,// 15 PAY  12 

    0x54099f91,// 16 PAY  13 

    0x65ada0d0,// 17 PAY  14 

    0x6b151460,// 18 PAY  15 

    0x88059661,// 19 PAY  16 

    0x733560d1,// 20 PAY  17 

    0xae67dc6f,// 21 PAY  18 

    0xf07c56a6,// 22 PAY  19 

    0xc08fc5a1,// 23 PAY  20 

    0x1417515e,// 24 PAY  21 

    0xb7feec1b,// 25 PAY  22 

    0xebbf2996,// 26 PAY  23 

    0x99c9ea6a,// 27 PAY  24 

    0x3188399c,// 28 PAY  25 

    0xc38d12dc,// 29 PAY  26 

    0xba485b91,// 30 PAY  27 

    0x0259d4a0,// 31 PAY  28 

    0x77eebae2,// 32 PAY  29 

    0xcb7b6316,// 33 PAY  30 

    0x2beac27e,// 34 PAY  31 

    0xad5d5ef6,// 35 PAY  32 

    0x59dc9588,// 36 PAY  33 

    0xd3206589,// 37 PAY  34 

    0x42a5ee23,// 38 PAY  35 

    0x8b2f5423,// 39 PAY  36 

    0x3d00d0d7,// 40 PAY  37 

    0x63bfcf39,// 41 PAY  38 

    0xb9d40d86,// 42 PAY  39 

    0x0b97dbe3,// 43 PAY  40 

    0x3775585d,// 44 PAY  41 

    0xd465d292,// 45 PAY  42 

    0x5fe82a12,// 46 PAY  43 

    0x057ece70,// 47 PAY  44 

    0x0f8ef99c,// 48 PAY  45 

    0x197ff913,// 49 PAY  46 

    0x48cdc151,// 50 PAY  47 

    0x88d01b9e,// 51 PAY  48 

    0x8a853f9e,// 52 PAY  49 

    0xe278d1ad,// 53 PAY  50 

    0xc2a7b8b4,// 54 PAY  51 

    0xb82d2b82,// 55 PAY  52 

    0x9e4b56e1,// 56 PAY  53 

    0x97841017,// 57 PAY  54 

    0x2081fe67,// 58 PAY  55 

    0xeac125a5,// 59 PAY  56 

    0xd102170a,// 60 PAY  57 

    0x79563095,// 61 PAY  58 

    0x7cd41db1,// 62 PAY  59 

    0x1f62e5e3,// 63 PAY  60 

    0xd272644d,// 64 PAY  61 

    0x3232000d,// 65 PAY  62 

    0xbd30db2b,// 66 PAY  63 

    0x89dbb70e,// 67 PAY  64 

    0x8950aa89,// 68 PAY  65 

    0xd564a8c9,// 69 PAY  66 

    0x3c01cc3b,// 70 PAY  67 

    0x5f271e3a,// 71 PAY  68 

    0x6a11cca9,// 72 PAY  69 

    0x2cd1fddc,// 73 PAY  70 

    0x2326774f,// 74 PAY  71 

    0x940b6a2a,// 75 PAY  72 

    0xbd848ee6,// 76 PAY  73 

    0x53f38aca,// 77 PAY  74 

    0x30e849ef,// 78 PAY  75 

    0x2541a435,// 79 PAY  76 

    0xd3b16b02,// 80 PAY  77 

    0xe66a512d,// 81 PAY  78 

    0x1150280a,// 82 PAY  79 

    0xb24a8bd0,// 83 PAY  80 

    0x552c9cea,// 84 PAY  81 

    0x0c15b879,// 85 PAY  82 

    0x744112bb,// 86 PAY  83 

    0xdc2a9f03,// 87 PAY  84 

    0x5703ed5e,// 88 PAY  85 

    0xbd7c9e6b,// 89 PAY  86 

    0x16bdc59a,// 90 PAY  87 

    0x89f7e2b8,// 91 PAY  88 

    0x14edc88d,// 92 PAY  89 

    0x4786ec3f,// 93 PAY  90 

    0xd2d8bb14,// 94 PAY  91 

    0xda1c82bc,// 95 PAY  92 

    0x3d16deb6,// 96 PAY  93 

    0xd177ae74,// 97 PAY  94 

    0x47db56c5,// 98 PAY  95 

    0x0ee470bc,// 99 PAY  96 

    0x4cf9a364,// 100 PAY  97 

    0xa8e39c34,// 101 PAY  98 

    0xea33485f,// 102 PAY  99 

    0x7ecd35b6,// 103 PAY 100 

    0xa6aaf191,// 104 PAY 101 

    0x05c6fba1,// 105 PAY 102 

    0x57bb6ce0,// 106 PAY 103 

    0x9a33be40,// 107 PAY 104 

    0xdd219c97,// 108 PAY 105 

    0x236308d8,// 109 PAY 106 

    0x09e5854b,// 110 PAY 107 

    0x81f895c4,// 111 PAY 108 

    0x46cf58c2,// 112 PAY 109 

    0x31a8440e,// 113 PAY 110 

    0x467be644,// 114 PAY 111 

    0xe0c4e5f9,// 115 PAY 112 

    0x725de6cd,// 116 PAY 113 

    0x57000000,// 117 PAY 114 

/// HASH is  16 bytes 

    0xdc2a9f03,// 118 HSH   1 

    0x5703ed5e,// 119 HSH   2 

    0xbd7c9e6b,// 120 HSH   3 

    0x16bdc59a,// 121 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 145 

/// STA pkt_idx        : 230 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x03994d91 // 122 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt26_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 35 words. 

/// BDA size     is 134 (0x86) 

/// BDA id       is 0x9ed4 

    0x00869ed4,// 3 BDA   1 

/// PAY Generic Data size   : 134 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x680450c7,// 4 PAY   1 

    0x27eddf55,// 5 PAY   2 

    0xedc35bc2,// 6 PAY   3 

    0x2aa246a7,// 7 PAY   4 

    0xc7b89fc7,// 8 PAY   5 

    0x50e0afac,// 9 PAY   6 

    0x88d53bd6,// 10 PAY   7 

    0xe73fa6a1,// 11 PAY   8 

    0x71c96987,// 12 PAY   9 

    0x28118cdd,// 13 PAY  10 

    0x8892f878,// 14 PAY  11 

    0xd851777b,// 15 PAY  12 

    0x08084719,// 16 PAY  13 

    0x896e5bda,// 17 PAY  14 

    0x652b30fd,// 18 PAY  15 

    0x72cbfc03,// 19 PAY  16 

    0x431cde6e,// 20 PAY  17 

    0xafe33b91,// 21 PAY  18 

    0x4c6ca3ff,// 22 PAY  19 

    0xe7c3fa74,// 23 PAY  20 

    0xbf169a28,// 24 PAY  21 

    0xd113d8f7,// 25 PAY  22 

    0x24094e70,// 26 PAY  23 

    0x1c1b895a,// 27 PAY  24 

    0x330fca13,// 28 PAY  25 

    0xb7aeead7,// 29 PAY  26 

    0x7a554816,// 30 PAY  27 

    0x9a909003,// 31 PAY  28 

    0x0107de09,// 32 PAY  29 

    0xa6997ca0,// 33 PAY  30 

    0x7a1342b5,// 34 PAY  31 

    0x50c81312,// 35 PAY  32 

    0x76f6915c,// 36 PAY  33 

    0x70000000,// 37 PAY  34 

/// STA is 1 words. 

/// STA num_pkts       : 14 

/// STA pkt_idx        : 249 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc8 

    0x03e4c80e // 38 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt27_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 448 words. 

/// BDA size     is 1787 (0x6fb) 

/// BDA id       is 0x601b 

    0x06fb601b,// 3 BDA   1 

/// PAY Generic Data size   : 1787 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x71609395,// 4 PAY   1 

    0x6c64867b,// 5 PAY   2 

    0x5bb6228d,// 6 PAY   3 

    0xc8c86762,// 7 PAY   4 

    0x2ca14855,// 8 PAY   5 

    0x7a722ff4,// 9 PAY   6 

    0xad73e42f,// 10 PAY   7 

    0xbcc47455,// 11 PAY   8 

    0xcdf18a40,// 12 PAY   9 

    0xd3f62183,// 13 PAY  10 

    0x92775984,// 14 PAY  11 

    0xe34236bb,// 15 PAY  12 

    0xb44a5934,// 16 PAY  13 

    0x71173019,// 17 PAY  14 

    0x84e46f87,// 18 PAY  15 

    0x5000fe1f,// 19 PAY  16 

    0x794ba69d,// 20 PAY  17 

    0x9d8b4f8f,// 21 PAY  18 

    0x5b108b2e,// 22 PAY  19 

    0x186ae61f,// 23 PAY  20 

    0x12a9c891,// 24 PAY  21 

    0xa3304bf1,// 25 PAY  22 

    0xb890593b,// 26 PAY  23 

    0x4576f216,// 27 PAY  24 

    0xc60bb5d7,// 28 PAY  25 

    0xd9db3d56,// 29 PAY  26 

    0xcf73be06,// 30 PAY  27 

    0x0561875d,// 31 PAY  28 

    0x37b2baaa,// 32 PAY  29 

    0x2093dac6,// 33 PAY  30 

    0x1be8651f,// 34 PAY  31 

    0xfc1ffd3c,// 35 PAY  32 

    0x5b34902d,// 36 PAY  33 

    0xd27c4575,// 37 PAY  34 

    0xa5a69a35,// 38 PAY  35 

    0x48a09f4d,// 39 PAY  36 

    0x6e6f1b1b,// 40 PAY  37 

    0xb5bfacbe,// 41 PAY  38 

    0xafa63d08,// 42 PAY  39 

    0xbeddd132,// 43 PAY  40 

    0x9c5a6580,// 44 PAY  41 

    0x2473f568,// 45 PAY  42 

    0x490a6dcf,// 46 PAY  43 

    0xd04eac19,// 47 PAY  44 

    0xb859904f,// 48 PAY  45 

    0x6d808980,// 49 PAY  46 

    0xe288deda,// 50 PAY  47 

    0x0b3f9da0,// 51 PAY  48 

    0x8166e540,// 52 PAY  49 

    0xce50acb0,// 53 PAY  50 

    0xc98a92fc,// 54 PAY  51 

    0x86fe576c,// 55 PAY  52 

    0x5ae172ca,// 56 PAY  53 

    0x639282a2,// 57 PAY  54 

    0xbdd61997,// 58 PAY  55 

    0x87d9493d,// 59 PAY  56 

    0xedde6002,// 60 PAY  57 

    0xda19474c,// 61 PAY  58 

    0xf468656b,// 62 PAY  59 

    0xc8c807bb,// 63 PAY  60 

    0xf4dedd31,// 64 PAY  61 

    0x0463914b,// 65 PAY  62 

    0x48e8d065,// 66 PAY  63 

    0x88828adc,// 67 PAY  64 

    0x565a1592,// 68 PAY  65 

    0x990dc53c,// 69 PAY  66 

    0xb3f40b41,// 70 PAY  67 

    0xf66e05e7,// 71 PAY  68 

    0x171cc561,// 72 PAY  69 

    0x9a9d5778,// 73 PAY  70 

    0xfbe60dec,// 74 PAY  71 

    0xc5d524d4,// 75 PAY  72 

    0xf15497df,// 76 PAY  73 

    0x5f980a1c,// 77 PAY  74 

    0x3096b2ab,// 78 PAY  75 

    0x531fbcdf,// 79 PAY  76 

    0x44cf542f,// 80 PAY  77 

    0x388f4d04,// 81 PAY  78 

    0x1037a2a3,// 82 PAY  79 

    0x466ba3c2,// 83 PAY  80 

    0xef23b37e,// 84 PAY  81 

    0xf63f4fca,// 85 PAY  82 

    0x7635a31c,// 86 PAY  83 

    0x2a59b677,// 87 PAY  84 

    0x4f573ebe,// 88 PAY  85 

    0x6dd4a533,// 89 PAY  86 

    0x68d5ecd2,// 90 PAY  87 

    0x830ea1d3,// 91 PAY  88 

    0x8c7df4b1,// 92 PAY  89 

    0x63e1c45d,// 93 PAY  90 

    0xcfe70ac6,// 94 PAY  91 

    0xe791839d,// 95 PAY  92 

    0x075f81d2,// 96 PAY  93 

    0xa1783e7f,// 97 PAY  94 

    0x316c658c,// 98 PAY  95 

    0x0ec406c6,// 99 PAY  96 

    0xcd1a91b4,// 100 PAY  97 

    0x7f6b78b8,// 101 PAY  98 

    0xebf3d7aa,// 102 PAY  99 

    0x7686f91c,// 103 PAY 100 

    0xb518e789,// 104 PAY 101 

    0x455af88a,// 105 PAY 102 

    0xa746d672,// 106 PAY 103 

    0x5d680e97,// 107 PAY 104 

    0x8116d227,// 108 PAY 105 

    0x7f33ce8a,// 109 PAY 106 

    0xb423682f,// 110 PAY 107 

    0x7b3014de,// 111 PAY 108 

    0x08fc9f24,// 112 PAY 109 

    0x4cd3b535,// 113 PAY 110 

    0x4c4f127a,// 114 PAY 111 

    0x5aff0f2e,// 115 PAY 112 

    0xb3b45eac,// 116 PAY 113 

    0x89c311ee,// 117 PAY 114 

    0x2607ec4d,// 118 PAY 115 

    0x17101c40,// 119 PAY 116 

    0xcab3b6b9,// 120 PAY 117 

    0x1cdb5a98,// 121 PAY 118 

    0xe7d4e5c6,// 122 PAY 119 

    0x2fe91b55,// 123 PAY 120 

    0x91d25bdf,// 124 PAY 121 

    0xf0564ce9,// 125 PAY 122 

    0x660d0e82,// 126 PAY 123 

    0x381b3ac0,// 127 PAY 124 

    0xcf023707,// 128 PAY 125 

    0xec3a7444,// 129 PAY 126 

    0x5feb55e2,// 130 PAY 127 

    0xa0eb1ddb,// 131 PAY 128 

    0xd3ccf203,// 132 PAY 129 

    0x7385ed93,// 133 PAY 130 

    0x4ee0dc69,// 134 PAY 131 

    0x91a2ca00,// 135 PAY 132 

    0x6cc4e883,// 136 PAY 133 

    0x2d4fa6f4,// 137 PAY 134 

    0x748cea69,// 138 PAY 135 

    0xd6dc2b65,// 139 PAY 136 

    0x3b48624b,// 140 PAY 137 

    0xf70f13c9,// 141 PAY 138 

    0x24dbdbdc,// 142 PAY 139 

    0x0c25e62a,// 143 PAY 140 

    0x55cbcc03,// 144 PAY 141 

    0xa7b78e50,// 145 PAY 142 

    0xe9d0be1a,// 146 PAY 143 

    0x0279911d,// 147 PAY 144 

    0x917b5fcf,// 148 PAY 145 

    0x3f741cc1,// 149 PAY 146 

    0xfc9fc50e,// 150 PAY 147 

    0xafe0b03a,// 151 PAY 148 

    0x9c11f81e,// 152 PAY 149 

    0x4170b3da,// 153 PAY 150 

    0x49db9a28,// 154 PAY 151 

    0xce0f61c6,// 155 PAY 152 

    0x8353d0b8,// 156 PAY 153 

    0x5f6b87fb,// 157 PAY 154 

    0x554511fc,// 158 PAY 155 

    0xf932d764,// 159 PAY 156 

    0x9fe8f614,// 160 PAY 157 

    0x0aa172cb,// 161 PAY 158 

    0xb926dad9,// 162 PAY 159 

    0x7a1927ca,// 163 PAY 160 

    0x305cd413,// 164 PAY 161 

    0xece9213b,// 165 PAY 162 

    0xa55c2af6,// 166 PAY 163 

    0xb27d73dc,// 167 PAY 164 

    0x82a765d1,// 168 PAY 165 

    0x300ed7bc,// 169 PAY 166 

    0xc133e1c1,// 170 PAY 167 

    0xc7a7e415,// 171 PAY 168 

    0x480f0080,// 172 PAY 169 

    0xba7dc32e,// 173 PAY 170 

    0xe4358d2a,// 174 PAY 171 

    0xcb395692,// 175 PAY 172 

    0xb5d98273,// 176 PAY 173 

    0x505a0aa4,// 177 PAY 174 

    0xbf838732,// 178 PAY 175 

    0x64f7eb1e,// 179 PAY 176 

    0xecf2cdb2,// 180 PAY 177 

    0x3c37c299,// 181 PAY 178 

    0x2c035203,// 182 PAY 179 

    0x7d2825a0,// 183 PAY 180 

    0x7776a028,// 184 PAY 181 

    0xd1551f82,// 185 PAY 182 

    0x399d982f,// 186 PAY 183 

    0x342aba99,// 187 PAY 184 

    0x2a9e051c,// 188 PAY 185 

    0xfec0c878,// 189 PAY 186 

    0x113b4972,// 190 PAY 187 

    0x7d3be178,// 191 PAY 188 

    0x5fa472e9,// 192 PAY 189 

    0x3368bb7f,// 193 PAY 190 

    0xa5f03f21,// 194 PAY 191 

    0xce41a9a3,// 195 PAY 192 

    0xdcab506e,// 196 PAY 193 

    0xb64aa7aa,// 197 PAY 194 

    0x0dd8eab5,// 198 PAY 195 

    0x94bc9160,// 199 PAY 196 

    0xf567fa8f,// 200 PAY 197 

    0xace61eea,// 201 PAY 198 

    0x097b7bbc,// 202 PAY 199 

    0x1573d64f,// 203 PAY 200 

    0x9a75191b,// 204 PAY 201 

    0xb291fd0b,// 205 PAY 202 

    0xfb7b17f5,// 206 PAY 203 

    0x56c9c770,// 207 PAY 204 

    0x13bdb6ff,// 208 PAY 205 

    0xa46ccc58,// 209 PAY 206 

    0x89d232d7,// 210 PAY 207 

    0xec71dc7b,// 211 PAY 208 

    0x166f979e,// 212 PAY 209 

    0xb0ea6910,// 213 PAY 210 

    0xdb74d2f3,// 214 PAY 211 

    0xe2ed53b1,// 215 PAY 212 

    0x464ea985,// 216 PAY 213 

    0x6a6ff746,// 217 PAY 214 

    0xf7433900,// 218 PAY 215 

    0x880c1141,// 219 PAY 216 

    0x5935c0f7,// 220 PAY 217 

    0x46df027b,// 221 PAY 218 

    0xc149cc82,// 222 PAY 219 

    0xef7878b7,// 223 PAY 220 

    0x1084c283,// 224 PAY 221 

    0xfb8d15b7,// 225 PAY 222 

    0x77aaa02d,// 226 PAY 223 

    0x9b64d34f,// 227 PAY 224 

    0xb155a21d,// 228 PAY 225 

    0x4b159ac9,// 229 PAY 226 

    0x983fa0b3,// 230 PAY 227 

    0x08e5e5d1,// 231 PAY 228 

    0xe804c210,// 232 PAY 229 

    0xeb770aea,// 233 PAY 230 

    0xb3516f8d,// 234 PAY 231 

    0x9a6c4ad3,// 235 PAY 232 

    0x0fcf2805,// 236 PAY 233 

    0xd07c0319,// 237 PAY 234 

    0xe8cb1e5a,// 238 PAY 235 

    0x98226876,// 239 PAY 236 

    0x8391858b,// 240 PAY 237 

    0xc038037e,// 241 PAY 238 

    0x8cfe5e44,// 242 PAY 239 

    0x2c288d08,// 243 PAY 240 

    0x883389a7,// 244 PAY 241 

    0x1993ca98,// 245 PAY 242 

    0xaa00f6d5,// 246 PAY 243 

    0xab84e5b4,// 247 PAY 244 

    0x7e12a846,// 248 PAY 245 

    0x74a7a5f5,// 249 PAY 246 

    0xafe8b346,// 250 PAY 247 

    0x46cbb766,// 251 PAY 248 

    0x2c7eaa3c,// 252 PAY 249 

    0x5e347f6b,// 253 PAY 250 

    0x4b40daec,// 254 PAY 251 

    0x2fd36456,// 255 PAY 252 

    0xa1988198,// 256 PAY 253 

    0x852b443b,// 257 PAY 254 

    0x5b3f38b3,// 258 PAY 255 

    0x7d67a0f6,// 259 PAY 256 

    0xb846a376,// 260 PAY 257 

    0xcf1ac5dd,// 261 PAY 258 

    0x71fa9dae,// 262 PAY 259 

    0xe8d28bb3,// 263 PAY 260 

    0x709bf0da,// 264 PAY 261 

    0x03d5ef13,// 265 PAY 262 

    0x8c404311,// 266 PAY 263 

    0xb42ecf81,// 267 PAY 264 

    0x3c618323,// 268 PAY 265 

    0x13e963af,// 269 PAY 266 

    0xab6df4c7,// 270 PAY 267 

    0xf629c486,// 271 PAY 268 

    0x2d82a7b3,// 272 PAY 269 

    0xa0b40480,// 273 PAY 270 

    0x17e09914,// 274 PAY 271 

    0xfc4c1999,// 275 PAY 272 

    0xd7267b3e,// 276 PAY 273 

    0x943278cc,// 277 PAY 274 

    0xf98f5281,// 278 PAY 275 

    0xb683e5e0,// 279 PAY 276 

    0x18f096ea,// 280 PAY 277 

    0x5ef9c65e,// 281 PAY 278 

    0xdbf04b17,// 282 PAY 279 

    0x321e456a,// 283 PAY 280 

    0x680feb82,// 284 PAY 281 

    0xe43ea188,// 285 PAY 282 

    0x67250d63,// 286 PAY 283 

    0xdaf6876b,// 287 PAY 284 

    0xcee624bc,// 288 PAY 285 

    0x23d3f7b0,// 289 PAY 286 

    0x59b434c2,// 290 PAY 287 

    0x41c73cbd,// 291 PAY 288 

    0xf39f1b46,// 292 PAY 289 

    0x098b7647,// 293 PAY 290 

    0x26bfbd12,// 294 PAY 291 

    0x9e654f45,// 295 PAY 292 

    0x5fb64833,// 296 PAY 293 

    0xcf59a242,// 297 PAY 294 

    0xe543d401,// 298 PAY 295 

    0x2e9ae414,// 299 PAY 296 

    0xbe0a1b32,// 300 PAY 297 

    0xb674555b,// 301 PAY 298 

    0xdb7126c4,// 302 PAY 299 

    0x88564358,// 303 PAY 300 

    0x21daa2ff,// 304 PAY 301 

    0x2e67e4ba,// 305 PAY 302 

    0x417c50eb,// 306 PAY 303 

    0x72c55fde,// 307 PAY 304 

    0x15c73d4b,// 308 PAY 305 

    0x127993ce,// 309 PAY 306 

    0xdc793621,// 310 PAY 307 

    0x0f83b236,// 311 PAY 308 

    0xc79a444e,// 312 PAY 309 

    0x46067b36,// 313 PAY 310 

    0x1b7866f1,// 314 PAY 311 

    0x6b0b3fa0,// 315 PAY 312 

    0xe109f06a,// 316 PAY 313 

    0x4c7523d6,// 317 PAY 314 

    0x52b18cad,// 318 PAY 315 

    0x38c9aaa5,// 319 PAY 316 

    0xac2a693c,// 320 PAY 317 

    0x049f6bb0,// 321 PAY 318 

    0x998f2416,// 322 PAY 319 

    0xa403cc45,// 323 PAY 320 

    0xd5948800,// 324 PAY 321 

    0xda2c5d29,// 325 PAY 322 

    0x2a4eb4bb,// 326 PAY 323 

    0xb3e47bfb,// 327 PAY 324 

    0xd9d3491e,// 328 PAY 325 

    0x13f5f000,// 329 PAY 326 

    0xfaa6748d,// 330 PAY 327 

    0xb314b78a,// 331 PAY 328 

    0xa8745d54,// 332 PAY 329 

    0x588b9680,// 333 PAY 330 

    0x279f477b,// 334 PAY 331 

    0xe0ebb850,// 335 PAY 332 

    0xc16e9c7c,// 336 PAY 333 

    0x9af5bda1,// 337 PAY 334 

    0x29e450cd,// 338 PAY 335 

    0x97710016,// 339 PAY 336 

    0x764dc247,// 340 PAY 337 

    0x21228962,// 341 PAY 338 

    0x13288e24,// 342 PAY 339 

    0xaed4ca7c,// 343 PAY 340 

    0x324c7f18,// 344 PAY 341 

    0x97955f58,// 345 PAY 342 

    0xb822ca54,// 346 PAY 343 

    0xa471f30c,// 347 PAY 344 

    0x16a72e2a,// 348 PAY 345 

    0x264a7043,// 349 PAY 346 

    0xb09f618f,// 350 PAY 347 

    0x8560c677,// 351 PAY 348 

    0xf0455c04,// 352 PAY 349 

    0xa2042540,// 353 PAY 350 

    0x381f90b5,// 354 PAY 351 

    0x52afd3e5,// 355 PAY 352 

    0x6bc86827,// 356 PAY 353 

    0x55157ba2,// 357 PAY 354 

    0x2af032fa,// 358 PAY 355 

    0x4c9e77bc,// 359 PAY 356 

    0xfd12e3ac,// 360 PAY 357 

    0x6f6b2ca9,// 361 PAY 358 

    0x2b8c4aba,// 362 PAY 359 

    0x183b68f9,// 363 PAY 360 

    0x9bc589b6,// 364 PAY 361 

    0x6a18f4c6,// 365 PAY 362 

    0xd27f7a65,// 366 PAY 363 

    0x54fbb50d,// 367 PAY 364 

    0x9d83ebec,// 368 PAY 365 

    0x3013af9c,// 369 PAY 366 

    0xb8324d23,// 370 PAY 367 

    0xdfed1952,// 371 PAY 368 

    0x72a43a28,// 372 PAY 369 

    0x12906d5d,// 373 PAY 370 

    0x2b957967,// 374 PAY 371 

    0xbe103c45,// 375 PAY 372 

    0x4e769252,// 376 PAY 373 

    0x7ccaec99,// 377 PAY 374 

    0x2af84307,// 378 PAY 375 

    0x5d327cc5,// 379 PAY 376 

    0x95481873,// 380 PAY 377 

    0x5ead798b,// 381 PAY 378 

    0xab013bff,// 382 PAY 379 

    0x890b2a76,// 383 PAY 380 

    0x6b177126,// 384 PAY 381 

    0xa85ca3f0,// 385 PAY 382 

    0xcc0524e8,// 386 PAY 383 

    0x2a2ff3bf,// 387 PAY 384 

    0xfe0e95af,// 388 PAY 385 

    0x3de12dcf,// 389 PAY 386 

    0x0c58d0df,// 390 PAY 387 

    0xf559697f,// 391 PAY 388 

    0x297f0a52,// 392 PAY 389 

    0x32ee6bc4,// 393 PAY 390 

    0xaa664786,// 394 PAY 391 

    0x67c34955,// 395 PAY 392 

    0xebe4f062,// 396 PAY 393 

    0x31d94c5c,// 397 PAY 394 

    0x85898a57,// 398 PAY 395 

    0x61e28fee,// 399 PAY 396 

    0x4efb0b50,// 400 PAY 397 

    0x62838b01,// 401 PAY 398 

    0xff65819d,// 402 PAY 399 

    0x838f73ab,// 403 PAY 400 

    0xe9d9da52,// 404 PAY 401 

    0xb2a717ca,// 405 PAY 402 

    0x0ae74865,// 406 PAY 403 

    0xd07b24b5,// 407 PAY 404 

    0x3aff003e,// 408 PAY 405 

    0x76ab2b89,// 409 PAY 406 

    0xa2df3618,// 410 PAY 407 

    0xb4707d17,// 411 PAY 408 

    0x41af5508,// 412 PAY 409 

    0xf3244d93,// 413 PAY 410 

    0xb2170312,// 414 PAY 411 

    0x656363d5,// 415 PAY 412 

    0x3c5cd69f,// 416 PAY 413 

    0xaa27535f,// 417 PAY 414 

    0xf50a1d0f,// 418 PAY 415 

    0x5b3f0249,// 419 PAY 416 

    0x43f775c2,// 420 PAY 417 

    0xaa812ec1,// 421 PAY 418 

    0x5bef8257,// 422 PAY 419 

    0x4ce5c865,// 423 PAY 420 

    0x5fce7be5,// 424 PAY 421 

    0x553dcbf1,// 425 PAY 422 

    0xd357c400,// 426 PAY 423 

    0x021b86f8,// 427 PAY 424 

    0xa3f659b5,// 428 PAY 425 

    0x4f8b40e1,// 429 PAY 426 

    0x1d65da9e,// 430 PAY 427 

    0x5306fa45,// 431 PAY 428 

    0xdaa200ef,// 432 PAY 429 

    0x3cb7f535,// 433 PAY 430 

    0x477b0e20,// 434 PAY 431 

    0x447420c8,// 435 PAY 432 

    0x7ed7f06a,// 436 PAY 433 

    0x104b4a9b,// 437 PAY 434 

    0xd02545ae,// 438 PAY 435 

    0xb3714988,// 439 PAY 436 

    0x39f0e5c9,// 440 PAY 437 

    0xdfe92a59,// 441 PAY 438 

    0x654af32a,// 442 PAY 439 

    0x9674f2f2,// 443 PAY 440 

    0x0393647c,// 444 PAY 441 

    0xb4523744,// 445 PAY 442 

    0x083183d5,// 446 PAY 443 

    0x34e423a4,// 447 PAY 444 

    0x933a3f1f,// 448 PAY 445 

    0xcc402871,// 449 PAY 446 

    0xb7b9c800,// 450 PAY 447 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x03f8c1d6 // 451 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt28_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 285 words. 

/// BDA size     is 1133 (0x46d) 

/// BDA id       is 0x24a3 

    0x046d24a3,// 3 BDA   1 

/// PAY Generic Data size   : 1133 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x3f614c14,// 4 PAY   1 

    0x8408fcb9,// 5 PAY   2 

    0xdb600bb3,// 6 PAY   3 

    0xd3c8bd16,// 7 PAY   4 

    0xe46be464,// 8 PAY   5 

    0xd6ea03d2,// 9 PAY   6 

    0x76b0ef48,// 10 PAY   7 

    0x6ddcc8e2,// 11 PAY   8 

    0x1d2461fe,// 12 PAY   9 

    0x30a97d17,// 13 PAY  10 

    0xfac16eff,// 14 PAY  11 

    0xa90527ff,// 15 PAY  12 

    0x33b7f059,// 16 PAY  13 

    0x11ca82ad,// 17 PAY  14 

    0x1eecf4b3,// 18 PAY  15 

    0x0f86f155,// 19 PAY  16 

    0x7f72a861,// 20 PAY  17 

    0x9a27e3f5,// 21 PAY  18 

    0xdae6e14e,// 22 PAY  19 

    0x4d8508da,// 23 PAY  20 

    0xe4ed2a7a,// 24 PAY  21 

    0x1cf55ec8,// 25 PAY  22 

    0x7fb2b258,// 26 PAY  23 

    0x11ae51e9,// 27 PAY  24 

    0xd0007996,// 28 PAY  25 

    0x02a7a5b9,// 29 PAY  26 

    0xc88e4207,// 30 PAY  27 

    0x3908bbc7,// 31 PAY  28 

    0xc4ca33d6,// 32 PAY  29 

    0x415d6933,// 33 PAY  30 

    0x47f9f655,// 34 PAY  31 

    0xb07bf970,// 35 PAY  32 

    0x4b1cc118,// 36 PAY  33 

    0x8f9d7488,// 37 PAY  34 

    0xc6885a50,// 38 PAY  35 

    0x71a0fc0d,// 39 PAY  36 

    0x4f155524,// 40 PAY  37 

    0x456ceff5,// 41 PAY  38 

    0x939cf0c3,// 42 PAY  39 

    0xaf30c985,// 43 PAY  40 

    0xc95eb642,// 44 PAY  41 

    0xde251884,// 45 PAY  42 

    0x92845f61,// 46 PAY  43 

    0xc35ca720,// 47 PAY  44 

    0x7c9ca7e8,// 48 PAY  45 

    0x1e34917a,// 49 PAY  46 

    0x4e77a3b7,// 50 PAY  47 

    0x62ee2278,// 51 PAY  48 

    0x096b8a69,// 52 PAY  49 

    0xcaa4e8f9,// 53 PAY  50 

    0xb2418186,// 54 PAY  51 

    0xd579b2f1,// 55 PAY  52 

    0x2c31851f,// 56 PAY  53 

    0x12d00c66,// 57 PAY  54 

    0x23319a97,// 58 PAY  55 

    0x7e432b03,// 59 PAY  56 

    0xbe604377,// 60 PAY  57 

    0x7ab510d7,// 61 PAY  58 

    0x3f74d11c,// 62 PAY  59 

    0x0097964c,// 63 PAY  60 

    0xd59da03f,// 64 PAY  61 

    0xe8e4bfa6,// 65 PAY  62 

    0x27b92579,// 66 PAY  63 

    0x018b1ea5,// 67 PAY  64 

    0x06916f8f,// 68 PAY  65 

    0xda537b42,// 69 PAY  66 

    0x28829413,// 70 PAY  67 

    0x33061b83,// 71 PAY  68 

    0xb0e6282a,// 72 PAY  69 

    0x41d0fc28,// 73 PAY  70 

    0x55ea3aff,// 74 PAY  71 

    0x469b0701,// 75 PAY  72 

    0x243b4370,// 76 PAY  73 

    0xbe811ff6,// 77 PAY  74 

    0x4d1d8a95,// 78 PAY  75 

    0x6a662ad9,// 79 PAY  76 

    0x93dd55eb,// 80 PAY  77 

    0x560107d5,// 81 PAY  78 

    0x20995800,// 82 PAY  79 

    0xe4b601fe,// 83 PAY  80 

    0x98175199,// 84 PAY  81 

    0xa4f209a4,// 85 PAY  82 

    0x614e847c,// 86 PAY  83 

    0x2294b9e2,// 87 PAY  84 

    0xb250dfd1,// 88 PAY  85 

    0x8fe2cd49,// 89 PAY  86 

    0xc63c4a90,// 90 PAY  87 

    0x9c5a89d7,// 91 PAY  88 

    0x994b7622,// 92 PAY  89 

    0xa63bac38,// 93 PAY  90 

    0xd9de9334,// 94 PAY  91 

    0xa904c0c3,// 95 PAY  92 

    0x2003ba24,// 96 PAY  93 

    0x72e452b2,// 97 PAY  94 

    0xc7ed71cd,// 98 PAY  95 

    0x3738017a,// 99 PAY  96 

    0x488c39eb,// 100 PAY  97 

    0x37c75334,// 101 PAY  98 

    0xab2f28d2,// 102 PAY  99 

    0x48fb48fb,// 103 PAY 100 

    0xe5e55632,// 104 PAY 101 

    0x82d23804,// 105 PAY 102 

    0xf6d9f558,// 106 PAY 103 

    0x4d02e0c8,// 107 PAY 104 

    0x1d469fd1,// 108 PAY 105 

    0xc472208c,// 109 PAY 106 

    0x132278ca,// 110 PAY 107 

    0xb05166ac,// 111 PAY 108 

    0x2ae0ecc1,// 112 PAY 109 

    0x7bf7ae33,// 113 PAY 110 

    0xa2cf8326,// 114 PAY 111 

    0xdcd7e261,// 115 PAY 112 

    0x242c2df0,// 116 PAY 113 

    0xa3c66e46,// 117 PAY 114 

    0x577e2f0c,// 118 PAY 115 

    0x2fc9c54e,// 119 PAY 116 

    0xf2f4925a,// 120 PAY 117 

    0x3dc8a830,// 121 PAY 118 

    0x16f8c985,// 122 PAY 119 

    0x12fb4d90,// 123 PAY 120 

    0x2adea9fd,// 124 PAY 121 

    0xe5c171d4,// 125 PAY 122 

    0x4ca933e6,// 126 PAY 123 

    0xbb8a729d,// 127 PAY 124 

    0x13346c49,// 128 PAY 125 

    0x16733b6a,// 129 PAY 126 

    0xfd9439c4,// 130 PAY 127 

    0x778cb88a,// 131 PAY 128 

    0x14623871,// 132 PAY 129 

    0x14c0daba,// 133 PAY 130 

    0xb72ef37a,// 134 PAY 131 

    0x24ae7de6,// 135 PAY 132 

    0x00382e34,// 136 PAY 133 

    0x6c475dbe,// 137 PAY 134 

    0xad267448,// 138 PAY 135 

    0x864f6e64,// 139 PAY 136 

    0x4938f51d,// 140 PAY 137 

    0x8951f9d9,// 141 PAY 138 

    0x08c36268,// 142 PAY 139 

    0x4d96ec72,// 143 PAY 140 

    0xa7fd8e54,// 144 PAY 141 

    0x38e0c1c7,// 145 PAY 142 

    0xa00af3a7,// 146 PAY 143 

    0x2254febf,// 147 PAY 144 

    0x72267066,// 148 PAY 145 

    0x53d29d78,// 149 PAY 146 

    0x3b228e73,// 150 PAY 147 

    0xdcccc8c5,// 151 PAY 148 

    0x9c3b3cd3,// 152 PAY 149 

    0xd0214db1,// 153 PAY 150 

    0x11994723,// 154 PAY 151 

    0x5bd3b5d6,// 155 PAY 152 

    0x3ec8b269,// 156 PAY 153 

    0xcbe16cda,// 157 PAY 154 

    0x4aa07623,// 158 PAY 155 

    0xcd4682fc,// 159 PAY 156 

    0x14721f55,// 160 PAY 157 

    0x61bbfb88,// 161 PAY 158 

    0x298c97e7,// 162 PAY 159 

    0xc2bb8a0b,// 163 PAY 160 

    0xb0986a58,// 164 PAY 161 

    0x422145f2,// 165 PAY 162 

    0x4433c358,// 166 PAY 163 

    0x1bcce3d5,// 167 PAY 164 

    0xefa74849,// 168 PAY 165 

    0xb323ed99,// 169 PAY 166 

    0x8d4ed436,// 170 PAY 167 

    0xfab34cd7,// 171 PAY 168 

    0x4e9e2bbe,// 172 PAY 169 

    0xbbe2c00e,// 173 PAY 170 

    0xc59874aa,// 174 PAY 171 

    0xb40aeccb,// 175 PAY 172 

    0x06601553,// 176 PAY 173 

    0x40b008aa,// 177 PAY 174 

    0x840ff390,// 178 PAY 175 

    0x05647ea4,// 179 PAY 176 

    0x8e2ab29e,// 180 PAY 177 

    0x6d795f62,// 181 PAY 178 

    0x4b5e26df,// 182 PAY 179 

    0x82b516fe,// 183 PAY 180 

    0xd52b87d3,// 184 PAY 181 

    0xa8f92792,// 185 PAY 182 

    0x4ac43252,// 186 PAY 183 

    0xcac7604f,// 187 PAY 184 

    0x82a6dbdb,// 188 PAY 185 

    0x9f370de8,// 189 PAY 186 

    0xbf071cd0,// 190 PAY 187 

    0x33f76e2b,// 191 PAY 188 

    0x7f74421f,// 192 PAY 189 

    0x0e30aa13,// 193 PAY 190 

    0x542bb53f,// 194 PAY 191 

    0x1f874acc,// 195 PAY 192 

    0x98089caf,// 196 PAY 193 

    0x8ae6f424,// 197 PAY 194 

    0x2b3ad38f,// 198 PAY 195 

    0x8fd07439,// 199 PAY 196 

    0x91a4b6b5,// 200 PAY 197 

    0x46490b9b,// 201 PAY 198 

    0x6710e2d1,// 202 PAY 199 

    0x2b1203a7,// 203 PAY 200 

    0xeecf633a,// 204 PAY 201 

    0xfac1311a,// 205 PAY 202 

    0x03e452a4,// 206 PAY 203 

    0x6311561b,// 207 PAY 204 

    0x3f3aa77e,// 208 PAY 205 

    0xc09be41d,// 209 PAY 206 

    0x4e77992a,// 210 PAY 207 

    0x8ecde911,// 211 PAY 208 

    0x60be04b1,// 212 PAY 209 

    0xe2bd31a0,// 213 PAY 210 

    0x1c87e278,// 214 PAY 211 

    0xcfa6e3b9,// 215 PAY 212 

    0x7bc8bc72,// 216 PAY 213 

    0x0e038cc9,// 217 PAY 214 

    0x9294ca67,// 218 PAY 215 

    0x25419164,// 219 PAY 216 

    0x89babc96,// 220 PAY 217 

    0x20e662bf,// 221 PAY 218 

    0xfd79d9ab,// 222 PAY 219 

    0x8628c252,// 223 PAY 220 

    0x20916bf9,// 224 PAY 221 

    0xfe274369,// 225 PAY 222 

    0x2ad1dd1f,// 226 PAY 223 

    0x6178d11f,// 227 PAY 224 

    0x0b6d479f,// 228 PAY 225 

    0x312e92f3,// 229 PAY 226 

    0x90124922,// 230 PAY 227 

    0xbaf82b70,// 231 PAY 228 

    0xbfb14f64,// 232 PAY 229 

    0xc3eaf093,// 233 PAY 230 

    0x2e399875,// 234 PAY 231 

    0xd876feab,// 235 PAY 232 

    0xce4c79bd,// 236 PAY 233 

    0x6f503e4d,// 237 PAY 234 

    0x564b7b24,// 238 PAY 235 

    0x0baa53ce,// 239 PAY 236 

    0xdeddf6c3,// 240 PAY 237 

    0x97bb49c5,// 241 PAY 238 

    0xb650f7d9,// 242 PAY 239 

    0x0d025088,// 243 PAY 240 

    0x49758c5e,// 244 PAY 241 

    0x65b62b43,// 245 PAY 242 

    0xdebf5738,// 246 PAY 243 

    0x9e5285c5,// 247 PAY 244 

    0x73eb350d,// 248 PAY 245 

    0x33d5ae71,// 249 PAY 246 

    0x62d489eb,// 250 PAY 247 

    0xe873bb5b,// 251 PAY 248 

    0x8d84ba27,// 252 PAY 249 

    0xe72beab2,// 253 PAY 250 

    0xfd3fe1a6,// 254 PAY 251 

    0x63ffab63,// 255 PAY 252 

    0xc9fbca15,// 256 PAY 253 

    0x4d248eec,// 257 PAY 254 

    0xa9319df3,// 258 PAY 255 

    0x26b6b359,// 259 PAY 256 

    0xdefe6e9f,// 260 PAY 257 

    0x0fc344c4,// 261 PAY 258 

    0xefcb1f17,// 262 PAY 259 

    0x37d976ae,// 263 PAY 260 

    0xbf1bd335,// 264 PAY 261 

    0x24fb2e1c,// 265 PAY 262 

    0xc420993c,// 266 PAY 263 

    0x73421364,// 267 PAY 264 

    0xe5c4cbe2,// 268 PAY 265 

    0x5d448a3f,// 269 PAY 266 

    0xea48a347,// 270 PAY 267 

    0xee5f7557,// 271 PAY 268 

    0x859dc015,// 272 PAY 269 

    0x3c3701e0,// 273 PAY 270 

    0xa75ee144,// 274 PAY 271 

    0xfec1666f,// 275 PAY 272 

    0x8c97e0cb,// 276 PAY 273 

    0x41f179f3,// 277 PAY 274 

    0x31a3f847,// 278 PAY 275 

    0x81ac6f55,// 279 PAY 276 

    0xf66936b5,// 280 PAY 277 

    0xc9f11a37,// 281 PAY 278 

    0xe476ff76,// 282 PAY 279 

    0x5de2fd1d,// 283 PAY 280 

    0x9200cfd4,// 284 PAY 281 

    0x470cdc1a,// 285 PAY 282 

    0x96cd4e84,// 286 PAY 283 

    0x8b000000,// 287 PAY 284 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe5 

    0x01e8e580 // 288 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt29_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 37 words. 

/// BDA size     is 143 (0x8f) 

/// BDA id       is 0x31c5 

    0x008f31c5,// 3 BDA   1 

/// PAY Generic Data size   : 143 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xba159dab,// 4 PAY   1 

    0x3a5810fd,// 5 PAY   2 

    0x2c0db1af,// 6 PAY   3 

    0x086eb5ac,// 7 PAY   4 

    0x9d08338e,// 8 PAY   5 

    0x5229c8a9,// 9 PAY   6 

    0x5715c70b,// 10 PAY   7 

    0xb72e6137,// 11 PAY   8 

    0xa0e5f1d7,// 12 PAY   9 

    0x6e3f3ff3,// 13 PAY  10 

    0x84d1c428,// 14 PAY  11 

    0x2a86fa97,// 15 PAY  12 

    0xa66ec9e4,// 16 PAY  13 

    0xd52fcde2,// 17 PAY  14 

    0xc7d63825,// 18 PAY  15 

    0xaee144d8,// 19 PAY  16 

    0x653ccaaf,// 20 PAY  17 

    0xfb516d22,// 21 PAY  18 

    0xe8089937,// 22 PAY  19 

    0xa6538f25,// 23 PAY  20 

    0x9508e4fb,// 24 PAY  21 

    0x6351bf9e,// 25 PAY  22 

    0x05da0f56,// 26 PAY  23 

    0xa1ea519e,// 27 PAY  24 

    0xcc087906,// 28 PAY  25 

    0xadea3d12,// 29 PAY  26 

    0xbc70ca58,// 30 PAY  27 

    0x886d36bd,// 31 PAY  28 

    0x466cea68,// 32 PAY  29 

    0x42c823ab,// 33 PAY  30 

    0x89901401,// 34 PAY  31 

    0x6f68c5df,// 35 PAY  32 

    0x91b8ae1f,// 36 PAY  33 

    0x23918cb0,// 37 PAY  34 

    0x28b8cbcf,// 38 PAY  35 

    0xaca35900,// 39 PAY  36 

/// HASH is  8 bytes 

    0x89901401,// 40 HSH   1 

    0x6f68c5df,// 41 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 28 

/// STA pkt_idx        : 43 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x00ac6e1c // 42 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt30_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 20 words. 

/// BDA size     is 73 (0x49) 

/// BDA id       is 0x15c 

    0x0049015c,// 3 BDA   1 

/// PAY Generic Data size   : 73 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x577d1c20,// 4 PAY   1 

    0x04325a35,// 5 PAY   2 

    0xd67c2de0,// 6 PAY   3 

    0x67fe963e,// 7 PAY   4 

    0xfeb65a49,// 8 PAY   5 

    0x14434ee2,// 9 PAY   6 

    0x2746f125,// 10 PAY   7 

    0x63be5f21,// 11 PAY   8 

    0xd50e1a83,// 12 PAY   9 

    0x697a7177,// 13 PAY  10 

    0x2c43222d,// 14 PAY  11 

    0xac4e2924,// 15 PAY  12 

    0x5153db27,// 16 PAY  13 

    0x6aefc1c1,// 17 PAY  14 

    0xd163bc79,// 18 PAY  15 

    0xc8974470,// 19 PAY  16 

    0x650e0314,// 20 PAY  17 

    0x81f01952,// 21 PAY  18 

    0xc6000000,// 22 PAY  19 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 105 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x53 

    0x01a55306 // 23 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt31_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 14 words. 

/// BDA size     is 52 (0x34) 

/// BDA id       is 0x2b67 

    0x00342b67,// 3 BDA   1 

/// PAY Generic Data size   : 52 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xa59181ba,// 4 PAY   1 

    0x7d280840,// 5 PAY   2 

    0x7396fef7,// 6 PAY   3 

    0xfaca1bc6,// 7 PAY   4 

    0x80ff6115,// 8 PAY   5 

    0xfb901840,// 9 PAY   6 

    0xd2ceba4a,// 10 PAY   7 

    0xdd45d0a8,// 11 PAY   8 

    0x5f82c4c7,// 12 PAY   9 

    0x00b79393,// 13 PAY  10 

    0x2978ea9a,// 14 PAY  11 

    0xdb5d39e2,// 15 PAY  12 

    0x602e43bc,// 16 PAY  13 

/// HASH is  4 bytes 

    0x5f82c4c7,// 17 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 179 

/// STA pkt_idx        : 154 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb8 

    0x0269b8b3 // 18 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt32_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 118 words. 

/// BDA size     is 468 (0x1d4) 

/// BDA id       is 0x1659 

    0x01d41659,// 3 BDA   1 

/// PAY Generic Data size   : 468 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xe14a0e3a,// 4 PAY   1 

    0xf1804402,// 5 PAY   2 

    0x8efb7816,// 6 PAY   3 

    0x03eb350c,// 7 PAY   4 

    0xeb921450,// 8 PAY   5 

    0x705badd3,// 9 PAY   6 

    0x8244a412,// 10 PAY   7 

    0xa1e0ef84,// 11 PAY   8 

    0xfa8a69bd,// 12 PAY   9 

    0x5af0a664,// 13 PAY  10 

    0x535df48c,// 14 PAY  11 

    0xae14108a,// 15 PAY  12 

    0x8d5a7c77,// 16 PAY  13 

    0x6df535a6,// 17 PAY  14 

    0x10127b92,// 18 PAY  15 

    0xc9fd2ccd,// 19 PAY  16 

    0x1283b40e,// 20 PAY  17 

    0x5c6c8b31,// 21 PAY  18 

    0x4a14922c,// 22 PAY  19 

    0x00f38a16,// 23 PAY  20 

    0xa96fbfed,// 24 PAY  21 

    0x41839512,// 25 PAY  22 

    0x11d0795b,// 26 PAY  23 

    0xb441dbbd,// 27 PAY  24 

    0x3cd1838c,// 28 PAY  25 

    0x4be5055d,// 29 PAY  26 

    0xa33c7f22,// 30 PAY  27 

    0x0c7b9fea,// 31 PAY  28 

    0xb0587a44,// 32 PAY  29 

    0x8178a3dd,// 33 PAY  30 

    0xb1f7cc95,// 34 PAY  31 

    0x3e51e5b8,// 35 PAY  32 

    0x813fe830,// 36 PAY  33 

    0x69ad6c32,// 37 PAY  34 

    0xac74fe65,// 38 PAY  35 

    0x9c8d071f,// 39 PAY  36 

    0xb86ff75b,// 40 PAY  37 

    0xecbad958,// 41 PAY  38 

    0xdd6e40ad,// 42 PAY  39 

    0xa61b1f4b,// 43 PAY  40 

    0xbf262c72,// 44 PAY  41 

    0xd3626f38,// 45 PAY  42 

    0xacc4cce0,// 46 PAY  43 

    0x2ceb22b4,// 47 PAY  44 

    0xc356cae1,// 48 PAY  45 

    0xbfa31031,// 49 PAY  46 

    0x2665accf,// 50 PAY  47 

    0xd1cf4086,// 51 PAY  48 

    0x85231a2d,// 52 PAY  49 

    0xae3dd39d,// 53 PAY  50 

    0xcda84fbd,// 54 PAY  51 

    0xc83a9074,// 55 PAY  52 

    0xa77dc269,// 56 PAY  53 

    0xc51ae4cb,// 57 PAY  54 

    0xed97432e,// 58 PAY  55 

    0xf1ad4dbe,// 59 PAY  56 

    0xcd01c95b,// 60 PAY  57 

    0x15d28c43,// 61 PAY  58 

    0x7606aaad,// 62 PAY  59 

    0x52e28a68,// 63 PAY  60 

    0xfecd51be,// 64 PAY  61 

    0xbe1aee75,// 65 PAY  62 

    0xc0cef3e2,// 66 PAY  63 

    0x64427c46,// 67 PAY  64 

    0xeed09566,// 68 PAY  65 

    0xdb09f656,// 69 PAY  66 

    0x385851d2,// 70 PAY  67 

    0x2f9f618a,// 71 PAY  68 

    0x4ac32376,// 72 PAY  69 

    0x65aded9f,// 73 PAY  70 

    0xbfd3aa84,// 74 PAY  71 

    0xa84f3648,// 75 PAY  72 

    0xfc195c90,// 76 PAY  73 

    0x411d1406,// 77 PAY  74 

    0x526bc264,// 78 PAY  75 

    0x62a4881b,// 79 PAY  76 

    0x50488361,// 80 PAY  77 

    0x3b26800d,// 81 PAY  78 

    0x5718763d,// 82 PAY  79 

    0xce694ed1,// 83 PAY  80 

    0x85b54e61,// 84 PAY  81 

    0xecfcf2a0,// 85 PAY  82 

    0xb9ef5acb,// 86 PAY  83 

    0xaee0b763,// 87 PAY  84 

    0x45bf0bc7,// 88 PAY  85 

    0xd42e9bfe,// 89 PAY  86 

    0x687fbac7,// 90 PAY  87 

    0x8eeae278,// 91 PAY  88 

    0xa498537e,// 92 PAY  89 

    0x1523870e,// 93 PAY  90 

    0x4947ed26,// 94 PAY  91 

    0xe2da4973,// 95 PAY  92 

    0x0cfe45c0,// 96 PAY  93 

    0xff756fda,// 97 PAY  94 

    0x5188b8ce,// 98 PAY  95 

    0x1d16cfbd,// 99 PAY  96 

    0x4c58579e,// 100 PAY  97 

    0x23710478,// 101 PAY  98 

    0x113c41c0,// 102 PAY  99 

    0x8da435ca,// 103 PAY 100 

    0x4c81d469,// 104 PAY 101 

    0x69d43474,// 105 PAY 102 

    0xb4a26982,// 106 PAY 103 

    0xc76e4da8,// 107 PAY 104 

    0x5e776fef,// 108 PAY 105 

    0x04a4958a,// 109 PAY 106 

    0x77163f2e,// 110 PAY 107 

    0x0614efc3,// 111 PAY 108 

    0x1558be96,// 112 PAY 109 

    0xbabb575b,// 113 PAY 110 

    0x007cdd82,// 114 PAY 111 

    0xe548d3b6,// 115 PAY 112 

    0xa2d438e0,// 116 PAY 113 

    0x4b5cc796,// 117 PAY 114 

    0x87cea418,// 118 PAY 115 

    0x4eff9eb5,// 119 PAY 116 

    0xe58791ac,// 120 PAY 117 

/// HASH is  12 bytes 

    0xecbad958,// 121 HSH   1 

    0xdd6e40ad,// 122 HSH   2 

    0xa61b1f4b,// 123 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 75 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x28 

    0x0111284b // 124 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt33_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 201 words. 

/// BDA size     is 799 (0x31f) 

/// BDA id       is 0x8db2 

    0x031f8db2,// 3 BDA   1 

/// PAY Generic Data size   : 799 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x05b81d17,// 4 PAY   1 

    0x3a6c8254,// 5 PAY   2 

    0xf64565c2,// 6 PAY   3 

    0x93c25d7c,// 7 PAY   4 

    0x314b3f43,// 8 PAY   5 

    0x7bea6b0b,// 9 PAY   6 

    0xbcc46941,// 10 PAY   7 

    0x55a75b8d,// 11 PAY   8 

    0xc6537966,// 12 PAY   9 

    0xa7c0863f,// 13 PAY  10 

    0x1b43b86f,// 14 PAY  11 

    0x177c6e9e,// 15 PAY  12 

    0xcc0975d1,// 16 PAY  13 

    0x0e05f001,// 17 PAY  14 

    0x243dce1c,// 18 PAY  15 

    0xc1fc80c4,// 19 PAY  16 

    0x7f173b72,// 20 PAY  17 

    0xf46b1bfe,// 21 PAY  18 

    0x98a33d18,// 22 PAY  19 

    0xc757d2d0,// 23 PAY  20 

    0xa46c03b5,// 24 PAY  21 

    0xd74ab563,// 25 PAY  22 

    0xa52499ab,// 26 PAY  23 

    0xfabac680,// 27 PAY  24 

    0x07cd5628,// 28 PAY  25 

    0x8eb3d4ee,// 29 PAY  26 

    0x175dd802,// 30 PAY  27 

    0x58e374e4,// 31 PAY  28 

    0x62cf8c14,// 32 PAY  29 

    0xb747aa4c,// 33 PAY  30 

    0x822dad3a,// 34 PAY  31 

    0x642f8073,// 35 PAY  32 

    0xd6b8b21c,// 36 PAY  33 

    0x78800bd0,// 37 PAY  34 

    0x8c51e4f9,// 38 PAY  35 

    0x087765c2,// 39 PAY  36 

    0xda99b2b1,// 40 PAY  37 

    0x1ad5cbb4,// 41 PAY  38 

    0xb16c7e95,// 42 PAY  39 

    0xcc4e2c2c,// 43 PAY  40 

    0x60cf8022,// 44 PAY  41 

    0x445a817d,// 45 PAY  42 

    0xbfb0ee22,// 46 PAY  43 

    0x3fcf5385,// 47 PAY  44 

    0x1b00aaa4,// 48 PAY  45 

    0xe66d332e,// 49 PAY  46 

    0xb69f53cb,// 50 PAY  47 

    0x6f881758,// 51 PAY  48 

    0x1c133094,// 52 PAY  49 

    0x74c778e0,// 53 PAY  50 

    0x6b47d71c,// 54 PAY  51 

    0x26b797ce,// 55 PAY  52 

    0x4f32c755,// 56 PAY  53 

    0x69c16eac,// 57 PAY  54 

    0x66ccd825,// 58 PAY  55 

    0xe6cb435f,// 59 PAY  56 

    0x09f622e2,// 60 PAY  57 

    0x3356a0f6,// 61 PAY  58 

    0xcb9db251,// 62 PAY  59 

    0x23f9925e,// 63 PAY  60 

    0xfd03ceb4,// 64 PAY  61 

    0x6072225b,// 65 PAY  62 

    0x915b6568,// 66 PAY  63 

    0xebaca34a,// 67 PAY  64 

    0xe7efae46,// 68 PAY  65 

    0x6ac89109,// 69 PAY  66 

    0x81550a97,// 70 PAY  67 

    0xd28bfb54,// 71 PAY  68 

    0x21b0ba9a,// 72 PAY  69 

    0x985b0092,// 73 PAY  70 

    0xb6f6767d,// 74 PAY  71 

    0x201e8768,// 75 PAY  72 

    0xd94a9c6a,// 76 PAY  73 

    0x482fafe7,// 77 PAY  74 

    0xa04a317f,// 78 PAY  75 

    0x60d6ac7e,// 79 PAY  76 

    0x58a854f4,// 80 PAY  77 

    0x7e692755,// 81 PAY  78 

    0x850a40f9,// 82 PAY  79 

    0x11e6b27c,// 83 PAY  80 

    0xeda1f283,// 84 PAY  81 

    0xdf8bade0,// 85 PAY  82 

    0x9eaeac14,// 86 PAY  83 

    0xfcc4b9db,// 87 PAY  84 

    0x5d68ed45,// 88 PAY  85 

    0x7270a528,// 89 PAY  86 

    0x9543ccf0,// 90 PAY  87 

    0xd68512f8,// 91 PAY  88 

    0xd70f96cc,// 92 PAY  89 

    0x0f7d534c,// 93 PAY  90 

    0x42c4e068,// 94 PAY  91 

    0x709158a7,// 95 PAY  92 

    0xfaaa6fe3,// 96 PAY  93 

    0xe116826c,// 97 PAY  94 

    0xe6f1225f,// 98 PAY  95 

    0xe448df2e,// 99 PAY  96 

    0xd558f01c,// 100 PAY  97 

    0x582ac99b,// 101 PAY  98 

    0x26337d7c,// 102 PAY  99 

    0xa6877ceb,// 103 PAY 100 

    0x519d8aa9,// 104 PAY 101 

    0x8a6dcd44,// 105 PAY 102 

    0x5468792b,// 106 PAY 103 

    0x85359562,// 107 PAY 104 

    0x363c2d66,// 108 PAY 105 

    0x2997dc5d,// 109 PAY 106 

    0x04c0bbd1,// 110 PAY 107 

    0x5e1838ac,// 111 PAY 108 

    0x2c6067c8,// 112 PAY 109 

    0x60719a94,// 113 PAY 110 

    0x436586e1,// 114 PAY 111 

    0x1eab69a1,// 115 PAY 112 

    0x736c836f,// 116 PAY 113 

    0x543f8efe,// 117 PAY 114 

    0x3b06b727,// 118 PAY 115 

    0x53ccc91e,// 119 PAY 116 

    0xd6260bf3,// 120 PAY 117 

    0xa059ddbc,// 121 PAY 118 

    0x51a40531,// 122 PAY 119 

    0x6affc22d,// 123 PAY 120 

    0x777d4920,// 124 PAY 121 

    0x219c218e,// 125 PAY 122 

    0x48a16eb8,// 126 PAY 123 

    0xa1f73963,// 127 PAY 124 

    0xcfd4802a,// 128 PAY 125 

    0xfdb21b52,// 129 PAY 126 

    0x770be155,// 130 PAY 127 

    0xfe5403f7,// 131 PAY 128 

    0x90fda9c7,// 132 PAY 129 

    0x4d3b2d5c,// 133 PAY 130 

    0x6f829a64,// 134 PAY 131 

    0xeb4c611d,// 135 PAY 132 

    0x0f38b57a,// 136 PAY 133 

    0xb0523cd4,// 137 PAY 134 

    0x14353042,// 138 PAY 135 

    0xf8899a78,// 139 PAY 136 

    0xb56e89b1,// 140 PAY 137 

    0x39b0c170,// 141 PAY 138 

    0xf1eb9099,// 142 PAY 139 

    0x56d31b20,// 143 PAY 140 

    0xe48e35bb,// 144 PAY 141 

    0x65d4962f,// 145 PAY 142 

    0x855c4430,// 146 PAY 143 

    0xf9e95764,// 147 PAY 144 

    0xc703199d,// 148 PAY 145 

    0x30989479,// 149 PAY 146 

    0x3fcd1747,// 150 PAY 147 

    0x9b78579e,// 151 PAY 148 

    0x6454e44b,// 152 PAY 149 

    0xd08c98ee,// 153 PAY 150 

    0x3d5558c5,// 154 PAY 151 

    0xe7058a59,// 155 PAY 152 

    0xd8425e31,// 156 PAY 153 

    0x4ef0f5d8,// 157 PAY 154 

    0x655f737a,// 158 PAY 155 

    0x91b4e611,// 159 PAY 156 

    0x58b5f69f,// 160 PAY 157 

    0x4ad3f305,// 161 PAY 158 

    0x92ad4725,// 162 PAY 159 

    0x40efeebc,// 163 PAY 160 

    0xb17a7770,// 164 PAY 161 

    0xfcc65109,// 165 PAY 162 

    0x46df42b8,// 166 PAY 163 

    0x28a4d5a9,// 167 PAY 164 

    0x4cc6b6b5,// 168 PAY 165 

    0xf2eaaaee,// 169 PAY 166 

    0xa47f9818,// 170 PAY 167 

    0x64447c6c,// 171 PAY 168 

    0x52202afa,// 172 PAY 169 

    0x9774f130,// 173 PAY 170 

    0x6c6a6e1d,// 174 PAY 171 

    0x5c23c61b,// 175 PAY 172 

    0x979186f6,// 176 PAY 173 

    0xaf1ef0fd,// 177 PAY 174 

    0x839aa4af,// 178 PAY 175 

    0x8c03b0d6,// 179 PAY 176 

    0x6e3f7459,// 180 PAY 177 

    0xd96b208c,// 181 PAY 178 

    0x815daeb6,// 182 PAY 179 

    0x52bd56ac,// 183 PAY 180 

    0xe4fb2caf,// 184 PAY 181 

    0x92bdab92,// 185 PAY 182 

    0x949cd8ff,// 186 PAY 183 

    0xe5b3d6c8,// 187 PAY 184 

    0x1fdba3a7,// 188 PAY 185 

    0x2fef1630,// 189 PAY 186 

    0x1a2011df,// 190 PAY 187 

    0x15785472,// 191 PAY 188 

    0xf7c331c2,// 192 PAY 189 

    0x5362b00f,// 193 PAY 190 

    0x16158739,// 194 PAY 191 

    0x082e73bd,// 195 PAY 192 

    0x04c0f265,// 196 PAY 193 

    0xa829ef6e,// 197 PAY 194 

    0xaa9a5131,// 198 PAY 195 

    0x859a706e,// 199 PAY 196 

    0xaf6b8773,// 200 PAY 197 

    0xab0652ce,// 201 PAY 198 

    0x0eb0ba5b,// 202 PAY 199 

    0x3f81fb00,// 203 PAY 200 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 141 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb2 

    0x0234b237 // 204 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt34_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 193 words. 

/// BDA size     is 767 (0x2ff) 

/// BDA id       is 0xaed0 

    0x02ffaed0,// 3 BDA   1 

/// PAY Generic Data size   : 767 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x3d3c77be,// 4 PAY   1 

    0x206c8a93,// 5 PAY   2 

    0x92fe18bd,// 6 PAY   3 

    0xd66538b3,// 7 PAY   4 

    0xa50421ad,// 8 PAY   5 

    0xb3421882,// 9 PAY   6 

    0x947629a2,// 10 PAY   7 

    0xa58b039f,// 11 PAY   8 

    0x86a174ff,// 12 PAY   9 

    0x7e249d5b,// 13 PAY  10 

    0xca50768a,// 14 PAY  11 

    0xcc21cee2,// 15 PAY  12 

    0x46c6923c,// 16 PAY  13 

    0xb04808b2,// 17 PAY  14 

    0x7b474ed1,// 18 PAY  15 

    0x0e2c712b,// 19 PAY  16 

    0x963a6fbe,// 20 PAY  17 

    0xb985e166,// 21 PAY  18 

    0x7b76ca95,// 22 PAY  19 

    0x2a090c88,// 23 PAY  20 

    0xb44bcc6a,// 24 PAY  21 

    0x7055710b,// 25 PAY  22 

    0x2b27dfca,// 26 PAY  23 

    0xcfad69da,// 27 PAY  24 

    0x9fb24514,// 28 PAY  25 

    0xfa8cd839,// 29 PAY  26 

    0x9273420f,// 30 PAY  27 

    0x05486779,// 31 PAY  28 

    0xb9e1194c,// 32 PAY  29 

    0xd66fd611,// 33 PAY  30 

    0x839461e4,// 34 PAY  31 

    0x80b3c42f,// 35 PAY  32 

    0x7927239f,// 36 PAY  33 

    0x478d4d24,// 37 PAY  34 

    0x770dffeb,// 38 PAY  35 

    0x37641bca,// 39 PAY  36 

    0x0326e909,// 40 PAY  37 

    0x6113ca04,// 41 PAY  38 

    0x6aa31716,// 42 PAY  39 

    0x7149869e,// 43 PAY  40 

    0xb79df335,// 44 PAY  41 

    0x1a163a4f,// 45 PAY  42 

    0xca6f1676,// 46 PAY  43 

    0x381202f5,// 47 PAY  44 

    0x9a171248,// 48 PAY  45 

    0x184c417c,// 49 PAY  46 

    0x8736a021,// 50 PAY  47 

    0x22c969e4,// 51 PAY  48 

    0x6132fda9,// 52 PAY  49 

    0xf7b8b4ff,// 53 PAY  50 

    0xfb62025b,// 54 PAY  51 

    0x81096d5e,// 55 PAY  52 

    0x9aae7590,// 56 PAY  53 

    0x4cfefb4a,// 57 PAY  54 

    0x85e84fd9,// 58 PAY  55 

    0x0c31af7d,// 59 PAY  56 

    0x59eca01a,// 60 PAY  57 

    0xf9bb89a4,// 61 PAY  58 

    0x901c7359,// 62 PAY  59 

    0x838242e9,// 63 PAY  60 

    0x84f97f79,// 64 PAY  61 

    0xc39adb5c,// 65 PAY  62 

    0xe6f84622,// 66 PAY  63 

    0x29b824a3,// 67 PAY  64 

    0x4dab448b,// 68 PAY  65 

    0xdd32e640,// 69 PAY  66 

    0x83171a8c,// 70 PAY  67 

    0xe5424639,// 71 PAY  68 

    0xfc407749,// 72 PAY  69 

    0x57fd9554,// 73 PAY  70 

    0x960afe9c,// 74 PAY  71 

    0x200f7a00,// 75 PAY  72 

    0xf048681c,// 76 PAY  73 

    0x0ee7c8a8,// 77 PAY  74 

    0x3efb9ded,// 78 PAY  75 

    0x8322d98b,// 79 PAY  76 

    0x60136528,// 80 PAY  77 

    0x2e85d995,// 81 PAY  78 

    0x89278a29,// 82 PAY  79 

    0x7701eccc,// 83 PAY  80 

    0x25c0e4bc,// 84 PAY  81 

    0x7ab40d82,// 85 PAY  82 

    0x73a98ea5,// 86 PAY  83 

    0x5f6ee49f,// 87 PAY  84 

    0x9d35636f,// 88 PAY  85 

    0xc4c70211,// 89 PAY  86 

    0xeb9d7bfe,// 90 PAY  87 

    0x7b8d90e5,// 91 PAY  88 

    0xba815f1e,// 92 PAY  89 

    0x49063319,// 93 PAY  90 

    0xb7b733d6,// 94 PAY  91 

    0xc433488d,// 95 PAY  92 

    0xa1a1a917,// 96 PAY  93 

    0xd55d711c,// 97 PAY  94 

    0xf9753c4a,// 98 PAY  95 

    0xb2bba2b9,// 99 PAY  96 

    0xaecef008,// 100 PAY  97 

    0x9c6abefe,// 101 PAY  98 

    0x07dfe13b,// 102 PAY  99 

    0x9eb742fb,// 103 PAY 100 

    0x9ef00b6b,// 104 PAY 101 

    0x31548a7e,// 105 PAY 102 

    0x576bf319,// 106 PAY 103 

    0x0728bb9a,// 107 PAY 104 

    0xb4f73c8a,// 108 PAY 105 

    0x3a4aae32,// 109 PAY 106 

    0x519a855e,// 110 PAY 107 

    0xc8d2d16e,// 111 PAY 108 

    0x3cc4f99f,// 112 PAY 109 

    0xc1c1b4bf,// 113 PAY 110 

    0x1e696446,// 114 PAY 111 

    0xe629f5e3,// 115 PAY 112 

    0x18d596aa,// 116 PAY 113 

    0xfc62b3b7,// 117 PAY 114 

    0xea6b4410,// 118 PAY 115 

    0x7d8ea53e,// 119 PAY 116 

    0xc3e8867a,// 120 PAY 117 

    0x81af1376,// 121 PAY 118 

    0xb195ea39,// 122 PAY 119 

    0xcdc935da,// 123 PAY 120 

    0x6b614404,// 124 PAY 121 

    0x65f66a49,// 125 PAY 122 

    0x54bbd24a,// 126 PAY 123 

    0x691c43c1,// 127 PAY 124 

    0xa65885e0,// 128 PAY 125 

    0xffa591fc,// 129 PAY 126 

    0x96f0e2ef,// 130 PAY 127 

    0x1a29f074,// 131 PAY 128 

    0x3ab6eefd,// 132 PAY 129 

    0x84f9a7b7,// 133 PAY 130 

    0xcd35e40b,// 134 PAY 131 

    0x0e91a600,// 135 PAY 132 

    0xac369f3b,// 136 PAY 133 

    0x28f1cf3b,// 137 PAY 134 

    0x47c8a888,// 138 PAY 135 

    0x9110e2cb,// 139 PAY 136 

    0xda37abf5,// 140 PAY 137 

    0xb1f0d06b,// 141 PAY 138 

    0xadef2028,// 142 PAY 139 

    0x86f89aa5,// 143 PAY 140 

    0x221eccae,// 144 PAY 141 

    0xd934998c,// 145 PAY 142 

    0x380e4b07,// 146 PAY 143 

    0x6e5b06f4,// 147 PAY 144 

    0xcba67db8,// 148 PAY 145 

    0x4cd1cca1,// 149 PAY 146 

    0x18ca567a,// 150 PAY 147 

    0x5ac66860,// 151 PAY 148 

    0x87176eb5,// 152 PAY 149 

    0xd52a7824,// 153 PAY 150 

    0xfb111683,// 154 PAY 151 

    0xedeb4af8,// 155 PAY 152 

    0x5eaa1644,// 156 PAY 153 

    0x0224e4ef,// 157 PAY 154 

    0x605a7d61,// 158 PAY 155 

    0xdfe84dc0,// 159 PAY 156 

    0xda5f8374,// 160 PAY 157 

    0x93ef05b2,// 161 PAY 158 

    0xe5da4f0a,// 162 PAY 159 

    0x3f8904d0,// 163 PAY 160 

    0x5b1459c9,// 164 PAY 161 

    0xe90c381d,// 165 PAY 162 

    0x21253887,// 166 PAY 163 

    0x83521b44,// 167 PAY 164 

    0x83ba30c3,// 168 PAY 165 

    0x96ae08b5,// 169 PAY 166 

    0xf2ce9e74,// 170 PAY 167 

    0xf602cf3c,// 171 PAY 168 

    0xbc87a4aa,// 172 PAY 169 

    0xa0e4f1f7,// 173 PAY 170 

    0xaa48762c,// 174 PAY 171 

    0xb7b3900a,// 175 PAY 172 

    0x839854d1,// 176 PAY 173 

    0xbf9a27d2,// 177 PAY 174 

    0x4009b783,// 178 PAY 175 

    0xc90f14b0,// 179 PAY 176 

    0x4d7408b7,// 180 PAY 177 

    0x420eead1,// 181 PAY 178 

    0x6900c8dd,// 182 PAY 179 

    0x7b3d59bd,// 183 PAY 180 

    0x9ee15267,// 184 PAY 181 

    0xdc172585,// 185 PAY 182 

    0xc7613b08,// 186 PAY 183 

    0x61c31ca0,// 187 PAY 184 

    0x0e0a8a9f,// 188 PAY 185 

    0xd7172dc6,// 189 PAY 186 

    0x6d821f91,// 190 PAY 187 

    0xd75c744b,// 191 PAY 188 

    0x5a0369b3,// 192 PAY 189 

    0x15349a13,// 193 PAY 190 

    0x613006ed,// 194 PAY 191 

    0xce29c200,// 195 PAY 192 

/// HASH is  12 bytes 

    0x6d821f91,// 196 HSH   1 

    0xd75c744b,// 197 HSH   2 

    0x5a0369b3,// 198 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 84 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3d 

    0x02b53d54 // 199 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt35_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 273 words. 

/// BDA size     is 1085 (0x43d) 

/// BDA id       is 0x41dc 

    0x043d41dc,// 3 BDA   1 

/// PAY Generic Data size   : 1085 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xf2c8a447,// 4 PAY   1 

    0xf6f06e9b,// 5 PAY   2 

    0xa26b1531,// 6 PAY   3 

    0x50537162,// 7 PAY   4 

    0x9ea65629,// 8 PAY   5 

    0xa8844f83,// 9 PAY   6 

    0x2aebe292,// 10 PAY   7 

    0xd3ee7c39,// 11 PAY   8 

    0x6b94f80d,// 12 PAY   9 

    0xbedfe334,// 13 PAY  10 

    0x278c36c8,// 14 PAY  11 

    0x963041b3,// 15 PAY  12 

    0x5e929bf7,// 16 PAY  13 

    0x9561fd77,// 17 PAY  14 

    0xcb388331,// 18 PAY  15 

    0x80e66b5a,// 19 PAY  16 

    0x70957cc3,// 20 PAY  17 

    0xd59a704b,// 21 PAY  18 

    0xc52b25cc,// 22 PAY  19 

    0x98b85508,// 23 PAY  20 

    0xfd00c1ec,// 24 PAY  21 

    0xe2820b1e,// 25 PAY  22 

    0x58920b9a,// 26 PAY  23 

    0x555ec779,// 27 PAY  24 

    0xddbcf506,// 28 PAY  25 

    0x12f849e0,// 29 PAY  26 

    0xc528711d,// 30 PAY  27 

    0x2c13cf87,// 31 PAY  28 

    0xd165b0d0,// 32 PAY  29 

    0x7b88587b,// 33 PAY  30 

    0xd519e896,// 34 PAY  31 

    0xa4638ea7,// 35 PAY  32 

    0x3e523fd6,// 36 PAY  33 

    0xa25693c0,// 37 PAY  34 

    0x5e97527c,// 38 PAY  35 

    0x78e2aa1b,// 39 PAY  36 

    0xcab75821,// 40 PAY  37 

    0x7e07a93d,// 41 PAY  38 

    0xd2ee6fbf,// 42 PAY  39 

    0xf81f72e0,// 43 PAY  40 

    0xa88e752f,// 44 PAY  41 

    0x25de4a22,// 45 PAY  42 

    0xb3a38224,// 46 PAY  43 

    0x718599a7,// 47 PAY  44 

    0x770fafd1,// 48 PAY  45 

    0x15d3cba9,// 49 PAY  46 

    0x04be3598,// 50 PAY  47 

    0x55ed6c06,// 51 PAY  48 

    0x61a6a77b,// 52 PAY  49 

    0xd4dce8a2,// 53 PAY  50 

    0x0169d013,// 54 PAY  51 

    0x309a3039,// 55 PAY  52 

    0x13f62cf5,// 56 PAY  53 

    0x6b694ef1,// 57 PAY  54 

    0x2e94b632,// 58 PAY  55 

    0xb09a2e78,// 59 PAY  56 

    0xc6e5243f,// 60 PAY  57 

    0x581f9cec,// 61 PAY  58 

    0x72e28cf8,// 62 PAY  59 

    0xbc3531e3,// 63 PAY  60 

    0xa3841a06,// 64 PAY  61 

    0xce355f7a,// 65 PAY  62 

    0x04585036,// 66 PAY  63 

    0x69433980,// 67 PAY  64 

    0x0f6f2ff7,// 68 PAY  65 

    0x98ff57cd,// 69 PAY  66 

    0x1a435129,// 70 PAY  67 

    0x7efff22e,// 71 PAY  68 

    0xf60640c9,// 72 PAY  69 

    0x1860c115,// 73 PAY  70 

    0x65c894a7,// 74 PAY  71 

    0xbd0feb8a,// 75 PAY  72 

    0x7684d49e,// 76 PAY  73 

    0x0e127fe6,// 77 PAY  74 

    0x1a7230b1,// 78 PAY  75 

    0x8fb6deda,// 79 PAY  76 

    0x937df72b,// 80 PAY  77 

    0x6f6b90a3,// 81 PAY  78 

    0xbad6632d,// 82 PAY  79 

    0x4751b736,// 83 PAY  80 

    0x0c1b7b87,// 84 PAY  81 

    0xfdce8b08,// 85 PAY  82 

    0x0157dafd,// 86 PAY  83 

    0x24cc714f,// 87 PAY  84 

    0xc96d64cd,// 88 PAY  85 

    0x8ae9eb60,// 89 PAY  86 

    0x7a178864,// 90 PAY  87 

    0xca2411ac,// 91 PAY  88 

    0x8ae76eb7,// 92 PAY  89 

    0x8c5f4653,// 93 PAY  90 

    0xdecba7b8,// 94 PAY  91 

    0xdb400265,// 95 PAY  92 

    0x961ea054,// 96 PAY  93 

    0xb1ab37a8,// 97 PAY  94 

    0x07f2565d,// 98 PAY  95 

    0xe49e6b04,// 99 PAY  96 

    0xf78c51ba,// 100 PAY  97 

    0x18ef07ae,// 101 PAY  98 

    0x36931b1b,// 102 PAY  99 

    0x47780ad6,// 103 PAY 100 

    0x1f3ca8e8,// 104 PAY 101 

    0x1ed896b1,// 105 PAY 102 

    0x0931f2e7,// 106 PAY 103 

    0xd3f1d907,// 107 PAY 104 

    0xb381a2ae,// 108 PAY 105 

    0x427716c7,// 109 PAY 106 

    0x4bceddc4,// 110 PAY 107 

    0x9bfdb80a,// 111 PAY 108 

    0x10689d23,// 112 PAY 109 

    0x144e3b02,// 113 PAY 110 

    0x66108c8f,// 114 PAY 111 

    0x85e58975,// 115 PAY 112 

    0x83d7ead5,// 116 PAY 113 

    0x8a2f73cf,// 117 PAY 114 

    0x1ba7ef5d,// 118 PAY 115 

    0x496f7266,// 119 PAY 116 

    0x2c558ad1,// 120 PAY 117 

    0xec00d638,// 121 PAY 118 

    0x2e5b410d,// 122 PAY 119 

    0x829ddd1a,// 123 PAY 120 

    0x4d4a8f62,// 124 PAY 121 

    0xdcf98780,// 125 PAY 122 

    0x1c29789f,// 126 PAY 123 

    0xcd0e9f6a,// 127 PAY 124 

    0x7a4c7717,// 128 PAY 125 

    0xcb6f7e7f,// 129 PAY 126 

    0xf3888f4f,// 130 PAY 127 

    0x554faecb,// 131 PAY 128 

    0x128705d0,// 132 PAY 129 

    0x13ba3ade,// 133 PAY 130 

    0xda232b3e,// 134 PAY 131 

    0xd1d6f636,// 135 PAY 132 

    0x14b433cb,// 136 PAY 133 

    0x1d9a9841,// 137 PAY 134 

    0x331ef720,// 138 PAY 135 

    0x4b3819bb,// 139 PAY 136 

    0x953dfb18,// 140 PAY 137 

    0x9d1aa90e,// 141 PAY 138 

    0xa7338743,// 142 PAY 139 

    0xc6a7fa6f,// 143 PAY 140 

    0x1f7e0aeb,// 144 PAY 141 

    0x651f4ac5,// 145 PAY 142 

    0x6b0d3f93,// 146 PAY 143 

    0xf790f411,// 147 PAY 144 

    0x1a45c3d4,// 148 PAY 145 

    0xed745e3e,// 149 PAY 146 

    0x694ed9be,// 150 PAY 147 

    0xae92141b,// 151 PAY 148 

    0xa4d4c325,// 152 PAY 149 

    0x77b561e3,// 153 PAY 150 

    0xc371b1e6,// 154 PAY 151 

    0x316af308,// 155 PAY 152 

    0x6c08a8a0,// 156 PAY 153 

    0x73bbd37a,// 157 PAY 154 

    0x0c477a45,// 158 PAY 155 

    0xdd784d0d,// 159 PAY 156 

    0xba7e5a55,// 160 PAY 157 

    0x85dea188,// 161 PAY 158 

    0x8a1f5ef7,// 162 PAY 159 

    0x99c25d80,// 163 PAY 160 

    0x324c4890,// 164 PAY 161 

    0xb6433015,// 165 PAY 162 

    0xcfa00e15,// 166 PAY 163 

    0x14eb1953,// 167 PAY 164 

    0xa42c4d76,// 168 PAY 165 

    0x4ce2a4d4,// 169 PAY 166 

    0x9b4cd1ae,// 170 PAY 167 

    0xad2c6dc0,// 171 PAY 168 

    0x81614f17,// 172 PAY 169 

    0x558ee608,// 173 PAY 170 

    0xf229a065,// 174 PAY 171 

    0x3c93a8f4,// 175 PAY 172 

    0x63caea84,// 176 PAY 173 

    0xe6a60939,// 177 PAY 174 

    0x724439f1,// 178 PAY 175 

    0x46d94ba7,// 179 PAY 176 

    0x3e125262,// 180 PAY 177 

    0x171bad54,// 181 PAY 178 

    0x56601712,// 182 PAY 179 

    0x4f7f1327,// 183 PAY 180 

    0xe08f9896,// 184 PAY 181 

    0xd9deaa61,// 185 PAY 182 

    0x9333b225,// 186 PAY 183 

    0x4f6bacd1,// 187 PAY 184 

    0xb34a2bbd,// 188 PAY 185 

    0xf5bf880d,// 189 PAY 186 

    0x8181d765,// 190 PAY 187 

    0xecb3b7de,// 191 PAY 188 

    0xb8499907,// 192 PAY 189 

    0xc96e28b6,// 193 PAY 190 

    0x219b31b0,// 194 PAY 191 

    0x2d596cb5,// 195 PAY 192 

    0xe825ca38,// 196 PAY 193 

    0x8bba4790,// 197 PAY 194 

    0x2beacfe7,// 198 PAY 195 

    0x27cc33ea,// 199 PAY 196 

    0x7b8f71d4,// 200 PAY 197 

    0xac42e979,// 201 PAY 198 

    0x85dc5db8,// 202 PAY 199 

    0xeac91e7d,// 203 PAY 200 

    0x0d54159c,// 204 PAY 201 

    0x120cb784,// 205 PAY 202 

    0xb85d2122,// 206 PAY 203 

    0xf3fa4659,// 207 PAY 204 

    0xb4e3473d,// 208 PAY 205 

    0x6f4ffb20,// 209 PAY 206 

    0xf06b0e4d,// 210 PAY 207 

    0x991ca390,// 211 PAY 208 

    0xa79a97f3,// 212 PAY 209 

    0xea2a36db,// 213 PAY 210 

    0x1c60908c,// 214 PAY 211 

    0x8b25f871,// 215 PAY 212 

    0x66e13c7c,// 216 PAY 213 

    0x7e06c6f6,// 217 PAY 214 

    0x72f10e16,// 218 PAY 215 

    0x5bc8eef4,// 219 PAY 216 

    0x7629a7f7,// 220 PAY 217 

    0x7865c839,// 221 PAY 218 

    0x4029f20f,// 222 PAY 219 

    0x29a4666d,// 223 PAY 220 

    0x2f60c4e4,// 224 PAY 221 

    0x26fd0bca,// 225 PAY 222 

    0x750a5e3c,// 226 PAY 223 

    0xc959bd49,// 227 PAY 224 

    0xdec68cba,// 228 PAY 225 

    0x13848c41,// 229 PAY 226 

    0xb726fa04,// 230 PAY 227 

    0x0929aea0,// 231 PAY 228 

    0x8deb2bc0,// 232 PAY 229 

    0x7e93a834,// 233 PAY 230 

    0x50f2dd78,// 234 PAY 231 

    0x8017f19f,// 235 PAY 232 

    0xee7539ec,// 236 PAY 233 

    0xbd91d663,// 237 PAY 234 

    0xbe04b1a6,// 238 PAY 235 

    0xe3f01bb2,// 239 PAY 236 

    0x2f5e1a6b,// 240 PAY 237 

    0xa1e27727,// 241 PAY 238 

    0x79549205,// 242 PAY 239 

    0x6cc0093a,// 243 PAY 240 

    0xbb083943,// 244 PAY 241 

    0xa1bf595a,// 245 PAY 242 

    0xed7b3a9f,// 246 PAY 243 

    0x732d7f59,// 247 PAY 244 

    0x7259a81c,// 248 PAY 245 

    0xadddfebd,// 249 PAY 246 

    0x3e954a97,// 250 PAY 247 

    0x54769ac1,// 251 PAY 248 

    0xa5c3777f,// 252 PAY 249 

    0x2bc7cd36,// 253 PAY 250 

    0x2dcb4226,// 254 PAY 251 

    0x963ff68d,// 255 PAY 252 

    0xfe924176,// 256 PAY 253 

    0x347f45db,// 257 PAY 254 

    0x0232b214,// 258 PAY 255 

    0xf9d9b90c,// 259 PAY 256 

    0xa3f82095,// 260 PAY 257 

    0xd0eb2cb3,// 261 PAY 258 

    0x3cccfa16,// 262 PAY 259 

    0x206c1497,// 263 PAY 260 

    0x14854a40,// 264 PAY 261 

    0xa0e76c85,// 265 PAY 262 

    0xf56532f0,// 266 PAY 263 

    0x84bc47bb,// 267 PAY 264 

    0xc99b2f9e,// 268 PAY 265 

    0x5c5e5c4d,// 269 PAY 266 

    0x9df5151a,// 270 PAY 267 

    0x2e3eaa04,// 271 PAY 268 

    0x0a00c275,// 272 PAY 269 

    0x00ed4b48,// 273 PAY 270 

    0xd4548018,// 274 PAY 271 

    0x39000000,// 275 PAY 272 

/// HASH is  12 bytes 

    0x14854a40,// 276 HSH   1 

    0xa0e76c85,// 277 HSH   2 

    0xf56532f0,// 278 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 7 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xfa 

    0x001cfad6 // 279 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt36_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 504 words. 

/// BDA size     is 2011 (0x7db) 

/// BDA id       is 0xda48 

    0x07dbda48,// 3 BDA   1 

/// PAY Generic Data size   : 2011 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x96c6c923,// 4 PAY   1 

    0x242fc1bc,// 5 PAY   2 

    0xd72c3874,// 6 PAY   3 

    0x5d9e80fd,// 7 PAY   4 

    0xc6488417,// 8 PAY   5 

    0xdf21fc07,// 9 PAY   6 

    0x279ed917,// 10 PAY   7 

    0x13e878e6,// 11 PAY   8 

    0xcacdc0fa,// 12 PAY   9 

    0xebbefbc5,// 13 PAY  10 

    0xb8e08913,// 14 PAY  11 

    0x6ae3434d,// 15 PAY  12 

    0xd055ea99,// 16 PAY  13 

    0xb5d3230a,// 17 PAY  14 

    0xd611ea53,// 18 PAY  15 

    0x093ed743,// 19 PAY  16 

    0xd41734c8,// 20 PAY  17 

    0xa5f6fec5,// 21 PAY  18 

    0x31d12bd1,// 22 PAY  19 

    0xc66cef9a,// 23 PAY  20 

    0x1bd474c3,// 24 PAY  21 

    0xe167ce65,// 25 PAY  22 

    0x65d131f7,// 26 PAY  23 

    0xbc01a531,// 27 PAY  24 

    0x187c6667,// 28 PAY  25 

    0xff70ae48,// 29 PAY  26 

    0x0537629d,// 30 PAY  27 

    0x1f1b4b11,// 31 PAY  28 

    0x648bd3da,// 32 PAY  29 

    0xec3509ef,// 33 PAY  30 

    0xfa229ae0,// 34 PAY  31 

    0xf85e029a,// 35 PAY  32 

    0x873a80be,// 36 PAY  33 

    0x1ad08ba2,// 37 PAY  34 

    0x48e14c62,// 38 PAY  35 

    0x929a2510,// 39 PAY  36 

    0xa4ad31b7,// 40 PAY  37 

    0xb1e57d65,// 41 PAY  38 

    0xe72f4bcd,// 42 PAY  39 

    0x178e6f48,// 43 PAY  40 

    0x66ed535c,// 44 PAY  41 

    0x4dc581db,// 45 PAY  42 

    0xbcd7138d,// 46 PAY  43 

    0xb0ca34e8,// 47 PAY  44 

    0x183f1c1b,// 48 PAY  45 

    0x9a4c3d5f,// 49 PAY  46 

    0x9f617a3e,// 50 PAY  47 

    0x58f76a9b,// 51 PAY  48 

    0x41120e7d,// 52 PAY  49 

    0xd90482bd,// 53 PAY  50 

    0x861bf6eb,// 54 PAY  51 

    0xa995a7eb,// 55 PAY  52 

    0x7828708c,// 56 PAY  53 

    0xc61ff245,// 57 PAY  54 

    0x1b823e4f,// 58 PAY  55 

    0xca352e5e,// 59 PAY  56 

    0x70d48bb3,// 60 PAY  57 

    0x655ddb35,// 61 PAY  58 

    0x28bf4ef9,// 62 PAY  59 

    0xca4e980e,// 63 PAY  60 

    0x9360472d,// 64 PAY  61 

    0x7135f45f,// 65 PAY  62 

    0x67073b7a,// 66 PAY  63 

    0xd946de30,// 67 PAY  64 

    0x75fc3963,// 68 PAY  65 

    0x6fd6f8c2,// 69 PAY  66 

    0x95581e69,// 70 PAY  67 

    0x0ba5cc48,// 71 PAY  68 

    0x4d993608,// 72 PAY  69 

    0xb1bad365,// 73 PAY  70 

    0x3680c4fb,// 74 PAY  71 

    0x00d800ed,// 75 PAY  72 

    0x547b9d9a,// 76 PAY  73 

    0x6e6ffe96,// 77 PAY  74 

    0xde8fc168,// 78 PAY  75 

    0xcb307847,// 79 PAY  76 

    0xd2d94a95,// 80 PAY  77 

    0xfa56f5d6,// 81 PAY  78 

    0x36a19fa6,// 82 PAY  79 

    0xb8b32e72,// 83 PAY  80 

    0xb2e884c4,// 84 PAY  81 

    0x2893fc98,// 85 PAY  82 

    0x90dfa9bd,// 86 PAY  83 

    0xc6d2111f,// 87 PAY  84 

    0x32ac05de,// 88 PAY  85 

    0xa20b1dab,// 89 PAY  86 

    0x9347234e,// 90 PAY  87 

    0xadda82f5,// 91 PAY  88 

    0x9337ded1,// 92 PAY  89 

    0xc1927c13,// 93 PAY  90 

    0x38d7a3f0,// 94 PAY  91 

    0xc605437b,// 95 PAY  92 

    0x78b6c58b,// 96 PAY  93 

    0x7f860ded,// 97 PAY  94 

    0x6940d2b1,// 98 PAY  95 

    0x31508b08,// 99 PAY  96 

    0xe36f18ea,// 100 PAY  97 

    0x478520bc,// 101 PAY  98 

    0x6b1b68f4,// 102 PAY  99 

    0xb9c6a331,// 103 PAY 100 

    0xd34cae72,// 104 PAY 101 

    0xb2f31dee,// 105 PAY 102 

    0xde18beda,// 106 PAY 103 

    0xbf5d63f4,// 107 PAY 104 

    0xd8a26cea,// 108 PAY 105 

    0x224630bd,// 109 PAY 106 

    0xb9bc71c1,// 110 PAY 107 

    0x8b284107,// 111 PAY 108 

    0x90713570,// 112 PAY 109 

    0x969afc99,// 113 PAY 110 

    0x1dca7df7,// 114 PAY 111 

    0xc5204b46,// 115 PAY 112 

    0x285632af,// 116 PAY 113 

    0x1c8fbcf6,// 117 PAY 114 

    0xe36aea93,// 118 PAY 115 

    0x3b6b8eb6,// 119 PAY 116 

    0x27958877,// 120 PAY 117 

    0xb7ca9c97,// 121 PAY 118 

    0x630cc3e8,// 122 PAY 119 

    0xd467bb4a,// 123 PAY 120 

    0xd639a5f0,// 124 PAY 121 

    0xa5f2f909,// 125 PAY 122 

    0xf7d15f70,// 126 PAY 123 

    0x727d44f0,// 127 PAY 124 

    0xfb2b9e55,// 128 PAY 125 

    0xbb9b2d45,// 129 PAY 126 

    0x5ae8008b,// 130 PAY 127 

    0x0785d2b3,// 131 PAY 128 

    0x16452d8f,// 132 PAY 129 

    0xd8e67bb9,// 133 PAY 130 

    0x37314b27,// 134 PAY 131 

    0x96e1334a,// 135 PAY 132 

    0x45699698,// 136 PAY 133 

    0x9b8cbdf8,// 137 PAY 134 

    0xc63418c4,// 138 PAY 135 

    0xd92b1331,// 139 PAY 136 

    0x3cccb362,// 140 PAY 137 

    0x76688afa,// 141 PAY 138 

    0xa7eca16c,// 142 PAY 139 

    0x73b127af,// 143 PAY 140 

    0x38047244,// 144 PAY 141 

    0xe4c80ed9,// 145 PAY 142 

    0x33986ab5,// 146 PAY 143 

    0x363692d5,// 147 PAY 144 

    0xaf74a572,// 148 PAY 145 

    0x9c7a20f8,// 149 PAY 146 

    0x98eaa150,// 150 PAY 147 

    0x0262cb53,// 151 PAY 148 

    0xe409b369,// 152 PAY 149 

    0x03632373,// 153 PAY 150 

    0x9eeaef2c,// 154 PAY 151 

    0x12006d53,// 155 PAY 152 

    0x493742f8,// 156 PAY 153 

    0x07882d12,// 157 PAY 154 

    0x58458820,// 158 PAY 155 

    0xe9d4c55f,// 159 PAY 156 

    0x5a5778ea,// 160 PAY 157 

    0x7b065672,// 161 PAY 158 

    0x425b7288,// 162 PAY 159 

    0xfe5da59e,// 163 PAY 160 

    0xbf81c5b9,// 164 PAY 161 

    0x2cdd14d9,// 165 PAY 162 

    0x2e0d50d8,// 166 PAY 163 

    0x4338e6e0,// 167 PAY 164 

    0x5b771614,// 168 PAY 165 

    0x611fbf37,// 169 PAY 166 

    0x7e87142a,// 170 PAY 167 

    0x0547a45b,// 171 PAY 168 

    0x86431373,// 172 PAY 169 

    0x40e51aef,// 173 PAY 170 

    0xec5f839a,// 174 PAY 171 

    0x5cddd369,// 175 PAY 172 

    0x55d44b14,// 176 PAY 173 

    0x886abd97,// 177 PAY 174 

    0x3b4d1904,// 178 PAY 175 

    0xf127d206,// 179 PAY 176 

    0x462731e1,// 180 PAY 177 

    0xc5045ae9,// 181 PAY 178 

    0xcf1198ef,// 182 PAY 179 

    0x62403f0d,// 183 PAY 180 

    0x25007392,// 184 PAY 181 

    0x33f1e7ea,// 185 PAY 182 

    0x27869f27,// 186 PAY 183 

    0x8b9d6c2b,// 187 PAY 184 

    0x0885ce8e,// 188 PAY 185 

    0x30fd8788,// 189 PAY 186 

    0x640d05e3,// 190 PAY 187 

    0xdb2d701f,// 191 PAY 188 

    0xec94e48c,// 192 PAY 189 

    0xe3cfa226,// 193 PAY 190 

    0xe36c3633,// 194 PAY 191 

    0xbbbb9e69,// 195 PAY 192 

    0x8e8091b7,// 196 PAY 193 

    0xbc2491cb,// 197 PAY 194 

    0xc4566458,// 198 PAY 195 

    0xa2c61176,// 199 PAY 196 

    0x0f4d4b79,// 200 PAY 197 

    0xdb61fe45,// 201 PAY 198 

    0xfe26ee0c,// 202 PAY 199 

    0x0738dec7,// 203 PAY 200 

    0xe8317396,// 204 PAY 201 

    0xd1ba58e1,// 205 PAY 202 

    0x5245ec15,// 206 PAY 203 

    0xdaa0bed3,// 207 PAY 204 

    0xa2629e2b,// 208 PAY 205 

    0x3227d8aa,// 209 PAY 206 

    0x92f70ff2,// 210 PAY 207 

    0x8964c418,// 211 PAY 208 

    0x3f8e8932,// 212 PAY 209 

    0x4211f516,// 213 PAY 210 

    0xc6fde3a6,// 214 PAY 211 

    0x3e791c6a,// 215 PAY 212 

    0xd62724ae,// 216 PAY 213 

    0x076d3787,// 217 PAY 214 

    0xdc18dce3,// 218 PAY 215 

    0xc0f1fec2,// 219 PAY 216 

    0x28480a31,// 220 PAY 217 

    0x070a7449,// 221 PAY 218 

    0x9f2efb13,// 222 PAY 219 

    0x018b7a9b,// 223 PAY 220 

    0xbf2f2d42,// 224 PAY 221 

    0x40e4a6d2,// 225 PAY 222 

    0xee05fd1e,// 226 PAY 223 

    0xe60d5386,// 227 PAY 224 

    0x44b87038,// 228 PAY 225 

    0x174ac1e2,// 229 PAY 226 

    0x646bb304,// 230 PAY 227 

    0xa8390ea0,// 231 PAY 228 

    0xb66f97e5,// 232 PAY 229 

    0xea44a70c,// 233 PAY 230 

    0x637a84ba,// 234 PAY 231 

    0x43a64490,// 235 PAY 232 

    0x2244589b,// 236 PAY 233 

    0xdaee930b,// 237 PAY 234 

    0xbb708072,// 238 PAY 235 

    0xa95ba3af,// 239 PAY 236 

    0xc9ded651,// 240 PAY 237 

    0x4271f221,// 241 PAY 238 

    0xa7de8168,// 242 PAY 239 

    0x716072cb,// 243 PAY 240 

    0x4114987f,// 244 PAY 241 

    0x684ec04c,// 245 PAY 242 

    0xb93184ac,// 246 PAY 243 

    0x85eafe1e,// 247 PAY 244 

    0x5d75e417,// 248 PAY 245 

    0x79e2d91c,// 249 PAY 246 

    0xadfdea8a,// 250 PAY 247 

    0xedaf9584,// 251 PAY 248 

    0x8443e033,// 252 PAY 249 

    0xf000245d,// 253 PAY 250 

    0xdc5ec11a,// 254 PAY 251 

    0xd69318ae,// 255 PAY 252 

    0x9e74b65b,// 256 PAY 253 

    0x6630a630,// 257 PAY 254 

    0x3c334fb3,// 258 PAY 255 

    0x0223716c,// 259 PAY 256 

    0xb9db00f9,// 260 PAY 257 

    0xc60d2447,// 261 PAY 258 

    0x78847979,// 262 PAY 259 

    0x59854365,// 263 PAY 260 

    0x1d402266,// 264 PAY 261 

    0x36b057ba,// 265 PAY 262 

    0x228b9efc,// 266 PAY 263 

    0xfa7a7af2,// 267 PAY 264 

    0x4d391700,// 268 PAY 265 

    0x5205dc9e,// 269 PAY 266 

    0x199792e1,// 270 PAY 267 

    0x777e2a6d,// 271 PAY 268 

    0x0b8a6b71,// 272 PAY 269 

    0xecaa29ac,// 273 PAY 270 

    0x46e82704,// 274 PAY 271 

    0x7a3e7f99,// 275 PAY 272 

    0xe736a961,// 276 PAY 273 

    0x61356204,// 277 PAY 274 

    0xfbba9f5c,// 278 PAY 275 

    0xcebe95ad,// 279 PAY 276 

    0xb53e57b1,// 280 PAY 277 

    0xc0c4a3d2,// 281 PAY 278 

    0xffef571c,// 282 PAY 279 

    0x1f4d8558,// 283 PAY 280 

    0x3b30c0e9,// 284 PAY 281 

    0x9ca34205,// 285 PAY 282 

    0x7e5958ed,// 286 PAY 283 

    0xedd72295,// 287 PAY 284 

    0x7bc59782,// 288 PAY 285 

    0x0abecede,// 289 PAY 286 

    0x5b962dba,// 290 PAY 287 

    0xfdb7a062,// 291 PAY 288 

    0x69ebb806,// 292 PAY 289 

    0x35c90093,// 293 PAY 290 

    0xca3e3442,// 294 PAY 291 

    0x704d8928,// 295 PAY 292 

    0xe5062988,// 296 PAY 293 

    0xc85695a7,// 297 PAY 294 

    0x268f9cf4,// 298 PAY 295 

    0x6516aa45,// 299 PAY 296 

    0x4b092b04,// 300 PAY 297 

    0x713ae8f8,// 301 PAY 298 

    0xd8b15ff0,// 302 PAY 299 

    0x64511f90,// 303 PAY 300 

    0xe37a23e2,// 304 PAY 301 

    0x4c8c0526,// 305 PAY 302 

    0xb25b1b36,// 306 PAY 303 

    0x2e368e35,// 307 PAY 304 

    0x4cbd1c30,// 308 PAY 305 

    0x4966814a,// 309 PAY 306 

    0xb5c3984f,// 310 PAY 307 

    0x5c05895a,// 311 PAY 308 

    0xf5816f30,// 312 PAY 309 

    0x5c233c97,// 313 PAY 310 

    0x328aba75,// 314 PAY 311 

    0x2bbbb667,// 315 PAY 312 

    0x04a33793,// 316 PAY 313 

    0x91a9b728,// 317 PAY 314 

    0xa7afb07a,// 318 PAY 315 

    0x50ca47c0,// 319 PAY 316 

    0xd8b2434f,// 320 PAY 317 

    0x409ee1e2,// 321 PAY 318 

    0x498d0d13,// 322 PAY 319 

    0x824cf0ca,// 323 PAY 320 

    0xea73bf80,// 324 PAY 321 

    0x155fced0,// 325 PAY 322 

    0x16a0cfe6,// 326 PAY 323 

    0x8243fc51,// 327 PAY 324 

    0x4022fd2b,// 328 PAY 325 

    0x573cedfd,// 329 PAY 326 

    0xcb5ddbd1,// 330 PAY 327 

    0x785b0ed5,// 331 PAY 328 

    0x9dd0ac68,// 332 PAY 329 

    0xe836fbd4,// 333 PAY 330 

    0x4d1a0bdb,// 334 PAY 331 

    0x9f2dcff8,// 335 PAY 332 

    0x8b933536,// 336 PAY 333 

    0x7470cc67,// 337 PAY 334 

    0x62e77802,// 338 PAY 335 

    0xdc5e5832,// 339 PAY 336 

    0x476367aa,// 340 PAY 337 

    0xd802cbc4,// 341 PAY 338 

    0x08bf93f4,// 342 PAY 339 

    0xab9f5bd6,// 343 PAY 340 

    0x2108c7af,// 344 PAY 341 

    0x81a95649,// 345 PAY 342 

    0xff7bdcb1,// 346 PAY 343 

    0xc79c64eb,// 347 PAY 344 

    0xd7c393e6,// 348 PAY 345 

    0x28db3a6a,// 349 PAY 346 

    0x777fad5a,// 350 PAY 347 

    0x995bcf49,// 351 PAY 348 

    0x10021218,// 352 PAY 349 

    0x8a419795,// 353 PAY 350 

    0x04cf6c62,// 354 PAY 351 

    0x150bf2f1,// 355 PAY 352 

    0x4ea52386,// 356 PAY 353 

    0xf876e793,// 357 PAY 354 

    0xe1d5d29a,// 358 PAY 355 

    0xc9d67894,// 359 PAY 356 

    0x07427658,// 360 PAY 357 

    0xa9ea0d90,// 361 PAY 358 

    0xca014eba,// 362 PAY 359 

    0x12b15e2a,// 363 PAY 360 

    0x201f5fec,// 364 PAY 361 

    0xe3134bae,// 365 PAY 362 

    0x72130bf8,// 366 PAY 363 

    0x3cc924b4,// 367 PAY 364 

    0x76265101,// 368 PAY 365 

    0x0cfe5d03,// 369 PAY 366 

    0x651a6255,// 370 PAY 367 

    0x0629de60,// 371 PAY 368 

    0x6b053c20,// 372 PAY 369 

    0x93c2c846,// 373 PAY 370 

    0x3157ee89,// 374 PAY 371 

    0x522d2598,// 375 PAY 372 

    0xb3799ec3,// 376 PAY 373 

    0x980be56c,// 377 PAY 374 

    0x96f9f44a,// 378 PAY 375 

    0x80a92b71,// 379 PAY 376 

    0x5775e5e0,// 380 PAY 377 

    0xcef61f3c,// 381 PAY 378 

    0x4ddf05df,// 382 PAY 379 

    0xd04a4101,// 383 PAY 380 

    0xea59f074,// 384 PAY 381 

    0xcbdddeb6,// 385 PAY 382 

    0xe2714d19,// 386 PAY 383 

    0x8e3efa40,// 387 PAY 384 

    0x7236529d,// 388 PAY 385 

    0x6d74db25,// 389 PAY 386 

    0x2087dcb5,// 390 PAY 387 

    0xcbd87c15,// 391 PAY 388 

    0x30311473,// 392 PAY 389 

    0x49d723ce,// 393 PAY 390 

    0xddcc1723,// 394 PAY 391 

    0x391b5636,// 395 PAY 392 

    0xb2bdee3f,// 396 PAY 393 

    0x2d7fbc9d,// 397 PAY 394 

    0xae8c8fa1,// 398 PAY 395 

    0x8b353883,// 399 PAY 396 

    0x6ee0dc02,// 400 PAY 397 

    0x4b7f16e5,// 401 PAY 398 

    0x2d8f014c,// 402 PAY 399 

    0x609e3e79,// 403 PAY 400 

    0xf1a7405c,// 404 PAY 401 

    0x193b8f5f,// 405 PAY 402 

    0x64bd5ca0,// 406 PAY 403 

    0xe50938e2,// 407 PAY 404 

    0xcfc5b8e5,// 408 PAY 405 

    0xd8959317,// 409 PAY 406 

    0x2ed36dc3,// 410 PAY 407 

    0x7cfa32f3,// 411 PAY 408 

    0x60adc777,// 412 PAY 409 

    0x598eb1a3,// 413 PAY 410 

    0xaf49a61e,// 414 PAY 411 

    0x041193e7,// 415 PAY 412 

    0x11385dd0,// 416 PAY 413 

    0x4d6921b3,// 417 PAY 414 

    0xa0e68ddc,// 418 PAY 415 

    0x799e7c8f,// 419 PAY 416 

    0x2fd75c97,// 420 PAY 417 

    0x344253c6,// 421 PAY 418 

    0x45a9f4ae,// 422 PAY 419 

    0x8d669b85,// 423 PAY 420 

    0xac032eac,// 424 PAY 421 

    0x4482366b,// 425 PAY 422 

    0x6dfd82c6,// 426 PAY 423 

    0xbe39fdd8,// 427 PAY 424 

    0x8c5beff2,// 428 PAY 425 

    0x7ce14dc5,// 429 PAY 426 

    0x7b3b8f59,// 430 PAY 427 

    0x289e9d7e,// 431 PAY 428 

    0x6a5bf21e,// 432 PAY 429 

    0x5fcff832,// 433 PAY 430 

    0xa7af6f3a,// 434 PAY 431 

    0xf6be0584,// 435 PAY 432 

    0x456616a2,// 436 PAY 433 

    0x086744cb,// 437 PAY 434 

    0x7eeab153,// 438 PAY 435 

    0x39e32d65,// 439 PAY 436 

    0xd046cbcd,// 440 PAY 437 

    0xa7fed317,// 441 PAY 438 

    0x48f8101e,// 442 PAY 439 

    0x10bc5bb3,// 443 PAY 440 

    0xd3453575,// 444 PAY 441 

    0xae9e03c1,// 445 PAY 442 

    0x562d155b,// 446 PAY 443 

    0x85ce7503,// 447 PAY 444 

    0x9f2e65a9,// 448 PAY 445 

    0xf42d5c15,// 449 PAY 446 

    0x8b602445,// 450 PAY 447 

    0xa7cd6983,// 451 PAY 448 

    0x22c9988b,// 452 PAY 449 

    0x12765876,// 453 PAY 450 

    0x38784a36,// 454 PAY 451 

    0x89de0dfa,// 455 PAY 452 

    0x889fafe2,// 456 PAY 453 

    0x827188fb,// 457 PAY 454 

    0x3675cb2f,// 458 PAY 455 

    0x57f2af32,// 459 PAY 456 

    0x3cc0d9a1,// 460 PAY 457 

    0xa0d39adf,// 461 PAY 458 

    0x3b5fbbf3,// 462 PAY 459 

    0xd31c4544,// 463 PAY 460 

    0x0b236ed4,// 464 PAY 461 

    0xddeaea33,// 465 PAY 462 

    0x62f0fbff,// 466 PAY 463 

    0x5296a89b,// 467 PAY 464 

    0x9cb418f0,// 468 PAY 465 

    0xc4148163,// 469 PAY 466 

    0x2f105c27,// 470 PAY 467 

    0x660b7c07,// 471 PAY 468 

    0x01643565,// 472 PAY 469 

    0x9c9413d5,// 473 PAY 470 

    0xdc99ac87,// 474 PAY 471 

    0xb582ef6e,// 475 PAY 472 

    0xac083e08,// 476 PAY 473 

    0xed721e13,// 477 PAY 474 

    0x7c278727,// 478 PAY 475 

    0x25b54e66,// 479 PAY 476 

    0xc42eba64,// 480 PAY 477 

    0xac74e8cc,// 481 PAY 478 

    0x3c971f62,// 482 PAY 479 

    0x73bb0ff2,// 483 PAY 480 

    0xc368fe1a,// 484 PAY 481 

    0x89aacac6,// 485 PAY 482 

    0x818263ec,// 486 PAY 483 

    0xc770234f,// 487 PAY 484 

    0x8d15136c,// 488 PAY 485 

    0x37b995ff,// 489 PAY 486 

    0xd3f8f764,// 490 PAY 487 

    0x261ade06,// 491 PAY 488 

    0xf8868496,// 492 PAY 489 

    0x71e65046,// 493 PAY 490 

    0x6c1bd15c,// 494 PAY 491 

    0xd74da15c,// 495 PAY 492 

    0x3d4812ba,// 496 PAY 493 

    0x276a0159,// 497 PAY 494 

    0x4f676751,// 498 PAY 495 

    0x065e26da,// 499 PAY 496 

    0xd8260b87,// 500 PAY 497 

    0x89bd941f,// 501 PAY 498 

    0x50f97e4d,// 502 PAY 499 

    0x8d428774,// 503 PAY 500 

    0xbc55ea8a,// 504 PAY 501 

    0x94ccb5b2,// 505 PAY 502 

    0x1e82ad00,// 506 PAY 503 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x32 

    0x03f932b2 // 507 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt37_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 67 words. 

/// BDA size     is 261 (0x105) 

/// BDA id       is 0x1608 

    0x01051608,// 3 BDA   1 

/// PAY Generic Data size   : 261 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x26f42cad,// 4 PAY   1 

    0x16bb4f67,// 5 PAY   2 

    0x71bcb186,// 6 PAY   3 

    0x75e59f9a,// 7 PAY   4 

    0xd165d466,// 8 PAY   5 

    0x56b7e8f2,// 9 PAY   6 

    0x46838d6d,// 10 PAY   7 

    0xc17766d1,// 11 PAY   8 

    0xde366c32,// 12 PAY   9 

    0x74d6a101,// 13 PAY  10 

    0x11fc4b16,// 14 PAY  11 

    0xaec89687,// 15 PAY  12 

    0x2ae94e41,// 16 PAY  13 

    0x1144fb49,// 17 PAY  14 

    0xd90fb5a8,// 18 PAY  15 

    0x40374c84,// 19 PAY  16 

    0x023912bc,// 20 PAY  17 

    0xe07566b3,// 21 PAY  18 

    0x5ed3097b,// 22 PAY  19 

    0x5a442524,// 23 PAY  20 

    0x116836ae,// 24 PAY  21 

    0xd2dbedbd,// 25 PAY  22 

    0x4285933c,// 26 PAY  23 

    0x7c1662f0,// 27 PAY  24 

    0x3c8b1b40,// 28 PAY  25 

    0x95545f22,// 29 PAY  26 

    0x8480aa38,// 30 PAY  27 

    0xfd219bbd,// 31 PAY  28 

    0x05bc3d82,// 32 PAY  29 

    0xc5e04016,// 33 PAY  30 

    0x21260e1f,// 34 PAY  31 

    0x728828e6,// 35 PAY  32 

    0x573a5d94,// 36 PAY  33 

    0x05fab92d,// 37 PAY  34 

    0xf5416dcb,// 38 PAY  35 

    0x9684dcfb,// 39 PAY  36 

    0x4b452f37,// 40 PAY  37 

    0x47adec8c,// 41 PAY  38 

    0x12d3e180,// 42 PAY  39 

    0x4e27fea8,// 43 PAY  40 

    0x9c2a3f82,// 44 PAY  41 

    0x21665265,// 45 PAY  42 

    0xec1b3d84,// 46 PAY  43 

    0x409e2e81,// 47 PAY  44 

    0xc4924d39,// 48 PAY  45 

    0x6d10b6be,// 49 PAY  46 

    0xc3706033,// 50 PAY  47 

    0x7d6106e7,// 51 PAY  48 

    0x7e13c48f,// 52 PAY  49 

    0x6a86327c,// 53 PAY  50 

    0x672ac703,// 54 PAY  51 

    0xa2fe3488,// 55 PAY  52 

    0xf407aa95,// 56 PAY  53 

    0xc1dbfa38,// 57 PAY  54 

    0xadc0c315,// 58 PAY  55 

    0xa862de1d,// 59 PAY  56 

    0xc0f57c86,// 60 PAY  57 

    0x92d09a8f,// 61 PAY  58 

    0xb7cdc9c6,// 62 PAY  59 

    0xb95886a4,// 63 PAY  60 

    0x5e226bc8,// 64 PAY  61 

    0x344dc815,// 65 PAY  62 

    0xfd3d44a2,// 66 PAY  63 

    0x5cd7294c,// 67 PAY  64 

    0x91a5d416,// 68 PAY  65 

    0x2b000000,// 69 PAY  66 

/// HASH is  4 bytes 

    0xc0f57c86,// 70 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 164 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x0161bea4 // 71 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt38_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 506 words. 

/// BDA size     is 2019 (0x7e3) 

/// BDA id       is 0x1528 

    0x07e31528,// 3 BDA   1 

/// PAY Generic Data size   : 2019 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x53b7195f,// 4 PAY   1 

    0xb51e615f,// 5 PAY   2 

    0xd2df75e6,// 6 PAY   3 

    0x3acf6003,// 7 PAY   4 

    0x11493995,// 8 PAY   5 

    0x2cd21b65,// 9 PAY   6 

    0x623fe049,// 10 PAY   7 

    0x387c8b2e,// 11 PAY   8 

    0x10f56049,// 12 PAY   9 

    0xe6186814,// 13 PAY  10 

    0x296a24ff,// 14 PAY  11 

    0xc449402b,// 15 PAY  12 

    0x73c55340,// 16 PAY  13 

    0xed6f0ee3,// 17 PAY  14 

    0x4b4d2989,// 18 PAY  15 

    0x1d4a05e8,// 19 PAY  16 

    0xd25b03e1,// 20 PAY  17 

    0x693faf82,// 21 PAY  18 

    0x688edf7b,// 22 PAY  19 

    0x720add7c,// 23 PAY  20 

    0xe800ee21,// 24 PAY  21 

    0x3ab19435,// 25 PAY  22 

    0xa69f2859,// 26 PAY  23 

    0x006e247e,// 27 PAY  24 

    0x3b917cea,// 28 PAY  25 

    0x0222158e,// 29 PAY  26 

    0x89f82a52,// 30 PAY  27 

    0xe8a053fd,// 31 PAY  28 

    0x072c16a1,// 32 PAY  29 

    0x5d56af15,// 33 PAY  30 

    0x821e5e31,// 34 PAY  31 

    0x9ef7a70b,// 35 PAY  32 

    0xf509ac55,// 36 PAY  33 

    0xbcdc2e47,// 37 PAY  34 

    0x19e839f0,// 38 PAY  35 

    0x9d1ef204,// 39 PAY  36 

    0xd219283c,// 40 PAY  37 

    0x36594562,// 41 PAY  38 

    0x32e17291,// 42 PAY  39 

    0xd62f78c5,// 43 PAY  40 

    0xd141b980,// 44 PAY  41 

    0x66cab2ab,// 45 PAY  42 

    0xd9fae650,// 46 PAY  43 

    0xda460d6f,// 47 PAY  44 

    0xae8fb276,// 48 PAY  45 

    0x65e16c69,// 49 PAY  46 

    0x1e42b923,// 50 PAY  47 

    0xd11334d8,// 51 PAY  48 

    0xeeff41d9,// 52 PAY  49 

    0x71f186da,// 53 PAY  50 

    0x9294d02d,// 54 PAY  51 

    0xb701c2a5,// 55 PAY  52 

    0xe2b3eb0d,// 56 PAY  53 

    0x06907c6b,// 57 PAY  54 

    0x82302d8e,// 58 PAY  55 

    0x3c4fab63,// 59 PAY  56 

    0x4c7417d9,// 60 PAY  57 

    0x265850fa,// 61 PAY  58 

    0x97a84cd9,// 62 PAY  59 

    0x4115d20d,// 63 PAY  60 

    0x4a8fd5c4,// 64 PAY  61 

    0x03f1d461,// 65 PAY  62 

    0x3fe00874,// 66 PAY  63 

    0x23f810ee,// 67 PAY  64 

    0x02772939,// 68 PAY  65 

    0xf28163a0,// 69 PAY  66 

    0xd281652d,// 70 PAY  67 

    0x26caee56,// 71 PAY  68 

    0x4647be7c,// 72 PAY  69 

    0x7e48d899,// 73 PAY  70 

    0x9ffe818e,// 74 PAY  71 

    0xf1ad5475,// 75 PAY  72 

    0x85792f28,// 76 PAY  73 

    0x2e6578aa,// 77 PAY  74 

    0x780dc5ae,// 78 PAY  75 

    0xb73e9be6,// 79 PAY  76 

    0x3c2c7ea2,// 80 PAY  77 

    0x85292175,// 81 PAY  78 

    0x4c0de51d,// 82 PAY  79 

    0xa996d186,// 83 PAY  80 

    0xab372122,// 84 PAY  81 

    0xefd8a713,// 85 PAY  82 

    0x8d5adb3c,// 86 PAY  83 

    0xed60631d,// 87 PAY  84 

    0xe8ab2eb5,// 88 PAY  85 

    0x53f279bd,// 89 PAY  86 

    0x7633504d,// 90 PAY  87 

    0x6db0be43,// 91 PAY  88 

    0x468ce3a1,// 92 PAY  89 

    0x0c20627e,// 93 PAY  90 

    0x413d3dfa,// 94 PAY  91 

    0xfce2e5e0,// 95 PAY  92 

    0x78b13086,// 96 PAY  93 

    0xcff4181e,// 97 PAY  94 

    0x5c0bb034,// 98 PAY  95 

    0x153ca351,// 99 PAY  96 

    0xe57f88b1,// 100 PAY  97 

    0x9a8de605,// 101 PAY  98 

    0x2ce4f7eb,// 102 PAY  99 

    0x5bd20c2e,// 103 PAY 100 

    0xb0d75b35,// 104 PAY 101 

    0x87827035,// 105 PAY 102 

    0x60200d67,// 106 PAY 103 

    0x189dba77,// 107 PAY 104 

    0xe3073364,// 108 PAY 105 

    0xb670e29b,// 109 PAY 106 

    0x4715d7a1,// 110 PAY 107 

    0x64c562ee,// 111 PAY 108 

    0xdb864df9,// 112 PAY 109 

    0x6995f71e,// 113 PAY 110 

    0x28ee0cf7,// 114 PAY 111 

    0xe9720378,// 115 PAY 112 

    0xe741f744,// 116 PAY 113 

    0xcf38d564,// 117 PAY 114 

    0xf15b9367,// 118 PAY 115 

    0x033ca801,// 119 PAY 116 

    0x024bc96d,// 120 PAY 117 

    0x5c7a9c37,// 121 PAY 118 

    0x2d55fb40,// 122 PAY 119 

    0x93d42c3b,// 123 PAY 120 

    0x620881d9,// 124 PAY 121 

    0xdae23778,// 125 PAY 122 

    0x083d47fa,// 126 PAY 123 

    0xf43117d4,// 127 PAY 124 

    0x8cc59730,// 128 PAY 125 

    0x8ed6e7cf,// 129 PAY 126 

    0xdb8918c7,// 130 PAY 127 

    0xc7120b01,// 131 PAY 128 

    0xdb2ce2c1,// 132 PAY 129 

    0x7c57a491,// 133 PAY 130 

    0xe6b1892c,// 134 PAY 131 

    0xd6faba16,// 135 PAY 132 

    0x8a8b8d07,// 136 PAY 133 

    0xf44baee7,// 137 PAY 134 

    0x47b33743,// 138 PAY 135 

    0x9622831d,// 139 PAY 136 

    0xef61d23f,// 140 PAY 137 

    0xe6a63587,// 141 PAY 138 

    0xbf2eea9f,// 142 PAY 139 

    0x9089d1bd,// 143 PAY 140 

    0xfe66a3ef,// 144 PAY 141 

    0xe0d2e7fb,// 145 PAY 142 

    0x9753e0ba,// 146 PAY 143 

    0x4ced21bd,// 147 PAY 144 

    0x5402124b,// 148 PAY 145 

    0x0bebf9b7,// 149 PAY 146 

    0x6a4108a4,// 150 PAY 147 

    0x06e7a17c,// 151 PAY 148 

    0x8b66b2ad,// 152 PAY 149 

    0x5c6a9934,// 153 PAY 150 

    0x4f0da627,// 154 PAY 151 

    0x2d927650,// 155 PAY 152 

    0xbdb23acc,// 156 PAY 153 

    0x497cd366,// 157 PAY 154 

    0xd8c2ea06,// 158 PAY 155 

    0x63894e4b,// 159 PAY 156 

    0x234c5d2e,// 160 PAY 157 

    0xa1e6304b,// 161 PAY 158 

    0x897d49bc,// 162 PAY 159 

    0x5e99e37f,// 163 PAY 160 

    0xaa8d298d,// 164 PAY 161 

    0xae579db8,// 165 PAY 162 

    0xdbc952ff,// 166 PAY 163 

    0x307b1815,// 167 PAY 164 

    0xc92b4171,// 168 PAY 165 

    0x5335bf4e,// 169 PAY 166 

    0x12834bfd,// 170 PAY 167 

    0xbc8980ff,// 171 PAY 168 

    0xe0e77304,// 172 PAY 169 

    0x958c225d,// 173 PAY 170 

    0x06ea9dbd,// 174 PAY 171 

    0xb1d69144,// 175 PAY 172 

    0xee5c30c9,// 176 PAY 173 

    0x61dfe844,// 177 PAY 174 

    0x3bed9481,// 178 PAY 175 

    0x1699de74,// 179 PAY 176 

    0xc2d048c9,// 180 PAY 177 

    0xdfd89f6b,// 181 PAY 178 

    0x5843d050,// 182 PAY 179 

    0xe4baead4,// 183 PAY 180 

    0xf973edb7,// 184 PAY 181 

    0x5cc0fe65,// 185 PAY 182 

    0x8c61a2da,// 186 PAY 183 

    0x90e5eb4f,// 187 PAY 184 

    0xe64b16a4,// 188 PAY 185 

    0x1172537c,// 189 PAY 186 

    0x5d2d2557,// 190 PAY 187 

    0x7f964d2f,// 191 PAY 188 

    0x45de378e,// 192 PAY 189 

    0xa79a659a,// 193 PAY 190 

    0xec3562cd,// 194 PAY 191 

    0x576da7da,// 195 PAY 192 

    0xdaafbdd9,// 196 PAY 193 

    0x53176c00,// 197 PAY 194 

    0x36db8346,// 198 PAY 195 

    0x1faf8aeb,// 199 PAY 196 

    0xd39db41d,// 200 PAY 197 

    0x758b22fe,// 201 PAY 198 

    0xb7b835cf,// 202 PAY 199 

    0xb5d02e02,// 203 PAY 200 

    0xa379399f,// 204 PAY 201 

    0xf76cb7f7,// 205 PAY 202 

    0x9c05151d,// 206 PAY 203 

    0x00f61d74,// 207 PAY 204 

    0x8789b47f,// 208 PAY 205 

    0xb179c165,// 209 PAY 206 

    0xee722c3a,// 210 PAY 207 

    0x19c4c39f,// 211 PAY 208 

    0x36e6283d,// 212 PAY 209 

    0x77b79a6d,// 213 PAY 210 

    0x646302d9,// 214 PAY 211 

    0x002b562d,// 215 PAY 212 

    0x07a60b9c,// 216 PAY 213 

    0xb7cfbae2,// 217 PAY 214 

    0xf6be1e5e,// 218 PAY 215 

    0xb07d34b6,// 219 PAY 216 

    0xa1ac62ac,// 220 PAY 217 

    0x86fdc6b6,// 221 PAY 218 

    0xed34a507,// 222 PAY 219 

    0x51ffc841,// 223 PAY 220 

    0x42f3a3a2,// 224 PAY 221 

    0x5d6af412,// 225 PAY 222 

    0x80f123b9,// 226 PAY 223 

    0x4d5f1635,// 227 PAY 224 

    0x8dc374cc,// 228 PAY 225 

    0x3a774298,// 229 PAY 226 

    0x8ca436d3,// 230 PAY 227 

    0xe27f9a8c,// 231 PAY 228 

    0xd1a3e71d,// 232 PAY 229 

    0x26cf5109,// 233 PAY 230 

    0x591d7de6,// 234 PAY 231 

    0x6e6eabdf,// 235 PAY 232 

    0x20617b89,// 236 PAY 233 

    0x99f4a0e7,// 237 PAY 234 

    0x16e873b9,// 238 PAY 235 

    0x3e9f6816,// 239 PAY 236 

    0x3a4b732b,// 240 PAY 237 

    0x3373314e,// 241 PAY 238 

    0x631c0a70,// 242 PAY 239 

    0x4cc3d4b9,// 243 PAY 240 

    0x4b3ce583,// 244 PAY 241 

    0x10d1fc0e,// 245 PAY 242 

    0x46b0f5e0,// 246 PAY 243 

    0xc1042e76,// 247 PAY 244 

    0x97a9d63a,// 248 PAY 245 

    0x495ba8fd,// 249 PAY 246 

    0x770f3f78,// 250 PAY 247 

    0x02014a83,// 251 PAY 248 

    0x87f9b7d2,// 252 PAY 249 

    0x646bf453,// 253 PAY 250 

    0xfae483d6,// 254 PAY 251 

    0xde1d00c1,// 255 PAY 252 

    0xea8fd1f6,// 256 PAY 253 

    0xfc070852,// 257 PAY 254 

    0xb756e10e,// 258 PAY 255 

    0x5a04268f,// 259 PAY 256 

    0x819964c5,// 260 PAY 257 

    0x374a08cf,// 261 PAY 258 

    0xfca99a9f,// 262 PAY 259 

    0xc32d793b,// 263 PAY 260 

    0x24cc4e08,// 264 PAY 261 

    0x38acfc82,// 265 PAY 262 

    0x3cf85c8b,// 266 PAY 263 

    0x99c5c357,// 267 PAY 264 

    0x69fd49b5,// 268 PAY 265 

    0xc9a15161,// 269 PAY 266 

    0xed60056d,// 270 PAY 267 

    0x3d2378f7,// 271 PAY 268 

    0x334b542e,// 272 PAY 269 

    0xe226f81d,// 273 PAY 270 

    0x2a95778f,// 274 PAY 271 

    0x691ace4d,// 275 PAY 272 

    0x7d780527,// 276 PAY 273 

    0xbe79f979,// 277 PAY 274 

    0xc4c2c52e,// 278 PAY 275 

    0xc6e8b61c,// 279 PAY 276 

    0xde37f5da,// 280 PAY 277 

    0xafc9efd3,// 281 PAY 278 

    0xc5df0b4a,// 282 PAY 279 

    0x6a888dcd,// 283 PAY 280 

    0x57314a70,// 284 PAY 281 

    0xb959f31e,// 285 PAY 282 

    0x4bad0cd8,// 286 PAY 283 

    0x29b02a8a,// 287 PAY 284 

    0x7ac08633,// 288 PAY 285 

    0xb1d71bb3,// 289 PAY 286 

    0x1847b7ab,// 290 PAY 287 

    0xf7aee9cf,// 291 PAY 288 

    0xe70581f7,// 292 PAY 289 

    0x51d818bf,// 293 PAY 290 

    0x56f80f10,// 294 PAY 291 

    0x26eece91,// 295 PAY 292 

    0x20b82882,// 296 PAY 293 

    0xdbf58f62,// 297 PAY 294 

    0xc99140e1,// 298 PAY 295 

    0xaf8612cd,// 299 PAY 296 

    0x5288e3db,// 300 PAY 297 

    0x432e756e,// 301 PAY 298 

    0x3069ccc2,// 302 PAY 299 

    0x5f65bc1f,// 303 PAY 300 

    0x8ec92b04,// 304 PAY 301 

    0x626274fb,// 305 PAY 302 

    0xba5516ed,// 306 PAY 303 

    0x4c942479,// 307 PAY 304 

    0xc1802428,// 308 PAY 305 

    0x471987d9,// 309 PAY 306 

    0xa591a105,// 310 PAY 307 

    0x1e60b280,// 311 PAY 308 

    0x2cc44562,// 312 PAY 309 

    0x5d9df1a8,// 313 PAY 310 

    0x9ccfae6e,// 314 PAY 311 

    0x2e209636,// 315 PAY 312 

    0x96ccb8f6,// 316 PAY 313 

    0xe281d869,// 317 PAY 314 

    0x6c6ed117,// 318 PAY 315 

    0x0e6edc16,// 319 PAY 316 

    0x8c2ebd92,// 320 PAY 317 

    0xd51f5818,// 321 PAY 318 

    0xc4452bc9,// 322 PAY 319 

    0x49c671e0,// 323 PAY 320 

    0x6aba6f1b,// 324 PAY 321 

    0x12efe8d9,// 325 PAY 322 

    0x6f5ce336,// 326 PAY 323 

    0xf527c7a5,// 327 PAY 324 

    0x6512b0f5,// 328 PAY 325 

    0x58faf01d,// 329 PAY 326 

    0x212ce813,// 330 PAY 327 

    0x6a18f855,// 331 PAY 328 

    0x412ab56d,// 332 PAY 329 

    0xba33e98c,// 333 PAY 330 

    0x0e582869,// 334 PAY 331 

    0x6cda8c7a,// 335 PAY 332 

    0x308565ad,// 336 PAY 333 

    0xd229816c,// 337 PAY 334 

    0x1cdecb92,// 338 PAY 335 

    0x5e3f81c2,// 339 PAY 336 

    0x1d340b65,// 340 PAY 337 

    0x6dee13c9,// 341 PAY 338 

    0xd8015b72,// 342 PAY 339 

    0xa4753683,// 343 PAY 340 

    0x7006bd14,// 344 PAY 341 

    0x4204ad51,// 345 PAY 342 

    0x42ef2112,// 346 PAY 343 

    0x705207aa,// 347 PAY 344 

    0x2837ed3e,// 348 PAY 345 

    0x1d4b5cf2,// 349 PAY 346 

    0xb9870b2a,// 350 PAY 347 

    0xf24a3c9a,// 351 PAY 348 

    0x559acd96,// 352 PAY 349 

    0x59df7c98,// 353 PAY 350 

    0x29c20c84,// 354 PAY 351 

    0xfa34a94e,// 355 PAY 352 

    0xb88b19d8,// 356 PAY 353 

    0xb259b454,// 357 PAY 354 

    0xc89cc823,// 358 PAY 355 

    0x86f637c6,// 359 PAY 356 

    0xb2a4fb85,// 360 PAY 357 

    0x1212e9e4,// 361 PAY 358 

    0x4ccaf02f,// 362 PAY 359 

    0x198a8754,// 363 PAY 360 

    0xb0302c32,// 364 PAY 361 

    0xc52d7e9e,// 365 PAY 362 

    0xfe868d5c,// 366 PAY 363 

    0x01c6031c,// 367 PAY 364 

    0x902749fe,// 368 PAY 365 

    0xd5a40491,// 369 PAY 366 

    0x1c2596d5,// 370 PAY 367 

    0x19f9c37b,// 371 PAY 368 

    0xa7492430,// 372 PAY 369 

    0xf1e85075,// 373 PAY 370 

    0xaa4df9d2,// 374 PAY 371 

    0x5ae7dbb4,// 375 PAY 372 

    0xd22dc336,// 376 PAY 373 

    0x77d0778d,// 377 PAY 374 

    0xc77e7ba7,// 378 PAY 375 

    0x48668886,// 379 PAY 376 

    0x0444beba,// 380 PAY 377 

    0x7c1566b6,// 381 PAY 378 

    0x9572c369,// 382 PAY 379 

    0xc810c267,// 383 PAY 380 

    0xd718adea,// 384 PAY 381 

    0x9c884209,// 385 PAY 382 

    0x305bdc91,// 386 PAY 383 

    0x2c0c8663,// 387 PAY 384 

    0xda02427f,// 388 PAY 385 

    0x8ba8ae45,// 389 PAY 386 

    0x989fd775,// 390 PAY 387 

    0x10dc33d7,// 391 PAY 388 

    0x480b7cd4,// 392 PAY 389 

    0x91ddbd32,// 393 PAY 390 

    0x01349f76,// 394 PAY 391 

    0xd1a4bf0f,// 395 PAY 392 

    0x5486450c,// 396 PAY 393 

    0x88331135,// 397 PAY 394 

    0xee092f0a,// 398 PAY 395 

    0xaac08b19,// 399 PAY 396 

    0xf3e4ea32,// 400 PAY 397 

    0xc3df0597,// 401 PAY 398 

    0xac9332fa,// 402 PAY 399 

    0x239f682c,// 403 PAY 400 

    0x1612e6bd,// 404 PAY 401 

    0x2a83eeeb,// 405 PAY 402 

    0x7f013d6c,// 406 PAY 403 

    0xeeed0777,// 407 PAY 404 

    0x20286fd9,// 408 PAY 405 

    0xd1373d3e,// 409 PAY 406 

    0x1805507c,// 410 PAY 407 

    0x46dee1eb,// 411 PAY 408 

    0x01e52469,// 412 PAY 409 

    0x9eb9ed98,// 413 PAY 410 

    0x8aecffe6,// 414 PAY 411 

    0x44ba3826,// 415 PAY 412 

    0x15674a20,// 416 PAY 413 

    0xb834b7e0,// 417 PAY 414 

    0xba4a4238,// 418 PAY 415 

    0xac80f7aa,// 419 PAY 416 

    0xd433ec42,// 420 PAY 417 

    0xf2246953,// 421 PAY 418 

    0xfa42dc94,// 422 PAY 419 

    0x53bd4593,// 423 PAY 420 

    0xba2eafb6,// 424 PAY 421 

    0x82994f83,// 425 PAY 422 

    0xb068fca5,// 426 PAY 423 

    0x07b9eda2,// 427 PAY 424 

    0xaf72a1da,// 428 PAY 425 

    0xa028668e,// 429 PAY 426 

    0x5c22c8e8,// 430 PAY 427 

    0xb03aec0f,// 431 PAY 428 

    0x3a56b516,// 432 PAY 429 

    0x6882a949,// 433 PAY 430 

    0xb9cf85cf,// 434 PAY 431 

    0x6e8483a7,// 435 PAY 432 

    0x8cf77005,// 436 PAY 433 

    0xce894537,// 437 PAY 434 

    0x4135a621,// 438 PAY 435 

    0x2d4188ae,// 439 PAY 436 

    0x53531091,// 440 PAY 437 

    0x39688b60,// 441 PAY 438 

    0xdeb26d52,// 442 PAY 439 

    0x07d979cb,// 443 PAY 440 

    0xe0877ffa,// 444 PAY 441 

    0x18289aa9,// 445 PAY 442 

    0x5daa4897,// 446 PAY 443 

    0x36457bd8,// 447 PAY 444 

    0x825f9e83,// 448 PAY 445 

    0xeec0fbfe,// 449 PAY 446 

    0x3f5614cc,// 450 PAY 447 

    0x74c19545,// 451 PAY 448 

    0x652dc693,// 452 PAY 449 

    0xa9f9057d,// 453 PAY 450 

    0x40df5bda,// 454 PAY 451 

    0x808b2322,// 455 PAY 452 

    0x73a7e088,// 456 PAY 453 

    0xd822b923,// 457 PAY 454 

    0x9390b118,// 458 PAY 455 

    0xfefc8113,// 459 PAY 456 

    0xde9a557a,// 460 PAY 457 

    0x81d45911,// 461 PAY 458 

    0x49d76b35,// 462 PAY 459 

    0xa114ffb0,// 463 PAY 460 

    0xf38ded73,// 464 PAY 461 

    0x8843df54,// 465 PAY 462 

    0xc5a28a87,// 466 PAY 463 

    0x875e191b,// 467 PAY 464 

    0x76b5e462,// 468 PAY 465 

    0xb3e66ddd,// 469 PAY 466 

    0x28ae2489,// 470 PAY 467 

    0xa1a7366b,// 471 PAY 468 

    0xd5d0ab39,// 472 PAY 469 

    0x6e83c124,// 473 PAY 470 

    0x5993514a,// 474 PAY 471 

    0x9553f441,// 475 PAY 472 

    0xc44bbbab,// 476 PAY 473 

    0x3442c145,// 477 PAY 474 

    0xc1eb529e,// 478 PAY 475 

    0x1773ad36,// 479 PAY 476 

    0x4ba4287d,// 480 PAY 477 

    0xd9ff0b1a,// 481 PAY 478 

    0x86d6edc3,// 482 PAY 479 

    0x92c523fa,// 483 PAY 480 

    0x0702c7d0,// 484 PAY 481 

    0x24a2e98a,// 485 PAY 482 

    0xb63404c5,// 486 PAY 483 

    0x5eed8c33,// 487 PAY 484 

    0x79043bf6,// 488 PAY 485 

    0x898a2d9e,// 489 PAY 486 

    0xd2ab52c9,// 490 PAY 487 

    0x333576fe,// 491 PAY 488 

    0xf73de63b,// 492 PAY 489 

    0x26c89076,// 493 PAY 490 

    0x384d4737,// 494 PAY 491 

    0x0aea8c6e,// 495 PAY 492 

    0xf3dd237b,// 496 PAY 493 

    0x07cc9e31,// 497 PAY 494 

    0x8dd2e00c,// 498 PAY 495 

    0x736d7d18,// 499 PAY 496 

    0xba78ef0c,// 500 PAY 497 

    0x89e6a932,// 501 PAY 498 

    0x53742dc0,// 502 PAY 499 

    0xd7163e9c,// 503 PAY 500 

    0xd985138d,// 504 PAY 501 

    0xaa11c5a6,// 505 PAY 502 

    0xc91ff947,// 506 PAY 503 

    0xbae2cb00,// 507 PAY 504 

    0x8f9ac300,// 508 PAY 505 

/// HASH is  20 bytes 

    0x07b9eda2,// 509 HSH   1 

    0xaf72a1da,// 510 HSH   2 

    0xa028668e,// 511 HSH   3 

    0x5c22c8e8,// 512 HSH   4 

    0xb03aec0f,// 513 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 229 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4f 

    0x01314fe5 // 514 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt39_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 411 words. 

/// BDA size     is 1638 (0x666) 

/// BDA id       is 0xc25c 

    0x0666c25c,// 3 BDA   1 

/// PAY Generic Data size   : 1638 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xd3cf96ff,// 4 PAY   1 

    0x4e5cde7d,// 5 PAY   2 

    0x9ea04124,// 6 PAY   3 

    0x2fce91de,// 7 PAY   4 

    0xb6f7a075,// 8 PAY   5 

    0xdf8bfc4d,// 9 PAY   6 

    0xc509743f,// 10 PAY   7 

    0x8b9ca1f7,// 11 PAY   8 

    0xaf07c95f,// 12 PAY   9 

    0x32941a74,// 13 PAY  10 

    0x2c52f2c5,// 14 PAY  11 

    0x058c7175,// 15 PAY  12 

    0x137d114f,// 16 PAY  13 

    0xd5041939,// 17 PAY  14 

    0x951bb58c,// 18 PAY  15 

    0x9ccfbeb7,// 19 PAY  16 

    0x9ea3a777,// 20 PAY  17 

    0xc737d1d3,// 21 PAY  18 

    0xc5f5ac4b,// 22 PAY  19 

    0x3a7fc954,// 23 PAY  20 

    0x9ec0028a,// 24 PAY  21 

    0x41318cae,// 25 PAY  22 

    0xbdf33194,// 26 PAY  23 

    0xdef80707,// 27 PAY  24 

    0x21ba9e9f,// 28 PAY  25 

    0xb9340e1c,// 29 PAY  26 

    0x88eefb03,// 30 PAY  27 

    0x506de955,// 31 PAY  28 

    0x5d99dff2,// 32 PAY  29 

    0x895a65e1,// 33 PAY  30 

    0x8881624a,// 34 PAY  31 

    0x9dd23d4d,// 35 PAY  32 

    0xd07f27a0,// 36 PAY  33 

    0x653eb63a,// 37 PAY  34 

    0x8f5e47e6,// 38 PAY  35 

    0xff8b6575,// 39 PAY  36 

    0xae439428,// 40 PAY  37 

    0xa8dc3f41,// 41 PAY  38 

    0x3cb02aa1,// 42 PAY  39 

    0x03a87d40,// 43 PAY  40 

    0xea7f6654,// 44 PAY  41 

    0x65e4922c,// 45 PAY  42 

    0x2c009cdd,// 46 PAY  43 

    0x3a3c6d14,// 47 PAY  44 

    0xbd49bf5e,// 48 PAY  45 

    0x78cc53e2,// 49 PAY  46 

    0x5fe4d9c1,// 50 PAY  47 

    0xcfef256d,// 51 PAY  48 

    0x9f981af0,// 52 PAY  49 

    0xb0c39104,// 53 PAY  50 

    0xbea3c705,// 54 PAY  51 

    0xdb4639a7,// 55 PAY  52 

    0xd18ea835,// 56 PAY  53 

    0x5f642f54,// 57 PAY  54 

    0xbc070c33,// 58 PAY  55 

    0xabeac319,// 59 PAY  56 

    0x28742c1f,// 60 PAY  57 

    0x25007546,// 61 PAY  58 

    0x6aef8421,// 62 PAY  59 

    0xcc583707,// 63 PAY  60 

    0x4b8fd347,// 64 PAY  61 

    0xa6f84765,// 65 PAY  62 

    0x8e24ff00,// 66 PAY  63 

    0xb73a024f,// 67 PAY  64 

    0x7339dac2,// 68 PAY  65 

    0x82454fb5,// 69 PAY  66 

    0xb1d5a7a9,// 70 PAY  67 

    0x7bdb4881,// 71 PAY  68 

    0x022b2576,// 72 PAY  69 

    0x95094dbb,// 73 PAY  70 

    0x3f111111,// 74 PAY  71 

    0xdeb69bb3,// 75 PAY  72 

    0x9f2c200f,// 76 PAY  73 

    0x0bc34b42,// 77 PAY  74 

    0xe1db7073,// 78 PAY  75 

    0x42202fe9,// 79 PAY  76 

    0xe2e6239b,// 80 PAY  77 

    0xa848980e,// 81 PAY  78 

    0x12dff5b0,// 82 PAY  79 

    0x57ac345c,// 83 PAY  80 

    0xedd0c5d6,// 84 PAY  81 

    0x26a9abb0,// 85 PAY  82 

    0xec7aaffd,// 86 PAY  83 

    0x0178ff7f,// 87 PAY  84 

    0xed9abe88,// 88 PAY  85 

    0x81f4e212,// 89 PAY  86 

    0x5dd891f4,// 90 PAY  87 

    0xded56a81,// 91 PAY  88 

    0x740df4bb,// 92 PAY  89 

    0x4db4d6a3,// 93 PAY  90 

    0x85fed43e,// 94 PAY  91 

    0x67585870,// 95 PAY  92 

    0x964f82db,// 96 PAY  93 

    0x3fb801c0,// 97 PAY  94 

    0x9cc0b9b0,// 98 PAY  95 

    0xaafa8223,// 99 PAY  96 

    0x06a08906,// 100 PAY  97 

    0xe28c8325,// 101 PAY  98 

    0xdf08eeb7,// 102 PAY  99 

    0x567483e8,// 103 PAY 100 

    0x030736ac,// 104 PAY 101 

    0xefcc2135,// 105 PAY 102 

    0xbb7d3f31,// 106 PAY 103 

    0xffb37f92,// 107 PAY 104 

    0x11f1c88e,// 108 PAY 105 

    0xebe800f0,// 109 PAY 106 

    0x12b291de,// 110 PAY 107 

    0x34686734,// 111 PAY 108 

    0xd783b8cc,// 112 PAY 109 

    0x2598f951,// 113 PAY 110 

    0x492d775d,// 114 PAY 111 

    0xee3cde4b,// 115 PAY 112 

    0xa59959d9,// 116 PAY 113 

    0xfec1b711,// 117 PAY 114 

    0xe62e02d6,// 118 PAY 115 

    0x5401be39,// 119 PAY 116 

    0x5debbbf3,// 120 PAY 117 

    0xc28506a3,// 121 PAY 118 

    0xf53cfdd8,// 122 PAY 119 

    0x7e510891,// 123 PAY 120 

    0xa104d296,// 124 PAY 121 

    0x2397ec11,// 125 PAY 122 

    0x5fd3649a,// 126 PAY 123 

    0xe54fb7a4,// 127 PAY 124 

    0x2928f4d9,// 128 PAY 125 

    0x7a09a43e,// 129 PAY 126 

    0xb5ac609d,// 130 PAY 127 

    0x6c31e37f,// 131 PAY 128 

    0x161f8682,// 132 PAY 129 

    0x9d0cc214,// 133 PAY 130 

    0x63e50d48,// 134 PAY 131 

    0xa52503d3,// 135 PAY 132 

    0x6e313d7d,// 136 PAY 133 

    0x5b485e1a,// 137 PAY 134 

    0xe1f560ab,// 138 PAY 135 

    0x64cedd1c,// 139 PAY 136 

    0xa22df970,// 140 PAY 137 

    0x2a90c8f0,// 141 PAY 138 

    0x6dc31282,// 142 PAY 139 

    0x8d1597ec,// 143 PAY 140 

    0x15212013,// 144 PAY 141 

    0xba4eb2de,// 145 PAY 142 

    0xa2707674,// 146 PAY 143 

    0xe0ce3b5c,// 147 PAY 144 

    0xf109400d,// 148 PAY 145 

    0x9cf96d32,// 149 PAY 146 

    0x9dbd1dc6,// 150 PAY 147 

    0x8b692d9f,// 151 PAY 148 

    0x996da97b,// 152 PAY 149 

    0x0efda48a,// 153 PAY 150 

    0xcae8a059,// 154 PAY 151 

    0xa27f3d92,// 155 PAY 152 

    0x2192016f,// 156 PAY 153 

    0x3d20ef65,// 157 PAY 154 

    0xa9e7faf5,// 158 PAY 155 

    0xfc617cb3,// 159 PAY 156 

    0x350b9baa,// 160 PAY 157 

    0x6b781076,// 161 PAY 158 

    0x1927911e,// 162 PAY 159 

    0xe851f8f8,// 163 PAY 160 

    0x23a98e5a,// 164 PAY 161 

    0x4de956b3,// 165 PAY 162 

    0x6675f404,// 166 PAY 163 

    0x5c3a7cac,// 167 PAY 164 

    0x9824cc67,// 168 PAY 165 

    0xc87c5357,// 169 PAY 166 

    0x9db729f9,// 170 PAY 167 

    0xa7c7650f,// 171 PAY 168 

    0x2364d5a9,// 172 PAY 169 

    0x72ad3e53,// 173 PAY 170 

    0xc66184ca,// 174 PAY 171 

    0xea6ae791,// 175 PAY 172 

    0x2b88423c,// 176 PAY 173 

    0xeb969def,// 177 PAY 174 

    0x02810c71,// 178 PAY 175 

    0xbdfd7e6d,// 179 PAY 176 

    0x8b6e33e0,// 180 PAY 177 

    0x5ed2a582,// 181 PAY 178 

    0x7ab8943c,// 182 PAY 179 

    0xb7d623c8,// 183 PAY 180 

    0x2381f7ae,// 184 PAY 181 

    0xaa006fcb,// 185 PAY 182 

    0x2aca73b8,// 186 PAY 183 

    0xd145258a,// 187 PAY 184 

    0xd85d0242,// 188 PAY 185 

    0xc4d9180b,// 189 PAY 186 

    0x63e2ac5c,// 190 PAY 187 

    0x9f74f83b,// 191 PAY 188 

    0x9acbd520,// 192 PAY 189 

    0x5777cb2d,// 193 PAY 190 

    0xfc0a1dab,// 194 PAY 191 

    0xde033032,// 195 PAY 192 

    0x8ab92833,// 196 PAY 193 

    0x0d048f83,// 197 PAY 194 

    0xf9a90fc1,// 198 PAY 195 

    0x3501139e,// 199 PAY 196 

    0xe00463be,// 200 PAY 197 

    0xde3308d0,// 201 PAY 198 

    0xa200cb7b,// 202 PAY 199 

    0xb7c2c1d3,// 203 PAY 200 

    0x2570a56c,// 204 PAY 201 

    0xdf35a264,// 205 PAY 202 

    0x853ed70d,// 206 PAY 203 

    0xd2a8830d,// 207 PAY 204 

    0x1160fc21,// 208 PAY 205 

    0x4ce74a7a,// 209 PAY 206 

    0xc2ba26ba,// 210 PAY 207 

    0xff9629dc,// 211 PAY 208 

    0xb9eb943f,// 212 PAY 209 

    0x2b719f3e,// 213 PAY 210 

    0x71063df3,// 214 PAY 211 

    0xfedc6920,// 215 PAY 212 

    0xadbd096e,// 216 PAY 213 

    0x5d1837bd,// 217 PAY 214 

    0xa24d5c07,// 218 PAY 215 

    0x71450c0c,// 219 PAY 216 

    0x15ad50db,// 220 PAY 217 

    0x61e30b17,// 221 PAY 218 

    0x392aae2a,// 222 PAY 219 

    0xb0080cd3,// 223 PAY 220 

    0xc050ccd9,// 224 PAY 221 

    0x8cf59bcf,// 225 PAY 222 

    0xc9a6a298,// 226 PAY 223 

    0x21f72e28,// 227 PAY 224 

    0xe312411e,// 228 PAY 225 

    0x9b67f630,// 229 PAY 226 

    0x796ff2ee,// 230 PAY 227 

    0x0c338a9c,// 231 PAY 228 

    0x542b9675,// 232 PAY 229 

    0xc2f449cf,// 233 PAY 230 

    0x150b61d3,// 234 PAY 231 

    0x744b9cd0,// 235 PAY 232 

    0xdf64042c,// 236 PAY 233 

    0x24d5be6c,// 237 PAY 234 

    0x18670690,// 238 PAY 235 

    0x72cdb890,// 239 PAY 236 

    0x2a7a9a32,// 240 PAY 237 

    0xde6dc020,// 241 PAY 238 

    0x4d2cb331,// 242 PAY 239 

    0xa6111a02,// 243 PAY 240 

    0xcba00077,// 244 PAY 241 

    0x3cea937f,// 245 PAY 242 

    0xb6dbffe4,// 246 PAY 243 

    0xe185d49d,// 247 PAY 244 

    0x203b7be6,// 248 PAY 245 

    0x57c800a3,// 249 PAY 246 

    0x05534371,// 250 PAY 247 

    0x7fbdebf3,// 251 PAY 248 

    0x21761c34,// 252 PAY 249 

    0x640b2b96,// 253 PAY 250 

    0x263ffa9d,// 254 PAY 251 

    0xdd91ca98,// 255 PAY 252 

    0x22762e0c,// 256 PAY 253 

    0xb6965bfd,// 257 PAY 254 

    0xc34d63d4,// 258 PAY 255 

    0xa50227d2,// 259 PAY 256 

    0x8d91f0ce,// 260 PAY 257 

    0x9758edf9,// 261 PAY 258 

    0xa9049b54,// 262 PAY 259 

    0xf10eb311,// 263 PAY 260 

    0x258a24f4,// 264 PAY 261 

    0x01aca27a,// 265 PAY 262 

    0x91864d1a,// 266 PAY 263 

    0xba83aee2,// 267 PAY 264 

    0x6fc71baf,// 268 PAY 265 

    0x42e99a96,// 269 PAY 266 

    0x4726736a,// 270 PAY 267 

    0xbb2e5108,// 271 PAY 268 

    0xda13aaac,// 272 PAY 269 

    0x6fce0a3e,// 273 PAY 270 

    0x2886eb68,// 274 PAY 271 

    0x82da17f7,// 275 PAY 272 

    0x48beb3cd,// 276 PAY 273 

    0x5eea6033,// 277 PAY 274 

    0x365521f5,// 278 PAY 275 

    0x015dd40a,// 279 PAY 276 

    0x0eea7dc6,// 280 PAY 277 

    0x7fa3949d,// 281 PAY 278 

    0xde551d52,// 282 PAY 279 

    0x739c5ca4,// 283 PAY 280 

    0x7df26ef2,// 284 PAY 281 

    0x74ffa896,// 285 PAY 282 

    0x4331d89a,// 286 PAY 283 

    0x7dbc09a0,// 287 PAY 284 

    0xc28fa981,// 288 PAY 285 

    0x42e8f73d,// 289 PAY 286 

    0x43cb69b4,// 290 PAY 287 

    0xc0b7421b,// 291 PAY 288 

    0x2eb5fe59,// 292 PAY 289 

    0x56590717,// 293 PAY 290 

    0x0a7ef424,// 294 PAY 291 

    0xe01dfc8d,// 295 PAY 292 

    0x13d4e9f7,// 296 PAY 293 

    0xc7394764,// 297 PAY 294 

    0xed7c8f6a,// 298 PAY 295 

    0x0996a4cd,// 299 PAY 296 

    0x11f3b0f0,// 300 PAY 297 

    0x2a6720ed,// 301 PAY 298 

    0x6f3ce026,// 302 PAY 299 

    0xbeaca972,// 303 PAY 300 

    0xaa3bce02,// 304 PAY 301 

    0x00c54c9c,// 305 PAY 302 

    0xd7b39983,// 306 PAY 303 

    0xfeafb6f4,// 307 PAY 304 

    0xb7224df8,// 308 PAY 305 

    0xb2a01604,// 309 PAY 306 

    0x1abae8eb,// 310 PAY 307 

    0x8e6f4512,// 311 PAY 308 

    0xc576693b,// 312 PAY 309 

    0x028468a2,// 313 PAY 310 

    0x00733a41,// 314 PAY 311 

    0xe612a0b4,// 315 PAY 312 

    0x592bfa14,// 316 PAY 313 

    0x3f312c6a,// 317 PAY 314 

    0xb0819426,// 318 PAY 315 

    0x01551127,// 319 PAY 316 

    0xd88252b2,// 320 PAY 317 

    0xa89c2afd,// 321 PAY 318 

    0xf327ae9e,// 322 PAY 319 

    0x7e65454d,// 323 PAY 320 

    0x4c46849e,// 324 PAY 321 

    0x18a46198,// 325 PAY 322 

    0x81b771b7,// 326 PAY 323 

    0x5916668b,// 327 PAY 324 

    0x94794211,// 328 PAY 325 

    0x494af2f6,// 329 PAY 326 

    0xdded6dc6,// 330 PAY 327 

    0x8ddaad4a,// 331 PAY 328 

    0x8ea71550,// 332 PAY 329 

    0x301331bd,// 333 PAY 330 

    0x5431392f,// 334 PAY 331 

    0x80b2e19a,// 335 PAY 332 

    0xace18dee,// 336 PAY 333 

    0xf55d9475,// 337 PAY 334 

    0xd1e7b5c7,// 338 PAY 335 

    0xd0d7a0f0,// 339 PAY 336 

    0xc5672787,// 340 PAY 337 

    0x27aebde2,// 341 PAY 338 

    0xf224ee94,// 342 PAY 339 

    0x3ee8b048,// 343 PAY 340 

    0x42651852,// 344 PAY 341 

    0x5edd75bb,// 345 PAY 342 

    0x1622b050,// 346 PAY 343 

    0xfb4563c6,// 347 PAY 344 

    0x370eaa38,// 348 PAY 345 

    0xdb7c2fc9,// 349 PAY 346 

    0xa5479248,// 350 PAY 347 

    0xb244ebd4,// 351 PAY 348 

    0xfb9f183e,// 352 PAY 349 

    0x45bb07d7,// 353 PAY 350 

    0xbb457979,// 354 PAY 351 

    0xb75839fd,// 355 PAY 352 

    0x179a7327,// 356 PAY 353 

    0x498d22a9,// 357 PAY 354 

    0x11afc316,// 358 PAY 355 

    0x64f5fe3f,// 359 PAY 356 

    0x7357e44e,// 360 PAY 357 

    0xc227d28d,// 361 PAY 358 

    0x879d98f7,// 362 PAY 359 

    0x3d029a42,// 363 PAY 360 

    0x5c04eaff,// 364 PAY 361 

    0x9280271b,// 365 PAY 362 

    0x41cfa267,// 366 PAY 363 

    0x88380466,// 367 PAY 364 

    0xc22e2638,// 368 PAY 365 

    0x4616b4ab,// 369 PAY 366 

    0xecdd5af7,// 370 PAY 367 

    0xb03e9e7b,// 371 PAY 368 

    0xf6ba4094,// 372 PAY 369 

    0x7092eafa,// 373 PAY 370 

    0x63d79767,// 374 PAY 371 

    0xe657bff9,// 375 PAY 372 

    0x95415bad,// 376 PAY 373 

    0x44930999,// 377 PAY 374 

    0x60c8e62a,// 378 PAY 375 

    0xf1fb2ed1,// 379 PAY 376 

    0xcbc33e52,// 380 PAY 377 

    0x5a2109f6,// 381 PAY 378 

    0xb6290f48,// 382 PAY 379 

    0x086218ad,// 383 PAY 380 

    0xcc27e0b7,// 384 PAY 381 

    0xa7edee42,// 385 PAY 382 

    0x15b7de73,// 386 PAY 383 

    0x78ba9c5c,// 387 PAY 384 

    0x989dcb2f,// 388 PAY 385 

    0x104c5d7f,// 389 PAY 386 

    0xa132e5c0,// 390 PAY 387 

    0xba0188ce,// 391 PAY 388 

    0x3398ce07,// 392 PAY 389 

    0x83c63447,// 393 PAY 390 

    0x932cf248,// 394 PAY 391 

    0x2f550b7b,// 395 PAY 392 

    0x47780c7f,// 396 PAY 393 

    0x51e1f472,// 397 PAY 394 

    0x9b2633d1,// 398 PAY 395 

    0x87b5d4f5,// 399 PAY 396 

    0x7a189d74,// 400 PAY 397 

    0x5d27403d,// 401 PAY 398 

    0xed46234f,// 402 PAY 399 

    0xeb9fcb4a,// 403 PAY 400 

    0xd93ae09a,// 404 PAY 401 

    0x159b02c8,// 405 PAY 402 

    0x9cf3f7b9,// 406 PAY 403 

    0xc6fef43b,// 407 PAY 404 

    0x613f3f5b,// 408 PAY 405 

    0x772a5491,// 409 PAY 406 

    0x29787519,// 410 PAY 407 

    0x11df8459,// 411 PAY 408 

    0xc442d45e,// 412 PAY 409 

    0x47d60000,// 413 PAY 410 

/// HASH is  8 bytes 

    0x4de956b3,// 414 HSH   1 

    0x6675f404,// 415 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 208 

/// STA pkt_idx        : 48 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5f 

    0x00c05fd0 // 416 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt40_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 156 words. 

/// BDA size     is 620 (0x26c) 

/// BDA id       is 0xd94d 

    0x026cd94d,// 3 BDA   1 

/// PAY Generic Data size   : 620 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x075384a8,// 4 PAY   1 

    0x19234baa,// 5 PAY   2 

    0xf83613f2,// 6 PAY   3 

    0x92f3451e,// 7 PAY   4 

    0x7f79d209,// 8 PAY   5 

    0x9929350e,// 9 PAY   6 

    0x9266537e,// 10 PAY   7 

    0x1bc33237,// 11 PAY   8 

    0x8e1d01da,// 12 PAY   9 

    0xc972ce7e,// 13 PAY  10 

    0x449d4d3d,// 14 PAY  11 

    0xee7d3023,// 15 PAY  12 

    0x673e5e66,// 16 PAY  13 

    0x74b01187,// 17 PAY  14 

    0xd7128cf0,// 18 PAY  15 

    0x7e652ee6,// 19 PAY  16 

    0x4a83a48e,// 20 PAY  17 

    0xd5fa584c,// 21 PAY  18 

    0xf17c3e5a,// 22 PAY  19 

    0x436f03be,// 23 PAY  20 

    0xadd24fb7,// 24 PAY  21 

    0x4703cc08,// 25 PAY  22 

    0x91684bed,// 26 PAY  23 

    0xf1bb333a,// 27 PAY  24 

    0x7eac0f63,// 28 PAY  25 

    0xff0e0abe,// 29 PAY  26 

    0x9eec80af,// 30 PAY  27 

    0x7b8d9555,// 31 PAY  28 

    0x257e24cc,// 32 PAY  29 

    0x89afb42c,// 33 PAY  30 

    0x102dde2a,// 34 PAY  31 

    0x4114e332,// 35 PAY  32 

    0x4b576624,// 36 PAY  33 

    0x2fb7c050,// 37 PAY  34 

    0x6af28db9,// 38 PAY  35 

    0x5b4f0c3b,// 39 PAY  36 

    0x0b817188,// 40 PAY  37 

    0x38d0d8df,// 41 PAY  38 

    0x573560b6,// 42 PAY  39 

    0xcb422060,// 43 PAY  40 

    0xa0e8bcc2,// 44 PAY  41 

    0x8a99f6ad,// 45 PAY  42 

    0x33d997d9,// 46 PAY  43 

    0xc481042c,// 47 PAY  44 

    0x6698e850,// 48 PAY  45 

    0x96020607,// 49 PAY  46 

    0x9396a620,// 50 PAY  47 

    0xe790de9b,// 51 PAY  48 

    0x5637b632,// 52 PAY  49 

    0x5983082c,// 53 PAY  50 

    0x1eb62e30,// 54 PAY  51 

    0xe5f1c179,// 55 PAY  52 

    0xf7af0a39,// 56 PAY  53 

    0xae181fb8,// 57 PAY  54 

    0x7e59d283,// 58 PAY  55 

    0x87f1ce62,// 59 PAY  56 

    0x1a74cd16,// 60 PAY  57 

    0x355f9cf5,// 61 PAY  58 

    0xd1be7b8f,// 62 PAY  59 

    0x3320fe8e,// 63 PAY  60 

    0xc27245d9,// 64 PAY  61 

    0xd8aa0eba,// 65 PAY  62 

    0xa0d0d195,// 66 PAY  63 

    0xee1fa3bc,// 67 PAY  64 

    0x7a8aa1db,// 68 PAY  65 

    0x15df5880,// 69 PAY  66 

    0xfac994dc,// 70 PAY  67 

    0x6ea9038f,// 71 PAY  68 

    0x8ceeab39,// 72 PAY  69 

    0x071aa154,// 73 PAY  70 

    0xc69075f3,// 74 PAY  71 

    0x66f2e3c8,// 75 PAY  72 

    0x4ef51fa5,// 76 PAY  73 

    0x0d718378,// 77 PAY  74 

    0xcfbec60b,// 78 PAY  75 

    0xf5a35b0c,// 79 PAY  76 

    0x17fa8187,// 80 PAY  77 

    0x541b5b93,// 81 PAY  78 

    0x45e88be9,// 82 PAY  79 

    0x81c77d1a,// 83 PAY  80 

    0x71344bb6,// 84 PAY  81 

    0x9cd45da4,// 85 PAY  82 

    0x3805038b,// 86 PAY  83 

    0x052e8f84,// 87 PAY  84 

    0xcf4e0f53,// 88 PAY  85 

    0x07f43898,// 89 PAY  86 

    0x20323dde,// 90 PAY  87 

    0xb43b9857,// 91 PAY  88 

    0x7a636b28,// 92 PAY  89 

    0x9257fac9,// 93 PAY  90 

    0xc967d378,// 94 PAY  91 

    0xd06c78c0,// 95 PAY  92 

    0x2b3e871a,// 96 PAY  93 

    0x1ee41e1b,// 97 PAY  94 

    0xaebcd8ce,// 98 PAY  95 

    0x8418d6dd,// 99 PAY  96 

    0x8625ef5a,// 100 PAY  97 

    0x31c5cf17,// 101 PAY  98 

    0xed951a08,// 102 PAY  99 

    0x704da581,// 103 PAY 100 

    0x9576c1a6,// 104 PAY 101 

    0x5565d2e5,// 105 PAY 102 

    0xd32d9f6b,// 106 PAY 103 

    0x808e8dfa,// 107 PAY 104 

    0xeb8e28b5,// 108 PAY 105 

    0xa6011388,// 109 PAY 106 

    0xcbef1ed7,// 110 PAY 107 

    0x7fbfbcdf,// 111 PAY 108 

    0x0dae2ba1,// 112 PAY 109 

    0x05d2bb13,// 113 PAY 110 

    0x183d9040,// 114 PAY 111 

    0x04229b59,// 115 PAY 112 

    0x667c0178,// 116 PAY 113 

    0x97590318,// 117 PAY 114 

    0xbd1d1ce7,// 118 PAY 115 

    0xfcec4db1,// 119 PAY 116 

    0x9c9ac63c,// 120 PAY 117 

    0x42ddca50,// 121 PAY 118 

    0x79222d12,// 122 PAY 119 

    0x44a9b470,// 123 PAY 120 

    0x56be7cca,// 124 PAY 121 

    0xd4ada77f,// 125 PAY 122 

    0x1edbb7ec,// 126 PAY 123 

    0xe2ab521d,// 127 PAY 124 

    0xd75461f4,// 128 PAY 125 

    0x0b6f8d96,// 129 PAY 126 

    0xcf0c52d6,// 130 PAY 127 

    0xb1b42baa,// 131 PAY 128 

    0xacafbe95,// 132 PAY 129 

    0x2cd63657,// 133 PAY 130 

    0x8f995aa0,// 134 PAY 131 

    0x04e42d9e,// 135 PAY 132 

    0x616e8c45,// 136 PAY 133 

    0x1afeba4a,// 137 PAY 134 

    0x2b44bf29,// 138 PAY 135 

    0x008e3134,// 139 PAY 136 

    0x5e396341,// 140 PAY 137 

    0x3b54f090,// 141 PAY 138 

    0x7aab07bb,// 142 PAY 139 

    0x3afced4d,// 143 PAY 140 

    0xa6428b8d,// 144 PAY 141 

    0x71c7467c,// 145 PAY 142 

    0xd0670813,// 146 PAY 143 

    0xe7fcaa95,// 147 PAY 144 

    0xd313ab63,// 148 PAY 145 

    0x2715f1ed,// 149 PAY 146 

    0x6ec3b1d6,// 150 PAY 147 

    0xb8f9c968,// 151 PAY 148 

    0x3171b91e,// 152 PAY 149 

    0xce3aa591,// 153 PAY 150 

    0x36694164,// 154 PAY 151 

    0x7c55e7a1,// 155 PAY 152 

    0x1595ceb8,// 156 PAY 153 

    0xebd1e9db,// 157 PAY 154 

    0x8d5949a5,// 158 PAY 155 

/// HASH is  20 bytes 

    0x9257fac9,// 159 HSH   1 

    0xc967d378,// 160 HSH   2 

    0xd06c78c0,// 161 HSH   3 

    0x2b3e871a,// 162 HSH   4 

    0x1ee41e1b,// 163 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 120 

/// STA pkt_idx        : 243 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdb 

    0x03ccdb78 // 164 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt41_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 416 words. 

/// BDA size     is 1657 (0x679) 

/// BDA id       is 0x4f88 

    0x06794f88,// 3 BDA   1 

/// PAY Generic Data size   : 1657 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x723b2094,// 4 PAY   1 

    0xeff2abdf,// 5 PAY   2 

    0x65c04c8c,// 6 PAY   3 

    0x595e6b90,// 7 PAY   4 

    0xf9edab8c,// 8 PAY   5 

    0x0bf30441,// 9 PAY   6 

    0x3cb3ea35,// 10 PAY   7 

    0xcf4f59ed,// 11 PAY   8 

    0x5a9a0b75,// 12 PAY   9 

    0x4e3a5fe6,// 13 PAY  10 

    0x50a8309d,// 14 PAY  11 

    0x177b23ab,// 15 PAY  12 

    0xc25c3456,// 16 PAY  13 

    0xfe445a1d,// 17 PAY  14 

    0xecf49b39,// 18 PAY  15 

    0x46f7db3b,// 19 PAY  16 

    0xf3216f6d,// 20 PAY  17 

    0xfca6a228,// 21 PAY  18 

    0x37eb7a0c,// 22 PAY  19 

    0x87bbba57,// 23 PAY  20 

    0xb7ef432e,// 24 PAY  21 

    0x9d2fbd28,// 25 PAY  22 

    0x42571dee,// 26 PAY  23 

    0x0e3fa8a6,// 27 PAY  24 

    0xd056e47f,// 28 PAY  25 

    0x2b4d6f53,// 29 PAY  26 

    0xf4f9a600,// 30 PAY  27 

    0x7c9a71b0,// 31 PAY  28 

    0xf05f5c9d,// 32 PAY  29 

    0x986df09d,// 33 PAY  30 

    0x9612070a,// 34 PAY  31 

    0x846d9e03,// 35 PAY  32 

    0x3d3189d2,// 36 PAY  33 

    0xd69113c6,// 37 PAY  34 

    0x46e96321,// 38 PAY  35 

    0xcd77c506,// 39 PAY  36 

    0xeace73bb,// 40 PAY  37 

    0xc23db1a8,// 41 PAY  38 

    0x2d19c117,// 42 PAY  39 

    0xd2fbd9cf,// 43 PAY  40 

    0x7d002c60,// 44 PAY  41 

    0x5ef12e92,// 45 PAY  42 

    0x8f724ecb,// 46 PAY  43 

    0xd9cc35c3,// 47 PAY  44 

    0x98f028ed,// 48 PAY  45 

    0x416b4b00,// 49 PAY  46 

    0x7b657cd2,// 50 PAY  47 

    0x4eb42f55,// 51 PAY  48 

    0x42fd467f,// 52 PAY  49 

    0xf354fb0b,// 53 PAY  50 

    0x6f67871b,// 54 PAY  51 

    0x50fd03ca,// 55 PAY  52 

    0xf4c545fb,// 56 PAY  53 

    0x21f5ba41,// 57 PAY  54 

    0xb8912662,// 58 PAY  55 

    0x5c43b598,// 59 PAY  56 

    0x7db326d9,// 60 PAY  57 

    0xbd5d831d,// 61 PAY  58 

    0xbe47e475,// 62 PAY  59 

    0x9651b6a1,// 63 PAY  60 

    0x13d56bcb,// 64 PAY  61 

    0xb83c5db9,// 65 PAY  62 

    0xc4ba4013,// 66 PAY  63 

    0xbdf91fc5,// 67 PAY  64 

    0x04e975ae,// 68 PAY  65 

    0xa715f342,// 69 PAY  66 

    0x58fb5c59,// 70 PAY  67 

    0xb4a94ef4,// 71 PAY  68 

    0xb57808db,// 72 PAY  69 

    0xb9f0954f,// 73 PAY  70 

    0x7f0b1982,// 74 PAY  71 

    0xa51e715b,// 75 PAY  72 

    0x863b5a91,// 76 PAY  73 

    0x2a36e648,// 77 PAY  74 

    0xb8f3b5ae,// 78 PAY  75 

    0xd712b213,// 79 PAY  76 

    0x8897dbad,// 80 PAY  77 

    0xa6704e48,// 81 PAY  78 

    0xc180bca4,// 82 PAY  79 

    0xe42515e6,// 83 PAY  80 

    0x3b35331d,// 84 PAY  81 

    0x02ddc18d,// 85 PAY  82 

    0x72421405,// 86 PAY  83 

    0xaeeae95f,// 87 PAY  84 

    0x106a572a,// 88 PAY  85 

    0x1db52d42,// 89 PAY  86 

    0x9985c3e0,// 90 PAY  87 

    0xa1d7289f,// 91 PAY  88 

    0x9ff2ba5f,// 92 PAY  89 

    0x2e6b8845,// 93 PAY  90 

    0x402fa060,// 94 PAY  91 

    0x6539fdf1,// 95 PAY  92 

    0x6d371090,// 96 PAY  93 

    0xf62ca997,// 97 PAY  94 

    0xefbb6f81,// 98 PAY  95 

    0x1ac35c89,// 99 PAY  96 

    0xcbcff5c7,// 100 PAY  97 

    0xf8551b6e,// 101 PAY  98 

    0x01a54f62,// 102 PAY  99 

    0xa4df2dc3,// 103 PAY 100 

    0xa4ec7f89,// 104 PAY 101 

    0x128321e4,// 105 PAY 102 

    0x2f54cd4d,// 106 PAY 103 

    0xfba3329e,// 107 PAY 104 

    0x31ba9861,// 108 PAY 105 

    0xd2c08dd3,// 109 PAY 106 

    0x9a8633ff,// 110 PAY 107 

    0xc15ce407,// 111 PAY 108 

    0x751763c9,// 112 PAY 109 

    0xceefcd67,// 113 PAY 110 

    0xc1b030f9,// 114 PAY 111 

    0x671bb721,// 115 PAY 112 

    0x90461d18,// 116 PAY 113 

    0xd48b59e0,// 117 PAY 114 

    0xc982cf49,// 118 PAY 115 

    0x31a3af60,// 119 PAY 116 

    0xaf69cf47,// 120 PAY 117 

    0xbffd78a6,// 121 PAY 118 

    0x320646a9,// 122 PAY 119 

    0x90227be8,// 123 PAY 120 

    0xb985c8f7,// 124 PAY 121 

    0xe883fd4b,// 125 PAY 122 

    0x63946c94,// 126 PAY 123 

    0x3e375400,// 127 PAY 124 

    0xf47218ca,// 128 PAY 125 

    0x510daf7f,// 129 PAY 126 

    0x6842311b,// 130 PAY 127 

    0xb8e0fb23,// 131 PAY 128 

    0xa33f8bb9,// 132 PAY 129 

    0x52522ac6,// 133 PAY 130 

    0x78286c9c,// 134 PAY 131 

    0x0c30da94,// 135 PAY 132 

    0xc371708b,// 136 PAY 133 

    0xbee2106c,// 137 PAY 134 

    0x4b549321,// 138 PAY 135 

    0x65cc8007,// 139 PAY 136 

    0x27239179,// 140 PAY 137 

    0x941fe5bf,// 141 PAY 138 

    0xc54d0cfe,// 142 PAY 139 

    0x7c3b82cc,// 143 PAY 140 

    0xba01751d,// 144 PAY 141 

    0x845f9860,// 145 PAY 142 

    0x8aae209e,// 146 PAY 143 

    0x552c50f2,// 147 PAY 144 

    0x5f9fa005,// 148 PAY 145 

    0x89cb7401,// 149 PAY 146 

    0xa9259a39,// 150 PAY 147 

    0x4184ab5c,// 151 PAY 148 

    0x280cfe83,// 152 PAY 149 

    0x647c26fd,// 153 PAY 150 

    0x82b51a97,// 154 PAY 151 

    0x626e9faf,// 155 PAY 152 

    0xa205824d,// 156 PAY 153 

    0xb36377ae,// 157 PAY 154 

    0x4dc38c26,// 158 PAY 155 

    0x841db16d,// 159 PAY 156 

    0xe1f3d655,// 160 PAY 157 

    0xe69224a0,// 161 PAY 158 

    0xa56a69c8,// 162 PAY 159 

    0x19b66163,// 163 PAY 160 

    0x99b430d0,// 164 PAY 161 

    0x10eedaa7,// 165 PAY 162 

    0x4a8378db,// 166 PAY 163 

    0xe2a5077b,// 167 PAY 164 

    0xd21d4b2a,// 168 PAY 165 

    0xd81e38d0,// 169 PAY 166 

    0xbc12a989,// 170 PAY 167 

    0xb9a868c3,// 171 PAY 168 

    0x635bb610,// 172 PAY 169 

    0x5b7dd459,// 173 PAY 170 

    0x134beba6,// 174 PAY 171 

    0xfbef10be,// 175 PAY 172 

    0x6792b7ce,// 176 PAY 173 

    0x457b9c67,// 177 PAY 174 

    0xb4eb98ea,// 178 PAY 175 

    0xd93ad498,// 179 PAY 176 

    0x0ed7aa63,// 180 PAY 177 

    0x378af3e3,// 181 PAY 178 

    0x246da7e8,// 182 PAY 179 

    0x60c3bf9c,// 183 PAY 180 

    0xed8fc041,// 184 PAY 181 

    0x1cb6eea2,// 185 PAY 182 

    0x8ff67d24,// 186 PAY 183 

    0xa749f975,// 187 PAY 184 

    0x7e1ab472,// 188 PAY 185 

    0x452f26a5,// 189 PAY 186 

    0x8580144f,// 190 PAY 187 

    0xa2f8fdd6,// 191 PAY 188 

    0xb4b9e9fa,// 192 PAY 189 

    0xe9f02e92,// 193 PAY 190 

    0x40ebe005,// 194 PAY 191 

    0x657fb197,// 195 PAY 192 

    0x88ad682e,// 196 PAY 193 

    0x1d5e2613,// 197 PAY 194 

    0x05b12924,// 198 PAY 195 

    0x9200e2b0,// 199 PAY 196 

    0xc90a63c8,// 200 PAY 197 

    0x573ea73b,// 201 PAY 198 

    0xea80b8f3,// 202 PAY 199 

    0x9e3f0770,// 203 PAY 200 

    0xb8639ebc,// 204 PAY 201 

    0xed92b507,// 205 PAY 202 

    0x586b7b98,// 206 PAY 203 

    0x4a84c136,// 207 PAY 204 

    0xdc5b0d49,// 208 PAY 205 

    0x2093998b,// 209 PAY 206 

    0x496b136c,// 210 PAY 207 

    0x6a364770,// 211 PAY 208 

    0x7414ec14,// 212 PAY 209 

    0x98f614ea,// 213 PAY 210 

    0x09dc3dae,// 214 PAY 211 

    0xbdd35e24,// 215 PAY 212 

    0x34f17e3b,// 216 PAY 213 

    0x06fc1dba,// 217 PAY 214 

    0x83ea8f80,// 218 PAY 215 

    0x72547452,// 219 PAY 216 

    0xc8eb1eec,// 220 PAY 217 

    0x44336a51,// 221 PAY 218 

    0xde0b6ad6,// 222 PAY 219 

    0x69d18f87,// 223 PAY 220 

    0x46c3fcd0,// 224 PAY 221 

    0xfb43da8f,// 225 PAY 222 

    0xe2cb8ceb,// 226 PAY 223 

    0x74c195e2,// 227 PAY 224 

    0x340d854a,// 228 PAY 225 

    0xc72f3ea1,// 229 PAY 226 

    0xa8d21eb7,// 230 PAY 227 

    0x7303ac0a,// 231 PAY 228 

    0x2393c376,// 232 PAY 229 

    0x6844046d,// 233 PAY 230 

    0x465ff82f,// 234 PAY 231 

    0x37ab40b2,// 235 PAY 232 

    0x7d6656ed,// 236 PAY 233 

    0xa8b298d9,// 237 PAY 234 

    0x408b6a12,// 238 PAY 235 

    0xc6f24628,// 239 PAY 236 

    0x2fc62e33,// 240 PAY 237 

    0xa192778b,// 241 PAY 238 

    0x13ebace5,// 242 PAY 239 

    0x0085284a,// 243 PAY 240 

    0x88ef0ac2,// 244 PAY 241 

    0x2bbbafce,// 245 PAY 242 

    0x83b8cc0e,// 246 PAY 243 

    0xb2a9c31a,// 247 PAY 244 

    0x2b1427c4,// 248 PAY 245 

    0x69824443,// 249 PAY 246 

    0xf9513449,// 250 PAY 247 

    0x95db6ee7,// 251 PAY 248 

    0x13f84ec2,// 252 PAY 249 

    0x48038094,// 253 PAY 250 

    0x8e5775f9,// 254 PAY 251 

    0xde4f3a2d,// 255 PAY 252 

    0xeabb3574,// 256 PAY 253 

    0xb253c34f,// 257 PAY 254 

    0xdd1fab13,// 258 PAY 255 

    0xe210b23e,// 259 PAY 256 

    0x3255a8a1,// 260 PAY 257 

    0xa0b58898,// 261 PAY 258 

    0x4394f052,// 262 PAY 259 

    0xeac0213a,// 263 PAY 260 

    0xfd3e2632,// 264 PAY 261 

    0xfad57328,// 265 PAY 262 

    0xccc01b76,// 266 PAY 263 

    0x04b53b4c,// 267 PAY 264 

    0x75f5b7b4,// 268 PAY 265 

    0xb5a56f5c,// 269 PAY 266 

    0x1ca95ea9,// 270 PAY 267 

    0x9544d2f9,// 271 PAY 268 

    0xd7f2d3b8,// 272 PAY 269 

    0x419a5927,// 273 PAY 270 

    0x98a1c40f,// 274 PAY 271 

    0x9c03a582,// 275 PAY 272 

    0x81928413,// 276 PAY 273 

    0x94d3c29e,// 277 PAY 274 

    0xa415a90c,// 278 PAY 275 

    0x2fbbf626,// 279 PAY 276 

    0x94ec9d4a,// 280 PAY 277 

    0x73140ef0,// 281 PAY 278 

    0x36388186,// 282 PAY 279 

    0x2f760b03,// 283 PAY 280 

    0xdbcd6d38,// 284 PAY 281 

    0x874249f2,// 285 PAY 282 

    0x9d0e9500,// 286 PAY 283 

    0x65ee4efb,// 287 PAY 284 

    0x8b359578,// 288 PAY 285 

    0x40c152a6,// 289 PAY 286 

    0x3392567f,// 290 PAY 287 

    0x4568d00e,// 291 PAY 288 

    0x25b3fd25,// 292 PAY 289 

    0x6bf599fb,// 293 PAY 290 

    0x70fa504f,// 294 PAY 291 

    0xe1893820,// 295 PAY 292 

    0xcd9e0e9e,// 296 PAY 293 

    0xf30fab56,// 297 PAY 294 

    0xaf4ede84,// 298 PAY 295 

    0x2b200f1f,// 299 PAY 296 

    0xd5c67366,// 300 PAY 297 

    0x546dcd44,// 301 PAY 298 

    0x9dc8fcfa,// 302 PAY 299 

    0x1eceaf93,// 303 PAY 300 

    0x0cc503a7,// 304 PAY 301 

    0x43647930,// 305 PAY 302 

    0x32c28769,// 306 PAY 303 

    0x1ba63eeb,// 307 PAY 304 

    0x41b2a41b,// 308 PAY 305 

    0xa2b45ea6,// 309 PAY 306 

    0x4321a8f5,// 310 PAY 307 

    0xc15b1a06,// 311 PAY 308 

    0xf0528d15,// 312 PAY 309 

    0xb5b54bca,// 313 PAY 310 

    0xb8cb2cbf,// 314 PAY 311 

    0x9d5cacbe,// 315 PAY 312 

    0xd72e9175,// 316 PAY 313 

    0x9c1b997e,// 317 PAY 314 

    0xf5f7213d,// 318 PAY 315 

    0x8f8094de,// 319 PAY 316 

    0x13b0db4a,// 320 PAY 317 

    0x4a8a6c61,// 321 PAY 318 

    0x6fd00ed7,// 322 PAY 319 

    0xd3050f12,// 323 PAY 320 

    0xad7171f2,// 324 PAY 321 

    0x254b3c66,// 325 PAY 322 

    0x0f2e0264,// 326 PAY 323 

    0x508b5d96,// 327 PAY 324 

    0x5468db18,// 328 PAY 325 

    0xd94efff9,// 329 PAY 326 

    0xd704fe68,// 330 PAY 327 

    0xf775b442,// 331 PAY 328 

    0xb3086660,// 332 PAY 329 

    0x4f300e70,// 333 PAY 330 

    0x0b641801,// 334 PAY 331 

    0x185e4aee,// 335 PAY 332 

    0xe6d89cc8,// 336 PAY 333 

    0xa74d67cc,// 337 PAY 334 

    0x9fe2f1c9,// 338 PAY 335 

    0xe2f18e16,// 339 PAY 336 

    0x14a6c935,// 340 PAY 337 

    0xff60f44d,// 341 PAY 338 

    0x32dc9e3c,// 342 PAY 339 

    0xccea2558,// 343 PAY 340 

    0x3f80abc1,// 344 PAY 341 

    0xb887dbe5,// 345 PAY 342 

    0xac689cce,// 346 PAY 343 

    0xea63be28,// 347 PAY 344 

    0xc944481e,// 348 PAY 345 

    0x5b708629,// 349 PAY 346 

    0x0fdced9a,// 350 PAY 347 

    0xa7b24777,// 351 PAY 348 

    0xf35b72d7,// 352 PAY 349 

    0xf9fab108,// 353 PAY 350 

    0xe54ad67c,// 354 PAY 351 

    0x8eede568,// 355 PAY 352 

    0x87936013,// 356 PAY 353 

    0x636bf4cd,// 357 PAY 354 

    0x1fdf429a,// 358 PAY 355 

    0x18cd6bf0,// 359 PAY 356 

    0x837d3dcf,// 360 PAY 357 

    0x269b53df,// 361 PAY 358 

    0x519cbcc8,// 362 PAY 359 

    0x1d75ae91,// 363 PAY 360 

    0x1d59113a,// 364 PAY 361 

    0x9d10a68a,// 365 PAY 362 

    0xc6f8090b,// 366 PAY 363 

    0x77ba9787,// 367 PAY 364 

    0x249662e1,// 368 PAY 365 

    0xd056d907,// 369 PAY 366 

    0x33ebcc02,// 370 PAY 367 

    0x1f125093,// 371 PAY 368 

    0x9a3c2555,// 372 PAY 369 

    0x3010cf8d,// 373 PAY 370 

    0x880bee83,// 374 PAY 371 

    0x64d7fe8c,// 375 PAY 372 

    0xda57bd29,// 376 PAY 373 

    0x03399c68,// 377 PAY 374 

    0x616a361e,// 378 PAY 375 

    0x5274d41c,// 379 PAY 376 

    0xd8c648ff,// 380 PAY 377 

    0x970ecf15,// 381 PAY 378 

    0x03c3e089,// 382 PAY 379 

    0x09974a18,// 383 PAY 380 

    0xe57b6738,// 384 PAY 381 

    0xb6e9329e,// 385 PAY 382 

    0xb5ae25a8,// 386 PAY 383 

    0x57920b58,// 387 PAY 384 

    0x77eca008,// 388 PAY 385 

    0x034fd45e,// 389 PAY 386 

    0xe0dae201,// 390 PAY 387 

    0xf09b598a,// 391 PAY 388 

    0x208e2431,// 392 PAY 389 

    0x012f8dcc,// 393 PAY 390 

    0x419c18be,// 394 PAY 391 

    0x0ad11b4f,// 395 PAY 392 

    0xbcf60d2b,// 396 PAY 393 

    0x2ad5d77d,// 397 PAY 394 

    0x8636600d,// 398 PAY 395 

    0xdf820c1c,// 399 PAY 396 

    0x2a4a5a2e,// 400 PAY 397 

    0x2e0c7eb4,// 401 PAY 398 

    0x5d9a7ab5,// 402 PAY 399 

    0x3075e168,// 403 PAY 400 

    0xa877d249,// 404 PAY 401 

    0x819a62c4,// 405 PAY 402 

    0xa3ae7264,// 406 PAY 403 

    0x73d0e487,// 407 PAY 404 

    0xc41ce11a,// 408 PAY 405 

    0x600b7efd,// 409 PAY 406 

    0xf83f693d,// 410 PAY 407 

    0x6cd45fee,// 411 PAY 408 

    0x4596ed1f,// 412 PAY 409 

    0xf2550c1e,// 413 PAY 410 

    0xf9108178,// 414 PAY 411 

    0x6193b7c4,// 415 PAY 412 

    0x739a5681,// 416 PAY 413 

    0xea406fca,// 417 PAY 414 

    0x3b000000,// 418 PAY 415 

/// STA is 1 words. 

/// STA num_pkts       : 154 

/// STA pkt_idx        : 101 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4a 

    0x01954a9a // 419 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt42_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 440 words. 

/// BDA size     is 1753 (0x6d9) 

/// BDA id       is 0x80ec 

    0x06d980ec,// 3 BDA   1 

/// PAY Generic Data size   : 1753 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x606442f4,// 4 PAY   1 

    0xbd82c5c1,// 5 PAY   2 

    0x69d399a4,// 6 PAY   3 

    0x2435d2fe,// 7 PAY   4 

    0x2ae65844,// 8 PAY   5 

    0x87fec28b,// 9 PAY   6 

    0x8e8f84ec,// 10 PAY   7 

    0x58c1ae6a,// 11 PAY   8 

    0xad5b7570,// 12 PAY   9 

    0x1470f8f8,// 13 PAY  10 

    0xfc62aefd,// 14 PAY  11 

    0xf8f88932,// 15 PAY  12 

    0xed17fbb6,// 16 PAY  13 

    0x62170f20,// 17 PAY  14 

    0x2eeb63a8,// 18 PAY  15 

    0x0f7ccb85,// 19 PAY  16 

    0xe53d7678,// 20 PAY  17 

    0xbb0d7fe4,// 21 PAY  18 

    0x75fb2de9,// 22 PAY  19 

    0xbaa84cf9,// 23 PAY  20 

    0x8e2a0e48,// 24 PAY  21 

    0x2aebd23e,// 25 PAY  22 

    0x3d476af6,// 26 PAY  23 

    0xddd0c8c3,// 27 PAY  24 

    0x6e56891f,// 28 PAY  25 

    0x95575c72,// 29 PAY  26 

    0x1a394e48,// 30 PAY  27 

    0x23a4db1f,// 31 PAY  28 

    0xd5b3ca05,// 32 PAY  29 

    0x0c27e32e,// 33 PAY  30 

    0x6d91bebb,// 34 PAY  31 

    0x059e7637,// 35 PAY  32 

    0x9627aa39,// 36 PAY  33 

    0x9a9fd5eb,// 37 PAY  34 

    0x88d3eace,// 38 PAY  35 

    0xbe475f02,// 39 PAY  36 

    0x27b3c6de,// 40 PAY  37 

    0x821e51a9,// 41 PAY  38 

    0x28e7b201,// 42 PAY  39 

    0x1a304335,// 43 PAY  40 

    0x7ebf4c1f,// 44 PAY  41 

    0xd3499f86,// 45 PAY  42 

    0xa36e08c6,// 46 PAY  43 

    0x96f30dfc,// 47 PAY  44 

    0xa0781fe9,// 48 PAY  45 

    0xd97cafb9,// 49 PAY  46 

    0xb852ea56,// 50 PAY  47 

    0x54b52cd7,// 51 PAY  48 

    0x5c522647,// 52 PAY  49 

    0x80989cef,// 53 PAY  50 

    0x0be5c9fb,// 54 PAY  51 

    0x8332218a,// 55 PAY  52 

    0x65751170,// 56 PAY  53 

    0x052ec9b5,// 57 PAY  54 

    0xd83221f0,// 58 PAY  55 

    0xcab13d49,// 59 PAY  56 

    0x74b4c246,// 60 PAY  57 

    0x2fea38ad,// 61 PAY  58 

    0xbfaab0a1,// 62 PAY  59 

    0x4852517f,// 63 PAY  60 

    0x79e9c175,// 64 PAY  61 

    0x60a52769,// 65 PAY  62 

    0x03c7015f,// 66 PAY  63 

    0xe8f972c9,// 67 PAY  64 

    0x7a769451,// 68 PAY  65 

    0xab85c362,// 69 PAY  66 

    0x828c5481,// 70 PAY  67 

    0x49e908e2,// 71 PAY  68 

    0x5c6be753,// 72 PAY  69 

    0x08f9db1a,// 73 PAY  70 

    0x776df91d,// 74 PAY  71 

    0x9b82f644,// 75 PAY  72 

    0x2a51273a,// 76 PAY  73 

    0xc2eb522d,// 77 PAY  74 

    0x4d13b374,// 78 PAY  75 

    0x1da02d85,// 79 PAY  76 

    0xa7eb9fc3,// 80 PAY  77 

    0x18164c29,// 81 PAY  78 

    0xd09ab380,// 82 PAY  79 

    0xd8670dc7,// 83 PAY  80 

    0x29b2142c,// 84 PAY  81 

    0xc57104e4,// 85 PAY  82 

    0x52f6fafd,// 86 PAY  83 

    0xc4587b88,// 87 PAY  84 

    0x3444da40,// 88 PAY  85 

    0x9e8a1889,// 89 PAY  86 

    0x6d172349,// 90 PAY  87 

    0xc89371c6,// 91 PAY  88 

    0x4af740d9,// 92 PAY  89 

    0xa09d7067,// 93 PAY  90 

    0xf55b827e,// 94 PAY  91 

    0xa67396d3,// 95 PAY  92 

    0x03b8f689,// 96 PAY  93 

    0x58ac7447,// 97 PAY  94 

    0x411e8840,// 98 PAY  95 

    0xc891a638,// 99 PAY  96 

    0x68352ffc,// 100 PAY  97 

    0xb3fd5412,// 101 PAY  98 

    0x0f4be509,// 102 PAY  99 

    0xfc2d704d,// 103 PAY 100 

    0x5ff84f1d,// 104 PAY 101 

    0x73535d3c,// 105 PAY 102 

    0x9d92ca43,// 106 PAY 103 

    0x75cf9c9b,// 107 PAY 104 

    0x6c672727,// 108 PAY 105 

    0x5e1d5a06,// 109 PAY 106 

    0x15523cd9,// 110 PAY 107 

    0xb318916f,// 111 PAY 108 

    0x1a2eff32,// 112 PAY 109 

    0xb143005e,// 113 PAY 110 

    0x33891402,// 114 PAY 111 

    0x58f8561b,// 115 PAY 112 

    0xd04a5c5e,// 116 PAY 113 

    0x5b4ed414,// 117 PAY 114 

    0x4cb381a3,// 118 PAY 115 

    0xdadb4fc1,// 119 PAY 116 

    0xe9392d33,// 120 PAY 117 

    0x672dd52e,// 121 PAY 118 

    0xe5f748af,// 122 PAY 119 

    0x2dd90b2e,// 123 PAY 120 

    0xc838bff6,// 124 PAY 121 

    0x9317accc,// 125 PAY 122 

    0x2ef95bf5,// 126 PAY 123 

    0x093b247a,// 127 PAY 124 

    0x251628f6,// 128 PAY 125 

    0xa36d3dd7,// 129 PAY 126 

    0x4b523195,// 130 PAY 127 

    0x735e52e9,// 131 PAY 128 

    0x79ff89ad,// 132 PAY 129 

    0xe6b5e732,// 133 PAY 130 

    0x2f0beabe,// 134 PAY 131 

    0x33659423,// 135 PAY 132 

    0x61bed9d7,// 136 PAY 133 

    0x4a0990e0,// 137 PAY 134 

    0x770ac170,// 138 PAY 135 

    0x738f3442,// 139 PAY 136 

    0x5192ea17,// 140 PAY 137 

    0xdb570626,// 141 PAY 138 

    0xe7e18f13,// 142 PAY 139 

    0x4997e9bd,// 143 PAY 140 

    0xb54ed83e,// 144 PAY 141 

    0xbe9877b5,// 145 PAY 142 

    0x3bc03980,// 146 PAY 143 

    0x69dac4ff,// 147 PAY 144 

    0xda700527,// 148 PAY 145 

    0x35ed8851,// 149 PAY 146 

    0x4cdb5f0d,// 150 PAY 147 

    0xd220030d,// 151 PAY 148 

    0xea525b2a,// 152 PAY 149 

    0xb3b61f25,// 153 PAY 150 

    0x8753929c,// 154 PAY 151 

    0x56482f98,// 155 PAY 152 

    0x98195ef3,// 156 PAY 153 

    0xb2efd6b4,// 157 PAY 154 

    0x32cf0272,// 158 PAY 155 

    0xe312050d,// 159 PAY 156 

    0x337eb2f1,// 160 PAY 157 

    0x7f8fa580,// 161 PAY 158 

    0x7d5662fe,// 162 PAY 159 

    0x9cd260ed,// 163 PAY 160 

    0x242ff5c5,// 164 PAY 161 

    0x4c34bcb9,// 165 PAY 162 

    0xb37fbf65,// 166 PAY 163 

    0x4a86c39f,// 167 PAY 164 

    0x2b50e0d0,// 168 PAY 165 

    0x9f59b91c,// 169 PAY 166 

    0xf0bb98db,// 170 PAY 167 

    0xca10c78b,// 171 PAY 168 

    0xfb4f5f5a,// 172 PAY 169 

    0xd71a630a,// 173 PAY 170 

    0x419b4365,// 174 PAY 171 

    0x840b11ba,// 175 PAY 172 

    0xf089c056,// 176 PAY 173 

    0x7bc67ff1,// 177 PAY 174 

    0x2d4deac5,// 178 PAY 175 

    0x323125f4,// 179 PAY 176 

    0x49ca802e,// 180 PAY 177 

    0xd2899370,// 181 PAY 178 

    0xad7da62f,// 182 PAY 179 

    0x5ad2c325,// 183 PAY 180 

    0x78ca9f29,// 184 PAY 181 

    0x8d620b34,// 185 PAY 182 

    0x820ac6ff,// 186 PAY 183 

    0xfd8f49ac,// 187 PAY 184 

    0x0fe758d6,// 188 PAY 185 

    0x3510143b,// 189 PAY 186 

    0x954ca9ba,// 190 PAY 187 

    0xc148000f,// 191 PAY 188 

    0x5fffd5da,// 192 PAY 189 

    0xae8709c6,// 193 PAY 190 

    0xce6eb217,// 194 PAY 191 

    0x262e1b1c,// 195 PAY 192 

    0xd0255622,// 196 PAY 193 

    0xb3d5337d,// 197 PAY 194 

    0x63bc6b50,// 198 PAY 195 

    0xc2afd419,// 199 PAY 196 

    0xe6296263,// 200 PAY 197 

    0xd0b983a2,// 201 PAY 198 

    0x36098d6f,// 202 PAY 199 

    0xfb117e72,// 203 PAY 200 

    0x90255aa2,// 204 PAY 201 

    0xee12c933,// 205 PAY 202 

    0xc617369b,// 206 PAY 203 

    0xb1907c27,// 207 PAY 204 

    0x03af2c30,// 208 PAY 205 

    0x3c7a0546,// 209 PAY 206 

    0x34266527,// 210 PAY 207 

    0xce2e9df8,// 211 PAY 208 

    0xc032be48,// 212 PAY 209 

    0xe16ab81d,// 213 PAY 210 

    0x72e57bc4,// 214 PAY 211 

    0x3e6c9dc2,// 215 PAY 212 

    0x303d5176,// 216 PAY 213 

    0x662b86f9,// 217 PAY 214 

    0xc9d7003c,// 218 PAY 215 

    0x82aec7eb,// 219 PAY 216 

    0x5a7d10c7,// 220 PAY 217 

    0xfcd8a41b,// 221 PAY 218 

    0x8fb02e4a,// 222 PAY 219 

    0x128729e9,// 223 PAY 220 

    0x0ad1bbcc,// 224 PAY 221 

    0x038d12cb,// 225 PAY 222 

    0x2d44c864,// 226 PAY 223 

    0x481ceb51,// 227 PAY 224 

    0x88b46af0,// 228 PAY 225 

    0x0bdac4d1,// 229 PAY 226 

    0x54fe8807,// 230 PAY 227 

    0xb15652d5,// 231 PAY 228 

    0xcb546537,// 232 PAY 229 

    0x47550286,// 233 PAY 230 

    0xd5c3f658,// 234 PAY 231 

    0x4bdfc735,// 235 PAY 232 

    0x6b7bf49d,// 236 PAY 233 

    0xb22ce27e,// 237 PAY 234 

    0xb533d98c,// 238 PAY 235 

    0xe09d937e,// 239 PAY 236 

    0x9435c7e9,// 240 PAY 237 

    0x3a5ee9c6,// 241 PAY 238 

    0x49a9fde6,// 242 PAY 239 

    0xc9f631a9,// 243 PAY 240 

    0xf7159bfc,// 244 PAY 241 

    0x6e1f876e,// 245 PAY 242 

    0x22a8c285,// 246 PAY 243 

    0xcc55c518,// 247 PAY 244 

    0x850e9696,// 248 PAY 245 

    0xc5ee261e,// 249 PAY 246 

    0xe8a3ab7e,// 250 PAY 247 

    0x0ea81a6f,// 251 PAY 248 

    0x5b234c36,// 252 PAY 249 

    0xdc078f8b,// 253 PAY 250 

    0x8d4c43f3,// 254 PAY 251 

    0xd06aa851,// 255 PAY 252 

    0x517ba95c,// 256 PAY 253 

    0x68a11158,// 257 PAY 254 

    0x9e92cf21,// 258 PAY 255 

    0xdcf095b5,// 259 PAY 256 

    0xe00de62d,// 260 PAY 257 

    0x31bfb95a,// 261 PAY 258 

    0x6e3bc80b,// 262 PAY 259 

    0xc0ef1de1,// 263 PAY 260 

    0xc212fef8,// 264 PAY 261 

    0xaad9846b,// 265 PAY 262 

    0x106c0ac7,// 266 PAY 263 

    0x3d3c741f,// 267 PAY 264 

    0xe1cd59a5,// 268 PAY 265 

    0x6665fd39,// 269 PAY 266 

    0x1d698f73,// 270 PAY 267 

    0x7cfafe0d,// 271 PAY 268 

    0x9984a7ec,// 272 PAY 269 

    0x14bdda69,// 273 PAY 270 

    0x0c897de9,// 274 PAY 271 

    0xc5f97ef8,// 275 PAY 272 

    0x16d6a05a,// 276 PAY 273 

    0xa5251d90,// 277 PAY 274 

    0xb6f60b18,// 278 PAY 275 

    0x7bf876c5,// 279 PAY 276 

    0x0ff409f4,// 280 PAY 277 

    0x40686cf0,// 281 PAY 278 

    0x816f28fa,// 282 PAY 279 

    0x86064804,// 283 PAY 280 

    0x956f8160,// 284 PAY 281 

    0x1974a401,// 285 PAY 282 

    0x2d10729c,// 286 PAY 283 

    0xf39322be,// 287 PAY 284 

    0xac4a8e57,// 288 PAY 285 

    0xbf3d28e4,// 289 PAY 286 

    0x2e694fce,// 290 PAY 287 

    0x94e040dc,// 291 PAY 288 

    0xdfa0d2ea,// 292 PAY 289 

    0x775b701f,// 293 PAY 290 

    0x92caa4e8,// 294 PAY 291 

    0x45878687,// 295 PAY 292 

    0x7ca4d36a,// 296 PAY 293 

    0x2c966c6e,// 297 PAY 294 

    0xc40b6678,// 298 PAY 295 

    0x95026812,// 299 PAY 296 

    0xdac8ba5d,// 300 PAY 297 

    0x5c95b489,// 301 PAY 298 

    0x93d5407a,// 302 PAY 299 

    0xef410f12,// 303 PAY 300 

    0x0ef40cb6,// 304 PAY 301 

    0x84021fbe,// 305 PAY 302 

    0x275046aa,// 306 PAY 303 

    0x47708193,// 307 PAY 304 

    0xb1218d46,// 308 PAY 305 

    0x6eeb429e,// 309 PAY 306 

    0x1007f54f,// 310 PAY 307 

    0x070c8406,// 311 PAY 308 

    0x3ab327b8,// 312 PAY 309 

    0x9feb80e0,// 313 PAY 310 

    0x9e02b7ba,// 314 PAY 311 

    0xe2c89538,// 315 PAY 312 

    0xfe034039,// 316 PAY 313 

    0x85b39f18,// 317 PAY 314 

    0xc9d6be77,// 318 PAY 315 

    0x6b92f385,// 319 PAY 316 

    0xb36fc9d4,// 320 PAY 317 

    0x63e6e51a,// 321 PAY 318 

    0x0d6ec531,// 322 PAY 319 

    0x85b1d929,// 323 PAY 320 

    0x3a85172c,// 324 PAY 321 

    0x4934a8c5,// 325 PAY 322 

    0x2631795f,// 326 PAY 323 

    0x2d449c70,// 327 PAY 324 

    0xe353e62e,// 328 PAY 325 

    0xe3cc215c,// 329 PAY 326 

    0x084def5a,// 330 PAY 327 

    0x529c2634,// 331 PAY 328 

    0x1f69a706,// 332 PAY 329 

    0x4ac55add,// 333 PAY 330 

    0x1c5162f9,// 334 PAY 331 

    0x01b5c3cf,// 335 PAY 332 

    0x6f431c5d,// 336 PAY 333 

    0xc19100c1,// 337 PAY 334 

    0x90d7be14,// 338 PAY 335 

    0x373ef6bc,// 339 PAY 336 

    0xdbae8d61,// 340 PAY 337 

    0xb5beb0f3,// 341 PAY 338 

    0x66bf9b13,// 342 PAY 339 

    0xd069ffdd,// 343 PAY 340 

    0x24f7f493,// 344 PAY 341 

    0x278519a1,// 345 PAY 342 

    0x0b7de29e,// 346 PAY 343 

    0x3008eaf0,// 347 PAY 344 

    0x2947b251,// 348 PAY 345 

    0x3659ade2,// 349 PAY 346 

    0x12f32cf4,// 350 PAY 347 

    0xc4188045,// 351 PAY 348 

    0xdd9c775a,// 352 PAY 349 

    0xb106ea68,// 353 PAY 350 

    0xf2be1c3e,// 354 PAY 351 

    0x7350e172,// 355 PAY 352 

    0x351bd32b,// 356 PAY 353 

    0x44523914,// 357 PAY 354 

    0x38976fe3,// 358 PAY 355 

    0x08c2966f,// 359 PAY 356 

    0x209e81b8,// 360 PAY 357 

    0x77d88166,// 361 PAY 358 

    0x21623941,// 362 PAY 359 

    0x0bcb92a9,// 363 PAY 360 

    0x6979f10f,// 364 PAY 361 

    0x0ac51b5e,// 365 PAY 362 

    0xa4fcabb8,// 366 PAY 363 

    0xbaf5ff6f,// 367 PAY 364 

    0xa04589eb,// 368 PAY 365 

    0xb2df75fb,// 369 PAY 366 

    0x10e9a13d,// 370 PAY 367 

    0x87b174eb,// 371 PAY 368 

    0xadcf5d54,// 372 PAY 369 

    0x4525fec0,// 373 PAY 370 

    0xcfc622d8,// 374 PAY 371 

    0xeaf135c0,// 375 PAY 372 

    0x42e2d2bd,// 376 PAY 373 

    0x4ad89730,// 377 PAY 374 

    0x53f90e8f,// 378 PAY 375 

    0xa49daced,// 379 PAY 376 

    0xdabe62a8,// 380 PAY 377 

    0x80a85c9f,// 381 PAY 378 

    0x276f5b84,// 382 PAY 379 

    0x878e20fc,// 383 PAY 380 

    0xf550e6a4,// 384 PAY 381 

    0xf5257d67,// 385 PAY 382 

    0xee3cb45c,// 386 PAY 383 

    0x45556dfe,// 387 PAY 384 

    0xeaab626f,// 388 PAY 385 

    0xe71d6952,// 389 PAY 386 

    0x64cd800f,// 390 PAY 387 

    0x6f073dd9,// 391 PAY 388 

    0x26bea21c,// 392 PAY 389 

    0x1d4f85f5,// 393 PAY 390 

    0x5cc9c378,// 394 PAY 391 

    0x361e6303,// 395 PAY 392 

    0x7b06e1b6,// 396 PAY 393 

    0x68b7a622,// 397 PAY 394 

    0xe8dd9874,// 398 PAY 395 

    0x0d48613f,// 399 PAY 396 

    0x9e66f609,// 400 PAY 397 

    0x85b97fbb,// 401 PAY 398 

    0x2480578c,// 402 PAY 399 

    0xf6400d34,// 403 PAY 400 

    0x2c907c46,// 404 PAY 401 

    0x5ae590a0,// 405 PAY 402 

    0x56f9d37c,// 406 PAY 403 

    0x44eab228,// 407 PAY 404 

    0x96146c02,// 408 PAY 405 

    0x81084d9c,// 409 PAY 406 

    0xa3478f9d,// 410 PAY 407 

    0x3c6ea128,// 411 PAY 408 

    0x68be9e23,// 412 PAY 409 

    0xf22c5f31,// 413 PAY 410 

    0xcbc23661,// 414 PAY 411 

    0xb0e71d8b,// 415 PAY 412 

    0x47badd97,// 416 PAY 413 

    0x1a14352b,// 417 PAY 414 

    0x1e3f706f,// 418 PAY 415 

    0x13cc8319,// 419 PAY 416 

    0x4bb124c4,// 420 PAY 417 

    0x61d86d24,// 421 PAY 418 

    0x2c4b4609,// 422 PAY 419 

    0x606cef7b,// 423 PAY 420 

    0xb7555e55,// 424 PAY 421 

    0x49d4960f,// 425 PAY 422 

    0xbc96e1e8,// 426 PAY 423 

    0x0e04a4af,// 427 PAY 424 

    0xc3ad5710,// 428 PAY 425 

    0x98ba79f2,// 429 PAY 426 

    0x2871d6ab,// 430 PAY 427 

    0x5f61e958,// 431 PAY 428 

    0x97123220,// 432 PAY 429 

    0xbc289ad2,// 433 PAY 430 

    0x9a7f319c,// 434 PAY 431 

    0xc1334e87,// 435 PAY 432 

    0xd9ac9db1,// 436 PAY 433 

    0xc92ff2fd,// 437 PAY 434 

    0xa83c6885,// 438 PAY 435 

    0x9ac3bfff,// 439 PAY 436 

    0x8e59430e,// 440 PAY 437 

    0xbf6408c0,// 441 PAY 438 

    0x6d000000,// 442 PAY 439 

/// HASH is  12 bytes 

    0x81084d9c,// 443 HSH   1 

    0xa3478f9d,// 444 HSH   2 

    0x3c6ea128,// 445 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 54 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8b 

    0x00d98b2d // 446 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt43_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 238 words. 

/// BDA size     is 947 (0x3b3) 

/// BDA id       is 0x6967 

    0x03b36967,// 3 BDA   1 

/// PAY Generic Data size   : 947 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf92c0258,// 4 PAY   1 

    0xc8773c86,// 5 PAY   2 

    0xd01b08a3,// 6 PAY   3 

    0x7ea7139a,// 7 PAY   4 

    0x9a8e2cc8,// 8 PAY   5 

    0xd53fda5a,// 9 PAY   6 

    0x8977cc8d,// 10 PAY   7 

    0x85413ce4,// 11 PAY   8 

    0xf1347bd6,// 12 PAY   9 

    0x1429e8b1,// 13 PAY  10 

    0x96a30568,// 14 PAY  11 

    0x6c4b2c00,// 15 PAY  12 

    0x1687d012,// 16 PAY  13 

    0x7aedffc9,// 17 PAY  14 

    0x1b7ddcbe,// 18 PAY  15 

    0xbf86778b,// 19 PAY  16 

    0xf47477e2,// 20 PAY  17 

    0x031457a0,// 21 PAY  18 

    0x682d2c2a,// 22 PAY  19 

    0x633b0d72,// 23 PAY  20 

    0x4f8a9541,// 24 PAY  21 

    0x38f993f3,// 25 PAY  22 

    0x919e1766,// 26 PAY  23 

    0x490850ad,// 27 PAY  24 

    0xe8fcbeb5,// 28 PAY  25 

    0x85016bb2,// 29 PAY  26 

    0xdd6b95f0,// 30 PAY  27 

    0xbf9844a3,// 31 PAY  28 

    0xfb4e8474,// 32 PAY  29 

    0x3fc333ac,// 33 PAY  30 

    0x4318a412,// 34 PAY  31 

    0xd80e1eeb,// 35 PAY  32 

    0x149f23d0,// 36 PAY  33 

    0x8d65f999,// 37 PAY  34 

    0xa8da2219,// 38 PAY  35 

    0xb6f95bdf,// 39 PAY  36 

    0x5f04b142,// 40 PAY  37 

    0xfc9e2c2a,// 41 PAY  38 

    0xb981bb24,// 42 PAY  39 

    0x94049ce7,// 43 PAY  40 

    0xe52ee97d,// 44 PAY  41 

    0x0d520d28,// 45 PAY  42 

    0x98b6ecb8,// 46 PAY  43 

    0xf9cd0b66,// 47 PAY  44 

    0x3c3d9691,// 48 PAY  45 

    0x4e2dd99e,// 49 PAY  46 

    0xb8646130,// 50 PAY  47 

    0xf5b9a8ac,// 51 PAY  48 

    0xd5bae249,// 52 PAY  49 

    0x4725ebdd,// 53 PAY  50 

    0x798821e7,// 54 PAY  51 

    0xc24f9d33,// 55 PAY  52 

    0xc5b57af3,// 56 PAY  53 

    0x748733fd,// 57 PAY  54 

    0xf9b47bd7,// 58 PAY  55 

    0x1a48a175,// 59 PAY  56 

    0xe1f92f33,// 60 PAY  57 

    0x61b09380,// 61 PAY  58 

    0x5458cef1,// 62 PAY  59 

    0xaefa3fe6,// 63 PAY  60 

    0xc7a5d1bc,// 64 PAY  61 

    0x3b43b4f5,// 65 PAY  62 

    0xf17bac12,// 66 PAY  63 

    0x12154555,// 67 PAY  64 

    0x25d260e8,// 68 PAY  65 

    0x663985eb,// 69 PAY  66 

    0x36fd0d18,// 70 PAY  67 

    0xe706e5a2,// 71 PAY  68 

    0x75dfa17c,// 72 PAY  69 

    0xb3e4dfa5,// 73 PAY  70 

    0x06186874,// 74 PAY  71 

    0xb35550b7,// 75 PAY  72 

    0xd0e6b4ea,// 76 PAY  73 

    0xabd0c323,// 77 PAY  74 

    0xd3845fc8,// 78 PAY  75 

    0x3bced460,// 79 PAY  76 

    0x1b120aeb,// 80 PAY  77 

    0x64ceb88e,// 81 PAY  78 

    0xb5b7a178,// 82 PAY  79 

    0x39727d0d,// 83 PAY  80 

    0x5e3113ae,// 84 PAY  81 

    0xc0d5c0c4,// 85 PAY  82 

    0x5f3742fc,// 86 PAY  83 

    0x91c19089,// 87 PAY  84 

    0xf60e9fc4,// 88 PAY  85 

    0xc98e15ad,// 89 PAY  86 

    0x4bf68e38,// 90 PAY  87 

    0x78336345,// 91 PAY  88 

    0x24e377b6,// 92 PAY  89 

    0x3bc8c968,// 93 PAY  90 

    0x94100819,// 94 PAY  91 

    0xae9961b3,// 95 PAY  92 

    0xf7055ead,// 96 PAY  93 

    0x1deea81d,// 97 PAY  94 

    0x655b5ab0,// 98 PAY  95 

    0x74383bca,// 99 PAY  96 

    0x28ecefe6,// 100 PAY  97 

    0x2751786a,// 101 PAY  98 

    0xc5f135a4,// 102 PAY  99 

    0x134cc6d6,// 103 PAY 100 

    0x885a417b,// 104 PAY 101 

    0x00eb711d,// 105 PAY 102 

    0x8e3bda80,// 106 PAY 103 

    0x3046d8d7,// 107 PAY 104 

    0x0f148944,// 108 PAY 105 

    0x503eedeb,// 109 PAY 106 

    0x06d8752f,// 110 PAY 107 

    0xb065f160,// 111 PAY 108 

    0x4427e07e,// 112 PAY 109 

    0x9773f6f4,// 113 PAY 110 

    0xa6a342da,// 114 PAY 111 

    0x0b66e3ee,// 115 PAY 112 

    0xcccd1d65,// 116 PAY 113 

    0xb8fee531,// 117 PAY 114 

    0xbe64894a,// 118 PAY 115 

    0xdbdc4bf5,// 119 PAY 116 

    0x87d0b47b,// 120 PAY 117 

    0x96ce3f15,// 121 PAY 118 

    0xdc762a91,// 122 PAY 119 

    0xd8669718,// 123 PAY 120 

    0xc26edd9e,// 124 PAY 121 

    0x1597794d,// 125 PAY 122 

    0x43f4bc25,// 126 PAY 123 

    0x56144b20,// 127 PAY 124 

    0xfc5f6e8b,// 128 PAY 125 

    0xb1b02a54,// 129 PAY 126 

    0xee6e8c85,// 130 PAY 127 

    0x6024739d,// 131 PAY 128 

    0xcfd4b883,// 132 PAY 129 

    0xfe635414,// 133 PAY 130 

    0xd02c28b6,// 134 PAY 131 

    0xc3b5ccf4,// 135 PAY 132 

    0xa1458a80,// 136 PAY 133 

    0x7be1b713,// 137 PAY 134 

    0x78a08153,// 138 PAY 135 

    0x32648c39,// 139 PAY 136 

    0x99b4ffdd,// 140 PAY 137 

    0x2cf784c5,// 141 PAY 138 

    0x7073c016,// 142 PAY 139 

    0x92339b1e,// 143 PAY 140 

    0x6a1587e8,// 144 PAY 141 

    0x3ed3c510,// 145 PAY 142 

    0xb5aa258c,// 146 PAY 143 

    0xfa2be0ac,// 147 PAY 144 

    0x7429e582,// 148 PAY 145 

    0x231f21a8,// 149 PAY 146 

    0xf96ef710,// 150 PAY 147 

    0xf9e02451,// 151 PAY 148 

    0xfcda6bab,// 152 PAY 149 

    0xaae43e17,// 153 PAY 150 

    0xc61bc7e6,// 154 PAY 151 

    0xe73b2580,// 155 PAY 152 

    0x50c53eba,// 156 PAY 153 

    0x078b442d,// 157 PAY 154 

    0x29ac40fd,// 158 PAY 155 

    0x5aaac4ff,// 159 PAY 156 

    0xd7d58eeb,// 160 PAY 157 

    0x42836e63,// 161 PAY 158 

    0x97f110a7,// 162 PAY 159 

    0xeead643b,// 163 PAY 160 

    0x100fbb0c,// 164 PAY 161 

    0xa9de26bf,// 165 PAY 162 

    0x9408c230,// 166 PAY 163 

    0xa81faccb,// 167 PAY 164 

    0x0c79657c,// 168 PAY 165 

    0x9da72206,// 169 PAY 166 

    0x3d5b520e,// 170 PAY 167 

    0x5883ad68,// 171 PAY 168 

    0xe98440ad,// 172 PAY 169 

    0xdc26b77e,// 173 PAY 170 

    0x5eeb5653,// 174 PAY 171 

    0xb8353236,// 175 PAY 172 

    0xd9d1f67c,// 176 PAY 173 

    0x250fc03e,// 177 PAY 174 

    0x31747fe5,// 178 PAY 175 

    0x682b24aa,// 179 PAY 176 

    0x60cf1610,// 180 PAY 177 

    0x6015f3d3,// 181 PAY 178 

    0xaeb68296,// 182 PAY 179 

    0x357ca54a,// 183 PAY 180 

    0x07cbb48a,// 184 PAY 181 

    0x67b1592e,// 185 PAY 182 

    0x8bf18de5,// 186 PAY 183 

    0x591bb89c,// 187 PAY 184 

    0x45e9fa2a,// 188 PAY 185 

    0x37c1046b,// 189 PAY 186 

    0x441dc91d,// 190 PAY 187 

    0x51ba4c6a,// 191 PAY 188 

    0x3a328bf6,// 192 PAY 189 

    0x432de29b,// 193 PAY 190 

    0xdc699b56,// 194 PAY 191 

    0x23699d7d,// 195 PAY 192 

    0x96c53a94,// 196 PAY 193 

    0x451eec0e,// 197 PAY 194 

    0x535b87d4,// 198 PAY 195 

    0xb167d4aa,// 199 PAY 196 

    0xe65e9460,// 200 PAY 197 

    0x596704ca,// 201 PAY 198 

    0xf13b778f,// 202 PAY 199 

    0xef62e387,// 203 PAY 200 

    0x5647601c,// 204 PAY 201 

    0xb81a0e00,// 205 PAY 202 

    0x84425321,// 206 PAY 203 

    0x74ef25f6,// 207 PAY 204 

    0x8b4215a5,// 208 PAY 205 

    0xbb1dde38,// 209 PAY 206 

    0x06c2006b,// 210 PAY 207 

    0x702c4b95,// 211 PAY 208 

    0x939e7730,// 212 PAY 209 

    0x6e7d6048,// 213 PAY 210 

    0x872a7b3d,// 214 PAY 211 

    0xa8a20fd2,// 215 PAY 212 

    0x7c68f39d,// 216 PAY 213 

    0x5249f980,// 217 PAY 214 

    0x61407be3,// 218 PAY 215 

    0x6b327764,// 219 PAY 216 

    0x8582f093,// 220 PAY 217 

    0x85cc5d91,// 221 PAY 218 

    0x2c811701,// 222 PAY 219 

    0xeca97d3a,// 223 PAY 220 

    0x3c9f2e1e,// 224 PAY 221 

    0x806aecc7,// 225 PAY 222 

    0x17ea9029,// 226 PAY 223 

    0x8046532e,// 227 PAY 224 

    0xad0f0fe6,// 228 PAY 225 

    0x785d4727,// 229 PAY 226 

    0xadf3f019,// 230 PAY 227 

    0x705d9714,// 231 PAY 228 

    0x97e326cf,// 232 PAY 229 

    0x341ebac1,// 233 PAY 230 

    0x09c51616,// 234 PAY 231 

    0x60bbed38,// 235 PAY 232 

    0xa1c0b1c0,// 236 PAY 233 

    0x55aa6426,// 237 PAY 234 

    0x97e69b55,// 238 PAY 235 

    0x4def24b8,// 239 PAY 236 

    0x7d70a500,// 240 PAY 237 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 232 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x79 

    0x03a07985 // 241 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt44_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 117 words. 

/// BDA size     is 463 (0x1cf) 

/// BDA id       is 0x4bfd 

    0x01cf4bfd,// 3 BDA   1 

/// PAY Generic Data size   : 463 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x2618a4dd,// 4 PAY   1 

    0xd5ddb035,// 5 PAY   2 

    0x00ebd56d,// 6 PAY   3 

    0x8ace5a6f,// 7 PAY   4 

    0x5f31f803,// 8 PAY   5 

    0x98dcf9be,// 9 PAY   6 

    0xdcdb9913,// 10 PAY   7 

    0xea38b643,// 11 PAY   8 

    0xaa99e142,// 12 PAY   9 

    0xff5a5e13,// 13 PAY  10 

    0x27b1c63d,// 14 PAY  11 

    0x879319d5,// 15 PAY  12 

    0xb5ece30e,// 16 PAY  13 

    0xcdd233bf,// 17 PAY  14 

    0xf272d8ad,// 18 PAY  15 

    0xc24c9cc1,// 19 PAY  16 

    0x2443bc6c,// 20 PAY  17 

    0xd6750736,// 21 PAY  18 

    0x7a133301,// 22 PAY  19 

    0x9e290e77,// 23 PAY  20 

    0x280bd5aa,// 24 PAY  21 

    0xe9496f3f,// 25 PAY  22 

    0x4dae7e30,// 26 PAY  23 

    0x31002a21,// 27 PAY  24 

    0x00809e82,// 28 PAY  25 

    0xdac06543,// 29 PAY  26 

    0xa507726e,// 30 PAY  27 

    0x6ebef936,// 31 PAY  28 

    0x74373e84,// 32 PAY  29 

    0x0883059e,// 33 PAY  30 

    0x5d36e54f,// 34 PAY  31 

    0x0633e8eb,// 35 PAY  32 

    0xc9229687,// 36 PAY  33 

    0xbbdd6890,// 37 PAY  34 

    0x6bd05a00,// 38 PAY  35 

    0x8c76642f,// 39 PAY  36 

    0x0c3a63e2,// 40 PAY  37 

    0x837cbf4c,// 41 PAY  38 

    0xbc9064a0,// 42 PAY  39 

    0xb67cebb7,// 43 PAY  40 

    0xc844604c,// 44 PAY  41 

    0x9718dd97,// 45 PAY  42 

    0x47bbb555,// 46 PAY  43 

    0x91a87709,// 47 PAY  44 

    0x5bd99efd,// 48 PAY  45 

    0x4bb723de,// 49 PAY  46 

    0x80cfaf76,// 50 PAY  47 

    0x38a2795b,// 51 PAY  48 

    0x6e542684,// 52 PAY  49 

    0x563bbeee,// 53 PAY  50 

    0xc4576dc1,// 54 PAY  51 

    0x87bec1b3,// 55 PAY  52 

    0x3bfc60de,// 56 PAY  53 

    0xb510db9c,// 57 PAY  54 

    0xeab051ff,// 58 PAY  55 

    0xb4f92ad2,// 59 PAY  56 

    0xba0ba322,// 60 PAY  57 

    0xbcc09c8e,// 61 PAY  58 

    0xfa26dfdb,// 62 PAY  59 

    0x4f511ca6,// 63 PAY  60 

    0x4602682a,// 64 PAY  61 

    0x50df3842,// 65 PAY  62 

    0x25e023ad,// 66 PAY  63 

    0x83c074e0,// 67 PAY  64 

    0x5ae44ce6,// 68 PAY  65 

    0x29ec93c8,// 69 PAY  66 

    0xc6365207,// 70 PAY  67 

    0x1dc71678,// 71 PAY  68 

    0x7df23dc5,// 72 PAY  69 

    0x0c689e60,// 73 PAY  70 

    0x19d72f5c,// 74 PAY  71 

    0xb2a50a6d,// 75 PAY  72 

    0x26ee061c,// 76 PAY  73 

    0x663c8ca9,// 77 PAY  74 

    0x2b4d3dd4,// 78 PAY  75 

    0x3e002986,// 79 PAY  76 

    0xf7a6b1ae,// 80 PAY  77 

    0xdee5f6d7,// 81 PAY  78 

    0xc2ea4f54,// 82 PAY  79 

    0x62cf7239,// 83 PAY  80 

    0x6b37f936,// 84 PAY  81 

    0x0141b240,// 85 PAY  82 

    0x3fe0c46c,// 86 PAY  83 

    0xa7152dca,// 87 PAY  84 

    0x13923f56,// 88 PAY  85 

    0x07d0788c,// 89 PAY  86 

    0x64b7c1ab,// 90 PAY  87 

    0xd5260a01,// 91 PAY  88 

    0x20456ea3,// 92 PAY  89 

    0xbc4a75e7,// 93 PAY  90 

    0x6c0160d1,// 94 PAY  91 

    0xefe1760f,// 95 PAY  92 

    0xd6438f2e,// 96 PAY  93 

    0x777c4f0b,// 97 PAY  94 

    0x9e07f9b0,// 98 PAY  95 

    0x00a9e900,// 99 PAY  96 

    0xd9ff0ada,// 100 PAY  97 

    0xa0ee2462,// 101 PAY  98 

    0xbd01670b,// 102 PAY  99 

    0xa3d808dd,// 103 PAY 100 

    0xf08cc747,// 104 PAY 101 

    0xb1d62a5a,// 105 PAY 102 

    0xd965f48e,// 106 PAY 103 

    0x108cda0f,// 107 PAY 104 

    0x69839443,// 108 PAY 105 

    0x8d167e87,// 109 PAY 106 

    0x489e143d,// 110 PAY 107 

    0x38443208,// 111 PAY 108 

    0x74c49891,// 112 PAY 109 

    0x01804072,// 113 PAY 110 

    0x74fa275d,// 114 PAY 111 

    0x211d34ad,// 115 PAY 112 

    0xb45e3c1f,// 116 PAY 113 

    0x5a09a78a,// 117 PAY 114 

    0xa28b2b94,// 118 PAY 115 

    0xe6782c00,// 119 PAY 116 

/// STA is 1 words. 

/// STA num_pkts       : 13 

/// STA pkt_idx        : 80 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9a 

    0x01419a0d // 120 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt45_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 496 words. 

/// BDA size     is 1980 (0x7bc) 

/// BDA id       is 0xae4b 

    0x07bcae4b,// 3 BDA   1 

/// PAY Generic Data size   : 1980 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x88e19dc1,// 4 PAY   1 

    0xffeed6ae,// 5 PAY   2 

    0x231d9ee1,// 6 PAY   3 

    0xc0b43afe,// 7 PAY   4 

    0x3f113c37,// 8 PAY   5 

    0x16afc947,// 9 PAY   6 

    0xa5710d88,// 10 PAY   7 

    0x38646275,// 11 PAY   8 

    0x09182bc9,// 12 PAY   9 

    0x04bcf778,// 13 PAY  10 

    0x9a1fcaa3,// 14 PAY  11 

    0x53c7e99f,// 15 PAY  12 

    0xa14682ae,// 16 PAY  13 

    0x95bddc1d,// 17 PAY  14 

    0x8359be55,// 18 PAY  15 

    0xf8bf660c,// 19 PAY  16 

    0x88865217,// 20 PAY  17 

    0x1b527864,// 21 PAY  18 

    0x862a9560,// 22 PAY  19 

    0xfaa1434c,// 23 PAY  20 

    0xdc05bfae,// 24 PAY  21 

    0x01b32149,// 25 PAY  22 

    0x068e6936,// 26 PAY  23 

    0x28e81a03,// 27 PAY  24 

    0xa2c92664,// 28 PAY  25 

    0xb0676c52,// 29 PAY  26 

    0xd28d49e1,// 30 PAY  27 

    0x1566d34b,// 31 PAY  28 

    0x27850bfc,// 32 PAY  29 

    0x9322456a,// 33 PAY  30 

    0x6fbf0805,// 34 PAY  31 

    0x6ccb5ce2,// 35 PAY  32 

    0x83b55891,// 36 PAY  33 

    0x1d544d3b,// 37 PAY  34 

    0xc93798ae,// 38 PAY  35 

    0xbd716ea3,// 39 PAY  36 

    0xe06eb71c,// 40 PAY  37 

    0xe636e2fa,// 41 PAY  38 

    0xb29c3df7,// 42 PAY  39 

    0xc26b0f0c,// 43 PAY  40 

    0x73db8482,// 44 PAY  41 

    0xc192f0e4,// 45 PAY  42 

    0x2b612b41,// 46 PAY  43 

    0x40539195,// 47 PAY  44 

    0x124e9c7a,// 48 PAY  45 

    0xf29a2f0c,// 49 PAY  46 

    0xd0252688,// 50 PAY  47 

    0x4f9d81a6,// 51 PAY  48 

    0x6247c27c,// 52 PAY  49 

    0x753fc5d8,// 53 PAY  50 

    0x8aa250a5,// 54 PAY  51 

    0x48a787ed,// 55 PAY  52 

    0x20116542,// 56 PAY  53 

    0x6f4f5ba9,// 57 PAY  54 

    0xa4b3ee68,// 58 PAY  55 

    0x0b2e0e75,// 59 PAY  56 

    0xac5d25aa,// 60 PAY  57 

    0x18cac41a,// 61 PAY  58 

    0x9bff17bd,// 62 PAY  59 

    0x83192ab1,// 63 PAY  60 

    0x6c8a8a2a,// 64 PAY  61 

    0xb4e64037,// 65 PAY  62 

    0x83307ed9,// 66 PAY  63 

    0xdab764bd,// 67 PAY  64 

    0xff1ec885,// 68 PAY  65 

    0x03c738c4,// 69 PAY  66 

    0x75484455,// 70 PAY  67 

    0x6e267ec0,// 71 PAY  68 

    0x33c71bde,// 72 PAY  69 

    0x430792e9,// 73 PAY  70 

    0xbe8d7909,// 74 PAY  71 

    0x6ca11771,// 75 PAY  72 

    0x3751eb80,// 76 PAY  73 

    0x462e7996,// 77 PAY  74 

    0x018a3644,// 78 PAY  75 

    0x601a0c46,// 79 PAY  76 

    0xa7865d48,// 80 PAY  77 

    0xaeca0c96,// 81 PAY  78 

    0x3f33dd77,// 82 PAY  79 

    0x8a56814b,// 83 PAY  80 

    0xb0e5d7d9,// 84 PAY  81 

    0xfb9e76a0,// 85 PAY  82 

    0x0d296dbe,// 86 PAY  83 

    0x70d66675,// 87 PAY  84 

    0x0b339d7d,// 88 PAY  85 

    0xda0c6d03,// 89 PAY  86 

    0x882342bd,// 90 PAY  87 

    0xa584fdf0,// 91 PAY  88 

    0x5e5af58c,// 92 PAY  89 

    0x5e527623,// 93 PAY  90 

    0xc6fa3ba4,// 94 PAY  91 

    0xb31cb3ee,// 95 PAY  92 

    0x1a9427d1,// 96 PAY  93 

    0x1c0369f1,// 97 PAY  94 

    0x8fc11b13,// 98 PAY  95 

    0xf1114633,// 99 PAY  96 

    0x837a8c1e,// 100 PAY  97 

    0xc001845b,// 101 PAY  98 

    0x4841432a,// 102 PAY  99 

    0x8cca8ee3,// 103 PAY 100 

    0x5a2b403e,// 104 PAY 101 

    0x3dc04cba,// 105 PAY 102 

    0xd2ddf2f2,// 106 PAY 103 

    0x3ac8c6b5,// 107 PAY 104 

    0x09584cf5,// 108 PAY 105 

    0xa962d5ff,// 109 PAY 106 

    0x31902b71,// 110 PAY 107 

    0x07725c00,// 111 PAY 108 

    0xa82caebb,// 112 PAY 109 

    0x34ba885d,// 113 PAY 110 

    0x71c518ab,// 114 PAY 111 

    0x1e5bfda2,// 115 PAY 112 

    0x06ac37a2,// 116 PAY 113 

    0xf39e6bfd,// 117 PAY 114 

    0xaceede11,// 118 PAY 115 

    0xe19ffbe0,// 119 PAY 116 

    0x967e3415,// 120 PAY 117 

    0x10432d56,// 121 PAY 118 

    0x83b1412b,// 122 PAY 119 

    0x20eae470,// 123 PAY 120 

    0x778d758a,// 124 PAY 121 

    0xe6621fe2,// 125 PAY 122 

    0x720130a2,// 126 PAY 123 

    0xc6960519,// 127 PAY 124 

    0xf00d2ec6,// 128 PAY 125 

    0x1d541f3c,// 129 PAY 126 

    0x45481bea,// 130 PAY 127 

    0x883ce1d5,// 131 PAY 128 

    0xe86eb1da,// 132 PAY 129 

    0xc20fc8ac,// 133 PAY 130 

    0xdd0173df,// 134 PAY 131 

    0xb4beb262,// 135 PAY 132 

    0x6e137d33,// 136 PAY 133 

    0x21aec037,// 137 PAY 134 

    0x36c36f94,// 138 PAY 135 

    0xd938d6fa,// 139 PAY 136 

    0x8325edc1,// 140 PAY 137 

    0x7d0c7147,// 141 PAY 138 

    0x2c2c5b59,// 142 PAY 139 

    0x95d3a466,// 143 PAY 140 

    0xa94f6f4f,// 144 PAY 141 

    0x35f407e4,// 145 PAY 142 

    0xff748851,// 146 PAY 143 

    0x82f0f155,// 147 PAY 144 

    0x1f72623b,// 148 PAY 145 

    0x4ca78316,// 149 PAY 146 

    0xbcbd6178,// 150 PAY 147 

    0xc0d3d571,// 151 PAY 148 

    0x805e76c6,// 152 PAY 149 

    0x3853c1f3,// 153 PAY 150 

    0x2d5545ee,// 154 PAY 151 

    0xc6413356,// 155 PAY 152 

    0xddc34df7,// 156 PAY 153 

    0xd2375494,// 157 PAY 154 

    0x41f2ca6d,// 158 PAY 155 

    0xd4cecf9a,// 159 PAY 156 

    0x2eb9ea6a,// 160 PAY 157 

    0x9f7baa87,// 161 PAY 158 

    0xd01f4500,// 162 PAY 159 

    0xcaac825a,// 163 PAY 160 

    0xb6e3ce1d,// 164 PAY 161 

    0x670d2de9,// 165 PAY 162 

    0x7becc031,// 166 PAY 163 

    0x75e8f238,// 167 PAY 164 

    0x1677df86,// 168 PAY 165 

    0xe029a071,// 169 PAY 166 

    0x99cf901a,// 170 PAY 167 

    0x4d64c9ea,// 171 PAY 168 

    0x173463a0,// 172 PAY 169 

    0x832cc218,// 173 PAY 170 

    0x7975e392,// 174 PAY 171 

    0xc07bd3f0,// 175 PAY 172 

    0x418dae49,// 176 PAY 173 

    0x66525cfa,// 177 PAY 174 

    0x05dde32b,// 178 PAY 175 

    0xa245ddfb,// 179 PAY 176 

    0x8243887a,// 180 PAY 177 

    0x02b7c1c7,// 181 PAY 178 

    0x2406c00c,// 182 PAY 179 

    0x5ec64b14,// 183 PAY 180 

    0x76cd6788,// 184 PAY 181 

    0x2ce86154,// 185 PAY 182 

    0x599d02df,// 186 PAY 183 

    0xa674312c,// 187 PAY 184 

    0xd35bf9be,// 188 PAY 185 

    0xca88f3eb,// 189 PAY 186 

    0x1b5f3715,// 190 PAY 187 

    0xe1d8ab66,// 191 PAY 188 

    0xc5ed2a66,// 192 PAY 189 

    0x73e9675c,// 193 PAY 190 

    0x73455690,// 194 PAY 191 

    0x7fa7a3e3,// 195 PAY 192 

    0xc5a99723,// 196 PAY 193 

    0x39c27018,// 197 PAY 194 

    0xcf5833f3,// 198 PAY 195 

    0x1020b380,// 199 PAY 196 

    0x52ab6e0a,// 200 PAY 197 

    0x03693e52,// 201 PAY 198 

    0xbb5fc633,// 202 PAY 199 

    0xdffcb986,// 203 PAY 200 

    0x617401c2,// 204 PAY 201 

    0x9afbd51c,// 205 PAY 202 

    0x1727afdd,// 206 PAY 203 

    0xa9414f1e,// 207 PAY 204 

    0x14c604ab,// 208 PAY 205 

    0x4c2027d2,// 209 PAY 206 

    0x800d3a5e,// 210 PAY 207 

    0xd2f7b344,// 211 PAY 208 

    0x87245942,// 212 PAY 209 

    0xf24bd1eb,// 213 PAY 210 

    0x867ac9d6,// 214 PAY 211 

    0x2c126907,// 215 PAY 212 

    0x170e36cc,// 216 PAY 213 

    0x46daeb51,// 217 PAY 214 

    0xd04c7c41,// 218 PAY 215 

    0x91b5bd85,// 219 PAY 216 

    0x6415235b,// 220 PAY 217 

    0x5b609cea,// 221 PAY 218 

    0x6ff0d514,// 222 PAY 219 

    0x2f3c4b79,// 223 PAY 220 

    0xfb4154c9,// 224 PAY 221 

    0x4f8d0579,// 225 PAY 222 

    0x54620d3c,// 226 PAY 223 

    0x162cd13d,// 227 PAY 224 

    0x3386c545,// 228 PAY 225 

    0x2200d1df,// 229 PAY 226 

    0x9d68fd9c,// 230 PAY 227 

    0xa032c846,// 231 PAY 228 

    0x1fdde4e2,// 232 PAY 229 

    0x7c42303e,// 233 PAY 230 

    0x6e4e16ab,// 234 PAY 231 

    0x7717e87c,// 235 PAY 232 

    0x65ca5128,// 236 PAY 233 

    0x2b3361dd,// 237 PAY 234 

    0xd2b6d9d6,// 238 PAY 235 

    0xa15af49c,// 239 PAY 236 

    0x0ce2085d,// 240 PAY 237 

    0x19b744a7,// 241 PAY 238 

    0x65bb2432,// 242 PAY 239 

    0x0917cd97,// 243 PAY 240 

    0x07881b10,// 244 PAY 241 

    0x903d3f5f,// 245 PAY 242 

    0xbf6dfe0f,// 246 PAY 243 

    0xcde186b7,// 247 PAY 244 

    0x23d2629e,// 248 PAY 245 

    0x1d2c5b74,// 249 PAY 246 

    0x031b36ab,// 250 PAY 247 

    0x324b1ad9,// 251 PAY 248 

    0x8761d56f,// 252 PAY 249 

    0xdff1655a,// 253 PAY 250 

    0xbb469f91,// 254 PAY 251 

    0xc76c97f9,// 255 PAY 252 

    0xf765a067,// 256 PAY 253 

    0x73a65bea,// 257 PAY 254 

    0x295d0db6,// 258 PAY 255 

    0x45710d01,// 259 PAY 256 

    0xc134e0f7,// 260 PAY 257 

    0x265413c8,// 261 PAY 258 

    0xa36373ed,// 262 PAY 259 

    0xf8965251,// 263 PAY 260 

    0x2bb7a13f,// 264 PAY 261 

    0x8cd0ba22,// 265 PAY 262 

    0x4f083f1b,// 266 PAY 263 

    0x04dff404,// 267 PAY 264 

    0xbf4a9e93,// 268 PAY 265 

    0x9f5f4332,// 269 PAY 266 

    0x213c7590,// 270 PAY 267 

    0x8b1fac65,// 271 PAY 268 

    0x8a86a9eb,// 272 PAY 269 

    0x14debec7,// 273 PAY 270 

    0xd19f6544,// 274 PAY 271 

    0xd91abd1a,// 275 PAY 272 

    0x67c44cc2,// 276 PAY 273 

    0xb5a4f43d,// 277 PAY 274 

    0x7b58796b,// 278 PAY 275 

    0xb1786770,// 279 PAY 276 

    0x3e7c9daa,// 280 PAY 277 

    0x41049e14,// 281 PAY 278 

    0x16f96519,// 282 PAY 279 

    0xbac94e14,// 283 PAY 280 

    0xa4e576a2,// 284 PAY 281 

    0x6509b2f5,// 285 PAY 282 

    0x28ceeb74,// 286 PAY 283 

    0x914a3b7b,// 287 PAY 284 

    0x65b6d6d4,// 288 PAY 285 

    0x6c7e1dc3,// 289 PAY 286 

    0xdf255335,// 290 PAY 287 

    0x76afb062,// 291 PAY 288 

    0xddbb3c19,// 292 PAY 289 

    0xd72cff5c,// 293 PAY 290 

    0xc1202bde,// 294 PAY 291 

    0x3e40d739,// 295 PAY 292 

    0x25ff283d,// 296 PAY 293 

    0x53db243e,// 297 PAY 294 

    0x679f0626,// 298 PAY 295 

    0x7c4ccc3f,// 299 PAY 296 

    0xc8326ab1,// 300 PAY 297 

    0x9b209bea,// 301 PAY 298 

    0x7b7bec46,// 302 PAY 299 

    0x3d3227de,// 303 PAY 300 

    0x10ef2162,// 304 PAY 301 

    0x94d02d44,// 305 PAY 302 

    0xc85d4cc0,// 306 PAY 303 

    0xbe0e11e5,// 307 PAY 304 

    0xe0ccedc8,// 308 PAY 305 

    0x06a4d0fa,// 309 PAY 306 

    0x3872461a,// 310 PAY 307 

    0xc1ede095,// 311 PAY 308 

    0x2e7c275b,// 312 PAY 309 

    0x7abb90f2,// 313 PAY 310 

    0x594fd48f,// 314 PAY 311 

    0xb6d29c27,// 315 PAY 312 

    0x669e96c6,// 316 PAY 313 

    0x2c883f79,// 317 PAY 314 

    0xfca1d23c,// 318 PAY 315 

    0x3d817cbf,// 319 PAY 316 

    0xde1c845d,// 320 PAY 317 

    0x54becce8,// 321 PAY 318 

    0xd0db5fbf,// 322 PAY 319 

    0x8e5f63e8,// 323 PAY 320 

    0x06e49eb5,// 324 PAY 321 

    0x5e2cc574,// 325 PAY 322 

    0x1310581d,// 326 PAY 323 

    0x3e3849b4,// 327 PAY 324 

    0x023af647,// 328 PAY 325 

    0xc13c0966,// 329 PAY 326 

    0x5316037a,// 330 PAY 327 

    0x93e149ad,// 331 PAY 328 

    0x74bad2f0,// 332 PAY 329 

    0x89962fe0,// 333 PAY 330 

    0xad914cf7,// 334 PAY 331 

    0x3c073bf0,// 335 PAY 332 

    0xfa9780d6,// 336 PAY 333 

    0x760bf921,// 337 PAY 334 

    0xe9fac2a4,// 338 PAY 335 

    0xe26a0447,// 339 PAY 336 

    0x1acbd9d2,// 340 PAY 337 

    0xf0cd80ae,// 341 PAY 338 

    0xa640a73a,// 342 PAY 339 

    0x80e3c7c2,// 343 PAY 340 

    0xe3c75437,// 344 PAY 341 

    0xe010eb87,// 345 PAY 342 

    0xa192e6c8,// 346 PAY 343 

    0xf8c5641d,// 347 PAY 344 

    0x2f7ec1e6,// 348 PAY 345 

    0x19cd58dd,// 349 PAY 346 

    0x436c4eca,// 350 PAY 347 

    0xc045443a,// 351 PAY 348 

    0xb87e442e,// 352 PAY 349 

    0x698b7c0d,// 353 PAY 350 

    0x295be789,// 354 PAY 351 

    0xc0b30594,// 355 PAY 352 

    0xee20af2f,// 356 PAY 353 

    0xe2de8d91,// 357 PAY 354 

    0xdfea6f91,// 358 PAY 355 

    0xd3bed7c7,// 359 PAY 356 

    0xdf438663,// 360 PAY 357 

    0x52f167b4,// 361 PAY 358 

    0x41a40bee,// 362 PAY 359 

    0xd0fa1880,// 363 PAY 360 

    0x6012a883,// 364 PAY 361 

    0x6d72b18e,// 365 PAY 362 

    0xa763a1cc,// 366 PAY 363 

    0x8c44956b,// 367 PAY 364 

    0x7b7f61cf,// 368 PAY 365 

    0xa26e55d1,// 369 PAY 366 

    0x17a670e2,// 370 PAY 367 

    0x4eb88f52,// 371 PAY 368 

    0xe7c64b51,// 372 PAY 369 

    0x578a741c,// 373 PAY 370 

    0x2743e2ab,// 374 PAY 371 

    0x2c4a1e7c,// 375 PAY 372 

    0x7bbb9756,// 376 PAY 373 

    0x36e6157e,// 377 PAY 374 

    0xf952601b,// 378 PAY 375 

    0x8b3d3d21,// 379 PAY 376 

    0x52dbb36e,// 380 PAY 377 

    0x799a15e2,// 381 PAY 378 

    0xcfa1924d,// 382 PAY 379 

    0x24bc38f4,// 383 PAY 380 

    0x21e1bda5,// 384 PAY 381 

    0x3637a463,// 385 PAY 382 

    0x5b9ac262,// 386 PAY 383 

    0x090acfcd,// 387 PAY 384 

    0x1772b0e2,// 388 PAY 385 

    0x44247bd9,// 389 PAY 386 

    0xf802046f,// 390 PAY 387 

    0xb13e7f4e,// 391 PAY 388 

    0x3a04b4ff,// 392 PAY 389 

    0x1aaec2ad,// 393 PAY 390 

    0x58019a65,// 394 PAY 391 

    0x0247da2f,// 395 PAY 392 

    0x72be2f09,// 396 PAY 393 

    0x4ae88846,// 397 PAY 394 

    0x79b1add6,// 398 PAY 395 

    0x02e3d4e7,// 399 PAY 396 

    0xf2fad9c2,// 400 PAY 397 

    0x54e694fa,// 401 PAY 398 

    0x0a9718f5,// 402 PAY 399 

    0xf1fa0882,// 403 PAY 400 

    0x05e728b4,// 404 PAY 401 

    0xa5c2fa62,// 405 PAY 402 

    0x77e23a78,// 406 PAY 403 

    0x015fd9c1,// 407 PAY 404 

    0xd3fec01d,// 408 PAY 405 

    0x6561ac96,// 409 PAY 406 

    0x8bbd3b71,// 410 PAY 407 

    0x24bc3ca6,// 411 PAY 408 

    0x3b1cde94,// 412 PAY 409 

    0xa21e6cc1,// 413 PAY 410 

    0xafaa9f83,// 414 PAY 411 

    0xef848705,// 415 PAY 412 

    0x18dad722,// 416 PAY 413 

    0xeabdcb82,// 417 PAY 414 

    0xea3e4e85,// 418 PAY 415 

    0x200a745c,// 419 PAY 416 

    0xcff99c8d,// 420 PAY 417 

    0xc2674ff1,// 421 PAY 418 

    0x98d59988,// 422 PAY 419 

    0xb225edd6,// 423 PAY 420 

    0xd46838c6,// 424 PAY 421 

    0x1446bcd3,// 425 PAY 422 

    0x6293107e,// 426 PAY 423 

    0xb47e4b4f,// 427 PAY 424 

    0xb3799b76,// 428 PAY 425 

    0x4fb0f62e,// 429 PAY 426 

    0x5cff24a0,// 430 PAY 427 

    0x2af2cbdd,// 431 PAY 428 

    0x45dc0456,// 432 PAY 429 

    0x3388cb53,// 433 PAY 430 

    0x92381966,// 434 PAY 431 

    0xd8c574bb,// 435 PAY 432 

    0x5e788dfb,// 436 PAY 433 

    0x25f7b1eb,// 437 PAY 434 

    0x7bebde96,// 438 PAY 435 

    0xa8dfc78a,// 439 PAY 436 

    0xba300dd7,// 440 PAY 437 

    0x71d6ed5a,// 441 PAY 438 

    0x4aa52642,// 442 PAY 439 

    0x75908772,// 443 PAY 440 

    0xdb9f8a2c,// 444 PAY 441 

    0x494c8ffb,// 445 PAY 442 

    0xe304fc85,// 446 PAY 443 

    0x317f98fa,// 447 PAY 444 

    0x2c4e6721,// 448 PAY 445 

    0xca5177aa,// 449 PAY 446 

    0x726d4363,// 450 PAY 447 

    0x9e6d65d5,// 451 PAY 448 

    0xb760f9d3,// 452 PAY 449 

    0x7d5dffbb,// 453 PAY 450 

    0x158f3ed0,// 454 PAY 451 

    0x382aecb1,// 455 PAY 452 

    0xb8c8491a,// 456 PAY 453 

    0x079cd241,// 457 PAY 454 

    0xcd4c3fd1,// 458 PAY 455 

    0xb9b46b11,// 459 PAY 456 

    0x9dd9cc98,// 460 PAY 457 

    0x7f7da70d,// 461 PAY 458 

    0xcb1ba2ec,// 462 PAY 459 

    0x9d09a013,// 463 PAY 460 

    0xf9542abb,// 464 PAY 461 

    0x8a214241,// 465 PAY 462 

    0xfd48693c,// 466 PAY 463 

    0xd80b504d,// 467 PAY 464 

    0xa2d21b4e,// 468 PAY 465 

    0xc0d0d50a,// 469 PAY 466 

    0xe0ac183e,// 470 PAY 467 

    0xba54c014,// 471 PAY 468 

    0xfc3c6b37,// 472 PAY 469 

    0x4a13fd6b,// 473 PAY 470 

    0x329a3863,// 474 PAY 471 

    0x5c095a75,// 475 PAY 472 

    0x32017b59,// 476 PAY 473 

    0x5d3b4691,// 477 PAY 474 

    0xea619a28,// 478 PAY 475 

    0x3b2d849f,// 479 PAY 476 

    0xd4c7e015,// 480 PAY 477 

    0x21d10239,// 481 PAY 478 

    0xd9316041,// 482 PAY 479 

    0x9a0297dd,// 483 PAY 480 

    0xecb87ba1,// 484 PAY 481 

    0x8340a146,// 485 PAY 482 

    0x5c27ad28,// 486 PAY 483 

    0xdf2ab27e,// 487 PAY 484 

    0xcca40895,// 488 PAY 485 

    0x6f428cd8,// 489 PAY 486 

    0x83479e71,// 490 PAY 487 

    0x7535899e,// 491 PAY 488 

    0x3d37768a,// 492 PAY 489 

    0x0e7cb0e5,// 493 PAY 490 

    0x3b12cc6f,// 494 PAY 491 

    0xf5e1bbd6,// 495 PAY 492 

    0x35bd88de,// 496 PAY 493 

    0x364f54d5,// 497 PAY 494 

    0x85270d4d,// 498 PAY 495 

/// STA is 1 words. 

/// STA num_pkts       : 173 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3f 

    0x02cd3fad // 499 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt46_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 403 words. 

/// BDA size     is 1605 (0x645) 

/// BDA id       is 0x3c4b 

    0x06453c4b,// 3 BDA   1 

/// PAY Generic Data size   : 1605 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0fb295dd,// 4 PAY   1 

    0x266ef1ab,// 5 PAY   2 

    0xb4eed800,// 6 PAY   3 

    0xcaadb61a,// 7 PAY   4 

    0x45be5225,// 8 PAY   5 

    0x99a73398,// 9 PAY   6 

    0x196ed03a,// 10 PAY   7 

    0xfd7f23b6,// 11 PAY   8 

    0x589a9b37,// 12 PAY   9 

    0x6c430a78,// 13 PAY  10 

    0x43061ea9,// 14 PAY  11 

    0x7b739cb2,// 15 PAY  12 

    0x96992abb,// 16 PAY  13 

    0x65002627,// 17 PAY  14 

    0xef2039f9,// 18 PAY  15 

    0x7703fbee,// 19 PAY  16 

    0x7fcfa321,// 20 PAY  17 

    0x978d8222,// 21 PAY  18 

    0x4399d677,// 22 PAY  19 

    0x64539cf9,// 23 PAY  20 

    0x654634cb,// 24 PAY  21 

    0x3f38ce25,// 25 PAY  22 

    0x95c877a5,// 26 PAY  23 

    0x59703ce9,// 27 PAY  24 

    0xf722668f,// 28 PAY  25 

    0xbab23149,// 29 PAY  26 

    0xe77a46d4,// 30 PAY  27 

    0xc4c13441,// 31 PAY  28 

    0x5e850648,// 32 PAY  29 

    0x05df9b12,// 33 PAY  30 

    0xb109823c,// 34 PAY  31 

    0x33def586,// 35 PAY  32 

    0x4af8f8e4,// 36 PAY  33 

    0xa4fabcc1,// 37 PAY  34 

    0xa968949c,// 38 PAY  35 

    0x8b8f8443,// 39 PAY  36 

    0x8a6b4933,// 40 PAY  37 

    0xa95d84f7,// 41 PAY  38 

    0x35abfb52,// 42 PAY  39 

    0xe4ed1f03,// 43 PAY  40 

    0xecb9dad4,// 44 PAY  41 

    0xce1db557,// 45 PAY  42 

    0xf6fff7d3,// 46 PAY  43 

    0xdbf8dc80,// 47 PAY  44 

    0xc6c7883f,// 48 PAY  45 

    0xe9e5ea18,// 49 PAY  46 

    0x14205f28,// 50 PAY  47 

    0x0d6e806c,// 51 PAY  48 

    0x29f4d671,// 52 PAY  49 

    0x6c97d8b9,// 53 PAY  50 

    0xc9043cfd,// 54 PAY  51 

    0xb2c63235,// 55 PAY  52 

    0xd882f1fb,// 56 PAY  53 

    0x43995437,// 57 PAY  54 

    0x7589fb0a,// 58 PAY  55 

    0x0c299c47,// 59 PAY  56 

    0xa9a98801,// 60 PAY  57 

    0x5daaa565,// 61 PAY  58 

    0x3269121f,// 62 PAY  59 

    0xc04b6afc,// 63 PAY  60 

    0x289e3273,// 64 PAY  61 

    0x90fd715e,// 65 PAY  62 

    0xcb16247d,// 66 PAY  63 

    0x789cb769,// 67 PAY  64 

    0x15d83b09,// 68 PAY  65 

    0x39d94c6c,// 69 PAY  66 

    0x0302fc82,// 70 PAY  67 

    0x295fc49f,// 71 PAY  68 

    0xef2d6b35,// 72 PAY  69 

    0x8a94f163,// 73 PAY  70 

    0xf3c4f1b7,// 74 PAY  71 

    0x1cfa812f,// 75 PAY  72 

    0x1c7caafb,// 76 PAY  73 

    0xd9c60e22,// 77 PAY  74 

    0xc53d5526,// 78 PAY  75 

    0x35a12ebd,// 79 PAY  76 

    0x98d6a0dc,// 80 PAY  77 

    0x7c26ad3a,// 81 PAY  78 

    0xf7a31d39,// 82 PAY  79 

    0x8ab07f0a,// 83 PAY  80 

    0xa8d38cb2,// 84 PAY  81 

    0xd44456e6,// 85 PAY  82 

    0x1478cb9f,// 86 PAY  83 

    0xf1b4e491,// 87 PAY  84 

    0xa170a484,// 88 PAY  85 

    0xd6a1bfb8,// 89 PAY  86 

    0xe9f1efe4,// 90 PAY  87 

    0x5ed7e966,// 91 PAY  88 

    0x06e51c2b,// 92 PAY  89 

    0xcb807729,// 93 PAY  90 

    0x9da68b95,// 94 PAY  91 

    0x5f28659b,// 95 PAY  92 

    0xca21740a,// 96 PAY  93 

    0xf9fc624d,// 97 PAY  94 

    0xf3cbb9f4,// 98 PAY  95 

    0xe3cdbbdb,// 99 PAY  96 

    0xb2c80166,// 100 PAY  97 

    0xc7a905fc,// 101 PAY  98 

    0x38196ee2,// 102 PAY  99 

    0x6c606c0f,// 103 PAY 100 

    0xe29136d6,// 104 PAY 101 

    0x18ced30c,// 105 PAY 102 

    0xb319beab,// 106 PAY 103 

    0xe9b48597,// 107 PAY 104 

    0x25638b81,// 108 PAY 105 

    0x76ba21bb,// 109 PAY 106 

    0x0df0ea14,// 110 PAY 107 

    0x152942f7,// 111 PAY 108 

    0x284a3ebc,// 112 PAY 109 

    0xf7fde00b,// 113 PAY 110 

    0xcbe2302a,// 114 PAY 111 

    0xea4b4668,// 115 PAY 112 

    0x000bc1bb,// 116 PAY 113 

    0xdad3a017,// 117 PAY 114 

    0x63a417cb,// 118 PAY 115 

    0x745f4599,// 119 PAY 116 

    0xfefeca47,// 120 PAY 117 

    0x2bc00df0,// 121 PAY 118 

    0x9fd49a20,// 122 PAY 119 

    0xe16cba5b,// 123 PAY 120 

    0x95e138bb,// 124 PAY 121 

    0x87335311,// 125 PAY 122 

    0xf6130723,// 126 PAY 123 

    0xb45e1814,// 127 PAY 124 

    0x788db2ec,// 128 PAY 125 

    0x8525b075,// 129 PAY 126 

    0x9549c2f6,// 130 PAY 127 

    0xa0d84e08,// 131 PAY 128 

    0x3d3e2691,// 132 PAY 129 

    0x91607508,// 133 PAY 130 

    0x11d253b9,// 134 PAY 131 

    0xc83f742b,// 135 PAY 132 

    0x1718e38b,// 136 PAY 133 

    0xef833287,// 137 PAY 134 

    0xfe1f6812,// 138 PAY 135 

    0x70bf994d,// 139 PAY 136 

    0x5a1afc25,// 140 PAY 137 

    0x8e2548ba,// 141 PAY 138 

    0x298a6337,// 142 PAY 139 

    0x4049a529,// 143 PAY 140 

    0xe83c381b,// 144 PAY 141 

    0x1d055fe3,// 145 PAY 142 

    0x6c0d8f86,// 146 PAY 143 

    0x91fb0b39,// 147 PAY 144 

    0x2e2a136b,// 148 PAY 145 

    0x45a1ec65,// 149 PAY 146 

    0xe1a6f587,// 150 PAY 147 

    0xbce45721,// 151 PAY 148 

    0x5e5690e5,// 152 PAY 149 

    0x77a67d53,// 153 PAY 150 

    0xe58b9e44,// 154 PAY 151 

    0xea384163,// 155 PAY 152 

    0xab41d395,// 156 PAY 153 

    0x07feb167,// 157 PAY 154 

    0xb5451eb7,// 158 PAY 155 

    0x822f0861,// 159 PAY 156 

    0x328662db,// 160 PAY 157 

    0xb3e29895,// 161 PAY 158 

    0xf31b5bff,// 162 PAY 159 

    0x0f6a3237,// 163 PAY 160 

    0x822ef6ad,// 164 PAY 161 

    0x8234f939,// 165 PAY 162 

    0x0c0e2e59,// 166 PAY 163 

    0x29c70b13,// 167 PAY 164 

    0x2c2615fa,// 168 PAY 165 

    0x59765752,// 169 PAY 166 

    0x610ad2cd,// 170 PAY 167 

    0x57feb738,// 171 PAY 168 

    0xe330e3f1,// 172 PAY 169 

    0xc510b09b,// 173 PAY 170 

    0x8c536989,// 174 PAY 171 

    0xf0c242ed,// 175 PAY 172 

    0x28419513,// 176 PAY 173 

    0xd7258f07,// 177 PAY 174 

    0x9ebb70c1,// 178 PAY 175 

    0x8484e3a4,// 179 PAY 176 

    0xf221cc7b,// 180 PAY 177 

    0x35fddfa4,// 181 PAY 178 

    0xbe44f2e2,// 182 PAY 179 

    0xef8bd01a,// 183 PAY 180 

    0x7da59f86,// 184 PAY 181 

    0x30552797,// 185 PAY 182 

    0x4d79e39d,// 186 PAY 183 

    0x03baadd0,// 187 PAY 184 

    0x84243eb7,// 188 PAY 185 

    0x42359493,// 189 PAY 186 

    0x1db9ebe1,// 190 PAY 187 

    0xfb9c5156,// 191 PAY 188 

    0x4a334f1e,// 192 PAY 189 

    0x8f109adb,// 193 PAY 190 

    0x67292c18,// 194 PAY 191 

    0x55a04dd1,// 195 PAY 192 

    0x6a2fc0cf,// 196 PAY 193 

    0x4c016402,// 197 PAY 194 

    0xf9522d34,// 198 PAY 195 

    0x06262be5,// 199 PAY 196 

    0x5ce91d5f,// 200 PAY 197 

    0xc8716726,// 201 PAY 198 

    0x291a35d5,// 202 PAY 199 

    0x75dbf21c,// 203 PAY 200 

    0x329e9eef,// 204 PAY 201 

    0x0a6497ad,// 205 PAY 202 

    0x4b69a1b9,// 206 PAY 203 

    0x8fadf5df,// 207 PAY 204 

    0x7cbf5db3,// 208 PAY 205 

    0x29b88a74,// 209 PAY 206 

    0x7a86b89b,// 210 PAY 207 

    0xec9c8a8b,// 211 PAY 208 

    0xb98cc125,// 212 PAY 209 

    0xcfff294a,// 213 PAY 210 

    0x81367acd,// 214 PAY 211 

    0xc0888b96,// 215 PAY 212 

    0x61e91564,// 216 PAY 213 

    0x17a240d0,// 217 PAY 214 

    0xcd7c8189,// 218 PAY 215 

    0xe0fbb189,// 219 PAY 216 

    0x9ef9a593,// 220 PAY 217 

    0x2e72e775,// 221 PAY 218 

    0x2aff25c4,// 222 PAY 219 

    0x0b05a63e,// 223 PAY 220 

    0xa3911b3c,// 224 PAY 221 

    0x79a0932e,// 225 PAY 222 

    0x37258685,// 226 PAY 223 

    0xbf703731,// 227 PAY 224 

    0xebad2936,// 228 PAY 225 

    0xc00d84b0,// 229 PAY 226 

    0x093cc8b1,// 230 PAY 227 

    0xbc37110f,// 231 PAY 228 

    0x6eab4d41,// 232 PAY 229 

    0x28885975,// 233 PAY 230 

    0xb42f1cde,// 234 PAY 231 

    0xe0171a5d,// 235 PAY 232 

    0xcb442bd7,// 236 PAY 233 

    0x6c9c495b,// 237 PAY 234 

    0xdf760b14,// 238 PAY 235 

    0x9227ae77,// 239 PAY 236 

    0xaa36882c,// 240 PAY 237 

    0x0131126a,// 241 PAY 238 

    0x121ca70b,// 242 PAY 239 

    0x694e6301,// 243 PAY 240 

    0xbca2f6f9,// 244 PAY 241 

    0xa14f2e0a,// 245 PAY 242 

    0xb08b1b64,// 246 PAY 243 

    0x1bb801fb,// 247 PAY 244 

    0xc37ab33d,// 248 PAY 245 

    0x3c71766d,// 249 PAY 246 

    0x7078ce20,// 250 PAY 247 

    0x9f72147a,// 251 PAY 248 

    0x4422d1b3,// 252 PAY 249 

    0x9622c299,// 253 PAY 250 

    0x77427dea,// 254 PAY 251 

    0xeaea4fbd,// 255 PAY 252 

    0xcad0f24f,// 256 PAY 253 

    0x8dd25f57,// 257 PAY 254 

    0x3eda24c4,// 258 PAY 255 

    0x1e453343,// 259 PAY 256 

    0xfdd14321,// 260 PAY 257 

    0x60727473,// 261 PAY 258 

    0xffa202f7,// 262 PAY 259 

    0x78f53966,// 263 PAY 260 

    0x8b08129c,// 264 PAY 261 

    0x82de429d,// 265 PAY 262 

    0x2373d9b2,// 266 PAY 263 

    0xa84d30d3,// 267 PAY 264 

    0x147d3b4f,// 268 PAY 265 

    0xbba408df,// 269 PAY 266 

    0x8703d41f,// 270 PAY 267 

    0x69ffe851,// 271 PAY 268 

    0xf8f96324,// 272 PAY 269 

    0xa3a610f4,// 273 PAY 270 

    0xa04bd362,// 274 PAY 271 

    0xe9323e70,// 275 PAY 272 

    0x1a4c2e05,// 276 PAY 273 

    0x4279bde6,// 277 PAY 274 

    0x6965fe18,// 278 PAY 275 

    0x9f921037,// 279 PAY 276 

    0x4f3ed37c,// 280 PAY 277 

    0x0295e8cd,// 281 PAY 278 

    0xe467b2f1,// 282 PAY 279 

    0x6f0b5c47,// 283 PAY 280 

    0x2bbd75da,// 284 PAY 281 

    0xb4191f6b,// 285 PAY 282 

    0xe4db4c4d,// 286 PAY 283 

    0x435ece79,// 287 PAY 284 

    0x7b2acf08,// 288 PAY 285 

    0x35b6408b,// 289 PAY 286 

    0x4db06547,// 290 PAY 287 

    0x6bc0ca96,// 291 PAY 288 

    0x17d4b2fd,// 292 PAY 289 

    0xf489974b,// 293 PAY 290 

    0x4e709f98,// 294 PAY 291 

    0x71f6e530,// 295 PAY 292 

    0xfcf22b6d,// 296 PAY 293 

    0x3af87161,// 297 PAY 294 

    0x5d533ebd,// 298 PAY 295 

    0xdce53db1,// 299 PAY 296 

    0x272f980e,// 300 PAY 297 

    0x882ace3e,// 301 PAY 298 

    0x0a55bbb8,// 302 PAY 299 

    0xc9abe753,// 303 PAY 300 

    0x984a44ca,// 304 PAY 301 

    0xae511ce0,// 305 PAY 302 

    0x2ffd2764,// 306 PAY 303 

    0x02d35c76,// 307 PAY 304 

    0x02f64cac,// 308 PAY 305 

    0x57a976e9,// 309 PAY 306 

    0x56d8cebe,// 310 PAY 307 

    0x5135e6bc,// 311 PAY 308 

    0x8ad9ddec,// 312 PAY 309 

    0xfb904e27,// 313 PAY 310 

    0x71794eeb,// 314 PAY 311 

    0x03e6bebc,// 315 PAY 312 

    0x5caa2de4,// 316 PAY 313 

    0xb81cc043,// 317 PAY 314 

    0x15c50535,// 318 PAY 315 

    0x4e844aac,// 319 PAY 316 

    0xa03bdda2,// 320 PAY 317 

    0x41af51a8,// 321 PAY 318 

    0x705c2ffe,// 322 PAY 319 

    0xcc7bf6fd,// 323 PAY 320 

    0xcda17fe3,// 324 PAY 321 

    0x4360056d,// 325 PAY 322 

    0xda1b716e,// 326 PAY 323 

    0x3c4a9bf0,// 327 PAY 324 

    0x3720ea01,// 328 PAY 325 

    0x712303f2,// 329 PAY 326 

    0x040e3783,// 330 PAY 327 

    0xf20e9366,// 331 PAY 328 

    0x163d04f0,// 332 PAY 329 

    0x1d41fe8e,// 333 PAY 330 

    0x08f0c411,// 334 PAY 331 

    0x9f239e23,// 335 PAY 332 

    0x23e75e58,// 336 PAY 333 

    0xfb557d4b,// 337 PAY 334 

    0x75e9a125,// 338 PAY 335 

    0x1a3c9dfe,// 339 PAY 336 

    0xe95342a4,// 340 PAY 337 

    0x920104b2,// 341 PAY 338 

    0x3a44ac4a,// 342 PAY 339 

    0x07e0aa4f,// 343 PAY 340 

    0x21aff1f5,// 344 PAY 341 

    0x097ef1f1,// 345 PAY 342 

    0x9628c829,// 346 PAY 343 

    0x72a6254e,// 347 PAY 344 

    0x3d8a4ae4,// 348 PAY 345 

    0x60280f20,// 349 PAY 346 

    0x7f4ca83b,// 350 PAY 347 

    0x89cc29b9,// 351 PAY 348 

    0x5e5d3017,// 352 PAY 349 

    0x94cb84d5,// 353 PAY 350 

    0x499857c5,// 354 PAY 351 

    0x2853d131,// 355 PAY 352 

    0xdd7d83ec,// 356 PAY 353 

    0x2fb23857,// 357 PAY 354 

    0x66f302e6,// 358 PAY 355 

    0x66da4973,// 359 PAY 356 

    0x1dc72e97,// 360 PAY 357 

    0x864f920a,// 361 PAY 358 

    0xd66e6564,// 362 PAY 359 

    0x2591a852,// 363 PAY 360 

    0x7b1a47a2,// 364 PAY 361 

    0x5d760790,// 365 PAY 362 

    0x7655855a,// 366 PAY 363 

    0x23b65073,// 367 PAY 364 

    0x0fb71e8d,// 368 PAY 365 

    0x1e96f0de,// 369 PAY 366 

    0xc46be294,// 370 PAY 367 

    0xfd7b39a2,// 371 PAY 368 

    0xbf5794de,// 372 PAY 369 

    0x0d45adb1,// 373 PAY 370 

    0x152b0148,// 374 PAY 371 

    0x1dae3041,// 375 PAY 372 

    0xec427c3b,// 376 PAY 373 

    0x4ce3a7e4,// 377 PAY 374 

    0xf484cc36,// 378 PAY 375 

    0xeaa08481,// 379 PAY 376 

    0xb99d731b,// 380 PAY 377 

    0x74c21f26,// 381 PAY 378 

    0x75e5460c,// 382 PAY 379 

    0x2e61e9c1,// 383 PAY 380 

    0x6e517c73,// 384 PAY 381 

    0x0b32d726,// 385 PAY 382 

    0x21cf0bc4,// 386 PAY 383 

    0x1b5e0ce2,// 387 PAY 384 

    0xe7100bbc,// 388 PAY 385 

    0xeee1c01a,// 389 PAY 386 

    0x67e272dd,// 390 PAY 387 

    0x5e824666,// 391 PAY 388 

    0x70451715,// 392 PAY 389 

    0x130952f9,// 393 PAY 390 

    0x4d6ff172,// 394 PAY 391 

    0x89362f4d,// 395 PAY 392 

    0x79434b51,// 396 PAY 393 

    0x88d768b4,// 397 PAY 394 

    0xeb9b8a45,// 398 PAY 395 

    0xe948ec2a,// 399 PAY 396 

    0x740092d5,// 400 PAY 397 

    0xaca97ca3,// 401 PAY 398 

    0xb642fcf8,// 402 PAY 399 

    0x352a432c,// 403 PAY 400 

    0x1e37b583,// 404 PAY 401 

    0x58000000,// 405 PAY 402 

/// STA is 1 words. 

/// STA num_pkts       : 100 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6f 

    0x02616f64 // 406 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt47_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 488 words. 

/// BDA size     is 1945 (0x799) 

/// BDA id       is 0x4ac8 

    0x07994ac8,// 3 BDA   1 

/// PAY Generic Data size   : 1945 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x17f35a00,// 4 PAY   1 

    0x0ded7f2e,// 5 PAY   2 

    0xb71a4020,// 6 PAY   3 

    0x5c393370,// 7 PAY   4 

    0xed06343d,// 8 PAY   5 

    0xd3741901,// 9 PAY   6 

    0x3cf8dd80,// 10 PAY   7 

    0x350f900c,// 11 PAY   8 

    0x321fb5c2,// 12 PAY   9 

    0xee83bb94,// 13 PAY  10 

    0x2685ebdc,// 14 PAY  11 

    0x31d7f6c7,// 15 PAY  12 

    0x8c2d5b2a,// 16 PAY  13 

    0x62f1a843,// 17 PAY  14 

    0x6ba8297a,// 18 PAY  15 

    0x2ed49060,// 19 PAY  16 

    0x4f38675b,// 20 PAY  17 

    0x8795a9fa,// 21 PAY  18 

    0x46b9c08b,// 22 PAY  19 

    0xa83a873e,// 23 PAY  20 

    0xd9acf3d3,// 24 PAY  21 

    0x400d34ec,// 25 PAY  22 

    0x2a059c1c,// 26 PAY  23 

    0x022d0f89,// 27 PAY  24 

    0xf6650390,// 28 PAY  25 

    0x9b2d98ec,// 29 PAY  26 

    0xcda10488,// 30 PAY  27 

    0xf22e50cd,// 31 PAY  28 

    0x19a7dd6b,// 32 PAY  29 

    0xc988a04f,// 33 PAY  30 

    0xd46db5d5,// 34 PAY  31 

    0x7b6a0882,// 35 PAY  32 

    0xa1c64a84,// 36 PAY  33 

    0x1c244e06,// 37 PAY  34 

    0x1e845345,// 38 PAY  35 

    0xefadf28d,// 39 PAY  36 

    0xc8b20def,// 40 PAY  37 

    0xc3affeef,// 41 PAY  38 

    0x0f36b879,// 42 PAY  39 

    0xf7275e36,// 43 PAY  40 

    0x6325b9c0,// 44 PAY  41 

    0x3986845a,// 45 PAY  42 

    0xd01f43f8,// 46 PAY  43 

    0x8a5ac832,// 47 PAY  44 

    0x78952ead,// 48 PAY  45 

    0x862b075d,// 49 PAY  46 

    0x4fb6941d,// 50 PAY  47 

    0xf5054872,// 51 PAY  48 

    0x742e8f9f,// 52 PAY  49 

    0xd30d52a7,// 53 PAY  50 

    0x3dfa95a6,// 54 PAY  51 

    0x5b1918d2,// 55 PAY  52 

    0x632d6b7f,// 56 PAY  53 

    0x1867ae34,// 57 PAY  54 

    0x62e0eb60,// 58 PAY  55 

    0x54832774,// 59 PAY  56 

    0x723f007a,// 60 PAY  57 

    0x4a291b7d,// 61 PAY  58 

    0xb18e3a0d,// 62 PAY  59 

    0x0e18628c,// 63 PAY  60 

    0x15efb77c,// 64 PAY  61 

    0xb6f850f1,// 65 PAY  62 

    0x1a494bb1,// 66 PAY  63 

    0x10285c1b,// 67 PAY  64 

    0x3b55d23f,// 68 PAY  65 

    0x77763c81,// 69 PAY  66 

    0xa94a418a,// 70 PAY  67 

    0x051087d8,// 71 PAY  68 

    0x791fd616,// 72 PAY  69 

    0xcc051808,// 73 PAY  70 

    0xfe2cf169,// 74 PAY  71 

    0x9a955c31,// 75 PAY  72 

    0x1b67283f,// 76 PAY  73 

    0xf15c78fe,// 77 PAY  74 

    0x92ebeb70,// 78 PAY  75 

    0x7480f49d,// 79 PAY  76 

    0x80370a37,// 80 PAY  77 

    0xae2b9984,// 81 PAY  78 

    0x010c8966,// 82 PAY  79 

    0x288f7b31,// 83 PAY  80 

    0xdec5b5ab,// 84 PAY  81 

    0x70542d20,// 85 PAY  82 

    0x29d06881,// 86 PAY  83 

    0x1734f7c1,// 87 PAY  84 

    0xfaa08535,// 88 PAY  85 

    0xca9345e1,// 89 PAY  86 

    0xfff691a2,// 90 PAY  87 

    0xe4c78df8,// 91 PAY  88 

    0x1f447709,// 92 PAY  89 

    0x5e938c9c,// 93 PAY  90 

    0xf8107fef,// 94 PAY  91 

    0x9a6a0ac2,// 95 PAY  92 

    0x9fe0c207,// 96 PAY  93 

    0xd7b35a65,// 97 PAY  94 

    0xfd6301df,// 98 PAY  95 

    0xc0b32da3,// 99 PAY  96 

    0x60964545,// 100 PAY  97 

    0x1948dbda,// 101 PAY  98 

    0x7b716c47,// 102 PAY  99 

    0x32a3eb97,// 103 PAY 100 

    0x7f0f46f6,// 104 PAY 101 

    0xfb6e53e7,// 105 PAY 102 

    0xeb426bc1,// 106 PAY 103 

    0xde8ae597,// 107 PAY 104 

    0x05531453,// 108 PAY 105 

    0x41f609b8,// 109 PAY 106 

    0x04cea21c,// 110 PAY 107 

    0x19068d00,// 111 PAY 108 

    0xc917ccc7,// 112 PAY 109 

    0x3d291894,// 113 PAY 110 

    0x91bd823d,// 114 PAY 111 

    0xf58b968b,// 115 PAY 112 

    0xce978ab3,// 116 PAY 113 

    0xa8d77d05,// 117 PAY 114 

    0xb9c0a016,// 118 PAY 115 

    0xa6a5fd63,// 119 PAY 116 

    0x93f0057d,// 120 PAY 117 

    0xd846882e,// 121 PAY 118 

    0xc926adb5,// 122 PAY 119 

    0x10ca82ca,// 123 PAY 120 

    0x49061f28,// 124 PAY 121 

    0x68c75ae2,// 125 PAY 122 

    0x5441c969,// 126 PAY 123 

    0x1c649dc8,// 127 PAY 124 

    0x7d99a73b,// 128 PAY 125 

    0x30b379cc,// 129 PAY 126 

    0xdbf77a7c,// 130 PAY 127 

    0x6c6f3bf1,// 131 PAY 128 

    0x4809dcbc,// 132 PAY 129 

    0x4bca84ff,// 133 PAY 130 

    0xbf5d1685,// 134 PAY 131 

    0xc3baa784,// 135 PAY 132 

    0x9ce4da13,// 136 PAY 133 

    0xa42b7eac,// 137 PAY 134 

    0xf7aea91f,// 138 PAY 135 

    0xbb7f6d1c,// 139 PAY 136 

    0xfa1cb82e,// 140 PAY 137 

    0x747988b5,// 141 PAY 138 

    0x2e2c688d,// 142 PAY 139 

    0xcba33a85,// 143 PAY 140 

    0xa44a2f73,// 144 PAY 141 

    0xcd217896,// 145 PAY 142 

    0x86349007,// 146 PAY 143 

    0xc4c3e7b2,// 147 PAY 144 

    0x0fd85f98,// 148 PAY 145 

    0x615a65c1,// 149 PAY 146 

    0xdc7e12f6,// 150 PAY 147 

    0xa6e3c618,// 151 PAY 148 

    0x64a2f546,// 152 PAY 149 

    0xfe779dca,// 153 PAY 150 

    0x8641e2e2,// 154 PAY 151 

    0xbbb65b9c,// 155 PAY 152 

    0x1b573d7d,// 156 PAY 153 

    0xd4c24ac8,// 157 PAY 154 

    0x9747b2ef,// 158 PAY 155 

    0xf5a7371c,// 159 PAY 156 

    0x3ef77425,// 160 PAY 157 

    0x8f9f3dd4,// 161 PAY 158 

    0x7687aee4,// 162 PAY 159 

    0x1bdb27fe,// 163 PAY 160 

    0x06014166,// 164 PAY 161 

    0xadec45b4,// 165 PAY 162 

    0x6a3973c6,// 166 PAY 163 

    0x41df9d10,// 167 PAY 164 

    0xf2268e6e,// 168 PAY 165 

    0x784893b1,// 169 PAY 166 

    0x8242c4b3,// 170 PAY 167 

    0xb171fd22,// 171 PAY 168 

    0x38ee964f,// 172 PAY 169 

    0x838479e7,// 173 PAY 170 

    0xd65f0cf1,// 174 PAY 171 

    0x47c896ef,// 175 PAY 172 

    0x5e41fd91,// 176 PAY 173 

    0xea4cb478,// 177 PAY 174 

    0xf3924d2f,// 178 PAY 175 

    0xb63fe9ce,// 179 PAY 176 

    0xfdca432e,// 180 PAY 177 

    0x0f326439,// 181 PAY 178 

    0x1143c356,// 182 PAY 179 

    0xaae994a6,// 183 PAY 180 

    0x1d3447e0,// 184 PAY 181 

    0x0b1a9a9c,// 185 PAY 182 

    0x53781d9e,// 186 PAY 183 

    0xf59f7fe0,// 187 PAY 184 

    0x9042a27b,// 188 PAY 185 

    0xf0e0737f,// 189 PAY 186 

    0x62d6d57f,// 190 PAY 187 

    0x24c6930f,// 191 PAY 188 

    0x726c6bf4,// 192 PAY 189 

    0xc8586970,// 193 PAY 190 

    0xe1992a2c,// 194 PAY 191 

    0x6f31a3cf,// 195 PAY 192 

    0xa9f2831b,// 196 PAY 193 

    0x1c1b8a89,// 197 PAY 194 

    0xd059bf35,// 198 PAY 195 

    0x8b269a02,// 199 PAY 196 

    0xd987e778,// 200 PAY 197 

    0xaf4ae694,// 201 PAY 198 

    0x1db17da6,// 202 PAY 199 

    0xf1c4dc01,// 203 PAY 200 

    0xe581f391,// 204 PAY 201 

    0x7d5a4e26,// 205 PAY 202 

    0x5af036f5,// 206 PAY 203 

    0xede7022b,// 207 PAY 204 

    0x0297c64c,// 208 PAY 205 

    0x1c7d3e76,// 209 PAY 206 

    0x7520d912,// 210 PAY 207 

    0xdd094c07,// 211 PAY 208 

    0xfd01914e,// 212 PAY 209 

    0x6c0e65c2,// 213 PAY 210 

    0x6de7e500,// 214 PAY 211 

    0x16afdfac,// 215 PAY 212 

    0xe428d327,// 216 PAY 213 

    0x76514834,// 217 PAY 214 

    0x15dc13b1,// 218 PAY 215 

    0xba0a1b1f,// 219 PAY 216 

    0xa2b5b1e8,// 220 PAY 217 

    0x452e3a3a,// 221 PAY 218 

    0x20fa34b5,// 222 PAY 219 

    0x67be082e,// 223 PAY 220 

    0x87c530e4,// 224 PAY 221 

    0x7302ec7c,// 225 PAY 222 

    0xbc79ad67,// 226 PAY 223 

    0x7c097852,// 227 PAY 224 

    0xa608a0ca,// 228 PAY 225 

    0xf2543939,// 229 PAY 226 

    0x71e42b03,// 230 PAY 227 

    0xf6b82f2a,// 231 PAY 228 

    0x75651669,// 232 PAY 229 

    0x6dff3192,// 233 PAY 230 

    0xba146e65,// 234 PAY 231 

    0xa7bc81fa,// 235 PAY 232 

    0x999169fe,// 236 PAY 233 

    0xf86d28c9,// 237 PAY 234 

    0xb4de706a,// 238 PAY 235 

    0xbc6fe474,// 239 PAY 236 

    0xf2186d3f,// 240 PAY 237 

    0x184751af,// 241 PAY 238 

    0x87a50325,// 242 PAY 239 

    0x9396527d,// 243 PAY 240 

    0x20384bb7,// 244 PAY 241 

    0x758a955f,// 245 PAY 242 

    0x6f3a0678,// 246 PAY 243 

    0x843c0543,// 247 PAY 244 

    0xb4b4976f,// 248 PAY 245 

    0x1300db5f,// 249 PAY 246 

    0x8adea50e,// 250 PAY 247 

    0x35330ec4,// 251 PAY 248 

    0x252c05f0,// 252 PAY 249 

    0x5e7a3ae7,// 253 PAY 250 

    0x8ed78b73,// 254 PAY 251 

    0x35783b62,// 255 PAY 252 

    0x1532d9a1,// 256 PAY 253 

    0xd621620b,// 257 PAY 254 

    0x3512e33c,// 258 PAY 255 

    0xc31ec95f,// 259 PAY 256 

    0xc58e79a5,// 260 PAY 257 

    0x07d314cc,// 261 PAY 258 

    0xf0a4b6e9,// 262 PAY 259 

    0xd348ef19,// 263 PAY 260 

    0x154c4f45,// 264 PAY 261 

    0x241a08da,// 265 PAY 262 

    0x657c4707,// 266 PAY 263 

    0x7ff48c28,// 267 PAY 264 

    0xb8c1d978,// 268 PAY 265 

    0x1e313802,// 269 PAY 266 

    0x19792c7a,// 270 PAY 267 

    0xc4bf66e0,// 271 PAY 268 

    0x9a63390f,// 272 PAY 269 

    0xfc83d88d,// 273 PAY 270 

    0x7c8a6b21,// 274 PAY 271 

    0x8d0b929e,// 275 PAY 272 

    0xebcaa76c,// 276 PAY 273 

    0x1b9002f4,// 277 PAY 274 

    0xc0c2716b,// 278 PAY 275 

    0x93ed1e7d,// 279 PAY 276 

    0xfb5cece0,// 280 PAY 277 

    0xb855c227,// 281 PAY 278 

    0x1bfade2f,// 282 PAY 279 

    0x588dd5c7,// 283 PAY 280 

    0x14f43e88,// 284 PAY 281 

    0x26a87efc,// 285 PAY 282 

    0x47a768a8,// 286 PAY 283 

    0xb16b2e0b,// 287 PAY 284 

    0x85fd3bf2,// 288 PAY 285 

    0xad86df72,// 289 PAY 286 

    0xb9754d6b,// 290 PAY 287 

    0x2d39465e,// 291 PAY 288 

    0x69031601,// 292 PAY 289 

    0xe780a6fa,// 293 PAY 290 

    0xffb14adb,// 294 PAY 291 

    0x37b73ed4,// 295 PAY 292 

    0x88cdb3a5,// 296 PAY 293 

    0x1105974c,// 297 PAY 294 

    0x71b80376,// 298 PAY 295 

    0x3c2a7f0d,// 299 PAY 296 

    0x0741ff82,// 300 PAY 297 

    0x57bf746e,// 301 PAY 298 

    0x1ae91abf,// 302 PAY 299 

    0xf6d9962b,// 303 PAY 300 

    0xf4530633,// 304 PAY 301 

    0x250aae6b,// 305 PAY 302 

    0x30a43816,// 306 PAY 303 

    0x1e08d9ff,// 307 PAY 304 

    0xb4389376,// 308 PAY 305 

    0xa900c725,// 309 PAY 306 

    0xe902a5d6,// 310 PAY 307 

    0xba044446,// 311 PAY 308 

    0xad8c0ff7,// 312 PAY 309 

    0x2db40df7,// 313 PAY 310 

    0x2171e431,// 314 PAY 311 

    0x7b5fb96a,// 315 PAY 312 

    0x0a94724d,// 316 PAY 313 

    0x7d2c2aac,// 317 PAY 314 

    0xf4b999e1,// 318 PAY 315 

    0x8415e425,// 319 PAY 316 

    0x7347b2c8,// 320 PAY 317 

    0x6440c2f5,// 321 PAY 318 

    0x7ce57946,// 322 PAY 319 

    0xfbe70850,// 323 PAY 320 

    0x9ffb2cee,// 324 PAY 321 

    0xdb660e07,// 325 PAY 322 

    0x7245bd37,// 326 PAY 323 

    0x7da5a31d,// 327 PAY 324 

    0x31d9421d,// 328 PAY 325 

    0xd2adf2fd,// 329 PAY 326 

    0x2ca583e9,// 330 PAY 327 

    0xf21f34fe,// 331 PAY 328 

    0xed6db358,// 332 PAY 329 

    0xc266cfc1,// 333 PAY 330 

    0xa8c9a3ce,// 334 PAY 331 

    0xc9338271,// 335 PAY 332 

    0xf5ac9fec,// 336 PAY 333 

    0x050809b2,// 337 PAY 334 

    0x4ec47e52,// 338 PAY 335 

    0x7a1dcc48,// 339 PAY 336 

    0x23c028e6,// 340 PAY 337 

    0x04fc51d2,// 341 PAY 338 

    0xfaa03c58,// 342 PAY 339 

    0x4b068983,// 343 PAY 340 

    0xfe3b136e,// 344 PAY 341 

    0xd12556fc,// 345 PAY 342 

    0xd3e03137,// 346 PAY 343 

    0x3b658115,// 347 PAY 344 

    0x8e9d16b8,// 348 PAY 345 

    0x04e81e69,// 349 PAY 346 

    0xb0820219,// 350 PAY 347 

    0x396b4fbd,// 351 PAY 348 

    0x6baa3e73,// 352 PAY 349 

    0xafcbfac5,// 353 PAY 350 

    0x232c77f7,// 354 PAY 351 

    0x475cbc19,// 355 PAY 352 

    0xe89df8ee,// 356 PAY 353 

    0x272a6820,// 357 PAY 354 

    0x55c7e681,// 358 PAY 355 

    0x76a2f4d0,// 359 PAY 356 

    0x1bc14e5b,// 360 PAY 357 

    0xfabee65d,// 361 PAY 358 

    0x1d4fd39e,// 362 PAY 359 

    0x1228331a,// 363 PAY 360 

    0xb2cdc026,// 364 PAY 361 

    0xe07e5142,// 365 PAY 362 

    0x1361519c,// 366 PAY 363 

    0x3f572156,// 367 PAY 364 

    0x279afc36,// 368 PAY 365 

    0xc453117a,// 369 PAY 366 

    0x9f7bbdba,// 370 PAY 367 

    0xdc8ad25c,// 371 PAY 368 

    0x7d688183,// 372 PAY 369 

    0xe7660349,// 373 PAY 370 

    0x85434596,// 374 PAY 371 

    0x25c42d3c,// 375 PAY 372 

    0xa0f75c1c,// 376 PAY 373 

    0xb74e8619,// 377 PAY 374 

    0xd470183f,// 378 PAY 375 

    0x44b12bb7,// 379 PAY 376 

    0x1447508d,// 380 PAY 377 

    0x796b05eb,// 381 PAY 378 

    0x898cda80,// 382 PAY 379 

    0x18bc4030,// 383 PAY 380 

    0x69e51d20,// 384 PAY 381 

    0x4d597aea,// 385 PAY 382 

    0x32dfdff0,// 386 PAY 383 

    0x4c9c357f,// 387 PAY 384 

    0x175b8c5f,// 388 PAY 385 

    0x49c4bdec,// 389 PAY 386 

    0x73c12c22,// 390 PAY 387 

    0x87d8afd2,// 391 PAY 388 

    0x0cc37825,// 392 PAY 389 

    0xbcba6d08,// 393 PAY 390 

    0xed4815f2,// 394 PAY 391 

    0xffafc7ba,// 395 PAY 392 

    0x385d04d9,// 396 PAY 393 

    0x6c0bf8a1,// 397 PAY 394 

    0x326c1397,// 398 PAY 395 

    0xdd3ef823,// 399 PAY 396 

    0x964be5e7,// 400 PAY 397 

    0xf1ecda4e,// 401 PAY 398 

    0xf7a59f68,// 402 PAY 399 

    0x2120cd38,// 403 PAY 400 

    0xa8be8b0f,// 404 PAY 401 

    0xd7fe6476,// 405 PAY 402 

    0xc0dd664d,// 406 PAY 403 

    0x8ebcf7ab,// 407 PAY 404 

    0xeafec9d7,// 408 PAY 405 

    0xebd149f6,// 409 PAY 406 

    0xaa34a8ef,// 410 PAY 407 

    0x3686ed33,// 411 PAY 408 

    0xd6a2b890,// 412 PAY 409 

    0x89857fa0,// 413 PAY 410 

    0x8648caf0,// 414 PAY 411 

    0xef0ff8b2,// 415 PAY 412 

    0xe961a3d3,// 416 PAY 413 

    0x4888e893,// 417 PAY 414 

    0xf4272965,// 418 PAY 415 

    0xf7250e08,// 419 PAY 416 

    0xade4aaa4,// 420 PAY 417 

    0xb3e0f0ca,// 421 PAY 418 

    0xcb90f908,// 422 PAY 419 

    0xedeaa29b,// 423 PAY 420 

    0xd78fa992,// 424 PAY 421 

    0x7758d0d1,// 425 PAY 422 

    0x16024a96,// 426 PAY 423 

    0xbb519499,// 427 PAY 424 

    0x5db6ab48,// 428 PAY 425 

    0x28aad91a,// 429 PAY 426 

    0x2ed6df1f,// 430 PAY 427 

    0xb2e65132,// 431 PAY 428 

    0x436b96e4,// 432 PAY 429 

    0xa02a84bd,// 433 PAY 430 

    0x1c79f0f3,// 434 PAY 431 

    0xdae9aa38,// 435 PAY 432 

    0xbe251297,// 436 PAY 433 

    0x75b5f232,// 437 PAY 434 

    0xc85fc5e6,// 438 PAY 435 

    0x708e4bf5,// 439 PAY 436 

    0xb50997a0,// 440 PAY 437 

    0x54fac336,// 441 PAY 438 

    0xc95c423c,// 442 PAY 439 

    0xb91ef05b,// 443 PAY 440 

    0x44f99310,// 444 PAY 441 

    0x654b9a9a,// 445 PAY 442 

    0x6c527012,// 446 PAY 443 

    0x968416ee,// 447 PAY 444 

    0x126c236e,// 448 PAY 445 

    0x2d3b0a7b,// 449 PAY 446 

    0x90da20fc,// 450 PAY 447 

    0xb7c10c27,// 451 PAY 448 

    0x51e3462c,// 452 PAY 449 

    0x63dacada,// 453 PAY 450 

    0xe8a2b027,// 454 PAY 451 

    0x20af6b3e,// 455 PAY 452 

    0x588170e7,// 456 PAY 453 

    0xa42efb5c,// 457 PAY 454 

    0x698d4823,// 458 PAY 455 

    0xa02e5de6,// 459 PAY 456 

    0x8fb2e797,// 460 PAY 457 

    0x05973df7,// 461 PAY 458 

    0x86651f38,// 462 PAY 459 

    0x33c73096,// 463 PAY 460 

    0x515fbf78,// 464 PAY 461 

    0xca5c5cd8,// 465 PAY 462 

    0x9299a857,// 466 PAY 463 

    0x4a6a61d9,// 467 PAY 464 

    0x78977eae,// 468 PAY 465 

    0x8f485763,// 469 PAY 466 

    0x2b4d7a16,// 470 PAY 467 

    0x7d721c29,// 471 PAY 468 

    0xa7505e36,// 472 PAY 469 

    0xdba92987,// 473 PAY 470 

    0xe8a29aad,// 474 PAY 471 

    0xed9bc1ff,// 475 PAY 472 

    0xb9c6d742,// 476 PAY 473 

    0x1206646a,// 477 PAY 474 

    0x105d33cb,// 478 PAY 475 

    0xaf4725f3,// 479 PAY 476 

    0x4d1b748c,// 480 PAY 477 

    0xae55ebd8,// 481 PAY 478 

    0x9f5e646a,// 482 PAY 479 

    0x1974445a,// 483 PAY 480 

    0x66c906ab,// 484 PAY 481 

    0x7029a6cd,// 485 PAY 482 

    0xf6fb6cf8,// 486 PAY 483 

    0x39d28743,// 487 PAY 484 

    0xc1f3b39c,// 488 PAY 485 

    0x88fe148a,// 489 PAY 486 

    0xdd000000,// 490 PAY 487 

/// HASH is  8 bytes 

    0xe8a2b027,// 491 HSH   1 

    0x20af6b3e,// 492 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 35 

/// STA pkt_idx        : 128 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xac 

    0x0200ac23 // 493 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt48_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 403 words. 

/// BDA size     is 1605 (0x645) 

/// BDA id       is 0x267c 

    0x0645267c,// 3 BDA   1 

/// PAY Generic Data size   : 1605 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xfd8a6d05,// 4 PAY   1 

    0xb7f7937f,// 5 PAY   2 

    0x9044353c,// 6 PAY   3 

    0xbd0063ee,// 7 PAY   4 

    0x9b77f7c9,// 8 PAY   5 

    0x02691ac2,// 9 PAY   6 

    0x62205994,// 10 PAY   7 

    0x6c9615c4,// 11 PAY   8 

    0x10632f23,// 12 PAY   9 

    0xeba6508c,// 13 PAY  10 

    0xdd0f2b77,// 14 PAY  11 

    0x2136d884,// 15 PAY  12 

    0xcf547425,// 16 PAY  13 

    0x9c51664c,// 17 PAY  14 

    0x7652fb6b,// 18 PAY  15 

    0x06463ea4,// 19 PAY  16 

    0x31774c40,// 20 PAY  17 

    0xe3bd3206,// 21 PAY  18 

    0x731f357a,// 22 PAY  19 

    0x9e26b5d3,// 23 PAY  20 

    0xefbdfec7,// 24 PAY  21 

    0xa89d4c5a,// 25 PAY  22 

    0x977a205d,// 26 PAY  23 

    0x43f4288d,// 27 PAY  24 

    0x7e3283aa,// 28 PAY  25 

    0x2e9dbf83,// 29 PAY  26 

    0x9b254f1b,// 30 PAY  27 

    0x38386642,// 31 PAY  28 

    0x5f08fe9e,// 32 PAY  29 

    0xbfc1f3a6,// 33 PAY  30 

    0xd1185024,// 34 PAY  31 

    0x8bfaa05b,// 35 PAY  32 

    0x3da33c95,// 36 PAY  33 

    0x4edebd12,// 37 PAY  34 

    0xa530da24,// 38 PAY  35 

    0x89862e6b,// 39 PAY  36 

    0xf613f69b,// 40 PAY  37 

    0x24deab06,// 41 PAY  38 

    0x361ed05e,// 42 PAY  39 

    0xbd591020,// 43 PAY  40 

    0x03a4fe2e,// 44 PAY  41 

    0x4507e985,// 45 PAY  42 

    0x75702a5d,// 46 PAY  43 

    0x311d8d49,// 47 PAY  44 

    0xd09d1a42,// 48 PAY  45 

    0x3a7d8349,// 49 PAY  46 

    0x6c6412f2,// 50 PAY  47 

    0xf38c5ec5,// 51 PAY  48 

    0xb088fdef,// 52 PAY  49 

    0x2b8d67e2,// 53 PAY  50 

    0x420e98b4,// 54 PAY  51 

    0xd560d183,// 55 PAY  52 

    0x4e7739e4,// 56 PAY  53 

    0x7e0a9893,// 57 PAY  54 

    0x67ba2a95,// 58 PAY  55 

    0x0cf989d2,// 59 PAY  56 

    0x542f8ed6,// 60 PAY  57 

    0x35c5e8b2,// 61 PAY  58 

    0x8db2bbf2,// 62 PAY  59 

    0x057f245f,// 63 PAY  60 

    0x69192525,// 64 PAY  61 

    0x61044a67,// 65 PAY  62 

    0x9990a72c,// 66 PAY  63 

    0x5e3d6bad,// 67 PAY  64 

    0xe334ebb6,// 68 PAY  65 

    0xe6e8db5e,// 69 PAY  66 

    0x6875240e,// 70 PAY  67 

    0x8655b285,// 71 PAY  68 

    0x103909da,// 72 PAY  69 

    0x18a66106,// 73 PAY  70 

    0xbfa9f389,// 74 PAY  71 

    0xa22aa0b9,// 75 PAY  72 

    0x007fdeb7,// 76 PAY  73 

    0x8de72f78,// 77 PAY  74 

    0x5c4c4619,// 78 PAY  75 

    0xfbe3e0fa,// 79 PAY  76 

    0x0126c2b8,// 80 PAY  77 

    0xb3e39b74,// 81 PAY  78 

    0x0904992b,// 82 PAY  79 

    0x44c54f58,// 83 PAY  80 

    0x0e0870a3,// 84 PAY  81 

    0xc5043605,// 85 PAY  82 

    0x489fc72a,// 86 PAY  83 

    0x4fe12459,// 87 PAY  84 

    0x61b2a10b,// 88 PAY  85 

    0x69dd00cd,// 89 PAY  86 

    0x1173cf58,// 90 PAY  87 

    0xeac9da33,// 91 PAY  88 

    0xf37558d7,// 92 PAY  89 

    0x2ca5f6e5,// 93 PAY  90 

    0x1376a8f8,// 94 PAY  91 

    0xcdf9133e,// 95 PAY  92 

    0x134360c5,// 96 PAY  93 

    0x66105a34,// 97 PAY  94 

    0x06d6fafa,// 98 PAY  95 

    0x4207d9ed,// 99 PAY  96 

    0x92db5f1d,// 100 PAY  97 

    0xc87d3b48,// 101 PAY  98 

    0x3e26501d,// 102 PAY  99 

    0xb5be4a1e,// 103 PAY 100 

    0xbbd9804f,// 104 PAY 101 

    0xf92b461d,// 105 PAY 102 

    0x909ba325,// 106 PAY 103 

    0x3c3c1759,// 107 PAY 104 

    0xf825f482,// 108 PAY 105 

    0x8d461936,// 109 PAY 106 

    0xc9959efe,// 110 PAY 107 

    0x8d15949a,// 111 PAY 108 

    0xef6a90a4,// 112 PAY 109 

    0xab0822f8,// 113 PAY 110 

    0xc9696fde,// 114 PAY 111 

    0xf40e1d40,// 115 PAY 112 

    0xaa729350,// 116 PAY 113 

    0x12140b88,// 117 PAY 114 

    0x664051f4,// 118 PAY 115 

    0x24880281,// 119 PAY 116 

    0xd9023e99,// 120 PAY 117 

    0x0c30139d,// 121 PAY 118 

    0x7eb2d9c1,// 122 PAY 119 

    0x257b7c05,// 123 PAY 120 

    0x928f63ce,// 124 PAY 121 

    0x5c9d0533,// 125 PAY 122 

    0xb4830a20,// 126 PAY 123 

    0x6650b993,// 127 PAY 124 

    0xfc6bb893,// 128 PAY 125 

    0xf302b965,// 129 PAY 126 

    0x8dc0a0dd,// 130 PAY 127 

    0xdcba2c3b,// 131 PAY 128 

    0x40e697f0,// 132 PAY 129 

    0xb1b2c028,// 133 PAY 130 

    0x098939cb,// 134 PAY 131 

    0x4d5685f3,// 135 PAY 132 

    0xe4f3cfb1,// 136 PAY 133 

    0x74071fd2,// 137 PAY 134 

    0xcd664d61,// 138 PAY 135 

    0xc919ea4a,// 139 PAY 136 

    0xfc5d9446,// 140 PAY 137 

    0x868b99b4,// 141 PAY 138 

    0xcc07bcd8,// 142 PAY 139 

    0xd76d2abd,// 143 PAY 140 

    0x59a04541,// 144 PAY 141 

    0x2c13805e,// 145 PAY 142 

    0x11c0a8d7,// 146 PAY 143 

    0x71f3cd3a,// 147 PAY 144 

    0xb56fd72c,// 148 PAY 145 

    0xb2926990,// 149 PAY 146 

    0xcd4c5acb,// 150 PAY 147 

    0xcbf620fb,// 151 PAY 148 

    0x3b4cbd7f,// 152 PAY 149 

    0x48d728c5,// 153 PAY 150 

    0xc76a5cd9,// 154 PAY 151 

    0xad4f1522,// 155 PAY 152 

    0x6fb67422,// 156 PAY 153 

    0xd0b719b8,// 157 PAY 154 

    0x9a15fba9,// 158 PAY 155 

    0x3d6ca274,// 159 PAY 156 

    0x218fc112,// 160 PAY 157 

    0x30a7d83a,// 161 PAY 158 

    0x7e1db17a,// 162 PAY 159 

    0xa453d922,// 163 PAY 160 

    0x36dace76,// 164 PAY 161 

    0xdf1345b2,// 165 PAY 162 

    0x0dbf5b3d,// 166 PAY 163 

    0xee02bab8,// 167 PAY 164 

    0x9e65a2f7,// 168 PAY 165 

    0x09307173,// 169 PAY 166 

    0xfb15e4e0,// 170 PAY 167 

    0x68062d5f,// 171 PAY 168 

    0x19aa972d,// 172 PAY 169 

    0x0dcf41dc,// 173 PAY 170 

    0xadfc9fc3,// 174 PAY 171 

    0x3d6857cf,// 175 PAY 172 

    0xb6680e9f,// 176 PAY 173 

    0x0d29a7fe,// 177 PAY 174 

    0xb6b0affd,// 178 PAY 175 

    0xbaf1e349,// 179 PAY 176 

    0x4e5c5313,// 180 PAY 177 

    0xb99f0b67,// 181 PAY 178 

    0x4eb6b62b,// 182 PAY 179 

    0x22ebbb32,// 183 PAY 180 

    0xf975a1b6,// 184 PAY 181 

    0x11079e3c,// 185 PAY 182 

    0x9ccee6cf,// 186 PAY 183 

    0xb1a9896e,// 187 PAY 184 

    0x89eecaa6,// 188 PAY 185 

    0x6de85ae1,// 189 PAY 186 

    0xff852da7,// 190 PAY 187 

    0x0649b26d,// 191 PAY 188 

    0x8462f12c,// 192 PAY 189 

    0x0b3bd1de,// 193 PAY 190 

    0xac023ef3,// 194 PAY 191 

    0x99a2591d,// 195 PAY 192 

    0xb88aea28,// 196 PAY 193 

    0x9adeabb0,// 197 PAY 194 

    0x1c138f8d,// 198 PAY 195 

    0xb3d718af,// 199 PAY 196 

    0x4d5331e8,// 200 PAY 197 

    0x28c5fa65,// 201 PAY 198 

    0xa372c325,// 202 PAY 199 

    0x5a6263ad,// 203 PAY 200 

    0x0f99163e,// 204 PAY 201 

    0x6dc10def,// 205 PAY 202 

    0x76398794,// 206 PAY 203 

    0x9b270b7d,// 207 PAY 204 

    0x3f42d68e,// 208 PAY 205 

    0xd3744ec7,// 209 PAY 206 

    0x8953710a,// 210 PAY 207 

    0x0e7bfdaf,// 211 PAY 208 

    0x65f201af,// 212 PAY 209 

    0xb583dec7,// 213 PAY 210 

    0xbe8aa133,// 214 PAY 211 

    0x785c9cff,// 215 PAY 212 

    0x372b6871,// 216 PAY 213 

    0x3539a61e,// 217 PAY 214 

    0x4ce329ee,// 218 PAY 215 

    0x9a5db398,// 219 PAY 216 

    0x0e48abe2,// 220 PAY 217 

    0x245687be,// 221 PAY 218 

    0xce1ff15d,// 222 PAY 219 

    0xd089546c,// 223 PAY 220 

    0xb9730f9e,// 224 PAY 221 

    0x1b343d36,// 225 PAY 222 

    0xc800c15b,// 226 PAY 223 

    0xd3ea9de0,// 227 PAY 224 

    0xfbd7a42f,// 228 PAY 225 

    0x8b75fb41,// 229 PAY 226 

    0xfb7c8d90,// 230 PAY 227 

    0xb78c131d,// 231 PAY 228 

    0xe8b58efb,// 232 PAY 229 

    0x31561b4c,// 233 PAY 230 

    0xf41faff9,// 234 PAY 231 

    0x971af749,// 235 PAY 232 

    0x34bdf6f8,// 236 PAY 233 

    0xb23eda51,// 237 PAY 234 

    0x2d556bc8,// 238 PAY 235 

    0x32c7e3ff,// 239 PAY 236 

    0x4dddb3a4,// 240 PAY 237 

    0x993b8680,// 241 PAY 238 

    0xb63c4709,// 242 PAY 239 

    0x31ce7931,// 243 PAY 240 

    0xb88da4cd,// 244 PAY 241 

    0x98f4ef9b,// 245 PAY 242 

    0x5ddecf63,// 246 PAY 243 

    0xe6142c53,// 247 PAY 244 

    0x86c63222,// 248 PAY 245 

    0x827132b5,// 249 PAY 246 

    0x7d4c53cb,// 250 PAY 247 

    0x6ba27c80,// 251 PAY 248 

    0xc3b74cc8,// 252 PAY 249 

    0xc93082cb,// 253 PAY 250 

    0xad9461a1,// 254 PAY 251 

    0x0d43e94d,// 255 PAY 252 

    0xd699332a,// 256 PAY 253 

    0x80193bd3,// 257 PAY 254 

    0x2e2fe3d8,// 258 PAY 255 

    0xc8d233e9,// 259 PAY 256 

    0xaeeb88ec,// 260 PAY 257 

    0x039a0f4e,// 261 PAY 258 

    0x1ec15cab,// 262 PAY 259 

    0x9dd9e432,// 263 PAY 260 

    0xc94f8674,// 264 PAY 261 

    0x03808618,// 265 PAY 262 

    0x490a20ba,// 266 PAY 263 

    0xffcee243,// 267 PAY 264 

    0xa9d815e0,// 268 PAY 265 

    0xfe57d871,// 269 PAY 266 

    0xe1826c82,// 270 PAY 267 

    0xcbdf6c41,// 271 PAY 268 

    0x95faf059,// 272 PAY 269 

    0x22f06ad5,// 273 PAY 270 

    0x796dba96,// 274 PAY 271 

    0xcfffbeb7,// 275 PAY 272 

    0x6c206e63,// 276 PAY 273 

    0x9bb7fe60,// 277 PAY 274 

    0x7789a6c4,// 278 PAY 275 

    0xfaea8cb3,// 279 PAY 276 

    0xe42392cc,// 280 PAY 277 

    0x5b87e9b2,// 281 PAY 278 

    0x53edde08,// 282 PAY 279 

    0x263f41c3,// 283 PAY 280 

    0xdeafc8d9,// 284 PAY 281 

    0x26e526c0,// 285 PAY 282 

    0xf063924b,// 286 PAY 283 

    0xd413053f,// 287 PAY 284 

    0xa3ad8615,// 288 PAY 285 

    0xeb628b5f,// 289 PAY 286 

    0xbcacf7fe,// 290 PAY 287 

    0x7a90b9b2,// 291 PAY 288 

    0x93b2bd9a,// 292 PAY 289 

    0x312963db,// 293 PAY 290 

    0x1e46573d,// 294 PAY 291 

    0xdc0651d0,// 295 PAY 292 

    0x01267bcf,// 296 PAY 293 

    0x4a416639,// 297 PAY 294 

    0xcbdd60a9,// 298 PAY 295 

    0x69cdd288,// 299 PAY 296 

    0xcd6f637e,// 300 PAY 297 

    0x66e9d329,// 301 PAY 298 

    0x1f85979f,// 302 PAY 299 

    0x2d29d60b,// 303 PAY 300 

    0xf4d5387a,// 304 PAY 301 

    0x9925b0d3,// 305 PAY 302 

    0xb1b48d5d,// 306 PAY 303 

    0x9b27fa1c,// 307 PAY 304 

    0xb0a5aa2f,// 308 PAY 305 

    0xcec35821,// 309 PAY 306 

    0x1c9c9fe9,// 310 PAY 307 

    0xc2542d7b,// 311 PAY 308 

    0xbff41a63,// 312 PAY 309 

    0xdbb464e1,// 313 PAY 310 

    0xc2e2f861,// 314 PAY 311 

    0x59c0e5d9,// 315 PAY 312 

    0xb9c1af51,// 316 PAY 313 

    0x992ea003,// 317 PAY 314 

    0xcfc99dbf,// 318 PAY 315 

    0x2aef1860,// 319 PAY 316 

    0x29f8f580,// 320 PAY 317 

    0x1d3a4f0f,// 321 PAY 318 

    0xd15adc6e,// 322 PAY 319 

    0x26a0e74e,// 323 PAY 320 

    0x424a2cfa,// 324 PAY 321 

    0xd088901c,// 325 PAY 322 

    0x71269859,// 326 PAY 323 

    0xff737a8d,// 327 PAY 324 

    0x397d04f8,// 328 PAY 325 

    0xa52ae0aa,// 329 PAY 326 

    0xb883ace4,// 330 PAY 327 

    0x1f25e67e,// 331 PAY 328 

    0x11b29527,// 332 PAY 329 

    0xf0a27648,// 333 PAY 330 

    0x1bdae625,// 334 PAY 331 

    0x4cc5e173,// 335 PAY 332 

    0x1fc0279f,// 336 PAY 333 

    0xbb09934b,// 337 PAY 334 

    0xbe50f83c,// 338 PAY 335 

    0x2163faa5,// 339 PAY 336 

    0x5cb87f18,// 340 PAY 337 

    0xeab902dd,// 341 PAY 338 

    0xe9f51191,// 342 PAY 339 

    0x10d2e7ae,// 343 PAY 340 

    0x1502fd18,// 344 PAY 341 

    0x9d969f8e,// 345 PAY 342 

    0x412c06b4,// 346 PAY 343 

    0xc04d9788,// 347 PAY 344 

    0x14af3a4a,// 348 PAY 345 

    0xd400542f,// 349 PAY 346 

    0x91f919d1,// 350 PAY 347 

    0x0358fd8c,// 351 PAY 348 

    0x70f07c4d,// 352 PAY 349 

    0xebc6bf7a,// 353 PAY 350 

    0x82287cfa,// 354 PAY 351 

    0x1091f923,// 355 PAY 352 

    0xd0d34b95,// 356 PAY 353 

    0xd472c184,// 357 PAY 354 

    0xd4be5368,// 358 PAY 355 

    0xd6ca9570,// 359 PAY 356 

    0xf545f4fa,// 360 PAY 357 

    0xd9b6ef45,// 361 PAY 358 

    0x2d0301d2,// 362 PAY 359 

    0x7786cbb3,// 363 PAY 360 

    0x9e3bedcf,// 364 PAY 361 

    0x967632eb,// 365 PAY 362 

    0x0d7d444b,// 366 PAY 363 

    0xd52cf426,// 367 PAY 364 

    0xeb546712,// 368 PAY 365 

    0x2cd02de5,// 369 PAY 366 

    0x35ec7c70,// 370 PAY 367 

    0x142e66af,// 371 PAY 368 

    0x99b6aca6,// 372 PAY 369 

    0xc3fb6fbb,// 373 PAY 370 

    0xd657770e,// 374 PAY 371 

    0x6ba2668b,// 375 PAY 372 

    0x4e0b3e13,// 376 PAY 373 

    0xcc2c5090,// 377 PAY 374 

    0x34c7f10d,// 378 PAY 375 

    0xc7d909b7,// 379 PAY 376 

    0xdb8918c5,// 380 PAY 377 

    0xcf2b3789,// 381 PAY 378 

    0x364d1bb8,// 382 PAY 379 

    0xee129317,// 383 PAY 380 

    0x1f374f71,// 384 PAY 381 

    0x6533616f,// 385 PAY 382 

    0xee3b29ac,// 386 PAY 383 

    0x1a97a2a7,// 387 PAY 384 

    0xe4194674,// 388 PAY 385 

    0x7259005e,// 389 PAY 386 

    0x47541f9a,// 390 PAY 387 

    0xe36ee295,// 391 PAY 388 

    0x576c18e2,// 392 PAY 389 

    0xdf192761,// 393 PAY 390 

    0x5a073778,// 394 PAY 391 

    0x9980341f,// 395 PAY 392 

    0x4e380821,// 396 PAY 393 

    0x0da937c5,// 397 PAY 394 

    0x59d79ac8,// 398 PAY 395 

    0x69bf8c5a,// 399 PAY 396 

    0x7d134383,// 400 PAY 397 

    0xcb62d389,// 401 PAY 398 

    0x2e963eab,// 402 PAY 399 

    0x9889b195,// 403 PAY 400 

    0xef925fb0,// 404 PAY 401 

    0xd4000000,// 405 PAY 402 

/// STA is 1 words. 

/// STA num_pkts       : 144 

/// STA pkt_idx        : 108 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x01b11c90 // 406 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt49_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 133 words. 

/// BDA size     is 525 (0x20d) 

/// BDA id       is 0x5630 

    0x020d5630,// 3 BDA   1 

/// PAY Generic Data size   : 525 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x541feb8f,// 4 PAY   1 

    0x5124c42a,// 5 PAY   2 

    0x2d3453fd,// 6 PAY   3 

    0xccf6eb4c,// 7 PAY   4 

    0x21bedbf6,// 8 PAY   5 

    0xa2bdebc1,// 9 PAY   6 

    0xe7f551bf,// 10 PAY   7 

    0xda5af626,// 11 PAY   8 

    0x603a69fd,// 12 PAY   9 

    0xb253c889,// 13 PAY  10 

    0xec7ec1fa,// 14 PAY  11 

    0x2b608290,// 15 PAY  12 

    0xc69d2d34,// 16 PAY  13 

    0xf2acb57b,// 17 PAY  14 

    0x9ea6fa7f,// 18 PAY  15 

    0x4688e65c,// 19 PAY  16 

    0xc3450fce,// 20 PAY  17 

    0xf43554cf,// 21 PAY  18 

    0x58fee916,// 22 PAY  19 

    0x4c5d7008,// 23 PAY  20 

    0xbf824a22,// 24 PAY  21 

    0x1cb9c2ed,// 25 PAY  22 

    0x0dd58a81,// 26 PAY  23 

    0x692ff012,// 27 PAY  24 

    0x513baaeb,// 28 PAY  25 

    0xf778b767,// 29 PAY  26 

    0x66a487fb,// 30 PAY  27 

    0xfb3c5564,// 31 PAY  28 

    0xb8fac05f,// 32 PAY  29 

    0x84a2260d,// 33 PAY  30 

    0x229dad88,// 34 PAY  31 

    0x05078d99,// 35 PAY  32 

    0x0a56ab85,// 36 PAY  33 

    0x55baf04c,// 37 PAY  34 

    0xec255d5e,// 38 PAY  35 

    0x55d11901,// 39 PAY  36 

    0x4806ff07,// 40 PAY  37 

    0x264cbec0,// 41 PAY  38 

    0xbc74251a,// 42 PAY  39 

    0x55b4801b,// 43 PAY  40 

    0x76cf99ac,// 44 PAY  41 

    0x9c0ec0bf,// 45 PAY  42 

    0x617c74b1,// 46 PAY  43 

    0x94f0712b,// 47 PAY  44 

    0xe47460dc,// 48 PAY  45 

    0x58f2a2f2,// 49 PAY  46 

    0x820b7ebc,// 50 PAY  47 

    0x74d906e2,// 51 PAY  48 

    0xde1210a8,// 52 PAY  49 

    0xc0251594,// 53 PAY  50 

    0xa347b571,// 54 PAY  51 

    0x2d84c9a4,// 55 PAY  52 

    0xd9a392cd,// 56 PAY  53 

    0x111155c9,// 57 PAY  54 

    0xc063832f,// 58 PAY  55 

    0x943e77aa,// 59 PAY  56 

    0x3f052594,// 60 PAY  57 

    0x5e057a5b,// 61 PAY  58 

    0xac42bc5f,// 62 PAY  59 

    0x324d7084,// 63 PAY  60 

    0x449ca7d3,// 64 PAY  61 

    0x76fc9c5e,// 65 PAY  62 

    0x4c54e116,// 66 PAY  63 

    0x09ea6b90,// 67 PAY  64 

    0xd38cc668,// 68 PAY  65 

    0x5087a325,// 69 PAY  66 

    0x7a640920,// 70 PAY  67 

    0xd264d910,// 71 PAY  68 

    0x064329d6,// 72 PAY  69 

    0x0830c88c,// 73 PAY  70 

    0xae566630,// 74 PAY  71 

    0xdf2e5d8d,// 75 PAY  72 

    0x49f87ad6,// 76 PAY  73 

    0xeb068f2d,// 77 PAY  74 

    0x23039853,// 78 PAY  75 

    0xc1809163,// 79 PAY  76 

    0x6202cf6e,// 80 PAY  77 

    0xaa6d3fbc,// 81 PAY  78 

    0x5e6c39b4,// 82 PAY  79 

    0x40154798,// 83 PAY  80 

    0x8620b543,// 84 PAY  81 

    0x5666b9b4,// 85 PAY  82 

    0x11e15e40,// 86 PAY  83 

    0xa4a830c4,// 87 PAY  84 

    0x5e9a47ac,// 88 PAY  85 

    0x555e2c89,// 89 PAY  86 

    0xcd3343f1,// 90 PAY  87 

    0x486e3e84,// 91 PAY  88 

    0xc98980a2,// 92 PAY  89 

    0x4316e5fd,// 93 PAY  90 

    0xc6cb944e,// 94 PAY  91 

    0x9e993551,// 95 PAY  92 

    0x0ce39f1d,// 96 PAY  93 

    0xef686a2b,// 97 PAY  94 

    0xa5ee6b88,// 98 PAY  95 

    0xf7fb816c,// 99 PAY  96 

    0x36a2a7e9,// 100 PAY  97 

    0x16565df1,// 101 PAY  98 

    0x674e402c,// 102 PAY  99 

    0x83c02859,// 103 PAY 100 

    0xedba4396,// 104 PAY 101 

    0xfcae0978,// 105 PAY 102 

    0x7d76516d,// 106 PAY 103 

    0x8d95422d,// 107 PAY 104 

    0x17f71de5,// 108 PAY 105 

    0x75224a54,// 109 PAY 106 

    0xd39bfb6a,// 110 PAY 107 

    0xee31a0b8,// 111 PAY 108 

    0x39746012,// 112 PAY 109 

    0x5f3ec2c6,// 113 PAY 110 

    0x8a7624be,// 114 PAY 111 

    0x10950b22,// 115 PAY 112 

    0xf3683204,// 116 PAY 113 

    0x9acc281c,// 117 PAY 114 

    0x4cffa542,// 118 PAY 115 

    0x10d5185a,// 119 PAY 116 

    0x63a15a44,// 120 PAY 117 

    0x6acfa303,// 121 PAY 118 

    0x447aaf36,// 122 PAY 119 

    0x77e00163,// 123 PAY 120 

    0xc21fae96,// 124 PAY 121 

    0x48064ca1,// 125 PAY 122 

    0x3552b5f7,// 126 PAY 123 

    0x0cf50f6d,// 127 PAY 124 

    0x49bdbe1b,// 128 PAY 125 

    0xeca2dd89,// 129 PAY 126 

    0x792083c6,// 130 PAY 127 

    0x10f9d7bd,// 131 PAY 128 

    0x2f085e21,// 132 PAY 129 

    0x97c8dfa9,// 133 PAY 130 

    0xc04dc484,// 134 PAY 131 

    0x59000000,// 135 PAY 132 

/// HASH is  16 bytes 

    0x16565df1,// 136 HSH   1 

    0x674e402c,// 137 HSH   2 

    0x83c02859,// 138 HSH   3 

    0xedba4396,// 139 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 101 

/// STA pkt_idx        : 196 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0311ce65 // 140 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt50_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 287 words. 

/// BDA size     is 1143 (0x477) 

/// BDA id       is 0x3ee5 

    0x04773ee5,// 3 BDA   1 

/// PAY Generic Data size   : 1143 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xa57f7cfd,// 4 PAY   1 

    0x587f72e6,// 5 PAY   2 

    0x3adadad5,// 6 PAY   3 

    0x318786dc,// 7 PAY   4 

    0x2f843933,// 8 PAY   5 

    0xff68ed61,// 9 PAY   6 

    0x564732b1,// 10 PAY   7 

    0xccf25fd6,// 11 PAY   8 

    0x5fbd8021,// 12 PAY   9 

    0x24af0c53,// 13 PAY  10 

    0x3cdd28e8,// 14 PAY  11 

    0x9ad1442a,// 15 PAY  12 

    0xdf5125bb,// 16 PAY  13 

    0x048563a0,// 17 PAY  14 

    0x6e2065c6,// 18 PAY  15 

    0x3ff96cf5,// 19 PAY  16 

    0x2b3434cf,// 20 PAY  17 

    0x64d46f28,// 21 PAY  18 

    0x985c0501,// 22 PAY  19 

    0xe96be8ed,// 23 PAY  20 

    0xfded9546,// 24 PAY  21 

    0xe2f06790,// 25 PAY  22 

    0x097bf0eb,// 26 PAY  23 

    0x10c6bd6b,// 27 PAY  24 

    0xd9192ebd,// 28 PAY  25 

    0x1ab29a7f,// 29 PAY  26 

    0x3aefc132,// 30 PAY  27 

    0x22f3dc69,// 31 PAY  28 

    0x69d042a9,// 32 PAY  29 

    0x63dd46e1,// 33 PAY  30 

    0xb1300211,// 34 PAY  31 

    0x20f6159b,// 35 PAY  32 

    0xd2c73959,// 36 PAY  33 

    0x0e15169c,// 37 PAY  34 

    0x3c013816,// 38 PAY  35 

    0xec22ada4,// 39 PAY  36 

    0x3646b0a2,// 40 PAY  37 

    0xa1ac5f19,// 41 PAY  38 

    0xee25d734,// 42 PAY  39 

    0x526c8228,// 43 PAY  40 

    0x1f88bb54,// 44 PAY  41 

    0x2a79626e,// 45 PAY  42 

    0xfaa93e0f,// 46 PAY  43 

    0x9209642e,// 47 PAY  44 

    0x30ab237e,// 48 PAY  45 

    0x2a46bd66,// 49 PAY  46 

    0xdd46641c,// 50 PAY  47 

    0x5ffb6457,// 51 PAY  48 

    0xcb6dac55,// 52 PAY  49 

    0x9ec4898b,// 53 PAY  50 

    0xc5da157d,// 54 PAY  51 

    0xcc2166fc,// 55 PAY  52 

    0x0f199e68,// 56 PAY  53 

    0x534e1b18,// 57 PAY  54 

    0x980aadf1,// 58 PAY  55 

    0x19c1aae2,// 59 PAY  56 

    0xcaa5e366,// 60 PAY  57 

    0x2e0efffb,// 61 PAY  58 

    0x242edac8,// 62 PAY  59 

    0x647ee171,// 63 PAY  60 

    0xafa75e36,// 64 PAY  61 

    0xe304ab7b,// 65 PAY  62 

    0xfe25b954,// 66 PAY  63 

    0x5de51706,// 67 PAY  64 

    0x5a9e2d09,// 68 PAY  65 

    0xc42e3b80,// 69 PAY  66 

    0x17c53859,// 70 PAY  67 

    0xd2ad16d7,// 71 PAY  68 

    0x837b6ff4,// 72 PAY  69 

    0x6e2d8012,// 73 PAY  70 

    0xb1c35909,// 74 PAY  71 

    0x6c9d4c24,// 75 PAY  72 

    0x6aec00d4,// 76 PAY  73 

    0xdcd2b55b,// 77 PAY  74 

    0x4d525627,// 78 PAY  75 

    0x93b3026e,// 79 PAY  76 

    0xaaee6deb,// 80 PAY  77 

    0x43a71c32,// 81 PAY  78 

    0xd04d3b94,// 82 PAY  79 

    0x3567b004,// 83 PAY  80 

    0xccaf81ab,// 84 PAY  81 

    0xe61085a7,// 85 PAY  82 

    0x8a49097f,// 86 PAY  83 

    0x6de5574a,// 87 PAY  84 

    0x653efe2c,// 88 PAY  85 

    0xb8b31869,// 89 PAY  86 

    0xfc7748ba,// 90 PAY  87 

    0xb9fdd1f1,// 91 PAY  88 

    0x92e2655b,// 92 PAY  89 

    0x35165066,// 93 PAY  90 

    0x8e0c84a2,// 94 PAY  91 

    0x3bff525c,// 95 PAY  92 

    0x4fe8453b,// 96 PAY  93 

    0x78c3167e,// 97 PAY  94 

    0x4fe279a1,// 98 PAY  95 

    0x13865be1,// 99 PAY  96 

    0x108fae06,// 100 PAY  97 

    0x70f668c5,// 101 PAY  98 

    0x51ead9c5,// 102 PAY  99 

    0x57389b80,// 103 PAY 100 

    0xc2e3862e,// 104 PAY 101 

    0x41d55169,// 105 PAY 102 

    0xcdb06b10,// 106 PAY 103 

    0x7fc41f92,// 107 PAY 104 

    0x5220d11c,// 108 PAY 105 

    0xd97ce8ca,// 109 PAY 106 

    0x495f303d,// 110 PAY 107 

    0xf8822b61,// 111 PAY 108 

    0x0de57df1,// 112 PAY 109 

    0x7b7ab041,// 113 PAY 110 

    0xa573779e,// 114 PAY 111 

    0xb9dc42a2,// 115 PAY 112 

    0x2fbb9603,// 116 PAY 113 

    0x9a7991a5,// 117 PAY 114 

    0xfaa8cb32,// 118 PAY 115 

    0xa2190126,// 119 PAY 116 

    0x3dc4a6d6,// 120 PAY 117 

    0x5128f8cb,// 121 PAY 118 

    0xb6e5ad28,// 122 PAY 119 

    0xad418f52,// 123 PAY 120 

    0xeace4e52,// 124 PAY 121 

    0x37e6fdc1,// 125 PAY 122 

    0xc40a4616,// 126 PAY 123 

    0x55a5e48f,// 127 PAY 124 

    0xecfa8187,// 128 PAY 125 

    0xbbc502f9,// 129 PAY 126 

    0x3a0882a6,// 130 PAY 127 

    0x1ee73552,// 131 PAY 128 

    0x3b3e2dac,// 132 PAY 129 

    0xe7b84ad0,// 133 PAY 130 

    0xc18e1a66,// 134 PAY 131 

    0xbe7188d7,// 135 PAY 132 

    0x2be6e34c,// 136 PAY 133 

    0xe137b628,// 137 PAY 134 

    0xf5498044,// 138 PAY 135 

    0x0f01c7b9,// 139 PAY 136 

    0x2ae664f3,// 140 PAY 137 

    0xa7aecb6e,// 141 PAY 138 

    0xb0ae2a9d,// 142 PAY 139 

    0xffc26cc9,// 143 PAY 140 

    0x3002a9a0,// 144 PAY 141 

    0xb0158d5f,// 145 PAY 142 

    0xb9be1411,// 146 PAY 143 

    0xe8a7b9d3,// 147 PAY 144 

    0xa7ac7f5a,// 148 PAY 145 

    0x8b4a610f,// 149 PAY 146 

    0x490fde88,// 150 PAY 147 

    0x697be014,// 151 PAY 148 

    0x34d9426f,// 152 PAY 149 

    0x708d730e,// 153 PAY 150 

    0xd8dd7d9c,// 154 PAY 151 

    0x71e98634,// 155 PAY 152 

    0xe8295336,// 156 PAY 153 

    0x76a90e75,// 157 PAY 154 

    0xbd81a7f9,// 158 PAY 155 

    0x8f0f295f,// 159 PAY 156 

    0x09094749,// 160 PAY 157 

    0x4c49900d,// 161 PAY 158 

    0x4cab4de5,// 162 PAY 159 

    0xd80a98dd,// 163 PAY 160 

    0xde5fc984,// 164 PAY 161 

    0x896d15f6,// 165 PAY 162 

    0x5ba040ff,// 166 PAY 163 

    0xcf5723b1,// 167 PAY 164 

    0x053b1722,// 168 PAY 165 

    0xad3f1c32,// 169 PAY 166 

    0xac627e63,// 170 PAY 167 

    0xdacc808f,// 171 PAY 168 

    0xbb060679,// 172 PAY 169 

    0xc1511e28,// 173 PAY 170 

    0xdaca2b2c,// 174 PAY 171 

    0xf46cd823,// 175 PAY 172 

    0x7d7b1a6e,// 176 PAY 173 

    0x89b1eacd,// 177 PAY 174 

    0x7b9407ed,// 178 PAY 175 

    0x36a4a001,// 179 PAY 176 

    0xc041b05b,// 180 PAY 177 

    0xf173081d,// 181 PAY 178 

    0x6cd1aec8,// 182 PAY 179 

    0xdafb25a5,// 183 PAY 180 

    0x8eac9e9c,// 184 PAY 181 

    0x3f6d6875,// 185 PAY 182 

    0xf8c4761d,// 186 PAY 183 

    0xc5573e0a,// 187 PAY 184 

    0x3e24f40a,// 188 PAY 185 

    0x4a3b1da4,// 189 PAY 186 

    0x5178b30c,// 190 PAY 187 

    0x7dc974cf,// 191 PAY 188 

    0xb848850a,// 192 PAY 189 

    0x226b613a,// 193 PAY 190 

    0x6cb1496c,// 194 PAY 191 

    0xb550ce8b,// 195 PAY 192 

    0x9bb204f2,// 196 PAY 193 

    0xb2fe1b95,// 197 PAY 194 

    0xbce921d0,// 198 PAY 195 

    0x0526e159,// 199 PAY 196 

    0xe06494d3,// 200 PAY 197 

    0x38d1487f,// 201 PAY 198 

    0x80d75f6a,// 202 PAY 199 

    0xb3c1bce7,// 203 PAY 200 

    0x1629beb9,// 204 PAY 201 

    0x10182989,// 205 PAY 202 

    0x3e11d649,// 206 PAY 203 

    0x9a03bd93,// 207 PAY 204 

    0x956868d8,// 208 PAY 205 

    0x78e23e06,// 209 PAY 206 

    0x97b14ad4,// 210 PAY 207 

    0x8c9561ed,// 211 PAY 208 

    0x216688c3,// 212 PAY 209 

    0xa7322057,// 213 PAY 210 

    0xbe908ab2,// 214 PAY 211 

    0xc2c984d9,// 215 PAY 212 

    0xc269e076,// 216 PAY 213 

    0x9b9e61fc,// 217 PAY 214 

    0xe53841d8,// 218 PAY 215 

    0x01202404,// 219 PAY 216 

    0x6ac81979,// 220 PAY 217 

    0xe7d29f14,// 221 PAY 218 

    0x795e67d0,// 222 PAY 219 

    0x5eb7a000,// 223 PAY 220 

    0xcec54cec,// 224 PAY 221 

    0x2db59833,// 225 PAY 222 

    0x53adb8f2,// 226 PAY 223 

    0x121105bd,// 227 PAY 224 

    0x4f01b8c1,// 228 PAY 225 

    0x1307bf38,// 229 PAY 226 

    0x12dd6858,// 230 PAY 227 

    0xe07b9ae2,// 231 PAY 228 

    0x98d669dd,// 232 PAY 229 

    0x5d504ba2,// 233 PAY 230 

    0x8e2a6015,// 234 PAY 231 

    0x0607969e,// 235 PAY 232 

    0x3769237d,// 236 PAY 233 

    0xccae1960,// 237 PAY 234 

    0x1f5dce27,// 238 PAY 235 

    0x0785e138,// 239 PAY 236 

    0x6fb8f83d,// 240 PAY 237 

    0xb524c4ee,// 241 PAY 238 

    0xddc24e04,// 242 PAY 239 

    0x3b890b80,// 243 PAY 240 

    0xc2c24c5a,// 244 PAY 241 

    0x7fd338ff,// 245 PAY 242 

    0xb19b708b,// 246 PAY 243 

    0xe6657d63,// 247 PAY 244 

    0x1158a008,// 248 PAY 245 

    0xbacfc931,// 249 PAY 246 

    0xad6f2283,// 250 PAY 247 

    0xe803f986,// 251 PAY 248 

    0x15383164,// 252 PAY 249 

    0xf62568bd,// 253 PAY 250 

    0x3f34516f,// 254 PAY 251 

    0x6ef91f05,// 255 PAY 252 

    0xa1b75ebb,// 256 PAY 253 

    0xd93db0ec,// 257 PAY 254 

    0xcdd0cf95,// 258 PAY 255 

    0x81cb96bb,// 259 PAY 256 

    0x3f6ac714,// 260 PAY 257 

    0x69bd1f19,// 261 PAY 258 

    0x50ac9a0b,// 262 PAY 259 

    0x73ab64a2,// 263 PAY 260 

    0x4f3b73ac,// 264 PAY 261 

    0xc1e3317a,// 265 PAY 262 

    0x689ace55,// 266 PAY 263 

    0xd7b31fdb,// 267 PAY 264 

    0x304ea833,// 268 PAY 265 

    0xe68f2ca7,// 269 PAY 266 

    0x5080f08f,// 270 PAY 267 

    0xf1856405,// 271 PAY 268 

    0x9044e685,// 272 PAY 269 

    0xa3eb78e8,// 273 PAY 270 

    0xc91812a0,// 274 PAY 271 

    0x638ad762,// 275 PAY 272 

    0x467a3218,// 276 PAY 273 

    0xe55351c8,// 277 PAY 274 

    0x8913d81c,// 278 PAY 275 

    0xa7f59af2,// 279 PAY 276 

    0x51679b60,// 280 PAY 277 

    0x30bae111,// 281 PAY 278 

    0xad6e2ada,// 282 PAY 279 

    0xdee9eaa4,// 283 PAY 280 

    0xec220c53,// 284 PAY 281 

    0x2d62d201,// 285 PAY 282 

    0x4e5dfc0b,// 286 PAY 283 

    0x57a50617,// 287 PAY 284 

    0x59d0df95,// 288 PAY 285 

    0x3b18e800,// 289 PAY 286 

/// STA is 1 words. 

/// STA num_pkts       : 196 

/// STA pkt_idx        : 51 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x52 

    0x00cc52c4 // 290 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt51_tmpl[] = {
    0x0c010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 83 words. 

/// BDA size     is 328 (0x148) 

/// BDA id       is 0x30ad 

    0x014830ad,// 3 BDA   1 

/// PAY Generic Data size   : 328 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x72424e02,// 4 PAY   1 

    0x505869ac,// 5 PAY   2 

    0xe428ede6,// 6 PAY   3 

    0x749af780,// 7 PAY   4 

    0x3ad02330,// 8 PAY   5 

    0x782348c1,// 9 PAY   6 

    0x87396b09,// 10 PAY   7 

    0x33673979,// 11 PAY   8 

    0x742fe244,// 12 PAY   9 

    0x52bd9e87,// 13 PAY  10 

    0x56255316,// 14 PAY  11 

    0x9fa33173,// 15 PAY  12 

    0xa6ee3b54,// 16 PAY  13 

    0x0622a4b2,// 17 PAY  14 

    0x251401ca,// 18 PAY  15 

    0x2e3f1d03,// 19 PAY  16 

    0x2f24c639,// 20 PAY  17 

    0x3708cc1d,// 21 PAY  18 

    0xe6d1f82d,// 22 PAY  19 

    0x18b8f69c,// 23 PAY  20 

    0xc327b403,// 24 PAY  21 

    0xbf304886,// 25 PAY  22 

    0x3c683791,// 26 PAY  23 

    0x07d52ddb,// 27 PAY  24 

    0xe12a0ccc,// 28 PAY  25 

    0x25a7a0be,// 29 PAY  26 

    0xb03e3236,// 30 PAY  27 

    0xd0020712,// 31 PAY  28 

    0x65dc1009,// 32 PAY  29 

    0xf2b5de87,// 33 PAY  30 

    0xc467506a,// 34 PAY  31 

    0x36910bf8,// 35 PAY  32 

    0x27d43052,// 36 PAY  33 

    0xa5d740c1,// 37 PAY  34 

    0xb5af7896,// 38 PAY  35 

    0xb2dd3808,// 39 PAY  36 

    0xcf8b572f,// 40 PAY  37 

    0x145be074,// 41 PAY  38 

    0x74495fff,// 42 PAY  39 

    0x89846acc,// 43 PAY  40 

    0xb74e8f91,// 44 PAY  41 

    0xbd36e972,// 45 PAY  42 

    0x3df8d168,// 46 PAY  43 

    0x21e29ede,// 47 PAY  44 

    0xfd66aa6c,// 48 PAY  45 

    0x38e33216,// 49 PAY  46 

    0x285b2ec9,// 50 PAY  47 

    0x1ac6476a,// 51 PAY  48 

    0x68f23c08,// 52 PAY  49 

    0x42200f8e,// 53 PAY  50 

    0xbd3bca15,// 54 PAY  51 

    0xbfa6c832,// 55 PAY  52 

    0x6a7c7d67,// 56 PAY  53 

    0xccc261a2,// 57 PAY  54 

    0x0a578c12,// 58 PAY  55 

    0x22dc20cb,// 59 PAY  56 

    0x5ad47c53,// 60 PAY  57 

    0x3373f823,// 61 PAY  58 

    0xda587d68,// 62 PAY  59 

    0xb4428b95,// 63 PAY  60 

    0x93204702,// 64 PAY  61 

    0xf6ca10f1,// 65 PAY  62 

    0xd532caca,// 66 PAY  63 

    0xba03accc,// 67 PAY  64 

    0xeb16f7ea,// 68 PAY  65 

    0xbfb0a82a,// 69 PAY  66 

    0x5ebbe96a,// 70 PAY  67 

    0x011d491c,// 71 PAY  68 

    0xfe031ccd,// 72 PAY  69 

    0xe76817ea,// 73 PAY  70 

    0x599a641f,// 74 PAY  71 

    0x73c872d7,// 75 PAY  72 

    0xad108d48,// 76 PAY  73 

    0x9b229f44,// 77 PAY  74 

    0x6262b922,// 78 PAY  75 

    0x16432c60,// 79 PAY  76 

    0xfdf2346c,// 80 PAY  77 

    0x72190c63,// 81 PAY  78 

    0x7fa6f6fb,// 82 PAY  79 

    0x8cd50632,// 83 PAY  80 

    0xeddabe69,// 84 PAY  81 

    0x2ea1a270,// 85 PAY  82 

/// HASH is  4 bytes 

    0x6262b922,// 86 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 59 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb6 

    0x0050b63b // 87 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt52_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 430 words. 

/// BDA size     is 1714 (0x6b2) 

/// BDA id       is 0xa607 

    0x06b2a607,// 3 BDA   1 

/// PAY Generic Data size   : 1714 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x33105f2e,// 4 PAY   1 

    0x7f19f843,// 5 PAY   2 

    0x23d8ce04,// 6 PAY   3 

    0x131ce033,// 7 PAY   4 

    0x11f9b668,// 8 PAY   5 

    0x5f52d663,// 9 PAY   6 

    0xd4250daa,// 10 PAY   7 

    0x0ada0740,// 11 PAY   8 

    0xb51f695d,// 12 PAY   9 

    0x2fc37156,// 13 PAY  10 

    0x9914a81f,// 14 PAY  11 

    0xc066b56d,// 15 PAY  12 

    0xda9be7b6,// 16 PAY  13 

    0x9c220504,// 17 PAY  14 

    0xab1ae513,// 18 PAY  15 

    0x22821948,// 19 PAY  16 

    0x4ab47baa,// 20 PAY  17 

    0xfe7d94aa,// 21 PAY  18 

    0x781d163a,// 22 PAY  19 

    0x5ad532c8,// 23 PAY  20 

    0x950f4f0d,// 24 PAY  21 

    0x752075a4,// 25 PAY  22 

    0x3a319640,// 26 PAY  23 

    0xa5a6301a,// 27 PAY  24 

    0x267611ae,// 28 PAY  25 

    0x09550b6e,// 29 PAY  26 

    0x7d5fdbec,// 30 PAY  27 

    0xcf492ce3,// 31 PAY  28 

    0x4440d03a,// 32 PAY  29 

    0x753ce970,// 33 PAY  30 

    0xa9a8b14e,// 34 PAY  31 

    0xdfd70dc9,// 35 PAY  32 

    0x49187fee,// 36 PAY  33 

    0x7170560a,// 37 PAY  34 

    0x62e1eb9a,// 38 PAY  35 

    0x3866419d,// 39 PAY  36 

    0x37f99a79,// 40 PAY  37 

    0x94790d9a,// 41 PAY  38 

    0x8c3b3132,// 42 PAY  39 

    0xf29dd4d4,// 43 PAY  40 

    0x284f3e9f,// 44 PAY  41 

    0x8ff5dba6,// 45 PAY  42 

    0x6d707fc6,// 46 PAY  43 

    0x615bbf2b,// 47 PAY  44 

    0x8e0660c5,// 48 PAY  45 

    0xd2a5bcc2,// 49 PAY  46 

    0xb5b16ebc,// 50 PAY  47 

    0xfa97ab51,// 51 PAY  48 

    0xc478ac54,// 52 PAY  49 

    0x2351b7dc,// 53 PAY  50 

    0xa2e41d08,// 54 PAY  51 

    0xab1e831e,// 55 PAY  52 

    0xda19fb87,// 56 PAY  53 

    0xd30fecb0,// 57 PAY  54 

    0x0cd07fc0,// 58 PAY  55 

    0xf5ec1762,// 59 PAY  56 

    0x7623125f,// 60 PAY  57 

    0x0b8f91b4,// 61 PAY  58 

    0x2fe58ae3,// 62 PAY  59 

    0xf48471cc,// 63 PAY  60 

    0xd923d905,// 64 PAY  61 

    0x000891fe,// 65 PAY  62 

    0x32b17152,// 66 PAY  63 

    0xdcb3808d,// 67 PAY  64 

    0x6c6ba1bb,// 68 PAY  65 

    0x19e67cd7,// 69 PAY  66 

    0xda9cc6e5,// 70 PAY  67 

    0x83ab5baf,// 71 PAY  68 

    0xcdeb047b,// 72 PAY  69 

    0x13f0d4c7,// 73 PAY  70 

    0x0a2cd3f1,// 74 PAY  71 

    0xfddfc92e,// 75 PAY  72 

    0xc6552fa2,// 76 PAY  73 

    0xbb29a826,// 77 PAY  74 

    0xa1218fce,// 78 PAY  75 

    0x2043d5aa,// 79 PAY  76 

    0x50035c8a,// 80 PAY  77 

    0x6f4d6fd3,// 81 PAY  78 

    0x323e2cdb,// 82 PAY  79 

    0x841cc7cb,// 83 PAY  80 

    0xcc90d070,// 84 PAY  81 

    0xdbab11b4,// 85 PAY  82 

    0x958a7268,// 86 PAY  83 

    0x44d1e36e,// 87 PAY  84 

    0x841066c8,// 88 PAY  85 

    0x6742ce24,// 89 PAY  86 

    0xc0e8a5a3,// 90 PAY  87 

    0xee151feb,// 91 PAY  88 

    0x9c98c639,// 92 PAY  89 

    0xb8c6c77a,// 93 PAY  90 

    0xaee01503,// 94 PAY  91 

    0xdfe00597,// 95 PAY  92 

    0xfb36fc85,// 96 PAY  93 

    0x5a09845a,// 97 PAY  94 

    0xe981528a,// 98 PAY  95 

    0x477a8489,// 99 PAY  96 

    0x216ca9a7,// 100 PAY  97 

    0x6d789eac,// 101 PAY  98 

    0xdf22741b,// 102 PAY  99 

    0x2abaf1ac,// 103 PAY 100 

    0x56d4ee35,// 104 PAY 101 

    0x5431ceaf,// 105 PAY 102 

    0x39fac5f3,// 106 PAY 103 

    0x2db204a1,// 107 PAY 104 

    0xbc5312d6,// 108 PAY 105 

    0xd306461a,// 109 PAY 106 

    0xb5c7ea5b,// 110 PAY 107 

    0xd9de1a99,// 111 PAY 108 

    0x5d55c528,// 112 PAY 109 

    0x0242136e,// 113 PAY 110 

    0x0574955e,// 114 PAY 111 

    0xb000b614,// 115 PAY 112 

    0x69b6e810,// 116 PAY 113 

    0xba4d56e1,// 117 PAY 114 

    0xeb4ade86,// 118 PAY 115 

    0x029695cc,// 119 PAY 116 

    0x7d47f526,// 120 PAY 117 

    0x6bf05d43,// 121 PAY 118 

    0x54adcf17,// 122 PAY 119 

    0xbab39651,// 123 PAY 120 

    0xb996a1e0,// 124 PAY 121 

    0xd2ae1482,// 125 PAY 122 

    0x8fd18df0,// 126 PAY 123 

    0x1e341068,// 127 PAY 124 

    0x586be254,// 128 PAY 125 

    0x18cd70a8,// 129 PAY 126 

    0x70fe324e,// 130 PAY 127 

    0x895eed47,// 131 PAY 128 

    0x463a2bb2,// 132 PAY 129 

    0x029c2468,// 133 PAY 130 

    0x73b0c49d,// 134 PAY 131 

    0x6e33ed7c,// 135 PAY 132 

    0x682ec5e5,// 136 PAY 133 

    0xcfb7fd8d,// 137 PAY 134 

    0x79b18e37,// 138 PAY 135 

    0x73928311,// 139 PAY 136 

    0x17388abe,// 140 PAY 137 

    0x821562c0,// 141 PAY 138 

    0x9df7abc4,// 142 PAY 139 

    0x42d9a935,// 143 PAY 140 

    0x778a8d6c,// 144 PAY 141 

    0xafa8925d,// 145 PAY 142 

    0xd0738f1f,// 146 PAY 143 

    0xcea99eac,// 147 PAY 144 

    0x871c3261,// 148 PAY 145 

    0x7f092595,// 149 PAY 146 

    0xfdfd658d,// 150 PAY 147 

    0x83d42b41,// 151 PAY 148 

    0x023cc236,// 152 PAY 149 

    0x8f88f7f1,// 153 PAY 150 

    0x0e148636,// 154 PAY 151 

    0xad4e133c,// 155 PAY 152 

    0xac6449e8,// 156 PAY 153 

    0xbe776df8,// 157 PAY 154 

    0xb1b990f0,// 158 PAY 155 

    0xee8a2e3d,// 159 PAY 156 

    0xc64627b2,// 160 PAY 157 

    0x94846e3a,// 161 PAY 158 

    0x3358f984,// 162 PAY 159 

    0xf4218298,// 163 PAY 160 

    0x4e311b30,// 164 PAY 161 

    0x7a2b40ef,// 165 PAY 162 

    0x04463aae,// 166 PAY 163 

    0xc9e8c269,// 167 PAY 164 

    0xa46a3121,// 168 PAY 165 

    0x46944f5e,// 169 PAY 166 

    0x19fe819a,// 170 PAY 167 

    0xecd090da,// 171 PAY 168 

    0x5bec25a4,// 172 PAY 169 

    0x55f95002,// 173 PAY 170 

    0x8ad7854d,// 174 PAY 171 

    0xc4a0d347,// 175 PAY 172 

    0x9c0166b1,// 176 PAY 173 

    0xe61040b6,// 177 PAY 174 

    0xab88c6da,// 178 PAY 175 

    0xd6a0d29a,// 179 PAY 176 

    0x23142aa9,// 180 PAY 177 

    0x39976b94,// 181 PAY 178 

    0x931f7fbd,// 182 PAY 179 

    0x0a33e329,// 183 PAY 180 

    0x447c5408,// 184 PAY 181 

    0x2532bb54,// 185 PAY 182 

    0xa01846e7,// 186 PAY 183 

    0xe6ab66ef,// 187 PAY 184 

    0x16c332dc,// 188 PAY 185 

    0xa5c87361,// 189 PAY 186 

    0x848fce97,// 190 PAY 187 

    0xb8aa35a0,// 191 PAY 188 

    0x1cfd7feb,// 192 PAY 189 

    0xf6524c76,// 193 PAY 190 

    0x9beb5854,// 194 PAY 191 

    0x3f613d59,// 195 PAY 192 

    0xf7b811f5,// 196 PAY 193 

    0x3599d32f,// 197 PAY 194 

    0xa93f2e8b,// 198 PAY 195 

    0xc2ce43b0,// 199 PAY 196 

    0xb596b1e4,// 200 PAY 197 

    0x78401804,// 201 PAY 198 

    0x1dc586d5,// 202 PAY 199 

    0xd846dd7c,// 203 PAY 200 

    0xe13be88b,// 204 PAY 201 

    0xa5fcabeb,// 205 PAY 202 

    0x345232c3,// 206 PAY 203 

    0xd19b0e49,// 207 PAY 204 

    0xc7628c7e,// 208 PAY 205 

    0x5874e764,// 209 PAY 206 

    0xc7e01ca3,// 210 PAY 207 

    0x25bffc8b,// 211 PAY 208 

    0xe5456acc,// 212 PAY 209 

    0x37921523,// 213 PAY 210 

    0xeb59a883,// 214 PAY 211 

    0xb8ae060f,// 215 PAY 212 

    0x1aedccba,// 216 PAY 213 

    0xdf491617,// 217 PAY 214 

    0xcd805c9d,// 218 PAY 215 

    0x552ab502,// 219 PAY 216 

    0x6b8c1670,// 220 PAY 217 

    0x6a4b0d62,// 221 PAY 218 

    0x719e9780,// 222 PAY 219 

    0xb9016bdc,// 223 PAY 220 

    0x939dde8b,// 224 PAY 221 

    0x25c22281,// 225 PAY 222 

    0x74cfb3ac,// 226 PAY 223 

    0x94266d0c,// 227 PAY 224 

    0xfe787876,// 228 PAY 225 

    0xd9e7809a,// 229 PAY 226 

    0x19e28c4e,// 230 PAY 227 

    0x4affa8d5,// 231 PAY 228 

    0x18e76c95,// 232 PAY 229 

    0x0ad19475,// 233 PAY 230 

    0xe2cb05a4,// 234 PAY 231 

    0x72e3d475,// 235 PAY 232 

    0x5b0f0bf1,// 236 PAY 233 

    0xbe880ea9,// 237 PAY 234 

    0x520bbe77,// 238 PAY 235 

    0x6436cdcc,// 239 PAY 236 

    0x2d98941f,// 240 PAY 237 

    0xb09284ce,// 241 PAY 238 

    0xbb78c75c,// 242 PAY 239 

    0x13e8210d,// 243 PAY 240 

    0x7d3662fa,// 244 PAY 241 

    0x6c2331cf,// 245 PAY 242 

    0x20d58f1e,// 246 PAY 243 

    0x911418d2,// 247 PAY 244 

    0x88b99916,// 248 PAY 245 

    0x59562e49,// 249 PAY 246 

    0x261d00aa,// 250 PAY 247 

    0x99fc4b57,// 251 PAY 248 

    0x9cfb2630,// 252 PAY 249 

    0x12b63d5c,// 253 PAY 250 

    0x1aa200ec,// 254 PAY 251 

    0xbb4ba48b,// 255 PAY 252 

    0x16af669c,// 256 PAY 253 

    0x76820869,// 257 PAY 254 

    0x385a05f6,// 258 PAY 255 

    0xf9b4259c,// 259 PAY 256 

    0x9bad3109,// 260 PAY 257 

    0x4dcf7995,// 261 PAY 258 

    0x399bebd5,// 262 PAY 259 

    0xe52d04d4,// 263 PAY 260 

    0x1b51363a,// 264 PAY 261 

    0x0ff92493,// 265 PAY 262 

    0xed93c62b,// 266 PAY 263 

    0xaef2c1c5,// 267 PAY 264 

    0x89609835,// 268 PAY 265 

    0xade004ec,// 269 PAY 266 

    0xa6d67bed,// 270 PAY 267 

    0x45d0caa9,// 271 PAY 268 

    0xcecfc224,// 272 PAY 269 

    0x2854c413,// 273 PAY 270 

    0xd8076780,// 274 PAY 271 

    0x2036b372,// 275 PAY 272 

    0x793541c8,// 276 PAY 273 

    0x4e23283a,// 277 PAY 274 

    0xbb81e950,// 278 PAY 275 

    0xbbb1545b,// 279 PAY 276 

    0x81a5b24a,// 280 PAY 277 

    0x35edfbaa,// 281 PAY 278 

    0x33fe1a7f,// 282 PAY 279 

    0x41888c1e,// 283 PAY 280 

    0xf479fafc,// 284 PAY 281 

    0x92847917,// 285 PAY 282 

    0x324f3b60,// 286 PAY 283 

    0x4cca32f0,// 287 PAY 284 

    0xaab3b9cf,// 288 PAY 285 

    0x25f75d23,// 289 PAY 286 

    0xd7e3f09c,// 290 PAY 287 

    0x0b13752c,// 291 PAY 288 

    0x224a120f,// 292 PAY 289 

    0x9d4eb325,// 293 PAY 290 

    0x2bf17d19,// 294 PAY 291 

    0x0c675947,// 295 PAY 292 

    0x8bd83f65,// 296 PAY 293 

    0x54a8c2c6,// 297 PAY 294 

    0xba4acb91,// 298 PAY 295 

    0x43b02159,// 299 PAY 296 

    0x0fa7ac0c,// 300 PAY 297 

    0x61a36afd,// 301 PAY 298 

    0x5130fce5,// 302 PAY 299 

    0x86f87b61,// 303 PAY 300 

    0xca7f67db,// 304 PAY 301 

    0xb84825f0,// 305 PAY 302 

    0xea360a65,// 306 PAY 303 

    0xa21edc18,// 307 PAY 304 

    0xf5b333c8,// 308 PAY 305 

    0xf292a53a,// 309 PAY 306 

    0x85cb5325,// 310 PAY 307 

    0x86507b70,// 311 PAY 308 

    0x41fe111a,// 312 PAY 309 

    0xef7d4028,// 313 PAY 310 

    0xb97160de,// 314 PAY 311 

    0x240dc9d3,// 315 PAY 312 

    0xf58fdd8f,// 316 PAY 313 

    0x8a138fae,// 317 PAY 314 

    0x1b05579e,// 318 PAY 315 

    0x81b031ed,// 319 PAY 316 

    0x10ae0543,// 320 PAY 317 

    0x42d8087c,// 321 PAY 318 

    0x263dd099,// 322 PAY 319 

    0x516eb1d8,// 323 PAY 320 

    0x9deb2376,// 324 PAY 321 

    0x665243a9,// 325 PAY 322 

    0x0b24b82e,// 326 PAY 323 

    0x85aac5c0,// 327 PAY 324 

    0xa7617c4f,// 328 PAY 325 

    0x2d299fa3,// 329 PAY 326 

    0x7f774532,// 330 PAY 327 

    0x55f7b3f8,// 331 PAY 328 

    0x287f729f,// 332 PAY 329 

    0x94086ada,// 333 PAY 330 

    0xf2e4127c,// 334 PAY 331 

    0x722e7b8a,// 335 PAY 332 

    0xd5900bcf,// 336 PAY 333 

    0xb1952417,// 337 PAY 334 

    0xd795da30,// 338 PAY 335 

    0x5c6a82e2,// 339 PAY 336 

    0xac268884,// 340 PAY 337 

    0xc7a4f669,// 341 PAY 338 

    0xdf3ba2f8,// 342 PAY 339 

    0x0868c5e4,// 343 PAY 340 

    0x1a516d0a,// 344 PAY 341 

    0x7fa5bb87,// 345 PAY 342 

    0x202ed8e7,// 346 PAY 343 

    0xab6855b9,// 347 PAY 344 

    0x2b98f0ee,// 348 PAY 345 

    0xa849ec9d,// 349 PAY 346 

    0xf685442b,// 350 PAY 347 

    0x732727d9,// 351 PAY 348 

    0x63d29408,// 352 PAY 349 

    0x3a88a024,// 353 PAY 350 

    0xf08ef514,// 354 PAY 351 

    0x4dd8f103,// 355 PAY 352 

    0xe26442f7,// 356 PAY 353 

    0xb65a384e,// 357 PAY 354 

    0xb0e80ec9,// 358 PAY 355 

    0xb706da3b,// 359 PAY 356 

    0xe2d5f53d,// 360 PAY 357 

    0xa5dccd35,// 361 PAY 358 

    0x19524162,// 362 PAY 359 

    0xda1d1f37,// 363 PAY 360 

    0x24dd481c,// 364 PAY 361 

    0x0fd3a45c,// 365 PAY 362 

    0x46cab546,// 366 PAY 363 

    0x697159f0,// 367 PAY 364 

    0x35991265,// 368 PAY 365 

    0x24c60b3e,// 369 PAY 366 

    0xb607119e,// 370 PAY 367 

    0x516826da,// 371 PAY 368 

    0xc58f94be,// 372 PAY 369 

    0x1e38b218,// 373 PAY 370 

    0xb76de88d,// 374 PAY 371 

    0xbb9c9454,// 375 PAY 372 

    0x7a089d19,// 376 PAY 373 

    0x88a21c8b,// 377 PAY 374 

    0x0a46b515,// 378 PAY 375 

    0x03831bd5,// 379 PAY 376 

    0xda4dcd72,// 380 PAY 377 

    0x99e8335c,// 381 PAY 378 

    0x1f155f47,// 382 PAY 379 

    0xf1d81b16,// 383 PAY 380 

    0xc3e049ea,// 384 PAY 381 

    0x6f0fc1f4,// 385 PAY 382 

    0xd210ad12,// 386 PAY 383 

    0x093e0c1e,// 387 PAY 384 

    0xa680339b,// 388 PAY 385 

    0xcabe63d2,// 389 PAY 386 

    0x37291036,// 390 PAY 387 

    0xa0026072,// 391 PAY 388 

    0x35c834b8,// 392 PAY 389 

    0x4427e94c,// 393 PAY 390 

    0xc0a966b2,// 394 PAY 391 

    0x8d99cc57,// 395 PAY 392 

    0xf3ce67aa,// 396 PAY 393 

    0x5fb4a97d,// 397 PAY 394 

    0x6e10e075,// 398 PAY 395 

    0x6b7b762e,// 399 PAY 396 

    0x47b70c13,// 400 PAY 397 

    0x2e938d56,// 401 PAY 398 

    0xc454707b,// 402 PAY 399 

    0x4b3b8e71,// 403 PAY 400 

    0xed637217,// 404 PAY 401 

    0x76dc5bc2,// 405 PAY 402 

    0x49a57d50,// 406 PAY 403 

    0xe09cdce3,// 407 PAY 404 

    0xe026d10e,// 408 PAY 405 

    0x723a8197,// 409 PAY 406 

    0x3e14f691,// 410 PAY 407 

    0x74b9fc0c,// 411 PAY 408 

    0x4aa28a7e,// 412 PAY 409 

    0x99b2ecfd,// 413 PAY 410 

    0xa218e765,// 414 PAY 411 

    0x6a94aeb1,// 415 PAY 412 

    0xc5f724dd,// 416 PAY 413 

    0x629250b7,// 417 PAY 414 

    0xcbfd0a9d,// 418 PAY 415 

    0x4e76b4ff,// 419 PAY 416 

    0xe681f87e,// 420 PAY 417 

    0x5931b0dd,// 421 PAY 418 

    0x90c62ae5,// 422 PAY 419 

    0xc94bbeb2,// 423 PAY 420 

    0x8a88b534,// 424 PAY 421 

    0xbfec765f,// 425 PAY 422 

    0xec18c9a2,// 426 PAY 423 

    0x05d8e707,// 427 PAY 424 

    0xb963cb44,// 428 PAY 425 

    0x76d5cea8,// 429 PAY 426 

    0x6a0b9064,// 430 PAY 427 

    0x694549d4,// 431 PAY 428 

    0x45e90000,// 432 PAY 429 

/// STA is 1 words. 

/// STA num_pkts       : 163 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x58 

    0x03d958a3 // 433 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt53_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 206 words. 

/// BDA size     is 818 (0x332) 

/// BDA id       is 0x7e9d 

    0x03327e9d,// 3 BDA   1 

/// PAY Generic Data size   : 818 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x031b1dab,// 4 PAY   1 

    0x833dc7a1,// 5 PAY   2 

    0x85352d53,// 6 PAY   3 

    0x4c4c9dc0,// 7 PAY   4 

    0x4ab0789c,// 8 PAY   5 

    0xb33ee669,// 9 PAY   6 

    0x4bc33486,// 10 PAY   7 

    0xca4bd54a,// 11 PAY   8 

    0x43b19d95,// 12 PAY   9 

    0xf4ccf62c,// 13 PAY  10 

    0x2e1a0b76,// 14 PAY  11 

    0x1e964558,// 15 PAY  12 

    0xc71249f9,// 16 PAY  13 

    0xd23ec784,// 17 PAY  14 

    0x82a930ed,// 18 PAY  15 

    0xe6ba09a0,// 19 PAY  16 

    0x1f4b475b,// 20 PAY  17 

    0x0f71042e,// 21 PAY  18 

    0xbbaa06ca,// 22 PAY  19 

    0xedc48898,// 23 PAY  20 

    0x438cbc30,// 24 PAY  21 

    0x9fe81316,// 25 PAY  22 

    0x05850868,// 26 PAY  23 

    0x9656b8f0,// 27 PAY  24 

    0x45cd27ed,// 28 PAY  25 

    0x319829d8,// 29 PAY  26 

    0x5e52f244,// 30 PAY  27 

    0x6ade11e3,// 31 PAY  28 

    0xeda5eeab,// 32 PAY  29 

    0xd19163d6,// 33 PAY  30 

    0xdad582ec,// 34 PAY  31 

    0x196071e5,// 35 PAY  32 

    0x381c2509,// 36 PAY  33 

    0xfed7adc7,// 37 PAY  34 

    0x25d3f39e,// 38 PAY  35 

    0xd7538d4e,// 39 PAY  36 

    0x4004856c,// 40 PAY  37 

    0x8e9dc914,// 41 PAY  38 

    0xa76f7013,// 42 PAY  39 

    0x35780ff6,// 43 PAY  40 

    0x5093929a,// 44 PAY  41 

    0x03b3eb14,// 45 PAY  42 

    0x063e31fc,// 46 PAY  43 

    0xd4615185,// 47 PAY  44 

    0x1d12dd49,// 48 PAY  45 

    0xd5e4e8d5,// 49 PAY  46 

    0x2e5f2aa9,// 50 PAY  47 

    0xd45a46f9,// 51 PAY  48 

    0x24ecb69b,// 52 PAY  49 

    0xb809be75,// 53 PAY  50 

    0x6ad416f3,// 54 PAY  51 

    0x3fa5fc64,// 55 PAY  52 

    0x7f437d6d,// 56 PAY  53 

    0xb0e3eb0f,// 57 PAY  54 

    0x01304da1,// 58 PAY  55 

    0x852cbfc9,// 59 PAY  56 

    0xd6cf607f,// 60 PAY  57 

    0x496a85b4,// 61 PAY  58 

    0x395ce093,// 62 PAY  59 

    0x6c5057f3,// 63 PAY  60 

    0x473fe019,// 64 PAY  61 

    0xceb2fd89,// 65 PAY  62 

    0xc23600ff,// 66 PAY  63 

    0x0f105cbc,// 67 PAY  64 

    0x995889f6,// 68 PAY  65 

    0x7140433c,// 69 PAY  66 

    0x61c8c957,// 70 PAY  67 

    0x1530e6a9,// 71 PAY  68 

    0x2fd0a33a,// 72 PAY  69 

    0x3290fcf9,// 73 PAY  70 

    0x7e9fc35b,// 74 PAY  71 

    0xec096d11,// 75 PAY  72 

    0x997ea2aa,// 76 PAY  73 

    0xf1a65fc7,// 77 PAY  74 

    0xe90057a3,// 78 PAY  75 

    0x9e1fe629,// 79 PAY  76 

    0x870d63f3,// 80 PAY  77 

    0x35593eff,// 81 PAY  78 

    0x6dc31984,// 82 PAY  79 

    0xb4a7a1ae,// 83 PAY  80 

    0x0ac7f9c7,// 84 PAY  81 

    0x7b9eaf1e,// 85 PAY  82 

    0x7819e6f5,// 86 PAY  83 

    0xb17f2a01,// 87 PAY  84 

    0xa165bffc,// 88 PAY  85 

    0x4823e423,// 89 PAY  86 

    0xc42f6379,// 90 PAY  87 

    0x51d964ed,// 91 PAY  88 

    0xfa52d534,// 92 PAY  89 

    0x2673c795,// 93 PAY  90 

    0xf10c051a,// 94 PAY  91 

    0x4197dfea,// 95 PAY  92 

    0x2f3512c3,// 96 PAY  93 

    0xa629886a,// 97 PAY  94 

    0x29ac4124,// 98 PAY  95 

    0x472cb575,// 99 PAY  96 

    0x82ab8f6c,// 100 PAY  97 

    0x67008d77,// 101 PAY  98 

    0x351ed91d,// 102 PAY  99 

    0x24018caa,// 103 PAY 100 

    0x96ada729,// 104 PAY 101 

    0xda221212,// 105 PAY 102 

    0x23a3f21f,// 106 PAY 103 

    0x1a290894,// 107 PAY 104 

    0x3e93ffef,// 108 PAY 105 

    0x832354d5,// 109 PAY 106 

    0xc59aab98,// 110 PAY 107 

    0x63e8f534,// 111 PAY 108 

    0x6aaac5d7,// 112 PAY 109 

    0xf2f91458,// 113 PAY 110 

    0x8d96e7cd,// 114 PAY 111 

    0x651c62ab,// 115 PAY 112 

    0x6d512444,// 116 PAY 113 

    0xcce5979c,// 117 PAY 114 

    0x849da775,// 118 PAY 115 

    0xb95d55eb,// 119 PAY 116 

    0xec0179c9,// 120 PAY 117 

    0x64b7c874,// 121 PAY 118 

    0x858970a0,// 122 PAY 119 

    0x1c25bee2,// 123 PAY 120 

    0x341d3781,// 124 PAY 121 

    0x9548c4d8,// 125 PAY 122 

    0xc738c4f0,// 126 PAY 123 

    0x270a4077,// 127 PAY 124 

    0xa65c81c4,// 128 PAY 125 

    0x96666b17,// 129 PAY 126 

    0x7c6e0b36,// 130 PAY 127 

    0x39f837a4,// 131 PAY 128 

    0xb1439c5a,// 132 PAY 129 

    0xd97757c4,// 133 PAY 130 

    0x318b920b,// 134 PAY 131 

    0x5abad888,// 135 PAY 132 

    0x6258dd9d,// 136 PAY 133 

    0x7753a323,// 137 PAY 134 

    0xa8100013,// 138 PAY 135 

    0x14943b07,// 139 PAY 136 

    0xfcf94c16,// 140 PAY 137 

    0xa3ba9af9,// 141 PAY 138 

    0x414b01e8,// 142 PAY 139 

    0xb89c3380,// 143 PAY 140 

    0x915927c0,// 144 PAY 141 

    0x765cbb92,// 145 PAY 142 

    0x35bb8027,// 146 PAY 143 

    0x230ced20,// 147 PAY 144 

    0x2f829af4,// 148 PAY 145 

    0x97450a54,// 149 PAY 146 

    0x7460ef16,// 150 PAY 147 

    0x07b408b3,// 151 PAY 148 

    0xc2812255,// 152 PAY 149 

    0x244e192c,// 153 PAY 150 

    0x327d3910,// 154 PAY 151 

    0xf8a146d7,// 155 PAY 152 

    0xfcc0e5c4,// 156 PAY 153 

    0xfb88670b,// 157 PAY 154 

    0x36005265,// 158 PAY 155 

    0xdf9f9e91,// 159 PAY 156 

    0x5a4ab589,// 160 PAY 157 

    0xa55d340d,// 161 PAY 158 

    0x15a47640,// 162 PAY 159 

    0xe7ca6b1e,// 163 PAY 160 

    0x282eeb22,// 164 PAY 161 

    0x63984a88,// 165 PAY 162 

    0xea3dee0b,// 166 PAY 163 

    0xf28c9bcc,// 167 PAY 164 

    0x2c302de4,// 168 PAY 165 

    0x835622cd,// 169 PAY 166 

    0x82251c8b,// 170 PAY 167 

    0x39480446,// 171 PAY 168 

    0xe56b3cfb,// 172 PAY 169 

    0xca72f7d1,// 173 PAY 170 

    0xd7096e10,// 174 PAY 171 

    0x13c1055a,// 175 PAY 172 

    0xec7a285a,// 176 PAY 173 

    0x836dea3a,// 177 PAY 174 

    0x772d65eb,// 178 PAY 175 

    0x5deb013f,// 179 PAY 176 

    0x08b859b1,// 180 PAY 177 

    0xa2f484ee,// 181 PAY 178 

    0x08eba95b,// 182 PAY 179 

    0xf4cd8fd2,// 183 PAY 180 

    0xa5382fa4,// 184 PAY 181 

    0x9aef4aa7,// 185 PAY 182 

    0xbb778058,// 186 PAY 183 

    0x4b41c88c,// 187 PAY 184 

    0x4446f180,// 188 PAY 185 

    0x8ac14022,// 189 PAY 186 

    0x73486841,// 190 PAY 187 

    0x81293da1,// 191 PAY 188 

    0x8e54e2fd,// 192 PAY 189 

    0xcbfdfd77,// 193 PAY 190 

    0xfb0c97f1,// 194 PAY 191 

    0x6fce7047,// 195 PAY 192 

    0x92fef02a,// 196 PAY 193 

    0x9f527108,// 197 PAY 194 

    0xcdf6ca38,// 198 PAY 195 

    0xbd16e3bd,// 199 PAY 196 

    0x9b07e073,// 200 PAY 197 

    0x8a5d7a06,// 201 PAY 198 

    0x08f486e3,// 202 PAY 199 

    0xcae69a19,// 203 PAY 200 

    0x8333749a,// 204 PAY 201 

    0xd119567d,// 205 PAY 202 

    0xdabed7b6,// 206 PAY 203 

    0xb8e6132c,// 207 PAY 204 

    0x3b490000,// 208 PAY 205 

/// HASH is  16 bytes 

    0xf28c9bcc,// 209 HSH   1 

    0x2c302de4,// 210 HSH   2 

    0x835622cd,// 211 HSH   3 

    0x82251c8b,// 212 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 44 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf8 

    0x00b1f806 // 213 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt54_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 408 words. 

/// BDA size     is 1627 (0x65b) 

/// BDA id       is 0x5aee 

    0x065b5aee,// 3 BDA   1 

/// PAY Generic Data size   : 1627 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x34b149f4,// 4 PAY   1 

    0x0893b453,// 5 PAY   2 

    0xde108f3b,// 6 PAY   3 

    0xc77048ae,// 7 PAY   4 

    0xcda5d842,// 8 PAY   5 

    0x15625d34,// 9 PAY   6 

    0x3c7991fb,// 10 PAY   7 

    0x8bc66ef0,// 11 PAY   8 

    0x6cc44c50,// 12 PAY   9 

    0x4caddf4a,// 13 PAY  10 

    0x3bb9c417,// 14 PAY  11 

    0x34103ec8,// 15 PAY  12 

    0x0718b64e,// 16 PAY  13 

    0xb521271b,// 17 PAY  14 

    0x37785e81,// 18 PAY  15 

    0x2dd2e6b6,// 19 PAY  16 

    0x54edfa29,// 20 PAY  17 

    0x601a49a2,// 21 PAY  18 

    0x503b972f,// 22 PAY  19 

    0x4bf2f32d,// 23 PAY  20 

    0x83e8b926,// 24 PAY  21 

    0xf518f492,// 25 PAY  22 

    0x7c259850,// 26 PAY  23 

    0xba1a0c66,// 27 PAY  24 

    0xa2936ee5,// 28 PAY  25 

    0x52e357a6,// 29 PAY  26 

    0x7a1a9bc1,// 30 PAY  27 

    0x72434800,// 31 PAY  28 

    0xe9771ceb,// 32 PAY  29 

    0xf04681b2,// 33 PAY  30 

    0x36c1b3f9,// 34 PAY  31 

    0x76d7d013,// 35 PAY  32 

    0xf2251f7a,// 36 PAY  33 

    0x10df8a5d,// 37 PAY  34 

    0xac7f5792,// 38 PAY  35 

    0xdf39500e,// 39 PAY  36 

    0xd95ce01e,// 40 PAY  37 

    0x961db0f9,// 41 PAY  38 

    0x2b7c13cf,// 42 PAY  39 

    0xe951e51b,// 43 PAY  40 

    0xad15c4bb,// 44 PAY  41 

    0x362fab60,// 45 PAY  42 

    0x58b178ed,// 46 PAY  43 

    0x6f2b381c,// 47 PAY  44 

    0xf1206006,// 48 PAY  45 

    0x565f1929,// 49 PAY  46 

    0x9c211359,// 50 PAY  47 

    0xbb85c0a3,// 51 PAY  48 

    0x2d8899c6,// 52 PAY  49 

    0xd1620b92,// 53 PAY  50 

    0xcb1cbd34,// 54 PAY  51 

    0x0a546adc,// 55 PAY  52 

    0x7f329a48,// 56 PAY  53 

    0x65f5c345,// 57 PAY  54 

    0xbbb475ec,// 58 PAY  55 

    0xdbb7c66e,// 59 PAY  56 

    0xa00951ed,// 60 PAY  57 

    0x51abebe9,// 61 PAY  58 

    0x2ba2bff0,// 62 PAY  59 

    0x95d9222a,// 63 PAY  60 

    0x84a54c20,// 64 PAY  61 

    0xb4bbfd8c,// 65 PAY  62 

    0xfbb495a9,// 66 PAY  63 

    0xc859983d,// 67 PAY  64 

    0x14abac79,// 68 PAY  65 

    0x570164ee,// 69 PAY  66 

    0xbae1f14c,// 70 PAY  67 

    0x1f56759c,// 71 PAY  68 

    0x4f2f3073,// 72 PAY  69 

    0xfcdbea45,// 73 PAY  70 

    0xb9521a34,// 74 PAY  71 

    0x568bc973,// 75 PAY  72 

    0x07170d01,// 76 PAY  73 

    0x2b418779,// 77 PAY  74 

    0x9df8c4f3,// 78 PAY  75 

    0x74638660,// 79 PAY  76 

    0xe2390ca0,// 80 PAY  77 

    0x331e4676,// 81 PAY  78 

    0xe5c01249,// 82 PAY  79 

    0x1c9ae511,// 83 PAY  80 

    0x0243124e,// 84 PAY  81 

    0x372fab29,// 85 PAY  82 

    0x51636147,// 86 PAY  83 

    0xa69a0bb2,// 87 PAY  84 

    0xd8028d9d,// 88 PAY  85 

    0x3de03004,// 89 PAY  86 

    0xe44eff91,// 90 PAY  87 

    0x52dea5f6,// 91 PAY  88 

    0xa058c052,// 92 PAY  89 

    0xdb85f737,// 93 PAY  90 

    0x3091e70a,// 94 PAY  91 

    0xee35e9b6,// 95 PAY  92 

    0x799ba375,// 96 PAY  93 

    0xb24e31f6,// 97 PAY  94 

    0xea66df9b,// 98 PAY  95 

    0x6b2caf27,// 99 PAY  96 

    0x3a3dccde,// 100 PAY  97 

    0xc50356f3,// 101 PAY  98 

    0x2902115b,// 102 PAY  99 

    0xe5559f0e,// 103 PAY 100 

    0x14093c5c,// 104 PAY 101 

    0xb78e919e,// 105 PAY 102 

    0x704065a5,// 106 PAY 103 

    0xe1664ec0,// 107 PAY 104 

    0x2a18f67f,// 108 PAY 105 

    0xed4f460d,// 109 PAY 106 

    0x7c5cfbbd,// 110 PAY 107 

    0xd07cbe76,// 111 PAY 108 

    0xbc01499c,// 112 PAY 109 

    0x4c8f69f1,// 113 PAY 110 

    0x846b01a8,// 114 PAY 111 

    0x6564d339,// 115 PAY 112 

    0x72a3a541,// 116 PAY 113 

    0xca9f8316,// 117 PAY 114 

    0xed78115a,// 118 PAY 115 

    0x31bc7eb8,// 119 PAY 116 

    0x1b3e1472,// 120 PAY 117 

    0xf521e154,// 121 PAY 118 

    0x8d183a2f,// 122 PAY 119 

    0x60916a25,// 123 PAY 120 

    0x58f773c6,// 124 PAY 121 

    0xa354c182,// 125 PAY 122 

    0x25b9df2a,// 126 PAY 123 

    0x927ac74b,// 127 PAY 124 

    0x093c8a95,// 128 PAY 125 

    0x30e1a09a,// 129 PAY 126 

    0x2c5b487b,// 130 PAY 127 

    0x41f86f40,// 131 PAY 128 

    0x440d9f5b,// 132 PAY 129 

    0x1d6baa44,// 133 PAY 130 

    0x9443395d,// 134 PAY 131 

    0xd5a6a006,// 135 PAY 132 

    0x8b1dd47d,// 136 PAY 133 

    0x66be5e5c,// 137 PAY 134 

    0x26fcd882,// 138 PAY 135 

    0x8b317986,// 139 PAY 136 

    0xbf9a0c5b,// 140 PAY 137 

    0x24226e19,// 141 PAY 138 

    0x7ea66bbe,// 142 PAY 139 

    0xb78f01ec,// 143 PAY 140 

    0xe4c597c7,// 144 PAY 141 

    0xe0c67425,// 145 PAY 142 

    0xaecb40d1,// 146 PAY 143 

    0xc701a0aa,// 147 PAY 144 

    0xad8aa9cf,// 148 PAY 145 

    0x53765bb3,// 149 PAY 146 

    0xead7f775,// 150 PAY 147 

    0xc862064a,// 151 PAY 148 

    0xaacb161f,// 152 PAY 149 

    0x29b7bd96,// 153 PAY 150 

    0x436e1ec7,// 154 PAY 151 

    0x25cade8f,// 155 PAY 152 

    0x372a07d3,// 156 PAY 153 

    0x3d1bd843,// 157 PAY 154 

    0x0e36ca37,// 158 PAY 155 

    0xd27b6518,// 159 PAY 156 

    0x762f6b2b,// 160 PAY 157 

    0xa5d22678,// 161 PAY 158 

    0x2a269adf,// 162 PAY 159 

    0xe8fe853a,// 163 PAY 160 

    0xdfaf1f9a,// 164 PAY 161 

    0xcc47ddd3,// 165 PAY 162 

    0x94c562e4,// 166 PAY 163 

    0xb258dad4,// 167 PAY 164 

    0xc6bc47ef,// 168 PAY 165 

    0x0bf21330,// 169 PAY 166 

    0x40efcb52,// 170 PAY 167 

    0x6a0c1c9e,// 171 PAY 168 

    0x4028a955,// 172 PAY 169 

    0xa211b234,// 173 PAY 170 

    0x2b4cfbbd,// 174 PAY 171 

    0x90b7a519,// 175 PAY 172 

    0x89a80d12,// 176 PAY 173 

    0x765e2b49,// 177 PAY 174 

    0x300d713c,// 178 PAY 175 

    0x663439d1,// 179 PAY 176 

    0xa0f4e67d,// 180 PAY 177 

    0xad86c10f,// 181 PAY 178 

    0x57bddc45,// 182 PAY 179 

    0x26946cf6,// 183 PAY 180 

    0xeb23201e,// 184 PAY 181 

    0xd0040ee2,// 185 PAY 182 

    0xcc0e81ef,// 186 PAY 183 

    0x047d57a5,// 187 PAY 184 

    0x336663a8,// 188 PAY 185 

    0x8cdcc7fe,// 189 PAY 186 

    0xefeefb18,// 190 PAY 187 

    0xd79f3741,// 191 PAY 188 

    0x3e0467d3,// 192 PAY 189 

    0xdcceb00c,// 193 PAY 190 

    0xf0bb588e,// 194 PAY 191 

    0xfe28b5a7,// 195 PAY 192 

    0x6829efb8,// 196 PAY 193 

    0x1e5aeb80,// 197 PAY 194 

    0xf739dddf,// 198 PAY 195 

    0x6cabdb76,// 199 PAY 196 

    0xf13eea99,// 200 PAY 197 

    0x72f55e78,// 201 PAY 198 

    0x059dc7a6,// 202 PAY 199 

    0xef432c35,// 203 PAY 200 

    0xb175c672,// 204 PAY 201 

    0x09e4360a,// 205 PAY 202 

    0x5f47cc1b,// 206 PAY 203 

    0x611666fa,// 207 PAY 204 

    0xc6055694,// 208 PAY 205 

    0x15ba1d21,// 209 PAY 206 

    0xedeefcc3,// 210 PAY 207 

    0x7d389f7d,// 211 PAY 208 

    0x3919e234,// 212 PAY 209 

    0xf4a962f3,// 213 PAY 210 

    0x6a445ee7,// 214 PAY 211 

    0xa039a10d,// 215 PAY 212 

    0xa98e23f4,// 216 PAY 213 

    0xe7cfe7e8,// 217 PAY 214 

    0x2455c6e2,// 218 PAY 215 

    0xf2423e8e,// 219 PAY 216 

    0xc17bbf6e,// 220 PAY 217 

    0x74b7ed6b,// 221 PAY 218 

    0x8ce1cbe5,// 222 PAY 219 

    0x1d276b95,// 223 PAY 220 

    0x83059754,// 224 PAY 221 

    0x3bea12b5,// 225 PAY 222 

    0xfdd8c924,// 226 PAY 223 

    0x537e2a26,// 227 PAY 224 

    0xc343612f,// 228 PAY 225 

    0x0d632e7c,// 229 PAY 226 

    0x424e9a2a,// 230 PAY 227 

    0xad6c552f,// 231 PAY 228 

    0x48797e51,// 232 PAY 229 

    0x93cf7f2b,// 233 PAY 230 

    0xa05977e8,// 234 PAY 231 

    0x6f376e88,// 235 PAY 232 

    0x76e09e46,// 236 PAY 233 

    0xaf98c4fb,// 237 PAY 234 

    0x5e0354d1,// 238 PAY 235 

    0x10e1c65b,// 239 PAY 236 

    0x26e713a1,// 240 PAY 237 

    0xc81203f5,// 241 PAY 238 

    0x2ee4b1ae,// 242 PAY 239 

    0x2c4e7110,// 243 PAY 240 

    0x08c50c82,// 244 PAY 241 

    0x8fb30aa4,// 245 PAY 242 

    0xcddb9653,// 246 PAY 243 

    0x25caa15d,// 247 PAY 244 

    0xf799ebef,// 248 PAY 245 

    0xb9a44399,// 249 PAY 246 

    0x5018d0b0,// 250 PAY 247 

    0x3042d401,// 251 PAY 248 

    0x7b791494,// 252 PAY 249 

    0x7accd9a8,// 253 PAY 250 

    0x53ea24bd,// 254 PAY 251 

    0x2c63bac6,// 255 PAY 252 

    0x4c5b0e62,// 256 PAY 253 

    0xa0df6f40,// 257 PAY 254 

    0x15c7e5fc,// 258 PAY 255 

    0x659c5c6b,// 259 PAY 256 

    0x2bafa06f,// 260 PAY 257 

    0x06d8442a,// 261 PAY 258 

    0x8ac77a33,// 262 PAY 259 

    0x1caf788f,// 263 PAY 260 

    0x29bfd3b0,// 264 PAY 261 

    0x2ef0a190,// 265 PAY 262 

    0xeccd1a8a,// 266 PAY 263 

    0x1c8af9c1,// 267 PAY 264 

    0xdd7bcd57,// 268 PAY 265 

    0x28917edd,// 269 PAY 266 

    0x2ad38c90,// 270 PAY 267 

    0xd607c043,// 271 PAY 268 

    0x99326c50,// 272 PAY 269 

    0xddc7efe6,// 273 PAY 270 

    0x4af2322d,// 274 PAY 271 

    0x677a82ba,// 275 PAY 272 

    0x589eedef,// 276 PAY 273 

    0xb73abb3b,// 277 PAY 274 

    0x7bb95ffa,// 278 PAY 275 

    0x8ab1bd4b,// 279 PAY 276 

    0x5bd25eef,// 280 PAY 277 

    0xaf384d14,// 281 PAY 278 

    0x9d9443df,// 282 PAY 279 

    0xb6a7b111,// 283 PAY 280 

    0x24dcf8c9,// 284 PAY 281 

    0x994ad3fe,// 285 PAY 282 

    0x4fdcef7e,// 286 PAY 283 

    0x39e7b392,// 287 PAY 284 

    0xf9a509cf,// 288 PAY 285 

    0xd647f1ff,// 289 PAY 286 

    0x6796bd2c,// 290 PAY 287 

    0x0853cdd4,// 291 PAY 288 

    0x06acf5e3,// 292 PAY 289 

    0x3965050b,// 293 PAY 290 

    0xc0ccb065,// 294 PAY 291 

    0xfc5a6715,// 295 PAY 292 

    0xdcb9bd85,// 296 PAY 293 

    0x3a3a355a,// 297 PAY 294 

    0x6b73aac9,// 298 PAY 295 

    0xd4a20c86,// 299 PAY 296 

    0xe351c63c,// 300 PAY 297 

    0x7689acff,// 301 PAY 298 

    0xcc2ab1c2,// 302 PAY 299 

    0x0146bf3d,// 303 PAY 300 

    0xc3b34387,// 304 PAY 301 

    0x97a4254e,// 305 PAY 302 

    0xdeed97f4,// 306 PAY 303 

    0xc20c9f43,// 307 PAY 304 

    0xa17154d2,// 308 PAY 305 

    0x912fe102,// 309 PAY 306 

    0xac97d1a2,// 310 PAY 307 

    0x270511cb,// 311 PAY 308 

    0x6f63a0d0,// 312 PAY 309 

    0xebc8ac04,// 313 PAY 310 

    0xdd2ee351,// 314 PAY 311 

    0xc069ed2d,// 315 PAY 312 

    0xa9b9803e,// 316 PAY 313 

    0x068fb93d,// 317 PAY 314 

    0x7ff58a9f,// 318 PAY 315 

    0x4d5ca911,// 319 PAY 316 

    0x0ca208cb,// 320 PAY 317 

    0x634bea88,// 321 PAY 318 

    0xcf0dfdc6,// 322 PAY 319 

    0xd712c825,// 323 PAY 320 

    0x76d0ddaa,// 324 PAY 321 

    0x9ab55f02,// 325 PAY 322 

    0xdffebf99,// 326 PAY 323 

    0xc28d730e,// 327 PAY 324 

    0xe436979c,// 328 PAY 325 

    0x65e1c112,// 329 PAY 326 

    0x65772785,// 330 PAY 327 

    0x122c6d8c,// 331 PAY 328 

    0x5d793523,// 332 PAY 329 

    0x733a1b4d,// 333 PAY 330 

    0xd6176b84,// 334 PAY 331 

    0xe86b0335,// 335 PAY 332 

    0xfad14dde,// 336 PAY 333 

    0x36777a3f,// 337 PAY 334 

    0x62774976,// 338 PAY 335 

    0x2f6d3b34,// 339 PAY 336 

    0xacbd237d,// 340 PAY 337 

    0x01d6b766,// 341 PAY 338 

    0x75e1ba9e,// 342 PAY 339 

    0x00239250,// 343 PAY 340 

    0x64b2db71,// 344 PAY 341 

    0x32ab4164,// 345 PAY 342 

    0x7a50d7aa,// 346 PAY 343 

    0xaa4c4289,// 347 PAY 344 

    0x161efa2e,// 348 PAY 345 

    0xda2e8dbd,// 349 PAY 346 

    0xf82f2157,// 350 PAY 347 

    0x15bda06a,// 351 PAY 348 

    0xa3b535c3,// 352 PAY 349 

    0x752a4ce1,// 353 PAY 350 

    0x4538a5d8,// 354 PAY 351 

    0xdd4e1f73,// 355 PAY 352 

    0x85c82fd0,// 356 PAY 353 

    0xfaa865d0,// 357 PAY 354 

    0x45ddf5a6,// 358 PAY 355 

    0x5f3cbe26,// 359 PAY 356 

    0x2d47d674,// 360 PAY 357 

    0xe6b16b3d,// 361 PAY 358 

    0x8be33066,// 362 PAY 359 

    0x788f4636,// 363 PAY 360 

    0xa80851b8,// 364 PAY 361 

    0x40c7d263,// 365 PAY 362 

    0xdc2db729,// 366 PAY 363 

    0x765269f1,// 367 PAY 364 

    0x98b6e39a,// 368 PAY 365 

    0xeccdd1de,// 369 PAY 366 

    0x51760d36,// 370 PAY 367 

    0x22210e8a,// 371 PAY 368 

    0xdf8c24a9,// 372 PAY 369 

    0x2f28d4fa,// 373 PAY 370 

    0x1b6cb558,// 374 PAY 371 

    0xc848f11c,// 375 PAY 372 

    0xb0fbef19,// 376 PAY 373 

    0x715ad0df,// 377 PAY 374 

    0x19ba9e99,// 378 PAY 375 

    0x435ba109,// 379 PAY 376 

    0xc406e7bf,// 380 PAY 377 

    0x8d606cea,// 381 PAY 378 

    0xfb3074da,// 382 PAY 379 

    0xc0e1aaf2,// 383 PAY 380 

    0x668a9cd2,// 384 PAY 381 

    0x5f429812,// 385 PAY 382 

    0x4d7a6f1e,// 386 PAY 383 

    0x1c98f45a,// 387 PAY 384 

    0x2cce5fd3,// 388 PAY 385 

    0xb319f5b8,// 389 PAY 386 

    0xc5a1aeff,// 390 PAY 387 

    0x509690b9,// 391 PAY 388 

    0xde179983,// 392 PAY 389 

    0x3920680b,// 393 PAY 390 

    0xa8945cce,// 394 PAY 391 

    0x47aa8fb9,// 395 PAY 392 

    0x36691774,// 396 PAY 393 

    0xfc93d479,// 397 PAY 394 

    0xa4dbc077,// 398 PAY 395 

    0x4e395a01,// 399 PAY 396 

    0x7d8734be,// 400 PAY 397 

    0xd756441a,// 401 PAY 398 

    0xaa54ac1e,// 402 PAY 399 

    0x24cb5e4d,// 403 PAY 400 

    0xf1c68c10,// 404 PAY 401 

    0xa1198d72,// 405 PAY 402 

    0xf006a015,// 406 PAY 403 

    0xaa3dd3d0,// 407 PAY 404 

    0xbd5979f4,// 408 PAY 405 

    0x6d304162,// 409 PAY 406 

    0x69e28300,// 410 PAY 407 

/// HASH is  12 bytes 

    0xf799ebef,// 411 HSH   1 

    0xb9a44399,// 412 HSH   2 

    0x5018d0b0,// 413 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 33 

/// STA pkt_idx        : 109 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x19 

    0x01b41921 // 414 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt55_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 452 words. 

/// BDA size     is 1804 (0x70c) 

/// BDA id       is 0x49d1 

    0x070c49d1,// 3 BDA   1 

/// PAY Generic Data size   : 1804 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x94f57c48,// 4 PAY   1 

    0x277a8a19,// 5 PAY   2 

    0xee558153,// 6 PAY   3 

    0x8fcbb7d2,// 7 PAY   4 

    0x88099a36,// 8 PAY   5 

    0x6d47157a,// 9 PAY   6 

    0xcb15cd71,// 10 PAY   7 

    0x5ae93a45,// 11 PAY   8 

    0xea012dfd,// 12 PAY   9 

    0xbf16031d,// 13 PAY  10 

    0x0e3b1bbf,// 14 PAY  11 

    0xeb38b04f,// 15 PAY  12 

    0x39ecbddb,// 16 PAY  13 

    0x33b1efc7,// 17 PAY  14 

    0xc78dff1b,// 18 PAY  15 

    0x5ab9b5ec,// 19 PAY  16 

    0x7dcc89ba,// 20 PAY  17 

    0x7456c910,// 21 PAY  18 

    0x2d5b279b,// 22 PAY  19 

    0x41aff120,// 23 PAY  20 

    0x2e5e5249,// 24 PAY  21 

    0x4368fde9,// 25 PAY  22 

    0x8c3d65c3,// 26 PAY  23 

    0xc6711a14,// 27 PAY  24 

    0xdd4f88dd,// 28 PAY  25 

    0x8dd3927e,// 29 PAY  26 

    0x92421c3f,// 30 PAY  27 

    0x038c566b,// 31 PAY  28 

    0x5d1a1fb7,// 32 PAY  29 

    0x6ddc4045,// 33 PAY  30 

    0x38461262,// 34 PAY  31 

    0x7ab9f94f,// 35 PAY  32 

    0xb4003ab5,// 36 PAY  33 

    0x1e35803e,// 37 PAY  34 

    0x2bd124e0,// 38 PAY  35 

    0x503eb667,// 39 PAY  36 

    0x40c454d3,// 40 PAY  37 

    0x698fc5fb,// 41 PAY  38 

    0xf6ef5726,// 42 PAY  39 

    0xb4b31e5b,// 43 PAY  40 

    0x905596ef,// 44 PAY  41 

    0x98319123,// 45 PAY  42 

    0xfad8ab2e,// 46 PAY  43 

    0x105898b7,// 47 PAY  44 

    0xeda103f8,// 48 PAY  45 

    0x8184953b,// 49 PAY  46 

    0x5b305945,// 50 PAY  47 

    0x517e6c46,// 51 PAY  48 

    0xafe29b3b,// 52 PAY  49 

    0xe12720f7,// 53 PAY  50 

    0x8d0c7a15,// 54 PAY  51 

    0xa396c7af,// 55 PAY  52 

    0x928a02e3,// 56 PAY  53 

    0x09f215a8,// 57 PAY  54 

    0xc7bcf216,// 58 PAY  55 

    0x4f7811aa,// 59 PAY  56 

    0x17834467,// 60 PAY  57 

    0x7b207636,// 61 PAY  58 

    0x922ad5ab,// 62 PAY  59 

    0x974d7ab6,// 63 PAY  60 

    0x1ae0739e,// 64 PAY  61 

    0xcf186b5d,// 65 PAY  62 

    0x3539a9da,// 66 PAY  63 

    0xa4668b9e,// 67 PAY  64 

    0xe0416142,// 68 PAY  65 

    0x65173df6,// 69 PAY  66 

    0x84af7802,// 70 PAY  67 

    0x08e54d8c,// 71 PAY  68 

    0xfc680977,// 72 PAY  69 

    0x2031b82b,// 73 PAY  70 

    0xc750d0c2,// 74 PAY  71 

    0x1f603cab,// 75 PAY  72 

    0x2429aa19,// 76 PAY  73 

    0xa7fa2134,// 77 PAY  74 

    0xd2a9a812,// 78 PAY  75 

    0x0cbd0cdd,// 79 PAY  76 

    0x1c507038,// 80 PAY  77 

    0x7b03628b,// 81 PAY  78 

    0xf0c22c99,// 82 PAY  79 

    0xbda6093f,// 83 PAY  80 

    0x665eb2d9,// 84 PAY  81 

    0xc58f0d33,// 85 PAY  82 

    0xa045ca1a,// 86 PAY  83 

    0x99d6e620,// 87 PAY  84 

    0xd18847ce,// 88 PAY  85 

    0xf0d38c14,// 89 PAY  86 

    0xf6e1a30d,// 90 PAY  87 

    0xd4185e2a,// 91 PAY  88 

    0x18f428e1,// 92 PAY  89 

    0x21070430,// 93 PAY  90 

    0x11f85afd,// 94 PAY  91 

    0x5ebac0fc,// 95 PAY  92 

    0xc7ba878d,// 96 PAY  93 

    0x02ce0328,// 97 PAY  94 

    0x213f4346,// 98 PAY  95 

    0x122b5570,// 99 PAY  96 

    0xe6010a57,// 100 PAY  97 

    0x7b346e25,// 101 PAY  98 

    0x37e15219,// 102 PAY  99 

    0x474045d0,// 103 PAY 100 

    0x12a79319,// 104 PAY 101 

    0xc6c6e811,// 105 PAY 102 

    0xcc069352,// 106 PAY 103 

    0x92781627,// 107 PAY 104 

    0x658c7fd1,// 108 PAY 105 

    0x0c54f60c,// 109 PAY 106 

    0xb8d66247,// 110 PAY 107 

    0xf5d10f9f,// 111 PAY 108 

    0x42bbf40f,// 112 PAY 109 

    0x1c70b5bd,// 113 PAY 110 

    0x47cd96aa,// 114 PAY 111 

    0xaecfb0e2,// 115 PAY 112 

    0x35b1b6f8,// 116 PAY 113 

    0xf7272933,// 117 PAY 114 

    0x4d447ff4,// 118 PAY 115 

    0xba47629c,// 119 PAY 116 

    0x223490c0,// 120 PAY 117 

    0xf6b05768,// 121 PAY 118 

    0x4e9fa973,// 122 PAY 119 

    0xdf095662,// 123 PAY 120 

    0xc40a0332,// 124 PAY 121 

    0x0d453e0f,// 125 PAY 122 

    0x5ce6659c,// 126 PAY 123 

    0xf530700a,// 127 PAY 124 

    0xb6a0d26e,// 128 PAY 125 

    0x86d0eb64,// 129 PAY 126 

    0x399bc998,// 130 PAY 127 

    0xf8215d57,// 131 PAY 128 

    0x134e8099,// 132 PAY 129 

    0x2bbe33c4,// 133 PAY 130 

    0x67122f17,// 134 PAY 131 

    0x3a2382d7,// 135 PAY 132 

    0x7a0c233c,// 136 PAY 133 

    0x19f5a87d,// 137 PAY 134 

    0x0fe6fcc5,// 138 PAY 135 

    0x4aced94d,// 139 PAY 136 

    0x6ec3c41e,// 140 PAY 137 

    0x1d1fbdf1,// 141 PAY 138 

    0x36f7b129,// 142 PAY 139 

    0x69499e3d,// 143 PAY 140 

    0xfcea2ba0,// 144 PAY 141 

    0x4f94f478,// 145 PAY 142 

    0xda3853e6,// 146 PAY 143 

    0x610c2c07,// 147 PAY 144 

    0x4e98c9ca,// 148 PAY 145 

    0x6a384242,// 149 PAY 146 

    0xdc99fa05,// 150 PAY 147 

    0x7b86cdae,// 151 PAY 148 

    0x9c84ef76,// 152 PAY 149 

    0x1ed5f5ec,// 153 PAY 150 

    0x6cbd361d,// 154 PAY 151 

    0x057f0308,// 155 PAY 152 

    0xac5aee4b,// 156 PAY 153 

    0xe08c2528,// 157 PAY 154 

    0xd5f877d3,// 158 PAY 155 

    0x327d4776,// 159 PAY 156 

    0x8bddaa15,// 160 PAY 157 

    0x07ea3683,// 161 PAY 158 

    0x09cef9d3,// 162 PAY 159 

    0x8cc3ff7e,// 163 PAY 160 

    0xfb12f09a,// 164 PAY 161 

    0xb18b3746,// 165 PAY 162 

    0xccd74698,// 166 PAY 163 

    0xfbbec903,// 167 PAY 164 

    0x621eceb3,// 168 PAY 165 

    0x0c4b8803,// 169 PAY 166 

    0xc6d6b38d,// 170 PAY 167 

    0xf4cc4905,// 171 PAY 168 

    0x1138fc88,// 172 PAY 169 

    0x131e7e04,// 173 PAY 170 

    0x6ee948b6,// 174 PAY 171 

    0xdc9413e8,// 175 PAY 172 

    0x9f73ca90,// 176 PAY 173 

    0x4f353f63,// 177 PAY 174 

    0x19567da9,// 178 PAY 175 

    0x61c856b2,// 179 PAY 176 

    0xa1979a1b,// 180 PAY 177 

    0x58304d47,// 181 PAY 178 

    0xf7c17ff4,// 182 PAY 179 

    0xd4e62b40,// 183 PAY 180 

    0x43900245,// 184 PAY 181 

    0x11ea6233,// 185 PAY 182 

    0x2a55fb6b,// 186 PAY 183 

    0x0210de29,// 187 PAY 184 

    0x090de7ab,// 188 PAY 185 

    0x747626a6,// 189 PAY 186 

    0xe1f1e82a,// 190 PAY 187 

    0x0ea840e2,// 191 PAY 188 

    0x2c4c1351,// 192 PAY 189 

    0xed1f1a04,// 193 PAY 190 

    0x009b3635,// 194 PAY 191 

    0xd1f27750,// 195 PAY 192 

    0x9a00d5d9,// 196 PAY 193 

    0x02e2422e,// 197 PAY 194 

    0x3fe150d1,// 198 PAY 195 

    0x63f2edd7,// 199 PAY 196 

    0xfe54afe8,// 200 PAY 197 

    0x6b901027,// 201 PAY 198 

    0xfe637c22,// 202 PAY 199 

    0x8a2847d7,// 203 PAY 200 

    0x8a19a759,// 204 PAY 201 

    0xc9980b03,// 205 PAY 202 

    0xb7964de6,// 206 PAY 203 

    0x4d766fb1,// 207 PAY 204 

    0x50b99e61,// 208 PAY 205 

    0x453874a6,// 209 PAY 206 

    0x07656fb1,// 210 PAY 207 

    0x32898b42,// 211 PAY 208 

    0xe848060d,// 212 PAY 209 

    0xbbfaecc7,// 213 PAY 210 

    0xf1fa5d86,// 214 PAY 211 

    0x399c99e3,// 215 PAY 212 

    0xd86aaa81,// 216 PAY 213 

    0xcf7229c9,// 217 PAY 214 

    0x9dec40b1,// 218 PAY 215 

    0x4ec8267c,// 219 PAY 216 

    0x125e4b90,// 220 PAY 217 

    0x645a8869,// 221 PAY 218 

    0xd14eb79d,// 222 PAY 219 

    0x306a99b9,// 223 PAY 220 

    0x1f7aee20,// 224 PAY 221 

    0x55bb6bd1,// 225 PAY 222 

    0x2721bdac,// 226 PAY 223 

    0xc211027a,// 227 PAY 224 

    0x5409a660,// 228 PAY 225 

    0xb5403db1,// 229 PAY 226 

    0x7fd63903,// 230 PAY 227 

    0x2da14510,// 231 PAY 228 

    0x0c5f5e85,// 232 PAY 229 

    0x9a84413f,// 233 PAY 230 

    0x2be19c84,// 234 PAY 231 

    0x32cbc8d8,// 235 PAY 232 

    0xc2cac576,// 236 PAY 233 

    0x88ce2836,// 237 PAY 234 

    0xfacf8d96,// 238 PAY 235 

    0x41db8d35,// 239 PAY 236 

    0x76dbceee,// 240 PAY 237 

    0xae3c4f4c,// 241 PAY 238 

    0xf8bca1b6,// 242 PAY 239 

    0x440400d3,// 243 PAY 240 

    0x0f139480,// 244 PAY 241 

    0x416fff05,// 245 PAY 242 

    0x9544918f,// 246 PAY 243 

    0xcf4ad08b,// 247 PAY 244 

    0xdce65f82,// 248 PAY 245 

    0xd6a16c8d,// 249 PAY 246 

    0x1d70b17f,// 250 PAY 247 

    0xf2ca7161,// 251 PAY 248 

    0xed566f61,// 252 PAY 249 

    0xea316f3d,// 253 PAY 250 

    0xc9412477,// 254 PAY 251 

    0xa9fb7fa9,// 255 PAY 252 

    0x1e251d49,// 256 PAY 253 

    0x0e085400,// 257 PAY 254 

    0xaac94bd1,// 258 PAY 255 

    0x934c7670,// 259 PAY 256 

    0xdc64ac6e,// 260 PAY 257 

    0xc49e3d0c,// 261 PAY 258 

    0xbf2b7b7a,// 262 PAY 259 

    0x6506af88,// 263 PAY 260 

    0x74bc241c,// 264 PAY 261 

    0x9babf201,// 265 PAY 262 

    0x7f34dcc7,// 266 PAY 263 

    0x0ac28d5e,// 267 PAY 264 

    0xae686405,// 268 PAY 265 

    0xee5a657f,// 269 PAY 266 

    0xf53cf28b,// 270 PAY 267 

    0x30818345,// 271 PAY 268 

    0x122251ca,// 272 PAY 269 

    0x99695687,// 273 PAY 270 

    0xbbb6c5d4,// 274 PAY 271 

    0x75016613,// 275 PAY 272 

    0x4d4caabe,// 276 PAY 273 

    0x55861def,// 277 PAY 274 

    0x67b0131d,// 278 PAY 275 

    0x74fd5b7f,// 279 PAY 276 

    0x769afcee,// 280 PAY 277 

    0x9b6be906,// 281 PAY 278 

    0xed95a007,// 282 PAY 279 

    0x9c8aec83,// 283 PAY 280 

    0x1db9c045,// 284 PAY 281 

    0xb2ac0f88,// 285 PAY 282 

    0x08b07060,// 286 PAY 283 

    0xba8921ed,// 287 PAY 284 

    0xbaa98d40,// 288 PAY 285 

    0x45852b3c,// 289 PAY 286 

    0xb458339e,// 290 PAY 287 

    0x7402a89b,// 291 PAY 288 

    0x0fc8b7b9,// 292 PAY 289 

    0x9563cf22,// 293 PAY 290 

    0x6f9515aa,// 294 PAY 291 

    0x6347ab22,// 295 PAY 292 

    0x48597a81,// 296 PAY 293 

    0x1afe32fa,// 297 PAY 294 

    0xc936d896,// 298 PAY 295 

    0xeff0e7eb,// 299 PAY 296 

    0x72f01129,// 300 PAY 297 

    0x9871c12e,// 301 PAY 298 

    0x2ba509b1,// 302 PAY 299 

    0x802a2e8c,// 303 PAY 300 

    0xf5242930,// 304 PAY 301 

    0xe90fe0fb,// 305 PAY 302 

    0x1859c032,// 306 PAY 303 

    0xd43689a8,// 307 PAY 304 

    0xdfa8e53b,// 308 PAY 305 

    0x800c125b,// 309 PAY 306 

    0xe5667342,// 310 PAY 307 

    0xb799418a,// 311 PAY 308 

    0xb9957a6b,// 312 PAY 309 

    0x9a72b624,// 313 PAY 310 

    0x6f666bea,// 314 PAY 311 

    0xba65e8e2,// 315 PAY 312 

    0xc18acdba,// 316 PAY 313 

    0xf782a9ec,// 317 PAY 314 

    0x375954c8,// 318 PAY 315 

    0x84bfd531,// 319 PAY 316 

    0xf18ef6e9,// 320 PAY 317 

    0xda409491,// 321 PAY 318 

    0x7ae37729,// 322 PAY 319 

    0x66bc938c,// 323 PAY 320 

    0x5b6093ed,// 324 PAY 321 

    0xc33e7bf8,// 325 PAY 322 

    0x96ade1ba,// 326 PAY 323 

    0xa0f80b54,// 327 PAY 324 

    0x432d722f,// 328 PAY 325 

    0x856150af,// 329 PAY 326 

    0x691bae3d,// 330 PAY 327 

    0x5c8a3027,// 331 PAY 328 

    0x1a2160c7,// 332 PAY 329 

    0x79629c4a,// 333 PAY 330 

    0x2630b4ee,// 334 PAY 331 

    0x59ea5802,// 335 PAY 332 

    0x2777741f,// 336 PAY 333 

    0x6d40c1eb,// 337 PAY 334 

    0xe74e2e8d,// 338 PAY 335 

    0xa26d3c52,// 339 PAY 336 

    0x134ffca1,// 340 PAY 337 

    0x53f28cfe,// 341 PAY 338 

    0x2da4883f,// 342 PAY 339 

    0x3b04f30d,// 343 PAY 340 

    0xe9795da3,// 344 PAY 341 

    0xe74ece40,// 345 PAY 342 

    0xf2f84320,// 346 PAY 343 

    0xb094f228,// 347 PAY 344 

    0xea0d2f17,// 348 PAY 345 

    0x7a022012,// 349 PAY 346 

    0x1bbb5a5e,// 350 PAY 347 

    0x6d4220ab,// 351 PAY 348 

    0x77def4f1,// 352 PAY 349 

    0x1cb07d47,// 353 PAY 350 

    0xc696b580,// 354 PAY 351 

    0xfc430b08,// 355 PAY 352 

    0x62e3ca6f,// 356 PAY 353 

    0xa2097045,// 357 PAY 354 

    0xfeb2a760,// 358 PAY 355 

    0xfbb756be,// 359 PAY 356 

    0xfae33897,// 360 PAY 357 

    0x0faab7c9,// 361 PAY 358 

    0x57162137,// 362 PAY 359 

    0x0d153e80,// 363 PAY 360 

    0x90800f38,// 364 PAY 361 

    0x390d96b3,// 365 PAY 362 

    0x3504a721,// 366 PAY 363 

    0x16a314db,// 367 PAY 364 

    0x72ee30c1,// 368 PAY 365 

    0x1efa258e,// 369 PAY 366 

    0x83f2866d,// 370 PAY 367 

    0xfa87565c,// 371 PAY 368 

    0xc487c1c4,// 372 PAY 369 

    0x8660ad5a,// 373 PAY 370 

    0x4e7142bc,// 374 PAY 371 

    0x8cce6553,// 375 PAY 372 

    0x5fe5c2d5,// 376 PAY 373 

    0xb9fcf362,// 377 PAY 374 

    0x216b21ac,// 378 PAY 375 

    0xb37b5dca,// 379 PAY 376 

    0xf4e5899e,// 380 PAY 377 

    0xe5df0611,// 381 PAY 378 

    0xa572a5b0,// 382 PAY 379 

    0x34dae4bf,// 383 PAY 380 

    0xf858201f,// 384 PAY 381 

    0xb68a6c6c,// 385 PAY 382 

    0x71969a27,// 386 PAY 383 

    0xa788b181,// 387 PAY 384 

    0x56113af0,// 388 PAY 385 

    0x7cf10abd,// 389 PAY 386 

    0x43e7a022,// 390 PAY 387 

    0x69f17c4b,// 391 PAY 388 

    0x6db384b3,// 392 PAY 389 

    0x82d10f89,// 393 PAY 390 

    0xfd8be6f6,// 394 PAY 391 

    0x2de789d6,// 395 PAY 392 

    0x4f36b83d,// 396 PAY 393 

    0xf089fa2e,// 397 PAY 394 

    0x65d8de8d,// 398 PAY 395 

    0xbc9b7456,// 399 PAY 396 

    0xff30a335,// 400 PAY 397 

    0xe4298cbe,// 401 PAY 398 

    0x2b20e12d,// 402 PAY 399 

    0xf77c5f87,// 403 PAY 400 

    0xf3710784,// 404 PAY 401 

    0x500ff1e7,// 405 PAY 402 

    0x9cb00780,// 406 PAY 403 

    0xe8d90349,// 407 PAY 404 

    0x38233271,// 408 PAY 405 

    0xa69b83b7,// 409 PAY 406 

    0x2739cf11,// 410 PAY 407 

    0xc9a761a2,// 411 PAY 408 

    0xb80a4b43,// 412 PAY 409 

    0x9d4e32f9,// 413 PAY 410 

    0xe4f2e60c,// 414 PAY 411 

    0x3e278796,// 415 PAY 412 

    0xe7fae5d0,// 416 PAY 413 

    0xed97913e,// 417 PAY 414 

    0xe731d9cc,// 418 PAY 415 

    0xd4018388,// 419 PAY 416 

    0xc9f2bfd7,// 420 PAY 417 

    0x4f57b51e,// 421 PAY 418 

    0x129121ff,// 422 PAY 419 

    0xbc19fe1e,// 423 PAY 420 

    0xe0574b7c,// 424 PAY 421 

    0x0c324723,// 425 PAY 422 

    0x996dcd7d,// 426 PAY 423 

    0xcfb79292,// 427 PAY 424 

    0x1f0f59ab,// 428 PAY 425 

    0x18f4af23,// 429 PAY 426 

    0xa8dc4db3,// 430 PAY 427 

    0x3a1fdd22,// 431 PAY 428 

    0xb024c81b,// 432 PAY 429 

    0x750a171e,// 433 PAY 430 

    0x10fe5789,// 434 PAY 431 

    0xad824b91,// 435 PAY 432 

    0x61191ecc,// 436 PAY 433 

    0x4537f07a,// 437 PAY 434 

    0x36d2698d,// 438 PAY 435 

    0xfe55d967,// 439 PAY 436 

    0x9241ec95,// 440 PAY 437 

    0x771c2557,// 441 PAY 438 

    0xf629d164,// 442 PAY 439 

    0x3b475265,// 443 PAY 440 

    0x2769f0df,// 444 PAY 441 

    0xf5c98551,// 445 PAY 442 

    0x73c9f553,// 446 PAY 443 

    0x43dee9e4,// 447 PAY 444 

    0xf042e1e7,// 448 PAY 445 

    0xe95fc30d,// 449 PAY 446 

    0x7ddc6470,// 450 PAY 447 

    0xd3255d75,// 451 PAY 448 

    0x348f669c,// 452 PAY 449 

    0x96ac7849,// 453 PAY 450 

    0xe45026a5,// 454 PAY 451 

/// STA is 1 words. 

/// STA num_pkts       : 215 

/// STA pkt_idx        : 91 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc4 

    0x016cc4d7 // 455 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt56_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 495 words. 

/// BDA size     is 1974 (0x7b6) 

/// BDA id       is 0x1b7b 

    0x07b61b7b,// 3 BDA   1 

/// PAY Generic Data size   : 1974 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x5bd72f2e,// 4 PAY   1 

    0x67a442a8,// 5 PAY   2 

    0xe45bcb45,// 6 PAY   3 

    0xc358803f,// 7 PAY   4 

    0xbecd2918,// 8 PAY   5 

    0x4ec008a7,// 9 PAY   6 

    0xc3a61394,// 10 PAY   7 

    0xc07027d6,// 11 PAY   8 

    0x5ff9baec,// 12 PAY   9 

    0x5444b7b8,// 13 PAY  10 

    0xc44b144d,// 14 PAY  11 

    0xeeeec9ab,// 15 PAY  12 

    0xad1addc8,// 16 PAY  13 

    0x0c08558e,// 17 PAY  14 

    0x482a4c95,// 18 PAY  15 

    0xb9d49955,// 19 PAY  16 

    0xfee2211b,// 20 PAY  17 

    0x96f7e716,// 21 PAY  18 

    0xa30c406d,// 22 PAY  19 

    0xd45c71fe,// 23 PAY  20 

    0xcadb8f09,// 24 PAY  21 

    0x6a6aecf3,// 25 PAY  22 

    0x7519bede,// 26 PAY  23 

    0x35db1f2d,// 27 PAY  24 

    0xb32bea1c,// 28 PAY  25 

    0xe155336d,// 29 PAY  26 

    0x50f188fe,// 30 PAY  27 

    0xeb5089d1,// 31 PAY  28 

    0xc1f63d6c,// 32 PAY  29 

    0x351a98a9,// 33 PAY  30 

    0x99d31461,// 34 PAY  31 

    0xc6bb7582,// 35 PAY  32 

    0xcedaea63,// 36 PAY  33 

    0xcdb7ad95,// 37 PAY  34 

    0xf36341aa,// 38 PAY  35 

    0xf41ec9bf,// 39 PAY  36 

    0xd8dfc88b,// 40 PAY  37 

    0xc6e8cee7,// 41 PAY  38 

    0xdfba769d,// 42 PAY  39 

    0x6cbdea5c,// 43 PAY  40 

    0x5b5b72fe,// 44 PAY  41 

    0xc43f518a,// 45 PAY  42 

    0x8858da43,// 46 PAY  43 

    0x31cafc03,// 47 PAY  44 

    0xae64daa7,// 48 PAY  45 

    0xa5e9d1d9,// 49 PAY  46 

    0xba699f4f,// 50 PAY  47 

    0x305df8f6,// 51 PAY  48 

    0x927f1e99,// 52 PAY  49 

    0xf2a38eca,// 53 PAY  50 

    0x576bfaaa,// 54 PAY  51 

    0x1d81ca55,// 55 PAY  52 

    0x0323fdfb,// 56 PAY  53 

    0x31f0eb54,// 57 PAY  54 

    0x41f760ae,// 58 PAY  55 

    0x6fa731b2,// 59 PAY  56 

    0x8529bb1e,// 60 PAY  57 

    0x47160023,// 61 PAY  58 

    0x828f48de,// 62 PAY  59 

    0x3058d39c,// 63 PAY  60 

    0x0a19c99e,// 64 PAY  61 

    0x1545bfcc,// 65 PAY  62 

    0xc9cbb73d,// 66 PAY  63 

    0xf53eeaae,// 67 PAY  64 

    0x28dde8cc,// 68 PAY  65 

    0xf430e1f2,// 69 PAY  66 

    0xc6db1d86,// 70 PAY  67 

    0x024426b9,// 71 PAY  68 

    0x050fb297,// 72 PAY  69 

    0x555942df,// 73 PAY  70 

    0x5e3840ec,// 74 PAY  71 

    0x9183336e,// 75 PAY  72 

    0x14ef95b0,// 76 PAY  73 

    0x8b1b50da,// 77 PAY  74 

    0x2ca7f920,// 78 PAY  75 

    0x5e13fbab,// 79 PAY  76 

    0x703ddc6e,// 80 PAY  77 

    0x861fb14e,// 81 PAY  78 

    0x4bd0626c,// 82 PAY  79 

    0x7b9e2dda,// 83 PAY  80 

    0xe152bbce,// 84 PAY  81 

    0xc0d26413,// 85 PAY  82 

    0xc8877f47,// 86 PAY  83 

    0x11a1c308,// 87 PAY  84 

    0x35f6b4bc,// 88 PAY  85 

    0x3c1dd95e,// 89 PAY  86 

    0x00a281eb,// 90 PAY  87 

    0xf86d069e,// 91 PAY  88 

    0x15c0217c,// 92 PAY  89 

    0x42f34c5b,// 93 PAY  90 

    0xfe480bef,// 94 PAY  91 

    0xba4b3af3,// 95 PAY  92 

    0xe4735943,// 96 PAY  93 

    0x08e29004,// 97 PAY  94 

    0x73b3e91b,// 98 PAY  95 

    0x5bea4234,// 99 PAY  96 

    0x7c52b4c4,// 100 PAY  97 

    0x5835966a,// 101 PAY  98 

    0xbc2c5c08,// 102 PAY  99 

    0xf1054306,// 103 PAY 100 

    0xf154e9df,// 104 PAY 101 

    0x75c36da1,// 105 PAY 102 

    0xd4d98458,// 106 PAY 103 

    0xb25f32f9,// 107 PAY 104 

    0xcd300a82,// 108 PAY 105 

    0xe58222e7,// 109 PAY 106 

    0xc8f8fd38,// 110 PAY 107 

    0x8eef51f3,// 111 PAY 108 

    0x9408dec8,// 112 PAY 109 

    0x021c9a29,// 113 PAY 110 

    0x9e96115b,// 114 PAY 111 

    0x430e1fe2,// 115 PAY 112 

    0x0ad1dfbd,// 116 PAY 113 

    0xb0bd6eb4,// 117 PAY 114 

    0x4224dc8c,// 118 PAY 115 

    0xcfbd9185,// 119 PAY 116 

    0xca277e7c,// 120 PAY 117 

    0x5ba8c125,// 121 PAY 118 

    0xf71be716,// 122 PAY 119 

    0xd0c6df63,// 123 PAY 120 

    0x88e25534,// 124 PAY 121 

    0x2274d5b9,// 125 PAY 122 

    0x2cd5300e,// 126 PAY 123 

    0xc9b1a256,// 127 PAY 124 

    0x117150bc,// 128 PAY 125 

    0x0f6043b6,// 129 PAY 126 

    0x1af6e51b,// 130 PAY 127 

    0x312e7798,// 131 PAY 128 

    0xc0fae3c4,// 132 PAY 129 

    0x72f07f65,// 133 PAY 130 

    0x7277c874,// 134 PAY 131 

    0x7411d034,// 135 PAY 132 

    0xa01d056c,// 136 PAY 133 

    0xfa7b03f1,// 137 PAY 134 

    0x976b4f79,// 138 PAY 135 

    0x74c3bef1,// 139 PAY 136 

    0x476f65b8,// 140 PAY 137 

    0x22672bba,// 141 PAY 138 

    0x30d41b6c,// 142 PAY 139 

    0x0f5e67df,// 143 PAY 140 

    0x59a85e4c,// 144 PAY 141 

    0x3bcf1e8c,// 145 PAY 142 

    0x8cd5963f,// 146 PAY 143 

    0x22c84250,// 147 PAY 144 

    0x665def0a,// 148 PAY 145 

    0x1f3255c4,// 149 PAY 146 

    0x8ca46ce3,// 150 PAY 147 

    0x178f7830,// 151 PAY 148 

    0xbd9d28f1,// 152 PAY 149 

    0xe6eb7f7f,// 153 PAY 150 

    0x6c1d30df,// 154 PAY 151 

    0xcd766f75,// 155 PAY 152 

    0xbdb6aed1,// 156 PAY 153 

    0x56f4e868,// 157 PAY 154 

    0x71e837ee,// 158 PAY 155 

    0xa4d7a01b,// 159 PAY 156 

    0xb9aea520,// 160 PAY 157 

    0x0f7cbf1d,// 161 PAY 158 

    0xf823ed02,// 162 PAY 159 

    0xfb316ec9,// 163 PAY 160 

    0xe657dbe1,// 164 PAY 161 

    0x14ef46b0,// 165 PAY 162 

    0x726ebb79,// 166 PAY 163 

    0x6481234c,// 167 PAY 164 

    0x81fb295e,// 168 PAY 165 

    0xb60d212f,// 169 PAY 166 

    0x70f5066e,// 170 PAY 167 

    0x35dd8a09,// 171 PAY 168 

    0x355a1659,// 172 PAY 169 

    0x203ce37a,// 173 PAY 170 

    0x15836020,// 174 PAY 171 

    0xdbc81fd3,// 175 PAY 172 

    0x70c6b5fc,// 176 PAY 173 

    0xa16e15a8,// 177 PAY 174 

    0x53e39f71,// 178 PAY 175 

    0x001e4a41,// 179 PAY 176 

    0x55fa11d5,// 180 PAY 177 

    0x3c6e401c,// 181 PAY 178 

    0xae5a1121,// 182 PAY 179 

    0xc16a3dbb,// 183 PAY 180 

    0x8386fc9a,// 184 PAY 181 

    0x91f4b736,// 185 PAY 182 

    0x0a7fd7d9,// 186 PAY 183 

    0x5013da5d,// 187 PAY 184 

    0x5261fc78,// 188 PAY 185 

    0x16d9bb0f,// 189 PAY 186 

    0xe9474ee3,// 190 PAY 187 

    0x1ec9dea4,// 191 PAY 188 

    0x7af6357b,// 192 PAY 189 

    0x068515ce,// 193 PAY 190 

    0x4088d759,// 194 PAY 191 

    0x4ac40012,// 195 PAY 192 

    0xa78f4858,// 196 PAY 193 

    0xca8384ed,// 197 PAY 194 

    0x79984fb4,// 198 PAY 195 

    0x6d54f989,// 199 PAY 196 

    0xc0fe3683,// 200 PAY 197 

    0x29f5e04f,// 201 PAY 198 

    0x6cf447fb,// 202 PAY 199 

    0x6975d786,// 203 PAY 200 

    0x1c290b94,// 204 PAY 201 

    0x297dd8da,// 205 PAY 202 

    0xfac5990a,// 206 PAY 203 

    0x3f756ff8,// 207 PAY 204 

    0xc3873c79,// 208 PAY 205 

    0xc0fbd5f0,// 209 PAY 206 

    0xb1844ebe,// 210 PAY 207 

    0xc9141946,// 211 PAY 208 

    0x2c9102be,// 212 PAY 209 

    0x58f4c32f,// 213 PAY 210 

    0x94a1e025,// 214 PAY 211 

    0xfe45ab01,// 215 PAY 212 

    0x7b49d3c0,// 216 PAY 213 

    0xffca9fa5,// 217 PAY 214 

    0xf7bb90d1,// 218 PAY 215 

    0x161887a1,// 219 PAY 216 

    0xf367606d,// 220 PAY 217 

    0x1effb875,// 221 PAY 218 

    0x05ccd0db,// 222 PAY 219 

    0x7bf0ba83,// 223 PAY 220 

    0x3ec09275,// 224 PAY 221 

    0xe7e03c9a,// 225 PAY 222 

    0x9b8e7064,// 226 PAY 223 

    0xfe6819e8,// 227 PAY 224 

    0xd1075e50,// 228 PAY 225 

    0xb92495bf,// 229 PAY 226 

    0x52944350,// 230 PAY 227 

    0x866e2028,// 231 PAY 228 

    0x4c0f43af,// 232 PAY 229 

    0x885403b6,// 233 PAY 230 

    0x5ff466ec,// 234 PAY 231 

    0x5019c958,// 235 PAY 232 

    0x7398c009,// 236 PAY 233 

    0x360b1941,// 237 PAY 234 

    0xf74fc073,// 238 PAY 235 

    0x7d0c854e,// 239 PAY 236 

    0x9b3fc06b,// 240 PAY 237 

    0x69eaaa9a,// 241 PAY 238 

    0x00b68050,// 242 PAY 239 

    0x3ea56df4,// 243 PAY 240 

    0x674635b9,// 244 PAY 241 

    0x6e5b30ff,// 245 PAY 242 

    0x34c60f3b,// 246 PAY 243 

    0x50456d6b,// 247 PAY 244 

    0x50bd23dc,// 248 PAY 245 

    0x3783ce5e,// 249 PAY 246 

    0x451b9cc8,// 250 PAY 247 

    0xb8bfa651,// 251 PAY 248 

    0xbad82c3c,// 252 PAY 249 

    0xa206fbed,// 253 PAY 250 

    0x7fac0d71,// 254 PAY 251 

    0x03de7e99,// 255 PAY 252 

    0x38934621,// 256 PAY 253 

    0x942612a0,// 257 PAY 254 

    0x90eac12f,// 258 PAY 255 

    0xc51bc559,// 259 PAY 256 

    0xb0831df3,// 260 PAY 257 

    0xd40c029d,// 261 PAY 258 

    0xbc5dbb85,// 262 PAY 259 

    0x6fe37eb0,// 263 PAY 260 

    0x170ed068,// 264 PAY 261 

    0xa1201802,// 265 PAY 262 

    0x1e624565,// 266 PAY 263 

    0xfd801f4a,// 267 PAY 264 

    0x07d31050,// 268 PAY 265 

    0xca8dcf06,// 269 PAY 266 

    0x7e63eca9,// 270 PAY 267 

    0x2c0c8ec5,// 271 PAY 268 

    0xc6955c9d,// 272 PAY 269 

    0x1d50df45,// 273 PAY 270 

    0x10311865,// 274 PAY 271 

    0x5e54fc53,// 275 PAY 272 

    0x628a60a7,// 276 PAY 273 

    0x568f5cfe,// 277 PAY 274 

    0x7b21a085,// 278 PAY 275 

    0x6c6c4316,// 279 PAY 276 

    0x76f3c992,// 280 PAY 277 

    0x38a94882,// 281 PAY 278 

    0xd58acc20,// 282 PAY 279 

    0x9e0c73f6,// 283 PAY 280 

    0xffb87781,// 284 PAY 281 

    0xc62643fe,// 285 PAY 282 

    0xb6b51d1e,// 286 PAY 283 

    0xccfa2bec,// 287 PAY 284 

    0x9ea60714,// 288 PAY 285 

    0x1d44c0c1,// 289 PAY 286 

    0x885bf5a1,// 290 PAY 287 

    0xc75e81d5,// 291 PAY 288 

    0x1b252716,// 292 PAY 289 

    0x85e758c5,// 293 PAY 290 

    0x5e90a664,// 294 PAY 291 

    0x27e0fd65,// 295 PAY 292 

    0x7feeb9c2,// 296 PAY 293 

    0x7f938336,// 297 PAY 294 

    0xe7f76ace,// 298 PAY 295 

    0xd08d82ce,// 299 PAY 296 

    0xa5a134b0,// 300 PAY 297 

    0x05068e6b,// 301 PAY 298 

    0x39adcb7c,// 302 PAY 299 

    0x32262340,// 303 PAY 300 

    0x100571b4,// 304 PAY 301 

    0x1abacd79,// 305 PAY 302 

    0xd5ff1ccc,// 306 PAY 303 

    0x4c93ee72,// 307 PAY 304 

    0x88b061ee,// 308 PAY 305 

    0x7c382da9,// 309 PAY 306 

    0x4756b4ec,// 310 PAY 307 

    0xdc381908,// 311 PAY 308 

    0xf7fa1229,// 312 PAY 309 

    0x5489b6c5,// 313 PAY 310 

    0x78b48b48,// 314 PAY 311 

    0x96b585fe,// 315 PAY 312 

    0xc6c8c6c7,// 316 PAY 313 

    0x11f8f455,// 317 PAY 314 

    0x7de1fa17,// 318 PAY 315 

    0x544e2fc8,// 319 PAY 316 

    0x37193d94,// 320 PAY 317 

    0x80de6d7f,// 321 PAY 318 

    0xfec8586a,// 322 PAY 319 

    0x57bbcc20,// 323 PAY 320 

    0x8ca887bc,// 324 PAY 321 

    0xdcda88c9,// 325 PAY 322 

    0x4c2fefbc,// 326 PAY 323 

    0x7302814a,// 327 PAY 324 

    0x4e8b0b40,// 328 PAY 325 

    0x7c8398b4,// 329 PAY 326 

    0xefb02862,// 330 PAY 327 

    0xf755eff8,// 331 PAY 328 

    0x3a4f6bf8,// 332 PAY 329 

    0xf164c0c8,// 333 PAY 330 

    0xd1a1b097,// 334 PAY 331 

    0x9e19820a,// 335 PAY 332 

    0x61058c53,// 336 PAY 333 

    0x88e41bdb,// 337 PAY 334 

    0xdb47b4cc,// 338 PAY 335 

    0x7483f316,// 339 PAY 336 

    0xaad7eea3,// 340 PAY 337 

    0x18695c8c,// 341 PAY 338 

    0x441a09af,// 342 PAY 339 

    0x368242e2,// 343 PAY 340 

    0x423b4013,// 344 PAY 341 

    0x4e83323c,// 345 PAY 342 

    0x8b40b603,// 346 PAY 343 

    0x36a32f03,// 347 PAY 344 

    0x9b76a8d3,// 348 PAY 345 

    0x84aa6390,// 349 PAY 346 

    0x277e445a,// 350 PAY 347 

    0x3f73d15b,// 351 PAY 348 

    0xf9338181,// 352 PAY 349 

    0x3597c3b8,// 353 PAY 350 

    0x7869be53,// 354 PAY 351 

    0x34e6d4cd,// 355 PAY 352 

    0xb0657544,// 356 PAY 353 

    0x7d9643d7,// 357 PAY 354 

    0x370bea3d,// 358 PAY 355 

    0x28f08cc8,// 359 PAY 356 

    0x94ae5008,// 360 PAY 357 

    0xdf6d3562,// 361 PAY 358 

    0x8e2db270,// 362 PAY 359 

    0x23777097,// 363 PAY 360 

    0x1b2e68f9,// 364 PAY 361 

    0xc4adb49a,// 365 PAY 362 

    0xacafa10a,// 366 PAY 363 

    0x093d28c0,// 367 PAY 364 

    0xa02dfe4b,// 368 PAY 365 

    0xe788c94d,// 369 PAY 366 

    0xd163b390,// 370 PAY 367 

    0xd28614f6,// 371 PAY 368 

    0xd3830c55,// 372 PAY 369 

    0xda472386,// 373 PAY 370 

    0x04059e6a,// 374 PAY 371 

    0xbf243fcf,// 375 PAY 372 

    0x07756414,// 376 PAY 373 

    0x0ba6e45b,// 377 PAY 374 

    0xc3930834,// 378 PAY 375 

    0xb8f36edf,// 379 PAY 376 

    0xf6c32dbe,// 380 PAY 377 

    0x56923e94,// 381 PAY 378 

    0x94d8d752,// 382 PAY 379 

    0x2070d46e,// 383 PAY 380 

    0xb5973427,// 384 PAY 381 

    0x2e0b770f,// 385 PAY 382 

    0xb12ff02f,// 386 PAY 383 

    0x93f14274,// 387 PAY 384 

    0x84f758a5,// 388 PAY 385 

    0xcefecb00,// 389 PAY 386 

    0x87b4e82d,// 390 PAY 387 

    0x1b005428,// 391 PAY 388 

    0x1ce73297,// 392 PAY 389 

    0x8efa7380,// 393 PAY 390 

    0x49748b50,// 394 PAY 391 

    0xfe78c01a,// 395 PAY 392 

    0xde222d0d,// 396 PAY 393 

    0x4dc44357,// 397 PAY 394 

    0xd7b80684,// 398 PAY 395 

    0x0cb1820f,// 399 PAY 396 

    0xd654cfdc,// 400 PAY 397 

    0x3d6fe4e1,// 401 PAY 398 

    0x3e313311,// 402 PAY 399 

    0xba56a4f8,// 403 PAY 400 

    0x972cd991,// 404 PAY 401 

    0x032d46dc,// 405 PAY 402 

    0x2611ac83,// 406 PAY 403 

    0x6d6d8dbf,// 407 PAY 404 

    0x3a9bfbc6,// 408 PAY 405 

    0x4862dc81,// 409 PAY 406 

    0x8f0bc642,// 410 PAY 407 

    0x998c94e5,// 411 PAY 408 

    0xe0c9e9b5,// 412 PAY 409 

    0x48eaf55c,// 413 PAY 410 

    0xc878b645,// 414 PAY 411 

    0x621bac64,// 415 PAY 412 

    0x70d85b30,// 416 PAY 413 

    0xf1e19aa5,// 417 PAY 414 

    0x8aacf431,// 418 PAY 415 

    0xc3f236f9,// 419 PAY 416 

    0xa1cee21e,// 420 PAY 417 

    0x9aeb0b4a,// 421 PAY 418 

    0x77e8109a,// 422 PAY 419 

    0x21e8bd52,// 423 PAY 420 

    0x6d4269cb,// 424 PAY 421 

    0xc5b4d05c,// 425 PAY 422 

    0x11989aa3,// 426 PAY 423 

    0x231aa71a,// 427 PAY 424 

    0xaa991ddc,// 428 PAY 425 

    0x5d858126,// 429 PAY 426 

    0x0f5e3184,// 430 PAY 427 

    0x0794e9aa,// 431 PAY 428 

    0xb2b13309,// 432 PAY 429 

    0xa363017d,// 433 PAY 430 

    0x130ccf41,// 434 PAY 431 

    0x632e0a70,// 435 PAY 432 

    0xbcbc45f2,// 436 PAY 433 

    0xb58fd149,// 437 PAY 434 

    0x5e01a9bc,// 438 PAY 435 

    0x940206de,// 439 PAY 436 

    0xdedd9f6d,// 440 PAY 437 

    0x9ec50555,// 441 PAY 438 

    0x06d3c20a,// 442 PAY 439 

    0x8469d27c,// 443 PAY 440 

    0xc7226afa,// 444 PAY 441 

    0x1149057b,// 445 PAY 442 

    0xd8d209f6,// 446 PAY 443 

    0xb4b46a2d,// 447 PAY 444 

    0xd9276abe,// 448 PAY 445 

    0xc49aa332,// 449 PAY 446 

    0x67990080,// 450 PAY 447 

    0xcef122f1,// 451 PAY 448 

    0x3cc15c7d,// 452 PAY 449 

    0x2b8af73c,// 453 PAY 450 

    0xc4f23457,// 454 PAY 451 

    0x199cdc6b,// 455 PAY 452 

    0xfebbab39,// 456 PAY 453 

    0x186042be,// 457 PAY 454 

    0x2166dc88,// 458 PAY 455 

    0xcec33723,// 459 PAY 456 

    0xa952dc42,// 460 PAY 457 

    0x9070acf3,// 461 PAY 458 

    0x6abc3d27,// 462 PAY 459 

    0xaa1ed7df,// 463 PAY 460 

    0x429d2869,// 464 PAY 461 

    0xff64792b,// 465 PAY 462 

    0xfe2dd280,// 466 PAY 463 

    0x808942ae,// 467 PAY 464 

    0xcc3b6660,// 468 PAY 465 

    0xb86ad204,// 469 PAY 466 

    0x261904ed,// 470 PAY 467 

    0x9ab102b9,// 471 PAY 468 

    0xdb1955c2,// 472 PAY 469 

    0xda281ccf,// 473 PAY 470 

    0xa485234c,// 474 PAY 471 

    0xa3ea2c8d,// 475 PAY 472 

    0xddba337c,// 476 PAY 473 

    0x40cc3b25,// 477 PAY 474 

    0xe59d8367,// 478 PAY 475 

    0x710dfe74,// 479 PAY 476 

    0x8e3a46f1,// 480 PAY 477 

    0xc798ed65,// 481 PAY 478 

    0x79e48fe0,// 482 PAY 479 

    0xe8010eef,// 483 PAY 480 

    0x1c2b3fc1,// 484 PAY 481 

    0x2a3945f9,// 485 PAY 482 

    0xee356bb6,// 486 PAY 483 

    0x4051512f,// 487 PAY 484 

    0x12265ef7,// 488 PAY 485 

    0xae595a9c,// 489 PAY 486 

    0xe11b9264,// 490 PAY 487 

    0x094dd352,// 491 PAY 488 

    0x301c1886,// 492 PAY 489 

    0x71144eee,// 493 PAY 490 

    0x764333de,// 494 PAY 491 

    0xc8f7bb7d,// 495 PAY 492 

    0xe289ec2a,// 496 PAY 493 

    0xcd490000,// 497 PAY 494 

/// STA is 1 words. 

/// STA num_pkts       : 160 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf7 

    0x02d1f7a0 // 498 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt57_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 31 words. 

/// BDA size     is 117 (0x75) 

/// BDA id       is 0xf427 

    0x0075f427,// 3 BDA   1 

/// PAY Generic Data size   : 117 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x8934aa3b,// 4 PAY   1 

    0xd1d61ebf,// 5 PAY   2 

    0xaf8cbe80,// 6 PAY   3 

    0x96ab63e8,// 7 PAY   4 

    0x46007e5d,// 8 PAY   5 

    0x3e3de77c,// 9 PAY   6 

    0x4003fb42,// 10 PAY   7 

    0x1fd87df6,// 11 PAY   8 

    0xc68217de,// 12 PAY   9 

    0x5fba3bfe,// 13 PAY  10 

    0x315a3ed5,// 14 PAY  11 

    0x3bbfa372,// 15 PAY  12 

    0x24bc6799,// 16 PAY  13 

    0xe762317a,// 17 PAY  14 

    0x4c3bb256,// 18 PAY  15 

    0xd29830bd,// 19 PAY  16 

    0x039ef5f8,// 20 PAY  17 

    0x1bb3ddf4,// 21 PAY  18 

    0xc8f9b61b,// 22 PAY  19 

    0x36a53fae,// 23 PAY  20 

    0xfa4e08a9,// 24 PAY  21 

    0x0c2b182c,// 25 PAY  22 

    0x5ad290b0,// 26 PAY  23 

    0xf22989bd,// 27 PAY  24 

    0xd0efa55c,// 28 PAY  25 

    0x077cac75,// 29 PAY  26 

    0x2890f826,// 30 PAY  27 

    0x6fd888cf,// 31 PAY  28 

    0xb2981635,// 32 PAY  29 

    0x63000000,// 33 PAY  30 

/// HASH is  16 bytes 

    0xc8f9b61b,// 34 HSH   1 

    0x36a53fae,// 35 HSH   2 

    0xfa4e08a9,// 36 HSH   3 

    0x0c2b182c,// 37 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 231 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9 

    0x03dc09e7 // 38 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt58_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 296 words. 

/// BDA size     is 1179 (0x49b) 

/// BDA id       is 0x5042 

    0x049b5042,// 3 BDA   1 

/// PAY Generic Data size   : 1179 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x2e16bc41,// 4 PAY   1 

    0xfbe99a09,// 5 PAY   2 

    0x2e91c8ad,// 6 PAY   3 

    0xcfd079d7,// 7 PAY   4 

    0x77e29276,// 8 PAY   5 

    0x7d336873,// 9 PAY   6 

    0x34602e75,// 10 PAY   7 

    0x8770d505,// 11 PAY   8 

    0x9bb96f8d,// 12 PAY   9 

    0x37bf307f,// 13 PAY  10 

    0xe42d1f41,// 14 PAY  11 

    0xba821e17,// 15 PAY  12 

    0xbd245575,// 16 PAY  13 

    0xa6c10db5,// 17 PAY  14 

    0x2de6e446,// 18 PAY  15 

    0xb5a02b80,// 19 PAY  16 

    0x70018e44,// 20 PAY  17 

    0x93a8194c,// 21 PAY  18 

    0x60fc5ed5,// 22 PAY  19 

    0xcea40a16,// 23 PAY  20 

    0x0f1e1d3c,// 24 PAY  21 

    0xae1ef164,// 25 PAY  22 

    0x63c7d7d1,// 26 PAY  23 

    0x3edd9313,// 27 PAY  24 

    0xe379355c,// 28 PAY  25 

    0xe742d5a4,// 29 PAY  26 

    0x645a792f,// 30 PAY  27 

    0xedc972df,// 31 PAY  28 

    0xcbe27842,// 32 PAY  29 

    0x2193a206,// 33 PAY  30 

    0xb1a270f7,// 34 PAY  31 

    0xc5413a7d,// 35 PAY  32 

    0x0784fd6c,// 36 PAY  33 

    0x39d336fb,// 37 PAY  34 

    0xc74a0f41,// 38 PAY  35 

    0x8b6594fe,// 39 PAY  36 

    0x66dcd55b,// 40 PAY  37 

    0xb989f1fc,// 41 PAY  38 

    0xe988e86c,// 42 PAY  39 

    0xe2ae6108,// 43 PAY  40 

    0xba85c8b1,// 44 PAY  41 

    0x3518d7db,// 45 PAY  42 

    0xe979a42e,// 46 PAY  43 

    0xc3ff24b5,// 47 PAY  44 

    0x35fdd523,// 48 PAY  45 

    0x6eab508c,// 49 PAY  46 

    0x1e9f3e99,// 50 PAY  47 

    0x3f8eab1e,// 51 PAY  48 

    0x32c3e8f3,// 52 PAY  49 

    0x7b5cd77e,// 53 PAY  50 

    0xd1878c7f,// 54 PAY  51 

    0x582d03e0,// 55 PAY  52 

    0x8389f62f,// 56 PAY  53 

    0xdbc07442,// 57 PAY  54 

    0x82079a64,// 58 PAY  55 

    0x9954c8e3,// 59 PAY  56 

    0x84a758c4,// 60 PAY  57 

    0x23564c9b,// 61 PAY  58 

    0x4715233a,// 62 PAY  59 

    0xbd30c208,// 63 PAY  60 

    0x0bd5a271,// 64 PAY  61 

    0xa2d6c257,// 65 PAY  62 

    0x9e07f740,// 66 PAY  63 

    0xcfaacf09,// 67 PAY  64 

    0x13b537dc,// 68 PAY  65 

    0x6cb86579,// 69 PAY  66 

    0x912af7dc,// 70 PAY  67 

    0xee671e50,// 71 PAY  68 

    0x516a8009,// 72 PAY  69 

    0x06c6cc23,// 73 PAY  70 

    0x819d4e59,// 74 PAY  71 

    0xf56ee244,// 75 PAY  72 

    0xad85a3d0,// 76 PAY  73 

    0xf89d49a2,// 77 PAY  74 

    0xc7b79690,// 78 PAY  75 

    0xf4d0c7e9,// 79 PAY  76 

    0xa513a7cb,// 80 PAY  77 

    0x0fef36e7,// 81 PAY  78 

    0xbbe7f192,// 82 PAY  79 

    0x5dd0142b,// 83 PAY  80 

    0xea7d8736,// 84 PAY  81 

    0x164c449f,// 85 PAY  82 

    0xef64360e,// 86 PAY  83 

    0x146ea541,// 87 PAY  84 

    0x3d5f8add,// 88 PAY  85 

    0xf925f206,// 89 PAY  86 

    0x74b9d0b0,// 90 PAY  87 

    0x36f26f72,// 91 PAY  88 

    0xceaebff5,// 92 PAY  89 

    0xf3dc85e3,// 93 PAY  90 

    0xeb941a7d,// 94 PAY  91 

    0x78d93de5,// 95 PAY  92 

    0xe4e6bcab,// 96 PAY  93 

    0x1160e408,// 97 PAY  94 

    0xb624948f,// 98 PAY  95 

    0xde6f2fe4,// 99 PAY  96 

    0xf09bccac,// 100 PAY  97 

    0x859d3277,// 101 PAY  98 

    0x11251992,// 102 PAY  99 

    0xb7278bf4,// 103 PAY 100 

    0x5e795bd0,// 104 PAY 101 

    0x8100ef6e,// 105 PAY 102 

    0xa3ceaccb,// 106 PAY 103 

    0xe0b8451a,// 107 PAY 104 

    0x88acedde,// 108 PAY 105 

    0xfe37c612,// 109 PAY 106 

    0x928b53a9,// 110 PAY 107 

    0x339f22ec,// 111 PAY 108 

    0x0ee37281,// 112 PAY 109 

    0xa1dd43e2,// 113 PAY 110 

    0xf2e43e7b,// 114 PAY 111 

    0xda2e75cc,// 115 PAY 112 

    0x6ae9371e,// 116 PAY 113 

    0x1dfa0f88,// 117 PAY 114 

    0xc747b41b,// 118 PAY 115 

    0x50cb46cb,// 119 PAY 116 

    0x93c1d803,// 120 PAY 117 

    0x4eaeab18,// 121 PAY 118 

    0x6cb0e54a,// 122 PAY 119 

    0xf2449d7b,// 123 PAY 120 

    0x32a37488,// 124 PAY 121 

    0xe1be2eed,// 125 PAY 122 

    0x55be92bf,// 126 PAY 123 

    0x6d26b208,// 127 PAY 124 

    0x2c73ad8d,// 128 PAY 125 

    0x914cc624,// 129 PAY 126 

    0x9964f42d,// 130 PAY 127 

    0x7cd441e5,// 131 PAY 128 

    0x78a82683,// 132 PAY 129 

    0xb170765e,// 133 PAY 130 

    0xa4cc694c,// 134 PAY 131 

    0x40322134,// 135 PAY 132 

    0xbc52fbce,// 136 PAY 133 

    0x2097bbcd,// 137 PAY 134 

    0x05829cd1,// 138 PAY 135 

    0xef8d3d69,// 139 PAY 136 

    0x38d6175b,// 140 PAY 137 

    0x9ef86122,// 141 PAY 138 

    0x4449e484,// 142 PAY 139 

    0x0563b74f,// 143 PAY 140 

    0xa847150c,// 144 PAY 141 

    0xf0cc4df0,// 145 PAY 142 

    0x62d9a1ae,// 146 PAY 143 

    0xd6c9577e,// 147 PAY 144 

    0x220c924f,// 148 PAY 145 

    0x6e4d1e2b,// 149 PAY 146 

    0x5234528f,// 150 PAY 147 

    0x5d4d4525,// 151 PAY 148 

    0x952377ae,// 152 PAY 149 

    0x0b877731,// 153 PAY 150 

    0xf165d633,// 154 PAY 151 

    0x7afceb4f,// 155 PAY 152 

    0x2e881570,// 156 PAY 153 

    0x861d2630,// 157 PAY 154 

    0xfcac507f,// 158 PAY 155 

    0x3bf6577c,// 159 PAY 156 

    0x1bfa71cf,// 160 PAY 157 

    0xd2b6862e,// 161 PAY 158 

    0x5e11c2ab,// 162 PAY 159 

    0xe5ed1b88,// 163 PAY 160 

    0x494bd55c,// 164 PAY 161 

    0x9cd9b181,// 165 PAY 162 

    0x24f97611,// 166 PAY 163 

    0x1cdcaed7,// 167 PAY 164 

    0xda405a16,// 168 PAY 165 

    0x90abdecc,// 169 PAY 166 

    0xa368c666,// 170 PAY 167 

    0x92caddb4,// 171 PAY 168 

    0xc588fef7,// 172 PAY 169 

    0xaef0508b,// 173 PAY 170 

    0xd0c1c6b0,// 174 PAY 171 

    0xdaeb7337,// 175 PAY 172 

    0xf3ae41ff,// 176 PAY 173 

    0x39c8c638,// 177 PAY 174 

    0x52fbfb74,// 178 PAY 175 

    0x5a5c8e1b,// 179 PAY 176 

    0x5f42bbf0,// 180 PAY 177 

    0x1b1560b6,// 181 PAY 178 

    0xa450f3d1,// 182 PAY 179 

    0xe1731cda,// 183 PAY 180 

    0xc02f6c35,// 184 PAY 181 

    0xbd4e6c33,// 185 PAY 182 

    0x23868ca1,// 186 PAY 183 

    0xef2622bc,// 187 PAY 184 

    0x05f73fe4,// 188 PAY 185 

    0x53dfbd6e,// 189 PAY 186 

    0x4dae091b,// 190 PAY 187 

    0x133bbf22,// 191 PAY 188 

    0x7928748c,// 192 PAY 189 

    0x64d09b75,// 193 PAY 190 

    0xf0372281,// 194 PAY 191 

    0x5c00a13c,// 195 PAY 192 

    0xafd76a3b,// 196 PAY 193 

    0x4cb152b1,// 197 PAY 194 

    0x1aee81bc,// 198 PAY 195 

    0xae464700,// 199 PAY 196 

    0xce169f7f,// 200 PAY 197 

    0xca15fa4e,// 201 PAY 198 

    0x08094514,// 202 PAY 199 

    0xbefdfcb9,// 203 PAY 200 

    0xd3a0a132,// 204 PAY 201 

    0x0f5ff80d,// 205 PAY 202 

    0x7eba0a9f,// 206 PAY 203 

    0x4debb848,// 207 PAY 204 

    0x38c180dd,// 208 PAY 205 

    0xcbe5f48a,// 209 PAY 206 

    0x788b70c3,// 210 PAY 207 

    0x518b2b4b,// 211 PAY 208 

    0xb345232f,// 212 PAY 209 

    0x74fb1a17,// 213 PAY 210 

    0xd7078667,// 214 PAY 211 

    0x8cb4a22b,// 215 PAY 212 

    0xac5b0937,// 216 PAY 213 

    0xdb8e9d08,// 217 PAY 214 

    0x854545fa,// 218 PAY 215 

    0x0e3c100f,// 219 PAY 216 

    0x4c0a2689,// 220 PAY 217 

    0xf9e750e9,// 221 PAY 218 

    0x123edbad,// 222 PAY 219 

    0x68f6888b,// 223 PAY 220 

    0xb3eb7256,// 224 PAY 221 

    0xd5f9302f,// 225 PAY 222 

    0x100f8732,// 226 PAY 223 

    0x70caa38c,// 227 PAY 224 

    0x9a908355,// 228 PAY 225 

    0x9bbb1817,// 229 PAY 226 

    0x3e1c3377,// 230 PAY 227 

    0x8e49d671,// 231 PAY 228 

    0xe35a1fec,// 232 PAY 229 

    0x233f4c8e,// 233 PAY 230 

    0x63bee6fd,// 234 PAY 231 

    0x99ead7aa,// 235 PAY 232 

    0xb987615b,// 236 PAY 233 

    0x83e06833,// 237 PAY 234 

    0x25b4aefa,// 238 PAY 235 

    0x852d22ae,// 239 PAY 236 

    0x8a6eaf80,// 240 PAY 237 

    0xac7e6528,// 241 PAY 238 

    0x5db51095,// 242 PAY 239 

    0x82794e23,// 243 PAY 240 

    0x7344f0a7,// 244 PAY 241 

    0x0c88db72,// 245 PAY 242 

    0x73c2b790,// 246 PAY 243 

    0x64ffd0c8,// 247 PAY 244 

    0x620d591d,// 248 PAY 245 

    0x3e8c7cdf,// 249 PAY 246 

    0x12369bac,// 250 PAY 247 

    0xb4762d4e,// 251 PAY 248 

    0x47010767,// 252 PAY 249 

    0xfe6d5f1c,// 253 PAY 250 

    0xc21c05c7,// 254 PAY 251 

    0x3501223b,// 255 PAY 252 

    0xa027f453,// 256 PAY 253 

    0x8a79e95b,// 257 PAY 254 

    0xa09bbb46,// 258 PAY 255 

    0x732d382e,// 259 PAY 256 

    0x7b952202,// 260 PAY 257 

    0x976230c3,// 261 PAY 258 

    0xdb189bb6,// 262 PAY 259 

    0xdd211d83,// 263 PAY 260 

    0x8132b286,// 264 PAY 261 

    0x08c25a85,// 265 PAY 262 

    0x1cfa6397,// 266 PAY 263 

    0x83c76731,// 267 PAY 264 

    0x0742c10e,// 268 PAY 265 

    0xad892a86,// 269 PAY 266 

    0x46711935,// 270 PAY 267 

    0x1466c62e,// 271 PAY 268 

    0x9a775c02,// 272 PAY 269 

    0x7effe4e5,// 273 PAY 270 

    0xc698a934,// 274 PAY 271 

    0xcbe3e455,// 275 PAY 272 

    0x85d2b436,// 276 PAY 273 

    0x7b5d5c79,// 277 PAY 274 

    0xd130c6e4,// 278 PAY 275 

    0x5b88ddca,// 279 PAY 276 

    0xe5ce1e61,// 280 PAY 277 

    0x98e51ebe,// 281 PAY 278 

    0xa06b6235,// 282 PAY 279 

    0x879f7bc2,// 283 PAY 280 

    0xae66e25a,// 284 PAY 281 

    0x8bcc665b,// 285 PAY 282 

    0xdf655ad1,// 286 PAY 283 

    0x515d9ee8,// 287 PAY 284 

    0x4f011218,// 288 PAY 285 

    0x352da860,// 289 PAY 286 

    0xc5ff4d67,// 290 PAY 287 

    0xa7356eea,// 291 PAY 288 

    0xc0a83739,// 292 PAY 289 

    0x7fd30891,// 293 PAY 290 

    0xd040d325,// 294 PAY 291 

    0x0171ea95,// 295 PAY 292 

    0xf721cfcd,// 296 PAY 293 

    0x1a7e78bc,// 297 PAY 294 

    0x0955d900,// 298 PAY 295 

/// STA is 1 words. 

/// STA num_pkts       : 207 

/// STA pkt_idx        : 160 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1e 

    0x02811ecf // 299 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt59_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 131 words. 

/// BDA size     is 517 (0x205) 

/// BDA id       is 0x1f49 

    0x02051f49,// 3 BDA   1 

/// PAY Generic Data size   : 517 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xd2c9dfe6,// 4 PAY   1 

    0x1384c9dc,// 5 PAY   2 

    0x4b0428e2,// 6 PAY   3 

    0xc529a9dd,// 7 PAY   4 

    0x7c1aadc7,// 8 PAY   5 

    0xda01ac75,// 9 PAY   6 

    0xec621064,// 10 PAY   7 

    0x8a7a4d9b,// 11 PAY   8 

    0xb02a79d2,// 12 PAY   9 

    0xc0d65a8f,// 13 PAY  10 

    0x1c097008,// 14 PAY  11 

    0x5b0255a8,// 15 PAY  12 

    0x0520980b,// 16 PAY  13 

    0xb231b77c,// 17 PAY  14 

    0x753385f1,// 18 PAY  15 

    0x2666629a,// 19 PAY  16 

    0x3619109e,// 20 PAY  17 

    0x600a5bbc,// 21 PAY  18 

    0x352804d8,// 22 PAY  19 

    0xa3fe9455,// 23 PAY  20 

    0xbd704350,// 24 PAY  21 

    0x762bef74,// 25 PAY  22 

    0x17c57615,// 26 PAY  23 

    0xc41310cf,// 27 PAY  24 

    0x823351d8,// 28 PAY  25 

    0x6d99fdcc,// 29 PAY  26 

    0x41936330,// 30 PAY  27 

    0x3486dd17,// 31 PAY  28 

    0xf28c563a,// 32 PAY  29 

    0x8f0858bb,// 33 PAY  30 

    0x03a7ef78,// 34 PAY  31 

    0x795bc02c,// 35 PAY  32 

    0x9ac87d81,// 36 PAY  33 

    0x0f3b729a,// 37 PAY  34 

    0xbc4067c1,// 38 PAY  35 

    0x47d5c5fd,// 39 PAY  36 

    0x6a805656,// 40 PAY  37 

    0xa7da2cad,// 41 PAY  38 

    0x10557b6d,// 42 PAY  39 

    0x7d1f6837,// 43 PAY  40 

    0x80324980,// 44 PAY  41 

    0x17faa5b8,// 45 PAY  42 

    0x6aa12720,// 46 PAY  43 

    0x458b0e78,// 47 PAY  44 

    0xbda0409f,// 48 PAY  45 

    0x5af3cd12,// 49 PAY  46 

    0x25397088,// 50 PAY  47 

    0x5cca54e0,// 51 PAY  48 

    0x1892c8f4,// 52 PAY  49 

    0xbfd0b2fe,// 53 PAY  50 

    0xd29a4b38,// 54 PAY  51 

    0x951c043c,// 55 PAY  52 

    0xf062be56,// 56 PAY  53 

    0xe6a4997d,// 57 PAY  54 

    0xa73df7fc,// 58 PAY  55 

    0x1bb7d39e,// 59 PAY  56 

    0x6fabe8aa,// 60 PAY  57 

    0x4a1bbecb,// 61 PAY  58 

    0x6c9c1c28,// 62 PAY  59 

    0x5e2bb4da,// 63 PAY  60 

    0xeaa0b09b,// 64 PAY  61 

    0x798fc14a,// 65 PAY  62 

    0x6d14ecf7,// 66 PAY  63 

    0xe3d5cfc8,// 67 PAY  64 

    0x868022d5,// 68 PAY  65 

    0x4ef5832f,// 69 PAY  66 

    0xe632d969,// 70 PAY  67 

    0x89da5e94,// 71 PAY  68 

    0xae4a68fc,// 72 PAY  69 

    0xc894e55f,// 73 PAY  70 

    0xa499b580,// 74 PAY  71 

    0xdf62a4aa,// 75 PAY  72 

    0x21abbbdc,// 76 PAY  73 

    0xe13458de,// 77 PAY  74 

    0x5f35a165,// 78 PAY  75 

    0x24cb9fde,// 79 PAY  76 

    0x2605e6b6,// 80 PAY  77 

    0x6622a2a2,// 81 PAY  78 

    0xa1884d71,// 82 PAY  79 

    0x78fd88c3,// 83 PAY  80 

    0x6aa1fd1c,// 84 PAY  81 

    0xb6f99946,// 85 PAY  82 

    0x07a07f92,// 86 PAY  83 

    0x00b3d27d,// 87 PAY  84 

    0x8cc41be3,// 88 PAY  85 

    0x0f34e924,// 89 PAY  86 

    0xf5388c93,// 90 PAY  87 

    0xf8ec9370,// 91 PAY  88 

    0x8576012a,// 92 PAY  89 

    0x568216d6,// 93 PAY  90 

    0x3f3c6432,// 94 PAY  91 

    0x706171b5,// 95 PAY  92 

    0x2a9ffa3e,// 96 PAY  93 

    0xd8592cd2,// 97 PAY  94 

    0x7168dbfa,// 98 PAY  95 

    0x5b6a9470,// 99 PAY  96 

    0x2666dbcb,// 100 PAY  97 

    0x6b851531,// 101 PAY  98 

    0xd38beeba,// 102 PAY  99 

    0x2f5f1582,// 103 PAY 100 

    0xe4cf79ff,// 104 PAY 101 

    0x966fb515,// 105 PAY 102 

    0xd863dec9,// 106 PAY 103 

    0xfe7ffd8f,// 107 PAY 104 

    0xf91c89a3,// 108 PAY 105 

    0x90a41af4,// 109 PAY 106 

    0x95150e94,// 110 PAY 107 

    0xdc042bc0,// 111 PAY 108 

    0x51a7148c,// 112 PAY 109 

    0xff64fcd5,// 113 PAY 110 

    0x68f99d48,// 114 PAY 111 

    0xceffb823,// 115 PAY 112 

    0x4081abb6,// 116 PAY 113 

    0xac379345,// 117 PAY 114 

    0x23e83215,// 118 PAY 115 

    0x014e5897,// 119 PAY 116 

    0x2a2cf6d8,// 120 PAY 117 

    0xedc52e96,// 121 PAY 118 

    0x2b82485c,// 122 PAY 119 

    0x99d8d8ae,// 123 PAY 120 

    0x936d18ae,// 124 PAY 121 

    0x98ad13a2,// 125 PAY 122 

    0xfd26a123,// 126 PAY 123 

    0x84cc4da3,// 127 PAY 124 

    0x9508ebc2,// 128 PAY 125 

    0xfb5f3edd,// 129 PAY 126 

    0xf7c1180e,// 130 PAY 127 

    0x4002ba26,// 131 PAY 128 

    0xb276b007,// 132 PAY 129 

    0xaf000000,// 133 PAY 130 

/// HASH is  16 bytes 

    0xa499b580,// 134 HSH   1 

    0xdf62a4aa,// 135 HSH   2 

    0x21abbbdc,// 136 HSH   3 

    0xe13458de,// 137 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 244 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7a 

    0x01107af4 // 138 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt60_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 71 words. 

/// BDA size     is 277 (0x115) 

/// BDA id       is 0x6d6b 

    0x01156d6b,// 3 BDA   1 

/// PAY Generic Data size   : 277 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xd8949def,// 4 PAY   1 

    0x65ab970b,// 5 PAY   2 

    0x08f921a4,// 6 PAY   3 

    0x7e8b6c13,// 7 PAY   4 

    0x590a999f,// 8 PAY   5 

    0xa93a409e,// 9 PAY   6 

    0x70f111f0,// 10 PAY   7 

    0xdd46c662,// 11 PAY   8 

    0x081155ae,// 12 PAY   9 

    0x3bb1cf67,// 13 PAY  10 

    0x6e1260e4,// 14 PAY  11 

    0x0d5c75a9,// 15 PAY  12 

    0x06ab90e7,// 16 PAY  13 

    0x93711c5a,// 17 PAY  14 

    0x38bda3f0,// 18 PAY  15 

    0xfb7b0a3b,// 19 PAY  16 

    0x72b2e5cb,// 20 PAY  17 

    0x0e4a0d7a,// 21 PAY  18 

    0x5dd443d0,// 22 PAY  19 

    0x81782db8,// 23 PAY  20 

    0xd64ec731,// 24 PAY  21 

    0xdca0f562,// 25 PAY  22 

    0xc1bb70df,// 26 PAY  23 

    0x8093f14e,// 27 PAY  24 

    0x5421a821,// 28 PAY  25 

    0x9a3ad9fb,// 29 PAY  26 

    0xe4c38e2d,// 30 PAY  27 

    0xaa1951b5,// 31 PAY  28 

    0x15993a02,// 32 PAY  29 

    0x65ffc0c0,// 33 PAY  30 

    0xf71fccc2,// 34 PAY  31 

    0x5de33066,// 35 PAY  32 

    0xb7de89af,// 36 PAY  33 

    0x4e514fba,// 37 PAY  34 

    0x5be1a258,// 38 PAY  35 

    0x663add1f,// 39 PAY  36 

    0x4b113471,// 40 PAY  37 

    0x5530dc2c,// 41 PAY  38 

    0x82547ac0,// 42 PAY  39 

    0x6024d3ca,// 43 PAY  40 

    0x04ae6de5,// 44 PAY  41 

    0xc9750621,// 45 PAY  42 

    0x51e9ffad,// 46 PAY  43 

    0xc9af76f4,// 47 PAY  44 

    0xb27adb11,// 48 PAY  45 

    0x6972f129,// 49 PAY  46 

    0x5964ab03,// 50 PAY  47 

    0xc2a270b8,// 51 PAY  48 

    0x92723007,// 52 PAY  49 

    0x113c7a5b,// 53 PAY  50 

    0xd2b0fd61,// 54 PAY  51 

    0x430c0fac,// 55 PAY  52 

    0xc96660e3,// 56 PAY  53 

    0x94124a18,// 57 PAY  54 

    0x6fa4ca6b,// 58 PAY  55 

    0x52834ea6,// 59 PAY  56 

    0xcf57d3f8,// 60 PAY  57 

    0x97c981c1,// 61 PAY  58 

    0x5644f15e,// 62 PAY  59 

    0x98901130,// 63 PAY  60 

    0xe66ab173,// 64 PAY  61 

    0x8ccad062,// 65 PAY  62 

    0x2896f173,// 66 PAY  63 

    0x5740700f,// 67 PAY  64 

    0x2d9613b8,// 68 PAY  65 

    0x930b62d4,// 69 PAY  66 

    0x550e6299,// 70 PAY  67 

    0x4bdf023e,// 71 PAY  68 

    0x7d994681,// 72 PAY  69 

    0x9a000000,// 73 PAY  70 

/// HASH is  12 bytes 

    0x5740700f,// 74 HSH   1 

    0x2d9613b8,// 75 HSH   2 

    0x930b62d4,// 76 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 18 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xda 

    0x0051da12 // 77 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt61_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 175 words. 

/// BDA size     is 694 (0x2b6) 

/// BDA id       is 0xb58b 

    0x02b6b58b,// 3 BDA   1 

/// PAY Generic Data size   : 694 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x0e5ad5ff,// 4 PAY   1 

    0x19409564,// 5 PAY   2 

    0xb3540957,// 6 PAY   3 

    0xc73a6d89,// 7 PAY   4 

    0x14bcfdd1,// 8 PAY   5 

    0xbe04d062,// 9 PAY   6 

    0xfc1075ea,// 10 PAY   7 

    0xb4f0f79e,// 11 PAY   8 

    0xf6199306,// 12 PAY   9 

    0xe23ae645,// 13 PAY  10 

    0x82d97d8b,// 14 PAY  11 

    0x37010bdb,// 15 PAY  12 

    0xa11db938,// 16 PAY  13 

    0xfd22e970,// 17 PAY  14 

    0x0e15e0c4,// 18 PAY  15 

    0x28850c34,// 19 PAY  16 

    0x70505611,// 20 PAY  17 

    0xc18da583,// 21 PAY  18 

    0xd082dc58,// 22 PAY  19 

    0x4f053472,// 23 PAY  20 

    0xbdbb5850,// 24 PAY  21 

    0xd13ae5b3,// 25 PAY  22 

    0x707d824c,// 26 PAY  23 

    0x33f07fed,// 27 PAY  24 

    0xed7de9d1,// 28 PAY  25 

    0xc3151584,// 29 PAY  26 

    0x578fa83d,// 30 PAY  27 

    0xa2001963,// 31 PAY  28 

    0x9b399dc3,// 32 PAY  29 

    0x2f6fc91f,// 33 PAY  30 

    0x09e3fa92,// 34 PAY  31 

    0x0393938d,// 35 PAY  32 

    0xa122f2db,// 36 PAY  33 

    0x49858b4c,// 37 PAY  34 

    0xb78151b0,// 38 PAY  35 

    0x6a2e4037,// 39 PAY  36 

    0x7b2ddae7,// 40 PAY  37 

    0x3279bae0,// 41 PAY  38 

    0xb80ec4ed,// 42 PAY  39 

    0xd3e8c5bb,// 43 PAY  40 

    0xa27ba7d5,// 44 PAY  41 

    0xcba6384b,// 45 PAY  42 

    0x4ba7153f,// 46 PAY  43 

    0xa4412498,// 47 PAY  44 

    0x145efdbe,// 48 PAY  45 

    0xc1f39d3f,// 49 PAY  46 

    0x6ff5b67a,// 50 PAY  47 

    0xac694a2d,// 51 PAY  48 

    0x6609a13e,// 52 PAY  49 

    0xd34b0fb8,// 53 PAY  50 

    0x402f0853,// 54 PAY  51 

    0x86d30eb3,// 55 PAY  52 

    0x9400e4d5,// 56 PAY  53 

    0x88349a3e,// 57 PAY  54 

    0xbbf499a1,// 58 PAY  55 

    0x5a8222c4,// 59 PAY  56 

    0xca03dfa0,// 60 PAY  57 

    0x1764fdbc,// 61 PAY  58 

    0xaea2ff9b,// 62 PAY  59 

    0x8cf789d6,// 63 PAY  60 

    0x7866df99,// 64 PAY  61 

    0xb99639c6,// 65 PAY  62 

    0x654882ad,// 66 PAY  63 

    0x58897f3c,// 67 PAY  64 

    0x2dacadd4,// 68 PAY  65 

    0x6373352d,// 69 PAY  66 

    0x2269ccf3,// 70 PAY  67 

    0x8ca73f64,// 71 PAY  68 

    0x3e4b9986,// 72 PAY  69 

    0xa2debe5b,// 73 PAY  70 

    0x1fac72ea,// 74 PAY  71 

    0xf21a5427,// 75 PAY  72 

    0x3de13454,// 76 PAY  73 

    0x2c000232,// 77 PAY  74 

    0x91041352,// 78 PAY  75 

    0x70b1457b,// 79 PAY  76 

    0xa5e6d12e,// 80 PAY  77 

    0x56aba714,// 81 PAY  78 

    0x9493528a,// 82 PAY  79 

    0x88d87c00,// 83 PAY  80 

    0x02677010,// 84 PAY  81 

    0x25b31ba1,// 85 PAY  82 

    0x06c0c5be,// 86 PAY  83 

    0x54bffb0f,// 87 PAY  84 

    0x62daeec5,// 88 PAY  85 

    0x1d9295f2,// 89 PAY  86 

    0x20c7f86a,// 90 PAY  87 

    0x678bd828,// 91 PAY  88 

    0x26709867,// 92 PAY  89 

    0x7e7d8ebd,// 93 PAY  90 

    0x5ba17a7e,// 94 PAY  91 

    0x0edf6aef,// 95 PAY  92 

    0x095704af,// 96 PAY  93 

    0x3c5baa5d,// 97 PAY  94 

    0xa86dff65,// 98 PAY  95 

    0xec70f1cf,// 99 PAY  96 

    0xb22228cf,// 100 PAY  97 

    0x6b16748f,// 101 PAY  98 

    0x26d538e6,// 102 PAY  99 

    0x0ab5eb05,// 103 PAY 100 

    0x4c779145,// 104 PAY 101 

    0x6cfb1128,// 105 PAY 102 

    0x53616ada,// 106 PAY 103 

    0x3b69f7fa,// 107 PAY 104 

    0x18722e71,// 108 PAY 105 

    0xa5bd16ec,// 109 PAY 106 

    0xc467261e,// 110 PAY 107 

    0x8627c949,// 111 PAY 108 

    0xd688be47,// 112 PAY 109 

    0x6ed12a5c,// 113 PAY 110 

    0xf10fdd94,// 114 PAY 111 

    0x097a4c16,// 115 PAY 112 

    0xbbd4d07a,// 116 PAY 113 

    0xf33542fe,// 117 PAY 114 

    0xf4324f61,// 118 PAY 115 

    0x333efb2e,// 119 PAY 116 

    0x5bf91726,// 120 PAY 117 

    0x29acacb1,// 121 PAY 118 

    0x50ac9fdb,// 122 PAY 119 

    0xbfb86510,// 123 PAY 120 

    0x89460451,// 124 PAY 121 

    0x281d5c5c,// 125 PAY 122 

    0x7c9f070b,// 126 PAY 123 

    0xbd929316,// 127 PAY 124 

    0xe1cf6232,// 128 PAY 125 

    0xd224ff01,// 129 PAY 126 

    0x27973b9f,// 130 PAY 127 

    0x9b7a09f7,// 131 PAY 128 

    0xc5ff4be6,// 132 PAY 129 

    0x5838bc54,// 133 PAY 130 

    0x610e3a89,// 134 PAY 131 

    0x94fcd911,// 135 PAY 132 

    0x26314d66,// 136 PAY 133 

    0xf81f90fc,// 137 PAY 134 

    0xc96deab1,// 138 PAY 135 

    0x4837eddc,// 139 PAY 136 

    0x968ad125,// 140 PAY 137 

    0x489ff91a,// 141 PAY 138 

    0xe835a642,// 142 PAY 139 

    0xcb8dea08,// 143 PAY 140 

    0x7b991d53,// 144 PAY 141 

    0xe65ea6b7,// 145 PAY 142 

    0x6cf7e9dc,// 146 PAY 143 

    0xb0e8fa30,// 147 PAY 144 

    0xa5f7dc25,// 148 PAY 145 

    0x75930657,// 149 PAY 146 

    0x85ec7315,// 150 PAY 147 

    0x9e237082,// 151 PAY 148 

    0x725d69b9,// 152 PAY 149 

    0xcc510b93,// 153 PAY 150 

    0x4823a89e,// 154 PAY 151 

    0xe504af1e,// 155 PAY 152 

    0xdc8255db,// 156 PAY 153 

    0x7e20f22e,// 157 PAY 154 

    0x8f9f0916,// 158 PAY 155 

    0xde8b0926,// 159 PAY 156 

    0x82363779,// 160 PAY 157 

    0x5002f992,// 161 PAY 158 

    0xf37ba8a7,// 162 PAY 159 

    0x4093c10f,// 163 PAY 160 

    0xf34e231d,// 164 PAY 161 

    0xa3c9124a,// 165 PAY 162 

    0x841e088b,// 166 PAY 163 

    0xcd5f68ed,// 167 PAY 164 

    0xd0412a76,// 168 PAY 165 

    0x3e96a588,// 169 PAY 166 

    0x2a7889f9,// 170 PAY 167 

    0xdbbb6467,// 171 PAY 168 

    0x8e972452,// 172 PAY 169 

    0xef9bcee0,// 173 PAY 170 

    0xf83f45a9,// 174 PAY 171 

    0x85c146fa,// 175 PAY 172 

    0xa75a16c5,// 176 PAY 173 

    0x95c20000,// 177 PAY 174 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x03586eb2 // 178 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt62_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 482 words. 

/// BDA size     is 1921 (0x781) 

/// BDA id       is 0x8eca 

    0x07818eca,// 3 BDA   1 

/// PAY Generic Data size   : 1921 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x82f087f5,// 4 PAY   1 

    0xf5e15740,// 5 PAY   2 

    0x8bda2bc7,// 6 PAY   3 

    0xc3012014,// 7 PAY   4 

    0x0f6dd83c,// 8 PAY   5 

    0xcbe9ab0b,// 9 PAY   6 

    0x6e66390b,// 10 PAY   7 

    0xb392d028,// 11 PAY   8 

    0xa3d9678d,// 12 PAY   9 

    0x020a4e21,// 13 PAY  10 

    0xbd980e50,// 14 PAY  11 

    0xb9c8aac6,// 15 PAY  12 

    0xb86d530b,// 16 PAY  13 

    0xab81c644,// 17 PAY  14 

    0x21297432,// 18 PAY  15 

    0xa5ca8ef9,// 19 PAY  16 

    0xad7df6d7,// 20 PAY  17 

    0x60a31bd6,// 21 PAY  18 

    0xed9c61eb,// 22 PAY  19 

    0x1c366484,// 23 PAY  20 

    0xcff80914,// 24 PAY  21 

    0xef869c06,// 25 PAY  22 

    0xc1af5142,// 26 PAY  23 

    0x670c9a5b,// 27 PAY  24 

    0x42575957,// 28 PAY  25 

    0x0bd4eaec,// 29 PAY  26 

    0xb11dcae8,// 30 PAY  27 

    0xadde7d2a,// 31 PAY  28 

    0x3c6b13f0,// 32 PAY  29 

    0x64341781,// 33 PAY  30 

    0x9d7b72c9,// 34 PAY  31 

    0x8e59b58a,// 35 PAY  32 

    0x33403142,// 36 PAY  33 

    0xaaab97bf,// 37 PAY  34 

    0xe1369b8d,// 38 PAY  35 

    0xb2a65519,// 39 PAY  36 

    0x8f685c8c,// 40 PAY  37 

    0x58eb4901,// 41 PAY  38 

    0x89a82ac5,// 42 PAY  39 

    0xfe74d872,// 43 PAY  40 

    0x876546db,// 44 PAY  41 

    0x81fe036f,// 45 PAY  42 

    0xecf0a24d,// 46 PAY  43 

    0x3bb90850,// 47 PAY  44 

    0xe44fe81d,// 48 PAY  45 

    0x9e7a5b53,// 49 PAY  46 

    0x6b363a83,// 50 PAY  47 

    0x81d65839,// 51 PAY  48 

    0xa8026694,// 52 PAY  49 

    0xd4e58a1b,// 53 PAY  50 

    0x14a75218,// 54 PAY  51 

    0x1a4eab42,// 55 PAY  52 

    0x8b63e9c4,// 56 PAY  53 

    0x8c22ee4c,// 57 PAY  54 

    0x851375b2,// 58 PAY  55 

    0xeabf0f73,// 59 PAY  56 

    0xbf625830,// 60 PAY  57 

    0x8cb52d2f,// 61 PAY  58 

    0x4d56ff49,// 62 PAY  59 

    0xf4dcbe7f,// 63 PAY  60 

    0x8a5bcb40,// 64 PAY  61 

    0x91e1b7b9,// 65 PAY  62 

    0x57a57a77,// 66 PAY  63 

    0x809306eb,// 67 PAY  64 

    0x7d7eb05b,// 68 PAY  65 

    0xf02d3e9e,// 69 PAY  66 

    0x1b644cfd,// 70 PAY  67 

    0x6c3fbdd0,// 71 PAY  68 

    0x838df044,// 72 PAY  69 

    0x9fffd2b5,// 73 PAY  70 

    0xd0ef8228,// 74 PAY  71 

    0x92635d5d,// 75 PAY  72 

    0xdeb903fb,// 76 PAY  73 

    0xa7aa4c1a,// 77 PAY  74 

    0xfd2336e9,// 78 PAY  75 

    0xeb2bb83b,// 79 PAY  76 

    0xaead477a,// 80 PAY  77 

    0xf4cec541,// 81 PAY  78 

    0x0c561435,// 82 PAY  79 

    0xa346e9d7,// 83 PAY  80 

    0x42f3730c,// 84 PAY  81 

    0x81156901,// 85 PAY  82 

    0xbd9e0c5b,// 86 PAY  83 

    0xbc0b7c9d,// 87 PAY  84 

    0x19e87875,// 88 PAY  85 

    0x3c8b9dcd,// 89 PAY  86 

    0x946ad01e,// 90 PAY  87 

    0xc8174c24,// 91 PAY  88 

    0x8d52ca88,// 92 PAY  89 

    0xd7bf79f1,// 93 PAY  90 

    0xebaeb6eb,// 94 PAY  91 

    0xfc69193d,// 95 PAY  92 

    0x50d4255e,// 96 PAY  93 

    0xf8c11df0,// 97 PAY  94 

    0x0cddb64f,// 98 PAY  95 

    0x1512b3a6,// 99 PAY  96 

    0xef5a5496,// 100 PAY  97 

    0x6c46cb42,// 101 PAY  98 

    0x75abdc37,// 102 PAY  99 

    0x615fbf01,// 103 PAY 100 

    0xb453ce46,// 104 PAY 101 

    0x6ef5c9c5,// 105 PAY 102 

    0x7469bae7,// 106 PAY 103 

    0xb3039008,// 107 PAY 104 

    0xef35ba43,// 108 PAY 105 

    0x2dfd819b,// 109 PAY 106 

    0xb014ff2d,// 110 PAY 107 

    0xa7a6895b,// 111 PAY 108 

    0x1240ca2d,// 112 PAY 109 

    0xc2438c03,// 113 PAY 110 

    0x0d0f308b,// 114 PAY 111 

    0x2e8490bb,// 115 PAY 112 

    0xeaf0dc33,// 116 PAY 113 

    0x6347f7bc,// 117 PAY 114 

    0xcd924fce,// 118 PAY 115 

    0x11342387,// 119 PAY 116 

    0x55e15928,// 120 PAY 117 

    0xb86ef830,// 121 PAY 118 

    0x8d0bf526,// 122 PAY 119 

    0xbdc3e285,// 123 PAY 120 

    0xa7ac4118,// 124 PAY 121 

    0x8a62e01b,// 125 PAY 122 

    0x2e729289,// 126 PAY 123 

    0xc2e5dbc0,// 127 PAY 124 

    0x81117162,// 128 PAY 125 

    0xcdbede80,// 129 PAY 126 

    0x3a4070d5,// 130 PAY 127 

    0xa2521532,// 131 PAY 128 

    0x64749177,// 132 PAY 129 

    0x7d57ad4e,// 133 PAY 130 

    0xbffefa86,// 134 PAY 131 

    0x127a7db5,// 135 PAY 132 

    0x24dc8d94,// 136 PAY 133 

    0x7f684b34,// 137 PAY 134 

    0x182c34d2,// 138 PAY 135 

    0x0cfea711,// 139 PAY 136 

    0xbbe9e334,// 140 PAY 137 

    0xa0173da6,// 141 PAY 138 

    0x7d402f59,// 142 PAY 139 

    0x21314374,// 143 PAY 140 

    0x96fb7c71,// 144 PAY 141 

    0x333c57f3,// 145 PAY 142 

    0xdb6ef879,// 146 PAY 143 

    0x946e3d46,// 147 PAY 144 

    0xb1486c4d,// 148 PAY 145 

    0x0ec1567d,// 149 PAY 146 

    0x2a5e3740,// 150 PAY 147 

    0x9e7bc7b4,// 151 PAY 148 

    0x5827303a,// 152 PAY 149 

    0xdf5f7b6b,// 153 PAY 150 

    0xea60e4b1,// 154 PAY 151 

    0x1bdd61a2,// 155 PAY 152 

    0xf8d7b82c,// 156 PAY 153 

    0xd9268b16,// 157 PAY 154 

    0x161d628a,// 158 PAY 155 

    0xe89c910c,// 159 PAY 156 

    0x1930f47c,// 160 PAY 157 

    0xd00b646e,// 161 PAY 158 

    0x7d426960,// 162 PAY 159 

    0x8c225bcb,// 163 PAY 160 

    0x249f5f56,// 164 PAY 161 

    0xbc050cef,// 165 PAY 162 

    0x1621cb9c,// 166 PAY 163 

    0x5f6974cf,// 167 PAY 164 

    0x75103e06,// 168 PAY 165 

    0x0eecbf7b,// 169 PAY 166 

    0xafecce96,// 170 PAY 167 

    0x30351dc8,// 171 PAY 168 

    0x9815a223,// 172 PAY 169 

    0xdb69fdb1,// 173 PAY 170 

    0x4fa3d17d,// 174 PAY 171 

    0x03bf9ad1,// 175 PAY 172 

    0x868cd9d7,// 176 PAY 173 

    0xf6e83e7f,// 177 PAY 174 

    0x4f53080a,// 178 PAY 175 

    0xb9c49c1e,// 179 PAY 176 

    0xc65f1df8,// 180 PAY 177 

    0x1dd02148,// 181 PAY 178 

    0x8b257aa4,// 182 PAY 179 

    0x01f34199,// 183 PAY 180 

    0xedf7c76e,// 184 PAY 181 

    0xda83caac,// 185 PAY 182 

    0x20211849,// 186 PAY 183 

    0x1ec6f82e,// 187 PAY 184 

    0x88605c61,// 188 PAY 185 

    0xe361ece8,// 189 PAY 186 

    0x0a3e80f8,// 190 PAY 187 

    0x675794e5,// 191 PAY 188 

    0xe3ae603a,// 192 PAY 189 

    0x18deeba8,// 193 PAY 190 

    0xfa862d7b,// 194 PAY 191 

    0x7f9472ec,// 195 PAY 192 

    0x5a51afe5,// 196 PAY 193 

    0x180a1b5b,// 197 PAY 194 

    0x201f85f5,// 198 PAY 195 

    0xeeb5484f,// 199 PAY 196 

    0xe397b86a,// 200 PAY 197 

    0xce7dbce2,// 201 PAY 198 

    0xefd1c76b,// 202 PAY 199 

    0x74004e85,// 203 PAY 200 

    0x5a93397d,// 204 PAY 201 

    0x4a8d184b,// 205 PAY 202 

    0xbaf0859c,// 206 PAY 203 

    0xea3f3e51,// 207 PAY 204 

    0x150ef2d3,// 208 PAY 205 

    0x73a00d7b,// 209 PAY 206 

    0xff49544a,// 210 PAY 207 

    0xe36086ca,// 211 PAY 208 

    0x0300176c,// 212 PAY 209 

    0x62167ebf,// 213 PAY 210 

    0x5f97be52,// 214 PAY 211 

    0xea7c10bd,// 215 PAY 212 

    0x77a4bfc1,// 216 PAY 213 

    0xd8c6c480,// 217 PAY 214 

    0x7c3bdf7b,// 218 PAY 215 

    0x1a1cac43,// 219 PAY 216 

    0xfe605f76,// 220 PAY 217 

    0x70b1a8a8,// 221 PAY 218 

    0x6bf6b68d,// 222 PAY 219 

    0x1a386d6e,// 223 PAY 220 

    0x49d2bcb1,// 224 PAY 221 

    0x354a7f9b,// 225 PAY 222 

    0x6594a061,// 226 PAY 223 

    0x177e37b4,// 227 PAY 224 

    0x0f24df63,// 228 PAY 225 

    0xd955d051,// 229 PAY 226 

    0xc9883dad,// 230 PAY 227 

    0x6adc18c5,// 231 PAY 228 

    0x20b9fef3,// 232 PAY 229 

    0x14e386f8,// 233 PAY 230 

    0x900691e7,// 234 PAY 231 

    0x5157d6ca,// 235 PAY 232 

    0xa422db62,// 236 PAY 233 

    0x3041ba11,// 237 PAY 234 

    0x0d2a2259,// 238 PAY 235 

    0x7185159a,// 239 PAY 236 

    0x172f5028,// 240 PAY 237 

    0x22d049f0,// 241 PAY 238 

    0xe042d4c1,// 242 PAY 239 

    0xdca1ec3e,// 243 PAY 240 

    0x47ddb0f5,// 244 PAY 241 

    0x0e98b5f5,// 245 PAY 242 

    0x7ba019f2,// 246 PAY 243 

    0x3c270035,// 247 PAY 244 

    0x396c696b,// 248 PAY 245 

    0x6f4dc860,// 249 PAY 246 

    0x18e57c6b,// 250 PAY 247 

    0xfb2c7163,// 251 PAY 248 

    0xd5b94708,// 252 PAY 249 

    0x0bb83ade,// 253 PAY 250 

    0x1b6cd5de,// 254 PAY 251 

    0xe176877a,// 255 PAY 252 

    0x49b54f04,// 256 PAY 253 

    0x1eaf6a0f,// 257 PAY 254 

    0x73a72007,// 258 PAY 255 

    0xf95b74b5,// 259 PAY 256 

    0x3fe5808b,// 260 PAY 257 

    0x4f14694f,// 261 PAY 258 

    0x4a616166,// 262 PAY 259 

    0x3c302cbd,// 263 PAY 260 

    0xc2350110,// 264 PAY 261 

    0x5233893b,// 265 PAY 262 

    0xb1ee28a3,// 266 PAY 263 

    0x388be92d,// 267 PAY 264 

    0x86df4643,// 268 PAY 265 

    0xe97765f5,// 269 PAY 266 

    0x1b929276,// 270 PAY 267 

    0x71b076be,// 271 PAY 268 

    0xea44479e,// 272 PAY 269 

    0x9ad99973,// 273 PAY 270 

    0x7a33159b,// 274 PAY 271 

    0xf9efaf52,// 275 PAY 272 

    0x9134de94,// 276 PAY 273 

    0xb1b3383e,// 277 PAY 274 

    0xfad291f2,// 278 PAY 275 

    0x28f256a1,// 279 PAY 276 

    0x435be923,// 280 PAY 277 

    0x2e50deac,// 281 PAY 278 

    0x8ecaf827,// 282 PAY 279 

    0x687d1f21,// 283 PAY 280 

    0xbb8dcd98,// 284 PAY 281 

    0x3e6afef4,// 285 PAY 282 

    0xac1741b9,// 286 PAY 283 

    0xd8d64cef,// 287 PAY 284 

    0x6e76cd06,// 288 PAY 285 

    0xadff46ba,// 289 PAY 286 

    0xb0371407,// 290 PAY 287 

    0x4f20b434,// 291 PAY 288 

    0x76263706,// 292 PAY 289 

    0xaad04c69,// 293 PAY 290 

    0x85bc5806,// 294 PAY 291 

    0xc0bdda8c,// 295 PAY 292 

    0xa5d25216,// 296 PAY 293 

    0x1835cd74,// 297 PAY 294 

    0xff1755ea,// 298 PAY 295 

    0x5168c6f3,// 299 PAY 296 

    0x59695415,// 300 PAY 297 

    0x61f610b0,// 301 PAY 298 

    0x6d749f10,// 302 PAY 299 

    0x825ec621,// 303 PAY 300 

    0xdaf2f2b4,// 304 PAY 301 

    0x63d3ddd3,// 305 PAY 302 

    0xcf758e13,// 306 PAY 303 

    0xae7ef4a8,// 307 PAY 304 

    0xf2c7ce3b,// 308 PAY 305 

    0xbeb267ea,// 309 PAY 306 

    0x9f7d3fa3,// 310 PAY 307 

    0x3ea13910,// 311 PAY 308 

    0x7259aaeb,// 312 PAY 309 

    0xfe4b2fd9,// 313 PAY 310 

    0x1ee6732c,// 314 PAY 311 

    0x44de99e0,// 315 PAY 312 

    0x0bf3fa32,// 316 PAY 313 

    0x329fa948,// 317 PAY 314 

    0xf66bc465,// 318 PAY 315 

    0xe80bad9e,// 319 PAY 316 

    0x3eb366cc,// 320 PAY 317 

    0x25b4af18,// 321 PAY 318 

    0x5a2496ea,// 322 PAY 319 

    0xe6da103a,// 323 PAY 320 

    0x0b987f8d,// 324 PAY 321 

    0xc2ac1ad4,// 325 PAY 322 

    0x3c179081,// 326 PAY 323 

    0x7c0aa866,// 327 PAY 324 

    0x81747d01,// 328 PAY 325 

    0x08a8d8a3,// 329 PAY 326 

    0xd2b37d47,// 330 PAY 327 

    0x89283747,// 331 PAY 328 

    0x3be2a085,// 332 PAY 329 

    0x2a01a738,// 333 PAY 330 

    0x896ba867,// 334 PAY 331 

    0xafbecacf,// 335 PAY 332 

    0xae2c30a9,// 336 PAY 333 

    0xc2d74d32,// 337 PAY 334 

    0x5b98cbe0,// 338 PAY 335 

    0xf8d70943,// 339 PAY 336 

    0xa88a55fb,// 340 PAY 337 

    0x9a1f59fa,// 341 PAY 338 

    0xa7c3c53f,// 342 PAY 339 

    0xd219a679,// 343 PAY 340 

    0x1e2aef06,// 344 PAY 341 

    0x7189c4d2,// 345 PAY 342 

    0x9d1f08fe,// 346 PAY 343 

    0xeff20426,// 347 PAY 344 

    0xdb809944,// 348 PAY 345 

    0xaa384c6c,// 349 PAY 346 

    0x41854823,// 350 PAY 347 

    0x617ea640,// 351 PAY 348 

    0x398f4214,// 352 PAY 349 

    0xbfa32f71,// 353 PAY 350 

    0xb2b16a74,// 354 PAY 351 

    0xb71ac2d9,// 355 PAY 352 

    0x55e4c41b,// 356 PAY 353 

    0x15bf2063,// 357 PAY 354 

    0x1567b75a,// 358 PAY 355 

    0x3037114d,// 359 PAY 356 

    0x0b09e608,// 360 PAY 357 

    0x616f4d2a,// 361 PAY 358 

    0xda9cfb36,// 362 PAY 359 

    0x0bde2276,// 363 PAY 360 

    0xd732f25e,// 364 PAY 361 

    0xc72a8fab,// 365 PAY 362 

    0x0a19e79e,// 366 PAY 363 

    0x5031cb0b,// 367 PAY 364 

    0x16b69711,// 368 PAY 365 

    0xe04af3a0,// 369 PAY 366 

    0x57b6b17c,// 370 PAY 367 

    0xf3d21c04,// 371 PAY 368 

    0xe71e516f,// 372 PAY 369 

    0xaf2219ea,// 373 PAY 370 

    0xb06001ce,// 374 PAY 371 

    0xc16d1050,// 375 PAY 372 

    0xfeb8a2a6,// 376 PAY 373 

    0x9d5fe557,// 377 PAY 374 

    0x13003d56,// 378 PAY 375 

    0xa7da9661,// 379 PAY 376 

    0x67ff0e49,// 380 PAY 377 

    0x89c0407b,// 381 PAY 378 

    0xaf164825,// 382 PAY 379 

    0x915840bc,// 383 PAY 380 

    0x22d8557e,// 384 PAY 381 

    0x36ace5c3,// 385 PAY 382 

    0x726d85ef,// 386 PAY 383 

    0xdbe95140,// 387 PAY 384 

    0x878ebb64,// 388 PAY 385 

    0x80794160,// 389 PAY 386 

    0xd96bf2c9,// 390 PAY 387 

    0x1a18729b,// 391 PAY 388 

    0xa21227b3,// 392 PAY 389 

    0x64007c09,// 393 PAY 390 

    0x9a43dbc3,// 394 PAY 391 

    0xea619be6,// 395 PAY 392 

    0x8d7bc6ef,// 396 PAY 393 

    0x945b972f,// 397 PAY 394 

    0x4d8c6523,// 398 PAY 395 

    0xeaa281fa,// 399 PAY 396 

    0x72321f6d,// 400 PAY 397 

    0xae2b28cc,// 401 PAY 398 

    0xb3c0c592,// 402 PAY 399 

    0xfb3c1ad0,// 403 PAY 400 

    0xf552d133,// 404 PAY 401 

    0x43498309,// 405 PAY 402 

    0x210cabc9,// 406 PAY 403 

    0xfa4c7656,// 407 PAY 404 

    0xb042350f,// 408 PAY 405 

    0x10132caf,// 409 PAY 406 

    0x906089ab,// 410 PAY 407 

    0x1cdb67b5,// 411 PAY 408 

    0xdfbd2d1b,// 412 PAY 409 

    0x166a11ed,// 413 PAY 410 

    0xbfa3f52d,// 414 PAY 411 

    0x40a38b81,// 415 PAY 412 

    0xbbcf2b0e,// 416 PAY 413 

    0x1b039620,// 417 PAY 414 

    0x11416c7e,// 418 PAY 415 

    0x81d4c306,// 419 PAY 416 

    0x6940c259,// 420 PAY 417 

    0x65ff1a25,// 421 PAY 418 

    0x278b7040,// 422 PAY 419 

    0x208c336c,// 423 PAY 420 

    0xf3c61f25,// 424 PAY 421 

    0xb05c15c6,// 425 PAY 422 

    0x8a099682,// 426 PAY 423 

    0x2a57ea29,// 427 PAY 424 

    0x482860a2,// 428 PAY 425 

    0x32b6e0f3,// 429 PAY 426 

    0xf549e2ba,// 430 PAY 427 

    0xea951c23,// 431 PAY 428 

    0xa6e1c0e4,// 432 PAY 429 

    0x92a5cf89,// 433 PAY 430 

    0xa224c498,// 434 PAY 431 

    0xf04fe31d,// 435 PAY 432 

    0x1a5069ba,// 436 PAY 433 

    0x5297a829,// 437 PAY 434 

    0x2dcdd0da,// 438 PAY 435 

    0xf192ba8a,// 439 PAY 436 

    0x362e8bc4,// 440 PAY 437 

    0x0f0b6ca3,// 441 PAY 438 

    0x006216ec,// 442 PAY 439 

    0x6a01b669,// 443 PAY 440 

    0x05e27b48,// 444 PAY 441 

    0x9d4415f4,// 445 PAY 442 

    0x0ef0f7f5,// 446 PAY 443 

    0xdc2aabab,// 447 PAY 444 

    0xdfc28db4,// 448 PAY 445 

    0x4aaa78f4,// 449 PAY 446 

    0xcfc9903f,// 450 PAY 447 

    0x5363de60,// 451 PAY 448 

    0xded86830,// 452 PAY 449 

    0xb8673ac1,// 453 PAY 450 

    0xf065fe14,// 454 PAY 451 

    0x66ef507c,// 455 PAY 452 

    0xde037e83,// 456 PAY 453 

    0xdff4acb6,// 457 PAY 454 

    0xd2830599,// 458 PAY 455 

    0xd75018e1,// 459 PAY 456 

    0xa997a1dd,// 460 PAY 457 

    0x5384c4db,// 461 PAY 458 

    0x0a5dd3df,// 462 PAY 459 

    0x98275b0e,// 463 PAY 460 

    0xd1cfac36,// 464 PAY 461 

    0xb815a395,// 465 PAY 462 

    0xac8aa5b0,// 466 PAY 463 

    0xfc60c30b,// 467 PAY 464 

    0x0cc68f2f,// 468 PAY 465 

    0xd843e03d,// 469 PAY 466 

    0xc4788323,// 470 PAY 467 

    0x0139a4d7,// 471 PAY 468 

    0x19d812d9,// 472 PAY 469 

    0x5a9d2074,// 473 PAY 470 

    0xbfba43ae,// 474 PAY 471 

    0x9ee6750e,// 475 PAY 472 

    0xf066af2a,// 476 PAY 473 

    0xe39fa706,// 477 PAY 474 

    0xd85db270,// 478 PAY 475 

    0x227f7448,// 479 PAY 476 

    0xeb4a0ebf,// 480 PAY 477 

    0x2ff54b7c,// 481 PAY 478 

    0x809465a2,// 482 PAY 479 

    0x1a6b15d3,// 483 PAY 480 

    0x65000000,// 484 PAY 481 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 187 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdd 

    0x02ecdd80 // 485 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt63_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 167 words. 

/// BDA size     is 664 (0x298) 

/// BDA id       is 0xad27 

    0x0298ad27,// 3 BDA   1 

/// PAY Generic Data size   : 664 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x99021180,// 4 PAY   1 

    0x2ee4f095,// 5 PAY   2 

    0xdf2fef4e,// 6 PAY   3 

    0x9fcf0407,// 7 PAY   4 

    0xf81c60f6,// 8 PAY   5 

    0x13618706,// 9 PAY   6 

    0xebd99b8f,// 10 PAY   7 

    0x1cca8ba3,// 11 PAY   8 

    0x23a4add8,// 12 PAY   9 

    0x897cca7c,// 13 PAY  10 

    0x7298199c,// 14 PAY  11 

    0xb32377a1,// 15 PAY  12 

    0x2f488067,// 16 PAY  13 

    0x079341aa,// 17 PAY  14 

    0x0ff4218e,// 18 PAY  15 

    0x434d57f4,// 19 PAY  16 

    0x42fefe84,// 20 PAY  17 

    0x839bacc1,// 21 PAY  18 

    0x34ab628f,// 22 PAY  19 

    0x9411986c,// 23 PAY  20 

    0x823e8e4d,// 24 PAY  21 

    0x1556d949,// 25 PAY  22 

    0xd15bc2e8,// 26 PAY  23 

    0xdc4957f7,// 27 PAY  24 

    0x5ec52f45,// 28 PAY  25 

    0xb4b54a1f,// 29 PAY  26 

    0x2c0748c8,// 30 PAY  27 

    0x99c0c1a0,// 31 PAY  28 

    0xdceb45bb,// 32 PAY  29 

    0x2733aeaa,// 33 PAY  30 

    0x98ad3ee7,// 34 PAY  31 

    0x8bfbb958,// 35 PAY  32 

    0xf7b871d5,// 36 PAY  33 

    0xb0570023,// 37 PAY  34 

    0x05506f32,// 38 PAY  35 

    0x9a0ea1aa,// 39 PAY  36 

    0xce071eaf,// 40 PAY  37 

    0xadf710bb,// 41 PAY  38 

    0x8cd3c69f,// 42 PAY  39 

    0xaef8ba7d,// 43 PAY  40 

    0x3af1d56c,// 44 PAY  41 

    0xf78598bf,// 45 PAY  42 

    0x37d65432,// 46 PAY  43 

    0xcbf0bcd0,// 47 PAY  44 

    0xd28e5fe5,// 48 PAY  45 

    0x628c0e7a,// 49 PAY  46 

    0x2392ccab,// 50 PAY  47 

    0x53fba7e2,// 51 PAY  48 

    0x2f853e42,// 52 PAY  49 

    0xb3337629,// 53 PAY  50 

    0xc8a32c7b,// 54 PAY  51 

    0x9c35fe3f,// 55 PAY  52 

    0x2d75de84,// 56 PAY  53 

    0xf7b1a724,// 57 PAY  54 

    0x34b92d38,// 58 PAY  55 

    0x6ffc1464,// 59 PAY  56 

    0xe2c9b65c,// 60 PAY  57 

    0xd263f67f,// 61 PAY  58 

    0xfed5d062,// 62 PAY  59 

    0xf1e14076,// 63 PAY  60 

    0x37d40c42,// 64 PAY  61 

    0x5d1c4b4e,// 65 PAY  62 

    0xb5ead67b,// 66 PAY  63 

    0x1aef8948,// 67 PAY  64 

    0x76b21cd9,// 68 PAY  65 

    0xb7a866cf,// 69 PAY  66 

    0x4f865e49,// 70 PAY  67 

    0x2a76f432,// 71 PAY  68 

    0x1795a166,// 72 PAY  69 

    0xc7bf2d08,// 73 PAY  70 

    0x57322dbd,// 74 PAY  71 

    0xe8a6ed40,// 75 PAY  72 

    0xfc8d80ce,// 76 PAY  73 

    0xbae85405,// 77 PAY  74 

    0x4c065006,// 78 PAY  75 

    0xef41e2c8,// 79 PAY  76 

    0xda322e29,// 80 PAY  77 

    0xa15e6d40,// 81 PAY  78 

    0x230ec301,// 82 PAY  79 

    0x38c0ecc0,// 83 PAY  80 

    0x60397b81,// 84 PAY  81 

    0xd65dbb8d,// 85 PAY  82 

    0x86a8307f,// 86 PAY  83 

    0x8d2e43d5,// 87 PAY  84 

    0xf2e948ff,// 88 PAY  85 

    0x563d25cf,// 89 PAY  86 

    0xbc2866ce,// 90 PAY  87 

    0x4e2c6c54,// 91 PAY  88 

    0x8a5ed2f1,// 92 PAY  89 

    0xf7385b5c,// 93 PAY  90 

    0xa4873908,// 94 PAY  91 

    0x763fe3a8,// 95 PAY  92 

    0x4e428e9a,// 96 PAY  93 

    0x0a0d4a5a,// 97 PAY  94 

    0x7019a37d,// 98 PAY  95 

    0xe9454a42,// 99 PAY  96 

    0xef0fb731,// 100 PAY  97 

    0xbc1e62f6,// 101 PAY  98 

    0x1593cb15,// 102 PAY  99 

    0x66244d47,// 103 PAY 100 

    0x9f71de81,// 104 PAY 101 

    0x6a81b063,// 105 PAY 102 

    0x90f71a12,// 106 PAY 103 

    0x0f9b6b63,// 107 PAY 104 

    0x9eff9917,// 108 PAY 105 

    0x3c8327de,// 109 PAY 106 

    0xe4dc128d,// 110 PAY 107 

    0x5fbe0921,// 111 PAY 108 

    0x4f510848,// 112 PAY 109 

    0xed18ee20,// 113 PAY 110 

    0xbf1ec2ba,// 114 PAY 111 

    0xb095d27e,// 115 PAY 112 

    0x9c5d4df8,// 116 PAY 113 

    0x8dc541c6,// 117 PAY 114 

    0xb69c6081,// 118 PAY 115 

    0x1a36fa94,// 119 PAY 116 

    0x37f7fce4,// 120 PAY 117 

    0x2fe0cfee,// 121 PAY 118 

    0xf2352f37,// 122 PAY 119 

    0x76363170,// 123 PAY 120 

    0xa0661cad,// 124 PAY 121 

    0x59335573,// 125 PAY 122 

    0xe407081d,// 126 PAY 123 

    0x0d77aa9b,// 127 PAY 124 

    0x17eade5a,// 128 PAY 125 

    0xac998893,// 129 PAY 126 

    0xb073bdc1,// 130 PAY 127 

    0xbf0b7915,// 131 PAY 128 

    0xd3a9fbd2,// 132 PAY 129 

    0x958b266b,// 133 PAY 130 

    0x4ea19f8a,// 134 PAY 131 

    0x5b12fb3b,// 135 PAY 132 

    0x382ea45c,// 136 PAY 133 

    0xe2efd7b3,// 137 PAY 134 

    0xae393e88,// 138 PAY 135 

    0xf0d611d5,// 139 PAY 136 

    0x5c58117c,// 140 PAY 137 

    0x4976c2e2,// 141 PAY 138 

    0x917ebf4a,// 142 PAY 139 

    0x0481f5e8,// 143 PAY 140 

    0x24fa4c03,// 144 PAY 141 

    0xd36d5b5c,// 145 PAY 142 

    0x1abe0bb9,// 146 PAY 143 

    0x36617332,// 147 PAY 144 

    0xde1a5bf5,// 148 PAY 145 

    0x46c540c3,// 149 PAY 146 

    0xafa35254,// 150 PAY 147 

    0x7c7b7e59,// 151 PAY 148 

    0x9fd0e49f,// 152 PAY 149 

    0x9e6691aa,// 153 PAY 150 

    0x900fd842,// 154 PAY 151 

    0x4014392e,// 155 PAY 152 

    0x51148d5d,// 156 PAY 153 

    0x285556f9,// 157 PAY 154 

    0xf52a6697,// 158 PAY 155 

    0xaaa92d91,// 159 PAY 156 

    0xa4f0bd91,// 160 PAY 157 

    0x9fb46dda,// 161 PAY 158 

    0x6a3613ca,// 162 PAY 159 

    0x0e84d81a,// 163 PAY 160 

    0x01ee3ad5,// 164 PAY 161 

    0x5e593379,// 165 PAY 162 

    0x53382346,// 166 PAY 163 

    0xe2191127,// 167 PAY 164 

    0x83e9c57b,// 168 PAY 165 

    0x04fe0c60,// 169 PAY 166 

/// HASH is  12 bytes 

    0x900fd842,// 170 HSH   1 

    0x4014392e,// 171 HSH   2 

    0x51148d5d,// 172 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 218 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x25 

    0x03682553 // 173 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt64_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 138 words. 

/// BDA size     is 545 (0x221) 

/// BDA id       is 0x770a 

    0x0221770a,// 3 BDA   1 

/// PAY Generic Data size   : 545 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x849b0bcf,// 4 PAY   1 

    0xc5fbde3c,// 5 PAY   2 

    0x550eb7b4,// 6 PAY   3 

    0x5e0e8faf,// 7 PAY   4 

    0x7b02df0e,// 8 PAY   5 

    0x491496a7,// 9 PAY   6 

    0xf0da445d,// 10 PAY   7 

    0xf7ba2066,// 11 PAY   8 

    0xe145a31d,// 12 PAY   9 

    0x02db19e0,// 13 PAY  10 

    0x7c316baa,// 14 PAY  11 

    0xbe14d815,// 15 PAY  12 

    0x61e5f84a,// 16 PAY  13 

    0x0deabcf7,// 17 PAY  14 

    0x2c5545df,// 18 PAY  15 

    0xf6fd771c,// 19 PAY  16 

    0x5ec99562,// 20 PAY  17 

    0x73b178f8,// 21 PAY  18 

    0xaf448ec5,// 22 PAY  19 

    0x6bf01503,// 23 PAY  20 

    0xf02c5462,// 24 PAY  21 

    0x19733cca,// 25 PAY  22 

    0xc2e84d71,// 26 PAY  23 

    0x4e015165,// 27 PAY  24 

    0xae608a64,// 28 PAY  25 

    0x046b6bdc,// 29 PAY  26 

    0x03eca4a0,// 30 PAY  27 

    0xbc01920e,// 31 PAY  28 

    0x51e696e8,// 32 PAY  29 

    0x07bcac02,// 33 PAY  30 

    0x313e4714,// 34 PAY  31 

    0x15a5a4f0,// 35 PAY  32 

    0x825991ce,// 36 PAY  33 

    0x52be061e,// 37 PAY  34 

    0x2d41272b,// 38 PAY  35 

    0xcd57b6a4,// 39 PAY  36 

    0x97094558,// 40 PAY  37 

    0xb9d6a156,// 41 PAY  38 

    0x60909b34,// 42 PAY  39 

    0xfac89954,// 43 PAY  40 

    0x9a807624,// 44 PAY  41 

    0x53041191,// 45 PAY  42 

    0xdf004a14,// 46 PAY  43 

    0x5f4b5fd6,// 47 PAY  44 

    0xfa3868dd,// 48 PAY  45 

    0xf707879c,// 49 PAY  46 

    0xdbce37f5,// 50 PAY  47 

    0x75c67f28,// 51 PAY  48 

    0x1adb3ceb,// 52 PAY  49 

    0xa399d93c,// 53 PAY  50 

    0xee16131e,// 54 PAY  51 

    0x159b5d66,// 55 PAY  52 

    0x963dd9ad,// 56 PAY  53 

    0x5c4c73af,// 57 PAY  54 

    0x8864f8ab,// 58 PAY  55 

    0xb1b3b2c7,// 59 PAY  56 

    0xc6c887f8,// 60 PAY  57 

    0x30a2c6ce,// 61 PAY  58 

    0x3c3c7bf6,// 62 PAY  59 

    0x22b60f10,// 63 PAY  60 

    0xe603f780,// 64 PAY  61 

    0x51863e3b,// 65 PAY  62 

    0xe8d19e1f,// 66 PAY  63 

    0x4a923399,// 67 PAY  64 

    0x8e141f08,// 68 PAY  65 

    0x97e10788,// 69 PAY  66 

    0x21e5aebd,// 70 PAY  67 

    0xc1bc9ae3,// 71 PAY  68 

    0x78f2c5b2,// 72 PAY  69 

    0x92aad564,// 73 PAY  70 

    0x1f41c1b4,// 74 PAY  71 

    0xa822231f,// 75 PAY  72 

    0x3f021c0d,// 76 PAY  73 

    0x6abc56fd,// 77 PAY  74 

    0xd0431fac,// 78 PAY  75 

    0x8bb71441,// 79 PAY  76 

    0x0302c2a9,// 80 PAY  77 

    0x4fbed9b3,// 81 PAY  78 

    0x7e64b87f,// 82 PAY  79 

    0xa2ace290,// 83 PAY  80 

    0x9a9c5100,// 84 PAY  81 

    0x5c422b68,// 85 PAY  82 

    0x5b881e1f,// 86 PAY  83 

    0xc2619e8f,// 87 PAY  84 

    0x0a770239,// 88 PAY  85 

    0x6858fcfb,// 89 PAY  86 

    0xafd162d6,// 90 PAY  87 

    0x8ec34452,// 91 PAY  88 

    0xbcce85d8,// 92 PAY  89 

    0x77c1b7cd,// 93 PAY  90 

    0x0e9a4f21,// 94 PAY  91 

    0x28c3d7f0,// 95 PAY  92 

    0xdf3b9cfe,// 96 PAY  93 

    0x5c370029,// 97 PAY  94 

    0x62901b66,// 98 PAY  95 

    0xa05f358a,// 99 PAY  96 

    0x7b5bc37b,// 100 PAY  97 

    0xb1d192a7,// 101 PAY  98 

    0xf2ae481a,// 102 PAY  99 

    0x23455cce,// 103 PAY 100 

    0x49707280,// 104 PAY 101 

    0x87c0496f,// 105 PAY 102 

    0x37bbebf6,// 106 PAY 103 

    0xa79c743b,// 107 PAY 104 

    0x01893203,// 108 PAY 105 

    0x638db109,// 109 PAY 106 

    0x173fb2de,// 110 PAY 107 

    0x25296538,// 111 PAY 108 

    0x589d3279,// 112 PAY 109 

    0x2dd8261b,// 113 PAY 110 

    0xecdd88b6,// 114 PAY 111 

    0x3b91ee07,// 115 PAY 112 

    0xd1f3bd82,// 116 PAY 113 

    0x769ac3c3,// 117 PAY 114 

    0x6afdef3c,// 118 PAY 115 

    0x2efc44da,// 119 PAY 116 

    0x342a8f39,// 120 PAY 117 

    0x640b054a,// 121 PAY 118 

    0x6833da95,// 122 PAY 119 

    0x55704c16,// 123 PAY 120 

    0x1473e804,// 124 PAY 121 

    0xcc4634f7,// 125 PAY 122 

    0x1bc24df0,// 126 PAY 123 

    0x616e5ed1,// 127 PAY 124 

    0x983d03f9,// 128 PAY 125 

    0xbe3e07ad,// 129 PAY 126 

    0xce372a2c,// 130 PAY 127 

    0x4270a9e3,// 131 PAY 128 

    0x40136ff2,// 132 PAY 129 

    0x2d9accc8,// 133 PAY 130 

    0xb33e6af3,// 134 PAY 131 

    0x12d769f5,// 135 PAY 132 

    0xc223e967,// 136 PAY 133 

    0x3f96c765,// 137 PAY 134 

    0xa5dcfdbd,// 138 PAY 135 

    0xdf253a4a,// 139 PAY 136 

    0x66000000,// 140 PAY 137 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 131 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbb 

    0x020cbb33 // 141 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt65_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 455 words. 

/// BDA size     is 1816 (0x718) 

/// BDA id       is 0x5fa4 

    0x07185fa4,// 3 BDA   1 

/// PAY Generic Data size   : 1816 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xbfd9397f,// 4 PAY   1 

    0x6d04d6ed,// 5 PAY   2 

    0xee156db3,// 6 PAY   3 

    0xc6b3d068,// 7 PAY   4 

    0xf0586530,// 8 PAY   5 

    0x107d5ca5,// 9 PAY   6 

    0x350bed4c,// 10 PAY   7 

    0x9712c11d,// 11 PAY   8 

    0xf1933926,// 12 PAY   9 

    0x43d3391c,// 13 PAY  10 

    0x2874cdae,// 14 PAY  11 

    0x06fb1d5e,// 15 PAY  12 

    0x658dd663,// 16 PAY  13 

    0x12f18691,// 17 PAY  14 

    0xe1e83b96,// 18 PAY  15 

    0xac41b903,// 19 PAY  16 

    0xa634229f,// 20 PAY  17 

    0x9fc9ff1b,// 21 PAY  18 

    0x6ee412c3,// 22 PAY  19 

    0xd8ac86fd,// 23 PAY  20 

    0x13692168,// 24 PAY  21 

    0x7f7865e0,// 25 PAY  22 

    0xd6bd6e2a,// 26 PAY  23 

    0x500ccb23,// 27 PAY  24 

    0xa2f5eb38,// 28 PAY  25 

    0x9ce96c9f,// 29 PAY  26 

    0x36db1e76,// 30 PAY  27 

    0x301189a2,// 31 PAY  28 

    0x3c6bcc78,// 32 PAY  29 

    0x8da09311,// 33 PAY  30 

    0xd320dfdc,// 34 PAY  31 

    0x1948ee69,// 35 PAY  32 

    0xd4e8c936,// 36 PAY  33 

    0xba918cf6,// 37 PAY  34 

    0xd83f236f,// 38 PAY  35 

    0x720bbb66,// 39 PAY  36 

    0xc8f08686,// 40 PAY  37 

    0x40254c85,// 41 PAY  38 

    0xf60ed8e5,// 42 PAY  39 

    0x8dafbaea,// 43 PAY  40 

    0x056a0b9d,// 44 PAY  41 

    0xdb03feac,// 45 PAY  42 

    0x8fc113d4,// 46 PAY  43 

    0xb5966d3a,// 47 PAY  44 

    0xbdfdc0ff,// 48 PAY  45 

    0xace19ea0,// 49 PAY  46 

    0x5069d9bb,// 50 PAY  47 

    0xd05995db,// 51 PAY  48 

    0x126eff2d,// 52 PAY  49 

    0x2f97c239,// 53 PAY  50 

    0x752a1ee3,// 54 PAY  51 

    0xb1896c54,// 55 PAY  52 

    0x95fbd1d3,// 56 PAY  53 

    0x89f6d493,// 57 PAY  54 

    0x5ab83eec,// 58 PAY  55 

    0x86f48574,// 59 PAY  56 

    0xf0fe39ad,// 60 PAY  57 

    0x9c6c514b,// 61 PAY  58 

    0x5960b42e,// 62 PAY  59 

    0xc15ddc68,// 63 PAY  60 

    0x93d097cc,// 64 PAY  61 

    0x8c0a02f8,// 65 PAY  62 

    0x99a20910,// 66 PAY  63 

    0x95f6f49a,// 67 PAY  64 

    0x613c7fcf,// 68 PAY  65 

    0xd24368f1,// 69 PAY  66 

    0xd1b6112f,// 70 PAY  67 

    0x9dfcb027,// 71 PAY  68 

    0xbc8519bc,// 72 PAY  69 

    0xf2f3ab07,// 73 PAY  70 

    0xfc908edf,// 74 PAY  71 

    0xa472cf5c,// 75 PAY  72 

    0xeee49859,// 76 PAY  73 

    0xbfb8f865,// 77 PAY  74 

    0xc0362c5e,// 78 PAY  75 

    0xc08c0e90,// 79 PAY  76 

    0x8f738b53,// 80 PAY  77 

    0xfcdd7ee8,// 81 PAY  78 

    0x8a36879b,// 82 PAY  79 

    0x28de31da,// 83 PAY  80 

    0x1dbc9ef8,// 84 PAY  81 

    0x6a88ba7d,// 85 PAY  82 

    0xff3929a3,// 86 PAY  83 

    0xf2b4a4a4,// 87 PAY  84 

    0x39968a66,// 88 PAY  85 

    0x667c774e,// 89 PAY  86 

    0xbce425cf,// 90 PAY  87 

    0x19ee2e71,// 91 PAY  88 

    0x9d40b322,// 92 PAY  89 

    0x285f68a7,// 93 PAY  90 

    0xa368a411,// 94 PAY  91 

    0x71a4e664,// 95 PAY  92 

    0xb1f4bcb7,// 96 PAY  93 

    0x22a9bcba,// 97 PAY  94 

    0xccc93608,// 98 PAY  95 

    0xa737aa1d,// 99 PAY  96 

    0x0562fbbc,// 100 PAY  97 

    0xb851cd93,// 101 PAY  98 

    0x94989f55,// 102 PAY  99 

    0xc1d0c72a,// 103 PAY 100 

    0x84114788,// 104 PAY 101 

    0xa1faa567,// 105 PAY 102 

    0x5d812d80,// 106 PAY 103 

    0xba78fbb0,// 107 PAY 104 

    0x3e938c2c,// 108 PAY 105 

    0x62aa1334,// 109 PAY 106 

    0xda850d93,// 110 PAY 107 

    0xab488022,// 111 PAY 108 

    0x73e3e90e,// 112 PAY 109 

    0x19d898ca,// 113 PAY 110 

    0xcf94c55c,// 114 PAY 111 

    0x9c7d7d7b,// 115 PAY 112 

    0x9998307f,// 116 PAY 113 

    0xfca893cd,// 117 PAY 114 

    0x575dc9d6,// 118 PAY 115 

    0xcd72645d,// 119 PAY 116 

    0x0d191124,// 120 PAY 117 

    0xf5ca1ce3,// 121 PAY 118 

    0xf996f2d0,// 122 PAY 119 

    0x3370df6d,// 123 PAY 120 

    0xf113b63e,// 124 PAY 121 

    0xee4ad4b8,// 125 PAY 122 

    0x8ac7ad07,// 126 PAY 123 

    0x50fff347,// 127 PAY 124 

    0x3cde91ec,// 128 PAY 125 

    0xac10a702,// 129 PAY 126 

    0xd73f3f54,// 130 PAY 127 

    0x9b66fc60,// 131 PAY 128 

    0xa5da83b9,// 132 PAY 129 

    0x58cdaab5,// 133 PAY 130 

    0x1b17cfba,// 134 PAY 131 

    0x6d6067ab,// 135 PAY 132 

    0xbb282136,// 136 PAY 133 

    0xf0187f3f,// 137 PAY 134 

    0x4b8db4cb,// 138 PAY 135 

    0x5e88c1ec,// 139 PAY 136 

    0xfe6c3ecc,// 140 PAY 137 

    0xaf3efef8,// 141 PAY 138 

    0x19d1a1e1,// 142 PAY 139 

    0x8c85173e,// 143 PAY 140 

    0xe4cd2c8c,// 144 PAY 141 

    0xebb746e9,// 145 PAY 142 

    0xbc2e7ca9,// 146 PAY 143 

    0xf4c7c123,// 147 PAY 144 

    0x5310d75c,// 148 PAY 145 

    0xc299c910,// 149 PAY 146 

    0x9c760877,// 150 PAY 147 

    0x862d8817,// 151 PAY 148 

    0xf3c50a50,// 152 PAY 149 

    0xac2845b4,// 153 PAY 150 

    0x5bb8c238,// 154 PAY 151 

    0x3bdfee3f,// 155 PAY 152 

    0x677a75ab,// 156 PAY 153 

    0x78a8c9b5,// 157 PAY 154 

    0x8de39588,// 158 PAY 155 

    0xa6a45847,// 159 PAY 156 

    0xce31358a,// 160 PAY 157 

    0x5662bf6a,// 161 PAY 158 

    0x5dff60bf,// 162 PAY 159 

    0x5e6c1d73,// 163 PAY 160 

    0x0976b9cb,// 164 PAY 161 

    0x725b7da1,// 165 PAY 162 

    0x9edf4407,// 166 PAY 163 

    0x3a183ece,// 167 PAY 164 

    0xcb4098a9,// 168 PAY 165 

    0xcdcd3137,// 169 PAY 166 

    0x11eb6029,// 170 PAY 167 

    0xf154bd00,// 171 PAY 168 

    0xf23392ee,// 172 PAY 169 

    0x22c08c3c,// 173 PAY 170 

    0xcdc6320f,// 174 PAY 171 

    0x04bd868a,// 175 PAY 172 

    0xb750ecdb,// 176 PAY 173 

    0x4b6086be,// 177 PAY 174 

    0xcc5e4f73,// 178 PAY 175 

    0xb2e9e0ab,// 179 PAY 176 

    0x1755aa9d,// 180 PAY 177 

    0x0ac6eaeb,// 181 PAY 178 

    0x94c3770b,// 182 PAY 179 

    0x86cb6e81,// 183 PAY 180 

    0xb8cd8e4f,// 184 PAY 181 

    0x719efe6c,// 185 PAY 182 

    0xa58cfe9b,// 186 PAY 183 

    0x04fd16fe,// 187 PAY 184 

    0xae28e062,// 188 PAY 185 

    0x80d1b8dc,// 189 PAY 186 

    0x7f2e3748,// 190 PAY 187 

    0xe2850d58,// 191 PAY 188 

    0x14dd0e3b,// 192 PAY 189 

    0xedd78376,// 193 PAY 190 

    0x13200c6c,// 194 PAY 191 

    0x8fae6e7e,// 195 PAY 192 

    0x8cf7c6d2,// 196 PAY 193 

    0xc2b7917b,// 197 PAY 194 

    0x3def94f5,// 198 PAY 195 

    0x64292426,// 199 PAY 196 

    0x69308a20,// 200 PAY 197 

    0xdbf5468a,// 201 PAY 198 

    0x11060698,// 202 PAY 199 

    0xbd98a504,// 203 PAY 200 

    0x32856b2a,// 204 PAY 201 

    0x82512337,// 205 PAY 202 

    0x631cd115,// 206 PAY 203 

    0xa3829d5f,// 207 PAY 204 

    0x7210f4ea,// 208 PAY 205 

    0xbbcd4b7f,// 209 PAY 206 

    0x921ec853,// 210 PAY 207 

    0xef599a2e,// 211 PAY 208 

    0xd8d32fc7,// 212 PAY 209 

    0x5c317332,// 213 PAY 210 

    0xdc0de57f,// 214 PAY 211 

    0x25589b45,// 215 PAY 212 

    0xe9408766,// 216 PAY 213 

    0xc538b8f5,// 217 PAY 214 

    0x643968e3,// 218 PAY 215 

    0x074dd7fe,// 219 PAY 216 

    0xbda0d605,// 220 PAY 217 

    0x951653e1,// 221 PAY 218 

    0xacfe4281,// 222 PAY 219 

    0x504095f8,// 223 PAY 220 

    0xcc2bf28e,// 224 PAY 221 

    0x807ad5d0,// 225 PAY 222 

    0x1f9ac667,// 226 PAY 223 

    0xc86dce25,// 227 PAY 224 

    0x546ce476,// 228 PAY 225 

    0xa4a0e82f,// 229 PAY 226 

    0xab0ee756,// 230 PAY 227 

    0xb1835b2f,// 231 PAY 228 

    0xce58dc28,// 232 PAY 229 

    0xf51a0485,// 233 PAY 230 

    0x5f2cfa9f,// 234 PAY 231 

    0xe52acb63,// 235 PAY 232 

    0x8bbab1db,// 236 PAY 233 

    0xa47f62e4,// 237 PAY 234 

    0xd8e451be,// 238 PAY 235 

    0x7a5dc3cc,// 239 PAY 236 

    0x21d01003,// 240 PAY 237 

    0x2bcc46a2,// 241 PAY 238 

    0xcdd1e62e,// 242 PAY 239 

    0x698ce63f,// 243 PAY 240 

    0xe12c6916,// 244 PAY 241 

    0x97815667,// 245 PAY 242 

    0x338d115d,// 246 PAY 243 

    0x5e39e127,// 247 PAY 244 

    0xf0137165,// 248 PAY 245 

    0x4c927e3a,// 249 PAY 246 

    0x6229763b,// 250 PAY 247 

    0x23e6729b,// 251 PAY 248 

    0xfc1baf54,// 252 PAY 249 

    0x0ec882e6,// 253 PAY 250 

    0x04dd45e9,// 254 PAY 251 

    0x8a00b89c,// 255 PAY 252 

    0xb467c55f,// 256 PAY 253 

    0x97cc827b,// 257 PAY 254 

    0x6cb49af5,// 258 PAY 255 

    0x9420ddff,// 259 PAY 256 

    0x483673d7,// 260 PAY 257 

    0x0292ca91,// 261 PAY 258 

    0x7ffe2232,// 262 PAY 259 

    0x0da3e6af,// 263 PAY 260 

    0x44fdd90f,// 264 PAY 261 

    0xfe80eda8,// 265 PAY 262 

    0xf8774d66,// 266 PAY 263 

    0x9b58d42f,// 267 PAY 264 

    0xe813eaf9,// 268 PAY 265 

    0x92925096,// 269 PAY 266 

    0xaa453f90,// 270 PAY 267 

    0x0d26fa07,// 271 PAY 268 

    0x2831ace5,// 272 PAY 269 

    0x7feae921,// 273 PAY 270 

    0x1956d12f,// 274 PAY 271 

    0x8b4e5dc6,// 275 PAY 272 

    0x74a7bb57,// 276 PAY 273 

    0xba364712,// 277 PAY 274 

    0x441bfbdb,// 278 PAY 275 

    0xecb157d8,// 279 PAY 276 

    0x28f3ca5a,// 280 PAY 277 

    0x603057fd,// 281 PAY 278 

    0x3350180e,// 282 PAY 279 

    0x66bb53d5,// 283 PAY 280 

    0x7c24ae5e,// 284 PAY 281 

    0x809e3794,// 285 PAY 282 

    0xe119fa65,// 286 PAY 283 

    0xd03fef41,// 287 PAY 284 

    0xfde217af,// 288 PAY 285 

    0xb227efb5,// 289 PAY 286 

    0x702de9fe,// 290 PAY 287 

    0x2533aa55,// 291 PAY 288 

    0xdc6e2666,// 292 PAY 289 

    0xb9a5911a,// 293 PAY 290 

    0xb36e2685,// 294 PAY 291 

    0xd5781efa,// 295 PAY 292 

    0xed9264db,// 296 PAY 293 

    0x0dd615e4,// 297 PAY 294 

    0xde487623,// 298 PAY 295 

    0xbb2a1fd7,// 299 PAY 296 

    0xa29f902a,// 300 PAY 297 

    0xb61666ac,// 301 PAY 298 

    0x40958204,// 302 PAY 299 

    0xa5f15d01,// 303 PAY 300 

    0x51c7a2cf,// 304 PAY 301 

    0x4451a5e4,// 305 PAY 302 

    0x81c2c4a6,// 306 PAY 303 

    0x20b66d68,// 307 PAY 304 

    0xb6fd4ab9,// 308 PAY 305 

    0xc5b1df31,// 309 PAY 306 

    0x7f597e6f,// 310 PAY 307 

    0x958ed55b,// 311 PAY 308 

    0x8283bf05,// 312 PAY 309 

    0x305b07d7,// 313 PAY 310 

    0xacc4ac7b,// 314 PAY 311 

    0x5a102c1a,// 315 PAY 312 

    0x2f317eca,// 316 PAY 313 

    0x476ebec1,// 317 PAY 314 

    0xc4cdeae4,// 318 PAY 315 

    0x9ce9379e,// 319 PAY 316 

    0x5ba5744c,// 320 PAY 317 

    0x2f9e8660,// 321 PAY 318 

    0x625abb80,// 322 PAY 319 

    0xf7c95a16,// 323 PAY 320 

    0x668984a0,// 324 PAY 321 

    0x6a0df220,// 325 PAY 322 

    0x174f2da4,// 326 PAY 323 

    0x230b3b7b,// 327 PAY 324 

    0xe875f6d4,// 328 PAY 325 

    0x2e6f0cb9,// 329 PAY 326 

    0x4df9b04f,// 330 PAY 327 

    0xff09c357,// 331 PAY 328 

    0xe542a357,// 332 PAY 329 

    0xc6d69e9c,// 333 PAY 330 

    0xd110b587,// 334 PAY 331 

    0x9939fa84,// 335 PAY 332 

    0x72d8be95,// 336 PAY 333 

    0x9409c231,// 337 PAY 334 

    0x1eb59329,// 338 PAY 335 

    0x50ebc1b2,// 339 PAY 336 

    0x3f67c727,// 340 PAY 337 

    0x15ebf306,// 341 PAY 338 

    0x811a1944,// 342 PAY 339 

    0x0ddffac3,// 343 PAY 340 

    0xd0b8c3fa,// 344 PAY 341 

    0xd9203f33,// 345 PAY 342 

    0x4126c8ab,// 346 PAY 343 

    0xacbc17b3,// 347 PAY 344 

    0x275bacc1,// 348 PAY 345 

    0x2bb5138e,// 349 PAY 346 

    0x720b2ebb,// 350 PAY 347 

    0xb39d2a08,// 351 PAY 348 

    0xe37889a8,// 352 PAY 349 

    0xac178152,// 353 PAY 350 

    0xcba8a454,// 354 PAY 351 

    0xaa6e06ea,// 355 PAY 352 

    0x3b9957bb,// 356 PAY 353 

    0x6e54741b,// 357 PAY 354 

    0x19668985,// 358 PAY 355 

    0xd6c80017,// 359 PAY 356 

    0x91794403,// 360 PAY 357 

    0xc4725b1e,// 361 PAY 358 

    0x394c3b03,// 362 PAY 359 

    0x22daa953,// 363 PAY 360 

    0xb8981fad,// 364 PAY 361 

    0x1eecb365,// 365 PAY 362 

    0x28f0f3e0,// 366 PAY 363 

    0x34136528,// 367 PAY 364 

    0x66c1b337,// 368 PAY 365 

    0x31d6adb1,// 369 PAY 366 

    0x6375eca1,// 370 PAY 367 

    0xbac0a31f,// 371 PAY 368 

    0xe95a25d8,// 372 PAY 369 

    0x5ae149c1,// 373 PAY 370 

    0x626ebe48,// 374 PAY 371 

    0xbfc2a4c0,// 375 PAY 372 

    0x4de886f3,// 376 PAY 373 

    0x403fc764,// 377 PAY 374 

    0x1df0dc02,// 378 PAY 375 

    0x820d411b,// 379 PAY 376 

    0xe4a43fa9,// 380 PAY 377 

    0x7be0d1ee,// 381 PAY 378 

    0xc0f29671,// 382 PAY 379 

    0x7a65f2f2,// 383 PAY 380 

    0x4cf8c269,// 384 PAY 381 

    0x5004ae87,// 385 PAY 382 

    0xdb047f75,// 386 PAY 383 

    0xe39416cb,// 387 PAY 384 

    0x0ce1de85,// 388 PAY 385 

    0x1bc2733f,// 389 PAY 386 

    0xa8b33c78,// 390 PAY 387 

    0x19186a7b,// 391 PAY 388 

    0x160ad993,// 392 PAY 389 

    0xcd7e5a53,// 393 PAY 390 

    0x10860e25,// 394 PAY 391 

    0xeb784f10,// 395 PAY 392 

    0x8b9ce717,// 396 PAY 393 

    0xd82fb8bf,// 397 PAY 394 

    0x227f3794,// 398 PAY 395 

    0xd9b0a02a,// 399 PAY 396 

    0xec98b5a1,// 400 PAY 397 

    0x0717eede,// 401 PAY 398 

    0x2039556c,// 402 PAY 399 

    0x136cf73a,// 403 PAY 400 

    0x9c1cb330,// 404 PAY 401 

    0x09b36b23,// 405 PAY 402 

    0x37633550,// 406 PAY 403 

    0xf1d2cdbf,// 407 PAY 404 

    0x6bb62cd6,// 408 PAY 405 

    0xf1b52bca,// 409 PAY 406 

    0x94c2fa72,// 410 PAY 407 

    0x777d6851,// 411 PAY 408 

    0x18fdc77d,// 412 PAY 409 

    0xb70d6978,// 413 PAY 410 

    0x42666066,// 414 PAY 411 

    0xf02b166f,// 415 PAY 412 

    0xf9792596,// 416 PAY 413 

    0xbac941b6,// 417 PAY 414 

    0x6b595863,// 418 PAY 415 

    0x9352c8b4,// 419 PAY 416 

    0x178ae6d2,// 420 PAY 417 

    0xc144aa6d,// 421 PAY 418 

    0x882a3745,// 422 PAY 419 

    0xe00ab5f4,// 423 PAY 420 

    0xe1300040,// 424 PAY 421 

    0xf1e5a1e3,// 425 PAY 422 

    0x4f3964a5,// 426 PAY 423 

    0x4b282b63,// 427 PAY 424 

    0x0ad39ddc,// 428 PAY 425 

    0x97cea0b2,// 429 PAY 426 

    0xaff4aec5,// 430 PAY 427 

    0x258ff127,// 431 PAY 428 

    0x6730ced8,// 432 PAY 429 

    0x9f964874,// 433 PAY 430 

    0x5e240f0d,// 434 PAY 431 

    0x5692f505,// 435 PAY 432 

    0x976ba2d1,// 436 PAY 433 

    0xa5973739,// 437 PAY 434 

    0xffcad24c,// 438 PAY 435 

    0xbac6aac8,// 439 PAY 436 

    0x1afa6510,// 440 PAY 437 

    0x65824008,// 441 PAY 438 

    0xf25af4ba,// 442 PAY 439 

    0x8af86ca7,// 443 PAY 440 

    0xdebfe2b8,// 444 PAY 441 

    0x49ebcba6,// 445 PAY 442 

    0xfe6cc425,// 446 PAY 443 

    0x577ec22c,// 447 PAY 444 

    0x762feadb,// 448 PAY 445 

    0x3ef15f1c,// 449 PAY 446 

    0xab39a1bc,// 450 PAY 447 

    0x7eafc247,// 451 PAY 448 

    0x8a60ec84,// 452 PAY 449 

    0x82e3947f,// 453 PAY 450 

    0x280933f1,// 454 PAY 451 

    0x559f3f0a,// 455 PAY 452 

    0x7a660f52,// 456 PAY 453 

    0x266cd26d,// 457 PAY 454 

/// HASH is  20 bytes 

    0xfe6cc425,// 458 HSH   1 

    0x577ec22c,// 459 HSH   2 

    0x762feadb,// 460 HSH   3 

    0x3ef15f1c,// 461 HSH   4 

    0xab39a1bc,// 462 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x45 

    0x00d44533 // 463 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt66_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 99 words. 

/// BDA size     is 390 (0x186) 

/// BDA id       is 0xb717 

    0x0186b717,// 3 BDA   1 

/// PAY Generic Data size   : 390 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x3ff158a8,// 4 PAY   1 

    0xe134f01d,// 5 PAY   2 

    0x51fd036b,// 6 PAY   3 

    0x803364d1,// 7 PAY   4 

    0xa0381744,// 8 PAY   5 

    0x96594988,// 9 PAY   6 

    0x0cead9c0,// 10 PAY   7 

    0x9f8f7033,// 11 PAY   8 

    0xa871c9bc,// 12 PAY   9 

    0x5655fc46,// 13 PAY  10 

    0x55cdbf54,// 14 PAY  11 

    0x5b2193fb,// 15 PAY  12 

    0xda924f83,// 16 PAY  13 

    0xff5fe806,// 17 PAY  14 

    0x010ce3b8,// 18 PAY  15 

    0x2f4e0d90,// 19 PAY  16 

    0x3050b54c,// 20 PAY  17 

    0xe5b7414e,// 21 PAY  18 

    0x75a39ec6,// 22 PAY  19 

    0xccfd1028,// 23 PAY  20 

    0xe324fdd8,// 24 PAY  21 

    0xbe925b09,// 25 PAY  22 

    0xb3356859,// 26 PAY  23 

    0x2981b477,// 27 PAY  24 

    0x712a3aa4,// 28 PAY  25 

    0x07394cfc,// 29 PAY  26 

    0x91aff1b3,// 30 PAY  27 

    0x3f4888cb,// 31 PAY  28 

    0x56403f25,// 32 PAY  29 

    0x9e92171e,// 33 PAY  30 

    0xefa85b9b,// 34 PAY  31 

    0x326fe355,// 35 PAY  32 

    0x6ceb6e20,// 36 PAY  33 

    0x778cb3e7,// 37 PAY  34 

    0xd36b9772,// 38 PAY  35 

    0x602891c6,// 39 PAY  36 

    0xc207658d,// 40 PAY  37 

    0x3d212ead,// 41 PAY  38 

    0xa7d619f2,// 42 PAY  39 

    0x2616e394,// 43 PAY  40 

    0xbfe7ad24,// 44 PAY  41 

    0x8e6d0bac,// 45 PAY  42 

    0x5d00317d,// 46 PAY  43 

    0x6f48a1f7,// 47 PAY  44 

    0xe2ce266b,// 48 PAY  45 

    0x4ff4a92b,// 49 PAY  46 

    0x1cfd7fd4,// 50 PAY  47 

    0xcb2cba90,// 51 PAY  48 

    0x4251e762,// 52 PAY  49 

    0x6db075a9,// 53 PAY  50 

    0xd6bbc8af,// 54 PAY  51 

    0x177e284b,// 55 PAY  52 

    0x75d3d8bb,// 56 PAY  53 

    0x899938d6,// 57 PAY  54 

    0x1f374566,// 58 PAY  55 

    0xcd5d78ef,// 59 PAY  56 

    0x1ad7c31a,// 60 PAY  57 

    0xefcafd26,// 61 PAY  58 

    0x4c8b7e68,// 62 PAY  59 

    0xa2c38a6a,// 63 PAY  60 

    0xa9a9ba39,// 64 PAY  61 

    0x964d0e83,// 65 PAY  62 

    0x7281077b,// 66 PAY  63 

    0x33a7509b,// 67 PAY  64 

    0xc187ca2a,// 68 PAY  65 

    0x531e7512,// 69 PAY  66 

    0x9510a407,// 70 PAY  67 

    0x626a7b3b,// 71 PAY  68 

    0xa9069a01,// 72 PAY  69 

    0xf0f01534,// 73 PAY  70 

    0x19841239,// 74 PAY  71 

    0x55af6c9e,// 75 PAY  72 

    0x0b1c1f67,// 76 PAY  73 

    0xd3af9e49,// 77 PAY  74 

    0x52e5f12f,// 78 PAY  75 

    0xf3f3c25c,// 79 PAY  76 

    0xf315d661,// 80 PAY  77 

    0xd1f168eb,// 81 PAY  78 

    0x48e3c554,// 82 PAY  79 

    0x774c698d,// 83 PAY  80 

    0x5b576c5f,// 84 PAY  81 

    0x024d824e,// 85 PAY  82 

    0xd8be758a,// 86 PAY  83 

    0xa9eeb0b0,// 87 PAY  84 

    0xc13a386f,// 88 PAY  85 

    0xcbf81839,// 89 PAY  86 

    0x1202e478,// 90 PAY  87 

    0xe52ed787,// 91 PAY  88 

    0x5cd180cf,// 92 PAY  89 

    0xe7f4ff41,// 93 PAY  90 

    0x4afb77e7,// 94 PAY  91 

    0xfa6c750a,// 95 PAY  92 

    0x85e86d16,// 96 PAY  93 

    0xc3075d99,// 97 PAY  94 

    0xcfe904bd,// 98 PAY  95 

    0x3b09a799,// 99 PAY  96 

    0x74365f0b,// 100 PAY  97 

    0x53cd0000,// 101 PAY  98 

/// HASH is  12 bytes 

    0x024d824e,// 102 HSH   1 

    0xd8be758a,// 103 HSH   2 

    0xa9eeb0b0,// 104 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 91 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x83 

    0x01e8835b // 105 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt67_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 138 words. 

/// BDA size     is 548 (0x224) 

/// BDA id       is 0xa522 

    0x0224a522,// 3 BDA   1 

/// PAY Generic Data size   : 548 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x46171e7e,// 4 PAY   1 

    0xa47bb997,// 5 PAY   2 

    0x3051d3bd,// 6 PAY   3 

    0x5c06709d,// 7 PAY   4 

    0xc46f8176,// 8 PAY   5 

    0x2492b0c5,// 9 PAY   6 

    0x9723028c,// 10 PAY   7 

    0x891ff451,// 11 PAY   8 

    0x1dccd13f,// 12 PAY   9 

    0x00eb2dd8,// 13 PAY  10 

    0x5ce548e0,// 14 PAY  11 

    0xb57fc470,// 15 PAY  12 

    0x1f5e45c0,// 16 PAY  13 

    0x0e3bd5b4,// 17 PAY  14 

    0x40f8c8c6,// 18 PAY  15 

    0x040b1128,// 19 PAY  16 

    0xd08ceee5,// 20 PAY  17 

    0x5ccb141d,// 21 PAY  18 

    0x6918058c,// 22 PAY  19 

    0x489f9869,// 23 PAY  20 

    0xe15f9111,// 24 PAY  21 

    0x63d8780c,// 25 PAY  22 

    0x8b7fe659,// 26 PAY  23 

    0xe614c8c6,// 27 PAY  24 

    0x0af6b299,// 28 PAY  25 

    0x649e241d,// 29 PAY  26 

    0x9c9e9a56,// 30 PAY  27 

    0xdfe47dc8,// 31 PAY  28 

    0xcab1e5bf,// 32 PAY  29 

    0xbe376eca,// 33 PAY  30 

    0x92f3d1c8,// 34 PAY  31 

    0x71f97929,// 35 PAY  32 

    0xdddfee3c,// 36 PAY  33 

    0x6d4e0042,// 37 PAY  34 

    0xdc0c1f71,// 38 PAY  35 

    0x9a679176,// 39 PAY  36 

    0xbf30fa37,// 40 PAY  37 

    0xf5fcdd84,// 41 PAY  38 

    0x02df1408,// 42 PAY  39 

    0xdafbc7b1,// 43 PAY  40 

    0xa3e9aa90,// 44 PAY  41 

    0x77a96aa2,// 45 PAY  42 

    0x0ea079a0,// 46 PAY  43 

    0x7deaa16f,// 47 PAY  44 

    0x0e112894,// 48 PAY  45 

    0x6e5f1e4f,// 49 PAY  46 

    0x1c7bb2a5,// 50 PAY  47 

    0xa22080b3,// 51 PAY  48 

    0x88ec7603,// 52 PAY  49 

    0x3f7ce6ef,// 53 PAY  50 

    0xf4fca6f1,// 54 PAY  51 

    0x9a435fc9,// 55 PAY  52 

    0x811a611e,// 56 PAY  53 

    0x3c73849c,// 57 PAY  54 

    0x25e8e5e7,// 58 PAY  55 

    0xd0f490c2,// 59 PAY  56 

    0x204c4745,// 60 PAY  57 

    0xb924c611,// 61 PAY  58 

    0x66c800af,// 62 PAY  59 

    0x456be0d6,// 63 PAY  60 

    0x18b37f44,// 64 PAY  61 

    0x57d5e50f,// 65 PAY  62 

    0x15aec892,// 66 PAY  63 

    0x2d71afac,// 67 PAY  64 

    0x5d985ef0,// 68 PAY  65 

    0x637f5695,// 69 PAY  66 

    0x80f89b91,// 70 PAY  67 

    0x3ee1533a,// 71 PAY  68 

    0xc19854d3,// 72 PAY  69 

    0xd0769645,// 73 PAY  70 

    0x2da4b5db,// 74 PAY  71 

    0xbee631e3,// 75 PAY  72 

    0xd91cf5bf,// 76 PAY  73 

    0xf3ce2b4a,// 77 PAY  74 

    0x3bee34ab,// 78 PAY  75 

    0xb320fa5b,// 79 PAY  76 

    0x1796ae18,// 80 PAY  77 

    0xb698f1c5,// 81 PAY  78 

    0x476b57e8,// 82 PAY  79 

    0x14653fcd,// 83 PAY  80 

    0xb3dc58b7,// 84 PAY  81 

    0x9ac96ce3,// 85 PAY  82 

    0x629787a7,// 86 PAY  83 

    0xf66568f0,// 87 PAY  84 

    0xa83ed914,// 88 PAY  85 

    0x45a9295c,// 89 PAY  86 

    0x8e768f10,// 90 PAY  87 

    0x497a473e,// 91 PAY  88 

    0xfe485afc,// 92 PAY  89 

    0xa0e50bbd,// 93 PAY  90 

    0x835a00ee,// 94 PAY  91 

    0x4f0e864f,// 95 PAY  92 

    0x99366bdf,// 96 PAY  93 

    0x4026a268,// 97 PAY  94 

    0xc21b383a,// 98 PAY  95 

    0x09d5ba9e,// 99 PAY  96 

    0x620913b3,// 100 PAY  97 

    0xbf05dfe2,// 101 PAY  98 

    0x75c8f7ea,// 102 PAY  99 

    0x983d1394,// 103 PAY 100 

    0x6e32a371,// 104 PAY 101 

    0x5d08ba9a,// 105 PAY 102 

    0x12e528e3,// 106 PAY 103 

    0x9f881c2a,// 107 PAY 104 

    0xca0664f8,// 108 PAY 105 

    0x1297bf68,// 109 PAY 106 

    0xb62d3eb3,// 110 PAY 107 

    0xb7db6827,// 111 PAY 108 

    0x76622e61,// 112 PAY 109 

    0x19861dee,// 113 PAY 110 

    0x8c2476a4,// 114 PAY 111 

    0xe9243794,// 115 PAY 112 

    0x9d86996c,// 116 PAY 113 

    0xe64f0a2d,// 117 PAY 114 

    0x0a4eb5f6,// 118 PAY 115 

    0x092333b0,// 119 PAY 116 

    0x2f009ce8,// 120 PAY 117 

    0x38460f8b,// 121 PAY 118 

    0x0879f970,// 122 PAY 119 

    0x3d68f0dc,// 123 PAY 120 

    0x0d3b02a2,// 124 PAY 121 

    0x55821923,// 125 PAY 122 

    0x2dd04c03,// 126 PAY 123 

    0xc7047fc2,// 127 PAY 124 

    0x5f4b9858,// 128 PAY 125 

    0x3b1920c3,// 129 PAY 126 

    0x26497a2e,// 130 PAY 127 

    0x3686955a,// 131 PAY 128 

    0x6b54016e,// 132 PAY 129 

    0x2e16e3cf,// 133 PAY 130 

    0x417e0d86,// 134 PAY 131 

    0xd2375ffc,// 135 PAY 132 

    0xf48292e3,// 136 PAY 133 

    0xf22d2879,// 137 PAY 134 

    0x8b344cbe,// 138 PAY 135 

    0x4fd3c65a,// 139 PAY 136 

    0xd1701901,// 140 PAY 137 

/// HASH is  8 bytes 

    0x2e16e3cf,// 141 HSH   1 

    0x417e0d86,// 142 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 221 

/// STA pkt_idx        : 35 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd4 

    0x008cd4dd // 143 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt68_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 233 words. 

/// BDA size     is 925 (0x39d) 

/// BDA id       is 0x7985 

    0x039d7985,// 3 BDA   1 

/// PAY Generic Data size   : 925 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xa80643cf,// 4 PAY   1 

    0xf59b8586,// 5 PAY   2 

    0x875480de,// 6 PAY   3 

    0xc8392996,// 7 PAY   4 

    0xa268d1de,// 8 PAY   5 

    0xcffb293f,// 9 PAY   6 

    0x0383b8b5,// 10 PAY   7 

    0xcd0f788a,// 11 PAY   8 

    0x46879c46,// 12 PAY   9 

    0x70c4d894,// 13 PAY  10 

    0x3020ce6b,// 14 PAY  11 

    0x6d49ff2e,// 15 PAY  12 

    0x22d33768,// 16 PAY  13 

    0xf079e834,// 17 PAY  14 

    0x6a862af7,// 18 PAY  15 

    0x2519fa9f,// 19 PAY  16 

    0x0403fd0f,// 20 PAY  17 

    0xba535edc,// 21 PAY  18 

    0x84fb404d,// 22 PAY  19 

    0xd6e94104,// 23 PAY  20 

    0xdc387a2a,// 24 PAY  21 

    0x7f1d2697,// 25 PAY  22 

    0x725893c1,// 26 PAY  23 

    0x462b0ece,// 27 PAY  24 

    0xab12d90f,// 28 PAY  25 

    0x694a31bb,// 29 PAY  26 

    0xfdb92830,// 30 PAY  27 

    0xea99238e,// 31 PAY  28 

    0xfaaab101,// 32 PAY  29 

    0xfd90757c,// 33 PAY  30 

    0xa7d51527,// 34 PAY  31 

    0xf9eef21c,// 35 PAY  32 

    0x03a2c37a,// 36 PAY  33 

    0xae0306eb,// 37 PAY  34 

    0x2564cb70,// 38 PAY  35 

    0x03ae1cfa,// 39 PAY  36 

    0x6ef32654,// 40 PAY  37 

    0xd61e1006,// 41 PAY  38 

    0x8c4a023a,// 42 PAY  39 

    0x247e4037,// 43 PAY  40 

    0x7e480c8a,// 44 PAY  41 

    0x5e6c6db0,// 45 PAY  42 

    0x3e758296,// 46 PAY  43 

    0xc74b4dd1,// 47 PAY  44 

    0xe957a5fb,// 48 PAY  45 

    0xd8e4a9fc,// 49 PAY  46 

    0x2ae8dd0b,// 50 PAY  47 

    0xc079da24,// 51 PAY  48 

    0x72bbfc8a,// 52 PAY  49 

    0x82ed0695,// 53 PAY  50 

    0x7b7a8687,// 54 PAY  51 

    0xf1c1baf9,// 55 PAY  52 

    0x740eae09,// 56 PAY  53 

    0xf019e21c,// 57 PAY  54 

    0x4489d392,// 58 PAY  55 

    0x1dc9b595,// 59 PAY  56 

    0x55b0ac3d,// 60 PAY  57 

    0xfda80d5f,// 61 PAY  58 

    0x1ddcdd4f,// 62 PAY  59 

    0x8847152e,// 63 PAY  60 

    0x10f9571b,// 64 PAY  61 

    0x395cc47a,// 65 PAY  62 

    0x3f87cdb7,// 66 PAY  63 

    0xed518d44,// 67 PAY  64 

    0x197a5f11,// 68 PAY  65 

    0x83e6d658,// 69 PAY  66 

    0x3ab47ce6,// 70 PAY  67 

    0x9cbe7201,// 71 PAY  68 

    0x38bcef26,// 72 PAY  69 

    0x5c9899ee,// 73 PAY  70 

    0x63c8cd34,// 74 PAY  71 

    0x07d6d454,// 75 PAY  72 

    0x18b4fbd2,// 76 PAY  73 

    0x6e54f1ac,// 77 PAY  74 

    0x78b10003,// 78 PAY  75 

    0x9c1bfe92,// 79 PAY  76 

    0x2220db58,// 80 PAY  77 

    0x1fb25112,// 81 PAY  78 

    0xaf1350bd,// 82 PAY  79 

    0x20f4a1f2,// 83 PAY  80 

    0xe880e737,// 84 PAY  81 

    0x506ee528,// 85 PAY  82 

    0x9709f76b,// 86 PAY  83 

    0xd1facfec,// 87 PAY  84 

    0x73096481,// 88 PAY  85 

    0xe5c739ed,// 89 PAY  86 

    0x5332fa59,// 90 PAY  87 

    0x62876828,// 91 PAY  88 

    0x92f1f303,// 92 PAY  89 

    0x79637ae3,// 93 PAY  90 

    0xe57aefe3,// 94 PAY  91 

    0xf2e90bfa,// 95 PAY  92 

    0xf48920d1,// 96 PAY  93 

    0xf2261e8e,// 97 PAY  94 

    0x06ac90fb,// 98 PAY  95 

    0xf3389ca4,// 99 PAY  96 

    0x2b493141,// 100 PAY  97 

    0xe275378f,// 101 PAY  98 

    0x9f44b688,// 102 PAY  99 

    0xd20462cb,// 103 PAY 100 

    0xe52eb83e,// 104 PAY 101 

    0xa27e106d,// 105 PAY 102 

    0x800fe20c,// 106 PAY 103 

    0xdb4189b6,// 107 PAY 104 

    0xfe452be9,// 108 PAY 105 

    0x6f01a475,// 109 PAY 106 

    0x3cea356f,// 110 PAY 107 

    0x7233cafa,// 111 PAY 108 

    0x0ff0b9c6,// 112 PAY 109 

    0x8f8e60ab,// 113 PAY 110 

    0x24714ba7,// 114 PAY 111 

    0xdc0697fe,// 115 PAY 112 

    0x8380bf92,// 116 PAY 113 

    0xba358085,// 117 PAY 114 

    0xa4dd7a1c,// 118 PAY 115 

    0xc29a9da7,// 119 PAY 116 

    0x01312085,// 120 PAY 117 

    0x9f18aadf,// 121 PAY 118 

    0x8f924406,// 122 PAY 119 

    0x81ab79af,// 123 PAY 120 

    0xca0efd89,// 124 PAY 121 

    0x2c3568a9,// 125 PAY 122 

    0x7f9da731,// 126 PAY 123 

    0xafbd3865,// 127 PAY 124 

    0x60220cf6,// 128 PAY 125 

    0x7191ead8,// 129 PAY 126 

    0xf1778e43,// 130 PAY 127 

    0x54704fff,// 131 PAY 128 

    0x8886b4c6,// 132 PAY 129 

    0x87062bb0,// 133 PAY 130 

    0x16787071,// 134 PAY 131 

    0xee1fefb9,// 135 PAY 132 

    0x311fde05,// 136 PAY 133 

    0x699bda3c,// 137 PAY 134 

    0x7ebb60ae,// 138 PAY 135 

    0xd44c3cad,// 139 PAY 136 

    0xb8a9ea69,// 140 PAY 137 

    0xfd67a068,// 141 PAY 138 

    0xa89e6325,// 142 PAY 139 

    0x25c0aba8,// 143 PAY 140 

    0x7026efd2,// 144 PAY 141 

    0xb87c8ef0,// 145 PAY 142 

    0xadc41a98,// 146 PAY 143 

    0x90b1d040,// 147 PAY 144 

    0xcfd156d9,// 148 PAY 145 

    0x20598222,// 149 PAY 146 

    0x8553f289,// 150 PAY 147 

    0xb70d46d7,// 151 PAY 148 

    0x24fae6fd,// 152 PAY 149 

    0xc91c07f9,// 153 PAY 150 

    0xd7021beb,// 154 PAY 151 

    0x154e5022,// 155 PAY 152 

    0xd167a9c7,// 156 PAY 153 

    0xa7322074,// 157 PAY 154 

    0x92d5d6a2,// 158 PAY 155 

    0xaa9e64fd,// 159 PAY 156 

    0xc0b64a48,// 160 PAY 157 

    0xe466719a,// 161 PAY 158 

    0x7568733f,// 162 PAY 159 

    0x22670696,// 163 PAY 160 

    0x463d9c66,// 164 PAY 161 

    0x46e824ac,// 165 PAY 162 

    0xcfaf32cd,// 166 PAY 163 

    0x52728a83,// 167 PAY 164 

    0xda19917e,// 168 PAY 165 

    0xf8d782f7,// 169 PAY 166 

    0x1c9f370d,// 170 PAY 167 

    0xee2f8891,// 171 PAY 168 

    0xdc4dfa28,// 172 PAY 169 

    0x15d9a557,// 173 PAY 170 

    0xaf7747d0,// 174 PAY 171 

    0xa8b854f6,// 175 PAY 172 

    0x634aceac,// 176 PAY 173 

    0xcc4e6971,// 177 PAY 174 

    0x8c79c1f7,// 178 PAY 175 

    0x122a2bff,// 179 PAY 176 

    0xa2d6f5cd,// 180 PAY 177 

    0xa6046036,// 181 PAY 178 

    0xab05865d,// 182 PAY 179 

    0x2b5dc5f2,// 183 PAY 180 

    0xd24118ae,// 184 PAY 181 

    0xc89d1408,// 185 PAY 182 

    0x9f889d83,// 186 PAY 183 

    0xbfc95864,// 187 PAY 184 

    0xa85ad47e,// 188 PAY 185 

    0x00519353,// 189 PAY 186 

    0xf0f67131,// 190 PAY 187 

    0xe898f752,// 191 PAY 188 

    0x84b29145,// 192 PAY 189 

    0x89e395fb,// 193 PAY 190 

    0x12d12a40,// 194 PAY 191 

    0x3e3b5045,// 195 PAY 192 

    0x1b819aa9,// 196 PAY 193 

    0xdfdd3fb2,// 197 PAY 194 

    0xb038e4e7,// 198 PAY 195 

    0x4af1c2ce,// 199 PAY 196 

    0x3eaa5454,// 200 PAY 197 

    0x8d87c8fb,// 201 PAY 198 

    0x92c58333,// 202 PAY 199 

    0x62e166d8,// 203 PAY 200 

    0xaf5e09d1,// 204 PAY 201 

    0xab980f01,// 205 PAY 202 

    0x0e5498ba,// 206 PAY 203 

    0xf4dd4325,// 207 PAY 204 

    0x82de3936,// 208 PAY 205 

    0xd2a2cd20,// 209 PAY 206 

    0x389737ee,// 210 PAY 207 

    0x013f44e8,// 211 PAY 208 

    0x8f4c7f2f,// 212 PAY 209 

    0x9e9e7669,// 213 PAY 210 

    0x90eef5d1,// 214 PAY 211 

    0x911be171,// 215 PAY 212 

    0xd13e73d9,// 216 PAY 213 

    0xb39961b2,// 217 PAY 214 

    0xf40257f5,// 218 PAY 215 

    0xa1fa7760,// 219 PAY 216 

    0xa27593fe,// 220 PAY 217 

    0x1392fe83,// 221 PAY 218 

    0x6d70a643,// 222 PAY 219 

    0xc1f0288c,// 223 PAY 220 

    0x2f82e4ce,// 224 PAY 221 

    0x759df7db,// 225 PAY 222 

    0x55435e6d,// 226 PAY 223 

    0xe21488a8,// 227 PAY 224 

    0x726a3d6d,// 228 PAY 225 

    0x6b0f417b,// 229 PAY 226 

    0x6490ef49,// 230 PAY 227 

    0x435d6722,// 231 PAY 228 

    0x52294bf1,// 232 PAY 229 

    0x17eadfeb,// 233 PAY 230 

    0xf5e893e0,// 234 PAY 231 

    0xd6000000,// 235 PAY 232 

/// HASH is  20 bytes 

    0x55435e6d,// 236 HSH   1 

    0xe21488a8,// 237 HSH   2 

    0x726a3d6d,// 238 HSH   3 

    0x6b0f417b,// 239 HSH   4 

    0x6490ef49,// 240 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 198 

/// STA pkt_idx        : 217 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9d 

    0x03659dc6 // 241 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt69_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 252 words. 

/// BDA size     is 1001 (0x3e9) 

/// BDA id       is 0xe834 

    0x03e9e834,// 3 BDA   1 

/// PAY Generic Data size   : 1001 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xe0cbd582,// 4 PAY   1 

    0x899f358b,// 5 PAY   2 

    0x77287992,// 6 PAY   3 

    0x9a8be002,// 7 PAY   4 

    0x7ef280e4,// 8 PAY   5 

    0xff1926ab,// 9 PAY   6 

    0xd241b86e,// 10 PAY   7 

    0xb277cacc,// 11 PAY   8 

    0x23b7182c,// 12 PAY   9 

    0xab05adc2,// 13 PAY  10 

    0xf6af014a,// 14 PAY  11 

    0xe8773a15,// 15 PAY  12 

    0xa76917f3,// 16 PAY  13 

    0x370d0225,// 17 PAY  14 

    0x224b76f1,// 18 PAY  15 

    0xec70922d,// 19 PAY  16 

    0x03f03973,// 20 PAY  17 

    0x69e0c98e,// 21 PAY  18 

    0xa5b11733,// 22 PAY  19 

    0xfd325efb,// 23 PAY  20 

    0xd6103059,// 24 PAY  21 

    0x7265d6bf,// 25 PAY  22 

    0xf066d8ef,// 26 PAY  23 

    0x0e39ee4d,// 27 PAY  24 

    0x8190daff,// 28 PAY  25 

    0x21880f02,// 29 PAY  26 

    0xbeda5f6a,// 30 PAY  27 

    0x1e4bfade,// 31 PAY  28 

    0x5b34237f,// 32 PAY  29 

    0xf9681429,// 33 PAY  30 

    0x71e81557,// 34 PAY  31 

    0x72239c35,// 35 PAY  32 

    0x5832f0e4,// 36 PAY  33 

    0x79d8b242,// 37 PAY  34 

    0x408f8288,// 38 PAY  35 

    0xbcc16df5,// 39 PAY  36 

    0x75ceb821,// 40 PAY  37 

    0x0b76d7c7,// 41 PAY  38 

    0x0affc07a,// 42 PAY  39 

    0x65358d8d,// 43 PAY  40 

    0xc46b0ac7,// 44 PAY  41 

    0x14462f43,// 45 PAY  42 

    0x836bc9b7,// 46 PAY  43 

    0x94067df0,// 47 PAY  44 

    0x53cf69e6,// 48 PAY  45 

    0x14cd8886,// 49 PAY  46 

    0xfe258da3,// 50 PAY  47 

    0xa3938772,// 51 PAY  48 

    0x6e5bfae0,// 52 PAY  49 

    0x7943758a,// 53 PAY  50 

    0x9143ef99,// 54 PAY  51 

    0x1bb70b28,// 55 PAY  52 

    0x69c23c78,// 56 PAY  53 

    0x13bde055,// 57 PAY  54 

    0x01b1b7d0,// 58 PAY  55 

    0xdd16643e,// 59 PAY  56 

    0x35e24993,// 60 PAY  57 

    0x9e2097a8,// 61 PAY  58 

    0x2aefc1bc,// 62 PAY  59 

    0xe83d4106,// 63 PAY  60 

    0x83ee2a62,// 64 PAY  61 

    0xef970178,// 65 PAY  62 

    0x8838ad4a,// 66 PAY  63 

    0x7c99fd4a,// 67 PAY  64 

    0x40e62d81,// 68 PAY  65 

    0xb74a9ad3,// 69 PAY  66 

    0x6b98bebc,// 70 PAY  67 

    0x54f1c59c,// 71 PAY  68 

    0xa4754fba,// 72 PAY  69 

    0x503c836c,// 73 PAY  70 

    0xd4677536,// 74 PAY  71 

    0xaf873c05,// 75 PAY  72 

    0x1258cdf5,// 76 PAY  73 

    0x0836b495,// 77 PAY  74 

    0x1c4c874b,// 78 PAY  75 

    0x9bb01696,// 79 PAY  76 

    0x74b4c2bd,// 80 PAY  77 

    0x3602ed03,// 81 PAY  78 

    0xb764d3a7,// 82 PAY  79 

    0x362bddbf,// 83 PAY  80 

    0x3c3f1961,// 84 PAY  81 

    0x10eec89d,// 85 PAY  82 

    0xff60380f,// 86 PAY  83 

    0x76a32d58,// 87 PAY  84 

    0x70717367,// 88 PAY  85 

    0xceb2ed54,// 89 PAY  86 

    0x318dc4c8,// 90 PAY  87 

    0xa23aa1f6,// 91 PAY  88 

    0x4fa683a3,// 92 PAY  89 

    0x504b1f01,// 93 PAY  90 

    0x91d454dd,// 94 PAY  91 

    0x413f2df0,// 95 PAY  92 

    0x57ec28c2,// 96 PAY  93 

    0x2b806b96,// 97 PAY  94 

    0x0a6b327a,// 98 PAY  95 

    0x264d75d3,// 99 PAY  96 

    0xad3ab59e,// 100 PAY  97 

    0x7fdc19d9,// 101 PAY  98 

    0x228c22b8,// 102 PAY  99 

    0xd9875fd2,// 103 PAY 100 

    0x42a9ada7,// 104 PAY 101 

    0x4d485edf,// 105 PAY 102 

    0x5f600df8,// 106 PAY 103 

    0x6ef97e41,// 107 PAY 104 

    0x32621879,// 108 PAY 105 

    0xd718f099,// 109 PAY 106 

    0x54c4e205,// 110 PAY 107 

    0xcc37bc67,// 111 PAY 108 

    0x51eb9a28,// 112 PAY 109 

    0x60052a84,// 113 PAY 110 

    0xf23b16c9,// 114 PAY 111 

    0x8181c92c,// 115 PAY 112 

    0x423c6460,// 116 PAY 113 

    0xc20e0cee,// 117 PAY 114 

    0xfa96538d,// 118 PAY 115 

    0x3bc45fe5,// 119 PAY 116 

    0x6c3a42ab,// 120 PAY 117 

    0xf8abd54c,// 121 PAY 118 

    0x2cf6eedb,// 122 PAY 119 

    0xd4f381e4,// 123 PAY 120 

    0x9e6f48ff,// 124 PAY 121 

    0xdce6af13,// 125 PAY 122 

    0x06df2757,// 126 PAY 123 

    0xa9e6ea34,// 127 PAY 124 

    0xa0dafffb,// 128 PAY 125 

    0xe901b5b2,// 129 PAY 126 

    0x6e9417a3,// 130 PAY 127 

    0xed3f6ddc,// 131 PAY 128 

    0xa7ceebe4,// 132 PAY 129 

    0xbc0abd11,// 133 PAY 130 

    0x43498bdf,// 134 PAY 131 

    0x2eee88c3,// 135 PAY 132 

    0x87089588,// 136 PAY 133 

    0x78dbab9c,// 137 PAY 134 

    0x76e46ee5,// 138 PAY 135 

    0xc5156c0f,// 139 PAY 136 

    0x189b3dd9,// 140 PAY 137 

    0x5da610ef,// 141 PAY 138 

    0xeb9ee7dc,// 142 PAY 139 

    0x87144ead,// 143 PAY 140 

    0x266aefe6,// 144 PAY 141 

    0xa91db10e,// 145 PAY 142 

    0x491ca2e7,// 146 PAY 143 

    0x7a0a3962,// 147 PAY 144 

    0x1cad827a,// 148 PAY 145 

    0x27b1c1d2,// 149 PAY 146 

    0xd890569d,// 150 PAY 147 

    0x287b2494,// 151 PAY 148 

    0x69414c28,// 152 PAY 149 

    0xdc92695b,// 153 PAY 150 

    0x2cb2d138,// 154 PAY 151 

    0x320312b0,// 155 PAY 152 

    0xd9a5fb13,// 156 PAY 153 

    0x5f1f7781,// 157 PAY 154 

    0x165a4c9a,// 158 PAY 155 

    0x01cc65ef,// 159 PAY 156 

    0x27b065fd,// 160 PAY 157 

    0xedc29901,// 161 PAY 158 

    0xf36c8678,// 162 PAY 159 

    0x64aa9eb6,// 163 PAY 160 

    0xfc14e015,// 164 PAY 161 

    0x698a93f2,// 165 PAY 162 

    0x981c1261,// 166 PAY 163 

    0x1ce87baa,// 167 PAY 164 

    0x46f848ad,// 168 PAY 165 

    0xaa4246be,// 169 PAY 166 

    0x91970ecb,// 170 PAY 167 

    0xbbeab6d0,// 171 PAY 168 

    0xd048bcd4,// 172 PAY 169 

    0x036763ca,// 173 PAY 170 

    0x73ead135,// 174 PAY 171 

    0x1e1acde0,// 175 PAY 172 

    0x5aca35d8,// 176 PAY 173 

    0x27f29aec,// 177 PAY 174 

    0x488a4e58,// 178 PAY 175 

    0x5cc51821,// 179 PAY 176 

    0x821767e8,// 180 PAY 177 

    0x9e3953e3,// 181 PAY 178 

    0x5f74da4d,// 182 PAY 179 

    0x76263571,// 183 PAY 180 

    0xbf462c2e,// 184 PAY 181 

    0xe7e6aeea,// 185 PAY 182 

    0xbaef91cd,// 186 PAY 183 

    0x8764c22f,// 187 PAY 184 

    0x37642ab5,// 188 PAY 185 

    0xf99396c1,// 189 PAY 186 

    0x3f9b68f4,// 190 PAY 187 

    0x07d06c73,// 191 PAY 188 

    0x1cf306aa,// 192 PAY 189 

    0xf33b3c4b,// 193 PAY 190 

    0xce1851fe,// 194 PAY 191 

    0x98044f00,// 195 PAY 192 

    0x8f51c7a5,// 196 PAY 193 

    0xfdf461a8,// 197 PAY 194 

    0x818108f1,// 198 PAY 195 

    0xabf7dd2d,// 199 PAY 196 

    0xb66cbfd2,// 200 PAY 197 

    0x449a5c9d,// 201 PAY 198 

    0x20042617,// 202 PAY 199 

    0xac075975,// 203 PAY 200 

    0x53cdae0e,// 204 PAY 201 

    0xc4bb1c38,// 205 PAY 202 

    0x881e13a2,// 206 PAY 203 

    0xeb850728,// 207 PAY 204 

    0xce7a7577,// 208 PAY 205 

    0xff981773,// 209 PAY 206 

    0x3dd24e80,// 210 PAY 207 

    0x92df9fe7,// 211 PAY 208 

    0xab22d160,// 212 PAY 209 

    0xa4500997,// 213 PAY 210 

    0x93535ef4,// 214 PAY 211 

    0x31b37ca3,// 215 PAY 212 

    0xaf8633d3,// 216 PAY 213 

    0x4fc3bc01,// 217 PAY 214 

    0x9ee538a3,// 218 PAY 215 

    0x28c621b9,// 219 PAY 216 

    0x09a89d27,// 220 PAY 217 

    0xde451940,// 221 PAY 218 

    0xf3e0e9db,// 222 PAY 219 

    0xeab308c7,// 223 PAY 220 

    0x3f454e35,// 224 PAY 221 

    0xf1e3d20d,// 225 PAY 222 

    0x803d2386,// 226 PAY 223 

    0x67d61b83,// 227 PAY 224 

    0x384bdbeb,// 228 PAY 225 

    0xf472a74b,// 229 PAY 226 

    0x325b01b5,// 230 PAY 227 

    0xf574cc3a,// 231 PAY 228 

    0x7e9a3c27,// 232 PAY 229 

    0xbece69ff,// 233 PAY 230 

    0x9502f31c,// 234 PAY 231 

    0x02c84a34,// 235 PAY 232 

    0x503b3557,// 236 PAY 233 

    0xc753a40b,// 237 PAY 234 

    0x77bdc6b6,// 238 PAY 235 

    0xe1979a55,// 239 PAY 236 

    0xfbfedba7,// 240 PAY 237 

    0x14dc3fec,// 241 PAY 238 

    0xf4d74a73,// 242 PAY 239 

    0xced3855b,// 243 PAY 240 

    0x1dc6c491,// 244 PAY 241 

    0xc9aa6366,// 245 PAY 242 

    0x459f3af9,// 246 PAY 243 

    0xb61c1b52,// 247 PAY 244 

    0x9fe39dee,// 248 PAY 245 

    0x2268b24e,// 249 PAY 246 

    0x04f85423,// 250 PAY 247 

    0x4a155942,// 251 PAY 248 

    0x4398f0f6,// 252 PAY 249 

    0x0eb7bdc8,// 253 PAY 250 

    0xa5000000,// 254 PAY 251 

/// STA is 1 words. 

/// STA num_pkts       : 24 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xed 

    0x03dded18 // 255 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt70_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 44 words. 

/// BDA size     is 171 (0xab) 

/// BDA id       is 0x8e57 

    0x00ab8e57,// 3 BDA   1 

/// PAY Generic Data size   : 171 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x1cbedd36,// 4 PAY   1 

    0x6a6e0d7f,// 5 PAY   2 

    0x6761ff41,// 6 PAY   3 

    0x5f357407,// 7 PAY   4 

    0x31d98b7e,// 8 PAY   5 

    0x0607cb12,// 9 PAY   6 

    0xcdfcdc7d,// 10 PAY   7 

    0xb7ba3106,// 11 PAY   8 

    0x094cc94e,// 12 PAY   9 

    0x0810326a,// 13 PAY  10 

    0xd46955d2,// 14 PAY  11 

    0x2c98545b,// 15 PAY  12 

    0xfea1d8fb,// 16 PAY  13 

    0x8dd10c4f,// 17 PAY  14 

    0x1cd86ae7,// 18 PAY  15 

    0x42eb08e4,// 19 PAY  16 

    0x3677b8d9,// 20 PAY  17 

    0x0ad6d40d,// 21 PAY  18 

    0xa8ed0fea,// 22 PAY  19 

    0x204e6612,// 23 PAY  20 

    0x84debfec,// 24 PAY  21 

    0x6c12ba9f,// 25 PAY  22 

    0x6d9b0c72,// 26 PAY  23 

    0x1e6fe4c7,// 27 PAY  24 

    0x7e4bb708,// 28 PAY  25 

    0x5da758f6,// 29 PAY  26 

    0x4a3a488e,// 30 PAY  27 

    0x3a2621c9,// 31 PAY  28 

    0xd2fed722,// 32 PAY  29 

    0xa3514fc2,// 33 PAY  30 

    0xac4b4043,// 34 PAY  31 

    0x48bd84fd,// 35 PAY  32 

    0xda4020bd,// 36 PAY  33 

    0xb30fb395,// 37 PAY  34 

    0x1a399574,// 38 PAY  35 

    0x977c1f7d,// 39 PAY  36 

    0x6fe580be,// 40 PAY  37 

    0x8f39299c,// 41 PAY  38 

    0xc98ea1eb,// 42 PAY  39 

    0x7d34a112,// 43 PAY  40 

    0x04bee801,// 44 PAY  41 

    0x70fe27f1,// 45 PAY  42 

    0x3fd01f00,// 46 PAY  43 

/// HASH is  12 bytes 

    0xa8ed0fea,// 47 HSH   1 

    0x204e6612,// 48 HSH   2 

    0x84debfec,// 49 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 20 

/// STA pkt_idx        : 132 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x13 

    0x02111314 // 50 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt71_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 481 words. 

/// BDA size     is 1919 (0x77f) 

/// BDA id       is 0xc9c9 

    0x077fc9c9,// 3 BDA   1 

/// PAY Generic Data size   : 1919 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x0790e05e,// 4 PAY   1 

    0x0c3902ea,// 5 PAY   2 

    0x676a4738,// 6 PAY   3 

    0xc84002a0,// 7 PAY   4 

    0xdf770a3c,// 8 PAY   5 

    0x4a26abc6,// 9 PAY   6 

    0x13c8bf28,// 10 PAY   7 

    0x8444d2a1,// 11 PAY   8 

    0x49939dd8,// 12 PAY   9 

    0xc97e9961,// 13 PAY  10 

    0xbab85138,// 14 PAY  11 

    0x8a07b77b,// 15 PAY  12 

    0x5603aac4,// 16 PAY  13 

    0x7a4062e3,// 17 PAY  14 

    0xa73abf15,// 18 PAY  15 

    0x67ce8cb2,// 19 PAY  16 

    0x49fb5cc3,// 20 PAY  17 

    0xdf0f087b,// 21 PAY  18 

    0xc03460f3,// 22 PAY  19 

    0xfe3b6f01,// 23 PAY  20 

    0x4bbb968d,// 24 PAY  21 

    0x6818f886,// 25 PAY  22 

    0x5bb26ab2,// 26 PAY  23 

    0x6356c1a5,// 27 PAY  24 

    0x1bd9f2aa,// 28 PAY  25 

    0xbf7351c0,// 29 PAY  26 

    0xd950425b,// 30 PAY  27 

    0x3d1d7045,// 31 PAY  28 

    0x8f55fa0f,// 32 PAY  29 

    0xc01e8a2c,// 33 PAY  30 

    0x022e8f97,// 34 PAY  31 

    0xd757e94b,// 35 PAY  32 

    0x50bd56e3,// 36 PAY  33 

    0x3aa199b1,// 37 PAY  34 

    0x84d7fb12,// 38 PAY  35 

    0x24aadf41,// 39 PAY  36 

    0x48e14b14,// 40 PAY  37 

    0x150a5ce7,// 41 PAY  38 

    0x22e9570e,// 42 PAY  39 

    0xe6a31863,// 43 PAY  40 

    0x3896ac94,// 44 PAY  41 

    0x3f29c46f,// 45 PAY  42 

    0x0abd109b,// 46 PAY  43 

    0x1ff3bd70,// 47 PAY  44 

    0x19d2b13e,// 48 PAY  45 

    0xd46412e3,// 49 PAY  46 

    0x340cdb9b,// 50 PAY  47 

    0x17a93f31,// 51 PAY  48 

    0x927d24a5,// 52 PAY  49 

    0x6c901de5,// 53 PAY  50 

    0xf8d90108,// 54 PAY  51 

    0x920bfbfb,// 55 PAY  52 

    0xcc6bb847,// 56 PAY  53 

    0xda45c261,// 57 PAY  54 

    0x2b49dd66,// 58 PAY  55 

    0x17a8c2c5,// 59 PAY  56 

    0x995c6375,// 60 PAY  57 

    0x4d970acd,// 61 PAY  58 

    0xfbe410c4,// 62 PAY  59 

    0x4c650481,// 63 PAY  60 

    0xbf85fdd3,// 64 PAY  61 

    0xaaf02d3a,// 65 PAY  62 

    0x31f7d3cf,// 66 PAY  63 

    0xaf91c7e0,// 67 PAY  64 

    0x466ceee3,// 68 PAY  65 

    0xd9bd2ac0,// 69 PAY  66 

    0xf10a3cf2,// 70 PAY  67 

    0xefe7f4bb,// 71 PAY  68 

    0x0e7edd55,// 72 PAY  69 

    0x8009d8ed,// 73 PAY  70 

    0x9b088c61,// 74 PAY  71 

    0xe2159b98,// 75 PAY  72 

    0xeed88430,// 76 PAY  73 

    0x8b339fe1,// 77 PAY  74 

    0x639b32ea,// 78 PAY  75 

    0x71fee5e6,// 79 PAY  76 

    0x6d35f90d,// 80 PAY  77 

    0x342af5a5,// 81 PAY  78 

    0x1c8c6d52,// 82 PAY  79 

    0xc87dd239,// 83 PAY  80 

    0x8e8d4bb1,// 84 PAY  81 

    0x208836b0,// 85 PAY  82 

    0xb54d7afe,// 86 PAY  83 

    0x18d65053,// 87 PAY  84 

    0x03e9aee6,// 88 PAY  85 

    0x97598eda,// 89 PAY  86 

    0x9d702d6b,// 90 PAY  87 

    0x75371f90,// 91 PAY  88 

    0x43f7a8fd,// 92 PAY  89 

    0x84d94c40,// 93 PAY  90 

    0x9742fbea,// 94 PAY  91 

    0xbabb5725,// 95 PAY  92 

    0xe4e80abe,// 96 PAY  93 

    0xf7f2e871,// 97 PAY  94 

    0x6edbac50,// 98 PAY  95 

    0xbba7f86c,// 99 PAY  96 

    0x846c815a,// 100 PAY  97 

    0x61d79912,// 101 PAY  98 

    0xb7f98925,// 102 PAY  99 

    0xd10a8d64,// 103 PAY 100 

    0x502e7bf2,// 104 PAY 101 

    0xbdb5967f,// 105 PAY 102 

    0xbaadb147,// 106 PAY 103 

    0xcaa81349,// 107 PAY 104 

    0x3382eb84,// 108 PAY 105 

    0x10d14573,// 109 PAY 106 

    0xa1558c42,// 110 PAY 107 

    0x90f0e0a6,// 111 PAY 108 

    0xb1b0513f,// 112 PAY 109 

    0xbd4ef02b,// 113 PAY 110 

    0xeecb15d6,// 114 PAY 111 

    0x53cccbb2,// 115 PAY 112 

    0x1213de38,// 116 PAY 113 

    0xb5d3f8f4,// 117 PAY 114 

    0x9530499a,// 118 PAY 115 

    0xb83f7e27,// 119 PAY 116 

    0xbb52d86e,// 120 PAY 117 

    0x084b0295,// 121 PAY 118 

    0x0e22523e,// 122 PAY 119 

    0x3fc7f8dc,// 123 PAY 120 

    0x55d3a3de,// 124 PAY 121 

    0x63f29116,// 125 PAY 122 

    0x3767f3ac,// 126 PAY 123 

    0x1fe2d3d0,// 127 PAY 124 

    0x14f7a05a,// 128 PAY 125 

    0xb35d1c4e,// 129 PAY 126 

    0x90c04c7b,// 130 PAY 127 

    0x550fbac0,// 131 PAY 128 

    0x9ec983a6,// 132 PAY 129 

    0xe9341f9a,// 133 PAY 130 

    0x48e5fcbf,// 134 PAY 131 

    0xc784808d,// 135 PAY 132 

    0x58ad39bd,// 136 PAY 133 

    0x5c37a590,// 137 PAY 134 

    0xc28b6b3e,// 138 PAY 135 

    0x8175e897,// 139 PAY 136 

    0xc72330d2,// 140 PAY 137 

    0x480c3e2c,// 141 PAY 138 

    0x7b56e608,// 142 PAY 139 

    0x5f0f760a,// 143 PAY 140 

    0x3e29a181,// 144 PAY 141 

    0x0b6ea9c3,// 145 PAY 142 

    0x30d2fc2c,// 146 PAY 143 

    0x63392f72,// 147 PAY 144 

    0x4d868d74,// 148 PAY 145 

    0x7bfbb405,// 149 PAY 146 

    0x1c85f184,// 150 PAY 147 

    0xadf1180d,// 151 PAY 148 

    0x4dc09921,// 152 PAY 149 

    0x8fe3515e,// 153 PAY 150 

    0x3c97b5cd,// 154 PAY 151 

    0xfe7de79a,// 155 PAY 152 

    0x4d2dd0d4,// 156 PAY 153 

    0xa626bfa2,// 157 PAY 154 

    0x18a9442b,// 158 PAY 155 

    0xdb7809d1,// 159 PAY 156 

    0xdb9fb619,// 160 PAY 157 

    0xfc79c2f1,// 161 PAY 158 

    0x0d96622f,// 162 PAY 159 

    0x6926cdf8,// 163 PAY 160 

    0x1522f7fb,// 164 PAY 161 

    0x869966ef,// 165 PAY 162 

    0xadb3d940,// 166 PAY 163 

    0x80da5716,// 167 PAY 164 

    0x1937167a,// 168 PAY 165 

    0x62d6e10a,// 169 PAY 166 

    0xb55e574a,// 170 PAY 167 

    0xbc09c30c,// 171 PAY 168 

    0x2a1d079a,// 172 PAY 169 

    0x3e06e2aa,// 173 PAY 170 

    0xa3e92ebf,// 174 PAY 171 

    0x09f9da06,// 175 PAY 172 

    0x62c8ac4f,// 176 PAY 173 

    0x5540cd62,// 177 PAY 174 

    0x8647dca5,// 178 PAY 175 

    0xd8e1e8e2,// 179 PAY 176 

    0x186d0ca7,// 180 PAY 177 

    0xe67662aa,// 181 PAY 178 

    0xccab7d6f,// 182 PAY 179 

    0xf48bf12b,// 183 PAY 180 

    0xfbdca24d,// 184 PAY 181 

    0x12b7cdec,// 185 PAY 182 

    0x1bafddf2,// 186 PAY 183 

    0x3f46de0b,// 187 PAY 184 

    0xe85a3cb3,// 188 PAY 185 

    0x4812e1c3,// 189 PAY 186 

    0x0fdb476a,// 190 PAY 187 

    0x27bc447b,// 191 PAY 188 

    0xa0e446dd,// 192 PAY 189 

    0x3c7e8259,// 193 PAY 190 

    0x16cbed18,// 194 PAY 191 

    0xdb2b171e,// 195 PAY 192 

    0x175b765d,// 196 PAY 193 

    0xd56b8f39,// 197 PAY 194 

    0x822ab482,// 198 PAY 195 

    0xc2f58efb,// 199 PAY 196 

    0xc7d11716,// 200 PAY 197 

    0x626d9739,// 201 PAY 198 

    0x6074098b,// 202 PAY 199 

    0xa85c1758,// 203 PAY 200 

    0xe231a294,// 204 PAY 201 

    0x2edc088f,// 205 PAY 202 

    0xfc0068a6,// 206 PAY 203 

    0x40939411,// 207 PAY 204 

    0xf84c3f99,// 208 PAY 205 

    0xa3af96c4,// 209 PAY 206 

    0x6df24224,// 210 PAY 207 

    0x6c37fed2,// 211 PAY 208 

    0x3de39fa1,// 212 PAY 209 

    0xa7cba219,// 213 PAY 210 

    0x9d34bf76,// 214 PAY 211 

    0xe2e8428f,// 215 PAY 212 

    0x1e6991b2,// 216 PAY 213 

    0xa8092cf9,// 217 PAY 214 

    0x2a15cbaf,// 218 PAY 215 

    0xe4aec899,// 219 PAY 216 

    0x667434af,// 220 PAY 217 

    0xc052efef,// 221 PAY 218 

    0x8a10ed3c,// 222 PAY 219 

    0x405d23ea,// 223 PAY 220 

    0xca19a5ef,// 224 PAY 221 

    0x9c7e9fe1,// 225 PAY 222 

    0x0d092e4c,// 226 PAY 223 

    0x8f44845e,// 227 PAY 224 

    0x6a3223b3,// 228 PAY 225 

    0x6b2b3497,// 229 PAY 226 

    0x8cb947ed,// 230 PAY 227 

    0x8be40ca5,// 231 PAY 228 

    0x5f531346,// 232 PAY 229 

    0x0391e257,// 233 PAY 230 

    0x145d3034,// 234 PAY 231 

    0xa7e3b420,// 235 PAY 232 

    0x26b0514e,// 236 PAY 233 

    0xf3a63d6c,// 237 PAY 234 

    0x22f18208,// 238 PAY 235 

    0xe2ce0835,// 239 PAY 236 

    0x95eccb2f,// 240 PAY 237 

    0x7c82364f,// 241 PAY 238 

    0xf57fe107,// 242 PAY 239 

    0x8ddd1f35,// 243 PAY 240 

    0x9af74a17,// 244 PAY 241 

    0xf1292140,// 245 PAY 242 

    0x93baa022,// 246 PAY 243 

    0xd889c15a,// 247 PAY 244 

    0x381898fc,// 248 PAY 245 

    0xab8c72c7,// 249 PAY 246 

    0x22e0f06c,// 250 PAY 247 

    0xefa68f50,// 251 PAY 248 

    0x159c4632,// 252 PAY 249 

    0xf1149571,// 253 PAY 250 

    0x2a17d39f,// 254 PAY 251 

    0xa6b19785,// 255 PAY 252 

    0x537bb3d4,// 256 PAY 253 

    0x5582d983,// 257 PAY 254 

    0x52921999,// 258 PAY 255 

    0x705ca01f,// 259 PAY 256 

    0xc4df5b02,// 260 PAY 257 

    0x7eb02e24,// 261 PAY 258 

    0xcd76d848,// 262 PAY 259 

    0x63801ee4,// 263 PAY 260 

    0x35f2ed30,// 264 PAY 261 

    0x6bc4804e,// 265 PAY 262 

    0xd16a3c7c,// 266 PAY 263 

    0x3f01b1c6,// 267 PAY 264 

    0xd7c41b18,// 268 PAY 265 

    0xa79a2569,// 269 PAY 266 

    0x869eedba,// 270 PAY 267 

    0x49a2581b,// 271 PAY 268 

    0x928ccfa9,// 272 PAY 269 

    0xf29ebcf3,// 273 PAY 270 

    0x351f4ca4,// 274 PAY 271 

    0xb7e5d945,// 275 PAY 272 

    0x59066568,// 276 PAY 273 

    0x7bfe764e,// 277 PAY 274 

    0xdbdc9669,// 278 PAY 275 

    0xdc9bec84,// 279 PAY 276 

    0xaebde1f9,// 280 PAY 277 

    0x182a2923,// 281 PAY 278 

    0xc4aa871e,// 282 PAY 279 

    0xb7c19f41,// 283 PAY 280 

    0xf983cd05,// 284 PAY 281 

    0xafad39d5,// 285 PAY 282 

    0x762ba32e,// 286 PAY 283 

    0xb4cc6c2e,// 287 PAY 284 

    0x4714bd3c,// 288 PAY 285 

    0xba54b963,// 289 PAY 286 

    0x613a7520,// 290 PAY 287 

    0x3433113d,// 291 PAY 288 

    0xa8a762a9,// 292 PAY 289 

    0x40749acb,// 293 PAY 290 

    0xfd530b69,// 294 PAY 291 

    0x2a4ee7f3,// 295 PAY 292 

    0xac1874be,// 296 PAY 293 

    0x4beef707,// 297 PAY 294 

    0xc4d18805,// 298 PAY 295 

    0xc4704f37,// 299 PAY 296 

    0xc9be9f82,// 300 PAY 297 

    0x39a274de,// 301 PAY 298 

    0x69de2cf8,// 302 PAY 299 

    0x15c820db,// 303 PAY 300 

    0xb7ccd921,// 304 PAY 301 

    0x8a3479fe,// 305 PAY 302 

    0xe165c96c,// 306 PAY 303 

    0x537ee4f6,// 307 PAY 304 

    0x1c1aee5a,// 308 PAY 305 

    0x4340d233,// 309 PAY 306 

    0x9bec2138,// 310 PAY 307 

    0xab1e15d8,// 311 PAY 308 

    0x8abd0451,// 312 PAY 309 

    0xeaf35917,// 313 PAY 310 

    0x72b0af5e,// 314 PAY 311 

    0x50c444b9,// 315 PAY 312 

    0xb0e82c97,// 316 PAY 313 

    0x922640cb,// 317 PAY 314 

    0x835962fd,// 318 PAY 315 

    0xae522b80,// 319 PAY 316 

    0x8aa64481,// 320 PAY 317 

    0x7ea18dcc,// 321 PAY 318 

    0x98a937bd,// 322 PAY 319 

    0xecc8286f,// 323 PAY 320 

    0x31a4bad9,// 324 PAY 321 

    0x0edd2e6d,// 325 PAY 322 

    0xfd7fc345,// 326 PAY 323 

    0x592b25b9,// 327 PAY 324 

    0x87f23a29,// 328 PAY 325 

    0x3d6883e9,// 329 PAY 326 

    0xf655347c,// 330 PAY 327 

    0xecb9f0aa,// 331 PAY 328 

    0x4076a3fd,// 332 PAY 329 

    0x259078f8,// 333 PAY 330 

    0xbb309c1a,// 334 PAY 331 

    0xcba123c8,// 335 PAY 332 

    0x46e7684e,// 336 PAY 333 

    0x704b8164,// 337 PAY 334 

    0x639e6fb2,// 338 PAY 335 

    0x989a5ae0,// 339 PAY 336 

    0x0fa0d68f,// 340 PAY 337 

    0x8d8e4fb3,// 341 PAY 338 

    0xf92e6e23,// 342 PAY 339 

    0x2355b339,// 343 PAY 340 

    0x36da1cad,// 344 PAY 341 

    0xf46db92f,// 345 PAY 342 

    0x67d90d58,// 346 PAY 343 

    0xf2c75b48,// 347 PAY 344 

    0x1b019897,// 348 PAY 345 

    0x62ff8ee5,// 349 PAY 346 

    0x50f2b0e9,// 350 PAY 347 

    0x1581a56f,// 351 PAY 348 

    0x2c563c95,// 352 PAY 349 

    0x7b1b7473,// 353 PAY 350 

    0xd6ae261e,// 354 PAY 351 

    0xaf06d348,// 355 PAY 352 

    0x50e63144,// 356 PAY 353 

    0x4d716264,// 357 PAY 354 

    0x95443caf,// 358 PAY 355 

    0x2bbd243e,// 359 PAY 356 

    0x5ceb5cd1,// 360 PAY 357 

    0x18cb1648,// 361 PAY 358 

    0x9fb90fb3,// 362 PAY 359 

    0x080a9a3d,// 363 PAY 360 

    0xe689802e,// 364 PAY 361 

    0x6100ce43,// 365 PAY 362 

    0x93a47abe,// 366 PAY 363 

    0x2e7fe68a,// 367 PAY 364 

    0xc8a4bf4c,// 368 PAY 365 

    0xa50297b0,// 369 PAY 366 

    0x83be43aa,// 370 PAY 367 

    0xa914beab,// 371 PAY 368 

    0x49bfdeab,// 372 PAY 369 

    0x60cd44bf,// 373 PAY 370 

    0x54940b9c,// 374 PAY 371 

    0x32be4ae8,// 375 PAY 372 

    0x5f490423,// 376 PAY 373 

    0x034fbcc4,// 377 PAY 374 

    0xfc99ccdc,// 378 PAY 375 

    0xc124e4f0,// 379 PAY 376 

    0x8bb7f0da,// 380 PAY 377 

    0xccf4fd43,// 381 PAY 378 

    0x1b65f7a9,// 382 PAY 379 

    0xa11abecb,// 383 PAY 380 

    0xbf2658fd,// 384 PAY 381 

    0x3a588b20,// 385 PAY 382 

    0x86da69a0,// 386 PAY 383 

    0x5a2193a1,// 387 PAY 384 

    0x41404cd7,// 388 PAY 385 

    0x00ae2751,// 389 PAY 386 

    0xf0c47be8,// 390 PAY 387 

    0xb910f75a,// 391 PAY 388 

    0xe2f2472f,// 392 PAY 389 

    0xf78ccf67,// 393 PAY 390 

    0x16b7ca33,// 394 PAY 391 

    0xa4a6c25a,// 395 PAY 392 

    0xf151f161,// 396 PAY 393 

    0x90953ea3,// 397 PAY 394 

    0xc726edaa,// 398 PAY 395 

    0xeb55fa60,// 399 PAY 396 

    0x36391185,// 400 PAY 397 

    0xd6a89a3e,// 401 PAY 398 

    0xea05a23b,// 402 PAY 399 

    0x9d11bc77,// 403 PAY 400 

    0xc351bc60,// 404 PAY 401 

    0x3d67b971,// 405 PAY 402 

    0x814272dd,// 406 PAY 403 

    0xa0e1e0ce,// 407 PAY 404 

    0x52444a76,// 408 PAY 405 

    0x07a0c2bf,// 409 PAY 406 

    0xa80626e3,// 410 PAY 407 

    0x4f558f74,// 411 PAY 408 

    0x87cc03eb,// 412 PAY 409 

    0xc37daf70,// 413 PAY 410 

    0x48fb3909,// 414 PAY 411 

    0x81bb2025,// 415 PAY 412 

    0xf23f1d96,// 416 PAY 413 

    0xa86af1d7,// 417 PAY 414 

    0x5e9370d6,// 418 PAY 415 

    0x93eaa366,// 419 PAY 416 

    0xe9fd64a0,// 420 PAY 417 

    0x8016063d,// 421 PAY 418 

    0xeba1370f,// 422 PAY 419 

    0xa7ef736b,// 423 PAY 420 

    0x7d55f99b,// 424 PAY 421 

    0x23a25191,// 425 PAY 422 

    0x511042e3,// 426 PAY 423 

    0x29106f0c,// 427 PAY 424 

    0x1fb99d29,// 428 PAY 425 

    0x97cb4e0f,// 429 PAY 426 

    0xf9e0233b,// 430 PAY 427 

    0x0bdad38f,// 431 PAY 428 

    0x2360c613,// 432 PAY 429 

    0xf025436c,// 433 PAY 430 

    0x56d1400c,// 434 PAY 431 

    0xb8e5690b,// 435 PAY 432 

    0x9f6796f6,// 436 PAY 433 

    0xc2b3dc7c,// 437 PAY 434 

    0xc3f8de71,// 438 PAY 435 

    0xf7856764,// 439 PAY 436 

    0x0147a782,// 440 PAY 437 

    0x817f8c3b,// 441 PAY 438 

    0x1ea1153c,// 442 PAY 439 

    0x4dd39879,// 443 PAY 440 

    0x9afaced9,// 444 PAY 441 

    0x92723fef,// 445 PAY 442 

    0x0fd30606,// 446 PAY 443 

    0x75131e45,// 447 PAY 444 

    0xd32cd00b,// 448 PAY 445 

    0xc1d077a3,// 449 PAY 446 

    0x7f8541d1,// 450 PAY 447 

    0x38facef4,// 451 PAY 448 

    0x37a619db,// 452 PAY 449 

    0xbc2c98e4,// 453 PAY 450 

    0x156a2e4b,// 454 PAY 451 

    0xb72bcf4d,// 455 PAY 452 

    0x4a70aedb,// 456 PAY 453 

    0xe741544c,// 457 PAY 454 

    0x62549c67,// 458 PAY 455 

    0xe2f45535,// 459 PAY 456 

    0x641d2594,// 460 PAY 457 

    0xb87d5ce8,// 461 PAY 458 

    0xcb2e09eb,// 462 PAY 459 

    0x10738417,// 463 PAY 460 

    0x9727a8a0,// 464 PAY 461 

    0x54912e26,// 465 PAY 462 

    0xc25d30eb,// 466 PAY 463 

    0x60b7e1fe,// 467 PAY 464 

    0x83e8eea1,// 468 PAY 465 

    0x2ae54ba2,// 469 PAY 466 

    0xf204cf46,// 470 PAY 467 

    0xaf547b67,// 471 PAY 468 

    0x3d7f34e5,// 472 PAY 469 

    0xca05b218,// 473 PAY 470 

    0xa4e796f2,// 474 PAY 471 

    0xd1a6c848,// 475 PAY 472 

    0x785f5577,// 476 PAY 473 

    0x49aca4f4,// 477 PAY 474 

    0x2d5cff46,// 478 PAY 475 

    0x457534ce,// 479 PAY 476 

    0xadcb3f79,// 480 PAY 477 

    0x1e63b7de,// 481 PAY 478 

    0xac314dda,// 482 PAY 479 

    0x39d96800,// 483 PAY 480 

/// HASH is  4 bytes 

    0xf1292140,// 484 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 85 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf5 

    0x02ccf555 // 485 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt72_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 143 words. 

/// BDA size     is 565 (0x235) 

/// BDA id       is 0xdad4 

    0x0235dad4,// 3 BDA   1 

/// PAY Generic Data size   : 565 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x1e3cf271,// 4 PAY   1 

    0xb75681eb,// 5 PAY   2 

    0x8ca0bfdd,// 6 PAY   3 

    0x9c48921e,// 7 PAY   4 

    0x1c2fa72a,// 8 PAY   5 

    0xb4dd5f0f,// 9 PAY   6 

    0x0d35f45d,// 10 PAY   7 

    0x667838d2,// 11 PAY   8 

    0x54619c80,// 12 PAY   9 

    0xb5474065,// 13 PAY  10 

    0x2c6b4197,// 14 PAY  11 

    0x5b313f86,// 15 PAY  12 

    0xbafb6178,// 16 PAY  13 

    0x5b6290c5,// 17 PAY  14 

    0xbc5163cf,// 18 PAY  15 

    0x25a3b7c6,// 19 PAY  16 

    0xa0574c75,// 20 PAY  17 

    0x98fc257f,// 21 PAY  18 

    0xdd9e030f,// 22 PAY  19 

    0x4e130035,// 23 PAY  20 

    0x81fb49ac,// 24 PAY  21 

    0x838955fa,// 25 PAY  22 

    0x427db4ec,// 26 PAY  23 

    0x7ba8b7f6,// 27 PAY  24 

    0xf4f54f21,// 28 PAY  25 

    0xa5b0e53b,// 29 PAY  26 

    0x4a9487ac,// 30 PAY  27 

    0x8444efce,// 31 PAY  28 

    0x69f7cb3e,// 32 PAY  29 

    0xdfedab48,// 33 PAY  30 

    0x9494d575,// 34 PAY  31 

    0xdc076036,// 35 PAY  32 

    0x0898197e,// 36 PAY  33 

    0xea18e49c,// 37 PAY  34 

    0xe216a3ee,// 38 PAY  35 

    0xc54f3fe7,// 39 PAY  36 

    0x07858af5,// 40 PAY  37 

    0xfd682dff,// 41 PAY  38 

    0xf0931886,// 42 PAY  39 

    0xbb99c5cb,// 43 PAY  40 

    0xbba2a92c,// 44 PAY  41 

    0x68c5c443,// 45 PAY  42 

    0x6048a463,// 46 PAY  43 

    0x7da69e31,// 47 PAY  44 

    0x6e590599,// 48 PAY  45 

    0x76328fb7,// 49 PAY  46 

    0xc7df8299,// 50 PAY  47 

    0xef875760,// 51 PAY  48 

    0xea36655f,// 52 PAY  49 

    0xa70b8ba8,// 53 PAY  50 

    0x31054f3a,// 54 PAY  51 

    0x60a1d4aa,// 55 PAY  52 

    0xb2eb1bf5,// 56 PAY  53 

    0x53fc757f,// 57 PAY  54 

    0xc4142f3c,// 58 PAY  55 

    0xec510fec,// 59 PAY  56 

    0xae9a60b6,// 60 PAY  57 

    0x69472267,// 61 PAY  58 

    0x959a3eec,// 62 PAY  59 

    0xf956bc92,// 63 PAY  60 

    0x6fa1e3d3,// 64 PAY  61 

    0x7da77457,// 65 PAY  62 

    0x3fdde558,// 66 PAY  63 

    0x3c6c7c3f,// 67 PAY  64 

    0xf67ad2aa,// 68 PAY  65 

    0x53f8dbfc,// 69 PAY  66 

    0x7653849e,// 70 PAY  67 

    0xb8c29261,// 71 PAY  68 

    0xc6c601c3,// 72 PAY  69 

    0x79dcad30,// 73 PAY  70 

    0x48b10d6a,// 74 PAY  71 

    0x0fa4eba6,// 75 PAY  72 

    0x0c10764e,// 76 PAY  73 

    0xbdbaec3c,// 77 PAY  74 

    0xb7cc5c89,// 78 PAY  75 

    0x64681ccd,// 79 PAY  76 

    0x07b6fa58,// 80 PAY  77 

    0x8fa8b261,// 81 PAY  78 

    0xa581f4d1,// 82 PAY  79 

    0xa90aeece,// 83 PAY  80 

    0x991ff9c5,// 84 PAY  81 

    0x467887d2,// 85 PAY  82 

    0x39eea243,// 86 PAY  83 

    0x0abe22a1,// 87 PAY  84 

    0x95f15c2d,// 88 PAY  85 

    0xdc53597e,// 89 PAY  86 

    0x09e5a5bc,// 90 PAY  87 

    0x0319194c,// 91 PAY  88 

    0xf78309b1,// 92 PAY  89 

    0xd463ca95,// 93 PAY  90 

    0x6950fcc7,// 94 PAY  91 

    0xbfc34eac,// 95 PAY  92 

    0x2c02549b,// 96 PAY  93 

    0x2c1b07c1,// 97 PAY  94 

    0x1659acf8,// 98 PAY  95 

    0x09ce6be8,// 99 PAY  96 

    0x4dc2e961,// 100 PAY  97 

    0x5ea5e86f,// 101 PAY  98 

    0xc375db1f,// 102 PAY  99 

    0x7ea55912,// 103 PAY 100 

    0x2f8b8bf9,// 104 PAY 101 

    0x14a9b336,// 105 PAY 102 

    0x78b15929,// 106 PAY 103 

    0x8f87a1c5,// 107 PAY 104 

    0x65931bec,// 108 PAY 105 

    0x127025c4,// 109 PAY 106 

    0x40016f8e,// 110 PAY 107 

    0x81d2ce9e,// 111 PAY 108 

    0x2a3235fc,// 112 PAY 109 

    0x45ce43c8,// 113 PAY 110 

    0x9c5a3a7c,// 114 PAY 111 

    0x85ced3db,// 115 PAY 112 

    0x63284e87,// 116 PAY 113 

    0xe8030ebd,// 117 PAY 114 

    0xda9203a3,// 118 PAY 115 

    0x3cfcb832,// 119 PAY 116 

    0xe0110d4c,// 120 PAY 117 

    0x1d7e1e20,// 121 PAY 118 

    0x2416c5a4,// 122 PAY 119 

    0x951a584b,// 123 PAY 120 

    0x01a49c5d,// 124 PAY 121 

    0x70fc4867,// 125 PAY 122 

    0x50e5ed4d,// 126 PAY 123 

    0xdd5701ab,// 127 PAY 124 

    0x6949123b,// 128 PAY 125 

    0x734bd37a,// 129 PAY 126 

    0x76f57699,// 130 PAY 127 

    0x881f9df0,// 131 PAY 128 

    0xfde9693a,// 132 PAY 129 

    0x2ecf6c36,// 133 PAY 130 

    0xe47766ff,// 134 PAY 131 

    0x24162075,// 135 PAY 132 

    0x354b1f51,// 136 PAY 133 

    0x569e088a,// 137 PAY 134 

    0xc978fd50,// 138 PAY 135 

    0x86907b09,// 139 PAY 136 

    0x80685308,// 140 PAY 137 

    0x5c4ec09a,// 141 PAY 138 

    0x210a3fd5,// 142 PAY 139 

    0xcf219441,// 143 PAY 140 

    0x4940084d,// 144 PAY 141 

    0xfa000000,// 145 PAY 142 

/// HASH is  20 bytes 

    0x4dc2e961,// 146 HSH   1 

    0x5ea5e86f,// 147 HSH   2 

    0xc375db1f,// 148 HSH   3 

    0x7ea55912,// 149 HSH   4 

    0x2f8b8bf9,// 150 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 45 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x94 

    0x00b5942c // 151 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt73_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 478 words. 

/// BDA size     is 1905 (0x771) 

/// BDA id       is 0xb041 

    0x0771b041,// 3 BDA   1 

/// PAY Generic Data size   : 1905 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x7ade8c5f,// 4 PAY   1 

    0x47ae005f,// 5 PAY   2 

    0x8b08cd18,// 6 PAY   3 

    0xe7747e41,// 7 PAY   4 

    0xc434a1a7,// 8 PAY   5 

    0x4be3b573,// 9 PAY   6 

    0x178a4e8b,// 10 PAY   7 

    0xf04a717d,// 11 PAY   8 

    0x41e2c8a8,// 12 PAY   9 

    0x18782cab,// 13 PAY  10 

    0xc4d1fa5b,// 14 PAY  11 

    0x3c9ccfe6,// 15 PAY  12 

    0xf96be32c,// 16 PAY  13 

    0x94cb32c5,// 17 PAY  14 

    0x914d0a12,// 18 PAY  15 

    0x5e746b05,// 19 PAY  16 

    0xa188f8eb,// 20 PAY  17 

    0xed97b84c,// 21 PAY  18 

    0x7e191d23,// 22 PAY  19 

    0xe01e2df2,// 23 PAY  20 

    0xa1a77f68,// 24 PAY  21 

    0x0852fd48,// 25 PAY  22 

    0xf3523617,// 26 PAY  23 

    0x5108f46f,// 27 PAY  24 

    0x7c5e737e,// 28 PAY  25 

    0x59987233,// 29 PAY  26 

    0x777478a4,// 30 PAY  27 

    0x1034605a,// 31 PAY  28 

    0xb796e67e,// 32 PAY  29 

    0x11ab7e7a,// 33 PAY  30 

    0xb23bda64,// 34 PAY  31 

    0x0d81845d,// 35 PAY  32 

    0x0e54bf65,// 36 PAY  33 

    0x180eddc0,// 37 PAY  34 

    0x4357768e,// 38 PAY  35 

    0x13765849,// 39 PAY  36 

    0x26fd3949,// 40 PAY  37 

    0xd5d30d73,// 41 PAY  38 

    0xac3b2c22,// 42 PAY  39 

    0x253ee495,// 43 PAY  40 

    0xbb7689e4,// 44 PAY  41 

    0xfcfd595d,// 45 PAY  42 

    0xcd1055bc,// 46 PAY  43 

    0xed9bc390,// 47 PAY  44 

    0xf5ed6739,// 48 PAY  45 

    0x5cb92cf2,// 49 PAY  46 

    0x33520483,// 50 PAY  47 

    0x9dbbb7fb,// 51 PAY  48 

    0x985a13b9,// 52 PAY  49 

    0x50645dc6,// 53 PAY  50 

    0xe5752864,// 54 PAY  51 

    0x5da47299,// 55 PAY  52 

    0x113a2391,// 56 PAY  53 

    0xc3a565ff,// 57 PAY  54 

    0x046b34b3,// 58 PAY  55 

    0x2dc89e23,// 59 PAY  56 

    0xb5fb9ddb,// 60 PAY  57 

    0xc1ac1bc4,// 61 PAY  58 

    0x46a345ea,// 62 PAY  59 

    0x6f42c6ce,// 63 PAY  60 

    0x54ee0304,// 64 PAY  61 

    0x9c45443e,// 65 PAY  62 

    0xe24c4d24,// 66 PAY  63 

    0x9b588071,// 67 PAY  64 

    0x0b5aeaf5,// 68 PAY  65 

    0x6c056462,// 69 PAY  66 

    0x22abaf9f,// 70 PAY  67 

    0x38e66dbb,// 71 PAY  68 

    0x3167cd72,// 72 PAY  69 

    0x21094064,// 73 PAY  70 

    0xf3a46c21,// 74 PAY  71 

    0x968cbe51,// 75 PAY  72 

    0xa323ea3b,// 76 PAY  73 

    0xacc772f3,// 77 PAY  74 

    0x63bc447f,// 78 PAY  75 

    0x2bdcc64d,// 79 PAY  76 

    0xdc20f80f,// 80 PAY  77 

    0x611e0178,// 81 PAY  78 

    0x1f3aa337,// 82 PAY  79 

    0x2629bcc1,// 83 PAY  80 

    0x62d2fb14,// 84 PAY  81 

    0x384417c2,// 85 PAY  82 

    0x2f5b797c,// 86 PAY  83 

    0x614db4d1,// 87 PAY  84 

    0xb5b061be,// 88 PAY  85 

    0x38dfc035,// 89 PAY  86 

    0x15b05609,// 90 PAY  87 

    0x936a27e1,// 91 PAY  88 

    0x946fd56c,// 92 PAY  89 

    0xe8626c78,// 93 PAY  90 

    0x7c47244f,// 94 PAY  91 

    0x6fd290e0,// 95 PAY  92 

    0xd40df44f,// 96 PAY  93 

    0x41ca1b3b,// 97 PAY  94 

    0xe03d8b15,// 98 PAY  95 

    0x2097f288,// 99 PAY  96 

    0xfb49f753,// 100 PAY  97 

    0x2aa486d3,// 101 PAY  98 

    0xaf55b21f,// 102 PAY  99 

    0x34a39c1b,// 103 PAY 100 

    0xc9d7bedf,// 104 PAY 101 

    0xe443048f,// 105 PAY 102 

    0xa9489540,// 106 PAY 103 

    0x92192063,// 107 PAY 104 

    0x3a5b8a62,// 108 PAY 105 

    0xe60e380b,// 109 PAY 106 

    0x85250d9d,// 110 PAY 107 

    0x145927b4,// 111 PAY 108 

    0xc967e939,// 112 PAY 109 

    0x0b0fcbaa,// 113 PAY 110 

    0x0100daca,// 114 PAY 111 

    0xd86886d4,// 115 PAY 112 

    0x1bdb8650,// 116 PAY 113 

    0x5a7e3775,// 117 PAY 114 

    0xb2bda177,// 118 PAY 115 

    0x276db8c4,// 119 PAY 116 

    0x16c629cd,// 120 PAY 117 

    0x38bee713,// 121 PAY 118 

    0x2556ea74,// 122 PAY 119 

    0x3b48e94c,// 123 PAY 120 

    0x10c8316d,// 124 PAY 121 

    0x3fc92b4d,// 125 PAY 122 

    0x0c88b02e,// 126 PAY 123 

    0xe113bbbf,// 127 PAY 124 

    0x7b0c63b9,// 128 PAY 125 

    0x55f5438e,// 129 PAY 126 

    0x12191ac5,// 130 PAY 127 

    0xa36937d3,// 131 PAY 128 

    0xaa6c6a38,// 132 PAY 129 

    0xa241ce17,// 133 PAY 130 

    0x9e7545c6,// 134 PAY 131 

    0x3d9ced0f,// 135 PAY 132 

    0xab78f534,// 136 PAY 133 

    0xf62d7e8e,// 137 PAY 134 

    0x6457d20b,// 138 PAY 135 

    0x7e2d2322,// 139 PAY 136 

    0x5d3a965e,// 140 PAY 137 

    0x3fd9c0b1,// 141 PAY 138 

    0x12baea22,// 142 PAY 139 

    0x9008b121,// 143 PAY 140 

    0xc1115367,// 144 PAY 141 

    0x69407d77,// 145 PAY 142 

    0xee559f81,// 146 PAY 143 

    0xd32b7c56,// 147 PAY 144 

    0x73746090,// 148 PAY 145 

    0x9cb95a44,// 149 PAY 146 

    0xdd67b5de,// 150 PAY 147 

    0xa3d14182,// 151 PAY 148 

    0xb62945e4,// 152 PAY 149 

    0xb2c53f79,// 153 PAY 150 

    0x4277b122,// 154 PAY 151 

    0x8b043f88,// 155 PAY 152 

    0x90c43bac,// 156 PAY 153 

    0x7b2ea867,// 157 PAY 154 

    0xf27064b6,// 158 PAY 155 

    0x5a1e1a92,// 159 PAY 156 

    0x00dc00b1,// 160 PAY 157 

    0x8f216257,// 161 PAY 158 

    0xa0e3d751,// 162 PAY 159 

    0x5be66629,// 163 PAY 160 

    0xa81dd09a,// 164 PAY 161 

    0x7c5352f4,// 165 PAY 162 

    0xfe11c291,// 166 PAY 163 

    0x1ab0d8fc,// 167 PAY 164 

    0xbdbfac1c,// 168 PAY 165 

    0x24dfadea,// 169 PAY 166 

    0x1284b541,// 170 PAY 167 

    0xc8b4f65e,// 171 PAY 168 

    0x2eb70400,// 172 PAY 169 

    0x207c48d2,// 173 PAY 170 

    0x5d2875e1,// 174 PAY 171 

    0x6f283785,// 175 PAY 172 

    0x830de6f7,// 176 PAY 173 

    0xef0c99c4,// 177 PAY 174 

    0x43a05b6a,// 178 PAY 175 

    0x24aa0f2a,// 179 PAY 176 

    0x0ea68045,// 180 PAY 177 

    0xe8e5dce7,// 181 PAY 178 

    0xeb9fb12d,// 182 PAY 179 

    0xdc9d5720,// 183 PAY 180 

    0xfff8abe7,// 184 PAY 181 

    0xb753aee6,// 185 PAY 182 

    0x01207b90,// 186 PAY 183 

    0x84530973,// 187 PAY 184 

    0x072838b3,// 188 PAY 185 

    0x777a9d5e,// 189 PAY 186 

    0x6b46a34e,// 190 PAY 187 

    0xb4abaddf,// 191 PAY 188 

    0xd3e49609,// 192 PAY 189 

    0x12555f7f,// 193 PAY 190 

    0x572fb596,// 194 PAY 191 

    0x1a230aa1,// 195 PAY 192 

    0x8ee60f57,// 196 PAY 193 

    0xa1404a1e,// 197 PAY 194 

    0x4a53e79a,// 198 PAY 195 

    0x02d908a2,// 199 PAY 196 

    0x44210caf,// 200 PAY 197 

    0x934b7306,// 201 PAY 198 

    0x72c85721,// 202 PAY 199 

    0x7d0ed246,// 203 PAY 200 

    0x5b3f079d,// 204 PAY 201 

    0xcf2a29ba,// 205 PAY 202 

    0xc8a05856,// 206 PAY 203 

    0x39c86f69,// 207 PAY 204 

    0x6e01f1e7,// 208 PAY 205 

    0x1b820798,// 209 PAY 206 

    0x691494f6,// 210 PAY 207 

    0x48b06b7a,// 211 PAY 208 

    0xd3d10396,// 212 PAY 209 

    0xf6c29c2a,// 213 PAY 210 

    0x26d336c2,// 214 PAY 211 

    0x097981df,// 215 PAY 212 

    0x17a32dc3,// 216 PAY 213 

    0x38c82837,// 217 PAY 214 

    0x1ae6f6d0,// 218 PAY 215 

    0x69de543d,// 219 PAY 216 

    0x4d07eb09,// 220 PAY 217 

    0xa2c976da,// 221 PAY 218 

    0x0d608312,// 222 PAY 219 

    0x0e961ff6,// 223 PAY 220 

    0x3663b84e,// 224 PAY 221 

    0x05297ae8,// 225 PAY 222 

    0xa415eb95,// 226 PAY 223 

    0x55a23651,// 227 PAY 224 

    0xf616a6ad,// 228 PAY 225 

    0x2f01ee30,// 229 PAY 226 

    0xf979c810,// 230 PAY 227 

    0x231daeaa,// 231 PAY 228 

    0x55d3b72b,// 232 PAY 229 

    0x4958415c,// 233 PAY 230 

    0xcb89b432,// 234 PAY 231 

    0xf7c11c81,// 235 PAY 232 

    0xfa97c7fa,// 236 PAY 233 

    0xe273df96,// 237 PAY 234 

    0x2fd9a6fe,// 238 PAY 235 

    0x71e0405a,// 239 PAY 236 

    0xbc360ad6,// 240 PAY 237 

    0xe8ed31ec,// 241 PAY 238 

    0x660527f7,// 242 PAY 239 

    0xbe31859d,// 243 PAY 240 

    0x2bdbdc92,// 244 PAY 241 

    0xd3366938,// 245 PAY 242 

    0xd1c3ef76,// 246 PAY 243 

    0xa541d86d,// 247 PAY 244 

    0xf547dea1,// 248 PAY 245 

    0x7f8c6425,// 249 PAY 246 

    0xcc8184cd,// 250 PAY 247 

    0xd12eeeed,// 251 PAY 248 

    0x8270e563,// 252 PAY 249 

    0x31f0fea4,// 253 PAY 250 

    0xf5d1af29,// 254 PAY 251 

    0x3ce6e97c,// 255 PAY 252 

    0x44decd7f,// 256 PAY 253 

    0x4f082f64,// 257 PAY 254 

    0xb546f6aa,// 258 PAY 255 

    0x69b6dcb7,// 259 PAY 256 

    0x1168bab8,// 260 PAY 257 

    0xf1bb57d5,// 261 PAY 258 

    0xee01625f,// 262 PAY 259 

    0xf6e4310f,// 263 PAY 260 

    0x96647447,// 264 PAY 261 

    0xcbf7a517,// 265 PAY 262 

    0x698b9956,// 266 PAY 263 

    0xd52576be,// 267 PAY 264 

    0x2f85125a,// 268 PAY 265 

    0x84cd1d3d,// 269 PAY 266 

    0x77ec6f9b,// 270 PAY 267 

    0xc390ac71,// 271 PAY 268 

    0xc85cb8e8,// 272 PAY 269 

    0x0ca504f7,// 273 PAY 270 

    0x4fd65a3a,// 274 PAY 271 

    0xa35fc19a,// 275 PAY 272 

    0xdc5b8f3b,// 276 PAY 273 

    0xc89d1633,// 277 PAY 274 

    0x6f0c7a35,// 278 PAY 275 

    0x5bae64af,// 279 PAY 276 

    0x0a55fdec,// 280 PAY 277 

    0x35d4ab7f,// 281 PAY 278 

    0x2e3ea78c,// 282 PAY 279 

    0xf70ccc1e,// 283 PAY 280 

    0x79f1529c,// 284 PAY 281 

    0x462430db,// 285 PAY 282 

    0x3a95fe05,// 286 PAY 283 

    0x3af0d4f4,// 287 PAY 284 

    0x6da3c152,// 288 PAY 285 

    0xc14b69eb,// 289 PAY 286 

    0x96412c56,// 290 PAY 287 

    0xce8a29d9,// 291 PAY 288 

    0x03f2ca58,// 292 PAY 289 

    0xc7795aa1,// 293 PAY 290 

    0xf2f4e83d,// 294 PAY 291 

    0xad58d374,// 295 PAY 292 

    0x7f2d7588,// 296 PAY 293 

    0x363454cf,// 297 PAY 294 

    0xc709465d,// 298 PAY 295 

    0x5fc402b9,// 299 PAY 296 

    0x7bb9e6d0,// 300 PAY 297 

    0x47665145,// 301 PAY 298 

    0x93e86d5a,// 302 PAY 299 

    0x50019dd9,// 303 PAY 300 

    0x2b0f5f83,// 304 PAY 301 

    0x9c8cd746,// 305 PAY 302 

    0x3b237ef7,// 306 PAY 303 

    0xdb7ee634,// 307 PAY 304 

    0xd8cdcd48,// 308 PAY 305 

    0xfad5e45e,// 309 PAY 306 

    0x598d8941,// 310 PAY 307 

    0x6bcec74e,// 311 PAY 308 

    0x002f4dfe,// 312 PAY 309 

    0xce6025b6,// 313 PAY 310 

    0x61a235e2,// 314 PAY 311 

    0xbe80506a,// 315 PAY 312 

    0xda4aa89d,// 316 PAY 313 

    0x856e3ed8,// 317 PAY 314 

    0x55fcc3bf,// 318 PAY 315 

    0x1397b5db,// 319 PAY 316 

    0xa28a056a,// 320 PAY 317 

    0x8e30937d,// 321 PAY 318 

    0x479e152e,// 322 PAY 319 

    0x99629b4b,// 323 PAY 320 

    0x158d0b98,// 324 PAY 321 

    0x96b22da9,// 325 PAY 322 

    0x24497bfb,// 326 PAY 323 

    0x15cf157a,// 327 PAY 324 

    0xcbc444c6,// 328 PAY 325 

    0xe7169ea8,// 329 PAY 326 

    0xe63711c8,// 330 PAY 327 

    0xcb31e4a0,// 331 PAY 328 

    0xb65c115e,// 332 PAY 329 

    0x351f696a,// 333 PAY 330 

    0xe8a6879d,// 334 PAY 331 

    0x449b1b10,// 335 PAY 332 

    0x9e06a2a2,// 336 PAY 333 

    0xdb90462f,// 337 PAY 334 

    0x66252a6d,// 338 PAY 335 

    0x7556ad78,// 339 PAY 336 

    0x9c3e4d79,// 340 PAY 337 

    0x0bd0c981,// 341 PAY 338 

    0xd5e94b71,// 342 PAY 339 

    0x9fdfbdcd,// 343 PAY 340 

    0x798e5d1e,// 344 PAY 341 

    0x3ca240f9,// 345 PAY 342 

    0xa928c390,// 346 PAY 343 

    0xbcdd28e0,// 347 PAY 344 

    0x948443e5,// 348 PAY 345 

    0xac604b3a,// 349 PAY 346 

    0xe0def22e,// 350 PAY 347 

    0xb8633ea6,// 351 PAY 348 

    0x700e5a3b,// 352 PAY 349 

    0x54397cc8,// 353 PAY 350 

    0xb60fc621,// 354 PAY 351 

    0x085baf12,// 355 PAY 352 

    0xbf5dcade,// 356 PAY 353 

    0x557bc8d6,// 357 PAY 354 

    0x7b6412cd,// 358 PAY 355 

    0xafa13f68,// 359 PAY 356 

    0x76648053,// 360 PAY 357 

    0xd026c099,// 361 PAY 358 

    0x092e8fbd,// 362 PAY 359 

    0x0cc55c4f,// 363 PAY 360 

    0xfda16cad,// 364 PAY 361 

    0x20b4da78,// 365 PAY 362 

    0xd8956566,// 366 PAY 363 

    0xca532164,// 367 PAY 364 

    0xdcef7af3,// 368 PAY 365 

    0xb1237fae,// 369 PAY 366 

    0x9c518591,// 370 PAY 367 

    0xcd703820,// 371 PAY 368 

    0xfb8e78f1,// 372 PAY 369 

    0xb4a3079e,// 373 PAY 370 

    0x2de7ca94,// 374 PAY 371 

    0x31cbb286,// 375 PAY 372 

    0x6964dcf8,// 376 PAY 373 

    0x554ba1d3,// 377 PAY 374 

    0x6996a4c6,// 378 PAY 375 

    0x31a4d1ec,// 379 PAY 376 

    0xd319ff6d,// 380 PAY 377 

    0x64a4168d,// 381 PAY 378 

    0x8bba94fb,// 382 PAY 379 

    0xb9b3c23d,// 383 PAY 380 

    0xfbc0f715,// 384 PAY 381 

    0xb97de7b2,// 385 PAY 382 

    0x9d9f8b1d,// 386 PAY 383 

    0x5ca03371,// 387 PAY 384 

    0xdf66cdac,// 388 PAY 385 

    0xaca427ba,// 389 PAY 386 

    0x20771d99,// 390 PAY 387 

    0xd0d58363,// 391 PAY 388 

    0xf120effc,// 392 PAY 389 

    0xd87fea26,// 393 PAY 390 

    0x79a952b8,// 394 PAY 391 

    0xa574c610,// 395 PAY 392 

    0xe4e619ee,// 396 PAY 393 

    0xc8d52012,// 397 PAY 394 

    0x97c8cadf,// 398 PAY 395 

    0x3fb66276,// 399 PAY 396 

    0xc27a8454,// 400 PAY 397 

    0x4b128300,// 401 PAY 398 

    0xbd454a36,// 402 PAY 399 

    0x9fc379a7,// 403 PAY 400 

    0x03f5988f,// 404 PAY 401 

    0xd9fe1c8e,// 405 PAY 402 

    0xde8ad789,// 406 PAY 403 

    0x560e1d33,// 407 PAY 404 

    0xb639bdcc,// 408 PAY 405 

    0x26c89300,// 409 PAY 406 

    0xccce9495,// 410 PAY 407 

    0xd21282fa,// 411 PAY 408 

    0x706d9c07,// 412 PAY 409 

    0x624f33ad,// 413 PAY 410 

    0x4c96eb5f,// 414 PAY 411 

    0xdabfda44,// 415 PAY 412 

    0x4c9d1e87,// 416 PAY 413 

    0x8f351168,// 417 PAY 414 

    0xe676f3d8,// 418 PAY 415 

    0xc292146e,// 419 PAY 416 

    0xcb79ddcb,// 420 PAY 417 

    0xbf524c5e,// 421 PAY 418 

    0x5a740b52,// 422 PAY 419 

    0x9dbf7ebc,// 423 PAY 420 

    0x9f61b7de,// 424 PAY 421 

    0xe1526c9e,// 425 PAY 422 

    0x37f3708b,// 426 PAY 423 

    0x444dc7ec,// 427 PAY 424 

    0x48b501a1,// 428 PAY 425 

    0xdb0b045e,// 429 PAY 426 

    0x5bc9a037,// 430 PAY 427 

    0x14c92e01,// 431 PAY 428 

    0xedfc9cb8,// 432 PAY 429 

    0x82e70bef,// 433 PAY 430 

    0xb83b4dee,// 434 PAY 431 

    0x41fd0a57,// 435 PAY 432 

    0xdc0d3c01,// 436 PAY 433 

    0xa48ef85a,// 437 PAY 434 

    0x9c089a02,// 438 PAY 435 

    0x3edf4eb7,// 439 PAY 436 

    0xf4423e97,// 440 PAY 437 

    0x151d6115,// 441 PAY 438 

    0xf9f3080e,// 442 PAY 439 

    0xa43d573a,// 443 PAY 440 

    0x408a0824,// 444 PAY 441 

    0x1f9d0c1b,// 445 PAY 442 

    0xa14a4f18,// 446 PAY 443 

    0xcd0dbfeb,// 447 PAY 444 

    0x67cce776,// 448 PAY 445 

    0x64a3534e,// 449 PAY 446 

    0x9848aaf7,// 450 PAY 447 

    0x2eb42f85,// 451 PAY 448 

    0xcdc93192,// 452 PAY 449 

    0xb929bd14,// 453 PAY 450 

    0xdf0a404a,// 454 PAY 451 

    0x3993afb8,// 455 PAY 452 

    0x03881c18,// 456 PAY 453 

    0x2eca40b8,// 457 PAY 454 

    0x4ebbdd90,// 458 PAY 455 

    0x7731608a,// 459 PAY 456 

    0x08c58daf,// 460 PAY 457 

    0xd6ed142f,// 461 PAY 458 

    0x045ccdcb,// 462 PAY 459 

    0xed1eedf6,// 463 PAY 460 

    0xc1c32077,// 464 PAY 461 

    0xc93dca43,// 465 PAY 462 

    0x1f0b7f50,// 466 PAY 463 

    0x3409fbde,// 467 PAY 464 

    0x7e5f9352,// 468 PAY 465 

    0x30fe0127,// 469 PAY 466 

    0x7f0b6efd,// 470 PAY 467 

    0xdc10830a,// 471 PAY 468 

    0x7469b829,// 472 PAY 469 

    0xf93a2825,// 473 PAY 470 

    0x5fb17c88,// 474 PAY 471 

    0x7d1b5c75,// 475 PAY 472 

    0xc1cba335,// 476 PAY 473 

    0x874dd758,// 477 PAY 474 

    0xd5c3ff1a,// 478 PAY 475 

    0x2c8692dd,// 479 PAY 476 

    0x14000000,// 480 PAY 477 

/// HASH is  8 bytes 

    0xc1c32077,// 481 HSH   1 

    0xc93dca43,// 482 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 218 

/// STA pkt_idx        : 241 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x44 

    0x03c544da // 483 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt74_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 264 words. 

/// BDA size     is 1050 (0x41a) 

/// BDA id       is 0xa0be 

    0x041aa0be,// 3 BDA   1 

/// PAY Generic Data size   : 1050 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x81d412a6,// 4 PAY   1 

    0x2cc85308,// 5 PAY   2 

    0x812f2d65,// 6 PAY   3 

    0x80734e93,// 7 PAY   4 

    0x9b75e249,// 8 PAY   5 

    0xfe071e25,// 9 PAY   6 

    0xaf6646b9,// 10 PAY   7 

    0x61cf071a,// 11 PAY   8 

    0xb4e15e21,// 12 PAY   9 

    0xae119cc6,// 13 PAY  10 

    0x0ae46ecb,// 14 PAY  11 

    0xc59f929a,// 15 PAY  12 

    0x9e25402f,// 16 PAY  13 

    0x1247c380,// 17 PAY  14 

    0xeb34367e,// 18 PAY  15 

    0x7a22c6aa,// 19 PAY  16 

    0xfa47f032,// 20 PAY  17 

    0x02703359,// 21 PAY  18 

    0xc48349da,// 22 PAY  19 

    0xaf2e7bb4,// 23 PAY  20 

    0xaa3aaa60,// 24 PAY  21 

    0x2f74bac2,// 25 PAY  22 

    0x91ccac29,// 26 PAY  23 

    0xf54f44d5,// 27 PAY  24 

    0xbae23c5a,// 28 PAY  25 

    0x34cd7a28,// 29 PAY  26 

    0x8700f19d,// 30 PAY  27 

    0xbcd2f877,// 31 PAY  28 

    0x37175925,// 32 PAY  29 

    0x04cfdf73,// 33 PAY  30 

    0x0562638e,// 34 PAY  31 

    0x9b3e0207,// 35 PAY  32 

    0xe094e3c7,// 36 PAY  33 

    0xad67c4c5,// 37 PAY  34 

    0x2d8344b0,// 38 PAY  35 

    0xdfeabc5a,// 39 PAY  36 

    0x01ce7cf6,// 40 PAY  37 

    0x4934a073,// 41 PAY  38 

    0xb58a4b75,// 42 PAY  39 

    0xc74c15b6,// 43 PAY  40 

    0xee40a6c4,// 44 PAY  41 

    0x296e92bd,// 45 PAY  42 

    0xa5a8d7b3,// 46 PAY  43 

    0x579b7fa7,// 47 PAY  44 

    0xd4f44efd,// 48 PAY  45 

    0x4d5c1e44,// 49 PAY  46 

    0x7ef776ad,// 50 PAY  47 

    0x3dff29ea,// 51 PAY  48 

    0x7c7fae64,// 52 PAY  49 

    0xbae1540f,// 53 PAY  50 

    0x5dc9c42f,// 54 PAY  51 

    0x71443a6d,// 55 PAY  52 

    0x8d997fa8,// 56 PAY  53 

    0x1eda9222,// 57 PAY  54 

    0x5263bddb,// 58 PAY  55 

    0x21a9be3e,// 59 PAY  56 

    0xbdc6aea4,// 60 PAY  57 

    0xf97edb48,// 61 PAY  58 

    0xaa84707f,// 62 PAY  59 

    0x72d86587,// 63 PAY  60 

    0xd4f82320,// 64 PAY  61 

    0xb9c960a6,// 65 PAY  62 

    0xa9cb221b,// 66 PAY  63 

    0x651f0b2f,// 67 PAY  64 

    0x056dcf9f,// 68 PAY  65 

    0x4c47917e,// 69 PAY  66 

    0x5b3a0c5e,// 70 PAY  67 

    0x47ffab1f,// 71 PAY  68 

    0xd8175afb,// 72 PAY  69 

    0x04476140,// 73 PAY  70 

    0x85f9b020,// 74 PAY  71 

    0x1db35e27,// 75 PAY  72 

    0x72a9e3a8,// 76 PAY  73 

    0x27eb06bc,// 77 PAY  74 

    0x5ebfc441,// 78 PAY  75 

    0xecd048f9,// 79 PAY  76 

    0x037173c4,// 80 PAY  77 

    0x856c289c,// 81 PAY  78 

    0x27df5db0,// 82 PAY  79 

    0x8cee0092,// 83 PAY  80 

    0xe44b3ad0,// 84 PAY  81 

    0x365c1329,// 85 PAY  82 

    0xff382457,// 86 PAY  83 

    0x4afb0eee,// 87 PAY  84 

    0x41859ac5,// 88 PAY  85 

    0x2f3160f7,// 89 PAY  86 

    0x191305e8,// 90 PAY  87 

    0x4e00ea46,// 91 PAY  88 

    0xa2acbf15,// 92 PAY  89 

    0x5ecfdeec,// 93 PAY  90 

    0x00e83d82,// 94 PAY  91 

    0x1599638e,// 95 PAY  92 

    0xdfa9f1eb,// 96 PAY  93 

    0x3ec4f585,// 97 PAY  94 

    0x3e47b551,// 98 PAY  95 

    0x92f53c2b,// 99 PAY  96 

    0x96d2a57c,// 100 PAY  97 

    0x2b5b1bd7,// 101 PAY  98 

    0xdaf2bc51,// 102 PAY  99 

    0xbc935cc2,// 103 PAY 100 

    0xdcc967a6,// 104 PAY 101 

    0xd49c5cdc,// 105 PAY 102 

    0xd2b180b4,// 106 PAY 103 

    0x9127d59b,// 107 PAY 104 

    0x0031e187,// 108 PAY 105 

    0xe7a78cb3,// 109 PAY 106 

    0x33c3be7f,// 110 PAY 107 

    0xc920afdc,// 111 PAY 108 

    0x8169cd65,// 112 PAY 109 

    0xf26870f0,// 113 PAY 110 

    0x256245ee,// 114 PAY 111 

    0x6e87c2d3,// 115 PAY 112 

    0x0c7e4f1d,// 116 PAY 113 

    0xaee75cd0,// 117 PAY 114 

    0xcce57dc4,// 118 PAY 115 

    0x6a9b304e,// 119 PAY 116 

    0x6994b8dd,// 120 PAY 117 

    0xc616d8ff,// 121 PAY 118 

    0xfef3d0cc,// 122 PAY 119 

    0x3ae5304b,// 123 PAY 120 

    0xdb545a6b,// 124 PAY 121 

    0x171a6f04,// 125 PAY 122 

    0x32b5f5f0,// 126 PAY 123 

    0xf1466242,// 127 PAY 124 

    0x5f5ea4c6,// 128 PAY 125 

    0x94eaca01,// 129 PAY 126 

    0x1e100f3b,// 130 PAY 127 

    0xd3816c4a,// 131 PAY 128 

    0x0746d15c,// 132 PAY 129 

    0x1f53a79a,// 133 PAY 130 

    0xd79a854d,// 134 PAY 131 

    0x83cd4161,// 135 PAY 132 

    0xd57bbd2b,// 136 PAY 133 

    0x65368ca0,// 137 PAY 134 

    0xb0586a82,// 138 PAY 135 

    0xcd46b67c,// 139 PAY 136 

    0x6230717e,// 140 PAY 137 

    0x69bb35ae,// 141 PAY 138 

    0x75b59ae3,// 142 PAY 139 

    0x790465ab,// 143 PAY 140 

    0x1eea1cd9,// 144 PAY 141 

    0x26c7670f,// 145 PAY 142 

    0x9d34de4e,// 146 PAY 143 

    0xcbffd58f,// 147 PAY 144 

    0xf679d2f9,// 148 PAY 145 

    0x9be9a20e,// 149 PAY 146 

    0x98b0b8e7,// 150 PAY 147 

    0xf49fb0e7,// 151 PAY 148 

    0x6081bcaf,// 152 PAY 149 

    0x6df5f018,// 153 PAY 150 

    0x6e1971cb,// 154 PAY 151 

    0xb0cfbfa8,// 155 PAY 152 

    0x83506234,// 156 PAY 153 

    0xba898c05,// 157 PAY 154 

    0xb54983cb,// 158 PAY 155 

    0x2b81b82c,// 159 PAY 156 

    0x33e3f069,// 160 PAY 157 

    0x00317e1c,// 161 PAY 158 

    0xf4a79466,// 162 PAY 159 

    0x2705e3d6,// 163 PAY 160 

    0x49793e51,// 164 PAY 161 

    0xf66bfc85,// 165 PAY 162 

    0x52fcd4c0,// 166 PAY 163 

    0x8fb3de97,// 167 PAY 164 

    0x9a30fa54,// 168 PAY 165 

    0xda2581ef,// 169 PAY 166 

    0x1650eccc,// 170 PAY 167 

    0xa8c8b39c,// 171 PAY 168 

    0x38355013,// 172 PAY 169 

    0x3311889e,// 173 PAY 170 

    0xdb08c74a,// 174 PAY 171 

    0xa058fa12,// 175 PAY 172 

    0x96b80012,// 176 PAY 173 

    0xc1e412aa,// 177 PAY 174 

    0xe947de80,// 178 PAY 175 

    0x47141fc3,// 179 PAY 176 

    0x44268174,// 180 PAY 177 

    0xc32cc647,// 181 PAY 178 

    0x98abbe7f,// 182 PAY 179 

    0x14f1743d,// 183 PAY 180 

    0xe9a4a329,// 184 PAY 181 

    0xb52a08e6,// 185 PAY 182 

    0x18eeadd3,// 186 PAY 183 

    0x66bbf8ae,// 187 PAY 184 

    0x6c9f2a31,// 188 PAY 185 

    0xb68419b5,// 189 PAY 186 

    0x72349853,// 190 PAY 187 

    0x3061dd4e,// 191 PAY 188 

    0xc9ed3787,// 192 PAY 189 

    0xec90d578,// 193 PAY 190 

    0x9aac210f,// 194 PAY 191 

    0x46abaaf7,// 195 PAY 192 

    0x4bd085d1,// 196 PAY 193 

    0x1338b232,// 197 PAY 194 

    0x111be187,// 198 PAY 195 

    0x154e5c58,// 199 PAY 196 

    0x3b2ac451,// 200 PAY 197 

    0x72b6cfce,// 201 PAY 198 

    0x8f286513,// 202 PAY 199 

    0x77f9d5a5,// 203 PAY 200 

    0x50efa950,// 204 PAY 201 

    0xfdeb8472,// 205 PAY 202 

    0x73c399d2,// 206 PAY 203 

    0xfe598f54,// 207 PAY 204 

    0xd1fc3ba4,// 208 PAY 205 

    0x8508972e,// 209 PAY 206 

    0xe34aea4f,// 210 PAY 207 

    0xceb023ca,// 211 PAY 208 

    0xaf93fb4d,// 212 PAY 209 

    0x5d7fe955,// 213 PAY 210 

    0x62058bc5,// 214 PAY 211 

    0x487785e6,// 215 PAY 212 

    0xf3e250d5,// 216 PAY 213 

    0x91ce1654,// 217 PAY 214 

    0x3faa2785,// 218 PAY 215 

    0x525289a3,// 219 PAY 216 

    0xee536ca3,// 220 PAY 217 

    0x87c301ef,// 221 PAY 218 

    0x738b4746,// 222 PAY 219 

    0x41758eb9,// 223 PAY 220 

    0x3453462e,// 224 PAY 221 

    0x10d6a591,// 225 PAY 222 

    0xe7353d38,// 226 PAY 223 

    0x00dbc516,// 227 PAY 224 

    0x43666421,// 228 PAY 225 

    0x0e02ca09,// 229 PAY 226 

    0x626b933d,// 230 PAY 227 

    0x01dbdeea,// 231 PAY 228 

    0xb75a5f3e,// 232 PAY 229 

    0x9f5ae494,// 233 PAY 230 

    0x1815f132,// 234 PAY 231 

    0xaaecf338,// 235 PAY 232 

    0xb9bfe2b9,// 236 PAY 233 

    0xdedf8d50,// 237 PAY 234 

    0xa98732f7,// 238 PAY 235 

    0xf7ee8896,// 239 PAY 236 

    0xa3ab4bf2,// 240 PAY 237 

    0x969641f4,// 241 PAY 238 

    0xe14fe560,// 242 PAY 239 

    0x2606912b,// 243 PAY 240 

    0x84ee38c1,// 244 PAY 241 

    0xaa121dd3,// 245 PAY 242 

    0xb0f532ec,// 246 PAY 243 

    0xf93b7e43,// 247 PAY 244 

    0x7b635909,// 248 PAY 245 

    0x60460fb4,// 249 PAY 246 

    0x6115df7b,// 250 PAY 247 

    0x73fd209a,// 251 PAY 248 

    0x81850ba7,// 252 PAY 249 

    0xd4835109,// 253 PAY 250 

    0x0e6da804,// 254 PAY 251 

    0x149ee413,// 255 PAY 252 

    0x68c31c4a,// 256 PAY 253 

    0x489dda12,// 257 PAY 254 

    0xf071e4ae,// 258 PAY 255 

    0x2c3743d7,// 259 PAY 256 

    0xce2b72a3,// 260 PAY 257 

    0xd1e010a2,// 261 PAY 258 

    0x3f28f888,// 262 PAY 259 

    0x1156fac5,// 263 PAY 260 

    0x139a93a6,// 264 PAY 261 

    0x35500adb,// 265 PAY 262 

    0x88150000,// 266 PAY 263 

/// HASH is  8 bytes 

    0xd1e010a2,// 267 HSH   1 

    0x3f28f888,// 268 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 111 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x01f04d6f // 269 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt75_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 440 words. 

/// BDA size     is 1754 (0x6da) 

/// BDA id       is 0xec5d 

    0x06daec5d,// 3 BDA   1 

/// PAY Generic Data size   : 1754 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x01c8d7ac,// 4 PAY   1 

    0x74048048,// 5 PAY   2 

    0xc12fea9a,// 6 PAY   3 

    0xb93c9e5a,// 7 PAY   4 

    0x848fdf48,// 8 PAY   5 

    0x033ee831,// 9 PAY   6 

    0xf316f7d1,// 10 PAY   7 

    0x1b4f84a6,// 11 PAY   8 

    0xa0e9f697,// 12 PAY   9 

    0xc7f6b567,// 13 PAY  10 

    0xcbdb3466,// 14 PAY  11 

    0x78b306c7,// 15 PAY  12 

    0xe82d37d2,// 16 PAY  13 

    0xff3a6934,// 17 PAY  14 

    0x69f16c49,// 18 PAY  15 

    0x1d73230f,// 19 PAY  16 

    0x9ea291d7,// 20 PAY  17 

    0x07c7ec22,// 21 PAY  18 

    0xdfdc0c6e,// 22 PAY  19 

    0xa055e028,// 23 PAY  20 

    0xe6286730,// 24 PAY  21 

    0x273cc1e9,// 25 PAY  22 

    0x4241c85c,// 26 PAY  23 

    0xbb090f39,// 27 PAY  24 

    0x0a903727,// 28 PAY  25 

    0x02f28f50,// 29 PAY  26 

    0x49654a69,// 30 PAY  27 

    0x991ecbe5,// 31 PAY  28 

    0xccc0724e,// 32 PAY  29 

    0x766e22a1,// 33 PAY  30 

    0xdcc1ed99,// 34 PAY  31 

    0x81a7efea,// 35 PAY  32 

    0x5296cf73,// 36 PAY  33 

    0x9cc13b28,// 37 PAY  34 

    0x8c04ea9d,// 38 PAY  35 

    0x6049be21,// 39 PAY  36 

    0x0628b9d4,// 40 PAY  37 

    0x7d6a2649,// 41 PAY  38 

    0x576ac4ce,// 42 PAY  39 

    0x7d268f65,// 43 PAY  40 

    0x2a11e3b0,// 44 PAY  41 

    0x156eb59a,// 45 PAY  42 

    0x05da577a,// 46 PAY  43 

    0xd9593655,// 47 PAY  44 

    0x7a8b8247,// 48 PAY  45 

    0xdd0d85fb,// 49 PAY  46 

    0xd70447fb,// 50 PAY  47 

    0xd89278ea,// 51 PAY  48 

    0x9f7ae760,// 52 PAY  49 

    0xfac4046d,// 53 PAY  50 

    0x6798d58c,// 54 PAY  51 

    0x79f1af43,// 55 PAY  52 

    0x4e5d13a6,// 56 PAY  53 

    0x55134754,// 57 PAY  54 

    0x04e4692c,// 58 PAY  55 

    0x87a576fa,// 59 PAY  56 

    0xb3ae88ad,// 60 PAY  57 

    0x46684fde,// 61 PAY  58 

    0x77ee3484,// 62 PAY  59 

    0xa95396dc,// 63 PAY  60 

    0x79cfdbda,// 64 PAY  61 

    0xc9171cb9,// 65 PAY  62 

    0xc06e3208,// 66 PAY  63 

    0xb1072a52,// 67 PAY  64 

    0x211e95ed,// 68 PAY  65 

    0x3114258e,// 69 PAY  66 

    0x3e0d0c38,// 70 PAY  67 

    0xfdb9e7ab,// 71 PAY  68 

    0xb1691085,// 72 PAY  69 

    0x37fb6479,// 73 PAY  70 

    0x637e8d3e,// 74 PAY  71 

    0x0a2d83a5,// 75 PAY  72 

    0x9ab66010,// 76 PAY  73 

    0xf9b988ed,// 77 PAY  74 

    0x9c665b04,// 78 PAY  75 

    0xf8b69799,// 79 PAY  76 

    0x60c693cc,// 80 PAY  77 

    0xcc8cbaf1,// 81 PAY  78 

    0x6a2197bd,// 82 PAY  79 

    0x22ee16e1,// 83 PAY  80 

    0x6ff8ede5,// 84 PAY  81 

    0x6360056f,// 85 PAY  82 

    0xfce3c065,// 86 PAY  83 

    0xab6a0261,// 87 PAY  84 

    0xaf63f9f1,// 88 PAY  85 

    0x859b1fb2,// 89 PAY  86 

    0x01dc02be,// 90 PAY  87 

    0x0ba26857,// 91 PAY  88 

    0x4d6dcc72,// 92 PAY  89 

    0x7c9b7dba,// 93 PAY  90 

    0x99aaa779,// 94 PAY  91 

    0x74e7056d,// 95 PAY  92 

    0xccc142f4,// 96 PAY  93 

    0x335d3d3b,// 97 PAY  94 

    0x25477c18,// 98 PAY  95 

    0x845d4549,// 99 PAY  96 

    0x5058a773,// 100 PAY  97 

    0x88c81101,// 101 PAY  98 

    0x2c377e5f,// 102 PAY  99 

    0x6c5a157d,// 103 PAY 100 

    0xb9e8d83c,// 104 PAY 101 

    0x3635ccba,// 105 PAY 102 

    0x16299061,// 106 PAY 103 

    0xbb34579c,// 107 PAY 104 

    0x102f6a1b,// 108 PAY 105 

    0x0863feee,// 109 PAY 106 

    0x6c611e82,// 110 PAY 107 

    0x324144a7,// 111 PAY 108 

    0x8042743a,// 112 PAY 109 

    0xa4ca96ed,// 113 PAY 110 

    0x08c49431,// 114 PAY 111 

    0x3169a774,// 115 PAY 112 

    0xfda07886,// 116 PAY 113 

    0x6ed2d6ae,// 117 PAY 114 

    0x85130d27,// 118 PAY 115 

    0x48a1f0ac,// 119 PAY 116 

    0xcf3dc58c,// 120 PAY 117 

    0xed46fc5b,// 121 PAY 118 

    0xb603259c,// 122 PAY 119 

    0x4389e559,// 123 PAY 120 

    0xed7d6e53,// 124 PAY 121 

    0xbb222be8,// 125 PAY 122 

    0xe80a3c60,// 126 PAY 123 

    0x25dd6bd3,// 127 PAY 124 

    0xfa996443,// 128 PAY 125 

    0xdd68b91a,// 129 PAY 126 

    0xfd12653b,// 130 PAY 127 

    0xb3d4d4f4,// 131 PAY 128 

    0xf7f3b4cb,// 132 PAY 129 

    0x51863a67,// 133 PAY 130 

    0xf95ecb32,// 134 PAY 131 

    0x4e5292ff,// 135 PAY 132 

    0x1078a75c,// 136 PAY 133 

    0xf4b16e9c,// 137 PAY 134 

    0xc761e40d,// 138 PAY 135 

    0x718da36c,// 139 PAY 136 

    0xd36d1814,// 140 PAY 137 

    0x7144bde2,// 141 PAY 138 

    0x0b2cb55d,// 142 PAY 139 

    0x902b34db,// 143 PAY 140 

    0xcd395b34,// 144 PAY 141 

    0x7060f384,// 145 PAY 142 

    0xdfc1f9ce,// 146 PAY 143 

    0xe5921e99,// 147 PAY 144 

    0x2a9239a8,// 148 PAY 145 

    0x9b4a7837,// 149 PAY 146 

    0x6fe45c67,// 150 PAY 147 

    0x0ca631c1,// 151 PAY 148 

    0x51850fbe,// 152 PAY 149 

    0xe9954492,// 153 PAY 150 

    0x11b06f85,// 154 PAY 151 

    0xbc3ef2f0,// 155 PAY 152 

    0x2b6ecd20,// 156 PAY 153 

    0x59a0a286,// 157 PAY 154 

    0x1e9ec3b3,// 158 PAY 155 

    0x78ad21a6,// 159 PAY 156 

    0x6434626c,// 160 PAY 157 

    0xe982e302,// 161 PAY 158 

    0x93a87232,// 162 PAY 159 

    0x995eecc0,// 163 PAY 160 

    0x34a89ed5,// 164 PAY 161 

    0x542cc95d,// 165 PAY 162 

    0xa09039df,// 166 PAY 163 

    0x634ac035,// 167 PAY 164 

    0x16ab99c4,// 168 PAY 165 

    0x7cbc23d9,// 169 PAY 166 

    0x64978232,// 170 PAY 167 

    0x028d2da0,// 171 PAY 168 

    0x8685f3d8,// 172 PAY 169 

    0x408d0cfc,// 173 PAY 170 

    0x164dfdc3,// 174 PAY 171 

    0xcdde7500,// 175 PAY 172 

    0xfaaf99de,// 176 PAY 173 

    0xd6c91923,// 177 PAY 174 

    0x56a4413c,// 178 PAY 175 

    0xcfc34069,// 179 PAY 176 

    0x03a6d173,// 180 PAY 177 

    0x43ff130a,// 181 PAY 178 

    0x9546f30e,// 182 PAY 179 

    0xf429e2d2,// 183 PAY 180 

    0xf3ca7342,// 184 PAY 181 

    0x9f789bf9,// 185 PAY 182 

    0xfcd9e583,// 186 PAY 183 

    0xeb714614,// 187 PAY 184 

    0xfe9aa0fd,// 188 PAY 185 

    0xc0e7ee1e,// 189 PAY 186 

    0x86febbaf,// 190 PAY 187 

    0x762605f0,// 191 PAY 188 

    0x3db7f61b,// 192 PAY 189 

    0x18fe7652,// 193 PAY 190 

    0xd511bc0e,// 194 PAY 191 

    0xe92347d4,// 195 PAY 192 

    0x81b7ec0f,// 196 PAY 193 

    0x8952300b,// 197 PAY 194 

    0x8488a0c5,// 198 PAY 195 

    0x48ba0f5a,// 199 PAY 196 

    0x7987d5d7,// 200 PAY 197 

    0xf4e0f882,// 201 PAY 198 

    0xcfffe034,// 202 PAY 199 

    0x41b6e3a8,// 203 PAY 200 

    0x7fc5ae6e,// 204 PAY 201 

    0x58419727,// 205 PAY 202 

    0x84c0cb4a,// 206 PAY 203 

    0x1d6000fc,// 207 PAY 204 

    0x60aa8138,// 208 PAY 205 

    0x84d97585,// 209 PAY 206 

    0x72f70149,// 210 PAY 207 

    0x4e5c5a00,// 211 PAY 208 

    0xd10d5fe7,// 212 PAY 209 

    0x68713a3f,// 213 PAY 210 

    0x5dc95f23,// 214 PAY 211 

    0xaca17107,// 215 PAY 212 

    0x1baae811,// 216 PAY 213 

    0xbdd6f988,// 217 PAY 214 

    0xa7bc68e2,// 218 PAY 215 

    0xd0c47522,// 219 PAY 216 

    0xeaab7163,// 220 PAY 217 

    0x079f277c,// 221 PAY 218 

    0x753a4492,// 222 PAY 219 

    0x2d2e92f2,// 223 PAY 220 

    0x21c13608,// 224 PAY 221 

    0x747cebab,// 225 PAY 222 

    0x131c0f82,// 226 PAY 223 

    0x7993633c,// 227 PAY 224 

    0x627bff5a,// 228 PAY 225 

    0x9c1be560,// 229 PAY 226 

    0x6c14b4dc,// 230 PAY 227 

    0x54bbd15f,// 231 PAY 228 

    0xa18cdab7,// 232 PAY 229 

    0x267cd62f,// 233 PAY 230 

    0xa46942be,// 234 PAY 231 

    0x7cd425c3,// 235 PAY 232 

    0x8ac45d72,// 236 PAY 233 

    0xa8c5b12a,// 237 PAY 234 

    0x165073df,// 238 PAY 235 

    0x7895da27,// 239 PAY 236 

    0x74c56897,// 240 PAY 237 

    0x18f50289,// 241 PAY 238 

    0x29055093,// 242 PAY 239 

    0x9da5a4ac,// 243 PAY 240 

    0xb0708275,// 244 PAY 241 

    0x2b8970e0,// 245 PAY 242 

    0x4e264700,// 246 PAY 243 

    0x7337d99e,// 247 PAY 244 

    0xb8eddaa8,// 248 PAY 245 

    0xcf3c8e15,// 249 PAY 246 

    0xb99ef9df,// 250 PAY 247 

    0xac629c70,// 251 PAY 248 

    0x40633ce5,// 252 PAY 249 

    0xf2b7f80c,// 253 PAY 250 

    0xb854bab7,// 254 PAY 251 

    0x74110381,// 255 PAY 252 

    0x56c4c116,// 256 PAY 253 

    0x28672516,// 257 PAY 254 

    0xd276d20d,// 258 PAY 255 

    0x8dbd196e,// 259 PAY 256 

    0x4bfaded0,// 260 PAY 257 

    0x00def49c,// 261 PAY 258 

    0xb4ebbd80,// 262 PAY 259 

    0xff3c7459,// 263 PAY 260 

    0x36f5fe95,// 264 PAY 261 

    0x62435af1,// 265 PAY 262 

    0xe2e25157,// 266 PAY 263 

    0x172034d2,// 267 PAY 264 

    0x7e0edced,// 268 PAY 265 

    0x5faf6659,// 269 PAY 266 

    0x41223389,// 270 PAY 267 

    0x5f033623,// 271 PAY 268 

    0xc5c0aa31,// 272 PAY 269 

    0xf8deec3b,// 273 PAY 270 

    0x4f6f75a3,// 274 PAY 271 

    0x0f760e69,// 275 PAY 272 

    0xec743082,// 276 PAY 273 

    0xaa397094,// 277 PAY 274 

    0xd941c36e,// 278 PAY 275 

    0x446ca146,// 279 PAY 276 

    0x35af4cc7,// 280 PAY 277 

    0x381dc555,// 281 PAY 278 

    0xdabcf880,// 282 PAY 279 

    0x82040dd6,// 283 PAY 280 

    0x11b4b910,// 284 PAY 281 

    0xf53a5a8e,// 285 PAY 282 

    0x3c078c11,// 286 PAY 283 

    0x8ebe6c43,// 287 PAY 284 

    0x02fbb658,// 288 PAY 285 

    0xbdb95197,// 289 PAY 286 

    0xee282435,// 290 PAY 287 

    0x4a509933,// 291 PAY 288 

    0x2b2dab1b,// 292 PAY 289 

    0x3b6b133c,// 293 PAY 290 

    0x952092cf,// 294 PAY 291 

    0x93b0d4c4,// 295 PAY 292 

    0x26c46f2c,// 296 PAY 293 

    0x0e713b04,// 297 PAY 294 

    0xcb773a52,// 298 PAY 295 

    0xe88a24da,// 299 PAY 296 

    0x9e41ff70,// 300 PAY 297 

    0x6533f0cc,// 301 PAY 298 

    0xb1c43086,// 302 PAY 299 

    0x797484ad,// 303 PAY 300 

    0xd517fe5e,// 304 PAY 301 

    0x643583a0,// 305 PAY 302 

    0x834d81a6,// 306 PAY 303 

    0x8fe74c29,// 307 PAY 304 

    0xdbe5e2e4,// 308 PAY 305 

    0x67e72da5,// 309 PAY 306 

    0xc42125cb,// 310 PAY 307 

    0x03cb6e75,// 311 PAY 308 

    0xcb9a6624,// 312 PAY 309 

    0xbbe5dd3f,// 313 PAY 310 

    0x05d5e2fe,// 314 PAY 311 

    0x6017179d,// 315 PAY 312 

    0x60649b74,// 316 PAY 313 

    0xf2837982,// 317 PAY 314 

    0xc9deaf13,// 318 PAY 315 

    0xcc5c822c,// 319 PAY 316 

    0x41144892,// 320 PAY 317 

    0xc113d1ba,// 321 PAY 318 

    0x632ebfe0,// 322 PAY 319 

    0x2ff6587f,// 323 PAY 320 

    0x3d5b7d3a,// 324 PAY 321 

    0x09eec3cb,// 325 PAY 322 

    0xf1deb2b4,// 326 PAY 323 

    0x9f993a5c,// 327 PAY 324 

    0xe3d0ddb5,// 328 PAY 325 

    0x3817c5f6,// 329 PAY 326 

    0x98ceb0c6,// 330 PAY 327 

    0x766113dd,// 331 PAY 328 

    0xcdab5bad,// 332 PAY 329 

    0xec1e7096,// 333 PAY 330 

    0x479cdab4,// 334 PAY 331 

    0x31aa2213,// 335 PAY 332 

    0x2a544efb,// 336 PAY 333 

    0x78a1a822,// 337 PAY 334 

    0xd82d4567,// 338 PAY 335 

    0xd8fdadcf,// 339 PAY 336 

    0xa030b99a,// 340 PAY 337 

    0x16c3192f,// 341 PAY 338 

    0x3a547823,// 342 PAY 339 

    0x74aac38a,// 343 PAY 340 

    0xb60de313,// 344 PAY 341 

    0xd71835f8,// 345 PAY 342 

    0x6c920af3,// 346 PAY 343 

    0x217782e0,// 347 PAY 344 

    0x09e8397f,// 348 PAY 345 

    0x47abbdde,// 349 PAY 346 

    0x42ca9f58,// 350 PAY 347 

    0x6cef5265,// 351 PAY 348 

    0x54a9aabe,// 352 PAY 349 

    0x9bb72479,// 353 PAY 350 

    0x64ed60da,// 354 PAY 351 

    0xddf3b35f,// 355 PAY 352 

    0x9480271c,// 356 PAY 353 

    0x6f8b9959,// 357 PAY 354 

    0x4f59ec9f,// 358 PAY 355 

    0xbc5145fe,// 359 PAY 356 

    0x8cb55043,// 360 PAY 357 

    0x89317868,// 361 PAY 358 

    0x836899a6,// 362 PAY 359 

    0xa891d6ae,// 363 PAY 360 

    0x0b86cce6,// 364 PAY 361 

    0xe9eb057b,// 365 PAY 362 

    0x6b0bffba,// 366 PAY 363 

    0xc9fc98de,// 367 PAY 364 

    0xa3de6a84,// 368 PAY 365 

    0x92a91bb6,// 369 PAY 366 

    0xc50a3d44,// 370 PAY 367 

    0x1997fdda,// 371 PAY 368 

    0x88272f7b,// 372 PAY 369 

    0xf6883a3b,// 373 PAY 370 

    0xd15af502,// 374 PAY 371 

    0x4a971ae3,// 375 PAY 372 

    0xb805b529,// 376 PAY 373 

    0xaa10e06b,// 377 PAY 374 

    0x710610f6,// 378 PAY 375 

    0x3436aec4,// 379 PAY 376 

    0xdd50ccf5,// 380 PAY 377 

    0x4ec43d8e,// 381 PAY 378 

    0x71d3ba3a,// 382 PAY 379 

    0x63e8e08e,// 383 PAY 380 

    0xa8f3a476,// 384 PAY 381 

    0x9f962bd8,// 385 PAY 382 

    0xfa233a8d,// 386 PAY 383 

    0xc2da2794,// 387 PAY 384 

    0x0cee2ec5,// 388 PAY 385 

    0xa1de9b66,// 389 PAY 386 

    0xe2c30d53,// 390 PAY 387 

    0x7614c910,// 391 PAY 388 

    0xe62497ee,// 392 PAY 389 

    0xb4995084,// 393 PAY 390 

    0x35c15f31,// 394 PAY 391 

    0x7017f65b,// 395 PAY 392 

    0xed7c2e18,// 396 PAY 393 

    0xd3dc05e4,// 397 PAY 394 

    0x5fb695ab,// 398 PAY 395 

    0xa8c7062e,// 399 PAY 396 

    0x8baa42bc,// 400 PAY 397 

    0x418d1931,// 401 PAY 398 

    0xfad492cc,// 402 PAY 399 

    0xeb9f3ebe,// 403 PAY 400 

    0x143f8810,// 404 PAY 401 

    0xb9d5270d,// 405 PAY 402 

    0xe0615aeb,// 406 PAY 403 

    0xeb30cc52,// 407 PAY 404 

    0x1486cf9a,// 408 PAY 405 

    0xf00c27fb,// 409 PAY 406 

    0x78c2d862,// 410 PAY 407 

    0x3585e71f,// 411 PAY 408 

    0x508a3576,// 412 PAY 409 

    0x730218d1,// 413 PAY 410 

    0x317ab156,// 414 PAY 411 

    0x7f10e41b,// 415 PAY 412 

    0x36c02d08,// 416 PAY 413 

    0xe016a3eb,// 417 PAY 414 

    0x7e44bf63,// 418 PAY 415 

    0x6e253d43,// 419 PAY 416 

    0x96da5ac1,// 420 PAY 417 

    0x81ea9a2d,// 421 PAY 418 

    0x3b57abd2,// 422 PAY 419 

    0x2a02f90a,// 423 PAY 420 

    0x8f224017,// 424 PAY 421 

    0xc18144c3,// 425 PAY 422 

    0xe0ada1ad,// 426 PAY 423 

    0x3c52aac9,// 427 PAY 424 

    0x39adc753,// 428 PAY 425 

    0xa7c5f334,// 429 PAY 426 

    0x3e3eac07,// 430 PAY 427 

    0xfa9ac5df,// 431 PAY 428 

    0x9d4abf1a,// 432 PAY 429 

    0xa063a1e8,// 433 PAY 430 

    0x8ed1e4c3,// 434 PAY 431 

    0x4f2b5333,// 435 PAY 432 

    0xd1018462,// 436 PAY 433 

    0x3cbec1d5,// 437 PAY 434 

    0xdbca9d94,// 438 PAY 435 

    0x24da9978,// 439 PAY 436 

    0x58b77315,// 440 PAY 437 

    0x097ee72f,// 441 PAY 438 

    0x8d850000,// 442 PAY 439 

/// STA is 1 words. 

/// STA num_pkts       : 147 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x96 

    0x02d09693 // 443 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt76_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 303 words. 

/// BDA size     is 1206 (0x4b6) 

/// BDA id       is 0xd76a 

    0x04b6d76a,// 3 BDA   1 

/// PAY Generic Data size   : 1206 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xb70bd23b,// 4 PAY   1 

    0x88602dbd,// 5 PAY   2 

    0x285e8169,// 6 PAY   3 

    0xf3e9bd9b,// 7 PAY   4 

    0xdf9754d2,// 8 PAY   5 

    0xb9db87a2,// 9 PAY   6 

    0x86040217,// 10 PAY   7 

    0xd16007e4,// 11 PAY   8 

    0x3b1d1e59,// 12 PAY   9 

    0xa031ad82,// 13 PAY  10 

    0xd9b4cc97,// 14 PAY  11 

    0x80a35ca0,// 15 PAY  12 

    0x8f245fba,// 16 PAY  13 

    0xc0d4f5a7,// 17 PAY  14 

    0xe607650e,// 18 PAY  15 

    0x7023d959,// 19 PAY  16 

    0xae43e514,// 20 PAY  17 

    0xdc53d9b8,// 21 PAY  18 

    0x03ea3e82,// 22 PAY  19 

    0x60d4bccb,// 23 PAY  20 

    0x99622489,// 24 PAY  21 

    0xeb6c3458,// 25 PAY  22 

    0x99720159,// 26 PAY  23 

    0x4724d605,// 27 PAY  24 

    0x1dc8b1d4,// 28 PAY  25 

    0x6ebad712,// 29 PAY  26 

    0xe42241c5,// 30 PAY  27 

    0x5948f633,// 31 PAY  28 

    0x25265d36,// 32 PAY  29 

    0x961964dd,// 33 PAY  30 

    0x756f372d,// 34 PAY  31 

    0x986d2bd9,// 35 PAY  32 

    0xc0937062,// 36 PAY  33 

    0xca018cf0,// 37 PAY  34 

    0x98a44638,// 38 PAY  35 

    0xb59c0ac7,// 39 PAY  36 

    0xb9924983,// 40 PAY  37 

    0xb0c8b555,// 41 PAY  38 

    0xa4cc5f6e,// 42 PAY  39 

    0x96cb929a,// 43 PAY  40 

    0xa9f9f95c,// 44 PAY  41 

    0x8daf86cf,// 45 PAY  42 

    0x1a7c20c8,// 46 PAY  43 

    0xb1b60d0c,// 47 PAY  44 

    0x2b554c84,// 48 PAY  45 

    0x10e31230,// 49 PAY  46 

    0x8ba22332,// 50 PAY  47 

    0xcb46a029,// 51 PAY  48 

    0x9dbedcfc,// 52 PAY  49 

    0xadb4988c,// 53 PAY  50 

    0xa4e92bd7,// 54 PAY  51 

    0xdee07436,// 55 PAY  52 

    0xd17fffcb,// 56 PAY  53 

    0xe2ef5730,// 57 PAY  54 

    0x17080d11,// 58 PAY  55 

    0x9ef9ff22,// 59 PAY  56 

    0x99459a19,// 60 PAY  57 

    0x9d48c20e,// 61 PAY  58 

    0xb7257d77,// 62 PAY  59 

    0xa8baf3f9,// 63 PAY  60 

    0xe762f1ba,// 64 PAY  61 

    0x5affb523,// 65 PAY  62 

    0x54a2b300,// 66 PAY  63 

    0xee7e71d9,// 67 PAY  64 

    0x2857a653,// 68 PAY  65 

    0xfc7e6cdb,// 69 PAY  66 

    0x118f31c6,// 70 PAY  67 

    0xe88ea549,// 71 PAY  68 

    0xa46a3709,// 72 PAY  69 

    0xd650139e,// 73 PAY  70 

    0xc41a7d2b,// 74 PAY  71 

    0xb838011b,// 75 PAY  72 

    0x5ce2d841,// 76 PAY  73 

    0x996c8b92,// 77 PAY  74 

    0xd918350f,// 78 PAY  75 

    0xe74866d1,// 79 PAY  76 

    0x276cf890,// 80 PAY  77 

    0x0590eb8d,// 81 PAY  78 

    0xa2f86e69,// 82 PAY  79 

    0xd45ef1e0,// 83 PAY  80 

    0x4c2017db,// 84 PAY  81 

    0x8189c25b,// 85 PAY  82 

    0xa0bfbe7e,// 86 PAY  83 

    0x29548b55,// 87 PAY  84 

    0xc6eb2328,// 88 PAY  85 

    0xbaaa31d7,// 89 PAY  86 

    0xca024a53,// 90 PAY  87 

    0x475973ad,// 91 PAY  88 

    0x06c8fe74,// 92 PAY  89 

    0x31450756,// 93 PAY  90 

    0x5c7098f2,// 94 PAY  91 

    0x3663d52e,// 95 PAY  92 

    0xc1522d50,// 96 PAY  93 

    0xad5217ba,// 97 PAY  94 

    0xc8755afa,// 98 PAY  95 

    0x368c7622,// 99 PAY  96 

    0x2e87a51b,// 100 PAY  97 

    0xfa97cbb9,// 101 PAY  98 

    0x1986a696,// 102 PAY  99 

    0x3cd0a802,// 103 PAY 100 

    0x00d3ac23,// 104 PAY 101 

    0xb258a197,// 105 PAY 102 

    0xe29b1f2f,// 106 PAY 103 

    0xf73a015d,// 107 PAY 104 

    0x7611c9e1,// 108 PAY 105 

    0xa29d69b3,// 109 PAY 106 

    0x896a57ac,// 110 PAY 107 

    0x3eaee271,// 111 PAY 108 

    0xf0872f02,// 112 PAY 109 

    0x68da8af5,// 113 PAY 110 

    0x0149de5e,// 114 PAY 111 

    0x7a4aa272,// 115 PAY 112 

    0x2fcf4cc5,// 116 PAY 113 

    0xa32e6034,// 117 PAY 114 

    0xf939e570,// 118 PAY 115 

    0x5ae9709a,// 119 PAY 116 

    0x37290729,// 120 PAY 117 

    0xe27d6154,// 121 PAY 118 

    0x573dbb0e,// 122 PAY 119 

    0x7fe132d0,// 123 PAY 120 

    0x469e6318,// 124 PAY 121 

    0x0cf2b0f9,// 125 PAY 122 

    0xa72977d2,// 126 PAY 123 

    0x7d37d0d4,// 127 PAY 124 

    0xac9d1a77,// 128 PAY 125 

    0xfbbaabdb,// 129 PAY 126 

    0x00e75740,// 130 PAY 127 

    0xac1a0f9d,// 131 PAY 128 

    0xbf44b522,// 132 PAY 129 

    0x77026dd9,// 133 PAY 130 

    0x8c76ade3,// 134 PAY 131 

    0x20e8ced3,// 135 PAY 132 

    0x07ff145d,// 136 PAY 133 

    0xab0d6ec3,// 137 PAY 134 

    0x393b4d33,// 138 PAY 135 

    0xc0f2ed49,// 139 PAY 136 

    0x4459f623,// 140 PAY 137 

    0x1c41e7e4,// 141 PAY 138 

    0x49232bc0,// 142 PAY 139 

    0x2f774fc4,// 143 PAY 140 

    0xc57af5da,// 144 PAY 141 

    0x5a64080f,// 145 PAY 142 

    0x9794249d,// 146 PAY 143 

    0x7659b79e,// 147 PAY 144 

    0x8edfa118,// 148 PAY 145 

    0xe3a3039a,// 149 PAY 146 

    0x5e71a979,// 150 PAY 147 

    0x35e6b41d,// 151 PAY 148 

    0xb9db8b76,// 152 PAY 149 

    0x00220556,// 153 PAY 150 

    0x601a3a3e,// 154 PAY 151 

    0x406b2fcb,// 155 PAY 152 

    0x9ba5f5e5,// 156 PAY 153 

    0x0d0b97da,// 157 PAY 154 

    0xbc40269c,// 158 PAY 155 

    0xa6bc5c69,// 159 PAY 156 

    0x1f84b679,// 160 PAY 157 

    0xf675352e,// 161 PAY 158 

    0xa16c0b27,// 162 PAY 159 

    0x9d2b06bf,// 163 PAY 160 

    0x3a801c45,// 164 PAY 161 

    0xbf0f16b1,// 165 PAY 162 

    0x0e4de8bc,// 166 PAY 163 

    0x8f63df05,// 167 PAY 164 

    0xbf2ba1bd,// 168 PAY 165 

    0xda616aec,// 169 PAY 166 

    0x1cf31104,// 170 PAY 167 

    0xcb59afa8,// 171 PAY 168 

    0x48baafbe,// 172 PAY 169 

    0x4e1ca358,// 173 PAY 170 

    0x398a480f,// 174 PAY 171 

    0xdd6bd614,// 175 PAY 172 

    0xd1c818d7,// 176 PAY 173 

    0xa8765f23,// 177 PAY 174 

    0xaaf16b14,// 178 PAY 175 

    0x19240aa0,// 179 PAY 176 

    0xea66c591,// 180 PAY 177 

    0xc2f33cb0,// 181 PAY 178 

    0x1aae809b,// 182 PAY 179 

    0x39e47c36,// 183 PAY 180 

    0xcec22f86,// 184 PAY 181 

    0x64581cb3,// 185 PAY 182 

    0xea1a5844,// 186 PAY 183 

    0xb91cfb8e,// 187 PAY 184 

    0xa1005c4c,// 188 PAY 185 

    0x4e36621c,// 189 PAY 186 

    0x063a8cdd,// 190 PAY 187 

    0x9a488820,// 191 PAY 188 

    0x5c4dbd85,// 192 PAY 189 

    0xdedfc6f7,// 193 PAY 190 

    0xad661a33,// 194 PAY 191 

    0xe21ddd97,// 195 PAY 192 

    0xa041aa06,// 196 PAY 193 

    0x7ac8703f,// 197 PAY 194 

    0x4bda2cc5,// 198 PAY 195 

    0x15e98f13,// 199 PAY 196 

    0x96a715bc,// 200 PAY 197 

    0x3ac6e9dd,// 201 PAY 198 

    0x7ea8a8d2,// 202 PAY 199 

    0xd7c14b90,// 203 PAY 200 

    0x037ccff0,// 204 PAY 201 

    0x6688ba51,// 205 PAY 202 

    0x025a28f2,// 206 PAY 203 

    0x4c2cb6c1,// 207 PAY 204 

    0x461b90dd,// 208 PAY 205 

    0x949721a4,// 209 PAY 206 

    0x97a08994,// 210 PAY 207 

    0xf48e479e,// 211 PAY 208 

    0x21527ad6,// 212 PAY 209 

    0xe91437a4,// 213 PAY 210 

    0xf89f161f,// 214 PAY 211 

    0xff154feb,// 215 PAY 212 

    0xdd0c7a92,// 216 PAY 213 

    0x1b2af819,// 217 PAY 214 

    0x06f19ce8,// 218 PAY 215 

    0x318ee3e9,// 219 PAY 216 

    0x3aa7f6f4,// 220 PAY 217 

    0xfe48d50f,// 221 PAY 218 

    0x9b7b950f,// 222 PAY 219 

    0x6b9859f8,// 223 PAY 220 

    0x3167f33a,// 224 PAY 221 

    0x362e2ad7,// 225 PAY 222 

    0x20cabdd3,// 226 PAY 223 

    0xe8052a12,// 227 PAY 224 

    0x362c3327,// 228 PAY 225 

    0x97c1ca03,// 229 PAY 226 

    0x23a59869,// 230 PAY 227 

    0xa22c3794,// 231 PAY 228 

    0x9fce7470,// 232 PAY 229 

    0x12f8e275,// 233 PAY 230 

    0x24547ca1,// 234 PAY 231 

    0x5ab73dd8,// 235 PAY 232 

    0x0786db4e,// 236 PAY 233 

    0x701f0304,// 237 PAY 234 

    0x95ebf6e7,// 238 PAY 235 

    0x24531661,// 239 PAY 236 

    0x9c7f1382,// 240 PAY 237 

    0x54974523,// 241 PAY 238 

    0x868c3e98,// 242 PAY 239 

    0x96bea438,// 243 PAY 240 

    0xbe40792f,// 244 PAY 241 

    0xdde9a14f,// 245 PAY 242 

    0x1635f810,// 246 PAY 243 

    0x8df2c9a7,// 247 PAY 244 

    0xcc977c7e,// 248 PAY 245 

    0xf94b7ee1,// 249 PAY 246 

    0x54a60ca1,// 250 PAY 247 

    0xa1b51829,// 251 PAY 248 

    0x5151653e,// 252 PAY 249 

    0xe6b087a9,// 253 PAY 250 

    0x6946f6ee,// 254 PAY 251 

    0x4984ad57,// 255 PAY 252 

    0x0cb07cd3,// 256 PAY 253 

    0xd0f9dd2f,// 257 PAY 254 

    0xeb8a6421,// 258 PAY 255 

    0xef9e5467,// 259 PAY 256 

    0xc010ec8c,// 260 PAY 257 

    0x0ceee0a8,// 261 PAY 258 

    0xb0f22e8e,// 262 PAY 259 

    0x058da70b,// 263 PAY 260 

    0x342d974a,// 264 PAY 261 

    0x3dbee534,// 265 PAY 262 

    0xdc77a295,// 266 PAY 263 

    0x603532d4,// 267 PAY 264 

    0x11b10792,// 268 PAY 265 

    0xedff7e1d,// 269 PAY 266 

    0x2157167e,// 270 PAY 267 

    0xde4b82c1,// 271 PAY 268 

    0x2579e5f0,// 272 PAY 269 

    0xe9b7e567,// 273 PAY 270 

    0x98447156,// 274 PAY 271 

    0x1f0b96b0,// 275 PAY 272 

    0xcf60922c,// 276 PAY 273 

    0xc7343df9,// 277 PAY 274 

    0xd9729f25,// 278 PAY 275 

    0x23fec0dc,// 279 PAY 276 

    0x78dbb687,// 280 PAY 277 

    0xf5f0f495,// 281 PAY 278 

    0x194e1e14,// 282 PAY 279 

    0xd156e853,// 283 PAY 280 

    0x253f2e30,// 284 PAY 281 

    0xbae375a7,// 285 PAY 282 

    0xd6424856,// 286 PAY 283 

    0xbf404e82,// 287 PAY 284 

    0x8852c00c,// 288 PAY 285 

    0x2bb59f01,// 289 PAY 286 

    0x3d87d386,// 290 PAY 287 

    0xb89a331e,// 291 PAY 288 

    0x3f279d83,// 292 PAY 289 

    0xda5c4c3a,// 293 PAY 290 

    0x20595838,// 294 PAY 291 

    0x1c40f492,// 295 PAY 292 

    0x51989775,// 296 PAY 293 

    0x7d53a98f,// 297 PAY 294 

    0x19054e49,// 298 PAY 295 

    0x33812981,// 299 PAY 296 

    0x3f00df34,// 300 PAY 297 

    0x77ea94bc,// 301 PAY 298 

    0x7e15ce08,// 302 PAY 299 

    0x66c27f93,// 303 PAY 300 

    0x92c7b865,// 304 PAY 301 

    0xac530000,// 305 PAY 302 

/// STA is 1 words. 

/// STA num_pkts       : 235 

/// STA pkt_idx        : 178 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x02c81ceb // 306 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt77_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 230 words. 

/// BDA size     is 914 (0x392) 

/// BDA id       is 0xaa5 

    0x03920aa5,// 3 BDA   1 

/// PAY Generic Data size   : 914 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xc18f5c2a,// 4 PAY   1 

    0x00af5b06,// 5 PAY   2 

    0x4d91d0ba,// 6 PAY   3 

    0x474c0bd1,// 7 PAY   4 

    0x852ae63c,// 8 PAY   5 

    0xae2e3b9a,// 9 PAY   6 

    0x207571df,// 10 PAY   7 

    0xafa28488,// 11 PAY   8 

    0xbfd2fd3c,// 12 PAY   9 

    0xfda6d462,// 13 PAY  10 

    0xbb7f2da2,// 14 PAY  11 

    0xef825697,// 15 PAY  12 

    0x25e4ceb6,// 16 PAY  13 

    0x25ebb385,// 17 PAY  14 

    0x5efbefbd,// 18 PAY  15 

    0x57465be8,// 19 PAY  16 

    0xc62eccbb,// 20 PAY  17 

    0xd8f4740e,// 21 PAY  18 

    0xde70b080,// 22 PAY  19 

    0x37aa34bf,// 23 PAY  20 

    0xe763f7d8,// 24 PAY  21 

    0x2c60419c,// 25 PAY  22 

    0xb77aaf6b,// 26 PAY  23 

    0x07030f71,// 27 PAY  24 

    0xa9e0c1fc,// 28 PAY  25 

    0x16af5290,// 29 PAY  26 

    0xfad0e040,// 30 PAY  27 

    0x4d7117fc,// 31 PAY  28 

    0xad76ed0e,// 32 PAY  29 

    0x52ac3ca3,// 33 PAY  30 

    0x9cfa711a,// 34 PAY  31 

    0x06ba1d61,// 35 PAY  32 

    0x171d9438,// 36 PAY  33 

    0x1136e9e4,// 37 PAY  34 

    0xc8bf672a,// 38 PAY  35 

    0xc84ddbf0,// 39 PAY  36 

    0x50aa7dea,// 40 PAY  37 

    0xe7fab572,// 41 PAY  38 

    0x4488688c,// 42 PAY  39 

    0x566ade33,// 43 PAY  40 

    0x04b6b263,// 44 PAY  41 

    0xf8bbf3d7,// 45 PAY  42 

    0x294ebfed,// 46 PAY  43 

    0x49694676,// 47 PAY  44 

    0xe17145dd,// 48 PAY  45 

    0x795993cb,// 49 PAY  46 

    0x1093867c,// 50 PAY  47 

    0xd5471a81,// 51 PAY  48 

    0xf0705c8e,// 52 PAY  49 

    0x999e2a9e,// 53 PAY  50 

    0x94730033,// 54 PAY  51 

    0xfb580dd8,// 55 PAY  52 

    0x1fe3bf69,// 56 PAY  53 

    0x59841b14,// 57 PAY  54 

    0xc428db12,// 58 PAY  55 

    0xfca66c24,// 59 PAY  56 

    0x9250c07a,// 60 PAY  57 

    0x63746feb,// 61 PAY  58 

    0x90817ea6,// 62 PAY  59 

    0xfd5a283f,// 63 PAY  60 

    0x95411166,// 64 PAY  61 

    0x6c5738c8,// 65 PAY  62 

    0x9f41cc90,// 66 PAY  63 

    0x42b5993a,// 67 PAY  64 

    0x1e6551e9,// 68 PAY  65 

    0xcba0a303,// 69 PAY  66 

    0x1b6a29a4,// 70 PAY  67 

    0x24814d27,// 71 PAY  68 

    0xff2ddce5,// 72 PAY  69 

    0x6b9c5b6e,// 73 PAY  70 

    0x42ce5930,// 74 PAY  71 

    0x903048a5,// 75 PAY  72 

    0xd4bac7d6,// 76 PAY  73 

    0x70a0b83d,// 77 PAY  74 

    0x96dc2828,// 78 PAY  75 

    0x78d1733c,// 79 PAY  76 

    0x0f4af923,// 80 PAY  77 

    0xe892aa0e,// 81 PAY  78 

    0xf708a57c,// 82 PAY  79 

    0x7787b579,// 83 PAY  80 

    0x36e4218d,// 84 PAY  81 

    0xa0aa2b15,// 85 PAY  82 

    0x6a7fd9ef,// 86 PAY  83 

    0x9a97b9ac,// 87 PAY  84 

    0x02a9b47c,// 88 PAY  85 

    0xbbbbac6b,// 89 PAY  86 

    0x5fe8ff30,// 90 PAY  87 

    0xd560f6bf,// 91 PAY  88 

    0x8b66d100,// 92 PAY  89 

    0xe96a19c0,// 93 PAY  90 

    0x51078bb5,// 94 PAY  91 

    0xea2651b0,// 95 PAY  92 

    0x0bd2c585,// 96 PAY  93 

    0x57e4e379,// 97 PAY  94 

    0x239c1167,// 98 PAY  95 

    0x157d14d5,// 99 PAY  96 

    0x17b08f07,// 100 PAY  97 

    0x00fd8b2d,// 101 PAY  98 

    0x9e483079,// 102 PAY  99 

    0x7c373aa8,// 103 PAY 100 

    0x17f447c1,// 104 PAY 101 

    0x4a0ee021,// 105 PAY 102 

    0x6858a7dd,// 106 PAY 103 

    0x5e73874f,// 107 PAY 104 

    0xe438b112,// 108 PAY 105 

    0x9080980d,// 109 PAY 106 

    0xab0b1fac,// 110 PAY 107 

    0x69094b5e,// 111 PAY 108 

    0x5f124273,// 112 PAY 109 

    0x9e78029f,// 113 PAY 110 

    0x6d634055,// 114 PAY 111 

    0xa2b21bdf,// 115 PAY 112 

    0x9297a65f,// 116 PAY 113 

    0xc2dcbf2b,// 117 PAY 114 

    0xce4c57c6,// 118 PAY 115 

    0x8ab2796f,// 119 PAY 116 

    0x3f19c5ea,// 120 PAY 117 

    0x39d7f4d1,// 121 PAY 118 

    0x65195c72,// 122 PAY 119 

    0x1db263fc,// 123 PAY 120 

    0x2a3e856e,// 124 PAY 121 

    0x31e5f23b,// 125 PAY 122 

    0x95eb6adb,// 126 PAY 123 

    0x0942e7a1,// 127 PAY 124 

    0xc534d88c,// 128 PAY 125 

    0xe8c34a88,// 129 PAY 126 

    0x35cd6497,// 130 PAY 127 

    0x57a40935,// 131 PAY 128 

    0x9d39e3f3,// 132 PAY 129 

    0xfa1fb6db,// 133 PAY 130 

    0x107dc17e,// 134 PAY 131 

    0xd348cb50,// 135 PAY 132 

    0x442e25e8,// 136 PAY 133 

    0xd736e9ac,// 137 PAY 134 

    0xd5020fdf,// 138 PAY 135 

    0x8cfa6e8f,// 139 PAY 136 

    0x30da5818,// 140 PAY 137 

    0x726309de,// 141 PAY 138 

    0xbf14593f,// 142 PAY 139 

    0x2defe2d0,// 143 PAY 140 

    0x2e399f9d,// 144 PAY 141 

    0x6d7e6650,// 145 PAY 142 

    0x491e2635,// 146 PAY 143 

    0xb79dab7e,// 147 PAY 144 

    0x90eb8e11,// 148 PAY 145 

    0x9e1d60c0,// 149 PAY 146 

    0x70c5454d,// 150 PAY 147 

    0x54f23690,// 151 PAY 148 

    0x00fe4b96,// 152 PAY 149 

    0xfddaf112,// 153 PAY 150 

    0x5a5130dc,// 154 PAY 151 

    0xf55bb9a2,// 155 PAY 152 

    0x07c4c8c9,// 156 PAY 153 

    0xe3d8fcf5,// 157 PAY 154 

    0xf07e5cbd,// 158 PAY 155 

    0xc94b4e5e,// 159 PAY 156 

    0x51f482b7,// 160 PAY 157 

    0x2e8431ec,// 161 PAY 158 

    0x8c8fa098,// 162 PAY 159 

    0xbe60155c,// 163 PAY 160 

    0xde0b7bc9,// 164 PAY 161 

    0xd511dfff,// 165 PAY 162 

    0xd7c3afa3,// 166 PAY 163 

    0xcf610365,// 167 PAY 164 

    0x27808b29,// 168 PAY 165 

    0x6bf32684,// 169 PAY 166 

    0x4df690f7,// 170 PAY 167 

    0x3ce938ad,// 171 PAY 168 

    0x34e7e8fe,// 172 PAY 169 

    0x61f4896f,// 173 PAY 170 

    0x79cb9fc9,// 174 PAY 171 

    0xe4a43220,// 175 PAY 172 

    0x5cad8f01,// 176 PAY 173 

    0x0d4204c4,// 177 PAY 174 

    0xa0e740e3,// 178 PAY 175 

    0x5df32d68,// 179 PAY 176 

    0x46a2b342,// 180 PAY 177 

    0x373a5ec7,// 181 PAY 178 

    0xf7beabea,// 182 PAY 179 

    0xddce95c5,// 183 PAY 180 

    0xcc85f1ab,// 184 PAY 181 

    0x8ef0c9bc,// 185 PAY 182 

    0x788b00ca,// 186 PAY 183 

    0x02b419e6,// 187 PAY 184 

    0x31f68080,// 188 PAY 185 

    0x39ed5859,// 189 PAY 186 

    0x7b2db9e6,// 190 PAY 187 

    0x3dece79b,// 191 PAY 188 

    0xa792193a,// 192 PAY 189 

    0x98e40185,// 193 PAY 190 

    0xe734ffb1,// 194 PAY 191 

    0xb5151117,// 195 PAY 192 

    0xe0d84119,// 196 PAY 193 

    0xeefee000,// 197 PAY 194 

    0xec93acdd,// 198 PAY 195 

    0xf1f16cdb,// 199 PAY 196 

    0x5a683189,// 200 PAY 197 

    0x54e427cb,// 201 PAY 198 

    0x669d7a51,// 202 PAY 199 

    0xaa2b9705,// 203 PAY 200 

    0xd3df2694,// 204 PAY 201 

    0xcd72a0fc,// 205 PAY 202 

    0xc600ccd9,// 206 PAY 203 

    0x4411dff2,// 207 PAY 204 

    0xe8e0ebc8,// 208 PAY 205 

    0x7e88e415,// 209 PAY 206 

    0xfeacf916,// 210 PAY 207 

    0x4741a800,// 211 PAY 208 

    0x90f38208,// 212 PAY 209 

    0x91771327,// 213 PAY 210 

    0x3eddb84f,// 214 PAY 211 

    0x0e1c7a93,// 215 PAY 212 

    0x344c9291,// 216 PAY 213 

    0x9e9b6a4f,// 217 PAY 214 

    0xe7620d50,// 218 PAY 215 

    0x1787f7f0,// 219 PAY 216 

    0x0a736e27,// 220 PAY 217 

    0x0bc8e65f,// 221 PAY 218 

    0x869e4f2a,// 222 PAY 219 

    0xb92333c9,// 223 PAY 220 

    0x322aa4d9,// 224 PAY 221 

    0x69698dad,// 225 PAY 222 

    0x229be6ba,// 226 PAY 223 

    0xaf19c359,// 227 PAY 224 

    0x0822641e,// 228 PAY 225 

    0xcc0289f9,// 229 PAY 226 

    0xcc6031ef,// 230 PAY 227 

    0x92aeebc2,// 231 PAY 228 

    0x80770000,// 232 PAY 229 

/// STA is 1 words. 

/// STA num_pkts       : 254 

/// STA pkt_idx        : 71 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x011dbefe // 233 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt78_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 468 words. 

/// BDA size     is 1867 (0x74b) 

/// BDA id       is 0xdef3 

    0x074bdef3,// 3 BDA   1 

/// PAY Generic Data size   : 1867 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xc36d0202,// 4 PAY   1 

    0xaf50a402,// 5 PAY   2 

    0x379e75ec,// 6 PAY   3 

    0x882d29ed,// 7 PAY   4 

    0xfc20d619,// 8 PAY   5 

    0x3a46b6e1,// 9 PAY   6 

    0x52f48fb6,// 10 PAY   7 

    0x14b718da,// 11 PAY   8 

    0x96321fff,// 12 PAY   9 

    0x2c06312d,// 13 PAY  10 

    0xca284945,// 14 PAY  11 

    0x804a72e9,// 15 PAY  12 

    0x39a65ab2,// 16 PAY  13 

    0x25ca096c,// 17 PAY  14 

    0x90649540,// 18 PAY  15 

    0x55e408bd,// 19 PAY  16 

    0xe3b7a4b3,// 20 PAY  17 

    0x3d552aaa,// 21 PAY  18 

    0x56404f62,// 22 PAY  19 

    0xc902b875,// 23 PAY  20 

    0x841cc291,// 24 PAY  21 

    0xf118d9cd,// 25 PAY  22 

    0x6ac39bdc,// 26 PAY  23 

    0x6bfaa1a7,// 27 PAY  24 

    0xe3a5f167,// 28 PAY  25 

    0xda098f67,// 29 PAY  26 

    0x5e03fc13,// 30 PAY  27 

    0x2567c89b,// 31 PAY  28 

    0xb4c358d6,// 32 PAY  29 

    0x256d53f4,// 33 PAY  30 

    0xc279fbe6,// 34 PAY  31 

    0xf8814ce0,// 35 PAY  32 

    0x51f52eb3,// 36 PAY  33 

    0xa2d05c20,// 37 PAY  34 

    0xb625f3dd,// 38 PAY  35 

    0x497ed5fe,// 39 PAY  36 

    0x7de7bdba,// 40 PAY  37 

    0xab08a209,// 41 PAY  38 

    0xd8d62b09,// 42 PAY  39 

    0xcdbacb1c,// 43 PAY  40 

    0x9480147e,// 44 PAY  41 

    0xabce81bd,// 45 PAY  42 

    0xd346676d,// 46 PAY  43 

    0x12883534,// 47 PAY  44 

    0x933532ec,// 48 PAY  45 

    0x2f07df39,// 49 PAY  46 

    0x7db6f826,// 50 PAY  47 

    0x214287e8,// 51 PAY  48 

    0x4ca95985,// 52 PAY  49 

    0x655109af,// 53 PAY  50 

    0x926b400e,// 54 PAY  51 

    0x45acc4b7,// 55 PAY  52 

    0xfe7dfed3,// 56 PAY  53 

    0x60bda922,// 57 PAY  54 

    0x5efab7e4,// 58 PAY  55 

    0x32998e3d,// 59 PAY  56 

    0x3439883c,// 60 PAY  57 

    0x6f0c6ca6,// 61 PAY  58 

    0x560a3cc9,// 62 PAY  59 

    0x09e3bc8f,// 63 PAY  60 

    0xe09e90a8,// 64 PAY  61 

    0xbf78699d,// 65 PAY  62 

    0xcc085612,// 66 PAY  63 

    0xcd75c6b1,// 67 PAY  64 

    0x230124bf,// 68 PAY  65 

    0x350e94f3,// 69 PAY  66 

    0x3c06ff0e,// 70 PAY  67 

    0x992bbe68,// 71 PAY  68 

    0x026cfabe,// 72 PAY  69 

    0xb00c00d9,// 73 PAY  70 

    0xd5b44625,// 74 PAY  71 

    0x1495aee2,// 75 PAY  72 

    0x1ebe6595,// 76 PAY  73 

    0x3f1c051e,// 77 PAY  74 

    0x8b801897,// 78 PAY  75 

    0xfea26028,// 79 PAY  76 

    0x99d8ab51,// 80 PAY  77 

    0xb54a3f71,// 81 PAY  78 

    0x56a0d332,// 82 PAY  79 

    0x01ecaca6,// 83 PAY  80 

    0xfba62d7c,// 84 PAY  81 

    0xc4da2c11,// 85 PAY  82 

    0xb339373a,// 86 PAY  83 

    0x9637ef94,// 87 PAY  84 

    0x6e21db34,// 88 PAY  85 

    0x08c3c3f1,// 89 PAY  86 

    0x0881d705,// 90 PAY  87 

    0x315d62bc,// 91 PAY  88 

    0x13598c90,// 92 PAY  89 

    0x8b54ca15,// 93 PAY  90 

    0xd65ef1de,// 94 PAY  91 

    0x5abfad2d,// 95 PAY  92 

    0x2fb85e81,// 96 PAY  93 

    0xc7b0e368,// 97 PAY  94 

    0x74a7cca4,// 98 PAY  95 

    0x9354869c,// 99 PAY  96 

    0x1232b3cc,// 100 PAY  97 

    0x1d6b7d68,// 101 PAY  98 

    0xb9e53086,// 102 PAY  99 

    0x1a881009,// 103 PAY 100 

    0xd13a71a5,// 104 PAY 101 

    0x8b299e86,// 105 PAY 102 

    0x333f53d2,// 106 PAY 103 

    0xfd224ee6,// 107 PAY 104 

    0x1054a4d0,// 108 PAY 105 

    0x5ebe1444,// 109 PAY 106 

    0x9e84756a,// 110 PAY 107 

    0x47d23ee5,// 111 PAY 108 

    0x970ad201,// 112 PAY 109 

    0xb36c4f11,// 113 PAY 110 

    0xbdefb119,// 114 PAY 111 

    0x1293beda,// 115 PAY 112 

    0x3842030f,// 116 PAY 113 

    0x1b0b11bd,// 117 PAY 114 

    0x76503752,// 118 PAY 115 

    0x03f5504f,// 119 PAY 116 

    0xa5eaee5c,// 120 PAY 117 

    0x66493bb7,// 121 PAY 118 

    0xff3a15e5,// 122 PAY 119 

    0x54d93522,// 123 PAY 120 

    0xaefd6e13,// 124 PAY 121 

    0xb2c7114b,// 125 PAY 122 

    0xe5ab40ac,// 126 PAY 123 

    0xe4fca1a2,// 127 PAY 124 

    0x723b88b3,// 128 PAY 125 

    0xda3a8ea3,// 129 PAY 126 

    0x9e525f5d,// 130 PAY 127 

    0x829026b8,// 131 PAY 128 

    0x969c1faf,// 132 PAY 129 

    0x59f42ddf,// 133 PAY 130 

    0x1acc5a92,// 134 PAY 131 

    0xd8019c38,// 135 PAY 132 

    0x897b749a,// 136 PAY 133 

    0x25214dbb,// 137 PAY 134 

    0x4fc6b04b,// 138 PAY 135 

    0x5bfdd9d7,// 139 PAY 136 

    0xd590ea2e,// 140 PAY 137 

    0x48bb6885,// 141 PAY 138 

    0xc1086529,// 142 PAY 139 

    0x4ced6c8b,// 143 PAY 140 

    0xe1b73007,// 144 PAY 141 

    0xac7eacc5,// 145 PAY 142 

    0x3174c44e,// 146 PAY 143 

    0x267279aa,// 147 PAY 144 

    0x71015e2b,// 148 PAY 145 

    0x9a611578,// 149 PAY 146 

    0x73dbb5f9,// 150 PAY 147 

    0xb1e0ebd9,// 151 PAY 148 

    0xdfa974fd,// 152 PAY 149 

    0x99370a1b,// 153 PAY 150 

    0x2c33fc93,// 154 PAY 151 

    0x20844c07,// 155 PAY 152 

    0x5bdfc355,// 156 PAY 153 

    0x8a20cda7,// 157 PAY 154 

    0x00cdf8a0,// 158 PAY 155 

    0xf3ecf1a6,// 159 PAY 156 

    0xb214722f,// 160 PAY 157 

    0xbc6cdbf3,// 161 PAY 158 

    0x7a0009c2,// 162 PAY 159 

    0x434ec714,// 163 PAY 160 

    0xf3ccfa0f,// 164 PAY 161 

    0xb82dc7b7,// 165 PAY 162 

    0xd6193303,// 166 PAY 163 

    0x743492fc,// 167 PAY 164 

    0x84671a34,// 168 PAY 165 

    0x2875e85a,// 169 PAY 166 

    0xafe7398c,// 170 PAY 167 

    0x50e6d302,// 171 PAY 168 

    0x44237b01,// 172 PAY 169 

    0xb4eb96b9,// 173 PAY 170 

    0xa6d01978,// 174 PAY 171 

    0x2474c3b3,// 175 PAY 172 

    0x2ffe0e38,// 176 PAY 173 

    0x83db3f38,// 177 PAY 174 

    0x026de4b8,// 178 PAY 175 

    0x703f750f,// 179 PAY 176 

    0xe484b550,// 180 PAY 177 

    0xfd68305b,// 181 PAY 178 

    0x77780d5f,// 182 PAY 179 

    0xfaf131f9,// 183 PAY 180 

    0x5267cd83,// 184 PAY 181 

    0x7f058dd4,// 185 PAY 182 

    0x94602f60,// 186 PAY 183 

    0x88b9168c,// 187 PAY 184 

    0x07029e7c,// 188 PAY 185 

    0xe4d3401c,// 189 PAY 186 

    0xfc873d37,// 190 PAY 187 

    0x1a6a06c6,// 191 PAY 188 

    0xfc32fce9,// 192 PAY 189 

    0xedb36b39,// 193 PAY 190 

    0x3a89681e,// 194 PAY 191 

    0x14986af2,// 195 PAY 192 

    0xf8ad7f4a,// 196 PAY 193 

    0x18111abe,// 197 PAY 194 

    0xb7d75ec8,// 198 PAY 195 

    0xf5715a1d,// 199 PAY 196 

    0x13d7dce1,// 200 PAY 197 

    0xc05cf545,// 201 PAY 198 

    0x212e4832,// 202 PAY 199 

    0x7e247031,// 203 PAY 200 

    0xfc788128,// 204 PAY 201 

    0x04f9b128,// 205 PAY 202 

    0xb2098eca,// 206 PAY 203 

    0x133e8e39,// 207 PAY 204 

    0x13980642,// 208 PAY 205 

    0xae4a2b03,// 209 PAY 206 

    0xacfda3c8,// 210 PAY 207 

    0x8df92c07,// 211 PAY 208 

    0x46ef107a,// 212 PAY 209 

    0xff09cf2a,// 213 PAY 210 

    0x31b3d890,// 214 PAY 211 

    0x1bff3ae0,// 215 PAY 212 

    0x98f81b5d,// 216 PAY 213 

    0x5fe9ad1e,// 217 PAY 214 

    0x6b76f8f5,// 218 PAY 215 

    0x241ef953,// 219 PAY 216 

    0x18d2efcd,// 220 PAY 217 

    0x6568cdac,// 221 PAY 218 

    0x554cffce,// 222 PAY 219 

    0x896e9da1,// 223 PAY 220 

    0xa8af9529,// 224 PAY 221 

    0x482bbb00,// 225 PAY 222 

    0x810e569c,// 226 PAY 223 

    0x27929e59,// 227 PAY 224 

    0xd3a16a9f,// 228 PAY 225 

    0x26bffebf,// 229 PAY 226 

    0xac204502,// 230 PAY 227 

    0x417dbaf8,// 231 PAY 228 

    0xde61d800,// 232 PAY 229 

    0x92125885,// 233 PAY 230 

    0xbca8c964,// 234 PAY 231 

    0x7a60b449,// 235 PAY 232 

    0x3643243d,// 236 PAY 233 

    0x5eccdebc,// 237 PAY 234 

    0x3d766f24,// 238 PAY 235 

    0xe23063bd,// 239 PAY 236 

    0x89330b94,// 240 PAY 237 

    0x6ec683ef,// 241 PAY 238 

    0x235eaede,// 242 PAY 239 

    0x92cc7510,// 243 PAY 240 

    0xd96c65ff,// 244 PAY 241 

    0x2e16d36f,// 245 PAY 242 

    0xdfd5cdc3,// 246 PAY 243 

    0xb1c5da2f,// 247 PAY 244 

    0x3bd65595,// 248 PAY 245 

    0xaacf4a2f,// 249 PAY 246 

    0xed414d37,// 250 PAY 247 

    0x68a5a55e,// 251 PAY 248 

    0xaacb4ec9,// 252 PAY 249 

    0xf68a19a2,// 253 PAY 250 

    0x368ec880,// 254 PAY 251 

    0x1f430927,// 255 PAY 252 

    0xde40c4ab,// 256 PAY 253 

    0x8bd860df,// 257 PAY 254 

    0xe6a80a9b,// 258 PAY 255 

    0xa76d3715,// 259 PAY 256 

    0x475da44e,// 260 PAY 257 

    0xb991ea21,// 261 PAY 258 

    0xeb45289a,// 262 PAY 259 

    0xb71bd0c1,// 263 PAY 260 

    0x41ce5298,// 264 PAY 261 

    0x2cb33dd1,// 265 PAY 262 

    0xf213dfde,// 266 PAY 263 

    0x137e42a3,// 267 PAY 264 

    0xa4723141,// 268 PAY 265 

    0x04653491,// 269 PAY 266 

    0xfbe01c74,// 270 PAY 267 

    0x91d24ffc,// 271 PAY 268 

    0xac545033,// 272 PAY 269 

    0xe5e08d51,// 273 PAY 270 

    0x941915b1,// 274 PAY 271 

    0xc37dc514,// 275 PAY 272 

    0x9b1d8da9,// 276 PAY 273 

    0x0d64312d,// 277 PAY 274 

    0x59911a09,// 278 PAY 275 

    0xdf1c8020,// 279 PAY 276 

    0xb64133e0,// 280 PAY 277 

    0xd04a9a3b,// 281 PAY 278 

    0x48c33dca,// 282 PAY 279 

    0x9bc45ed9,// 283 PAY 280 

    0xc4178d9b,// 284 PAY 281 

    0x3e3aaa22,// 285 PAY 282 

    0x6e029919,// 286 PAY 283 

    0x62416fed,// 287 PAY 284 

    0x8bc0c722,// 288 PAY 285 

    0x3205bc2b,// 289 PAY 286 

    0x1411aa0e,// 290 PAY 287 

    0x84890dba,// 291 PAY 288 

    0x8ee2c07c,// 292 PAY 289 

    0xd2a4e6f5,// 293 PAY 290 

    0x9b479f0c,// 294 PAY 291 

    0xa4125868,// 295 PAY 292 

    0x5b4e4656,// 296 PAY 293 

    0x6cd4342c,// 297 PAY 294 

    0x09f617f0,// 298 PAY 295 

    0x3722bcfc,// 299 PAY 296 

    0x824f6bf2,// 300 PAY 297 

    0x8c54e5c0,// 301 PAY 298 

    0x17bc86ac,// 302 PAY 299 

    0x8c6db3e3,// 303 PAY 300 

    0x4b1f8b8b,// 304 PAY 301 

    0x0bb9944e,// 305 PAY 302 

    0x085c1402,// 306 PAY 303 

    0x5487d7dc,// 307 PAY 304 

    0x6d82734e,// 308 PAY 305 

    0x90cda1b2,// 309 PAY 306 

    0xd00a3bef,// 310 PAY 307 

    0x63d9efa2,// 311 PAY 308 

    0xa6b7e711,// 312 PAY 309 

    0x6dfab344,// 313 PAY 310 

    0x2ba5ce6f,// 314 PAY 311 

    0xc95f3fdf,// 315 PAY 312 

    0xcda654ec,// 316 PAY 313 

    0x39e0f41f,// 317 PAY 314 

    0x3a044e3c,// 318 PAY 315 

    0x295cc25e,// 319 PAY 316 

    0x1a6f0962,// 320 PAY 317 

    0xab9cab75,// 321 PAY 318 

    0x2ebb7097,// 322 PAY 319 

    0x3dcfd74d,// 323 PAY 320 

    0xd6bb4493,// 324 PAY 321 

    0x2023f1d9,// 325 PAY 322 

    0x05a13803,// 326 PAY 323 

    0xf51ea95a,// 327 PAY 324 

    0x2ba36fed,// 328 PAY 325 

    0x935d542d,// 329 PAY 326 

    0x8883da02,// 330 PAY 327 

    0x6a27fd08,// 331 PAY 328 

    0x3ca56a32,// 332 PAY 329 

    0xaa5c32bb,// 333 PAY 330 

    0x9c173f3a,// 334 PAY 331 

    0x08d19e2d,// 335 PAY 332 

    0x4ad8b0d1,// 336 PAY 333 

    0x23a5eedc,// 337 PAY 334 

    0xee9e7409,// 338 PAY 335 

    0x23366316,// 339 PAY 336 

    0x32c9dddb,// 340 PAY 337 

    0xf3677bc0,// 341 PAY 338 

    0x30e13f29,// 342 PAY 339 

    0x9402a188,// 343 PAY 340 

    0xeee51612,// 344 PAY 341 

    0xafb7632e,// 345 PAY 342 

    0x5e2a4d31,// 346 PAY 343 

    0x5e33da30,// 347 PAY 344 

    0x6697c81f,// 348 PAY 345 

    0x0d7e1555,// 349 PAY 346 

    0x95e7880a,// 350 PAY 347 

    0xb7d6d931,// 351 PAY 348 

    0x2d170932,// 352 PAY 349 

    0x993af291,// 353 PAY 350 

    0x7a946bfe,// 354 PAY 351 

    0xa16f84a6,// 355 PAY 352 

    0xa76973fb,// 356 PAY 353 

    0x7aa3df6e,// 357 PAY 354 

    0xe810b230,// 358 PAY 355 

    0xbb06b735,// 359 PAY 356 

    0xa63287b6,// 360 PAY 357 

    0x2d07cf70,// 361 PAY 358 

    0xa856882d,// 362 PAY 359 

    0x878ffba0,// 363 PAY 360 

    0x9a9126ea,// 364 PAY 361 

    0x3f10ef6e,// 365 PAY 362 

    0x75131e15,// 366 PAY 363 

    0xf97e8457,// 367 PAY 364 

    0x47e34b0f,// 368 PAY 365 

    0x136fc20e,// 369 PAY 366 

    0xe2f535a7,// 370 PAY 367 

    0x9c0f8d88,// 371 PAY 368 

    0x22274a2f,// 372 PAY 369 

    0x0890e860,// 373 PAY 370 

    0xa79dd23b,// 374 PAY 371 

    0xcb7e2ecf,// 375 PAY 372 

    0xe28aef6e,// 376 PAY 373 

    0xa9fd3e00,// 377 PAY 374 

    0xf32d64c4,// 378 PAY 375 

    0xaab19862,// 379 PAY 376 

    0x181e4a87,// 380 PAY 377 

    0xd2153315,// 381 PAY 378 

    0x1bd1eab7,// 382 PAY 379 

    0xa3999928,// 383 PAY 380 

    0x858b173a,// 384 PAY 381 

    0x1f90e031,// 385 PAY 382 

    0x344b21ac,// 386 PAY 383 

    0x2f4fd924,// 387 PAY 384 

    0x9200aee9,// 388 PAY 385 

    0x59bc5b57,// 389 PAY 386 

    0xde65021a,// 390 PAY 387 

    0x58612b9c,// 391 PAY 388 

    0xe1d9526e,// 392 PAY 389 

    0x2f4934ea,// 393 PAY 390 

    0xf4f784d3,// 394 PAY 391 

    0x33315e22,// 395 PAY 392 

    0x85c2ec6e,// 396 PAY 393 

    0x28555107,// 397 PAY 394 

    0x0a71e3a8,// 398 PAY 395 

    0x27f2e494,// 399 PAY 396 

    0xf5c5960f,// 400 PAY 397 

    0xcc2d95f0,// 401 PAY 398 

    0xa51fe2fd,// 402 PAY 399 

    0xd87304bd,// 403 PAY 400 

    0xb5c46c87,// 404 PAY 401 

    0xf6c2be0a,// 405 PAY 402 

    0xcc324ed1,// 406 PAY 403 

    0x26869d9b,// 407 PAY 404 

    0x29a924dc,// 408 PAY 405 

    0x56f79c92,// 409 PAY 406 

    0x71c2161a,// 410 PAY 407 

    0xdddb6875,// 411 PAY 408 

    0xeb60adb5,// 412 PAY 409 

    0x3449c59c,// 413 PAY 410 

    0x0467fa44,// 414 PAY 411 

    0xa79b1550,// 415 PAY 412 

    0x7b8ee55f,// 416 PAY 413 

    0xda2535f8,// 417 PAY 414 

    0x6520aac7,// 418 PAY 415 

    0xcb596df1,// 419 PAY 416 

    0x6d4c22e5,// 420 PAY 417 

    0xd80ce3fa,// 421 PAY 418 

    0x30b09b60,// 422 PAY 419 

    0xcacfb2b9,// 423 PAY 420 

    0xed5b0c59,// 424 PAY 421 

    0x45afec8c,// 425 PAY 422 

    0x227fa658,// 426 PAY 423 

    0x1243d9c9,// 427 PAY 424 

    0x391a2a9f,// 428 PAY 425 

    0x16d9ba47,// 429 PAY 426 

    0x7727e89b,// 430 PAY 427 

    0x456e94c5,// 431 PAY 428 

    0x8fcb5008,// 432 PAY 429 

    0x81a7151b,// 433 PAY 430 

    0xa1a18065,// 434 PAY 431 

    0x3fbd32f4,// 435 PAY 432 

    0xa14aff75,// 436 PAY 433 

    0x164d7e8f,// 437 PAY 434 

    0x1bb1b336,// 438 PAY 435 

    0xdf1a14e0,// 439 PAY 436 

    0x004036ba,// 440 PAY 437 

    0x763e9e09,// 441 PAY 438 

    0x9e3825dc,// 442 PAY 439 

    0xbe96f7e7,// 443 PAY 440 

    0x6db32d12,// 444 PAY 441 

    0xb3724b12,// 445 PAY 442 

    0x9c6fc1fe,// 446 PAY 443 

    0x39f2f85f,// 447 PAY 444 

    0x516aaaf1,// 448 PAY 445 

    0x3bd84c7f,// 449 PAY 446 

    0x35627b58,// 450 PAY 447 

    0x66da6db5,// 451 PAY 448 

    0xfd6fdaa9,// 452 PAY 449 

    0x566d09fc,// 453 PAY 450 

    0xd691de97,// 454 PAY 451 

    0x3c62c31f,// 455 PAY 452 

    0x61bfa746,// 456 PAY 453 

    0xc05d5177,// 457 PAY 454 

    0x78077788,// 458 PAY 455 

    0x2e0e05a6,// 459 PAY 456 

    0xe7465b26,// 460 PAY 457 

    0xa2ac4240,// 461 PAY 458 

    0x191eec52,// 462 PAY 459 

    0x5a0f491e,// 463 PAY 460 

    0x70806509,// 464 PAY 461 

    0xe4b85ba1,// 465 PAY 462 

    0xa35abf9b,// 466 PAY 463 

    0xf2f55ec7,// 467 PAY 464 

    0x2df3f1e5,// 468 PAY 465 

    0x1d8a9f46,// 469 PAY 466 

    0xec4f7d00,// 470 PAY 467 

/// HASH is  4 bytes 

    0x941915b1,// 471 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 168 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf4 

    0x02a0f437 // 472 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt79_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 183 words. 

/// BDA size     is 725 (0x2d5) 

/// BDA id       is 0x4d3 

    0x02d504d3,// 3 BDA   1 

/// PAY Generic Data size   : 725 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xf8852ac5,// 4 PAY   1 

    0x0a9cb98f,// 5 PAY   2 

    0xcb1b6bae,// 6 PAY   3 

    0x1c3c6759,// 7 PAY   4 

    0x2b0bede6,// 8 PAY   5 

    0xcc57fea4,// 9 PAY   6 

    0x8a3c1008,// 10 PAY   7 

    0x61bc2290,// 11 PAY   8 

    0xc50fc322,// 12 PAY   9 

    0x16dbe889,// 13 PAY  10 

    0x9df4feeb,// 14 PAY  11 

    0x13146578,// 15 PAY  12 

    0x388ba55a,// 16 PAY  13 

    0x31e5dce8,// 17 PAY  14 

    0x2e583c85,// 18 PAY  15 

    0xc9b14c48,// 19 PAY  16 

    0x1eabfe90,// 20 PAY  17 

    0x8bbaf334,// 21 PAY  18 

    0x407089ce,// 22 PAY  19 

    0xb240381c,// 23 PAY  20 

    0xa462509a,// 24 PAY  21 

    0xf437c5c6,// 25 PAY  22 

    0x26908ef2,// 26 PAY  23 

    0x3a2e40c5,// 27 PAY  24 

    0x0ca87800,// 28 PAY  25 

    0x15d93093,// 29 PAY  26 

    0x691f2ab7,// 30 PAY  27 

    0x13ed2fd4,// 31 PAY  28 

    0x305f50a0,// 32 PAY  29 

    0x89ccc7b6,// 33 PAY  30 

    0x4e0d785b,// 34 PAY  31 

    0x880df010,// 35 PAY  32 

    0x69e16a80,// 36 PAY  33 

    0x9bbc858f,// 37 PAY  34 

    0x1f4cd2fc,// 38 PAY  35 

    0x498bea34,// 39 PAY  36 

    0x64d254ba,// 40 PAY  37 

    0xb70fecc9,// 41 PAY  38 

    0x6dcb28cc,// 42 PAY  39 

    0x2642a8b2,// 43 PAY  40 

    0x06d6efc6,// 44 PAY  41 

    0x6069eff2,// 45 PAY  42 

    0xd3575b2f,// 46 PAY  43 

    0x6478bd8f,// 47 PAY  44 

    0x640926ff,// 48 PAY  45 

    0x4bd0de10,// 49 PAY  46 

    0x0ff5300a,// 50 PAY  47 

    0x84876017,// 51 PAY  48 

    0x44126d12,// 52 PAY  49 

    0xe6d2851c,// 53 PAY  50 

    0xc427bc98,// 54 PAY  51 

    0x1716ea80,// 55 PAY  52 

    0x1746be75,// 56 PAY  53 

    0x271ca9c4,// 57 PAY  54 

    0xa76a7a0c,// 58 PAY  55 

    0x0ddbb3c7,// 59 PAY  56 

    0x181eb68c,// 60 PAY  57 

    0x0b3f09a9,// 61 PAY  58 

    0xfecbaf63,// 62 PAY  59 

    0xed760ee7,// 63 PAY  60 

    0xe80f305a,// 64 PAY  61 

    0xfa9013fd,// 65 PAY  62 

    0x8d4732d5,// 66 PAY  63 

    0xcccdfe0e,// 67 PAY  64 

    0xf4b4cf94,// 68 PAY  65 

    0x45b41eb5,// 69 PAY  66 

    0x6ec22e4a,// 70 PAY  67 

    0xafb15c94,// 71 PAY  68 

    0x541d96f9,// 72 PAY  69 

    0x56faaac6,// 73 PAY  70 

    0x5086bb06,// 74 PAY  71 

    0x6f268a5c,// 75 PAY  72 

    0x1c05bb10,// 76 PAY  73 

    0x29ae52a2,// 77 PAY  74 

    0x3cf52bde,// 78 PAY  75 

    0xfa23ef33,// 79 PAY  76 

    0xfd79d597,// 80 PAY  77 

    0x9137b6fd,// 81 PAY  78 

    0xf95b8fd8,// 82 PAY  79 

    0x8c1fbf0d,// 83 PAY  80 

    0x374eca03,// 84 PAY  81 

    0xa947a92f,// 85 PAY  82 

    0x98f33f08,// 86 PAY  83 

    0x8d7803ad,// 87 PAY  84 

    0x711ca66c,// 88 PAY  85 

    0xc23de4ad,// 89 PAY  86 

    0x17d924dc,// 90 PAY  87 

    0x9ce3d29f,// 91 PAY  88 

    0x33b46195,// 92 PAY  89 

    0x0b629d67,// 93 PAY  90 

    0x42b81b9f,// 94 PAY  91 

    0xa1cc8730,// 95 PAY  92 

    0xc3966d05,// 96 PAY  93 

    0xad6e8510,// 97 PAY  94 

    0x0d8063c5,// 98 PAY  95 

    0x4be0c034,// 99 PAY  96 

    0x0fdb991d,// 100 PAY  97 

    0x9a64780a,// 101 PAY  98 

    0xcfd6c24f,// 102 PAY  99 

    0x033c5bcf,// 103 PAY 100 

    0x08bcc024,// 104 PAY 101 

    0x443a2f05,// 105 PAY 102 

    0xc19208b9,// 106 PAY 103 

    0x99213359,// 107 PAY 104 

    0xbd078312,// 108 PAY 105 

    0x41944a21,// 109 PAY 106 

    0xc7fae7b8,// 110 PAY 107 

    0x447658ac,// 111 PAY 108 

    0x643e1bfa,// 112 PAY 109 

    0x324e4124,// 113 PAY 110 

    0xc99f7974,// 114 PAY 111 

    0x7ae8cc50,// 115 PAY 112 

    0x0337e98d,// 116 PAY 113 

    0x61ba634c,// 117 PAY 114 

    0xda544797,// 118 PAY 115 

    0xbc1c97ff,// 119 PAY 116 

    0x7c5fd6a3,// 120 PAY 117 

    0x8b1e548b,// 121 PAY 118 

    0x5b1420b7,// 122 PAY 119 

    0x5f1730ba,// 123 PAY 120 

    0x6400ddd4,// 124 PAY 121 

    0x3c1e387e,// 125 PAY 122 

    0x7571abb1,// 126 PAY 123 

    0x71c2950b,// 127 PAY 124 

    0xb682e986,// 128 PAY 125 

    0xc476b87f,// 129 PAY 126 

    0x8eea0018,// 130 PAY 127 

    0x406740c7,// 131 PAY 128 

    0x4f9548bd,// 132 PAY 129 

    0xcf13660f,// 133 PAY 130 

    0x8a5fdb28,// 134 PAY 131 

    0x890ab3a4,// 135 PAY 132 

    0xec216813,// 136 PAY 133 

    0x792db5b7,// 137 PAY 134 

    0xf189dff1,// 138 PAY 135 

    0x28006226,// 139 PAY 136 

    0x42c2872f,// 140 PAY 137 

    0x284481af,// 141 PAY 138 

    0xfe590fba,// 142 PAY 139 

    0x94a13399,// 143 PAY 140 

    0x0d4397e8,// 144 PAY 141 

    0x913e5173,// 145 PAY 142 

    0x4ad3fb8a,// 146 PAY 143 

    0xb4eb83bd,// 147 PAY 144 

    0xc52ad38b,// 148 PAY 145 

    0x84834d59,// 149 PAY 146 

    0x1778f8c3,// 150 PAY 147 

    0x2e5410fc,// 151 PAY 148 

    0xa49dbcf4,// 152 PAY 149 

    0x392b1124,// 153 PAY 150 

    0x2553d300,// 154 PAY 151 

    0xfc0305a7,// 155 PAY 152 

    0xe4fd922d,// 156 PAY 153 

    0xe5b0379c,// 157 PAY 154 

    0xdc0221ed,// 158 PAY 155 

    0xaf163ab8,// 159 PAY 156 

    0x95fe0955,// 160 PAY 157 

    0x915949b5,// 161 PAY 158 

    0x78814b58,// 162 PAY 159 

    0x604973c0,// 163 PAY 160 

    0x052fd3bd,// 164 PAY 161 

    0xc4d4f180,// 165 PAY 162 

    0xa6438492,// 166 PAY 163 

    0x64f00779,// 167 PAY 164 

    0x57551ac0,// 168 PAY 165 

    0xb079e2b8,// 169 PAY 166 

    0xb4f04480,// 170 PAY 167 

    0x6f06b6e4,// 171 PAY 168 

    0xef5ca4b9,// 172 PAY 169 

    0x24edc366,// 173 PAY 170 

    0xa76548c3,// 174 PAY 171 

    0xf2118ec4,// 175 PAY 172 

    0xb88c6795,// 176 PAY 173 

    0x990d6ffc,// 177 PAY 174 

    0x4a70e0c6,// 178 PAY 175 

    0x92471cd5,// 179 PAY 176 

    0xf59d1f45,// 180 PAY 177 

    0x71b21bb8,// 181 PAY 178 

    0x2823ed7b,// 182 PAY 179 

    0xac26dddf,// 183 PAY 180 

    0x8b60d2af,// 184 PAY 181 

    0xa4000000,// 185 PAY 182 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf 

    0x01690f2d // 186 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt80_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 168 words. 

/// BDA size     is 665 (0x299) 

/// BDA id       is 0xcf5d 

    0x0299cf5d,// 3 BDA   1 

/// PAY Generic Data size   : 665 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x112fb103,// 4 PAY   1 

    0xef887df9,// 5 PAY   2 

    0xe0ca6ffc,// 6 PAY   3 

    0x6855b46a,// 7 PAY   4 

    0x6a9070b0,// 8 PAY   5 

    0xa7fc897b,// 9 PAY   6 

    0x312a179b,// 10 PAY   7 

    0x476e8d63,// 11 PAY   8 

    0x5ef5fb7e,// 12 PAY   9 

    0x005764ba,// 13 PAY  10 

    0x4f58cdb3,// 14 PAY  11 

    0x4a3965b4,// 15 PAY  12 

    0xf6900a8d,// 16 PAY  13 

    0xd6423a49,// 17 PAY  14 

    0x8e7469be,// 18 PAY  15 

    0xc7443611,// 19 PAY  16 

    0x2f4d2943,// 20 PAY  17 

    0xc8f26b56,// 21 PAY  18 

    0x58bdcd04,// 22 PAY  19 

    0xd52db8dd,// 23 PAY  20 

    0xd4f0bb7e,// 24 PAY  21 

    0xb4c14021,// 25 PAY  22 

    0xf3e11b10,// 26 PAY  23 

    0xa3de78a4,// 27 PAY  24 

    0xd2fd495d,// 28 PAY  25 

    0xe48fb4cb,// 29 PAY  26 

    0x3696e254,// 30 PAY  27 

    0xe37cf27c,// 31 PAY  28 

    0xaec10fe6,// 32 PAY  29 

    0x81a2d3b2,// 33 PAY  30 

    0x8af770c8,// 34 PAY  31 

    0xe73cb39d,// 35 PAY  32 

    0x490dfbc9,// 36 PAY  33 

    0x835f77b1,// 37 PAY  34 

    0x35105e96,// 38 PAY  35 

    0x3f40b00a,// 39 PAY  36 

    0xdfd1e824,// 40 PAY  37 

    0x110ba00e,// 41 PAY  38 

    0xbbb908c2,// 42 PAY  39 

    0xa3adb4fd,// 43 PAY  40 

    0x2a4bc648,// 44 PAY  41 

    0x7a822d15,// 45 PAY  42 

    0xd4458b6d,// 46 PAY  43 

    0xa151dbcc,// 47 PAY  44 

    0xd536a8be,// 48 PAY  45 

    0xa0ed7077,// 49 PAY  46 

    0xd984cbfb,// 50 PAY  47 

    0xc4851837,// 51 PAY  48 

    0xa65100f7,// 52 PAY  49 

    0xf76427b2,// 53 PAY  50 

    0xabb432e4,// 54 PAY  51 

    0xb01db8fc,// 55 PAY  52 

    0xc5bc3a0c,// 56 PAY  53 

    0xa183e3f4,// 57 PAY  54 

    0x7344fe49,// 58 PAY  55 

    0x897c0588,// 59 PAY  56 

    0xada5b8f3,// 60 PAY  57 

    0x14181eae,// 61 PAY  58 

    0xf96f9905,// 62 PAY  59 

    0x56e75513,// 63 PAY  60 

    0xaef59fcf,// 64 PAY  61 

    0x52f6d1f5,// 65 PAY  62 

    0x864032e2,// 66 PAY  63 

    0x182e8803,// 67 PAY  64 

    0xea68f07a,// 68 PAY  65 

    0x1945b9c9,// 69 PAY  66 

    0xe1d9b0c8,// 70 PAY  67 

    0xd4f6262a,// 71 PAY  68 

    0x94a0f6ec,// 72 PAY  69 

    0xb4abb6a3,// 73 PAY  70 

    0x289bb28f,// 74 PAY  71 

    0x0e14c402,// 75 PAY  72 

    0xa3ef2e26,// 76 PAY  73 

    0xc3d52946,// 77 PAY  74 

    0xe42245b0,// 78 PAY  75 

    0x4ec3fbd5,// 79 PAY  76 

    0x3d44e06b,// 80 PAY  77 

    0x12ea2e9e,// 81 PAY  78 

    0x2164aba6,// 82 PAY  79 

    0x9cebfc8a,// 83 PAY  80 

    0xbc5de34b,// 84 PAY  81 

    0x5d54c7c9,// 85 PAY  82 

    0x1eeb1d8b,// 86 PAY  83 

    0x54adb716,// 87 PAY  84 

    0x91595072,// 88 PAY  85 

    0xbb416e7c,// 89 PAY  86 

    0x4a562c2a,// 90 PAY  87 

    0xebb2d9ef,// 91 PAY  88 

    0x38d4d8eb,// 92 PAY  89 

    0x48bcd044,// 93 PAY  90 

    0x0665d844,// 94 PAY  91 

    0x79189ee0,// 95 PAY  92 

    0x1def1323,// 96 PAY  93 

    0x44ffdc79,// 97 PAY  94 

    0xade654d7,// 98 PAY  95 

    0x9efc6494,// 99 PAY  96 

    0xf8c77b68,// 100 PAY  97 

    0x66bb7e0c,// 101 PAY  98 

    0x4dd25f69,// 102 PAY  99 

    0x0812de14,// 103 PAY 100 

    0xd3d3e3f2,// 104 PAY 101 

    0x821bcf82,// 105 PAY 102 

    0xe15e6e90,// 106 PAY 103 

    0x4e6fa30d,// 107 PAY 104 

    0x1e8b4b0c,// 108 PAY 105 

    0xf947239b,// 109 PAY 106 

    0xb0b2b466,// 110 PAY 107 

    0x7aef5ae3,// 111 PAY 108 

    0x17b4e94d,// 112 PAY 109 

    0xd933da6e,// 113 PAY 110 

    0x4fd95c06,// 114 PAY 111 

    0xa1623311,// 115 PAY 112 

    0x75601988,// 116 PAY 113 

    0xb225ade6,// 117 PAY 114 

    0x4ee01029,// 118 PAY 115 

    0x10003ca7,// 119 PAY 116 

    0xaae10b5a,// 120 PAY 117 

    0xe26bc0a3,// 121 PAY 118 

    0xd11f6e9c,// 122 PAY 119 

    0xf91533ed,// 123 PAY 120 

    0x060d7413,// 124 PAY 121 

    0x1d3dae7b,// 125 PAY 122 

    0x294a23d8,// 126 PAY 123 

    0x76ac0404,// 127 PAY 124 

    0xc9c5f018,// 128 PAY 125 

    0xb9b78485,// 129 PAY 126 

    0xa9756d9a,// 130 PAY 127 

    0xec6cd481,// 131 PAY 128 

    0x6cbbf150,// 132 PAY 129 

    0xc848a996,// 133 PAY 130 

    0xf30ca2b7,// 134 PAY 131 

    0x1d232061,// 135 PAY 132 

    0x57654e31,// 136 PAY 133 

    0x34479af6,// 137 PAY 134 

    0x3c3b7cbb,// 138 PAY 135 

    0xcf5730c3,// 139 PAY 136 

    0xd750d82a,// 140 PAY 137 

    0x4df92c08,// 141 PAY 138 

    0xfaf839f7,// 142 PAY 139 

    0x40e4c1e3,// 143 PAY 140 

    0xd278fc96,// 144 PAY 141 

    0x2da20403,// 145 PAY 142 

    0x3476ba39,// 146 PAY 143 

    0xf6c41f30,// 147 PAY 144 

    0x74978845,// 148 PAY 145 

    0x74b6635f,// 149 PAY 146 

    0x18c7de26,// 150 PAY 147 

    0x4c774732,// 151 PAY 148 

    0xb8623b91,// 152 PAY 149 

    0xf0ddc803,// 153 PAY 150 

    0x12c20915,// 154 PAY 151 

    0xcc143594,// 155 PAY 152 

    0x3c20995f,// 156 PAY 153 

    0x6c935059,// 157 PAY 154 

    0x4e514b5d,// 158 PAY 155 

    0x8f2e56fb,// 159 PAY 156 

    0xc12d944b,// 160 PAY 157 

    0x7ebc51c2,// 161 PAY 158 

    0xd709d9d9,// 162 PAY 159 

    0x5426cba0,// 163 PAY 160 

    0x0c6263c0,// 164 PAY 161 

    0xe9790a02,// 165 PAY 162 

    0x88825991,// 166 PAY 163 

    0x1d538121,// 167 PAY 164 

    0x7b4e78c5,// 168 PAY 165 

    0x68c0e96d,// 169 PAY 166 

    0xf7000000,// 170 PAY 167 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 255 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x03fc0485 // 171 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 80 (0x50) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt81_tmpl[] = {
    0x08010050,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 156 words. 

/// BDA size     is 619 (0x26b) 

/// BDA id       is 0xe599 

    0x026be599,// 3 BDA   1 

/// PAY Generic Data size   : 619 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x1e6fd184,// 4 PAY   1 

    0xf5c0b2e6,// 5 PAY   2 

    0x6d967f18,// 6 PAY   3 

    0xd7edadc5,// 7 PAY   4 

    0x9a2ddec2,// 8 PAY   5 

    0xe515affa,// 9 PAY   6 

    0xe755b4eb,// 10 PAY   7 

    0x93656266,// 11 PAY   8 

    0x107336cc,// 12 PAY   9 

    0x0cc9ad16,// 13 PAY  10 

    0xff0bd07e,// 14 PAY  11 

    0x979a7b90,// 15 PAY  12 

    0x24853716,// 16 PAY  13 

    0x0362db59,// 17 PAY  14 

    0x7b3a7e30,// 18 PAY  15 

    0x8661bf6e,// 19 PAY  16 

    0x5ba02c50,// 20 PAY  17 

    0x44787274,// 21 PAY  18 

    0x2087cbf0,// 22 PAY  19 

    0xd08e9cd4,// 23 PAY  20 

    0x949251fc,// 24 PAY  21 

    0x02df3435,// 25 PAY  22 

    0xb8163cac,// 26 PAY  23 

    0x0bda4a97,// 27 PAY  24 

    0x3dcbedd2,// 28 PAY  25 

    0x00988fe3,// 29 PAY  26 

    0x770b4849,// 30 PAY  27 

    0xaf774cd3,// 31 PAY  28 

    0x2c40d3cc,// 32 PAY  29 

    0xff9a8e57,// 33 PAY  30 

    0x75f27f59,// 34 PAY  31 

    0x5015a05e,// 35 PAY  32 

    0xec399f06,// 36 PAY  33 

    0x15a33a46,// 37 PAY  34 

    0x5c9ecf67,// 38 PAY  35 

    0xc776eb66,// 39 PAY  36 

    0xcdee9cbd,// 40 PAY  37 

    0x6230f2e9,// 41 PAY  38 

    0x18b8d48a,// 42 PAY  39 

    0x57dbe6ba,// 43 PAY  40 

    0x68cbdffe,// 44 PAY  41 

    0x898c3b92,// 45 PAY  42 

    0x0360091a,// 46 PAY  43 

    0x1bceeb47,// 47 PAY  44 

    0x71ca1a0c,// 48 PAY  45 

    0x95a8a34a,// 49 PAY  46 

    0x9b9a04c6,// 50 PAY  47 

    0xf25e742a,// 51 PAY  48 

    0xba7017ee,// 52 PAY  49 

    0x22c16305,// 53 PAY  50 

    0x2ca4d41d,// 54 PAY  51 

    0xddee6527,// 55 PAY  52 

    0xba2b29ab,// 56 PAY  53 

    0x1ebc7565,// 57 PAY  54 

    0xdb094193,// 58 PAY  55 

    0x0f8012c2,// 59 PAY  56 

    0x6ddae70e,// 60 PAY  57 

    0x3e9dc1f3,// 61 PAY  58 

    0xe63acc4c,// 62 PAY  59 

    0x365c3be3,// 63 PAY  60 

    0x7a3abdaa,// 64 PAY  61 

    0x8464bfe7,// 65 PAY  62 

    0x09c0e809,// 66 PAY  63 

    0xe5307516,// 67 PAY  64 

    0xa8c84596,// 68 PAY  65 

    0x03246ea4,// 69 PAY  66 

    0xcbc7592a,// 70 PAY  67 

    0x36dc0f5c,// 71 PAY  68 

    0xe4c8b61b,// 72 PAY  69 

    0x31d91536,// 73 PAY  70 

    0xb016bbf4,// 74 PAY  71 

    0x90a5f955,// 75 PAY  72 

    0x88d4e711,// 76 PAY  73 

    0x9db17ab5,// 77 PAY  74 

    0xad449f9e,// 78 PAY  75 

    0x292e7b21,// 79 PAY  76 

    0x7486dcc0,// 80 PAY  77 

    0xe0fa86d8,// 81 PAY  78 

    0x9b72939b,// 82 PAY  79 

    0x99402dee,// 83 PAY  80 

    0x93804a5d,// 84 PAY  81 

    0xda3217e6,// 85 PAY  82 

    0x97d7c005,// 86 PAY  83 

    0xe3d6095a,// 87 PAY  84 

    0xcf884b0f,// 88 PAY  85 

    0xd0095a68,// 89 PAY  86 

    0xb528602a,// 90 PAY  87 

    0x59d290b7,// 91 PAY  88 

    0x490dcc6c,// 92 PAY  89 

    0x1eb91476,// 93 PAY  90 

    0x17f09b38,// 94 PAY  91 

    0x2a5b3e7f,// 95 PAY  92 

    0x8e342218,// 96 PAY  93 

    0xcab2b610,// 97 PAY  94 

    0x113c97cc,// 98 PAY  95 

    0x08ee2f74,// 99 PAY  96 

    0xb3b7f18b,// 100 PAY  97 

    0x64242487,// 101 PAY  98 

    0xbf23fc13,// 102 PAY  99 

    0x9d7b9bb6,// 103 PAY 100 

    0x684782d2,// 104 PAY 101 

    0xea416f87,// 105 PAY 102 

    0x3edc4314,// 106 PAY 103 

    0xa13040a7,// 107 PAY 104 

    0x372b5406,// 108 PAY 105 

    0x135757ff,// 109 PAY 106 

    0x52cbb5dc,// 110 PAY 107 

    0x83006b8f,// 111 PAY 108 

    0xe18c914c,// 112 PAY 109 

    0x13fc0b6c,// 113 PAY 110 

    0xb6d21180,// 114 PAY 111 

    0xf232d9c4,// 115 PAY 112 

    0x2d251337,// 116 PAY 113 

    0xf1c7a9f1,// 117 PAY 114 

    0xb9c1fd46,// 118 PAY 115 

    0xf82ea63a,// 119 PAY 116 

    0xbe41a74c,// 120 PAY 117 

    0x007c799e,// 121 PAY 118 

    0x876ca738,// 122 PAY 119 

    0xca5862f5,// 123 PAY 120 

    0x6566ac5e,// 124 PAY 121 

    0xcb259acf,// 125 PAY 122 

    0xeeafdf22,// 126 PAY 123 

    0x3b39706d,// 127 PAY 124 

    0x23e5c72b,// 128 PAY 125 

    0xbd5edce5,// 129 PAY 126 

    0xda6488d8,// 130 PAY 127 

    0xaf8ed06d,// 131 PAY 128 

    0xfa992f35,// 132 PAY 129 

    0x6935044b,// 133 PAY 130 

    0xdcf6f4a9,// 134 PAY 131 

    0xf925f9d9,// 135 PAY 132 

    0xb6642cbe,// 136 PAY 133 

    0x1654f63f,// 137 PAY 134 

    0xdedb3b57,// 138 PAY 135 

    0x31dc8ee6,// 139 PAY 136 

    0x9e7e3505,// 140 PAY 137 

    0x23ad48a1,// 141 PAY 138 

    0x464cb102,// 142 PAY 139 

    0x4c82bd54,// 143 PAY 140 

    0x42484f9c,// 144 PAY 141 

    0xfead2285,// 145 PAY 142 

    0x11ffbd75,// 146 PAY 143 

    0x88c85ebb,// 147 PAY 144 

    0xce497c5b,// 148 PAY 145 

    0xc7649772,// 149 PAY 146 

    0x6dc01a2c,// 150 PAY 147 

    0x86dcebb5,// 151 PAY 148 

    0x941c5b30,// 152 PAY 149 

    0x7dc771bc,// 153 PAY 150 

    0x8324547b,// 154 PAY 151 

    0x318d0a90,// 155 PAY 152 

    0x662c14f7,// 156 PAY 153 

    0x769c77f8,// 157 PAY 154 

    0x11acf300,// 158 PAY 155 

/// STA is 1 words. 

/// STA num_pkts       : 210 

/// STA pkt_idx        : 251 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1f 

    0x03ed1fd2 // 159 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt82_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 260 words. 

/// BDA size     is 1034 (0x40a) 

/// BDA id       is 0x8517 

    0x040a8517,// 3 BDA   1 

/// PAY Generic Data size   : 1034 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x5c7865cf,// 4 PAY   1 

    0xe1bf0fa5,// 5 PAY   2 

    0x0ec9ba25,// 6 PAY   3 

    0xe906f9da,// 7 PAY   4 

    0x7b8d2a24,// 8 PAY   5 

    0xfb741fe8,// 9 PAY   6 

    0xefa61b68,// 10 PAY   7 

    0x8df342b2,// 11 PAY   8 

    0x69c92aa1,// 12 PAY   9 

    0xb2f92f4e,// 13 PAY  10 

    0x4f3e3a3b,// 14 PAY  11 

    0xd67949c1,// 15 PAY  12 

    0x2a5f2927,// 16 PAY  13 

    0x16fdf7fc,// 17 PAY  14 

    0xc7e5a0c9,// 18 PAY  15 

    0xc8bcd42d,// 19 PAY  16 

    0xfab67935,// 20 PAY  17 

    0xf48c16ba,// 21 PAY  18 

    0xd988788a,// 22 PAY  19 

    0x88697738,// 23 PAY  20 

    0x96c0e8bf,// 24 PAY  21 

    0x18834df7,// 25 PAY  22 

    0xad5b163f,// 26 PAY  23 

    0xa20a9405,// 27 PAY  24 

    0xc4e3530b,// 28 PAY  25 

    0xa4815b32,// 29 PAY  26 

    0x06220567,// 30 PAY  27 

    0xc83b3e39,// 31 PAY  28 

    0xe4840966,// 32 PAY  29 

    0x7e18622e,// 33 PAY  30 

    0xddde866c,// 34 PAY  31 

    0x8e448dc9,// 35 PAY  32 

    0xfb8d51a3,// 36 PAY  33 

    0xf07b7470,// 37 PAY  34 

    0xee2e661b,// 38 PAY  35 

    0xc56bf53f,// 39 PAY  36 

    0x551aaf17,// 40 PAY  37 

    0x92327589,// 41 PAY  38 

    0xd4a59a11,// 42 PAY  39 

    0x1432fe7d,// 43 PAY  40 

    0x2ca9256a,// 44 PAY  41 

    0x12443a61,// 45 PAY  42 

    0x2e543867,// 46 PAY  43 

    0x38b0535a,// 47 PAY  44 

    0xefaf3f9e,// 48 PAY  45 

    0xba340ac8,// 49 PAY  46 

    0x2d488251,// 50 PAY  47 

    0x370da4a3,// 51 PAY  48 

    0xbd2f6847,// 52 PAY  49 

    0x688d5a54,// 53 PAY  50 

    0x3ee28453,// 54 PAY  51 

    0xbcf2d16d,// 55 PAY  52 

    0x308cdf6e,// 56 PAY  53 

    0x95cb0436,// 57 PAY  54 

    0xa1cc0897,// 58 PAY  55 

    0x578dce5f,// 59 PAY  56 

    0x9e14e78f,// 60 PAY  57 

    0x2c046c5f,// 61 PAY  58 

    0x3d9d21db,// 62 PAY  59 

    0x484309f9,// 63 PAY  60 

    0x1c54143c,// 64 PAY  61 

    0x3242af52,// 65 PAY  62 

    0x7ed3a3e7,// 66 PAY  63 

    0x182a9e19,// 67 PAY  64 

    0x3870b9e1,// 68 PAY  65 

    0xcac12dc2,// 69 PAY  66 

    0xc8a2d2c8,// 70 PAY  67 

    0x6a8bb317,// 71 PAY  68 

    0xe792e2f7,// 72 PAY  69 

    0x762bccff,// 73 PAY  70 

    0x3cc91118,// 74 PAY  71 

    0x47578617,// 75 PAY  72 

    0x8c742c85,// 76 PAY  73 

    0x2acbf70f,// 77 PAY  74 

    0xf5f4378b,// 78 PAY  75 

    0xdd4c9b92,// 79 PAY  76 

    0xf64d9c6b,// 80 PAY  77 

    0x1f0877c4,// 81 PAY  78 

    0x17316e4e,// 82 PAY  79 

    0xbd9f60ae,// 83 PAY  80 

    0xe168efe8,// 84 PAY  81 

    0x201e18ea,// 85 PAY  82 

    0xb6be696b,// 86 PAY  83 

    0x26d1bf9b,// 87 PAY  84 

    0x56c3bfa0,// 88 PAY  85 

    0xcf302eb4,// 89 PAY  86 

    0x0e30f959,// 90 PAY  87 

    0x44220191,// 91 PAY  88 

    0x72ffcca6,// 92 PAY  89 

    0x85e8e7e4,// 93 PAY  90 

    0x26a36b52,// 94 PAY  91 

    0x9d4660d7,// 95 PAY  92 

    0x7106584a,// 96 PAY  93 

    0xb7a77bf4,// 97 PAY  94 

    0xb03aec67,// 98 PAY  95 

    0x9583e68e,// 99 PAY  96 

    0x6c761c7c,// 100 PAY  97 

    0xa721fb50,// 101 PAY  98 

    0xd482ed86,// 102 PAY  99 

    0x4095d279,// 103 PAY 100 

    0x85e1000c,// 104 PAY 101 

    0xc04a1a5c,// 105 PAY 102 

    0xcbfb6eae,// 106 PAY 103 

    0x49e54a93,// 107 PAY 104 

    0x1cee031c,// 108 PAY 105 

    0x9f632453,// 109 PAY 106 

    0xf7e9470e,// 110 PAY 107 

    0x9c020c45,// 111 PAY 108 

    0x3227fc3d,// 112 PAY 109 

    0x9e8a0fe7,// 113 PAY 110 

    0x84c83bdc,// 114 PAY 111 

    0xa176eea1,// 115 PAY 112 

    0x8af0b079,// 116 PAY 113 

    0xe068ee4e,// 117 PAY 114 

    0x274fba59,// 118 PAY 115 

    0xa93e9ee2,// 119 PAY 116 

    0x71dcb54c,// 120 PAY 117 

    0x01980e08,// 121 PAY 118 

    0x66ab7e5a,// 122 PAY 119 

    0x2174e03c,// 123 PAY 120 

    0x6f7719ae,// 124 PAY 121 

    0x51d6f47f,// 125 PAY 122 

    0x48b6d299,// 126 PAY 123 

    0xe0117d0c,// 127 PAY 124 

    0xacb1c578,// 128 PAY 125 

    0x1be9aad3,// 129 PAY 126 

    0x54d806f7,// 130 PAY 127 

    0x9b01eac1,// 131 PAY 128 

    0x02edebd4,// 132 PAY 129 

    0x3ef34c60,// 133 PAY 130 

    0x9be1fbf7,// 134 PAY 131 

    0x272d4779,// 135 PAY 132 

    0x7d3ba83b,// 136 PAY 133 

    0xb40cbf9b,// 137 PAY 134 

    0xc39927d1,// 138 PAY 135 

    0x49043f9b,// 139 PAY 136 

    0xa0ed345d,// 140 PAY 137 

    0xc19299e8,// 141 PAY 138 

    0xda82b3d5,// 142 PAY 139 

    0xd1798287,// 143 PAY 140 

    0xae7cae37,// 144 PAY 141 

    0x4b9c1051,// 145 PAY 142 

    0x3cecf81c,// 146 PAY 143 

    0x2a46f716,// 147 PAY 144 

    0x884b2474,// 148 PAY 145 

    0xa13a5776,// 149 PAY 146 

    0x799451c1,// 150 PAY 147 

    0x5ddb1714,// 151 PAY 148 

    0xd6328035,// 152 PAY 149 

    0x40ee3941,// 153 PAY 150 

    0x18f8c0be,// 154 PAY 151 

    0x609ffd5e,// 155 PAY 152 

    0xd465c598,// 156 PAY 153 

    0x8273755b,// 157 PAY 154 

    0x437afbc1,// 158 PAY 155 

    0x94d7f7f4,// 159 PAY 156 

    0xc73104ed,// 160 PAY 157 

    0xcc60bd6b,// 161 PAY 158 

    0x3217fed0,// 162 PAY 159 

    0x6c015a9d,// 163 PAY 160 

    0x691bed09,// 164 PAY 161 

    0xd487584e,// 165 PAY 162 

    0x36ce2fb7,// 166 PAY 163 

    0x960ddaac,// 167 PAY 164 

    0x1a38e754,// 168 PAY 165 

    0x4f85cd47,// 169 PAY 166 

    0x3b7769a3,// 170 PAY 167 

    0x3c292992,// 171 PAY 168 

    0x1c596e3e,// 172 PAY 169 

    0x5d72d9bb,// 173 PAY 170 

    0xe949f9de,// 174 PAY 171 

    0x7d9238ec,// 175 PAY 172 

    0x8d20709b,// 176 PAY 173 

    0xf16ba776,// 177 PAY 174 

    0x37ee35f7,// 178 PAY 175 

    0xfce72a3f,// 179 PAY 176 

    0x62c00fe9,// 180 PAY 177 

    0xd93f44a1,// 181 PAY 178 

    0x88a0675a,// 182 PAY 179 

    0x3cb21dc2,// 183 PAY 180 

    0x348db55e,// 184 PAY 181 

    0x538f88c5,// 185 PAY 182 

    0x894437d1,// 186 PAY 183 

    0xfb553156,// 187 PAY 184 

    0xf702b54c,// 188 PAY 185 

    0x197211a8,// 189 PAY 186 

    0xedd9af4a,// 190 PAY 187 

    0xbc28328d,// 191 PAY 188 

    0xd86f9873,// 192 PAY 189 

    0x773cbc0b,// 193 PAY 190 

    0x01f161b5,// 194 PAY 191 

    0x89614a0f,// 195 PAY 192 

    0x8fa6b2ad,// 196 PAY 193 

    0x1b7f8bf2,// 197 PAY 194 

    0x87ffd5f5,// 198 PAY 195 

    0xb88715cd,// 199 PAY 196 

    0x71a43545,// 200 PAY 197 

    0xcd6e2561,// 201 PAY 198 

    0x60223309,// 202 PAY 199 

    0x48c53c8a,// 203 PAY 200 

    0xac0c211b,// 204 PAY 201 

    0x624ee381,// 205 PAY 202 

    0xc0097bc3,// 206 PAY 203 

    0x3c1d74a8,// 207 PAY 204 

    0x6f89b449,// 208 PAY 205 

    0xec0018fd,// 209 PAY 206 

    0x977a52ef,// 210 PAY 207 

    0xe3bb7cfc,// 211 PAY 208 

    0x2e56a8d4,// 212 PAY 209 

    0xef5e05f8,// 213 PAY 210 

    0xfddd61e9,// 214 PAY 211 

    0x84322ed4,// 215 PAY 212 

    0x56e1fff9,// 216 PAY 213 

    0x634f1d73,// 217 PAY 214 

    0xb6f042f8,// 218 PAY 215 

    0x9c754e20,// 219 PAY 216 

    0xa527b7e4,// 220 PAY 217 

    0x64097978,// 221 PAY 218 

    0xf27caa08,// 222 PAY 219 

    0x7fb9ceba,// 223 PAY 220 

    0x762e4eb3,// 224 PAY 221 

    0x1dd1b64a,// 225 PAY 222 

    0x3f55444b,// 226 PAY 223 

    0xdb92f00a,// 227 PAY 224 

    0x1c973711,// 228 PAY 225 

    0x52652dff,// 229 PAY 226 

    0xe03fd1c8,// 230 PAY 227 

    0x5f9ccdc3,// 231 PAY 228 

    0x24c13386,// 232 PAY 229 

    0x9fd22ac0,// 233 PAY 230 

    0xa8b84ffc,// 234 PAY 231 

    0xb02e9a3b,// 235 PAY 232 

    0x2cc0ed1f,// 236 PAY 233 

    0x666ba837,// 237 PAY 234 

    0xd234d0d9,// 238 PAY 235 

    0xbe96a59c,// 239 PAY 236 

    0x70b3a743,// 240 PAY 237 

    0xccb088ff,// 241 PAY 238 

    0x56cd3919,// 242 PAY 239 

    0xa3946e78,// 243 PAY 240 

    0x9fe74185,// 244 PAY 241 

    0xd2e8070a,// 245 PAY 242 

    0x8d95a628,// 246 PAY 243 

    0x39edc4fd,// 247 PAY 244 

    0xc2578bd0,// 248 PAY 245 

    0xa521599c,// 249 PAY 246 

    0x320f5694,// 250 PAY 247 

    0x91ffe11f,// 251 PAY 248 

    0xeaf8a0fd,// 252 PAY 249 

    0x9792a92d,// 253 PAY 250 

    0x68d157fb,// 254 PAY 251 

    0x95633d51,// 255 PAY 252 

    0x354e53b8,// 256 PAY 253 

    0xa4211ba7,// 257 PAY 254 

    0x4d725ce4,// 258 PAY 255 

    0x66e420e8,// 259 PAY 256 

    0x09096b85,// 260 PAY 257 

    0xac0bbd21,// 261 PAY 258 

    0xc0190000,// 262 PAY 259 

/// HASH is  16 bytes 

    0x272d4779,// 263 HSH   1 

    0x7d3ba83b,// 264 HSH   2 

    0xb40cbf9b,// 265 HSH   3 

    0xc39927d1,// 266 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 81 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdc 

    0x03dcdc51 // 267 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt83_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 63 words. 

/// BDA size     is 248 (0xf8) 

/// BDA id       is 0x8d6d 

    0x00f88d6d,// 3 BDA   1 

/// PAY Generic Data size   : 248 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x3d345847,// 4 PAY   1 

    0x75cf24ed,// 5 PAY   2 

    0xbbba0272,// 6 PAY   3 

    0x7f218899,// 7 PAY   4 

    0x323773d8,// 8 PAY   5 

    0x34227fef,// 9 PAY   6 

    0x742d53b5,// 10 PAY   7 

    0xc17677f9,// 11 PAY   8 

    0x0ceb32b4,// 12 PAY   9 

    0x54cb7777,// 13 PAY  10 

    0xeb13bccb,// 14 PAY  11 

    0x8d257e35,// 15 PAY  12 

    0x3d16a20b,// 16 PAY  13 

    0x2a8efe05,// 17 PAY  14 

    0x09567af1,// 18 PAY  15 

    0x2d5ddb01,// 19 PAY  16 

    0x57702a3b,// 20 PAY  17 

    0xd3e6c183,// 21 PAY  18 

    0x79c4f58f,// 22 PAY  19 

    0x203e4abd,// 23 PAY  20 

    0x62ef77c1,// 24 PAY  21 

    0x9357281e,// 25 PAY  22 

    0x7f0fb911,// 26 PAY  23 

    0x659ae8fc,// 27 PAY  24 

    0x9c431cb1,// 28 PAY  25 

    0x1e91a8d1,// 29 PAY  26 

    0xb3b344ca,// 30 PAY  27 

    0x6b8d17d0,// 31 PAY  28 

    0x5106272c,// 32 PAY  29 

    0xbd9eb1a9,// 33 PAY  30 

    0x91463475,// 34 PAY  31 

    0x85ae7c11,// 35 PAY  32 

    0xe3e99b37,// 36 PAY  33 

    0x9108b5c5,// 37 PAY  34 

    0x82b0eaa1,// 38 PAY  35 

    0x314ac907,// 39 PAY  36 

    0xae06ac7f,// 40 PAY  37 

    0x2bbd51ed,// 41 PAY  38 

    0x4c2f7471,// 42 PAY  39 

    0x5a2677ab,// 43 PAY  40 

    0xf5968ae6,// 44 PAY  41 

    0x7d5f67ca,// 45 PAY  42 

    0xa135c250,// 46 PAY  43 

    0x61a6781d,// 47 PAY  44 

    0x07b21f6f,// 48 PAY  45 

    0x28f25a8b,// 49 PAY  46 

    0xd0bf1b28,// 50 PAY  47 

    0x63a0dcc3,// 51 PAY  48 

    0x97773b16,// 52 PAY  49 

    0xf4e2beff,// 53 PAY  50 

    0xef330c9d,// 54 PAY  51 

    0x0fd2a971,// 55 PAY  52 

    0xbe1a9f7b,// 56 PAY  53 

    0x187196bd,// 57 PAY  54 

    0xca644892,// 58 PAY  55 

    0xd902fa54,// 59 PAY  56 

    0xed91cf0b,// 60 PAY  57 

    0x9bbf0d02,// 61 PAY  58 

    0x25f84150,// 62 PAY  59 

    0x2c9168c6,// 63 PAY  60 

    0x11e4eb6e,// 64 PAY  61 

    0x94230425,// 65 PAY  62 

/// HASH is  12 bytes 

    0xca644892,// 66 HSH   1 

    0xd902fa54,// 67 HSH   2 

    0xed91cf0b,// 68 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 206 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x16 

    0x03391649 // 69 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt84_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 229 words. 

/// BDA size     is 909 (0x38d) 

/// BDA id       is 0x8ef 

    0x038d08ef,// 3 BDA   1 

/// PAY Generic Data size   : 909 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x8bcc3884,// 4 PAY   1 

    0x43b24b86,// 5 PAY   2 

    0xda18df92,// 6 PAY   3 

    0x0289e5a1,// 7 PAY   4 

    0x30429588,// 8 PAY   5 

    0x0083015f,// 9 PAY   6 

    0x8dd84a57,// 10 PAY   7 

    0xff187053,// 11 PAY   8 

    0xc84c1a22,// 12 PAY   9 

    0x5ddf92f3,// 13 PAY  10 

    0x575e80d5,// 14 PAY  11 

    0x506bb79b,// 15 PAY  12 

    0x498e56aa,// 16 PAY  13 

    0xf465763f,// 17 PAY  14 

    0x51643cc0,// 18 PAY  15 

    0x0cacac93,// 19 PAY  16 

    0xb60e1ab5,// 20 PAY  17 

    0x96f2e49b,// 21 PAY  18 

    0xc0ec5b55,// 22 PAY  19 

    0xe49ac39e,// 23 PAY  20 

    0x2466371d,// 24 PAY  21 

    0x2fa965db,// 25 PAY  22 

    0x5e997376,// 26 PAY  23 

    0x412bf2eb,// 27 PAY  24 

    0x0b2483f3,// 28 PAY  25 

    0xf9b941ec,// 29 PAY  26 

    0xebc5d1a9,// 30 PAY  27 

    0x1e8c3287,// 31 PAY  28 

    0xa68a1469,// 32 PAY  29 

    0xc2a85e9f,// 33 PAY  30 

    0xd1871512,// 34 PAY  31 

    0x760fb4b5,// 35 PAY  32 

    0x415d2b4f,// 36 PAY  33 

    0xce6c9445,// 37 PAY  34 

    0x9f143d6f,// 38 PAY  35 

    0x76a89f3a,// 39 PAY  36 

    0x4ba3485c,// 40 PAY  37 

    0xf66e7cf0,// 41 PAY  38 

    0xc646dc8d,// 42 PAY  39 

    0xfd79d453,// 43 PAY  40 

    0xfd7d0d46,// 44 PAY  41 

    0xd79d722a,// 45 PAY  42 

    0x97ebbad4,// 46 PAY  43 

    0xffefd700,// 47 PAY  44 

    0x9fab533d,// 48 PAY  45 

    0xf91feefb,// 49 PAY  46 

    0xde531f5c,// 50 PAY  47 

    0x008d2522,// 51 PAY  48 

    0xf7a4aa2c,// 52 PAY  49 

    0x3f4de058,// 53 PAY  50 

    0xb36a15d0,// 54 PAY  51 

    0xbdfef8a6,// 55 PAY  52 

    0xabb190eb,// 56 PAY  53 

    0x27e11e59,// 57 PAY  54 

    0xfc4f1ced,// 58 PAY  55 

    0x85a9ead2,// 59 PAY  56 

    0xb6cd7379,// 60 PAY  57 

    0x9a5d61e0,// 61 PAY  58 

    0x56aeaa35,// 62 PAY  59 

    0x7f8a31d7,// 63 PAY  60 

    0xec6a8435,// 64 PAY  61 

    0xb8599bef,// 65 PAY  62 

    0x4ef20e68,// 66 PAY  63 

    0x677b1079,// 67 PAY  64 

    0xe7d1f895,// 68 PAY  65 

    0x8ce1a764,// 69 PAY  66 

    0xa0fe3268,// 70 PAY  67 

    0x1d81b6a9,// 71 PAY  68 

    0xf3abda71,// 72 PAY  69 

    0x4defc312,// 73 PAY  70 

    0xe7029aed,// 74 PAY  71 

    0xdcda25ad,// 75 PAY  72 

    0x37dcdc98,// 76 PAY  73 

    0xa1480b78,// 77 PAY  74 

    0x3d1bc612,// 78 PAY  75 

    0x311cf992,// 79 PAY  76 

    0xbb71d30a,// 80 PAY  77 

    0xbd4e0751,// 81 PAY  78 

    0x1a45a1ea,// 82 PAY  79 

    0xfe6be178,// 83 PAY  80 

    0xba9ccefb,// 84 PAY  81 

    0xc0c7bb84,// 85 PAY  82 

    0x27a61327,// 86 PAY  83 

    0xf5892ce6,// 87 PAY  84 

    0x2661b36f,// 88 PAY  85 

    0x2d63f449,// 89 PAY  86 

    0x527b9628,// 90 PAY  87 

    0x7b2b74ab,// 91 PAY  88 

    0x2ca7c009,// 92 PAY  89 

    0x38a645c1,// 93 PAY  90 

    0x8ff1fd64,// 94 PAY  91 

    0x6c6c8276,// 95 PAY  92 

    0xb1d57ddb,// 96 PAY  93 

    0x86a77899,// 97 PAY  94 

    0x2c6ffe4f,// 98 PAY  95 

    0x6ec91b2f,// 99 PAY  96 

    0x045ee6a3,// 100 PAY  97 

    0x2a56fbda,// 101 PAY  98 

    0x0ea239c5,// 102 PAY  99 

    0x19621cd4,// 103 PAY 100 

    0x8ab58366,// 104 PAY 101 

    0x407d869b,// 105 PAY 102 

    0xcdc3fee0,// 106 PAY 103 

    0x7a8bdcc2,// 107 PAY 104 

    0x5534f69c,// 108 PAY 105 

    0x3851b234,// 109 PAY 106 

    0x91a0cd44,// 110 PAY 107 

    0xc768ff08,// 111 PAY 108 

    0xf18bc453,// 112 PAY 109 

    0xc019f04c,// 113 PAY 110 

    0xab9e7d37,// 114 PAY 111 

    0xa884f9a5,// 115 PAY 112 

    0x3e135742,// 116 PAY 113 

    0x0112284e,// 117 PAY 114 

    0xd7d08d2a,// 118 PAY 115 

    0x964ac898,// 119 PAY 116 

    0xc630b07b,// 120 PAY 117 

    0xc2bca704,// 121 PAY 118 

    0xcfc317fc,// 122 PAY 119 

    0x1ce5de6e,// 123 PAY 120 

    0x036a891a,// 124 PAY 121 

    0x9ce7e9ab,// 125 PAY 122 

    0x26edeab2,// 126 PAY 123 

    0x9391010c,// 127 PAY 124 

    0x3a352344,// 128 PAY 125 

    0x0e1b4e63,// 129 PAY 126 

    0x3d6f8567,// 130 PAY 127 

    0xb98601a2,// 131 PAY 128 

    0xccbee3fc,// 132 PAY 129 

    0x863b9d04,// 133 PAY 130 

    0x93c2f1e8,// 134 PAY 131 

    0xbebf9b89,// 135 PAY 132 

    0x54ba02dd,// 136 PAY 133 

    0x5d00ac59,// 137 PAY 134 

    0xbb951554,// 138 PAY 135 

    0x71581e0d,// 139 PAY 136 

    0xbe226bbe,// 140 PAY 137 

    0x6d5324ce,// 141 PAY 138 

    0x6ab9204e,// 142 PAY 139 

    0xd8580ede,// 143 PAY 140 

    0xf7db4bbc,// 144 PAY 141 

    0x4c035a64,// 145 PAY 142 

    0x122b44f8,// 146 PAY 143 

    0x3eb135c3,// 147 PAY 144 

    0x027e2249,// 148 PAY 145 

    0xe9f2df3f,// 149 PAY 146 

    0x20be0f4a,// 150 PAY 147 

    0x5860f194,// 151 PAY 148 

    0x558a0c66,// 152 PAY 149 

    0xd48a0cc0,// 153 PAY 150 

    0xb0b2c9c9,// 154 PAY 151 

    0xbdec6f50,// 155 PAY 152 

    0xfba84aff,// 156 PAY 153 

    0xfb7de1c2,// 157 PAY 154 

    0x3bb6da6b,// 158 PAY 155 

    0xd7e5bab1,// 159 PAY 156 

    0x9780b2d0,// 160 PAY 157 

    0x8d3fdeac,// 161 PAY 158 

    0x9f8aad85,// 162 PAY 159 

    0xa236977b,// 163 PAY 160 

    0x719f9e79,// 164 PAY 161 

    0xde39bcde,// 165 PAY 162 

    0xe43a7e3a,// 166 PAY 163 

    0xf4fd7e32,// 167 PAY 164 

    0x3876d059,// 168 PAY 165 

    0xf642b889,// 169 PAY 166 

    0x40edb4b2,// 170 PAY 167 

    0xcffda78a,// 171 PAY 168 

    0xa0a7882a,// 172 PAY 169 

    0x0fafc609,// 173 PAY 170 

    0x642262a9,// 174 PAY 171 

    0xb3cfdf80,// 175 PAY 172 

    0x61ae36b1,// 176 PAY 173 

    0xf4ab69e8,// 177 PAY 174 

    0x0eee852d,// 178 PAY 175 

    0xb26279d5,// 179 PAY 176 

    0x50e4ca4f,// 180 PAY 177 

    0x9346777a,// 181 PAY 178 

    0x9f3c3309,// 182 PAY 179 

    0x61d5a722,// 183 PAY 180 

    0xfabbb03d,// 184 PAY 181 

    0x76dd7a13,// 185 PAY 182 

    0xeae999a1,// 186 PAY 183 

    0xfd140e9d,// 187 PAY 184 

    0x88a0e68b,// 188 PAY 185 

    0xf141e819,// 189 PAY 186 

    0x3bd0ccbe,// 190 PAY 187 

    0xe487ec4b,// 191 PAY 188 

    0x0194aee0,// 192 PAY 189 

    0xf84e5460,// 193 PAY 190 

    0x1cf868af,// 194 PAY 191 

    0xf7d71474,// 195 PAY 192 

    0x5610afdb,// 196 PAY 193 

    0x20ddf83f,// 197 PAY 194 

    0xbecfefef,// 198 PAY 195 

    0xc9aae30b,// 199 PAY 196 

    0x202a3b0e,// 200 PAY 197 

    0xf7a38702,// 201 PAY 198 

    0xa2c0259c,// 202 PAY 199 

    0x57ed21bf,// 203 PAY 200 

    0x4bf1aad9,// 204 PAY 201 

    0x3993e1f3,// 205 PAY 202 

    0x4ce29a6e,// 206 PAY 203 

    0xfe0d8f60,// 207 PAY 204 

    0x825ff14f,// 208 PAY 205 

    0x72bf4b3a,// 209 PAY 206 

    0x114a0abb,// 210 PAY 207 

    0x2cd7dad8,// 211 PAY 208 

    0xd31723dc,// 212 PAY 209 

    0x9db7195a,// 213 PAY 210 

    0x5eae8e1a,// 214 PAY 211 

    0x5e9b9b67,// 215 PAY 212 

    0x0092600b,// 216 PAY 213 

    0x6b5ff530,// 217 PAY 214 

    0x8aa2dfe7,// 218 PAY 215 

    0x33cc0ba3,// 219 PAY 216 

    0x93335d39,// 220 PAY 217 

    0xf9acb386,// 221 PAY 218 

    0xf1311540,// 222 PAY 219 

    0xb06392af,// 223 PAY 220 

    0x5851efff,// 224 PAY 221 

    0x50520ac2,// 225 PAY 222 

    0x08aad551,// 226 PAY 223 

    0xa76c5366,// 227 PAY 224 

    0x15a92790,// 228 PAY 225 

    0xc62b9b5a,// 229 PAY 226 

    0x7b2da50b,// 230 PAY 227 

    0x72000000,// 231 PAY 228 

/// STA is 1 words. 

/// STA num_pkts       : 53 

/// STA pkt_idx        : 49 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb4 

    0x00c4b435 // 232 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt85_tmpl[] = {
    0x08010060,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 81 words. 

/// BDA size     is 320 (0x140) 

/// BDA id       is 0xca9 

    0x01400ca9,// 3 BDA   1 

/// PAY Generic Data size   : 320 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

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

    0xcac47846,// 46 PAY  43 

    0x092d9ea2,// 47 PAY  44 

    0x481faa0e,// 48 PAY  45 

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

    0xb8367ebd,// 70 PAY  67 

    0xa027faee,// 71 PAY  68 

    0x197d793f,// 72 PAY  69 

    0x21893224,// 73 PAY  70 

    0xe2e6e261,// 74 PAY  71 

    0x8a2d8d44,// 75 PAY  72 

    0x7d00b943,// 76 PAY  73 

    0x040e94cd,// 77 PAY  74 

    0x3ab1a727,// 78 PAY  75 

    0x0b2187f4,// 79 PAY  76 

    0xfd052d1c,// 80 PAY  77 

    0x4732be9c,// 81 PAY  78 

    0xd233acf0,// 82 PAY  79 

    0xf685114f,// 83 PAY  80 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5e 

    0x00d55e7e // 84 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt86_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 167 words. 

/// BDA size     is 664 (0x298) 

/// BDA id       is 0xf6c2 

    0x0298f6c2,// 3 BDA   1 

/// PAY Generic Data size   : 664 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xdbbed9dd,// 4 PAY   1 

    0x12625417,// 5 PAY   2 

    0x3ba11e02,// 6 PAY   3 

    0x18fd13ee,// 7 PAY   4 

    0x3fcdfca0,// 8 PAY   5 

    0xe32d175f,// 9 PAY   6 

    0x4b32557f,// 10 PAY   7 

    0x82140a07,// 11 PAY   8 

    0x3bfb6992,// 12 PAY   9 

    0x6f2cef1d,// 13 PAY  10 

    0xc55959f7,// 14 PAY  11 

    0xdaed788a,// 15 PAY  12 

    0x96bc6719,// 16 PAY  13 

    0x72a95d9e,// 17 PAY  14 

    0x4d4a3e4d,// 18 PAY  15 

    0xa874d11c,// 19 PAY  16 

    0xc43f31a3,// 20 PAY  17 

    0x59341228,// 21 PAY  18 

    0x4260dcd2,// 22 PAY  19 

    0x93ded0fc,// 23 PAY  20 

    0x72aa71e3,// 24 PAY  21 

    0x25dfc285,// 25 PAY  22 

    0x21ee6f8c,// 26 PAY  23 

    0x8eb6ff63,// 27 PAY  24 

    0x3c4556f8,// 28 PAY  25 

    0x4a2c1f97,// 29 PAY  26 

    0x05bad142,// 30 PAY  27 

    0xf485909c,// 31 PAY  28 

    0x79e86bd0,// 32 PAY  29 

    0x46d6d21d,// 33 PAY  30 

    0x2feae521,// 34 PAY  31 

    0xf8c388e5,// 35 PAY  32 

    0xa38bd9b9,// 36 PAY  33 

    0x220729ec,// 37 PAY  34 

    0xa6e0e6e1,// 38 PAY  35 

    0xa72346e9,// 39 PAY  36 

    0x44e2f6a9,// 40 PAY  37 

    0xdfd1fb15,// 41 PAY  38 

    0x042536a1,// 42 PAY  39 

    0xc73d1c12,// 43 PAY  40 

    0xf23b4a7d,// 44 PAY  41 

    0x8386b044,// 45 PAY  42 

    0x47114a19,// 46 PAY  43 

    0x588c51c8,// 47 PAY  44 

    0x19cdd18c,// 48 PAY  45 

    0xa433baed,// 49 PAY  46 

    0x1e5e2317,// 50 PAY  47 

    0x0e0c59f3,// 51 PAY  48 

    0x172aa259,// 52 PAY  49 

    0x9579c235,// 53 PAY  50 

    0x9d5c0a74,// 54 PAY  51 

    0xfcf423ef,// 55 PAY  52 

    0x54736e8d,// 56 PAY  53 

    0xfe203872,// 57 PAY  54 

    0xcea72c11,// 58 PAY  55 

    0x878eeae3,// 59 PAY  56 

    0x7ae2daa3,// 60 PAY  57 

    0x772a75f9,// 61 PAY  58 

    0xefd1dbe1,// 62 PAY  59 

    0xd59a3f65,// 63 PAY  60 

    0x1cdbeba9,// 64 PAY  61 

    0x8aa9f34f,// 65 PAY  62 

    0x09bab02d,// 66 PAY  63 

    0xd9384ea7,// 67 PAY  64 

    0x4df80cf4,// 68 PAY  65 

    0x9eef6e81,// 69 PAY  66 

    0x6aa80b32,// 70 PAY  67 

    0x234fee70,// 71 PAY  68 

    0x1258c433,// 72 PAY  69 

    0xfd045260,// 73 PAY  70 

    0xb796052a,// 74 PAY  71 

    0xde5b41a9,// 75 PAY  72 

    0x66ad8510,// 76 PAY  73 

    0x5bc77597,// 77 PAY  74 

    0xdb59bd96,// 78 PAY  75 

    0x0b349a86,// 79 PAY  76 

    0xb8bd938f,// 80 PAY  77 

    0x18d5a502,// 81 PAY  78 

    0xa7ed1c74,// 82 PAY  79 

    0xeb5a62f2,// 83 PAY  80 

    0x73d837fa,// 84 PAY  81 

    0x9063ed9f,// 85 PAY  82 

    0x424af5a9,// 86 PAY  83 

    0x70fe5f16,// 87 PAY  84 

    0x5cc4e16a,// 88 PAY  85 

    0x270c0b3c,// 89 PAY  86 

    0x2733d574,// 90 PAY  87 

    0x5bb80ee0,// 91 PAY  88 

    0x0a280673,// 92 PAY  89 

    0x3a55428b,// 93 PAY  90 

    0x743274d2,// 94 PAY  91 

    0x3e012be5,// 95 PAY  92 

    0x61258a40,// 96 PAY  93 

    0x0792028a,// 97 PAY  94 

    0xe596b208,// 98 PAY  95 

    0x75b3c00b,// 99 PAY  96 

    0xf0662147,// 100 PAY  97 

    0x75326226,// 101 PAY  98 

    0x1918d33b,// 102 PAY  99 

    0x9a9fd0d3,// 103 PAY 100 

    0xab74b1d2,// 104 PAY 101 

    0x216913de,// 105 PAY 102 

    0xc11e3931,// 106 PAY 103 

    0x8545e6e4,// 107 PAY 104 

    0x505b941f,// 108 PAY 105 

    0xf45e6606,// 109 PAY 106 

    0x3f3a1504,// 110 PAY 107 

    0x66df181c,// 111 PAY 108 

    0xe2cd4a52,// 112 PAY 109 

    0xa3a89c7f,// 113 PAY 110 

    0x3a54e520,// 114 PAY 111 

    0x31d9d5d0,// 115 PAY 112 

    0x53b7fa09,// 116 PAY 113 

    0xad0922cc,// 117 PAY 114 

    0xfadcda04,// 118 PAY 115 

    0x17bf791e,// 119 PAY 116 

    0x92a1f21c,// 120 PAY 117 

    0x84b94734,// 121 PAY 118 

    0x3679dc29,// 122 PAY 119 

    0x91b89800,// 123 PAY 120 

    0x66ed7d79,// 124 PAY 121 

    0x68d2bfb6,// 125 PAY 122 

    0x9f8ab1dd,// 126 PAY 123 

    0x83a2c170,// 127 PAY 124 

    0xacb872b7,// 128 PAY 125 

    0x46bdd9a0,// 129 PAY 126 

    0xe1950c67,// 130 PAY 127 

    0x69e99743,// 131 PAY 128 

    0xa9c55ce5,// 132 PAY 129 

    0xe36881c8,// 133 PAY 130 

    0x3c238558,// 134 PAY 131 

    0x720bde03,// 135 PAY 132 

    0x5db5578c,// 136 PAY 133 

    0x71dc0bbb,// 137 PAY 134 

    0x207f971f,// 138 PAY 135 

    0x810c51b1,// 139 PAY 136 

    0x97963451,// 140 PAY 137 

    0xcf20e181,// 141 PAY 138 

    0xa70ff795,// 142 PAY 139 

    0x03adb374,// 143 PAY 140 

    0x7ef840bb,// 144 PAY 141 

    0x958772c9,// 145 PAY 142 

    0x2e6ec2a4,// 146 PAY 143 

    0xe70ae758,// 147 PAY 144 

    0x05b10736,// 148 PAY 145 

    0x6d59d136,// 149 PAY 146 

    0x4d5cc948,// 150 PAY 147 

    0xb566aba2,// 151 PAY 148 

    0xc65c9b22,// 152 PAY 149 

    0xceb09612,// 153 PAY 150 

    0x20600395,// 154 PAY 151 

    0xe37870cc,// 155 PAY 152 

    0xe60eb481,// 156 PAY 153 

    0x7395b4c3,// 157 PAY 154 

    0x760be11f,// 158 PAY 155 

    0x99ccd646,// 159 PAY 156 

    0x78667297,// 160 PAY 157 

    0x5ebf2132,// 161 PAY 158 

    0x2092bbea,// 162 PAY 159 

    0x0325167e,// 163 PAY 160 

    0xd5dfd0ad,// 164 PAY 161 

    0x3359d6a5,// 165 PAY 162 

    0xbf4533a8,// 166 PAY 163 

    0x421b006d,// 167 PAY 164 

    0xccbd4598,// 168 PAY 165 

    0x35f267ca,// 169 PAY 166 

/// HASH is  16 bytes 

    0x66ed7d79,// 170 HSH   1 

    0x68d2bfb6,// 171 HSH   2 

    0x9f8ab1dd,// 172 HSH   3 

    0x83a2c170,// 173 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 233 

/// STA pkt_idx        : 239 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x03bc5be9 // 174 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt87_tmpl[] = {
    0x08010068,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 144 words. 

/// BDA size     is 571 (0x23b) 

/// BDA id       is 0x316c 

    0x023b316c,// 3 BDA   1 

/// PAY Generic Data size   : 571 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xdba36cba,// 4 PAY   1 

    0x42723184,// 5 PAY   2 

    0xf1a48290,// 6 PAY   3 

    0x2c1b2b20,// 7 PAY   4 

    0xd18691c5,// 8 PAY   5 

    0xdece49e9,// 9 PAY   6 

    0xc3422681,// 10 PAY   7 

    0x32783261,// 11 PAY   8 

    0xbea3277d,// 12 PAY   9 

    0xdeb6b2c7,// 13 PAY  10 

    0x6f4c5e55,// 14 PAY  11 

    0x68951e0a,// 15 PAY  12 

    0x48395596,// 16 PAY  13 

    0x62aed2dd,// 17 PAY  14 

    0xd378e3fa,// 18 PAY  15 

    0x262bf77f,// 19 PAY  16 

    0xa6a941e9,// 20 PAY  17 

    0x3ac3fad5,// 21 PAY  18 

    0xbab35004,// 22 PAY  19 

    0xa62102bb,// 23 PAY  20 

    0x5ed8556d,// 24 PAY  21 

    0xeb6e7c3e,// 25 PAY  22 

    0xf50246f9,// 26 PAY  23 

    0xdd620140,// 27 PAY  24 

    0x7da0abf3,// 28 PAY  25 

    0xffae2969,// 29 PAY  26 

    0x58f5277b,// 30 PAY  27 

    0x33df8f7d,// 31 PAY  28 

    0xe930ddfa,// 32 PAY  29 

    0x269134da,// 33 PAY  30 

    0xaa33fda7,// 34 PAY  31 

    0xf662d658,// 35 PAY  32 

    0x33c2bc7e,// 36 PAY  33 

    0xeac37dad,// 37 PAY  34 

    0xf832b395,// 38 PAY  35 

    0x8b5bd124,// 39 PAY  36 

    0xcc40202b,// 40 PAY  37 

    0xe2871ca3,// 41 PAY  38 

    0xb152e10f,// 42 PAY  39 

    0x42532ae8,// 43 PAY  40 

    0xf95d94e2,// 44 PAY  41 

    0xc3cbce1a,// 45 PAY  42 

    0x58581f66,// 46 PAY  43 

    0xb70a7879,// 47 PAY  44 

    0x3d2e96d9,// 48 PAY  45 

    0x7e28ba7b,// 49 PAY  46 

    0x18c0bd63,// 50 PAY  47 

    0x4ce04a2b,// 51 PAY  48 

    0x40d72aac,// 52 PAY  49 

    0x08df5918,// 53 PAY  50 

    0x8c0f1934,// 54 PAY  51 

    0x662558bf,// 55 PAY  52 

    0x65b235b5,// 56 PAY  53 

    0x84a9d25c,// 57 PAY  54 

    0x1a338fec,// 58 PAY  55 

    0x958d513b,// 59 PAY  56 

    0xfce8cb4b,// 60 PAY  57 

    0xc9b5345f,// 61 PAY  58 

    0x339aa018,// 62 PAY  59 

    0xfdcbb903,// 63 PAY  60 

    0xaa28bf80,// 64 PAY  61 

    0x45ee8a34,// 65 PAY  62 

    0x4f53e596,// 66 PAY  63 

    0xb21a90ef,// 67 PAY  64 

    0x55084eb4,// 68 PAY  65 

    0x0ae26da4,// 69 PAY  66 

    0x2f726b91,// 70 PAY  67 

    0x35557df7,// 71 PAY  68 

    0xf9ae74a4,// 72 PAY  69 

    0xdd2c547c,// 73 PAY  70 

    0x38907a55,// 74 PAY  71 

    0x4b9a9574,// 75 PAY  72 

    0xde714c67,// 76 PAY  73 

    0x44f84476,// 77 PAY  74 

    0x3556e3b9,// 78 PAY  75 

    0x6274ed8e,// 79 PAY  76 

    0x942014fb,// 80 PAY  77 

    0x0b7acd71,// 81 PAY  78 

    0xf94ab886,// 82 PAY  79 

    0x6231b826,// 83 PAY  80 

    0xcf932371,// 84 PAY  81 

    0x64d5cc49,// 85 PAY  82 

    0xbf10a5c0,// 86 PAY  83 

    0xec07323f,// 87 PAY  84 

    0x93eab135,// 88 PAY  85 

    0x4502d902,// 89 PAY  86 

    0x8538e8db,// 90 PAY  87 

    0xb3446883,// 91 PAY  88 

    0x436fbb21,// 92 PAY  89 

    0x17f98396,// 93 PAY  90 

    0x0ec66fb6,// 94 PAY  91 

    0xcf41a431,// 95 PAY  92 

    0xa25c527e,// 96 PAY  93 

    0x83ffd6f9,// 97 PAY  94 

    0x4d607e4a,// 98 PAY  95 

    0x5973d369,// 99 PAY  96 

    0xb1b3be0c,// 100 PAY  97 

    0x3d70dfde,// 101 PAY  98 

    0x56529b3b,// 102 PAY  99 

    0x0617606d,// 103 PAY 100 

    0xe13b7f4b,// 104 PAY 101 

    0x900b8bdf,// 105 PAY 102 

    0x452656bc,// 106 PAY 103 

    0x6665ec4d,// 107 PAY 104 

    0x3f1c6020,// 108 PAY 105 

    0xe6009115,// 109 PAY 106 

    0x1d0ad7af,// 110 PAY 107 

    0xb1581736,// 111 PAY 108 

    0x7c4c7cd7,// 112 PAY 109 

    0x46d0f9bc,// 113 PAY 110 

    0x1e292d8e,// 114 PAY 111 

    0x22698704,// 115 PAY 112 

    0x33bc03e5,// 116 PAY 113 

    0x01643f17,// 117 PAY 114 

    0x8d3f2bc5,// 118 PAY 115 

    0x3c28575e,// 119 PAY 116 

    0xe970499c,// 120 PAY 117 

    0x8b987e8c,// 121 PAY 118 

    0x3c143458,// 122 PAY 119 

    0x8efd7eee,// 123 PAY 120 

    0x850db779,// 124 PAY 121 

    0xe92ef1c8,// 125 PAY 122 

    0x65cc8b00,// 126 PAY 123 

    0xf71e061c,// 127 PAY 124 

    0xd096234b,// 128 PAY 125 

    0xc3497aab,// 129 PAY 126 

    0x9f501349,// 130 PAY 127 

    0x2f83f6f4,// 131 PAY 128 

    0x459d74ff,// 132 PAY 129 

    0xd6c45f68,// 133 PAY 130 

    0x788b644f,// 134 PAY 131 

    0x79c807ab,// 135 PAY 132 

    0x1194340e,// 136 PAY 133 

    0x4d4eed9a,// 137 PAY 134 

    0xfba0c222,// 138 PAY 135 

    0x28411122,// 139 PAY 136 

    0x975796dd,// 140 PAY 137 

    0x8c64f39b,// 141 PAY 138 

    0x1243ddc0,// 142 PAY 139 

    0x5b09b7f6,// 143 PAY 140 

    0xc6c7d6f9,// 144 PAY 141 

    0x2204cbd7,// 145 PAY 142 

    0x94563500,// 146 PAY 143 

/// STA is 1 words. 

/// STA num_pkts       : 189 

/// STA pkt_idx        : 136 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcb 

    0x0220cbbd // 147 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt88_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 220 words. 

/// BDA size     is 876 (0x36c) 

/// BDA id       is 0x441 

    0x036c0441,// 3 BDA   1 

/// PAY Generic Data size   : 876 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x7acebea8,// 4 PAY   1 

    0xd5514e31,// 5 PAY   2 

    0x26156985,// 6 PAY   3 

    0xa8d6e61e,// 7 PAY   4 

    0xef3722c4,// 8 PAY   5 

    0x550b5465,// 9 PAY   6 

    0xdccf218a,// 10 PAY   7 

    0x65687369,// 11 PAY   8 

    0xaf7a77cb,// 12 PAY   9 

    0x6eaeb821,// 13 PAY  10 

    0xe1419600,// 14 PAY  11 

    0x77818520,// 15 PAY  12 

    0x54589006,// 16 PAY  13 

    0xbe855685,// 17 PAY  14 

    0x72174cea,// 18 PAY  15 

    0x40106c78,// 19 PAY  16 

    0xa71d5920,// 20 PAY  17 

    0x6219fa5d,// 21 PAY  18 

    0xc3ff9a52,// 22 PAY  19 

    0x47a1ab79,// 23 PAY  20 

    0x8c61f767,// 24 PAY  21 

    0xaa5c9f8d,// 25 PAY  22 

    0xaa28c41c,// 26 PAY  23 

    0xb42b6b8d,// 27 PAY  24 

    0xf4dfc616,// 28 PAY  25 

    0xed272b10,// 29 PAY  26 

    0xa9d7cef4,// 30 PAY  27 

    0xb44629ba,// 31 PAY  28 

    0x26fab90e,// 32 PAY  29 

    0xeb083f04,// 33 PAY  30 

    0xa85f552d,// 34 PAY  31 

    0x62ee6612,// 35 PAY  32 

    0x9fcc0241,// 36 PAY  33 

    0x9e0b47b9,// 37 PAY  34 

    0x5783fa32,// 38 PAY  35 

    0xaf51b14d,// 39 PAY  36 

    0x7c8188d7,// 40 PAY  37 

    0x627778fe,// 41 PAY  38 

    0x4d8e4a23,// 42 PAY  39 

    0x462366ae,// 43 PAY  40 

    0xde3a4241,// 44 PAY  41 

    0xad8f84cf,// 45 PAY  42 

    0x28419ba7,// 46 PAY  43 

    0xbd90b3ab,// 47 PAY  44 

    0xbbd31cb9,// 48 PAY  45 

    0x0bd46739,// 49 PAY  46 

    0x4d0e3d8f,// 50 PAY  47 

    0x4c04c2c9,// 51 PAY  48 

    0x7f0f57b3,// 52 PAY  49 

    0xf1eba365,// 53 PAY  50 

    0xd9ecc4c4,// 54 PAY  51 

    0x13214ab6,// 55 PAY  52 

    0x053f5295,// 56 PAY  53 

    0x7e0b5266,// 57 PAY  54 

    0x814b8af5,// 58 PAY  55 

    0x8d99aea5,// 59 PAY  56 

    0x079d70e2,// 60 PAY  57 

    0xb4a47eec,// 61 PAY  58 

    0xe6dcc29d,// 62 PAY  59 

    0x49a2f6e0,// 63 PAY  60 

    0x4cecb755,// 64 PAY  61 

    0x5d24a6b2,// 65 PAY  62 

    0x7ff63e55,// 66 PAY  63 

    0x5b96ce1d,// 67 PAY  64 

    0x05644b36,// 68 PAY  65 

    0x4e617815,// 69 PAY  66 

    0x4c8c74f7,// 70 PAY  67 

    0x486f0620,// 71 PAY  68 

    0xc2f1195c,// 72 PAY  69 

    0xd38630a9,// 73 PAY  70 

    0xb324c6f4,// 74 PAY  71 

    0xd321de24,// 75 PAY  72 

    0x6af20b2c,// 76 PAY  73 

    0x33fb0445,// 77 PAY  74 

    0x4090b8f9,// 78 PAY  75 

    0x4966bd8f,// 79 PAY  76 

    0x66ec6066,// 80 PAY  77 

    0x8c813d9c,// 81 PAY  78 

    0x4ddc208d,// 82 PAY  79 

    0xadaddc71,// 83 PAY  80 

    0xb7486068,// 84 PAY  81 

    0x2b39b759,// 85 PAY  82 

    0xa79d3f4e,// 86 PAY  83 

    0x8a4e6c6b,// 87 PAY  84 

    0xfca783a9,// 88 PAY  85 

    0x86689537,// 89 PAY  86 

    0x16a66ad1,// 90 PAY  87 

    0x7dde61c7,// 91 PAY  88 

    0x2ac6da34,// 92 PAY  89 

    0x7feded06,// 93 PAY  90 

    0xabdb0432,// 94 PAY  91 

    0x576b9f24,// 95 PAY  92 

    0x90bda9e4,// 96 PAY  93 

    0xf9333192,// 97 PAY  94 

    0xf6d7a59b,// 98 PAY  95 

    0x53ffceb6,// 99 PAY  96 

    0x9a87aa0b,// 100 PAY  97 

    0xd0fdb4c8,// 101 PAY  98 

    0xdbdbee72,// 102 PAY  99 

    0xe5743de8,// 103 PAY 100 

    0xe7dee7e5,// 104 PAY 101 

    0xfcaafc31,// 105 PAY 102 

    0x5abb9115,// 106 PAY 103 

    0x3f374a80,// 107 PAY 104 

    0x0f979448,// 108 PAY 105 

    0xc060e38f,// 109 PAY 106 

    0x650a55a7,// 110 PAY 107 

    0x0ab13160,// 111 PAY 108 

    0x89ac0f73,// 112 PAY 109 

    0x94a1d94a,// 113 PAY 110 

    0x8a2a29aa,// 114 PAY 111 

    0x5b5b9adc,// 115 PAY 112 

    0xe0df7b23,// 116 PAY 113 

    0x164e29b2,// 117 PAY 114 

    0xc197440b,// 118 PAY 115 

    0x8738efe3,// 119 PAY 116 

    0x5d92070a,// 120 PAY 117 

    0x0dc228f7,// 121 PAY 118 

    0x037dc16c,// 122 PAY 119 

    0x6f46223e,// 123 PAY 120 

    0x77cc75e6,// 124 PAY 121 

    0x13778352,// 125 PAY 122 

    0x61806c5f,// 126 PAY 123 

    0x9522e43d,// 127 PAY 124 

    0x8570fc2e,// 128 PAY 125 

    0x54ac05eb,// 129 PAY 126 

    0x295848bd,// 130 PAY 127 

    0x5d1b5d53,// 131 PAY 128 

    0x5383bc6e,// 132 PAY 129 

    0xf5f9127a,// 133 PAY 130 

    0x87f484b7,// 134 PAY 131 

    0x14895a8c,// 135 PAY 132 

    0x4014fcd8,// 136 PAY 133 

    0x2610e049,// 137 PAY 134 

    0x0d835e92,// 138 PAY 135 

    0xd4e933d8,// 139 PAY 136 

    0x5f493eb7,// 140 PAY 137 

    0xd0904639,// 141 PAY 138 

    0xdd168893,// 142 PAY 139 

    0x6c70ee91,// 143 PAY 140 

    0x07010725,// 144 PAY 141 

    0x73b7bcd2,// 145 PAY 142 

    0x60e60f0a,// 146 PAY 143 

    0x7edbaeb2,// 147 PAY 144 

    0x55a40964,// 148 PAY 145 

    0x21ac6029,// 149 PAY 146 

    0xddf1ea51,// 150 PAY 147 

    0xb2c9188f,// 151 PAY 148 

    0xfd0c9f87,// 152 PAY 149 

    0x2700656b,// 153 PAY 150 

    0x1a43f0e5,// 154 PAY 151 

    0x996a6077,// 155 PAY 152 

    0x7283fec9,// 156 PAY 153 

    0x0fed4629,// 157 PAY 154 

    0x86094ad0,// 158 PAY 155 

    0x44f486f5,// 159 PAY 156 

    0xf3ebfe45,// 160 PAY 157 

    0xa7dd46f8,// 161 PAY 158 

    0xb065f92f,// 162 PAY 159 

    0xb815294a,// 163 PAY 160 

    0xbaa58bda,// 164 PAY 161 

    0x9c79e309,// 165 PAY 162 

    0x398121e6,// 166 PAY 163 

    0xdf0536d8,// 167 PAY 164 

    0x02f56633,// 168 PAY 165 

    0x6d66c3f1,// 169 PAY 166 

    0xd76b3e03,// 170 PAY 167 

    0x3a263023,// 171 PAY 168 

    0x5c0a8de0,// 172 PAY 169 

    0x52b0f583,// 173 PAY 170 

    0x9f74ecd3,// 174 PAY 171 

    0xf9b9f110,// 175 PAY 172 

    0x7e729762,// 176 PAY 173 

    0x32abba1d,// 177 PAY 174 

    0x88c4fd28,// 178 PAY 175 

    0x443f83bb,// 179 PAY 176 

    0x36c41cfe,// 180 PAY 177 

    0xf77dcbe8,// 181 PAY 178 

    0x9a540a09,// 182 PAY 179 

    0xd5daac94,// 183 PAY 180 

    0xdec74db7,// 184 PAY 181 

    0x9eb5fa3d,// 185 PAY 182 

    0xe919827b,// 186 PAY 183 

    0x6996c415,// 187 PAY 184 

    0x06820d66,// 188 PAY 185 

    0x7804a0a2,// 189 PAY 186 

    0x23c3a939,// 190 PAY 187 

    0x821253e6,// 191 PAY 188 

    0xedf15de9,// 192 PAY 189 

    0x0b3b432a,// 193 PAY 190 

    0xa431f51e,// 194 PAY 191 

    0xe4e7ee56,// 195 PAY 192 

    0xc5863294,// 196 PAY 193 

    0x4c50098d,// 197 PAY 194 

    0x224d9eea,// 198 PAY 195 

    0xc60175df,// 199 PAY 196 

    0x5ded2263,// 200 PAY 197 

    0x95e6cc5d,// 201 PAY 198 

    0x9de262a3,// 202 PAY 199 

    0x5d2a05f4,// 203 PAY 200 

    0x1491e04c,// 204 PAY 201 

    0xbe744de4,// 205 PAY 202 

    0xb8dd0714,// 206 PAY 203 

    0xc3ae680d,// 207 PAY 204 

    0xe08fc7b7,// 208 PAY 205 

    0x1675b4eb,// 209 PAY 206 

    0x9e501dc5,// 210 PAY 207 

    0xf1564a30,// 211 PAY 208 

    0xe5fb5cfd,// 212 PAY 209 

    0xa35cdb57,// 213 PAY 210 

    0xdd90001c,// 214 PAY 211 

    0x47324830,// 215 PAY 212 

    0x983da4ad,// 216 PAY 213 

    0xc3ec4b5b,// 217 PAY 214 

    0xb1d99d62,// 218 PAY 215 

    0x5f9099b8,// 219 PAY 216 

    0x499483d3,// 220 PAY 217 

    0x268180b4,// 221 PAY 218 

    0x5022c473,// 222 PAY 219 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf1 

    0x0248f17e // 223 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 96 (0x60) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt89_tmpl[] = {
    0x0c010060,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 151 words. 

/// BDA size     is 598 (0x256) 

/// BDA id       is 0x8e30 

    0x02568e30,// 3 BDA   1 

/// PAY Generic Data size   : 598 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe146635f,// 4 PAY   1 

    0x89d2da83,// 5 PAY   2 

    0x1f6dba17,// 6 PAY   3 

    0xe86c2b07,// 7 PAY   4 

    0x10c0fe65,// 8 PAY   5 

    0x8b34e1ca,// 9 PAY   6 

    0x0df221e4,// 10 PAY   7 

    0x33fb03d5,// 11 PAY   8 

    0x1b51fc89,// 12 PAY   9 

    0xec072aeb,// 13 PAY  10 

    0x90b039d5,// 14 PAY  11 

    0xb3954bfb,// 15 PAY  12 

    0x8dd28ec7,// 16 PAY  13 

    0x1ff7ced1,// 17 PAY  14 

    0x1ee7888a,// 18 PAY  15 

    0xd29f717a,// 19 PAY  16 

    0x063f512e,// 20 PAY  17 

    0xc0de4218,// 21 PAY  18 

    0x780dae5a,// 22 PAY  19 

    0x2450aadb,// 23 PAY  20 

    0xebf2c28c,// 24 PAY  21 

    0x499eeb91,// 25 PAY  22 

    0x68bf1f2f,// 26 PAY  23 

    0x97e8ef3e,// 27 PAY  24 

    0x7b31de36,// 28 PAY  25 

    0xa68e54aa,// 29 PAY  26 

    0x1e436caf,// 30 PAY  27 

    0xe8866f52,// 31 PAY  28 

    0x75a6638c,// 32 PAY  29 

    0x6d2eb596,// 33 PAY  30 

    0x6b12e1e1,// 34 PAY  31 

    0x334280fc,// 35 PAY  32 

    0x120edeff,// 36 PAY  33 

    0x21a7a5d1,// 37 PAY  34 

    0x134ee840,// 38 PAY  35 

    0x3e333d85,// 39 PAY  36 

    0x0ba6865f,// 40 PAY  37 

    0xccbd015f,// 41 PAY  38 

    0x97c77712,// 42 PAY  39 

    0x89e61caa,// 43 PAY  40 

    0x8a978b5e,// 44 PAY  41 

    0x571825aa,// 45 PAY  42 

    0x197a4240,// 46 PAY  43 

    0xef85175a,// 47 PAY  44 

    0xcc41ec6e,// 48 PAY  45 

    0x22bd4783,// 49 PAY  46 

    0x53555b98,// 50 PAY  47 

    0x47fcd24d,// 51 PAY  48 

    0x9d3f0630,// 52 PAY  49 

    0x02dce6d8,// 53 PAY  50 

    0xdfdfdef0,// 54 PAY  51 

    0x4166b5a4,// 55 PAY  52 

    0x05a43c5b,// 56 PAY  53 

    0x7f64a1d3,// 57 PAY  54 

    0x2ea6205a,// 58 PAY  55 

    0x3dfc156f,// 59 PAY  56 

    0xf7fbba5e,// 60 PAY  57 

    0xd38d8990,// 61 PAY  58 

    0xd17981e4,// 62 PAY  59 

    0xa27b794d,// 63 PAY  60 

    0x4676f912,// 64 PAY  61 

    0x42cb9c40,// 65 PAY  62 

    0x4772b27e,// 66 PAY  63 

    0x8bfea91e,// 67 PAY  64 

    0xf55f1724,// 68 PAY  65 

    0x8b9bbe96,// 69 PAY  66 

    0x8562c330,// 70 PAY  67 

    0x8262ef5a,// 71 PAY  68 

    0x91d84f48,// 72 PAY  69 

    0xbe18e47d,// 73 PAY  70 

    0xe36f344e,// 74 PAY  71 

    0xe655866b,// 75 PAY  72 

    0xfb2fde75,// 76 PAY  73 

    0x62abcc65,// 77 PAY  74 

    0x0da6ced8,// 78 PAY  75 

    0x571998c7,// 79 PAY  76 

    0xa6267136,// 80 PAY  77 

    0x426a1ee4,// 81 PAY  78 

    0x258790aa,// 82 PAY  79 

    0x9b668ec1,// 83 PAY  80 

    0xef2407c4,// 84 PAY  81 

    0x424cd6bf,// 85 PAY  82 

    0x1db1feb3,// 86 PAY  83 

    0x820689a2,// 87 PAY  84 

    0x6dcee281,// 88 PAY  85 

    0x260c2207,// 89 PAY  86 

    0x75960025,// 90 PAY  87 

    0xa1777cc2,// 91 PAY  88 

    0x154392d5,// 92 PAY  89 

    0x29458cf7,// 93 PAY  90 

    0x83df5766,// 94 PAY  91 

    0x86a083b6,// 95 PAY  92 

    0xdfc697b8,// 96 PAY  93 

    0x903d06e6,// 97 PAY  94 

    0x689ae8c3,// 98 PAY  95 

    0xf59ba270,// 99 PAY  96 

    0xd1df899e,// 100 PAY  97 

    0x3b3ec44f,// 101 PAY  98 

    0xeb9f7ac9,// 102 PAY  99 

    0x4b6823ce,// 103 PAY 100 

    0xbe3c9ab6,// 104 PAY 101 

    0xa330ca22,// 105 PAY 102 

    0xacd6800e,// 106 PAY 103 

    0x92823421,// 107 PAY 104 

    0x576165bf,// 108 PAY 105 

    0xb7ba9853,// 109 PAY 106 

    0xe2d8f521,// 110 PAY 107 

    0x571e4c47,// 111 PAY 108 

    0x39e00c39,// 112 PAY 109 

    0x774f258b,// 113 PAY 110 

    0xca81a2c9,// 114 PAY 111 

    0x03f73255,// 115 PAY 112 

    0x5b8cdfba,// 116 PAY 113 

    0x8c09a786,// 117 PAY 114 

    0x0a5f91da,// 118 PAY 115 

    0x7ed251ab,// 119 PAY 116 

    0x73caa0af,// 120 PAY 117 

    0xb4030c5a,// 121 PAY 118 

    0x8c67282b,// 122 PAY 119 

    0xafcfcee9,// 123 PAY 120 

    0x06fa06c8,// 124 PAY 121 

    0x12d544df,// 125 PAY 122 

    0x4f1b11e7,// 126 PAY 123 

    0xc8049043,// 127 PAY 124 

    0xe790659b,// 128 PAY 125 

    0x1fc86d77,// 129 PAY 126 

    0xecb3d902,// 130 PAY 127 

    0xa65fe930,// 131 PAY 128 

    0x6df8be46,// 132 PAY 129 

    0x45fe91bd,// 133 PAY 130 

    0xe9cd6111,// 134 PAY 131 

    0x9d745a77,// 135 PAY 132 

    0x32bf0553,// 136 PAY 133 

    0x0eb7c3d9,// 137 PAY 134 

    0x13134e98,// 138 PAY 135 

    0x5cdfa052,// 139 PAY 136 

    0x039ef2b5,// 140 PAY 137 

    0x71a50376,// 141 PAY 138 

    0xa417a849,// 142 PAY 139 

    0xe3e586b0,// 143 PAY 140 

    0x97b3e520,// 144 PAY 141 

    0xa1541371,// 145 PAY 142 

    0xa92f0e5d,// 146 PAY 143 

    0xbfeb1183,// 147 PAY 144 

    0x54db508c,// 148 PAY 145 

    0xf97a3e6e,// 149 PAY 146 

    0x41b866d4,// 150 PAY 147 

    0xe170b017,// 151 PAY 148 

    0x8ea8cd41,// 152 PAY 149 

    0xb3a30000,// 153 PAY 150 

/// HASH is  4 bytes 

    0x4b6823ce,// 154 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 49 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x0188c131 // 155 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt90_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 334 words. 

/// BDA size     is 1332 (0x534) 

/// BDA id       is 0xa336 

    0x0534a336,// 3 BDA   1 

/// PAY Generic Data size   : 1332 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x8ec31b5b,// 4 PAY   1 

    0xb460a486,// 5 PAY   2 

    0xdbe3abeb,// 6 PAY   3 

    0x8e014cdf,// 7 PAY   4 

    0x92147cb4,// 8 PAY   5 

    0x31dd6ac5,// 9 PAY   6 

    0xd41620da,// 10 PAY   7 

    0x8a320268,// 11 PAY   8 

    0x6682606a,// 12 PAY   9 

    0x2f9a86a0,// 13 PAY  10 

    0x2e0aee87,// 14 PAY  11 

    0x06125246,// 15 PAY  12 

    0x807b24a4,// 16 PAY  13 

    0x66fef024,// 17 PAY  14 

    0xf4774c58,// 18 PAY  15 

    0x83b5dcb5,// 19 PAY  16 

    0x0d9bd9d9,// 20 PAY  17 

    0xf7109d37,// 21 PAY  18 

    0x43f4c4f1,// 22 PAY  19 

    0x46e042fd,// 23 PAY  20 

    0x4d481813,// 24 PAY  21 

    0x23492d1c,// 25 PAY  22 

    0x7c387d2b,// 26 PAY  23 

    0x0275c669,// 27 PAY  24 

    0x5f787574,// 28 PAY  25 

    0x659e5b13,// 29 PAY  26 

    0x4fb5a507,// 30 PAY  27 

    0x5a6da792,// 31 PAY  28 

    0xbb8aeeea,// 32 PAY  29 

    0x73ef8c89,// 33 PAY  30 

    0x12d399f5,// 34 PAY  31 

    0x4b9788b6,// 35 PAY  32 

    0x92217964,// 36 PAY  33 

    0x1c30b3cd,// 37 PAY  34 

    0xd429ed8b,// 38 PAY  35 

    0x29e64db0,// 39 PAY  36 

    0xad21b871,// 40 PAY  37 

    0x5711f47e,// 41 PAY  38 

    0x82da0b5f,// 42 PAY  39 

    0xab40f67a,// 43 PAY  40 

    0x1dc9a2f2,// 44 PAY  41 

    0x410dd110,// 45 PAY  42 

    0x7106aed9,// 46 PAY  43 

    0xb4726d2d,// 47 PAY  44 

    0x03c47673,// 48 PAY  45 

    0xc9cc59a4,// 49 PAY  46 

    0xa773192f,// 50 PAY  47 

    0x54000da1,// 51 PAY  48 

    0x21c33560,// 52 PAY  49 

    0xbeaae612,// 53 PAY  50 

    0x241f2b64,// 54 PAY  51 

    0xbcd5b530,// 55 PAY  52 

    0x40d024af,// 56 PAY  53 

    0x82ef952c,// 57 PAY  54 

    0x876816a5,// 58 PAY  55 

    0x1b5035b8,// 59 PAY  56 

    0xb633f100,// 60 PAY  57 

    0x825750db,// 61 PAY  58 

    0x5c67c9e0,// 62 PAY  59 

    0xc6b58bae,// 63 PAY  60 

    0xe61e10d6,// 64 PAY  61 

    0xe4252504,// 65 PAY  62 

    0x08c9b451,// 66 PAY  63 

    0x9f9b0444,// 67 PAY  64 

    0xcbd0ca6e,// 68 PAY  65 

    0x8bae36a8,// 69 PAY  66 

    0x19d6103b,// 70 PAY  67 

    0x825d52e6,// 71 PAY  68 

    0x77777132,// 72 PAY  69 

    0x0373717a,// 73 PAY  70 

    0xf12ea05f,// 74 PAY  71 

    0xd676c5a6,// 75 PAY  72 

    0x933bff6f,// 76 PAY  73 

    0x372a5f96,// 77 PAY  74 

    0x4e2507a8,// 78 PAY  75 

    0x26b7d0e5,// 79 PAY  76 

    0x007bf4b0,// 80 PAY  77 

    0x88060984,// 81 PAY  78 

    0x1fe234c2,// 82 PAY  79 

    0xb54c8218,// 83 PAY  80 

    0x4817a3f3,// 84 PAY  81 

    0x4d7cf9d7,// 85 PAY  82 

    0xd19b505a,// 86 PAY  83 

    0x4e083226,// 87 PAY  84 

    0xc7ffbaee,// 88 PAY  85 

    0x427d9d42,// 89 PAY  86 

    0xa785aa3a,// 90 PAY  87 

    0xcbf919bd,// 91 PAY  88 

    0x6c363f17,// 92 PAY  89 

    0x839964b0,// 93 PAY  90 

    0x28043696,// 94 PAY  91 

    0x46fa283c,// 95 PAY  92 

    0x81ea01ac,// 96 PAY  93 

    0x82c9cf23,// 97 PAY  94 

    0xba0180a0,// 98 PAY  95 

    0x19cc0776,// 99 PAY  96 

    0x5efbbb59,// 100 PAY  97 

    0x2bad6b74,// 101 PAY  98 

    0x1b763595,// 102 PAY  99 

    0x9b0fbe37,// 103 PAY 100 

    0x42b9201b,// 104 PAY 101 

    0x2b1d911d,// 105 PAY 102 

    0xdc529861,// 106 PAY 103 

    0xaa06e1b8,// 107 PAY 104 

    0xf24c060a,// 108 PAY 105 

    0x6719698a,// 109 PAY 106 

    0xff407107,// 110 PAY 107 

    0x9dd8909d,// 111 PAY 108 

    0x446207bf,// 112 PAY 109 

    0x8d456eb2,// 113 PAY 110 

    0xd96a0d2e,// 114 PAY 111 

    0xf70639f2,// 115 PAY 112 

    0x47c62039,// 116 PAY 113 

    0x1992439d,// 117 PAY 114 

    0xb2593ef2,// 118 PAY 115 

    0x072afc7a,// 119 PAY 116 

    0x8eb6f6f6,// 120 PAY 117 

    0xbde34487,// 121 PAY 118 

    0xe3d778ad,// 122 PAY 119 

    0xaadc4182,// 123 PAY 120 

    0x64208ec0,// 124 PAY 121 

    0xe6288f63,// 125 PAY 122 

    0x752b92bd,// 126 PAY 123 

    0x27a83344,// 127 PAY 124 

    0xc50cea09,// 128 PAY 125 

    0xa8b1f0a5,// 129 PAY 126 

    0x7bc893d6,// 130 PAY 127 

    0x5ccd4f54,// 131 PAY 128 

    0xddfc1939,// 132 PAY 129 

    0x270df9f1,// 133 PAY 130 

    0x31cc05ec,// 134 PAY 131 

    0xee887989,// 135 PAY 132 

    0xdc36ab62,// 136 PAY 133 

    0x8662ec03,// 137 PAY 134 

    0x1dfac0ae,// 138 PAY 135 

    0x53103fd1,// 139 PAY 136 

    0x1c2d57d6,// 140 PAY 137 

    0xb5bc72e9,// 141 PAY 138 

    0xbe4e0b9f,// 142 PAY 139 

    0xb90f1550,// 143 PAY 140 

    0x542298bb,// 144 PAY 141 

    0x1c5a004f,// 145 PAY 142 

    0x18c14d7a,// 146 PAY 143 

    0xc76383a6,// 147 PAY 144 

    0x6c5d846d,// 148 PAY 145 

    0xd6cd5e2b,// 149 PAY 146 

    0xf52a36a9,// 150 PAY 147 

    0x605e185f,// 151 PAY 148 

    0x79db21eb,// 152 PAY 149 

    0x3ddab6ef,// 153 PAY 150 

    0xc0843ce0,// 154 PAY 151 

    0x87e53105,// 155 PAY 152 

    0x21624c40,// 156 PAY 153 

    0xf710041a,// 157 PAY 154 

    0x9177028c,// 158 PAY 155 

    0xe42a727b,// 159 PAY 156 

    0x83c0de50,// 160 PAY 157 

    0x91c6e614,// 161 PAY 158 

    0x2d92cbd6,// 162 PAY 159 

    0x1340e93c,// 163 PAY 160 

    0xebe66d7c,// 164 PAY 161 

    0x3c696f50,// 165 PAY 162 

    0x15a777ac,// 166 PAY 163 

    0xfd305f54,// 167 PAY 164 

    0x2372dba0,// 168 PAY 165 

    0xa8ca1363,// 169 PAY 166 

    0x2445dfe2,// 170 PAY 167 

    0x212179da,// 171 PAY 168 

    0x4d9b9323,// 172 PAY 169 

    0x4cd80ab3,// 173 PAY 170 

    0x1a4e57ef,// 174 PAY 171 

    0x82d759b7,// 175 PAY 172 

    0x506ca5f1,// 176 PAY 173 

    0x488cd545,// 177 PAY 174 

    0xabd8a715,// 178 PAY 175 

    0x71934622,// 179 PAY 176 

    0x2bf00e25,// 180 PAY 177 

    0x5ea1e37f,// 181 PAY 178 

    0x6522b4d0,// 182 PAY 179 

    0xba28c7ae,// 183 PAY 180 

    0x81167d26,// 184 PAY 181 

    0xc9bd9e1a,// 185 PAY 182 

    0x2ff1d0d3,// 186 PAY 183 

    0x4d662b6f,// 187 PAY 184 

    0xa2bcd1cd,// 188 PAY 185 

    0xd8daf1c6,// 189 PAY 186 

    0xf708f510,// 190 PAY 187 

    0xdea01ffa,// 191 PAY 188 

    0x8dd99a37,// 192 PAY 189 

    0xd6ae9ba3,// 193 PAY 190 

    0x726dfae7,// 194 PAY 191 

    0x76d9e8a5,// 195 PAY 192 

    0x5a182c82,// 196 PAY 193 

    0x51f8bcea,// 197 PAY 194 

    0x073ae0cf,// 198 PAY 195 

    0x4677865e,// 199 PAY 196 

    0x22447e24,// 200 PAY 197 

    0x536fe4a4,// 201 PAY 198 

    0x29381ce7,// 202 PAY 199 

    0x420b7168,// 203 PAY 200 

    0xcab54187,// 204 PAY 201 

    0x7b0940af,// 205 PAY 202 

    0x3e2939e3,// 206 PAY 203 

    0x171d9fef,// 207 PAY 204 

    0x0f495444,// 208 PAY 205 

    0xaafa61d7,// 209 PAY 206 

    0x383611e4,// 210 PAY 207 

    0xeae968f6,// 211 PAY 208 

    0xe65d1bce,// 212 PAY 209 

    0x4365b807,// 213 PAY 210 

    0x5259ba0d,// 214 PAY 211 

    0x9e1b46a2,// 215 PAY 212 

    0x70240690,// 216 PAY 213 

    0xa47cda81,// 217 PAY 214 

    0xb54f8198,// 218 PAY 215 

    0xd8c1d7eb,// 219 PAY 216 

    0x86962fb7,// 220 PAY 217 

    0xc868ba42,// 221 PAY 218 

    0x4015af9e,// 222 PAY 219 

    0xf57be33a,// 223 PAY 220 

    0x30efcf03,// 224 PAY 221 

    0xa36867d7,// 225 PAY 222 

    0xc677b161,// 226 PAY 223 

    0x8ad9ef90,// 227 PAY 224 

    0x849c0d66,// 228 PAY 225 

    0x244ac6e9,// 229 PAY 226 

    0x73260699,// 230 PAY 227 

    0x83b7222a,// 231 PAY 228 

    0x7b5f811a,// 232 PAY 229 

    0xab36c148,// 233 PAY 230 

    0xef43746c,// 234 PAY 231 

    0x410bc95d,// 235 PAY 232 

    0x785159b6,// 236 PAY 233 

    0xfcaeec28,// 237 PAY 234 

    0x74d6d293,// 238 PAY 235 

    0x88afae9c,// 239 PAY 236 

    0x06f89b14,// 240 PAY 237 

    0x36041e5f,// 241 PAY 238 

    0x3235466d,// 242 PAY 239 

    0xb26ffe7a,// 243 PAY 240 

    0x7d114257,// 244 PAY 241 

    0x3ad50155,// 245 PAY 242 

    0xc48c3e90,// 246 PAY 243 

    0xb201f0d7,// 247 PAY 244 

    0x1ced08df,// 248 PAY 245 

    0x57097bdf,// 249 PAY 246 

    0x03c75cc4,// 250 PAY 247 

    0x9582274e,// 251 PAY 248 

    0x7b0b24d1,// 252 PAY 249 

    0x591df92b,// 253 PAY 250 

    0x50cc85d0,// 254 PAY 251 

    0x4a8dee6f,// 255 PAY 252 

    0x551bfc31,// 256 PAY 253 

    0x2c22a25f,// 257 PAY 254 

    0x6487f7a9,// 258 PAY 255 

    0x5c2b27d6,// 259 PAY 256 

    0x9fc95d58,// 260 PAY 257 

    0xe278b523,// 261 PAY 258 

    0x1a9962a3,// 262 PAY 259 

    0xc9cc4a3d,// 263 PAY 260 

    0x07a8bf38,// 264 PAY 261 

    0xce78f9fd,// 265 PAY 262 

    0xbde2c508,// 266 PAY 263 

    0xcc39b915,// 267 PAY 264 

    0xa784fe52,// 268 PAY 265 

    0xe241d1fe,// 269 PAY 266 

    0x25cf2110,// 270 PAY 267 

    0x6bf8ff95,// 271 PAY 268 

    0x45fe67c1,// 272 PAY 269 

    0x0e301046,// 273 PAY 270 

    0x21593869,// 274 PAY 271 

    0x70656f22,// 275 PAY 272 

    0x762de594,// 276 PAY 273 

    0xf4f58672,// 277 PAY 274 

    0xae24b9f4,// 278 PAY 275 

    0x59872a7d,// 279 PAY 276 

    0x315f923e,// 280 PAY 277 

    0xcff6588b,// 281 PAY 278 

    0x1ed2772c,// 282 PAY 279 

    0x9f28bf67,// 283 PAY 280 

    0x224d702b,// 284 PAY 281 

    0xa53ea84f,// 285 PAY 282 

    0xa7b2fe23,// 286 PAY 283 

    0x7065681f,// 287 PAY 284 

    0x115f8fd8,// 288 PAY 285 

    0x21314129,// 289 PAY 286 

    0x6970e7bb,// 290 PAY 287 

    0x26b2735c,// 291 PAY 288 

    0x7dccd3d7,// 292 PAY 289 

    0x07450564,// 293 PAY 290 

    0xbb8260fb,// 294 PAY 291 

    0x7b61a56d,// 295 PAY 292 

    0x18e71c4b,// 296 PAY 293 

    0xed136e65,// 297 PAY 294 

    0xfe6e899c,// 298 PAY 295 

    0x38b614ae,// 299 PAY 296 

    0xa91b831b,// 300 PAY 297 

    0x4e8354ca,// 301 PAY 298 

    0x562a9fb5,// 302 PAY 299 

    0xacaba58c,// 303 PAY 300 

    0xdfd1950c,// 304 PAY 301 

    0xb49b9f75,// 305 PAY 302 

    0x506a672f,// 306 PAY 303 

    0xa0c03e6f,// 307 PAY 304 

    0xdb5d15ec,// 308 PAY 305 

    0x14b5e3ad,// 309 PAY 306 

    0xfc8646f8,// 310 PAY 307 

    0xd72be61f,// 311 PAY 308 

    0xb604a6bd,// 312 PAY 309 

    0x09937785,// 313 PAY 310 

    0x70476f9e,// 314 PAY 311 

    0x41a4be1f,// 315 PAY 312 

    0x2456b66b,// 316 PAY 313 

    0xef465d39,// 317 PAY 314 

    0x02e3af14,// 318 PAY 315 

    0x6ac188a2,// 319 PAY 316 

    0x328085cc,// 320 PAY 317 

    0x717c828e,// 321 PAY 318 

    0x8ff33acc,// 322 PAY 319 

    0x2be9594a,// 323 PAY 320 

    0xa78862aa,// 324 PAY 321 

    0x65d0c74d,// 325 PAY 322 

    0xd3835914,// 326 PAY 323 

    0xd019f458,// 327 PAY 324 

    0xda49dbb2,// 328 PAY 325 

    0x493d5533,// 329 PAY 326 

    0xacfd6143,// 330 PAY 327 

    0x14a7aae5,// 331 PAY 328 

    0x078c3a6b,// 332 PAY 329 

    0x206c87c4,// 333 PAY 330 

    0x38eab481,// 334 PAY 331 

    0x4f7d5b7b,// 335 PAY 332 

    0xedd1277d,// 336 PAY 333 

/// HASH is  12 bytes 

    0x9fc95d58,// 337 HSH   1 

    0xe278b523,// 338 HSH   2 

    0x1a9962a3,// 339 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 171 

/// STA pkt_idx        : 56 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc7 

    0x00e0c7ab // 340 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt91_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 148 words. 

/// BDA size     is 586 (0x24a) 

/// BDA id       is 0xd7b7 

    0x024ad7b7,// 3 BDA   1 

/// PAY Generic Data size   : 586 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x491612e8,// 4 PAY   1 

    0x8596e8c4,// 5 PAY   2 

    0x4fefa29f,// 6 PAY   3 

    0xe76015a7,// 7 PAY   4 

    0xa8388b13,// 8 PAY   5 

    0xda43c32e,// 9 PAY   6 

    0xed36a573,// 10 PAY   7 

    0x687f1528,// 11 PAY   8 

    0x2701f753,// 12 PAY   9 

    0x8272620c,// 13 PAY  10 

    0x838c4dab,// 14 PAY  11 

    0xcb746848,// 15 PAY  12 

    0x7a7ed674,// 16 PAY  13 

    0xf18b9b54,// 17 PAY  14 

    0x17543ce7,// 18 PAY  15 

    0x92bf64b2,// 19 PAY  16 

    0xb28b93fb,// 20 PAY  17 

    0xdd5e4181,// 21 PAY  18 

    0xa0f16d79,// 22 PAY  19 

    0xfea7c9c2,// 23 PAY  20 

    0x104daaf0,// 24 PAY  21 

    0x0fbe37d1,// 25 PAY  22 

    0x2b04db93,// 26 PAY  23 

    0x56b117e4,// 27 PAY  24 

    0x48857033,// 28 PAY  25 

    0xba95bb85,// 29 PAY  26 

    0xe48a754b,// 30 PAY  27 

    0x85ecc591,// 31 PAY  28 

    0x72955949,// 32 PAY  29 

    0x2a6003c8,// 33 PAY  30 

    0xf604b653,// 34 PAY  31 

    0x4c7f8b7b,// 35 PAY  32 

    0x1b4553e0,// 36 PAY  33 

    0x09e95966,// 37 PAY  34 

    0x784d5c8a,// 38 PAY  35 

    0x3ffa160d,// 39 PAY  36 

    0xa9901192,// 40 PAY  37 

    0x339b7d12,// 41 PAY  38 

    0x2d07f61f,// 42 PAY  39 

    0x1b9b9555,// 43 PAY  40 

    0xb97c3a23,// 44 PAY  41 

    0x6f5cba65,// 45 PAY  42 

    0xdbe18169,// 46 PAY  43 

    0xd24e2fa5,// 47 PAY  44 

    0x7f279210,// 48 PAY  45 

    0x83a1f24c,// 49 PAY  46 

    0xc514afe7,// 50 PAY  47 

    0x67de3fe1,// 51 PAY  48 

    0x246844d4,// 52 PAY  49 

    0x6a4e9539,// 53 PAY  50 

    0xad57a8df,// 54 PAY  51 

    0x6fe2aa44,// 55 PAY  52 

    0x3d5ef42b,// 56 PAY  53 

    0x81bfd0e2,// 57 PAY  54 

    0xe92176aa,// 58 PAY  55 

    0xc30ddfa4,// 59 PAY  56 

    0x3ed0868d,// 60 PAY  57 

    0xd017cdab,// 61 PAY  58 

    0x075fbbf3,// 62 PAY  59 

    0x9e21670b,// 63 PAY  60 

    0x99a7aaad,// 64 PAY  61 

    0x3fe72425,// 65 PAY  62 

    0x86a44b1a,// 66 PAY  63 

    0xec65ec04,// 67 PAY  64 

    0xbdd2e6ac,// 68 PAY  65 

    0x8d3a8937,// 69 PAY  66 

    0x08a4b027,// 70 PAY  67 

    0x597377e6,// 71 PAY  68 

    0x5de9aca9,// 72 PAY  69 

    0xdf8a78e5,// 73 PAY  70 

    0x7576cdc8,// 74 PAY  71 

    0x9b9577ce,// 75 PAY  72 

    0x468ea220,// 76 PAY  73 

    0x9ea70769,// 77 PAY  74 

    0xdfba29bc,// 78 PAY  75 

    0xfbfb2d6b,// 79 PAY  76 

    0x9b181a56,// 80 PAY  77 

    0x8153b182,// 81 PAY  78 

    0x47322e48,// 82 PAY  79 

    0x033c1420,// 83 PAY  80 

    0x7de9b50d,// 84 PAY  81 

    0x3cdec7c4,// 85 PAY  82 

    0x1c183dcd,// 86 PAY  83 

    0xd00b1494,// 87 PAY  84 

    0xb88f6355,// 88 PAY  85 

    0x6c9ea901,// 89 PAY  86 

    0x1af58475,// 90 PAY  87 

    0xe9ea0d96,// 91 PAY  88 

    0x85e4d52d,// 92 PAY  89 

    0xaf864356,// 93 PAY  90 

    0x27d48e74,// 94 PAY  91 

    0xaa79695e,// 95 PAY  92 

    0x45c8ef14,// 96 PAY  93 

    0xb3182869,// 97 PAY  94 

    0xec84a4c1,// 98 PAY  95 

    0x25d73c6e,// 99 PAY  96 

    0xff080570,// 100 PAY  97 

    0x8f9cbb01,// 101 PAY  98 

    0xfdd04668,// 102 PAY  99 

    0xc29e4ccd,// 103 PAY 100 

    0x525aa1fc,// 104 PAY 101 

    0x36f00d14,// 105 PAY 102 

    0xe42f9d1a,// 106 PAY 103 

    0x213584c8,// 107 PAY 104 

    0x0c5af466,// 108 PAY 105 

    0xfbb02557,// 109 PAY 106 

    0x35d86126,// 110 PAY 107 

    0xc5275c06,// 111 PAY 108 

    0x7c13cb26,// 112 PAY 109 

    0xaf93600a,// 113 PAY 110 

    0x1ebdacd6,// 114 PAY 111 

    0xcfe4eabf,// 115 PAY 112 

    0x878e75ee,// 116 PAY 113 

    0xb2dc0067,// 117 PAY 114 

    0x6182d6d9,// 118 PAY 115 

    0x56431068,// 119 PAY 116 

    0x8dc535ad,// 120 PAY 117 

    0x7dea7977,// 121 PAY 118 

    0x7ad707fd,// 122 PAY 119 

    0xbc16dd2f,// 123 PAY 120 

    0x204ab82c,// 124 PAY 121 

    0xec95b24b,// 125 PAY 122 

    0x2c327756,// 126 PAY 123 

    0x191341c4,// 127 PAY 124 

    0xea233509,// 128 PAY 125 

    0x4d42719a,// 129 PAY 126 

    0x29955398,// 130 PAY 127 

    0x357a2aba,// 131 PAY 128 

    0x34597805,// 132 PAY 129 

    0x5bd1279f,// 133 PAY 130 

    0x72b484e6,// 134 PAY 131 

    0x1651476b,// 135 PAY 132 

    0xb9343fa6,// 136 PAY 133 

    0xc64db4ba,// 137 PAY 134 

    0x5440ca92,// 138 PAY 135 

    0x61f32168,// 139 PAY 136 

    0x26f3ed76,// 140 PAY 137 

    0xa7d188e5,// 141 PAY 138 

    0x00760b72,// 142 PAY 139 

    0x70915cdb,// 143 PAY 140 

    0xf375e8f5,// 144 PAY 141 

    0x3b005850,// 145 PAY 142 

    0x24b4892e,// 146 PAY 143 

    0x47fdbc50,// 147 PAY 144 

    0x6ffd8376,// 148 PAY 145 

    0xa318ee51,// 149 PAY 146 

    0x7d180000,// 150 PAY 147 

/// STA is 1 words. 

/// STA num_pkts       : 249 

/// STA pkt_idx        : 166 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x029904f9 // 151 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt92_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 339 words. 

/// BDA size     is 1349 (0x545) 

/// BDA id       is 0xb198 

    0x0545b198,// 3 BDA   1 

/// PAY Generic Data size   : 1349 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xbfbb9237,// 4 PAY   1 

    0x4e7f93cb,// 5 PAY   2 

    0x0672d332,// 6 PAY   3 

    0xc8b946df,// 7 PAY   4 

    0xfff6aaf3,// 8 PAY   5 

    0x7b54db20,// 9 PAY   6 

    0x98fe9626,// 10 PAY   7 

    0x46b789b0,// 11 PAY   8 

    0x803f2d1f,// 12 PAY   9 

    0xd052ffe8,// 13 PAY  10 

    0x46a5a0ac,// 14 PAY  11 

    0xff64f617,// 15 PAY  12 

    0x0c529373,// 16 PAY  13 

    0xf5c7c6b7,// 17 PAY  14 

    0x6d3263e0,// 18 PAY  15 

    0xd2c2a403,// 19 PAY  16 

    0xd8377f21,// 20 PAY  17 

    0xaf62e025,// 21 PAY  18 

    0xdea708ff,// 22 PAY  19 

    0xe4a665a2,// 23 PAY  20 

    0xb66b9df2,// 24 PAY  21 

    0x1a9054e5,// 25 PAY  22 

    0x3dccdd1e,// 26 PAY  23 

    0x252a8948,// 27 PAY  24 

    0x206cacb9,// 28 PAY  25 

    0x6ae96d0d,// 29 PAY  26 

    0x08ee9199,// 30 PAY  27 

    0x89a408ff,// 31 PAY  28 

    0xb563387d,// 32 PAY  29 

    0x5a32a298,// 33 PAY  30 

    0x70e29e85,// 34 PAY  31 

    0x211bbea1,// 35 PAY  32 

    0xcff96260,// 36 PAY  33 

    0x79586c1d,// 37 PAY  34 

    0xeaf3609e,// 38 PAY  35 

    0x2d758922,// 39 PAY  36 

    0x04901e67,// 40 PAY  37 

    0xe8b04799,// 41 PAY  38 

    0x0be52019,// 42 PAY  39 

    0xb11d5de7,// 43 PAY  40 

    0x28bcf53d,// 44 PAY  41 

    0xa19dac8a,// 45 PAY  42 

    0x5c7c683b,// 46 PAY  43 

    0xcc0a9b66,// 47 PAY  44 

    0x534dd592,// 48 PAY  45 

    0x8fb09985,// 49 PAY  46 

    0x78bc1351,// 50 PAY  47 

    0x874bd3d2,// 51 PAY  48 

    0xb4f71ddf,// 52 PAY  49 

    0xf52a0cb8,// 53 PAY  50 

    0xf65cbf43,// 54 PAY  51 

    0x2ca26ea0,// 55 PAY  52 

    0x5cdc7a13,// 56 PAY  53 

    0x6d13e1d2,// 57 PAY  54 

    0x05ec8546,// 58 PAY  55 

    0xad41a08a,// 59 PAY  56 

    0x32ffbc3c,// 60 PAY  57 

    0x732e28a8,// 61 PAY  58 

    0xb0ee3bd3,// 62 PAY  59 

    0x17eab0eb,// 63 PAY  60 

    0x77c4ae1a,// 64 PAY  61 

    0x43e424ff,// 65 PAY  62 

    0x4808c27f,// 66 PAY  63 

    0x26ae1530,// 67 PAY  64 

    0x1bff00e9,// 68 PAY  65 

    0x5419c93f,// 69 PAY  66 

    0xc36f77f3,// 70 PAY  67 

    0x1ed50e28,// 71 PAY  68 

    0xc178a916,// 72 PAY  69 

    0x2b798cb0,// 73 PAY  70 

    0x2a59fe32,// 74 PAY  71 

    0x07e85ca4,// 75 PAY  72 

    0xbd6b961c,// 76 PAY  73 

    0x858671b7,// 77 PAY  74 

    0x9b733bbf,// 78 PAY  75 

    0x13bb4220,// 79 PAY  76 

    0x8781f7f5,// 80 PAY  77 

    0xe3e78fc3,// 81 PAY  78 

    0x41a45cac,// 82 PAY  79 

    0x37fcc0bb,// 83 PAY  80 

    0xbefae919,// 84 PAY  81 

    0xc76d1120,// 85 PAY  82 

    0x7feb5910,// 86 PAY  83 

    0xa057fa6a,// 87 PAY  84 

    0xd9db4967,// 88 PAY  85 

    0x466fc5b6,// 89 PAY  86 

    0x00f87188,// 90 PAY  87 

    0xdf1c7874,// 91 PAY  88 

    0x078cfe38,// 92 PAY  89 

    0xbc3723dc,// 93 PAY  90 

    0x3202a134,// 94 PAY  91 

    0xa467e60e,// 95 PAY  92 

    0x16662ce0,// 96 PAY  93 

    0x880f0d27,// 97 PAY  94 

    0xf4574096,// 98 PAY  95 

    0x2efabc8e,// 99 PAY  96 

    0xc9631fce,// 100 PAY  97 

    0xa22354ee,// 101 PAY  98 

    0xe6a5ab66,// 102 PAY  99 

    0xe8035e04,// 103 PAY 100 

    0xc2dad513,// 104 PAY 101 

    0xdcc9c7d7,// 105 PAY 102 

    0x390be9e8,// 106 PAY 103 

    0xd5f7fb86,// 107 PAY 104 

    0x9f1de1ba,// 108 PAY 105 

    0x0e336667,// 109 PAY 106 

    0x64d7d777,// 110 PAY 107 

    0x311c275e,// 111 PAY 108 

    0x4725140d,// 112 PAY 109 

    0xb61c825b,// 113 PAY 110 

    0xb07f6765,// 114 PAY 111 

    0x5c5d12d1,// 115 PAY 112 

    0xdf71229c,// 116 PAY 113 

    0xd539eb78,// 117 PAY 114 

    0x5346de82,// 118 PAY 115 

    0x4f31b1ae,// 119 PAY 116 

    0x9ad39086,// 120 PAY 117 

    0x6d29d56d,// 121 PAY 118 

    0x3c42d6b8,// 122 PAY 119 

    0xff817f03,// 123 PAY 120 

    0xbcd02ade,// 124 PAY 121 

    0xa4836706,// 125 PAY 122 

    0x83be4957,// 126 PAY 123 

    0x68d7c489,// 127 PAY 124 

    0x897720c7,// 128 PAY 125 

    0x71a6c202,// 129 PAY 126 

    0xcdf8f114,// 130 PAY 127 

    0x99c7ad0b,// 131 PAY 128 

    0xce5b5e09,// 132 PAY 129 

    0x47f739fb,// 133 PAY 130 

    0x53e87bae,// 134 PAY 131 

    0x9a443b92,// 135 PAY 132 

    0x69054b6a,// 136 PAY 133 

    0x60bcf2f4,// 137 PAY 134 

    0xa3ae670e,// 138 PAY 135 

    0x861ffa18,// 139 PAY 136 

    0x05171441,// 140 PAY 137 

    0xd25f8252,// 141 PAY 138 

    0xe6be4331,// 142 PAY 139 

    0x49ae8ccb,// 143 PAY 140 

    0x4ef171df,// 144 PAY 141 

    0xecfbdd1c,// 145 PAY 142 

    0x2ec96503,// 146 PAY 143 

    0x7d37d89a,// 147 PAY 144 

    0x02db9527,// 148 PAY 145 

    0x09bec862,// 149 PAY 146 

    0xf27dde39,// 150 PAY 147 

    0xcee4a2ea,// 151 PAY 148 

    0x42be00bf,// 152 PAY 149 

    0xe31cc77a,// 153 PAY 150 

    0x9ca4f8e7,// 154 PAY 151 

    0xaaaae962,// 155 PAY 152 

    0x4d61b534,// 156 PAY 153 

    0x4075e620,// 157 PAY 154 

    0xacdd294e,// 158 PAY 155 

    0x19736fbc,// 159 PAY 156 

    0x18b00c57,// 160 PAY 157 

    0x5c6950af,// 161 PAY 158 

    0x2bbd686f,// 162 PAY 159 

    0x766213ab,// 163 PAY 160 

    0xebac90ff,// 164 PAY 161 

    0xee4448b3,// 165 PAY 162 

    0x5e1d9bdb,// 166 PAY 163 

    0xa551f327,// 167 PAY 164 

    0x89f29770,// 168 PAY 165 

    0x714f676c,// 169 PAY 166 

    0x62bff5ce,// 170 PAY 167 

    0xed0ba87c,// 171 PAY 168 

    0x0068dba6,// 172 PAY 169 

    0x647323de,// 173 PAY 170 

    0x85498c51,// 174 PAY 171 

    0x46f53052,// 175 PAY 172 

    0x44384de0,// 176 PAY 173 

    0x18ce1e37,// 177 PAY 174 

    0x00e3a894,// 178 PAY 175 

    0x397c00ab,// 179 PAY 176 

    0x10698ce6,// 180 PAY 177 

    0xccfe6a0a,// 181 PAY 178 

    0x52bbcf22,// 182 PAY 179 

    0xa6c7ef3c,// 183 PAY 180 

    0x179ae5f4,// 184 PAY 181 

    0xc4a547b1,// 185 PAY 182 

    0x53284efa,// 186 PAY 183 

    0xd82185f4,// 187 PAY 184 

    0xfa1a90f5,// 188 PAY 185 

    0x4ef4a7d5,// 189 PAY 186 

    0x634baf86,// 190 PAY 187 

    0x34c66cb9,// 191 PAY 188 

    0x9361fe8e,// 192 PAY 189 

    0x93a2c4ba,// 193 PAY 190 

    0xd6a7f0e7,// 194 PAY 191 

    0xecb9936c,// 195 PAY 192 

    0xdec521bb,// 196 PAY 193 

    0x0942127a,// 197 PAY 194 

    0xc27d02e7,// 198 PAY 195 

    0x42ab751b,// 199 PAY 196 

    0x1dc7104a,// 200 PAY 197 

    0xf223b4af,// 201 PAY 198 

    0x4b016ecf,// 202 PAY 199 

    0xf8be814c,// 203 PAY 200 

    0xb1e14b97,// 204 PAY 201 

    0xe4b7e4f1,// 205 PAY 202 

    0x6065a225,// 206 PAY 203 

    0x5da140b7,// 207 PAY 204 

    0xc567100e,// 208 PAY 205 

    0xfd333555,// 209 PAY 206 

    0x02c978e4,// 210 PAY 207 

    0x2432d3ef,// 211 PAY 208 

    0x793def0a,// 212 PAY 209 

    0x9ac4aeaf,// 213 PAY 210 

    0xca5038de,// 214 PAY 211 

    0x0ebef674,// 215 PAY 212 

    0x90a9c4c1,// 216 PAY 213 

    0xc577c7d6,// 217 PAY 214 

    0x5f1ebf34,// 218 PAY 215 

    0x227ab3dd,// 219 PAY 216 

    0xbd45e38f,// 220 PAY 217 

    0x97cd2c34,// 221 PAY 218 

    0x480ecf8f,// 222 PAY 219 

    0x3eefdf13,// 223 PAY 220 

    0x9fc2c154,// 224 PAY 221 

    0x5351393f,// 225 PAY 222 

    0xaff849c5,// 226 PAY 223 

    0x571daa43,// 227 PAY 224 

    0x99ff500f,// 228 PAY 225 

    0xacbbdf47,// 229 PAY 226 

    0x6180e64e,// 230 PAY 227 

    0xeba33f99,// 231 PAY 228 

    0xba056211,// 232 PAY 229 

    0x37a28577,// 233 PAY 230 

    0x16bfa42b,// 234 PAY 231 

    0x09da3f39,// 235 PAY 232 

    0x3bd955ee,// 236 PAY 233 

    0xb8c7c753,// 237 PAY 234 

    0x983e531e,// 238 PAY 235 

    0xc60bdcf0,// 239 PAY 236 

    0xed2c9705,// 240 PAY 237 

    0x14f54e6a,// 241 PAY 238 

    0x8d964fa1,// 242 PAY 239 

    0x18094735,// 243 PAY 240 

    0x23d983c7,// 244 PAY 241 

    0xc08abae0,// 245 PAY 242 

    0xd38a4f94,// 246 PAY 243 

    0x48da58a6,// 247 PAY 244 

    0x6d5f5ee5,// 248 PAY 245 

    0x27c0a6b0,// 249 PAY 246 

    0xf0420a70,// 250 PAY 247 

    0xca4ea0d8,// 251 PAY 248 

    0x458a7cb2,// 252 PAY 249 

    0x264b7734,// 253 PAY 250 

    0x4c62b801,// 254 PAY 251 

    0x9e05f5e4,// 255 PAY 252 

    0x48f72ac8,// 256 PAY 253 

    0x0a9a8e3e,// 257 PAY 254 

    0x9cb21b4e,// 258 PAY 255 

    0x0608fb02,// 259 PAY 256 

    0xb3ca4927,// 260 PAY 257 

    0xc2ded748,// 261 PAY 258 

    0x0d8cdf66,// 262 PAY 259 

    0xda80f1a7,// 263 PAY 260 

    0x93c9d2e2,// 264 PAY 261 

    0x92a7abb2,// 265 PAY 262 

    0xd534f5b3,// 266 PAY 263 

    0xdd48c1ea,// 267 PAY 264 

    0x3deaf0ba,// 268 PAY 265 

    0xae643e92,// 269 PAY 266 

    0x1344d063,// 270 PAY 267 

    0xb2ae0e79,// 271 PAY 268 

    0x61055cf1,// 272 PAY 269 

    0x2f46c1ae,// 273 PAY 270 

    0xa112224d,// 274 PAY 271 

    0x680ba368,// 275 PAY 272 

    0xfa38a5c6,// 276 PAY 273 

    0x63ff1a9e,// 277 PAY 274 

    0x2931716c,// 278 PAY 275 

    0x2859e905,// 279 PAY 276 

    0x7ff8c3c0,// 280 PAY 277 

    0xdcec4ded,// 281 PAY 278 

    0xe3c0e0e8,// 282 PAY 279 

    0x160a2e41,// 283 PAY 280 

    0x2a4b39bf,// 284 PAY 281 

    0xb4cb6020,// 285 PAY 282 

    0xa10f6062,// 286 PAY 283 

    0xad52e57c,// 287 PAY 284 

    0x05f4d633,// 288 PAY 285 

    0x7d16b8a2,// 289 PAY 286 

    0x9f8cc5ae,// 290 PAY 287 

    0x644b9c7d,// 291 PAY 288 

    0x3b9b2595,// 292 PAY 289 

    0x60f08d20,// 293 PAY 290 

    0x70836746,// 294 PAY 291 

    0x3148d088,// 295 PAY 292 

    0x80d2deb6,// 296 PAY 293 

    0x5ad7edbc,// 297 PAY 294 

    0x1486ed0c,// 298 PAY 295 

    0x8bc248b9,// 299 PAY 296 

    0xafd9a50d,// 300 PAY 297 

    0x72809848,// 301 PAY 298 

    0xea1d9aef,// 302 PAY 299 

    0xc2d27a7e,// 303 PAY 300 

    0x6543ecf5,// 304 PAY 301 

    0x6c0c3069,// 305 PAY 302 

    0xe0b1f9af,// 306 PAY 303 

    0xb0c515d6,// 307 PAY 304 

    0x6b62a169,// 308 PAY 305 

    0xf833119c,// 309 PAY 306 

    0x321abd9e,// 310 PAY 307 

    0x93cedbe2,// 311 PAY 308 

    0x0c232463,// 312 PAY 309 

    0x293ee24b,// 313 PAY 310 

    0x6bcfce2a,// 314 PAY 311 

    0xd4118c59,// 315 PAY 312 

    0x34cf1324,// 316 PAY 313 

    0xd79dc6c2,// 317 PAY 314 

    0xf973862c,// 318 PAY 315 

    0xca59e989,// 319 PAY 316 

    0x59e0fd1e,// 320 PAY 317 

    0x71a8493a,// 321 PAY 318 

    0xabceb416,// 322 PAY 319 

    0xbeb39386,// 323 PAY 320 

    0xbbe2010a,// 324 PAY 321 

    0x0b813023,// 325 PAY 322 

    0x7ff07839,// 326 PAY 323 

    0x7216c4ab,// 327 PAY 324 

    0xbe6a6eab,// 328 PAY 325 

    0x6558a008,// 329 PAY 326 

    0x18e7b998,// 330 PAY 327 

    0x7d197f10,// 331 PAY 328 

    0x394fa286,// 332 PAY 329 

    0x08e0da71,// 333 PAY 330 

    0xa7da78ed,// 334 PAY 331 

    0x3a53e305,// 335 PAY 332 

    0x31caf70d,// 336 PAY 333 

    0xbbc144ca,// 337 PAY 334 

    0x7fd24c84,// 338 PAY 335 

    0x1d3bdbdf,// 339 PAY 336 

    0x04e4f075,// 340 PAY 337 

    0x55000000,// 341 PAY 338 

/// HASH is  16 bytes 

    0xd25f8252,// 342 HSH   1 

    0xe6be4331,// 343 HSH   2 

    0x49ae8ccb,// 344 HSH   3 

    0x4ef171df,// 345 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 86 

/// STA pkt_idx        : 224 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf3 

    0x0381f356 // 346 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 104 (0x68) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt93_tmpl[] = {
    0x0c010068,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 377 words. 

/// BDA size     is 1504 (0x5e0) 

/// BDA id       is 0x3f09 

    0x05e03f09,// 3 BDA   1 

/// PAY Generic Data size   : 1504 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x9e115b70,// 4 PAY   1 

    0x93b894a5,// 5 PAY   2 

    0xac2b6bbd,// 6 PAY   3 

    0x7f90ace6,// 7 PAY   4 

    0xe3bca47c,// 8 PAY   5 

    0x2e0e4e02,// 9 PAY   6 

    0x630819af,// 10 PAY   7 

    0x8019ee20,// 11 PAY   8 

    0x082c1fa4,// 12 PAY   9 

    0xcb932028,// 13 PAY  10 

    0xdb63dd73,// 14 PAY  11 

    0x306cdecf,// 15 PAY  12 

    0x6a48321b,// 16 PAY  13 

    0x6c4fd51d,// 17 PAY  14 

    0x152cb477,// 18 PAY  15 

    0x7f0e50a9,// 19 PAY  16 

    0x6667a3bc,// 20 PAY  17 

    0x9a77d373,// 21 PAY  18 

    0x26fda115,// 22 PAY  19 

    0x21b99162,// 23 PAY  20 

    0x78cd0a7e,// 24 PAY  21 

    0x14fc5559,// 25 PAY  22 

    0x37d72d78,// 26 PAY  23 

    0x96a2f549,// 27 PAY  24 

    0x55899458,// 28 PAY  25 

    0xca1239ed,// 29 PAY  26 

    0xbcd17b5a,// 30 PAY  27 

    0x7f1b4d04,// 31 PAY  28 

    0xab3a8917,// 32 PAY  29 

    0x9c6bdaac,// 33 PAY  30 

    0xc7a8a4b6,// 34 PAY  31 

    0x03bc8383,// 35 PAY  32 

    0x1d186496,// 36 PAY  33 

    0xf84ce957,// 37 PAY  34 

    0x8820fd4a,// 38 PAY  35 

    0xd1ee0d6c,// 39 PAY  36 

    0x0eade609,// 40 PAY  37 

    0x35d734d5,// 41 PAY  38 

    0xa069b0c7,// 42 PAY  39 

    0x3e345d9e,// 43 PAY  40 

    0x76fd174b,// 44 PAY  41 

    0xf60e95c9,// 45 PAY  42 

    0x6328d70a,// 46 PAY  43 

    0x80a148aa,// 47 PAY  44 

    0xf7bc31c2,// 48 PAY  45 

    0x2fdf05dd,// 49 PAY  46 

    0xef3d8602,// 50 PAY  47 

    0xaf35fabd,// 51 PAY  48 

    0x239394e1,// 52 PAY  49 

    0xdb63bf90,// 53 PAY  50 

    0x567c7a39,// 54 PAY  51 

    0x1ca87d03,// 55 PAY  52 

    0x4cae362d,// 56 PAY  53 

    0x8ae553a6,// 57 PAY  54 

    0x346206a7,// 58 PAY  55 

    0x7f037a0d,// 59 PAY  56 

    0x5541a332,// 60 PAY  57 

    0x934d5dab,// 61 PAY  58 

    0xff8d0446,// 62 PAY  59 

    0x8447a682,// 63 PAY  60 

    0xc3bd73eb,// 64 PAY  61 

    0x25a4218b,// 65 PAY  62 

    0x946b1b99,// 66 PAY  63 

    0x043a591b,// 67 PAY  64 

    0x9b8353a6,// 68 PAY  65 

    0xf34bcb4e,// 69 PAY  66 

    0xf0283a27,// 70 PAY  67 

    0x8c7d1b86,// 71 PAY  68 

    0x455d423b,// 72 PAY  69 

    0x4491b890,// 73 PAY  70 

    0x1116e5cd,// 74 PAY  71 

    0xfbb7d532,// 75 PAY  72 

    0x2161ae47,// 76 PAY  73 

    0x6a7933dd,// 77 PAY  74 

    0x6a68d189,// 78 PAY  75 

    0x64e2fc55,// 79 PAY  76 

    0xc8c08719,// 80 PAY  77 

    0x2b04ae1f,// 81 PAY  78 

    0x937c1553,// 82 PAY  79 

    0x50ba2a72,// 83 PAY  80 

    0x56ee2365,// 84 PAY  81 

    0x94368a2e,// 85 PAY  82 

    0x2256129c,// 86 PAY  83 

    0x52e0f413,// 87 PAY  84 

    0x16925e55,// 88 PAY  85 

    0x3f440a4e,// 89 PAY  86 

    0x27c4656d,// 90 PAY  87 

    0x86874ddd,// 91 PAY  88 

    0xaf0e1bb7,// 92 PAY  89 

    0x89315f55,// 93 PAY  90 

    0x127782f7,// 94 PAY  91 

    0xdf928c29,// 95 PAY  92 

    0xb6c6f499,// 96 PAY  93 

    0x3c594cc5,// 97 PAY  94 

    0x1ec4d605,// 98 PAY  95 

    0x31be228a,// 99 PAY  96 

    0x34496b81,// 100 PAY  97 

    0x7548e026,// 101 PAY  98 

    0x99fe5377,// 102 PAY  99 

    0x5d2e732f,// 103 PAY 100 

    0xdc43ffad,// 104 PAY 101 

    0x920d9351,// 105 PAY 102 

    0x95549734,// 106 PAY 103 

    0xa4560a08,// 107 PAY 104 

    0x1105056d,// 108 PAY 105 

    0x88139bd4,// 109 PAY 106 

    0xeedc9d63,// 110 PAY 107 

    0x978d3d4d,// 111 PAY 108 

    0xf77ddc69,// 112 PAY 109 

    0x9903cc7d,// 113 PAY 110 

    0x330e1042,// 114 PAY 111 

    0x6c6f1145,// 115 PAY 112 

    0x204a7e9c,// 116 PAY 113 

    0x917d9207,// 117 PAY 114 

    0xc3fdebe5,// 118 PAY 115 

    0x3317deac,// 119 PAY 116 

    0x1398bd2c,// 120 PAY 117 

    0x5bedc155,// 121 PAY 118 

    0x3101dfe8,// 122 PAY 119 

    0x8f56d900,// 123 PAY 120 

    0xa6c7b545,// 124 PAY 121 

    0x9c6b1158,// 125 PAY 122 

    0x981f161c,// 126 PAY 123 

    0x92925d19,// 127 PAY 124 

    0x42fd52b0,// 128 PAY 125 

    0x0586bd22,// 129 PAY 126 

    0x889fa9df,// 130 PAY 127 

    0xc46659e7,// 131 PAY 128 

    0x48cc596a,// 132 PAY 129 

    0x70557271,// 133 PAY 130 

    0x1dcd979a,// 134 PAY 131 

    0xa98a180a,// 135 PAY 132 

    0x809302df,// 136 PAY 133 

    0x006c4a49,// 137 PAY 134 

    0x492aded9,// 138 PAY 135 

    0x6bc3fbfb,// 139 PAY 136 

    0x34cec8f0,// 140 PAY 137 

    0xfb47f0d4,// 141 PAY 138 

    0xc0d3d204,// 142 PAY 139 

    0x9e0af40b,// 143 PAY 140 

    0x7f69fe6f,// 144 PAY 141 

    0x701c86af,// 145 PAY 142 

    0xf54bbaca,// 146 PAY 143 

    0x4b2a4d18,// 147 PAY 144 

    0xd1c30ce6,// 148 PAY 145 

    0x9ec023c7,// 149 PAY 146 

    0x52d57684,// 150 PAY 147 

    0x05002f65,// 151 PAY 148 

    0x08744b50,// 152 PAY 149 

    0x9ae0629f,// 153 PAY 150 

    0x3aa1c068,// 154 PAY 151 

    0xca0624bb,// 155 PAY 152 

    0x84189994,// 156 PAY 153 

    0xa402c1e1,// 157 PAY 154 

    0x743185d7,// 158 PAY 155 

    0xa8217aee,// 159 PAY 156 

    0xf6675eb0,// 160 PAY 157 

    0x649a5421,// 161 PAY 158 

    0xc7a70556,// 162 PAY 159 

    0x45f5930b,// 163 PAY 160 

    0x1e6b3d09,// 164 PAY 161 

    0x02486d58,// 165 PAY 162 

    0xccbc7f2e,// 166 PAY 163 

    0xe79f4f33,// 167 PAY 164 

    0x66c22c76,// 168 PAY 165 

    0x315afd4d,// 169 PAY 166 

    0x5c140d23,// 170 PAY 167 

    0x76efcc07,// 171 PAY 168 

    0x258617ce,// 172 PAY 169 

    0x6945ac7e,// 173 PAY 170 

    0x64e0551f,// 174 PAY 171 

    0x3a96b2d0,// 175 PAY 172 

    0x03f0ea9d,// 176 PAY 173 

    0xeb815eb9,// 177 PAY 174 

    0x31cc0434,// 178 PAY 175 

    0x062b28af,// 179 PAY 176 

    0xc213c016,// 180 PAY 177 

    0x953276bc,// 181 PAY 178 

    0xb5d7792a,// 182 PAY 179 

    0xe26cb8fd,// 183 PAY 180 

    0x5e11eff3,// 184 PAY 181 

    0x6ab6a443,// 185 PAY 182 

    0xacaf9977,// 186 PAY 183 

    0xcb2436eb,// 187 PAY 184 

    0x72efd4e9,// 188 PAY 185 

    0x4d6b675c,// 189 PAY 186 

    0x1506adf3,// 190 PAY 187 

    0xfeea2c10,// 191 PAY 188 

    0xe87daebc,// 192 PAY 189 

    0x06df42a9,// 193 PAY 190 

    0x11c176c3,// 194 PAY 191 

    0xf5314e11,// 195 PAY 192 

    0xf351a06c,// 196 PAY 193 

    0xb11786c1,// 197 PAY 194 

    0x985fc5db,// 198 PAY 195 

    0xeb77668c,// 199 PAY 196 

    0xa944246e,// 200 PAY 197 

    0x06ded0ea,// 201 PAY 198 

    0x4bd16281,// 202 PAY 199 

    0xf7840779,// 203 PAY 200 

    0xb32b3c57,// 204 PAY 201 

    0x1542b4c5,// 205 PAY 202 

    0x3dccb1d7,// 206 PAY 203 

    0x7358107a,// 207 PAY 204 

    0xe4abc3ee,// 208 PAY 205 

    0x49163fce,// 209 PAY 206 

    0x12401100,// 210 PAY 207 

    0x8ba74f46,// 211 PAY 208 

    0x35c2fb1f,// 212 PAY 209 

    0xd974e9c8,// 213 PAY 210 

    0x0ec84e01,// 214 PAY 211 

    0x75c003c9,// 215 PAY 212 

    0x0dd2980e,// 216 PAY 213 

    0x912aaade,// 217 PAY 214 

    0xea2bd3d8,// 218 PAY 215 

    0xdb98d988,// 219 PAY 216 

    0xeaae2912,// 220 PAY 217 

    0x96661bb3,// 221 PAY 218 

    0xb07b5a5d,// 222 PAY 219 

    0x166b8a1e,// 223 PAY 220 

    0xbb0620ea,// 224 PAY 221 

    0xc0ba6283,// 225 PAY 222 

    0x8f1f9b16,// 226 PAY 223 

    0x40157039,// 227 PAY 224 

    0xaae24d39,// 228 PAY 225 

    0xb4b7ac78,// 229 PAY 226 

    0x6f19dcad,// 230 PAY 227 

    0x923b5d0d,// 231 PAY 228 

    0x9e345767,// 232 PAY 229 

    0x8d20912b,// 233 PAY 230 

    0x6b8915c7,// 234 PAY 231 

    0xf81682b8,// 235 PAY 232 

    0x0e9bca88,// 236 PAY 233 

    0xf6ada36a,// 237 PAY 234 

    0x515ed9e1,// 238 PAY 235 

    0x970c5f9f,// 239 PAY 236 

    0xa0522bc8,// 240 PAY 237 

    0x5926994a,// 241 PAY 238 

    0xa4f20e4b,// 242 PAY 239 

    0x5d8f3d43,// 243 PAY 240 

    0xa3b698d8,// 244 PAY 241 

    0xe9245990,// 245 PAY 242 

    0x368a75bf,// 246 PAY 243 

    0xbef7fd94,// 247 PAY 244 

    0xc5eaa5c2,// 248 PAY 245 

    0xb1228221,// 249 PAY 246 

    0x2086fcd1,// 250 PAY 247 

    0x6bcbb53e,// 251 PAY 248 

    0x327ca951,// 252 PAY 249 

    0xd9bfd62f,// 253 PAY 250 

    0xcd6f346c,// 254 PAY 251 

    0xe58d0a45,// 255 PAY 252 

    0x1db64244,// 256 PAY 253 

    0x322460b4,// 257 PAY 254 

    0x01a3c591,// 258 PAY 255 

    0x9072ff53,// 259 PAY 256 

    0x78f8860b,// 260 PAY 257 

    0x5b7fcae1,// 261 PAY 258 

    0xa59ad7f0,// 262 PAY 259 

    0x0b270e46,// 263 PAY 260 

    0x3c336904,// 264 PAY 261 

    0xbb3ade71,// 265 PAY 262 

    0x87baf635,// 266 PAY 263 

    0x887fd2f4,// 267 PAY 264 

    0x23454e23,// 268 PAY 265 

    0x147fa0df,// 269 PAY 266 

    0x6efe01ec,// 270 PAY 267 

    0x7eda4869,// 271 PAY 268 

    0x1433a481,// 272 PAY 269 

    0x30d74167,// 273 PAY 270 

    0x8c334943,// 274 PAY 271 

    0x284208a8,// 275 PAY 272 

    0x82c94202,// 276 PAY 273 

    0x7a22aefd,// 277 PAY 274 

    0x89de39b7,// 278 PAY 275 

    0xe4a1f4d2,// 279 PAY 276 

    0xe86e3a87,// 280 PAY 277 

    0x38c5fdc3,// 281 PAY 278 

    0x016769ef,// 282 PAY 279 

    0x3fe84fd8,// 283 PAY 280 

    0x4adb2195,// 284 PAY 281 

    0x045127ab,// 285 PAY 282 

    0xf27f7e29,// 286 PAY 283 

    0x24f136ea,// 287 PAY 284 

    0x50970f1c,// 288 PAY 285 

    0x577e3742,// 289 PAY 286 

    0x5daccbce,// 290 PAY 287 

    0x3b8704e9,// 291 PAY 288 

    0xe88db1f0,// 292 PAY 289 

    0x03b81ead,// 293 PAY 290 

    0x53423d5b,// 294 PAY 291 

    0xc7adb43b,// 295 PAY 292 

    0x06d6da9f,// 296 PAY 293 

    0x65e6a637,// 297 PAY 294 

    0x04109d97,// 298 PAY 295 

    0xc51296b6,// 299 PAY 296 

    0xdeee792a,// 300 PAY 297 

    0x9b093deb,// 301 PAY 298 

    0xa1f5acd6,// 302 PAY 299 

    0xea22ddff,// 303 PAY 300 

    0x4839bed0,// 304 PAY 301 

    0x82c39f55,// 305 PAY 302 

    0x1d1a6f47,// 306 PAY 303 

    0x3b0dfcfd,// 307 PAY 304 

    0x79015aa6,// 308 PAY 305 

    0xb5986100,// 309 PAY 306 

    0xff9f9f26,// 310 PAY 307 

    0x30aa723c,// 311 PAY 308 

    0xcbb72d01,// 312 PAY 309 

    0x14220578,// 313 PAY 310 

    0x4c53dcc4,// 314 PAY 311 

    0x5302387e,// 315 PAY 312 

    0x266db291,// 316 PAY 313 

    0x3d13f8c4,// 317 PAY 314 

    0x43b0dcf0,// 318 PAY 315 

    0x4566dfd6,// 319 PAY 316 

    0xbda85ce9,// 320 PAY 317 

    0x9fd40dad,// 321 PAY 318 

    0xf4082642,// 322 PAY 319 

    0x542cf66e,// 323 PAY 320 

    0xf1d07057,// 324 PAY 321 

    0x41742b1b,// 325 PAY 322 

    0x2c0a3db8,// 326 PAY 323 

    0x248ce8c9,// 327 PAY 324 

    0x4ce07962,// 328 PAY 325 

    0xb8351a19,// 329 PAY 326 

    0xa6120403,// 330 PAY 327 

    0x5e5b916b,// 331 PAY 328 

    0xd4c9da7f,// 332 PAY 329 

    0x619e98c4,// 333 PAY 330 

    0xe9be78eb,// 334 PAY 331 

    0xb4d87222,// 335 PAY 332 

    0xa2c7993e,// 336 PAY 333 

    0x7c6ee843,// 337 PAY 334 

    0x9ac8c364,// 338 PAY 335 

    0xf86eb207,// 339 PAY 336 

    0xb54bbb74,// 340 PAY 337 

    0x63141673,// 341 PAY 338 

    0x67f536e5,// 342 PAY 339 

    0x54a2fb80,// 343 PAY 340 

    0xb7b0de2e,// 344 PAY 341 

    0xb0a8f44d,// 345 PAY 342 

    0x2d5ef5b2,// 346 PAY 343 

    0x6640dbc1,// 347 PAY 344 

    0x0624f108,// 348 PAY 345 

    0x23733b57,// 349 PAY 346 

    0xd73c5d05,// 350 PAY 347 

    0x485137c4,// 351 PAY 348 

    0x30b98045,// 352 PAY 349 

    0xf7fb77ad,// 353 PAY 350 

    0x2817f5d9,// 354 PAY 351 

    0xd9cbb430,// 355 PAY 352 

    0x040cdd29,// 356 PAY 353 

    0xeb16cff0,// 357 PAY 354 

    0x2abbbaa2,// 358 PAY 355 

    0x8c9d4c1e,// 359 PAY 356 

    0x9ae4313e,// 360 PAY 357 

    0xb3dede1d,// 361 PAY 358 

    0x88c4c1c3,// 362 PAY 359 

    0x408e9074,// 363 PAY 360 

    0xcb5a4dba,// 364 PAY 361 

    0xbf863ba6,// 365 PAY 362 

    0x2f47c5c2,// 366 PAY 363 

    0x80eb8121,// 367 PAY 364 

    0x00e02833,// 368 PAY 365 

    0xfa6d1ff2,// 369 PAY 366 

    0xe6f5a968,// 370 PAY 367 

    0xae80ca9e,// 371 PAY 368 

    0xe0a10ba3,// 372 PAY 369 

    0x971a8a5a,// 373 PAY 370 

    0xf82476f8,// 374 PAY 371 

    0x251037e7,// 375 PAY 372 

    0x1550b3df,// 376 PAY 373 

    0x461ec4aa,// 377 PAY 374 

    0x43e66695,// 378 PAY 375 

    0x7d79dc66,// 379 PAY 376 

/// HASH is  12 bytes 

    0x6640dbc1,// 380 HSH   1 

    0x0624f108,// 381 HSH   2 

    0x23733b57,// 382 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 67 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd1 

    0x0168d143 // 383 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt94_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 29 words. 

/// BDA size     is 109 (0x6d) 

/// BDA id       is 0xda3e 

    0x006dda3e,// 3 BDA   1 

/// PAY Generic Data size   : 109 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xb27d72e5,// 4 PAY   1 

    0xfa6cd44d,// 5 PAY   2 

    0x3edab3ec,// 6 PAY   3 

    0xf737991d,// 7 PAY   4 

    0x9412aa63,// 8 PAY   5 

    0x8f1fc029,// 9 PAY   6 

    0x0f18a6e1,// 10 PAY   7 

    0x574b88d3,// 11 PAY   8 

    0x12f64404,// 12 PAY   9 

    0x1b51d2a2,// 13 PAY  10 

    0xf4b5590a,// 14 PAY  11 

    0xd52b0fe4,// 15 PAY  12 

    0x9ce658e6,// 16 PAY  13 

    0xefa75a14,// 17 PAY  14 

    0xba60ce52,// 18 PAY  15 

    0x37ea34fd,// 19 PAY  16 

    0x6d9ae3cb,// 20 PAY  17 

    0xfb18e0eb,// 21 PAY  18 

    0x3c7e64c3,// 22 PAY  19 

    0xab3a35a8,// 23 PAY  20 

    0x6af6826c,// 24 PAY  21 

    0x67da6bf6,// 25 PAY  22 

    0x1d12f200,// 26 PAY  23 

    0x173442b1,// 27 PAY  24 

    0xc29bf154,// 28 PAY  25 

    0xfdd0c00a,// 29 PAY  26 

    0x7b379c49,// 30 PAY  27 

    0xf0000000,// 31 PAY  28 

/// STA is 1 words. 

/// STA num_pkts       : 175 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc9 

    0x005dc9af // 32 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt95_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 186 words. 

/// BDA size     is 740 (0x2e4) 

/// BDA id       is 0x6c9d 

    0x02e46c9d,// 3 BDA   1 

/// PAY Generic Data size   : 740 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x3cf3c0aa,// 4 PAY   1 

    0x4d7607fa,// 5 PAY   2 

    0xea1582d7,// 6 PAY   3 

    0x75725f3b,// 7 PAY   4 

    0x2fe09295,// 8 PAY   5 

    0xf5e8c9f1,// 9 PAY   6 

    0xfd689736,// 10 PAY   7 

    0xf5d6f6e8,// 11 PAY   8 

    0x4f4cbfad,// 12 PAY   9 

    0x8ec12151,// 13 PAY  10 

    0xdf9cde60,// 14 PAY  11 

    0xca59eb70,// 15 PAY  12 

    0x5b968e29,// 16 PAY  13 

    0xb711feaa,// 17 PAY  14 

    0x07d30ac3,// 18 PAY  15 

    0xcfb69af4,// 19 PAY  16 

    0x87b36797,// 20 PAY  17 

    0xd5b688ee,// 21 PAY  18 

    0x89864a8d,// 22 PAY  19 

    0xe91e3d90,// 23 PAY  20 

    0xa3ca43fb,// 24 PAY  21 

    0xac149ac4,// 25 PAY  22 

    0xc7dc8d76,// 26 PAY  23 

    0x93b86d8a,// 27 PAY  24 

    0x2d4ffb9b,// 28 PAY  25 

    0x59c6b1f7,// 29 PAY  26 

    0x2fd5d572,// 30 PAY  27 

    0x51b85382,// 31 PAY  28 

    0xccc6fae5,// 32 PAY  29 

    0x84903b09,// 33 PAY  30 

    0xce8387ca,// 34 PAY  31 

    0xac0bce93,// 35 PAY  32 

    0xd948640b,// 36 PAY  33 

    0x0c9dbabe,// 37 PAY  34 

    0xad07bf5c,// 38 PAY  35 

    0xdc5ab6d7,// 39 PAY  36 

    0xde096a14,// 40 PAY  37 

    0x15766b40,// 41 PAY  38 

    0x0273b9dd,// 42 PAY  39 

    0x3f24c974,// 43 PAY  40 

    0x2345df30,// 44 PAY  41 

    0xecbc6135,// 45 PAY  42 

    0xfd42913a,// 46 PAY  43 

    0x7f0cfa1c,// 47 PAY  44 

    0xb6804090,// 48 PAY  45 

    0xd208e100,// 49 PAY  46 

    0xb3d7c932,// 50 PAY  47 

    0x7d067fc1,// 51 PAY  48 

    0x1fa81bbb,// 52 PAY  49 

    0x10bdc03a,// 53 PAY  50 

    0xb09f0967,// 54 PAY  51 

    0xafb75c4e,// 55 PAY  52 

    0xaa29cca1,// 56 PAY  53 

    0x87a766d5,// 57 PAY  54 

    0xc26a7905,// 58 PAY  55 

    0x651a0843,// 59 PAY  56 

    0xf2fbf034,// 60 PAY  57 

    0x80b42b6d,// 61 PAY  58 

    0x180c620f,// 62 PAY  59 

    0x38df75f8,// 63 PAY  60 

    0x8e5193d5,// 64 PAY  61 

    0x1013a0e3,// 65 PAY  62 

    0x5fa9de38,// 66 PAY  63 

    0xa0b804b8,// 67 PAY  64 

    0xd66dc13e,// 68 PAY  65 

    0x6963a5ce,// 69 PAY  66 

    0x7404faa2,// 70 PAY  67 

    0x666494c4,// 71 PAY  68 

    0xaf3e5c10,// 72 PAY  69 

    0x2c150ef3,// 73 PAY  70 

    0x7c766a36,// 74 PAY  71 

    0x042106e6,// 75 PAY  72 

    0x2524cebe,// 76 PAY  73 

    0x3b8b3306,// 77 PAY  74 

    0xc2974a9b,// 78 PAY  75 

    0x4dc65e23,// 79 PAY  76 

    0x6d93cfb6,// 80 PAY  77 

    0xea669958,// 81 PAY  78 

    0x84b327c9,// 82 PAY  79 

    0x2d97552a,// 83 PAY  80 

    0x4a61ea6a,// 84 PAY  81 

    0xd2446017,// 85 PAY  82 

    0xb67d3bd2,// 86 PAY  83 

    0x19fe1f0e,// 87 PAY  84 

    0xeb3ae20c,// 88 PAY  85 

    0xea5053df,// 89 PAY  86 

    0xc3caaedb,// 90 PAY  87 

    0x644dcc62,// 91 PAY  88 

    0x5304f7f1,// 92 PAY  89 

    0xb4cefe6c,// 93 PAY  90 

    0x1269df7a,// 94 PAY  91 

    0x3dc92c1b,// 95 PAY  92 

    0xebfe0106,// 96 PAY  93 

    0x2dd01cf3,// 97 PAY  94 

    0x5544c5c3,// 98 PAY  95 

    0x82ec90a1,// 99 PAY  96 

    0x4896162c,// 100 PAY  97 

    0xd8774b88,// 101 PAY  98 

    0xace8b8c9,// 102 PAY  99 

    0x838863d9,// 103 PAY 100 

    0xd0de58e5,// 104 PAY 101 

    0x29702e23,// 105 PAY 102 

    0x3bc330cf,// 106 PAY 103 

    0xb9412154,// 107 PAY 104 

    0xf52c441b,// 108 PAY 105 

    0x427d064a,// 109 PAY 106 

    0xced44640,// 110 PAY 107 

    0x59af7906,// 111 PAY 108 

    0x94568b57,// 112 PAY 109 

    0x5732edbf,// 113 PAY 110 

    0x5be74a04,// 114 PAY 111 

    0x835dd237,// 115 PAY 112 

    0xdf66b77d,// 116 PAY 113 

    0xd717949c,// 117 PAY 114 

    0xbd8aa672,// 118 PAY 115 

    0xcf66a3db,// 119 PAY 116 

    0x6d2dffd9,// 120 PAY 117 

    0xc5be0bae,// 121 PAY 118 

    0x85c3b8eb,// 122 PAY 119 

    0x25f1bc1f,// 123 PAY 120 

    0xbe99d26c,// 124 PAY 121 

    0x2f95b3d4,// 125 PAY 122 

    0x473b7103,// 126 PAY 123 

    0x093079d0,// 127 PAY 124 

    0x766f6094,// 128 PAY 125 

    0xf4790f27,// 129 PAY 126 

    0xcacabf01,// 130 PAY 127 

    0xec472a8b,// 131 PAY 128 

    0xb334bf3c,// 132 PAY 129 

    0x35502e9e,// 133 PAY 130 

    0x3402b5aa,// 134 PAY 131 

    0x31c11f89,// 135 PAY 132 

    0x89300aa5,// 136 PAY 133 

    0x4985b46c,// 137 PAY 134 

    0xcd3cf4c6,// 138 PAY 135 

    0x4004c4f7,// 139 PAY 136 

    0x6a054f3f,// 140 PAY 137 

    0x35834fa5,// 141 PAY 138 

    0x2dc7a057,// 142 PAY 139 

    0x4d621ce5,// 143 PAY 140 

    0x460433ba,// 144 PAY 141 

    0x289140ad,// 145 PAY 142 

    0x1bcfdb94,// 146 PAY 143 

    0x8ca185f6,// 147 PAY 144 

    0xf7e9f6e0,// 148 PAY 145 

    0x1ec7e018,// 149 PAY 146 

    0xf78f7fbf,// 150 PAY 147 

    0x60f37eb5,// 151 PAY 148 

    0xc8971f88,// 152 PAY 149 

    0x60882344,// 153 PAY 150 

    0x22b77518,// 154 PAY 151 

    0x0744c6a7,// 155 PAY 152 

    0x875756a4,// 156 PAY 153 

    0x0951d18a,// 157 PAY 154 

    0x6b453f98,// 158 PAY 155 

    0xda1ffb02,// 159 PAY 156 

    0xf8b0bf0d,// 160 PAY 157 

    0xc6a35fd5,// 161 PAY 158 

    0xf3a56447,// 162 PAY 159 

    0xcd6fee7b,// 163 PAY 160 

    0xa071f5ad,// 164 PAY 161 

    0xce2ed66f,// 165 PAY 162 

    0x7fb4c49d,// 166 PAY 163 

    0x536562f0,// 167 PAY 164 

    0xcb629963,// 168 PAY 165 

    0x54db0974,// 169 PAY 166 

    0x541d29a8,// 170 PAY 167 

    0xca16d9c2,// 171 PAY 168 

    0x925b347c,// 172 PAY 169 

    0x4f4b8d9d,// 173 PAY 170 

    0x3f542b81,// 174 PAY 171 

    0xf2a3420b,// 175 PAY 172 

    0xfd0b7d69,// 176 PAY 173 

    0xaf8e49c6,// 177 PAY 174 

    0x6b722093,// 178 PAY 175 

    0x6dab3b54,// 179 PAY 176 

    0xe6b49407,// 180 PAY 177 

    0x93c70f8c,// 181 PAY 178 

    0x3ad10393,// 182 PAY 179 

    0x23db3986,// 183 PAY 180 

    0x9ab7a1d0,// 184 PAY 181 

    0x3339240c,// 185 PAY 182 

    0xeeee3e0e,// 186 PAY 183 

    0x18897f08,// 187 PAY 184 

    0x94ec2c96,// 188 PAY 185 

/// STA is 1 words. 

/// STA num_pkts       : 93 

/// STA pkt_idx        : 87 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe2 

    0x015ce25d // 189 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt96_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 51 words. 

/// BDA size     is 198 (0xc6) 

/// BDA id       is 0xc09b 

    0x00c6c09b,// 3 BDA   1 

/// PAY Generic Data size   : 198 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xf558a42e,// 4 PAY   1 

    0x39978b18,// 5 PAY   2 

    0x37ea64f7,// 6 PAY   3 

    0x95a60206,// 7 PAY   4 

    0x3fe9f422,// 8 PAY   5 

    0xb6692768,// 9 PAY   6 

    0xe9b3a7dd,// 10 PAY   7 

    0x2964265a,// 11 PAY   8 

    0xc210f297,// 12 PAY   9 

    0x4f6123ff,// 13 PAY  10 

    0xdc658091,// 14 PAY  11 

    0x54dc153d,// 15 PAY  12 

    0x30fafbde,// 16 PAY  13 

    0x621c1fca,// 17 PAY  14 

    0x32228d6f,// 18 PAY  15 

    0x96df5b4c,// 19 PAY  16 

    0xc835ad6a,// 20 PAY  17 

    0x978ed06a,// 21 PAY  18 

    0x53f675b4,// 22 PAY  19 

    0x5d3d7b46,// 23 PAY  20 

    0x941c7b14,// 24 PAY  21 

    0xf274383c,// 25 PAY  22 

    0xf6f53a18,// 26 PAY  23 

    0xe51029e5,// 27 PAY  24 

    0x2cf42bc4,// 28 PAY  25 

    0xc899866a,// 29 PAY  26 

    0x42461422,// 30 PAY  27 

    0xb74d53f6,// 31 PAY  28 

    0x4dc69d14,// 32 PAY  29 

    0x59e87b1b,// 33 PAY  30 

    0xc72d7afc,// 34 PAY  31 

    0x3d76c708,// 35 PAY  32 

    0xe8f70c7f,// 36 PAY  33 

    0x3247afff,// 37 PAY  34 

    0xee02d100,// 38 PAY  35 

    0x1707c335,// 39 PAY  36 

    0xa168328b,// 40 PAY  37 

    0x57e16ac8,// 41 PAY  38 

    0x63a4a686,// 42 PAY  39 

    0x0a7c6009,// 43 PAY  40 

    0x66573c93,// 44 PAY  41 

    0x9dcf3aef,// 45 PAY  42 

    0xfda96529,// 46 PAY  43 

    0x219140a6,// 47 PAY  44 

    0x6339b3d7,// 48 PAY  45 

    0x81835241,// 49 PAY  46 

    0x41fd66ec,// 50 PAY  47 

    0xcbc612e7,// 51 PAY  48 

    0x1ea8c74a,// 52 PAY  49 

    0x65e60000,// 53 PAY  50 

/// HASH is  12 bytes 

    0x0a7c6009,// 54 HSH   1 

    0x66573c93,// 55 HSH   2 

    0x9dcf3aef,// 56 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 201 

/// STA pkt_idx        : 14 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5 

    0x003905c9 // 57 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt97_tmpl[] = {
    0x0c010058,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 321 words. 

/// BDA size     is 1278 (0x4fe) 

/// BDA id       is 0x2790 

    0x04fe2790,// 3 BDA   1 

/// PAY Generic Data size   : 1278 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xc117737c,// 4 PAY   1 

    0x0021eb6c,// 5 PAY   2 

    0xd1e5ee18,// 6 PAY   3 

    0xa61ccd8d,// 7 PAY   4 

    0x6dfa57d1,// 8 PAY   5 

    0xd8803e78,// 9 PAY   6 

    0x86309f9f,// 10 PAY   7 

    0x5908f7d0,// 11 PAY   8 

    0x23037919,// 12 PAY   9 

    0x59ae0524,// 13 PAY  10 

    0x3d4f7064,// 14 PAY  11 

    0xbec04593,// 15 PAY  12 

    0xaca216ca,// 16 PAY  13 

    0x28dc3e5a,// 17 PAY  14 

    0x43aad52b,// 18 PAY  15 

    0xb0f07d56,// 19 PAY  16 

    0x30795180,// 20 PAY  17 

    0x34ef9e35,// 21 PAY  18 

    0xf2623ccb,// 22 PAY  19 

    0x9fa5c46a,// 23 PAY  20 

    0x8fd14981,// 24 PAY  21 

    0x025800ad,// 25 PAY  22 

    0xeaede4c0,// 26 PAY  23 

    0x27f6bb5a,// 27 PAY  24 

    0x4ff3523c,// 28 PAY  25 

    0x8040f667,// 29 PAY  26 

    0x12b9011a,// 30 PAY  27 

    0x9794f4ee,// 31 PAY  28 

    0xf5cbbded,// 32 PAY  29 

    0x8bc2f60d,// 33 PAY  30 

    0x84a4cea9,// 34 PAY  31 

    0x5ffeceea,// 35 PAY  32 

    0x0e55308c,// 36 PAY  33 

    0x69535c1c,// 37 PAY  34 

    0x63fdf839,// 38 PAY  35 

    0x91b12173,// 39 PAY  36 

    0xed388a2a,// 40 PAY  37 

    0x2d63e11d,// 41 PAY  38 

    0x11c19c89,// 42 PAY  39 

    0xbb7bcd25,// 43 PAY  40 

    0xf19784ea,// 44 PAY  41 

    0xa400b2da,// 45 PAY  42 

    0x6b48b449,// 46 PAY  43 

    0x8925b08e,// 47 PAY  44 

    0xd1b1cb14,// 48 PAY  45 

    0x1a435951,// 49 PAY  46 

    0x266661fe,// 50 PAY  47 

    0xffa78077,// 51 PAY  48 

    0x61615b2c,// 52 PAY  49 

    0x55f29be9,// 53 PAY  50 

    0x69b43a9c,// 54 PAY  51 

    0x8b1584aa,// 55 PAY  52 

    0xdd2324bb,// 56 PAY  53 

    0x3bc4fa5d,// 57 PAY  54 

    0x9f6c4f2a,// 58 PAY  55 

    0x130c69be,// 59 PAY  56 

    0x468d9ee2,// 60 PAY  57 

    0xa0e74101,// 61 PAY  58 

    0x7beffb68,// 62 PAY  59 

    0xebba7165,// 63 PAY  60 

    0x074b6b60,// 64 PAY  61 

    0x6ea6e401,// 65 PAY  62 

    0x2c62eb9e,// 66 PAY  63 

    0x5f81e5d6,// 67 PAY  64 

    0xbb5a4f77,// 68 PAY  65 

    0x93ae6646,// 69 PAY  66 

    0xdcd3d63e,// 70 PAY  67 

    0xa5a40c64,// 71 PAY  68 

    0x96dda0af,// 72 PAY  69 

    0xde7251b2,// 73 PAY  70 

    0x4b572b03,// 74 PAY  71 

    0x23d22e3e,// 75 PAY  72 

    0x4c72183a,// 76 PAY  73 

    0xe40ff4c3,// 77 PAY  74 

    0x6a86a3a9,// 78 PAY  75 

    0x4ae5eda1,// 79 PAY  76 

    0x3975d5c0,// 80 PAY  77 

    0x053fdc6b,// 81 PAY  78 

    0x6424f42e,// 82 PAY  79 

    0x3fe66582,// 83 PAY  80 

    0xe866732d,// 84 PAY  81 

    0xa2030fcc,// 85 PAY  82 

    0x7e1d8c78,// 86 PAY  83 

    0x4335fb2f,// 87 PAY  84 

    0x4237ebcb,// 88 PAY  85 

    0xb3c6329b,// 89 PAY  86 

    0x78dcc626,// 90 PAY  87 

    0xa9926a9a,// 91 PAY  88 

    0x874dc714,// 92 PAY  89 

    0x9c5c79c1,// 93 PAY  90 

    0x75b14c58,// 94 PAY  91 

    0xbb5a8288,// 95 PAY  92 

    0x856fcb6c,// 96 PAY  93 

    0x079b00e4,// 97 PAY  94 

    0x0f26dff0,// 98 PAY  95 

    0xc9334ac7,// 99 PAY  96 

    0xf11a1105,// 100 PAY  97 

    0x1d7b3577,// 101 PAY  98 

    0x3e10f5f2,// 102 PAY  99 

    0x8a8e547b,// 103 PAY 100 

    0x5d407e61,// 104 PAY 101 

    0x6c90398b,// 105 PAY 102 

    0x078394fb,// 106 PAY 103 

    0xbb802e08,// 107 PAY 104 

    0x5fc46260,// 108 PAY 105 

    0xc24b4f91,// 109 PAY 106 

    0xf2db1983,// 110 PAY 107 

    0x63a6e3a9,// 111 PAY 108 

    0x85e7d1cb,// 112 PAY 109 

    0xa972f4ea,// 113 PAY 110 

    0x81817d80,// 114 PAY 111 

    0xf94fe164,// 115 PAY 112 

    0x0be5bcf7,// 116 PAY 113 

    0xdb2d4b5c,// 117 PAY 114 

    0xcfb42f78,// 118 PAY 115 

    0x9d3d6c7e,// 119 PAY 116 

    0x8705d39a,// 120 PAY 117 

    0x247ff85c,// 121 PAY 118 

    0x3f7fa8c0,// 122 PAY 119 

    0x0864fa9d,// 123 PAY 120 

    0xbd9b2839,// 124 PAY 121 

    0x9917d747,// 125 PAY 122 

    0xc86d3010,// 126 PAY 123 

    0xcfbd64ef,// 127 PAY 124 

    0x59cf2584,// 128 PAY 125 

    0xb8a6f71d,// 129 PAY 126 

    0xf1a5d62a,// 130 PAY 127 

    0x275a990c,// 131 PAY 128 

    0xe8d3596f,// 132 PAY 129 

    0x2e1211a4,// 133 PAY 130 

    0x03c7052f,// 134 PAY 131 

    0xa66c2d4f,// 135 PAY 132 

    0xaec26415,// 136 PAY 133 

    0xce63b253,// 137 PAY 134 

    0x3f5278ec,// 138 PAY 135 

    0x079bc2dd,// 139 PAY 136 

    0x95f0f710,// 140 PAY 137 

    0x709c4417,// 141 PAY 138 

    0x817dc7c7,// 142 PAY 139 

    0x2ccdd4e0,// 143 PAY 140 

    0x9e6a301b,// 144 PAY 141 

    0xa8563bbd,// 145 PAY 142 

    0x9621f480,// 146 PAY 143 

    0xea120ae5,// 147 PAY 144 

    0x0e2fc92c,// 148 PAY 145 

    0x918a6bb5,// 149 PAY 146 

    0x59e893da,// 150 PAY 147 

    0x6c169d7a,// 151 PAY 148 

    0x513c1512,// 152 PAY 149 

    0x15c59cf0,// 153 PAY 150 

    0x1dabcde4,// 154 PAY 151 

    0x28296ae7,// 155 PAY 152 

    0x2bfc9a0a,// 156 PAY 153 

    0x54e15155,// 157 PAY 154 

    0x2cd65d46,// 158 PAY 155 

    0x362189b3,// 159 PAY 156 

    0x0b2e21b1,// 160 PAY 157 

    0xf94e6cae,// 161 PAY 158 

    0x1b1ec798,// 162 PAY 159 

    0xd22ca30a,// 163 PAY 160 

    0xc62aee10,// 164 PAY 161 

    0x04ab49e0,// 165 PAY 162 

    0xc704eee9,// 166 PAY 163 

    0xebf5f70f,// 167 PAY 164 

    0x3be7e70f,// 168 PAY 165 

    0x05ab2e53,// 169 PAY 166 

    0xa7522b5c,// 170 PAY 167 

    0x5c34ac18,// 171 PAY 168 

    0x672b6edf,// 172 PAY 169 

    0xb7dd6bf8,// 173 PAY 170 

    0xbb586d08,// 174 PAY 171 

    0xdc822601,// 175 PAY 172 

    0xb96686e0,// 176 PAY 173 

    0x342de05a,// 177 PAY 174 

    0x994910b8,// 178 PAY 175 

    0x958f5f55,// 179 PAY 176 

    0xa06d6980,// 180 PAY 177 

    0xebb92a24,// 181 PAY 178 

    0x4a0d4b09,// 182 PAY 179 

    0xcd9fe585,// 183 PAY 180 

    0x665570c5,// 184 PAY 181 

    0xe78b8601,// 185 PAY 182 

    0x711e8d02,// 186 PAY 183 

    0xc7a8b1e1,// 187 PAY 184 

    0x9707a930,// 188 PAY 185 

    0x27fbde6c,// 189 PAY 186 

    0xa96df568,// 190 PAY 187 

    0xf797ffef,// 191 PAY 188 

    0x8852c913,// 192 PAY 189 

    0xb011f62d,// 193 PAY 190 

    0x96096288,// 194 PAY 191 

    0xeed6535d,// 195 PAY 192 

    0xeec49843,// 196 PAY 193 

    0x462b6aef,// 197 PAY 194 

    0xf21005c2,// 198 PAY 195 

    0x51b5c14e,// 199 PAY 196 

    0x72f34155,// 200 PAY 197 

    0xc5997649,// 201 PAY 198 

    0xeeba246d,// 202 PAY 199 

    0x43bf53a3,// 203 PAY 200 

    0x633b06cb,// 204 PAY 201 

    0x2ab6793a,// 205 PAY 202 

    0xaebe69d2,// 206 PAY 203 

    0x294ae86a,// 207 PAY 204 

    0x7509786b,// 208 PAY 205 

    0x71da6a5f,// 209 PAY 206 

    0x820dd757,// 210 PAY 207 

    0x8efd4157,// 211 PAY 208 

    0x255ec702,// 212 PAY 209 

    0x50f70f6c,// 213 PAY 210 

    0xd78d6b4f,// 214 PAY 211 

    0x5844c2e3,// 215 PAY 212 

    0xe97e8378,// 216 PAY 213 

    0x1b3575d9,// 217 PAY 214 

    0x9980dad5,// 218 PAY 215 

    0x51314b69,// 219 PAY 216 

    0x704eae66,// 220 PAY 217 

    0xc48e6e6e,// 221 PAY 218 

    0x30223e81,// 222 PAY 219 

    0x19b2a71f,// 223 PAY 220 

    0xad928302,// 224 PAY 221 

    0x3526df59,// 225 PAY 222 

    0x7a5f3ffa,// 226 PAY 223 

    0x0477c66c,// 227 PAY 224 

    0x9f83f8df,// 228 PAY 225 

    0xcd62dc67,// 229 PAY 226 

    0x262f63cf,// 230 PAY 227 

    0x59c69077,// 231 PAY 228 

    0x81ce7b21,// 232 PAY 229 

    0xe0cbecd4,// 233 PAY 230 

    0x41d1671d,// 234 PAY 231 

    0x4d412e78,// 235 PAY 232 

    0x87e848b3,// 236 PAY 233 

    0x28b8dfda,// 237 PAY 234 

    0x6874092b,// 238 PAY 235 

    0xbf5c4b72,// 239 PAY 236 

    0x78da3893,// 240 PAY 237 

    0xa650f0b5,// 241 PAY 238 

    0xdca81a4c,// 242 PAY 239 

    0x1df80e21,// 243 PAY 240 

    0x401568af,// 244 PAY 241 

    0x40822aa1,// 245 PAY 242 

    0x4c2eb5b0,// 246 PAY 243 

    0x9073f9c6,// 247 PAY 244 

    0x002edeb8,// 248 PAY 245 

    0x17b7c9e6,// 249 PAY 246 

    0xd1431a65,// 250 PAY 247 

    0xbfdccc4c,// 251 PAY 248 

    0x3d5c2c04,// 252 PAY 249 

    0xe1964a3e,// 253 PAY 250 

    0x7844899f,// 254 PAY 251 

    0x1579ad2d,// 255 PAY 252 

    0x364347cb,// 256 PAY 253 

    0xd32f799e,// 257 PAY 254 

    0x75f4d7e4,// 258 PAY 255 

    0x51cf23f8,// 259 PAY 256 

    0xbc054ad6,// 260 PAY 257 

    0xe55aa3ad,// 261 PAY 258 

    0x77038c1b,// 262 PAY 259 

    0x351a1c39,// 263 PAY 260 

    0x10c4754a,// 264 PAY 261 

    0xd73bf5e5,// 265 PAY 262 

    0x3350bd0c,// 266 PAY 263 

    0x755e6959,// 267 PAY 264 

    0x207e4132,// 268 PAY 265 

    0xe9de34cf,// 269 PAY 266 

    0x9cbff1c1,// 270 PAY 267 

    0x796fd009,// 271 PAY 268 

    0x92783581,// 272 PAY 269 

    0xa768039b,// 273 PAY 270 

    0x2da28d62,// 274 PAY 271 

    0x6d7bf262,// 275 PAY 272 

    0x2d2cc291,// 276 PAY 273 

    0xe1a92c22,// 277 PAY 274 

    0xcd1e52a3,// 278 PAY 275 

    0x38730e28,// 279 PAY 276 

    0x409df094,// 280 PAY 277 

    0xd07af775,// 281 PAY 278 

    0x2a151ebc,// 282 PAY 279 

    0xa9bbfb9d,// 283 PAY 280 

    0xab582fa3,// 284 PAY 281 

    0xaab23634,// 285 PAY 282 

    0xb3e7af71,// 286 PAY 283 

    0x8ee02335,// 287 PAY 284 

    0xe2a4c73f,// 288 PAY 285 

    0xa0083326,// 289 PAY 286 

    0xbf600511,// 290 PAY 287 

    0xd190d5dd,// 291 PAY 288 

    0xea935a76,// 292 PAY 289 

    0x4be90a10,// 293 PAY 290 

    0xfbdefb43,// 294 PAY 291 

    0xa4835f08,// 295 PAY 292 

    0x077eec1f,// 296 PAY 293 

    0xfd22ca9b,// 297 PAY 294 

    0x2c3580b6,// 298 PAY 295 

    0xa53493e0,// 299 PAY 296 

    0x1ab16ccc,// 300 PAY 297 

    0x8a9091fc,// 301 PAY 298 

    0xa771530a,// 302 PAY 299 

    0x22279442,// 303 PAY 300 

    0x78cef0e0,// 304 PAY 301 

    0x22827f48,// 305 PAY 302 

    0xbc111823,// 306 PAY 303 

    0xdfa3507c,// 307 PAY 304 

    0xaf1fc1ee,// 308 PAY 305 

    0x0e580f77,// 309 PAY 306 

    0xae956dd2,// 310 PAY 307 

    0xeedbc5f2,// 311 PAY 308 

    0x6adcf35c,// 312 PAY 309 

    0x88903de0,// 313 PAY 310 

    0x1ac6507f,// 314 PAY 311 

    0xc514504a,// 315 PAY 312 

    0x1508b5ff,// 316 PAY 313 

    0x82bead3c,// 317 PAY 314 

    0x2a338fe8,// 318 PAY 315 

    0x9522a48a,// 319 PAY 316 

    0xbe3173b8,// 320 PAY 317 

    0x04783bcd,// 321 PAY 318 

    0x18afedd6,// 322 PAY 319 

    0x059b0000,// 323 PAY 320 

/// HASH is  12 bytes 

    0xebba7165,// 324 HSH   1 

    0x074b6b60,// 325 HSH   2 

    0x6ea6e401,// 326 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 248 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbf 

    0x03e1bf74 // 327 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt98_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 173 words. 

/// BDA size     is 687 (0x2af) 

/// BDA id       is 0xf658 

    0x02aff658,// 3 BDA   1 

/// PAY Generic Data size   : 687 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x73070f50,// 4 PAY   1 

    0x63b5ef63,// 5 PAY   2 

    0x82a21d91,// 6 PAY   3 

    0x99fb6f68,// 7 PAY   4 

    0xd327f8c9,// 8 PAY   5 

    0x56f950ec,// 9 PAY   6 

    0xb24467e6,// 10 PAY   7 

    0x49236835,// 11 PAY   8 

    0xe2d1e8c4,// 12 PAY   9 

    0xa06225a3,// 13 PAY  10 

    0x8a3dd063,// 14 PAY  11 

    0xe954063f,// 15 PAY  12 

    0xd0ed8ebe,// 16 PAY  13 

    0xd4544d9a,// 17 PAY  14 

    0xa94de1da,// 18 PAY  15 

    0xef7a444b,// 19 PAY  16 

    0x6a301621,// 20 PAY  17 

    0x64543b86,// 21 PAY  18 

    0x637d5415,// 22 PAY  19 

    0x788cc451,// 23 PAY  20 

    0x5ce1b9a9,// 24 PAY  21 

    0xac7ed76c,// 25 PAY  22 

    0xd5dfc89b,// 26 PAY  23 

    0x720664a4,// 27 PAY  24 

    0x7ba4d45c,// 28 PAY  25 

    0x2a95f8a4,// 29 PAY  26 

    0xd72e7084,// 30 PAY  27 

    0x27ba4f3a,// 31 PAY  28 

    0x5afc5ee2,// 32 PAY  29 

    0x926818c3,// 33 PAY  30 

    0xc5f5bc6c,// 34 PAY  31 

    0x445c55cf,// 35 PAY  32 

    0x4a6aa1e5,// 36 PAY  33 

    0x08e13702,// 37 PAY  34 

    0x224e06c1,// 38 PAY  35 

    0xcb6ca93e,// 39 PAY  36 

    0x48689b0f,// 40 PAY  37 

    0x0b65dc4d,// 41 PAY  38 

    0x00a8abbb,// 42 PAY  39 

    0xa38c78a9,// 43 PAY  40 

    0xf6298991,// 44 PAY  41 

    0x132f0afd,// 45 PAY  42 

    0xf4ba7caf,// 46 PAY  43 

    0x2096bb1d,// 47 PAY  44 

    0xd3e5e50b,// 48 PAY  45 

    0xece3ca4a,// 49 PAY  46 

    0xb335785a,// 50 PAY  47 

    0x6de512e5,// 51 PAY  48 

    0x82d3dc9b,// 52 PAY  49 

    0xc2d97132,// 53 PAY  50 

    0xe1bfad2f,// 54 PAY  51 

    0x47b7be17,// 55 PAY  52 

    0x550d50c4,// 56 PAY  53 

    0xe5cc104a,// 57 PAY  54 

    0xd1e96348,// 58 PAY  55 

    0x3d49a72b,// 59 PAY  56 

    0x842feb95,// 60 PAY  57 

    0xcf875182,// 61 PAY  58 

    0xdd5d1288,// 62 PAY  59 

    0x4026bb64,// 63 PAY  60 

    0x87a997cc,// 64 PAY  61 

    0xb6b21666,// 65 PAY  62 

    0x1a233c1e,// 66 PAY  63 

    0x15c245fa,// 67 PAY  64 

    0xfc43a293,// 68 PAY  65 

    0xdb1f14b4,// 69 PAY  66 

    0xe0b29b25,// 70 PAY  67 

    0x57e8282e,// 71 PAY  68 

    0xce681376,// 72 PAY  69 

    0x0bf5b313,// 73 PAY  70 

    0x524f79d3,// 74 PAY  71 

    0xf55cde18,// 75 PAY  72 

    0x80a5f6e7,// 76 PAY  73 

    0x26b8887b,// 77 PAY  74 

    0x89b98ee2,// 78 PAY  75 

    0xc0f21a4b,// 79 PAY  76 

    0x45959653,// 80 PAY  77 

    0x0197c66a,// 81 PAY  78 

    0xba7751fd,// 82 PAY  79 

    0x56a64e38,// 83 PAY  80 

    0x56c5005a,// 84 PAY  81 

    0x5068a5b2,// 85 PAY  82 

    0xa02be3d3,// 86 PAY  83 

    0x06e3aa75,// 87 PAY  84 

    0xdf5f1031,// 88 PAY  85 

    0x38246149,// 89 PAY  86 

    0xe1939566,// 90 PAY  87 

    0x34ac2e54,// 91 PAY  88 

    0xf42ee62e,// 92 PAY  89 

    0xb8c57e18,// 93 PAY  90 

    0x2f69afcf,// 94 PAY  91 

    0x526c9fcd,// 95 PAY  92 

    0xcf039ba0,// 96 PAY  93 

    0xa6bbcffe,// 97 PAY  94 

    0x9fa81faf,// 98 PAY  95 

    0xfa242226,// 99 PAY  96 

    0x79c983e6,// 100 PAY  97 

    0xea62d006,// 101 PAY  98 

    0xfe62fb2c,// 102 PAY  99 

    0x0a70190a,// 103 PAY 100 

    0xe10b80de,// 104 PAY 101 

    0xc88d2216,// 105 PAY 102 

    0x49b71399,// 106 PAY 103 

    0x5c78e03a,// 107 PAY 104 

    0x82773695,// 108 PAY 105 

    0x1ac0f56d,// 109 PAY 106 

    0x88827575,// 110 PAY 107 

    0xa8214b80,// 111 PAY 108 

    0x7229573d,// 112 PAY 109 

    0x5822b7c4,// 113 PAY 110 

    0x28ae27fd,// 114 PAY 111 

    0x088b46f9,// 115 PAY 112 

    0xcf45375a,// 116 PAY 113 

    0xe14d917a,// 117 PAY 114 

    0x8550a9df,// 118 PAY 115 

    0xd5328970,// 119 PAY 116 

    0x0a4f4b9a,// 120 PAY 117 

    0x92ebdc99,// 121 PAY 118 

    0x0bd140d0,// 122 PAY 119 

    0xfa294cda,// 123 PAY 120 

    0x16a05efa,// 124 PAY 121 

    0x6dc4813a,// 125 PAY 122 

    0x35cb97b1,// 126 PAY 123 

    0x91e13f8e,// 127 PAY 124 

    0xffbf76ef,// 128 PAY 125 

    0xe5a908b2,// 129 PAY 126 

    0x30cc63da,// 130 PAY 127 

    0x000885df,// 131 PAY 128 

    0x57e08859,// 132 PAY 129 

    0x3ab70bb5,// 133 PAY 130 

    0x27dbc399,// 134 PAY 131 

    0xeb513ad1,// 135 PAY 132 

    0x1a7cb952,// 136 PAY 133 

    0x5f8d57c0,// 137 PAY 134 

    0xf147045b,// 138 PAY 135 

    0x10c77a57,// 139 PAY 136 

    0xa15ac79f,// 140 PAY 137 

    0x67e20d65,// 141 PAY 138 

    0x1604a2c8,// 142 PAY 139 

    0x26e95a61,// 143 PAY 140 

    0x00d58f36,// 144 PAY 141 

    0xaa13d577,// 145 PAY 142 

    0xf0444d2a,// 146 PAY 143 

    0x940e1598,// 147 PAY 144 

    0x768ddeaa,// 148 PAY 145 

    0xa92fe36f,// 149 PAY 146 

    0xc73a7324,// 150 PAY 147 

    0x568889ba,// 151 PAY 148 

    0x5c42f048,// 152 PAY 149 

    0xf2214022,// 153 PAY 150 

    0x0a920082,// 154 PAY 151 

    0x6833f7c5,// 155 PAY 152 

    0xcf896962,// 156 PAY 153 

    0xa35315dd,// 157 PAY 154 

    0xc1d740e1,// 158 PAY 155 

    0x2e23fd8d,// 159 PAY 156 

    0x287aaaa4,// 160 PAY 157 

    0x69303cac,// 161 PAY 158 

    0xa4936792,// 162 PAY 159 

    0x16ebe7a9,// 163 PAY 160 

    0x4c6eb84f,// 164 PAY 161 

    0xb0bd7fc9,// 165 PAY 162 

    0xf7987eb5,// 166 PAY 163 

    0xd93723bc,// 167 PAY 164 

    0x04803ef0,// 168 PAY 165 

    0x3426a841,// 169 PAY 166 

    0x540e891c,// 170 PAY 167 

    0x50b9dce1,// 171 PAY 168 

    0xf26dd7fe,// 172 PAY 169 

    0x4137e680,// 173 PAY 170 

    0xf4203463,// 174 PAY 171 

    0xc5472400,// 175 PAY 172 

/// STA is 1 words. 

/// STA num_pkts       : 65 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8 

    0x02b40841 // 176 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 88 (0x58) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_aes_sha1_pkt99_tmpl[] = {
    0x08010058,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 411 words. 

/// BDA size     is 1637 (0x665) 

/// BDA id       is 0xb4f7 

    0x0665b4f7,// 3 BDA   1 

/// PAY Generic Data size   : 1637 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x138dadcc,// 4 PAY   1 

    0xeccc332e,// 5 PAY   2 

    0x82df3ee0,// 6 PAY   3 

    0xca8b7ce7,// 7 PAY   4 

    0x877bc784,// 8 PAY   5 

    0xd6bf4874,// 9 PAY   6 

    0x0d7449b8,// 10 PAY   7 

    0x1ddab077,// 11 PAY   8 

    0x1f1695da,// 12 PAY   9 

    0x6bacd4b7,// 13 PAY  10 

    0x575a48eb,// 14 PAY  11 

    0xc9501dfd,// 15 PAY  12 

    0xde8dea91,// 16 PAY  13 

    0x8a0ba11f,// 17 PAY  14 

    0x9e8b58ea,// 18 PAY  15 

    0xad3fb74e,// 19 PAY  16 

    0x7adf36eb,// 20 PAY  17 

    0x4172da5c,// 21 PAY  18 

    0x50f2c1c5,// 22 PAY  19 

    0xa8ca2975,// 23 PAY  20 

    0xe57c8560,// 24 PAY  21 

    0xfb8b091d,// 25 PAY  22 

    0xd4583008,// 26 PAY  23 

    0x30cba9c1,// 27 PAY  24 

    0x67d116ed,// 28 PAY  25 

    0xb02fa1d3,// 29 PAY  26 

    0xa8657d4e,// 30 PAY  27 

    0x748cb8f4,// 31 PAY  28 

    0x352c9415,// 32 PAY  29 

    0xe6a0d6f6,// 33 PAY  30 

    0x34ec8a9e,// 34 PAY  31 

    0xff28f034,// 35 PAY  32 

    0xa4005a9b,// 36 PAY  33 

    0x81c3a4a3,// 37 PAY  34 

    0x97ebd006,// 38 PAY  35 

    0xd09008a1,// 39 PAY  36 

    0xded7120e,// 40 PAY  37 

    0x7e8c8e29,// 41 PAY  38 

    0x5e2887e4,// 42 PAY  39 

    0x49ae1614,// 43 PAY  40 

    0x90d6ed08,// 44 PAY  41 

    0xda8f1709,// 45 PAY  42 

    0x142060e5,// 46 PAY  43 

    0x422f8feb,// 47 PAY  44 

    0xbb652e1e,// 48 PAY  45 

    0x43844b46,// 49 PAY  46 

    0xfc34a0c0,// 50 PAY  47 

    0x1f420ac7,// 51 PAY  48 

    0xb6bf9f7f,// 52 PAY  49 

    0xb3427c57,// 53 PAY  50 

    0xd2c7981a,// 54 PAY  51 

    0xe97aacc4,// 55 PAY  52 

    0x4118a76c,// 56 PAY  53 

    0x4e0f75a0,// 57 PAY  54 

    0xb5f9b464,// 58 PAY  55 

    0x84181d62,// 59 PAY  56 

    0x3d24e1d3,// 60 PAY  57 

    0x46d6b102,// 61 PAY  58 

    0xdc336ef6,// 62 PAY  59 

    0xc2c50d32,// 63 PAY  60 

    0xbad52e55,// 64 PAY  61 

    0x43482524,// 65 PAY  62 

    0x75e6a7da,// 66 PAY  63 

    0xf094bc26,// 67 PAY  64 

    0xb0bafbd8,// 68 PAY  65 

    0x5814c6c9,// 69 PAY  66 

    0x8a8d4966,// 70 PAY  67 

    0x57ad6893,// 71 PAY  68 

    0x21c99cc1,// 72 PAY  69 

    0xf2a55dc0,// 73 PAY  70 

    0x75de5ffd,// 74 PAY  71 

    0xe856a2f7,// 75 PAY  72 

    0x8e582df8,// 76 PAY  73 

    0x649c3cd0,// 77 PAY  74 

    0xf97100ef,// 78 PAY  75 

    0xa245db29,// 79 PAY  76 

    0xb9ca7ac2,// 80 PAY  77 

    0x3ce71d1e,// 81 PAY  78 

    0x658f8a80,// 82 PAY  79 

    0x3693ae67,// 83 PAY  80 

    0x83bac970,// 84 PAY  81 

    0x8be0f463,// 85 PAY  82 

    0xdb6a3a5f,// 86 PAY  83 

    0x12f46ad3,// 87 PAY  84 

    0xd99861c5,// 88 PAY  85 

    0x0ba69190,// 89 PAY  86 

    0x63793009,// 90 PAY  87 

    0x524060c4,// 91 PAY  88 

    0x96e941d5,// 92 PAY  89 

    0xa6dc88d0,// 93 PAY  90 

    0xe0e07580,// 94 PAY  91 

    0xcd232e10,// 95 PAY  92 

    0x4d1bdb2a,// 96 PAY  93 

    0xd2fc3f4e,// 97 PAY  94 

    0xa44af6f3,// 98 PAY  95 

    0x84d1751c,// 99 PAY  96 

    0x0c594b12,// 100 PAY  97 

    0x3eb8e65c,// 101 PAY  98 

    0x82f2eb8d,// 102 PAY  99 

    0xb3c97988,// 103 PAY 100 

    0xf365c076,// 104 PAY 101 

    0x1c20e940,// 105 PAY 102 

    0xf49f40ac,// 106 PAY 103 

    0xaba1c7d9,// 107 PAY 104 

    0xd796a9e3,// 108 PAY 105 

    0xfee1fa3c,// 109 PAY 106 

    0x2d1c6011,// 110 PAY 107 

    0xe5659470,// 111 PAY 108 

    0xbdb6f87b,// 112 PAY 109 

    0x80e17903,// 113 PAY 110 

    0x077ccf19,// 114 PAY 111 

    0xa65743b3,// 115 PAY 112 

    0x2d80c232,// 116 PAY 113 

    0x95ff373d,// 117 PAY 114 

    0xe80cf547,// 118 PAY 115 

    0x689e396a,// 119 PAY 116 

    0x23fecd78,// 120 PAY 117 

    0xa1058259,// 121 PAY 118 

    0x90bc80ef,// 122 PAY 119 

    0xe647e02a,// 123 PAY 120 

    0x5755915c,// 124 PAY 121 

    0x5a463c78,// 125 PAY 122 

    0x1f7bbc76,// 126 PAY 123 

    0x8a121896,// 127 PAY 124 

    0x41dbccfb,// 128 PAY 125 

    0x53375faf,// 129 PAY 126 

    0xf12c0ced,// 130 PAY 127 

    0x3c574360,// 131 PAY 128 

    0x0bbd702f,// 132 PAY 129 

    0x913400fe,// 133 PAY 130 

    0x69e0dbff,// 134 PAY 131 

    0x30017404,// 135 PAY 132 

    0xf43e2d6e,// 136 PAY 133 

    0xe4c53719,// 137 PAY 134 

    0x2b2c3893,// 138 PAY 135 

    0x5158af56,// 139 PAY 136 

    0x295e7502,// 140 PAY 137 

    0x3b055fe7,// 141 PAY 138 

    0xf77e3c4f,// 142 PAY 139 

    0x06f2b468,// 143 PAY 140 

    0x0332eb10,// 144 PAY 141 

    0xaef56b70,// 145 PAY 142 

    0x36821767,// 146 PAY 143 

    0xe9d5f2ee,// 147 PAY 144 

    0xcb25fbe6,// 148 PAY 145 

    0xf71be7d8,// 149 PAY 146 

    0xcc52a62b,// 150 PAY 147 

    0x6101902f,// 151 PAY 148 

    0xcbe9a035,// 152 PAY 149 

    0xe155cd16,// 153 PAY 150 

    0xece9e1bb,// 154 PAY 151 

    0xe2f54fe1,// 155 PAY 152 

    0x604c0eef,// 156 PAY 153 

    0xe8f7ad6b,// 157 PAY 154 

    0x0077e7c8,// 158 PAY 155 

    0x527057f3,// 159 PAY 156 

    0x4b38ca12,// 160 PAY 157 

    0xd501f84f,// 161 PAY 158 

    0x8b91551e,// 162 PAY 159 

    0x0b8afbe3,// 163 PAY 160 

    0x0a2bd3ab,// 164 PAY 161 

    0xfd847935,// 165 PAY 162 

    0xce48d122,// 166 PAY 163 

    0x7e36fdd8,// 167 PAY 164 

    0x740e5873,// 168 PAY 165 

    0xb3fc9c7f,// 169 PAY 166 

    0x90cd4a9e,// 170 PAY 167 

    0x9363f73e,// 171 PAY 168 

    0x7227daa0,// 172 PAY 169 

    0x75afadc7,// 173 PAY 170 

    0xa5f15dbb,// 174 PAY 171 

    0x5d9983bd,// 175 PAY 172 

    0xbdb2d6ae,// 176 PAY 173 

    0x973498e5,// 177 PAY 174 

    0xf337c2d4,// 178 PAY 175 

    0xacfc069c,// 179 PAY 176 

    0x4360bd79,// 180 PAY 177 

    0x3807eb1c,// 181 PAY 178 

    0x4c1d7388,// 182 PAY 179 

    0xb8591b31,// 183 PAY 180 

    0x14bf7a6c,// 184 PAY 181 

    0xb747f805,// 185 PAY 182 

    0x105a0965,// 186 PAY 183 

    0x94e6dde4,// 187 PAY 184 

    0x9860d57d,// 188 PAY 185 

    0xebb85321,// 189 PAY 186 

    0xa8a5b648,// 190 PAY 187 

    0x878eb8af,// 191 PAY 188 

    0x8546793f,// 192 PAY 189 

    0x7b93296c,// 193 PAY 190 

    0x73e59e29,// 194 PAY 191 

    0xd220dbd0,// 195 PAY 192 

    0xb6d4d1d8,// 196 PAY 193 

    0x859429b7,// 197 PAY 194 

    0x9239a833,// 198 PAY 195 

    0x5fd7c51a,// 199 PAY 196 

    0x2c323636,// 200 PAY 197 

    0x5f62f77e,// 201 PAY 198 

    0xc5f53b06,// 202 PAY 199 

    0x64d5a30a,// 203 PAY 200 

    0x0184e02c,// 204 PAY 201 

    0x90dbc747,// 205 PAY 202 

    0xa7df4a5c,// 206 PAY 203 

    0x36cc0499,// 207 PAY 204 

    0xaf4053fb,// 208 PAY 205 

    0x89a94f82,// 209 PAY 206 

    0xf8311585,// 210 PAY 207 

    0x68217f22,// 211 PAY 208 

    0xf942c7df,// 212 PAY 209 

    0x93353bd8,// 213 PAY 210 

    0x0bb95783,// 214 PAY 211 

    0x33329352,// 215 PAY 212 

    0xe296114e,// 216 PAY 213 

    0xbbc35248,// 217 PAY 214 

    0x10c6b54a,// 218 PAY 215 

    0xe4a1e6d5,// 219 PAY 216 

    0x68fbc065,// 220 PAY 217 

    0xc667d407,// 221 PAY 218 

    0xac267056,// 222 PAY 219 

    0x7273ae8c,// 223 PAY 220 

    0xbe48d815,// 224 PAY 221 

    0x47087ad9,// 225 PAY 222 

    0x770a3e20,// 226 PAY 223 

    0x5c5ff138,// 227 PAY 224 

    0x634e47e2,// 228 PAY 225 

    0x2adcd678,// 229 PAY 226 

    0x23d088a3,// 230 PAY 227 

    0x408b065c,// 231 PAY 228 

    0x8f940ad3,// 232 PAY 229 

    0x9fcb1452,// 233 PAY 230 

    0xe1632bb7,// 234 PAY 231 

    0x61f9fc0f,// 235 PAY 232 

    0x3902b81e,// 236 PAY 233 

    0x42956046,// 237 PAY 234 

    0x146acf09,// 238 PAY 235 

    0x36c6c894,// 239 PAY 236 

    0x4d245423,// 240 PAY 237 

    0xd243d37d,// 241 PAY 238 

    0x8e3155dc,// 242 PAY 239 

    0x5efe4daf,// 243 PAY 240 

    0xbc323563,// 244 PAY 241 

    0x921ebef6,// 245 PAY 242 

    0x23c9a6a1,// 246 PAY 243 

    0x594fee8d,// 247 PAY 244 

    0x9e7b4db6,// 248 PAY 245 

    0x03669830,// 249 PAY 246 

    0x80504543,// 250 PAY 247 

    0x82d7d5f2,// 251 PAY 248 

    0xef41fbf2,// 252 PAY 249 

    0x988d3684,// 253 PAY 250 

    0x8630e135,// 254 PAY 251 

    0xce0e51f2,// 255 PAY 252 

    0x9de90ce1,// 256 PAY 253 

    0xbc090e90,// 257 PAY 254 

    0x6a870e06,// 258 PAY 255 

    0xd2d3c5f2,// 259 PAY 256 

    0x069674a1,// 260 PAY 257 

    0xf75e1eb1,// 261 PAY 258 

    0x2a443bfa,// 262 PAY 259 

    0x7de563e7,// 263 PAY 260 

    0xe9bb3536,// 264 PAY 261 

    0x775dc6bf,// 265 PAY 262 

    0x96d8b50b,// 266 PAY 263 

    0xb8cf9dfc,// 267 PAY 264 

    0xa1fa7459,// 268 PAY 265 

    0x02ae7399,// 269 PAY 266 

    0x55b408b4,// 270 PAY 267 

    0x1cf911f2,// 271 PAY 268 

    0x587eccde,// 272 PAY 269 

    0xdae7b46e,// 273 PAY 270 

    0x8bd75d3d,// 274 PAY 271 

    0x299b9949,// 275 PAY 272 

    0x6767193c,// 276 PAY 273 

    0xa55fea92,// 277 PAY 274 

    0x1d75b419,// 278 PAY 275 

    0xef07ddac,// 279 PAY 276 

    0xef728682,// 280 PAY 277 

    0xcf20752c,// 281 PAY 278 

    0x66ceb48c,// 282 PAY 279 

    0xe5b97d0c,// 283 PAY 280 

    0x129920cc,// 284 PAY 281 

    0x730f66ac,// 285 PAY 282 

    0x1af8c6da,// 286 PAY 283 

    0x12fe732f,// 287 PAY 284 

    0xdb0126c6,// 288 PAY 285 

    0x433cc20a,// 289 PAY 286 

    0x704d208d,// 290 PAY 287 

    0x0ee44d9b,// 291 PAY 288 

    0x1e048ba3,// 292 PAY 289 

    0xcc41dbf0,// 293 PAY 290 

    0x647fb5ff,// 294 PAY 291 

    0xb780c0f6,// 295 PAY 292 

    0x194bb2c9,// 296 PAY 293 

    0xfee6e1f4,// 297 PAY 294 

    0x82af2361,// 298 PAY 295 

    0xc6411931,// 299 PAY 296 

    0xd5e0ce3d,// 300 PAY 297 

    0x446adc0c,// 301 PAY 298 

    0x74246323,// 302 PAY 299 

    0x73051e22,// 303 PAY 300 

    0x39dfbe40,// 304 PAY 301 

    0x7ddf44a9,// 305 PAY 302 

    0x47bcd591,// 306 PAY 303 

    0xa5b01545,// 307 PAY 304 

    0xa01bba42,// 308 PAY 305 

    0x4e531e52,// 309 PAY 306 

    0x1cd506c0,// 310 PAY 307 

    0xccefab7f,// 311 PAY 308 

    0xaeb15fdd,// 312 PAY 309 

    0x73a1666d,// 313 PAY 310 

    0x87b10c6e,// 314 PAY 311 

    0x44f9183a,// 315 PAY 312 

    0x0f8d7e3d,// 316 PAY 313 

    0x7d07c06d,// 317 PAY 314 

    0x08155204,// 318 PAY 315 

    0xef0b9c52,// 319 PAY 316 

    0x66ba4a77,// 320 PAY 317 

    0x692e84a2,// 321 PAY 318 

    0xfa9d1c40,// 322 PAY 319 

    0x13737fb8,// 323 PAY 320 

    0xda501848,// 324 PAY 321 

    0xcc579217,// 325 PAY 322 

    0x8480ef16,// 326 PAY 323 

    0x1827ac61,// 327 PAY 324 

    0xdd142067,// 328 PAY 325 

    0x65efc58b,// 329 PAY 326 

    0xcdd68834,// 330 PAY 327 

    0xb5aa2256,// 331 PAY 328 

    0x69dd1efe,// 332 PAY 329 

    0x7f4f37ae,// 333 PAY 330 

    0x3dab03fb,// 334 PAY 331 

    0xc79084a1,// 335 PAY 332 

    0x9fd734d7,// 336 PAY 333 

    0xfb5b4347,// 337 PAY 334 

    0x4c1d3093,// 338 PAY 335 

    0x94d8729d,// 339 PAY 336 

    0x94f04664,// 340 PAY 337 

    0xf1d2ba88,// 341 PAY 338 

    0x41d0c325,// 342 PAY 339 

    0x5384e0e2,// 343 PAY 340 

    0x8acdbf89,// 344 PAY 341 

    0x2601ea26,// 345 PAY 342 

    0x1b3b6877,// 346 PAY 343 

    0x2fe64941,// 347 PAY 344 

    0xc37f23ee,// 348 PAY 345 

    0x96381f99,// 349 PAY 346 

    0xebf46bc4,// 350 PAY 347 

    0x24e772f5,// 351 PAY 348 

    0x5a51749e,// 352 PAY 349 

    0x4f5441db,// 353 PAY 350 

    0x8df76024,// 354 PAY 351 

    0xa23825ec,// 355 PAY 352 

    0x45fdaacb,// 356 PAY 353 

    0xb0016c9e,// 357 PAY 354 

    0x97c4f7d7,// 358 PAY 355 

    0x34f2d4a1,// 359 PAY 356 

    0xcad734e8,// 360 PAY 357 

    0x7b374631,// 361 PAY 358 

    0x5c6c0664,// 362 PAY 359 

    0x15b601c0,// 363 PAY 360 

    0x9a7983cd,// 364 PAY 361 

    0x02101f76,// 365 PAY 362 

    0x5be5913b,// 366 PAY 363 

    0xacdd4f03,// 367 PAY 364 

    0xd507f7d0,// 368 PAY 365 

    0x5f1b30c3,// 369 PAY 366 

    0x2f997104,// 370 PAY 367 

    0x1b5799e1,// 371 PAY 368 

    0xdd93fd54,// 372 PAY 369 

    0x30670bf4,// 373 PAY 370 

    0xf12ddc6d,// 374 PAY 371 

    0x221e8ac0,// 375 PAY 372 

    0x72401170,// 376 PAY 373 

    0x0772a3d5,// 377 PAY 374 

    0xced63b8e,// 378 PAY 375 

    0x5a5e1348,// 379 PAY 376 

    0xa6288f6f,// 380 PAY 377 

    0x33505010,// 381 PAY 378 

    0x52afcc24,// 382 PAY 379 

    0x42d24545,// 383 PAY 380 

    0xd300a2a5,// 384 PAY 381 

    0xdace40bc,// 385 PAY 382 

    0x4f306afc,// 386 PAY 383 

    0x421dda55,// 387 PAY 384 

    0xf633c19a,// 388 PAY 385 

    0x37b13eb4,// 389 PAY 386 

    0xb0d7e3b7,// 390 PAY 387 

    0x271b9800,// 391 PAY 388 

    0x70937ed2,// 392 PAY 389 

    0x2aba064e,// 393 PAY 390 

    0x4ce67e14,// 394 PAY 391 

    0x419120c4,// 395 PAY 392 

    0x3de41183,// 396 PAY 393 

    0xa50f6b9d,// 397 PAY 394 

    0x03ddd847,// 398 PAY 395 

    0x02e7be69,// 399 PAY 396 

    0x325cd977,// 400 PAY 397 

    0xbb79d4a4,// 401 PAY 398 

    0x96489360,// 402 PAY 399 

    0x032476ac,// 403 PAY 400 

    0x4f3017ab,// 404 PAY 401 

    0x6630705c,// 405 PAY 402 

    0x3283cd09,// 406 PAY 403 

    0x3c1f4381,// 407 PAY 404 

    0x77429563,// 408 PAY 405 

    0xf15c86b6,// 409 PAY 406 

    0xc8e74134,// 410 PAY 407 

    0x2e2e5c5d,// 411 PAY 408 

    0x3a1fe417,// 412 PAY 409 

    0xfe000000,// 413 PAY 410 

/// STA is 1 words. 

/// STA num_pkts       : 62 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0361e83e // 414 STA   1 

};

//


uint16 rx_aes_sha1_pkt_len[] = {
    sizeof(rx_aes_sha1_pkt0_tmpl),
    sizeof(rx_aes_sha1_pkt1_tmpl),
    sizeof(rx_aes_sha1_pkt2_tmpl),
    sizeof(rx_aes_sha1_pkt3_tmpl),
    sizeof(rx_aes_sha1_pkt4_tmpl),
    sizeof(rx_aes_sha1_pkt5_tmpl),
    sizeof(rx_aes_sha1_pkt6_tmpl),
    sizeof(rx_aes_sha1_pkt7_tmpl),
    sizeof(rx_aes_sha1_pkt8_tmpl),
    sizeof(rx_aes_sha1_pkt9_tmpl),
    sizeof(rx_aes_sha1_pkt10_tmpl),
    sizeof(rx_aes_sha1_pkt11_tmpl),
    sizeof(rx_aes_sha1_pkt12_tmpl),
    sizeof(rx_aes_sha1_pkt13_tmpl),
    sizeof(rx_aes_sha1_pkt14_tmpl),
    sizeof(rx_aes_sha1_pkt15_tmpl),
    sizeof(rx_aes_sha1_pkt16_tmpl),
    sizeof(rx_aes_sha1_pkt17_tmpl),
    sizeof(rx_aes_sha1_pkt18_tmpl),
    sizeof(rx_aes_sha1_pkt19_tmpl),
    sizeof(rx_aes_sha1_pkt20_tmpl),
    sizeof(rx_aes_sha1_pkt21_tmpl),
    sizeof(rx_aes_sha1_pkt22_tmpl),
    sizeof(rx_aes_sha1_pkt23_tmpl),
    sizeof(rx_aes_sha1_pkt24_tmpl),
    sizeof(rx_aes_sha1_pkt25_tmpl),
    sizeof(rx_aes_sha1_pkt26_tmpl),
    sizeof(rx_aes_sha1_pkt27_tmpl),
    sizeof(rx_aes_sha1_pkt28_tmpl),
    sizeof(rx_aes_sha1_pkt29_tmpl),
    sizeof(rx_aes_sha1_pkt30_tmpl),
    sizeof(rx_aes_sha1_pkt31_tmpl),
    sizeof(rx_aes_sha1_pkt32_tmpl),
    sizeof(rx_aes_sha1_pkt33_tmpl),
    sizeof(rx_aes_sha1_pkt34_tmpl),
    sizeof(rx_aes_sha1_pkt35_tmpl),
    sizeof(rx_aes_sha1_pkt36_tmpl),
    sizeof(rx_aes_sha1_pkt37_tmpl),
    sizeof(rx_aes_sha1_pkt38_tmpl),
    sizeof(rx_aes_sha1_pkt39_tmpl),
    sizeof(rx_aes_sha1_pkt40_tmpl),
    sizeof(rx_aes_sha1_pkt41_tmpl),
    sizeof(rx_aes_sha1_pkt42_tmpl),
    sizeof(rx_aes_sha1_pkt43_tmpl),
    sizeof(rx_aes_sha1_pkt44_tmpl),
    sizeof(rx_aes_sha1_pkt45_tmpl),
    sizeof(rx_aes_sha1_pkt46_tmpl),
    sizeof(rx_aes_sha1_pkt47_tmpl),
    sizeof(rx_aes_sha1_pkt48_tmpl),
    sizeof(rx_aes_sha1_pkt49_tmpl),
    sizeof(rx_aes_sha1_pkt50_tmpl),
    sizeof(rx_aes_sha1_pkt51_tmpl),
    sizeof(rx_aes_sha1_pkt52_tmpl),
    sizeof(rx_aes_sha1_pkt53_tmpl),
    sizeof(rx_aes_sha1_pkt54_tmpl),
    sizeof(rx_aes_sha1_pkt55_tmpl),
    sizeof(rx_aes_sha1_pkt56_tmpl),
    sizeof(rx_aes_sha1_pkt57_tmpl),
    sizeof(rx_aes_sha1_pkt58_tmpl),
    sizeof(rx_aes_sha1_pkt59_tmpl),
    sizeof(rx_aes_sha1_pkt60_tmpl),
    sizeof(rx_aes_sha1_pkt61_tmpl),
    sizeof(rx_aes_sha1_pkt62_tmpl),
    sizeof(rx_aes_sha1_pkt63_tmpl),
    sizeof(rx_aes_sha1_pkt64_tmpl),
    sizeof(rx_aes_sha1_pkt65_tmpl),
    sizeof(rx_aes_sha1_pkt66_tmpl),
    sizeof(rx_aes_sha1_pkt67_tmpl),
    sizeof(rx_aes_sha1_pkt68_tmpl),
    sizeof(rx_aes_sha1_pkt69_tmpl),
    sizeof(rx_aes_sha1_pkt70_tmpl),
    sizeof(rx_aes_sha1_pkt71_tmpl),
    sizeof(rx_aes_sha1_pkt72_tmpl),
    sizeof(rx_aes_sha1_pkt73_tmpl),
    sizeof(rx_aes_sha1_pkt74_tmpl),
    sizeof(rx_aes_sha1_pkt75_tmpl),
    sizeof(rx_aes_sha1_pkt76_tmpl),
    sizeof(rx_aes_sha1_pkt77_tmpl),
    sizeof(rx_aes_sha1_pkt78_tmpl),
    sizeof(rx_aes_sha1_pkt79_tmpl),
    sizeof(rx_aes_sha1_pkt80_tmpl),
    sizeof(rx_aes_sha1_pkt81_tmpl),
    sizeof(rx_aes_sha1_pkt82_tmpl),
    sizeof(rx_aes_sha1_pkt83_tmpl),
    sizeof(rx_aes_sha1_pkt84_tmpl),
    sizeof(rx_aes_sha1_pkt85_tmpl),
    sizeof(rx_aes_sha1_pkt86_tmpl),
    sizeof(rx_aes_sha1_pkt87_tmpl),
    sizeof(rx_aes_sha1_pkt88_tmpl),
    sizeof(rx_aes_sha1_pkt89_tmpl),
    sizeof(rx_aes_sha1_pkt90_tmpl),
    sizeof(rx_aes_sha1_pkt91_tmpl),
    sizeof(rx_aes_sha1_pkt92_tmpl),
    sizeof(rx_aes_sha1_pkt93_tmpl),
    sizeof(rx_aes_sha1_pkt94_tmpl),
    sizeof(rx_aes_sha1_pkt95_tmpl),
    sizeof(rx_aes_sha1_pkt96_tmpl),
    sizeof(rx_aes_sha1_pkt97_tmpl),
    sizeof(rx_aes_sha1_pkt98_tmpl),
    sizeof(rx_aes_sha1_pkt99_tmpl)
};

unsigned char *rx_aes_sha1_test_pkts[] = {
    (unsigned char *)&rx_aes_sha1_pkt0_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt1_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt2_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt3_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt4_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt5_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt6_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt7_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt8_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt9_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt10_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt11_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt12_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt13_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt14_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt15_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt16_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt17_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt18_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt19_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt20_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt21_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt22_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt23_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt24_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt25_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt26_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt27_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt28_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt29_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt30_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt31_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt32_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt33_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt34_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt35_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt36_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt37_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt38_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt39_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt40_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt41_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt42_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt43_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt44_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt45_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt46_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt47_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt48_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt49_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt50_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt51_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt52_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt53_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt54_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt55_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt56_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt57_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt58_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt59_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt60_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt61_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt62_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt63_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt64_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt65_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt66_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt67_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt68_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt69_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt70_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt71_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt72_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt73_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt74_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt75_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt76_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt77_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt78_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt79_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt80_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt81_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt82_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt83_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt84_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt85_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt86_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt87_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt88_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt89_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt90_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt91_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt92_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt93_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt94_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt95_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt96_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt97_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt98_tmpl[0],
    (unsigned char *)&rx_aes_sha1_pkt99_tmpl[0]
};
#endif /* CONFIG_BCM_SPU_TEST */
#endif /* __SPUDRV_RX_AES_SHA1_DATA_H__ */
