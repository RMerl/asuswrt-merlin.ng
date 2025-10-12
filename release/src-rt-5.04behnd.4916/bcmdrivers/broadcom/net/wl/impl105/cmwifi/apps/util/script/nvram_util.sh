#!/bin/sh

# This utility is used to generate loadlabe nvram text file based on given tempates

NVRAM_DEFAULT_AP_TEMP="/tmp/nvram_default_ap_temp.txt"
NVRAM_DEFAULT_RADIO_TEMP="/tmp/nvram_default_radio_temp.txt"
NVRAM_DEFAULT_APPS_TEMP="/tmp/nvram_default_apps_temp.txt"
lanXifnames=""
vifs=""
num_radios=""

# query radio's cap and construct virtual interface string
# input: radio index starting with 0
# return: radio's possible virtual interfaces "wl0.1 wl0.2 ...."
get_radio_vifs() {
	_radio_idx=$1
	_mbss_num=$(wl -i wl$_radio_idx cap |sed -n 's/.*mbss\([0-9]\+\).*/\1/p')
	if [[ -n $_mbss_num  && $_mbss_num -gt 1 ]]; then
		[[ $_mbss_num -gt $MAX_NUM_BSS_PER_RADIO ]] && _mbss_num=$MAX_NUM_BSS_PER_RADIO
		for i in `seq 1  $(($_mbss_num-1))`;do
			echo -n "wl$_radio_idx.$i "
		done
	fi
}
# input: apInex, apTempateFile
# output: default nvram file for AP.
#         default nvram value for these entries are identifical to all AP
#         replace wlX.Y with apName based on apIndex
get_common_nvram_by_ap_index()
{
	apIndex=$1
	apTemplateFile=$2
	apName=$(get_ap_name_by_index $apIndex)
	grep "wlX.Y_" ${apTemplateFile} | sed "s/wlX.Y/${apName}/g" >> ${NVRAM_DEFAULT_AP_TEMP}
}

# input: apIndex, apTemplateFile
# outpute: default nvram file for AP.
#          default nvram value for these entried are unique teach AP
get_unique_nvram_by_ap_index()
{
	apIndex=$1
	apTemplateFile=$2
	apName=$(get_ap_name_by_index $apIndex)

	grep "${apName}_" ${apTemplateFile} >> ${NVRAM_DEFAULT_AP_TEMP}

	# handle 6g band unique settings
	band=$(get_band_from_apIndex $apIndex)
	if [ "${band}" == "6g" ]; then
		sed -i "s/${apName}_akm=.*/${apName}_akm=sae/g" ${NVRAM_DEFAULT_AP_TEMP}
		sed -i "s/${apName}_mfp=.*/${apName}_mfp=2/g" ${NVRAM_DEFAULT_AP_TEMP}
		sed -i "s/${apName}_wps_mode=.*/${apName}_wps_mode=disabled/g" ${NVRAM_DEFAULT_AP_TEMP}
	fi
}

# input: apIndex, nvram entry name
# output: nvram value already been loaded
get_nvram_value_by_ap_index()
{
	apIndex=$1
	nvName=$2
	apName=$(get_ap_name_by_index $apIndex)
	value=`nvram get ${apName}_${nvName}`
	echo $value
}

# input: apIndex
# output: default nvram file for AP
#         obtain default wlX.Y_bss_enabled entry based on lanX_ifnames
#         except for primary APs, by default, wlX.Y_bss_enabled is set to 0 unless this AP is also assigned to  bridget group.
get_bss_enable_by_ap_index()
{
	apIndex=$1
	apName=$(get_ap_name_by_index $apIndex)
	radioName=$(get_radio_name_by_ap_index $apIndex)
	is_primary=$(is_ap_primary $apIndex)
	is_set_in_vifs=0
	is_set_in_lanx_ifnames=0

	[ "${lanXifnames}" == "" ] && lanXifnames=$(getlanXifnames)

	[ "${vifs}" == "" ] && vifs=`nvram get ${radioName}_vifs`

	for name in ${vifs}
	do
		if [ "${name}" == ${apName} ]; then
			is_set_in_vifs=1
			for name in ${lanXifnames}
			do
				if [ ${name} == ${apName} ]; then
					is_set_in_lanx_ifnames=1
					break
				fi
			done
		fi
	done
	if  [[ ${is_set_in_vifs} -eq 1 ]] && [[ ${is_set_in_lanx_ifnames} -eq 1 ]] || [[ ${is_primary} == 1 ]]; then
		echo "${apName}_bss_enabled=1" >> ${NVRAM_DEFAULT_AP_TEMP}
	else
		if [[ $BCACPE_RDKB_BUILD -ne 1 ]]; then
			#for BCACPE_RDK_BUILD, wlmdm has default bss_enabled, do not disable it if it is already enabled in wlmdm
			echo "${apName}_bss_enabled=0" >> ${NVRAM_DEFAULT_AP_TEMP}
		fi
	fi
}

