/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

/*
 * HTTP server providing data-model info in json format on port 8080
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "ieee1905_glue.h"
#include "ieee1905_json.h"
#include "ieee1905_socket.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_trace.h"
#include <stdarg.h>

#if defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)
#include "httpdshared.h"
#include "libwebsockets.h"
#endif // endif

#define PORT 8080
#define I5_TRACE_MODULE i5TraceJson
#define I5_JSON_SHOW_LEGACY_FILENAME "/data/plc/ieee1905ShowLegacy"
#define WSD_PORT                        7683

#define MAX_PRINT_LEN                   512
#define I5_JSON_WEBSOCK_POLL_IN_MS      100
#define I5_JSON_WEBSOCK_RESTART_IN_MS   1000

// Turn the following on to allow websockets to split a string in two parts, should the string
// pass the end of the libwebsocket's send window.

#if defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)

// NOTE: the websocket Q must be concurrently safe, as i5JsonWebSocketsCallback is not
// run in the same context as the rest of the 1905 app.
//
// This uses a lockless concurency scheme (first/last/divider) to prevent concurrency issues

typedef struct _I5WsQElem {
    char                    *str;
    struct _I5WsQElem       *next;
} I5WsQElem;

typedef struct _WebSocketQ {
    I5WsQElem                       *first;
    I5WsQElem                       *divider;
    I5WsQElem                       *last;
    struct lws                      *wsi;
} WebSocketQ;

typedef struct _I5WsUser {
	// JU_TBD: move wsq into here
	unsigned short sessionKey;
        int            hasStarted;
} I5WsUser;

static struct lws_context *gWebSocketContext;
static timer_elem_type *gWebSocketPollTimer = NULL;
static timer_elem_type *gWebSocketRestartTimer = NULL;

// normally there would be one queue per wsi, but right now, only accept one wsi:
WebSocketQ gWsq = {};

int i5JsonWebSocketsInit(void);
void i5JsonWebSocketsDeinit(void);
int i5JsonWebSocketsWrite(const char * str);
void i5JsonWebSocketsPoll(void * arg);
void i5JsonWebSocketsRestart(void * arg);
#endif // endif

static int _areWebSocketsActive();
static int _sock_printf(void * client, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

#ifndef MIN
#define MIN(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })
#endif // endif

static int strnlen_safe(const char *str, int max) __attribute__ ((unused));
static int strnlen_safe(const char *str, int max) {
  // note: strnlen is not supported on all compilers -- create our own version
  const char * end;
  if (!str)
    return -1;
  end = memchr(str, '\0', max+1);
  if (end && *end == '\0') {
       return (int)(end-str);
  }
  return -1;
}

// this is just to prevent doing some unnecessary cpu work, and reduces some unnecessary
// debug logs when json is not active (which should be the normal case...):
int numSocketConnections = 0;
static inline int _amActive() {
#if defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)
    return (numSocketConnections || gWsq.wsi);
#else
    return (numSocketConnections);
#endif // endif
}

// -------------------------------------------------------------

/* returns 2 when connected to GEPHI,
 *         3 when connected to web pages:
 */
static int _getLegacyNeighborNodeType() {
    return _areWebSocketsActive() ? 3 : 2;
}

/* returns 0  if successful
 *         -1 if socket fails
 */
int i5JsonFflush(i5_socket_type *psock) {
  if (fflush(psock->u.sinr.fp) < 0) {
    fclose(psock->u.sinr.fp);
    i5TraceError("Socket died\n");
    i5SocketClose(psock);
    numSocketConnections--;
    return -1;
  }
  return 0;
}

void i5JsonDeviceUpdate(void * client, i5_dm_device_type *device)
{
  if (!_amActive())
    return;
  _sock_printf(client, "{\"cn\":{\"Dev-" I5_MAC_FMT "\":{\"label\":\"%s\"}}}\r", I5_MAC_PRM(device->DeviceId), device->friendlyName);
}

/* returns 0  if successful
 *         -1 if socket failure
 */
int i5JsonDevicePrint(void *client, i5_dm_device_type *device, enum i5JsonOperationType opType)
{
  int rc;

  if (!_amActive())
    return 0;

  if (opType == i5Json_Add) {
    rc = _sock_printf(client, "{\"an\":{\"Dev-" I5_MAC_FMT "\":{\"label\":\"%s\",\"Type\":\"1\"}}}\r", I5_MAC_PRM(device->DeviceId), device->friendlyName);
  }
  else {
    rc = _sock_printf(client, "{\"dn\":{\"Dev-" I5_MAC_FMT "\":{}}}\r", I5_MAC_PRM(device->DeviceId));
  }
  return rc <= 0 ? -1 : 0;
}

