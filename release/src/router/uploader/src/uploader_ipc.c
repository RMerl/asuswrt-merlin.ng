
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>

#include <sys/select.h>

#include <arpa/inet.h>
#include <pthread.h>

#include "json.h"

#include "uploader_ipc.h"

#include "upload_api.h"


#include "log.h"


extern char mac_no_symbol[32];


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

    Cdbg(APC_DBG, "Get ipc Packet msg = %s", args);

    // publish_shadow_remote_connection(1, args);
    
    // goto err;

    json_object *root = NULL;
    json_object *o_api = NULL, *o_file_path = NULL, *o_file_name = NULL;

    struct ipcArgStruct *ipcArgs = (struct ipcArgStruct *)args;
    unsigned char *pPktBuf = NULL;

    if (IsNULL_PTR(ipcArgs->data)) {
        Cdbg(APC_DBG, "cm_ipcPacketHandler data is null!");
        goto err;
    }

    pPktBuf = &ipcArgs->data[0];

    Cdbg(APC_DBG, "Get ipc Packet msg = %s", (char *)pPktBuf);

    // publish_shadow_remote_connection(1, args);

    // goto err;

    root = json_tokener_parse((char *)pPktBuf);

    if (root) {


      char timestamp_str[16] = {0};
      getTimeInMillis(timestamp_str);

      Cdbg(APC_DBG, "timestamp_str = %s", timestamp_str);

      /* create temp folder */
      if(!check_if_dir_exist(UPLOADER_FOLDER)) {

          Cdbg(APC_DBG, "create temp folder for uploader (%s)", UPLOADER_FOLDER);
          mkdir(UPLOADER_FOLDER, 0755);
      }


      json_object_object_get_ex(root, "api", &o_api);
      json_object_object_get_ex(root, "file_path", &o_file_path);
      json_object_object_get_ex(root, "file_name", &o_file_name);


      const char *api = json_object_get_string(o_api);
      const char *file_path = json_object_get_string(o_file_path);
      const char *file_name = json_object_get_string(o_file_name);


      char msg[512] = {0};
      snprintf(msg, sizeof(msg) ,"{\"api\":\"%s\", \"file_path\":\"%s\",\"file_name\":\"router_%s_%s.cfg\"}", api, UPLOADER_FOLDER, mac_no_symbol, timestamp_str);

      Cdbg(APC_DBG, "cm_ipcPacketHandler write msg = %s", msg);

      char upload_file[128] = {0};

      snprintf(upload_file, sizeof(upload_file), "%srouter_%s_%s.cfg", UPLOADER_FOLDER, mac_no_symbol, timestamp_str);

      write_file(upload_file, msg);


      // ipc_api_process(api, file_path, file_name);


      json_object_put(root);


    } else {
      Cdbg(APC_DBG, "root is invalid");
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

  Cdbg(APC_DBG, "cm_rcvIpcHandler enter");

  clientSock = accept(sock, NULL, NULL);

  if (clientSock < 0) {
    Cdbg(APC_DBG, "cm_rcvIpcHandler Failed to socket accept() !!!");
    return;
  }

  /* handle the packet */
  if ((len = read(clientSock, pPktBuf, sizeof(pPktBuf))) <= 0) {
    Cdbg(APC_DBG, "cm_rcvIpcHandler Failed to socket read()!!");
    close(clientSock);
    return;
  }

  close(clientSock);

  args = malloc(sizeof(struct ipcArgStruct));
  memset(args, 0, sizeof(struct ipcArgStruct));
  memcpy(args->data, (unsigned char *)&pPktBuf[0], len);
  args->dataLen = len;

  Cdbg(APC_DBG, "cm_rcvIpcHandler create thread for handle ipc packet");
  if (pthread_create(&sockThread, attrp, cm_ipcPacketHandler, args) < 0) {
    Cdbg(APC_DBG, "could not create thread !!!");
    free(args);
  }

  Cdbg(APC_DBG, "cm_rcvIpcHandler leave");
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

  Cdbg(APC_DBG, "ncm_sendIpcHandler enter");

  Cdbg(APC_DBG, "enter AF_UNIX = %d", AF_UNIX);
  Cdbg(APC_DBG, "enter SOCK_STREAM = %d", SOCK_STREAM);
  
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    Cdbg(APC_DBG, "ipc socket error!");
    goto err;
  }

  /* set NONBLOCK for connect() */
  if ((flags = fcntl(fd, F_GETFL)) < 0) {
    Cdbg(APC_DBG, "F_GETFL error!");
    goto err;
  }


  flags |= O_NONBLOCK;


  if (fcntl(fd, F_SETFL, flags) < 0) {
    Cdbg(APC_DBG, "F_SETFL error!");
    goto err;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, ipcPath, sizeof(addr.sun_path)-1);

  Cdbg(APC_DBG, "addr.sun_path  = %s", addr.sun_path);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

    if (errno == EINPROGRESS) {
      FD_ZERO(&writeFds);
      FD_SET(fd, &writeFds);

      selectRet = select(fd + 1, NULL, &writeFds, NULL, &timeout);

      //Check return, -1 is error, 0 is timeout
      if (selectRet == -1 || selectRet == 0) {
        Cdbg(APC_DBG, "ipc connect error");
        goto err;
      }
    }
    else
    {
      Cdbg(APC_DBG, "ipc connect error");
      goto err;
    }
  }

  /* check the status of connect() */
  status = 0;
  statusLen = sizeof(status);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &statusLen) == -1) {
    Cdbg(APC_DBG, "getsockopt(SO_ERROR): %s", strerror(errno));
    goto err;
  }

  length = write(fd, data, dataLen);

  if (length < 0) {
    Cdbg(APC_DBG, "error writing:%s", strerror(errno));
    goto err;
  }

  ret = 1;

  Cdbg(APC_DBG, "send data out (%s) via (%s)", data, ipcPath);

err:
  if (fd >= 0)
    close(fd);

  Cdbg(APC_DBG, "cm_sendIpcHandler leave");
  return ret;
} /* End of cm_sendIpcHandler */
