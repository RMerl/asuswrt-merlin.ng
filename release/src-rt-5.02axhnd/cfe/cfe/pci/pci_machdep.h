/*
 * Copyright (c) 2001,2002 SiByte, Inc.  All rights reserved.
 */

#ifndef _PCI_MACHDEP_H_
#define _PCI_MACHDEP_H_

/*
 * Machine-specific definitions for PCI autoconfiguration.
 *
 * See the comments in pci_machdep.c for more explanation.
 */

#include "lib_types.h"

/*
 * Address types, as integers.
 */

typedef uint32_t pci_addr_t;
typedef uint64_t phys_addr_t;   /* ZBbus physical addresses. */

/*
 * Configuration tag; created from a {bus,device,function} triplet by
 * pci_make_tag(), and passed to pci_conf_read() and pci_conf_write().
 */
typedef uint32_t pcitag_t;

/*
 * Type of a value read from or written to a configuration register.
 * Always 32 bits.
 */
typedef uint32_t pcireg_t;

#endif /* _PCI_MACHDEP_H_ */
