/*
 * Copyright (c) 2008 CACE Technologies, Davis (California)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CACE Technologies nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // endif

#include <pcap.h>
#include <pcap-int.h>

#include "pcap-tc.h"

#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#include <tchar.h>
#endif // endif

typedef TC_STATUS	(TC_CALLCONV *TcFcnQueryPortList)			(PTC_PORT *ppPorts, PULONG pLength);
typedef TC_STATUS	(TC_CALLCONV *TcFcnFreePortList)			(TC_PORT *pPorts);

typedef PCHAR		(TC_CALLCONV *TcFcnStatusGetString)			(TC_STATUS status);

typedef PCHAR		(TC_CALLCONV *TcFcnPortGetName)				(TC_PORT port);
typedef PCHAR		(TC_CALLCONV *TcFcnPortGetDescription)		(TC_PORT port);

typedef TC_STATUS	(TC_CALLCONV *TcFcnInstanceOpenByName)		(PCHAR name, PTC_INSTANCE pInstance);
typedef TC_STATUS	(TC_CALLCONV *TcFcnInstanceClose)			(TC_INSTANCE instance);
typedef TC_STATUS	(TC_CALLCONV *TcFcnInstanceSetFeature)		(TC_INSTANCE instance, ULONG feature, ULONG value);
typedef TC_STATUS	(TC_CALLCONV *TcFcnInstanceQueryFeature)	(TC_INSTANCE instance, ULONG feature, PULONG pValue);
typedef TC_STATUS	(TC_CALLCONV *TcFcnInstanceReceivePackets)	(TC_INSTANCE instance, PTC_PACKETS_BUFFER pBuffer);
typedef HANDLE		(TC_CALLCONV *TcFcnInstanceGetReceiveWaitHandle) (TC_INSTANCE instance);
typedef TC_STATUS	(TC_CALLCONV *TcFcnInstanceTransmitPackets)	(TC_INSTANCE instance, TC_PACKETS_BUFFER pBuffer);
typedef TC_STATUS	(TC_CALLCONV *TcFcnInstanceQueryStatistics)	(TC_INSTANCE instance, PTC_STATISTICS pStatistics);

typedef TC_STATUS	(TC_CALLCONV *TcFcnPacketsBufferCreate)		(ULONG size, PTC_PACKETS_BUFFER pBuffer);
typedef VOID		(TC_CALLCONV *TcFcnPacketsBufferDestroy)	(TC_PACKETS_BUFFER buffer);
typedef TC_STATUS	(TC_CALLCONV *TcFcnPacketsBufferQueryNextPacket)(TC_PACKETS_BUFFER buffer, PTC_PACKET_HEADER pHeader, PVOID *ppData);
typedef TC_STATUS	(TC_CALLCONV *TcFcnPacketsBufferCommitNextPacket)(TC_PACKETS_BUFFER buffer, PTC_PACKET_HEADER pHeader, PVOID pData);

typedef VOID		(TC_CALLCONV *TcFcnStatisticsDestroy)		(TC_STATISTICS statistics);
typedef TC_STATUS	(TC_CALLCONV *TcFcnStatisticsUpdate)		(TC_STATISTICS statistics);
typedef TC_STATUS	(TC_CALLCONV *TcFcnStatisticsQueryValue)	(TC_STATISTICS statistics, ULONG counterId, PULONGLONG pValue);

typedef enum LONG
{
	TC_API_UNLOADED = 0,
	TC_API_LOADED,
	TC_API_CANNOT_LOAD,
	TC_API_LOADING
}
	TC_API_LOAD_STATUS;

typedef struct _TC_FUNCTIONS
{
	TC_API_LOAD_STATUS			LoadStatus;
#ifdef WIN32
	HMODULE						hTcApiDllHandle;
#endif // endif
	TcFcnQueryPortList			QueryPortList;
	TcFcnFreePortList			FreePortList;
	TcFcnStatusGetString		StatusGetString;

	TcFcnPortGetName			PortGetName;
	TcFcnPortGetDescription		PortGetDescription;

	TcFcnInstanceOpenByName		InstanceOpenByName;
	TcFcnInstanceClose			InstanceClose;
	TcFcnInstanceSetFeature		InstanceSetFeature;
	TcFcnInstanceQueryFeature	InstanceQueryFeature;
	TcFcnInstanceReceivePackets	InstanceReceivePackets;
#ifdef WIN32
	TcFcnInstanceGetReceiveWaitHandle InstanceGetReceiveWaitHandle;
#endif // endif
	TcFcnInstanceTransmitPackets InstanceTransmitPackets;
	TcFcnInstanceQueryStatistics InstanceQueryStatistics;

	TcFcnPacketsBufferCreate	PacketsBufferCreate;
	TcFcnPacketsBufferDestroy	PacketsBufferDestroy;
	TcFcnPacketsBufferQueryNextPacket	PacketsBufferQueryNextPacket;
	TcFcnPacketsBufferCommitNextPacket  PacketsBufferCommitNextPacket;

	TcFcnStatisticsDestroy		StatisticsDestroy;
	TcFcnStatisticsUpdate		StatisticsUpdate;
	TcFcnStatisticsQueryValue	StatisticsQueryValue;
}
	TC_FUNCTIONS;

static pcap_if_t* TcCreatePcapIfFromPort(TC_PORT port);
static void TcCleanup(pcap_t *p);
static int TcInject(pcap_t *p, const void *buf, size_t size);
static int TcRead(pcap_t *p, int cnt, pcap_handler callback, u_char *user);
static int TcSetMinToCopy(pcap_t *p, int size);
static int TcSetMode(pcap_t *p, int mode);
static int TcSetBuff(pcap_t *p, int dim);
static int TcStats(pcap_t *p, struct pcap_stat *ps);
static int TcSetFilter(pcap_t *p, struct bpf_program *fp);
static int TcGetNonBlock(pcap_t *p, char *errbuf);
static int TcSetNonBlock(pcap_t *p, int nonblock, char *errbuf);
static int TcSetDatalink(pcap_t *p, int dlt);

#ifdef WIN32
TC_FUNCTIONS g_TcFunctions =
{
	TC_API_UNLOADED, /* LoadStatus */
	NULL,  /* hTcApiDllHandle */
	NULL,  /* QueryPortList */
	NULL,  /* FreePortList */
	NULL,  /* StatusGetString */
	NULL,  /* PortGetName */
	NULL,  /* PortGetDescription */
	NULL,  /* InstanceOpenByName */
	NULL,  /* InstanceClose */
	NULL,  /* InstanceSetFeature */
	NULL,  /* InstanceQueryFeature */
	NULL,  /* InstanceReceivePackets */
	NULL,  /* InstanceGetReceiveWaitHandle */
	NULL,  /* InstanceTransmitPackets */
	NULL,  /* InstanceQueryStatistics */
	NULL,  /* PacketsBufferCreate */
	NULL,  /* PacketsBufferDestroy */
	NULL,  /* PacketsBufferQueryNextPacket */
	NULL,  /* PacketsBufferCommitNextPacket */
	NULL,  /* StatisticsDestroy */
	NULL,  /* StatisticsUpdate */
	NULL  /* StatisticsQueryValue */
};
#else
TC_FUNCTIONS g_TcFunctions =
{
	TC_API_LOADED, /* LoadStatus */
	TcQueryPortList,
	TcFreePortList,
	TcStatusGetString,
	TcPortGetName,
	TcPortGetDescription,
	TcInstanceOpenByName,
	TcInstanceClose,
	TcInstanceSetFeature,
	TcInstanceQueryFeature,
	TcInstanceReceivePackets,
#ifdef WIN32
	TcInstanceGetReceiveWaitHandle,
#endif // endif
	TcInstanceTransmitPackets,
	TcInstanceQueryStatistics,
	TcPacketsBufferCreate,
	TcPacketsBufferDestroy,
	TcPacketsBufferQueryNextPacket,
	TcPacketsBufferCommitNextPacket,
	TcStatisticsDestroy,
	TcStatisticsUpdate,
	TcStatisticsQueryValue,
};
#endif // endif

