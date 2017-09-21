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

/*!
 * \file tdts_shell_ioctl.h
 * \brief tdts_shell char device to rcv/feedback ioctl commands. ioctl commands are also managed here.
 */
#ifndef TDTS_SHELL_IOCTL_H_
#define TDTS_SHELL_IOCTL_H_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/types.h>

#else
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h> // memset

#endif

#include "tdts/tmcfg.h"

#define TDTS_SHELL_IOCTL_CHRDEV_NAME TMCFG_E_KMOD_IOCTL_DEV_NAME
#define TDTS_SHELL_IOCTL_CHRDEV_PATH "/dev/" TDTS_SHELL_IOCTL_CHRDEV_NAME //!< The device node path in system.
#define TDTS_SHELL_IOCTL_CHRDEV_MAJOR TMCFG_E_KMOD_IOCTL_DEV_MAJ
#define TDTS_SHELL_IOCTL_CHRDEV_MINOR TMCFG_E_KMOD_IOCTL_DEV_MIN

#ifdef SHN_LICENSE_CONTROL
#define TMCFG_E_UDB_SHELL_IOCTL_DEV_MAJ 191
#define TMCFG_E_UDB_SHELL_IOCTL_DEV_NAME "idpfw"
#define UDB_SHELL_IOCTL_CHRDEV_NAME TMCFG_E_UDB_SHELL_IOCTL_DEV_NAME
#define UDB_SHELL_IOCTL_CHRDEV_PATH "/dev/" UDB_SHELL_IOCTL_CHRDEV_NAME //!< The device node path in system.
#define UDB_SHELL_IOCTL_CHRDEV_MAJOR TMCFG_E_UDB_SHELL_IOCTL_DEV_MAJ
#endif

/*!
 * \defgroup tdts_shell_ioctl_t tdts_shell ioctl
 * @{
 */

/*!
 * \brief tdts_shell ioctl.
 *
 * \warning Cannot exceed 128 bytes.
 */
typedef struct tdts_shell_ioctl
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
} __attribute__((packed)) tdts_shell_ioctl_t;

#ifdef SHN_LICENSE_CONTROL
typedef struct eudb_shell_ioctl
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
} eudb_shell_ioctl_t;
#endif

#define _IOCTL_CMD_R(_nr) _IOR(TMCFG_E_KMOD_IOCTL_DEV_MAGIC, _nr, tdts_shell_ioctl_t)
#define _IOCTL_CMD_W(_nr) _IOW(TMCFG_E_KMOD_IOCTL_DEV_MAGIC, _nr, tdts_shell_ioctl_t)
#define _IOCTL_CMD_WR(_nr) _IOWR(TMCFG_E_KMOD_IOCTL_DEV_MAGIC, _nr, tdts_shell_ioctl_t)

#ifdef SHN_LICENSE_CONTROL
//SHN for License control
#define _UDB_IOCTL_CMD_WR(_nr) _IOWR(UDB_SHELL_IOCTL_CHRDEV_MAJOR, _nr, eudb_shell_ioctl_t)
#endif

enum
{
	TDTS_SHELL_IOCTL_NR_NA = 0x00, //!< Not available.
	TDTS_SHELL_IOCTL_NR_DBG,  //!< Debug only
	TDTS_SHELL_IOCTL_NR_SIG,  //!< Signature handling
	TDTS_SHELL_IOCTL_NR_STAT, //!< Engine statistics report
	TDTS_SHELL_IOCTL_NR_MAX   //!< Max ioctl no. \warning Should not exceed 0xff.
};

#define TDTS_SHELL_IOCTL_NR_NA 0x00 //!< N/A. Do not use
#define TDTS_SHELL_IOCTL_CMD_DBG _IOCTL_CMD_WR(TDTS_SHELL_IOCTL_NR_DBG)   //!< \copydoc TDTS_SHELL_IOCTL_NR_DBG
#define TDTS_SHELL_IOCTL_CMD_SIG _IOCTL_CMD_WR(TDTS_SHELL_IOCTL_NR_SIG)   //!< \copydoc TDTS_SHELL_IOCTL_NR_SIG
#define TDTS_SHELL_IOCTL_CMD_STAT _IOCTL_CMD_WR(TDTS_SHELL_IOCTL_NR_STAT) //!< \copydoc TDTS_SHELL_IOCTL_NR_STAT

