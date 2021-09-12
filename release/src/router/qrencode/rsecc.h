/*
 * qrencode - QR Code encoder
 *
 * Reed solomon error correction code encoder specialized for QR code.
 * This code is rewritten by Kentaro Fukuchi, referring to the FEC library
 * developed by Phil Karn (KA9Q).
 *
 * Copyright (C) 2002, 2003, 2004, 2006 Phil Karn, KA9Q
 * Copyright (C) 2014-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RSECC_H
#define RSECC_H

extern int RSECC_encode(size_t data_length, size_t ecc_length, const unsigned char *data, unsigned char *ecc);

#endif /* RSECC_H */
