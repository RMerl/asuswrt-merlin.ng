/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/



/* This is an automated file. Do not edit its contents. */


#include "rdd_ag_dhd_tx_post.h"

int rdd_ag_dhd_tx_post_wlan_mcast_ssid_stats_table_set(uint32_t _entry, uint32_t packets, uint32_t bytes)
{
    if(_entry >= RDD_WLAN_MCAST_SSID_STATS_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_WLAN_MCAST_SSID_STATS_ENTRY_PACKETS_WRITE_G(packets, RDD_WLAN_MCAST_SSID_STATS_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_SSID_STATS_ENTRY_BYTES_WRITE_G(bytes, RDD_WLAN_MCAST_SSID_STATS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_ssid_stats_table_get(uint32_t _entry, uint32_t *packets, uint32_t *bytes)
{
    if(_entry >= RDD_WLAN_MCAST_SSID_STATS_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_WLAN_MCAST_SSID_STATS_ENTRY_PACKETS_READ_G(*packets, RDD_WLAN_MCAST_SSID_STATS_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_SSID_STATS_ENTRY_BYTES_READ_G(*bytes, RDD_WLAN_MCAST_SSID_STATS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_table_set(uint32_t _entry, uint16_t mac_address_high, uint16_t mac_address_mid, uint16_t mac_address_low, uint8_t radio_index, bdmf_boolean valid, uint8_t tx_priority, uint16_t flowring_index)
{
    if(_entry >= RDD_WLAN_MCAST_DHD_STATION_TABLE_SIZE || radio_index >= 4 || tx_priority >= 8 || flowring_index >= 1024)
          return BDMF_ERR_PARM;

    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_HIGH_WRITE_G(mac_address_high, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_MID_WRITE_G(mac_address_mid, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_LOW_WRITE_G(mac_address_low, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_RADIO_INDEX_WRITE_G(radio_index, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_VALID_WRITE_G(valid, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_TX_PRIORITY_WRITE_G(tx_priority, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_FLOWRING_INDEX_WRITE_G(flowring_index, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_table_get(uint32_t _entry, uint16_t *mac_address_high, uint16_t *mac_address_mid, uint16_t *mac_address_low, uint8_t *radio_index, bdmf_boolean *valid, uint8_t *tx_priority, uint16_t *flowring_index)
{
    if(_entry >= RDD_WLAN_MCAST_DHD_STATION_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_HIGH_READ_G(*mac_address_high, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_MID_READ_G(*mac_address_mid, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_LOW_READ_G(*mac_address_low, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_RADIO_INDEX_READ_G(*radio_index, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_VALID_READ_G(*valid, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_TX_PRIORITY_READ_G(*tx_priority, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_FLOWRING_INDEX_READ_G(*flowring_index, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_ctx_table_set(uint32_t _entry, uint8_t ssid)
{
    if(_entry >= RDD_WLAN_MCAST_DHD_STATION_CTX_TABLE_SIZE || ssid >= 16)
          return BDMF_ERR_PARM;

    RDD_WLAN_MCAST_DHD_STATION_CTX_ENTRY_SSID_WRITE_G(ssid, RDD_WLAN_MCAST_DHD_STATION_CTX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dhd_station_ctx_table_get(uint32_t _entry, uint8_t *ssid)
{
    if(_entry >= RDD_WLAN_MCAST_DHD_STATION_CTX_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_WLAN_MCAST_DHD_STATION_CTX_ENTRY_SSID_READ_G(*ssid, RDD_WLAN_MCAST_DHD_STATION_CTX_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dft_list_size_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_WLAN_MCAST_DFT_LIST_SIZE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_WLAN_MCAST_DFT_LIST_SIZE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dft_list_size_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_WLAN_MCAST_DFT_LIST_SIZE_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_WLAN_MCAST_DFT_LIST_SIZE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dft_addr_set(uint32_t low, uint32_t high)
{
    RDD_PHYS_ADDR_64_PTR_LOW_WRITE_G(low, RDD_WLAN_MCAST_DFT_ADDR_ADDRESS_ARR, 0);
    RDD_PHYS_ADDR_64_PTR_HIGH_WRITE_G(high, RDD_WLAN_MCAST_DFT_ADDR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_dhd_tx_post_wlan_mcast_dft_addr_get(uint32_t *low, uint32_t *high)
{
    RDD_PHYS_ADDR_64_PTR_LOW_READ_G(*low, RDD_WLAN_MCAST_DFT_ADDR_ADDRESS_ARR, 0);
    RDD_PHYS_ADDR_64_PTR_HIGH_READ_G(*high, RDD_WLAN_MCAST_DFT_ADDR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

