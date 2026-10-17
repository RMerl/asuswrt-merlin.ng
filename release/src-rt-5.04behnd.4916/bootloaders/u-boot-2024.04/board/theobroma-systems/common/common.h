/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2023 Theobroma Systems Design und Consulting GmbH
 */

/*
 * setup_boottargets() - Swap mmc0 and mmc1 in boot_targets depending on U-Boot
 * proper load medium.
 *
 * If bootsource is uSD-card we can assume that we want to use the
 * SD-Card instead of the eMMC as first boot_target for distroboot.
 * We only want to swap the defaults and not any custom environment a
 * user has set. We exit early if a changed boot_targets environment
 * is detected.
 *
 * Return:
 * 0 if OK, -1 otherwise
 */
int setup_boottargets(void);
