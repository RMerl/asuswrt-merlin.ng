/*
***************************************************************************
*  Mediatek Inc.
* 4F, No. 2 Technology 5th Rd.
* Science-based Industrial Park
* Hsin-chu, Taiwan, R.O.C.
*
* (c) Copyright 2002-2011, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Ralink Tech. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
***************************************************************************

                Module Name:
                steer_fsm.h

                Abstract:
                Header file containing the various parameter, enum and structure 
                definitions along with the function declarations

                Revision History:
                Who         When          What
                --------    ----------    -----------------------------------------
                Anirudh.S   2018/04/19	  Header file containing required info
*/

#ifndef STEER_FSM_H
#define STEER_FSM_H

/* PARAMETER DEFINITIONS */
/* ***************************************************************************** */
#define CHAN_MAX 60
/* ***************************************************************************** */


/* Enumerations */
/* ***************************************************************************** */
typedef enum client_steering_state 
{
	CLI_STATE_INVALID,
	CLI_STATE_IDLE,
	CLI_STATE_STEER_DECISION,
	CLI_STATE_STEER_PREPARATION,
	CLI_STATE_STEER_EXEC_MONITOR,
	CLI_STATE_STEER_COMPLETE
} STEERING_STATE;

typedef enum client_steer_method 
{
	MANDATE,
	ACTIVE_STANDALONE_DG,
	IDLE_STANDALONE_DG,
#ifdef SUPPORT_MULTI_AP
	NOL_MULTIAP,
#endif
	OFFLOADING,
	ACTIVE_STANDALONE_UG,
	IDLE_STANDALONE_UG,
	ACTIVE_5GL_TO_5GH,
	ACTIVE_5GH_TO_5GL,
	IDLE_5GL_TO_5GH,
	IDLE_5GH_TO_5GL,
	NONE,
	MAX_NUM_STR_METHODS,
} STEERING_METHOD_TYPE;


typedef enum client_measurement_state 
{
	MEAS_STATE_IDLE,
#ifdef SUPPORT_MULTI_AP
	MEAS_STATE_AIR_MON_TRIGGERED,
#endif
	MEAS_STATE_11K_TRIGGERED
} MEASUREMENT_STATE;

typedef enum client_execution_state
{
	EXEC_IDLE,
	EXEC_WAITING_BTM_RSP,
	EXEC_WAITING_ASSOC
} EXEC_MONITOR_STATE;
#ifdef SUPPORT_MULTI_AP
typedef enum client_cooordination_state 
{
	COORD_STATE_IDLE,
	COORD_STATE_RFS_TRIGGERED,
	COORD_STATE_RFS_COMPLETED,
	COORD_STATE_TSQ_TRIGGERED,
	COORD_STATE_RFS_RECEIVED,
	COORD_STATE_ASSOC_CONTROL_RECEIVED
} COORDINATION_STATE;
#endif

typedef enum client_state_machine_trigger 
{
	MANDATE_TRIGGER,
	OFFLOADING_TRIGGER,
	ACTIVE_STANDALONE_DG_TRIGGER,
	IDLE_STANDALONE_UG_TRIGGER,
	IDLE_STANDALONE_DG_TRIGGER,
	ACTIVE_STANDALONE_UG_TRIGGER,
#ifdef SUPPORT_MULTI_AP
	NOL_MULTIAP_TRIGGER,
#endif
	IDLE_STANDALONE_5GL_TO_5GH_TRIGGER,
	IDLE_STANDALONE_5GH_TO_5GL_TRIGGER,
	ACTIVE_STANDALONE_5GL_TO_5GH_TRIGGER,
	ACTIVE_STANDALONE_5GH_TO_5GL_TRIGGER,
	CHAN_MEASUREMENT_FAIL,
	CHAN_MEASUREMENT_COMPLETE,
	CHAN_MEASUREMENT_DISALLOWED,
#ifdef SUPPORT_MULTI_AP
	RFS_SUCCESS,
	RFS_FAILURE,
	RFS_TIMEOUT,
#endif
	BTM_SUCCESS,
	BTM_FAILURE,
	BTM_TIMEOUT,
	REMOTE_TOPOLOGY_NOTIFICATION,
	CLIENT_ASSOCIATED,
	CLIENT_DISSOCIATED,
	CLIENT_ASSOCIATION_TIMEOUT,
#ifdef SUPPORT_MULTI_AP
	AUTH_DENY_MAX_REACHED,
	TSQ_SUCCESS,
	TSQ_FAILURE,
	TSQ_TIMEOUT
#else
	AUTH_DENY_MAX_REACHED
#endif
} TRIGGER_TYPE;

typedef enum return_to_idle_reason
{
	IDLE_SUCCESS,
	IDLE_CHAN_MEAS_FAILED,
	IDLE_CHAN_MEAS_DISALLOWED,	
	IDLE_BSS_NOT_FOUND,
#ifdef SUPPORT_MULTI_AP
	IDLE_RFS_FAILED,
#endif
	IDLE_EXEC_FAILED,
#ifdef SUPPORT_MULTI_AP
	IDLE_UNEXPECTED_TRIGGER,
	IDLE_MANDATE_RECEIVED
#else
	IDLE_UNEXPECTED_TRIGGER
#endif
	//add additional here
} IDLE_STATUS_CODE;