void i5JsonUpdateInterfacePrint(void * client, i5_dm_interface_type *myInterface)
{
  if (!_amActive())
    return;

  if (NULL == myInterface) {
    i5Trace("Error, null pointer\n");
    return;
  }

  _sock_printf(client, "{\"cn\":{\"" I5_MAC_FMT "\":{\"label\":\"%s\"}}}\r",
          I5_MAC_PRM(myInterface->InterfaceId), i5DmGetNameForMediaType(myInterface->MediaType));
}

/* returns 0  if successful
 *         -1 if flush returns socket failure
 */
int i5JsonInterfacePrint(void * client, i5_dm_device_type *device, i5_dm_interface_type *myInterface, enum i5JsonOperationType opType)
{
  int rc;

  if (!_amActive())
    return 0;

  if (opType == i5Json_Add) {
    rc = _sock_printf(client, "{\"an\":{\"" I5_MAC_FMT "\":{\"label\":\"%s\",\"Type\":\"2\",\"info\":\"" I5_MAC_DELIM_FMT "\"}}}\r",
            I5_MAC_PRM(myInterface->InterfaceId), i5DmGetNameForMediaType(myInterface->MediaType), I5_MAC_PRM(myInterface->InterfaceId));
    rc = _sock_printf(client, "{\"ae\":{\"Dev-" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"source\":\"Dev-" I5_MAC_FMT "\",\"directed\":false,\"target\":\"" I5_MAC_FMT "\",\"EdgeType\":\"1\"%s}}}\r",
            I5_MAC_PRM(device->DeviceId), I5_MAC_PRM(myInterface->InterfaceId), I5_MAC_PRM(device->DeviceId), I5_MAC_PRM(myInterface->InterfaceId), _areWebSocketsActive()?",\"label\":\"\"" : "");
  }
  else {
    rc = _sock_printf(client, "{\"de\":{\"" I5_MAC_FMT "_" I5_MAC_FMT "\":{}}}\r", I5_MAC_PRM(device->DeviceId), I5_MAC_PRM(myInterface->InterfaceId));
    rc = _sock_printf(client, "{\"dn\":{\"" I5_MAC_FMT "\":{}}}\r", I5_MAC_PRM(myInterface->InterfaceId));
  }

  return rc <= 0 ? -1 : 0;
}

/* returns 0  if successful
 *         -1 if flush returns socket failure
 */

int i5Json1905LegacyNeighborPrint(void * client, i5_dm_legacy_neighbor_type *neighbor, enum i5JsonOperationType opType)
{
  int rc = 0;

  if (!_amActive())
    return 0;

  if (opType == i5Json_Add) {
    /* Use the local interface MAC in the node's definition (but not the label)
       - this node is different from other nodes that are created for the same actual non-1905 device
       - this avoids a problem with non-1905 devices moving from one place to another in the graph */

    if (i5_config.jsonLegacyDisplayEnabled) {
      rc = _sock_printf(client, "{\"an\":{\"Node" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"label\":\"N-%02hhX%02hhX%02hhX\",\"Type\":\"%d\"}}}\r",
                       I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
                       neighbor->NeighborInterfaceId[3], neighbor->NeighborInterfaceId[4], neighbor->NeighborInterfaceId[5],
                       _getLegacyNeighborNodeType());
      rc = _sock_printf( client,
              "{\"ae\":{\"Leg" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"source\":\"" I5_MAC_FMT "\",\"directed\":false,\"target\":\"Node" I5_MAC_FMT "_" I5_MAC_FMT "\",\"EdgeType\":\"2\"%s}}}\r",
              I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
              I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
              _areWebSocketsActive()?",\"label\":\"\"" : "");
    }
  }
  else {
    rc = _sock_printf(client,  "{\"de\":{\"Leg" I5_MAC_FMT "_" I5_MAC_FMT "\":{}}}\r", I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId));
    rc = _sock_printf(client,  "{\"dn\":{\"Node" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"label\":\"N-%02hhX%02hhX%02hhX\",\"Type\":\"%d\"}}}\r",
            I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
            neighbor->NeighborInterfaceId[3], neighbor->NeighborInterfaceId[4], neighbor->NeighborInterfaceId[5],
            _getLegacyNeighborNodeType());
  }

  return rc < 0 ? -1 : 0;
}

/* returns 0  if successful
 *         -1 if flush returns socket failure
 */