# input: apIndex, apTemplateFile
# output: default nvram file for AP per bss
#         combiing all the nvram for each bss
nvram_per_bss()
{
	get_common_nvram_by_ap_index $@
	get_unique_nvram_by_ap_index $@
	get_bss_enable_by_ap_index $1
}

# input: radioIndex, radioTemplateFile
# output: default nvram file for Radio.
#         default nvram value for these entries are identifical to all Radios
#         replace wlX with radioName converted from radioIndex
get_common_nvram_by_radio_index()
{
	radioIndex=$1
	radioTemplateFile=$2
	radioName=$(get_radio_name_by_index ${radioIndex})
	grep "wlX_" ${radioTemplateFile} | sed "s/wlX/${radioName}/g"  >> ${NVRAM_DEFAULT_RADIO_TEMP}
}

# input: radioIndex, radioTemplateFile
# output: default nvram file for Radio.
#         default nvram value for these entries are unique for each Radios.
#         replace wlX with radioName converted from radioIndex.
get_unique_nvram_by_radio_index()
{
	radioIndex=$1
	radioTemplateFile=$2
	radioName=$(get_radio_name_by_index ${radioIndex})
	grep "${radioName}_" ${radioTemplateFile} >> ${NVRAM_DEFAULT_RADIO_TEMP}
}

# input: radioIndex, radioTempateFile
# output: default nvram file for Radio per radio
#         combining all the nvrams for each radio
nvram_per_radio()
{
	get_common_nvram_by_radio_index $@
	get_unique_nvram_by_radio_index $@
}

# input: radioIndex, radioTemplateFile, nvram entry
# output: nvram value already loaded
get_nvram_value_by_radio_index()
{
	radioIndex=$1
	nvName=$2
	radioName=$(get_radio_name_by_index ${radioIndex})
	value=`nvram get ${radioName}_${nvName}`
	echo $value
}

# input: total number of wifi radios on the platform
# output: load nvram file for applications
reset_apps_nvram()
{
	num_radios=$1
	appsTemplateFile=$2

	brname=$(get_bridge_name)
	wlX_brinfo=${brname}
	for i in $(seq 0 $((num_radios-1)))
	do
		wlX_brinfo="${wlX_brinfo}:wl${i}"
		[ ${i} == 0 ] && ifnames="wl${i}" || ifnames="${ifnames} wl${i}"
	done

	cp ${appsTemplateFile} ${NVRAM_DEFAULT_APPS_TEMP}
	sed -i "s/_BRINFO_/${wlX_brinfo}/g" ${NVRAM_DEFAULT_APPS_TEMP}
	sed -i "s/_IFNAMES_/${ifnames}/g" ${NVRAM_DEFAULT_APPS_TEMP}
	sed -i "s/_BRNAME_/${brname}/g" ${NVRAM_DEFAULT_APPS_TEMP}

	nvram load -t ${NVRAM_DEFAULT_APPS_TEMP}
	rm -f NVRAM_DEFAULT_APPS_TEMP 2> /dev/null
}

# restore nvram for ap by ap indices
reset_ap_nvram_by_index()
{
	print_to_wifi_log "<$FUNCNAME> $@"

	apIndices=$@
	templateFile=${AP_DEFAULT_FILE}

	rm -f ${NVRAM_DEFAULT_AP_TEMP} 2> /dev/null
	for idx in ${apIndices}
	do
		nvram_per_bss ${idx} ${templateFile}
	done
	nvram load -t ${NVRAM_DEFAULT_AP_TEMP}
	rc=$?
	rm -f ${NVRAM_DEFAULT_AP_TEMP} 2> /dev/null
	print_to_wifi_log "restore nvram from ${templateFile} for apIndices ${apIndices}, rc=$rc"
}

