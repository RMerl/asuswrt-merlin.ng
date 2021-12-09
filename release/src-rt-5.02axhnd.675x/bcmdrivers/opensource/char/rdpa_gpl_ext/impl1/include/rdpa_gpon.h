/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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

#ifndef _RDPA_GPON_H_
#define _RDPA_GPON_H_

#include "bcm_ploam_api.h"
#include "rdpa_types.h"
#include "bcm_intr.h"
#include "rdpa_gpon_cfg.h"
#include <bcm_ploam_api.h>

#define RDPA_IC_GPON_RX_IRQ   INTERRUPT_ID_WAN_GPON_RX   /**< GPON rx IRQ in IC */
#define RDPA_IC_GPON_TX_IRQ   INTERRUPT_ID_WAN_GPON_TX   /**< GPON tx IRQ in IC */

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
#define RDPA_IC_NGPON_RX_IRQ     INTERRUPT_ID_WAN_XGRX         /**< XGPON/NGPON rx IRQ in IC */
#define RDPA_IC_NGPON_TX_1_IRQ   INTERRUPT_ID_WAN_XGTX_INTR1   /**< XGPON/NGPON tx IRQ #1 in IC */
#define RDPA_IC_NGPON_TX_0_IRQ   INTERRUPT_ID_WAN_XGTX_INTR0   /**< XGPON/NGPON tx IRQ #0 in IC */
#elif defined(CONFIG_BCM963158)
#define RDPA_IC_NGPON_RX_IRQ     INTERRUPT_ID_WAN_XGRX         /**< XGPON/NGPON rx IRQ in IC */
#define RDPA_IC_NGPON_TX_1_IRQ   INTERRUPT_ID_WAN_XGTX_INTR1   /**< XGPON/NGPON tx IRQ #1 in IC */
#define RDPA_IC_NGPON_TX_0_IRQ   INTERRUPT_ID_WAN_XGTX_INTR0   /**< XGPON/NGPON tx IRQ #0 in IC */
#else
#define RDPA_IC_NGPON_RX_IRQ     0         
#define RDPA_IC_NGPON_TX_1_IRQ   0   
#define RDPA_IC_NGPON_TX_0_IRQ   0  
#endif


/** \defgroup xgpon xGPON Management
 * Objects in this group control xGPON-related configuration
 */

/**
 * \defgroup gpon Link Management
 * \ingroup xgpon
 * Objects and functions in this group are used for configuration and
 * status monitoring on PON link and ONU level:
 * - PON link configuration
 * - PON link statistics
 * - ONU SN and password configuration
 * @{
 */

 #define RDPA_PLOAM_LENGTH 12


/** PON indications */
typedef enum
{
    rdpa_indication_lof,                            /**< LOF indication */
    rdpa_indication_err,                            /**< ERR indication */
    rdpa_indication_sf,                             /**< SF indication */
    rdpa_indication_sd,                             /**< SD indication */
    rdpa_indication_lcdg,                           /**< LCDG indication */
    rdpa_indication_dact,                           /**< DACT indication */
    rdpa_indication_dis,                            /**< DIS indication */
    rdpa_indication_pee,                            /**< PEE indication */
    rdpa_indication_mem,                            /**< MEM indication */ 
    rdpa_indication_pst_msg,                        /**< PST Message indication */
    rdpa_indication_ranging_start,                  /**< Ranging start indication */
    rdpa_indication_ranging_stop,                   /**< Ranging stop indication */
    rdpa_indication_assign_alloc_id_msg,            /**< Assign Alloc ID Message indication */
    rdpa_indication_cfg_port_id_msg,                /**< Configure Port ID Message indication */
    rdpa_indication_encrypted_port_id_msg,          /**< Encrypted Port ID Message indication */
    rdpa_indication_key_switched_out_of_sync,       /**< Key switched out of sync indication */
    rdpa_indication_key_switched,                   /**< Key switched indication */
    rdpa_indication_state_transition,               /**< State transition indication */
    rdpa_indication_assign_onu_id_msg,              /**< Assign ONU ID Message indication */
    rdpa_indication_link_state_transition,          /**< Link state transition indication */
    rdpa_indication_rogue_onu,                      /**< Rogue ONU indication */
    rdpa_indication_pmd_alarm,                      /**< PMD alarm indication */
    rdpa_indication_reboot_msg,                     /**< Reboot Message */
    rdpa_indication_none = 0xffff
}rdpa_pon_indication;

