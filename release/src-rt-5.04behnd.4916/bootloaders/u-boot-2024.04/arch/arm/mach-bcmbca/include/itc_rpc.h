/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef _ITC_RPC_H_
#define _ITC_RPC_H_

#include <stdbool.h>
#include "itc_msg_defs.h"
#include "itc_channel_defs.h"

static inline void rpc_msg_init_header(rpc_msg *msg,
				       int service,
				       int function,
				       int version)
{
	msg->header = RPC_MSG_MAKE_HEADER(version, 0, 0, service, function, 0);
}

static inline void rpc_msg_init(rpc_msg *msg,
				int service,
				int function,
				int version,
				uint32_t data0,
				uint32_t data1,
				uint32_t data2)
{
	msg->header = RPC_MSG_MAKE_HEADER(version, 0, 0, service, function, 0);
	msg->data[0] = data0;
	msg->data[1] = data1;
	msg->data[2] = data2;
}

/*
 * functions used by the service handler to handle incoming messages/requests
 * the rpc_msg * memory is owned and maintained by ItcRpc
 */

typedef int (*rpc_rx_handler)(enum rpc_tunnel_idx tunnel, rpc_msg *msg);

typedef struct {
	rpc_rx_handler func;
	int version;
} rpc_function;

/*
 * service function table that is an
 * array of function pointers that correspond to the function indexes
 * you have defined somewhere in your code.
 */

int rpc_register_functions(int service,
			   rpc_function *func_table,
			   int func_cnt);

/*
 * register a single function
 * must be done after you have rpc_register_functions()
 * this is useful when you have a function in a module
 * that will be loaded later than the rpc_register_functions()
 */
int rpc_register_function(int service,
			  int function,
			  rpc_function *func);

/*
 * unregister a previously registered service function table
 */
int rpc_unregister_functions(int service);

/*
 * unregister a previously registered service function
 */
int rpc_unregister_function(int service, int function);

/*
 * Reply to message.
 * Used by request handler.
 * If you are replying to a message from your service callback function
 * you MUST use this because this api will set the reply
 * bit so that the other side can do the right thing.
 */
int rpc_send_reply(enum rpc_tunnel_idx tunnel, rpc_msg *msg);

/*
 * functions used to send messages
 */

/* wait for response
 * rpc_send_request cannot be used in one of your callback functions
 * on the service side. There you can only use rpc_send_reply, or
 * rpc_send_message.
 * Ideally your callback funtion would only send rpc_send_reply
 * or nothing at all.
 */
int rpc_send_request_timeout(enum rpc_tunnel_idx tunnel, rpc_msg *msg, int sec);

/*
 * do not wait for response
 * will return error if the tunnel has not completed init
 * do not use this to reply to a message, use rpc_send_reply
 */
int rpc_send_message(enum rpc_tunnel_idx tunnel, rpc_msg *msg);

/* poll for pending messages */
void msg_poll(enum rpc_tunnel_idx idx);

/* debug */
void rpc_dump_msg(rpc_msg *msg);

/* init everything */
int rpc_init(void);
int rpc_tunnel_init(enum rpc_tunnel_idx idx, bool handshake);
int rpc_tunnel_get_name(enum rpc_tunnel_idx idx, char **name);
void rpc_exit(void);

#endif
