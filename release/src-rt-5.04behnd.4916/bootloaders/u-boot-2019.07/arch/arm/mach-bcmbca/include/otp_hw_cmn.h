/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */
#ifndef _OTP_HW_CMN_H_
#define _OTP_HW_CMN_H_


/* Generic OTP controller Interface */

typedef struct otp_hw_cmn_s otp_hw_cmn_t;

typedef enum otp_hw_cmn_err_e{
   OTP_HW_CMN_OK = 0,
   OTP_HW_CMN_ERR_FAIL = -1,
   OTP_HW_CMN_ERR_UNSP = -2,
   OTP_HW_CMN_ERR_KEY_EMPTY = -3,
   OTP_HW_CMN_ERR_DESC_ECC = -4,
   OTP_HW_CMN_ERR_DATA_ECC = -5, 
   OTP_HW_CMN_ERR_TMO = -6,
   OTP_HW_CMN_ERR_INVAL = -7,
   OTP_HW_CMN_ERR_BAD_PARAM = -8,
   OTP_HW_CMN_ERR_WRITE_FAIL= -9,
} otp_hw_cmn_err_t;




/*status command to use in control function*/
/*  
 * (N)SRD - (Non)Secure Master Read 
 * (N)SW - (Non)Secure Master Write 
 * S - Secure  
 * PAC - peripheral access control either block or registers  
 *  - Secure Master Write 
 * */
typedef enum otp_hw_cmn_status_e{
   OTP_HW_CMN_STATUS_UNLOCKED		= 0x0,
   OTP_HW_CMN_STATUS_SRD_LOCKED		= 0x1,
   OTP_HW_CMN_STATUS_SW_LOCKED		= 0x2,
   OTP_HW_CMN_STATUS_NSRD_LOCKED	= 0x4,
   OTP_HW_CMN_STATUS_NSW_LOCKED		= 0x8,
   OTP_HW_CMN_STATUS_SW_PAC_LOCKED	= 0x10,
   OTP_HW_CMN_STATUS_NSW_PAC_LOCKED	= 0x20,
   OTP_HW_CMN_STATUS_SRD_PAC_LOCKED	= 0x40,
   OTP_HW_CMN_STATUS_NSRD_PAC_LOCKED	= 0x80,
   OTP_HW_CMN_STATUS_NS_LOCKED 		= 0x100,
   OTP_HW_CMN_STATUS_S_LOCKED 		= 0x200,
   OTP_HW_CMN_STATUS_ROW_W_LOCKED 	= 0x400,
   OTP_HW_CMN_STATUS_ROW_RD_LOCKED 	= 0x800,
   OTP_HW_CMN_STATUS_ROW_DATA_VALID	= 0x1000
} otp_hw_cmn_status_t;

/*generic lock command to use in control function*/
/*  
 * (N)SRD - (Non)Secure Master Read 
 * (N)SW - (Non)Secure Master Write 
 * S - Secure  
 * PAC - peripheral access control either block or registers  
 *  - Secure Master Write 
 * */
typedef enum otp_hw_cmn_perm_e{
   OTP_HW_CMN_CTL_LOCK_NONE 		= 0x0,
   OTP_HW_CMN_CTL_LOCK_PAC_SW 		= 0x1,
   OTP_HW_CMN_CTL_LOCK_PAC_NSW 		= 0x2,
   OTP_HW_CMN_CTL_LOCK_PAC_NSRD 	= 0x4,
   OTP_HW_CMN_CTL_LOCK_PAC_SRD 		= 0x8,
   OTP_HW_CMN_CTL_LOCK_SRD 		= 0x10,
   OTP_HW_CMN_CTL_LOCK_NSRD 		= 0x20,
   OTP_HW_CMN_CTL_LOCK_SW 		= 0x40,
   OTP_HW_CMN_CTL_LOCK_NSW 		= 0x80,
   OTP_HW_CMN_CTL_LOCK_ALL 		= 0x100,
   OTP_HW_CMN_CTL_LOCK_NS 		= 0x200,
   OTP_HW_CMN_CTL_LOCK_NS_PROV 		= 0x400,
   OTP_HW_CMN_CTL_LOCK_S 		= 0x800,
   OTP_HW_CMN_CTL_LOCK_ROW_RD 		= 0x1000,
   OTP_HW_CMN_CTL_LOCK_ROW_W 		= 0x2000,
} otp_hw_cmn_perm_t;
/* container for the control call*/
typedef struct _otp_hw_ctl_data_s {
	u32 addr;
	union {
		otp_hw_cmn_perm_t perm;
		otp_hw_cmn_status_t status;
	};
} otp_hw_ctl_data_t;

