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
#ifndef __SPUDRV_RX_DES_SHA1_DATA_H__
#define __SPUDRV_RX_DES_SHA1_DATA_H__

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

uint32 rx_des_sha1_pkt0_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 413 words. 

/// BDA size     is 1645 (0x66d) 

/// BDA id       is 0xd8ea 

    0x066dd8ea,// 3 BDA   1 

/// PAY Generic Data size   : 1645 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x9caaa9d1,// 4 PAY   1 

    0x56560a7d,// 5 PAY   2 

    0xecf9af8d,// 6 PAY   3 

    0x435c473c,// 7 PAY   4 

    0x7cc37371,// 8 PAY   5 

    0xf5a3ae10,// 9 PAY   6 

    0xe4a09d20,// 10 PAY   7 

    0xbf1ca7f5,// 11 PAY   8 

    0xc12575ba,// 12 PAY   9 

    0x41eeb1be,// 13 PAY  10 

    0x8a31985b,// 14 PAY  11 

    0x451fbfdd,// 15 PAY  12 

    0x25d94f67,// 16 PAY  13 

    0xebb2bf6f,// 17 PAY  14 

    0xde846817,// 18 PAY  15 

    0xb77c883f,// 19 PAY  16 

    0x296d1675,// 20 PAY  17 

    0x9b1ac799,// 21 PAY  18 

    0xb0c36c3b,// 22 PAY  19 

    0x8552660b,// 23 PAY  20 

    0xe9b38d10,// 24 PAY  21 

    0xcc1f04f9,// 25 PAY  22 

    0xb7b39b3c,// 26 PAY  23 

    0xab437f85,// 27 PAY  24 

    0xdffc14cd,// 28 PAY  25 

    0x7d5428bc,// 29 PAY  26 

    0xe5b8bd4e,// 30 PAY  27 

    0x4aedc3ea,// 31 PAY  28 

    0xaac9fa9a,// 32 PAY  29 

    0x55cc8e81,// 33 PAY  30 

    0x854fd3b6,// 34 PAY  31 

    0x85d375e6,// 35 PAY  32 

    0x98f10b61,// 36 PAY  33 

    0x467cf6b9,// 37 PAY  34 

    0x553a1338,// 38 PAY  35 

    0x7e6fd9e8,// 39 PAY  36 

    0x4e3f5156,// 40 PAY  37 

    0xca53365b,// 41 PAY  38 

    0xc5023077,// 42 PAY  39 

    0xa1ad3c43,// 43 PAY  40 

    0xfea08e37,// 44 PAY  41 

    0xcaf74e80,// 45 PAY  42 

    0x9b430d50,// 46 PAY  43 

    0x159b20c0,// 47 PAY  44 

    0xa7ac5f96,// 48 PAY  45 

    0xac4c411f,// 49 PAY  46 

    0xe8ff3df2,// 50 PAY  47 

    0xe94a6099,// 51 PAY  48 

    0xc92ec1fa,// 52 PAY  49 

    0x3e515c4c,// 53 PAY  50 

    0x30a8c740,// 54 PAY  51 

    0xe9078e7d,// 55 PAY  52 

    0x63505f11,// 56 PAY  53 

    0x1fb0d0fe,// 57 PAY  54 

    0x527e936f,// 58 PAY  55 

    0x254ffed6,// 59 PAY  56 

    0xfcd843c0,// 60 PAY  57 

    0xd38a29b9,// 61 PAY  58 

    0xa411eecc,// 62 PAY  59 

    0x91d06375,// 63 PAY  60 

    0xeda283ac,// 64 PAY  61 

    0x288bb325,// 65 PAY  62 

    0xd2f420d4,// 66 PAY  63 

    0x6d15f938,// 67 PAY  64 

    0xe9890815,// 68 PAY  65 

    0x0b30a891,// 69 PAY  66 

    0xaea0c448,// 70 PAY  67 

    0xcd026436,// 71 PAY  68 

    0x39cabffd,// 72 PAY  69 

    0x4f915466,// 73 PAY  70 

    0x4a98d2b5,// 74 PAY  71 

    0xd53ffe78,// 75 PAY  72 

    0x4d4ce0b7,// 76 PAY  73 

    0x9263a1bc,// 77 PAY  74 

    0x83bfd807,// 78 PAY  75 

    0xd668acd9,// 79 PAY  76 

    0xa1dfeee0,// 80 PAY  77 

    0x22fd4160,// 81 PAY  78 

    0x3345501d,// 82 PAY  79 

    0xcdf6026c,// 83 PAY  80 

    0x5e5bbdeb,// 84 PAY  81 

    0xcc2f9d27,// 85 PAY  82 

    0xc48093de,// 86 PAY  83 

    0x1d2cc294,// 87 PAY  84 

    0x58045f73,// 88 PAY  85 

    0x964c3feb,// 89 PAY  86 

    0x1b11d624,// 90 PAY  87 

    0x0074269e,// 91 PAY  88 

    0x40c26054,// 92 PAY  89 

    0xf9a502f5,// 93 PAY  90 

    0x796ff36d,// 94 PAY  91 

    0x60cc749f,// 95 PAY  92 

    0x266c2bed,// 96 PAY  93 

    0x6392a810,// 97 PAY  94 

    0xbb49567f,// 98 PAY  95 

    0x239a930d,// 99 PAY  96 

    0x4e8da708,// 100 PAY  97 

    0xf616c625,// 101 PAY  98 

    0x193afc8d,// 102 PAY  99 

    0x9ab60685,// 103 PAY 100 

    0x0afe614a,// 104 PAY 101 

    0x8e087dab,// 105 PAY 102 

    0xe474fd7a,// 106 PAY 103 

    0x038703bb,// 107 PAY 104 

    0xef0d0731,// 108 PAY 105 

    0x0a8ba17f,// 109 PAY 106 

    0x1bd34399,// 110 PAY 107 

    0xc7f468f9,// 111 PAY 108 

    0xe1d9d0e3,// 112 PAY 109 

    0xcd46a267,// 113 PAY 110 

    0x0f404a36,// 114 PAY 111 

    0xbb60dff6,// 115 PAY 112 

    0xbf3dcf6d,// 116 PAY 113 

    0x6aca2789,// 117 PAY 114 

    0x2a36b3a6,// 118 PAY 115 

    0x42590b53,// 119 PAY 116 

    0xf96d381f,// 120 PAY 117 

    0x4d3b6fa9,// 121 PAY 118 

    0x032e69c3,// 122 PAY 119 

    0xe0affc2a,// 123 PAY 120 

    0xf810cf77,// 124 PAY 121 

    0xde575894,// 125 PAY 122 

    0x007c207e,// 126 PAY 123 

    0xadff0e97,// 127 PAY 124 

    0x8c8071a3,// 128 PAY 125 

    0x65f15b16,// 129 PAY 126 

    0xcb2d9b11,// 130 PAY 127 

    0xbea8ea54,// 131 PAY 128 

    0xc92cb272,// 132 PAY 129 

    0xd3c2c591,// 133 PAY 130 

    0x4f4ddb75,// 134 PAY 131 

    0x3ad549cc,// 135 PAY 132 

    0xe16ea0f4,// 136 PAY 133 

    0x51511e69,// 137 PAY 134 

    0x540418b8,// 138 PAY 135 

    0x7c09d9f4,// 139 PAY 136 

    0xcb9322ae,// 140 PAY 137 

    0x2f7e880f,// 141 PAY 138 

    0x4509c98c,// 142 PAY 139 

    0xdaf8278f,// 143 PAY 140 

    0x114483ea,// 144 PAY 141 

    0x1d130ef7,// 145 PAY 142 

    0xd560849b,// 146 PAY 143 

    0x2a23a7c4,// 147 PAY 144 

    0x604db3df,// 148 PAY 145 

    0x193fb978,// 149 PAY 146 

    0x73d57336,// 150 PAY 147 

    0x2c47f3bb,// 151 PAY 148 

    0x48bed600,// 152 PAY 149 

    0xbf7543bf,// 153 PAY 150 

    0xa6bfddaf,// 154 PAY 151 

    0x8b1fee76,// 155 PAY 152 

    0xab64bac8,// 156 PAY 153 

    0xe108ee60,// 157 PAY 154 

    0xb96a069c,// 158 PAY 155 

    0xe083d68e,// 159 PAY 156 

    0x55ca5e32,// 160 PAY 157 

    0x3945fe08,// 161 PAY 158 

    0xe249ca01,// 162 PAY 159 

    0xda9c7866,// 163 PAY 160 

    0x5a7a4012,// 164 PAY 161 

    0xa1f8e879,// 165 PAY 162 

    0x490ec80f,// 166 PAY 163 

    0x232a812f,// 167 PAY 164 

    0xfcca31ea,// 168 PAY 165 

    0x207e564e,// 169 PAY 166 

    0xc321aa13,// 170 PAY 167 

    0x89977914,// 171 PAY 168 

    0xdf11482c,// 172 PAY 169 

    0x7dfdc86e,// 173 PAY 170 

    0x4a85847e,// 174 PAY 171 

    0x2857e8fe,// 175 PAY 172 

    0xbf8aba2b,// 176 PAY 173 

    0x27b444b1,// 177 PAY 174 

    0x32971315,// 178 PAY 175 

    0xf597a194,// 179 PAY 176 

    0xe3fd3323,// 180 PAY 177 

    0xea7405a5,// 181 PAY 178 

    0x3cd1d177,// 182 PAY 179 

    0xe43e1dec,// 183 PAY 180 

    0x66567923,// 184 PAY 181 

    0x1f391459,// 185 PAY 182 

    0xaf3aca9e,// 186 PAY 183 

    0xd55b0c01,// 187 PAY 184 

    0xf5d354f8,// 188 PAY 185 

    0x79ea0718,// 189 PAY 186 

    0x07cc7715,// 190 PAY 187 

    0x6d5bb49e,// 191 PAY 188 

    0x7ef91c86,// 192 PAY 189 

    0x5391bcee,// 193 PAY 190 

    0x3839e347,// 194 PAY 191 

    0xe15aa8e3,// 195 PAY 192 

    0x2dfd03c6,// 196 PAY 193 

    0xeced8a6e,// 197 PAY 194 

    0x91cd229e,// 198 PAY 195 

    0xeb56e2ce,// 199 PAY 196 

    0xbc4b0657,// 200 PAY 197 

    0x77376f0e,// 201 PAY 198 

    0x0ad5ea60,// 202 PAY 199 

    0xe5975db0,// 203 PAY 200 

    0xcef42566,// 204 PAY 201 

    0x9c80ed9f,// 205 PAY 202 

    0xc2224889,// 206 PAY 203 

    0x146de6eb,// 207 PAY 204 

    0x5ceaf887,// 208 PAY 205 

    0x3af488a0,// 209 PAY 206 

    0x62779917,// 210 PAY 207 

    0x9638d3c5,// 211 PAY 208 

    0x3b0aac70,// 212 PAY 209 

    0x495ef84a,// 213 PAY 210 

    0x330f3d92,// 214 PAY 211 

    0xc0f68c17,// 215 PAY 212 

    0x25c7698d,// 216 PAY 213 

    0xe8a1bc76,// 217 PAY 214 

    0x11210f6f,// 218 PAY 215 

    0x39673477,// 219 PAY 216 

    0xe130f1bc,// 220 PAY 217 

    0x44f4c914,// 221 PAY 218 

    0x8a59f261,// 222 PAY 219 

    0x7826dfa3,// 223 PAY 220 

    0x8cb68a3c,// 224 PAY 221 

    0x8b1e5805,// 225 PAY 222 

    0x40f8a296,// 226 PAY 223 

    0x659e264b,// 227 PAY 224 

    0x1ff48307,// 228 PAY 225 

    0x20cb2988,// 229 PAY 226 

    0xb1a5cd63,// 230 PAY 227 

    0x91fcec90,// 231 PAY 228 

    0x0fa89967,// 232 PAY 229 

    0x8e0e823b,// 233 PAY 230 

    0x91e20a25,// 234 PAY 231 

    0x28e1b865,// 235 PAY 232 

    0x16577c99,// 236 PAY 233 

    0x7a30459c,// 237 PAY 234 

    0x4d41a39e,// 238 PAY 235 

    0x5831450b,// 239 PAY 236 

    0x9209b601,// 240 PAY 237 

    0xebdd799b,// 241 PAY 238 

    0x2cdce2f4,// 242 PAY 239 

    0x212da149,// 243 PAY 240 

    0xfc471472,// 244 PAY 241 

    0x4c135ec6,// 245 PAY 242 

    0x1ea2b7cd,// 246 PAY 243 

    0x6f59ed36,// 247 PAY 244 

    0x293453ef,// 248 PAY 245 

    0x5147ed91,// 249 PAY 246 

    0xc8bb4a89,// 250 PAY 247 

    0xeccf8fa4,// 251 PAY 248 

    0x6e4692cc,// 252 PAY 249 

    0x14222b5f,// 253 PAY 250 

    0x879adf98,// 254 PAY 251 

    0x5b864acb,// 255 PAY 252 

    0x677eb837,// 256 PAY 253 

    0xd3022512,// 257 PAY 254 

    0x76808bac,// 258 PAY 255 

    0x75a71ea2,// 259 PAY 256 

    0x16ec67e4,// 260 PAY 257 

    0x9cbaeb6e,// 261 PAY 258 

    0x2c29cde5,// 262 PAY 259 

    0x0a6a2d86,// 263 PAY 260 

    0x17457a58,// 264 PAY 261 

    0xae35c179,// 265 PAY 262 

    0xe27cfdb2,// 266 PAY 263 

    0x04a802e2,// 267 PAY 264 

    0xeeb7f315,// 268 PAY 265 

    0x97aa9da8,// 269 PAY 266 

    0x58f271ed,// 270 PAY 267 

    0x27529deb,// 271 PAY 268 

    0xb5af5fea,// 272 PAY 269 

    0x2acec009,// 273 PAY 270 

    0xdf95f0ee,// 274 PAY 271 

    0xfec5364d,// 275 PAY 272 

    0x5218ef57,// 276 PAY 273 

    0x60b9f744,// 277 PAY 274 

    0xf4ff0229,// 278 PAY 275 

    0x764d4449,// 279 PAY 276 

    0x7ef3bce8,// 280 PAY 277 

    0x4b434b0e,// 281 PAY 278 

    0x6c80e725,// 282 PAY 279 

    0xf99c0627,// 283 PAY 280 

    0x7ee95ef0,// 284 PAY 281 

    0xc8d1f6a2,// 285 PAY 282 

    0x891492df,// 286 PAY 283 

    0x3cc9d2dd,// 287 PAY 284 

    0x86aad98f,// 288 PAY 285 

    0x3c9b623d,// 289 PAY 286 

    0x8115d771,// 290 PAY 287 

    0x105e316b,// 291 PAY 288 

    0x7c0d5a72,// 292 PAY 289 

    0x3c9af809,// 293 PAY 290 

    0xaaeeee37,// 294 PAY 291 

    0xde7cd786,// 295 PAY 292 

    0x011b5577,// 296 PAY 293 

    0xfdcb7e33,// 297 PAY 294 

    0x49446b81,// 298 PAY 295 

    0xdca09b8f,// 299 PAY 296 

    0x57409123,// 300 PAY 297 

    0x03da37d1,// 301 PAY 298 

    0x0e60be95,// 302 PAY 299 

    0xe8ba99fa,// 303 PAY 300 

    0xc630f062,// 304 PAY 301 

    0xb1d0962c,// 305 PAY 302 

    0x99c7e004,// 306 PAY 303 

    0x6e495e6c,// 307 PAY 304 

    0x3d17b67c,// 308 PAY 305 

    0xd48aaf48,// 309 PAY 306 

    0x4202a338,// 310 PAY 307 

    0xcc409839,// 311 PAY 308 

    0x0062d662,// 312 PAY 309 

    0xf2122129,// 313 PAY 310 

    0x57055f80,// 314 PAY 311 

    0x8ed1b0b4,// 315 PAY 312 

    0x866c3350,// 316 PAY 313 

    0x2c785890,// 317 PAY 314 

    0xb73543c1,// 318 PAY 315 

    0x5269cfe9,// 319 PAY 316 

    0x1f83b2a7,// 320 PAY 317 

    0x29855876,// 321 PAY 318 

    0x95bac161,// 322 PAY 319 

    0x3ab8e012,// 323 PAY 320 

    0xffc7c48a,// 324 PAY 321 

    0xe52e92c9,// 325 PAY 322 

    0xdfd79d1c,// 326 PAY 323 

    0x2b5f3034,// 327 PAY 324 

    0xba0d38ff,// 328 PAY 325 

    0x5dc6ec91,// 329 PAY 326 

    0x8a5d3d7e,// 330 PAY 327 

    0x5e790f66,// 331 PAY 328 

    0xaaadb239,// 332 PAY 329 

    0xcf4d6522,// 333 PAY 330 

    0x2c33f917,// 334 PAY 331 

    0x9ffa0797,// 335 PAY 332 

    0x27b0d393,// 336 PAY 333 

    0xb27ac7a6,// 337 PAY 334 

    0xda5b8699,// 338 PAY 335 

    0xb138cfed,// 339 PAY 336 

    0x8ed7fe8c,// 340 PAY 337 

    0xa173552d,// 341 PAY 338 

    0x30416447,// 342 PAY 339 

    0x06543a03,// 343 PAY 340 

    0x04e6db3f,// 344 PAY 341 

    0x9cb331c3,// 345 PAY 342 

    0xf22f7982,// 346 PAY 343 

    0xe92edae2,// 347 PAY 344 

    0xda842fdd,// 348 PAY 345 

    0x2eb0749b,// 349 PAY 346 

    0xa6fbdc72,// 350 PAY 347 

    0x14e6a943,// 351 PAY 348 

    0x23377791,// 352 PAY 349 

    0xf6001646,// 353 PAY 350 

    0xdb5d4ca7,// 354 PAY 351 

    0xe0850674,// 355 PAY 352 

    0xaa7d2992,// 356 PAY 353 

    0x4ee0b50a,// 357 PAY 354 

    0x76f712f3,// 358 PAY 355 

    0x2c042901,// 359 PAY 356 

    0x780e21f0,// 360 PAY 357 

    0xa9d78b36,// 361 PAY 358 

    0xb44dab84,// 362 PAY 359 

    0x147fc8db,// 363 PAY 360 

    0x7f30e5ea,// 364 PAY 361 

    0x70dbb1eb,// 365 PAY 362 

    0x8d037b5a,// 366 PAY 363 

    0xc3036285,// 367 PAY 364 

    0x5011ac70,// 368 PAY 365 

    0x97b49f8a,// 369 PAY 366 

    0xacbd1b25,// 370 PAY 367 

    0x0a5e39de,// 371 PAY 368 

    0xe7d4e714,// 372 PAY 369 

    0x325cdffe,// 373 PAY 370 

    0x2dfe549c,// 374 PAY 371 

    0x8da7f049,// 375 PAY 372 

    0xe54bb19e,// 376 PAY 373 

    0x11244340,// 377 PAY 374 

    0x83699047,// 378 PAY 375 

    0x8f2080c9,// 379 PAY 376 

    0xd880eaea,// 380 PAY 377 

    0x2d44c2d0,// 381 PAY 378 

    0x950d2852,// 382 PAY 379 

    0xe5db731d,// 383 PAY 380 

    0x6b7aba32,// 384 PAY 381 

    0xb5d07048,// 385 PAY 382 

    0x02b83dec,// 386 PAY 383 

    0x0e383250,// 387 PAY 384 

    0x893d8aef,// 388 PAY 385 

    0x595ff34c,// 389 PAY 386 

    0x4a86352a,// 390 PAY 387 

    0x350fff2d,// 391 PAY 388 

    0xc061cf16,// 392 PAY 389 

    0x1cc82520,// 393 PAY 390 

    0x2fd25313,// 394 PAY 391 

    0xa06bf3e0,// 395 PAY 392 

    0x8a9e7cee,// 396 PAY 393 

    0x61bf5d48,// 397 PAY 394 

    0xb5d489af,// 398 PAY 395 

    0xb39e1e3e,// 399 PAY 396 

    0xe1e750ef,// 400 PAY 397 

    0xcdd2f34c,// 401 PAY 398 

    0x2b2559bd,// 402 PAY 399 

    0x12689cb1,// 403 PAY 400 

    0x33907a2e,// 404 PAY 401 

    0xd8fa4a4c,// 405 PAY 402 

    0x32d31a5e,// 406 PAY 403 

    0xacf37c3a,// 407 PAY 404 

    0xc8c7fed5,// 408 PAY 405 

    0xe5fe1f02,// 409 PAY 406 

    0xde59f3c2,// 410 PAY 407 

    0xbe144662,// 411 PAY 408 

    0xa03e3e57,// 412 PAY 409 

    0x52b5d7e7,// 413 PAY 410 

    0x76d48e66,// 414 PAY 411 

    0x10000000,// 415 PAY 412 

/// HASH is  16 bytes 

    0x011b5577,// 416 HSH   1 

    0xfdcb7e33,// 417 HSH   2 

    0x49446b81,// 418 HSH   3 

    0xdca09b8f,// 419 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 99 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x93 

    0x02609363 // 420 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt1_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 71 words. 

/// BDA size     is 277 (0x115) 

/// BDA id       is 0x3d82 

    0x01153d82,// 3 BDA   1 

/// PAY Generic Data size   : 277 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x006bf034,// 4 PAY   1 

    0x36e61533,// 5 PAY   2 

    0x22bc3fec,// 6 PAY   3 

    0x5c27acd3,// 7 PAY   4 

    0xcadc5c10,// 8 PAY   5 

    0xfdd6f2f6,// 9 PAY   6 

    0x4baf7b94,// 10 PAY   7 

    0x5ccb75a7,// 11 PAY   8 

    0x28498b09,// 12 PAY   9 

    0xcfeb6091,// 13 PAY  10 

    0xd3015900,// 14 PAY  11 

    0x532f2430,// 15 PAY  12 

    0x545e58f7,// 16 PAY  13 

    0xa780d561,// 17 PAY  14 

    0x560cda6e,// 18 PAY  15 

    0x64a08e6b,// 19 PAY  16 

    0x08eeb9b3,// 20 PAY  17 

    0xd412aee6,// 21 PAY  18 

    0xb98d6995,// 22 PAY  19 

    0xc5427b4a,// 23 PAY  20 

    0x0b8e7c61,// 24 PAY  21 

    0x81a19e15,// 25 PAY  22 

    0xcd8737ca,// 26 PAY  23 

    0x7dd3b29d,// 27 PAY  24 

    0xd01a8d76,// 28 PAY  25 

    0x4de7d99b,// 29 PAY  26 

    0x5d25bcd7,// 30 PAY  27 

    0x792ef098,// 31 PAY  28 

    0x7ea2f522,// 32 PAY  29 

    0xc058ed06,// 33 PAY  30 

    0xc67f5b78,// 34 PAY  31 

    0x23c7d5e2,// 35 PAY  32 

    0x0f73bc6a,// 36 PAY  33 

    0x1fc1624c,// 37 PAY  34 

    0xa1bd14b5,// 38 PAY  35 

    0x66dd08f4,// 39 PAY  36 

    0x5b09563f,// 40 PAY  37 

    0x3adb1262,// 41 PAY  38 

    0x04f2ce71,// 42 PAY  39 

    0xa53acb73,// 43 PAY  40 

    0xd327139c,// 44 PAY  41 

    0x03902048,// 45 PAY  42 

    0x1cb7be68,// 46 PAY  43 

    0x533b8f90,// 47 PAY  44 

    0x517f8995,// 48 PAY  45 

    0x7fb36c5c,// 49 PAY  46 

    0xc617abdf,// 50 PAY  47 

    0x59938007,// 51 PAY  48 

    0x3af43e7c,// 52 PAY  49 

    0x7cebc9bc,// 53 PAY  50 

    0xa2aec587,// 54 PAY  51 

    0x76e41f7d,// 55 PAY  52 

    0xa5a9d092,// 56 PAY  53 

    0xdf91dd00,// 57 PAY  54 

    0x7a13b635,// 58 PAY  55 

    0x92ddb002,// 59 PAY  56 

    0x605f21fb,// 60 PAY  57 

    0x04b77c9b,// 61 PAY  58 

    0xd689cd26,// 62 PAY  59 

    0x4ee41a27,// 63 PAY  60 

    0x083e58c5,// 64 PAY  61 

    0xf96b0632,// 65 PAY  62 

    0xd89ac72d,// 66 PAY  63 

    0x545d3eed,// 67 PAY  64 

    0xde123558,// 68 PAY  65 

    0x4a056fd4,// 69 PAY  66 

    0xeeb6e7a3,// 70 PAY  67 

    0xf1420dc2,// 71 PAY  68 

    0x60146676,// 72 PAY  69 

    0xf0000000,// 73 PAY  70 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x018880b2 // 74 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt2_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 149 words. 

/// BDA size     is 592 (0x250) 

/// BDA id       is 0xb49d 

    0x0250b49d,// 3 BDA   1 

/// PAY Generic Data size   : 592 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x77f29563,// 4 PAY   1 

    0x5a0cbf1d,// 5 PAY   2 

    0xf5cd007c,// 6 PAY   3 

    0xc72a5e6a,// 7 PAY   4 

    0xfc9c85d5,// 8 PAY   5 

    0x26942087,// 9 PAY   6 

    0xfb3b5286,// 10 PAY   7 

    0x56fa3df1,// 11 PAY   8 

    0xb4f0fef8,// 12 PAY   9 

    0x45c876bb,// 13 PAY  10 

    0xe6bb8807,// 14 PAY  11 

    0x9455c8a9,// 15 PAY  12 

    0x4e237977,// 16 PAY  13 

    0x8e84ed67,// 17 PAY  14 

    0xd5e6e7a1,// 18 PAY  15 

    0xb6c4dc8b,// 19 PAY  16 

    0x4f790bd6,// 20 PAY  17 

    0x0318e91e,// 21 PAY  18 

    0xd4988d26,// 22 PAY  19 

    0x58be9890,// 23 PAY  20 

    0xfb342480,// 24 PAY  21 

    0xf6139bd5,// 25 PAY  22 

    0x70c21c11,// 26 PAY  23 

    0x0754561c,// 27 PAY  24 

    0xcf85b9d6,// 28 PAY  25 

    0xb2dbd063,// 29 PAY  26 

    0x6e3f2e10,// 30 PAY  27 

    0x9999e5a7,// 31 PAY  28 

    0xc5cd7736,// 32 PAY  29 

    0x7fa4c518,// 33 PAY  30 

    0x610de9a5,// 34 PAY  31 

    0x1563869b,// 35 PAY  32 

    0xc9cb03a7,// 36 PAY  33 

    0x4c0eb75d,// 37 PAY  34 

    0xbf7b6114,// 38 PAY  35 

    0xeeb6e552,// 39 PAY  36 

    0x20e71bc0,// 40 PAY  37 

    0x46928f0b,// 41 PAY  38 

    0x8eec144d,// 42 PAY  39 

    0x43656b82,// 43 PAY  40 

    0x2bf32eab,// 44 PAY  41 

    0x9ffee422,// 45 PAY  42 

    0x884cd9f1,// 46 PAY  43 

    0x175dc2aa,// 47 PAY  44 

    0x033508a2,// 48 PAY  45 

    0xe3adc0f4,// 49 PAY  46 

    0xeae98ece,// 50 PAY  47 

    0xdf0b6d84,// 51 PAY  48 

    0xf4ab7075,// 52 PAY  49 

    0x9e319e04,// 53 PAY  50 

    0x8a85d69d,// 54 PAY  51 

    0x740559b8,// 55 PAY  52 

    0xbb10a234,// 56 PAY  53 

    0x5d2c8bf4,// 57 PAY  54 

    0x5dbcf3fe,// 58 PAY  55 

    0x1b39ea0a,// 59 PAY  56 

    0xa150f1ef,// 60 PAY  57 

    0x3a823713,// 61 PAY  58 

    0x9e94ddf4,// 62 PAY  59 

    0xadb0c443,// 63 PAY  60 

    0xe5bf3d16,// 64 PAY  61 

    0xad342dc4,// 65 PAY  62 

    0x0202620a,// 66 PAY  63 

    0x38dfc533,// 67 PAY  64 

    0x31fdc1c5,// 68 PAY  65 

    0x3fbeea00,// 69 PAY  66 

    0x449d3b5c,// 70 PAY  67 

    0xad81eea9,// 71 PAY  68 

    0x5967e2ca,// 72 PAY  69 

    0x7af5ccaf,// 73 PAY  70 

    0xe4425c78,// 74 PAY  71 

    0x9be653b7,// 75 PAY  72 

    0xf967c487,// 76 PAY  73 

    0xf5a618cc,// 77 PAY  74 

    0xd19fa0de,// 78 PAY  75 

    0x4faa12f6,// 79 PAY  76 

    0x9e0fbe3a,// 80 PAY  77 

    0xd6acaf72,// 81 PAY  78 

    0xd0895e06,// 82 PAY  79 

    0x80000d38,// 83 PAY  80 

    0x85f71e8d,// 84 PAY  81 

    0xe0a0cb72,// 85 PAY  82 

    0x52ae8c41,// 86 PAY  83 

    0xcc4b247c,// 87 PAY  84 

    0x014ce28e,// 88 PAY  85 

    0x7ee4b0c9,// 89 PAY  86 

    0xa8c45108,// 90 PAY  87 

    0x963846eb,// 91 PAY  88 

    0x14dd1391,// 92 PAY  89 

    0x3e058496,// 93 PAY  90 

    0xe4949e46,// 94 PAY  91 

    0xe71c4610,// 95 PAY  92 

    0x978564b0,// 96 PAY  93 

    0x71c4d072,// 97 PAY  94 

    0x598d7451,// 98 PAY  95 

    0x2c4d7beb,// 99 PAY  96 

    0xa8788caf,// 100 PAY  97 

    0x29a03f10,// 101 PAY  98 

    0x8a9be6c7,// 102 PAY  99 

    0x9b8e1585,// 103 PAY 100 

    0xd4bb5a96,// 104 PAY 101 

    0xee3bc58e,// 105 PAY 102 

    0x77f3c5a5,// 106 PAY 103 

    0x56738ab0,// 107 PAY 104 

    0x979aee5a,// 108 PAY 105 

    0xb97c6089,// 109 PAY 106 

    0x1b853d14,// 110 PAY 107 

    0xbd844109,// 111 PAY 108 

    0x264a05f6,// 112 PAY 109 

    0x13009385,// 113 PAY 110 

    0x7c55dd44,// 114 PAY 111 

    0xbd731af6,// 115 PAY 112 

    0xf43e93f9,// 116 PAY 113 

    0x503303df,// 117 PAY 114 

    0x1874ce3f,// 118 PAY 115 

    0x383f4ac8,// 119 PAY 116 

    0xcc76c488,// 120 PAY 117 

    0x22e9368c,// 121 PAY 118 

    0x75f22075,// 122 PAY 119 

    0x5d9ba284,// 123 PAY 120 

    0xe31e8feb,// 124 PAY 121 

    0xb0259a6c,// 125 PAY 122 

    0x1d8ecda5,// 126 PAY 123 

    0xb5373dea,// 127 PAY 124 

    0x5332e9ae,// 128 PAY 125 

    0x64e92d30,// 129 PAY 126 

    0x76b073cd,// 130 PAY 127 

    0xd45bba02,// 131 PAY 128 

    0xbae5452a,// 132 PAY 129 

    0x0bb58c2d,// 133 PAY 130 

    0x56cc303e,// 134 PAY 131 

    0x2ce6a6c2,// 135 PAY 132 

    0xb162a135,// 136 PAY 133 

    0xefa0ed66,// 137 PAY 134 

    0xc3522e3e,// 138 PAY 135 

    0x05e25ce3,// 139 PAY 136 

    0xcb2f748b,// 140 PAY 137 

    0xd237ae06,// 141 PAY 138 

    0x909d1255,// 142 PAY 139 

    0xef8beff6,// 143 PAY 140 

    0x2e6ad892,// 144 PAY 141 

    0xcd1e459c,// 145 PAY 142 

    0xdf74e905,// 146 PAY 143 

    0x45d6c4eb,// 147 PAY 144 

    0xd94ee7cf,// 148 PAY 145 

    0x9c67cc94,// 149 PAY 146 

    0xa7ad4014,// 150 PAY 147 

    0xb31e419f,// 151 PAY 148 

/// STA is 1 words. 

/// STA num_pkts       : 74 

/// STA pkt_idx        : 189 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe9 

    0x02f5e94a // 152 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt3_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 471 words. 

/// BDA size     is 1877 (0x755) 

/// BDA id       is 0x61e 

    0x0755061e,// 3 BDA   1 

/// PAY Generic Data size   : 1877 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x8d01eebf,// 4 PAY   1 

    0x46615a4d,// 5 PAY   2 

    0x4d43d717,// 6 PAY   3 

    0x55e1a86b,// 7 PAY   4 

    0xbd727c05,// 8 PAY   5 

    0x28997dd3,// 9 PAY   6 

    0x663865eb,// 10 PAY   7 

    0x029feb98,// 11 PAY   8 

    0x4551aa6e,// 12 PAY   9 

    0x194c2063,// 13 PAY  10 

    0xd5ec8ba1,// 14 PAY  11 

    0x050faccd,// 15 PAY  12 

    0xfad5fde8,// 16 PAY  13 

    0x9d195c09,// 17 PAY  14 

    0x333d1bd5,// 18 PAY  15 

    0x434c6783,// 19 PAY  16 

    0x77359b7b,// 20 PAY  17 

    0x3a7743c8,// 21 PAY  18 

    0xeab1a016,// 22 PAY  19 

    0xfaee1e83,// 23 PAY  20 

    0x9ed7f0de,// 24 PAY  21 

    0x6cecb079,// 25 PAY  22 

    0x0eda7bab,// 26 PAY  23 

    0x6864bf6a,// 27 PAY  24 

    0x1b969e38,// 28 PAY  25 

    0x462341ba,// 29 PAY  26 

    0x9e345193,// 30 PAY  27 

    0xdaeb34b7,// 31 PAY  28 

    0xa914fbf6,// 32 PAY  29 

    0xfc457b2d,// 33 PAY  30 

    0xdcaec82a,// 34 PAY  31 

    0x70490899,// 35 PAY  32 

    0x2e0b255b,// 36 PAY  33 

    0x6ec5bf8d,// 37 PAY  34 

    0xd03697f4,// 38 PAY  35 

    0xc2e93989,// 39 PAY  36 

    0x6092ae26,// 40 PAY  37 

    0x9984f0fc,// 41 PAY  38 

    0x2fa00e71,// 42 PAY  39 

    0xd1e9c83d,// 43 PAY  40 

    0x1d7a8d9f,// 44 PAY  41 

    0x561d63d3,// 45 PAY  42 

    0x22c8b604,// 46 PAY  43 

    0x6df47677,// 47 PAY  44 

    0x2e50dd9b,// 48 PAY  45 

    0xdda5ff8b,// 49 PAY  46 

    0x649a68a5,// 50 PAY  47 

    0xc97dca0b,// 51 PAY  48 

    0x5aff7f83,// 52 PAY  49 

    0x9f26976a,// 53 PAY  50 

    0x3076dec7,// 54 PAY  51 

    0x8fef599a,// 55 PAY  52 

    0x968dfdb1,// 56 PAY  53 

    0xda9aaf93,// 57 PAY  54 

    0x39996dfc,// 58 PAY  55 

    0x7b0c5cc1,// 59 PAY  56 

    0x3f32532b,// 60 PAY  57 

    0x6687c8e2,// 61 PAY  58 

    0xd140f34c,// 62 PAY  59 

    0xb575ed9c,// 63 PAY  60 

    0xd5a8c884,// 64 PAY  61 

    0xc909a88a,// 65 PAY  62 

    0x68a3d654,// 66 PAY  63 

    0xa9894a52,// 67 PAY  64 

    0xd766a30b,// 68 PAY  65 

    0xb6f28bf2,// 69 PAY  66 

    0x51b96760,// 70 PAY  67 

    0x90e7c55c,// 71 PAY  68 

    0xad423119,// 72 PAY  69 

    0x2cd9e26b,// 73 PAY  70 

    0xb6850d76,// 74 PAY  71 

    0x61d620c1,// 75 PAY  72 

    0x6e7b4edc,// 76 PAY  73 

    0x57344172,// 77 PAY  74 

    0x4d047828,// 78 PAY  75 

    0x6bc098e7,// 79 PAY  76 

    0xecd52ab6,// 80 PAY  77 

    0x64442b51,// 81 PAY  78 

    0xeb5545c0,// 82 PAY  79 

    0x7699a726,// 83 PAY  80 

    0xdc81ede5,// 84 PAY  81 

    0xaaa7cb0b,// 85 PAY  82 

    0xfee2cf9b,// 86 PAY  83 

    0x26bc2837,// 87 PAY  84 

    0x471dfa1d,// 88 PAY  85 

    0x36d22c0b,// 89 PAY  86 

    0xdea50b30,// 90 PAY  87 

    0xb0166f2d,// 91 PAY  88 

    0xf8fb0a8c,// 92 PAY  89 

    0xbb41fc45,// 93 PAY  90 

    0xdb02f966,// 94 PAY  91 

    0x6396b7a7,// 95 PAY  92 

    0xf77a0c65,// 96 PAY  93 

    0x38b77e0c,// 97 PAY  94 

    0xac7d2929,// 98 PAY  95 

    0xe96f7f09,// 99 PAY  96 

    0x49ebf04e,// 100 PAY  97 

    0xade20220,// 101 PAY  98 

    0x86e890d8,// 102 PAY  99 

    0x5f1c2437,// 103 PAY 100 

    0x4a50356f,// 104 PAY 101 

    0x5981f554,// 105 PAY 102 

    0xb8a746c9,// 106 PAY 103 

    0x6cfe9350,// 107 PAY 104 

    0x07aad8ec,// 108 PAY 105 

    0x7cc02c5f,// 109 PAY 106 

    0xe93c0867,// 110 PAY 107 

    0x4e122d56,// 111 PAY 108 

    0xc13b2c70,// 112 PAY 109 

    0xe819a591,// 113 PAY 110 

    0x09d64f38,// 114 PAY 111 

    0x81e25855,// 115 PAY 112 

    0x60038e25,// 116 PAY 113 

    0x6a1c7ef2,// 117 PAY 114 

    0xd537189a,// 118 PAY 115 

    0x2115dda8,// 119 PAY 116 

    0xeb552fcf,// 120 PAY 117 

    0x3c6bec36,// 121 PAY 118 

    0xd027b782,// 122 PAY 119 

    0xf9542110,// 123 PAY 120 

    0xbef9407a,// 124 PAY 121 

    0x319c892b,// 125 PAY 122 

    0xd11b1481,// 126 PAY 123 

    0x11b76615,// 127 PAY 124 

    0xfed8240a,// 128 PAY 125 

    0x4d5c6bd6,// 129 PAY 126 

    0xf070c82c,// 130 PAY 127 

    0x9681f0c0,// 131 PAY 128 

    0x2d24cf2b,// 132 PAY 129 

    0x043c7c29,// 133 PAY 130 

    0x1bca7090,// 134 PAY 131 

    0x2aa34bdf,// 135 PAY 132 

    0xba8bba4f,// 136 PAY 133 

    0x346625e1,// 137 PAY 134 

    0x419808ac,// 138 PAY 135 

    0x83b45019,// 139 PAY 136 

    0x19eb43d3,// 140 PAY 137 

    0x16c146bc,// 141 PAY 138 

    0x561b342b,// 142 PAY 139 

    0x00832e96,// 143 PAY 140 

    0xfb8a5175,// 144 PAY 141 

    0x717ef3dc,// 145 PAY 142 

    0x18d11be5,// 146 PAY 143 

    0x09511229,// 147 PAY 144 

    0xdade1e7c,// 148 PAY 145 

    0xa45d3d54,// 149 PAY 146 

    0x5c1a59b1,// 150 PAY 147 

    0xa1689cb4,// 151 PAY 148 

    0x233bf2d6,// 152 PAY 149 

    0x63edbf5b,// 153 PAY 150 

    0xfba86602,// 154 PAY 151 

    0xfea53956,// 155 PAY 152 

    0xd4623b8c,// 156 PAY 153 

    0x8b08b5a9,// 157 PAY 154 

    0x77406f93,// 158 PAY 155 

    0xe51d7edf,// 159 PAY 156 

    0x1050db22,// 160 PAY 157 

    0x72304c8b,// 161 PAY 158 

    0xe2f110bb,// 162 PAY 159 

    0xcb0be014,// 163 PAY 160 

    0x93daf9ea,// 164 PAY 161 

    0xb87bfc5f,// 165 PAY 162 

    0x723e859b,// 166 PAY 163 

    0x6b728bbc,// 167 PAY 164 

    0x280ade51,// 168 PAY 165 

    0x7c9a52ec,// 169 PAY 166 

    0x49a25a9e,// 170 PAY 167 

    0x850f33f1,// 171 PAY 168 

    0x2ded73b5,// 172 PAY 169 

    0xf9abedd5,// 173 PAY 170 

    0x8bf446ce,// 174 PAY 171 

    0xa42c1fc1,// 175 PAY 172 

    0x25a84981,// 176 PAY 173 

    0x5bbb94b7,// 177 PAY 174 

    0x43f28df3,// 178 PAY 175 

    0x8617cae2,// 179 PAY 176 

    0x779c042c,// 180 PAY 177 

    0x027ce365,// 181 PAY 178 

    0x429be128,// 182 PAY 179 

    0x9c9e4975,// 183 PAY 180 

    0x6fdc8891,// 184 PAY 181 

    0x575a9b39,// 185 PAY 182 

    0xe267e582,// 186 PAY 183 

    0xd4492ee9,// 187 PAY 184 

    0xa30a1df4,// 188 PAY 185 

    0x1f1ef35e,// 189 PAY 186 

    0xbfa5559e,// 190 PAY 187 

    0x28e16bb6,// 191 PAY 188 

    0x24ce19ee,// 192 PAY 189 

    0xffd21990,// 193 PAY 190 

    0x07cbd5ec,// 194 PAY 191 

    0x429a3cd4,// 195 PAY 192 

    0x8f6a2dec,// 196 PAY 193 

    0xdf5b685b,// 197 PAY 194 

    0x87de9fe1,// 198 PAY 195 

    0x82fe3002,// 199 PAY 196 

    0x001ff3a5,// 200 PAY 197 

    0xc718b5df,// 201 PAY 198 

    0x740d2bc8,// 202 PAY 199 

    0x8c10ca4f,// 203 PAY 200 

    0x8a4cda41,// 204 PAY 201 

    0xe16fb23a,// 205 PAY 202 

    0xab2881d8,// 206 PAY 203 

    0xdf26d1ce,// 207 PAY 204 

    0xba6b99f0,// 208 PAY 205 

    0x67d80176,// 209 PAY 206 

    0x4ffd6f48,// 210 PAY 207 

    0x08844072,// 211 PAY 208 

    0xe477d365,// 212 PAY 209 

    0xd1ce729b,// 213 PAY 210 

    0xe7594d13,// 214 PAY 211 

    0xe588b38b,// 215 PAY 212 

    0xdb1d5193,// 216 PAY 213 

    0x6a141d8e,// 217 PAY 214 

    0xc9f07ada,// 218 PAY 215 

    0xf360ff9b,// 219 PAY 216 

    0x42b7a873,// 220 PAY 217 

    0x3dbe9115,// 221 PAY 218 

    0x80388e31,// 222 PAY 219 

    0x41ba948c,// 223 PAY 220 

    0xcdeeebcd,// 224 PAY 221 

    0xf133eab7,// 225 PAY 222 

    0x65d3afec,// 226 PAY 223 

    0x08904ca2,// 227 PAY 224 

    0xe2651d1f,// 228 PAY 225 

    0x3c0b330d,// 229 PAY 226 

    0x7a5bcc02,// 230 PAY 227 

    0xefa7a2eb,// 231 PAY 228 

    0x13a5184a,// 232 PAY 229 

    0x424677f3,// 233 PAY 230 

    0xd7ff66be,// 234 PAY 231 

    0x2e1214d1,// 235 PAY 232 

    0x28424067,// 236 PAY 233 

    0xc40ad686,// 237 PAY 234 

    0x4edbc7fe,// 238 PAY 235 

    0xea40444c,// 239 PAY 236 

    0x3a306a50,// 240 PAY 237 

    0x223c5b65,// 241 PAY 238 

    0x0f53ca48,// 242 PAY 239 

    0xb31c3a0c,// 243 PAY 240 

    0xf956e4f7,// 244 PAY 241 

    0xfc915dbe,// 245 PAY 242 

    0x26b1806d,// 246 PAY 243 

    0x387ce920,// 247 PAY 244 

    0xbe2cbc08,// 248 PAY 245 

    0x95d4df41,// 249 PAY 246 

    0x51e3e93f,// 250 PAY 247 

    0x047e7a32,// 251 PAY 248 

    0xf3a702db,// 252 PAY 249 

    0x76392907,// 253 PAY 250 

    0x94207bf7,// 254 PAY 251 

    0x068e5ab8,// 255 PAY 252 

    0x9bb3aadd,// 256 PAY 253 

    0xe0ecfd99,// 257 PAY 254 

    0xd983b8f2,// 258 PAY 255 

    0xeefa8707,// 259 PAY 256 

    0x69c467f1,// 260 PAY 257 

    0x309e9c37,// 261 PAY 258 

    0x47e201e4,// 262 PAY 259 

    0x79b38151,// 263 PAY 260 

    0x7b2515bb,// 264 PAY 261 

    0x4ab30d33,// 265 PAY 262 

    0xf14b5678,// 266 PAY 263 

    0x9730c047,// 267 PAY 264 

    0x26ea37fd,// 268 PAY 265 

    0x10e48248,// 269 PAY 266 

    0x3aec8ea9,// 270 PAY 267 

    0x46f3cbc0,// 271 PAY 268 

    0xbb9d4869,// 272 PAY 269 

    0x44f13eca,// 273 PAY 270 

    0x979344f3,// 274 PAY 271 

    0x9d08a269,// 275 PAY 272 

    0x53ace54d,// 276 PAY 273 

    0xcae1f0b5,// 277 PAY 274 

    0x4a73ead9,// 278 PAY 275 

    0x46cbef54,// 279 PAY 276 

    0xdf7262e2,// 280 PAY 277 

    0x06e12fb2,// 281 PAY 278 

    0x0c91d8ac,// 282 PAY 279 

    0xf8677bb0,// 283 PAY 280 

    0xbc6f7b93,// 284 PAY 281 

    0x82900f33,// 285 PAY 282 

    0xc799b623,// 286 PAY 283 

    0x6287d23c,// 287 PAY 284 

    0xe3bd8db4,// 288 PAY 285 

    0x23d69f12,// 289 PAY 286 

    0x67336d46,// 290 PAY 287 

    0xe2f1dcab,// 291 PAY 288 

    0xd0a91b25,// 292 PAY 289 

    0x90939a71,// 293 PAY 290 

    0xcad0ebad,// 294 PAY 291 

    0x383cd909,// 295 PAY 292 

    0x0ab014ec,// 296 PAY 293 

    0x767be2ee,// 297 PAY 294 

    0xeb0a9d5a,// 298 PAY 295 

    0xc76ac521,// 299 PAY 296 

    0xcd55d322,// 300 PAY 297 

    0x833a4b4b,// 301 PAY 298 

    0x40debbf5,// 302 PAY 299 

    0x78efe1fd,// 303 PAY 300 

    0xd2339472,// 304 PAY 301 

    0x59399f2f,// 305 PAY 302 

    0x6da4bb03,// 306 PAY 303 

    0x4c409e58,// 307 PAY 304 

    0x2d542866,// 308 PAY 305 

    0x33f8c9c9,// 309 PAY 306 

    0xdd8e556a,// 310 PAY 307 

    0xc1687299,// 311 PAY 308 

    0x084b0f75,// 312 PAY 309 

    0x46f737a9,// 313 PAY 310 

    0x3c2f9bea,// 314 PAY 311 

    0x43490694,// 315 PAY 312 

    0x358f0ddf,// 316 PAY 313 

    0x6e963802,// 317 PAY 314 

    0xe97b7f20,// 318 PAY 315 

    0xb1df2622,// 319 PAY 316 

    0x2888f91c,// 320 PAY 317 

    0x92a4c08a,// 321 PAY 318 

    0xf098a7a7,// 322 PAY 319 

    0x26f4264d,// 323 PAY 320 

    0x81caa278,// 324 PAY 321 

    0x97b32c08,// 325 PAY 322 

    0xaa347fa6,// 326 PAY 323 

    0x21c4a108,// 327 PAY 324 

    0x7fde893e,// 328 PAY 325 

    0x93e17e34,// 329 PAY 326 

    0xdd6a74d5,// 330 PAY 327 

    0xa1a99987,// 331 PAY 328 

    0x243bf539,// 332 PAY 329 

    0x9cd7cb40,// 333 PAY 330 

    0x610c1b42,// 334 PAY 331 

    0xab9ffad3,// 335 PAY 332 

    0xc639bca6,// 336 PAY 333 

    0x5842d976,// 337 PAY 334 

    0xfb97a9d5,// 338 PAY 335 

    0x18446439,// 339 PAY 336 

    0xc6be8e26,// 340 PAY 337 

    0x186106ae,// 341 PAY 338 

    0x693102f0,// 342 PAY 339 

    0x8e0d791d,// 343 PAY 340 

    0xe7702d96,// 344 PAY 341 

    0x170c04bc,// 345 PAY 342 

    0x2f9644e0,// 346 PAY 343 

    0x7819f7d4,// 347 PAY 344 

    0x0428b728,// 348 PAY 345 

    0x6ab3b423,// 349 PAY 346 

    0x1f5937da,// 350 PAY 347 

    0x47bf4064,// 351 PAY 348 

    0x68611428,// 352 PAY 349 

    0x97d62795,// 353 PAY 350 

    0xd78651a7,// 354 PAY 351 

    0xad824439,// 355 PAY 352 

    0x7e0e443f,// 356 PAY 353 

    0x2f1bcc87,// 357 PAY 354 

    0x31fcad9c,// 358 PAY 355 

    0x7dcbb59a,// 359 PAY 356 

    0x90c55c7d,// 360 PAY 357 

    0x40e72384,// 361 PAY 358 

    0x50146615,// 362 PAY 359 

    0x145f99d9,// 363 PAY 360 

    0x6005b0ff,// 364 PAY 361 

    0x6fe2ac95,// 365 PAY 362 

    0xd9045678,// 366 PAY 363 

    0x3005c6c7,// 367 PAY 364 

    0x53398519,// 368 PAY 365 

    0x1d543a1b,// 369 PAY 366 

    0x40986056,// 370 PAY 367 

    0x01a72e4e,// 371 PAY 368 

    0xbd7b2e82,// 372 PAY 369 

    0xf0044a84,// 373 PAY 370 

    0x1763212a,// 374 PAY 371 

    0xa7298fcc,// 375 PAY 372 

    0x6913d8ef,// 376 PAY 373 

    0xc72c5479,// 377 PAY 374 

    0xaf1a9818,// 378 PAY 375 

    0xbb4b1f61,// 379 PAY 376 

    0x2bb8bbfc,// 380 PAY 377 

    0x31b5dc39,// 381 PAY 378 

    0x312b0ffc,// 382 PAY 379 

    0x06f89363,// 383 PAY 380 

    0xf93f6962,// 384 PAY 381 

    0x931922eb,// 385 PAY 382 

    0x057a7f9b,// 386 PAY 383 

    0xf40680c4,// 387 PAY 384 

    0xdbb72a18,// 388 PAY 385 

    0xbc73f998,// 389 PAY 386 

    0x1f6cf671,// 390 PAY 387 

    0x66525ed1,// 391 PAY 388 

    0x067da3a3,// 392 PAY 389 

    0xca216d42,// 393 PAY 390 

    0x12dfd153,// 394 PAY 391 

    0x9ffe625d,// 395 PAY 392 

    0x3c4213bf,// 396 PAY 393 

    0x1bd7f883,// 397 PAY 394 

    0x4ac4b065,// 398 PAY 395 

    0xb9d5cef8,// 399 PAY 396 

    0xc4c718d0,// 400 PAY 397 

    0x44d0fe62,// 401 PAY 398 

    0xe4856580,// 402 PAY 399 

    0xe5d09e4b,// 403 PAY 400 

    0xa985c996,// 404 PAY 401 

    0xad061329,// 405 PAY 402 

    0x1c6b2678,// 406 PAY 403 

    0x66055b7f,// 407 PAY 404 

    0xb216cae9,// 408 PAY 405 

    0xd07e3b6f,// 409 PAY 406 

    0xdc40b1a5,// 410 PAY 407 

    0x5d67f30b,// 411 PAY 408 

    0x9cc94e42,// 412 PAY 409 

    0x4deecfaf,// 413 PAY 410 

    0xd36b37f2,// 414 PAY 411 

    0xa582bea0,// 415 PAY 412 

    0x604b7c56,// 416 PAY 413 

    0x7a4db53d,// 417 PAY 414 

    0xcf6fa762,// 418 PAY 415 

    0xab18fe80,// 419 PAY 416 

    0x2207515e,// 420 PAY 417 

    0x07eee550,// 421 PAY 418 

    0x0508e991,// 422 PAY 419 

    0x241e0cb1,// 423 PAY 420 

    0xdd6761a7,// 424 PAY 421 

    0x1ba8edd2,// 425 PAY 422 

    0x853a87f2,// 426 PAY 423 

    0x25b47f29,// 427 PAY 424 

    0x11bbdec5,// 428 PAY 425 

    0x3a2e435e,// 429 PAY 426 

    0x32982038,// 430 PAY 427 

    0x8b4f7c3e,// 431 PAY 428 

    0xb16c0f0a,// 432 PAY 429 

    0x2dd7ced5,// 433 PAY 430 

    0x8bc84e94,// 434 PAY 431 

    0xabdb0924,// 435 PAY 432 

    0x0d9aca9e,// 436 PAY 433 

    0xf06fc3db,// 437 PAY 434 

    0xa1cbebdb,// 438 PAY 435 

    0xa3d29678,// 439 PAY 436 

    0xd375c118,// 440 PAY 437 

    0x8c7d0dd4,// 441 PAY 438 

    0x2bad7724,// 442 PAY 439 

    0x474aac14,// 443 PAY 440 

    0x36c8e129,// 444 PAY 441 

    0x80a23305,// 445 PAY 442 

    0xc683ce86,// 446 PAY 443 

    0xb6a3dc5a,// 447 PAY 444 

    0x14c05454,// 448 PAY 445 

    0xc9897f13,// 449 PAY 446 

    0xa55edd87,// 450 PAY 447 

    0x4f90d4dd,// 451 PAY 448 

    0xd7db5cf9,// 452 PAY 449 

    0xe9e0f913,// 453 PAY 450 

    0x455c2c43,// 454 PAY 451 

    0x656b579c,// 455 PAY 452 

    0x33a077e3,// 456 PAY 453 

    0x4976623e,// 457 PAY 454 

    0xb2dc6875,// 458 PAY 455 

    0x98208aa6,// 459 PAY 456 

    0xf6abfcca,// 460 PAY 457 

    0x15156318,// 461 PAY 458 

    0x387bc325,// 462 PAY 459 

    0xe464e551,// 463 PAY 460 

    0x1942c31a,// 464 PAY 461 

    0x7348055e,// 465 PAY 462 

    0x21ef4a0a,// 466 PAY 463 

    0x13095a7c,// 467 PAY 464 

    0x2df976e5,// 468 PAY 465 

    0x258e5144,// 469 PAY 466 

    0x23f1659a,// 470 PAY 467 

    0x7dd970d3,// 471 PAY 468 

    0x46c855b0,// 472 PAY 469 

    0xd7000000,// 473 PAY 470 

/// HASH is  8 bytes 

    0xdbb72a18,// 474 HSH   1 

    0xbc73f998,// 475 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 112 

/// STA pkt_idx        : 18 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb1 

    0x0049b170 // 476 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt4_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 335 words. 

/// BDA size     is 1335 (0x537) 

/// BDA id       is 0xff9 

    0x05370ff9,// 3 BDA   1 

/// PAY Generic Data size   : 1335 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x6ff827f6,// 4 PAY   1 

    0x9cc3ea28,// 5 PAY   2 

    0xac2d1bc1,// 6 PAY   3 

    0x2db725a8,// 7 PAY   4 

    0x7e20c550,// 8 PAY   5 

    0x69d1ab36,// 9 PAY   6 

    0x07e6345e,// 10 PAY   7 

    0x47130e20,// 11 PAY   8 

    0xbf76a80e,// 12 PAY   9 

    0x35cff346,// 13 PAY  10 

    0x85871029,// 14 PAY  11 

    0xafc36089,// 15 PAY  12 

    0x3582746d,// 16 PAY  13 

    0x95c261a3,// 17 PAY  14 

    0xa085123b,// 18 PAY  15 

    0xf2e953c3,// 19 PAY  16 

    0x70a19794,// 20 PAY  17 

    0x07a7666b,// 21 PAY  18 

    0x02c4c9bf,// 22 PAY  19 

    0xff3a0db8,// 23 PAY  20 

    0x8cce456b,// 24 PAY  21 

    0x2f027094,// 25 PAY  22 

    0xaf13ad86,// 26 PAY  23 

    0x13a184df,// 27 PAY  24 

    0x191ddb8e,// 28 PAY  25 

    0xe9abc9e4,// 29 PAY  26 

    0x96584127,// 30 PAY  27 

    0x37a1c59d,// 31 PAY  28 

    0x757a984b,// 32 PAY  29 

    0xa625d354,// 33 PAY  30 

    0x71a91da8,// 34 PAY  31 

    0x04863431,// 35 PAY  32 

    0x2c69d683,// 36 PAY  33 

    0x2301bc0f,// 37 PAY  34 

    0xab1770a9,// 38 PAY  35 

    0x54a886a1,// 39 PAY  36 

    0x0f69500a,// 40 PAY  37 

    0xa2fc3871,// 41 PAY  38 

    0x296ae74e,// 42 PAY  39 

    0x6b7a1c05,// 43 PAY  40 

    0xbe8ab141,// 44 PAY  41 

    0x3def5924,// 45 PAY  42 

    0xe3493e5e,// 46 PAY  43 

    0x4b5cc6fe,// 47 PAY  44 

    0x9474d62d,// 48 PAY  45 

    0x47ed206e,// 49 PAY  46 

    0x13ecd0a2,// 50 PAY  47 

    0xa1a98002,// 51 PAY  48 

    0xf37393cc,// 52 PAY  49 

    0xe8ce4fb0,// 53 PAY  50 

    0x801ae1f0,// 54 PAY  51 

    0x5e16ec41,// 55 PAY  52 

    0x861761da,// 56 PAY  53 

    0xd07ce261,// 57 PAY  54 

    0x5985a079,// 58 PAY  55 

    0x7597cec0,// 59 PAY  56 

    0xa3bd305d,// 60 PAY  57 

    0xf93b4caa,// 61 PAY  58 

    0x89b8168e,// 62 PAY  59 

    0xf9c9a662,// 63 PAY  60 

    0x6e359b37,// 64 PAY  61 

    0xd9300cb7,// 65 PAY  62 

    0x6fcea01f,// 66 PAY  63 

    0x200622f4,// 67 PAY  64 

    0xdaae0b23,// 68 PAY  65 

    0x3de523f6,// 69 PAY  66 

    0x0d0950e5,// 70 PAY  67 

    0x3336a184,// 71 PAY  68 

    0x9ef778e1,// 72 PAY  69 

    0xe200750d,// 73 PAY  70 

    0x48ca31f7,// 74 PAY  71 

    0x6602ea14,// 75 PAY  72 

    0xe9e85bde,// 76 PAY  73 

    0xed0a6304,// 77 PAY  74 

    0x09a42be4,// 78 PAY  75 

    0x4ea4055a,// 79 PAY  76 

    0x37361084,// 80 PAY  77 

    0x3d6dd997,// 81 PAY  78 

    0x2b88ce1b,// 82 PAY  79 

    0x364dcc17,// 83 PAY  80 

    0x7c5d8b25,// 84 PAY  81 

    0x9397869c,// 85 PAY  82 

    0xd81af02d,// 86 PAY  83 

    0x551a1fde,// 87 PAY  84 

    0x42e66ddd,// 88 PAY  85 

    0xbf059a71,// 89 PAY  86 

    0x24d2e8f7,// 90 PAY  87 

    0xff51bdab,// 91 PAY  88 

    0x539b3e84,// 92 PAY  89 

    0x1c2c8a16,// 93 PAY  90 

    0x83e412ad,// 94 PAY  91 

    0x30170b18,// 95 PAY  92 

    0xdb182514,// 96 PAY  93 

    0xf7ea7757,// 97 PAY  94 

    0x195e7322,// 98 PAY  95 

    0x98df5c21,// 99 PAY  96 

    0x9361baf9,// 100 PAY  97 

    0xbe5518c6,// 101 PAY  98 

    0xc34b77ff,// 102 PAY  99 

    0xe9e5a073,// 103 PAY 100 

    0xbf79aabf,// 104 PAY 101 

    0x6fd57898,// 105 PAY 102 

    0x6ca3ab7b,// 106 PAY 103 

    0xe76ed1c4,// 107 PAY 104 

    0x69f4cdc9,// 108 PAY 105 

    0xd20701ed,// 109 PAY 106 

    0x0556716e,// 110 PAY 107 

    0x73bbb3eb,// 111 PAY 108 

    0x0f0a809d,// 112 PAY 109 

    0x8a48cf0e,// 113 PAY 110 

    0xe19f76a6,// 114 PAY 111 

    0x8a61d4cc,// 115 PAY 112 

    0x62c3b35c,// 116 PAY 113 

    0x401e8f70,// 117 PAY 114 

    0x98ea7123,// 118 PAY 115 

    0x71dec9aa,// 119 PAY 116 

    0xf34c0e0b,// 120 PAY 117 

    0x48725294,// 121 PAY 118 

    0x53986186,// 122 PAY 119 

    0x6cb5f62c,// 123 PAY 120 

    0x4a0d1714,// 124 PAY 121 

    0x648c34a1,// 125 PAY 122 

    0xac74e211,// 126 PAY 123 

    0x90766a9f,// 127 PAY 124 

    0x1b8b35a3,// 128 PAY 125 

    0x8a0609d6,// 129 PAY 126 

    0x68366476,// 130 PAY 127 

    0x35349390,// 131 PAY 128 

    0xb4922eba,// 132 PAY 129 

    0x96aa416e,// 133 PAY 130 

    0xe5826cbf,// 134 PAY 131 

    0xf9c2dc4e,// 135 PAY 132 

    0x36592988,// 136 PAY 133 

    0x11cc38f7,// 137 PAY 134 

    0x978a8e15,// 138 PAY 135 

    0x1608cce9,// 139 PAY 136 

    0x0dbbd88e,// 140 PAY 137 

    0x8a532de0,// 141 PAY 138 

    0x1d3dfa1a,// 142 PAY 139 

    0x82ad0f6d,// 143 PAY 140 

    0x473e7623,// 144 PAY 141 

    0x5df047ee,// 145 PAY 142 

    0xaa5c37a5,// 146 PAY 143 

    0x13a7d3aa,// 147 PAY 144 

    0x7eb9f38f,// 148 PAY 145 

    0xd346f664,// 149 PAY 146 

    0x322297f6,// 150 PAY 147 

    0xa07d6e32,// 151 PAY 148 

    0x56a22c7b,// 152 PAY 149 

    0xac415d96,// 153 PAY 150 

    0xfcd5655f,// 154 PAY 151 

    0xae864614,// 155 PAY 152 

    0xfeed1650,// 156 PAY 153 

    0xec8bb65d,// 157 PAY 154 

    0xef48e422,// 158 PAY 155 

    0x11aff699,// 159 PAY 156 

    0x37890d79,// 160 PAY 157 

    0x042645c3,// 161 PAY 158 

    0x0b4d1503,// 162 PAY 159 

    0x9d2da9ed,// 163 PAY 160 

    0xf5239ed1,// 164 PAY 161 

    0x2626b2be,// 165 PAY 162 

    0xe344693f,// 166 PAY 163 

    0xfbd845bb,// 167 PAY 164 

    0xf4073a21,// 168 PAY 165 

    0x9364a32f,// 169 PAY 166 

    0xd1e67e3e,// 170 PAY 167 

    0x9bddb148,// 171 PAY 168 

    0xb38c9cca,// 172 PAY 169 

    0x9b21e933,// 173 PAY 170 

    0xda08d07b,// 174 PAY 171 

    0x6f9f2814,// 175 PAY 172 

    0xac28e3a1,// 176 PAY 173 

    0x1938a4c3,// 177 PAY 174 

    0x0fad7f32,// 178 PAY 175 

    0x8cf65bc8,// 179 PAY 176 

    0x13b37275,// 180 PAY 177 

    0xd20407c3,// 181 PAY 178 

    0x6042838d,// 182 PAY 179 

    0x6660fdd4,// 183 PAY 180 

    0x01cf57ab,// 184 PAY 181 

    0xc08a1e65,// 185 PAY 182 

    0x8a260939,// 186 PAY 183 

    0x4ca2c663,// 187 PAY 184 

    0xd571cddf,// 188 PAY 185 

    0x17be8fb0,// 189 PAY 186 

    0xe7c59c35,// 190 PAY 187 

    0xf5acc4fb,// 191 PAY 188 

    0x12f96b41,// 192 PAY 189 

    0xfc65da29,// 193 PAY 190 

    0x88a9dfe9,// 194 PAY 191 

    0x88711fb7,// 195 PAY 192 

    0x87cee88a,// 196 PAY 193 

    0x88f42ab9,// 197 PAY 194 

    0xf1b12370,// 198 PAY 195 

    0xe72b59f3,// 199 PAY 196 

    0x68957171,// 200 PAY 197 

    0x4143f4e9,// 201 PAY 198 

    0x3cd9fe36,// 202 PAY 199 

    0x31417479,// 203 PAY 200 

    0xf6a0f884,// 204 PAY 201 

    0xe01b1257,// 205 PAY 202 

    0xb34b856b,// 206 PAY 203 

    0x06b43c3f,// 207 PAY 204 

    0xa1de3286,// 208 PAY 205 

    0xdbbf09bc,// 209 PAY 206 

    0x604d0ea9,// 210 PAY 207 

    0xd9595b6b,// 211 PAY 208 

    0xa4c95238,// 212 PAY 209 

    0x60fa19cd,// 213 PAY 210 

    0xa850065e,// 214 PAY 211 

    0x84fd25ac,// 215 PAY 212 

    0x4a676e02,// 216 PAY 213 

    0x56cc6287,// 217 PAY 214 

    0xbe5963b0,// 218 PAY 215 

    0xaeee4bf4,// 219 PAY 216 

    0xf5579467,// 220 PAY 217 

    0x41da59ec,// 221 PAY 218 

    0x122e52be,// 222 PAY 219 

    0xb8da859d,// 223 PAY 220 

    0x3cc1c619,// 224 PAY 221 

    0x00f5ddc7,// 225 PAY 222 

    0x0b54faca,// 226 PAY 223 

    0x7fb1a00d,// 227 PAY 224 

    0xca36c0fe,// 228 PAY 225 

    0x987f9e50,// 229 PAY 226 

    0xc760e710,// 230 PAY 227 

    0xe16f0238,// 231 PAY 228 

    0x5b328b89,// 232 PAY 229 

    0x3742cc34,// 233 PAY 230 

    0x75fc7a22,// 234 PAY 231 

    0x0b395458,// 235 PAY 232 

    0x8544dc38,// 236 PAY 233 

    0x2f7bf171,// 237 PAY 234 

    0x22bb2b6e,// 238 PAY 235 

    0xb0ebcbf5,// 239 PAY 236 

    0x50b68810,// 240 PAY 237 

    0xaaa73335,// 241 PAY 238 

    0x1343b49e,// 242 PAY 239 

    0xa7479902,// 243 PAY 240 

    0x434cedf9,// 244 PAY 241 

    0x58f953d3,// 245 PAY 242 

    0x73f41806,// 246 PAY 243 

    0x379f2fcc,// 247 PAY 244 

    0xaebdc9aa,// 248 PAY 245 

    0xc4f5d18f,// 249 PAY 246 

    0xf04fc99e,// 250 PAY 247 

    0xbebe6017,// 251 PAY 248 

    0xbe17a86d,// 252 PAY 249 

    0xb6861baa,// 253 PAY 250 

    0x0e4d5f68,// 254 PAY 251 

    0xc8d14490,// 255 PAY 252 

    0x91769e8b,// 256 PAY 253 

    0x0be3ad28,// 257 PAY 254 

    0x6d7fc0bd,// 258 PAY 255 

    0x05b755a7,// 259 PAY 256 

    0x135671e0,// 260 PAY 257 

    0xeeaa2d6f,// 261 PAY 258 

    0x952c1735,// 262 PAY 259 

    0xfc54cbf9,// 263 PAY 260 

    0x072391f9,// 264 PAY 261 

    0x94e6081d,// 265 PAY 262 

    0xc420c988,// 266 PAY 263 

    0xb1665448,// 267 PAY 264 

    0x877b61de,// 268 PAY 265 

    0x3431561d,// 269 PAY 266 

    0x13930043,// 270 PAY 267 

    0xef0ad1eb,// 271 PAY 268 

    0xe9f8cac3,// 272 PAY 269 

    0xa6ecfabf,// 273 PAY 270 

    0xb272f0e7,// 274 PAY 271 

    0x4dfbeb43,// 275 PAY 272 

    0x4a430343,// 276 PAY 273 

    0xe546afeb,// 277 PAY 274 

    0x4aca0e97,// 278 PAY 275 

    0xde3d73eb,// 279 PAY 276 

    0x965ea15a,// 280 PAY 277 

    0x03ed58a6,// 281 PAY 278 

    0x51ddffb5,// 282 PAY 279 

    0x65d5664c,// 283 PAY 280 

    0x4f3c3448,// 284 PAY 281 

    0xca1ac0ff,// 285 PAY 282 

    0x7009fe17,// 286 PAY 283 

    0xb4507d31,// 287 PAY 284 

    0xc7493345,// 288 PAY 285 

    0x607303b1,// 289 PAY 286 

    0x3a5cddf1,// 290 PAY 287 

    0xfd592b8a,// 291 PAY 288 

    0x0f144e7f,// 292 PAY 289 

    0xe3365cd2,// 293 PAY 290 

    0x6f2f1d68,// 294 PAY 291 

    0xeefd7a40,// 295 PAY 292 

    0x61ad4a74,// 296 PAY 293 

    0x38504f8d,// 297 PAY 294 

    0x5481dadc,// 298 PAY 295 

    0x6f5b049b,// 299 PAY 296 

    0x7ea1d21a,// 300 PAY 297 

    0x76476bfe,// 301 PAY 298 

    0x7ea799e8,// 302 PAY 299 

    0x0d237105,// 303 PAY 300 

    0x1b3c13b1,// 304 PAY 301 

    0x23b89a46,// 305 PAY 302 

    0xa0e4f5bf,// 306 PAY 303 

    0x21e3f1ed,// 307 PAY 304 

    0x2b01599d,// 308 PAY 305 

    0x7a05f786,// 309 PAY 306 

    0xd9fc0b49,// 310 PAY 307 

    0x5b9b7a6d,// 311 PAY 308 

    0xb0473b73,// 312 PAY 309 

    0xe1d352d1,// 313 PAY 310 

    0x0a06ccec,// 314 PAY 311 

    0x57cc420b,// 315 PAY 312 

    0x0e4f0bcd,// 316 PAY 313 

    0xea33e395,// 317 PAY 314 

    0x3d5d550d,// 318 PAY 315 

    0x43395db7,// 319 PAY 316 

    0x58428cbc,// 320 PAY 317 

    0xaa4e6068,// 321 PAY 318 

    0xa02f4c05,// 322 PAY 319 

    0xe03f5fa8,// 323 PAY 320 

    0x74aec080,// 324 PAY 321 

    0x7d4f4966,// 325 PAY 322 

    0xc263ab75,// 326 PAY 323 

    0x0b4f5ad2,// 327 PAY 324 

    0x591025ea,// 328 PAY 325 

    0x482b578b,// 329 PAY 326 

    0x01d684ed,// 330 PAY 327 

    0x220ab8df,// 331 PAY 328 

    0x8430ba45,// 332 PAY 329 

    0xa8f56e83,// 333 PAY 330 

    0xe7fc1c20,// 334 PAY 331 

    0xabc50c0a,// 335 PAY 332 

    0x9cd7db82,// 336 PAY 333 

    0xae2f8c00,// 337 PAY 334 

/// HASH is  16 bytes 

    0xac415d96,// 338 HSH   1 

    0xfcd5655f,// 339 HSH   2 

    0xae864614,// 340 HSH   3 

    0xfeed1650,// 341 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 162 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0051cea2 // 342 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt5_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 507 words. 

/// BDA size     is 2024 (0x7e8) 

/// BDA id       is 0x93b0 

    0x07e893b0,// 3 BDA   1 

/// PAY Generic Data size   : 2024 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x5fc230c1,// 4 PAY   1 

    0x2081d301,// 5 PAY   2 

    0x83c386d8,// 6 PAY   3 

    0xf9b79e7a,// 7 PAY   4 

    0x0213824b,// 8 PAY   5 

    0xfb8e34d0,// 9 PAY   6 

    0xa270b7a0,// 10 PAY   7 

    0x04b40780,// 11 PAY   8 

    0xb65480d0,// 12 PAY   9 

    0x4c17e57a,// 13 PAY  10 

    0xf956f3ce,// 14 PAY  11 

    0x50f284e7,// 15 PAY  12 

    0xbed5f332,// 16 PAY  13 

    0x3ef08e62,// 17 PAY  14 

    0xd0e77981,// 18 PAY  15 

    0xf8f0ba1c,// 19 PAY  16 

    0xd9f79ed4,// 20 PAY  17 

    0x3057e87c,// 21 PAY  18 

    0xe9d63259,// 22 PAY  19 

    0x4b464d65,// 23 PAY  20 

    0x141b2bbe,// 24 PAY  21 

    0xae74f3f0,// 25 PAY  22 

    0x34c15605,// 26 PAY  23 

    0xaf71562f,// 27 PAY  24 

    0x6e49e2ee,// 28 PAY  25 

    0xb59942a1,// 29 PAY  26 

    0x1b51f826,// 30 PAY  27 

    0x4a73113f,// 31 PAY  28 

    0x640eba58,// 32 PAY  29 

    0x2b163ef0,// 33 PAY  30 

    0xa4d3c018,// 34 PAY  31 

    0x83ef563a,// 35 PAY  32 

    0xe39f11ef,// 36 PAY  33 

    0x23ef7055,// 37 PAY  34 

    0x4a3cc2c3,// 38 PAY  35 

    0x8741858f,// 39 PAY  36 

    0xf4a023e9,// 40 PAY  37 

    0x144cf4be,// 41 PAY  38 

    0xc2531afe,// 42 PAY  39 

    0xf7ae99ce,// 43 PAY  40 

    0xcd9f338e,// 44 PAY  41 

    0x8467d369,// 45 PAY  42 

    0x1d51d005,// 46 PAY  43 

    0x3d5f4569,// 47 PAY  44 

    0xe48bf5e4,// 48 PAY  45 

    0x28776b32,// 49 PAY  46 

    0x3f382c3c,// 50 PAY  47 

    0xa1cec91e,// 51 PAY  48 

    0x77bd6663,// 52 PAY  49 

    0x88ee59c9,// 53 PAY  50 

    0x15c1728c,// 54 PAY  51 

    0x789ebcc8,// 55 PAY  52 

    0xad18dbc2,// 56 PAY  53 

    0x883ebc4a,// 57 PAY  54 

    0xdc7a7d89,// 58 PAY  55 

    0x54472eb8,// 59 PAY  56 

    0x2a20f0e4,// 60 PAY  57 

    0x00a2c8a1,// 61 PAY  58 

    0x50c4f110,// 62 PAY  59 

    0x73df4672,// 63 PAY  60 

    0x5498649a,// 64 PAY  61 

    0xfca132ff,// 65 PAY  62 

    0xa41b4410,// 66 PAY  63 

    0x5ca52e2b,// 67 PAY  64 

    0xa33c3e41,// 68 PAY  65 

    0xb01629a3,// 69 PAY  66 

    0x3847ed47,// 70 PAY  67 

    0x7181d28b,// 71 PAY  68 

    0xcb059f47,// 72 PAY  69 

    0xab2bd516,// 73 PAY  70 

    0xc9c1c692,// 74 PAY  71 

    0x1b73406c,// 75 PAY  72 

    0x6c16fd19,// 76 PAY  73 

    0xd8486a1b,// 77 PAY  74 

    0x573b2570,// 78 PAY  75 

    0x78289c34,// 79 PAY  76 

    0x7ad75cde,// 80 PAY  77 

    0xb884e70e,// 81 PAY  78 

    0xb68a7e8a,// 82 PAY  79 

    0x69cae614,// 83 PAY  80 

    0xef66a6ba,// 84 PAY  81 

    0x3cc7c0e8,// 85 PAY  82 

    0x647a5120,// 86 PAY  83 

    0x412613da,// 87 PAY  84 

    0x85949fd2,// 88 PAY  85 

    0x44d135a9,// 89 PAY  86 

    0x545a071a,// 90 PAY  87 

    0x48b401fe,// 91 PAY  88 

    0x3c605946,// 92 PAY  89 

    0xc3ef674b,// 93 PAY  90 

    0x5d4b1578,// 94 PAY  91 

    0x3f71533a,// 95 PAY  92 

    0x974454ff,// 96 PAY  93 

    0x62332356,// 97 PAY  94 

    0xc5dd4c62,// 98 PAY  95 

    0xea4ac58f,// 99 PAY  96 

    0xcf859bdb,// 100 PAY  97 

    0x701b44da,// 101 PAY  98 

    0x2fcca7e3,// 102 PAY  99 

    0xa2781c4a,// 103 PAY 100 

    0x0fd1c5b0,// 104 PAY 101 

    0xf95e421e,// 105 PAY 102 

    0xc90ffc72,// 106 PAY 103 

    0x0184d51b,// 107 PAY 104 

    0x29277656,// 108 PAY 105 

    0x4e9d142a,// 109 PAY 106 

    0xae6dfe94,// 110 PAY 107 

    0x860e0bcb,// 111 PAY 108 

    0x54ff3c70,// 112 PAY 109 

    0xe6514eba,// 113 PAY 110 

    0xe5598a5d,// 114 PAY 111 

    0x4dc8d6fe,// 115 PAY 112 

    0xdf732149,// 116 PAY 113 

    0x293cfff3,// 117 PAY 114 

    0x040369c8,// 118 PAY 115 

    0xbafd8fc0,// 119 PAY 116 

    0xbbd333ac,// 120 PAY 117 

    0x1e3992e9,// 121 PAY 118 

    0x41311439,// 122 PAY 119 

    0xcd5886a9,// 123 PAY 120 

    0xf9aaa5ec,// 124 PAY 121 

    0x2ff71806,// 125 PAY 122 

    0xd860281b,// 126 PAY 123 

    0x3b93b7c3,// 127 PAY 124 

    0xed43c392,// 128 PAY 125 

    0x1ce0d26e,// 129 PAY 126 

    0xe2da9595,// 130 PAY 127 

    0xdb2eebb7,// 131 PAY 128 

    0x57293ad3,// 132 PAY 129 

    0xdd244f28,// 133 PAY 130 

    0xeaf6963a,// 134 PAY 131 

    0x398fdfcc,// 135 PAY 132 

    0xad862a84,// 136 PAY 133 

    0xcc2aca91,// 137 PAY 134 

    0x72410a4c,// 138 PAY 135 

    0xad7e69af,// 139 PAY 136 

    0xc4497795,// 140 PAY 137 

    0x08aa753d,// 141 PAY 138 

    0x809c0a2e,// 142 PAY 139 

    0xe894d78d,// 143 PAY 140 

    0xdfccb1cc,// 144 PAY 141 

    0xbf68058d,// 145 PAY 142 

    0x40b13faa,// 146 PAY 143 

    0x40fd2354,// 147 PAY 144 

    0x83b58e7c,// 148 PAY 145 

    0x986ebde4,// 149 PAY 146 

    0x6dc773e6,// 150 PAY 147 

    0x8c5697d2,// 151 PAY 148 

    0xdf36e2e9,// 152 PAY 149 

    0xc186d3bd,// 153 PAY 150 

    0x3af3a61b,// 154 PAY 151 

    0x8996dc04,// 155 PAY 152 

    0x90711514,// 156 PAY 153 

    0x8b6a12b1,// 157 PAY 154 

    0x4666c53c,// 158 PAY 155 

    0x6e92db6a,// 159 PAY 156 

    0x64e33b08,// 160 PAY 157 

    0x8616099a,// 161 PAY 158 

    0x3b9c36dd,// 162 PAY 159 

    0x73023e5b,// 163 PAY 160 

    0xf1cdf6d5,// 164 PAY 161 

    0x8c18c317,// 165 PAY 162 

    0x09557b5c,// 166 PAY 163 

    0xfc8303c8,// 167 PAY 164 

    0x57ccbebb,// 168 PAY 165 

    0x854fe961,// 169 PAY 166 

    0x704a962d,// 170 PAY 167 

    0xe190506c,// 171 PAY 168 

    0xef3f798d,// 172 PAY 169 

    0x217784bb,// 173 PAY 170 

    0xde8cf6bb,// 174 PAY 171 

    0x8df29e45,// 175 PAY 172 

    0x458a13e2,// 176 PAY 173 

    0x51ffda1f,// 177 PAY 174 

    0xc5b97c4f,// 178 PAY 175 

    0x8429179a,// 179 PAY 176 

    0x3f944e6b,// 180 PAY 177 

    0xc05cbbe7,// 181 PAY 178 

    0x2e14473f,// 182 PAY 179 

    0x462a3809,// 183 PAY 180 

    0x2889a507,// 184 PAY 181 

    0x598579ab,// 185 PAY 182 

    0x0f079119,// 186 PAY 183 

    0x531121d1,// 187 PAY 184 

    0x1e23d706,// 188 PAY 185 

    0x2bc80cce,// 189 PAY 186 

    0x02506f61,// 190 PAY 187 

    0x6f463e3a,// 191 PAY 188 

    0xc3f73b31,// 192 PAY 189 

    0x8965263a,// 193 PAY 190 

    0xa9ea22d3,// 194 PAY 191 

    0xec24ba94,// 195 PAY 192 

    0x6cb132df,// 196 PAY 193 

    0x8244f1f1,// 197 PAY 194 

    0xfcc2997c,// 198 PAY 195 

    0x014a0456,// 199 PAY 196 

    0x4610b21a,// 200 PAY 197 

    0xc19a5255,// 201 PAY 198 

    0x0953dad7,// 202 PAY 199 

    0xca00160d,// 203 PAY 200 

    0xd23a12c0,// 204 PAY 201 

    0x10d7c6ac,// 205 PAY 202 

    0xf93d95c3,// 206 PAY 203 

    0x3b71c687,// 207 PAY 204 

    0xbdc00612,// 208 PAY 205 

    0xa2546c3b,// 209 PAY 206 

    0x5ec29d14,// 210 PAY 207 

    0x48104906,// 211 PAY 208 

    0x3f989ac7,// 212 PAY 209 

    0x9b8dcf7d,// 213 PAY 210 

    0x38acaecc,// 214 PAY 211 

    0x3d19af53,// 215 PAY 212 

    0x8b584355,// 216 PAY 213 

    0x602636ed,// 217 PAY 214 

    0x2d46bf5c,// 218 PAY 215 

    0x071b6991,// 219 PAY 216 

    0x715ce77a,// 220 PAY 217 

    0x634502a6,// 221 PAY 218 

    0xcf99015c,// 222 PAY 219 

    0x9866347f,// 223 PAY 220 

    0xdf622071,// 224 PAY 221 

    0x76926942,// 225 PAY 222 

    0xa860b4ff,// 226 PAY 223 

    0x39845b11,// 227 PAY 224 

    0x2299f87b,// 228 PAY 225 

    0x91f0ea5e,// 229 PAY 226 

    0x938aac17,// 230 PAY 227 

    0x1c05832e,// 231 PAY 228 

    0x0aee591d,// 232 PAY 229 

    0x8a6c797b,// 233 PAY 230 

    0x763460ea,// 234 PAY 231 

    0x614b56dd,// 235 PAY 232 

    0xa8d7d352,// 236 PAY 233 

    0xab3fc4b9,// 237 PAY 234 

    0x35bfaf24,// 238 PAY 235 

    0x195a12ed,// 239 PAY 236 

    0xe5f7d049,// 240 PAY 237 

    0x6d24301a,// 241 PAY 238 

    0x6b4bd631,// 242 PAY 239 

    0x97afe29e,// 243 PAY 240 

    0x9e3b8d16,// 244 PAY 241 

    0x1896f3a5,// 245 PAY 242 

    0x74d79a89,// 246 PAY 243 

    0x574d4663,// 247 PAY 244 

    0x908fa4b6,// 248 PAY 245 

    0x187b028d,// 249 PAY 246 

    0xd603d19e,// 250 PAY 247 

    0x17b3cf22,// 251 PAY 248 

    0xc21190df,// 252 PAY 249 

    0x1b0444c1,// 253 PAY 250 

    0x483cbd00,// 254 PAY 251 

    0x66d0bc36,// 255 PAY 252 

    0x5987cfd9,// 256 PAY 253 

    0xcae2681a,// 257 PAY 254 

    0x706a8b9d,// 258 PAY 255 

    0xfd8079ca,// 259 PAY 256 

    0x0e74cb2f,// 260 PAY 257 

    0x3ac9ec53,// 261 PAY 258 

    0x19863774,// 262 PAY 259 

    0x2157058b,// 263 PAY 260 

    0x19721f75,// 264 PAY 261 

    0x1365670a,// 265 PAY 262 

    0x79f9bb96,// 266 PAY 263 

    0xaab64ce6,// 267 PAY 264 

    0xd1535d41,// 268 PAY 265 

    0x4d9c3949,// 269 PAY 266 

    0xda01dac8,// 270 PAY 267 

    0x729d9c8d,// 271 PAY 268 

    0x1ab53f35,// 272 PAY 269 

    0xc3230680,// 273 PAY 270 

    0x41832aff,// 274 PAY 271 

    0x31377cec,// 275 PAY 272 

    0x9fffa8ea,// 276 PAY 273 

    0x989e548e,// 277 PAY 274 

    0xea52ee8a,// 278 PAY 275 

    0x4131eba8,// 279 PAY 276 

    0xa1f5bceb,// 280 PAY 277 

    0xd1656c1c,// 281 PAY 278 

    0x62b05625,// 282 PAY 279 

    0x1138c1ec,// 283 PAY 280 

    0x940eb47a,// 284 PAY 281 

    0x0b0f938c,// 285 PAY 282 

    0x95142c8c,// 286 PAY 283 

    0x9139667f,// 287 PAY 284 

    0x16132263,// 288 PAY 285 

    0xd8718112,// 289 PAY 286 

    0xdce92160,// 290 PAY 287 

    0xfc63beca,// 291 PAY 288 

    0x2c0ce901,// 292 PAY 289 

    0x89f2f523,// 293 PAY 290 

    0xbe0521f5,// 294 PAY 291 

    0xaf28c5b3,// 295 PAY 292 

    0x3541a454,// 296 PAY 293 

    0xb3f0a4a0,// 297 PAY 294 

    0xa892c32f,// 298 PAY 295 

    0x7add54ff,// 299 PAY 296 

    0xd48f4833,// 300 PAY 297 

    0x2e5e309e,// 301 PAY 298 

    0x3bab1ddd,// 302 PAY 299 

    0x2c213f80,// 303 PAY 300 

    0x199b2655,// 304 PAY 301 

    0x15f5a912,// 305 PAY 302 

    0xc0c7689e,// 306 PAY 303 

    0xba46dfa4,// 307 PAY 304 

    0xa2e735b7,// 308 PAY 305 

    0x0b2cdbb2,// 309 PAY 306 

    0x969f020f,// 310 PAY 307 

    0xf14b60ae,// 311 PAY 308 

    0x5949dcc8,// 312 PAY 309 

    0x1ff3ae6d,// 313 PAY 310 

    0xbd3f2958,// 314 PAY 311 

    0x01221238,// 315 PAY 312 

    0x952af91b,// 316 PAY 313 

    0x114a7eb9,// 317 PAY 314 

    0x859cb1c4,// 318 PAY 315 

    0x4185dc14,// 319 PAY 316 

    0x395fd167,// 320 PAY 317 

    0xcb1b402a,// 321 PAY 318 

    0x27ca7c73,// 322 PAY 319 

    0xf1c740e0,// 323 PAY 320 

    0xfdce2042,// 324 PAY 321 

    0xdf174394,// 325 PAY 322 

    0xa0ec4862,// 326 PAY 323 

    0x31b5fcb4,// 327 PAY 324 

    0x5bd43278,// 328 PAY 325 

    0xa6604301,// 329 PAY 326 

    0xa66b4014,// 330 PAY 327 

    0x9b28d55e,// 331 PAY 328 

    0xb509c659,// 332 PAY 329 

    0x99fec805,// 333 PAY 330 

    0xf3d3033d,// 334 PAY 331 

    0xd0a538f9,// 335 PAY 332 

    0x18eb7966,// 336 PAY 333 

    0xf53a891b,// 337 PAY 334 

    0x6b702efb,// 338 PAY 335 

    0x098168c7,// 339 PAY 336 

    0x244f9415,// 340 PAY 337 

    0x957908f5,// 341 PAY 338 

    0xf0f3513a,// 342 PAY 339 

    0x28b326c0,// 343 PAY 340 

    0xb06a6c4d,// 344 PAY 341 

    0x1170b7b3,// 345 PAY 342 

    0xd6bc2e9f,// 346 PAY 343 

    0x1d25062c,// 347 PAY 344 

    0x6d887cfa,// 348 PAY 345 

    0x084936ae,// 349 PAY 346 

    0x61a1a9c7,// 350 PAY 347 

    0x1d5ecc4f,// 351 PAY 348 

    0xbc93db76,// 352 PAY 349 

    0xd2646b08,// 353 PAY 350 

    0x726d42e1,// 354 PAY 351 

    0x56bd29ae,// 355 PAY 352 

    0xd28808b2,// 356 PAY 353 

    0x3e39abd1,// 357 PAY 354 

    0x394bd26a,// 358 PAY 355 

    0x78b792f5,// 359 PAY 356 

    0xe12253d1,// 360 PAY 357 

    0xa26b74d9,// 361 PAY 358 

    0x834e7141,// 362 PAY 359 

    0x1ce54403,// 363 PAY 360 

    0x403e622e,// 364 PAY 361 

    0x1309bf16,// 365 PAY 362 

    0x58b21a08,// 366 PAY 363 

    0x1580300d,// 367 PAY 364 

    0x6188b9e2,// 368 PAY 365 

    0x4d28521c,// 369 PAY 366 

    0xe1cd1831,// 370 PAY 367 

    0x9f9c76a0,// 371 PAY 368 

    0xce4a3006,// 372 PAY 369 

    0xcca2f26e,// 373 PAY 370 

    0xbde4f8e9,// 374 PAY 371 

    0xd5650e3f,// 375 PAY 372 

    0xd5ddfe95,// 376 PAY 373 

    0xd7c6f557,// 377 PAY 374 

    0xac95eeab,// 378 PAY 375 

    0xef10aa9f,// 379 PAY 376 

    0x1a6a4d0e,// 380 PAY 377 

    0xbf9a2122,// 381 PAY 378 

    0x75be2bc6,// 382 PAY 379 

    0x02a1e3c1,// 383 PAY 380 

    0xedb95832,// 384 PAY 381 

    0xe3d11c41,// 385 PAY 382 

    0x9dc4417b,// 386 PAY 383 

    0x36e09e91,// 387 PAY 384 

    0x18806608,// 388 PAY 385 

    0x479f0bb8,// 389 PAY 386 

    0xdd2a57c0,// 390 PAY 387 

    0x97c2ff13,// 391 PAY 388 

    0x67f77681,// 392 PAY 389 

    0xf907ce34,// 393 PAY 390 

    0x2827087e,// 394 PAY 391 

    0x23c4c63a,// 395 PAY 392 

    0x969b7f6f,// 396 PAY 393 

    0xbaad6bab,// 397 PAY 394 

    0x7bc195fd,// 398 PAY 395 

    0xcc0d8839,// 399 PAY 396 

    0xd8a7926f,// 400 PAY 397 

    0xe04cd566,// 401 PAY 398 

    0x01ab0e11,// 402 PAY 399 

    0x4077e961,// 403 PAY 400 

    0xd67aeaff,// 404 PAY 401 

    0xce4e534b,// 405 PAY 402 

    0x1d80d9c1,// 406 PAY 403 

    0x826c27c0,// 407 PAY 404 

    0x5e976a6f,// 408 PAY 405 

    0x33d59c33,// 409 PAY 406 

    0xd5f7a0e7,// 410 PAY 407 

    0x52a62da4,// 411 PAY 408 

    0xdd64e019,// 412 PAY 409 

    0x7f705095,// 413 PAY 410 

    0x60544f0c,// 414 PAY 411 

    0xe1bda549,// 415 PAY 412 

    0xfcc4a050,// 416 PAY 413 

    0xaad7c57a,// 417 PAY 414 

    0x275c100d,// 418 PAY 415 

    0x58c4d9f1,// 419 PAY 416 

    0x7d53d8eb,// 420 PAY 417 

    0xaf035cc7,// 421 PAY 418 

    0xb719b94e,// 422 PAY 419 

    0x92774db3,// 423 PAY 420 

    0x18fdc0a6,// 424 PAY 421 

    0x1f20e49b,// 425 PAY 422 

    0x958d8c38,// 426 PAY 423 

    0xb7857671,// 427 PAY 424 

    0x9f403402,// 428 PAY 425 

    0xf104b5e7,// 429 PAY 426 

    0xcf4f944b,// 430 PAY 427 

    0xfeb1e426,// 431 PAY 428 

    0x1750aa9b,// 432 PAY 429 

    0x84ae022f,// 433 PAY 430 

    0x87d04569,// 434 PAY 431 

    0x77b0fd77,// 435 PAY 432 

    0xab7a7156,// 436 PAY 433 

    0x74e8f297,// 437 PAY 434 

    0xcd846526,// 438 PAY 435 

    0xcbc071e2,// 439 PAY 436 

    0xf18a5b0c,// 440 PAY 437 

    0x581b8116,// 441 PAY 438 

    0x9a511ff0,// 442 PAY 439 

    0x822e00a3,// 443 PAY 440 

    0xd4f244ab,// 444 PAY 441 

    0xb8c7dceb,// 445 PAY 442 

    0xc9af26ab,// 446 PAY 443 

    0x6b3a0910,// 447 PAY 444 

    0xfff07428,// 448 PAY 445 

    0x6b888bea,// 449 PAY 446 

    0x0963970b,// 450 PAY 447 

    0x7625c51e,// 451 PAY 448 

    0x22a1faba,// 452 PAY 449 

    0x6d9f93d5,// 453 PAY 450 

    0x103fd05b,// 454 PAY 451 

    0x43f70739,// 455 PAY 452 

    0x99a3e1b9,// 456 PAY 453 

    0x65b572a0,// 457 PAY 454 

    0xbe3ddb86,// 458 PAY 455 

    0x886fcb76,// 459 PAY 456 

    0x7f67145e,// 460 PAY 457 

    0xfabefd80,// 461 PAY 458 

    0x6240d382,// 462 PAY 459 

    0xaec060e9,// 463 PAY 460 

    0x242b16a2,// 464 PAY 461 

    0xca9945e9,// 465 PAY 462 

    0xd042b5f3,// 466 PAY 463 

    0xdfd3b3e5,// 467 PAY 464 

    0x8e8894b7,// 468 PAY 465 

    0xd2eb2937,// 469 PAY 466 

    0xf30db6ae,// 470 PAY 467 

    0x60fef4f9,// 471 PAY 468 

    0xca6f34ec,// 472 PAY 469 

    0x46874fa1,// 473 PAY 470 

    0x411e50a1,// 474 PAY 471 

    0x122187f5,// 475 PAY 472 

    0x6cffb351,// 476 PAY 473 

    0x9d95350b,// 477 PAY 474 

    0x20f6634c,// 478 PAY 475 

    0x9e71f3e9,// 479 PAY 476 

    0x20706c5a,// 480 PAY 477 

    0x8ae925f8,// 481 PAY 478 

    0x469d1d49,// 482 PAY 479 

    0x8039921b,// 483 PAY 480 

    0x1dd73725,// 484 PAY 481 

    0x9e6564c0,// 485 PAY 482 

    0xc588cc3a,// 486 PAY 483 

    0xca694a44,// 487 PAY 484 

    0xa8ae8df3,// 488 PAY 485 

    0x0cfd390f,// 489 PAY 486 

    0xb84b9d3b,// 490 PAY 487 

    0x38a440e0,// 491 PAY 488 

    0xf1c8e1d5,// 492 PAY 489 

    0xcd688a79,// 493 PAY 490 

    0x9752956f,// 494 PAY 491 

    0x974ac115,// 495 PAY 492 

    0xbd5ac2f5,// 496 PAY 493 

    0x1a706688,// 497 PAY 494 

    0x4643e5d4,// 498 PAY 495 

    0xc883b4d2,// 499 PAY 496 

    0x074cc212,// 500 PAY 497 

    0x36f982e2,// 501 PAY 498 

    0xd6647f5e,// 502 PAY 499 

    0x1f9c6cca,// 503 PAY 500 

    0x6e7799fb,// 504 PAY 501 

    0x1dcef89f,// 505 PAY 502 

    0x5dd78891,// 506 PAY 503 

    0x40dcefa0,// 507 PAY 504 

    0x7f388d76,// 508 PAY 505 

    0x228c11b7,// 509 PAY 506 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x20 

    0x02492053 // 510 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt6_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 81 words. 

/// BDA size     is 317 (0x13d) 

/// BDA id       is 0x4456 

    0x013d4456,// 3 BDA   1 

/// PAY Generic Data size   : 317 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x558f5839,// 4 PAY   1 

    0xd60e96e6,// 5 PAY   2 

    0xf013c9ad,// 6 PAY   3 

    0x41302c10,// 7 PAY   4 

    0xedc5c112,// 8 PAY   5 

    0x5f0a810b,// 9 PAY   6 

    0xdd947454,// 10 PAY   7 

    0x7d5b771a,// 11 PAY   8 

    0xdbc88449,// 12 PAY   9 

    0xa0f9ee1d,// 13 PAY  10 

    0x001d26ea,// 14 PAY  11 

    0x03b931d1,// 15 PAY  12 

    0xf29e94ee,// 16 PAY  13 

    0xff8e4cce,// 17 PAY  14 

    0x2ba9cca7,// 18 PAY  15 

    0xb8d139ac,// 19 PAY  16 

    0xd73582fa,// 20 PAY  17 

    0xbdb43075,// 21 PAY  18 

    0x0b960f31,// 22 PAY  19 

    0x4125ea8c,// 23 PAY  20 

    0x836eca2e,// 24 PAY  21 

    0xc021df7d,// 25 PAY  22 

    0x8e2f39cb,// 26 PAY  23 

    0x04f2176c,// 27 PAY  24 

    0x2eee9128,// 28 PAY  25 

    0x8d2a805a,// 29 PAY  26 

    0xf8d3e0ab,// 30 PAY  27 

    0x30459aba,// 31 PAY  28 

    0xb7ab1541,// 32 PAY  29 

    0x172519d5,// 33 PAY  30 

    0x1e81c3ee,// 34 PAY  31 

    0xa0a4a5ee,// 35 PAY  32 

    0xb0fad917,// 36 PAY  33 

    0x5722ef5d,// 37 PAY  34 

    0x7979c0a4,// 38 PAY  35 

    0x9e3bc31d,// 39 PAY  36 

    0xd90a8f75,// 40 PAY  37 

    0xb43a9113,// 41 PAY  38 

    0x87c6dd19,// 42 PAY  39 

    0x61658089,// 43 PAY  40 

    0x1f91c27b,// 44 PAY  41 

    0xe1484be6,// 45 PAY  42 

    0xcd7f1afb,// 46 PAY  43 

    0x867d2f5f,// 47 PAY  44 

    0x1906f6a8,// 48 PAY  45 

    0x5682ae7f,// 49 PAY  46 

    0xecd0b98d,// 50 PAY  47 

    0xfd0db859,// 51 PAY  48 

    0xa1d15e61,// 52 PAY  49 

    0xdc5ad313,// 53 PAY  50 

    0xb45b44e2,// 54 PAY  51 

    0x7be6698e,// 55 PAY  52 

    0x99e2eb9c,// 56 PAY  53 

    0x28a6c679,// 57 PAY  54 

    0x6000ceb5,// 58 PAY  55 

    0x1b2a4369,// 59 PAY  56 

    0x590df091,// 60 PAY  57 

    0x80c76e2c,// 61 PAY  58 

    0x74aa2f4c,// 62 PAY  59 

    0x8f2fe166,// 63 PAY  60 

    0x630f591e,// 64 PAY  61 

    0xea35289d,// 65 PAY  62 

    0x4a016e6d,// 66 PAY  63 

    0x27f68a58,// 67 PAY  64 

    0x41781969,// 68 PAY  65 

    0x0323f2c6,// 69 PAY  66 

    0x759fe614,// 70 PAY  67 

    0x37533c0c,// 71 PAY  68 

    0x0a2f38dd,// 72 PAY  69 

    0x2a46c13b,// 73 PAY  70 

    0x4b76081e,// 74 PAY  71 

    0x6094712d,// 75 PAY  72 

    0x99189e3c,// 76 PAY  73 

    0x4e332a39,// 77 PAY  74 

    0x7e8abe05,// 78 PAY  75 

    0x741ce4ca,// 79 PAY  76 

    0x2a3c7452,// 80 PAY  77 

    0xa6a0783d,// 81 PAY  78 

    0xd5d57584,// 82 PAY  79 

    0x51000000,// 83 PAY  80 

/// HASH is  12 bytes 

    0xd73582fa,// 84 HSH   1 

    0xbdb43075,// 85 HSH   2 

    0x0b960f31,// 86 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 34 

/// STA pkt_idx        : 231 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdf 

    0x039cdf22 // 87 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt7_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 256 words. 

/// BDA size     is 1019 (0x3fb) 

/// BDA id       is 0xd61 

    0x03fb0d61,// 3 BDA   1 

/// PAY Generic Data size   : 1019 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x2c7f7a9f,// 4 PAY   1 

    0xf8798061,// 5 PAY   2 

    0x42d783db,// 6 PAY   3 

    0x8171576e,// 7 PAY   4 

    0x2423ca39,// 8 PAY   5 

    0x910e9f79,// 9 PAY   6 

    0x960acfd4,// 10 PAY   7 

    0x99ba4681,// 11 PAY   8 

    0xeef62ea6,// 12 PAY   9 

    0xccc6f040,// 13 PAY  10 

    0xe078e32d,// 14 PAY  11 

    0xcb285c3a,// 15 PAY  12 

    0x4f360876,// 16 PAY  13 

    0x5e658a7c,// 17 PAY  14 

    0x48a3dbef,// 18 PAY  15 

    0xacfb1914,// 19 PAY  16 

    0xd2b9861d,// 20 PAY  17 

    0x40c4e0bd,// 21 PAY  18 

    0x66d086f0,// 22 PAY  19 

    0x94d2db5d,// 23 PAY  20 

    0x05711aa5,// 24 PAY  21 

    0x8cc85540,// 25 PAY  22 

    0xe6500b6a,// 26 PAY  23 

    0x056bafdf,// 27 PAY  24 

    0x5f9cc538,// 28 PAY  25 

    0x064a442e,// 29 PAY  26 

    0x3232eae9,// 30 PAY  27 

    0xca61caa7,// 31 PAY  28 

    0x67233c7b,// 32 PAY  29 

    0x7d59d1b3,// 33 PAY  30 

    0x6257c98d,// 34 PAY  31 

    0x377a3efd,// 35 PAY  32 

    0xd95d9033,// 36 PAY  33 

    0x73249773,// 37 PAY  34 

    0x05e5f33d,// 38 PAY  35 

    0x44e229de,// 39 PAY  36 

    0x0821aac3,// 40 PAY  37 

    0x76b2fac0,// 41 PAY  38 

    0x23dd47f1,// 42 PAY  39 

    0x092d0954,// 43 PAY  40 

    0x0c6fef7a,// 44 PAY  41 

    0x76c0f1db,// 45 PAY  42 

    0xca7e3a89,// 46 PAY  43 

    0x1d83541e,// 47 PAY  44 

    0x8b70e3e4,// 48 PAY  45 

    0x099b5ff1,// 49 PAY  46 

    0x11ff6891,// 50 PAY  47 

    0x9bfc9214,// 51 PAY  48 

    0xdb5fe1b7,// 52 PAY  49 

    0x31034e5b,// 53 PAY  50 

    0x41de562d,// 54 PAY  51 

    0xc48a73e7,// 55 PAY  52 

    0xdb8c4716,// 56 PAY  53 

    0x9efc1aa1,// 57 PAY  54 

    0xa92c8a1f,// 58 PAY  55 

    0xdc7704be,// 59 PAY  56 

    0xf344bff8,// 60 PAY  57 

    0xebaa9218,// 61 PAY  58 

    0x14ab847e,// 62 PAY  59 

    0x90971ced,// 63 PAY  60 

    0x09d208e9,// 64 PAY  61 

    0x89f44d39,// 65 PAY  62 

    0xdbf19e8d,// 66 PAY  63 

    0x82a3bcce,// 67 PAY  64 

    0x120aed7b,// 68 PAY  65 

    0xd1da7731,// 69 PAY  66 

    0x3ae2376b,// 70 PAY  67 

    0x4427f42c,// 71 PAY  68 

    0x9289cde1,// 72 PAY  69 

    0xcad5976a,// 73 PAY  70 

    0xe1bc01e9,// 74 PAY  71 

    0x4a16994d,// 75 PAY  72 

    0x5c50e4f4,// 76 PAY  73 

    0x1962efe5,// 77 PAY  74 

    0xe1629f2b,// 78 PAY  75 

    0x700d2f70,// 79 PAY  76 

    0x6cd74a9f,// 80 PAY  77 

    0x0abfc154,// 81 PAY  78 

    0x5e4f8e80,// 82 PAY  79 

    0x9c2787a0,// 83 PAY  80 

    0x0a3a1efd,// 84 PAY  81 

    0x318b33d0,// 85 PAY  82 

    0x9b44f1a9,// 86 PAY  83 

    0x28b4c6d4,// 87 PAY  84 

    0x3c4b2c70,// 88 PAY  85 

    0x863114cb,// 89 PAY  86 

    0x4fb3b6ed,// 90 PAY  87 

    0x9865c16a,// 91 PAY  88 

    0x6abfd233,// 92 PAY  89 

    0x29faba82,// 93 PAY  90 

    0x9015a916,// 94 PAY  91 

    0x5819545e,// 95 PAY  92 

    0xbc682de2,// 96 PAY  93 

    0x05d2ba7d,// 97 PAY  94 

    0x85007315,// 98 PAY  95 

    0x36569496,// 99 PAY  96 

    0x1c07ba64,// 100 PAY  97 

    0x7aacc174,// 101 PAY  98 

    0x924e195e,// 102 PAY  99 

    0xc9b543a5,// 103 PAY 100 

    0xa3db2630,// 104 PAY 101 

    0x120dfcba,// 105 PAY 102 

    0x1681206f,// 106 PAY 103 

    0x23774655,// 107 PAY 104 

    0x2ca354d7,// 108 PAY 105 

    0xaaa735a3,// 109 PAY 106 

    0xf566c6d0,// 110 PAY 107 

    0xf404c4fe,// 111 PAY 108 

    0x3dbc3f84,// 112 PAY 109 

    0xd57ac817,// 113 PAY 110 

    0x27618d7d,// 114 PAY 111 

    0xf06c4c64,// 115 PAY 112 

    0x96b65831,// 116 PAY 113 

    0xb3c80b2d,// 117 PAY 114 

    0x97e3226c,// 118 PAY 115 

    0x770c6869,// 119 PAY 116 

    0x75662386,// 120 PAY 117 

    0x39ebaed7,// 121 PAY 118 

    0x105f929b,// 122 PAY 119 

    0xdb4b1811,// 123 PAY 120 

    0x0d2d8667,// 124 PAY 121 

    0xad4d3acf,// 125 PAY 122 

    0x27039173,// 126 PAY 123 

    0x2b424eee,// 127 PAY 124 

    0xd2405cb2,// 128 PAY 125 

    0x4df92045,// 129 PAY 126 

    0x15a6b774,// 130 PAY 127 

    0x2825361e,// 131 PAY 128 

    0xef21837d,// 132 PAY 129 

    0xea220ba4,// 133 PAY 130 

    0x98a56757,// 134 PAY 131 

    0xca2d8f45,// 135 PAY 132 

    0xaca0d8b6,// 136 PAY 133 

    0x9fe4dd46,// 137 PAY 134 

    0x9fd455db,// 138 PAY 135 

    0xfe1a08ce,// 139 PAY 136 

    0x26837694,// 140 PAY 137 

    0x81d97e19,// 141 PAY 138 

    0x918f548a,// 142 PAY 139 

    0xd5481e0d,// 143 PAY 140 

    0xbc6726df,// 144 PAY 141 

    0x13a6341b,// 145 PAY 142 

    0x60fec6b5,// 146 PAY 143 

    0xa064fc35,// 147 PAY 144 

    0x5ec29957,// 148 PAY 145 

    0xb733b88d,// 149 PAY 146 

    0x9a7e8280,// 150 PAY 147 

    0x6e1a5dde,// 151 PAY 148 

    0x5bac4870,// 152 PAY 149 

    0x867048ba,// 153 PAY 150 

    0x437dfdd4,// 154 PAY 151 

    0x03c90d27,// 155 PAY 152 

    0x3e89f59d,// 156 PAY 153 

    0x36b1f214,// 157 PAY 154 

    0x01ea3293,// 158 PAY 155 

    0x2325f93c,// 159 PAY 156 

    0x23aba90f,// 160 PAY 157 

    0x168df48c,// 161 PAY 158 

    0x9b3a279d,// 162 PAY 159 

    0x15d8bc76,// 163 PAY 160 

    0xf0d5eace,// 164 PAY 161 

    0x1e56040d,// 165 PAY 162 

    0xdec7e267,// 166 PAY 163 

    0xf6281c07,// 167 PAY 164 

    0x3739a820,// 168 PAY 165 

    0x5c56e4ef,// 169 PAY 166 

    0x63ec946f,// 170 PAY 167 

    0xd392dc91,// 171 PAY 168 

    0x89009070,// 172 PAY 169 

    0x85ed17b0,// 173 PAY 170 

    0x73ceab55,// 174 PAY 171 

    0x8ee07cff,// 175 PAY 172 

    0xeb1a971b,// 176 PAY 173 

    0x55ac654b,// 177 PAY 174 

    0x74a90072,// 178 PAY 175 

    0xfd231e42,// 179 PAY 176 

    0xf4c8d280,// 180 PAY 177 

    0xb32f987d,// 181 PAY 178 

    0xea680c5e,// 182 PAY 179 

    0x26460583,// 183 PAY 180 

    0x176bb53c,// 184 PAY 181 

    0x818bb77b,// 185 PAY 182 

    0x360e3138,// 186 PAY 183 

    0x18095884,// 187 PAY 184 

    0xfd235b3e,// 188 PAY 185 

    0x2c9cde9b,// 189 PAY 186 

    0x2a2e2eb3,// 190 PAY 187 

    0xc7294421,// 191 PAY 188 

    0x130f7947,// 192 PAY 189 

    0x8f6fc691,// 193 PAY 190 

    0x2c0ddd8e,// 194 PAY 191 

    0x017960a6,// 195 PAY 192 

    0x75b23f87,// 196 PAY 193 

    0x63efbead,// 197 PAY 194 

    0xa4111f6a,// 198 PAY 195 

    0x63048c58,// 199 PAY 196 

    0x246a8061,// 200 PAY 197 

    0x3329669e,// 201 PAY 198 

    0x833939a3,// 202 PAY 199 

    0x364738f1,// 203 PAY 200 

    0xd305bfb2,// 204 PAY 201 

    0x1be52755,// 205 PAY 202 

    0xeab155d2,// 206 PAY 203 

    0xd4090b26,// 207 PAY 204 

    0x92240d75,// 208 PAY 205 

    0x87aa63f8,// 209 PAY 206 

    0xe2bf9fb4,// 210 PAY 207 

    0x63c29f54,// 211 PAY 208 

    0x654c96f7,// 212 PAY 209 

    0x449ac839,// 213 PAY 210 

    0x634c2195,// 214 PAY 211 

    0x0f1bb796,// 215 PAY 212 

    0x12a48fac,// 216 PAY 213 

    0xd446f50d,// 217 PAY 214 

    0x145cc428,// 218 PAY 215 

    0xa65a66fb,// 219 PAY 216 

    0xe0842f32,// 220 PAY 217 

    0x94ab320f,// 221 PAY 218 

    0xeeaeac5c,// 222 PAY 219 

    0xf4b51f69,// 223 PAY 220 

    0x3847bb99,// 224 PAY 221 

    0xd884d33b,// 225 PAY 222 

    0x3f38dbe5,// 226 PAY 223 

    0xf9f758ca,// 227 PAY 224 

    0xd5e69317,// 228 PAY 225 

    0x9b408939,// 229 PAY 226 

    0xa137088a,// 230 PAY 227 

    0xad6886f4,// 231 PAY 228 

    0xfcb7dbf3,// 232 PAY 229 

    0xaa539ce4,// 233 PAY 230 

    0xb4e5c319,// 234 PAY 231 

    0x7f8b4b15,// 235 PAY 232 

    0x8730ea8b,// 236 PAY 233 

    0x143e0c3e,// 237 PAY 234 

    0x944077cd,// 238 PAY 235 

    0x1c9ca3dc,// 239 PAY 236 

    0x7cd4b19d,// 240 PAY 237 

    0xecadf052,// 241 PAY 238 

    0xd3fd1fa1,// 242 PAY 239 

    0x72831f76,// 243 PAY 240 

    0xf53daf8b,// 244 PAY 241 

    0xfc39668c,// 245 PAY 242 

    0x99e796b2,// 246 PAY 243 

    0xd5d83ed9,// 247 PAY 244 

    0x1fd2d42f,// 248 PAY 245 

    0x83799589,// 249 PAY 246 

    0xfbd64c09,// 250 PAY 247 

    0xbbbfa2eb,// 251 PAY 248 

    0xd3ab0817,// 252 PAY 249 

    0x0dc590fb,// 253 PAY 250 

    0x79d16e68,// 254 PAY 251 

    0x97e365d1,// 255 PAY 252 

    0xd71a67f7,// 256 PAY 253 

    0x05739977,// 257 PAY 254 

    0xcc301d00,// 258 PAY 255 

/// STA is 1 words. 

/// STA num_pkts       : 140 

/// STA pkt_idx        : 203 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xee 

    0x032dee8c // 259 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt8_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 281 words. 

/// BDA size     is 1120 (0x460) 

/// BDA id       is 0x5dfa 

    0x04605dfa,// 3 BDA   1 

/// PAY Generic Data size   : 1120 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x9cd900ed,// 4 PAY   1 

    0x29288a5f,// 5 PAY   2 

    0x64bda180,// 6 PAY   3 

    0xe996ed50,// 7 PAY   4 

    0xdd9ed543,// 8 PAY   5 

    0xf08c778d,// 9 PAY   6 

    0xd8dd67ea,// 10 PAY   7 

    0x7ea887f6,// 11 PAY   8 

    0x191f55f9,// 12 PAY   9 

    0xa9518ef4,// 13 PAY  10 

    0xc3cb978c,// 14 PAY  11 

    0x95c47680,// 15 PAY  12 

    0xa309130e,// 16 PAY  13 

    0xc7972f83,// 17 PAY  14 

    0xd5abfad6,// 18 PAY  15 

    0x6b1c837f,// 19 PAY  16 

    0xe9e79b17,// 20 PAY  17 

    0xa9975c98,// 21 PAY  18 

    0x1422218f,// 22 PAY  19 

    0x2ac304e4,// 23 PAY  20 

    0x0beee17a,// 24 PAY  21 

    0x05e4ff9d,// 25 PAY  22 

    0x49c0d20c,// 26 PAY  23 

    0xa8e2cac3,// 27 PAY  24 

    0x5fa1cca8,// 28 PAY  25 

    0xa2cc7d67,// 29 PAY  26 

    0x69299eb8,// 30 PAY  27 

    0x0778a559,// 31 PAY  28 

    0x7ca6ef2b,// 32 PAY  29 

    0x5ac06dce,// 33 PAY  30 

    0xd989b21f,// 34 PAY  31 

    0x43d4bf59,// 35 PAY  32 

    0x93adb471,// 36 PAY  33 

    0x45d9092b,// 37 PAY  34 

    0x51c14524,// 38 PAY  35 

    0xa987f025,// 39 PAY  36 

    0xf09a295b,// 40 PAY  37 

    0xd5904c8c,// 41 PAY  38 

    0xcd8d6854,// 42 PAY  39 

    0xeed87893,// 43 PAY  40 

    0x177dd54c,// 44 PAY  41 

    0xef02b7ba,// 45 PAY  42 

    0x9f8e3f6e,// 46 PAY  43 

    0x31d63649,// 47 PAY  44 

    0x216b7ef0,// 48 PAY  45 

    0x263d24e4,// 49 PAY  46 

    0xec05dc83,// 50 PAY  47 

    0xde932c64,// 51 PAY  48 

    0x2bcdef9e,// 52 PAY  49 

    0xa399ca56,// 53 PAY  50 

    0x6d2d173e,// 54 PAY  51 

    0x2ee23013,// 55 PAY  52 

    0xf7c2912c,// 56 PAY  53 

    0x645c1716,// 57 PAY  54 

    0x510abbed,// 58 PAY  55 

    0x7b740fc0,// 59 PAY  56 

    0x07cd872d,// 60 PAY  57 

    0x37ebb706,// 61 PAY  58 

    0x0b943e17,// 62 PAY  59 

    0x246f73f6,// 63 PAY  60 

    0x11133f2c,// 64 PAY  61 

    0x0874a8b6,// 65 PAY  62 

    0xccac69d0,// 66 PAY  63 

    0x97749535,// 67 PAY  64 

    0xa97eb3cb,// 68 PAY  65 

    0xe1e3add5,// 69 PAY  66 

    0x9e3cb55c,// 70 PAY  67 

    0x2421d8e7,// 71 PAY  68 

    0x9391fa3a,// 72 PAY  69 

    0x5d3bd7c3,// 73 PAY  70 

    0xc5443a49,// 74 PAY  71 

    0x1263c0c5,// 75 PAY  72 

    0xfbd2749b,// 76 PAY  73 

    0x55af85b8,// 77 PAY  74 

    0x2d4ad433,// 78 PAY  75 

    0xd1a0adae,// 79 PAY  76 

    0x45ceb9c9,// 80 PAY  77 

    0xb821beb4,// 81 PAY  78 

    0x50f10546,// 82 PAY  79 

    0x81feec6e,// 83 PAY  80 

    0xe0f2b3f9,// 84 PAY  81 

    0xe3ebf110,// 85 PAY  82 

    0xe6a604a2,// 86 PAY  83 

    0xa60d81bb,// 87 PAY  84 

    0x6ea3d224,// 88 PAY  85 

    0x08987351,// 89 PAY  86 

    0x469cac3a,// 90 PAY  87 

    0x6b80a72f,// 91 PAY  88 

    0x43b8d7f1,// 92 PAY  89 

    0xed8e611b,// 93 PAY  90 

    0x319d892b,// 94 PAY  91 

    0x7fe13f72,// 95 PAY  92 

    0x73a9a4a7,// 96 PAY  93 

    0x5d686f0b,// 97 PAY  94 

    0x9c9b427b,// 98 PAY  95 

    0x8a43b761,// 99 PAY  96 

    0xd7c6ade1,// 100 PAY  97 

    0x3024cd7b,// 101 PAY  98 

    0xfb789cb3,// 102 PAY  99 

    0x816e9241,// 103 PAY 100 

    0xd96de90a,// 104 PAY 101 

    0xd5eda847,// 105 PAY 102 

    0x0b672234,// 106 PAY 103 

    0x35e5357e,// 107 PAY 104 

    0x409c05ef,// 108 PAY 105 

    0x5f9fb076,// 109 PAY 106 

    0x551dfdb5,// 110 PAY 107 

    0x7a5f0588,// 111 PAY 108 

    0x795f3792,// 112 PAY 109 

    0x1da7dcc3,// 113 PAY 110 

    0xf4ef661c,// 114 PAY 111 

    0x74718e52,// 115 PAY 112 

    0x5acbf42f,// 116 PAY 113 

    0x6c3fbe02,// 117 PAY 114 

    0x651c2a63,// 118 PAY 115 

    0x298b4e80,// 119 PAY 116 

    0x44642c96,// 120 PAY 117 

    0xa087395e,// 121 PAY 118 

    0xb5f60fb5,// 122 PAY 119 

    0xc3769258,// 123 PAY 120 

    0x412873ed,// 124 PAY 121 

    0x70619484,// 125 PAY 122 

    0x0fa58aed,// 126 PAY 123 

    0x0ce06bb3,// 127 PAY 124 

    0x6dd59e69,// 128 PAY 125 

    0x5d74d1f9,// 129 PAY 126 

    0xb7dc17b5,// 130 PAY 127 

    0x2a310324,// 131 PAY 128 

    0x2c95da97,// 132 PAY 129 

    0x7633ad71,// 133 PAY 130 

    0x1088b62a,// 134 PAY 131 

    0x1701987d,// 135 PAY 132 

    0xc8d675e4,// 136 PAY 133 

    0xc107d5e3,// 137 PAY 134 

    0x76aaec28,// 138 PAY 135 

    0xf53752f0,// 139 PAY 136 

    0xb369bc8d,// 140 PAY 137 

    0xb67ebdd5,// 141 PAY 138 

    0x978c814f,// 142 PAY 139 

    0x7ef7d659,// 143 PAY 140 

    0xb9beed46,// 144 PAY 141 

    0xed0b704e,// 145 PAY 142 

    0xc9a5c8f6,// 146 PAY 143 

    0x9d24c91b,// 147 PAY 144 

    0xd808fb30,// 148 PAY 145 

    0x9191ed39,// 149 PAY 146 

    0x0c59c732,// 150 PAY 147 

    0xcc27042b,// 151 PAY 148 

    0x4288e616,// 152 PAY 149 

    0x6e13d5af,// 153 PAY 150 

    0x35ba4b49,// 154 PAY 151 

    0x63526a55,// 155 PAY 152 

    0x188e9dab,// 156 PAY 153 

    0x287082dd,// 157 PAY 154 

    0xe083c07b,// 158 PAY 155 

    0x70ffee1d,// 159 PAY 156 

    0x2586d6d1,// 160 PAY 157 

    0x2fd39fcb,// 161 PAY 158 

    0x201b05a4,// 162 PAY 159 

    0x5ae5855b,// 163 PAY 160 

    0xa40c3fd8,// 164 PAY 161 

    0x4dac5e9a,// 165 PAY 162 

    0x9f37cbe9,// 166 PAY 163 

    0x8fa86edb,// 167 PAY 164 

    0x60c44509,// 168 PAY 165 

    0xb44a92ae,// 169 PAY 166 

    0x32e1270a,// 170 PAY 167 

    0x08e80f58,// 171 PAY 168 

    0xfd30bf9d,// 172 PAY 169 

    0xb3f1cef3,// 173 PAY 170 

    0xd422f4d1,// 174 PAY 171 

    0xaea78ed5,// 175 PAY 172 

    0xe0fcb014,// 176 PAY 173 

    0x3efc9819,// 177 PAY 174 

    0xd8543b2d,// 178 PAY 175 

    0x8d181661,// 179 PAY 176 

    0x20d974cb,// 180 PAY 177 

    0x27b71c5a,// 181 PAY 178 

    0x565cc1d4,// 182 PAY 179 

    0x2ad53fba,// 183 PAY 180 

    0x8dadb7ea,// 184 PAY 181 

    0xe7068d1d,// 185 PAY 182 

    0x971f3a51,// 186 PAY 183 

    0x14e43006,// 187 PAY 184 

    0xba7b3bec,// 188 PAY 185 

    0x11f639d4,// 189 PAY 186 

    0xf830b1cd,// 190 PAY 187 

    0xace3614b,// 191 PAY 188 

    0x00a6d06b,// 192 PAY 189 

    0x9940d019,// 193 PAY 190 

    0x372964d6,// 194 PAY 191 

    0x5777e5ca,// 195 PAY 192 

    0xede07175,// 196 PAY 193 

    0x19239282,// 197 PAY 194 

    0x7cd81607,// 198 PAY 195 

    0x5e53d6f8,// 199 PAY 196 

    0x3dfd2a8a,// 200 PAY 197 

    0xe527ed12,// 201 PAY 198 

    0x9f95339c,// 202 PAY 199 

    0x852ea5a4,// 203 PAY 200 

    0x7ef01a3c,// 204 PAY 201 

    0x00557f91,// 205 PAY 202 

    0x184295a6,// 206 PAY 203 

    0xd5895e79,// 207 PAY 204 

    0xfcea90ce,// 208 PAY 205 

    0x7b171a39,// 209 PAY 206 

    0x3ace4f19,// 210 PAY 207 

    0x98efe520,// 211 PAY 208 

    0x3c9d1e0e,// 212 PAY 209 

    0x6100b714,// 213 PAY 210 

    0x87553313,// 214 PAY 211 

    0x14e79af4,// 215 PAY 212 

    0x6a1f3d90,// 216 PAY 213 

    0x241a003c,// 217 PAY 214 

    0x4ccee3d0,// 218 PAY 215 

    0xbad9e974,// 219 PAY 216 

    0x24ad06e7,// 220 PAY 217 

    0x74dc1cb9,// 221 PAY 218 

    0x56f38c98,// 222 PAY 219 

    0x68b8d1bf,// 223 PAY 220 

    0xcb2d444f,// 224 PAY 221 

    0x570b13d7,// 225 PAY 222 

    0x001db932,// 226 PAY 223 

    0x04dc1dcf,// 227 PAY 224 

    0xa922e3f6,// 228 PAY 225 

    0x0285dd48,// 229 PAY 226 

    0xf486452e,// 230 PAY 227 

    0xa71b77ee,// 231 PAY 228 

    0xebb7bb5b,// 232 PAY 229 

    0x867ce605,// 233 PAY 230 

    0x5b0fe70c,// 234 PAY 231 

    0x4b108a77,// 235 PAY 232 

    0x0b68def0,// 236 PAY 233 

    0xbc1278d8,// 237 PAY 234 

    0x98e158b0,// 238 PAY 235 

    0x509883d8,// 239 PAY 236 

    0x8cadad07,// 240 PAY 237 

    0xdd635db6,// 241 PAY 238 

    0x4b4333b6,// 242 PAY 239 

    0x90ff372d,// 243 PAY 240 

    0xd3f3ce2e,// 244 PAY 241 

    0x428f9e36,// 245 PAY 242 

    0xd12a0b4f,// 246 PAY 243 

    0xf12fd24b,// 247 PAY 244 

    0x584b9236,// 248 PAY 245 

    0xe1171f03,// 249 PAY 246 

    0x768bba28,// 250 PAY 247 

    0x0e948bc3,// 251 PAY 248 

    0xc8ebd928,// 252 PAY 249 

    0x1226fe84,// 253 PAY 250 

    0x34955c8c,// 254 PAY 251 

    0xc68d59d5,// 255 PAY 252 

    0x061d85ba,// 256 PAY 253 

    0xef0d44d3,// 257 PAY 254 

    0x9af038a8,// 258 PAY 255 

    0x085a45ea,// 259 PAY 256 

    0x171dc8c3,// 260 PAY 257 

    0x916999e2,// 261 PAY 258 

    0x664cd32d,// 262 PAY 259 

    0xa7289383,// 263 PAY 260 

    0xc27250ca,// 264 PAY 261 

    0x89706aea,// 265 PAY 262 

    0x8f1bac50,// 266 PAY 263 

    0xe156304f,// 267 PAY 264 

    0xf06956f1,// 268 PAY 265 

    0x2e15d70d,// 269 PAY 266 

    0x1dbb8aea,// 270 PAY 267 

    0x5566037f,// 271 PAY 268 

    0xaeabc387,// 272 PAY 269 

    0x22414bd5,// 273 PAY 270 

    0x74d59170,// 274 PAY 271 

    0xc07f333b,// 275 PAY 272 

    0x9da70497,// 276 PAY 273 

    0xb5248bca,// 277 PAY 274 

    0x3df17f2a,// 278 PAY 275 

    0xa056291f,// 279 PAY 276 

    0xb6cea918,// 280 PAY 277 

    0x7093d495,// 281 PAY 278 

    0x6d4d8409,// 282 PAY 279 

    0xe367ca62,// 283 PAY 280 

/// HASH is  4 bytes 

    0x70619484,// 284 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 239 

/// STA pkt_idx        : 63 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x00fc1cef // 285 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt9_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 37 words. 

/// BDA size     is 144 (0x90) 

/// BDA id       is 0xc580 

    0x0090c580,// 3 BDA   1 

/// PAY Generic Data size   : 144 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x654b1bde,// 4 PAY   1 

    0xc4cef84d,// 5 PAY   2 

    0x13b2d1b7,// 6 PAY   3 

    0x03fd1f04,// 7 PAY   4 

    0x9cf71d85,// 8 PAY   5 

    0x3102b2ba,// 9 PAY   6 

    0xa2eab23a,// 10 PAY   7 

    0x5cc5071f,// 11 PAY   8 

    0xae13f401,// 12 PAY   9 

    0x672a6420,// 13 PAY  10 

    0x2ca93146,// 14 PAY  11 

    0xeb59fcf5,// 15 PAY  12 

    0x99c14911,// 16 PAY  13 

    0x2a472931,// 17 PAY  14 

    0x07cce5a5,// 18 PAY  15 

    0xc80f05e3,// 19 PAY  16 

    0x85c4a973,// 20 PAY  17 

    0xa23cdc22,// 21 PAY  18 

    0xc140fbc2,// 22 PAY  19 

    0xbf62f425,// 23 PAY  20 

    0x5bd96823,// 24 PAY  21 

    0xf2e5ab45,// 25 PAY  22 

    0xf09c2ac7,// 26 PAY  23 

    0x083fd084,// 27 PAY  24 

    0x27774f88,// 28 PAY  25 

    0xa491c271,// 29 PAY  26 

    0xe7711103,// 30 PAY  27 

    0x7456aaa9,// 31 PAY  28 

    0xac4d04ed,// 32 PAY  29 

    0x4198a83f,// 33 PAY  30 

    0x204f2cc5,// 34 PAY  31 

    0x9a37f320,// 35 PAY  32 

    0x190837be,// 36 PAY  33 

    0x18e63571,// 37 PAY  34 

    0xb5eba53e,// 38 PAY  35 

    0x318567de,// 39 PAY  36 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x71 

    0x01317149 // 40 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt10_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 147 words. 

/// BDA size     is 584 (0x248) 

/// BDA id       is 0xdf39 

    0x0248df39,// 3 BDA   1 

/// PAY Generic Data size   : 584 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xd5eb2821,// 4 PAY   1 

    0x17e443eb,// 5 PAY   2 

    0x3474dbb2,// 6 PAY   3 

    0xcdb58b8f,// 7 PAY   4 

    0x96612f20,// 8 PAY   5 

    0xfbe3aabb,// 9 PAY   6 

    0x332ac90a,// 10 PAY   7 

    0xa49b37f9,// 11 PAY   8 

    0xdafab1e5,// 12 PAY   9 

    0x5db4cc9e,// 13 PAY  10 

    0x600a70dd,// 14 PAY  11 

    0x2a6bd1d2,// 15 PAY  12 

    0xd7f14ec6,// 16 PAY  13 

    0x7c34f5aa,// 17 PAY  14 

    0x166dbc64,// 18 PAY  15 

    0x0524dda4,// 19 PAY  16 

    0x7b144cab,// 20 PAY  17 

    0x4b809ba2,// 21 PAY  18 

    0xda8e78f5,// 22 PAY  19 

    0x60e23ed0,// 23 PAY  20 

    0xafa32971,// 24 PAY  21 

    0xea0bcb6f,// 25 PAY  22 

    0xa1716039,// 26 PAY  23 

    0xf7fe3fec,// 27 PAY  24 

    0x32211ac9,// 28 PAY  25 

    0x22d32135,// 29 PAY  26 

    0x3bba678a,// 30 PAY  27 

    0x26c74a34,// 31 PAY  28 

    0x0afb631b,// 32 PAY  29 

    0x670d8e79,// 33 PAY  30 

    0x6b0f3f1c,// 34 PAY  31 

    0x358c30ba,// 35 PAY  32 

    0x187d4102,// 36 PAY  33 

    0x163283d0,// 37 PAY  34 

    0x873f0bd3,// 38 PAY  35 

    0x325c4287,// 39 PAY  36 

    0xe3d1f490,// 40 PAY  37 

    0x5c78e992,// 41 PAY  38 

    0xf13edabc,// 42 PAY  39 

    0x2ed5f14f,// 43 PAY  40 

    0x81a5e309,// 44 PAY  41 

    0x9380c980,// 45 PAY  42 

    0x373502ff,// 46 PAY  43 

    0xec610684,// 47 PAY  44 

    0x49f286b0,// 48 PAY  45 

    0x41ac2118,// 49 PAY  46 

    0x1dc405af,// 50 PAY  47 

    0xc2050e71,// 51 PAY  48 

    0x9a6f19c8,// 52 PAY  49 

    0xc500a05f,// 53 PAY  50 

    0x832704ba,// 54 PAY  51 

    0x1d399ed1,// 55 PAY  52 

    0x285f74a0,// 56 PAY  53 

    0x5d227fb0,// 57 PAY  54 

    0x8d84a3fe,// 58 PAY  55 

    0xef377df7,// 59 PAY  56 

    0x43651da6,// 60 PAY  57 

    0x8532249b,// 61 PAY  58 

    0x2150c598,// 62 PAY  59 

    0x672e55c3,// 63 PAY  60 

    0xbc58a218,// 64 PAY  61 

    0x511dfe2c,// 65 PAY  62 

    0x3126ab6e,// 66 PAY  63 

    0xb4b08f82,// 67 PAY  64 

    0x09c915e0,// 68 PAY  65 

    0xe0a00725,// 69 PAY  66 

    0x46e69acc,// 70 PAY  67 

    0xcfe7592f,// 71 PAY  68 

    0x96e53bca,// 72 PAY  69 

    0x2a730736,// 73 PAY  70 

    0x8cccb8be,// 74 PAY  71 

    0xe69860d7,// 75 PAY  72 

    0x30f563ff,// 76 PAY  73 

    0xde4831ee,// 77 PAY  74 

    0x20aefa45,// 78 PAY  75 

    0x0a7e2c4a,// 79 PAY  76 

    0xe5357b2f,// 80 PAY  77 

    0x170e4769,// 81 PAY  78 

    0x45a26686,// 82 PAY  79 

    0x1e731e4c,// 83 PAY  80 

    0xb1cff114,// 84 PAY  81 

    0xc84cd95c,// 85 PAY  82 

    0x9c129d14,// 86 PAY  83 

    0x46b1370e,// 87 PAY  84 

    0x3c323704,// 88 PAY  85 

    0xf5f8de3d,// 89 PAY  86 

    0x31eae9b3,// 90 PAY  87 

    0x713c086e,// 91 PAY  88 

    0x7389781b,// 92 PAY  89 

    0xbecb19a7,// 93 PAY  90 

    0x619a1166,// 94 PAY  91 

    0x59d4b083,// 95 PAY  92 

    0x52a59c21,// 96 PAY  93 

    0xa1e8e64e,// 97 PAY  94 

    0xa3de5cf8,// 98 PAY  95 

    0x1f08e3ec,// 99 PAY  96 

    0xd99a2b89,// 100 PAY  97 

    0x2bf19a21,// 101 PAY  98 

    0xa7b85955,// 102 PAY  99 

    0x115a5dc1,// 103 PAY 100 

    0x9dbdc44a,// 104 PAY 101 

    0xbe641b48,// 105 PAY 102 

    0x98112f10,// 106 PAY 103 

    0xb0101577,// 107 PAY 104 

    0xaba8044c,// 108 PAY 105 

    0x01b3a9cc,// 109 PAY 106 

    0xa7197c4e,// 110 PAY 107 

    0xcc672b16,// 111 PAY 108 

    0xdd72f6c3,// 112 PAY 109 

    0x66dba2ce,// 113 PAY 110 

    0xa08746c0,// 114 PAY 111 

    0xd5097d59,// 115 PAY 112 

    0x4ae5ccf1,// 116 PAY 113 

    0x64baa585,// 117 PAY 114 

    0xfc693ffa,// 118 PAY 115 

    0x98741b8e,// 119 PAY 116 

    0xdadb4ebe,// 120 PAY 117 

    0x56a92c2d,// 121 PAY 118 

    0xb9622ab1,// 122 PAY 119 

    0x61727264,// 123 PAY 120 

    0x79856e6d,// 124 PAY 121 

    0x23c2529e,// 125 PAY 122 

    0x65095323,// 126 PAY 123 

    0xe0dad87f,// 127 PAY 124 

    0x61f3d557,// 128 PAY 125 

    0x21e9b0ff,// 129 PAY 126 

    0xc9b90567,// 130 PAY 127 

    0x7a8ccca9,// 131 PAY 128 

    0x2c6a75e8,// 132 PAY 129 

    0x3a465b44,// 133 PAY 130 

    0x915dc9ce,// 134 PAY 131 

    0xc44f92a0,// 135 PAY 132 

    0x24071ca9,// 136 PAY 133 

    0x3bd23028,// 137 PAY 134 

    0xa96c3647,// 138 PAY 135 

    0x6fd5d0ec,// 139 PAY 136 

    0x20ee49bf,// 140 PAY 137 

    0xa47606d3,// 141 PAY 138 

    0x03ad9d1d,// 142 PAY 139 

    0x82305815,// 143 PAY 140 

    0xd9812ae2,// 144 PAY 141 

    0x1209e81b,// 145 PAY 142 

    0x157f400e,// 146 PAY 143 

    0xeec2b593,// 147 PAY 144 

    0x03f66141,// 148 PAY 145 

    0x4ac2809e,// 149 PAY 146 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0359e82c // 150 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt11_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 107 words. 

/// BDA size     is 421 (0x1a5) 

/// BDA id       is 0x906 

    0x01a50906,// 3 BDA   1 

/// PAY Generic Data size   : 421 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x90c9d778,// 4 PAY   1 

    0xe4671329,// 5 PAY   2 

    0x7f1e915d,// 6 PAY   3 

    0x796cb881,// 7 PAY   4 

    0x1bd58991,// 8 PAY   5 

    0xb9113c12,// 9 PAY   6 

    0x661c2bab,// 10 PAY   7 

    0xafb8dff1,// 11 PAY   8 

    0x332895dc,// 12 PAY   9 

    0x9726416f,// 13 PAY  10 

    0xc34caf98,// 14 PAY  11 

    0x30fc2457,// 15 PAY  12 

    0x73f5b9f6,// 16 PAY  13 

    0x80f880f9,// 17 PAY  14 

    0x25e046a2,// 18 PAY  15 

    0xf5ccfaa0,// 19 PAY  16 

    0x424b7dd1,// 20 PAY  17 

    0x3c722eb3,// 21 PAY  18 

    0x7079862e,// 22 PAY  19 

    0xa01628eb,// 23 PAY  20 

    0x4a8c7623,// 24 PAY  21 

    0xe6bc295e,// 25 PAY  22 

    0x27ecd949,// 26 PAY  23 

    0x4b38f536,// 27 PAY  24 

    0x7d73541c,// 28 PAY  25 

    0x2edfb427,// 29 PAY  26 

    0x2e70c987,// 30 PAY  27 

    0x595251f8,// 31 PAY  28 

    0xb17a80f4,// 32 PAY  29 

    0xe993f175,// 33 PAY  30 

    0xf7aff672,// 34 PAY  31 

    0xc7789db4,// 35 PAY  32 

    0x517a8b03,// 36 PAY  33 

    0x2fc4aacd,// 37 PAY  34 

    0x443208c9,// 38 PAY  35 

    0x0f87e496,// 39 PAY  36 

    0xf449908a,// 40 PAY  37 

    0xce884495,// 41 PAY  38 

    0x366face1,// 42 PAY  39 

    0x6d579359,// 43 PAY  40 

    0x6542bf5d,// 44 PAY  41 

    0x1a0419a0,// 45 PAY  42 

    0x207a3c75,// 46 PAY  43 

    0xb6c56c4f,// 47 PAY  44 

    0x3570ddec,// 48 PAY  45 

    0x9f2c537a,// 49 PAY  46 

    0x689aa935,// 50 PAY  47 

    0xa8904cd9,// 51 PAY  48 

    0x76a5a665,// 52 PAY  49 

    0x2ee98b7c,// 53 PAY  50 

    0x5bd74f0f,// 54 PAY  51 

    0x712a3905,// 55 PAY  52 

    0xb79cc5f7,// 56 PAY  53 

    0xbbe2c5d3,// 57 PAY  54 

    0xb0b65a85,// 58 PAY  55 

    0x4b18c715,// 59 PAY  56 

    0x15cfe6d5,// 60 PAY  57 

    0x4fab5863,// 61 PAY  58 

    0x6978f16b,// 62 PAY  59 

    0x9c9077d6,// 63 PAY  60 

    0x5ff414ee,// 64 PAY  61 

    0xa08bf3f9,// 65 PAY  62 

    0x4f0beb38,// 66 PAY  63 

    0x978763dd,// 67 PAY  64 

    0x3c3524c4,// 68 PAY  65 

    0xf6b2fcad,// 69 PAY  66 

    0xaf29e387,// 70 PAY  67 

    0xdec87324,// 71 PAY  68 

    0xfda25476,// 72 PAY  69 

    0xaa05f7dc,// 73 PAY  70 

    0x1b881778,// 74 PAY  71 

    0x17f0d0cb,// 75 PAY  72 

    0xbdc1ae8b,// 76 PAY  73 

    0xb19c7c32,// 77 PAY  74 

    0x7b71f1d4,// 78 PAY  75 

    0x29842eed,// 79 PAY  76 

    0x65811a47,// 80 PAY  77 

    0x4ee4fabc,// 81 PAY  78 

    0xadde1832,// 82 PAY  79 

    0x20829980,// 83 PAY  80 

    0x999c63fc,// 84 PAY  81 

    0xdc99bc02,// 85 PAY  82 

    0xfc5757aa,// 86 PAY  83 

    0x0b8bdf6e,// 87 PAY  84 

    0x15d8d2f9,// 88 PAY  85 

    0xe15a1479,// 89 PAY  86 

    0xe7aa90ae,// 90 PAY  87 

    0x08ee9461,// 91 PAY  88 

    0x98873b21,// 92 PAY  89 

    0x48a03d94,// 93 PAY  90 

    0x598310a9,// 94 PAY  91 

    0x4ef7b002,// 95 PAY  92 

    0x3d88ff20,// 96 PAY  93 

    0x342ff87f,// 97 PAY  94 

    0x4a1a247d,// 98 PAY  95 

    0xf0b09c86,// 99 PAY  96 

    0x92ce69bf,// 100 PAY  97 

    0x0a40a108,// 101 PAY  98 

    0x3d0b2343,// 102 PAY  99 

    0xd59d0621,// 103 PAY 100 

    0xfb4f6faf,// 104 PAY 101 

    0x4e10f570,// 105 PAY 102 

    0x91110629,// 106 PAY 103 

    0xf2543b8c,// 107 PAY 104 

    0x7f8b8209,// 108 PAY 105 

    0x62000000,// 109 PAY 106 

/// STA is 1 words. 

/// STA num_pkts       : 10 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7c 

    0x03607c0a // 110 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt12_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 372 words. 

/// BDA size     is 1481 (0x5c9) 

/// BDA id       is 0x79ee 

    0x05c979ee,// 3 BDA   1 

/// PAY Generic Data size   : 1481 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x99ecd7e4,// 4 PAY   1 

    0x5172d006,// 5 PAY   2 

    0x19acfad4,// 6 PAY   3 

    0xc023a5c3,// 7 PAY   4 

    0x0c0a4f40,// 8 PAY   5 

    0xb384106c,// 9 PAY   6 

    0x3a5ec74e,// 10 PAY   7 

    0x84598d52,// 11 PAY   8 

    0xd17085b9,// 12 PAY   9 

    0x6ad9d9c3,// 13 PAY  10 

    0x602f30bb,// 14 PAY  11 

    0x0afda75a,// 15 PAY  12 

    0x10ca7048,// 16 PAY  13 

    0x882ea1e3,// 17 PAY  14 

    0xaf75f43d,// 18 PAY  15 

    0x5ec1855e,// 19 PAY  16 

    0x81f94cdf,// 20 PAY  17 

    0x224b9c51,// 21 PAY  18 

    0x7b3fd896,// 22 PAY  19 

    0xa0f859ea,// 23 PAY  20 

    0x09a5f6f0,// 24 PAY  21 

    0xd70a86c0,// 25 PAY  22 

    0x97881d36,// 26 PAY  23 

    0x2962bfe1,// 27 PAY  24 

    0x8611f5d9,// 28 PAY  25 

    0xe0296378,// 29 PAY  26 

    0x459f5c9a,// 30 PAY  27 

    0x913f6173,// 31 PAY  28 

    0x6d812b58,// 32 PAY  29 

    0x09062425,// 33 PAY  30 

    0xeed78987,// 34 PAY  31 

    0x2ab65813,// 35 PAY  32 

    0x7ffdb1d0,// 36 PAY  33 

    0x03a7b380,// 37 PAY  34 

    0xa91cb202,// 38 PAY  35 

    0x20bf39b0,// 39 PAY  36 

    0x3a64aa58,// 40 PAY  37 

    0x9a88c6a7,// 41 PAY  38 

    0xe2f44729,// 42 PAY  39 

    0x679db8bf,// 43 PAY  40 

    0x43286cc9,// 44 PAY  41 

    0x7e959271,// 45 PAY  42 

    0x119c95ea,// 46 PAY  43 

    0xa2035c2e,// 47 PAY  44 

    0x29a60a51,// 48 PAY  45 

    0xc19574eb,// 49 PAY  46 

    0x3c4e208a,// 50 PAY  47 

    0xdde1ba63,// 51 PAY  48 

    0xb0db2859,// 52 PAY  49 

    0xb924ba32,// 53 PAY  50 

    0x923135d3,// 54 PAY  51 

    0x48de564d,// 55 PAY  52 

    0xb057a259,// 56 PAY  53 

    0x0888c9e4,// 57 PAY  54 

    0xd201a88a,// 58 PAY  55 

    0x219cb509,// 59 PAY  56 

    0x45165732,// 60 PAY  57 

    0x11fac70d,// 61 PAY  58 

    0x6640c108,// 62 PAY  59 

    0x424c6667,// 63 PAY  60 

    0x6195b203,// 64 PAY  61 

    0x7bbbc86f,// 65 PAY  62 

    0x96dbbfca,// 66 PAY  63 

    0x5b71e02d,// 67 PAY  64 

    0x8cddfd58,// 68 PAY  65 

    0x8c8a9cb7,// 69 PAY  66 

    0x6b6be30c,// 70 PAY  67 

    0x32d72bb4,// 71 PAY  68 

    0x0f057e75,// 72 PAY  69 

    0xe3300a72,// 73 PAY  70 

    0x189351ba,// 74 PAY  71 

    0x6eaa3cd1,// 75 PAY  72 

    0xaf8dfa2f,// 76 PAY  73 

    0xa0d04a9a,// 77 PAY  74 

    0xbc7e37dc,// 78 PAY  75 

    0xe4669d59,// 79 PAY  76 

    0x6c67b314,// 80 PAY  77 

    0xd9036999,// 81 PAY  78 

    0x765fb775,// 82 PAY  79 

    0xfe36cd53,// 83 PAY  80 

    0x602067ff,// 84 PAY  81 

    0x5b9ca6bb,// 85 PAY  82 

    0xda34c1e1,// 86 PAY  83 

    0x488519f2,// 87 PAY  84 

    0x9324ce9d,// 88 PAY  85 

    0x55d70463,// 89 PAY  86 

    0x1560ac8c,// 90 PAY  87 

    0x6a246241,// 91 PAY  88 

    0xbc2a0be0,// 92 PAY  89 

    0x61d516c4,// 93 PAY  90 

    0xca721d56,// 94 PAY  91 

    0x38cc4ec9,// 95 PAY  92 

    0x462dbc41,// 96 PAY  93 

    0xe2523f45,// 97 PAY  94 

    0xb515377f,// 98 PAY  95 

    0x1222576a,// 99 PAY  96 

    0x6a2cfdfb,// 100 PAY  97 

    0x40de1567,// 101 PAY  98 

    0x790c7592,// 102 PAY  99 

    0x0b28dbde,// 103 PAY 100 

    0x331fdde7,// 104 PAY 101 

    0x8f6c9f39,// 105 PAY 102 

    0xaa699adc,// 106 PAY 103 

    0xa3179b1d,// 107 PAY 104 

    0x703357f2,// 108 PAY 105 

    0x206910de,// 109 PAY 106 

    0xb86151f8,// 110 PAY 107 

    0xfa8c377d,// 111 PAY 108 

    0xa9e08b15,// 112 PAY 109 

    0x948fc4a7,// 113 PAY 110 

    0x5584ce30,// 114 PAY 111 

    0x1c559e4e,// 115 PAY 112 

    0x434305e1,// 116 PAY 113 

    0xefdc0bb3,// 117 PAY 114 

    0xafded7cb,// 118 PAY 115 

    0x01f1116e,// 119 PAY 116 

    0x963cda0e,// 120 PAY 117 

    0xf758e163,// 121 PAY 118 

    0xe7e78135,// 122 PAY 119 

    0xdd80ab21,// 123 PAY 120 

    0x9028b462,// 124 PAY 121 

    0x26d6061d,// 125 PAY 122 

    0xce89a675,// 126 PAY 123 

    0xebb72fd4,// 127 PAY 124 

    0xe882eaec,// 128 PAY 125 

    0xf66d936d,// 129 PAY 126 

    0xc6dc7c83,// 130 PAY 127 

    0xb48292dc,// 131 PAY 128 

    0xa69f9d19,// 132 PAY 129 

    0x902df374,// 133 PAY 130 

    0x0af53e2d,// 134 PAY 131 

    0x3c4bd0c0,// 135 PAY 132 

    0xedd707a2,// 136 PAY 133 

    0x90d5beba,// 137 PAY 134 

    0xae21b402,// 138 PAY 135 

    0x50c8f1c6,// 139 PAY 136 

    0x357440fa,// 140 PAY 137 

    0xf6e7e66d,// 141 PAY 138 

    0x6e985181,// 142 PAY 139 

    0x8ab5e98e,// 143 PAY 140 

    0xe1ea2e25,// 144 PAY 141 

    0x05be7cf2,// 145 PAY 142 

    0x42b30650,// 146 PAY 143 

    0x3ae064cf,// 147 PAY 144 

    0xd2324851,// 148 PAY 145 

    0x7bbb71f9,// 149 PAY 146 

    0x2ea21afa,// 150 PAY 147 

    0x333bae1b,// 151 PAY 148 

    0x048fbb2e,// 152 PAY 149 

    0x05c42db8,// 153 PAY 150 

    0x0574fae3,// 154 PAY 151 

    0xf99a8923,// 155 PAY 152 

    0x64df8473,// 156 PAY 153 

    0xbf09069a,// 157 PAY 154 

    0x54aa474f,// 158 PAY 155 

    0x20ef967f,// 159 PAY 156 

    0x29c2e38a,// 160 PAY 157 

    0xb4472921,// 161 PAY 158 

    0x60606780,// 162 PAY 159 

    0xb14c2d2d,// 163 PAY 160 

    0x366db535,// 164 PAY 161 

    0x70f9900f,// 165 PAY 162 

    0x37aa43da,// 166 PAY 163 

    0x70aa2df9,// 167 PAY 164 

    0x5bdca226,// 168 PAY 165 

    0x1e8854bf,// 169 PAY 166 

    0xa20c1528,// 170 PAY 167 

    0x5bbdd7e9,// 171 PAY 168 

    0x767f7e2d,// 172 PAY 169 

    0x403161ce,// 173 PAY 170 

    0x1d652c27,// 174 PAY 171 

    0x73a97385,// 175 PAY 172 

    0x065efc32,// 176 PAY 173 

    0x3a68a19b,// 177 PAY 174 

    0x5ef196c9,// 178 PAY 175 

    0xdea1a479,// 179 PAY 176 

    0x1d8cadd1,// 180 PAY 177 

    0x535bef0c,// 181 PAY 178 

    0xf2f81722,// 182 PAY 179 

    0xb98eafbb,// 183 PAY 180 

    0x5e3384a9,// 184 PAY 181 

    0xb5999e61,// 185 PAY 182 

    0x3fc6c910,// 186 PAY 183 

    0x6f1a4b2a,// 187 PAY 184 

    0x21dee880,// 188 PAY 185 

    0xe04f3182,// 189 PAY 186 

    0x082bb1d5,// 190 PAY 187 

    0x6124fbd1,// 191 PAY 188 

    0xd3dd71ef,// 192 PAY 189 

    0xf672830b,// 193 PAY 190 

    0xca626fc6,// 194 PAY 191 

    0x26c3ffb8,// 195 PAY 192 

    0xa0e81871,// 196 PAY 193 

    0x2a15493e,// 197 PAY 194 

    0xf91c89df,// 198 PAY 195 

    0x42ed27b1,// 199 PAY 196 

    0x4441021b,// 200 PAY 197 

    0x028b679b,// 201 PAY 198 

    0x15fe7e60,// 202 PAY 199 

    0x9a5a4f1a,// 203 PAY 200 

    0x680f15e8,// 204 PAY 201 

    0xfaa2b0d9,// 205 PAY 202 

    0x7702e1dc,// 206 PAY 203 

    0x9d083c9e,// 207 PAY 204 

    0xd1062996,// 208 PAY 205 

    0x04260173,// 209 PAY 206 

    0x3e5e23e5,// 210 PAY 207 

    0x249b391e,// 211 PAY 208 

    0xade7a289,// 212 PAY 209 

    0xec203d52,// 213 PAY 210 

    0xe060e60e,// 214 PAY 211 

    0x6545834d,// 215 PAY 212 

    0xb967dc81,// 216 PAY 213 

    0x106875a1,// 217 PAY 214 

    0x41121130,// 218 PAY 215 

    0x3d848761,// 219 PAY 216 

    0x0325030a,// 220 PAY 217 

    0x8d328852,// 221 PAY 218 

    0xc39f8698,// 222 PAY 219 

    0xe6ba262b,// 223 PAY 220 

    0x4f8acd9c,// 224 PAY 221 

    0x22e525e0,// 225 PAY 222 

    0xbe139c13,// 226 PAY 223 

    0xded9a2bc,// 227 PAY 224 

    0x8f30d70d,// 228 PAY 225 

    0x86251fa3,// 229 PAY 226 

    0xe70b1a67,// 230 PAY 227 

    0xf926310c,// 231 PAY 228 

    0x979c09f9,// 232 PAY 229 

    0xb299e4ce,// 233 PAY 230 

    0x7f6dd4e5,// 234 PAY 231 

    0x7816a78c,// 235 PAY 232 

    0x34e448d9,// 236 PAY 233 

    0x1e78ec09,// 237 PAY 234 

    0x92cdc170,// 238 PAY 235 

    0xcb31d710,// 239 PAY 236 

    0xabcaae94,// 240 PAY 237 

    0x50e579c9,// 241 PAY 238 

    0x95ac45ed,// 242 PAY 239 

    0xf4cfc829,// 243 PAY 240 

    0x8f0dd078,// 244 PAY 241 

    0xf952f66b,// 245 PAY 242 

    0x2f979716,// 246 PAY 243 

    0x57ede1be,// 247 PAY 244 

    0xe98dddeb,// 248 PAY 245 

    0x3839e909,// 249 PAY 246 

    0x424da693,// 250 PAY 247 

    0x75761a74,// 251 PAY 248 

    0x2ce21b63,// 252 PAY 249 

    0x584d07d6,// 253 PAY 250 

    0xecc31ad0,// 254 PAY 251 

    0xe828c046,// 255 PAY 252 

    0xa839ad5c,// 256 PAY 253 

    0x63ad6e04,// 257 PAY 254 

    0x98f4dc68,// 258 PAY 255 

    0xdb064e6f,// 259 PAY 256 

    0x81f72289,// 260 PAY 257 

    0xd39a62e5,// 261 PAY 258 

    0x9c0321ef,// 262 PAY 259 

    0xdf46666d,// 263 PAY 260 

    0xaa9936c7,// 264 PAY 261 

    0x4db0d8be,// 265 PAY 262 

    0x80a8cb4c,// 266 PAY 263 

    0x1c9eb5d1,// 267 PAY 264 

    0x41eb56de,// 268 PAY 265 

    0xa8461152,// 269 PAY 266 

    0xc64ef84c,// 270 PAY 267 

    0xe48c5a7a,// 271 PAY 268 

    0xbf22bd88,// 272 PAY 269 

    0x31308a6d,// 273 PAY 270 

    0x4cac5c75,// 274 PAY 271 

    0xbce0b39d,// 275 PAY 272 

    0xa91453f9,// 276 PAY 273 

    0x502a72fc,// 277 PAY 274 

    0x0cecf0b7,// 278 PAY 275 

    0xf4e057bd,// 279 PAY 276 

    0xa40449ca,// 280 PAY 277 

    0xf248080c,// 281 PAY 278 

    0x76f6d704,// 282 PAY 279 

    0x771d18e9,// 283 PAY 280 

    0x7603bdbc,// 284 PAY 281 

    0x14e92e1d,// 285 PAY 282 

    0x33bdd99d,// 286 PAY 283 

    0x5f3a8446,// 287 PAY 284 

    0x6ec7739a,// 288 PAY 285 

    0x4cf19acc,// 289 PAY 286 

    0x5fad7607,// 290 PAY 287 

    0x1ddf0ed2,// 291 PAY 288 

    0xc5a7b582,// 292 PAY 289 

    0x882fc93d,// 293 PAY 290 

    0x8bfd35e4,// 294 PAY 291 

    0x048b99f1,// 295 PAY 292 

    0x56fe59ed,// 296 PAY 293 

    0x701facd5,// 297 PAY 294 

    0xd308c352,// 298 PAY 295 

    0xd5d0ec7b,// 299 PAY 296 

    0x28cbeb69,// 300 PAY 297 

    0x938487f0,// 301 PAY 298 

    0x8884b145,// 302 PAY 299 

    0x3dc125b4,// 303 PAY 300 

    0x9613381b,// 304 PAY 301 

    0x74b6d836,// 305 PAY 302 

    0xb8210636,// 306 PAY 303 

    0xebe518fd,// 307 PAY 304 

    0x95b7c368,// 308 PAY 305 

    0x8ac47307,// 309 PAY 306 

    0x7f55c822,// 310 PAY 307 

    0x4672ace3,// 311 PAY 308 

    0xadb76717,// 312 PAY 309 

    0x64201278,// 313 PAY 310 

    0x0302a96c,// 314 PAY 311 

    0x27197dcb,// 315 PAY 312 

    0x06cc7103,// 316 PAY 313 

    0x9b8d0c2e,// 317 PAY 314 

    0xdbb13955,// 318 PAY 315 

    0xdf1b16f4,// 319 PAY 316 

    0xe577afe6,// 320 PAY 317 

    0xbd4160f1,// 321 PAY 318 

    0x29957c64,// 322 PAY 319 

    0x5fd989e5,// 323 PAY 320 

    0xe43b4d9d,// 324 PAY 321 

    0xc0672cb7,// 325 PAY 322 

    0x0083d814,// 326 PAY 323 

    0xfd3e0ec6,// 327 PAY 324 

    0x2343c0c8,// 328 PAY 325 

    0xeb019f36,// 329 PAY 326 

    0x7c1406b4,// 330 PAY 327 

    0xfa27fb70,// 331 PAY 328 

    0xd9266c6d,// 332 PAY 329 

    0x1e4b20c7,// 333 PAY 330 

    0xdc03b762,// 334 PAY 331 

    0x82d2a6e3,// 335 PAY 332 

    0x86de51fe,// 336 PAY 333 

    0x008ab658,// 337 PAY 334 

    0x313bf032,// 338 PAY 335 

    0xc6c451f9,// 339 PAY 336 

    0x8c7b5b38,// 340 PAY 337 

    0xb541b0e0,// 341 PAY 338 

    0xe6a41674,// 342 PAY 339 

    0x7b4b5810,// 343 PAY 340 

    0x98c3091a,// 344 PAY 341 

    0x7dbb2120,// 345 PAY 342 

    0x62862bfc,// 346 PAY 343 

    0xb13090f6,// 347 PAY 344 

    0xb16b7d25,// 348 PAY 345 

    0x32ad952f,// 349 PAY 346 

    0xb801f05f,// 350 PAY 347 

    0x27a3e203,// 351 PAY 348 

    0xa3cc3af0,// 352 PAY 349 

    0x54503d34,// 353 PAY 350 

    0x97d388cf,// 354 PAY 351 

    0x91437f59,// 355 PAY 352 

    0xaa97e3ca,// 356 PAY 353 

    0xf8a7393f,// 357 PAY 354 

    0x66b2ddef,// 358 PAY 355 

    0xea428aae,// 359 PAY 356 

    0x8f926fca,// 360 PAY 357 

    0x552f323f,// 361 PAY 358 

    0x639550ad,// 362 PAY 359 

    0x7c67c046,// 363 PAY 360 

    0xdabddd42,// 364 PAY 361 

    0x371c0a5f,// 365 PAY 362 

    0xccedc26e,// 366 PAY 363 

    0x9cc2cfd8,// 367 PAY 364 

    0x0174fafe,// 368 PAY 365 

    0x318a211e,// 369 PAY 366 

    0xdc033817,// 370 PAY 367 

    0x54708f3a,// 371 PAY 368 

    0xe7c9dc5b,// 372 PAY 369 

    0x17876f16,// 373 PAY 370 

    0x54000000,// 374 PAY 371 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 70 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcf 

    0x0119cf74 // 375 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt13_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 283 words. 

/// BDA size     is 1128 (0x468) 

/// BDA id       is 0x91a 

    0x0468091a,// 3 BDA   1 

/// PAY Generic Data size   : 1128 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xe89f9cd3,// 4 PAY   1 

    0x49908891,// 5 PAY   2 

    0x03fb2584,// 6 PAY   3 

    0x20d5b349,// 7 PAY   4 

    0xfbde5fab,// 8 PAY   5 

    0x55ddfd20,// 9 PAY   6 

    0x8945cff6,// 10 PAY   7 

    0x242b1d85,// 11 PAY   8 

    0x669971a4,// 12 PAY   9 

    0x2a1eeb1b,// 13 PAY  10 

    0x1843cf3d,// 14 PAY  11 

    0x05e0adbb,// 15 PAY  12 

    0x6e441854,// 16 PAY  13 

    0x8285792d,// 17 PAY  14 

    0xfe922c31,// 18 PAY  15 

    0x86ebc963,// 19 PAY  16 

    0x04cf465f,// 20 PAY  17 

    0xce9f5a02,// 21 PAY  18 

    0xe7fd9226,// 22 PAY  19 

    0x0ad9ee69,// 23 PAY  20 

    0x9f598e83,// 24 PAY  21 

    0x9c4e66cb,// 25 PAY  22 

    0xef26feed,// 26 PAY  23 

    0xaf32caad,// 27 PAY  24 

    0x0241030f,// 28 PAY  25 

    0x9e1d59e8,// 29 PAY  26 

    0xf20c7c3b,// 30 PAY  27 

    0xa806c465,// 31 PAY  28 

    0x1a388640,// 32 PAY  29 

    0x495a3f0b,// 33 PAY  30 

    0xa1b2946e,// 34 PAY  31 

    0x6e84db7b,// 35 PAY  32 

    0xaed7c922,// 36 PAY  33 

    0x47cf11e9,// 37 PAY  34 

    0x23493f5f,// 38 PAY  35 

    0x69f419c8,// 39 PAY  36 

    0x2749a187,// 40 PAY  37 

    0x5a4347b7,// 41 PAY  38 

    0xddd65d6a,// 42 PAY  39 

    0x9210d460,// 43 PAY  40 

    0xdb11088f,// 44 PAY  41 

    0x831386f0,// 45 PAY  42 

    0x183f69e6,// 46 PAY  43 

    0x881ed6e4,// 47 PAY  44 

    0x6b653f56,// 48 PAY  45 

    0x1a6f8111,// 49 PAY  46 

    0x125dd0df,// 50 PAY  47 

    0xe674ea8a,// 51 PAY  48 

    0x9d5ed9d1,// 52 PAY  49 

    0xf270bf60,// 53 PAY  50 

    0xf86c1f88,// 54 PAY  51 

    0x432d994d,// 55 PAY  52 

    0xcd245632,// 56 PAY  53 

    0xae973ffb,// 57 PAY  54 

    0xae3976ab,// 58 PAY  55 

    0xd98802ba,// 59 PAY  56 

    0xbb966db4,// 60 PAY  57 

    0xf92e7a1a,// 61 PAY  58 

    0x9f35701c,// 62 PAY  59 

    0xc3b81626,// 63 PAY  60 

    0x6de4c956,// 64 PAY  61 

    0xbd73a86a,// 65 PAY  62 

    0x76eeda10,// 66 PAY  63 

    0x409a76e6,// 67 PAY  64 

    0x2370c406,// 68 PAY  65 

    0x2cf06569,// 69 PAY  66 

    0xd676e733,// 70 PAY  67 

    0x2d09d7db,// 71 PAY  68 

    0x58c6219e,// 72 PAY  69 

    0xce48921e,// 73 PAY  70 

    0x388c8ed7,// 74 PAY  71 

    0x6937189f,// 75 PAY  72 

    0xba92e108,// 76 PAY  73 

    0xb70ebf58,// 77 PAY  74 

    0xe262d00a,// 78 PAY  75 

    0x16ae0651,// 79 PAY  76 

    0x2bb4ab74,// 80 PAY  77 

    0xa45d3698,// 81 PAY  78 

    0x0afd1aa7,// 82 PAY  79 

    0x36c9b4a9,// 83 PAY  80 

    0xc15fc2a1,// 84 PAY  81 

    0xea161c2e,// 85 PAY  82 

    0x96d2ab0b,// 86 PAY  83 

    0x42d3e3e0,// 87 PAY  84 

    0xdf1c891a,// 88 PAY  85 

    0x50666fca,// 89 PAY  86 

    0xb69eb97e,// 90 PAY  87 

    0xc427b202,// 91 PAY  88 

    0xd55436a5,// 92 PAY  89 

    0xc044a2e6,// 93 PAY  90 

    0x6bc0e24f,// 94 PAY  91 

    0x05ae00a3,// 95 PAY  92 

    0xd069cd2a,// 96 PAY  93 

    0x9c65f476,// 97 PAY  94 

    0xfd03cb96,// 98 PAY  95 

    0x898c8fd6,// 99 PAY  96 

    0xe5094fe9,// 100 PAY  97 

    0x0c25ba47,// 101 PAY  98 

    0xcb2c94b5,// 102 PAY  99 

    0x7de7927c,// 103 PAY 100 

    0xa9aa21a2,// 104 PAY 101 

    0x6014dbac,// 105 PAY 102 

    0x31351246,// 106 PAY 103 

    0x304b649a,// 107 PAY 104 

    0x472e34b5,// 108 PAY 105 

    0x2ba30fc3,// 109 PAY 106 

    0x28452a15,// 110 PAY 107 

    0x9e10b59c,// 111 PAY 108 

    0x182e6a0a,// 112 PAY 109 

    0x2e2b4db8,// 113 PAY 110 

    0x0c2ff5d8,// 114 PAY 111 

    0x3eb23f8e,// 115 PAY 112 

    0x334f5537,// 116 PAY 113 

    0x6c9f9471,// 117 PAY 114 

    0xe6046115,// 118 PAY 115 

    0xf0c0aba8,// 119 PAY 116 

    0xfbfc4bbd,// 120 PAY 117 

    0xbf031c15,// 121 PAY 118 

    0x7fa1f758,// 122 PAY 119 

    0x3e8bf106,// 123 PAY 120 

    0x706fc565,// 124 PAY 121 

    0xb245b6b0,// 125 PAY 122 

    0xbaced57e,// 126 PAY 123 

    0xb03e5da3,// 127 PAY 124 

    0xf27233f1,// 128 PAY 125 

    0xd1688303,// 129 PAY 126 

    0xd5a4e6e5,// 130 PAY 127 

    0x917fb94a,// 131 PAY 128 

    0xdf4e978b,// 132 PAY 129 

    0x6b32b2c5,// 133 PAY 130 

    0xb0970987,// 134 PAY 131 

    0xeaa832c5,// 135 PAY 132 

    0x7f0c69e3,// 136 PAY 133 

    0x9c320b42,// 137 PAY 134 

    0x60d5f5b2,// 138 PAY 135 

    0x0b67f975,// 139 PAY 136 

    0xe267efaf,// 140 PAY 137 

    0x19c44204,// 141 PAY 138 

    0xfebaa6ef,// 142 PAY 139 

    0x9defc1cb,// 143 PAY 140 

    0x52550f1a,// 144 PAY 141 

    0x92ca8566,// 145 PAY 142 

    0xaef3b83a,// 146 PAY 143 

    0xa7ae5e07,// 147 PAY 144 

    0x0f8428d6,// 148 PAY 145 

    0x8c4d2304,// 149 PAY 146 

    0x125529a0,// 150 PAY 147 

    0x2c228b74,// 151 PAY 148 

    0xa640a0f1,// 152 PAY 149 

    0x29ad4d16,// 153 PAY 150 

    0xdec9ad19,// 154 PAY 151 

    0xe6a43e37,// 155 PAY 152 

    0xb4a14e11,// 156 PAY 153 

    0x95ad3741,// 157 PAY 154 

    0x15189841,// 158 PAY 155 

    0x4a043e15,// 159 PAY 156 

    0x40b47261,// 160 PAY 157 

    0x9a241410,// 161 PAY 158 

    0x91598154,// 162 PAY 159 

    0xc2f0fb97,// 163 PAY 160 

    0xdbc6b6ee,// 164 PAY 161 

    0xf314e5c6,// 165 PAY 162 

    0x7fdea901,// 166 PAY 163 

    0x849bcc52,// 167 PAY 164 

    0x277cc372,// 168 PAY 165 

    0x88639561,// 169 PAY 166 

    0x138f9796,// 170 PAY 167 

    0x650ba079,// 171 PAY 168 

    0xefe11bd4,// 172 PAY 169 

    0xdc614875,// 173 PAY 170 

    0xd6b56d32,// 174 PAY 171 

    0x1d000eb3,// 175 PAY 172 

    0xaf92b4d6,// 176 PAY 173 

    0x6bc5497a,// 177 PAY 174 

    0xb719666d,// 178 PAY 175 

    0x19ef9618,// 179 PAY 176 

    0xec8aea68,// 180 PAY 177 

    0x70f3c731,// 181 PAY 178 

    0x33984ab4,// 182 PAY 179 

    0x14154dc8,// 183 PAY 180 

    0xa0b9f4ff,// 184 PAY 181 

    0xca68d849,// 185 PAY 182 

    0xaa31dd0c,// 186 PAY 183 

    0x4dfa42d7,// 187 PAY 184 

    0x09761b22,// 188 PAY 185 

    0x236c828f,// 189 PAY 186 

    0x1526a848,// 190 PAY 187 

    0xa22d9e15,// 191 PAY 188 

    0xb1dc4763,// 192 PAY 189 

    0x690e3bb0,// 193 PAY 190 

    0x5916ccae,// 194 PAY 191 

    0x17030dd0,// 195 PAY 192 

    0x0c4d4a60,// 196 PAY 193 

    0x6402dbd0,// 197 PAY 194 

    0x741ea35e,// 198 PAY 195 

    0x79be86da,// 199 PAY 196 

    0x6aec87a7,// 200 PAY 197 

    0x94c56ad0,// 201 PAY 198 

    0x75e4b2ac,// 202 PAY 199 

    0x27cc49f8,// 203 PAY 200 

    0x7ab86750,// 204 PAY 201 

    0xc37d063d,// 205 PAY 202 

    0x54f3355a,// 206 PAY 203 

    0x81f5c57d,// 207 PAY 204 

    0xf737a763,// 208 PAY 205 

    0x6f429854,// 209 PAY 206 

    0x9480c4ee,// 210 PAY 207 

    0x2539bd4c,// 211 PAY 208 

    0x6b49d37c,// 212 PAY 209 

    0x01b34ad6,// 213 PAY 210 

    0x50e0495d,// 214 PAY 211 

    0x61bc1bb1,// 215 PAY 212 

    0xb14c0e1c,// 216 PAY 213 

    0x669b1dbc,// 217 PAY 214 

    0xc804c4cb,// 218 PAY 215 

    0xc3d10b07,// 219 PAY 216 

    0x9f2f6a0f,// 220 PAY 217 

    0x65d2457e,// 221 PAY 218 

    0x5141d781,// 222 PAY 219 

    0xf8b31267,// 223 PAY 220 

    0x1f0cc512,// 224 PAY 221 

    0x6366b112,// 225 PAY 222 

    0xacd4bf6f,// 226 PAY 223 

    0x63945765,// 227 PAY 224 

    0x7d769c64,// 228 PAY 225 

    0x5b6d9f47,// 229 PAY 226 

    0x50bdf654,// 230 PAY 227 

    0xecc2da01,// 231 PAY 228 

    0x1410db3b,// 232 PAY 229 

    0x94845df4,// 233 PAY 230 

    0xc7c82e44,// 234 PAY 231 

    0xc82d6312,// 235 PAY 232 

    0xbc367860,// 236 PAY 233 

    0xf97e9676,// 237 PAY 234 

    0xbf0ba480,// 238 PAY 235 

    0x9959aec6,// 239 PAY 236 

    0x81d4dbc7,// 240 PAY 237 

    0x35e61e31,// 241 PAY 238 

    0xccd584a5,// 242 PAY 239 

    0xc7b88e5a,// 243 PAY 240 

    0xa468eabc,// 244 PAY 241 

    0x2241c371,// 245 PAY 242 

    0x1d42d93d,// 246 PAY 243 

    0x88babd01,// 247 PAY 244 

    0xaf02c04d,// 248 PAY 245 

    0x5c96f279,// 249 PAY 246 

    0x93801ffb,// 250 PAY 247 

    0x707741c9,// 251 PAY 248 

    0x174fcaca,// 252 PAY 249 

    0x7e2fef27,// 253 PAY 250 

    0xcae18e65,// 254 PAY 251 

    0xd7310d0c,// 255 PAY 252 

    0x431b3c11,// 256 PAY 253 

    0x54fc0548,// 257 PAY 254 

    0x68df36d8,// 258 PAY 255 

    0x75aaa62a,// 259 PAY 256 

    0xd2aea238,// 260 PAY 257 

    0x6e5c111b,// 261 PAY 258 

    0x1e7b728a,// 262 PAY 259 

    0xf1055e4a,// 263 PAY 260 

    0xf454fa0f,// 264 PAY 261 

    0xde3c771c,// 265 PAY 262 

    0x67663e9d,// 266 PAY 263 

    0x2494046e,// 267 PAY 264 

    0x6068a1eb,// 268 PAY 265 

    0xbe1988f5,// 269 PAY 266 

    0xf109946a,// 270 PAY 267 

    0x8684ee85,// 271 PAY 268 

    0x255bd29e,// 272 PAY 269 

    0x5596b77f,// 273 PAY 270 

    0x0c8aa184,// 274 PAY 271 

    0x7acec6a3,// 275 PAY 272 

    0x2d6ece8b,// 276 PAY 273 

    0x8e35abd8,// 277 PAY 274 

    0x4e47a77f,// 278 PAY 275 

    0x4f1f5635,// 279 PAY 276 

    0x02eba323,// 280 PAY 277 

    0xf219ee99,// 281 PAY 278 

    0xc1166bb3,// 282 PAY 279 

    0x7c7efcec,// 283 PAY 280 

    0xc5d9acd0,// 284 PAY 281 

    0x721a1587,// 285 PAY 282 

/// HASH is  20 bytes 

    0xb1dc4763,// 286 HSH   1 

    0x690e3bb0,// 287 HSH   2 

    0x5916ccae,// 288 HSH   3 

    0x17030dd0,// 289 HSH   4 

    0x0c4d4a60,// 290 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 168 

/// STA pkt_idx        : 182 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3e 

    0x02d93ea8 // 291 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt14_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 304 words. 

/// BDA size     is 1212 (0x4bc) 

/// BDA id       is 0x241b 

    0x04bc241b,// 3 BDA   1 

/// PAY Generic Data size   : 1212 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x235f1c0e,// 4 PAY   1 

    0x754f6a7a,// 5 PAY   2 

    0xc7f15c1b,// 6 PAY   3 

    0x6edf4676,// 7 PAY   4 

    0xb35c17e6,// 8 PAY   5 

    0x85573caa,// 9 PAY   6 

    0xdf90feca,// 10 PAY   7 

    0x4f4e61e2,// 11 PAY   8 

    0xf92cd996,// 12 PAY   9 

    0x8cc4d616,// 13 PAY  10 

    0x5c03e85e,// 14 PAY  11 

    0x1c241b12,// 15 PAY  12 

    0x8abe65e7,// 16 PAY  13 

    0xba07ac54,// 17 PAY  14 

    0x791b4cc7,// 18 PAY  15 

    0x18b8968d,// 19 PAY  16 

    0x2837e5b2,// 20 PAY  17 

    0xefcfcfc9,// 21 PAY  18 

    0x2cbdac60,// 22 PAY  19 

    0x116439ea,// 23 PAY  20 

    0xda5876e3,// 24 PAY  21 

    0x83db4a71,// 25 PAY  22 

    0x34d2678b,// 26 PAY  23 

    0xf3950404,// 27 PAY  24 

    0x59015221,// 28 PAY  25 

    0x11af3584,// 29 PAY  26 

    0x6fc7a5bb,// 30 PAY  27 

    0x7eb771a7,// 31 PAY  28 

    0x442f9b8b,// 32 PAY  29 

    0xa1abf4d4,// 33 PAY  30 

    0x730f32ec,// 34 PAY  31 

    0x359a8e74,// 35 PAY  32 

    0x1558c17d,// 36 PAY  33 

    0x48214075,// 37 PAY  34 

    0xfc9c5452,// 38 PAY  35 

    0x09067cd6,// 39 PAY  36 

    0x1f891df4,// 40 PAY  37 

    0x061f4eb0,// 41 PAY  38 

    0x23b4a4d9,// 42 PAY  39 

    0xcdd10984,// 43 PAY  40 

    0x9e41e792,// 44 PAY  41 

    0x90b5b38d,// 45 PAY  42 

    0x1a1ac95c,// 46 PAY  43 

    0xddc92607,// 47 PAY  44 

    0x6e988d26,// 48 PAY  45 

    0xdbf31818,// 49 PAY  46 

    0x87eb25d5,// 50 PAY  47 

    0x14177f2c,// 51 PAY  48 

    0x9d78d72a,// 52 PAY  49 

    0x16758209,// 53 PAY  50 

    0x20fb7f38,// 54 PAY  51 

    0xfa8d59bb,// 55 PAY  52 

    0x5c8493b5,// 56 PAY  53 

    0xd46d5c88,// 57 PAY  54 

    0xaf5e555a,// 58 PAY  55 

    0xca86661c,// 59 PAY  56 

    0xe86d33d3,// 60 PAY  57 

    0x271ba221,// 61 PAY  58 

    0xe79df9b8,// 62 PAY  59 

    0xf25f6440,// 63 PAY  60 

    0xb4df1dba,// 64 PAY  61 

    0xb2fad567,// 65 PAY  62 

    0xec02f6fb,// 66 PAY  63 

    0x6dda3129,// 67 PAY  64 

    0x1b6061cf,// 68 PAY  65 

    0x660fdb70,// 69 PAY  66 

    0x418c303b,// 70 PAY  67 

    0x3b4cfac8,// 71 PAY  68 

    0x17ae50c7,// 72 PAY  69 

    0x3ff68bc1,// 73 PAY  70 

    0x824ec9d7,// 74 PAY  71 

    0x8aec5759,// 75 PAY  72 

    0x12fbdcee,// 76 PAY  73 

    0x1bf2e6f4,// 77 PAY  74 

    0x52d10c85,// 78 PAY  75 

    0x914ff2f4,// 79 PAY  76 

    0x6bb6ffca,// 80 PAY  77 

    0x3b02c817,// 81 PAY  78 

    0x9d332140,// 82 PAY  79 

    0xbc747569,// 83 PAY  80 

    0x39b2d6fb,// 84 PAY  81 

    0x346fc51d,// 85 PAY  82 

    0x9dc3b75a,// 86 PAY  83 

    0x276947e1,// 87 PAY  84 

    0x2f0469cd,// 88 PAY  85 

    0xb142b058,// 89 PAY  86 

    0x2450bd08,// 90 PAY  87 

    0x2dc4f0db,// 91 PAY  88 

    0x61169dd7,// 92 PAY  89 

    0x0d821471,// 93 PAY  90 

    0x093e50e6,// 94 PAY  91 

    0xd4682654,// 95 PAY  92 

    0x4547ce4a,// 96 PAY  93 

    0xebcdcecf,// 97 PAY  94 

    0xb5fc5f7e,// 98 PAY  95 

    0x78963d3c,// 99 PAY  96 

    0x098fab99,// 100 PAY  97 

    0xa04fbf21,// 101 PAY  98 

    0x5bf31364,// 102 PAY  99 

    0x8f435821,// 103 PAY 100 

    0xdfa4793f,// 104 PAY 101 

    0xf61a7f09,// 105 PAY 102 

    0x1c03e4f6,// 106 PAY 103 

    0xb599477e,// 107 PAY 104 

    0xcf29435b,// 108 PAY 105 

    0x75308fbd,// 109 PAY 106 

    0x93d5d9c6,// 110 PAY 107 

    0x93ce4c22,// 111 PAY 108 

    0x50d1a321,// 112 PAY 109 

    0x5cd62e20,// 113 PAY 110 

    0x3fb0e010,// 114 PAY 111 

    0xd44d5660,// 115 PAY 112 

    0x0b65a50e,// 116 PAY 113 

    0xf18eee89,// 117 PAY 114 

    0x7bf6a8d1,// 118 PAY 115 

    0xb72c475c,// 119 PAY 116 

    0xbd95bcba,// 120 PAY 117 

    0xc4a43ea5,// 121 PAY 118 

    0xb39e4020,// 122 PAY 119 

    0xb7501fb5,// 123 PAY 120 

    0x5fd7487a,// 124 PAY 121 

    0x17b832b0,// 125 PAY 122 

    0xf0bff337,// 126 PAY 123 

    0xae3c0afc,// 127 PAY 124 

    0x5acb2e62,// 128 PAY 125 

    0x3b85bf79,// 129 PAY 126 

    0xca7d6247,// 130 PAY 127 

    0x97b17ea1,// 131 PAY 128 

    0x01384790,// 132 PAY 129 

    0xb6856625,// 133 PAY 130 

    0x1dbe84bb,// 134 PAY 131 

    0x52150125,// 135 PAY 132 

    0x4209d7b4,// 136 PAY 133 

    0x81ee7dcc,// 137 PAY 134 

    0xa6c4f273,// 138 PAY 135 

    0xc77860cc,// 139 PAY 136 

    0xccf6ab7f,// 140 PAY 137 

    0x30b95baf,// 141 PAY 138 

    0xf7a18233,// 142 PAY 139 

    0xe5eb967c,// 143 PAY 140 

    0x13d8ec05,// 144 PAY 141 

    0xa60ea7da,// 145 PAY 142 

    0xccf557c9,// 146 PAY 143 

    0x71e42a70,// 147 PAY 144 

    0x092fbd5a,// 148 PAY 145 

    0x378c3e41,// 149 PAY 146 

    0x7f4264d8,// 150 PAY 147 

    0x04742938,// 151 PAY 148 

    0x06fba2c9,// 152 PAY 149 

    0x8a737e43,// 153 PAY 150 

    0xf41260a3,// 154 PAY 151 

    0xf9307efd,// 155 PAY 152 

    0x2943ec34,// 156 PAY 153 

    0x0e78d7f3,// 157 PAY 154 

    0x79c5e851,// 158 PAY 155 

    0xe58937b4,// 159 PAY 156 

    0x6807daa7,// 160 PAY 157 

    0xa46b3bad,// 161 PAY 158 

    0x12e26ccb,// 162 PAY 159 

    0x90248e97,// 163 PAY 160 

    0x414d597c,// 164 PAY 161 

    0x605e1d55,// 165 PAY 162 

    0xbf116639,// 166 PAY 163 

    0x2cc72228,// 167 PAY 164 

    0x93d899b6,// 168 PAY 165 

    0xfa206c3d,// 169 PAY 166 

    0xd1b20526,// 170 PAY 167 

    0xa67dd28a,// 171 PAY 168 

    0x36360958,// 172 PAY 169 

    0x1cec8c28,// 173 PAY 170 

    0x2624f7ea,// 174 PAY 171 

    0xf6e7eba3,// 175 PAY 172 

    0xfbfa9050,// 176 PAY 173 

    0x5f5aacaa,// 177 PAY 174 

    0x4f9896bb,// 178 PAY 175 

    0x4f8dda95,// 179 PAY 176 

    0xa95f1888,// 180 PAY 177 

    0xcf87073d,// 181 PAY 178 

    0x3a9742de,// 182 PAY 179 

    0x866c288a,// 183 PAY 180 

    0x15e21d49,// 184 PAY 181 

    0xbc50fb86,// 185 PAY 182 

    0x8aaf421d,// 186 PAY 183 

    0x46926b28,// 187 PAY 184 

    0x998aa1cc,// 188 PAY 185 

    0x8a850d75,// 189 PAY 186 

    0xa3b33c34,// 190 PAY 187 

    0x370c3810,// 191 PAY 188 

    0x07d2d75e,// 192 PAY 189 

    0x208c029d,// 193 PAY 190 

    0x535bfc1d,// 194 PAY 191 

    0x32e921d9,// 195 PAY 192 

    0xe332717a,// 196 PAY 193 

    0xc1141b55,// 197 PAY 194 

    0x281aad8b,// 198 PAY 195 

    0x8ef9288e,// 199 PAY 196 

    0xad79b18f,// 200 PAY 197 

    0x3723da7d,// 201 PAY 198 

    0xc15b2add,// 202 PAY 199 

    0x465fd7d1,// 203 PAY 200 

    0x77c56280,// 204 PAY 201 

    0x795d2c07,// 205 PAY 202 

    0x48e00627,// 206 PAY 203 

    0x334e679d,// 207 PAY 204 

    0x1c40c5fc,// 208 PAY 205 

    0x9647e728,// 209 PAY 206 

    0x256c2824,// 210 PAY 207 

    0x2b5c04b7,// 211 PAY 208 

    0x8c30b24d,// 212 PAY 209 

    0x7b0e4206,// 213 PAY 210 

    0xe1588dc7,// 214 PAY 211 

    0x3a274438,// 215 PAY 212 

    0x8810f2ae,// 216 PAY 213 

    0x418c6bcf,// 217 PAY 214 

    0xcb4cc53a,// 218 PAY 215 

    0x9f6e8743,// 219 PAY 216 

    0x852f536e,// 220 PAY 217 

    0x856d8cb0,// 221 PAY 218 

    0x9fadd4dd,// 222 PAY 219 

    0x7dadb272,// 223 PAY 220 

    0x9433ecb5,// 224 PAY 221 

    0x9e2f8097,// 225 PAY 222 

    0x187eeb25,// 226 PAY 223 

    0x9d77825f,// 227 PAY 224 

    0x4b2f787c,// 228 PAY 225 

    0x9f9459e2,// 229 PAY 226 

    0xb32441eb,// 230 PAY 227 

    0xc6bdc080,// 231 PAY 228 

    0x6333b17c,// 232 PAY 229 

    0x482afefe,// 233 PAY 230 

    0x960f6604,// 234 PAY 231 

    0xfb96a0dd,// 235 PAY 232 

    0xbb9ffb8b,// 236 PAY 233 

    0x982a3bc2,// 237 PAY 234 

    0xf5cca740,// 238 PAY 235 

    0x8144bfe3,// 239 PAY 236 

    0x25347edb,// 240 PAY 237 

    0x2b17a5db,// 241 PAY 238 

    0xae6bcb48,// 242 PAY 239 

    0x21ec4e90,// 243 PAY 240 

    0x51edbc61,// 244 PAY 241 

    0x643ead67,// 245 PAY 242 

    0xd23f62b4,// 246 PAY 243 

    0xff714e6d,// 247 PAY 244 

    0x406be718,// 248 PAY 245 

    0xec279611,// 249 PAY 246 

    0x0211af75,// 250 PAY 247 

    0x776ad587,// 251 PAY 248 

    0x9adc7029,// 252 PAY 249 

    0xec0cbde5,// 253 PAY 250 

    0xd9b18a6f,// 254 PAY 251 

    0xf39f748a,// 255 PAY 252 

    0x21d9469a,// 256 PAY 253 

    0x80465486,// 257 PAY 254 

    0x8bfdd97d,// 258 PAY 255 

    0x42706e30,// 259 PAY 256 

    0xaa3a1170,// 260 PAY 257 

    0x2b87ee19,// 261 PAY 258 

    0x1397e1e1,// 262 PAY 259 

    0x25f8c2c3,// 263 PAY 260 

    0x39efbce4,// 264 PAY 261 

    0x4a287a35,// 265 PAY 262 

    0xd22e61ec,// 266 PAY 263 

    0xb449ad9d,// 267 PAY 264 

    0xb764c542,// 268 PAY 265 

    0x3dc51f12,// 269 PAY 266 

    0xb31864a9,// 270 PAY 267 

    0x090c9da8,// 271 PAY 268 

    0x69070010,// 272 PAY 269 

    0x90116b8c,// 273 PAY 270 

    0x54df863a,// 274 PAY 271 

    0xa147441e,// 275 PAY 272 

    0x564b9e14,// 276 PAY 273 

    0x4012f6ad,// 277 PAY 274 

    0x3ff03110,// 278 PAY 275 

    0xb6df8afa,// 279 PAY 276 

    0xb6988cb0,// 280 PAY 277 

    0xf48a97aa,// 281 PAY 278 

    0x908c816b,// 282 PAY 279 

    0x31809010,// 283 PAY 280 

    0x27b3aef6,// 284 PAY 281 

    0xf8224840,// 285 PAY 282 

    0x8bfdd505,// 286 PAY 283 

    0xc1d5feb8,// 287 PAY 284 

    0x2dd6397d,// 288 PAY 285 

    0xd9b21b87,// 289 PAY 286 

    0xb6054050,// 290 PAY 287 

    0xf8752de8,// 291 PAY 288 

    0xf0cbad42,// 292 PAY 289 

    0x84027f3b,// 293 PAY 290 

    0x8f6e08d1,// 294 PAY 291 

    0xb3786410,// 295 PAY 292 

    0x43b7b11d,// 296 PAY 293 

    0xb2205b50,// 297 PAY 294 

    0x6db5f779,// 298 PAY 295 

    0x1c71d7ac,// 299 PAY 296 

    0x73a605ad,// 300 PAY 297 

    0x211e7326,// 301 PAY 298 

    0xe7bd45ba,// 302 PAY 299 

    0x519b2e70,// 303 PAY 300 

    0x81f36a3d,// 304 PAY 301 

    0x0813c465,// 305 PAY 302 

    0xff7bf94f,// 306 PAY 303 

/// HASH is  16 bytes 

    0xcf87073d,// 307 HSH   1 

    0x3a9742de,// 308 HSH   2 

    0x866c288a,// 309 HSH   3 

    0x15e21d49,// 310 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 132 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x26 

    0x01f12684 // 311 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt15_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 50 words. 

/// BDA size     is 194 (0xc2) 

/// BDA id       is 0xa24 

    0x00c20a24,// 3 BDA   1 

/// PAY Generic Data size   : 194 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x436db5cb,// 4 PAY   1 

    0x3cea67e6,// 5 PAY   2 

    0x0c4e3d93,// 6 PAY   3 

    0x96510e8e,// 7 PAY   4 

    0x46ff83a1,// 8 PAY   5 

    0xc9a5df06,// 9 PAY   6 

    0x2ae9f275,// 10 PAY   7 

    0x883de9f3,// 11 PAY   8 

    0xcfc11ba8,// 12 PAY   9 

    0xf031c5f8,// 13 PAY  10 

    0xf6ce2774,// 14 PAY  11 

    0x8d78b1cb,// 15 PAY  12 

    0x3b3befd1,// 16 PAY  13 

    0x6e93b9dd,// 17 PAY  14 

    0x327a5b4c,// 18 PAY  15 

    0xa43f3a97,// 19 PAY  16 

    0x1a1caa8f,// 20 PAY  17 

    0xa8682e9c,// 21 PAY  18 

    0x890552f8,// 22 PAY  19 

    0x73260d03,// 23 PAY  20 

    0x54f979a0,// 24 PAY  21 

    0x90b0dbaf,// 25 PAY  22 

    0xda810f0c,// 26 PAY  23 

    0x23511260,// 27 PAY  24 

    0x55a7424f,// 28 PAY  25 

    0x5e6c73b3,// 29 PAY  26 

    0x384cef42,// 30 PAY  27 

    0x2d63d83e,// 31 PAY  28 

    0x5f5212a3,// 32 PAY  29 

    0x7b306669,// 33 PAY  30 

    0x6ab0af8d,// 34 PAY  31 

    0xd456c3c0,// 35 PAY  32 

    0x2fad6279,// 36 PAY  33 

    0x7a374d22,// 37 PAY  34 

    0x06978533,// 38 PAY  35 

    0xcb3255bb,// 39 PAY  36 

    0xdca531f4,// 40 PAY  37 

    0xb3ff11d0,// 41 PAY  38 

    0xafa281e6,// 42 PAY  39 

    0x95b68e4e,// 43 PAY  40 

    0xa7322a7d,// 44 PAY  41 

    0x2fb90315,// 45 PAY  42 

    0x32492be3,// 46 PAY  43 

    0x8fb911bc,// 47 PAY  44 

    0x42e37ff2,// 48 PAY  45 

    0xa896ef3c,// 49 PAY  46 

    0x2eda34e0,// 50 PAY  47 

    0x80c57db2,// 51 PAY  48 

    0x73a60000,// 52 PAY  49 

/// STA is 1 words. 

/// STA num_pkts       : 92 

/// STA pkt_idx        : 100 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xde 

    0x0191de5c // 53 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt16_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 82 words. 

/// BDA size     is 324 (0x144) 

/// BDA id       is 0x8a3d 

    0x01448a3d,// 3 BDA   1 

/// PAY Generic Data size   : 324 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x4097a599,// 4 PAY   1 

    0x04da4b99,// 5 PAY   2 

    0x0ac20c76,// 6 PAY   3 

    0x4b0d03f0,// 7 PAY   4 

    0xbde167a0,// 8 PAY   5 

    0xd6ec2b0e,// 9 PAY   6 

    0x57c379a1,// 10 PAY   7 

    0x7a3bacbe,// 11 PAY   8 

    0x897eaa7a,// 12 PAY   9 

    0xdeaa1025,// 13 PAY  10 

    0x62f95478,// 14 PAY  11 

    0xdc6bcd9c,// 15 PAY  12 

    0xcd21adb1,// 16 PAY  13 

    0x936c93a5,// 17 PAY  14 

    0x65166f71,// 18 PAY  15 

    0x24ecd272,// 19 PAY  16 

    0xc4e09331,// 20 PAY  17 

    0x08d6880d,// 21 PAY  18 

    0x1dc4105c,// 22 PAY  19 

    0x9b277432,// 23 PAY  20 

    0xc2c30a8b,// 24 PAY  21 

    0x789ced9b,// 25 PAY  22 

    0x951446d2,// 26 PAY  23 

    0xa99560c5,// 27 PAY  24 

    0xc1b16564,// 28 PAY  25 

    0x696585fc,// 29 PAY  26 

    0x3a5a3dfe,// 30 PAY  27 

    0x46bd60cd,// 31 PAY  28 

    0x8c1a8a0c,// 32 PAY  29 

    0xfbe1a72d,// 33 PAY  30 

    0xa2416c85,// 34 PAY  31 

    0x142e932e,// 35 PAY  32 

    0xe9cb5ed8,// 36 PAY  33 

    0xa0b38cce,// 37 PAY  34 

    0x61beb0a7,// 38 PAY  35 

    0x0441b0f5,// 39 PAY  36 

    0x3654ff7b,// 40 PAY  37 

    0xee950e53,// 41 PAY  38 

    0x13f503b5,// 42 PAY  39 

    0x57dacba0,// 43 PAY  40 

    0x6c4a4c4e,// 44 PAY  41 

    0x0b35a841,// 45 PAY  42 

    0xbdde5084,// 46 PAY  43 

    0x242fab69,// 47 PAY  44 

    0x847d6db8,// 48 PAY  45 

    0xee0e7d04,// 49 PAY  46 

    0xa454d304,// 50 PAY  47 

    0x2aa08c50,// 51 PAY  48 

    0x4f773750,// 52 PAY  49 

    0x6a32fcc2,// 53 PAY  50 

    0xd9bdcdcd,// 54 PAY  51 

    0x1f3a29ed,// 55 PAY  52 

    0xdac9c854,// 56 PAY  53 

    0x18585741,// 57 PAY  54 

    0xb194b9c7,// 58 PAY  55 

    0x42bb6c25,// 59 PAY  56 

    0x53aff12e,// 60 PAY  57 

    0x292ae46f,// 61 PAY  58 

    0x42a0ff06,// 62 PAY  59 

    0xdf434399,// 63 PAY  60 

    0x4bdd1314,// 64 PAY  61 

    0x827ba9e4,// 65 PAY  62 

    0x9873a51d,// 66 PAY  63 

    0xbba3e8a7,// 67 PAY  64 

    0xb8a062be,// 68 PAY  65 

    0x025381da,// 69 PAY  66 

    0x819ec04b,// 70 PAY  67 

    0x9ec150f0,// 71 PAY  68 

    0x73b61b84,// 72 PAY  69 

    0x006a9117,// 73 PAY  70 

    0xf9a2eca8,// 74 PAY  71 

    0x89458043,// 75 PAY  72 

    0xaffa00f9,// 76 PAY  73 

    0xb6126c66,// 77 PAY  74 

    0x30187af8,// 78 PAY  75 

    0xd8cb6e5a,// 79 PAY  76 

    0xd39403e8,// 80 PAY  77 

    0x4b4b6912,// 81 PAY  78 

    0x1e84c385,// 82 PAY  79 

    0xf6ad0277,// 83 PAY  80 

    0xeeb4ddd5,// 84 PAY  81 

/// STA is 1 words. 

/// STA num_pkts       : 124 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc5 

    0x03dcc57c // 85 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt17_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 482 words. 

/// BDA size     is 1922 (0x782) 

/// BDA id       is 0xc3ba 

    0x0782c3ba,// 3 BDA   1 

/// PAY Generic Data size   : 1922 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x8bba353c,// 4 PAY   1 

    0x3e7dec84,// 5 PAY   2 

    0x010b0da9,// 6 PAY   3 

    0xc8088058,// 7 PAY   4 

    0x25a81c0b,// 8 PAY   5 

    0xbda826c4,// 9 PAY   6 

    0x4e7c21cc,// 10 PAY   7 

    0x73f3d61d,// 11 PAY   8 

    0xe739ae09,// 12 PAY   9 

    0x7791aabb,// 13 PAY  10 

    0xd9fc40dd,// 14 PAY  11 

    0x69a4dbc9,// 15 PAY  12 

    0x88fffb9e,// 16 PAY  13 

    0x6d8d5609,// 17 PAY  14 

    0xfd308f38,// 18 PAY  15 

    0x10b1911e,// 19 PAY  16 

    0xc8c294d6,// 20 PAY  17 

    0xc4bbb7e5,// 21 PAY  18 

    0xd967c849,// 22 PAY  19 

    0x88e750e4,// 23 PAY  20 

    0xadbd7f91,// 24 PAY  21 

    0x7efd05dd,// 25 PAY  22 

    0x866efb02,// 26 PAY  23 

    0x12391a15,// 27 PAY  24 

    0xf55eb464,// 28 PAY  25 

    0x317e0f29,// 29 PAY  26 

    0xf25d5037,// 30 PAY  27 

    0x71a833b6,// 31 PAY  28 

    0x1648e696,// 32 PAY  29 

    0xcb9949a1,// 33 PAY  30 

    0xe8f83439,// 34 PAY  31 

    0xa06cc068,// 35 PAY  32 

    0x3b0c095c,// 36 PAY  33 

    0x6cc70313,// 37 PAY  34 

    0xacab14c6,// 38 PAY  35 

    0xfd51b6bf,// 39 PAY  36 

    0x08754a4b,// 40 PAY  37 

    0xd4615f45,// 41 PAY  38 

    0x998ecf25,// 42 PAY  39 

    0xc8d8fd59,// 43 PAY  40 

    0xab0fb83b,// 44 PAY  41 

    0x4e8ac1c6,// 45 PAY  42 

    0x96fee4d9,// 46 PAY  43 

    0x7d8fc21e,// 47 PAY  44 

    0x22627f0e,// 48 PAY  45 

    0xcd90384b,// 49 PAY  46 

    0xb5b66c99,// 50 PAY  47 

    0x4c1061f8,// 51 PAY  48 

    0xb068a0ee,// 52 PAY  49 

    0x08fef841,// 53 PAY  50 

    0xfaf8a575,// 54 PAY  51 

    0xb813de9c,// 55 PAY  52 

    0xb3618e06,// 56 PAY  53 

    0xcb05bda7,// 57 PAY  54 

    0xf48e760e,// 58 PAY  55 

    0xe389c79a,// 59 PAY  56 

    0x8d53bbf1,// 60 PAY  57 

    0x94f471eb,// 61 PAY  58 

    0x9653551d,// 62 PAY  59 

    0x33dec2b6,// 63 PAY  60 

    0xd6c2dfef,// 64 PAY  61 

    0xd826ef6a,// 65 PAY  62 

    0x2249f93c,// 66 PAY  63 

    0x8cfdef87,// 67 PAY  64 

    0x896a9652,// 68 PAY  65 

    0x1116aa46,// 69 PAY  66 

    0x4020cbc9,// 70 PAY  67 

    0x9687cbb1,// 71 PAY  68 

    0x9958aabe,// 72 PAY  69 

    0xadad8167,// 73 PAY  70 

    0xd71e9aa8,// 74 PAY  71 

    0x93518a69,// 75 PAY  72 

    0xf494939c,// 76 PAY  73 

    0x0237e022,// 77 PAY  74 

    0xedd14f98,// 78 PAY  75 

    0x85c08de7,// 79 PAY  76 

    0xe4e59b7d,// 80 PAY  77 

    0xe3925297,// 81 PAY  78 

    0xaa0a91cd,// 82 PAY  79 

    0x9128a168,// 83 PAY  80 

    0xe6abc90f,// 84 PAY  81 

    0x86a6052c,// 85 PAY  82 

    0xb47342aa,// 86 PAY  83 

    0xe3b106b0,// 87 PAY  84 

    0x7671395b,// 88 PAY  85 

    0xe0d0d9c7,// 89 PAY  86 

    0x67edc5af,// 90 PAY  87 

    0x405d83cf,// 91 PAY  88 

    0x9d3942a0,// 92 PAY  89 

    0x5258282a,// 93 PAY  90 

    0x51732f05,// 94 PAY  91 

    0xe363d5d8,// 95 PAY  92 

    0xb826efa5,// 96 PAY  93 

    0xb3a95439,// 97 PAY  94 

    0x881fa2a4,// 98 PAY  95 

    0xa04d2df1,// 99 PAY  96 

    0x5a8dc412,// 100 PAY  97 

    0xaded396b,// 101 PAY  98 

    0x1834a98b,// 102 PAY  99 

    0x898391a9,// 103 PAY 100 

    0x0c6531de,// 104 PAY 101 

    0xef7dd1d8,// 105 PAY 102 

    0x387276fa,// 106 PAY 103 

    0x618f2c95,// 107 PAY 104 

    0xbb84c89a,// 108 PAY 105 

    0xa69f8a61,// 109 PAY 106 

    0x714e2fef,// 110 PAY 107 

    0xcd719bf0,// 111 PAY 108 

    0xf538704d,// 112 PAY 109 

    0x9448a395,// 113 PAY 110 

    0x88e06ce4,// 114 PAY 111 

    0x1ade376a,// 115 PAY 112 

    0x5df95e5d,// 116 PAY 113 

    0x94bd1ce2,// 117 PAY 114 

    0x7201d5a3,// 118 PAY 115 

    0x97f496fc,// 119 PAY 116 

    0x5b3d9525,// 120 PAY 117 

    0xddf2f8ac,// 121 PAY 118 

    0x8976c38e,// 122 PAY 119 

    0xee16a877,// 123 PAY 120 

    0xd95d774f,// 124 PAY 121 

    0x793ebd99,// 125 PAY 122 

    0x0600857e,// 126 PAY 123 

    0xe90c9640,// 127 PAY 124 

    0x0d723895,// 128 PAY 125 

    0x160f5cee,// 129 PAY 126 

    0xd0a3599d,// 130 PAY 127 

    0x7351462a,// 131 PAY 128 

    0x76b7e299,// 132 PAY 129 

    0x13c5dd27,// 133 PAY 130 

    0x8eb11bf7,// 134 PAY 131 

    0xc9ce8933,// 135 PAY 132 

    0x2337fb2b,// 136 PAY 133 

    0x62896df6,// 137 PAY 134 

    0x7d91e9ad,// 138 PAY 135 

    0x4dadf760,// 139 PAY 136 

    0xc0724ac2,// 140 PAY 137 

    0x2d102bb6,// 141 PAY 138 

    0xdb46a07f,// 142 PAY 139 

    0x1eaa06f1,// 143 PAY 140 

    0x91692df1,// 144 PAY 141 

    0xf7eab869,// 145 PAY 142 

    0xc3b20574,// 146 PAY 143 

    0xe19bf4c4,// 147 PAY 144 

    0xc15da5c3,// 148 PAY 145 

    0x24b7f79d,// 149 PAY 146 

    0xf1d5ab72,// 150 PAY 147 

    0x016d8988,// 151 PAY 148 

    0x6373c78b,// 152 PAY 149 

    0xe41ad8a2,// 153 PAY 150 

    0x50c570a1,// 154 PAY 151 

    0x9c557c3e,// 155 PAY 152 

    0x9082c806,// 156 PAY 153 

    0xbf25879f,// 157 PAY 154 

    0x5bd1cbb8,// 158 PAY 155 

    0xcbf8bee0,// 159 PAY 156 

    0xb3e2b971,// 160 PAY 157 

    0x10d38568,// 161 PAY 158 

    0x3dc947ba,// 162 PAY 159 

    0x536fc5d0,// 163 PAY 160 

    0x1ce7201e,// 164 PAY 161 

    0xe1cf9e0d,// 165 PAY 162 

    0x4a30d65c,// 166 PAY 163 

    0x4d199c2f,// 167 PAY 164 

    0x4c623984,// 168 PAY 165 

    0xaba96ccc,// 169 PAY 166 

    0xd8c6ac42,// 170 PAY 167 

    0x6f825c19,// 171 PAY 168 

    0xa0ef5588,// 172 PAY 169 

    0x1f324b30,// 173 PAY 170 

    0x3e48bae0,// 174 PAY 171 

    0x524fbfb5,// 175 PAY 172 

    0x4870fe17,// 176 PAY 173 

    0x039ebc44,// 177 PAY 174 

    0x7d9908db,// 178 PAY 175 

    0x73d0b925,// 179 PAY 176 

    0x80236510,// 180 PAY 177 

    0xf808c71a,// 181 PAY 178 

    0x4c02e872,// 182 PAY 179 

    0x3eb71bb0,// 183 PAY 180 

    0x8d970c7a,// 184 PAY 181 

    0x60411262,// 185 PAY 182 

    0x009d0204,// 186 PAY 183 

    0xb019451f,// 187 PAY 184 

    0x6d51377e,// 188 PAY 185 

    0xda649e2f,// 189 PAY 186 

    0x88d2e594,// 190 PAY 187 

    0x53eefeb0,// 191 PAY 188 

    0xe7ef110f,// 192 PAY 189 

    0x8b09a8b7,// 193 PAY 190 

    0xbb1611f6,// 194 PAY 191 

    0x3352a938,// 195 PAY 192 

    0x3954c5ef,// 196 PAY 193 

    0x4ffeb2ae,// 197 PAY 194 

    0x982779ed,// 198 PAY 195 

    0x176b023a,// 199 PAY 196 

    0x5f24552b,// 200 PAY 197 

    0x9543135b,// 201 PAY 198 

    0x47a29bf5,// 202 PAY 199 

    0x629ab7b5,// 203 PAY 200 

    0x05960861,// 204 PAY 201 

    0xce5fd20f,// 205 PAY 202 

    0x2d67bc6c,// 206 PAY 203 

    0x998308e9,// 207 PAY 204 

    0x4ae3ec89,// 208 PAY 205 

    0x7e2e0e42,// 209 PAY 206 

    0x74189848,// 210 PAY 207 

    0xcb6e2041,// 211 PAY 208 

    0xb816ae5e,// 212 PAY 209 

    0x9fad92d0,// 213 PAY 210 

    0x5c92bd67,// 214 PAY 211 

    0x1b192721,// 215 PAY 212 

    0xeb93652f,// 216 PAY 213 

    0xc104bcb7,// 217 PAY 214 

    0xe9c9975e,// 218 PAY 215 

    0x2481f8a0,// 219 PAY 216 

    0xb110d244,// 220 PAY 217 

    0x9843be5e,// 221 PAY 218 

    0x405eb80d,// 222 PAY 219 

    0x5385c5b9,// 223 PAY 220 

    0xbd466b20,// 224 PAY 221 

    0x510af210,// 225 PAY 222 

    0x26467813,// 226 PAY 223 

    0x00735a92,// 227 PAY 224 

    0x905b7edd,// 228 PAY 225 

    0x514c0e54,// 229 PAY 226 

    0x1810e595,// 230 PAY 227 

    0x1059845a,// 231 PAY 228 

    0x3b8f2324,// 232 PAY 229 

    0xae6d7ab3,// 233 PAY 230 

    0xce58e1ce,// 234 PAY 231 

    0xf568761e,// 235 PAY 232 

    0x672b4eec,// 236 PAY 233 

    0x92426fcc,// 237 PAY 234 

    0x4cbee6b7,// 238 PAY 235 

    0x8a91aa14,// 239 PAY 236 

    0x8805bd1c,// 240 PAY 237 

    0x21527953,// 241 PAY 238 

    0x0c2edbd8,// 242 PAY 239 

    0x40d41120,// 243 PAY 240 

    0x95b186ba,// 244 PAY 241 

    0x7a8de434,// 245 PAY 242 

    0xd2ac05de,// 246 PAY 243 

    0x5efebf5d,// 247 PAY 244 

    0x29e49c37,// 248 PAY 245 

    0xf80db2be,// 249 PAY 246 

    0xb0f02b15,// 250 PAY 247 

    0x0e483d02,// 251 PAY 248 

    0x2be58b6d,// 252 PAY 249 

    0xa05165ad,// 253 PAY 250 

    0xdac5bd2f,// 254 PAY 251 

    0x3fc95823,// 255 PAY 252 

    0x4240fbf0,// 256 PAY 253 

    0x75428b2a,// 257 PAY 254 

    0xd535f1e3,// 258 PAY 255 

    0x7216c36f,// 259 PAY 256 

    0xf7f8b7b1,// 260 PAY 257 

    0x7df709b3,// 261 PAY 258 

    0x20115765,// 262 PAY 259 

    0xcfcb6f49,// 263 PAY 260 

    0x5cf1216e,// 264 PAY 261 

    0xd2758d44,// 265 PAY 262 

    0xe51ed5d1,// 266 PAY 263 

    0x6cd98e4d,// 267 PAY 264 

    0x501dd550,// 268 PAY 265 

    0x827ae6b9,// 269 PAY 266 

    0xefaa9668,// 270 PAY 267 

    0x5ecafdf3,// 271 PAY 268 

    0xa3f44401,// 272 PAY 269 

    0x056e5e45,// 273 PAY 270 

    0x4022a876,// 274 PAY 271 

    0xe540b7ed,// 275 PAY 272 

    0x19ee1847,// 276 PAY 273 

    0x035825ca,// 277 PAY 274 

    0xeb990693,// 278 PAY 275 

    0x4ef4b8fa,// 279 PAY 276 

    0xdebb2777,// 280 PAY 277 

    0x9292866d,// 281 PAY 278 

    0x989681c0,// 282 PAY 279 

    0x3aa509da,// 283 PAY 280 

    0x4a78b342,// 284 PAY 281 

    0x7d9cc6e5,// 285 PAY 282 

    0xa3c27430,// 286 PAY 283 

    0x61ac4ea4,// 287 PAY 284 

    0x92cb7651,// 288 PAY 285 

    0x5bf12b80,// 289 PAY 286 

    0x22d22570,// 290 PAY 287 

    0x34b2422a,// 291 PAY 288 

    0xc67973c3,// 292 PAY 289 

    0x19ab06b0,// 293 PAY 290 

    0x56c5bfd1,// 294 PAY 291 

    0xf2174b83,// 295 PAY 292 

    0xca9b3f8d,// 296 PAY 293 

    0x53ae881a,// 297 PAY 294 

    0x5d53d10c,// 298 PAY 295 

    0x08ce71ef,// 299 PAY 296 

    0x1af107f9,// 300 PAY 297 

    0xdbe8af62,// 301 PAY 298 

    0x24dedc23,// 302 PAY 299 

    0xa71fb770,// 303 PAY 300 

    0x7bc2331d,// 304 PAY 301 

    0x27ec7195,// 305 PAY 302 

    0x9409b735,// 306 PAY 303 

    0xaabc677e,// 307 PAY 304 

    0x2e935f5d,// 308 PAY 305 

    0x0d6bbe7d,// 309 PAY 306 

    0x157c7963,// 310 PAY 307 

    0x85998556,// 311 PAY 308 

    0xdfcbd999,// 312 PAY 309 

    0x826210d0,// 313 PAY 310 

    0xa3d4157a,// 314 PAY 311 

    0x6342c8b8,// 315 PAY 312 

    0x1c36bb31,// 316 PAY 313 

    0xa9c25808,// 317 PAY 314 

    0xd80a5297,// 318 PAY 315 

    0x39a07a24,// 319 PAY 316 

    0x475204d4,// 320 PAY 317 

    0x80a55511,// 321 PAY 318 

    0x3da530ed,// 322 PAY 319 

    0x947e23b7,// 323 PAY 320 

    0x9a99620c,// 324 PAY 321 

    0x5a032b5b,// 325 PAY 322 

    0xaab447c3,// 326 PAY 323 

    0x6590db73,// 327 PAY 324 

    0x113177b1,// 328 PAY 325 

    0x0416fbbf,// 329 PAY 326 

    0x9fc6d080,// 330 PAY 327 

    0x8cf3aa55,// 331 PAY 328 

    0x50015a02,// 332 PAY 329 

    0x6e279afd,// 333 PAY 330 

    0x78296460,// 334 PAY 331 

    0x0b206262,// 335 PAY 332 

    0x029f3c06,// 336 PAY 333 

    0x06e129eb,// 337 PAY 334 

    0x2d207292,// 338 PAY 335 

    0xe06a89ce,// 339 PAY 336 

    0x8e3b857d,// 340 PAY 337 

    0xf85e0d58,// 341 PAY 338 

    0x669a7df2,// 342 PAY 339 

    0xbfcef3fa,// 343 PAY 340 

    0xecccf1a1,// 344 PAY 341 

    0xea39f248,// 345 PAY 342 

    0x2b749e2b,// 346 PAY 343 

    0xac2630ed,// 347 PAY 344 

    0xfcc63bba,// 348 PAY 345 

    0xcbb4bef5,// 349 PAY 346 

    0xbe755f54,// 350 PAY 347 

    0x95d3c9e7,// 351 PAY 348 

    0xb6e5f174,// 352 PAY 349 

    0x10ff2a66,// 353 PAY 350 

    0x5850868a,// 354 PAY 351 

    0x2201fda4,// 355 PAY 352 

    0x1e4e08de,// 356 PAY 353 

    0x0e036c1b,// 357 PAY 354 

    0xaf41dd44,// 358 PAY 355 

    0xd23c67a8,// 359 PAY 356 

    0xc9502521,// 360 PAY 357 

    0xc79168bf,// 361 PAY 358 

    0x1c3cf261,// 362 PAY 359 

    0x441361b4,// 363 PAY 360 

    0x927c80dc,// 364 PAY 361 

    0x3580110f,// 365 PAY 362 

    0x868331da,// 366 PAY 363 

    0x77ee9ab2,// 367 PAY 364 

    0x19e3d98e,// 368 PAY 365 

    0xa79b13e3,// 369 PAY 366 

    0x66d5c596,// 370 PAY 367 

    0x3aaffcb3,// 371 PAY 368 

    0x1b4f7274,// 372 PAY 369 

    0xb7a0dbfe,// 373 PAY 370 

    0x69f30d07,// 374 PAY 371 

    0x93ce4a92,// 375 PAY 372 

    0xf5def4ba,// 376 PAY 373 

    0x4827109e,// 377 PAY 374 

    0xad961191,// 378 PAY 375 

    0xe4c71d0b,// 379 PAY 376 

    0x70a1ef42,// 380 PAY 377 

    0x06dc5fba,// 381 PAY 378 

    0x0ee1ecb8,// 382 PAY 379 

    0xa7ae1663,// 383 PAY 380 

    0xb533959d,// 384 PAY 381 

    0x47a0020f,// 385 PAY 382 

    0xce43ccfa,// 386 PAY 383 

    0xd2433472,// 387 PAY 384 

    0xf9ae1193,// 388 PAY 385 

    0xbcf45d65,// 389 PAY 386 

    0x25f0254d,// 390 PAY 387 

    0x3a1b1bdb,// 391 PAY 388 

    0x8fbb7dcd,// 392 PAY 389 

    0xe2cb8b1c,// 393 PAY 390 

    0x72cada00,// 394 PAY 391 

    0xe207ef5c,// 395 PAY 392 

    0xf690f819,// 396 PAY 393 

    0xb5d4217f,// 397 PAY 394 

    0x8f2749d2,// 398 PAY 395 

    0xa6ffc83b,// 399 PAY 396 

    0x2f9235bc,// 400 PAY 397 

    0xb58f5e11,// 401 PAY 398 

    0x7217f52b,// 402 PAY 399 

    0xb811ed76,// 403 PAY 400 

    0xb3f4f713,// 404 PAY 401 

    0x8d47ec36,// 405 PAY 402 

    0x8332cfb6,// 406 PAY 403 

    0xcd391cf2,// 407 PAY 404 

    0xd476971a,// 408 PAY 405 

    0x681e209e,// 409 PAY 406 

    0x7e7ce548,// 410 PAY 407 

    0x16fb2b7c,// 411 PAY 408 

    0x8ca37551,// 412 PAY 409 

    0x5b15d678,// 413 PAY 410 

    0x8929c7c3,// 414 PAY 411 

    0xc436f324,// 415 PAY 412 

    0x1fb195a6,// 416 PAY 413 

    0x9561e13b,// 417 PAY 414 

    0x129a5d3d,// 418 PAY 415 

    0x41897ec2,// 419 PAY 416 

    0x98d32395,// 420 PAY 417 

    0xf0521c9f,// 421 PAY 418 

    0x1ba54330,// 422 PAY 419 

    0x091cb417,// 423 PAY 420 

    0xf21294cf,// 424 PAY 421 

    0xa75efe93,// 425 PAY 422 

    0x1f1db6a9,// 426 PAY 423 

    0x844e346c,// 427 PAY 424 

    0x6f11c2d5,// 428 PAY 425 

    0x0b8f97b1,// 429 PAY 426 

    0xacaa12a8,// 430 PAY 427 

    0x37d22d51,// 431 PAY 428 

    0x48fa9b7c,// 432 PAY 429 

    0x69d304c3,// 433 PAY 430 

    0x8b927e26,// 434 PAY 431 

    0x74ac7dec,// 435 PAY 432 

    0xebd012b8,// 436 PAY 433 

    0x13ffe71a,// 437 PAY 434 

    0xacfddace,// 438 PAY 435 

    0x2528fe4e,// 439 PAY 436 

    0xc64828ef,// 440 PAY 437 

    0xbd93894d,// 441 PAY 438 

    0x47158067,// 442 PAY 439 

    0x0c8a3d35,// 443 PAY 440 

    0x9da1505c,// 444 PAY 441 

    0x2303e8f1,// 445 PAY 442 

    0x5ec1f636,// 446 PAY 443 

    0x16db887c,// 447 PAY 444 

    0xa7c6ea0b,// 448 PAY 445 

    0x99e84dfc,// 449 PAY 446 

    0x9e10daa0,// 450 PAY 447 

    0x6570f990,// 451 PAY 448 

    0xb1f97f28,// 452 PAY 449 

    0xa01f9d16,// 453 PAY 450 

    0x42a5742b,// 454 PAY 451 

    0xdd03139d,// 455 PAY 452 

    0x793d89f7,// 456 PAY 453 

    0x47c90fcf,// 457 PAY 454 

    0x5dc7367d,// 458 PAY 455 

    0x465f17f0,// 459 PAY 456 

    0x044bc444,// 460 PAY 457 

    0xf68cb8c8,// 461 PAY 458 

    0xc32cce52,// 462 PAY 459 

    0xa4815027,// 463 PAY 460 

    0x3edc1fcb,// 464 PAY 461 

    0xad8f5e8f,// 465 PAY 462 

    0xf0a63567,// 466 PAY 463 

    0xba288c68,// 467 PAY 464 

    0x88e44a6a,// 468 PAY 465 

    0xd06eed2c,// 469 PAY 466 

    0x566d7683,// 470 PAY 467 

    0x4dec8985,// 471 PAY 468 

    0xf9c9afe5,// 472 PAY 469 

    0xba334357,// 473 PAY 470 

    0xb0a0f709,// 474 PAY 471 

    0xda7b6898,// 475 PAY 472 

    0xe65ccdb4,// 476 PAY 473 

    0xaf0657fc,// 477 PAY 474 

    0xda92ebf2,// 478 PAY 475 

    0x19725407,// 479 PAY 476 

    0xcb0612df,// 480 PAY 477 

    0x2c8ba4fe,// 481 PAY 478 

    0xc5b510d2,// 482 PAY 479 

    0xc3a79d56,// 483 PAY 480 

    0x44ef0000,// 484 PAY 481 

/// HASH is  20 bytes 

    0xecccf1a1,// 485 HSH   1 

    0xea39f248,// 486 HSH   2 

    0x2b749e2b,// 487 HSH   3 

    0xac2630ed,// 488 HSH   4 

    0xfcc63bba,// 489 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 137 

/// STA pkt_idx        : 109 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc0 

    0x01b5c089 // 490 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt18_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 303 words. 

/// BDA size     is 1207 (0x4b7) 

/// BDA id       is 0xdbea 

    0x04b7dbea,// 3 BDA   1 

/// PAY Generic Data size   : 1207 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xdb09bb79,// 4 PAY   1 

    0x2b7f0b58,// 5 PAY   2 

    0xec9e9b07,// 6 PAY   3 

    0xe58ae5de,// 7 PAY   4 

    0xadfd22dc,// 8 PAY   5 

    0xe73b86f3,// 9 PAY   6 

    0x8b405f69,// 10 PAY   7 

    0x6a32ee9d,// 11 PAY   8 

    0xa763704d,// 12 PAY   9 

    0x65bcbbe7,// 13 PAY  10 

    0x7cde02a7,// 14 PAY  11 

    0x555f8ff9,// 15 PAY  12 

    0xfae6c43a,// 16 PAY  13 

    0xbad1ada1,// 17 PAY  14 

    0xa306655e,// 18 PAY  15 

    0x3c1767c3,// 19 PAY  16 

    0xcbe5643c,// 20 PAY  17 

    0xb4432836,// 21 PAY  18 

    0xbf0fde2d,// 22 PAY  19 

    0xd6b67e81,// 23 PAY  20 

    0xa38aef23,// 24 PAY  21 

    0x1bc89327,// 25 PAY  22 

    0xcdba0158,// 26 PAY  23 

    0x8a6e224f,// 27 PAY  24 

    0x91c11ca3,// 28 PAY  25 

    0x0d2fc465,// 29 PAY  26 

    0xd55abae0,// 30 PAY  27 

    0xf239849d,// 31 PAY  28 

    0x138bf6c9,// 32 PAY  29 

    0xf872b3eb,// 33 PAY  30 

    0xfaf92b6c,// 34 PAY  31 

    0x61e483fb,// 35 PAY  32 

    0x21b7aad2,// 36 PAY  33 

    0xf5f7ce0c,// 37 PAY  34 

    0xdab48014,// 38 PAY  35 

    0x880a4016,// 39 PAY  36 

    0x2e5f8979,// 40 PAY  37 

    0x8ecfb2cb,// 41 PAY  38 

    0x18754ff7,// 42 PAY  39 

    0x98522089,// 43 PAY  40 

    0xf769e9c0,// 44 PAY  41 

    0x623efbbd,// 45 PAY  42 

    0xe03d41cf,// 46 PAY  43 

    0x3a3dc714,// 47 PAY  44 

    0xf9569fa2,// 48 PAY  45 

    0x7b4db31e,// 49 PAY  46 

    0xad972af9,// 50 PAY  47 

    0xd1afc5d8,// 51 PAY  48 

    0xf8aed718,// 52 PAY  49 

    0x28cacd53,// 53 PAY  50 

    0x329e7dfe,// 54 PAY  51 

    0x2f535442,// 55 PAY  52 

    0x0cf69ec5,// 56 PAY  53 

    0x52004cf4,// 57 PAY  54 

    0x7ced60fc,// 58 PAY  55 

    0x9e4d6f1c,// 59 PAY  56 

    0x90d564a0,// 60 PAY  57 

    0x1c1297c7,// 61 PAY  58 

    0xff3af8ba,// 62 PAY  59 

    0xc3ecf2bb,// 63 PAY  60 

    0x37d29bd3,// 64 PAY  61 

    0xae7bf82b,// 65 PAY  62 

    0xb1c68acb,// 66 PAY  63 

    0x2ab7f9fb,// 67 PAY  64 

    0x2a02c1db,// 68 PAY  65 

    0xc3c1a17a,// 69 PAY  66 

    0xaa02cf69,// 70 PAY  67 

    0xc0d0cbf6,// 71 PAY  68 

    0x5108a395,// 72 PAY  69 

    0x164fa5ef,// 73 PAY  70 

    0x399a0548,// 74 PAY  71 

    0xfcd8ccb6,// 75 PAY  72 

    0x6e8f9430,// 76 PAY  73 

    0x434aa55e,// 77 PAY  74 

    0x9e4adc92,// 78 PAY  75 

    0xfeb4c01a,// 79 PAY  76 

    0x9004a6f9,// 80 PAY  77 

    0xf1d32517,// 81 PAY  78 

    0xa22db90a,// 82 PAY  79 

    0x7386a2c2,// 83 PAY  80 

    0x540209f6,// 84 PAY  81 

    0xca307a0e,// 85 PAY  82 

    0x90a0e602,// 86 PAY  83 

    0x7b61aa98,// 87 PAY  84 

    0xf6c96265,// 88 PAY  85 

    0x226f7a3b,// 89 PAY  86 

    0xb39a0c5b,// 90 PAY  87 

    0x7840392c,// 91 PAY  88 

    0xe534e546,// 92 PAY  89 

    0xad479c54,// 93 PAY  90 

    0x75543414,// 94 PAY  91 

    0x255b50eb,// 95 PAY  92 

    0x6eef5235,// 96 PAY  93 

    0x0008ab4d,// 97 PAY  94 

    0x18142cd6,// 98 PAY  95 

    0x7286198e,// 99 PAY  96 

    0x619ee447,// 100 PAY  97 

    0xa0b7d70b,// 101 PAY  98 

    0x8573b2e6,// 102 PAY  99 

    0xae125311,// 103 PAY 100 

    0xd38b2887,// 104 PAY 101 

    0x0e03f605,// 105 PAY 102 

    0x29f0e3b7,// 106 PAY 103 

    0xb13ece3c,// 107 PAY 104 

    0x66e4bee1,// 108 PAY 105 

    0x0b142e46,// 109 PAY 106 

    0xd837529c,// 110 PAY 107 

    0x4f050dd6,// 111 PAY 108 

    0xe31735e3,// 112 PAY 109 

    0xe210eb5f,// 113 PAY 110 

    0x63eb5e48,// 114 PAY 111 

    0x32241a93,// 115 PAY 112 

    0x91fb269a,// 116 PAY 113 

    0xb6f13b3c,// 117 PAY 114 

    0xa4fc483e,// 118 PAY 115 

    0x17bc71b6,// 119 PAY 116 

    0xaae1460a,// 120 PAY 117 

    0x7eaa1d97,// 121 PAY 118 

    0x9273ec58,// 122 PAY 119 

    0xac19a89c,// 123 PAY 120 

    0x0e6b4e1e,// 124 PAY 121 

    0x5039d399,// 125 PAY 122 

    0x73e208f3,// 126 PAY 123 

    0x12968318,// 127 PAY 124 

    0x8c6f88de,// 128 PAY 125 

    0xd0d9018e,// 129 PAY 126 

    0xe9e5bbd3,// 130 PAY 127 

    0xd0840aaf,// 131 PAY 128 

    0x9c4b00e6,// 132 PAY 129 

    0x2ea681a5,// 133 PAY 130 

    0x424d69bd,// 134 PAY 131 

    0x88e3e303,// 135 PAY 132 

    0x4e7a9b06,// 136 PAY 133 

    0xf517bb2a,// 137 PAY 134 

    0x0920a8b5,// 138 PAY 135 

    0xeb6b0353,// 139 PAY 136 

    0xb2b36be1,// 140 PAY 137 

    0x1c35cd81,// 141 PAY 138 

    0xab94274c,// 142 PAY 139 

    0xfbe5f14e,// 143 PAY 140 

    0xf5bbbcba,// 144 PAY 141 

    0x6dbd64bc,// 145 PAY 142 

    0xcb1e2e31,// 146 PAY 143 

    0x38a03466,// 147 PAY 144 

    0x44d2d089,// 148 PAY 145 

    0x65447379,// 149 PAY 146 

    0xcbd30d15,// 150 PAY 147 

    0xe32c8cff,// 151 PAY 148 

    0x8f48bf6d,// 152 PAY 149 

    0x73538c41,// 153 PAY 150 

    0x671a2ea4,// 154 PAY 151 

    0xd7262c29,// 155 PAY 152 

    0x9c361ccf,// 156 PAY 153 

    0x8ee44ae1,// 157 PAY 154 

    0xf0718b4c,// 158 PAY 155 

    0x0fbb15b0,// 159 PAY 156 

    0xec5e2891,// 160 PAY 157 

    0xee192f43,// 161 PAY 158 

    0x17fe4e3a,// 162 PAY 159 

    0xf24aeac0,// 163 PAY 160 

    0x218034b8,// 164 PAY 161 

    0x9e34980b,// 165 PAY 162 

    0x59cb20a4,// 166 PAY 163 

    0x7306c2a3,// 167 PAY 164 

    0x4d847189,// 168 PAY 165 

    0x85c3179a,// 169 PAY 166 

    0xde252273,// 170 PAY 167 

    0xd90b7d85,// 171 PAY 168 

    0x62b70689,// 172 PAY 169 

    0x1d935dd9,// 173 PAY 170 

    0x0169edf3,// 174 PAY 171 

    0xdc8f77c0,// 175 PAY 172 

    0x0eb1dfd6,// 176 PAY 173 

    0x1677694a,// 177 PAY 174 

    0x80f05569,// 178 PAY 175 

    0xf4a0aa21,// 179 PAY 176 

    0xdafb33c0,// 180 PAY 177 

    0xe864929e,// 181 PAY 178 

    0x172f9a7a,// 182 PAY 179 

    0x860d77ba,// 183 PAY 180 

    0x15523e5a,// 184 PAY 181 

    0xb634131e,// 185 PAY 182 

    0x4ad7a067,// 186 PAY 183 

    0x6884d842,// 187 PAY 184 

    0xa661a6d4,// 188 PAY 185 

    0x24197fe6,// 189 PAY 186 

    0x5e67a5d2,// 190 PAY 187 

    0x935102fb,// 191 PAY 188 

    0x872641c8,// 192 PAY 189 

    0xc2359485,// 193 PAY 190 

    0x1daa1f75,// 194 PAY 191 

    0x808b8d6a,// 195 PAY 192 

    0x20330709,// 196 PAY 193 

    0xe27ec664,// 197 PAY 194 

    0x44aec5c0,// 198 PAY 195 

    0x8ceb94e2,// 199 PAY 196 

    0x120c3d4c,// 200 PAY 197 

    0x5c1b62a0,// 201 PAY 198 

    0x9761231e,// 202 PAY 199 

    0x36d2792b,// 203 PAY 200 

    0x1329269a,// 204 PAY 201 

    0x5a958ec9,// 205 PAY 202 

    0x94c46679,// 206 PAY 203 

    0x9098226a,// 207 PAY 204 

    0x60e9ed4c,// 208 PAY 205 

    0x491282db,// 209 PAY 206 

    0x3f9e9954,// 210 PAY 207 

    0xedc82c4e,// 211 PAY 208 

    0xbaac16cc,// 212 PAY 209 

    0x85d225d4,// 213 PAY 210 

    0x9ba18dd5,// 214 PAY 211 

    0x00a44a1f,// 215 PAY 212 

    0xc7b7622f,// 216 PAY 213 

    0xe4664418,// 217 PAY 214 

    0x8b817dc9,// 218 PAY 215 

    0x66db06c6,// 219 PAY 216 

    0x00e5d2a3,// 220 PAY 217 

    0x2dc3e9ae,// 221 PAY 218 

    0xea0f19b1,// 222 PAY 219 

    0xc4e7f974,// 223 PAY 220 

    0xa67a4644,// 224 PAY 221 

    0x664bd416,// 225 PAY 222 

    0x0a328de8,// 226 PAY 223 

    0x52d85121,// 227 PAY 224 

    0xca2df507,// 228 PAY 225 

    0x3992adab,// 229 PAY 226 

    0xf4542260,// 230 PAY 227 

    0x272d7a38,// 231 PAY 228 

    0xc680dcf5,// 232 PAY 229 

    0x0bbb8714,// 233 PAY 230 

    0xde4e2da7,// 234 PAY 231 

    0x545acf03,// 235 PAY 232 

    0xb5678a06,// 236 PAY 233 

    0x49082682,// 237 PAY 234 

    0x7270164c,// 238 PAY 235 

    0x02981a5e,// 239 PAY 236 

    0x5b594f66,// 240 PAY 237 

    0xf5d7ea51,// 241 PAY 238 

    0x65a38f4f,// 242 PAY 239 

    0x97426613,// 243 PAY 240 

    0x36428f2c,// 244 PAY 241 

    0xd64f23b7,// 245 PAY 242 

    0xdd30084e,// 246 PAY 243 

    0x7add85c8,// 247 PAY 244 

    0x6afb6e7e,// 248 PAY 245 

    0xc8bb52c2,// 249 PAY 246 

    0xe43ef0c3,// 250 PAY 247 

    0x38d83f03,// 251 PAY 248 

    0x52229996,// 252 PAY 249 

    0xc470b571,// 253 PAY 250 

    0xa4dcd2cf,// 254 PAY 251 

    0x0ed51a59,// 255 PAY 252 

    0x01bbf8bc,// 256 PAY 253 

    0x507f0cd6,// 257 PAY 254 

    0xbb5e85a3,// 258 PAY 255 

    0x5b032dd1,// 259 PAY 256 

    0xfd259909,// 260 PAY 257 

    0x61a0b487,// 261 PAY 258 

    0x81a3e888,// 262 PAY 259 

    0x5ef8289d,// 263 PAY 260 

    0xec9650b5,// 264 PAY 261 

    0x38e7a72f,// 265 PAY 262 

    0xd0fadb15,// 266 PAY 263 

    0x2ebe694d,// 267 PAY 264 

    0x871d188d,// 268 PAY 265 

    0xd99a9914,// 269 PAY 266 

    0xe1117ef4,// 270 PAY 267 

    0x96233b9c,// 271 PAY 268 

    0x3cae317e,// 272 PAY 269 

    0xef3dc9c9,// 273 PAY 270 

    0x2ba0d83c,// 274 PAY 271 

    0x48e908c7,// 275 PAY 272 

    0xc34d8842,// 276 PAY 273 

    0x5eeba3a0,// 277 PAY 274 

    0x772a4e54,// 278 PAY 275 

    0x925fbf28,// 279 PAY 276 

    0xba544482,// 280 PAY 277 

    0x2409e174,// 281 PAY 278 

    0x55442991,// 282 PAY 279 

    0x8919575c,// 283 PAY 280 

    0xa7a8bc1d,// 284 PAY 281 

    0xd443f896,// 285 PAY 282 

    0xc51073ee,// 286 PAY 283 

    0x4c78bb2f,// 287 PAY 284 

    0x3cbebc2e,// 288 PAY 285 

    0x5acc046a,// 289 PAY 286 

    0xe178e63a,// 290 PAY 287 

    0x3df664e1,// 291 PAY 288 

    0x3d0c59d8,// 292 PAY 289 

    0xb2018e0c,// 293 PAY 290 

    0xfde799db,// 294 PAY 291 

    0xbe7d217a,// 295 PAY 292 

    0x203619da,// 296 PAY 293 

    0x86d627eb,// 297 PAY 294 

    0x9efd8101,// 298 PAY 295 

    0x3a66a1e4,// 299 PAY 296 

    0x6499efac,// 300 PAY 297 

    0x222d569d,// 301 PAY 298 

    0xf40cbce1,// 302 PAY 299 

    0x86b2b863,// 303 PAY 300 

    0xc0e84a59,// 304 PAY 301 

    0x12af9d00,// 305 PAY 302 

/// HASH is  4 bytes 

    0xea0f19b1,// 306 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 212 

/// STA pkt_idx        : 86 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe6 

    0x0158e6d4 // 307 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt19_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 139 words. 

/// BDA size     is 551 (0x227) 

/// BDA id       is 0x426c 

    0x0227426c,// 3 BDA   1 

/// PAY Generic Data size   : 551 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xffc26e1a,// 4 PAY   1 

    0x0d426658,// 5 PAY   2 

    0xa17a2be6,// 6 PAY   3 

    0x84d98c9a,// 7 PAY   4 

    0xa7b72b50,// 8 PAY   5 

    0x25fa4781,// 9 PAY   6 

    0xb09dc61a,// 10 PAY   7 

    0xd82d7fc9,// 11 PAY   8 

    0xfce28fe1,// 12 PAY   9 

    0x5ef6f9a7,// 13 PAY  10 

    0x0e5f1dc8,// 14 PAY  11 

    0xe566116c,// 15 PAY  12 

    0x35ba864d,// 16 PAY  13 

    0x4265553d,// 17 PAY  14 

    0xee1173d0,// 18 PAY  15 

    0x7e44b9da,// 19 PAY  16 

    0xe3672f72,// 20 PAY  17 

    0xa02c399f,// 21 PAY  18 

    0xd95b4ebd,// 22 PAY  19 

    0x203c754b,// 23 PAY  20 

    0x79e5f3d2,// 24 PAY  21 

    0x7e93a981,// 25 PAY  22 

    0x832c4be2,// 26 PAY  23 

    0xba0e4f38,// 27 PAY  24 

    0x0f6eaaca,// 28 PAY  25 

    0xdfdec57d,// 29 PAY  26 

    0x6aecfd2f,// 30 PAY  27 

    0x7e4a4712,// 31 PAY  28 

    0xd823f988,// 32 PAY  29 

    0x0b6f3c03,// 33 PAY  30 

    0x13aaabe1,// 34 PAY  31 

    0xb49c9a67,// 35 PAY  32 

    0x659b6ea0,// 36 PAY  33 

    0x8e082c71,// 37 PAY  34 

    0x3cb6ca28,// 38 PAY  35 

    0x59adc412,// 39 PAY  36 

    0x32832c88,// 40 PAY  37 

    0x8464d3da,// 41 PAY  38 

    0xdcb1f6c0,// 42 PAY  39 

    0xe4fc130b,// 43 PAY  40 

    0xb385dd1c,// 44 PAY  41 

    0xf7728119,// 45 PAY  42 

    0x96b53c8f,// 46 PAY  43 

    0x85f32c30,// 47 PAY  44 

    0x2802b5bb,// 48 PAY  45 

    0x419e9f24,// 49 PAY  46 

    0x9eeb0764,// 50 PAY  47 

    0xba7f22b5,// 51 PAY  48 

    0xefb08011,// 52 PAY  49 

    0x4304a7f6,// 53 PAY  50 

    0xdcafeb6f,// 54 PAY  51 

    0x983696e3,// 55 PAY  52 

    0xb1cb50e8,// 56 PAY  53 

    0x5f56f06b,// 57 PAY  54 

    0x2a226b86,// 58 PAY  55 

    0x91a5c7b7,// 59 PAY  56 

    0x314df5b2,// 60 PAY  57 

    0x2045ac14,// 61 PAY  58 

    0x082c1a54,// 62 PAY  59 

    0x58e446cd,// 63 PAY  60 

    0xa752c762,// 64 PAY  61 

    0xb03c6427,// 65 PAY  62 

    0x79cba794,// 66 PAY  63 

    0x403a7043,// 67 PAY  64 

    0x36c8b6c9,// 68 PAY  65 

    0x01d9c21c,// 69 PAY  66 

    0x3c0857ec,// 70 PAY  67 

    0x42c629c9,// 71 PAY  68 

    0xb1494c58,// 72 PAY  69 

    0xd427a3b7,// 73 PAY  70 

    0x4359cef6,// 74 PAY  71 

    0x3d8deacb,// 75 PAY  72 

    0xf75dfad7,// 76 PAY  73 

    0x99c2dfb7,// 77 PAY  74 

    0xdee07c65,// 78 PAY  75 

    0xc7c80cd6,// 79 PAY  76 

    0xe713ae3a,// 80 PAY  77 

    0xd528c2c7,// 81 PAY  78 

    0x28eee043,// 82 PAY  79 

    0x286501c1,// 83 PAY  80 

    0xcec45cb2,// 84 PAY  81 

    0x1f37587b,// 85 PAY  82 

    0x22affe32,// 86 PAY  83 

    0x0fabba2e,// 87 PAY  84 

    0xb8c3a74f,// 88 PAY  85 

    0x52414704,// 89 PAY  86 

    0x667d9b53,// 90 PAY  87 

    0x835cc724,// 91 PAY  88 

    0x3f471c2a,// 92 PAY  89 

    0x3719d82f,// 93 PAY  90 

    0x903d5f1b,// 94 PAY  91 

    0x927f0b19,// 95 PAY  92 

    0x5224917d,// 96 PAY  93 

    0xb843d3e2,// 97 PAY  94 

    0x09286a23,// 98 PAY  95 

    0x431f95f9,// 99 PAY  96 

    0xd8a4e08f,// 100 PAY  97 

    0xe36d9df7,// 101 PAY  98 

    0x446f72c9,// 102 PAY  99 

    0xfe2731d0,// 103 PAY 100 

    0x23ae23b2,// 104 PAY 101 

    0xa11d9284,// 105 PAY 102 

    0x6f0e84f3,// 106 PAY 103 

    0xe6e47cb1,// 107 PAY 104 

    0x3b6f5ef6,// 108 PAY 105 

    0x34408367,// 109 PAY 106 

    0x55ca0810,// 110 PAY 107 

    0xb2312f2e,// 111 PAY 108 

    0x6fa8aed4,// 112 PAY 109 

    0x27529af8,// 113 PAY 110 

    0x3e0f0e69,// 114 PAY 111 

    0xd29a2465,// 115 PAY 112 

    0x5cbdd730,// 116 PAY 113 

    0x95ee6948,// 117 PAY 114 

    0xa392a7ca,// 118 PAY 115 

    0xbaac6564,// 119 PAY 116 

    0xc343b8fa,// 120 PAY 117 

    0xcf29760d,// 121 PAY 118 

    0x4ee041e5,// 122 PAY 119 

    0x9ffbe18d,// 123 PAY 120 

    0x750442e7,// 124 PAY 121 

    0xc65a76ec,// 125 PAY 122 

    0x71bcc36e,// 126 PAY 123 

    0xbef965ca,// 127 PAY 124 

    0xf6d8f5c3,// 128 PAY 125 

    0x625f2515,// 129 PAY 126 

    0xc8c488e9,// 130 PAY 127 

    0x0b662b8a,// 131 PAY 128 

    0xb4dfca3c,// 132 PAY 129 

    0xec96183f,// 133 PAY 130 

    0x6f0af78f,// 134 PAY 131 

    0xec12becc,// 135 PAY 132 

    0xc8bb0b40,// 136 PAY 133 

    0x4ce64d2b,// 137 PAY 134 

    0x037e1225,// 138 PAY 135 

    0x4ab6f646,// 139 PAY 136 

    0xea25e6ad,// 140 PAY 137 

    0x64eb5500,// 141 PAY 138 

/// HASH is  20 bytes 

    0x22affe32,// 142 HSH   1 

    0x0fabba2e,// 143 HSH   2 

    0xb8c3a74f,// 144 HSH   3 

    0x52414704,// 145 HSH   4 

    0x667d9b53,// 146 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 27 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02cc3c1b // 147 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt20_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 156 words. 

/// BDA size     is 620 (0x26c) 

/// BDA id       is 0x1692 

    0x026c1692,// 3 BDA   1 

/// PAY Generic Data size   : 620 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x74e9fb19,// 4 PAY   1 

    0x216ce32a,// 5 PAY   2 

    0x59280c87,// 6 PAY   3 

    0xa4d5b50c,// 7 PAY   4 

    0x8602db43,// 8 PAY   5 

    0xf7a5b9b1,// 9 PAY   6 

    0x7268164b,// 10 PAY   7 

    0x8a50203f,// 11 PAY   8 

    0xf75de2d8,// 12 PAY   9 

    0xc8372649,// 13 PAY  10 

    0xdda4c283,// 14 PAY  11 

    0xe27fcf28,// 15 PAY  12 

    0x00dd1a92,// 16 PAY  13 

    0xddf78b0a,// 17 PAY  14 

    0x944ca458,// 18 PAY  15 

    0x3db4cef6,// 19 PAY  16 

    0x1331b786,// 20 PAY  17 

    0xfe966e9c,// 21 PAY  18 

    0xfc09a0b0,// 22 PAY  19 

    0x5c4aa83d,// 23 PAY  20 

    0x6ef679f0,// 24 PAY  21 

    0xff747ed8,// 25 PAY  22 

    0x86d34b09,// 26 PAY  23 

    0x9906ac97,// 27 PAY  24 

    0x9a250401,// 28 PAY  25 

    0x8419ca5f,// 29 PAY  26 

    0x0313be60,// 30 PAY  27 

    0xfa39ed46,// 31 PAY  28 

    0x8654f0d4,// 32 PAY  29 

    0x1ccf1a27,// 33 PAY  30 

    0x5fc545da,// 34 PAY  31 

    0xac701665,// 35 PAY  32 

    0x43429d52,// 36 PAY  33 

    0x7ab93a84,// 37 PAY  34 

    0x1f867a6a,// 38 PAY  35 

    0xce82e971,// 39 PAY  36 

    0xc97049c2,// 40 PAY  37 

    0x308aa1de,// 41 PAY  38 

    0xf2a769f7,// 42 PAY  39 

    0x68e05089,// 43 PAY  40 

    0xb08e1307,// 44 PAY  41 

    0x897d61f0,// 45 PAY  42 

    0xd9058209,// 46 PAY  43 

    0xa8959c48,// 47 PAY  44 

    0xe6dcbef4,// 48 PAY  45 

    0xbd2a4bec,// 49 PAY  46 

    0x882cbf33,// 50 PAY  47 

    0x0711d793,// 51 PAY  48 

    0xa2938634,// 52 PAY  49 

    0x944943d3,// 53 PAY  50 

    0xeb792dec,// 54 PAY  51 

    0x64e6118f,// 55 PAY  52 

    0xd856e76c,// 56 PAY  53 

    0x0f280e62,// 57 PAY  54 

    0xe04de893,// 58 PAY  55 

    0x8b89711d,// 59 PAY  56 

    0x9738743e,// 60 PAY  57 

    0x678ff8d1,// 61 PAY  58 

    0x02c596c3,// 62 PAY  59 

    0x4a82edf5,// 63 PAY  60 

    0x18cbb3d7,// 64 PAY  61 

    0xd0105883,// 65 PAY  62 

    0xf958c837,// 66 PAY  63 

    0x8599b64b,// 67 PAY  64 

    0x3dd9f037,// 68 PAY  65 

    0xa5719c68,// 69 PAY  66 

    0xf29a39bb,// 70 PAY  67 

    0xf1a8ce61,// 71 PAY  68 

    0xcfe5a9b4,// 72 PAY  69 

    0x0cfffac5,// 73 PAY  70 

    0xe5f26910,// 74 PAY  71 

    0xed55c2c1,// 75 PAY  72 

    0x9be8249b,// 76 PAY  73 

    0x45cf8ace,// 77 PAY  74 

    0x98d0b335,// 78 PAY  75 

    0xa39f858b,// 79 PAY  76 

    0x11e32e68,// 80 PAY  77 

    0x0f7a61d9,// 81 PAY  78 

    0x520a00ba,// 82 PAY  79 

    0x51bdf617,// 83 PAY  80 

    0xe73c1ba7,// 84 PAY  81 

    0x40235b22,// 85 PAY  82 

    0xa1eb5183,// 86 PAY  83 

    0xe98b8f56,// 87 PAY  84 

    0x86db59bc,// 88 PAY  85 

    0xc42a688d,// 89 PAY  86 

    0x52e03012,// 90 PAY  87 

    0x599bad64,// 91 PAY  88 

    0x884aca85,// 92 PAY  89 

    0xf6a28373,// 93 PAY  90 

    0x40125293,// 94 PAY  91 

    0x13099cb8,// 95 PAY  92 

    0xe00dc833,// 96 PAY  93 

    0x0a9d4138,// 97 PAY  94 

    0x1a135831,// 98 PAY  95 

    0xe3a22e86,// 99 PAY  96 

    0x779920b7,// 100 PAY  97 

    0xa9157d04,// 101 PAY  98 

    0x66f50141,// 102 PAY  99 

    0xc46b52b3,// 103 PAY 100 

    0xc909c740,// 104 PAY 101 

    0x48ac4a2b,// 105 PAY 102 

    0x06f7fc59,// 106 PAY 103 

    0x663460bf,// 107 PAY 104 

    0xd96d9738,// 108 PAY 105 

    0x25c1b219,// 109 PAY 106 

    0x7a6b4c07,// 110 PAY 107 

    0xed6160ac,// 111 PAY 108 

    0x72048e41,// 112 PAY 109 

    0x4ad589a2,// 113 PAY 110 

    0x9eea4cfc,// 114 PAY 111 

    0x098f459e,// 115 PAY 112 

    0x3445af5d,// 116 PAY 113 

    0x9944043d,// 117 PAY 114 

    0xe07f07e1,// 118 PAY 115 

    0x3ea340de,// 119 PAY 116 

    0x6cce3816,// 120 PAY 117 

    0xa8c05fdd,// 121 PAY 118 

    0x56588881,// 122 PAY 119 

    0x4f31603f,// 123 PAY 120 

    0x5b0ded6a,// 124 PAY 121 

    0x97b8793d,// 125 PAY 122 

    0x2c4d7567,// 126 PAY 123 

    0x2a82ba1c,// 127 PAY 124 

    0x2c50b503,// 128 PAY 125 

    0xb13766bb,// 129 PAY 126 

    0x7486521a,// 130 PAY 127 

    0x8884a061,// 131 PAY 128 

    0x1a7d5c70,// 132 PAY 129 

    0x60f5f9e0,// 133 PAY 130 

    0x5076fa5c,// 134 PAY 131 

    0x19ee123f,// 135 PAY 132 

    0xe2ff59f3,// 136 PAY 133 

    0x4ff17e40,// 137 PAY 134 

    0x8c9e2ea7,// 138 PAY 135 

    0xd03290e0,// 139 PAY 136 

    0x959904dc,// 140 PAY 137 

    0x99d8304f,// 141 PAY 138 

    0x681c9fcb,// 142 PAY 139 

    0xe92e8f4f,// 143 PAY 140 

    0x0dd5636d,// 144 PAY 141 

    0xdf99e13e,// 145 PAY 142 

    0x59ff730c,// 146 PAY 143 

    0x666ca776,// 147 PAY 144 

    0xb7009497,// 148 PAY 145 

    0x0b5b0b12,// 149 PAY 146 

    0x376b0fd7,// 150 PAY 147 

    0x0acd7a65,// 151 PAY 148 

    0xf485e473,// 152 PAY 149 

    0xcfb838d4,// 153 PAY 150 

    0x39ac01bd,// 154 PAY 151 

    0xbe4bec55,// 155 PAY 152 

    0x653bf2a6,// 156 PAY 153 

    0x5bd02184,// 157 PAY 154 

    0x44fb5f4b,// 158 PAY 155 

/// HASH is  16 bytes 

    0x0acd7a65,// 159 HSH   1 

    0xf485e473,// 160 HSH   2 

    0xcfb838d4,// 161 HSH   3 

    0x39ac01bd,// 162 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 230 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x005c5be6 // 163 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt21_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 90 words. 

/// BDA size     is 353 (0x161) 

/// BDA id       is 0xb7c2 

    0x0161b7c2,// 3 BDA   1 

/// PAY Generic Data size   : 353 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xb6591da0,// 4 PAY   1 

    0x922ef262,// 5 PAY   2 

    0x75e9f0ac,// 6 PAY   3 

    0xca6f0a1c,// 7 PAY   4 

    0x4f6ea0d3,// 8 PAY   5 

    0xdee66eda,// 9 PAY   6 

    0x49f9b79e,// 10 PAY   7 

    0xebae19bf,// 11 PAY   8 

    0x8fcb7ec2,// 12 PAY   9 

    0x439d9722,// 13 PAY  10 

    0x37e58514,// 14 PAY  11 

    0x5c73e96d,// 15 PAY  12 

    0x8b98e4c8,// 16 PAY  13 

    0x5e9502eb,// 17 PAY  14 

    0x8511c5e5,// 18 PAY  15 

    0xb2cda944,// 19 PAY  16 

    0x19eb87cc,// 20 PAY  17 

    0xb4735207,// 21 PAY  18 

    0xa40142a3,// 22 PAY  19 

    0xf112ae01,// 23 PAY  20 

    0xbb762c9a,// 24 PAY  21 

    0x20d6ed75,// 25 PAY  22 

    0x6fe95012,// 26 PAY  23 

    0xd7c788ab,// 27 PAY  24 

    0x2f4cd4ca,// 28 PAY  25 

    0x070796f5,// 29 PAY  26 

    0x2fe85961,// 30 PAY  27 

    0x03d7b2ee,// 31 PAY  28 

    0x6cd567e5,// 32 PAY  29 

    0xa0b98540,// 33 PAY  30 

    0x6ab4b727,// 34 PAY  31 

    0x5d883d76,// 35 PAY  32 

    0xfe2143e0,// 36 PAY  33 

    0xb4e71717,// 37 PAY  34 

    0xe371992a,// 38 PAY  35 

    0x7758c8bf,// 39 PAY  36 

    0xf93437c4,// 40 PAY  37 

    0xa5772681,// 41 PAY  38 

    0x2f9ac84c,// 42 PAY  39 

    0xe29a9c70,// 43 PAY  40 

    0xbc0e4488,// 44 PAY  41 

    0xbaa7df93,// 45 PAY  42 

    0x04a520a2,// 46 PAY  43 

    0x112a5580,// 47 PAY  44 

    0x8236b1f8,// 48 PAY  45 

    0xa5861cbd,// 49 PAY  46 

    0x7ff7071a,// 50 PAY  47 

    0xe678da23,// 51 PAY  48 

    0x4ace2022,// 52 PAY  49 

    0x789b736f,// 53 PAY  50 

    0xa92f8e36,// 54 PAY  51 

    0x475fe2a3,// 55 PAY  52 

    0xa79e9adb,// 56 PAY  53 

    0xbfc31626,// 57 PAY  54 

    0xde70a480,// 58 PAY  55 

    0x805e2dca,// 59 PAY  56 

    0x6680934d,// 60 PAY  57 

    0xc46d92ea,// 61 PAY  58 

    0xec8e351b,// 62 PAY  59 

    0xcfb34d68,// 63 PAY  60 

    0xf9aa95fc,// 64 PAY  61 

    0xa2df2e2e,// 65 PAY  62 

    0x9d192e92,// 66 PAY  63 

    0xcae0d908,// 67 PAY  64 

    0xddca430a,// 68 PAY  65 

    0xc730d333,// 69 PAY  66 

    0x24448528,// 70 PAY  67 

    0x2262f405,// 71 PAY  68 

    0x1d723e08,// 72 PAY  69 

    0x72c4faf0,// 73 PAY  70 

    0xd49f6772,// 74 PAY  71 

    0x6062eca4,// 75 PAY  72 

    0x82a847ae,// 76 PAY  73 

    0xf03ad882,// 77 PAY  74 

    0xa59e5f5f,// 78 PAY  75 

    0x9c8b95f3,// 79 PAY  76 

    0xd1a1fce8,// 80 PAY  77 

    0x08a8934f,// 81 PAY  78 

    0x6e0ddea2,// 82 PAY  79 

    0x230065f5,// 83 PAY  80 

    0xf8d8411f,// 84 PAY  81 

    0xbe28d514,// 85 PAY  82 

    0xd77f0796,// 86 PAY  83 

    0xdbf84842,// 87 PAY  84 

    0x73dc6712,// 88 PAY  85 

    0x531b2e1a,// 89 PAY  86 

    0xb091e342,// 90 PAY  87 

    0xe03aba8c,// 91 PAY  88 

    0x37000000,// 92 PAY  89 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 59 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x14 

    0x00ec1485 // 93 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt22_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 49 words. 

/// BDA size     is 192 (0xc0) 

/// BDA id       is 0x393e 

    0x00c0393e,// 3 BDA   1 

/// PAY Generic Data size   : 192 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x418c52d4,// 4 PAY   1 

    0xd60898d1,// 5 PAY   2 

    0x8182a67c,// 6 PAY   3 

    0x6c78a494,// 7 PAY   4 

    0x65bf36ff,// 8 PAY   5 

    0x48aefdfd,// 9 PAY   6 

    0x2146eddb,// 10 PAY   7 

    0xc4e65fb0,// 11 PAY   8 

    0xbcff692e,// 12 PAY   9 

    0x268788ab,// 13 PAY  10 

    0xeed82f2f,// 14 PAY  11 

    0x1a551e4d,// 15 PAY  12 

    0x23001631,// 16 PAY  13 

    0xd0d11b3d,// 17 PAY  14 

    0xc5c57d96,// 18 PAY  15 

    0x080e3c74,// 19 PAY  16 

    0xcbc46fbb,// 20 PAY  17 

    0x485385e8,// 21 PAY  18 

    0x1a8f3bd2,// 22 PAY  19 

    0x2fe9f5ce,// 23 PAY  20 

    0xaa7cbf16,// 24 PAY  21 

    0x32efcc51,// 25 PAY  22 

    0x864b274b,// 26 PAY  23 

    0xbb4b808c,// 27 PAY  24 

    0xee77e78c,// 28 PAY  25 

    0xc016f331,// 29 PAY  26 

    0xccacaeac,// 30 PAY  27 

    0x38845d1e,// 31 PAY  28 

    0x50e608a7,// 32 PAY  29 

    0x5daef07b,// 33 PAY  30 

    0xc79b7880,// 34 PAY  31 

    0x83601c74,// 35 PAY  32 

    0xb8394a90,// 36 PAY  33 

    0xe440ed5a,// 37 PAY  34 

    0x8772bc53,// 38 PAY  35 

    0x6dbf306a,// 39 PAY  36 

    0xa08a3160,// 40 PAY  37 

    0x0f5bc0bd,// 41 PAY  38 

    0x69882ecf,// 42 PAY  39 

    0x8bc8930a,// 43 PAY  40 

    0x62e73e7c,// 44 PAY  41 

    0x51462a73,// 45 PAY  42 

    0xab0cf552,// 46 PAY  43 

    0x2a6ecfb9,// 47 PAY  44 

    0x12f556be,// 48 PAY  45 

    0xac222cb3,// 49 PAY  46 

    0x69ff1604,// 50 PAY  47 

    0xf1a7e87b,// 51 PAY  48 

/// HASH is  4 bytes 

    0x23001631,// 52 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 76 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x80 

    0x0160804c // 53 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt23_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 77 words. 

/// BDA size     is 303 (0x12f) 

/// BDA id       is 0x6a15 

    0x012f6a15,// 3 BDA   1 

/// PAY Generic Data size   : 303 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x3220ea17,// 4 PAY   1 

    0xc62717e4,// 5 PAY   2 

    0x4ff878ef,// 6 PAY   3 

    0x1275b11e,// 7 PAY   4 

    0x5025efaa,// 8 PAY   5 

    0xa37cf9ea,// 9 PAY   6 

    0x90ec4c6b,// 10 PAY   7 

    0x9632c572,// 11 PAY   8 

    0x4688546d,// 12 PAY   9 

    0xb111a0af,// 13 PAY  10 

    0xb09e15ad,// 14 PAY  11 

    0xcc38780d,// 15 PAY  12 

    0xa3055a3c,// 16 PAY  13 

    0xa71a0ee8,// 17 PAY  14 

    0x10b71338,// 18 PAY  15 

    0x6a26bb3c,// 19 PAY  16 

    0x0ece6c90,// 20 PAY  17 

    0x91a6f263,// 21 PAY  18 

    0x6138212f,// 22 PAY  19 

    0xe91aa4fd,// 23 PAY  20 

    0xf663fc5e,// 24 PAY  21 

    0x5f1d545d,// 25 PAY  22 

    0x2c06d337,// 26 PAY  23 

    0xa22f2e38,// 27 PAY  24 

    0x4b1e101a,// 28 PAY  25 

    0xa229aaf6,// 29 PAY  26 

    0x29065055,// 30 PAY  27 

    0x67fe1cab,// 31 PAY  28 

    0x1e73b8a8,// 32 PAY  29 

    0x83c3b379,// 33 PAY  30 

    0xccc88bbc,// 34 PAY  31 

    0x568c14a5,// 35 PAY  32 

    0x92b11151,// 36 PAY  33 

    0xc4f5ec86,// 37 PAY  34 

    0xd0a4e31a,// 38 PAY  35 

    0x2eaae476,// 39 PAY  36 

    0x9455f01d,// 40 PAY  37 

    0x083aa7f3,// 41 PAY  38 

    0xe069e128,// 42 PAY  39 

    0x4f5ea006,// 43 PAY  40 

    0x1f864fcf,// 44 PAY  41 

    0x60dd4069,// 45 PAY  42 

    0x6bc771ce,// 46 PAY  43 

    0x182c8a14,// 47 PAY  44 

    0xd1dacb79,// 48 PAY  45 

    0xb2c6f969,// 49 PAY  46 

    0x5bbcda09,// 50 PAY  47 

    0x2272382f,// 51 PAY  48 

    0x6db758a8,// 52 PAY  49 

    0x3c6d6141,// 53 PAY  50 

    0xf5eb9c13,// 54 PAY  51 

    0x071824cd,// 55 PAY  52 

    0xbae54ebf,// 56 PAY  53 

    0x7281b5e2,// 57 PAY  54 

    0x274792ed,// 58 PAY  55 

    0xc4b5c19a,// 59 PAY  56 

    0x4bbd2361,// 60 PAY  57 

    0xb47fc716,// 61 PAY  58 

    0x9c076936,// 62 PAY  59 

    0x1fbebd79,// 63 PAY  60 

    0xa90b0e84,// 64 PAY  61 

    0x1e51db66,// 65 PAY  62 

    0x476adf27,// 66 PAY  63 

    0x86df435f,// 67 PAY  64 

    0x719c2938,// 68 PAY  65 

    0xf58f4d66,// 69 PAY  66 

    0x03c22949,// 70 PAY  67 

    0x3f2b6ec3,// 71 PAY  68 

    0x258ab9ab,// 72 PAY  69 

    0x6efa4197,// 73 PAY  70 

    0x44a2bd09,// 74 PAY  71 

    0xaea7b2f4,// 75 PAY  72 

    0xdba9c85a,// 76 PAY  73 

    0xcb0a9b73,// 77 PAY  74 

    0xbbd157ce,// 78 PAY  75 

    0xee80c600,// 79 PAY  76 

/// STA is 1 words. 

/// STA num_pkts       : 23 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd8 

    0x03d9d817 // 80 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt24_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 99 words. 

/// BDA size     is 391 (0x187) 

/// BDA id       is 0x2a9b 

    0x01872a9b,// 3 BDA   1 

/// PAY Generic Data size   : 391 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf69b4fbf,// 4 PAY   1 

    0xb6688db8,// 5 PAY   2 

    0x14f73122,// 6 PAY   3 

    0xcce95ba7,// 7 PAY   4 

    0xd9a321b9,// 8 PAY   5 

    0x9f877cf3,// 9 PAY   6 

    0xf8edbee8,// 10 PAY   7 

    0x228b5303,// 11 PAY   8 

    0xaa81c0fe,// 12 PAY   9 

    0xb368922d,// 13 PAY  10 

    0xd8a4bd86,// 14 PAY  11 

    0xe794d5b7,// 15 PAY  12 

    0xb8a813bc,// 16 PAY  13 

    0xef1780b5,// 17 PAY  14 

    0xe900886a,// 18 PAY  15 

    0x212d31b5,// 19 PAY  16 

    0x1011eb65,// 20 PAY  17 

    0x46a1debd,// 21 PAY  18 

    0x9cc20f91,// 22 PAY  19 

    0x759c5ea9,// 23 PAY  20 

    0xe83ccddd,// 24 PAY  21 

    0xc52915cc,// 25 PAY  22 

    0x9308ec13,// 26 PAY  23 

    0x847dc0ce,// 27 PAY  24 

    0xd4a5276b,// 28 PAY  25 

    0x7eff7d22,// 29 PAY  26 

    0xbf965bf7,// 30 PAY  27 

    0xc0670e5d,// 31 PAY  28 

    0x5280b671,// 32 PAY  29 

    0xade567d7,// 33 PAY  30 

    0xa130e63f,// 34 PAY  31 

    0x51a3fd28,// 35 PAY  32 

    0x773b5f28,// 36 PAY  33 

    0xba3e5feb,// 37 PAY  34 

    0xc721eb62,// 38 PAY  35 

    0xf5641777,// 39 PAY  36 

    0x6950d00b,// 40 PAY  37 

    0x9849d94c,// 41 PAY  38 

    0x3b7f963a,// 42 PAY  39 

    0x3224a363,// 43 PAY  40 

    0xc0f48359,// 44 PAY  41 

    0x80b4bfdb,// 45 PAY  42 

    0x0a6e2271,// 46 PAY  43 

    0xfbab37ab,// 47 PAY  44 

    0x8c802f4a,// 48 PAY  45 

    0x11b79297,// 49 PAY  46 

    0x57f11283,// 50 PAY  47 

    0x927c9ec7,// 51 PAY  48 

    0x9676c564,// 52 PAY  49 

    0x2db76391,// 53 PAY  50 

    0x85518125,// 54 PAY  51 

    0xdf92d815,// 55 PAY  52 

    0xeed87add,// 56 PAY  53 

    0x9ee247aa,// 57 PAY  54 

    0x6194ac5d,// 58 PAY  55 

    0x0be4d40d,// 59 PAY  56 

    0x3f53e24a,// 60 PAY  57 

    0x569e418e,// 61 PAY  58 

    0x493c0ab6,// 62 PAY  59 

    0xf72cdf5a,// 63 PAY  60 

    0x11c8e4e0,// 64 PAY  61 

    0xab53388a,// 65 PAY  62 

    0x42782ccf,// 66 PAY  63 

    0x769938ad,// 67 PAY  64 

    0x4102fcdd,// 68 PAY  65 

    0x290a874f,// 69 PAY  66 

    0x6d467ab9,// 70 PAY  67 

    0x4b28ced0,// 71 PAY  68 

    0x73f36f27,// 72 PAY  69 

    0xb6f4a54a,// 73 PAY  70 

    0x17ae2873,// 74 PAY  71 

    0x50ed9195,// 75 PAY  72 

    0xa2a92a23,// 76 PAY  73 

    0x54fb0c56,// 77 PAY  74 

    0xe25caed2,// 78 PAY  75 

    0x9bc7cd02,// 79 PAY  76 

    0x6b8c3eeb,// 80 PAY  77 

    0x79110be1,// 81 PAY  78 

    0x222029fa,// 82 PAY  79 

    0x3f47cce2,// 83 PAY  80 

    0x66c4e230,// 84 PAY  81 

    0x017205be,// 85 PAY  82 

    0xc1142bcc,// 86 PAY  83 

    0x387594d5,// 87 PAY  84 

    0xf33b09d0,// 88 PAY  85 

    0x14439f2a,// 89 PAY  86 

    0x561cffc4,// 90 PAY  87 

    0x52ed032a,// 91 PAY  88 

    0x82cb44d3,// 92 PAY  89 

    0xba4342fc,// 93 PAY  90 

    0x8b33c303,// 94 PAY  91 

    0x66a0fb09,// 95 PAY  92 

    0xf0ed431d,// 96 PAY  93 

    0x8963a3dd,// 97 PAY  94 

    0xb62310e8,// 98 PAY  95 

    0xb99d53eb,// 99 PAY  96 

    0x6e904060,// 100 PAY  97 

    0x314c6b00,// 101 PAY  98 

/// STA is 1 words. 

/// STA num_pkts       : 151 

/// STA pkt_idx        : 156 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3c 

    0x02703c97 // 102 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt25_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 152 words. 

/// BDA size     is 604 (0x25c) 

/// BDA id       is 0xdca0 

    0x025cdca0,// 3 BDA   1 

/// PAY Generic Data size   : 604 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x765a2763,// 4 PAY   1 

    0xcb98b105,// 5 PAY   2 

    0x11c77263,// 6 PAY   3 

    0xc9719261,// 7 PAY   4 

    0x84f4e884,// 8 PAY   5 

    0x2a5498e7,// 9 PAY   6 

    0xd032c78d,// 10 PAY   7 

    0x6ea3728a,// 11 PAY   8 

    0xbd4cda88,// 12 PAY   9 

    0xb3016017,// 13 PAY  10 

    0x1cff1483,// 14 PAY  11 

    0x80df93b9,// 15 PAY  12 

    0x4327fa13,// 16 PAY  13 

    0x10e997e0,// 17 PAY  14 

    0x98272f74,// 18 PAY  15 

    0xe1dc69a2,// 19 PAY  16 

    0xdda24def,// 20 PAY  17 

    0x17f30e7e,// 21 PAY  18 

    0x27950709,// 22 PAY  19 

    0xc70922fe,// 23 PAY  20 

    0x80c5a8ac,// 24 PAY  21 

    0x577ab577,// 25 PAY  22 

    0x02635781,// 26 PAY  23 

    0x83e9c364,// 27 PAY  24 

    0x47886273,// 28 PAY  25 

    0x7710ea8b,// 29 PAY  26 

    0x1c70797e,// 30 PAY  27 

    0x35e2455d,// 31 PAY  28 

    0xb7810b6e,// 32 PAY  29 

    0xe943de09,// 33 PAY  30 

    0x8d31de6f,// 34 PAY  31 

    0xdc04d496,// 35 PAY  32 

    0x27c343ae,// 36 PAY  33 

    0x10fc307a,// 37 PAY  34 

    0x682d9813,// 38 PAY  35 

    0x630e5797,// 39 PAY  36 

    0xbea1f354,// 40 PAY  37 

    0x8f3724bb,// 41 PAY  38 

    0xc422637b,// 42 PAY  39 

    0xdace7f17,// 43 PAY  40 

    0x1781f5d0,// 44 PAY  41 

    0x8ab4fa04,// 45 PAY  42 

    0x32393878,// 46 PAY  43 

    0x7c0382a1,// 47 PAY  44 

    0x8667fa62,// 48 PAY  45 

    0xebb5a156,// 49 PAY  46 

    0x1f7923ee,// 50 PAY  47 

    0xd3a4f11e,// 51 PAY  48 

    0x3a61c0a9,// 52 PAY  49 

    0x35addb2e,// 53 PAY  50 

    0x19eeebc8,// 54 PAY  51 

    0x8bb39bbb,// 55 PAY  52 

    0x4c2476fe,// 56 PAY  53 

    0x69e527e8,// 57 PAY  54 

    0x72311876,// 58 PAY  55 

    0x2a89cdef,// 59 PAY  56 

    0x1e26ec42,// 60 PAY  57 

    0xab5be55d,// 61 PAY  58 

    0xe2406154,// 62 PAY  59 

    0x083d2006,// 63 PAY  60 

    0x4b627f7f,// 64 PAY  61 

    0x4c63a693,// 65 PAY  62 

    0xeeac102d,// 66 PAY  63 

    0x8e0978b6,// 67 PAY  64 

    0x9c4bafe1,// 68 PAY  65 

    0xaa20c66f,// 69 PAY  66 

    0xff4c8ce3,// 70 PAY  67 

    0xf5df04c0,// 71 PAY  68 

    0x87673677,// 72 PAY  69 

    0xa8c7f57f,// 73 PAY  70 

    0x39b4fc49,// 74 PAY  71 

    0x00161428,// 75 PAY  72 

    0x07039c71,// 76 PAY  73 

    0xc744839c,// 77 PAY  74 

    0x2dd59421,// 78 PAY  75 

    0x4bc00bfd,// 79 PAY  76 

    0xc14134f6,// 80 PAY  77 

    0x4a1f7048,// 81 PAY  78 

    0xe85a3c99,// 82 PAY  79 

    0xf2fc8d6d,// 83 PAY  80 

    0xea05f237,// 84 PAY  81 

    0x719e9572,// 85 PAY  82 

    0xb3c7db66,// 86 PAY  83 

    0x2fbfe980,// 87 PAY  84 

    0x42a44acd,// 88 PAY  85 

    0x94a89e98,// 89 PAY  86 

    0xf2b97b76,// 90 PAY  87 

    0x2bcdad43,// 91 PAY  88 

    0xe7426bc3,// 92 PAY  89 

    0x772e2bf2,// 93 PAY  90 

    0x0e12d841,// 94 PAY  91 

    0xa4690f12,// 95 PAY  92 

    0xc13d83b1,// 96 PAY  93 

    0xb096ef02,// 97 PAY  94 

    0xfb0484a4,// 98 PAY  95 

    0xe8782b14,// 99 PAY  96 

    0x118614a3,// 100 PAY  97 

    0xdd2f89ad,// 101 PAY  98 

    0x8eba930e,// 102 PAY  99 

    0x961a92c5,// 103 PAY 100 

    0x4f42578e,// 104 PAY 101 

    0xe017005f,// 105 PAY 102 

    0x3628699a,// 106 PAY 103 

    0xd368c698,// 107 PAY 104 

    0x3a788571,// 108 PAY 105 

    0xdca892d9,// 109 PAY 106 

    0x828c97ea,// 110 PAY 107 

    0xad75441f,// 111 PAY 108 

    0x454afe68,// 112 PAY 109 

    0xe8062bc3,// 113 PAY 110 

    0x7eb9044d,// 114 PAY 111 

    0x274ef03f,// 115 PAY 112 

    0x6e793dff,// 116 PAY 113 

    0x50fbbac4,// 117 PAY 114 

    0x4f909c11,// 118 PAY 115 

    0x2b925014,// 119 PAY 116 

    0x88df44da,// 120 PAY 117 

    0x4ceb72ff,// 121 PAY 118 

    0x0f844d6f,// 122 PAY 119 

    0xbb66b6e2,// 123 PAY 120 

    0xd5503bb1,// 124 PAY 121 

    0xf6c5319a,// 125 PAY 122 

    0x497ea22b,// 126 PAY 123 

    0x1e3b796e,// 127 PAY 124 

    0x6a2178f5,// 128 PAY 125 

    0xe7d8f259,// 129 PAY 126 

    0x0dde3f60,// 130 PAY 127 

    0x5c3012ba,// 131 PAY 128 

    0x37e8541d,// 132 PAY 129 

    0x8dea4678,// 133 PAY 130 

    0x12ebbb46,// 134 PAY 131 

    0xd039803c,// 135 PAY 132 

    0x48a51c46,// 136 PAY 133 

    0x752e4697,// 137 PAY 134 

    0x52ecde21,// 138 PAY 135 

    0x673ee5d3,// 139 PAY 136 

    0xd8208bca,// 140 PAY 137 

    0x935752a3,// 141 PAY 138 

    0xd9fbc3d9,// 142 PAY 139 

    0x6503f912,// 143 PAY 140 

    0x092a9d4b,// 144 PAY 141 

    0x676331c9,// 145 PAY 142 

    0x138e1c0a,// 146 PAY 143 

    0xbf680b03,// 147 PAY 144 

    0xcfb58f5c,// 148 PAY 145 

    0xaa489033,// 149 PAY 146 

    0x01d56012,// 150 PAY 147 

    0x5dd16939,// 151 PAY 148 

    0xa28495b6,// 152 PAY 149 

    0x4862083b,// 153 PAY 150 

    0x13fe2db5,// 154 PAY 151 

/// HASH is  16 bytes 

    0x6e793dff,// 155 HSH   1 

    0x50fbbac4,// 156 HSH   2 

    0x4f909c11,// 157 HSH   3 

    0x2b925014,// 158 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 145 

/// STA pkt_idx        : 230 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x03994d91 // 159 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt26_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 295 words. 

/// BDA size     is 1175 (0x497) 

/// BDA id       is 0x9ed4 

    0x04979ed4,// 3 BDA   1 

/// PAY Generic Data size   : 1175 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x7cb399fd,// 4 PAY   1 

    0xdd277b25,// 5 PAY   2 

    0xd4fedf62,// 6 PAY   3 

    0x4dac0c91,// 7 PAY   4 

    0x5eb1bec0,// 8 PAY   5 

    0x76c38205,// 9 PAY   6 

    0x50b03b5a,// 10 PAY   7 

    0x879d3a22,// 11 PAY   8 

    0xf55619d2,// 12 PAY   9 

    0x2312b2f4,// 13 PAY  10 

    0x8b773bd2,// 14 PAY  11 

    0xabc23b37,// 15 PAY  12 

    0x7597fd38,// 16 PAY  13 

    0x303fcaaf,// 17 PAY  14 

    0xd3e39c74,// 18 PAY  15 

    0x5a973d9b,// 19 PAY  16 

    0xb8333ebe,// 20 PAY  17 

    0x0e5045dd,// 21 PAY  18 

    0x9ce92bce,// 22 PAY  19 

    0x43b78583,// 23 PAY  20 

    0x2073b36c,// 24 PAY  21 

    0x1a903987,// 25 PAY  22 

    0x178f52ba,// 26 PAY  23 

    0x194ba172,// 27 PAY  24 

    0xe49d419a,// 28 PAY  25 

    0x5ec6dae5,// 29 PAY  26 

    0xac24fcf1,// 30 PAY  27 

    0xd4084c25,// 31 PAY  28 

    0xe426b64d,// 32 PAY  29 

    0xe8bd03c6,// 33 PAY  30 

    0x926d1a6d,// 34 PAY  31 

    0x1e705504,// 35 PAY  32 

    0xaebaf796,// 36 PAY  33 

    0x033e66af,// 37 PAY  34 

    0xf93d5d41,// 38 PAY  35 

    0x75f0a1ca,// 39 PAY  36 

    0x223e8d31,// 40 PAY  37 

    0x3219aba0,// 41 PAY  38 

    0x4f3bc1f7,// 42 PAY  39 

    0x3fb9ddf7,// 43 PAY  40 

    0x6c7e49cc,// 44 PAY  41 

    0x01be39e3,// 45 PAY  42 

    0xe18d3000,// 46 PAY  43 

    0x0cc02e2d,// 47 PAY  44 

    0x1e6ba6fd,// 48 PAY  45 

    0x52f8eedd,// 49 PAY  46 

    0x3097c942,// 50 PAY  47 

    0x503f38e5,// 51 PAY  48 

    0x043679ca,// 52 PAY  49 

    0x16e35c24,// 53 PAY  50 

    0x64c037fe,// 54 PAY  51 

    0x7392fc44,// 55 PAY  52 

    0x02643e72,// 56 PAY  53 

    0x2a266637,// 57 PAY  54 

    0x245b18f7,// 58 PAY  55 

    0x1d9ee281,// 59 PAY  56 

    0x88d98109,// 60 PAY  57 

    0x520c733d,// 61 PAY  58 

    0x247d602f,// 62 PAY  59 

    0xa06c3078,// 63 PAY  60 

    0x82c538e3,// 64 PAY  61 

    0x1e75d093,// 65 PAY  62 

    0xfa30c1e5,// 66 PAY  63 

    0x119aa495,// 67 PAY  64 

    0x446dff3c,// 68 PAY  65 

    0x33fa6d3f,// 69 PAY  66 

    0x02270e35,// 70 PAY  67 

    0x1a381bb5,// 71 PAY  68 

    0xd3041eea,// 72 PAY  69 

    0xe98d7c8f,// 73 PAY  70 

    0x98c6508a,// 74 PAY  71 

    0x87ed0d25,// 75 PAY  72 

    0xde9c2b61,// 76 PAY  73 

    0x5b91427d,// 77 PAY  74 

    0x7454f58e,// 78 PAY  75 

    0xe83150cd,// 79 PAY  76 

    0x73239ebf,// 80 PAY  77 

    0x7dc45047,// 81 PAY  78 

    0xd80201f1,// 82 PAY  79 

    0x9f1f81e5,// 83 PAY  80 

    0x6e63d969,// 84 PAY  81 

    0x74fa6165,// 85 PAY  82 

    0x63485d9c,// 86 PAY  83 

    0xbfdbd37c,// 87 PAY  84 

    0xac7634e0,// 88 PAY  85 

    0x2fde3a7d,// 89 PAY  86 

    0xa24c4338,// 90 PAY  87 

    0xc1d29bfa,// 91 PAY  88 

    0x92c803c1,// 92 PAY  89 

    0xa5dac28e,// 93 PAY  90 

    0xfcc84ce2,// 94 PAY  91 

    0x8298b189,// 95 PAY  92 

    0x5530d0ba,// 96 PAY  93 

    0xa1eff4ae,// 97 PAY  94 

    0x3012f6b8,// 98 PAY  95 

    0x71d61f74,// 99 PAY  96 

    0x87cd10bd,// 100 PAY  97 

    0x8a43669a,// 101 PAY  98 

    0xb548279f,// 102 PAY  99 

    0x0a8bee17,// 103 PAY 100 

    0xfe389be7,// 104 PAY 101 

    0x7501a92e,// 105 PAY 102 

    0xbc46b148,// 106 PAY 103 

    0x892657bc,// 107 PAY 104 

    0x8f3183c0,// 108 PAY 105 

    0x1c4c7f2d,// 109 PAY 106 

    0xa2abfd45,// 110 PAY 107 

    0xbe950715,// 111 PAY 108 

    0x062cbbf5,// 112 PAY 109 

    0x87f0fa8d,// 113 PAY 110 

    0x167ab394,// 114 PAY 111 

    0xc14b481a,// 115 PAY 112 

    0x6ef857cf,// 116 PAY 113 

    0x3ac294ec,// 117 PAY 114 

    0x96d44e59,// 118 PAY 115 

    0xd9a7499c,// 119 PAY 116 

    0x7d3d4825,// 120 PAY 117 

    0x92ca0eb6,// 121 PAY 118 

    0x00234111,// 122 PAY 119 

    0xc2b7f24d,// 123 PAY 120 

    0xe7b67a63,// 124 PAY 121 

    0xf01663c4,// 125 PAY 122 

    0xb991564f,// 126 PAY 123 

    0x3d1ba290,// 127 PAY 124 

    0xe430d0a7,// 128 PAY 125 

    0xea9e6441,// 129 PAY 126 

    0x3b272914,// 130 PAY 127 

    0x89cbd0ac,// 131 PAY 128 

    0x6881a57a,// 132 PAY 129 

    0xc4472840,// 133 PAY 130 

    0x2dea35ca,// 134 PAY 131 

    0x7cf80498,// 135 PAY 132 

    0xa020d081,// 136 PAY 133 

    0x8cdb55db,// 137 PAY 134 

    0x7f27398c,// 138 PAY 135 

    0x2c91a3ca,// 139 PAY 136 

    0x28639a9e,// 140 PAY 137 

    0x9f804aca,// 141 PAY 138 

    0x14bd2a01,// 142 PAY 139 

    0x823f41a0,// 143 PAY 140 

    0xbfae014d,// 144 PAY 141 

    0xff26fb48,// 145 PAY 142 

    0x63b2e221,// 146 PAY 143 

    0xc924a421,// 147 PAY 144 

    0x3302888b,// 148 PAY 145 

    0xb4a971ba,// 149 PAY 146 

    0x6e1ff127,// 150 PAY 147 

    0x290be738,// 151 PAY 148 

    0xe98c1512,// 152 PAY 149 

    0xca68609f,// 153 PAY 150 

    0x2a1ffc28,// 154 PAY 151 

    0xaf3462da,// 155 PAY 152 

    0x8c0ac601,// 156 PAY 153 

    0x64cece64,// 157 PAY 154 

    0x611691f1,// 158 PAY 155 

    0x78be3a7d,// 159 PAY 156 

    0x3c3244b6,// 160 PAY 157 

    0xb4cbb768,// 161 PAY 158 

    0xc34838ac,// 162 PAY 159 

    0x63e28c9d,// 163 PAY 160 

    0xcff1a796,// 164 PAY 161 

    0x2d163e39,// 165 PAY 162 

    0x3059b99d,// 166 PAY 163 

    0x6f3375cf,// 167 PAY 164 

    0xba57a608,// 168 PAY 165 

    0xf27c1bee,// 169 PAY 166 

    0x829178b0,// 170 PAY 167 

    0x7281af6b,// 171 PAY 168 

    0x6b5b3bee,// 172 PAY 169 

    0x230c65a8,// 173 PAY 170 

    0x71224a56,// 174 PAY 171 

    0xe43facbe,// 175 PAY 172 

    0x09cdbb6d,// 176 PAY 173 

    0x2b39b0e4,// 177 PAY 174 

    0x9c815b64,// 178 PAY 175 

    0xbf857892,// 179 PAY 176 

    0x919ce6f0,// 180 PAY 177 

    0xa183ed0a,// 181 PAY 178 

    0x7f4ef415,// 182 PAY 179 

    0xb869f6b9,// 183 PAY 180 

    0xda82bff0,// 184 PAY 181 

    0xa3ae439f,// 185 PAY 182 

    0xddc9021c,// 186 PAY 183 

    0x7bdb7f81,// 187 PAY 184 

    0xb2069e7a,// 188 PAY 185 

    0x3f1663ad,// 189 PAY 186 

    0x22b17a79,// 190 PAY 187 

    0xbd595ae6,// 191 PAY 188 

    0x3d5c91d3,// 192 PAY 189 

    0x969cdaa1,// 193 PAY 190 

    0xb9db2eae,// 194 PAY 191 

    0xe9458902,// 195 PAY 192 

    0x67adfda7,// 196 PAY 193 

    0xb6996274,// 197 PAY 194 

    0xc78801b5,// 198 PAY 195 

    0xbea988ae,// 199 PAY 196 

    0x46f60c03,// 200 PAY 197 

    0x68fe2e76,// 201 PAY 198 

    0x5513f889,// 202 PAY 199 

    0x42a9e3f6,// 203 PAY 200 

    0x9f61285d,// 204 PAY 201 

    0xe8fddb92,// 205 PAY 202 

    0x92bfb559,// 206 PAY 203 

    0x555316f0,// 207 PAY 204 

    0x52e6de94,// 208 PAY 205 

    0x873c8019,// 209 PAY 206 

    0x8412dd19,// 210 PAY 207 

    0xbc1bf365,// 211 PAY 208 

    0xb24377a2,// 212 PAY 209 

    0xd01bdac4,// 213 PAY 210 

    0x52422f78,// 214 PAY 211 

    0x8b870aa7,// 215 PAY 212 

    0x17acfd8d,// 216 PAY 213 

    0xf0847a37,// 217 PAY 214 

    0xf245be2d,// 218 PAY 215 

    0x781cac1f,// 219 PAY 216 

    0x319507b9,// 220 PAY 217 

    0x99a4adc4,// 221 PAY 218 

    0xe849a308,// 222 PAY 219 

    0x073cc81b,// 223 PAY 220 

    0xedf7bd87,// 224 PAY 221 

    0x083e9c84,// 225 PAY 222 

    0xf8d9e0a0,// 226 PAY 223 

    0x2a097f9c,// 227 PAY 224 

    0x162f2fa7,// 228 PAY 225 

    0x99bb6d8a,// 229 PAY 226 

    0xe374ab75,// 230 PAY 227 

    0xecb96f38,// 231 PAY 228 

    0x82d39eda,// 232 PAY 229 

    0x1d705210,// 233 PAY 230 

    0x6fca323d,// 234 PAY 231 

    0x60fd275f,// 235 PAY 232 

    0xfed5dc3c,// 236 PAY 233 

    0x0fe7a062,// 237 PAY 234 

    0x4f9cdc4a,// 238 PAY 235 

    0x6be3f988,// 239 PAY 236 

    0x0c51297e,// 240 PAY 237 

    0x109d8ca2,// 241 PAY 238 

    0xd75a94b5,// 242 PAY 239 

    0x3b443e3a,// 243 PAY 240 

    0x079ce2b6,// 244 PAY 241 

    0xfa503581,// 245 PAY 242 

    0xe07c498b,// 246 PAY 243 

    0x73127dbd,// 247 PAY 244 

    0x3a8b0ed7,// 248 PAY 245 

    0x61a9ab09,// 249 PAY 246 

    0x02835ab2,// 250 PAY 247 

    0x27d11b13,// 251 PAY 248 

    0xa8603a69,// 252 PAY 249 

    0x84e7291a,// 253 PAY 250 

    0x756e7275,// 254 PAY 251 

    0xd6912bd2,// 255 PAY 252 

    0x6a3c9a71,// 256 PAY 253 

    0xeac7beaa,// 257 PAY 254 

    0x36782f0d,// 258 PAY 255 

    0x23d55869,// 259 PAY 256 

    0x57f122c7,// 260 PAY 257 

    0x400d6973,// 261 PAY 258 

    0xd23671fb,// 262 PAY 259 

    0xbdbd106f,// 263 PAY 260 

    0xe0cb787c,// 264 PAY 261 

    0xc07b2283,// 265 PAY 262 

    0xaeb5e645,// 266 PAY 263 

    0x1fe76012,// 267 PAY 264 

    0xcf177c19,// 268 PAY 265 

    0xa13fb766,// 269 PAY 266 

    0x6c6cc4f0,// 270 PAY 267 

    0xd0a55442,// 271 PAY 268 

    0x8bafa729,// 272 PAY 269 

    0xc16f1c5d,// 273 PAY 270 

    0x937706bc,// 274 PAY 271 

    0x87977526,// 275 PAY 272 

    0x6e14aff0,// 276 PAY 273 

    0x9b4c40f9,// 277 PAY 274 

    0x45853ce2,// 278 PAY 275 

    0x258519bf,// 279 PAY 276 

    0xca556c95,// 280 PAY 277 

    0x4449649a,// 281 PAY 278 

    0x944986aa,// 282 PAY 279 

    0xc172cedf,// 283 PAY 280 

    0xb271f66b,// 284 PAY 281 

    0x995f5118,// 285 PAY 282 

    0x1f441d44,// 286 PAY 283 

    0xeb9e9112,// 287 PAY 284 

    0xb233a7f2,// 288 PAY 285 

    0x17b53ea8,// 289 PAY 286 

    0xaa866110,// 290 PAY 287 

    0x28f734ea,// 291 PAY 288 

    0x5db38a8c,// 292 PAY 289 

    0xb526fe7c,// 293 PAY 290 

    0x683eeac0,// 294 PAY 291 

    0x3d295065,// 295 PAY 292 

    0xe877d41c,// 296 PAY 293 

    0xa5a22e00,// 297 PAY 294 

/// STA is 1 words. 

/// STA num_pkts       : 14 

/// STA pkt_idx        : 249 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc8 

    0x03e4c80e // 298 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt27_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 464 words. 

/// BDA size     is 1851 (0x73b) 

/// BDA id       is 0x601b 

    0x073b601b,// 3 BDA   1 

/// PAY Generic Data size   : 1851 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x4caf6171,// 4 PAY   1 

    0xc3cada88,// 5 PAY   2 

    0xafd8b4bc,// 6 PAY   3 

    0x38ffa337,// 7 PAY   4 

    0x6b6c87a5,// 8 PAY   5 

    0xe5c1f9fc,// 9 PAY   6 

    0x2e25df86,// 10 PAY   7 

    0x59646d30,// 11 PAY   8 

    0xda1ace77,// 12 PAY   9 

    0xfde0f0d6,// 13 PAY  10 

    0xe6b01292,// 14 PAY  11 

    0xc050b042,// 15 PAY  12 

    0x8779d0cc,// 16 PAY  13 

    0xfe1d4807,// 17 PAY  14 

    0xeb6190e0,// 18 PAY  15 

    0x821607f8,// 19 PAY  16 

    0x44d3d106,// 20 PAY  17 

    0xe54c6deb,// 21 PAY  18 

    0xe50335c2,// 22 PAY  19 

    0x96c0a8e5,// 23 PAY  20 

    0xb5def83b,// 24 PAY  21 

    0xed1c69fe,// 25 PAY  22 

    0x30934be7,// 26 PAY  23 

    0xc8f67965,// 27 PAY  24 

    0x1cda1053,// 28 PAY  25 

    0x2075724f,// 29 PAY  26 

    0x2802ba31,// 30 PAY  27 

    0x3f13b03e,// 31 PAY  28 

    0x59584817,// 32 PAY  29 

    0x0beb7dbf,// 33 PAY  30 

    0x5ecadbb4,// 34 PAY  31 

    0x65071c52,// 35 PAY  32 

    0x2b83d669,// 36 PAY  33 

    0xa215162e,// 37 PAY  34 

    0x55e822d9,// 38 PAY  35 

    0x44b1b620,// 39 PAY  36 

    0xecffa843,// 40 PAY  37 

    0xeff6f4ef,// 41 PAY  38 

    0x3b794e8d,// 42 PAY  39 

    0x497945e1,// 43 PAY  40 

    0x0fab4181,// 44 PAY  41 

    0xc60cc56a,// 45 PAY  42 

    0x9cb36323,// 46 PAY  43 

    0xcecc69c4,// 47 PAY  44 

    0x23b6d830,// 48 PAY  45 

    0xed457ba4,// 49 PAY  46 

    0x0c6b1db0,// 50 PAY  47 

    0xf0976b90,// 51 PAY  48 

    0xf32369c3,// 52 PAY  49 

    0xdd19ecf4,// 53 PAY  50 

    0x4a9e5f82,// 54 PAY  51 

    0xb94bc9ee,// 55 PAY  52 

    0xfa711bab,// 56 PAY  53 

    0x12637d67,// 57 PAY  54 

    0xdf6782b2,// 58 PAY  55 

    0x4704f523,// 59 PAY  56 

    0x87145ff7,// 60 PAY  57 

    0xbbad2053,// 61 PAY  58 

    0x4e9006ba,// 62 PAY  59 

    0x086eb00c,// 63 PAY  60 

    0xb5556976,// 64 PAY  61 

    0xc4de6018,// 65 PAY  62 

    0x92f32f7f,// 66 PAY  63 

    0x4fa946ad,// 67 PAY  64 

    0xee3f2ce2,// 68 PAY  65 

    0x7de045c7,// 69 PAY  66 

    0x38292758,// 70 PAY  67 

    0xaca509bf,// 71 PAY  68 

    0x2f489dec,// 72 PAY  69 

    0x4e0d167e,// 73 PAY  70 

    0x1c4e32a9,// 74 PAY  71 

    0xf6c0b554,// 75 PAY  72 

    0x515cb24b,// 76 PAY  73 

    0xaa8d00b8,// 77 PAY  74 

    0x37300051,// 78 PAY  75 

    0x921f7e30,// 79 PAY  76 

    0x1edaa9a4,// 80 PAY  77 

    0xe67ed09d,// 81 PAY  78 

    0x512a2f27,// 82 PAY  79 

    0xd69d7eca,// 83 PAY  80 

    0x9ff59006,// 84 PAY  81 

    0x214e632d,// 85 PAY  82 

    0xcbd5eec8,// 86 PAY  83 

    0xcc6a2ab7,// 87 PAY  84 

    0xbfe8024f,// 88 PAY  85 

    0x36d580a2,// 89 PAY  86 

    0xbdaa35a5,// 90 PAY  87 

    0x04eed0f2,// 91 PAY  88 

    0x920c726b,// 92 PAY  89 

    0x97e3da8c,// 93 PAY  90 

    0xb67451bd,// 94 PAY  91 

    0x11eab909,// 95 PAY  92 

    0x1977885c,// 96 PAY  93 

    0x449632c8,// 97 PAY  94 

    0x6b91d573,// 98 PAY  95 

    0xae6ae94b,// 99 PAY  96 

    0x19d117ac,// 100 PAY  97 

    0x5a948075,// 101 PAY  98 

    0x893b2971,// 102 PAY  99 

    0x37fca9c9,// 103 PAY 100 

    0x402de510,// 104 PAY 101 

    0x14d60337,// 105 PAY 102 

    0x123c8cd5,// 106 PAY 103 

    0x4c59f9be,// 107 PAY 104 

    0xcbce21b2,// 108 PAY 105 

    0xc5250e3f,// 109 PAY 106 

    0x2e4d9978,// 110 PAY 107 

    0x16e514f4,// 111 PAY 108 

    0x4c7ea33c,// 112 PAY 109 

    0xc9810fe4,// 113 PAY 110 

    0x34a574fb,// 114 PAY 111 

    0x302d5692,// 115 PAY 112 

    0xba7fb3a1,// 116 PAY 113 

    0xe4738503,// 117 PAY 114 

    0xa6d4a551,// 118 PAY 115 

    0x2c8bccb0,// 119 PAY 116 

    0x8a6b7041,// 120 PAY 117 

    0x7804d68e,// 121 PAY 118 

    0x1a17824d,// 122 PAY 119 

    0x41f650ad,// 123 PAY 120 

    0x211852b9,// 124 PAY 121 

    0xc0e05082,// 125 PAY 122 

    0x6db89269,// 126 PAY 123 

    0xd2f13e02,// 127 PAY 124 

    0x7911c321,// 128 PAY 125 

    0x3061cdfd,// 129 PAY 126 

    0x4d096a15,// 130 PAY 127 

    0x83258d45,// 131 PAY 128 

    0xe141bf2b,// 132 PAY 129 

    0x58f54027,// 133 PAY 130 

    0x3ba6a050,// 134 PAY 131 

    0x3724d0be,// 135 PAY 132 

    0x8a6d9380,// 136 PAY 133 

    0xa69d79e0,// 137 PAY 134 

    0x6ff9fdec,// 138 PAY 135 

    0xf31502e2,// 139 PAY 136 

    0xd22cda81,// 140 PAY 137 

    0x168baf74,// 141 PAY 138 

    0x0cd847cd,// 142 PAY 139 

    0xf4e87489,// 143 PAY 140 

    0x263463db,// 144 PAY 141 

    0x564a4e66,// 145 PAY 142 

    0xa45457a7,// 146 PAY 143 

    0xb388b6c2,// 147 PAY 144 

    0x32a7741f,// 148 PAY 145 

    0xe96dda63,// 149 PAY 146 

    0xec0bc9f2,// 150 PAY 147 

    0x15849baf,// 151 PAY 148 

    0xe017f489,// 152 PAY 149 

    0x9e2cd1bf,// 153 PAY 150 

    0x4e54e292,// 154 PAY 151 

    0xb454f764,// 155 PAY 152 

    0x79d3467e,// 156 PAY 153 

    0x69db21c5,// 157 PAY 154 

    0x0530b022,// 158 PAY 155 

    0x7ab299ef,// 159 PAY 156 

    0xea54ad31,// 160 PAY 157 

    0xa0470d7a,// 161 PAY 158 

    0x0655ceb8,// 162 PAY 159 

    0x14fe1749,// 163 PAY 160 

    0x97648580,// 164 PAY 161 

    0xf57f2cd4,// 165 PAY 162 

    0x7bc2ceb9,// 166 PAY 163 

    0xe1ccb177,// 167 PAY 164 

    0xf4e999b9,// 168 PAY 165 

    0x69c15926,// 169 PAY 166 

    0x66ecaf25,// 170 PAY 167 

    0x4e666fea,// 171 PAY 168 

    0x3b19348e,// 172 PAY 169 

    0xaa15e9f3,// 173 PAY 170 

    0x9a6c45eb,// 174 PAY 171 

    0xa9eaa008,// 175 PAY 172 

    0x6b8f848f,// 176 PAY 173 

    0x525ee4f3,// 177 PAY 174 

    0x8976e934,// 178 PAY 175 

    0xc3b1c1f0,// 179 PAY 176 

    0xa623d261,// 180 PAY 177 

    0xdfaffdbe,// 181 PAY 178 

    0x5c566f38,// 182 PAY 179 

    0xfb253181,// 183 PAY 180 

    0x4ecc12d4,// 184 PAY 181 

    0xcdd4abc5,// 185 PAY 182 

    0x0dc0dbd6,// 186 PAY 183 

    0x0896647e,// 187 PAY 184 

    0x86f9caaf,// 188 PAY 185 

    0x09d3d721,// 189 PAY 186 

    0xf60047f0,// 190 PAY 187 

    0xdb8d5bf3,// 191 PAY 188 

    0x4261605d,// 192 PAY 189 

    0x3c83d69a,// 193 PAY 190 

    0xbca4fe8f,// 194 PAY 191 

    0xd006ce1c,// 195 PAY 192 

    0x53f02c0d,// 196 PAY 193 

    0x65d864f4,// 197 PAY 194 

    0xf6ff76e4,// 198 PAY 195 

    0xabd769aa,// 199 PAY 196 

    0x8592bba8,// 200 PAY 197 

    0x6bca728b,// 201 PAY 198 

    0x8153856f,// 202 PAY 199 

    0xfbc075fd,// 203 PAY 200 

    0xc0d2eac4,// 204 PAY 201 

    0xd98178e1,// 205 PAY 202 

    0x17f71a62,// 206 PAY 203 

    0xd134f851,// 207 PAY 204 

    0xcc6ea311,// 208 PAY 205 

    0xd9c1ca12,// 209 PAY 206 

    0xb9e9f8b1,// 210 PAY 207 

    0xa1822f3c,// 211 PAY 208 

    0x6f6165d5,// 212 PAY 209 

    0x21c763df,// 213 PAY 210 

    0x63057320,// 214 PAY 211 

    0x3d8c66cf,// 215 PAY 212 

    0xa3cba1ff,// 216 PAY 213 

    0xaf0158d0,// 217 PAY 214 

    0x904a29d5,// 218 PAY 215 

    0x512c9d4a,// 219 PAY 216 

    0x1da12df3,// 220 PAY 217 

    0xe02fa5b4,// 221 PAY 218 

    0xd278ae22,// 222 PAY 219 

    0x1deaa853,// 223 PAY 220 

    0xb9923bb4,// 224 PAY 221 

    0xef1b1313,// 225 PAY 222 

    0xaa85c938,// 226 PAY 223 

    0x93c5dc27,// 227 PAY 224 

    0x5fb32893,// 228 PAY 225 

    0x28101560,// 229 PAY 226 

    0xe0970638,// 230 PAY 227 

    0x35e553a5,// 231 PAY 228 

    0x0ae66f2c,// 232 PAY 229 

    0xbf432d1b,// 233 PAY 230 

    0x8b6822bf,// 234 PAY 231 

    0x0521286d,// 235 PAY 232 

    0x4777e933,// 236 PAY 233 

    0xd0858db6,// 237 PAY 234 

    0xe8f37f63,// 238 PAY 235 

    0xc8e56613,// 239 PAY 236 

    0x33e4976b,// 240 PAY 237 

    0xfccaf747,// 241 PAY 238 

    0xdba64db8,// 242 PAY 239 

    0x2f1166b3,// 243 PAY 240 

    0xe49ccdc5,// 244 PAY 241 

    0x8fba5a12,// 245 PAY 242 

    0xf1c618bf,// 246 PAY 243 

    0x1bcc36cc,// 247 PAY 244 

    0x3dbf72bc,// 248 PAY 245 

    0x7752ed55,// 249 PAY 246 

    0xc08e1ab1,// 250 PAY 247 

    0x053688dd,// 251 PAY 248 

    0x13b61792,// 252 PAY 249 

    0x832607c9,// 253 PAY 250 

    0x8055db37,// 254 PAY 251 

    0xf6c59957,// 255 PAY 252 

    0x473c698a,// 256 PAY 253 

    0x25d20a0a,// 257 PAY 254 

    0x5a6fd67a,// 258 PAY 255 

    0x34824930,// 259 PAY 256 

    0xa45557f5,// 260 PAY 257 

    0xaa190428,// 261 PAY 258 

    0xad735a3b,// 262 PAY 259 

    0xa3671d47,// 263 PAY 260 

    0xa225254f,// 264 PAY 261 

    0x7be260a6,// 265 PAY 262 

    0x75a70932,// 266 PAY 263 

    0x4314e949,// 267 PAY 264 

    0x0bddcd86,// 268 PAY 265 

    0xf4ad681e,// 269 PAY 266 

    0xc2d0adfa,// 270 PAY 267 

    0x142f2d20,// 271 PAY 268 

    0xb928cfae,// 272 PAY 269 

    0x69b35a4a,// 273 PAY 270 

    0x74795b0c,// 274 PAY 271 

    0x26111565,// 275 PAY 272 

    0x8fb49b1a,// 276 PAY 273 

    0xac978841,// 277 PAY 274 

    0xc0391fb9,// 278 PAY 275 

    0x4c72967f,// 279 PAY 276 

    0x676f15c1,// 280 PAY 277 

    0xe3f04e55,// 281 PAY 278 

    0x6dd3dd83,// 282 PAY 279 

    0x148f3fe0,// 283 PAY 280 

    0x5fb63df9,// 284 PAY 281 

    0x640ad9cf,// 285 PAY 282 

    0x57071785,// 286 PAY 283 

    0xbe689f08,// 287 PAY 284 

    0x03e7a23c,// 288 PAY 285 

    0x43fbcde3,// 289 PAY 286 

    0xaf2ffd48,// 290 PAY 287 

    0x9c6e5ba1,// 291 PAY 288 

    0x748dbc91,// 292 PAY 289 

    0x6e4acd68,// 293 PAY 290 

    0x7bea989e,// 294 PAY 291 

    0x2d02650c,// 295 PAY 292 

    0x6202a46c,// 296 PAY 293 

    0x615cd7b5,// 297 PAY 294 

    0x06d6b3d1,// 298 PAY 295 

    0x56c133b6,// 299 PAY 296 

    0xb588a583,// 300 PAY 297 

    0x060d7e8d,// 301 PAY 298 

    0x5a4df92e,// 302 PAY 299 

    0xf1695c33,// 303 PAY 300 

    0x05a263ae,// 304 PAY 301 

    0x9465cbdb,// 305 PAY 302 

    0xfe1cfcba,// 306 PAY 303 

    0xf5ac324c,// 307 PAY 304 

    0xd302ab60,// 308 PAY 305 

    0x17123458,// 309 PAY 306 

    0xc3d240a8,// 310 PAY 307 

    0x4a1a7061,// 311 PAY 308 

    0x50ffcb73,// 312 PAY 309 

    0x7ce8aa82,// 313 PAY 310 

    0x4d8c3bc5,// 314 PAY 311 

    0xdf7ee746,// 315 PAY 312 

    0x47a120db,// 316 PAY 313 

    0x3d93c132,// 317 PAY 314 

    0xc81bdad2,// 318 PAY 315 

    0xdbdbb053,// 319 PAY 316 

    0x0fdc7f82,// 320 PAY 317 

    0x39d704ef,// 321 PAY 318 

    0x46383d0f,// 322 PAY 319 

    0x21f1530a,// 323 PAY 320 

    0x66459bc6,// 324 PAY 321 

    0x5612ab68,// 325 PAY 322 

    0x8283e68c,// 326 PAY 323 

    0x34f36ead,// 327 PAY 324 

    0x2c1789e9,// 328 PAY 325 

    0x79aad830,// 329 PAY 326 

    0x33591d83,// 330 PAY 327 

    0x8351b019,// 331 PAY 328 

    0xb23530a3,// 332 PAY 329 

    0x78d9bca0,// 333 PAY 330 

    0x4243546c,// 334 PAY 331 

    0x8972883e,// 335 PAY 332 

    0x5b3df9bc,// 336 PAY 333 

    0x6d3e6210,// 337 PAY 334 

    0x6b3bab66,// 338 PAY 335 

    0x8b9f316d,// 339 PAY 336 

    0xa3fdb9f9,// 340 PAY 337 

    0xc1d47029,// 341 PAY 338 

    0xaa7bc224,// 342 PAY 339 

    0xb63403f2,// 343 PAY 340 

    0x049d913e,// 344 PAY 341 

    0x19d1f3cd,// 345 PAY 342 

    0x33bba504,// 346 PAY 343 

    0x15479661,// 347 PAY 344 

    0xf2a9788e,// 348 PAY 345 

    0x121b63ea,// 349 PAY 346 

    0xf42c496e,// 350 PAY 347 

    0x0c5d652a,// 351 PAY 348 

    0xc16ae5b1,// 352 PAY 349 

    0xcb63166c,// 353 PAY 350 

    0x1f5479d3,// 354 PAY 351 

    0xbdd75f6a,// 355 PAY 352 

    0x09d672a6,// 356 PAY 353 

    0xfbf9725a,// 357 PAY 354 

    0x362ac73d,// 358 PAY 355 

    0x3c8a132e,// 359 PAY 356 

    0x095bcea8,// 360 PAY 357 

    0x4101aa70,// 361 PAY 358 

    0x546116d6,// 362 PAY 359 

    0x5613ee6e,// 363 PAY 360 

    0x97b9cb38,// 364 PAY 361 

    0xee75560f,// 365 PAY 362 

    0xa4533d43,// 366 PAY 363 

    0xaff3b7d6,// 367 PAY 364 

    0xd8a8fed6,// 368 PAY 365 

    0xc84c5643,// 369 PAY 366 

    0x5b8673a8,// 370 PAY 367 

    0x1b21d478,// 371 PAY 368 

    0xddb9d8f1,// 372 PAY 369 

    0x77e7f662,// 373 PAY 370 

    0x4f66237f,// 374 PAY 371 

    0xd20f0078,// 375 PAY 372 

    0x68327005,// 376 PAY 373 

    0xf7b9629f,// 377 PAY 374 

    0xed3d8958,// 378 PAY 375 

    0xbbc8a929,// 379 PAY 376 

    0xbf9241cb,// 380 PAY 377 

    0x21b67b02,// 381 PAY 378 

    0xe6420514,// 382 PAY 379 

    0x7dc6b32b,// 383 PAY 380 

    0xc0d7bffb,// 384 PAY 381 

    0x657218f9,// 385 PAY 382 

    0x93871f40,// 386 PAY 383 

    0x5168e597,// 387 PAY 384 

    0xe41617e0,// 388 PAY 385 

    0x5e06d650,// 389 PAY 386 

    0x3b19ad1f,// 390 PAY 387 

    0x0ec98945,// 391 PAY 388 

    0x058cd2f4,// 392 PAY 389 

    0xe1185fc4,// 393 PAY 390 

    0xae5eff08,// 394 PAY 391 

    0x0d5c1161,// 395 PAY 392 

    0x61f60748,// 396 PAY 393 

    0x17e38c71,// 397 PAY 394 

    0x9d566960,// 398 PAY 395 

    0xeb33b759,// 399 PAY 396 

    0x230c9c26,// 400 PAY 397 

    0x17860fdf,// 401 PAY 398 

    0xf05d10af,// 402 PAY 399 

    0xb54a32de,// 403 PAY 400 

    0x29b20064,// 404 PAY 401 

    0xbaf386bb,// 405 PAY 402 

    0x4ddecd82,// 406 PAY 403 

    0x2c3a2a47,// 407 PAY 404 

    0xc8b2c2fd,// 408 PAY 405 

    0xe6ba0565,// 409 PAY 406 

    0xac8446cf,// 410 PAY 407 

    0x89eb076d,// 411 PAY 408 

    0x6b3946d9,// 412 PAY 409 

    0x19219609,// 413 PAY 410 

    0xf4c9d20c,// 414 PAY 411 

    0xc088649a,// 415 PAY 412 

    0x40435fa7,// 416 PAY 413 

    0x4ed7b402,// 417 PAY 414 

    0x45ac1743,// 418 PAY 415 

    0xec83ac49,// 419 PAY 416 

    0xb8d962b5,// 420 PAY 417 

    0x2e938de4,// 421 PAY 418 

    0x2e45d95a,// 422 PAY 419 

    0x9e5e4cb8,// 423 PAY 420 

    0x142ffc58,// 424 PAY 421 

    0x5fb562fb,// 425 PAY 422 

    0xca88101b,// 426 PAY 423 

    0xc2d52696,// 427 PAY 424 

    0x96dbfd0c,// 428 PAY 425 

    0xaa88a8ac,// 429 PAY 426 

    0xdfd7b06c,// 430 PAY 427 

    0x61857ca2,// 431 PAY 428 

    0x3b6f3075,// 432 PAY 429 

    0x0e55d807,// 433 PAY 430 

    0x95c718a6,// 434 PAY 431 

    0x460fa4b9,// 435 PAY 432 

    0x19857c42,// 436 PAY 433 

    0xe3c066fe,// 437 PAY 434 

    0x1a69dbd1,// 438 PAY 435 

    0x394d0cb2,// 439 PAY 436 

    0x2efe1e0f,// 440 PAY 437 

    0xb5919241,// 441 PAY 438 

    0x1a39f560,// 442 PAY 439 

    0x0b3bacbd,// 443 PAY 440 

    0xa712dc03,// 444 PAY 441 

    0x3bbf798d,// 445 PAY 442 

    0x720e7390,// 446 PAY 443 

    0x00b5304d,// 447 PAY 444 

    0x32d8e43a,// 448 PAY 445 

    0xbd412470,// 449 PAY 446 

    0x556e41d9,// 450 PAY 447 

    0xd6f4c04e,// 451 PAY 448 

    0xded6c881,// 452 PAY 449 

    0xa54c4d3a,// 453 PAY 450 

    0x5329b09e,// 454 PAY 451 

    0x40879146,// 455 PAY 452 

    0xf81498ce,// 456 PAY 453 

    0x15539ee5,// 457 PAY 454 

    0xd0a4ce70,// 458 PAY 455 

    0x12cdb171,// 459 PAY 456 

    0x47741f9a,// 460 PAY 457 

    0xa39799f3,// 461 PAY 458 

    0xdee3fe30,// 462 PAY 459 

    0xeebdf926,// 463 PAY 460 

    0x8b7c9366,// 464 PAY 461 

    0xaa2cff96,// 465 PAY 462 

    0x27cbb200,// 466 PAY 463 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x03f8c1d6 // 467 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt28_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 236 words. 

/// BDA size     is 939 (0x3ab) 

/// BDA id       is 0x24a3 

    0x03ab24a3,// 3 BDA   1 

/// PAY Generic Data size   : 939 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x435d4eb7,// 4 PAY   1 

    0x5d6187d7,// 5 PAY   2 

    0xcef32f24,// 6 PAY   3 

    0x21b92d69,// 7 PAY   4 

    0x1689c3ab,// 8 PAY   5 

    0xa1880128,// 9 PAY   6 

    0x14a9cd40,// 10 PAY   7 

    0xb3c0dcf2,// 11 PAY   8 

    0x2df363f1,// 12 PAY   9 

    0x0488adb2,// 13 PAY  10 

    0x0bc1aab0,// 14 PAY  11 

    0xd05aee5a,// 15 PAY  12 

    0x834f2f2c,// 16 PAY  13 

    0xa4165cff,// 17 PAY  14 

    0x6b4354b3,// 18 PAY  15 

    0x63dbbfb0,// 19 PAY  16 

    0xdce2ceeb,// 20 PAY  17 

    0x891d8688,// 21 PAY  18 

    0x47964b55,// 22 PAY  19 

    0xe699f686,// 23 PAY  20 

    0x3aa4e8d0,// 24 PAY  21 

    0x54d6ac9b,// 25 PAY  22 

    0x46a27b8e,// 26 PAY  23 

    0x8d22bddb,// 27 PAY  24 

    0x1e684bfa,// 28 PAY  25 

    0xea1aa16e,// 29 PAY  26 

    0xfb5b18c6,// 30 PAY  27 

    0x2f8187b1,// 31 PAY  28 

    0x69283ede,// 32 PAY  29 

    0xa18f387c,// 33 PAY  30 

    0x54ee1d3d,// 34 PAY  31 

    0xeb6f4991,// 35 PAY  32 

    0x3bf9c7d7,// 36 PAY  33 

    0x1943a052,// 37 PAY  34 

    0x36997cef,// 38 PAY  35 

    0xb4cbb786,// 39 PAY  36 

    0x75a6fe10,// 40 PAY  37 

    0x0e721fe4,// 41 PAY  38 

    0x82b4fc8a,// 42 PAY  39 

    0x7aefb87f,// 43 PAY  40 

    0xcea9b92d,// 44 PAY  41 

    0x0a576bbd,// 45 PAY  42 

    0xdd229c63,// 46 PAY  43 

    0x1aef47c6,// 47 PAY  44 

    0x164e524b,// 48 PAY  45 

    0x9ace60d3,// 49 PAY  46 

    0x4e5ae37d,// 50 PAY  47 

    0xf6b48bab,// 51 PAY  48 

    0xfe1ec290,// 52 PAY  49 

    0x1b824187,// 53 PAY  50 

    0x20ccdf3a,// 54 PAY  51 

    0x4dd39d58,// 55 PAY  52 

    0xe4ec93b2,// 56 PAY  53 

    0x06fe7436,// 57 PAY  54 

    0x7e0c49e3,// 58 PAY  55 

    0x8403f5dc,// 59 PAY  56 

    0x31e3c55f,// 60 PAY  57 

    0x58f2be78,// 61 PAY  58 

    0x27337a8b,// 62 PAY  59 

    0xfe249a08,// 63 PAY  60 

    0x2994daab,// 64 PAY  61 

    0x8f694bec,// 65 PAY  62 

    0x0455d773,// 66 PAY  63 

    0x0a0ccfa9,// 67 PAY  64 

    0x87cfbe67,// 68 PAY  65 

    0x409e6aac,// 69 PAY  66 

    0xbbb3a05e,// 70 PAY  67 

    0x849e864f,// 71 PAY  68 

    0x8c997a1c,// 72 PAY  69 

    0x288e4dc4,// 73 PAY  70 

    0x3fb796f6,// 74 PAY  71 

    0x7542fcfa,// 75 PAY  72 

    0xc9db8412,// 76 PAY  73 

    0x4032158c,// 77 PAY  74 

    0xeb1388a7,// 78 PAY  75 

    0xb01db4e8,// 79 PAY  76 

    0x9ffae8b0,// 80 PAY  77 

    0x5372d6a4,// 81 PAY  78 

    0x947a8882,// 82 PAY  79 

    0x11d4f001,// 83 PAY  80 

    0xffb03bb1,// 84 PAY  81 

    0x5bfcdca0,// 85 PAY  82 

    0x0dec100f,// 86 PAY  83 

    0x07a8d1ab,// 87 PAY  84 

    0x2ada92d6,// 88 PAY  85 

    0xa3dc94fa,// 89 PAY  86 

    0x65c28623,// 90 PAY  87 

    0x086f05fe,// 91 PAY  88 

    0x6f30693c,// 92 PAY  89 

    0x3c693a05,// 93 PAY  90 

    0x4784a031,// 94 PAY  91 

    0x074d06b5,// 95 PAY  92 

    0x684b753d,// 96 PAY  93 

    0xa122739e,// 97 PAY  94 

    0xd9e0ec03,// 98 PAY  95 

    0x0bfa87d8,// 99 PAY  96 

    0x032d8d57,// 100 PAY  97 

    0xfa19181a,// 101 PAY  98 

    0xad5a5694,// 102 PAY  99 

    0x5b13338d,// 103 PAY 100 

    0xd4820527,// 104 PAY 101 

    0xe7fc1f28,// 105 PAY 102 

    0xd8344056,// 106 PAY 103 

    0x5f1ca1ec,// 107 PAY 104 

    0xc970b740,// 108 PAY 105 

    0x51b67783,// 109 PAY 106 

    0x14ccd8c5,// 110 PAY 107 

    0xeb6f6a0f,// 111 PAY 108 

    0xf07c96ad,// 112 PAY 109 

    0xfc5516cc,// 113 PAY 110 

    0x4e41da19,// 114 PAY 111 

    0x6b48cbd0,// 115 PAY 112 

    0x8775aa0f,// 116 PAY 113 

    0x402a6344,// 117 PAY 114 

    0x91633104,// 118 PAY 115 

    0x1f05206b,// 119 PAY 116 

    0xffce900f,// 120 PAY 117 

    0x7011a5ec,// 121 PAY 118 

    0xfa84239f,// 122 PAY 119 

    0x258aa528,// 123 PAY 120 

    0x1cbf179b,// 124 PAY 121 

    0xa3f553bc,// 125 PAY 122 

    0x06931961,// 126 PAY 123 

    0x955b5a6a,// 127 PAY 124 

    0x92dddffa,// 128 PAY 125 

    0x1589cbae,// 129 PAY 126 

    0x124ae5e0,// 130 PAY 127 

    0xb8618b53,// 131 PAY 128 

    0x11744156,// 132 PAY 129 

    0xb0ddc85f,// 133 PAY 130 

    0x3865ad7d,// 134 PAY 131 

    0xa0d06cea,// 135 PAY 132 

    0xd315f0bc,// 136 PAY 133 

    0xdc841d26,// 137 PAY 134 

    0xacda4f39,// 138 PAY 135 

    0x4bd7a08c,// 139 PAY 136 

    0x8dbe705e,// 140 PAY 137 

    0x018770e9,// 141 PAY 138 

    0x182a4aca,// 142 PAY 139 

    0x7250b344,// 143 PAY 140 

    0x41250cd3,// 144 PAY 141 

    0xc15dfd5f,// 145 PAY 142 

    0xa097f093,// 146 PAY 143 

    0xed43cc1c,// 147 PAY 144 

    0xcc5d942e,// 148 PAY 145 

    0xc5f41572,// 149 PAY 146 

    0x3d2198de,// 150 PAY 147 

    0x1c915083,// 151 PAY 148 

    0xcbf64d70,// 152 PAY 149 

    0x72ff52ae,// 153 PAY 150 

    0xf836a079,// 154 PAY 151 

    0xba1ae169,// 155 PAY 152 

    0xbecff53a,// 156 PAY 153 

    0x1c0ac0a9,// 157 PAY 154 

    0x99893560,// 158 PAY 155 

    0x8f00dbbc,// 159 PAY 156 

    0xd8b78fd6,// 160 PAY 157 

    0x4f1d54cf,// 161 PAY 158 

    0xfc505140,// 162 PAY 159 

    0xa5e207c0,// 163 PAY 160 

    0xb4458c08,// 164 PAY 161 

    0x611fdc72,// 165 PAY 162 

    0x948bed07,// 166 PAY 163 

    0x23376166,// 167 PAY 164 

    0x350b8d19,// 168 PAY 165 

    0xc61606d7,// 169 PAY 166 

    0x2eb0de28,// 170 PAY 167 

    0x50fada5d,// 171 PAY 168 

    0x349cbacc,// 172 PAY 169 

    0x9f9f6be0,// 173 PAY 170 

    0x15e97f22,// 174 PAY 171 

    0xc12daaab,// 175 PAY 172 

    0x9ad98026,// 176 PAY 173 

    0x42dde8db,// 177 PAY 174 

    0x0f459c16,// 178 PAY 175 

    0xd0949fed,// 179 PAY 176 

    0xe1a76db7,// 180 PAY 177 

    0xff653226,// 181 PAY 178 

    0x24b5e876,// 182 PAY 179 

    0xefd4fb1f,// 183 PAY 180 

    0x359db28f,// 184 PAY 181 

    0x1b50e11d,// 185 PAY 182 

    0xd446c5ef,// 186 PAY 183 

    0x59c5563e,// 187 PAY 184 

    0xebbe29f1,// 188 PAY 185 

    0xb783fd52,// 189 PAY 186 

    0x3575ae0d,// 190 PAY 187 

    0xd0ea5cfd,// 191 PAY 188 

    0xc99a3700,// 192 PAY 189 

    0x963b6299,// 193 PAY 190 

    0x713762a0,// 194 PAY 191 

    0x06340789,// 195 PAY 192 

    0x11336408,// 196 PAY 193 

    0x6386b9fb,// 197 PAY 194 

    0x95824f33,// 198 PAY 195 

    0x1aa66f54,// 199 PAY 196 

    0x79a71246,// 200 PAY 197 

    0x370595ed,// 201 PAY 198 

    0x273cf62a,// 202 PAY 199 

    0xf877a13c,// 203 PAY 200 

    0xbccedc51,// 204 PAY 201 

    0xf9a3ff78,// 205 PAY 202 

    0x3d7f746c,// 206 PAY 203 

    0x4d201146,// 207 PAY 204 

    0x696f0ac4,// 208 PAY 205 

    0x2713dc44,// 209 PAY 206 

    0x7f65d29e,// 210 PAY 207 

    0xb80b7bfa,// 211 PAY 208 

    0x9f5fc719,// 212 PAY 209 

    0x5e9885c2,// 213 PAY 210 

    0x2de9b3b3,// 214 PAY 211 

    0x9c6805f3,// 215 PAY 212 

    0x8fffbeee,// 216 PAY 213 

    0x53baa216,// 217 PAY 214 

    0xc3e49e4c,// 218 PAY 215 

    0x31b1e416,// 219 PAY 216 

    0x47e4215c,// 220 PAY 217 

    0xed6fe4ff,// 221 PAY 218 

    0xc8ebe382,// 222 PAY 219 

    0x00bbc057,// 223 PAY 220 

    0x7aa2bb4a,// 224 PAY 221 

    0x2808cd06,// 225 PAY 222 

    0xafcb3514,// 226 PAY 223 

    0xa0bb5bb3,// 227 PAY 224 

    0xba0fc9c5,// 228 PAY 225 

    0xe9046977,// 229 PAY 226 

    0xa756e8d4,// 230 PAY 227 

    0x8b61b408,// 231 PAY 228 

    0x29650b01,// 232 PAY 229 

    0x1084c18d,// 233 PAY 230 

    0x73b55de8,// 234 PAY 231 

    0x6c1d0a50,// 235 PAY 232 

    0x8f99d2d8,// 236 PAY 233 

    0x1c644da9,// 237 PAY 234 

    0x64ab0900,// 238 PAY 235 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe5 

    0x01e8e580 // 239 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt29_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 355 words. 

/// BDA size     is 1415 (0x587) 

/// BDA id       is 0x31c5 

    0x058731c5,// 3 BDA   1 

/// PAY Generic Data size   : 1415 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x7f7cac8b,// 4 PAY   1 

    0x9c72253c,// 5 PAY   2 

    0xe1c5b1b3,// 6 PAY   3 

    0x3eeca832,// 7 PAY   4 

    0xd9b6052a,// 8 PAY   5 

    0x686709a7,// 9 PAY   6 

    0xd3608b6f,// 10 PAY   7 

    0xfffbb6f0,// 11 PAY   8 

    0x6571bc8d,// 12 PAY   9 

    0x5e357df0,// 13 PAY  10 

    0x759a57f4,// 14 PAY  11 

    0x1d18a297,// 15 PAY  12 

    0xf3aea47f,// 16 PAY  13 

    0xabaf006d,// 17 PAY  14 

    0x0d4f1c68,// 18 PAY  15 

    0x6e60d984,// 19 PAY  16 

    0xdf4139b4,// 20 PAY  17 

    0x215f0c2a,// 21 PAY  18 

    0x3f128837,// 22 PAY  19 

    0xd7bf3830,// 23 PAY  20 

    0x918ac106,// 24 PAY  21 

    0x23e41595,// 25 PAY  22 

    0xd65601fb,// 26 PAY  23 

    0x21c2743c,// 27 PAY  24 

    0x91372caf,// 28 PAY  25 

    0x3d58574f,// 29 PAY  26 

    0xeb6f9d15,// 30 PAY  27 

    0xae5c513f,// 31 PAY  28 

    0x069edebe,// 32 PAY  29 

    0x20601538,// 33 PAY  30 

    0xea7a2f70,// 34 PAY  31 

    0xdd96bf5e,// 35 PAY  32 

    0xf67eb667,// 36 PAY  33 

    0x3c3a1daa,// 37 PAY  34 

    0xaf859d40,// 38 PAY  35 

    0x4f00dd1f,// 39 PAY  36 

    0xd1d0e712,// 40 PAY  37 

    0x4535aea6,// 41 PAY  38 

    0xd49111ce,// 42 PAY  39 

    0x00cfdb89,// 43 PAY  40 

    0x93a27741,// 44 PAY  41 

    0xd82d2464,// 45 PAY  42 

    0xba3b5c1c,// 46 PAY  43 

    0x1b35fcf0,// 47 PAY  44 

    0x3554f991,// 48 PAY  45 

    0x1c337275,// 49 PAY  46 

    0x609b12c4,// 50 PAY  47 

    0x5394dabe,// 51 PAY  48 

    0x5a77bb68,// 52 PAY  49 

    0xe1b125e8,// 53 PAY  50 

    0xca2ff296,// 54 PAY  51 

    0x9c984c61,// 55 PAY  52 

    0x7e896a54,// 56 PAY  53 

    0x53dd02d0,// 57 PAY  54 

    0x7eadb562,// 58 PAY  55 

    0x8fbd92e2,// 59 PAY  56 

    0x334ce669,// 60 PAY  57 

    0xb0f45eb0,// 61 PAY  58 

    0x23ec22ba,// 62 PAY  59 

    0xaac8a4b2,// 63 PAY  60 

    0x4bf1fa9b,// 64 PAY  61 

    0x07bbecb4,// 65 PAY  62 

    0xe71d5e67,// 66 PAY  63 

    0x7f9ae1a8,// 67 PAY  64 

    0xaaf1663d,// 68 PAY  65 

    0x7334742e,// 69 PAY  66 

    0x896a27c5,// 70 PAY  67 

    0x961ff471,// 71 PAY  68 

    0xeb172cb5,// 72 PAY  69 

    0x464e7ab1,// 73 PAY  70 

    0x46254c0b,// 74 PAY  71 

    0xe7167be1,// 75 PAY  72 

    0xca492531,// 76 PAY  73 

    0x09985603,// 77 PAY  74 

    0x7fc0507b,// 78 PAY  75 

    0x0c3de502,// 79 PAY  76 

    0x84fc822c,// 80 PAY  77 

    0xec87d0c0,// 81 PAY  78 

    0xa81cff77,// 82 PAY  79 

    0xbff8fa17,// 83 PAY  80 

    0xc5a9c3f0,// 84 PAY  81 

    0x8efc5f5d,// 85 PAY  82 

    0x630f2c00,// 86 PAY  83 

    0x1bf8afac,// 87 PAY  84 

    0x37e72f5e,// 88 PAY  85 

    0x5c92f060,// 89 PAY  86 

    0xc262ce29,// 90 PAY  87 

    0x9c856ade,// 91 PAY  88 

    0x751b00d7,// 92 PAY  89 

    0x81fa0c2d,// 93 PAY  90 

    0x2252387d,// 94 PAY  91 

    0x96953931,// 95 PAY  92 

    0x1f077641,// 96 PAY  93 

    0xcca1c566,// 97 PAY  94 

    0xb1f3b200,// 98 PAY  95 

    0x035f6c3f,// 99 PAY  96 

    0x40ae7d8e,// 100 PAY  97 

    0x5a288352,// 101 PAY  98 

    0xa72a687e,// 102 PAY  99 

    0x0448c21d,// 103 PAY 100 

    0xbb6f5487,// 104 PAY 101 

    0xb8f252df,// 105 PAY 102 

    0x74ccb5f0,// 106 PAY 103 

    0x1aece0ce,// 107 PAY 104 

    0x910edfea,// 108 PAY 105 

    0x7189e639,// 109 PAY 106 

    0xfd8e718a,// 110 PAY 107 

    0x68b29105,// 111 PAY 108 

    0x1f4be6b8,// 112 PAY 109 

    0x6e510058,// 113 PAY 110 

    0xee5a9d7f,// 114 PAY 111 

    0x5b3c13e7,// 115 PAY 112 

    0xac4ae5d3,// 116 PAY 113 

    0x0e998aa2,// 117 PAY 114 

    0x8bc26b4c,// 118 PAY 115 

    0x89273a69,// 119 PAY 116 

    0x7d779966,// 120 PAY 117 

    0xccb59f31,// 121 PAY 118 

    0x2f8fee43,// 122 PAY 119 

    0xe1416717,// 123 PAY 120 

    0x4c593f9f,// 124 PAY 121 

    0x6388fd72,// 125 PAY 122 

    0x51882379,// 126 PAY 123 

    0xb31a8e39,// 127 PAY 124 

    0x210a39dd,// 128 PAY 125 

    0x567df4b3,// 129 PAY 126 

    0xa723a060,// 130 PAY 127 

    0x682bae00,// 131 PAY 128 

    0x104eac15,// 132 PAY 129 

    0xb7fe6b27,// 133 PAY 130 

    0x017cfb34,// 134 PAY 131 

    0x4e1b6328,// 135 PAY 132 

    0x7f0ccc04,// 136 PAY 133 

    0x88daa6e3,// 137 PAY 134 

    0x6d59f5e7,// 138 PAY 135 

    0xf9e8d79d,// 139 PAY 136 

    0x0431fff6,// 140 PAY 137 

    0xeb883f40,// 141 PAY 138 

    0x5d090735,// 142 PAY 139 

    0xfa8d49d8,// 143 PAY 140 

    0x74672930,// 144 PAY 141 

    0xd8d99a54,// 145 PAY 142 

    0xba66c8fc,// 146 PAY 143 

    0x5a4767a0,// 147 PAY 144 

    0x6b2451bf,// 148 PAY 145 

    0x583abf34,// 149 PAY 146 

    0x5b27f07e,// 150 PAY 147 

    0x4a348499,// 151 PAY 148 

    0x8a76c64b,// 152 PAY 149 

    0xc848e899,// 153 PAY 150 

    0x1442dba6,// 154 PAY 151 

    0xbf383f4e,// 155 PAY 152 

    0x1805d380,// 156 PAY 153 

    0x35d3f012,// 157 PAY 154 

    0x542c818b,// 158 PAY 155 

    0xa1be5ed3,// 159 PAY 156 

    0x8dd7769b,// 160 PAY 157 

    0x762f676e,// 161 PAY 158 

    0x6c3a8867,// 162 PAY 159 

    0xc7cab5b6,// 163 PAY 160 

    0xfd387165,// 164 PAY 161 

    0x78814fb2,// 165 PAY 162 

    0x0a3ad9a0,// 166 PAY 163 

    0x5389794b,// 167 PAY 164 

    0xcae715cf,// 168 PAY 165 

    0x4206ffa9,// 169 PAY 166 

    0x650f4d0b,// 170 PAY 167 

    0xe14807b6,// 171 PAY 168 

    0x99add00c,// 172 PAY 169 

    0x1370f00d,// 173 PAY 170 

    0x657406f6,// 174 PAY 171 

    0x6d4d56d3,// 175 PAY 172 

    0x811d15b7,// 176 PAY 173 

    0x0bf9e5c9,// 177 PAY 174 

    0x4f762f39,// 178 PAY 175 

    0x47253758,// 179 PAY 176 

    0x10b2379d,// 180 PAY 177 

    0x2dc83fff,// 181 PAY 178 

    0x7239345e,// 182 PAY 179 

    0xd0a9f877,// 183 PAY 180 

    0x432a992a,// 184 PAY 181 

    0x9cd6a7dd,// 185 PAY 182 

    0xd5395ff3,// 186 PAY 183 

    0x49141133,// 187 PAY 184 

    0x0bf069a8,// 188 PAY 185 

    0x82fe5860,// 189 PAY 186 

    0x50c3415b,// 190 PAY 187 

    0x31081b9d,// 191 PAY 188 

    0xc60229d0,// 192 PAY 189 

    0xf74657f6,// 193 PAY 190 

    0x897922ef,// 194 PAY 191 

    0x38af1e60,// 195 PAY 192 

    0x6a2593da,// 196 PAY 193 

    0xc17db52a,// 197 PAY 194 

    0x6d1b3364,// 198 PAY 195 

    0x246ac585,// 199 PAY 196 

    0x09b21465,// 200 PAY 197 

    0x4a4e177f,// 201 PAY 198 

    0x8dae3465,// 202 PAY 199 

    0x958ff289,// 203 PAY 200 

    0x547358fe,// 204 PAY 201 

    0x09d8aad5,// 205 PAY 202 

    0x28a25697,// 206 PAY 203 

    0x18ff95e0,// 207 PAY 204 

    0xf9c12193,// 208 PAY 205 

    0xa6513a8c,// 209 PAY 206 

    0x8c215bfb,// 210 PAY 207 

    0xdee9106d,// 211 PAY 208 

    0x5122db8f,// 212 PAY 209 

    0x41a38880,// 213 PAY 210 

    0x96c2c08c,// 214 PAY 211 

    0x9a130361,// 215 PAY 212 

    0x70da903f,// 216 PAY 213 

    0x9f78cf48,// 217 PAY 214 

    0x6865e75a,// 218 PAY 215 

    0xdb27ddc7,// 219 PAY 216 

    0x9cf18611,// 220 PAY 217 

    0xcdfb8bad,// 221 PAY 218 

    0x661dd859,// 222 PAY 219 

    0x36fc3421,// 223 PAY 220 

    0xcaba4698,// 224 PAY 221 

    0x4d4d32f4,// 225 PAY 222 

    0xe58cee2c,// 226 PAY 223 

    0xa1245d21,// 227 PAY 224 

    0x12a6db22,// 228 PAY 225 

    0xd3515355,// 229 PAY 226 

    0xc565a8a6,// 230 PAY 227 

    0x83790d20,// 231 PAY 228 

    0xaa784fbe,// 232 PAY 229 

    0x4e042d6e,// 233 PAY 230 

    0x39826af5,// 234 PAY 231 

    0x9c3d85f1,// 235 PAY 232 

    0x1e4f9265,// 236 PAY 233 

    0x987f8158,// 237 PAY 234 

    0x1ed46684,// 238 PAY 235 

    0x22aff49e,// 239 PAY 236 

    0xf77b90d7,// 240 PAY 237 

    0x031968d9,// 241 PAY 238 

    0x311edd04,// 242 PAY 239 

    0x2e7a4a40,// 243 PAY 240 

    0x463ecece,// 244 PAY 241 

    0xf41032da,// 245 PAY 242 

    0x1635ecf2,// 246 PAY 243 

    0x4b889296,// 247 PAY 244 

    0x9d23944f,// 248 PAY 245 

    0x42805ddd,// 249 PAY 246 

    0x02cbce32,// 250 PAY 247 

    0x3b4de8c5,// 251 PAY 248 

    0xbfe15359,// 252 PAY 249 

    0x677f5319,// 253 PAY 250 

    0x71272ff8,// 254 PAY 251 

    0x88b2d154,// 255 PAY 252 

    0x9f0130bc,// 256 PAY 253 

    0x7bcc7d06,// 257 PAY 254 

    0xee1434b5,// 258 PAY 255 

    0x12cd0abd,// 259 PAY 256 

    0x4975fcc2,// 260 PAY 257 

    0xb9ab1582,// 261 PAY 258 

    0x1e90225b,// 262 PAY 259 

    0x1f400f70,// 263 PAY 260 

    0xc6893a07,// 264 PAY 261 

    0x3f314454,// 265 PAY 262 

    0xca4045fc,// 266 PAY 263 

    0xd56674c9,// 267 PAY 264 

    0xa789b3a3,// 268 PAY 265 

    0x7f4f3d1c,// 269 PAY 266 

    0x9e10bb0d,// 270 PAY 267 

    0x70ac7c14,// 271 PAY 268 

    0x8efef134,// 272 PAY 269 

    0x2ab19480,// 273 PAY 270 

    0x954eed78,// 274 PAY 271 

    0xed8a0244,// 275 PAY 272 

    0xa30a2e68,// 276 PAY 273 

    0xf5c8b771,// 277 PAY 274 

    0xf6f0cec2,// 278 PAY 275 

    0xd3ef512b,// 279 PAY 276 

    0xa69f547d,// 280 PAY 277 

    0x0ffd8b4e,// 281 PAY 278 

    0x6d782326,// 282 PAY 279 

    0x10d34c02,// 283 PAY 280 

    0x601f684c,// 284 PAY 281 

    0xa8fdeba7,// 285 PAY 282 

    0x7d82fc7a,// 286 PAY 283 

    0x20d08fb4,// 287 PAY 284 

    0x2925892c,// 288 PAY 285 

    0x6e9e13a4,// 289 PAY 286 

    0x549ab820,// 290 PAY 287 

    0x63c6fec9,// 291 PAY 288 

    0x43707279,// 292 PAY 289 

    0xbf7a3c55,// 293 PAY 290 

    0x30696545,// 294 PAY 291 

    0xdc215c81,// 295 PAY 292 

    0x61ed6494,// 296 PAY 293 

    0x4e062b47,// 297 PAY 294 

    0x0b6d1f5f,// 298 PAY 295 

    0x657715d3,// 299 PAY 296 

    0xa2373b06,// 300 PAY 297 

    0xb183ef53,// 301 PAY 298 

    0x3c4e5545,// 302 PAY 299 

    0xbe2f2fd7,// 303 PAY 300 

    0xcd0f28ba,// 304 PAY 301 

    0x0ede6b6a,// 305 PAY 302 

    0x10fbddb8,// 306 PAY 303 

    0xc9391b86,// 307 PAY 304 

    0x97d6d358,// 308 PAY 305 

    0xe25449d8,// 309 PAY 306 

    0x1dff35db,// 310 PAY 307 

    0xcef21b91,// 311 PAY 308 

    0xcb502799,// 312 PAY 309 

    0xeb09a772,// 313 PAY 310 

    0xd28bdac0,// 314 PAY 311 

    0x1b35a946,// 315 PAY 312 

    0x6b5fb813,// 316 PAY 313 

    0x40233fd3,// 317 PAY 314 

    0x69f71886,// 318 PAY 315 

    0x54d8a8bd,// 319 PAY 316 

    0x8fab434e,// 320 PAY 317 

    0x0bb355be,// 321 PAY 318 

    0xa09dd0bb,// 322 PAY 319 

    0x30458666,// 323 PAY 320 

    0x489f19c4,// 324 PAY 321 

    0x24f656f0,// 325 PAY 322 

    0xcf3bc443,// 326 PAY 323 

    0x6067a823,// 327 PAY 324 

    0x9ba9a018,// 328 PAY 325 

    0x4fb7c7ca,// 329 PAY 326 

    0xfa2806f6,// 330 PAY 327 

    0x51463dbd,// 331 PAY 328 

    0x266a290a,// 332 PAY 329 

    0xdad4e60a,// 333 PAY 330 

    0x708604a3,// 334 PAY 331 

    0x88d84a7f,// 335 PAY 332 

    0x3e5ac9a9,// 336 PAY 333 

    0x235e3d9f,// 337 PAY 334 

    0x6447663f,// 338 PAY 335 

    0xc2f87018,// 339 PAY 336 

    0xb28e39b2,// 340 PAY 337 

    0x4085fe54,// 341 PAY 338 

    0xab9ec9c1,// 342 PAY 339 

    0xe42fcfa7,// 343 PAY 340 

    0xf5f78ff4,// 344 PAY 341 

    0xf68a204f,// 345 PAY 342 

    0xc2e12ffc,// 346 PAY 343 

    0x6c6ae378,// 347 PAY 344 

    0xbc59fd91,// 348 PAY 345 

    0x94732e91,// 349 PAY 346 

    0x8d2312d4,// 350 PAY 347 

    0x071d254d,// 351 PAY 348 

    0xa1f5165f,// 352 PAY 349 

    0xda0fff3d,// 353 PAY 350 

    0x148ca7b7,// 354 PAY 351 

    0xaf6243ea,// 355 PAY 352 

    0x359f05e6,// 356 PAY 353 

    0xa7107000,// 357 PAY 354 

/// HASH is  8 bytes 

    0xc2e12ffc,// 358 HSH   1 

    0x6c6ae378,// 359 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 28 

/// STA pkt_idx        : 43 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x00ac6e1c // 360 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt30_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 144 words. 

/// BDA size     is 571 (0x23b) 

/// BDA id       is 0x15c 

    0x023b015c,// 3 BDA   1 

/// PAY Generic Data size   : 571 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x23a654b7,// 4 PAY   1 

    0x4fe1fa57,// 5 PAY   2 

    0x10f97228,// 6 PAY   3 

    0x5060b8c1,// 7 PAY   4 

    0xf9cd9e16,// 8 PAY   5 

    0xfc114b16,// 9 PAY   6 

    0x4f942479,// 10 PAY   7 

    0x85c76346,// 11 PAY   8 

    0xbe21d250,// 12 PAY   9 

    0x50db7306,// 13 PAY  10 

    0xcf614709,// 14 PAY  11 

    0x84998627,// 15 PAY  12 

    0xb9e0834b,// 16 PAY  13 

    0xc76509be,// 17 PAY  14 

    0x5a9b0d1c,// 18 PAY  15 

    0x369eaaa4,// 19 PAY  16 

    0xdaf6790e,// 20 PAY  17 

    0xac445c77,// 21 PAY  18 

    0xb6ba6810,// 22 PAY  19 

    0xd3926b07,// 23 PAY  20 

    0x302b3d81,// 24 PAY  21 

    0x298184ed,// 25 PAY  22 

    0x67f93e47,// 26 PAY  23 

    0x01f122b2,// 27 PAY  24 

    0x7b78c098,// 28 PAY  25 

    0x6950f19a,// 29 PAY  26 

    0x1fdd3c9e,// 30 PAY  27 

    0xcc6b88c0,// 31 PAY  28 

    0x861ccab4,// 32 PAY  29 

    0x1d00b8ce,// 33 PAY  30 

    0x4d537828,// 34 PAY  31 

    0x474d3dba,// 35 PAY  32 

    0x2055ede8,// 36 PAY  33 

    0xd8757278,// 37 PAY  34 

    0xc9b23765,// 38 PAY  35 

    0x8e870fff,// 39 PAY  36 

    0x4c38ec13,// 40 PAY  37 

    0x9d72af98,// 41 PAY  38 

    0x788bfe87,// 42 PAY  39 

    0xe10df9a8,// 43 PAY  40 

    0xd1cbc666,// 44 PAY  41 

    0xa220a256,// 45 PAY  42 

    0x77a240ff,// 46 PAY  43 

    0x3646d7cf,// 47 PAY  44 

    0x0d916670,// 48 PAY  45 

    0x7caea41e,// 49 PAY  46 

    0x8c0253c1,// 50 PAY  47 

    0xbde8eba9,// 51 PAY  48 

    0x6ea54282,// 52 PAY  49 

    0x31719bcd,// 53 PAY  50 

    0x030d0fe7,// 54 PAY  51 

    0x95b0c7a6,// 55 PAY  52 

    0x07578966,// 56 PAY  53 

    0xbeb8ae47,// 57 PAY  54 

    0xbcb5efca,// 58 PAY  55 

    0xf348c518,// 59 PAY  56 

    0x6eab1f9c,// 60 PAY  57 

    0x912e8066,// 61 PAY  58 

    0x246597cf,// 62 PAY  59 

    0xef244605,// 63 PAY  60 

    0x4baf4282,// 64 PAY  61 

    0xffcebe5f,// 65 PAY  62 

    0x89a2662a,// 66 PAY  63 

    0x70f066d9,// 67 PAY  64 

    0xbcd42656,// 68 PAY  65 

    0x95f76f9d,// 69 PAY  66 

    0x6b51a52f,// 70 PAY  67 

    0x8436f1f7,// 71 PAY  68 

    0xc5c1183a,// 72 PAY  69 

    0x88f334c0,// 73 PAY  70 

    0x6997b0b8,// 74 PAY  71 

    0x44737bba,// 75 PAY  72 

    0x54da4072,// 76 PAY  73 

    0x09cdf752,// 77 PAY  74 

    0x2a1da16d,// 78 PAY  75 

    0xcefe4e10,// 79 PAY  76 

    0x3d0ef6ee,// 80 PAY  77 

    0x836d983d,// 81 PAY  78 

    0xf71394d6,// 82 PAY  79 

    0xbb97319b,// 83 PAY  80 

    0x3401ec65,// 84 PAY  81 

    0x2c83f2a3,// 85 PAY  82 

    0xae86afc0,// 86 PAY  83 

    0x494f58cb,// 87 PAY  84 

    0x4e46ae3d,// 88 PAY  85 

    0x2bcb5b61,// 89 PAY  86 

    0xddfb7e91,// 90 PAY  87 

    0x4a8750fc,// 91 PAY  88 

    0x1b0927ab,// 92 PAY  89 

    0x413d05a5,// 93 PAY  90 

    0x692d7c13,// 94 PAY  91 

    0x28421a61,// 95 PAY  92 

    0x03f60817,// 96 PAY  93 

    0x55a094fa,// 97 PAY  94 

    0xc159a35d,// 98 PAY  95 

    0x1f468196,// 99 PAY  96 

    0xec7a55d5,// 100 PAY  97 

    0x9b4236b3,// 101 PAY  98 

    0x4cb94f02,// 102 PAY  99 

    0x22aebf68,// 103 PAY 100 

    0xdddbec70,// 104 PAY 101 

    0x957eeb0b,// 105 PAY 102 

    0xcd980a45,// 106 PAY 103 

    0x6b90d376,// 107 PAY 104 

    0xcb425232,// 108 PAY 105 

    0xab523b04,// 109 PAY 106 

    0x38ef2afb,// 110 PAY 107 

    0xa6c89718,// 111 PAY 108 

    0x5f8d49c6,// 112 PAY 109 

    0x4091aeaf,// 113 PAY 110 

    0x969a6c93,// 114 PAY 111 

    0x094ae59c,// 115 PAY 112 

    0x01fa44b0,// 116 PAY 113 

    0x6e065ef2,// 117 PAY 114 

    0x40b4dd19,// 118 PAY 115 

    0x03398b74,// 119 PAY 116 

    0x95eee99f,// 120 PAY 117 

    0xd93f9154,// 121 PAY 118 

    0x44b281ac,// 122 PAY 119 

    0x3bf552a6,// 123 PAY 120 

    0x5c707ac6,// 124 PAY 121 

    0x1799ef59,// 125 PAY 122 

    0x84d9b2b7,// 126 PAY 123 

    0xb33dca79,// 127 PAY 124 

    0x01106e4d,// 128 PAY 125 

    0x6b1c91d8,// 129 PAY 126 

    0xb6d2cb0d,// 130 PAY 127 

    0x05f6a94c,// 131 PAY 128 

    0x36301011,// 132 PAY 129 

    0x3fa25cd4,// 133 PAY 130 

    0xd1d9995a,// 134 PAY 131 

    0x383265cf,// 135 PAY 132 

    0xb745cb6a,// 136 PAY 133 

    0xbc4781e9,// 137 PAY 134 

    0xc4da061f,// 138 PAY 135 

    0x769c7142,// 139 PAY 136 

    0x0422d762,// 140 PAY 137 

    0x7ba5fa5c,// 141 PAY 138 

    0xb9024a7f,// 142 PAY 139 

    0x0b78f4c8,// 143 PAY 140 

    0x1fe4af63,// 144 PAY 141 

    0x3b81dbf0,// 145 PAY 142 

    0xf6043600,// 146 PAY 143 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 105 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x53 

    0x01a55306 // 147 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt31_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 295 words. 

/// BDA size     is 1175 (0x497) 

/// BDA id       is 0x2b67 

    0x04972b67,// 3 BDA   1 

/// PAY Generic Data size   : 1175 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xc0b9698d,// 4 PAY   1 

    0x91d3fe47,// 5 PAY   2 

    0x9dbccc99,// 6 PAY   3 

    0x345bc19a,// 7 PAY   4 

    0xefe25556,// 8 PAY   5 

    0x5aac0377,// 9 PAY   6 

    0xa4df94f1,// 10 PAY   7 

    0x99a50150,// 11 PAY   8 

    0xb93b7c4a,// 12 PAY   9 

    0xf8d430c7,// 13 PAY  10 

    0xbed7b2ab,// 14 PAY  11 

    0x513089ca,// 15 PAY  12 

    0xa542a31f,// 16 PAY  13 

    0xda2e6a87,// 17 PAY  14 

    0x85024d40,// 18 PAY  15 

    0x6644c0e3,// 19 PAY  16 

    0x71a348b0,// 20 PAY  17 

    0x9b329b00,// 21 PAY  18 

    0x979b37d6,// 22 PAY  19 

    0xb4ec20e9,// 23 PAY  20 

    0xd46a738d,// 24 PAY  21 

    0x1dac5041,// 25 PAY  22 

    0x61e7ed33,// 26 PAY  23 

    0x5f53265b,// 27 PAY  24 

    0x64ae5326,// 28 PAY  25 

    0xd1556110,// 29 PAY  26 

    0x60e74c10,// 30 PAY  27 

    0x0facacdf,// 31 PAY  28 

    0xa7b61a02,// 32 PAY  29 

    0x140df53f,// 33 PAY  30 

    0x55ba32ed,// 34 PAY  31 

    0x1b911dd9,// 35 PAY  32 

    0xd2c7bb7b,// 36 PAY  33 

    0x12793d36,// 37 PAY  34 

    0x04937993,// 38 PAY  35 

    0xee940935,// 39 PAY  36 

    0xd487b034,// 40 PAY  37 

    0xf804c2a2,// 41 PAY  38 

    0xa5fbddc1,// 42 PAY  39 

    0xa177e7ea,// 43 PAY  40 

    0xaeeec320,// 44 PAY  41 

    0xb804c3e2,// 45 PAY  42 

    0x7fef1071,// 46 PAY  43 

    0x7a2d41e4,// 47 PAY  44 

    0xfc0d21d3,// 48 PAY  45 

    0x0afed52f,// 49 PAY  46 

    0x98d8e13b,// 50 PAY  47 

    0x9fcbb082,// 51 PAY  48 

    0x7ae4168a,// 52 PAY  49 

    0xce249e5f,// 53 PAY  50 

    0xf0d8f580,// 54 PAY  51 

    0x743518e9,// 55 PAY  52 

    0x771fff12,// 56 PAY  53 

    0xb1a36e70,// 57 PAY  54 

    0x8cf39d8a,// 58 PAY  55 

    0x9a8478ed,// 59 PAY  56 

    0xe1979b72,// 60 PAY  57 

    0xfd0a721d,// 61 PAY  58 

    0xd733021c,// 62 PAY  59 

    0x823cfbb3,// 63 PAY  60 

    0x29cff339,// 64 PAY  61 

    0xe4b6817c,// 65 PAY  62 

    0x258ec3b4,// 66 PAY  63 

    0x0c3418de,// 67 PAY  64 

    0x15a4eb2d,// 68 PAY  65 

    0x85ed4d87,// 69 PAY  66 

    0xda2824a7,// 70 PAY  67 

    0xedb16750,// 71 PAY  68 

    0xd4c6bf97,// 72 PAY  69 

    0x7be8fc17,// 73 PAY  70 

    0xe9af424a,// 74 PAY  71 

    0x74227c7d,// 75 PAY  72 

    0xc794277d,// 76 PAY  73 

    0xfa3bb3f5,// 77 PAY  74 

    0xc66b78b9,// 78 PAY  75 

    0x0cb215cb,// 79 PAY  76 

    0x5cb59d70,// 80 PAY  77 

    0xd73031a9,// 81 PAY  78 

    0x28d40085,// 82 PAY  79 

    0xea5c4fb5,// 83 PAY  80 

    0x0de93448,// 84 PAY  81 

    0x3cfa7142,// 85 PAY  82 

    0x0851234e,// 86 PAY  83 

    0x6bb09621,// 87 PAY  84 

    0x44fb28ed,// 88 PAY  85 

    0xc014056a,// 89 PAY  86 

    0xbad7e500,// 90 PAY  87 

    0x7731078a,// 91 PAY  88 

    0x9963e54a,// 92 PAY  89 

    0xb13665db,// 93 PAY  90 

    0xf0aa34a9,// 94 PAY  91 

    0xad96da86,// 95 PAY  92 

    0xab8055b3,// 96 PAY  93 

    0x4b72c9c2,// 97 PAY  94 

    0x93092018,// 98 PAY  95 

    0x721a18c2,// 99 PAY  96 

    0xee55d621,// 100 PAY  97 

    0xc9fc6bf4,// 101 PAY  98 

    0x6f6fa126,// 102 PAY  99 

    0xbfeff0a7,// 103 PAY 100 

    0xfe9811a7,// 104 PAY 101 

    0xb63fcb22,// 105 PAY 102 

    0x389fbdaa,// 106 PAY 103 

    0xf5d6e886,// 107 PAY 104 

    0xc30b05a3,// 108 PAY 105 

    0x4e5ab0e9,// 109 PAY 106 

    0x9f54d24d,// 110 PAY 107 

    0x38fae6c1,// 111 PAY 108 

    0x67cb9d76,// 112 PAY 109 

    0xc78e5169,// 113 PAY 110 

    0x88f74f7e,// 114 PAY 111 

    0xcbcd211b,// 115 PAY 112 

    0xe02c5729,// 116 PAY 113 

    0xd7572720,// 117 PAY 114 

    0xd785241e,// 118 PAY 115 

    0x3617f120,// 119 PAY 116 

    0x7f931991,// 120 PAY 117 

    0x0facf5eb,// 121 PAY 118 

    0x08ec273d,// 122 PAY 119 

    0x39f0574a,// 123 PAY 120 

    0xaed03c4a,// 124 PAY 121 

    0x386218fd,// 125 PAY 122 

    0x7da04401,// 126 PAY 123 

    0xdb25b1f1,// 127 PAY 124 

    0xefb689a9,// 128 PAY 125 

    0xdb5e3fb7,// 129 PAY 126 

    0xc15d08c7,// 130 PAY 127 

    0x8dd20ce4,// 131 PAY 128 

    0x1ba21329,// 132 PAY 129 

    0x613b247a,// 133 PAY 130 

    0x3f45b009,// 134 PAY 131 

    0xd7f20566,// 135 PAY 132 

    0x54e4b6ff,// 136 PAY 133 

    0x038ef826,// 137 PAY 134 

    0xf0735369,// 138 PAY 135 

    0x176eb81a,// 139 PAY 136 

    0x81bcf51d,// 140 PAY 137 

    0x09dd71e2,// 141 PAY 138 

    0x3ca34708,// 142 PAY 139 

    0x9ed60315,// 143 PAY 140 

    0xf9ddaa4e,// 144 PAY 141 

    0xe8548d3a,// 145 PAY 142 

    0x1d33acfe,// 146 PAY 143 

    0xe9ce01b5,// 147 PAY 144 

    0xb84189e5,// 148 PAY 145 

    0x85391d98,// 149 PAY 146 

    0x6cece17d,// 150 PAY 147 

    0x761939b0,// 151 PAY 148 

    0x40ad74dc,// 152 PAY 149 

    0x2933a88e,// 153 PAY 150 

    0x774deb6d,// 154 PAY 151 

    0x469e8370,// 155 PAY 152 

    0x34ef951d,// 156 PAY 153 

    0xb558b748,// 157 PAY 154 

    0x3f5f046a,// 158 PAY 155 

    0xbba2bb1a,// 159 PAY 156 

    0x4c2001d1,// 160 PAY 157 

    0xdb328285,// 161 PAY 158 

    0xc5d73b1f,// 162 PAY 159 

    0x4cf240fe,// 163 PAY 160 

    0x26b90fa9,// 164 PAY 161 

    0x52113337,// 165 PAY 162 

    0x8c03c598,// 166 PAY 163 

    0x531101c1,// 167 PAY 164 

    0xf2031bc5,// 168 PAY 165 

    0x869a3902,// 169 PAY 166 

    0xe8e1de58,// 170 PAY 167 

    0xe18f674b,// 171 PAY 168 

    0xc1ad7739,// 172 PAY 169 

    0x279812a1,// 173 PAY 170 

    0xba281bd6,// 174 PAY 171 

    0xbfa371c2,// 175 PAY 172 

    0x0d5c1b53,// 176 PAY 173 

    0xa5f5c98f,// 177 PAY 174 

    0x5e47e5a3,// 178 PAY 175 

    0x5761f8be,// 179 PAY 176 

    0x6a5e50bc,// 180 PAY 177 

    0xe4ef8b50,// 181 PAY 178 

    0x9a01d152,// 182 PAY 179 

    0x88a615f1,// 183 PAY 180 

    0xe268227f,// 184 PAY 181 

    0x64394bf4,// 185 PAY 182 

    0x99d194d8,// 186 PAY 183 

    0x058533bc,// 187 PAY 184 

    0x1c4228b3,// 188 PAY 185 

    0xcf098f46,// 189 PAY 186 

    0x5ea49c24,// 190 PAY 187 

    0x33b2283c,// 191 PAY 188 

    0xffbae27a,// 192 PAY 189 

    0xd7884843,// 193 PAY 190 

    0x469a0c97,// 194 PAY 191 

    0xe396a31e,// 195 PAY 192 

    0x494df5a0,// 196 PAY 193 

    0xf7659839,// 197 PAY 194 

    0x1833c568,// 198 PAY 195 

    0x82f93d36,// 199 PAY 196 

    0x1e500d7d,// 200 PAY 197 

    0xa1774794,// 201 PAY 198 

    0x8f1d65df,// 202 PAY 199 

    0x07c590f6,// 203 PAY 200 

    0x9fbc1004,// 204 PAY 201 

    0xc121c18e,// 205 PAY 202 

    0xf45e503c,// 206 PAY 203 

    0x0cd326cf,// 207 PAY 204 

    0x12a3f678,// 208 PAY 205 

    0x906e7296,// 209 PAY 206 

    0x568588b1,// 210 PAY 207 

    0x506016bc,// 211 PAY 208 

    0xab794ef5,// 212 PAY 209 

    0xcc97b91a,// 213 PAY 210 

    0xea3a10b4,// 214 PAY 211 

    0xf4e2dae3,// 215 PAY 212 

    0x16d39aab,// 216 PAY 213 

    0x33497a03,// 217 PAY 214 

    0xf84cdfb4,// 218 PAY 215 

    0xf759353f,// 219 PAY 216 

    0x968d5c3e,// 220 PAY 217 

    0xcf3fae3a,// 221 PAY 218 

    0x35edc071,// 222 PAY 219 

    0x0765cf38,// 223 PAY 220 

    0xc31e3ab5,// 224 PAY 221 

    0xdbc23bf4,// 225 PAY 222 

    0x10423526,// 226 PAY 223 

    0x53b461cd,// 227 PAY 224 

    0xdfe41662,// 228 PAY 225 

    0xa8e2b450,// 229 PAY 226 

    0xef102264,// 230 PAY 227 

    0x1b2247b8,// 231 PAY 228 

    0x94706fe5,// 232 PAY 229 

    0x264f406c,// 233 PAY 230 

    0xb73f9318,// 234 PAY 231 

    0xf16acd04,// 235 PAY 232 

    0x8bff9605,// 236 PAY 233 

    0x93cdf187,// 237 PAY 234 

    0x6408039b,// 238 PAY 235 

    0xfba46069,// 239 PAY 236 

    0x6ffb8c23,// 240 PAY 237 

    0xe2cf1ff5,// 241 PAY 238 

    0xd0e8f5fa,// 242 PAY 239 

    0xef3f52a5,// 243 PAY 240 

    0x010de071,// 244 PAY 241 

    0xb426ac9d,// 245 PAY 242 

    0x773f44ab,// 246 PAY 243 

    0x53eb5b6f,// 247 PAY 244 

    0x10039187,// 248 PAY 245 

    0x99afe9d7,// 249 PAY 246 

    0x83cf8cd1,// 250 PAY 247 

    0x2ff4c20b,// 251 PAY 248 

    0x1b46ce4b,// 252 PAY 249 

    0x55b193ba,// 253 PAY 250 

    0xc5b62717,// 254 PAY 251 

    0xf74749e5,// 255 PAY 252 

    0x63d25e51,// 256 PAY 253 

    0x5cb7916a,// 257 PAY 254 

    0x014b36a6,// 258 PAY 255 

    0xde041fd8,// 259 PAY 256 

    0x6445d734,// 260 PAY 257 

    0x905de24b,// 261 PAY 258 

    0xe87f9af9,// 262 PAY 259 

    0x54bdab44,// 263 PAY 260 

    0xe5ee39e7,// 264 PAY 261 

    0xcf709848,// 265 PAY 262 

    0x4b01555b,// 266 PAY 263 

    0xfac353e3,// 267 PAY 264 

    0xf2d089fa,// 268 PAY 265 

    0x40aeb3b4,// 269 PAY 266 

    0x6c8016e8,// 270 PAY 267 

    0xc04b74ec,// 271 PAY 268 

    0x6712bcac,// 272 PAY 269 

    0x989278e3,// 273 PAY 270 

    0x93da1293,// 274 PAY 271 

    0x94b9573c,// 275 PAY 272 

    0x138ebc7d,// 276 PAY 273 

    0xf61b4bce,// 277 PAY 274 

    0x853c23cf,// 278 PAY 275 

    0x6affcc29,// 279 PAY 276 

    0xb8cbdab4,// 280 PAY 277 

    0x08ab97c3,// 281 PAY 278 

    0x15ee197b,// 282 PAY 279 

    0x554b3358,// 283 PAY 280 

    0x091164ef,// 284 PAY 281 

    0x0953d51e,// 285 PAY 282 

    0x154732e6,// 286 PAY 283 

    0xbab957a5,// 287 PAY 284 

    0x3b02de3b,// 288 PAY 285 

    0xb34a1b6a,// 289 PAY 286 

    0x9fd800b5,// 290 PAY 287 

    0xca315db8,// 291 PAY 288 

    0xf3f9c461,// 292 PAY 289 

    0x2c0d6593,// 293 PAY 290 

    0x79c92fcc,// 294 PAY 291 

    0xe736dcf7,// 295 PAY 292 

    0x13033f50,// 296 PAY 293 

    0x32e5ba00,// 297 PAY 294 

/// HASH is  4 bytes 

    0xf3f9c461,// 298 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 179 

/// STA pkt_idx        : 154 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb8 

    0x0269b8b3 // 299 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt32_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 431 words. 

/// BDA size     is 1717 (0x6b5) 

/// BDA id       is 0x1659 

    0x06b51659,// 3 BDA   1 

/// PAY Generic Data size   : 1717 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xcb708fb6,// 4 PAY   1 

    0xcc3f97cd,// 5 PAY   2 

    0x87769167,// 6 PAY   3 

    0x6797b0a7,// 7 PAY   4 

    0xb15b5efd,// 8 PAY   5 

    0x490eabe7,// 9 PAY   6 

    0xbdf0e28f,// 10 PAY   7 

    0x93482352,// 11 PAY   8 

    0x87aed7bd,// 12 PAY   9 

    0x47ab4638,// 13 PAY  10 

    0xceb01282,// 14 PAY  11 

    0x78b4e3f9,// 15 PAY  12 

    0x048d4678,// 16 PAY  13 

    0x9858ae62,// 17 PAY  14 

    0xf628e0c4,// 18 PAY  15 

    0xda653e77,// 19 PAY  16 

    0x7ad419c0,// 20 PAY  17 

    0xf9450080,// 21 PAY  18 

    0xa70f76de,// 22 PAY  19 

    0x0814d024,// 23 PAY  20 

    0x77ce2440,// 24 PAY  21 

    0x4dd80348,// 25 PAY  22 

    0xb3615fce,// 26 PAY  23 

    0xa812ed65,// 27 PAY  24 

    0x708cd149,// 28 PAY  25 

    0xcd205567,// 29 PAY  26 

    0x9bcf9443,// 30 PAY  27 

    0xb042395b,// 31 PAY  28 

    0xe7b72cff,// 32 PAY  29 

    0x4347b12f,// 33 PAY  30 

    0x44131d62,// 34 PAY  31 

    0x391b023c,// 35 PAY  32 

    0x884b452d,// 36 PAY  33 

    0x0a640e09,// 37 PAY  34 

    0x985f0fbd,// 38 PAY  35 

    0x6c0343dc,// 39 PAY  36 

    0x6a9d170e,// 40 PAY  37 

    0xa0bb88d3,// 41 PAY  38 

    0x566a0bfb,// 42 PAY  39 

    0x1eb398eb,// 43 PAY  40 

    0x758be03f,// 44 PAY  41 

    0xaa3ecec4,// 45 PAY  42 

    0x3488bc9e,// 46 PAY  43 

    0x8de0e85b,// 47 PAY  44 

    0xbfb71c72,// 48 PAY  45 

    0xb4121330,// 49 PAY  46 

    0x40c47c66,// 50 PAY  47 

    0xf6427592,// 51 PAY  48 

    0x8b35b8fa,// 52 PAY  49 

    0x3d426d15,// 53 PAY  50 

    0xc4e46513,// 54 PAY  51 

    0x6da3342e,// 55 PAY  52 

    0x0380c97c,// 56 PAY  53 

    0x56ec9841,// 57 PAY  54 

    0x24357a77,// 58 PAY  55 

    0xc47722ca,// 59 PAY  56 

    0x98f8731d,// 60 PAY  57 

    0x080eb503,// 61 PAY  58 

    0xfd55f456,// 62 PAY  59 

    0x3b6c5a76,// 63 PAY  60 

    0x59c1a2fc,// 64 PAY  61 

    0xa8f90ffd,// 65 PAY  62 

    0xe924390c,// 66 PAY  63 

    0x755edd8d,// 67 PAY  64 

    0x0946f8fd,// 68 PAY  65 

    0xf42586e1,// 69 PAY  66 

    0x276e9621,// 70 PAY  67 

    0xc6e37a62,// 71 PAY  68 

    0xdc66a414,// 72 PAY  69 

    0x0d0e1348,// 73 PAY  70 

    0xe101b6fc,// 74 PAY  71 

    0xe2d0b0fc,// 75 PAY  72 

    0xfc29ef1a,// 76 PAY  73 

    0xa3218286,// 77 PAY  74 

    0xc34c76af,// 78 PAY  75 

    0x81674056,// 79 PAY  76 

    0xf1f83950,// 80 PAY  77 

    0xe8490a03,// 81 PAY  78 

    0xec7f089c,// 82 PAY  79 

    0xaba41b29,// 83 PAY  80 

    0x4921e315,// 84 PAY  81 

    0xa4851790,// 85 PAY  82 

    0x527337cc,// 86 PAY  83 

    0x4da83836,// 87 PAY  84 

    0xf389b714,// 88 PAY  85 

    0x670044e7,// 89 PAY  86 

    0xafc85df1,// 90 PAY  87 

    0x6b6f1566,// 91 PAY  88 

    0xe1560309,// 92 PAY  89 

    0xf79ef567,// 93 PAY  90 

    0xcefddcee,// 94 PAY  91 

    0x7c8c3189,// 95 PAY  92 

    0xfef402fa,// 96 PAY  93 

    0xf92b91a6,// 97 PAY  94 

    0xd60010df,// 98 PAY  95 

    0xed4e230c,// 99 PAY  96 

    0xdee30f98,// 100 PAY  97 

    0x86d49486,// 101 PAY  98 

    0xbdcae232,// 102 PAY  99 

    0xbc77608f,// 103 PAY 100 

    0x58d52ab9,// 104 PAY 101 

    0xd7370ec7,// 105 PAY 102 

    0xcbc5053e,// 106 PAY 103 

    0xe1802f07,// 107 PAY 104 

    0x496bc812,// 108 PAY 105 

    0x148de4dd,// 109 PAY 106 

    0xbf7e795b,// 110 PAY 107 

    0xa4d2753a,// 111 PAY 108 

    0x2ca5c862,// 112 PAY 109 

    0xb71c8fb0,// 113 PAY 110 

    0xf614558a,// 114 PAY 111 

    0xaf9a73c4,// 115 PAY 112 

    0x1ff36370,// 116 PAY 113 

    0xfe7de949,// 117 PAY 114 

    0xef967870,// 118 PAY 115 

    0x5d8293e5,// 119 PAY 116 

    0xdaa4d0ed,// 120 PAY 117 

    0xa3ede4ed,// 121 PAY 118 

    0x34d6ce94,// 122 PAY 119 

    0xc0445f55,// 123 PAY 120 

    0x67db0668,// 124 PAY 121 

    0xf1624832,// 125 PAY 122 

    0x501c5058,// 126 PAY 123 

    0x39c5f79c,// 127 PAY 124 

    0x1109e017,// 128 PAY 125 

    0x5a1ec6ee,// 129 PAY 126 

    0x369d95ad,// 130 PAY 127 

    0x2d49be40,// 131 PAY 128 

    0x45494d1e,// 132 PAY 129 

    0x839a2ebe,// 133 PAY 130 

    0x5aea56e7,// 134 PAY 131 

    0x04ff5791,// 135 PAY 132 

    0xdb54db4a,// 136 PAY 133 

    0x84683829,// 137 PAY 134 

    0xc2bf8539,// 138 PAY 135 

    0x76a056e7,// 139 PAY 136 

    0xa46686dc,// 140 PAY 137 

    0x4fdf6530,// 141 PAY 138 

    0x669d73ff,// 142 PAY 139 

    0x6d8611c2,// 143 PAY 140 

    0xe0202df8,// 144 PAY 141 

    0xedb83a2a,// 145 PAY 142 

    0xab2ed3ab,// 146 PAY 143 

    0x356bfc49,// 147 PAY 144 

    0xe3e71992,// 148 PAY 145 

    0xfa4b0aee,// 149 PAY 146 

    0x296b43a0,// 150 PAY 147 

    0x3ed82008,// 151 PAY 148 

    0xad64e42f,// 152 PAY 149 

    0xde826f67,// 153 PAY 150 

    0x89feeb58,// 154 PAY 151 

    0x82ec627a,// 155 PAY 152 

    0x03af211c,// 156 PAY 153 

    0x0d992ce1,// 157 PAY 154 

    0xe3a903f2,// 158 PAY 155 

    0xcd9ce720,// 159 PAY 156 

    0x5e642533,// 160 PAY 157 

    0x6ac930dc,// 161 PAY 158 

    0x372c5833,// 162 PAY 159 

    0xe5f7e2c4,// 163 PAY 160 

    0x78e662f3,// 164 PAY 161 

    0xb20ac4bb,// 165 PAY 162 

    0x28b671d0,// 166 PAY 163 

    0x9764febd,// 167 PAY 164 

    0xab4e3a7e,// 168 PAY 165 

    0xbe2309a4,// 169 PAY 166 

    0x9319950d,// 170 PAY 167 

    0x9067a976,// 171 PAY 168 

    0xe332ed3f,// 172 PAY 169 

    0x00925daa,// 173 PAY 170 

    0x3500e2d8,// 174 PAY 171 

    0x4d4b4bb4,// 175 PAY 172 

    0x7bca1e2e,// 176 PAY 173 

    0xf81317f1,// 177 PAY 174 

    0x228a9dc0,// 178 PAY 175 

    0x2f2e290f,// 179 PAY 176 

    0x0c3f0465,// 180 PAY 177 

    0x9c816185,// 181 PAY 178 

    0x76ef3291,// 182 PAY 179 

    0xcb524dcc,// 183 PAY 180 

    0x58a94e24,// 184 PAY 181 

    0xcbf6cf59,// 185 PAY 182 

    0x2e44ebf8,// 186 PAY 183 

    0x2ed209ea,// 187 PAY 184 

    0x49eb1917,// 188 PAY 185 

    0x7841a5a4,// 189 PAY 186 

    0x5bedfcb2,// 190 PAY 187 

    0x2db7683e,// 191 PAY 188 

    0xf59a2dd8,// 192 PAY 189 

    0xaf37d5a5,// 193 PAY 190 

    0x2d0ce308,// 194 PAY 191 

    0x527a9320,// 195 PAY 192 

    0x73b8c864,// 196 PAY 193 

    0x4f9464e0,// 197 PAY 194 

    0xf27c60ff,// 198 PAY 195 

    0xf7b85b63,// 199 PAY 196 

    0x352dd401,// 200 PAY 197 

    0x614a17b3,// 201 PAY 198 

    0xd4bfdfe5,// 202 PAY 199 

    0x6fc92e8e,// 203 PAY 200 

    0xda538c7d,// 204 PAY 201 

    0x1425cd9e,// 205 PAY 202 

    0x69e8f6cb,// 206 PAY 203 

    0x7bbc0d8e,// 207 PAY 204 

    0x8c378b8b,// 208 PAY 205 

    0x671aa0cb,// 209 PAY 206 

    0x33880aff,// 210 PAY 207 

    0x4327f49d,// 211 PAY 208 

    0x3df71093,// 212 PAY 209 

    0x54f1c138,// 213 PAY 210 

    0x2bf61121,// 214 PAY 211 

    0xdc8b2b6a,// 215 PAY 212 

    0x73bc093e,// 216 PAY 213 

    0xae9d0693,// 217 PAY 214 

    0x16af64c4,// 218 PAY 215 

    0xe18e1af4,// 219 PAY 216 

    0x9aea380b,// 220 PAY 217 

    0x9868d705,// 221 PAY 218 

    0xa9a0d7e3,// 222 PAY 219 

    0xe1fb86d8,// 223 PAY 220 

    0x343454a0,// 224 PAY 221 

    0x00e4363b,// 225 PAY 222 

    0x3bdf14ed,// 226 PAY 223 

    0x05ead44b,// 227 PAY 224 

    0x027c2938,// 228 PAY 225 

    0xee210b2e,// 229 PAY 226 

    0x91cd5ff4,// 230 PAY 227 

    0x3b74e469,// 231 PAY 228 

    0x6d8afd4f,// 232 PAY 229 

    0x4f6e5f43,// 233 PAY 230 

    0x96cbc1ef,// 234 PAY 231 

    0x24f16789,// 235 PAY 232 

    0x91652e65,// 236 PAY 233 

    0xceb360a4,// 237 PAY 234 

    0xa0770ee5,// 238 PAY 235 

    0x0403f0d4,// 239 PAY 236 

    0x558315c4,// 240 PAY 237 

    0x583d3d6e,// 241 PAY 238 

    0xf38285ab,// 242 PAY 239 

    0x2722b3b2,// 243 PAY 240 

    0x9022eaac,// 244 PAY 241 

    0xebedbd1a,// 245 PAY 242 

    0xa7c2c18b,// 246 PAY 243 

    0x4b55ee4c,// 247 PAY 244 

    0xfb872784,// 248 PAY 245 

    0xda03cbe5,// 249 PAY 246 

    0x82856583,// 250 PAY 247 

    0x1a78503b,// 251 PAY 248 

    0x47a2a654,// 252 PAY 249 

    0x766f965f,// 253 PAY 250 

    0xd7898c7e,// 254 PAY 251 

    0xf58e245a,// 255 PAY 252 

    0x2f1ed373,// 256 PAY 253 

    0x86f35c3d,// 257 PAY 254 

    0xfcb22a49,// 258 PAY 255 

    0x86d72692,// 259 PAY 256 

    0xeafc60db,// 260 PAY 257 

    0xafe7f4d6,// 261 PAY 258 

    0x9c806667,// 262 PAY 259 

    0x215cd579,// 263 PAY 260 

    0x8d6f58bc,// 264 PAY 261 

    0x4724512c,// 265 PAY 262 

    0x4a8bd6c8,// 266 PAY 263 

    0xd07247e1,// 267 PAY 264 

    0xd4acf821,// 268 PAY 265 

    0xc3bf11ba,// 269 PAY 266 

    0x6ad629d3,// 270 PAY 267 

    0xbc3d305c,// 271 PAY 268 

    0x98829e69,// 272 PAY 269 

    0xd4762a42,// 273 PAY 270 

    0x2184da8f,// 274 PAY 271 

    0x1a3eb327,// 275 PAY 272 

    0x22c68361,// 276 PAY 273 

    0xf0170121,// 277 PAY 274 

    0x00ba7401,// 278 PAY 275 

    0xf7ebfd3e,// 279 PAY 276 

    0x2f12af7e,// 280 PAY 277 

    0x04882835,// 281 PAY 278 

    0xb7935d08,// 282 PAY 279 

    0xb82bacb4,// 283 PAY 280 

    0x9bb4d163,// 284 PAY 281 

    0x79dfb8fd,// 285 PAY 282 

    0x55d600c9,// 286 PAY 283 

    0xd995cf71,// 287 PAY 284 

    0xd317307c,// 288 PAY 285 

    0x848e1d3b,// 289 PAY 286 

    0x598bc1a5,// 290 PAY 287 

    0x7d737d01,// 291 PAY 288 

    0x302fe7cc,// 292 PAY 289 

    0x121e3b87,// 293 PAY 290 

    0x09184172,// 294 PAY 291 

    0x42d89f5a,// 295 PAY 292 

    0x288f0ca4,// 296 PAY 293 

    0x7fdff556,// 297 PAY 294 

    0x639dbe1b,// 298 PAY 295 

    0x3571583c,// 299 PAY 296 

    0xe07c3a96,// 300 PAY 297 

    0xe297f4b9,// 301 PAY 298 

    0x820cfd3e,// 302 PAY 299 

    0x01059d53,// 303 PAY 300 

    0x361fdc67,// 304 PAY 301 

    0xcb1a5fb0,// 305 PAY 302 

    0x5a5d6709,// 306 PAY 303 

    0x405e9c78,// 307 PAY 304 

    0x1929046b,// 308 PAY 305 

    0x24762d47,// 309 PAY 306 

    0x7ce8c9e9,// 310 PAY 307 

    0xdc6b6ffb,// 311 PAY 308 

    0xe18a6fba,// 312 PAY 309 

    0xad6b8018,// 313 PAY 310 

    0x5b715b71,// 314 PAY 311 

    0x807f3089,// 315 PAY 312 

    0xc42d3ee3,// 316 PAY 313 

    0xfb553a7a,// 317 PAY 314 

    0xca9c59bc,// 318 PAY 315 

    0x13885e0d,// 319 PAY 316 

    0xc7c51533,// 320 PAY 317 

    0x0f15015a,// 321 PAY 318 

    0x8f8dfc0b,// 322 PAY 319 

    0xfa3aed2f,// 323 PAY 320 

    0x3112277d,// 324 PAY 321 

    0x596f70c1,// 325 PAY 322 

    0x10326c58,// 326 PAY 323 

    0x9c933db4,// 327 PAY 324 

    0x1eb0a115,// 328 PAY 325 

    0xa508477a,// 329 PAY 326 

    0x6d776fb9,// 330 PAY 327 

    0x712836c4,// 331 PAY 328 

    0x03ab5d77,// 332 PAY 329 

    0x310bbe49,// 333 PAY 330 

    0x257e3bf8,// 334 PAY 331 

    0x0a5bc5f5,// 335 PAY 332 

    0xa8c9678a,// 336 PAY 333 

    0xfcfd89a4,// 337 PAY 334 

    0x728d0738,// 338 PAY 335 

    0x3d57f376,// 339 PAY 336 

    0x5946c3be,// 340 PAY 337 

    0xb51b4fc9,// 341 PAY 338 

    0x98ac4917,// 342 PAY 339 

    0x2903b967,// 343 PAY 340 

    0xdd3da8a7,// 344 PAY 341 

    0x54296891,// 345 PAY 342 

    0xaff94688,// 346 PAY 343 

    0xb0ae5696,// 347 PAY 344 

    0xddaef993,// 348 PAY 345 

    0x78d248b9,// 349 PAY 346 

    0x1992a810,// 350 PAY 347 

    0xb366b5b4,// 351 PAY 348 

    0x325802e5,// 352 PAY 349 

    0x348fa571,// 353 PAY 350 

    0x0ed7d0b0,// 354 PAY 351 

    0x7ee6244e,// 355 PAY 352 

    0x46caf135,// 356 PAY 353 

    0x452fdbae,// 357 PAY 354 

    0x2bc2dcff,// 358 PAY 355 

    0x4a493acb,// 359 PAY 356 

    0x4678bbf7,// 360 PAY 357 

    0x4e25fd57,// 361 PAY 358 

    0x7b66335d,// 362 PAY 359 

    0x405d3ff4,// 363 PAY 360 

    0x7d5bb931,// 364 PAY 361 

    0x52ef9a76,// 365 PAY 362 

    0x8181b40b,// 366 PAY 363 

    0xff287703,// 367 PAY 364 

    0x478903c7,// 368 PAY 365 

    0x5d15cf24,// 369 PAY 366 

    0xbad10cd5,// 370 PAY 367 

    0xd0ab45b8,// 371 PAY 368 

    0xab37ff49,// 372 PAY 369 

    0xbd7ed0b0,// 373 PAY 370 

    0xf8e68554,// 374 PAY 371 

    0x099f9165,// 375 PAY 372 

    0xada0d06b,// 376 PAY 373 

    0x15146088,// 377 PAY 374 

    0x05966173,// 378 PAY 375 

    0x3560d1ae,// 379 PAY 376 

    0x67dc6ff0,// 380 PAY 377 

    0x7c56a6c0,// 381 PAY 378 

    0x8fc5a114,// 382 PAY 379 

    0x17515eb7,// 383 PAY 380 

    0xfeec1beb,// 384 PAY 381 

    0xbf299699,// 385 PAY 382 

    0xc9ea6a31,// 386 PAY 383 

    0x88399cc3,// 387 PAY 384 

    0x8d12dcba,// 388 PAY 385 

    0x485b9102,// 389 PAY 386 

    0x59d4a077,// 390 PAY 387 

    0xeebae2cb,// 391 PAY 388 

    0x7b63162b,// 392 PAY 389 

    0xeac27ead,// 393 PAY 390 

    0x5d5ef659,// 394 PAY 391 

    0xdc9588d3,// 395 PAY 392 

    0x20658942,// 396 PAY 393 

    0xa5ee238b,// 397 PAY 394 

    0x2f54233d,// 398 PAY 395 

    0x00d0d763,// 399 PAY 396 

    0xbfcf39b9,// 400 PAY 397 

    0xd40d860b,// 401 PAY 398 

    0x97dbe337,// 402 PAY 399 

    0x75585dd4,// 403 PAY 400 

    0x65d2925f,// 404 PAY 401 

    0xe82a1205,// 405 PAY 402 

    0x7ece700f,// 406 PAY 403 

    0x8ef99c19,// 407 PAY 404 

    0x7ff91348,// 408 PAY 405 

    0xcdc15188,// 409 PAY 406 

    0xd01b9e8a,// 410 PAY 407 

    0x853f9ee2,// 411 PAY 408 

    0x78d1adc2,// 412 PAY 409 

    0xa7b8b4b8,// 413 PAY 410 

    0x2d2b829e,// 414 PAY 411 

    0x4b56e197,// 415 PAY 412 

    0x84101720,// 416 PAY 413 

    0x81fe67ea,// 417 PAY 414 

    0xc125a5d1,// 418 PAY 415 

    0x02170a79,// 419 PAY 416 

    0x5630957c,// 420 PAY 417 

    0xd41db11f,// 421 PAY 418 

    0x62e5e3d2,// 422 PAY 419 

    0x72644d32,// 423 PAY 420 

    0x32000dbd,// 424 PAY 421 

    0x30db2b89,// 425 PAY 422 

    0xdbb70e89,// 426 PAY 423 

    0x50aa89d5,// 427 PAY 424 

    0x64a8c93c,// 428 PAY 425 

    0x01cc3b5f,// 429 PAY 426 

    0x271e3a6a,// 430 PAY 427 

    0x11cca92c,// 431 PAY 428 

    0xd1fddc23,// 432 PAY 429 

    0x26000000,// 433 PAY 430 

/// HASH is  12 bytes 

    0x45494d1e,// 434 HSH   1 

    0x839a2ebe,// 435 HSH   2 

    0x5aea56e7,// 436 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 75 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x28 

    0x0111284b // 437 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt33_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 245 words. 

/// BDA size     is 974 (0x3ce) 

/// BDA id       is 0x8db2 

    0x03ce8db2,// 3 BDA   1 

/// PAY Generic Data size   : 974 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x4f940b6a,// 4 PAY   1 

    0x2abd848e,// 5 PAY   2 

    0xe653f38a,// 6 PAY   3 

    0xca30e849,// 7 PAY   4 

    0xef2541a4,// 8 PAY   5 

    0x35d3b16b,// 9 PAY   6 

    0x02e66a51,// 10 PAY   7 

    0x2d115028,// 11 PAY   8 

    0x0ab24a8b,// 12 PAY   9 

    0xd0552c9c,// 13 PAY  10 

    0xea0c15b8,// 14 PAY  11 

    0x79744112,// 15 PAY  12 

    0xbb70dad1,// 16 PAY  13 

    0xb98169dd,// 17 PAY  14 

    0x629734f4,// 18 PAY  15 

    0x4ac0b885,// 19 PAY  16 

    0xde89f7e2,// 20 PAY  17 

    0xb814edc8,// 21 PAY  18 

    0x8d4786ec,// 22 PAY  19 

    0x3fd2d8bb,// 23 PAY  20 

    0x14da1c82,// 24 PAY  21 

    0xbc3d16de,// 25 PAY  22 

    0xb6d177ae,// 26 PAY  23 

    0x7447db56,// 27 PAY  24 

    0xc50ee470,// 28 PAY  25 

    0xbc4cf9a3,// 29 PAY  26 

    0x64a8e39c,// 30 PAY  27 

    0x34ea3348,// 31 PAY  28 

    0x5f7ecd35,// 32 PAY  29 

    0xb6a6aaf1,// 33 PAY  30 

    0x9105c6fb,// 34 PAY  31 

    0xa157bb6c,// 35 PAY  32 

    0xe09a33be,// 36 PAY  33 

    0x40dd219c,// 37 PAY  34 

    0x97236308,// 38 PAY  35 

    0xd809e585,// 39 PAY  36 

    0x4b81f895,// 40 PAY  37 

    0xc446cf58,// 41 PAY  38 

    0xc231a844,// 42 PAY  39 

    0x0e467be6,// 43 PAY  40 

    0x44e0c4e5,// 44 PAY  41 

    0xf9725de6,// 45 PAY  42 

    0xcd570a68,// 46 PAY  43 

    0x0450c727,// 47 PAY  44 

    0xeddf55ed,// 48 PAY  45 

    0xc35bc22a,// 49 PAY  46 

    0xa246a7c7,// 50 PAY  47 

    0xb89fc750,// 51 PAY  48 

    0xe0afac88,// 52 PAY  49 

    0xd53bd6e7,// 53 PAY  50 

    0x3fa6a171,// 54 PAY  51 

    0xc9698728,// 55 PAY  52 

    0x118cdd88,// 56 PAY  53 

    0x92f878d8,// 57 PAY  54 

    0x51777b08,// 58 PAY  55 

    0x08471989,// 59 PAY  56 

    0x6e5bda65,// 60 PAY  57 

    0x2b30fd72,// 61 PAY  58 

    0xcbfc0343,// 62 PAY  59 

    0x1cde6eaf,// 63 PAY  60 

    0xe33b914c,// 64 PAY  61 

    0x6ca3ffe7,// 65 PAY  62 

    0xc3fa74bf,// 66 PAY  63 

    0x169a28d1,// 67 PAY  64 

    0x13d8f724,// 68 PAY  65 

    0x094e701c,// 69 PAY  66 

    0x1b895a33,// 70 PAY  67 

    0x0fca1359,// 71 PAY  68 

    0x052e391d,// 72 PAY  69 

    0x179ed43e,// 73 PAY  70 

    0x1090fe5a,// 74 PAY  71 

    0x8e90916b,// 75 PAY  72 

    0x5b27607a,// 76 PAY  73 

    0x1342b550,// 77 PAY  74 

    0xc8131276,// 78 PAY  75 

    0xf6915c70,// 79 PAY  76 

    0x00de7160,// 80 PAY  77 

    0x93956c64,// 81 PAY  78 

    0x867b5bb6,// 82 PAY  79 

    0x228dc8c8,// 83 PAY  80 

    0x67622ca1,// 84 PAY  81 

    0x48557a72,// 85 PAY  82 

    0x2ff4ad73,// 86 PAY  83 

    0xe42fbcc4,// 87 PAY  84 

    0x7455cdf1,// 88 PAY  85 

    0x8a40d3f6,// 89 PAY  86 

    0x21839277,// 90 PAY  87 

    0x5984e342,// 91 PAY  88 

    0x36bbb44a,// 92 PAY  89 

    0x59347117,// 93 PAY  90 

    0x301984e4,// 94 PAY  91 

    0x6f875000,// 95 PAY  92 

    0xfe1f794b,// 96 PAY  93 

    0xa69d9d8b,// 97 PAY  94 

    0x4f8f5b10,// 98 PAY  95 

    0x8b2e186a,// 99 PAY  96 

    0xe61f12a9,// 100 PAY  97 

    0xc891a330,// 101 PAY  98 

    0x4bf1b890,// 102 PAY  99 

    0x593b4576,// 103 PAY 100 

    0xf216c60b,// 104 PAY 101 

    0xb5d7d9db,// 105 PAY 102 

    0x3d56cf73,// 106 PAY 103 

    0xbe060561,// 107 PAY 104 

    0x875d37b2,// 108 PAY 105 

    0xbaaa2093,// 109 PAY 106 

    0xdac61be8,// 110 PAY 107 

    0x651ffc1f,// 111 PAY 108 

    0xfd3c5b34,// 112 PAY 109 

    0x902dd27c,// 113 PAY 110 

    0x4575a5a6,// 114 PAY 111 

    0x9a3548a0,// 115 PAY 112 

    0x9f4d6e6f,// 116 PAY 113 

    0x1b1bb5bf,// 117 PAY 114 

    0xacbeafa6,// 118 PAY 115 

    0x3d08bedd,// 119 PAY 116 

    0xd1329c5a,// 120 PAY 117 

    0x65802473,// 121 PAY 118 

    0xf568490a,// 122 PAY 119 

    0x6dcfd04e,// 123 PAY 120 

    0xac19b859,// 124 PAY 121 

    0x904f6d80,// 125 PAY 122 

    0x8980e288,// 126 PAY 123 

    0xdeda0b3f,// 127 PAY 124 

    0x9da08166,// 128 PAY 125 

    0xe540ce50,// 129 PAY 126 

    0xacb0c98a,// 130 PAY 127 

    0x92fc86fe,// 131 PAY 128 

    0x576c5ae1,// 132 PAY 129 

    0x72ca6392,// 133 PAY 130 

    0x82a2bdd6,// 134 PAY 131 

    0x199787d9,// 135 PAY 132 

    0x493dedde,// 136 PAY 133 

    0x6002da19,// 137 PAY 134 

    0x474cf468,// 138 PAY 135 

    0x656bc8c8,// 139 PAY 136 

    0x07bbf4de,// 140 PAY 137 

    0xdd310463,// 141 PAY 138 

    0x914b48e8,// 142 PAY 139 

    0xd0658882,// 143 PAY 140 

    0x8adc565a,// 144 PAY 141 

    0x1592990d,// 145 PAY 142 

    0xc53cb3f4,// 146 PAY 143 

    0x0b41f66e,// 147 PAY 144 

    0x05e7171c,// 148 PAY 145 

    0xc5619a9d,// 149 PAY 146 

    0x5778fbe6,// 150 PAY 147 

    0x0decc5d5,// 151 PAY 148 

    0x24d4f154,// 152 PAY 149 

    0x97df5f98,// 153 PAY 150 

    0x0a1c3096,// 154 PAY 151 

    0xb2ab531f,// 155 PAY 152 

    0xbcdf44cf,// 156 PAY 153 

    0x542f388f,// 157 PAY 154 

    0x4d041037,// 158 PAY 155 

    0xa2a3466b,// 159 PAY 156 

    0xa3c2ef23,// 160 PAY 157 

    0xb37ef63f,// 161 PAY 158 

    0x4fca7635,// 162 PAY 159 

    0xa31c2a59,// 163 PAY 160 

    0xb6774f57,// 164 PAY 161 

    0x3ebe6dd4,// 165 PAY 162 

    0xa53368d5,// 166 PAY 163 

    0xecd2830e,// 167 PAY 164 

    0xa1d38c7d,// 168 PAY 165 

    0xf4b163e1,// 169 PAY 166 

    0xc45dcfe7,// 170 PAY 167 

    0x0ac6e791,// 171 PAY 168 

    0x839d075f,// 172 PAY 169 

    0x81d2a178,// 173 PAY 170 

    0x3e7f316c,// 174 PAY 171 

    0x658c0ec4,// 175 PAY 172 

    0x06c6cd1a,// 176 PAY 173 

    0x91b47f6b,// 177 PAY 174 

    0x78b8ebf3,// 178 PAY 175 

    0xd7aa7686,// 179 PAY 176 

    0xf91cb518,// 180 PAY 177 

    0xe789455a,// 181 PAY 178 

    0xf88aa746,// 182 PAY 179 

    0xd6725d68,// 183 PAY 180 

    0x0e978116,// 184 PAY 181 

    0xd2277f33,// 185 PAY 182 

    0xce8ab423,// 186 PAY 183 

    0x682f7b30,// 187 PAY 184 

    0x14de08fc,// 188 PAY 185 

    0x9f244cd3,// 189 PAY 186 

    0xb5354c4f,// 190 PAY 187 

    0x127a5aff,// 191 PAY 188 

    0x0f2eb3b4,// 192 PAY 189 

    0x5eac89c3,// 193 PAY 190 

    0x11ee2607,// 194 PAY 191 

    0xec4d1710,// 195 PAY 192 

    0x1c40cab3,// 196 PAY 193 

    0xb6b91cdb,// 197 PAY 194 

    0x5a98e7d4,// 198 PAY 195 

    0xe5c62fe9,// 199 PAY 196 

    0x1b5591d2,// 200 PAY 197 

    0x5bdff056,// 201 PAY 198 

    0x4ce9660d,// 202 PAY 199 

    0x0e82381b,// 203 PAY 200 

    0x3ac0cf02,// 204 PAY 201 

    0x3707ec3a,// 205 PAY 202 

    0x74445feb,// 206 PAY 203 

    0x55e2a0eb,// 207 PAY 204 

    0x1ddbd3cc,// 208 PAY 205 

    0xf2037385,// 209 PAY 206 

    0xed934ee0,// 210 PAY 207 

    0xdc6991a2,// 211 PAY 208 

    0xca006cc4,// 212 PAY 209 

    0xe8832d4f,// 213 PAY 210 

    0xa6f4748c,// 214 PAY 211 

    0xea69d6dc,// 215 PAY 212 

    0x2b653b48,// 216 PAY 213 

    0x624bf70f,// 217 PAY 214 

    0x13c924db,// 218 PAY 215 

    0xdbdc0c25,// 219 PAY 216 

    0xe62a55cb,// 220 PAY 217 

    0xcc03a7b7,// 221 PAY 218 

    0x8e50e9d0,// 222 PAY 219 

    0xbe1a0279,// 223 PAY 220 

    0x911d917b,// 224 PAY 221 

    0x5fcf3f74,// 225 PAY 222 

    0x1cc1fc9f,// 226 PAY 223 

    0xc50eafe0,// 227 PAY 224 

    0xb03a9c11,// 228 PAY 225 

    0xf81e4170,// 229 PAY 226 

    0xb3da49db,// 230 PAY 227 

    0x9a28ce0f,// 231 PAY 228 

    0x61c68353,// 232 PAY 229 

    0xd0b85f6b,// 233 PAY 230 

    0x87fb5545,// 234 PAY 231 

    0x89edfa00,// 235 PAY 232 

    0x655c9448,// 236 PAY 233 

    0x18749531,// 237 PAY 234 

    0x200c3ba2,// 238 PAY 235 

    0xc9e81b20,// 239 PAY 236 

    0x27ca305c,// 240 PAY 237 

    0xd413ece9,// 241 PAY 238 

    0x213ba55c,// 242 PAY 239 

    0x2af6b27d,// 243 PAY 240 

    0x73dc82a7,// 244 PAY 241 

    0x65d1300e,// 245 PAY 242 

    0xd7bcc133,// 246 PAY 243 

    0xe1c10000,// 247 PAY 244 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 141 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb2 

    0x0234b237 // 248 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt34_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 402 words. 

/// BDA size     is 1601 (0x641) 

/// BDA id       is 0xaed0 

    0x0641aed0,// 3 BDA   1 

/// PAY Generic Data size   : 1601 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xa7e41548,// 4 PAY   1 

    0x0f0080ba,// 5 PAY   2 

    0x7dc32ee4,// 6 PAY   3 

    0x358d2acb,// 7 PAY   4 

    0x395692b5,// 8 PAY   5 

    0xd9827350,// 9 PAY   6 

    0x5a0aa4bf,// 10 PAY   7 

    0x83873264,// 11 PAY   8 

    0xf7eb1eec,// 12 PAY   9 

    0xf2cdb23c,// 13 PAY  10 

    0x37c2992c,// 14 PAY  11 

    0x0352037d,// 15 PAY  12 

    0x2825a077,// 16 PAY  13 

    0x76a028d1,// 17 PAY  14 

    0x551f8239,// 18 PAY  15 

    0x9d982f34,// 19 PAY  16 

    0x2aba992a,// 20 PAY  17 

    0x9e051cfe,// 21 PAY  18 

    0xc0c87811,// 22 PAY  19 

    0x3b49727d,// 23 PAY  20 

    0x3be1785f,// 24 PAY  21 

    0xa472e933,// 25 PAY  22 

    0x68bb7fa5,// 26 PAY  23 

    0xf03f21ce,// 27 PAY  24 

    0x41a9a3dc,// 28 PAY  25 

    0xab506eb6,// 29 PAY  26 

    0x4aa7aa0d,// 30 PAY  27 

    0xd8eab594,// 31 PAY  28 

    0xbc9160f5,// 32 PAY  29 

    0x67fa8fac,// 33 PAY  30 

    0xe61eea09,// 34 PAY  31 

    0x7b7bbc15,// 35 PAY  32 

    0x73d64f9a,// 36 PAY  33 

    0x75191bb2,// 37 PAY  34 

    0x91fd0bfb,// 38 PAY  35 

    0x7b17f556,// 39 PAY  36 

    0xc9c77013,// 40 PAY  37 

    0xbdb6ffa4,// 41 PAY  38 

    0x6ccc5889,// 42 PAY  39 

    0xd232d7ec,// 43 PAY  40 

    0x71dc7b16,// 44 PAY  41 

    0x6f979eb0,// 45 PAY  42 

    0xea6910db,// 46 PAY  43 

    0x74d2f3e2,// 47 PAY  44 

    0xed53b146,// 48 PAY  45 

    0x4ea9856a,// 49 PAY  46 

    0x6ff746f7,// 50 PAY  47 

    0x43390088,// 51 PAY  48 

    0x0c114159,// 52 PAY  49 

    0x35c0f746,// 53 PAY  50 

    0xdf027bc1,// 54 PAY  51 

    0x49cc82ef,// 55 PAY  52 

    0x7878b710,// 56 PAY  53 

    0x84c283fb,// 57 PAY  54 

    0x8d15b777,// 58 PAY  55 

    0xaaa02d9b,// 59 PAY  56 

    0x64d34fb1,// 60 PAY  57 

    0x55a21d4b,// 61 PAY  58 

    0x159ac998,// 62 PAY  59 

    0x3fa0b308,// 63 PAY  60 

    0xe5e5d1e8,// 64 PAY  61 

    0x04c210eb,// 65 PAY  62 

    0x770aeab3,// 66 PAY  63 

    0x516f8d9a,// 67 PAY  64 

    0x6c4ad30f,// 68 PAY  65 

    0xcf2805d0,// 69 PAY  66 

    0x7c0319e8,// 70 PAY  67 

    0xcb1e5a98,// 71 PAY  68 

    0x22687683,// 72 PAY  69 

    0x91858bc0,// 73 PAY  70 

    0x38037e8c,// 74 PAY  71 

    0xfe5e442c,// 75 PAY  72 

    0x288d0888,// 76 PAY  73 

    0x3389a719,// 77 PAY  74 

    0x93ca98aa,// 78 PAY  75 

    0x00f6d5ab,// 79 PAY  76 

    0x84e5b47e,// 80 PAY  77 

    0x12a84674,// 81 PAY  78 

    0xa7a5f5af,// 82 PAY  79 

    0xe8b34646,// 83 PAY  80 

    0xcbb7662c,// 84 PAY  81 

    0x7eaa3c5e,// 85 PAY  82 

    0x347f6b4b,// 86 PAY  83 

    0x40daec2f,// 87 PAY  84 

    0xd36456a1,// 88 PAY  85 

    0x98819885,// 89 PAY  86 

    0x2b443b5b,// 90 PAY  87 

    0x3f38b37d,// 91 PAY  88 

    0x67a0f6b8,// 92 PAY  89 

    0x46a376cf,// 93 PAY  90 

    0x1ac5dd71,// 94 PAY  91 

    0xfa9daee8,// 95 PAY  92 

    0xd28bb370,// 96 PAY  93 

    0x9bf0da03,// 97 PAY  94 

    0xd5ef138c,// 98 PAY  95 

    0x404311b4,// 99 PAY  96 

    0x2ecf813c,// 100 PAY  97 

    0x61832313,// 101 PAY  98 

    0xe963afab,// 102 PAY  99 

    0x6df4c7f6,// 103 PAY 100 

    0x29c4862d,// 104 PAY 101 

    0x82a7b3a0,// 105 PAY 102 

    0xb4048017,// 106 PAY 103 

    0xe09914fc,// 107 PAY 104 

    0x4c1999d7,// 108 PAY 105 

    0x267b3e94,// 109 PAY 106 

    0x3278ccf9,// 110 PAY 107 

    0x8f5281b6,// 111 PAY 108 

    0x83e5e018,// 112 PAY 109 

    0xf096ea5e,// 113 PAY 110 

    0xf9c65edb,// 114 PAY 111 

    0xf04b1732,// 115 PAY 112 

    0x1e456a68,// 116 PAY 113 

    0x0feb82e4,// 117 PAY 114 

    0x3ea18867,// 118 PAY 115 

    0x250d63da,// 119 PAY 116 

    0xf6876bce,// 120 PAY 117 

    0xe624bc23,// 121 PAY 118 

    0xd3f7b059,// 122 PAY 119 

    0xb434c241,// 123 PAY 120 

    0xc73cbdf3,// 124 PAY 121 

    0x9f1b4609,// 125 PAY 122 

    0x8b764726,// 126 PAY 123 

    0xbfbd129e,// 127 PAY 124 

    0x654f455f,// 128 PAY 125 

    0xb64833cf,// 129 PAY 126 

    0x59a242e5,// 130 PAY 127 

    0x43d4012e,// 131 PAY 128 

    0x9ae414be,// 132 PAY 129 

    0x0a1b32b6,// 133 PAY 130 

    0x74555bdb,// 134 PAY 131 

    0x7126c488,// 135 PAY 132 

    0x56435821,// 136 PAY 133 

    0xdaa2ff2e,// 137 PAY 134 

    0x67e4ba41,// 138 PAY 135 

    0x7c50eb72,// 139 PAY 136 

    0xc55fde15,// 140 PAY 137 

    0xc73d4b12,// 141 PAY 138 

    0x7993cedc,// 142 PAY 139 

    0x7936210f,// 143 PAY 140 

    0x83b236c7,// 144 PAY 141 

    0x9a444e46,// 145 PAY 142 

    0x067b361b,// 146 PAY 143 

    0x7866f16b,// 147 PAY 144 

    0x0b3fa0e1,// 148 PAY 145 

    0x09f06a4c,// 149 PAY 146 

    0x7523d652,// 150 PAY 147 

    0xb18cad38,// 151 PAY 148 

    0xc9aaa5ac,// 152 PAY 149 

    0x2a693c04,// 153 PAY 150 

    0x9f6bb099,// 154 PAY 151 

    0x8f2416a4,// 155 PAY 152 

    0x03cc45d5,// 156 PAY 153 

    0x948800da,// 157 PAY 154 

    0x2c5d292a,// 158 PAY 155 

    0x4eb4bbb3,// 159 PAY 156 

    0xe47bfbd9,// 160 PAY 157 

    0xd3491e13,// 161 PAY 158 

    0xf5f000fa,// 162 PAY 159 

    0xa6748db3,// 163 PAY 160 

    0x14b78aa8,// 164 PAY 161 

    0x745d5458,// 165 PAY 162 

    0x8b968027,// 166 PAY 163 

    0x9f477be0,// 167 PAY 164 

    0xebb850c1,// 168 PAY 165 

    0x6e9c7c9a,// 169 PAY 166 

    0xf5bda129,// 170 PAY 167 

    0xe450cd97,// 171 PAY 168 

    0x71001676,// 172 PAY 169 

    0x4dc24721,// 173 PAY 170 

    0x22896213,// 174 PAY 171 

    0x288e24ae,// 175 PAY 172 

    0xd4ca7c32,// 176 PAY 173 

    0x4c7f1897,// 177 PAY 174 

    0x955f58b8,// 178 PAY 175 

    0x22ca54a4,// 179 PAY 176 

    0x71f30c16,// 180 PAY 177 

    0xa72e2a26,// 181 PAY 178 

    0x4a7043b0,// 182 PAY 179 

    0x9f618f85,// 183 PAY 180 

    0x60c677f0,// 184 PAY 181 

    0x455c04a2,// 185 PAY 182 

    0x04254038,// 186 PAY 183 

    0x1f90b552,// 187 PAY 184 

    0xafd3e56b,// 188 PAY 185 

    0xc8682755,// 189 PAY 186 

    0x157ba22a,// 190 PAY 187 

    0xf032fa4c,// 191 PAY 188 

    0x9e77bcfd,// 192 PAY 189 

    0x12e3ac6f,// 193 PAY 190 

    0x6b2ca92b,// 194 PAY 191 

    0x8c4aba18,// 195 PAY 192 

    0x3b68f99b,// 196 PAY 193 

    0xc589b66a,// 197 PAY 194 

    0x18f4c6d2,// 198 PAY 195 

    0x7f7a6554,// 199 PAY 196 

    0xfbb50d9d,// 200 PAY 197 

    0x83ebec30,// 201 PAY 198 

    0x13af9cb8,// 202 PAY 199 

    0x324d23df,// 203 PAY 200 

    0xed195272,// 204 PAY 201 

    0xa43a2812,// 205 PAY 202 

    0x906d5d2b,// 206 PAY 203 

    0x957967be,// 207 PAY 204 

    0x103c454e,// 208 PAY 205 

    0x7692527c,// 209 PAY 206 

    0xcaec992a,// 210 PAY 207 

    0xf843075d,// 211 PAY 208 

    0x327cc595,// 212 PAY 209 

    0x4818735e,// 213 PAY 210 

    0xad798bab,// 214 PAY 211 

    0x013bff89,// 215 PAY 212 

    0x0b2a766b,// 216 PAY 213 

    0x177126a8,// 217 PAY 214 

    0x5ca3f0cc,// 218 PAY 215 

    0x0524e82a,// 219 PAY 216 

    0x2ff3bffe,// 220 PAY 217 

    0x0e95af3d,// 221 PAY 218 

    0xe12dcf0c,// 222 PAY 219 

    0x58d0dff5,// 223 PAY 220 

    0x59697f29,// 224 PAY 221 

    0x7f0a5232,// 225 PAY 222 

    0xee6bc4aa,// 226 PAY 223 

    0x66478667,// 227 PAY 224 

    0xc34955eb,// 228 PAY 225 

    0xe4f06231,// 229 PAY 226 

    0xd94c5c85,// 230 PAY 227 

    0x898a5761,// 231 PAY 228 

    0xe28fee4e,// 232 PAY 229 

    0xfb0b5062,// 233 PAY 230 

    0x838b01ff,// 234 PAY 231 

    0x65819d83,// 235 PAY 232 

    0x8f73abe9,// 236 PAY 233 

    0xd9da52b2,// 237 PAY 234 

    0xa717ca0a,// 238 PAY 235 

    0xe74865d0,// 239 PAY 236 

    0x7b24b53a,// 240 PAY 237 

    0xff003e76,// 241 PAY 238 

    0xab2b89a2,// 242 PAY 239 

    0xdf3618b4,// 243 PAY 240 

    0x707d1741,// 244 PAY 241 

    0xaf5508f3,// 245 PAY 242 

    0x244d93b2,// 246 PAY 243 

    0x17031265,// 247 PAY 244 

    0x6363d53c,// 248 PAY 245 

    0x5cd69faa,// 249 PAY 246 

    0x27535ff5,// 250 PAY 247 

    0x0a1d0f5b,// 251 PAY 248 

    0x3f024943,// 252 PAY 249 

    0xf775c2aa,// 253 PAY 250 

    0x812ec15b,// 254 PAY 251 

    0xef82574c,// 255 PAY 252 

    0xe5c8655f,// 256 PAY 253 

    0xce7be555,// 257 PAY 254 

    0x3dcbf1d3,// 258 PAY 255 

    0x57c40002,// 259 PAY 256 

    0x1b86f8a3,// 260 PAY 257 

    0xf659b54f,// 261 PAY 258 

    0x8b40e11d,// 262 PAY 259 

    0x65da9e53,// 263 PAY 260 

    0x06fa45d2,// 264 PAY 261 

    0x0c01d937,// 265 PAY 262 

    0xb70c49a6,// 266 PAY 263 

    0xbaf927b7,// 267 PAY 264 

    0x4b0b157e,// 268 PAY 265 

    0xd7f06a10,// 269 PAY 266 

    0x4b4a9bd0,// 270 PAY 267 

    0x2545aeb3,// 271 PAY 268 

    0x71498839,// 272 PAY 269 

    0xf0e5c9df,// 273 PAY 270 

    0xe92a5965,// 274 PAY 271 

    0x4af32a96,// 275 PAY 272 

    0x74f2f203,// 276 PAY 273 

    0x93647cb4,// 277 PAY 274 

    0x52374408,// 278 PAY 275 

    0x3183d534,// 279 PAY 276 

    0xe423a493,// 280 PAY 277 

    0x3a3f1fcc,// 281 PAY 278 

    0x402871b7,// 282 PAY 279 

    0xb9c88a3f,// 283 PAY 280 

    0x614c1484,// 284 PAY 281 

    0x08fcb9db,// 285 PAY 282 

    0x600bb3d3,// 286 PAY 283 

    0xc8bd16e4,// 287 PAY 284 

    0x6be464d6,// 288 PAY 285 

    0xea03d276,// 289 PAY 286 

    0xb0ef486d,// 290 PAY 287 

    0xdcc8e21d,// 291 PAY 288 

    0x2461fe30,// 292 PAY 289 

    0xa97d17fa,// 293 PAY 290 

    0xc16effa9,// 294 PAY 291 

    0x0527ff33,// 295 PAY 292 

    0xb7f05911,// 296 PAY 293 

    0xca82ad1e,// 297 PAY 294 

    0xecf4b30f,// 298 PAY 295 

    0x86f1557f,// 299 PAY 296 

    0x72a8619a,// 300 PAY 297 

    0x27e3f5da,// 301 PAY 298 

    0xe6e14e4d,// 302 PAY 299 

    0x8508dae4,// 303 PAY 300 

    0xed2a7a1c,// 304 PAY 301 

    0xf55ec87f,// 305 PAY 302 

    0xb2b25811,// 306 PAY 303 

    0xae51e9d0,// 307 PAY 304 

    0x00799602,// 308 PAY 305 

    0xa7a5b9c8,// 309 PAY 306 

    0x8e420739,// 310 PAY 307 

    0x08bbc7c4,// 311 PAY 308 

    0xca33d641,// 312 PAY 309 

    0x5d693347,// 313 PAY 310 

    0xf9f655b0,// 314 PAY 311 

    0x7bf9704b,// 315 PAY 312 

    0x1cc1188f,// 316 PAY 313 

    0x9d7488c6,// 317 PAY 314 

    0x885a5071,// 318 PAY 315 

    0xa0fc0d4f,// 319 PAY 316 

    0x15552445,// 320 PAY 317 

    0x6ceff593,// 321 PAY 318 

    0x9cf0c3af,// 322 PAY 319 

    0x30c985c9,// 323 PAY 320 

    0x5eb642de,// 324 PAY 321 

    0x25188492,// 325 PAY 322 

    0x845f61c3,// 326 PAY 323 

    0x5ca7207c,// 327 PAY 324 

    0x9ca7e81e,// 328 PAY 325 

    0x34917a4e,// 329 PAY 326 

    0x77a3b762,// 330 PAY 327 

    0xee227809,// 331 PAY 328 

    0x6b8a69ca,// 332 PAY 329 

    0xa4e8f9b2,// 333 PAY 330 

    0x418186d5,// 334 PAY 331 

    0x79b2f12c,// 335 PAY 332 

    0x31851f12,// 336 PAY 333 

    0xd00c6623,// 337 PAY 334 

    0x319a977e,// 338 PAY 335 

    0x432b03be,// 339 PAY 336 

    0x6043777a,// 340 PAY 337 

    0xb510d73f,// 341 PAY 338 

    0x74d11c00,// 342 PAY 339 

    0x97964cd5,// 343 PAY 340 

    0x9da03fe8,// 344 PAY 341 

    0xe4bfa627,// 345 PAY 342 

    0xb9257901,// 346 PAY 343 

    0x8b1ea506,// 347 PAY 344 

    0x916f8fda,// 348 PAY 345 

    0x537b4228,// 349 PAY 346 

    0x82941333,// 350 PAY 347 

    0x061b83b0,// 351 PAY 348 

    0xe6282a41,// 352 PAY 349 

    0xd0fc2855,// 353 PAY 350 

    0xea3aff46,// 354 PAY 351 

    0x9b070124,// 355 PAY 352 

    0x3b4370be,// 356 PAY 353 

    0x811ff64d,// 357 PAY 354 

    0x1d8a956a,// 358 PAY 355 

    0x662ad993,// 359 PAY 356 

    0xdd55eb56,// 360 PAY 357 

    0x0107d520,// 361 PAY 358 

    0x995800e4,// 362 PAY 359 

    0xb601fe98,// 363 PAY 360 

    0x175199a4,// 364 PAY 361 

    0xf209a461,// 365 PAY 362 

    0x4e847c22,// 366 PAY 363 

    0x94b9e2b2,// 367 PAY 364 

    0x50dfd18f,// 368 PAY 365 

    0xe2cd49c6,// 369 PAY 366 

    0x3c4a909c,// 370 PAY 367 

    0x5a89d799,// 371 PAY 368 

    0x4b7622a6,// 372 PAY 369 

    0x3bac38d9,// 373 PAY 370 

    0xde9334a9,// 374 PAY 371 

    0x04c0c320,// 375 PAY 372 

    0x03ba2472,// 376 PAY 373 

    0xe452b2c7,// 377 PAY 374 

    0xed71cd37,// 378 PAY 375 

    0x38017a48,// 379 PAY 376 

    0x8c39eb37,// 380 PAY 377 

    0xc75334ab,// 381 PAY 378 

    0x2f28d248,// 382 PAY 379 

    0xfb48fbe5,// 383 PAY 380 

    0xe5563282,// 384 PAY 381 

    0xd23804f6,// 385 PAY 382 

    0xd9f5584d,// 386 PAY 383 

    0x02e0c81d,// 387 PAY 384 

    0x469fd1c4,// 388 PAY 385 

    0x72208c13,// 389 PAY 386 

    0x2278cab0,// 390 PAY 387 

    0x5166ac2a,// 391 PAY 388 

    0xe0ecc17b,// 392 PAY 389 

    0xf7ae33a2,// 393 PAY 390 

    0xcf8326dc,// 394 PAY 391 

    0xd7e26124,// 395 PAY 392 

    0x2c2df0a3,// 396 PAY 393 

    0xc66e4657,// 397 PAY 394 

    0x7e2f0c2f,// 398 PAY 395 

    0x2243f7ad,// 399 PAY 396 

    0xc36a5014,// 400 PAY 397 

    0x04ea79fe,// 401 PAY 398 

    0xf8c98512,// 402 PAY 399 

    0xfb4d902a,// 403 PAY 400 

    0xde000000,// 404 PAY 401 

/// HASH is  12 bytes 

    0x2243f7ad,// 405 HSH   1 

    0xc36a5014,// 406 HSH   2 

    0x04ea79fe,// 407 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 84 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3d 

    0x02b53d54 // 408 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt35_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 343 words. 

/// BDA size     is 1365 (0x555) 

/// BDA id       is 0x41dc 

    0x055541dc,// 3 BDA   1 

/// PAY Generic Data size   : 1365 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xfde5c171,// 4 PAY   1 

    0xd44ca933,// 5 PAY   2 

    0xe6bb8a72,// 6 PAY   3 

    0x9d13346c,// 7 PAY   4 

    0x4916733b,// 8 PAY   5 

    0x6afd9439,// 9 PAY   6 

    0xc4778cb8,// 10 PAY   7 

    0x8a146238,// 11 PAY   8 

    0x7114c0da,// 12 PAY   9 

    0xbab72ef3,// 13 PAY  10 

    0x7a24ae7d,// 14 PAY  11 

    0xe600382e,// 15 PAY  12 

    0x346c475d,// 16 PAY  13 

    0xbead2674,// 17 PAY  14 

    0x48864f6e,// 18 PAY  15 

    0x644938f5,// 19 PAY  16 

    0x1d8951f9,// 20 PAY  17 

    0xd908c362,// 21 PAY  18 

    0x684d96ec,// 22 PAY  19 

    0x72a7fd8e,// 23 PAY  20 

    0x5438e0c1,// 24 PAY  21 

    0xc7a00af3,// 25 PAY  22 

    0xa72254fe,// 26 PAY  23 

    0xbf722670,// 27 PAY  24 

    0x6653d29d,// 28 PAY  25 

    0x783b228e,// 29 PAY  26 

    0x73dcccc8,// 30 PAY  27 

    0xc59c3b3c,// 31 PAY  28 

    0xd3d0214d,// 32 PAY  29 

    0xb1119947,// 33 PAY  30 

    0x235bd3b5,// 34 PAY  31 

    0xd63ec8b2,// 35 PAY  32 

    0x69cbe16c,// 36 PAY  33 

    0xda4aa076,// 37 PAY  34 

    0x23cd4682,// 38 PAY  35 

    0xfc14721f,// 39 PAY  36 

    0x5561bbfb,// 40 PAY  37 

    0x88298c97,// 41 PAY  38 

    0xe7c2bb8a,// 42 PAY  39 

    0x0bb0986a,// 43 PAY  40 

    0x58422145,// 44 PAY  41 

    0xf24433c3,// 45 PAY  42 

    0x581bcce3,// 46 PAY  43 

    0xd5efa748,// 47 PAY  44 

    0x49b323ed,// 48 PAY  45 

    0x998d4ed4,// 49 PAY  46 

    0x36fab34c,// 50 PAY  47 

    0xd74e9e2b,// 51 PAY  48 

    0xbebbe2c0,// 52 PAY  49 

    0x0ec59874,// 53 PAY  50 

    0xaab40aec,// 54 PAY  51 

    0xcb066015,// 55 PAY  52 

    0x5340b008,// 56 PAY  53 

    0xaa840ff3,// 57 PAY  54 

    0x9005647e,// 58 PAY  55 

    0xa48e2ab2,// 59 PAY  56 

    0x9e6d795f,// 60 PAY  57 

    0x624b5e26,// 61 PAY  58 

    0xdf82b516,// 62 PAY  59 

    0xfed52b87,// 63 PAY  60 

    0xd3a8f927,// 64 PAY  61 

    0x924ac432,// 65 PAY  62 

    0x52cac760,// 66 PAY  63 

    0x4f82a6db,// 67 PAY  64 

    0xdb9f370d,// 68 PAY  65 

    0xe8bf071c,// 69 PAY  66 

    0xd033f76e,// 70 PAY  67 

    0x2b7f7442,// 71 PAY  68 

    0x1f0e30aa,// 72 PAY  69 

    0x13542bb5,// 73 PAY  70 

    0x3f1f874a,// 74 PAY  71 

    0xcc98089c,// 75 PAY  72 

    0xaf8ae6f4,// 76 PAY  73 

    0x242b3ad3,// 77 PAY  74 

    0x8f8fd074,// 78 PAY  75 

    0x3991a4b6,// 79 PAY  76 

    0xb546490b,// 80 PAY  77 

    0x9b6710e2,// 81 PAY  78 

    0xd12b1203,// 82 PAY  79 

    0xa7eecf63,// 83 PAY  80 

    0x3afac131,// 84 PAY  81 

    0x1a03e452,// 85 PAY  82 

    0xa4631156,// 86 PAY  83 

    0x1b3f3aa7,// 87 PAY  84 

    0x7ec09be4,// 88 PAY  85 

    0x1d4e7799,// 89 PAY  86 

    0x2a8ecde9,// 90 PAY  87 

    0x1160be04,// 91 PAY  88 

    0xb1e2bd31,// 92 PAY  89 

    0xa01c87e2,// 93 PAY  90 

    0x78cfa6e3,// 94 PAY  91 

    0xb97bc8bc,// 95 PAY  92 

    0x720e038c,// 96 PAY  93 

    0xc99294ca,// 97 PAY  94 

    0x67254191,// 98 PAY  95 

    0x6489babc,// 99 PAY  96 

    0x9620e662,// 100 PAY  97 

    0xbffd79d9,// 101 PAY  98 

    0xab8628c2,// 102 PAY  99 

    0x5220916b,// 103 PAY 100 

    0xf9fe2743,// 104 PAY 101 

    0x692ad1dd,// 105 PAY 102 

    0x1f6178d1,// 106 PAY 103 

    0x1f0b6d47,// 107 PAY 104 

    0x9f312e92,// 108 PAY 105 

    0xf3901249,// 109 PAY 106 

    0x22baf82b,// 110 PAY 107 

    0x70bfb14f,// 111 PAY 108 

    0x64c3eaf0,// 112 PAY 109 

    0x932e3998,// 113 PAY 110 

    0x75d876fe,// 114 PAY 111 

    0xabce4c79,// 115 PAY 112 

    0xbd6f503e,// 116 PAY 113 

    0x4d564b7b,// 117 PAY 114 

    0x240baa53,// 118 PAY 115 

    0xceadad08,// 119 PAY 116 

    0x183b01fe,// 120 PAY 117 

    0xeb1d0192,// 121 PAY 118 

    0xb5a10c6f,// 122 PAY 119 

    0x493a67d0,// 123 PAY 120 

    0x5f65b62b,// 124 PAY 121 

    0x43debf57,// 125 PAY 122 

    0x389e5285,// 126 PAY 123 

    0xc573eb35,// 127 PAY 124 

    0x0d33d5ae,// 128 PAY 125 

    0x7162d489,// 129 PAY 126 

    0xebe873bb,// 130 PAY 127 

    0x5b8d84ba,// 131 PAY 128 

    0x27e72bea,// 132 PAY 129 

    0xb2fd3fe1,// 133 PAY 130 

    0xa663ffab,// 134 PAY 131 

    0x63c9fbca,// 135 PAY 132 

    0x154d248e,// 136 PAY 133 

    0xeca9319d,// 137 PAY 134 

    0xf326b6b3,// 138 PAY 135 

    0x59defe6e,// 139 PAY 136 

    0x9f0fc344,// 140 PAY 137 

    0xc4efcb1f,// 141 PAY 138 

    0x1737d976,// 142 PAY 139 

    0xaebf1bd3,// 143 PAY 140 

    0x3524fb2e,// 144 PAY 141 

    0x1cc42099,// 145 PAY 142 

    0x3c734213,// 146 PAY 143 

    0x64e5c4cb,// 147 PAY 144 

    0xe25d448a,// 148 PAY 145 

    0x3fea48a3,// 149 PAY 146 

    0x47ee5f75,// 150 PAY 147 

    0x57859dc0,// 151 PAY 148 

    0x153c3701,// 152 PAY 149 

    0xe0a75ee1,// 153 PAY 150 

    0x44fec166,// 154 PAY 151 

    0x6f8c97e0,// 155 PAY 152 

    0xcb41f179,// 156 PAY 153 

    0xf331a3f8,// 157 PAY 154 

    0x4781ac6f,// 158 PAY 155 

    0x55f66936,// 159 PAY 156 

    0xb5c9f11a,// 160 PAY 157 

    0x37e476ff,// 161 PAY 158 

    0x765de2fd,// 162 PAY 159 

    0x1d9200cf,// 163 PAY 160 

    0xd4470cdc,// 164 PAY 161 

    0x1a96cd4e,// 165 PAY 162 

    0x848b0dba,// 166 PAY 163 

    0x159dab3a,// 167 PAY 164 

    0x5810fd2c,// 168 PAY 165 

    0x0db1af08,// 169 PAY 166 

    0x6eb5ac9d,// 170 PAY 167 

    0x08338e52,// 171 PAY 168 

    0x29c8a957,// 172 PAY 169 

    0x15c70bb7,// 173 PAY 170 

    0x2e6137a0,// 174 PAY 171 

    0xe5f1d76e,// 175 PAY 172 

    0x3f3ff384,// 176 PAY 173 

    0xd1c4282a,// 177 PAY 174 

    0x86fa97a6,// 178 PAY 175 

    0x6ec9e4d5,// 179 PAY 176 

    0x2fcde2c7,// 180 PAY 177 

    0xd63825ae,// 181 PAY 178 

    0xe144d865,// 182 PAY 179 

    0x3ccaaffb,// 183 PAY 180 

    0x516d22e8,// 184 PAY 181 

    0x089937a6,// 185 PAY 182 

    0x538f2595,// 186 PAY 183 

    0x08e4fb63,// 187 PAY 184 

    0x51bf9e05,// 188 PAY 185 

    0xda0f56a1,// 189 PAY 186 

    0xea519ecc,// 190 PAY 187 

    0x087906ad,// 191 PAY 188 

    0xea3d12bc,// 192 PAY 189 

    0x70ca5888,// 193 PAY 190 

    0x6d36bd46,// 194 PAY 191 

    0x6cea6842,// 195 PAY 192 

    0xc823abb3,// 196 PAY 193 

    0x7c83710f,// 197 PAY 194 

    0x02de8f91,// 198 PAY 195 

    0xb8ae1f23,// 199 PAY 196 

    0x918cb028,// 200 PAY 197 

    0xb8cbcfac,// 201 PAY 198 

    0xa3590257,// 202 PAY 199 

    0x7d1c2004,// 203 PAY 200 

    0x325a35d6,// 204 PAY 201 

    0x7c2de067,// 205 PAY 202 

    0xfe963efe,// 206 PAY 203 

    0xb65a4914,// 207 PAY 204 

    0x434ee227,// 208 PAY 205 

    0x46f12563,// 209 PAY 206 

    0xbe5f21d5,// 210 PAY 207 

    0x0e1a8369,// 211 PAY 208 

    0x7a71772c,// 212 PAY 209 

    0x43222dac,// 213 PAY 210 

    0x4e292451,// 214 PAY 211 

    0x53db27ce,// 215 PAY 212 

    0xc16e03d1,// 216 PAY 213 

    0x63bc79c8,// 217 PAY 214 

    0x97447065,// 218 PAY 215 

    0x0e031481,// 219 PAY 216 

    0xf01952c6,// 220 PAY 217 

    0x00a59181,// 221 PAY 218 

    0xba7d2808,// 222 PAY 219 

    0x407396fe,// 223 PAY 220 

    0xf7faca1b,// 224 PAY 221 

    0xc680ff61,// 225 PAY 222 

    0x15fb9018,// 226 PAY 223 

    0x40d2ceba,// 227 PAY 224 

    0x4add45d0,// 228 PAY 225 

    0xa83ab5ef,// 229 PAY 226 

    0xa300b793,// 230 PAY 227 

    0x932978ea,// 231 PAY 228 

    0x9adb5d39,// 232 PAY 229 

    0xe2602e43,// 233 PAY 230 

    0xbc36e14a,// 234 PAY 231 

    0x0e3af180,// 235 PAY 232 

    0x44028efb,// 236 PAY 233 

    0x781603eb,// 237 PAY 234 

    0x350ceb92,// 238 PAY 235 

    0x1450705b,// 239 PAY 236 

    0xadd38244,// 240 PAY 237 

    0xa412a1e0,// 241 PAY 238 

    0xef84fa8a,// 242 PAY 239 

    0x69bd5af0,// 243 PAY 240 

    0xa664535d,// 244 PAY 241 

    0xf48cae14,// 245 PAY 242 

    0x108a8d5a,// 246 PAY 243 

    0x7c776df5,// 247 PAY 244 

    0x35a61012,// 248 PAY 245 

    0x7b92c9fd,// 249 PAY 246 

    0x2ccd1283,// 250 PAY 247 

    0xb40e5c6c,// 251 PAY 248 

    0x8b314a14,// 252 PAY 249 

    0x922c00f3,// 253 PAY 250 

    0x8a16a96f,// 254 PAY 251 

    0xbfed4183,// 255 PAY 252 

    0x951211d0,// 256 PAY 253 

    0x795bb441,// 257 PAY 254 

    0xdbbd3cd1,// 258 PAY 255 

    0x838c4be5,// 259 PAY 256 

    0x055da33c,// 260 PAY 257 

    0x7f220c7b,// 261 PAY 258 

    0x9feab058,// 262 PAY 259 

    0x7a448178,// 263 PAY 260 

    0xa3ddb1f7,// 264 PAY 261 

    0xcc953e51,// 265 PAY 262 

    0xe5b8813f,// 266 PAY 263 

    0xe83069ad,// 267 PAY 264 

    0x6c32ac74,// 268 PAY 265 

    0xfe659c8d,// 269 PAY 266 

    0x071fb86f,// 270 PAY 267 

    0xf75bed8d,// 271 PAY 268 

    0xf39c869d,// 272 PAY 269 

    0x153a2bce,// 273 PAY 270 

    0xa4b9bf26,// 274 PAY 271 

    0x2c72d362,// 275 PAY 272 

    0x6f38acc4,// 276 PAY 273 

    0xcce02ceb,// 277 PAY 274 

    0x22b4c356,// 278 PAY 275 

    0xcae1bfa3,// 279 PAY 276 

    0x10312665,// 280 PAY 277 

    0xaccfd1cf,// 281 PAY 278 

    0x40868523,// 282 PAY 279 

    0x1a2dae3d,// 283 PAY 280 

    0xd39dcda8,// 284 PAY 281 

    0x4fbdc83a,// 285 PAY 282 

    0x9074a77d,// 286 PAY 283 

    0xc269c51a,// 287 PAY 284 

    0xe4cbed97,// 288 PAY 285 

    0x432ef1ad,// 289 PAY 286 

    0x4dbecd01,// 290 PAY 287 

    0xc95b15d2,// 291 PAY 288 

    0x8c437606,// 292 PAY 289 

    0xaaad52e2,// 293 PAY 290 

    0x8a68fecd,// 294 PAY 291 

    0x51bebe1a,// 295 PAY 292 

    0xee75c0ce,// 296 PAY 293 

    0xf3e26442,// 297 PAY 294 

    0x7c46eed0,// 298 PAY 295 

    0x9566db09,// 299 PAY 296 

    0xf6563858,// 300 PAY 297 

    0x51d22f9f,// 301 PAY 298 

    0x618a4ac3,// 302 PAY 299 

    0x237665ad,// 303 PAY 300 

    0xed9fbfd3,// 304 PAY 301 

    0xaa84a84f,// 305 PAY 302 

    0x3648fc19,// 306 PAY 303 

    0x5c90411d,// 307 PAY 304 

    0x1406526b,// 308 PAY 305 

    0xc26462a4,// 309 PAY 306 

    0x881b5048,// 310 PAY 307 

    0x83613b26,// 311 PAY 308 

    0x800d5718,// 312 PAY 309 

    0x763dce69,// 313 PAY 310 

    0x4ed185b5,// 314 PAY 311 

    0x4e61ecfc,// 315 PAY 312 

    0xf2a0b9ef,// 316 PAY 313 

    0x5acbaee0,// 317 PAY 314 

    0xb76345bf,// 318 PAY 315 

    0x0bc7d42e,// 319 PAY 316 

    0x9bfe687f,// 320 PAY 317 

    0xbac78eea,// 321 PAY 318 

    0xe278a498,// 322 PAY 319 

    0x537e1523,// 323 PAY 320 

    0x870e4947,// 324 PAY 321 

    0xed26e2da,// 325 PAY 322 

    0x49730cfe,// 326 PAY 323 

    0x45c0ff75,// 327 PAY 324 

    0x6fda5188,// 328 PAY 325 

    0xb8ce1d16,// 329 PAY 326 

    0xcfbd4c58,// 330 PAY 327 

    0x579e2371,// 331 PAY 328 

    0xf3d3a13f,// 332 PAY 329 

    0x1f182632,// 333 PAY 330 

    0x8c53293b,// 334 PAY 331 

    0xd46969d4,// 335 PAY 332 

    0x3474b4a2,// 336 PAY 333 

    0x6982c76e,// 337 PAY 334 

    0x4da85e77,// 338 PAY 335 

    0x6fef04a4,// 339 PAY 336 

    0x958a7716,// 340 PAY 337 

    0x3f2e0614,// 341 PAY 338 

    0xefc31558,// 342 PAY 339 

    0xbe96babb,// 343 PAY 340 

    0x575b007c,// 344 PAY 341 

    0xdd000000,// 345 PAY 342 

/// HASH is  12 bytes 

    0xf3d3a13f,// 346 HSH   1 

    0x1f182632,// 347 HSH   2 

    0x8c53293b,// 348 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 214 

/// STA pkt_idx        : 7 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xfa 

    0x001cfad6 // 349 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt36_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 268 words. 

/// BDA size     is 1065 (0x429) 

/// BDA id       is 0xda48 

    0x0429da48,// 3 BDA   1 

/// PAY Generic Data size   : 1065 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xe548d3b6,// 4 PAY   1 

    0xa2d438e0,// 5 PAY   2 

    0x4b5cc796,// 6 PAY   3 

    0x87cea418,// 7 PAY   4 

    0x4eff9eb5,// 8 PAY   5 

    0xe58791ac,// 9 PAY   6 

    0x5f05b81d,// 10 PAY   7 

    0x173a6c82,// 11 PAY   8 

    0x54f64565,// 12 PAY   9 

    0xc293c25d,// 13 PAY  10 

    0x7c314b3f,// 14 PAY  11 

    0x437bea6b,// 15 PAY  12 

    0x0bbcc469,// 16 PAY  13 

    0x4155a75b,// 17 PAY  14 

    0x8dc65379,// 18 PAY  15 

    0x66a7c086,// 19 PAY  16 

    0x3f1b43b8,// 20 PAY  17 

    0x6f177c6e,// 21 PAY  18 

    0x9ecc0975,// 22 PAY  19 

    0xd10e05f0,// 23 PAY  20 

    0x01243dce,// 24 PAY  21 

    0x1cc1fc80,// 25 PAY  22 

    0xc47f173b,// 26 PAY  23 

    0x72f46b1b,// 27 PAY  24 

    0xfe98a33d,// 28 PAY  25 

    0x18c757d2,// 29 PAY  26 

    0xd0a46c03,// 30 PAY  27 

    0xb5d74ab5,// 31 PAY  28 

    0x63a52499,// 32 PAY  29 

    0xabfabac6,// 33 PAY  30 

    0x8007cd56,// 34 PAY  31 

    0x288eb3d4,// 35 PAY  32 

    0xee175dd8,// 36 PAY  33 

    0x0258e374,// 37 PAY  34 

    0xe462cf8c,// 38 PAY  35 

    0x14b747aa,// 39 PAY  36 

    0x4c822dad,// 40 PAY  37 

    0x3a642f80,// 41 PAY  38 

    0x73d6b8b2,// 42 PAY  39 

    0x1c78800b,// 43 PAY  40 

    0xd08c51e4,// 44 PAY  41 

    0xf9087765,// 45 PAY  42 

    0xc2da99b2,// 46 PAY  43 

    0xb11ad5cb,// 47 PAY  44 

    0xb4b16c7e,// 48 PAY  45 

    0x95cc4e2c,// 49 PAY  46 

    0x2c60cf80,// 50 PAY  47 

    0x22445a81,// 51 PAY  48 

    0x7dbfb0ee,// 52 PAY  49 

    0x223fcf53,// 53 PAY  50 

    0x851b00aa,// 54 PAY  51 

    0xa4e66d33,// 55 PAY  52 

    0x2eb69f53,// 56 PAY  53 

    0xcb6f8817,// 57 PAY  54 

    0x581c1330,// 58 PAY  55 

    0x9474c778,// 59 PAY  56 

    0xe06b47d7,// 60 PAY  57 

    0x1c26b797,// 61 PAY  58 

    0xce4f32c7,// 62 PAY  59 

    0x5569c16e,// 63 PAY  60 

    0xac66ccd8,// 64 PAY  61 

    0x25e6cb43,// 65 PAY  62 

    0x5f09f622,// 66 PAY  63 

    0xe23356a0,// 67 PAY  64 

    0xf6cb9db2,// 68 PAY  65 

    0x5123f992,// 69 PAY  66 

    0x5efd03ce,// 70 PAY  67 

    0xb4607222,// 71 PAY  68 

    0x5b915b65,// 72 PAY  69 

    0x68ebaca3,// 73 PAY  70 

    0x4ae7efae,// 74 PAY  71 

    0x466ac891,// 75 PAY  72 

    0x0981550a,// 76 PAY  73 

    0x97d28bfb,// 77 PAY  74 

    0x5421b0ba,// 78 PAY  75 

    0x9a985b00,// 79 PAY  76 

    0x92b6f676,// 80 PAY  77 

    0x7d201e87,// 81 PAY  78 

    0x68d94a9c,// 82 PAY  79 

    0x6a482faf,// 83 PAY  80 

    0xe7a04a31,// 84 PAY  81 

    0x7f60d6ac,// 85 PAY  82 

    0x7e58a854,// 86 PAY  83 

    0xf47e6927,// 87 PAY  84 

    0x55850a40,// 88 PAY  85 

    0xf911e6b2,// 89 PAY  86 

    0x7ceda1f2,// 90 PAY  87 

    0x83df8bad,// 91 PAY  88 

    0xe09eaeac,// 92 PAY  89 

    0x14fcc4b9,// 93 PAY  90 

    0xdb5d68ed,// 94 PAY  91 

    0x457270a5,// 95 PAY  92 

    0x289543cc,// 96 PAY  93 

    0xf0d68512,// 97 PAY  94 

    0xf8d70f96,// 98 PAY  95 

    0xcc0f7d53,// 99 PAY  96 

    0x4c42c4e0,// 100 PAY  97 

    0x68709158,// 101 PAY  98 

    0xa7faaa6f,// 102 PAY  99 

    0xe3e11682,// 103 PAY 100 

    0x6ce6f122,// 104 PAY 101 

    0x5fe448df,// 105 PAY 102 

    0x2ed558f0,// 106 PAY 103 

    0x1c582ac9,// 107 PAY 104 

    0x9b26337d,// 108 PAY 105 

    0x7ca6877c,// 109 PAY 106 

    0xeb519d8a,// 110 PAY 107 

    0xa98a6dcd,// 111 PAY 108 

    0x44546879,// 112 PAY 109 

    0x2b853595,// 113 PAY 110 

    0x62363c2d,// 114 PAY 111 

    0x662997dc,// 115 PAY 112 

    0x5d04c0bb,// 116 PAY 113 

    0xd15e1838,// 117 PAY 114 

    0xac2c6067,// 118 PAY 115 

    0xc860719a,// 119 PAY 116 

    0x94436586,// 120 PAY 117 

    0xe11eab69,// 121 PAY 118 

    0xa1736c83,// 122 PAY 119 

    0x6f543f8e,// 123 PAY 120 

    0xfe3b06b7,// 124 PAY 121 

    0x2753ccc9,// 125 PAY 122 

    0x1ed6260b,// 126 PAY 123 

    0xf3a059dd,// 127 PAY 124 

    0xbc51a405,// 128 PAY 125 

    0x316affc2,// 129 PAY 126 

    0x2d777d49,// 130 PAY 127 

    0x20219c21,// 131 PAY 128 

    0x8e48a16e,// 132 PAY 129 

    0xb8a1f739,// 133 PAY 130 

    0x63cfd480,// 134 PAY 131 

    0x2afdb21b,// 135 PAY 132 

    0x52770be1,// 136 PAY 133 

    0x55fe5403,// 137 PAY 134 

    0xf790fda9,// 138 PAY 135 

    0xc74d3b2d,// 139 PAY 136 

    0x5c6f829a,// 140 PAY 137 

    0x64eb4c61,// 141 PAY 138 

    0x1d0f38b5,// 142 PAY 139 

    0x7ab0523c,// 143 PAY 140 

    0xd4143530,// 144 PAY 141 

    0x42f8899a,// 145 PAY 142 

    0x78b56e89,// 146 PAY 143 

    0xb139b0c1,// 147 PAY 144 

    0x70f1eb90,// 148 PAY 145 

    0x9956d31b,// 149 PAY 146 

    0x20e48e35,// 150 PAY 147 

    0xbb65d496,// 151 PAY 148 

    0x2f855c44,// 152 PAY 149 

    0x30f9e957,// 153 PAY 150 

    0x64c70319,// 154 PAY 151 

    0x9d309894,// 155 PAY 152 

    0x793fcd17,// 156 PAY 153 

    0x479b7857,// 157 PAY 154 

    0x9e6454e4,// 158 PAY 155 

    0x4bd08c98,// 159 PAY 156 

    0xee3d5558,// 160 PAY 157 

    0xc5e7058a,// 161 PAY 158 

    0x59d8425e,// 162 PAY 159 

    0x314ef0f5,// 163 PAY 160 

    0xd8655f73,// 164 PAY 161 

    0x7a91b4e6,// 165 PAY 162 

    0x1158b5f6,// 166 PAY 163 

    0x9f4ad3f3,// 167 PAY 164 

    0x0592ad47,// 168 PAY 165 

    0x2540efee,// 169 PAY 166 

    0xbcb17a77,// 170 PAY 167 

    0x70fcc651,// 171 PAY 168 

    0x0946df42,// 172 PAY 169 

    0xb828a4d5,// 173 PAY 170 

    0xa94cc6b6,// 174 PAY 171 

    0xb5f2eaaa,// 175 PAY 172 

    0xeea47f98,// 176 PAY 173 

    0x1864447c,// 177 PAY 174 

    0x6c52202a,// 178 PAY 175 

    0xfa9774f1,// 179 PAY 176 

    0x306c6a6e,// 180 PAY 177 

    0x1d5c23c6,// 181 PAY 178 

    0x1b979186,// 182 PAY 179 

    0xf6af1ef0,// 183 PAY 180 

    0xfd839aa4,// 184 PAY 181 

    0xaf8c03b0,// 185 PAY 182 

    0xd66e3f74,// 186 PAY 183 

    0x59d96b20,// 187 PAY 184 

    0x8c815dae,// 188 PAY 185 

    0xb652bd56,// 189 PAY 186 

    0xace4fb2c,// 190 PAY 187 

    0xaf92bdab,// 191 PAY 188 

    0x92949cd8,// 192 PAY 189 

    0xffe5b3d6,// 193 PAY 190 

    0xc81fdba3,// 194 PAY 191 

    0xa72fef16,// 195 PAY 192 

    0x301a2011,// 196 PAY 193 

    0xdf157854,// 197 PAY 194 

    0x72f7c331,// 198 PAY 195 

    0xc220ff8a,// 199 PAY 196 

    0xc0f9d67e,// 200 PAY 197 

    0xd572ce5f,// 201 PAY 198 

    0x8b22933d,// 202 PAY 199 

    0x8c930082,// 203 PAY 200 

    0xbcaa9a51,// 204 PAY 201 

    0x31859a70,// 205 PAY 202 

    0x6eaf6b87,// 206 PAY 203 

    0x73ab0652,// 207 PAY 204 

    0xce0eb0ba,// 208 PAY 205 

    0x5b3f81fb,// 209 PAY 206 

    0x5b3d3c77,// 210 PAY 207 

    0xbe206c8a,// 211 PAY 208 

    0x9392fe18,// 212 PAY 209 

    0xbdd66538,// 213 PAY 210 

    0xb3a50421,// 214 PAY 211 

    0xadb34218,// 215 PAY 212 

    0x82947629,// 216 PAY 213 

    0xa2a58b03,// 217 PAY 214 

    0x9f86a174,// 218 PAY 215 

    0xff7e249d,// 219 PAY 216 

    0x5bca5076,// 220 PAY 217 

    0x8acc21ce,// 221 PAY 218 

    0xe246c692,// 222 PAY 219 

    0x3cb04808,// 223 PAY 220 

    0xb27b474e,// 224 PAY 221 

    0xd10e2c71,// 225 PAY 222 

    0x2b963a6f,// 226 PAY 223 

    0xbeb985e1,// 227 PAY 224 

    0x667b76ca,// 228 PAY 225 

    0x952a090c,// 229 PAY 226 

    0x88b44bcc,// 230 PAY 227 

    0x6a705571,// 231 PAY 228 

    0x0b2b27df,// 232 PAY 229 

    0xcacfad69,// 233 PAY 230 

    0xda9fb245,// 234 PAY 231 

    0x14fa8cd8,// 235 PAY 232 

    0x39927342,// 236 PAY 233 

    0x0f054867,// 237 PAY 234 

    0x79b9e119,// 238 PAY 235 

    0x4cd66fd6,// 239 PAY 236 

    0x11839461,// 240 PAY 237 

    0xe480b3c4,// 241 PAY 238 

    0x2f792723,// 242 PAY 239 

    0x9f478d4d,// 243 PAY 240 

    0x24770dff,// 244 PAY 241 

    0xeb37641b,// 245 PAY 242 

    0xca0326e9,// 246 PAY 243 

    0x096113ca,// 247 PAY 244 

    0x046aa317,// 248 PAY 245 

    0x16714986,// 249 PAY 246 

    0x9eb79df3,// 250 PAY 247 

    0x351a163a,// 251 PAY 248 

    0x4fca6f16,// 252 PAY 249 

    0x9d2eaab2,// 253 PAY 250 

    0xcfcd3f17,// 254 PAY 251 

    0x4eee3525,// 255 PAY 252 

    0x7c8736a0,// 256 PAY 253 

    0x2122c969,// 257 PAY 254 

    0xe46132fd,// 258 PAY 255 

    0xa9f7b8b4,// 259 PAY 256 

    0xfffb6202,// 260 PAY 257 

    0x5b81096d,// 261 PAY 258 

    0x5e9aae75,// 262 PAY 259 

    0x904cfefb,// 263 PAY 260 

    0x4a85e84f,// 264 PAY 261 

    0xd90c31af,// 265 PAY 262 

    0x7d59eca0,// 266 PAY 263 

    0x1af9bb89,// 267 PAY 264 

    0xa4901c73,// 268 PAY 265 

    0x59838242,// 269 PAY 266 

    0xe9000000,// 270 PAY 267 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 254 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x32 

    0x03f932b2 // 271 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt37_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 270 words. 

/// BDA size     is 1075 (0x433) 

/// BDA id       is 0x1608 

    0x04331608,// 3 BDA   1 

/// PAY Generic Data size   : 1075 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf97f79c3,// 4 PAY   1 

    0x9adb5ce6,// 5 PAY   2 

    0xf8462229,// 6 PAY   3 

    0xb824a34d,// 7 PAY   4 

    0xab448bdd,// 8 PAY   5 

    0x32e64083,// 9 PAY   6 

    0x171a8ce5,// 10 PAY   7 

    0x424639fc,// 11 PAY   8 

    0x40774957,// 12 PAY   9 

    0xfd955496,// 13 PAY  10 

    0x0afe9c20,// 14 PAY  11 

    0x0f7a00f0,// 15 PAY  12 

    0x48681c0e,// 16 PAY  13 

    0xe7c8a83e,// 17 PAY  14 

    0xfb9ded83,// 18 PAY  15 

    0x22d98b60,// 19 PAY  16 

    0x1365282e,// 20 PAY  17 

    0x85d99589,// 21 PAY  18 

    0x278a2977,// 22 PAY  19 

    0x01eccc25,// 23 PAY  20 

    0xc0e4bc7a,// 24 PAY  21 

    0xb40d8273,// 25 PAY  22 

    0xa98ea55f,// 26 PAY  23 

    0x6ee49f9d,// 27 PAY  24 

    0x35636fc4,// 28 PAY  25 

    0xc70211eb,// 29 PAY  26 

    0x9d7bfe7b,// 30 PAY  27 

    0x8d90e5ba,// 31 PAY  28 

    0x815f1e49,// 32 PAY  29 

    0x063319b7,// 33 PAY  30 

    0xb733d6c4,// 34 PAY  31 

    0x33488da1,// 35 PAY  32 

    0xa1a917d5,// 36 PAY  33 

    0x5d711cf9,// 37 PAY  34 

    0x753c4ab2,// 38 PAY  35 

    0xbba2b9ae,// 39 PAY  36 

    0xcef0089c,// 40 PAY  37 

    0x6abefe07,// 41 PAY  38 

    0xdfe13b9e,// 42 PAY  39 

    0xb742fb9e,// 43 PAY  40 

    0xf00b6b31,// 44 PAY  41 

    0x548a7e57,// 45 PAY  42 

    0x6bf31907,// 46 PAY  43 

    0x28bb9ab4,// 47 PAY  44 

    0xf73c8a3a,// 48 PAY  45 

    0x4aae3251,// 49 PAY  46 

    0x9a855ec8,// 50 PAY  47 

    0xd2d16e3c,// 51 PAY  48 

    0xc4f99fc1,// 52 PAY  49 

    0xc1b4bf1e,// 53 PAY  50 

    0x696446e6,// 54 PAY  51 

    0x29f5e318,// 55 PAY  52 

    0xd596aafc,// 56 PAY  53 

    0x62b3b7ea,// 57 PAY  54 

    0x6b44107d,// 58 PAY  55 

    0x8ea53ec3,// 59 PAY  56 

    0xe8867a81,// 60 PAY  57 

    0xaf1376b1,// 61 PAY  58 

    0x95ea39cd,// 62 PAY  59 

    0xc935da6b,// 63 PAY  60 

    0x61440465,// 64 PAY  61 

    0xf66a4954,// 65 PAY  62 

    0xbbd24a69,// 66 PAY  63 

    0x1c43c1a6,// 67 PAY  64 

    0x5885e0ff,// 68 PAY  65 

    0xa591fc96,// 69 PAY  66 

    0xf0e2ef1a,// 70 PAY  67 

    0x29f0743a,// 71 PAY  68 

    0xb6eefd84,// 72 PAY  69 

    0xf9a7b7cd,// 73 PAY  70 

    0x35e40b0e,// 74 PAY  71 

    0x91a600ac,// 75 PAY  72 

    0x369f3b28,// 76 PAY  73 

    0xf1cf3b47,// 77 PAY  74 

    0xc8a88891,// 78 PAY  75 

    0x10e2cbda,// 79 PAY  76 

    0x37abf5b1,// 80 PAY  77 

    0xf0d06bad,// 81 PAY  78 

    0xef202886,// 82 PAY  79 

    0xf89aa522,// 83 PAY  80 

    0x1eccaed9,// 84 PAY  81 

    0x34998c38,// 85 PAY  82 

    0x0e4b076e,// 86 PAY  83 

    0x5b06f4cb,// 87 PAY  84 

    0xa67db84c,// 88 PAY  85 

    0xd1cca118,// 89 PAY  86 

    0xca567a5a,// 90 PAY  87 

    0xc6686087,// 91 PAY  88 

    0x176eb5d5,// 92 PAY  89 

    0x2a7824fb,// 93 PAY  90 

    0x111683ed,// 94 PAY  91 

    0xeb4af85e,// 95 PAY  92 

    0xaa164402,// 96 PAY  93 

    0x24e4ef60,// 97 PAY  94 

    0x5a7d61df,// 98 PAY  95 

    0xe84dc0da,// 99 PAY  96 

    0x5f837493,// 100 PAY  97 

    0xef05b2e5,// 101 PAY  98 

    0xda4f0a3f,// 102 PAY  99 

    0x8904d05b,// 103 PAY 100 

    0x1459c9e9,// 104 PAY 101 

    0x0c381d21,// 105 PAY 102 

    0x25388783,// 106 PAY 103 

    0x521b4483,// 107 PAY 104 

    0xba30c396,// 108 PAY 105 

    0xae08b5f2,// 109 PAY 106 

    0xce9e74f6,// 110 PAY 107 

    0x02cf3cbc,// 111 PAY 108 

    0x87a4aaa0,// 112 PAY 109 

    0xe4f1f7aa,// 113 PAY 110 

    0x48762cb7,// 114 PAY 111 

    0xb3900a83,// 115 PAY 112 

    0x9854d1bf,// 116 PAY 113 

    0x9a27d240,// 117 PAY 114 

    0x09b783c9,// 118 PAY 115 

    0x0f14b04d,// 119 PAY 116 

    0x7408b742,// 120 PAY 117 

    0x0eead169,// 121 PAY 118 

    0x00c8dd7b,// 122 PAY 119 

    0x3d59bd9e,// 123 PAY 120 

    0xe15267dc,// 124 PAY 121 

    0x172585c7,// 125 PAY 122 

    0x613b0861,// 126 PAY 123 

    0xc31ca00e,// 127 PAY 124 

    0x0a8a9fd7,// 128 PAY 125 

    0x172dc63b,// 129 PAY 126 

    0x93cddb1e,// 130 PAY 127 

    0xf6ed4f6e,// 131 PAY 128 

    0x7c3a6115,// 132 PAY 129 

    0x349a1361,// 133 PAY 130 

    0x3006edce,// 134 PAY 131 

    0x29c284f2,// 135 PAY 132 

    0xc8a447f6,// 136 PAY 133 

    0xf06e9ba2,// 137 PAY 134 

    0x6b153150,// 138 PAY 135 

    0x5371629e,// 139 PAY 136 

    0xa65629a8,// 140 PAY 137 

    0x844f832a,// 141 PAY 138 

    0xebe292d3,// 142 PAY 139 

    0xee7c396b,// 143 PAY 140 

    0x94f80dbe,// 144 PAY 141 

    0xdfe33427,// 145 PAY 142 

    0x8c36c896,// 146 PAY 143 

    0x3041b35e,// 147 PAY 144 

    0x929bf795,// 148 PAY 145 

    0x61fd77cb,// 149 PAY 146 

    0x38833180,// 150 PAY 147 

    0xe66b5a70,// 151 PAY 148 

    0x957cc3d5,// 152 PAY 149 

    0x9a704bc5,// 153 PAY 150 

    0x2b25cc98,// 154 PAY 151 

    0xb85508fd,// 155 PAY 152 

    0x00c1ece2,// 156 PAY 153 

    0x820b1e58,// 157 PAY 154 

    0x920b9a55,// 158 PAY 155 

    0x5ec779dd,// 159 PAY 156 

    0xbcf50612,// 160 PAY 157 

    0xf849e0c5,// 161 PAY 158 

    0x28711d2c,// 162 PAY 159 

    0x13cf87d1,// 163 PAY 160 

    0x65b0d07b,// 164 PAY 161 

    0x88587bd5,// 165 PAY 162 

    0x19e896a4,// 166 PAY 163 

    0x638ea73e,// 167 PAY 164 

    0x523fd6a2,// 168 PAY 165 

    0x5693c05e,// 169 PAY 166 

    0x97527c78,// 170 PAY 167 

    0xe2aa1bca,// 171 PAY 168 

    0xb758217e,// 172 PAY 169 

    0x07a93dd2,// 173 PAY 170 

    0xee6fbff8,// 174 PAY 171 

    0x1f72e0a8,// 175 PAY 172 

    0x8e752f25,// 176 PAY 173 

    0xde4a22b3,// 177 PAY 174 

    0xa3822471,// 178 PAY 175 

    0x8599a777,// 179 PAY 176 

    0x0fafd115,// 180 PAY 177 

    0xd3cba904,// 181 PAY 178 

    0xbe359855,// 182 PAY 179 

    0xed6c0661,// 183 PAY 180 

    0xa6a77bd4,// 184 PAY 181 

    0xdce8a201,// 185 PAY 182 

    0x69d01330,// 186 PAY 183 

    0x9a303913,// 187 PAY 184 

    0xf62cf56b,// 188 PAY 185 

    0x694ef12e,// 189 PAY 186 

    0x94b632b0,// 190 PAY 187 

    0x9a2e78c6,// 191 PAY 188 

    0xe5243f58,// 192 PAY 189 

    0x1f9cec72,// 193 PAY 190 

    0xe28cf8bc,// 194 PAY 191 

    0x3531e3a3,// 195 PAY 192 

    0x841a06ce,// 196 PAY 193 

    0x355f7a04,// 197 PAY 194 

    0x58503669,// 198 PAY 195 

    0x4339800f,// 199 PAY 196 

    0x6f2ff798,// 200 PAY 197 

    0xff57cd1a,// 201 PAY 198 

    0x4351297e,// 202 PAY 199 

    0xfff22ef6,// 203 PAY 200 

    0x0640c918,// 204 PAY 201 

    0x60c11565,// 205 PAY 202 

    0xc894a7bd,// 206 PAY 203 

    0x0feb8a76,// 207 PAY 204 

    0x84d49e0e,// 208 PAY 205 

    0x127fe61a,// 209 PAY 206 

    0x7230b18f,// 210 PAY 207 

    0xb6deda93,// 211 PAY 208 

    0x7df72b6f,// 212 PAY 209 

    0x6b90a3ba,// 213 PAY 210 

    0xd6632d47,// 214 PAY 211 

    0x51b7360c,// 215 PAY 212 

    0x1b7b87fd,// 216 PAY 213 

    0xce8b0801,// 217 PAY 214 

    0x57dafd24,// 218 PAY 215 

    0xcc714fc9,// 219 PAY 216 

    0x6d64cd8a,// 220 PAY 217 

    0xe9eb607a,// 221 PAY 218 

    0x178864ca,// 222 PAY 219 

    0x2411ac8a,// 223 PAY 220 

    0xe76eb78c,// 224 PAY 221 

    0x5f4653de,// 225 PAY 222 

    0xcba7b8db,// 226 PAY 223 

    0x40026596,// 227 PAY 224 

    0x1ea054b1,// 228 PAY 225 

    0xab37a807,// 229 PAY 226 

    0xf2565de4,// 230 PAY 227 

    0x9e6b04f7,// 231 PAY 228 

    0x8c51ba18,// 232 PAY 229 

    0xef07ae36,// 233 PAY 230 

    0x931b1b47,// 234 PAY 231 

    0x780ad61f,// 235 PAY 232 

    0x3ca8e81e,// 236 PAY 233 

    0xd896b109,// 237 PAY 234 

    0x31f2e7d3,// 238 PAY 235 

    0xf1d907b3,// 239 PAY 236 

    0x81a2ae42,// 240 PAY 237 

    0x7716c74b,// 241 PAY 238 

    0xceddc49b,// 242 PAY 239 

    0xfdb80a10,// 243 PAY 240 

    0x689d2314,// 244 PAY 241 

    0x4e3b0266,// 245 PAY 242 

    0x9cfe6a2d,// 246 PAY 243 

    0xe5897583,// 247 PAY 244 

    0xd7ead58a,// 248 PAY 245 

    0x2f73cf1b,// 249 PAY 246 

    0xa7ef5d49,// 250 PAY 247 

    0x6f72662c,// 251 PAY 248 

    0x558ad1ec,// 252 PAY 249 

    0x00d6382e,// 253 PAY 250 

    0x5b410d82,// 254 PAY 251 

    0x9ddd1a4d,// 255 PAY 252 

    0x4a8f62dc,// 256 PAY 253 

    0xf987801c,// 257 PAY 254 

    0x29789fcd,// 258 PAY 255 

    0x0e9f6a7a,// 259 PAY 256 

    0x4c7717cb,// 260 PAY 257 

    0x6f7e7ff3,// 261 PAY 258 

    0x888f4f55,// 262 PAY 259 

    0x4faecb12,// 263 PAY 260 

    0x8705d013,// 264 PAY 261 

    0xba3adeda,// 265 PAY 262 

    0x232b3ed1,// 266 PAY 263 

    0xd6f63614,// 267 PAY 264 

    0xb433cb1d,// 268 PAY 265 

    0x9a984133,// 269 PAY 266 

    0x1ef7204b,// 270 PAY 267 

    0x3819bb95,// 271 PAY 268 

    0x3dfb1800,// 272 PAY 269 

/// HASH is  4 bytes 

    0x9cfe6a2d,// 273 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 164 

/// STA pkt_idx        : 88 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x0161bea4 // 274 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt38_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 320 words. 

/// BDA size     is 1274 (0x4fa) 

/// BDA id       is 0x1528 

    0x04fa1528,// 3 BDA   1 

/// PAY Generic Data size   : 1274 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x1aa90ea7,// 4 PAY   1 

    0x338743c6,// 5 PAY   2 

    0xa7fa6f1f,// 6 PAY   3 

    0x7e0aeb65,// 7 PAY   4 

    0x1f4ac56b,// 8 PAY   5 

    0x0d3f93f7,// 9 PAY   6 

    0x90f4111a,// 10 PAY   7 

    0x45c3d4ed,// 11 PAY   8 

    0x745e3e69,// 12 PAY   9 

    0x4ed9beae,// 13 PAY  10 

    0x92141ba4,// 14 PAY  11 

    0xd4c32577,// 15 PAY  12 

    0xb561e3c3,// 16 PAY  13 

    0x71b1e631,// 17 PAY  14 

    0x6af3086c,// 18 PAY  15 

    0x08a8a073,// 19 PAY  16 

    0xbbd37a0c,// 20 PAY  17 

    0x477a45dd,// 21 PAY  18 

    0x784d0dba,// 22 PAY  19 

    0x7e5a5585,// 23 PAY  20 

    0xdea1888a,// 24 PAY  21 

    0x1f5ef799,// 25 PAY  22 

    0xc25d8032,// 26 PAY  23 

    0x4c4890b6,// 27 PAY  24 

    0x433015cf,// 28 PAY  25 

    0xa00e1514,// 29 PAY  26 

    0xeb1953a4,// 30 PAY  27 

    0x2c4d764c,// 31 PAY  28 

    0xe2a4d49b,// 32 PAY  29 

    0x4cd1aead,// 33 PAY  30 

    0x2c6dc081,// 34 PAY  31 

    0x614f1755,// 35 PAY  32 

    0x8ee608f2,// 36 PAY  33 

    0x29a0653c,// 37 PAY  34 

    0x93a8f463,// 38 PAY  35 

    0xcaea84e6,// 39 PAY  36 

    0xa6093972,// 40 PAY  37 

    0x4439f146,// 41 PAY  38 

    0xd94ba73e,// 42 PAY  39 

    0x12526217,// 43 PAY  40 

    0x1bad5456,// 44 PAY  41 

    0x6017124f,// 45 PAY  42 

    0x7f1327e0,// 46 PAY  43 

    0x8f9896d9,// 47 PAY  44 

    0xdeaa6193,// 48 PAY  45 

    0x33b2254f,// 49 PAY  46 

    0x6bacd1b3,// 50 PAY  47 

    0x4a2bbdf5,// 51 PAY  48 

    0xbf880d81,// 52 PAY  49 

    0x81d765ec,// 53 PAY  50 

    0xb3b7deb8,// 54 PAY  51 

    0x499907c9,// 55 PAY  52 

    0x6e28b621,// 56 PAY  53 

    0x9b31b02d,// 57 PAY  54 

    0x596cb5e8,// 58 PAY  55 

    0x25ca388b,// 59 PAY  56 

    0xba47902b,// 60 PAY  57 

    0xeacfe727,// 61 PAY  58 

    0xcc33ea7b,// 62 PAY  59 

    0x8f71d4ac,// 63 PAY  60 

    0x42e97985,// 64 PAY  61 

    0xdc5db8ea,// 65 PAY  62 

    0xc91e7d0d,// 66 PAY  63 

    0x54159c12,// 67 PAY  64 

    0x0cb784b8,// 68 PAY  65 

    0x5d2122f3,// 69 PAY  66 

    0xfa4659b4,// 70 PAY  67 

    0xe3473d6f,// 71 PAY  68 

    0x4ffb20f0,// 72 PAY  69 

    0x6b0e4d99,// 73 PAY  70 

    0x1ca390a7,// 74 PAY  71 

    0x9a97f3ea,// 75 PAY  72 

    0x2a36db1c,// 76 PAY  73 

    0x60908c8b,// 77 PAY  74 

    0x25f87166,// 78 PAY  75 

    0xe13c7c7e,// 79 PAY  76 

    0x06c6f672,// 80 PAY  77 

    0xf10e165b,// 81 PAY  78 

    0xc8eef476,// 82 PAY  79 

    0x29a7f778,// 83 PAY  80 

    0x65c83940,// 84 PAY  81 

    0x29f20f29,// 85 PAY  82 

    0xa4666d2f,// 86 PAY  83 

    0x60c4e426,// 87 PAY  84 

    0xfd0bca75,// 88 PAY  85 

    0x0a5e3cc9,// 89 PAY  86 

    0x59bd49de,// 90 PAY  87 

    0xc68cba13,// 91 PAY  88 

    0x848c41b7,// 92 PAY  89 

    0x26fa0409,// 93 PAY  90 

    0x29aea08d,// 94 PAY  91 

    0xeb2bc07e,// 95 PAY  92 

    0x93a83450,// 96 PAY  93 

    0xf2dd7880,// 97 PAY  94 

    0x17f19fee,// 98 PAY  95 

    0x7539ecbd,// 99 PAY  96 

    0x91d663be,// 100 PAY  97 

    0x04b1a6e3,// 101 PAY  98 

    0xf01bb22f,// 102 PAY  99 

    0x5e1a6ba1,// 103 PAY 100 

    0xe2772779,// 104 PAY 101 

    0x5492056c,// 105 PAY 102 

    0xc0093abb,// 106 PAY 103 

    0x083943a1,// 107 PAY 104 

    0xbf595aed,// 108 PAY 105 

    0x7b3a9f73,// 109 PAY 106 

    0x2d7f5972,// 110 PAY 107 

    0x59a81cad,// 111 PAY 108 

    0xddfebd3e,// 112 PAY 109 

    0x954a9754,// 113 PAY 110 

    0x769ac1a5,// 114 PAY 111 

    0xc3777f2b,// 115 PAY 112 

    0xc7cd362d,// 116 PAY 113 

    0xcb422696,// 117 PAY 114 

    0x3ff68dfe,// 118 PAY 115 

    0x92417634,// 119 PAY 116 

    0x7f45db02,// 120 PAY 117 

    0x32b214f9,// 121 PAY 118 

    0xd9b90ca3,// 122 PAY 119 

    0xf82095d0,// 123 PAY 120 

    0xeb2cb33c,// 124 PAY 121 

    0xccfa1620,// 125 PAY 122 

    0x6c1497b1,// 126 PAY 123 

    0x71c3481c,// 127 PAY 124 

    0xcb9e8253,// 128 PAY 125 

    0xad6d2484,// 129 PAY 126 

    0xbc47bbc9,// 130 PAY 127 

    0x9b2f9e5c,// 131 PAY 128 

    0x5e5c4d9d,// 132 PAY 129 

    0xf5151a2e,// 133 PAY 130 

    0x3eaa040a,// 134 PAY 131 

    0x00c27500,// 135 PAY 132 

    0xed4b48d4,// 136 PAY 133 

    0x54801839,// 137 PAY 134 

    0xfb96c6c9,// 138 PAY 135 

    0x23242fc1,// 139 PAY 136 

    0xbcd72c38,// 140 PAY 137 

    0x745d9e80,// 141 PAY 138 

    0xfdc64884,// 142 PAY 139 

    0x17df21fc,// 143 PAY 140 

    0x07279ed9,// 144 PAY 141 

    0x1713e878,// 145 PAY 142 

    0xe6cacdc0,// 146 PAY 143 

    0xfaebbefb,// 147 PAY 144 

    0xc5b8e089,// 148 PAY 145 

    0x136ae343,// 149 PAY 146 

    0x4dd055ea,// 150 PAY 147 

    0x99b5d323,// 151 PAY 148 

    0x0ad611ea,// 152 PAY 149 

    0x53093ed7,// 153 PAY 150 

    0x43d41734,// 154 PAY 151 

    0xc8a5f6fe,// 155 PAY 152 

    0xc531d12b,// 156 PAY 153 

    0xd1c66cef,// 157 PAY 154 

    0x9a1bd474,// 158 PAY 155 

    0xc3e167ce,// 159 PAY 156 

    0x6565d131,// 160 PAY 157 

    0xf7bc01a5,// 161 PAY 158 

    0x31187c66,// 162 PAY 159 

    0x67ff70ae,// 163 PAY 160 

    0x48053762,// 164 PAY 161 

    0x9d1f1b4b,// 165 PAY 162 

    0x11648bd3,// 166 PAY 163 

    0xdaec3509,// 167 PAY 164 

    0xeffa229a,// 168 PAY 165 

    0xe0f85e02,// 169 PAY 166 

    0x9a873a80,// 170 PAY 167 

    0xbe1ad08b,// 171 PAY 168 

    0xa248e14c,// 172 PAY 169 

    0x62929a25,// 173 PAY 170 

    0x10a4ad31,// 174 PAY 171 

    0xb7b1e57d,// 175 PAY 172 

    0x65e72f4b,// 176 PAY 173 

    0xcd178e6f,// 177 PAY 174 

    0x4866ed53,// 178 PAY 175 

    0x5c4dc581,// 179 PAY 176 

    0xdbbcd713,// 180 PAY 177 

    0x8db0ca34,// 181 PAY 178 

    0xe8183f1c,// 182 PAY 179 

    0x1b9a4c3d,// 183 PAY 180 

    0x5f9f617a,// 184 PAY 181 

    0x3e58f76a,// 185 PAY 182 

    0x9b41120e,// 186 PAY 183 

    0x7dd90482,// 187 PAY 184 

    0xbd861bf6,// 188 PAY 185 

    0xeba995a7,// 189 PAY 186 

    0xeb782870,// 190 PAY 187 

    0x8cc61ff2,// 191 PAY 188 

    0x451b823e,// 192 PAY 189 

    0x4fca352e,// 193 PAY 190 

    0x5e70d48b,// 194 PAY 191 

    0xb3655ddb,// 195 PAY 192 

    0x3528bf4e,// 196 PAY 193 

    0xf9ca4e98,// 197 PAY 194 

    0x0e936047,// 198 PAY 195 

    0x2d7135f4,// 199 PAY 196 

    0x5f67073b,// 200 PAY 197 

    0x7ad946de,// 201 PAY 198 

    0x3075fc39,// 202 PAY 199 

    0x636fd6f8,// 203 PAY 200 

    0xc295581e,// 204 PAY 201 

    0x690ba5cc,// 205 PAY 202 

    0x484d9936,// 206 PAY 203 

    0x08b1bad3,// 207 PAY 204 

    0x653680c4,// 208 PAY 205 

    0xfb00d800,// 209 PAY 206 

    0xed547b9d,// 210 PAY 207 

    0x9a6e6ffe,// 211 PAY 208 

    0x96de8fc1,// 212 PAY 209 

    0x68cb3078,// 213 PAY 210 

    0x47d2d94a,// 214 PAY 211 

    0x95fa56f5,// 215 PAY 212 

    0xd636a19f,// 216 PAY 213 

    0xa6b8b32e,// 217 PAY 214 

    0x72b2e884,// 218 PAY 215 

    0xc42893fc,// 219 PAY 216 

    0x9890dfa9,// 220 PAY 217 

    0xbdc6d211,// 221 PAY 218 

    0x1f32ac05,// 222 PAY 219 

    0xdea20b1d,// 223 PAY 220 

    0xab934723,// 224 PAY 221 

    0x4eadda82,// 225 PAY 222 

    0xf59337de,// 226 PAY 223 

    0xd1c1927c,// 227 PAY 224 

    0x1338d7a3,// 228 PAY 225 

    0xf0c60543,// 229 PAY 226 

    0x7b78b6c5,// 230 PAY 227 

    0x8b7f860d,// 231 PAY 228 

    0xed6940d2,// 232 PAY 229 

    0xb131508b,// 233 PAY 230 

    0x08e36f18,// 234 PAY 231 

    0xea478520,// 235 PAY 232 

    0xbc6b1b68,// 236 PAY 233 

    0xf4b9c6a3,// 237 PAY 234 

    0x31d34cae,// 238 PAY 235 

    0x72b2f31d,// 239 PAY 236 

    0xeede18be,// 240 PAY 237 

    0xdabf5d63,// 241 PAY 238 

    0xf4d8a26c,// 242 PAY 239 

    0xea224630,// 243 PAY 240 

    0xbdb9bc71,// 244 PAY 241 

    0xc18b2841,// 245 PAY 242 

    0x07907135,// 246 PAY 243 

    0x70969afc,// 247 PAY 244 

    0x991dca7d,// 248 PAY 245 

    0xf7c5204b,// 249 PAY 246 

    0x46285632,// 250 PAY 247 

    0xaf1c8fbc,// 251 PAY 248 

    0xf6e36aea,// 252 PAY 249 

    0x933b6b8e,// 253 PAY 250 

    0xb6279588,// 254 PAY 251 

    0x77b7ca9c,// 255 PAY 252 

    0x97630cc3,// 256 PAY 253 

    0xe8d467bb,// 257 PAY 254 

    0x4ad639a5,// 258 PAY 255 

    0xf0a5f2f9,// 259 PAY 256 

    0x09f7d15f,// 260 PAY 257 

    0x70727d44,// 261 PAY 258 

    0xf0fb2b9e,// 262 PAY 259 

    0x55bb9b2d,// 263 PAY 260 

    0x455ae800,// 264 PAY 261 

    0x8b0785d2,// 265 PAY 262 

    0xb316452d,// 266 PAY 263 

    0x8fd8e67b,// 267 PAY 264 

    0xb937314b,// 268 PAY 265 

    0xb39bb0d2,// 269 PAY 266 

    0x2568b4ed,// 270 PAY 267 

    0x8fb1fa68,// 271 PAY 268 

    0x3e850355,// 272 PAY 269 

    0xe28754e5,// 273 PAY 270 

    0x313cccb3,// 274 PAY 271 

    0x6276688a,// 275 PAY 272 

    0xfaa7eca1,// 276 PAY 273 

    0x6c73b127,// 277 PAY 274 

    0xaf380472,// 278 PAY 275 

    0x44e4c80e,// 279 PAY 276 

    0xd933986a,// 280 PAY 277 

    0xb5363692,// 281 PAY 278 

    0xd5af74a5,// 282 PAY 279 

    0x729c7a20,// 283 PAY 280 

    0xf898eaa1,// 284 PAY 281 

    0x500262cb,// 285 PAY 282 

    0x53e409b3,// 286 PAY 283 

    0x69036323,// 287 PAY 284 

    0x739eeaef,// 288 PAY 285 

    0x2c12006d,// 289 PAY 286 

    0x53493742,// 290 PAY 287 

    0xf807882d,// 291 PAY 288 

    0x12584588,// 292 PAY 289 

    0x20e9d4c5,// 293 PAY 290 

    0x5f5a5778,// 294 PAY 291 

    0xea7b0656,// 295 PAY 292 

    0x72425b72,// 296 PAY 293 

    0x88fe5da5,// 297 PAY 294 

    0x9ebf81c5,// 298 PAY 295 

    0xb92cdd14,// 299 PAY 296 

    0xd92e0d50,// 300 PAY 297 

    0xd84338e6,// 301 PAY 298 

    0xe05b7716,// 302 PAY 299 

    0x14611fbf,// 303 PAY 300 

    0x377e8714,// 304 PAY 301 

    0x2a0547a4,// 305 PAY 302 

    0x5b864313,// 306 PAY 303 

    0x7340e51a,// 307 PAY 304 

    0xefec5f83,// 308 PAY 305 

    0x9a5cddd3,// 309 PAY 306 

    0x6955d44b,// 310 PAY 307 

    0x14886abd,// 311 PAY 308 

    0x973b4d19,// 312 PAY 309 

    0x04f127d2,// 313 PAY 310 

    0x06462731,// 314 PAY 311 

    0xe1c5045a,// 315 PAY 312 

    0xe9cf1198,// 316 PAY 313 

    0xef62403f,// 317 PAY 314 

    0x0d250073,// 318 PAY 315 

    0x9233f1e7,// 319 PAY 316 

    0xea27869f,// 320 PAY 317 

    0x278b9d6c,// 321 PAY 318 

    0x2b080000,// 322 PAY 319 

/// HASH is  20 bytes 

    0xb39bb0d2,// 323 HSH   1 

    0x2568b4ed,// 324 HSH   2 

    0x8fb1fa68,// 325 HSH   3 

    0x3e850355,// 326 HSH   4 

    0xe28754e5,// 327 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 229 

/// STA pkt_idx        : 76 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4f 

    0x01314fe5 // 328 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt39_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 273 words. 

/// BDA size     is 1088 (0x440) 

/// BDA id       is 0xc25c 

    0x0440c25c,// 3 BDA   1 

/// PAY Generic Data size   : 1088 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xce8e30fd,// 4 PAY   1 

    0x8788640d,// 5 PAY   2 

    0x05e3db2d,// 6 PAY   3 

    0x701fec94,// 7 PAY   4 

    0xe48ce3cf,// 8 PAY   5 

    0xa226e36c,// 9 PAY   6 

    0x3633bbbb,// 10 PAY   7 

    0x9e698e80,// 11 PAY   8 

    0x91b7bc24,// 12 PAY   9 

    0x91cbc456,// 13 PAY  10 

    0x6458a2c6,// 14 PAY  11 

    0x11760f4d,// 15 PAY  12 

    0x4b79db61,// 16 PAY  13 

    0xfe45fe26,// 17 PAY  14 

    0xee0c0738,// 18 PAY  15 

    0xdec7e831,// 19 PAY  16 

    0x7396d1ba,// 20 PAY  17 

    0x58e15245,// 21 PAY  18 

    0xec15daa0,// 22 PAY  19 

    0xbed3a262,// 23 PAY  20 

    0x9e2b3227,// 24 PAY  21 

    0xd8aa92f7,// 25 PAY  22 

    0x0ff28964,// 26 PAY  23 

    0xc4183f8e,// 27 PAY  24 

    0x89324211,// 28 PAY  25 

    0xf516c6fd,// 29 PAY  26 

    0xe3a63e79,// 30 PAY  27 

    0x1c6ad627,// 31 PAY  28 

    0x24ae076d,// 32 PAY  29 

    0x3787dc18,// 33 PAY  30 

    0xdce3c0f1,// 34 PAY  31 

    0xfec22848,// 35 PAY  32 

    0x0a31070a,// 36 PAY  33 

    0x74499f2e,// 37 PAY  34 

    0xfb13018b,// 38 PAY  35 

    0x7a9bbf2f,// 39 PAY  36 

    0x2d4240e4,// 40 PAY  37 

    0xa6d2ee05,// 41 PAY  38 

    0xfd1ee60d,// 42 PAY  39 

    0x538644b8,// 43 PAY  40 

    0x7038174a,// 44 PAY  41 

    0xc1e2646b,// 45 PAY  42 

    0xb304a839,// 46 PAY  43 

    0x0ea0b66f,// 47 PAY  44 

    0x97e5ea44,// 48 PAY  45 

    0xa70c637a,// 49 PAY  46 

    0x84ba43a6,// 50 PAY  47 

    0x44902244,// 51 PAY  48 

    0x589bdaee,// 52 PAY  49 

    0x930bbb70,// 53 PAY  50 

    0x8072a95b,// 54 PAY  51 

    0xa3afc9de,// 55 PAY  52 

    0xd6514271,// 56 PAY  53 

    0xf221a7de,// 57 PAY  54 

    0x81687160,// 58 PAY  55 

    0x72cb4114,// 59 PAY  56 

    0x987f684e,// 60 PAY  57 

    0xc04cb931,// 61 PAY  58 

    0x84ac85ea,// 62 PAY  59 

    0xfe1e5d75,// 63 PAY  60 

    0xe41779e2,// 64 PAY  61 

    0xd91cadfd,// 65 PAY  62 

    0xea8aedaf,// 66 PAY  63 

    0x95848443,// 67 PAY  64 

    0xe033f000,// 68 PAY  65 

    0x245ddc5e,// 69 PAY  66 

    0xc11ad693,// 70 PAY  67 

    0x18ae9e74,// 71 PAY  68 

    0xb65b6630,// 72 PAY  69 

    0xa6303c33,// 73 PAY  70 

    0x4fb30223,// 74 PAY  71 

    0x716cb9db,// 75 PAY  72 

    0x00f9c60d,// 76 PAY  73 

    0x24477884,// 77 PAY  74 

    0x79795985,// 78 PAY  75 

    0x43651d40,// 79 PAY  76 

    0x226636b0,// 80 PAY  77 

    0x57ba228b,// 81 PAY  78 

    0x9efcfa7a,// 82 PAY  79 

    0x7af24d39,// 83 PAY  80 

    0x17005205,// 84 PAY  81 

    0xdc9e1997,// 85 PAY  82 

    0x92e1777e,// 86 PAY  83 

    0x2a6d0b8a,// 87 PAY  84 

    0x6b71ecaa,// 88 PAY  85 

    0x29ac46e8,// 89 PAY  86 

    0x27047a3e,// 90 PAY  87 

    0x7f99e736,// 91 PAY  88 

    0xa9616135,// 92 PAY  89 

    0x6204fbba,// 93 PAY  90 

    0x9f5ccebe,// 94 PAY  91 

    0x95adb53e,// 95 PAY  92 

    0x57b1c0c4,// 96 PAY  93 

    0xa3d2ffef,// 97 PAY  94 

    0x571c1f4d,// 98 PAY  95 

    0x85583b30,// 99 PAY  96 

    0xc0e99ca3,// 100 PAY  97 

    0x42057e59,// 101 PAY  98 

    0x58ededd7,// 102 PAY  99 

    0x22957bc5,// 103 PAY 100 

    0x97820abe,// 104 PAY 101 

    0xcede5b96,// 105 PAY 102 

    0x2dbafdb7,// 106 PAY 103 

    0xa06269eb,// 107 PAY 104 

    0xb80635c9,// 108 PAY 105 

    0x0093ca3e,// 109 PAY 106 

    0x37c12428,// 110 PAY 107 

    0xe1cb2669,// 111 PAY 108 

    0x2988c856,// 112 PAY 109 

    0x95a7268f,// 113 PAY 110 

    0x9cf46516,// 114 PAY 111 

    0xaa454b09,// 115 PAY 112 

    0x2b04713a,// 116 PAY 113 

    0xe8f8d8b1,// 117 PAY 114 

    0x5ff06451,// 118 PAY 115 

    0x1f90e37a,// 119 PAY 116 

    0x23e24c8c,// 120 PAY 117 

    0x0526b25b,// 121 PAY 118 

    0x1b362e36,// 122 PAY 119 

    0x8e354cbd,// 123 PAY 120 

    0x1c304966,// 124 PAY 121 

    0x814ab5c3,// 125 PAY 122 

    0x984f5c05,// 126 PAY 123 

    0x895af581,// 127 PAY 124 

    0x6f305c23,// 128 PAY 125 

    0x3c97328a,// 129 PAY 126 

    0xba752bbb,// 130 PAY 127 

    0xb66704a3,// 131 PAY 128 

    0x379391a9,// 132 PAY 129 

    0xb728a7af,// 133 PAY 130 

    0xb07a50ca,// 134 PAY 131 

    0x47c0d8b2,// 135 PAY 132 

    0x434f409e,// 136 PAY 133 

    0xe1e2498d,// 137 PAY 134 

    0x0d13824c,// 138 PAY 135 

    0xf0caea73,// 139 PAY 136 

    0xbf80155f,// 140 PAY 137 

    0xced016a0,// 141 PAY 138 

    0xcfe68243,// 142 PAY 139 

    0xfc514022,// 143 PAY 140 

    0xfd2b573c,// 144 PAY 141 

    0xedfdcb5d,// 145 PAY 142 

    0xdbd1785b,// 146 PAY 143 

    0x0ed59dd0,// 147 PAY 144 

    0xac68e836,// 148 PAY 145 

    0xfbd44d1a,// 149 PAY 146 

    0x0bdb9f2d,// 150 PAY 147 

    0xcff88b93,// 151 PAY 148 

    0x35367470,// 152 PAY 149 

    0xcc6762e7,// 153 PAY 150 

    0x7802dc5e,// 154 PAY 151 

    0x58324763,// 155 PAY 152 

    0x67aad802,// 156 PAY 153 

    0xcbc408bf,// 157 PAY 154 

    0x93f4ab9f,// 158 PAY 155 

    0x5bd62108,// 159 PAY 156 

    0xc7af81a9,// 160 PAY 157 

    0x5649ff7b,// 161 PAY 158 

    0xdcb1c79c,// 162 PAY 159 

    0x64ebd7c3,// 163 PAY 160 

    0x93e628db,// 164 PAY 161 

    0x3a6a777f,// 165 PAY 162 

    0xad5a995b,// 166 PAY 163 

    0xcf491002,// 167 PAY 164 

    0x12188a41,// 168 PAY 165 

    0x979504cf,// 169 PAY 166 

    0x6c62150b,// 170 PAY 167 

    0xf2f14ea5,// 171 PAY 168 

    0x2386f876,// 172 PAY 169 

    0xe793e1d5,// 173 PAY 170 

    0xd29ac9d6,// 174 PAY 171 

    0x78940742,// 175 PAY 172 

    0x7658a9ea,// 176 PAY 173 

    0x0d90ca01,// 177 PAY 174 

    0x4eba12b1,// 178 PAY 175 

    0x5e2a201f,// 179 PAY 176 

    0x5fece313,// 180 PAY 177 

    0x4bae7213,// 181 PAY 178 

    0x0bf83cc9,// 182 PAY 179 

    0x24b47626,// 183 PAY 180 

    0x51010cfe,// 184 PAY 181 

    0x5d03651a,// 185 PAY 182 

    0x62550629,// 186 PAY 183 

    0xde606b05,// 187 PAY 184 

    0x3c2093c2,// 188 PAY 185 

    0xc8463157,// 189 PAY 186 

    0xee89522d,// 190 PAY 187 

    0x2598b379,// 191 PAY 188 

    0x9ec3980b,// 192 PAY 189 

    0xe56c96f9,// 193 PAY 190 

    0xf44a80a9,// 194 PAY 191 

    0x2b715775,// 195 PAY 192 

    0xe5e0cef6,// 196 PAY 193 

    0x1f3c4ddf,// 197 PAY 194 

    0x05dfd04a,// 198 PAY 195 

    0x4101ea59,// 199 PAY 196 

    0xf074cbdd,// 200 PAY 197 

    0xdeb6e271,// 201 PAY 198 

    0x4d198e3e,// 202 PAY 199 

    0xfa407236,// 203 PAY 200 

    0x529d6d74,// 204 PAY 201 

    0xdb252087,// 205 PAY 202 

    0xdcb5cbd8,// 206 PAY 203 

    0x7c153031,// 207 PAY 204 

    0x147349d7,// 208 PAY 205 

    0x23ceddcc,// 209 PAY 206 

    0x1723391b,// 210 PAY 207 

    0x5636b2bd,// 211 PAY 208 

    0xee3f2d7f,// 212 PAY 209 

    0xbc9dae8c,// 213 PAY 210 

    0x8fa18b35,// 214 PAY 211 

    0x38836ee0,// 215 PAY 212 

    0xdc024b7f,// 216 PAY 213 

    0x16e52d8f,// 217 PAY 214 

    0x014c609e,// 218 PAY 215 

    0x3e79f1a7,// 219 PAY 216 

    0x405c193b,// 220 PAY 217 

    0x8f5f64bd,// 221 PAY 218 

    0x5ca0e509,// 222 PAY 219 

    0x38e2cfc5,// 223 PAY 220 

    0xb8e5d895,// 224 PAY 221 

    0x93172ed3,// 225 PAY 222 

    0x6dc37cfa,// 226 PAY 223 

    0x32f360ad,// 227 PAY 224 

    0xc777598e,// 228 PAY 225 

    0xb1a3af49,// 229 PAY 226 

    0xa61e0411,// 230 PAY 227 

    0x93e71138,// 231 PAY 228 

    0x5dd04d69,// 232 PAY 229 

    0x21b3a0e6,// 233 PAY 230 

    0x8ddc799e,// 234 PAY 231 

    0x7c8f2fd7,// 235 PAY 232 

    0x5c973442,// 236 PAY 233 

    0x53c645a9,// 237 PAY 234 

    0xf4ae8d66,// 238 PAY 235 

    0x9b85ac03,// 239 PAY 236 

    0x2eac4482,// 240 PAY 237 

    0x366b6dfd,// 241 PAY 238 

    0x82c6be39,// 242 PAY 239 

    0xfdd88c5b,// 243 PAY 240 

    0xeff27ce1,// 244 PAY 241 

    0x4dc57b3b,// 245 PAY 242 

    0x8f59289e,// 246 PAY 243 

    0x9d7e6a5b,// 247 PAY 244 

    0xf21e5fcf,// 248 PAY 245 

    0xf832a7af,// 249 PAY 246 

    0x6f3af6be,// 250 PAY 247 

    0x05844566,// 251 PAY 248 

    0x16a20867,// 252 PAY 249 

    0x44cb7eea,// 253 PAY 250 

    0xb15339e3,// 254 PAY 251 

    0x2d65d046,// 255 PAY 252 

    0xcbcda7fe,// 256 PAY 253 

    0xd31748f8,// 257 PAY 254 

    0x101e10bc,// 258 PAY 255 

    0x5bb3d345,// 259 PAY 256 

    0x3575ae9e,// 260 PAY 257 

    0x03c1562d,// 261 PAY 258 

    0x155b85ce,// 262 PAY 259 

    0x75039f2e,// 263 PAY 260 

    0x65a9f42d,// 264 PAY 261 

    0x5c158b60,// 265 PAY 262 

    0x2445a7cd,// 266 PAY 263 

    0x698322c9,// 267 PAY 264 

    0x988b1276,// 268 PAY 265 

    0x58763878,// 269 PAY 266 

    0x4a3689de,// 270 PAY 267 

    0x0dfa889f,// 271 PAY 268 

    0xafe28271,// 272 PAY 269 

    0x88fb3675,// 273 PAY 270 

    0xcb2f57f2,// 274 PAY 271 

    0xaf323cc0,// 275 PAY 272 

/// HASH is  8 bytes 

    0x37c12428,// 276 HSH   1 

    0xe1cb2669,// 277 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 208 

/// STA pkt_idx        : 48 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5f 

    0x00c05fd0 // 278 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt40_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 439 words. 

/// BDA size     is 1749 (0x6d5) 

/// BDA id       is 0xd94d 

    0x06d5d94d,// 3 BDA   1 

/// PAY Generic Data size   : 1749 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xa1a0d39a,// 4 PAY   1 

    0xdf3b5fbb,// 5 PAY   2 

    0xf3d31c45,// 6 PAY   3 

    0x440b236e,// 7 PAY   4 

    0xd4ddeaea,// 8 PAY   5 

    0x3362f0fb,// 9 PAY   6 

    0xff5296a8,// 10 PAY   7 

    0x9b9cb418,// 11 PAY   8 

    0xf0c41481,// 12 PAY   9 

    0x632f105c,// 13 PAY  10 

    0x27660b7c,// 14 PAY  11 

    0x07016435,// 15 PAY  12 

    0x659c9413,// 16 PAY  13 

    0xd5dc99ac,// 17 PAY  14 

    0x87b582ef,// 18 PAY  15 

    0x6eac083e,// 19 PAY  16 

    0x08ed721e,// 20 PAY  17 

    0x13aad280,// 21 PAY  18 

    0xa9a90c1b,// 22 PAY  19 

    0x9636e107,// 23 PAY  20 

    0xb3ac74e8,// 24 PAY  21 

    0xcc3c971f,// 25 PAY  22 

    0x6273bb0f,// 26 PAY  23 

    0xf2c368fe,// 27 PAY  24 

    0x1a89aaca,// 28 PAY  25 

    0xc6818263,// 29 PAY  26 

    0xecc77023,// 30 PAY  27 

    0x4f8d1513,// 31 PAY  28 

    0x6c37b995,// 32 PAY  29 

    0xffd3f8f7,// 33 PAY  30 

    0x64261ade,// 34 PAY  31 

    0x06f88684,// 35 PAY  32 

    0x9671e650,// 36 PAY  33 

    0x466c1bd1,// 37 PAY  34 

    0x5cd74da1,// 38 PAY  35 

    0x5c3d4812,// 39 PAY  36 

    0xba276a01,// 40 PAY  37 

    0x594f6767,// 41 PAY  38 

    0x51065e26,// 42 PAY  39 

    0xdad8260b,// 43 PAY  40 

    0x8789bd94,// 44 PAY  41 

    0x1f50f97e,// 45 PAY  42 

    0x4d8d4287,// 46 PAY  43 

    0x74bc55ea,// 47 PAY  44 

    0x8a94ccb5,// 48 PAY  45 

    0xb21e82ad,// 49 PAY  46 

    0x1a26f42c,// 50 PAY  47 

    0xad16bb4f,// 51 PAY  48 

    0x6771bcb1,// 52 PAY  49 

    0x8675e59f,// 53 PAY  50 

    0x9ad165d4,// 54 PAY  51 

    0x6656b7e8,// 55 PAY  52 

    0xf246838d,// 56 PAY  53 

    0x6dc17766,// 57 PAY  54 

    0xd1de366c,// 58 PAY  55 

    0x3274d6a1,// 59 PAY  56 

    0x0111fc4b,// 60 PAY  57 

    0x16aec896,// 61 PAY  58 

    0x872ae94e,// 62 PAY  59 

    0x411144fb,// 63 PAY  60 

    0x49d90fb5,// 64 PAY  61 

    0xa840374c,// 65 PAY  62 

    0x84023912,// 66 PAY  63 

    0xbce07566,// 67 PAY  64 

    0xb35ed309,// 68 PAY  65 

    0x7b5a4425,// 69 PAY  66 

    0x24116836,// 70 PAY  67 

    0xaed2dbed,// 71 PAY  68 

    0xbd428593,// 72 PAY  69 

    0x3c7c1662,// 73 PAY  70 

    0xf03c8b1b,// 74 PAY  71 

    0x4095545f,// 75 PAY  72 

    0x228480aa,// 76 PAY  73 

    0x38fd219b,// 77 PAY  74 

    0xbd05bc3d,// 78 PAY  75 

    0x82c5e040,// 79 PAY  76 

    0x1621260e,// 80 PAY  77 

    0x1f728828,// 81 PAY  78 

    0xe6573a5d,// 82 PAY  79 

    0x9405fab9,// 83 PAY  80 

    0x2df5416d,// 84 PAY  81 

    0xcb9684dc,// 85 PAY  82 

    0xfb4b452f,// 86 PAY  83 

    0x3747adec,// 87 PAY  84 

    0x8c12d3e1,// 88 PAY  85 

    0x804e27fe,// 89 PAY  86 

    0xa89c2a3f,// 90 PAY  87 

    0x82216652,// 91 PAY  88 

    0x65ec1b3d,// 92 PAY  89 

    0x84409e2e,// 93 PAY  90 

    0x81c4924d,// 94 PAY  91 

    0x396d10b6,// 95 PAY  92 

    0xbec37060,// 96 PAY  93 

    0x337d6106,// 97 PAY  94 

    0xe77e13c4,// 98 PAY  95 

    0x8f6a8632,// 99 PAY  96 

    0x7c672ac7,// 100 PAY  97 

    0x03a2fe34,// 101 PAY  98 

    0x88f407aa,// 102 PAY  99 

    0x95c1dbfa,// 103 PAY 100 

    0x38adc0c3,// 104 PAY 101 

    0x15a862de,// 105 PAY 102 

    0x1d822b27,// 106 PAY 103 

    0xdb92d09a,// 107 PAY 104 

    0x8fb7cdc9,// 108 PAY 105 

    0xc6b95886,// 109 PAY 106 

    0xa45e226b,// 110 PAY 107 

    0xc8344dc8,// 111 PAY 108 

    0x15fd3d44,// 112 PAY 109 

    0xa25cd729,// 113 PAY 110 

    0x4c91a5d4,// 114 PAY 111 

    0x162bfc53,// 115 PAY 112 

    0xb7195fb5,// 116 PAY 113 

    0x1e615fd2,// 117 PAY 114 

    0xdf75e63a,// 118 PAY 115 

    0xcf600311,// 119 PAY 116 

    0x4939952c,// 120 PAY 117 

    0xd21b6562,// 121 PAY 118 

    0x3fe04938,// 122 PAY 119 

    0x7c8b2e10,// 123 PAY 120 

    0xf56049e6,// 124 PAY 121 

    0x18681429,// 125 PAY 122 

    0x6a24ffc4,// 126 PAY 123 

    0x49402b73,// 127 PAY 124 

    0xc55340ed,// 128 PAY 125 

    0x6f0ee34b,// 129 PAY 126 

    0x4d29891d,// 130 PAY 127 

    0x4a05e8d2,// 131 PAY 128 

    0x5b03e169,// 132 PAY 129 

    0x3faf8268,// 133 PAY 130 

    0x8edf7b72,// 134 PAY 131 

    0x0add7ce8,// 135 PAY 132 

    0x00ee213a,// 136 PAY 133 

    0xb19435a6,// 137 PAY 134 

    0x9f285900,// 138 PAY 135 

    0x6e247e3b,// 139 PAY 136 

    0x917cea02,// 140 PAY 137 

    0x22158e89,// 141 PAY 138 

    0xf82a52e8,// 142 PAY 139 

    0xa053fd07,// 143 PAY 140 

    0x2c16a15d,// 144 PAY 141 

    0x56af1582,// 145 PAY 142 

    0x1e5e319e,// 146 PAY 143 

    0xf7a70bf5,// 147 PAY 144 

    0x09ac55bc,// 148 PAY 145 

    0xdc2e4719,// 149 PAY 146 

    0xe839f09d,// 150 PAY 147 

    0x1ef204d2,// 151 PAY 148 

    0x19283c36,// 152 PAY 149 

    0x59456232,// 153 PAY 150 

    0xe17291d6,// 154 PAY 151 

    0x2f78c5d1,// 155 PAY 152 

    0x41b98066,// 156 PAY 153 

    0xcab2abd9,// 157 PAY 154 

    0xfae650da,// 158 PAY 155 

    0x460d6fae,// 159 PAY 156 

    0x8fb27665,// 160 PAY 157 

    0xe16c691e,// 161 PAY 158 

    0x42b923d1,// 162 PAY 159 

    0x1334d8ee,// 163 PAY 160 

    0xff41d971,// 164 PAY 161 

    0xf186da92,// 165 PAY 162 

    0x94d02db7,// 166 PAY 163 

    0x01c2a5e2,// 167 PAY 164 

    0xb3eb0d06,// 168 PAY 165 

    0x907c6b82,// 169 PAY 166 

    0x302d8e3c,// 170 PAY 167 

    0x4fab634c,// 171 PAY 168 

    0x7417d926,// 172 PAY 169 

    0x5850fa97,// 173 PAY 170 

    0xa84cd941,// 174 PAY 171 

    0x15d20d4a,// 175 PAY 172 

    0x8fd5c403,// 176 PAY 173 

    0xf1d4613f,// 177 PAY 174 

    0xe0087423,// 178 PAY 175 

    0xf810ee02,// 179 PAY 176 

    0x772939f2,// 180 PAY 177 

    0x8163a0d2,// 181 PAY 178 

    0x81652d26,// 182 PAY 179 

    0xcaee5646,// 183 PAY 180 

    0x47be7c7e,// 184 PAY 181 

    0x48d8999f,// 185 PAY 182 

    0xfe818ef1,// 186 PAY 183 

    0xad547585,// 187 PAY 184 

    0x792f282e,// 188 PAY 185 

    0x6578aa78,// 189 PAY 186 

    0x0dc5aeb7,// 190 PAY 187 

    0x3e9be63c,// 191 PAY 188 

    0x2c7ea285,// 192 PAY 189 

    0x2921754c,// 193 PAY 190 

    0x0de51da9,// 194 PAY 191 

    0x96d186ab,// 195 PAY 192 

    0x372122ef,// 196 PAY 193 

    0xd8a7138d,// 197 PAY 194 

    0x5adb3ced,// 198 PAY 195 

    0x60631de8,// 199 PAY 196 

    0xab2eb553,// 200 PAY 197 

    0xf279bd76,// 201 PAY 198 

    0x33504d6d,// 202 PAY 199 

    0xb0be4346,// 203 PAY 200 

    0x8ce3a10c,// 204 PAY 201 

    0x20627e41,// 205 PAY 202 

    0x3d3dfafc,// 206 PAY 203 

    0xe2e5e078,// 207 PAY 204 

    0xb13086cf,// 208 PAY 205 

    0xf4181e5c,// 209 PAY 206 

    0x0bb03415,// 210 PAY 207 

    0x3ca351e5,// 211 PAY 208 

    0x7f88b19a,// 212 PAY 209 

    0x8de6052c,// 213 PAY 210 

    0xe4f7eb5b,// 214 PAY 211 

    0xd20c2eb0,// 215 PAY 212 

    0xd75b3587,// 216 PAY 213 

    0x82703560,// 217 PAY 214 

    0x200d6718,// 218 PAY 215 

    0x9dba77e3,// 219 PAY 216 

    0x073364b6,// 220 PAY 217 

    0x70e29b47,// 221 PAY 218 

    0x15d7a164,// 222 PAY 219 

    0xc562eedb,// 223 PAY 220 

    0x864df969,// 224 PAY 221 

    0x95f71e28,// 225 PAY 222 

    0xee0cf7e9,// 226 PAY 223 

    0x720378e7,// 227 PAY 224 

    0x41f744cf,// 228 PAY 225 

    0x38d564f1,// 229 PAY 226 

    0x5b936703,// 230 PAY 227 

    0x3ca80102,// 231 PAY 228 

    0x4bc96d5c,// 232 PAY 229 

    0x7a9c372d,// 233 PAY 230 

    0x55fb4093,// 234 PAY 231 

    0xd42c3b62,// 235 PAY 232 

    0x0881d9da,// 236 PAY 233 

    0xe2377808,// 237 PAY 234 

    0x3d47faf4,// 238 PAY 235 

    0x3117d48c,// 239 PAY 236 

    0xc597308e,// 240 PAY 237 

    0xd6e7cfdb,// 241 PAY 238 

    0x8918c7c7,// 242 PAY 239 

    0x120b01db,// 243 PAY 240 

    0x2ce2c17c,// 244 PAY 241 

    0x57a491e6,// 245 PAY 242 

    0xb1892cd6,// 246 PAY 243 

    0xfaba168a,// 247 PAY 244 

    0x8b8d07f4,// 248 PAY 245 

    0x4baee747,// 249 PAY 246 

    0xb3374396,// 250 PAY 247 

    0x22831def,// 251 PAY 248 

    0x61d23fe6,// 252 PAY 249 

    0xa63587bf,// 253 PAY 250 

    0x11d442b4,// 254 PAY 251 

    0x51c1ff1c,// 255 PAY 252 

    0x7c784ed5,// 256 PAY 253 

    0x85ca7042,// 257 PAY 254 

    0x1a1e0e67,// 258 PAY 255 

    0xed21bd54,// 259 PAY 256 

    0x02124b0b,// 260 PAY 257 

    0xebf9b76a,// 261 PAY 258 

    0x4108a406,// 262 PAY 259 

    0xe7a17c8b,// 263 PAY 260 

    0x66b2ad5c,// 264 PAY 261 

    0x6a99344f,// 265 PAY 262 

    0x0da6272d,// 266 PAY 263 

    0x927650bd,// 267 PAY 264 

    0xb23acc49,// 268 PAY 265 

    0x7cd366d8,// 269 PAY 266 

    0xc2ea0663,// 270 PAY 267 

    0x894e4b23,// 271 PAY 268 

    0x4c5d2ea1,// 272 PAY 269 

    0xe6304b89,// 273 PAY 270 

    0x7d49bc5e,// 274 PAY 271 

    0x99e37faa,// 275 PAY 272 

    0x8d298dae,// 276 PAY 273 

    0x579db8db,// 277 PAY 274 

    0xc952ff30,// 278 PAY 275 

    0x7b1815c9,// 279 PAY 276 

    0x2b417153,// 280 PAY 277 

    0x35bf4e12,// 281 PAY 278 

    0x834bfdbc,// 282 PAY 279 

    0x8980ffe0,// 283 PAY 280 

    0xe7730495,// 284 PAY 281 

    0x8c225d06,// 285 PAY 282 

    0xea9dbdb1,// 286 PAY 283 

    0xd69144ee,// 287 PAY 284 

    0x5c30c961,// 288 PAY 285 

    0xdfe8443b,// 289 PAY 286 

    0xed948116,// 290 PAY 287 

    0x99de74c2,// 291 PAY 288 

    0xd048c9df,// 292 PAY 289 

    0xd89f6b58,// 293 PAY 290 

    0x43d050e4,// 294 PAY 291 

    0xbaead4f9,// 295 PAY 292 

    0x73edb75c,// 296 PAY 293 

    0xc0fe658c,// 297 PAY 294 

    0x61a2da90,// 298 PAY 295 

    0xe5eb4fe6,// 299 PAY 296 

    0x4b16a411,// 300 PAY 297 

    0x72537c5d,// 301 PAY 298 

    0x2d25577f,// 302 PAY 299 

    0x964d2f45,// 303 PAY 300 

    0xde378ea7,// 304 PAY 301 

    0x9a659aec,// 305 PAY 302 

    0x3562cd57,// 306 PAY 303 

    0x6da7dada,// 307 PAY 304 

    0xafbdd953,// 308 PAY 305 

    0x176c0036,// 309 PAY 306 

    0xdb83461f,// 310 PAY 307 

    0xaf8aebd3,// 311 PAY 308 

    0x9db41d75,// 312 PAY 309 

    0x8b22feb7,// 313 PAY 310 

    0xb835cfb5,// 314 PAY 311 

    0xd02e02a3,// 315 PAY 312 

    0x79399ff7,// 316 PAY 313 

    0x6cb7f79c,// 317 PAY 314 

    0x05151d00,// 318 PAY 315 

    0xf61d7487,// 319 PAY 316 

    0x89b47fb1,// 320 PAY 317 

    0x79c165ee,// 321 PAY 318 

    0x722c3a19,// 322 PAY 319 

    0xc4c39f36,// 323 PAY 320 

    0xe6283d77,// 324 PAY 321 

    0xb79a6d64,// 325 PAY 322 

    0x6302d900,// 326 PAY 323 

    0x2b562d07,// 327 PAY 324 

    0xa60b9cb7,// 328 PAY 325 

    0xcfbae2f6,// 329 PAY 326 

    0xbe1e5eb0,// 330 PAY 327 

    0x7d34b6a1,// 331 PAY 328 

    0xac62ac86,// 332 PAY 329 

    0xfdc6b6ed,// 333 PAY 330 

    0x34a50751,// 334 PAY 331 

    0xffc84142,// 335 PAY 332 

    0xf3a3a25d,// 336 PAY 333 

    0x6af41280,// 337 PAY 334 

    0xf123b94d,// 338 PAY 335 

    0x5f16358d,// 339 PAY 336 

    0xc374cc3a,// 340 PAY 337 

    0x7742988c,// 341 PAY 338 

    0xa436d3e2,// 342 PAY 339 

    0x7f9a8cd1,// 343 PAY 340 

    0xa3e71d26,// 344 PAY 341 

    0xcf510959,// 345 PAY 342 

    0x1d7de66e,// 346 PAY 343 

    0x6eabdf20,// 347 PAY 344 

    0x617b8999,// 348 PAY 345 

    0xf4a0e716,// 349 PAY 346 

    0xe873b93e,// 350 PAY 347 

    0x9f68163a,// 351 PAY 348 

    0x4b732b33,// 352 PAY 349 

    0x73314e63,// 353 PAY 350 

    0x1c0a704c,// 354 PAY 351 

    0xc3d4b94b,// 355 PAY 352 

    0x3ce58310,// 356 PAY 353 

    0xd1fc0e46,// 357 PAY 354 

    0xb0f5e0c1,// 358 PAY 355 

    0x042e7697,// 359 PAY 356 

    0xa9d63a49,// 360 PAY 357 

    0x5ba8fd77,// 361 PAY 358 

    0x0f3f7802,// 362 PAY 359 

    0x014a8387,// 363 PAY 360 

    0xf9b7d264,// 364 PAY 361 

    0x6bf453fa,// 365 PAY 362 

    0xe483d6de,// 366 PAY 363 

    0x1d00c1ea,// 367 PAY 364 

    0x8fd1f6fc,// 368 PAY 365 

    0x070852b7,// 369 PAY 366 

    0x56e10e5a,// 370 PAY 367 

    0x04268f81,// 371 PAY 368 

    0x9964c537,// 372 PAY 369 

    0x4a08cffc,// 373 PAY 370 

    0xa99a9fc3,// 374 PAY 371 

    0x2d793b24,// 375 PAY 372 

    0xcc4e0838,// 376 PAY 373 

    0xacfc823c,// 377 PAY 374 

    0xf85c8b99,// 378 PAY 375 

    0xc5c35769,// 379 PAY 376 

    0xfd49b5c9,// 380 PAY 377 

    0xa15161ed,// 381 PAY 378 

    0x60056d3d,// 382 PAY 379 

    0x2378f733,// 383 PAY 380 

    0x4b542ee2,// 384 PAY 381 

    0x26f81d2a,// 385 PAY 382 

    0x95778f69,// 386 PAY 383 

    0x1ace4d7d,// 387 PAY 384 

    0x780527be,// 388 PAY 385 

    0x79f979c4,// 389 PAY 386 

    0xc2c52ec6,// 390 PAY 387 

    0xe8b61cde,// 391 PAY 388 

    0x37f5daaf,// 392 PAY 389 

    0xc9efd3c5,// 393 PAY 390 

    0xdf0b4a6a,// 394 PAY 391 

    0x888dcd57,// 395 PAY 392 

    0x314a70b9,// 396 PAY 393 

    0x59f31e4b,// 397 PAY 394 

    0xad0cd829,// 398 PAY 395 

    0xb02a8a7a,// 399 PAY 396 

    0xc08633b1,// 400 PAY 397 

    0xd71bb318,// 401 PAY 398 

    0x47b7abf7,// 402 PAY 399 

    0xaee9cfe7,// 403 PAY 400 

    0x0581f751,// 404 PAY 401 

    0xd818bf56,// 405 PAY 402 

    0xf80f1026,// 406 PAY 403 

    0xeece9120,// 407 PAY 404 

    0xb82882db,// 408 PAY 405 

    0xf58f62c9,// 409 PAY 406 

    0x9140e1af,// 410 PAY 407 

    0x8612cd52,// 411 PAY 408 

    0x88e3db43,// 412 PAY 409 

    0x2e756e30,// 413 PAY 410 

    0x69ccc25f,// 414 PAY 411 

    0x65bc1f8e,// 415 PAY 412 

    0xc92b0462,// 416 PAY 413 

    0x6274fbba,// 417 PAY 414 

    0x5516ed4c,// 418 PAY 415 

    0x942479c1,// 419 PAY 416 

    0x80242847,// 420 PAY 417 

    0x1987d9a5,// 421 PAY 418 

    0x91a1051e,// 422 PAY 419 

    0x60b2802c,// 423 PAY 420 

    0xc445625d,// 424 PAY 421 

    0x9df1a89c,// 425 PAY 422 

    0xcfae6e2e,// 426 PAY 423 

    0x20963696,// 427 PAY 424 

    0xccb8f6e2,// 428 PAY 425 

    0x81d8696c,// 429 PAY 426 

    0x6ed1170e,// 430 PAY 427 

    0x6edc168c,// 431 PAY 428 

    0x2ebd92d5,// 432 PAY 429 

    0x1f5818c4,// 433 PAY 430 

    0x452bc949,// 434 PAY 431 

    0xc671e06a,// 435 PAY 432 

    0xba6f1b12,// 436 PAY 433 

    0xefe8d96f,// 437 PAY 434 

    0x5ce336f5,// 438 PAY 435 

    0x27c7a565,// 439 PAY 436 

    0x12b0f558,// 440 PAY 437 

    0xfa000000,// 441 PAY 438 

/// HASH is  20 bytes 

    0x11d442b4,// 442 HSH   1 

    0x51c1ff1c,// 443 HSH   2 

    0x7c784ed5,// 444 HSH   3 

    0x85ca7042,// 445 HSH   4 

    0x1a1e0e67,// 446 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 120 

/// STA pkt_idx        : 243 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdb 

    0x03ccdb78 // 447 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt41_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 483 words. 

/// BDA size     is 1925 (0x785) 

/// BDA id       is 0x4f88 

    0x07854f88,// 3 BDA   1 

/// PAY Generic Data size   : 1925 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x1d212ce8,// 4 PAY   1 

    0x136a18f8,// 5 PAY   2 

    0x55412ab5,// 6 PAY   3 

    0x6dba33e9,// 7 PAY   4 

    0x8c0e5828,// 8 PAY   5 

    0x696cda8c,// 9 PAY   6 

    0x7a308565,// 10 PAY   7 

    0xadd22981,// 11 PAY   8 

    0x6c1cdecb,// 12 PAY   9 

    0x925e3f81,// 13 PAY  10 

    0xc21d340b,// 14 PAY  11 

    0x656dee13,// 15 PAY  12 

    0xc9d8015b,// 16 PAY  13 

    0x72a47536,// 17 PAY  14 

    0x837006bd,// 18 PAY  15 

    0x144204ad,// 19 PAY  16 

    0x5142ef21,// 20 PAY  17 

    0x12705207,// 21 PAY  18 

    0xaa2837ed,// 22 PAY  19 

    0x3e1d4b5c,// 23 PAY  20 

    0xf2b9870b,// 24 PAY  21 

    0x2af24a3c,// 25 PAY  22 

    0x9a559acd,// 26 PAY  23 

    0x9659df7c,// 27 PAY  24 

    0x9829c20c,// 28 PAY  25 

    0x84fa34a9,// 29 PAY  26 

    0x4eb88b19,// 30 PAY  27 

    0xd8b259b4,// 31 PAY  28 

    0x54c89cc8,// 32 PAY  29 

    0x2386f637,// 33 PAY  30 

    0xc6b2a4fb,// 34 PAY  31 

    0x851212e9,// 35 PAY  32 

    0xe44ccaf0,// 36 PAY  33 

    0x2f198a87,// 37 PAY  34 

    0x54b0302c,// 38 PAY  35 

    0x32c52d7e,// 39 PAY  36 

    0x9efe868d,// 40 PAY  37 

    0x5c01c603,// 41 PAY  38 

    0x1c902749,// 42 PAY  39 

    0xfed5a404,// 43 PAY  40 

    0x911c2596,// 44 PAY  41 

    0xd519f9c3,// 45 PAY  42 

    0x7ba74924,// 46 PAY  43 

    0x30f1e850,// 47 PAY  44 

    0x75aa4df9,// 48 PAY  45 

    0xd25ae7db,// 49 PAY  46 

    0xb4d22dc3,// 50 PAY  47 

    0x3677d077,// 51 PAY  48 

    0x8dc77e7b,// 52 PAY  49 

    0xa7486688,// 53 PAY  50 

    0x860444be,// 54 PAY  51 

    0xba7c1566,// 55 PAY  52 

    0xb69572c3,// 56 PAY  53 

    0x69c810c2,// 57 PAY  54 

    0x67d718ad,// 58 PAY  55 

    0xea9c8842,// 59 PAY  56 

    0x09305bdc,// 60 PAY  57 

    0x912c0c86,// 61 PAY  58 

    0x63da0242,// 62 PAY  59 

    0x7f8ba8ae,// 63 PAY  60 

    0x45989fd7,// 64 PAY  61 

    0x7510dc33,// 65 PAY  62 

    0xd7480b7c,// 66 PAY  63 

    0xd491ddbd,// 67 PAY  64 

    0x3201349f,// 68 PAY  65 

    0x76d1a4bf,// 69 PAY  66 

    0x0f548645,// 70 PAY  67 

    0x0c883311,// 71 PAY  68 

    0x35ee092f,// 72 PAY  69 

    0x0aaac08b,// 73 PAY  70 

    0x19f3e4ea,// 74 PAY  71 

    0x32c3df05,// 75 PAY  72 

    0x97ac9332,// 76 PAY  73 

    0xfa239f68,// 77 PAY  74 

    0x2c1612e6,// 78 PAY  75 

    0xbd2a83ee,// 79 PAY  76 

    0xeb7f013d,// 80 PAY  77 

    0x6ceeed07,// 81 PAY  78 

    0x7720286f,// 82 PAY  79 

    0xd9d1373d,// 83 PAY  80 

    0x3e180550,// 84 PAY  81 

    0x7c46dee1,// 85 PAY  82 

    0xeb01e524,// 86 PAY  83 

    0x699eb9ed,// 87 PAY  84 

    0x988aecff,// 88 PAY  85 

    0xe644ba38,// 89 PAY  86 

    0x2615674a,// 90 PAY  87 

    0x20b834b7,// 91 PAY  88 

    0xe0ba4a42,// 92 PAY  89 

    0x38ac80f7,// 93 PAY  90 

    0xaad433ec,// 94 PAY  91 

    0x42f22469,// 95 PAY  92 

    0x53fa42dc,// 96 PAY  93 

    0x9453bd45,// 97 PAY  94 

    0x93ba2eaf,// 98 PAY  95 

    0xb682994f,// 99 PAY  96 

    0x83b068fc,// 100 PAY  97 

    0xa5b41e51,// 101 PAY  98 

    0xdd887d15,// 102 PAY  99 

    0x8fff3e3c,// 103 PAY 100 

    0x66b4b4e8,// 104 PAY 101 

    0x9601e50a,// 105 PAY 102 

    0x873a56b5,// 106 PAY 103 

    0x166882a9,// 107 PAY 104 

    0x49b9cf85,// 108 PAY 105 

    0xcf6e8483,// 109 PAY 106 

    0xa78cf770,// 110 PAY 107 

    0x05ce8945,// 111 PAY 108 

    0x374135a6,// 112 PAY 109 

    0x212d4188,// 113 PAY 110 

    0xae535310,// 114 PAY 111 

    0x9139688b,// 115 PAY 112 

    0x60deb26d,// 116 PAY 113 

    0x5207d979,// 117 PAY 114 

    0xcbe0877f,// 118 PAY 115 

    0xfa18289a,// 119 PAY 116 

    0xa95daa48,// 120 PAY 117 

    0x9736457b,// 121 PAY 118 

    0xd8825f9e,// 122 PAY 119 

    0x83eec0fb,// 123 PAY 120 

    0xfe3f5614,// 124 PAY 121 

    0xcc74c195,// 125 PAY 122 

    0x45652dc6,// 126 PAY 123 

    0x93a9f905,// 127 PAY 124 

    0x7d40df5b,// 128 PAY 125 

    0xda808b23,// 129 PAY 126 

    0x2273a7e0,// 130 PAY 127 

    0x88d822b9,// 131 PAY 128 

    0x239390b1,// 132 PAY 129 

    0x18fefc81,// 133 PAY 130 

    0x13de9a55,// 134 PAY 131 

    0x7a81d459,// 135 PAY 132 

    0x1149d76b,// 136 PAY 133 

    0x35a114ff,// 137 PAY 134 

    0xb0f38ded,// 138 PAY 135 

    0x738843df,// 139 PAY 136 

    0x54c5a28a,// 140 PAY 137 

    0x87875e19,// 141 PAY 138 

    0x1b76b5e4,// 142 PAY 139 

    0x62b3e66d,// 143 PAY 140 

    0xdd28ae24,// 144 PAY 141 

    0x89a1a736,// 145 PAY 142 

    0x6bd5d0ab,// 146 PAY 143 

    0x396e83c1,// 147 PAY 144 

    0x24599351,// 148 PAY 145 

    0x4a9553f4,// 149 PAY 146 

    0x41c44bbb,// 150 PAY 147 

    0xab3442c1,// 151 PAY 148 

    0x45c1eb52,// 152 PAY 149 

    0x9e1773ad,// 153 PAY 150 

    0x364ba428,// 154 PAY 151 

    0x7dd9ff0b,// 155 PAY 152 

    0x1a86d6ed,// 156 PAY 153 

    0xc392c523,// 157 PAY 154 

    0xfa0702c7,// 158 PAY 155 

    0xd024a2e9,// 159 PAY 156 

    0x8ab63404,// 160 PAY 157 

    0xc55eed8c,// 161 PAY 158 

    0x3379043b,// 162 PAY 159 

    0xf6898a2d,// 163 PAY 160 

    0x9ed2ab52,// 164 PAY 161 

    0xc9333576,// 165 PAY 162 

    0xfef73de6,// 166 PAY 163 

    0x3b26c890,// 167 PAY 164 

    0x76384d47,// 168 PAY 165 

    0x370aea8c,// 169 PAY 166 

    0x6ef3dd23,// 170 PAY 167 

    0x7b07cc9e,// 171 PAY 168 

    0x318dd2e0,// 172 PAY 169 

    0x0c736d7d,// 173 PAY 170 

    0x18ba78ef,// 174 PAY 171 

    0x0c89e6a9,// 175 PAY 172 

    0x3253742d,// 176 PAY 173 

    0xc0d7163e,// 177 PAY 174 

    0x9cd98513,// 178 PAY 175 

    0x8daa11c5,// 179 PAY 176 

    0xa6c91ff9,// 180 PAY 177 

    0x47bae2cb,// 181 PAY 178 

    0x008f9ac3,// 182 PAY 179 

    0xcbd3cf96,// 183 PAY 180 

    0xff4e5cde,// 184 PAY 181 

    0x7d9ea041,// 185 PAY 182 

    0x242fce91,// 186 PAY 183 

    0xdeb6f7a0,// 187 PAY 184 

    0x75df8bfc,// 188 PAY 185 

    0x4dc50974,// 189 PAY 186 

    0x3f8b9ca1,// 190 PAY 187 

    0xf7af07c9,// 191 PAY 188 

    0x5f32941a,// 192 PAY 189 

    0x742c52f2,// 193 PAY 190 

    0xc5058c71,// 194 PAY 191 

    0x75137d11,// 195 PAY 192 

    0x4fd50419,// 196 PAY 193 

    0x39951bb5,// 197 PAY 194 

    0x8c9ccfbe,// 198 PAY 195 

    0xb79ea3a7,// 199 PAY 196 

    0x77c737d1,// 200 PAY 197 

    0xd3c5f5ac,// 201 PAY 198 

    0x4b3a7fc9,// 202 PAY 199 

    0x549ec002,// 203 PAY 200 

    0x8a41318c,// 204 PAY 201 

    0xaebdf331,// 205 PAY 202 

    0x94def807,// 206 PAY 203 

    0x0721ba9e,// 207 PAY 204 

    0x9fb9340e,// 208 PAY 205 

    0x1c88eefb,// 209 PAY 206 

    0x03506de9,// 210 PAY 207 

    0x555d99df,// 211 PAY 208 

    0xf2895a65,// 212 PAY 209 

    0xe1888162,// 213 PAY 210 

    0x4a9dd23d,// 214 PAY 211 

    0x4dd07f27,// 215 PAY 212 

    0xa0653eb6,// 216 PAY 213 

    0x3a8f5e47,// 217 PAY 214 

    0xe6ff8b65,// 218 PAY 215 

    0x75ae4394,// 219 PAY 216 

    0x28a8dc3f,// 220 PAY 217 

    0x413cb02a,// 221 PAY 218 

    0xa103a87d,// 222 PAY 219 

    0x40ea7f66,// 223 PAY 220 

    0x5465e492,// 224 PAY 221 

    0x2c2c009c,// 225 PAY 222 

    0xdd3a3c6d,// 226 PAY 223 

    0x14bd49bf,// 227 PAY 224 

    0x5e78cc53,// 228 PAY 225 

    0xe25fe4d9,// 229 PAY 226 

    0xc1cfef25,// 230 PAY 227 

    0x6d9f981a,// 231 PAY 228 

    0xf0b0c391,// 232 PAY 229 

    0x04bea3c7,// 233 PAY 230 

    0x05db4639,// 234 PAY 231 

    0xa7d18ea8,// 235 PAY 232 

    0x355f642f,// 236 PAY 233 

    0x54bc070c,// 237 PAY 234 

    0x33abeac3,// 238 PAY 235 

    0x1928742c,// 239 PAY 236 

    0x1f250075,// 240 PAY 237 

    0x466aef84,// 241 PAY 238 

    0x21cc5837,// 242 PAY 239 

    0x074b8fd3,// 243 PAY 240 

    0x47a6f847,// 244 PAY 241 

    0x658e24ff,// 245 PAY 242 

    0x00b73a02,// 246 PAY 243 

    0x4f7339da,// 247 PAY 244 

    0xc282454f,// 248 PAY 245 

    0xb5b1d5a7,// 249 PAY 246 

    0xa97bdb48,// 250 PAY 247 

    0x81022b25,// 251 PAY 248 

    0x7695094d,// 252 PAY 249 

    0xbb3f1111,// 253 PAY 250 

    0x11deb69b,// 254 PAY 251 

    0xb39f2c20,// 255 PAY 252 

    0x0f0bc34b,// 256 PAY 253 

    0x42e1db70,// 257 PAY 254 

    0x7342202f,// 258 PAY 255 

    0xe9e2e623,// 259 PAY 256 

    0x9ba84898,// 260 PAY 257 

    0x0e12dff5,// 261 PAY 258 

    0xb057ac34,// 262 PAY 259 

    0x5cedd0c5,// 263 PAY 260 

    0xd626a9ab,// 264 PAY 261 

    0xb0ec7aaf,// 265 PAY 262 

    0xfd0178ff,// 266 PAY 263 

    0x7fed9abe,// 267 PAY 264 

    0x8881f4e2,// 268 PAY 265 

    0x125dd891,// 269 PAY 266 

    0xf4ded56a,// 270 PAY 267 

    0x81740df4,// 271 PAY 268 

    0xbb4db4d6,// 272 PAY 269 

    0xa385fed4,// 273 PAY 270 

    0x3e675858,// 274 PAY 271 

    0x70964f82,// 275 PAY 272 

    0xdb3fb801,// 276 PAY 273 

    0xc09cc0b9,// 277 PAY 274 

    0xb0aafa82,// 278 PAY 275 

    0x2306a089,// 279 PAY 276 

    0x06e28c83,// 280 PAY 277 

    0x25df08ee,// 281 PAY 278 

    0xb7567483,// 282 PAY 279 

    0xe8030736,// 283 PAY 280 

    0xacefcc21,// 284 PAY 281 

    0x35bb7d3f,// 285 PAY 282 

    0x31ffb37f,// 286 PAY 283 

    0x9211f1c8,// 287 PAY 284 

    0x8eebe800,// 288 PAY 285 

    0xf012b291,// 289 PAY 286 

    0xde346867,// 290 PAY 287 

    0x34d783b8,// 291 PAY 288 

    0xcc2598f9,// 292 PAY 289 

    0x51492d77,// 293 PAY 290 

    0x5dee3cde,// 294 PAY 291 

    0x4ba59959,// 295 PAY 292 

    0xd9fec1b7,// 296 PAY 293 

    0x11e62e02,// 297 PAY 294 

    0xd65401be,// 298 PAY 295 

    0x395debbb,// 299 PAY 296 

    0xf3c28506,// 300 PAY 297 

    0xa3f53cfd,// 301 PAY 298 

    0xd87e5108,// 302 PAY 299 

    0x91a104d2,// 303 PAY 300 

    0x962397ec,// 304 PAY 301 

    0x115fd364,// 305 PAY 302 

    0x9ae54fb7,// 306 PAY 303 

    0xa42928f4,// 307 PAY 304 

    0xd97a09a4,// 308 PAY 305 

    0x3eb5ac60,// 309 PAY 306 

    0x9d6c31e3,// 310 PAY 307 

    0x7f161f86,// 311 PAY 308 

    0x829d0cc2,// 312 PAY 309 

    0x1463e50d,// 313 PAY 310 

    0x48a52503,// 314 PAY 311 

    0xd36e313d,// 315 PAY 312 

    0x7d5b485e,// 316 PAY 313 

    0x1ae1f560,// 317 PAY 314 

    0xab64cedd,// 318 PAY 315 

    0x1ca22df9,// 319 PAY 316 

    0x702a90c8,// 320 PAY 317 

    0xf06dc312,// 321 PAY 318 

    0x828d1597,// 322 PAY 319 

    0xec152120,// 323 PAY 320 

    0x13ba4eb2,// 324 PAY 321 

    0xdea27076,// 325 PAY 322 

    0x74e0ce3b,// 326 PAY 323 

    0x5cf10940,// 327 PAY 324 

    0x0d9cf96d,// 328 PAY 325 

    0x329dbd1d,// 329 PAY 326 

    0xc68b692d,// 330 PAY 327 

    0x9f996da9,// 331 PAY 328 

    0x7b0efda4,// 332 PAY 329 

    0x8acae8a0,// 333 PAY 330 

    0x59a27f3d,// 334 PAY 331 

    0x92219201,// 335 PAY 332 

    0x6f3d20ef,// 336 PAY 333 

    0x65a9e7fa,// 337 PAY 334 

    0xf5fc617c,// 338 PAY 335 

    0xb3350b9b,// 339 PAY 336 

    0xaa6b7810,// 340 PAY 337 

    0x76192791,// 341 PAY 338 

    0x1ee851f8,// 342 PAY 339 

    0xf823a98e,// 343 PAY 340 

    0x5aee0612,// 344 PAY 341 

    0x1f9d45e0,// 345 PAY 342 

    0x1e5c3a7c,// 346 PAY 343 

    0xac9824cc,// 347 PAY 344 

    0x67c87c53,// 348 PAY 345 

    0x579db729,// 349 PAY 346 

    0xf9a7c765,// 350 PAY 347 

    0x0f2364d5,// 351 PAY 348 

    0x814a560c,// 352 PAY 349 

    0xf966d51a,// 353 PAY 350 

    0x33f406c7,// 354 PAY 351 

    0x928b76c4,// 355 PAY 352 

    0x3ceb969d,// 356 PAY 353 

    0xef02810c,// 357 PAY 354 

    0x71bdfd7e,// 358 PAY 355 

    0x6d8b6e33,// 359 PAY 356 

    0xe05ed2a5,// 360 PAY 357 

    0x827ab894,// 361 PAY 358 

    0x3cb7d623,// 362 PAY 359 

    0xc82381f7,// 363 PAY 360 

    0xaeaa006f,// 364 PAY 361 

    0xcb2aca73,// 365 PAY 362 

    0xb8d14525,// 366 PAY 363 

    0x8ad85d02,// 367 PAY 364 

    0x42c4d918,// 368 PAY 365 

    0x0b63e2ac,// 369 PAY 366 

    0x5c9f74f8,// 370 PAY 367 

    0x3b9acbd5,// 371 PAY 368 

    0x205777cb,// 372 PAY 369 

    0x2dfc0a1d,// 373 PAY 370 

    0xabde0330,// 374 PAY 371 

    0x328ab928,// 375 PAY 372 

    0x330d048f,// 376 PAY 373 

    0x83f9a90f,// 377 PAY 374 

    0xc1350113,// 378 PAY 375 

    0x9ee00463,// 379 PAY 376 

    0xbede3308,// 380 PAY 377 

    0xd0a200cb,// 381 PAY 378 

    0x7bb7c2c1,// 382 PAY 379 

    0xd32570a5,// 383 PAY 380 

    0x6cdf35a2,// 384 PAY 381 

    0x64853ed7,// 385 PAY 382 

    0x0dd2a883,// 386 PAY 383 

    0x0d1160fc,// 387 PAY 384 

    0x214ce74a,// 388 PAY 385 

    0x7ac2ba26,// 389 PAY 386 

    0xbaff9629,// 390 PAY 387 

    0xdcb9eb94,// 391 PAY 388 

    0x3f2b719f,// 392 PAY 389 

    0x3e71063d,// 393 PAY 390 

    0xf3fedc69,// 394 PAY 391 

    0x20adbd09,// 395 PAY 392 

    0x6e5d1837,// 396 PAY 393 

    0xbda24d5c,// 397 PAY 394 

    0x0771450c,// 398 PAY 395 

    0x0c15ad50,// 399 PAY 396 

    0xdb61e30b,// 400 PAY 397 

    0x17392aae,// 401 PAY 398 

    0x2ab0080c,// 402 PAY 399 

    0xd3c050cc,// 403 PAY 400 

    0xd98cf59b,// 404 PAY 401 

    0xcfc9a6a2,// 405 PAY 402 

    0x9821f72e,// 406 PAY 403 

    0x28e31241,// 407 PAY 404 

    0x1e9b67f6,// 408 PAY 405 

    0x30796ff2,// 409 PAY 406 

    0xee0c338a,// 410 PAY 407 

    0x9c542b96,// 411 PAY 408 

    0x75c2f449,// 412 PAY 409 

    0xcf150b61,// 413 PAY 410 

    0xd3744b9c,// 414 PAY 411 

    0xd0df6404,// 415 PAY 412 

    0x2c24d5be,// 416 PAY 413 

    0x6c186706,// 417 PAY 414 

    0x9072cdb8,// 418 PAY 415 

    0x902a7a9a,// 419 PAY 416 

    0x32de6dc0,// 420 PAY 417 

    0x204d2cb3,// 421 PAY 418 

    0x31a6111a,// 422 PAY 419 

    0x02cba000,// 423 PAY 420 

    0x773cea93,// 424 PAY 421 

    0x7fb6dbff,// 425 PAY 422 

    0xe4e185d4,// 426 PAY 423 

    0x9d203b7b,// 427 PAY 424 

    0xe657c800,// 428 PAY 425 

    0xa3055343,// 429 PAY 426 

    0x717fbdeb,// 430 PAY 427 

    0xf321761c,// 431 PAY 428 

    0x34640b2b,// 432 PAY 429 

    0x96263ffa,// 433 PAY 430 

    0x9ddd91ca,// 434 PAY 431 

    0x9822762e,// 435 PAY 432 

    0x0cb6965b,// 436 PAY 433 

    0xfdc34d63,// 437 PAY 434 

    0xd4a50227,// 438 PAY 435 

    0xd28d91f0,// 439 PAY 436 

    0xce9758ed,// 440 PAY 437 

    0xf9a9049b,// 441 PAY 438 

    0x54f10eb3,// 442 PAY 439 

    0x11258a24,// 443 PAY 440 

    0xf401aca2,// 444 PAY 441 

    0x7a91864d,// 445 PAY 442 

    0x1aba83ae,// 446 PAY 443 

    0xe26fc71b,// 447 PAY 444 

    0xaf42e99a,// 448 PAY 445 

    0x96472673,// 449 PAY 446 

    0x6abb2e51,// 450 PAY 447 

    0x08da13aa,// 451 PAY 448 

    0xac6fce0a,// 452 PAY 449 

    0x3e2886eb,// 453 PAY 450 

    0x6882da17,// 454 PAY 451 

    0xf748beb3,// 455 PAY 452 

    0xcd5eea60,// 456 PAY 453 

    0x33365521,// 457 PAY 454 

    0xf5015dd4,// 458 PAY 455 

    0x0a0eea7d,// 459 PAY 456 

    0xc67fa394,// 460 PAY 457 

    0x9dde551d,// 461 PAY 458 

    0x52739c5c,// 462 PAY 459 

    0xa47df26e,// 463 PAY 460 

    0xf274ffa8,// 464 PAY 461 

    0x964331d8,// 465 PAY 462 

    0x9a7dbc09,// 466 PAY 463 

    0xa0c28fa9,// 467 PAY 464 

    0x8142e8f7,// 468 PAY 465 

    0x3d43cb69,// 469 PAY 466 

    0xb4c0b742,// 470 PAY 467 

    0x1b2eb5fe,// 471 PAY 468 

    0x59565907,// 472 PAY 469 

    0x170a7ef4,// 473 PAY 470 

    0x24e01dfc,// 474 PAY 471 

    0x8d13d4e9,// 475 PAY 472 

    0xf7c73947,// 476 PAY 473 

    0x64ed7c8f,// 477 PAY 474 

    0x6a0996a4,// 478 PAY 475 

    0xcd11f3b0,// 479 PAY 476 

    0xf02a6720,// 480 PAY 477 

    0xed6f3ce0,// 481 PAY 478 

    0x26beaca9,// 482 PAY 479 

    0x72aa3bce,// 483 PAY 480 

    0x0200c54c,// 484 PAY 481 

    0x9c000000,// 485 PAY 482 

/// STA is 1 words. 

/// STA num_pkts       : 154 

/// STA pkt_idx        : 101 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4a 

    0x01954a9a // 486 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt42_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 434 words. 

/// BDA size     is 1730 (0x6c2) 

/// BDA id       is 0x80ec 

    0x06c280ec,// 3 BDA   1 

/// PAY Generic Data size   : 1730 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xb39983fe,// 4 PAY   1 

    0xafb6f4b7,// 5 PAY   2 

    0x224df8b2,// 6 PAY   3 

    0xa016041a,// 7 PAY   4 

    0xbae8eb8e,// 8 PAY   5 

    0x6f4512c5,// 9 PAY   6 

    0x76693b02,// 10 PAY   7 

    0x8468a200,// 11 PAY   8 

    0x733a41e6,// 12 PAY   9 

    0x12a0b459,// 13 PAY  10 

    0x2bfa143f,// 14 PAY  11 

    0x312c6ab0,// 15 PAY  12 

    0x81942601,// 16 PAY  13 

    0x551127d8,// 17 PAY  14 

    0x8252b2a8,// 18 PAY  15 

    0x9c2afdf3,// 19 PAY  16 

    0x27ae9e7e,// 20 PAY  17 

    0x65454d4c,// 21 PAY  18 

    0x46849e18,// 22 PAY  19 

    0xa4619881,// 23 PAY  20 

    0xb771b759,// 24 PAY  21 

    0x16668b94,// 25 PAY  22 

    0x79421149,// 26 PAY  23 

    0x4af2f6dd,// 27 PAY  24 

    0xed6dc68d,// 28 PAY  25 

    0xdaad4a8e,// 29 PAY  26 

    0xa7155030,// 30 PAY  27 

    0x1331bd54,// 31 PAY  28 

    0x31392f80,// 32 PAY  29 

    0xb2e19aac,// 33 PAY  30 

    0xe18deef5,// 34 PAY  31 

    0x5d9475d1,// 35 PAY  32 

    0xe7b5c7d0,// 36 PAY  33 

    0xd7a0f0c5,// 37 PAY  34 

    0x67278727,// 38 PAY  35 

    0xaebde2f2,// 39 PAY  36 

    0x24ee943e,// 40 PAY  37 

    0xe8b04842,// 41 PAY  38 

    0x6518525e,// 42 PAY  39 

    0xdd75bb16,// 43 PAY  40 

    0x22b050fb,// 44 PAY  41 

    0x4563c637,// 45 PAY  42 

    0x0eaa38db,// 46 PAY  43 

    0x7c2fc9a5,// 47 PAY  44 

    0x479248b2,// 48 PAY  45 

    0x44ebd4fb,// 49 PAY  46 

    0x9f183e45,// 50 PAY  47 

    0xbb07d7bb,// 51 PAY  48 

    0x457979b7,// 52 PAY  49 

    0x5839fd17,// 53 PAY  50 

    0x9a732749,// 54 PAY  51 

    0x8d22a911,// 55 PAY  52 

    0xafc31664,// 56 PAY  53 

    0xf5fe3f73,// 57 PAY  54 

    0x57e44ec2,// 58 PAY  55 

    0x27d28d87,// 59 PAY  56 

    0x9d98f73d,// 60 PAY  57 

    0x029a425c,// 61 PAY  58 

    0x04eaff92,// 62 PAY  59 

    0x80271b41,// 63 PAY  60 

    0xcfa26788,// 64 PAY  61 

    0x380466c2,// 65 PAY  62 

    0x2e263846,// 66 PAY  63 

    0x16b4abec,// 67 PAY  64 

    0xdd5af7b0,// 68 PAY  65 

    0x3e9e7bf6,// 69 PAY  66 

    0xba409470,// 70 PAY  67 

    0x92eafa63,// 71 PAY  68 

    0xd79767e6,// 72 PAY  69 

    0x57bff995,// 73 PAY  70 

    0x415bad44,// 74 PAY  71 

    0x93099960,// 75 PAY  72 

    0xc8e62af1,// 76 PAY  73 

    0xfb2ed1cb,// 77 PAY  74 

    0xc33e525a,// 78 PAY  75 

    0x2109f6b6,// 79 PAY  76 

    0x290f4808,// 80 PAY  77 

    0x6218adcc,// 81 PAY  78 

    0x27e0b7a7,// 82 PAY  79 

    0xedee4215,// 83 PAY  80 

    0xb7de7378,// 84 PAY  81 

    0xba9c5c98,// 85 PAY  82 

    0x9dcb2f10,// 86 PAY  83 

    0x4c5d7fa1,// 87 PAY  84 

    0x32e5c0ba,// 88 PAY  85 

    0x0188ce33,// 89 PAY  86 

    0x98ce0783,// 90 PAY  87 

    0xc6344793,// 91 PAY  88 

    0x2cf2482f,// 92 PAY  89 

    0x550b7b47,// 93 PAY  90 

    0x780c7f51,// 94 PAY  91 

    0xe1f4729b,// 95 PAY  92 

    0x2633d187,// 96 PAY  93 

    0xb5d4f57a,// 97 PAY  94 

    0x189d745d,// 98 PAY  95 

    0x27403ded,// 99 PAY  96 

    0x46234feb,// 100 PAY  97 

    0x9fcb4ad9,// 101 PAY  98 

    0x3ae09a15,// 102 PAY  99 

    0x9b02c89c,// 103 PAY 100 

    0xf3f7b9c6,// 104 PAY 101 

    0xfef43b61,// 105 PAY 102 

    0x3f3f5b77,// 106 PAY 103 

    0x2a549129,// 107 PAY 104 

    0x78751911,// 108 PAY 105 

    0xdf8459c4,// 109 PAY 106 

    0x42d45e47,// 110 PAY 107 

    0xd6490753,// 111 PAY 108 

    0x84a81923,// 112 PAY 109 

    0x4baaf836,// 113 PAY 110 

    0x13f292f3,// 114 PAY 111 

    0x451e7f79,// 115 PAY 112 

    0xd2099929,// 116 PAY 113 

    0x350e9266,// 117 PAY 114 

    0x537e1bc3,// 118 PAY 115 

    0x32378e1d,// 119 PAY 116 

    0x01dac972,// 120 PAY 117 

    0xce7e449d,// 121 PAY 118 

    0x4d3dee7d,// 122 PAY 119 

    0x3023673e,// 123 PAY 120 

    0x5e6674b0,// 124 PAY 121 

    0x1187d712,// 125 PAY 122 

    0x8cf07e65,// 126 PAY 123 

    0x2ee64a83,// 127 PAY 124 

    0xa48ed5fa,// 128 PAY 125 

    0x584cf17c,// 129 PAY 126 

    0x3e5a436f,// 130 PAY 127 

    0x03beadd2,// 131 PAY 128 

    0x4fb74703,// 132 PAY 129 

    0xcc089168,// 133 PAY 130 

    0x4bedf1bb,// 134 PAY 131 

    0x333a7eac,// 135 PAY 132 

    0x0f63ff0e,// 136 PAY 133 

    0x0abe9eec,// 137 PAY 134 

    0x80af7b8d,// 138 PAY 135 

    0x9555257e,// 139 PAY 136 

    0x24cc89af,// 140 PAY 137 

    0xb42c102d,// 141 PAY 138 

    0xde2a4114,// 142 PAY 139 

    0xe3324b57,// 143 PAY 140 

    0x66242fb7,// 144 PAY 141 

    0xc0506af2,// 145 PAY 142 

    0x8db95b4f,// 146 PAY 143 

    0x0c3b0b81,// 147 PAY 144 

    0x718838d0,// 148 PAY 145 

    0xd8df5735,// 149 PAY 146 

    0x60b6cb42,// 150 PAY 147 

    0x2060a0e8,// 151 PAY 148 

    0xbcc28a99,// 152 PAY 149 

    0xf6ad33d9,// 153 PAY 150 

    0x97d9c481,// 154 PAY 151 

    0x042c6698,// 155 PAY 152 

    0xe8509602,// 156 PAY 153 

    0x06079396,// 157 PAY 154 

    0xa620e790,// 158 PAY 155 

    0xde9b5637,// 159 PAY 156 

    0xb6325983,// 160 PAY 157 

    0x082c1eb6,// 161 PAY 158 

    0x2e30e5f1,// 162 PAY 159 

    0xc179f7af,// 163 PAY 160 

    0x0a39ae18,// 164 PAY 161 

    0x1fb87e59,// 165 PAY 162 

    0xd28387f1,// 166 PAY 163 

    0xce621a74,// 167 PAY 164 

    0xcd16355f,// 168 PAY 165 

    0x9cf5d1be,// 169 PAY 166 

    0x7b8f3320,// 170 PAY 167 

    0xfe8ec272,// 171 PAY 168 

    0x45d9d8aa,// 172 PAY 169 

    0x0ebaa0d0,// 173 PAY 170 

    0xd195ee1f,// 174 PAY 171 

    0xa3bc7a8a,// 175 PAY 172 

    0xa1db15df,// 176 PAY 173 

    0x5880fac9,// 177 PAY 174 

    0x94dc6ea9,// 178 PAY 175 

    0x038f8cee,// 179 PAY 176 

    0xab39071a,// 180 PAY 177 

    0xa154c690,// 181 PAY 178 

    0x75f366f2,// 182 PAY 179 

    0xe3c84ef5,// 183 PAY 180 

    0x1fa50d71,// 184 PAY 181 

    0x8378cfbe,// 185 PAY 182 

    0xc60bf5a3,// 186 PAY 183 

    0x5b0c17fa,// 187 PAY 184 

    0x8187541b,// 188 PAY 185 

    0x5b9345e8,// 189 PAY 186 

    0x8be981c7,// 190 PAY 187 

    0x7d1a7134,// 191 PAY 188 

    0x4bb69cd4,// 192 PAY 189 

    0x5da43805,// 193 PAY 190 

    0x038b052e,// 194 PAY 191 

    0x8f84cf4e,// 195 PAY 192 

    0x0f5307f4,// 196 PAY 193 

    0x38982032,// 197 PAY 194 

    0x3ddeb43b,// 198 PAY 195 

    0x98577a63,// 199 PAY 196 

    0x6b28db93,// 200 PAY 197 

    0x90a676c1,// 201 PAY 198 

    0x29c2bcee,// 202 PAY 199 

    0xe3bb94b7,// 203 PAY 200 

    0x028755d0,// 204 PAY 201 

    0x970faebc,// 205 PAY 202 

    0xd8ce8418,// 206 PAY 203 

    0xd6dd8625,// 207 PAY 204 

    0xef5a31c5,// 208 PAY 205 

    0xcf17ed95,// 209 PAY 206 

    0x1a08704d,// 210 PAY 207 

    0xa5819576,// 211 PAY 208 

    0xc1a65565,// 212 PAY 209 

    0xd2e5d32d,// 213 PAY 210 

    0x9f6b808e,// 214 PAY 211 

    0x8dfaeb8e,// 215 PAY 212 

    0x28b5a601,// 216 PAY 213 

    0x1388cbef,// 217 PAY 214 

    0x1ed77fbf,// 218 PAY 215 

    0xbcdf0dae,// 219 PAY 216 

    0x2ba105d2,// 220 PAY 217 

    0xbb13183d,// 221 PAY 218 

    0x90400422,// 222 PAY 219 

    0x9b59667c,// 223 PAY 220 

    0x01789759,// 224 PAY 221 

    0x0318bd1d,// 225 PAY 222 

    0x1ce7fcec,// 226 PAY 223 

    0x4db19c9a,// 227 PAY 224 

    0xc63c42dd,// 228 PAY 225 

    0xca507922,// 229 PAY 226 

    0x2d1244a9,// 230 PAY 227 

    0xb47056be,// 231 PAY 228 

    0x7ccad4ad,// 232 PAY 229 

    0xa77f1edb,// 233 PAY 230 

    0xb7ece2ab,// 234 PAY 231 

    0x521dd754,// 235 PAY 232 

    0x61f40b6f,// 236 PAY 233 

    0x8d96cf0c,// 237 PAY 234 

    0x52d6b1b4,// 238 PAY 235 

    0x2baaacaf,// 239 PAY 236 

    0xbe952cd6,// 240 PAY 237 

    0x36578f99,// 241 PAY 238 

    0x5aa004e4,// 242 PAY 239 

    0x2d9e616e,// 243 PAY 240 

    0x8c451afe,// 244 PAY 241 

    0xba4a2b44,// 245 PAY 242 

    0xbf29008e,// 246 PAY 243 

    0x31345e39,// 247 PAY 244 

    0x63413b54,// 248 PAY 245 

    0xf0907aab,// 249 PAY 246 

    0x07bb3afc,// 250 PAY 247 

    0xed4da642,// 251 PAY 248 

    0x8b8d71c7,// 252 PAY 249 

    0x467cd067,// 253 PAY 250 

    0x0813e7fc,// 254 PAY 251 

    0xaa95d313,// 255 PAY 252 

    0xab632715,// 256 PAY 253 

    0xf1ed6ec3,// 257 PAY 254 

    0xb1d6b8f9,// 258 PAY 255 

    0xc9683171,// 259 PAY 256 

    0xb91ece3a,// 260 PAY 257 

    0xa5913669,// 261 PAY 258 

    0x41647c55,// 262 PAY 259 

    0xe7a11595,// 263 PAY 260 

    0xceb8ebd1,// 264 PAY 261 

    0xe9db8d59,// 265 PAY 262 

    0x49a5cd72,// 266 PAY 263 

    0x3b2094ef,// 267 PAY 264 

    0xf2abdf65,// 268 PAY 265 

    0xc04c8c59,// 269 PAY 266 

    0x5e6b90f9,// 270 PAY 267 

    0xedab8c0b,// 271 PAY 268 

    0xf304413c,// 272 PAY 269 

    0xb3ea35cf,// 273 PAY 270 

    0x4f59ed5a,// 274 PAY 271 

    0x9a0b754e,// 275 PAY 272 

    0x3a5fe650,// 276 PAY 273 

    0xa8309d17,// 277 PAY 274 

    0x7b23abc2,// 278 PAY 275 

    0x5c3456fe,// 279 PAY 276 

    0x445a1dec,// 280 PAY 277 

    0xf49b3946,// 281 PAY 278 

    0xf7db3bf3,// 282 PAY 279 

    0x216f6dfc,// 283 PAY 280 

    0xa6a22837,// 284 PAY 281 

    0xeb7a0c87,// 285 PAY 282 

    0xbbba57b7,// 286 PAY 283 

    0xef432e9d,// 287 PAY 284 

    0x2fbd2842,// 288 PAY 285 

    0x571dee0e,// 289 PAY 286 

    0x3fa8a6d0,// 290 PAY 287 

    0x56e47f2b,// 291 PAY 288 

    0x4d6f53f4,// 292 PAY 289 

    0xf9a6007c,// 293 PAY 290 

    0x9a71b0f0,// 294 PAY 291 

    0x5f5c9d98,// 295 PAY 292 

    0x6df09d96,// 296 PAY 293 

    0x12070a84,// 297 PAY 294 

    0x6d9e033d,// 298 PAY 295 

    0x3189d2d6,// 299 PAY 296 

    0x9113c646,// 300 PAY 297 

    0xe96321cd,// 301 PAY 298 

    0x77c506ea,// 302 PAY 299 

    0xce73bbc2,// 303 PAY 300 

    0x3db1a82d,// 304 PAY 301 

    0x19c117d2,// 305 PAY 302 

    0xfbd9cf7d,// 306 PAY 303 

    0x002c605e,// 307 PAY 304 

    0xf12e928f,// 308 PAY 305 

    0x724ecbd9,// 309 PAY 306 

    0xcc35c398,// 310 PAY 307 

    0xf028ed41,// 311 PAY 308 

    0x6b4b007b,// 312 PAY 309 

    0x657cd24e,// 313 PAY 310 

    0xb42f5542,// 314 PAY 311 

    0xfd467ff3,// 315 PAY 312 

    0x54fb0b6f,// 316 PAY 313 

    0x67871b50,// 317 PAY 314 

    0xfd03caf4,// 318 PAY 315 

    0xc545fb21,// 319 PAY 316 

    0xf5ba41b8,// 320 PAY 317 

    0x9126625c,// 321 PAY 318 

    0x43b5987d,// 322 PAY 319 

    0xb326d9bd,// 323 PAY 320 

    0x5d831dbe,// 324 PAY 321 

    0x47e47596,// 325 PAY 322 

    0x51b6a113,// 326 PAY 323 

    0xd56bcbb8,// 327 PAY 324 

    0x3c5db9c4,// 328 PAY 325 

    0xba4013bd,// 329 PAY 326 

    0xf91fc504,// 330 PAY 327 

    0xe975aea7,// 331 PAY 328 

    0x15f34258,// 332 PAY 329 

    0xfb5c59b4,// 333 PAY 330 

    0xa94ef4b5,// 334 PAY 331 

    0x7808dbb9,// 335 PAY 332 

    0xf0954f7f,// 336 PAY 333 

    0x0b1982a5,// 337 PAY 334 

    0x1e715b86,// 338 PAY 335 

    0x3b5a912a,// 339 PAY 336 

    0x36e648b8,// 340 PAY 337 

    0xf3b5aed7,// 341 PAY 338 

    0x12b21388,// 342 PAY 339 

    0x97dbada6,// 343 PAY 340 

    0x704e48c1,// 344 PAY 341 

    0x80bca4e4,// 345 PAY 342 

    0x2515e63b,// 346 PAY 343 

    0x35331d02,// 347 PAY 344 

    0xddc18d72,// 348 PAY 345 

    0x421405ae,// 349 PAY 346 

    0xeae95f10,// 350 PAY 347 

    0x6a572a1d,// 351 PAY 348 

    0xb52d4299,// 352 PAY 349 

    0x85c3e0a1,// 353 PAY 350 

    0xd7289f9f,// 354 PAY 351 

    0xf2ba5f2e,// 355 PAY 352 

    0x6b884540,// 356 PAY 353 

    0x2fa06065,// 357 PAY 354 

    0x39fdf16d,// 358 PAY 355 

    0x371090f6,// 359 PAY 356 

    0x2ca997ef,// 360 PAY 357 

    0xbb6f811a,// 361 PAY 358 

    0xc35c89cb,// 362 PAY 359 

    0xcff5c7f8,// 363 PAY 360 

    0x551b6e01,// 364 PAY 361 

    0xa54f62a4,// 365 PAY 362 

    0xdf2dc3a4,// 366 PAY 363 

    0xec7f8912,// 367 PAY 364 

    0x8321e42f,// 368 PAY 365 

    0x54cd4dfb,// 369 PAY 366 

    0xa3329e31,// 370 PAY 367 

    0xba9861d2,// 371 PAY 368 

    0xc08dd39a,// 372 PAY 369 

    0x8633ffc1,// 373 PAY 370 

    0x5ce40775,// 374 PAY 371 

    0x1763c9ce,// 375 PAY 372 

    0xefcd67c1,// 376 PAY 373 

    0xb030f967,// 377 PAY 374 

    0x1bb72190,// 378 PAY 375 

    0x461d18d4,// 379 PAY 376 

    0x8b59e0c9,// 380 PAY 377 

    0x82cf4931,// 381 PAY 378 

    0xa3af60af,// 382 PAY 379 

    0x69cf47bf,// 383 PAY 380 

    0xfd78a632,// 384 PAY 381 

    0x0646a990,// 385 PAY 382 

    0x227be8b9,// 386 PAY 383 

    0x85c8f7e8,// 387 PAY 384 

    0x83fd4b63,// 388 PAY 385 

    0x946c943e,// 389 PAY 386 

    0x375400f4,// 390 PAY 387 

    0x7218ca51,// 391 PAY 388 

    0x0daf7f68,// 392 PAY 389 

    0x42311bb8,// 393 PAY 390 

    0xe0fb23a3,// 394 PAY 391 

    0x3f8bb952,// 395 PAY 392 

    0x522ac678,// 396 PAY 393 

    0x286c9c0c,// 397 PAY 394 

    0x30da94c3,// 398 PAY 395 

    0x71708bbe,// 399 PAY 396 

    0xe2106c4b,// 400 PAY 397 

    0x54932165,// 401 PAY 398 

    0xcc800727,// 402 PAY 399 

    0x76ac2377,// 403 PAY 400 

    0x51ac10ab,// 404 PAY 401 

    0xd011a1ca,// 405 PAY 402 

    0x3b82ccba,// 406 PAY 403 

    0x01751d84,// 407 PAY 404 

    0x5f98608a,// 408 PAY 405 

    0xae209e55,// 409 PAY 406 

    0x2c50f25f,// 410 PAY 407 

    0x9fa00589,// 411 PAY 408 

    0xcb7401a9,// 412 PAY 409 

    0x259a3941,// 413 PAY 410 

    0x84ab5c28,// 414 PAY 411 

    0x0cfe8364,// 415 PAY 412 

    0x7c26fd82,// 416 PAY 413 

    0xb51a9762,// 417 PAY 414 

    0x6e9fafa2,// 418 PAY 415 

    0x05824db3,// 419 PAY 416 

    0x6377ae4d,// 420 PAY 417 

    0xc38c2684,// 421 PAY 418 

    0x1db16de1,// 422 PAY 419 

    0xf3d655e6,// 423 PAY 420 

    0x9224a0a5,// 424 PAY 421 

    0x6a69c819,// 425 PAY 422 

    0xb6616399,// 426 PAY 423 

    0xb430d010,// 427 PAY 424 

    0xeedaa74a,// 428 PAY 425 

    0x8378dbe2,// 429 PAY 426 

    0xa5077bd2,// 430 PAY 427 

    0x1d4b2ad8,// 431 PAY 428 

    0x1e38d0bc,// 432 PAY 429 

    0x12a989b9,// 433 PAY 430 

    0xa868c363,// 434 PAY 431 

    0x5bb6105b,// 435 PAY 432 

    0x7dd40000,// 436 PAY 433 

/// HASH is  12 bytes 

    0x76ac2377,// 437 HSH   1 

    0x51ac10ab,// 438 HSH   2 

    0xd011a1ca,// 439 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 54 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8b 

    0x00d98b2d // 440 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt43_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 185 words. 

/// BDA size     is 736 (0x2e0) 

/// BDA id       is 0x6967 

    0x02e06967,// 3 BDA   1 

/// PAY Generic Data size   : 736 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x134beba6,// 4 PAY   1 

    0xfbef10be,// 5 PAY   2 

    0x6792b7ce,// 6 PAY   3 

    0x457b9c67,// 7 PAY   4 

    0xb4eb98ea,// 8 PAY   5 

    0xd93ad498,// 9 PAY   6 

    0x0ed7aa63,// 10 PAY   7 

    0x378af3e3,// 11 PAY   8 

    0x246da7e8,// 12 PAY   9 

    0x60c3bf9c,// 13 PAY  10 

    0xed8fc041,// 14 PAY  11 

    0x1cb6eea2,// 15 PAY  12 

    0x8ff67d24,// 16 PAY  13 

    0xa749f975,// 17 PAY  14 

    0x7e1ab472,// 18 PAY  15 

    0x452f26a5,// 19 PAY  16 

    0x8580144f,// 20 PAY  17 

    0xa2f8fdd6,// 21 PAY  18 

    0xb4b9e9fa,// 22 PAY  19 

    0xe9f02e92,// 23 PAY  20 

    0x40ebe005,// 24 PAY  21 

    0x657fb197,// 25 PAY  22 

    0x88ad682e,// 26 PAY  23 

    0x1d5e2613,// 27 PAY  24 

    0x05b12924,// 28 PAY  25 

    0x9200e2b0,// 29 PAY  26 

    0xc90a63c8,// 30 PAY  27 

    0x573ea73b,// 31 PAY  28 

    0xea80b8f3,// 32 PAY  29 

    0x9e3f0770,// 33 PAY  30 

    0xb8639ebc,// 34 PAY  31 

    0xed92b507,// 35 PAY  32 

    0x586b7b98,// 36 PAY  33 

    0x4a84c136,// 37 PAY  34 

    0xdc5b0d49,// 38 PAY  35 

    0x2093998b,// 39 PAY  36 

    0x496b136c,// 40 PAY  37 

    0x6a364770,// 41 PAY  38 

    0x7414ec14,// 42 PAY  39 

    0x98f614ea,// 43 PAY  40 

    0x09dc3dae,// 44 PAY  41 

    0xbdd35e24,// 45 PAY  42 

    0x34f17e3b,// 46 PAY  43 

    0x06fc1dba,// 47 PAY  44 

    0x83ea8f80,// 48 PAY  45 

    0x72547452,// 49 PAY  46 

    0xc8eb1eec,// 50 PAY  47 

    0x44336a51,// 51 PAY  48 

    0xde0b6ad6,// 52 PAY  49 

    0x69d18f87,// 53 PAY  50 

    0x46c3fcd0,// 54 PAY  51 

    0xfb43da8f,// 55 PAY  52 

    0xe2cb8ceb,// 56 PAY  53 

    0x74c195e2,// 57 PAY  54 

    0x340d854a,// 58 PAY  55 

    0xc72f3ea1,// 59 PAY  56 

    0xa8d21eb7,// 60 PAY  57 

    0x7303ac0a,// 61 PAY  58 

    0x2393c376,// 62 PAY  59 

    0x6844046d,// 63 PAY  60 

    0x465ff82f,// 64 PAY  61 

    0x37ab40b2,// 65 PAY  62 

    0x7d6656ed,// 66 PAY  63 

    0xa8b298d9,// 67 PAY  64 

    0x408b6a12,// 68 PAY  65 

    0xc6f24628,// 69 PAY  66 

    0x2fc62e33,// 70 PAY  67 

    0xa192778b,// 71 PAY  68 

    0x13ebace5,// 72 PAY  69 

    0x0085284a,// 73 PAY  70 

    0x88ef0ac2,// 74 PAY  71 

    0x2bbbafce,// 75 PAY  72 

    0x83b8cc0e,// 76 PAY  73 

    0xb2a9c31a,// 77 PAY  74 

    0x2b1427c4,// 78 PAY  75 

    0x69824443,// 79 PAY  76 

    0xf9513449,// 80 PAY  77 

    0x95db6ee7,// 81 PAY  78 

    0x13f84ec2,// 82 PAY  79 

    0x48038094,// 83 PAY  80 

    0x8e5775f9,// 84 PAY  81 

    0xde4f3a2d,// 85 PAY  82 

    0xeabb3574,// 86 PAY  83 

    0xb253c34f,// 87 PAY  84 

    0xdd1fab13,// 88 PAY  85 

    0xe210b23e,// 89 PAY  86 

    0x3255a8a1,// 90 PAY  87 

    0xa0b58898,// 91 PAY  88 

    0x4394f052,// 92 PAY  89 

    0xeac0213a,// 93 PAY  90 

    0xfd3e2632,// 94 PAY  91 

    0xfad57328,// 95 PAY  92 

    0xccc01b76,// 96 PAY  93 

    0x04b53b4c,// 97 PAY  94 

    0x75f5b7b4,// 98 PAY  95 

    0xb5a56f5c,// 99 PAY  96 

    0x1ca95ea9,// 100 PAY  97 

    0x9544d2f9,// 101 PAY  98 

    0xd7f2d3b8,// 102 PAY  99 

    0x419a5927,// 103 PAY 100 

    0x98a1c40f,// 104 PAY 101 

    0x9c03a582,// 105 PAY 102 

    0x81928413,// 106 PAY 103 

    0x94d3c29e,// 107 PAY 104 

    0xa415a90c,// 108 PAY 105 

    0x2fbbf626,// 109 PAY 106 

    0x94ec9d4a,// 110 PAY 107 

    0x73140ef0,// 111 PAY 108 

    0x36388186,// 112 PAY 109 

    0x2f760b03,// 113 PAY 110 

    0xdbcd6d38,// 114 PAY 111 

    0x874249f2,// 115 PAY 112 

    0x9d0e9500,// 116 PAY 113 

    0x65ee4efb,// 117 PAY 114 

    0x8b359578,// 118 PAY 115 

    0x40c152a6,// 119 PAY 116 

    0x3392567f,// 120 PAY 117 

    0x4568d00e,// 121 PAY 118 

    0x25b3fd25,// 122 PAY 119 

    0x6bf599fb,// 123 PAY 120 

    0x70fa504f,// 124 PAY 121 

    0xe1893820,// 125 PAY 122 

    0xcd9e0e9e,// 126 PAY 123 

    0xf30fab56,// 127 PAY 124 

    0xaf4ede84,// 128 PAY 125 

    0x2b200f1f,// 129 PAY 126 

    0xd5c67366,// 130 PAY 127 

    0x546dcd44,// 131 PAY 128 

    0x9dc8fcfa,// 132 PAY 129 

    0x1eceaf93,// 133 PAY 130 

    0x04b60a96,// 134 PAY 131 

    0x8abe46be,// 135 PAY 132 

    0x5c587aae,// 136 PAY 133 

    0xf73359cf,// 137 PAY 134 

    0x41b2a41b,// 138 PAY 135 

    0xa2b45ea6,// 139 PAY 136 

    0x4321a8f5,// 140 PAY 137 

    0xc15b1a06,// 141 PAY 138 

    0xf0528d15,// 142 PAY 139 

    0xb5b54bca,// 143 PAY 140 

    0xb8cb2cbf,// 144 PAY 141 

    0x9d5cacbe,// 145 PAY 142 

    0xd72e9175,// 146 PAY 143 

    0x9c1b997e,// 147 PAY 144 

    0xf5f7213d,// 148 PAY 145 

    0x8f8094de,// 149 PAY 146 

    0x13b0db4a,// 150 PAY 147 

    0x4a8a6c61,// 151 PAY 148 

    0x6fd00ed7,// 152 PAY 149 

    0xd3050f12,// 153 PAY 150 

    0xad7171f2,// 154 PAY 151 

    0x254b3c66,// 155 PAY 152 

    0x0f2e0264,// 156 PAY 153 

    0x508b5d96,// 157 PAY 154 

    0x5468db18,// 158 PAY 155 

    0xd94efff9,// 159 PAY 156 

    0xd704fe68,// 160 PAY 157 

    0xf775b442,// 161 PAY 158 

    0xb3086660,// 162 PAY 159 

    0x4f300e70,// 163 PAY 160 

    0x0b641801,// 164 PAY 161 

    0x185e4aee,// 165 PAY 162 

    0xe6d89cc8,// 166 PAY 163 

    0xa74d67cc,// 167 PAY 164 

    0x9fe2f1c9,// 168 PAY 165 

    0xe2f18e16,// 169 PAY 166 

    0x14a6c935,// 170 PAY 167 

    0xff60f44d,// 171 PAY 168 

    0x7f1b3463,// 172 PAY 169 

    0xeb3b47cf,// 173 PAY 170 

    0x936647fd,// 174 PAY 171 

    0xb887dbe5,// 175 PAY 172 

    0xac689cce,// 176 PAY 173 

    0xea63be28,// 177 PAY 174 

    0xc944481e,// 178 PAY 175 

    0x5b708629,// 179 PAY 176 

    0x0fdced9a,// 180 PAY 177 

    0xa7b24777,// 181 PAY 178 

    0xf35b72d7,// 182 PAY 179 

    0xf9fab108,// 183 PAY 180 

    0xe54ad67c,// 184 PAY 181 

    0x8eede568,// 185 PAY 182 

    0x87936013,// 186 PAY 183 

    0x636bf4cd,// 187 PAY 184 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 232 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x79 

    0x03a07985 // 188 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt44_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 73 words. 

/// BDA size     is 286 (0x11e) 

/// BDA id       is 0x4bfd 

    0x011e4bfd,// 3 BDA   1 

/// PAY Generic Data size   : 286 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xdf429a18,// 4 PAY   1 

    0xcd6bf083,// 5 PAY   2 

    0x7d3dcf26,// 6 PAY   3 

    0x9b53df51,// 7 PAY   4 

    0x9cbcc81d,// 8 PAY   5 

    0x75ae911d,// 9 PAY   6 

    0x59113a9d,// 10 PAY   7 

    0x10a68ac6,// 11 PAY   8 

    0xf8090b77,// 12 PAY   9 

    0xba978724,// 13 PAY  10 

    0x9662e1d0,// 14 PAY  11 

    0x56d90733,// 15 PAY  12 

    0xebcc021f,// 16 PAY  13 

    0x1250939a,// 17 PAY  14 

    0x3c255530,// 18 PAY  15 

    0x10cf8d88,// 19 PAY  16 

    0x0bee8364,// 20 PAY  17 

    0xd7fe8cda,// 21 PAY  18 

    0x57bd2903,// 22 PAY  19 

    0x399c6861,// 23 PAY  20 

    0x6a361e52,// 24 PAY  21 

    0x74d41cd8,// 25 PAY  22 

    0xc648ff97,// 26 PAY  23 

    0x0ecf1503,// 27 PAY  24 

    0xc3e08909,// 28 PAY  25 

    0x974a18e5,// 29 PAY  26 

    0x7b6738b6,// 30 PAY  27 

    0x354003e7,// 31 PAY  28 

    0xae25a857,// 32 PAY  29 

    0x920b5877,// 33 PAY  30 

    0xeca00803,// 34 PAY  31 

    0x4fd45ee0,// 35 PAY  32 

    0xdae201f0,// 36 PAY  33 

    0x9b598a20,// 37 PAY  34 

    0x8e243101,// 38 PAY  35 

    0x2f8dcc41,// 39 PAY  36 

    0x9c18be0a,// 40 PAY  37 

    0xd11b4fbc,// 41 PAY  38 

    0xf60d2b2a,// 42 PAY  39 

    0xd5d77d86,// 43 PAY  40 

    0x36600ddf,// 44 PAY  41 

    0x820c1c2a,// 45 PAY  42 

    0x4a5a2e2e,// 46 PAY  43 

    0x0c7eb45d,// 47 PAY  44 

    0x9a7ab530,// 48 PAY  45 

    0x75e168a8,// 49 PAY  46 

    0x77d24981,// 50 PAY  47 

    0x9a62c4a3,// 51 PAY  48 

    0xae726473,// 52 PAY  49 

    0xd0e487c4,// 53 PAY  50 

    0x1ce11a60,// 54 PAY  51 

    0x0b7efdf8,// 55 PAY  52 

    0x3f693d6c,// 56 PAY  53 

    0xd45fee45,// 57 PAY  54 

    0x96ed1ff2,// 58 PAY  55 

    0x550c1ef9,// 59 PAY  56 

    0x10817861,// 60 PAY  57 

    0x93b7c473,// 61 PAY  58 

    0x9a5681ea,// 62 PAY  59 

    0x406fca3b,// 63 PAY  60 

    0xda606442,// 64 PAY  61 

    0xf4bd82c5,// 65 PAY  62 

    0xc169d399,// 66 PAY  63 

    0xa42435d2,// 67 PAY  64 

    0xfe2ae658,// 68 PAY  65 

    0x4487fec2,// 69 PAY  66 

    0x8b8e8f84,// 70 PAY  67 

    0xec58c1ae,// 71 PAY  68 

    0x6aad5b75,// 72 PAY  69 

    0x701470f8,// 73 PAY  70 

    0xf8fc62ae,// 74 PAY  71 

    0xfdf80000,// 75 PAY  72 

/// STA is 1 words. 

/// STA num_pkts       : 13 

/// STA pkt_idx        : 80 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9a 

    0x01419a0d // 76 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt45_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x01 

/// ECH pdu_tag        : 0x00 

    0x00010000,// 2 ECH   1 

/// BDA is 498 words. 

/// BDA size     is 1986 (0x7c2) 

/// BDA id       is 0xae4b 

    0x07c2ae4b,// 3 BDA   1 

/// PAY Generic Data size   : 1986 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x8932ed17,// 4 PAY   1 

    0xfbb66217,// 5 PAY   2 

    0x0f202eeb,// 6 PAY   3 

    0x63a80f7c,// 7 PAY   4 

    0xcb85e53d,// 8 PAY   5 

    0x7678bb0d,// 9 PAY   6 

    0x7fe475fb,// 10 PAY   7 

    0x2de9baa8,// 11 PAY   8 

    0x4cf98e2a,// 12 PAY   9 

    0x0e482aeb,// 13 PAY  10 

    0xd23e3d47,// 14 PAY  11 

    0x6af6ddd0,// 15 PAY  12 

    0xc8c36e56,// 16 PAY  13 

    0x891f9557,// 17 PAY  14 

    0x5c721a39,// 18 PAY  15 

    0x4e4823a4,// 19 PAY  16 

    0xdb1fd5b3,// 20 PAY  17 

    0xca050c27,// 21 PAY  18 

    0xe32e6d91,// 22 PAY  19 

    0xbebb059e,// 23 PAY  20 

    0x76379627,// 24 PAY  21 

    0xaa399a9f,// 25 PAY  22 

    0xd5eb88d3,// 26 PAY  23 

    0xeacebe47,// 27 PAY  24 

    0x5f0227b3,// 28 PAY  25 

    0xc6de821e,// 29 PAY  26 

    0x51a928e7,// 30 PAY  27 

    0xb2011a30,// 31 PAY  28 

    0x43357ebf,// 32 PAY  29 

    0x4c1fd349,// 33 PAY  30 

    0x9f86a36e,// 34 PAY  31 

    0x08c696f3,// 35 PAY  32 

    0x0dfca078,// 36 PAY  33 

    0x1fe9d97c,// 37 PAY  34 

    0xafb9b852,// 38 PAY  35 

    0xea5654b5,// 39 PAY  36 

    0x2cd75c52,// 40 PAY  37 

    0x26478098,// 41 PAY  38 

    0x9cef0be5,// 42 PAY  39 

    0xc9fb8332,// 43 PAY  40 

    0x218a6575,// 44 PAY  41 

    0x1170052e,// 45 PAY  42 

    0xc9b5d832,// 46 PAY  43 

    0x21f0cab1,// 47 PAY  44 

    0x3d4974b4,// 48 PAY  45 

    0xc2462fea,// 49 PAY  46 

    0x38adbfaa,// 50 PAY  47 

    0xb0a14852,// 51 PAY  48 

    0x517f79e9,// 52 PAY  49 

    0xc17560a5,// 53 PAY  50 

    0x276903c7,// 54 PAY  51 

    0x015fe8f9,// 55 PAY  52 

    0x72c97a76,// 56 PAY  53 

    0x9451ab85,// 57 PAY  54 

    0xc362828c,// 58 PAY  55 

    0x548149e9,// 59 PAY  56 

    0x08e25c6b,// 60 PAY  57 

    0xe75308f9,// 61 PAY  58 

    0xdb1a776d,// 62 PAY  59 

    0xf91d9b82,// 63 PAY  60 

    0xf6442a51,// 64 PAY  61 

    0x273ac2eb,// 65 PAY  62 

    0x522d4d13,// 66 PAY  63 

    0xb3741da0,// 67 PAY  64 

    0x2d85a7eb,// 68 PAY  65 

    0x9fc31816,// 69 PAY  66 

    0x4c29d09a,// 70 PAY  67 

    0xb380d867,// 71 PAY  68 

    0x0dc729b2,// 72 PAY  69 

    0x142cc571,// 73 PAY  70 

    0x04e452f6,// 74 PAY  71 

    0xfafdc458,// 75 PAY  72 

    0x7b883444,// 76 PAY  73 

    0xda409e8a,// 77 PAY  74 

    0x18896d17,// 78 PAY  75 

    0x2349c893,// 79 PAY  76 

    0x71c64af7,// 80 PAY  77 

    0x40d9a09d,// 81 PAY  78 

    0x7067f55b,// 82 PAY  79 

    0x827ea673,// 83 PAY  80 

    0x96d303b8,// 84 PAY  81 

    0xf68958ac,// 85 PAY  82 

    0x7447411e,// 86 PAY  83 

    0x8840c891,// 87 PAY  84 

    0xa6386835,// 88 PAY  85 

    0x2ffcb3fd,// 89 PAY  86 

    0x54120f4b,// 90 PAY  87 

    0xe509fc2d,// 91 PAY  88 

    0x704d5ff8,// 92 PAY  89 

    0x4f1d7353,// 93 PAY  90 

    0x5d3c9d92,// 94 PAY  91 

    0xca4375cf,// 95 PAY  92 

    0x9c9b6c67,// 96 PAY  93 

    0x27275e1d,// 97 PAY  94 

    0x5a061552,// 98 PAY  95 

    0x3cd9b318,// 99 PAY  96 

    0x916f1a2e,// 100 PAY  97 

    0xff32b143,// 101 PAY  98 

    0x005e3389,// 102 PAY  99 

    0x140258f8,// 103 PAY 100 

    0x561bd04a,// 104 PAY 101 

    0x5c5e5b4e,// 105 PAY 102 

    0xd4144cb3,// 106 PAY 103 

    0x81a3dadb,// 107 PAY 104 

    0x4fc1e939,// 108 PAY 105 

    0x2d33672d,// 109 PAY 106 

    0xd52ee5f7,// 110 PAY 107 

    0x48af2dd9,// 111 PAY 108 

    0x0b2ec838,// 112 PAY 109 

    0xbff69317,// 113 PAY 110 

    0xaccc2ef9,// 114 PAY 111 

    0x5bf5093b,// 115 PAY 112 

    0x247a2516,// 116 PAY 113 

    0x28f6a36d,// 117 PAY 114 

    0x3dd74b52,// 118 PAY 115 

    0x3195735e,// 119 PAY 116 

    0x52e979ff,// 120 PAY 117 

    0x89ade6b5,// 121 PAY 118 

    0xe7322f0b,// 122 PAY 119 

    0xeabe3365,// 123 PAY 120 

    0x942361be,// 124 PAY 121 

    0xd9d74a09,// 125 PAY 122 

    0x90e0770a,// 126 PAY 123 

    0xc170738f,// 127 PAY 124 

    0x34425192,// 128 PAY 125 

    0xea17db57,// 129 PAY 126 

    0x0626e7e1,// 130 PAY 127 

    0x8f134997,// 131 PAY 128 

    0xe9bdb54e,// 132 PAY 129 

    0xd83ebe98,// 133 PAY 130 

    0x77b53bc0,// 134 PAY 131 

    0x398069da,// 135 PAY 132 

    0xc4ffda70,// 136 PAY 133 

    0x052735ed,// 137 PAY 134 

    0x88514cdb,// 138 PAY 135 

    0x5f0dd220,// 139 PAY 136 

    0x030dea52,// 140 PAY 137 

    0x5b2ab3b6,// 141 PAY 138 

    0x1f258753,// 142 PAY 139 

    0x929c5648,// 143 PAY 140 

    0x2f989819,// 144 PAY 141 

    0x5ef3b2ef,// 145 PAY 142 

    0xd6b432cf,// 146 PAY 143 

    0x0272e312,// 147 PAY 144 

    0x050d337e,// 148 PAY 145 

    0xb2f17f8f,// 149 PAY 146 

    0xa5807d56,// 150 PAY 147 

    0x62fe9cd2,// 151 PAY 148 

    0x60ed242f,// 152 PAY 149 

    0xf5c54c34,// 153 PAY 150 

    0xbcb9b37f,// 154 PAY 151 

    0xbf654a86,// 155 PAY 152 

    0xc39f2b50,// 156 PAY 153 

    0xe0d09f59,// 157 PAY 154 

    0xb91cf0bb,// 158 PAY 155 

    0x98dbca10,// 159 PAY 156 

    0xc78bfb4f,// 160 PAY 157 

    0x5f5ad71a,// 161 PAY 158 

    0x630a419b,// 162 PAY 159 

    0x4365840b,// 163 PAY 160 

    0x11baf089,// 164 PAY 161 

    0xc0567bc6,// 165 PAY 162 

    0x7ff12d4d,// 166 PAY 163 

    0xeac53231,// 167 PAY 164 

    0x25f449ca,// 168 PAY 165 

    0x802ed289,// 169 PAY 166 

    0x9370ad7d,// 170 PAY 167 

    0xa62f5ad2,// 171 PAY 168 

    0xc32578ca,// 172 PAY 169 

    0x9f298d62,// 173 PAY 170 

    0x0b34820a,// 174 PAY 171 

    0xc6fffd8f,// 175 PAY 172 

    0x49ac0fe7,// 176 PAY 173 

    0x58d63510,// 177 PAY 174 

    0x143b954c,// 178 PAY 175 

    0xa9bac148,// 179 PAY 176 

    0x000f5fff,// 180 PAY 177 

    0xd5daae87,// 181 PAY 178 

    0x09c6ce6e,// 182 PAY 179 

    0xb217262e,// 183 PAY 180 

    0x1b1cd025,// 184 PAY 181 

    0x5622b3d5,// 185 PAY 182 

    0x337d63bc,// 186 PAY 183 

    0x6b50c2af,// 187 PAY 184 

    0xd419e629,// 188 PAY 185 

    0x6263d0b9,// 189 PAY 186 

    0x83a23609,// 190 PAY 187 

    0x8d6ffb11,// 191 PAY 188 

    0x7e729025,// 192 PAY 189 

    0x5aa2ee12,// 193 PAY 190 

    0xc933c617,// 194 PAY 191 

    0x369bb190,// 195 PAY 192 

    0x7c2703af,// 196 PAY 193 

    0x2c303c7a,// 197 PAY 194 

    0x05463426,// 198 PAY 195 

    0x6527ce2e,// 199 PAY 196 

    0x9df8c032,// 200 PAY 197 

    0xbe48e16a,// 201 PAY 198 

    0xb81d72e5,// 202 PAY 199 

    0x7bc43e6c,// 203 PAY 200 

    0x9dc2303d,// 204 PAY 201 

    0x5176662b,// 205 PAY 202 

    0x86f9c9d7,// 206 PAY 203 

    0x003c82ae,// 207 PAY 204 

    0xc7eb5a7d,// 208 PAY 205 

    0x10c7fcd8,// 209 PAY 206 

    0xa41b8fb0,// 210 PAY 207 

    0x2e4a1287,// 211 PAY 208 

    0x29e90ad1,// 212 PAY 209 

    0xbbcc038d,// 213 PAY 210 

    0x12cb2d44,// 214 PAY 211 

    0xc864481c,// 215 PAY 212 

    0xeb5188b4,// 216 PAY 213 

    0x6af00bda,// 217 PAY 214 

    0xc4d154fe,// 218 PAY 215 

    0x8807b156,// 219 PAY 216 

    0x52d5cb54,// 220 PAY 217 

    0x65374755,// 221 PAY 218 

    0x0286d5c3,// 222 PAY 219 

    0xf6584bdf,// 223 PAY 220 

    0xc7356b7b,// 224 PAY 221 

    0xf49db22c,// 225 PAY 222 

    0xe27eb533,// 226 PAY 223 

    0xd98ce09d,// 227 PAY 224 

    0x937e9435,// 228 PAY 225 

    0xc7e93a5e,// 229 PAY 226 

    0xe9c649a9,// 230 PAY 227 

    0xfde6c9f6,// 231 PAY 228 

    0x31a9f715,// 232 PAY 229 

    0x9bfc6e1f,// 233 PAY 230 

    0x876e22a8,// 234 PAY 231 

    0xc285cc55,// 235 PAY 232 

    0xc518850e,// 236 PAY 233 

    0x9696c5ee,// 237 PAY 234 

    0x261ee8a3,// 238 PAY 235 

    0xab7e0ea8,// 239 PAY 236 

    0x1a6f5b23,// 240 PAY 237 

    0x4c36dc07,// 241 PAY 238 

    0x8f8b8d4c,// 242 PAY 239 

    0x43f3d06a,// 243 PAY 240 

    0xa851517b,// 244 PAY 241 

    0xa95c68a1,// 245 PAY 242 

    0x11589e92,// 246 PAY 243 

    0xcf21dcf0,// 247 PAY 244 

    0x95b5e00d,// 248 PAY 245 

    0xe62d31bf,// 249 PAY 246 

    0xb95a6e3b,// 250 PAY 247 

    0xc80bc0ef,// 251 PAY 248 

    0x1de1c212,// 252 PAY 249 

    0xfef8aad9,// 253 PAY 250 

    0x846b106c,// 254 PAY 251 

    0x0ac73d3c,// 255 PAY 252 

    0x741fe1cd,// 256 PAY 253 

    0x59a56665,// 257 PAY 254 

    0xfd391d69,// 258 PAY 255 

    0x8f737cfa,// 259 PAY 256 

    0xfe0d9984,// 260 PAY 257 

    0xa7ec14bd,// 261 PAY 258 

    0xda690c89,// 262 PAY 259 

    0x7de9c5f9,// 263 PAY 260 

    0x7ef816d6,// 264 PAY 261 

    0xa05aa525,// 265 PAY 262 

    0x1d90b6f6,// 266 PAY 263 

    0x0b187bf8,// 267 PAY 264 

    0x76c50ff4,// 268 PAY 265 

    0x09f44068,// 269 PAY 266 

    0x6cf0816f,// 270 PAY 267 

    0x28fa8606,// 271 PAY 268 

    0x4804956f,// 272 PAY 269 

    0x81601974,// 273 PAY 270 

    0xa4012d10,// 274 PAY 271 

    0x729cf393,// 275 PAY 272 

    0x22beac4a,// 276 PAY 273 

    0x8e57bf3d,// 277 PAY 274 

    0x28e42e69,// 278 PAY 275 

    0x4fce94e0,// 279 PAY 276 

    0x40dcdfa0,// 280 PAY 277 

    0xd2ea775b,// 281 PAY 278 

    0x701f92ca,// 282 PAY 279 

    0xa4e84587,// 283 PAY 280 

    0x86877ca4,// 284 PAY 281 

    0xd36a2c96,// 285 PAY 282 

    0x6c6ec40b,// 286 PAY 283 

    0x66789502,// 287 PAY 284 

    0x6812dac8,// 288 PAY 285 

    0xba5d5c95,// 289 PAY 286 

    0xb48993d5,// 290 PAY 287 

    0x407aef41,// 291 PAY 288 

    0x0f120ef4,// 292 PAY 289 

    0x0cb68402,// 293 PAY 290 

    0x1fbe2750,// 294 PAY 291 

    0x46aa4770,// 295 PAY 292 

    0x8193b121,// 296 PAY 293 

    0x8d466eeb,// 297 PAY 294 

    0x429e1007,// 298 PAY 295 

    0xf54f070c,// 299 PAY 296 

    0x84063ab3,// 300 PAY 297 

    0x27b89feb,// 301 PAY 298 

    0x80e09e02,// 302 PAY 299 

    0xb7bae2c8,// 303 PAY 300 

    0x9538fe03,// 304 PAY 301 

    0x403985b3,// 305 PAY 302 

    0x9f18c9d6,// 306 PAY 303 

    0xbe776b92,// 307 PAY 304 

    0xf385b36f,// 308 PAY 305 

    0xc9d463e6,// 309 PAY 306 

    0xe51a0d6e,// 310 PAY 307 

    0xc53185b1,// 311 PAY 308 

    0xd9293a85,// 312 PAY 309 

    0x172c4934,// 313 PAY 310 

    0xa8c52631,// 314 PAY 311 

    0x795f2d44,// 315 PAY 312 

    0x9c70e353,// 316 PAY 313 

    0xe62ee3cc,// 317 PAY 314 

    0x215c084d,// 318 PAY 315 

    0xef5a529c,// 319 PAY 316 

    0x26341f69,// 320 PAY 317 

    0xa7064ac5,// 321 PAY 318 

    0x5add1c51,// 322 PAY 319 

    0x62f901b5,// 323 PAY 320 

    0xc3cf6f43,// 324 PAY 321 

    0x1c5dc191,// 325 PAY 322 

    0x00c190d7,// 326 PAY 323 

    0xbe14373e,// 327 PAY 324 

    0xf6bcdbae,// 328 PAY 325 

    0x8d61b5be,// 329 PAY 326 

    0xb0f366bf,// 330 PAY 327 

    0x9b13d069,// 331 PAY 328 

    0xffdd24f7,// 332 PAY 329 

    0xf4932785,// 333 PAY 330 

    0x19a10b7d,// 334 PAY 331 

    0xe29e3008,// 335 PAY 332 

    0xeaf02947,// 336 PAY 333 

    0xb2513659,// 337 PAY 334 

    0xade212f3,// 338 PAY 335 

    0x2cf4c418,// 339 PAY 336 

    0x8045dd9c,// 340 PAY 337 

    0x775ab106,// 341 PAY 338 

    0xea68f2be,// 342 PAY 339 

    0x1c3e7350,// 343 PAY 340 

    0xe172351b,// 344 PAY 341 

    0xd32b4452,// 345 PAY 342 

    0x39143897,// 346 PAY 343 

    0x6fe308c2,// 347 PAY 344 

    0x966f209e,// 348 PAY 345 

    0x81b877d8,// 349 PAY 346 

    0x81662162,// 350 PAY 347 

    0x39410bcb,// 351 PAY 348 

    0x92a96979,// 352 PAY 349 

    0xf10f0ac5,// 353 PAY 350 

    0x1b5ea4fc,// 354 PAY 351 

    0xabb8baf5,// 355 PAY 352 

    0xff6fa045,// 356 PAY 353 

    0x89ebb2df,// 357 PAY 354 

    0x75fb10e9,// 358 PAY 355 

    0xa13d87b1,// 359 PAY 356 

    0x74ebadcf,// 360 PAY 357 

    0x5d544525,// 361 PAY 358 

    0xfec0cfc6,// 362 PAY 359 

    0x22d8eaf1,// 363 PAY 360 

    0x35c042e2,// 364 PAY 361 

    0xd2bd4ad8,// 365 PAY 362 

    0x973053f9,// 366 PAY 363 

    0x0e8fa49d,// 367 PAY 364 

    0xaceddabe,// 368 PAY 365 

    0x62a880a8,// 369 PAY 366 

    0x5c9f276f,// 370 PAY 367 

    0x5b84878e,// 371 PAY 368 

    0x20fcf550,// 372 PAY 369 

    0xe6a4f525,// 373 PAY 370 

    0x7d67ee3c,// 374 PAY 371 

    0xb45c4555,// 375 PAY 372 

    0x6dfeeaab,// 376 PAY 373 

    0x626fe71d,// 377 PAY 374 

    0x695264cd,// 378 PAY 375 

    0x800f6f07,// 379 PAY 376 

    0x3dd926be,// 380 PAY 377 

    0xa21c1d4f,// 381 PAY 378 

    0x85f55cc9,// 382 PAY 379 

    0xc378361e,// 383 PAY 380 

    0x63037b06,// 384 PAY 381 

    0xe1b668b7,// 385 PAY 382 

    0xa622e8dd,// 386 PAY 383 

    0x98740d48,// 387 PAY 384 

    0x613f9e66,// 388 PAY 385 

    0xf60985b9,// 389 PAY 386 

    0x7fbb2480,// 390 PAY 387 

    0x578cf640,// 391 PAY 388 

    0x0d342c90,// 392 PAY 389 

    0x7c465ae5,// 393 PAY 390 

    0x90a056f9,// 394 PAY 391 

    0xd37c44ea,// 395 PAY 392 

    0xb2289614,// 396 PAY 393 

    0x6c02d2d9,// 397 PAY 394 

    0xbb7fe4b7,// 398 PAY 395 

    0xff4830a1,// 399 PAY 396 

    0xfa4368be,// 400 PAY 397 

    0x9e23f22c,// 401 PAY 398 

    0x5f31cbc2,// 402 PAY 399 

    0x3661b0e7,// 403 PAY 400 

    0x1d8b47ba,// 404 PAY 401 

    0xdd971a14,// 405 PAY 402 

    0x352b1e3f,// 406 PAY 403 

    0x706f13cc,// 407 PAY 404 

    0x83194bb1,// 408 PAY 405 

    0x24c461d8,// 409 PAY 406 

    0x6d242c4b,// 410 PAY 407 

    0x4609606c,// 411 PAY 408 

    0xef7bb755,// 412 PAY 409 

    0x5e5549d4,// 413 PAY 410 

    0x960fbc96,// 414 PAY 411 

    0xe1e80e04,// 415 PAY 412 

    0xa4afc3ad,// 416 PAY 413 

    0x571098ba,// 417 PAY 414 

    0x79f22871,// 418 PAY 415 

    0xd6ab5f61,// 419 PAY 416 

    0xe9589712,// 420 PAY 417 

    0x3220bc28,// 421 PAY 418 

    0x9ad29a7f,// 422 PAY 419 

    0x319cc133,// 423 PAY 420 

    0x4e87d9ac,// 424 PAY 421 

    0x9db1c92f,// 425 PAY 422 

    0xf2fda83c,// 426 PAY 423 

    0x68859ac3,// 427 PAY 424 

    0xbfff8e59,// 428 PAY 425 

    0x430ebf64,// 429 PAY 426 

    0x08c06d72,// 430 PAY 427 

    0xf92c0258,// 431 PAY 428 

    0xc8773c86,// 432 PAY 429 

    0xd01b08a3,// 433 PAY 430 

    0x7ea7139a,// 434 PAY 431 

    0x9a8e2cc8,// 435 PAY 432 

    0xd53fda5a,// 436 PAY 433 

    0xec2ccfa7,// 437 PAY 434 

    0x87a16c64,// 438 PAY 435 

    0xe43b2984,// 439 PAY 436 

    0x7ee0d705,// 440 PAY 437 

    0x96a30568,// 441 PAY 438 

    0x6c4b2c00,// 442 PAY 439 

    0x1687d012,// 443 PAY 440 

    0x7aedffc9,// 444 PAY 441 

    0x1b7ddcbe,// 445 PAY 442 

    0xbf86778b,// 446 PAY 443 

    0xf47477e2,// 447 PAY 444 

    0x031457a0,// 448 PAY 445 

    0x682d2c2a,// 449 PAY 446 

    0x633b0d72,// 450 PAY 447 

    0x4f8a9541,// 451 PAY 448 

    0x38f993f3,// 452 PAY 449 

    0x919e1766,// 453 PAY 450 

    0x490850ad,// 454 PAY 451 

    0xe8fcbeb5,// 455 PAY 452 

    0x85016bb2,// 456 PAY 453 

    0xdd6b95f0,// 457 PAY 454 

    0xbf9844a3,// 458 PAY 455 

    0xfb4e8474,// 459 PAY 456 

    0x3fc333ac,// 460 PAY 457 

    0x4318a412,// 461 PAY 458 

    0xd80e1eeb,// 462 PAY 459 

    0x149f23d0,// 463 PAY 460 

    0x8d65f999,// 464 PAY 461 

    0xa8da2219,// 465 PAY 462 

    0xb6f95bdf,// 466 PAY 463 

    0x5f04b142,// 467 PAY 464 

    0xfc9e2c2a,// 468 PAY 465 

    0xb981bb24,// 469 PAY 466 

    0x94049ce7,// 470 PAY 467 

    0xe52ee97d,// 471 PAY 468 

    0x0d520d28,// 472 PAY 469 

    0x98b6ecb8,// 473 PAY 470 

    0xf9cd0b66,// 474 PAY 471 

    0x3c3d9691,// 475 PAY 472 

    0x4e2dd99e,// 476 PAY 473 

    0xb8646130,// 477 PAY 474 

    0xf5b9a8ac,// 478 PAY 475 

    0xd5bae249,// 479 PAY 476 

    0x4725ebdd,// 480 PAY 477 

    0x798821e7,// 481 PAY 478 

    0xc24f9d33,// 482 PAY 479 

    0xc5b57af3,// 483 PAY 480 

    0x748733fd,// 484 PAY 481 

    0xf9b47bd7,// 485 PAY 482 

    0x1a48a175,// 486 PAY 483 

    0xe1f92f33,// 487 PAY 484 

    0x61b09380,// 488 PAY 485 

    0x5458cef1,// 489 PAY 486 

    0xaefa3fe6,// 490 PAY 487 

    0xc7a5d1bc,// 491 PAY 488 

    0x3b43b4f5,// 492 PAY 489 

    0xf17bac12,// 493 PAY 490 

    0x12154555,// 494 PAY 491 

    0x25d260e8,// 495 PAY 492 

    0x663985eb,// 496 PAY 493 

    0x36fd0d18,// 497 PAY 494 

    0xe706e5a2,// 498 PAY 495 

    0x75dfa17c,// 499 PAY 496 

    0xb3e40000,// 500 PAY 497 

/// STA is 1 words. 

/// STA num_pkts       : 173 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x3f 

    0x02cd3fad // 501 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt46_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 450 words. 

/// BDA size     is 1794 (0x702) 

/// BDA id       is 0x3c4b 

    0x07023c4b,// 3 BDA   1 

/// PAY Generic Data size   : 1794 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa5061868,// 4 PAY   1 

    0x74b35550,// 5 PAY   2 

    0xb7d0e6b4,// 6 PAY   3 

    0xeaabd0c3,// 7 PAY   4 

    0x23d3845f,// 8 PAY   5 

    0xc83bced4,// 9 PAY   6 

    0x601b120a,// 10 PAY   7 

    0xeb64ceb8,// 11 PAY   8 

    0x8eb5b7a1,// 12 PAY   9 

    0x7839727d,// 13 PAY  10 

    0x0d5e3113,// 14 PAY  11 

    0xaec0d5c0,// 15 PAY  12 

    0xc45f3742,// 16 PAY  13 

    0xfc91c190,// 17 PAY  14 

    0x89f60e9f,// 18 PAY  15 

    0xc4c98e15,// 19 PAY  16 

    0xad4bf68e,// 20 PAY  17 

    0x38783363,// 21 PAY  18 

    0x4524e377,// 22 PAY  19 

    0xb63bc8c9,// 23 PAY  20 

    0x68941008,// 24 PAY  21 

    0x19ae9961,// 25 PAY  22 

    0xb3f7055e,// 26 PAY  23 

    0xad1deea8,// 27 PAY  24 

    0x1d655b5a,// 28 PAY  25 

    0xb074383b,// 29 PAY  26 

    0xca28ecef,// 30 PAY  27 

    0xe6275178,// 31 PAY  28 

    0x6ac5f135,// 32 PAY  29 

    0xa4134cc6,// 33 PAY  30 

    0xd6885a41,// 34 PAY  31 

    0x7b00eb71,// 35 PAY  32 

    0x1d8e3bda,// 36 PAY  33 

    0x803046d8,// 37 PAY  34 

    0xd70f1489,// 38 PAY  35 

    0x44503eed,// 39 PAY  36 

    0xeb06d875,// 40 PAY  37 

    0x2fb065f1,// 41 PAY  38 

    0x604427e0,// 42 PAY  39 

    0x7e9773f6,// 43 PAY  40 

    0xf4a6a342,// 44 PAY  41 

    0xda0b66e3,// 45 PAY  42 

    0xeecccd1d,// 46 PAY  43 

    0x65b8fee5,// 47 PAY  44 

    0x31be6489,// 48 PAY  45 

    0x4adbdc4b,// 49 PAY  46 

    0xf587d0b4,// 50 PAY  47 

    0x7b96ce3f,// 51 PAY  48 

    0x15dc762a,// 52 PAY  49 

    0x91d86697,// 53 PAY  50 

    0x18c26edd,// 54 PAY  51 

    0x9e159779,// 55 PAY  52 

    0x4d43f4bc,// 56 PAY  53 

    0x2556144b,// 57 PAY  54 

    0x20fc5f6e,// 58 PAY  55 

    0x8bb1b02a,// 59 PAY  56 

    0x54ee6e8c,// 60 PAY  57 

    0x85602473,// 61 PAY  58 

    0x9dcfd4b8,// 62 PAY  59 

    0x83fe6354,// 63 PAY  60 

    0x14d02c28,// 64 PAY  61 

    0xb6c3b5cc,// 65 PAY  62 

    0xf4a1458a,// 66 PAY  63 

    0x807be1b7,// 67 PAY  64 

    0x1378a081,// 68 PAY  65 

    0x5332648c,// 69 PAY  66 

    0x3999b4ff,// 70 PAY  67 

    0xdd2cf784,// 71 PAY  68 

    0xc57073c0,// 72 PAY  69 

    0x1692339b,// 73 PAY  70 

    0x1e6a1587,// 74 PAY  71 

    0xe83ed3c5,// 75 PAY  72 

    0x10b5aa25,// 76 PAY  73 

    0x8cfa2be0,// 77 PAY  74 

    0xac7429e5,// 78 PAY  75 

    0x82231f21,// 79 PAY  76 

    0xa8f96ef7,// 80 PAY  77 

    0x10f9e024,// 81 PAY  78 

    0x51fcda6b,// 82 PAY  79 

    0xabaae43e,// 83 PAY  80 

    0x17c61bc7,// 84 PAY  81 

    0xe6e73b25,// 85 PAY  82 

    0x8050c53e,// 86 PAY  83 

    0xba078b44,// 87 PAY  84 

    0x2d29ac40,// 88 PAY  85 

    0xfd5aaac4,// 89 PAY  86 

    0xffd7d58e,// 90 PAY  87 

    0xeb42836e,// 91 PAY  88 

    0x6397f110,// 92 PAY  89 

    0xa7eead64,// 93 PAY  90 

    0x3b100fbb,// 94 PAY  91 

    0x0ca9de26,// 95 PAY  92 

    0xbf9408c2,// 96 PAY  93 

    0x30a81fac,// 97 PAY  94 

    0xcb0c7965,// 98 PAY  95 

    0x7c9da722,// 99 PAY  96 

    0x063d5b52,// 100 PAY  97 

    0x0e5883ad,// 101 PAY  98 

    0x68e98440,// 102 PAY  99 

    0xaddc26b7,// 103 PAY 100 

    0x7e5eeb56,// 104 PAY 101 

    0x53b83532,// 105 PAY 102 

    0x36d9d1f6,// 106 PAY 103 

    0x7c250fc0,// 107 PAY 104 

    0x3e31747f,// 108 PAY 105 

    0xe5682b24,// 109 PAY 106 

    0xaa60cf16,// 110 PAY 107 

    0x106015f3,// 111 PAY 108 

    0xd3aeb682,// 112 PAY 109 

    0x96357ca5,// 113 PAY 110 

    0x4a07cbb4,// 114 PAY 111 

    0x8a67b159,// 115 PAY 112 

    0x2e8bf18d,// 116 PAY 113 

    0xe5591bb8,// 117 PAY 114 

    0x9c45e9fa,// 118 PAY 115 

    0x2a37c104,// 119 PAY 116 

    0x6b441dc9,// 120 PAY 117 

    0x1d51ba4c,// 121 PAY 118 

    0x6a3a328b,// 122 PAY 119 

    0xf6432de2,// 123 PAY 120 

    0x9bdc699b,// 124 PAY 121 

    0x5623699d,// 125 PAY 122 

    0x7d96c53a,// 126 PAY 123 

    0x94451eec,// 127 PAY 124 

    0x0e535b87,// 128 PAY 125 

    0xd4b167d4,// 129 PAY 126 

    0xaae65e94,// 130 PAY 127 

    0x60596704,// 131 PAY 128 

    0xcaf13b77,// 132 PAY 129 

    0x8fef62e3,// 133 PAY 130 

    0x87564760,// 134 PAY 131 

    0x1cb81a0e,// 135 PAY 132 

    0x00844253,// 136 PAY 133 

    0x2174ef25,// 137 PAY 134 

    0xf68b4215,// 138 PAY 135 

    0xa5bb1dde,// 139 PAY 136 

    0x3806c200,// 140 PAY 137 

    0x6b702c4b,// 141 PAY 138 

    0x95939e77,// 142 PAY 139 

    0x306e7d60,// 143 PAY 140 

    0x48872a7b,// 144 PAY 141 

    0x3da8a20f,// 145 PAY 142 

    0xd27c68f3,// 146 PAY 143 

    0x9d5249f9,// 147 PAY 144 

    0x8061407b,// 148 PAY 145 

    0xe36b3277,// 149 PAY 146 

    0x648582f0,// 150 PAY 147 

    0x93f0a8b0,// 151 PAY 148 

    0x052b906e,// 152 PAY 149 

    0xab97a457,// 153 PAY 150 

    0x213c9f2e,// 154 PAY 151 

    0x1e806aec,// 155 PAY 152 

    0xc717ea90,// 156 PAY 153 

    0x29804653,// 157 PAY 154 

    0x2ead0f0f,// 158 PAY 155 

    0xe6785d47,// 159 PAY 156 

    0x27adf3f0,// 160 PAY 157 

    0x19705d97,// 161 PAY 158 

    0x1497e326,// 162 PAY 159 

    0xcf341eba,// 163 PAY 160 

    0xc109c516,// 164 PAY 161 

    0x1660bbed,// 165 PAY 162 

    0x38a1c0b1,// 166 PAY 163 

    0xc055aa64,// 167 PAY 164 

    0x2697e69b,// 168 PAY 165 

    0x554def24,// 169 PAY 166 

    0xb87d70a5,// 170 PAY 167 

    0x342618a4,// 171 PAY 168 

    0xddd5ddb0,// 172 PAY 169 

    0x3500ebd5,// 173 PAY 170 

    0x6d8ace5a,// 174 PAY 171 

    0x6f5f31f8,// 175 PAY 172 

    0x0398dcf9,// 176 PAY 173 

    0xbedcdb99,// 177 PAY 174 

    0x13ea38b6,// 178 PAY 175 

    0x43aa99e1,// 179 PAY 176 

    0x42ff5a5e,// 180 PAY 177 

    0x1327b1c6,// 181 PAY 178 

    0x3d879319,// 182 PAY 179 

    0xd5b5ece3,// 183 PAY 180 

    0x0ecdd233,// 184 PAY 181 

    0xbff272d8,// 185 PAY 182 

    0xadc24c9c,// 186 PAY 183 

    0xc12443bc,// 187 PAY 184 

    0x6cd67507,// 188 PAY 185 

    0x367a1333,// 189 PAY 186 

    0x019e290e,// 190 PAY 187 

    0x77280bd5,// 191 PAY 188 

    0xaae9496f,// 192 PAY 189 

    0x3f4dae7e,// 193 PAY 190 

    0x3031002a,// 194 PAY 191 

    0x2100809e,// 195 PAY 192 

    0x82dac065,// 196 PAY 193 

    0x43a50772,// 197 PAY 194 

    0x6e6ebef9,// 198 PAY 195 

    0x3674373e,// 199 PAY 196 

    0x84088305,// 200 PAY 197 

    0x9e5d36e5,// 201 PAY 198 

    0x4f0633e8,// 202 PAY 199 

    0xebc92296,// 203 PAY 200 

    0x87bbdd68,// 204 PAY 201 

    0x906bd05a,// 205 PAY 202 

    0x008c7664,// 206 PAY 203 

    0x2f0c3a63,// 207 PAY 204 

    0xe2837cbf,// 208 PAY 205 

    0x4cbc9064,// 209 PAY 206 

    0xa0b67ceb,// 210 PAY 207 

    0xb7c84460,// 211 PAY 208 

    0x4c9718dd,// 212 PAY 209 

    0x9747bbb5,// 213 PAY 210 

    0x5591a877,// 214 PAY 211 

    0x095bd99e,// 215 PAY 212 

    0xfd4bb723,// 216 PAY 213 

    0xde796bbe,// 217 PAY 214 

    0x2c38a279,// 218 PAY 215 

    0x5b6e5426,// 219 PAY 216 

    0x84563bbe,// 220 PAY 217 

    0xeec4576d,// 221 PAY 218 

    0xc187bec1,// 222 PAY 219 

    0xb33bfc60,// 223 PAY 220 

    0xdeb510db,// 224 PAY 221 

    0x9ceab051,// 225 PAY 222 

    0xffb4f92a,// 226 PAY 223 

    0xd2ba0ba3,// 227 PAY 224 

    0x22bcc09c,// 228 PAY 225 

    0x8efa26df,// 229 PAY 226 

    0xdb4f511c,// 230 PAY 227 

    0xa6460268,// 231 PAY 228 

    0x2a50df38,// 232 PAY 229 

    0x4225e023,// 233 PAY 230 

    0xad83c074,// 234 PAY 231 

    0xe05ae44c,// 235 PAY 232 

    0xe629ec93,// 236 PAY 233 

    0xc8c63652,// 237 PAY 234 

    0x071dc716,// 238 PAY 235 

    0x787df23d,// 239 PAY 236 

    0xc50c689e,// 240 PAY 237 

    0x6019d72f,// 241 PAY 238 

    0x5cb2a50a,// 242 PAY 239 

    0x6d26ee06,// 243 PAY 240 

    0x1c663c8c,// 244 PAY 241 

    0xa92b4d3d,// 245 PAY 242 

    0xd43e0029,// 246 PAY 243 

    0x86f7a6b1,// 247 PAY 244 

    0xaedee5f6,// 248 PAY 245 

    0xd7c2ea4f,// 249 PAY 246 

    0x5462cf72,// 250 PAY 247 

    0x396b37f9,// 251 PAY 248 

    0x360141b2,// 252 PAY 249 

    0x403fe0c4,// 253 PAY 250 

    0x6ca7152d,// 254 PAY 251 

    0xca13923f,// 255 PAY 252 

    0x5607d078,// 256 PAY 253 

    0x8c64b7c1,// 257 PAY 254 

    0xabd5260a,// 258 PAY 255 

    0x0120456e,// 259 PAY 256 

    0xa3bc4a75,// 260 PAY 257 

    0xe76c0160,// 261 PAY 258 

    0xd1efe176,// 262 PAY 259 

    0x0fd6438f,// 263 PAY 260 

    0x2e777c4f,// 264 PAY 261 

    0x0b9e07f9,// 265 PAY 262 

    0xb000a9e9,// 266 PAY 263 

    0x00d9ff0a,// 267 PAY 264 

    0xdaa0ee24,// 268 PAY 265 

    0x62bd0167,// 269 PAY 266 

    0x0ba3d808,// 270 PAY 267 

    0xddf08cc7,// 271 PAY 268 

    0x47b1d62a,// 272 PAY 269 

    0x5ad965f4,// 273 PAY 270 

    0x8e108cda,// 274 PAY 271 

    0x0f698394,// 275 PAY 272 

    0x438d167e,// 276 PAY 273 

    0x87489e14,// 277 PAY 274 

    0x3d384432,// 278 PAY 275 

    0x0874c498,// 279 PAY 276 

    0x91018040,// 280 PAY 277 

    0x7274fa27,// 281 PAY 278 

    0x5d211d34,// 282 PAY 279 

    0xadb45e3c,// 283 PAY 280 

    0x1f5a09a7,// 284 PAY 281 

    0x8aa28b2b,// 285 PAY 282 

    0x94e6782c,// 286 PAY 283 

    0xf788e19d,// 287 PAY 284 

    0xc1ffeed6,// 288 PAY 285 

    0xae231d9e,// 289 PAY 286 

    0xe1c0b43a,// 290 PAY 287 

    0xfe3f113c,// 291 PAY 288 

    0x3716afc9,// 292 PAY 289 

    0x47a5710d,// 293 PAY 290 

    0x88386462,// 294 PAY 291 

    0x7509182b,// 295 PAY 292 

    0xc904bcf7,// 296 PAY 293 

    0x789a1fca,// 297 PAY 294 

    0xa353c7e9,// 298 PAY 295 

    0x9fa14682,// 299 PAY 296 

    0xae95bddc,// 300 PAY 297 

    0x1d8359be,// 301 PAY 298 

    0x55f8bf66,// 302 PAY 299 

    0x0c888652,// 303 PAY 300 

    0x171b5278,// 304 PAY 301 

    0x64862a95,// 305 PAY 302 

    0x60faa143,// 306 PAY 303 

    0x4cdc05bf,// 307 PAY 304 

    0xae01b321,// 308 PAY 305 

    0x49068e69,// 309 PAY 306 

    0x3628e81a,// 310 PAY 307 

    0x03a2c926,// 311 PAY 308 

    0x64b0676c,// 312 PAY 309 

    0x52d28d49,// 313 PAY 310 

    0xe11566d3,// 314 PAY 311 

    0x4b27850b,// 315 PAY 312 

    0xfc932245,// 316 PAY 313 

    0x6a6fbf08,// 317 PAY 314 

    0x056ccb5c,// 318 PAY 315 

    0xe283b558,// 319 PAY 316 

    0x911d544d,// 320 PAY 317 

    0x3bc93798,// 321 PAY 318 

    0xaebd716e,// 322 PAY 319 

    0xa3e06eb7,// 323 PAY 320 

    0x1ce636e2,// 324 PAY 321 

    0xfab29c3d,// 325 PAY 322 

    0xf7c26b0f,// 326 PAY 323 

    0x0c73db84,// 327 PAY 324 

    0x82c192f0,// 328 PAY 325 

    0xe42b612b,// 329 PAY 326 

    0x41405391,// 330 PAY 327 

    0x95124e9c,// 331 PAY 328 

    0x7af29a2f,// 332 PAY 329 

    0x0cd02526,// 333 PAY 330 

    0x884f9d81,// 334 PAY 331 

    0xa66247c2,// 335 PAY 332 

    0x7c753fc5,// 336 PAY 333 

    0xd88aa250,// 337 PAY 334 

    0xa548a787,// 338 PAY 335 

    0xed201165,// 339 PAY 336 

    0x426f4f5b,// 340 PAY 337 

    0xa9a4b3ee,// 341 PAY 338 

    0x680b2e0e,// 342 PAY 339 

    0x75ac5d25,// 343 PAY 340 

    0xaa18cac4,// 344 PAY 341 

    0x1a9bff17,// 345 PAY 342 

    0xbd83192a,// 346 PAY 343 

    0xb16c8a8a,// 347 PAY 344 

    0x2ab4e640,// 348 PAY 345 

    0x3783307e,// 349 PAY 346 

    0xd9dab764,// 350 PAY 347 

    0xbdff1ec8,// 351 PAY 348 

    0x8503c738,// 352 PAY 349 

    0xc4754844,// 353 PAY 350 

    0x556e267e,// 354 PAY 351 

    0xc033c71b,// 355 PAY 352 

    0xde430792,// 356 PAY 353 

    0xe9be8d79,// 357 PAY 354 

    0x096ca117,// 358 PAY 355 

    0x713751eb,// 359 PAY 356 

    0x80462e79,// 360 PAY 357 

    0x96018a36,// 361 PAY 358 

    0x44601a0c,// 362 PAY 359 

    0x46a7865d,// 363 PAY 360 

    0x48aeca0c,// 364 PAY 361 

    0x963f33dd,// 365 PAY 362 

    0x778a5681,// 366 PAY 363 

    0x4bb0e5d7,// 367 PAY 364 

    0xd9fb9e76,// 368 PAY 365 

    0xa00d296d,// 369 PAY 366 

    0xbe70d666,// 370 PAY 367 

    0x750b339d,// 371 PAY 368 

    0x7dda0c6d,// 372 PAY 369 

    0x03882342,// 373 PAY 370 

    0xbda584fd,// 374 PAY 371 

    0xf05e5af5,// 375 PAY 372 

    0x8c5e5276,// 376 PAY 373 

    0x23c6fa3b,// 377 PAY 374 

    0xa4b31cb3,// 378 PAY 375 

    0xee1a9427,// 379 PAY 376 

    0xd11c0369,// 380 PAY 377 

    0xf18fc11b,// 381 PAY 378 

    0x13f11146,// 382 PAY 379 

    0x33837a8c,// 383 PAY 380 

    0x1ec00184,// 384 PAY 381 

    0x5b484143,// 385 PAY 382 

    0x2a8cca8e,// 386 PAY 383 

    0xe35a2b40,// 387 PAY 384 

    0x3e3dc04c,// 388 PAY 385 

    0xbad2ddf2,// 389 PAY 386 

    0xf23ac8c6,// 390 PAY 387 

    0xb509584c,// 391 PAY 388 

    0xf5a962d5,// 392 PAY 389 

    0xff31902b,// 393 PAY 390 

    0x7107725c,// 394 PAY 391 

    0x00a82cae,// 395 PAY 392 

    0xbb34ba88,// 396 PAY 393 

    0x5d71c518,// 397 PAY 394 

    0xab1e5bfd,// 398 PAY 395 

    0xa206ac37,// 399 PAY 396 

    0xa2f39e6b,// 400 PAY 397 

    0xfdaceede,// 401 PAY 398 

    0x11e19ffb,// 402 PAY 399 

    0xe0967e34,// 403 PAY 400 

    0x1510432d,// 404 PAY 401 

    0x5683b141,// 405 PAY 402 

    0x2b20eae4,// 406 PAY 403 

    0x70778d75,// 407 PAY 404 

    0x8ae6621f,// 408 PAY 405 

    0xe2720130,// 409 PAY 406 

    0xa2c69605,// 410 PAY 407 

    0x19f00d2e,// 411 PAY 408 

    0xc61d541f,// 412 PAY 409 

    0x3c45481b,// 413 PAY 410 

    0xea883ce1,// 414 PAY 411 

    0xd5e86eb1,// 415 PAY 412 

    0xdac20fc8,// 416 PAY 413 

    0xacdd0173,// 417 PAY 414 

    0xdfb4beb2,// 418 PAY 415 

    0x626e137d,// 419 PAY 416 

    0x3321aec0,// 420 PAY 417 

    0x3736c36f,// 421 PAY 418 

    0x94d938d6,// 422 PAY 419 

    0xfa8325ed,// 423 PAY 420 

    0xc17d0c71,// 424 PAY 421 

    0x472c2c5b,// 425 PAY 422 

    0x5995d3a4,// 426 PAY 423 

    0x66a94f6f,// 427 PAY 424 

    0x4f35f407,// 428 PAY 425 

    0xe4ff7488,// 429 PAY 426 

    0x5182f0f1,// 430 PAY 427 

    0x551f7262,// 431 PAY 428 

    0x3b4ca783,// 432 PAY 429 

    0x11d22511,// 433 PAY 430 

    0xdb2ae5ec,// 434 PAY 431 

    0x3db110c0,// 435 PAY 432 

    0x9d9f1081,// 436 PAY 433 

    0xf32d5545,// 437 PAY 434 

    0xeec64133,// 438 PAY 435 

    0x56ddc34d,// 439 PAY 436 

    0xf7d23754,// 440 PAY 437 

    0x9441f2ca,// 441 PAY 438 

    0x6dd4cecf,// 442 PAY 439 

    0x9a2eb9ea,// 443 PAY 440 

    0x6a9f7baa,// 444 PAY 441 

    0x87d01f45,// 445 PAY 442 

    0x00caac82,// 446 PAY 443 

    0x5ab6e3ce,// 447 PAY 444 

    0x1d670d2d,// 448 PAY 445 

    0xe97becc0,// 449 PAY 446 

    0x3175e8f2,// 450 PAY 447 

    0x381677df,// 451 PAY 448 

    0x86e00000,// 452 PAY 449 

/// STA is 1 words. 

/// STA num_pkts       : 100 

/// STA pkt_idx        : 152 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6f 

    0x02616f64 // 453 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt47_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 91 words. 

/// BDA size     is 360 (0x168) 

/// BDA id       is 0x4ac8 

    0x01684ac8,// 3 BDA   1 

/// PAY Generic Data size   : 360 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xa07199cf,// 4 PAY   1 

    0x901a4d64,// 5 PAY   2 

    0xc9ea1734,// 6 PAY   3 

    0x63a0832c,// 7 PAY   4 

    0xc2187975,// 8 PAY   5 

    0xe392c07b,// 9 PAY   6 

    0xd3f0418d,// 10 PAY   7 

    0xae496652,// 11 PAY   8 

    0x5cfa05dd,// 12 PAY   9 

    0xe32ba245,// 13 PAY  10 

    0xddfb8243,// 14 PAY  11 

    0x887a02b7,// 15 PAY  12 

    0xc1c72406,// 16 PAY  13 

    0xc00c5ec6,// 17 PAY  14 

    0x4b1476cd,// 18 PAY  15 

    0x67882ce8,// 19 PAY  16 

    0x6154599d,// 20 PAY  17 

    0x02dfa674,// 21 PAY  18 

    0x312cd35b,// 22 PAY  19 

    0xf9beca88,// 23 PAY  20 

    0xf3eb1b5f,// 24 PAY  21 

    0x3715e1d8,// 25 PAY  22 

    0xab66c5ed,// 26 PAY  23 

    0x2a6673e9,// 27 PAY  24 

    0x675c7345,// 28 PAY  25 

    0x56907fa7,// 29 PAY  26 

    0xa3e3c5a9,// 30 PAY  27 

    0x972339c2,// 31 PAY  28 

    0x7018cf58,// 32 PAY  29 

    0x33f31020,// 33 PAY  30 

    0xb38052ab,// 34 PAY  31 

    0x6e0a0369,// 35 PAY  32 

    0x3e52bb5f,// 36 PAY  33 

    0xc633dffc,// 37 PAY  34 

    0xb9866174,// 38 PAY  35 

    0x01c29afb,// 39 PAY  36 

    0xd51c1727,// 40 PAY  37 

    0xafdda941,// 41 PAY  38 

    0x4f1e14c6,// 42 PAY  39 

    0x04ab4c20,// 43 PAY  40 

    0x27d2800d,// 44 PAY  41 

    0x3a5ed2f7,// 45 PAY  42 

    0xb3448724,// 46 PAY  43 

    0x5942f24b,// 47 PAY  44 

    0xd1eb867a,// 48 PAY  45 

    0xc9d62c12,// 49 PAY  46 

    0x6907170e,// 50 PAY  47 

    0x36cc46da,// 51 PAY  48 

    0xeb51d04c,// 52 PAY  49 

    0x7c4191b5,// 53 PAY  50 

    0xbd856415,// 54 PAY  51 

    0x235b5b60,// 55 PAY  52 

    0x9cea6ff0,// 56 PAY  53 

    0xd5142f3c,// 57 PAY  54 

    0x4b79fb41,// 58 PAY  55 

    0x54c94f8d,// 59 PAY  56 

    0x05795462,// 60 PAY  57 

    0x0d3c162c,// 61 PAY  58 

    0xd13d3386,// 62 PAY  59 

    0xc5452200,// 63 PAY  60 

    0xd1df9d68,// 64 PAY  61 

    0xfd9ca032,// 65 PAY  62 

    0xc8461fdd,// 66 PAY  63 

    0xe4e27c42,// 67 PAY  64 

    0x303e6e4e,// 68 PAY  65 

    0x16ab7717,// 69 PAY  66 

    0xe87c65ca,// 70 PAY  67 

    0x51282b33,// 71 PAY  68 

    0x61ddd2b6,// 72 PAY  69 

    0xd9d6a15a,// 73 PAY  70 

    0xf49c0ce2,// 74 PAY  71 

    0x085d19b7,// 75 PAY  72 

    0x44a765bb,// 76 PAY  73 

    0x24320917,// 77 PAY  74 

    0xcd970788,// 78 PAY  75 

    0x1b10903d,// 79 PAY  76 

    0x3f5fbf6d,// 80 PAY  77 

    0xfe0fcde1,// 81 PAY  78 

    0x86b723d2,// 82 PAY  79 

    0x629e1d2c,// 83 PAY  80 

    0x85799a6e,// 84 PAY  81 

    0x4623ac88,// 85 PAY  82 

    0x1ad98761,// 86 PAY  83 

    0xd56fdff1,// 87 PAY  84 

    0x655abb46,// 88 PAY  85 

    0x9f91c76c,// 89 PAY  86 

    0x97f9f765,// 90 PAY  87 

    0xa06773a6,// 91 PAY  88 

    0x5bea295d,// 92 PAY  89 

    0x0db64571,// 93 PAY  90 

/// HASH is  8 bytes 

    0x85799a6e,// 94 HSH   1 

    0x4623ac88,// 95 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 35 

/// STA pkt_idx        : 128 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xac 

    0x0200ac23 // 96 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt48_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x07 

/// ECH pdu_tag        : 0x00 

    0x00070000,// 2 ECH   1 

/// BDA is 37 words. 

/// BDA size     is 144 (0x90) 

/// BDA id       is 0x267c 

    0x0090267c,// 3 BDA   1 

/// PAY Generic Data size   : 144 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x01c134e0,// 4 PAY   1 

    0xf7265413,// 5 PAY   2 

    0xc8a36373,// 6 PAY   3 

    0xedf89652,// 7 PAY   4 

    0x512bb7a1,// 8 PAY   5 

    0x3f8cd0ba,// 9 PAY   6 

    0x224f083f,// 10 PAY   7 

    0x1b04dff4,// 11 PAY   8 

    0x04bf4a9e,// 12 PAY   9 

    0x939f5f43,// 13 PAY  10 

    0x32213c75,// 14 PAY  11 

    0x908b1fac,// 15 PAY  12 

    0x658a86a9,// 16 PAY  13 

    0xeb14debe,// 17 PAY  14 

    0xc7d19f65,// 18 PAY  15 

    0x44d91abd,// 19 PAY  16 

    0x1a67c44c,// 20 PAY  17 

    0xc2b5a4f4,// 21 PAY  18 

    0x3d7b5879,// 22 PAY  19 

    0x6bb17867,// 23 PAY  20 

    0x703e7c9d,// 24 PAY  21 

    0xaa41049e,// 25 PAY  22 

    0x1416f965,// 26 PAY  23 

    0x19bac94e,// 27 PAY  24 

    0x14a4e576,// 28 PAY  25 

    0xa26509b2,// 29 PAY  26 

    0xf528ceeb,// 30 PAY  27 

    0x74914a3b,// 31 PAY  28 

    0x7b65b6d6,// 32 PAY  29 

    0xd46c7e1d,// 33 PAY  30 

    0xc3df2553,// 34 PAY  31 

    0x9b454562,// 35 PAY  32 

    0xc01e909d,// 36 PAY  33 

    0xae227a76,// 37 PAY  34 

    0xf3f9cfa3,// 38 PAY  35 

    0xde3e40d7,// 39 PAY  36 

/// STA is 1 words. 

/// STA num_pkts       : 144 

/// STA pkt_idx        : 108 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x01b11c90 // 40 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt49_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 124 words. 

/// BDA size     is 490 (0x1ea) 

/// BDA id       is 0x5630 

    0x01ea5630,// 3 BDA   1 

/// PAY Generic Data size   : 490 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x25ff283d,// 4 PAY   1 

    0x53db243e,// 5 PAY   2 

    0x679f0626,// 6 PAY   3 

    0x7c4ccc3f,// 7 PAY   4 

    0xc8326ab1,// 8 PAY   5 

    0x9b209bea,// 9 PAY   6 

    0x7b7bec46,// 10 PAY   7 

    0x3d3227de,// 11 PAY   8 

    0x10ef2162,// 12 PAY   9 

    0x94d02d44,// 13 PAY  10 

    0xc85d4cc0,// 14 PAY  11 

    0xbe0e11e5,// 15 PAY  12 

    0xe0ccedc8,// 16 PAY  13 

    0x06a4d0fa,// 17 PAY  14 

    0x3872461a,// 18 PAY  15 

    0xc1ede095,// 19 PAY  16 

    0x2e7c275b,// 20 PAY  17 

    0x7abb90f2,// 21 PAY  18 

    0x594fd48f,// 22 PAY  19 

    0xb6d29c27,// 23 PAY  20 

    0x669e96c6,// 24 PAY  21 

    0x2c883f79,// 25 PAY  22 

    0xfca1d23c,// 26 PAY  23 

    0x3d817cbf,// 27 PAY  24 

    0xde1c845d,// 28 PAY  25 

    0x54becce8,// 29 PAY  26 

    0xd0db5fbf,// 30 PAY  27 

    0x8e5f63e8,// 31 PAY  28 

    0x06e49eb5,// 32 PAY  29 

    0x5e2cc574,// 33 PAY  30 

    0x1310581d,// 34 PAY  31 

    0x3e3849b4,// 35 PAY  32 

    0x023af647,// 36 PAY  33 

    0xc13c0966,// 37 PAY  34 

    0x5316037a,// 38 PAY  35 

    0x93e149ad,// 39 PAY  36 

    0x74bad2f0,// 40 PAY  37 

    0x89962fe0,// 41 PAY  38 

    0xad914cf7,// 42 PAY  39 

    0x3c073bf0,// 43 PAY  40 

    0xfa9780d6,// 44 PAY  41 

    0x760bf921,// 45 PAY  42 

    0xe9fac2a4,// 46 PAY  43 

    0xe26a0447,// 47 PAY  44 

    0x1acbd9d2,// 48 PAY  45 

    0xf0cd80ae,// 49 PAY  46 

    0xa640a73a,// 50 PAY  47 

    0x80e3c7c2,// 51 PAY  48 

    0xe3c75437,// 52 PAY  49 

    0xe010eb87,// 53 PAY  50 

    0xa192e6c8,// 54 PAY  51 

    0xf8c5641d,// 55 PAY  52 

    0x2f7ec1e6,// 56 PAY  53 

    0x19cd58dd,// 57 PAY  54 

    0x436c4eca,// 58 PAY  55 

    0xc045443a,// 59 PAY  56 

    0xb87e442e,// 60 PAY  57 

    0x698b7c0d,// 61 PAY  58 

    0x295be789,// 62 PAY  59 

    0xc0b30594,// 63 PAY  60 

    0xee20af2f,// 64 PAY  61 

    0xe2de8d91,// 65 PAY  62 

    0xdfea6f91,// 66 PAY  63 

    0xd3bed7c7,// 67 PAY  64 

    0xdf438663,// 68 PAY  65 

    0x52f167b4,// 69 PAY  66 

    0x41a40bee,// 70 PAY  67 

    0xd0fa1880,// 71 PAY  68 

    0x6012a883,// 72 PAY  69 

    0x6d72b18e,// 73 PAY  70 

    0xa763a1cc,// 74 PAY  71 

    0x8c44956b,// 75 PAY  72 

    0x7b7f61cf,// 76 PAY  73 

    0xa26e55d1,// 77 PAY  74 

    0x17a670e2,// 78 PAY  75 

    0x4eb88f52,// 79 PAY  76 

    0xe7c64b51,// 80 PAY  77 

    0x578a741c,// 81 PAY  78 

    0x2743e2ab,// 82 PAY  79 

    0x2c4a1e7c,// 83 PAY  80 

    0x7bbb9756,// 84 PAY  81 

    0x36e6157e,// 85 PAY  82 

    0xf952601b,// 86 PAY  83 

    0x8b3d3d21,// 87 PAY  84 

    0x52dbb36e,// 88 PAY  85 

    0x799a15e2,// 89 PAY  86 

    0xcfa1924d,// 90 PAY  87 

    0x24bc38f4,// 91 PAY  88 

    0x21e1bda5,// 92 PAY  89 

    0x9e898fb1,// 93 PAY  90 

    0x0122534b,// 94 PAY  91 

    0xe139b253,// 95 PAY  92 

    0x3c947f6f,// 96 PAY  93 

    0x44247bd9,// 97 PAY  94 

    0xf802046f,// 98 PAY  95 

    0xb13e7f4e,// 99 PAY  96 

    0x3a04b4ff,// 100 PAY  97 

    0x1aaec2ad,// 101 PAY  98 

    0x58019a65,// 102 PAY  99 

    0x0247da2f,// 103 PAY 100 

    0x72be2f09,// 104 PAY 101 

    0x4ae88846,// 105 PAY 102 

    0x79b1add6,// 106 PAY 103 

    0x02e3d4e7,// 107 PAY 104 

    0xf2fad9c2,// 108 PAY 105 

    0x54e694fa,// 109 PAY 106 

    0x0a9718f5,// 110 PAY 107 

    0xf1fa0882,// 111 PAY 108 

    0x05e728b4,// 112 PAY 109 

    0xa5c2fa62,// 113 PAY 110 

    0x77e23a78,// 114 PAY 111 

    0x015fd9c1,// 115 PAY 112 

    0xd3fec01d,// 116 PAY 113 

    0x6561ac96,// 117 PAY 114 

    0x8bbd3b71,// 118 PAY 115 

    0x24bc3ca6,// 119 PAY 116 

    0x3b1cde94,// 120 PAY 117 

    0xa21e6cc1,// 121 PAY 118 

    0xafaa9f83,// 122 PAY 119 

    0xef848705,// 123 PAY 120 

    0x18dad722,// 124 PAY 121 

    0xeabdcb82,// 125 PAY 122 

    0xea3e0000,// 126 PAY 123 

/// HASH is  16 bytes 

    0x9e898fb1,// 127 HSH   1 

    0x0122534b,// 128 HSH   2 

    0xe139b253,// 129 HSH   3 

    0x3c947f6f,// 130 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 101 

/// STA pkt_idx        : 196 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xce 

    0x0311ce65 // 131 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt50_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 164 words. 

/// BDA size     is 652 (0x28c) 

/// BDA id       is 0x3ee5 

    0x028c3ee5,// 3 BDA   1 

/// PAY Generic Data size   : 652 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x85200a74,// 4 PAY   1 

    0x5ccff99c,// 5 PAY   2 

    0x8dc2674f,// 6 PAY   3 

    0xf198d599,// 7 PAY   4 

    0x88b225ed,// 8 PAY   5 

    0xd6d46838,// 9 PAY   6 

    0xc61446bc,// 10 PAY   7 

    0xd3629310,// 11 PAY   8 

    0x7eb47e4b,// 12 PAY   9 

    0x4fb3799b,// 13 PAY  10 

    0x764fb0f6,// 14 PAY  11 

    0x2e5cff24,// 15 PAY  12 

    0xa02af2cb,// 16 PAY  13 

    0xdd45dc04,// 17 PAY  14 

    0x563388cb,// 18 PAY  15 

    0x53923819,// 19 PAY  16 

    0x66d8c574,// 20 PAY  17 

    0xbbfe8316,// 21 PAY  18 

    0xd412faa3,// 22 PAY  19 

    0x78006e13,// 23 PAY  20 

    0xd47642f1,// 24 PAY  21 

    0xfaba300d,// 25 PAY  22 

    0xd771d6ed,// 26 PAY  23 

    0x5a4aa526,// 27 PAY  24 

    0x42759087,// 28 PAY  25 

    0x72db9f8a,// 29 PAY  26 

    0x2c494c8f,// 30 PAY  27 

    0xfbe304fc,// 31 PAY  28 

    0x85317f98,// 32 PAY  29 

    0xfa2c4e67,// 33 PAY  30 

    0x21ca5177,// 34 PAY  31 

    0xaa726d43,// 35 PAY  32 

    0x639e6d65,// 36 PAY  33 

    0xd5b760f9,// 37 PAY  34 

    0xd37d5dff,// 38 PAY  35 

    0xbb158f3e,// 39 PAY  36 

    0xd0382aec,// 40 PAY  37 

    0xb1b8c849,// 41 PAY  38 

    0x1a079cd2,// 42 PAY  39 

    0x41cd4c3f,// 43 PAY  40 

    0xd1b9b46b,// 44 PAY  41 

    0x119dd9cc,// 45 PAY  42 

    0x987f7da7,// 46 PAY  43 

    0x0dcb1ba2,// 47 PAY  44 

    0xec9d09a0,// 48 PAY  45 

    0x13f9542a,// 49 PAY  46 

    0xbb8a2142,// 50 PAY  47 

    0x41fd4869,// 51 PAY  48 

    0x3cd80b50,// 52 PAY  49 

    0x4da2d21b,// 53 PAY  50 

    0x4ec0d0d5,// 54 PAY  51 

    0x0ae0ac18,// 55 PAY  52 

    0x3eba54c0,// 56 PAY  53 

    0x14fc3c6b,// 57 PAY  54 

    0x374a13fd,// 58 PAY  55 

    0x6b329a38,// 59 PAY  56 

    0x635c095a,// 60 PAY  57 

    0x7532017b,// 61 PAY  58 

    0x595d3b46,// 62 PAY  59 

    0x91ea619a,// 63 PAY  60 

    0x283b2d84,// 64 PAY  61 

    0x9fd4c7e0,// 65 PAY  62 

    0x1521d102,// 66 PAY  63 

    0x39d93160,// 67 PAY  64 

    0x419a0297,// 68 PAY  65 

    0xddecb87b,// 69 PAY  66 

    0xa18340a1,// 70 PAY  67 

    0x465c27ad,// 71 PAY  68 

    0x28df2ab2,// 72 PAY  69 

    0x7ecca408,// 73 PAY  70 

    0x956f428c,// 74 PAY  71 

    0xd883479e,// 75 PAY  72 

    0x71753589,// 76 PAY  73 

    0x9e3d3776,// 77 PAY  74 

    0x8a0e7cb0,// 78 PAY  75 

    0xe53b12cc,// 79 PAY  76 

    0x6ff5e1bb,// 80 PAY  77 

    0xd635bd88,// 81 PAY  78 

    0xde364f54,// 82 PAY  79 

    0xd585270d,// 83 PAY  80 

    0x4dc70fb2,// 84 PAY  81 

    0x95dd266e,// 85 PAY  82 

    0xf1abb4ee,// 86 PAY  83 

    0xd800caad,// 87 PAY  84 

    0xb61a45be,// 88 PAY  85 

    0x522599a7,// 89 PAY  86 

    0x3398196e,// 90 PAY  87 

    0xd03afd7f,// 91 PAY  88 

    0x23b6589a,// 92 PAY  89 

    0x9b376c43,// 93 PAY  90 

    0x0a784306,// 94 PAY  91 

    0x1ea97b73,// 95 PAY  92 

    0x9cb29699,// 96 PAY  93 

    0x2abb6500,// 97 PAY  94 

    0x2627ef20,// 98 PAY  95 

    0x39f97703,// 99 PAY  96 

    0xfbee7fcf,// 100 PAY  97 

    0xa321978d,// 101 PAY  98 

    0x82224399,// 102 PAY  99 

    0xd6776453,// 103 PAY 100 

    0x9cf96546,// 104 PAY 101 

    0x34cb3f38,// 105 PAY 102 

    0xce2595c8,// 106 PAY 103 

    0x77a55970,// 107 PAY 104 

    0x3ce9f722,// 108 PAY 105 

    0x668fbab2,// 109 PAY 106 

    0x3149e77a,// 110 PAY 107 

    0x46d4c4c1,// 111 PAY 108 

    0x34415e85,// 112 PAY 109 

    0x064805df,// 113 PAY 110 

    0x9b12b109,// 114 PAY 111 

    0x823c33de,// 115 PAY 112 

    0xf5864af8,// 116 PAY 113 

    0xf8e4a4fa,// 117 PAY 114 

    0xbcc1a968,// 118 PAY 115 

    0x949c8b8f,// 119 PAY 116 

    0x84438a6b,// 120 PAY 117 

    0x4933a95d,// 121 PAY 118 

    0x84f735ab,// 122 PAY 119 

    0xfb52e4ed,// 123 PAY 120 

    0x1f03ecb9,// 124 PAY 121 

    0xdad4ce1d,// 125 PAY 122 

    0xb557f6ff,// 126 PAY 123 

    0xf7d3dbf8,// 127 PAY 124 

    0xdc80c6c7,// 128 PAY 125 

    0x883fe9e5,// 129 PAY 126 

    0xea181420,// 130 PAY 127 

    0x5f280d6e,// 131 PAY 128 

    0x806c29f4,// 132 PAY 129 

    0xd6716c97,// 133 PAY 130 

    0xd8b9c904,// 134 PAY 131 

    0x3cfdb2c6,// 135 PAY 132 

    0x3235d882,// 136 PAY 133 

    0xf1fb4399,// 137 PAY 134 

    0x54377589,// 138 PAY 135 

    0xfb0a0c29,// 139 PAY 136 

    0x9c47a9a9,// 140 PAY 137 

    0x88015daa,// 141 PAY 138 

    0xa5653269,// 142 PAY 139 

    0x121fc04b,// 143 PAY 140 

    0x6afc289e,// 144 PAY 141 

    0x327390fd,// 145 PAY 142 

    0x715ecb16,// 146 PAY 143 

    0x247d789c,// 147 PAY 144 

    0xb76915d8,// 148 PAY 145 

    0x3b0939d9,// 149 PAY 146 

    0x57701d72,// 150 PAY 147 

    0x739f6cde,// 151 PAY 148 

    0xc49fef2d,// 152 PAY 149 

    0x6b358a94,// 153 PAY 150 

    0xf163f3c4,// 154 PAY 151 

    0xf1b71cfa,// 155 PAY 152 

    0x812f1c7c,// 156 PAY 153 

    0xaafbd9c6,// 157 PAY 154 

    0x0e22c53d,// 158 PAY 155 

    0x552635a1,// 159 PAY 156 

    0x2ebd98d6,// 160 PAY 157 

    0xa0dc7c26,// 161 PAY 158 

    0xad3af7a3,// 162 PAY 159 

    0x1d398ab0,// 163 PAY 160 

    0x7f0aa8d3,// 164 PAY 161 

    0x8cb2d444,// 165 PAY 162 

    0x56e61478,// 166 PAY 163 

/// STA is 1 words. 

/// STA num_pkts       : 196 

/// STA pkt_idx        : 51 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x52 

    0x00cc52c4 // 167 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt51_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 410 words. 

/// BDA size     is 1634 (0x662) 

/// BDA id       is 0x30ad 

    0x066230ad,// 3 BDA   1 

/// PAY Generic Data size   : 1634 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x9ff1b4e4,// 4 PAY   1 

    0x91a170a4,// 5 PAY   2 

    0x84d6a1bf,// 6 PAY   3 

    0xb8e9f1ef,// 7 PAY   4 

    0xe45ed7e9,// 8 PAY   5 

    0x6606e51c,// 9 PAY   6 

    0x2bcb8077,// 10 PAY   7 

    0x299da68b,// 11 PAY   8 

    0x955f2865,// 12 PAY   9 

    0x9bca2174,// 13 PAY  10 

    0x0af9fc62,// 14 PAY  11 

    0x4df3cbb9,// 15 PAY  12 

    0xf4e3cdbb,// 16 PAY  13 

    0xdbb2c801,// 17 PAY  14 

    0x66c7a905,// 18 PAY  15 

    0xfc38196e,// 19 PAY  16 

    0xe26c606c,// 20 PAY  17 

    0x0fe29136,// 21 PAY  18 

    0xd618ced3,// 22 PAY  19 

    0x0cb319be,// 23 PAY  20 

    0xabe9b485,// 24 PAY  21 

    0x9725638b,// 25 PAY  22 

    0x8176ba21,// 26 PAY  23 

    0xbb0df0ea,// 27 PAY  24 

    0x14152942,// 28 PAY  25 

    0xf7284a3e,// 29 PAY  26 

    0xbcf7fde0,// 30 PAY  27 

    0x0bcbe230,// 31 PAY  28 

    0x2aea4b46,// 32 PAY  29 

    0x68000bc1,// 33 PAY  30 

    0xbbdad3a0,// 34 PAY  31 

    0x1763a417,// 35 PAY  32 

    0xcb745f45,// 36 PAY  33 

    0x99fefeca,// 37 PAY  34 

    0x472bc00d,// 38 PAY  35 

    0xf09fd49a,// 39 PAY  36 

    0x20e16cba,// 40 PAY  37 

    0x5b95e138,// 41 PAY  38 

    0xbb873353,// 42 PAY  39 

    0x11f61307,// 43 PAY  40 

    0x23b45e18,// 44 PAY  41 

    0x14788db2,// 45 PAY  42 

    0xec8525b0,// 46 PAY  43 

    0x759549c2,// 47 PAY  44 

    0xf6a0d84e,// 48 PAY  45 

    0x083d3e26,// 49 PAY  46 

    0x91916075,// 50 PAY  47 

    0x0811d253,// 51 PAY  48 

    0xb9c83f74,// 52 PAY  49 

    0x2b1718e3,// 53 PAY  50 

    0x8bef8332,// 54 PAY  51 

    0x87fe1f68,// 55 PAY  52 

    0x1270bf99,// 56 PAY  53 

    0x4d5a1afc,// 57 PAY  54 

    0x258e2548,// 58 PAY  55 

    0xba298a63,// 59 PAY  56 

    0x374049a5,// 60 PAY  57 

    0x29e83c38,// 61 PAY  58 

    0x1b1d055f,// 62 PAY  59 

    0xe36c0d8f,// 63 PAY  60 

    0x8691fb0b,// 64 PAY  61 

    0x392e2a13,// 65 PAY  62 

    0x6b45a1ec,// 66 PAY  63 

    0x65e1a6f5,// 67 PAY  64 

    0x87bce457,// 68 PAY  65 

    0x215e5690,// 69 PAY  66 

    0xe577a67d,// 70 PAY  67 

    0x53e58b9e,// 71 PAY  68 

    0x44ea3841,// 72 PAY  69 

    0x63ab41d3,// 73 PAY  70 

    0x9507feb1,// 74 PAY  71 

    0x67b5451e,// 75 PAY  72 

    0xb7822f08,// 76 PAY  73 

    0x61328662,// 77 PAY  74 

    0xdbb3e298,// 78 PAY  75 

    0x95f31b5b,// 79 PAY  76 

    0xff0f6a32,// 80 PAY  77 

    0x37822ef6,// 81 PAY  78 

    0xad8234f9,// 82 PAY  79 

    0x390c0e2e,// 83 PAY  80 

    0x5929c70b,// 84 PAY  81 

    0x132c2615,// 85 PAY  82 

    0xfa597657,// 86 PAY  83 

    0x52610ad2,// 87 PAY  84 

    0xcd57feb7,// 88 PAY  85 

    0x38e330e3,// 89 PAY  86 

    0xf1c510b0,// 90 PAY  87 

    0x9b8c5369,// 91 PAY  88 

    0x89f0c242,// 92 PAY  89 

    0xed284195,// 93 PAY  90 

    0x13d7258f,// 94 PAY  91 

    0x079ebb70,// 95 PAY  92 

    0xc18484e3,// 96 PAY  93 

    0xa4f221cc,// 97 PAY  94 

    0x7b35fddf,// 98 PAY  95 

    0xa4be44f2,// 99 PAY  96 

    0xe2ef8bd0,// 100 PAY  97 

    0x1a7da59f,// 101 PAY  98 

    0x86305527,// 102 PAY  99 

    0x974d79e3,// 103 PAY 100 

    0x9d03baad,// 104 PAY 101 

    0xd084243e,// 105 PAY 102 

    0xb7423594,// 106 PAY 103 

    0x931db9eb,// 107 PAY 104 

    0xe1fb9c51,// 108 PAY 105 

    0x564a334f,// 109 PAY 106 

    0x1e8f109a,// 110 PAY 107 

    0xdb67292c,// 111 PAY 108 

    0x1855a04d,// 112 PAY 109 

    0xd16a2fc0,// 113 PAY 110 

    0xcf4c0164,// 114 PAY 111 

    0x02f9522d,// 115 PAY 112 

    0x3406262b,// 116 PAY 113 

    0xe55ce91d,// 117 PAY 114 

    0x5fc87167,// 118 PAY 115 

    0x26291a35,// 119 PAY 116 

    0xd575dbf2,// 120 PAY 117 

    0x1c329e9e,// 121 PAY 118 

    0xef0a6497,// 122 PAY 119 

    0xad4b69a1,// 123 PAY 120 

    0xb98fadf5,// 124 PAY 121 

    0xdf7cbf5d,// 125 PAY 122 

    0xb329b88a,// 126 PAY 123 

    0x747a86b8,// 127 PAY 124 

    0x9bec9c8a,// 128 PAY 125 

    0x8bb98cc1,// 129 PAY 126 

    0x25cfff29,// 130 PAY 127 

    0x4a81367a,// 131 PAY 128 

    0xcdc0888b,// 132 PAY 129 

    0x9661e915,// 133 PAY 130 

    0x6417a240,// 134 PAY 131 

    0xd0cd7c81,// 135 PAY 132 

    0x89e0fbb1,// 136 PAY 133 

    0x899ef9a5,// 137 PAY 134 

    0x932e72e7,// 138 PAY 135 

    0x752aff25,// 139 PAY 136 

    0xc40b05a6,// 140 PAY 137 

    0x3ea3911b,// 141 PAY 138 

    0x3c79a093,// 142 PAY 139 

    0x2e372586,// 143 PAY 140 

    0x85bf7037,// 144 PAY 141 

    0x31ebad29,// 145 PAY 142 

    0x36c00d84,// 146 PAY 143 

    0xb0093cc8,// 147 PAY 144 

    0xb1bc3711,// 148 PAY 145 

    0x0f6eab4d,// 149 PAY 146 

    0x41288859,// 150 PAY 147 

    0x75b42f1c,// 151 PAY 148 

    0xdee0171a,// 152 PAY 149 

    0x5dcb442b,// 153 PAY 150 

    0xd76c9c49,// 154 PAY 151 

    0x5bdf760b,// 155 PAY 152 

    0x149227ae,// 156 PAY 153 

    0x77aa3688,// 157 PAY 154 

    0x2c013112,// 158 PAY 155 

    0x6a121ca7,// 159 PAY 156 

    0x0b694e63,// 160 PAY 157 

    0x01bca2f6,// 161 PAY 158 

    0xf9a14f2e,// 162 PAY 159 

    0x0ab08b1b,// 163 PAY 160 

    0x641bb801,// 164 PAY 161 

    0xfbc37ab3,// 165 PAY 162 

    0x3d3c7176,// 166 PAY 163 

    0x6d7078ce,// 167 PAY 164 

    0x209f7214,// 168 PAY 165 

    0x7a4422d1,// 169 PAY 166 

    0xb39622c2,// 170 PAY 167 

    0x9977427d,// 171 PAY 168 

    0xeaeaea4f,// 172 PAY 169 

    0xbdcad0f2,// 173 PAY 170 

    0x4f8dd25f,// 174 PAY 171 

    0x573eda24,// 175 PAY 172 

    0xc41e4533,// 176 PAY 173 

    0x43fdd143,// 177 PAY 174 

    0x21607274,// 178 PAY 175 

    0x73ffa202,// 179 PAY 176 

    0xf778f539,// 180 PAY 177 

    0x668b0812,// 181 PAY 178 

    0x9c82de42,// 182 PAY 179 

    0x9d2373d9,// 183 PAY 180 

    0xb2a84d30,// 184 PAY 181 

    0xd3147d3b,// 185 PAY 182 

    0x4fbba408,// 186 PAY 183 

    0xdf8703d4,// 187 PAY 184 

    0x1f69ffe8,// 188 PAY 185 

    0x51f8f963,// 189 PAY 186 

    0x24a3a610,// 190 PAY 187 

    0xf4a04bd3,// 191 PAY 188 

    0x62e9323e,// 192 PAY 189 

    0x701a4c2e,// 193 PAY 190 

    0x054279bd,// 194 PAY 191 

    0xe66965fe,// 195 PAY 192 

    0x189f9210,// 196 PAY 193 

    0x374f3ed3,// 197 PAY 194 

    0x7c0295e8,// 198 PAY 195 

    0xcde467b2,// 199 PAY 196 

    0xf16f0b5c,// 200 PAY 197 

    0x472bbd75,// 201 PAY 198 

    0xdab4191f,// 202 PAY 199 

    0x6be4db4c,// 203 PAY 200 

    0x4d435ece,// 204 PAY 201 

    0x797b2acf,// 205 PAY 202 

    0x0835b640,// 206 PAY 203 

    0x8b4db065,// 207 PAY 204 

    0x476bc0ca,// 208 PAY 205 

    0x9617d4b2,// 209 PAY 206 

    0xfdf48997,// 210 PAY 207 

    0x4b4e709f,// 211 PAY 208 

    0x9871f6e5,// 212 PAY 209 

    0x30fcf22b,// 213 PAY 210 

    0x6d3af871,// 214 PAY 211 

    0x615d533e,// 215 PAY 212 

    0xbddce53d,// 216 PAY 213 

    0xb1272f98,// 217 PAY 214 

    0x0e882ace,// 218 PAY 215 

    0x3e0a55bb,// 219 PAY 216 

    0xb8c9abe7,// 220 PAY 217 

    0x53984a44,// 221 PAY 218 

    0xcaae511c,// 222 PAY 219 

    0xe02ffd27,// 223 PAY 220 

    0x6402d35c,// 224 PAY 221 

    0x7602f64c,// 225 PAY 222 

    0xac57a976,// 226 PAY 223 

    0xe956d8ce,// 227 PAY 224 

    0xbe5135e6,// 228 PAY 225 

    0xbc8ad9dd,// 229 PAY 226 

    0xecfb904e,// 230 PAY 227 

    0x2771794e,// 231 PAY 228 

    0xeb03e6be,// 232 PAY 229 

    0xbc5caa2d,// 233 PAY 230 

    0xe4b81cc0,// 234 PAY 231 

    0x4315c505,// 235 PAY 232 

    0x354e844a,// 236 PAY 233 

    0xaca03bdd,// 237 PAY 234 

    0xa241af51,// 238 PAY 235 

    0xa8705c2f,// 239 PAY 236 

    0xfecc7bf6,// 240 PAY 237 

    0xfdcda17f,// 241 PAY 238 

    0xe3436005,// 242 PAY 239 

    0x6dda1b71,// 243 PAY 240 

    0x6e3c4a9b,// 244 PAY 241 

    0xf03720ea,// 245 PAY 242 

    0x01712303,// 246 PAY 243 

    0xf2040e37,// 247 PAY 244 

    0x83f20e93,// 248 PAY 245 

    0x66163d04,// 249 PAY 246 

    0xf01d41fe,// 250 PAY 247 

    0x8e08f0c4,// 251 PAY 248 

    0x119f239e,// 252 PAY 249 

    0x2323e75e,// 253 PAY 250 

    0x58fb557d,// 254 PAY 251 

    0x4b75e9a1,// 255 PAY 252 

    0x251a3c9d,// 256 PAY 253 

    0xfee95342,// 257 PAY 254 

    0xa4920104,// 258 PAY 255 

    0xb23a44ac,// 259 PAY 256 

    0x4a07e0aa,// 260 PAY 257 

    0x4f21aff1,// 261 PAY 258 

    0xf5097ef1,// 262 PAY 259 

    0xf19628c8,// 263 PAY 260 

    0x2972a625,// 264 PAY 261 

    0x4e3d8a4a,// 265 PAY 262 

    0xe460280f,// 266 PAY 263 

    0x207f4ca8,// 267 PAY 264 

    0x3b89cc29,// 268 PAY 265 

    0xb95e5d30,// 269 PAY 266 

    0x1794cb84,// 270 PAY 267 

    0xd5499857,// 271 PAY 268 

    0xc52853d1,// 272 PAY 269 

    0x31dd7d83,// 273 PAY 270 

    0xec2fb238,// 274 PAY 271 

    0x5766f302,// 275 PAY 272 

    0xe666da49,// 276 PAY 273 

    0x731dc72e,// 277 PAY 274 

    0x97864f92,// 278 PAY 275 

    0x0ad66e65,// 279 PAY 276 

    0x642591a8,// 280 PAY 277 

    0x527b1a47,// 281 PAY 278 

    0xa25d7607,// 282 PAY 279 

    0x90765585,// 283 PAY 280 

    0x5a23b650,// 284 PAY 281 

    0x730fb71e,// 285 PAY 282 

    0x8d1e96f0,// 286 PAY 283 

    0xdec46be2,// 287 PAY 284 

    0x94fd7b39,// 288 PAY 285 

    0xa2bf5794,// 289 PAY 286 

    0xde0d45ad,// 290 PAY 287 

    0xb1152b01,// 291 PAY 288 

    0x481dae30,// 292 PAY 289 

    0x41ec427c,// 293 PAY 290 

    0x3b4ce3a7,// 294 PAY 291 

    0xe4f484cc,// 295 PAY 292 

    0x36eaa084,// 296 PAY 293 

    0x81b99d73,// 297 PAY 294 

    0x1b74c21f,// 298 PAY 295 

    0x2675e546,// 299 PAY 296 

    0x0c2e61e9,// 300 PAY 297 

    0xc16e517c,// 301 PAY 298 

    0x730b32d7,// 302 PAY 299 

    0x2621cf0b,// 303 PAY 300 

    0xc4741460,// 304 PAY 301 

    0x978440b6,// 305 PAY 302 

    0xe5a36859,// 306 PAY 303 

    0x5232f8d5,// 307 PAY 304 

    0x5b5e8246,// 308 PAY 305 

    0x66704517,// 309 PAY 306 

    0x15130952,// 310 PAY 307 

    0xf94d6ff1,// 311 PAY 308 

    0x7289362f,// 312 PAY 309 

    0x4d79434b,// 313 PAY 310 

    0x5188d768,// 314 PAY 311 

    0xb4eb9b8a,// 315 PAY 312 

    0x45e948ec,// 316 PAY 313 

    0x2a740092,// 317 PAY 314 

    0xd5aca97c,// 318 PAY 315 

    0xa3b642fc,// 319 PAY 316 

    0xf8352a43,// 320 PAY 317 

    0x2c1e37b5,// 321 PAY 318 

    0x8358f217,// 322 PAY 319 

    0xf35a000d,// 323 PAY 320 

    0xed7f2eb7,// 324 PAY 321 

    0x1a40205c,// 325 PAY 322 

    0x393370ed,// 326 PAY 323 

    0x06343dd3,// 327 PAY 324 

    0x7419013c,// 328 PAY 325 

    0xf8dd8035,// 329 PAY 326 

    0x0f900c32,// 330 PAY 327 

    0x1fb5c2ee,// 331 PAY 328 

    0x83bb9426,// 332 PAY 329 

    0x85ebdc31,// 333 PAY 330 

    0xd7f6c78c,// 334 PAY 331 

    0x2d5b2a62,// 335 PAY 332 

    0xf1a8436b,// 336 PAY 333 

    0xa8297a2e,// 337 PAY 334 

    0xd490604f,// 338 PAY 335 

    0x38675b87,// 339 PAY 336 

    0x95a9fa46,// 340 PAY 337 

    0xb9c08ba8,// 341 PAY 338 

    0x3a873ed9,// 342 PAY 339 

    0xacf3d340,// 343 PAY 340 

    0x0d34ec2a,// 344 PAY 341 

    0x059c1c02,// 345 PAY 342 

    0x2d0f89f6,// 346 PAY 343 

    0x6503909b,// 347 PAY 344 

    0x2d98eccd,// 348 PAY 345 

    0xa10488f2,// 349 PAY 346 

    0x2e50cd19,// 350 PAY 347 

    0xa7dd6bc9,// 351 PAY 348 

    0x88a04fd4,// 352 PAY 349 

    0x6db5d57b,// 353 PAY 350 

    0x6a0882a1,// 354 PAY 351 

    0xc64a841c,// 355 PAY 352 

    0x244e061e,// 356 PAY 353 

    0x845345ef,// 357 PAY 354 

    0xadf28dc8,// 358 PAY 355 

    0xb20defc3,// 359 PAY 356 

    0xaffeef0f,// 360 PAY 357 

    0x36b879f7,// 361 PAY 358 

    0x275e3663,// 362 PAY 359 

    0x25b9c039,// 363 PAY 360 

    0x86845ad0,// 364 PAY 361 

    0x1f43f88a,// 365 PAY 362 

    0x5ac83278,// 366 PAY 363 

    0x952ead86,// 367 PAY 364 

    0x2b075d4f,// 368 PAY 365 

    0xb6941df5,// 369 PAY 366 

    0x05487274,// 370 PAY 367 

    0x2e8f9fd3,// 371 PAY 368 

    0x0d52a73d,// 372 PAY 369 

    0xfa95a65b,// 373 PAY 370 

    0x1918d263,// 374 PAY 371 

    0x2d6b7f18,// 375 PAY 372 

    0x67ae3462,// 376 PAY 373 

    0xe0eb6054,// 377 PAY 374 

    0x83277472,// 378 PAY 375 

    0x3f007a4a,// 379 PAY 376 

    0x291b7db1,// 380 PAY 377 

    0x8e3a0d0e,// 381 PAY 378 

    0x18628c15,// 382 PAY 379 

    0xefb77cb6,// 383 PAY 380 

    0xf850f11a,// 384 PAY 381 

    0x494bb110,// 385 PAY 382 

    0x285c1b3b,// 386 PAY 383 

    0x55d23f77,// 387 PAY 384 

    0x8bcfa743,// 388 PAY 385 

    0x4a418a05,// 389 PAY 386 

    0x1087d879,// 390 PAY 387 

    0x1fd616cc,// 391 PAY 388 

    0x051808fe,// 392 PAY 389 

    0x2cf1699a,// 393 PAY 390 

    0x955c311b,// 394 PAY 391 

    0x67283ff1,// 395 PAY 392 

    0x5c78fe92,// 396 PAY 393 

    0xebeb7074,// 397 PAY 394 

    0x80f49d80,// 398 PAY 395 

    0x370a37ae,// 399 PAY 396 

    0x2b998401,// 400 PAY 397 

    0x0c896628,// 401 PAY 398 

    0x8f7b31de,// 402 PAY 399 

    0xc5b5ab70,// 403 PAY 400 

    0x542d2029,// 404 PAY 401 

    0xd0688117,// 405 PAY 402 

    0x34f7c1fa,// 406 PAY 403 

    0xa08535ca,// 407 PAY 404 

    0x9345e1ff,// 408 PAY 405 

    0xf691a2e4,// 409 PAY 406 

    0xc78df81f,// 410 PAY 407 

    0x4477095e,// 411 PAY 408 

    0x938c0000,// 412 PAY 409 

/// HASH is  4 bytes 

    0x8bcfa743,// 413 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 59 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb6 

    0x0050b63b // 414 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt52_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 317 words. 

/// BDA size     is 1262 (0x4ee) 

/// BDA id       is 0xa607 

    0x04eea607,// 3 BDA   1 

/// PAY Generic Data size   : 1262 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xf8107fef,// 4 PAY   1 

    0x9a6a0ac2,// 5 PAY   2 

    0x9fe0c207,// 6 PAY   3 

    0xd7b35a65,// 7 PAY   4 

    0xfd6301df,// 8 PAY   5 

    0xc0b32da3,// 9 PAY   6 

    0x60964545,// 10 PAY   7 

    0x1948dbda,// 11 PAY   8 

    0x7b716c47,// 12 PAY   9 

    0x32a3eb97,// 13 PAY  10 

    0x7f0f46f6,// 14 PAY  11 

    0xfb6e53e7,// 15 PAY  12 

    0xeb426bc1,// 16 PAY  13 

    0xde8ae597,// 17 PAY  14 

    0x05531453,// 18 PAY  15 

    0x41f609b8,// 19 PAY  16 

    0x04cea21c,// 20 PAY  17 

    0x19068d00,// 21 PAY  18 

    0xc917ccc7,// 22 PAY  19 

    0x3d291894,// 23 PAY  20 

    0x91bd823d,// 24 PAY  21 

    0xf58b968b,// 25 PAY  22 

    0xce978ab3,// 26 PAY  23 

    0xa8d77d05,// 27 PAY  24 

    0xb9c0a016,// 28 PAY  25 

    0xa6a5fd63,// 29 PAY  26 

    0x93f0057d,// 30 PAY  27 

    0xd846882e,// 31 PAY  28 

    0xc926adb5,// 32 PAY  29 

    0x10ca82ca,// 33 PAY  30 

    0x49061f28,// 34 PAY  31 

    0x68c75ae2,// 35 PAY  32 

    0x5441c969,// 36 PAY  33 

    0x1c649dc8,// 37 PAY  34 

    0x7d99a73b,// 38 PAY  35 

    0x30b379cc,// 39 PAY  36 

    0xdbf77a7c,// 40 PAY  37 

    0x6c6f3bf1,// 41 PAY  38 

    0x4809dcbc,// 42 PAY  39 

    0x4bca84ff,// 43 PAY  40 

    0xbf5d1685,// 44 PAY  41 

    0xc3baa784,// 45 PAY  42 

    0x9ce4da13,// 46 PAY  43 

    0xa42b7eac,// 47 PAY  44 

    0xf7aea91f,// 48 PAY  45 

    0xbb7f6d1c,// 49 PAY  46 

    0xfa1cb82e,// 50 PAY  47 

    0x747988b5,// 51 PAY  48 

    0x2e2c688d,// 52 PAY  49 

    0xcba33a85,// 53 PAY  50 

    0xa44a2f73,// 54 PAY  51 

    0xcd217896,// 55 PAY  52 

    0x86349007,// 56 PAY  53 

    0xc4c3e7b2,// 57 PAY  54 

    0x0fd85f98,// 58 PAY  55 

    0x615a65c1,// 59 PAY  56 

    0xdc7e12f6,// 60 PAY  57 

    0xa6e3c618,// 61 PAY  58 

    0x64a2f546,// 62 PAY  59 

    0xfe779dca,// 63 PAY  60 

    0x8641e2e2,// 64 PAY  61 

    0xbbb65b9c,// 65 PAY  62 

    0x1b573d7d,// 66 PAY  63 

    0xd4c24ac8,// 67 PAY  64 

    0x9747b2ef,// 68 PAY  65 

    0xf5a7371c,// 69 PAY  66 

    0x3ef77425,// 70 PAY  67 

    0x8f9f3dd4,// 71 PAY  68 

    0x7687aee4,// 72 PAY  69 

    0x1bdb27fe,// 73 PAY  70 

    0x06014166,// 74 PAY  71 

    0xadec45b4,// 75 PAY  72 

    0x6a3973c6,// 76 PAY  73 

    0x41df9d10,// 77 PAY  74 

    0xf2268e6e,// 78 PAY  75 

    0x784893b1,// 79 PAY  76 

    0x8242c4b3,// 80 PAY  77 

    0xb171fd22,// 81 PAY  78 

    0x38ee964f,// 82 PAY  79 

    0x838479e7,// 83 PAY  80 

    0xd65f0cf1,// 84 PAY  81 

    0x47c896ef,// 85 PAY  82 

    0x5e41fd91,// 86 PAY  83 

    0xea4cb478,// 87 PAY  84 

    0xf3924d2f,// 88 PAY  85 

    0xb63fe9ce,// 89 PAY  86 

    0xfdca432e,// 90 PAY  87 

    0x0f326439,// 91 PAY  88 

    0x1143c356,// 92 PAY  89 

    0xaae994a6,// 93 PAY  90 

    0x1d3447e0,// 94 PAY  91 

    0x0b1a9a9c,// 95 PAY  92 

    0x53781d9e,// 96 PAY  93 

    0xf59f7fe0,// 97 PAY  94 

    0x9042a27b,// 98 PAY  95 

    0xf0e0737f,// 99 PAY  96 

    0x62d6d57f,// 100 PAY  97 

    0x24c6930f,// 101 PAY  98 

    0x726c6bf4,// 102 PAY  99 

    0xc8586970,// 103 PAY 100 

    0xe1992a2c,// 104 PAY 101 

    0x6f31a3cf,// 105 PAY 102 

    0xa9f2831b,// 106 PAY 103 

    0x1c1b8a89,// 107 PAY 104 

    0xd059bf35,// 108 PAY 105 

    0x8b269a02,// 109 PAY 106 

    0xd987e778,// 110 PAY 107 

    0xaf4ae694,// 111 PAY 108 

    0x1db17da6,// 112 PAY 109 

    0xf1c4dc01,// 113 PAY 110 

    0xe581f391,// 114 PAY 111 

    0x7d5a4e26,// 115 PAY 112 

    0x5af036f5,// 116 PAY 113 

    0xede7022b,// 117 PAY 114 

    0x0297c64c,// 118 PAY 115 

    0x1c7d3e76,// 119 PAY 116 

    0x7520d912,// 120 PAY 117 

    0xdd094c07,// 121 PAY 118 

    0xfd01914e,// 122 PAY 119 

    0x6c0e65c2,// 123 PAY 120 

    0x6de7e500,// 124 PAY 121 

    0x16afdfac,// 125 PAY 122 

    0xe428d327,// 126 PAY 123 

    0x76514834,// 127 PAY 124 

    0x15dc13b1,// 128 PAY 125 

    0xba0a1b1f,// 129 PAY 126 

    0xa2b5b1e8,// 130 PAY 127 

    0x452e3a3a,// 131 PAY 128 

    0x20fa34b5,// 132 PAY 129 

    0x67be082e,// 133 PAY 130 

    0x87c530e4,// 134 PAY 131 

    0x7302ec7c,// 135 PAY 132 

    0xbc79ad67,// 136 PAY 133 

    0x7c097852,// 137 PAY 134 

    0xa608a0ca,// 138 PAY 135 

    0xf2543939,// 139 PAY 136 

    0x71e42b03,// 140 PAY 137 

    0xf6b82f2a,// 141 PAY 138 

    0x75651669,// 142 PAY 139 

    0x6dff3192,// 143 PAY 140 

    0xba146e65,// 144 PAY 141 

    0xa7bc81fa,// 145 PAY 142 

    0x999169fe,// 146 PAY 143 

    0xf86d28c9,// 147 PAY 144 

    0xb4de706a,// 148 PAY 145 

    0xbc6fe474,// 149 PAY 146 

    0xf2186d3f,// 150 PAY 147 

    0x184751af,// 151 PAY 148 

    0x87a50325,// 152 PAY 149 

    0x9396527d,// 153 PAY 150 

    0x20384bb7,// 154 PAY 151 

    0x758a955f,// 155 PAY 152 

    0x6f3a0678,// 156 PAY 153 

    0x843c0543,// 157 PAY 154 

    0xb4b4976f,// 158 PAY 155 

    0x1300db5f,// 159 PAY 156 

    0x8adea50e,// 160 PAY 157 

    0x35330ec4,// 161 PAY 158 

    0x252c05f0,// 162 PAY 159 

    0x5e7a3ae7,// 163 PAY 160 

    0x8ed78b73,// 164 PAY 161 

    0x35783b62,// 165 PAY 162 

    0x1532d9a1,// 166 PAY 163 

    0xd621620b,// 167 PAY 164 

    0x3512e33c,// 168 PAY 165 

    0xc31ec95f,// 169 PAY 166 

    0xc58e79a5,// 170 PAY 167 

    0x07d314cc,// 171 PAY 168 

    0xf0a4b6e9,// 172 PAY 169 

    0xd348ef19,// 173 PAY 170 

    0x154c4f45,// 174 PAY 171 

    0x241a08da,// 175 PAY 172 

    0x657c4707,// 176 PAY 173 

    0x7ff48c28,// 177 PAY 174 

    0xb8c1d978,// 178 PAY 175 

    0x1e313802,// 179 PAY 176 

    0x19792c7a,// 180 PAY 177 

    0xc4bf66e0,// 181 PAY 178 

    0x9a63390f,// 182 PAY 179 

    0xfc83d88d,// 183 PAY 180 

    0x7c8a6b21,// 184 PAY 181 

    0x8d0b929e,// 185 PAY 182 

    0xebcaa76c,// 186 PAY 183 

    0x1b9002f4,// 187 PAY 184 

    0xc0c2716b,// 188 PAY 185 

    0x93ed1e7d,// 189 PAY 186 

    0xfb5cece0,// 190 PAY 187 

    0xb855c227,// 191 PAY 188 

    0x1bfade2f,// 192 PAY 189 

    0x588dd5c7,// 193 PAY 190 

    0x14f43e88,// 194 PAY 191 

    0x26a87efc,// 195 PAY 192 

    0x47a768a8,// 196 PAY 193 

    0xb16b2e0b,// 197 PAY 194 

    0x85fd3bf2,// 198 PAY 195 

    0xad86df72,// 199 PAY 196 

    0xb9754d6b,// 200 PAY 197 

    0x2d39465e,// 201 PAY 198 

    0x69031601,// 202 PAY 199 

    0xe780a6fa,// 203 PAY 200 

    0xffb14adb,// 204 PAY 201 

    0x37b73ed4,// 205 PAY 202 

    0x88cdb3a5,// 206 PAY 203 

    0x1105974c,// 207 PAY 204 

    0x71b80376,// 208 PAY 205 

    0x3c2a7f0d,// 209 PAY 206 

    0x0741ff82,// 210 PAY 207 

    0x57bf746e,// 211 PAY 208 

    0x1ae91abf,// 212 PAY 209 

    0xf6d9962b,// 213 PAY 210 

    0xf4530633,// 214 PAY 211 

    0x250aae6b,// 215 PAY 212 

    0x30a43816,// 216 PAY 213 

    0x1e08d9ff,// 217 PAY 214 

    0xb4389376,// 218 PAY 215 

    0xa900c725,// 219 PAY 216 

    0xe902a5d6,// 220 PAY 217 

    0xba044446,// 221 PAY 218 

    0xad8c0ff7,// 222 PAY 219 

    0x2db40df7,// 223 PAY 220 

    0x2171e431,// 224 PAY 221 

    0x7b5fb96a,// 225 PAY 222 

    0xf6239b94,// 226 PAY 223 

    0x3b38573e,// 227 PAY 224 

    0x996c724c,// 228 PAY 225 

    0x8415e425,// 229 PAY 226 

    0x7347b2c8,// 230 PAY 227 

    0x6440c2f5,// 231 PAY 228 

    0x7ce57946,// 232 PAY 229 

    0xfbe70850,// 233 PAY 230 

    0x9ffb2cee,// 234 PAY 231 

    0xdb660e07,// 235 PAY 232 

    0x7245bd37,// 236 PAY 233 

    0x7da5a31d,// 237 PAY 234 

    0x31d9421d,// 238 PAY 235 

    0xd2adf2fd,// 239 PAY 236 

    0x2ca583e9,// 240 PAY 237 

    0xf21f34fe,// 241 PAY 238 

    0xed6db358,// 242 PAY 239 

    0xc266cfc1,// 243 PAY 240 

    0xa8c9a3ce,// 244 PAY 241 

    0xc9338271,// 245 PAY 242 

    0xf5ac9fec,// 246 PAY 243 

    0x050809b2,// 247 PAY 244 

    0x4ec47e52,// 248 PAY 245 

    0x7a1dcc48,// 249 PAY 246 

    0x23c028e6,// 250 PAY 247 

    0x04fc51d2,// 251 PAY 248 

    0xfaa03c58,// 252 PAY 249 

    0x4b068983,// 253 PAY 250 

    0xfe3b136e,// 254 PAY 251 

    0xd12556fc,// 255 PAY 252 

    0xd3e03137,// 256 PAY 253 

    0x3b658115,// 257 PAY 254 

    0x8e9d16b8,// 258 PAY 255 

    0x04e81e69,// 259 PAY 256 

    0xb0820219,// 260 PAY 257 

    0x396b4fbd,// 261 PAY 258 

    0x6baa3e73,// 262 PAY 259 

    0xafcbfac5,// 263 PAY 260 

    0x232c77f7,// 264 PAY 261 

    0x475cbc19,// 265 PAY 262 

    0xe89df8ee,// 266 PAY 263 

    0x272a6820,// 267 PAY 264 

    0x55c7e681,// 268 PAY 265 

    0x76a2f4d0,// 269 PAY 266 

    0x1bc14e5b,// 270 PAY 267 

    0xfabee65d,// 271 PAY 268 

    0x1d4fd39e,// 272 PAY 269 

    0x1228331a,// 273 PAY 270 

    0xb2cdc026,// 274 PAY 271 

    0xe07e5142,// 275 PAY 272 

    0x1361519c,// 276 PAY 273 

    0x3f572156,// 277 PAY 274 

    0x279afc36,// 278 PAY 275 

    0xc453117a,// 279 PAY 276 

    0x9f7bbdba,// 280 PAY 277 

    0xdc8ad25c,// 281 PAY 278 

    0x7d688183,// 282 PAY 279 

    0xe7660349,// 283 PAY 280 

    0x85434596,// 284 PAY 281 

    0x25c42d3c,// 285 PAY 282 

    0xa0f75c1c,// 286 PAY 283 

    0xb74e8619,// 287 PAY 284 

    0xd470183f,// 288 PAY 285 

    0x44b12bb7,// 289 PAY 286 

    0x1447508d,// 290 PAY 287 

    0x796b05eb,// 291 PAY 288 

    0x898cda80,// 292 PAY 289 

    0x18bc4030,// 293 PAY 290 

    0x69e51d20,// 294 PAY 291 

    0x4d597aea,// 295 PAY 292 

    0x32dfdff0,// 296 PAY 293 

    0x4c9c357f,// 297 PAY 294 

    0x175b8c5f,// 298 PAY 295 

    0x49c4bdec,// 299 PAY 296 

    0x73c12c22,// 300 PAY 297 

    0x87d8afd2,// 301 PAY 298 

    0x0cc37825,// 302 PAY 299 

    0xbcba6d08,// 303 PAY 300 

    0xed4815f2,// 304 PAY 301 

    0xffafc7ba,// 305 PAY 302 

    0x385d04d9,// 306 PAY 303 

    0x6c0bf8a1,// 307 PAY 304 

    0x326c1397,// 308 PAY 305 

    0xdd3ef823,// 309 PAY 306 

    0x964be5e7,// 310 PAY 307 

    0xf1ecda4e,// 311 PAY 308 

    0xf7a59f68,// 312 PAY 309 

    0x2120cd38,// 313 PAY 310 

    0xa8be8b0f,// 314 PAY 311 

    0xd7fe6476,// 315 PAY 312 

    0xc0dd664d,// 316 PAY 313 

    0x8ebcf7ab,// 317 PAY 314 

    0xeafec9d7,// 318 PAY 315 

    0xebd10000,// 319 PAY 316 

/// STA is 1 words. 

/// STA num_pkts       : 163 

/// STA pkt_idx        : 246 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x58 

    0x03d958a3 // 320 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt53_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 156 words. 

/// BDA size     is 617 (0x269) 

/// BDA id       is 0x7e9d 

    0x02697e9d,// 3 BDA   1 

/// PAY Generic Data size   : 617 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xf6aa34a8,// 4 PAY   1 

    0xef3686ed,// 5 PAY   2 

    0x33d6a2b8,// 6 PAY   3 

    0x9089857f,// 7 PAY   4 

    0xa08648ca,// 8 PAY   5 

    0xf0ef0ff8,// 9 PAY   6 

    0xb2e961a3,// 10 PAY   7 

    0xd34888e8,// 11 PAY   8 

    0x93f42729,// 12 PAY   9 

    0x65f7250e,// 13 PAY  10 

    0x08ade4aa,// 14 PAY  11 

    0xa4b3e0f0,// 15 PAY  12 

    0xcacb90f9,// 16 PAY  13 

    0x08edeaa2,// 17 PAY  14 

    0x9bd78fa9,// 18 PAY  15 

    0x927758d0,// 19 PAY  16 

    0xd116024a,// 20 PAY  17 

    0x96bb5194,// 21 PAY  18 

    0x995db6ab,// 22 PAY  19 

    0x4828aad9,// 23 PAY  20 

    0x1a2ed6df,// 24 PAY  21 

    0x1fb2e651,// 25 PAY  22 

    0x32436b96,// 26 PAY  23 

    0xe4a02a84,// 27 PAY  24 

    0xbd1c79f0,// 28 PAY  25 

    0xf3dae9aa,// 29 PAY  26 

    0x38be2512,// 30 PAY  27 

    0x9775b5f2,// 31 PAY  28 

    0x32c85fc5,// 32 PAY  29 

    0xe6708e4b,// 33 PAY  30 

    0xf5b50997,// 34 PAY  31 

    0xa054fac3,// 35 PAY  32 

    0x36c95c42,// 36 PAY  33 

    0x3cb91ef0,// 37 PAY  34 

    0x5b44f993,// 38 PAY  35 

    0x10654b9a,// 39 PAY  36 

    0x9a6c5270,// 40 PAY  37 

    0x12968416,// 41 PAY  38 

    0xee126c23,// 42 PAY  39 

    0x6e2d3b0a,// 43 PAY  40 

    0x7b90da20,// 44 PAY  41 

    0xfcb7c10c,// 45 PAY  42 

    0x2751e346,// 46 PAY  43 

    0x2c63daca,// 47 PAY  44 

    0xda140988,// 48 PAY  45 

    0x3a55353b,// 49 PAY  46 

    0x89588170,// 50 PAY  47 

    0xe7a42efb,// 51 PAY  48 

    0x5c698d48,// 52 PAY  49 

    0x23a02e5d,// 53 PAY  50 

    0xe68fb2e7,// 54 PAY  51 

    0x9705973d,// 55 PAY  52 

    0xf786651f,// 56 PAY  53 

    0x3833c730,// 57 PAY  54 

    0x96515fbf,// 58 PAY  55 

    0x78ca5c5c,// 59 PAY  56 

    0xd89299a8,// 60 PAY  57 

    0x574a6a61,// 61 PAY  58 

    0xd978977e,// 62 PAY  59 

    0xae8f4857,// 63 PAY  60 

    0x632b4d7a,// 64 PAY  61 

    0x167d721c,// 65 PAY  62 

    0x29a7505e,// 66 PAY  63 

    0x36dba929,// 67 PAY  64 

    0x87e8a29a,// 68 PAY  65 

    0xaded9bc1,// 69 PAY  66 

    0xffb9c6d7,// 70 PAY  67 

    0x42120664,// 71 PAY  68 

    0x6a105d33,// 72 PAY  69 

    0xcbaf4725,// 73 PAY  70 

    0xf34d1b74,// 74 PAY  71 

    0x8cae55eb,// 75 PAY  72 

    0xd89f5e64,// 76 PAY  73 

    0x6a197444,// 77 PAY  74 

    0x5a66c906,// 78 PAY  75 

    0xab7029a6,// 79 PAY  76 

    0xcdf6fb6c,// 80 PAY  77 

    0xf839d287,// 81 PAY  78 

    0x43c1f3b3,// 82 PAY  79 

    0x9c88fe14,// 83 PAY  80 

    0x8addc7fd,// 84 PAY  81 

    0x8a6d05b7,// 85 PAY  82 

    0xf7937f90,// 86 PAY  83 

    0x44353cbd,// 87 PAY  84 

    0x0063ee9b,// 88 PAY  85 

    0x77f7c902,// 89 PAY  86 

    0x691ac262,// 90 PAY  87 

    0x2059946c,// 91 PAY  88 

    0x9615c410,// 92 PAY  89 

    0x632f23eb,// 93 PAY  90 

    0xa6508cdd,// 94 PAY  91 

    0x0f2b7721,// 95 PAY  92 

    0x36d884cf,// 96 PAY  93 

    0x5474259c,// 97 PAY  94 

    0x51664c76,// 98 PAY  95 

    0x52fb6b06,// 99 PAY  96 

    0x463ea431,// 100 PAY  97 

    0x774c40e3,// 101 PAY  98 

    0xbd320673,// 102 PAY  99 

    0x1f357a9e,// 103 PAY 100 

    0x26b5d3ef,// 104 PAY 101 

    0xbdfec7a8,// 105 PAY 102 

    0x9d4c5a97,// 106 PAY 103 

    0x7a205d43,// 107 PAY 104 

    0xf4288d7e,// 108 PAY 105 

    0x3283aa2e,// 109 PAY 106 

    0x9dbf839b,// 110 PAY 107 

    0x254f1b38,// 111 PAY 108 

    0x3866425f,// 112 PAY 109 

    0x08fe9ebf,// 113 PAY 110 

    0xc1f3a6d1,// 114 PAY 111 

    0x1850248b,// 115 PAY 112 

    0xfaa05b3d,// 116 PAY 113 

    0xa33c954e,// 117 PAY 114 

    0xdebd12a5,// 118 PAY 115 

    0x30da2489,// 119 PAY 116 

    0x862e6bf6,// 120 PAY 117 

    0x13f69b24,// 121 PAY 118 

    0xdeab0636,// 122 PAY 119 

    0x1ed05ebd,// 123 PAY 120 

    0x59102003,// 124 PAY 121 

    0x8612fcad,// 125 PAY 122 

    0x17de5751,// 126 PAY 123 

    0x16bbdf08,// 127 PAY 124 

    0x9785cc1a,// 128 PAY 125 

    0x9d1a423a,// 129 PAY 126 

    0x7d83496c,// 130 PAY 127 

    0x6412f2f3,// 131 PAY 128 

    0x8c5ec5b0,// 132 PAY 129 

    0x88fdef2b,// 133 PAY 130 

    0x8d67e242,// 134 PAY 131 

    0x0e98b4d5,// 135 PAY 132 

    0x60d1834e,// 136 PAY 133 

    0x7739e47e,// 137 PAY 134 

    0x0a989367,// 138 PAY 135 

    0xba2a950c,// 139 PAY 136 

    0xf989d254,// 140 PAY 137 

    0x2f8ed635,// 141 PAY 138 

    0xc5e8b28d,// 142 PAY 139 

    0xb2bbf205,// 143 PAY 140 

    0x7f245f69,// 144 PAY 141 

    0x19252561,// 145 PAY 142 

    0x044a6799,// 146 PAY 143 

    0x90a72c5e,// 147 PAY 144 

    0x3d6bade3,// 148 PAY 145 

    0x34ebb6e6,// 149 PAY 146 

    0xe8db5e68,// 150 PAY 147 

    0x75240e86,// 151 PAY 148 

    0x55b28510,// 152 PAY 149 

    0x3909da18,// 153 PAY 150 

    0xa66106bf,// 154 PAY 151 

    0xa9f389a2,// 155 PAY 152 

    0x2aa0b900,// 156 PAY 153 

    0x7fdeb78d,// 157 PAY 154 

    0xe7000000,// 158 PAY 155 

/// HASH is  16 bytes 

    0x8612fcad,// 159 HSH   1 

    0x17de5751,// 160 HSH   2 

    0x16bbdf08,// 161 HSH   3 

    0x9785cc1a,// 162 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 6 

/// STA pkt_idx        : 44 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf8 

    0x00b1f806 // 163 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt54_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 103 words. 

/// BDA size     is 408 (0x198) 

/// BDA id       is 0x5aee 

    0x01985aee,// 3 BDA   1 

/// PAY Generic Data size   : 408 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x785c4c46,// 4 PAY   1 

    0x19fbe3e0,// 5 PAY   2 

    0xfa0126c2,// 6 PAY   3 

    0xb8b3e39b,// 7 PAY   4 

    0x74090499,// 8 PAY   5 

    0x2b44c54f,// 9 PAY   6 

    0x580e0870,// 10 PAY   7 

    0xa3c50436,// 11 PAY   8 

    0x05489fc7,// 12 PAY   9 

    0x2a4fe124,// 13 PAY  10 

    0x5961b2a1,// 14 PAY  11 

    0x0b69dd00,// 15 PAY  12 

    0xcd1173cf,// 16 PAY  13 

    0x58eac9da,// 17 PAY  14 

    0x33f37558,// 18 PAY  15 

    0xd72ca5f6,// 19 PAY  16 

    0xe51376a8,// 20 PAY  17 

    0xf8cdf913,// 21 PAY  18 

    0x3e134360,// 22 PAY  19 

    0xc566105a,// 23 PAY  20 

    0x3406d6fa,// 24 PAY  21 

    0xfa4207d9,// 25 PAY  22 

    0xed92db5f,// 26 PAY  23 

    0x1dc87d3b,// 27 PAY  24 

    0x483e2650,// 28 PAY  25 

    0x1db5be4a,// 29 PAY  26 

    0x1ebbd980,// 30 PAY  27 

    0x4ff92b46,// 31 PAY  28 

    0x1d909ba3,// 32 PAY  29 

    0x253c3c17,// 33 PAY  30 

    0x59f825f4,// 34 PAY  31 

    0x828d4619,// 35 PAY  32 

    0x36c9959e,// 36 PAY  33 

    0xfe8d1594,// 37 PAY  34 

    0x9aef6a90,// 38 PAY  35 

    0xa4ab0822,// 39 PAY  36 

    0xf8c9696f,// 40 PAY  37 

    0xdef40e1d,// 41 PAY  38 

    0x40aa7293,// 42 PAY  39 

    0x5012140b,// 43 PAY  40 

    0x88664051,// 44 PAY  41 

    0xf4248802,// 45 PAY  42 

    0x81d9023e,// 46 PAY  43 

    0x990c3013,// 47 PAY  44 

    0x9d7eb2d9,// 48 PAY  45 

    0xc1257b7c,// 49 PAY  46 

    0x05928f63,// 50 PAY  47 

    0xce5c9d05,// 51 PAY  48 

    0x33b4830a,// 52 PAY  49 

    0x206650b9,// 53 PAY  50 

    0x93fc6bb8,// 54 PAY  51 

    0x93f302b9,// 55 PAY  52 

    0x658dc0a0,// 56 PAY  53 

    0xdddcba2c,// 57 PAY  54 

    0x3b40e697,// 58 PAY  55 

    0xf0b1b2c0,// 59 PAY  56 

    0x28098939,// 60 PAY  57 

    0xcb4d5685,// 61 PAY  58 

    0xf3e4f3cf,// 62 PAY  59 

    0xb174071f,// 63 PAY  60 

    0xa5beb2c6,// 64 PAY  61 

    0x16fb6105,// 65 PAY  62 

    0xc918bad5,// 66 PAY  63 

    0x46868b99,// 67 PAY  64 

    0xb4cc07bc,// 68 PAY  65 

    0xd8d76d2a,// 69 PAY  66 

    0xbd59a045,// 70 PAY  67 

    0x412c1380,// 71 PAY  68 

    0x5e11c0a8,// 72 PAY  69 

    0xd771f3cd,// 73 PAY  70 

    0x3ab56fd7,// 74 PAY  71 

    0x2cb29269,// 75 PAY  72 

    0x90cd4c5a,// 76 PAY  73 

    0xcbcbf620,// 77 PAY  74 

    0xfb3b4cbd,// 78 PAY  75 

    0x7f48d728,// 79 PAY  76 

    0xc5c76a5c,// 80 PAY  77 

    0xd9ad4f15,// 81 PAY  78 

    0x226fb674,// 82 PAY  79 

    0x22d0b719,// 83 PAY  80 

    0xb89a15fb,// 84 PAY  81 

    0xa93d6ca2,// 85 PAY  82 

    0x74218fc1,// 86 PAY  83 

    0x1230a7d8,// 87 PAY  84 

    0x3a7e1db1,// 88 PAY  85 

    0x7aa453d9,// 89 PAY  86 

    0x2236dace,// 90 PAY  87 

    0x76df1345,// 91 PAY  88 

    0xb20dbf5b,// 92 PAY  89 

    0x3dee02ba,// 93 PAY  90 

    0xb89e65a2,// 94 PAY  91 

    0xf7093071,// 95 PAY  92 

    0x73fb15e4,// 96 PAY  93 

    0xe068062d,// 97 PAY  94 

    0x5f19aa97,// 98 PAY  95 

    0x2d0dcf41,// 99 PAY  96 

    0xdcadfc9f,// 100 PAY  97 

    0xc33d6857,// 101 PAY  98 

    0xcfb6680e,// 102 PAY  99 

    0x9f0d29a7,// 103 PAY 100 

    0xfeb6b0af,// 104 PAY 101 

    0xfdbaf1e3,// 105 PAY 102 

/// HASH is  12 bytes 

    0xa5beb2c6,// 106 HSH   1 

    0x16fb6105,// 107 HSH   2 

    0xc918bad5,// 108 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 33 

/// STA pkt_idx        : 109 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x19 

    0x01b41921 // 109 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt55_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x03 

/// ECH pdu_tag        : 0x00 

    0x00030000,// 2 ECH   1 

/// BDA is 154 words. 

/// BDA size     is 611 (0x263) 

/// BDA id       is 0x49d1 

    0x026349d1,// 3 BDA   1 

/// PAY Generic Data size   : 611 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x4e5c5313,// 4 PAY   1 

    0xb99f0b67,// 5 PAY   2 

    0x4eb6b62b,// 6 PAY   3 

    0x22ebbb32,// 7 PAY   4 

    0xf975a1b6,// 8 PAY   5 

    0x11079e3c,// 9 PAY   6 

    0x9ccee6cf,// 10 PAY   7 

    0xb1a9896e,// 11 PAY   8 

    0x89eecaa6,// 12 PAY   9 

    0x6de85ae1,// 13 PAY  10 

    0xff852da7,// 14 PAY  11 

    0x0649b26d,// 15 PAY  12 

    0x8462f12c,// 16 PAY  13 

    0x0b3bd1de,// 17 PAY  14 

    0xac023ef3,// 18 PAY  15 

    0x99a2591d,// 19 PAY  16 

    0xb88aea28,// 20 PAY  17 

    0x9adeabb0,// 21 PAY  18 

    0x1c138f8d,// 22 PAY  19 

    0xb3d718af,// 23 PAY  20 

    0x4d5331e8,// 24 PAY  21 

    0x28c5fa65,// 25 PAY  22 

    0xa372c325,// 26 PAY  23 

    0x5a6263ad,// 27 PAY  24 

    0x0f99163e,// 28 PAY  25 

    0x6dc10def,// 29 PAY  26 

    0x76398794,// 30 PAY  27 

    0x9b270b7d,// 31 PAY  28 

    0x3f42d68e,// 32 PAY  29 

    0xd3744ec7,// 33 PAY  30 

    0x8953710a,// 34 PAY  31 

    0x0e7bfdaf,// 35 PAY  32 

    0x65f201af,// 36 PAY  33 

    0xb583dec7,// 37 PAY  34 

    0xbe8aa133,// 38 PAY  35 

    0x785c9cff,// 39 PAY  36 

    0x372b6871,// 40 PAY  37 

    0x3539a61e,// 41 PAY  38 

    0x4ce329ee,// 42 PAY  39 

    0x9a5db398,// 43 PAY  40 

    0x0e48abe2,// 44 PAY  41 

    0x245687be,// 45 PAY  42 

    0xce1ff15d,// 46 PAY  43 

    0xd089546c,// 47 PAY  44 

    0xb9730f9e,// 48 PAY  45 

    0x1b343d36,// 49 PAY  46 

    0xc800c15b,// 50 PAY  47 

    0xd3ea9de0,// 51 PAY  48 

    0xfbd7a42f,// 52 PAY  49 

    0x8b75fb41,// 53 PAY  50 

    0xfb7c8d90,// 54 PAY  51 

    0xb78c131d,// 55 PAY  52 

    0xe8b58efb,// 56 PAY  53 

    0x31561b4c,// 57 PAY  54 

    0xf41faff9,// 58 PAY  55 

    0x971af749,// 59 PAY  56 

    0x34bdf6f8,// 60 PAY  57 

    0xb23eda51,// 61 PAY  58 

    0x2d556bc8,// 62 PAY  59 

    0x32c7e3ff,// 63 PAY  60 

    0x4dddb3a4,// 64 PAY  61 

    0x993b8680,// 65 PAY  62 

    0xb63c4709,// 66 PAY  63 

    0x31ce7931,// 67 PAY  64 

    0xb88da4cd,// 68 PAY  65 

    0x98f4ef9b,// 69 PAY  66 

    0x5ddecf63,// 70 PAY  67 

    0xe6142c53,// 71 PAY  68 

    0x86c63222,// 72 PAY  69 

    0x827132b5,// 73 PAY  70 

    0x7d4c53cb,// 74 PAY  71 

    0x6ba27c80,// 75 PAY  72 

    0xc3b74cc8,// 76 PAY  73 

    0xc93082cb,// 77 PAY  74 

    0xad9461a1,// 78 PAY  75 

    0x0d43e94d,// 79 PAY  76 

    0xd699332a,// 80 PAY  77 

    0x80193bd3,// 81 PAY  78 

    0x2e2fe3d8,// 82 PAY  79 

    0xc8d233e9,// 83 PAY  80 

    0xaeeb88ec,// 84 PAY  81 

    0x039a0f4e,// 85 PAY  82 

    0x1ec15cab,// 86 PAY  83 

    0x9dd9e432,// 87 PAY  84 

    0xc94f8674,// 88 PAY  85 

    0x03808618,// 89 PAY  86 

    0x490a20ba,// 90 PAY  87 

    0xffcee243,// 91 PAY  88 

    0xa9d815e0,// 92 PAY  89 

    0xfe57d871,// 93 PAY  90 

    0xe1826c82,// 94 PAY  91 

    0xcbdf6c41,// 95 PAY  92 

    0x95faf059,// 96 PAY  93 

    0x22f06ad5,// 97 PAY  94 

    0x796dba96,// 98 PAY  95 

    0xcfffbeb7,// 99 PAY  96 

    0x6c206e63,// 100 PAY  97 

    0x9bb7fe60,// 101 PAY  98 

    0x7789a6c4,// 102 PAY  99 

    0xfaea8cb3,// 103 PAY 100 

    0xe42392cc,// 104 PAY 101 

    0x5b87e9b2,// 105 PAY 102 

    0x53edde08,// 106 PAY 103 

    0x263f41c3,// 107 PAY 104 

    0xdeafc8d9,// 108 PAY 105 

    0x26e526c0,// 109 PAY 106 

    0xf063924b,// 110 PAY 107 

    0xd413053f,// 111 PAY 108 

    0xa3ad8615,// 112 PAY 109 

    0xeb628b5f,// 113 PAY 110 

    0xbcacf7fe,// 114 PAY 111 

    0x7a90b9b2,// 115 PAY 112 

    0x93b2bd9a,// 116 PAY 113 

    0x312963db,// 117 PAY 114 

    0x1e46573d,// 118 PAY 115 

    0xdc0651d0,// 119 PAY 116 

    0x01267bcf,// 120 PAY 117 

    0x4a416639,// 121 PAY 118 

    0xcbdd60a9,// 122 PAY 119 

    0x69cdd288,// 123 PAY 120 

    0xcd6f637e,// 124 PAY 121 

    0x66e9d329,// 125 PAY 122 

    0x1f85979f,// 126 PAY 123 

    0x2d29d60b,// 127 PAY 124 

    0xf4d5387a,// 128 PAY 125 

    0x9925b0d3,// 129 PAY 126 

    0xb1b48d5d,// 130 PAY 127 

    0x9b27fa1c,// 131 PAY 128 

    0xb0a5aa2f,// 132 PAY 129 

    0xcec35821,// 133 PAY 130 

    0x1c9c9fe9,// 134 PAY 131 

    0xc2542d7b,// 135 PAY 132 

    0xbff41a63,// 136 PAY 133 

    0xdbb464e1,// 137 PAY 134 

    0xc2e2f861,// 138 PAY 135 

    0x59c0e5d9,// 139 PAY 136 

    0xb9c1af51,// 140 PAY 137 

    0x992ea003,// 141 PAY 138 

    0x9444fd97,// 142 PAY 139 

    0x6a11745b,// 143 PAY 140 

    0x65274d0a,// 144 PAY 141 

    0x336719f7,// 145 PAY 142 

    0x425d6b4d,// 146 PAY 143 

    0x26a0e74e,// 147 PAY 144 

    0x424a2cfa,// 148 PAY 145 

    0xd088901c,// 149 PAY 146 

    0x71269859,// 150 PAY 147 

    0xff737a8d,// 151 PAY 148 

    0x397d04f8,// 152 PAY 149 

    0xa52ae0aa,// 153 PAY 150 

    0xb883ace4,// 154 PAY 151 

    0x1f25e67e,// 155 PAY 152 

    0x11b29500,// 156 PAY 153 

/// STA is 1 words. 

/// STA num_pkts       : 215 

/// STA pkt_idx        : 91 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc4 

    0x016cc4d7 // 157 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt56_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0b 

/// ECH pdu_tag        : 0x00 

    0x000b0000,// 2 ECH   1 

/// BDA is 89 words. 

/// BDA size     is 349 (0x15d) 

/// BDA id       is 0x1b7b 

    0x015d1b7b,// 3 BDA   1 

/// PAY Generic Data size   : 349 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xf0a27648,// 4 PAY   1 

    0x1bdae625,// 5 PAY   2 

    0x4cc5e173,// 6 PAY   3 

    0x1fc0279f,// 7 PAY   4 

    0xbb09934b,// 8 PAY   5 

    0xbe50f83c,// 9 PAY   6 

    0x2163faa5,// 10 PAY   7 

    0x5cb87f18,// 11 PAY   8 

    0xeab902dd,// 12 PAY   9 

    0xe9f51191,// 13 PAY  10 

    0x10d2e7ae,// 14 PAY  11 

    0x1502fd18,// 15 PAY  12 

    0x9d969f8e,// 16 PAY  13 

    0x412c06b4,// 17 PAY  14 

    0xc04d9788,// 18 PAY  15 

    0x14af3a4a,// 19 PAY  16 

    0xd400542f,// 20 PAY  17 

    0x91f919d1,// 21 PAY  18 

    0x0358fd8c,// 22 PAY  19 

    0x70f07c4d,// 23 PAY  20 

    0xebc6bf7a,// 24 PAY  21 

    0x82287cfa,// 25 PAY  22 

    0x1091f923,// 26 PAY  23 

    0xd0d34b95,// 27 PAY  24 

    0xd472c184,// 28 PAY  25 

    0xd4be5368,// 29 PAY  26 

    0xd6ca9570,// 30 PAY  27 

    0xf545f4fa,// 31 PAY  28 

    0xd9b6ef45,// 32 PAY  29 

    0x2d0301d2,// 33 PAY  30 

    0x7786cbb3,// 34 PAY  31 

    0x9e3bedcf,// 35 PAY  32 

    0x967632eb,// 36 PAY  33 

    0x0d7d444b,// 37 PAY  34 

    0xd52cf426,// 38 PAY  35 

    0xeb546712,// 39 PAY  36 

    0x2cd02de5,// 40 PAY  37 

    0x35ec7c70,// 41 PAY  38 

    0x142e66af,// 42 PAY  39 

    0x99b6aca6,// 43 PAY  40 

    0xc3fb6fbb,// 44 PAY  41 

    0xd657770e,// 45 PAY  42 

    0x6ba2668b,// 46 PAY  43 

    0x4e0b3e13,// 47 PAY  44 

    0xcc2c5090,// 48 PAY  45 

    0x34c7f10d,// 49 PAY  46 

    0xc7d909b7,// 50 PAY  47 

    0xdb8918c5,// 51 PAY  48 

    0xcf2b3789,// 52 PAY  49 

    0x364d1bb8,// 53 PAY  50 

    0xee129317,// 54 PAY  51 

    0x1f374f71,// 55 PAY  52 

    0x6533616f,// 56 PAY  53 

    0xee3b29ac,// 57 PAY  54 

    0x1a97a2a7,// 58 PAY  55 

    0xe4194674,// 59 PAY  56 

    0x7259005e,// 60 PAY  57 

    0x47541f9a,// 61 PAY  58 

    0xe36ee295,// 62 PAY  59 

    0x576c18e2,// 63 PAY  60 

    0xdf192761,// 64 PAY  61 

    0x5a073778,// 65 PAY  62 

    0x9980341f,// 66 PAY  63 

    0x4e380821,// 67 PAY  64 

    0x0da937c5,// 68 PAY  65 

    0x59d79ac8,// 69 PAY  66 

    0x69bf8c5a,// 70 PAY  67 

    0xd68b1470,// 71 PAY  68 

    0x0a2e61e8,// 72 PAY  69 

    0x0d31e21a,// 73 PAY  70 

    0xa0f2f439,// 74 PAY  71 

    0xef925fb0,// 75 PAY  72 

    0xd43c541f,// 76 PAY  73 

    0xeb8f5124,// 77 PAY  74 

    0xc42a2d34,// 78 PAY  75 

    0x79948a7a,// 79 PAY  76 

    0x3c37ffd3,// 80 PAY  77 

    0xcea64b87,// 81 PAY  78 

    0x776f25d9,// 82 PAY  79 

    0xd5636a68,// 83 PAY  80 

    0xf626603a,// 84 PAY  81 

    0x69fdb253,// 85 PAY  82 

    0xc889ec7e,// 86 PAY  83 

    0xc1fa2b60,// 87 PAY  84 

    0x8290c69d,// 88 PAY  85 

    0x2d34f2ac,// 89 PAY  86 

    0xb57b9ea6,// 90 PAY  87 

    0xfa000000,// 91 PAY  88 

/// STA is 1 words. 

/// STA num_pkts       : 160 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf7 

    0x02d1f7a0 // 92 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt57_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 260 words. 

/// BDA size     is 1035 (0x40b) 

/// BDA id       is 0xf427 

    0x040bf427,// 3 BDA   1 

/// PAY Generic Data size   : 1035 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x4688e65c,// 4 PAY   1 

    0xc3450fce,// 5 PAY   2 

    0xf43554cf,// 6 PAY   3 

    0x58fee916,// 7 PAY   4 

    0x4c5d7008,// 8 PAY   5 

    0xbf824a22,// 9 PAY   6 

    0x1cb9c2ed,// 10 PAY   7 

    0x0dd58a81,// 11 PAY   8 

    0x692ff012,// 12 PAY   9 

    0x513baaeb,// 13 PAY  10 

    0xf778b767,// 14 PAY  11 

    0x66a487fb,// 15 PAY  12 

    0xfb3c5564,// 16 PAY  13 

    0xb8fac05f,// 17 PAY  14 

    0x84a2260d,// 18 PAY  15 

    0x229dad88,// 19 PAY  16 

    0x05078d99,// 20 PAY  17 

    0x0a56ab85,// 21 PAY  18 

    0x55baf04c,// 22 PAY  19 

    0xec255d5e,// 23 PAY  20 

    0x55d11901,// 24 PAY  21 

    0x4806ff07,// 25 PAY  22 

    0x264cbec0,// 26 PAY  23 

    0xbc74251a,// 27 PAY  24 

    0x55b4801b,// 28 PAY  25 

    0x76cf99ac,// 29 PAY  26 

    0x9c0ec0bf,// 30 PAY  27 

    0x617c74b1,// 31 PAY  28 

    0x94f0712b,// 32 PAY  29 

    0xe47460dc,// 33 PAY  30 

    0x58f2a2f2,// 34 PAY  31 

    0x820b7ebc,// 35 PAY  32 

    0x74d906e2,// 36 PAY  33 

    0xde1210a8,// 37 PAY  34 

    0xc0251594,// 38 PAY  35 

    0xa347b571,// 39 PAY  36 

    0x2d84c9a4,// 40 PAY  37 

    0xd9a392cd,// 41 PAY  38 

    0x111155c9,// 42 PAY  39 

    0xc063832f,// 43 PAY  40 

    0x943e77aa,// 44 PAY  41 

    0x3f052594,// 45 PAY  42 

    0x5e057a5b,// 46 PAY  43 

    0xac42bc5f,// 47 PAY  44 

    0x324d7084,// 48 PAY  45 

    0x449ca7d3,// 49 PAY  46 

    0x76fc9c5e,// 50 PAY  47 

    0x4c54e116,// 51 PAY  48 

    0x09ea6b90,// 52 PAY  49 

    0xd38cc668,// 53 PAY  50 

    0x5087a325,// 54 PAY  51 

    0x7a640920,// 55 PAY  52 

    0xd264d910,// 56 PAY  53 

    0x064329d6,// 57 PAY  54 

    0x0830c88c,// 58 PAY  55 

    0xae566630,// 59 PAY  56 

    0xdf2e5d8d,// 60 PAY  57 

    0x49f87ad6,// 61 PAY  58 

    0xeb068f2d,// 62 PAY  59 

    0x23039853,// 63 PAY  60 

    0xc1809163,// 64 PAY  61 

    0x6202cf6e,// 65 PAY  62 

    0xaa6d3fbc,// 66 PAY  63 

    0x5e6c39b4,// 67 PAY  64 

    0x40154798,// 68 PAY  65 

    0x8620b543,// 69 PAY  66 

    0x5666b9b4,// 70 PAY  67 

    0x11e15e40,// 71 PAY  68 

    0xa4a830c4,// 72 PAY  69 

    0x5e9a47ac,// 73 PAY  70 

    0x555e2c89,// 74 PAY  71 

    0xcd3343f1,// 75 PAY  72 

    0x486e3e84,// 76 PAY  73 

    0xc98980a2,// 77 PAY  74 

    0x4316e5fd,// 78 PAY  75 

    0xc6cb944e,// 79 PAY  76 

    0x9e993551,// 80 PAY  77 

    0x0ce39f1d,// 81 PAY  78 

    0xef686a2b,// 82 PAY  79 

    0xa5ee6b88,// 83 PAY  80 

    0xf7fb816c,// 84 PAY  81 

    0x36a2a7e9,// 85 PAY  82 

    0x01e15406,// 86 PAY  83 

    0xd18329a8,// 87 PAY  84 

    0x78baa1f1,// 88 PAY  85 

    0xd0fec72d,// 89 PAY  86 

    0xfcae0978,// 90 PAY  87 

    0x7d76516d,// 91 PAY  88 

    0x8d95422d,// 92 PAY  89 

    0x17f71de5,// 93 PAY  90 

    0x75224a54,// 94 PAY  91 

    0xd39bfb6a,// 95 PAY  92 

    0xee31a0b8,// 96 PAY  93 

    0x39746012,// 97 PAY  94 

    0x5f3ec2c6,// 98 PAY  95 

    0x8a7624be,// 99 PAY  96 

    0x10950b22,// 100 PAY  97 

    0xf3683204,// 101 PAY  98 

    0x9acc281c,// 102 PAY  99 

    0x4cffa542,// 103 PAY 100 

    0x10d5185a,// 104 PAY 101 

    0x63a15a44,// 105 PAY 102 

    0x6acfa303,// 106 PAY 103 

    0x447aaf36,// 107 PAY 104 

    0x77e00163,// 108 PAY 105 

    0xc21fae96,// 109 PAY 106 

    0x48064ca1,// 110 PAY 107 

    0x3552b5f7,// 111 PAY 108 

    0x0cf50f6d,// 112 PAY 109 

    0x49bdbe1b,// 113 PAY 110 

    0xeca2dd89,// 114 PAY 111 

    0x792083c6,// 115 PAY 112 

    0x10f9d7bd,// 116 PAY 113 

    0x2f085e21,// 117 PAY 114 

    0x97c8dfa9,// 118 PAY 115 

    0xc04dc484,// 119 PAY 116 

    0x598ba57f,// 120 PAY 117 

    0x7cfd587f,// 121 PAY 118 

    0x72e63ada,// 122 PAY 119 

    0xdad53187,// 123 PAY 120 

    0x86dc2f84,// 124 PAY 121 

    0x3933ff68,// 125 PAY 122 

    0xed615647,// 126 PAY 123 

    0x32b1ccf2,// 127 PAY 124 

    0x5fd65fbd,// 128 PAY 125 

    0x802124af,// 129 PAY 126 

    0x0c533cdd,// 130 PAY 127 

    0x28e89ad1,// 131 PAY 128 

    0x442adf51,// 132 PAY 129 

    0x25bb0485,// 133 PAY 130 

    0x63a06e20,// 134 PAY 131 

    0x65c63ff9,// 135 PAY 132 

    0x6cf52b34,// 136 PAY 133 

    0x34cf64d4,// 137 PAY 134 

    0x6f28985c,// 138 PAY 135 

    0x0501e96b,// 139 PAY 136 

    0xe8edfded,// 140 PAY 137 

    0x9546e2f0,// 141 PAY 138 

    0x6790097b,// 142 PAY 139 

    0xf0eb10c6,// 143 PAY 140 

    0xbd6bd919,// 144 PAY 141 

    0x2ebd1ab2,// 145 PAY 142 

    0x9a7f3aef,// 146 PAY 143 

    0xc13222f3,// 147 PAY 144 

    0xdc6969d0,// 148 PAY 145 

    0x42a963dd,// 149 PAY 146 

    0x46e1b130,// 150 PAY 147 

    0x021120f6,// 151 PAY 148 

    0x159bd2c7,// 152 PAY 149 

    0x39590e15,// 153 PAY 150 

    0x169c3c01,// 154 PAY 151 

    0x3816ec22,// 155 PAY 152 

    0xada43646,// 156 PAY 153 

    0xb0a2a1ac,// 157 PAY 154 

    0x5f19ee25,// 158 PAY 155 

    0xd734526c,// 159 PAY 156 

    0x82281f88,// 160 PAY 157 

    0xbb542a79,// 161 PAY 158 

    0x626efaa9,// 162 PAY 159 

    0x50d62ae9,// 163 PAY 160 

    0x54b40335,// 164 PAY 161 

    0x654a2096,// 165 PAY 162 

    0x0b3448b9,// 166 PAY 163 

    0x641c5ffb,// 167 PAY 164 

    0x6457cb6d,// 168 PAY 165 

    0xac559ec4,// 169 PAY 166 

    0x898bc5da,// 170 PAY 167 

    0x157dcc21,// 171 PAY 168 

    0x66fc0f19,// 172 PAY 169 

    0x9e68534e,// 173 PAY 170 

    0x1b18980a,// 174 PAY 171 

    0xadf119c1,// 175 PAY 172 

    0xaae2caa5,// 176 PAY 173 

    0xe3662e0e,// 177 PAY 174 

    0xfffb242e,// 178 PAY 175 

    0xdac8647e,// 179 PAY 176 

    0xe171afa7,// 180 PAY 177 

    0x5e36e304,// 181 PAY 178 

    0xab7bfe25,// 182 PAY 179 

    0xb9545de5,// 183 PAY 180 

    0x17065a9e,// 184 PAY 181 

    0x2d09c42e,// 185 PAY 182 

    0x3b8017c5,// 186 PAY 183 

    0x3859d2ad,// 187 PAY 184 

    0x16d7837b,// 188 PAY 185 

    0x6ff46e2d,// 189 PAY 186 

    0x8012b1c3,// 190 PAY 187 

    0x59096c9d,// 191 PAY 188 

    0x4c246aec,// 192 PAY 189 

    0x00d4dcd2,// 193 PAY 190 

    0xb55b4d52,// 194 PAY 191 

    0x562793b3,// 195 PAY 192 

    0x026eaaee,// 196 PAY 193 

    0x6deb43a7,// 197 PAY 194 

    0x1c32d04d,// 198 PAY 195 

    0x3b943567,// 199 PAY 196 

    0xb004ccaf,// 200 PAY 197 

    0x81abe610,// 201 PAY 198 

    0x85a78a49,// 202 PAY 199 

    0x097f6de5,// 203 PAY 200 

    0x574a653e,// 204 PAY 201 

    0xfe2cb8b3,// 205 PAY 202 

    0x1869fc77,// 206 PAY 203 

    0x48bab9fd,// 207 PAY 204 

    0xd1f192e2,// 208 PAY 205 

    0x655b3516,// 209 PAY 206 

    0x50668e0c,// 210 PAY 207 

    0x84a23bff,// 211 PAY 208 

    0x525c4fe8,// 212 PAY 209 

    0x453b78c3,// 213 PAY 210 

    0x167e4fe2,// 214 PAY 211 

    0x79a11386,// 215 PAY 212 

    0x5be1108f,// 216 PAY 213 

    0xae0670f6,// 217 PAY 214 

    0x68c551ea,// 218 PAY 215 

    0xd9c55738,// 219 PAY 216 

    0x9b80c2e3,// 220 PAY 217 

    0x862e41d5,// 221 PAY 218 

    0x5169cdb0,// 222 PAY 219 

    0x6b107fc4,// 223 PAY 220 

    0x1f925220,// 224 PAY 221 

    0xd11cd97c,// 225 PAY 222 

    0xe8ca495f,// 226 PAY 223 

    0x303df882,// 227 PAY 224 

    0x2b610de5,// 228 PAY 225 

    0x7df17b7a,// 229 PAY 226 

    0xb041a573,// 230 PAY 227 

    0x779eb9dc,// 231 PAY 228 

    0x42a22fbb,// 232 PAY 229 

    0x96039a79,// 233 PAY 230 

    0x91a5faa8,// 234 PAY 231 

    0xcb32a219,// 235 PAY 232 

    0x01263dc4,// 236 PAY 233 

    0xa6d65128,// 237 PAY 234 

    0xf8cbb6e5,// 238 PAY 235 

    0xad28ad41,// 239 PAY 236 

    0x8f52eace,// 240 PAY 237 

    0x4e5237e6,// 241 PAY 238 

    0xfdc1c40a,// 242 PAY 239 

    0x461655a5,// 243 PAY 240 

    0xe48fecfa,// 244 PAY 241 

    0x8187bbc5,// 245 PAY 242 

    0x02f93a08,// 246 PAY 243 

    0x82a61ee7,// 247 PAY 244 

    0x35523b3e,// 248 PAY 245 

    0x2dace7b8,// 249 PAY 246 

    0x4ad0c18e,// 250 PAY 247 

    0x1a66be71,// 251 PAY 248 

    0x88d72be6,// 252 PAY 249 

    0xe34ce137,// 253 PAY 250 

    0xb628f549,// 254 PAY 251 

    0x80440f01,// 255 PAY 252 

    0xc7b92ae6,// 256 PAY 253 

    0x64f3a7ae,// 257 PAY 254 

    0xcb6eb0ae,// 258 PAY 255 

    0x2a9dffc2,// 259 PAY 256 

    0x6cc93002,// 260 PAY 257 

    0xa9a0b015,// 261 PAY 258 

    0x8d5fb900,// 262 PAY 259 

/// HASH is  16 bytes 

    0x50d62ae9,// 263 HSH   1 

    0x54b40335,// 264 HSH   2 

    0x654a2096,// 265 HSH   3 

    0x0b3448b9,// 266 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 231 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9 

    0x03dc09e7 // 267 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt58_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 386 words. 

/// BDA size     is 1537 (0x601) 

/// BDA id       is 0x5042 

    0x06015042,// 3 BDA   1 

/// PAY Generic Data size   : 1537 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x1411e8a7,// 4 PAY   1 

    0xb9d3a7ac,// 5 PAY   2 

    0x7f5a8b4a,// 6 PAY   3 

    0x610f490f,// 7 PAY   4 

    0xde88697b,// 8 PAY   5 

    0xe01434d9,// 9 PAY   6 

    0x426f708d,// 10 PAY   7 

    0x730ed8dd,// 11 PAY   8 

    0x7d9c71e9,// 12 PAY   9 

    0x8634e829,// 13 PAY  10 

    0x533676a9,// 14 PAY  11 

    0x0e75bd81,// 15 PAY  12 

    0xa7f98f0f,// 16 PAY  13 

    0x295f0909,// 17 PAY  14 

    0x47494c49,// 18 PAY  15 

    0x900d4cab,// 19 PAY  16 

    0x4de5d80a,// 20 PAY  17 

    0x98ddde5f,// 21 PAY  18 

    0xc984896d,// 22 PAY  19 

    0x15f65ba0,// 23 PAY  20 

    0x40ffcf57,// 24 PAY  21 

    0x23b1053b,// 25 PAY  22 

    0x1722ad3f,// 26 PAY  23 

    0x1c32ac62,// 27 PAY  24 

    0x7e63dacc,// 28 PAY  25 

    0x808fbb06,// 29 PAY  26 

    0x0679c151,// 30 PAY  27 

    0x1e28daca,// 31 PAY  28 

    0x2b2cf46c,// 32 PAY  29 

    0xd8237d7b,// 33 PAY  30 

    0x1a6e89b1,// 34 PAY  31 

    0xeacd7b94,// 35 PAY  32 

    0x07ed36a4,// 36 PAY  33 

    0xa001c041,// 37 PAY  34 

    0xb05bf173,// 38 PAY  35 

    0x081d6cd1,// 39 PAY  36 

    0xaec8dafb,// 40 PAY  37 

    0x25a58eac,// 41 PAY  38 

    0x9e9c3f6d,// 42 PAY  39 

    0x6875f8c4,// 43 PAY  40 

    0x761dc557,// 44 PAY  41 

    0x3e0a3e24,// 45 PAY  42 

    0xf40a4a3b,// 46 PAY  43 

    0x1da45178,// 47 PAY  44 

    0xb30c7dc9,// 48 PAY  45 

    0x74cfb848,// 49 PAY  46 

    0x850a226b,// 50 PAY  47 

    0x613a6cb1,// 51 PAY  48 

    0x496cb550,// 52 PAY  49 

    0xce8b9bb2,// 53 PAY  50 

    0x04f2b2fe,// 54 PAY  51 

    0x1b95bce9,// 55 PAY  52 

    0x21d00526,// 56 PAY  53 

    0xe159e064,// 57 PAY  54 

    0x94d338d1,// 58 PAY  55 

    0x487f80d7,// 59 PAY  56 

    0x5f6ab3c1,// 60 PAY  57 

    0xbce71629,// 61 PAY  58 

    0xbeb91018,// 62 PAY  59 

    0x29893e11,// 63 PAY  60 

    0xd6499a03,// 64 PAY  61 

    0xbd939568,// 65 PAY  62 

    0x68d878e2,// 66 PAY  63 

    0x3e0697b1,// 67 PAY  64 

    0x4ad48c95,// 68 PAY  65 

    0x61ed2166,// 69 PAY  66 

    0x88c3a732,// 70 PAY  67 

    0x2057be90,// 71 PAY  68 

    0x8ab2c2c9,// 72 PAY  69 

    0x84d9c269,// 73 PAY  70 

    0xe0769b9e,// 74 PAY  71 

    0x61fce538,// 75 PAY  72 

    0x41d80120,// 76 PAY  73 

    0x24046ac8,// 77 PAY  74 

    0x1979e7d2,// 78 PAY  75 

    0x9f14795e,// 79 PAY  76 

    0x67d05eb7,// 80 PAY  77 

    0xa000cec5,// 81 PAY  78 

    0x4cec2db5,// 82 PAY  79 

    0x983353ad,// 83 PAY  80 

    0xb8f21211,// 84 PAY  81 

    0x05bd4f01,// 85 PAY  82 

    0xb8c11307,// 86 PAY  83 

    0xbf3812dd,// 87 PAY  84 

    0x6858e07b,// 88 PAY  85 

    0x9ae298d6,// 89 PAY  86 

    0x69dd5d50,// 90 PAY  87 

    0x4ba28e2a,// 91 PAY  88 

    0x60150607,// 92 PAY  89 

    0x969e3769,// 93 PAY  90 

    0x237dccae,// 94 PAY  91 

    0x19601f5d,// 95 PAY  92 

    0xce270785,// 96 PAY  93 

    0xe1386fb8,// 97 PAY  94 

    0xf83db524,// 98 PAY  95 

    0xc4eeddc2,// 99 PAY  96 

    0x4e043b89,// 100 PAY  97 

    0x0b80c2c2,// 101 PAY  98 

    0x4c5a7fd3,// 102 PAY  99 

    0x38ffb19b,// 103 PAY 100 

    0x708be665,// 104 PAY 101 

    0x7d631158,// 105 PAY 102 

    0xa008bacf,// 106 PAY 103 

    0xc931ad6f,// 107 PAY 104 

    0x2283e803,// 108 PAY 105 

    0xf9861538,// 109 PAY 106 

    0x3164f625,// 110 PAY 107 

    0x68bd3f34,// 111 PAY 108 

    0x516f6ef9,// 112 PAY 109 

    0x1f05a1b7,// 113 PAY 110 

    0x5ebbd93d,// 114 PAY 111 

    0xb0eccdd0,// 115 PAY 112 

    0xcf9581cb,// 116 PAY 113 

    0x96bb3f6a,// 117 PAY 114 

    0xc71469bd,// 118 PAY 115 

    0x1f1950ac,// 119 PAY 116 

    0x9a0bf046,// 120 PAY 117 

    0x5b773d81,// 121 PAY 118 

    0xce01c1e3,// 122 PAY 119 

    0x317a689a,// 123 PAY 120 

    0xce55d7b3,// 124 PAY 121 

    0x1fdb304e,// 125 PAY 122 

    0xa833e68f,// 126 PAY 123 

    0x2ca75080,// 127 PAY 124 

    0xf08ff185,// 128 PAY 125 

    0x64059044,// 129 PAY 126 

    0xe685a3eb,// 130 PAY 127 

    0x78e8c918,// 131 PAY 128 

    0x12a0638a,// 132 PAY 129 

    0xd762467a,// 133 PAY 130 

    0x3218e553,// 134 PAY 131 

    0x51c88913,// 135 PAY 132 

    0xd81ca7f5,// 136 PAY 133 

    0x9af25167,// 137 PAY 134 

    0x9b6030ba,// 138 PAY 135 

    0xe111ad6e,// 139 PAY 136 

    0x2adadee9,// 140 PAY 137 

    0xeaa4ec22,// 141 PAY 138 

    0x0c532d62,// 142 PAY 139 

    0xd2014e5d,// 143 PAY 140 

    0xfc0b57a5,// 144 PAY 141 

    0x061759d0,// 145 PAY 142 

    0xdf953b18,// 146 PAY 143 

    0xe8237242,// 147 PAY 144 

    0x4e025058,// 148 PAY 145 

    0x69ace428,// 149 PAY 146 

    0xede6749a,// 150 PAY 147 

    0xf7803ad0,// 151 PAY 148 

    0x23307823,// 152 PAY 149 

    0x48c18739,// 153 PAY 150 

    0x6b093367,// 154 PAY 151 

    0x3979742f,// 155 PAY 152 

    0xe24452bd,// 156 PAY 153 

    0x9e875625,// 157 PAY 154 

    0x53169fa3,// 158 PAY 155 

    0x3173a6ee,// 159 PAY 156 

    0x3b540622,// 160 PAY 157 

    0xa4b22514,// 161 PAY 158 

    0x01ca2e3f,// 162 PAY 159 

    0x1d032f24,// 163 PAY 160 

    0xc6393708,// 164 PAY 161 

    0xcc1de6d1,// 165 PAY 162 

    0xf82d18b8,// 166 PAY 163 

    0xf69cc327,// 167 PAY 164 

    0xb403bf30,// 168 PAY 165 

    0x48863c68,// 169 PAY 166 

    0x379107d5,// 170 PAY 167 

    0x2ddbe12a,// 171 PAY 168 

    0x0ccc25a7,// 172 PAY 169 

    0xa0beb03e,// 173 PAY 170 

    0x3236d002,// 174 PAY 171 

    0x071265dc,// 175 PAY 172 

    0x1009f2b5,// 176 PAY 173 

    0xde87c467,// 177 PAY 174 

    0x506a3691,// 178 PAY 175 

    0x0bf827d4,// 179 PAY 176 

    0x3052a5d7,// 180 PAY 177 

    0x40c1b5af,// 181 PAY 178 

    0x7896b2dd,// 182 PAY 179 

    0x3808cf8b,// 183 PAY 180 

    0x572f145b,// 184 PAY 181 

    0xe0747449,// 185 PAY 182 

    0x5fff8984,// 186 PAY 183 

    0x6accb74e,// 187 PAY 184 

    0x8f91bd36,// 188 PAY 185 

    0xe9723df8,// 189 PAY 186 

    0xd16821e2,// 190 PAY 187 

    0x9edefd66,// 191 PAY 188 

    0xaa6c38e3,// 192 PAY 189 

    0x3216285b,// 193 PAY 190 

    0x2ec91ac6,// 194 PAY 191 

    0x476a68f2,// 195 PAY 192 

    0x3c084220,// 196 PAY 193 

    0x0f8ebd3b,// 197 PAY 194 

    0xca15bfa6,// 198 PAY 195 

    0xc8326a7c,// 199 PAY 196 

    0x7d67ccc2,// 200 PAY 197 

    0x61a20a57,// 201 PAY 198 

    0x8c1222dc,// 202 PAY 199 

    0x20cb5ad4,// 203 PAY 200 

    0x7c533373,// 204 PAY 201 

    0xf823da58,// 205 PAY 202 

    0x7d68b442,// 206 PAY 203 

    0x8b959320,// 207 PAY 204 

    0x4702f6ca,// 208 PAY 205 

    0x10f1d532,// 209 PAY 206 

    0xcacaba03,// 210 PAY 207 

    0xaccceb16,// 211 PAY 208 

    0xf7eabfb0,// 212 PAY 209 

    0xa82a5ebb,// 213 PAY 210 

    0xe96a011d,// 214 PAY 211 

    0x491cfe03,// 215 PAY 212 

    0x1ccde768,// 216 PAY 213 

    0x17ea599a,// 217 PAY 214 

    0x641f73c8,// 218 PAY 215 

    0x72d7ad10,// 219 PAY 216 

    0x8d489b22,// 220 PAY 217 

    0x9f44bffe,// 221 PAY 218 

    0x933a1643,// 222 PAY 219 

    0x2c60fdf2,// 223 PAY 220 

    0x346c7219,// 224 PAY 221 

    0x0c637fa6,// 225 PAY 222 

    0xf6fb8cd5,// 226 PAY 223 

    0x0632edda,// 227 PAY 224 

    0xbe692ea1,// 228 PAY 225 

    0xa270d533,// 229 PAY 226 

    0x105f2e7f,// 230 PAY 227 

    0x19f84323,// 231 PAY 228 

    0xd8ce0413,// 232 PAY 229 

    0x1ce03311,// 233 PAY 230 

    0xf9b6685f,// 234 PAY 231 

    0x52d663d4,// 235 PAY 232 

    0x250daa0a,// 236 PAY 233 

    0xda0740b5,// 237 PAY 234 

    0x1f695d2f,// 238 PAY 235 

    0xc3715699,// 239 PAY 236 

    0x14a81fc0,// 240 PAY 237 

    0x66b56dda,// 241 PAY 238 

    0x9be7b69c,// 242 PAY 239 

    0x220504ab,// 243 PAY 240 

    0x1ae51322,// 244 PAY 241 

    0x8219484a,// 245 PAY 242 

    0xb47baafe,// 246 PAY 243 

    0x7d94aa78,// 247 PAY 244 

    0x1d163a5a,// 248 PAY 245 

    0xd532c895,// 249 PAY 246 

    0x0f4f0d75,// 250 PAY 247 

    0x2075a43a,// 251 PAY 248 

    0x319640a5,// 252 PAY 249 

    0xa6301a26,// 253 PAY 250 

    0x7611ae09,// 254 PAY 251 

    0x550b6e7d,// 255 PAY 252 

    0x5fdbeccf,// 256 PAY 253 

    0x492ce344,// 257 PAY 254 

    0x40d03a75,// 258 PAY 255 

    0x3ce970a9,// 259 PAY 256 

    0xa8b14edf,// 260 PAY 257 

    0xd70dc949,// 261 PAY 258 

    0x187fee71,// 262 PAY 259 

    0x70560a62,// 263 PAY 260 

    0xe1eb9a38,// 264 PAY 261 

    0x66419d37,// 265 PAY 262 

    0xf99a7994,// 266 PAY 263 

    0x790d9a8c,// 267 PAY 264 

    0x3b3132f2,// 268 PAY 265 

    0x9dd4d428,// 269 PAY 266 

    0x4f3e9f8f,// 270 PAY 267 

    0xf5dba66d,// 271 PAY 268 

    0x707fc661,// 272 PAY 269 

    0x5bbf2b8e,// 273 PAY 270 

    0x0660c5d2,// 274 PAY 271 

    0xa5bcc2b5,// 275 PAY 272 

    0xb16ebcfa,// 276 PAY 273 

    0x97ab51c4,// 277 PAY 274 

    0x78ac5423,// 278 PAY 275 

    0x51b7dca2,// 279 PAY 276 

    0xe41d08ab,// 280 PAY 277 

    0x1e831eda,// 281 PAY 278 

    0x19fb87d3,// 282 PAY 279 

    0x0fecb00c,// 283 PAY 280 

    0xd07fc0f5,// 284 PAY 281 

    0xec176276,// 285 PAY 282 

    0x23125f0b,// 286 PAY 283 

    0x8f91b42f,// 287 PAY 284 

    0xe58ae3f4,// 288 PAY 285 

    0x8471ccd9,// 289 PAY 286 

    0x23d90500,// 290 PAY 287 

    0x0891fe32,// 291 PAY 288 

    0xb17152dc,// 292 PAY 289 

    0xb3808d6c,// 293 PAY 290 

    0x6ba1bb19,// 294 PAY 291 

    0xe67cd7da,// 295 PAY 292 

    0x9cc6e583,// 296 PAY 293 

    0xab5bafcd,// 297 PAY 294 

    0xeb047b13,// 298 PAY 295 

    0xf0d4c70a,// 299 PAY 296 

    0x2cd3f1fd,// 300 PAY 297 

    0xdfc92ec6,// 301 PAY 298 

    0x552fa2bb,// 302 PAY 299 

    0x29a826a1,// 303 PAY 300 

    0x218fce20,// 304 PAY 301 

    0x43d5aa50,// 305 PAY 302 

    0x035c8a6f,// 306 PAY 303 

    0x4d6fd332,// 307 PAY 304 

    0x3e2cdb84,// 308 PAY 305 

    0x1cc7cbcc,// 309 PAY 306 

    0x90d070db,// 310 PAY 307 

    0xab11b495,// 311 PAY 308 

    0x8a726844,// 312 PAY 309 

    0xd1e36e84,// 313 PAY 310 

    0x1066c867,// 314 PAY 311 

    0x42ce24c0,// 315 PAY 312 

    0xe8a5a3ee,// 316 PAY 313 

    0x151feb9c,// 317 PAY 314 

    0x98c639b8,// 318 PAY 315 

    0xc6c77aae,// 319 PAY 316 

    0xe01503df,// 320 PAY 317 

    0xe00597fb,// 321 PAY 318 

    0x36fc855a,// 322 PAY 319 

    0x09845ae9,// 323 PAY 320 

    0x81528a47,// 324 PAY 321 

    0x7a848921,// 325 PAY 322 

    0x6ca9a76d,// 326 PAY 323 

    0x789eacdf,// 327 PAY 324 

    0x22741b2a,// 328 PAY 325 

    0xbaf1ac56,// 329 PAY 326 

    0xd4ee3554,// 330 PAY 327 

    0x31ceaf39,// 331 PAY 328 

    0xfac5f32d,// 332 PAY 329 

    0xb204a1bc,// 333 PAY 330 

    0x5312d6d3,// 334 PAY 331 

    0x06461ab5,// 335 PAY 332 

    0xc7ea5bd9,// 336 PAY 333 

    0xde1a995d,// 337 PAY 334 

    0x55c52802,// 338 PAY 335 

    0x42136e05,// 339 PAY 336 

    0x74955eb0,// 340 PAY 337 

    0x00b61469,// 341 PAY 338 

    0xb6e810ba,// 342 PAY 339 

    0x4d56e1eb,// 343 PAY 340 

    0x4ade8602,// 344 PAY 341 

    0x9695cc7d,// 345 PAY 342 

    0x47f5266b,// 346 PAY 343 

    0xf05d4354,// 347 PAY 344 

    0xadcf17ba,// 348 PAY 345 

    0xb39651b9,// 349 PAY 346 

    0x96a1e0d2,// 350 PAY 347 

    0xae14828f,// 351 PAY 348 

    0xd18df01e,// 352 PAY 349 

    0x34106858,// 353 PAY 350 

    0x6be25418,// 354 PAY 351 

    0xcd70a870,// 355 PAY 352 

    0xfe324e89,// 356 PAY 353 

    0x5eed4746,// 357 PAY 354 

    0x3a2bb202,// 358 PAY 355 

    0x9c246873,// 359 PAY 356 

    0xb0c49d6e,// 360 PAY 357 

    0x78726b0b,// 361 PAY 358 

    0xa3947e99,// 362 PAY 359 

    0xb7fd8d79,// 363 PAY 360 

    0xb18e3773,// 364 PAY 361 

    0x92831117,// 365 PAY 362 

    0x388abe82,// 366 PAY 363 

    0x1562c09d,// 367 PAY 364 

    0xf7abc442,// 368 PAY 365 

    0xd9a93577,// 369 PAY 366 

    0x8a8d6caf,// 370 PAY 367 

    0xa8925dd0,// 371 PAY 368 

    0x738f1fce,// 372 PAY 369 

    0xa99eac87,// 373 PAY 370 

    0x1c32617f,// 374 PAY 371 

    0x092595fd,// 375 PAY 372 

    0xfd658d83,// 376 PAY 373 

    0xd42b4102,// 377 PAY 374 

    0x3cc2368f,// 378 PAY 375 

    0x88f7f10e,// 379 PAY 376 

    0x148636ad,// 380 PAY 377 

    0x4e133cac,// 381 PAY 378 

    0x6449e8be,// 382 PAY 379 

    0x776df8b1,// 383 PAY 380 

    0xb990f0ee,// 384 PAY 381 

    0x8a2e3dc6,// 385 PAY 382 

    0x4627b294,// 386 PAY 383 

    0x846e3a33,// 387 PAY 384 

    0x58000000,// 388 PAY 385 

/// STA is 1 words. 

/// STA num_pkts       : 207 

/// STA pkt_idx        : 160 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1e 

    0x02811ecf // 389 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt59_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 500 words. 

/// BDA size     is 1995 (0x7cb) 

/// BDA id       is 0x1f49 

    0x07cb1f49,// 3 BDA   1 

/// PAY Generic Data size   : 1995 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x84f42182,// 4 PAY   1 

    0x984e311b,// 5 PAY   2 

    0x307a2b40,// 6 PAY   3 

    0xef04463a,// 7 PAY   4 

    0xaec9e8c2,// 8 PAY   5 

    0x69a46a31,// 9 PAY   6 

    0x2146944f,// 10 PAY   7 

    0x5e19fe81,// 11 PAY   8 

    0x9aecd090,// 12 PAY   9 

    0xda5bec25,// 13 PAY  10 

    0xa455f950,// 14 PAY  11 

    0x028ad785,// 15 PAY  12 

    0x4dc4a0d3,// 16 PAY  13 

    0x479c0166,// 17 PAY  14 

    0xb1e61040,// 18 PAY  15 

    0xb6ab88c6,// 19 PAY  16 

    0xdad6a0d2,// 20 PAY  17 

    0x9a23142a,// 21 PAY  18 

    0xa939976b,// 22 PAY  19 

    0x94931f7f,// 23 PAY  20 

    0xbd0a33e3,// 24 PAY  21 

    0x29447c54,// 25 PAY  22 

    0x082532bb,// 26 PAY  23 

    0x54a01846,// 27 PAY  24 

    0xe7e6ab66,// 28 PAY  25 

    0xef16c332,// 29 PAY  26 

    0xdca5c873,// 30 PAY  27 

    0x61848fce,// 31 PAY  28 

    0x97b8aa35,// 32 PAY  29 

    0xa01cfd7f,// 33 PAY  30 

    0xebf6524c,// 34 PAY  31 

    0x769beb58,// 35 PAY  32 

    0x543f613d,// 36 PAY  33 

    0x59f7b811,// 37 PAY  34 

    0xf53599d3,// 38 PAY  35 

    0x2fa93f2e,// 39 PAY  36 

    0x8bc2ce43,// 40 PAY  37 

    0xb0b596b1,// 41 PAY  38 

    0xe4784018,// 42 PAY  39 

    0x041dc586,// 43 PAY  40 

    0xd5d846dd,// 44 PAY  41 

    0x7ce13be8,// 45 PAY  42 

    0x8ba5fcab,// 46 PAY  43 

    0xeb345232,// 47 PAY  44 

    0xc3d19b0e,// 48 PAY  45 

    0x49c7628c,// 49 PAY  46 

    0x7e5874e7,// 50 PAY  47 

    0x64c7e01c,// 51 PAY  48 

    0xa325bffc,// 52 PAY  49 

    0x8be5456a,// 53 PAY  50 

    0xcc379215,// 54 PAY  51 

    0x23eb59a8,// 55 PAY  52 

    0x83b8ae06,// 56 PAY  53 

    0x0f1aedcc,// 57 PAY  54 

    0xbadf4916,// 58 PAY  55 

    0x17cd805c,// 59 PAY  56 

    0x9d552ab5,// 60 PAY  57 

    0x026b8c16,// 61 PAY  58 

    0x706a4b0d,// 62 PAY  59 

    0x62719e97,// 63 PAY  60 

    0x80b9016b,// 64 PAY  61 

    0xdc939dde,// 65 PAY  62 

    0x8b25c222,// 66 PAY  63 

    0x8174cfb3,// 67 PAY  64 

    0xac94266d,// 68 PAY  65 

    0x0cfe7878,// 69 PAY  66 

    0x76d9e780,// 70 PAY  67 

    0x9a19e28c,// 71 PAY  68 

    0x4e4affa8,// 72 PAY  69 

    0xd518e76c,// 73 PAY  70 

    0x950ad194,// 74 PAY  71 

    0x75e2cb05,// 75 PAY  72 

    0xa472e3d4,// 76 PAY  73 

    0x755b0f0b,// 77 PAY  74 

    0xf1be880e,// 78 PAY  75 

    0xa9520bbe,// 79 PAY  76 

    0x776436cd,// 80 PAY  77 

    0xcc2d9894,// 81 PAY  78 

    0x1fb09284,// 82 PAY  79 

    0xcebb78c7,// 83 PAY  80 

    0x5c13e821,// 84 PAY  81 

    0x0d7d3662,// 85 PAY  82 

    0xfa6c2331,// 86 PAY  83 

    0xcf20d58f,// 87 PAY  84 

    0x1e911418,// 88 PAY  85 

    0xd288b999,// 89 PAY  86 

    0x1659562e,// 90 PAY  87 

    0x49261d00,// 91 PAY  88 

    0xaa99fc4b,// 92 PAY  89 

    0x579cfb26,// 93 PAY  90 

    0x3012b63d,// 94 PAY  91 

    0x5c1aa200,// 95 PAY  92 

    0xecbb4ba4,// 96 PAY  93 

    0x8b16af66,// 97 PAY  94 

    0x9c768208,// 98 PAY  95 

    0x69385a05,// 99 PAY  96 

    0xf6f9b425,// 100 PAY  97 

    0x9c9bad31,// 101 PAY  98 

    0x094dcf79,// 102 PAY  99 

    0x95399beb,// 103 PAY 100 

    0xd5e52d04,// 104 PAY 101 

    0xd41b5136,// 105 PAY 102 

    0x3a0ff924,// 106 PAY 103 

    0x93ed93c6,// 107 PAY 104 

    0x2baef2c1,// 108 PAY 105 

    0xc5896098,// 109 PAY 106 

    0x35ade004,// 110 PAY 107 

    0xeca6d67b,// 111 PAY 108 

    0xed45d0ca,// 112 PAY 109 

    0xa9cecfc2,// 113 PAY 110 

    0x242854c4,// 114 PAY 111 

    0x13d80767,// 115 PAY 112 

    0x802036b3,// 116 PAY 113 

    0x72793541,// 117 PAY 114 

    0xc84e2328,// 118 PAY 115 

    0x3abb81e9,// 119 PAY 116 

    0x50bbb154,// 120 PAY 117 

    0x5b81a5b2,// 121 PAY 118 

    0x4a35edfb,// 122 PAY 119 

    0xaa33fe1a,// 123 PAY 120 

    0x7f41888c,// 124 PAY 121 

    0x1ef479fa,// 125 PAY 122 

    0xfc928479,// 126 PAY 123 

    0x17324f3b,// 127 PAY 124 

    0x604cca32,// 128 PAY 125 

    0xf0aab3b9,// 129 PAY 126 

    0xcf25f75d,// 130 PAY 127 

    0x23d7e3f0,// 131 PAY 128 

    0x9c0b1375,// 132 PAY 129 

    0x2c224a12,// 133 PAY 130 

    0x0f9d4eb3,// 134 PAY 131 

    0x252bf17d,// 135 PAY 132 

    0x190c6759,// 136 PAY 133 

    0x478bd83f,// 137 PAY 134 

    0x6554a8c2,// 138 PAY 135 

    0xc6ba4acb,// 139 PAY 136 

    0x9143b021,// 140 PAY 137 

    0x590fa7ac,// 141 PAY 138 

    0x0c61a36a,// 142 PAY 139 

    0xfd5130fc,// 143 PAY 140 

    0xe586f87b,// 144 PAY 141 

    0x61ca7f67,// 145 PAY 142 

    0xdbb84825,// 146 PAY 143 

    0xf0d6f9c1,// 147 PAY 144 

    0x5b924b95,// 148 PAY 145 

    0x5d8d83ed,// 149 PAY 146 

    0x81f292a5,// 150 PAY 147 

    0x3a85cb53,// 151 PAY 148 

    0x2586507b,// 152 PAY 149 

    0x7041fe11,// 153 PAY 150 

    0x1aef7d40,// 154 PAY 151 

    0x28b97160,// 155 PAY 152 

    0xde240dc9,// 156 PAY 153 

    0xd3f58fdd,// 157 PAY 154 

    0x8f8a138f,// 158 PAY 155 

    0xae1b0557,// 159 PAY 156 

    0x9e81b031,// 160 PAY 157 

    0xed10ae05,// 161 PAY 158 

    0x4342d808,// 162 PAY 159 

    0x7c263dd0,// 163 PAY 160 

    0x99516eb1,// 164 PAY 161 

    0xd89deb23,// 165 PAY 162 

    0x76665243,// 166 PAY 163 

    0xa90b24b8,// 167 PAY 164 

    0x2e85aac5,// 168 PAY 165 

    0xc0a7617c,// 169 PAY 166 

    0x4f2d299f,// 170 PAY 167 

    0xa37f7745,// 171 PAY 168 

    0x3255f7b3,// 172 PAY 169 

    0xf8287f72,// 173 PAY 170 

    0x9f94086a,// 174 PAY 171 

    0xdaf2e412,// 175 PAY 172 

    0x7c722e7b,// 176 PAY 173 

    0x8ad5900b,// 177 PAY 174 

    0xcfb19524,// 178 PAY 175 

    0x17d795da,// 179 PAY 176 

    0x305c6a82,// 180 PAY 177 

    0xe2ac2688,// 181 PAY 178 

    0x84c7a4f6,// 182 PAY 179 

    0x69df3ba2,// 183 PAY 180 

    0xf80868c5,// 184 PAY 181 

    0xe41a516d,// 185 PAY 182 

    0x0a7fa5bb,// 186 PAY 183 

    0x87202ed8,// 187 PAY 184 

    0xe7ab6855,// 188 PAY 185 

    0xb92b98f0,// 189 PAY 186 

    0xeea849ec,// 190 PAY 187 

    0x9df68544,// 191 PAY 188 

    0x2b732727,// 192 PAY 189 

    0xd963d294,// 193 PAY 190 

    0x083a88a0,// 194 PAY 191 

    0x24f08ef5,// 195 PAY 192 

    0x144dd8f1,// 196 PAY 193 

    0x03e26442,// 197 PAY 194 

    0xf7b65a38,// 198 PAY 195 

    0x4eb0e80e,// 199 PAY 196 

    0xc9b706da,// 200 PAY 197 

    0x3be2d5f5,// 201 PAY 198 

    0x3da5dccd,// 202 PAY 199 

    0x35195241,// 203 PAY 200 

    0x62da1d1f,// 204 PAY 201 

    0x3724dd48,// 205 PAY 202 

    0x1c0fd3a4,// 206 PAY 203 

    0x5c46cab5,// 207 PAY 204 

    0x46697159,// 208 PAY 205 

    0xf0359912,// 209 PAY 206 

    0x6524c60b,// 210 PAY 207 

    0x3eb60711,// 211 PAY 208 

    0x9e516826,// 212 PAY 209 

    0xdac58f94,// 213 PAY 210 

    0xbe1e38b2,// 214 PAY 211 

    0x18b76de8,// 215 PAY 212 

    0x8dbb9c94,// 216 PAY 213 

    0x547a089d,// 217 PAY 214 

    0x1988a21c,// 218 PAY 215 

    0x8b0a46b5,// 219 PAY 216 

    0x1503831b,// 220 PAY 217 

    0xd5da4dcd,// 221 PAY 218 

    0x7299e833,// 222 PAY 219 

    0x5c1f155f,// 223 PAY 220 

    0x47f1d81b,// 224 PAY 221 

    0x16c3e049,// 225 PAY 222 

    0xea6f0fc1,// 226 PAY 223 

    0xf4d210ad,// 227 PAY 224 

    0x12093e0c,// 228 PAY 225 

    0x1ea68033,// 229 PAY 226 

    0x9bcabe63,// 230 PAY 227 

    0xd2372910,// 231 PAY 228 

    0x36a00260,// 232 PAY 229 

    0x7235c834,// 233 PAY 230 

    0xb84427e9,// 234 PAY 231 

    0x4cc0a966,// 235 PAY 232 

    0xb28d99cc,// 236 PAY 233 

    0x57f3ce67,// 237 PAY 234 

    0xaa5fb4a9,// 238 PAY 235 

    0x7d6e10e0,// 239 PAY 236 

    0x756b7b76,// 240 PAY 237 

    0x2e47b70c,// 241 PAY 238 

    0x132e938d,// 242 PAY 239 

    0x56c45470,// 243 PAY 240 

    0x7b4b3b8e,// 244 PAY 241 

    0x71ed6372,// 245 PAY 242 

    0x1776dc5b,// 246 PAY 243 

    0xc249a57d,// 247 PAY 244 

    0x50e09cdc,// 248 PAY 245 

    0xe3e026d1,// 249 PAY 246 

    0x0e723a81,// 250 PAY 247 

    0x973e14f6,// 251 PAY 248 

    0x9174b9fc,// 252 PAY 249 

    0x0c4aa28a,// 253 PAY 250 

    0x7e99b2ec,// 254 PAY 251 

    0xfda218e7,// 255 PAY 252 

    0x656a94ae,// 256 PAY 253 

    0xb1c5f724,// 257 PAY 254 

    0xdd629250,// 258 PAY 255 

    0xb7cbfd0a,// 259 PAY 256 

    0x9d4e76b4,// 260 PAY 257 

    0xffe681f8,// 261 PAY 258 

    0x7e5931b0,// 262 PAY 259 

    0xdd90c62a,// 263 PAY 260 

    0xe5c94bbe,// 264 PAY 261 

    0xb28a88b5,// 265 PAY 262 

    0x34bfec76,// 266 PAY 263 

    0x5fec18c9,// 267 PAY 264 

    0xa205d8e7,// 268 PAY 265 

    0x07b963cb,// 269 PAY 266 

    0x4476d5ce,// 270 PAY 267 

    0xa86a0b90,// 271 PAY 268 

    0x64694549,// 272 PAY 269 

    0xd2224ebc,// 273 PAY 270 

    0x50bb063b,// 274 PAY 271 

    0xe5418174,// 275 PAY 272 

    0x2b48a957,// 276 PAY 273 

    0x4c4c9dc0,// 277 PAY 274 

    0x4ab0789c,// 278 PAY 275 

    0xb33ee669,// 279 PAY 276 

    0x4bc33486,// 280 PAY 277 

    0xca4bd54a,// 281 PAY 278 

    0x43b19d95,// 282 PAY 279 

    0xf4ccf62c,// 283 PAY 280 

    0x2e1a0b76,// 284 PAY 281 

    0x1e964558,// 285 PAY 282 

    0xc71249f9,// 286 PAY 283 

    0xd23ec784,// 287 PAY 284 

    0x82a930ed,// 288 PAY 285 

    0xe6ba09a0,// 289 PAY 286 

    0x1f4b475b,// 290 PAY 287 

    0x0f71042e,// 291 PAY 288 

    0xbbaa06ca,// 292 PAY 289 

    0xedc48898,// 293 PAY 290 

    0x438cbc30,// 294 PAY 291 

    0x9fe81316,// 295 PAY 292 

    0x05850868,// 296 PAY 293 

    0x9656b8f0,// 297 PAY 294 

    0x45cd27ed,// 298 PAY 295 

    0x319829d8,// 299 PAY 296 

    0x5e52f244,// 300 PAY 297 

    0x6ade11e3,// 301 PAY 298 

    0xeda5eeab,// 302 PAY 299 

    0xd19163d6,// 303 PAY 300 

    0xdad582ec,// 304 PAY 301 

    0x196071e5,// 305 PAY 302 

    0x381c2509,// 306 PAY 303 

    0xfed7adc7,// 307 PAY 304 

    0x25d3f39e,// 308 PAY 305 

    0xd7538d4e,// 309 PAY 306 

    0x4004856c,// 310 PAY 307 

    0x8e9dc914,// 311 PAY 308 

    0xa76f7013,// 312 PAY 309 

    0x35780ff6,// 313 PAY 310 

    0x5093929a,// 314 PAY 311 

    0x03b3eb14,// 315 PAY 312 

    0x063e31fc,// 316 PAY 313 

    0xd4615185,// 317 PAY 314 

    0x1d12dd49,// 318 PAY 315 

    0xd5e4e8d5,// 319 PAY 316 

    0x2e5f2aa9,// 320 PAY 317 

    0xd45a46f9,// 321 PAY 318 

    0x24ecb69b,// 322 PAY 319 

    0xb809be75,// 323 PAY 320 

    0x6ad416f3,// 324 PAY 321 

    0x3fa5fc64,// 325 PAY 322 

    0x7f437d6d,// 326 PAY 323 

    0xb0e3eb0f,// 327 PAY 324 

    0x01304da1,// 328 PAY 325 

    0x852cbfc9,// 329 PAY 326 

    0xd6cf607f,// 330 PAY 327 

    0x496a85b4,// 331 PAY 328 

    0x395ce093,// 332 PAY 329 

    0x6c5057f3,// 333 PAY 330 

    0x473fe019,// 334 PAY 331 

    0xceb2fd89,// 335 PAY 332 

    0xc23600ff,// 336 PAY 333 

    0x0f105cbc,// 337 PAY 334 

    0x995889f6,// 338 PAY 335 

    0x7140433c,// 339 PAY 336 

    0x61c8c957,// 340 PAY 337 

    0x1530e6a9,// 341 PAY 338 

    0x2fd0a33a,// 342 PAY 339 

    0x3290fcf9,// 343 PAY 340 

    0x7e9fc35b,// 344 PAY 341 

    0xec096d11,// 345 PAY 342 

    0x997ea2aa,// 346 PAY 343 

    0xf1a65fc7,// 347 PAY 344 

    0xe90057a3,// 348 PAY 345 

    0x9e1fe629,// 349 PAY 346 

    0x870d63f3,// 350 PAY 347 

    0x35593eff,// 351 PAY 348 

    0x6dc31984,// 352 PAY 349 

    0xb4a7a1ae,// 353 PAY 350 

    0x0ac7f9c7,// 354 PAY 351 

    0x7b9eaf1e,// 355 PAY 352 

    0x7819e6f5,// 356 PAY 353 

    0xb17f2a01,// 357 PAY 354 

    0xa165bffc,// 358 PAY 355 

    0x4823e423,// 359 PAY 356 

    0xc42f6379,// 360 PAY 357 

    0x51d964ed,// 361 PAY 358 

    0xfa52d534,// 362 PAY 359 

    0x2673c795,// 363 PAY 360 

    0xf10c051a,// 364 PAY 361 

    0x4197dfea,// 365 PAY 362 

    0x2f3512c3,// 366 PAY 363 

    0xa629886a,// 367 PAY 364 

    0x29ac4124,// 368 PAY 365 

    0x472cb575,// 369 PAY 366 

    0x82ab8f6c,// 370 PAY 367 

    0x67008d77,// 371 PAY 368 

    0x351ed91d,// 372 PAY 369 

    0x24018caa,// 373 PAY 370 

    0x96ada729,// 374 PAY 371 

    0xda221212,// 375 PAY 372 

    0x23a3f21f,// 376 PAY 373 

    0x1a290894,// 377 PAY 374 

    0x3e93ffef,// 378 PAY 375 

    0x832354d5,// 379 PAY 376 

    0xc59aab98,// 380 PAY 377 

    0x63e8f534,// 381 PAY 378 

    0x6aaac5d7,// 382 PAY 379 

    0xf2f91458,// 383 PAY 380 

    0x8d96e7cd,// 384 PAY 381 

    0x651c62ab,// 385 PAY 382 

    0x6d512444,// 386 PAY 383 

    0xcce5979c,// 387 PAY 384 

    0x849da775,// 388 PAY 385 

    0xb95d55eb,// 389 PAY 386 

    0xec0179c9,// 390 PAY 387 

    0x64b7c874,// 391 PAY 388 

    0x858970a0,// 392 PAY 389 

    0x1c25bee2,// 393 PAY 390 

    0x341d3781,// 394 PAY 391 

    0x9548c4d8,// 395 PAY 392 

    0xc738c4f0,// 396 PAY 393 

    0x270a4077,// 397 PAY 394 

    0xa65c81c4,// 398 PAY 395 

    0x96666b17,// 399 PAY 396 

    0x7c6e0b36,// 400 PAY 397 

    0x39f837a4,// 401 PAY 398 

    0xb1439c5a,// 402 PAY 399 

    0xd97757c4,// 403 PAY 400 

    0x318b920b,// 404 PAY 401 

    0x5abad888,// 405 PAY 402 

    0x6258dd9d,// 406 PAY 403 

    0x7753a323,// 407 PAY 404 

    0xa8100013,// 408 PAY 405 

    0x14943b07,// 409 PAY 406 

    0xfcf94c16,// 410 PAY 407 

    0xa3ba9af9,// 411 PAY 408 

    0x414b01e8,// 412 PAY 409 

    0xb89c3380,// 413 PAY 410 

    0x915927c0,// 414 PAY 411 

    0x765cbb92,// 415 PAY 412 

    0x35bb8027,// 416 PAY 413 

    0x230ced20,// 417 PAY 414 

    0x2f829af4,// 418 PAY 415 

    0x97450a54,// 419 PAY 416 

    0x7460ef16,// 420 PAY 417 

    0x07b408b3,// 421 PAY 418 

    0xc2812255,// 422 PAY 419 

    0x244e192c,// 423 PAY 420 

    0x327d3910,// 424 PAY 421 

    0xf8a146d7,// 425 PAY 422 

    0xfcc0e5c4,// 426 PAY 423 

    0xfb88670b,// 427 PAY 424 

    0x36005265,// 428 PAY 425 

    0xdf9f9e91,// 429 PAY 426 

    0x5a4ab589,// 430 PAY 427 

    0xa55d340d,// 431 PAY 428 

    0x15a47640,// 432 PAY 429 

    0xe7ca6b1e,// 433 PAY 430 

    0x282eeb22,// 434 PAY 431 

    0x63984a88,// 435 PAY 432 

    0xea3dee0b,// 436 PAY 433 

    0x78f30a10,// 437 PAY 434 

    0x2052fe35,// 438 PAY 435 

    0xd828fa14,// 439 PAY 436 

    0x2d8c7629,// 440 PAY 437 

    0x39480446,// 441 PAY 438 

    0xe56b3cfb,// 442 PAY 439 

    0xca72f7d1,// 443 PAY 440 

    0xd7096e10,// 444 PAY 441 

    0x13c1055a,// 445 PAY 442 

    0xec7a285a,// 446 PAY 443 

    0x836dea3a,// 447 PAY 444 

    0x772d65eb,// 448 PAY 445 

    0x5deb013f,// 449 PAY 446 

    0x08b859b1,// 450 PAY 447 

    0xa2f484ee,// 451 PAY 448 

    0x08eba95b,// 452 PAY 449 

    0xf4cd8fd2,// 453 PAY 450 

    0xa5382fa4,// 454 PAY 451 

    0x9aef4aa7,// 455 PAY 452 

    0xbb778058,// 456 PAY 453 

    0x4b41c88c,// 457 PAY 454 

    0x4446f180,// 458 PAY 455 

    0x8ac14022,// 459 PAY 456 

    0x73486841,// 460 PAY 457 

    0x81293da1,// 461 PAY 458 

    0x8e54e2fd,// 462 PAY 459 

    0xcbfdfd77,// 463 PAY 460 

    0xfb0c97f1,// 464 PAY 461 

    0x6fce7047,// 465 PAY 462 

    0x92fef02a,// 466 PAY 463 

    0x9f527108,// 467 PAY 464 

    0xcdf6ca38,// 468 PAY 465 

    0xbd16e3bd,// 469 PAY 466 

    0x9b07e073,// 470 PAY 467 

    0x8a5d7a06,// 471 PAY 468 

    0x08f486e3,// 472 PAY 469 

    0xcae69a19,// 473 PAY 470 

    0x8333749a,// 474 PAY 471 

    0xd119567d,// 475 PAY 472 

    0xdabed7b6,// 476 PAY 473 

    0xb8e6132c,// 477 PAY 474 

    0x3b49ca34,// 478 PAY 475 

    0xb149f408,// 479 PAY 476 

    0x93b453de,// 480 PAY 477 

    0x108f3bc7,// 481 PAY 478 

    0x7048aecd,// 482 PAY 479 

    0xa5d84215,// 483 PAY 480 

    0x625d343c,// 484 PAY 481 

    0x7991fb8b,// 485 PAY 482 

    0xc66ef06c,// 486 PAY 483 

    0xc44c504c,// 487 PAY 484 

    0xaddf4a3b,// 488 PAY 485 

    0xb9c41734,// 489 PAY 486 

    0x103ec807,// 490 PAY 487 

    0x18b64eb5,// 491 PAY 488 

    0x21271b37,// 492 PAY 489 

    0x785e812d,// 493 PAY 490 

    0xd2e6b654,// 494 PAY 491 

    0xedfa2960,// 495 PAY 492 

    0x1a49a250,// 496 PAY 493 

    0x3b972f4b,// 497 PAY 494 

    0xf2f32d83,// 498 PAY 495 

    0xe8b926f5,// 499 PAY 496 

    0x18f4927c,// 500 PAY 497 

    0x259850ba,// 501 PAY 498 

    0x1a0c6600,// 502 PAY 499 

/// HASH is  16 bytes 

    0xd2224ebc,// 503 HSH   1 

    0x50bb063b,// 504 HSH   2 

    0xe5418174,// 505 HSH   3 

    0x2b48a957,// 506 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 244 

/// STA pkt_idx        : 68 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x7a 

    0x01107af4 // 507 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt60_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0d 

/// ECH pdu_tag        : 0x00 

    0x000d0000,// 2 ECH   1 

/// BDA is 330 words. 

/// BDA size     is 1315 (0x523) 

/// BDA id       is 0x6d6b 

    0x05236d6b,// 3 BDA   1 

/// PAY Generic Data size   : 1315 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x936ee552,// 4 PAY   1 

    0xe357a67a,// 5 PAY   2 

    0x1a9bc172,// 6 PAY   3 

    0x434800e9,// 7 PAY   4 

    0x771cebf0,// 8 PAY   5 

    0x4681b236,// 9 PAY   6 

    0xc1b3f976,// 10 PAY   7 

    0xd7d013f2,// 11 PAY   8 

    0x251f7a10,// 12 PAY   9 

    0xdf8a5dac,// 13 PAY  10 

    0x7f5792df,// 14 PAY  11 

    0x39500ed9,// 15 PAY  12 

    0x5ce01e96,// 16 PAY  13 

    0x1db0f92b,// 17 PAY  14 

    0x7c13cfe9,// 18 PAY  15 

    0x51e51bad,// 19 PAY  16 

    0x15c4bb36,// 20 PAY  17 

    0x2fab6058,// 21 PAY  18 

    0xb178ed6f,// 22 PAY  19 

    0x2b381cf1,// 23 PAY  20 

    0x20600656,// 24 PAY  21 

    0x5f19299c,// 25 PAY  22 

    0x211359bb,// 26 PAY  23 

    0x85c0a32d,// 27 PAY  24 

    0x8899c6d1,// 28 PAY  25 

    0x620b92cb,// 29 PAY  26 

    0x1cbd340a,// 30 PAY  27 

    0x546adc7f,// 31 PAY  28 

    0x329a4865,// 32 PAY  29 

    0xf5c345bb,// 33 PAY  30 

    0xb475ecdb,// 34 PAY  31 

    0xb7c66ea0,// 35 PAY  32 

    0x0951ed51,// 36 PAY  33 

    0xabebe92b,// 37 PAY  34 

    0xa2bff095,// 38 PAY  35 

    0xd9222a84,// 39 PAY  36 

    0xa54c20b4,// 40 PAY  37 

    0xbbfd8cfb,// 41 PAY  38 

    0xb495a9c8,// 42 PAY  39 

    0x59983d14,// 43 PAY  40 

    0xabac7957,// 44 PAY  41 

    0x0164eeba,// 45 PAY  42 

    0xe1f14c1f,// 46 PAY  43 

    0x56759c4f,// 47 PAY  44 

    0x2f3073fc,// 48 PAY  45 

    0xdbea45b9,// 49 PAY  46 

    0x521a3456,// 50 PAY  47 

    0x8bc97307,// 51 PAY  48 

    0x170d012b,// 52 PAY  49 

    0x4187799d,// 53 PAY  50 

    0xf8c4f374,// 54 PAY  51 

    0x638660e2,// 55 PAY  52 

    0x390ca033,// 56 PAY  53 

    0x1e4676e5,// 57 PAY  54 

    0xc012491c,// 58 PAY  55 

    0x9ae51102,// 59 PAY  56 

    0x43124e37,// 60 PAY  57 

    0x2fab2951,// 61 PAY  58 

    0x636147a6,// 62 PAY  59 

    0x9a0bb2d8,// 63 PAY  60 

    0x028d9d3d,// 64 PAY  61 

    0xe03004e4,// 65 PAY  62 

    0x4eff9152,// 66 PAY  63 

    0xdea5f6a0,// 67 PAY  64 

    0x58c052db,// 68 PAY  65 

    0x85f73730,// 69 PAY  66 

    0x91e70aee,// 70 PAY  67 

    0x35e9b679,// 71 PAY  68 

    0x9ba375b2,// 72 PAY  69 

    0x4e31f6ea,// 73 PAY  70 

    0x66df9b6b,// 74 PAY  71 

    0x2caf273a,// 75 PAY  72 

    0x3dccdec5,// 76 PAY  73 

    0x0356f329,// 77 PAY  74 

    0x02115be5,// 78 PAY  75 

    0x559f0e14,// 79 PAY  76 

    0x093c5cb7,// 80 PAY  77 

    0x8e919e70,// 81 PAY  78 

    0x4065a5e1,// 82 PAY  79 

    0x664ec02a,// 83 PAY  80 

    0x18f67fed,// 84 PAY  81 

    0x4f460d7c,// 85 PAY  82 

    0x5cfbbdd0,// 86 PAY  83 

    0x7cbe76bc,// 87 PAY  84 

    0x01499c4c,// 88 PAY  85 

    0x8f69f184,// 89 PAY  86 

    0x6b01a865,// 90 PAY  87 

    0x64d33972,// 91 PAY  88 

    0xa3a541ca,// 92 PAY  89 

    0x9f8316ed,// 93 PAY  90 

    0x78115a31,// 94 PAY  91 

    0xbc7eb81b,// 95 PAY  92 

    0x3e1472f5,// 96 PAY  93 

    0x21e1548d,// 97 PAY  94 

    0x183a2f60,// 98 PAY  95 

    0x916a2558,// 99 PAY  96 

    0xf773c6a3,// 100 PAY  97 

    0x54c18225,// 101 PAY  98 

    0xb9df2a92,// 102 PAY  99 

    0x7ac74b09,// 103 PAY 100 

    0x3c8a9530,// 104 PAY 101 

    0xe1a09a2c,// 105 PAY 102 

    0x5b487b41,// 106 PAY 103 

    0xf86f4044,// 107 PAY 104 

    0x0d9f5b1d,// 108 PAY 105 

    0x6baa4494,// 109 PAY 106 

    0x43395dd5,// 110 PAY 107 

    0xa6a0068b,// 111 PAY 108 

    0x1dd47d66,// 112 PAY 109 

    0xbe5e5c26,// 113 PAY 110 

    0xfcd8828b,// 114 PAY 111 

    0x317986bf,// 115 PAY 112 

    0x9a0c5b24,// 116 PAY 113 

    0x226e197e,// 117 PAY 114 

    0xa66bbeb7,// 118 PAY 115 

    0x8f01ece4,// 119 PAY 116 

    0xc597c7e0,// 120 PAY 117 

    0xc67425ae,// 121 PAY 118 

    0xcb40d1c7,// 122 PAY 119 

    0x01a0aaad,// 123 PAY 120 

    0x8aa9cf53,// 124 PAY 121 

    0x765bb3ea,// 125 PAY 122 

    0xd7f775c8,// 126 PAY 123 

    0x62064aaa,// 127 PAY 124 

    0xcb161f29,// 128 PAY 125 

    0xb7bd9643,// 129 PAY 126 

    0x6e1ec725,// 130 PAY 127 

    0xcade8f37,// 131 PAY 128 

    0x2a07d33d,// 132 PAY 129 

    0x1bd8430e,// 133 PAY 130 

    0x36ca37d2,// 134 PAY 131 

    0x7b651876,// 135 PAY 132 

    0x2f6b2ba5,// 136 PAY 133 

    0xd226782a,// 137 PAY 134 

    0x269adfe8,// 138 PAY 135 

    0xfe853adf,// 139 PAY 136 

    0xaf1f9acc,// 140 PAY 137 

    0x47ddd394,// 141 PAY 138 

    0xc562e4b2,// 142 PAY 139 

    0x58dad4c6,// 143 PAY 140 

    0xbc47ef0b,// 144 PAY 141 

    0xf2133040,// 145 PAY 142 

    0xefcb526a,// 146 PAY 143 

    0x0c1c9e40,// 147 PAY 144 

    0x28a955a2,// 148 PAY 145 

    0x11b2342b,// 149 PAY 146 

    0x4cfbbd90,// 150 PAY 147 

    0xb7a51989,// 151 PAY 148 

    0xa80d1276,// 152 PAY 149 

    0x5e2b4930,// 153 PAY 150 

    0x0d713c66,// 154 PAY 151 

    0x3439d1a0,// 155 PAY 152 

    0xf4e67dad,// 156 PAY 153 

    0x86c10f57,// 157 PAY 154 

    0xbddc4526,// 158 PAY 155 

    0x946cf6eb,// 159 PAY 156 

    0x23201ed0,// 160 PAY 157 

    0x040ee2cc,// 161 PAY 158 

    0x0e81ef04,// 162 PAY 159 

    0x7d57a533,// 163 PAY 160 

    0x6663a88c,// 164 PAY 161 

    0xdcc7feef,// 165 PAY 162 

    0xeefb18d7,// 166 PAY 163 

    0x9f37413e,// 167 PAY 164 

    0x0467d3dc,// 168 PAY 165 

    0xceb00cf0,// 169 PAY 166 

    0xbb588efe,// 170 PAY 167 

    0x28b5a768,// 171 PAY 168 

    0x29efb81e,// 172 PAY 169 

    0x5aeb80f7,// 173 PAY 170 

    0x39dddf6c,// 174 PAY 171 

    0xabdb76f1,// 175 PAY 172 

    0x3eea9972,// 176 PAY 173 

    0xf55e7805,// 177 PAY 174 

    0x9dc7a6ef,// 178 PAY 175 

    0x432c35b1,// 179 PAY 176 

    0x75c67209,// 180 PAY 177 

    0xe4360a5f,// 181 PAY 178 

    0x47cc1b61,// 182 PAY 179 

    0x1666fac6,// 183 PAY 180 

    0x05569415,// 184 PAY 181 

    0xba1d21ed,// 185 PAY 182 

    0xeefcc37d,// 186 PAY 183 

    0x389f7d39,// 187 PAY 184 

    0x19e234f4,// 188 PAY 185 

    0xa962f36a,// 189 PAY 186 

    0x445ee7a0,// 190 PAY 187 

    0x39a10da9,// 191 PAY 188 

    0x8e23f4e7,// 192 PAY 189 

    0xcfe7e824,// 193 PAY 190 

    0x55c6e2f2,// 194 PAY 191 

    0x423e8ec1,// 195 PAY 192 

    0x7bbf6e74,// 196 PAY 193 

    0xb7ed6b8c,// 197 PAY 194 

    0xe1cbe51d,// 198 PAY 195 

    0x276b9583,// 199 PAY 196 

    0x0597543b,// 200 PAY 197 

    0xea12b5fd,// 201 PAY 198 

    0xd8c92453,// 202 PAY 199 

    0x7e2a26c3,// 203 PAY 200 

    0x43612f0d,// 204 PAY 201 

    0x632e7c42,// 205 PAY 202 

    0x4e9a2aad,// 206 PAY 203 

    0x6c552f48,// 207 PAY 204 

    0x797e5193,// 208 PAY 205 

    0xcf7f2ba0,// 209 PAY 206 

    0x5977e86f,// 210 PAY 207 

    0x376e8876,// 211 PAY 208 

    0xe09e46af,// 212 PAY 209 

    0x98c4fb5e,// 213 PAY 210 

    0x0354d110,// 214 PAY 211 

    0xe1c65b26,// 215 PAY 212 

    0xe713a1c8,// 216 PAY 213 

    0x1203f52e,// 217 PAY 214 

    0xe4b1ae2c,// 218 PAY 215 

    0x4e711008,// 219 PAY 216 

    0xc50c828f,// 220 PAY 217 

    0xb30aa4cd,// 221 PAY 218 

    0xdb965325,// 222 PAY 219 

    0xcaa15d82,// 223 PAY 220 

    0xab293cf3,// 224 PAY 221 

    0x6f3ea023,// 225 PAY 222 

    0x211fa230,// 226 PAY 223 

    0x42d4017b,// 227 PAY 224 

    0x7914947a,// 228 PAY 225 

    0xccd9a853,// 229 PAY 226 

    0xea24bd2c,// 230 PAY 227 

    0x63bac64c,// 231 PAY 228 

    0x5b0e62a0,// 232 PAY 229 

    0xdf6f4015,// 233 PAY 230 

    0xc7e5fc65,// 234 PAY 231 

    0x9c5c6b2b,// 235 PAY 232 

    0xafa06f06,// 236 PAY 233 

    0xd8442a8a,// 237 PAY 234 

    0xc77a331c,// 238 PAY 235 

    0xaf788f29,// 239 PAY 236 

    0xbfd3b02e,// 240 PAY 237 

    0xf0a190ec,// 241 PAY 238 

    0xcd1a8a1c,// 242 PAY 239 

    0x8af9c1dd,// 243 PAY 240 

    0x7bcd5728,// 244 PAY 241 

    0x917edd2a,// 245 PAY 242 

    0xd38c90d6,// 246 PAY 243 

    0x07c04399,// 247 PAY 244 

    0x326c50dd,// 248 PAY 245 

    0xc7efe64a,// 249 PAY 246 

    0xf2322d67,// 250 PAY 247 

    0x7a82ba58,// 251 PAY 248 

    0x9eedefb7,// 252 PAY 249 

    0x3abb3b7b,// 253 PAY 250 

    0xb95ffa8a,// 254 PAY 251 

    0xb1bd4b5b,// 255 PAY 252 

    0xd25eefaf,// 256 PAY 253 

    0x384d149d,// 257 PAY 254 

    0x9443dfb6,// 258 PAY 255 

    0xa7b11124,// 259 PAY 256 

    0xdcf8c999,// 260 PAY 257 

    0x4ad3fe4f,// 261 PAY 258 

    0xdcef7e39,// 262 PAY 259 

    0xe7b392f9,// 263 PAY 260 

    0xa509cfd6,// 264 PAY 261 

    0x47f1ff67,// 265 PAY 262 

    0x96bd2c08,// 266 PAY 263 

    0x53cdd406,// 267 PAY 264 

    0xacf5e339,// 268 PAY 265 

    0x65050bc0,// 269 PAY 266 

    0xccb065fc,// 270 PAY 267 

    0x5a6715dc,// 271 PAY 268 

    0xb9bd853a,// 272 PAY 269 

    0x3a355a6b,// 273 PAY 270 

    0x73aac9d4,// 274 PAY 271 

    0xa20c86e3,// 275 PAY 272 

    0x51c63c76,// 276 PAY 273 

    0x89acffcc,// 277 PAY 274 

    0x2ab1c201,// 278 PAY 275 

    0x46bf3dc3,// 279 PAY 276 

    0xb3438797,// 280 PAY 277 

    0xa4254ede,// 281 PAY 278 

    0xed97f4c2,// 282 PAY 279 

    0x0c9f43a1,// 283 PAY 280 

    0x7154d291,// 284 PAY 281 

    0x2fe102ac,// 285 PAY 282 

    0x97d1a227,// 286 PAY 283 

    0x0511cb6f,// 287 PAY 284 

    0x63a0d0eb,// 288 PAY 285 

    0xc8ac04dd,// 289 PAY 286 

    0x2ee351c0,// 290 PAY 287 

    0x69ed2da9,// 291 PAY 288 

    0xb9803e06,// 292 PAY 289 

    0x8fb93d7f,// 293 PAY 290 

    0xf58a9f4d,// 294 PAY 291 

    0x5ca9110c,// 295 PAY 292 

    0xa208cb63,// 296 PAY 293 

    0x4bea88cf,// 297 PAY 294 

    0x0dfdc6d7,// 298 PAY 295 

    0x12c82576,// 299 PAY 296 

    0xd0ddaa9a,// 300 PAY 297 

    0xb55f02df,// 301 PAY 298 

    0xfebf99c2,// 302 PAY 299 

    0x8d730ee4,// 303 PAY 300 

    0x36979c65,// 304 PAY 301 

    0xe1c11265,// 305 PAY 302 

    0x77278512,// 306 PAY 303 

    0x2c6d8c5d,// 307 PAY 304 

    0x79352373,// 308 PAY 305 

    0x3a1b4dd6,// 309 PAY 306 

    0x176b84e8,// 310 PAY 307 

    0x6b0335fa,// 311 PAY 308 

    0xd14dde36,// 312 PAY 309 

    0x777a3f62,// 313 PAY 310 

    0x7749762f,// 314 PAY 311 

    0x6d3b34ac,// 315 PAY 312 

    0xbd237d01,// 316 PAY 313 

    0xd6b76675,// 317 PAY 314 

    0xe1ba9e00,// 318 PAY 315 

    0x23925064,// 319 PAY 316 

    0xb2db7132,// 320 PAY 317 

    0xab41647a,// 321 PAY 318 

    0x76c7cb96,// 322 PAY 319 

    0x77274f00,// 323 PAY 320 

    0x91aa00aa,// 324 PAY 321 

    0x2e8dbdf8,// 325 PAY 322 

    0x2f215715,// 326 PAY 323 

    0xbda06aa3,// 327 PAY 324 

    0xb535c375,// 328 PAY 325 

    0x2a4ce145,// 329 PAY 326 

    0x38a5d8dd,// 330 PAY 327 

    0x4e1f7385,// 331 PAY 328 

    0xc82fd000,// 332 PAY 329 

/// HASH is  12 bytes 

    0x76c7cb96,// 333 HSH   1 

    0x77274f00,// 334 HSH   2 

    0x91aa00aa,// 335 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 18 

/// STA pkt_idx        : 20 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xda 

    0x0051da12 // 336 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt61_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 504 words. 

/// BDA size     is 2009 (0x7d9) 

/// BDA id       is 0xb58b 

    0x07d9b58b,// 3 BDA   1 

/// PAY Generic Data size   : 2009 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0xa865d045,// 4 PAY   1 

    0xddf5a65f,// 5 PAY   2 

    0x3cbe262d,// 6 PAY   3 

    0x47d674e6,// 7 PAY   4 

    0xb16b3d8b,// 8 PAY   5 

    0xe3306678,// 9 PAY   6 

    0x8f4636a8,// 10 PAY   7 

    0x0851b840,// 11 PAY   8 

    0xc7d263dc,// 12 PAY   9 

    0x2db72976,// 13 PAY  10 

    0x5269f198,// 14 PAY  11 

    0xb6e39aec,// 15 PAY  12 

    0xcdd1de51,// 16 PAY  13 

    0x760d3622,// 17 PAY  14 

    0x210e8adf,// 18 PAY  15 

    0x8c24a92f,// 19 PAY  16 

    0x28d4fa1b,// 20 PAY  17 

    0x6cb558c8,// 21 PAY  18 

    0x48f11cb0,// 22 PAY  19 

    0xfbef1971,// 23 PAY  20 

    0x5ad0df19,// 24 PAY  21 

    0xba9e9943,// 25 PAY  22 

    0x5ba109c4,// 26 PAY  23 

    0x06e7bf8d,// 27 PAY  24 

    0x606ceafb,// 28 PAY  25 

    0x3074dac0,// 29 PAY  26 

    0xe1aaf266,// 30 PAY  27 

    0x8a9cd25f,// 31 PAY  28 

    0x4298124d,// 32 PAY  29 

    0x7a6f1e1c,// 33 PAY  30 

    0x98f45a2c,// 34 PAY  31 

    0xce5fd3b3,// 35 PAY  32 

    0x19f5b8c5,// 36 PAY  33 

    0xa1aeff50,// 37 PAY  34 

    0x9690b9de,// 38 PAY  35 

    0x17998339,// 39 PAY  36 

    0x20680ba8,// 40 PAY  37 

    0x945cce47,// 41 PAY  38 

    0xaa8fb936,// 42 PAY  39 

    0x691774fc,// 43 PAY  40 

    0x93d479a4,// 44 PAY  41 

    0xdbc0774e,// 45 PAY  42 

    0x395a017d,// 46 PAY  43 

    0x8734bed7,// 47 PAY  44 

    0x56441aaa,// 48 PAY  45 

    0x54ac1e24,// 49 PAY  46 

    0xcb5e4df1,// 50 PAY  47 

    0xc68c10a1,// 51 PAY  48 

    0x198d72f0,// 52 PAY  49 

    0x06a015aa,// 53 PAY  50 

    0x3dd3d0bd,// 54 PAY  51 

    0x5979f46d,// 55 PAY  52 

    0x30416269,// 56 PAY  53 

    0xe283e094,// 57 PAY  54 

    0xf57c4827,// 58 PAY  55 

    0x7a8a19ee,// 59 PAY  56 

    0x5581538f,// 60 PAY  57 

    0xcbb7d288,// 61 PAY  58 

    0x099a366d,// 62 PAY  59 

    0x47157acb,// 63 PAY  60 

    0x15cd715a,// 64 PAY  61 

    0xe93a45ea,// 65 PAY  62 

    0x012dfdbf,// 66 PAY  63 

    0x16031d0e,// 67 PAY  64 

    0x3b1bbfeb,// 68 PAY  65 

    0x38b04f39,// 69 PAY  66 

    0xecbddb33,// 70 PAY  67 

    0xb1efc7c7,// 71 PAY  68 

    0x8dff1b5a,// 72 PAY  69 

    0xb9b5ec7d,// 73 PAY  70 

    0xcc89ba74,// 74 PAY  71 

    0x56c9102d,// 75 PAY  72 

    0x5b279b41,// 76 PAY  73 

    0xaff1202e,// 77 PAY  74 

    0x5e524943,// 78 PAY  75 

    0x68fde98c,// 79 PAY  76 

    0x3d65c3c6,// 80 PAY  77 

    0x711a14dd,// 81 PAY  78 

    0x4f88dd8d,// 82 PAY  79 

    0xd3927e92,// 83 PAY  80 

    0x421c3f03,// 84 PAY  81 

    0x8c566b5d,// 85 PAY  82 

    0x1a1fb76d,// 86 PAY  83 

    0xdc404538,// 87 PAY  84 

    0x4612627a,// 88 PAY  85 

    0xb9f94fb4,// 89 PAY  86 

    0x003ab51e,// 90 PAY  87 

    0x35803e2b,// 91 PAY  88 

    0xd124e050,// 92 PAY  89 

    0x3eb66740,// 93 PAY  90 

    0xc454d369,// 94 PAY  91 

    0x8fc5fbf6,// 95 PAY  92 

    0xef5726b4,// 96 PAY  93 

    0xb31e5b90,// 97 PAY  94 

    0x5596ef98,// 98 PAY  95 

    0x319123fa,// 99 PAY  96 

    0xd8ab2e10,// 100 PAY  97 

    0x5898b7ed,// 101 PAY  98 

    0xa103f881,// 102 PAY  99 

    0x84953b5b,// 103 PAY 100 

    0x30594551,// 104 PAY 101 

    0x7e6c46af,// 105 PAY 102 

    0xe29b3be1,// 106 PAY 103 

    0x2720f78d,// 107 PAY 104 

    0x0c7a15a3,// 108 PAY 105 

    0x96c7af92,// 109 PAY 106 

    0x8a02e309,// 110 PAY 107 

    0xf215a8c7,// 111 PAY 108 

    0xbcf2164f,// 112 PAY 109 

    0x7811aa17,// 113 PAY 110 

    0x8344677b,// 114 PAY 111 

    0x20763692,// 115 PAY 112 

    0x2ad5ab97,// 116 PAY 113 

    0x4d7ab61a,// 117 PAY 114 

    0xe0739ecf,// 118 PAY 115 

    0x186b5d35,// 119 PAY 116 

    0x39a9daa4,// 120 PAY 117 

    0x668b9ee0,// 121 PAY 118 

    0x41614265,// 122 PAY 119 

    0x173df684,// 123 PAY 120 

    0xaf780208,// 124 PAY 121 

    0xe54d8cfc,// 125 PAY 122 

    0x68097720,// 126 PAY 123 

    0x31b82bc7,// 127 PAY 124 

    0x50d0c21f,// 128 PAY 125 

    0x603cab24,// 129 PAY 126 

    0x29aa19a7,// 130 PAY 127 

    0xfa2134d2,// 131 PAY 128 

    0xa9a8120c,// 132 PAY 129 

    0xbd0cdd1c,// 133 PAY 130 

    0x5070387b,// 134 PAY 131 

    0x03628bf0,// 135 PAY 132 

    0xc22c99bd,// 136 PAY 133 

    0xa6093f66,// 137 PAY 134 

    0x5eb2d9c5,// 138 PAY 135 

    0x8f0d33a0,// 139 PAY 136 

    0x45ca1a99,// 140 PAY 137 

    0xd6e620d1,// 141 PAY 138 

    0x8847cef0,// 142 PAY 139 

    0xd38c14f6,// 143 PAY 140 

    0xe1a30dd4,// 144 PAY 141 

    0x185e2a18,// 145 PAY 142 

    0xf428e121,// 146 PAY 143 

    0x07043011,// 147 PAY 144 

    0xf85afd5e,// 148 PAY 145 

    0xbac0fcc7,// 149 PAY 146 

    0xba878d02,// 150 PAY 147 

    0xce032821,// 151 PAY 148 

    0x3f434612,// 152 PAY 149 

    0x2b5570e6,// 153 PAY 150 

    0x010a577b,// 154 PAY 151 

    0x346e2537,// 155 PAY 152 

    0xe1521947,// 156 PAY 153 

    0x4045d012,// 157 PAY 154 

    0xa79319c6,// 158 PAY 155 

    0xc6e811cc,// 159 PAY 156 

    0x06935292,// 160 PAY 157 

    0x78162765,// 161 PAY 158 

    0x8c7fd10c,// 162 PAY 159 

    0x54f60cb8,// 163 PAY 160 

    0xd66247f5,// 164 PAY 161 

    0xd10f9f42,// 165 PAY 162 

    0xbbf40f1c,// 166 PAY 163 

    0x70b5bd47,// 167 PAY 164 

    0xcd96aaae,// 168 PAY 165 

    0xcfb0e235,// 169 PAY 166 

    0xb1b6f8f7,// 170 PAY 167 

    0x2729334d,// 171 PAY 168 

    0x447ff4ba,// 172 PAY 169 

    0x47629c22,// 173 PAY 170 

    0x3490c0f6,// 174 PAY 171 

    0xb057684e,// 175 PAY 172 

    0x9fa973df,// 176 PAY 173 

    0x095662c4,// 177 PAY 174 

    0x0a03320d,// 178 PAY 175 

    0x453e0f5c,// 179 PAY 176 

    0xe6659cf5,// 180 PAY 177 

    0x30700ab6,// 181 PAY 178 

    0xa0d26e86,// 182 PAY 179 

    0xd0eb6439,// 183 PAY 180 

    0x9bc998f8,// 184 PAY 181 

    0x215d5713,// 185 PAY 182 

    0x4e80992b,// 186 PAY 183 

    0xbe33c467,// 187 PAY 184 

    0x122f173a,// 188 PAY 185 

    0x2382d77a,// 189 PAY 186 

    0x0c233c19,// 190 PAY 187 

    0xf5a87d0f,// 191 PAY 188 

    0xe6fcc54a,// 192 PAY 189 

    0xced94d6e,// 193 PAY 190 

    0xc3c41e1d,// 194 PAY 191 

    0x1fbdf136,// 195 PAY 192 

    0xf7b12969,// 196 PAY 193 

    0x499e3dfc,// 197 PAY 194 

    0xea2ba04f,// 198 PAY 195 

    0x94f478da,// 199 PAY 196 

    0x3853e661,// 200 PAY 197 

    0x0c2c074e,// 201 PAY 198 

    0x98c9ca6a,// 202 PAY 199 

    0x384242dc,// 203 PAY 200 

    0x99fa057b,// 204 PAY 201 

    0x86cdae9c,// 205 PAY 202 

    0x84ef761e,// 206 PAY 203 

    0xd5f5ec6c,// 207 PAY 204 

    0xbd361d05,// 208 PAY 205 

    0x7f0308ac,// 209 PAY 206 

    0x5aee4be0,// 210 PAY 207 

    0x8c2528d5,// 211 PAY 208 

    0xf877d332,// 212 PAY 209 

    0x7d47768b,// 213 PAY 210 

    0xddaa1507,// 214 PAY 211 

    0xea368309,// 215 PAY 212 

    0xcef9d38c,// 216 PAY 213 

    0xc3ff7efb,// 217 PAY 214 

    0x12f09ab1,// 218 PAY 215 

    0x8b3746cc,// 219 PAY 216 

    0xd74698fb,// 220 PAY 217 

    0xbec90362,// 221 PAY 218 

    0x1eceb30c,// 222 PAY 219 

    0x4b8803c6,// 223 PAY 220 

    0xd6b38df4,// 224 PAY 221 

    0xcc490511,// 225 PAY 222 

    0x38fc8813,// 226 PAY 223 

    0x1e7e046e,// 227 PAY 224 

    0xe948b6dc,// 228 PAY 225 

    0x9413e89f,// 229 PAY 226 

    0x73ca904f,// 230 PAY 227 

    0x353f6319,// 231 PAY 228 

    0x567da961,// 232 PAY 229 

    0xc856b2a1,// 233 PAY 230 

    0x979a1b58,// 234 PAY 231 

    0x304d47f7,// 235 PAY 232 

    0xc17ff4d4,// 236 PAY 233 

    0xe62b4043,// 237 PAY 234 

    0x90024511,// 238 PAY 235 

    0xea62332a,// 239 PAY 236 

    0x55fb6b02,// 240 PAY 237 

    0x10de2909,// 241 PAY 238 

    0x0de7ab74,// 242 PAY 239 

    0x7626a6e1,// 243 PAY 240 

    0xf1e82a0e,// 244 PAY 241 

    0xa840e22c,// 245 PAY 242 

    0x4c1351ed,// 246 PAY 243 

    0x1f1a0400,// 247 PAY 244 

    0x9b3635d1,// 248 PAY 245 

    0xf277509a,// 249 PAY 246 

    0x00d5d902,// 250 PAY 247 

    0xe2422e3f,// 251 PAY 248 

    0xe150d163,// 252 PAY 249 

    0xf2edd7fe,// 253 PAY 250 

    0x54afe86b,// 254 PAY 251 

    0x901027fe,// 255 PAY 252 

    0x637c228a,// 256 PAY 253 

    0x2847d78a,// 257 PAY 254 

    0x19a759c9,// 258 PAY 255 

    0x980b03b7,// 259 PAY 256 

    0x964de64d,// 260 PAY 257 

    0x766fb150,// 261 PAY 258 

    0xb99e6145,// 262 PAY 259 

    0x3874a607,// 263 PAY 260 

    0x656fb132,// 264 PAY 261 

    0x898b42e8,// 265 PAY 262 

    0x48060dbb,// 266 PAY 263 

    0xfaecc7f1,// 267 PAY 264 

    0xfa5d8639,// 268 PAY 265 

    0x9c99e3d8,// 269 PAY 266 

    0x6aaa81cf,// 270 PAY 267 

    0x7229c99d,// 271 PAY 268 

    0xec40b14e,// 272 PAY 269 

    0xc8267c12,// 273 PAY 270 

    0x5e4b9064,// 274 PAY 271 

    0x5a8869d1,// 275 PAY 272 

    0x4eb79d30,// 276 PAY 273 

    0x6a99b91f,// 277 PAY 274 

    0x7aee2055,// 278 PAY 275 

    0xbb6bd127,// 279 PAY 276 

    0x21bdacc2,// 280 PAY 277 

    0x11027a54,// 281 PAY 278 

    0x09a660b5,// 282 PAY 279 

    0x403db17f,// 283 PAY 280 

    0xd639032d,// 284 PAY 281 

    0xa145100c,// 285 PAY 282 

    0x5f5e859a,// 286 PAY 283 

    0x84413f2b,// 287 PAY 284 

    0xe19c8432,// 288 PAY 285 

    0xcbc8d8c2,// 289 PAY 286 

    0xcac57688,// 290 PAY 287 

    0xce2836fa,// 291 PAY 288 

    0xcf8d9641,// 292 PAY 289 

    0xdb8d3576,// 293 PAY 290 

    0xdbceeeae,// 294 PAY 291 

    0x3c4f4cf8,// 295 PAY 292 

    0xbca1b644,// 296 PAY 293 

    0x0400d30f,// 297 PAY 294 

    0x13948041,// 298 PAY 295 

    0x6fff0595,// 299 PAY 296 

    0x44918fcf,// 300 PAY 297 

    0x4ad08bdc,// 301 PAY 298 

    0xe65f82d6,// 302 PAY 299 

    0xa16c8d1d,// 303 PAY 300 

    0x70b17ff2,// 304 PAY 301 

    0xca7161ed,// 305 PAY 302 

    0x566f61ea,// 306 PAY 303 

    0x316f3dc9,// 307 PAY 304 

    0x412477a9,// 308 PAY 305 

    0xfb7fa91e,// 309 PAY 306 

    0x251d490e,// 310 PAY 307 

    0x085400aa,// 311 PAY 308 

    0xc94bd193,// 312 PAY 309 

    0x4c7670dc,// 313 PAY 310 

    0x64ac6ec4,// 314 PAY 311 

    0x9e3d0cbf,// 315 PAY 312 

    0x2b7b7a65,// 316 PAY 313 

    0x06af8874,// 317 PAY 314 

    0xbc241c9b,// 318 PAY 315 

    0xabf2017f,// 319 PAY 316 

    0x34dcc70a,// 320 PAY 317 

    0xc28d5eae,// 321 PAY 318 

    0x686405ee,// 322 PAY 319 

    0x5a657ff5,// 323 PAY 320 

    0x3cf28b30,// 324 PAY 321 

    0x81834512,// 325 PAY 322 

    0x2251ca99,// 326 PAY 323 

    0x695687bb,// 327 PAY 324 

    0xb6c5d475,// 328 PAY 325 

    0x0166134d,// 329 PAY 326 

    0x4caabe55,// 330 PAY 327 

    0x861def67,// 331 PAY 328 

    0xb0131d74,// 332 PAY 329 

    0xfd5b7f76,// 333 PAY 330 

    0x9afcee9b,// 334 PAY 331 

    0x6be906ed,// 335 PAY 332 

    0x95a0079c,// 336 PAY 333 

    0x8aec831d,// 337 PAY 334 

    0xb9c045b2,// 338 PAY 335 

    0xac0f8808,// 339 PAY 336 

    0xb07060ba,// 340 PAY 337 

    0x8921edba,// 341 PAY 338 

    0xa98d4045,// 342 PAY 339 

    0x852b3cb4,// 343 PAY 340 

    0x58339e74,// 344 PAY 341 

    0x02a89b0f,// 345 PAY 342 

    0xc8b7b995,// 346 PAY 343 

    0x63cf226f,// 347 PAY 344 

    0x9515aa63,// 348 PAY 345 

    0x47ab2248,// 349 PAY 346 

    0x597a811a,// 350 PAY 347 

    0xfe32fac9,// 351 PAY 348 

    0x36d896ef,// 352 PAY 349 

    0xf0e7eb72,// 353 PAY 350 

    0xf0112998,// 354 PAY 351 

    0x71c12e2b,// 355 PAY 352 

    0xa509b180,// 356 PAY 353 

    0x2a2e8cf5,// 357 PAY 354 

    0x242930e9,// 358 PAY 355 

    0x0fe0fb18,// 359 PAY 356 

    0x59c032d4,// 360 PAY 357 

    0x3689a8df,// 361 PAY 358 

    0xa8e53b80,// 362 PAY 359 

    0x0c125be5,// 363 PAY 360 

    0x667342b7,// 364 PAY 361 

    0x99418ab9,// 365 PAY 362 

    0x957a6b9a,// 366 PAY 363 

    0x72b6246f,// 367 PAY 364 

    0x666beaba,// 368 PAY 365 

    0x65e8e2c1,// 369 PAY 366 

    0x8acdbaf7,// 370 PAY 367 

    0x82a9ec37,// 371 PAY 368 

    0x5954c884,// 372 PAY 369 

    0xbfd531f1,// 373 PAY 370 

    0x8ef6e9da,// 374 PAY 371 

    0x4094917a,// 375 PAY 372 

    0xe3772966,// 376 PAY 373 

    0xbc938c5b,// 377 PAY 374 

    0x6093edc3,// 378 PAY 375 

    0x3e7bf896,// 379 PAY 376 

    0xade1baa0,// 380 PAY 377 

    0xf80b5443,// 381 PAY 378 

    0x2d722f85,// 382 PAY 379 

    0x6150af69,// 383 PAY 380 

    0x1bae3d5c,// 384 PAY 381 

    0x8a30271a,// 385 PAY 382 

    0x2160c779,// 386 PAY 383 

    0x629c4a26,// 387 PAY 384 

    0x30b4ee59,// 388 PAY 385 

    0xea580227,// 389 PAY 386 

    0x77741f6d,// 390 PAY 387 

    0x40c1ebe7,// 391 PAY 388 

    0x4e2e8da2,// 392 PAY 389 

    0x6d3c5213,// 393 PAY 390 

    0x4ffca153,// 394 PAY 391 

    0xf28cfe2d,// 395 PAY 392 

    0xa4883f3b,// 396 PAY 393 

    0x04f30de9,// 397 PAY 394 

    0x795da3e7,// 398 PAY 395 

    0x4ece40f2,// 399 PAY 396 

    0xf84320b0,// 400 PAY 397 

    0x94f228ea,// 401 PAY 398 

    0x0d2f177a,// 402 PAY 399 

    0x0220121b,// 403 PAY 400 

    0xbb5a5e6d,// 404 PAY 401 

    0x4220ab77,// 405 PAY 402 

    0xdef4f11c,// 406 PAY 403 

    0xb07d47c6,// 407 PAY 404 

    0x96b580fc,// 408 PAY 405 

    0x430b0862,// 409 PAY 406 

    0xe3ca6fa2,// 410 PAY 407 

    0x097045fe,// 411 PAY 408 

    0xb2a760fb,// 412 PAY 409 

    0xb756befa,// 413 PAY 410 

    0xe338970f,// 414 PAY 411 

    0xaab7c957,// 415 PAY 412 

    0x1621370d,// 416 PAY 413 

    0x153e8090,// 417 PAY 414 

    0x800f3839,// 418 PAY 415 

    0x0d96b335,// 419 PAY 416 

    0x04a72116,// 420 PAY 417 

    0xa314db72,// 421 PAY 418 

    0xee30c11e,// 422 PAY 419 

    0xfa258e83,// 423 PAY 420 

    0xf2866dfa,// 424 PAY 421 

    0x87565cc4,// 425 PAY 422 

    0x87c1c486,// 426 PAY 423 

    0x60ad5a4e,// 427 PAY 424 

    0x7142bc8c,// 428 PAY 425 

    0xce65535f,// 429 PAY 426 

    0xe5c2d5b9,// 430 PAY 427 

    0xfcf36221,// 431 PAY 428 

    0x6b21acb3,// 432 PAY 429 

    0x7b5dcaf4,// 433 PAY 430 

    0xe5899ee5,// 434 PAY 431 

    0xdf0611a5,// 435 PAY 432 

    0x72a5b034,// 436 PAY 433 

    0xdae4bff8,// 437 PAY 434 

    0x58201fb6,// 438 PAY 435 

    0x8a6c6c71,// 439 PAY 436 

    0x969a27a7,// 440 PAY 437 

    0x88b18156,// 441 PAY 438 

    0x113af07c,// 442 PAY 439 

    0xf10abd43,// 443 PAY 440 

    0xe7a02269,// 444 PAY 441 

    0xf17c4b6d,// 445 PAY 442 

    0xb384b382,// 446 PAY 443 

    0xd10f89fd,// 447 PAY 444 

    0x8be6f62d,// 448 PAY 445 

    0xe789d64f,// 449 PAY 446 

    0x36b83df0,// 450 PAY 447 

    0x89fa2e65,// 451 PAY 448 

    0xd8de8dbc,// 452 PAY 449 

    0x9b7456ff,// 453 PAY 450 

    0x30a335e4,// 454 PAY 451 

    0x298cbe2b,// 455 PAY 452 

    0x20e12df7,// 456 PAY 453 

    0x7c5f87f3,// 457 PAY 454 

    0x71078450,// 458 PAY 455 

    0x0ff1e79c,// 459 PAY 456 

    0xb00780e8,// 460 PAY 457 

    0xd9034938,// 461 PAY 458 

    0x233271a6,// 462 PAY 459 

    0x9b83b727,// 463 PAY 460 

    0x39cf11c9,// 464 PAY 461 

    0xa761a2b8,// 465 PAY 462 

    0x0a4b439d,// 466 PAY 463 

    0x4e32f9e4,// 467 PAY 464 

    0x4bfe6c40,// 468 PAY 465 

    0xa1b7168b,// 469 PAY 466 

    0xe7196593,// 470 PAY 467 

    0x97913ee7,// 471 PAY 468 

    0x31d9ccd4,// 472 PAY 469 

    0x018388c9,// 473 PAY 470 

    0xf2bfd74f,// 474 PAY 471 

    0x57b51efb,// 475 PAY 472 

    0x1b9118f8,// 476 PAY 473 

    0x22ec6622,// 477 PAY 474 

    0x0db7689a,// 478 PAY 475 

    0x5bb51a21,// 479 PAY 476 

    0xf38cdfcf,// 480 PAY 477 

    0xb792921f,// 481 PAY 478 

    0x0f59ab18,// 482 PAY 479 

    0xf4af23a8,// 483 PAY 480 

    0xdc4db33a,// 484 PAY 481 

    0x1fdd22b0,// 485 PAY 482 

    0x24c81b75,// 486 PAY 483 

    0x0a171e10,// 487 PAY 484 

    0xfe5789ad,// 488 PAY 485 

    0x824b9161,// 489 PAY 486 

    0x191ecc45,// 490 PAY 487 

    0x37f07a36,// 491 PAY 488 

    0xd2698dfe,// 492 PAY 489 

    0x55d96792,// 493 PAY 490 

    0x41ec9577,// 494 PAY 491 

    0x1c2557f6,// 495 PAY 492 

    0x29d1643b,// 496 PAY 493 

    0x47526527,// 497 PAY 494 

    0x69f0dff5,// 498 PAY 495 

    0xc9855173,// 499 PAY 496 

    0xc9f55343,// 500 PAY 497 

    0xdee9e4f0,// 501 PAY 498 

    0x42e1e7e9,// 502 PAY 499 

    0x5fc30d7d,// 503 PAY 500 

    0xdc6470d3,// 504 PAY 501 

    0x255d7534,// 505 PAY 502 

    0x8f000000,// 506 PAY 503 

/// STA is 1 words. 

/// STA num_pkts       : 178 

/// STA pkt_idx        : 214 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x6e 

    0x03586eb2 // 507 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt62_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 212 words. 

/// BDA size     is 844 (0x34c) 

/// BDA id       is 0x8eca 

    0x034c8eca,// 3 BDA   1 

/// PAY Generic Data size   : 844 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x9c96ac78,// 4 PAY   1 

    0x49e45026,// 5 PAY   2 

    0xa5f65bd7,// 6 PAY   3 

    0x2f2e67a4,// 7 PAY   4 

    0x42a8e45b,// 8 PAY   5 

    0xcb45c358,// 9 PAY   6 

    0x803fbecd,// 10 PAY   7 

    0x29184ec0,// 11 PAY   8 

    0x08a7c3a6,// 12 PAY   9 

    0x1394c070,// 13 PAY  10 

    0x27d65ff9,// 14 PAY  11 

    0xbaec5444,// 15 PAY  12 

    0xb7b8c44b,// 16 PAY  13 

    0x144deeee,// 17 PAY  14 

    0xc9abad1a,// 18 PAY  15 

    0xddc80c08,// 19 PAY  16 

    0x558e482a,// 20 PAY  17 

    0x4c95b9d4,// 21 PAY  18 

    0x9955fee2,// 22 PAY  19 

    0x211b96f7,// 23 PAY  20 

    0xe716a30c,// 24 PAY  21 

    0x406dd45c,// 25 PAY  22 

    0x71fecadb,// 26 PAY  23 

    0x8f096a6a,// 27 PAY  24 

    0xecf37519,// 28 PAY  25 

    0xbede35db,// 29 PAY  26 

    0x1f2db32b,// 30 PAY  27 

    0xea1ce155,// 31 PAY  28 

    0x336d50f1,// 32 PAY  29 

    0x88feeb50,// 33 PAY  30 

    0x89d1c1f6,// 34 PAY  31 

    0x3d6c351a,// 35 PAY  32 

    0x98a999d3,// 36 PAY  33 

    0x1461c6bb,// 37 PAY  34 

    0x7582ceda,// 38 PAY  35 

    0xea63cdb7,// 39 PAY  36 

    0xad95f363,// 40 PAY  37 

    0x41aaf41e,// 41 PAY  38 

    0xc9bfd8df,// 42 PAY  39 

    0xc88bc6e8,// 43 PAY  40 

    0xcee7dfba,// 44 PAY  41 

    0x769d6cbd,// 45 PAY  42 

    0xea5c5b5b,// 46 PAY  43 

    0x72fec43f,// 47 PAY  44 

    0x518a8858,// 48 PAY  45 

    0xda4331ca,// 49 PAY  46 

    0xfc03ae64,// 50 PAY  47 

    0xdaa7a5e9,// 51 PAY  48 

    0xd1d9ba69,// 52 PAY  49 

    0x9f4f305d,// 53 PAY  50 

    0xf8f6927f,// 54 PAY  51 

    0x1e99f2a3,// 55 PAY  52 

    0x8eca576b,// 56 PAY  53 

    0xfaaa1d81,// 57 PAY  54 

    0xca550323,// 58 PAY  55 

    0xfdfb31f0,// 59 PAY  56 

    0xeb5441f7,// 60 PAY  57 

    0x60ae6fa7,// 61 PAY  58 

    0x31b28529,// 62 PAY  59 

    0xbb1e4716,// 63 PAY  60 

    0x0023828f,// 64 PAY  61 

    0x48de3058,// 65 PAY  62 

    0xd39c0a19,// 66 PAY  63 

    0xc99e1545,// 67 PAY  64 

    0xbfccc9cb,// 68 PAY  65 

    0xb73df53e,// 69 PAY  66 

    0xeaae28dd,// 70 PAY  67 

    0xe8ccf430,// 71 PAY  68 

    0xe1f2c6db,// 72 PAY  69 

    0x1d860244,// 73 PAY  70 

    0x26b9050f,// 74 PAY  71 

    0xb2975559,// 75 PAY  72 

    0x42df5e38,// 76 PAY  73 

    0x40ec9183,// 77 PAY  74 

    0x336e14ef,// 78 PAY  75 

    0x95b08b1b,// 79 PAY  76 

    0x50da2ca7,// 80 PAY  77 

    0xf9205e13,// 81 PAY  78 

    0xfbab703d,// 82 PAY  79 

    0xdc6e861f,// 83 PAY  80 

    0xb14e4bd0,// 84 PAY  81 

    0x626c7b9e,// 85 PAY  82 

    0x2ddae152,// 86 PAY  83 

    0xbbcec0d2,// 87 PAY  84 

    0x6413c887,// 88 PAY  85 

    0x7f4711a1,// 89 PAY  86 

    0xc30835f6,// 90 PAY  87 

    0xb4bc3c1d,// 91 PAY  88 

    0xd95e00a2,// 92 PAY  89 

    0x81ebf86d,// 93 PAY  90 

    0x069e15c0,// 94 PAY  91 

    0x217c42f3,// 95 PAY  92 

    0x4c5bfe48,// 96 PAY  93 

    0x0befba4b,// 97 PAY  94 

    0x3af3e473,// 98 PAY  95 

    0x594308e2,// 99 PAY  96 

    0x900473b3,// 100 PAY  97 

    0xe91b5bea,// 101 PAY  98 

    0x42347c52,// 102 PAY  99 

    0xb4c45835,// 103 PAY 100 

    0x966abc2c,// 104 PAY 101 

    0x5c08f105,// 105 PAY 102 

    0x4306f154,// 106 PAY 103 

    0xe9df75c3,// 107 PAY 104 

    0x6da1d4d9,// 108 PAY 105 

    0x8458b25f,// 109 PAY 106 

    0x32f9cd30,// 110 PAY 107 

    0x0a82e582,// 111 PAY 108 

    0x22e7c8f8,// 112 PAY 109 

    0xfd388eef,// 113 PAY 110 

    0x51f39408,// 114 PAY 111 

    0xdec8021c,// 115 PAY 112 

    0x9a299e96,// 116 PAY 113 

    0x115b430e,// 117 PAY 114 

    0x1fe20ad1,// 118 PAY 115 

    0xdfbdb0bd,// 119 PAY 116 

    0x6eb44224,// 120 PAY 117 

    0xdc8ccfbd,// 121 PAY 118 

    0x9185ca27,// 122 PAY 119 

    0x7e7c5ba8,// 123 PAY 120 

    0xc125f71b,// 124 PAY 121 

    0xe716d0c6,// 125 PAY 122 

    0xdf6388e2,// 126 PAY 123 

    0x55342274,// 127 PAY 124 

    0xd5b92cd5,// 128 PAY 125 

    0x300ec9b1,// 129 PAY 126 

    0xa2561171,// 130 PAY 127 

    0x50bc0f60,// 131 PAY 128 

    0x43b61af6,// 132 PAY 129 

    0xe51b312e,// 133 PAY 130 

    0x7798c0fa,// 134 PAY 131 

    0xe3c472f0,// 135 PAY 132 

    0x7f657277,// 136 PAY 133 

    0xc8747411,// 137 PAY 134 

    0xd034a01d,// 138 PAY 135 

    0x056cfa7b,// 139 PAY 136 

    0x03f1976b,// 140 PAY 137 

    0x4f7974c3,// 141 PAY 138 

    0xbef1476f,// 142 PAY 139 

    0x65b82267,// 143 PAY 140 

    0x2bba30d4,// 144 PAY 141 

    0x1b6c0f5e,// 145 PAY 142 

    0x67df59a8,// 146 PAY 143 

    0x5e4c3bcf,// 147 PAY 144 

    0x1e8c8cd5,// 148 PAY 145 

    0x963f22c8,// 149 PAY 146 

    0x4250665d,// 150 PAY 147 

    0xef0a1f32,// 151 PAY 148 

    0x55c48ca4,// 152 PAY 149 

    0x6ce3178f,// 153 PAY 150 

    0x7830bd9d,// 154 PAY 151 

    0x28f1e6eb,// 155 PAY 152 

    0x7f7f6c1d,// 156 PAY 153 

    0x30dfcd76,// 157 PAY 154 

    0x6f75bdb6,// 158 PAY 155 

    0xaed156f4,// 159 PAY 156 

    0xe86871e8,// 160 PAY 157 

    0x37eea4d7,// 161 PAY 158 

    0xa01bb9ae,// 162 PAY 159 

    0xa5200f7c,// 163 PAY 160 

    0xbf1df823,// 164 PAY 161 

    0xed02fb31,// 165 PAY 162 

    0x6ec9e657,// 166 PAY 163 

    0xdbe114ef,// 167 PAY 164 

    0x46b0726e,// 168 PAY 165 

    0xbf899fc5,// 169 PAY 166 

    0x34657ab2,// 170 PAY 167 

    0x182f1ee5,// 171 PAY 168 

    0x958635aa,// 172 PAY 169 

    0x0a06eff0,// 173 PAY 170 

    0x8a09355a,// 174 PAY 171 

    0x1659203c,// 175 PAY 172 

    0xe37a1583,// 176 PAY 173 

    0x6020dbc8,// 177 PAY 174 

    0x1fd370c6,// 178 PAY 175 

    0xb5fca16e,// 179 PAY 176 

    0x15a853e3,// 180 PAY 177 

    0x9f71001e,// 181 PAY 178 

    0x4a4155fa,// 182 PAY 179 

    0x11d53c6e,// 183 PAY 180 

    0x401cae5a,// 184 PAY 181 

    0x1121c16a,// 185 PAY 182 

    0x3dbb8386,// 186 PAY 183 

    0xfc9a91f4,// 187 PAY 184 

    0xb7360a7f,// 188 PAY 185 

    0xd7d95013,// 189 PAY 186 

    0xda5d5261,// 190 PAY 187 

    0xfc7816d9,// 191 PAY 188 

    0xbb0fe947,// 192 PAY 189 

    0x4ee31ec9,// 193 PAY 190 

    0xdea47af6,// 194 PAY 191 

    0x357b0685,// 195 PAY 192 

    0x15ce4088,// 196 PAY 193 

    0xd7594ac4,// 197 PAY 194 

    0x0012a78f,// 198 PAY 195 

    0x4858ca83,// 199 PAY 196 

    0x84ed7998,// 200 PAY 197 

    0x4fb46d54,// 201 PAY 198 

    0xf989c0fe,// 202 PAY 199 

    0x368329f5,// 203 PAY 200 

    0xe04f6cf4,// 204 PAY 201 

    0x47fb6975,// 205 PAY 202 

    0xd7861c29,// 206 PAY 203 

    0x0b94297d,// 207 PAY 204 

    0xd8dafac5,// 208 PAY 205 

    0x990a3f75,// 209 PAY 206 

    0x6ff8c387,// 210 PAY 207 

    0x3c79c0fb,// 211 PAY 208 

    0xd5f0b184,// 212 PAY 209 

    0x4ebec914,// 213 PAY 210 

    0x19462c91,// 214 PAY 211 

/// STA is 1 words. 

/// STA num_pkts       : 128 

/// STA pkt_idx        : 187 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdd 

    0x02ecdd80 // 215 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt63_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 15 words. 

/// BDA size     is 56 (0x38) 

/// BDA id       is 0xad27 

    0x0038ad27,// 3 BDA   1 

/// PAY Generic Data size   : 56 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xbe58f4c3,// 4 PAY   1 

    0x2f94a1e0,// 5 PAY   2 

    0x25fe45ab,// 6 PAY   3 

    0x017b49d3,// 7 PAY   4 

    0xc0ffca9f,// 8 PAY   5 

    0xa5f7bb90,// 9 PAY   6 

    0xd1161887,// 10 PAY   7 

    0xa1f36760,// 11 PAY   8 

    0x6d1effb8,// 12 PAY   9 

    0x6388a906,// 13 PAY  10 

    0x1724b2dc,// 14 PAY  11 

    0xb58fa53c,// 15 PAY  12 

    0x75e7e03c,// 16 PAY  13 

    0x9a9b8e70,// 17 PAY  14 

/// HASH is  12 bytes 

    0x6388a906,// 18 HSH   1 

    0x1724b2dc,// 19 HSH   2 

    0xb58fa53c,// 20 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 83 

/// STA pkt_idx        : 218 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x25 

    0x03682553 // 21 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt64_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 208 words. 

/// BDA size     is 826 (0x33a) 

/// BDA id       is 0x770a 

    0x033a770a,// 3 BDA   1 

/// PAY Generic Data size   : 826 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xfe6819e8,// 4 PAY   1 

    0xd1075e50,// 5 PAY   2 

    0xb92495bf,// 6 PAY   3 

    0x52944350,// 7 PAY   4 

    0x866e2028,// 8 PAY   5 

    0x4c0f43af,// 9 PAY   6 

    0x885403b6,// 10 PAY   7 

    0x5ff466ec,// 11 PAY   8 

    0x5019c958,// 12 PAY   9 

    0x7398c009,// 13 PAY  10 

    0x360b1941,// 14 PAY  11 

    0xf74fc073,// 15 PAY  12 

    0x7d0c854e,// 16 PAY  13 

    0x9b3fc06b,// 17 PAY  14 

    0x69eaaa9a,// 18 PAY  15 

    0x00b68050,// 19 PAY  16 

    0x3ea56df4,// 20 PAY  17 

    0x674635b9,// 21 PAY  18 

    0x6e5b30ff,// 22 PAY  19 

    0x34c60f3b,// 23 PAY  20 

    0x50456d6b,// 24 PAY  21 

    0x50bd23dc,// 25 PAY  22 

    0x3783ce5e,// 26 PAY  23 

    0x451b9cc8,// 27 PAY  24 

    0xb8bfa651,// 28 PAY  25 

    0xbad82c3c,// 29 PAY  26 

    0xa206fbed,// 30 PAY  27 

    0x7fac0d71,// 31 PAY  28 

    0x03de7e99,// 32 PAY  29 

    0x38934621,// 33 PAY  30 

    0x942612a0,// 34 PAY  31 

    0x90eac12f,// 35 PAY  32 

    0xc51bc559,// 36 PAY  33 

    0xb0831df3,// 37 PAY  34 

    0xd40c029d,// 38 PAY  35 

    0xbc5dbb85,// 39 PAY  36 

    0x6fe37eb0,// 40 PAY  37 

    0x170ed068,// 41 PAY  38 

    0xa1201802,// 42 PAY  39 

    0x1e624565,// 43 PAY  40 

    0xfd801f4a,// 44 PAY  41 

    0x07d31050,// 45 PAY  42 

    0xca8dcf06,// 46 PAY  43 

    0x7e63eca9,// 47 PAY  44 

    0x2c0c8ec5,// 48 PAY  45 

    0xc6955c9d,// 49 PAY  46 

    0x1d50df45,// 50 PAY  47 

    0x10311865,// 51 PAY  48 

    0x5e54fc53,// 52 PAY  49 

    0x628a60a7,// 53 PAY  50 

    0x568f5cfe,// 54 PAY  51 

    0x7b21a085,// 55 PAY  52 

    0x6c6c4316,// 56 PAY  53 

    0x76f3c992,// 57 PAY  54 

    0x38a94882,// 58 PAY  55 

    0xd58acc20,// 59 PAY  56 

    0x9e0c73f6,// 60 PAY  57 

    0xffb87781,// 61 PAY  58 

    0xc62643fe,// 62 PAY  59 

    0xb6b51d1e,// 63 PAY  60 

    0xccfa2bec,// 64 PAY  61 

    0x9ea60714,// 65 PAY  62 

    0x1d44c0c1,// 66 PAY  63 

    0x885bf5a1,// 67 PAY  64 

    0xc75e81d5,// 68 PAY  65 

    0x1b252716,// 69 PAY  66 

    0x85e758c5,// 70 PAY  67 

    0x5e90a664,// 71 PAY  68 

    0x27e0fd65,// 72 PAY  69 

    0x7feeb9c2,// 73 PAY  70 

    0x7f938336,// 74 PAY  71 

    0xe7f76ace,// 75 PAY  72 

    0xd08d82ce,// 76 PAY  73 

    0xa5a134b0,// 77 PAY  74 

    0x05068e6b,// 78 PAY  75 

    0x39adcb7c,// 79 PAY  76 

    0x32262340,// 80 PAY  77 

    0x100571b4,// 81 PAY  78 

    0x1abacd79,// 82 PAY  79 

    0xd5ff1ccc,// 83 PAY  80 

    0x4c93ee72,// 84 PAY  81 

    0x88b061ee,// 85 PAY  82 

    0x7c382da9,// 86 PAY  83 

    0x4756b4ec,// 87 PAY  84 

    0xdc381908,// 88 PAY  85 

    0xf7fa1229,// 89 PAY  86 

    0x5489b6c5,// 90 PAY  87 

    0x78b48b48,// 91 PAY  88 

    0x96b585fe,// 92 PAY  89 

    0xc6c8c6c7,// 93 PAY  90 

    0x11f8f455,// 94 PAY  91 

    0x7de1fa17,// 95 PAY  92 

    0x544e2fc8,// 96 PAY  93 

    0x37193d94,// 97 PAY  94 

    0x80de6d7f,// 98 PAY  95 

    0xfec8586a,// 99 PAY  96 

    0x57bbcc20,// 100 PAY  97 

    0x8ca887bc,// 101 PAY  98 

    0xdcda88c9,// 102 PAY  99 

    0x4c2fefbc,// 103 PAY 100 

    0x7302814a,// 104 PAY 101 

    0x4e8b0b40,// 105 PAY 102 

    0x7c8398b4,// 106 PAY 103 

    0xefb02862,// 107 PAY 104 

    0xf755eff8,// 108 PAY 105 

    0x3a4f6bf8,// 109 PAY 106 

    0xf164c0c8,// 110 PAY 107 

    0xd1a1b097,// 111 PAY 108 

    0x9e19820a,// 112 PAY 109 

    0x61058c53,// 113 PAY 110 

    0x88e41bdb,// 114 PAY 111 

    0xdb47b4cc,// 115 PAY 112 

    0x7483f316,// 116 PAY 113 

    0xaad7eea3,// 117 PAY 114 

    0x18695c8c,// 118 PAY 115 

    0x441a09af,// 119 PAY 116 

    0x368242e2,// 120 PAY 117 

    0x423b4013,// 121 PAY 118 

    0x4e83323c,// 122 PAY 119 

    0x8b40b603,// 123 PAY 120 

    0x36a32f03,// 124 PAY 121 

    0x9b76a8d3,// 125 PAY 122 

    0x84aa6390,// 126 PAY 123 

    0x277e445a,// 127 PAY 124 

    0x3f73d15b,// 128 PAY 125 

    0xf9338181,// 129 PAY 126 

    0x3597c3b8,// 130 PAY 127 

    0x7869be53,// 131 PAY 128 

    0x34e6d4cd,// 132 PAY 129 

    0xb0657544,// 133 PAY 130 

    0x7d9643d7,// 134 PAY 131 

    0x370bea3d,// 135 PAY 132 

    0x28f08cc8,// 136 PAY 133 

    0x94ae5008,// 137 PAY 134 

    0xdf6d3562,// 138 PAY 135 

    0x8e2db270,// 139 PAY 136 

    0x23777097,// 140 PAY 137 

    0x1b2e68f9,// 141 PAY 138 

    0xc4adb49a,// 142 PAY 139 

    0xacafa10a,// 143 PAY 140 

    0x093d28c0,// 144 PAY 141 

    0xa02dfe4b,// 145 PAY 142 

    0xe788c94d,// 146 PAY 143 

    0xd163b390,// 147 PAY 144 

    0xd28614f6,// 148 PAY 145 

    0xd3830c55,// 149 PAY 146 

    0xda472386,// 150 PAY 147 

    0x04059e6a,// 151 PAY 148 

    0xbf243fcf,// 152 PAY 149 

    0x07756414,// 153 PAY 150 

    0x0ba6e45b,// 154 PAY 151 

    0xc3930834,// 155 PAY 152 

    0xb8f36edf,// 156 PAY 153 

    0xf6c32dbe,// 157 PAY 154 

    0x56923e94,// 158 PAY 155 

    0x94d8d752,// 159 PAY 156 

    0x2070d46e,// 160 PAY 157 

    0xb5973427,// 161 PAY 158 

    0x2e0b770f,// 162 PAY 159 

    0xb12ff02f,// 163 PAY 160 

    0x93f14274,// 164 PAY 161 

    0x84f758a5,// 165 PAY 162 

    0xcefecb00,// 166 PAY 163 

    0x87b4e82d,// 167 PAY 164 

    0x1b005428,// 168 PAY 165 

    0x1ce73297,// 169 PAY 166 

    0x8efa7380,// 170 PAY 167 

    0x49748b50,// 171 PAY 168 

    0xfe78c01a,// 172 PAY 169 

    0xde222d0d,// 173 PAY 170 

    0x4dc44357,// 174 PAY 171 

    0xd7b80684,// 175 PAY 172 

    0x0cb1820f,// 176 PAY 173 

    0xd654cfdc,// 177 PAY 174 

    0x3d6fe4e1,// 178 PAY 175 

    0x3e313311,// 179 PAY 176 

    0xba56a4f8,// 180 PAY 177 

    0x972cd991,// 181 PAY 178 

    0x032d46dc,// 182 PAY 179 

    0x2611ac83,// 183 PAY 180 

    0x6d6d8dbf,// 184 PAY 181 

    0x3a9bfbc6,// 185 PAY 182 

    0x4862dc81,// 186 PAY 183 

    0x8f0bc642,// 187 PAY 184 

    0x998c94e5,// 188 PAY 185 

    0xe0c9e9b5,// 189 PAY 186 

    0xf0322a2e,// 190 PAY 187 

    0x99269919,// 191 PAY 188 

    0xc0cd76d3,// 192 PAY 189 

    0x7743e5ac,// 193 PAY 190 

    0xf1e19aa5,// 194 PAY 191 

    0x8aacf431,// 195 PAY 192 

    0xc3f236f9,// 196 PAY 193 

    0xa1cee21e,// 197 PAY 194 

    0x9aeb0b4a,// 198 PAY 195 

    0x77e8109a,// 199 PAY 196 

    0x21e8bd52,// 200 PAY 197 

    0x6d4269cb,// 201 PAY 198 

    0xc5b4d05c,// 202 PAY 199 

    0x11989aa3,// 203 PAY 200 

    0x231aa71a,// 204 PAY 201 

    0xaa991ddc,// 205 PAY 202 

    0x5d858126,// 206 PAY 203 

    0x0f5e3184,// 207 PAY 204 

    0x0794e9aa,// 208 PAY 205 

    0xb2b13309,// 209 PAY 206 

    0xa3630000,// 210 PAY 207 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 131 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbb 

    0x020cbb33 // 211 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt65_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 14 words. 

/// BDA size     is 51 (0x33) 

/// BDA id       is 0x5fa4 

    0x00335fa4,// 3 BDA   1 

/// PAY Generic Data size   : 51 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x7d130ccf,// 4 PAY   1 

    0x41632e0a,// 5 PAY   2 

    0x70bcbc45,// 6 PAY   3 

    0xf2b58fd1,// 7 PAY   4 

    0x495e01a9,// 8 PAY   5 

    0xbc940206,// 9 PAY   6 

    0xdededd9f,// 10 PAY   7 

    0xe71fd6d4,// 11 PAY   8 

    0x05695e8c,// 12 PAY   9 

    0x1aa034c5,// 13 PAY  10 

    0x843facd2,// 14 PAY  11 

    0xf9bf2a4e,// 15 PAY  12 

    0x7bd8d200,// 16 PAY  13 

/// HASH is  20 bytes 

    0xe71fd6d4,// 17 HSH   1 

    0x05695e8c,// 18 HSH   2 

    0x1aa034c5,// 19 HSH   3 

    0x843facd2,// 20 HSH   4 

    0xf9bf2a4e,// 21 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 51 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x45 

    0x00d44533 // 22 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt66_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 29 words. 

/// BDA size     is 111 (0x6f) 

/// BDA id       is 0xb717 

    0x006fb717,// 3 BDA   1 

/// PAY Generic Data size   : 111 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf6b4b46a,// 4 PAY   1 

    0x2dd9276a,// 5 PAY   2 

    0xbeb25a81,// 6 PAY   3 

    0x2d4a26e0,// 7 PAY   4 

    0x8897f2b0,// 8 PAY   5 

    0x6f1d0580,// 9 PAY   6 

    0xf69a874e,// 10 PAY   7 

    0x4dc4f234,// 11 PAY   8 

    0x57199cdc,// 12 PAY   9 

    0x6bfebbab,// 13 PAY  10 

    0x39186042,// 14 PAY  11 

    0xbe2166dc,// 15 PAY  12 

    0x88cec337,// 16 PAY  13 

    0x23a952dc,// 17 PAY  14 

    0x429070ac,// 18 PAY  15 

    0xf36abc3d,// 19 PAY  16 

    0x27aa1ed7,// 20 PAY  17 

    0xdf429d28,// 21 PAY  18 

    0x69ff6479,// 22 PAY  19 

    0x2bfe2dd2,// 23 PAY  20 

    0xf4a04f3c,// 24 PAY  21 

    0x0453c292,// 25 PAY  22 

    0x7cf8e31e,// 26 PAY  23 

    0x04261904,// 27 PAY  24 

    0xed9ab102,// 28 PAY  25 

    0xb9db1955,// 29 PAY  26 

    0xc2da281c,// 30 PAY  27 

    0xcfa48500,// 31 PAY  28 

/// HASH is  12 bytes 

    0xf4a04f3c,// 32 HSH   1 

    0x0453c292,// 33 HSH   2 

    0x7cf8e31e,// 34 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 91 

/// STA pkt_idx        : 122 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x83 

    0x01e8835b // 35 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt67_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 80 words. 

/// BDA size     is 313 (0x139) 

/// BDA id       is 0xa522 

    0x0139a522,// 3 BDA   1 

/// PAY Generic Data size   : 313 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x4ca3ea2c,// 4 PAY   1 

    0x8dddba33,// 5 PAY   2 

    0x7c40cc3b,// 6 PAY   3 

    0x25e59d83,// 7 PAY   4 

    0x67710dfe,// 8 PAY   5 

    0x748e3a46,// 9 PAY   6 

    0xf1c798ed,// 10 PAY   7 

    0x6579e48f,// 11 PAY   8 

    0xe0e8010e,// 12 PAY   9 

    0xef1c2b3f,// 13 PAY  10 

    0xc12a3945,// 14 PAY  11 

    0xf9ee356b,// 15 PAY  12 

    0xb6405151,// 16 PAY  13 

    0x2f12265e,// 17 PAY  14 

    0xf7ae595a,// 18 PAY  15 

    0x9ce11b92,// 19 PAY  16 

    0x64094dd3,// 20 PAY  17 

    0x52301c18,// 21 PAY  18 

    0x8671144e,// 22 PAY  19 

    0xee764333,// 23 PAY  20 

    0xdec8f7bb,// 24 PAY  21 

    0x7de289ec,// 25 PAY  22 

    0x2acd4908,// 26 PAY  23 

    0x8934aa3b,// 27 PAY  24 

    0xd1d61ebf,// 28 PAY  25 

    0xaf8cbe80,// 29 PAY  26 

    0x96ab63e8,// 30 PAY  27 

    0x46007e5d,// 31 PAY  28 

    0x3e3de77c,// 32 PAY  29 

    0x4003fb42,// 33 PAY  30 

    0x1fd87df6,// 34 PAY  31 

    0xc68217de,// 35 PAY  32 

    0x5fba3bfe,// 36 PAY  33 

    0x315a3ed5,// 37 PAY  34 

    0x3bbfa372,// 38 PAY  35 

    0x24bc6799,// 39 PAY  36 

    0xe762317a,// 40 PAY  37 

    0x4c3bb256,// 41 PAY  38 

    0xd29830bd,// 42 PAY  39 

    0x039ef5f8,// 43 PAY  40 

    0x1bb3ddf4,// 44 PAY  41 

    0x559bc361,// 45 PAY  42 

    0x5f6e99f7,// 46 PAY  43 

    0xede8813b,// 47 PAY  44 

    0x9d6c24d8,// 48 PAY  45 

    0x5ad290b0,// 49 PAY  46 

    0xf22989bd,// 50 PAY  47 

    0xd0efa55c,// 51 PAY  48 

    0x077cac75,// 52 PAY  49 

    0x2890f826,// 53 PAY  50 

    0x6fd888cf,// 54 PAY  51 

    0xb2981635,// 55 PAY  52 

    0x63902e16,// 56 PAY  53 

    0xbc41fbe9,// 57 PAY  54 

    0x9a092e91,// 58 PAY  55 

    0xc8adcfd0,// 59 PAY  56 

    0x79d777e2,// 60 PAY  57 

    0x92767d33,// 61 PAY  58 

    0x68733460,// 62 PAY  59 

    0x2e758770,// 63 PAY  60 

    0xd5059bb9,// 64 PAY  61 

    0x6f8d37bf,// 65 PAY  62 

    0x307fe42d,// 66 PAY  63 

    0x1f41ba82,// 67 PAY  64 

    0x1e17bd24,// 68 PAY  65 

    0x5575a6c1,// 69 PAY  66 

    0x0db52de6,// 70 PAY  67 

    0xe446b5a0,// 71 PAY  68 

    0x2b807001,// 72 PAY  69 

    0x8e4493a8,// 73 PAY  70 

    0x194c60fc,// 74 PAY  71 

    0x229deeb8,// 75 PAY  72 

    0x76f323f9,// 76 PAY  73 

    0x1d3cae1e,// 77 PAY  74 

    0xf16463c7,// 78 PAY  75 

    0xd7d13edd,// 79 PAY  76 

    0x9313e379,// 80 PAY  77 

    0x355ce742,// 81 PAY  78 

    0xd5000000,// 82 PAY  79 

/// HASH is  8 bytes 

    0x229deeb8,// 83 HSH   1 

    0x76f323f9,// 84 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 221 

/// STA pkt_idx        : 35 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd4 

    0x008cd4dd // 85 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt68_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 333 words. 

/// BDA size     is 1327 (0x52f) 

/// BDA id       is 0x7985 

    0x052f7985,// 3 BDA   1 

/// PAY Generic Data size   : 1327 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x645a792f,// 4 PAY   1 

    0xedc972df,// 5 PAY   2 

    0xcbe27842,// 6 PAY   3 

    0x2193a206,// 7 PAY   4 

    0xb1a270f7,// 8 PAY   5 

    0xc5413a7d,// 9 PAY   6 

    0x0784fd6c,// 10 PAY   7 

    0x39d336fb,// 11 PAY   8 

    0xc74a0f41,// 12 PAY   9 

    0x8b6594fe,// 13 PAY  10 

    0x66dcd55b,// 14 PAY  11 

    0xb989f1fc,// 15 PAY  12 

    0xe988e86c,// 16 PAY  13 

    0xe2ae6108,// 17 PAY  14 

    0xba85c8b1,// 18 PAY  15 

    0x3518d7db,// 19 PAY  16 

    0xe979a42e,// 20 PAY  17 

    0xc3ff24b5,// 21 PAY  18 

    0x35fdd523,// 22 PAY  19 

    0x6eab508c,// 23 PAY  20 

    0x1e9f3e99,// 24 PAY  21 

    0x3f8eab1e,// 25 PAY  22 

    0x32c3e8f3,// 26 PAY  23 

    0x7b5cd77e,// 27 PAY  24 

    0xd1878c7f,// 28 PAY  25 

    0x582d03e0,// 29 PAY  26 

    0x8389f62f,// 30 PAY  27 

    0xdbc07442,// 31 PAY  28 

    0x82079a64,// 32 PAY  29 

    0x9954c8e3,// 33 PAY  30 

    0x84a758c4,// 34 PAY  31 

    0x23564c9b,// 35 PAY  32 

    0x4715233a,// 36 PAY  33 

    0xbd30c208,// 37 PAY  34 

    0x0bd5a271,// 38 PAY  35 

    0xa2d6c257,// 39 PAY  36 

    0x9e07f740,// 40 PAY  37 

    0xcfaacf09,// 41 PAY  38 

    0x13b537dc,// 42 PAY  39 

    0x6cb86579,// 43 PAY  40 

    0x912af7dc,// 44 PAY  41 

    0xee671e50,// 45 PAY  42 

    0x516a8009,// 46 PAY  43 

    0x06c6cc23,// 47 PAY  44 

    0x819d4e59,// 48 PAY  45 

    0xf56ee244,// 49 PAY  46 

    0xad85a3d0,// 50 PAY  47 

    0xf89d49a2,// 51 PAY  48 

    0xc7b79690,// 52 PAY  49 

    0xf4d0c7e9,// 53 PAY  50 

    0xa513a7cb,// 54 PAY  51 

    0x0fef36e7,// 55 PAY  52 

    0xbbe7f192,// 56 PAY  53 

    0x5dd0142b,// 57 PAY  54 

    0xea7d8736,// 58 PAY  55 

    0x164c449f,// 59 PAY  56 

    0xef64360e,// 60 PAY  57 

    0x146ea541,// 61 PAY  58 

    0x3d5f8add,// 62 PAY  59 

    0xf925f206,// 63 PAY  60 

    0x74b9d0b0,// 64 PAY  61 

    0x36f26f72,// 65 PAY  62 

    0xceaebff5,// 66 PAY  63 

    0xf3dc85e3,// 67 PAY  64 

    0xeb941a7d,// 68 PAY  65 

    0x78d93de5,// 69 PAY  66 

    0xe4e6bcab,// 70 PAY  67 

    0x1160e408,// 71 PAY  68 

    0xb624948f,// 72 PAY  69 

    0xde6f2fe4,// 73 PAY  70 

    0xf09bccac,// 74 PAY  71 

    0x859d3277,// 75 PAY  72 

    0x11251992,// 76 PAY  73 

    0xb7278bf4,// 77 PAY  74 

    0x5e795bd0,// 78 PAY  75 

    0x8100ef6e,// 79 PAY  76 

    0xa3ceaccb,// 80 PAY  77 

    0xe0b8451a,// 81 PAY  78 

    0x88acedde,// 82 PAY  79 

    0xfe37c612,// 83 PAY  80 

    0x928b53a9,// 84 PAY  81 

    0x339f22ec,// 85 PAY  82 

    0x0ee37281,// 86 PAY  83 

    0xa1dd43e2,// 87 PAY  84 

    0xf2e43e7b,// 88 PAY  85 

    0xda2e75cc,// 89 PAY  86 

    0x6ae9371e,// 90 PAY  87 

    0x1dfa0f88,// 91 PAY  88 

    0xc747b41b,// 92 PAY  89 

    0x50cb46cb,// 93 PAY  90 

    0x93c1d803,// 94 PAY  91 

    0x4eaeab18,// 95 PAY  92 

    0x6cb0e54a,// 96 PAY  93 

    0xf2449d7b,// 97 PAY  94 

    0x32a37488,// 98 PAY  95 

    0xe1be2eed,// 99 PAY  96 

    0x55be92bf,// 100 PAY  97 

    0x6d26b208,// 101 PAY  98 

    0x2c73ad8d,// 102 PAY  99 

    0x914cc624,// 103 PAY 100 

    0x9964f42d,// 104 PAY 101 

    0x7cd441e5,// 105 PAY 102 

    0x78a82683,// 106 PAY 103 

    0xb170765e,// 107 PAY 104 

    0xa4cc694c,// 108 PAY 105 

    0x40322134,// 109 PAY 106 

    0xbc52fbce,// 110 PAY 107 

    0x2097bbcd,// 111 PAY 108 

    0x05829cd1,// 112 PAY 109 

    0xef8d3d69,// 113 PAY 110 

    0x38d6175b,// 114 PAY 111 

    0x9ef86122,// 115 PAY 112 

    0x4449e484,// 116 PAY 113 

    0x0563b74f,// 117 PAY 114 

    0xa847150c,// 118 PAY 115 

    0xf0cc4df0,// 119 PAY 116 

    0x62d9a1ae,// 120 PAY 117 

    0xd6c9577e,// 121 PAY 118 

    0x220c924f,// 122 PAY 119 

    0x6e4d1e2b,// 123 PAY 120 

    0x5234528f,// 124 PAY 121 

    0x5d4d4525,// 125 PAY 122 

    0x952377ae,// 126 PAY 123 

    0x0b877731,// 127 PAY 124 

    0xf165d633,// 128 PAY 125 

    0x7afceb4f,// 129 PAY 126 

    0x2e881570,// 130 PAY 127 

    0x861d2630,// 131 PAY 128 

    0xfcac507f,// 132 PAY 129 

    0x3bf6577c,// 133 PAY 130 

    0x1bfa71cf,// 134 PAY 131 

    0xd2b6862e,// 135 PAY 132 

    0x5e11c2ab,// 136 PAY 133 

    0xe5ed1b88,// 137 PAY 134 

    0x494bd55c,// 138 PAY 135 

    0x9cd9b181,// 139 PAY 136 

    0x24f97611,// 140 PAY 137 

    0x1cdcaed7,// 141 PAY 138 

    0xda405a16,// 142 PAY 139 

    0x90abdecc,// 143 PAY 140 

    0xa368c666,// 144 PAY 141 

    0x92caddb4,// 145 PAY 142 

    0xc588fef7,// 146 PAY 143 

    0xaef0508b,// 147 PAY 144 

    0xd0c1c6b0,// 148 PAY 145 

    0xdaeb7337,// 149 PAY 146 

    0xf3ae41ff,// 150 PAY 147 

    0x39c8c638,// 151 PAY 148 

    0x52fbfb74,// 152 PAY 149 

    0x5a5c8e1b,// 153 PAY 150 

    0x5f42bbf0,// 154 PAY 151 

    0x1b1560b6,// 155 PAY 152 

    0xa450f3d1,// 156 PAY 153 

    0xe1731cda,// 157 PAY 154 

    0xc02f6c35,// 158 PAY 155 

    0xbd4e6c33,// 159 PAY 156 

    0x23868ca1,// 160 PAY 157 

    0xef2622bc,// 161 PAY 158 

    0x05f73fe4,// 162 PAY 159 

    0x53dfbd6e,// 163 PAY 160 

    0x4dae091b,// 164 PAY 161 

    0x133bbf22,// 165 PAY 162 

    0x7928748c,// 166 PAY 163 

    0x64d09b75,// 167 PAY 164 

    0xf0372281,// 168 PAY 165 

    0x5c00a13c,// 169 PAY 166 

    0xafd76a3b,// 170 PAY 167 

    0x4cb152b1,// 171 PAY 168 

    0x1aee81bc,// 172 PAY 169 

    0xae464700,// 173 PAY 170 

    0xce169f7f,// 174 PAY 171 

    0xca15fa4e,// 175 PAY 172 

    0x08094514,// 176 PAY 173 

    0xbefdfcb9,// 177 PAY 174 

    0xd3a0a132,// 178 PAY 175 

    0x0f5ff80d,// 179 PAY 176 

    0x7eba0a9f,// 180 PAY 177 

    0x4debb848,// 181 PAY 178 

    0x38c180dd,// 182 PAY 179 

    0xcbe5f48a,// 183 PAY 180 

    0x788b70c3,// 184 PAY 181 

    0x518b2b4b,// 185 PAY 182 

    0xb345232f,// 186 PAY 183 

    0x74fb1a17,// 187 PAY 184 

    0xd7078667,// 188 PAY 185 

    0x8cb4a22b,// 189 PAY 186 

    0xac5b0937,// 190 PAY 187 

    0xdb8e9d08,// 191 PAY 188 

    0x854545fa,// 192 PAY 189 

    0x0e3c100f,// 193 PAY 190 

    0x4c0a2689,// 194 PAY 191 

    0xf9e750e9,// 195 PAY 192 

    0x123edbad,// 196 PAY 193 

    0x68f6888b,// 197 PAY 194 

    0xb3eb7256,// 198 PAY 195 

    0xd5f9302f,// 199 PAY 196 

    0x100f8732,// 200 PAY 197 

    0x70caa38c,// 201 PAY 198 

    0x9a908355,// 202 PAY 199 

    0x9bbb1817,// 203 PAY 200 

    0x3e1c3377,// 204 PAY 201 

    0x8e49d671,// 205 PAY 202 

    0xe35a1fec,// 206 PAY 203 

    0x233f4c8e,// 207 PAY 204 

    0x63bee6fd,// 208 PAY 205 

    0x99ead7aa,// 209 PAY 206 

    0xb987615b,// 210 PAY 207 

    0x83e06833,// 211 PAY 208 

    0x25b4aefa,// 212 PAY 209 

    0x852d22ae,// 213 PAY 210 

    0x8a6eaf80,// 214 PAY 211 

    0xac7e6528,// 215 PAY 212 

    0x5db51095,// 216 PAY 213 

    0x82794e23,// 217 PAY 214 

    0x7344f0a7,// 218 PAY 215 

    0x0c88db72,// 219 PAY 216 

    0x73c2b790,// 220 PAY 217 

    0x64ffd0c8,// 221 PAY 218 

    0x620d591d,// 222 PAY 219 

    0x3e8c7cdf,// 223 PAY 220 

    0x12369bac,// 224 PAY 221 

    0xb4762d4e,// 225 PAY 222 

    0x47010767,// 226 PAY 223 

    0xfe6d5f1c,// 227 PAY 224 

    0xc21c05c7,// 228 PAY 225 

    0x3501223b,// 229 PAY 226 

    0xa027f453,// 230 PAY 227 

    0x8a79e95b,// 231 PAY 228 

    0xa09bbb46,// 232 PAY 229 

    0x732d382e,// 233 PAY 230 

    0x7b952202,// 234 PAY 231 

    0x976230c3,// 235 PAY 232 

    0xdb189bb6,// 236 PAY 233 

    0xdd211d83,// 237 PAY 234 

    0x8132b286,// 238 PAY 235 

    0x08c25a85,// 239 PAY 236 

    0x1cfa6397,// 240 PAY 237 

    0x83c76731,// 241 PAY 238 

    0x0742c10e,// 242 PAY 239 

    0xad892a86,// 243 PAY 240 

    0x46711935,// 244 PAY 241 

    0x1466c62e,// 245 PAY 242 

    0x9a775c02,// 246 PAY 243 

    0x7effe4e5,// 247 PAY 244 

    0xc698a934,// 248 PAY 245 

    0xcbe3e455,// 249 PAY 246 

    0x85d2b436,// 250 PAY 247 

    0x8962c1c9,// 251 PAY 248 

    0x84e19b02,// 252 PAY 249 

    0x5b88ddca,// 253 PAY 250 

    0xe5ce1e61,// 254 PAY 251 

    0x98e51ebe,// 255 PAY 252 

    0xa06b6235,// 256 PAY 253 

    0x879f7bc2,// 257 PAY 254 

    0xae66e25a,// 258 PAY 255 

    0x8bcc665b,// 259 PAY 256 

    0xdf655ad1,// 260 PAY 257 

    0x515d9ee8,// 261 PAY 258 

    0x4f011218,// 262 PAY 259 

    0x352da860,// 263 PAY 260 

    0xc5ff4d67,// 264 PAY 261 

    0xa7356eea,// 265 PAY 262 

    0xc0a83739,// 266 PAY 263 

    0x7fd30891,// 267 PAY 264 

    0xd040d325,// 268 PAY 265 

    0x0171ea95,// 269 PAY 266 

    0xf721cfcd,// 270 PAY 267 

    0x1a7e78bc,// 271 PAY 268 

    0x0955d93c,// 272 PAY 269 

    0xd2c9dfe6,// 273 PAY 270 

    0x1384c9dc,// 274 PAY 271 

    0x4b0428e2,// 275 PAY 272 

    0xc529a9dd,// 276 PAY 273 

    0x7c1aadc7,// 277 PAY 274 

    0xda01ac75,// 278 PAY 275 

    0xec621064,// 279 PAY 276 

    0x8a7a4d9b,// 280 PAY 277 

    0xb02a79d2,// 281 PAY 278 

    0xc0d65a8f,// 282 PAY 279 

    0x1c097008,// 283 PAY 280 

    0x5b0255a8,// 284 PAY 281 

    0x0520980b,// 285 PAY 282 

    0xb231b77c,// 286 PAY 283 

    0x753385f1,// 287 PAY 284 

    0x2666629a,// 288 PAY 285 

    0x3619109e,// 289 PAY 286 

    0x600a5bbc,// 290 PAY 287 

    0x352804d8,// 291 PAY 288 

    0xa3fe9455,// 292 PAY 289 

    0xbd704350,// 293 PAY 290 

    0x762bef74,// 294 PAY 291 

    0x17c57615,// 295 PAY 292 

    0xc41310cf,// 296 PAY 293 

    0x823351d8,// 297 PAY 294 

    0x6d99fdcc,// 298 PAY 295 

    0x41936330,// 299 PAY 296 

    0x3486dd17,// 300 PAY 297 

    0xf28c563a,// 301 PAY 298 

    0x8f0858bb,// 302 PAY 299 

    0x03a7ef78,// 303 PAY 300 

    0x795bc02c,// 304 PAY 301 

    0x9ac87d81,// 305 PAY 302 

    0x0f3b729a,// 306 PAY 303 

    0xbc4067c1,// 307 PAY 304 

    0x47d5c5fd,// 308 PAY 305 

    0x6a805656,// 309 PAY 306 

    0xa7da2cad,// 310 PAY 307 

    0x10557b6d,// 311 PAY 308 

    0x7d1f6837,// 312 PAY 309 

    0x80324980,// 313 PAY 310 

    0x17faa5b8,// 314 PAY 311 

    0x6aa12720,// 315 PAY 312 

    0x458b0e78,// 316 PAY 313 

    0xbda0409f,// 317 PAY 314 

    0x5af3cd12,// 318 PAY 315 

    0x25397088,// 319 PAY 316 

    0x5cca54e0,// 320 PAY 317 

    0x1892c8f4,// 321 PAY 318 

    0xbfd0b2fe,// 322 PAY 319 

    0xd29a4b38,// 323 PAY 320 

    0x4cbbc246,// 324 PAY 321 

    0x312241b8,// 325 PAY 322 

    0x4bd19aa2,// 326 PAY 323 

    0x56b05278,// 327 PAY 324 

    0x6d68d755,// 328 PAY 325 

    0x6fabe8aa,// 329 PAY 326 

    0x4a1bbecb,// 330 PAY 327 

    0x6c9c1c28,// 331 PAY 328 

    0x5e2bb4da,// 332 PAY 329 

    0xeaa0b09b,// 333 PAY 330 

    0x798fc14a,// 334 PAY 331 

    0x6d14ec00,// 335 PAY 332 

/// HASH is  20 bytes 

    0x4cbbc246,// 336 HSH   1 

    0x312241b8,// 337 HSH   2 

    0x4bd19aa2,// 338 HSH   3 

    0x56b05278,// 339 HSH   4 

    0x6d68d755,// 340 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 198 

/// STA pkt_idx        : 217 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x9d 

    0x03659dc6 // 341 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt69_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 497 words. 

/// BDA size     is 1982 (0x7be) 

/// BDA id       is 0xe834 

    0x07bee834,// 3 BDA   1 

/// PAY Generic Data size   : 1982 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xe3d5cfc8,// 4 PAY   1 

    0x868022d5,// 5 PAY   2 

    0x4ef5832f,// 6 PAY   3 

    0xe632d969,// 7 PAY   4 

    0x89da5e94,// 8 PAY   5 

    0xae4a68fc,// 9 PAY   6 

    0xc894e55f,// 10 PAY   7 

    0xc605c8a0,// 11 PAY   8 

    0x20e88a44,// 12 PAY   9 

    0x8140cc41,// 13 PAY  10 

    0xa20ff1ce,// 14 PAY  11 

    0x5f35a165,// 15 PAY  12 

    0x24cb9fde,// 16 PAY  13 

    0x2605e6b6,// 17 PAY  14 

    0x6622a2a2,// 18 PAY  15 

    0xa1884d71,// 19 PAY  16 

    0x78fd88c3,// 20 PAY  17 

    0x6aa1fd1c,// 21 PAY  18 

    0xb6f99946,// 22 PAY  19 

    0x07a07f92,// 23 PAY  20 

    0x00b3d27d,// 24 PAY  21 

    0x8cc41be3,// 25 PAY  22 

    0x0f34e924,// 26 PAY  23 

    0xf5388c93,// 27 PAY  24 

    0xf8ec9370,// 28 PAY  25 

    0x8576012a,// 29 PAY  26 

    0x568216d6,// 30 PAY  27 

    0x3f3c6432,// 31 PAY  28 

    0x706171b5,// 32 PAY  29 

    0x2a9ffa3e,// 33 PAY  30 

    0xd8592cd2,// 34 PAY  31 

    0x7168dbfa,// 35 PAY  32 

    0x5b6a9470,// 36 PAY  33 

    0x2666dbcb,// 37 PAY  34 

    0x6b851531,// 38 PAY  35 

    0xd38beeba,// 39 PAY  36 

    0x2f5f1582,// 40 PAY  37 

    0xe4cf79ff,// 41 PAY  38 

    0x966fb515,// 42 PAY  39 

    0xd863dec9,// 43 PAY  40 

    0xfe7ffd8f,// 44 PAY  41 

    0xf91c89a3,// 45 PAY  42 

    0x90a41af4,// 46 PAY  43 

    0x95150e94,// 47 PAY  44 

    0xdc042bc0,// 48 PAY  45 

    0x51a7148c,// 49 PAY  46 

    0xff64fcd5,// 50 PAY  47 

    0x68f99d48,// 51 PAY  48 

    0xceffb823,// 52 PAY  49 

    0x4081abb6,// 53 PAY  50 

    0xac379345,// 54 PAY  51 

    0x23e83215,// 55 PAY  52 

    0x014e5897,// 56 PAY  53 

    0x2a2cf6d8,// 57 PAY  54 

    0xedc52e96,// 58 PAY  55 

    0x2b82485c,// 59 PAY  56 

    0x99d8d8ae,// 60 PAY  57 

    0x936d18ae,// 61 PAY  58 

    0x98ad13a2,// 62 PAY  59 

    0xfd26a123,// 63 PAY  60 

    0x84cc4da3,// 64 PAY  61 

    0x9508ebc2,// 65 PAY  62 

    0xfb5f3edd,// 66 PAY  63 

    0xf7c1180e,// 67 PAY  64 

    0x4002ba26,// 68 PAY  65 

    0xb276b007,// 69 PAY  66 

    0xaf1cd894,// 70 PAY  67 

    0x9def65ab,// 71 PAY  68 

    0x970b08f9,// 72 PAY  69 

    0x21a47e8b,// 73 PAY  70 

    0x6c13590a,// 74 PAY  71 

    0x999fa93a,// 75 PAY  72 

    0x409e70f1,// 76 PAY  73 

    0x11f0dd46,// 77 PAY  74 

    0xc6620811,// 78 PAY  75 

    0x55ae3bb1,// 79 PAY  76 

    0xcf676e12,// 80 PAY  77 

    0x60e40d5c,// 81 PAY  78 

    0x75a906ab,// 82 PAY  79 

    0x90e79371,// 83 PAY  80 

    0x1c5a38bd,// 84 PAY  81 

    0xa3f0fb7b,// 85 PAY  82 

    0x0a3b72b2,// 86 PAY  83 

    0xe5cb0e4a,// 87 PAY  84 

    0x0d7a5dd4,// 88 PAY  85 

    0x43d08178,// 89 PAY  86 

    0x2db8d64e,// 90 PAY  87 

    0xc731dca0,// 91 PAY  88 

    0xf562c1bb,// 92 PAY  89 

    0x70df8093,// 93 PAY  90 

    0xf14e5421,// 94 PAY  91 

    0xa8219a3a,// 95 PAY  92 

    0xd9fbe4c3,// 96 PAY  93 

    0x8e2daa19,// 97 PAY  94 

    0x51b51599,// 98 PAY  95 

    0x3a0265ff,// 99 PAY  96 

    0xc0c0f71f,// 100 PAY  97 

    0xccc25de3,// 101 PAY  98 

    0x3066b7de,// 102 PAY  99 

    0x89af4e51,// 103 PAY 100 

    0x4fba5be1,// 104 PAY 101 

    0xa258663a,// 105 PAY 102 

    0xdd1f4b11,// 106 PAY 103 

    0x34715530,// 107 PAY 104 

    0xdc2c8254,// 108 PAY 105 

    0x7ac06024,// 109 PAY 106 

    0xd3ca04ae,// 110 PAY 107 

    0x6de5c975,// 111 PAY 108 

    0x062151e9,// 112 PAY 109 

    0xffadc9af,// 113 PAY 110 

    0x76f4b27a,// 114 PAY 111 

    0xdb116972,// 115 PAY 112 

    0xf1295964,// 116 PAY 113 

    0xab03c2a2,// 117 PAY 114 

    0x70b89272,// 118 PAY 115 

    0x3007113c,// 119 PAY 116 

    0x7a5bd2b0,// 120 PAY 117 

    0xfd61430c,// 121 PAY 118 

    0x0facc966,// 122 PAY 119 

    0x60e39412,// 123 PAY 120 

    0x4a186fa4,// 124 PAY 121 

    0xca6b5283,// 125 PAY 122 

    0x4ea6cf57,// 126 PAY 123 

    0xd3f897c9,// 127 PAY 124 

    0x81c15644,// 128 PAY 125 

    0xf15e9890,// 129 PAY 126 

    0x1130e66a,// 130 PAY 127 

    0xb1738cca,// 131 PAY 128 

    0xd0622896,// 132 PAY 129 

    0xf173aff2,// 133 PAY 130 

    0x8f34f190,// 134 PAY 131 

    0x9c1b5862,// 135 PAY 132 

    0x8b58550e,// 136 PAY 133 

    0x62994bdf,// 137 PAY 134 

    0x023e7d99,// 138 PAY 135 

    0x46819a53,// 139 PAY 136 

    0x0e5ad5ff,// 140 PAY 137 

    0x19409564,// 141 PAY 138 

    0xb3540957,// 142 PAY 139 

    0xc73a6d89,// 143 PAY 140 

    0x14bcfdd1,// 144 PAY 141 

    0xbe04d062,// 145 PAY 142 

    0xfc1075ea,// 146 PAY 143 

    0xb4f0f79e,// 147 PAY 144 

    0xf6199306,// 148 PAY 145 

    0xe23ae645,// 149 PAY 146 

    0x82d97d8b,// 150 PAY 147 

    0x37010bdb,// 151 PAY 148 

    0xa11db938,// 152 PAY 149 

    0xfd22e970,// 153 PAY 150 

    0x0e15e0c4,// 154 PAY 151 

    0x28850c34,// 155 PAY 152 

    0x70505611,// 156 PAY 153 

    0xc18da583,// 157 PAY 154 

    0xd082dc58,// 158 PAY 155 

    0x4f053472,// 159 PAY 156 

    0xbdbb5850,// 160 PAY 157 

    0xd13ae5b3,// 161 PAY 158 

    0x707d824c,// 162 PAY 159 

    0x33f07fed,// 163 PAY 160 

    0xed7de9d1,// 164 PAY 161 

    0xc3151584,// 165 PAY 162 

    0x578fa83d,// 166 PAY 163 

    0xa2001963,// 167 PAY 164 

    0x9b399dc3,// 168 PAY 165 

    0x2f6fc91f,// 169 PAY 166 

    0x09e3fa92,// 170 PAY 167 

    0x0393938d,// 171 PAY 168 

    0xa122f2db,// 172 PAY 169 

    0x49858b4c,// 173 PAY 170 

    0xb78151b0,// 174 PAY 171 

    0x6a2e4037,// 175 PAY 172 

    0x7b2ddae7,// 176 PAY 173 

    0x3279bae0,// 177 PAY 174 

    0xb80ec4ed,// 178 PAY 175 

    0xd3e8c5bb,// 179 PAY 176 

    0xa27ba7d5,// 180 PAY 177 

    0xcba6384b,// 181 PAY 178 

    0x4ba7153f,// 182 PAY 179 

    0xa4412498,// 183 PAY 180 

    0x145efdbe,// 184 PAY 181 

    0xc1f39d3f,// 185 PAY 182 

    0x6ff5b67a,// 186 PAY 183 

    0xac694a2d,// 187 PAY 184 

    0x6609a13e,// 188 PAY 185 

    0xd34b0fb8,// 189 PAY 186 

    0x402f0853,// 190 PAY 187 

    0x86d30eb3,// 191 PAY 188 

    0x9400e4d5,// 192 PAY 189 

    0x88349a3e,// 193 PAY 190 

    0xbbf499a1,// 194 PAY 191 

    0x5a8222c4,// 195 PAY 192 

    0xca03dfa0,// 196 PAY 193 

    0x1764fdbc,// 197 PAY 194 

    0xaea2ff9b,// 198 PAY 195 

    0x8cf789d6,// 199 PAY 196 

    0x7866df99,// 200 PAY 197 

    0xb99639c6,// 201 PAY 198 

    0x654882ad,// 202 PAY 199 

    0x58897f3c,// 203 PAY 200 

    0x2dacadd4,// 204 PAY 201 

    0x6373352d,// 205 PAY 202 

    0x2269ccf3,// 206 PAY 203 

    0x8ca73f64,// 207 PAY 204 

    0x3e4b9986,// 208 PAY 205 

    0xa2debe5b,// 209 PAY 206 

    0x1fac72ea,// 210 PAY 207 

    0xf21a5427,// 211 PAY 208 

    0x3de13454,// 212 PAY 209 

    0x2c000232,// 213 PAY 210 

    0x91041352,// 214 PAY 211 

    0x70b1457b,// 215 PAY 212 

    0xa5e6d12e,// 216 PAY 213 

    0x56aba714,// 217 PAY 214 

    0x9493528a,// 218 PAY 215 

    0x88d87c00,// 219 PAY 216 

    0x02677010,// 220 PAY 217 

    0x25b31ba1,// 221 PAY 218 

    0x06c0c5be,// 222 PAY 219 

    0x54bffb0f,// 223 PAY 220 

    0x62daeec5,// 224 PAY 221 

    0x1d9295f2,// 225 PAY 222 

    0x20c7f86a,// 226 PAY 223 

    0x678bd828,// 227 PAY 224 

    0x26709867,// 228 PAY 225 

    0x7e7d8ebd,// 229 PAY 226 

    0x5ba17a7e,// 230 PAY 227 

    0x0edf6aef,// 231 PAY 228 

    0x095704af,// 232 PAY 229 

    0x3c5baa5d,// 233 PAY 230 

    0xa86dff65,// 234 PAY 231 

    0xec70f1cf,// 235 PAY 232 

    0xb22228cf,// 236 PAY 233 

    0x6b16748f,// 237 PAY 234 

    0x26d538e6,// 238 PAY 235 

    0x0ab5eb05,// 239 PAY 236 

    0x4c779145,// 240 PAY 237 

    0x6cfb1128,// 241 PAY 238 

    0x53616ada,// 242 PAY 239 

    0x3b69f7fa,// 243 PAY 240 

    0x18722e71,// 244 PAY 241 

    0xa5bd16ec,// 245 PAY 242 

    0xc467261e,// 246 PAY 243 

    0x8627c949,// 247 PAY 244 

    0xd688be47,// 248 PAY 245 

    0x6ed12a5c,// 249 PAY 246 

    0xf10fdd94,// 250 PAY 247 

    0x097a4c16,// 251 PAY 248 

    0xbbd4d07a,// 252 PAY 249 

    0xf33542fe,// 253 PAY 250 

    0xf4324f61,// 254 PAY 251 

    0x333efb2e,// 255 PAY 252 

    0x5bf91726,// 256 PAY 253 

    0x29acacb1,// 257 PAY 254 

    0x50ac9fdb,// 258 PAY 255 

    0xbfb86510,// 259 PAY 256 

    0x89460451,// 260 PAY 257 

    0x281d5c5c,// 261 PAY 258 

    0x7c9f070b,// 262 PAY 259 

    0xbd929316,// 263 PAY 260 

    0xe1cf6232,// 264 PAY 261 

    0xd224ff01,// 265 PAY 262 

    0x27973b9f,// 266 PAY 263 

    0x9b7a09f7,// 267 PAY 264 

    0xc5ff4be6,// 268 PAY 265 

    0x5838bc54,// 269 PAY 266 

    0x610e3a89,// 270 PAY 267 

    0x94fcd911,// 271 PAY 268 

    0x26314d66,// 272 PAY 269 

    0xf81f90fc,// 273 PAY 270 

    0xc96deab1,// 274 PAY 271 

    0x4837eddc,// 275 PAY 272 

    0x968ad125,// 276 PAY 273 

    0x489ff91a,// 277 PAY 274 

    0xe835a642,// 278 PAY 275 

    0xcb8dea08,// 279 PAY 276 

    0x7b991d53,// 280 PAY 277 

    0xe65ea6b7,// 281 PAY 278 

    0x6cf7e9dc,// 282 PAY 279 

    0xb0e8fa30,// 283 PAY 280 

    0xa5f7dc25,// 284 PAY 281 

    0x75930657,// 285 PAY 282 

    0x85ec7315,// 286 PAY 283 

    0x9e237082,// 287 PAY 284 

    0x725d69b9,// 288 PAY 285 

    0xcc510b93,// 289 PAY 286 

    0x4823a89e,// 290 PAY 287 

    0xe504af1e,// 291 PAY 288 

    0xdc8255db,// 292 PAY 289 

    0x7e20f22e,// 293 PAY 290 

    0x8f9f0916,// 294 PAY 291 

    0xde8b0926,// 295 PAY 292 

    0x82363779,// 296 PAY 293 

    0xed5d04ac,// 297 PAY 294 

    0xd33d4298,// 298 PAY 295 

    0x28074314,// 299 PAY 296 

    0xf34e231d,// 300 PAY 297 

    0xa3c9124a,// 301 PAY 298 

    0x841e088b,// 302 PAY 299 

    0xcd5f68ed,// 303 PAY 300 

    0xd0412a76,// 304 PAY 301 

    0x3e96a588,// 305 PAY 302 

    0x2a7889f9,// 306 PAY 303 

    0xdbbb6467,// 307 PAY 304 

    0x8e972452,// 308 PAY 305 

    0xef9bcee0,// 309 PAY 306 

    0xf83f45a9,// 310 PAY 307 

    0x85c146fa,// 311 PAY 308 

    0xa75a16c5,// 312 PAY 309 

    0x95c2ef82,// 313 PAY 310 

    0xf087f5f5,// 314 PAY 311 

    0xe157408b,// 315 PAY 312 

    0xda2bc7c3,// 316 PAY 313 

    0x0120140f,// 317 PAY 314 

    0x6dd83ccb,// 318 PAY 315 

    0xe9ab0b6e,// 319 PAY 316 

    0x66390bb3,// 320 PAY 317 

    0x92d028a3,// 321 PAY 318 

    0xd9678d02,// 322 PAY 319 

    0x0a4e21bd,// 323 PAY 320 

    0x980e50b9,// 324 PAY 321 

    0xc8aac6b8,// 325 PAY 322 

    0x6d530bab,// 326 PAY 323 

    0x81c64421,// 327 PAY 324 

    0x297432a5,// 328 PAY 325 

    0xca8ef9ad,// 329 PAY 326 

    0x7df6d760,// 330 PAY 327 

    0xa31bd6ed,// 331 PAY 328 

    0x9c61eb1c,// 332 PAY 329 

    0x366484cf,// 333 PAY 330 

    0xf80914ef,// 334 PAY 331 

    0x869c06c1,// 335 PAY 332 

    0xaf514267,// 336 PAY 333 

    0x0c9a5b42,// 337 PAY 334 

    0x5759570b,// 338 PAY 335 

    0xd4eaecb1,// 339 PAY 336 

    0x1dcae8ad,// 340 PAY 337 

    0xde7d2a3c,// 341 PAY 338 

    0x6b13f064,// 342 PAY 339 

    0x3417819d,// 343 PAY 340 

    0x7b72c98e,// 344 PAY 341 

    0x59b58a33,// 345 PAY 342 

    0x403142aa,// 346 PAY 343 

    0xab97bfe1,// 347 PAY 344 

    0x369b8db2,// 348 PAY 345 

    0xa655198f,// 349 PAY 346 

    0x685c8c58,// 350 PAY 347 

    0xeb490189,// 351 PAY 348 

    0xa82ac5fe,// 352 PAY 349 

    0x74d87287,// 353 PAY 350 

    0x6546db81,// 354 PAY 351 

    0xfe036fec,// 355 PAY 352 

    0xf0a24d3b,// 356 PAY 353 

    0xb90850e4,// 357 PAY 354 

    0x4fe81d9e,// 358 PAY 355 

    0x7a5b536b,// 359 PAY 356 

    0x363a8381,// 360 PAY 357 

    0xd65839a8,// 361 PAY 358 

    0x026694d4,// 362 PAY 359 

    0xe58a1b14,// 363 PAY 360 

    0xa752181a,// 364 PAY 361 

    0x4eab428b,// 365 PAY 362 

    0x63e9c48c,// 366 PAY 363 

    0xbfcfddff,// 367 PAY 364 

    0xd6315cef,// 368 PAY 365 

    0xd75b8acb,// 369 PAY 366 

    0x5412ed7c,// 370 PAY 367 

    0x63432c4b,// 371 PAY 368 

    0x56ff49f4,// 372 PAY 369 

    0xdcbe7f8a,// 373 PAY 370 

    0x5bcb4091,// 374 PAY 371 

    0xe1b7b957,// 375 PAY 372 

    0xa57a7780,// 376 PAY 373 

    0x9306eb7d,// 377 PAY 374 

    0x7eb05bf0,// 378 PAY 375 

    0x2d3e9e1b,// 379 PAY 376 

    0x644cfd6c,// 380 PAY 377 

    0x3fbdd083,// 381 PAY 378 

    0x8df0449f,// 382 PAY 379 

    0xffd2b5d0,// 383 PAY 380 

    0xef822892,// 384 PAY 381 

    0x635d5dde,// 385 PAY 382 

    0xb903fba7,// 386 PAY 383 

    0xaa4c1afd,// 387 PAY 384 

    0x2336e9eb,// 388 PAY 385 

    0x2bb83bae,// 389 PAY 386 

    0xad477af4,// 390 PAY 387 

    0xcec5410c,// 391 PAY 388 

    0x561435a3,// 392 PAY 389 

    0x46e9d742,// 393 PAY 390 

    0xf3730c81,// 394 PAY 391 

    0x156901bd,// 395 PAY 392 

    0x9e0c5bbc,// 396 PAY 393 

    0x0b7c9d19,// 397 PAY 394 

    0xe878753c,// 398 PAY 395 

    0x8b9dcd94,// 399 PAY 396 

    0x6ad01ec8,// 400 PAY 397 

    0x174c248d,// 401 PAY 398 

    0x52ca88d7,// 402 PAY 399 

    0xbf79f1eb,// 403 PAY 400 

    0xaeb6ebfc,// 404 PAY 401 

    0x69193d50,// 405 PAY 402 

    0xd4255ef8,// 406 PAY 403 

    0xc11df00c,// 407 PAY 404 

    0xddb64f15,// 408 PAY 405 

    0x12b3a6ef,// 409 PAY 406 

    0x5a54966c,// 410 PAY 407 

    0x46cb4275,// 411 PAY 408 

    0xabdc3761,// 412 PAY 409 

    0x5fbf01b4,// 413 PAY 410 

    0x53ce466e,// 414 PAY 411 

    0xf5c9c574,// 415 PAY 412 

    0x69bae7b3,// 416 PAY 413 

    0x039008ef,// 417 PAY 414 

    0x35ba432d,// 418 PAY 415 

    0xfd819bb0,// 419 PAY 416 

    0x14ff2da7,// 420 PAY 417 

    0xa6895b12,// 421 PAY 418 

    0x40ca2dc2,// 422 PAY 419 

    0x438c030d,// 423 PAY 420 

    0x0f308b2e,// 424 PAY 421 

    0x8490bbea,// 425 PAY 422 

    0xf0dc3363,// 426 PAY 423 

    0x47f7bccd,// 427 PAY 424 

    0x924fce11,// 428 PAY 425 

    0x34238755,// 429 PAY 426 

    0xe15928b8,// 430 PAY 427 

    0x6ef8308d,// 431 PAY 428 

    0x0bf526bd,// 432 PAY 429 

    0xc3e285a7,// 433 PAY 430 

    0xac41188a,// 434 PAY 431 

    0x62e01b2e,// 435 PAY 432 

    0x729289c2,// 436 PAY 433 

    0xe5dbc081,// 437 PAY 434 

    0x117162cd,// 438 PAY 435 

    0xbede803a,// 439 PAY 436 

    0x4070d5a2,// 440 PAY 437 

    0x52153264,// 441 PAY 438 

    0x7491777d,// 442 PAY 439 

    0x57ad4ebf,// 443 PAY 440 

    0xfefa8612,// 444 PAY 441 

    0x7a7db524,// 445 PAY 442 

    0xdc8d947f,// 446 PAY 443 

    0x684b3418,// 447 PAY 444 

    0x2c34d20c,// 448 PAY 445 

    0xfea711bb,// 449 PAY 446 

    0xe9e334a0,// 450 PAY 447 

    0x173da67d,// 451 PAY 448 

    0x402f5921,// 452 PAY 449 

    0x31437496,// 453 PAY 450 

    0xfb7c7133,// 454 PAY 451 

    0x3c57f3db,// 455 PAY 452 

    0x6ef87994,// 456 PAY 453 

    0x6e3d46b1,// 457 PAY 454 

    0x486c4d0e,// 458 PAY 455 

    0xc1567d2a,// 459 PAY 456 

    0x5e37409e,// 460 PAY 457 

    0x7bc7b458,// 461 PAY 458 

    0x27303adf,// 462 PAY 459 

    0x5f7b6bea,// 463 PAY 460 

    0x60e4b11b,// 464 PAY 461 

    0xdd61a2f8,// 465 PAY 462 

    0xd7b82cd9,// 466 PAY 463 

    0x268b1616,// 467 PAY 464 

    0x1d628ae8,// 468 PAY 465 

    0x9c910c19,// 469 PAY 466 

    0x30f47cd0,// 470 PAY 467 

    0x0b646e7d,// 471 PAY 468 

    0x4269608c,// 472 PAY 469 

    0x225bcb24,// 473 PAY 470 

    0x9f5f56bc,// 474 PAY 471 

    0x050cef16,// 475 PAY 472 

    0x21cb9c5f,// 476 PAY 473 

    0x6974cf75,// 477 PAY 474 

    0x103e060e,// 478 PAY 475 

    0xecbf7baf,// 479 PAY 476 

    0xecce9630,// 480 PAY 477 

    0x351dc898,// 481 PAY 478 

    0x15a223db,// 482 PAY 479 

    0x69fdb14f,// 483 PAY 480 

    0xa3d17d03,// 484 PAY 481 

    0xbf9ad186,// 485 PAY 482 

    0x8cd9d7f6,// 486 PAY 483 

    0xe83e7f4f,// 487 PAY 484 

    0x53080ab9,// 488 PAY 485 

    0xc49c1ec6,// 489 PAY 486 

    0x5f1df81d,// 490 PAY 487 

    0xd021488b,// 491 PAY 488 

    0x257aa401,// 492 PAY 489 

    0xf34199ed,// 493 PAY 490 

    0xf7c76eda,// 494 PAY 491 

    0x83caac20,// 495 PAY 492 

    0x2118491e,// 496 PAY 493 

    0xc6f82e88,// 497 PAY 494 

    0x605c61e3,// 498 PAY 495 

    0x61ec0000,// 499 PAY 496 

/// STA is 1 words. 

/// STA num_pkts       : 24 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xed 

    0x03dded18 // 500 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt70_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 468 words. 

/// BDA size     is 1866 (0x74a) 

/// BDA id       is 0x8e57 

    0x074a8e57,// 3 BDA   1 

/// PAY Generic Data size   : 1866 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x0a3e80f8,// 4 PAY   1 

    0x675794e5,// 5 PAY   2 

    0xe3ae603a,// 6 PAY   3 

    0x18deeba8,// 7 PAY   4 

    0xfa862d7b,// 8 PAY   5 

    0x7f9472ec,// 9 PAY   6 

    0x5a51afe5,// 10 PAY   7 

    0x180a1b5b,// 11 PAY   8 

    0x201f85f5,// 12 PAY   9 

    0xeeb5484f,// 13 PAY  10 

    0xe397b86a,// 14 PAY  11 

    0xce7dbce2,// 15 PAY  12 

    0xefd1c76b,// 16 PAY  13 

    0x74004e85,// 17 PAY  14 

    0x5a93397d,// 18 PAY  15 

    0x4a8d184b,// 19 PAY  16 

    0xbaf0859c,// 20 PAY  17 

    0xea3f3e51,// 21 PAY  18 

    0x150ef2d3,// 22 PAY  19 

    0x73a00d7b,// 23 PAY  20 

    0xff49544a,// 24 PAY  21 

    0xe36086ca,// 25 PAY  22 

    0x0300176c,// 26 PAY  23 

    0x62167ebf,// 27 PAY  24 

    0x5f97be52,// 28 PAY  25 

    0xea7c10bd,// 29 PAY  26 

    0x77a4bfc1,// 30 PAY  27 

    0xd8c6c480,// 31 PAY  28 

    0x7c3bdf7b,// 32 PAY  29 

    0x1a1cac43,// 33 PAY  30 

    0xfe605f76,// 34 PAY  31 

    0x70b1a8a8,// 35 PAY  32 

    0x6bf6b68d,// 36 PAY  33 

    0x1a386d6e,// 37 PAY  34 

    0x49d2bcb1,// 38 PAY  35 

    0x354a7f9b,// 39 PAY  36 

    0x6594a061,// 40 PAY  37 

    0x177e37b4,// 41 PAY  38 

    0x0f24df63,// 42 PAY  39 

    0xd955d051,// 43 PAY  40 

    0xc9883dad,// 44 PAY  41 

    0x6adc18c5,// 45 PAY  42 

    0x20b9fef3,// 46 PAY  43 

    0x14e386f8,// 47 PAY  44 

    0x900691e7,// 48 PAY  45 

    0x5157d6ca,// 49 PAY  46 

    0xa422db62,// 50 PAY  47 

    0x3041ba11,// 51 PAY  48 

    0x0d2a2259,// 52 PAY  49 

    0x7185159a,// 53 PAY  50 

    0x172f5028,// 54 PAY  51 

    0x22d049f0,// 55 PAY  52 

    0xe042d4c1,// 56 PAY  53 

    0xdca1ec3e,// 57 PAY  54 

    0x47ddb0f5,// 58 PAY  55 

    0x0e98b5f5,// 59 PAY  56 

    0x7ba019f2,// 60 PAY  57 

    0x3c270035,// 61 PAY  58 

    0x396c696b,// 62 PAY  59 

    0x6f4dc860,// 63 PAY  60 

    0x18e57c6b,// 64 PAY  61 

    0xfb2c7163,// 65 PAY  62 

    0xd5b94708,// 66 PAY  63 

    0x0bb83ade,// 67 PAY  64 

    0x1b6cd5de,// 68 PAY  65 

    0xe176877a,// 69 PAY  66 

    0x49b54f04,// 70 PAY  67 

    0x1eaf6a0f,// 71 PAY  68 

    0x73a72007,// 72 PAY  69 

    0xf95b74b5,// 73 PAY  70 

    0x3fe5808b,// 74 PAY  71 

    0x4f14694f,// 75 PAY  72 

    0x4a616166,// 76 PAY  73 

    0x3c302cbd,// 77 PAY  74 

    0xc2350110,// 78 PAY  75 

    0x5233893b,// 79 PAY  76 

    0xb1ee28a3,// 80 PAY  77 

    0x388be92d,// 81 PAY  78 

    0x86df4643,// 82 PAY  79 

    0xe97765f5,// 83 PAY  80 

    0x1b929276,// 84 PAY  81 

    0x71b076be,// 85 PAY  82 

    0xea44479e,// 86 PAY  83 

    0x9ad99973,// 87 PAY  84 

    0x7a33159b,// 88 PAY  85 

    0xf9efaf52,// 89 PAY  86 

    0x9134de94,// 90 PAY  87 

    0xb1b3383e,// 91 PAY  88 

    0xfad291f2,// 92 PAY  89 

    0x28f256a1,// 93 PAY  90 

    0x435be923,// 94 PAY  91 

    0x2e50deac,// 95 PAY  92 

    0x8ecaf827,// 96 PAY  93 

    0x687d1f21,// 97 PAY  94 

    0xbb8dcd98,// 98 PAY  95 

    0x3e6afef4,// 99 PAY  96 

    0xac1741b9,// 100 PAY  97 

    0xd8d64cef,// 101 PAY  98 

    0x6e76cd06,// 102 PAY  99 

    0xadff46ba,// 103 PAY 100 

    0xb0371407,// 104 PAY 101 

    0x4f20b434,// 105 PAY 102 

    0x76263706,// 106 PAY 103 

    0xaad04c69,// 107 PAY 104 

    0x85bc5806,// 108 PAY 105 

    0xc0bdda8c,// 109 PAY 106 

    0xa5d25216,// 110 PAY 107 

    0x1835cd74,// 111 PAY 108 

    0xff1755ea,// 112 PAY 109 

    0x5168c6f3,// 113 PAY 110 

    0x59695415,// 114 PAY 111 

    0x61f610b0,// 115 PAY 112 

    0x6d749f10,// 116 PAY 113 

    0x825ec621,// 117 PAY 114 

    0xdaf2f2b4,// 118 PAY 115 

    0x63d3ddd3,// 119 PAY 116 

    0xcf758e13,// 120 PAY 117 

    0xae7ef4a8,// 121 PAY 118 

    0xf2c7ce3b,// 122 PAY 119 

    0xbeb267ea,// 123 PAY 120 

    0x9f7d3fa3,// 124 PAY 121 

    0x3ea13910,// 125 PAY 122 

    0x7259aaeb,// 126 PAY 123 

    0xfe4b2fd9,// 127 PAY 124 

    0x1ee6732c,// 128 PAY 125 

    0x44de99e0,// 129 PAY 126 

    0x0bf3fa32,// 130 PAY 127 

    0x329fa948,// 131 PAY 128 

    0xf66bc465,// 132 PAY 129 

    0xe80bad9e,// 133 PAY 130 

    0x3eb366cc,// 134 PAY 131 

    0x25b4af18,// 135 PAY 132 

    0x5a2496ea,// 136 PAY 133 

    0xe6da103a,// 137 PAY 134 

    0x0b987f8d,// 138 PAY 135 

    0xc2ac1ad4,// 139 PAY 136 

    0x3c179081,// 140 PAY 137 

    0x7c0aa866,// 141 PAY 138 

    0x81747d01,// 142 PAY 139 

    0x08a8d8a3,// 143 PAY 140 

    0xd2b37d47,// 144 PAY 141 

    0x89283747,// 145 PAY 142 

    0x3be2a085,// 146 PAY 143 

    0x2a01a738,// 147 PAY 144 

    0x896ba867,// 148 PAY 145 

    0xafbecacf,// 149 PAY 146 

    0xae2c30a9,// 150 PAY 147 

    0xc2d74d32,// 151 PAY 148 

    0x5b98cbe0,// 152 PAY 149 

    0xf8d70943,// 153 PAY 150 

    0xa88a55fb,// 154 PAY 151 

    0x9a1f59fa,// 155 PAY 152 

    0xa7c3c53f,// 156 PAY 153 

    0xd219a679,// 157 PAY 154 

    0x1e2aef06,// 158 PAY 155 

    0x7189c4d2,// 159 PAY 156 

    0x9d1f08fe,// 160 PAY 157 

    0xeff20426,// 161 PAY 158 

    0xdb809944,// 162 PAY 159 

    0xaa384c6c,// 163 PAY 160 

    0x41854823,// 164 PAY 161 

    0x617ea640,// 165 PAY 162 

    0x398f4214,// 166 PAY 163 

    0xbfa32f71,// 167 PAY 164 

    0xb2b16a74,// 168 PAY 165 

    0xb71ac2d9,// 169 PAY 166 

    0x55e4c41b,// 170 PAY 167 

    0x15bf2063,// 171 PAY 168 

    0x1567b75a,// 172 PAY 169 

    0x3037114d,// 173 PAY 170 

    0x0b09e608,// 174 PAY 171 

    0x616f4d2a,// 175 PAY 172 

    0xda9cfb36,// 176 PAY 173 

    0x0bde2276,// 177 PAY 174 

    0xd732f25e,// 178 PAY 175 

    0xc72a8fab,// 179 PAY 176 

    0x0a19e79e,// 180 PAY 177 

    0x5031cb0b,// 181 PAY 178 

    0x16b69711,// 182 PAY 179 

    0xe04af3a0,// 183 PAY 180 

    0x57b6b17c,// 184 PAY 181 

    0xf3d21c04,// 185 PAY 182 

    0xe71e516f,// 186 PAY 183 

    0xaf2219ea,// 187 PAY 184 

    0xb06001ce,// 188 PAY 185 

    0xc16d1050,// 189 PAY 186 

    0xfeb8a2a6,// 190 PAY 187 

    0x9d5fe557,// 191 PAY 188 

    0x13003d56,// 192 PAY 189 

    0xa7da9661,// 193 PAY 190 

    0x67ff0e49,// 194 PAY 191 

    0x89c0407b,// 195 PAY 192 

    0xaf164825,// 196 PAY 193 

    0x915840bc,// 197 PAY 194 

    0x30f032b0,// 198 PAY 195 

    0x1d15dabe,// 199 PAY 196 

    0x655c616e,// 200 PAY 197 

    0xa150c3e4,// 201 PAY 198 

    0xb390252e,// 202 PAY 199 

    0x80794160,// 203 PAY 200 

    0xd96bf2c9,// 204 PAY 201 

    0x09088235,// 205 PAY 202 

    0x5832f40e,// 206 PAY 203 

    0xeef94a4b,// 207 PAY 204 

    0x9a43dbc3,// 208 PAY 205 

    0xea619be6,// 209 PAY 206 

    0x8d7bc6ef,// 210 PAY 207 

    0x945b972f,// 211 PAY 208 

    0x4d8c6523,// 212 PAY 209 

    0xeaa281fa,// 213 PAY 210 

    0x72321f6d,// 214 PAY 211 

    0xae2b28cc,// 215 PAY 212 

    0xb3c0c592,// 216 PAY 213 

    0xfb3c1ad0,// 217 PAY 214 

    0xf552d133,// 218 PAY 215 

    0x43498309,// 219 PAY 216 

    0x210cabc9,// 220 PAY 217 

    0xfa4c7656,// 221 PAY 218 

    0xb042350f,// 222 PAY 219 

    0x10132caf,// 223 PAY 220 

    0x906089ab,// 224 PAY 221 

    0x1cdb67b5,// 225 PAY 222 

    0xdfbd2d1b,// 226 PAY 223 

    0x166a11ed,// 227 PAY 224 

    0xbfa3f52d,// 228 PAY 225 

    0x40a38b81,// 229 PAY 226 

    0xbbcf2b0e,// 230 PAY 227 

    0x1b039620,// 231 PAY 228 

    0x11416c7e,// 232 PAY 229 

    0x81d4c306,// 233 PAY 230 

    0x6940c259,// 234 PAY 231 

    0x65ff1a25,// 235 PAY 232 

    0x278b7040,// 236 PAY 233 

    0x208c336c,// 237 PAY 234 

    0xf3c61f25,// 238 PAY 235 

    0xb05c15c6,// 239 PAY 236 

    0x8a099682,// 240 PAY 237 

    0x2a57ea29,// 241 PAY 238 

    0x482860a2,// 242 PAY 239 

    0x32b6e0f3,// 243 PAY 240 

    0xf549e2ba,// 244 PAY 241 

    0xea951c23,// 245 PAY 242 

    0xa6e1c0e4,// 246 PAY 243 

    0x92a5cf89,// 247 PAY 244 

    0xa224c498,// 248 PAY 245 

    0xf04fe31d,// 249 PAY 246 

    0x1a5069ba,// 250 PAY 247 

    0x5297a829,// 251 PAY 248 

    0x2dcdd0da,// 252 PAY 249 

    0xf192ba8a,// 253 PAY 250 

    0x362e8bc4,// 254 PAY 251 

    0x0f0b6ca3,// 255 PAY 252 

    0x006216ec,// 256 PAY 253 

    0x6a01b669,// 257 PAY 254 

    0x05e27b48,// 258 PAY 255 

    0x9d4415f4,// 259 PAY 256 

    0x0ef0f7f5,// 260 PAY 257 

    0xdc2aabab,// 261 PAY 258 

    0xdfc28db4,// 262 PAY 259 

    0x4aaa78f4,// 263 PAY 260 

    0xcfc9903f,// 264 PAY 261 

    0x5363de60,// 265 PAY 262 

    0xded86830,// 266 PAY 263 

    0xb8673ac1,// 267 PAY 264 

    0xf065fe14,// 268 PAY 265 

    0x66ef507c,// 269 PAY 266 

    0xde037e83,// 270 PAY 267 

    0xdff4acb6,// 271 PAY 268 

    0xd2830599,// 272 PAY 269 

    0xd75018e1,// 273 PAY 270 

    0xa997a1dd,// 274 PAY 271 

    0x5384c4db,// 275 PAY 272 

    0x0a5dd3df,// 276 PAY 273 

    0x98275b0e,// 277 PAY 274 

    0xd1cfac36,// 278 PAY 275 

    0xb815a395,// 279 PAY 276 

    0xac8aa5b0,// 280 PAY 277 

    0xfc60c30b,// 281 PAY 278 

    0x0cc68f2f,// 282 PAY 279 

    0xd843e03d,// 283 PAY 280 

    0xc4788323,// 284 PAY 281 

    0x0139a4d7,// 285 PAY 282 

    0x19d812d9,// 286 PAY 283 

    0x5a9d2074,// 287 PAY 284 

    0xbfba43ae,// 288 PAY 285 

    0x9ee6750e,// 289 PAY 286 

    0xf066af2a,// 290 PAY 287 

    0xe39fa706,// 291 PAY 288 

    0xd85db270,// 292 PAY 289 

    0x227f7448,// 293 PAY 290 

    0xeb4a0ebf,// 294 PAY 291 

    0x2ff54b7c,// 295 PAY 292 

    0x809465a2,// 296 PAY 293 

    0x1a6b15d3,// 297 PAY 294 

    0x654e9902,// 298 PAY 295 

    0x11802ee4,// 299 PAY 296 

    0xf095df2f,// 300 PAY 297 

    0xef4e9fcf,// 301 PAY 298 

    0x0407f81c,// 302 PAY 299 

    0x60f61361,// 303 PAY 300 

    0x8706ebd9,// 304 PAY 301 

    0x9b8f1cca,// 305 PAY 302 

    0x8ba323a4,// 306 PAY 303 

    0xadd8897c,// 307 PAY 304 

    0xca7c7298,// 308 PAY 305 

    0x199cb323,// 309 PAY 306 

    0x77a12f48,// 310 PAY 307 

    0x80670793,// 311 PAY 308 

    0x41aa0ff4,// 312 PAY 309 

    0x218e434d,// 313 PAY 310 

    0x57f442fe,// 314 PAY 311 

    0xfe84839b,// 315 PAY 312 

    0xacc134ab,// 316 PAY 313 

    0x628f9411,// 317 PAY 314 

    0x986c823e,// 318 PAY 315 

    0x8e4d1556,// 319 PAY 316 

    0xd949d15b,// 320 PAY 317 

    0xc2e8dc49,// 321 PAY 318 

    0x57f75ec5,// 322 PAY 319 

    0x2f45b4b5,// 323 PAY 320 

    0x4a1f2c07,// 324 PAY 321 

    0x48c899c0,// 325 PAY 322 

    0xc1a0dceb,// 326 PAY 323 

    0x45bb2733,// 327 PAY 324 

    0xaeaa98ad,// 328 PAY 325 

    0x3ee78bfb,// 329 PAY 326 

    0xb958f7b8,// 330 PAY 327 

    0x71d5b057,// 331 PAY 328 

    0x00230550,// 332 PAY 329 

    0x6f329a0e,// 333 PAY 330 

    0xa1aace07,// 334 PAY 331 

    0x1eafadf7,// 335 PAY 332 

    0x10bb8cd3,// 336 PAY 333 

    0xc69faef8,// 337 PAY 334 

    0xba7d3af1,// 338 PAY 335 

    0xd56cf785,// 339 PAY 336 

    0x98bf37d6,// 340 PAY 337 

    0x5432cbf0,// 341 PAY 338 

    0xbcd0d28e,// 342 PAY 339 

    0x5fe5628c,// 343 PAY 340 

    0x0e7a2392,// 344 PAY 341 

    0xccab53fb,// 345 PAY 342 

    0xa7e22f85,// 346 PAY 343 

    0x3e42b333,// 347 PAY 344 

    0x7629c8a3,// 348 PAY 345 

    0x2c7b9c35,// 349 PAY 346 

    0xfe3f2d75,// 350 PAY 347 

    0xde84f7b1,// 351 PAY 348 

    0xa72434b9,// 352 PAY 349 

    0x2d386ffc,// 353 PAY 350 

    0x1464e2c9,// 354 PAY 351 

    0xb65cd263,// 355 PAY 352 

    0xf67ffed5,// 356 PAY 353 

    0xd062f1e1,// 357 PAY 354 

    0x407637d4,// 358 PAY 355 

    0x0c425d1c,// 359 PAY 356 

    0x4b4eb5ea,// 360 PAY 357 

    0xd67b1aef,// 361 PAY 358 

    0x894876b2,// 362 PAY 359 

    0x1cd9b7a8,// 363 PAY 360 

    0x66cf4f86,// 364 PAY 361 

    0x5e492a76,// 365 PAY 362 

    0xf4321795,// 366 PAY 363 

    0xa166c7bf,// 367 PAY 364 

    0x2d085732,// 368 PAY 365 

    0x2dbde8a6,// 369 PAY 366 

    0xed40fc8d,// 370 PAY 367 

    0x80cebae8,// 371 PAY 368 

    0x54054c06,// 372 PAY 369 

    0x5006ef41,// 373 PAY 370 

    0xe2c8da32,// 374 PAY 371 

    0x2e29a15e,// 375 PAY 372 

    0x6d40230e,// 376 PAY 373 

    0xc30138c0,// 377 PAY 374 

    0xecc06039,// 378 PAY 375 

    0x7b81d65d,// 379 PAY 376 

    0xbb8d86a8,// 380 PAY 377 

    0x307f8d2e,// 381 PAY 378 

    0x43d5f2e9,// 382 PAY 379 

    0x48ff563d,// 383 PAY 380 

    0x25cfbc28,// 384 PAY 381 

    0x66ce4e2c,// 385 PAY 382 

    0x6c548a5e,// 386 PAY 383 

    0xd2f1f738,// 387 PAY 384 

    0x5b5ca487,// 388 PAY 385 

    0x3908763f,// 389 PAY 386 

    0xe3a84e42,// 390 PAY 387 

    0x8e9a0a0d,// 391 PAY 388 

    0x4a5a7019,// 392 PAY 389 

    0xa37de945,// 393 PAY 390 

    0x4a42ef0f,// 394 PAY 391 

    0xb731bc1e,// 395 PAY 392 

    0x62f61593,// 396 PAY 393 

    0xcb156624,// 397 PAY 394 

    0x4d479f71,// 398 PAY 395 

    0xde816a81,// 399 PAY 396 

    0xb06390f7,// 400 PAY 397 

    0x1a120f9b,// 401 PAY 398 

    0x6b639eff,// 402 PAY 399 

    0x99173c83,// 403 PAY 400 

    0x27dee4dc,// 404 PAY 401 

    0x128d5fbe,// 405 PAY 402 

    0x09214f51,// 406 PAY 403 

    0x0848ed18,// 407 PAY 404 

    0xee20bf1e,// 408 PAY 405 

    0xc2bab095,// 409 PAY 406 

    0xd27e9c5d,// 410 PAY 407 

    0x4df88dc5,// 411 PAY 408 

    0x41c6b69c,// 412 PAY 409 

    0x60811a36,// 413 PAY 410 

    0xfa9437f7,// 414 PAY 411 

    0xfce42fe0,// 415 PAY 412 

    0xcfeef235,// 416 PAY 413 

    0x2f377636,// 417 PAY 414 

    0x3170a066,// 418 PAY 415 

    0x1cad5933,// 419 PAY 416 

    0x5573e407,// 420 PAY 417 

    0x081d0d77,// 421 PAY 418 

    0xaa9b17ea,// 422 PAY 419 

    0xde5aac99,// 423 PAY 420 

    0x8893b073,// 424 PAY 421 

    0xbdc1bf0b,// 425 PAY 422 

    0x7915d3a9,// 426 PAY 423 

    0xfbd2958b,// 427 PAY 424 

    0x266b4ea1,// 428 PAY 425 

    0x9f8a5b12,// 429 PAY 426 

    0xfb3b382e,// 430 PAY 427 

    0xa45ce2ef,// 431 PAY 428 

    0xd7b3ae39,// 432 PAY 429 

    0x3e88f0d6,// 433 PAY 430 

    0x11d55c58,// 434 PAY 431 

    0x117c4976,// 435 PAY 432 

    0xc2e2917e,// 436 PAY 433 

    0xbf4a0481,// 437 PAY 434 

    0xf5e824fa,// 438 PAY 435 

    0x4c03d36d,// 439 PAY 436 

    0x5b5c1abe,// 440 PAY 437 

    0x0bb93661,// 441 PAY 438 

    0x7332de1a,// 442 PAY 439 

    0x5bf546c5,// 443 PAY 440 

    0x40c3afa3,// 444 PAY 441 

    0x52547c7b,// 445 PAY 442 

    0x7e599fd0,// 446 PAY 443 

    0xe49f9e66,// 447 PAY 444 

    0x91aa75ff,// 448 PAY 445 

    0x3c0f088e,// 449 PAY 446 

    0xf11ccec8,// 450 PAY 447 

    0x9de92855,// 451 PAY 448 

    0x56f9f52a,// 452 PAY 449 

    0x6697aaa9,// 453 PAY 450 

    0x2d91a4f0,// 454 PAY 451 

    0xbd919fb4,// 455 PAY 452 

    0x6dda6a36,// 456 PAY 453 

    0x13ca0e84,// 457 PAY 454 

    0xd81a01ee,// 458 PAY 455 

    0x3ad55e59,// 459 PAY 456 

    0x33795338,// 460 PAY 457 

    0x2346e219,// 461 PAY 458 

    0x112783e9,// 462 PAY 459 

    0xc57b04fe,// 463 PAY 460 

    0x0c603f84,// 464 PAY 461 

    0x9b0bcfc5,// 465 PAY 462 

    0xfbde3c55,// 466 PAY 463 

    0x0eb7b45e,// 467 PAY 464 

    0x0e8faf7b,// 468 PAY 465 

    0x02df0e49,// 469 PAY 466 

    0x14960000,// 470 PAY 467 

/// HASH is  12 bytes 

    0x09088235,// 471 HSH   1 

    0x5832f40e,// 472 HSH   2 

    0xeef94a4b,// 473 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 20 

/// STA pkt_idx        : 132 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x13 

    0x02111314 // 474 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt71_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 339 words. 

/// BDA size     is 1350 (0x546) 

/// BDA id       is 0xc9c9 

    0x0546c9c9,// 3 BDA   1 

/// PAY Generic Data size   : 1350 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xf0da445d,// 4 PAY   1 

    0xf7ba2066,// 5 PAY   2 

    0xe145a31d,// 6 PAY   3 

    0x02db19e0,// 7 PAY   4 

    0x7c316baa,// 8 PAY   5 

    0xbe14d815,// 9 PAY   6 

    0x61e5f84a,// 10 PAY   7 

    0x0deabcf7,// 11 PAY   8 

    0x2c5545df,// 12 PAY   9 

    0xf6fd771c,// 13 PAY  10 

    0x5ec99562,// 14 PAY  11 

    0x73b178f8,// 15 PAY  12 

    0xaf448ec5,// 16 PAY  13 

    0x6bf01503,// 17 PAY  14 

    0xf02c5462,// 18 PAY  15 

    0x19733cca,// 19 PAY  16 

    0xc2e84d71,// 20 PAY  17 

    0x4e015165,// 21 PAY  18 

    0xae608a64,// 22 PAY  19 

    0x046b6bdc,// 23 PAY  20 

    0x03eca4a0,// 24 PAY  21 

    0xbc01920e,// 25 PAY  22 

    0x51e696e8,// 26 PAY  23 

    0x07bcac02,// 27 PAY  24 

    0x313e4714,// 28 PAY  25 

    0x15a5a4f0,// 29 PAY  26 

    0x825991ce,// 30 PAY  27 

    0x52be061e,// 31 PAY  28 

    0x2d41272b,// 32 PAY  29 

    0xcd57b6a4,// 33 PAY  30 

    0x97094558,// 34 PAY  31 

    0xb9d6a156,// 35 PAY  32 

    0x60909b34,// 36 PAY  33 

    0xfac89954,// 37 PAY  34 

    0x9a807624,// 38 PAY  35 

    0x53041191,// 39 PAY  36 

    0xdf004a14,// 40 PAY  37 

    0x5f4b5fd6,// 41 PAY  38 

    0xfa3868dd,// 42 PAY  39 

    0xf707879c,// 43 PAY  40 

    0xdbce37f5,// 44 PAY  41 

    0x75c67f28,// 45 PAY  42 

    0x1adb3ceb,// 46 PAY  43 

    0xa399d93c,// 47 PAY  44 

    0xee16131e,// 48 PAY  45 

    0x159b5d66,// 49 PAY  46 

    0x963dd9ad,// 50 PAY  47 

    0x5c4c73af,// 51 PAY  48 

    0x8864f8ab,// 52 PAY  49 

    0xb1b3b2c7,// 53 PAY  50 

    0xc6c887f8,// 54 PAY  51 

    0x30a2c6ce,// 55 PAY  52 

    0x3c3c7bf6,// 56 PAY  53 

    0x22b60f10,// 57 PAY  54 

    0xe603f780,// 58 PAY  55 

    0x51863e3b,// 59 PAY  56 

    0xe8d19e1f,// 60 PAY  57 

    0x4a923399,// 61 PAY  58 

    0x8e141f08,// 62 PAY  59 

    0x97e10788,// 63 PAY  60 

    0x21e5aebd,// 64 PAY  61 

    0xc1bc9ae3,// 65 PAY  62 

    0x78f2c5b2,// 66 PAY  63 

    0x92aad564,// 67 PAY  64 

    0x1f41c1b4,// 68 PAY  65 

    0xa822231f,// 69 PAY  66 

    0x3f021c0d,// 70 PAY  67 

    0x6abc56fd,// 71 PAY  68 

    0xd0431fac,// 72 PAY  69 

    0x8bb71441,// 73 PAY  70 

    0x0302c2a9,// 74 PAY  71 

    0x4fbed9b3,// 75 PAY  72 

    0x7e64b87f,// 76 PAY  73 

    0xa2ace290,// 77 PAY  74 

    0x9a9c5100,// 78 PAY  75 

    0x5c422b68,// 79 PAY  76 

    0x5b881e1f,// 80 PAY  77 

    0xc2619e8f,// 81 PAY  78 

    0x0a770239,// 82 PAY  79 

    0x6858fcfb,// 83 PAY  80 

    0xafd162d6,// 84 PAY  81 

    0x8ec34452,// 85 PAY  82 

    0xbcce85d8,// 86 PAY  83 

    0x77c1b7cd,// 87 PAY  84 

    0x0e9a4f21,// 88 PAY  85 

    0x28c3d7f0,// 89 PAY  86 

    0xdf3b9cfe,// 90 PAY  87 

    0x5c370029,// 91 PAY  88 

    0x62901b66,// 92 PAY  89 

    0xa05f358a,// 93 PAY  90 

    0x7b5bc37b,// 94 PAY  91 

    0xb1d192a7,// 95 PAY  92 

    0xf2ae481a,// 96 PAY  93 

    0x23455cce,// 97 PAY  94 

    0x49707280,// 98 PAY  95 

    0x87c0496f,// 99 PAY  96 

    0x37bbebf6,// 100 PAY  97 

    0xa79c743b,// 101 PAY  98 

    0x01893203,// 102 PAY  99 

    0x638db109,// 103 PAY 100 

    0x173fb2de,// 104 PAY 101 

    0x25296538,// 105 PAY 102 

    0x589d3279,// 106 PAY 103 

    0x2dd8261b,// 107 PAY 104 

    0xecdd88b6,// 108 PAY 105 

    0x3b91ee07,// 109 PAY 106 

    0xd1f3bd82,// 110 PAY 107 

    0x769ac3c3,// 111 PAY 108 

    0x6afdef3c,// 112 PAY 109 

    0x2efc44da,// 113 PAY 110 

    0x342a8f39,// 114 PAY 111 

    0x640b054a,// 115 PAY 112 

    0x6833da95,// 116 PAY 113 

    0x55704c16,// 117 PAY 114 

    0x1473e804,// 118 PAY 115 

    0xcc4634f7,// 119 PAY 116 

    0xb9dc316b,// 120 PAY 117 

    0x7868e59f,// 121 PAY 118 

    0x8e1943b9,// 122 PAY 119 

    0x44408c5a,// 123 PAY 120 

    0xce372a2c,// 124 PAY 121 

    0x4270a9e3,// 125 PAY 122 

    0x40136ff2,// 126 PAY 123 

    0x2d9accc8,// 127 PAY 124 

    0xb33e6af3,// 128 PAY 125 

    0x12d769f5,// 129 PAY 126 

    0xc223e967,// 130 PAY 127 

    0x3f96c765,// 131 PAY 128 

    0xa5dcfdbd,// 132 PAY 129 

    0xdf253a4a,// 133 PAY 130 

    0x66e2bfd9,// 134 PAY 131 

    0x397f6d04,// 135 PAY 132 

    0xd6edee15,// 136 PAY 133 

    0x6db3c6b3,// 137 PAY 134 

    0xd068f058,// 138 PAY 135 

    0x6530107d,// 139 PAY 136 

    0x5ca5350b,// 140 PAY 137 

    0xed4c9712,// 141 PAY 138 

    0xc11df193,// 142 PAY 139 

    0x392643d3,// 143 PAY 140 

    0x391c2874,// 144 PAY 141 

    0xcdae06fb,// 145 PAY 142 

    0x1d5e658d,// 146 PAY 143 

    0xd66312f1,// 147 PAY 144 

    0x8691e1e8,// 148 PAY 145 

    0x3b96ac41,// 149 PAY 146 

    0xb903a634,// 150 PAY 147 

    0x229f9fc9,// 151 PAY 148 

    0xff1b6ee4,// 152 PAY 149 

    0x12c3d8ac,// 153 PAY 150 

    0x86fd1369,// 154 PAY 151 

    0x21687f78,// 155 PAY 152 

    0x65e0d6bd,// 156 PAY 153 

    0x6e2a500c,// 157 PAY 154 

    0xcb23a2f5,// 158 PAY 155 

    0xeb389ce9,// 159 PAY 156 

    0x6c9f36db,// 160 PAY 157 

    0x1e763011,// 161 PAY 158 

    0x89a23c6b,// 162 PAY 159 

    0xcc788da0,// 163 PAY 160 

    0x9311d320,// 164 PAY 161 

    0xdfdc1948,// 165 PAY 162 

    0xee69d4e8,// 166 PAY 163 

    0xc936ba91,// 167 PAY 164 

    0x8cf6d83f,// 168 PAY 165 

    0x236f720b,// 169 PAY 166 

    0xbb66c8f0,// 170 PAY 167 

    0x86864025,// 171 PAY 168 

    0x73e57f53,// 172 PAY 169 

    0xd8e58daf,// 173 PAY 170 

    0xbaea056a,// 174 PAY 171 

    0x0b9ddb03,// 175 PAY 172 

    0xfeac8fc1,// 176 PAY 173 

    0x13d4b596,// 177 PAY 174 

    0x6d3abdfd,// 178 PAY 175 

    0xc0fface1,// 179 PAY 176 

    0x9ea05069,// 180 PAY 177 

    0xd9bbd059,// 181 PAY 178 

    0x95db126e,// 182 PAY 179 

    0xff2d2f97,// 183 PAY 180 

    0xc239752a,// 184 PAY 181 

    0x1ee3b189,// 185 PAY 182 

    0x6c5495fb,// 186 PAY 183 

    0xd1d389f6,// 187 PAY 184 

    0xd4935ab8,// 188 PAY 185 

    0x3eec86f4,// 189 PAY 186 

    0x8574f0fe,// 190 PAY 187 

    0x39ad9c6c,// 191 PAY 188 

    0x514b5960,// 192 PAY 189 

    0xb42ec15d,// 193 PAY 190 

    0xdc6893d0,// 194 PAY 191 

    0x97cc8c0a,// 195 PAY 192 

    0x02f899a2,// 196 PAY 193 

    0x091095f6,// 197 PAY 194 

    0xf49a613c,// 198 PAY 195 

    0x7fcfd243,// 199 PAY 196 

    0x68f1d1b6,// 200 PAY 197 

    0x112f9dfc,// 201 PAY 198 

    0xb027bc85,// 202 PAY 199 

    0x19bcf2f3,// 203 PAY 200 

    0xab07fc90,// 204 PAY 201 

    0x8edfa472,// 205 PAY 202 

    0xcf5ceee4,// 206 PAY 203 

    0x9859bfb8,// 207 PAY 204 

    0xf865c036,// 208 PAY 205 

    0x2c5ec08c,// 209 PAY 206 

    0x0e908f73,// 210 PAY 207 

    0x8b53fcdd,// 211 PAY 208 

    0x7ee88a36,// 212 PAY 209 

    0x879b28de,// 213 PAY 210 

    0x31da1dbc,// 214 PAY 211 

    0x9ef86a88,// 215 PAY 212 

    0xba7dff39,// 216 PAY 213 

    0x29a3f2b4,// 217 PAY 214 

    0xa4a43996,// 218 PAY 215 

    0x8a66667c,// 219 PAY 216 

    0x774ebce4,// 220 PAY 217 

    0x25cf19ee,// 221 PAY 218 

    0x2e719d40,// 222 PAY 219 

    0xb322285f,// 223 PAY 220 

    0x68a7a368,// 224 PAY 221 

    0xa41171a4,// 225 PAY 222 

    0xe664b1f4,// 226 PAY 223 

    0xbcb722a9,// 227 PAY 224 

    0xbcbaccc9,// 228 PAY 225 

    0x3608a737,// 229 PAY 226 

    0xaa1d0562,// 230 PAY 227 

    0xfbbcb851,// 231 PAY 228 

    0xcd939498,// 232 PAY 229 

    0x9f55c1d0,// 233 PAY 230 

    0xc72a8411,// 234 PAY 231 

    0x4788a1fa,// 235 PAY 232 

    0xa5675d81,// 236 PAY 233 

    0x2d80ba78,// 237 PAY 234 

    0xfbb03e93,// 238 PAY 235 

    0x8c2c62aa,// 239 PAY 236 

    0x1334da85,// 240 PAY 237 

    0x0d93ab48,// 241 PAY 238 

    0x802273e3,// 242 PAY 239 

    0xe90e19d8,// 243 PAY 240 

    0x98cacf94,// 244 PAY 241 

    0xc55c9c7d,// 245 PAY 242 

    0x7d7b9998,// 246 PAY 243 

    0x307ffca8,// 247 PAY 244 

    0x93cd575d,// 248 PAY 245 

    0xc9d6cd72,// 249 PAY 246 

    0x645d0d19,// 250 PAY 247 

    0x1124f5ca,// 251 PAY 248 

    0x1ce3f996,// 252 PAY 249 

    0xf2d03370,// 253 PAY 250 

    0xdf6df113,// 254 PAY 251 

    0xb63eee4a,// 255 PAY 252 

    0xd4b88ac7,// 256 PAY 253 

    0xad0750ff,// 257 PAY 254 

    0xf3473cde,// 258 PAY 255 

    0x91ecac10,// 259 PAY 256 

    0xa702d73f,// 260 PAY 257 

    0x3f549b66,// 261 PAY 258 

    0xfc60a5da,// 262 PAY 259 

    0x83b958cd,// 263 PAY 260 

    0xaab51b17,// 264 PAY 261 

    0xcfba6d60,// 265 PAY 262 

    0x67abbb28,// 266 PAY 263 

    0x2136f018,// 267 PAY 264 

    0x7f3f4b8d,// 268 PAY 265 

    0xb4cb5e88,// 269 PAY 266 

    0xc1ecfe6c,// 270 PAY 267 

    0x3eccaf3e,// 271 PAY 268 

    0xfef819d1,// 272 PAY 269 

    0xa1e18c85,// 273 PAY 270 

    0x173ee4cd,// 274 PAY 271 

    0x2c8cebb7,// 275 PAY 272 

    0x46e9bc2e,// 276 PAY 273 

    0x7ca9f4c7,// 277 PAY 274 

    0xc1235310,// 278 PAY 275 

    0xd75cc299,// 279 PAY 276 

    0xc9109c76,// 280 PAY 277 

    0x0877862d,// 281 PAY 278 

    0x8817f3c5,// 282 PAY 279 

    0x0a50ac28,// 283 PAY 280 

    0x45b45bb8,// 284 PAY 281 

    0xc2383bdf,// 285 PAY 282 

    0xee3f677a,// 286 PAY 283 

    0x75ab78a8,// 287 PAY 284 

    0xc9b58de3,// 288 PAY 285 

    0x9588a6a4,// 289 PAY 286 

    0x5847ce31,// 290 PAY 287 

    0x358a5662,// 291 PAY 288 

    0xbf6a5dff,// 292 PAY 289 

    0x60bf5e6c,// 293 PAY 290 

    0x1d730976,// 294 PAY 291 

    0xb9cb725b,// 295 PAY 292 

    0x7da19edf,// 296 PAY 293 

    0x44073a18,// 297 PAY 294 

    0x3ececb40,// 298 PAY 295 

    0x98a9cdcd,// 299 PAY 296 

    0x313711eb,// 300 PAY 297 

    0x6029f154,// 301 PAY 298 

    0xbd00f233,// 302 PAY 299 

    0x92ee22c0,// 303 PAY 300 

    0x8c3ccdc6,// 304 PAY 301 

    0x320f04bd,// 305 PAY 302 

    0x868ab750,// 306 PAY 303 

    0xecdb4b60,// 307 PAY 304 

    0x86becc5e,// 308 PAY 305 

    0x4f73b2e9,// 309 PAY 306 

    0xe0ab1755,// 310 PAY 307 

    0xaa9d0ac6,// 311 PAY 308 

    0xeaeb94c3,// 312 PAY 309 

    0x770b86cb,// 313 PAY 310 

    0x6e81b8cd,// 314 PAY 311 

    0x8e4f719e,// 315 PAY 312 

    0xfe6ca58c,// 316 PAY 313 

    0xfe9b04fd,// 317 PAY 314 

    0x16feae28,// 318 PAY 315 

    0xe06280d1,// 319 PAY 316 

    0xb8dc7f2e,// 320 PAY 317 

    0x3748e285,// 321 PAY 318 

    0x0d5814dd,// 322 PAY 319 

    0x0e3bedd7,// 323 PAY 320 

    0x83761320,// 324 PAY 321 

    0x0c6c8fae,// 325 PAY 322 

    0x6e7e8cf7,// 326 PAY 323 

    0xc6d2c2b7,// 327 PAY 324 

    0x917b3def,// 328 PAY 325 

    0x94f56429,// 329 PAY 326 

    0x24266930,// 330 PAY 327 

    0x8a20dbf5,// 331 PAY 328 

    0x468a1106,// 332 PAY 329 

    0x0698bd98,// 333 PAY 330 

    0xa5043285,// 334 PAY 331 

    0x6b2a8251,// 335 PAY 332 

    0x2337631c,// 336 PAY 333 

    0xd115a382,// 337 PAY 334 

    0x9d5f7210,// 338 PAY 335 

    0xf4eabbcd,// 339 PAY 336 

    0x4b7f921e,// 340 PAY 337 

    0xc8530000,// 341 PAY 338 

/// HASH is  4 bytes 

    0x73e57f53,// 342 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 85 

/// STA pkt_idx        : 179 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf5 

    0x02ccf555 // 343 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt72_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 480 words. 

/// BDA size     is 1915 (0x77b) 

/// BDA id       is 0xdad4 

    0x077bdad4,// 3 BDA   1 

/// PAY Generic Data size   : 1915 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x599a2ed8,// 4 PAY   1 

    0xd32fc75c,// 5 PAY   2 

    0x317332dc,// 6 PAY   3 

    0x0de57f25,// 7 PAY   4 

    0x589b45e9,// 8 PAY   5 

    0x408766c5,// 9 PAY   6 

    0x38b8f564,// 10 PAY   7 

    0x3968e307,// 11 PAY   8 

    0x4dd7febd,// 12 PAY   9 

    0xa0d60595,// 13 PAY  10 

    0x1653e1ac,// 14 PAY  11 

    0xfe428150,// 15 PAY  12 

    0x4095f8cc,// 16 PAY  13 

    0x2bf28e80,// 17 PAY  14 

    0x7ad5d01f,// 18 PAY  15 

    0x9ac667c8,// 19 PAY  16 

    0x6dce2554,// 20 PAY  17 

    0x6ce476a4,// 21 PAY  18 

    0xa0e82fab,// 22 PAY  19 

    0x0ee756b1,// 23 PAY  20 

    0x835b2fce,// 24 PAY  21 

    0x58dc28f5,// 25 PAY  22 

    0x1a04855f,// 26 PAY  23 

    0x2cfa9fe5,// 27 PAY  24 

    0x2acb638b,// 28 PAY  25 

    0xbab1dba4,// 29 PAY  26 

    0x7f62e4d8,// 30 PAY  27 

    0xe451be7a,// 31 PAY  28 

    0x5dc3cc21,// 32 PAY  29 

    0xd010032b,// 33 PAY  30 

    0xcc46a2cd,// 34 PAY  31 

    0xd1e62e69,// 35 PAY  32 

    0x8ce63fe1,// 36 PAY  33 

    0x2c691697,// 37 PAY  34 

    0x81566733,// 38 PAY  35 

    0x8d115d5e,// 39 PAY  36 

    0x39e127f0,// 40 PAY  37 

    0x1371654c,// 41 PAY  38 

    0x927e3a62,// 42 PAY  39 

    0x29763b23,// 43 PAY  40 

    0xe6729bfc,// 44 PAY  41 

    0x1baf540e,// 45 PAY  42 

    0xc882e604,// 46 PAY  43 

    0xdd45e98a,// 47 PAY  44 

    0x00b89cb4,// 48 PAY  45 

    0x67c55f97,// 49 PAY  46 

    0xcc827b6c,// 50 PAY  47 

    0xb49af594,// 51 PAY  48 

    0x20ddff48,// 52 PAY  49 

    0x3673d702,// 53 PAY  50 

    0x92ca917f,// 54 PAY  51 

    0xfe22320d,// 55 PAY  52 

    0xa3e6af44,// 56 PAY  53 

    0xfdd90ffe,// 57 PAY  54 

    0x80eda8f8,// 58 PAY  55 

    0x774d669b,// 59 PAY  56 

    0x58d42fe8,// 60 PAY  57 

    0x13eaf992,// 61 PAY  58 

    0x925096aa,// 62 PAY  59 

    0x453f900d,// 63 PAY  60 

    0x26fa0728,// 64 PAY  61 

    0x31ace57f,// 65 PAY  62 

    0xeae92119,// 66 PAY  63 

    0x56d12f8b,// 67 PAY  64 

    0x4e5dc674,// 68 PAY  65 

    0xa7bb57ba,// 69 PAY  66 

    0x36471244,// 70 PAY  67 

    0x1bfbdbec,// 71 PAY  68 

    0xb157d828,// 72 PAY  69 

    0xf3ca5a60,// 73 PAY  70 

    0x3057fd33,// 74 PAY  71 

    0x50180e66,// 75 PAY  72 

    0xbb53d57c,// 76 PAY  73 

    0x24ae5e80,// 77 PAY  74 

    0x9e3794e1,// 78 PAY  75 

    0x19fa65d0,// 79 PAY  76 

    0x3fef41fd,// 80 PAY  77 

    0xe217afb2,// 81 PAY  78 

    0x27efb570,// 82 PAY  79 

    0x2de9fe25,// 83 PAY  80 

    0x33aa55dc,// 84 PAY  81 

    0x6e2666b9,// 85 PAY  82 

    0xa5911ab3,// 86 PAY  83 

    0x6e2685d5,// 87 PAY  84 

    0x781efaed,// 88 PAY  85 

    0x9264db0d,// 89 PAY  86 

    0xd615e4de,// 90 PAY  87 

    0x487623bb,// 91 PAY  88 

    0x2a1fd7a2,// 92 PAY  89 

    0x9f902ab6,// 93 PAY  90 

    0x1666ac40,// 94 PAY  91 

    0x958204a5,// 95 PAY  92 

    0xf15d0151,// 96 PAY  93 

    0xc7a2cf44,// 97 PAY  94 

    0x51a5e481,// 98 PAY  95 

    0xc2c4a620,// 99 PAY  96 

    0xb66d68b6,// 100 PAY  97 

    0xfd4ab9c5,// 101 PAY  98 

    0xb1df317f,// 102 PAY  99 

    0x597e6f95,// 103 PAY 100 

    0x8ed55b82,// 104 PAY 101 

    0x83bf0530,// 105 PAY 102 

    0x5b07d7ac,// 106 PAY 103 

    0xc4ac7b5a,// 107 PAY 104 

    0x102c1a2f,// 108 PAY 105 

    0x317eca47,// 109 PAY 106 

    0x6ebec1c4,// 110 PAY 107 

    0xcdeae49c,// 111 PAY 108 

    0xe9379e5b,// 112 PAY 109 

    0xa5744c2f,// 113 PAY 110 

    0x9e866062,// 114 PAY 111 

    0x5abb80f7,// 115 PAY 112 

    0xc95a1666,// 116 PAY 113 

    0x8984a06a,// 117 PAY 114 

    0x0df22017,// 118 PAY 115 

    0x4f2da423,// 119 PAY 116 

    0x0b3b7be8,// 120 PAY 117 

    0x75f6d42e,// 121 PAY 118 

    0x6f0cb94d,// 122 PAY 119 

    0xf9b04fff,// 123 PAY 120 

    0x09c357e5,// 124 PAY 121 

    0x42a357c6,// 125 PAY 122 

    0xd69e9cd1,// 126 PAY 123 

    0x10b58799,// 127 PAY 124 

    0x39fa8472,// 128 PAY 125 

    0xd8be9594,// 129 PAY 126 

    0x09c2311e,// 130 PAY 127 

    0xb5932950,// 131 PAY 128 

    0xebc1b23f,// 132 PAY 129 

    0x67c72715,// 133 PAY 130 

    0xebf30681,// 134 PAY 131 

    0x1a19440d,// 135 PAY 132 

    0xdffac3d0,// 136 PAY 133 

    0xb8c3fad9,// 137 PAY 134 

    0x203f3341,// 138 PAY 135 

    0x26c8abac,// 139 PAY 136 

    0xbc17b327,// 140 PAY 137 

    0x5bacc12b,// 141 PAY 138 

    0xb5138e72,// 142 PAY 139 

    0x0b2ebbb3,// 143 PAY 140 

    0x9d2a08e3,// 144 PAY 141 

    0x7889a8ac,// 145 PAY 142 

    0x178152cb,// 146 PAY 143 

    0xa8a454aa,// 147 PAY 144 

    0x6e06ea3b,// 148 PAY 145 

    0x9957bb6e,// 149 PAY 146 

    0x54741b19,// 150 PAY 147 

    0x668985d6,// 151 PAY 148 

    0xc8001791,// 152 PAY 149 

    0x794403c4,// 153 PAY 150 

    0x725b1e39,// 154 PAY 151 

    0x4c3b0322,// 155 PAY 152 

    0xdaa953b8,// 156 PAY 153 

    0x981fad1e,// 157 PAY 154 

    0xecb36528,// 158 PAY 155 

    0xf0f3e034,// 159 PAY 156 

    0x13652866,// 160 PAY 157 

    0xc1b33731,// 161 PAY 158 

    0xd6adb163,// 162 PAY 159 

    0x75eca1ba,// 163 PAY 160 

    0xc0a31fe9,// 164 PAY 161 

    0x5a25d85a,// 165 PAY 162 

    0xe149c162,// 166 PAY 163 

    0x6ebe48bf,// 167 PAY 164 

    0xc2a4c04d,// 168 PAY 165 

    0xe886f340,// 169 PAY 166 

    0x3fc7641d,// 170 PAY 167 

    0xf0dc0282,// 171 PAY 168 

    0x0d411be4,// 172 PAY 169 

    0xa43fa97b,// 173 PAY 170 

    0xe0d1eec0,// 174 PAY 171 

    0xf296717a,// 175 PAY 172 

    0x65f2f24c,// 176 PAY 173 

    0xf8c26950,// 177 PAY 174 

    0x04ae87db,// 178 PAY 175 

    0x047f75e3,// 179 PAY 176 

    0x9416cb0c,// 180 PAY 177 

    0xe1de851b,// 181 PAY 178 

    0xc2733fa8,// 182 PAY 179 

    0xb33c7819,// 183 PAY 180 

    0x186a7b16,// 184 PAY 181 

    0x0ad993cd,// 185 PAY 182 

    0x7e5a5310,// 186 PAY 183 

    0x860e25eb,// 187 PAY 184 

    0x784f108b,// 188 PAY 185 

    0x9ce717d8,// 189 PAY 186 

    0x2fb8bf22,// 190 PAY 187 

    0x7f3794d9,// 191 PAY 188 

    0xb0a02aec,// 192 PAY 189 

    0x98b5a107,// 193 PAY 190 

    0x17eede20,// 194 PAY 191 

    0x39556c13,// 195 PAY 192 

    0x6cf73a9c,// 196 PAY 193 

    0x1cb33009,// 197 PAY 194 

    0xb36b2337,// 198 PAY 195 

    0x633550f1,// 199 PAY 196 

    0xd2cdbf6b,// 200 PAY 197 

    0xb62cd6f1,// 201 PAY 198 

    0xb52bca94,// 202 PAY 199 

    0xc2fa7277,// 203 PAY 200 

    0x7d685118,// 204 PAY 201 

    0xfdc77db7,// 205 PAY 202 

    0x0d697842,// 206 PAY 203 

    0x666066f0,// 207 PAY 204 

    0x2b166ff9,// 208 PAY 205 

    0x792596ba,// 209 PAY 206 

    0xc941b66b,// 210 PAY 207 

    0x59586393,// 211 PAY 208 

    0x52c8b417,// 212 PAY 209 

    0x8ae6d2c1,// 213 PAY 210 

    0x44aa6d88,// 214 PAY 211 

    0x2a3745e0,// 215 PAY 212 

    0x0ab5f4e1,// 216 PAY 213 

    0x300040f1,// 217 PAY 214 

    0xe5a1e34f,// 218 PAY 215 

    0x3964a54b,// 219 PAY 216 

    0x282b630a,// 220 PAY 217 

    0xd39ddc97,// 221 PAY 218 

    0xcea0b2af,// 222 PAY 219 

    0xf4aec525,// 223 PAY 220 

    0x8ff12767,// 224 PAY 221 

    0x30ced89f,// 225 PAY 222 

    0x9648745e,// 226 PAY 223 

    0x240f0d56,// 227 PAY 224 

    0x92f50597,// 228 PAY 225 

    0x6ba2d1a5,// 229 PAY 226 

    0x973739ff,// 230 PAY 227 

    0xcad24cba,// 231 PAY 228 

    0xc6aac81a,// 232 PAY 229 

    0xfa651065,// 233 PAY 230 

    0x824008f2,// 234 PAY 231 

    0x5af4ba8a,// 235 PAY 232 

    0xf86ca7de,// 236 PAY 233 

    0xbfe2b849,// 237 PAY 234 

    0xebcba6c3,// 238 PAY 235 

    0x3f1c530c,// 239 PAY 236 

    0xe6d7b8ed,// 240 PAY 237 

    0x44820cdf,// 241 PAY 238 

    0x082419ba,// 242 PAY 239 

    0x71b08d7e,// 243 PAY 240 

    0xafc2478a,// 244 PAY 241 

    0x60ec8482,// 245 PAY 242 

    0xe3947f28,// 246 PAY 243 

    0x0933f155,// 247 PAY 244 

    0x9f3f0a7a,// 248 PAY 245 

    0x660f5226,// 249 PAY 246 

    0x6cd26d2b,// 250 PAY 247 

    0x3ff158a8,// 251 PAY 248 

    0xe134f01d,// 252 PAY 249 

    0x51fd036b,// 253 PAY 250 

    0x803364d1,// 254 PAY 251 

    0xa0381744,// 255 PAY 252 

    0x96594988,// 256 PAY 253 

    0x0cead9c0,// 257 PAY 254 

    0x9f8f7033,// 258 PAY 255 

    0xa871c9bc,// 259 PAY 256 

    0x5655fc46,// 260 PAY 257 

    0x55cdbf54,// 261 PAY 258 

    0x5b2193fb,// 262 PAY 259 

    0xda924f83,// 263 PAY 260 

    0xff5fe806,// 264 PAY 261 

    0x010ce3b8,// 265 PAY 262 

    0x2f4e0d90,// 266 PAY 263 

    0x3050b54c,// 267 PAY 264 

    0xe5b7414e,// 268 PAY 265 

    0x75a39ec6,// 269 PAY 266 

    0xccfd1028,// 270 PAY 267 

    0xe324fdd8,// 271 PAY 268 

    0xbe925b09,// 272 PAY 269 

    0xb3356859,// 273 PAY 270 

    0x2981b477,// 274 PAY 271 

    0x712a3aa4,// 275 PAY 272 

    0x07394cfc,// 276 PAY 273 

    0x91aff1b3,// 277 PAY 274 

    0x3f4888cb,// 278 PAY 275 

    0x56403f25,// 279 PAY 276 

    0x9e92171e,// 280 PAY 277 

    0xefa85b9b,// 281 PAY 278 

    0x326fe355,// 282 PAY 279 

    0x6ceb6e20,// 283 PAY 280 

    0x778cb3e7,// 284 PAY 281 

    0xd36b9772,// 285 PAY 282 

    0x602891c6,// 286 PAY 283 

    0xc207658d,// 287 PAY 284 

    0x3d212ead,// 288 PAY 285 

    0xa7d619f2,// 289 PAY 286 

    0x2616e394,// 290 PAY 287 

    0xbfe7ad24,// 291 PAY 288 

    0x8e6d0bac,// 292 PAY 289 

    0x5d00317d,// 293 PAY 290 

    0x6f48a1f7,// 294 PAY 291 

    0xe2ce266b,// 295 PAY 292 

    0x4ff4a92b,// 296 PAY 293 

    0x1cfd7fd4,// 297 PAY 294 

    0xcb2cba90,// 298 PAY 295 

    0x4251e762,// 299 PAY 296 

    0x6db075a9,// 300 PAY 297 

    0xd6bbc8af,// 301 PAY 298 

    0x177e284b,// 302 PAY 299 

    0x75d3d8bb,// 303 PAY 300 

    0x899938d6,// 304 PAY 301 

    0x1f374566,// 305 PAY 302 

    0xcd5d78ef,// 306 PAY 303 

    0x1ad7c31a,// 307 PAY 304 

    0xefcafd26,// 308 PAY 305 

    0x4c8b7e68,// 309 PAY 306 

    0xa2c38a6a,// 310 PAY 307 

    0xa9a9ba39,// 311 PAY 308 

    0x964d0e83,// 312 PAY 309 

    0x7281077b,// 313 PAY 310 

    0x33a7509b,// 314 PAY 311 

    0xc187ca2a,// 315 PAY 312 

    0x531e7512,// 316 PAY 313 

    0x9510a407,// 317 PAY 314 

    0x626a7b3b,// 318 PAY 315 

    0xa9069a01,// 319 PAY 316 

    0xf0f01534,// 320 PAY 317 

    0x19841239,// 321 PAY 318 

    0x55af6c9e,// 322 PAY 319 

    0x0b1c1f67,// 323 PAY 320 

    0xd3af9e49,// 324 PAY 321 

    0x52e5f12f,// 325 PAY 322 

    0xf3f3c25c,// 326 PAY 323 

    0xf315d661,// 327 PAY 324 

    0xea3e6ff1,// 328 PAY 325 

    0x71a61728,// 329 PAY 326 

    0xbfcd3013,// 330 PAY 327 

    0x295cf77e,// 331 PAY 328 

    0x2a4b5993,// 332 PAY 329 

    0x59ee121c,// 333 PAY 330 

    0xbafaf42f,// 334 PAY 331 

    0xc13a386f,// 335 PAY 332 

    0xcbf81839,// 336 PAY 333 

    0x1202e478,// 337 PAY 334 

    0xe52ed787,// 338 PAY 335 

    0x5cd180cf,// 339 PAY 336 

    0xe7f4ff41,// 340 PAY 337 

    0x4afb77e7,// 341 PAY 338 

    0xfa6c750a,// 342 PAY 339 

    0x85e86d16,// 343 PAY 340 

    0xc3075d99,// 344 PAY 341 

    0xcfe904bd,// 345 PAY 342 

    0x3b09a799,// 346 PAY 343 

    0x74365f0b,// 347 PAY 344 

    0x53cd3f46,// 348 PAY 345 

    0x171e7ea4,// 349 PAY 346 

    0x7bb99730,// 350 PAY 347 

    0x51d3bd5c,// 351 PAY 348 

    0x06709dc4,// 352 PAY 349 

    0x6f817624,// 353 PAY 350 

    0x92b0c597,// 354 PAY 351 

    0x23028c89,// 355 PAY 352 

    0x1ff4511d,// 356 PAY 353 

    0xccd13f00,// 357 PAY 354 

    0xeb2dd85c,// 358 PAY 355 

    0xe548e0b5,// 359 PAY 356 

    0x7fc4701f,// 360 PAY 357 

    0x5e45c00e,// 361 PAY 358 

    0x3bd5b440,// 362 PAY 359 

    0xf8c8c604,// 363 PAY 360 

    0x0b1128d0,// 364 PAY 361 

    0x8ceee55c,// 365 PAY 362 

    0xcb141d69,// 366 PAY 363 

    0x18058c48,// 367 PAY 364 

    0x9f9869e1,// 368 PAY 365 

    0x5f911163,// 369 PAY 366 

    0xd8780c8b,// 370 PAY 367 

    0x7fe659e6,// 371 PAY 368 

    0x14c8c60a,// 372 PAY 369 

    0xf6b29964,// 373 PAY 370 

    0x9e241d9c,// 374 PAY 371 

    0x9e9a56df,// 375 PAY 372 

    0xe47dc8ca,// 376 PAY 373 

    0xb1e5bfbe,// 377 PAY 374 

    0x376eca92,// 378 PAY 375 

    0xf3d1c871,// 379 PAY 376 

    0xf97929dd,// 380 PAY 377 

    0xdfee3c6d,// 381 PAY 378 

    0x4e0042dc,// 382 PAY 379 

    0x0c1f719a,// 383 PAY 380 

    0x679176bf,// 384 PAY 381 

    0x30fa37f5,// 385 PAY 382 

    0xfcdd8402,// 386 PAY 383 

    0xdf1408da,// 387 PAY 384 

    0xfbc7b1a3,// 388 PAY 385 

    0xe9aa9077,// 389 PAY 386 

    0xa96aa20e,// 390 PAY 387 

    0xa079a07d,// 391 PAY 388 

    0xeaa16f0e,// 392 PAY 389 

    0x1128946e,// 393 PAY 390 

    0x5f1e4f1c,// 394 PAY 391 

    0x7bb2a5a2,// 395 PAY 392 

    0x2080b388,// 396 PAY 393 

    0xec76033f,// 397 PAY 394 

    0x7ce6eff4,// 398 PAY 395 

    0xfca6f19a,// 399 PAY 396 

    0x435fc981,// 400 PAY 397 

    0x1a611e3c,// 401 PAY 398 

    0x73849c25,// 402 PAY 399 

    0xe8e5e7d0,// 403 PAY 400 

    0xf490c220,// 404 PAY 401 

    0x4c4745b9,// 405 PAY 402 

    0x24c61166,// 406 PAY 403 

    0xc800af45,// 407 PAY 404 

    0x6be0d618,// 408 PAY 405 

    0xb37f4457,// 409 PAY 406 

    0xd5e50f15,// 410 PAY 407 

    0xaec8922d,// 411 PAY 408 

    0x71afac5d,// 412 PAY 409 

    0x985ef063,// 413 PAY 410 

    0x7f569580,// 414 PAY 411 

    0xf89b913e,// 415 PAY 412 

    0xe1533ac1,// 416 PAY 413 

    0x9854d3d0,// 417 PAY 414 

    0x7696452d,// 418 PAY 415 

    0xa4b5dbbe,// 419 PAY 416 

    0xe631e3d9,// 420 PAY 417 

    0x1cf5bff3,// 421 PAY 418 

    0xce2b4a3b,// 422 PAY 419 

    0xee34abb3,// 423 PAY 420 

    0x20fa5b17,// 424 PAY 421 

    0x96ae18b6,// 425 PAY 422 

    0x98f1c547,// 426 PAY 423 

    0x6b57e814,// 427 PAY 424 

    0x653fcdb3,// 428 PAY 425 

    0xdc58b79a,// 429 PAY 426 

    0xc96ce362,// 430 PAY 427 

    0x9787a7f6,// 431 PAY 428 

    0x6568f0a8,// 432 PAY 429 

    0x3ed91445,// 433 PAY 430 

    0xa9295c8e,// 434 PAY 431 

    0x768f1049,// 435 PAY 432 

    0x7a473efe,// 436 PAY 433 

    0x485afca0,// 437 PAY 434 

    0xe50bbd83,// 438 PAY 435 

    0x5a00ee4f,// 439 PAY 436 

    0x0e864f99,// 440 PAY 437 

    0x366bdf40,// 441 PAY 438 

    0x26a268c2,// 442 PAY 439 

    0x1b383a09,// 443 PAY 440 

    0xd5ba9e62,// 444 PAY 441 

    0x0913b3bf,// 445 PAY 442 

    0x05dfe275,// 446 PAY 443 

    0xc8f7ea98,// 447 PAY 444 

    0x3d13946e,// 448 PAY 445 

    0x32a3715d,// 449 PAY 446 

    0x08ba9a12,// 450 PAY 447 

    0xe528e39f,// 451 PAY 448 

    0x881c2aca,// 452 PAY 449 

    0x0664f812,// 453 PAY 450 

    0x97bf68b6,// 454 PAY 451 

    0x2d3eb3b7,// 455 PAY 452 

    0xdb682776,// 456 PAY 453 

    0x622e6119,// 457 PAY 454 

    0x861dee8c,// 458 PAY 455 

    0x2476a4e9,// 459 PAY 456 

    0x2437949d,// 460 PAY 457 

    0x86996ce6,// 461 PAY 458 

    0x4f0a2d0a,// 462 PAY 459 

    0x4eb5f609,// 463 PAY 460 

    0x2333b02f,// 464 PAY 461 

    0x009ce838,// 465 PAY 462 

    0x460f8b08,// 466 PAY 463 

    0x79f9703d,// 467 PAY 464 

    0x68f0dc0d,// 468 PAY 465 

    0x3b02a255,// 469 PAY 466 

    0x8219232d,// 470 PAY 467 

    0xd04c03c7,// 471 PAY 468 

    0x047fc25f,// 472 PAY 469 

    0x4b98583b,// 473 PAY 470 

    0x1920c326,// 474 PAY 471 

    0x497a2e36,// 475 PAY 472 

    0x86955a6b,// 476 PAY 473 

    0x54016eeb,// 477 PAY 474 

    0x395c3882,// 478 PAY 475 

    0xc30fdfd2,// 479 PAY 476 

    0x375ffcf4,// 480 PAY 477 

    0x8292e3f2,// 481 PAY 478 

    0x2d287900,// 482 PAY 479 

/// HASH is  20 bytes 

    0xea3e6ff1,// 483 HSH   1 

    0x71a61728,// 484 HSH   2 

    0xbfcd3013,// 485 HSH   3 

    0x295cf77e,// 486 HSH   4 

    0x2a4b5993,// 487 HSH   5 

/// STA is 1 words. 

/// STA num_pkts       : 44 

/// STA pkt_idx        : 45 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x94 

    0x00b5942c // 488 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt73_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 284 words. 

/// BDA size     is 1129 (0x469) 

/// BDA id       is 0xb041 

    0x0469b041,// 3 BDA   1 

/// PAY Generic Data size   : 1129 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x344cbe4f,// 4 PAY   1 

    0xd3c65ad1,// 5 PAY   2 

    0x7019016f,// 6 PAY   3 

    0xa80643cf,// 7 PAY   4 

    0xf59b8586,// 8 PAY   5 

    0x875480de,// 9 PAY   6 

    0xc8392996,// 10 PAY   7 

    0xa268d1de,// 11 PAY   8 

    0xcffb293f,// 12 PAY   9 

    0x0383b8b5,// 13 PAY  10 

    0xcd0f788a,// 14 PAY  11 

    0x46879c46,// 15 PAY  12 

    0x70c4d894,// 16 PAY  13 

    0x3020ce6b,// 17 PAY  14 

    0x6d49ff2e,// 18 PAY  15 

    0x22d33768,// 19 PAY  16 

    0xf079e834,// 20 PAY  17 

    0x6a862af7,// 21 PAY  18 

    0x2519fa9f,// 22 PAY  19 

    0x0403fd0f,// 23 PAY  20 

    0xba535edc,// 24 PAY  21 

    0x84fb404d,// 25 PAY  22 

    0xd6e94104,// 26 PAY  23 

    0xdc387a2a,// 27 PAY  24 

    0x7f1d2697,// 28 PAY  25 

    0x725893c1,// 29 PAY  26 

    0x462b0ece,// 30 PAY  27 

    0xab12d90f,// 31 PAY  28 

    0x694a31bb,// 32 PAY  29 

    0xfdb92830,// 33 PAY  30 

    0xea99238e,// 34 PAY  31 

    0xfaaab101,// 35 PAY  32 

    0xfd90757c,// 36 PAY  33 

    0xa7d51527,// 37 PAY  34 

    0xf9eef21c,// 38 PAY  35 

    0x03a2c37a,// 39 PAY  36 

    0xae0306eb,// 40 PAY  37 

    0x2564cb70,// 41 PAY  38 

    0x03ae1cfa,// 42 PAY  39 

    0x6ef32654,// 43 PAY  40 

    0xd61e1006,// 44 PAY  41 

    0x8c4a023a,// 45 PAY  42 

    0x247e4037,// 46 PAY  43 

    0x7e480c8a,// 47 PAY  44 

    0x5e6c6db0,// 48 PAY  45 

    0x3e758296,// 49 PAY  46 

    0xc74b4dd1,// 50 PAY  47 

    0xe957a5fb,// 51 PAY  48 

    0xd8e4a9fc,// 52 PAY  49 

    0x2ae8dd0b,// 53 PAY  50 

    0xc079da24,// 54 PAY  51 

    0x72bbfc8a,// 55 PAY  52 

    0x82ed0695,// 56 PAY  53 

    0x7b7a8687,// 57 PAY  54 

    0xf1c1baf9,// 58 PAY  55 

    0x740eae09,// 59 PAY  56 

    0xf019e21c,// 60 PAY  57 

    0x4489d392,// 61 PAY  58 

    0x1dc9b595,// 62 PAY  59 

    0x55b0ac3d,// 63 PAY  60 

    0xfda80d5f,// 64 PAY  61 

    0x1ddcdd4f,// 65 PAY  62 

    0x8847152e,// 66 PAY  63 

    0x10f9571b,// 67 PAY  64 

    0x395cc47a,// 68 PAY  65 

    0x3f87cdb7,// 69 PAY  66 

    0xed518d44,// 70 PAY  67 

    0x197a5f11,// 71 PAY  68 

    0x83e6d658,// 72 PAY  69 

    0x3ab47ce6,// 73 PAY  70 

    0x9cbe7201,// 74 PAY  71 

    0x38bcef26,// 75 PAY  72 

    0x5c9899ee,// 76 PAY  73 

    0x63c8cd34,// 77 PAY  74 

    0x07d6d454,// 78 PAY  75 

    0x18b4fbd2,// 79 PAY  76 

    0x6e54f1ac,// 80 PAY  77 

    0x78b10003,// 81 PAY  78 

    0x9c1bfe92,// 82 PAY  79 

    0x2220db58,// 83 PAY  80 

    0x1fb25112,// 84 PAY  81 

    0xaf1350bd,// 85 PAY  82 

    0x20f4a1f2,// 86 PAY  83 

    0xe880e737,// 87 PAY  84 

    0x506ee528,// 88 PAY  85 

    0x9709f76b,// 89 PAY  86 

    0xd1facfec,// 90 PAY  87 

    0x73096481,// 91 PAY  88 

    0xe5c739ed,// 92 PAY  89 

    0x5332fa59,// 93 PAY  90 

    0x62876828,// 94 PAY  91 

    0x92f1f303,// 95 PAY  92 

    0x79637ae3,// 96 PAY  93 

    0xe57aefe3,// 97 PAY  94 

    0xf2e90bfa,// 98 PAY  95 

    0xf48920d1,// 99 PAY  96 

    0xf2261e8e,// 100 PAY  97 

    0x06ac90fb,// 101 PAY  98 

    0xf3389ca4,// 102 PAY  99 

    0x2b493141,// 103 PAY 100 

    0xe275378f,// 104 PAY 101 

    0x9f44b688,// 105 PAY 102 

    0xd20462cb,// 106 PAY 103 

    0xe52eb83e,// 107 PAY 104 

    0xa27e106d,// 108 PAY 105 

    0x800fe20c,// 109 PAY 106 

    0xdb4189b6,// 110 PAY 107 

    0xfe452be9,// 111 PAY 108 

    0x6f01a475,// 112 PAY 109 

    0x3cea356f,// 113 PAY 110 

    0x7233cafa,// 114 PAY 111 

    0x0ff0b9c6,// 115 PAY 112 

    0x8f8e60ab,// 116 PAY 113 

    0x24714ba7,// 117 PAY 114 

    0xdc0697fe,// 118 PAY 115 

    0x8380bf92,// 119 PAY 116 

    0xba358085,// 120 PAY 117 

    0xa4dd7a1c,// 121 PAY 118 

    0xc29a9da7,// 122 PAY 119 

    0x01312085,// 123 PAY 120 

    0x9f18aadf,// 124 PAY 121 

    0x8f924406,// 125 PAY 122 

    0x81ab79af,// 126 PAY 123 

    0xca0efd89,// 127 PAY 124 

    0x2c3568a9,// 128 PAY 125 

    0x7f9da731,// 129 PAY 126 

    0xafbd3865,// 130 PAY 127 

    0x60220cf6,// 131 PAY 128 

    0x7191ead8,// 132 PAY 129 

    0xf1778e43,// 133 PAY 130 

    0x54704fff,// 134 PAY 131 

    0x8886b4c6,// 135 PAY 132 

    0x87062bb0,// 136 PAY 133 

    0x16787071,// 137 PAY 134 

    0xee1fefb9,// 138 PAY 135 

    0x311fde05,// 139 PAY 136 

    0x699bda3c,// 140 PAY 137 

    0x7ebb60ae,// 141 PAY 138 

    0xd44c3cad,// 142 PAY 139 

    0xb8a9ea69,// 143 PAY 140 

    0xfd67a068,// 144 PAY 141 

    0xa89e6325,// 145 PAY 142 

    0x25c0aba8,// 146 PAY 143 

    0x7026efd2,// 147 PAY 144 

    0xb87c8ef0,// 148 PAY 145 

    0xadc41a98,// 149 PAY 146 

    0x90b1d040,// 150 PAY 147 

    0xcfd156d9,// 151 PAY 148 

    0x20598222,// 152 PAY 149 

    0x8553f289,// 153 PAY 150 

    0xb70d46d7,// 154 PAY 151 

    0x24fae6fd,// 155 PAY 152 

    0xc91c07f9,// 156 PAY 153 

    0xd7021beb,// 157 PAY 154 

    0x154e5022,// 158 PAY 155 

    0xd167a9c7,// 159 PAY 156 

    0xa7322074,// 160 PAY 157 

    0x92d5d6a2,// 161 PAY 158 

    0xaa9e64fd,// 162 PAY 159 

    0xc0b64a48,// 163 PAY 160 

    0xe466719a,// 164 PAY 161 

    0x7568733f,// 165 PAY 162 

    0x22670696,// 166 PAY 163 

    0x463d9c66,// 167 PAY 164 

    0x46e824ac,// 168 PAY 165 

    0xcfaf32cd,// 169 PAY 166 

    0x52728a83,// 170 PAY 167 

    0xda19917e,// 171 PAY 168 

    0xf8d782f7,// 172 PAY 169 

    0x1c9f370d,// 173 PAY 170 

    0xee2f8891,// 174 PAY 171 

    0xdc4dfa28,// 175 PAY 172 

    0x15d9a557,// 176 PAY 173 

    0xaf7747d0,// 177 PAY 174 

    0xa8b854f6,// 178 PAY 175 

    0x634aceac,// 179 PAY 176 

    0xcc4e6971,// 180 PAY 177 

    0x8c79c1f7,// 181 PAY 178 

    0x122a2bff,// 182 PAY 179 

    0xa2d6f5cd,// 183 PAY 180 

    0xa6046036,// 184 PAY 181 

    0xab05865d,// 185 PAY 182 

    0x2b5dc5f2,// 186 PAY 183 

    0xd24118ae,// 187 PAY 184 

    0xc89d1408,// 188 PAY 185 

    0x9f889d83,// 189 PAY 186 

    0xbfc95864,// 190 PAY 187 

    0xa85ad47e,// 191 PAY 188 

    0x00519353,// 192 PAY 189 

    0xf0f67131,// 193 PAY 190 

    0xe898f752,// 194 PAY 191 

    0x84b29145,// 195 PAY 192 

    0x89e395fb,// 196 PAY 193 

    0x12d12a40,// 197 PAY 194 

    0x3e3b5045,// 198 PAY 195 

    0x1b819aa9,// 199 PAY 196 

    0xdfdd3fb2,// 200 PAY 197 

    0xb038e4e7,// 201 PAY 198 

    0x4af1c2ce,// 202 PAY 199 

    0x3eaa5454,// 203 PAY 200 

    0x8d87c8fb,// 204 PAY 201 

    0x92c58333,// 205 PAY 202 

    0x62e166d8,// 206 PAY 203 

    0xaf5e09d1,// 207 PAY 204 

    0xab980f01,// 208 PAY 205 

    0x0e5498ba,// 209 PAY 206 

    0xf4dd4325,// 210 PAY 207 

    0x82de3936,// 211 PAY 208 

    0xd2a2cd20,// 212 PAY 209 

    0x389737ee,// 213 PAY 210 

    0x013f44e8,// 214 PAY 211 

    0x8f4c7f2f,// 215 PAY 212 

    0x9e9e7669,// 216 PAY 213 

    0x90eef5d1,// 217 PAY 214 

    0x911be171,// 218 PAY 215 

    0xd13e73d9,// 219 PAY 216 

    0xb39961b2,// 220 PAY 217 

    0xf40257f5,// 221 PAY 218 

    0xa1fa7760,// 222 PAY 219 

    0xa27593fe,// 223 PAY 220 

    0x1392fe83,// 224 PAY 221 

    0x6d70a643,// 225 PAY 222 

    0xc1f0288c,// 226 PAY 223 

    0x2f82e4ce,// 227 PAY 224 

    0x759df7db,// 228 PAY 225 

    0xa10ea940,// 229 PAY 226 

    0xd30b682f,// 230 PAY 227 

    0x3cd26752,// 231 PAY 228 

    0x77c0e9a2,// 232 PAY 229 

    0xe7e99c88,// 233 PAY 230 

    0x435d6722,// 234 PAY 231 

    0x52294bf1,// 235 PAY 232 

    0x17eadfeb,// 236 PAY 233 

    0xf5e893e0,// 237 PAY 234 

    0xd679e0cb,// 238 PAY 235 

    0xd582899f,// 239 PAY 236 

    0x358b7728,// 240 PAY 237 

    0x79929a8b,// 241 PAY 238 

    0xe0027ef2,// 242 PAY 239 

    0x80e4ff19,// 243 PAY 240 

    0x26abd241,// 244 PAY 241 

    0xb86eb277,// 245 PAY 242 

    0xcacc23b7,// 246 PAY 243 

    0x182cab05,// 247 PAY 244 

    0xadc2f6af,// 248 PAY 245 

    0x014ae877,// 249 PAY 246 

    0x3a15a769,// 250 PAY 247 

    0x17f3370d,// 251 PAY 248 

    0x0225224b,// 252 PAY 249 

    0x76f1ec70,// 253 PAY 250 

    0x922d03f0,// 254 PAY 251 

    0x397369e0,// 255 PAY 252 

    0xc98ea5b1,// 256 PAY 253 

    0x1733fd32,// 257 PAY 254 

    0x5efbd610,// 258 PAY 255 

    0x30597265,// 259 PAY 256 

    0xd6bff066,// 260 PAY 257 

    0xd8ef0e39,// 261 PAY 258 

    0xee4d8190,// 262 PAY 259 

    0xdaff2188,// 263 PAY 260 

    0x0f02beda,// 264 PAY 261 

    0x5f6a1e4b,// 265 PAY 262 

    0xfade5b34,// 266 PAY 263 

    0x237ff968,// 267 PAY 264 

    0x142971e8,// 268 PAY 265 

    0x15577223,// 269 PAY 266 

    0x9c355832,// 270 PAY 267 

    0xf0e479d8,// 271 PAY 268 

    0xb242408f,// 272 PAY 269 

    0x8288bcc1,// 273 PAY 270 

    0x6df575ce,// 274 PAY 271 

    0x8c2c1a55,// 275 PAY 272 

    0xeaa3d2d4,// 276 PAY 273 

    0xc07a6535,// 277 PAY 274 

    0x8d8dc46b,// 278 PAY 275 

    0x0ac71446,// 279 PAY 276 

    0x2f43836b,// 280 PAY 277 

    0xc9b79406,// 281 PAY 278 

    0x7df053cf,// 282 PAY 279 

    0x69e614cd,// 283 PAY 280 

    0x8886fe25,// 284 PAY 281 

    0x8da3a393,// 285 PAY 282 

    0x87000000,// 286 PAY 283 

/// HASH is  8 bytes 

    0x8c2c1a55,// 287 HSH   1 

    0xeaa3d2d4,// 288 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 218 

/// STA pkt_idx        : 241 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x44 

    0x03c544da // 289 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt74_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 235 words. 

/// BDA size     is 935 (0x3a7) 

/// BDA id       is 0xa0be 

    0x03a7a0be,// 3 BDA   1 

/// PAY Generic Data size   : 935 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x6e5bfae0,// 4 PAY   1 

    0x7943758a,// 5 PAY   2 

    0x9143ef99,// 6 PAY   3 

    0x1bb70b28,// 7 PAY   4 

    0x69c23c78,// 8 PAY   5 

    0x13bde055,// 9 PAY   6 

    0x01b1b7d0,// 10 PAY   7 

    0xdd16643e,// 11 PAY   8 

    0x35e24993,// 12 PAY   9 

    0x9e2097a8,// 13 PAY  10 

    0x2aefc1bc,// 14 PAY  11 

    0xe83d4106,// 15 PAY  12 

    0x83ee2a62,// 16 PAY  13 

    0xef970178,// 17 PAY  14 

    0x8838ad4a,// 18 PAY  15 

    0x7c99fd4a,// 19 PAY  16 

    0x40e62d81,// 20 PAY  17 

    0xb74a9ad3,// 21 PAY  18 

    0x6b98bebc,// 22 PAY  19 

    0x54f1c59c,// 23 PAY  20 

    0xa4754fba,// 24 PAY  21 

    0x503c836c,// 25 PAY  22 

    0xd4677536,// 26 PAY  23 

    0xaf873c05,// 27 PAY  24 

    0x1258cdf5,// 28 PAY  25 

    0x0836b495,// 29 PAY  26 

    0x1c4c874b,// 30 PAY  27 

    0x9bb01696,// 31 PAY  28 

    0x74b4c2bd,// 32 PAY  29 

    0x3602ed03,// 33 PAY  30 

    0xb764d3a7,// 34 PAY  31 

    0x362bddbf,// 35 PAY  32 

    0x3c3f1961,// 36 PAY  33 

    0x10eec89d,// 37 PAY  34 

    0xff60380f,// 38 PAY  35 

    0x76a32d58,// 39 PAY  36 

    0x70717367,// 40 PAY  37 

    0xceb2ed54,// 41 PAY  38 

    0x318dc4c8,// 42 PAY  39 

    0xa23aa1f6,// 43 PAY  40 

    0x4fa683a3,// 44 PAY  41 

    0x504b1f01,// 45 PAY  42 

    0x91d454dd,// 46 PAY  43 

    0x413f2df0,// 47 PAY  44 

    0x57ec28c2,// 48 PAY  45 

    0x2b806b96,// 49 PAY  46 

    0x0a6b327a,// 50 PAY  47 

    0x264d75d3,// 51 PAY  48 

    0xad3ab59e,// 52 PAY  49 

    0x7fdc19d9,// 53 PAY  50 

    0x228c22b8,// 54 PAY  51 

    0xd9875fd2,// 55 PAY  52 

    0x42a9ada7,// 56 PAY  53 

    0x4d485edf,// 57 PAY  54 

    0x5f600df8,// 58 PAY  55 

    0x6ef97e41,// 59 PAY  56 

    0x32621879,// 60 PAY  57 

    0xd718f099,// 61 PAY  58 

    0x54c4e205,// 62 PAY  59 

    0xcc37bc67,// 63 PAY  60 

    0x51eb9a28,// 64 PAY  61 

    0x60052a84,// 65 PAY  62 

    0xf23b16c9,// 66 PAY  63 

    0x8181c92c,// 67 PAY  64 

    0x423c6460,// 68 PAY  65 

    0xc20e0cee,// 69 PAY  66 

    0xfa96538d,// 70 PAY  67 

    0x3bc45fe5,// 71 PAY  68 

    0x6c3a42ab,// 72 PAY  69 

    0xf8abd54c,// 73 PAY  70 

    0x2cf6eedb,// 74 PAY  71 

    0xd4f381e4,// 75 PAY  72 

    0x9e6f48ff,// 76 PAY  73 

    0xdce6af13,// 77 PAY  74 

    0x06df2757,// 78 PAY  75 

    0xa9e6ea34,// 79 PAY  76 

    0xa0dafffb,// 80 PAY  77 

    0xe901b5b2,// 81 PAY  78 

    0x6e9417a3,// 82 PAY  79 

    0xed3f6ddc,// 83 PAY  80 

    0xa7ceebe4,// 84 PAY  81 

    0xbc0abd11,// 85 PAY  82 

    0x43498bdf,// 86 PAY  83 

    0x2eee88c3,// 87 PAY  84 

    0x87089588,// 88 PAY  85 

    0x78dbab9c,// 89 PAY  86 

    0x76e46ee5,// 90 PAY  87 

    0xc5156c0f,// 91 PAY  88 

    0x189b3dd9,// 92 PAY  89 

    0x5da610ef,// 93 PAY  90 

    0xeb9ee7dc,// 94 PAY  91 

    0x87144ead,// 95 PAY  92 

    0x266aefe6,// 96 PAY  93 

    0xa91db10e,// 97 PAY  94 

    0x491ca2e7,// 98 PAY  95 

    0x7a0a3962,// 99 PAY  96 

    0x1cad827a,// 100 PAY  97 

    0x27b1c1d2,// 101 PAY  98 

    0xd890569d,// 102 PAY  99 

    0x287b2494,// 103 PAY 100 

    0x69414c28,// 104 PAY 101 

    0xdc92695b,// 105 PAY 102 

    0x2cb2d138,// 106 PAY 103 

    0x320312b0,// 107 PAY 104 

    0xd9a5fb13,// 108 PAY 105 

    0x5f1f7781,// 109 PAY 106 

    0x165a4c9a,// 110 PAY 107 

    0x01cc65ef,// 111 PAY 108 

    0x27b065fd,// 112 PAY 109 

    0xedc29901,// 113 PAY 110 

    0xf36c8678,// 114 PAY 111 

    0x64aa9eb6,// 115 PAY 112 

    0xfc14e015,// 116 PAY 113 

    0x698a93f2,// 117 PAY 114 

    0x981c1261,// 118 PAY 115 

    0x1ce87baa,// 119 PAY 116 

    0x46f848ad,// 120 PAY 117 

    0xaa4246be,// 121 PAY 118 

    0x91970ecb,// 122 PAY 119 

    0xbbeab6d0,// 123 PAY 120 

    0xd048bcd4,// 124 PAY 121 

    0x036763ca,// 125 PAY 122 

    0x73ead135,// 126 PAY 123 

    0x1e1acde0,// 127 PAY 124 

    0x5aca35d8,// 128 PAY 125 

    0x27f29aec,// 129 PAY 126 

    0x488a4e58,// 130 PAY 127 

    0x5cc51821,// 131 PAY 128 

    0x821767e8,// 132 PAY 129 

    0x9e3953e3,// 133 PAY 130 

    0x5f74da4d,// 134 PAY 131 

    0x76263571,// 135 PAY 132 

    0xbf462c2e,// 136 PAY 133 

    0xe7e6aeea,// 137 PAY 134 

    0x510f1d9a,// 138 PAY 135 

    0xa72d2cb8,// 139 PAY 136 

    0xb7c10be1,// 140 PAY 137 

    0xed91aa2b,// 141 PAY 138 

    0x0404c639,// 142 PAY 139 

    0x07d06c73,// 143 PAY 140 

    0x1cf306aa,// 144 PAY 141 

    0xf33b3c4b,// 145 PAY 142 

    0xce1851fe,// 146 PAY 143 

    0x98044f00,// 147 PAY 144 

    0x8f51c7a5,// 148 PAY 145 

    0xfdf461a8,// 149 PAY 146 

    0x818108f1,// 150 PAY 147 

    0xabf7dd2d,// 151 PAY 148 

    0xb66cbfd2,// 152 PAY 149 

    0x449a5c9d,// 153 PAY 150 

    0x20042617,// 154 PAY 151 

    0xac075975,// 155 PAY 152 

    0x53cdae0e,// 156 PAY 153 

    0xc4bb1c38,// 157 PAY 154 

    0x881e13a2,// 158 PAY 155 

    0xeb850728,// 159 PAY 156 

    0xce7a7577,// 160 PAY 157 

    0xff981773,// 161 PAY 158 

    0x3dd24e80,// 162 PAY 159 

    0x92df9fe7,// 163 PAY 160 

    0xab22d160,// 164 PAY 161 

    0xa4500997,// 165 PAY 162 

    0x93535ef4,// 166 PAY 163 

    0x31b37ca3,// 167 PAY 164 

    0xaf8633d3,// 168 PAY 165 

    0x4fc3bc01,// 169 PAY 166 

    0x9ee538a3,// 170 PAY 167 

    0x28c621b9,// 171 PAY 168 

    0x09a89d27,// 172 PAY 169 

    0xde451940,// 173 PAY 170 

    0xf3e0e9db,// 174 PAY 171 

    0xeab308c7,// 175 PAY 172 

    0x3f454e35,// 176 PAY 173 

    0xf1e3d20d,// 177 PAY 174 

    0x803d2386,// 178 PAY 175 

    0x67d61b83,// 179 PAY 176 

    0x384bdbeb,// 180 PAY 177 

    0xf472a74b,// 181 PAY 178 

    0x325b01b5,// 182 PAY 179 

    0xf574cc3a,// 183 PAY 180 

    0x7e9a3c27,// 184 PAY 181 

    0xbece69ff,// 185 PAY 182 

    0x9502f31c,// 186 PAY 183 

    0x02c84a34,// 187 PAY 184 

    0x503b3557,// 188 PAY 185 

    0xc753a40b,// 189 PAY 186 

    0x77bdc6b6,// 190 PAY 187 

    0xe1979a55,// 191 PAY 188 

    0xfbfedba7,// 192 PAY 189 

    0x14dc3fec,// 193 PAY 190 

    0xf4d74a73,// 194 PAY 191 

    0xced3855b,// 195 PAY 192 

    0x1dc6c491,// 196 PAY 193 

    0xc9aa6366,// 197 PAY 194 

    0x459f3af9,// 198 PAY 195 

    0xb61c1b52,// 199 PAY 196 

    0x9fe39dee,// 200 PAY 197 

    0x2268b24e,// 201 PAY 198 

    0x04f85423,// 202 PAY 199 

    0x4a155942,// 203 PAY 200 

    0x4398f0f6,// 204 PAY 201 

    0x0eb7bdc8,// 205 PAY 202 

    0xa5101cbe,// 206 PAY 203 

    0xdd366a6e,// 207 PAY 204 

    0x0d7f6761,// 208 PAY 205 

    0xff415f35,// 209 PAY 206 

    0x740731d9,// 210 PAY 207 

    0x8b7e0607,// 211 PAY 208 

    0xcb12cdfc,// 212 PAY 209 

    0xdc7db7ba,// 213 PAY 210 

    0x3106094c,// 214 PAY 211 

    0xc94e0810,// 215 PAY 212 

    0x326ad469,// 216 PAY 213 

    0x55d22c98,// 217 PAY 214 

    0x545bfea1,// 218 PAY 215 

    0xd8fb8dd1,// 219 PAY 216 

    0x0c4f1cd8,// 220 PAY 217 

    0x6ae742eb,// 221 PAY 218 

    0x08e43677,// 222 PAY 219 

    0xb8d90ad6,// 223 PAY 220 

    0xd40d8ff9,// 224 PAY 221 

    0xf50df376,// 225 PAY 222 

    0x61bf5345,// 226 PAY 223 

    0xf6a16c12,// 227 PAY 224 

    0xba9f6d9b,// 228 PAY 225 

    0x0c721e6f,// 229 PAY 226 

    0xe4c77e4b,// 230 PAY 227 

    0xb7085da7,// 231 PAY 228 

    0x7ebf5a04,// 232 PAY 229 

    0x9f29911d,// 233 PAY 230 

    0x21c9d2fe,// 234 PAY 231 

    0xd722a351,// 235 PAY 232 

    0x4fc2ac4b,// 236 PAY 233 

    0x40434800,// 237 PAY 234 

/// HASH is  8 bytes 

    0x7ebf5a04,// 238 HSH   1 

    0x9f29911d,// 239 HSH   2 

/// STA is 1 words. 

/// STA num_pkts       : 111 

/// STA pkt_idx        : 124 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4d 

    0x01f04d6f // 240 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt75_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 382 words. 

/// BDA size     is 1523 (0x5f3) 

/// BDA id       is 0xec5d 

    0x05f3ec5d,// 3 BDA   1 

/// PAY Generic Data size   : 1523 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x84fdda40,// 4 PAY   1 

    0x20bdb30f,// 5 PAY   2 

    0xb3951a39,// 6 PAY   3 

    0x9574977c,// 7 PAY   4 

    0x1f7d6fe5,// 8 PAY   5 

    0x80be8f39,// 9 PAY   6 

    0x299cc98e,// 10 PAY   7 

    0xa1eb7d34,// 11 PAY   8 

    0xa11204be,// 12 PAY   9 

    0xe80170fe,// 13 PAY  10 

    0x27f13fd0,// 14 PAY  11 

    0x1fef0790,// 15 PAY  12 

    0xe05e0c39,// 16 PAY  13 

    0x02ea676a,// 17 PAY  14 

    0x4738c840,// 18 PAY  15 

    0x02a0df77,// 19 PAY  16 

    0x0a3c4a26,// 20 PAY  17 

    0xabc613c8,// 21 PAY  18 

    0xbf288444,// 22 PAY  19 

    0xd2a14993,// 23 PAY  20 

    0x9dd8c97e,// 24 PAY  21 

    0x9961bab8,// 25 PAY  22 

    0x51388a07,// 26 PAY  23 

    0xb77b5603,// 27 PAY  24 

    0xaac47a40,// 28 PAY  25 

    0x62e3a73a,// 29 PAY  26 

    0xbf1567ce,// 30 PAY  27 

    0x8cb249fb,// 31 PAY  28 

    0x5cc3df0f,// 32 PAY  29 

    0x087bc034,// 33 PAY  30 

    0x60f3fe3b,// 34 PAY  31 

    0x6f014bbb,// 35 PAY  32 

    0x968d6818,// 36 PAY  33 

    0xf8865bb2,// 37 PAY  34 

    0x6ab26356,// 38 PAY  35 

    0xc1a51bd9,// 39 PAY  36 

    0xf2aabf73,// 40 PAY  37 

    0x51c0d950,// 41 PAY  38 

    0x425b3d1d,// 42 PAY  39 

    0x70458f55,// 43 PAY  40 

    0xfa0fc01e,// 44 PAY  41 

    0x8a2c022e,// 45 PAY  42 

    0x8f97d757,// 46 PAY  43 

    0xe94b50bd,// 47 PAY  44 

    0x56e33aa1,// 48 PAY  45 

    0x99b184d7,// 49 PAY  46 

    0xfb1224aa,// 50 PAY  47 

    0xdf4148e1,// 51 PAY  48 

    0x4b14150a,// 52 PAY  49 

    0x5ce722e9,// 53 PAY  50 

    0x570ee6a3,// 54 PAY  51 

    0x18633896,// 55 PAY  52 

    0xac943f29,// 56 PAY  53 

    0xc46f0abd,// 57 PAY  54 

    0x109b1ff3,// 58 PAY  55 

    0xbd7019d2,// 59 PAY  56 

    0xb13ed464,// 60 PAY  57 

    0x12e3340c,// 61 PAY  58 

    0xdb9b17a9,// 62 PAY  59 

    0x3f31927d,// 63 PAY  60 

    0x24a56c90,// 64 PAY  61 

    0x1de5f8d9,// 65 PAY  62 

    0x0108920b,// 66 PAY  63 

    0xfbfbcc6b,// 67 PAY  64 

    0xb847da45,// 68 PAY  65 

    0xc2612b49,// 69 PAY  66 

    0xdd6617a8,// 70 PAY  67 

    0xc2c5995c,// 71 PAY  68 

    0x63754d97,// 72 PAY  69 

    0x0acdfbe4,// 73 PAY  70 

    0x10c44c65,// 74 PAY  71 

    0x0481bf85,// 75 PAY  72 

    0xfdd3aaf0,// 76 PAY  73 

    0x2d3a31f7,// 77 PAY  74 

    0xd3cfaf91,// 78 PAY  75 

    0xc7e0466c,// 79 PAY  76 

    0xeee3d9bd,// 80 PAY  77 

    0x2ac0f10a,// 81 PAY  78 

    0x3cf2efe7,// 82 PAY  79 

    0xf4bb0e7e,// 83 PAY  80 

    0xdd558009,// 84 PAY  81 

    0xd8ed9b08,// 85 PAY  82 

    0x8c61e215,// 86 PAY  83 

    0x9b98eed8,// 87 PAY  84 

    0x84308b33,// 88 PAY  85 

    0x9fe1639b,// 89 PAY  86 

    0x32ea71fe,// 90 PAY  87 

    0xe5e66d35,// 91 PAY  88 

    0xf90d342a,// 92 PAY  89 

    0xf5a51c8c,// 93 PAY  90 

    0x6d52c87d,// 94 PAY  91 

    0xd2398e8d,// 95 PAY  92 

    0x4bb12088,// 96 PAY  93 

    0x36b0b54d,// 97 PAY  94 

    0x7afe18d6,// 98 PAY  95 

    0x505303e9,// 99 PAY  96 

    0xaee69759,// 100 PAY  97 

    0x8eda9d70,// 101 PAY  98 

    0x2d6b7537,// 102 PAY  99 

    0x1f9043f7,// 103 PAY 100 

    0xa8fd84d9,// 104 PAY 101 

    0x4c409742,// 105 PAY 102 

    0xfbeababb,// 106 PAY 103 

    0x5725e4e8,// 107 PAY 104 

    0x0abef7f2,// 108 PAY 105 

    0xe8716edb,// 109 PAY 106 

    0xac50bba7,// 110 PAY 107 

    0xf86c846c,// 111 PAY 108 

    0x815a61d7,// 112 PAY 109 

    0x9912b7f9,// 113 PAY 110 

    0x8925d10a,// 114 PAY 111 

    0x8d64502e,// 115 PAY 112 

    0x7bf2bdb5,// 116 PAY 113 

    0x967fbaad,// 117 PAY 114 

    0xb147caa8,// 118 PAY 115 

    0x13493382,// 119 PAY 116 

    0xeb8410d1,// 120 PAY 117 

    0x4573a155,// 121 PAY 118 

    0x8c4290f0,// 122 PAY 119 

    0xe0a6b1b0,// 123 PAY 120 

    0x513fbd4e,// 124 PAY 121 

    0xf02beecb,// 125 PAY 122 

    0x15d653cc,// 126 PAY 123 

    0xcbb21213,// 127 PAY 124 

    0xde38b5d3,// 128 PAY 125 

    0xf8f49530,// 129 PAY 126 

    0x499ab83f,// 130 PAY 127 

    0x7e27bb52,// 131 PAY 128 

    0xd86e084b,// 132 PAY 129 

    0x02950e22,// 133 PAY 130 

    0x523e3fc7,// 134 PAY 131 

    0xf8dc55d3,// 135 PAY 132 

    0xa3de63f2,// 136 PAY 133 

    0x91163767,// 137 PAY 134 

    0xf3ac1fe2,// 138 PAY 135 

    0xd3d014f7,// 139 PAY 136 

    0xa05ab35d,// 140 PAY 137 

    0x1c4e90c0,// 141 PAY 138 

    0x4c7b550f,// 142 PAY 139 

    0xbac09ec9,// 143 PAY 140 

    0x83a6e934,// 144 PAY 141 

    0x1f9a48e5,// 145 PAY 142 

    0xfcbfc784,// 146 PAY 143 

    0x808d58ad,// 147 PAY 144 

    0x39bd5c37,// 148 PAY 145 

    0xa590c28b,// 149 PAY 146 

    0x6b3e8175,// 150 PAY 147 

    0xe897c723,// 151 PAY 148 

    0x30d2480c,// 152 PAY 149 

    0x3e2c7b56,// 153 PAY 150 

    0xe6085f0f,// 154 PAY 151 

    0x760a3e29,// 155 PAY 152 

    0xa1810b6e,// 156 PAY 153 

    0xa9c330d2,// 157 PAY 154 

    0xfc2c6339,// 158 PAY 155 

    0x2f724d86,// 159 PAY 156 

    0x8d747bfb,// 160 PAY 157 

    0xb4051c85,// 161 PAY 158 

    0xf184adf1,// 162 PAY 159 

    0x180d4dc0,// 163 PAY 160 

    0x99218fe3,// 164 PAY 161 

    0x515e3c97,// 165 PAY 162 

    0xb5cdfe7d,// 166 PAY 163 

    0xe79a4d2d,// 167 PAY 164 

    0xd0d4a626,// 168 PAY 165 

    0xbfa218a9,// 169 PAY 166 

    0x442bdb78,// 170 PAY 167 

    0x09d1db9f,// 171 PAY 168 

    0xb619fc79,// 172 PAY 169 

    0xc2f10d96,// 173 PAY 170 

    0x622f6926,// 174 PAY 171 

    0xcdf81522,// 175 PAY 172 

    0xf7fb8699,// 176 PAY 173 

    0x66efadb3,// 177 PAY 174 

    0xd94080da,// 178 PAY 175 

    0x57161937,// 179 PAY 176 

    0x167a62d6,// 180 PAY 177 

    0xe10ab55e,// 181 PAY 178 

    0x574abc09,// 182 PAY 179 

    0xc30c2a1d,// 183 PAY 180 

    0x079a3e06,// 184 PAY 181 

    0xe2aaa3e9,// 185 PAY 182 

    0x2ebf09f9,// 186 PAY 183 

    0xda0662c8,// 187 PAY 184 

    0xac4f5540,// 188 PAY 185 

    0xcd628647,// 189 PAY 186 

    0xdca5d8e1,// 190 PAY 187 

    0xe8e2186d,// 191 PAY 188 

    0xd1c555ec,// 192 PAY 189 

    0x8b396e2a,// 193 PAY 190 

    0x880cdf37,// 194 PAY 191 

    0x8d9f8eda,// 195 PAY 192 

    0x1a8d4b4f,// 196 PAY 193 

    0xcdec1baf,// 197 PAY 194 

    0xddf23f46,// 198 PAY 195 

    0xde0be85a,// 199 PAY 196 

    0x3cb34812,// 200 PAY 197 

    0xe1c30fdb,// 201 PAY 198 

    0x476a27bc,// 202 PAY 199 

    0x447ba0e4,// 203 PAY 200 

    0x46dd3c7e,// 204 PAY 201 

    0x825916cb,// 205 PAY 202 

    0xed18db2b,// 206 PAY 203 

    0x171e175b,// 207 PAY 204 

    0x765dd56b,// 208 PAY 205 

    0x8f39822a,// 209 PAY 206 

    0xb482c2f5,// 210 PAY 207 

    0x8efbc7d1,// 211 PAY 208 

    0x1716626d,// 212 PAY 209 

    0x97396074,// 213 PAY 210 

    0x098ba85c,// 214 PAY 211 

    0x1758e231,// 215 PAY 212 

    0xa2942edc,// 216 PAY 213 

    0x088ffc00,// 217 PAY 214 

    0x68a64093,// 218 PAY 215 

    0x9411f84c,// 219 PAY 216 

    0x3f99a3af,// 220 PAY 217 

    0x96c46df2,// 221 PAY 218 

    0x42246c37,// 222 PAY 219 

    0xfed23de3,// 223 PAY 220 

    0x9fa1a7cb,// 224 PAY 221 

    0xa2199d34,// 225 PAY 222 

    0xbf76e2e8,// 226 PAY 223 

    0x428f1e69,// 227 PAY 224 

    0x91b2a809,// 228 PAY 225 

    0x2cf92a15,// 229 PAY 226 

    0xcbafe4ae,// 230 PAY 227 

    0xc8996674,// 231 PAY 228 

    0x34afc052,// 232 PAY 229 

    0xefef8a10,// 233 PAY 230 

    0xed3c405d,// 234 PAY 231 

    0x23eaca19,// 235 PAY 232 

    0xa5ef9c7e,// 236 PAY 233 

    0x9fe10d09,// 237 PAY 234 

    0x2e4c8f44,// 238 PAY 235 

    0x845e6a32,// 239 PAY 236 

    0x23b36b2b,// 240 PAY 237 

    0x34978cb9,// 241 PAY 238 

    0x47ed8be4,// 242 PAY 239 

    0x0ca55f53,// 243 PAY 240 

    0x13460391,// 244 PAY 241 

    0xe257145d,// 245 PAY 242 

    0x3034a7e3,// 246 PAY 243 

    0xb42026b0,// 247 PAY 244 

    0x514ef3a6,// 248 PAY 245 

    0x3d6c22f1,// 249 PAY 246 

    0x8208e2ce,// 250 PAY 247 

    0x083595ec,// 251 PAY 248 

    0xcb2f7c82,// 252 PAY 249 

    0x364ff57f,// 253 PAY 250 

    0xe1078ddd,// 254 PAY 251 

    0x1f359af7,// 255 PAY 252 

    0x4a1740d9,// 256 PAY 253 

    0x885493ba,// 257 PAY 254 

    0xa022d889,// 258 PAY 255 

    0xc15a3818,// 259 PAY 256 

    0x98fcab8c,// 260 PAY 257 

    0x72c722e0,// 261 PAY 258 

    0xf06cefa6,// 262 PAY 259 

    0x8f50159c,// 263 PAY 260 

    0x4632f114,// 264 PAY 261 

    0x95712a17,// 265 PAY 262 

    0xd39fa6b1,// 266 PAY 263 

    0x9785537b,// 267 PAY 264 

    0xb3d45582,// 268 PAY 265 

    0xd9835292,// 269 PAY 266 

    0x1999705c,// 270 PAY 267 

    0xa01fc4df,// 271 PAY 268 

    0x5b027eb0,// 272 PAY 269 

    0x2e24cd76,// 273 PAY 270 

    0xd8486380,// 274 PAY 271 

    0x1ee435f2,// 275 PAY 272 

    0xed306bc4,// 276 PAY 273 

    0x804ed16a,// 277 PAY 274 

    0x3c7c3f01,// 278 PAY 275 

    0xb1c6d7c4,// 279 PAY 276 

    0x1b18a79a,// 280 PAY 277 

    0x2569869e,// 281 PAY 278 

    0xedba49a2,// 282 PAY 279 

    0x581b928c,// 283 PAY 280 

    0xcfa9f29e,// 284 PAY 281 

    0xbcf3351f,// 285 PAY 282 

    0x4ca4b7e5,// 286 PAY 283 

    0xd9455906,// 287 PAY 284 

    0x65687bfe,// 288 PAY 285 

    0x764edbdc,// 289 PAY 286 

    0x9669dc9b,// 290 PAY 287 

    0xec84aebd,// 291 PAY 288 

    0xe1f9182a,// 292 PAY 289 

    0x2923c4aa,// 293 PAY 290 

    0x871eb7c1,// 294 PAY 291 

    0x9f41f983,// 295 PAY 292 

    0xcd05afad,// 296 PAY 293 

    0x39d5762b,// 297 PAY 294 

    0xa32eb4cc,// 298 PAY 295 

    0x6c2e4714,// 299 PAY 296 

    0xbd3cba54,// 300 PAY 297 

    0xb963613a,// 301 PAY 298 

    0x75203433,// 302 PAY 299 

    0x113da8a7,// 303 PAY 300 

    0x62a94074,// 304 PAY 301 

    0x9acbfd53,// 305 PAY 302 

    0x0b692a4e,// 306 PAY 303 

    0xe7f3ac18,// 307 PAY 304 

    0x74be4bee,// 308 PAY 305 

    0xf707c4d1,// 309 PAY 306 

    0x8805c470,// 310 PAY 307 

    0x4f37c9be,// 311 PAY 308 

    0x9f8239a2,// 312 PAY 309 

    0x74de69de,// 313 PAY 310 

    0x2cf815c8,// 314 PAY 311 

    0x20dbb7cc,// 315 PAY 312 

    0xd9218a34,// 316 PAY 313 

    0x79fee165,// 317 PAY 314 

    0xc96c537e,// 318 PAY 315 

    0xe4f61c1a,// 319 PAY 316 

    0xee5a4340,// 320 PAY 317 

    0xd2339bec,// 321 PAY 318 

    0x2138ab1e,// 322 PAY 319 

    0x15d88abd,// 323 PAY 320 

    0x0451eaf3,// 324 PAY 321 

    0x591772b0,// 325 PAY 322 

    0xaf5e50c4,// 326 PAY 323 

    0x44b9b0e8,// 327 PAY 324 

    0x2c979226,// 328 PAY 325 

    0x40cb8359,// 329 PAY 326 

    0x62fdae52,// 330 PAY 327 

    0x2b808aa6,// 331 PAY 328 

    0x44817ea1,// 332 PAY 329 

    0x8dcc98a9,// 333 PAY 330 

    0x37bdecc8,// 334 PAY 331 

    0x286f31a4,// 335 PAY 332 

    0xbad90edd,// 336 PAY 333 

    0x2e6dfd7f,// 337 PAY 334 

    0xc345592b,// 338 PAY 335 

    0x25b987f2,// 339 PAY 336 

    0x3a293d68,// 340 PAY 337 

    0x83e9f655,// 341 PAY 338 

    0x347cecb9,// 342 PAY 339 

    0xf0aa4076,// 343 PAY 340 

    0xa3fd2590,// 344 PAY 341 

    0x78f8bb30,// 345 PAY 342 

    0x9c1acba1,// 346 PAY 343 

    0x23c846e7,// 347 PAY 344 

    0x684e704b,// 348 PAY 345 

    0x8164639e,// 349 PAY 346 

    0x6fb2989a,// 350 PAY 347 

    0x5ae00fa0,// 351 PAY 348 

    0xd68f8d8e,// 352 PAY 349 

    0x4fb3f92e,// 353 PAY 350 

    0x6e232355,// 354 PAY 351 

    0xb33936da,// 355 PAY 352 

    0x1cadf46d,// 356 PAY 353 

    0xb92f67d9,// 357 PAY 354 

    0x0d58f2c7,// 358 PAY 355 

    0x5b481b01,// 359 PAY 356 

    0x989762ff,// 360 PAY 357 

    0x8ee550f2,// 361 PAY 358 

    0xb0e91581,// 362 PAY 359 

    0xa56f2c56,// 363 PAY 360 

    0x3c957b1b,// 364 PAY 361 

    0x7473d6ae,// 365 PAY 362 

    0x261eaf06,// 366 PAY 363 

    0xd34850e6,// 367 PAY 364 

    0x31444d71,// 368 PAY 365 

    0x62649544,// 369 PAY 366 

    0x3caf2bbd,// 370 PAY 367 

    0x243e5ceb,// 371 PAY 368 

    0x5cd118cb,// 372 PAY 369 

    0x16489fb9,// 373 PAY 370 

    0x0fb3080a,// 374 PAY 371 

    0x9a3de689,// 375 PAY 372 

    0x802e6100,// 376 PAY 373 

    0xce4393a4,// 377 PAY 374 

    0x7abe2e7f,// 378 PAY 375 

    0xe68ac8a4,// 379 PAY 376 

    0xbf4ca502,// 380 PAY 377 

    0x97b083be,// 381 PAY 378 

    0x43aaa914,// 382 PAY 379 

    0xbeab49bf,// 383 PAY 380 

    0xdeab6000,// 384 PAY 381 

/// STA is 1 words. 

/// STA num_pkts       : 147 

/// STA pkt_idx        : 180 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x96 

    0x02d09693 // 385 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt76_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 415 words. 

/// BDA size     is 1654 (0x676) 

/// BDA id       is 0xd76a 

    0x0676d76a,// 3 BDA   1 

/// PAY Generic Data size   : 1654 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x44bf5494,// 4 PAY   1 

    0x0b9c32be,// 5 PAY   2 

    0x4ae85f49,// 6 PAY   3 

    0x0423034f,// 7 PAY   4 

    0xbcc4fc99,// 8 PAY   5 

    0xccdcc124,// 9 PAY   6 

    0xe4f08bb7,// 10 PAY   7 

    0xf0daccf4,// 11 PAY   8 

    0xfd431b65,// 12 PAY   9 

    0xf7a9a11a,// 13 PAY  10 

    0xbecbbf26,// 14 PAY  11 

    0x58fd3a58,// 15 PAY  12 

    0x8b2086da,// 16 PAY  13 

    0x69a05a21,// 17 PAY  14 

    0x93a14140,// 18 PAY  15 

    0x4cd700ae,// 19 PAY  16 

    0x2751f0c4,// 20 PAY  17 

    0x7be8b910,// 21 PAY  18 

    0xf75ae2f2,// 22 PAY  19 

    0x472ff78c,// 23 PAY  20 

    0xcf6716b7,// 24 PAY  21 

    0xca33a4a6,// 25 PAY  22 

    0xc25af151,// 26 PAY  23 

    0xf1619095,// 27 PAY  24 

    0x3ea3c726,// 28 PAY  25 

    0xedaaeb55,// 29 PAY  26 

    0xfa603639,// 30 PAY  27 

    0x1185d6a8,// 31 PAY  28 

    0x9a3eea05,// 32 PAY  29 

    0xa23b9d11,// 33 PAY  30 

    0xbc77c351,// 34 PAY  31 

    0xbc603d67,// 35 PAY  32 

    0xb9718142,// 36 PAY  33 

    0x72dda0e1,// 37 PAY  34 

    0xe0ce5244,// 38 PAY  35 

    0x4a7607a0,// 39 PAY  36 

    0xc2bfa806,// 40 PAY  37 

    0x26e34f55,// 41 PAY  38 

    0x8f7487cc,// 42 PAY  39 

    0x03ebc37d,// 43 PAY  40 

    0xaf7048fb,// 44 PAY  41 

    0x390981bb,// 45 PAY  42 

    0x2025f23f,// 46 PAY  43 

    0x1d96a86a,// 47 PAY  44 

    0xf1d75e93,// 48 PAY  45 

    0x70d693ea,// 49 PAY  46 

    0xa366e9fd,// 50 PAY  47 

    0x64a08016,// 51 PAY  48 

    0x063deba1,// 52 PAY  49 

    0x370fa7ef,// 53 PAY  50 

    0x736b7d55,// 54 PAY  51 

    0xf99b23a2,// 55 PAY  52 

    0x51915110,// 56 PAY  53 

    0x42e32910,// 57 PAY  54 

    0x6f0c1fb9,// 58 PAY  55 

    0x9d2997cb,// 59 PAY  56 

    0x4e0ff9e0,// 60 PAY  57 

    0x233b0bda,// 61 PAY  58 

    0xd38f2360,// 62 PAY  59 

    0xc613f025,// 63 PAY  60 

    0x436c56d1,// 64 PAY  61 

    0x400cb8e5,// 65 PAY  62 

    0x690b9f67,// 66 PAY  63 

    0x96f6c2b3,// 67 PAY  64 

    0xdc7cc3f8,// 68 PAY  65 

    0xde71f785,// 69 PAY  66 

    0x67640147,// 70 PAY  67 

    0xa782817f,// 71 PAY  68 

    0x8c3b1ea1,// 72 PAY  69 

    0x153c4dd3,// 73 PAY  70 

    0x98799afa,// 74 PAY  71 

    0xced99272,// 75 PAY  72 

    0x3fef0fd3,// 76 PAY  73 

    0x06067513,// 77 PAY  74 

    0x1e45d32c,// 78 PAY  75 

    0xd00bc1d0,// 79 PAY  76 

    0x77a37f85,// 80 PAY  77 

    0x41d138fa,// 81 PAY  78 

    0xcef437a6,// 82 PAY  79 

    0x19dbbc2c,// 83 PAY  80 

    0x98e4156a,// 84 PAY  81 

    0x2e4bb72b,// 85 PAY  82 

    0xcf4d4a70,// 86 PAY  83 

    0xaedbe741,// 87 PAY  84 

    0x544c6254,// 88 PAY  85 

    0x9c67e2f4,// 89 PAY  86 

    0x5535641d,// 90 PAY  87 

    0x2594b87d,// 91 PAY  88 

    0x5ce8cb2e,// 92 PAY  89 

    0x09eb1073,// 93 PAY  90 

    0x84179727,// 94 PAY  91 

    0xa8a05491,// 95 PAY  92 

    0x2e26c25d,// 96 PAY  93 

    0x30eb60b7,// 97 PAY  94 

    0xe1fe83e8,// 98 PAY  95 

    0xeea12ae5,// 99 PAY  96 

    0x4ba2f204,// 100 PAY  97 

    0xcf46af54,// 101 PAY  98 

    0x7b673d7f,// 102 PAY  99 

    0x34e5ca05,// 103 PAY 100 

    0xb218a4e7,// 104 PAY 101 

    0x96f2d1a6,// 105 PAY 102 

    0xc848785f,// 106 PAY 103 

    0x557749ac,// 107 PAY 104 

    0xa4f42d5c,// 108 PAY 105 

    0xff464575,// 109 PAY 106 

    0x34ceadcb,// 110 PAY 107 

    0x3f791e63,// 111 PAY 108 

    0xb7deac31,// 112 PAY 109 

    0x4dda39d9,// 113 PAY 110 

    0x68411e3c,// 114 PAY 111 

    0xf271b756,// 115 PAY 112 

    0x81eb8ca0,// 116 PAY 113 

    0xbfdd9c48,// 117 PAY 114 

    0x921e1c2f,// 118 PAY 115 

    0xa72ab4dd,// 119 PAY 116 

    0x5f0f0d35,// 120 PAY 117 

    0xf45d6678,// 121 PAY 118 

    0x38d25461,// 122 PAY 119 

    0x9c80b547,// 123 PAY 120 

    0x40652c6b,// 124 PAY 121 

    0x41975b31,// 125 PAY 122 

    0x3f86bafb,// 126 PAY 123 

    0x61785b62,// 127 PAY 124 

    0x90c5bc51,// 128 PAY 125 

    0x63cf25a3,// 129 PAY 126 

    0xb7c6a057,// 130 PAY 127 

    0x4c7598fc,// 131 PAY 128 

    0x257fdd9e,// 132 PAY 129 

    0x030f4e13,// 133 PAY 130 

    0x003581fb,// 134 PAY 131 

    0x49ac8389,// 135 PAY 132 

    0x55fa427d,// 136 PAY 133 

    0xb4ec7ba8,// 137 PAY 134 

    0xb7f6f4f5,// 138 PAY 135 

    0x4f21a5b0,// 139 PAY 136 

    0xe53b4a94,// 140 PAY 137 

    0x87ac8444,// 141 PAY 138 

    0xefce69f7,// 142 PAY 139 

    0xcb3edfed,// 143 PAY 140 

    0xab489494,// 144 PAY 141 

    0xd575dc07,// 145 PAY 142 

    0x60360898,// 146 PAY 143 

    0x197eea18,// 147 PAY 144 

    0xe49ce216,// 148 PAY 145 

    0xa3eec54f,// 149 PAY 146 

    0x3fe70785,// 150 PAY 147 

    0x8af5fd68,// 151 PAY 148 

    0x2dfff093,// 152 PAY 149 

    0x1886bb99,// 153 PAY 150 

    0xc5cbbba2,// 154 PAY 151 

    0xa92c68c5,// 155 PAY 152 

    0xc4436048,// 156 PAY 153 

    0xa4637da6,// 157 PAY 154 

    0x9e316e59,// 158 PAY 155 

    0x05997632,// 159 PAY 156 

    0x8fb7c7df,// 160 PAY 157 

    0x8299ef87,// 161 PAY 158 

    0x5760ea36,// 162 PAY 159 

    0x655fa70b,// 163 PAY 160 

    0x8ba83105,// 164 PAY 161 

    0x4f3a60a1,// 165 PAY 162 

    0xd4aab2eb,// 166 PAY 163 

    0x1bf553fc,// 167 PAY 164 

    0x757fc414,// 168 PAY 165 

    0x2f3cec51,// 169 PAY 166 

    0x0fecae9a,// 170 PAY 167 

    0x60b66947,// 171 PAY 168 

    0x2267959a,// 172 PAY 169 

    0x3eecf956,// 173 PAY 170 

    0xbc926fa1,// 174 PAY 171 

    0xe3d37da7,// 175 PAY 172 

    0xdedb005f,// 176 PAY 173 

    0x816dec6a,// 177 PAY 174 

    0x5ca6a48d,// 178 PAY 175 

    0xa1ba5e5c,// 179 PAY 176 

    0xeb811ffd,// 180 PAY 177 

    0x849eb8c2,// 181 PAY 178 

    0x9261c6c6,// 182 PAY 179 

    0x01c379dc,// 183 PAY 180 

    0xad3048b1,// 184 PAY 181 

    0x0d6a0fa4,// 185 PAY 182 

    0xeba60c10,// 186 PAY 183 

    0x764ebdba,// 187 PAY 184 

    0xec3cb7cc,// 188 PAY 185 

    0x5c896468,// 189 PAY 186 

    0x1ccd07b6,// 190 PAY 187 

    0xfa588fa8,// 191 PAY 188 

    0xb261a581,// 192 PAY 189 

    0xf4d1a90a,// 193 PAY 190 

    0xeece991f,// 194 PAY 191 

    0xf9c54678,// 195 PAY 192 

    0x87d239ee,// 196 PAY 193 

    0xa2430abe,// 197 PAY 194 

    0x22a195f1,// 198 PAY 195 

    0x5c2ddc53,// 199 PAY 196 

    0x597e09e5,// 200 PAY 197 

    0xa5bc0319,// 201 PAY 198 

    0x194cf783,// 202 PAY 199 

    0x09b1d463,// 203 PAY 200 

    0xca956950,// 204 PAY 201 

    0xfcc7bfc3,// 205 PAY 202 

    0x4eac2c02,// 206 PAY 203 

    0x549b2c1b,// 207 PAY 204 

    0x07c11659,// 208 PAY 205 

    0xacf809ce,// 209 PAY 206 

    0x6be8b89a,// 210 PAY 207 

    0x8b1e799e,// 211 PAY 208 

    0x3ae1f2f6,// 212 PAY 209 

    0x7e8f7e12,// 213 PAY 210 

    0x06d9baa2,// 214 PAY 211 

    0x86c914a9,// 215 PAY 212 

    0xb33678b1,// 216 PAY 213 

    0x59298f87,// 217 PAY 214 

    0xa1c56593,// 218 PAY 215 

    0x1bec1270,// 219 PAY 216 

    0x25c44001,// 220 PAY 217 

    0x6f8e81d2,// 221 PAY 218 

    0xce9e2a32,// 222 PAY 219 

    0x35fc45ce,// 223 PAY 220 

    0x43c89c5a,// 224 PAY 221 

    0x3a7c85ce,// 225 PAY 222 

    0xd3db6328,// 226 PAY 223 

    0x4e87e803,// 227 PAY 224 

    0x0ebdda92,// 228 PAY 225 

    0x03a33cfc,// 229 PAY 226 

    0xb832e011,// 230 PAY 227 

    0x0d4c1d7e,// 231 PAY 228 

    0x1e202416,// 232 PAY 229 

    0xc5a4951a,// 233 PAY 230 

    0x584b01a4,// 234 PAY 231 

    0x9c5d70fc,// 235 PAY 232 

    0x486750e5,// 236 PAY 233 

    0xed4ddd57,// 237 PAY 234 

    0x01ab6949,// 238 PAY 235 

    0x123b734b,// 239 PAY 236 

    0xd37a76f5,// 240 PAY 237 

    0x7699881f,// 241 PAY 238 

    0x9df0fde9,// 242 PAY 239 

    0x693a2ecf,// 243 PAY 240 

    0x6c36e477,// 244 PAY 241 

    0x66ff2416,// 245 PAY 242 

    0x2075354b,// 246 PAY 243 

    0x1f51569e,// 247 PAY 244 

    0x088ac978,// 248 PAY 245 

    0xfd508690,// 249 PAY 246 

    0x7b098068,// 250 PAY 247 

    0x53085c4e,// 251 PAY 248 

    0xc09a210a,// 252 PAY 249 

    0x3fd5cf21,// 253 PAY 250 

    0x94414940,// 254 PAY 251 

    0x084dfaed,// 255 PAY 252 

    0x7ade8c5f,// 256 PAY 253 

    0x47ae005f,// 257 PAY 254 

    0x8b08cd18,// 258 PAY 255 

    0xe7747e41,// 259 PAY 256 

    0xc434a1a7,// 260 PAY 257 

    0x4be3b573,// 261 PAY 258 

    0x178a4e8b,// 262 PAY 259 

    0xf04a717d,// 263 PAY 260 

    0x41e2c8a8,// 264 PAY 261 

    0x18782cab,// 265 PAY 262 

    0xc4d1fa5b,// 266 PAY 263 

    0x3c9ccfe6,// 267 PAY 264 

    0xf96be32c,// 268 PAY 265 

    0x94cb32c5,// 269 PAY 266 

    0x914d0a12,// 270 PAY 267 

    0x5e746b05,// 271 PAY 268 

    0xa188f8eb,// 272 PAY 269 

    0xed97b84c,// 273 PAY 270 

    0x7e191d23,// 274 PAY 271 

    0xe01e2df2,// 275 PAY 272 

    0xa1a77f68,// 276 PAY 273 

    0x0852fd48,// 277 PAY 274 

    0xf3523617,// 278 PAY 275 

    0x5108f46f,// 279 PAY 276 

    0x7c5e737e,// 280 PAY 277 

    0x59987233,// 281 PAY 278 

    0x777478a4,// 282 PAY 279 

    0x1034605a,// 283 PAY 280 

    0xb796e67e,// 284 PAY 281 

    0x11ab7e7a,// 285 PAY 282 

    0xb23bda64,// 286 PAY 283 

    0x0d81845d,// 287 PAY 284 

    0x0e54bf65,// 288 PAY 285 

    0x180eddc0,// 289 PAY 286 

    0x4357768e,// 290 PAY 287 

    0x13765849,// 291 PAY 288 

    0x26fd3949,// 292 PAY 289 

    0xd5d30d73,// 293 PAY 290 

    0xac3b2c22,// 294 PAY 291 

    0x253ee495,// 295 PAY 292 

    0xbb7689e4,// 296 PAY 293 

    0xfcfd595d,// 297 PAY 294 

    0xcd1055bc,// 298 PAY 295 

    0xed9bc390,// 299 PAY 296 

    0xf5ed6739,// 300 PAY 297 

    0x5cb92cf2,// 301 PAY 298 

    0x33520483,// 302 PAY 299 

    0x9dbbb7fb,// 303 PAY 300 

    0x985a13b9,// 304 PAY 301 

    0x50645dc6,// 305 PAY 302 

    0xe5752864,// 306 PAY 303 

    0x5da47299,// 307 PAY 304 

    0x113a2391,// 308 PAY 305 

    0xc3a565ff,// 309 PAY 306 

    0x046b34b3,// 310 PAY 307 

    0x2dc89e23,// 311 PAY 308 

    0xb5fb9ddb,// 312 PAY 309 

    0xc1ac1bc4,// 313 PAY 310 

    0x46a345ea,// 314 PAY 311 

    0x6f42c6ce,// 315 PAY 312 

    0x54ee0304,// 316 PAY 313 

    0x9c45443e,// 317 PAY 314 

    0xe24c4d24,// 318 PAY 315 

    0x9b588071,// 319 PAY 316 

    0x0b5aeaf5,// 320 PAY 317 

    0x6c056462,// 321 PAY 318 

    0x22abaf9f,// 322 PAY 319 

    0x38e66dbb,// 323 PAY 320 

    0x3167cd72,// 324 PAY 321 

    0x21094064,// 325 PAY 322 

    0xf3a46c21,// 326 PAY 323 

    0x968cbe51,// 327 PAY 324 

    0xa323ea3b,// 328 PAY 325 

    0xacc772f3,// 329 PAY 326 

    0x63bc447f,// 330 PAY 327 

    0x2bdcc64d,// 331 PAY 328 

    0xdc20f80f,// 332 PAY 329 

    0x611e0178,// 333 PAY 330 

    0x1f3aa337,// 334 PAY 331 

    0x2629bcc1,// 335 PAY 332 

    0x62d2fb14,// 336 PAY 333 

    0x384417c2,// 337 PAY 334 

    0x2f5b797c,// 338 PAY 335 

    0x614db4d1,// 339 PAY 336 

    0xb5b061be,// 340 PAY 337 

    0x38dfc035,// 341 PAY 338 

    0x15b05609,// 342 PAY 339 

    0x936a27e1,// 343 PAY 340 

    0x946fd56c,// 344 PAY 341 

    0xe8626c78,// 345 PAY 342 

    0x7c47244f,// 346 PAY 343 

    0x6fd290e0,// 347 PAY 344 

    0xd40df44f,// 348 PAY 345 

    0x41ca1b3b,// 349 PAY 346 

    0xe03d8b15,// 350 PAY 347 

    0x2097f288,// 351 PAY 348 

    0xfb49f753,// 352 PAY 349 

    0x2aa486d3,// 353 PAY 350 

    0xaf55b21f,// 354 PAY 351 

    0x34a39c1b,// 355 PAY 352 

    0xc9d7bedf,// 356 PAY 353 

    0xe443048f,// 357 PAY 354 

    0xa9489540,// 358 PAY 355 

    0x92192063,// 359 PAY 356 

    0x3a5b8a62,// 360 PAY 357 

    0xe60e380b,// 361 PAY 358 

    0x85250d9d,// 362 PAY 359 

    0x145927b4,// 363 PAY 360 

    0xc967e939,// 364 PAY 361 

    0x0b0fcbaa,// 365 PAY 362 

    0x0100daca,// 366 PAY 363 

    0xd86886d4,// 367 PAY 364 

    0x1bdb8650,// 368 PAY 365 

    0x5a7e3775,// 369 PAY 366 

    0xb2bda177,// 370 PAY 367 

    0x276db8c4,// 371 PAY 368 

    0x16c629cd,// 372 PAY 369 

    0x38bee713,// 373 PAY 370 

    0x2556ea74,// 374 PAY 371 

    0x3b48e94c,// 375 PAY 372 

    0x10c8316d,// 376 PAY 373 

    0x3fc92b4d,// 377 PAY 374 

    0x0c88b02e,// 378 PAY 375 

    0xe113bbbf,// 379 PAY 376 

    0x7b0c63b9,// 380 PAY 377 

    0x55f5438e,// 381 PAY 378 

    0x12191ac5,// 382 PAY 379 

    0xa36937d3,// 383 PAY 380 

    0xaa6c6a38,// 384 PAY 381 

    0xa241ce17,// 385 PAY 382 

    0x9e7545c6,// 386 PAY 383 

    0x3d9ced0f,// 387 PAY 384 

    0xab78f534,// 388 PAY 385 

    0xf62d7e8e,// 389 PAY 386 

    0x6457d20b,// 390 PAY 387 

    0x7e2d2322,// 391 PAY 388 

    0x5d3a965e,// 392 PAY 389 

    0x3fd9c0b1,// 393 PAY 390 

    0x12baea22,// 394 PAY 391 

    0x9008b121,// 395 PAY 392 

    0xc1115367,// 396 PAY 393 

    0x69407d77,// 397 PAY 394 

    0xee559f81,// 398 PAY 395 

    0xd32b7c56,// 399 PAY 396 

    0x73746090,// 400 PAY 397 

    0x9cb95a44,// 401 PAY 398 

    0xdd67b5de,// 402 PAY 399 

    0xa3d14182,// 403 PAY 400 

    0xb62945e4,// 404 PAY 401 

    0xb2c53f79,// 405 PAY 402 

    0x4277b122,// 406 PAY 403 

    0x8b043f88,// 407 PAY 404 

    0x90c43bac,// 408 PAY 405 

    0x7b2ea867,// 409 PAY 406 

    0xf27064b6,// 410 PAY 407 

    0x5a1e1a92,// 411 PAY 408 

    0x00dc00b1,// 412 PAY 409 

    0x8f216257,// 413 PAY 410 

    0xa0e3d751,// 414 PAY 411 

    0x5be66629,// 415 PAY 412 

    0xa81dd09a,// 416 PAY 413 

    0x7c530000,// 417 PAY 414 

/// STA is 1 words. 

/// STA num_pkts       : 235 

/// STA pkt_idx        : 178 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1c 

    0x02c81ceb // 418 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt77_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x08 

/// ECH pdu_tag        : 0x00 

    0x00080000,// 2 ECH   1 

/// BDA is 172 words. 

/// BDA size     is 683 (0x2ab) 

/// BDA id       is 0xaa5 

    0x02ab0aa5,// 3 BDA   1 

/// PAY Generic Data size   : 683 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0xf4fe11c2,// 4 PAY   1 

    0x911ab0d8,// 5 PAY   2 

    0xfcbdbfac,// 6 PAY   3 

    0x1c24dfad,// 7 PAY   4 

    0xea1284b5,// 8 PAY   5 

    0x41c8b4f6,// 9 PAY   6 

    0x5e2eb704,// 10 PAY   7 

    0x00207c48,// 11 PAY   8 

    0xd25d2875,// 12 PAY   9 

    0xe16f2837,// 13 PAY  10 

    0x85830de6,// 14 PAY  11 

    0xf7ef0c99,// 15 PAY  12 

    0xc443a05b,// 16 PAY  13 

    0x6a24aa0f,// 17 PAY  14 

    0x2a0ea680,// 18 PAY  15 

    0x45e8e5dc,// 19 PAY  16 

    0xe7eb9fb1,// 20 PAY  17 

    0x2ddc9d57,// 21 PAY  18 

    0x20fff8ab,// 22 PAY  19 

    0xe7b753ae,// 23 PAY  20 

    0xe601207b,// 24 PAY  21 

    0x90845309,// 25 PAY  22 

    0x73072838,// 26 PAY  23 

    0xb3777a9d,// 27 PAY  24 

    0x5e6b46a3,// 28 PAY  25 

    0x4eb4abad,// 29 PAY  26 

    0xdfd3e496,// 30 PAY  27 

    0x0912555f,// 31 PAY  28 

    0x7f572fb5,// 32 PAY  29 

    0x961a230a,// 33 PAY  30 

    0xa18ee60f,// 34 PAY  31 

    0x57a1404a,// 35 PAY  32 

    0x1e4a53e7,// 36 PAY  33 

    0x9a02d908,// 37 PAY  34 

    0xa244210c,// 38 PAY  35 

    0xaf934b73,// 39 PAY  36 

    0x0672c857,// 40 PAY  37 

    0x217d0ed2,// 41 PAY  38 

    0x465b3f07,// 42 PAY  39 

    0x9dcf2a29,// 43 PAY  40 

    0xbac8a058,// 44 PAY  41 

    0x5639c86f,// 45 PAY  42 

    0x696e01f1,// 46 PAY  43 

    0xe71b8207,// 47 PAY  44 

    0x98691494,// 48 PAY  45 

    0xf648b06b,// 49 PAY  46 

    0x7ad3d103,// 50 PAY  47 

    0x96f6c29c,// 51 PAY  48 

    0x2a26d336,// 52 PAY  49 

    0xc2097981,// 53 PAY  50 

    0xdf17a32d,// 54 PAY  51 

    0xc338c828,// 55 PAY  52 

    0x371ae6f6,// 56 PAY  53 

    0xd069de54,// 57 PAY  54 

    0x3d4d07eb,// 58 PAY  55 

    0x09a2c976,// 59 PAY  56 

    0xda0d6083,// 60 PAY  57 

    0x120e961f,// 61 PAY  58 

    0xf63663b8,// 62 PAY  59 

    0x4e05297a,// 63 PAY  60 

    0xe8a415eb,// 64 PAY  61 

    0x9555a236,// 65 PAY  62 

    0x51f616a6,// 66 PAY  63 

    0xad2f01ee,// 67 PAY  64 

    0x30f979c8,// 68 PAY  65 

    0x10231dae,// 69 PAY  66 

    0xaa55d3b7,// 70 PAY  67 

    0x2b495841,// 71 PAY  68 

    0x5ccb89b4,// 72 PAY  69 

    0x32f7c11c,// 73 PAY  70 

    0x81fa97c7,// 74 PAY  71 

    0xfae273df,// 75 PAY  72 

    0x962fd9a6,// 76 PAY  73 

    0xfe71e040,// 77 PAY  74 

    0x5abc360a,// 78 PAY  75 

    0xd6e8ed31,// 79 PAY  76 

    0xec660527,// 80 PAY  77 

    0xf7be3185,// 81 PAY  78 

    0x9d2bdbdc,// 82 PAY  79 

    0x92d33669,// 83 PAY  80 

    0x38d1c3ef,// 84 PAY  81 

    0x76a541d8,// 85 PAY  82 

    0x6df547de,// 86 PAY  83 

    0xa17f8c64,// 87 PAY  84 

    0x25cc8184,// 88 PAY  85 

    0xcdd12eee,// 89 PAY  86 

    0xed8270e5,// 90 PAY  87 

    0x6331f0fe,// 91 PAY  88 

    0xa4f5d1af,// 92 PAY  89 

    0x293ce6e9,// 93 PAY  90 

    0x7c44decd,// 94 PAY  91 

    0x7f4f082f,// 95 PAY  92 

    0x64b546f6,// 96 PAY  93 

    0xaa69b6dc,// 97 PAY  94 

    0xb71168ba,// 98 PAY  95 

    0xb8f1bb57,// 99 PAY  96 

    0xd5ee0162,// 100 PAY  97 

    0x5ff6e431,// 101 PAY  98 

    0x0f966474,// 102 PAY  99 

    0x47cbf7a5,// 103 PAY 100 

    0x17698b99,// 104 PAY 101 

    0x56d52576,// 105 PAY 102 

    0xbe2f8512,// 106 PAY 103 

    0x5a84cd1d,// 107 PAY 104 

    0x3d77ec6f,// 108 PAY 105 

    0x9bc390ac,// 109 PAY 106 

    0x71c85cb8,// 110 PAY 107 

    0xe80ca504,// 111 PAY 108 

    0xf74fd65a,// 112 PAY 109 

    0x3aa35fc1,// 113 PAY 110 

    0x9adc5b8f,// 114 PAY 111 

    0x3bc89d16,// 115 PAY 112 

    0x336f0c7a,// 116 PAY 113 

    0x355bae64,// 117 PAY 114 

    0xaf0a55fd,// 118 PAY 115 

    0xec35d4ab,// 119 PAY 116 

    0x7f2e3ea7,// 120 PAY 117 

    0x8cf70ccc,// 121 PAY 118 

    0x1e79f152,// 122 PAY 119 

    0x9c462430,// 123 PAY 120 

    0xdb3a95fe,// 124 PAY 121 

    0x053af0d4,// 125 PAY 122 

    0xf46da3c1,// 126 PAY 123 

    0x52c14b69,// 127 PAY 124 

    0xeb96412c,// 128 PAY 125 

    0x56ce8a29,// 129 PAY 126 

    0xd903f2ca,// 130 PAY 127 

    0x58c7795a,// 131 PAY 128 

    0xa1f2f4e8,// 132 PAY 129 

    0x3dad58d3,// 133 PAY 130 

    0x747f2d75,// 134 PAY 131 

    0x88363454,// 135 PAY 132 

    0xcfc70946,// 136 PAY 133 

    0x5d5fc402,// 137 PAY 134 

    0xb97bb9e6,// 138 PAY 135 

    0xd0476651,// 139 PAY 136 

    0x4593e86d,// 140 PAY 137 

    0x5a50019d,// 141 PAY 138 

    0xd92b0f5f,// 142 PAY 139 

    0x839c8cd7,// 143 PAY 140 

    0x463b237e,// 144 PAY 141 

    0xf7db7ee6,// 145 PAY 142 

    0x34d8cdcd,// 146 PAY 143 

    0x48fad5e4,// 147 PAY 144 

    0x5e598d89,// 148 PAY 145 

    0x416bcec7,// 149 PAY 146 

    0x4e002f4d,// 150 PAY 147 

    0xfece6025,// 151 PAY 148 

    0xb661a235,// 152 PAY 149 

    0xe2be8050,// 153 PAY 150 

    0x6ada4aa8,// 154 PAY 151 

    0x9d856e3e,// 155 PAY 152 

    0xd855fcc3,// 156 PAY 153 

    0xbf1397b5,// 157 PAY 154 

    0xdba28a05,// 158 PAY 155 

    0xc9c1bf9c,// 159 PAY 156 

    0x7d479e15,// 160 PAY 157 

    0x2e99629b,// 161 PAY 158 

    0x4b158d0b,// 162 PAY 159 

    0x9896b22d,// 163 PAY 160 

    0xa924497b,// 164 PAY 161 

    0xfb15cf15,// 165 PAY 162 

    0x7acbc444,// 166 PAY 163 

    0xc6e7169e,// 167 PAY 164 

    0xa8e63711,// 168 PAY 165 

    0xc8cb31e4,// 169 PAY 166 

    0xa0b65c11,// 170 PAY 167 

    0x5e351f69,// 171 PAY 168 

    0x6ae8a687,// 172 PAY 169 

    0x9d449b1b,// 173 PAY 170 

    0x109e0600,// 174 PAY 171 

/// STA is 1 words. 

/// STA num_pkts       : 254 

/// STA pkt_idx        : 71 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbe 

    0x011dbefe // 175 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt78_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x00 

/// ECH pdu_tag        : 0x00 

    0x00000000,// 2 ECH   1 

/// BDA is 329 words. 

/// BDA size     is 1312 (0x520) 

/// BDA id       is 0xdef3 

    0x0520def3,// 3 BDA   1 

/// PAY Generic Data size   : 1312 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xa2db9046,// 4 PAY   1 

    0x2f66252a,// 5 PAY   2 

    0x6d7556ad,// 6 PAY   3 

    0x789c3e4d,// 7 PAY   4 

    0x790bd0c9,// 8 PAY   5 

    0x81d5e94b,// 9 PAY   6 

    0x719fdfbd,// 10 PAY   7 

    0xcd798e5d,// 11 PAY   8 

    0x1e3ca240,// 12 PAY   9 

    0xf9a928c3,// 13 PAY  10 

    0x90bcdd28,// 14 PAY  11 

    0xe0948443,// 15 PAY  12 

    0xe5ac604b,// 16 PAY  13 

    0x3ae0def2,// 17 PAY  14 

    0x2eb8633e,// 18 PAY  15 

    0xa6700e5a,// 19 PAY  16 

    0x3b54397c,// 20 PAY  17 

    0xc8b60fc6,// 21 PAY  18 

    0x21085baf,// 22 PAY  19 

    0x12bf5dca,// 23 PAY  20 

    0xde557bc8,// 24 PAY  21 

    0xd67b6412,// 25 PAY  22 

    0xcdafa13f,// 26 PAY  23 

    0x68766480,// 27 PAY  24 

    0x53d026c0,// 28 PAY  25 

    0x99092e8f,// 29 PAY  26 

    0xbd0cc55c,// 30 PAY  27 

    0x4ffda16c,// 31 PAY  28 

    0xad20b4da,// 32 PAY  29 

    0x78d89565,// 33 PAY  30 

    0x66ca5321,// 34 PAY  31 

    0x64dcef7a,// 35 PAY  32 

    0xf3b1237f,// 36 PAY  33 

    0xae9c5185,// 37 PAY  34 

    0x91cd7038,// 38 PAY  35 

    0x20fb8e78,// 39 PAY  36 

    0xf1b4a307,// 40 PAY  37 

    0x9e2de7ca,// 41 PAY  38 

    0x9431cbb2,// 42 PAY  39 

    0x866964dc,// 43 PAY  40 

    0xf8554ba1,// 44 PAY  41 

    0xd36996a4,// 45 PAY  42 

    0xc631a4d1,// 46 PAY  43 

    0xecd319ff,// 47 PAY  44 

    0x6d64a416,// 48 PAY  45 

    0x8d8bba94,// 49 PAY  46 

    0xfbb9b3c2,// 50 PAY  47 

    0x3dfbc0f7,// 51 PAY  48 

    0x15b97de7,// 52 PAY  49 

    0xb29d9f8b,// 53 PAY  50 

    0x1d5ca033,// 54 PAY  51 

    0x71df66cd,// 55 PAY  52 

    0xacaca427,// 56 PAY  53 

    0xba20771d,// 57 PAY  54 

    0x99d0d583,// 58 PAY  55 

    0x63f120ef,// 59 PAY  56 

    0xfcd87fea,// 60 PAY  57 

    0x2679a952,// 61 PAY  58 

    0xb8a574c6,// 62 PAY  59 

    0x10e4e619,// 63 PAY  60 

    0xeec8d520,// 64 PAY  61 

    0x1297c8ca,// 65 PAY  62 

    0xdf3fb662,// 66 PAY  63 

    0x76c27a84,// 67 PAY  64 

    0x544b1283,// 68 PAY  65 

    0x00bd454a,// 69 PAY  66 

    0x369fc379,// 70 PAY  67 

    0xa703f598,// 71 PAY  68 

    0x8fd9fe1c,// 72 PAY  69 

    0x8ede8ad7,// 73 PAY  70 

    0x89560e1d,// 74 PAY  71 

    0x33b639bd,// 75 PAY  72 

    0xcc26c893,// 76 PAY  73 

    0x00ccce94,// 77 PAY  74 

    0x95d21282,// 78 PAY  75 

    0xfa706d9c,// 79 PAY  76 

    0x07624f33,// 80 PAY  77 

    0xad4c96eb,// 81 PAY  78 

    0x5fdabfda,// 82 PAY  79 

    0x444c9d1e,// 83 PAY  80 

    0x878f3511,// 84 PAY  81 

    0x68e676f3,// 85 PAY  82 

    0xd8c29214,// 86 PAY  83 

    0x6ecb79dd,// 87 PAY  84 

    0xcbbf524c,// 88 PAY  85 

    0x5e5a740b,// 89 PAY  86 

    0x529dbf7e,// 90 PAY  87 

    0xbc9f61b7,// 91 PAY  88 

    0xdee1526c,// 92 PAY  89 

    0x9e37f370,// 93 PAY  90 

    0x8b444dc7,// 94 PAY  91 

    0xec48b501,// 95 PAY  92 

    0xa1db0b04,// 96 PAY  93 

    0x5e5bc9a0,// 97 PAY  94 

    0x3714c92e,// 98 PAY  95 

    0x01edfc9c,// 99 PAY  96 

    0xb882e70b,// 100 PAY  97 

    0xefb83b4d,// 101 PAY  98 

    0xee41fd0a,// 102 PAY  99 

    0x57dc0d3c,// 103 PAY 100 

    0x01a48ef8,// 104 PAY 101 

    0x5a9c089a,// 105 PAY 102 

    0x023edf4e,// 106 PAY 103 

    0xb7f4423e,// 107 PAY 104 

    0x97151d61,// 108 PAY 105 

    0x15f9f308,// 109 PAY 106 

    0x0ea43d57,// 110 PAY 107 

    0x3a408a08,// 111 PAY 108 

    0x241f9d0c,// 112 PAY 109 

    0x1ba14a4f,// 113 PAY 110 

    0x18cd0dbf,// 114 PAY 111 

    0xeb67cce7,// 115 PAY 112 

    0x7664a353,// 116 PAY 113 

    0x4e9848aa,// 117 PAY 114 

    0xf72eb42f,// 118 PAY 115 

    0x85cdc931,// 119 PAY 116 

    0x92b929bd,// 120 PAY 117 

    0x14df0a40,// 121 PAY 118 

    0x4a3993af,// 122 PAY 119 

    0xb803881c,// 123 PAY 120 

    0x182eca40,// 124 PAY 121 

    0xb84ebbdd,// 125 PAY 122 

    0x90773160,// 126 PAY 123 

    0x8a08c58d,// 127 PAY 124 

    0xafd6ed14,// 128 PAY 125 

    0x2f045ccd,// 129 PAY 126 

    0xcbed1eed,// 130 PAY 127 

    0xf6a46165,// 131 PAY 128 

    0x89160345,// 132 PAY 129 

    0x4f1f0b7f,// 133 PAY 130 

    0x503409fb,// 134 PAY 131 

    0xde7e5f93,// 135 PAY 132 

    0x5230fe01,// 136 PAY 133 

    0x277f0b6e,// 137 PAY 134 

    0xfddc1083,// 138 PAY 135 

    0x0a7469b8,// 139 PAY 136 

    0x29f93a28,// 140 PAY 137 

    0x255fb17c,// 141 PAY 138 

    0x887d1b5c,// 142 PAY 139 

    0x75c1cba3,// 143 PAY 140 

    0x35874dd7,// 144 PAY 141 

    0x58d5c3ff,// 145 PAY 142 

    0x1a2c8692,// 146 PAY 143 

    0xdd148081,// 147 PAY 144 

    0xd412a62c,// 148 PAY 145 

    0xc8530881,// 149 PAY 146 

    0x2f2d6580,// 150 PAY 147 

    0x734e939b,// 151 PAY 148 

    0x75e249fe,// 152 PAY 149 

    0x071e25af,// 153 PAY 150 

    0x6646b961,// 154 PAY 151 

    0xcf071ab4,// 155 PAY 152 

    0xe15e21ae,// 156 PAY 153 

    0x119cc60a,// 157 PAY 154 

    0xe46ecbc5,// 158 PAY 155 

    0x9f929a9e,// 159 PAY 156 

    0x25402f12,// 160 PAY 157 

    0x47c380eb,// 161 PAY 158 

    0x34367e7a,// 162 PAY 159 

    0x22c6aafa,// 163 PAY 160 

    0x47f03202,// 164 PAY 161 

    0x703359c4,// 165 PAY 162 

    0x8349daaf,// 166 PAY 163 

    0x2e7bb4aa,// 167 PAY 164 

    0x3aaa602f,// 168 PAY 165 

    0x74bac291,// 169 PAY 166 

    0xccac29f5,// 170 PAY 167 

    0x4f44d5ba,// 171 PAY 168 

    0xe23c5a34,// 172 PAY 169 

    0xcd7a2887,// 173 PAY 170 

    0x00f19dbc,// 174 PAY 171 

    0xd2f87737,// 175 PAY 172 

    0x17592504,// 176 PAY 173 

    0xcfdf7305,// 177 PAY 174 

    0x62638e9b,// 178 PAY 175 

    0x3e0207e0,// 179 PAY 176 

    0x94e3c7ad,// 180 PAY 177 

    0x67c4c52d,// 181 PAY 178 

    0x8344b0df,// 182 PAY 179 

    0xeabc5a01,// 183 PAY 180 

    0xce7cf649,// 184 PAY 181 

    0x34a073b5,// 185 PAY 182 

    0x8a4b75c7,// 186 PAY 183 

    0x4c15b6ee,// 187 PAY 184 

    0x40a6c429,// 188 PAY 185 

    0x6e92bda5,// 189 PAY 186 

    0xa8d7b357,// 190 PAY 187 

    0x9b7fa7d4,// 191 PAY 188 

    0xf44efd4d,// 192 PAY 189 

    0xa54f13f3,// 193 PAY 190 

    0xf776ad3d,// 194 PAY 191 

    0xff29ea7c,// 195 PAY 192 

    0x7fae64ba,// 196 PAY 193 

    0xe1540f5d,// 197 PAY 194 

    0xc9c42f71,// 198 PAY 195 

    0x443a6d8d,// 199 PAY 196 

    0x997fa81e,// 200 PAY 197 

    0xda922252,// 201 PAY 198 

    0x63bddb21,// 202 PAY 199 

    0xa9be3ebd,// 203 PAY 200 

    0xc6aea4f9,// 204 PAY 201 

    0x7edb48aa,// 205 PAY 202 

    0x84707f72,// 206 PAY 203 

    0xd86587d4,// 207 PAY 204 

    0xf82320b9,// 208 PAY 205 

    0xc960a6a9,// 209 PAY 206 

    0xcb221b65,// 210 PAY 207 

    0x1f0b2f05,// 211 PAY 208 

    0x6dcf9f4c,// 212 PAY 209 

    0x47917e5b,// 213 PAY 210 

    0x3a0c5e47,// 214 PAY 211 

    0xffab1fd8,// 215 PAY 212 

    0x175afb04,// 216 PAY 213 

    0x47614085,// 217 PAY 214 

    0xf9b0201d,// 218 PAY 215 

    0xb35e2772,// 219 PAY 216 

    0xa9e3a827,// 220 PAY 217 

    0xeb06bc5e,// 221 PAY 218 

    0xbfc441ec,// 222 PAY 219 

    0xd048f903,// 223 PAY 220 

    0x7173c485,// 224 PAY 221 

    0x6c289c27,// 225 PAY 222 

    0xdf5db08c,// 226 PAY 223 

    0xee0092e4,// 227 PAY 224 

    0x4b3ad036,// 228 PAY 225 

    0x5c1329ff,// 229 PAY 226 

    0x3824574a,// 230 PAY 227 

    0xfb0eee41,// 231 PAY 228 

    0x859ac52f,// 232 PAY 229 

    0x3160f719,// 233 PAY 230 

    0x1305e84e,// 234 PAY 231 

    0x00ea46a2,// 235 PAY 232 

    0xacbf155e,// 236 PAY 233 

    0xcfdeec00,// 237 PAY 234 

    0xe83d8215,// 238 PAY 235 

    0x99638edf,// 239 PAY 236 

    0xa9f1eb3e,// 240 PAY 237 

    0xc4f5853e,// 241 PAY 238 

    0x47b55192,// 242 PAY 239 

    0xf53c2b96,// 243 PAY 240 

    0xd2a57c2b,// 244 PAY 241 

    0x5b1bd7da,// 245 PAY 242 

    0xf2bc51bc,// 246 PAY 243 

    0x935cc2dc,// 247 PAY 244 

    0xc967a6d4,// 248 PAY 245 

    0x9c5cdcd2,// 249 PAY 246 

    0xb180b491,// 250 PAY 247 

    0x27d59b00,// 251 PAY 248 

    0x31e187e7,// 252 PAY 249 

    0xa78cb333,// 253 PAY 250 

    0xc3be7fc9,// 254 PAY 251 

    0x20afdc81,// 255 PAY 252 

    0x69cd65f2,// 256 PAY 253 

    0x6870f025,// 257 PAY 254 

    0x6245ee6e,// 258 PAY 255 

    0x87c2d30c,// 259 PAY 256 

    0x7e4f1dae,// 260 PAY 257 

    0xe75cd0cc,// 261 PAY 258 

    0xe57dc46a,// 262 PAY 259 

    0x9b304e69,// 263 PAY 260 

    0x94b8ddc6,// 264 PAY 261 

    0x16d8fffe,// 265 PAY 262 

    0xf3d0cc3a,// 266 PAY 263 

    0xe5304bdb,// 267 PAY 264 

    0x545a6b17,// 268 PAY 265 

    0x1a6f0432,// 269 PAY 266 

    0xb5f5f0f1,// 270 PAY 267 

    0x4662425f,// 271 PAY 268 

    0x5ea4c694,// 272 PAY 269 

    0xeaca011e,// 273 PAY 270 

    0x100f3bd3,// 274 PAY 271 

    0x816c4a07,// 275 PAY 272 

    0x46d15c1f,// 276 PAY 273 

    0x53a79ad7,// 277 PAY 274 

    0x9a854d83,// 278 PAY 275 

    0xcd4161d5,// 279 PAY 276 

    0x7bbd2b65,// 280 PAY 277 

    0x368ca0b0,// 281 PAY 278 

    0x586a82cd,// 282 PAY 279 

    0x46b67c62,// 283 PAY 280 

    0x30717e69,// 284 PAY 281 

    0xbb35ae75,// 285 PAY 282 

    0xb59ae379,// 286 PAY 283 

    0x0465ab1e,// 287 PAY 284 

    0xea1cd926,// 288 PAY 285 

    0xc7670f9d,// 289 PAY 286 

    0x34de4ecb,// 290 PAY 287 

    0xffd58ff6,// 291 PAY 288 

    0x79d2f99b,// 292 PAY 289 

    0xe9a20e98,// 293 PAY 290 

    0xb0b8e7f4,// 294 PAY 291 

    0x9fb0e760,// 295 PAY 292 

    0x81bcaf6d,// 296 PAY 293 

    0xf5f0186e,// 297 PAY 294 

    0x1971cbb0,// 298 PAY 295 

    0xcfbfa883,// 299 PAY 296 

    0x506234ba,// 300 PAY 297 

    0x898c05b5,// 301 PAY 298 

    0x4983cb2b,// 302 PAY 299 

    0x81b82c33,// 303 PAY 300 

    0xe3f06900,// 304 PAY 301 

    0x317e1cf4,// 305 PAY 302 

    0xa7946627,// 306 PAY 303 

    0x05e3d649,// 307 PAY 304 

    0x793e51f6,// 308 PAY 305 

    0x6bfc8552,// 309 PAY 306 

    0xfcd4c08f,// 310 PAY 307 

    0xb3de979a,// 311 PAY 308 

    0x30fa54da,// 312 PAY 309 

    0x2581ef16,// 313 PAY 310 

    0x50eccca8,// 314 PAY 311 

    0xc8b39c38,// 315 PAY 312 

    0x35501333,// 316 PAY 313 

    0x11889edb,// 317 PAY 314 

    0x08c74aa0,// 318 PAY 315 

    0x58fa1296,// 319 PAY 316 

    0xb80012c1,// 320 PAY 317 

    0xe412aae9,// 321 PAY 318 

    0x47de8047,// 322 PAY 319 

    0x141fc344,// 323 PAY 320 

    0x268174c3,// 324 PAY 321 

    0x2cc64798,// 325 PAY 322 

    0xabbe7f14,// 326 PAY 323 

    0xf1743de9,// 327 PAY 324 

    0xa4a329b5,// 328 PAY 325 

    0x2a08e618,// 329 PAY 326 

    0xeeadd366,// 330 PAY 327 

    0xbbf8ae6c,// 331 PAY 328 

/// HASH is  4 bytes 

    0xa54f13f3,// 332 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 55 

/// STA pkt_idx        : 168 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf4 

    0x02a0f437 // 333 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt79_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 323 words. 

/// BDA size     is 1287 (0x507) 

/// BDA id       is 0x4d3 

    0x050704d3,// 3 BDA   1 

/// PAY Generic Data size   : 1287 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x2a31b684,// 4 PAY   1 

    0x19b57234,// 5 PAY   2 

    0x98533061,// 6 PAY   3 

    0xdd4ec9ed,// 7 PAY   4 

    0x3787ec90,// 8 PAY   5 

    0xd5789aac,// 9 PAY   6 

    0x210f46ab,// 10 PAY   7 

    0xaaf74bd0,// 11 PAY   8 

    0x85d11338,// 12 PAY   9 

    0xb232111b,// 13 PAY  10 

    0xe187154e,// 14 PAY  11 

    0x5c583b2a,// 15 PAY  12 

    0xc45172b6,// 16 PAY  13 

    0xcfce8f28,// 17 PAY  14 

    0x651377f9,// 18 PAY  15 

    0xd5a550ef,// 19 PAY  16 

    0xa950fdeb,// 20 PAY  17 

    0x847273c3,// 21 PAY  18 

    0x99d2fe59,// 22 PAY  19 

    0x8f54d1fc,// 23 PAY  20 

    0x3ba48508,// 24 PAY  21 

    0x972ee34a,// 25 PAY  22 

    0xea4fceb0,// 26 PAY  23 

    0x23caaf93,// 27 PAY  24 

    0xfb4d5d7f,// 28 PAY  25 

    0xe9556205,// 29 PAY  26 

    0x8bc54877,// 30 PAY  27 

    0x85e6f3e2,// 31 PAY  28 

    0x50d591ce,// 32 PAY  29 

    0x16543faa,// 33 PAY  30 

    0x27855252,// 34 PAY  31 

    0x89a3ee53,// 35 PAY  32 

    0x6ca387c3,// 36 PAY  33 

    0x01ef738b,// 37 PAY  34 

    0x47464175,// 38 PAY  35 

    0x8eb93453,// 39 PAY  36 

    0x462e10d6,// 40 PAY  37 

    0xa591e735,// 41 PAY  38 

    0x3d3800db,// 42 PAY  39 

    0xc5164366,// 43 PAY  40 

    0x64210e02,// 44 PAY  41 

    0xca09626b,// 45 PAY  42 

    0x933d01db,// 46 PAY  43 

    0xdeeab75a,// 47 PAY  44 

    0x5f3e9f5a,// 48 PAY  45 

    0xe4941815,// 49 PAY  46 

    0xf132aaec,// 50 PAY  47 

    0xf338b9bf,// 51 PAY  48 

    0xe2b9dedf,// 52 PAY  49 

    0x8d50a987,// 53 PAY  50 

    0x32f7f7ee,// 54 PAY  51 

    0x8896a3ab,// 55 PAY  52 

    0x4bf29696,// 56 PAY  53 

    0x41f4e14f,// 57 PAY  54 

    0xe5602606,// 58 PAY  55 

    0x912b84ee,// 59 PAY  56 

    0x38c1aa12,// 60 PAY  57 

    0x1dd3b0f5,// 61 PAY  58 

    0x32ecf93b,// 62 PAY  59 

    0x7e437b63,// 63 PAY  60 

    0x59096046,// 64 PAY  61 

    0x0fb46115,// 65 PAY  62 

    0xdf7b73fd,// 66 PAY  63 

    0x209a8185,// 67 PAY  64 

    0x0ba7d483,// 68 PAY  65 

    0x51090e6d,// 69 PAY  66 

    0xa804149e,// 70 PAY  67 

    0xe41368c3,// 71 PAY  68 

    0x1c4a489d,// 72 PAY  69 

    0xda12f071,// 73 PAY  70 

    0xe4ae2c37,// 74 PAY  71 

    0x43d7ce2b,// 75 PAY  72 

    0x72a3f2c3,// 76 PAY  73 

    0xb7ffd6cf,// 77 PAY  74 

    0x88c81156,// 78 PAY  75 

    0xfac5139a,// 79 PAY  76 

    0x93a63550,// 80 PAY  77 

    0x0adb8815,// 81 PAY  78 

    0xda01c8d7,// 82 PAY  79 

    0xac740480,// 83 PAY  80 

    0x48c12fea,// 84 PAY  81 

    0x9ab93c9e,// 85 PAY  82 

    0x5a848fdf,// 86 PAY  83 

    0x48033ee8,// 87 PAY  84 

    0x31f316f7,// 88 PAY  85 

    0xd11b4f84,// 89 PAY  86 

    0xa6a0e9f6,// 90 PAY  87 

    0x97c7f6b5,// 91 PAY  88 

    0x67cbdb34,// 92 PAY  89 

    0x6678b306,// 93 PAY  90 

    0xc7e82d37,// 94 PAY  91 

    0xd2ff3a69,// 95 PAY  92 

    0x3469f16c,// 96 PAY  93 

    0x491d7323,// 97 PAY  94 

    0x0f9ea291,// 98 PAY  95 

    0xd707c7ec,// 99 PAY  96 

    0x22dfdc0c,// 100 PAY  97 

    0x6ea055e0,// 101 PAY  98 

    0x28e62867,// 102 PAY  99 

    0x30273cc1,// 103 PAY 100 

    0xe94241c8,// 104 PAY 101 

    0x5cbb090f,// 105 PAY 102 

    0x390a9037,// 106 PAY 103 

    0x2702f28f,// 107 PAY 104 

    0x5049654a,// 108 PAY 105 

    0x69991ecb,// 109 PAY 106 

    0xe5ccc072,// 110 PAY 107 

    0x4e766e22,// 111 PAY 108 

    0xa1dcc1ed,// 112 PAY 109 

    0x9981a7ef,// 113 PAY 110 

    0xea5296cf,// 114 PAY 111 

    0x739cc13b,// 115 PAY 112 

    0x288c04ea,// 116 PAY 113 

    0x9d6049be,// 117 PAY 114 

    0x210628b9,// 118 PAY 115 

    0xd47d6a26,// 119 PAY 116 

    0x49576ac4,// 120 PAY 117 

    0xce7d268f,// 121 PAY 118 

    0x652a11e3,// 122 PAY 119 

    0xb0156eb5,// 123 PAY 120 

    0x02629009,// 124 PAY 121 

    0xf8c61d79,// 125 PAY 122 

    0xc45ded1d,// 126 PAY 123 

    0x6ad36e41,// 127 PAY 124 

    0xfbd70447,// 128 PAY 125 

    0xfbd89278,// 129 PAY 126 

    0xea9f7ae7,// 130 PAY 127 

    0x60fac404,// 131 PAY 128 

    0x6d6798d5,// 132 PAY 129 

    0x8c79f1af,// 133 PAY 130 

    0x434e5d13,// 134 PAY 131 

    0xa6551347,// 135 PAY 132 

    0x5404e469,// 136 PAY 133 

    0x2c87a576,// 137 PAY 134 

    0xfab3ae88,// 138 PAY 135 

    0xad46684f,// 139 PAY 136 

    0xde77ee34,// 140 PAY 137 

    0x84a95396,// 141 PAY 138 

    0xdc79cfdb,// 142 PAY 139 

    0xdac9171c,// 143 PAY 140 

    0xb9c06e32,// 144 PAY 141 

    0x08b1072a,// 145 PAY 142 

    0x52211e95,// 146 PAY 143 

    0xed311425,// 147 PAY 144 

    0x8e3e0d0c,// 148 PAY 145 

    0x38fdb9e7,// 149 PAY 146 

    0xabb16910,// 150 PAY 147 

    0x8537fb64,// 151 PAY 148 

    0x79637e8d,// 152 PAY 149 

    0x3e0a2d83,// 153 PAY 150 

    0xa59ab660,// 154 PAY 151 

    0x10f9b988,// 155 PAY 152 

    0xed9c665b,// 156 PAY 153 

    0x04f8b697,// 157 PAY 154 

    0x9960c693,// 158 PAY 155 

    0xcccc8cba,// 159 PAY 156 

    0xf16a2197,// 160 PAY 157 

    0xbd22ee16,// 161 PAY 158 

    0xe16ff8ed,// 162 PAY 159 

    0xe5636005,// 163 PAY 160 

    0x6ffce3c0,// 164 PAY 161 

    0x65ab6a02,// 165 PAY 162 

    0x61af63f9,// 166 PAY 163 

    0xf1859b1f,// 167 PAY 164 

    0xb201dc02,// 168 PAY 165 

    0xbe0ba268,// 169 PAY 166 

    0x574d6dcc,// 170 PAY 167 

    0x727c9b7d,// 171 PAY 168 

    0xba99aaa7,// 172 PAY 169 

    0x7974e705,// 173 PAY 170 

    0x6dccc142,// 174 PAY 171 

    0xf4335d3d,// 175 PAY 172 

    0x3b25477c,// 176 PAY 173 

    0x18845d45,// 177 PAY 174 

    0x495058a7,// 178 PAY 175 

    0x7388c811,// 179 PAY 176 

    0x012c377e,// 180 PAY 177 

    0x5f6c5a15,// 181 PAY 178 

    0x7db9e8d8,// 182 PAY 179 

    0x3c3635cc,// 183 PAY 180 

    0xba162990,// 184 PAY 181 

    0x61bb3457,// 185 PAY 182 

    0x9c102f6a,// 186 PAY 183 

    0x1b0863fe,// 187 PAY 184 

    0xee6c611e,// 188 PAY 185 

    0x82324144,// 189 PAY 186 

    0xa7804274,// 190 PAY 187 

    0x3aa4ca96,// 191 PAY 188 

    0xed08c494,// 192 PAY 189 

    0x313169a7,// 193 PAY 190 

    0x74fda078,// 194 PAY 191 

    0x866ed2d6,// 195 PAY 192 

    0xae85130d,// 196 PAY 193 

    0x2748a1f0,// 197 PAY 194 

    0xaccf3dc5,// 198 PAY 195 

    0x8ced46fc,// 199 PAY 196 

    0x5bb60325,// 200 PAY 197 

    0x9c4389e5,// 201 PAY 198 

    0x59ed7d6e,// 202 PAY 199 

    0x53bb222b,// 203 PAY 200 

    0xe8e80a3c,// 204 PAY 201 

    0x6025dd6b,// 205 PAY 202 

    0xd3fa9964,// 206 PAY 203 

    0x43dd68b9,// 207 PAY 204 

    0x1afd1265,// 208 PAY 205 

    0x3bb3d4d4,// 209 PAY 206 

    0xf4f7f3b4,// 210 PAY 207 

    0xcb51863a,// 211 PAY 208 

    0x67f95ecb,// 212 PAY 209 

    0x324e5292,// 213 PAY 210 

    0xff1078a7,// 214 PAY 211 

    0x5cf4b16e,// 215 PAY 212 

    0x9cc761e4,// 216 PAY 213 

    0x0d718da3,// 217 PAY 214 

    0x6cd36d18,// 218 PAY 215 

    0x147144bd,// 219 PAY 216 

    0xe20b2cb5,// 220 PAY 217 

    0x5d902b34,// 221 PAY 218 

    0xdbcd395b,// 222 PAY 219 

    0x347060f3,// 223 PAY 220 

    0x84dfc1f9,// 224 PAY 221 

    0xcee5921e,// 225 PAY 222 

    0x992a9239,// 226 PAY 223 

    0xa89b4a78,// 227 PAY 224 

    0x376fe45c,// 228 PAY 225 

    0x670ca631,// 229 PAY 226 

    0xc151850f,// 230 PAY 227 

    0xbee99544,// 231 PAY 228 

    0x9211b06f,// 232 PAY 229 

    0x85bc3ef2,// 233 PAY 230 

    0xf02b6ecd,// 234 PAY 231 

    0x2059a0a2,// 235 PAY 232 

    0x861e9ec3,// 236 PAY 233 

    0xb378ad21,// 237 PAY 234 

    0xa6643462,// 238 PAY 235 

    0x6ce982e3,// 239 PAY 236 

    0x0293a872,// 240 PAY 237 

    0x32995eec,// 241 PAY 238 

    0xc034a89e,// 242 PAY 239 

    0xd5542cc9,// 243 PAY 240 

    0x5da09039,// 244 PAY 241 

    0xdf634ac0,// 245 PAY 242 

    0x3516ab99,// 246 PAY 243 

    0xc47cbc23,// 247 PAY 244 

    0xd9649782,// 248 PAY 245 

    0x32028d2d,// 249 PAY 246 

    0xa08685f3,// 250 PAY 247 

    0xd8408d0c,// 251 PAY 248 

    0xfc164dfd,// 252 PAY 249 

    0xc3cdde75,// 253 PAY 250 

    0x00faaf99,// 254 PAY 251 

    0xded6c919,// 255 PAY 252 

    0x2356a441,// 256 PAY 253 

    0x3ccfc340,// 257 PAY 254 

    0x6903a6d1,// 258 PAY 255 

    0x7343ff13,// 259 PAY 256 

    0x0a9546f3,// 260 PAY 257 

    0x0ef429e2,// 261 PAY 258 

    0xd2f3ca73,// 262 PAY 259 

    0x429f789b,// 263 PAY 260 

    0xf9fcd9e5,// 264 PAY 261 

    0x83eb7146,// 265 PAY 262 

    0x14fe9aa0,// 266 PAY 263 

    0xfdc0e7ee,// 267 PAY 264 

    0x1e86febb,// 268 PAY 265 

    0xaf762605,// 269 PAY 266 

    0xf03db7f6,// 270 PAY 267 

    0x1b18fe76,// 271 PAY 268 

    0x52d511bc,// 272 PAY 269 

    0x0ee92347,// 273 PAY 270 

    0xd481b7ec,// 274 PAY 271 

    0x0f895230,// 275 PAY 272 

    0x0b8488a0,// 276 PAY 273 

    0xc548ba0f,// 277 PAY 274 

    0x5a7987d5,// 278 PAY 275 

    0xd7f4e0f8,// 279 PAY 276 

    0x82cfffe0,// 280 PAY 277 

    0x3441b6e3,// 281 PAY 278 

    0xa87fc5ae,// 282 PAY 279 

    0x6e584197,// 283 PAY 280 

    0x2784c0cb,// 284 PAY 281 

    0x4a1d6000,// 285 PAY 282 

    0xfc60aa81,// 286 PAY 283 

    0x3884d975,// 287 PAY 284 

    0x8572f701,// 288 PAY 285 

    0x494e5c5a,// 289 PAY 286 

    0x00d10d5f,// 290 PAY 287 

    0xe768713a,// 291 PAY 288 

    0x3f5dc95f,// 292 PAY 289 

    0x23aca171,// 293 PAY 290 

    0x071baae8,// 294 PAY 291 

    0x11bdd6f9,// 295 PAY 292 

    0x88a7bc68,// 296 PAY 293 

    0xe2d0c475,// 297 PAY 294 

    0x22eaab71,// 298 PAY 295 

    0x63079f27,// 299 PAY 296 

    0x7ce32da1,// 300 PAY 297 

    0x7ad60baa,// 301 PAY 298 

    0x28663f7c,// 302 PAY 299 

    0xe41e4603,// 303 PAY 300 

    0x0ffbf733,// 304 PAY 301 

    0x37799363,// 305 PAY 302 

    0x3c627bff,// 306 PAY 303 

    0x5a9c1be5,// 307 PAY 304 

    0x606c14b4,// 308 PAY 305 

    0xdc54bbd1,// 309 PAY 306 

    0x5fa18cda,// 310 PAY 307 

    0xb7267cd6,// 311 PAY 308 

    0x2fa46942,// 312 PAY 309 

    0xbe7cd425,// 313 PAY 310 

    0xc38ac45d,// 314 PAY 311 

    0x72a8c5b1,// 315 PAY 312 

    0x2a165073,// 316 PAY 313 

    0xdf7895da,// 317 PAY 314 

    0x2774c568,// 318 PAY 315 

    0x9718f502,// 319 PAY 316 

    0x89290550,// 320 PAY 317 

    0x939da5a4,// 321 PAY 318 

    0xacb07082,// 322 PAY 319 

    0x752b8970,// 323 PAY 320 

    0xe04e2647,// 324 PAY 321 

    0x00733700,// 325 PAY 322 

/// STA is 1 words. 

/// STA num_pkts       : 45 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf 

    0x01690f2d // 326 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt80_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 438 words. 

/// BDA size     is 1748 (0x6d4) 

/// BDA id       is 0xcf5d 

    0x06d4cf5d,// 3 BDA   1 

/// PAY Generic Data size   : 1748 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x9eb8edda,// 4 PAY   1 

    0xa8cf3c8e,// 5 PAY   2 

    0x15b99ef9,// 6 PAY   3 

    0xdfac629c,// 7 PAY   4 

    0x7040633c,// 8 PAY   5 

    0xe5f2b7f8,// 9 PAY   6 

    0x0cb854ba,// 10 PAY   7 

    0xb7741103,// 11 PAY   8 

    0x8156c4c1,// 12 PAY   9 

    0x16286725,// 13 PAY  10 

    0x16d276d2,// 14 PAY  11 

    0x0d8dbd19,// 15 PAY  12 

    0x6e4bfade,// 16 PAY  13 

    0xd000def4,// 17 PAY  14 

    0x9cb4ebbd,// 18 PAY  15 

    0x80ff3c74,// 19 PAY  16 

    0x5936f5fe,// 20 PAY  17 

    0x9562435a,// 21 PAY  18 

    0xf1e2e251,// 22 PAY  19 

    0x57172034,// 23 PAY  20 

    0xd27e0edc,// 24 PAY  21 

    0xed5faf66,// 25 PAY  22 

    0x59412233,// 26 PAY  23 

    0x895f0336,// 27 PAY  24 

    0x23c5c0aa,// 28 PAY  25 

    0x31f8deec,// 29 PAY  26 

    0x3b4f6f75,// 30 PAY  27 

    0xa30f760e,// 31 PAY  28 

    0x69ec7430,// 32 PAY  29 

    0x82aa3970,// 33 PAY  30 

    0x94d941c3,// 34 PAY  31 

    0x6e446ca1,// 35 PAY  32 

    0x4635af4c,// 36 PAY  33 

    0xc7381dc5,// 37 PAY  34 

    0x55dabcf8,// 38 PAY  35 

    0x8082040d,// 39 PAY  36 

    0xd611b4b9,// 40 PAY  37 

    0x10f53a5a,// 41 PAY  38 

    0x8e3c078c,// 42 PAY  39 

    0x118ebe6c,// 43 PAY  40 

    0x4302fbb6,// 44 PAY  41 

    0x58bdb951,// 45 PAY  42 

    0x97ee2824,// 46 PAY  43 

    0x354a5099,// 47 PAY  44 

    0x332b2dab,// 48 PAY  45 

    0x1b3b6b13,// 49 PAY  46 

    0x3c952092,// 50 PAY  47 

    0xcf93b0d4,// 51 PAY  48 

    0xc426c46f,// 52 PAY  49 

    0x2c0e713b,// 53 PAY  50 

    0x04cb773a,// 54 PAY  51 

    0x52e88a24,// 55 PAY  52 

    0xda9e41ff,// 56 PAY  53 

    0x706533f0,// 57 PAY  54 

    0xccb1c430,// 58 PAY  55 

    0x86797484,// 59 PAY  56 

    0xadd517fe,// 60 PAY  57 

    0x5e643583,// 61 PAY  58 

    0xa0834d81,// 62 PAY  59 

    0xa68fe74c,// 63 PAY  60 

    0x29dbe5e2,// 64 PAY  61 

    0xe467e72d,// 65 PAY  62 

    0xa5c42125,// 66 PAY  63 

    0xcb03cb6e,// 67 PAY  64 

    0x75cb9a66,// 68 PAY  65 

    0x24bbe5dd,// 69 PAY  66 

    0x3f05d5e2,// 70 PAY  67 

    0xfe601717,// 71 PAY  68 

    0x9d60649b,// 72 PAY  69 

    0x74f28379,// 73 PAY  70 

    0x82c9deaf,// 74 PAY  71 

    0x13cc5c82,// 75 PAY  72 

    0x2c411448,// 76 PAY  73 

    0x92c113d1,// 77 PAY  74 

    0xba632ebf,// 78 PAY  75 

    0xe02ff658,// 79 PAY  76 

    0x7f3d5b7d,// 80 PAY  77 

    0x3a09eec3,// 81 PAY  78 

    0xcbf1deb2,// 82 PAY  79 

    0xb49f993a,// 83 PAY  80 

    0x5ce3d0dd,// 84 PAY  81 

    0xb53817c5,// 85 PAY  82 

    0xf698ceb0,// 86 PAY  83 

    0xc6766113,// 87 PAY  84 

    0xddcdab5b,// 88 PAY  85 

    0xadec1e70,// 89 PAY  86 

    0x96479cda,// 90 PAY  87 

    0xb431aa22,// 91 PAY  88 

    0x132a544e,// 92 PAY  89 

    0xfb78a1a8,// 93 PAY  90 

    0x22d82d45,// 94 PAY  91 

    0x67d8fdad,// 95 PAY  92 

    0xcfa030b9,// 96 PAY  93 

    0x9a16c319,// 97 PAY  94 

    0x2f3a5478,// 98 PAY  95 

    0x2374aac3,// 99 PAY  96 

    0x8ab60de3,// 100 PAY  97 

    0x13d71835,// 101 PAY  98 

    0xf86c920a,// 102 PAY  99 

    0xf3217782,// 103 PAY 100 

    0xe009e839,// 104 PAY 101 

    0x7f47abbd,// 105 PAY 102 

    0xde42ca9f,// 106 PAY 103 

    0x586cef52,// 107 PAY 104 

    0x6554a9aa,// 108 PAY 105 

    0xbe9bb724,// 109 PAY 106 

    0x7964ed60,// 110 PAY 107 

    0xdaddf3b3,// 111 PAY 108 

    0x5f948027,// 112 PAY 109 

    0x1c6f8b99,// 113 PAY 110 

    0x594f59ec,// 114 PAY 111 

    0x9fbc5145,// 115 PAY 112 

    0xfe8cb550,// 116 PAY 113 

    0x43893178,// 117 PAY 114 

    0x68836899,// 118 PAY 115 

    0xa6a891d6,// 119 PAY 116 

    0xae0b86cc,// 120 PAY 117 

    0xe6e9eb05,// 121 PAY 118 

    0x7b6b0bff,// 122 PAY 119 

    0xbac9fc98,// 123 PAY 120 

    0xdea3de6a,// 124 PAY 121 

    0x8492a91b,// 125 PAY 122 

    0xb6c50a3d,// 126 PAY 123 

    0x441997fd,// 127 PAY 124 

    0xda88272f,// 128 PAY 125 

    0x7bf6883a,// 129 PAY 126 

    0x3bd15af5,// 130 PAY 127 

    0x024a971a,// 131 PAY 128 

    0xe3b805b5,// 132 PAY 129 

    0x29aa10e0,// 133 PAY 130 

    0x6b710610,// 134 PAY 131 

    0xf63436ae,// 135 PAY 132 

    0xc4dd50cc,// 136 PAY 133 

    0xf54ec43d,// 137 PAY 134 

    0x8e71d3ba,// 138 PAY 135 

    0x3a63e8e0,// 139 PAY 136 

    0x8ea8f3a4,// 140 PAY 137 

    0x769f962b,// 141 PAY 138 

    0xd8fa233a,// 142 PAY 139 

    0x8dc2da27,// 143 PAY 140 

    0x940cee2e,// 144 PAY 141 

    0xc5a1de9b,// 145 PAY 142 

    0x66e2c30d,// 146 PAY 143 

    0x537614c9,// 147 PAY 144 

    0x10e62497,// 148 PAY 145 

    0xeeb49950,// 149 PAY 146 

    0x8435c15f,// 150 PAY 147 

    0x317017f6,// 151 PAY 148 

    0x5bed7c2e,// 152 PAY 149 

    0x18d3dc05,// 153 PAY 150 

    0xe45fb695,// 154 PAY 151 

    0xaba8c706,// 155 PAY 152 

    0x2e8baa42,// 156 PAY 153 

    0xbc418d19,// 157 PAY 154 

    0x31fad492,// 158 PAY 155 

    0xcceb9f3e,// 159 PAY 156 

    0xbe143f88,// 160 PAY 157 

    0x10b9d527,// 161 PAY 158 

    0x0de0615a,// 162 PAY 159 

    0xebeb30cc,// 163 PAY 160 

    0x521486cf,// 164 PAY 161 

    0x9af00c27,// 165 PAY 162 

    0xfb78c2d8,// 166 PAY 163 

    0x623585e7,// 167 PAY 164 

    0x1f508a35,// 168 PAY 165 

    0x76730218,// 169 PAY 166 

    0xd1317ab1,// 170 PAY 167 

    0x567f10e4,// 171 PAY 168 

    0x1b36c02d,// 172 PAY 169 

    0x08e016a3,// 173 PAY 170 

    0xeb7e44bf,// 174 PAY 171 

    0x636e253d,// 175 PAY 172 

    0x4396da5a,// 176 PAY 173 

    0xc181ea9a,// 177 PAY 174 

    0x2d3b57ab,// 178 PAY 175 

    0xd22a02f9,// 179 PAY 176 

    0x0a8f2240,// 180 PAY 177 

    0x17c18144,// 181 PAY 178 

    0xc3e0ada1,// 182 PAY 179 

    0xad3c52aa,// 183 PAY 180 

    0xc939adc7,// 184 PAY 181 

    0x53a7c5f3,// 185 PAY 182 

    0x343e3eac,// 186 PAY 183 

    0x07fa9ac5,// 187 PAY 184 

    0xdf9d4abf,// 188 PAY 185 

    0x1aa063a1,// 189 PAY 186 

    0xe88ed1e4,// 190 PAY 187 

    0xc34f2b53,// 191 PAY 188 

    0x33d10184,// 192 PAY 189 

    0x623cbec1,// 193 PAY 190 

    0xd5dbca9d,// 194 PAY 191 

    0x9424da99,// 195 PAY 192 

    0x7858b773,// 196 PAY 193 

    0x15097ee7,// 197 PAY 194 

    0x2f8d8594,// 198 PAY 195 

    0xb70bd23b,// 199 PAY 196 

    0x88602dbd,// 200 PAY 197 

    0x285e8169,// 201 PAY 198 

    0xf3e9bd9b,// 202 PAY 199 

    0xdf9754d2,// 203 PAY 200 

    0xb9db87a2,// 204 PAY 201 

    0x86040217,// 205 PAY 202 

    0xd16007e4,// 206 PAY 203 

    0x3b1d1e59,// 207 PAY 204 

    0xa031ad82,// 208 PAY 205 

    0xd9b4cc97,// 209 PAY 206 

    0x80a35ca0,// 210 PAY 207 

    0x8f245fba,// 211 PAY 208 

    0xc0d4f5a7,// 212 PAY 209 

    0xe607650e,// 213 PAY 210 

    0x7023d959,// 214 PAY 211 

    0xae43e514,// 215 PAY 212 

    0xdc53d9b8,// 216 PAY 213 

    0x03ea3e82,// 217 PAY 214 

    0x60d4bccb,// 218 PAY 215 

    0x99622489,// 219 PAY 216 

    0xeb6c3458,// 220 PAY 217 

    0x99720159,// 221 PAY 218 

    0x4724d605,// 222 PAY 219 

    0x1dc8b1d4,// 223 PAY 220 

    0x6ebad712,// 224 PAY 221 

    0xe42241c5,// 225 PAY 222 

    0x5948f633,// 226 PAY 223 

    0x25265d36,// 227 PAY 224 

    0x961964dd,// 228 PAY 225 

    0x756f372d,// 229 PAY 226 

    0x986d2bd9,// 230 PAY 227 

    0xc0937062,// 231 PAY 228 

    0xca018cf0,// 232 PAY 229 

    0x98a44638,// 233 PAY 230 

    0xb59c0ac7,// 234 PAY 231 

    0xb9924983,// 235 PAY 232 

    0xb0c8b555,// 236 PAY 233 

    0xa4cc5f6e,// 237 PAY 234 

    0x96cb929a,// 238 PAY 235 

    0xa9f9f95c,// 239 PAY 236 

    0x8daf86cf,// 240 PAY 237 

    0x1a7c20c8,// 241 PAY 238 

    0xb1b60d0c,// 242 PAY 239 

    0x2b554c84,// 243 PAY 240 

    0x10e31230,// 244 PAY 241 

    0x8ba22332,// 245 PAY 242 

    0xcb46a029,// 246 PAY 243 

    0x9dbedcfc,// 247 PAY 244 

    0xadb4988c,// 248 PAY 245 

    0xa4e92bd7,// 249 PAY 246 

    0xdee07436,// 250 PAY 247 

    0xd17fffcb,// 251 PAY 248 

    0xe2ef5730,// 252 PAY 249 

    0x17080d11,// 253 PAY 250 

    0x9ef9ff22,// 254 PAY 251 

    0x99459a19,// 255 PAY 252 

    0x9d48c20e,// 256 PAY 253 

    0xb7257d77,// 257 PAY 254 

    0xa8baf3f9,// 258 PAY 255 

    0xe762f1ba,// 259 PAY 256 

    0x5affb523,// 260 PAY 257 

    0x54a2b300,// 261 PAY 258 

    0xee7e71d9,// 262 PAY 259 

    0x2857a653,// 263 PAY 260 

    0xfc7e6cdb,// 264 PAY 261 

    0x118f31c6,// 265 PAY 262 

    0xe88ea549,// 266 PAY 263 

    0xa46a3709,// 267 PAY 264 

    0xd650139e,// 268 PAY 265 

    0xc41a7d2b,// 269 PAY 266 

    0xb838011b,// 270 PAY 267 

    0x5ce2d841,// 271 PAY 268 

    0x996c8b92,// 272 PAY 269 

    0xd918350f,// 273 PAY 270 

    0xe74866d1,// 274 PAY 271 

    0x276cf890,// 275 PAY 272 

    0x0590eb8d,// 276 PAY 273 

    0xa2f86e69,// 277 PAY 274 

    0xd45ef1e0,// 278 PAY 275 

    0x1da02d09,// 279 PAY 276 

    0x59216f16,// 280 PAY 277 

    0xfb3846b3,// 281 PAY 278 

    0x29548b55,// 282 PAY 279 

    0xc6eb2328,// 283 PAY 280 

    0xbaaa31d7,// 284 PAY 281 

    0xca024a53,// 285 PAY 282 

    0x475973ad,// 286 PAY 283 

    0x06c8fe74,// 287 PAY 284 

    0x31450756,// 288 PAY 285 

    0x5c7098f2,// 289 PAY 286 

    0x3663d52e,// 290 PAY 287 

    0xc1522d50,// 291 PAY 288 

    0xad5217ba,// 292 PAY 289 

    0xc8755afa,// 293 PAY 290 

    0x368c7622,// 294 PAY 291 

    0x2e87a51b,// 295 PAY 292 

    0xfa97cbb9,// 296 PAY 293 

    0x1986a696,// 297 PAY 294 

    0x3cd0a802,// 298 PAY 295 

    0x00d3ac23,// 299 PAY 296 

    0xb258a197,// 300 PAY 297 

    0xe29b1f2f,// 301 PAY 298 

    0xf73a015d,// 302 PAY 299 

    0x7611c9e1,// 303 PAY 300 

    0xa29d69b3,// 304 PAY 301 

    0x896a57ac,// 305 PAY 302 

    0x3eaee271,// 306 PAY 303 

    0xf0872f02,// 307 PAY 304 

    0x68da8af5,// 308 PAY 305 

    0x0149de5e,// 309 PAY 306 

    0x7a4aa272,// 310 PAY 307 

    0x2fcf4cc5,// 311 PAY 308 

    0xa32e6034,// 312 PAY 309 

    0xf939e570,// 313 PAY 310 

    0x5ae9709a,// 314 PAY 311 

    0x37290729,// 315 PAY 312 

    0xe27d6154,// 316 PAY 313 

    0x573dbb0e,// 317 PAY 314 

    0x7fe132d0,// 318 PAY 315 

    0x469e6318,// 319 PAY 316 

    0x0cf2b0f9,// 320 PAY 317 

    0xa72977d2,// 321 PAY 318 

    0x7d37d0d4,// 322 PAY 319 

    0xac9d1a77,// 323 PAY 320 

    0xfbbaabdb,// 324 PAY 321 

    0x00e75740,// 325 PAY 322 

    0xac1a0f9d,// 326 PAY 323 

    0xa9588fa8,// 327 PAY 324 

    0x455ca353,// 328 PAY 325 

    0x8b7fe34e,// 329 PAY 326 

    0x16ed5286,// 330 PAY 327 

    0xe7b7edc7,// 331 PAY 328 

    0xab0d6ec3,// 332 PAY 329 

    0x393b4d33,// 333 PAY 330 

    0xc0f2ed49,// 334 PAY 331 

    0x4459f623,// 335 PAY 332 

    0x1c41e7e4,// 336 PAY 333 

    0x49232bc0,// 337 PAY 334 

    0x2f774fc4,// 338 PAY 335 

    0xc57af5da,// 339 PAY 336 

    0x5a64080f,// 340 PAY 337 

    0x9794249d,// 341 PAY 338 

    0x7659b79e,// 342 PAY 339 

    0x8edfa118,// 343 PAY 340 

    0xe3a3039a,// 344 PAY 341 

    0x5e71a979,// 345 PAY 342 

    0x35e6b41d,// 346 PAY 343 

    0xb9db8b76,// 347 PAY 344 

    0x00220556,// 348 PAY 345 

    0x601a3a3e,// 349 PAY 346 

    0x406b2fcb,// 350 PAY 347 

    0x9ba5f5e5,// 351 PAY 348 

    0x0d0b97da,// 352 PAY 349 

    0xbc40269c,// 353 PAY 350 

    0xa6bc5c69,// 354 PAY 351 

    0x1f84b679,// 355 PAY 352 

    0xf675352e,// 356 PAY 353 

    0xa16c0b27,// 357 PAY 354 

    0x9d2b06bf,// 358 PAY 355 

    0x3a801c45,// 359 PAY 356 

    0xbf0f16b1,// 360 PAY 357 

    0x0e4de8bc,// 361 PAY 358 

    0x8f63df05,// 362 PAY 359 

    0xbf2ba1bd,// 363 PAY 360 

    0xda616aec,// 364 PAY 361 

    0x1cf31104,// 365 PAY 362 

    0xcb59afa8,// 366 PAY 363 

    0x48baafbe,// 367 PAY 364 

    0x4e1ca358,// 368 PAY 365 

    0x398a480f,// 369 PAY 366 

    0xdd6bd614,// 370 PAY 367 

    0xd1c818d7,// 371 PAY 368 

    0xa8765f23,// 372 PAY 369 

    0xaaf16b14,// 373 PAY 370 

    0x19240aa0,// 374 PAY 371 

    0xea66c591,// 375 PAY 372 

    0xc2f33cb0,// 376 PAY 373 

    0x1aae809b,// 377 PAY 374 

    0x39e47c36,// 378 PAY 375 

    0xcec22f86,// 379 PAY 376 

    0x64581cb3,// 380 PAY 377 

    0xea1a5844,// 381 PAY 378 

    0xb91cfb8e,// 382 PAY 379 

    0xa1005c4c,// 383 PAY 380 

    0x4e36621c,// 384 PAY 381 

    0x063a8cdd,// 385 PAY 382 

    0x9a488820,// 386 PAY 383 

    0x5c4dbd85,// 387 PAY 384 

    0xdedfc6f7,// 388 PAY 385 

    0xad661a33,// 389 PAY 386 

    0xe21ddd97,// 390 PAY 387 

    0xa041aa06,// 391 PAY 388 

    0x7ac8703f,// 392 PAY 389 

    0x4bda2cc5,// 393 PAY 390 

    0x15e98f13,// 394 PAY 391 

    0x96a715bc,// 395 PAY 392 

    0x3ac6e9dd,// 396 PAY 393 

    0x7ea8a8d2,// 397 PAY 394 

    0xd7c14b90,// 398 PAY 395 

    0x037ccff0,// 399 PAY 396 

    0x6688ba51,// 400 PAY 397 

    0x025a28f2,// 401 PAY 398 

    0x4c2cb6c1,// 402 PAY 399 

    0x461b90dd,// 403 PAY 400 

    0x949721a4,// 404 PAY 401 

    0x97a08994,// 405 PAY 402 

    0xf48e479e,// 406 PAY 403 

    0x21527ad6,// 407 PAY 404 

    0xe91437a4,// 408 PAY 405 

    0xf89f161f,// 409 PAY 406 

    0xff154feb,// 410 PAY 407 

    0xdd0c7a92,// 411 PAY 408 

    0x1b2af819,// 412 PAY 409 

    0x06f19ce8,// 413 PAY 410 

    0x318ee3e9,// 414 PAY 411 

    0x3aa7f6f4,// 415 PAY 412 

    0xfe48d50f,// 416 PAY 413 

    0x9b7b950f,// 417 PAY 414 

    0x6b9859f8,// 418 PAY 415 

    0x3167f33a,// 419 PAY 416 

    0x362e2ad7,// 420 PAY 417 

    0x20cabdd3,// 421 PAY 418 

    0xe8052a12,// 422 PAY 419 

    0x362c3327,// 423 PAY 420 

    0x97c1ca03,// 424 PAY 421 

    0x23a59869,// 425 PAY 422 

    0xa22c3794,// 426 PAY 423 

    0x9fce7470,// 427 PAY 424 

    0x12f8e275,// 428 PAY 425 

    0x24547ca1,// 429 PAY 426 

    0x5ab73dd8,// 430 PAY 427 

    0x0786db4e,// 431 PAY 428 

    0x701f0304,// 432 PAY 429 

    0x95ebf6e7,// 433 PAY 430 

    0x24531661,// 434 PAY 431 

    0x9c7f1382,// 435 PAY 432 

    0x54974523,// 436 PAY 433 

    0x868c3e98,// 437 PAY 434 

    0x96bea438,// 438 PAY 435 

    0xbe40792f,// 439 PAY 436 

    0xdde9a14f,// 440 PAY 437 

/// STA is 1 words. 

/// STA num_pkts       : 133 

/// STA pkt_idx        : 255 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x03fc0485 // 441 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt81_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 54 words. 

/// BDA size     is 210 (0xd2) 

/// BDA id       is 0xe599 

    0x00d2e599,// 3 BDA   1 

/// PAY Generic Data size   : 210 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x35f8108d,// 4 PAY   1 

    0xf2c9a7cc,// 5 PAY   2 

    0x977c7ef9,// 6 PAY   3 

    0x4b7ee154,// 7 PAY   4 

    0xa60ca1a1,// 8 PAY   5 

    0xb5182951,// 9 PAY   6 

    0x51653ee6,// 10 PAY   7 

    0xb087a969,// 11 PAY   8 

    0x46f6ee49,// 12 PAY   9 

    0x84ad570c,// 13 PAY  10 

    0xb07cd3d0,// 14 PAY  11 

    0xf9dd2feb,// 15 PAY  12 

    0x8a6421ef,// 16 PAY  13 

    0x9e5467c0,// 17 PAY  14 

    0x10ec8c0c,// 18 PAY  15 

    0xeee0a8b0,// 19 PAY  16 

    0xf22e8e05,// 20 PAY  17 

    0x8da70b34,// 21 PAY  18 

    0x2d974a3d,// 22 PAY  19 

    0xbee534dc,// 23 PAY  20 

    0x77a29560,// 24 PAY  21 

    0x3532d411,// 25 PAY  22 

    0xb10792ed,// 26 PAY  23 

    0xff7e1d21,// 27 PAY  24 

    0x57167ede,// 28 PAY  25 

    0x4b82c125,// 29 PAY  26 

    0x79e5f0e9,// 30 PAY  27 

    0xb7e56798,// 31 PAY  28 

    0x4471561f,// 32 PAY  29 

    0x0b96b0cf,// 33 PAY  30 

    0x60922cc7,// 34 PAY  31 

    0x343df9d9,// 35 PAY  32 

    0x729f2523,// 36 PAY  33 

    0xfec0dc78,// 37 PAY  34 

    0xdbb687f5,// 38 PAY  35 

    0x7fe3ffd8,// 39 PAY  36 

    0x4e1e14d1,// 40 PAY  37 

    0x56e85325,// 41 PAY  38 

    0x3f2e30ba,// 42 PAY  39 

    0xe375a7d6,// 43 PAY  40 

    0x424856bf,// 44 PAY  41 

    0x404e8288,// 45 PAY  42 

    0x52c00c2b,// 46 PAY  43 

    0xb59f013d,// 47 PAY  44 

    0x87d386b8,// 48 PAY  45 

    0x9a331e3f,// 49 PAY  46 

    0x279d83da,// 50 PAY  47 

    0x5c4c3a20,// 51 PAY  48 

    0x5958381c,// 52 PAY  49 

    0x40f49251,// 53 PAY  50 

    0x9897757d,// 54 PAY  51 

    0x53a98f19,// 55 PAY  52 

    0x054e0000,// 56 PAY  53 

/// STA is 1 words. 

/// STA num_pkts       : 210 

/// STA pkt_idx        : 251 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x1f 

    0x03ed1fd2 // 57 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt82_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 155 words. 

/// BDA size     is 616 (0x268) 

/// BDA id       is 0x8517 

    0x02688517,// 3 BDA   1 

/// PAY Generic Data size   : 616 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x33812981,// 4 PAY   1 

    0x3f00df34,// 5 PAY   2 

    0x77ea94bc,// 6 PAY   3 

    0x7e15ce08,// 7 PAY   4 

    0x66c27f93,// 8 PAY   5 

    0x92c7b865,// 9 PAY   6 

    0xac536fc1,// 10 PAY   7 

    0x8f5c2a00,// 11 PAY   8 

    0xaf5b064d,// 12 PAY   9 

    0x91d0ba47,// 13 PAY  10 

    0x4c0bd185,// 14 PAY  11 

    0x2ae63cae,// 15 PAY  12 

    0x2e3b9a20,// 16 PAY  13 

    0x7571dfaf,// 17 PAY  14 

    0xa28488bf,// 18 PAY  15 

    0xd2fd3cfd,// 19 PAY  16 

    0xa6d462bb,// 20 PAY  17 

    0x7f2da2ef,// 21 PAY  18 

    0x82569725,// 22 PAY  19 

    0xe4ceb625,// 23 PAY  20 

    0xebb3855e,// 24 PAY  21 

    0xfbefbd57,// 25 PAY  22 

    0x465be8c6,// 26 PAY  23 

    0x2eccbbd8,// 27 PAY  24 

    0xf4740ede,// 28 PAY  25 

    0x70b08037,// 29 PAY  26 

    0xaa34bfe7,// 30 PAY  27 

    0x63f7d82c,// 31 PAY  28 

    0x60419cb7,// 32 PAY  29 

    0x7aaf6b07,// 33 PAY  30 

    0x030f71a9,// 34 PAY  31 

    0xe0c1fc16,// 35 PAY  32 

    0xaf5290fa,// 36 PAY  33 

    0xd0e0404d,// 37 PAY  34 

    0x7117fcad,// 38 PAY  35 

    0x76ed0e52,// 39 PAY  36 

    0xac3ca39c,// 40 PAY  37 

    0xfa711a06,// 41 PAY  38 

    0xba1d6117,// 42 PAY  39 

    0x1d943811,// 43 PAY  40 

    0x36e9e4c8,// 44 PAY  41 

    0xbf672ac8,// 45 PAY  42 

    0x4ddbf050,// 46 PAY  43 

    0xaa7deae7,// 47 PAY  44 

    0xfab57244,// 48 PAY  45 

    0x88688c56,// 49 PAY  46 

    0x6ade3304,// 50 PAY  47 

    0xb6b263f8,// 51 PAY  48 

    0xbbf3d729,// 52 PAY  49 

    0x4ebfed49,// 53 PAY  50 

    0x694676e1,// 54 PAY  51 

    0x7145dd79,// 55 PAY  52 

    0x5993cb10,// 56 PAY  53 

    0x93867cd5,// 57 PAY  54 

    0x471a81f0,// 58 PAY  55 

    0x705c8e99,// 59 PAY  56 

    0x9e2a9e94,// 60 PAY  57 

    0x730033fb,// 61 PAY  58 

    0x580dd81f,// 62 PAY  59 

    0xe3bf6959,// 63 PAY  60 

    0x841b14c4,// 64 PAY  61 

    0x28db12fc,// 65 PAY  62 

    0xa66c2492,// 66 PAY  63 

    0x50c07a63,// 67 PAY  64 

    0x746feb90,// 68 PAY  65 

    0x817ea6fd,// 69 PAY  66 

    0x5a283f95,// 70 PAY  67 

    0x4111666c,// 71 PAY  68 

    0x5738c89f,// 72 PAY  69 

    0x41cc9042,// 73 PAY  70 

    0xb5993a1e,// 74 PAY  71 

    0x6551e9cb,// 75 PAY  72 

    0xa0a3031b,// 76 PAY  73 

    0x6a29a424,// 77 PAY  74 

    0x814d27ff,// 78 PAY  75 

    0x2ddce56b,// 79 PAY  76 

    0x9c5b6e42,// 80 PAY  77 

    0x730e7f9e,// 81 PAY  78 

    0x3ca77e6b,// 82 PAY  79 

    0x7d862b99,// 83 PAY  80 

    0x4394a8eb,// 84 PAY  81 

    0xdc282878,// 85 PAY  82 

    0xd1733c0f,// 86 PAY  83 

    0x4af923e8,// 87 PAY  84 

    0x92aa0ef7,// 88 PAY  85 

    0x08a57c77,// 89 PAY  86 

    0x87b57936,// 90 PAY  87 

    0xe4218da0,// 91 PAY  88 

    0xaa2b156a,// 92 PAY  89 

    0x7fd9ef9a,// 93 PAY  90 

    0x97b9ac02,// 94 PAY  91 

    0xa9b47cbb,// 95 PAY  92 

    0xbbac6b5f,// 96 PAY  93 

    0xe8ff30d5,// 97 PAY  94 

    0x60f6bf8b,// 98 PAY  95 

    0x66d100e9,// 99 PAY  96 

    0x6a19c051,// 100 PAY  97 

    0x078bb5ea,// 101 PAY  98 

    0x2651b00b,// 102 PAY  99 

    0xd2c58557,// 103 PAY 100 

    0xe4e37923,// 104 PAY 101 

    0x9c116715,// 105 PAY 102 

    0x7d14d517,// 106 PAY 103 

    0xb08f0700,// 107 PAY 104 

    0xfd8b2d9e,// 108 PAY 105 

    0x4830797c,// 109 PAY 106 

    0x373aa817,// 110 PAY 107 

    0xf447c14a,// 111 PAY 108 

    0x0ee02168,// 112 PAY 109 

    0x58a7dd5e,// 113 PAY 110 

    0x73874fe4,// 114 PAY 111 

    0x38b11290,// 115 PAY 112 

    0x80980dab,// 116 PAY 113 

    0x0b1fac69,// 117 PAY 114 

    0x094b5e5f,// 118 PAY 115 

    0x1242739e,// 119 PAY 116 

    0x78029f6d,// 120 PAY 117 

    0x634055a2,// 121 PAY 118 

    0xb21bdf92,// 122 PAY 119 

    0x97a65fc2,// 123 PAY 120 

    0xdcbf2bce,// 124 PAY 121 

    0x4c57c68a,// 125 PAY 122 

    0xb2796f3f,// 126 PAY 123 

    0x19c5ea39,// 127 PAY 124 

    0xd7f4d165,// 128 PAY 125 

    0x195c721d,// 129 PAY 126 

    0xb263fc2a,// 130 PAY 127 

    0x3e856e31,// 131 PAY 128 

    0xe5f23b95,// 132 PAY 129 

    0xeb6adb09,// 133 PAY 130 

    0x42e7a1c5,// 134 PAY 131 

    0x34d88ce8,// 135 PAY 132 

    0xc34a8835,// 136 PAY 133 

    0xcd649757,// 137 PAY 134 

    0xa409359d,// 138 PAY 135 

    0x39e3f3fa,// 139 PAY 136 

    0x1fb6db10,// 140 PAY 137 

    0x7dc17ed3,// 141 PAY 138 

    0x48cb5044,// 142 PAY 139 

    0x2e25e8d7,// 143 PAY 140 

    0x36e9acd5,// 144 PAY 141 

    0x020fdf8c,// 145 PAY 142 

    0xfa6e8f30,// 146 PAY 143 

    0xda581872,// 147 PAY 144 

    0x6309debf,// 148 PAY 145 

    0x14593f2d,// 149 PAY 146 

    0xefe2d02e,// 150 PAY 147 

    0x399f9d6d,// 151 PAY 148 

    0x7e665049,// 152 PAY 149 

    0x1e2635b7,// 153 PAY 150 

    0x9dab7e90,// 154 PAY 151 

    0xeb8e119e,// 155 PAY 152 

    0x1d60c070,// 156 PAY 153 

    0xc5454d54,// 157 PAY 154 

/// HASH is  16 bytes 

    0x730e7f9e,// 158 HSH   1 

    0x3ca77e6b,// 159 HSH   2 

    0x7d862b99,// 160 HSH   3 

    0x4394a8eb,// 161 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 81 

/// STA pkt_idx        : 247 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xdc 

    0x03dcdc51 // 162 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt83_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0f 

/// ECH pdu_tag        : 0x00 

    0x000f0000,// 2 ECH   1 

/// BDA is 488 words. 

/// BDA size     is 1946 (0x79a) 

/// BDA id       is 0x8d6d 

    0x079a8d6d,// 3 BDA   1 

/// PAY Generic Data size   : 1946 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x369000fe,// 4 PAY   1 

    0x4b96fdda,// 5 PAY   2 

    0xf1125a51,// 6 PAY   3 

    0x30dcf55b,// 7 PAY   4 

    0xb9a207c4,// 8 PAY   5 

    0xc8c9e3d8,// 9 PAY   6 

    0xfcf5f07e,// 10 PAY   7 

    0x5cbdc94b,// 11 PAY   8 

    0x4e5e51f4,// 12 PAY   9 

    0x82b72e84,// 13 PAY  10 

    0x31ec8c8f,// 14 PAY  11 

    0xa098be60,// 15 PAY  12 

    0x155cde0b,// 16 PAY  13 

    0x7bc9d511,// 17 PAY  14 

    0xdfffd7c3,// 18 PAY  15 

    0xafa3cf61,// 19 PAY  16 

    0x03652780,// 20 PAY  17 

    0x8b296bf3,// 21 PAY  18 

    0x26844df6,// 22 PAY  19 

    0x90f73ce9,// 23 PAY  20 

    0x38ad34e7,// 24 PAY  21 

    0xe8fe61f4,// 25 PAY  22 

    0x896f79cb,// 26 PAY  23 

    0x9fc9e4a4,// 27 PAY  24 

    0x32205cad,// 28 PAY  25 

    0x8f010d42,// 29 PAY  26 

    0x04c4a0e7,// 30 PAY  27 

    0x40e35df3,// 31 PAY  28 

    0x2d6846a2,// 32 PAY  29 

    0xb342373a,// 33 PAY  30 

    0x5ec7f7be,// 34 PAY  31 

    0xabeaddce,// 35 PAY  32 

    0x95c5cc85,// 36 PAY  33 

    0xf1ab8ef0,// 37 PAY  34 

    0xc9bc788b,// 38 PAY  35 

    0x00ca02b4,// 39 PAY  36 

    0x19e631f6,// 40 PAY  37 

    0x808039ed,// 41 PAY  38 

    0x58597b2d,// 42 PAY  39 

    0xb9e63dec,// 43 PAY  40 

    0xe79ba792,// 44 PAY  41 

    0x193a98e4,// 45 PAY  42 

    0x0185e734,// 46 PAY  43 

    0xffb1b515,// 47 PAY  44 

    0x1117e0d8,// 48 PAY  45 

    0x4119eefe,// 49 PAY  46 

    0xe000ec93,// 50 PAY  47 

    0xacddf1f1,// 51 PAY  48 

    0x6cdb5a68,// 52 PAY  49 

    0x318954e4,// 53 PAY  50 

    0x27cb669d,// 54 PAY  51 

    0x7a51aa2b,// 55 PAY  52 

    0x9705d3df,// 56 PAY  53 

    0x2694cd72,// 57 PAY  54 

    0xa0fcc600,// 58 PAY  55 

    0xccd94411,// 59 PAY  56 

    0xdff2e8e0,// 60 PAY  57 

    0xebc87e88,// 61 PAY  58 

    0xe415feac,// 62 PAY  59 

    0xf9164741,// 63 PAY  60 

    0xa80090f3,// 64 PAY  61 

    0x82089177,// 65 PAY  62 

    0x1327e04e,// 66 PAY  63 

    0x84910e1c,// 67 PAY  64 

    0x7a93344c,// 68 PAY  65 

    0x92919e9b,// 69 PAY  66 

    0x6a4fe762,// 70 PAY  67 

    0x0d501787,// 71 PAY  68 

    0xf7f00a73,// 72 PAY  69 

    0x6e270bc8,// 73 PAY  70 

    0xe65f869e,// 74 PAY  71 

    0x4f2ab923,// 75 PAY  72 

    0x33c9322a,// 76 PAY  73 

    0xa4d96969,// 77 PAY  74 

    0x8dad229b,// 78 PAY  75 

    0xe6baaf19,// 79 PAY  76 

    0xc3590822,// 80 PAY  77 

    0x641ecc02,// 81 PAY  78 

    0x89f9cc60,// 82 PAY  79 

    0x31ef92ae,// 83 PAY  80 

    0xebc28077,// 84 PAY  81 

    0xe8c36d02,// 85 PAY  82 

    0x02af50a4,// 86 PAY  83 

    0x02379e75,// 87 PAY  84 

    0xec882d29,// 88 PAY  85 

    0xedfc20d6,// 89 PAY  86 

    0x193a46b6,// 90 PAY  87 

    0xe152f48f,// 91 PAY  88 

    0xb614b718,// 92 PAY  89 

    0xda96321f,// 93 PAY  90 

    0xff2c0631,// 94 PAY  91 

    0x2dca2849,// 95 PAY  92 

    0x45804a72,// 96 PAY  93 

    0xe939a65a,// 97 PAY  94 

    0xb225ca09,// 98 PAY  95 

    0x6c906495,// 99 PAY  96 

    0x4055e408,// 100 PAY  97 

    0xbde3b7a4,// 101 PAY  98 

    0xb33d552a,// 102 PAY  99 

    0xaa56404f,// 103 PAY 100 

    0x62c902b8,// 104 PAY 101 

    0x75841cc2,// 105 PAY 102 

    0x91f118d9,// 106 PAY 103 

    0xcd6ac39b,// 107 PAY 104 

    0xdc6bfaa1,// 108 PAY 105 

    0xa7e3a5f1,// 109 PAY 106 

    0x67da098f,// 110 PAY 107 

    0x675e03fc,// 111 PAY 108 

    0x132567c8,// 112 PAY 109 

    0x9bb4c358,// 113 PAY 110 

    0xd6256d53,// 114 PAY 111 

    0xf4c279fb,// 115 PAY 112 

    0xe6f8814c,// 116 PAY 113 

    0xe051f52e,// 117 PAY 114 

    0xb3a2d05c,// 118 PAY 115 

    0x20b625f3,// 119 PAY 116 

    0xdd497ed5,// 120 PAY 117 

    0xfe7de7bd,// 121 PAY 118 

    0xbaab08a2,// 122 PAY 119 

    0x09d8d62b,// 123 PAY 120 

    0x09cdbacb,// 124 PAY 121 

    0x1c948014,// 125 PAY 122 

    0x7eabce81,// 126 PAY 123 

    0xbdd34667,// 127 PAY 124 

    0x6d128835,// 128 PAY 125 

    0x34933532,// 129 PAY 126 

    0xec2f07df,// 130 PAY 127 

    0x397db6f8,// 131 PAY 128 

    0x26214287,// 132 PAY 129 

    0xe84ca959,// 133 PAY 130 

    0x85655109,// 134 PAY 131 

    0xaf926b40,// 135 PAY 132 

    0x0e45acc4,// 136 PAY 133 

    0xb7fe7dfe,// 137 PAY 134 

    0xd360bda9,// 138 PAY 135 

    0x225efab7,// 139 PAY 136 

    0xe432998e,// 140 PAY 137 

    0x3d343988,// 141 PAY 138 

    0x3c6f0c6c,// 142 PAY 139 

    0xa6560a3c,// 143 PAY 140 

    0xc909e3bc,// 144 PAY 141 

    0x8fe09e90,// 145 PAY 142 

    0xa8bf7869,// 146 PAY 143 

    0x9dcc0856,// 147 PAY 144 

    0x12cd75c6,// 148 PAY 145 

    0xb1230124,// 149 PAY 146 

    0xbf350e94,// 150 PAY 147 

    0xf33c06ff,// 151 PAY 148 

    0x0e992bbe,// 152 PAY 149 

    0x68026cfa,// 153 PAY 150 

    0xbeb00c00,// 154 PAY 151 

    0xd9d5b446,// 155 PAY 152 

    0x251495ae,// 156 PAY 153 

    0xe21ebe65,// 157 PAY 154 

    0x953f1c05,// 158 PAY 155 

    0x1e8b8018,// 159 PAY 156 

    0x97fea260,// 160 PAY 157 

    0x2899d8ab,// 161 PAY 158 

    0x51b54a3f,// 162 PAY 159 

    0x7156a0d3,// 163 PAY 160 

    0x3201ecac,// 164 PAY 161 

    0xa6fba62d,// 165 PAY 162 

    0x7cc4da2c,// 166 PAY 163 

    0x11b33937,// 167 PAY 164 

    0x3a9637ef,// 168 PAY 165 

    0x946e21db,// 169 PAY 166 

    0x3408c3c3,// 170 PAY 167 

    0xf10881d7,// 171 PAY 168 

    0x05315d62,// 172 PAY 169 

    0xbc13598c,// 173 PAY 170 

    0x908b54ca,// 174 PAY 171 

    0x15d65ef1,// 175 PAY 172 

    0xde5abfad,// 176 PAY 173 

    0x2d2fb85e,// 177 PAY 174 

    0x81c7b0e3,// 178 PAY 175 

    0x6874a7cc,// 179 PAY 176 

    0xa4935486,// 180 PAY 177 

    0x9c1232b3,// 181 PAY 178 

    0xcc1d6b7d,// 182 PAY 179 

    0x68b9e530,// 183 PAY 180 

    0x861a8810,// 184 PAY 181 

    0x09d13a71,// 185 PAY 182 

    0xa58b299e,// 186 PAY 183 

    0x86333f53,// 187 PAY 184 

    0xd2fd224e,// 188 PAY 185 

    0xe61054a4,// 189 PAY 186 

    0xd05ebe14,// 190 PAY 187 

    0x449e8475,// 191 PAY 188 

    0x6a47d23e,// 192 PAY 189 

    0xe5970ad2,// 193 PAY 190 

    0x01b36c4f,// 194 PAY 191 

    0x11bdefb1,// 195 PAY 192 

    0x191293be,// 196 PAY 193 

    0xda384203,// 197 PAY 194 

    0x0f1b0b11,// 198 PAY 195 

    0xbd765037,// 199 PAY 196 

    0x5203f550,// 200 PAY 197 

    0x4fa5eaee,// 201 PAY 198 

    0x5c66493b,// 202 PAY 199 

    0xb7ff3a15,// 203 PAY 200 

    0xe554d935,// 204 PAY 201 

    0x22aefd6e,// 205 PAY 202 

    0x13b2c711,// 206 PAY 203 

    0x4be5ab40,// 207 PAY 204 

    0xace4fca1,// 208 PAY 205 

    0xa2723b88,// 209 PAY 206 

    0xb3da3a8e,// 210 PAY 207 

    0xa39e525f,// 211 PAY 208 

    0x5d829026,// 212 PAY 209 

    0xb8969c1f,// 213 PAY 210 

    0xaf59f42d,// 214 PAY 211 

    0xdf1acc5a,// 215 PAY 212 

    0x92d8019c,// 216 PAY 213 

    0x38897b74,// 217 PAY 214 

    0x9a25214d,// 218 PAY 215 

    0xbb4fc6b0,// 219 PAY 216 

    0x4b5bfdd9,// 220 PAY 217 

    0xd7d590ea,// 221 PAY 218 

    0x2e48bb68,// 222 PAY 219 

    0x85c10865,// 223 PAY 220 

    0x294ced6c,// 224 PAY 221 

    0x8be1b730,// 225 PAY 222 

    0x07ac7eac,// 226 PAY 223 

    0xc53174c4,// 227 PAY 224 

    0x4e267279,// 228 PAY 225 

    0xaa71015e,// 229 PAY 226 

    0x2b9a6115,// 230 PAY 227 

    0x7873dbb5,// 231 PAY 228 

    0xf9b1e0eb,// 232 PAY 229 

    0xd9dfa974,// 233 PAY 230 

    0xfd99370a,// 234 PAY 231 

    0x1b2c33fc,// 235 PAY 232 

    0x9320844c,// 236 PAY 233 

    0x075bdfc3,// 237 PAY 234 

    0x558a20cd,// 238 PAY 235 

    0xa700cdf8,// 239 PAY 236 

    0xa0f3ecf1,// 240 PAY 237 

    0xa6b21472,// 241 PAY 238 

    0x2fbc6cdb,// 242 PAY 239 

    0xf37a0009,// 243 PAY 240 

    0xc2434ec7,// 244 PAY 241 

    0x14f3ccfa,// 245 PAY 242 

    0x0fb82dc7,// 246 PAY 243 

    0xb7d61933,// 247 PAY 244 

    0x03743492,// 248 PAY 245 

    0xfc84671a,// 249 PAY 246 

    0x342875e8,// 250 PAY 247 

    0x5aafe739,// 251 PAY 248 

    0x8c50e6d3,// 252 PAY 249 

    0x0244237b,// 253 PAY 250 

    0x01b4eb96,// 254 PAY 251 

    0xb9a6d019,// 255 PAY 252 

    0x782474c3,// 256 PAY 253 

    0xb32ffe0e,// 257 PAY 254 

    0x3883db3f,// 258 PAY 255 

    0x38026de4,// 259 PAY 256 

    0xb8703f75,// 260 PAY 257 

    0x0fe484b5,// 261 PAY 258 

    0x50fd6830,// 262 PAY 259 

    0x5b77780d,// 263 PAY 260 

    0x5ffaf131,// 264 PAY 261 

    0xf95267cd,// 265 PAY 262 

    0x837f058d,// 266 PAY 263 

    0xd494602f,// 267 PAY 264 

    0x6088b916,// 268 PAY 265 

    0x8c07029e,// 269 PAY 266 

    0x7ce4d340,// 270 PAY 267 

    0x1cfc873d,// 271 PAY 268 

    0x371a6a06,// 272 PAY 269 

    0xc6fc32fc,// 273 PAY 270 

    0xe9edb36b,// 274 PAY 271 

    0x393a8968,// 275 PAY 272 

    0x1e14986a,// 276 PAY 273 

    0xf2f8ad7f,// 277 PAY 274 

    0x4a18111a,// 278 PAY 275 

    0xbeb7d75e,// 279 PAY 276 

    0xc8f5715a,// 280 PAY 277 

    0x1d13d7dc,// 281 PAY 278 

    0xe1c05cf5,// 282 PAY 279 

    0x45212e48,// 283 PAY 280 

    0x327e2470,// 284 PAY 281 

    0x31fc7881,// 285 PAY 282 

    0x2804f9b1,// 286 PAY 283 

    0x28b2098e,// 287 PAY 284 

    0xca133e8e,// 288 PAY 285 

    0x39139806,// 289 PAY 286 

    0x42ae4a2b,// 290 PAY 287 

    0x03acfda3,// 291 PAY 288 

    0xc88df92c,// 292 PAY 289 

    0x0746ef10,// 293 PAY 290 

    0x7aff09cf,// 294 PAY 291 

    0x2a31b3d8,// 295 PAY 292 

    0x901bff3a,// 296 PAY 293 

    0xe098f81b,// 297 PAY 294 

    0x5d5fe9ad,// 298 PAY 295 

    0x1e6b76f8,// 299 PAY 296 

    0xf5241ef9,// 300 PAY 297 

    0x5318d2ef,// 301 PAY 298 

    0xcd6568cd,// 302 PAY 299 

    0xac554cff,// 303 PAY 300 

    0xce896e9d,// 304 PAY 301 

    0xa1a8af95,// 305 PAY 302 

    0x29482bbb,// 306 PAY 303 

    0x00810e56,// 307 PAY 304 

    0x9c27929e,// 308 PAY 305 

    0x59d3a16a,// 309 PAY 306 

    0x9f26bffe,// 310 PAY 307 

    0xbfac2045,// 311 PAY 308 

    0x02417dba,// 312 PAY 309 

    0xf8de61d8,// 313 PAY 310 

    0x00921258,// 314 PAY 311 

    0x85bca8c9,// 315 PAY 312 

    0x647a60b4,// 316 PAY 313 

    0x49364324,// 317 PAY 314 

    0x3d5eccde,// 318 PAY 315 

    0xbc3d766f,// 319 PAY 316 

    0x24e23063,// 320 PAY 317 

    0xbd89330b,// 321 PAY 318 

    0x946ec683,// 322 PAY 319 

    0xef235eae,// 323 PAY 320 

    0xde92cc75,// 324 PAY 321 

    0x10d96c65,// 325 PAY 322 

    0xff2e16d3,// 326 PAY 323 

    0x6fdfd5cd,// 327 PAY 324 

    0xc3b1c5da,// 328 PAY 325 

    0x2f3bd655,// 329 PAY 326 

    0x95aacf4a,// 330 PAY 327 

    0x2fed414d,// 331 PAY 328 

    0x3768a5a5,// 332 PAY 329 

    0x5eaacb4e,// 333 PAY 330 

    0xc9f68a19,// 334 PAY 331 

    0xa2368ec8,// 335 PAY 332 

    0x801f4309,// 336 PAY 333 

    0x27de40c4,// 337 PAY 334 

    0xab8bd860,// 338 PAY 335 

    0xdfe6a80a,// 339 PAY 336 

    0x9ba76d37,// 340 PAY 337 

    0x15475da4,// 341 PAY 338 

    0x4eb991ea,// 342 PAY 339 

    0x21eb4528,// 343 PAY 340 

    0x9ab71bd0,// 344 PAY 341 

    0xc141ce52,// 345 PAY 342 

    0x982cb33d,// 346 PAY 343 

    0xd1f213df,// 347 PAY 344 

    0xde137e42,// 348 PAY 345 

    0xa3a47231,// 349 PAY 346 

    0x41046534,// 350 PAY 347 

    0x91fbe01c,// 351 PAY 348 

    0x7491d24f,// 352 PAY 349 

    0xfcac5450,// 353 PAY 350 

    0x33e5e08d,// 354 PAY 351 

    0x516ba443,// 355 PAY 352 

    0x2bc37dc5,// 356 PAY 353 

    0x149b1d8d,// 357 PAY 354 

    0xa90d6431,// 358 PAY 355 

    0x2d59911a,// 359 PAY 356 

    0x09df1c80,// 360 PAY 357 

    0x20b64133,// 361 PAY 358 

    0xe0d04a9a,// 362 PAY 359 

    0x3b48c33d,// 363 PAY 360 

    0xca9bc45e,// 364 PAY 361 

    0xd9c4178d,// 365 PAY 362 

    0x9b3e3aaa,// 366 PAY 363 

    0x226e0299,// 367 PAY 364 

    0x1962416f,// 368 PAY 365 

    0xed8bc0c7,// 369 PAY 366 

    0x223205bc,// 370 PAY 367 

    0x2b1411aa,// 371 PAY 368 

    0x0e84890d,// 372 PAY 369 

    0xba8ee2c0,// 373 PAY 370 

    0x7cd2a4e6,// 374 PAY 371 

    0xf59b479f,// 375 PAY 372 

    0x0ca41258,// 376 PAY 373 

    0x685b4e46,// 377 PAY 374 

    0x566cd434,// 378 PAY 375 

    0x2c09f617,// 379 PAY 376 

    0xf03722bc,// 380 PAY 377 

    0xfc824f6b,// 381 PAY 378 

    0xf28c54e5,// 382 PAY 379 

    0xc017bc86,// 383 PAY 380 

    0xac8c6db3,// 384 PAY 381 

    0xe34b1f8b,// 385 PAY 382 

    0x8b0bb994,// 386 PAY 383 

    0x4e085c14,// 387 PAY 384 

    0x025487d7,// 388 PAY 385 

    0xdc6d8273,// 389 PAY 386 

    0x4e90cda1,// 390 PAY 387 

    0xb2d00a3b,// 391 PAY 388 

    0xef63d9ef,// 392 PAY 389 

    0xa2a6b7e7,// 393 PAY 390 

    0x116dfab3,// 394 PAY 391 

    0x442ba5ce,// 395 PAY 392 

    0x6fc95f3f,// 396 PAY 393 

    0xdfcda654,// 397 PAY 394 

    0xec39e0f4,// 398 PAY 395 

    0x1f3a044e,// 399 PAY 396 

    0x3c295cc2,// 400 PAY 397 

    0x5e1a6f09,// 401 PAY 398 

    0x62ab9cab,// 402 PAY 399 

    0x752ebb70,// 403 PAY 400 

    0x973dcfd7,// 404 PAY 401 

    0x4dd6bb44,// 405 PAY 402 

    0x932023f1,// 406 PAY 403 

    0xd905a138,// 407 PAY 404 

    0x03f51ea9,// 408 PAY 405 

    0x5a2ba36f,// 409 PAY 406 

    0xed935d54,// 410 PAY 407 

    0x2d8883da,// 411 PAY 408 

    0x026a27fd,// 412 PAY 409 

    0x083ca56a,// 413 PAY 410 

    0x32aa5c32,// 414 PAY 411 

    0xbb9c173f,// 415 PAY 412 

    0x3a08d19e,// 416 PAY 413 

    0x2d4ad8b0,// 417 PAY 414 

    0xd123a5ee,// 418 PAY 415 

    0xdcee9e74,// 419 PAY 416 

    0x09233663,// 420 PAY 417 

    0x1632c9dd,// 421 PAY 418 

    0xdbf3677b,// 422 PAY 419 

    0xc030e13f,// 423 PAY 420 

    0x299402a1,// 424 PAY 421 

    0x88eee516,// 425 PAY 422 

    0x12afb763,// 426 PAY 423 

    0x2e5e2a4d,// 427 PAY 424 

    0x315e33da,// 428 PAY 425 

    0x306697c8,// 429 PAY 426 

    0x1f0d7e15,// 430 PAY 427 

    0x5595e788,// 431 PAY 428 

    0x0ab7d6d9,// 432 PAY 429 

    0x312d1709,// 433 PAY 430 

    0x32993af2,// 434 PAY 431 

    0x917a946b,// 435 PAY 432 

    0xfea16f84,// 436 PAY 433 

    0xa6a76973,// 437 PAY 434 

    0xfb7aa3df,// 438 PAY 435 

    0x6ee810b2,// 439 PAY 436 

    0x30bb06b7,// 440 PAY 437 

    0x35a63287,// 441 PAY 438 

    0xb62d07cf,// 442 PAY 439 

    0x70a85688,// 443 PAY 440 

    0x2d878ffb,// 444 PAY 441 

    0xa09a9126,// 445 PAY 442 

    0xea3f10ef,// 446 PAY 443 

    0x6e75131e,// 447 PAY 444 

    0x15f97e84,// 448 PAY 445 

    0x5747e34b,// 449 PAY 446 

    0x0f136fc2,// 450 PAY 447 

    0x0ee2f535,// 451 PAY 448 

    0xa79c0f8d,// 452 PAY 449 

    0x8822274a,// 453 PAY 450 

    0x2f0890e8,// 454 PAY 451 

    0x60a79dd2,// 455 PAY 452 

    0x3bcb7e2e,// 456 PAY 453 

    0x05b2a12b,// 457 PAY 454 

    0x80abf8de,// 458 PAY 455 

    0xf410a3b0,// 459 PAY 456 

    0xc4aab198,// 460 PAY 457 

    0x62181e4a,// 461 PAY 458 

    0x87d21533,// 462 PAY 459 

    0x151bd1ea,// 463 PAY 460 

    0xb7a39999,// 464 PAY 461 

    0x28858b17,// 465 PAY 462 

    0x3a1f90e0,// 466 PAY 463 

    0x31344b21,// 467 PAY 464 

    0xac2f4fd9,// 468 PAY 465 

    0x249200ae,// 469 PAY 466 

    0xe959bc5b,// 470 PAY 467 

    0x57de6502,// 471 PAY 468 

    0x1a58612b,// 472 PAY 469 

    0x9ce1d952,// 473 PAY 470 

    0x6e2f4934,// 474 PAY 471 

    0xeaf4f784,// 475 PAY 472 

    0xd333315e,// 476 PAY 473 

    0x2285c2ec,// 477 PAY 474 

    0x6e285551,// 478 PAY 475 

    0x070a71e3,// 479 PAY 476 

    0xa827f2e4,// 480 PAY 477 

    0x94f5c596,// 481 PAY 478 

    0x0fcc2d95,// 482 PAY 479 

    0xf0a51fe2,// 483 PAY 480 

    0xfdd87304,// 484 PAY 481 

    0xbdb5c46c,// 485 PAY 482 

    0x87f6c2be,// 486 PAY 483 

    0x0acc324e,// 487 PAY 484 

    0xd126869d,// 488 PAY 485 

    0x9b29a924,// 489 PAY 486 

    0xdc560000,// 490 PAY 487 

/// HASH is  12 bytes 

    0x05b2a12b,// 491 HSH   1 

    0x80abf8de,// 492 HSH   2 

    0xf410a3b0,// 493 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 73 

/// STA pkt_idx        : 206 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x16 

    0x03391649 // 494 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt84_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 497 words. 

/// BDA size     is 1983 (0x7bf) 

/// BDA id       is 0x8ef 

    0x07bf08ef,// 3 BDA   1 

/// PAY Generic Data size   : 1983 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x9c9271c2,// 4 PAY   1 

    0x161adddb,// 5 PAY   2 

    0x6875eb60,// 6 PAY   3 

    0xadb53449,// 7 PAY   4 

    0xc59c0467,// 8 PAY   5 

    0xfa44a79b,// 9 PAY   6 

    0x15507b8e,// 10 PAY   7 

    0xe55fda25,// 11 PAY   8 

    0x35f86520,// 12 PAY   9 

    0xaac7cb59,// 13 PAY  10 

    0x6df16d4c,// 14 PAY  11 

    0x22e5d80c,// 15 PAY  12 

    0xe3fa30b0,// 16 PAY  13 

    0x9b60cacf,// 17 PAY  14 

    0xb2b9ed5b,// 18 PAY  15 

    0x0c5945af,// 19 PAY  16 

    0xec8c227f,// 20 PAY  17 

    0xa6581243,// 21 PAY  18 

    0xd9c9391a,// 22 PAY  19 

    0x2a9f16d9,// 23 PAY  20 

    0xba477727,// 24 PAY  21 

    0xe89b456e,// 25 PAY  22 

    0x94c58fcb,// 26 PAY  23 

    0x500881a7,// 27 PAY  24 

    0x151ba1a1,// 28 PAY  25 

    0x80653fbd,// 29 PAY  26 

    0x32f4a14a,// 30 PAY  27 

    0xff75164d,// 31 PAY  28 

    0x7e8f1bb1,// 32 PAY  29 

    0xb336df1a,// 33 PAY  30 

    0x14e00040,// 34 PAY  31 

    0x36ba763e,// 35 PAY  32 

    0x9e099e38,// 36 PAY  33 

    0x25dcbe96,// 37 PAY  34 

    0xf7e76db3,// 38 PAY  35 

    0x2d12b372,// 39 PAY  36 

    0x4b129c6f,// 40 PAY  37 

    0xc1fe39f2,// 41 PAY  38 

    0xf85f516a,// 42 PAY  39 

    0xaaf13bd8,// 43 PAY  40 

    0x4c7f3562,// 44 PAY  41 

    0x7b5866da,// 45 PAY  42 

    0x6db5fd6f,// 46 PAY  43 

    0xdaa9566d,// 47 PAY  44 

    0x09fcd691,// 48 PAY  45 

    0xde973c62,// 49 PAY  46 

    0xc31f61bf,// 50 PAY  47 

    0xa746c05d,// 51 PAY  48 

    0x51777807,// 52 PAY  49 

    0x77882e0e,// 53 PAY  50 

    0x05a6e746,// 54 PAY  51 

    0x5b26a2ac,// 55 PAY  52 

    0x4240191e,// 56 PAY  53 

    0xec525a0f,// 57 PAY  54 

    0x491e7080,// 58 PAY  55 

    0x6509e4b8,// 59 PAY  56 

    0x5ba1a35a,// 60 PAY  57 

    0xbf9bf2f5,// 61 PAY  58 

    0x5ec72df3,// 62 PAY  59 

    0xf1e51d8a,// 63 PAY  60 

    0x9f46ec4f,// 64 PAY  61 

    0x7d57f885,// 65 PAY  62 

    0x2ac50a9c,// 66 PAY  63 

    0xb98fcb1b,// 67 PAY  64 

    0x6bae1c3c,// 68 PAY  65 

    0x67592b0b,// 69 PAY  66 

    0xede6cc57,// 70 PAY  67 

    0xfea48a3c,// 71 PAY  68 

    0x100861bc,// 72 PAY  69 

    0x2290c50f,// 73 PAY  70 

    0xc32216db,// 74 PAY  71 

    0xe8899df4,// 75 PAY  72 

    0xfeeb1314,// 76 PAY  73 

    0x6578388b,// 77 PAY  74 

    0xa55a31e5,// 78 PAY  75 

    0xdce82e58,// 79 PAY  76 

    0x3c85c9b1,// 80 PAY  77 

    0x4c481eab,// 81 PAY  78 

    0xfe908bba,// 82 PAY  79 

    0xf3344070,// 83 PAY  80 

    0x89ceb240,// 84 PAY  81 

    0x381ca462,// 85 PAY  82 

    0x509af437,// 86 PAY  83 

    0xc5c62690,// 87 PAY  84 

    0x8ef23a2e,// 88 PAY  85 

    0x40c50ca8,// 89 PAY  86 

    0x780015d9,// 90 PAY  87 

    0x3093691f,// 91 PAY  88 

    0x2ab713ed,// 92 PAY  89 

    0x2fd4305f,// 93 PAY  90 

    0x50a089cc,// 94 PAY  91 

    0xc7b64e0d,// 95 PAY  92 

    0x785b880d,// 96 PAY  93 

    0xf01069e1,// 97 PAY  94 

    0x6a809bbc,// 98 PAY  95 

    0x858f1f4c,// 99 PAY  96 

    0xd2fc498b,// 100 PAY  97 

    0xea3464d2,// 101 PAY  98 

    0x54bab70f,// 102 PAY  99 

    0xecc96dcb,// 103 PAY 100 

    0x28cc2642,// 104 PAY 101 

    0xa8b206d6,// 105 PAY 102 

    0xefc66069,// 106 PAY 103 

    0xeff2d357,// 107 PAY 104 

    0x5b2f6478,// 108 PAY 105 

    0xbd8f6409,// 109 PAY 106 

    0x26ff4bd0,// 110 PAY 107 

    0xde100ff5,// 111 PAY 108 

    0x300a8487,// 112 PAY 109 

    0x60174412,// 113 PAY 110 

    0x6d12e6d2,// 114 PAY 111 

    0x851cc427,// 115 PAY 112 

    0xbc981716,// 116 PAY 113 

    0xea801746,// 117 PAY 114 

    0xbe75271c,// 118 PAY 115 

    0xa9c4a76a,// 119 PAY 116 

    0x7a0c0ddb,// 120 PAY 117 

    0xb3c7181e,// 121 PAY 118 

    0xb68c0b3f,// 122 PAY 119 

    0x09a9fecb,// 123 PAY 120 

    0xaf63ed76,// 124 PAY 121 

    0x0ee7e80f,// 125 PAY 122 

    0x305afa90,// 126 PAY 123 

    0x13fd8d47,// 127 PAY 124 

    0x32d5cccd,// 128 PAY 125 

    0xfe0ef4b4,// 129 PAY 126 

    0xcf9445b4,// 130 PAY 127 

    0x1eb56ec2,// 131 PAY 128 

    0x2e4aafb1,// 132 PAY 129 

    0x5c946ca8,// 133 PAY 130 

    0x530b88a8,// 134 PAY 131 

    0xe23b3fe0,// 135 PAY 132 

    0x7c651087,// 136 PAY 133 

    0x52761c05,// 137 PAY 134 

    0xbb1029ae,// 138 PAY 135 

    0x52a23cf5,// 139 PAY 136 

    0x2bdefa23,// 140 PAY 137 

    0xef33fd79,// 141 PAY 138 

    0xd5979137,// 142 PAY 139 

    0xb6fdf95b,// 143 PAY 140 

    0x8fd88c1f,// 144 PAY 141 

    0xbf0d374e,// 145 PAY 142 

    0xca03a947,// 146 PAY 143 

    0xa92f98f3,// 147 PAY 144 

    0x3f088d78,// 148 PAY 145 

    0x03ad711c,// 149 PAY 146 

    0xa66cc23d,// 150 PAY 147 

    0xe4ad17d9,// 151 PAY 148 

    0x24dc9ce3,// 152 PAY 149 

    0xd29f33b4,// 153 PAY 150 

    0x61950b62,// 154 PAY 151 

    0x9d6742b8,// 155 PAY 152 

    0x1b9fa1cc,// 156 PAY 153 

    0x8730c396,// 157 PAY 154 

    0x6d05ad6e,// 158 PAY 155 

    0x85100d80,// 159 PAY 156 

    0x63c54be0,// 160 PAY 157 

    0xc0340fdb,// 161 PAY 158 

    0x991d9a64,// 162 PAY 159 

    0x780acfd6,// 163 PAY 160 

    0xc24f033c,// 164 PAY 161 

    0x5bcf08bc,// 165 PAY 162 

    0xc024443a,// 166 PAY 163 

    0x2f05c192,// 167 PAY 164 

    0x08b99921,// 168 PAY 165 

    0x3359bd07,// 169 PAY 166 

    0x83124194,// 170 PAY 167 

    0x4a21c7fa,// 171 PAY 168 

    0xe7b84476,// 172 PAY 169 

    0x58ac643e,// 173 PAY 170 

    0x1bfa324e,// 174 PAY 171 

    0x4124c99f,// 175 PAY 172 

    0x79747ae8,// 176 PAY 173 

    0xcc500337,// 177 PAY 174 

    0xe98d61ba,// 178 PAY 175 

    0x634cda54,// 179 PAY 176 

    0x4797bc1c,// 180 PAY 177 

    0x97ff7c5f,// 181 PAY 178 

    0xd6a38b1e,// 182 PAY 179 

    0x548b5b14,// 183 PAY 180 

    0x20b75f17,// 184 PAY 181 

    0x30ba6400,// 185 PAY 182 

    0xddd43c1e,// 186 PAY 183 

    0x387e7571,// 187 PAY 184 

    0xabb171c2,// 188 PAY 185 

    0x950bb682,// 189 PAY 186 

    0xe986c476,// 190 PAY 187 

    0xb87f8eea,// 191 PAY 188 

    0x00184067,// 192 PAY 189 

    0x40c74f95,// 193 PAY 190 

    0x48bdcf13,// 194 PAY 191 

    0x660f8a5f,// 195 PAY 192 

    0xdb28890a,// 196 PAY 193 

    0xb3a4ec21,// 197 PAY 194 

    0x6813792d,// 198 PAY 195 

    0xb5b7f189,// 199 PAY 196 

    0xdff12800,// 200 PAY 197 

    0x622642c2,// 201 PAY 198 

    0x872f2844,// 202 PAY 199 

    0x81affe59,// 203 PAY 200 

    0x0fba94a1,// 204 PAY 201 

    0x33990d43,// 205 PAY 202 

    0x97e8913e,// 206 PAY 203 

    0x51734ad3,// 207 PAY 204 

    0xfb8ab4eb,// 208 PAY 205 

    0x83bdc52a,// 209 PAY 206 

    0xd38b8483,// 210 PAY 207 

    0x4d591778,// 211 PAY 208 

    0xf8c32e54,// 212 PAY 209 

    0x10fca49d,// 213 PAY 210 

    0xbcf4392b,// 214 PAY 211 

    0x11242553,// 215 PAY 212 

    0xd300fc03,// 216 PAY 213 

    0x05a7e4fd,// 217 PAY 214 

    0x922de5b0,// 218 PAY 215 

    0x379cdc02,// 219 PAY 216 

    0x21edaf16,// 220 PAY 217 

    0x3ab895fe,// 221 PAY 218 

    0x09559159,// 222 PAY 219 

    0x49b57881,// 223 PAY 220 

    0x4b586049,// 224 PAY 221 

    0x73c0052f,// 225 PAY 222 

    0xd3bdc4d4,// 226 PAY 223 

    0xf180a643,// 227 PAY 224 

    0x849264f0,// 228 PAY 225 

    0x07795755,// 229 PAY 226 

    0x1ac0b079,// 230 PAY 227 

    0xe2b8b4f0,// 231 PAY 228 

    0x44806f06,// 232 PAY 229 

    0xb6e4ef5c,// 233 PAY 230 

    0xa4b924ed,// 234 PAY 231 

    0xc366a765,// 235 PAY 232 

    0x48c3f211,// 236 PAY 233 

    0x8ec4b88c,// 237 PAY 234 

    0x6795990d,// 238 PAY 235 

    0x6ffc4a70,// 239 PAY 236 

    0xe0c69247,// 240 PAY 237 

    0x1cd5f59d,// 241 PAY 238 

    0x1f4571b2,// 242 PAY 239 

    0x1bb82823,// 243 PAY 240 

    0xed7bac26,// 244 PAY 241 

    0xdddf8b60,// 245 PAY 242 

    0xd2afa44f,// 246 PAY 243 

    0x112fb103,// 247 PAY 244 

    0xef887df9,// 248 PAY 245 

    0xe0ca6ffc,// 249 PAY 246 

    0x6855b46a,// 250 PAY 247 

    0x6a9070b0,// 251 PAY 248 

    0xa7fc897b,// 252 PAY 249 

    0x312a179b,// 253 PAY 250 

    0x476e8d63,// 254 PAY 251 

    0x5ef5fb7e,// 255 PAY 252 

    0x005764ba,// 256 PAY 253 

    0x4f58cdb3,// 257 PAY 254 

    0x4a3965b4,// 258 PAY 255 

    0xf6900a8d,// 259 PAY 256 

    0xd6423a49,// 260 PAY 257 

    0x8e7469be,// 261 PAY 258 

    0xc7443611,// 262 PAY 259 

    0x2f4d2943,// 263 PAY 260 

    0xc8f26b56,// 264 PAY 261 

    0x58bdcd04,// 265 PAY 262 

    0xd52db8dd,// 266 PAY 263 

    0xd4f0bb7e,// 267 PAY 264 

    0xb4c14021,// 268 PAY 265 

    0xf3e11b10,// 269 PAY 266 

    0xa3de78a4,// 270 PAY 267 

    0xd2fd495d,// 271 PAY 268 

    0xe48fb4cb,// 272 PAY 269 

    0x3696e254,// 273 PAY 270 

    0xe37cf27c,// 274 PAY 271 

    0xaec10fe6,// 275 PAY 272 

    0x81a2d3b2,// 276 PAY 273 

    0x8af770c8,// 277 PAY 274 

    0xe73cb39d,// 278 PAY 275 

    0x490dfbc9,// 279 PAY 276 

    0x835f77b1,// 280 PAY 277 

    0x35105e96,// 281 PAY 278 

    0x3f40b00a,// 282 PAY 279 

    0xdfd1e824,// 283 PAY 280 

    0x110ba00e,// 284 PAY 281 

    0xbbb908c2,// 285 PAY 282 

    0xa3adb4fd,// 286 PAY 283 

    0x2a4bc648,// 287 PAY 284 

    0x7a822d15,// 288 PAY 285 

    0xd4458b6d,// 289 PAY 286 

    0xa151dbcc,// 290 PAY 287 

    0xd536a8be,// 291 PAY 288 

    0xa0ed7077,// 292 PAY 289 

    0xd984cbfb,// 293 PAY 290 

    0xc4851837,// 294 PAY 291 

    0xa65100f7,// 295 PAY 292 

    0xf76427b2,// 296 PAY 293 

    0xabb432e4,// 297 PAY 294 

    0xb01db8fc,// 298 PAY 295 

    0xc5bc3a0c,// 299 PAY 296 

    0xa183e3f4,// 300 PAY 297 

    0x7344fe49,// 301 PAY 298 

    0x897c0588,// 302 PAY 299 

    0xada5b8f3,// 303 PAY 300 

    0x14181eae,// 304 PAY 301 

    0xf96f9905,// 305 PAY 302 

    0x56e75513,// 306 PAY 303 

    0xaef59fcf,// 307 PAY 304 

    0x52f6d1f5,// 308 PAY 305 

    0x864032e2,// 309 PAY 306 

    0x182e8803,// 310 PAY 307 

    0xea68f07a,// 311 PAY 308 

    0x1945b9c9,// 312 PAY 309 

    0xe1d9b0c8,// 313 PAY 310 

    0xd4f6262a,// 314 PAY 311 

    0x94a0f6ec,// 315 PAY 312 

    0xb4abb6a3,// 316 PAY 313 

    0x289bb28f,// 317 PAY 314 

    0x0e14c402,// 318 PAY 315 

    0xa3ef2e26,// 319 PAY 316 

    0xc3d52946,// 320 PAY 317 

    0xe42245b0,// 321 PAY 318 

    0x4ec3fbd5,// 322 PAY 319 

    0x3d44e06b,// 323 PAY 320 

    0x12ea2e9e,// 324 PAY 321 

    0x2164aba6,// 325 PAY 322 

    0x9cebfc8a,// 326 PAY 323 

    0xbc5de34b,// 327 PAY 324 

    0x5d54c7c9,// 328 PAY 325 

    0x1eeb1d8b,// 329 PAY 326 

    0x54adb716,// 330 PAY 327 

    0x91595072,// 331 PAY 328 

    0xbb416e7c,// 332 PAY 329 

    0x4a562c2a,// 333 PAY 330 

    0xebb2d9ef,// 334 PAY 331 

    0x38d4d8eb,// 335 PAY 332 

    0x48bcd044,// 336 PAY 333 

    0x0665d844,// 337 PAY 334 

    0x79189ee0,// 338 PAY 335 

    0x1def1323,// 339 PAY 336 

    0x44ffdc79,// 340 PAY 337 

    0xade654d7,// 341 PAY 338 

    0x9efc6494,// 342 PAY 339 

    0xf8c77b68,// 343 PAY 340 

    0x66bb7e0c,// 344 PAY 341 

    0x4dd25f69,// 345 PAY 342 

    0x0812de14,// 346 PAY 343 

    0xd3d3e3f2,// 347 PAY 344 

    0x821bcf82,// 348 PAY 345 

    0xe15e6e90,// 349 PAY 346 

    0x4e6fa30d,// 350 PAY 347 

    0x2991ee29,// 351 PAY 348 

    0x0e1d47c2,// 352 PAY 349 

    0x8246fccc,// 353 PAY 350 

    0x7aef5ae3,// 354 PAY 351 

    0x17b4e94d,// 355 PAY 352 

    0xd933da6e,// 356 PAY 353 

    0x4fd95c06,// 357 PAY 354 

    0xa1623311,// 358 PAY 355 

    0x75601988,// 359 PAY 356 

    0xb225ade6,// 360 PAY 357 

    0x4ee01029,// 361 PAY 358 

    0x10003ca7,// 362 PAY 359 

    0xaae10b5a,// 363 PAY 360 

    0xe26bc0a3,// 364 PAY 361 

    0xd11f6e9c,// 365 PAY 362 

    0xf91533ed,// 366 PAY 363 

    0x060d7413,// 367 PAY 364 

    0x1d3dae7b,// 368 PAY 365 

    0x294a23d8,// 369 PAY 366 

    0x76ac0404,// 370 PAY 367 

    0xc9c5f018,// 371 PAY 368 

    0xb9b78485,// 372 PAY 369 

    0xa9756d9a,// 373 PAY 370 

    0xec6cd481,// 374 PAY 371 

    0x6cbbf150,// 375 PAY 372 

    0xc848a996,// 376 PAY 373 

    0xf30ca2b7,// 377 PAY 374 

    0x1d232061,// 378 PAY 375 

    0x57654e31,// 379 PAY 376 

    0x34479af6,// 380 PAY 377 

    0x3c3b7cbb,// 381 PAY 378 

    0xcf5730c3,// 382 PAY 379 

    0xd750d82a,// 383 PAY 380 

    0x4df92c08,// 384 PAY 381 

    0xfaf839f7,// 385 PAY 382 

    0x40e4c1e3,// 386 PAY 383 

    0xd278fc96,// 387 PAY 384 

    0x2da20403,// 388 PAY 385 

    0x3476ba39,// 389 PAY 386 

    0xf6c41f30,// 390 PAY 387 

    0x74978845,// 391 PAY 388 

    0x74b6635f,// 392 PAY 389 

    0x18c7de26,// 393 PAY 390 

    0x4c774732,// 394 PAY 391 

    0xb8623b91,// 395 PAY 392 

    0xf0ddc803,// 396 PAY 393 

    0x12c20915,// 397 PAY 394 

    0xcc143594,// 398 PAY 395 

    0x3c20995f,// 399 PAY 396 

    0x6c935059,// 400 PAY 397 

    0x4e514b5d,// 401 PAY 398 

    0x8f2e56fb,// 402 PAY 399 

    0xc12d944b,// 403 PAY 400 

    0x7ebc51c2,// 404 PAY 401 

    0xd709d9d9,// 405 PAY 402 

    0x5426cba0,// 406 PAY 403 

    0x0c6263c0,// 407 PAY 404 

    0xf6b84a03,// 408 PAY 405 

    0xe6535785,// 409 PAY 406 

    0xa02fa917,// 410 PAY 407 

    0x7b4e78c5,// 411 PAY 408 

    0x68c0e96d,// 412 PAY 409 

    0xf7481e6f,// 413 PAY 410 

    0xd184f5c0,// 414 PAY 411 

    0xb2e66d96,// 415 PAY 412 

    0x7f18d7ed,// 416 PAY 413 

    0xadc59a2d,// 417 PAY 414 

    0xdec2e515,// 418 PAY 415 

    0xaffae755,// 419 PAY 416 

    0xb4eb9365,// 420 PAY 417 

    0x62661073,// 421 PAY 418 

    0x36cc0cc9,// 422 PAY 419 

    0xad16ff0b,// 423 PAY 420 

    0xd07e979a,// 424 PAY 421 

    0x7b902485,// 425 PAY 422 

    0x37160362,// 426 PAY 423 

    0xdb597b3a,// 427 PAY 424 

    0x7e308661,// 428 PAY 425 

    0xbf6e5ba0,// 429 PAY 426 

    0x2c504478,// 430 PAY 427 

    0x72742087,// 431 PAY 428 

    0xcbf0d08e,// 432 PAY 429 

    0x9cd49492,// 433 PAY 430 

    0x51fc02df,// 434 PAY 431 

    0x3435b816,// 435 PAY 432 

    0x3cac0bda,// 436 PAY 433 

    0x4a973dcb,// 437 PAY 434 

    0xedd20098,// 438 PAY 435 

    0x8fe3770b,// 439 PAY 436 

    0x4849af77,// 440 PAY 437 

    0x4cd32c40,// 441 PAY 438 

    0xd3ccff9a,// 442 PAY 439 

    0x8e5775f2,// 443 PAY 440 

    0x7f595015,// 444 PAY 441 

    0xa05eec39,// 445 PAY 442 

    0x9f0615a3,// 446 PAY 443 

    0x3a465c9e,// 447 PAY 444 

    0xcf67c776,// 448 PAY 445 

    0xeb66cdee,// 449 PAY 446 

    0x9cbd6230,// 450 PAY 447 

    0xf2e918b8,// 451 PAY 448 

    0xd48a57db,// 452 PAY 449 

    0xe6ba68cb,// 453 PAY 450 

    0xdffe898c,// 454 PAY 451 

    0x3b920360,// 455 PAY 452 

    0x091a1bce,// 456 PAY 453 

    0xeb4771ca,// 457 PAY 454 

    0x1a0c95a8,// 458 PAY 455 

    0xa34a9b9a,// 459 PAY 456 

    0x04c6f25e,// 460 PAY 457 

    0x742aba70,// 461 PAY 458 

    0x17ee22c1,// 462 PAY 459 

    0x63052ca4,// 463 PAY 460 

    0xd41dddee,// 464 PAY 461 

    0x6527ba2b,// 465 PAY 462 

    0x29ab1ebc,// 466 PAY 463 

    0x7565db09,// 467 PAY 464 

    0x41930f80,// 468 PAY 465 

    0x12c26dda,// 469 PAY 466 

    0xe70e3e9d,// 470 PAY 467 

    0xc1f3e63a,// 471 PAY 468 

    0xcc4c365c,// 472 PAY 469 

    0x3be37a3a,// 473 PAY 470 

    0xbdaa8464,// 474 PAY 471 

    0xbfe709c0,// 475 PAY 472 

    0xe809e530,// 476 PAY 473 

    0x7516a8c8,// 477 PAY 474 

    0x45960324,// 478 PAY 475 

    0x6ea4cbc7,// 479 PAY 476 

    0x592a36dc,// 480 PAY 477 

    0x0f5ce4c8,// 481 PAY 478 

    0xb61b31d9,// 482 PAY 479 

    0x1536b016,// 483 PAY 480 

    0xbbf490a5,// 484 PAY 481 

    0xf95588d4,// 485 PAY 482 

    0xe7119db1,// 486 PAY 483 

    0x7ab5ad44,// 487 PAY 484 

    0x9f9e292e,// 488 PAY 485 

    0x7b217486,// 489 PAY 486 

    0xdcc0e0fa,// 490 PAY 487 

    0x86d89b72,// 491 PAY 488 

    0x939b9940,// 492 PAY 489 

    0x2dee9380,// 493 PAY 490 

    0x4a5dda32,// 494 PAY 491 

    0x17e697d7,// 495 PAY 492 

    0xc005e3d6,// 496 PAY 493 

    0x095acf88,// 497 PAY 494 

    0x4b0fd009,// 498 PAY 495 

    0x5a68b500,// 499 PAY 496 

/// STA is 1 words. 

/// STA num_pkts       : 53 

/// STA pkt_idx        : 49 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xb4 

    0x00c4b435 // 500 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt85_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 91 words. 

/// BDA size     is 357 (0x165) 

/// BDA id       is 0xca9 

    0x01650ca9,// 3 BDA   1 

/// PAY Generic Data size   : 357 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x602a59d2,// 4 PAY   1 

    0x90b7490d,// 5 PAY   2 

    0xcc6c1eb9,// 6 PAY   3 

    0x147617f0,// 7 PAY   4 

    0x9b382a5b,// 8 PAY   5 

    0x3e7f8e34,// 9 PAY   6 

    0x2218cab2,// 10 PAY   7 

    0xb610113c,// 11 PAY   8 

    0x97cc08ee,// 12 PAY   9 

    0x2f74b3b7,// 13 PAY  10 

    0xf18b6424,// 14 PAY  11 

    0x2487bf23,// 15 PAY  12 

    0xfc139d7b,// 16 PAY  13 

    0x9bb66847,// 17 PAY  14 

    0x82d2ea41,// 18 PAY  15 

    0x6f873edc,// 19 PAY  16 

    0x4314a130,// 20 PAY  17 

    0x40a7372b,// 21 PAY  18 

    0x54061357,// 22 PAY  19 

    0x57ff52cb,// 23 PAY  20 

    0xb5dc8300,// 24 PAY  21 

    0x6b8fe18c,// 25 PAY  22 

    0x914c13fc,// 26 PAY  23 

    0x0b6c1d40,// 27 PAY  24 

    0x1038f232,// 28 PAY  25 

    0xd9c42d25,// 29 PAY  26 

    0x1337f1c7,// 30 PAY  27 

    0xa9f1b9c1,// 31 PAY  28 

    0xfd46f82e,// 32 PAY  29 

    0xa63abe41,// 33 PAY  30 

    0xa74c007c,// 34 PAY  31 

    0x799e876c,// 35 PAY  32 

    0xa738ca58,// 36 PAY  33 

    0x62f56566,// 37 PAY  34 

    0xac5ecb25,// 38 PAY  35 

    0x9acfeeaf,// 39 PAY  36 

    0xdf223b39,// 40 PAY  37 

    0x706d23e5,// 41 PAY  38 

    0xc72bbd5e,// 42 PAY  39 

    0xdce5da64,// 43 PAY  40 

    0x88d8af8e,// 44 PAY  41 

    0xd06dfa99,// 45 PAY  42 

    0x2f356935,// 46 PAY  43 

    0x044bdcf6,// 47 PAY  44 

    0xf4a9f925,// 48 PAY  45 

    0xf9d9b664,// 49 PAY  46 

    0x2cbe1654,// 50 PAY  47 

    0xf63fdedb,// 51 PAY  48 

    0x3b5731dc,// 52 PAY  49 

    0x8ee69e7e,// 53 PAY  50 

    0x350523ad,// 54 PAY  51 

    0x48a1464c,// 55 PAY  52 

    0xb1024c82,// 56 PAY  53 

    0xbd544248,// 57 PAY  54 

    0x4f9cfead,// 58 PAY  55 

    0x228511ff,// 59 PAY  56 

    0xbd7588c8,// 60 PAY  57 

    0x5ebbce49,// 61 PAY  58 

    0x7c5bc764,// 62 PAY  59 

    0x97726dc0,// 63 PAY  60 

    0x1a2c86dc,// 64 PAY  61 

    0xebb5941c,// 65 PAY  62 

    0x5b307dc7,// 66 PAY  63 

    0x71bc8324,// 67 PAY  64 

    0x547b318d,// 68 PAY  65 

    0x0a90662c,// 69 PAY  66 

    0x14f7769c,// 70 PAY  67 

    0x77f811ac,// 71 PAY  68 

    0xf37d5c78,// 72 PAY  69 

    0x65cfe1bf,// 73 PAY  70 

    0x0fa50ec9,// 74 PAY  71 

    0xba25e906,// 75 PAY  72 

    0xf9da7b8d,// 76 PAY  73 

    0x2a24fb74,// 77 PAY  74 

    0x7281ea5e,// 78 PAY  75 

    0xba9433a4,// 79 PAY  76 

    0x92b35e08,// 80 PAY  77 

    0x17f28ada,// 81 PAY  78 

    0x569f1a28,// 82 PAY  79 

    0x3a3bd679,// 83 PAY  80 

    0x49c12a5f,// 84 PAY  81 

    0x292716fd,// 85 PAY  82 

    0xf7fcc7e5,// 86 PAY  83 

    0xa0c9c8bc,// 87 PAY  84 

    0xd42dfab6,// 88 PAY  85 

    0x7935f48c,// 89 PAY  86 

    0x16bad988,// 90 PAY  87 

    0x788a8869,// 91 PAY  88 

    0x773896c0,// 92 PAY  89 

    0xe8000000,// 93 PAY  90 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 53 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5e 

    0x00d55e7e // 94 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt86_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 387 words. 

/// BDA size     is 1544 (0x608) 

/// BDA id       is 0xf6c2 

    0x0608f6c2,// 3 BDA   1 

/// PAY Generic Data size   : 1544 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0x18834df7,// 4 PAY   1 

    0xad5b163f,// 5 PAY   2 

    0xa20a9405,// 6 PAY   3 

    0xc4e3530b,// 7 PAY   4 

    0xa4815b32,// 8 PAY   5 

    0x06220567,// 9 PAY   6 

    0xc83b3e39,// 10 PAY   7 

    0xe4840966,// 11 PAY   8 

    0x7e18622e,// 12 PAY   9 

    0xddde866c,// 13 PAY  10 

    0x8e448dc9,// 14 PAY  11 

    0xfb8d51a3,// 15 PAY  12 

    0xf07b7470,// 16 PAY  13 

    0xee2e661b,// 17 PAY  14 

    0xc56bf53f,// 18 PAY  15 

    0x551aaf17,// 19 PAY  16 

    0x92327589,// 20 PAY  17 

    0xd4a59a11,// 21 PAY  18 

    0x1432fe7d,// 22 PAY  19 

    0x2ca9256a,// 23 PAY  20 

    0x12443a61,// 24 PAY  21 

    0x2e543867,// 25 PAY  22 

    0x38b0535a,// 26 PAY  23 

    0xefaf3f9e,// 27 PAY  24 

    0xba340ac8,// 28 PAY  25 

    0x2d488251,// 29 PAY  26 

    0x370da4a3,// 30 PAY  27 

    0xbd2f6847,// 31 PAY  28 

    0x688d5a54,// 32 PAY  29 

    0x3ee28453,// 33 PAY  30 

    0xbcf2d16d,// 34 PAY  31 

    0x308cdf6e,// 35 PAY  32 

    0x95cb0436,// 36 PAY  33 

    0xa1cc0897,// 37 PAY  34 

    0x578dce5f,// 38 PAY  35 

    0x9e14e78f,// 39 PAY  36 

    0x2c046c5f,// 40 PAY  37 

    0x3d9d21db,// 41 PAY  38 

    0x484309f9,// 42 PAY  39 

    0x1c54143c,// 43 PAY  40 

    0x3242af52,// 44 PAY  41 

    0x7ed3a3e7,// 45 PAY  42 

    0x182a9e19,// 46 PAY  43 

    0x3870b9e1,// 47 PAY  44 

    0xcac12dc2,// 48 PAY  45 

    0xc8a2d2c8,// 49 PAY  46 

    0x6a8bb317,// 50 PAY  47 

    0xe792e2f7,// 51 PAY  48 

    0x762bccff,// 52 PAY  49 

    0x3cc91118,// 53 PAY  50 

    0x47578617,// 54 PAY  51 

    0x8c742c85,// 55 PAY  52 

    0x2acbf70f,// 56 PAY  53 

    0xf5f4378b,// 57 PAY  54 

    0xdd4c9b92,// 58 PAY  55 

    0xf64d9c6b,// 59 PAY  56 

    0x1f0877c4,// 60 PAY  57 

    0x17316e4e,// 61 PAY  58 

    0xbd9f60ae,// 62 PAY  59 

    0xe168efe8,// 63 PAY  60 

    0x201e18ea,// 64 PAY  61 

    0xb6be696b,// 65 PAY  62 

    0x26d1bf9b,// 66 PAY  63 

    0x56c3bfa0,// 67 PAY  64 

    0xcf302eb4,// 68 PAY  65 

    0x0e30f959,// 69 PAY  66 

    0x44220191,// 70 PAY  67 

    0x72ffcca6,// 71 PAY  68 

    0x85e8e7e4,// 72 PAY  69 

    0x26a36b52,// 73 PAY  70 

    0x9d4660d7,// 74 PAY  71 

    0x7106584a,// 75 PAY  72 

    0xb7a77bf4,// 76 PAY  73 

    0xb03aec67,// 77 PAY  74 

    0x9583e68e,// 78 PAY  75 

    0x6c761c7c,// 79 PAY  76 

    0xa721fb50,// 80 PAY  77 

    0xd482ed86,// 81 PAY  78 

    0x4095d279,// 82 PAY  79 

    0x85e1000c,// 83 PAY  80 

    0xc04a1a5c,// 84 PAY  81 

    0xcbfb6eae,// 85 PAY  82 

    0x49e54a93,// 86 PAY  83 

    0x1cee031c,// 87 PAY  84 

    0x9f632453,// 88 PAY  85 

    0xf7e9470e,// 89 PAY  86 

    0x9c020c45,// 90 PAY  87 

    0x3227fc3d,// 91 PAY  88 

    0x9e8a0fe7,// 92 PAY  89 

    0x84c83bdc,// 93 PAY  90 

    0xa176eea1,// 94 PAY  91 

    0x8af0b079,// 95 PAY  92 

    0xe068ee4e,// 96 PAY  93 

    0x274fba59,// 97 PAY  94 

    0xa93e9ee2,// 98 PAY  95 

    0x71dcb54c,// 99 PAY  96 

    0x01980e08,// 100 PAY  97 

    0x66ab7e5a,// 101 PAY  98 

    0x2174e03c,// 102 PAY  99 

    0x6f7719ae,// 103 PAY 100 

    0x51d6f47f,// 104 PAY 101 

    0x48b6d299,// 105 PAY 102 

    0xe0117d0c,// 106 PAY 103 

    0xacb1c578,// 107 PAY 104 

    0x1be9aad3,// 108 PAY 105 

    0x54d806f7,// 109 PAY 106 

    0x9b01eac1,// 110 PAY 107 

    0x02edebd4,// 111 PAY 108 

    0x3ef34c60,// 112 PAY 109 

    0x9be1fbf7,// 113 PAY 110 

    0x482b3e85,// 114 PAY 111 

    0xa61b26ad,// 115 PAY 112 

    0xf79debf2,// 116 PAY 113 

    0x39abb782,// 117 PAY 114 

    0x49043f9b,// 118 PAY 115 

    0xa0ed345d,// 119 PAY 116 

    0xc19299e8,// 120 PAY 117 

    0xda82b3d5,// 121 PAY 118 

    0xd1798287,// 122 PAY 119 

    0xae7cae37,// 123 PAY 120 

    0x4b9c1051,// 124 PAY 121 

    0x3cecf81c,// 125 PAY 122 

    0x2a46f716,// 126 PAY 123 

    0x884b2474,// 127 PAY 124 

    0xa13a5776,// 128 PAY 125 

    0x799451c1,// 129 PAY 126 

    0x5ddb1714,// 130 PAY 127 

    0xd6328035,// 131 PAY 128 

    0x40ee3941,// 132 PAY 129 

    0x18f8c0be,// 133 PAY 130 

    0x609ffd5e,// 134 PAY 131 

    0xd465c598,// 135 PAY 132 

    0x8273755b,// 136 PAY 133 

    0x437afbc1,// 137 PAY 134 

    0x94d7f7f4,// 138 PAY 135 

    0xc73104ed,// 139 PAY 136 

    0xcc60bd6b,// 140 PAY 137 

    0x3217fed0,// 141 PAY 138 

    0x6c015a9d,// 142 PAY 139 

    0x691bed09,// 143 PAY 140 

    0xd487584e,// 144 PAY 141 

    0x36ce2fb7,// 145 PAY 142 

    0x960ddaac,// 146 PAY 143 

    0x1a38e754,// 147 PAY 144 

    0x4f85cd47,// 148 PAY 145 

    0x3b7769a3,// 149 PAY 146 

    0x3c292992,// 150 PAY 147 

    0x1c596e3e,// 151 PAY 148 

    0x5d72d9bb,// 152 PAY 149 

    0xe949f9de,// 153 PAY 150 

    0x7d9238ec,// 154 PAY 151 

    0x8d20709b,// 155 PAY 152 

    0xf16ba776,// 156 PAY 153 

    0x37ee35f7,// 157 PAY 154 

    0xfce72a3f,// 158 PAY 155 

    0x62c00fe9,// 159 PAY 156 

    0xd93f44a1,// 160 PAY 157 

    0x88a0675a,// 161 PAY 158 

    0x3cb21dc2,// 162 PAY 159 

    0x348db55e,// 163 PAY 160 

    0x538f88c5,// 164 PAY 161 

    0x894437d1,// 165 PAY 162 

    0xfb553156,// 166 PAY 163 

    0xf702b54c,// 167 PAY 164 

    0x197211a8,// 168 PAY 165 

    0xedd9af4a,// 169 PAY 166 

    0xbc28328d,// 170 PAY 167 

    0xd86f9873,// 171 PAY 168 

    0x773cbc0b,// 172 PAY 169 

    0x01f161b5,// 173 PAY 170 

    0x89614a0f,// 174 PAY 171 

    0x8fa6b2ad,// 175 PAY 172 

    0x1b7f8bf2,// 176 PAY 173 

    0x87ffd5f5,// 177 PAY 174 

    0xb88715cd,// 178 PAY 175 

    0x71a43545,// 179 PAY 176 

    0xcd6e2561,// 180 PAY 177 

    0x60223309,// 181 PAY 178 

    0x48c53c8a,// 182 PAY 179 

    0xac0c211b,// 183 PAY 180 

    0x624ee381,// 184 PAY 181 

    0xc0097bc3,// 185 PAY 182 

    0x3c1d74a8,// 186 PAY 183 

    0x6f89b449,// 187 PAY 184 

    0xec0018fd,// 188 PAY 185 

    0x977a52ef,// 189 PAY 186 

    0xe3bb7cfc,// 190 PAY 187 

    0x2e56a8d4,// 191 PAY 188 

    0xef5e05f8,// 192 PAY 189 

    0xfddd61e9,// 193 PAY 190 

    0x84322ed4,// 194 PAY 191 

    0x56e1fff9,// 195 PAY 192 

    0x634f1d73,// 196 PAY 193 

    0xb6f042f8,// 197 PAY 194 

    0x9c754e20,// 198 PAY 195 

    0xa527b7e4,// 199 PAY 196 

    0x64097978,// 200 PAY 197 

    0xf27caa08,// 201 PAY 198 

    0x7fb9ceba,// 202 PAY 199 

    0x762e4eb3,// 203 PAY 200 

    0x1dd1b64a,// 204 PAY 201 

    0x3f55444b,// 205 PAY 202 

    0xdb92f00a,// 206 PAY 203 

    0x1c973711,// 207 PAY 204 

    0x52652dff,// 208 PAY 205 

    0xe03fd1c8,// 209 PAY 206 

    0x5f9ccdc3,// 210 PAY 207 

    0x24c13386,// 211 PAY 208 

    0x9fd22ac0,// 212 PAY 209 

    0xa8b84ffc,// 213 PAY 210 

    0xb02e9a3b,// 214 PAY 211 

    0x2cc0ed1f,// 215 PAY 212 

    0x666ba837,// 216 PAY 213 

    0xd234d0d9,// 217 PAY 214 

    0xbe96a59c,// 218 PAY 215 

    0x70b3a743,// 219 PAY 216 

    0xccb088ff,// 220 PAY 217 

    0x56cd3919,// 221 PAY 218 

    0xa3946e78,// 222 PAY 219 

    0x9fe74185,// 223 PAY 220 

    0xd2e8070a,// 224 PAY 221 

    0x8d95a628,// 225 PAY 222 

    0x39edc4fd,// 226 PAY 223 

    0xc2578bd0,// 227 PAY 224 

    0xa521599c,// 228 PAY 225 

    0x320f5694,// 229 PAY 226 

    0x91ffe11f,// 230 PAY 227 

    0xeaf8a0fd,// 231 PAY 228 

    0x9792a92d,// 232 PAY 229 

    0x68d157fb,// 233 PAY 230 

    0x95633d51,// 234 PAY 231 

    0x354e53b8,// 235 PAY 232 

    0xa4211ba7,// 236 PAY 233 

    0x4d725ce4,// 237 PAY 234 

    0x66e420e8,// 238 PAY 235 

    0x09096b85,// 239 PAY 236 

    0xac0bbd21,// 240 PAY 237 

    0xc0191a3d,// 241 PAY 238 

    0x34584775,// 242 PAY 239 

    0xcf24edbb,// 243 PAY 240 

    0xba02727f,// 244 PAY 241 

    0x21889932,// 245 PAY 242 

    0x3773d834,// 246 PAY 243 

    0x227fef74,// 247 PAY 244 

    0x2d53b5c1,// 248 PAY 245 

    0x7677f90c,// 249 PAY 246 

    0xeb32b454,// 250 PAY 247 

    0xcb7777eb,// 251 PAY 248 

    0x13bccb8d,// 252 PAY 249 

    0x257e353d,// 253 PAY 250 

    0x16a20b2a,// 254 PAY 251 

    0x8efe0509,// 255 PAY 252 

    0x567af12d,// 256 PAY 253 

    0x5ddb0157,// 257 PAY 254 

    0x702a3bd3,// 258 PAY 255 

    0xe6c18379,// 259 PAY 256 

    0xc4f58f20,// 260 PAY 257 

    0x3e4abd62,// 261 PAY 258 

    0xef77c193,// 262 PAY 259 

    0x57281e7f,// 263 PAY 260 

    0x0fb91165,// 264 PAY 261 

    0x9ae8fc9c,// 265 PAY 262 

    0x431cb11e,// 266 PAY 263 

    0x91a8d1b3,// 267 PAY 264 

    0xb344ca6b,// 268 PAY 265 

    0x8d17d051,// 269 PAY 266 

    0x06272cbd,// 270 PAY 267 

    0x9eb1a991,// 271 PAY 268 

    0x46347585,// 272 PAY 269 

    0xae7c11e3,// 273 PAY 270 

    0xe99b3791,// 274 PAY 271 

    0x08b5c582,// 275 PAY 272 

    0xb0eaa131,// 276 PAY 273 

    0x4ac907ae,// 277 PAY 274 

    0x06ac7f2b,// 278 PAY 275 

    0xbd51ed4c,// 279 PAY 276 

    0x2f74715a,// 280 PAY 277 

    0x2677abf5,// 281 PAY 278 

    0x968ae67d,// 282 PAY 279 

    0x5f67caa1,// 283 PAY 280 

    0x35c25061,// 284 PAY 281 

    0xa6781d07,// 285 PAY 282 

    0xfa72da9b,// 286 PAY 283 

    0x1552e983,// 287 PAY 284 

    0x75ecab39,// 288 PAY 285 

    0xfba3133a,// 289 PAY 286 

    0x773b16f4,// 290 PAY 287 

    0xe2beffef,// 291 PAY 288 

    0x330c9d0f,// 292 PAY 289 

    0xd2a971be,// 293 PAY 290 

    0x1a9f7b18,// 294 PAY 291 

    0x7196bd64,// 295 PAY 292 

    0x10e874a6,// 296 PAY 293 

    0x5958fbf7,// 297 PAY 294 

    0x8124c59b,// 298 PAY 295 

    0xbf0d0225,// 299 PAY 296 

    0xf841502c,// 300 PAY 297 

    0x9168c611,// 301 PAY 298 

    0xe4eb6e94,// 302 PAY 299 

    0x2304256d,// 303 PAY 300 

    0x8bcc3884,// 304 PAY 301 

    0x43b24b86,// 305 PAY 302 

    0xda18df92,// 306 PAY 303 

    0x0289e5a1,// 307 PAY 304 

    0x30429588,// 308 PAY 305 

    0x0083015f,// 309 PAY 306 

    0x8dd84a57,// 310 PAY 307 

    0xff187053,// 311 PAY 308 

    0xc84c1a22,// 312 PAY 309 

    0x5ddf92f3,// 313 PAY 310 

    0x575e80d5,// 314 PAY 311 

    0x506bb79b,// 315 PAY 312 

    0x498e56aa,// 316 PAY 313 

    0xf465763f,// 317 PAY 314 

    0x51643cc0,// 318 PAY 315 

    0x0cacac93,// 319 PAY 316 

    0xb60e1ab5,// 320 PAY 317 

    0x96f2e49b,// 321 PAY 318 

    0xc0ec5b55,// 322 PAY 319 

    0xe49ac39e,// 323 PAY 320 

    0x2466371d,// 324 PAY 321 

    0x2fa965db,// 325 PAY 322 

    0x5e997376,// 326 PAY 323 

    0x412bf2eb,// 327 PAY 324 

    0x0b2483f3,// 328 PAY 325 

    0xf9b941ec,// 329 PAY 326 

    0xebc5d1a9,// 330 PAY 327 

    0x1e8c3287,// 331 PAY 328 

    0xa68a1469,// 332 PAY 329 

    0xc2a85e9f,// 333 PAY 330 

    0xd1871512,// 334 PAY 331 

    0x760fb4b5,// 335 PAY 332 

    0x415d2b4f,// 336 PAY 333 

    0xce6c9445,// 337 PAY 334 

    0x9f143d6f,// 338 PAY 335 

    0x76a89f3a,// 339 PAY 336 

    0x4ba3485c,// 340 PAY 337 

    0xf66e7cf0,// 341 PAY 338 

    0xc646dc8d,// 342 PAY 339 

    0xfd79d453,// 343 PAY 340 

    0xfd7d0d46,// 344 PAY 341 

    0xd79d722a,// 345 PAY 342 

    0x97ebbad4,// 346 PAY 343 

    0xffefd700,// 347 PAY 344 

    0x9fab533d,// 348 PAY 345 

    0xf91feefb,// 349 PAY 346 

    0xde531f5c,// 350 PAY 347 

    0x008d2522,// 351 PAY 348 

    0xf7a4aa2c,// 352 PAY 349 

    0x3f4de058,// 353 PAY 350 

    0xb36a15d0,// 354 PAY 351 

    0xbdfef8a6,// 355 PAY 352 

    0xabb190eb,// 356 PAY 353 

    0x27e11e59,// 357 PAY 354 

    0xfc4f1ced,// 358 PAY 355 

    0x85a9ead2,// 359 PAY 356 

    0xb6cd7379,// 360 PAY 357 

    0x9a5d61e0,// 361 PAY 358 

    0x56aeaa35,// 362 PAY 359 

    0x7f8a31d7,// 363 PAY 360 

    0xec6a8435,// 364 PAY 361 

    0xb8599bef,// 365 PAY 362 

    0x4ef20e68,// 366 PAY 363 

    0x677b1079,// 367 PAY 364 

    0xe7d1f895,// 368 PAY 365 

    0x8ce1a764,// 369 PAY 366 

    0xa0fe3268,// 370 PAY 367 

    0x1d81b6a9,// 371 PAY 368 

    0xf3abda71,// 372 PAY 369 

    0x4defc312,// 373 PAY 370 

    0xe7029aed,// 374 PAY 371 

    0xdcda25ad,// 375 PAY 372 

    0x37dcdc98,// 376 PAY 373 

    0xa1480b78,// 377 PAY 374 

    0x3d1bc612,// 378 PAY 375 

    0x311cf992,// 379 PAY 376 

    0xbb71d30a,// 380 PAY 377 

    0xbd4e0751,// 381 PAY 378 

    0x1a45a1ea,// 382 PAY 379 

    0xfe6be178,// 383 PAY 380 

    0xba9ccefb,// 384 PAY 381 

    0xc0c7bb84,// 385 PAY 382 

    0x27a61327,// 386 PAY 383 

    0xf5892ce6,// 387 PAY 384 

    0x2661b36f,// 388 PAY 385 

    0x2d63f449,// 389 PAY 386 

/// HASH is  16 bytes 

    0xfa72da9b,// 390 HSH   1 

    0x1552e983,// 391 HSH   2 

    0x75ecab39,// 392 HSH   3 

    0xfba3133a,// 393 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 233 

/// STA pkt_idx        : 239 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5b 

    0x03bc5be9 // 394 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt87_tmpl[] = {
    0x08010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0e 

/// ECH pdu_tag        : 0x00 

    0x000e0000,// 2 ECH   1 

/// BDA is 173 words. 

/// BDA size     is 685 (0x2ad) 

/// BDA id       is 0x316c 

    0x02ad316c,// 3 BDA   1 

/// PAY Generic Data size   : 685 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x7b96287b,// 4 PAY   1 

    0x2b74ab2c,// 5 PAY   2 

    0xa7c00938,// 6 PAY   3 

    0xa645c18f,// 7 PAY   4 

    0xf1fd646c,// 8 PAY   5 

    0x6c8276b1,// 9 PAY   6 

    0xd57ddb86,// 10 PAY   7 

    0xa778992c,// 11 PAY   8 

    0x6ffe4f6e,// 12 PAY   9 

    0xc91b2f04,// 13 PAY  10 

    0x5ee6a32a,// 14 PAY  11 

    0x56fbda0e,// 15 PAY  12 

    0xa239c519,// 16 PAY  13 

    0x621cd48a,// 17 PAY  14 

    0xb5836640,// 18 PAY  15 

    0x7d869bcd,// 19 PAY  16 

    0xc3fee07a,// 20 PAY  17 

    0x8bdcc255,// 21 PAY  18 

    0x34f69c38,// 22 PAY  19 

    0x51b23491,// 23 PAY  20 

    0xa0cd44c7,// 24 PAY  21 

    0x68ff08f1,// 25 PAY  22 

    0x8bc453c0,// 26 PAY  23 

    0x19f04cab,// 27 PAY  24 

    0x9e7d37a8,// 28 PAY  25 

    0x84f9a53e,// 29 PAY  26 

    0x13574201,// 30 PAY  27 

    0x12284ed7,// 31 PAY  28 

    0xd08d2a96,// 32 PAY  29 

    0x4ac898c6,// 33 PAY  30 

    0x30b07bc2,// 34 PAY  31 

    0xbca704cf,// 35 PAY  32 

    0xc317fc1c,// 36 PAY  33 

    0xe5de6e03,// 37 PAY  34 

    0x6a891a9c,// 38 PAY  35 

    0xe7e9ab26,// 39 PAY  36 

    0xedeab293,// 40 PAY  37 

    0x91010c3a,// 41 PAY  38 

    0x3523440e,// 42 PAY  39 

    0x1b4e633d,// 43 PAY  40 

    0x6f8567b9,// 44 PAY  41 

    0x8601a2cc,// 45 PAY  42 

    0xbee3fc86,// 46 PAY  43 

    0x3b9d0493,// 47 PAY  44 

    0xc2f1e8be,// 48 PAY  45 

    0xbf9b8954,// 49 PAY  46 

    0xba02dd5d,// 50 PAY  47 

    0x00ac59bb,// 51 PAY  48 

    0x95155471,// 52 PAY  49 

    0x581e0dbe,// 53 PAY  50 

    0x226bbe6d,// 54 PAY  51 

    0x5324ce6a,// 55 PAY  52 

    0xb9204ed8,// 56 PAY  53 

    0x580edef7,// 57 PAY  54 

    0xdb4bbc4c,// 58 PAY  55 

    0x035a6412,// 59 PAY  56 

    0x2b44f83e,// 60 PAY  57 

    0xb135c302,// 61 PAY  58 

    0x7e2249e9,// 62 PAY  59 

    0xf2df3f20,// 63 PAY  60 

    0xbe0f4a58,// 64 PAY  61 

    0x60f19455,// 65 PAY  62 

    0x8a0c66d4,// 66 PAY  63 

    0x8a0cc0b0,// 67 PAY  64 

    0xb2c9c9bd,// 68 PAY  65 

    0xec6f50fb,// 69 PAY  66 

    0xa84afffb,// 70 PAY  67 

    0x7de1c23b,// 71 PAY  68 

    0xb6da6bd7,// 72 PAY  69 

    0xe5bab197,// 73 PAY  70 

    0x80b2d08d,// 74 PAY  71 

    0x3fdeac9f,// 75 PAY  72 

    0x8aad85a2,// 76 PAY  73 

    0x36977b71,// 77 PAY  74 

    0x9f9e79de,// 78 PAY  75 

    0x39bcdee4,// 79 PAY  76 

    0x3a7e3af4,// 80 PAY  77 

    0xfd7e3238,// 81 PAY  78 

    0x76d059f6,// 82 PAY  79 

    0x42b88940,// 83 PAY  80 

    0xedb4b2cf,// 84 PAY  81 

    0xfda78aa0,// 85 PAY  82 

    0xa7882a0f,// 86 PAY  83 

    0xafc60964,// 87 PAY  84 

    0x2262a9b3,// 88 PAY  85 

    0xcfdf8061,// 89 PAY  86 

    0xae36b1f4,// 90 PAY  87 

    0xab69e80e,// 91 PAY  88 

    0xee852db2,// 92 PAY  89 

    0x6279d550,// 93 PAY  90 

    0xe4ca4f93,// 94 PAY  91 

    0x46777a9f,// 95 PAY  92 

    0x3c330961,// 96 PAY  93 

    0xd5a722fa,// 97 PAY  94 

    0xbbb03d76,// 98 PAY  95 

    0xdd7a13ea,// 99 PAY  96 

    0xe999a1fd,// 100 PAY  97 

    0x140e9d59,// 101 PAY  98 

    0x6bdb9de5,// 102 PAY  99 

    0x14c7c219,// 103 PAY 100 

    0xdd777de4,// 104 PAY 101 

    0x87ec4b01,// 105 PAY 102 

    0x94aee0f8,// 106 PAY 103 

    0x4e54601c,// 107 PAY 104 

    0xf868aff7,// 108 PAY 105 

    0xd7147456,// 109 PAY 106 

    0x10afdb20,// 110 PAY 107 

    0xddf83fbe,// 111 PAY 108 

    0xcfefefc9,// 112 PAY 109 

    0xaae30b20,// 113 PAY 110 

    0x2a3b0ef7,// 114 PAY 111 

    0xa38702a2,// 115 PAY 112 

    0xc0259c57,// 116 PAY 113 

    0xed21bf4b,// 117 PAY 114 

    0xf1aad939,// 118 PAY 115 

    0x93e1f34c,// 119 PAY 116 

    0xe29a6efe,// 120 PAY 117 

    0x0d8f6082,// 121 PAY 118 

    0x5ff14f72,// 122 PAY 119 

    0xbf4b3a11,// 123 PAY 120 

    0x4a0abb2c,// 124 PAY 121 

    0xd7dad8d3,// 125 PAY 122 

    0x1723dc9d,// 126 PAY 123 

    0xb7195a5e,// 127 PAY 124 

    0xae8e1a5e,// 128 PAY 125 

    0x9b9b6700,// 129 PAY 126 

    0x92600b6b,// 130 PAY 127 

    0x5ff5308a,// 131 PAY 128 

    0xa2dfe733,// 132 PAY 129 

    0xcc0ba393,// 133 PAY 130 

    0x335d39f9,// 134 PAY 131 

    0xacb386f1,// 135 PAY 132 

    0x311540b0,// 136 PAY 133 

    0x6392af58,// 137 PAY 134 

    0xe3606671,// 138 PAY 135 

    0xc89225db,// 139 PAY 136 

    0x39a6458d,// 140 PAY 137 

    0xd0b9ee12,// 141 PAY 138 

    0xbd1c2f7b,// 142 PAY 139 

    0x2b9b5a7b,// 143 PAY 140 

    0x2da50b72,// 144 PAY 141 

    0x23e1a2d3,// 145 PAY 142 

    0xa3a4ae1a,// 146 PAY 143 

    0x85715d1c,// 147 PAY 144 

    0x2f527f5a,// 148 PAY 145 

    0x14bdde67,// 149 PAY 146 

    0x5b664941,// 150 PAY 147 

    0x14908e95,// 151 PAY 148 

    0x7b90c7f9,// 152 PAY 149 

    0x572bf281,// 153 PAY 150 

    0x6d20cdba,// 154 PAY 151 

    0xe300eda6,// 155 PAY 152 

    0xe99619b3,// 156 PAY 153 

    0xd5f82c60,// 157 PAY 154 

    0xcd22178c,// 158 PAY 155 

    0x5c882a41,// 159 PAY 156 

    0xef8adbab,// 160 PAY 157 

    0x1d92e479,// 161 PAY 158 

    0x5e7bab02,// 162 PAY 159 

    0xf1b77e7b,// 163 PAY 160 

    0x5c340b2f,// 164 PAY 161 

    0x28060ae5,// 165 PAY 162 

    0xdeb3d968,// 166 PAY 163 

    0xcd5d8249,// 167 PAY 164 

    0xed15cd0d,// 168 PAY 165 

    0x6ff70ccb,// 169 PAY 166 

    0x11eed592,// 170 PAY 167 

    0x1420e20b,// 171 PAY 168 

    0x4ee68569,// 172 PAY 169 

    0x4a57d9e6,// 173 PAY 170 

    0xa2e4ce15,// 174 PAY 171 

    0xa6000000,// 175 PAY 172 

/// STA is 1 words. 

/// STA num_pkts       : 189 

/// STA pkt_idx        : 136 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xcb 

    0x0220cbbd // 176 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt88_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 215 words. 

/// BDA size     is 855 (0x357) 

/// BDA id       is 0x441 

    0x03570441,// 3 BDA   1 

/// PAY Generic Data size   : 855 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x70f1b087,// 4 PAY   1 

    0xf8926a33,// 5 PAY   2 

    0xc843928a,// 6 PAY   3 

    0xfdb54d09,// 7 PAY   4 

    0x487e56ff,// 8 PAY   5 

    0xea3e55cd,// 9 PAY   6 

    0x7c863498,// 10 PAY   7 

    0x968a5c61,// 11 PAY   8 

    0x8c12badf,// 12 PAY   9 

    0x01f9854e,// 13 PAY  10 

    0xe71f7e32,// 14 PAY  11 

    0x64d1c5ca,// 15 PAY  12 

    0xc4784609,// 16 PAY  13 

    0x2d9ea248,// 17 PAY  14 

    0x1faa0ef6,// 18 PAY  15 

    0x23077a4a,// 19 PAY  16 

    0x3ccd6844,// 20 PAY  17 

    0xae72feee,// 21 PAY  18 

    0x50ef4a0b,// 22 PAY  19 

    0xcee41669,// 23 PAY  20 

    0xc85e12c1,// 24 PAY  21 

    0x25ea0424,// 25 PAY  22 

    0x5e8843c0,// 26 PAY  23 

    0x9e2ef598,// 27 PAY  24 

    0x6b0f6b84,// 28 PAY  25 

    0xdb9c5012,// 29 PAY  26 

    0x97b42139,// 30 PAY  27 

    0x5d6c5cbd,// 31 PAY  28 

    0x27924f2c,// 32 PAY  29 

    0xd0ac9e13,// 33 PAY  30 

    0x99f1ae97,// 34 PAY  31 

    0x23f5c7e3,// 35 PAY  32 

    0xc21aa78d,// 36 PAY  33 

    0xa1031b7b,// 37 PAY  34 

    0xaef82482,// 38 PAY  35 

    0x43f1b8e4,// 39 PAY  36 

    0x279c7ff6,// 40 PAY  37 

    0xee366fe9,// 41 PAY  38 

    0xc5ce5dd0,// 42 PAY  39 

    0xd560c14c,// 43 PAY  40 

    0x0249018a,// 44 PAY  41 

    0x2d8d447d,// 45 PAY  42 

    0x00b94304,// 46 PAY  43 

    0x0e94cd3a,// 47 PAY  44 

    0xb1a7270b,// 48 PAY  45 

    0x2187f4fd,// 49 PAY  46 

    0x052d1c47,// 50 PAY  47 

    0x32be9cd2,// 51 PAY  48 

    0x33acf0f6,// 52 PAY  49 

    0x85114f4f,// 53 PAY  50 

    0xdbbed9dd,// 54 PAY  51 

    0x12625417,// 55 PAY  52 

    0x3ba11e02,// 56 PAY  53 

    0x18fd13ee,// 57 PAY  54 

    0x3fcdfca0,// 58 PAY  55 

    0xe32d175f,// 59 PAY  56 

    0x4b32557f,// 60 PAY  57 

    0x82140a07,// 61 PAY  58 

    0x3bfb6992,// 62 PAY  59 

    0x6f2cef1d,// 63 PAY  60 

    0xc55959f7,// 64 PAY  61 

    0xdaed788a,// 65 PAY  62 

    0x96bc6719,// 66 PAY  63 

    0x72a95d9e,// 67 PAY  64 

    0x4d4a3e4d,// 68 PAY  65 

    0xa874d11c,// 69 PAY  66 

    0xc43f31a3,// 70 PAY  67 

    0x59341228,// 71 PAY  68 

    0x4260dcd2,// 72 PAY  69 

    0x93ded0fc,// 73 PAY  70 

    0x72aa71e3,// 74 PAY  71 

    0x25dfc285,// 75 PAY  72 

    0x21ee6f8c,// 76 PAY  73 

    0x8eb6ff63,// 77 PAY  74 

    0x3c4556f8,// 78 PAY  75 

    0x4a2c1f97,// 79 PAY  76 

    0x05bad142,// 80 PAY  77 

    0xf485909c,// 81 PAY  78 

    0x79e86bd0,// 82 PAY  79 

    0x46d6d21d,// 83 PAY  80 

    0x2feae521,// 84 PAY  81 

    0xf8c388e5,// 85 PAY  82 

    0xa38bd9b9,// 86 PAY  83 

    0x220729ec,// 87 PAY  84 

    0xa6e0e6e1,// 88 PAY  85 

    0xa72346e9,// 89 PAY  86 

    0x44e2f6a9,// 90 PAY  87 

    0xdfd1fb15,// 91 PAY  88 

    0x042536a1,// 92 PAY  89 

    0xc73d1c12,// 93 PAY  90 

    0xf23b4a7d,// 94 PAY  91 

    0x8386b044,// 95 PAY  92 

    0x47114a19,// 96 PAY  93 

    0x588c51c8,// 97 PAY  94 

    0x19cdd18c,// 98 PAY  95 

    0xa433baed,// 99 PAY  96 

    0x1e5e2317,// 100 PAY  97 

    0x0e0c59f3,// 101 PAY  98 

    0x172aa259,// 102 PAY  99 

    0x9579c235,// 103 PAY 100 

    0x9d5c0a74,// 104 PAY 101 

    0xfcf423ef,// 105 PAY 102 

    0x54736e8d,// 106 PAY 103 

    0xfe203872,// 107 PAY 104 

    0xcea72c11,// 108 PAY 105 

    0x878eeae3,// 109 PAY 106 

    0x7ae2daa3,// 110 PAY 107 

    0x772a75f9,// 111 PAY 108 

    0xefd1dbe1,// 112 PAY 109 

    0xd59a3f65,// 113 PAY 110 

    0x1cdbeba9,// 114 PAY 111 

    0x8aa9f34f,// 115 PAY 112 

    0x09bab02d,// 116 PAY 113 

    0xd9384ea7,// 117 PAY 114 

    0x4df80cf4,// 118 PAY 115 

    0x9eef6e81,// 119 PAY 116 

    0x6aa80b32,// 120 PAY 117 

    0x234fee70,// 121 PAY 118 

    0x1258c433,// 122 PAY 119 

    0xfd045260,// 123 PAY 120 

    0xb796052a,// 124 PAY 121 

    0xde5b41a9,// 125 PAY 122 

    0x66ad8510,// 126 PAY 123 

    0x5bc77597,// 127 PAY 124 

    0xdb59bd96,// 128 PAY 125 

    0x0b349a86,// 129 PAY 126 

    0xb8bd938f,// 130 PAY 127 

    0x18d5a502,// 131 PAY 128 

    0xa7ed1c74,// 132 PAY 129 

    0xeb5a62f2,// 133 PAY 130 

    0x73d837fa,// 134 PAY 131 

    0x9063ed9f,// 135 PAY 132 

    0x424af5a9,// 136 PAY 133 

    0x70fe5f16,// 137 PAY 134 

    0x5cc4e16a,// 138 PAY 135 

    0x270c0b3c,// 139 PAY 136 

    0x2733d574,// 140 PAY 137 

    0x5bb80ee0,// 141 PAY 138 

    0x0a280673,// 142 PAY 139 

    0x3a55428b,// 143 PAY 140 

    0x743274d2,// 144 PAY 141 

    0x3e012be5,// 145 PAY 142 

    0x61258a40,// 146 PAY 143 

    0x0792028a,// 147 PAY 144 

    0xe596b208,// 148 PAY 145 

    0x75b3c00b,// 149 PAY 146 

    0xf0662147,// 150 PAY 147 

    0x75326226,// 151 PAY 148 

    0x1918d33b,// 152 PAY 149 

    0x9a9fd0d3,// 153 PAY 150 

    0xab74b1d2,// 154 PAY 151 

    0x216913de,// 155 PAY 152 

    0xc11e3931,// 156 PAY 153 

    0x8545e6e4,// 157 PAY 154 

    0x505b941f,// 158 PAY 155 

    0xf45e6606,// 159 PAY 156 

    0x3f3a1504,// 160 PAY 157 

    0x66df181c,// 161 PAY 158 

    0xe2cd4a52,// 162 PAY 159 

    0xa3a89c7f,// 163 PAY 160 

    0x3a54e520,// 164 PAY 161 

    0x31d9d5d0,// 165 PAY 162 

    0x53b7fa09,// 166 PAY 163 

    0xad0922cc,// 167 PAY 164 

    0xfadcda04,// 168 PAY 165 

    0x17bf791e,// 169 PAY 166 

    0x92a1f21c,// 170 PAY 167 

    0x84b94734,// 171 PAY 168 

    0x3679dc29,// 172 PAY 169 

    0x91b89800,// 173 PAY 170 

    0x3b3060f6,// 174 PAY 171 

    0xbc5b08c3,// 175 PAY 172 

    0xa8b55892,// 176 PAY 173 

    0x6b5df41c,// 177 PAY 174 

    0xdc8d19b1,// 178 PAY 175 

    0x46bdd9a0,// 179 PAY 176 

    0xe1950c67,// 180 PAY 177 

    0x69e99743,// 181 PAY 178 

    0xa9c55ce5,// 182 PAY 179 

    0xe36881c8,// 183 PAY 180 

    0x3c238558,// 184 PAY 181 

    0x720bde03,// 185 PAY 182 

    0x5db5578c,// 186 PAY 183 

    0x71dc0bbb,// 187 PAY 184 

    0x207f971f,// 188 PAY 185 

    0x810c51b1,// 189 PAY 186 

    0x97963451,// 190 PAY 187 

    0xcf20e181,// 191 PAY 188 

    0xa70ff795,// 192 PAY 189 

    0x03adb374,// 193 PAY 190 

    0x7ef840bb,// 194 PAY 191 

    0x958772c9,// 195 PAY 192 

    0x2e6ec2a4,// 196 PAY 193 

    0xe70ae758,// 197 PAY 194 

    0x05b10736,// 198 PAY 195 

    0x6d59d136,// 199 PAY 196 

    0x4d5cc948,// 200 PAY 197 

    0xb566aba2,// 201 PAY 198 

    0xc65c9b22,// 202 PAY 199 

    0xceb09612,// 203 PAY 200 

    0x20600395,// 204 PAY 201 

    0xe37870cc,// 205 PAY 202 

    0xe60eb481,// 206 PAY 203 

    0x7395b4c3,// 207 PAY 204 

    0x760be11f,// 208 PAY 205 

    0x99ccd646,// 209 PAY 206 

    0x78667297,// 210 PAY 207 

    0x5ebf2132,// 211 PAY 208 

    0x2092bbea,// 212 PAY 209 

    0x0325167e,// 213 PAY 210 

    0xd5dfd0ad,// 214 PAY 211 

    0x3359d6a5,// 215 PAY 212 

    0xbf4533a8,// 216 PAY 213 

    0x421b0000,// 217 PAY 214 

/// STA is 1 words. 

/// STA num_pkts       : 126 

/// STA pkt_idx        : 146 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf1 

    0x0248f17e // 218 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt89_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0c 

/// ECH pdu_tag        : 0x00 

    0x000c0000,// 2 ECH   1 

/// BDA is 225 words. 

/// BDA size     is 894 (0x37e) 

/// BDA id       is 0x8e30 

    0x037e8e30,// 3 BDA   1 

/// PAY Generic Data size   : 894 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xccbd4598,// 4 PAY   1 

    0x35f267ca,// 5 PAY   2 

    0x43dba36c,// 6 PAY   3 

    0xba427231,// 7 PAY   4 

    0x84f1a482,// 8 PAY   5 

    0x902c1b2b,// 9 PAY   6 

    0x20d18691,// 10 PAY   7 

    0xc5dece49,// 11 PAY   8 

    0xe9c34226,// 12 PAY   9 

    0x81327832,// 13 PAY  10 

    0x61bea327,// 14 PAY  11 

    0x7ddeb6b2,// 15 PAY  12 

    0xc76f4c5e,// 16 PAY  13 

    0x5568951e,// 17 PAY  14 

    0x0a483955,// 18 PAY  15 

    0x9662aed2,// 19 PAY  16 

    0xddd378e3,// 20 PAY  17 

    0xfa262bf7,// 21 PAY  18 

    0x7fa6a941,// 22 PAY  19 

    0xe93ac3fa,// 23 PAY  20 

    0xd5bab350,// 24 PAY  21 

    0x04a62102,// 25 PAY  22 

    0xbb5ed855,// 26 PAY  23 

    0x6deb6e7c,// 27 PAY  24 

    0x3ef50246,// 28 PAY  25 

    0xf9dd6201,// 29 PAY  26 

    0x407da0ab,// 30 PAY  27 

    0xf3ffae29,// 31 PAY  28 

    0x6958f527,// 32 PAY  29 

    0x7b33df8f,// 33 PAY  30 

    0x7de930dd,// 34 PAY  31 

    0xfa269134,// 35 PAY  32 

    0xdaaa33fd,// 36 PAY  33 

    0xa7f662d6,// 37 PAY  34 

    0x5833c2bc,// 38 PAY  35 

    0x7eeac37d,// 39 PAY  36 

    0xadf832b3,// 40 PAY  37 

    0x958b5bd1,// 41 PAY  38 

    0x24cc4020,// 42 PAY  39 

    0x2be2871c,// 43 PAY  40 

    0xa3b152e1,// 44 PAY  41 

    0x0f42532a,// 45 PAY  42 

    0xe8f95d94,// 46 PAY  43 

    0xe2c3cbce,// 47 PAY  44 

    0x1a58581f,// 48 PAY  45 

    0x66b70a78,// 49 PAY  46 

    0x793d2e96,// 50 PAY  47 

    0xd97e28ba,// 51 PAY  48 

    0x7b18c0bd,// 52 PAY  49 

    0x634ce04a,// 53 PAY  50 

    0x2b40d72a,// 54 PAY  51 

    0xac08df59,// 55 PAY  52 

    0x188c0f19,// 56 PAY  53 

    0x34662558,// 57 PAY  54 

    0xbf65b235,// 58 PAY  55 

    0xb584a9d2,// 59 PAY  56 

    0x5c1a338f,// 60 PAY  57 

    0xec958d51,// 61 PAY  58 

    0x3bfce8cb,// 62 PAY  59 

    0x4bc9b534,// 63 PAY  60 

    0x5f339aa0,// 64 PAY  61 

    0x18fdcbb9,// 65 PAY  62 

    0x03aa28bf,// 66 PAY  63 

    0x8045ee8a,// 67 PAY  64 

    0x344f53e5,// 68 PAY  65 

    0x96b21a90,// 69 PAY  66 

    0xef55084e,// 70 PAY  67 

    0xb40ae26d,// 71 PAY  68 

    0xa42f726b,// 72 PAY  69 

    0x9135557d,// 73 PAY  70 

    0xf7f9ae74,// 74 PAY  71 

    0xa4dd2c54,// 75 PAY  72 

    0x7c38907a,// 76 PAY  73 

    0x554b9a95,// 77 PAY  74 

    0x74de714c,// 78 PAY  75 

    0x6744f844,// 79 PAY  76 

    0x763556e3,// 80 PAY  77 

    0xb96274ed,// 81 PAY  78 

    0x8e942014,// 82 PAY  79 

    0xfb0b7acd,// 83 PAY  80 

    0x71f94ab8,// 84 PAY  81 

    0x866231b8,// 85 PAY  82 

    0x26cf9323,// 86 PAY  83 

    0x7164d5cc,// 87 PAY  84 

    0x49bf10a5,// 88 PAY  85 

    0xc0ec0732,// 89 PAY  86 

    0x3f93eab1,// 90 PAY  87 

    0x354502d9,// 91 PAY  88 

    0x028538e8,// 92 PAY  89 

    0xdbb34468,// 93 PAY  90 

    0x83436fbb,// 94 PAY  91 

    0x2117f983,// 95 PAY  92 

    0x960ec66f,// 96 PAY  93 

    0xb6cf41a4,// 97 PAY  94 

    0x31a25c52,// 98 PAY  95 

    0x7e83ffd6,// 99 PAY  96 

    0xf94d607e,// 100 PAY  97 

    0x4a5973d3,// 101 PAY  98 

    0x69b1b3be,// 102 PAY  99 

    0x0c3d70df,// 103 PAY 100 

    0xde56529b,// 104 PAY 101 

    0x3b061760,// 105 PAY 102 

    0x6de13b7f,// 106 PAY 103 

    0x4b900b8b,// 107 PAY 104 

    0xdf452656,// 108 PAY 105 

    0xbc6665ec,// 109 PAY 106 

    0x4d3f1c60,// 110 PAY 107 

    0x20e60091,// 111 PAY 108 

    0x151d0ad7,// 112 PAY 109 

    0xafb15817,// 113 PAY 110 

    0x367c4c7c,// 114 PAY 111 

    0xd746d0f9,// 115 PAY 112 

    0xbc1e292d,// 116 PAY 113 

    0x8e969492,// 117 PAY 114 

    0xdc16ddd3,// 118 PAY 115 

    0x263e6350,// 119 PAY 116 

    0x3b3bbceb,// 120 PAY 117 

    0x53c9a07e,// 121 PAY 118 

    0x6ae97049,// 122 PAY 119 

    0x9c8b987e,// 123 PAY 120 

    0x8c3c1434,// 124 PAY 121 

    0x588efd7e,// 125 PAY 122 

    0xee850db7,// 126 PAY 123 

    0x79e92ef1,// 127 PAY 124 

    0xc865cc8b,// 128 PAY 125 

    0x00f71e06,// 129 PAY 126 

    0x1cd09623,// 130 PAY 127 

    0x4bc3497a,// 131 PAY 128 

    0xab9f5013,// 132 PAY 129 

    0x492f83f6,// 133 PAY 130 

    0xf4459d74,// 134 PAY 131 

    0xffd6c45f,// 135 PAY 132 

    0x68788b64,// 136 PAY 133 

    0x4f79c807,// 137 PAY 134 

    0xab119434,// 138 PAY 135 

    0x0e4d4eed,// 139 PAY 136 

    0x9afba0c2,// 140 PAY 137 

    0x22284111,// 141 PAY 138 

    0x22975796,// 142 PAY 139 

    0xdd8c64f3,// 143 PAY 140 

    0x9b1243dd,// 144 PAY 141 

    0xc05b09b7,// 145 PAY 142 

    0xf6c6c7d6,// 146 PAY 143 

    0xf92204cb,// 147 PAY 144 

    0xd7945635,// 148 PAY 145 

    0x697acebe,// 149 PAY 146 

    0x2453f7c3,// 150 PAY 147 

    0x31261569,// 151 PAY 148 

    0x85a8d6e6,// 152 PAY 149 

    0x1eef3722,// 153 PAY 150 

    0xc4550b54,// 154 PAY 151 

    0x65dccf21,// 155 PAY 152 

    0x8a656873,// 156 PAY 153 

    0x69af7a77,// 157 PAY 154 

    0xcb6eaeb8,// 158 PAY 155 

    0x21e14196,// 159 PAY 156 

    0x00778185,// 160 PAY 157 

    0x20545890,// 161 PAY 158 

    0x06be8556,// 162 PAY 159 

    0x8572174c,// 163 PAY 160 

    0xea40106c,// 164 PAY 161 

    0x78a71d59,// 165 PAY 162 

    0x206219fa,// 166 PAY 163 

    0x5dc3ff9a,// 167 PAY 164 

    0x5247a1ab,// 168 PAY 165 

    0x798c61f7,// 169 PAY 166 

    0x67aa5c9f,// 170 PAY 167 

    0x8daa28c4,// 171 PAY 168 

    0x1cb42b6b,// 172 PAY 169 

    0x8df4dfc6,// 173 PAY 170 

    0x16ed272b,// 174 PAY 171 

    0x10a9d7ce,// 175 PAY 172 

    0xf4b44629,// 176 PAY 173 

    0xba26fab9,// 177 PAY 174 

    0x0eeb083f,// 178 PAY 175 

    0x04a85f55,// 179 PAY 176 

    0x2d62ee66,// 180 PAY 177 

    0x129fcc02,// 181 PAY 178 

    0x419e0b47,// 182 PAY 179 

    0xb95783fa,// 183 PAY 180 

    0x32af51b1,// 184 PAY 181 

    0x4d7c8188,// 185 PAY 182 

    0xd7627778,// 186 PAY 183 

    0xfe4d8e4a,// 187 PAY 184 

    0x23462366,// 188 PAY 185 

    0xaede3a42,// 189 PAY 186 

    0x41ad8f84,// 190 PAY 187 

    0xcf28419b,// 191 PAY 188 

    0xa7bd90b3,// 192 PAY 189 

    0xabbbd31c,// 193 PAY 190 

    0xb90bd467,// 194 PAY 191 

    0x394d0e3d,// 195 PAY 192 

    0x8f4c04c2,// 196 PAY 193 

    0xc97f0f57,// 197 PAY 194 

    0xb3f1eba3,// 198 PAY 195 

    0x65d9ecc4,// 199 PAY 196 

    0xc413214a,// 200 PAY 197 

    0xb6053f52,// 201 PAY 198 

    0x957e0b52,// 202 PAY 199 

    0x66814b8a,// 203 PAY 200 

    0xf58d99ae,// 204 PAY 201 

    0xa5079d70,// 205 PAY 202 

    0xe2b4a47e,// 206 PAY 203 

    0xece6dcc2,// 207 PAY 204 

    0x9d49a2f6,// 208 PAY 205 

    0xe04cecb7,// 209 PAY 206 

    0x555d24a6,// 210 PAY 207 

    0xb27ff63e,// 211 PAY 208 

    0x555b96ce,// 212 PAY 209 

    0x1d05644b,// 213 PAY 210 

    0x364e6178,// 214 PAY 211 

    0x154c8c74,// 215 PAY 212 

    0xf7486f06,// 216 PAY 213 

    0x20c2f119,// 217 PAY 214 

    0x5cd38630,// 218 PAY 215 

    0xa9b324c6,// 219 PAY 216 

    0xf4d321de,// 220 PAY 217 

    0x246af20b,// 221 PAY 218 

    0x2c33fb04,// 222 PAY 219 

    0x454090b8,// 223 PAY 220 

    0xf94966bd,// 224 PAY 221 

    0x8f66ec60,// 225 PAY 222 

    0x668c813d,// 226 PAY 223 

    0x9c4d0000,// 227 PAY 224 

/// HASH is  4 bytes 

    0x2453f7c3,// 228 HSH   1 

/// STA is 1 words. 

/// STA num_pkts       : 49 

/// STA pkt_idx        : 98 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc1 

    0x0188c131 // 229 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt90_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 443 words. 

/// BDA size     is 1766 (0x6e6) 

/// BDA id       is 0xa336 

    0x06e6a336,// 3 BDA   1 

/// PAY Generic Data size   : 1766 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x208dadad,// 4 PAY   1 

    0xdc71b748,// 5 PAY   2 

    0x60682b39,// 6 PAY   3 

    0xb759a79d,// 7 PAY   4 

    0x3f4e8a4e,// 8 PAY   5 

    0x6c6bfca7,// 9 PAY   6 

    0x83a98668,// 10 PAY   7 

    0x953716a6,// 11 PAY   8 

    0x6ad17dde,// 12 PAY   9 

    0x61c72ac6,// 13 PAY  10 

    0xda347fed,// 14 PAY  11 

    0xed06abdb,// 15 PAY  12 

    0x0432576b,// 16 PAY  13 

    0x9f2490bd,// 17 PAY  14 

    0xa9e4f933,// 18 PAY  15 

    0x3192f6d7,// 19 PAY  16 

    0xa59b53ff,// 20 PAY  17 

    0xceb69a87,// 21 PAY  18 

    0xaa0bd0fd,// 22 PAY  19 

    0xb4c8dbdb,// 23 PAY  20 

    0xee72e574,// 24 PAY  21 

    0x3de8e7de,// 25 PAY  22 

    0xe7e5fcaa,// 26 PAY  23 

    0xfc315abb,// 27 PAY  24 

    0x91153f37,// 28 PAY  25 

    0x4a800f97,// 29 PAY  26 

    0x9448c060,// 30 PAY  27 

    0xe38f650a,// 31 PAY  28 

    0x55a70ab1,// 32 PAY  29 

    0x316089ac,// 33 PAY  30 

    0x0f7394a1,// 34 PAY  31 

    0xd94a8a2a,// 35 PAY  32 

    0x29aa5b5b,// 36 PAY  33 

    0x9adce0df,// 37 PAY  34 

    0x7b23164e,// 38 PAY  35 

    0x29b2c197,// 39 PAY  36 

    0x440b8738,// 40 PAY  37 

    0xefe35d92,// 41 PAY  38 

    0x070a0dc2,// 42 PAY  39 

    0x28f7037d,// 43 PAY  40 

    0xc16c6f46,// 44 PAY  41 

    0x223e77cc,// 45 PAY  42 

    0x75e61377,// 46 PAY  43 

    0x83526180,// 47 PAY  44 

    0x6c5f9522,// 48 PAY  45 

    0xe43d8570,// 49 PAY  46 

    0xfc2e54ac,// 50 PAY  47 

    0x05eb2958,// 51 PAY  48 

    0x48bd5d1b,// 52 PAY  49 

    0x5d535383,// 53 PAY  50 

    0xbc6ef5f9,// 54 PAY  51 

    0x127a87f4,// 55 PAY  52 

    0x84b71489,// 56 PAY  53 

    0x5a8c4014,// 57 PAY  54 

    0xfcd82610,// 58 PAY  55 

    0xe0490d83,// 59 PAY  56 

    0x5e92d4e9,// 60 PAY  57 

    0x33d85f49,// 61 PAY  58 

    0x3eb7d090,// 62 PAY  59 

    0x4639dd16,// 63 PAY  60 

    0x88936c70,// 64 PAY  61 

    0xee910701,// 65 PAY  62 

    0x072573b7,// 66 PAY  63 

    0xbcd260e6,// 67 PAY  64 

    0x0f0a7edb,// 68 PAY  65 

    0xaeb255a4,// 69 PAY  66 

    0x096421ac,// 70 PAY  67 

    0x6029ddf1,// 71 PAY  68 

    0xea51b2c9,// 72 PAY  69 

    0x188ffd0c,// 73 PAY  70 

    0x9f872700,// 74 PAY  71 

    0x656b1a43,// 75 PAY  72 

    0xf0e5996a,// 76 PAY  73 

    0x60777283,// 77 PAY  74 

    0xfec90fed,// 78 PAY  75 

    0x46298609,// 79 PAY  76 

    0x4ad044f4,// 80 PAY  77 

    0x86f5f3eb,// 81 PAY  78 

    0xfe45a7dd,// 82 PAY  79 

    0x46f8b065,// 83 PAY  80 

    0xf92fb815,// 84 PAY  81 

    0x294abaa5,// 85 PAY  82 

    0x8bda9c79,// 86 PAY  83 

    0xe3093981,// 87 PAY  84 

    0x21e6df05,// 88 PAY  85 

    0x36d802f5,// 89 PAY  86 

    0x66336d66,// 90 PAY  87 

    0xc3f1d76b,// 91 PAY  88 

    0x3e033a26,// 92 PAY  89 

    0x30235c0a,// 93 PAY  90 

    0x8de052b0,// 94 PAY  91 

    0xf5839f74,// 95 PAY  92 

    0xecd3f9b9,// 96 PAY  93 

    0xf1107e72,// 97 PAY  94 

    0x976232ab,// 98 PAY  95 

    0xba1d88c4,// 99 PAY  96 

    0xfd28443f,// 100 PAY  97 

    0x83bb36c4,// 101 PAY  98 

    0x1cfef77d,// 102 PAY  99 

    0xcbe89a54,// 103 PAY 100 

    0x0a09502c,// 104 PAY 101 

    0xd8ffdec7,// 105 PAY 102 

    0x4db79eb5,// 106 PAY 103 

    0xfa3de919,// 107 PAY 104 

    0x827b6996,// 108 PAY 105 

    0xc4150682,// 109 PAY 106 

    0x0d667804,// 110 PAY 107 

    0xa0a223c3,// 111 PAY 108 

    0xa9398212,// 112 PAY 109 

    0x53e6edf1,// 113 PAY 110 

    0x5de90b3b,// 114 PAY 111 

    0x432aa431,// 115 PAY 112 

    0xf51ee4e7,// 116 PAY 113 

    0xee56c586,// 117 PAY 114 

    0x32944c50,// 118 PAY 115 

    0x098d224d,// 119 PAY 116 

    0x9eeac601,// 120 PAY 117 

    0x75df5ded,// 121 PAY 118 

    0x226395e6,// 122 PAY 119 

    0xcc5d9de2,// 123 PAY 120 

    0x62a35d2a,// 124 PAY 121 

    0x05f41491,// 125 PAY 122 

    0xe04cbe74,// 126 PAY 123 

    0x4de4b8dd,// 127 PAY 124 

    0x0714c3ae,// 128 PAY 125 

    0x680de08f,// 129 PAY 126 

    0xc7b71675,// 130 PAY 127 

    0xb4eb9e50,// 131 PAY 128 

    0x1dc5f156,// 132 PAY 129 

    0x4a30e5fb,// 133 PAY 130 

    0x5cfda35c,// 134 PAY 131 

    0xdb57dd90,// 135 PAY 132 

    0x001c4732,// 136 PAY 133 

    0x4830983d,// 137 PAY 134 

    0xa4adc3ec,// 138 PAY 135 

    0x4b5bb1d9,// 139 PAY 136 

    0x9d625f90,// 140 PAY 137 

    0x99b84994,// 141 PAY 138 

    0x83d32681,// 142 PAY 139 

    0x80b45022,// 143 PAY 140 

    0xc47346e1,// 144 PAY 141 

    0x46635f89,// 145 PAY 142 

    0xd2da831f,// 146 PAY 143 

    0x6dba17e8,// 147 PAY 144 

    0x6c2b0710,// 148 PAY 145 

    0xc0fe658b,// 149 PAY 146 

    0x34e1ca0d,// 150 PAY 147 

    0xf221e433,// 151 PAY 148 

    0xfb03d51b,// 152 PAY 149 

    0x51fc89ec,// 153 PAY 150 

    0x072aeb90,// 154 PAY 151 

    0xb039d5b3,// 155 PAY 152 

    0x954bfb8d,// 156 PAY 153 

    0xd28ec71f,// 157 PAY 154 

    0xf7ced11e,// 158 PAY 155 

    0xe7888ad2,// 159 PAY 156 

    0x9f717a06,// 160 PAY 157 

    0x3f512ec0,// 161 PAY 158 

    0xde421878,// 162 PAY 159 

    0x0dae5a24,// 163 PAY 160 

    0x50aadbeb,// 164 PAY 161 

    0xf2c28c49,// 165 PAY 162 

    0x9eeb9168,// 166 PAY 163 

    0xbf1f2f97,// 167 PAY 164 

    0xe8ef3e7b,// 168 PAY 165 

    0x31de36a6,// 169 PAY 166 

    0x8e54aa1e,// 170 PAY 167 

    0x436cafe8,// 171 PAY 168 

    0x866f5275,// 172 PAY 169 

    0xa6638c6d,// 173 PAY 170 

    0x2eb5966b,// 174 PAY 171 

    0x12e1e133,// 175 PAY 172 

    0x4280fc12,// 176 PAY 173 

    0x0edeff21,// 177 PAY 174 

    0xa7a5d113,// 178 PAY 175 

    0x4ee8403e,// 179 PAY 176 

    0x333d850b,// 180 PAY 177 

    0xa6865fcc,// 181 PAY 178 

    0xbd015f97,// 182 PAY 179 

    0xc7771289,// 183 PAY 180 

    0xe61caa8a,// 184 PAY 181 

    0x978b5e57,// 185 PAY 182 

    0x1825aa19,// 186 PAY 183 

    0x7a4240ef,// 187 PAY 184 

    0x85175acc,// 188 PAY 185 

    0x41ec6e22,// 189 PAY 186 

    0xbd478353,// 190 PAY 187 

    0x555b9847,// 191 PAY 188 

    0xfcd24d9d,// 192 PAY 189 

    0x3f063002,// 193 PAY 190 

    0xdce6d8df,// 194 PAY 191 

    0xdfdef041,// 195 PAY 192 

    0x66b5a405,// 196 PAY 193 

    0xa43c5b7f,// 197 PAY 194 

    0x64a1d32e,// 198 PAY 195 

    0xa6205a3d,// 199 PAY 196 

    0xfc156ff7,// 200 PAY 197 

    0xfbba5ed3,// 201 PAY 198 

    0x8d8990d1,// 202 PAY 199 

    0x7981e4a2,// 203 PAY 200 

    0x7b794d46,// 204 PAY 201 

    0x76f91242,// 205 PAY 202 

    0xcb9c4047,// 206 PAY 203 

    0x72b27e8b,// 207 PAY 204 

    0xfea91ef5,// 208 PAY 205 

    0x5f17248b,// 209 PAY 206 

    0x9bbe9685,// 210 PAY 207 

    0x62c33082,// 211 PAY 208 

    0x62ef5a91,// 212 PAY 209 

    0xd84f48be,// 213 PAY 210 

    0x18e47de3,// 214 PAY 211 

    0x6f344ee6,// 215 PAY 212 

    0x55866bfb,// 216 PAY 213 

    0x2fde7562,// 217 PAY 214 

    0xabcc650d,// 218 PAY 215 

    0xa6ced857,// 219 PAY 216 

    0x1998c7a6,// 220 PAY 217 

    0x26713642,// 221 PAY 218 

    0x6a1ee425,// 222 PAY 219 

    0x8790aa9b,// 223 PAY 220 

    0x668ec1ef,// 224 PAY 221 

    0x2407c442,// 225 PAY 222 

    0x4cd6bf1d,// 226 PAY 223 

    0xb1feb382,// 227 PAY 224 

    0x0689a26d,// 228 PAY 225 

    0xcee28126,// 229 PAY 226 

    0x0c220775,// 230 PAY 227 

    0x960025a1,// 231 PAY 228 

    0x777cc215,// 232 PAY 229 

    0x4392d529,// 233 PAY 230 

    0x458cf783,// 234 PAY 231 

    0xdf576686,// 235 PAY 232 

    0xa083b6df,// 236 PAY 233 

    0xc697b890,// 237 PAY 234 

    0x3d06e668,// 238 PAY 235 

    0x9ae8c3f5,// 239 PAY 236 

    0x9ba270d1,// 240 PAY 237 

    0xdf899e3b,// 241 PAY 238 

    0x3ec44feb,// 242 PAY 239 

    0x9f7ac911,// 243 PAY 240 

    0x9eb4adbe,// 244 PAY 241 

    0x3c9ab6a3,// 245 PAY 242 

    0x30ca22ac,// 246 PAY 243 

    0xd6800e92,// 247 PAY 244 

    0x82342157,// 248 PAY 245 

    0x6165bfb7,// 249 PAY 246 

    0xba9853e2,// 250 PAY 247 

    0xd8f52157,// 251 PAY 248 

    0x1e4c4739,// 252 PAY 249 

    0xe00c3977,// 253 PAY 250 

    0x4f258bca,// 254 PAY 251 

    0x81a2c903,// 255 PAY 252 

    0xf732555b,// 256 PAY 253 

    0x8cdfba8c,// 257 PAY 254 

    0x09a7860a,// 258 PAY 255 

    0x5f91da7e,// 259 PAY 256 

    0xd251ab73,// 260 PAY 257 

    0xcaa0afb4,// 261 PAY 258 

    0x030c5a8c,// 262 PAY 259 

    0x67282baf,// 263 PAY 260 

    0xcfcee906,// 264 PAY 261 

    0xfa06c812,// 265 PAY 262 

    0xd544df4f,// 266 PAY 263 

    0x1b11e7c8,// 267 PAY 264 

    0x049043e7,// 268 PAY 265 

    0x90659b1f,// 269 PAY 266 

    0xc86d77ec,// 270 PAY 267 

    0xb3d902a6,// 271 PAY 268 

    0x5fe9306d,// 272 PAY 269 

    0xf8be4645,// 273 PAY 270 

    0xfe91bde9,// 274 PAY 271 

    0xcd61119d,// 275 PAY 272 

    0x745a7732,// 276 PAY 273 

    0xbf05530e,// 277 PAY 274 

    0xb7c3d913,// 278 PAY 275 

    0x134e985c,// 279 PAY 276 

    0xdfa05203,// 280 PAY 277 

    0x9ef2b571,// 281 PAY 278 

    0xa50376a4,// 282 PAY 279 

    0x17a849e3,// 283 PAY 280 

    0xe586b097,// 284 PAY 281 

    0xb3e520a1,// 285 PAY 282 

    0x541371a9,// 286 PAY 283 

    0x2f0e5dbf,// 287 PAY 284 

    0xeb118354,// 288 PAY 285 

    0xdb508cf9,// 289 PAY 286 

    0x7a3e6e41,// 290 PAY 287 

    0xb866d4e1,// 291 PAY 288 

    0x70b0178e,// 292 PAY 289 

    0xa8cd41b3,// 293 PAY 290 

    0xa3a48ec3,// 294 PAY 291 

    0x1b5bb460,// 295 PAY 292 

    0xa486dbe3,// 296 PAY 293 

    0xabeb8e01,// 297 PAY 294 

    0x4cdf9214,// 298 PAY 295 

    0x7cb431dd,// 299 PAY 296 

    0x6ac5d416,// 300 PAY 297 

    0x20da8a32,// 301 PAY 298 

    0x02686682,// 302 PAY 299 

    0x606a2f9a,// 303 PAY 300 

    0x86a02e0a,// 304 PAY 301 

    0xee870612,// 305 PAY 302 

    0x5246807b,// 306 PAY 303 

    0x24a466fe,// 307 PAY 304 

    0xf024f477,// 308 PAY 305 

    0x4c5883b5,// 309 PAY 306 

    0xdcb50d9b,// 310 PAY 307 

    0xd9d9f710,// 311 PAY 308 

    0x9d3743f4,// 312 PAY 309 

    0xc4f146e0,// 313 PAY 310 

    0x42fd4d48,// 314 PAY 311 

    0x18132349,// 315 PAY 312 

    0x2d1c7c38,// 316 PAY 313 

    0x7d2b0275,// 317 PAY 314 

    0xc6695f78,// 318 PAY 315 

    0x7574659e,// 319 PAY 316 

    0x5b134fb5,// 320 PAY 317 

    0xa5075a6d,// 321 PAY 318 

    0xa792bb8a,// 322 PAY 319 

    0xeeea73ef,// 323 PAY 320 

    0x8c8912d3,// 324 PAY 321 

    0x99f54b97,// 325 PAY 322 

    0x88b69221,// 326 PAY 323 

    0x79641c30,// 327 PAY 324 

    0xb3cdd429,// 328 PAY 325 

    0xed8b29e6,// 329 PAY 326 

    0x4db0ad21,// 330 PAY 327 

    0xb8715711,// 331 PAY 328 

    0xf47e82da,// 332 PAY 329 

    0x0b5fab40,// 333 PAY 330 

    0xf67a1dc9,// 334 PAY 331 

    0xa2f2410d,// 335 PAY 332 

    0xd1107106,// 336 PAY 333 

    0xaed9b472,// 337 PAY 334 

    0x6d2d03c4,// 338 PAY 335 

    0x7673c9cc,// 339 PAY 336 

    0x59a4a773,// 340 PAY 337 

    0x192f5400,// 341 PAY 338 

    0xb29f268c,// 342 PAY 339 

    0x61e02264,// 343 PAY 340 

    0x6b59c10b,// 344 PAY 341 

    0x2b64bcd5,// 345 PAY 342 

    0xb53040d0,// 346 PAY 343 

    0x24af82ef,// 347 PAY 344 

    0x952c8768,// 348 PAY 345 

    0x16a51b50,// 349 PAY 346 

    0x35b8b633,// 350 PAY 347 

    0xf1008257,// 351 PAY 348 

    0x50db5c67,// 352 PAY 349 

    0xc9e0c6b5,// 353 PAY 350 

    0x8baee61e,// 354 PAY 351 

    0x10d6e425,// 355 PAY 352 

    0x250408c9,// 356 PAY 353 

    0xb4519f9b,// 357 PAY 354 

    0x0444cbd0,// 358 PAY 355 

    0xca6e8bae,// 359 PAY 356 

    0x36a819d6,// 360 PAY 357 

    0x103b825d,// 361 PAY 358 

    0x52e67777,// 362 PAY 359 

    0x71320373,// 363 PAY 360 

    0x717af12e,// 364 PAY 361 

    0xa05fd676,// 365 PAY 362 

    0xc5a6933b,// 366 PAY 363 

    0xff6f372a,// 367 PAY 364 

    0x5f964e25,// 368 PAY 365 

    0x07a826b7,// 369 PAY 366 

    0xd0e5007b,// 370 PAY 367 

    0xf4b08806,// 371 PAY 368 

    0x09841fe2,// 372 PAY 369 

    0x34c2b54c,// 373 PAY 370 

    0x82184817,// 374 PAY 371 

    0xa3f34d7c,// 375 PAY 372 

    0xf9d7d19b,// 376 PAY 373 

    0x505a4e08,// 377 PAY 374 

    0x3226c7ff,// 378 PAY 375 

    0xbaee427d,// 379 PAY 376 

    0x9d42a785,// 380 PAY 377 

    0xaa3acbf9,// 381 PAY 378 

    0x19bd6c36,// 382 PAY 379 

    0x3f178399,// 383 PAY 380 

    0x64b02804,// 384 PAY 381 

    0x369646fa,// 385 PAY 382 

    0x283c81ea,// 386 PAY 383 

    0x01ac82c9,// 387 PAY 384 

    0xcf23ba01,// 388 PAY 385 

    0x80a019cc,// 389 PAY 386 

    0x07765efb,// 390 PAY 387 

    0xbb592bad,// 391 PAY 388 

    0x6b741b76,// 392 PAY 389 

    0x35959b0f,// 393 PAY 390 

    0xbe3742b9,// 394 PAY 391 

    0x201b2b1d,// 395 PAY 392 

    0x911ddc52,// 396 PAY 393 

    0x9861aa06,// 397 PAY 394 

    0xe1b8f24c,// 398 PAY 395 

    0x060a6719,// 399 PAY 396 

    0x698aff40,// 400 PAY 397 

    0x71079dd8,// 401 PAY 398 

    0x909d4462,// 402 PAY 399 

    0x07bf8d45,// 403 PAY 400 

    0x6eb2d96a,// 404 PAY 401 

    0x0d2ef706,// 405 PAY 402 

    0x39f247c6,// 406 PAY 403 

    0x20391992,// 407 PAY 404 

    0x439db259,// 408 PAY 405 

    0x3ef2072a,// 409 PAY 406 

    0xfc7a8eb6,// 410 PAY 407 

    0xf6f6bde3,// 411 PAY 408 

    0x4487e3d7,// 412 PAY 409 

    0x78adaadc,// 413 PAY 410 

    0x41826420,// 414 PAY 411 

    0x8ec0e628,// 415 PAY 412 

    0x8f63752b,// 416 PAY 413 

    0x92bd27a8,// 417 PAY 414 

    0x3344c50c,// 418 PAY 415 

    0xea09a8b1,// 419 PAY 416 

    0xf0a57bc8,// 420 PAY 417 

    0x93d65ccd,// 421 PAY 418 

    0x4f54ddfc,// 422 PAY 419 

    0x1939270d,// 423 PAY 420 

    0xf9f131cc,// 424 PAY 421 

    0x05ecee88,// 425 PAY 422 

    0x7989dc36,// 426 PAY 423 

    0xab628662,// 427 PAY 424 

    0xec031dfa,// 428 PAY 425 

    0xc0ae5310,// 429 PAY 426 

    0x3fd11c2d,// 430 PAY 427 

    0x57d6b5bc,// 431 PAY 428 

    0x72e9be4e,// 432 PAY 429 

    0x0b9fb90f,// 433 PAY 430 

    0x15505422,// 434 PAY 431 

    0x98bb1c5a,// 435 PAY 432 

    0x004f18c1,// 436 PAY 433 

    0x4d7ac763,// 437 PAY 434 

    0x83a66c5d,// 438 PAY 435 

    0x846dd6cd,// 439 PAY 436 

    0x5e2bf52a,// 440 PAY 437 

    0x36a9605e,// 441 PAY 438 

    0x185f79db,// 442 PAY 439 

    0x21eb3dda,// 443 PAY 440 

    0xb6efc084,// 444 PAY 441 

    0x3ce00000,// 445 PAY 442 

/// HASH is  12 bytes 

    0xb29f268c,// 446 HSH   1 

    0x61e02264,// 447 HSH   2 

    0x6b59c10b,// 448 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 171 

/// STA pkt_idx        : 56 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc7 

    0x00e0c7ab // 449 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt91_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x05 

/// ECH pdu_tag        : 0x00 

    0x00050000,// 2 ECH   1 

/// BDA is 277 words. 

/// BDA size     is 1104 (0x450) 

/// BDA id       is 0xd7b7 

    0x0450d7b7,// 3 BDA   1 

/// PAY Generic Data size   : 1104 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xe5310521,// 4 PAY   1 

    0x624c40f7,// 5 PAY   2 

    0x10041a91,// 6 PAY   3 

    0x77028ce4,// 7 PAY   4 

    0x2a727b83,// 8 PAY   5 

    0xc0de5091,// 9 PAY   6 

    0xc6e6142d,// 10 PAY   7 

    0x92cbd613,// 11 PAY   8 

    0x40e93ceb,// 12 PAY   9 

    0xe66d7c3c,// 13 PAY  10 

    0x696f5015,// 14 PAY  11 

    0xa777acfd,// 15 PAY  12 

    0x305f5423,// 16 PAY  13 

    0x72dba0a8,// 17 PAY  14 

    0xca136324,// 18 PAY  15 

    0x45dfe221,// 19 PAY  16 

    0x2179da4d,// 20 PAY  17 

    0x9b93234c,// 21 PAY  18 

    0xd80ab31a,// 22 PAY  19 

    0x4e57ef82,// 23 PAY  20 

    0xd759b750,// 24 PAY  21 

    0x6ca5f148,// 25 PAY  22 

    0x8cd545ab,// 26 PAY  23 

    0xd8a71571,// 27 PAY  24 

    0x9346222b,// 28 PAY  25 

    0xf00e255e,// 29 PAY  26 

    0xa1e37f65,// 30 PAY  27 

    0x22b4d0ba,// 31 PAY  28 

    0x28c7ae81,// 32 PAY  29 

    0x167d26c9,// 33 PAY  30 

    0xbd9e1a2f,// 34 PAY  31 

    0xf1d0d34d,// 35 PAY  32 

    0x662b6fa2,// 36 PAY  33 

    0xbcd1cdd8,// 37 PAY  34 

    0xdaf1c6f7,// 38 PAY  35 

    0x08f510de,// 39 PAY  36 

    0xa01ffa8d,// 40 PAY  37 

    0xd99a37d6,// 41 PAY  38 

    0xae9ba372,// 42 PAY  39 

    0x6dfae776,// 43 PAY  40 

    0xd9e8a55a,// 44 PAY  41 

    0x182c8251,// 45 PAY  42 

    0xf8bcea07,// 46 PAY  43 

    0x3ae0cf46,// 47 PAY  44 

    0x77865e22,// 48 PAY  45 

    0x447e2453,// 49 PAY  46 

    0x6fe4a429,// 50 PAY  47 

    0x381ce742,// 51 PAY  48 

    0x0b7168ca,// 52 PAY  49 

    0xb541877b,// 53 PAY  50 

    0x0940af3e,// 54 PAY  51 

    0x2939e317,// 55 PAY  52 

    0x1d9fef0f,// 56 PAY  53 

    0x495444aa,// 57 PAY  54 

    0xfa61d738,// 58 PAY  55 

    0x3611e4ea,// 59 PAY  56 

    0xe968f6e6,// 60 PAY  57 

    0x5d1bce43,// 61 PAY  58 

    0x65b80752,// 62 PAY  59 

    0x59ba0d9e,// 63 PAY  60 

    0x3a028563,// 64 PAY  61 

    0x2755b6fc,// 65 PAY  62 

    0xa882ba83,// 66 PAY  63 

    0x4f8198d8,// 67 PAY  64 

    0xc1d7eb86,// 68 PAY  65 

    0x962fb7c8,// 69 PAY  66 

    0x68ba4240,// 70 PAY  67 

    0x15af9ef5,// 71 PAY  68 

    0x7be33a30,// 72 PAY  69 

    0xefcf03a3,// 73 PAY  70 

    0x6867d7c6,// 74 PAY  71 

    0x77b1618a,// 75 PAY  72 

    0xd9ef9084,// 76 PAY  73 

    0x9c0d6624,// 77 PAY  74 

    0x4ac6e973,// 78 PAY  75 

    0x26069983,// 79 PAY  76 

    0xb7222a7b,// 80 PAY  77 

    0x5f811aab,// 81 PAY  78 

    0x36c148ef,// 82 PAY  79 

    0x43746c41,// 83 PAY  80 

    0x0bc95d78,// 84 PAY  81 

    0x5159b6fc,// 85 PAY  82 

    0xaeec2874,// 86 PAY  83 

    0xd6d29388,// 87 PAY  84 

    0xafae9c06,// 88 PAY  85 

    0xf89b1436,// 89 PAY  86 

    0x041e5f32,// 90 PAY  87 

    0x35466db2,// 91 PAY  88 

    0x6ffe7a7d,// 92 PAY  89 

    0x1142573a,// 93 PAY  90 

    0xd50155c4,// 94 PAY  91 

    0x8c3e90b2,// 95 PAY  92 

    0x01f0d71c,// 96 PAY  93 

    0xed08df57,// 97 PAY  94 

    0x097bdf03,// 98 PAY  95 

    0xc75cc495,// 99 PAY  96 

    0x82274e7b,// 100 PAY  97 

    0x0b24d159,// 101 PAY  98 

    0x1df92b50,// 102 PAY  99 

    0xcc85d04a,// 103 PAY 100 

    0x8dee6f55,// 104 PAY 101 

    0x1bfc312c,// 105 PAY 102 

    0x22a25f64,// 106 PAY 103 

    0x87f7a95c,// 107 PAY 104 

    0x2b27d6a0,// 108 PAY 105 

    0x90d33003,// 109 PAY 106 

    0x36e948cc,// 110 PAY 107 

    0xc1d4cdc9,// 111 PAY 108 

    0xcc4a3d07,// 112 PAY 109 

    0xa8bf38ce,// 113 PAY 110 

    0x78f9fdbd,// 114 PAY 111 

    0xe2c508cc,// 115 PAY 112 

    0x39b915a7,// 116 PAY 113 

    0x84fe52e2,// 117 PAY 114 

    0x41d1fe25,// 118 PAY 115 

    0xcf21106b,// 119 PAY 116 

    0xf8ff9545,// 120 PAY 117 

    0xfe67c10e,// 121 PAY 118 

    0x30104621,// 122 PAY 119 

    0x59386970,// 123 PAY 120 

    0x656f2276,// 124 PAY 121 

    0x2de594f4,// 125 PAY 122 

    0xf58672ae,// 126 PAY 123 

    0x24b9f459,// 127 PAY 124 

    0x872a7d31,// 128 PAY 125 

    0x5f923ecf,// 129 PAY 126 

    0xf6588b1e,// 130 PAY 127 

    0xd2772c9f,// 131 PAY 128 

    0x28bf6722,// 132 PAY 129 

    0x4d702ba5,// 133 PAY 130 

    0x3ea84fa7,// 134 PAY 131 

    0xb2fe2370,// 135 PAY 132 

    0x65681f11,// 136 PAY 133 

    0x5f8fd821,// 137 PAY 134 

    0x31412969,// 138 PAY 135 

    0x70e7bb26,// 139 PAY 136 

    0xb2735c7d,// 140 PAY 137 

    0xccd3d707,// 141 PAY 138 

    0x450564bb,// 142 PAY 139 

    0x8260fb7b,// 143 PAY 140 

    0x61a56d18,// 144 PAY 141 

    0xe71c4bed,// 145 PAY 142 

    0x136e65fe,// 146 PAY 143 

    0x6e899c38,// 147 PAY 144 

    0xb614aea9,// 148 PAY 145 

    0x1b831b4e,// 149 PAY 146 

    0x8354ca56,// 150 PAY 147 

    0x2a9fb5ac,// 151 PAY 148 

    0xaba58cdf,// 152 PAY 149 

    0xd1950cb4,// 153 PAY 150 

    0x9b9f7550,// 154 PAY 151 

    0x6a672fa0,// 155 PAY 152 

    0xc03e6fdb,// 156 PAY 153 

    0x5d15ec14,// 157 PAY 154 

    0xb5e3adfc,// 158 PAY 155 

    0x8646f8d7,// 159 PAY 156 

    0x2be61fb6,// 160 PAY 157 

    0x04a6bd09,// 161 PAY 158 

    0x93778570,// 162 PAY 159 

    0x476f9e41,// 163 PAY 160 

    0xa4be1f24,// 164 PAY 161 

    0x56b66bef,// 165 PAY 162 

    0x465d3902,// 166 PAY 163 

    0xe3af146a,// 167 PAY 164 

    0xc188a232,// 168 PAY 165 

    0x8085cc71,// 169 PAY 166 

    0x7c828e8f,// 170 PAY 167 

    0xf33acc2b,// 171 PAY 168 

    0xe9594aa7,// 172 PAY 169 

    0x8862aa65,// 173 PAY 170 

    0xd0c74dd3,// 174 PAY 171 

    0x835914d0,// 175 PAY 172 

    0x19f458da,// 176 PAY 173 

    0x49dbb249,// 177 PAY 174 

    0x3d5533ac,// 178 PAY 175 

    0xfd614314,// 179 PAY 176 

    0xa7aae507,// 180 PAY 177 

    0x8c3a6b20,// 181 PAY 178 

    0x6c87c438,// 182 PAY 179 

    0xeab4814f,// 183 PAY 180 

    0x7d5b7bed,// 184 PAY 181 

    0xd1277d44,// 185 PAY 182 

    0x491612e8,// 186 PAY 183 

    0x8596e8c4,// 187 PAY 184 

    0x4fefa29f,// 188 PAY 185 

    0xe76015a7,// 189 PAY 186 

    0xa8388b13,// 190 PAY 187 

    0xda43c32e,// 191 PAY 188 

    0xed36a573,// 192 PAY 189 

    0x687f1528,// 193 PAY 190 

    0x2701f753,// 194 PAY 191 

    0x8272620c,// 195 PAY 192 

    0x838c4dab,// 196 PAY 193 

    0xcb746848,// 197 PAY 194 

    0x7a7ed674,// 198 PAY 195 

    0xf18b9b54,// 199 PAY 196 

    0x17543ce7,// 200 PAY 197 

    0x92bf64b2,// 201 PAY 198 

    0xb28b93fb,// 202 PAY 199 

    0xdd5e4181,// 203 PAY 200 

    0xa0f16d79,// 204 PAY 201 

    0xfea7c9c2,// 205 PAY 202 

    0x104daaf0,// 206 PAY 203 

    0x0fbe37d1,// 207 PAY 204 

    0x2b04db93,// 208 PAY 205 

    0x56b117e4,// 209 PAY 206 

    0x48857033,// 210 PAY 207 

    0xba95bb85,// 211 PAY 208 

    0xe48a754b,// 212 PAY 209 

    0x85ecc591,// 213 PAY 210 

    0x72955949,// 214 PAY 211 

    0x2a6003c8,// 215 PAY 212 

    0xf604b653,// 216 PAY 213 

    0x4c7f8b7b,// 217 PAY 214 

    0x1b4553e0,// 218 PAY 215 

    0x09e95966,// 219 PAY 216 

    0x784d5c8a,// 220 PAY 217 

    0x3ffa160d,// 221 PAY 218 

    0x4256be04,// 222 PAY 219 

    0xbb2c729a,// 223 PAY 220 

    0xb0a9e192,// 224 PAY 221 

    0x1b9b9555,// 225 PAY 222 

    0xb97c3a23,// 226 PAY 223 

    0x6f5cba65,// 227 PAY 224 

    0xdbe18169,// 228 PAY 225 

    0xd24e2fa5,// 229 PAY 226 

    0x7f279210,// 230 PAY 227 

    0x83a1f24c,// 231 PAY 228 

    0xc514afe7,// 232 PAY 229 

    0x67de3fe1,// 233 PAY 230 

    0x246844d4,// 234 PAY 231 

    0x6a4e9539,// 235 PAY 232 

    0xad57a8df,// 236 PAY 233 

    0x6fe2aa44,// 237 PAY 234 

    0x3d5ef42b,// 238 PAY 235 

    0x81bfd0e2,// 239 PAY 236 

    0xe92176aa,// 240 PAY 237 

    0xc30ddfa4,// 241 PAY 238 

    0x3ed0868d,// 242 PAY 239 

    0xd017cdab,// 243 PAY 240 

    0x075fbbf3,// 244 PAY 241 

    0x9e21670b,// 245 PAY 242 

    0x99a7aaad,// 246 PAY 243 

    0x3fe72425,// 247 PAY 244 

    0x86a44b1a,// 248 PAY 245 

    0xec65ec04,// 249 PAY 246 

    0xbdd2e6ac,// 250 PAY 247 

    0x8d3a8937,// 251 PAY 248 

    0x08a4b027,// 252 PAY 249 

    0x597377e6,// 253 PAY 250 

    0x5de9aca9,// 254 PAY 251 

    0xdf8a78e5,// 255 PAY 252 

    0x7576cdc8,// 256 PAY 253 

    0x9b9577ce,// 257 PAY 254 

    0x468ea220,// 258 PAY 255 

    0x9ea70769,// 259 PAY 256 

    0xdfba29bc,// 260 PAY 257 

    0xfbfb2d6b,// 261 PAY 258 

    0x9b181a56,// 262 PAY 259 

    0x8153b182,// 263 PAY 260 

    0x47322e48,// 264 PAY 261 

    0x033c1420,// 265 PAY 262 

    0x7de9b50d,// 266 PAY 263 

    0x3cdec7c4,// 267 PAY 264 

    0x1c183dcd,// 268 PAY 265 

    0xd00b1494,// 269 PAY 266 

    0xb88f6355,// 270 PAY 267 

    0x6c9ea901,// 271 PAY 268 

    0x1af58475,// 272 PAY 269 

    0xe9ea0d96,// 273 PAY 270 

    0x85e4d52d,// 274 PAY 271 

    0xaf864356,// 275 PAY 272 

    0x27d48e74,// 276 PAY 273 

    0xaa79695e,// 277 PAY 274 

    0x45c8ef14,// 278 PAY 275 

    0xb3182869,// 279 PAY 276 

/// STA is 1 words. 

/// STA num_pkts       : 249 

/// STA pkt_idx        : 166 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x4 

    0x029904f9 // 280 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt92_tmpl[] = {
    0x0c010040,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 476 words. 

/// BDA size     is 1898 (0x76a) 

/// BDA id       is 0xb198 

    0x076ab198,// 3 BDA   1 

/// PAY Generic Data size   : 1898 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0x84a4c125,// 4 PAY   1 

    0xd73c6eff,// 5 PAY   2 

    0x0805708f,// 6 PAY   3 

    0x9cbb01fd,// 7 PAY   4 

    0xd04668c2,// 8 PAY   5 

    0x9e4ccd52,// 9 PAY   6 

    0x5aa1fc36,// 10 PAY   7 

    0xf00d14e4,// 11 PAY   8 

    0x2f9d1a21,// 12 PAY   9 

    0x3584c80c,// 13 PAY  10 

    0x5af466fb,// 14 PAY  11 

    0xb0255735,// 15 PAY  12 

    0xd86126c5,// 16 PAY  13 

    0x275c067c,// 17 PAY  14 

    0x13cb26af,// 18 PAY  15 

    0x93600a1e,// 19 PAY  16 

    0xbdacd6cf,// 20 PAY  17 

    0xe4eabf87,// 21 PAY  18 

    0x8e75eeb2,// 22 PAY  19 

    0xdc006761,// 23 PAY  20 

    0x82d6d956,// 24 PAY  21 

    0x4310688d,// 25 PAY  22 

    0xc535ad7d,// 26 PAY  23 

    0xea79777a,// 27 PAY  24 

    0xd707fdbc,// 28 PAY  25 

    0x16dd2f20,// 29 PAY  26 

    0x4ab82cec,// 30 PAY  27 

    0x95b24b2c,// 31 PAY  28 

    0x32775619,// 32 PAY  29 

    0x1341c4ea,// 33 PAY  30 

    0x2335094d,// 34 PAY  31 

    0x42719a29,// 35 PAY  32 

    0x95539835,// 36 PAY  33 

    0x7a2aba34,// 37 PAY  34 

    0x5978055b,// 38 PAY  35 

    0xd1279f72,// 39 PAY  36 

    0xb484e616,// 40 PAY  37 

    0x51476bb9,// 41 PAY  38 

    0x343fa6c6,// 42 PAY  39 

    0x4db4ba54,// 43 PAY  40 

    0x40ca9261,// 44 PAY  41 

    0xf3216826,// 45 PAY  42 

    0xf3ed76a7,// 46 PAY  43 

    0xd188e500,// 47 PAY  44 

    0x760b7270,// 48 PAY  45 

    0x915cdbf3,// 49 PAY  46 

    0x75e8f53b,// 50 PAY  47 

    0x00585024,// 51 PAY  48 

    0xb4892e47,// 52 PAY  49 

    0xfdbc506f,// 53 PAY  50 

    0xfd8376a3,// 54 PAY  51 

    0x18ee517d,// 55 PAY  52 

    0x18a6bfbb,// 56 PAY  53 

    0x92374e7f,// 57 PAY  54 

    0x93cb0672,// 58 PAY  55 

    0xd332c8b9,// 59 PAY  56 

    0x46dffff6,// 60 PAY  57 

    0xaaf37b54,// 61 PAY  58 

    0xdb2098fe,// 62 PAY  59 

    0x962646b7,// 63 PAY  60 

    0x89b0803f,// 64 PAY  61 

    0x2d1fd052,// 65 PAY  62 

    0xffe846a5,// 66 PAY  63 

    0xa0acff64,// 67 PAY  64 

    0xf6170c52,// 68 PAY  65 

    0x9373f5c7,// 69 PAY  66 

    0xc6b76d32,// 70 PAY  67 

    0x63e0d2c2,// 71 PAY  68 

    0xa403d837,// 72 PAY  69 

    0x7f21af62,// 73 PAY  70 

    0xe025dea7,// 74 PAY  71 

    0x08ffe4a6,// 75 PAY  72 

    0x65a2b66b,// 76 PAY  73 

    0x9df21a90,// 77 PAY  74 

    0x54e53dcc,// 78 PAY  75 

    0xdd1e252a,// 79 PAY  76 

    0x8948206c,// 80 PAY  77 

    0xacb96ae9,// 81 PAY  78 

    0x6d0d08ee,// 82 PAY  79 

    0x919989a4,// 83 PAY  80 

    0x08ffb563,// 84 PAY  81 

    0x387d5a32,// 85 PAY  82 

    0xa29870e2,// 86 PAY  83 

    0x9e85211b,// 87 PAY  84 

    0xbea1cff9,// 88 PAY  85 

    0x62607958,// 89 PAY  86 

    0x6c1deaf3,// 90 PAY  87 

    0x609e2d75,// 91 PAY  88 

    0x89220490,// 92 PAY  89 

    0x1e67e8b0,// 93 PAY  90 

    0x47990be5,// 94 PAY  91 

    0x2019b11d,// 95 PAY  92 

    0x5de728bc,// 96 PAY  93 

    0xf53da19d,// 97 PAY  94 

    0xac8a5c7c,// 98 PAY  95 

    0x683bcc0a,// 99 PAY  96 

    0x9b66534d,// 100 PAY  97 

    0xd5928fb0,// 101 PAY  98 

    0x998578bc,// 102 PAY  99 

    0x1351874b,// 103 PAY 100 

    0xd3d2b4f7,// 104 PAY 101 

    0x1ddff52a,// 105 PAY 102 

    0x0cb8f65c,// 106 PAY 103 

    0xbf432ca2,// 107 PAY 104 

    0x6ea05cdc,// 108 PAY 105 

    0x7a136d13,// 109 PAY 106 

    0xe1d205ec,// 110 PAY 107 

    0x8546ad41,// 111 PAY 108 

    0xa08a32ff,// 112 PAY 109 

    0xbc3c732e,// 113 PAY 110 

    0x28a8b0ee,// 114 PAY 111 

    0x3bd317ea,// 115 PAY 112 

    0xb0eb77c4,// 116 PAY 113 

    0xae1a43e4,// 117 PAY 114 

    0x24ff4808,// 118 PAY 115 

    0xc27f26ae,// 119 PAY 116 

    0x15301bff,// 120 PAY 117 

    0x00e95419,// 121 PAY 118 

    0xc93fc36f,// 122 PAY 119 

    0x77f31ed5,// 123 PAY 120 

    0x0e28c178,// 124 PAY 121 

    0xa9162b79,// 125 PAY 122 

    0x8cb02a59,// 126 PAY 123 

    0xfe3207e8,// 127 PAY 124 

    0x5ca4bd6b,// 128 PAY 125 

    0x961c8586,// 129 PAY 126 

    0x71b79b73,// 130 PAY 127 

    0x3bbf13bb,// 131 PAY 128 

    0x42208781,// 132 PAY 129 

    0xf7f5e3e7,// 133 PAY 130 

    0x8fc341a4,// 134 PAY 131 

    0x5cac37fc,// 135 PAY 132 

    0xc0bbbefa,// 136 PAY 133 

    0xe919c76d,// 137 PAY 134 

    0x11207feb,// 138 PAY 135 

    0x5910a057,// 139 PAY 136 

    0xfa6ad9db,// 140 PAY 137 

    0x4967466f,// 141 PAY 138 

    0xc5b600f8,// 142 PAY 139 

    0x7188df1c,// 143 PAY 140 

    0x7874078c,// 144 PAY 141 

    0xfe38bc37,// 145 PAY 142 

    0x23dc3202,// 146 PAY 143 

    0xa134a467,// 147 PAY 144 

    0xe60e1666,// 148 PAY 145 

    0x2ce0880f,// 149 PAY 146 

    0x0d27f457,// 150 PAY 147 

    0x40962efa,// 151 PAY 148 

    0xbc8ec963,// 152 PAY 149 

    0x1fcea223,// 153 PAY 150 

    0x54eee6a5,// 154 PAY 151 

    0xab66e803,// 155 PAY 152 

    0x5e04c2da,// 156 PAY 153 

    0xd513dcc9,// 157 PAY 154 

    0xc7d7390b,// 158 PAY 155 

    0xe9e8d5f7,// 159 PAY 156 

    0xfb869f1d,// 160 PAY 157 

    0xe1ba0e33,// 161 PAY 158 

    0x666764d7,// 162 PAY 159 

    0xd777311c,// 163 PAY 160 

    0x275e4725,// 164 PAY 161 

    0x140db61c,// 165 PAY 162 

    0x825bb07f,// 166 PAY 163 

    0x67655c5d,// 167 PAY 164 

    0x12d1df71,// 168 PAY 165 

    0x229cd539,// 169 PAY 166 

    0xeb785346,// 170 PAY 167 

    0xde824f31,// 171 PAY 168 

    0xb1ae9ad3,// 172 PAY 169 

    0x90866d29,// 173 PAY 170 

    0xd56d3c42,// 174 PAY 171 

    0xd6b8ff81,// 175 PAY 172 

    0x7f03bcd0,// 176 PAY 173 

    0x2adea483,// 177 PAY 174 

    0x670683be,// 178 PAY 175 

    0x495768d7,// 179 PAY 176 

    0xc4898977,// 180 PAY 177 

    0x20c771a6,// 181 PAY 178 

    0xc202cdf8,// 182 PAY 179 

    0xf11499c7,// 183 PAY 180 

    0xad0bce5b,// 184 PAY 181 

    0x5e0947f7,// 185 PAY 182 

    0x39fb53e8,// 186 PAY 183 

    0x7bae9a44,// 187 PAY 184 

    0x3b926905,// 188 PAY 185 

    0x4b6a60bc,// 189 PAY 186 

    0xf2f4a3ae,// 190 PAY 187 

    0x670e861f,// 191 PAY 188 

    0xfa180517,// 192 PAY 189 

    0x1441a7da,// 193 PAY 190 

    0x5dd4463b,// 194 PAY 191 

    0xb83a7a1e,// 195 PAY 192 

    0xcfc4e10c,// 196 PAY 193 

    0xceddcbbd,// 197 PAY 194 

    0xdd1c2ec9,// 198 PAY 195 

    0x65037d37,// 199 PAY 196 

    0xd89a02db,// 200 PAY 197 

    0x952709be,// 201 PAY 198 

    0xc862f27d,// 202 PAY 199 

    0xde39cee4,// 203 PAY 200 

    0xa2ea42be,// 204 PAY 201 

    0x00bfe31c,// 205 PAY 202 

    0xc77a9ca4,// 206 PAY 203 

    0xf8e7aaaa,// 207 PAY 204 

    0xe9624d61,// 208 PAY 205 

    0xb5344075,// 209 PAY 206 

    0xe620acdd,// 210 PAY 207 

    0x294e1973,// 211 PAY 208 

    0x6fbc18b0,// 212 PAY 209 

    0x0c575c69,// 213 PAY 210 

    0x50af2bbd,// 214 PAY 211 

    0x686f7662,// 215 PAY 212 

    0x13abebac,// 216 PAY 213 

    0x90ffee44,// 217 PAY 214 

    0x48b35e1d,// 218 PAY 215 

    0x9bdba551,// 219 PAY 216 

    0xf32789f2,// 220 PAY 217 

    0x9770714f,// 221 PAY 218 

    0x676c62bf,// 222 PAY 219 

    0xf5ceed0b,// 223 PAY 220 

    0xa87c0068,// 224 PAY 221 

    0xdba66473,// 225 PAY 222 

    0x23de8549,// 226 PAY 223 

    0x8c5146f5,// 227 PAY 224 

    0x30524438,// 228 PAY 225 

    0x4de018ce,// 229 PAY 226 

    0x1e3700e3,// 230 PAY 227 

    0xa894397c,// 231 PAY 228 

    0x00ab1069,// 232 PAY 229 

    0x8ce6ccfe,// 233 PAY 230 

    0x6a0a52bb,// 234 PAY 231 

    0xcf22a6c7,// 235 PAY 232 

    0xef3c179a,// 236 PAY 233 

    0xe5f4c4a5,// 237 PAY 234 

    0x47b15328,// 238 PAY 235 

    0x4efad821,// 239 PAY 236 

    0x85f4fa1a,// 240 PAY 237 

    0x90f54ef4,// 241 PAY 238 

    0xa7d5634b,// 242 PAY 239 

    0xaf8634c6,// 243 PAY 240 

    0x6cb99361,// 244 PAY 241 

    0xfe8e93a2,// 245 PAY 242 

    0xc4bad6a7,// 246 PAY 243 

    0xf0e7ecb9,// 247 PAY 244 

    0x936cdec5,// 248 PAY 245 

    0x21bb0942,// 249 PAY 246 

    0x127ac27d,// 250 PAY 247 

    0x02e742ab,// 251 PAY 248 

    0x751b1dc7,// 252 PAY 249 

    0x104af223,// 253 PAY 250 

    0xb4af4b01,// 254 PAY 251 

    0x6ecff8be,// 255 PAY 252 

    0x814cb1e1,// 256 PAY 253 

    0x4b97e4b7,// 257 PAY 254 

    0xe4f16065,// 258 PAY 255 

    0xa2255da1,// 259 PAY 256 

    0x40b7c567,// 260 PAY 257 

    0x100efd33,// 261 PAY 258 

    0x355502c9,// 262 PAY 259 

    0x78e42432,// 263 PAY 260 

    0xd3ef793d,// 264 PAY 261 

    0xef0a9ac4,// 265 PAY 262 

    0xaeafca50,// 266 PAY 263 

    0x38de0ebe,// 267 PAY 264 

    0xf67490a9,// 268 PAY 265 

    0xc4c1c577,// 269 PAY 266 

    0xc7d65f1e,// 270 PAY 267 

    0xbf34227a,// 271 PAY 268 

    0xb3ddbd45,// 272 PAY 269 

    0xe38f97cd,// 273 PAY 270 

    0x2c34480e,// 274 PAY 271 

    0xcf8f3eef,// 275 PAY 272 

    0xdf139fc2,// 276 PAY 273 

    0xc1545351,// 277 PAY 274 

    0x393faff8,// 278 PAY 275 

    0x49c5571d,// 279 PAY 276 

    0xaa4399ff,// 280 PAY 277 

    0x500facbb,// 281 PAY 278 

    0xdf476180,// 282 PAY 279 

    0xe64eeba3,// 283 PAY 280 

    0x3f99ba05,// 284 PAY 281 

    0x621137a2,// 285 PAY 282 

    0x857716bf,// 286 PAY 283 

    0xa42b09da,// 287 PAY 284 

    0x3f393bd9,// 288 PAY 285 

    0x55eeb8c7,// 289 PAY 286 

    0xc753983e,// 290 PAY 287 

    0x531ec60b,// 291 PAY 288 

    0xdcf0ed2c,// 292 PAY 289 

    0x970514f5,// 293 PAY 290 

    0x4e6a8d96,// 294 PAY 291 

    0x4fa11809,// 295 PAY 292 

    0x473523d9,// 296 PAY 293 

    0x83c7c08a,// 297 PAY 294 

    0xbae0d38a,// 298 PAY 295 

    0x4f9448da,// 299 PAY 296 

    0x58a66d5f,// 300 PAY 297 

    0x5ee527c0,// 301 PAY 298 

    0xa6b0f042,// 302 PAY 299 

    0x0a70ca4e,// 303 PAY 300 

    0xa0d8458a,// 304 PAY 301 

    0x7cb2264b,// 305 PAY 302 

    0x77344c62,// 306 PAY 303 

    0xb8019e05,// 307 PAY 304 

    0xf5e448f7,// 308 PAY 305 

    0x2ac80a9a,// 309 PAY 306 

    0x8e3e9cb2,// 310 PAY 307 

    0x1b4e0608,// 311 PAY 308 

    0xfb02b3ca,// 312 PAY 309 

    0x4927c2de,// 313 PAY 310 

    0xd7480d8c,// 314 PAY 311 

    0xdf66da80,// 315 PAY 312 

    0xf1a793c9,// 316 PAY 313 

    0xd2e292a7,// 317 PAY 314 

    0xabb2d534,// 318 PAY 315 

    0xf5b3dd48,// 319 PAY 316 

    0xc1ea3dea,// 320 PAY 317 

    0xf0baae64,// 321 PAY 318 

    0x3e921344,// 322 PAY 319 

    0xd063b2ae,// 323 PAY 320 

    0x0e796105,// 324 PAY 321 

    0x5cf12f46,// 325 PAY 322 

    0xc1aea112,// 326 PAY 323 

    0x224d680b,// 327 PAY 324 

    0xa368fa38,// 328 PAY 325 

    0xa5c663ff,// 329 PAY 326 

    0x1a9e2931,// 330 PAY 327 

    0x716c2859,// 331 PAY 328 

    0xe9057ff8,// 332 PAY 329 

    0xc3c0dcec,// 333 PAY 330 

    0x4dede3c0,// 334 PAY 331 

    0xe0e8160a,// 335 PAY 332 

    0x2e412a4b,// 336 PAY 333 

    0x39bfb4cb,// 337 PAY 334 

    0x6020a10f,// 338 PAY 335 

    0x6062ad52,// 339 PAY 336 

    0xe57c05f4,// 340 PAY 337 

    0xd6337d16,// 341 PAY 338 

    0xb8a29f8c,// 342 PAY 339 

    0xc5ae644b,// 343 PAY 340 

    0x9c7d3b9b,// 344 PAY 341 

    0x259560f0,// 345 PAY 342 

    0x8d207083,// 346 PAY 343 

    0x67463148,// 347 PAY 344 

    0xd08880d2,// 348 PAY 345 

    0xdeb65ad7,// 349 PAY 346 

    0xedbc1486,// 350 PAY 347 

    0xed0c8bc2,// 351 PAY 348 

    0x48b9afd9,// 352 PAY 349 

    0xa50d7280,// 353 PAY 350 

    0x9848ea1d,// 354 PAY 351 

    0x9aefc2d2,// 355 PAY 352 

    0x7a7e6543,// 356 PAY 353 

    0xecf56c0c,// 357 PAY 354 

    0x3069e0b1,// 358 PAY 355 

    0xf9afb0c5,// 359 PAY 356 

    0x15d66b62,// 360 PAY 357 

    0xa169f833,// 361 PAY 358 

    0x119c321a,// 362 PAY 359 

    0xbd9e93ce,// 363 PAY 360 

    0xdbe20c23,// 364 PAY 361 

    0x2463293e,// 365 PAY 362 

    0xe24b6bcf,// 366 PAY 363 

    0xce2ad411,// 367 PAY 364 

    0x8c5934cf,// 368 PAY 365 

    0x1324d79d,// 369 PAY 366 

    0xc6c2f973,// 370 PAY 367 

    0x862cca59,// 371 PAY 368 

    0xe98959e0,// 372 PAY 369 

    0xfd1e71a8,// 373 PAY 370 

    0x493aabce,// 374 PAY 371 

    0xb416beb3,// 375 PAY 372 

    0x9386bbe2,// 376 PAY 373 

    0x010a0b81,// 377 PAY 374 

    0x30237ff0,// 378 PAY 375 

    0x78397216,// 379 PAY 376 

    0xc4abbe6a,// 380 PAY 377 

    0x6eab6558,// 381 PAY 378 

    0xa00818e7,// 382 PAY 379 

    0xb9987d19,// 383 PAY 380 

    0x7f10394f,// 384 PAY 381 

    0xa28608e0,// 385 PAY 382 

    0xda71a7da,// 386 PAY 383 

    0x78ed3a53,// 387 PAY 384 

    0xe30531ca,// 388 PAY 385 

    0xf70dbbc1,// 389 PAY 386 

    0x44ca7fd2,// 390 PAY 387 

    0x4c841d3b,// 391 PAY 388 

    0xdbdf04e4,// 392 PAY 389 

    0xf07555ba,// 393 PAY 390 

    0x9e115b70,// 394 PAY 391 

    0x93b894a5,// 395 PAY 392 

    0xac2b6bbd,// 396 PAY 393 

    0x7f90ace6,// 397 PAY 394 

    0xe3bca47c,// 398 PAY 395 

    0x2e0e4e02,// 399 PAY 396 

    0x630819af,// 400 PAY 397 

    0x8019ee20,// 401 PAY 398 

    0x082c1fa4,// 402 PAY 399 

    0xcb932028,// 403 PAY 400 

    0xdb63dd73,// 404 PAY 401 

    0x306cdecf,// 405 PAY 402 

    0x6a48321b,// 406 PAY 403 

    0x6c4fd51d,// 407 PAY 404 

    0x152cb477,// 408 PAY 405 

    0x7f0e50a9,// 409 PAY 406 

    0x6667a3bc,// 410 PAY 407 

    0x9a77d373,// 411 PAY 408 

    0x26fda115,// 412 PAY 409 

    0x21b99162,// 413 PAY 410 

    0x78cd0a7e,// 414 PAY 411 

    0x14fc5559,// 415 PAY 412 

    0x37d72d78,// 416 PAY 413 

    0x96a2f549,// 417 PAY 414 

    0x55899458,// 418 PAY 415 

    0xca1239ed,// 419 PAY 416 

    0xbcd17b5a,// 420 PAY 417 

    0x7f1b4d04,// 421 PAY 418 

    0xab3a8917,// 422 PAY 419 

    0x9c6bdaac,// 423 PAY 420 

    0xc7a8a4b6,// 424 PAY 421 

    0x03bc8383,// 425 PAY 422 

    0x1d186496,// 426 PAY 423 

    0xf84ce957,// 427 PAY 424 

    0x8820fd4a,// 428 PAY 425 

    0xd1ee0d6c,// 429 PAY 426 

    0x0eade609,// 430 PAY 427 

    0x35d734d5,// 431 PAY 428 

    0xa069b0c7,// 432 PAY 429 

    0x3e345d9e,// 433 PAY 430 

    0x76fd174b,// 434 PAY 431 

    0xf60e95c9,// 435 PAY 432 

    0x6328d70a,// 436 PAY 433 

    0x80a148aa,// 437 PAY 434 

    0xf7bc31c2,// 438 PAY 435 

    0x2fdf05dd,// 439 PAY 436 

    0xef3d8602,// 440 PAY 437 

    0xaf35fabd,// 441 PAY 438 

    0x239394e1,// 442 PAY 439 

    0xdb63bf90,// 443 PAY 440 

    0x567c7a39,// 444 PAY 441 

    0x1ca87d03,// 445 PAY 442 

    0x4cae362d,// 446 PAY 443 

    0x8ae553a6,// 447 PAY 444 

    0x346206a7,// 448 PAY 445 

    0x7f037a0d,// 449 PAY 446 

    0x5541a332,// 450 PAY 447 

    0x934d5dab,// 451 PAY 448 

    0xff8d0446,// 452 PAY 449 

    0x8447a682,// 453 PAY 450 

    0xc3bd73eb,// 454 PAY 451 

    0x25a4218b,// 455 PAY 452 

    0x946b1b99,// 456 PAY 453 

    0x043a591b,// 457 PAY 454 

    0x9b8353a6,// 458 PAY 455 

    0xf34bcb4e,// 459 PAY 456 

    0xf0283a27,// 460 PAY 457 

    0x8c7d1b86,// 461 PAY 458 

    0x455d423b,// 462 PAY 459 

    0x4491b890,// 463 PAY 460 

    0x1116e5cd,// 464 PAY 461 

    0xfbb7d532,// 465 PAY 462 

    0x2161ae47,// 466 PAY 463 

    0x6a7933dd,// 467 PAY 464 

    0x6a68d189,// 468 PAY 465 

    0x64e2fc55,// 469 PAY 466 

    0xc8c08719,// 470 PAY 467 

    0x2b04ae1f,// 471 PAY 468 

    0x937c1553,// 472 PAY 469 

    0x50ba2a72,// 473 PAY 470 

    0x56ee2365,// 474 PAY 471 

    0x94368a2e,// 475 PAY 472 

    0x2256129c,// 476 PAY 473 

    0x52e0f413,// 477 PAY 474 

    0x16920000,// 478 PAY 475 

/// HASH is  16 bytes 

    0x5dd4463b,// 479 HSH   1 

    0xb83a7a1e,// 480 HSH   2 

    0xcfc4e10c,// 481 HSH   3 

    0xceddcbbd,// 482 HSH   4 

/// STA is 1 words. 

/// STA num_pkts       : 86 

/// STA pkt_idx        : 224 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xf3 

    0x0381f356 // 483 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt93_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 197 words. 

/// BDA size     is 781 (0x30d) 

/// BDA id       is 0x3f09 

    0x030d3f09,// 3 BDA   1 

/// PAY Generic Data size   : 781 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x553f440a,// 4 PAY   1 

    0x4e27c465,// 5 PAY   2 

    0x6d86874d,// 6 PAY   3 

    0xddaf0e1b,// 7 PAY   4 

    0xb789315f,// 8 PAY   5 

    0x55127782,// 9 PAY   6 

    0xf7df928c,// 10 PAY   7 

    0x29b6c6f4,// 11 PAY   8 

    0x993c594c,// 12 PAY   9 

    0xc51ec4d6,// 13 PAY  10 

    0x0531be22,// 14 PAY  11 

    0x8a34496b,// 15 PAY  12 

    0x817548e0,// 16 PAY  13 

    0x2699fe53,// 17 PAY  14 

    0x775d2e73,// 18 PAY  15 

    0x2fdc43ff,// 19 PAY  16 

    0xad920d93,// 20 PAY  17 

    0x51955497,// 21 PAY  18 

    0x34a4560a,// 22 PAY  19 

    0x08110505,// 23 PAY  20 

    0x6d88139b,// 24 PAY  21 

    0xd4eedc9d,// 25 PAY  22 

    0x63978d3d,// 26 PAY  23 

    0x4df77ddc,// 27 PAY  24 

    0x699903cc,// 28 PAY  25 

    0x7d330e10,// 29 PAY  26 

    0x426c6f11,// 30 PAY  27 

    0x45204a7e,// 31 PAY  28 

    0x9c917d92,// 32 PAY  29 

    0x07c3fdeb,// 33 PAY  30 

    0xe53317de,// 34 PAY  31 

    0xac1398bd,// 35 PAY  32 

    0x2c5bedc1,// 36 PAY  33 

    0x553101df,// 37 PAY  34 

    0xe88f56d9,// 38 PAY  35 

    0x00a6c7b5,// 39 PAY  36 

    0x459c6b11,// 40 PAY  37 

    0x58981f16,// 41 PAY  38 

    0x1c92925d,// 42 PAY  39 

    0x1942fd52,// 43 PAY  40 

    0xb00586bd,// 44 PAY  41 

    0x22889fa9,// 45 PAY  42 

    0xdfc46659,// 46 PAY  43 

    0xe748cc59,// 47 PAY  44 

    0x6a705572,// 48 PAY  45 

    0x711dcd97,// 49 PAY  46 

    0x9aa98a18,// 50 PAY  47 

    0x0a809302,// 51 PAY  48 

    0xdf006c4a,// 52 PAY  49 

    0x49492ade,// 53 PAY  50 

    0xd96bc3fb,// 54 PAY  51 

    0xfb34cec8,// 55 PAY  52 

    0xf0fb47f0,// 56 PAY  53 

    0xd4c0d3d2,// 57 PAY  54 

    0x049e0af4,// 58 PAY  55 

    0x0b7f69fe,// 59 PAY  56 

    0x6f701c86,// 60 PAY  57 

    0xaff54bba,// 61 PAY  58 

    0xca4b2a4d,// 62 PAY  59 

    0x18d1c30c,// 63 PAY  60 

    0xe69ec023,// 64 PAY  61 

    0xc752d576,// 65 PAY  62 

    0x8405002f,// 66 PAY  63 

    0x6508744b,// 67 PAY  64 

    0x509ae062,// 68 PAY  65 

    0x9f3aa1c0,// 69 PAY  66 

    0x68ca0624,// 70 PAY  67 

    0xbb841899,// 71 PAY  68 

    0x94a402c1,// 72 PAY  69 

    0xe1743185,// 73 PAY  70 

    0xd7a8217a,// 74 PAY  71 

    0xeef6675e,// 75 PAY  72 

    0xb0649a54,// 76 PAY  73 

    0x21c7a705,// 77 PAY  74 

    0x5645f593,// 78 PAY  75 

    0x0b1e6b3d,// 79 PAY  76 

    0x0902486d,// 80 PAY  77 

    0x58ccbc7f,// 81 PAY  78 

    0x2ee79f4f,// 82 PAY  79 

    0x3366c22c,// 83 PAY  80 

    0x76315afd,// 84 PAY  81 

    0x4d5c140d,// 85 PAY  82 

    0x2376efcc,// 86 PAY  83 

    0x07258617,// 87 PAY  84 

    0xce6945ac,// 88 PAY  85 

    0x7e64e055,// 89 PAY  86 

    0x1f3a96b2,// 90 PAY  87 

    0xd003f0ea,// 91 PAY  88 

    0x9deb815e,// 92 PAY  89 

    0xb931cc04,// 93 PAY  90 

    0x34062b28,// 94 PAY  91 

    0xafc213c0,// 95 PAY  92 

    0x16953276,// 96 PAY  93 

    0xbcb5d779,// 97 PAY  94 

    0x2ae26cb8,// 98 PAY  95 

    0xfd5e11ef,// 99 PAY  96 

    0xf36ab6a4,// 100 PAY  97 

    0x43acaf99,// 101 PAY  98 

    0x77cb2436,// 102 PAY  99 

    0xeb72efd4,// 103 PAY 100 

    0xe94d6b67,// 104 PAY 101 

    0x5c1506ad,// 105 PAY 102 

    0xf3feea2c,// 106 PAY 103 

    0x10e87dae,// 107 PAY 104 

    0xbc06df42,// 108 PAY 105 

    0xa911c176,// 109 PAY 106 

    0xc3f5314e,// 110 PAY 107 

    0x11f351a0,// 111 PAY 108 

    0x6cb11786,// 112 PAY 109 

    0xc1985fc5,// 113 PAY 110 

    0xdbeb7766,// 114 PAY 111 

    0x8ca94424,// 115 PAY 112 

    0x6e06ded0,// 116 PAY 113 

    0xea4bd162,// 117 PAY 114 

    0x81f78407,// 118 PAY 115 

    0x79b32b3c,// 119 PAY 116 

    0x571542b4,// 120 PAY 117 

    0xc53dccb1,// 121 PAY 118 

    0xd7735810,// 122 PAY 119 

    0x7ae4abc3,// 123 PAY 120 

    0xee49163f,// 124 PAY 121 

    0xce124011,// 125 PAY 122 

    0x008ba74f,// 126 PAY 123 

    0x4635c2fb,// 127 PAY 124 

    0x1fd974e9,// 128 PAY 125 

    0xc80ec84e,// 129 PAY 126 

    0x0175c003,// 130 PAY 127 

    0xc90dd298,// 131 PAY 128 

    0x0e912aaa,// 132 PAY 129 

    0xdeea2bd3,// 133 PAY 130 

    0xd8db98d9,// 134 PAY 131 

    0x88eaae29,// 135 PAY 132 

    0x1296661b,// 136 PAY 133 

    0xb3b07b5a,// 137 PAY 134 

    0x5d166b8a,// 138 PAY 135 

    0x1ebb0620,// 139 PAY 136 

    0xeac0ba62,// 140 PAY 137 

    0x838f1f9b,// 141 PAY 138 

    0x16401570,// 142 PAY 139 

    0x39aae24d,// 143 PAY 140 

    0x39b4b7ac,// 144 PAY 141 

    0x786f19dc,// 145 PAY 142 

    0xad923b5d,// 146 PAY 143 

    0x0d9e3457,// 147 PAY 144 

    0x678d2091,// 148 PAY 145 

    0x2b6b8915,// 149 PAY 146 

    0xc7f81682,// 150 PAY 147 

    0xb80e9bca,// 151 PAY 148 

    0x88f6ada3,// 152 PAY 149 

    0x6a515ed9,// 153 PAY 150 

    0xe1970c5f,// 154 PAY 151 

    0x9fa0522b,// 155 PAY 152 

    0xc8592699,// 156 PAY 153 

    0x4aa4f20e,// 157 PAY 154 

    0x4b5d8f3d,// 158 PAY 155 

    0x43a3b698,// 159 PAY 156 

    0xd8e92459,// 160 PAY 157 

    0x90368a75,// 161 PAY 158 

    0xbfbef7fd,// 162 PAY 159 

    0x94c5eaa5,// 163 PAY 160 

    0xc2b12282,// 164 PAY 161 

    0x212086fc,// 165 PAY 162 

    0xd16bcbb5,// 166 PAY 163 

    0x3e327ca9,// 167 PAY 164 

    0x51d9bfd6,// 168 PAY 165 

    0x2fcd6f34,// 169 PAY 166 

    0x6ce58d0a,// 170 PAY 167 

    0x451db642,// 171 PAY 168 

    0x44322460,// 172 PAY 169 

    0xb401a3c5,// 173 PAY 170 

    0x919072ff,// 174 PAY 171 

    0x5378f886,// 175 PAY 172 

    0x0b5b7fca,// 176 PAY 173 

    0xe1a59ad7,// 177 PAY 174 

    0xf00b270e,// 178 PAY 175 

    0x463c3369,// 179 PAY 176 

    0xcc41b8fb,// 180 PAY 177 

    0x69fa9c92,// 181 PAY 178 

    0x073a8ca7,// 182 PAY 179 

    0xf423454e,// 183 PAY 180 

    0x23147fa0,// 184 PAY 181 

    0xdf6efe01,// 185 PAY 182 

    0xec7eda48,// 186 PAY 183 

    0x691433a4,// 187 PAY 184 

    0x8130d741,// 188 PAY 185 

    0x678c3349,// 189 PAY 186 

    0x43284208,// 190 PAY 187 

    0xa882c942,// 191 PAY 188 

    0x027a22ae,// 192 PAY 189 

    0xfd89de39,// 193 PAY 190 

    0xb7e4a1f4,// 194 PAY 191 

    0xd2e86e3a,// 195 PAY 192 

    0x8738c5fd,// 196 PAY 193 

    0xc3016769,// 197 PAY 194 

    0xef3fe84f,// 198 PAY 195 

    0xd8000000,// 199 PAY 196 

/// HASH is  12 bytes 

    0xcc41b8fb,// 200 HSH   1 

    0x69fa9c92,// 201 HSH   2 

    0x073a8ca7,// 202 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 67 

/// STA pkt_idx        : 90 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xd1 

    0x0168d143 // 203 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt94_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x06 

/// ECH pdu_tag        : 0x00 

    0x00060000,// 2 ECH   1 

/// BDA is 157 words. 

/// BDA size     is 624 (0x270) 

/// BDA id       is 0xda3e 

    0x0270da3e,// 3 BDA   1 

/// PAY Generic Data size   : 624 byte(s) 

/// PAD Buffer Data Pad size   : 0 byte(s) 

    0xdb219504,// 4 PAY   1 

    0x5127abf2,// 5 PAY   2 

    0x7f7e2924,// 6 PAY   3 

    0xf136ea50,// 7 PAY   4 

    0x970f1c57,// 8 PAY   5 

    0x7e37425d,// 9 PAY   6 

    0xaccbce3b,// 10 PAY   7 

    0x8704e9e8,// 11 PAY   8 

    0x8db1f003,// 12 PAY   9 

    0xb81ead53,// 13 PAY  10 

    0x423d5bc7,// 14 PAY  11 

    0xadb43b06,// 15 PAY  12 

    0xd6da9f65,// 16 PAY  13 

    0xe6a63704,// 17 PAY  14 

    0x109d97c5,// 18 PAY  15 

    0x1296b6de,// 19 PAY  16 

    0xee792a9b,// 20 PAY  17 

    0x093deba1,// 21 PAY  18 

    0xf5acd6ea,// 22 PAY  19 

    0x22ddff48,// 23 PAY  20 

    0x39bed082,// 24 PAY  21 

    0xc39f551d,// 25 PAY  22 

    0x1a6f473b,// 26 PAY  23 

    0x0dfcfd79,// 27 PAY  24 

    0x015aa6b5,// 28 PAY  25 

    0x986100ff,// 29 PAY  26 

    0x9f9f2630,// 30 PAY  27 

    0xaa723ccb,// 31 PAY  28 

    0xb72d0114,// 32 PAY  29 

    0x2205784c,// 33 PAY  30 

    0x53dcc453,// 34 PAY  31 

    0x02387e26,// 35 PAY  32 

    0x6db2913d,// 36 PAY  33 

    0x13f8c443,// 37 PAY  34 

    0xb0dcf045,// 38 PAY  35 

    0x66dfd6bd,// 39 PAY  36 

    0xa85ce99f,// 40 PAY  37 

    0xd40dadf4,// 41 PAY  38 

    0x08264254,// 42 PAY  39 

    0x2cf66ef1,// 43 PAY  40 

    0xd0705741,// 44 PAY  41 

    0x742b1b2c,// 45 PAY  42 

    0x0a3db824,// 46 PAY  43 

    0x8ce8c94c,// 47 PAY  44 

    0xe07962b8,// 48 PAY  45 

    0x351a19a6,// 49 PAY  46 

    0x1204035e,// 50 PAY  47 

    0x5b916bd4,// 51 PAY  48 

    0xc9da7f61,// 52 PAY  49 

    0x9e98c4e9,// 53 PAY  50 

    0xbe78ebb4,// 54 PAY  51 

    0xd87222a2,// 55 PAY  52 

    0xc7993e7c,// 56 PAY  53 

    0x6ee8439a,// 57 PAY  54 

    0xc8c364f8,// 58 PAY  55 

    0x6eb207b5,// 59 PAY  56 

    0x4bbb7463,// 60 PAY  57 

    0x14167367,// 61 PAY  58 

    0xf536e554,// 62 PAY  59 

    0xa2fb80b7,// 63 PAY  60 

    0xb0de2eb0,// 64 PAY  61 

    0xa8f44d2d,// 65 PAY  62 

    0x5ef5b2ab,// 66 PAY  63 

    0xbf302d7d,// 67 PAY  64 

    0x0703e0d2,// 68 PAY  65 

    0x4cb21dd7,// 69 PAY  66 

    0x3c5d0548,// 70 PAY  67 

    0x5137c430,// 71 PAY  68 

    0xb98045f7,// 72 PAY  69 

    0xfb77ad28,// 73 PAY  70 

    0x17f5d9d9,// 74 PAY  71 

    0xcbb43004,// 75 PAY  72 

    0x0cdd29eb,// 76 PAY  73 

    0x16cff02a,// 77 PAY  74 

    0xbbbaa28c,// 78 PAY  75 

    0x9d4c1e9a,// 79 PAY  76 

    0xe4313eb3,// 80 PAY  77 

    0xdede1d88,// 81 PAY  78 

    0xc4c1c340,// 82 PAY  79 

    0x8e9074cb,// 83 PAY  80 

    0x5a4dbabf,// 84 PAY  81 

    0x863ba62f,// 85 PAY  82 

    0x47c5c280,// 86 PAY  83 

    0xeb812100,// 87 PAY  84 

    0xe02833fa,// 88 PAY  85 

    0x6d1ff2e6,// 89 PAY  86 

    0xf5a968ae,// 90 PAY  87 

    0x80ca9ee0,// 91 PAY  88 

    0xa10ba397,// 92 PAY  89 

    0x1a8a5af8,// 93 PAY  90 

    0x2476f825,// 94 PAY  91 

    0x1037e715,// 95 PAY  92 

    0x50b3df46,// 96 PAY  93 

    0x1ec4aa43,// 97 PAY  94 

    0xe666957d,// 98 PAY  95 

    0x79dc6607,// 99 PAY  96 

    0xb27d72e5,// 100 PAY  97 

    0xfa6cd44d,// 101 PAY  98 

    0x3edab3ec,// 102 PAY  99 

    0xf737991d,// 103 PAY 100 

    0x9412aa63,// 104 PAY 101 

    0x8f1fc029,// 105 PAY 102 

    0x0f18a6e1,// 106 PAY 103 

    0x574b88d3,// 107 PAY 104 

    0x12f64404,// 108 PAY 105 

    0x1b51d2a2,// 109 PAY 106 

    0xf4b5590a,// 110 PAY 107 

    0xd52b0fe4,// 111 PAY 108 

    0x2f847766,// 112 PAY 109 

    0x1908fa15,// 113 PAY 110 

    0xba60ce52,// 114 PAY 111 

    0x37ea34fd,// 115 PAY 112 

    0x6d9ae3cb,// 116 PAY 113 

    0xfb18e0eb,// 117 PAY 114 

    0x41c2599e,// 118 PAY 115 

    0x2d5475dc,// 119 PAY 116 

    0x6af6826c,// 120 PAY 117 

    0x67da6bf6,// 121 PAY 118 

    0x1d12f200,// 122 PAY 119 

    0x173442b1,// 123 PAY 120 

    0xc29bf154,// 124 PAY 121 

    0xfdd0c00a,// 125 PAY 122 

    0x7b379c49,// 126 PAY 123 

    0xf0583cf3,// 127 PAY 124 

    0xc0aa4d76,// 128 PAY 125 

    0x07faea15,// 129 PAY 126 

    0x82d77572,// 130 PAY 127 

    0x5f3b2fe0,// 131 PAY 128 

    0x9295f5e8,// 132 PAY 129 

    0xc9f1fd68,// 133 PAY 130 

    0x9736f5d6,// 134 PAY 131 

    0xf6e84f4c,// 135 PAY 132 

    0xbfad8ec1,// 136 PAY 133 

    0x2151df9c,// 137 PAY 134 

    0xde60ca59,// 138 PAY 135 

    0xeb705b96,// 139 PAY 136 

    0x8e29b711,// 140 PAY 137 

    0xfeaa07d3,// 141 PAY 138 

    0x0ac3cfb6,// 142 PAY 139 

    0x9af487b3,// 143 PAY 140 

    0x6797d5b6,// 144 PAY 141 

    0x88ee8986,// 145 PAY 142 

    0x4a8de91e,// 146 PAY 143 

    0x3d90a3ca,// 147 PAY 144 

    0x43fbac14,// 148 PAY 145 

    0x9ac4c7dc,// 149 PAY 146 

    0x8d7693b8,// 150 PAY 147 

    0x6d8a2d4f,// 151 PAY 148 

    0xfb9b59c6,// 152 PAY 149 

    0xb1f72fd5,// 153 PAY 150 

    0xd57251b8,// 154 PAY 151 

    0x5382ccc6,// 155 PAY 152 

    0xfae58490,// 156 PAY 153 

    0x3b09ce83,// 157 PAY 154 

    0x87caac0b,// 158 PAY 155 

    0xce93d948,// 159 PAY 156 

/// STA is 1 words. 

/// STA num_pkts       : 175 

/// STA pkt_idx        : 23 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xc9 

    0x005dc9af // 160 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt95_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x0a 

/// ECH pdu_tag        : 0x00 

    0x000a0000,// 2 ECH   1 

/// BDA is 209 words. 

/// BDA size     is 829 (0x33d) 

/// BDA id       is 0x6c9d 

    0x033d6c9d,// 3 BDA   1 

/// PAY Generic Data size   : 829 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x0b0c9dba,// 4 PAY   1 

    0xbead07bf,// 5 PAY   2 

    0x5cdc5ab6,// 6 PAY   3 

    0xd7de096a,// 7 PAY   4 

    0x1415766b,// 8 PAY   5 

    0x400273b9,// 9 PAY   6 

    0xdd3f24c9,// 10 PAY   7 

    0x742345df,// 11 PAY   8 

    0x30ecbc61,// 12 PAY   9 

    0x35fd4291,// 13 PAY  10 

    0x3a7f0cfa,// 14 PAY  11 

    0x1cb68040,// 15 PAY  12 

    0x90d208e1,// 16 PAY  13 

    0x00b3d7c9,// 17 PAY  14 

    0x327d067f,// 18 PAY  15 

    0xc11fa81b,// 19 PAY  16 

    0xbb10bdc0,// 20 PAY  17 

    0x3ab09f09,// 21 PAY  18 

    0x67afb75c,// 22 PAY  19 

    0x4eaa29cc,// 23 PAY  20 

    0xa187a766,// 24 PAY  21 

    0xd5c26a79,// 25 PAY  22 

    0x05651a08,// 26 PAY  23 

    0x43f2fbf0,// 27 PAY  24 

    0x3480b42b,// 28 PAY  25 

    0x6d180c62,// 29 PAY  26 

    0x0f38df75,// 30 PAY  27 

    0xf88e5193,// 31 PAY  28 

    0xd51013a0,// 32 PAY  29 

    0xe35fa9de,// 33 PAY  30 

    0x38a0b804,// 34 PAY  31 

    0xb8d66dc1,// 35 PAY  32 

    0x3e6963a5,// 36 PAY  33 

    0xce7404fa,// 37 PAY  34 

    0xa2666494,// 38 PAY  35 

    0xc4af3e5c,// 39 PAY  36 

    0x102c150e,// 40 PAY  37 

    0xf37c766a,// 41 PAY  38 

    0x36042106,// 42 PAY  39 

    0xe62524ce,// 43 PAY  40 

    0xbe3b8b33,// 44 PAY  41 

    0x06c2974a,// 45 PAY  42 

    0x9b4dc65e,// 46 PAY  43 

    0x236d93cf,// 47 PAY  44 

    0xb6ea6699,// 48 PAY  45 

    0x5884b327,// 49 PAY  46 

    0xc92d9755,// 50 PAY  47 

    0x2a4a61ea,// 51 PAY  48 

    0x6ad24460,// 52 PAY  49 

    0x17b67d3b,// 53 PAY  50 

    0xd219fe1f,// 54 PAY  51 

    0x0eeb3ae2,// 55 PAY  52 

    0x0cea5053,// 56 PAY  53 

    0xdfc3caae,// 57 PAY  54 

    0xdb644dcc,// 58 PAY  55 

    0x625304f7,// 59 PAY  56 

    0xf1b4cefe,// 60 PAY  57 

    0x6c1269df,// 61 PAY  58 

    0x7a3dc92c,// 62 PAY  59 

    0x1bebfe01,// 63 PAY  60 

    0x062dd01c,// 64 PAY  61 

    0xf35544c5,// 65 PAY  62 

    0xc382ec90,// 66 PAY  63 

    0xa1489616,// 67 PAY  64 

    0x2cd8774b,// 68 PAY  65 

    0x88ace8b8,// 69 PAY  66 

    0xc9838863,// 70 PAY  67 

    0xd9d0de58,// 71 PAY  68 

    0xe529702e,// 72 PAY  69 

    0x233bc330,// 73 PAY  70 

    0xcfb94121,// 74 PAY  71 

    0x54f52c44,// 75 PAY  72 

    0x1b427d06,// 76 PAY  73 

    0x4aced446,// 77 PAY  74 

    0x4059af79,// 78 PAY  75 

    0x0694568b,// 79 PAY  76 

    0x575732ed,// 80 PAY  77 

    0xbf5be74a,// 81 PAY  78 

    0x04835dd2,// 82 PAY  79 

    0x37df66b7,// 83 PAY  80 

    0x7dd71794,// 84 PAY  81 

    0x9cbd8aa6,// 85 PAY  82 

    0x72cf66a3,// 86 PAY  83 

    0xdb6d2dff,// 87 PAY  84 

    0xd9c5be0b,// 88 PAY  85 

    0xae85c3b8,// 89 PAY  86 

    0xeb25f1bc,// 90 PAY  87 

    0x1fbe99d2,// 91 PAY  88 

    0x6c2f95b3,// 92 PAY  89 

    0xd4473b71,// 93 PAY  90 

    0x03093079,// 94 PAY  91 

    0xd0766f60,// 95 PAY  92 

    0x94f4790f,// 96 PAY  93 

    0x27cacabf,// 97 PAY  94 

    0x01ec472a,// 98 PAY  95 

    0x8bb334bf,// 99 PAY  96 

    0x3c35502e,// 100 PAY  97 

    0x9e3402b5,// 101 PAY  98 

    0xaa31c11f,// 102 PAY  99 

    0x8989300a,// 103 PAY 100 

    0xa54985b4,// 104 PAY 101 

    0x6ccd3cf4,// 105 PAY 102 

    0xc64004c4,// 106 PAY 103 

    0xf76a054f,// 107 PAY 104 

    0x3f35834f,// 108 PAY 105 

    0xa52dc7a0,// 109 PAY 106 

    0x574d621c,// 110 PAY 107 

    0xe5460433,// 111 PAY 108 

    0xba289140,// 112 PAY 109 

    0xad1bcfdb,// 113 PAY 110 

    0x948ca185,// 114 PAY 111 

    0xf6f7e9f6,// 115 PAY 112 

    0xe01ec7e0,// 116 PAY 113 

    0x18f78f7f,// 117 PAY 114 

    0xbf60f37e,// 118 PAY 115 

    0xb5c8971f,// 119 PAY 116 

    0x88608823,// 120 PAY 117 

    0x4422b775,// 121 PAY 118 

    0x180744c6,// 122 PAY 119 

    0xa7875756,// 123 PAY 120 

    0xa40951d1,// 124 PAY 121 

    0x8a6b453f,// 125 PAY 122 

    0x98da1ffb,// 126 PAY 123 

    0x02f8b0bf,// 127 PAY 124 

    0x0dc6a35f,// 128 PAY 125 

    0xd5f3a564,// 129 PAY 126 

    0x47cd6fee,// 130 PAY 127 

    0x7ba071f5,// 131 PAY 128 

    0xadce2ed6,// 132 PAY 129 

    0x6f7fb4c4,// 133 PAY 130 

    0x9d0683d2,// 134 PAY 131 

    0x08cb6299,// 135 PAY 132 

    0x6354db09,// 136 PAY 133 

    0x74541d29,// 137 PAY 134 

    0xa8ca16d9,// 138 PAY 135 

    0xc2925b34,// 139 PAY 136 

    0x7c4f4b8d,// 140 PAY 137 

    0x9d3f542b,// 141 PAY 138 

    0x81f2a342,// 142 PAY 139 

    0x0bfd0b7d,// 143 PAY 140 

    0x69af8e49,// 144 PAY 141 

    0xc66b7220,// 145 PAY 142 

    0x936dab3b,// 146 PAY 143 

    0x54e6b494,// 147 PAY 144 

    0x0793c70f,// 148 PAY 145 

    0x8c3ad103,// 149 PAY 146 

    0x9323db39,// 150 PAY 147 

    0x869ab7a1,// 151 PAY 148 

    0xd0333924,// 152 PAY 149 

    0x0ceeee3e,// 153 PAY 150 

    0x0e18897f,// 154 PAY 151 

    0x0894ec2c,// 155 PAY 152 

    0x9614f558,// 156 PAY 153 

    0xa42e3997,// 157 PAY 154 

    0x8b1837ea,// 158 PAY 155 

    0x64f795a6,// 159 PAY 156 

    0x02063fe9,// 160 PAY 157 

    0xf422b669,// 161 PAY 158 

    0x2768e9b3,// 162 PAY 159 

    0xa7dd2964,// 163 PAY 160 

    0x265ac210,// 164 PAY 161 

    0xf2974f61,// 165 PAY 162 

    0x23ffdc65,// 166 PAY 163 

    0x809154dc,// 167 PAY 164 

    0x153d30fa,// 168 PAY 165 

    0xfbde621c,// 169 PAY 166 

    0x1fca3222,// 170 PAY 167 

    0x8d6f96df,// 171 PAY 168 

    0x5b4cc835,// 172 PAY 169 

    0xad6a978e,// 173 PAY 170 

    0xd06a53f6,// 174 PAY 171 

    0x75b45d3d,// 175 PAY 172 

    0x7b46941c,// 176 PAY 173 

    0x7b14f274,// 177 PAY 174 

    0x383cf6f5,// 178 PAY 175 

    0x3a18e510,// 179 PAY 176 

    0x29e52cf4,// 180 PAY 177 

    0x2bc4c899,// 181 PAY 178 

    0x866a4246,// 182 PAY 179 

    0x1422b74d,// 183 PAY 180 

    0x53f64dc6,// 184 PAY 181 

    0x9d1459e8,// 185 PAY 182 

    0x4bd5abec,// 186 PAY 183 

    0x7afc3d76,// 187 PAY 184 

    0xc708e8f7,// 188 PAY 185 

    0x0c7f3247,// 189 PAY 186 

    0xafffee02,// 190 PAY 187 

    0xd1001707,// 191 PAY 188 

    0xc335a168,// 192 PAY 189 

    0x328b57e1,// 193 PAY 190 

    0x6ac863a4,// 194 PAY 191 

    0xa686e628,// 195 PAY 192 

    0x0f19cbe3,// 196 PAY 193 

    0xb8ee59db,// 197 PAY 194 

    0xded0fda9,// 198 PAY 195 

    0x65292191,// 199 PAY 196 

    0x40a66339,// 200 PAY 197 

    0xb3d78183,// 201 PAY 198 

    0x524141fd,// 202 PAY 199 

    0x66eccbc6,// 203 PAY 200 

    0x12e71ea8,// 204 PAY 201 

    0xc74a65e6,// 205 PAY 202 

    0x9dc11773,// 206 PAY 203 

    0x7c0021eb,// 207 PAY 204 

    0x6cd1e5ee,// 208 PAY 205 

    0x18a61ccd,// 209 PAY 206 

    0x8d6dfa57,// 210 PAY 207 

    0xd1000000,// 211 PAY 208 

/// STA is 1 words. 

/// STA num_pkts       : 93 

/// STA pkt_idx        : 87 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe2 

    0x015ce25d // 212 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt96_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 436 words. 

/// BDA size     is 1737 (0x6c9) 

/// BDA id       is 0xc09b 

    0x06c9c09b,// 3 BDA   1 

/// PAY Generic Data size   : 1737 byte(s) 

/// PAD Buffer Data Pad size   : 3 byte(s) 

    0x803e7886,// 4 PAY   1 

    0x309f9f59,// 5 PAY   2 

    0x08f7d023,// 6 PAY   3 

    0x03791959,// 7 PAY   4 

    0xae05243d,// 8 PAY   5 

    0x4f7064be,// 9 PAY   6 

    0xc04593ac,// 10 PAY   7 

    0xa216ca28,// 11 PAY   8 

    0xdc3e5a43,// 12 PAY   9 

    0xaad52bb0,// 13 PAY  10 

    0xf07d5630,// 14 PAY  11 

    0x79518034,// 15 PAY  12 

    0xef9e35f2,// 16 PAY  13 

    0x623ccb9f,// 17 PAY  14 

    0xa5c46a8f,// 18 PAY  15 

    0xd1498102,// 19 PAY  16 

    0x5800adea,// 20 PAY  17 

    0xede4c027,// 21 PAY  18 

    0xf6bb5a4f,// 22 PAY  19 

    0xf3523c80,// 23 PAY  20 

    0x40f66712,// 24 PAY  21 

    0xb9011a97,// 25 PAY  22 

    0x94f4eef5,// 26 PAY  23 

    0xcbbded8b,// 27 PAY  24 

    0xc2f60d84,// 28 PAY  25 

    0xa4cea95f,// 29 PAY  26 

    0xfeceea0e,// 30 PAY  27 

    0x55308c69,// 31 PAY  28 

    0x535c1c63,// 32 PAY  29 

    0xfdf83991,// 33 PAY  30 

    0xb12173ed,// 34 PAY  31 

    0x388a2a2d,// 35 PAY  32 

    0x63e11d11,// 36 PAY  33 

    0xc19c89bb,// 37 PAY  34 

    0x7bcd25f1,// 38 PAY  35 

    0x9784eaa4,// 39 PAY  36 

    0x00b2da6b,// 40 PAY  37 

    0x48b44989,// 41 PAY  38 

    0x25b08ed1,// 42 PAY  39 

    0xb1cb141a,// 43 PAY  40 

    0x43595126,// 44 PAY  41 

    0x6661feff,// 45 PAY  42 

    0xa7807761,// 46 PAY  43 

    0x615b2c55,// 47 PAY  44 

    0xf29be969,// 48 PAY  45 

    0xb43a9c8b,// 49 PAY  46 

    0x1584aadd,// 50 PAY  47 

    0x2324bb3b,// 51 PAY  48 

    0xc4fa5d9f,// 52 PAY  49 

    0x6c4f2a13,// 53 PAY  50 

    0x0c69be46,// 54 PAY  51 

    0x8d9ee2a0,// 55 PAY  52 

    0xe741017b,// 56 PAY  53 

    0xeffb6839,// 57 PAY  54 

    0x75f84527,// 58 PAY  55 

    0x71d77708,// 59 PAY  56 

    0x07f5bf2c,// 60 PAY  57 

    0x62eb9e5f,// 61 PAY  58 

    0x81e5d6bb,// 62 PAY  59 

    0x5a4f7793,// 63 PAY  60 

    0xae6646dc,// 64 PAY  61 

    0xd3d63ea5,// 65 PAY  62 

    0xa40c6496,// 66 PAY  63 

    0xdda0afde,// 67 PAY  64 

    0x7251b24b,// 68 PAY  65 

    0x572b0323,// 69 PAY  66 

    0xd22e3e4c,// 70 PAY  67 

    0x72183ae4,// 71 PAY  68 

    0x0ff4c36a,// 72 PAY  69 

    0x86a3a94a,// 73 PAY  70 

    0xe5eda139,// 74 PAY  71 

    0x75d5c005,// 75 PAY  72 

    0x3fdc6b64,// 76 PAY  73 

    0x24f42e3f,// 77 PAY  74 

    0xe66582e8,// 78 PAY  75 

    0x66732da2,// 79 PAY  76 

    0x030fcc7e,// 80 PAY  77 

    0x1d8c7843,// 81 PAY  78 

    0x35fb2f42,// 82 PAY  79 

    0x37ebcbb3,// 83 PAY  80 

    0xc6329b78,// 84 PAY  81 

    0xdcc626a9,// 85 PAY  82 

    0x926a9a87,// 86 PAY  83 

    0x4dc7149c,// 87 PAY  84 

    0x5c79c175,// 88 PAY  85 

    0xb14c58bb,// 89 PAY  86 

    0x5a828885,// 90 PAY  87 

    0x6fcb6c07,// 91 PAY  88 

    0x9b00e40f,// 92 PAY  89 

    0x26dff0c9,// 93 PAY  90 

    0x334ac7f1,// 94 PAY  91 

    0x1a11051d,// 95 PAY  92 

    0x7b35773e,// 96 PAY  93 

    0x10f5f28a,// 97 PAY  94 

    0x8e547b5d,// 98 PAY  95 

    0x407e616c,// 99 PAY  96 

    0x90398b07,// 100 PAY  97 

    0x8394fbbb,// 101 PAY  98 

    0x802e085f,// 102 PAY  99 

    0xc46260c2,// 103 PAY 100 

    0x4b4f91f2,// 104 PAY 101 

    0xdb198363,// 105 PAY 102 

    0xa6e3a985,// 106 PAY 103 

    0xe7d1cba9,// 107 PAY 104 

    0x72f4ea81,// 108 PAY 105 

    0x817d80f9,// 109 PAY 106 

    0x4fe1640b,// 110 PAY 107 

    0xe5bcf7db,// 111 PAY 108 

    0x2d4b5ccf,// 112 PAY 109 

    0xb42f789d,// 113 PAY 110 

    0x3d6c7e87,// 114 PAY 111 

    0x05d39a24,// 115 PAY 112 

    0x7ff85c3f,// 116 PAY 113 

    0x7fa8c008,// 117 PAY 114 

    0x64fa9dbd,// 118 PAY 115 

    0x9b283999,// 119 PAY 116 

    0x17d747c8,// 120 PAY 117 

    0x6d3010cf,// 121 PAY 118 

    0xbd64ef59,// 122 PAY 119 

    0xcf2584b8,// 123 PAY 120 

    0xa6f71df1,// 124 PAY 121 

    0xa5d62a27,// 125 PAY 122 

    0x5a990ce8,// 126 PAY 123 

    0xd3596f2e,// 127 PAY 124 

    0x1211a403,// 128 PAY 125 

    0xc7052fa6,// 129 PAY 126 

    0x6c2d4fae,// 130 PAY 127 

    0xc26415ce,// 131 PAY 128 

    0x63b2533f,// 132 PAY 129 

    0x5278ec07,// 133 PAY 130 

    0x9bc2dd95,// 134 PAY 131 

    0xf0f71070,// 135 PAY 132 

    0x9c441781,// 136 PAY 133 

    0x7dc7c72c,// 137 PAY 134 

    0xcdd4e09e,// 138 PAY 135 

    0x6a301ba8,// 139 PAY 136 

    0x563bbd96,// 140 PAY 137 

    0x21f480ea,// 141 PAY 138 

    0x120ae50e,// 142 PAY 139 

    0x2fc92c91,// 143 PAY 140 

    0x8a6bb559,// 144 PAY 141 

    0xe893da6c,// 145 PAY 142 

    0x169d7a51,// 146 PAY 143 

    0x3c151215,// 147 PAY 144 

    0xc59cf01d,// 148 PAY 145 

    0xabcde428,// 149 PAY 146 

    0x296ae72b,// 150 PAY 147 

    0xfc9a0a54,// 151 PAY 148 

    0xe151552c,// 152 PAY 149 

    0xd65d4636,// 153 PAY 150 

    0x2189b30b,// 154 PAY 151 

    0x2e21b1f9,// 155 PAY 152 

    0x4e6cae1b,// 156 PAY 153 

    0x1ec798d2,// 157 PAY 154 

    0x2ca30ac6,// 158 PAY 155 

    0x2aee1004,// 159 PAY 156 

    0xab49e0c7,// 160 PAY 157 

    0x04eee9eb,// 161 PAY 158 

    0xf5f70f3b,// 162 PAY 159 

    0xe7e70f05,// 163 PAY 160 

    0xab2e53a7,// 164 PAY 161 

    0x522b5c5c,// 165 PAY 162 

    0x34ac1867,// 166 PAY 163 

    0x2b6edfb7,// 167 PAY 164 

    0xdd6bf8bb,// 168 PAY 165 

    0x586d08dc,// 169 PAY 166 

    0x822601b9,// 170 PAY 167 

    0x6686e034,// 171 PAY 168 

    0x2de05a99,// 172 PAY 169 

    0x4910b895,// 173 PAY 170 

    0x8f5f55a0,// 174 PAY 171 

    0x6d6980eb,// 175 PAY 172 

    0xb92a244a,// 176 PAY 173 

    0x0d4b09cd,// 177 PAY 174 

    0x9fe58566,// 178 PAY 175 

    0x5570c5e7,// 179 PAY 176 

    0x8b860171,// 180 PAY 177 

    0x1e8d02c7,// 181 PAY 178 

    0xa8b1e197,// 182 PAY 179 

    0x07a93027,// 183 PAY 180 

    0xfbde6ca9,// 184 PAY 181 

    0x6df568f7,// 185 PAY 182 

    0x97ffef88,// 186 PAY 183 

    0x52c913b0,// 187 PAY 184 

    0x11f62d96,// 188 PAY 185 

    0x096288ee,// 189 PAY 186 

    0xd6535dee,// 190 PAY 187 

    0xc4984346,// 191 PAY 188 

    0x2b6aeff2,// 192 PAY 189 

    0x1005c251,// 193 PAY 190 

    0xb5c14e72,// 194 PAY 191 

    0xf34155c5,// 195 PAY 192 

    0x997649ee,// 196 PAY 193 

    0xba246d43,// 197 PAY 194 

    0xbf53a363,// 198 PAY 195 

    0x3b06cb2a,// 199 PAY 196 

    0xb6793aae,// 200 PAY 197 

    0xbe69d229,// 201 PAY 198 

    0x4ae86a75,// 202 PAY 199 

    0x09786b71,// 203 PAY 200 

    0xda6a5f82,// 204 PAY 201 

    0x0dd7578e,// 205 PAY 202 

    0xfd415725,// 206 PAY 203 

    0x5ec70250,// 207 PAY 204 

    0xf70f6cd7,// 208 PAY 205 

    0x8d6b4f58,// 209 PAY 206 

    0x44c2e3e9,// 210 PAY 207 

    0x7e83781b,// 211 PAY 208 

    0x3575d999,// 212 PAY 209 

    0x80dad551,// 213 PAY 210 

    0x314b6970,// 214 PAY 211 

    0x4eae66c4,// 215 PAY 212 

    0x8e6e6e30,// 216 PAY 213 

    0x223e8119,// 217 PAY 214 

    0xb2a71fad,// 218 PAY 215 

    0x92830235,// 219 PAY 216 

    0x26df597a,// 220 PAY 217 

    0x5f3ffa04,// 221 PAY 218 

    0x77c66c9f,// 222 PAY 219 

    0x83f8dfcd,// 223 PAY 220 

    0x62dc6726,// 224 PAY 221 

    0x2f63cf59,// 225 PAY 222 

    0xc6907781,// 226 PAY 223 

    0xce7b21e0,// 227 PAY 224 

    0xcbecd441,// 228 PAY 225 

    0xd1671d4d,// 229 PAY 226 

    0x412e7887,// 230 PAY 227 

    0xe848b328,// 231 PAY 228 

    0xb8dfda68,// 232 PAY 229 

    0x74092bbf,// 233 PAY 230 

    0x5c4b7278,// 234 PAY 231 

    0xda3893a6,// 235 PAY 232 

    0x50f0b5dc,// 236 PAY 233 

    0xa81a4c1d,// 237 PAY 234 

    0xf80e2140,// 238 PAY 235 

    0x1568af40,// 239 PAY 236 

    0x822aa14c,// 240 PAY 237 

    0x2eb5b090,// 241 PAY 238 

    0x73f9c600,// 242 PAY 239 

    0x2edeb817,// 243 PAY 240 

    0xb7c9e6d1,// 244 PAY 241 

    0x431a65bf,// 245 PAY 242 

    0xdccc4c3d,// 246 PAY 243 

    0x5c2c04e1,// 247 PAY 244 

    0x964a3e78,// 248 PAY 245 

    0x44899f15,// 249 PAY 246 

    0x79ad2d36,// 250 PAY 247 

    0x4347cbd3,// 251 PAY 248 

    0x2f799e75,// 252 PAY 249 

    0xf4d7e451,// 253 PAY 250 

    0xcf23f8bc,// 254 PAY 251 

    0x054ad6e5,// 255 PAY 252 

    0x5aa3ad77,// 256 PAY 253 

    0x038c1b35,// 257 PAY 254 

    0x1a1c3910,// 258 PAY 255 

    0xc4754ad7,// 259 PAY 256 

    0x3bf5e533,// 260 PAY 257 

    0x50bd0c75,// 261 PAY 258 

    0x5e695920,// 262 PAY 259 

    0x7e4132e9,// 263 PAY 260 

    0xde34cf9c,// 264 PAY 261 

    0xbff1c179,// 265 PAY 262 

    0x6fd00992,// 266 PAY 263 

    0x783581a7,// 267 PAY 264 

    0x68039b2d,// 268 PAY 265 

    0xa28d626d,// 269 PAY 266 

    0x7bf2622d,// 270 PAY 267 

    0x2cc291e1,// 271 PAY 268 

    0xa92c22cd,// 272 PAY 269 

    0x1e52a338,// 273 PAY 270 

    0x730e2840,// 274 PAY 271 

    0x9df094d0,// 275 PAY 272 

    0x7af7752a,// 276 PAY 273 

    0x151ebca9,// 277 PAY 274 

    0xbbfb9dab,// 278 PAY 275 

    0x582fa3aa,// 279 PAY 276 

    0xb23634b3,// 280 PAY 277 

    0xe7af718e,// 281 PAY 278 

    0xe02335e2,// 282 PAY 279 

    0xa4c73fa0,// 283 PAY 280 

    0x083326bf,// 284 PAY 281 

    0x600511d1,// 285 PAY 282 

    0x90d5ddea,// 286 PAY 283 

    0x935a764b,// 287 PAY 284 

    0xe90a10fb,// 288 PAY 285 

    0xdefb43a4,// 289 PAY 286 

    0x835f0807,// 290 PAY 287 

    0x7eec1ffd,// 291 PAY 288 

    0x22ca9b2c,// 292 PAY 289 

    0x3580b6a5,// 293 PAY 290 

    0x3493e01a,// 294 PAY 291 

    0xb16ccc8a,// 295 PAY 292 

    0x9091fca7,// 296 PAY 293 

    0x71530a22,// 297 PAY 294 

    0x27944278,// 298 PAY 295 

    0xcef0e022,// 299 PAY 296 

    0x827f48bc,// 300 PAY 297 

    0x111823df,// 301 PAY 298 

    0xa3507caf,// 302 PAY 299 

    0x1fc1ee0e,// 303 PAY 300 

    0x580f77ae,// 304 PAY 301 

    0x956dd2ee,// 305 PAY 302 

    0xdbc5f26a,// 306 PAY 303 

    0xdcf35c88,// 307 PAY 304 

    0x903de01a,// 308 PAY 305 

    0xc6507fc5,// 309 PAY 306 

    0x14504a15,// 310 PAY 307 

    0x08b5ff82,// 311 PAY 308 

    0xbead3c2a,// 312 PAY 309 

    0x338fe895,// 313 PAY 310 

    0x22a48abe,// 314 PAY 311 

    0x3173b804,// 315 PAY 312 

    0x783bcd18,// 316 PAY 313 

    0xafedd605,// 317 PAY 314 

    0x9b517307,// 318 PAY 315 

    0x0f5063b5,// 319 PAY 316 

    0xef6382a2,// 320 PAY 317 

    0x1d9199fb,// 321 PAY 318 

    0x6f68d327,// 322 PAY 319 

    0xf8c956f9,// 323 PAY 320 

    0x50ecb244,// 324 PAY 321 

    0x67e64923,// 325 PAY 322 

    0x6835e2d1,// 326 PAY 323 

    0xe8c4a062,// 327 PAY 324 

    0x25a38a3d,// 328 PAY 325 

    0xd063e954,// 329 PAY 326 

    0x063fd0ed,// 330 PAY 327 

    0x8ebed454,// 331 PAY 328 

    0x4d9aa94d,// 332 PAY 329 

    0xe1daef7a,// 333 PAY 330 

    0x444b6a30,// 334 PAY 331 

    0x16216454,// 335 PAY 332 

    0x3b86637d,// 336 PAY 333 

    0x5415788c,// 337 PAY 334 

    0xc4515ce1,// 338 PAY 335 

    0xb9a9ac7e,// 339 PAY 336 

    0xd76cd5df,// 340 PAY 337 

    0xc89b7206,// 341 PAY 338 

    0x64a47ba4,// 342 PAY 339 

    0xd45c2a95,// 343 PAY 340 

    0xf8a4d72e,// 344 PAY 341 

    0x708427ba,// 345 PAY 342 

    0x4f3a5afc,// 346 PAY 343 

    0x5ee29268,// 347 PAY 344 

    0x18c3c5f5,// 348 PAY 345 

    0xbc6c445c,// 349 PAY 346 

    0x55cf4a6a,// 350 PAY 347 

    0xa1e508e1,// 351 PAY 348 

    0x3702224e,// 352 PAY 349 

    0x06c1cb6c,// 353 PAY 350 

    0xa93e4868,// 354 PAY 351 

    0x9b0f0b65,// 355 PAY 352 

    0xdc4d00a8,// 356 PAY 353 

    0xabbba38c,// 357 PAY 354 

    0x78a9f629,// 358 PAY 355 

    0x8991132f,// 359 PAY 356 

    0x0afdf4ba,// 360 PAY 357 

    0x7caf2096,// 361 PAY 358 

    0xbb1dd3e5,// 362 PAY 359 

    0xe50bece3,// 363 PAY 360 

    0xca4ab335,// 364 PAY 361 

    0x785a6de5,// 365 PAY 362 

    0x12e582d3,// 366 PAY 363 

    0xdc9bc2d9,// 367 PAY 364 

    0x7132e1bf,// 368 PAY 365 

    0xad2f47b7,// 369 PAY 366 

    0xbe17550d,// 370 PAY 367 

    0xe4eb31b2,// 371 PAY 368 

    0x60d23ae7,// 372 PAY 369 

    0x66ed9057,// 373 PAY 370 

    0xa72b842f,// 374 PAY 371 

    0xeb95cf87,// 375 PAY 372 

    0x5182dd5d,// 376 PAY 373 

    0x12884026,// 377 PAY 374 

    0xbb6487a9,// 378 PAY 375 

    0x97ccb6b2,// 379 PAY 376 

    0x16661a23,// 380 PAY 377 

    0x3c1e15c2,// 381 PAY 378 

    0x45fafc43,// 382 PAY 379 

    0xa293db1f,// 383 PAY 380 

    0x14b4e0b2,// 384 PAY 381 

    0x9b2557e8,// 385 PAY 382 

    0x282ece68,// 386 PAY 383 

    0x13760bf5,// 387 PAY 384 

    0xb313524f,// 388 PAY 385 

    0x79d3f55c,// 389 PAY 386 

    0xde1880a5,// 390 PAY 387 

    0xf6e726b8,// 391 PAY 388 

    0x887b89b9,// 392 PAY 389 

    0x8ee2c0f2,// 393 PAY 390 

    0x1a4b4595,// 394 PAY 391 

    0x96530197,// 395 PAY 392 

    0xc66aba77,// 396 PAY 393 

    0x51fd56a6,// 397 PAY 394 

    0x4e3856c5,// 398 PAY 395 

    0x005a5068,// 399 PAY 396 

    0xa5b2a02b,// 400 PAY 397 

    0xe3d306e3,// 401 PAY 398 

    0xaa75df5f,// 402 PAY 399 

    0x10313824,// 403 PAY 400 

    0x6149e193,// 404 PAY 401 

    0x956634ac,// 405 PAY 402 

    0x2e54f42e,// 406 PAY 403 

    0xe62eb8c5,// 407 PAY 404 

    0x7e182f69,// 408 PAY 405 

    0xafcf526c,// 409 PAY 406 

    0x9fcdcf03,// 410 PAY 407 

    0x9ba0a6bb,// 411 PAY 408 

    0xcffe9fa8,// 412 PAY 409 

    0x1faffa24,// 413 PAY 410 

    0x222679c9,// 414 PAY 411 

    0x83e6ea62,// 415 PAY 412 

    0xd006fe62,// 416 PAY 413 

    0xfb2c0a70,// 417 PAY 414 

    0x190ae10b,// 418 PAY 415 

    0x80dec88d,// 419 PAY 416 

    0x221649b7,// 420 PAY 417 

    0x13995c78,// 421 PAY 418 

    0xe03a8277,// 422 PAY 419 

    0x36951ac0,// 423 PAY 420 

    0xf56d8882,// 424 PAY 421 

    0x7575a821,// 425 PAY 422 

    0x4b807229,// 426 PAY 423 

    0x573d5822,// 427 PAY 424 

    0xb7c428ae,// 428 PAY 425 

    0x27fd088b,// 429 PAY 426 

    0x46f9cf45,// 430 PAY 427 

    0x375ae14d,// 431 PAY 428 

    0x917a8550,// 432 PAY 429 

    0xa9dfd532,// 433 PAY 430 

    0x89700a4f,// 434 PAY 431 

    0x4b9a92eb,// 435 PAY 432 

    0xdc990bd1,// 436 PAY 433 

    0x40d0fa29,// 437 PAY 434 

    0x4c000000,// 438 PAY 435 

/// HASH is  12 bytes 

    0xe4eb31b2,// 439 HSH   1 

    0x60d23ae7,// 440 HSH   2 

    0x66ed9057,// 441 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 201 

/// STA pkt_idx        : 14 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x5 

    0x003905c9 // 442 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 72 (0x48) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt97_tmpl[] = {
    0x0c010048,// 1 CCH   1 

/// ECH cache_idx      : 0x02 

/// ECH pdu_tag        : 0x00 

    0x00020000,// 2 ECH   1 

/// BDA is 439 words. 

/// BDA size     is 1751 (0x6d7) 

/// BDA id       is 0x2790 

    0x06d72790,// 3 BDA   1 

/// PAY Generic Data size   : 1751 byte(s) 

/// PAD Buffer Data Pad size   : 1 byte(s) 

    0x16a05efa,// 4 PAY   1 

    0x6dc4813a,// 5 PAY   2 

    0x35cb97b1,// 6 PAY   3 

    0x44e733f6,// 7 PAY   4 

    0xe87fbfaa,// 8 PAY   5 

    0xc2b11dd5,// 9 PAY   6 

    0xc8dfdc7d,// 10 PAY   7 

    0x000885df,// 11 PAY   8 

    0x57e08859,// 12 PAY   9 

    0x3ab70bb5,// 13 PAY  10 

    0x27dbc399,// 14 PAY  11 

    0xeb513ad1,// 15 PAY  12 

    0x1a7cb952,// 16 PAY  13 

    0x5f8d57c0,// 17 PAY  14 

    0xf147045b,// 18 PAY  15 

    0x10c77a57,// 19 PAY  16 

    0xa15ac79f,// 20 PAY  17 

    0x67e20d65,// 21 PAY  18 

    0x1604a2c8,// 22 PAY  19 

    0x26e95a61,// 23 PAY  20 

    0x00d58f36,// 24 PAY  21 

    0xaa13d577,// 25 PAY  22 

    0xf0444d2a,// 26 PAY  23 

    0x940e1598,// 27 PAY  24 

    0x768ddeaa,// 28 PAY  25 

    0xa92fe36f,// 29 PAY  26 

    0xc73a7324,// 30 PAY  27 

    0x568889ba,// 31 PAY  28 

    0x5c42f048,// 32 PAY  29 

    0xf2214022,// 33 PAY  30 

    0x0a920082,// 34 PAY  31 

    0x6833f7c5,// 35 PAY  32 

    0xcf896962,// 36 PAY  33 

    0xa35315dd,// 37 PAY  34 

    0xc1d740e1,// 38 PAY  35 

    0x2e23fd8d,// 39 PAY  36 

    0x287aaaa4,// 40 PAY  37 

    0x69303cac,// 41 PAY  38 

    0xa4936792,// 42 PAY  39 

    0x16ebe7a9,// 43 PAY  40 

    0x4c6eb84f,// 44 PAY  41 

    0xb0bd7fc9,// 45 PAY  42 

    0xf7987eb5,// 46 PAY  43 

    0xd93723bc,// 47 PAY  44 

    0x04803ef0,// 48 PAY  45 

    0x3426a841,// 49 PAY  46 

    0x540e891c,// 50 PAY  47 

    0x50b9dce1,// 51 PAY  48 

    0xf26dd7fe,// 52 PAY  49 

    0x4137e680,// 53 PAY  50 

    0xf4203463,// 54 PAY  51 

    0xc54724cb,// 55 PAY  52 

    0x138dadcc,// 56 PAY  53 

    0xeccc332e,// 57 PAY  54 

    0x82df3ee0,// 58 PAY  55 

    0xca8b7ce7,// 59 PAY  56 

    0x877bc784,// 60 PAY  57 

    0xd6bf4874,// 61 PAY  58 

    0x0d7449b8,// 62 PAY  59 

    0x1ddab077,// 63 PAY  60 

    0x1f1695da,// 64 PAY  61 

    0x6bacd4b7,// 65 PAY  62 

    0x575a48eb,// 66 PAY  63 

    0xc9501dfd,// 67 PAY  64 

    0xde8dea91,// 68 PAY  65 

    0x8a0ba11f,// 69 PAY  66 

    0x9e8b58ea,// 70 PAY  67 

    0xad3fb74e,// 71 PAY  68 

    0x7adf36eb,// 72 PAY  69 

    0x4172da5c,// 73 PAY  70 

    0x50f2c1c5,// 74 PAY  71 

    0xa8ca2975,// 75 PAY  72 

    0xe57c8560,// 76 PAY  73 

    0xfb8b091d,// 77 PAY  74 

    0xd4583008,// 78 PAY  75 

    0xbc2e80c9,// 79 PAY  76 

    0x80de601f,// 80 PAY  77 

    0xebd969c7,// 81 PAY  78 

    0xa8657d4e,// 82 PAY  79 

    0x748cb8f4,// 83 PAY  80 

    0x352c9415,// 84 PAY  81 

    0xe6a0d6f6,// 85 PAY  82 

    0x34ec8a9e,// 86 PAY  83 

    0xff28f034,// 87 PAY  84 

    0xa4005a9b,// 88 PAY  85 

    0x81c3a4a3,// 89 PAY  86 

    0x97ebd006,// 90 PAY  87 

    0xd09008a1,// 91 PAY  88 

    0xded7120e,// 92 PAY  89 

    0x7e8c8e29,// 93 PAY  90 

    0x5e2887e4,// 94 PAY  91 

    0x49ae1614,// 95 PAY  92 

    0x90d6ed08,// 96 PAY  93 

    0xda8f1709,// 97 PAY  94 

    0x142060e5,// 98 PAY  95 

    0x422f8feb,// 99 PAY  96 

    0xbb652e1e,// 100 PAY  97 

    0x43844b46,// 101 PAY  98 

    0xfc34a0c0,// 102 PAY  99 

    0x1f420ac7,// 103 PAY 100 

    0xb6bf9f7f,// 104 PAY 101 

    0xb3427c57,// 105 PAY 102 

    0xd2c7981a,// 106 PAY 103 

    0xe97aacc4,// 107 PAY 104 

    0x4118a76c,// 108 PAY 105 

    0x4e0f75a0,// 109 PAY 106 

    0xb5f9b464,// 110 PAY 107 

    0x84181d62,// 111 PAY 108 

    0x3d24e1d3,// 112 PAY 109 

    0x46d6b102,// 113 PAY 110 

    0xdc336ef6,// 114 PAY 111 

    0xc2c50d32,// 115 PAY 112 

    0xbad52e55,// 116 PAY 113 

    0x43482524,// 117 PAY 114 

    0x75e6a7da,// 118 PAY 115 

    0xf094bc26,// 119 PAY 116 

    0xb0bafbd8,// 120 PAY 117 

    0x5814c6c9,// 121 PAY 118 

    0x8a8d4966,// 122 PAY 119 

    0x57ad6893,// 123 PAY 120 

    0x21c99cc1,// 124 PAY 121 

    0xf2a55dc0,// 125 PAY 122 

    0x75de5ffd,// 126 PAY 123 

    0xe856a2f7,// 127 PAY 124 

    0x8e582df8,// 128 PAY 125 

    0x649c3cd0,// 129 PAY 126 

    0xf97100ef,// 130 PAY 127 

    0xa245db29,// 131 PAY 128 

    0xb9ca7ac2,// 132 PAY 129 

    0x3ce71d1e,// 133 PAY 130 

    0x658f8a80,// 134 PAY 131 

    0x3693ae67,// 135 PAY 132 

    0x83bac970,// 136 PAY 133 

    0x8be0f463,// 137 PAY 134 

    0xdb6a3a5f,// 138 PAY 135 

    0x12f46ad3,// 139 PAY 136 

    0xd99861c5,// 140 PAY 137 

    0x0ba69190,// 141 PAY 138 

    0x63793009,// 142 PAY 139 

    0x524060c4,// 143 PAY 140 

    0x96e941d5,// 144 PAY 141 

    0xa6dc88d0,// 145 PAY 142 

    0xe0e07580,// 146 PAY 143 

    0xcd232e10,// 147 PAY 144 

    0x4d1bdb2a,// 148 PAY 145 

    0xd2fc3f4e,// 149 PAY 146 

    0xa44af6f3,// 150 PAY 147 

    0x84d1751c,// 151 PAY 148 

    0x0c594b12,// 152 PAY 149 

    0x3eb8e65c,// 153 PAY 150 

    0x82f2eb8d,// 154 PAY 151 

    0xb3c97988,// 155 PAY 152 

    0xf365c076,// 156 PAY 153 

    0x1c20e940,// 157 PAY 154 

    0xf49f40ac,// 158 PAY 155 

    0xaba1c7d9,// 159 PAY 156 

    0xd796a9e3,// 160 PAY 157 

    0xfee1fa3c,// 161 PAY 158 

    0x2d1c6011,// 162 PAY 159 

    0xe5659470,// 163 PAY 160 

    0xbdb6f87b,// 164 PAY 161 

    0x80e17903,// 165 PAY 162 

    0x077ccf19,// 166 PAY 163 

    0xa65743b3,// 167 PAY 164 

    0x2d80c232,// 168 PAY 165 

    0x95ff373d,// 169 PAY 166 

    0xe80cf547,// 170 PAY 167 

    0x689e396a,// 171 PAY 168 

    0x23fecd78,// 172 PAY 169 

    0xa1058259,// 173 PAY 170 

    0x90bc80ef,// 174 PAY 171 

    0xe647e02a,// 175 PAY 172 

    0x5755915c,// 176 PAY 173 

    0x5a463c78,// 177 PAY 174 

    0x1f7bbc76,// 178 PAY 175 

    0x8a121896,// 179 PAY 176 

    0x41dbccfb,// 180 PAY 177 

    0x53375faf,// 181 PAY 178 

    0xf12c0ced,// 182 PAY 179 

    0x3c574360,// 183 PAY 180 

    0x0bbd702f,// 184 PAY 181 

    0x913400fe,// 185 PAY 182 

    0x69e0dbff,// 186 PAY 183 

    0x30017404,// 187 PAY 184 

    0xf43e2d6e,// 188 PAY 185 

    0xe4c53719,// 189 PAY 186 

    0x2b2c3893,// 190 PAY 187 

    0x5158af56,// 191 PAY 188 

    0x295e7502,// 192 PAY 189 

    0x3b055fe7,// 193 PAY 190 

    0xf77e3c4f,// 194 PAY 191 

    0x06f2b468,// 195 PAY 192 

    0x0332eb10,// 196 PAY 193 

    0xaef56b70,// 197 PAY 194 

    0x36821767,// 198 PAY 195 

    0xe9d5f2ee,// 199 PAY 196 

    0xcb25fbe6,// 200 PAY 197 

    0xf71be7d8,// 201 PAY 198 

    0xcc52a62b,// 202 PAY 199 

    0x6101902f,// 203 PAY 200 

    0xcbe9a035,// 204 PAY 201 

    0xe155cd16,// 205 PAY 202 

    0xece9e1bb,// 206 PAY 203 

    0xe2f54fe1,// 207 PAY 204 

    0x604c0eef,// 208 PAY 205 

    0xe8f7ad6b,// 209 PAY 206 

    0x0077e7c8,// 210 PAY 207 

    0x527057f3,// 211 PAY 208 

    0x4b38ca12,// 212 PAY 209 

    0xd501f84f,// 213 PAY 210 

    0x8b91551e,// 214 PAY 211 

    0x0b8afbe3,// 215 PAY 212 

    0x0a2bd3ab,// 216 PAY 213 

    0xfd847935,// 217 PAY 214 

    0xce48d122,// 218 PAY 215 

    0x7e36fdd8,// 219 PAY 216 

    0x740e5873,// 220 PAY 217 

    0xb3fc9c7f,// 221 PAY 218 

    0x90cd4a9e,// 222 PAY 219 

    0x9363f73e,// 223 PAY 220 

    0x7227daa0,// 224 PAY 221 

    0x75afadc7,// 225 PAY 222 

    0xa5f15dbb,// 226 PAY 223 

    0x5d9983bd,// 227 PAY 224 

    0xbdb2d6ae,// 228 PAY 225 

    0x973498e5,// 229 PAY 226 

    0xf337c2d4,// 230 PAY 227 

    0xacfc069c,// 231 PAY 228 

    0x4360bd79,// 232 PAY 229 

    0x3807eb1c,// 233 PAY 230 

    0x4c1d7388,// 234 PAY 231 

    0xb8591b31,// 235 PAY 232 

    0x14bf7a6c,// 236 PAY 233 

    0xb747f805,// 237 PAY 234 

    0x105a0965,// 238 PAY 235 

    0x94e6dde4,// 239 PAY 236 

    0x9860d57d,// 240 PAY 237 

    0xebb85321,// 241 PAY 238 

    0xa8a5b648,// 242 PAY 239 

    0x878eb8af,// 243 PAY 240 

    0x8546793f,// 244 PAY 241 

    0x7b93296c,// 245 PAY 242 

    0x73e59e29,// 246 PAY 243 

    0xd220dbd0,// 247 PAY 244 

    0xb6d4d1d8,// 248 PAY 245 

    0x859429b7,// 249 PAY 246 

    0x9239a833,// 250 PAY 247 

    0x5fd7c51a,// 251 PAY 248 

    0x2c323636,// 252 PAY 249 

    0x5f62f77e,// 253 PAY 250 

    0xc5f53b06,// 254 PAY 251 

    0x64d5a30a,// 255 PAY 252 

    0x0184e02c,// 256 PAY 253 

    0x90dbc747,// 257 PAY 254 

    0xa7df4a5c,// 258 PAY 255 

    0x36cc0499,// 259 PAY 256 

    0xaf4053fb,// 260 PAY 257 

    0x89a94f82,// 261 PAY 258 

    0xf8311585,// 262 PAY 259 

    0x68217f22,// 263 PAY 260 

    0xf942c7df,// 264 PAY 261 

    0x93353bd8,// 265 PAY 262 

    0x0bb95783,// 266 PAY 263 

    0x33329352,// 267 PAY 264 

    0xe296114e,// 268 PAY 265 

    0xbbc35248,// 269 PAY 266 

    0x10c6b54a,// 270 PAY 267 

    0xe4a1e6d5,// 271 PAY 268 

    0x68fbc065,// 272 PAY 269 

    0xc667d407,// 273 PAY 270 

    0xac267056,// 274 PAY 271 

    0x7273ae8c,// 275 PAY 272 

    0xbe48d815,// 276 PAY 273 

    0x47087ad9,// 277 PAY 274 

    0x770a3e20,// 278 PAY 275 

    0x5c5ff138,// 279 PAY 276 

    0x634e47e2,// 280 PAY 277 

    0x2adcd678,// 281 PAY 278 

    0x23d088a3,// 282 PAY 279 

    0x408b065c,// 283 PAY 280 

    0x8f940ad3,// 284 PAY 281 

    0x9fcb1452,// 285 PAY 282 

    0xe1632bb7,// 286 PAY 283 

    0x61f9fc0f,// 287 PAY 284 

    0x3902b81e,// 288 PAY 285 

    0x42956046,// 289 PAY 286 

    0x146acf09,// 290 PAY 287 

    0x36c6c894,// 291 PAY 288 

    0x4d245423,// 292 PAY 289 

    0xd243d37d,// 293 PAY 290 

    0x8e3155dc,// 294 PAY 291 

    0x5efe4daf,// 295 PAY 292 

    0xbc323563,// 296 PAY 293 

    0x921ebef6,// 297 PAY 294 

    0x23c9a6a1,// 298 PAY 295 

    0x594fee8d,// 299 PAY 296 

    0x9e7b4db6,// 300 PAY 297 

    0x03669830,// 301 PAY 298 

    0x80504543,// 302 PAY 299 

    0x82d7d5f2,// 303 PAY 300 

    0xef41fbf2,// 304 PAY 301 

    0x988d3684,// 305 PAY 302 

    0x8630e135,// 306 PAY 303 

    0xce0e51f2,// 307 PAY 304 

    0x9de90ce1,// 308 PAY 305 

    0xbc090e90,// 309 PAY 306 

    0x6a870e06,// 310 PAY 307 

    0xd2d3c5f2,// 311 PAY 308 

    0x069674a1,// 312 PAY 309 

    0xf75e1eb1,// 313 PAY 310 

    0x2a443bfa,// 314 PAY 311 

    0x7de563e7,// 315 PAY 312 

    0xe9bb3536,// 316 PAY 313 

    0x775dc6bf,// 317 PAY 314 

    0x96d8b50b,// 318 PAY 315 

    0xb8cf9dfc,// 319 PAY 316 

    0xa1fa7459,// 320 PAY 317 

    0x02ae7399,// 321 PAY 318 

    0x55b408b4,// 322 PAY 319 

    0x1cf911f2,// 323 PAY 320 

    0x587eccde,// 324 PAY 321 

    0xdae7b46e,// 325 PAY 322 

    0x8bd75d3d,// 326 PAY 323 

    0x299b9949,// 327 PAY 324 

    0x6767193c,// 328 PAY 325 

    0xa55fea92,// 329 PAY 326 

    0x1d75b419,// 330 PAY 327 

    0xef07ddac,// 331 PAY 328 

    0xef728682,// 332 PAY 329 

    0xcf20752c,// 333 PAY 330 

    0x66ceb48c,// 334 PAY 331 

    0xe5b97d0c,// 335 PAY 332 

    0x129920cc,// 336 PAY 333 

    0x730f66ac,// 337 PAY 334 

    0x1af8c6da,// 338 PAY 335 

    0x12fe732f,// 339 PAY 336 

    0xdb0126c6,// 340 PAY 337 

    0x433cc20a,// 341 PAY 338 

    0x704d208d,// 342 PAY 339 

    0x0ee44d9b,// 343 PAY 340 

    0x1e048ba3,// 344 PAY 341 

    0xcc41dbf0,// 345 PAY 342 

    0x647fb5ff,// 346 PAY 343 

    0xb780c0f6,// 347 PAY 344 

    0x194bb2c9,// 348 PAY 345 

    0xfee6e1f4,// 349 PAY 346 

    0x82af2361,// 350 PAY 347 

    0xc6411931,// 351 PAY 348 

    0xd5e0ce3d,// 352 PAY 349 

    0x446adc0c,// 353 PAY 350 

    0x74246323,// 354 PAY 351 

    0x73051e22,// 355 PAY 352 

    0x39dfbe40,// 356 PAY 353 

    0x7ddf44a9,// 357 PAY 354 

    0x47bcd591,// 358 PAY 355 

    0xa5b01545,// 359 PAY 356 

    0xa01bba42,// 360 PAY 357 

    0x4e531e52,// 361 PAY 358 

    0x1cd506c0,// 362 PAY 359 

    0xccefab7f,// 363 PAY 360 

    0xaeb15fdd,// 364 PAY 361 

    0x73a1666d,// 365 PAY 362 

    0x87b10c6e,// 366 PAY 363 

    0x44f9183a,// 367 PAY 364 

    0x0f8d7e3d,// 368 PAY 365 

    0x7d07c06d,// 369 PAY 366 

    0x08155204,// 370 PAY 367 

    0xef0b9c52,// 371 PAY 368 

    0x66ba4a77,// 372 PAY 369 

    0x692e84a2,// 373 PAY 370 

    0xfa9d1c40,// 374 PAY 371 

    0x13737fb8,// 375 PAY 372 

    0xda501848,// 376 PAY 373 

    0xcc579217,// 377 PAY 374 

    0x8480ef16,// 378 PAY 375 

    0x1827ac61,// 379 PAY 376 

    0xdd142067,// 380 PAY 377 

    0x65efc58b,// 381 PAY 378 

    0xcdd68834,// 382 PAY 379 

    0xb5aa2256,// 383 PAY 380 

    0x69dd1efe,// 384 PAY 381 

    0x7f4f37ae,// 385 PAY 382 

    0x3dab03fb,// 386 PAY 383 

    0xc79084a1,// 387 PAY 384 

    0x9fd734d7,// 388 PAY 385 

    0xfb5b4347,// 389 PAY 386 

    0x4c1d3093,// 390 PAY 387 

    0x94d8729d,// 391 PAY 388 

    0x94f04664,// 392 PAY 389 

    0xf1d2ba88,// 393 PAY 390 

    0x41d0c325,// 394 PAY 391 

    0x5384e0e2,// 395 PAY 392 

    0x8acdbf89,// 396 PAY 393 

    0x2601ea26,// 397 PAY 394 

    0x1b3b6877,// 398 PAY 395 

    0x2fe64941,// 399 PAY 396 

    0xc37f23ee,// 400 PAY 397 

    0x96381f99,// 401 PAY 398 

    0xebf46bc4,// 402 PAY 399 

    0x24e772f5,// 403 PAY 400 

    0x5a51749e,// 404 PAY 401 

    0x4f5441db,// 405 PAY 402 

    0x8df76024,// 406 PAY 403 

    0xa23825ec,// 407 PAY 404 

    0x45fdaacb,// 408 PAY 405 

    0xb0016c9e,// 409 PAY 406 

    0x97c4f7d7,// 410 PAY 407 

    0x34f2d4a1,// 411 PAY 408 

    0xcad734e8,// 412 PAY 409 

    0x7b374631,// 413 PAY 410 

    0x5c6c0664,// 414 PAY 411 

    0x15b601c0,// 415 PAY 412 

    0x9a7983cd,// 416 PAY 413 

    0x02101f76,// 417 PAY 414 

    0x5be5913b,// 418 PAY 415 

    0xacdd4f03,// 419 PAY 416 

    0xd507f7d0,// 420 PAY 417 

    0x5f1b30c3,// 421 PAY 418 

    0x2f997104,// 422 PAY 419 

    0x1b5799e1,// 423 PAY 420 

    0x39da1d86,// 424 PAY 421 

    0x30670bf4,// 425 PAY 422 

    0xf12ddc6d,// 426 PAY 423 

    0x221e8ac0,// 427 PAY 424 

    0x72401170,// 428 PAY 425 

    0x0772a3d5,// 429 PAY 426 

    0xced63b8e,// 430 PAY 427 

    0x5a5e1348,// 431 PAY 428 

    0xa6288f6f,// 432 PAY 429 

    0x33505010,// 433 PAY 430 

    0x52afcc24,// 434 PAY 431 

    0x42d24545,// 435 PAY 432 

    0xd300a2a5,// 436 PAY 433 

    0xdace40bc,// 437 PAY 434 

    0x4f306afc,// 438 PAY 435 

    0x421dda55,// 439 PAY 436 

    0xf633c19a,// 440 PAY 437 

    0x37b13e00,// 441 PAY 438 

/// HASH is  12 bytes 

    0xbc2e80c9,// 442 HSH   1 

    0x80de601f,// 443 HSH   2 

    0xebd969c7,// 444 HSH   3 

/// STA is 1 words. 

/// STA num_pkts       : 116 

/// STA pkt_idx        : 248 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xbf 

    0x03e1bf74 // 445 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt98_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x09 

/// ECH pdu_tag        : 0x00 

    0x00090000,// 2 ECH   1 

/// BDA is 366 words. 

/// BDA size     is 1458 (0x5b2) 

/// BDA id       is 0xf658 

    0x05b2f658,// 3 BDA   1 

/// PAY Generic Data size   : 1458 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xb0d7e3b7,// 4 PAY   1 

    0x271b9800,// 5 PAY   2 

    0x70937ed2,// 6 PAY   3 

    0x2aba064e,// 7 PAY   4 

    0x4ce67e14,// 8 PAY   5 

    0x419120c4,// 9 PAY   6 

    0x3de41183,// 10 PAY   7 

    0xa50f6b9d,// 11 PAY   8 

    0x03ddd847,// 12 PAY   9 

    0x02e7be69,// 13 PAY  10 

    0x325cd977,// 14 PAY  11 

    0xbb79d4a4,// 15 PAY  12 

    0x96489360,// 16 PAY  13 

    0x032476ac,// 17 PAY  14 

    0x4f3017ab,// 18 PAY  15 

    0x6630705c,// 19 PAY  16 

    0x3283cd09,// 20 PAY  17 

    0x3c1f4381,// 21 PAY  18 

    0x77429563,// 22 PAY  19 

    0xf15c86b6,// 23 PAY  20 

    0xc8e74134,// 24 PAY  21 

    0x2e2e5c5d,// 25 PAY  22 

    0x3a1fe417,// 26 PAY  23 

    0xfe913f81,// 27 PAY  24 

    0xea852cb2,// 28 PAY  25 

    0xf8d63e35,// 29 PAY  26 

    0xd9b2d014,// 30 PAY  27 

    0x5a574b86,// 31 PAY  28 

    0x8dc9198e,// 32 PAY  29 

    0xe44dcf95,// 33 PAY  30 

    0x267977fc,// 34 PAY  31 

    0x46068993,// 35 PAY  32 

    0xa398dcc1,// 36 PAY  33 

    0x0d03ea6d,// 37 PAY  34 

    0xb115c9bc,// 38 PAY  35 

    0x8b1fab84,// 39 PAY  36 

    0x90b88ef6,// 40 PAY  37 

    0x74d88a46,// 41 PAY  38 

    0x0d4a99db,// 42 PAY  39 

    0xb44c26b7,// 43 PAY  40 

    0xadc62553,// 44 PAY  41 

    0x0840bf03,// 45 PAY  42 

    0x68ae751f,// 46 PAY  43 

    0x0ac482d6,// 47 PAY  44 

    0x21756791,// 48 PAY  45 

    0x2e137ed6,// 49 PAY  46 

    0x4456c879,// 50 PAY  47 

    0xe0ab36cc,// 51 PAY  48 

    0x84a99e66,// 52 PAY  49 

    0xf23a7a3c,// 53 PAY  50 

    0xbb2b3d1d,// 54 PAY  51 

    0xeb2acfca,// 55 PAY  52 

    0x98994744,// 56 PAY  53 

    0xeb32f3d8,// 57 PAY  54 

    0xb8f379dc,// 58 PAY  55 

    0xa1493002,// 59 PAY  56 

    0xf0cb93a2,// 60 PAY  57 

    0x100e808a,// 61 PAY  58 

    0xeffdbb3f,// 62 PAY  59 

    0x6195f9ba,// 63 PAY  60 

    0xab174345,// 64 PAY  61 

    0x6ded4022,// 65 PAY  62 

    0xcc474a31,// 66 PAY  63 

    0x8f6610a5,// 67 PAY  64 

    0x540ff879,// 68 PAY  65 

    0x5a870a2f,// 69 PAY  66 

    0x9b6c1e91,// 70 PAY  67 

    0x18845a8c,// 71 PAY  68 

    0xf1c9020f,// 72 PAY  69 

    0xce6484c5,// 73 PAY  70 

    0x370942fd,// 74 PAY  71 

    0xf1d0d16a,// 75 PAY  72 

    0x98742948,// 76 PAY  73 

    0xa8494236,// 77 PAY  74 

    0x5921e0a9,// 78 PAY  75 

    0xdf55cbaf,// 79 PAY  76 

    0x07e8d849,// 80 PAY  77 

    0x41c8283c,// 81 PAY  78 

    0x94bda462,// 82 PAY  79 

    0xade3082f,// 83 PAY  80 

    0x7a11e571,// 84 PAY  81 

    0x230a6f04,// 85 PAY  82 

    0xf0308cee,// 86 PAY  83 

    0xf6ae3de9,// 87 PAY  84 

    0x8026ad92,// 88 PAY  85 

    0x35c4dbdf,// 89 PAY  86 

    0xcc9d4db4,// 90 PAY  87 

    0x5f65a54b,// 91 PAY  88 

    0xb6fbe29a,// 92 PAY  89 

    0x56e6e2c5,// 93 PAY  90 

    0x3a7a5458,// 94 PAY  91 

    0x2d6923bc,// 95 PAY  92 

    0xd840b95e,// 96 PAY  93 

    0x275511a8,// 97 PAY  94 

    0x284c7b2f,// 98 PAY  95 

    0xe3451acf,// 99 PAY  96 

    0x6bcf11dd,// 100 PAY  97 

    0x138f6e5e,// 101 PAY  98 

    0xa55536d8,// 102 PAY  99 

    0x6a781aff,// 103 PAY 100 

    0xfcd5c808,// 104 PAY 101 

    0xd2c3064c,// 105 PAY 102 

    0xd8d4e51f,// 106 PAY 103 

    0x6a10bed1,// 107 PAY 104 

    0xf10b913e,// 108 PAY 105 

    0x093812cb,// 109 PAY 106 

    0x3c401bee,// 110 PAY 107 

    0xa3fa66e2,// 111 PAY 108 

    0xf58a833c,// 112 PAY 109 

    0x39831318,// 113 PAY 110 

    0x1d9e7f46,// 114 PAY 111 

    0x99628061,// 115 PAY 112 

    0xea0b2363,// 116 PAY 113 

    0x4c372012,// 117 PAY 114 

    0x893f0d2c,// 118 PAY 115 

    0x01da2231,// 119 PAY 116 

    0x0f718617,// 120 PAY 117 

    0x6bfd23ca,// 121 PAY 118 

    0x0d6fe1e8,// 122 PAY 119 

    0xcdb06a74,// 123 PAY 120 

    0x11aa13b5,// 124 PAY 121 

    0x26bf4655,// 125 PAY 122 

    0x5353e4f3,// 126 PAY 123 

    0x1aa5165d,// 127 PAY 124 

    0xf5736afd,// 128 PAY 125 

    0x8c5fee6e,// 129 PAY 126 

    0xf06e2253,// 130 PAY 127 

    0x0d45f193,// 131 PAY 128 

    0xa76ff9e0,// 132 PAY 129 

    0xd669a33f,// 133 PAY 130 

    0x69dc0ae8,// 134 PAY 131 

    0xad876e4d,// 135 PAY 132 

    0x77a11491,// 136 PAY 133 

    0x58165a71,// 137 PAY 134 

    0xccb8fc61,// 138 PAY 135 

    0x2a263b6b,// 139 PAY 136 

    0x7b2a2b88,// 140 PAY 137 

    0x00ca1399,// 141 PAY 138 

    0x7fd047ff,// 142 PAY 139 

    0x49b83ae2,// 143 PAY 140 

    0x3fba8a0c,// 144 PAY 141 

    0x6a11d66f,// 145 PAY 142 

    0x3598bbef,// 146 PAY 143 

    0xbe7f8f20,// 147 PAY 144 

    0x4e817355,// 148 PAY 145 

    0x40fc62c7,// 149 PAY 146 

    0xf3ac6af5,// 150 PAY 147 

    0x3178d125,// 151 PAY 148 

    0xa9b83d0d,// 152 PAY 149 

    0xd20f6e97,// 153 PAY 150 

    0xed838a49,// 154 PAY 151 

    0x1a23b11d,// 155 PAY 152 

    0x1e747f1b,// 156 PAY 153 

    0x3da0eb12,// 157 PAY 154 

    0x0767a360,// 158 PAY 155 

    0x0ea8d6fb,// 159 PAY 156 

    0x2cf96206,// 160 PAY 157 

    0x162f22b5,// 161 PAY 158 

    0x61710826,// 162 PAY 159 

    0x4082c621,// 163 PAY 160 

    0xe773cdcc,// 164 PAY 161 

    0x6fd63663,// 165 PAY 162 

    0x7fdaee88,// 166 PAY 163 

    0xb1db9175,// 167 PAY 164 

    0xc7b27339,// 168 PAY 165 

    0xaf295ebe,// 169 PAY 166 

    0x4a04e234,// 170 PAY 167 

    0xaeaf94b3,// 171 PAY 168 

    0x150a6fe4,// 172 PAY 169 

    0x68c93e48,// 173 PAY 170 

    0xb89723fa,// 174 PAY 171 

    0x499490d8,// 175 PAY 172 

    0x94a20871,// 176 PAY 173 

    0xe967a6f9,// 177 PAY 174 

    0xff5877fb,// 178 PAY 175 

    0x44be95b9,// 179 PAY 176 

    0x3de6fd1b,// 180 PAY 177 

    0x43565bb6,// 181 PAY 178 

    0x1fd73347,// 182 PAY 179 

    0xad525959,// 183 PAY 180 

    0x7933164b,// 184 PAY 181 

    0x4de9f470,// 185 PAY 182 

    0xdba0820c,// 186 PAY 183 

    0xa2d31829,// 187 PAY 184 

    0x38dfc3f3,// 188 PAY 185 

    0xac03ba50,// 189 PAY 186 

    0x8cd50e38,// 190 PAY 187 

    0x25498bec,// 191 PAY 188 

    0xf024fee6,// 192 PAY 189 

    0xd593d21b,// 193 PAY 190 

    0xc146d131,// 194 PAY 191 

    0xf3d26fce,// 195 PAY 192 

    0xceec7152,// 196 PAY 193 

    0x76325fe9,// 197 PAY 194 

    0x57c4cced,// 198 PAY 195 

    0x410ba9ac,// 199 PAY 196 

    0x1d4592b9,// 200 PAY 197 

    0x7da213b6,// 201 PAY 198 

    0x357612c3,// 202 PAY 199 

    0x2df28f3e,// 203 PAY 200 

    0xc4401dfd,// 204 PAY 201 

    0x75465554,// 205 PAY 202 

    0x7059be70,// 206 PAY 203 

    0xe00821f3,// 207 PAY 204 

    0xa8b09136,// 208 PAY 205 

    0x83ef30dc,// 209 PAY 206 

    0xe401c932,// 210 PAY 207 

    0xc7e10afb,// 211 PAY 208 

    0x34c64892,// 212 PAY 209 

    0x8772572b,// 213 PAY 210 

    0x3253fd25,// 214 PAY 211 

    0xbf7b7fd1,// 215 PAY 212 

    0x12f4578e,// 216 PAY 213 

    0x7563aaa6,// 217 PAY 214 

    0x40938bd7,// 218 PAY 215 

    0x7cd605b8,// 219 PAY 216 

    0x715ebc48,// 220 PAY 217 

    0xe339b628,// 221 PAY 218 

    0x1212016b,// 222 PAY 219 

    0x338a2f26,// 223 PAY 220 

    0x6057e559,// 224 PAY 221 

    0x539a538b,// 225 PAY 222 

    0x2ef88edc,// 226 PAY 223 

    0x93c1b87c,// 227 PAY 224 

    0x7396d74c,// 228 PAY 225 

    0x169d9d92,// 229 PAY 226 

    0xa625028f,// 230 PAY 227 

    0x3ee8b0bf,// 231 PAY 228 

    0xd29578c2,// 232 PAY 229 

    0x1e111172,// 233 PAY 230 

    0x214fc90b,// 234 PAY 231 

    0x0ef9023f,// 235 PAY 232 

    0x0c5ecdc2,// 236 PAY 233 

    0x98d84a10,// 237 PAY 234 

    0x511ab844,// 238 PAY 235 

    0xad19f7f9,// 239 PAY 236 

    0xe9fa5f10,// 240 PAY 237 

    0x460d9c93,// 241 PAY 238 

    0x736bf49b,// 242 PAY 239 

    0xac8bb87e,// 243 PAY 240 

    0x88d8aeb6,// 244 PAY 241 

    0x575ba545,// 245 PAY 242 

    0x773768dd,// 246 PAY 243 

    0x9c3a60b1,// 247 PAY 244 

    0xee7c4e39,// 248 PAY 245 

    0x8bf4e83d,// 249 PAY 246 

    0xf8586c3b,// 250 PAY 247 

    0x16bbf7a3,// 251 PAY 248 

    0x30a9c648,// 252 PAY 249 

    0xea135255,// 253 PAY 250 

    0x5f4df3e9,// 254 PAY 251 

    0x66adda06,// 255 PAY 252 

    0x50e310a6,// 256 PAY 253 

    0x7d8ae12d,// 257 PAY 254 

    0xb6a82244,// 258 PAY 255 

    0x27751067,// 259 PAY 256 

    0x1ba824af,// 260 PAY 257 

    0xbee21a13,// 261 PAY 258 

    0x23949345,// 262 PAY 259 

    0xd68cd0f0,// 263 PAY 260 

    0x864c46df,// 264 PAY 261 

    0xc03d42cc,// 265 PAY 262 

    0x05ae8358,// 266 PAY 263 

    0x1ab6ddec,// 267 PAY 264 

    0x434bd001,// 268 PAY 265 

    0x2cc8aa8d,// 269 PAY 266 

    0xd7a680ba,// 270 PAY 267 

    0x8b492f7a,// 271 PAY 268 

    0x8a810552,// 272 PAY 269 

    0xe07eaf32,// 273 PAY 270 

    0x2aa5ac49,// 274 PAY 271 

    0x08e92438,// 275 PAY 272 

    0x4b6de355,// 276 PAY 273 

    0xe3501837,// 277 PAY 274 

    0x640bef9b,// 278 PAY 275 

    0x970b168e,// 279 PAY 276 

    0xc5e521c1,// 280 PAY 277 

    0x77abf273,// 281 PAY 278 

    0x1cdd884c,// 282 PAY 279 

    0x365362d8,// 283 PAY 280 

    0x86b4e72c,// 284 PAY 281 

    0x5ff93769,// 285 PAY 282 

    0xf6dda42c,// 286 PAY 283 

    0xfa17dff7,// 287 PAY 284 

    0xa9950f65,// 288 PAY 285 

    0x865ced85,// 289 PAY 286 

    0x9457fc10,// 290 PAY 287 

    0x58afa76b,// 291 PAY 288 

    0xc1a0f8a3,// 292 PAY 289 

    0x7164a2f6,// 293 PAY 290 

    0x77a87244,// 294 PAY 291 

    0x48eaba68,// 295 PAY 292 

    0x6d6c39d4,// 296 PAY 293 

    0xedf1a1f1,// 297 PAY 294 

    0xae2889f3,// 298 PAY 295 

    0xb13b0a76,// 299 PAY 296 

    0x1ae3a865,// 300 PAY 297 

    0xd9d13ca9,// 301 PAY 298 

    0x32188a52,// 302 PAY 299 

    0x63cda7be,// 303 PAY 300 

    0x6c9b4229,// 304 PAY 301 

    0x2b01e1cd,// 305 PAY 302 

    0x87a607ef,// 306 PAY 303 

    0xa70595bf,// 307 PAY 304 

    0x8f39aa69,// 308 PAY 305 

    0x1439afbe,// 309 PAY 306 

    0xb629e5cd,// 310 PAY 307 

    0x82d9472a,// 311 PAY 308 

    0x011b91b2,// 312 PAY 309 

    0x188d9d5c,// 313 PAY 310 

    0xcb86487f,// 314 PAY 311 

    0x5c396e69,// 315 PAY 312 

    0x5cb95027,// 316 PAY 313 

    0x8d7f6bcb,// 317 PAY 314 

    0xb7c99b76,// 318 PAY 315 

    0x4f9a25ca,// 319 PAY 316 

    0x80a6ba33,// 320 PAY 317 

    0xabf48ebd,// 321 PAY 318 

    0x58a91b81,// 322 PAY 319 

    0x7d51f63f,// 323 PAY 320 

    0x0a8108b3,// 324 PAY 321 

    0x15a025e8,// 325 PAY 322 

    0xa81724f7,// 326 PAY 323 

    0x191613d0,// 327 PAY 324 

    0xce386476,// 328 PAY 325 

    0x8be11445,// 329 PAY 326 

    0xfb1f82f5,// 330 PAY 327 

    0x86a7331d,// 331 PAY 328 

    0xc35d329e,// 332 PAY 329 

    0x9abb7bd2,// 333 PAY 330 

    0x4becf8e9,// 334 PAY 331 

    0x1243314a,// 335 PAY 332 

    0x824808bd,// 336 PAY 333 

    0x1951cb89,// 337 PAY 334 

    0x773a4a55,// 338 PAY 335 

    0x693aa1ef,// 339 PAY 336 

    0x72b6037d,// 340 PAY 337 

    0xe0bd2618,// 341 PAY 338 

    0xc18a9201,// 342 PAY 339 

    0xbb8df5ed,// 343 PAY 340 

    0x56c531ce,// 344 PAY 341 

    0x6e98718e,// 345 PAY 342 

    0x9842fa1d,// 346 PAY 343 

    0x35dc1983,// 347 PAY 344 

    0xa9337a25,// 348 PAY 345 

    0x04dfa47a,// 349 PAY 346 

    0xd9e9e713,// 350 PAY 347 

    0x187b3a97,// 351 PAY 348 

    0x2dccb810,// 352 PAY 349 

    0x11a32a49,// 353 PAY 350 

    0x4e074a91,// 354 PAY 351 

    0xef07b4f4,// 355 PAY 352 

    0x7272ab65,// 356 PAY 353 

    0xff38200c,// 357 PAY 354 

    0xb7d7e969,// 358 PAY 355 

    0x3d5003df,// 359 PAY 356 

    0x7006d247,// 360 PAY 357 

    0xef2507e8,// 361 PAY 358 

    0x6f1149a1,// 362 PAY 359 

    0x2a54c662,// 363 PAY 360 

    0x359a0f6f,// 364 PAY 361 

    0x783c50ef,// 365 PAY 362 

    0x785d96c3,// 366 PAY 363 

    0xee6c8762,// 367 PAY 364 

    0x741c0000,// 368 PAY 365 

/// STA is 1 words. 

/// STA num_pkts       : 65 

/// STA pkt_idx        : 173 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0x8 

    0x02b40841 // 369 STA   1 

};

//

/// CCH is 1 words  

/// CCH is NEW format  

/// CCH Opcode      = GENERIC (0x1) 

/// CCH CCTX length = 64 (0x40) 

/// CCH pipe select = 0 

/// CCH !incl_sctx !incl_bct !incl_bdesc !incl_mfm  

/// CCH incl_bdata !incl_hash !incl_supdt !incl_sps !drop_bdata  

uint32 rx_des_sha1_pkt99_tmpl[] = {
    0x08010040,// 1 CCH   1 

/// ECH cache_idx      : 0x04 

/// ECH pdu_tag        : 0x00 

    0x00040000,// 2 ECH   1 

/// BDA is 23 words. 

/// BDA size     is 86 (0x56) 

/// BDA id       is 0xb4f7 

    0x0056b4f7,// 3 BDA   1 

/// PAY Generic Data size   : 86 byte(s) 

/// PAD Buffer Data Pad size   : 2 byte(s) 

    0xa517351f,// 4 PAY   1 

    0x86ef2aed,// 5 PAY   2 

    0x6daa3e31,// 6 PAY   3 

    0xbe6dae1f,// 7 PAY   4 

    0x514e7890,// 8 PAY   5 

    0x50ecb448,// 9 PAY   6 

    0x1deaba52,// 10 PAY   7 

    0xab15539f,// 11 PAY   8 

    0x0d72431b,// 12 PAY   9 

    0x67ed2db6,// 13 PAY  10 

    0xa2b0ea8d,// 14 PAY  11 

    0xb6f0b9e4,// 15 PAY  12 

    0x40dab52c,// 16 PAY  13 

    0x14296949,// 17 PAY  14 

    0x8bb5e100,// 18 PAY  15 

    0xcba469c4,// 19 PAY  16 

    0xc6ccbd95,// 20 PAY  17 

    0x6a551fc7,// 21 PAY  18 

    0x8feb4d67,// 22 PAY  19 

    0x7ad033fe,// 23 PAY  20 

    0xd351ccf7,// 24 PAY  21 

    0x7d8d0000,// 25 PAY  22 

/// STA is 1 words. 

/// STA num_pkts       : 62 

/// STA pkt_idx        : 216 

/// STA err_code       : SUCCESS 

/// STA error_details  : 0xe8 

    0x0361e83e // 26 STA   1 

};

//


uint16 rx_des_sha1_pkt_len[] = {
    sizeof(rx_des_sha1_pkt0_tmpl),
    sizeof(rx_des_sha1_pkt1_tmpl),
    sizeof(rx_des_sha1_pkt2_tmpl),
    sizeof(rx_des_sha1_pkt3_tmpl),
    sizeof(rx_des_sha1_pkt4_tmpl),
    sizeof(rx_des_sha1_pkt5_tmpl),
    sizeof(rx_des_sha1_pkt6_tmpl),
    sizeof(rx_des_sha1_pkt7_tmpl),
    sizeof(rx_des_sha1_pkt8_tmpl),
    sizeof(rx_des_sha1_pkt9_tmpl),
    sizeof(rx_des_sha1_pkt10_tmpl),
    sizeof(rx_des_sha1_pkt11_tmpl),
    sizeof(rx_des_sha1_pkt12_tmpl),
    sizeof(rx_des_sha1_pkt13_tmpl),
    sizeof(rx_des_sha1_pkt14_tmpl),
    sizeof(rx_des_sha1_pkt15_tmpl),
    sizeof(rx_des_sha1_pkt16_tmpl),
    sizeof(rx_des_sha1_pkt17_tmpl),
    sizeof(rx_des_sha1_pkt18_tmpl),
    sizeof(rx_des_sha1_pkt19_tmpl),
    sizeof(rx_des_sha1_pkt20_tmpl),
    sizeof(rx_des_sha1_pkt21_tmpl),
    sizeof(rx_des_sha1_pkt22_tmpl),
    sizeof(rx_des_sha1_pkt23_tmpl),
    sizeof(rx_des_sha1_pkt24_tmpl),
    sizeof(rx_des_sha1_pkt25_tmpl),
    sizeof(rx_des_sha1_pkt26_tmpl),
    sizeof(rx_des_sha1_pkt27_tmpl),
    sizeof(rx_des_sha1_pkt28_tmpl),
    sizeof(rx_des_sha1_pkt29_tmpl),
    sizeof(rx_des_sha1_pkt30_tmpl),
    sizeof(rx_des_sha1_pkt31_tmpl),
    sizeof(rx_des_sha1_pkt32_tmpl),
    sizeof(rx_des_sha1_pkt33_tmpl),
    sizeof(rx_des_sha1_pkt34_tmpl),
    sizeof(rx_des_sha1_pkt35_tmpl),
    sizeof(rx_des_sha1_pkt36_tmpl),
    sizeof(rx_des_sha1_pkt37_tmpl),
    sizeof(rx_des_sha1_pkt38_tmpl),
    sizeof(rx_des_sha1_pkt39_tmpl),
    sizeof(rx_des_sha1_pkt40_tmpl),
    sizeof(rx_des_sha1_pkt41_tmpl),
    sizeof(rx_des_sha1_pkt42_tmpl),
    sizeof(rx_des_sha1_pkt43_tmpl),
    sizeof(rx_des_sha1_pkt44_tmpl),
    sizeof(rx_des_sha1_pkt45_tmpl),
    sizeof(rx_des_sha1_pkt46_tmpl),
    sizeof(rx_des_sha1_pkt47_tmpl),
    sizeof(rx_des_sha1_pkt48_tmpl),
    sizeof(rx_des_sha1_pkt49_tmpl),
    sizeof(rx_des_sha1_pkt50_tmpl),
    sizeof(rx_des_sha1_pkt51_tmpl),
    sizeof(rx_des_sha1_pkt52_tmpl),
    sizeof(rx_des_sha1_pkt53_tmpl),
    sizeof(rx_des_sha1_pkt54_tmpl),
    sizeof(rx_des_sha1_pkt55_tmpl),
    sizeof(rx_des_sha1_pkt56_tmpl),
    sizeof(rx_des_sha1_pkt57_tmpl),
    sizeof(rx_des_sha1_pkt58_tmpl),
    sizeof(rx_des_sha1_pkt59_tmpl),
    sizeof(rx_des_sha1_pkt60_tmpl),
    sizeof(rx_des_sha1_pkt61_tmpl),
    sizeof(rx_des_sha1_pkt62_tmpl),
    sizeof(rx_des_sha1_pkt63_tmpl),
    sizeof(rx_des_sha1_pkt64_tmpl),
    sizeof(rx_des_sha1_pkt65_tmpl),
    sizeof(rx_des_sha1_pkt66_tmpl),
    sizeof(rx_des_sha1_pkt67_tmpl),
    sizeof(rx_des_sha1_pkt68_tmpl),
    sizeof(rx_des_sha1_pkt69_tmpl),
    sizeof(rx_des_sha1_pkt70_tmpl),
    sizeof(rx_des_sha1_pkt71_tmpl),
    sizeof(rx_des_sha1_pkt72_tmpl),
    sizeof(rx_des_sha1_pkt73_tmpl),
    sizeof(rx_des_sha1_pkt74_tmpl),
    sizeof(rx_des_sha1_pkt75_tmpl),
    sizeof(rx_des_sha1_pkt76_tmpl),
    sizeof(rx_des_sha1_pkt77_tmpl),
    sizeof(rx_des_sha1_pkt78_tmpl),
    sizeof(rx_des_sha1_pkt79_tmpl),
    sizeof(rx_des_sha1_pkt80_tmpl),
    sizeof(rx_des_sha1_pkt81_tmpl),
    sizeof(rx_des_sha1_pkt82_tmpl),
    sizeof(rx_des_sha1_pkt83_tmpl),
    sizeof(rx_des_sha1_pkt84_tmpl),
    sizeof(rx_des_sha1_pkt85_tmpl),
    sizeof(rx_des_sha1_pkt86_tmpl),
    sizeof(rx_des_sha1_pkt87_tmpl),
    sizeof(rx_des_sha1_pkt88_tmpl),
    sizeof(rx_des_sha1_pkt89_tmpl),
    sizeof(rx_des_sha1_pkt90_tmpl),
    sizeof(rx_des_sha1_pkt91_tmpl),
    sizeof(rx_des_sha1_pkt92_tmpl),
    sizeof(rx_des_sha1_pkt93_tmpl),
    sizeof(rx_des_sha1_pkt94_tmpl),
    sizeof(rx_des_sha1_pkt95_tmpl),
    sizeof(rx_des_sha1_pkt96_tmpl),
    sizeof(rx_des_sha1_pkt97_tmpl),
    sizeof(rx_des_sha1_pkt98_tmpl),
    sizeof(rx_des_sha1_pkt99_tmpl)
};

unsigned char *rx_des_sha1_test_pkts[] = {
    (unsigned char *)&rx_des_sha1_pkt0_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt1_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt2_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt3_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt4_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt5_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt6_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt7_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt8_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt9_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt10_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt11_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt12_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt13_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt14_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt15_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt16_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt17_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt18_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt19_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt20_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt21_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt22_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt23_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt24_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt25_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt26_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt27_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt28_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt29_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt30_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt31_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt32_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt33_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt34_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt35_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt36_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt37_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt38_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt39_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt40_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt41_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt42_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt43_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt44_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt45_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt46_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt47_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt48_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt49_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt50_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt51_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt52_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt53_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt54_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt55_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt56_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt57_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt58_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt59_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt60_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt61_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt62_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt63_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt64_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt65_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt66_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt67_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt68_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt69_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt70_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt71_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt72_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt73_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt74_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt75_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt76_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt77_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt78_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt79_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt80_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt81_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt82_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt83_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt84_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt85_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt86_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt87_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt88_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt89_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt90_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt91_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt92_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt93_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt94_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt95_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt96_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt97_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt98_tmpl[0],
    (unsigned char *)&rx_des_sha1_pkt99_tmpl[0]
};
#endif /*CONFIG_BCM_SPU_TEST */
#endif /* __SPUDRV_RX_DES_SHA1_DATA_H__ */
