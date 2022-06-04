#ifndef LSMTD_H
#define LSMTD_H

#define PROGRAM_NAME "lsmtd"
#include "common.h"
#include "xalloc.h"

#include <libmtd.h>
#include <libubi.h>

#define COL_DEVNAME 1
#define COL_DEVNUM 2
#define COL_TYPE 3
#define COL_NAME 4
#define COL_SIZE 5
#define COL_EBSIZE 6
#define COL_EBCOUNT 7
#define COL_MINIO 8
#define COL_SUBSIZE 9
#define COL_OOBSIZE 10
#define COL_MAXEC 11
#define COL_FREE 12
#define COL_FREE_LEB 13
#define COL_BAD_COUNT 14
#define COL_BAD_RSVD 15
#define COL_RO 16
#define COL_BB 17
#define COL_REGION 18
#define COL_CORRUPTED 19

#define COL_DT_STRING 1
#define COL_DT_NUMBER 2
#define COL_DT_SIZE 3
#define COL_DT_BOOL 4

struct ubi_node {
	struct ubi_dev_info info;
	struct ubi_vol_info *vol_info;
};

struct mtd_node {
	struct mtd_dev_info info;
	struct ubi_node *ubi;
};

struct column {
	const char *name;
	const char *desc;
	int type;
	int datatype;
	size_t width;
};

extern struct ubi_node *ubi_dev;
extern int num_ubi_devices;

extern struct mtd_node *mtd_dev;
extern int num_mtd_devices;

extern struct column *sort_by;

int scan_mtd(libmtd_t lib_mtd);
int scan_ubi(libubi_t lib_ubi);
void scan_free(void);

#endif /* LSMTD_H */

