#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/snmp.h>
#include <net-snmp/agent/snmp_agent.h>
#include <net-snmp/agent/snmp_vars.h>
#include "interface_private.h"
#include "net-snmp/data_access/interface.h"
#include <ws2tcpip.h>
#include <iphlpapi.h>

/* For Cygwin, MinGW32 and MSYS */
#ifndef NETIO_STATUS
#define NETIO_STATUS DWORD
#endif

#ifdef HAVE_MIB_IF_TABLE2
static void (*pFreeMibTable)(void *Table);
static NETIO_STATUS (*pGetIfEntry2)(MIB_IF_ROW2 *Row);
static NETIO_STATUS (*pGetIfTable2)(MIB_IF_TABLE2 **Table);
static NETIO_STATUS (*pSetIfEntry2)(MIB_IF_ROW2 *Row);
#endif

void
netsnmp_arch_interface_init(void)
{
#ifdef HAVE_MIB_IF_TABLE2
    HANDLE dll_handle = LoadLibrary("iphlpapi.dll");

    pFreeMibTable = (void(*)(void*))(uintptr_t)
        GetProcAddress(dll_handle, "FreeMibTable");
    pGetIfEntry2 = (NETIO_STATUS(*)(MIB_IF_ROW2*))(uintptr_t)
        GetProcAddress(dll_handle, "GetIfEntry2");
    pGetIfTable2 = (NETIO_STATUS(*)(MIB_IF_TABLE2**))(uintptr_t)
        GetProcAddress(dll_handle, "GetIfTable2");
    pSetIfEntry2 = (NETIO_STATUS(*)(MIB_IF_ROW2*))(uintptr_t)
        GetProcAddress(dll_handle, "SetIfEntry2");
#endif
}

/* Load interface data using GetIfTable() (Windows 2000 and later). */
static int netsnmp_arch_interface_load_old(netsnmp_container *container)
{
    DWORD           status;
    DWORD           dwActualSize = 0;
    MIB_IFTABLE    *iftable = NULL;
    MIB_IFROW      *row, *end;

    status = GetIfTable(iftable, &dwActualSize, /*bOrder=*/FALSE);
    if (status == NO_ERROR)
        return 0;
    if (status != ERROR_INSUFFICIENT_BUFFER)
        return SNMP_ERR_GENERR;
    iftable = malloc(dwActualSize);
    if (!iftable)
        return SNMP_ERR_GENERR;
    status = GetIfTable(iftable, &dwActualSize, /*bOrder=*/FALSE);
    if (status != NO_ERROR)
        return SNMP_ERR_GENERR;
    end = iftable->table + iftable->dwNumEntries;
    for (row = iftable->table; row < end; row++) {
        netsnmp_interface_entry *entry;

        entry = calloc(sizeof(*entry), 1);
        if (!entry)
            continue;
        entry->name = strdup((const char *) row->wszName);
        entry->oid_index.len = 1;
        entry->oid_index.oids = &entry->index;
        entry->index = row->dwIndex;
        entry->descr = netsnmp_memdup(row->bDescr, row->dwDescrLen);
        entry->type = row->dwType;
        entry->mtu = row->dwMtu;
        entry->speed = row->dwSpeed;
        entry->paddr_len = row->dwPhysAddrLen;
        entry->paddr = netsnmp_memdup(row->bPhysAddr, row->dwPhysAddrLen);
        entry->admin_status = row->dwAdminStatus;
        entry->oper_status = row->dwOperStatus ==
            MIB_IF_OPER_STATUS_OPERATIONAL ? 1/*up*/ : 2/*down*/;
        entry->lastchange = 0;  /* row->dwLastChange is not a Unix time */
        entry->stats.ibytes.low = row->dwInOctets;
        entry->stats.iucast.low = row->dwInUcastPkts;
        entry->stats.inucast = row->dwInNUcastPkts;
        entry->stats.ierrors = row->dwInErrors;
        entry->stats.idiscards = row->dwInDiscards;
        entry->stats.iunknown_protos = row->dwInUnknownProtos;
        entry->stats.obytes.low = row->dwOutOctets;
        entry->stats.oucast.low = row->dwOutUcastPkts;
        entry->stats.oerrors = row->dwOutErrors;
        entry->stats.oqlen = row->dwOutQLen;
        entry->stats.onucast = row->dwOutNUcastPkts;
        CONTAINER_INSERT(container, entry);
    }

    free(iftable);
    return 0;
}

