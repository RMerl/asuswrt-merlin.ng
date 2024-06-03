#ifndef __OTP_MAP_CMN_H
#define __OTP_MAP_CMN_H

#include "otp_map.h"
#include "otp_hw_cmn.h"

/* Generic OTP MAP Interface */
typedef struct _otp_map_cmn otp_map_cmn_t;

typedef enum _otp_map_cmn_err_ {
   OTP_MAP_CMN_OK = 0,
   OTP_MAP_CMN_ERR_FAIL = -1,
   OTP_MAP_CMN_ERR_UNSP = -2,
   OTP_MAP_CMN_ERR_INVAL= -3,
} otp_map_cmn_err_t;


struct _otp_map_cmn {
	otp_hw_cmn_t dev;
	otp_hw_cmn_init_t dev_init;
	/* 
 	* fuse/write feature.   Feed  with data aray of u32 and size 
 	* */
	otp_map_cmn_err_t (*write)(otp_map_cmn_t*, u32, const u32*, u32);
	/* 
 	* reads  feature content. Return data pointer and its size 
 	* */
	otp_map_cmn_err_t (*read)(otp_map_cmn_t*, u32, u32**, u32*);
	/* control various functions such as:
	*	-lock
	*	-status
 	*   based on otp_hw_cmd_ctl_cmd_t content 
 	* */
	otp_map_cmn_err_t (*ctl)(otp_map_cmn_t*, otp_hw_cmn_ctl_cmd_t*, u32*);
};
/* An initializer for otp_map_cmnt_t obj. 
 * Accept external driver object with an initializer function  */
otp_map_cmn_err_t otp_map_cmn_init(otp_map_cmn_t* map,
			otp_hw_cmn_init_t hw_init,  
			otp_hw_cmn_t* ext_drv);

otp_hw_cmn_err_t otp_hw_cmn_init(otp_hw_cmn_t* dev);
otp_hw_cmn_err_t sotp_hw_cmn_init(otp_hw_cmn_t* dev);

#endif