/** PON link administrative state */
typedef enum {
    rdpa_pon_admin_state_not_ready,     /**< Not ready */
    rdpa_pon_admin_state_inactive,      /**< Inactive */
    rdpa_pon_admin_state_active         /**< Active */
} rdpa_pon_admin_state;

/** PON link sub state */
typedef enum {
    rdpa_pon_oper_state_none,               /**< Not set */
    rdpa_pon_oper_state_standby,            /**< Standby */
    rdpa_pon_oper_state_operational,        /**< Operational */
    rdpa_pon_oper_state_stop,               /**< Stop */
    rdpa_pon_o1_no_sync_sub_state,          /**< No Sync substate of O1 */
    rdpa_pon_o1_profile_learn_sub_state,    /**< Profile Learning substate of O1 */
    rdpa_pon_o5_accociated_sub_state,       /**< Associated substate of O5 */
    rdpa_pon_o5_pending_sub_state,          /**< Pending [for tuning] substate of O5 */
    rdpa_pon_o8_no_sync_sub_state,          /**< No Sync sub state of O8 - tuning to another channel (NGPON2 only) */
    rdpa_pon_o8_profile_learn_sub_state     /**< Profile Learning substate of O8 - tuning to another channel (NGPON2 only) */
}  rdpa_pon_sub_state;

/** PON link Operational state */
typedef enum {
    rdpa_pon_init_o1,          /**< o1 */
    rdpa_pon_standby_o2,       /**< standby_o2*/
    rdpa_pon_serial_num_o3,    /**< serial_num_o3*/
    rdpa_pon_ranging_o4,       /**< ranging_o4*/
    rdpa_pon_operational_o5,   /**< operational_o5*/
    rdpa_pon_popup_o6,         /**< popup_o6*/
    rdpa_pon_emergency_stop_o7,/**< emergency_stop_o7*/
    rdpa_pon_ds_tuning_o8,     /**< ds_tuning_o8*/
    rdpa_pon_us_tuning_o9,     /**< us_tuning_o9*/
    rdpa_pon_serial_num_o2_3   /**< serial_num_o2_3*/
}  rdpa_pon_link_operational_state;

/** PON Link state */
typedef struct
{
    rdpa_pon_admin_state admin_state;                   /**< Administrative state */
    rdpa_pon_sub_state sub_state;                       /**< Sub state */       
    rdpa_pon_link_operational_state link_oper_state;    /**< Link operational state */
}rdpa_pon_link_state;

/** Ranging stop reason */
typedef enum
{
    rdpa_ranging_stop_to1_timeout,              /** TO1  timeout */
    rdpa_ranging_stop_link_folt,                /** Link folt */
    rdpa_ranging_stop_deactivate_onu,           /** Deactivate ONU */
    rdpa_ranging_stop_disable_onu,              /** Disable ONU */
    rdpa_ranging_stop_successful_ranging,       /** Successful ranging */
    rdpa_ranging_stop_unmatch_sn,               /** Unmatched serial number */
    rdpa_ranging_stop_link_deactivate           /** Link deactivated */
}rdpa_ranging_stop_reason;

/** PON status indication enable*/
typedef enum
{
    rdpa_indication_on,                         /** Indication enabled */
    rdpa_indication_off                         /** Indication disabled */
}rdpa_pon_status_indication;

/** ALLOC ID parameters */
typedef struct
{
    uint32_t alloc_id;
    rdpa_pon_status_indication status;
}rdpa_alloc_id_param;

