/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef _SHELL_IOCTL_H_
#define _SHELL_IOCTL_H_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h> // memset
#endif

#include "udb/tmcfg_udb.h"

#ifdef __KERNEL___
#include "udb/tdts_udb_core.h"
#endif

#include "udb/ioctl/udb_ioctl.h"

#if !defined(TMCFG_E_UDB_SHELL_IOCTL_DEV_NAME) || !defined(TMCFG_E_UDB_SHELL_IOCTL_DEV_MAJ) || !defined(TMCFG_E_UDB_SHELL_IOCTL_DEV_MIN)
#error "please check your ioctl configs in menuconfig!"
#endif

#define UDB_SHELL_IOCTL_CHRDEV_NAME TMCFG_E_UDB_SHELL_IOCTL_DEV_NAME
#define UDB_SHELL_IOCTL_CHRDEV_PATH "/dev/" UDB_SHELL_IOCTL_CHRDEV_NAME //!< The device node path in system.
#define UDB_SHELL_IOCTL_CHRDEV_MAJOR TMCFG_E_UDB_SHELL_IOCTL_DEV_MAJ
#define UDB_SHELL_IOCTL_CHRDEV_MINOR TMCFG_E_UDB_SHELL_IOCTL_DEV_MIN

/*! magic */
#define UDB_SHELL_IOCTL_MAGIC TMCFG_E_UDB_SHELL_IOCTL_DEV_MAGIC

/*!
 * \brief tdts_shell ioctl.
 *
 * \warning Cannot exceed 128 bytes.
 */
typedef struct udb_shell_ioctl
{
	uint32_t magic; //!< A fixed magic number to identify if this structure is for this module.

	uint8_t nr; //!< nr, ioctl nr to know which sub-system you want to call, e.g. bandwidth (bw), or other tables.
	uint8_t op; //!< op, the operation to run on the sub-system, e.g. set, reset, add, etc.

	uint8_t rsv[1];

	/* Input (user to kernel) */
	uint8_t in_type; //!< \sa tdts_shell_ioctl_type_t
	union
	{
		uint64_t in_raw; //!< use to store address to avoid the issue of 32bit user program in 64bit kernel.
		uint32_t in_u32;
	};

	uint32_t in_len; //!< Input length (bytes). Plz specify this value correctly.

	/* (Optional) Output (kernel to user) */
	uint64_t out; //!< Output buffer pointer (sent to kernel to save data)
	//!< use to store address to avoid the issue of 32bit user program in 64bit kernel.
	uint64_t out_used_len; //!< Output buffer used size
	 //!< use to store address to avoid the issue of 32bit user program in 64bit kernel.
	uint32_t out_len; //!< Available output length

	uint8_t rsv2[16]; //!< Reserve for future use
} udb_shell_ioctl_t;

#define _UDB_IOCTL_CMD_R(_nr) _IOR(UDB_SHELL_IOCTL_MAGIC, _nr, udb_shell_ioctl_t)
#define _UDB_IOCTL_CMD_W(_nr) _IOW(UDB_SHELL_IOCTL_MAGIC, _nr, udb_shell_ioctl_t)
#define _UDB_IOCTL_CMD_WR(_nr) _IOWR(UDB_SHELL_IOCTL_MAGIC, _nr, udb_shell_ioctl_t)

#define UDB_SHELL_IOCTL_CMD_NA		0x00 //!< N/A. Do not use
#define UDB_SHELL_IOCTL_CMD_INTERNAL 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_INTERNAL)
#define UDB_SHELL_IOCTL_CMD_COMMON 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_COMMON)
#define UDB_SHELL_IOCTL_CMD_PATROL_TQ 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_PATROL_TQ)
#define UDB_SHELL_IOCTL_CMD_WRS 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_WRS)
#define UDB_SHELL_IOCTL_CMD_WBL 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_WBL)
#define UDB_SHELL_IOCTL_CMD_APP_WBL 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_APP_WBL)
#define UDB_SHELL_IOCTL_CMD_IQOS 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_IQOS)
#define UDB_SHELL_IOCTL_CMD_GCTRL       _UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_GCTRL)
#define UDB_SHELL_IOCTL_CMD_VP 		_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_VP)
#define UDB_SHELL_IOCTL_CMD_ANOMALY 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_ANOMALY)
#define UDB_SHELL_IOCTL_CMD_DLOG 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_DLOG)
#define UDB_SHELL_IOCTL_CMD_HWNAT 	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_HWNAT)
#define UDB_SHELL_IOCTL_CMD_MESH	_UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_MESH)
/* Input/output type of value. */
typedef enum
{
	UDB_SHELL_IOCTL_TYPE_NA = 0, //!< if no input/output, set type as N/A.
	UDB_SHELL_IOCTL_TYPE_U32, //!< type is u32 (4 bytes unsigned)
	UDB_SHELL_IOCTL_TYPE_RAW, //!< type is raw data, length must be specified correctly.
	UDB_SHELL_IOCTL_TYPE_MAX
} udb_shell_ioctl_type_t;

