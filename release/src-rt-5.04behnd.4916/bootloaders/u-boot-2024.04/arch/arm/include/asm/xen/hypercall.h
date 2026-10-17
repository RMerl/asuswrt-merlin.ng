/* SPDX-License-Identifier: GPL-2.0
 *
 * hypercall.h
 *
 * Linux-specific hypervisor handling.
 *
 * Stefano Stabellini <stefano.stabellini@eu.citrix.com>, Citrix, 2012
 */

#ifndef _ASM_ARM_XEN_HYPERCALL_H
#define _ASM_ARM_XEN_HYPERCALL_H

#include <xen/interface/xen.h>

int HYPERVISOR_xen_version(int cmd, void *arg);
int HYPERVISOR_console_io(int cmd, int count, char *str);
int HYPERVISOR_grant_table_op(unsigned int cmd, void *uop, unsigned int count);
int HYPERVISOR_sched_op(int cmd, void *arg);
int HYPERVISOR_event_channel_op(int cmd, void *arg);
unsigned long HYPERVISOR_hvm_op(int op, void *arg);
int HYPERVISOR_memory_op(unsigned int cmd, void *arg);
#endif /* _ASM_ARM_XEN_HYPERCALL_H */
