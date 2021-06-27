/*
 * Copyright (c) 2020, NLNet Labs, Sinodun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the names of the copyright holders nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <stdarg.h>
#include <stdio.h>

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "configfile.h"
#include "log.h"
#include "server.h"
#include "util.h"

#include "service.h"

#include "windowsservice.h"

static void winerr(const TCHAR* operation, DWORD err)
{
        char msg[512];

        if ( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           err,
                           0,
                           msg,
                           sizeof(msg),
                           NULL) == 0 )
                fprintf(stderr, "Error: %s: errno=%d\n", operation, err);
        else
                fprintf(stderr, "Error: %s: %s\n", operation, msg);
        exit(EXIT_FAILURE);
}

static void winlasterr(const TCHAR* operation)
{
        winerr(operation, GetLastError());
}

// #pragma comment(lib, "advapi32.lib")

#define SVCNAME TEXT("Stubby")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;
int                     dnssec_validation = 0;

VOID SvcInstall(void);
VOID SvcRemove(void);
VOID SvcService(void);
VOID SvcStart(int loglevel, const char* config_file);
VOID SvcStop(void);
int SvcStatus(void);
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(LPTSTR);


void windows_service_command(const TCHAR* arg, int loglevel, const char* config_file)
{
        if ( lstrcmpi(arg, TEXT("install")) == 0 )
                SvcInstall();
        else if ( lstrcmpi(arg, TEXT("remove")) == 0 )
                SvcRemove();
        else if ( lstrcmpi(arg, TEXT("service")) == 0 )
                SvcService();
        else if ( lstrcmpi(arg, TEXT("start")) == 0 )
                SvcStart(loglevel, config_file);
        else if ( lstrcmpi(arg, TEXT("stop")) == 0 )
                SvcStop();
        else if ( lstrcmpi(arg, TEXT("status")) == 0 )
                exit(SvcStatus());
        else
        {
                fprintf(stderr, "Unknown Windows option '%s'\n", arg);
                exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
}

void report_verror(getdns_loglevel_type level, const char *fmt, va_list ap)
{
        char buf[256];
        HANDLE hEventSource;
        LPCTSTR lpszStrings[1];
        WORD eventType;
        DWORD eventId;

        hEventSource = RegisterEventSource(NULL, SVCNAME);
        if ( hEventSource == NULL )
                return;

        switch (level)
        {
        case GETDNS_LOG_EMERG:
                eventType = EVENTLOG_ERROR_TYPE;
                eventId = SVC_EMERGENCY;
                break;

        case GETDNS_LOG_ALERT:
                eventType = EVENTLOG_ERROR_TYPE;
                eventId = SVC_ALERT;
                break;

        case GETDNS_LOG_CRIT:
                eventType = EVENTLOG_ERROR_TYPE;
                eventId = SVC_CRITICAL;
                break;

        case GETDNS_LOG_ERR:
                eventType = EVENTLOG_ERROR_TYPE;
                eventId = SVC_ERROR;
                break;

        case GETDNS_LOG_WARNING:
                eventType = EVENTLOG_WARNING_TYPE;
                eventId = SVC_WARNING;
                break;

        case GETDNS_LOG_NOTICE:
                eventType = EVENTLOG_WARNING_TYPE;
                eventId = SVC_NOTICE;
                break;

        case GETDNS_LOG_INFO:
                eventType = EVENTLOG_INFORMATION_TYPE;
                eventId = SVC_INFO;
                break;

        default:
                eventType = EVENTLOG_INFORMATION_TYPE;
                eventId = SVC_DEBUG;
                break;

        }

        vsnprintf(buf, sizeof(buf), fmt, ap);

        lpszStrings[0] = buf;

        ReportEvent(hEventSource,        // event log handle
                    eventType,           // event type
                    0,                   // event category
                    eventId,             // event identifier
                    NULL,                // no security identifier
                    1,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
}

void report_vlog(void *userarg, uint64_t system,
                 getdns_loglevel_type level,
                 const char *fmt, va_list ap)
{
        (void) userarg;
        (void) system;
        report_verror(level, fmt, ap);
}


VOID report_winerr(LPTSTR operation)
{
        char msg[512];
        DWORD err = GetLastError();

        if ( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           err,
                           0,
                           msg,
                           sizeof(msg),
                           NULL) == 0 )
                stubby_error("Error: %s: errno=%d\n", operation, err);
        else
                stubby_error("Error: %s: %s\n", operation, msg);
}

VOID report_getdnserr(LPTSTR operation, getdns_return_t r)
{
        stubby_error("%s: %s", operation, stubby_getdns_strerror(r));
}

VOID SvcService(void)
{
        SERVICE_TABLE_ENTRY DispatchTable[] = {
                { SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain },
                { NULL, NULL }
        };

        // This call returns when the service has stopped.
        // The process should simply terminate when the call returns.
        if ( !StartServiceCtrlDispatcher(DispatchTable) )
        {
                report_winerr("StartServiceCtrlDispatcher");
        }
}

static void createRegistryEntries(const TCHAR* path)
{
        TCHAR buf[512];
        HKEY hkey;
        DWORD t;
        LSTATUS status;

        snprintf(buf, sizeof(buf), "SYSTEM\\CurrentControlSet\\Services"
                 "\\EventLog\\Application\\%s", SVCNAME);
        status = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                buf,       // Key
                0,         // Reserved
                NULL,      // Class
                REG_OPTION_NON_VOLATILE, // Info on file
                KEY_WRITE, // Access rights
                NULL,      // Security descriptor
                &hkey,     // Result
                NULL       // Don't care if it exists
                );
        if ( status != ERROR_SUCCESS )
                winerr("Create registry key", status);

        status = RegSetValueEx(
                hkey,                      // Key handle
                "EventMessageFile",        // Value name
                0,                         // Reserved
                REG_EXPAND_SZ,             // It's a string
                (const BYTE*) path,        // with this value
                (DWORD)(strlen(path) + 1)  // and this long
                );
        if ( status != ERROR_SUCCESS )
        {
                RegCloseKey(hkey);
                winerr("Set EventMessageFile", status);
        }

        status = RegSetValueEx(
                hkey,                      // Key handle
                "CategoryMessageFile",     // Value name
                0,                         // Reserved
                REG_EXPAND_SZ,             // It's a string
                (const BYTE*) path,        // with this value
                (DWORD)(strlen(path) + 1)  // and this long
                );
        if ( status != ERROR_SUCCESS )
        {
                RegCloseKey(hkey);
                winerr("Set CategoryMessageFile", status);
        }

        /* event types */
        t = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
        status = RegSetValueEx(
                hkey,                      // Key handle
                "TypesSupported",          // Value name
                0,                         // Reserved
                REG_DWORD,                 // It's a DWORD
                (const BYTE*) &t,          // with this value
                sizeof(t)                  // and this long
                );
        if ( status != ERROR_SUCCESS )
        {
                RegCloseKey(hkey);
                winerr("Set TypesSupported", status);
        }

        t = 1;
        status = RegSetValueEx(
                hkey,                      // Key handle
                "CategoryCount",           // Value name
                0,                         // Reserved
                REG_DWORD,                 // It's a DWORD
                (const BYTE*) &t,          // with this value
                sizeof(t)                  // and this long
                );
        if ( status != ERROR_SUCCESS )
        {
                RegCloseKey(hkey);
                winerr("Set CategoryCount", status);
        }
        RegCloseKey(hkey);
}

