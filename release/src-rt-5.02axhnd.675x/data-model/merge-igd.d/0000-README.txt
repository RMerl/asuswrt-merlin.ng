#
# <:copyright-BRCM:2019:proprietary:standard
# 
#    Copyright (c) 2019 Broadcom 
#    All Rights Reserved
# 
#  This program is the proprietary software of Broadcom and/or its
#  licensors, and may only be used, duplicated, modified or distributed pursuant
#  to the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied), right
#  to use, or waiver of any kind with respect to the Software, and Broadcom
#  expressly reserves all rights in and to the Software and all intellectual
#  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
#  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
#  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
# 
#  Except as expressly set forth in the Authorized License,
# 
#  1. This program, including its structure, sequence and organization,
#     constitutes the valuable trade secrets of Broadcom, and you shall use
#     all reasonable efforts to protect the confidentiality thereof, and to
#     use this information only in connection with your use of Broadcom
#     integrated circuit products.
# 
#  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
#     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
#     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
#     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
#     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
#     PERFORMANCE OF THE SOFTWARE.
# 
#  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
#     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
#     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
#     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
#     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
#     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
#     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
#     LIMITED REMEDY.
# :>
#
# Format of files in merge-igd.txt:
# All blank lines and lines beginning with # are ignored.
# All other lines must contain one of the commands described below.
# Each file can contain one or more commands, although for the sake
# of modularity, usually we only put one command per file.


# The 6 commands supported are:
#
# base <data-model-file>
# addObjBelowObj <data-model-file> <target-object>
# addObjBelowObj <data-model-file> <target-object>
# addFirstChildObjToObj <data-model-file> <target-object>
# addLastChildObjToObj <data-model-file> <target-object>
# deleteObj <target-object>


# The name of each file must begin with 4 digits, and end with .txt.
# The number ranges are defined as follows (note this applies to merge-igd.d only):
# 1000      : must be the first file and contain the "base" command.  This is
#             the only file that can contain the base command.  This file
#             creates the complete TR98 data model.
# 1001-1999 : Broadcom defined objects
# 2000-2999 : Other Broadband Forum data models
# 3000-3999 : PON data models
# 4000-4999 : Hybrid TR181 data model objects
# 5000-8000 : Reserved
# 8000-9999 : For customer use

