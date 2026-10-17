/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2022 Google LLC
 */

/**
 * mercury_aa1_read_mac() - Read mac address from on-board OTP memory
 *
 * @mac: Returned mac address
 * Return: 0 if successful, -ve on error
 */
int mercury_aa1_read_mac(u8 *mac);
