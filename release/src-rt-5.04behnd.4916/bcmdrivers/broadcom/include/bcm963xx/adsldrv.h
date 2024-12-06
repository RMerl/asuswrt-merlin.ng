/*
<:copyright-BRCM:2004:DUAL/GPL:standard

   Copyright (c) 2004 Broadcom 
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

/***************************************************************************
 * File Name  : AdslDrv.h
 *
 * Description: This file contains the definitions and structures for the
 *              Linux IOCTL interface that used between the user mode ADSL
 *              API library and the kernel ADSL API driver.
 *
 * Updates    : 11/02/2001  lkaplan.  Created.
 ***************************************************************************/

#if !defined(_ADSLDRV_H_)
#define _ADSLDRV_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Incldes. */
#include <bcmadsl.h>

/* Defines. */
#define DSL_IFNAME               "dsl0"
#if defined(LINUX_FW_EXTRAVERSION) && (LINUX_FW_EXTRAVERSION >= 50207)
#define ADSLDRV_MAJOR            333 /* arbitrary unused value, see targets/fs.src/etc/make_static_devnodes.sh */
#elif defined(LINUX_FW_EXTRAVERSION) && (LINUX_FW_EXTRAVERSION >= 50203)
#define ADSLDRV_MAJOR            3033 /* arbitrary unused value, see targets/fs.src/etc/make_static_devnodes.sh */
#else
#define ADSLDRV_MAJOR            208 /* arbitrary unused value, see targets/fs.src/etc/make_static_devnodes.sh */
#endif
#define ADSLIOCTL_CHECK \
    _IOR(ADSLDRV_MAJOR, 0, ADSLDRV_STATUS_ONLY)
#define ADSLIOCTL_INITIALIZE \
    _IOWR(ADSLDRV_MAJOR, 1, ADSLDRV_INITIALIZE)
#define ADSLIOCTL_UNINITIALIZE \
    _IOR(ADSLDRV_MAJOR, 2, ADSLDRV_STATUS_ONLY)
#define ADSLIOCTL_CONNECTION_START \
    _IOWR(ADSLDRV_MAJOR, 3, ADSLDRV_STATUS_ONLY)
#define ADSLIOCTL_CONNECTION_STOP \
    _IOR(ADSLDRV_MAJOR, 4, ADSLDRV_STATUS_ONLY)
#define ADSLIOCTL_GET_PHY_ADDR \
    _IOR(ADSLDRV_MAJOR, 5, ADSLDRV_PHY_ADDR)
#define ADSLIOCTL_SET_PHY_ADDR \
    _IOWR(ADSLDRV_MAJOR, 6, ADSLDRV_PHY_ADDR)
#define ADSLIOCTL_MAP_ATM_PORT_IDS \
    _IOWR(ADSLDRV_MAJOR, 7, ADSLDRV_MAP_ATM_PORT)
#define ADSLIOCTL_GET_CONNECTION_INFO \
    _IOR(ADSLDRV_MAJOR, 8, ADSLDRV_CONNECTION_INFO)
#define ADSLIOCTL_DIAG_COMMAND \
    _IOR(ADSLDRV_MAJOR, 9, ADSLDRV_DIAG)
#define ADSLIOCTL_GET_OBJ_VALUE \
    _IOR(ADSLDRV_MAJOR, 10, ADSLDRV_GET_OBJ)
#define ADSLIOCTL_START_BERT \
    _IOR(ADSLDRV_MAJOR, 11, ADSLDRV_BERT)
#define ADSLIOCTL_STOP_BERT \
    _IOR(ADSLDRV_MAJOR, 12, ADSLDRV_STATUS_ONLY)
#define ADSLIOCTL_CONFIGURE \
    _IOR(ADSLDRV_MAJOR, 13, ADSLDRV_CONFIGURE)
#define ADSLIOCTL_TEST \
    _IOR(ADSLDRV_MAJOR, 14, ADSLDRV_TEST)
#define ADSLIOCTL_GET_CONSTEL_POINTS \
    _IOR(ADSLDRV_MAJOR, 15, ADSLDRV_GET_CONSTEL_POINTS)
