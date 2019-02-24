/*
 * Copyright (C) 2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "bliss_param_set.h"

#include <asn1/oid.h>

ENUM(bliss_param_set_id_names, BLISS_I, BLISS_B_IV,
	"BLISS-I",
	"BLISS-II",
	"BLISS-III",
	"BLISS-IV",
	"BLISS-B-I",
	"BLISS-B-II",
	"BLISS-B-III",
	"BLISS-B-IV"
);

/**
 * sigma = 215, k_sigma = ceiling[ sqrt(2*ln 2) * sigma ] = 254
 *
 * c[i] = exp(-2^i/f), i = 0..20, with f = k_sigma^2 / ln 2 = 93'076.9
 */
static const uint8_t c_bliss_i[] = {
	255, 255,  75, 191, 247,  94,  30,  51, 147, 246,  89,  59,  99, 248,  26, 128,
	255, 254, 151, 128, 109, 166,  88, 143,  30, 175, 149,  20, 240,  81, 138, 111,
	255, 253,  47,   2, 214, 243, 188,  76, 236, 235,  40,  62,  54,  35,  33, 205,
	255, 250,  94,  13, 156, 120, 121, 216, 255, 120,  90,  11,  39, 232, 120, 111,
	255, 244, 188,  58, 242, 219, 157, 174,   6,  31, 131,  75,  88, 109, 112, 107,
	255, 233, 120, 244, 202, 151,  25,  10, 197, 109, 113, 255, 157,  89, 182, 141,
	255, 210, 243, 229,  18,  88,  50, 239, 130, 192,  12, 167,  62, 254, 211, 202,
	255, 165, 239, 183, 102, 186, 123, 249, 251,  59, 116, 143,  50, 174, 125, 198,
	255,  75, 255,  30,  65, 137, 228, 148,  14,  17, 113, 251,  81, 177, 151, 168,
	254, 152, 124, 205, 192, 136, 102,  79,   5,  62, 214,  95,  36, 223,   7,  20,
	253,  50, 242, 124, 187,  59,  68, 224,  90, 156,  53, 202,   9,  44, 191, 226,
	250, 109, 189, 110,  40, 124,  88,  12,  83,  78, 176,  86,  12, 102,  13,  41,
	244, 250, 133,   6,   3,  13,  45,   9, 120, 121, 150, 237,  69, 190,  62,  16,
	234, 110, 130, 187, 138, 174,  82, 229, 217, 154,  88, 138, 228, 153, 230,  13,
	214, 174,  54, 179, 117, 116, 223, 152,  97,  84,  31,  99,  68, 150, 122, 244,
	180,   7, 186,   2, 112,   3,  68,  13, 123, 133, 244, 184, 232, 216, 133,  18,
	126, 154, 221, 207,  32, 206,  66, 171,  94, 100, 164, 194, 117, 191,   1, 209,
	 62, 156, 208,   7, 129, 173, 200,   3,  23, 248, 140,  60,  69, 217, 195, 235,
	 15,  80,  84, 209, 213,   2, 107, 160,   1, 152,  43, 130,  93,  95, 241, 218,
	  0, 234, 131,  37, 182,  53, 201, 231,  26,   2, 151, 161,  13, 214, 150, 145,
	  0,   0, 214, 212,   4,  32, 184,  94,  84,  90, 244, 139,  48,  69,  33,  38
};

/**
 * sigma = 250, k_sigma = ceiling[ sqrt(2*ln 2) * sigma ] = 295
 *
 * c[i] = exp(-2^i/f), i = 0..20, with f = k_sigma^2 / ln 2 = 125'550.5
 */
