/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


/*******************************************************************
 * bdmf_mon_server.c
 *
 * BDMF framework - remote shell support
 *
 * This module is a back-end of remote shell support.
 * - multiple servers
 * - domain and TCP-based connections
 * - session access level - per server
 *******************************************************************/

#include <bdmf_shell_server.h>

typedef struct bdmfmons_server bdmfmons_server_t;

/* Server connection
 */
typedef struct bdmfmons_conn
{
    struct bdmfmons_conn *next;
    bdmfmons_server_t *server;
    const char *address; /* client address */
    int sock;             /* transport socket */
    bdmf_task rx_thread;
    bdmf_session_handle session;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    bdmf_task conn_thread;
} bdmfmons_conn_t;

/* Server control bdmfock
 */
struct bdmfmons_server
{
    bdmfmons_server_t *next;
    bdmfmons_conn_t *conn_list;
    int sock;             /* listening socket */
    bdmfmons_parm_t parms;
    int id;
    int nconns;
    bdmf_fastlock lock;
    bdmf_task listen_thread;
};

/* socaddr variants */
typedef union
{
    struct sockaddr sa;
    struct sockaddr_un domain_sa;
    struct sockaddr_in tcp_sa;
} sockaddr_any;

static bdmfmons_server_t *bdmfmons_servers;
static int bdmfmons_server_id;

static bdmfmons_server_t *bdmfmons_id_to_server(int hs, bdmfmons_server_t **prev)
{
    bdmfmons_server_t *s=bdmfmons_servers;
    if (prev)
        *prev = NULL;
    while(s)
    {
        if (s->id == hs)
            break;
        if (prev)
            *prev = s;
        s = s->next;
    }
    return s;
}

/* Parse address helper */
static int bdmfmons_parse_address(const bdmfmons_parm_t *parms, int *protocol, sockaddr_any *sa, int *len)
{
    switch(parms->transport)
    {
    case BDMFMONS_TRANSPORT_DOMAIN_SOCKET:
    {
        *protocol = AF_UNIX;
        sa->domain_sa.sun_family = AF_UNIX;  /* local is declared before socket() ^ */
        strcpy(sa->domain_sa.sun_path, parms->address);
        *len = strlen(sa->domain_sa.sun_path) + sizeof(sa->domain_sa.sun_family);
        break;
    }
    case BDMFMONS_TRANSPORT_TCP_SOCKET:
    {
        *protocol = AF_INET;
        sa->tcp_sa.sin_family = AF_INET;
        sa->tcp_sa.sin_port = htons(atoi(parms->address));
        sa->tcp_sa.sin_addr.s_addr = INADDR_ANY;
        *len = sizeof(sa->tcp_sa);
        break;
    }
    default:
        return BDMF_ERR_PARM;
    }
    return 0;
}


/* disconnect client and clear resources */
static void bdmfmons_disconnect(bdmfmons_conn_t *conn)
{
    bdmfmons_server_t *s=conn->server;
    bdmfmons_conn_t *c=s->conn_list, *prev=NULL;

    bdmf_fastlock_lock(&s->lock);
    while(c && c!=conn)
    {
        prev = c;
        c = c->next;
    }
    BUG_ON(!c);
    if (prev)
        prev->next = c->next;
    else
        s->conn_list = c->next;
    --s->nconns;
    bdmf_fastlock_unlock(&s->lock);
    bdmfmon_session_close(c->session);
    close(c->sock);
    bdmf_task_destroy(c->rx_thread);
    bdmf_free(c);
}

/*
 * Session callbacks
 */

/** Session's output function.
 * returns the number of bytes written or <0 if error
 */
static int bdmfmons_cb_sess_write(bdmf_session_handle session, const char *buf, uint32_t size)
{
    bdmfmons_conn_t *c=bdmf_session_user_priv(session);
    int rc;

    rc = send(c->sock, buf, size, 0);
    /* disconnect if IO error */
    if (rc < size)
        bdmfmons_disconnect(c);
    else
        c->bytes_sent += rc;
    return rc;
}