#define ADSLIOCTL_GET_VERSION \
    _IOR(ADSLDRV_MAJOR, 16, ADSLDRV_GET_VERSION)
#define ADSLIOCTL_SET_SDRAM_BASE \
    _IOR(ADSLDRV_MAJOR, 17, ADSLDRV_SET_SDRAM_BASE)
#define ADSLIOCTL_RESET_STAT_COUNTERS \
    _IOR(ADSLDRV_MAJOR, 18, ADSLDRV_STATUS_ONLY)
#define ADSLIOCTL_SET_OEM_PARAM \
    _IOR(ADSLDRV_MAJOR, 19, ADSLDRV_SET_OEM_PARAM)
#define ADSLIOCTL_START_BERT_EX \
    _IOR(ADSLDRV_MAJOR, 20, ADSLDRV_BERT_EX)
#define ADSLIOCTL_STOP_BERT_EX \
    _IOR(ADSLDRV_MAJOR, 21, ADSLDRV_STATUS_ONLY)
#define ADSLIOCTL_SET_OBJ_VALUE \
    _IOR(ADSLDRV_MAJOR, 22, ADSLDRV_GET_OBJ)
#define ADSLIOCTL_DRV_CALLBACK \
    _IOR(ADSLDRV_MAJOR, 23, ADSLDRV_STATUS_ONLY)
#define XDSLIOCTL_OPEN_EOC_IFACE \
    _IOR(ADSLDRV_MAJOR, 24, XDSLDRV_EOC_IFACE)
#define XDSLIOCTL_CLOSE_EOC_IFACE \
    _IOR(ADSLDRV_MAJOR, 25, XDSLDRV_EOC_IFACE)
#define XDSLIOCTL_SEND_HMI_MSG \
    _IOR(ADSLDRV_MAJOR, 26, XDSLDRV_HMI_MSG)
#define MAX_ADSLDRV_IOCTL_COMMANDS   27

/* Typedefs. */
typedef struct
{
    BCMADSL_STATUS bvStatus;
} ADSLDRV_STATUS_ONLY, *PADSLDRV_STATUS_ONLY;

typedef struct
{
    BCM_IOC_PTR(ADSL_FN_NOTIFY_CB, pFnNotifyCb);
#if defined(LINUX_FW_EXTRAVERSION) && (LINUX_FW_EXTRAVERSION >= 50204)
    BCM_IOC_PTR(void *, pParm);
#else
    BCM_IOC_PTR(unsigned long, ulParm);
#endif
    BCM_IOC_PTR(adslCfgProfile *, pAdslCfg);
    BCMADSL_STATUS bvStatus;
} ADSLDRV_INITIALIZE, *PADSLDRV_INITIALIZE;

typedef struct
{
    ADSL_CHANNEL_ADDR ChannelAddr;
    BCMADSL_STATUS bvStatus;
} ADSLDRV_PHY_ADDR, *PADSLDRV_PHY_ADDR;

typedef struct
{
    unsigned short usAtmFastPortId;
    unsigned short usAtmInterleavedPortId;
    BCMADSL_STATUS bvStatus;
} ADSLDRV_MAP_ATM_PORT, *PADSLDRV_MAP_ATM_PORT;

typedef struct
{
    ADSL_CONNECTION_INFO ConnectionInfo;
    BCMADSL_STATUS bvStatus;
} ADSLDRV_CONNECTION_INFO, *PADSLDRV_CONNECTION_INFO;

typedef struct
{
    int diagCmd;
    BCM_IOC_PTR(unsigned long, diagMap);
    int logTime;
    int srvIpAddr;
    int gwIpAddr;
    BCMADSL_STATUS      bvStatus;
} ADSLDRV_DIAG, *PADSLDRV_DIAG;

typedef struct
{
    BCM_IOC_PTR(char *, objId);
    int  objIdLen;
    BCM_IOC_PTR(char *, dataBuf);
    BCM_IOC_PTR(long, dataBufLen);
    BCMADSL_STATUS bvStatus;
} ADSLDRV_GET_OBJ, *PADSLDRV_GET_OBJ;

typedef struct
{
    unsigned int totalBits;
    BCMADSL_STATUS bvStatus;
} ADSLDRV_BERT, *PADSLDRV_BERT;

