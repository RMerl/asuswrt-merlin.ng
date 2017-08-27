/*
* <:copyright-BRCM:2013:GPL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef BL_GPON_PWM_GPL_H_INCLUDED
#define BL_GPON_PWM_GPL_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

/* Definitions */

typedef uint8_t CS_BL_PWM_CTRL ;

/* Disable */
#define CS_BL_PWM_CTRL_DISABLE ( ( CS_BL_PWM_CTRL ) 0 )
/* Enable */
#define CS_BL_PWM_CTRL_ENABLE  ( ( CS_BL_PWM_CTRL ) 1 )

typedef enum
{
	CE_BL_PWM_PARAM_IHOLD = 1,
	CE_BL_PWM_PARAM_IAWARE,
	CE_BL_PWM_PARAM_ISLEEP,
	CE_BL_PWM_PARAM_ITRANSINIT,
	CE_BL_PWM_PARAM_ITXINIT
}
BL_PWM_PARAM_DTE ;

typedef enum
{
	CE_BL_PWM_INFO_MSG_SA_ON = 1,
	CE_BL_PWM_INFO_MSG_SA_OFF,

	CE_BL_PWM_INFO_MSG_ACTIVE,
	CE_BL_PWM_INFO_MSG_INACTIVE,

	CE_BL_PWM_INFO_MSG_MODE_AWAKE,
	CE_BL_PWM_INFO_MSG_MODE_DOZE,
	CE_BL_PWM_INFO_MSG_MODE_SLEEP
}
BL_PWM_INFO_MSGS_DTE ;


typedef int ( * CB_BL_TRANS_HNDL ) ( void ) ;

typedef struct _bl_pwm_cbs_dte
{
	/* Commands */

	CB_BL_TRANS_HNDL pwm_cb_disable_tx		;
	CB_BL_TRANS_HNDL pwm_cb_disable_rx_tx	;

	CB_BL_TRANS_HNDL pwm_cb_enable_tx		;
	CB_BL_TRANS_HNDL pwm_cb_enable_rx_tx	;


	/* Information */
	void ( *pwm_cb_update_info ) ( BL_PWM_INFO_MSGS_DTE ) ;
}
BL_PWM_CBS_DTE ;


typedef enum
{
	CE_BL_PWM_MODE_AWAKE = 0,
	CE_BL_PWM_MODE_DOZE,
	CE_BL_PWM_MODE_SLEEP
}
BL_PWM_MODE_DTE ;

/* Errors */
typedef enum
{
	CS_BL_PWM_ERR_OK,
	CS_BL_PWM_ERR_SERVICE_DISABLED,
	CS_BL_PWM_ERR_CFG_INVALID_STATE,
	CS_BL_PWM_ERR_PARAM_NOT_SET,
	CS_BL_PWM_ERR_PARAM_INVALID_VAL,
	CS_BL_PWM_ERR_CB_INVALID_VAL,
	CS_BL_PWM_ERR_INVALID_IN,
	CS_BL_PWM_ERR_PLOAM_TX,
	CS_BL_PWM_ERR_LINK_CTRL,
	CS_BL_PWM_ERR_PON_STACK,
	CS_BL_PWM_ERR_SHELL,
	CS_BL_PWM_ERR_INTERNAL
}
BL_PWM_ERR_DTS ;

typedef struct _bl_pwm_params_dts
{
	/* Ihold */
	uint16_t		ihold			;
	bool	ihold_is_set	;

	/* Iaware */
	uint16_t		iaware			;
	bool	iaware_is_set	;

	/* Isleep */
	uint16_t		isleep			;
	bool	isleep_is_set	;

	/* Itransinit */
	uint16_t		itransinit			;
	bool	itransinit_is_set	;

	/* Itxinit */
	uint16_t		itxinit			;
	bool	itxinit_is_set	;
}
BL_PWM_PARAMS_DTS ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   api_pwm_param_set                                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BL Lilac CH - Sets Power Management parameter                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets Power Management parameter                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_val - Parameter value                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_PWM_ERR_DTS - Return code                                             */
/*                                                                            */
/******************************************************************************/
BL_PWM_ERR_DTS api_pwm_param_set ( BL_PWM_PARAM_DTE	xi_param,
								   uint16_t		xi_val ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   api_pwm_param_get                                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BL Lilac CH - Gets Power Management parameter                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets Power Management parameter                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xo_val - Parameter value                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_PWM_ERR_DTS - Return code                                             */
/*                                                                            */
/******************************************************************************/
BL_PWM_ERR_DTS api_pwm_param_get ( BL_PWM_PARAM_DTE		xi_param,
								   uint16_t* const	xo_val ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   api_pwm_reg_cbs                                                          */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BL Lilac CH - Registers Host Application's callback functions            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function registers Host Application's callback functions            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_cbs - Callback functions                                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_PWM_ERR_DTS - Return code                                             */
/*                                                                            */
/******************************************************************************/
BL_PWM_ERR_DTS api_pwm_reg_cbs ( const BL_PWM_CBS_DTE* const xi_cbs ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   api_pwm_ctrl                                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BL Lilac CH - Control (enable/disable) Power Management                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function controls (enables/disables) Power Management               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ctrl_flg - Control flag (enable/disable)                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_PWM_ERR_DTS - Return code                                             */
/*                                                                            */
/******************************************************************************/
BL_PWM_ERR_DTS api_pwm_ctrl ( CS_BL_PWM_CTRL xi_ctrl_flg ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   api_pwm_switch_mode                                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BL Lilac CH - Initiates Power Management mode                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function initiates Power Management mode                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   None                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   BL_PWM_ERR_DTS - Return code                                             */
/*                                                                            */
/******************************************************************************/
BL_PWM_ERR_DTS api_pwm_switch_mode ( BL_PWM_MODE_DTE xi_mode ) ;

#ifdef __cplusplus
}
#endif

#endif /* BL_GPON_PWM_GPL_H_INCLUDED */

