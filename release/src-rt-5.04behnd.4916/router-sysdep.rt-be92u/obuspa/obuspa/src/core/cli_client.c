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
 * \file cli_client.c
 *
 * Implements a command line interface client
 * Both client and server are USP Agent executables, but the server is running the USP Agent core application,
 * whilst the client is effectively just calling the server to implement the command and return the result to the client.
 * Communication between the client and server is via UNIX domain sockets (hence CLI cannot be run remotely)
 *
 */

#include <stdio.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


#include "common_defs.h"
#include "cli.h"
#include "data_model.h"
#include "database.h"


//------------------------------------------------------------------------
// If this executable is running a local CLI command (eg dbset), then it performs some aspects of initialisation, but not others
bool is_running_cli_local_command = false;

//------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int HandleCliCommandRemotely(char *cmd_buf);
int HandleCliCommandLocally(char *cmd_buf, char *db_file);

/*********************************************************************//**
**
** CLI_CLIENT_ExecCommand
**
** Executes the specified command, by either:-
**      (a) Sending the command to the CLI server running on the active USP Agent, and printing the response
**      (b) Running the command locally by connecting just to the database
**
** \param   argc - Number of arguments (including the command itself)
** \param   argv - Pointer to array of arguments (including the command itself at argv[0])
** \param   db_file - file containing the database
**
** \return  Error code that this executable should return
**
**************************************************************************/
int CLI_CLIENT_ExecCommand(int argc, char *argv[], char *db_file)
{
    int i;
    int err;
    char *arg;
    char buf[MAX_CLI_CMD_LEN];
    int len;
    int arg_len;
    bool is_run_locally;

    // Exit if no command specified
    if (argc < 1)
    {
        USP_LOG_Error("ERROR: command name not specified");
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Form the command to send in a buffer
    len = 0;
    for (i=0; i<argc; i++)
    {
        // Exit if command is too long to be sent
        arg = argv[i];
        arg_len = strlen(arg);
        if (len + arg_len + 2 > sizeof(buf))   // Plus 2 to include terminating '\n\0'
        {
            USP_LOG_Error("ERROR: command too long. Increase MAX_CLI_CMD_LEN.");
            return USP_ERR_INVALID_ARGUMENTS;
        }

        memcpy(&buf[len], arg, arg_len);
        len += arg_len;

        // Add separator, if this is not the last argument
        if (i != argc-1)
        {
            buf[len] = CLI_SEPARATOR;
            len++;
        }
    }

    // Terminate the command with a LF and turn it into a string
    buf[len++] = '\n';
    buf[len] = '\0';
    USP_ASSERT(len <= MAX_CLI_CMD_LEN);

    // Decide where to handle this command
    is_run_locally = CLI_SERVER_IsCmdRunLocally(argv[0]);
    if (is_run_locally)
    {
        // Database commands handled locally - this allows us to fix incorrect connection parameters in the DB
        err = HandleCliCommandLocally(buf, db_file);
    }
    else
    {
        // All other commands sent to the active USP Agent
        err = HandleCliCommandRemotely(buf);
    }

    return err;
}

/*********************************************************************//**
**
** HandleCliCommandRemotely
**
** Executes the specified command by sending the command to the CLI server
** running on the active USP Agent, and printing the response
**
** \param   cmd_buf - command and arguments, to send to the active USP Agent
**
** \return  Error code that this executable should return
**
**************************************************************************/
int HandleCliCommandRemotely(char *cmd_buf)
{
    int err;
    int sock;
    struct sockaddr_un sa;
    int bytes_sent;
    int bytes_received;
    int len;
    char buf[256];

    // Exit if unable to create a blocking socket to send the CLI command on
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        USP_ERR_ERRNO("socket", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Fill in sockaddr structure
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    USP_STRNCPY(sa.sun_path, cli_uds_file, sizeof(sa.sun_path));

    // Exit if unable to connect the socket to the unix domain file
    err = connect(sock, (struct sockaddr *) &sa, sizeof(struct sockaddr_un));
    if (err == -1)
    {
        USP_ERR_ERRNO("connect", errno);
        USP_LOG_Error("%s: Unable to connect to Unix domain socket file %s", __FUNCTION__, cli_uds_file);
        close(sock);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to send the command
    len = strlen(cmd_buf);
    bytes_sent = send(sock, cmd_buf, len, 0);
    if (bytes_sent == -1)
    {
        USP_ERR_ERRNO("send", errno);
        close(sock);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Print the response received back
    err = USP_ERR_OK;
    bytes_received = 1;
    while (bytes_received > 0)
    {
        bytes_received = recv(sock, buf, sizeof(buf)-1, 0);
        if (bytes_received == -1)
        {
            // Exit loop if no more response to receive
            if (errno == EPIPE)
            {
                err = USP_ERR_OK;
                break;
            }

            // Exit loop if an unexpected error occurred
            USP_ERR_ERRNO("send", errno);
            err = USP_ERR_INTERNAL_ERROR;
            break;
        }

        // Print response received back
        buf[bytes_received] = '\0';     // Ensure that string received is NULL terminated
        printf("%s", buf);
    }

    close(sock);
    return err;
}

/*********************************************************************//**
**
** HandleCliCommandLocally
**
** Executes the specified command by running the command locally and connecting just to the database
**
** \param   cmd_buf - command and arguments, to run locally
**
** \return  Error code that this executable should return
**
**************************************************************************/
int HandleCliCommandLocally(char *cmd_buf, char *db_file)
{
    int err;
    char *cmd_end;

    printf("Running command on database %s\n", db_file);

    // Set global variable which suppresses some normal actions on startup, which we don't want to occur when running a db command
    is_running_cli_local_command = true;

    // Exit if an error occurred when initialising the database
    err = DATABASE_Init(db_file);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if an error occurred when initialising the data model
    err = DATA_MODEL_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Make command into a string
    cmd_end = strchr(cmd_buf, '\n');
    *cmd_end = '\0';

    err = CLI_SERVER_ExecuteCliCommand(cmd_buf);

    return err;
}
