/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * (C) Copyright 2014, Cavium Inc.
**/

#ifndef __THUNDERX_SVC_H__
#define __THUNDERX_SVC_H__

/* SMC function IDs for general purpose queries */

#define THUNDERX_SVC_CALL_COUNT		0x4300ff00
#define THUNDERX_SVC_UID		0x4300ff01

#define THUNDERX_SVC_VERSION		0x4300ff03

#define ARM_STD_SVC_VERSION		0x8400ff03

/* ThunderX Service Calls version numbers */
#define THUNDERX_VERSION_MAJOR	0x0
#define THUNDERX_VERSION_MINOR	0x1

#define THUNDERX_MMC_READ		0x43000101
/* x1 - block address
 * x2 - size
 * x3 - buffer address
 */
#define THUNDERX_MMC_WRITE		0x43000102
/* x1 - block address
 * x2 - size
 * x3 - buffer address
 */

#define THUNDERX_NOR_READ		0x43000111
/* x1 - block address
 * x2 - size
 * x3 - buffer address
 */
#define THUNDERX_NOR_WRITE		0x43000112
/* x1 - block address
 * x2 - size
 * x3 - buffer address
 */
#define THUNDERX_NOR_ERASE		0x43000113
/* x1 - block address
 */

#define THUNDERX_PART_COUNT		0x43000201
#define THUNDERX_GET_PART		0x43000202
/* x1 - pointer to the buffer
 * x2 - index
 */

#define THUNDERX_DRAM_SIZE		0x43000301
/* x1 - node number
 */

#define THUNDERX_GTI_SYNC		0x43000401

#define THUNDERX_ENV_COUNT		0x43000501
#define THUNDERX_ENV_STRING		0x43000502
/* x1 - index
 */

#define THUNDERX_NODE_COUNT		0x43000601

#endif /* __THUNDERX_SVC_H__ */
