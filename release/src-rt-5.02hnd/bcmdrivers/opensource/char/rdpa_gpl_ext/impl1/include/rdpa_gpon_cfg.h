#ifndef _RDPA_GPON_CFG_H_
#define _RDPA_GPON_CFG_H_

#include "bdmf_data_types.h"

typedef enum {
    rdpa_polarity_active_low,       /**< Active low */
    rdpa_polarity_active_high,      /**< Active high */
} rdpa_polarity;


/** GPON Link Configuration */
typedef struct
{
    uint32_t to1_timeout;                             /**< TO1 timer */
    uint32_t to2_timeout;                             /**< TO2 timer */
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

#endif /* _RDPA_GPON_CFG_H_ */
