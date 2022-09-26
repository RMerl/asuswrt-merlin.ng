
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>

#include <sys/select.h>

#include <arpa/inet.h>
#include <pthread.h>


// #include "uploader_config.h"


#include "uploader_ipc.h"
#include "uploader_mnt.h"

#include "log.h"


#define APC_DBG 1

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



  Cdbg(APC_DBG, "AF_UNIX = %d", AF_UNIX);
  Cdbg(APC_DBG, "SOCK_STREAM = %d", SOCK_STREAM);
  Cdbg(APC_DBG, "socket = %d", socket(AF_UNIX, SOCK_STREAM, 0));


  /* IPC Socket */
  if ( (pCtrlBK->socketIpcSendRcv = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    Cdbg(APC_DBG, "Failed to IPC socket create!");
    goto err;
  }

  memset(&sock_addr_ipc, 0, sizeof(sock_addr_ipc));
  sock_addr_ipc.sun_family = AF_UNIX;

  Cdbg(APC_DBG, "UPLOADER_IPC_SOCKET_PATH = %s", UPLOADER_IPC_SOCKET_PATH);

  snprintf(sock_addr_ipc.sun_path, sizeof(sock_addr_ipc.sun_path), "%s", UPLOADER_IPC_SOCKET_PATH);
  int status = unlink(UPLOADER_IPC_SOCKET_PATH);
  
  Cdbg(APC_DBG, "unlink status = %d\n", status);

  if (bind(pCtrlBK->socketIpcSendRcv, (struct sockaddr*)&sock_addr_ipc, sizeof(sock_addr_ipc)) < -1) {
    Cdbg(APC_DBG, "Failed to IPC socket bind!");
    goto err;
  }

  if (listen(pCtrlBK->socketIpcSendRcv, MASTIFF_IPC_MAX_CONNECTION) == -1) {
    Cdbg(APC_DBG, "Failed to IPC socket listen!");
    goto err;
  }


  Cdbg(APC_DBG, "pCtrlBK->socketIpcSendRcv = %d", pCtrlBK->socketIpcSendRcv);

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

  Cdbg(APC_DBG, "cm_rcvPacket enter");

  /* init role */
  // pCtrlBK->role = IS_CLIENT;
  pCtrlBK->role = IS_SERVER;
  pCtrlBK->cost = -1;


  /* get interface info */
  // if (!cm_getIfInfo(pCtrlBK)) {
  //   IOT_INFO("interface information failed");
    
  // }

  /* init socket */
  if (cm_openSocket(pCtrlBK) == 0) {
    Cdbg(APC_DBG, "cm_openSocket err");
  }


  /* init */
  pCtrlBK->flagIsTerminated = 0;

  /* waiting for CM packets */
  while(!pCtrlBK->flagIsTerminated)
  {
    cm_rcvHandler(pCtrlBK);
  } 

  Cdbg(APC_DBG, "cm_rcvPacket leave");

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


  Cdbg(APC_DBG, "sockMax(%d)", sockMax);

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

  // if (pCtrlBK->socketTCPSend >= 0)
  //   close(pCtrlBK->socketTCPSend);

  // if (pCtrlBK->socketUdpSendRcv >= 0)
  //   close(pCtrlBK->socketUdpSendRcv);

  if (pCtrlBK->socketIpcSendRcv >= 0)
    close(pCtrlBK->socketIpcSendRcv);

} /* End of cm_closeSocket */



void uploader_ipc_start()
{
  pthread_t sockThread;

  Cdbg(APC_DBG, "startThread");

  /* start thread to receive packet */
  if (pthread_create(&sockThread, NULL, cm_rcvPacket, NULL) < 0) {
    Cdbg(APC_DBG, "could not create thread for sockThread");
  }


  Cdbg(APC_DBG, "sockThread = %u",  (unsigned int)sockThread);
}