/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 * Copyright (C) 2020, BT PLC
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file main.c
 *
 * Main function for USP Agent
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>

#include "common_defs.h"
#include "mtp_exec.h"
#include "dm_exec.h"
#include "bdc_exec.h"
#include "data_model.h"
#include "dm_access.h"
#include "device.h"
#include "database.h"
#include "sync_timer.h"
#include "cli.h"
#include "os_utils.h"
#include "text_utils.h"
#include "usp_coap.h"
#include "stomp.h"
#include "retry_wait.h"
#include "nu_macaddr.h"
#include "plugin.h"


#ifdef ENABLE_WEBSOCKETS
#include "wsclient.h"
#include "wsserver.h"
#endif

#ifndef REMOVE_USP_SERVICE
#include "usp_service.h"
#endif

#ifdef ENABLE_UDS
#include "uds.h"
#endif

//--------------------------------------------------------------------------------------
// Determine whether libcurl is required by the USP Agent

#ifndef REMOVE_DEVICE_BULKDATA
#undef REQUIRE_CURL
#define REQUIRE_CURL
#endif


#ifdef REQUIRE_CURL
#include <curl/curl.h>
#endif

#ifndef OVERRIDE_MAIN
//--------------------------------------------------------------------------------------
// Array used by the getopt_long() function to parse a command line
// See http://www.gnu.org/s/hello/manual/libc/Getopt-Long-Options.html
// NOTE: When altering this array, make sure that you also alter the short options array as well
static struct option long_options[] =
{
//  long option,   option+argument?,  flag, short option
    {"help",       no_argument,       NULL, 'h'},    // Prints help for command line options
    {"log",        required_argument, NULL, 'l'},    // Sets the destination for the log file (either syslog, stdout or a filename)
    {"dbfile",     required_argument, NULL, 'f'},    // Sets the name of the path to use for the database file
    {"verbose",    required_argument, NULL, 'v'},    // Verbosity level for debug logging
    {"meminfo",    no_argument,       NULL, 'm'},    // Collects and prints information useful to debugging memory leaks
#ifdef HAVE_EXECINFO_H
    {"error",      no_argument,       NULL, 'e'},    // Prints the callstack whenever an error is detected
#endif
    {"prototrace", no_argument,       NULL, 'p'},    // Enables logging of the protocol trace
    {"command",    no_argument,       NULL, 'c'},    // The rest of the command line is a command to invoke on the active USP Agent.
                                                     // Using this option turns this executable into just a CLI for the active USP Agent.
#ifndef REMOVE_DEVICE_SECURITY
    {"authcert",   required_argument, NULL, 'a'},    // Specifies the location of a file containing the client certificate to use authenticating this device
    {"truststore", required_argument, NULL, 't'},    // Specifies the location of a file containing the trust store certificates to use
#endif
    {"resetfile",  required_argument, NULL, 'r'},    // Specifies the location of a text file containing factory reset parameters
    {"interface",  required_argument, NULL, 'i'},    // Specifies the networking interface to use for communications
    {"cli",        required_argument, NULL, 's'},    // Specifies the Unix domain socket file to use for CLI communications
    {"register",   required_argument, NULL, 'R'},    // Specifies the top level data model objects to register. Use of this option runs the Agent as a USP Service,
    {"plugin",     required_argument, NULL, 'x'},    // Specifies the path to a vendor plugin - can be used multiple times to load multiple plugins

    {0, 0, 0, 0}
};

// In the string argument, the colons (after the option) mean that those options require arguments
static char short_options[] = "hl:f:v:a:t:r:i:mepcs:R:x:";
#endif // OVERRIDE_MAIN

//--------------------------------------------------------------------------------------
// Variables set by command line arguments
bool enable_callstack_debug = false;    // Enables printing of the callstack when an error occurs


char *cli_uds_file = CLI_UNIX_DOMAIN_FILE;  // filename path of Unix domain socket used for CLI commands

//--------------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void PrintUsage(char *prog_name);
int MAIN_Start(char *db_file, bool enable_mem_info);
void MAIN_Stop(void);