# restore nvram for ap by ap interface names
reset_ap_nvram_by_names()
{
	print_to_wifi_log "<$FUNCNAME> $@"
	apNames=$@
	apIndices=""

	for name in ${apNames}
	do
		apIndices="${apIndices} $(get_ap_index_by_name ${name})"
	done
	reset_ap_nvram_by_index ${apIndices}
	print_to_wifi_log "<$FUNCNAME> for AP ${apNames} by indices ${apIndices}"
}

# restore nvram for single or multiple radios by radio indices
reset_radio_nvram_by_index()
{
	print_to_wifi_log "<$FUNCNAME> $@"

	radioIndices=$@
	templateFile=${RADIO_DEFAULT_FILE}

	rm -f ${NVRAM_DEFAULT_RADIO_TEMP} 2> /dev/null
	for idx in ${radioIndices}
	do
		nvram_per_radio ${idx} ${templateFile}
	done
	nvram load -t ${NVRAM_DEFAULT_RADIO_TEMP}
	rc=$?
	rm -f ${NVRAM_DEFAULT_RADIO_TEMP} 2> /dev/null
	print_to_wifi_log "restore nvram from ${templateFile} for radioIndices ${radioIndices}, rc=$rc"
}

# restore nvram for radio by interface name
reset_radio_nvram_by_names()
{
	print_to_wifi_log "<$FUNCNAME> $@"
	radioNames=$@
	radioIndices=""

	for name in ${radioNames}
	do
		radioIndices="$(get_radio_index_by_name ${name}) ${radioIndices}"
	done
	reset_radio_nvram_by_index ${radioIndices}
	print_to_wifi_log "<$FUNCNAME> for Radio Index ${radioIndices}"
}

# get number of wifi radio detected from pcie bus
get_number_of_radios_detected()
{
	print_to_wifi_log "<$FUNCNAME> $@"
	num=`lspci |grep "0280" |wc -l`
	if [ "${num}" == "0" ]; then
		num=${MAX_NUM_RADIOS}
	fi

	print_to_wifi_log "<$FUNCNAME> found ${num} radios"
	echo ${num}
}

# restore nvram for applications to factory default
factory_reset_apps()
{
	[ "${num_radios}" == "" ] && num_radios=$(get_wl_instances ${WIFI_IFNAMES})
	print_to_wifi_log "<$FUNCNAME> num_radios ${num_radios}"
	reset_apps_nvram ${num_radios} ${APPS_DEFAULT_FILE}
	print_to_wifi_log "Done factory restore nvram for apps!!"
}

# restore nvram for all bss to factory default
factory_reset_all_aps()
{
	[ "${num_radios}" == "" ] && num_radios=$(get_wl_instances ${WIFI_IFNAMES})
	print_to_wifi_log "<$FUNCNAME> num_radios ${num_radios}"
	for idx in $(seq 0 $((num_radios - 1)))
	do
		if [[ $BCACPE_RDKB_BUILD -eq 1 ]]; then
			vifs="$(get_radio_vifs $idx)"
		else
			vifs=`nvram get wl${idx}_vifs`
		fi
		primary="wl${idx}"
		reset_ap_nvram_by_names ${primary} ${vifs}
		print_to_wifi_log "reset_ap_nvram_by_names ${primary} ${vifs}"
	done
	print_to_wifi_log "Done factory restore nvram for all APs!!"
}

# restore nvram for all radios to factory default
factory_reset_all_radios()
{
	[ "${num_radios}" == "" ] && num_radios=$(get_wl_instances ${WIFI_IFNAMES})
	print_to_wifi_log "<$FUNCNAME> num_radios ${num_radios}"
	radios=""
	for idx in $(seq 0 $((num_radios - 1)))
	do
		radios="${radios} wl${idx}"
	done
	reset_radio_nvram_by_names ${radios}
	print_to_wifi_log "Done facotry restore nvram for all Radios ${radios}!!"
}
