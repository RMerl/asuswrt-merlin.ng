/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

/****************************************************************************
 *
 * BcmCoreTest.c -- Bcm ADSL core driver main
 *
 * Description:
 *  This file contains BCM ADSL core driver system interface functions
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.3 $
 *
 * $Id: BcmAdslDiagRtems.c,v 1.3 2004/06/10 00:20:33 ilyas Exp $
 *
 * $Log: BcmAdslDiagRtems.c,v $
 * Revision 1.3  2004/06/10 00:20:33  ilyas
 * Added L2/L3 and SRA
 *
 * Revision 1.2  2004/04/21 18:44:29  ilyas
 * Fixed connection failure on second and next connections
 *
 * Revision 1.1  2004/04/21 04:02:58  ilyas
 * Diags support for RTEMS
 *
 ****************************************************************************/


/* Includes. */
#include "xapi.h"
#include "rtems.h"
#include "timer.h"
//#include "arptable.h"
#include "dbgio.h"
#include "alloc.h"
#include "subnet.h"
#include "ni_eth.h"
#include "ni.h"
#include "eth.h"
#include "ethadr.h"
#include "BcmOs.h"
#include "sbmips.h"
#include "mips.h"
#if defined(BOARD_bcm96338)
#include "6338_common.h"
#include "6338_map.h"
#include "6338_intr.h"
#elif defined(BOARD_bcm96348)
#include "6348_common.h"
#include "6348_map.h"
#include "6348_intr.h"
#endif
#include "bcmadsl.h"
#define ulong unsigned long
#include "BcmAdslCore.h"
#undef ulong
#include "AdslCore.h"
#include "AdslCoreMap.h"

#define EXCLUDE_CYGWIN32_TYPES
#include "softdsl/SoftDsl.h"

#include "BcmAdslDiag.h"
#include "DiagSock.h"

#undef  DIAG_DBG

#define MAX_LOG_TIME        512

#define VBF_NOT_FROM_POOL   VBF_PRIO_MSK
#define ADSL_DIAG_OWNER     MK_CID (CMA_DEV0, 0, 0)
#define NR_POOL_BUFS        50


extern dslCommandStruct adslCoreConnectionParam;
extern Bool adslCoreInitialized;
extern void BcmAdslCoreReset(void);

void BcmAdslCoreDiagDataInit(void);
void BcmAdslCoreDiagCommand(void);
void BcmAdslDiagRelease(VB f_list);

/* Local vars */

UINT32           diagDataMap = 0;
UINT32           diagLogTime = 0;
UINT32           diagTotalBufNum = 0;

UINT32           diagDmaIntrCnt = 0;
UINT32           diagDmaBlockCnt = 0;
UINT32           diagDmaOvrCnt = 0;
UINT32           diagDmaSeqBlockNum = 0;
UINT32           diagDmaSeqErrCnt = 0;
UINT32           diagDmaBlockWrCnt = 0;
UINT32           diagDmaBlockWrErrCnt = 0;
UINT32           diagDmaBlockWrBusy = 0;
UINT32           diagDmaLogBlockNum = 0;

int             diagEnableCnt = 0;




OS_SEMID g_DiagSem = -1;
SOCKET diag_cmd_sock = -1;
SOCKET globaldiagSOCKET = 0;
struct sockaddr gDiagSrcAddr;
int gDiagSrcAddrLen;

static char buffer_for_non_owners[sizeof(LogProtoHeader)+LOG_MAX_DATA_SIZE];
static char * p_buffer_for_non_owners = buffer_for_non_owners;
static UINT32 event_buf;
static UINT32 dma_buf;

static unsigned short waittime = 1;


OS_TASKID       BcmDiagTid= (OS_TASKID) 0;

#ifdef THROWAWAY
static IF_ST adsl_if = {
    "ADSL",             /* Interface name. */
    IFF_UP | IFF_POOL,  /* Interface flags. */
    0xffff,             /* Maximum transmission unit size = 64 KB. */
    0,                  /* Event flags. */
    NULL,               /* Error strings. */
    NULL,               /* Buffer send function. */
    NULL,               /* Input bind function. */
    BcmAdslDiagRelease, /* Free buffers function. */
    NULL,               /* Control function. */
    NULL,               /* Event reporting function. */
    NULL,               /* Interrupt handler function. */
    NULL,               /* Unbind function. */
    NULL,               /* State bind function. */
    NULL                /* Connection removal function. */
};
#endif

/*
**
**  Socket diagnostic support
**
*/

#define UNCACHED(p)             ((void *)((INT32)(p) | 0x20000000))
#define CACHED(p)               ((void *)((INT32)(p) & ~0x20000000))

#define kDiagDmaBlockSizeShift  11
#define kDiagDmaBlockSize       (1 << kDiagDmaBlockSizeShift)


typedef struct _diagIpHeader {
    uchar   ver_hl;         /* version & header length */
    uchar   tos;            /* type of service */
    ushort  len;            /* total length */
    ushort  id;             /* identification */
    ushort  off;            /* fragment offset field */
    uchar   ttl;            /* time to live */
    uchar   proto;          /* protocol */
    ushort  checksum;       /* checksum */
    UINT32   srcAddr;
    UINT32   dstAddr;        /* source and dest address */
} diagIpHeader;

typedef struct _diagUdpHeader {
    ushort  srcPort;        /* source port */
    ushort  dstPort;        /* destination port */
    ushort  len;            /* udp length */
    ushort  checksum;       /* udp checksum */
} diagUdpHeader;

#define DIAG_FRAME_PAD_SIZE     2
#define DIAG_FRAME_HEADER_LEN   (sizeof(diagSockFrame) - LOG_MAX_DATA_SIZE - DIAG_FRAME_PAD_SIZE)

struct ethhdr 
{
    unsigned char   h_dest[ETHADR_LEN];   /* destination eth addr */
    unsigned char   h_source[ETHADR_LEN]; /* source ether addr    */
    unsigned short  h_proto;            /* packet type ID field */
};

typedef struct _diagSockFrame {
    uchar               pad[DIAG_FRAME_PAD_SIZE];
    struct ethhdr       eth;
    diagIpHeader        ipHdr;
    diagUdpHeader       udpHdr;
    LogProtoHeader      diagHdr;
    uchar               diagData[LOG_MAX_DATA_SIZE];
} diagSockFrame;


typedef struct {
    VB_ST               vbuf;

    UINT32               len;
    UINT32               frameNum;
    UINT32               mark;
    LogProtoHeader      diagHdrDma;
    diagSockFrame       dataFrame;
} diagDmaBlock;