int i5Json1905NeighborPrint(void * client, i5_dm_1905_neighbor_type *neighbor, enum i5JsonOperationType opType)
{
  int rc;

  if (!_amActive())
    return 0;

  if (opType == i5Json_Add) {
    if (memcmp(neighbor->Ieee1905Id, i5_config.i5_mac_address, MAC_ADDR_LEN) ) {
      rc = _sock_printf(
              client, "{\"ae\":{\"" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"source\":\"" I5_MAC_FMT "\",\"directed\":false,\"target\":\"" I5_MAC_FMT "\",\"EdgeType\":\"2\"%s}}}\r",
              I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
              I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
              _areWebSocketsActive()?",\"label\":\"\"" : "");
    }
    else {
      rc = _sock_printf(
              client, "{\"ae\":{\"" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"source\":\"" I5_MAC_FMT "\",\"directed\":false,\"target\":\"" I5_MAC_FMT "\",\"Label\":\"0/0 Mbps\",\"EdgeType\":\"2\"}}}\r",
              I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
              I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId));
    }
  }
  else {
    /* Delete both links (from device interface to neighbor interface, and the other way around). If one did not exist, it will be ignored */
    rc = _sock_printf(client, "{\"de\":{\"" I5_MAC_FMT "_" I5_MAC_FMT "\":{}}}\r", I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId));
    rc = _sock_printf(client, "{\"de\":{\"" I5_MAC_FMT "_" I5_MAC_FMT "\":{}}}\r", I5_MAC_PRM(neighbor->NeighborInterfaceId), I5_MAC_PRM(neighbor->LocalInterfaceId));
  }

  return rc <= 0 ? -1 : 0;
}

void i5Json1905NeighborUpdatePrint(void * client, i5_dm_1905_neighbor_type *neighbor, i5_dm_1905_neighbor_type *symmNeighbor)
{
  char label[16];

  unsigned short avgAvail = 0;
  unsigned short avgThrough = 0;

  if (!_amActive())
    return;

  if (NULL != symmNeighbor) {
    i5TraceInfo("Averaging\n");
    avgAvail = (neighbor->availableThroughputCapacity >> 1) + (symmNeighbor->availableThroughputCapacity >> 1);
    avgThrough = (neighbor->MacThroughputCapacity >> 1) + (symmNeighbor->MacThroughputCapacity >> 1);
  } else {
    i5TraceInfo("Normal\n");
    avgAvail = neighbor->availableThroughputCapacity;
    avgThrough = neighbor->MacThroughputCapacity;
  }

  // Show the used throughput, not the available throughput
  if (avgAvail > avgThrough)
     avgAvail = avgThrough;
  avgAvail = avgThrough - avgAvail;
  if (avgAvail == 1)
     avgAvail = 0;

  snprintf(label,sizeof(label),"%d/%d Mbps", avgAvail, avgThrough);
  label[sizeof(label)-1] = '\0';

  // a touch of ugliness here: gephi requires Label (case sensitive).  The web page requires 'label'
  _sock_printf(client, "{\"ce\":{\"" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"%s\":\"%s\"}}}\r",
          I5_MAC_PRM(neighbor->LocalInterfaceId), I5_MAC_PRM(neighbor->NeighborInterfaceId),
          _areWebSocketsActive()?"label":"Label", label);
  _sock_printf(client, "{\"ce\":{\"" I5_MAC_FMT "_" I5_MAC_FMT "\":{\"%s\":\"%s\"}}}\r",
          I5_MAC_PRM(neighbor->NeighborInterfaceId), I5_MAC_PRM(neighbor->LocalInterfaceId),
          _areWebSocketsActive()?"label":"Label", label);
}

// note: to support multiple connections, this has to be called with the specific socket/websocket
// as a parameter.
void i5JsonPrintAll(void * client)
{
  i5_dm_device_type *device;

  if (!_amActive())
    return;

  /* Print the devices and their interfaces first */
  device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (device != NULL) {
    i5_dm_interface_type *interface = (i5_dm_interface_type *)device->interface_list.ll.next;
    if (i5JsonDevicePrint(client, device, 1) == -1) {
      i5TraceError("Socket died\n");
      return;
    }
    while (interface != NULL) {
      if (i5JsonInterfacePrint(client, device, interface, 1) == -1) {
        i5TraceError("Socket died\n");
        return;
      }
      interface = interface->ll.next;
    }
    device = device->ll.next;
  }

  /* Now print the links between the different interfaces */
  device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (device != NULL) {
    i5_dm_1905_neighbor_type *neighbor = (i5_dm_1905_neighbor_type *)device->neighbor1905_list.ll.next;
    i5_dm_legacy_neighbor_type *legNeighbor = (i5_dm_legacy_neighbor_type *)device->legacy_list.ll.next;
    while (neighbor != NULL) {
      if (!i5DmIsMacNull(neighbor->NeighborInterfaceId)) {
        if (i5Json1905NeighborPrint(client, neighbor, i5Json_Add) == -1) {
          i5TraceError("Socket died\n");
          return;
        }
      }
      neighbor = neighbor->ll.next;
    }
    while (legNeighbor != NULL) {
      if (i5Json1905LegacyNeighborPrint(client, legNeighbor, 1) == -1) {
        i5TraceError("Socket died\n");
        return;
      }
      legNeighbor = legNeighbor->ll.next;
    }
    device = device->ll.next;
  }

}