static const uint8_t c_bliss_iii[] = {
	255, 255, 122,  95,  16, 128,  14, 195,  60,  90, 166, 191, 205,  26, 144, 204,
	255, 254, 244, 190, 102, 192, 187, 141, 169,  92,  33,  30, 170, 141, 184,  56,
	255, 253, 233, 125, 228, 131,  93, 148, 121,  92,  52, 122, 149,  96,  29,  66,
	255, 251, 211,   0,  37,   9, 199, 244, 213, 217, 122, 205, 171, 200, 198,   5,
	255, 247, 166,  17, 185, 251,  90, 150,   1,  28,   7, 205, 125,  46,  84, 201,
	255, 239,  76, 105,  50, 114, 159, 235, 215, 165, 204, 182, 125, 143, 228, 222,
	255, 222, 153, 233,  85, 187,  45, 204, 236, 229,  38, 180,  20, 161,   7, 167,
	255, 189,  56,  46,  38,   4,  83,   8, 151, 137, 136,   1,   9, 180,  58, 204,
	255, 122, 129, 199, 240,  52, 248, 193,  76,  26, 160,  32, 195, 250, 217,  25,
	254, 245,  73,  44,  68, 229, 150,  74, 228,  74, 124, 249, 123,  94, 108, 127,
	253, 235, 168,  56, 252,  93, 188, 160, 249, 137, 236,  65,  62, 182, 153,  63,
	251, 219, 163, 110, 233, 251, 114, 216, 230,  35,  59, 210, 107, 100, 184,  16,
	247, 200, 110, 236, 134, 237, 213, 111, 240, 149, 109,  22, 216, 213, 237, 145,
	239, 212,  98, 249, 238,   1, 227, 248, 242,  51, 211, 134, 154, 115, 189,  83,
	224, 174,  65,   2, 190, 158,   9,   6, 184,  13, 130, 104, 247, 102,  38, 160,
	197,  49, 104,  97,  61, 210,  19, 115, 208,  54,  91,  27, 209, 227,  33,  26,
	151, 229,  20,  46, 200, 238,  35, 134,  72, 183, 253, 160, 193, 155, 117, 103,
	 90,  32,  10, 204,  78,  83, 191, 230,   0, 221, 219,   6,  43, 252, 185,  95,
	 31, 186, 139, 154,  90, 155,  17,   9,  42, 139,  40, 111, 246, 175,   4,  15,
	  3, 238, 181, 190, 138,  94,  50, 234, 128, 193,  95,  36,  65, 236, 170, 208,
	  0,  15, 118, 216, 230, 142, 121, 211,  13, 168, 207, 126, 145, 176,  24, 201
};

/**
 * sigma = 271, k_sigma = ceiling[ sqrt(2*ln 2) * sigma ] = 320
 *
 * c[i] = exp(-2^i/f), i = 0..21, with f = k_sigma^2 / ln 2 = 147'732.0
 */
