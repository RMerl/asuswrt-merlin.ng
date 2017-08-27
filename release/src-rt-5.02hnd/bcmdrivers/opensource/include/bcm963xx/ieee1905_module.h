/*
<:copyright-BRCM:2002:GPL/GPL:standard

   Copyright (c) 2002 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef _IEEE1905_MOD_H_
#define _IEEE1905_MOD_H_

#define I5_UDP_SERVER_PORT  11905

typedef enum t_i5_udp_cmd_name {
  I5_CMD_UNSPEC,
   
  /* unicast messages */
  I5_UDP_CMD_CLIENT_REGISTER,
  I5_UDP_CMD_PUSH_BUTTON_REGISTER,
  I5_UDP_CMD_SES_BUTTON_TRIGGER,
  
  /* multicast messages */
  I5_UDP_CMD_PUSH_BUTTON_NOTIFY,
  
  __I5_CMD_MAX,
} t_I5_UDP_CMD_NAME;

typedef struct t_i5_udp_msg
{
  t_I5_UDP_CMD_NAME  cmd;
  int                len;
} t_I5_UDP_MSG;

typedef struct t_i5_udp_push_button_register_msg
{
  t_I5_UDP_MSG udpMsg;
  unsigned int reg;
} t_i5_UDP_PUSH_BUTTON_REGISTER_MSG;

typedef struct t_i5_udp_push_button_handle_msg
{
  t_I5_UDP_MSG udpMsg;
  int forced;
} t_i5_UDP_PUSH_BUTTON_HANDLE_MSG;

#endif /* _IEEE1905_MOD_H_ */
