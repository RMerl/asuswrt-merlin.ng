/*
  Copyright (c) 2014 Broadcom Corporation
  All Rights Reserved

  <:label-BRCM:2014:DUAL/GPL:standard
    
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

#include "rdd.h"
#include "rdd_wlan_mcast_common.h"

extern wlan_mcast_dhd_list_table_t wlan_mcast_dhd_list_table_g;


/*
 *  Forwarding Table API
 */

static void f_rdd_wlan_mcast_fwd_entry_write(RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                             RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry_ptr)
{
    RDD_WLAN_MCAST_FWD_ENTRY_DHD_LIST_SIZE_WRITE(fwd_entry->dhd_list_size, fwd_entry_ptr);

    RDD_WLAN_MCAST_FWD_ENTRY_IS_PROXY_ENABLED_WRITE(fwd_entry->is_proxy_enabled, fwd_entry_ptr);

    RDD_WLAN_MCAST_FWD_ENTRY_TX_PRIORITY_WRITE(fwd_entry->tx_priority, fwd_entry_ptr);

    RDD_WLAN_MCAST_FWD_ENTRY_WFD_0_PRIORITY_WRITE(fwd_entry->wfd_0_priority, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_1_PRIORITY_WRITE(fwd_entry->wfd_1_priority, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_2_PRIORITY_WRITE(fwd_entry->wfd_2_priority, fwd_entry_ptr);

    RDD_WLAN_MCAST_FWD_ENTRY_WFD_0_SSID_VECTOR_WRITE(fwd_entry->wfd_0_ssid_vector, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_1_SSID_VECTOR_WRITE(fwd_entry->wfd_1_ssid_vector, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_2_SSID_VECTOR_WRITE(fwd_entry->wfd_2_ssid_vector, fwd_entry_ptr);
}

static void f_rdd_wlan_mcast_fwd_entry_read(RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                            RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry_ptr)
{
    RDD_WLAN_MCAST_FWD_ENTRY_DHD_LIST_SIZE_READ(fwd_entry->dhd_list_size, fwd_entry_ptr);


    RDD_WLAN_MCAST_FWD_ENTRY_IS_PROXY_ENABLED_READ(fwd_entry->is_proxy_enabled, fwd_entry_ptr);

    RDD_WLAN_MCAST_FWD_ENTRY_TX_PRIORITY_READ(fwd_entry->tx_priority, fwd_entry_ptr);

    RDD_WLAN_MCAST_FWD_ENTRY_WFD_0_PRIORITY_READ(fwd_entry->wfd_0_priority, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_1_PRIORITY_READ(fwd_entry->wfd_1_priority, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_2_PRIORITY_READ(fwd_entry->wfd_2_priority, fwd_entry_ptr);

    RDD_WLAN_MCAST_FWD_ENTRY_WFD_0_SSID_VECTOR_READ(fwd_entry->wfd_0_ssid_vector, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_1_SSID_VECTOR_READ(fwd_entry->wfd_1_ssid_vector, fwd_entry_ptr);
    RDD_WLAN_MCAST_FWD_ENTRY_WFD_2_SSID_VECTOR_READ(fwd_entry->wfd_2_ssid_vector, fwd_entry_ptr);
}

int rdd_wlan_mcast_fwd_entry_add(RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                 RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS dhd_list_entry,
                                 uint32_t *fwd_entry_index)
{
    RDD_WLAN_MCAST_FWD_TABLE_DTS *fwd_table_ptr = RDD_WLAN_MCAST_FWD_TABLE_PTR();
    RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry_ptr;
    int entry_index;

    for (entry_index = 0; entry_index < RDD_WLAN_MCAST_FWD_TABLE_SIZE; ++entry_index)
    {
        uint32_t valid;

        fwd_entry_ptr = &fwd_table_ptr->entry[entry_index];

        RDD_WLAN_MCAST_FWD_ENTRY_VALID_READ(valid, fwd_entry_ptr);

        if (!valid)
        {
#ifdef CONFIG_DHD_RUNNER
            if (dhd_list_entry.valid)
            {
                uint32_t dhd_list_size = 0;
                int ret;

                ret = rdd_wlan_mcast_dhd_list_entry_add(entry_index, &dhd_list_entry, &dhd_list_size);
                if (ret)
                    return ret;

                fwd_entry->dhd_list_size = dhd_list_size;
            }
#endif

            f_rdd_wlan_mcast_fwd_entry_write(fwd_entry, fwd_entry_ptr);

            RDD_WLAN_MCAST_FWD_ENTRY_VALID_WRITE(1, fwd_entry_ptr);

            *fwd_entry_index = entry_index;

            return BDMF_ERR_OK;
        }
    }

    return BDMF_ERR_NORES;
}

int rdd_wlan_mcast_fwd_entry_delete(uint32_t fwd_entry_index)
{
    RDD_WLAN_MCAST_FWD_TABLE_DTS *fwd_table_ptr = RDD_WLAN_MCAST_FWD_TABLE_PTR();
    RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry_ptr;
    uint32_t valid;

    if (fwd_entry_index >= RDD_WLAN_MCAST_FWD_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    fwd_entry_ptr = &fwd_table_ptr->entry[fwd_entry_index];

    RDD_WLAN_MCAST_FWD_ENTRY_VALID_READ(valid, fwd_entry_ptr);

    if (!valid)
        return BDMF_ERR_NOENT;

    RDD_WLAN_MCAST_FWD_ENTRY_VALID_WRITE(0, fwd_entry_ptr);
#ifdef CONFIG_DHD_RUNNER
    rdd_wlan_mcast_dhd_list_delete(fwd_entry_index);
#endif

    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_fwd_entry_read(uint32_t fwd_entry_index,
                                  RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                  RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS *dhd_list_entry_array,
                                  uint8_t *dhd_station_count)
{
    RDD_WLAN_MCAST_FWD_TABLE_DTS *fwd_table_ptr = RDD_WLAN_MCAST_FWD_TABLE_PTR();
    RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry_ptr;
    uint32_t valid;

    if (fwd_entry_index >= RDD_WLAN_MCAST_FWD_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    fwd_entry_ptr = &fwd_table_ptr->entry[fwd_entry_index];

    RDD_WLAN_MCAST_FWD_ENTRY_VALID_READ(valid, fwd_entry_ptr);
    if (!valid)
        return BDMF_ERR_NOENT;

    f_rdd_wlan_mcast_fwd_entry_read(fwd_entry, fwd_entry_ptr);
#ifdef CONFIG_DHD_RUNNER
    *dhd_station_count = rdd_wlan_mcast_dhd_list_scan(fwd_entry_index, dhd_list_entry_array);
#else
    *dhd_station_count = 0;
#endif

    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_fwd_entry_write(uint32_t fwd_entry_index,
                                   RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                   RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS dhd_list_entry,
                                   uint32_t dhd_list_add)
{
    RDD_WLAN_MCAST_FWD_TABLE_DTS *fwd_table_ptr = RDD_WLAN_MCAST_FWD_TABLE_PTR();
    RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry_ptr;
    uint32_t valid;

    if (fwd_entry_index >= RDD_WLAN_MCAST_FWD_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    fwd_entry_ptr = &fwd_table_ptr->entry[fwd_entry_index];

    RDD_WLAN_MCAST_FWD_ENTRY_VALID_READ(valid, fwd_entry_ptr);

    if (!valid)
        return BDMF_ERR_NOENT;

#ifdef CONFIG_DHD_RUNNER
    if (dhd_list_entry.valid)
    {
        uint32_t dhd_list_size = fwd_entry->dhd_list_size;
        int ret;

        if (1 == dhd_list_add)
        {
            __debug("Add DHD Station\n");

            ret = rdd_wlan_mcast_dhd_list_entry_find(fwd_entry_index, &dhd_list_entry);
            if (ret == BDMF_ERR_OK)
                return BDMF_ERR_ALREADY;

            ret = rdd_wlan_mcast_dhd_list_entry_add(fwd_entry_index, &dhd_list_entry, &dhd_list_size);
            if (ret)
                return ret;
        }
        else if (2 == dhd_list_add)
        {
            /* just test if the requested dhd_station already exists in requested fwd_table,
                    ** never change fwd_table.
                    */
            __debug("Test presence\n");

            ret = rdd_wlan_mcast_dhd_list_entry_find(fwd_entry_index, &dhd_list_entry);
            if (ret == BDMF_ERR_OK)
                return BDMF_ERR_ALREADY; /* Already present */
            else
                return BDMF_ERR_NOENT; /* Not present */
        }
        else
        {
            __debug("Delete DHD Station\n");

            ret = rdd_wlan_mcast_dhd_list_entry_delete(fwd_entry_index, &dhd_list_entry, &dhd_list_size);
            if (ret)
                return ret;
        }

        fwd_entry->dhd_list_size = dhd_list_size;
    }
#endif

    f_rdd_wlan_mcast_fwd_entry_write(fwd_entry, fwd_entry_ptr);

    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_fwd_entry_dhd_station_count_get(uint32_t fwd_entry_index,
                                                   uint8_t *dhd_station_count)
{
    RDD_WLAN_MCAST_FWD_TABLE_DTS *fwd_table_ptr = RDD_WLAN_MCAST_FWD_TABLE_PTR();
    RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry_ptr;
    uint32_t valid;

    if (fwd_entry_index >= RDD_WLAN_MCAST_FWD_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    fwd_entry_ptr = &fwd_table_ptr->entry[fwd_entry_index];

    RDD_WLAN_MCAST_FWD_ENTRY_VALID_READ(valid, fwd_entry_ptr);

    if (!valid)
        return BDMF_ERR_NOENT;

#ifdef CONFIG_DHD_RUNNER
    *dhd_station_count = rdd_wlan_mcast_dhd_list_scan(fwd_entry_index, NULL);
#else
    *dhd_station_count = 0;
#endif

    return BDMF_ERR_OK;
}


#ifdef CONFIG_DHD_RUNNER
/*
 *  DHD Station Table API
 */

static void f_rdd_wlan_mcast_dhd_station_read(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry,
                                              RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry_ptr)
{
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_HIGH_READ(dhd_station_entry->mac_address_high, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_MID_READ(dhd_station_entry->mac_address_mid, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_LOW_READ(dhd_station_entry->mac_address_low, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_SSID_READ(dhd_station_entry->ssid, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_RADIO_INDEX_READ(dhd_station_entry->radio_index, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_FLOWRING_INDEX_READ(dhd_station_entry->flowring_index, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_TX_PRIORITY_READ(dhd_station_entry->tx_priority, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_READ(dhd_station_entry->reference_count, dhd_station_entry_ptr);
}

static int f_rdd_wlan_mcast_dhd_station_write(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry,
                                              RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry_ptr)
{
    if (dhd_station_entry->radio_index > 2)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_HIGH_WRITE(dhd_station_entry->mac_address_high, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_MID_WRITE(dhd_station_entry->mac_address_mid, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_MAC_ADDRESS_LOW_WRITE(dhd_station_entry->mac_address_low, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_SSID_WRITE(dhd_station_entry->ssid, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_RADIO_INDEX_WRITE(dhd_station_entry->radio_index, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_FLOWRING_INDEX_WRITE(dhd_station_entry->flowring_index, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_TX_PRIORITY_WRITE(dhd_station_entry->tx_priority, dhd_station_entry_ptr);
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_WRITE(dhd_station_entry->reference_count, dhd_station_entry_ptr);

    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_dhd_station_read(uint32_t dhd_station_index,
                                    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry)
{
    RDD_WLAN_MCAST_DHD_STATION_TABLE_DTS *dhd_station_table = RDD_WLAN_MCAST_DHD_STATION_TABLE_PTR();
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry_ptr = &dhd_station_table->entry[dhd_station_index];
    uint32_t reference_count;

    if (dhd_station_index >= RDD_WLAN_MCAST_DHD_STATION_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_READ(reference_count, dhd_station_entry_ptr);

    if (!reference_count)
        return BDMF_ERR_NOENT;

    f_rdd_wlan_mcast_dhd_station_read(dhd_station_entry, dhd_station_entry_ptr);

    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_dhd_station_find(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry,
                                    uint32_t *dhd_station_index)
{
    RDD_WLAN_MCAST_DHD_STATION_TABLE_DTS *dhd_station_table = RDD_WLAN_MCAST_DHD_STATION_TABLE_PTR();
    int match_count;
    int i;

    for (i = 0; i < RDD_WLAN_MCAST_DHD_STATION_TABLE_SIZE; ++i)
    {
        RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry_ptr = &dhd_station_table->entry[i];
        RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS dhd_station_entry_search;

        f_rdd_wlan_mcast_dhd_station_read(&dhd_station_entry_search, dhd_station_entry_ptr);

        if (dhd_station_entry_search.reference_count && /* ref count != 0 means entry is in use */
            dhd_station_entry_search.mac_address_high == dhd_station_entry->mac_address_high &&
            dhd_station_entry_search.mac_address_low == dhd_station_entry->mac_address_low)
        {
            match_count = 0;
            match_count += (dhd_station_entry_search.ssid == dhd_station_entry->ssid) ? 1 : 0;
            match_count += (dhd_station_entry_search.radio_index == dhd_station_entry->radio_index) ? 1 : 0;
            match_count += (dhd_station_entry_search.flowring_index == dhd_station_entry->flowring_index) ? 1 : 0;
            /* For now ignoring tx_priority - no point in diffentiating streams based on priority for same STA
            * Moreover, it priority is different, flow-ring would have been different as well (most of the time) */

            if (match_count == 3) /* All above conditions matched i.e. perfect match */
            {
                *dhd_station_index = i;
                return BDMF_ERR_OK;
            }
            else /* One of the above condition did not match i.e. STA connection info changed but we have stale information still in use*/
            {
                __print("NOTICE : Possibility of stale STA <0x%08x%04x> entry : <%u:%u> <%u:%u> <%u:%u>\n",
                        dhd_station_entry->mac_address_high, dhd_station_entry->mac_address_low,
                        dhd_station_entry_search.ssid, dhd_station_entry->ssid,
                        dhd_station_entry_search.radio_index, dhd_station_entry->radio_index,
                        dhd_station_entry_search.flowring_index, dhd_station_entry->flowring_index);
            }
        }
    }

    return BDMF_ERR_NOENT;
}

int rdd_wlan_mcast_dhd_station_add(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry,
                                   uint32_t *dhd_station_index)
{
    RDD_WLAN_MCAST_DHD_STATION_TABLE_DTS *dhd_station_table = RDD_WLAN_MCAST_DHD_STATION_TABLE_PTR();
    int ret;

    ret = rdd_wlan_mcast_dhd_station_find(dhd_station_entry, dhd_station_index);
    if (ret)
    {
        uint32_t reference_count;
        int i;

        /* DHD Station does not exist, add a new table entry */

        for (i = 0; i < RDD_WLAN_MCAST_DHD_STATION_TABLE_SIZE; ++i)
        {
            RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry_ptr = &dhd_station_table->entry[i];

            RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_READ(reference_count, dhd_station_entry_ptr);

            if (!reference_count)
            {
                dhd_station_entry->reference_count = 1;

                ret = f_rdd_wlan_mcast_dhd_station_write(dhd_station_entry, dhd_station_entry_ptr);
                if (ret)
                    return ret;

                *dhd_station_index = i;

                return BDMF_ERR_OK;
            }
        }
    }
    else
    {
        RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry_ptr = &dhd_station_table->entry[*dhd_station_index];

        /* DHD Station already exists, increment the referece counter */

        RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_READ(dhd_station_entry->reference_count, dhd_station_entry_ptr);

        dhd_station_entry->reference_count++;

        RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_WRITE(dhd_station_entry->reference_count, dhd_station_entry_ptr);

        return BDMF_ERR_OK;
    }

    return BDMF_ERR_NORES;
}

int rdd_wlan_mcast_dhd_station_delete(uint32_t dhd_station_index)
{
    RDD_WLAN_MCAST_DHD_STATION_TABLE_DTS *dhd_station_table = RDD_WLAN_MCAST_DHD_STATION_TABLE_PTR();
    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry_ptr = &dhd_station_table->entry[dhd_station_index];
    uint32_t reference_count;

    if (dhd_station_index >= RDD_WLAN_MCAST_DHD_STATION_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_READ(reference_count, dhd_station_entry_ptr);

    if (!reference_count)
        return BDMF_ERR_NOENT;

    reference_count--;

    RDD_WLAN_MCAST_DHD_STATION_ENTRY_REFERENCE_COUNT_WRITE(reference_count, dhd_station_entry_ptr);

    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_ssid_mac_address_add(uint32_t radio_index, uint32_t ssid,
                                        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry,
                                        uint32_t *ssid_mac_address_index)
{
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_DTS *ssid_mac_address_table_ptr = RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_PTR();
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry_ptr;

    *ssid_mac_address_index = RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_INDEX(radio_index, ssid);

    if (*ssid_mac_address_index >= RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    ssid_mac_address_entry_ptr = &ssid_mac_address_table_ptr->entry[*ssid_mac_address_index];

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_REFERENCE_COUNT_READ(ssid_mac_address_entry->reference_count,
                                                               ssid_mac_address_entry_ptr);
    ssid_mac_address_entry->reference_count++;

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_REFERENCE_COUNT_WRITE(ssid_mac_address_entry->reference_count,
                                                                ssid_mac_address_entry_ptr);

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_HIGH_WRITE(ssid_mac_address_entry->mac_address_high,
                                                                 ssid_mac_address_entry_ptr);

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_LOW_WRITE(ssid_mac_address_entry->mac_address_low,
                                                                ssid_mac_address_entry_ptr);
    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_ssid_mac_address_delete(uint32_t ssid_mac_address_index)
{
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_DTS *ssid_mac_address_table_ptr = RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_PTR();
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry_ptr =
        &ssid_mac_address_table_ptr->entry[ssid_mac_address_index];
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS ssid_mac_address_entry;

    if (ssid_mac_address_index >= RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_REFERENCE_COUNT_READ(ssid_mac_address_entry.reference_count,
                                                               ssid_mac_address_entry_ptr);
    if (!ssid_mac_address_entry.reference_count)
        return BDMF_ERR_NOENT;

    ssid_mac_address_entry.reference_count--;

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_REFERENCE_COUNT_WRITE(ssid_mac_address_entry.reference_count,
                                                                ssid_mac_address_entry_ptr);

    if (!ssid_mac_address_entry.reference_count)
    {
        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_HIGH_WRITE(0, ssid_mac_address_entry_ptr);

        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_LOW_WRITE(0, ssid_mac_address_entry_ptr);
    }

    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_ssid_mac_address_read(uint32_t ssid_mac_address_index,
                                         RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry)
{
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_DTS *ssid_mac_address_table_ptr = RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_PTR();
    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry_ptr =
        &ssid_mac_address_table_ptr->entry[ssid_mac_address_index];

    if (ssid_mac_address_index >= RDD_WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_REFERENCE_COUNT_READ(ssid_mac_address_entry->reference_count,
                                                               ssid_mac_address_entry_ptr);
    if (!ssid_mac_address_entry->reference_count)
        return BDMF_ERR_NOENT;

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_HIGH_READ(ssid_mac_address_entry->mac_address_high,
                                                                ssid_mac_address_entry_ptr);

    RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_MAC_ADDRESS_LOW_READ(ssid_mac_address_entry->mac_address_low,
                                                               ssid_mac_address_entry_ptr);
    return BDMF_ERR_OK;
}

int rdd_wlan_mcast_ssid_stats_read(uint32_t ssid_stats_index,
                                   RDD_WLAN_MCAST_SSID_STATS_ENTRY_DTS *ssid_stats_entry)
{
    RDD_WLAN_MCAST_SSID_STATS_TABLE_DTS *ssid_stats_table_ptr = RDD_WLAN_MCAST_SSID_STATS_TABLE_PTR();
    RDD_WLAN_MCAST_SSID_STATS_ENTRY_DTS *ssid_stats_entry_ptr =
        &ssid_stats_table_ptr->entry[ssid_stats_index];

    if (ssid_stats_index >= RDD_WLAN_MCAST_SSID_STATS_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    RDD_WLAN_MCAST_SSID_STATS_ENTRY_PACKETS_READ(ssid_stats_entry->packets, ssid_stats_entry_ptr);
    if (!ssid_stats_entry->packets)
        return BDMF_ERR_NOENT;

    RDD_WLAN_MCAST_SSID_STATS_ENTRY_BYTES_READ(ssid_stats_entry->bytes, ssid_stats_entry_ptr);

    return BDMF_ERR_OK;
}
#endif /* CONFIG_DHD_RUNNER */

/*
 *  Initialization API
 */

int rdd_wlan_mcast_init(wlan_mcast_dhd_list_table_t *table)
{
    RDD_WLAN_MCAST_CONTROL_ENTRY_DTS *control_ptr = (RDD_WLAN_MCAST_CONTROL_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + WLAN_MCAST_CONTROL_TABLE_ADDRESS);

    rdd_wlan_mcast_init_common(table);
   
    __debug("\twlan_mcast_dhd_list_table_g: virt_p %p, phys_addr 0x%08x\n\n",
            wlan_mcast_dhd_list_table_g.virt_p, wlan_mcast_dhd_list_table_g.phys_addr);

    /* Configure Runner */
    RDD_WLAN_MCAST_CONTROL_ENTRY_DHD_LIST_BASE_ADDRESS_WRITE(wlan_mcast_dhd_list_table_g.phys_addr, control_ptr);

    RDD_WLAN_MCAST_CONTROL_ENTRY_INGRESS_QUEUE_WRITE_PTR_WRITE(WLAN_MCAST_INGRESS_QUEUE_ADDRESS, control_ptr);
    
#ifdef OREN
    RDD_WLAN_MCAST_CONTROL_ENTRY_INGRESS_QUEUE_PD_WRITE_PTR_WRITE(WLAN_MCAST_PD_INGRESS_QUEUE_ADDRESS, control_ptr);
    RDD_WLAN_MCAST_CONTROL_ENTRY_INGRESS_QUEUE_PD_READ_PTR_WRITE(WLAN_MCAST_PD_INGRESS_QUEUE_ADDRESS, control_ptr);
#endif    
    return BDMF_ERR_OK;
}

