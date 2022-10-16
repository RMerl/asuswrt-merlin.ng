
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>

#include <sys/select.h>

#include <arpa/inet.h>
#include <pthread.h>

#include "json.h"

#include "ipc.h"


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

    printf("cm_ipcPacketHandler msg(%s)\n", args);


    if (IsNULL_PTR(args)) {
        printf("cm_ipcPacketHandler data is null!\n");
        goto err;
    }

    json_object *root = NULL;
    json_object *o_topic = NULL, *o_msg = NULL;
    
    root = json_tokener_parse(args);


    if (root) {

      json_object_object_get_ex(root, "topic", &o_topic);
      json_object_object_get_ex(root, "msg", &o_msg);
       
      char *topic = json_object_get_string(o_topic);
      char *msg = json_object_get_string(o_msg);


      printf("%s topic = %s\n", __FUNCTION__, topic);
      printf("%s msg = %s\n", __FUNCTION__, msg);

      json_object_put(root);

    } else {
      printf("%s root is invalid\n", __FUNCTION__);
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

  printf("%s enter\n", __FUNCTION__);

  clientSock = accept(sock, NULL, NULL);

  if (clientSock < 0) {
    printf("%s Failed to socket accept() !!!\n", __FUNCTION__);
    return;
  }

  /* handle the packet */
  if ((len = read(clientSock, pPktBuf, sizeof(pPktBuf))) <= 0) {
    printf("%s Failed to socket accept() !!!\n", __FUNCTION__);
    close(clientSock);
    return;
  }

  close(clientSock);

  args = malloc(sizeof(struct ipcArgStruct));
  memset(args, 0, sizeof(struct ipcArgStruct));
  memcpy(args->data, (unsigned char *)&pPktBuf[0], len);
  args->dataLen = len;

  printf("%s create thread for handle ipc packet\n", __FUNCTION__);
  if (pthread_create(&sockThread, attrp, cm_ipcPacketHandler, args) < 0) {
    printf("%s could not create thread !!!\n", __FUNCTION__);
    free(args);
  }

  printf("%s leave !!!\n", __FUNCTION__);
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

  printf("%s enter\n", __FUNCTION__);

  
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    printf("%s ipc socket error!\n", __FUNCTION__);
    goto err;
  }

  /* set NONBLOCK for connect() */
  if ((flags = fcntl(fd, F_GETFL)) < 0) {
    printf("%s F_GETFL error!\n", __FUNCTION__);
    goto err;
  }


  flags |= O_NONBLOCK;

  if (fcntl(fd, F_SETFL, flags) < 0) {
    printf("%s F_SETFL error!\n", __FUNCTION__);
    goto err;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, ipcPath, sizeof(addr.sun_path)-1);

  printf("%s ipcPath  = %s\n", __FUNCTION__, ipcPath);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

    if (errno == EINPROGRESS) {
      FD_ZERO(&writeFds);
      FD_SET(fd, &writeFds);

      selectRet = select(fd + 1, NULL, &writeFds, NULL, &timeout);

      //Check return, -1 is error, 0 is timeout
      if (selectRet == -1 || selectRet == 0) {
        printf("%s ipc connect error\n", __FUNCTION__);
        goto err;
      }
    }
    else
    {
      printf("%s ipc connect error\n", __FUNCTION__);
      goto err;
    }
  }

  /* check the status of connect() */
  status = 0;
  statusLen = sizeof(status);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &statusLen) == -1) {
    printf("%s getsockopt(SO_ERROR): %s\n", __FUNCTION__, strerror(errno));
    goto err;
  }

  length = write(fd, data, dataLen);

  if (length < 0) {
    printf("%s error writing: %s\n", __FUNCTION__, strerror(errno));
    goto err;
  }

  ret = 1;

  printf("%s send data out (%s) via (%s)\n", __FUNCTION__, data, ipcPath);

err:
  if (fd >= 0)
    close(fd);

  printf("%s cm_sendIpcHandler leave\n", __FUNCTION__);
  return ret;
}