#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>
#include <string.h>
#include <sys/select.h>

#include <arpa/inet.h>
#include <pthread.h>

#include "json.h"

#include "awsiot_config.h"
#include "awsiot_ipc.h"

#include "awsiot.h"

#include "log.h"

#define APC_DBG 1

pthread_attr_t *attrp;

/*
========================================================================
Routine Description:
  Create a thread to handle received packets from ipc socket.

Arguments:
  *args   - arguments for socket

Return Value:
  None

Note:
========================================================================
*/

void *cm_ipcPacketHandler(void *args)
{

    pthread_detach(pthread_self());


    LogInfo( ( "cm_ipcPacketHandler msg(%s)", args) );
    Cdbg(APC_DBG, "Get ipc Packet msg = %s", args);




    // publish_shadow_remote_connection(1, args);
    
    // goto err;


    // struct ipcArgStruct *ipcArgs = (struct ipcArgStruct *)args;
    // unsigned char *pPktBuf = NULL;

    // if (IsNULL_PTR(ipcArgs->data)) {
    //     IOT_INFO("cm_ipcPacketHandler data is null!");
    //     goto err;
    // }

    // pPktBuf = &ipcArgs->data[0];

    // IOT_INFO("cm_ipcPacketHandler msg(%s)", (char *)pPktBuf);
    // Cdbg(APC_DBG, "Get ipc Packet msg = %s", (char *)pPktBuf);

    // publish_shadow_remote_connection(1, args);

    // goto err;

    if (IsNULL_PTR(args)) {
        LogInfo( ( "cm_ipcPacketHandler data is null!") );
        Cdbg(APC_DBG, "cm_ipcPacketHandler data is null!");
        goto err;
    }

    json_object *root = NULL;
    json_object *o_func_name = NULL;
    root = json_tokener_parse(args);

    if (root) {
      json_object_object_get_ex(root, "function_name", &o_func_name);

      if (o_func_name) {
        const char *func_name = json_object_get_string(o_func_name);
        
        if (!strcmp(func_name, "send_message")) {
          json_object *o_topic = NULL;
          json_object *o_msg = NULL;
          json_object_object_get_ex(root, "topic", &o_topic);
          json_object_object_get_ex(root, "msg", &o_msg);

          if (o_topic && o_msg) {
            const char *topic = json_object_get_string(o_topic);
            const char *msg = json_object_get_string(o_msg);
            
            Cdbg(APC_DBG, "topic = %s", topic);
            Cdbg(APC_DBG, "msg = %s", msg);

            publish_router_service_topic(topic, msg);
          } else {
            Cdbg(APC_DBG, "topic or msg is invalid");
          }
        } else if (!strcmp(func_name, "tunnel_status")) {
          char result[64];
          json_object *o_status = NULL;
          json_object_object_get_ex(root, "status", &o_status);
          if (o_status) {
            snprintf(result, sizeof(result), "{\"aae_sip_connected\":\"%s\"}", json_object_get_string(o_status));
            Cdbg(APC_DBG, "%s %s", __FUNCTION__, result);
            publish_shadow_remote_connection(1, result);
          } else {
            LogInfo( ( "%s status is invalid", __FUNCTION__) );
          }
        }
      }

      json_object_put(root);

    } else {
      LogInfo( ( "%s root is invalid", __FUNCTION__) );
      
    }


err:

  free(args);

  pthread_exit(NULL);

} /* End of cm_ipcPacketHandler */



