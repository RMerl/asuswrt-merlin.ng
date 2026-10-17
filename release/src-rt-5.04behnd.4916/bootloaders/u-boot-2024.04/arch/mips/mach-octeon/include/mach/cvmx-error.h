/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the Octeon extended error status.
 */

#ifndef __CVMX_ERROR_H__
#define __CVMX_ERROR_H__

/**
 * There are generally many error status bits associated with a
 * single logical group. The enumeration below is used to
 * communicate high level groups to the error infastructure so
 * error status bits can be enable or disabled in large groups.
 */
typedef enum {
	CVMX_ERROR_GROUP_INTERNAL,
	CVMX_ERROR_GROUP_L2C,
	CVMX_ERROR_GROUP_ETHERNET,
	CVMX_ERROR_GROUP_MGMT_PORT,
	CVMX_ERROR_GROUP_PCI,
	CVMX_ERROR_GROUP_SRIO,
	CVMX_ERROR_GROUP_USB,
	CVMX_ERROR_GROUP_LMC,
	CVMX_ERROR_GROUP_ILK,
	CVMX_ERROR_GROUP_DFM,
	CVMX_ERROR_GROUP_ILA,
} cvmx_error_group_t;

/**
 * Flags representing special handling for some error registers.
 * These flags are passed to cvmx_error_initialize() to control
 * the handling of bits where the same flags were passed to the
 * added cvmx_error_info_t.
 */
typedef enum {
	CVMX_ERROR_TYPE_NONE = 0,
	CVMX_ERROR_TYPE_SBE = 1 << 0,
	CVMX_ERROR_TYPE_DBE = 1 << 1,
} cvmx_error_type_t;

/**
 * When registering for interest in an error status register, the
 * type of the register needs to be known by cvmx-error. Most
 * registers are either IO64 or IO32, but some blocks contain
 * registers that can't be directly accessed. A good example of
 * would be PCIe extended error state stored in config space.
 */
typedef enum {
	__CVMX_ERROR_REGISTER_NONE,
	CVMX_ERROR_REGISTER_IO64,
	CVMX_ERROR_REGISTER_IO32,
	CVMX_ERROR_REGISTER_PCICONFIG,
	CVMX_ERROR_REGISTER_SRIOMAINT,
} cvmx_error_register_t;

struct cvmx_error_info;
/**
 * Error handling functions must have the following prototype.
 */
typedef int (*cvmx_error_func_t)(const struct cvmx_error_info *info);

/**
 * This structure is passed to all error handling functions.
 */
typedef struct cvmx_error_info {
	cvmx_error_register_t reg_type;
	u64 status_addr;
	u64 status_mask;
	u64 enable_addr;
	u64 enable_mask;
	cvmx_error_type_t flags;
	cvmx_error_group_t group;
	int group_index;
	cvmx_error_func_t func;
	u64 user_info;
	struct {
		cvmx_error_register_t reg_type;
		u64 status_addr;
		u64 status_mask;
	} parent;
} cvmx_error_info_t;

/**
 * Initialize the error status system. This should be called once
 * before any other functions are called. This function adds default
 * handlers for most all error events but does not enable them. Later
 * calls to cvmx_error_enable() are needed.
 *
 * @param flags  Optional flags.
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_initialize(void);

/**
 * Poll the error status registers and call the appropriate error
 * handlers. This should be called in the RSL interrupt handler
 * for your application or operating system.
 *
 * Return: Number of error handlers called. Zero means this call
 *         found no errors and was spurious.
 */
int cvmx_error_poll(void);

/**
 * Register to be called when an error status bit is set. Most users
 * will not need to call this function as cvmx_error_initialize()
 * registers default handlers for most error conditions. This function
 * is normally used to add more handlers without changing the existing
 * handlers.
 *
 * @param new_info Information about the handler for a error register. The
 *                 structure passed is copied and can be destroyed after the
 *                 call. All members of the structure must be populated, even the
 *                 parent information.
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_add(const cvmx_error_info_t *new_info);

/**
 * Remove all handlers for a status register and mask. Normally
 * this function should not be called. Instead a new handler should be
 * installed to replace the existing handler. In the even that all
 * reporting of a error bit should be removed, then use this
 * function.
 *
 * @param reg_type Type of the status register to remove
 * @param status_addr
 *                 Status register to remove.
 * @param status_mask
 *                 All handlers for this status register with this mask will be
 *                 removed.
 * @param old_info If not NULL, this is filled with information about the handler
 *                 that was removed.
 *
 * Return: Zero on success, negative on failure (not found).
 */