typedef enum return_to_invalid_reason
{
	INVALID_SUCCESS,
	INVALID_CLIENT_LEFT
	//add additional here
} INVALID_STATUS_CODE;

typedef enum move_to_coord_reason
{
	COMPLETE_SUCCESS,
#ifdef SUPPORT_MULTI_AP
	COMPLETE_FAIL_REJ_ANOTHER_DEVICE,
#endif
	COMPLETE_FAIL_AUTHDENYMAX_EXCEEDED,
	COMPLETE_FAIL_LOW_RSSI_ON_TARGET,
	COMPLETE_FAIL_STA_REJECTED_BTM_REQUEST,
	COMPLETE_FAIL_BTM_RESPONSE_TIMEOUT,
	COMPLETE_FAIL_WRONG_BSS,
	COMPLETE_FAIL_ASSOC_TIMEOUT,
#ifdef SUPPORT_MULTI_AP
	COMPLETE_FAIL_MANDATE,
	COMPLETE_FAIL_RFS_UNFRIENDLY,
	COMPLETE_FAIL_RFS_PROHIBIT,
#endif
	COMPLETE_FAIL_UNEXPECTED_TRIGGER

} COMPLETE_STATUS_CODE;

/* ***************************************************************************** */

/* Structure definitions for various state data passing */
/* ***************************************************************************** */
typedef struct fsm_data_struct_invalid
{
	INVALID_STATUS_CODE exit_code;
	//Add additional requirements here
} INVALID_DATA;

typedef struct fsm_data_struct_idle
{
	IDLE_STATUS_CODE exit_code;
		//add other required parameters here
} IDLE_DATA;

typedef struct fsm_data_struct_decision
{
	STEERING_METHOD_TYPE steer_method; } DECISION_DATA;

typedef struct fsm_data_struct_preparation
{ 
	u8 bss[ETH_ALEN];

} PREPARATION_DATA;

typedef struct fsm_data_struct_exec_monitor
{
	u8 bss[ETH_ALEN];

} EXEC_MONITOR_DATA;

typedef struct fsm_data_structure_complete
{
	COMPLETE_STATUS_CODE reason_code;
} COMPLETE_STATE_DATA;
/* ***************************************************************************** */

/* Declarations for the various methods */
/* ***************************************************************************** */

struct mapd_global;

void steer_fsm_step_state(struct mapd_global *pGlobal_dev, int client_id,
			  STEERING_STATE next_state, void *priv );
void steer_fsm_trigger(struct mapd_global *pGlobal_dev, int client_id,
		       TRIGGER_TYPE trigger, void *user_data);
void steer_fsm_invalid_state_handler(struct mapd_global *global, int client_id,      
		        STEERING_STATE previous_state, void *data);
void steer_fsm_idle_state_handler(struct mapd_global *pGlobal_dev, int client_id,
				  STEERING_STATE previous_state, void *data);
void steer_fsm_decision_state_handler(struct mapd_global *pGlobal_dev, 
				      int client_id, 
				      STEERING_STATE previous_state,
				      void *data);
void steer_fsm_preparation_state_handler(struct mapd_global *pGlobal_dev, 
					 int client_id,
					 STEERING_STATE previous_state,
					 void *data);
void steer_fsm_exec_monitor_state_handler(struct mapd_global *pGlobal_dev, 
					  int client_id,
					  STEERING_STATE previous_state,
					  void *data);
void steer_fsm_complete_state_handler(struct mapd_global *pGlobal_dev, 
				      int client_id,
				      STEERING_STATE previous_state,
				      void *data);

void steer_fsm_invalid_trigger_handler (struct mapd_global *pGlobal_dev,
		int client_id, TRIGGER_TYPE trigger, void *user_data);

void steer_fsm_idle_trigger_handler(struct mapd_global *pGlobal_dev, 
				    int client_id, TRIGGER_TYPE trigger,
				    void *user_data);
void steer_fsm_decision_trigger_handler(struct mapd_global *pGlobal_dev, 
					int client_id, TRIGGER_TYPE trigger,
					void *user_data);
void steer_fsm_preparation_trigger_handler(struct mapd_global *pGlobal_dev, 
					   int client_id, TRIGGER_TYPE trigger, 
					   void *user_data);
void steer_fsm_exec_monitor_trigger_handler(struct mapd_global *pGlobal_dev,
					    int client_id, TRIGGER_TYPE trigger,
					    void *user_data);
void steer_fsm_complete_trigger_handler(struct mapd_global *pGlobal_dev,
					int client_id, TRIGGER_TYPE trigger, 
					void *user_data);

void make_idle_data(IDLE_DATA *data, IDLE_STATUS_CODE exit_code);
void make_decision_data(DECISION_DATA *data, STEERING_METHOD_TYPE steer_method);
void make_preparation_data(PREPARATION_DATA *data, u8 *bss);
void make_exec_monitor_data(EXEC_MONITOR_DATA *data, u8 *bss);
void make_complete_data(COMPLETE_STATE_DATA *data, 
			COMPLETE_STATUS_CODE reason_code);
const char *get_steer_state_str(STEERING_STATE steer_state);
const char *get_steer_trigger_str(TRIGGER_TYPE type);
const char *str_method_str(STEERING_METHOD_TYPE steer_method);
/* ***************************************************************************** */
#endif