static const uint8_t c_bliss_iv[] = {
	255, 255, 142, 111, 102,   2, 141,  87, 150,  42,  18,  70,   6, 224,  18,  70,
	255, 255,  28, 222, 254, 102,  20,  78, 133,  78, 189, 107,  29,   7,  23, 193,
	255, 254,  57, 190, 198,  79, 181, 181, 108,  75, 142, 145,  45, 238, 193,  29,
	255, 252, 115, 128, 178, 170, 212, 166, 120, 157,  85,  96, 209, 180, 211,  83,
	255, 248, 231,  13, 253, 108, 245,  46, 238, 155,  30,  99, 141, 228, 149, 239,
	255, 241, 206,  78,  90, 132,  83, 172, 228, 179, 119, 115, 240,  51, 216,   6,
	255, 227, 157, 102,  46,  28,  61, 128,  58, 114, 174, 136,   8, 224, 133,  84,
	255, 199,  61, 242,  19, 216, 133, 241, 240,  22, 146,  43,  92,  57,  82, 248,
	255, 142, 136, 121, 160, 225, 119, 214, 241,  44, 159,  34, 133, 118,  96,  60,
	255,  29,  67,  61, 254,  49,  27, 152,  48, 124, 184,  87,  66, 214,  63, 133,
	254,  59,  79,  77, 206,  26, 238,  42,  69,  81, 191, 149, 146,  76, 255, 232,
	252, 121, 191,  28,  11, 107, 141, 223, 234,  42, 226,  50, 138, 102,  16,  97,
	248, 255, 234,  37, 109, 169, 103,  25, 240, 109,  93, 165, 177,  22, 133, 100,
	242,  48, 213, 124, 209,  49,  33,  48,  57, 237, 202,  62, 102, 132, 219,  48,
	229,  32,  92, 240, 188,  88,  70,  34, 179,  94, 244,  70,  25, 123,  76, 140,
	205,  18, 234,  94,  14, 226, 237,  76, 192,  18, 240,  50,  79,  63,  34,  96,
	164,  71,  76, 192, 111, 161, 157, 188,  19, 189, 133, 246,  67, 127,   6,  28,
	105, 107, 110,  50,  56, 199, 208, 174,  16,  95, 153, 106, 217, 198, 194, 179,
	 43, 105,  77, 122, 127, 254, 146, 221,  44, 235,  61,  22, 179,   9, 113, 118,
	  7,  92, 139,  87, 204, 239, 111, 200,  41, 129, 122,  49,  69, 113, 122, 239,
	  0,  54,  49,  19,  64,  40, 218, 222,  60,  82, 186, 246,  64, 155, 184,  47,
	  0,   0,  11, 120, 189, 135, 113,  62, 143, 175, 118, 239, 190, 120, 189, 250
};

/**
 * BLISS signature parameter set definitions
 */
