#ifndef __CFG_CONNDIAG_H__
#define __CFG_CONNDIAG_H__

extern void cm_processConnDiagPkt(TLV_Header tlv, unsigned char *data, char *peerIp);
extern int cm_connDiagPacketProcess(unsigned char *data);

#endif /* __CFG_CONNDIAG_H__ */
/* End of cfg_conndiag.h */
