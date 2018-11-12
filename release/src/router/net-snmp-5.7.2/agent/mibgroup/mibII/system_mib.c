/*
 *  System MIB group implementation - system.c
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
#include <net-snmp/net-snmp-features.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>

#if HAVE_UTSNAME_H
#include <utsname.h>
#else
#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#endif

#if defined(cygwin) || defined(mingw32)
#include <winerror.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/sysORTable.h>

#include "util_funcs.h"
#include "system_mib.h"
#include "updates.h"
#include "agent_global_vars.h"

netsnmp_feature_require(watcher_read_only_int_scalar)

        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

#define SYS_STRING_LEN	256
static char     version_descr[SYS_STRING_LEN] = NETSNMP_VERS_DESC;
static char     sysContact[SYS_STRING_LEN] = NETSNMP_SYS_CONTACT;
static char     sysName[SYS_STRING_LEN] = NETSNMP_SYS_NAME;
static char     sysLocation[SYS_STRING_LEN] = NETSNMP_SYS_LOC;
static oid      sysObjectID[MAX_OID_LEN];
static size_t sysObjectIDByteLength;

static int      sysServices = 72;
static int      sysServicesConfiged = 0;

static int      sysContactSet = 0, sysLocationSet = 0, sysNameSet = 0;

#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
static void     windowsOSVersionString(char [], size_t);
#endif

        /*********************
	 *
	 *  snmpd.conf config parsing
	 *
	 *********************/

static void
system_parse_config_string2(const char *token, char *cptr,
                            char* value, size_t size)
{
    if (strlen(cptr) < size) {
        strcpy(value, cptr);
    } else {
        netsnmp_config_error("%s token too long (must be < %lu):\n\t%s",
                             token, (unsigned long)size, cptr);
    }
}

static void
system_parse_config_string(const char *token, char *cptr,
                           const char *name, char* value, size_t size,
                           int* guard)
{
    if (*token == 'p') {
        if (*guard < 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- the value is
             * already configured read-only.
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only %s.0\n", name);
            return;
        } else {
            *guard = 1;
        }
    } else {
        if (*guard > 0) {
            /*
             * This is bogus (and shouldn't happen anyway) -- we already read a
             * persistent value which we should ignore in favour of this one.
             */
            snmp_log(LOG_WARNING,
                     "ignoring attempted override of read-only %s.0\n", name);
            /*
             * Fall through and copy in this value.
             */
        }
        *guard = -1;
    }

    system_parse_config_string2(token, cptr, value, size);
}

static void
system_parse_config_sysdescr(const char *token, char *cptr)
{
    system_parse_config_string2(token, cptr, version_descr,
                                sizeof(version_descr));
}

static void
system_parse_config_sysloc(const char *token, char *cptr)
{
    system_parse_config_string(token, cptr, "sysLocation", sysLocation,
                               sizeof(sysLocation), &sysLocationSet);
}

static void
system_parse_config_syscon(const char *token, char *cptr)
{
    system_parse_config_string(token, cptr, "sysContact", sysContact,
                               sizeof(sysContact), &sysContactSet);
}

static void
system_parse_config_sysname(const char *token, char *cptr)
{
    system_parse_config_string(token, cptr, "sysName", sysName,
                               sizeof(sysName), &sysNameSet);
}

static void
system_parse_config_sysServices(const char *token, char *cptr)
{
    sysServices = atoi(cptr);
    sysServicesConfiged = 1;
}

static void
system_parse_config_sysObjectID(const char *token, char *cptr)
{
    size_t sysObjectIDLength = MAX_OID_LEN;
    if (!read_objid(cptr, sysObjectID, &sysObjectIDLength)) {
	netsnmp_config_error("sysobjectid token not a parsable OID:\n\t%s",
			     cptr);
        sysObjectIDByteLength = version_sysoid_len  * sizeof(oid);
        memcpy(sysObjectID, version_sysoid, sysObjectIDByteLength);
    } else

		sysObjectIDByteLength = sysObjectIDLength * sizeof(oid);
}


        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/

oid             system_module_oid[] = { SNMP_OID_SNMPMODULES, 1 };
int             system_module_oid_len = OID_LENGTH(system_module_oid);
int             system_module_count = 0;

static int
system_store(int a, int b, void *c, void *d)
{
    char            line[SNMP_MAXBUF_SMALL];

    if (sysLocationSet > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "psyslocation %s", sysLocation);
        snmpd_store_config(line);
    }
    if (sysContactSet > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "psyscontact %s", sysContact);
        snmpd_store_config(line);
    }
    if (sysNameSet > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "psysname %s", sysName);
        snmpd_store_config(line);
    }

    return 0;
}

