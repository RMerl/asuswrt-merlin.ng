/* SPDX-License-Identifier: GPL-2.0
 *
 * (C) 2020, EPAM Systems Inc.
 */
#ifndef __XEN_H__
#define __XEN_H__

/**
 * xen_init() - Xen initialization
 *
 * Map Xen memory pages, initialize event handler and xenbus,
 * setup the grant table.
 */
int xen_init(void);

/**
 * xen_fini() - Board cleanup before Linux kernel start
 *
 * Unmap Xen memory pages the specified guest's pseudophysical
 * address space and unbind all event channels.
 */
void xen_fini(void);

#endif /* __XEN_H__ */
