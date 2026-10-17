/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017-2018, Intel Corporation
 */

#ifndef __INTEL_SMC_H
#define __INTEL_SMC_H

#include <linux/arm-smccc.h>
#include <linux/bitops.h>

/*
 * This file defines the Secure Monitor Call (SMC) message protocol used for
 * service layer driver in normal world (EL1) to communicate with secure
 * monitor software in Secure Monitor Exception Level 3 (EL3).
 *
 * This file is shared with secure firmware (FW) which is out of u-boot tree.
 *
 * An ARM SMC instruction takes a function identifier and up to 6 64-bit
 * register values as arguments, and can return up to 4 64-bit register
 * values. The operation of the secure monitor is determined by the parameter
 * values passed in through registers.

 * EL1 and EL3 communicates pointer as physical address rather than the
 * virtual address.
 */

/*
 * Functions specified by ARM SMC Calling convention:
 *
 * FAST call executes atomic operations, returns when the requested operation
 * has completed.
 * STD call starts a operation which can be preempted by a non-secure
 * interrupt. The call can return before the requested operation has
 * completed.
 *
 * a0..a7 is used as register names in the descriptions below, on arm32
 * that translates to r0..r7 and on arm64 to w0..w7.
 */

#define INTEL_SIP_SMC_STD_CALL_VAL(func_num) \
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_STD_CALL, ARM_SMCCC_SMC_64, \
	ARM_SMCCC_OWNER_SIP, (func_num))

#define INTEL_SIP_SMC_FAST_CALL_VAL(func_num) \
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_64, \
	ARM_SMCCC_OWNER_SIP, (func_num))

/*
 * Return values in INTEL_SIP_SMC_* call
 *
 * INTEL_SIP_SMC_RETURN_UNKNOWN_FUNCTION:
 * Secure monitor software doesn't recognize the request.
 *
 * INTEL_SIP_SMC_STATUS_OK:
 * SMC call completed successfully,
 * In case of FPGA configuration write operation, it means secure monitor
 * software can accept the next chunk of FPGA configuration data.
 *
 * INTEL_SIP_SMC_STATUS_BUSY:
 * In case of FPGA configuration write operation, it means secure monitor
 * software is still processing previous data & can't accept the next chunk
 * of data. Service driver needs to issue
 * INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE call to query the
 * completed block(s).
 *
 * INTEL_SIP_SMC_STATUS_ERROR:
 * There is error during the SMC call process.
 *
 * INTEL_SIP_SMC_REG_ERROR:
 * There is error during a read or write operation of the protected
 * registers.
 */
#define INTEL_SIP_SMC_RETURN_UNKNOWN_FUNCTION		0xFFFFFFFF
#define INTEL_SIP_SMC_STATUS_OK				0x0
#define INTEL_SIP_SMC_STATUS_BUSY			0x1
#define INTEL_SIP_SMC_STATUS_REJECTED			0x2
#define INTEL_SIP_SMC_STATUS_ERROR			0x4
#define INTEL_SIP_SMC_REG_ERROR				0x5
#define INTEL_SIP_SMC_RSU_ERROR				0x7

/*
 * Request INTEL_SIP_SMC_FPGA_CONFIG_START
 *
 * Sync call used by service driver at EL1 to request the FPGA in EL3 to
 * be prepare to receive a new configuration.
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_FPGA_CONFIG_START.
 * a1: flag for full or partial configuration
 *    0 full reconfiguration.
 *    1 partial reconfiguration.
 * a2-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK, or INTEL_SIP_SMC_STATUS_ERROR.
 * a1-3: not used.
 */
#define INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_START 1
#define INTEL_SIP_SMC_FPGA_CONFIG_START \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_START)

/*
 * Request INTEL_SIP_SMC_FPGA_CONFIG_WRITE
 *
 * Async call used by service driver at EL1 to provide FPGA configuration data
 * to secure world.
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_FPGA_CONFIG_WRITE.
 * a1: 64bit physical address of the configuration data memory block
 * a2: Size of configuration data block.
 * a3-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK, INTEL_SIP_SMC_STATUS_BUSY,
 * INTEL_SIP_SMC_STATUS_REJECTED or INTEL_SIP_SMC_STATUS_ERROR.
 * a1: 64bit physical address of 1st completed memory block if any completed
 * block, otherwise zero value.
 * a2: 64bit physical address of 2nd completed memory block if any completed
 * block, otherwise zero value.
 * a3: 64bit physical address of 3rd completed memory block if any completed
 * block, otherwise zero value.
 */