static int
handle_sysServices(netsnmp_mib_handler *handler,
                   netsnmp_handler_registration *reginfo,
                   netsnmp_agent_request_info *reqinfo,
                   netsnmp_request_info *requests)
{
#if NETSNMP_NO_DUMMY_VALUES
    if (reqinfo->mode == MODE_GET && !sysServicesConfiged)
        netsnmp_request_set_error(requests, SNMP_NOSUCHINSTANCE);
#endif
    return SNMP_ERR_NOERROR;
}

static int
handle_sysUpTime(netsnmp_mib_handler *handler,
                   netsnmp_handler_registration *reginfo,
                   netsnmp_agent_request_info *reqinfo,
                   netsnmp_request_info *requests)
{
    snmp_set_var_typed_integer(requests->requestvb, ASN_TIMETICKS,
                               netsnmp_get_agent_uptime());
    return SNMP_ERR_NOERROR;
}

void
init_system_mib(void)
{

#ifdef HAVE_UNAME
    struct utsname  utsName;

    uname(&utsName);
    snprintf(version_descr, sizeof(version_descr),
            "%s %s %s %s %s", utsName.sysname,
            utsName.nodename, utsName.release, utsName.version,
            utsName.machine);
    version_descr[ sizeof(version_descr)-1 ] = 0;
#else
#if HAVE_EXECV
    struct extensible extmp;

    /*
     * set default values of system stuff 
     */
    if (asprintf(&extmp.command, "%s -a", UNAMEPROG) < 0)
        extmp.command = NULL;
    /*
     * setup defaults 
     */
    extmp.type = EXECPROC;
    extmp.next = NULL;
    exec_command(&extmp);
    strlcpy(version_descr, extmp.output, sizeof(version_descr));
    if (strlen(version_descr) >= 1)
        version_descr[strlen(version_descr) - 1] = 0; /* chomp new line */
#else
#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
    windowsOSVersionString(version_descr, sizeof(version_descr));
#else
    strcpy(version_descr, "unknown");
#endif
#endif
#endif

#ifdef HAVE_GETHOSTNAME
    gethostname(sysName, sizeof(sysName));
#else
#ifdef HAVE_UNAME
    strlcpy(sysName, utsName.nodename, sizeof(sysName));
#else
#if defined (HAVE_EXECV) && !defined (mingw32)
    if (asprintf(&extmp.command, "%s -n", UNAMEPROG) < 0)
        extmp.command = NULL;
    /*
     * setup defaults 
     */
    extmp.type = EXECPROC;
    extmp.next = NULL;
    exec_command(&extmp);
    strlcpy(sysName, extmp.output, sizeof(sysName));
    if (strlen(sysName) >= 1)
        sysName[strlen(sysName) - 1] = 0; /* chomp new line */
#else
    strcpy(sysName, "unknown");
#endif                          /* HAVE_EXECV */
#endif                          /* HAVE_UNAME */
#endif                          /* HAVE_GETHOSTNAME */

#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
    {
      HKEY hKey;
      /* Default sysContact is the registered windows user */
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0,
                       KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
          char registeredOwner[256] = "";
          DWORD registeredOwnerSz = 256;
          if (RegQueryValueEx(hKey, "RegisteredOwner", NULL, NULL,
                              (LPBYTE)registeredOwner,
                              &registeredOwnerSz) == ERROR_SUCCESS) {
              strlcpy(sysContact, registeredOwner, sizeof(sysContact));
          }
          RegCloseKey(hKey);
      }
    }
