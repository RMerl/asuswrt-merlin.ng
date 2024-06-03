/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 */

#ifndef _BCMSTB_SDHCI_H
#define _BCMSTB_SDHCI_H

#include <linux/types.h>

int bcmstb_sdhci_init(phys_addr_t regbase);

#endif /* _BCMSTB_SDHCI_H */