/*!@}*/

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Init a declared ioctl structure.
 */
#define udb_shell_init_ioctl_entry(_ioc) \
	do { memset(_ioc, 0x00, sizeof(*(_ioc))); (_ioc)->magic = UDB_SHELL_IOCTL_MAGIC; } while (0)

/*!
 * \brief Set ioctl input as TDTS_SHELL_IOCTL_TYPE_RAW type.
 */
#define udb_shell_ioctl_set_in_raw(_ioc, _buf, _buf_len) \
	do { \
		(_ioc)->in_type = UDB_SHELL_IOCTL_TYPE_RAW; \
		(_ioc)->in_raw = (uintptr_t) _buf; \
		(_ioc)->in_len = (_buf_len); \
	} while (0)

/*!
 * \brief Set ioctl input as TDTS_SHELL_IOCTL_TYPE_U32 type.
 */
#define udb_shell_ioctl_set_in_u32(_ioc, _u32) \
	do { \
		(_ioc)->in_type = UDB_SHELL_IOCTL_TYPE_U32; \
		(_ioc)->in_len = sizeof(((udb_shell_ioctl_t *) 0)->in_u32); \
		(_ioc)->in_u32 = _u32; \
	} while (0)

/*!
 * \brief Set ioctl output buffer.
 */
#define udb_shell_ioctl_set_out_buf(_ioc, _buf, _buf_len, _buf_used_len_p) \
	do { \
		(_ioc)->out = (uintptr_t) _buf; \
		(_ioc)->out_len = _buf_len; \
		(_ioc)->out_used_len = (uintptr_t) _buf_used_len_p; \
	} while (0)

////////////////////////////////////////////////////////////////////////////////
#ifdef __KERNEL__

#define VMALLOC_INIT(_size) \
({ \
	void *ptr; \
	if ((ptr = vmalloc(_size))) \
	{ \
		memset(ptr, 0x00, _size); \
	} \
	ptr; \
})

#define VFREE_INIT(_p, _size) \
do { \
	if (likely(NULL != _p)) \
	{ \
		vfree(_p); \
		_p = NULL; \
	} \
} while (0)

// TODO: Add memory counters

#define MALLOC_IOC_OUT(_ioc, _buf) \
do { \
	if (unlikely(!_ioc->out || _ioc->out_len <= 0 || !_ioc->out_used_len)) \
	{ \
		ERR("Invalid output argument: %p %u bytes", \
			(void *)(uintptr_t)_ioc->out, _ioc->out_len); \
		return -EINVAL; \
	} \
	_buf = VMALLOC_INIT(_ioc->out_len); \
	if (unlikely(!_buf)) \
	{ \
		ERR("Cannot malloc container %u bytes", _ioc->out_len); \
		return -ENOMEM; \
	} \
} while (0)	

#define MALLOC_IOC_IN(_ioc, _buf) \
do { \
	if (unlikely(!_ioc->in_raw || _ioc->in_len <= 0 \
		|| _ioc->in_type != UDB_SHELL_IOCTL_TYPE_RAW)) \
	{ \
		ERR("Invalid output argument: %p %u bytes type %u.", \
			(void *)(uintptr_t)_ioc->in_raw, _ioc->in_len, _ioc->in_type); \
		return -EINVAL; \
	} \
	_buf = VMALLOC_INIT(_ioc->in_len); \
	if (unlikely(!_buf)) \
	{ \
		ERR("Cannot malloc container %u bytes", _ioc->in_len); \
		return -ENOMEM; \
	} \
} while (0)


int udb_shell_ioctl_init(void);
void udb_shell_ioctl_cleanup(void);
#endif // __KERNEL__

#endif /* _SHELL_IOCTL_H_ */