#endif

    /* default sysObjectID */
    memcpy(sysObjectID, version_sysoid, version_sysoid_len * sizeof(oid));
    sysObjectIDByteLength = version_sysoid_len * sizeof(oid);

    {
        const oid sysDescr_oid[] = { 1, 3, 6, 1, 2, 1, 1, 1 };
        static netsnmp_watcher_info sysDescr_winfo;
        netsnmp_register_watched_scalar(
            netsnmp_create_handler_registration(
                "mibII/sysDescr", NULL, sysDescr_oid, OID_LENGTH(sysDescr_oid),
                HANDLER_CAN_RONLY),
            netsnmp_init_watcher_info(&sysDescr_winfo, version_descr, 0,
				      ASN_OCTET_STR, WATCHER_SIZE_STRLEN));
    }
    {
        const oid sysObjectID_oid[] = { 1, 3, 6, 1, 2, 1, 1, 2 };
        static netsnmp_watcher_info sysObjectID_winfo;
        netsnmp_register_watched_scalar(
            netsnmp_create_handler_registration(
                "mibII/sysObjectID", NULL,
                sysObjectID_oid, OID_LENGTH(sysObjectID_oid),
                HANDLER_CAN_RONLY),
            netsnmp_init_watcher_info6(
		&sysObjectID_winfo, sysObjectID, 0, ASN_OBJECT_ID,
                WATCHER_MAX_SIZE | WATCHER_SIZE_IS_PTR,
                MAX_OID_LEN, &sysObjectIDByteLength));
    }
    {
        const oid sysUpTime_oid[] = { 1, 3, 6, 1, 2, 1, 1, 3 };
        netsnmp_register_scalar(
            netsnmp_create_handler_registration(
                "mibII/sysUpTime", handle_sysUpTime,
                sysUpTime_oid, OID_LENGTH(sysUpTime_oid),
                HANDLER_CAN_RONLY));
    }
    {
        const oid sysContact_oid[] = { 1, 3, 6, 1, 2, 1, 1, 4 };
        static netsnmp_watcher_info sysContact_winfo;
#ifndef NETSNMP_NO_WRITE_SUPPORT
        netsnmp_register_watched_scalar(
            netsnmp_create_update_handler_registration(
                "mibII/sysContact", sysContact_oid, OID_LENGTH(sysContact_oid), 
                HANDLER_CAN_RWRITE, &sysContactSet),
            netsnmp_init_watcher_info(
                &sysContact_winfo, sysContact, SYS_STRING_LEN - 1,
                ASN_OCTET_STR, WATCHER_MAX_SIZE | WATCHER_SIZE_STRLEN));
#else  /* !NETSNMP_NO_WRITE_SUPPORT */
        netsnmp_register_watched_scalar(
            netsnmp_create_update_handler_registration(
                "mibII/sysContact", sysContact_oid, OID_LENGTH(sysContact_oid),
                HANDLER_CAN_RONLY, &sysContactSet),
            netsnmp_init_watcher_info(
                &sysContact_winfo, sysContact, SYS_STRING_LEN - 1,
                ASN_OCTET_STR, WATCHER_MAX_SIZE | WATCHER_SIZE_STRLEN));
#endif /* !NETSNMP_NO_WRITE_SUPPORT */
    }
    {
        const oid sysName_oid[] = { 1, 3, 6, 1, 2, 1, 1, 5 };
        static netsnmp_watcher_info sysName_winfo;
#ifndef NETSNMP_NO_WRITE_SUPPORT
        netsnmp_register_watched_scalar(
            netsnmp_create_update_handler_registration(
                "mibII/sysName", sysName_oid, OID_LENGTH(sysName_oid),
                HANDLER_CAN_RWRITE, &sysNameSet),
            netsnmp_init_watcher_info(
                &sysName_winfo, sysName, SYS_STRING_LEN - 1, ASN_OCTET_STR,
                WATCHER_MAX_SIZE | WATCHER_SIZE_STRLEN));
#else  /* !NETSNMP_NO_WRITE_SUPPORT */
        netsnmp_register_watched_scalar(
            netsnmp_create_update_handler_registration(
                "mibII/sysName", sysName_oid, OID_LENGTH(sysName_oid),
                HANDLER_CAN_RONLY, &sysNameSet),
            netsnmp_init_watcher_info(
                &sysName_winfo, sysName, SYS_STRING_LEN - 1, ASN_OCTET_STR,
                WATCHER_MAX_SIZE | WATCHER_SIZE_STRLEN));
#endif /* !NETSNMP_NO_WRITE_SUPPORT */
    }
    {
        const oid sysLocation_oid[] = { 1, 3, 6, 1, 2, 1, 1, 6 };
        static netsnmp_watcher_info sysLocation_winfo;
#ifndef NETSNMP_NO_WRITE_SUPPORT
        netsnmp_register_watched_scalar(
            netsnmp_create_update_handler_registration(
                "mibII/sysLocation", sysLocation_oid,
                OID_LENGTH(sysLocation_oid),
                HANDLER_CAN_RWRITE, &sysLocationSet),
            netsnmp_init_watcher_info(
		&sysLocation_winfo, sysLocation, SYS_STRING_LEN - 1,
		ASN_OCTET_STR, WATCHER_MAX_SIZE | WATCHER_SIZE_STRLEN));
#else  /* !NETSNMP_NO_WRITE_SUPPORT */
        netsnmp_register_watched_scalar(
            netsnmp_create_update_handler_registration(
                "mibII/sysLocation", sysLocation_oid,
                OID_LENGTH(sysLocation_oid),
                HANDLER_CAN_RONLY, &sysLocationSet),
            netsnmp_init_watcher_info(
		&sysLocation_winfo, sysLocation, SYS_STRING_LEN - 1,
		ASN_OCTET_STR, WATCHER_MAX_SIZE | WATCHER_SIZE_STRLEN));
#endif /* !NETSNMP_NO_WRITE_SUPPORT */
    }
    {
        const oid sysServices_oid[] = { 1, 3, 6, 1, 2, 1, 1, 7 };
        netsnmp_register_read_only_int_scalar(
            "mibII/sysServices", sysServices_oid, OID_LENGTH(sysServices_oid),
            &sysServices, handle_sysServices);
    }
    if (++system_module_count == 3)
        REGISTER_SYSOR_ENTRY(system_module_oid,
                             "The MIB module for SNMPv2 entities");

    sysContactSet = sysLocationSet = sysNameSet = 0;

    /*
     * register our config handlers 
     */
    snmpd_register_config_handler("sysdescr",
                                  system_parse_config_sysdescr, NULL,
                                  "description");
    snmpd_register_config_handler("syslocation",
                                  system_parse_config_sysloc, NULL,
                                  "location");
    snmpd_register_config_handler("syscontact", system_parse_config_syscon,
                                  NULL, "contact-name");
    snmpd_register_config_handler("sysname", system_parse_config_sysname,
                                  NULL, "node-name");
    snmpd_register_config_handler("psyslocation",
                                  system_parse_config_sysloc, NULL, NULL);
    snmpd_register_config_handler("psyscontact",
                                  system_parse_config_syscon, NULL, NULL);
    snmpd_register_config_handler("psysname", system_parse_config_sysname,
                                  NULL, NULL);
    snmpd_register_config_handler("sysservices",
                                  system_parse_config_sysServices, NULL,
                                  "NUMBER");
    snmpd_register_config_handler("sysobjectid",
                                  system_parse_config_sysObjectID, NULL,
                                  "OID");
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           system_store, NULL);
}

        /*********************
	 *
	 *  Internal implementation functions - None
	 *
	 *********************/

