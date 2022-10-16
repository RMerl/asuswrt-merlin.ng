/*
 * AWS IoT Device SDK for Embedded C 202103.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Standard includes. */
#include <assert.h>
#include <string.h>

/* POSIX socket includes. */
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "plaintext_posix.h"

/*-----------------------------------------------------------*/

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    PlaintextParams_t * pParams;
};

/*-----------------------------------------------------------*/

/**
 * @brief Log possible error from send/recv.
 *
 * @param[in] errorNumber Error number to be logged.
 */
static void logTransportError( int32_t errorNumber );

/*-----------------------------------------------------------*/

static void logTransportError( int32_t errorNumber )
{
    /* Remove unused parameter warning. */
    ( void ) errorNumber;

    LogError( ( "A transport error occurred: %s.", strerror( errorNumber ) ) );
}
/*-----------------------------------------------------------*/

SocketStatus_t Plaintext_Connect( NetworkContext_t * pNetworkContext,
                                  const ServerInfo_t * pServerInfo,
                                  uint32_t sendTimeoutMs,
                                  uint32_t recvTimeoutMs )
{
    SocketStatus_t returnStatus = SOCKETS_SUCCESS;
    PlaintextParams_t * pPlaintextParams = NULL;

    /* Validate parameters. */
    if( ( pNetworkContext == NULL ) || ( pNetworkContext->pParams == NULL ) )
    {
        LogError( ( "Parameter check failed: pNetworkContext is NULL." ) );
        returnStatus = SOCKETS_INVALID_PARAMETER;
    }
    else
    {
        pPlaintextParams = pNetworkContext->pParams;
        returnStatus = Sockets_Connect( &pPlaintextParams->socketDescriptor,
                                        pServerInfo,
                                        sendTimeoutMs,
                                        recvTimeoutMs );
    }

    return returnStatus;
}
/*-----------------------------------------------------------*/

SocketStatus_t Plaintext_Disconnect( const NetworkContext_t * pNetworkContext )
{
    SocketStatus_t returnStatus = SOCKETS_SUCCESS;
    PlaintextParams_t * pPlaintextParams = NULL;

    /* Validate parameters. */
    if( ( pNetworkContext == NULL ) || ( pNetworkContext->pParams == NULL ) )
    {
        LogError( ( "Parameter check failed: pNetworkContext is NULL." ) );
        returnStatus = SOCKETS_INVALID_PARAMETER;
    }
    else
    {
        pPlaintextParams = pNetworkContext->pParams;
        returnStatus = Sockets_Disconnect( pPlaintextParams->socketDescriptor );
    }

    return returnStatus;
}
/*-----------------------------------------------------------*/

/* MISRA Rule 8.13 flags the following line for not using the const qualifier
 * on `pNetworkContext`. Indeed, the object pointed by it is not modified
 * by POSIX sockets, but other implementations of `TransportRecv_t` may do so. */
int32_t Plaintext_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv )
{
    PlaintextParams_t * pPlaintextParams = NULL;
    int32_t bytesReceived = -1, selectStatus = -1, getTimeoutStatus = -1;
    struct timeval recvTimeout;
    socklen_t recvTimeoutLen;
    fd_set readfds;

    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToRecv > 0 );

    /* Get receive timeout from the socket to use as the timeout for #select. */
    pPlaintextParams = pNetworkContext->pParams;
    recvTimeoutLen = ( socklen_t ) sizeof( recvTimeout );
    getTimeoutStatus = getsockopt( pPlaintextParams->socketDescriptor,
                                   SOL_SOCKET,
                                   SO_RCVTIMEO,
                                   &recvTimeout,
                                   &recvTimeoutLen );

    /* Make #select return immediately if getting the timeout failed. */
    if( getTimeoutStatus < 0 )
    {
        recvTimeout.tv_sec = 0;
        recvTimeout.tv_usec = 0;
    }

    /* MISRA Directive 4.6 flags the following line for a violation of using a
     * basic type "int" rather than a type that includes size and signedness information.
     * We suppress the violation as the flagged type, "fd_set", is a POSIX
     * system-specific type, and is used for the call to "select()". */

    /* MISRA Rule 14.4 flags the following line for using condition expression "0"
     * as a boolean type. We suppress the violation as the "FD_ZERO" is a POSIX
     * specific macro utility whose implementation is supplied by the system.
     * The "FD_ZERO" macro is called as specified by the POSIX manual here:
     * https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/select.h.html */
    /* coverity[misra_c_2012_directive_4_6_violation] */
    /* coverity[misra_c_2012_rule_14_4_violation] */
    FD_ZERO( &readfds );

    /* MISRA Directive 4.6 flags the following line for a violation of using a
     * basic type "int" rather than a type that includes size and signedness information.
     * We suppress the violation as the flagged type, "fd_set", is a POSIX
     * system-specific type, and is used for the call to "select()". */
    /* coverity[misra_c_2012_directive_4_6_violation] */

    /* MISRA Rule 10.1, Rule 10.8 and Rule 13.4 flag the following line for
     * implementation of the "FD_SET()" POSIX macro. We suppress these violations
     * "FD_SET" is a POSIX specific macro utility whose implementation
     * is supplied by the system.
     * The "FD_SET" macro is used as specified by the POSIX manual here:
     * https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/select.h.html */
    /* coverity[misra_c_2012_directive_4_6_violation] */
    /* coverity[misra_c_2012_rule_10_1_violation] */
    /* coverity[misra_c_2012_rule_13_4_violation] */
    /* coverity[misra_c_2012_rule_10_8_violation] */
    FD_SET( pPlaintextParams->socketDescriptor, &readfds );

    /* Check if there is data to read from the socket. */
    selectStatus = select( pPlaintextParams->socketDescriptor + 1,
                           &readfds,
                           NULL,
                           NULL,
                           &recvTimeout );

    if( selectStatus > 0 )
    {
        /* The socket is available for receiving data. */
        bytesReceived = ( int32_t ) recv( pPlaintextParams->socketDescriptor,
                                          pBuffer,
                                          bytesToRecv,
                                          0 );
    }
    else if( selectStatus < 0 )
    {
        /* An error occurred while polling. */
        bytesReceived = -1;
    }
    else
    {
        /* Timed out waiting for data to be received. */
        bytesReceived = 0;
    }

    if( ( selectStatus > 0 ) && ( bytesReceived == 0 ) )
    {
        /* Peer has closed the connection. Treat as an error. */
        bytesReceived = -1;
    }
    else if( bytesReceived < 0 )
    {
        logTransportError( errno );
    }
    else
    {
        /* Empty else MISRA 15.7 */
    }

    return bytesReceived;
}
/*-----------------------------------------------------------*/

