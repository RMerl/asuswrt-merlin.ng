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
 * \file socket_set.c
 *
 * Basic abstraction around read and write socket sets, with a timeout
 * Socket sets are used to implement flow control on a socket
 *
 */

#include <sys/select.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "common_defs.h"
#include "socket_set.h"

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void AddSocketToSet(int sock_fd, int timeout, socket_set_t *set, fd_set *fds);
void UpdateTimeout(int timeout, socket_set_t *set);

/*********************************************************************//**
**
** SOCKET_SET_Clear
**
** Clears a socket set of all sockets and sets the timeout to the maximum it can be
**
** \param   set - pointer to socket set structure to update
**
** \return  None
**
**************************************************************************/
void SOCKET_SET_Clear(socket_set_t *set)
{
    // Clear all fdsets
    set->numfds = -1;
    FD_ZERO(&set->readfds);
    FD_ZERO(&set->writefds);
    FD_ZERO(&set->execfds);
    set->timeout.tv_sec = INT_MAX;
    set->timeout.tv_usec = 0;
}

/*********************************************************************//**
**
** SOCKET_SET_AddSocketToReceiveFrom
**
** Adds a socket to receive from, to the set
**
** \param   sock_fd - socket file descriptor to add to the set
** \param   timeout - maximum timeout for activity on the socket (in ms)
** \param   set - pointer to socket set structure to update
**
** \return  None
**
**************************************************************************/
void SOCKET_SET_AddSocketToReceiveFrom(int sock_fd, int timeout, socket_set_t *set)
{
    AddSocketToSet(sock_fd, timeout, set, &set->readfds);
}

/*********************************************************************//**
**
** SOCKET_SET_AddSocketToSendTo
**
** Adds a socket to send to, to the set
**
** \param   sock_fd - socket file descriptor to add to the set
** \param   timeout - maximum timeout for activity on the socket (in ms)
** \param   set - pointer to socket set structure to update
**
** \return  None
**
**************************************************************************/
void SOCKET_SET_AddSocketToSendTo(int sock_fd, int timeout, socket_set_t *set)
{
    AddSocketToSet(sock_fd, timeout, set, &set->writefds);
}

/*********************************************************************//**
**
** SOCKET_SET_UpdateTimeout
**
** Updates the timeout that the select waits for socket activity
** This function is called to allow timer events to punctuate the socket activity
**
** \param   timeout - maximum timeout for activity on the socket (in ms)
** \param   set - pointer to socket set structure to update
**
** \return  None
**
**************************************************************************/
void SOCKET_SET_UpdateTimeout(int timeout, socket_set_t *set)
{
    UpdateTimeout(timeout, set);
}

/*********************************************************************//**
**
** SOCKET_SET_Select
**
** Waits for activity on the socket set, subject to the minimum timeout setup in the socket set
**
** \param   set - pointer to socket set structure
**
** \return  number of sockets that have activity on them
**          0 if no sockets have activity on them
**          -1 if an unrecoverable error occurred
**
**************************************************************************/
int SOCKET_SET_Select(socket_set_t *set)
{
    int num_sockets;

    // Perform the select
    num_sockets = select(set->numfds+1, &set->readfds, &set->writefds, &set->execfds, &set->timeout);

    // Exit if an error occurred
    if (num_sockets == -1)
    {
        // Ensure that no sockets are indicated as ready to read/write in this case, otherwise the code may attempt to read a socket and block
        SOCKET_SET_Clear(set);

        // If select aborted due to a signal, then just ignore the interruption, and get the caller to retry
        if (errno == EINTR)
        {
            return 0;
        }

        // Otherwise log the error and exit
        USP_ERR_ERRNO("select", errno);
        return -1;
    }

    // Exit if no sockets have activity on them
    if (num_sockets == 0)
    {
        return 0;
    }

    // Some sockets have activity which needs processing
    return num_sockets;
}

/*********************************************************************//**
**
** SOCKET_SET_IsReadyToWrite
**
** Determines whether the specified socket is ready to transmit data on
**
** \param   sock - socket to determine if it is ready to send data on
** \param   set - pointer to socket set structure
**
** \return  Non-zero if the socket is ready to transmit data on, zero if the socket is not ready to transmit data on
**
**************************************************************************/
int SOCKET_SET_IsReadyToWrite(int sock, socket_set_t *set)
{
    USP_ASSERT(sock != INVALID);
    return FD_ISSET(sock, &set->writefds);
}

/*********************************************************************//**
**
** SOCKET_SET_IsReadyToRead
**
** Determines whether the specified socket has data to read
**
** \param   sock - socket to determine if it has data to read
** \param   set - pointer to socket set structure
**
** \return  Non-zero if the socket has data to read, zero if the socket has no data to read
**
**************************************************************************/
int SOCKET_SET_IsReadyToRead(int sock, socket_set_t *set)
{
    USP_ASSERT(sock != INVALID);
    return FD_ISSET(sock, &set->readfds);
}

/*********************************************************************//**
**
** AddSocketToSet
**
** Adds a socket to send/receive from, to the set
**
** \param   sock_fd - socket file descriptor to add to the set
** \param   timeout - maximum timeout for activity on the socket (in ms)
** \param   set - pointer to socket set structure to update
** \param   fds - pointer read or write file descriptor set within the socket set structure
**
** \return  None
**
**************************************************************************/
void AddSocketToSet(int sock_fd, int timeout, socket_set_t *set, fd_set *fds)
{
    // Add the socket to the specified set
    USP_ASSERT(sock_fd != INVALID);
    FD_SET(sock_fd, fds);
    if (sock_fd > set->numfds)
    {
        set->numfds = sock_fd;
    }

    UpdateTimeout(timeout, set);
}

/*********************************************************************//**
**
** UpdateTimeout
**
** Updates the timeout used by the select to be the least of all specified timeouts
**
** \param   timeout - maximum timeout for activity on the socket (in ms)
** \param   set - pointer to socket set structure to update
**
** \return  None
**
**************************************************************************/
void UpdateTimeout(int timeout, socket_set_t *set)
{
    int period_sec;
    int period_usec;

    // Update the timeout for activity on any socket
    // Convert period from ms into seconds and us
    period_sec = timeout/1000;
    period_usec = (timeout % 1000) * 1000;

    // Replace timeout if period is less than the current timeout
    if ( (period_sec < set->timeout.tv_sec) ||
         ((period_sec == set->timeout.tv_sec) && (period_usec < set->timeout.tv_usec)) )
    {
        set->timeout.tv_sec = period_sec;
        set->timeout.tv_usec = period_usec;
    }
}