/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <net/sock.h>
#include <linux/udp.h>
#include <linux/net.h>
#include <ieee1905_module.h>
#include <board.h>
#include <pushbutton.h>

struct i5_work_t {
  struct work_struct  i5_work;
  union {
    struct sock       *sk;
    unsigned int       msgType;
  };
};

typedef struct {
  struct workqueue_struct  *i5_work_queue;
  struct socket            *i5_udp_socket;
  struct i5_work_t          i5_udp_queue_data;
  struct i5_work_t          i5_push_button_notify_data;

  unsigned short i5_listener_port;
}ieee1905_priv_data;

static ieee1905_priv_data i5_module_priv;

static void _i5ModulePushButtonNotify( unsigned long timeInMs, void* param )
{
  i5_module_priv.i5_push_button_notify_data.msgType = I5_UDP_CMD_PUSH_BUTTON_NOTIFY;
  queue_work(i5_module_priv.i5_work_queue, &i5_module_priv.i5_push_button_notify_data.i5_work);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
static void _i5ModuleDataReady(struct sock *sk) {
  i5_module_priv.i5_udp_queue_data.sk = sk;
  queue_work(i5_module_priv.i5_work_queue, &i5_module_priv.i5_udp_queue_data.i5_work);
}
#else
static void _i5ModuleDataReady(struct sock *sk, int bytes) {
  i5_module_priv.i5_udp_queue_data.sk = sk;
  queue_work(i5_module_priv.i5_work_queue, &i5_module_priv.i5_udp_queue_data.i5_work);
}
#endif

static void _i5ModuleNotificationWorkHandler(struct work_struct *work)
{
  struct i5_work_t *i5_wq_data = container_of(work, struct i5_work_t, i5_work);
  t_I5_UDP_MSG            udpMsg;
  struct msghdr           msg;
  struct iovec            iov;
  struct sockaddr_in      to;
  int                     sendLen;

  udpMsg.cmd = i5_wq_data->msgType;
  udpMsg.len = 0;

  if ( i5_module_priv.i5_listener_port ) {
    memset(&to, 0, sizeof(to));
    to.sin_family      = AF_INET;
    to.sin_addr.s_addr = htonl(0x7f000001);  
    to.sin_port        = htons(i5_module_priv.i5_listener_port);
 
    memset(&msg, 0, sizeof(msg));
    msg.msg_name       = &to;
    msg.msg_namelen    = sizeof(to);
    msg.msg_control    = NULL;
    msg.msg_controllen = 0;
    iov.iov_base       = &udpMsg;
    iov.iov_len        = sizeof(t_I5_UDP_MSG);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, iov.iov_len);
    sendLen = sock_sendmsg(i5_module_priv.i5_udp_socket, &msg);
#else
    msg.msg_iov        = &iov;
    msg.msg_iovlen     = 1;
    sendLen = sock_sendmsg(i5_module_priv.i5_udp_socket, &msg, sizeof(t_I5_UDP_MSG));
#endif
    if ( sendLen != sizeof(t_I5_UDP_MSG) ) {        
      printk("%s: error sending notification\n", __FUNCTION__);
    }
  }
}

static void _i5ModuleProcessMessage(struct work_struct *work)
{
  struct i5_work_t   *i5_wq_data = container_of(work, struct i5_work_t, i5_work);
  struct sk_buff           *skb = NULL;
  struct udphdr            *pUdpHdr;
  t_I5_UDP_MSG             *pUdpMsg;

  while ( 1 )
  {
    skb = skb_dequeue(&i5_wq_data->sk->sk_receive_queue);
    if ( NULL == skb ) {
      break;
    }

    pUdpMsg = (t_I5_UDP_MSG *)(skb->data + sizeof(struct udphdr));
    switch ( pUdpMsg->cmd ) {
      case I5_UDP_CMD_CLIENT_REGISTER:
        pUdpHdr = (struct udphdr *)skb->data;
        i5_module_priv.i5_listener_port = ntohs(pUdpHdr->source);
        break;

      case I5_UDP_CMD_PUSH_BUTTON_REGISTER:
        {
          t_i5_UDP_PUSH_BUTTON_REGISTER_MSG *pMsg;

          if ( pUdpMsg->len != (sizeof(t_i5_UDP_PUSH_BUTTON_REGISTER_MSG) - sizeof(t_I5_UDP_MSG))) {
            break;
          }
          pMsg = (t_i5_UDP_PUSH_BUTTON_REGISTER_MSG *)pUdpMsg;
          if ( 0 == pMsg->reg ) {
            kerSysDeregisterPlcUkeCallback(_i5ModulePushButtonNotify);
          }
          else {
            kerSysRegisterPlcUkeCallback(_i5ModulePushButtonNotify,NULL);
          }
          break;
        }

#if defined(WIRELESS)
      case I5_UDP_CMD_SES_BUTTON_TRIGGER:
        {
          t_i5_UDP_PUSH_BUTTON_HANDLE_MSG *pMsg;
          if ( pUdpMsg->len != (sizeof(t_i5_UDP_PUSH_BUTTON_HANDLE_MSG) - sizeof(t_I5_UDP_MSG))) {
            break;
          }
          pMsg = (t_i5_UDP_PUSH_BUTTON_HANDLE_MSG *)pUdpMsg;
          kerSysSesEventTrigger(pMsg->forced);
          break;
        }
#endif
      default:
         printk("%s: unknown message (%d)\n", __FUNCTION__, pUdpMsg->cmd);
         break;
    }

    kfree_skb(skb);
  }
}

int __init i5ModuleInitialize(void)
{
  struct sockaddr_in server;
  int err;

  i5_module_priv.i5_listener_port = 0;
  
  if (sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP, &i5_module_priv.i5_udp_socket) < 0) {
    printk( KERN_ERR "i5ModuleInitialize: Error creating udp socket\n" );
    return -EIO;
  }

  server.sin_family      = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port        = htons( (unsigned short)I5_UDP_SERVER_PORT);
  err = i5_module_priv.i5_udp_socket->ops->bind(i5_module_priv.i5_udp_socket, (struct sockaddr *) &server, sizeof(server ));
  if (err) {
    printk( KERN_ERR "i5ModuleInitialize: Error binding udp socket\n");
    sock_release(i5_module_priv.i5_udp_socket);
    return -EIO;
  }
  i5_module_priv.i5_udp_socket->sk->sk_data_ready = _i5ModuleDataReady;
 
  INIT_WORK(&i5_module_priv.i5_udp_queue_data.i5_work, _i5ModuleProcessMessage);
  INIT_WORK(&i5_module_priv.i5_push_button_notify_data.i5_work, _i5ModuleNotificationWorkHandler);
  
  i5_module_priv.i5_work_queue = create_singlethread_workqueue("i5_work_queue"); 
  if (!i5_module_priv.i5_work_queue){
    return -ENOMEM;
  }

  return 0;
}

void __exit i5ModuleTerminate(void)
{
  if (i5_module_priv.i5_udp_socket) {
    sock_release(i5_module_priv.i5_udp_socket);
  }

  if (i5_module_priv.i5_work_queue) {
    flush_workqueue(i5_module_priv.i5_work_queue);
    destroy_workqueue(i5_module_priv.i5_work_queue);
  }
}

module_init( i5ModuleInitialize );
module_exit( i5ModuleTerminate );

