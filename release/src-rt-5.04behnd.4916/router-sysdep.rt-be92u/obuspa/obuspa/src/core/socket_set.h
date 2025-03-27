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
 * \file socket_set.h
 *
 * Basic abstraction around read and write socket sets, with a timeout
 *
 */
#ifndef SOCKET_SET_H
#define SOCKET_SET_H

#include <sys/select.h>

//------------------------------------------------------------------------------
// Maximum socket timeout that the code uses - 1 hour in milliseconds
#define MAX_SOCKET_TIMEOUT_SECONDS 3600
#define MAX_SOCKET_TIMEOUT (MAX_SOCKET_TIMEOUT_SECONDS*SECONDS)

//------------------------------------------------------------------------------
// Socket set structure
typedef struct
{
    int numfds;
    fd_set readfds;
    fd_set writefds;
    fd_set execfds;
    struct timeval timeout;
} socket_set_t;

//------------------------------------------------------------------------------
// API functions
void SOCKET_SET_Clear(socket_set_t *set);
void SOCKET_SET_AddSocketToReceiveFrom(int sock_fd, int timeout, socket_set_t *set);
void SOCKET_SET_AddSocketToSendTo(int sock_fd, int timeout, socket_set_t *set);
void SOCKET_SET_UpdateTimeout(int timeout, socket_set_t *set);
int SOCKET_SET_IsReadyToWrite(int sock, socket_set_t *set);
int SOCKET_SET_IsReadyToRead(int sock, socket_set_t *set);
int SOCKET_SET_Select(socket_set_t *set);

#endif