volatile int g_intf = -1;

#ifdef THROWAWAY
char g_dsfModel[sizeof(diagSockFrame) - LOG_MAX_DATA_SIZE];
UINT32 g_poolId =  0;
#endif

volatile uchar           *diagBuf, *diagBufUC;
volatile diagDmaBlock    *diagStartBlock = NULL;
volatile diagDmaBlock    *diagWriteBlock = NULL;
volatile diagDmaBlock    *diagCurrBlock = NULL;
volatile diagDmaBlock    *diagEndBlock  = NULL;
volatile diagDmaBlock    *diagEyeBlock = NULL;
volatile diagDmaBlock    *diagUDPBlock = NULL;

static volatile diagDmaBlock * gdb = NULL;


typedef struct {
    UINT32       flags;
    UINT32       addr;
} adslDmaDesc;

volatile adslDmaDesc     *diagDescTbl = NULL;
volatile adslDmaDesc     *diagDescTblUC = NULL;

#if defined (COMPONENT_SANDBOX)
UINT32 TDSL_traceGlobal = 1;
#else
extern UINT32 TDSL_traceGlobal;
#endif
void DiagPrintf( char *fmt, ... )
{
    va_list arg;
    char buf[128];
    if (TDSL_traceGlobal) {
    va_start (arg, fmt);
    vsprintf (buf, fmt, arg);
    va_end (arg);
  
    trc_print(buf);
    }
}

static ushort DiagIpComputeChecksum(diagIpHeader *ipHdr)
{
    ushort  *pHdr = (ushort *) ipHdr;
    ushort  *pHdrEnd = pHdr + 10;
    UINT32   sum = 0;

    do {
        sum += pHdr[0];
        sum += pHdr[1];
        pHdr += 2;
    } while (pHdr != pHdrEnd);

    while (sum > 0xFFFF)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return sum ^ 0xFFFF;
}

static ushort DiagIpUpdateChecksum(int sum, ushort oldv, ushort newv)
{
    ushort  tmp;

    tmp = (newv - oldv);
    tmp -= (tmp >> 15);
    sum = (sum ^ 0xFFFF) + tmp;
    sum = (sum & 0xFFFF) + (sum >> 16);
    return sum ^ 0xFFFF;
}

static void DiagUpdateDataLen(diagSockFrame *diagFr, int dataLen)
{
    int     n;

    diagFr->udpHdr.len = dataLen + sizeof(LogProtoHeader) + sizeof(diagUdpHeader);
    n = diagFr->udpHdr.len + sizeof(diagIpHeader);

    diagFr->ipHdr.checksum = DiagIpUpdateChecksum(diagFr->ipHdr.checksum, diagFr->ipHdr.len, n);
    diagFr->ipHdr.len = n;
}

#if 0
static void DiagPrintData(VB vbp)
{
    int     i;

    DiagPrintf ("***VB:count=%u, flags=%lu, data ptr=0x%8.8lx\n", 
        vbp->vb_count, vbp->vb_flags, (unsigned long) vbp->vb_data_ptr);

    for (i = 0; i < vbp->vb_count; i++)
        DiagPrintf("%2.2x ", vbp->vb_data_ptr[i]);
    DiagPrintf("\n");
}
#endif

void DiagFlushDma()
{
    unsigned char stop = 0;
    int res;
    //DiagPrintf("DiagFillDma\n\r");
    while(g_intf != -1 && !stop)
    {
        // init counter
        if (gdb == NULL)
            gdb = diagUDPBlock;
        // send
        
        res = sendto(globaldiagSOCKET,
                     //db,
                     gdb->vbuf.vb_data_ptr,
                     gdb->vbuf.vb_length,
                     0,//MSG_RAWMEM,
                     &gDiagSrcAddr,
                     gDiagSrcAddrLen);
        
        if (LOG_SOCKET_ERROR(res)) { 
            DbgPrintf (("Can't write to socket. Error = %d\n", LOG_SOCKET_ERRCODE() ));
            break;
        }
        
        // clear buffer (hardware can reuse it)
        gdb->dataFrame.pad[0] = 0;
        
        if (diagUDPBlock == gdb)
            stop = 1;
        // increase counter
        gdb = (void *) (((char*)gdb) + kDiagDmaBlockSize);
        if (gdb == CACHED(diagEndBlock))
            gdb = CACHED(diagStartBlock);
        
    }
}


void DiagFlushData(uchar cmd, char * buf, int len)
{
    int res = 0;
    char bufferke[1500];

    if (g_intf == -1) return;
    LogProtoHeader * proto = (LogProtoHeader *) bufferke;
    *(short *) proto->logProtoId = *(short *) LOG_PROTO_ID;
    proto->logPartyId  = LOG_PARTY_CLIENT;
    proto->logCommmand = cmd;
    //DiagPrintf("DiagFillData\n\r");
    memcpy (bufferke+sizeof(LogProtoHeader), buf, len);
    
    res = sendto(globaldiagSOCKET,
                 bufferke,
                 sizeof(LogProtoHeader)+len,
                 0,
                 &gDiagSrcAddr,
                 gDiagSrcAddrLen);
    
    if (LOG_SOCKET_ERROR(res)) 
        DbgPrintf (("Can't write to socket. Error = %d\n", LOG_SOCKET_ERRCODE() ));
}

int DiagFlushBuffer()
{
    int res = 0;
    if ((g_intf != -1) && p_buffer_for_non_owners != buffer_for_non_owners) // there is data in buffer
    {
        while (bcmOsSemTake(g_DiagSem,OS_WAIT_FOREVER) != OS_STATUS_OK);
        
        if (g_intf != -1) {
            //DiagPrintf("DiagFlushBuffer\n\r");
            res = sendto(globaldiagSOCKET,
                         buffer_for_non_owners,
                         p_buffer_for_non_owners - buffer_for_non_owners,
                         0,
                         &gDiagSrcAddr,
                         gDiagSrcAddrLen);
            
            if (LOG_SOCKET_ERROR(res)) {
                DbgPrintf (("Can't write to socket. Error = %d\n", LOG_SOCKET_ERRCODE() ));
                bcmOsSemGive(g_DiagSem);
                return -1;
            }
        }
        p_buffer_for_non_owners = buffer_for_non_owners;
        bcmOsSemGive(g_DiagSem);
    }
    return 0;
}