/** Port ID parameters */
typedef struct
{
    uint32_t port_id;
    rdpa_pon_status_indication status;
}rdpa_cfg_port_id_param;

/** Port Encryption */
typedef struct
{
    uint32_t port_id;
    bdmf_boolean encryption;
}rdpa_encrypted_port_id_param;

/** Key switching */
typedef struct
{
    uint32_t current_superframe ;
    uint32_t switch_over_superframe ;
}rdpa_key_switch_param;

/** PON sub state */
typedef struct
{
    rdpa_pon_sub_state old_state;
    rdpa_pon_sub_state new_state;
}rdpa_oper_state_param;

/** PON Link operational state */
typedef struct
{
    rdpa_pon_link_operational_state old_state;
    rdpa_pon_link_operational_state new_state;
}rdpa_link_state_param;

/** Rogue ONU mode */
typedef enum
{
    rdpa_monitor_rogue_mode,
    rdpa_fault_rogue_mode,
}rdpa_rogue_onu_mode;

/** Rogue ONU parameters */
typedef struct
{
    bdmf_boolean status;
    rdpa_rogue_onu_mode type;
}rdpa_rogue_onu_param;

/** Rogue ONU Detection Mode */
typedef enum
{
    rdpa_rogue_tx_monitor_mode,
    rdpa_rogue_tx_fault_mode,
} rdpa_rogue_mode_t;

/** Rogue ONU Default Pin Configuration */
typedef struct
{
    uint32_t default_pin_mode_reg;  /**< Default pin mode cfg */
    uint32_t default_loop_sel_cfg;  /**< Default loop select reg configuration */
    uint32_t default_pin_mux_data;  /**< Default pin mux data */
} rdpa_rogue_default_pin_cfg_t;

/** Rogue ONU parameters */
typedef struct
{
    bdmf_boolean enable;    /**< Rogue State: disable / enable */
    rdpa_rogue_mode_t mode; /**< Rogue ONU Detection Mode: TX_MONITOR / TX_FAULT */
    uint32_t clock_cycle;   /**< Rogue ONU window_size in clock cycles */
} rdpa_rogue_onu_t;

/** MISC transmit parameters */
typedef struct
{
    bdmf_boolean enable;    /**< Generate tx burst depended data State: disable / enable */
    uint8_t prodc[16];      /**< Generate tx burst depended data Preamble overhead content */
    uint8_t prcl;           /**< Generate tx burst depended data configured length */
    uint8_t brc;            /**< Generate tx burst depended data Byte repetition content */
    uint8_t prl;            /**< Generate tx burst depended data Preamble repetition length */
    uint16_t msstart;       /**< Generate tx burst depended data frame start */
    uint16_t msstop;        /**< Generate tx burst depended data frame stop */
} rdpa_misc_tx_t;

/** PON transmit */
typedef enum
{
    rdpa_pon_tx,
    rdpa_pon_rx,
    rdpa_pon_none
} rdpa_pon_transmit;

/** PMD Alarm parameters */
typedef struct
{
    bdmf_boolean esc_be                ;
    bdmf_boolean esc_rogue             ;
    bdmf_boolean esc_mod_over_current  ;
    bdmf_boolean esc_bias_over_current ;
    bdmf_boolean esc_mpd_fault         ;
    bdmf_boolean esc_eye_safety        ;
    bdmf_boolean fault_alarm;
    bdmf_boolean esc_alarm;
}rdpa_pmd_alarm_param;

