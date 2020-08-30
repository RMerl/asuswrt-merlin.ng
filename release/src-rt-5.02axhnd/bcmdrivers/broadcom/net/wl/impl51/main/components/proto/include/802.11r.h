/*
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * Fundamental constants relating to 802.11r
 *
 * $Id$
 */

#ifndef _802_11r_H_
#define _802_11r_H_

#define FBT_R0KH_ID_LEN 49 /* includes null termination */
#define FBT_REASSOC_TIME_DEF	1000

#define DOT11_FBT_SUBELEM_ID_R1KH_ID		1
#define DOT11_FBT_SUBELEM_ID_GTK		2
#define DOT11_FBT_SUBELEM_ID_R0KH_ID		3
#define DOT11_FBT_SUBELEM_ID_IGTK		4

/*
* FBT Subelement id lenths
*/

#define DOT11_FBT_SUBELEM_R1KH_LEN		6
/* GTK_FIXED_LEN = Key_Info (2Bytes) + Key_Length (1Byte) + RSC (8Bytes) */
#define DOT11_FBT_SUBELEM_GTK_FIXED_LEN		11
/* GTK_MIN_LEN = GTK_FIXED_LEN + key (min 16 Bytes) + key_wrap (8Bytes) */
#define DOT11_FBT_SUBELEM_GTK_MIN_LEN		(DOT11_FBT_SUBELEM_GTK_FIXED_LEN + 24)
/* GTK_MAX_LEN = GTK_FIXED_LEN + key (max 32 Bytes) + key_wrap (8Bytes) */
#define DOT11_FBT_SUBELEM_GTK_MAX_LEN		(DOT11_FBT_SUBELEM_GTK_FIXED_LEN + 40)
#define DOT11_FBT_SUBELEM_R0KH_MIN_LEN		1
#define DOT11_FBT_SUBELEM_R0KH_MAX_LEN		48
/* IGTK_LEN = KeyID (2Bytes) + IPN (6Bytes) + Key_Length (1Byte) +
*		Wrapped_Key (key (16Bytes) + key_wrap (8Bytes))
*/
#define DOT11_FBT_SUBELEM_IGTK_LEN		33

#endif	/* #ifndef _802_11r_H_ */