#define MAX_TC_PACKET_SIZE	9500

#pragma pack(push, 1)

#define PPH_PH_FLAG_PADDING	((UCHAR)0x01)
#define PPH_PH_VERSION		((UCHAR)0x00)

typedef struct _PPI_PACKET_HEADER
{
	UCHAR	PphVersion;
	UCHAR	PphFlags;
	USHORT	PphLength;
	ULONG	PphDlt;
}
	PPI_PACKET_HEADER, *PPPI_PACKET_HEADER;

typedef struct _PPI_FIELD_HEADER
{
	USHORT PfhType;
	USHORT PfhLength;
}
	PPI_FIELD_HEADER, *PPPI_FIELD_HEADER;

#define		PPI_FIELD_TYPE_AGGREGATION_EXTENSION	((UCHAR)0x08)

typedef struct _PPI_FIELD_AGGREGATION_EXTENSION
{
	ULONG		InterfaceId;
}
	PPI_FIELD_AGGREGATION_EXTENSION, *PPPI_FIELD_AGGREGATION_EXTENSION;

#define		PPI_FIELD_TYPE_802_3_EXTENSION			((UCHAR)0x09)

#define PPI_FLD_802_3_EXT_FLAG_FCS_PRESENT			((ULONG)0x00000001)

typedef struct _PPI_FIELD_802_3_EXTENSION
{
	ULONG		Flags;
	ULONG		Errors;
}
	PPI_FIELD_802_3_EXTENSION, *PPPI_FIELD_802_3_EXTENSION;