static void deleteRegistryEntries(void)
{
        HKEY hkey;
        LSTATUS status;

        status = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                "SYSTEM\\CurrentControlSet\\Services"
                "\\EventLog\\Application",
                0,         // Reserved
                NULL,      // Class
                REG_OPTION_NON_VOLATILE, // Info on file
                DELETE,    // Access rights
                NULL,      // Security descriptor
                &hkey,     // Result
                NULL       // Don't care if it exists
                );
        if ( status != ERROR_SUCCESS )
                winerr("Create registry key", status);

        status = RegDeleteKey(hkey, SVCNAME);
        if ( status != ERROR_SUCCESS )
        {
                RegCloseKey(hkey);
                winerr("Delete registry key", status);
        }
        RegCloseKey(hkey);
}

VOID SvcInstall(void)
{
        SC_HANDLE schSCManager;
        SC_HANDLE schService;
        TCHAR modpath[MAX_PATH];
        TCHAR respath[MAX_PATH];
        const TCHAR ARG[] = "-w service";
        TCHAR cmd[MAX_PATH + 3 + sizeof(ARG)];
        SERVICE_DESCRIPTION description;
        HMODULE stubres;

        stubres = LoadLibrary("stubres");
        if ( stubres == NULL )
                winlasterr("Loading stubres.dll");
        if( !GetModuleFileName(stubres, respath, sizeof(respath)) )
                winlasterr("GetModuleFileName");
        FreeLibrary(stubres);
        if( !GetModuleFileName(NULL, modpath, sizeof(modpath)) )
                winlasterr("GetModuleFileName");
        snprintf(cmd, sizeof(cmd), "\"%s\" %s", modpath, ARG);

        createRegistryEntries(respath);

        schSCManager = OpenSCManager(
                NULL,                    // local computer
                NULL,                    // ServicesActive database
                GENERIC_WRITE);          // modify service install

        if (NULL == schSCManager)
        {
                deleteRegistryEntries();
                winlasterr("Open service manager");
        }

        schService = CreateService(
                schSCManager,              // SCM database
                SVCNAME,                   // name of service
                "Stubby DNS Privacy stub resolver", // service name to display
                SERVICE_ALL_ACCESS,        // desired access
                SERVICE_WIN32_OWN_PROCESS, // service type
                SERVICE_DEMAND_START,      // start type
                SERVICE_ERROR_NORMAL,      // error control type
                cmd,                       // path to service's binary
                NULL,                      // no load ordering group
                NULL,                      // no tag identifier
                NULL,                      // no dependencies
                NULL,                      // LocalSystem account
                NULL);                     // no password

        if (schService == NULL)
        {
                CloseServiceHandle(schSCManager);
                deleteRegistryEntries();
                winlasterr("Create service");
        }

        description.lpDescription = TEXT("Enable performing DNS name lookups over encrypted channels.");
        ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION,
                             (LPVOID) &description);

        printf("Service installed successfully\n");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
}

