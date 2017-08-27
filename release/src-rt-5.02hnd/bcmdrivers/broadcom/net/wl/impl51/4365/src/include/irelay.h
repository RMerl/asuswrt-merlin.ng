/*
 * Windows Broadcom relay device driver interface.
 *
 * Copyright(c) 2001 Broadcom Corporation
 * $Id: irelay.h 241182 2011-02-17 21:50:03Z $
 */

/* Win9x used FILE_READ_ACCESS instead of FILE_READ_DATA */
#ifndef FILE_READ_DATA
#define FILE_READ_DATA  FILE_READ_ACCESS
#define FILE_WRITE_DATA  FILE_WRITE_ACCESS
# endif

#ifndef IOCTL_NDIS_QUERY_GLOBAL_STATS
#    define _NDIS_CONTROL_CODE(request, method) \
		CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, request, method, FILE_ANY_ACCESS)

#    define IOCTL_NDIS_QUERY_GLOBAL_STATS   _NDIS_CONTROL_CODE(0, METHOD_OUT_DIRECT)
#endif

#define IOCTL_OID_RELAY		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, \
					 FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PKT_TX_RELAY	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, \
					 FILE_WRITE_DATA)
#define IOCTL_PKT_RX_RELAY	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, \
					 FILE_WRITE_DATA | FILE_READ_DATA)
#define IOCTL_LIST		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, \
					 FILE_WRITE_DATA | FILE_READ_DATA)
#define IOCTL_XLIST		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, \
					 FILE_WRITE_DATA | FILE_READ_DATA)
#define IOCTL_VERSION		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, \
					 FILE_WRITE_DATA | FILE_READ_DATA)
#define IOCTL_BIND		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, \
					 FILE_WRITE_DATA | FILE_READ_DATA)
#define IOCTL_UNBIND		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, \
					 FILE_WRITE_DATA | FILE_READ_DATA)
#define IOCTL_IR_DUMP		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED, \
					 FILE_WRITE_DATA | FILE_READ_DATA)

#pragma pack(push, 1)

/* structure definitions for sending packets via IOCTL_PKT_TX_RELAY */

typedef struct {
	ULONG	InstanceID;
	CHAR	Buffer[];
} RxRequest, *PRxRequest;

/* structure definitions for sending packets via IOCTL_PKT_TX_RELAY */

typedef struct {
	ULONG	InstanceID;
	ULONG	Copies;
	CHAR	Buffer[];
} TxRequest, *PTxRequest;

/* structure definitions for relaying OIDs through via IOCTL_OID_RELAY */

typedef struct {
	ULONG	OID;
	ULONG	IsQuery;
	ULONG	BufferLength;
	ULONG	Status;
} RelayHeader, *PRelayHeader;

typedef struct {
	RelayHeader rh;
	UCHAR Buffer[];
} RelayQuery, *PRelayQuery;

typedef struct {
	RelayHeader rh;
	ULONG Cookie;
	UCHAR Buffer[];
} RelaySet, *PRelaySet;

/* structure definitions for sending packets via IOCTL_BIND */

typedef struct {
    CHAR  name[80];
} BindRequest, *PBindRequest;


/* Structure for passing generic OIDs through irelay.  This is the
 * same as a normal set but it does not have a cookie.
 */
typedef RelayQuery RelayGenSet, *PRelayGenSet;

/* structure definitions for relaying OIDs through via IOCTL_VERSION */

typedef struct {
    ULONG   VersionMS;        /* e.g. 0x00030075 = "3.75" */
    ULONG   VersionLS;        /* e.g. 0x00000031 = "0.31" */
} VersionResponse, *PVersionResponse;

/* Combined generic/custom query/set relay oid structure. */

typedef struct _IRelay {
	RelayHeader rh;
	union {
		struct {
			ULONG Cookie;
			UCHAR Buffer[];
		} EpiOid;
		struct {
			UCHAR Buffer[];
		} GenOid;
		UCHAR Buffer[];
	};
} IRELAY, *PIRELAY;

#pragma pack(pop)