typedef struct _PPI_HEADER
{
	PPI_PACKET_HEADER PacketHeader;
	PPI_FIELD_HEADER  AggregationFieldHeader;
	PPI_FIELD_AGGREGATION_EXTENSION AggregationField;
	PPI_FIELD_HEADER  Dot3FieldHeader;
	PPI_FIELD_802_3_EXTENSION Dot3Field;
}
	PPI_HEADER, *PPPI_HEADER;
#pragma pack(pop)

#ifdef WIN32
//
// This wrapper around loadlibrary appends the system folder (usually c:\windows\system32)
// to the relative path of the DLL, so that the DLL is always loaded from an absolute path
// (It's no longer possible to load airpcap.dll from the application folder).
// This solves the DLL Hijacking issue discovered in August 2010
// http://blog.metasploit.com/2010/08/exploiting-dll-hijacking-flaws.html
//
HMODULE LoadLibrarySafe(LPCTSTR lpFileName)
{
  TCHAR path[MAX_PATH];
  TCHAR fullFileName[MAX_PATH];
  UINT res;
  HMODULE hModule = NULL;
  do
  {
	res = GetSystemDirectory(path, MAX_PATH);

	if (res == 0)
	{
		//
		// some bad failure occurred;
		//
		break;
	}

	if (res > MAX_PATH)
	{
		//
		// the buffer was not big enough
		//
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		break;
	}

	if (res + 1 + _tcslen(lpFileName) + 1 < MAX_PATH)
	{
		memcpy(fullFileName, path, res * sizeof(TCHAR));
		fullFileName[res] = _T('\\');
		memcpy(&fullFileName[res + 1], lpFileName, (_tcslen(lpFileName) + 1) * sizeof(TCHAR));

		hModule = LoadLibrary(fullFileName);
	}
	else
	{
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
	}

  }while(FALSE);

  return hModule;
}

/*
 * NOTE: this function should be called by the pcap functions that can theoretically
 *       deal with the Tc library for the first time, namely listing the adapters and
 *       opening one. All the other ones (close, read, write, set parameters) work
 *       on an open instance of TC, so we do not care to call this function
 */