#ifdef SHN_LICENSE_CONTROL
//SHN for License control
#define UDB_SHELL_IOCTL_CMD_INTERNAL _UDB_IOCTL_CMD_WR(UDB_IOCTL_NR_INTERNAL)
#endif

/* Debug ioctl operations */
enum
{
	TDTS_SHELL_IOCTL_DBG_OP_NA = 0,
	TDTS_SHELL_IOCTL_DBG_OP_ECHO, //!< Print a message on console and do nothing.
	TDTS_SHELL_IOCTL_DBG_OP_MAX
};

/* Signature ioctl operations */
enum
{
	TDTS_SHELL_IOCTL_SIG_OP_NA = 0,
	TDTS_SHELL_IOCTL_SIG_OP_LOAD, //!< Load a raw signature input.
	TDTS_SHELL_IOCTL_SIG_OP_UNLOAD, //!< Unload an existed signature input.
	TDTS_SHELL_IOCTL_SIG_OP_GET_SIG_VER, //!< Get the signature version
	TDTS_SHELL_IOCTL_SIG_OP_GET_SIG_NUM, //!< Get the number of signature
	TDTS_SHELL_IOCTL_SIG_OP_GET_ANO_SEC_TBL_LEN, //!< Get the tbl len for copying the data
	TDTS_SHELL_IOCTL_SIG_OP_GET_ANO_SEC_TBL, //!< Copy the anomaly and security data for shared memory
	TDTS_SHELL_IOCTL_SIG_OP_FREE_SHARED_INFO_DATA, //!< Free the memory of shared memory (anomaly, security, etc.)
	TDTS_SHELL_IOCTL_SIG_OP_SET_STATE, //!< Enable/Disable Engine
	TDTS_SHELL_IOCTL_SIG_OP_GET_STATE, //!< Get the engine state
	TDTS_SHELL_IOCTL_SIG_OP_GET_DEVID_DATA_LEN, //!< Get the devid data len for copying data
	TDTS_SHELL_IOCTL_SIG_OP_GET_DEVID_DATA, //!< Get the devid data to generate devid db
	TDTS_SHELL_IOCTL_SIG_OP_GET_NR_CAT_NAME,
	TDTS_SHELL_IOCTL_SIG_OP_GET_CAT_NAME, //!< Get the cat data to generate cat db
	TDTS_SHELL_IOCTL_SIG_OP_GET_NR_BEH_NAME,
	TDTS_SHELL_IOCTL_SIG_OP_GET_BEH_NAME, //!< Get the behavior data to generate beh db
	TDTS_SHELL_IOCTL_SIG_OP_GET_NR_APP_NAME,
	TDTS_SHELL_IOCTL_SIG_OP_GET_APP_NAME, //!< Get the app data to generate app db
	TDTS_SHELL_IOCTL_SIG_OP_GET_NR_APP_ID,
	TDTS_SHELL_IOCTL_SIG_OP_GET_APP_DB, //!< Get the appid data to generate appid db
	TDTS_SHELL_IOCTL_SIG_OP_GET_BNDWTH_NUM, //!< Get the bandwidth table entry number to generate bndwth db
	TDTS_SHELL_IOCTL_SIG_OP_GET_BNDWTH_DB, //!< Get the bandwidth table entry to generate bndwth db
	TDTS_SHELL_IOCTL_SIG_OP_MAX
};

/* Engine statistics report operations */
enum
{
	TDTS_SHELL_IOCTL_STAT_OP_NA = 0,
	TDTS_SHELL_IOCTL_STAT_OP_GET_SPEC, //!< Get TDTS spec information (pool size, etc.)
	TDTS_SHELL_IOCTL_STAT_OP_GET_MATCHED_RULE, //!< Get the matched rule list (like nk_policy)
	TDTS_SHELL_IOCTL_STAT_OP_GET_ENG_STATUS, //!< Get basic engine info (eng ver, sig ver, etc.)
	TDTS_SHELL_IOCTL_STAT_OP_GET_TCP_CONN_NUM,
	TDTS_SHELL_IOCTL_STAT_OP_GET_RULE_MEM_USAGE,
	TDTS_SHELL_IOCTL_STAT_OP_MAX
};