/* generic OTP command options*/
typedef enum otp_hw_cmn_ctl_e {
   OTP_HW_CMN_CTL_NONE			= 0x0,
   OTP_HW_CMN_CTL_STATUS 		= 0x1,
   OTP_HW_CMN_CTL_LOCK 			= 0x2,
   OTP_HW_CMN_CTL_UNLOCK 		= 0x4,
   OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG	= 0x8,
   OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG_DONE	= 0x10,
   OTP_HW_CMN_CTL_OTPCMD_ECC 		= 0x20,
   OTP_HW_CMN_CTL_OTPCMD_ECC_WREAD 	= 0x40,
   OTP_HW_CMN_CTL_OTPCMD_PROG_LOCK 	= 0x80,
   OTP_HW_CMN_CTL_CONF			= 0x100
} otp_hw_cmn_ctl_t;

/* generic row id*/
typedef enum otp_hw_cmn_row_addr_type_e{
   OTP_HW_CMN_ROW_ADDR_NONE = 0x0,
   OTP_HW_CMN_ROW_ADDR_ROW,
} otp_hw_cmn_row_addr_type_t;


/* a control functions argument */
typedef struct _otp_hw_ctl_data_ {
	otp_hw_cmn_ctl_t ctl;
	uintptr_t data;
	u32 size;
} otp_hw_cmn_ctl_cmd_t;

/* a row configuration container */
typedef struct otp_hw_row_conf_s {
	otp_hw_cmn_ctl_t	op_type;
	otp_hw_cmn_row_addr_type_t addr_type;
	ulong perm;
	uintptr_t arg;
} otp_hw_cmn_row_conf_t;

/* a row data container with map to feature name*/
typedef struct otp_hw_cmn_row_s {
	otp_map_feat_t feat;
	u32 addr;
	u32 mask;
	u32 shift;
	u32 range;
	otp_hw_cmn_row_conf_t conf;
	union {
		u8* pdata;
		u32 data;
	};
	u32 valid;	
} otp_hw_cmn_row_t;

/* otp hw object */
struct otp_hw_cmn_s {
	//Driver which is implemented elsewhere
	otp_hw_cmn_t *drv_ext;	
	uintptr_t mm;
	otp_hw_cmn_row_t *rows;
	u32 row_max;
	otp_hw_cmn_row_conf_t row_conf;
	otp_hw_cmn_ctl_cmd_t ctl_cmd;
	/* reads one or more rows from the device*/
	otp_hw_cmn_err_t (*read)(otp_hw_cmn_t *dev,
			u32 addr,
			u32 *,
			u32);

	/* writes one or more rows to the device*/
	otp_hw_cmn_err_t (*write)(otp_hw_cmn_t *,
			u32 addr,
			const u32 *,
			u32 );
	/* reads  one or more rows from the device based on otp_hw_cmn_row_conf_t - does not require
 	*  feature map
 	* */
	otp_hw_cmn_err_t (*read_ex)(otp_hw_cmn_t *dev,
			u32 addr,
			otp_hw_cmn_row_conf_t *,
			u32 *,
			u32);
	/* writes  one or more rows to the device based on otp_hw_cmn_row_conf_t 
 	* 	- does not require
 	*  feature map
 	* */
	otp_hw_cmn_err_t (*write_ex)(otp_hw_cmn_t *,
			u32 addr,
			otp_hw_cmn_row_conf_t *,
			const u32 *,
			u32 );
 
	/* control various device functions such as:
	*	-lock
	*	-status
 	*   based on otp_hw_cmdn_ctl_cmd_t content 
 	* */
	otp_hw_cmn_err_t (*ctl)(otp_hw_cmn_t *dev, 
			const otp_hw_cmn_ctl_cmd_t *cmd,
			u32* res);
};
/* on init pointer */
typedef otp_hw_cmn_err_t  (*otp_hw_cmn_init_t)(otp_hw_cmn_t* );

#endif


