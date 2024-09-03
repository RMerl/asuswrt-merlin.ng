/*
*    Copyright (c) 2020 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2020:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#ifndef TRACKER_H
#define TRACKER_H

/* The tracker allows to track arbitrary pointers across the kernel lifetime.
 *
 * Expected usage is to sprinkle track_printf's in the interesting points of
 * ptr usage.
 *
 * The resulting strings are accessible in the shell by catting /proc/tracker.
 *
 * When investigating specific badness tracker_print and tracker_find are useful
 * to dump or work with this information from inside of the kernel.
 */

/* Initialize tracker. Should be called once */
void track_init(void);

/* Dumps information regarding ptr to dmesg */
void tracker_print(void *ptr);

/* Looks for pointer ptr_any. If found, tracker_find calls cb callback for each recorded state.
 * In addition to state the callback gets arbitrary ctx passed by the caller.
 *
 * This function is useful to analyse the pointer states during runtime. Here is example
 * diff which helped us track freeing non-allocated pointer in linux kernel:
 *
 * +static void seen_alloc(void *ctx, const char *st) {
 * +	int *alloced = ctx;
 * +
 * +	if (strncmp(st, "header_alloc", strlen("header_alloc")) == 0)
 * +		*alloced = 1;
 * +}
 * +
 *  static void drop_sysctl_table(struct ctl_table_header *header)
 *  {
 *  	struct ctl_dir *parent = header->parent;
 * +	int alloced = 0;
 *  
 *  	if (--header->nreg)
 *  		return;
 *  
 * +	tracker_find(header, seen_alloc, &alloced);
 * +	if (!alloced) {
 * +		printk("boriss: non alloced header %px\n", header);
 * +		WARN_ON(1);
 * +	}
 * 
 */
void tracker_find(void *ptr_any, void (*cb)(void *ctx, const char *st), void *ctx);

/* printf into new state string attached to the pointer ptr. The resulting string will
 * be dumped by tracker_print and passed to the callback in tracker_find.
 */
void track_printf(void *ptr, const char *fmt, ...);

#endif /* TRACKER_H */