int i5JsonProcess(i5_socket_type *psock)
{
  FILE *fp ;

  if (!psock) {
    i5TraceError("\n");
    return -1;
  }

  fp = psock->u.sinr.fp;

  fprintf(fp, "HTTP/1.0 200 OK\r\n");
  fprintf(fp, "Content-Type: text/plain\r\n");
  fprintf(fp, "\r\n");

  i5JsonPrintAll(psock);

  return 0;
}

void i5JsonRead(i5_socket_type *psock)
{
  char buf[32];
  int isGet = 0;

  i5TraceInfo("\n");

  if (!psock) {
    i5TraceError("\n");
    return;
  }

  do {
    if (fgets(buf, sizeof(buf), psock->u.sinr.fp) == NULL) {
      if (errno != EWOULDBLOCK) {
        fclose(psock->u.sinr.fp);
        i5SocketClose(psock);
        numSocketConnections--;
        i5Trace("socket Connections = %d\n", numSocketConnections);
        return;
      }
      break;
    }
    if (memcmp(buf, "GET", 3) == 0) {
      isGet = 1;
    }
  } while(1);

  if (isGet) {
    psock->u.sinr.isGet = 1;
    i5JsonProcess(psock);
    return;
  }

  if (feof(psock->u.sinr.fp) || (!psock->u.sinr.isGet && errno == EWOULDBLOCK)) {
    fclose(psock->u.sinr.fp);
    i5SocketClose(psock);
    numSocketConnections--;
    i5Trace("socket Connections = %d\n", numSocketConnections);
    return;
  }
}

void i5JsonAccept(i5_socket_type *psock_listen)
{
  int sd;
  int flags;
  int fd;
  i5_socket_type *psock;

  i5TraceInfo("\n");

  if (!psock_listen) {
    i5TraceError("\n");
    return;
  }

  do {
    if (_areWebSocketsActive()) {
        i5Trace("attempt to attach to regular socket when websocket still open -- reject\n");
        break;
    }

    sd = accept(psock_listen->sd, NULL, NULL);
    if (sd < 0) {
      if (errno != EWOULDBLOCK) {
        printf("accept error\n");
      }
      break;
    }

    /* Set accepted sd to non-blocking */
    flags = fcntl(sd, F_GETFL, 0);
    if(flags < 0) {
      printf("cannot retrieve socket flags. error %s\n",strerror(errno));
    }
    if ( fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
      printf("cannot set socket to non-blocking. error %s\n",strerror(errno));
    }

    psock = i5SocketNew(sd, i5_socket_type_json, i5JsonRead);
    if ( psock == NULL ) {
      close(sd);
      continue;
    }
    numSocketConnections++;
    i5Trace("socket Connections = %d\n", numSocketConnections);

    psock->u.sinr.fp = fdopen(psock->sd, "a+");

    /* Set new fd to non-blocking */
    fd = fileno(psock->u.sinr.fp);
    flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0) {
      printf("cannot retrieve socket flags. error %s\n",strerror(errno));
    }
    flags |= O_NONBLOCK;
    if ( fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
       printf("cannot set socket to non-blocking. error %s\n",strerror(errno));
    }
  } while (sd != -1);
}

void i5JsonInitSigHandler(int signo)
{
  /* Just ignore SIGPIPE, the regular code will take care of it */
}

void i5JsonInit()
{
  i5_socket_type *psock;
  int optval = 1;
  socklen_t optlen = sizeof(optval);
  int sd;
  int flags;

  FILE* legacyFile = fopen(I5_JSON_SHOW_LEGACY_FILENAME, "r");
  if (legacyFile != NULL) {
    i5JsonConfigLegacyDisplay(I5_JSON_ALL_CLIENTS, 1);
    fclose(legacyFile);
  }

  if (i5_config.num_i5_sockets >= MAX_I5_SOCKETS) {
    printf("Too many sockets\n");
  }

  signal(SIGPIPE, i5JsonInitSigHandler);

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("socket() error\n");
    return;
  }
  psock = i5SocketNew(sd, i5_socket_type_in_listen, i5JsonAccept);
  if ( psock == NULL ) {
    close(sd);
    return;
  }

  /* Allow reusing the socket immediately when application is restarted */
  if (setsockopt(psock->sd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen)) {
    printf("setsockopt error %s\n", strerror(errno));
  }

  /* Set listening sd to non-blocking */
  flags = fcntl(psock->sd, F_GETFL, 0);
  if(flags < 0) {
    printf("cannot retrieve socket flags. error %s\n",strerror(errno));
  }
  if ( fcntl(psock->sd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
    printf("cannot set socket to non-blocking. error %s\n",strerror(errno));
  }

  psock->u.sinl.sa.sin_family   = AF_INET;
  psock->u.sinl.sa.sin_addr.s_addr = htonl(INADDR_ANY);
  psock->u.sinl.sa.sin_port = htons(PORT);

  if((bind(psock->sd, (struct sockaddr *)&(psock->u.sinl.sa), sizeof(struct sockaddr_in)))== -1) {
    printf("bind() error %s\n", strerror(errno));
  }

  /* In theory, this code should support as many connections as we can, but linux seems to insist
     on accepting only one connection at a time. Since we are streaming here, the connection is never closed
     by the server, so you need to stop the client before you can start another client */
  if (listen(psock->sd, SOMAXCONN)) {
    printf("listen() error %s\n", strerror(errno));
  }

#if defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)
  i5JsonWebSocketsInit();
#endif // endif
}

