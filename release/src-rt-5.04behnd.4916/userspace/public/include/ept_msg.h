/***********************************************************************
 *
 * Copyright (c) 2024 Broadcom
 * All Rights Reserved
 *
 * <:label-BRCM:2024:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 *
 ************************************************************************/
#ifndef __EPT_MSG_H__
#define __EPT_MSG_H__

/*!\file ept_msg.h
 * \brief header file that defines messages used to communicate
 * between eptcli/consoled and eptapp.
 */
#define MAX_BUFLEN_128 128

typedef enum
{
   VOICE_DIAG_CFG_SHOW           = 0,
   VOICE_DIAG_EPTCMD             = 1,
   VOICE_DIAG_STUNLKUP           = 2,
   VOICE_DIAG_STATS_SHOW         = 3,
   VOICE_DIAG_PROFILE            = 4,
   VOICE_DIAG_EPTAPP_HELP        = 5,
   VOICE_DIAG_EPTAPP_SHOW        = 6,
   VOICE_DIAG_EPTAPP_CREATECNX   = 7,
   VOICE_DIAG_EPTAPP_DELETECNX   = 8,
   VOICE_DIAG_EPTAPP_MODIFYCNX   = 9,
   VOICE_DIAG_EPTAPP_EPTSIG      = 10,
   VOICE_DIAG_EPTAPP_SET         = 11,
   VOICE_DIAG_EPTPROV            = 12,
   VOICE_DIAG_EPTAPP_DECTTEST    = 13,
   VOICE_DIAG_DECT_MEM_SET       = 14,
   VOICE_DIAG_DECT_MEM_GET       = 15,
   VOICE_DIAG_DECT_MODE_SET      = 16,
   VOICE_DIAG_DECT_MODE_GET      = 17,
   VOICE_DIAG_EPTPROBE           = 18,
   VOICE_DIAG_EPTAPP_VAS         = 19,
   VOICE_DIAG_EPTAPP_VRS         = 20,
   VOICE_DIAG_LINE_TEST_CMD      = 21,
   VOICE_DIAG_EPTAPP_LOG_LEVEL   = 22,
   VOICE_DIAG_EPTAPP_CAPS        = 23,
} VoiceDiagType;

/** Data body for Voice diagonistic message */
typedef struct
{
  VoiceDiagType type;
  UINT16 dataLen;
  char cmdLine[MAX_BUFLEN_128];
} VoiceDiagMsgBody;

#endif // __EPT_MSG_H__
