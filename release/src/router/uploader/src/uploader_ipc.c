
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>

#include <sys/select.h>

#include <arpa/inet.h>
#include <pthread.h>

#include "json.h"

#include "api.h"

#include "uploader_ipc.h"

#include "upload_api.h"

#include "log.h"

extern char g_formated_router_mac[32];
extern backup_file_types BACK_FILE_TYPES[];

#define APC_DBG 1
#define MAX_RESP_RAW_LEN         2048

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

void *cm_ipcPacketHandler(void *args) {

    pthread_detach(pthread_self());

    json_object *root = NULL;
    json_object *o_func_name = NULL;

    struct aaeIpcArgStruct *ipcArgs = (struct aaeIpcArgStruct *)args;
    unsigned char *pPktBuf = NULL;

    if (IsNULL_PTR(ipcArgs->data)) {
        Cdbg(APC_DBG, "cm_ipcPacketHandler data is null!");
        goto err;
    }

    pPktBuf = &ipcArgs->data[0];

    Cdbg(APC_DBG, "Get ipc Packet msg = %s", (char *)pPktBuf);

    char raw_resp[MAX_RESP_RAW_LEN] = {0};

    root = json_tokener_parse((char *)pPktBuf);

    if (root) {
        json_object_object_get_ex(root, "function_name", &o_func_name);

        if (o_func_name) {
            const char *func_name = json_object_get_string(o_func_name);

#ifdef RTCONFIG_FU_TEST
            if (!strcmp(func_name, "list_files")) {

                json_object *root_obj = json_object_new_array();

                int i = 0;
                backup_file_types *handler = NULL;
                for(handler = &BACK_FILE_TYPES[0]; handler->id>0; handler++){

                    int len = count_backup_file(handler->bf);

                    json_object *bk_obj = json_object_new_object();
                    json_object_object_add(bk_obj, "name", json_object_new_string(handler->bf_name));
                    json_object_object_add(bk_obj, "bf_len", json_object_new_int(handler->bf_len));
                    json_object_object_add(bk_obj, "len", json_object_new_int(len));
                    json_object_object_add(bk_obj, "limit", json_object_new_int(handler->max_file_limit));

                    // Cdbg(APC_DBG, "%s, %d(%d)/%d", handler->bf_name, handler->bf_len, len, handler->max_file_limit);
                    /////////////////////////////////////////////////////

                    json_object *bf_obj = json_object_new_array();
                    backup_files* curr_file = handler->bf;
                    while (curr_file) {

                        // Cdbg(APC_DBG, "%s", curr_file->filename);  

                        json_object_array_add(bf_obj, json_object_new_string(curr_file->filename));

                        curr_file = curr_file->next;
                    }
                    
                    json_object_object_add(bk_obj, "bf", bf_obj);
                    /////////////////////////////////////////////////////

                    json_object_array_add(root_obj, bk_obj);
                }
                
                const char *json_str = json_object_to_json_string(root_obj);
                
                snprintf(raw_resp, sizeof(raw_resp), "%s", json_str);

                json_object_put(root_obj);
            }
#endif

        }

        json_object_put(root);
        
        if (ipcArgs->waitResp) {
            
            struct aaeIpcArgStruct resp;
            int length;
            memset(&resp, 0, sizeof(struct aaeIpcArgStruct));
            
            resp.dataLen = snprintf((char *)resp.data, sizeof(resp.data), "%s", raw_resp);
            
            length = send(ipcArgs->sock, &resp, sizeof(struct aaeIpcArgStruct), MSG_NOSIGNAL);
            
            if (length < 0) {
                // DBG_ERR("error writing:%s\n", strerror(errno));
                Cdbg(APC_DBG, "error writing:%s", strerror(errno));
                goto err;
            }
        }

    } 
    else {
        Cdbg(APC_DBG, "root is invalid");
    }

err:
    if (ipcArgs->sock >= 0) {
        close(ipcArgs->sock);
    }

    free(ipcArgs);

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
void cm_rcvIpcHandler(int sock) {

    int clientSock = -1;
    pthread_t sockThread;
    struct aaeIpcArgStruct *args = NULL;
    int len = 0;

    //Cdbg(IPC_DBG, "enter");

    clientSock = accept(sock, NULL, NULL);

    if (clientSock < 0) {
        // Cdbg(IPC_DBG, "aae_rcvIpcHandler Failed to socket accept() !!! (%s)", strerror(errno));
        return;
    }

    args = malloc(sizeof(struct aaeIpcArgStruct));
    memset(args, 0, sizeof(struct aaeIpcArgStruct));

    /* handle the packet */
    if ((len = read(clientSock, args, sizeof(struct aaeIpcArgStruct))) <= 0) {
        // Cdbg(IPC_DBG, "aae_rcvIpcHandler Failed to socket read()!!! (%s)", strerror(errno));
        close(clientSock);
        return;
    }

    if (!args->waitResp)
        close(clientSock);
    else {
        args->sock = clientSock;
    }
    // Cdbg(IPC_DBG, "aae_rcvIpcHandler create thread for handle ipc packet");
    pthread_attr_t attr;
    pthread_attr_init(&attr);
#ifdef PTHREAD_STACK_SIZE
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE);
#endif
    if (pthread_create(&sockThread, &attr, (void *)cm_ipcPacketHandler, args) < 0) {
        // Cdbg(IPC_DBG, "could not create thread !!!", strerror(errno));
        free(args);
    }
    pthread_attr_destroy(&attr);

    //Cdbg(IPC_DBG, "leave");
}

void xxcm_rcvIpcHandler(int sock) {

    int clientSock = 0;
    pthread_t sockThread;
    struct aaeIpcArgStruct *args = NULL;
    unsigned char pPktBuf[AAE_MAX_IPC_PACKET_SIZE] = {0};
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

    args = malloc(sizeof(struct aaeIpcArgStruct));
    memset(args, 0, sizeof(struct aaeIpcArgStruct));
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
int cm_sendIpcHandler(char *ipcPath, char *data, int dataLen) {

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
        else {
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