void i5JsonDeinit(void) {
#if defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)
    i5JsonWebSocketsDeinit();
#endif // endif
}

/* Pushes updates for all Legacy neighbours (on all devices)
   enabled = 1 -> Add all
   enabled = 0 -> Delete all
 */
void i5JsonLegacyNeighborDisplayAll(void * client, unsigned int enabled)
{
  i5_dm_device_type *i5Device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  i5Trace(": %s\n", enabled ? "Showing" : "Hiding");

  while (i5Device != NULL) {
    i5_dm_legacy_neighbor_type *legacy_neighbor = (i5_dm_legacy_neighbor_type *)i5Device->legacy_list.ll.next;
    i5Trace(" Device " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(i5Device->DeviceId));
    while (legacy_neighbor != NULL) {
      i5Json1905LegacyNeighborPrint(client, legacy_neighbor, enabled ? i5Json_Add : i5Json_Delete);
      i5Trace(" Leg. neigh. " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(legacy_neighbor->NeighborInterfaceId));
      legacy_neighbor = (i5_dm_legacy_neighbor_type *)legacy_neighbor->ll.next;
    }
    i5Device = (i5_dm_device_type *)i5Device->ll.next;
  }
}

void i5JsonConfigLegacyDisplay(void * client, unsigned int legacyEnable)
{
  legacyEnable = !!legacyEnable;
  i5Trace("Setting Legacy Enable to %d\n", legacyEnable);

  if (i5_config.jsonLegacyDisplayEnabled != legacyEnable) {
    i5_config.jsonLegacyDisplayEnabled = legacyEnable;
    i5JsonLegacyNeighborDisplayAll(client, legacyEnable);
    i5Trace("Legacy display changed to %s\n", legacyEnable ? "Enabled" : "Disabled");
    if (1 == i5_config.jsonLegacyDisplayEnabled) {
      i5Trace("Creating " I5_JSON_SHOW_LEGACY_FILENAME "\n");
      FILE* legacyFile = fopen(I5_JSON_SHOW_LEGACY_FILENAME, "w");

      if (legacyFile != NULL) {
        fclose(legacyFile);
      }
    }
    else {
      i5Trace("Deleting " I5_JSON_SHOW_LEGACY_FILENAME "\n");
      remove(I5_JSON_SHOW_LEGACY_FILENAME);
    }
  }
}

// ========================= websocket support: ===============================
#if defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)

// -------------------- web socket queue: ------------------------------
static WebSocketQ * getWsq(struct lws *wsi) {
    if (gWsq.wsi == wsi)
        return &gWsq;
    return NULL;
}

static int i5JsonWebSocketQInit(struct lws *wsi) {
    i5Trace("\n");
    if (!wsi) {
        i5TraceError("\n");
        return -1;
    }

    if (gWsq.wsi) {
        i5TraceError("\n");
        return -1;
    }
    gWsq.first = gWsq.divider = gWsq.last = malloc(sizeof(I5WsQElem));
    if (gWsq.first == NULL) {
        i5TraceError("Error -- could not allocate websocket queue (%u)\n", sizeof(I5WsQElem));
        return -1;
    }

    gWsq.first->str = NULL;

    // important: wsi must be set after the rest of gWsq is set up:
    gWsq.wsi = wsi;

    return 0;
}

static int i5JsonWebSocketQDeinit(WebSocketQ *wsq) {
    I5WsQElem * freeElem;

    i5Trace("\n");

    if (!wsq) {
        i5TraceError("\n");
        return -1;
    }

    wsq->wsi = NULL;
    wsq->last = NULL;  // just to be safe.

    while (wsq->first) {
        freeElem = wsq->first;
        wsq->first = wsq->first->next;
        if (freeElem->str)
            free(freeElem->str);
        free(freeElem);
    }
    wsq->first = wsq->divider = wsq->last = NULL; // not necessary, but good practice

    i5Trace("\n");
    return 0;
}

// called from libwebsocket thread context:
static char * i5JsonWebSocketQPop( WebSocketQ *wsq, char **str, size_t *len ) {

    if (!wsq) {
        i5TraceError("\n");
        return NULL;
    }

    if (str == NULL) {
        i5TraceError("\n");
        return NULL;
    }

    if (wsq->divider != wsq->last) {
        int strLen = strnlen_safe(wsq->divider->next->str, MAX_PRINT_LEN);

        if (strLen < 0) {
            i5TraceError("Bad string in wsq\n");
            wsq->divider = wsq->divider->next;
            return NULL;
        }

        *str = malloc(strLen+1);
        if (!*str)
            return NULL;

        strcpy(*str, wsq->divider->next->str);
        if (len) {
            *len = strLen;
        }
        wsq->divider = wsq->divider->next;
        return *str;
    }
    return NULL;
}

