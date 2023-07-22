/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:> 
*/
#ifndef TEE_SOTP_SER_H
#define TEE_SOTP_SER_H

#define SOTP_SERVICE_UUID \
		{ 0xd3df7b18, 0x4c02, 0x4051,  \
		{ 0xb8, 0x65, 0xe3, 0x19, 0x34, 0x35, 0x3e, 0xfd } }

typedef enum {
	CMD_SOTP_SERVICE_INIT = 0,
	CMD_SOTP_SERVICE_READ,
	CMD_SOTP_SERVICE_WRITE,
	CMD_SOTP_SERVICE_LOCK,
} cmd_sotp_service;

#endif /* TEE_SOTP_SER_H */
