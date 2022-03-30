/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#include <linux/types.h>	       /* for ulong typedef */

#ifndef _FPGA_H_
#define _FPGA_H_

#ifndef CONFIG_MAX_FPGA_DEVICES
#define CONFIG_MAX_FPGA_DEVICES		5
#endif

/* fpga_xxxx function return value definitions */
#define FPGA_SUCCESS		0
#define FPGA_FAIL		-1

/* device numbers must be non-negative */
#define FPGA_INVALID_DEVICE	-1

#define FPGA_ENC_USR_KEY	1
#define FPGA_NO_ENC_OR_NO_AUTH	2

/* root data type defintions */
typedef enum {			/* typedef fpga_type */
	fpga_min_type,		/* range check value */
	fpga_xilinx,		/* Xilinx Family) */
	fpga_altera,		/* unimplemented */
	fpga_lattice,		/* Lattice family */
	fpga_undefined		/* invalid range check value */
} fpga_type;			/* end, typedef fpga_type */

typedef struct {		/* typedef fpga_desc */
	fpga_type devtype;	/* switch value to select sub-functions */
	void *devdesc;		/* real device descriptor */
} fpga_desc;			/* end, typedef fpga_desc */

typedef struct {                /* typedef fpga_desc */
	unsigned int blocksize;
	char *interface;
	char *dev_part;
	const char *filename;
	int fstype;
} fpga_fs_info;

struct fpga_secure_info {
	u8 *userkey_addr;
	u8 authflag;
	u8 encflag;
};

typedef enum {
	BIT_FULL = 0,
	BIT_PARTIAL,
	BIT_NONE = 0xFF,
} bitstream_type;

/* root function definitions */
void fpga_init(void);
int fpga_add(fpga_type devtype, void *desc);
int fpga_count(void);
const fpga_desc *const fpga_get_desc(int devnum);
int fpga_is_partial_data(int devnum, size_t img_len);
int fpga_load(int devnum, const void *buf, size_t bsize,
	      bitstream_type bstype);
int fpga_fsload(int devnum, const void *buf, size_t size,
		fpga_fs_info *fpga_fsinfo);
int fpga_loads(int devnum, const void *buf, size_t size,
	       struct fpga_secure_info *fpga_sec_info);
int fpga_loadbitstream(int devnum, char *fpgadata, size_t size,
		       bitstream_type bstype);
int fpga_dump(int devnum, const void *buf, size_t bsize);
int fpga_info(int devnum);
const fpga_desc *const fpga_validate(int devnum, const void *buf,
				     size_t bsize, char *fn);

#endif	/* _FPGA_H_ */