VOID SvcRemove(void)
{
        SC_HANDLE schSCManager;
        SC_HANDLE schService;

        schSCManager = OpenSCManager(
                NULL,                    // local computer
                NULL,                    // ServicesActive database
                GENERIC_WRITE);          // modify service install

        if (NULL == schSCManager)
                winlasterr("Open service manager");

        schService = OpenService(
                schSCManager,              // SCM database
                SVCNAME,                   // name of service
                DELETE);                   // intention

        if (schService == NULL)
        {
                CloseServiceHandle(schSCManager);
                winlasterr("Open service");
        }

        if ( DeleteService(schService) == 0 )
        {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                winlasterr("Delete service");
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        deleteRegistryEntries();

        printf("Service removed successfully\n");
}

VOID SvcStart(int loglevel, const char* config_file)
{
        SC_HANDLE schSCManager;
        SC_HANDLE schService;

        schSCManager = OpenSCManager(
                NULL,                    // local computer
                NULL,                    // ServicesActive database
                STANDARD_RIGHTS_WRITE);  // send commands

        if (NULL == schSCManager)
                winlasterr("Open service manager");

        schService = OpenService(
                schSCManager,              // SCM database
                SVCNAME,                   // name of service
                SERVICE_START);            // intention

        if (schService == NULL)
        {
                CloseServiceHandle(schSCManager);
                winlasterr("Open service");
        }

        TCHAR loglevelstr[2];
        loglevelstr[0] = '0' + loglevel;
        loglevelstr[1] = '\0';

        LPCTSTR args[2] = {
                loglevelstr,
                config_file
        };
        int nargs = config_file ? 2 : 1;

        if ( StartService(schService, nargs, args) == 0 )
        {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                winlasterr("Start service");
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        printf("Service started successfully\n");
}

VOID SvcStop(void)
{
        SC_HANDLE schSCManager;
        SC_HANDLE schService;

        schSCManager = OpenSCManager(
                NULL,                    // local computer
                NULL,                    // ServicesActive database
                STANDARD_RIGHTS_WRITE);  // send commands

        if (NULL == schSCManager)
                winlasterr("Open service manager");

        schService = OpenService(
                schSCManager,              // SCM database
                SVCNAME,                   // name of service
                SERVICE_STOP);             // intention

        if (schService == NULL)
        {
                CloseServiceHandle(schSCManager);
                winlasterr("Open service");
        }

        SERVICE_STATUS st;

        if ( ControlService(
                     schService,                // service
                     SERVICE_CONTROL_STOP,      // action
                     &st                        // result
                     ) == 0 )
        {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                winlasterr("Stop service");
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        printf("Service stopped successfully\n");
}

int SvcStatus(void)
{
        SC_HANDLE schSCManager;
        SC_HANDLE schService;

        schSCManager = OpenSCManager(
                NULL,                    // local computer
                NULL,                    // ServicesActive database
                STANDARD_RIGHTS_READ);   // Just read

        if (NULL == schSCManager)
                winlasterr("Open service manager");

        schService = OpenService(
                schSCManager,              // SCM database
                SVCNAME,                   // name of service
                SERVICE_QUERY_STATUS);     // intention

        if (schService == NULL)
        {
                CloseServiceHandle(schSCManager);
                winlasterr("Open service");
        }

        SERVICE_STATUS st;

        if ( QueryServiceStatus(
                     schService,                // service
                     &st                        // result
                     ) == 0 )
        {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                winlasterr("Query service");
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        switch (st.dwCurrentState)
        {
        case SERVICE_RUNNING:
                printf("Running");
                return 0;

        case SERVICE_START_PENDING:
                printf("Start pending");
                return 1;

        case SERVICE_STOP_PENDING:
                printf("Stop pending");
                return 2;

        case SERVICE_STOPPED:
                printf("Stopped");
                return 3;

        default:
                printf("Unexpected status: %d", st.dwCurrentState);
                return 99;
        }
}

static void set_startup(DWORD start_type) {

        SC_HANDLE schSCManager;
        SC_HANDLE schService;

        schSCManager = OpenSCManager(
                NULL,                    // local computer
                NULL,                    // ServicesActive database
                GENERIC_WRITE);           // modify service

        if (NULL == schSCManager)
                winlasterr("Open service manager");

        schService = OpenService(
                schSCManager,              // SCM database
                SVCNAME,                   // name of service
                SERVICE_CHANGE_CONFIG);     // intention

        if (schService == NULL)
        {
                CloseServiceHandle(schSCManager);
                winlasterr("Open service");
        }


         if (! ChangeServiceConfig( 
                schService,                // service handle
                SERVICE_NO_CHANGE,         // service type
                start_type,                // start type
                SERVICE_NO_CHANGE,         // error control type
                NULL,                      // path to service's binary
                NULL,                      // no load ordering group
                NULL,                      // no tag identifier
                NULL,                      // no dependencies
                NULL,                      // account name: no change 
                NULL,                      // password: no change 
                NULL) )                    // display name: no change
        {
                stubby_debug("ChangeServiceConfig failed (%d)\n", GetLastError()); 
        }
        //else stubby_debug("Service start type changed successfully.\n");               
}

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
        stubby_set_log_funcs(report_verror, report_vlog);

        gSvcStatusHandle = RegisterServiceCtrlHandler(
                SVCNAME,
                SvcCtrlHandler);

        if( !gSvcStatusHandle )
        {
                report_winerr("RegisterServiceCtrlHandler");
                return;
        }

        gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        gSvcStatus.dwServiceSpecificExitCode = 0;
        ReportSvcStatus(SERVICE_START_PENDING, 0, 3000);

        set_startup(SERVICE_AUTO_START);
        SvcInit(dwArgc, lpszArgv);
}

static void timeout_callback(void* userarg)
{
        (void) userarg;
}

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
        getdns_context *context = NULL;
        getdns_return_t r;
        int more = 1;
        getdns_eventloop *eventloop;
        getdns_eventloop_event eventloop_event;
        int can_block;

        int validate_dnssec;
        const char* config_file = NULL;

        ghSvcStopEvent = CreateEvent(
                NULL,    // default security attributes
                TRUE,    // manual reset event
                FALSE,   // not signaled
                NULL);   // no name

        if ( ghSvcStopEvent == NULL)
        {
                ReportSvcStatus(SERVICE_STOPPED, 1, 0);
                return;
        }

        ReportSvcStatus(SERVICE_START_PENDING, 0, 1000);
        if ( ( r = getdns_context_create(&context, 1) ) ) {
                stubby_error("Create context failed: %s", stubby_getdns_strerror(r));
                ReportSvcStatus(SERVICE_STOPPED, 1, 0);
                CloseHandle(ghSvcStopEvent);
                ghSvcStopEvent = NULL;
                return;
        }

        if ( dwArgc > 1 )
                stubby_set_getdns_logging(context, lpszArgv[1][0] - '0');
        if ( dwArgc > 2 )
                config_file = lpszArgv[2];
        else
                // have the service default to a dedicated file in its own directory
                config_file = system_service_config_file();

        init_config(context);
        ReportSvcStatus(SERVICE_START_PENDING, 0, 1010);
        stubby_debug("Starting %s with config file %s", STUBBY_PACKAGE_STRING, config_file);
        if ( !read_config(context, config_file, &validate_dnssec) ) {
                ReportSvcStatus(SERVICE_STOPPED, 1, 0);
                goto tidy_and_exit;
        }
        ReportSvcStatus(SERVICE_START_PENDING, 0, 1020);
        if ( !server_listen(context, dnssec_validation) ) {
                ReportSvcStatus(SERVICE_STOPPED, 1, 0);
                goto tidy_and_exit;
        }

        ReportSvcStatus(SERVICE_START_PENDING, 0, 1030);
        r = getdns_context_get_eventloop(context, &eventloop);
        if ( r != GETDNS_RETURN_GOOD ) {
                report_getdnserr("Get event loop", r);
                ReportSvcStatus(SERVICE_STOPPED, 1, 0);
                goto tidy_and_exit;
        }

        ReportSvcStatus(SERVICE_RUNNING, 0, 0);

        eventloop_event.userarg = NULL;
        eventloop_event.read_cb = eventloop_event.write_cb = NULL;
        eventloop_event.timeout_cb = timeout_callback;

        for(;;)
        {
                switch ( WaitForSingleObject(ghSvcStopEvent, 0) )
                {
                case WAIT_TIMEOUT:
                        break;

                case WAIT_ABANDONED:
                case WAIT_FAILED:
                        more = 0;
                        report_winerr("WaitForSingleObject");
                        break;

                default:
                        more = 0;
                        stubby_debug("Stop object signalled");
                        set_startup(SERVICE_DEMAND_START);
                        break;
                }

                if ( !more )
                        break;

                /*
                 * Run the getdns eventloop blocking, but ensure we go back
                 * to check the event after 0.25s. In case we can't set
                 * a timeout, run non-blocking until we can.
                 */
                r = eventloop->vmt->schedule(eventloop, -1, 250, &eventloop_event);
                can_block = ( r == GETDNS_RETURN_GOOD );
                if ( !can_block )
                        report_getdnserr("Set event timeout", r);
                eventloop->vmt->run_once(eventloop, can_block);
                if ( can_block ) {
                        r = eventloop->vmt->clear(eventloop, &eventloop_event);
                        if ( r != GETDNS_RETURN_GOOD )
                                report_getdnserr("Clear event timeout", r);
                }
        }
        ReportSvcStatus(SERVICE_STOPPED, 0, 0);

tidy_and_exit:
        getdns_context_destroy(context);
        delete_config();
        CloseHandle(ghSvcStopEvent);
        ghSvcStopEvent = NULL;
}

VOID ReportSvcStatus(DWORD dwCurrentState,
                     DWORD dwWin32ExitCode,
                     DWORD dwWaitHint)
{
        static DWORD dwCheckPoint = 1;

        gSvcStatus.dwCurrentState = dwCurrentState;
        gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
        gSvcStatus.dwWaitHint = dwWaitHint;

        if (dwCurrentState == SERVICE_START_PENDING)
                gSvcStatus.dwControlsAccepted = 0;
        else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

        if ( (dwCurrentState == SERVICE_RUNNING) ||
             (dwCurrentState == SERVICE_STOPPED) )
                gSvcStatus.dwCheckPoint = 0;
        else gSvcStatus.dwCheckPoint = dwCheckPoint++;

        SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
        switch(dwCtrl)
        {
        case SERVICE_CONTROL_STOP:
                ReportSvcStatus(SERVICE_STOP_PENDING, 0, 0);
                if ( !SetEvent(ghSvcStopEvent) )
                        stubby_error("SetEvent failed");
                break;

        case SERVICE_CONTROL_INTERROGATE:
                break;

        default:
                break;
        }
}
