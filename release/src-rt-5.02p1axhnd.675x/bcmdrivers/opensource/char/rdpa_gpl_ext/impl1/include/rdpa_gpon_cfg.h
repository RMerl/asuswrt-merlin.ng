#ifndef _RDPA_GPON_CFG_H_
#define _RDPA_GPON_CFG_H_

#include "bdmf_data_types.h"

#define DELIMITER_MAX_LENGTH  8
#define PREAMBLE_MAX_LENGTH   8
#define PON_TAG_SIZE          8


typedef enum {
    rdpa_polarity_active_low,       /**< Active low */
    rdpa_polarity_active_high,      /**< Active high */
} rdpa_polarity;


/** GPON Link Configuration */
typedef struct
{
    uint32_t to1_timeout;                             /**< TO1 timer */
    uint32_t to2_timeout;                             /**< TO2 timer */
#ifdef SUPPORT_TO6
    uint32_t to6_timeout;                             /**< TO6 timer */
#endif
    uint32_t dwell_timer_timeout;                     /**< dwell timer */
    uint32_t ber_interval;                            /**< Ber interval */
    uint32_t min_response_time;                       /**< Min response time */
    rdpa_polarity tx_data_polarity;                    /**< Tx data polarity */
    uint32_t transceiver_dv_setup_pattern;            /**< Transceiver DV setup pattern */
    uint32_t transceiver_dv_hold_pattern;             /**< Transceiver DV hold pattern */
    rdpa_polarity transceiver_dv_polarity;            /**< Transceiver DV polarity */
    bdmf_boolean transceiver_power_calibration_mode;  /**< Transceiver power calibration mode */
    uint32_t transceiver_power_calibration_pattern;   /**< Transceiver power calibration pattern */
    uint32_t transceiver_power_calibration_size;                /**< Transceiver power calibration size */
    bdmf_boolean rx_din_polarity;                     /**< RX Data Polarity */
    uint32_t ber_threshold_for_sd_assertion;          /**< BER threshold for SD assertion */
    uint32_t ber_threshold_for_sf_assertion;          /**< BER threshold for SF assertion */
    uint32_t number_of_psyncs_for_lof_assertion;      /**< Number of PSYNCs for LOF assertion */
    uint32_t number_of_psyncs_for_lof_clear;          /**< Number of PSYNCs for LOF clear */
} rdpa_gpon_link_cfg_t;


/** GPON Overhead Configuration */
typedef struct
{
    uint8_t overhead[16];                             /**< Overhead */
    uint8_t overhead_len;                             /**< Overhead length */
    uint8_t overhead_repetition;                      /**< Overhead repetition */
    uint8_t overhead_repetition_len;                  /**< Overhead repetition length */
}rdpa_gpon_overhead_cfg_t;

typedef struct
{
    bdmf_boolean profile_enable;
    uint8_t profile_index;
    uint8_t profile_version;
    uint8_t fec_ind;
    uint8_t delimiter_len;
    uint8_t fec_type;        /* AkivaS for NGPON2 */
    uint8_t delimiter[DELIMITER_MAX_LENGTH];
    uint8_t preamble_len;
    uint8_t preamble_repeat_counter ;
    uint8_t preamble[PREAMBLE_MAX_LENGTH];
    uint8_t pon_tag[PON_TAG_SIZE];
#ifdef G989_3_AMD2
    uint32_t downstream_pon_id;                       /* aligned, host order */
    uint8_t cross;
#endif
    uint8_t specific_line_rate;
}
XGPON_BURST_PROFILE_INFO;

/** Underlying type for burst profile aggregate */
typedef XGPON_BURST_PROFILE_INFO rdpa_gpon_burst_prof_t;

#endif /* _RDPA_GPON_CFG_H_ */
