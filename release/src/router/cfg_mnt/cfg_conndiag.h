#ifndef __CFG_CONNDIAG_H__
#define __CFG_CONNDIAG_H__

#define CONNDIAG_MIX_MODE CFG_CONNDIAG_MIX_MODE  // same with DIAGMODE_MIX in rc/conn_diag.h
#define CONNDIAG_PREFIX_PORTSTATUS CFG_CONNDIAG_PREFIX_PORTSTATUS
#define CONNDIAG_PORTSTATUS CFG_CONNDIAG_PORTSTATUS
#define CONNDIAG_PORTSTATUS_JSON_PATH CFG_CONNDIAG_PORTSTATUS_PATH
#define CONNDIAG_RSPSTATUS_SUCESS 0
#define CONNDIAG_RSPSTATUS_FAILED 1

extern void cm_processConnDiagPkt(TLV_Header tlv, unsigned char *data, char *peerIp);
extern int cm_connDiagPacketProcess(unsigned char *data);
extern void cm_addConnDiagPktToList(void *data);
extern void *cm_connDiagPktListHandler(void *args);
extern void cm_terminateConnDiagPktList();
extern int cm_reportPortstatusData(void);
extern int cm_connDiagCheckRspStatus(unsigned char *decryptedMsg);

#endif /* __CFG_CONNDIAG_H__ */
/* End of cfg_conndiag.h */