TC_API_LOAD_STATUS LoadTcFunctions()
{
	TC_API_LOAD_STATUS currentStatus;

	do
	{
		currentStatus = InterlockedCompareExchange((LONG*)&g_TcFunctions.LoadStatus, TC_API_LOADING, TC_API_UNLOADED);

		while(currentStatus == TC_API_LOADING)
		{
			currentStatus = InterlockedCompareExchange((LONG*)&g_TcFunctions.LoadStatus, TC_API_LOADING, TC_API_LOADING);
			Sleep(10);
		}

		/*
		 * at this point we are either in the LOADED state, unloaded state (i.e. we are the ones loading everything)
		 * or in cannot load
		 */
		if(currentStatus  == TC_API_LOADED)
		{
			return TC_API_LOADED;
		}

		if (currentStatus == TC_API_CANNOT_LOAD)
		{
			return TC_API_CANNOT_LOAD;
		}

		currentStatus = TC_API_CANNOT_LOAD;

		g_TcFunctions.hTcApiDllHandle = LoadLibrarySafe("TcApi.dll");
		if (g_TcFunctions.hTcApiDllHandle == NULL)	break;

		g_TcFunctions.QueryPortList					= (TcFcnQueryPortList)			GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcQueryPortList");
		g_TcFunctions.FreePortList					= (TcFcnFreePortList)			GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcFreePortList");

		g_TcFunctions.StatusGetString				= (TcFcnStatusGetString)		GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcStatusGetString");

		g_TcFunctions.PortGetName					= (TcFcnPortGetName)			GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcPortGetName");
		g_TcFunctions.PortGetDescription			= (TcFcnPortGetDescription)		GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcPortGetDescription");

		g_TcFunctions.InstanceOpenByName			= (TcFcnInstanceOpenByName)		GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceOpenByName");
		g_TcFunctions.InstanceClose					= (TcFcnInstanceClose)			GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceClose");
		g_TcFunctions.InstanceSetFeature			= (TcFcnInstanceSetFeature)		GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceSetFeature");
		g_TcFunctions.InstanceQueryFeature			= (TcFcnInstanceQueryFeature)	GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceQueryFeature");
		g_TcFunctions.InstanceReceivePackets		= (TcFcnInstanceReceivePackets)	GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceReceivePackets");
		g_TcFunctions.InstanceGetReceiveWaitHandle	= (TcFcnInstanceGetReceiveWaitHandle)GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceGetReceiveWaitHandle");
		g_TcFunctions.InstanceTransmitPackets		= (TcFcnInstanceTransmitPackets)GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceTransmitPackets");
		g_TcFunctions.InstanceQueryStatistics		= (TcFcnInstanceQueryStatistics)GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcInstanceQueryStatistics");

		g_TcFunctions.PacketsBufferCreate			= (TcFcnPacketsBufferCreate)	GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcPacketsBufferCreate");
		g_TcFunctions.PacketsBufferDestroy			= (TcFcnPacketsBufferDestroy)	GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcPacketsBufferDestroy");
		g_TcFunctions.PacketsBufferQueryNextPacket	= (TcFcnPacketsBufferQueryNextPacket)GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcPacketsBufferQueryNextPacket");
		g_TcFunctions.PacketsBufferCommitNextPacket	= (TcFcnPacketsBufferCommitNextPacket)GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcPacketsBufferCommitNextPacket");

		g_TcFunctions.StatisticsDestroy				= (TcFcnStatisticsDestroy)		GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcStatisticsDestroy");
		g_TcFunctions.StatisticsUpdate				= (TcFcnStatisticsUpdate)		GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcStatisticsUpdate");
		g_TcFunctions.StatisticsQueryValue			= (TcFcnStatisticsQueryValue)	GetProcAddress(g_TcFunctions.hTcApiDllHandle, "TcStatisticsQueryValue");

		if (   g_TcFunctions.QueryPortList == NULL
			|| g_TcFunctions.FreePortList == NULL
			|| g_TcFunctions.StatusGetString == NULL
			|| g_TcFunctions.PortGetName == NULL
			|| g_TcFunctions.PortGetDescription == NULL
			|| g_TcFunctions.InstanceOpenByName == NULL
			|| g_TcFunctions.InstanceClose == NULL
			|| g_TcFunctions.InstanceSetFeature	 == NULL
			|| g_TcFunctions.InstanceQueryFeature == NULL
			|| g_TcFunctions.InstanceReceivePackets == NULL
			|| g_TcFunctions.InstanceGetReceiveWaitHandle == NULL
			|| g_TcFunctions.InstanceTransmitPackets == NULL
			|| g_TcFunctions.InstanceQueryStatistics == NULL
			|| g_TcFunctions.PacketsBufferCreate == NULL
			|| g_TcFunctions.PacketsBufferDestroy == NULL
			|| g_TcFunctions.PacketsBufferQueryNextPacket == NULL
			|| g_TcFunctions.PacketsBufferCommitNextPacket == NULL
			|| g_TcFunctions.StatisticsDestroy == NULL
			|| g_TcFunctions.StatisticsUpdate == NULL
			|| g_TcFunctions.StatisticsQueryValue == NULL
		)
		{
			break;
		}

		/*
		 * everything got loaded, yay!!
		 */
		currentStatus = TC_API_LOADED;
	}while(FALSE);

	if (currentStatus != TC_API_LOADED)
	{
		if (g_TcFunctions.hTcApiDllHandle != NULL)
		{
			FreeLibrary(g_TcFunctions.hTcApiDllHandle);
			g_TcFunctions.hTcApiDllHandle = NULL;
		}
	}

	InterlockedExchange((LONG*)&g_TcFunctions.LoadStatus, currentStatus);

	return currentStatus;
}
#else
// static linking
TC_API_LOAD_STATUS LoadTcFunctions()
{
	return TC_API_LOADED;
}
#endif // endif

