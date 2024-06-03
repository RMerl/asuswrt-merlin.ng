/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Serial Presence Detect (SPD) EEPROM format according to the
 * Intel's PC SDRAM Serial Presence Detect (SPD) Specification,
 * revision 1.2B, November 1999
 */

#ifndef _SPD_H_
#define _SPD_H_

typedef struct spd_eeprom_s {
	unsigned char info_size;   /*  0 # bytes written into serial memory */
	unsigned char chip_size;   /*  1 Total # bytes of SPD memory device */
	unsigned char mem_type;    /*  2 Fundamental memory type */
	unsigned char nrow_addr;   /*  3 # of Row Addresses on this assembly */
	unsigned char ncol_addr;   /*  4 # of Column Addrs on this assembly */
	unsigned char nrows;       /*  5 # of Module Rows on this assembly */
	unsigned char dataw_lsb;   /*  6 Data Width of this assembly */
	unsigned char dataw_msb;   /*  7 ... Data Width continuation */
	unsigned char voltage;     /*  8 Voltage intf std of this assembly */
	unsigned char clk_cycle;   /*  9 SDRAM Cycle time at CL=X */
	unsigned char clk_access;  /* 10 SDRAM Access from Clock at CL=X */
	unsigned char config;      /* 11 DIMM Configuration type */
	unsigned char refresh;     /* 12 Refresh Rate/Type */
	unsigned char primw;       /* 13 Primary SDRAM Width */
	unsigned char ecw;         /* 14 Error Checking SDRAM width */
	unsigned char min_delay;   /* 15 for Back to Back Random Address */
	unsigned char burstl;      /* 16 Burst Lengths Supported */
	unsigned char nbanks;      /* 17 # of Banks on Each SDRAM Device */
	unsigned char cas_lat;     /* 18 CAS# Latencies Supported */
	unsigned char cs_lat;      /* 19 CS# Latency */
	unsigned char write_lat;   /* 20 Write Latency (aka Write Recovery) */
	unsigned char mod_attr;    /* 21 SDRAM Module Attributes */
	unsigned char dev_attr;    /* 22 SDRAM Device Attributes */
	unsigned char clk_cycle2;  /* 23 Min SDRAM Cycle time at CL=X-1 */
	unsigned char clk_access2; /* 24 SDRAM Access from Clock at CL=X-1 */
	unsigned char clk_cycle3;  /* 25 Min SDRAM Cycle time at CL=X-2 */
	unsigned char clk_access3; /* 26 Max Access from Clock at CL=X-2 */
	unsigned char trp;         /* 27 Min Row Precharge Time (tRP)*/
	unsigned char trrd;        /* 28 Min Row Active to Row Active (tRRD) */
	unsigned char trcd;        /* 29 Min RAS to CAS Delay (tRCD) */
	unsigned char tras;        /* 30 Minimum RAS Pulse Width (tRAS) */
	unsigned char row_dens;    /* 31 Density of each row on module */
	unsigned char ca_setup;    /* 32 Cmd + Addr signal input setup time */
	unsigned char ca_hold;     /* 33 Cmd and Addr signal input hold time */
	unsigned char data_setup;  /* 34 Data signal input setup time */
	unsigned char data_hold;   /* 35 Data signal input hold time */
	unsigned char twr;         /* 36 Write Recovery time tWR */
	unsigned char twtr;        /* 37 Int write to read delay tWTR */
	unsigned char trtp;        /* 38 Int read to precharge delay tRTP */
	unsigned char mem_probe;   /* 39 Mem analysis probe characteristics */
	unsigned char trctrfc_ext; /* 40 Extensions to trc and trfc */
	unsigned char trc;         /* 41 Min Active to Auto refresh time tRC */
	unsigned char trfc;        /* 42 Min Auto to Active period tRFC */
	unsigned char tckmax;      /* 43 Max device cycle time tCKmax */
	unsigned char tdqsq;       /* 44 Max DQS to DQ skew */
	unsigned char tqhs;        /* 45 Max Read DataHold skew tQHS */
	unsigned char pll_relock;  /* 46 PLL Relock time */
	unsigned char res[15];     /* 47-xx IDD in SPD and Reserved space */
	unsigned char spd_rev;     /* 62 SPD Data Revision Code */
	unsigned char cksum;       /* 63 Checksum for bytes 0-62 */
	unsigned char mid[8];      /* 64 Mfr's JEDEC ID code per JEP-108E */
	unsigned char mloc;        /* 72 Manufacturing Location */
	unsigned char mpart[18];   /* 73 Manufacturer's Part Number */
	unsigned char rev[2];      /* 91 Revision Code */
	unsigned char mdate[2];    /* 93 Manufacturing Date */
	unsigned char sernum[4];   /* 95 Assembly Serial Number */
	unsigned char mspec[27];   /* 99 Manufacturer Specific Data */

	/*
	 * Open for Customer Use starting with byte 128.
	 */
	unsigned char freq;        /* 128 Intel spec: frequency */
	unsigned char intel_cas;   /* 129 Intel spec: CAS# Latency support */
} spd_eeprom_t;


/*
 * Byte 2 Fundamental Memory Types.
 */
#define SPD_MEMTYPE_FPM		(0x01)
#define SPD_MEMTYPE_EDO		(0x02)
#define SPD_MEMTYPE_PIPE_NIBBLE	(0x03)
#define SPD_MEMTYPE_SDRAM	(0x04)
#define SPD_MEMTYPE_ROM		(0x05)
#define SPD_MEMTYPE_SGRAM	(0x06)
#define SPD_MEMTYPE_DDR		(0x07)
#define SPD_MEMTYPE_DDR2	(0x08)

#endif /* _SPD_H_ */
