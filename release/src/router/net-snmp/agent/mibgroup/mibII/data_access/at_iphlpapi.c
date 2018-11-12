/*
 *  Template MIB group implementation - at.c
 *
 */

/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/interface.h>
#include "../at.h"
#if defined(cygwin)
#include <windows.h>
#endif
#if defined(cygwin) || defined(mingw32)
#include <winerror.h>
#endif
#include <iphlpapi.h>

static WriteMethod write_arp;
MIB_IPNETROW   *arp_row = NULL;
int             create_flag = 0;

u_char         *
var_atEntry(struct variable *vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    /*
     * Address Translation table object identifier is of form:
     * 1.3.6.1.2.1.3.1.?.interface.1.A.B.C.D,  where A.B.C.D is IP address.
     * Interface is at offset 10,
     * IPADDR starts at offset 12.
     *
     * IP Net to Media table object identifier is of form:
     * 1.3.6.1.2.1.4.22.1.?.interface.A.B.C.D,  where A.B.C.D is IP address.
     * Interface is at offset 10,
     * IPADDR starts at offset 11.
     */
    u_char         *cp;
    oid            *op;
    oid             lowest[16];
    oid             current[16];
    int             oid_length;
    int             lowState = -1;      /* Don't have one yet */
    PMIB_IPNETTABLE pIpNetTable = NULL;
    DWORD           status = NO_ERROR;
    DWORD           dwActualSize = 0;
    UINT            i;
    int             j;
    u_char          dest_addr[4];
    void           *result = NULL;
    static in_addr_t	addr_ret;

    /*
     * fill in object part of name for current (less sizeof instance part)
     */
    memcpy((char *) current, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));

    if (current[6] == 3) {      /* AT group oid */
        oid_length = 16;
    } else {                    /* IP NetToMedia group oid */
        oid_length = 15;
    }

    status = GetIpNetTable(pIpNetTable, &dwActualSize, TRUE);
    if (status == ERROR_INSUFFICIENT_BUFFER) {
        pIpNetTable = malloc(dwActualSize);
        if (pIpNetTable)
            status = GetIpNetTable(pIpNetTable, &dwActualSize, TRUE);
    }

    i = -1;

    if (status == NO_ERROR) {
        for (i = 0; i < pIpNetTable->dwNumEntries; ++i) {
            current[10] = pIpNetTable->table[i].dwIndex;


            if (current[6] == 3) {      /* AT group oid */
                current[11] = 1;
                op = current + 12;
            } else {            /* IP NetToMedia group oid */
                op = current + 11;
            }
            cp = (u_char *) & pIpNetTable->table[i].dwAddr;
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;

            if (exact) {
                if (snmp_oid_compare(current, oid_length, name, *length) ==
                    0) {
                    memcpy((char *) lowest, (char *) current,
                           oid_length * sizeof(oid));
                    lowState = 0;
                    break;      /* no need to search further */
                }
            } else {
                if (snmp_oid_compare(current, oid_length, name, *length) >
                    0) {
                    memcpy((char *) lowest, (char *) current,
                           oid_length * sizeof(oid));
                    lowState = 0;
                    break;      /* As the table is sorted, no need to search further */
                }
            }
        }
    }
    if (arp_row == NULL) {
        /*
         * Free allocated memory in case of SET request's FREE phase
         */
        arp_row = (PMIB_IPNETROW) malloc(sizeof(MIB_IPNETROW));
    }

    if (lowState < 0 || status != NO_ERROR) {
        /*
         * for creation of new row, only ipNetToMediaTable case is considered
         */
        if (*length == 15 || *length == 16) {
            create_flag = 1;
            *write_method = write_arp;
            arp_row->dwIndex = name[10];

            if (*length == 15) {        /* ipNetToMediaTable */
                j = 11;
            } else {            /* at Table */

                j = 12;
            }

            dest_addr[0] = (u_char) name[j];
            dest_addr[1] = (u_char) name[j + 1];
            dest_addr[2] = (u_char) name[j + 2];
            dest_addr[3] = (u_char) name[j + 3];
            arp_row->dwAddr = *((DWORD *) dest_addr);

            arp_row->dwType = 4;        /* Static */
            arp_row->dwPhysAddrLen = 0;
        }
        goto out;
    }

    create_flag = 0;
    memcpy((char *) name, (char *) lowest, oid_length * sizeof(oid));
    *length = oid_length;
    *write_method = write_arp;
    netsnmp_assert(0 <= i && i < pIpNetTable->dwNumEntries);
    *arp_row = pIpNetTable->table[i];

    switch (vp->magic) {
    case IPMEDIAIFINDEX:       /* also ATIFINDEX */
        *var_len = sizeof long_return;
        long_return = pIpNetTable->table[i].dwIndex;
        result = &long_return;
        break;
    case IPMEDIAPHYSADDRESS:   /* also ATPHYSADDRESS */
        *var_len = pIpNetTable->table[i].dwPhysAddrLen;
        memcpy(return_buf, pIpNetTable->table[i].bPhysAddr, *var_len);
        result = return_buf;
        break;
    case IPMEDIANETADDRESS:    /* also ATNETADDRESS */
        *var_len = sizeof(addr_ret);
        addr_ret = pIpNetTable->table[i].dwAddr;
        result = &addr_ret;
        break;
    case IPMEDIATYPE:
        *var_len = sizeof long_return;
        long_return = pIpNetTable->table[i].dwType;
        result = &long_return;
        break;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_atEntry\n",
                    vp->magic));
        break;
    }
