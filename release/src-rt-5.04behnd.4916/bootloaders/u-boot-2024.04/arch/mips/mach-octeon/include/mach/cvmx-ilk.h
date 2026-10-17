/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * This file contains defines for the ILK interface
 */

#ifndef __CVMX_ILK_H__
#define __CVMX_ILK_H__

/* CSR typedefs have been moved to cvmx-ilk-defs.h */

/*
 * Note: this macro must match the first ilk port in the ipd_port_map_68xx[]
 * and ipd_port_map_78xx[] arrays.
 */
static inline int CVMX_ILK_GBL_BASE(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 5;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 6;
	return -1;
}

static inline int CVMX_ILK_QLM_BASE(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 1;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 4;
	return -1;
}

typedef struct {
	int intf_en : 1;
	int la_mode : 1;
	int reserved : 14; /* unused */
	int lane_speed : 16;
	/* add more here */
} cvmx_ilk_intf_t;

#define CVMX_NUM_ILK_INTF 2
static inline int CVMX_ILK_MAX_LANES(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 8;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 16;
	return -1;
}

extern unsigned short cvmx_ilk_lane_mask[CVMX_MAX_NODES][CVMX_NUM_ILK_INTF];

typedef struct {
	unsigned int pipe;
	unsigned int chan;
} cvmx_ilk_pipe_chan_t;

#define CVMX_ILK_MAX_PIPES 45
/* Max number of channels allowed */
#define CVMX_ILK_MAX_CHANS 256

extern int cvmx_ilk_chans[CVMX_MAX_NODES][CVMX_NUM_ILK_INTF];

typedef struct {
	unsigned int chan;
	unsigned int pknd;
} cvmx_ilk_chan_pknd_t;

#define CVMX_ILK_MAX_PKNDS 16 /* must be <45 */

typedef struct {
	int *chan_list; /* for discrete channels. or, must be null */
	unsigned int num_chans;

	unsigned int chan_start; /* for continuous channels */
	unsigned int chan_end;
	unsigned int chan_step;

	unsigned int clr_on_rd;
} cvmx_ilk_stats_ctrl_t;

#define CVMX_ILK_MAX_CAL      288
#define CVMX_ILK_MAX_CAL_IDX  (CVMX_ILK_MAX_CAL / 8)
#define CVMX_ILK_TX_MIN_CAL   1
#define CVMX_ILK_RX_MIN_CAL   1
#define CVMX_ILK_CAL_GRP_SZ   8
#define CVMX_ILK_PIPE_BPID_SZ 7
#define CVMX_ILK_ENT_CTRL_SZ  2
#define CVMX_ILK_RX_FIFO_WM   0x200

typedef enum { PIPE_BPID = 0, LINK, XOFF, XON } cvmx_ilk_cal_ent_ctrl_t;

typedef struct {
	unsigned char pipe_bpid;
	cvmx_ilk_cal_ent_ctrl_t ent_ctrl;
} cvmx_ilk_cal_entry_t;

typedef enum { CVMX_ILK_LPBK_DISA = 0, CVMX_ILK_LPBK_ENA } cvmx_ilk_lpbk_ena_t;

typedef enum { CVMX_ILK_LPBK_INT = 0, CVMX_ILK_LPBK_EXT } cvmx_ilk_lpbk_mode_t;

/**
 * This header is placed in front of all received ILK look-aside mode packets
 */
typedef union {
	u64 u64;

	struct {
		u32 reserved_63_57 : 7;	  /* bits 63...57 */
		u32 nsp_cmd : 5;	  /* bits 56...52 */
		u32 nsp_flags : 4;	  /* bits 51...48 */
		u32 nsp_grp_id_upper : 6; /* bits 47...42 */
		u32 reserved_41_40 : 2;	  /* bits 41...40 */
		/* Protocol type, 1 for LA mode packet */
		u32 la_mode : 1;	  /* bit  39      */
		u32 nsp_grp_id_lower : 2; /* bits 38...37 */
		u32 nsp_xid_upper : 4;	  /* bits 36...33 */
		/* ILK channel number, 0 or 1 */
		u32 ilk_channel : 1;   /* bit  32      */
		u32 nsp_xid_lower : 8; /* bits 31...24 */
		/* Unpredictable, may be any value */
		u32 reserved_23_0 : 24; /* bits 23...0  */
	} s;
} cvmx_ilk_la_nsp_compact_hdr_t;

typedef struct cvmx_ilk_LA_mode_struct {
	int ilk_LA_mode;
	int ilk_LA_mode_cal_ena;
} cvmx_ilk_LA_mode_t;

extern cvmx_ilk_LA_mode_t cvmx_ilk_LA_mode[CVMX_NUM_ILK_INTF];

int cvmx_ilk_use_la_mode(int interface, int channel);
int cvmx_ilk_start_interface(int interface, unsigned short num_lanes);
int cvmx_ilk_start_interface_la(int interface, unsigned char num_lanes);
int cvmx_ilk_set_pipe(int interface, int pipe_base, unsigned int pipe_len);
int cvmx_ilk_tx_set_channel(int interface, cvmx_ilk_pipe_chan_t *pch, unsigned int num_chs);
int cvmx_ilk_rx_set_pknd(int interface, cvmx_ilk_chan_pknd_t *chpknd, unsigned int num_pknd);
int cvmx_ilk_enable(int interface);
int cvmx_ilk_disable(int interface);
int cvmx_ilk_get_intf_ena(int interface);
int cvmx_ilk_get_chan_info(int interface, unsigned char **chans, unsigned char *num_chan);
cvmx_ilk_la_nsp_compact_hdr_t cvmx_ilk_enable_la_header(int ipd_port, int mode);
void cvmx_ilk_show_stats(int interface, cvmx_ilk_stats_ctrl_t *pstats);
int cvmx_ilk_cal_setup_rx(int interface, int cal_depth, cvmx_ilk_cal_entry_t *pent, int hi_wm,
			  unsigned char cal_ena);
int cvmx_ilk_cal_setup_tx(int interface, int cal_depth, cvmx_ilk_cal_entry_t *pent,
			  unsigned char cal_ena);
int cvmx_ilk_lpbk(int interface, cvmx_ilk_lpbk_ena_t enable, cvmx_ilk_lpbk_mode_t mode);
int cvmx_ilk_la_mode_enable_rx_calendar(int interface);

#endif /* __CVMX_ILK_H__ */