/* Input/output type of value. */
typedef enum
{
	TDTS_SHELL_IOCTL_TYPE_NA = 0, //!< if no input/output, set type as N/A.
	TDTS_SHELL_IOCTL_TYPE_U32, //!< type is u32 (4 bytes unsigned)
	TDTS_SHELL_IOCTL_TYPE_RAW, //!< type is raw data, length must be specified correctly.
	TDTS_SHELL_IOCTL_TYPE_MAX
} tdts_shell_ioctl_type_t;

/*! magic */
#define TDTS_SHELL_IOCTL_MAGIC TMCFG_E_KMOD_IOCTL_DEV_MAGIC
#ifdef SHN_LICENSE_CONTROL
//SHN License control
#define UDB_SHELL_IOCTL_MAGIC UDB_SHELL_IOCTL_CHRDEV_MAJOR
#endif

/*!@}*/

////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Init a declared ioctl structure.
 */
#define tdts_shell_init_ioctl_entry(_ioc) \
	do { memset(_ioc, 0x00, sizeof(*(_ioc))); (_ioc)->magic = TDTS_SHELL_IOCTL_MAGIC; } while (0)

#ifdef SHN_LICENSE_CONTROL
//SHN License control
#define udb_shell_init_ioctl_entry(_ioc) \
       do { memset(_ioc, 0x00, sizeof(*(_ioc))); (_ioc)->magic = UDB_SHELL_IOCTL_MAGIC; } while (0)
#endif

/*!
 * \brief Set ioctl input as TDTS_SHELL_IOCTL_TYPE_RAW type.
 */
#define tdts_shell_ioctl_set_in_raw(_ioc, _buf, _buf_len) \
	do { \
		(_ioc)->in_type = TDTS_SHELL_IOCTL_TYPE_RAW; \
		(_ioc)->in_raw = (uintptr_t) _buf; \
		(_ioc)->in_len = (_buf_len); \
	} while (0)

/*!
 * \brief Set ioctl input as TDTS_SHELL_IOCTL_TYPE_U32 type.
 */
#define tdts_shell_ioctl_set_in_u32(_ioc, _u32) \
	do { \
		(_ioc)->in_type = TDTS_SHELL_IOCTL_TYPE_U32; \
		(_ioc)->in_len = sizeof(((tdts_shell_ioctl_t *) 0)->in_u32); \
		(_ioc)->in_u32 = _u32; \
	} while (0)

/*!
 * \brief Set ioctl output buffer.
 */
#define tdts_shell_ioctl_set_out_buf(_ioc, _buf, _buf_len, _buf_used_len_p) \
	do { \
		(_ioc)->out = (uintptr_t) _buf; \
		(_ioc)->out_len = _buf_len; \
		(_ioc)->out_used_len = (uintptr_t) _buf_used_len_p; \
	} while (0)

#ifdef SHN_LICENSE_CONTROL
#ifndef uintptr_t
#define uintptr_t unsigned long
#endif
#define udb_shell_ioctl_set_out_buf(_ioc, _buf, _buf_len, _buf_used_len_p) \
	do { \
		(_ioc)->out = (uintptr_t) _buf; \
		(_ioc)->out_len = _buf_len; \
		(_ioc)->out_used_len = (uintptr_t) _buf_used_len_p; \
	} while (0)
#endif

////////////////////////////////////////////////////////////////////////////////
#ifdef __KERNEL__

typedef int (*tdts_shell_ioctl_cb_t)(tdts_shell_ioctl_t *ioc, uint8_t is_ioc_ro);

int tdts_shell_ioctl_init(void);
void tdts_shell_ioctl_cleanup(void);

unsigned long tdts_shell_ioctl_copy_out(tdts_shell_ioctl_t *ioc, void *buf, uint32_t buf_len);
#endif // __KERNEL__

#endif /* TDTS_SHELL_IOCTL_H_ */