// called from ieee1905 context:
static int i5JsonWebSocketQPush( WebSocketQ *wsq, const char *str ) {
    int strLen = strnlen_safe(str, MAX_PRINT_LEN);
    I5WsQElem *elem;

    if (strLen >= MAX_PRINT_LEN || strLen < 0) {
        i5TraceError("Error -- string length exceeded %d (%*s)\n", MAX_PRINT_LEN, MAX_PRINT_LEN, str);
        return -1;
    }

    elem = malloc(sizeof(I5WsQElem));

    if (!elem) {
        i5TraceError("Could not allocate buffer %d %*s\n", strLen, MAX_PRINT_LEN, str);
        return -1;
    }

    elem->str = malloc(strLen+1);
    if (!elem->str) {
        i5TraceError("Could not allocate buffer %d %*s\n", strLen, MAX_PRINT_LEN, str);
        return -1;
    }

    strcpy(elem->str, str);
    elem->next = NULL;

    wsq->last->next = elem;
    wsq->last = elem;

    // delayed clean up:  clean up everything upto the divider (do not free divider)
    while (wsq->first != wsq->divider) {
        I5WsQElem *freeElem = wsq->first;
        wsq->first = wsq->first->next;
        free(freeElem->str);
        free(freeElem);
    }
    return 0;
}

// -------------- end websocket queue: ---------------

int i5JsonWebSocketsWrite(const char * str) {
    int rc = 0;

    if (!str || strnlen_safe(str, 1) == 0 ){
        i5TraceError("");
        return -1;
    }

    // this should push to all wsq's (but because we only have one, just push to
    // global queue).
    if (gWsq.wsi) {
        rc = i5JsonWebSocketQPush(&gWsq, str);
        lws_callback_on_writable(gWsq.wsi);
    }
    return rc;
}

static int i5JsonWebSocketHttpCallback
    (struct lws *wsi,
     enum lws_callback_reasons reason,
     void *user,
     void *in,
     size_t len)
{
    static int lastReason = 0;
    if (reason != lastReason)
        i5TraceInfo("i5JsonWebSocketHttpCallback: reason %d\n", reason);
    return 0;
}

