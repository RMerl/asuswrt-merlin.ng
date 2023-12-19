/*
 * common.h - common code header
 *
 * Declarations for data and functions shared by ioctl and netlink code.
 */

#ifndef ETHTOOL_COMMON_H__
#define ETHTOOL_COMMON_H__

#include "internal.h"
#include <stddef.h>
#include <errno.h>

#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))

struct flag_info {
	const char *name;
	u32 value;
};

extern const struct flag_info flags_msglvl[];
extern const unsigned int n_flags_msglvl;

struct off_flag_def {
	const char *short_name;
	const char *long_name;
	const char *kernel_name;
	u32 get_cmd, set_cmd;
	u32 value;
	/* For features exposed through ETHTOOL_GFLAGS, the oldest
	 * kernel version for which we can trust the result.  Where
	 * the flag was added at the same time the kernel started
	 * supporting the feature, this is 0 (to allow for backports).
	 * Where the feature was supported before the flag was added,
	 * it is the version that introduced the flag.
	 */
	u32 min_kernel_ver;
};

#define OFF_FLAG_DEF_SIZE 12
extern const struct off_flag_def off_flag_def[OFF_FLAG_DEF_SIZE];

void print_flags(const struct flag_info *info, unsigned int n_info, u32 value);
int dump_wol(struct ethtool_wolinfo *wol);
void dump_mdix(u8 mdix, u8 mdix_ctrl);
void print_indir_table(struct cmd_context *ctx, u64 ring_count,
		       u32 indir_size, u32 *indir);
void print_rss_hkey(u8 *hkey, u32 hkey_size);
#endif /* ETHTOOL_COMMON_H__ */