#define CHAR_EOT 0x04

/** Session's input function.
 * returns the number of bytes read or <0 if error
 */
static char *bdmfmons_read_line(bdmfmons_conn_t *c, char *buf, uint32_t size)
{
    int i;
    int rc;
    int len=0;

    for(i=0; i<size-1; i++)
    {
        char ch;
        rc = recv(c->sock, &ch, 1, MSG_WAITALL);
        if (rc <= 0)
            break;
        if (ch == '\r')
            continue;
        if (ch == CHAR_EOT)
            break;
        buf[len++] = ch;
        if (ch == '\n')
            break;
    }
    c->bytes_received += i;
    buf[len] = 0;
    return (len ? buf : NULL);
}

/* Receive handler */
static int bdmfmons_rx_thread_handler(void *arg)
{
    char buf[512];
    bdmfmons_conn_t *c=arg;

    while(!bdmfmon_is_stopped(c->session) &&
          bdmfmons_read_line(c, buf, sizeof(buf)))
    {
        bdmfmon_parse(c->session, buf);
    }
    bdmfmons_disconnect(c);
    return 0;
}

/* New client connection indication */
static void bdmfmons_connect(bdmfmons_server_t *s, char *addr, int sock)
{
    bdmfmons_conn_t *c;
    bdmf_session_parm_t sess_parm;
    int rc;

    if (s->parms.max_clients && s->nconns >= s->parms.max_clients)
    {
        bdmf_print("bdmfmons: server %s: refused connection because max number has been reached\n", s->parms.address);
        close(sock);
        return;
    }

    c = bdmf_calloc(sizeof(*c) + strlen(addr) + 1);
    if (!c)
        goto cleanup;
    c->address = (char *)c + sizeof(*c);
    strcpy((char *)c->address, addr);
    c->server = s;
    c->sock = sock;

    /* create new management session */
    memset(&sess_parm, 0, sizeof(sess_parm));
    sess_parm.access_right = s->parms.access;
    sess_parm.write = bdmfmons_cb_sess_write;
    sess_parm.user_priv = c;
    rc = bdmfmon_session_open(&sess_parm, &c->session);
    if (rc)
        goto cleanup;

    /* wait for receive in a separate thread */
    rc = bdmf_task_create("bdmfmons_rx",
                    BDMFSYS_DEFAULT_TASK_PRIORITY,
                    BDMFSYS_DEFAULT_TASK_STACK,
                    bdmfmons_rx_thread_handler, c,
                    &c->rx_thread);
    if (rc)
        goto cleanup;

    bdmf_fastlock_lock(&s->lock);
    c->next = s->conn_list;
    s->conn_list = c;
    ++s->nconns;
    bdmf_fastlock_unlock(&s->lock);

    return;

cleanup:
    close(sock);
    if (c)
    {
        if (c->session)
            bdmfmon_session_close(c->session);
        bdmf_free(c);
    }
}

/* Receive handler */
static int bdmfmons_listen_thread_handler(void *arg)
{
    bdmfmons_server_t *s=arg;
    sockaddr_any addr;
    socklen_t len;
    int sock;

    while(1)
    {
        char caddr[64];
        len = sizeof(addr);
        sock = accept(s->sock, &addr.sa, &len);
        if (sock < 0)
        {
            perror("accept");
            break;
        }
        if (s->parms.transport==BDMFMONS_TRANSPORT_DOMAIN_SOCKET)
            strncpy(caddr, s->parms.address, sizeof(caddr)-1);
        else
        {
            snprintf(caddr, sizeof(caddr)-1, "%s:%d",
                inet_ntoa(addr.tcp_sa.sin_addr), ntohs(addr.tcp_sa.sin_port));
        }
        bdmfmons_connect(s, caddr, sock);
    }
    return 0;
}