static int i5JsonWebSocketsCallback
    (struct lws *wsi,
     enum lws_callback_reasons reason,
     void *user,
     void *in,
     size_t len)
{
    int ret = 0;
    WebSocketQ *wsq = getWsq(wsi);
    I5WsUser *wsu = (I5WsUser *)user;

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
            // after server has completed handshake with client:
            i5TraceInfo("LWS_CALLBACK_ESTABLISHED\n");
            i5JsonWebSocketQInit(wsi);
            i5TraceAssert(wsu != NULL);
            wsu->sessionKey = 0;
            break;
        case LWS_CALLBACK_CLOSED:
            // when the websocket session ends
            i5TraceInfo("LWS_CALLBACK_CLOSED\n");
            i5JsonWebSocketQDeinit(wsq);
            break;
        case LWS_CALLBACK_RECEIVE:
            // data has appeared from the server for the client connection, it can be found at *in and is len bytes long
            {
                int ver1, ver2;
                char method[64];
                char rsp[128];
                int id;
                int rc;
                int arg;

                i5TraceAssert(wsu != NULL);
                i5TraceInfo("LWS_CALLBACK_RECEIVE %s\n", (const char *)in);

                // Ok, kludging this one: I don't want to include a full JSON or RPC library:
                rc = sscanf((const char *)in, " { \"jsonrpc\" : \"%d.%d\" , \"method\" : \"%64[^\"]\" , \"id\" : \"%d\" }", &ver1, &ver2, method, &id);
                if (wsu->sessionKey && (rc != 4)) {
                    i5TraceError("Error parsing json string (%s)\n", (const char *)in);
                    break;
                } else {
                    if (wsu->sessionKey == 0) {
                        // haven't validated session key yet.  Must do this first:
                        rc = sscanf((const char *)method, "sessionKey=%hu", &wsu->sessionKey);
                        if (!wsu->sessionKey || !hsl_checkSessionKey(wsu->sessionKey)) {
                            i5TraceError("Error establishing sessionKey -- disconnecting (%s)\n", (const char *)in);
                            ret = -1;
                            //libwebsocket_context_destroy(webSocketContext);  //TBD: is there a better api to close this?
                            break;
                        }
                        snprintf(rsp, sizeof(rsp), "{\"jsonrpc\":\"2.0\",\"result\":\"1\",\"id\":%d}",id);
                        lws_write(wsi, (unsigned char *)rsp, strlen(rsp), LWS_WRITE_TEXT);
                    }
                    else if (strcmp("1905init", method) == 0) {
                        // send entire diagram
                        i5TraceInfo("%s\n", method);
                        i5JsonPrintAll(wsi);
                        snprintf(rsp, sizeof(rsp), "{\"jsonrpc\":\"2.0\",\"result\":\"1\",\"id\":%d}",id);
                        lws_write(wsi, (unsigned char *)rsp, strlen(rsp), LWS_WRITE_TEXT);
                        lws_callback_on_writable(wsi);
                    }
                    else if (sscanf(method, "hideNeighbors=%d", &arg) == 1) {
                        i5TraceInfo("%s\n", method);
                        i5JsonConfigLegacyDisplay(I5_JSON_ALL_CLIENTS, arg);
                    }
                    else if (strcmp(method, "toggleNeighbors") == 0) {
                        i5TraceInfo("%s\n", method);
                        i5JsonConfigLegacyDisplay(I5_JSON_ALL_CLIENTS, !i5_config.jsonLegacyDisplayEnabled);
                    }
                    else if (strcmp("1905delta", method) == 0) {
                        i5TraceInfo("%s\n", method);
                    }
                    else {
                        i5Trace("Unknown method %s\n", method);
                        snprintf(rsp, sizeof(rsp), "{\"jsonrpc\":\"2.0\",\"result\":\"0\",\"id\":%d}",id);
                        lws_write(wsi, (unsigned char *)rsp, strlen(rsp), LWS_WRITE_TEXT);
                        lws_callback_on_writable(wsi);
                    }
                }
            }
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            //you can write more down the http protocol link now.
            if (wsu->sessionKey == 0) {
                // do not write anything out until the session is validated.
                break;
            }
            i5TraceInfo("LWS_CALLBACK_SERVER_WRITEABLE\n");
            {
                char *str = NULL;
                size_t strLen;
                int  bytesSent;

                while (i5JsonWebSocketQPop(wsq, &str, &strLen)) {
                    //   For data being sent on a websocket connection (ie, not default http), this buffer MUST have
                    //   LWS_SEND_BUFFER_PRE_PADDING bytes valid BEFORE the pointer and an additional
                    //   LWS_SEND_BUFFER_POST_PADDING bytes valid in the buffer after
                    unsigned char *buf = malloc(LWS_SEND_BUFFER_PRE_PADDING+strLen+1+LWS_SEND_BUFFER_POST_PADDING);
                    unsigned char *bufStr = buf+LWS_SEND_BUFFER_PRE_PADDING;

                    if (strLen <= 0) {
                        i5TraceError("bad strLen (%d)\n", strLen);
                        continue;
                    }

                    if (buf != NULL) {
                        memcpy(bufStr, str, strLen+1);
                        free(str);
                        bytesSent=lws_write(wsi, bufStr, strLen, LWS_WRITE_TEXT);
                        if (bytesSent <= 0) {
                            i5TraceError("Error sending to libwebsocket (%d)\n", bytesSent);
                            break;
                        }
                        else if(bytesSent < strLen) {
                            // entire buffer could not be sent.  libwebsockets has queued the buffer to
                            // be sent later.  libwebsocket_callback_on_writable has been called, so this
                            // will fix itself.
                            i5Trace("could not send entire string -- breaking (bytesSent=%d, str=%.200s)\n", bytesSent, str);
                            break;
                        }
                        else {
                            free(buf);
                        }
                    }
                    else {
                        i5TraceError("Could not allocate buffer for websocket\n");
                    }

                    // This is to address a bug in libwebsockets:
                    // automatically break out of the send loop to prevent overflowing
                    // the libwebsocket buffer.   Call libwebsocket_callback_on_writable
                    // to ensure we send stuff if the Q is not empty...
                    lws_callback_on_writable(wsi);
                    break;
                }
            }
            break;

        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            i5TraceInfo("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION\n");
            if (wsq) {
                i5Trace("Rejecting new connection -- old connection still active\n");
                ret = -1;
                break;
            }
            break;
        case LWS_CALLBACK_PROTOCOL_INIT:
            // One-time call per protocol so it can do initial setup / allocations etc
            if (wsu != NULL) {
                memset(wsu, 0, sizeof(I5WsUser));
                wsu->hasStarted=1;
            }
            i5TraceInfo("LWS_CALLBACK_PROTOCOL_INIT\n");
            break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            gWebSocketContext = NULL;   // this has already been freed from within the websocket lib
            i5TraceInfo("LWS_CALLBACK_PROTOCOL_DESTROY\n");
            i5JsonWebSocketsDeinit();

            if (wsu == NULL || wsu->hasStarted != 1) {
                 // race condition: 1905 was likely started before websockets was ready.  In this
                 // case it appears as though the DESTROY call is triggered without the INIT call
                 // retry in 1s
                 gWebSocketRestartTimer = i5TimerNew(I5_JSON_WEBSOCK_RESTART_IN_MS, i5JsonWebSocketsRestart, NULL);
            }
            break;
        case LWS_CALLBACK_GET_THREAD_ID:
            /*
             * if you will call "libwebsocket_callback_on_writable"
             * from a different thread, return the caller thread ID
             * here so lws can use this information to work out if it
             * should signal the poll() loop to exit and restart early
             */

            /* return pthread_getthreadid_np(); */
            break;

        default:
            i5TraceInfo("Unahandled websocket message (%d) (ignoring) \n", reason);
            break;
    }

    return ret;
}