int DiagFillBuffer(uchar cmd, char *buf,int len)
{
    unsigned char tryagain = 1;
    LogProtoHeader *proto;
    if (g_DiagSem != -1)
    while (tryagain && g_intf != -1)
    {
        if (bcmOsSemTake(g_DiagSem,OS_WAIT_FOREVER) == OS_STATUS_OK)
        {
            // check if in the meanwhile not somebody else has used it
            if (p_buffer_for_non_owners == buffer_for_non_owners) 
            {
                //DiagPrintf("DiagFillBuffer\n\r");
                p_buffer_for_non_owners += len + sizeof(LogProtoHeader);
                
                proto = (LogProtoHeader *) buffer_for_non_owners;
                *(short *)proto->logProtoId = *(short *) LOG_PROTO_ID;
                proto->logPartyId  = LOG_PARTY_CLIENT;
                proto->logCommmand = cmd;
            
                memcpy (buffer_for_non_owners + sizeof(LogProtoHeader), buf, len);
                tryagain = 0;
                bcmOsSemGive(g_DiagSem);
            }
            else // there is still data in the buffer to be transmitted , wait and try again.
            {
                    bcmOsSemGive(g_DiagSem);
                    bcmOsSleep(10);
            }
        }
    }
    return 0;
}


int DiagWriteData(uchar cmd, char *buf0, int len0, char *buf1, int len1)
{
    UINT32 tid; 

    if (g_intf != -1)
    {
    
        xt_ident(NULL,0L,&tid);
        
        if (tid == BcmDiagTid) // this is diag_task
        {
            // flush the data
            DiagFlushData(cmd,buf0,len0);
            DiagFlushData(cmd,buf1,len1);
        }
        else // not diagtask
        {
            // fill the buffer make sure it is empty
            DiagFillBuffer(cmd,buf0,len0);
            DiagFillBuffer(cmd,buf1,len1);
            // notify diagtask
            bcmOsEvtSend(BcmDiagTid,event_buf);
        }
    }
    
    return(0);
}

void BcmAdslCoreDiagWriteStatusData(UINT32 cmd, char *buf0, int len0, char *buf1, int len1)
{
	DiagWriteData(cmd, buf0, len0, buf1, len1);
}

int BcmAdslCoreDiagWrite(void *pBuf, int len)
{
    DiagProtoFrame  *pDiagFrame = (DiagProtoFrame *) pBuf;

    return DiagWriteData(pDiagFrame->diagHdr.logCommmand,
        pDiagFrame->diagData, len - sizeof(LogProtoHeader),NULL,0);
}

#define DiagWriteMibData(dev,buf,len)       DiagWriteData(LOG_CMD_MIB_GET,buf,len,NULL,0)
#define DiagWriteStatusString(str)          DiagWriteData(LOG_CMD_SCRAMBLED_STRING,str,strlen(str)+1,NULL,0)

#if 0
#include "softdsl/StatusParser.h"
LOCAL void BcmAdslCoreDiagWriteStatusOrig(dslStatusStruct *status)
{
    static  char    statStr[4096];
    static  char    statStrAnnex[] = "\n";
    long            n, len;
    char            ch1 = 0, ch2 = 0, *pStr;

    if (-1 == g_intf)
        return;

    StatusParser (status, statStr);
    if (statStr[0] == 0)
        return;

    strcat(statStr, statStrAnnex);
    len = strlen(statStr);
    pStr = statStr;

    while (len > (LOG_MAX_DATA_SIZE-1)) {
        n = LOG_MAX_DATA_SIZE-1;
        ch1 = pStr[n-1];
        ch2 = pStr[n];
        pStr[n-1] = 1;
        pStr[n] = 0;

        DiagWriteData(LOG_CMD_STRING_DATA, pStr, n + 1);

        pStr[n-1] = ch1;
        pStr[n] = ch2;
        pStr += (n-1);
        len -= (n - 1);
    }
    DiagWriteData(LOG_CMD_STRING_DATA, pStr, len + 1);
}
#endif

void BcmAdslCoreDiagWriteLog(logDataCode logData, ...)
{
    static  char    logDataBuf[512];
    char            *logDataPtr = logDataBuf;
	INT32			n, i, *pCmdData;
    va_list         ap;

    if ((-1 == g_intf) || (0 == (diagDataMap & DIAG_DATA_LOG)))
        return;

    va_start(ap, logData);

    switch  (logData) {
        case    commandInfoData:
            logDataPtr += sprintf(logDataPtr, "%d:\t", (int)logData);   
			pCmdData = (void *) va_arg(ap, INT32);
			n = va_arg(ap, INT32);
			for (i = 0; i < n ; i++)
				logDataPtr += sprintf(logDataPtr, "%ld ", pCmdData[i]);	
            logDataPtr += sprintf(logDataPtr, "\n");    
            break;

    }

    if (logDataPtr != logDataBuf)
        DiagWriteData(logData, logDataBuf, (logDataPtr - logDataBuf),NULL,0);
    va_end(ap);
}

static void DiagUpdateVbForDmaBlock(diagDmaBlock *db, int len)
{
    DiagUpdateDataLen(&db->dataFrame, len);
    *(UINT32*)&db->dataFrame.diagHdr = *(UINT32*)&db->diagHdrDma;

    db->vbuf.vb_count = db->vbuf.vb_length = sizeof(LogProtoHeader) + len;
}

static int DiagWriteDmaBlock(diagDmaBlock *db)
{
    diagUDPBlock = db;
    bcmOsEvtSend(BcmDiagTid,dma_buf);
    return( 0 );
}