/*
 * External API
 */

/** Create shell server.
 * Immediately after creation server is ready to accept client connections
 * \param[in]   parms   Server parameters
 * \param[out]  hs      Server handle
 * \return  0 - OK\n
 *         <0 - error code
 */
bdmf_error_t bdmfmons_server_create(const bdmfmons_parm_t *parms, int *hs)
{
    bdmfmons_server_t *s;
    int protocol;
    sockaddr_any sa;
    int len;
    int rc;

    if (!parms || !hs || !parms->address)
        return BDMF_ERR_PARM;

    /* parse address */
    if (bdmfmons_parse_address(parms, &protocol, &sa, &len))
        return BDMF_ERR_PARM;

    /* allocate server structure */
    s = bdmf_calloc(sizeof(bdmfmons_server_t)+strlen(parms->address)+1);
    if (!s)
        return BDMF_ERR_NOMEM;
    s->parms = *parms;
    s->parms.address = (char *)s + sizeof(*s);
    strcpy(s->parms.address, parms->address);
    s->id = ++bdmfmons_server_id;
    bdmf_fastlock_init(&s->lock);

    /* create socket and start listening */
    s->sock = socket(protocol, SOCK_STREAM, 0);
    if ((s->sock < 0) ||
        (bind(s->sock, &sa.sa, len) < 0) ||
        (listen(s->sock, 1) < 0))
    {
        perror("socket/bind/listen");
        close(s->sock);
        bdmf_free(s);
        return BDMF_ERR_PARM;
    }

    /* wait for connection(s) in a separate thread */
    rc = bdmf_task_create("bdmfmons_listen",
                    BDMFSYS_DEFAULT_TASK_PRIORITY,
                    BDMFSYS_DEFAULT_TASK_STACK,
                    bdmfmons_listen_thread_handler, s,
                    &s->listen_thread);
    if (rc)
    {
        close(s->sock);
        bdmf_free(s);
        return rc;
    }

    /* all good */
    s->next = bdmfmons_servers;
    bdmfmons_servers = s;
    *hs = s->id;

    return 0;
}

/** Destroy shell server.
 * All client connections if any are closed
 * \param[in]   hs      Server handle
 * \return  0 - OK\n
 *         <0 - error code
 */
bdmf_error_t bdmfmons_server_destroy(int hs)
{
    bdmfmons_server_t *prev;
    bdmfmons_server_t *s = bdmfmons_id_to_server(hs, &prev);
    bdmfmons_conn_t *c;
    if (!s)
        return BDMF_ERR_NOENT;

    bdmf_task_destroy(s->listen_thread);
    close(s->sock);

    /* disconnect all clients */
    while((c = s->conn_list))
        bdmfmons_disconnect(c);

    /* destroy server */
    bdmf_fastlock_lock(&s->lock);
    if (prev)
        prev->next = s->next;
    else
        bdmfmons_servers = s->next;
    bdmf_fastlock_unlock(&s->lock);

    bdmf_free(s);
    return 0;
}

/*
 * Shell command handlers
 */

static bdmfmon_enum_val_t transport_type_enum_tabdmfe[] = {
    { .name="domain_socket", .val=BDMFMONS_TRANSPORT_DOMAIN_SOCKET},
    { .name="tcp_socket", .val=BDMFMONS_TRANSPORT_TCP_SOCKET},
    BDMFMON_ENUM_LAST
};

static bdmfmon_enum_val_t access_type_enum_tabdmfe[] = {
    { .name="guest", .val=BDMF_ACCESS_GUEST},
    { .name="admin", .val=BDMF_ACCESS_ADMIN},
    { .name="debug", .val=BDMF_ACCESS_DEBUG},
    BDMFMON_ENUM_LAST
};