int
TcFindAllDevs(pcap_if_t **alldevsp, char *errbuf)
{
	TC_API_LOAD_STATUS loadStatus;
	ULONG numPorts;
	PTC_PORT pPorts = NULL;
	TC_STATUS status;
	int result = 0;
	pcap_if_t *dev, *cursor;
	ULONG i;

	do
	{
		loadStatus = LoadTcFunctions();

		if (loadStatus != TC_API_LOADED)
		{
			result = 0;
			break;
		}

		/*
		 * enumerate the ports, and add them to the list
		 */
		status = g_TcFunctions.QueryPortList(&pPorts, &numPorts);

		if (status != TC_SUCCESS)
		{
			result = 0;
			break;
		}

		for (i = 0; i < numPorts; i++)
		{
			/*
			 * transform the port into an entry in the list
			 */
			dev = TcCreatePcapIfFromPort(pPorts[i]);

			if (dev != NULL)
			{
				/*
				 * append it at the end
				 */
				if (*alldevsp == NULL)
				{
					*alldevsp = dev;
				}
				else
				{
					for(cursor = *alldevsp; cursor->next != NULL; cursor = cursor->next);
					cursor->next = dev;
				}
			}
		}

		if (numPorts > 0)
		{
			/*
			 * ignore the result here
			 */
			status = g_TcFunctions.FreePortList(pPorts);
		}

	}while(FALSE);

	return result;
}

static pcap_if_t* TcCreatePcapIfFromPort(TC_PORT port)
{
	CHAR *name;
	CHAR *description;
	pcap_if_t *newIf = NULL;

	newIf = (pcap_if_t*)malloc(sizeof(*newIf));
	if (newIf == NULL)
	{
		return NULL;
	}

	memset(newIf, 0, sizeof(*newIf));

	name = g_TcFunctions.PortGetName(port);
	description = g_TcFunctions.PortGetDescription(port);

	newIf->name = (char*)malloc(strlen(name) + 1);
	if (newIf->name == NULL)
	{
		free(newIf);
		return NULL;
	}

	newIf->description = (char*)malloc(strlen(description) + 1);
	if (newIf->description == NULL)
	{
		free(newIf->name);
		free(newIf);
		return NULL;
	}

	strcpy(newIf->name, name);
	strcpy(newIf->description, description);

	newIf->addresses = NULL;
	newIf->next = NULL;
	newIf->flags = 0;

	return newIf;

}

int
IsTcDevice(pcap_t *p)
{
	ULONG numPorts;
	PTC_PORT pPorts = NULL;
	TC_STATUS status;
	int result = FALSE;
	ULONG i;

	do
	{
		if (LoadTcFunctions() != TC_API_LOADED)
		{
			result = FALSE;
			break;
		}

		/*
		 * enumerate the ports, and add them to the list
		 */
		status = g_TcFunctions.QueryPortList(&pPorts, &numPorts);

		if (status != TC_SUCCESS)
		{
			result = FALSE;
			break;
		}

		for (i = 0; i < numPorts; i++)
		{
			if(strcmp(g_TcFunctions.PortGetName(pPorts[i]), p->opt.source) == 0)
			{
				result = TRUE;
				break;
			}
		}

		if (numPorts > 0)
		{
			/*
			 * ignore the result here
			 */
			status = g_TcFunctions.FreePortList(pPorts);
		}

	}while(FALSE);

	return result;
}

/*
 * This function is registered as a callback only if IsTcDevice returned success i.e. Tc was loaded.
 * we do not need to check for it.
 */