#define INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_WRITE 2
#define INTEL_SIP_SMC_FPGA_CONFIG_WRITE \
	INTEL_SIP_SMC_STD_CALL_VAL(INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_WRITE)

/*
 * Request INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE
 *
 * Sync call used by service driver at EL1 to track the completed write
 * transactions. This request is called after INTEL_SIP_SMC_FPGA_CONFIG_WRITE
 * call returns INTEL_SIP_SMC_STATUS_BUSY.
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE.
 * a1-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK, INTEL_SIP_SMC_STATUS_BUSY or
 * INTEL_SIP_SMC_STATUS_ERROR.
 * a1: 64bit physical address of 1st completed memory block.
 * a2: 64bit physical address of 2nd completed memory block if
 * any completed block, otherwise zero value.
 * a3: 64bit physical address of 3rd completed memory block if
 * any completed block, otherwise zero value.
 */
#define INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_COMPLETED_WRITE 3
#define INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE \
INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_COMPLETED_WRITE)

/*
 * Request INTEL_SIP_SMC_FPGA_CONFIG_ISDONE
 *
 * Sync call used by service driver at EL1 to inform secure world that all
 * data are sent, to check whether or not the secure world had completed
 * the FPGA configuration process.
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_FPGA_CONFIG_ISDONE.
 * a1-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK, INTEL_SIP_SMC_STATUS_BUSY or
 * INTEL_SIP_SMC_STATUS_ERROR.
 * a1-3: not used.
 */
#define INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_ISDONE 4
#define INTEL_SIP_SMC_FPGA_CONFIG_ISDONE \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_ISDONE)

/*
 * Request INTEL_SIP_SMC_FPGA_CONFIG_GET_MEM
 *
 * Sync call used by service driver at EL1 to query the physical address of
 * memory block reserved by secure monitor software.
 *
 * Call register usage:
 * a0:INTEL_SIP_SMC_FPGA_CONFIG_GET_MEM.
 * a1-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_STATUS_ERROR.
 * a1: start of physical address of reserved memory block.
 * a2: size of reserved memory block.
 * a3: not used.
 */
#define INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_GET_MEM 5
#define INTEL_SIP_SMC_FPGA_CONFIG_GET_MEM \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_GET_MEM)

/*
 * Request INTEL_SIP_SMC_FPGA_CONFIG_LOOPBACK
 *
 * For SMC loop-back mode only, used for internal integration, debugging
 * or troubleshooting.
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_FPGA_CONFIG_LOOPBACK.
 * a1-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_STATUS_ERROR.
 * a1-3: not used.
 */
#define INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_LOOPBACK 6
#define INTEL_SIP_SMC_FPGA_CONFIG_LOOPBACK \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_FPGA_CONFIG_LOOPBACK)

/*
 * Request INTEL_SIP_SMC_REG_READ
 *
 * Read a protected register using SMCCC
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_REG_READ.
 * a1: register address.
 * a2-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_REG_ERROR.
 * a1: Value in the register
 * a2-3: not used.
 */
#define INTEL_SIP_SMC_FUNCID_REG_READ 7
#define INTEL_SIP_SMC_REG_READ \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_REG_READ)

/*
 * Request INTEL_SIP_SMC_REG_WRITE
 *
 * Write a protected register using SMCCC
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_REG_WRITE.
 * a1: register address
 * a2: value to program into register.
 * a3-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_REG_ERROR.
 * a1-3: not used.
 */
#define INTEL_SIP_SMC_FUNCID_REG_WRITE 8
#define INTEL_SIP_SMC_REG_WRITE \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_REG_WRITE)

/*
 * Request INTEL_SIP_SMC_FUNCID_REG_UPDATE
 *
 * Update one or more bits in a protected register using a
 * read-modify-write operation.
 *
 * Call register usage:
 * a0: INTEL_SIP_SMC_REG_UPDATE.
 * a1: register address
 * a2: Write Mask.
 * a3: Value to write.
 * a4-7: not used.
 *
 * Return status:
 * a0: INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_REG_ERROR.
 * a1-3: Not used.
 */