#ifndef OVERRIDE_MAIN
/*********************************************************************//**
**
** main
**
** Main function
**
** \param   argc - Number of command line arguments
** \param   argv - Array of pointers to command line argument strings
**
** \return  -1 (if this function ever returns, then it will be because of an error)
**
**************************************************************************/
int main(int argc, char *argv[])
{
    int err;
    int c;
    int option_index = 0;
    char *db_file = DEFAULT_DATABASE_FILE;
    bool enable_mem_info = false;

    // Determine a handle for the data model thread (this thread)
    OS_UTILS_SetDataModelThread();

    // Exit if unable to initialise basic subsystems
    USP_LOG_Init();
    USP_ERR_Init();
    err = USP_MEM_Init();
    if (err != USP_ERR_OK)
    {
        return -1;
    }

    // Iterate over all command line options
    while (FOREVER)
    {
        // Parse the next command line option
        c = getopt_long_only(argc, argv, short_options, long_options, &option_index);

        // Exit this loop, if no more options
        if (c == -1)
        {
            break;
        }

        // Determine which option was read this time
        switch (c)
        {
            case 'h':
                PrintUsage(argv[0]);
                exit(0);
                break;

            case 'l':
                // Exit if an error occurred whilst trying to open the log file
                err = USP_LOG_SetFile(optarg);
                if (err != USP_ERR_OK)
                {
                    goto exit;
                }
                break;

            case 'f':
                // Set the location of the database
                db_file = optarg;
                break;

            case 'm':
                // Enable memory info collection
                enable_mem_info = true;
                break;

#ifdef HAVE_EXECINFO_H
            case 'e':
                // Enable callstack printing when an error occurs
                enable_callstack_debug = true;
                break;
#endif

            case 'p':
                // Enable logging of protocol trace
                enable_protocol_trace = true;
                break;

#ifndef REMOVE_DEVICE_SECURITY

            case 'a':
                // Set the location of the client certificate file to use
                auth_cert_file = optarg;
                break;

            case 't':
                // Set the location of the file containing trust store certificates
                usp_trust_store_file = optarg;
                break;
#endif
            case 'r':
                // Set the location of the text file containing the factory reset parameters
                factory_reset_text_file = optarg;
                break;

            case 's':
                // Set the location of the Unix domain socket file to use for the CLI
                cli_uds_file = optarg;
                break;

            case 'R':
#ifndef REMOVE_USP_SERVICE
                // Set the top-level data model objects to register and run this Agent as a USP Service
                usp_service_objects = optarg;
#else
                USP_LOG_Error("ERROR: The -R (--register) option is not supported on builds compiled with REMOVE_USP_SERVICE defined");
                goto exit;
#endif
                break;
            case 'i':
                // Set the networking interface to use for USP communication
                if (nu_ipaddr_is_valid_interface(optarg) != true)
                {
                    usp_log_level = kLogLevel_Error;
                    USP_LOG_Error("ERROR: Network interface '%s' does not exist or has no IP address", optarg);
                    goto exit;
                }
                usp_interface = optarg;
                break;

            case 'v':
                // Verbosity level
                err = TEXT_UTILS_StringToUnsigned(optarg, &usp_log_level);
                if ((err != USP_ERR_OK) || (usp_log_level >= kLogLevel_Max))
                {
                    usp_log_level = kLogLevel_Error;
                    USP_LOG_Error("ERROR: Verbosity level (%s) is invalid or out of range", optarg);
                    goto exit;
                }
                break;

            case 'c':
                // Rest of command line contains a command to send to the active USP Agent
                err = CLI_CLIENT_ExecCommand(argc-optind, &argv[optind], db_file);
                return err;
                break;


            case 'x':
                PLUGIN_Load(optarg);
                break;

            default:
                USP_LOG_Error("ERROR: USP Agent was invoked with the '-%c' option but the code was not compiled in.", c);
                goto exit;
                break;

            case '?':
                usp_log_level = kLogLevel_Error;
                USP_LOG_Error("ERROR: Missing option value");
                goto exit;
                break;
        }
    }

    // Print a warning for any remaining command line arguments
    if (optind < argc)
    {
        USP_LOG_Error("WARNING: unknown command line arguments:-");
        while (optind < argc)
        {
            USP_LOG_Error("   %s", argv[optind++]);
        }
    }

    // Following debug is only logged when running as a daemon (not when running as CLI client).
    syslog(LOG_INFO, "USP Agent starting...");

    // Sleep until other services which USP Agent uses (eg DNS) are running
    // (ideally USP Agent should be started when the services are running, rather than sleeping here. But sometimes, there is no easy way to ensure this).
    if (DAEMON_START_DELAY_MS > 0)
    {
        usleep(DAEMON_START_DELAY_MS*1000);
    }


    // Exit if unable to start USP Agent
    err = MAIN_Start(db_file, enable_mem_info);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to spawn off a thread to service the MTPs
#ifndef DISABLE_STOMP
    err = OS_UTILS_CreateThread("MTP_STOMP", MTP_EXEC_StompMain, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

#ifdef ENABLE_COAP
    err = OS_UTILS_CreateThread("MTP_CoAP", MTP_EXEC_CoapMain, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

#ifdef ENABLE_MQTT
    err = OS_UTILS_CreateThread("MTP_MQTT", MTP_EXEC_MqttMain, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

#ifdef ENABLE_WEBSOCKETS
    err = OS_UTILS_CreateThread("MTP_WSClient", WSCLIENT_Main, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    err = OS_UTILS_CreateThread("MTP_WSServer", WSSERVER_Main, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

#ifdef ENABLE_UDS
    err = OS_UTILS_CreateThread("MTP_UDS", MTP_EXEC_UdsMain, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

#ifndef REMOVE_DEVICE_BULKDATA
    // Exit if unable to spawn off a thread to perform bulk data collection posts
    err = OS_UTILS_CreateThread("BulkDataColl", BDC_EXEC_Main, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
#endif

    // Run the data model main loop of USP Agent (this function does not return)
    DM_EXEC_Main(NULL);

exit:
    // If the code gets here, an error occurred
    USP_LOG_Error("USP Agent aborted unexpectedly");
    return -1;
}
#endif

/*********************************************************************//**
**
** MAIN_Start
**
** Initializes and starts USP Agent
**
** \param   db_file - pointer to name of USP Agent's database file to open
** \param   enable_mem_info - Set to true if memory debugging info should be collected
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MAIN_Start(char *db_file, bool enable_mem_info)
{
    int err;
#ifdef REQUIRE_CURL
    CURLcode curl_err;

#if (LIBCURL_VERSION_NUM >= 0x073800)
    CURLsslset curl_sslset_err;

    // Exit if unable to select SSL backend for curl
    // This is necessary as curl supports multiple SSL backends, the default of which might not be OpenSSL
    curl_sslset_err = curl_global_sslset(CURLSSLBACKEND_OPENSSL, NULL, NULL);
    if (curl_sslset_err != CURLSSLSET_OK)
    {
        USP_LOG_Error("%s: Failed to select OpenSSL backend for libcurl (curl_global_sslset err=%d)", __FUNCTION__, curl_sslset_err);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif

    // Exit if unable to initialise libraries which need to be initialised when running single threaded
    curl_err = curl_global_init(CURL_GLOBAL_ALL);
    if (curl_err != 0)
    {
        USP_LOG_Error("%s: curl_global_init() failed (curl_err=%d)", __FUNCTION__, curl_err);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif // REQUIRE_CURL

    SYNC_TIMER_Init();

    // Turn off SIGPIPE, since we use non-blocking connections and would prefer to get the EPIPE error
    // NOTE: If running USP Agent in GDB: GDB ignores this code and will still generate SIGPIPE
    signal(SIGPIPE, SIG_IGN);


    // Exit if an error occurred when initialising the database
    err = DATABASE_Init(db_file);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if an error occurred when initialising any of the the message queues used by the threads
    err = DM_EXEC_Init();
    err |= MTP_EXEC_Init();
#ifndef REMOVE_DEVICE_BULKDATA
    err |= BDC_EXEC_Init();
#endif
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Initialise the random number generator seeds
    RETRY_WAIT_Init();

    // Exit if unable to add all schema paths to the data model
    err = DATA_MODEL_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Start logging memory usage from now on (since the static data model schema allocations have completed)
    if (enable_mem_info)
    {
        USP_MEM_StartCollection();
    }

    // Exit if unable to start the datamodel objects
    err = DATA_MODEL_Start();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Start all MTP connections
    DEVICE_CONTROLLER_StartAllMtpClients();

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** MAIN_Stop
**
** Frees all memory and closes all sockets and file handles
** Called from the MTP thread
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void MAIN_Stop(void)
{
    // Free all memory used by USP Agent
    DM_EXEC_Destroy();
#ifdef REQUIRE_CURL
    curl_global_cleanup();
#endif
    USP_MEM_Destroy();
}

/*********************************************************************//**
**
** PrintUsage
**
** Prints the command line options for this program
**
** \param   prog_name - name of this executable from command line
**
** \return  None
**
**************************************************************************/
void PrintUsage(char *prog_name)
{
    char *p;
    char *name;

    // Strip off any leading directories from the executable path
    p = strrchr(prog_name, '/');
    name = (p == NULL) ? prog_name : &p[1];

    printf("USAGE: %s options\n", name);
    printf("--help (-h)       Displays this help\n");
    printf("--log (-l)        Sets the destination for debug logging. Default is 'stdout'. Can also use 'syslog' or a filename\n");
    printf("--dbfile (-f)     Sets the path of the file to store the database in (default=%s)\n", DEFAULT_DATABASE_FILE);
    printf("--verbose (-v)    Sets the debug verbosity log level: 0=Off, 1=Error(default), 2=Warning, 3=Info\n");
    printf("--prototrace (-p) Enables trace logging of the USP protocol messages\n");
    printf("--cli (-s)        Sets the path of the Unix domain socket file used for CLI communications\n");
#ifndef REMOVE_DEVICE_SECURITY
    printf("--authcert (-a)   Sets the path of the PEM formatted file containing a client certificate and private key to authenticate this device with\n");
    printf("--truststore (-t) Sets the path of the PEM formatted file containing trust store certificates\n");
#endif
    printf("--resetfile (-r)  Sets the path of the text file containing factory reset parameters\n");
    printf("--interface (-i)  Sets the name of the networking interface to use for USP communication\n");
    printf("--meminfo (-m)    Collects and prints information useful to debugging memory leaks\n");
#ifndef REMOVE_USP_SERVICE
    printf("--register (-R)   Sets the top-level data model objects to register when acting as a USP Service\n");
#endif
#ifdef HAVE_EXECINFO_H
    printf("--error (-e)      Enables printing of the callstack whenever an error is detected\n");
#endif
    printf("--command (-c)    Sends a CLI command to the running USP Agent and prints the response\n");
    printf("                  To get a list of all CLI commands use '-c help'\n");
    printf("--plugin (-x)     Specifies the path to a shared object vendor layer plug-in\n");
    printf("\n");
}

