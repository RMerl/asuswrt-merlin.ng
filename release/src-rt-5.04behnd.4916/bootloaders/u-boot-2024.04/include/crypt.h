/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 2020 Steffen Jaeckel <jaeckel-floss@eyet-services.de> */

/**
 * Compare should with the processed passphrase.
 *
 * @should      The crypt-style string to compare against
 * @passphrase  The plaintext passphrase
 * @equal       Pointer to an int where the result is stored
 *                 '0' = unequal
 *                 '1' = equal
 * Return: 0 on success, error code of errno else
 */
int crypt_compare(const char *should, const char *passphrase, int *equal);