#define INTEL_SIP_SMC_FUNCID_REG_UPDATE 9
#define INTEL_SIP_SMC_REG_UPDATE \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_REG_UPDATE)

/*
* Request INTEL_SIP_SMC_RSU_STATUS
*
* Sync call used by service driver at EL1 to query the RSU status
*
* Call register usage:
* a0 INTEL_SIP_SMC_RSU_STATUS
* a1-7 not used
*
* Return status
* a0: Current Image
* a1: Last Failing Image
* a2: Version [width 32 bit] | State [width 32 bit]
* a3: Error details [width 32 bit] | Error location [width 32 bit]
*
* Or
*
* a0: INTEL_SIP_SMC_RSU_ERROR
*/
#define INTEL_SIP_SMC_FUNCID_RSU_STATUS 11
#define INTEL_SIP_SMC_RSU_STATUS \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_STATUS)

/*
* Request INTEL_SIP_SMC_RSU_UPDATE
*
* Sync call used by service driver at EL1 to tell you next reboot is RSU_UPDATE
*
* Call register usage:
* a0 INTEL_SIP_SMC_RSU_UPDATE
* a1 64bit physical address of the configuration data memory in flash
* a2-7 not used
*
* Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
*/
#define INTEL_SIP_SMC_FUNCID_RSU_UPDATE 12
#define INTEL_SIP_SMC_RSU_UPDATE \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_UPDATE)

/*
 * Request INTEL_SIP_SMC_ECC_DBE
 *
 * Sync call used by service driver at EL1 alert EL3 that a Double Bit
 * ECC error has occurred.
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_ECC_DBE
 * a1 SysManager Double Bit Error value
 * a2-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 */
#define INTEL_SIP_SMC_FUNCID_ECC_DBE 13
#define INTEL_SIP_SMC_ECC_DBE \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_ECC_DBE)

/*
* Request INTEL_SIP_SMC_RSU_NOTIFY
*
* Sync call used by service driver at EL1 to report HPS software execution stage
*
* Call register usage:
* a0 INTEL_SIP_SMC_RSU_NOTIFY
* a1 32bit HPS software execution stage
* a2-7 not used
*
* Return status
 * a0 INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_REG_ERROR.
*/
#define INTEL_SIP_SMC_FUNCID_RSU_NOTIFY 14
#define INTEL_SIP_SMC_RSU_NOTIFY \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_NOTIFY)

/*
 * Request INTEL_SIP_SMC_RSU_RETRY_COUNTER
 *
 * Sync call used by service driver at EL1 to query the RSU retry counter
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_RSU_RETRY_COUNTER
 * a1-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_RSU_ERROR.
 * a1 retry counter
*/
#define INTEL_SIP_SMC_FUNCID_RSU_RETRY_COUNTER 15
#define INTEL_SIP_SMC_RSU_RETRY_COUNTER \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_RETRY_COUNTER)

/*
 * Request INTEL_SIP_SMC_RSU_DCMF_VERSION
 *
 * Sync call used by service driver at EL1 to query DCMF version
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_RSU_DCMF_VERSION
 * a1-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 * a1 dcmf1 version | dcmf0 version
 * a2 dcmf3 version | dcmf2 version
 *
 * Or
 *
 * a0 INTEL_SIP_SMC_RSU_ERROR
 */
#define INTEL_SIP_SMC_FUNCID_RSU_DCMF_VERSION 16
#define INTEL_SIP_SMC_RSU_DCMF_VERSION \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_DCMF_VERSION)

/*
 * Request INTEL_SIP_SMC_RSU_COPY_DCMF_VERSION
 *
 * Sync call used by SSBL (EL2) to copy DCMF version to ATF memory
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_RSU_COPY_DCMF_VERSION
 * a1 dcmf1 version | dcmf0 version
 * a2 dcmf3 version | dcmf2 version
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 */
#define INTEL_SIP_SMC_FUNCID_RSU_COPY_DCMF_VERSION 17
#define INTEL_SIP_SMC_RSU_COPY_DCMF_VERSION \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_COPY_DCMF_VERSION)

