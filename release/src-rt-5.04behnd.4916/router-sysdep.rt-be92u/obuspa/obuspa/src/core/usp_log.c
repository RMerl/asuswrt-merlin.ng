/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file usp_log.c
 *
 * Functions through which all debug prints are passed
 *
 */

#include "vendor_defs.h"   // For REMOVE_DEVICE_SECURITY

#ifndef REMOVE_DEVICE_SECURITY
#include <openssl/err.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <dlfcn.h>

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#include "common_defs.h"
#include "cli.h"
#include "usp_api.h"
#include "data_model.h"  // for vendor_hook_callbacks

//------------------------------------------------------------------------------------
// File to send logging output to
static FILE *log_fd;

//------------------------------------------------------------------------------------
// Global variables controlling logging
log_level_t usp_log_level = kLogLevel_Error;    // Verbosity level
bool enable_protocol_trace = false;             // Whether protocol tracing should be sent out or not

//------------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void LogMessageToFile(log_level_t log_level, FILE *fd, const char *str);
void CloseLog(void);

/*********************************************************************//**
**
** USP_LOG_Init
**
** Initialises logging. Default to logging to stdout
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_LOG_Init(void)
{
    // Default to logging to stdout
    log_fd = stdout;
}

/*********************************************************************//**
**
** USP_LOG_SetFile
**
** Sets the file to log to (stdout, syslog, filesystem file)
**
** \param   file - Name of file to log to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_LOG_SetFile(const char *file)
{
    // Close the current log file
    CloseLog();

    // Exit if syslog
    if ((file == NULL) || strcmp(file, "syslog")==0)
    {
        log_fd = NULL;
        return USP_ERR_OK;
    }

    // Exit if stdout
    if (strcmp(file, "stdout")==0)
    {
        log_fd = stdout;
        return USP_ERR_OK;
    }

    // Otherwise open the named file
    log_fd = fopen(file, "w");
    if (log_fd == NULL)
    {
        USP_ERR_ERRNO("fopen", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_LOG_Callstack
**
** Prints the callstack to the current log stream
**
** \param   None
**
** \return  None
**
**************************************************************************/
#ifdef HAVE_EXECINFO_H
void USP_LOG_Callstack(void)
{
    #define MAX_CALLSTACK  30
    void *callstack[MAX_CALLSTACK];
    int stack_size;
    int i;
    int symbols_found;
    const char *func_name;
    Dl_info info;
    int indent;

    // Get program counter return addresses of all functions in the callstack
    stack_size = backtrace(callstack, NUM_ELEM(callstack));

    // Convert all addresses to function name
    USP_LOG_Info("Callstack is:-");
    for (i=stack_size-3; i>0; i--)          // Minus 3 because we want to start at main() - not callstack prior to main()
    {
        symbols_found = dladdr(callstack[i], &info);
        indent = stack_size-3-i;
        if (symbols_found)
        {
            func_name = (info.dli_sname != NULL) ? info.dli_sname : "Unknown";
            USP_LOG_Info(" %*s%s()", indent, "", func_name);
        }
        else
        {
            USP_LOG_Info(" %*sUnable to dladdr(%p)", indent, "", callstack[i]);
        }
    }
}
#else
void USP_LOG_Callstack(void)
{
}
#endif

