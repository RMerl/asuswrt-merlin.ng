
#ifndef HMI_DEF_H
#define HMI_DEF_H

#define MAX_HMI_PACKET_SIZE	512
#define HMI_SERVER_PORT		57660

typedef struct
{
    unsigned int	remoteIpAddr;
    unsigned short	remotePort;
    unsigned short	hmiMsgLen;
    unsigned char	*pHmiMsg;
} ADSLDRV_HMI, *PADSLDRV_HMI;

#endif
