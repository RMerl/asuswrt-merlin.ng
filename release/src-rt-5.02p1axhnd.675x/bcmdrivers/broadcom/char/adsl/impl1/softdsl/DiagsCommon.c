#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/kernel.h>
#else
#include <stdio.h>
#include <string.h>
#endif
#ifdef XDSLDRV_ENABLE_PARSER
#include "EndianUtil.h"
#else
#include "Diags.h"
#endif
#include "DiagDef.h"
#include "SoftDsl.h"
#include "AdslMibDef.h"
//#include "DiagsNoiseData.h"
#include "Flatten.h"

#if defined(G993) || defined(VDSL_MODEM)
#include "SoftDslG993p2.h"
#endif

#ifdef XDSLDRV_ENABLE_PARSER
#include "BlockUtil.h"
#include "bcm_map.h"

#define	DiagEndianCl2Srv(x)		ADSL_ENDIAN_CONV_SHORT(x)
#define	DiagEndianCl2SrvLong(x)		ADSL_ENDIAN_CONV_INT32(x)
#ifdef CONFIG_ARM64
extern uint	*pStackPtr;
#endif
extern int printMaxItem;
extern char *parseBufTmp;
#endif

short DiagEndianSrv2Cl(short shNum)
{
	return DiagEndianCl2Srv(shNum);
}

int DiagEndianSrv2ClLong(int lNum)
{
	return DiagEndianCl2SrvLong(lNum);
}

void DiagEndianBlockCl2Srv(short *pBuf, int nSamples)
{
	short *pBufEnd = pBuf + nSamples;
	
	if(nSamples < 1)
		return;
	
	do {
		*pBuf = DiagEndianCl2Srv(*pBuf);
	} while (++pBuf != pBufEnd);
}

void DiagEndianBlockLongCl2Srv(int *pBuf, int nSamples)
{
	int *pBufEnd = pBuf + nSamples;
	
	if(nSamples < 1)
		return;
	
	do {
		*pBuf = DiagEndianCl2SrvLong(*pBuf);
	} while (++pBuf != pBufEnd);
}
#ifdef XDSLDRV_ENABLE_PARSER
void DiagEndianBlockInt64Cl2Srv(ulonglong *pBuf, int *pDstBuf, int nSamples)
{
	unsigned int	test1, test2;
	unsigned int	*pBuf1;
	unsigned int	*pBuf0;
	ulonglong *pBufEnd = pBuf + nSamples;
	
	if(nSamples < 1)
		return;
	
	do {
		pBuf0=(void *)pBuf;
		pBuf1=pBuf0+1;
		test1 = DiagEndianCl2SrvLong(*pBuf0);
		test2 = DiagEndianCl2SrvLong(*pBuf1);
		*pDstBuf++ = test2;
		*pDstBuf++ = test1;
	} while (++pBuf != pBufEnd);
}
#else
void DiagEndianBlockInt64Cl2Srv(ulonglong *pBuf, int nSamples)
{
	unsigned int	test1, test2;
	unsigned int	*pBuf1;
	unsigned int	*pBuf0;
	ulonglong *pBufEnd = pBuf + nSamples;
	
	if(nSamples < 1)
		return;
	
	do {
		pBuf0=(void *)pBuf;
		pBuf1=pBuf0+1;
		test1 = DiagEndianCl2SrvLong(*pBuf0);
		test2 = DiagEndianCl2SrvLong(*pBuf1);
		*pBuf0=test2;
		*pBuf1=test1;
	} while (++pBuf != pBufEnd);
}
#endif

