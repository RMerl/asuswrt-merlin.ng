/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _RDPA_TYPES_H_
#define _RDPA_TYPES_H_

/** \defgroup types Commonly used types and constants
 * @{
 */

#include <bdmf_data_types.h>


/** EMAC id */
typedef enum
{
    rdpa_emac0,             /**< EMAC0 */
    rdpa_emac1,             /**< EMAC1 */
    rdpa_emac2,             /**< EMAC2 */
    rdpa_emac3,             /**< EMAC3 */
    rdpa_emac4,             /**< EMAC4 */
    rdpa_emac5,             /**< EMAC5 */
    rdpa_emac6,             /**< EMAC6 */
    rdpa_emac7,             /**< EMAC7 */
    rdpa_emac__num_of,      /* Max number of EMACs */
    rdpa_emac_none,      /**< Indicates virtual port */
} rdpa_emac;
/** EMAC rates */
typedef enum
{
    rdpa_emac_rate_10m,     /**< 10 Mbps */
    rdpa_emac_rate_100m,    /**< 100 Mbps */
    rdpa_emac_rate_1g,      /**< 1 Gbps */
    rdpa_emac_rate_2_5g,    /**< 2.5 Gbps */

    rdpa_emac_rate__num_of, /* Number of rates */
} rdpa_emac_rate;

/** EMAC configuration */
typedef struct
{
    char loopback;      /**< 1 = line loopback */
    rdpa_emac_rate rate;       /**< EMAC rate */
    char generate_crc;  /**< 1 = generate CRC */
    char full_duplex;   /**< 1 = full duplex */
    char pad_short;     /**< 1 = pad short frames */
    char allow_too_long;/**< 1 = allow long frames */
    char check_length;  /**< 1 = check frame length */
    uint32_t preamble_length;   /**< Preamble length */
    uint32_t back2back_gap;     /**< Back2Back inter-packet gap */
    uint32_t non_back2back_gap; /**< Non Back2Back inter-packet gap */
    uint32_t min_interframe_gap; /**< Min inter-frame gap */
    char rx_flow_control;/**< 1 = enable RX flow control */
    char tx_flow_control;/**< 1 = enable TX flow control */
} rdpa_emac_cfg_t;
/** RX RMON counters.
 * Underlying type for emac_rx_stat aggregate type.
 */
typedef struct
{
    uint32_t byte;              /**< Receive Byte Counter */
    uint32_t packet;            /**< Receive Packet Counter */
    uint32_t frame_64;          /**< Receive 64 Byte Frame Counter */
    uint32_t frame_65_127;      /**< Receive 65 to 127 Byte Frame Counter */
    uint32_t frame_128_255;     /**< Receive 128 to 255 Byte Frame Counter */
    uint32_t frame_256_511;     /**< Receive 256 to 511 Byte Frame Counter */
    uint32_t frame_512_1023;    /**< Receive 512 to 1023 Byte Frame Counter */
    uint32_t frame_1024_1518;   /**< Receive 1024 to 1518 Byte Frame Counter */
    uint32_t frame_1519_mtu;    /**< Receive 1519 to MTU Frame Counter */
    uint32_t multicast_packet;  /**< Receive Multicast Packet */
    uint32_t broadcast_packet;  /**< Receive Broadcast Packet */
    uint32_t unicast_packet;    /**< Receive Unicast Packet */
    uint32_t alignment_error;   /**< Receive Alignment error */
    uint32_t frame_length_error;/**< Receive Frame Length Error Counter */
    uint32_t code_error;        /**< Receive Code Error Counter */
    uint32_t carrier_sense_error;/**< Receive Carrier sense error */
    uint32_t fcs_error;         /**< Receive FCS Error Counter */
    uint32_t control_frame;     /**< Receive Control Frame Counter */
    uint32_t pause_control_frame;/**< Receive Pause Control Frame */
    uint32_t unknown_opcode;    /**< Receive Unknown opcode */
    uint32_t undersize_packet;  /**< Receive Undersize Packet */
    uint32_t oversize_packet;   /**< Receive Oversize Packet */
    uint32_t fragments;         /**< Receive Fragments */
    uint32_t jabber;            /**< Receive Jabber counter */
    uint32_t overflow;          /**< Receive Overflow counter */
} rdpa_emac_rx_stat_t;
/** Tx RMON counters.
 * Underlying type for emac_tx_stat aggregate type.
 */
typedef struct
{
    uint32_t byte;              /**< Transmit Byte Counter */
    uint32_t packet;            /**< Transmit Packet Counter */
    uint32_t frame_64;          /**< Transmit 64 Byte Frame Counter */
    uint32_t frame_65_127;      /**< Transmit 65 to 127 Byte Frame Counter */
    uint32_t frame_128_255;     /**< Transmit 128 to 255 Byte Frame Counter */
    uint32_t frame_256_511;     /**< Transmit 256 to 511 Byte Frame Counter */
    uint32_t frame_512_1023;    /**< Transmit 512 to 1023 Byte Frame Counter */
    uint32_t frame_1024_1518;   /**< Transmit 1024 to 1518 Byte Frame Counter */
    uint32_t frame_1519_mtu;    /**< Transmit 1519 to MTU Frame Counter */
    uint32_t fcs_error;         /**< Transmit FCS Error */
    uint32_t multicast_packet;  /**< Transmit Multicast Packet */
    uint32_t broadcast_packet;  /**< Transmit Broadcast Packet */
    uint32_t unicast_packet;    /**< Transmit Unicast Packet */
    uint32_t excessive_collision; /**< Transmit Excessive collision counter */
    uint32_t late_collision;    /**< Transmit Late collision counter */
    uint32_t single_collision;  /**< Transmit Single collision frame counter */
    uint32_t multiple_collision;/**< Transmit Multiple collision frame counter */
    uint32_t total_collision;   /**< Transmit Total Collision Counter */
    uint32_t pause_control_frame; /**< Transmit PAUSE Control Frame */
    uint32_t deferral_packet;   /**< Transmit Deferral Packet */
    uint32_t excessive_deferral_packet; /**< Transmit Excessive Deferral Packet */
    uint32_t jabber_frame;      /**< Transmit Jabber Frame */
    uint32_t control_frame;     /**< Transmit Control Frame */
    uint32_t oversize_frame;    /**< Transmit Oversize Frame counter */
    uint32_t undersize_frame;   /**< Transmit Undersize Frame */
    uint32_t fragments_frame;   /**< Transmit Fragments Frame counter */
    uint32_t error;             /**< Transmission errors*/
    uint32_t underrun;          /**< Transmission underrun */
} rdpa_emac_tx_stat_t;
/** Emac statistics */
typedef struct
{
    rdpa_emac_rx_stat_t rx; /**< Emac Receive Statistics */
    rdpa_emac_tx_stat_t tx; /**< Emac Transmit Statistics */
} rdpa_emac_stat_t;
#endif /* _RDPA_TYPES_H_ */