int cvmx_error_remove(cvmx_error_register_t reg_type, u64 status_addr, u64 status_mask,
		      cvmx_error_info_t *old_info);

/**
 * Change the function and user_info for an existing error status
 * register. This function should be used to replace the default
 * handler with an application specific version as needed.
 *
 * @param reg_type Type of the status register to change
 * @param status_addr
 *                 Status register to change.
 * @param status_mask
 *                 All handlers for this status register with this mask will be
 *                 changed.
 * @param new_func New function to use to handle the error status
 * @param new_user_info
 *                 New user info parameter for the function
 * @param old_func If not NULL, the old function is returned. Useful for restoring
 *                 the old handler.
 * @param old_user_info
 *                 If not NULL, the old user info parameter.
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_error_change_handler(cvmx_error_register_t reg_type, u64 status_addr, u64 status_mask,
			      cvmx_error_func_t new_func, u64 new_user_info,
			      cvmx_error_func_t *old_func, u64 *old_user_info);

/**
 * Enable all error registers for a logical group. This should be
 * called whenever a logical group is brought online.
 *
 * @param group  Logical group to enable
 * @param group_index
 *               Index for the group as defined in the cvmx_error_group_t
 *               comments.
 *
 * Return: Zero on success, negative on failure.
 */
/*
 * Rather than conditionalize the calls throughout the executive to not enable
 * interrupts in Uboot, simply make the enable function do nothing
 */
static inline int cvmx_error_enable_group(cvmx_error_group_t group, int group_index)
{
	return 0;
}

/**
 * Disable all error registers for a logical group. This should be
 * called whenever a logical group is brought offline. Many blocks
 * will report spurious errors when offline unless this function
 * is called.
 *
 * @param group  Logical group to disable
 * @param group_index
 *               Index for the group as defined in the cvmx_error_group_t
 *               comments.
 *
 * Return: Zero on success, negative on failure.
 */
/*
 * Rather than conditionalize the calls throughout the executive to not disable
 * interrupts in Uboot, simply make the enable function do nothing
 */
static inline int cvmx_error_disable_group(cvmx_error_group_t group, int group_index)
{
	return 0;
}

/**
 * Enable all handlers for a specific status register mask.
 *
 * @param reg_type Type of the status register
 * @param status_addr
 *                 Status register address
 * @param status_mask
 *                 All handlers for this status register with this mask will be
 *                 enabled.
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_enable(cvmx_error_register_t reg_type, u64 status_addr, u64 status_mask);

/**
 * Disable all handlers for a specific status register and mask.
 *
 * @param reg_type Type of the status register
 * @param status_addr
 *                 Status register address
 * @param status_mask
 *                 All handlers for this status register with this mask will be
 *                 disabled.
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_disable(cvmx_error_register_t reg_type, u64 status_addr, u64 status_mask);

/**
 * @INTERNAL
 * Function for processing non leaf error status registers. This function
 * calls all handlers for this passed register and all children linked
 * to it.
 *
 * @param info   Error register to check
 *
 * Return: Number of error status bits found or zero if no bits were set.
 */
int __cvmx_error_decode(const cvmx_error_info_t *info);

/**
 * @INTERNAL
 * This error bit handler simply prints a message and clears the status bit
 *
 * @param info   Error register to check
 *
 * @return
 */
int __cvmx_error_display(const cvmx_error_info_t *info);

/**
 * Find the handler for a specific status register and mask
 *
 * @param status_addr
 *                Status register address
 *
 * Return:  Return the handler on success or null on failure.
 */
cvmx_error_info_t *cvmx_error_get_index(u64 status_addr);

void __cvmx_install_gmx_error_handler_for_xaui(void);

/**
 * 78xx related
 */
/**
 * Compare two INTSN values.
 *
 * @param key INTSN value to search for
 * @param data current entry from the searched array
 *
 * Return: Negative, 0 or positive when respectively key is less than,
 *		equal or greater than data.
 */
int cvmx_error_intsn_cmp(const void *key, const void *data);