void DiagEndianG992p3CapCl2Srv(g992p3PhyDataPumpCapabilities *pG992p3Cap, uint msgType)
{
	int		i;

	pG992p3Cap->rcvNOMPSDus = DiagEndianCl2Srv(pG992p3Cap->rcvNOMPSDus);
	pG992p3Cap->rcvMAXNOMPSDus = DiagEndianCl2Srv(pG992p3Cap->rcvMAXNOMPSDus);
	pG992p3Cap->rcvMAXNOMATPus = DiagEndianCl2Srv(pG992p3Cap->rcvMAXNOMATPus);
	for (i = 0; i < kG992p3p5MaxSpectBoundsUpSize; i++) {
		pG992p3Cap->usSubcarrierIndex[i] = DiagEndianCl2Srv(pG992p3Cap->usSubcarrierIndex[i]);
		pG992p3Cap->usLog_tss[i] = DiagEndianCl2Srv(pG992p3Cap->usLog_tss[i]);
	}
	pG992p3Cap->numUsSubcarrier = DiagEndianCl2Srv(pG992p3Cap->numUsSubcarrier);

	pG992p3Cap->rcvNOMPSDds = DiagEndianCl2Srv(pG992p3Cap->rcvNOMPSDds);
	pG992p3Cap->rcvMAXNOMPSDds = DiagEndianCl2Srv(pG992p3Cap->rcvMAXNOMPSDds);
	pG992p3Cap->rcvMAXNOMATPds = DiagEndianCl2Srv(pG992p3Cap->rcvMAXNOMATPds);
	for (i = 0; i < kG992p3p5MaxSpectBoundsDownSize; i++) {
		pG992p3Cap->dsSubcarrierIndex[i] = DiagEndianCl2Srv(pG992p3Cap->dsSubcarrierIndex[i]);
		pG992p3Cap->dsLog_tss[i] = DiagEndianCl2Srv(pG992p3Cap->dsLog_tss[i]);
	}
	pG992p3Cap->numDsSubcarrier = DiagEndianCl2Srv(pG992p3Cap->numDsSubcarrier);

	for (i = 0; i < 4; i++) {
		pG992p3Cap->minDownSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minDownSTM_TPS_TC[i]);
		pG992p3Cap->maxDownSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDownSTM_TPS_TC[i]);
		pG992p3Cap->minRevDownSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minRevDownSTM_TPS_TC[i]);
		pG992p3Cap->maxDelayDownSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDelayDownSTM_TPS_TC[i]);

		pG992p3Cap->minUpSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minUpSTM_TPS_TC[i]);
		pG992p3Cap->maxUpSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxUpSTM_TPS_TC[i]);
		pG992p3Cap->minRevUpSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minRevUpSTM_TPS_TC[i]);
		pG992p3Cap->maxDelayUpSTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDelayUpSTM_TPS_TC[i]);

		pG992p3Cap->maxDownPMS_TC_Latency[i] = DiagEndianCl2Srv(pG992p3Cap->maxDownPMS_TC_Latency[i]);
		pG992p3Cap->maxUpPMS_TC_Latency[i] = DiagEndianCl2Srv(pG992p3Cap->maxUpPMS_TC_Latency[i]);
		pG992p3Cap->maxDownR_PMS_TC_Latency[i] = DiagEndianCl2Srv(pG992p3Cap->maxDownR_PMS_TC_Latency[i]);
		pG992p3Cap->maxDownD_PMS_TC_Latency[i] = DiagEndianCl2Srv(pG992p3Cap->maxDownD_PMS_TC_Latency[i]);
		pG992p3Cap->maxUpR_PMS_TC_Latency[i] = DiagEndianCl2Srv(pG992p3Cap->maxUpR_PMS_TC_Latency[i]);
		pG992p3Cap->maxUpD_PMS_TC_Latency[i] = DiagEndianCl2Srv(pG992p3Cap->maxUpD_PMS_TC_Latency[i]);

		pG992p3Cap->minDownATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minDownATM_TPS_TC[i]);
		pG992p3Cap->maxDownATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDownATM_TPS_TC[i]);
		pG992p3Cap->minRevDownATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minRevDownATM_TPS_TC[i]);
		pG992p3Cap->maxDelayDownATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDelayDownATM_TPS_TC[i]);

		pG992p3Cap->minUpATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minUpATM_TPS_TC[i]);
		pG992p3Cap->maxUpATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxUpATM_TPS_TC[i]);
		pG992p3Cap->minRevUpATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minRevUpATM_TPS_TC[i]);
		pG992p3Cap->maxDelayUpATM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDelayUpATM_TPS_TC[i]);

		pG992p3Cap->minDownPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minDownPTM_TPS_TC[i]);
		pG992p3Cap->maxDownPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDownPTM_TPS_TC[i]);
		pG992p3Cap->minRevDownPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minRevDownPTM_TPS_TC[i]);
		pG992p3Cap->maxDelayDownPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDelayDownPTM_TPS_TC[i]);

		pG992p3Cap->minUpPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minUpPTM_TPS_TC[i]);
		pG992p3Cap->maxUpPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxUpPTM_TPS_TC[i]);
		pG992p3Cap->minRevUpPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->minRevUpPTM_TPS_TC[i]);
		pG992p3Cap->maxDelayUpPTM_TPS_TC[i] = DiagEndianCl2Srv(pG992p3Cap->maxDelayUpPTM_TPS_TC[i]);
	}

	pG992p3Cap->subModePSDMasks = DiagEndianCl2Srv(pG992p3Cap->subModePSDMasks);
}