int BcmAdslCoreDiagInit(PADSL_DIAG pAdslDiag)
{
#ifdef THROWAWAY
    static int firstTime = 1;
#endif

    /* Assume the Ethernet interface index is the first one, 0. */
    int intf = 0;

    DiagPrintf("DrvDiagCmd: CONNECT map=0x%X, logTime=%d srvIpAddr=0x%8.8lx "
               "gwIpAddr=0x%8.8lx\r\n", pAdslDiag->diagMap, pAdslDiag->logTime,
               pAdslDiag->srvIpAddr, pAdslDiag->gwIpAddr);

    diagDataMap = pAdslDiag->diagMap;
    diagLogTime = pAdslDiag->logTime;
    diagEnableCnt = 1;

#ifdef THROWAWAY
    if( firstTime == 1 ) {
        int error;

        firstTime = 0;

        /* Create a VB buffer pool. */
        if ((error = if_attach (CMA_DEV0, &adsl_if)) == 0) {

            if( pool_create("ADSL", NR_POOL_BUFS, sizeof(diagSockFrame) +
                sizeof(VB_ST) + 4, MEMF_CHIP, &g_poolId) != POOL_OK ) {

                DiagPrintf("ADSL Diag: pool_create error\r\n");
                intf = -1;
            }
        }
        else {
            DiagPrintf("ADSL Diag: if_attach error %d\r\n", error);
            intf = -1;
        }
    }

    if( intf == 0 ) {
        diagSockFrame *dd = (diagSockFrame *) g_dsfModel;

        memset( dd, 0x00, sizeof(g_dsfModel) );        
        if( eth_port_addr_get(intf, (ETHADR *) dd->eth.h_source ) == ETH_OK ) {

            extern INTF *Intf;
            extern unsigned NI_MaxIntf;
            unsigned i;
            unsigned short *mac1 = (unsigned short *) dd->eth.h_source;
            unsigned short *mac2;
            int arp_intf = 0;
            ARPEntry *ae;

            for( i = NI_MaxIntf - 1; i > 0 ; i-- )
            {
                if( Intf[i].ni_flags != NIFL_NONE )
                {
                    INTF *pi = &Intf[i];
                    mac2 = (unsigned short *) pi->ni_hwadr;
                    if(mac1[0]==mac2[0] && mac1[1]==mac2[1] && mac1[2]==mac2[2])
                    {
                        arp_intf = i;
                        break;
                    }
                }
            }


            
            ae = arptable_arp_find(arp_intf, pAdslDiag->srvIpAddr);
            if( ae ) {
                memcpy( dd->eth.h_dest, ae->ae_hwa, ETHADR_LEN );
                dd->eth.h_proto = ETYPE_IP;
                dd->ipHdr.ver_hl = 0x45;
                dd->ipHdr.tos = 0;
                dd->ipHdr.len = 0;          /* changes for frames */
                dd->ipHdr.id = 0x2000;
                dd->ipHdr.off = 0;
                dd->ipHdr.ttl = 128;
                dd->ipHdr.proto = 0x11;     /* always UDP */
                dd->ipHdr.checksum = 0;     /* changes for frames */
                dd->ipHdr.srcAddr = AP_BestSrcAll(pAdslDiag->srvIpAddr);
                dd->ipHdr.dstAddr = pAdslDiag->srvIpAddr;
                dd->ipHdr.checksum = DiagIpComputeChecksum(&dd->ipHdr);

                dd->udpHdr.srcPort = LOG_FILE_PORT;
                dd->udpHdr.dstPort = LOG_FILE_PORT;
                dd->udpHdr.len = 0;         /* changes for frames */
                dd->udpHdr.checksum = 0;    /* not used */

                DiagPrintf("srcIPAddr=0x%8.8lx, destIPAddr=0x%8.8lx\r\n",
                    dd->ipHdr.srcAddr, dd->ipHdr.dstAddr);
                DiagPrintf("srcMACAddr=%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\r\n", 
                    dd->eth.h_source[0], dd->eth.h_source[1],
                    dd->eth.h_source[2], dd->eth.h_source[3],
                    dd->eth.h_source[4], dd->eth.h_source[5]);
                DiagPrintf("destMACAddr=%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\r\n", 
                    dd->eth.h_dest[0], dd->eth.h_dest[1],
                    dd->eth.h_dest[2], dd->eth.h_dest[3],
                    dd->eth.h_dest[4], dd->eth.h_dest[5]);
            }
            else {
                DiagPrintf("ADSL Diag: unable to get dest MAC address\r\n");
                intf = -1;
            }

        }
        else {
            DiagPrintf("ADSL Diag: unable to get source MAC address\r\n");
            intf = -1;
        }
    }
#endif
    return( intf );
}

void BcmAdslCoreDiagDataFlush(void)
{
    int             i, n = 0;
    diagDmaBlock    *diagPtr;

    do {
        diagPtr = (void *) diagStartBlock;
        diagPtr = CACHED(diagPtr);

        for (i = 0; i < diagTotalBufNum; i++) {
            n = (int) (diagPtr->vbuf.vb_flags & VBF_NOT_FROM_POOL);
            if ((diagPtr->dataFrame.pad[0] != 0) && (VBF_NOT_FROM_POOL == n))
                break;

            diagPtr = (void *) (((char *) diagPtr)  + kDiagDmaBlockSize);
        }

        if (i >= diagTotalBufNum)
            break;
                
#ifdef DIAG_DBG
        DiagPrintf ("DiagDataFlush waiting for block %d to be sent. users = 0x%X\r\n", i, n);
#endif      
    } while (1);
}

void BcmAdslCoreDiagDmaInit(void)
{
	volatile UINT32	*pAdslEnum = (UINT32 *) ADSL_ENUM_BASE;

	pAdslEnum[ADSL_INTMASK_F] = 0;
	pAdslEnum[RCV_PTR_FAST] = 0;
	pAdslEnum[RCV_CTL_FAST] = 0;

	diagDmaBlockCnt = 0;
	diagDmaOvrCnt = 0;
	diagDmaSeqErrCnt = 0;
	diagDmaBlockWrCnt = 0;
	diagDmaBlockWrErrCnt = 0;
	diagDmaBlockWrBusy = 0;

	diagStartBlock = (void*)diagBufUC;
	diagCurrBlock = diagStartBlock;
    diagUDPBlock = diagStartBlock;
	diagWriteBlock = diagStartBlock;
	diagEyeBlock = diagStartBlock;
	diagEndBlock  = (void *) (((char*) diagCurrBlock) + diagTotalBufNum*kDiagDmaBlockSize);
    gdb = NULL;

	pAdslEnum[ADSL_INTMASK_F] |= ADSL_INT_RCV;

	pAdslEnum[RCV_ADDR_FAST] = (UINT32) diagDescTbl & 0x1FFFFFFF;
	pAdslEnum[RCV_CTL_FAST] = 1 | ((FLD_OFFSET(diagDmaBlock, dataFrame.diagData) - FLD_OFFSET(diagDmaBlock, len)) << 1);
	pAdslEnum[RCV_PTR_FAST] = diagTotalBufNum << 3;
}

void BcmAdslCoreDiagStartLog(UINT32 map, UINT32 time)
{
    /* Not implemented. */
}

LOCAL void BcmAdslCoreDiagDmaUninit(void)
{
	volatile UINT32	*pAdslEnum = (UINT32 *) ADSL_ENUM_BASE;

	pAdslEnum[ADSL_INTMASK_F] = 0;
	pAdslEnum[RCV_PTR_FAST] = 0;
	pAdslEnum[RCV_CTL_FAST] = 0;
    gdb = NULL;
	BcmAdslCoreDiagDataFlush();
}

