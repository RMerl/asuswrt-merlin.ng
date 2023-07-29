
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>

#include <sys/select.h>

#include <pthread.h>


#include <arpa/inet.h>
#include "ipc.h"
#include "mnt.h"



CM_CTRL cm_ctrlBlock;


static void cm_closeSocket(CM_CTRL *pCtrlBK);
static int cm_openSocket(CM_CTRL *pCtrlBK);

/*
========================================================================
Routine Description:
  Open socket.

Arguments:
  *pCtrlBK  - CM control blcok

Return Value:
  1   - open successfully
  0   - open fail

Note:
========================================================================
*/
static int cm_openSocket(CM_CTRL *pCtrlBK)
{

  struct sockaddr_un sock_addr_ipc;
  int broadcast = 1;
  int reused = 1;

  /* init */
  pCtrlBK->socketTCPSend = -1;
  pCtrlBK->socketIpcSendRcv = -1;

  /* IPC Socket */
  if ( (pCtrlBK->socketIpcSendRcv = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    printf("%s Failed to IPC socket create!\n", __FUNCTION__);
    goto err;
  }

  memset(&sock_addr_ipc, 0, sizeof(sock_addr_ipc));
  sock_addr_ipc.sun_family = AF_UNIX;

  printf("%s AWSIOT_IPC_SOCKET_PATH = %s\n", __FUNCTION__, AWSIOT_IPC_SOCKET_PATH);

  snprintf(sock_addr_ipc.sun_path, sizeof(sock_addr_ipc.sun_path), "%s", AWSIOT_IPC_SOCKET_PATH);
  int status = unlink(AWSIOT_IPC_SOCKET_PATH);
  

  if (bind(pCtrlBK->socketIpcSendRcv, (struct sockaddr*)&sock_addr_ipc, sizeof(sock_addr_ipc)) < -1) {
    printf("%s Failed to IPC socket bind!\n", __FUNCTION__);
    goto err;
  }

  if (listen(pCtrlBK->socketIpcSendRcv, AWSIOT_IPC_MAX_CONNECTION) == -1) {
    printf("%s Failed to IPC socket listen!\n", __FUNCTION__);
    goto err;
  }

  printf("%s pCtrlBK->socketIpcSendRcv = %d\n", __FUNCTION__, pCtrlBK->socketIpcSendRcv);

  return 1;

err:
  cm_closeSocket(pCtrlBK);
  return 0;
} /* End of cm_openSocket */




void *cm_rcvPacket(void *args)
{
  CM_CTRL *pCtrlBK = &cm_ctrlBlock;

  /* init */
  memset(pCtrlBK, 0, sizeof(CM_CTRL));

  pthread_detach(pthread_self());

  printf("%s pCtrlBK->enter\n", __FUNCTION__);

  /* init role */
  // pCtrlBK->role = IS_CLIENT;
  pCtrlBK->role = IS_SERVER;
  pCtrlBK->cost = -1;

  /* init socket */
  if (cm_openSocket(pCtrlBK) == 0) {
    printf("%s err\n", __FUNCTION__);
  }


  /* init */
  pCtrlBK->flagIsTerminated = 0;

  /* waiting for CM packets */
  while(!pCtrlBK->flagIsTerminated)
  {
    cm_rcvHandler(pCtrlBK);
  } 

  printf("%s leave\n", __FUNCTION__);

  pthread_exit(NULL);
} /* End of cm_rcvPacket */



/*
========================================================================
Routine Description:
  Handle received CM packets.

Arguments:
  *pCtrlBK  - CM control blcok

Return Value:
  None

Note:
========================================================================
*/
void cm_rcvHandler(CM_CTRL *pCtrlBK)
{
  fd_set fdSet;
  int sockMax;

  /* sanity check */
  if (pCtrlBK->flagIsRunning)
    return;

  /* init */
  pCtrlBK->flagIsRunning = 1;

  sockMax = pCtrlBK->socketTCPSend;


  if (pCtrlBK->socketIpcSendRcv > sockMax)
    sockMax = pCtrlBK->socketIpcSendRcv;


  printf("%s sockMax = %d\n", __FUNCTION__, sockMax);

  /* waiting for any packet */
  while(1)
  {
    /* must re- FD_SET before each select() */
    FD_ZERO(&fdSet);

    FD_SET(pCtrlBK->socketIpcSendRcv, &fdSet);

    /* must use sockMax+1, not sockMax */
    if (select(sockMax+1, &fdSet, NULL, NULL, NULL) < 0)
      break;


    /* handle packets from IPC */
    if (FD_ISSET(pCtrlBK->socketIpcSendRcv, &fdSet)) {
      cm_rcvIpcHandler(pCtrlBK->socketIpcSendRcv);
    }
  };

  pCtrlBK->flagIsRunning = 0;
} /* End of cm_rcvHandler */




static void cm_closeSocket(CM_CTRL *pCtrlBK)
{

  if (pCtrlBK->socketIpcSendRcv >= 0)
    close(pCtrlBK->socketIpcSendRcv);

} /* End of cm_closeSocket */


void awsiot_ipc_start()
{
  pthread_t sockThread;

  printf("%s startThread\n", __FUNCTION__);

  /* start thread to receive packet */
  if (pthread_create(&sockThread, NULL, cm_rcvPacket, NULL) < 0) {
    printf("%s could not create thread for sockThread\n", __FUNCTION__);
  }

  printf("%s sockThread = %u\n", __FUNCTION__, (unsigned int)sockThread);
}