/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2017 The Android Open Source Project
 */

#ifndef __ANDROID_AB_H
#define __ANDROID_AB_H

struct blk_desc;
struct disk_partition;

/* Android standard boot slot names are 'a', 'b', 'c', ... */
#define BOOT_SLOT_NAME(slot_num) ('a' + (slot_num))

/* Number of slots */
#define NUM_SLOTS 2

/**
 * Select the slot where to boot from.
 *
 * On Android devices with more than one boot slot (multiple copies of the
 * kernel and system images) selects which slot should be used to boot from and
 * registers the boot attempt. This is used in by the new A/B update model where
 * one slot is updated in the background while running from the other slot. If
 * the selected slot did not successfully boot in the past, a boot attempt is
 * registered before returning from this function so it isn't selected
 * indefinitely.
 *
 * @param[in] dev_desc Place to store the device description pointer
 * @param[in] part_info Place to store the partition information
 * Return: The slot number (>= 0) on success, or a negative on error
 */
int ab_select_slot(struct blk_desc *dev_desc, struct disk_partition *part_info,
                   bool dec_tries);

#endif /* __ANDROID_AB_H */