/*********************************************************************//**
**
** USP_LOG_HexBuffer
**
** Logs the contents of the specified buffer
**
** \param   title - pointer to string to print before the buffer contents
** \param   buf - pointer to buffer to print out
** \param   len - length of buffer to print out
**
** \return  None
**
**************************************************************************/
void USP_LOG_HexBuffer(const char *title, const unsigned char *buf, int len)
{
    int i;
    int residual;
    int num_rows;
    int j;

    #define NUM_COLUMNS 16      // NOTE: If you change this then you also need to change the row printf, and possibly the switch statement
    residual = len % NUM_COLUMNS;
    num_rows = len / NUM_COLUMNS;

    USP_LOG_Info("%s", title);
    for (i=0; i<num_rows; i++)
    {
        j = i*NUM_COLUMNS;
        USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, ",
                      buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                      buf[j+8], buf[j+9], buf[j+10], buf[j+11], buf[j+12], buf[j+13], buf[j+14], buf[j+15] );
    }

    j = num_rows*NUM_COLUMNS;
    switch(residual)
    {
        case 0:
            // Nothing to do
            break;

        case 1:
            USP_LOG_Info("0x%02x,", buf[j] );

            break;

        case 2:
            USP_LOG_Info("0x%02x, 0x%02x,",
                          buf[j], buf[j+1] );
            break;

        case 3:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2] );
            break;

        case 4:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3] );
            break;

        case 5:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4] );
            break;

        case 6:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5] );
            break;

        case 7:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6] );
            break;

        case 8:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7] );
            break;

        case 9:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                          buf[j+8] );
            break;

        case 10:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                          buf[j+8], buf[j+9] );
            break;

        case 11:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                          buf[j+8], buf[j+9], buf[j+10] );
            break;

        case 12:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                          buf[j+8], buf[j+9], buf[j+10], buf[j+11] );
            break;

        case 13:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                          buf[j+8], buf[j+9], buf[j+10], buf[j+11], buf[j+12] );
            break;

        case 14:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                          buf[j+8], buf[j+9], buf[j+10], buf[j+11], buf[j+12], buf[j+13] );
            break;

        case 15:
            USP_LOG_Info("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,",
                          buf[j], buf[j+1], buf[j+2], buf[j+3], buf[j+4], buf[j+5], buf[j+6], buf[j+7],
                          buf[j+8], buf[j+9], buf[j+10], buf[j+11], buf[j+12], buf[j+13], buf[j+14] );
            break;
    }
}

#ifndef REMOVE_DEVICE_SECURITY
/*********************************************************************//**
**
** USP_LOG_ErrorSSL
**
** Logs the cause of the SSL error
**
** \param   func_name - name of the function in which the error occurred
** \param   failure_string - operation being performed when the error occurred
** \param   ret - value returned from SSL_read() or SSL_write()
** \param   err - error
**
** \return  USP_ERR_OK if no error occurred
**
**************************************************************************/
void USP_LOG_ErrorSSL(const char *func_name, const char *failure_string, int ret, int err)
{
    char ssl_str[128] = {0};  // OpenSSL requires at least 120 bytes in this buffer
    char errno_str[128] = {0};
    long ssl_errno;
    char *str;

    str = USP_ERR_ToString(errno, errno_str, sizeof(errno_str));
    ssl_errno = ERR_get_error();
    ERR_error_string_n(ssl_errno, ssl_str, sizeof(ssl_str));
    USP_LOG_Warning("%s: %s: SSL ret=%d, error=%d, errno=%d (%s), ssl err=%s",
              func_name, failure_string, ret, err, errno, str, ssl_str);
}
#endif

/*********************************************************************//**
**
** USP_LOG_String
**
** Logs the specified string, splitting it on newlines
** This function is for printing very long buffers, which would get truncated by the normal logging functionality
**
** \param   log_level - severity level of the log
** \param   log_type - type of information which the string contains
** \param   str - pointer to string to log
**
** \return  None
**
**************************************************************************/
void USP_LOG_String(log_level_t log_level, log_type_t log_type, char *str)
{
    char *start;
    char *p;

    // Iterate over the whole of the string calling USP_LOG_Puts() for each line
    p = str;
    start = p;
    while (*p != '\0')
    {
        if (*p == '\n')
        {
            *p = '\0';          // Temporarily truncate the string
            USP_LOG_Puts(log_level, log_type, start);
            *p = '\n';          // Restore the string
            start = &p[1];      // Next line starts at the next character
        }

        // Move to next character in the string
        p++;
    }

    // Print any remaining characters on the last line
    if (*start != '\0')
    {
        USP_LOG_Puts(log_level, log_type, start);
    }
}

/*********************************************************************//**
**
** USP_LOG_Printf
**
** Logs the specified message
** This function deals with the varargs aspects of the message before calling USP_LOG_Puts()
**
** \param   log_level - severity level of the log
** \param   log_type - type of information which the message contains
** \param   fmt - printf style format
**
** \return  None
**
**************************************************************************/
void USP_LOG_Printf(log_level_t log_level, log_type_t log_type, const char *fmt, ...)
{
    va_list ap;
    char buf[USP_LOG_MAXLEN];
    int chars_written;

    // Print the message to the buffer
    va_start(ap, fmt);
    chars_written = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    // Ensure that if the log message has been truncated, that it is reported in the log
    if (chars_written >= sizeof(buf)-1)
    {
        #define TRUNCATED_STR "\n...[truncated]..."
        memcpy(&buf[sizeof(buf)-sizeof(TRUNCATED_STR)], TRUNCATED_STR, sizeof(TRUNCATED_STR));
    }

    USP_LOG_Puts(log_level, log_type, buf);
}

