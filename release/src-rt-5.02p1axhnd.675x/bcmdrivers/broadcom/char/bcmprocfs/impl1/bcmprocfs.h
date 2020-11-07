/*
 ** <:copyright-broadcom
 **
 **  Copyright (c) 2002 Broadcom Corporation
 **  All Rights Reserved
 **  No portions of this material may be reproduced in any form without the
 **  written permission of:
 **  Broadcom Corporation
 **  16215 Alton Parkway
 **  Irvine, California 92619
 **  All information contained in this document is Broadcom Corporation
 **  company private, proprietary, and trade secret.
 **
 **  :>
 ***/

#define  CREATE_SYMLINK    0x200
#define  CREATE_FILE       0x201
#define  RENAME_TELNETD    0x202
#define  RENAME_HTTPD      0x203
#define  RENAME_SSHD       0x204
#define  RENAME_SNMP       0x205
#define  RENAME_TR69C      0x206
#define  RENAME_CONSOLED   0x207
#define  RENAME_TR64       0x208

/* ln -s src dst */
struct symlink {
	char src[32];
	char dst[32];
};