/* MISRA Rule 8.13 flags the following line for not using the const qualifier
 * on `pNetworkContext`. Indeed, the object pointed by it is not modified
 * by POSIX sockets, but other implementations of `TransportSend_t` may do so. */
int32_t Plaintext_Send( NetworkContext_t * pNetworkContext,
                        const void * pBuffer,
                        size_t bytesToSend )
{
    PlaintextParams_t * pPlaintextParams = NULL;
    int32_t bytesSent = -1, selectStatus = -1, getTimeoutStatus = -1;
    struct timeval sendTimeout;
    socklen_t sendTimeoutLen;
    fd_set writefds;

    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToSend > 0 );

    /* Get send timeout from the socket to use as the timeout for #select. */
    pPlaintextParams = pNetworkContext->pParams;
    sendTimeoutLen = ( socklen_t ) sizeof( sendTimeout );
    getTimeoutStatus = getsockopt( pPlaintextParams->socketDescriptor,
                                   SOL_SOCKET,
                                   SO_SNDTIMEO,
                                   &sendTimeout,
                                   &sendTimeoutLen );

    /* Make #select return immediately if getting the timeout failed. */
    if( getTimeoutStatus < 0 )
    {
        sendTimeout.tv_sec = 0;
        sendTimeout.tv_usec = 0;
    }

    /* MISRA Directive 4.6 flags the following line for a violation of using a
     * basic type "int" rather than a type that includes size and signedness information.
     * We suppress the violation as the flagged type, "fd_set", is a POSIX
     * system-specific type, and is used for the call to "select()". */

    /* MISRA Rule 14.4 flags the following line for using condition expression "0"
     * as a boolean type. We suppress the violation as the "FD_ZERO" is a POSIX
     * specific macro utility whose implementation is supplied by the system.
     * The "FD_ZERO" macro is called as specified by the POSIX manual here:
     * https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/select.h.html */
    /* coverity[misra_c_2012_directive_4_6_violation] */
    /* coverity[misra_c_2012_rule_14_4_violation] */
    FD_ZERO( &writefds );

    /* MISRA Directive 4.6 flags the following line for a violation of using a
     * basic type "int" rather than a type that includes size and signedness information.
     * We suppress the violation as the flagged type, "fd_set", is a POSIX
     * system-specific type, and is used for the call to "select()". */

    /* MISRA Rule 10.1, Rule 10.8 and Rule 13.4 flag the following line for
     * implementation of the "FD_SET()" POSIX macro. We suppress these violations
     * as "FD_SET" is a POSIX specific macro utility whose implementation
     * is supplied by the system.
     * The "FD_ZERO" macro is used as specified by the POSIX manual here:
     * https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/select.h.html */
    /* coverity[misra_c_2012_directive_4_6_violation] */
    /* coverity[misra_c_2012_rule_10_1_violation] */
    /* coverity[misra_c_2012_rule_13_4_violation] */
    /* coverity[misra_c_2012_rule_10_8_violation] */
    FD_SET( pPlaintextParams->socketDescriptor, &writefds );
    /* Check if data can be written to the socket. */
    selectStatus = select( pPlaintextParams->socketDescriptor + 1,
                           NULL,
                           &writefds,
                           NULL,
                           &sendTimeout );

    if( selectStatus > 0 )
    {
        /* The socket is available for sending data. */
        bytesSent = ( int32_t ) send( pPlaintextParams->socketDescriptor,
                                      pBuffer,
                                      bytesToSend,
                                      0 );
    }
    else if( selectStatus < 0 )
    {
        /* An error occurred while polling. */
        bytesSent = -1;
    }
    else
    {
        /* Timed out waiting for data to be sent. */
        bytesSent = 0;
    }

    if( ( selectStatus > 0 ) && ( bytesSent == 0 ) )
    {
        /* Peer has closed the connection. Treat as an error. */
        bytesSent = -1;
    }
    else if( bytesSent < 0 )
    {
        logTransportError( errno );
    }
    else
    {
        /* Empty else MISRA 15.7 */
    }

    return bytesSent;
}
/*-----------------------------------------------------------*/