void * BcmAdslCoreDiagGetDmaDataAddr(int descNum)
{
	diagDmaBlock	*diagDmaPtr;

	diagDmaPtr = (void *) ((diagDescTblUC[descNum].addr & ~0xFF) | 0xA0000000);
	return &diagDmaPtr->dataFrame.diagData;
}

int BcmAdslCoreDiagGetDmaDataSize(int descNum)
{
	diagDmaBlock	*diagDmaPtr;

	diagDmaPtr = (void *) ((diagDescTblUC[descNum].addr & ~0xFF) | 0xA0000000);
	return diagDmaPtr->len;
}

int	 BcmAdslCoreDiagGetDmaBlockNum(void)
{
	return diagTotalBufNum;
}

void BcmAdslCoreDiagDataInit(void)
{
    int                 i;
    ushort              ipId;
    VB                  vbp;

    if (0 == diagDataMap)
        return;

    if (NULL == diagDescTbl) {
        if (diagDataMap & DIAG_DATA_EYE)
            diagTotalBufNum = 32;
        if (diagDataMap & DIAG_DATA_LOG)
            diagTotalBufNum = 511;

        diagDescTbl = (void *) xmalloc(diagTotalBufNum * kDiagDmaBlockSize + 0x2000);
        while (NULL == diagDescTbl && diagTotalBufNum) {
            diagTotalBufNum--;
            diagDescTbl = (void *) xmalloc(diagTotalBufNum * kDiagDmaBlockSize + 0x2000);
        }
        if (NULL == diagDescTbl) {
            diagDataMap = 0;
            return;
        }

        DiagPrintf ("diagDmaBlockSize=%d, diagTotalBufNum=%d, diagDescTbl = 0x%X\r\n", 
            sizeof(diagDmaBlock), (int)diagTotalBufNum, (int)diagDescTbl);
        diagDescTbl = (void *) (((UINT32)diagDescTbl + 0xFFF) & ~0xFFF);
    }
	else 
		BcmAdslCoreDiagDmaUninit();

    diagDescTblUC = UNCACHED(diagDescTbl);
    diagBuf = (uchar *)diagDescTbl + 0x1000;
    diagBufUC = UNCACHED(diagBuf);
    diagCurrBlock = (void*) diagBuf;

    for (i = 0; i < diagTotalBufNum; i++) {
        diagDescTblUC[i].flags = kDiagDmaBlockSize - FLD_OFFSET(diagDmaBlock, len);
        diagDescTblUC[i].addr  = ((UINT32)diagBuf & 0x1FFFFFFF) + FLD_OFFSET(diagDmaBlock, len);

        diagCurrBlock = (void*) diagBuf;
        vbp = (VB) &diagCurrBlock->vbuf;
        memset(vbp, 0, sizeof(*vbp));


        vbp->vb_data_ptr = (unsigned char *) &diagCurrBlock->dataFrame.diagHdr;
        vbp->vb_size = LOG_MAX_DATA_SIZE + sizeof(LogProtoHeader);
        //vbp->vb_data_ptr = (unsigned char *) &diagCurrBlock->dataFrame.eth;
        //vbp->vb_size = sizeof(diagSockFrame) - DIAG_FRAME_PAD_SIZE;
        vbp->vb_last = vbp->vb_sublast = vbp;
        vbp->vb_owner = ADSL_DIAG_OWNER;


        //memcpy(vbp->vb_data_ptr, g_dsfModel + DIAG_FRAME_PAD_SIZE, DIAG_FRAME_HEADER_LEN);

        ipId = diagCurrBlock->dataFrame.ipHdr.id;
        diagCurrBlock->dataFrame.ipHdr.id = 0x4000 + i;
        diagCurrBlock->dataFrame.ipHdr.checksum = DiagIpUpdateChecksum(
            diagCurrBlock->dataFrame.ipHdr.checksum, 
            ipId,
            diagCurrBlock->dataFrame.ipHdr.id);

        diagCurrBlock->len = 0;
        diagCurrBlock->frameNum = 0;
        diagCurrBlock->mark   = 0;
        diagCurrBlock->dataFrame.pad[0] = 0;

#if 0
        diagCurrBlock->diagHdrDma.logProtoId[0] = '*';
        diagCurrBlock->diagHdrDma.logProtoId[1] = 'L';
        diagCurrBlock->diagHdrDma.logPartyId = 1;
#if 0
        diagCurrBlock->dataFrame.eth.h_proto = 0x2A4C;
        diagCurrBlock->diagHdrDma.logCommmand = 0xF;
        diagCurrBlock->dataFrame.diagData[0] = 0;
        diagCurrBlock->dataFrame.diagData[1] = 0;
        diagCurrBlock->dataFrame.diagData[2] = (i >> 8) & 0xFF;
        diagCurrBlock->dataFrame.diagData[3] = i & 0xFF;
#else
        diagCurrBlock->diagHdrDma.logCommmand = LOG_CMD_STRING_DATA;
        diagCurrBlock->dataFrame.diagData[0] = 'A' + (i & 0xFF);
        diagCurrBlock->dataFrame.diagData[1] = 'B' + (i & 0xFF);
        diagCurrBlock->dataFrame.diagData[2] = 'C' + (i & 0xFF);
        diagCurrBlock->dataFrame.diagData[3] = 0;
#endif
        DiagUpdateVbForDmaBlock(diagCurrBlock, 4);
        DiagWriteDmaBlock (diagCurrBlock);
        /* DiagPrintData(skb); */
#endif

        diagBuf   += kDiagDmaBlockSize;
        diagBufUC += kDiagDmaBlockSize;
    }
    diagDescTblUC[diagTotalBufNum-1].flags |= 0x10000000;

    diagBuf = (uchar *)diagDescTbl + 0x1000;
    diagBufUC = UNCACHED(diagBuf);

	BcmAdslCoreDiagDmaInit();
    return;
}