/*
 * Request INTEL_SIP_SMC_RSU_MAX_RETRY
 *
 * Sync call used by service driver at EL1 to query max_retry parameter
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_RSU_MAX_RETRY
 * a1-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 * a1 max_retry
 *
 * Or
 *
 * a0 INTEL_SIP_SMC_RSU_ERROR
 */
#define INTEL_SIP_SMC_FUNCID_RSU_MAX_RETRY 18
#define INTEL_SIP_SMC_RSU_MAX_RETRY \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_MAX_RETRY)

/*
 * Request INTEL_SIP_SMC_RSU_COPY_MAX_RETRY
 *
 * Sync call used by SSBL (EL2) to copy RSU 'max retry' to ATF memory
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_RSU_COPY_MAX_RETRY
 * a1 max retry
 * a2-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 */
#define INTEL_SIP_SMC_FUNCID_RSU_COPY_MAX_RETRY 19
#define INTEL_SIP_SMC_RSU_COPY_MAX_RETRY \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_COPY_MAX_RETRY)

/*
 * Request INTEL_SIP_SMC_RSU_DCMF_STATUS
 *
 * Sync call used by service driver at EL1 to query DCMF status
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_RSU_DCMF_STATUS
 * a1-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 * a1 dcmf3 status | dcmf2 status | dcmf1 status | dcmf0 status
 *
 * Or
 *
 * a0 INTEL_SIP_SMC_RSU_ERROR
 */
#define INTEL_SIP_SMC_FUNCID_RSU_DCMF_STATUS 20
#define INTEL_SIP_SMC_RSU_DCMF_STATUS \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_DCMF_STATUS)

/*
 * Request INTEL_SIP_SMC_RSU_COPY_DCMF_STATUS
 *
 * Sync call used by SSBL (EL2) to copy RSU 'dcmf status' to ATF memory
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_RSU_COPY_DCMF_STATUS
 * a1 dcmf status
 * a2-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 */
#define INTEL_SIP_SMC_FUNCID_RSU_COPY_DCMF_STATUS 21
#define INTEL_SIP_SMC_RSU_COPY_DCMF_STATUS \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_RSU_COPY_DCMF_STATUS)

/*
 * Request INTEL_SIP_SMC_HPS_SET_BRIDGES
 *
 * Enable/disable the SoC FPGA bridges
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_HPS_SET_BRIDGES
 * a1 Set bridges status:
 *      0 - Disable
 *      1 - Enable
 * a2-7 not used
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK
 */
#define INTEL_SIP_SMC_FUNCID_HPS_SET_BRIDGES	50
#define INTEL_SIP_SMC_HPS_SET_BRIDGES \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_HPS_SET_BRIDGES)

/*
 * Request INTEL_SIP_SMC_MBOX_SEND_CMD
 *
 * Send mailbox command to SDM
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_MBOX_SEND_CMD
 * a1 Mailbox command
 * a2 64bit physical address pointer to command's arguments
 * a3 Length of the argument
 * a4 Urgent command:
 *      0 - Disable
 *      1 - Enable
 * a5 64bit physical address pointer to a buffer for receiving responses
 * a6 Length of the buffer
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_STATUS_ERROR
 * a1 Status of mailbox response
 * a2 Received length in the buffer
 */
#define INTEL_SIP_SMC_FUNCID_MBOX_SEND_CMD	60
#define INTEL_SIP_SMC_MBOX_SEND_CMD \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_MBOX_SEND_CMD)

/*
 * Request INTEL_SIP_SMC_GET_USERCODE
 *
 * Send mailbox command to get usercode from SDM
 *
 * Call register usage:
 * a0 INTEL_SIP_SMC_GET_USERCODE
 * a1-7 not used.
 *
 * Return status
 * a0 INTEL_SIP_SMC_STATUS_OK or INTEL_SIP_SMC_STATUS_ERROR
 * a1 User code
 * a2-3 not used.
 */
#define INTEL_SIP_SMC_FUNCID_GET_USERCODE	61
#define INTEL_SIP_SMC_GET_USERCODE \
	INTEL_SIP_SMC_FAST_CALL_VAL(INTEL_SIP_SMC_FUNCID_GET_USERCODE)

#endif
