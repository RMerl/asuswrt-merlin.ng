/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
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
 * \file cli.h
 *
 * Common header file for command line interface (CLI) server and client functionality
 *
 */
#ifndef CLI_H
#define CLI_H

#include "socket_set.h"

#define MAX_CLI_CMD_LEN  1024           // The maximum allowed size of a CLI command. The limit is arbitrary.
#define CLI_SEPARATOR '\xFF'            // Used to separate command and args in stream passed from client to server.
                                        // Used instead of a simple space, because args themselves might contain spaces


//------------------------------------------------------------------------------------
// Server API
int CLI_SERVER_Init(void);
void CLI_SERVER_UpdateSocketSet(socket_set_t *set);
void CLI_SERVER_ProcessSocketActivity(socket_set_t *set);
void CLI_SERVER_SendResponse(const char *s);
bool CLI_SERVER_IsCmdRunLocally(char *command);
int CLI_SERVER_ExecuteCliCommand(char *command);

//------------------------------------------------------------------------------------
// Client API
int CLI_CLIENT_ExecCommand(int argc, char *argv[], char *db_file);

//------------------------------------------------------------------------------------
extern bool dump_to_cli;   // If set, dump logging messages are sent back to the CLI client rather than their normal destination
extern bool is_running_cli_local_command; // Set if this executable is running a local CLI command (eg dbset)

#endif
