/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) 2020 EPAM Systems Inc.
 */

#ifndef _PVBLOCK_H
#define _PVBLOCK_H

/**
 * pvblock_init() - Initialize para-virtual block device class driver
 *
 * Bind PV block to UCLASS_ROOT device and probe all UCLASS_PVBLOCK
 * virtual block devices.
 */
void pvblock_init(void);

#endif /* _PVBLOCK_H */