void BcmAdslCoreDiagWriteBlocks(void)
{
    int             n;
    diagDmaBlock    *diagPtr;

    while (diagWriteBlock != diagCurrBlock) {
        diagPtr = CACHED(diagWriteBlock);

        n = (int) (diagPtr->vbuf.vb_flags & VBF_NOT_FROM_POOL);
        if ((diagPtr->dataFrame.pad[0] != 0) && (VBF_NOT_FROM_POOL == n))
            break;

#if 0
        if ((-1 != g_intf) && (eyeData != diagPtr->diagHdrDma.logCommmand))
#else
        if (-1 != g_intf) 
#endif
        {
            diagPtr->dataFrame.pad[0] = 1;
            diagPtr->vbuf.vb_flags |= VBF_HEADER | VBF_NOT_FROM_POOL;
            n = DiagWriteDmaBlock(diagPtr);
            if (n != 0)
                break;

            //diagPtr->dataFrame.pad[0] = 0; will be done assynchronously in working thread.
        }
        diagDmaBlockWrCnt++;

        diagWriteBlock = (void *) (((char*) diagWriteBlock) + kDiagDmaBlockSize);
        if (diagWriteBlock == diagEndBlock)
            diagWriteBlock = diagStartBlock;
    }
}

Bool BcmAdslCoreDiagIntrHandler(void)
{
    volatile UINT32  *pAdslEnum = (UINT32 *) ADSL_ENUM_BASE;
    UINT32           intStatus = pAdslEnum[ADSL_INTSTATUS_F];
    diagDmaBlock    *diagDmaPtr, *diagPtr;
    int             n;

    if (intStatus) {
        diagDmaIntrCnt++;
        pAdslEnum[ADSL_INTSTATUS_F] = intStatus;
    }

    if ((0 == diagDataMap) || (diagEnableCnt <= 0))
        return 0;

    diagDmaPtr = (void*) (((char*)diagStartBlock) + ((pAdslEnum[RCV_STATUS_FAST] & 0xFFF) << (kDiagDmaBlockSizeShift - 3)));
    while (diagCurrBlock != diagDmaPtr) {
        diagDmaBlockCnt++;
        diagPtr = CACHED(diagCurrBlock);
        n = (int) (diagPtr->vbuf.vb_flags & VBF_NOT_FROM_POOL);
        if ((diagPtr->dataFrame.pad[0] != 0) && (VBF_NOT_FROM_POOL == n))
            diagDmaOvrCnt++;

        if (diagCurrBlock->frameNum != diagDmaSeqBlockNum) {
#ifdef DIAG_DBG
            DiagPrintf ("blkNum=%ld, diagP=%X, dmaP=0x%X, len=%ld,anum=%ld,mark=0x%lX,lHdr=0x%lX,enum=%ld\r\n", 
                diagDmaBlockCnt, (int)diagCurrBlock, (int)diagDmaPtr, 
                diagCurrBlock->len,diagCurrBlock->frameNum, 
                diagCurrBlock->mark, *(UINT32*)&diagCurrBlock->diagHdrDma,diagDmaSeqBlockNum);
#endif
            diagDmaSeqErrCnt++;
            diagDmaSeqBlockNum = diagCurrBlock->frameNum;
        }
        diagDmaSeqBlockNum++;

        /* update vb for this DMA block */

        diagPtr->diagHdrDma = diagCurrBlock->diagHdrDma;
        DiagUpdateVbForDmaBlock(diagPtr, diagCurrBlock->len);

        diagCurrBlock->mark = 0;
        diagCurrBlock = (void *) (((char*) diagCurrBlock) + kDiagDmaBlockSize);
        if (diagCurrBlock == diagEndBlock)
            diagCurrBlock = diagStartBlock;

        if (diagCurrBlock == diagWriteBlock)
            diagDmaOvrCnt++;

        BcmAdslCoreDiagWriteBlocks();

#if 0
        diagDmaPtr = (void*) (((char*)diagStartBlock) + ((pAdslEnum[RCV_STATUS_FAST] & 0xFFF) << (kDiagDmaBlockSizeShift - 3)));
#endif
    }
    return 0;
}

void BcmAdslCoreDiagIsrTask(void)
{
}

int BcmAdslDiagGetConstellationPoints (int toneId, void *pointBuf, int numPoints)
{
    volatile UINT32  *pAdslEnum = (UINT32 *) ADSL_ENUM_BASE;
    diagDmaBlock    *diagDmaBlockPtr;
    UINT32           *pSrc, *pDst;
    int             i;

    if (0 == diagDataMap) {
        diagDataMap = DIAG_DATA_EYE;
        diagLogTime = 0;
        BcmAdslCoreDiagDataInit();
        BcmAdslCoreDiagCommand();
        return 0;
    }

    diagDmaBlockPtr = (void*) (((char*)diagStartBlock) + ((pAdslEnum[RCV_STATUS_FAST] & 0xFFF) << (kDiagDmaBlockSizeShift - 3)));
    while (diagEyeBlock != diagDmaBlockPtr) {
        if (eyeData == diagEyeBlock->diagHdrDma.logCommmand)
            break;

        diagEyeBlock = (void *) (((char*) diagEyeBlock) + kDiagDmaBlockSize);
        if (diagEyeBlock == diagEndBlock)
            diagEyeBlock = diagStartBlock;
    }
    if (diagEyeBlock == diagDmaBlockPtr)
        return 0;

    if (numPoints > (diagEyeBlock->len >> 3))
        numPoints = diagEyeBlock->len >> 3;

    pSrc = ((UINT32 *)diagEyeBlock->dataFrame.diagData) + (toneId != 0 ? 1 : 0);
    pDst = (UINT32 *) pointBuf;
    for (i = 0; i < numPoints; i++) {
        *pDst++ = *pSrc;
        pSrc += 2;
    }

    diagEyeBlock = (void *) (((char*) diagEyeBlock) + kDiagDmaBlockSize);
    if (diagEyeBlock == diagEndBlock)
        diagEyeBlock = diagStartBlock;
    return numPoints;
}