/* Create remote shell server
    BDMFMON_MAKE_PARM_ENUM("transport", "Transport type", transport_type_enum_tabdmfe, 0),
    BDMFMON_MAKE_PARM("address", "Bind address", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM_ENUM("access", "Access level", access_type_enum_tabdmfe, 0),
    BDMFMON_MAKE_PARM_DEFVAL("max_clients", "Max clients. 0=default", BDMFMON_PARM_NUMBER, 0, 0),
*/
static int bdmfmons_mon_create(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    bdmfmons_transport_type_t transport = (bdmfmons_transport_type_t)parm[0].value.number;
    char *address = (char *)parm[1].value.number;
    bdmf_access_right_t access = (bdmf_access_right_t)parm[2].value.number;
    int max_clients = (int)parm[3].value.number;
    bdmfmons_parm_t parms;
    int hs;
    int rc;

    memset(&parms, 0, sizeof(parms));
    parms.transport = transport;
    parms.access = access;
    parms.address = address;
    parms.max_clients = max_clients;
    rc = bdmfmons_server_create(&parms, &hs);
    if (rc)
        bdmf_session_print(session, "bdmfmons_server_create() failed with rc=%d - %s\n",
                        rc, bdmf_strerror(rc));
    else
        bdmf_session_print(session, "Remote shell server created. Server id %d\n", hs);
    return rc;
}

/* Destroy remote shell server
    BDMFMON_MAKE_PARM("server_id", "Server id", BDMFMON_PARM_NUMBER, 0),
*/
static int bdmfmons_mon_destroy(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    int hs = (int)parm[0].value.number;
    int rc;
    rc = bdmfmons_server_destroy(hs);
    bdmf_session_print(session, "Remote shell server %d destroyed. rc=%d - %s\n",
        hs, rc, bdmf_strerror(rc));
    return rc;
}

/* Show remote shell servers
*/
static int bdmfmons_mon_show(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    bdmfmons_server_t *s=bdmfmons_servers;
    bdmfmons_conn_t *c;
    while(s)
    {
        bdmf_session_print(session, "Remote server %d at %s\n", s->id, s->parms.address);
        c = s->conn_list;
        while(c)
        {
            bdmf_session_print(session, "\t - %s. bytes sent:%d received:%d\n",
                c->address, c->bytes_sent, c->bytes_received);
            c = c->next;
        }
        s = s->next;
    }
    return 0;
}

/* Create shell_server directory in root_dir
   Returns the "shell_server" directory handle
*/
bdmfmon_handle_t bdmfmons_server_mon_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t shell_dir;

    if ((shell_dir=bdmfmon_dir_find(NULL, "shell_server"))!=NULL)
        return NULL;

    shell_dir = bdmfmon_dir_add(root_dir, "shell_server",
                             "Remote Shell",
                             BDMF_ACCESS_GUEST, NULL);

    {
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM_ENUM("transport", "Transport type", transport_type_enum_tabdmfe, 0),
            BDMFMON_MAKE_PARM("address", "Bind address: domain_socket address or TCP port", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM_ENUM("access", "Access level", access_type_enum_tabdmfe, 0),
            BDMFMON_MAKE_PARM_DEFVAL("max_clients", "Max clients. 0=default", BDMFMON_PARM_NUMBER, 0, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(shell_dir, "create", bdmfmons_mon_create,
                      "Create remote shell server",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }

    {
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("server_id", "Server id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(shell_dir, "destroy", bdmfmons_mon_destroy,
                      "Destroy remote shell server",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }

    {
        bdmfmon_cmd_add(shell_dir, "show", bdmfmons_mon_show,
                      "Show remote shell servers",
                      BDMF_ACCESS_GUEST, NULL, NULL);
    }

    return shell_dir;
}

/* Destroy shell_server directory
*/
void bdmfmons_server_mon_destroy(void)
{
    bdmfmon_handle_t shell_dir;
    shell_dir=bdmfmon_dir_find(NULL, "shell_server");
    if (shell_dir)
        bdmfmon_token_destroy(shell_dir);
}

