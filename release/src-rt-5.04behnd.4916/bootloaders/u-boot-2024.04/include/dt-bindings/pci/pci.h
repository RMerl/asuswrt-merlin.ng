/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * This header provides common constants for PCI bindings.
 */

#ifndef _DT_BINDINGS_PCI_PCI_H
#define _DT_BINDINGS_PCI_PCI_H

/* Encode a vendor and device ID into a single cell */
#define PCI_VENDEV(v, d)	(((v) << 16) | (d))

#endif /* _DT_BINDINGS_PCI_PCI_H */