void DiagEndianStatusCl2Srv(void *st, unsigned int *pFlatStatus, int bufLen)
{
#ifdef XDSLDRV_ENABLE_PARSER
	int nItems;
#endif
	dslStatusStruct	*status = (dslStatusStruct	*) st;
	
	switch	(DSL_STATUS_CODE(status->code)) {
		case kDslConnectInfoStatus:
			switch (status->param.dslConnectInfo.code) {
				case kG992p2XmtCodingParamsInfo:
				case kG992p2RcvCodingParamsInfo:
					{
					G992CodingParams *codingParam, *codingParamDst;
					codingParam = (G992CodingParams*)status->param.dslConnectInfo.buffPtr;
#ifdef XDSLDRV_ENABLE_PARSER
					codingParamDst = (G992CodingParams *)parseBufTmp;
					*codingParamDst = *codingParam;
#else
					codingParamDst = codingParam;
#endif
					codingParamDst->K = DiagEndianCl2Srv (codingParam->K);
					codingParamDst->direction = DiagEndianCl2SrvLong(codingParam->direction);
#ifdef G992P1_NEWFRAME
					codingParamDst->N = DiagEndianCl2Srv (codingParam->N);
					codingParamDst->AS0BI = DiagEndianCl2Srv (codingParam->AS0BI);
#endif
					}
					break;
				case kG992p3XmtCodingParamsInfo:
				case kG992p3RcvCodingParamsInfo:
					{
					G992p3CodingParams	*p3, *p3Dst;
					p3 = (G992p3CodingParams*)status->param.dslConnectInfo.buffPtr;
#ifdef XDSLDRV_ENABLE_PARSER
					p3Dst = (G992p3CodingParams *)parseBufTmp;
					*p3Dst = *p3;
#else
					p3Dst = p3;
#endif
					p3Dst->MSGc= DiagEndianCl2Srv (p3->MSGc);
					p3Dst->L	= DiagEndianCl2SrvLong (p3->L);
					p3Dst->M	= DiagEndianCl2Srv (p3->M);
					p3Dst->T	= DiagEndianCl2Srv (p3->T);
					p3Dst->D	= DiagEndianCl2Srv (p3->D);
					p3Dst->R	= DiagEndianCl2Srv (p3->R);
					p3Dst->B	= DiagEndianCl2Srv (p3->B);
					}
					break;
				case kG992p2TrainingRcvCarrEdgeInfo:
#ifdef XDSLDRV_ENABLE_PARSER
					BlockLongMoveReverse(2, (int *)status->param.dslConnectInfo.buffPtr, (int *)parseBufTmp);
#else
					DiagEndianBlockLongCl2Srv((int *)status->param.dslConnectInfo.buffPtr, 2);
#endif
					break;
				case kG992ShowtimeMonitoringStatus:
#ifdef DSL_REPORT_ALL_COUNTERS
					{
					int		cnt;
#ifdef XDSLDRV_ENABLE_PARSER
					int *pCounters = (int *)parseBufTmp;
#else
					int *pCounters = (int*)status->param.dslConnectInfo.buffPtr;
#endif
					cnt = (bufLen >> 2) - 3;
					while (cnt < kG992ShowtimeNumOfMonitorCounters)
						pCounters[cnt++] = 0;
#ifdef XDSLDRV_ENABLE_PARSER
					pCounters = (int*)status->param.dslConnectInfo.buffPtr;
					BlockLongMoveReverse(kG992ShowtimeNumOfMonitorCounters, pCounters, (int *)parseBufTmp);
#else
					DiagEndianBlockLongCl2Srv(pCounters, kG992ShowtimeNumOfMonitorCounters);
#endif
					}
#else /* !DSL_REPORT_ALL_COUNTERS */
#ifdef XDSLDRV_ENABLE_PARSER
					BlockLongMoveReverse(kG992ShowtimeNumOfMonitorCounters, (int*)status->param.dslConnectInfo.buffPtr, (int *)parseBufTmp);
#else
					DiagEndianBlockLongCl2Srv((int*)status->param.dslConnectInfo.buffPtr, 12);
#endif
#endif
					break;
				case kDslTEQCoefInfo:
				case kDslRcvPsdInfo:
				case kDslRcvCarrierSNRInfo:
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = (printMaxItem < status->param.dslConnectInfo.value) ? printMaxItem: status->param.dslConnectInfo.value;
					
					BlockShortMoveReverse(nItems,(short*)status->param.dslConnectInfo.buffPtr,(short*)parseBufTmp);
#else
					DiagEndianBlockCl2Srv((short*)status->param.dslConnectInfo.buffPtr, status->param.dslConnectInfo.value);
#endif
					break;
			}
			break;

		case kDslShowtimeSNRMarginInfo:
		{
#ifndef XDSLDRV_ENABLE_PARSER
			int		i;
			ushort *dataPtr = (ushort *)(status->param.dslShowtimeSNRMarginInfo.buffPtr);
			status->param.dslShowtimeSNRMarginInfo.maxMarginCarrier = DiagEndianCl2SrvLong(status->param.dslShowtimeSNRMarginInfo.maxMarginCarrier);
			status->param.dslShowtimeSNRMarginInfo.maxSNRMargin = DiagEndianCl2SrvLong(status->param.dslShowtimeSNRMarginInfo.maxSNRMargin);
			status->param.dslShowtimeSNRMarginInfo.minMarginCarrier = DiagEndianCl2SrvLong(status->param.dslShowtimeSNRMarginInfo.minMarginCarrier);
			status->param.dslShowtimeSNRMarginInfo.minSNRMargin = DiagEndianCl2SrvLong(status->param.dslShowtimeSNRMarginInfo.minSNRMargin);
			status->param.dslShowtimeSNRMarginInfo.avgSNRMargin = DiagEndianCl2SrvLong(status->param.dslShowtimeSNRMarginInfo.avgSNRMargin);
			status->param.dslShowtimeSNRMarginInfo.nCarriers = DiagEndianCl2SrvLong(status->param.dslShowtimeSNRMarginInfo.nCarriers);
#endif
#ifdef XDSLDRV_ENABLE_PARSER
			ushort *dataPtr = (ushort *)(status->param.dslShowtimeSNRMarginInfo.buffPtr);
			nItems = (printMaxItem < status->param.dslShowtimeSNRMarginInfo.nCarriers) ? printMaxItem: status->param.dslShowtimeSNRMarginInfo.nCarriers;
			if(0 != nItems)
				BlockShortMoveReverse(nItems, dataPtr,(short*)parseBufTmp);
#else
			for (i = 0; i < status->param.dslShowtimeSNRMarginInfo.nCarriers; i++)
				dataPtr[i] = DiagEndianCl2Srv(dataPtr[i]);
#endif
		}
			break;

#ifdef G997_1
		case kDslReceivedEocCommand:
		{
			int	msgLen;

#ifndef XDSLDRV_ENABLE_PARSER
			if ((status->param.value >= kDslClearEocFirstCmd) &&
				!(status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask))
				status->param.dslClearEocMsg.dataPtr = (void *) DiagEndianCl2SrvLong((uint)status->param.dslClearEocMsg.dataPtr);
#endif
			msgLen = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
			if ((status->param.value >= kDslGeneralMsgStart) && (status->param.value != kDslGeneralMsgDbgFileName)) {
				if(kDsl993p2TestHlin == status->param.value)
					msgLen = status->param.dslClearEocMsg.msgType & 0x1FFFF;
				else if (kDslDbgDataSize16 == (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask)) {
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 1;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					BlockShortMoveReverse(nItems, (short*)status->param.dslClearEocMsg.dataPtr,(short*)parseBufTmp);
#else
					DiagEndianBlockCl2Srv((void *) status->param.dslClearEocMsg.dataPtr, msgLen >> 1);
#endif
				}
				else if (kDslDbgDataSize32 == (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask)) {
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 2;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					BlockLongMoveReverse(nItems, (int *)status->param.dslClearEocMsg.dataPtr,(int *)parseBufTmp);
#else
					DiagEndianBlockLongCl2Srv((void *) status->param.dslClearEocMsg.dataPtr, msgLen >> 2);
#endif
				}
				else if (kDslDbgDataSize64 == (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask)) {
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 3;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					DiagEndianBlockInt64Cl2Srv((ulonglong *)status->param.dslClearEocMsg.dataPtr,(int*)parseBufTmp, nItems);
#else
					DiagEndianBlockInt64Cl2Srv((void *) status->param.dslClearEocMsg.dataPtr, msgLen >> 3);
#endif
				}
#ifdef XDSLDRV_ENABLE_PARSER
				else {
					nItems = (msgLen < printMaxItem)? msgLen: printMaxItem;
					memcpy(parseBufTmp,status->param.dslClearEocMsg.dataPtr,nItems);
				}
#endif
			}

			switch (status->param.value) {
				case kDslVectoringErrorSamples:
#ifdef XDSLDRV_ENABLE_PARSER
					BlockShortMoveReverse(3, (short*)status->param.dslClearEocMsg.dataPtr,(short*)parseBufTmp);
#else
					DiagEndianBlockCl2Srv((void *) status->param.dslClearEocMsg.dataPtr, 3); /* lineId, syncCounter, nERBbytes */
#endif
					break;
				case kDslGeneralMsgDbgPrintf:
				case kDslGeneralMsgE14Print:
				case kDslGeneralMsgE14Print1:
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 2;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					BlockLongMoveReverse(nItems, (int *)status->param.dslClearEocMsg.dataPtr,(int *)parseBufTmp);
#else
					DiagEndianBlockLongCl2Srv(
						(void *) status->param.dslClearEocMsg.dataPtr, 
						msgLen >> 2);
#endif
				break;
				case kDslStrPrintf:
				case kDslStrDBPrintf:
					{
					uint *pData = (uint *) status->param.dslClearEocMsg.dataPtr;
					uint argNum = *pData;
					argNum = DiagEndianCl2SrvLong(argNum);
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = (argNum < printMaxItem)? argNum: printMaxItem;
					*(uint *)parseBufTmp = argNum;
					BlockLongMoveReverse(nItems, (int *)pData+1,(int *)parseBufTmp+1);
					strcpy((char *)((int *)parseBufTmp+1+argNum), (char *)(pData+1+argNum));
#else
					*pData = argNum;
					DiagEndianBlockLongCl2Srv((int*)pData+1, argNum);
#endif
					}
					break;
				case kDslGeneralMsgDbgPrintG992p3Cap:
#ifdef XDSLDRV_ENABLE_PARSER
					*(g992p3PhyDataPumpCapabilities *)parseBufTmp = *(g992p3PhyDataPumpCapabilities *)status->param.dslClearEocMsg.dataPtr;
					DiagEndianG992p3CapCl2Srv(
						(g992p3PhyDataPumpCapabilities *)parseBufTmp,
						status->param.dslClearEocMsg.msgType);
#else
					DiagEndianG992p3CapCl2Srv(
						(void *) status->param.dslClearEocMsg.dataPtr,
						status->param.dslClearEocMsg.msgType);
#endif
				break;
				case kDsl993p2BandPlanDsDump:
				case kDsl993p2BandPlanUsDump:
				case kDsl993p2PsdDump:
				case kDsl993p2MrefPSDds:
				case kDsl993p2MrefPSDus:
				case kDsl993p2LimitMask:
				case kDsl993p2VnPSDds:
				case kDsl993p2VnPSDus:
				case kDsl993p2US0mask:
				{
					short *pBuf =(short *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 1;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					*(short *)parseBufTmp = *pBuf;
					BlockShortMoveReverse(nItems, pBuf+1,(short *)parseBufTmp+1);
#else
					/* skip 1st word */
					pBuf++;
					DiagEndianBlockCl2Srv(pBuf,msgLen>>1);
#endif
				}
				break;
				case kDsl993p2QlnRaw:
				case kDsl993p2QlnRawRnc:
				case kDsl993p2HlogRaw:
				case kDsl993p2SnrRaw:
				case kDsl993p2LnAttnRaw:
				case kDsl993p2SATNpbRaw:
				case kDsl993p2NeGi:
				case kDsl993p2NeGiPhy:
				case kDsl993p2UsGi:
				case kDsl993p2BitSwapTones:
				case kDsl993p2FeGi:
				case kDsl993p2NeTi:
				case kDsl993p2FeTi:
				case kDsl993p2DSkl0perBand:
				case kDsl993p2USkl0perBand:
				case kDsl993p2AlnRaw:
				{
					short *pBuf =(short *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 1;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					BlockShortMoveReverse(nItems, pBuf,(short *)parseBufTmp);
#else
					DiagEndianBlockCl2Srv(pBuf,msgLen>>1);
#endif
				}
				break;
				case kDslNtrCounters:
				case kDsl993p2TestHlin:
				case kDslExcpType:
				case kDslExcpRegs:
				case kDslExcpArgs:
				case kDslExcpStack:
				case kGinpMonitoringCounters:
				case kDslTodTimeStamp:
				{
					int *pBuf =(int *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 2;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					BlockLongMoveReverse(nItems, pBuf,(int *)parseBufTmp);
#else
					DiagEndianBlockLongCl2Srv(pBuf,msgLen >> 2);
#endif
					break;
				}
				case kDsl993p2FramerDeframerUs:
				case kDsl993p2FramerDeframerDs:
				case kDsl993p2FramerAdslDs:
				case kDsl993p2FramerAdslUs:
				{
					FramerDeframerOptions *pFramerParam = (FramerDeframerOptions *) status->param.dslClearEocMsg.dataPtr;
#ifdef XDSLDRV_ENABLE_PARSER
					*(FramerDeframerOptions *)parseBufTmp = *pFramerParam;
					pFramerParam = (FramerDeframerOptions *)parseBufTmp;
#endif
					/* 1st 7 fields are short, the rest are char so just convert the 1st 7 */
					DiagEndianBlockCl2Srv((short *)pFramerParam,7);
					/* a uint, ETR_kbps was added */
					if(msgLen >= GINP_FRAMER_ETR_STRUCT_SIZE)
						pFramerParam->ETR_kbps = DiagEndianCl2SrvLong(pFramerParam->ETR_kbps);
					/* a ushort, inpShine was added to allow reporting of INP > 127 */
					if(msgLen >= GINP_FRAMER_INPSHINE_STRUCT_SIZE)
						pFramerParam->INPshine = DiagEndianCl2Srv(pFramerParam->INPshine);
					/* 32bits L */
					if(msgLen >= L32_FRAMER_STRUCT_SIZE)
						pFramerParam->L = DiagEndianCl2SrvLong(pFramerParam->L);
					else
						pFramerParam->L = pFramerParam->L16;
					/* GFAST: maxMemory, ndr, Ldr, Nret */
					if(msgLen >= ETR_MIN_EOC_FRAMER_STRUCT_SIZE) {
						pFramerParam->maxMemory = DiagEndianCl2SrvLong(pFramerParam->maxMemory);
						pFramerParam->ndr = DiagEndianCl2SrvLong(pFramerParam->ndr);
						pFramerParam->Ldr = DiagEndianCl2Srv(pFramerParam->Ldr);
					}
					if(msgLen >= RX_QUEUE16_FRAMER_STRUCT_SIZE) {
						pFramerParam->fireRxQueue = DiagEndianCl2Srv(pFramerParam->fireRxQueue);
					}
				}
					break;
				case kDsl993p2MaxRate:
				{
					int *pBuf =(int *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					BlockLongMoveReverse(2, pBuf,(int *)parseBufTmp);
#else
					DiagEndianBlockLongCl2Srv(pBuf,2);
#endif
				}
					break;
				case kDsl993p2PowerNeTxTot:
				case kDsl993p2PowerFeTxTot:
				case kDsl993p2SNRM:
				{
					short *pBuf =(short *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					*(short *)parseBufTmp = DiagEndianCl2Srv(*pBuf);
#else
					DiagEndianBlockCl2Srv(pBuf, 1);
#endif
				}
					break;
				case kDsl993p2FeTxPwrLD:
				case kDsl993p2NeTxPwrLD:
				case kDsl993p2FePbLatnLD:
				case kDsl993p2FePbSatnLD:
				case kDsl993p2FePbSnrLD:
				case kDsl993p2NePbLatnLD:
				case kDsl993p2NePbSatnLD:
				case kDsl993p2NePbSnrLD:
				case kDsl993p2FeHlinLD:
				case kDsl993p2NeHlinLD:
				case kDsl993p2FeHlogLD:
				case kDsl993p2NeHlogLD:
				case kDsl993p2PowerNeTxPb:
				case kDsl993p2PowerFeTxPb:
				case kDsl993p2SNRMpb:
				case kDsl993p2SnrROC:
				{
					short *pBuf =(short *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					nItems = msgLen >> 1;
					if(nItems > printMaxItem)
						nItems = printMaxItem;
					BlockShortMoveReverse(nItems, pBuf,(short *)parseBufTmp);
#else
					DiagEndianBlockCl2Srv(pBuf, msgLen >> 1);
#endif
				}
					break;
				case kDsl993p2FeAttnLD:
				case kDsl993p2NeAttnLD:
				case kDsl993p2dsATTNDR:
				case kDsl993p2BpType:
				{
					int *pBuf =(int *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					*(int *)parseBufTmp = DiagEndianCl2SrvLong(*pBuf);
#else
					DiagEndianBlockLongCl2Srv(pBuf,1);
#endif
				}
					break;
				case kDslSNRModeUs:
				case kDslSNRModeDs:
				case kDslActualCE:
				case kDslQLNmtDs:
				case kDslQLNmtUs:
				case kDslSNRmtDs:
				case kDslSNRmtUs:
				case kDslHLOGmtDs:
				case kDslHLOGmtUs:
				case kDslActualPSDDs:
				case kDslActualPSDUs:
					if (0 == (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask)) {
#ifdef XDSLDRV_ENABLE_PARSER
						nItems = msgLen >> 2;
						if(nItems > printMaxItem)
							nItems = printMaxItem;
						BlockLongMoveReverse(nItems, (int *)status->param.dslClearEocMsg.dataPtr,(int *)parseBufTmp);
#else
						DiagEndianBlockLongCl2Srv((void *) status->param.dslClearEocMsg.dataPtr, msgLen >> 2);
#endif
					}
					break;
				case kDslUPBOkle:
					if (0 == (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask)) {
#ifdef XDSLDRV_ENABLE_PARSER
						nItems = msgLen >> 1;
						if(nItems > printMaxItem)
							nItems = printMaxItem;
						BlockShortMoveReverse(nItems, (short *)status->param.dslClearEocMsg.dataPtr,(short *)parseBufTmp);
#else
						DiagEndianBlockCl2Srv((void *) status->param.dslClearEocMsg.dataPtr, msgLen >> 1);
#endif
					}
					break;
				case kDslBondDiscExchange:
				case kDslBondDiscExchangeDrv:
				{
					bonDiscExchangeStruct *pBDExch = (bonDiscExchangeStruct *) status->param.dslClearEocMsg.dataPtr;
#ifdef XDSLDRV_ENABLE_PARSER
					BlockShortMoveReverse(3, (short *)pBDExch,(short *)parseBufTmp);
					BlockLongMoveReverse(3, (int *)((short *)pBDExch+3), (int *)((short *)parseBufTmp+3));
#else
					pBDExch->bdCmd = DiagEndianCl2Srv(pBDExch->bdCmd);
					pBDExch->bdId  = DiagEndianCl2Srv(pBDExch->bdId);
					pBDExch->bondDisc.pmeRemoteDiscoveryHigh = DiagEndianCl2Srv(pBDExch->bondDisc.pmeRemoteDiscoveryHigh);
					DiagEndianBlockLongCl2Srv((void *) &pBDExch->bondDisc.pmeRemoteDiscoveryLow, 3);
#endif
				}
					break;
				case kDslAfeInfoCmd:
				{
					afeDescStruct *pAfeInfo = (afeDescStruct *) status->param.dslClearEocMsg.dataPtr;
#ifdef XDSLDRV_ENABLE_PARSER
					afeDescStruct *pAfeInfo1 = (afeDescStruct *)parseBufTmp;
#else
					afeDescStruct *pAfeInfo1 = pAfeInfo;
#endif
					pAfeInfo1->verId = DiagEndianCl2Srv(pAfeInfo->verId);
					pAfeInfo1->size  = DiagEndianCl2Srv(pAfeInfo->size);
					pAfeInfo1->chipId= DiagEndianCl2SrvLong(pAfeInfo->chipId);
					pAfeInfo1->boardAfeId = DiagEndianCl2SrvLong(pAfeInfo->boardAfeId);
					pAfeInfo1->afeChidIdConfig0 = DiagEndianCl2SrvLong(pAfeInfo->afeChidIdConfig0);
					pAfeInfo1->afeChidIdConfig1 = DiagEndianCl2SrvLong(pAfeInfo->afeChidIdConfig1);
				}
				break;
				case kDslPhyInfoCmd:
				{
					int *pBuf =(int *)(status->param.dslClearEocMsg.dataPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					*(adslPhyInfo *)parseBufTmp = *(adslPhyInfo *)pBuf;
					pBuf = (int *)parseBufTmp;
#endif
					DiagEndianBlockLongCl2Srv(pBuf, 4); pBuf += 4;
					DiagEndianBlockCl2Srv((short*)pBuf, 4); pBuf += 3;	/* fwType - mnVerNum */
					DiagEndianBlockLongCl2Srv(pBuf, 4);	/* features */
				}
					break;
			}
			break;
		}
#endif /* G997_1 */
		case kAtmStatus:
			switch (status->param.atmStatus.code) {
#ifdef XDSLDRV_ENABLE_PARSER
				case kAtmStatCounters:
				{
					int *pSrc = (int *)(ADSL_ADDR_TO_HOST(status->param.atmStatus.param.value));
					BlockShortMoveReverse(2, (short *)pSrc, (short *)parseBufTmp);  /* id and bertStatus(first 2 ushort) */
					BlockLongMoveReverse((sizeof(atmPhyCounters)>>2)-1, pSrc+1, (int *)parseBufTmp+1);
					break;
				}
#else
				case kAtmStatCounters1:
					{
					atmPhyCounters *pAtmCnt = (void *) (pFlatStatus + 3);

					status->param.atmStatus.param.value = (uint) pAtmCnt;
					pAtmCnt->id = DiagEndianCl2Srv(pAtmCnt->id);
					pAtmCnt->bertStatus = DiagEndianCl2Srv(pAtmCnt->bertStatus);
					if (pAtmCnt->id >= 4)
					  DiagEndianBlockLongCl2Srv((int*)pAtmCnt+1, (pAtmCnt->id >> 2) - 1);
					if (sizeof(atmPhyCounters) > pAtmCnt->id)
						memset(((char*) pAtmCnt) + pAtmCnt->id, 0, sizeof(atmPhyCounters) - pAtmCnt->id);
					}
					break;
#endif
				default:
					break;
			}
			break;
		case kDslExceptionStatus:
		{
			int		len, *sp;
#ifdef CONFIG_ARM64
			sp = pStackPtr;
#else
			sp = (int *) status->param.dslException.sp;
#endif
			DiagEndianBlockLongCl2Srv((void *)sp, 31);
			DiagEndianBlockLongCl2Srv((void *)status->param.dslException.argv, status->param.dslException.argc);

			len = sizeof(int) * (1 + 31 + 1 + status->param.dslException.argc);
			if (bufLen > len) {
				status->param.dslException.stackLen = (bufLen - len) >> 2;
				status->param.dslException.stackPtr = (void *) (((char*)pFlatStatus) + len);
				DiagEndianBlockLongCl2Srv((void *)status->param.dslException.stackPtr, status->param.dslException.stackLen);
			}
			else {
				status->param.dslException.stackLen = 0;
				status->param.dslException.stackPtr = NULL;
			}
		}
			break;
#ifndef XDSLDRV_ENABLE_PARSER
		case kDslOLRRequestStatus:
			{
			int		i;

			for (i = 0; i < 4; i++)
				status->param.dslOLRRequest.L[i] = DiagEndianCl2Srv(status->param.dslOLRRequest.L[i]);
			}
			break;
#endif
		default:
			break;
	}
}

void DiagUnflattenStatus1(int *pFlatStatus, int statusLen, void *pStat, void *pOutBuf, BOOL diagClientLE)
{
#ifdef XDSLDRV_ENABLE_PARSER
	int	nItems;
#endif
	unsigned int		code;
	dslStatusCode		statusCode;
	dslStatusStruct	*pStatus = pStat;
	
#ifndef XDSLDRV_ENABLE_PARSER
	UnflattenStatus ((uint *)pFlatStatus, pStatus);
	statusCode = DSL_STATUS_CODE(pStatus->code);
	
	if (!diagClientLE) {
		/* Work around for old driver */
		if ((kDslReceivedEocCommand == statusCode) &&
			(kDslGeneralMsgStart <= pStatus->param.value) &&
			!(kDslClearEocMsgDataVolatileMask & pStatus->param.dslClearEocMsg.msgType) ) {
			if(NULL != pOutBuf)
			sprintf((char *)pOutBuf, "kDslReceivedEocCommand:  msgId=%ld msgLen=%ld\n\tMessage data is missing, probably due to old driver\n",
				pStatus->param.value, pStatus->param.dslClearEocMsg.msgType&kDslClearEocMsgLengthMask);
			return;
		}

		DiagEndianStatusCl2Srv(pStatus, (uint *)pFlatStatus, statusLen);
	}
#else
	/* UnflattenStatus() was done by the driver */
	statusCode = DSL_STATUS_CODE(pStatus->code);
	if (!diagClientLE)
		DiagEndianStatusCl2Srv(pStatus, (uint *)pFlatStatus, statusLen);
#endif
	if (kDslConnectInfoStatus == statusCode) {
		code = pStatus->param.dslConnectInfo.code;
		if ((kDslChannelResponseLog == code)	||
			(kDslChannelResponseLinear == code)	||
			(kDslChannelQuietLineNoise == code)) {
#ifndef XDSLDRV_ENABLE_PARSER
			pStatus->param.dslConnectInfo.buffPtr = ((char *) pFlatStatus) +
			sizeof(pStatus->code) + sizeof(pStatus->param.dslConnectInfo);
#endif
			if (kDslChannelQuietLineNoise != code) {
#ifdef XDSLDRV_ENABLE_PARSER
				nItems = (printMaxItem < pStatus->param.dslConnectInfo.value) ? printMaxItem: pStatus->param.dslConnectInfo.value;
				BlockShortMoveReverse(nItems,(short*)pStatus->param.dslConnectInfo.buffPtr,(short*)parseBufTmp);
#else
				DiagEndianBlockCl2Srv((short*)pStatus->param.dslConnectInfo.buffPtr, pStatus->param.dslConnectInfo.value);
#endif
			}
		}
	}
	else
	if (kDslDspControlStatus == statusCode) {
		code = pStatus->param.dslConnectInfo.code;
		if ((kDslPLNPeakNoiseTablePtr == code)			||
			(kDslPerBinThldViolationTablePtr == code)	||
			(kDslImpulseNoiseDurationTablePtr == code)	||
			(kDslImpulseNoiseTimeTablePtr == code)	||
			(kDslInpBinTablePtr == code)				||
			(kDslItaBinTablePtr ==code)				||
			(kDslNLNoise == code)					||
			(kDslInitializationSNRMarginInfo == code)	||
			(kDslG992RcvShowtimeUpdateGainPtr == code) ) {
#ifndef XDSLDRV_ENABLE_PARSER
			pStatus->param.dslConnectInfo.buffPtr = ((char *) pFlatStatus) + 
				sizeof(pStatus->code) + sizeof(pStatus->param.dslConnectInfo);
#endif
#ifdef XDSLDRV_ENABLE_PARSER
			nItems = (printMaxItem < pStatus->param.dslConnectInfo.value) ? printMaxItem: pStatus->param.dslConnectInfo.value;
			BlockShortMoveReverse(nItems,(short*)pStatus->param.dslConnectInfo.buffPtr,(short*)parseBufTmp);
#else
			DiagEndianBlockCl2Srv((short*)pStatus->param.dslConnectInfo.buffPtr, pStatus->param.dslConnectInfo.value);
#endif
		}
#ifndef XDSLDRV_ENABLE_PARSER
		else if( (kDslTxOvhMsg == code) || (kDslRxOvhMsg == code) ) {
			pStatus->param.dslConnectInfo.buffPtr = ((char *) pFlatStatus) + 
				sizeof(pStatus->code) + sizeof(pStatus->param.dslConnectInfo);
		}
#endif
		else if( kFireMonitoringCounters == code) {
#ifndef XDSLDRV_ENABLE_PARSER
			pStatus->param.dslConnectInfo.buffPtr = ((char *) pFlatStatus) + 
				sizeof(pStatus->code) + sizeof(pStatus->param.dslConnectInfo);
#endif
#ifdef XDSLDRV_ENABLE_PARSER
			nItems = (printMaxItem < pStatus->param.dslConnectInfo.value) ? printMaxItem: pStatus->param.dslConnectInfo.value;
			BlockLongMoveReverse(nItems,(int*)pStatus->param.dslConnectInfo.buffPtr,(int*)parseBufTmp);
#else
			DiagEndianBlockLongCl2Srv((int*)pStatus->param.dslConnectInfo.buffPtr, pStatus->param.dslConnectInfo.value);
#endif
		}
	}
}

#ifndef XDSLDRV_ENABLE_PARSER
int StrAddLF(char *dstPtr, int len)
{
	if ((len > 0) && (dstPtr[len-1] != '\n')) {
		dstPtr[len]   = '\n';
		dstPtr[len+1] = 0;
		len++;
	}
	return len;
}

Boolean DiagParserIsVerInfoStatus(const char *buf, char **pVer)
{
	Boolean res = FALSE;
	if((0 == memcmp(VERSION_INFO_STR, buf, sizeof(VERSION_INFO_STR) -1)) ||
		(0 == memcmp(PHY_UPLOAD_VERSION_STR, buf, sizeof(PHY_UPLOAD_VERSION_STR) -1))) {
		res = TRUE;
		*pVer = (0 == memcmp(VERSION_INFO_STR, buf, sizeof(VERSION_INFO_STR) -1)) ?
			((char*)buf + sizeof(VERSION_INFO_STR) +1 + sizeof(VERSION_PREFIX) -1):
			((char*)buf + sizeof(PHY_UPLOAD_VERSION_STR));
	}
	
	return res;
}

char *DiagsParsePhyStdCaps(char *pVer, phyVersionInfoStruct *pPhyVerInfo)
{
	if ('2' == pVer[0]) {
		pPhyVerInfo->bAdsl = TRUE;
		if ('p' == pVer[1]) {
			pVer++;
		}
		pVer++;
	}
	
	if ('v' == pVer[0]) {
		pPhyVerInfo->bVdsl = TRUE;
		if ( (pVer[1] >= '0') && (pVer[1] <= '9') )	/* 3/4/6 band */
			pVer ++;
		pVer++;
		if('d' == pVer[0])	/* temporary target/version for 63138/63381 */
			pVer++;
	}
	
	if ('f' == pVer[0]) {
		pPhyVerInfo->bGfast = TRUE;
		pVer++;
		if('n' == pVer[0])	/* Non standard version */
			pVer++;
		if('o' == pVer[0])	/* CO version */
			pVer++;
	}
	
	if('b' == pVer[0]) {
		pPhyVerInfo->bBonding = TRUE;
		pVer++;	/* Bonding Phy */
		if('d' == pVer[0])	/* decomposition target such as 63138 PHY, A2pvbdH0xxx */
			pVer++;
	}
	
	return pVer;
}

int DslDiagProcessStatusString(char *srcPtr, int srcLen, char *dstPtr, int dstLen)
{
#if defined(LITE_VERSION) || defined(PROXY_VERSION)
	do {
		if (0 == memcmp(DIAG_VERSION_STR, srcPtr, sizeof(DIAG_VERSION_STR)-1))
			break;
		if (0 == memcmp(DIAG_PHYCMD_STR, srcPtr, sizeof(DIAG_PHYCMD_STR)-1))
			break;
		if (0 == memcmp(DIAG_RXGAIN_STR, srcPtr, sizeof(DIAG_RXGAIN_STR)-1))
			break;
		if (0 == memcmp(DIAG_SENDING_STR, srcPtr, sizeof(DIAG_SENDING_STR)-1))
			break;
		if (0 == memcmp(DIAG_TONE_SEL_STR, srcPtr, sizeof(DIAG_SENDING_STR)-1))
			break;
		if (0 == memcmp(DIAG_AFEID_STR, srcPtr, sizeof(DIAG_AFEID_STR)-1))
			break;
		if (0 == memcmp(DIAG_PHY_DBG_STR, srcPtr, sizeof(DIAG_PHY_DBG_STR)-1))
			break;

		srcLen = 0;
	} while (0);
#endif

	if (srcLen != 0)
		strcpy(dstPtr, srcPtr);
	return srcLen;
}
#endif /* !XDSLDRV_ENABLE_PARSER */