/** GPON Callback Indication */
typedef union
{
    rdpa_pon_status_indication lof_status;                      /* LOF status */
    uint32_t bip8_errors;                                       /* ERR parameters */
    rdpa_pon_status_indication sf_status;                       /* SF status */
    rdpa_pon_status_indication sd_status;                       /* SD status */
    rdpa_pon_status_indication lcdg_status;                     /* LCDG status */
    rdpa_pon_status_indication dis_status;                      /* DIS status */
    uint8_t unkown_ploam_mem[RDPA_PLOAM_LENGTH];                /* MEM parameters */
    uint8_t pst_ploam[RDPA_PLOAM_LENGTH];                       /* PST parameters */
    rdpa_ranging_stop_reason ranging_stop;                      /* Ranging stop parameters */
    rdpa_alloc_id_param assign_alloc_id_parameters;             /* Assign alloc ID parameters */
    uint16_t onu_id;                                            /* Assign ONU ID parameters */
    rdpa_cfg_port_id_param configure_port_id_parameters;        /* Configure port ID parameters */
    rdpa_encrypted_port_id_param encrypted_port_id_parameters;  /* Encrypted port ID parameters */
    rdpa_key_switch_param key_switch_parameters;                /* Key switch parameters */
    rdpa_oper_state_param state_transition_parameters;          /* State transition parameters */
    rdpa_link_state_param link_state_transition_parameters;     /* Link State transition parameters */
    rdpa_rogue_onu_param rogue_onu;                             /* Rogue ONU parameters */
    rdpa_pmd_alarm_param pmd_alarm;                             /* PMD Alarm parameters */
    PON_REBOOT_PLOAM_FLAGS reboot_ploam_flags;                  /* reboot ploam flags */
} rdpa_callback_indication;

/** PON-level counters.
 * Underlying type for gpon_stat aggregate
 */
typedef struct 
{
    uint32_t bip_errors;        /**< BIP error counter */
    uint32_t psbd_hec_errors;   /**< PSBd HEC error count */
    uint32_t xgtc_hec_errors;   /**< XGTC HEC error count */
    uint32_t xgem_key_errors;   /**< XGEM encryption key error count */
    uint32_t xgem_hec_errors;   /**< XGEM HEC error count */
    uint32_t fec_corrected_bits;/**< FEC corrected bits  */
    uint32_t fec_corrected_sym; /**< FEC corrected symbols */
    uint32_t fec_corrected_cw;  /**< FEC corrected codewords */
    uint32_t fec_uncorr_cw;     /**< FEC uncorrectable codewords */

    uint32_t crc_errors;        /**< PLOAM CRC or MIC errors */
    uint32_t rx_onu_id;         /**< Valid ONU id PLOAM counter */
    uint32_t rx_broadcast;      /**< Broadcast PLOAM counter */
    uint32_t rx_unknown;        /**< Rx unknown PLOAM counter */
    uint32_t rx_ploam;          /**< Total Rx PLOAM counter */
    uint32_t rx_profiles;       /**< Rx profile PLOAMs counter */
    uint32_t rx_ranging;        /**< Rx ranging PLOAMs counter */
    uint32_t rx_deactivate;     /**< Rx deactivate PLOAMs counter */
    uint32_t rx_disable;        /**< Rx disable PLOAMs counter */
    uint32_t rx_reg_req;        /**< Rx registration request PLOAM counter */
    uint32_t rx_alloc_id;       /**< Rx assign alloc id PLOAM counter */
    uint32_t rx_key_cntrl;      /**< Rx key control PLOAM counter */
    uint32_t rx_sleep;          /**< Rx sleep allow PLOAM counter */

    uint16_t tx_illegal_access; /**< Tx illegal access PLOAM counter */
    uint32_t tx_idle;           /**< Tx idle PLOAM counter */
    uint32_t tx_ploam;          /**< Tx PLOAM counter */
    uint32_t tx_reg;            /**< Tx registration PLOAM counter */
    uint32_t tx_ack;            /**< Tx acknowledge PLOAM counter */
    uint32_t tx_key_rep;        /**< Tx key report PLOAM counter */
    uint32_t tx_sleep_request;  /**< Tx sleep request PLOAM counter */
    uint32_t fec_errors;        /**< obsolete, duplicates fec_uncorr_cw*/
    uint32_t hec_errors;        /**< obsolete, duplicates psbd_hec_errors*/

    uint32_t success_act;       /**< successful activations counter*/
} rdpa_gpon_stat_t;