void BcmAdslCoreDiagCmd(PADSL_DIAG pAdslDiag)
{
    UINT32               origDiagMap;
    volatile int l_intf = -1;

    /* DiagPrintf ("DrvDiagCmd: %d\r\n", pAdslDiag->diagCmd); */
    switch (pAdslDiag->diagCmd) {
        case LOG_CMD_CONNECT:
            origDiagMap = diagDataMap;
            l_intf = BcmAdslCoreDiagInit(pAdslDiag);
            if (adslCoreInitialized && (-1 != l_intf)) {
                if (diagDataMap & DIAG_DATA_LOG) {
                    BcmAdslCoreReset();
                    g_intf = l_intf;
                }
                else {
                    BcmAdslCoreDiagDataInit();
                    g_intf = l_intf;
                    if (0 == origDiagMap) {
                        BcmAdslCoreDiagCommand();
                    }
                }
            }
            break;

		case LOG_CMD_DISCONNECT:
            g_intf = -1;
			diagDataMap = 0;
			BcmAdslCoreDiagCommand();
			break;


        case LOG_CMD_DEBUG:
            {
            DiagDebugData   *pDbgCmd = (void *)pAdslDiag->diagMap;

            switch (pDbgCmd->cmd) {
                case DIAG_DEBUG_CMD_CLEAR_STAT:
                    diagDmaIntrCnt  = 0;
                    diagDmaBlockCnt = 0;
                    diagDmaOvrCnt = 0;
                    diagDmaSeqErrCnt = 0;
                    diagDmaBlockWrCnt = 0;
                    diagDmaBlockWrErrCnt = 0;
                    diagDmaBlockWrBusy = 0;
                    break;
                case DIAG_DEBUG_CMD_PRINT_STAT:
                    BcmAdslCoreDiagWriteStatusString(
                        "DiagLinux Statistics:\n"
                        "   diagIntrCnt = %d\n"
                        "   dmaBlockCnt = %d\n"
                        "   dmaSeqNum   = %d\n"
                        "   dmaOvrCnt   = %d\n"
                        "   dmaSqErrCnt = %d\n"
                        "   ethWrCnt    = %d\n"
                        "   ethErrWrCnt = %d\n"
                        "   ethBusyCnt  = %d\n",
                        diagDmaIntrCnt,
                        diagDmaBlockCnt,
                        diagDmaSeqBlockNum,
                        diagDmaOvrCnt,
                        diagDmaSeqErrCnt,
                        diagDmaBlockWrCnt,
                        diagDmaBlockWrErrCnt,
                        diagDmaBlockWrBusy);
                    break;
            }
            BcmAdslCoreDebugCmd(pDbgCmd);
            }
            break;

		default:
			BcmAdslCoreDiagCmdCommon(pAdslDiag->diagCmd, pAdslDiag->logTime, (void*) pAdslDiag->diagMap);
			break;
    }
}

void BcmAdslCoreDiagCommand(void)
{
	dslCommandStruct	cmd;

	cmd.command = kDslDiagSetupCmd;
	cmd.param.dslDiagSpec.setup = 0;
	if (diagDataMap & DIAG_DATA_EYE)
		cmd.param.dslDiagSpec.setup |= kDslDiagEnableEyeData;
	if (diagDataMap & DIAG_DATA_LOG) {
		cmd.param.dslDiagSpec.setup |= kDslDiagEnableLogData;
		diagDmaLogBlockNum = 0;
		BcmAdslCoreDiagWriteLog(inputSignalData - 2, AC_TRUE);
		diagDmaLogBlockNum = 0;
	}
	cmd.param.dslDiagSpec.eyeConstIndex1 = 63; 
	cmd.param.dslDiagSpec.eyeConstIndex2 = 64;
	cmd.param.dslDiagSpec.logTime = diagLogTime;
	BcmCoreCommandHandlerSync(&cmd);
}


/***************************************************************************
 * Function Name: DiagConnect
 * Description  : Tries to connect to Diag server via sockets
 * Returns      : socket handle if successsful or -1 if error
 
 ***************************************************************************/
unsigned char diagOverTcp = 0;
static int DiagConnect( SOCKET *pdiagLibSock, INT32 diagRemoteIpAddr, int diagPort )
{
    ADSL_DIAG       Arg;
    SOCKET          diagSock;
    struct sockaddr diagSrcAddr;
    int             diagSrcAddrLen;
    int             diagLogTime = MAX_LOG_TIME;
    UINT32           diagMap = 0;

	if (diagRemoteIpAddr != 0) {
		diagSock = LogServerInit(diagRemoteIpAddr, diagPort, 20, 1000, 0, 0, diagOverTcp, 0);
		if (diagSock != -1) {
			diagMap = DIAG_DATA_EYE;
			diagSrcAddr.sa_family = AF_INET;
			((struct sockaddr_in *) &diagSrcAddr)->sin_addr.s_addr = diagRemoteIpAddr;
			((struct sockaddr_in *) &diagSrcAddr)->sin_port = htons(diagPort);
			diagSrcAddrLen = sizeof(diagSrcAddr);
		}
	}
	else
		diagSock = LogClientInit(pdiagLibSock, diagPort, &diagSrcAddr, &diagSrcAddrLen,
			(int *) &diagMap, 0, 0);

    if (diagSock != -1) {
        if (diagMap & DIAG_DATA_LOG_TIME) {
            diagLogTime = ((diagMap >> 8) & 0xFF00) | (diagMap >> 24);
            diagMap &= (~DIAG_DATA_LOG_TIME & 0xFF);
        }
        else
            diagLogTime = MAX_LOG_TIME;

        gDiagSrcAddr = diagSrcAddr;
        gDiagSrcAddrLen = diagSrcAddrLen;

        globaldiagSOCKET = diagSock;

        
        Arg.diagCmd   = LOG_CMD_CONNECT;
        Arg.diagMap   = diagMap;
        Arg.logTime   = diagLogTime;
        Arg.srvIpAddr = *((UINT32 *) (void *) &diagSrcAddr.sa_data[2]);
        Arg.gwIpAddr  = 0; /* TBD DiagGetDefaultGateway(); */
        BcmAdslCoreDiagCmd( &Arg );
    }

    return diagSock;
}
#ifndef COMPONENT_SANDBOX
unsigned char diagtoolrunning = 0; // GLOBAL VAR INDIACTING DIAGTOOL SERVER IS ACCEPTING CONNECTIONS OR NOT
#else
unsigned char diagtoolrunning = 1; // GLOBAL VAR INDIACTING DIAGTOOL SERVER IS ACCEPTING CONNECTIONS OR NOT
#endif
UINT32 diagRemoteIpAddr = 0;
unsigned int diagPort = LOG_FILE_PORT;
/***************************************************************************
** Function Name: BcmCoreDiagTask
** Description  : Runs in a separate thread of execution. Tries to establish
**                socket connection to LOG server
** Returns      : None.
***************************************************************************/
LOCAL void BcmCoreDiagTask()
{
    SOCKET          diagLibSock = -1;
    SOCKET          diagSock = -1;
    struct sockaddr srvIpAddr;
    int             srvIpAddrLen, res;
    DiagProtoFrame  diagCmd;
    ADSL_DIAG       Arg;
    int             bJustConnected = 0;
    UINT32          events_r = 0;
    char semname[] = "diac";
	/* long			diagRemoteIpAddr = 0xC0A80164; */
	int			diagPort = LOG_FILE_PORT;
    
    bcmOsEvtCreate(BcmDiagTid,&event_buf);
    bcmOsEvtCreate(BcmDiagTid,&dma_buf);
    bcmOsSemCreateCnt( semname,&g_DiagSem,1);
    srvIpAddrLen = sizeof(srvIpAddr);
    
    
    while (1) {
        if (diagSock == -1) {
            if (diagtoolrunning)
                diagSock = DiagConnect( &diagLibSock, diagRemoteIpAddr, diagPort );
            if (diagSock == -1) {
                bcmOsSleep( 1000 );
                continue;
            }
            else
                bJustConnected = 1;
        }


        
        res = LogRecvWithTimeout(diagSock, (char *)&diagCmd, sizeof(diagCmd), &srvIpAddr, 0, 0);


        if (waittime < 1000)  waittime = waittime + 100; 
        
        
        if (LOG_SOCKET_ERROR(res) ||
            ((res != 0) && (LOG_CMD_DISCONNECT == diagCmd.diagHdr.logCommmand))) {

            res = LOG_SOCKET_ERRCODE();
            DiagPrintf ("DrvDiagCmd : DISCONNECT\r\n");
            res = diagSock;
            gdb = NULL;
            diagSock = -1;
            diagLibSock = -1;
            closesocket(res);
            g_intf = -1;
            continue;
        }

        if (res != 0)
        {
            waittime = 1;
#if 0
            DiagPrintf ("DiagCmd received: len=%d, protoid=%c%c, cmd=%d\r\n", res,
                        diagCmd.diagHdr.logProtoId[0], diagCmd.diagHdr.logProtoId[1], diagCmd.diagHdr.logCommmand);
#endif
            if ((res < sizeof(LogProtoHeader)) || ((diagCmd.diagHdr.logPartyId & DIAG_PARTY_ID_MASK) != LOG_PARTY_SERVER))
                ;
            else
            {
                if (LOG_CMD_CONNECT != diagCmd.diagHdr.logCommmand) {
                    Arg.diagCmd   = diagCmd.diagHdr.logCommmand;
                    Arg.diagMap   = (int) diagCmd.diagData;
                    Arg.logTime   = res - sizeof(LogProtoHeader);
                    Arg.srvIpAddr = 0;
                    BcmAdslCoreDiagCmd( &Arg );
                }
                else {
                    if (!bJustConnected)
                        diagSock = -1;
                }
            }
        }
            
        bcmOsEvtReceive(event_buf | dma_buf, waittime, &events_r);
        if ((events_r & event_buf) != 0) // we received event
        {
            DiagFlushBuffer();
        }
        if ((events_r & dma_buf) != 0)
        {
            DiagFlushDma();
        }
    }
}