typedef struct
{
    unsigned int totalSec;
    BCMADSL_STATUS bvStatus;
} ADSLDRV_BERT_EX, *PADSLDRV_BERT_EX;

typedef struct
{
    BCM_IOC_PTR(adslCfgProfile *, pAdslCfg);
    BCMADSL_STATUS bvStatus;
} ADSLDRV_CONFIGURE, *PADSLDRV_CONFIGURE;

typedef struct
{
    unsigned int   testCmd;
    unsigned int   xmtStartTone;
    unsigned int   xmtNumTones;
    unsigned int   rcvStartTone;
    unsigned int   rcvNumTones;
    BCM_IOC_PTR(char *, xmtToneMap);
    BCM_IOC_PTR(char *, rcvToneMap);
    BCMADSL_STATUS      bvStatus;
} ADSLDRV_TEST, *PADSLDRV_TEST;

typedef struct
{
    int toneId;
    BCM_IOC_PTR(ADSL_CONSTELLATION_POINT *, pointBuf);
    int numPoints;
    BCMADSL_STATUS bvStatus;
} ADSLDRV_GET_CONSTEL_POINTS, *PADSLDRV_GET_CONSTEL_POINTS;

typedef struct
{
    BCM_IOC_PTR(adslVersionInfo *, pAdslVer);
    BCMADSL_STATUS  bvStatus;
} ADSLDRV_GET_VERSION, *PADSLDRV_GET_VERSION;

typedef struct
{
    BCM_IOC_PTR(void *, sdramBaseAddr);
    BCMADSL_STATUS bvStatus;
} ADSLDRV_SET_SDRAM_BASE, *PADSLDRV_SET_SDRAM_BASE;


typedef struct
{
    int paramId;
    BCM_IOC_PTR(void *, buf);
    int len;
    BCMADSL_STATUS bvStatus;
} ADSLDRV_SET_OEM_PARAM, *PADSLDRV_SET_OEM_PARAM;

typedef struct
{
    BCM_IOC_PTR(char *, ifName);
    int  ifNameLen;
    int  eocMsgType;
    BCMADSL_STATUS bvStatus;
} XDSLDRV_EOC_IFACE, *PXDSLDRV_EOC_IFACE;

typedef struct
{
    BCM_IOC_PTR(unsigned char *, header);
    BCM_IOC_PTR(unsigned char *, payload);
    BCM_IOC_PTR(unsigned char *, replyMessage);
    unsigned short headerSize;
    unsigned short payloadSize;
    unsigned short replyMaxMessageSize;
    BCMADSL_STATUS bvStatus;
} XDSLDRV_HMI_MSG, *PXDSLDRV_HMI_MSG;

#if defined(__cplusplus)
}
#endif


/* declaration for read, write, poll operations */
#define ADSL_EOC_FRAME_RCVD         (1 << BCM_ADSL_G997_FRAME_RECEIVED)
#define ADSL_EOC_FRAME_SENT         (1 << BCM_ADSL_G997_FRAME_SENT)
#define ADSL_LINK_DROPPED           (1 << BCM_ADSL_LINK_DOWN)
#define ADSL_EOC_ANY_EVENT          (ADSL_EOC_FRAME_RCVD | ADSL_EOC_FRAME_SENT | ADSL_LINK_DROPPED)
#define ADSL_EOC_RDWR_EVENT         (ADSL_EOC_FRAME_RCVD | ADSL_EOC_FRAME_SENT)

#define ADSL_EOC_HDR                {0xff, 0x03, 0x81, 0x4c}
#define ADSL_2P_HDR_OFFSET          2  /* 0x81, 4c */
#define ADSL_HDR_OFFSET             0  /* 0xff, 0x3, 0x81, 4c */
#define ADSL_EOC_HDR_LEN            4
#define ADSL_2P_EOC_HDR_LEN         2
#define ADSL_EOC_ENABLE             {0xff, 0x03, 0x00, 0x57}

void AdslCheckLinkupMsg(void);
void snmp_adsl_eoc_event(unsigned char lineId);

#endif // _ADSLDRV_H_