int
TcActivate(pcap_t *p)
{
	TC_STATUS status;
	ULONG timeout;
	PPPI_HEADER pPpiHeader;

	if (p->opt.rfmon)
	{
		/*
		 * No monitor mode on Tc cards.  It could be done on
		 * Vista with drivers that support the native 802.11
		 * mechanism and monitor mode.
		 */
		return (PCAP_ERROR_RFMON_NOTSUP);
	}

	p->PpiPacket = (PCHAR)malloc(sizeof(PPI_HEADER) + MAX_TC_PACKET_SIZE);

	if (p->PpiPacket == NULL)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "Error allocating memory");
		return PCAP_ERROR;
	}

	/*
	 * Initialize the PPI fixed fields
	 */
	pPpiHeader = (PPPI_HEADER)p->PpiPacket;
	pPpiHeader->PacketHeader.PphDlt = DLT_EN10MB;
	pPpiHeader->PacketHeader.PphLength = sizeof(PPI_HEADER);
	pPpiHeader->PacketHeader.PphFlags = 0;
	pPpiHeader->PacketHeader.PphVersion = 0;

	pPpiHeader->AggregationFieldHeader.PfhLength = sizeof(PPI_FIELD_AGGREGATION_EXTENSION);
	pPpiHeader->AggregationFieldHeader.PfhType = PPI_FIELD_TYPE_AGGREGATION_EXTENSION;

	pPpiHeader->Dot3FieldHeader.PfhLength = sizeof(PPI_FIELD_802_3_EXTENSION);
	pPpiHeader->Dot3FieldHeader.PfhType = PPI_FIELD_TYPE_802_3_EXTENSION;

	status = g_TcFunctions.InstanceOpenByName(p->opt.source, &p->TcInstance);

	if (status != TC_SUCCESS)
	{
		/* Adapter detected but we are not able to open it. Return failure. */
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "Error opening TurboCap adapter: %s", g_TcFunctions.StatusGetString(status));
		return PCAP_ERROR;
	}

	p->linktype = DLT_EN10MB;
	p->dlt_list = (u_int *) malloc(sizeof(u_int) * 2);
	/*
	 * If that fails, just leave the list empty.
	 */
	if (p->dlt_list != NULL) {
		p->dlt_list[0] = DLT_EN10MB;
		p->dlt_list[1] = DLT_PPI;
		p->dlt_count = 2;
	}

	/*
	 * ignore promiscuous mode
	 * p->opt.promisc
	 */

	/*
	 * ignore all the buffer sizes
	 */

	/*
	 * enable reception
	 */
	status = g_TcFunctions.InstanceSetFeature(p->TcInstance, TC_INST_FT_RX_STATUS, 1);

	if (status != TC_SUCCESS)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE,"Error enabling reception on a TurboCap instance: %s", g_TcFunctions.StatusGetString(status));
		goto bad;
	}

	/*
	 * enable transmission
	 */
	status = g_TcFunctions.InstanceSetFeature(p->TcInstance, TC_INST_FT_TX_STATUS, 1);
	/*
	 * Ignore the error here.
	 */

	p->inject_op = TcInject;
	/*
	 * if the timeout is -1, it means immediate return, no timeout
	 * if the timeout is 0, it means INFINITE
	 */

	if (p->md.timeout == 0)
	{
		timeout = 0xFFFFFFFF;
	}
	else
	if (p->md.timeout < 0)
	{
		/*
		 *  we insert a minimal timeout here
		 */
		timeout = 10;
	}
	else
	{
		timeout = p->md.timeout;
	}

	status = g_TcFunctions.InstanceSetFeature(p->TcInstance, TC_INST_FT_READ_TIMEOUT, timeout);

	if (status != TC_SUCCESS)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE,"Error setting the read timeout a TurboCap instance: %s", g_TcFunctions.StatusGetString(status));
		goto bad;
	}

	p->read_op = TcRead;
	p->setfilter_op = TcSetFilter;
	p->setdirection_op = NULL;	/* Not implemented. */
	p->set_datalink_op = TcSetDatalink;
	p->getnonblock_op = TcGetNonBlock;
	p->setnonblock_op = TcSetNonBlock;
	p->stats_op = TcStats;
#ifdef WIN32
	p->setbuff_op = TcSetBuff;
	p->setmode_op = TcSetMode;
	p->setmintocopy_op = TcSetMinToCopy;
#else
	p->selectable_fd = -1;
#endif // endif

	p->cleanup_op = TcCleanup;

	return (0);
bad:
	TcCleanup(p);
	return (PCAP_ERROR);
}

static int TcSetDatalink(pcap_t *p, int dlt)
{
	/*
	 * always return 0, as the check is done by pcap_set_datalink
	 */
	return 0;
}

static int TcGetNonBlock(pcap_t *p, char *errbuf)
{
	snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
		    "Getting the non blocking status is not available for TurboCap ports");
	snprintf(errbuf, PCAP_ERRBUF_SIZE,
		    "Getting the non blocking status is not available for TurboCap ports");
		return -1;

}
static int TcSetNonBlock(pcap_t *p, int nonblock, char *errbuf)
{
	snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
		    "Setting the non blocking status is not available for TurboCap ports");
	snprintf(errbuf, PCAP_ERRBUF_SIZE,
		    "Setting the non blocking status is not available for TurboCap ports");
		return -1;
}

static void TcCleanup(pcap_t *p)
{

	if (p->TcPacketsBuffer != NULL)
	{
		g_TcFunctions.PacketsBufferDestroy(p->TcPacketsBuffer);
		p->TcPacketsBuffer = NULL;
	}
	if (p->TcInstance != NULL)
	{
		/*
		 * here we do not check for the error values
		 */
		g_TcFunctions.InstanceClose(p->TcInstance);
		p->TcInstance = NULL;
	}

	if (p->PpiPacket != NULL)
	{
		free(p->PpiPacket);
		p->PpiPacket = NULL;
	}

	pcap_cleanup_live_common(p);
}

