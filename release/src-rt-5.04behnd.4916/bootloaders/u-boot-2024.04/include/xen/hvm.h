/* SPDX-License-Identifier: GPL-2.0
 *
 * Simple wrappers around HVM functions
 *
 * Copyright (c) 2002-2003, K A Fraser
 * Copyright (c) 2005, Grzegorz Milos, gm281@cam.ac.uk,Intel Research Cambridge
 * Copyright (c) 2020, EPAM Systems Inc.
 */
#ifndef XEN_HVM_H__
#define XEN_HVM_H__

#include <asm/xen/hypercall.h>
#include <xen/interface/hvm/params.h>
#include <xen/interface/xen.h>

extern struct shared_info *HYPERVISOR_shared_info;

int hvm_get_parameter(int idx, uint64_t *value);
int hvm_get_parameter_maintain_dcache(int idx, uint64_t *value);

struct shared_info *map_shared_info(void *p);
void do_hypervisor_callback(struct pt_regs *regs);
void mask_evtchn(uint32_t port);
void unmask_evtchn(uint32_t port);
void clear_evtchn(uint32_t port);

#endif /* XEN_HVM_H__ */
