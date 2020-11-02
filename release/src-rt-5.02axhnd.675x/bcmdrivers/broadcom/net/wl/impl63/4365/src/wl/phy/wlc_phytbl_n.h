/*
 * Declarations for Broadcom PHY core tables,
 * Networking Adapter Device Driver.
 *
 * THIS IS A GENERATED FILE - DO NOT EDIT (ARE WE SURE??)
 * Generated on Wed Aug 30 17:06:38 PDT 2006
 *
 * Copyright(c) 2005 Broadcom Corp.
 * All Rights Reserved.
 *
 * $Id: wlc_phytbl_n.h 382726 2013-02-04 03:41:32Z $
 */
/* FILE-CSTYLED */

#ifndef _WLC_PHYTBL_N_H_
#define _WLC_PHYTBL_N_H_

/* The position of the ant_swctrl_tbl in the volatile array of tables */
#define ANT_SWCTRL_TBL_REV3_IDX (0)

typedef phytbl_info_t mimophytbl_info_t;

typedef struct _phytbl_init {
	uint16	base;
	uint8 	num;
} phytbl_init_t;

extern CONST mimophytbl_info_t mimophytbl_info_rev0[], mimophytbl_info_rev0_volatile[];
extern CONST uint32 mimophytbl_info_sz_rev0, mimophytbl_info_sz_rev0_volatile;

extern CONST mimophytbl_info_t mimophytbl_info_rev3[], mimophytbl_info_rev3_volatile[],
        mimophytbl_info_rev3_volatile1[],  mimophytbl_info_rev3_volatile2[],
        mimophytbl_info_rev3_volatile3[];
extern CONST uint32 mimophytbl_info_sz_rev3, mimophytbl_info_sz_rev3_volatile,
        mimophytbl_info_sz_rev3_volatile1, mimophytbl_info_sz_rev3_volatile2,
        mimophytbl_info_sz_rev3_volatile3;

extern CONST uint32 noise_var_tbl_rev3[];

extern CONST mimophytbl_info_t mimophytbl_info_rev7[];
extern CONST uint32 mimophytbl_info_sz_rev7;
extern CONST uint32 noise_var_tbl_rev7[];
extern CONST uint32 tmap_tbl_rev7[];

extern CONST mimophytbl_info_t mimophytbl_info_rev8to10[];
extern CONST uint32 mimophytbl_info_sz_rev8to10;
extern CONST uint32 frame_struct_rev8[];

extern CONST uint32 frame_struct_rev8_redux[];
extern CONST uint32 tmap_tbl_rev7_redux[];
extern CONST phytbl_init_t frame_struct_rev8_offsets[];
extern CONST phytbl_init_t tmap_tbl_rev7_offsets[];
extern CONST uint8 chanest_tbl_rev3_offsets[];

/* PR 87670: PHY tables have unfortunately been ROM-ed for 43236,
 * so keep most of the original code so as not to increase RAM size
 */
extern CONST mimophytbl_info_t mimophytbl_info_43236[];
extern CONST uint32 mimophytbl_info_sz_43236;

/* LCNXN */
extern CONST mimophytbl_info_t mimophytbl_info_rev16[];
extern CONST uint32 mimophytbl_info_sz_rev16;

#endif /* _WLC_PHYTBL_N_H_ */
