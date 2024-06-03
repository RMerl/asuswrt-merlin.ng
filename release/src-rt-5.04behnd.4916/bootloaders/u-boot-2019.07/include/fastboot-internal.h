/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _FASTBOOT_INTERNAL_H_
#define _FASTBOOT_INTERNAL_H_

/**
 * fastboot_buf_addr - base address of the fastboot download buffer
 */
extern void *fastboot_buf_addr;

/**
 * fastboot_buf_size - size of the fastboot download buffer
 */
extern u32 fastboot_buf_size;

/**
 * fastboot_progress_callback - callback executed during long operations
 */
extern void (*fastboot_progress_callback)(const char *msg);

/**
 * fastboot_getvar() - Writes variable indicated by cmd_parameter to response.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Look up cmd_parameter first as an environment variable of the form
 * fastboot.<cmd_parameter>, if that exists return use its value to set
 * response.
 *
 * Otherwise lookup the name of variable and execute the appropriate
 * function to return the requested value.
 */
void fastboot_getvar(char *cmd_parameter, char *response);

#endif
