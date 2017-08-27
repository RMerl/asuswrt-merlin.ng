/*
 * Copyright (C) 2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2009-2013  Security Innovation
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

#include "ntru_param_set.h"

#include <utils/test.h>

ENUM(ntru_param_set_id_names, NTRU_EES401EP1, NTRU_EES743EP1,
	"ees401ep1",
	"ees449ep1",
	"ees677ep1",
	"ees1087ep2",
	"ees541ep1",
	"ees613ep1",
	"ees887ep1",
	"ees1171ep1",
	"ees659ep1",
	"ees761ep1",
	"ees1087ep1",
	"ees1499ep1",
	"ees401ep2",
	"ees439ep1",
	"ees593ep1",
	"ees743ep1"
);

/**
 * NTRU encryption parameter set definitions
 */
static ntru_param_set_t ntru_param_sets[] = {

	/* X9.98/IEEE 1363.1 parameter sets for best bandwidth (smallest size) */
    {
        NTRU_EES401EP1,              /* parameter-set id */
        {0x00, 0x02, 0x04},          /* OID */
        0x22,                        /* DER id */
        9,                           /* no. of bits in N (i.e., in an index) */
        401,                         /* N */
        14,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        113,                         /* df, dr */
        133,                         /* dg */
        60,                          /* maxMsgLenBytes */
        113,                         /* dm0 */
        11,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES449EP1,              /* parameter-set id */
        {0x00, 0x03, 0x03},          /* OID */
        0x23,                        /* DER id */
        9,                           /* no. of bits in N (i.e., in an index) */
        449,                         /* N */
        16,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        134,                         /* df, dr */
        149,                         /* dg */
        67,                          /* maxMsgLenBytes */
        134,                         /* dm0 */
        9,                           /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES677EP1,              /* parameter-set id */
        {0x00, 0x05, 0x03},          /* OID */
        0x24,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        677,                         /* N */
        24,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        157,                         /* df, dr */
        225,                         /* dg */
        101,                         /* maxMsgLenBytes */
        157,                         /* dm0 */
        11,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES1087EP2,             /* parameter-set id */
        {0x00, 0x06, 0x03},          /* OID */
        0x25,                        /* DER id */
        11,                          /* no. of bits in N (i.e., in an index) */
        1087,                        /* N */
        32,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        120,                         /* df, dr */
        362,                         /* dg */
        170,                         /* maxMsgLenBytes */
        120,                         /* dm0 */
        13,                          /* c */
        1,                           /* lLen */
    },

	/* X9.98/IEEE 1363.1 parameter sets balancing speed and bandwidth */
    {
        NTRU_EES541EP1,              /* parameter-set id */
        {0x00, 0x02, 0x05},          /* OID */
        0x26,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        541,                         /* N */
        14,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        49,                          /* df, dr */
        180,                         /* dg */
        86,                          /* maxMsgLenBytes */
        49,                          /* dm0 */
        12,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES613EP1,              /* parameter-set id */
        {0x00, 0x03, 0x04},          /* OID */
        0x27,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        613,                         /* N */
        16,                          /* securuity strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        55,                          /* df, dr */
        204,                         /* dg */
        97,                          /* maxMsgLenBytes */
        55,                          /* dm0 */
        11,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES887EP1,              /* parameter-set id */
        {0x00, 0x05, 0x04},          /* OID */
        0x28,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        887,                         /* N */
        24,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        81,                          /* df, dr */
        295,                         /* dg */
        141,                         /* maxMsgLenBytes */
        81,                          /* dm0 */
        10,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES1171EP1,             /* parameter-set id */
        {0x00, 0x06, 0x04},          /* OID */
        0x29,                        /* DER id */
        11,                          /* no. of bits in N (i.e., in an index) */
        1171,                        /* N */
        32,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        106,                         /* df, dr */
        390,                         /* dg */
        186,                         /* maxMsgLenBytes */
        106,                         /* dm0 */
        12,                          /* c */
        1,                           /* lLen */
    },

	/* X9.98/IEEE 1363.1 parameter sets for best speed */
    {
        NTRU_EES659EP1,              /* parameter-set id */
        {0x00, 0x02, 0x06},          /* OID */
        0x2a,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        659,                         /* N */
        14,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        38,                          /* df, dr */
        219,                         /* dg */
        108,                         /* maxMsgLenBytes */
        38,                          /* dm0 */
        11,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES761EP1,              /* parameter-set id */
        {0x00, 0x03, 0x05},          /* OID */
        0x2b,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        761,                         /* N */
        16,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        42,                          /* df, dr */
        253,                         /* dg */
        125,                         /* maxMsgLenBytes */
        42,                          /* dm0 */
        12,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES1087EP1,             /* parameter-set id */
        {0x00, 0x05, 0x05},          /* OID */
        0x2c,                        /* DER id */
        11,                          /* no. of bits in N (i.e., in an index) */
        1087,                        /* N */
        24,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        63,                          /* df, dr */
        362,                         /* dg */
        178,                         /* maxMsgLenBytes */
        63,                          /* dm0 */
        13,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES1499EP1,             /* parameter-set id */
        {0x00, 0x06, 0x05},          /* OID */
        0x2d,                        /* DER id */
        11,                          /* no. of bits in N (i.e., in an index) */
        1499,                        /* N */
        32,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        FALSE,                       /* product form */
        79,                          /* df, dr */
        499,                         /* dg */
        247,                         /* maxMsgLenBytes */
        79,                          /* dm0 */
        13,                          /* c */
        1,                           /* lLen */
    },

	/* Best bandwidth and speed, no X9.98 compatibility */
    {
        NTRU_EES401EP2,              /* parameter-set id */
        {0x00, 0x02, 0x10},          /* OID */
        0x2e,                        /* DER id */
        9,                           /* no. of bits in N (i.e., in an index) */
        401,                         /* N */
        14,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        TRUE,                        /* product form */
        8 + (8 << 8) + (6 << 16),    /* df, dr */
        133,                         /* dg */
        60,                          /* maxMsgLenBytes */
        136,                         /* m(1)_max */
        11,                          /* c */
        1,                           /* lLen */
   },

    {
        NTRU_EES439EP1,              /* parameter-set id */
        {0x00, 0x03, 0x10},          /* OID */
        0x2f,                        /* DER id */
        9,                           /* no. of bits in N (i.e., in an index) */
        439,                         /* N */
        16,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        TRUE,                        /* product form */
        9 + (8 << 8) + (5 << 16),    /* df, dr */
        146,                         /* dg */
        65,                          /* maxMsgLenBytes */
        126,                         /* m(1)_max */
        9,                           /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES593EP1,              /* parameter-set id */
        {0x00, 0x05, 0x10},          /* OID */
        0x30,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        593,                         /* N */
        24,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        TRUE,                        /* product form */
        10 + (10 << 8) + (8 << 16),  /* df, dr */
        197,                         /* dg */
        86,                          /* maxMsgLenBytes */
        90,                          /* m(1)_max */
        11,                          /* c */
        1,                           /* lLen */
    },

    {
        NTRU_EES743EP1,              /* parameter-set id */
        {0x00, 0x06, 0x10},          /* OID */
        0x31,                        /* DER id */
        10,                          /* no. of bits in N (i.e., in an index) */
        743,                         /* N */
        32,                          /* security strength in octets */
        2048,                        /* q */
        11,                          /* no. of bits in q (i.e., in a coeff) */
        TRUE,                        /* product form */
        11 + (11 << 8) + (15 << 16), /* df, dr */
        247,                         /* dg */
        106,                         /* maxMsgLenBytes */
        60,                          /* m(1)_max */
        13,                          /* c */
        1,                           /* lLen */
    },

};

/**
 * See header.
 */
ntru_param_set_t* ntru_param_set_get_by_id(ntru_param_set_id_t id)
{
	int i;

	for (i = 0; i < countof(ntru_param_sets); i++)
	{
		if (ntru_param_sets[i].id == id)
		{
			return &ntru_param_sets[i];
		}
	}
	return NULL;
}


/**
 * See header.
 */
ntru_param_set_t* ntru_param_set_get_by_oid(uint8_t const *oid)
{
	int i;

	for (i = 0; i < countof(ntru_param_sets); i++)
	{
		if (memeq(ntru_param_sets[i].oid, oid, 3))
		{
			return &ntru_param_sets[i];
		}
	}
	return NULL;
}

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_param_set_get_by_id);
