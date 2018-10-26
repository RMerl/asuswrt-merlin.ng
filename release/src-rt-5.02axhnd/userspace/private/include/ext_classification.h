/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/


#ifndef _EXT_CASSIFICATION_H_
#define _EXT_CASSIFICATION_H_

extern void extClassification_init(void);
extern int isExtClassificationFilter(void);
extern void extClassification_filterOnSkbMarkPort(uint8_t flowid, uint8_t exclude);
extern void extClassification_filterOnDscp(uint8_t v, uint8_t exclude);
extern void extClassification_filterOnL2Type(uint16_t v, uint8_t exclude);
extern void extClassification_filterOnL2Dst(uint8_t* l2dst, uint8_t len, uint8_t msb, uint8_t lsb, uint8_t exclude);
extern void extClassification_filterOnL2Src(uint8_t* l2src, uint8_t len, uint8_t msb, uint8_t lsb, uint8_t exclude);
extern void extClassification_filterOnIpProto(uint8_t v, uint8_t exclude);
extern void extClassification_filterOnIpv4Sa(uint8_t* v, uint8_t len, uint8_t msb, uint8_t lsb, uint8_t exclude);
extern void extClassification_filterOnIpv6Sa(uint8_t* v, uint8_t len, uint8_t msb, uint8_t lsb, uint8_t exclude);
extern void extClassification_filterOnIpv4Da(uint8_t* v, uint8_t len, uint8_t msb, uint8_t lsb, uint8_t exclude);
extern void extClassification_filterOnIpv6Da(uint8_t* v, uint8_t len, uint8_t msb, uint8_t lsb, uint8_t exclude);
extern void extClassification_filterOnTcpUdpSrcPort(uint16_t port, uint8_t exclude);
extern void extClassification_filterOnTcpUdpDestPort(uint16_t port, uint8_t exclude);
extern void extClassification_parseInfo(uint8_t port, uint8_t oamPriority, char *portName);
extern void extClassification_filterOnSVlan(void);
extern void extClassification_filterOnCVlan(void);
extern int extClassification_addRule(uint8_t port, uint32_t nbrOfTags, uint16_t tagRuleId);
extern int extClassification_delRule(uint8_t port, uint32_t nbrOfTags, uint16_t tagRuleId, uint8_t updateEbtable);

#endif

// _EXT_CASSIFICATION_H_