/***************************************************************************
** Function Name: BcmAdslDiagDisable/BcmAdslDiagEnable
** Description  : This function enables/disables diag interrupt processing
** Returns      : None.
***************************************************************************/
int BcmAdslDiagDisable(void)
{
    diagEnableCnt--;
    return diagEnableCnt;
}

int BcmAdslDiagEnable(void)
{
    diagEnableCnt++;
    return diagEnableCnt;
}

Bool BcmAdslDiagIsActive(void)
{
	return (g_intf != -1);
}

/***************************************************************************
** Function Name: BcmAdslDiagReset
** Description  : This function resets diag support after ADSL MIPS reset
** Returns      : None.
***************************************************************************/
void BcmAdslDiagReset(int map)
{
    BcmAdslDiagDisable();
    if (diagDataMap & (DIAG_DATA_LOG | DIAG_DATA_EYE)) {
        diagDmaSeqBlockNum = 0;
        BcmAdslCoreDiagDataInit();
    }
    BcmAdslDiagEnable();
    BcmAdslCoreDiagCommand();
}

/***************************************************************************
** Function Name: BcmAdslDiagInit
** Description  : This function intializes diag support on Host and ADSL MIPS
** Returns      : None.
***************************************************************************/
int BcmAdslDiagInit(int map)
{
    BcmAdslCoreDiagDataInit();
    BcmAdslCoreDiagCommand();
    return 0;
}



/**************************************************************************
** Function Name: BcmAdslDiagTaskInit
** Description  : This function intializes ADSL driver Diag task
** Returns      : None.
**************************************************************************/
int BcmAdslDiagTaskInit(void)
{
    bcmOsTaskCreate("BcmCoreDiag", 20*1024, 200, BcmCoreDiagTask, 0, &BcmDiagTid);

    return 0;
}

/**************************************************************************
** Function Name: BcmAdslDiagTaskUninit
** Description  : This function unintializes ADSL driver Diag task
** Returns      : None.
**************************************************************************/
void BcmAdslDiagTaskUninit(void)
{
    LogProtoHeader      logHeader;

    *(short *)logHeader.logProtoId = *(short *) LOG_PROTO_ID;
    logHeader.logPartyId  = LOG_PARTY_CLIENT;
    logHeader.logCommmand = LOG_CMD_DISCONNECT;
    BcmAdslCoreDiagWrite(&logHeader, sizeof(logHeader));

    if (BcmDiagTid != (OS_TASKID) 0) {
        bcmOsTaskDelete(BcmDiagTid);
        BcmDiagTid = 0;
    }
}


/**************************************************************************
** Function Name: BcmAdslDiagRelease
** Description  : Reclaim buffers that have been sent to Ethernet
** Returns      : None.
**************************************************************************/
void BcmAdslDiagRelease (VB f_list)
{
    VB f_list2, vbp;
    VB vbp_next = NULL;
    VB vbp_subnext = NULL;

    for( f_list2 = f_list; f_list2; f_list2 = vbp_next ) {

        vbp_next = f_list2->vb_next;
        for( vbp = f_list2; vbp; vbp = vbp_subnext ) {

            vbp_subnext = vbp->vb_subnext;

            vbp->vb_subnext = vbp->vb_next = vbp->vb_last = NULL;
            vbp->vb_last = vbp->vb_sublast = vbp;
            vbp->vb_cid = 0;

            if( (vbp->vb_flags & VBF_NOT_FROM_POOL) == 0 ) {

                CACHE_DMA_CLEAR((UINT8 *)vbp,sizeof(VB_ST)+sizeof(diagSockFrame));
                vb_free(vbp);
            }
            else
                CACHE_DMA_CLEAR((UINT8 *)vbp, kDiagDmaBlockSize);

            vbp->vb_flags = 0;
        }
    }
}