/*********************************************************************//**
**
** USP_LOG_Puts
**
** Logs the specified message to the correct destination, based on its type
**
** \param   log_level - severity level of the log
** \param   log_type - type of information which the string contains
** \param   str - pointer to string to log
**
** \return  None
**
**************************************************************************/
void USP_LOG_Puts(log_level_t log_level, log_type_t log_type, const char *str)
{
    switch(log_type)
    {
        case kLogType_Debug:
            if (dump_to_cli)
            {
                CLI_SERVER_SendResponse(str);
                CLI_SERVER_SendResponse("\n");
            }
            else
            {
                LogMessageToFile(log_level, log_fd, str);
            }
            break;

        case kLogType_Dump:
            if (dump_to_cli)
            {
                CLI_SERVER_SendResponse(str);
                CLI_SERVER_SendResponse("\n");
            }
            else
            {
                LogMessageToFile(log_level, log_fd, str);
            }
            break;

        case kLogType_Protocol:
            if (enable_protocol_trace)
            {
                LogMessageToFile(log_level, log_fd, str);
            }
            break;
    }
}

/*********************************************************************//**
**
** LogLevelToSyslogSeverity
**
** Returns the matching syslog severity level
**
** \param   log_level - severity level of the log
**
** \return  matching syslog severity level
**
**************************************************************************/
int LogLevelToSyslogSeverity(log_level_t log_level)
{
    int syslog_level = LOG_ERR;

    switch (log_level)
    {
        case kLogLevel_Trace:
        case kLogLevel_Debug:
            syslog_level = LOG_DEBUG;
            break;

        case kLogLevel_Info:
            syslog_level = LOG_INFO;
            break;

        case kLogLevel_Warning:
            syslog_level = LOG_WARNING;
            break;

        default:
        case kLogLevel_Error:
            syslog_level = LOG_ERR;
            break;
    }

    return syslog_level;
}

/*********************************************************************//**
**
** LogMessageToFile
**
** Logs the specified message to the specified file
** NOTE: This function automatically inserts a trailing '\n'
**
** \param   log_level - severity level of the log
** \param   fd - file to log the string to, or NULL if logging to syslog
** \param   str - pointer to string to log
**
** \return  None
**
**************************************************************************/
void LogMessageToFile(log_level_t log_level, FILE *fd, const char *str)
{
    log_message_cb_t log_message_cb;

    // Output to log file/stdout or syslog if none specified
    if (fd == NULL)
    {
#if defined(SYSLOG_SEVERITY_OVERRIDE)
        const int syslog_level = SYSLOG_SEVERITY_OVERRIDE;
#else
        const int syslog_level = LogLevelToSyslogSeverity(log_level);
#endif
        syslog(syslog_level, "%s", str);
    }
    else
    {
        fprintf(fd, "%s\n", str);
        fflush(fd);
    }

    // Send the message to the vendor hook
    log_message_cb = vendor_hook_callbacks.log_message_cb;
    if (log_message_cb != NULL)
    {
        log_message_cb(str);
    }
}

/*********************************************************************//**
**
** CloseLog
**
** Close the current log file, if it is a file on the filesystem
**
** \param   None
**
** \return  None
**
**************************************************************************/
void CloseLog(void)
{
    // Exit if current logfile is syslog, or stdout - Nothing to do
    if ((log_fd == NULL) || (log_fd == stdout))
    {
        return;
    }

    // If the code gets here, then the log file is a file on the file system, so close it
    fclose(log_fd);
    log_fd = NULL;
}

/*********************************************************************//**
**
** USP_SNPRINTF
**
** Safe version of snprintf, that ensures buffer is always zero terminated, and does not overrun
** Always returns length of the string in the buffer (possibly truncated).
** This is different from ordinary snprintf() which (if the string was truncated)
** returns the number of characters which we'd like to print in the buffer
**
** \param   buf - pointer to buffer in which to write the string
** \param   size - size of the buffer
** \param   fmt - printf style format
**
** \return  Number of characters written to the buffer
**
**************************************************************************/
int USP_SNPRINTF(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    int len;

    // Exit if no space left in the buffer
    if (size <= 0)
    {
        return 0;
    }

    // Print the message to the buffer
    va_start(ap, fmt);
    len = vsnprintf(buf, size, fmt, ap);
    buf[size-1] = '\0';
    va_end(ap);

    // Return the number of characters actually written to the buffer
    return MIN(len, size);
}
