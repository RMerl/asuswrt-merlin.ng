/* -*- mode: c; c-basic-offset: 2 -*- */

#include "chilli.h"
#include "chilli_module.h"

#define SOCK_PATH "/tmp/echo"

static int fd = 0;

static int acc(void *nullData, int sock) {
  int rlen;
  char req[512];

  if ((rlen = safe_read(fd, req, sizeof(req))) < 0) {
    log_err(errno, "acc()/read()");
    return -1;
  }

  log_dbg("Received echo %.*s", rlen, req);

  return CHILLI_MOD_OK;
}

static int module_initialize(char *conf, char isReload) {
  struct sockaddr_un local;
  
  log_dbg("%s('%s', %d)", __FUNCTION__, conf, (int) isReload);

  if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {

    log_err(errno, "could not allocate UNIX Socket!");

  } else {

    local.sun_family = AF_UNIX;

    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);

    if (bind(fd, (struct sockaddr *)&local, 
	     sizeof(struct sockaddr_un)) == -1) {
      log_err(errno, "could bind UNIX Socket!");
      close(fd);
      fd = 0;
    }
  }

  return CHILLI_MOD_OK;
}

static int module_net_select(select_ctx *sctx) {
  log_dbg("%s", __FUNCTION__);
  net_select_reg(sctx, fd, SELECT_READ, (select_callback) acc, 0, 0);
  return CHILLI_MOD_OK;
}

static int module_redir_login(struct redir_t *redir, 
			      struct redir_conn_t *conn,
			      struct redir_socket_t *sock) {
  log_dbg("%s", __FUNCTION__);
  return CHILLI_MOD_OK;
}

static int module_dhcp_connect(struct app_conn_t *appconn, 
			       struct dhcp_conn_t *dhcpconn) {
  log_dbg("%s", __FUNCTION__);
  return CHILLI_MOD_OK;
}

static int module_dhcp_disconnect(struct app_conn_t *appconn, 
				  struct dhcp_conn_t *dhcpconn) {
  log_dbg("%s", __FUNCTION__);
  return CHILLI_MOD_OK;
}

static int module_session_start(struct app_conn_t *appconn) {
  log_dbg("%s", __FUNCTION__);
  return CHILLI_MOD_OK;
}

static int module_session_update(struct app_conn_t *appconn) {
  log_dbg("%s", __FUNCTION__);
  return CHILLI_MOD_OK;
}

static int module_session_stop(struct app_conn_t *appconn) {
  log_dbg("%s", __FUNCTION__);
  return CHILLI_MOD_OK;
}

static int module_dns_handler (struct app_conn_t *appconn, 
			       struct dhcp_conn_t *dhcpconn,
			       uint8_t *pack, size_t *plen, int isReq) {
  log_dbg("%s", __FUNCTION__);
  return CHILLI_DNS_OK;
}

static int module_destroy(char isReload) {

  log_dbg("%s(%d)", __FUNCTION__, (int) isReload);

  close(fd);
  return CHILLI_MOD_OK;
}

struct chilli_module sample_module = {
  CHILLI_MODULE_INIT,
  module_initialize, 
  module_net_select,
  module_redir_login,
  module_dhcp_connect,
  module_dhcp_disconnect,
  module_session_start,
  module_session_update,
  module_session_stop,
  0,
  module_dns_handler,
  0,
  0,
  module_destroy,
};