/*
========================================================================
Routine Description:
  Handle received packets from IPC socket.

Arguments:
  sock    - sock fd for IPC

Return Value:
  None

Note:
========================================================================
*/
void cm_rcvIpcHandler(int sock)
{
  int clientSock = 0;
  pthread_t sockThread;
  struct ipcArgStruct *args = NULL;
  unsigned char pPktBuf[MAX_IPC_PACKET_SIZE] = {0};
  int len = 0;

  LogInfo( ( "cm_rcvIpcHandler enter") );

  clientSock = accept(sock, NULL, NULL);

  if (clientSock < 0) {
    LogInfo( ( "cm_rcvIpcHandler Failed to socket accept() !!!") );
    return;
  }

  /* handle the packet */
  if ((len = read(clientSock, pPktBuf, sizeof(pPktBuf))) <= 0) {
    LogInfo( ( "cm_rcvIpcHandler Failed to socket read()!!!") );
    close(clientSock);
    return;
  }

  close(clientSock);

  args = malloc(sizeof(struct ipcArgStruct));
  memset(args, 0, sizeof(struct ipcArgStruct));
  memcpy(args->data, (unsigned char *)&pPktBuf[0], len);
  args->dataLen = len;

  LogInfo( ( "cm_rcvIpcHandler create thread for handle ipc packet") );
  if (pthread_create(&sockThread, attrp, cm_ipcPacketHandler, args) < 0) {
    LogInfo( ( "could not create thread !!!") );
    free(args);
  }

  LogInfo( ( "cm_rcvIpcHandler leave") );
} /* End of cm_rcvIpcHandler */




/*
========================================================================
Routine Description:
  Send data to specificed IPC socket path.

Arguments:
  ipcPath   - ipc socket path
  data    - data will be sent out
  dataLen   - the length of data

Return Value:
  0   - fail
  1   - success

========================================================================
*/
int cm_sendIpcHandler(char *ipcPath, char *data, int dataLen)
{
  int fd = -1;
  int length = 0;
  int ret = 0;
  struct sockaddr_un addr;
  int flags;
  int status;
  socklen_t statusLen;
  fd_set writeFds;
  int selectRet;
  struct timeval timeout = {2, 0};

  // DBG_INFO("enter");
  LogInfo( ( "\ncm_sendIpcHandler enter") );

  LogInfo( ( "enter AF_UNIX = %d", AF_UNIX) );
  LogInfo( ( "enter SOCK_STREAM = %d", SOCK_STREAM) );
  
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    // DBG_ERR("ipc socket error!");
    LogInfo( ( "ipc socket error!") );
    goto err;
  }

  /* set NONBLOCK for connect() */
  if ((flags = fcntl(fd, F_GETFL)) < 0) {
    // DBG_ERR("F_GETFL error!");
    LogInfo( ( "F_GETFL error!") );
    goto err;
  }

  LogInfo( ( "flags = %d", flags) );

  flags |= O_NONBLOCK;

  LogInfo( ( "flags = %d", flags) );

  if (fcntl(fd, F_SETFL, flags) < 0) {
    // DBG_ERR("F_SETFL error!");
    LogInfo( ( "F_SETFL error!") );
    goto err;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, ipcPath, sizeof(addr.sun_path)-1);

  LogInfo( ( "addr.sun_path  = %s", addr.sun_path) );

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

    if (errno == EINPROGRESS) {
      FD_ZERO(&writeFds);
      FD_SET(fd, &writeFds);

      selectRet = select(fd + 1, NULL, &writeFds, NULL, &timeout);

      //Check return, -1 is error, 0 is timeout
      if (selectRet == -1 || selectRet == 0) {
        // DBG_ERR("ipc connect error");
        LogInfo( ( "000 ipc connect error") );
        goto err;
      }
    }
    else
    {
      // DBG_ERR("ipc connect error");
      LogInfo( ( "111 ipc connect error") );
      goto err;
    }
  }

  /* check the status of connect() */
  status = 0;
  statusLen = sizeof(status);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &statusLen) == -1) {
    // DBG_ERR("getsockopt(SO_ERROR): %s", strerror(errno));
    LogInfo( ( "getsockopt(SO_ERROR): %s", strerror(errno)) );
    goto err;
  }

  length = write(fd, data, dataLen);

  if (length < 0) {
    // DBG_ERR("error writing:%s", strerror(errno));
    LogInfo( ( "error writing:%s", strerror(errno)) );
    goto err;
  }

  ret = 1;

  // DBG_INFO("send data out (%s) via (%s)", data, ipcPath);
  LogInfo( ( "send data out (%s) via (%s)", data, ipcPath) );

err:
  if (fd >= 0)
    close(fd);

  // DBG_INFO("leave");
  LogInfo( ( "cm_sendIpcHandler leave\n") );
  return ret;
} /* End of cm_sendIpcHandler */