/** ONU serial number.
 * Underlying type for onu_sn aggregate
 */
typedef struct 
{
    uint8_t vendor_id[4];       /**< Vendor id */
    uint8_t vendor_specific[4]; /**< Vendor-specific number */
} rdpa_onu_sn_t;

/** ONU password */
typedef struct 
{
    uint8_t password[BCM_PLOAM_PASSWORD_SIZE_BYTES]; // [36]  //[10]; 
} rdpa_onu_password_t;


/** Send PLOAM type */
typedef enum
{
    gpon_ploam_type_dgasp, /**< Dying Gasp */
    gpon_ploam_type_pee, /**< PEE */
    gpon_ploam_type_pst, /**< PST */
} rdpa_ploam_type_t;

/** PST parameters */
typedef struct
{
    uint32_t k1_value; /**< K1 */
    uint32_t k2_value; /**< K2 */
    uint32_t line_number; /**< Line number */
} rdpa_pst_params_t;

/** Send PLOAM parameters */
typedef struct
{
    rdpa_ploam_type_t ploam_type;  /**< PLOAM Type: Dying Gasp / PEE / PST */
    rdpa_pst_params_t pst_params; /**< PST parameters: K1, K2, Line number */
} rdpa_send_ploam_params_t;

/** DBA status report */
typedef enum
{
    gpon_dba_disable, /**< Disable  */
    gpon_dba_interval_1_msec, /**< 1 milisec */
    gpon_dba_interval_2_msec, /**< 2 milisec */
    gpon_dba_interval_3_msec, /**< 3 milisec */
    gpon_dba_interval_4_msec, /**< 4 milisec */
    gpon_dba_interval_5_msec, /**< 5 milisec */
    gpon_dba_interval_6_msec, /**< 6 milisec */
    gpon_dba_interval_7_msec, /**< 7 milisec */
    gpon_dba_interval_8_msec, /**< 8 milisec */
    gpon_dba_interval_9_msec, /**< 9 milisec */
    gpon_dba_interval_10_msec, /**< 10 milisec */
    gpon_dba_interval_125_usec = 125, /**< 125 microsec */
    gpon_dba_interval_250_usec = 250, /**< 250 microsec */
    gpon_dba_interval_500_usec = 500, /**< 500 microsec */
} rdpa_dba_interval_t;

/** Link activate / deactivate */
typedef enum
{
    rdpa_link_deactivate, /**< Deactivate */
    rdpa_link_activate_O1, /**< Activate: init at initial state */
    rdpa_link_activate_O7, /**< Activate: init at Emergency stop state */
} rdpa_link_activate_t;

/** GPON MAC Operational Mode */
typedef enum 
{
    rdpa_stack_mode_xgpon,            /**< ITU-T G.987 XGPON */
    rdpa_stack_mode_ngpon2_10g,       /**< ITU-T G.989 NGPON2 10Gbps US */
    rdpa_stack_mode_ngpon2_2_5g,      /**< ITU-T G.989 NGPON2 10Gbps DS - 2.5Gbps US */
    rdpa_stack_mode_xgs,              /**< ITU-T G.tbd XGS 10Gbps US */
    rdpa_stack_mode_gpon,             /**< ITU-T G.984 GPON */
} rdpa_gpon_stack_mode_t;

/** GPON Encryption Type */
typedef enum 
{
    rdpa_no_enc,                     /**< No encryption */
    rdpa_unicast_both_dir_enc,       /**< Unicast payload encryption in both directions */
    rdpa_broadcast_enc,              /**< Broadcast (multicast) encryption */
    rdpa_unicast_ds_enc              /**< Unicast encryption, downstream only */
} rdpa_enc_ring_t;

/** DS GEM flow Type. */
typedef enum 
{
    rdpa_gpon_flow_type_ethernet,   /**< Ethernet Flow */
    rdpa_gpon_flow_type_omci        /**< OMCI Flow */
} rdpa_gpon_flow_type;


void _ploam_isr_callback(void);