static const bliss_param_set_t bliss_param_sets[] = {

	/* BLISS-I scheme */
	{
		.id = BLISS_I,
		.oid = OID_BLISS_I,
		.strength = 128,
		.q = 12289,
		.q_bits = 14,
		.q2_inv = 6145,
		.n = 512,
		.n_bits = 9,
		.fft_params = &ntt_fft_12289_512,
		.non_zero1 = 154,
		.non_zero2 = 0,
		.kappa = 23,
		.nks_max = 46479,
		.p_max = 0,     /* not needed */
		.sigma = 215,
		.k_sigma = 254,
		.k_sigma_bits = 8,
		.c = c_bliss_i,
		.c_cols = 16,
		.c_rows = 21,
		.z1_bits = 12,
		.d = 10,
		.p = 24,
		.M = 46539,     /* with alpha = 1.000 */
		.B_inf = 2047,  /* reduced from 2100 due to 12 bit z1 encoding */
		.B_l2 = 12872 * 12872
	},

	/* BLISS-III scheme */
	{
		.id = BLISS_III,
		.oid = OID_BLISS_III,
		.strength = 160,
		.q = 12289,
		.q_bits = 14,
		.q2_inv = 6145,
		.n = 512,
		.n_bits = 9,
		.fft_params = &ntt_fft_12289_512,
		.non_zero1 = 216,
		.non_zero2 = 16,
		.kappa = 30,
		.nks_max = 128626,
		.p_max = 0,     /* not needed */
		.sigma = 250,
		.k_sigma = 295,
		.k_sigma_bits = 9,
		.c = c_bliss_iii,
		.c_cols = 16,
		.c_rows = 21,
		.z1_bits = 12,
		.d = 9,
		.p = 48,
		.M = 128113,    /* with alpha = 0.700 */
		.B_inf = 1760,
		.B_l2 = 10206 * 10206
	},

	/* BLISS-IV scheme */
	{
		.id = BLISS_IV,
		.oid = OID_BLISS_IV,
		.strength = 192,
		.q = 12289,
		.q_bits = 14,
		.q2_inv = 6145,
		.n = 512,
		.n_bits = 9,
		.fft_params = &ntt_fft_12289_512,
		.non_zero1 = 231,
		.non_zero2 = 31,
		.kappa = 39,
		.nks_max = 244669,
		.p_max = 0,     /* not needed */
		.sigma = 271,
		.k_sigma = 320,
		.k_sigma_bits = 9,
		.c = c_bliss_iv,
		.c_cols = 16,
		.c_rows = 22,
		.z1_bits = 12,
		.d = 8,
		.p = 96,
		.M = 244186,    /* with alpha = 0.550 */
		.B_inf = 1613,
		.B_l2 = 9901 * 9901
	},

	/* BLISS-B-I scheme */
	{
		.id = BLISS_B_I,
		.oid = OID_BLISS_B_I,
		.strength = 128,
		.q = 12289,
		.q_bits = 14,
		.q2_inv = 6145,
		.n = 512,
		.n_bits = 9,
		.fft_params = &ntt_fft_12289_512,
		.non_zero1 = 154,
		.non_zero2 = 0,
		.kappa = 23,
		.nks_max = 0,   /* not needed */
		.p_max = 17825,
		.sigma = 215,
		.k_sigma = 254,
		.k_sigma_bits = 8,
		.c = c_bliss_i,
		.c_cols = 16,
		.c_rows = 21,
		.z1_bits = 12,
		.d = 10,
		.p = 24,
		.M = 17954,     /* with alpha = 1.610 */
		.B_inf = 2047,  /* reduced from 2100 due to 12 bit z1 encoding */
		.B_l2 = 12872 * 12872
	},

	/* BLISS-B-III scheme */
	{
		.id = BLISS_B_III,
		.oid = OID_BLISS_B_III,
		.strength = 160,
		.q = 12289,
		.q_bits = 14,
		.q2_inv = 6145,
		.n = 512,
		.n_bits = 9,
		.fft_params = &ntt_fft_12289_512,
		.non_zero1 = 216,
		.non_zero2 = 16,
		.kappa = 30,
		.nks_max = 0,   /* not needed */
		.p_max = 42270,
		.sigma = 250,
		.k_sigma = 295,
		.k_sigma_bits = 9,
		.c = c_bliss_iii,
		.c_cols = 16,
		.c_rows = 21,
		.z1_bits = 12,
		.d = 9,
		.p = 48,
		.M = 42455,     /* with alpha = 1.216 */
		.B_inf = 1760,
		.B_l2 = 10206 * 10206
	},

	/* BLISS-B-IV scheme */
	{
		.id = BLISS_B_IV,
		.oid = OID_BLISS_B_IV,
		.strength = 192,
		.q = 12289,
		.q_bits = 14,
		.q2_inv = 6145,
		.n = 512,
		.n_bits = 9,
		.fft_params = &ntt_fft_12289_512,
		.non_zero1 = 231,
		.non_zero2 = 31,
		.kappa = 39,
		.nks_max = 0,   /* not needed */
		.p_max = 69576,
		.sigma = 271,
		.k_sigma = 320,
		.k_sigma_bits = 9,
		.c = c_bliss_iv,
		.c_cols = 16,
		.c_rows = 22,
		.z1_bits = 12,
		.d = 8,
		.p = 96,
		.M = 70034,     /* with alpha = 1.027 */
		.B_inf = 1613,
		.B_l2 = 9901 * 9901
	}

};

/**
 * See header.
 */
const bliss_param_set_t* bliss_param_set_get_by_id(bliss_param_set_id_t id)
{
	int i;

	for (i = 0; i < countof(bliss_param_sets); i++)
	{
		if (bliss_param_sets[i].id == id)
		{
			return &bliss_param_sets[i];
		}
	}
	return NULL;
}


/**
 * See header.
 */
const bliss_param_set_t* bliss_param_set_get_by_oid(int oid)
{
	int i;

	for (i = 0; i < countof(bliss_param_sets); i++)
	{
		if (bliss_param_sets[i].oid == oid)
		{
			return &bliss_param_sets[i];
		}
	}
	return NULL;
}
