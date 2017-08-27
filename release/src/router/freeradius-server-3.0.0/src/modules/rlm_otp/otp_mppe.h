/*
 * $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2001,2002  Google, Inc.
 * Copyright 2005,2006 TRI-D Systems, Inc.
 */

#ifndef OTP_MPPE_H
#define OTP_MPPE_H

RCSIDH(otp_mppe_h, "$Id$")

/* MPPE encryption policy */
#define MPPE_ENC_POL_ENCRYPTION_FORBIDDEN "0x00000000"
#define MPPE_ENC_POL_ENCRYPTION_ALLOWED   "0x00000001"
#define MPPE_ENC_POL_ENCRYPTION_REQUIRED  "0x00000002"

/* MPPE encryption types */
#define MPPE_ENC_TYPES_RC4_40     "0x00000002"
#define MPPE_ENC_TYPES_RC4_128    "0x00000004"
#define MPPE_ENC_TYPES_RC4_40_128 "0x00000006"

/* Translate the above into something easily usable. */
static char const *otp_mppe_policy[3] = {
  MPPE_ENC_POL_ENCRYPTION_FORBIDDEN,
  MPPE_ENC_POL_ENCRYPTION_ALLOWED,
  MPPE_ENC_POL_ENCRYPTION_REQUIRED };

static char const *otp_mppe_types[3] = {
  MPPE_ENC_TYPES_RC4_40,
  MPPE_ENC_TYPES_RC4_128,
  MPPE_ENC_TYPES_RC4_40_128 };

#endif /* OTP_MPPE_H */