/* Send a packet to the network */
static int TcInject(pcap_t *p, const void *buf, size_t size)
{
	TC_STATUS status;
	TC_PACKETS_BUFFER buffer;
	TC_PACKET_HEADER header;

	if (size >= 0xFFFF)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "send error: the TurboCap API does not support packets larger than 64k");
		return -1;
	}

	status = g_TcFunctions.PacketsBufferCreate(sizeof(TC_PACKET_HEADER) + TC_ALIGN_USHORT_TO_64BIT((USHORT)size), &buffer);

	if (status != TC_SUCCESS)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "send error: TcPacketsBufferCreate failure: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
		return -1;
	}

	/*
	 * we assume that the packet is without the checksum, as common with WinPcap
	 */
	memset(&header, 0, sizeof(header));

	header.Length = (USHORT)size;
	header.CapturedLength = header.Length;

	status = g_TcFunctions.PacketsBufferCommitNextPacket(buffer, &header, (PVOID)buf);

	if (status == TC_SUCCESS)
	{
		status = g_TcFunctions.InstanceTransmitPackets(p->TcInstance, buffer);

		if (status != TC_SUCCESS)
		{
			snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "send error: TcInstanceTransmitPackets failure: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
		}
	}
	else
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "send error: TcPacketsBufferCommitNextPacket failure: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
	}

	g_TcFunctions.PacketsBufferDestroy(buffer);

	if (status != TC_SUCCESS)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

static int TcRead(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
{
	TC_STATUS status;
	int n = 0;

	/*
	 * Has "pcap_breakloop()" been called?
	 */
	if (p->break_loop)
	{
		/*
		 * Yes - clear the flag that indicates that it
		 * has, and return -2 to indicate that we were
		 * told to break out of the loop.
		 */
		p->break_loop = 0;
		return (-2);
	}

	if (p->TcPacketsBuffer == NULL)
	{
		status = g_TcFunctions.InstanceReceivePackets(p->TcInstance, &p->TcPacketsBuffer);
		if (status != TC_SUCCESS)
		{
			snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "read error, TcInstanceReceivePackets failure: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
			return (-1);
		}
	}

	while (TRUE)
	{
		struct pcap_pkthdr hdr;
		TC_PACKET_HEADER tcHeader;
		PVOID data;
		ULONG filterResult;

		/*
		 * Has "pcap_breakloop()" been called?
		 * If so, return immediately - if we haven't read any
		 * packets, clear the flag and return -2 to indicate
		 * that we were told to break out of the loop, otherwise
		 * leave the flag set, so that the *next* call will break
		 * out of the loop without having read any packets, and
		 * return the number of packets we've processed so far.
		 */
		if (p->break_loop)
		{
			if (n == 0)
			{
				p->break_loop = 0;
				return (-2);
			}
			else
			{
				return (n);
			}
		}

		if (p->TcPacketsBuffer == NULL)
		{
			break;
		}

		status = g_TcFunctions.PacketsBufferQueryNextPacket(p->TcPacketsBuffer, &tcHeader, &data);

		if (status == TC_ERROR_END_OF_BUFFER)
		{
			g_TcFunctions.PacketsBufferDestroy(p->TcPacketsBuffer);
			p->TcPacketsBuffer = NULL;
			break;
		}

		if (status != TC_SUCCESS)
		{
			snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "read error, TcPacketsBufferQueryNextPacket failure: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
			return (-1);
		}

		/* No underlaying filtering system. We need to filter on our own */
		if (p->fcode.bf_insns)
		{
			filterResult = bpf_filter(p->fcode.bf_insns, data, tcHeader.Length, tcHeader.CapturedLength);

			if (filterResult == 0)
			{
				continue;
			}

			if (filterResult > tcHeader.CapturedLength)
			{
				filterResult = tcHeader.CapturedLength;
			}
		}
		else
		{
			filterResult = tcHeader.CapturedLength;
		}

		p->TcAcceptedCount ++;

		hdr.ts.tv_sec = (bpf_u_int32)(tcHeader.Timestamp / (ULONGLONG)(1000  * 1000 * 1000));
		hdr.ts.tv_usec = (bpf_u_int32)((tcHeader.Timestamp % (ULONGLONG)(1000  * 1000 * 1000)) / 1000);

		if (p->linktype == DLT_EN10MB)
		{
			hdr.caplen = filterResult;
			hdr.len = tcHeader.Length;
			(*callback)(user, &hdr, data);
		}
		else
		{
			PPPI_HEADER pPpiHeader = (PPPI_HEADER)p->PpiPacket;
			PVOID data2 = pPpiHeader + 1;

			pPpiHeader->AggregationField.InterfaceId = TC_PH_FLAGS_RX_PORT_ID(tcHeader.Flags);
			pPpiHeader->Dot3Field.Errors = tcHeader.Errors;
			if (tcHeader.Flags & TC_PH_FLAGS_CHECKSUM)
			{
				pPpiHeader->Dot3Field.Flags = PPI_FLD_802_3_EXT_FLAG_FCS_PRESENT;
			}
			else
			{
				pPpiHeader->Dot3Field.Flags = 0;
			}

			if (filterResult <= MAX_TC_PACKET_SIZE)
			{
				memcpy(data2, data, filterResult);
				hdr.caplen = sizeof(PPI_HEADER) + filterResult;
				hdr.len = sizeof(PPI_HEADER) + tcHeader.Length;
			}
			else
			{
				memcpy(data2, data, MAX_TC_PACKET_SIZE);
				hdr.caplen = sizeof(PPI_HEADER) + MAX_TC_PACKET_SIZE;
				hdr.len = sizeof(PPI_HEADER) + tcHeader.Length;
			}

			(*callback)(user, &hdr, p->PpiPacket);

		}

		if (++n >= cnt && cnt > 0)
		{
			return (n);
		}
	}

	return (n);
}

#ifdef WIN32

static int
TcSetMinToCopy(pcap_t *p, int size)
{
	TC_STATUS status;

	if (size < 0)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "Mintocopy cannot be less than 0.");
		return -1;
	}

	status = g_TcFunctions.InstanceSetFeature(p->TcInstance, TC_INST_FT_MINTOCOPY, (ULONG)size);

	if (status != TC_SUCCESS)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "TurboCap error setting the mintocopy: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
	}

	return 0;
}