#if (defined (WIN32) && defined (HAVE_WIN32_PLATFORM_SDK)) || defined (mingw32)
static DWORD RegReadDword(HKEY hKey, LPCTSTR lpSubkey, LPCTSTR lpValueName)
{
    HKEY hSubkey;
    LONG qres;
    DWORD key_type;
    DWORD result = 0;
    DWORD result_len = sizeof(result);

    if (RegOpenKeyEx(hKey, lpSubkey, 0, KEY_READ, &hSubkey) != ERROR_SUCCESS)
        goto out;
    qres = RegQueryValueEx(hSubkey, lpValueName, NULL, &key_type,
                           (void *)&result, &result_len);
    if (qres != ERROR_SUCCESS || key_type != REG_DWORD ||
        result_len != sizeof(DWORD))
        result = 0;
    RegCloseKey(hKey);

out:
    return result;
}

static BOOL RegReadString(HKEY hKey, LPCTSTR lpSubkey, LPCTSTR lpValueName,
                          char *str, DWORD *str_len)
{
    HKEY hSubkey;
    LONG qres;
    DWORD key_type;
    BOOL result = FALSE;

    if (RegOpenKeyEx(hKey, lpSubkey, 0, KEY_READ, &hSubkey) != ERROR_SUCCESS)
        goto out;
    qres = RegQueryValueEx(hSubkey, lpValueName, NULL, &key_type, (void *)str,
			   str_len);
    if (qres == ERROR_SUCCESS && key_type == REG_SZ)
        result = TRUE;
    RegCloseKey(hKey);

out:
    return result;
}

static void
windowsOSVersionString(char stringbuf[], size_t stringbuflen)
{
    /* copy OS version to string buffer in 'uname -a' format */
    static const char wcv[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
    char windowsVersion[256] = "?";
    DWORD windowsVersionSz = sizeof(windowsVersion);
    char build[256] = "?";
    DWORD buildSz = sizeof(256);
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    char hostname[256] = "?";
    char identifier[256] = "?";
    DWORD identifierSz = sizeof(identifier);

    dwMajorVersion = RegReadDword(HKEY_LOCAL_MACHINE, wcv,
                                  "CurrentMajorVersionNumber");
    dwMinorVersion = RegReadDword(HKEY_LOCAL_MACHINE, wcv,
                                  "CurrentMinorVersionNumber");
    if (!RegReadString(HKEY_LOCAL_MACHINE, wcv, "CurrentBuildNumber",
                       build, &buildSz))
        RegReadString(HKEY_LOCAL_MACHINE, wcv, "CurrentBuild",
                      build, &buildSz);

    gethostname(hostname, sizeof(hostname));

    RegReadString(HKEY_LOCAL_MACHINE, wcv, "ProductName", windowsVersion,
                  &windowsVersionSz);
    RegReadString(HKEY_LOCAL_MACHINE,
                  "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                  "Identifier", identifier, &identifierSz);

    /* Output is made to look like results from uname -a */
    snprintf(stringbuf, stringbuflen, "Windows %s %d.%d.%s %s %s",
             hostname, (int)dwMajorVersion, (int)dwMinorVersion, build,
             windowsVersion, identifier);
}
#endif /* WIN32 and HAVE_WIN32_PLATFORM_SDK or mingw32 */

