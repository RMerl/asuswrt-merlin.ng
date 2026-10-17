/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _INCLUDE_LIBFDT_H_
#define _INCLUDE_LIBFDT_H_

#ifndef USE_HOSTCC
#include <linux/libfdt_env.h>
#endif
#include "../../scripts/dtc/libfdt/libfdt.h"

/* U-Boot local hacks */
extern struct fdt_header *working_fdt;  /* Pointer to the working fdt */

#endif /* _INCLUDE_LIBFDT_H_ */