#ifdef HAVE_MIB_IF_TABLE2
/*
 * Load interface data using GetIfTable2(). GetIfTable2() is available on
 * Windows Vista, Windows Server 2008 and later.
 */
static int netsnmp_arch_interface_load_new(netsnmp_container *container)
{
    DWORD           status;
    MIB_IF_TABLE2  *iftable = NULL;
    MIB_IF_ROW2    *row, *end;
    char           *descr;
    unsigned        descr_len;

    status = pGetIfTable2(&iftable);
    if (status != NO_ERROR)
        return SNMP_ERR_GENERR;
    end = iftable->Table + iftable->NumEntries;
    for (row = iftable->Table; row < end; row++) {
        netsnmp_interface_entry *entry;

        entry = calloc(sizeof(*entry), 1);
        if (!entry)
            continue;
        entry->name = strdup((const char *) row->Alias);
        entry->oid_index.len = 1;
        entry->oid_index.oids = &entry->index;
        entry->index = row->InterfaceIndex;
        descr_len = WideCharToMultiByte(CP_UTF8, 0, row->Description, -1, NULL,
                                        0, NULL, NULL);
        descr = malloc(descr_len);
        if (row && WideCharToMultiByte(CP_UTF8, 0, row->Description, -1, descr,
                                       descr_len, NULL, NULL) > 0) {
            entry->descr = descr;
        } else {
            free(row);
        }
        entry->type = row->Type;
        entry->mtu = row->Mtu;
        entry->speed = row->TransmitLinkSpeed;
        entry->paddr_len = row->PhysicalAddressLength;
        entry->paddr = netsnmp_memdup(row->PhysicalAddress,
                                      row->PhysicalAddressLength);
        entry->admin_status = row->AdminStatus;
        entry->oper_status = row->OperStatus;
        entry->lastchange = 0;  /* row->dwLastChange is not a Unix time */
        entry->stats.ibytes.low = row->InOctets;
        entry->stats.iucast.low = row->InUcastPkts;
        entry->stats.inucast = row->InNUcastPkts;
        entry->stats.ierrors = row->InErrors;
        entry->stats.idiscards = row->InDiscards;
        entry->stats.iunknown_protos = row->InUnknownProtos;
        entry->stats.obytes.low = row->OutOctets;
        entry->stats.oucast.low = row->OutUcastPkts;
        entry->stats.oerrors = row->OutErrors;
        entry->stats.oqlen = row->OutQLen;
        entry->stats.onucast = row->OutNUcastPkts;
        CONTAINER_INSERT(container, entry);
    }

    pFreeMibTable(iftable);
    return 0;
}
#else
static int netsnmp_arch_interface_load_new(netsnmp_container *container)
{
    return SNMP_ERR_GENERR;
}
#endif

int
netsnmp_arch_interface_container_load(netsnmp_container *container,
                                      u_int load_flags)
{
#ifdef HAVE_MIB_IF_TABLE2
    return pGetIfTable2 ? netsnmp_arch_interface_load_new(container) :
        netsnmp_arch_interface_load_old(container);
#else
    return netsnmp_arch_interface_load_old(container);
#endif
}

oid
netsnmp_arch_interface_index_find(const char *name)
{
    /* To do: implement this function. */
    return 0;
}

int
netsnmp_arch_set_admin_status(struct netsnmp_interface_entry_s *entry,
                              int ifAdminStatus_val)
{
    MIB_IFROW       row;

    memset(&row, 0, sizeof(row));
    row.dwIndex = entry->index;
    if (GetIfEntry(&row) != NO_ERROR) {
        snmp_log(LOG_ERR, "GetIfEntry() failed for index %" NETSNMP_PRIo "u",
                 entry->index);
        return SNMP_ERR_GENERR;
    }
    row.dwAdminStatus = ifAdminStatus_val;
    if (SetIfEntry(&row) != NO_ERROR) {
        snmp_log(LOG_ERR,
                 "SetIfEntry() failed for index %" NETSNMP_PRIo "u and admin status %u\n",
                 entry->index, ifAdminStatus_val);
        return SNMP_ERR_GENERR;
    }
    return 0;
}
