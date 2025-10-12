#!/bin/sh
. $(dirname "$0")/nvram_util.sh
. $(dirname "$0")/wifi_util.sh

NVRAM_DEFAULT_AP_TEMPLATE="nvram_default_ap.txt"
NVRAM_DEFAULT_RADIO_TEMPLATE="nvram_default_radio.txt"
APPS_DEFAULT_FILE="nvram_default_apps.txt"

TEMPFILE="nv_tmp.txt"
MAX_NUM_RADIOS=3
MAX_NUM_BSS_PER_RADIO=8
MAX_APS=$((MAX_NUM_RADIOS * MAX_NUM_BSS_PER_RADIO))

nvram_ap_all()
{
	for index in $(seq 0 $((MAX_APS -1))); do
		nvram_per_bss $index ${NVRAM_DEFAULT_AP_TEMPLATE}
	done
}

nvram_radio_all() {
	for index in $(seq 0 $((MAX_NUM_RADIOS - 1))); do
		nvram_per_radio $index ${NVRAM_DEFAULT_RADIO_TEMPLATE}
	done
}

# for debug or review generated default nvram for all radios and aps, uncomment the lines below
rm -f "${NVRAM_DEFAULT_AP_TEMP}"
rm -f "${NVRAM_DEFAULT_RADIO_TEMP}"

factory_reset_apps
factory_reset_all_aps
factory_reset_all_radios

#nvram_radio_all
#nvram_ap_all
#reset_apps_nvram 2 ${APPS_DEFAULT_FILE}

#nvram_per_radio 1 ${NVRAM_DEFAULT_RADIO_TEMPLATE}
#echo $(get_ap_index_by_name "wl2.1")
#echo $(get_ap_index_by_name "wl0")
#echo $(get_ap_index_by_name "wl1.3")

#echo $(get_radio_index_by_name "w0.1")
#echo $(get_radio_index_by_name "w1.3")
#echo $(get_radio_index_by_name "w2.7")