/**
 * @INTERNAL
 *
 * @param intsn   Interrupt source number to display
 *
 * @param node Node number
 *
 * Return: Zero on success, -1 on error
 */
int cvmx_error_intsn_display_v3(int node, u32 intsn);

/**
 * Initialize the error status system for cn78xx. This should be called once
 * before any other functions are called. This function enables the interrupts
 * described in the array.
 *
 * @param node Node number
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_initialize_cn78xx(int node);

/**
 * Enable interrupt for a specific INTSN.
 *
 * @param node Node number
 * @param intsn Interrupt source number
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_intsn_enable_v3(int node, u32 intsn);

/**
 * Disable interrupt for a specific INTSN.
 *
 * @param node Node number
 * @param intsn Interrupt source number
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_intsn_disable_v3(int node, u32 intsn);

/**
 * Clear interrupt for a specific INTSN.
 *
 * @param intsn Interrupt source number
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_intsn_clear_v3(int node, u32 intsn);

/**
 * Enable interrupts for a specific CSR(all the bits/intsn in the csr).
 *
 * @param node Node number
 * @param csr_address CSR address
 *
 * Return: Zero on success, negative on failure.
 */
int cvmx_error_csr_enable_v3(int node, u64 csr_address);

/**
 * Disable interrupts for a specific CSR (all the bits/intsn in the csr).
 *
 * @param node Node number
 * @param csr_address CSR address
 *
 * Return: Zero
 */
int cvmx_error_csr_disable_v3(int node, u64 csr_address);

/**
 * Enable all error registers for a logical group. This should be
 * called whenever a logical group is brought online.
 *
 * @param group  Logical group to enable
 * @param xipd_port  The IPD port value
 *
 * Return: Zero.
 */
int cvmx_error_enable_group_v3(cvmx_error_group_t group, int xipd_port);

/**
 * Disable all error registers for a logical group.
 *
 * @param group  Logical group to enable
 * @param xipd_port  The IPD port value
 *
 * Return: Zero.
 */
int cvmx_error_disable_group_v3(cvmx_error_group_t group, int xipd_port);

/**
 * Enable all error registers for a specific category in a logical group.
 * This should be called whenever a logical group is brought online.
 *
 * @param group  Logical group to enable
 * @param type   Category in a logical group to enable
 * @param xipd_port  The IPD port value
 *
 * Return: Zero.
 */
int cvmx_error_enable_group_type_v3(cvmx_error_group_t group, cvmx_error_type_t type,
				    int xipd_port);

/**
 * Disable all error registers for a specific category in a logical group.
 * This should be called whenever a logical group is brought online.
 *
 * @param group  Logical group to disable
 * @param type   Category in a logical group to disable
 * @param xipd_port  The IPD port value
 *
 * Return: Zero.
 */
int cvmx_error_disable_group_type_v3(cvmx_error_group_t group, cvmx_error_type_t type,
				     int xipd_port);

/**
 * Clear all error registers for a logical group.
 *
 * @param group  Logical group to disable
 * @param xipd_port  The IPD port value
 *
 * Return: Zero.
 */
int cvmx_error_clear_group_v3(cvmx_error_group_t group, int xipd_port);

/**
 * Enable all error registers for a particular category.
 *
 * @param node  CCPI node
 * @param type  category to enable
 *
 *Return: Zero.
 */
int cvmx_error_enable_type_v3(int node, cvmx_error_type_t type);

/**
 * Disable all error registers for a particular category.
 *
 * @param node  CCPI node
 * @param type  category to disable
 *
 *Return: Zero.
 */
int cvmx_error_disable_type_v3(int node, cvmx_error_type_t type);

void cvmx_octeon_hang(void) __attribute__((__noreturn__));

/**
 * @INTERNAL
 *
 * Process L2C single and multi-bit ECC errors
 *
 */
int __cvmx_cn7xxx_l2c_l2d_ecc_error_display(int node, int intsn);

/**
 * Handle L2 cache TAG ECC errors and noway errors
 *
 * @param	CCPI node
 * @param	intsn	intsn from error array.
 * @param	remote	true for remote node (cn78xx only)
 *
 * Return:	1 if handled, 0 if not handled
 */
int __cvmx_cn7xxx_l2c_tag_error_display(int node, int intsn, bool remote);

#endif
