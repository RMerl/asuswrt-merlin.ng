#ifndef _NATAPI_H
#define _NATAPI_H
#include <dlfcn.h>
#include <natnl_lib.h>

#define MAX_CALLS   8

int lib_load(void** handle, const char* lib_path);
int lib_unload	(void* handle);
int lib_get_func(void* handle, const char* func_name, void** func_sym);


// function pointer
typedef char* (*NAT_VERSION)	(void);
#ifdef TNL_CALLBACK_ENABLE
typedef int (*NAT_INIT3)	(struct natnl_config *cfg, struct natnl_callback *natnl_cb, void *app_data);
#else
typedef int (*NAT_INIT)		(struct natnl_config *cfg);
#endif
typedef int (*NAT_DEINIT)	(void);
typedef int (*NAT_MAKECALL)	(char *device_id, int tnl_port_cnt,
								natnl_tnl_port tnl_ports[],
							    char *user_id, int timeout_sec);
typedef int (*NAT_HANG_UP)	(int call_id);

typedef int (*NAT_POOL_DUMP)(int detail);

typedef int (*NAT_DETECT) (char *stun_srv);

typedef int (*NAT_READ_IM_MSG) (char **msg_buf);

typedef int (*NAT_WRITE_IM_RESP) (char *resp_msg);

typedef int (*NAT_REG_DEVICE) (void);

typedef int (*NAT_UNREG_DEVICE) (void);

typedef int (*NAT_MAKECALL3)	(char *device_id, int tnl_port_cnt,
								natnl_tnl_port tnl_ports[],
							    char *user_id, int timeout_sec, 
                                int use_sctp, int inst_id,
                                char *caller_device_pwd, struct natnl_tnl_info *tnl_info);

typedef int (*NAT_READ_TNL_INFO)	(int call_id, struct natnl_tnl_info *tnl_info);

int init_natnl_api(NAT_INIT3* nat_init3, 
    NAT_DEINIT* nat_deinit, 
    NAT_MAKECALL* nat_makecall, 
    NAT_HANG_UP* nat_hangup, 
    NAT_POOL_DUMP* nat_dump , 
    NAT_DETECT* nat_detect, 
    NAT_VERSION* nat_version, 
    NAT_READ_IM_MSG* nat_read_im_msg, 
    NAT_WRITE_IM_RESP* nat_write_im_resp, 
    NAT_REG_DEVICE* nat_reg_device, 
    NAT_UNREG_DEVICE* nat_unreg_device, 
    NAT_MAKECALL3* nat_makecall3,
    NAT_READ_TNL_INFO *nat_read_ntl_info);
#endif