/** GEM flow DS configuration.
 * Underlying type for rdpa_gpon_gem_ds_cfg_t aggregate
 */
typedef struct 
{
    uint16_t port; /**< GEM port */
    rdpa_discard_prty discard_prty; /**< Discard priority */
    bdmf_boolean encryption; /**< Encryption */
    bdmf_boolean crc;
    rdpa_enc_ring_t enc_ring;
    rdpa_gpon_flow_type flow_type;  /**< OMCI / ethernet */
} rdpa_gpon_gem_ds_cfg_t;

/** TCONT statistics.
 * Underlying type for tcont_stat aggregate
 */
typedef struct 
{
    uint32_t idle_gem_counter; /**< Idle GEM Counter */
    uint32_t non_idle_gem_counter; /**< Non-Idle GEM Counter */
    uint32_t packet_counter; /**< Packet Counter - number of transmitted packets from TCONT*/
    uint32_t valid_access_counter; /**< Valid accesses for TCONT */
} rdpa_tcont_stat_t;


/** Multicast encryption key selector */


/** Multicast encryption key paramters.
 * Underlying type for mcast_enc_key aggregate
 */
typedef struct 
{
    uint8_t is_encrypted;
    uint8_t key_1[16]; /**< encryption key */
    uint8_t key_2[16]; /**< encryption key */
} rdpa_gpon_mcast_enc_key_param_t;


typedef struct 
{
    uint8_t pon_id_type; /**< PON-ID type: an indication of the ODN architecture, 
                              the source of the reported launch power and the ODN class 
                              as read from PON-ID ploam */
    uint8_t pon_identifier[7]; /**< PON Identifier as read from PON-ID ploam. */
    uint8_t tx_optical_level[2]; /**< Transmit Optical level [TOL], an indication of the current 
                                      transceiver launch power of the appropriate network element 
                                      as read from PON-ID ploam */
} rdpa_gpon_pon_id_param_t;

/** BW recording Setting paramters.
 * Underlying type for bw_record_cfg aggregate
 */
typedef struct 
{
    bdmf_boolean rcd_stop; /**<  Whether to stop on map end or to stop when memory is full. 
                                 If asserted, only one map will be recorded and the recording 
                                 will be stopped at the end of the first map after recording  
                                 enable. */
    uint32_t record_type; /**< Type of record:                                         
                               0 - Record all accesses (Alloc-ID paramter is ignored)  
                               1 - Record only the accesses which are directed to one  
                                   of the 40 TCONTs of the ONU (This mode is not relevant for XGPON).                                                                                             
                               2 - Record only accesses of a specific alloc-id. */
    uint32_t alloc_id; /**< Configurable specific alloc id to record the access.         
                            valid if rcd_type = 2. */

    bdmf_boolean enable; /**<  Enable/Disable BW recording */

} rdpa_gpon_bw_record_cfg_t;


/** BW recording Results - GPON paramters.
 * Underlying type for bw_record_result_gpon aggregate
 */
typedef struct
{
    uint32_t alloc_id;              
    bdmf_boolean flag_pls;         
    bdmf_boolean flag_ploam;        
    bdmf_boolean flag_fec;          
    uint32_t flag_dbru;         
    bdmf_boolean crc_valid;         
    uint32_t sstart;                
    uint32_t sstop;                 
    uint32_t sf_counter;            
}rdpa_gpon_bw_record_result_gpon_t; 
                                    
/** BW recording Results - NGPON paramters.
 * Underlying type for bw_record_result_ngpon aggregate
 */
typedef struct
{
    uint16_t starttime;
    uint16_t allocid;
    uint16_t sfc_ls;
    bdmf_boolean hec_ok;
    uint8_t bprofile;
    bdmf_boolean fwi;
    bdmf_boolean ploamu;
    bdmf_boolean dbru;
    uint16_t grantsize;
}rdpa_gpon_bw_record_result_ngpon_t;


/** @} end of pon Doxygen group */

#endif /* _RDPA_GPON_H_ */