out:
    free(pIpNetTable);
    return result;
}

int
write_arp(int action,
          u_char * var_val,
          u_char var_val_type,
          size_t var_val_len, u_char * statP, oid * name, size_t length)
{
    int             var, retval = SNMP_ERR_NOERROR;
    static PMIB_IPNETROW oldarp_row = NULL;
    MIB_IPNETROW    temp_row;
    DWORD           status = NO_ERROR;

    /*
     * IP Net to Media table object identifier is of form:
     * 1.3.6.1.2.1.4.22.1.?.interface.A.B.C.D,  where A.B.C.D is IP address.
     * Interface is at offset 10,
     * IPADDR starts at offset 11.
     */

    if (name[6] == 3) {         /* AT group oid */
        if (length != 16) {
            snmp_log(LOG_ERR, "length error\n");
            return SNMP_ERR_NOCREATION;
        }
    } else {                    /* IP NetToMedia group oid */
        if (length != 15) {
            snmp_log(LOG_ERR, "length error\n");
            return SNMP_ERR_NOCREATION;
        }
    }


    /*
     * #define for ipNetToMediaTable entries are 1 less than corresponding sub-id in MIB
     * * i.e. IPMEDIAIFINDEX defined as 0, but ipNetToMediaIfIndex registered as 1
     */
    var = name[9] - 1;
    switch (action) {
    case RESERVE1:
        switch (var) {
        case IPMEDIAIFINDEX:
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR, "not integer\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if ((*((int *) var_val)) < 0) {
                snmp_log(LOG_ERR, "invalid media ifIndex");
                return SNMP_ERR_WRONGVALUE;
            }
            if (var_val_len > sizeof(int)) {
                snmp_log(LOG_ERR, "bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case IPMEDIANETADDRESS:
            if (var_val_type != ASN_IPADDRESS) {
                snmp_log(LOG_ERR, "not IP Address\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if ((*((int *) var_val)) < 0) {
                snmp_log(LOG_ERR, "invalid media net address");
                return SNMP_ERR_WRONGVALUE;
            }
            if (var_val_len > sizeof(DWORD)) {
                snmp_log(LOG_ERR, "bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case IPMEDIATYPE:
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR, "not integer\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if ((*((int *) var_val)) < 1 || (*((int *) var_val)) > 4) {
                snmp_log(LOG_ERR, "invalid media type");
                return SNMP_ERR_WRONGVALUE;
            }
            if (var_val_len > sizeof(int)) {
                snmp_log(LOG_ERR, "bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case IPMEDIAPHYSADDRESS:
            if (var_val_type != ASN_OCTET_STR) {
                snmp_log(LOG_ERR, "not octet str");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != 6) {
                snmp_log(LOG_ERR, "not correct ipAddress length: %d",
                         var_val_len);
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        default:
            DEBUGMSGTL(("snmpd", "unknown sub-id %d in write_rte\n",
                        var + 1));
            return SNMP_ERR_NOTWRITABLE;
        }
        break;
    case RESERVE2:
        /*
         * Save the old value, in case of UNDO
         */
        if (oldarp_row == NULL) {
            oldarp_row = (PMIB_IPNETROW) malloc(sizeof(MIB_IPNETROW));
            *oldarp_row = *arp_row;
        }
        break;
    case ACTION:               /* Perform the SET action (if reversible) */
        switch (var) {

        case IPMEDIAIFINDEX:
            temp_row = *arp_row;
            arp_row->dwIndex = *((int *) var_val);
            /*
             * In case of new entry, physical address is mandatory.
             * * SetIpNetEntry will be done in COMMIT case
             */
            if (!create_flag) {
                if (SetIpNetEntry(arp_row) != NO_ERROR) {
                    arp_row->dwIndex = temp_row.dwIndex;
                    retval = SNMP_ERR_COMMITFAILED;
                }
                /*
                 * Don't know yet, whether change in ifIndex creates new row or not
                 */
                /*
                 * else{
                 */
                /*
                 * temp_row.dwType = 2;
                 */
                /*
                 * if(SetIpNetEntry(&temp_row) != NO_ERROR)
                 */
                /*
                 * retval = SNMP_ERR_COMMITFAILED;
                 */
                /*
                 * }
                 */
            }
            break;
        case IPMEDIANETADDRESS:
            temp_row = *arp_row;
            arp_row->dwAddr = *((int *) var_val);
            if (!create_flag) {
                if (SetIpNetEntry(arp_row) != NO_ERROR) {
                    arp_row->dwAddr = oldarp_row->dwAddr;
                    retval = SNMP_ERR_COMMITFAILED;
                } else {
                    temp_row.dwType = 2;
                    if (SetIpNetEntry(&temp_row) != NO_ERROR) {
                        snmp_log(LOG_ERR,
                                 "Failed in ACTION, while deleting old row \n");
                        retval = SNMP_ERR_COMMITFAILED;
                    }
                }
            }
            break;
        case IPMEDIATYPE:
            arp_row->dwType = *((int *) var_val);
            if (!create_flag) {
                if (SetIpNetEntry(arp_row) != NO_ERROR)
                    retval = SNMP_ERR_COMMITFAILED;
            }
            break;
        case IPMEDIAPHYSADDRESS:
            memcpy(arp_row->bPhysAddr, var_val, var_val_len);
            arp_row->dwPhysAddrLen = var_val_len;
            if (!create_flag) {
                if (SetIpNetEntry(arp_row) != NO_ERROR)
                    retval = SNMP_ERR_COMMITFAILED;
            }
            break;
        default:
            DEBUGMSGTL(("snmpd", "unknown sub-id %d in write_arp\n",
                        var + 1));
            retval = SNMP_ERR_NOTWRITABLE;
        }
        return retval;
    case UNDO:
        /*
         * Reverse the SET action and free resources
         */
        if (oldarp_row != NULL) {
            /*
             * UNDO the changes done for existing entry.
             */
            if (!create_flag) {
                if ((status = SetIpNetEntry(oldarp_row)) != NO_ERROR) {
                    snmp_log(LOG_ERR, "Error in case UNDO, status : %lu\n",
                             status);
                    retval = SNMP_ERR_UNDOFAILED;
                }
            }

            if (oldarp_row->dwAddr != arp_row->dwAddr) {
                arp_row->dwType = 2;    /*If row was added/created delete that row */

                if ((status = SetIpNetEntry(arp_row)) != NO_ERROR) {
                    snmp_log(LOG_ERR,
                             "Error while deleting added row, status : %lu\n",
                             status);
                    retval = SNMP_ERR_UNDOFAILED;
                }
            }
            free(oldarp_row);
            oldarp_row = NULL;
            free(arp_row);
            arp_row = NULL;
            return retval;
        }
        break;
    case COMMIT:
        /*
         * if new entry and physical address specified, create new entry
         */
        if (create_flag) {
            if (arp_row->dwPhysAddrLen != 0) {
                if ((status = CreateIpNetEntry(arp_row)) != NO_ERROR) {
                    snmp_log(LOG_ERR,
                             "Inside COMMIT: CreateIpNetEntry failed, status %lu\n",
                             status);
                    retval = SNMP_ERR_COMMITFAILED;
                }
            } else {
                /*
                 * For new entry, physical address must be set.
                 */
                snmp_log(LOG_ERR,
                         "Can't create new entry without physical address\n");
                retval = SNMP_ERR_WRONGVALUE;
            }
            /*
             * unset the create_flag, so that CreateIpNetEntry called only once
             */
            create_flag = 0;
        }

    case FREE:
        /*
         * Free any resources allocated
         */
        free(oldarp_row);
        oldarp_row = NULL;
        free(arp_row);
        arp_row = NULL;
        break;
    }
    return retval;
}
