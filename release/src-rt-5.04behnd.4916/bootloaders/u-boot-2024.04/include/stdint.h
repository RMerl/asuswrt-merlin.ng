/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Dummy file to allow libraries linked with U-Boot to include stdint.h without
 * getting the system version.
 *
 * U-Boot uses linux types (linux/types.h) so does not make use of stdint.h
 */

#ifndef __UB_STDINT_H
#define __UB_STDINT_H

#define UINT8_MAX	0xff

#endif