static int
TcSetMode(pcap_t *p, int mode)
{
	if (mode != MODE_CAPT)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "Mode %u not supported by TurboCap devices. TurboCap only supports capture.", mode);
		return -1;
	}

	return 0;
}

/* Set the dimension of the kernel-level capture buffer */
static int
TcSetBuff(pcap_t *p, int dim)
{
	/*
	 * XXX turbocap has an internal way of managing buffers. And at the moment it's not configurable
	 */
	return 0;
}

#endif // endif

static int
TcStats(pcap_t *p, struct pcap_stat *ps)
{
	TC_STATISTICS statistics;
	TC_STATUS status;
	ULONGLONG counter;
	struct pcap_stat s;

	status = g_TcFunctions.InstanceQueryStatistics(p->TcInstance, &statistics);

	if (status != TC_SUCCESS)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "TurboCap error in TcInstanceQueryStatistics: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
		return -1;
	}

	memset(&s, 0, sizeof(s));

	status = g_TcFunctions.StatisticsQueryValue(statistics, TC_COUNTER_INSTANCE_TOTAL_RX_PACKETS, &counter);
	if (status != TC_SUCCESS)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "TurboCap error in TcStatisticsQueryValue: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
		return -1;
	}
	if (counter <= (ULONGLONG)0xFFFFFFFF)
	{
		s.ps_recv = (ULONG)counter;
	}
	else
	{
		s.ps_recv = 0xFFFFFFFF;
	}

	status = g_TcFunctions.StatisticsQueryValue(statistics, TC_COUNTER_INSTANCE_RX_DROPPED_PACKETS, &counter);
	if (status != TC_SUCCESS)
	{
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "TurboCap error in TcStatisticsQueryValue: %s (%08x)", g_TcFunctions.StatusGetString(status), status);
		return -1;
	}
	if (counter <= (ULONGLONG)0xFFFFFFFF)
	{
		s.ps_ifdrop = (ULONG)counter;
		s.ps_drop = (ULONG)counter;
	}
	else
	{
		s.ps_ifdrop = 0xFFFFFFFF;
		s.ps_drop = 0xFFFFFFFF;
	}

#ifdef HAVE_REMOTE
	s.ps_capt = p->TcAcceptedCount;
#endif // endif
	*ps = s;

	return 0;
}

/*
 * We filter at user level, since the kernel driver does't process the packets
 */
static int
TcSetFilter(pcap_t *p, struct bpf_program *fp)
{
	if(!fp)
	{
		strncpy(p->errbuf, "setfilter: No filter specified", sizeof(p->errbuf));
		return -1;
	}

	/* Install a user level filter */
	if (install_bpf_program(p, fp) < 0)
	{
		snprintf(p->errbuf, sizeof(p->errbuf),
			"setfilter, unable to install the filter: %s", pcap_strerror(errno));
		return -1;
	}

	p->md.use_bpf = 0;

	return (0);
}

#ifdef WIN32
HANDLE TcGetReceiveWaitHandle(pcap_t *p)
{
	return g_TcFunctions.InstanceGetReceiveWaitHandle(p->TcInstance);
}
#endif // endif