static struct lws_protocols webSocketProtocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",                    /* name */
        i5JsonWebSocketHttpCallback,    /* callback */
        0,                              /* per_session_data_size */
        0,                              /* max frame size / rx buffer */
    },
    {
        "1905_protocol",                    /* name */
        i5JsonWebSocketsCallback,           /* callback */
        sizeof(I5WsUser),                   /* per_session_data_size */
        0,                              /* max frame size / rx buffer */
    },
    { //terminator:
        NULL,
        NULL,
        0,
        0,
    }
};

int i5JsonWebSocketsInit() {
    int ret = 0;
    int opts = LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
    const char *iface = NULL;
    struct lws_context_creation_info info;

    i5Trace("\n");
    memset(&info, 0, sizeof(info));
    info.port = WSD_PORT;

    info.iface = iface;
    info.protocols = webSocketProtocols;

    info.ssl_cert_filepath = "/var/tmp/newkey.cert";
    info.ssl_private_key_filepath = "/var/tmp/newkey";

    info.gid = -1;
    info.uid = -1;
    info.options = opts;

    gWebSocketContext = lws_create_context(&info);
    if (gWebSocketContext == NULL)
    {
        i5TraceError("Websockets server initialization failed\n");
        ret = -1;
    }

    gWebSocketPollTimer = i5TimerNew(I5_JSON_WEBSOCK_POLL_IN_MS, i5JsonWebSocketsPoll, NULL);

    return ret;
}

void i5JsonWebSocketsDeinit(void) {
    i5TimerFree(gWebSocketPollTimer);
    i5TimerFree(gWebSocketRestartTimer);
    if (gWebSocketContext != NULL)
    {
        lws_context_destroy(gWebSocketContext);
    }
    return;
}

void i5JsonWebSocketsPoll(void * arg) {
    // notice: polling can be avoided if we start a new thread.

    if ( arg ) {
      i5TimerFree((timer_elem_type *)arg);
    }
    lws_service(gWebSocketContext, 0);
    gWebSocketPollTimer = i5TimerNew(I5_JSON_WEBSOCK_POLL_IN_MS, i5JsonWebSocketsPoll, NULL);
}

void i5JsonWebSocketsRestart(void * arg) {
    if ( arg ) {
      i5TimerFree((timer_elem_type *)arg);
    }
    i5JsonWebSocketsInit();
}

static int _areWebSocketsActive() {
    return !!gWsq.wsi;
}

#else //defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)

static int _areWebSocketsActive() {
    return 0;
}
#endif //else defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)

// returns a negative number on error, or the number of characters printed.
static int _sock_printf(void *client, const char *fmt, ...)
{
    va_list arglist;
    int rc = 0;
    char str[MAX_PRINT_LEN];
    int strLen;
    i5_socket_type *next;
    i5_socket_type *socket;

    char activeListener = 0;

    va_start(arglist, fmt);
    strLen=vsnprintf(str, MAX_PRINT_LEN-1, fmt, arglist);
    va_end(arglist);

    i5TraceInfo("%s\n",str);

    if (strLen > 0) {
#if defined(SUPPORT_1905_TOPOLOGY_WEB_PAGE)
        if (_areWebSocketsActive() && (client == I5_JSON_ALL_CLIENTS || client == gWsq.wsi)) {
            rc = i5JsonWebSocketsWrite(str);
            activeListener = 1;
        }
        else
#endif // endif
        {
            socket = i5_config.i5_socket_list.ll.next;
            while (socket != NULL) {
              next = socket->ll.next;
              if (socket->type == i5_socket_type_json) {
                if (client == NULL || client == socket) {
                    activeListener = 1;
                    rc = fputs(str, socket->u.sinr.fp);
                    i5JsonFflush(socket);
                    if (client == socket)
                        break;
                }
              }
              socket = next;
            }
        }
    }

    if (rc < 0) {
        i5Trace("_sock_printf failed (%d) - %s, %d\n", rc, fmt, strLen);
    }

    if (activeListener) {
      i5DmLinkMetricsActivate();
    }

    return rc < 0 ? rc : strLen;
}
// -------------------------------------------------------------
