/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2014 Rockchip Electronics
 */

#ifndef __RC4_H
#define __RC4_H

/**
 * rc4_encode() - encode a buf with the RC4 cipher
 *
 * @buf:	Buffer to encode (it is overwrite in the process
 * @len:	Length of buffer in bytes
 * @key:	16-byte key to use
 */
void rc4_encode(unsigned char *buf, unsigned int len, unsigned char key[16]);

#endif
