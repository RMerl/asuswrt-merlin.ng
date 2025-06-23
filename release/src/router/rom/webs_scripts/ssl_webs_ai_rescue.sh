#!/bin/sh

echo "---- Start RESET TO DEFAULT/RESCUE AISOM Process ----" > /ai/log/webs_ai_reset.log
logger -t AI_RESET "Start RESET TO DEFAULT/RESCUE AISOM Process"

# Initialize

ai_reset_beta_support=`nvram get rc_support | grep -i ai_reset_beta`

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

# Because of reset to default/rescue is also like upgrade
# Thus, use same nvram to store the status
nvram set webs_state_ai_upgrade=0
nvram set webs_state_ai_error=0

urlpath=`nvram get webs_state_url`
update_url=`cat /tmp/update_url`
ai_productid=`nvram get ai_productid`

fw_path="/ai/firmware/aisom_reset.swu"
fw_rsa_path="/tmp/aisom_reset_rsasign.bin"
ha_path="/ai/docker_images/homeassistant.tar"
ha_rsa_path="/tmp/ha_rsasign.bin"
frigate_path="/ai/docker_images/frigate.tar"
frigate_rsa_path="/tmp/frigate_rsasign.bin"

#kill old files
rm -f $fw_path
rm -f $fw_rsa_path
rm -f $ha_path
rm -f $ha_rsa_path
rm -f $frigate_path
rm -f $frigate_rsa_path

# for beta path
forbeta=0
if [ "$ai_reset_beta_support" != "" ]; then
	forbeta=`nvram get webs_update_beta`
fi

# for sq
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

urlpath=`nvram get webs_state_url`

wget_fw_result=0
wget_fw_rsa_result=0
wget_ha_result=0
wget_ha_rsa_result=0
wget_frigate_result=0
wget_frigate_rsa_result=0


##### Step 1 #####
# query ai board current version
rc rc_service "start_ai_request query"

sleep 10

cur_fw_version=`nvram get ai_sys_fw_version | tr '.' '_'`
cur_fw_commit_number=`nvram get ai_sys_fw_commit_number`
cur_fw_commit_hash=`nvram get ai_sys_fw_commit_hash`
cur_sdk_version=`nvram get ai_sys_sdk_version | tr '.' '_'`
cur_sdk_commit_hash=`nvram get ai_sys_sdk_commit_hash`

if [ -z "$cur_fw_version" ] || [ -z "$cur_fw_commit_number" ] || [ -z "$cur_fw_commit_hash" ] || [ -z "$cur_sdk_version" ] || [ -z "$cur_sdk_commit_hash" ]; then
	echo "---- Some information of AISOM are missing ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "Some information of AISOM are missing"
	nvram set webs_state_ai_error=11
	exit 0
fi

cur_state_info=${cur_fw_version}_${cur_fw_commit_number}-g${cur_fw_commit_hash}_${cur_sdk_version}-g${cur_sdk_commit_hash}
#ai_firmware_file=SL1680_1_0_2_51-g264fc65_1_5_0-gec2d4cb.zip
#ai_firmware_rsasign=SL1680_1_0_2_51-g264fc65_1_5_0-gec2d4cb_rsa.zip

fw_file=${ai_productid}_${cur_state_info}.zip
fw_rsa_file=${ai_productid}_${cur_state_info}_rsa.zip
ha_file=${ai_productid}_ha_${cur_state_info}.zip
ha_rsa_file=${ai_productid}_ha_${cur_state_info}_rsa.zip
frigate_file=${ai_productid}_frigate_${cur_state_info}.zip
frigate_rsa_file=${ai_productid}_frigate_${cur_state_info}_rsa.zip

echo "---- Succeed in fetching AISOM current information ${cur_state_info} ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "Succeed in fetching AISOM current information ${cur_state_info}"

##### Step 2 #####

if [ "$update_url" != "" ]; then
	dl_url=${update_url}
elif [ "$ai_reset_beta_support" != "" ] && [ "$forbeta" == "1" ]; then
	dl_url=${dl_path_SQ}
elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	dl_url=${dl_path_SQ_beta}${forsq}
elif [ "$forsq" == "1" ]; then
	dl_url=${dl_path_SQ}
elif [ "$urlpath" == "" ]; then
	dl_url=${dl_path_file}
else
	dl_url=${urlpath}
fi

# download reset fw
echo "---- wget reset fw ${dl_url}/$fw_file ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget reset fw $fw_file"
wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate --output-file=/tmp/ai_rst_fw_get_log ${dl_url}/$fw_file -O $fw_path
wget_fw_result=$?
echo "---- wget fw, exit code: ${wget_fw_result} ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget fw exit code: ${wget_fw_result}"

echo "---- wget fw rsa ${dl_url}/$fw_rsa_file ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget fw rsa $fw_rsa_file"
wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate ${dl_url}/$fw_rsa_file -O $fw_rsa_path
wget_fw_rsa_result=$?
echo "---- wget fw rsa, exit code: ${wget_fw_rsa_result} ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget fw rsa exit code: ${wget_fw_rsa_result}"

# download ha
echo "---- wget ha ${dl_url}/$ha_file ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget ha $ha_file"
wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate --output-file=/tmp/ai_rst_ha_get_log ${dl_url}/$ha_file -O $ha_path
wget_ha_result=$?
echo "---- wget ha, exit code: ${wget_ha_result} ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget ha exit code: ${wget_ha_result}"

echo "---- wget ha rsa ${dl_url}/$ha_rsa_file ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget ha rsa $ha_rsa_file"
wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate ${dl_url}/$ha_rsa_file -O $ha_rsa_path
wget_ha_rsa_result=$?
echo "---- wget ha rsa, exit code: ${wget_ha_rsa_result} ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget ha rsa exit code: ${wget_ha_rsa_result}"

# download frigate
echo "---- wget frigate ${dl_url}/$frigate_file ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget frigate $frigate_file"
wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate --output-file=/tmp/ai_rst_frigate_get_log ${dl_url}/$frigate_file -O $frigate_path
wget_frigate_result=$?
echo "---- wget frigate, exit code: ${wget_frigate_result} ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget frigate exit code: ${wget_frigate_result}"

echo "---- wget frigate rsa ${dl_url}/$frigate_rsa_file ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget frigate rsa $frigate_rsa_file"
wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate ${dl_url}/$frigate_rsa_file -O $frigate_rsa_path
wget_frigate_rsa_result=$?
echo "---- wget frigate rsa, exit code: ${wget_frigate_rsa_result} ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "wget frigate rsa exit code: ${wget_frigate_rsa_result}"


if [ "$wget_fw_result" != "0" ]; then
	echo "---- download fw failure ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "download fw failure"
	rm -f $fw_path
	nvram set webs_state_ai_error=12
	sleep 1
fi
if [ "$wget_fw_rsa_result" != "0" ]; then
	echo "---- download fw rsa failure ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "download fw rsa failure"
	rm -f $fw_rsa_path
	nvram set webs_state_ai_error=13
	sleep 1
fi
if [ "$wget_ha_result" != "0" ]; then
	echo "---- download ha failure ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "download ha failure"
	rm -f $ha_path
	nvram set webs_state_ai_error=14
	sleep 1
fi
if [ "$wget_ha_rsa_result" != "0" ]; then
	echo "---- download ha rsa failure ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "download ha rsa failure"
	rm -f $ha_rsa_path
	nvram set webs_state_ai_error=15
	sleep 1
fi
if [ "$wget_frigate_result" != "0" ]; then
	echo "---- download frigate failure ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "download frigate failure"
	rm -f $frigate_path
	nvram set webs_state_ai_error=16
	sleep 1
fi
if [ "$wget_frigate_rsa_result" != "0" ]; then
	echo "---- download frigate rsa failure ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "download frigate rsa failure"
	rm -f $frigate_rsa_path
	nvram set webs_state_ai_error=17
	sleep 1
fi

if [ "$(nvram get webs_state_ai_error)" != "0" ];then
	echo "---- Download AI RESET FW or HA or Frigate Fail ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "Download AI RESET FW or HA or Frigate Fail"
	exit 0
fi



echo "---- Download AI RESET FW & HA & Frigate OK ----" >> /ai/log/webs_ai_reset.log
logger -t AI_RESET "Download AI RESET FW & HA & Frigate OK"

nvram set ai_rsasign_check=0
rsasign_check $fw_path $fw_rsa_path
sleep 2
if [ "$(nvram get ai_rsasign_check)" == "1" ]; then
	echo "---- fw check OK ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "fw check OK"
	sleep 1
else
	echo "---- fw check error rsa: ${rsasign_check_ret} ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "fw check error rsa: ${rsasign_check_ret}"
	rm -f $fw_path
	nvram set webs_state_ai_error=18
	sleep 1
fi

nvram set ai_rsasign_check=0
rsasign_check $ha_path $ha_rsa_path
sleep 2
if [ "$(nvram get ai_rsasign_check)" == "1" ]; then
	echo "---- ha check OK ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "ha check OK"
	sleep 1
else
	echo "---- ha check error rsa: ${rsasign_check_ret} ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "ha check error rsa: ${rsasign_check_ret}"
	rm -f $ha_path
	nvram set webs_state_ai_error=19
	sleep 1
fi

nvram set ai_rsasign_check=0
rsasign_check $frigate_path $frigate_rsa_path
sleep 2
if [ "$(nvram get ai_rsasign_check)" == "1" ]; then
	echo "---- frigate check OK ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "frigate check OK"
	sleep 1
else
	echo "---- frigate check error rsa: ${rsasign_check_ret} ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "frigate check error rsa: ${rsasign_check_ret}"
	rm -f $frigate_path
	nvram set webs_state_ai_error=20
	sleep 1
fi

if [ "$(nvram get webs_state_ai_error)" != "0" ];then
	echo "---- Check RSA of AI RESET FW or HA or Frigate Fail ----" >> /ai/log/webs_ai_reset.log
	logger -t AI_RESET "Check RSA of AI RESET FW or HA or Frigate Fail"
	exit 0
fi

# set ai_fw_path for request
nvram set ai_fw_path="firmware/aisom_reset.swu"
nvram set webs_state_ai_upgrade=1

rc rc_service "start_ai_request update rescue"
sleep 1

# pull GPIO 16 -> low -> high to reboot
echo 255 > /sys/class/leds/led_gpio_16/brightness
echo 0 > /sys/class/leds/led_gpio_16/brightness
# pull GPIO 21 -> low -> high to get into rescue mode (tiny linux) when boot
echo 255 > /sys/class/leds/led_gpio_21/brightness
sleep 5
echo 0 > /sys/class/leds/led_gpio_21/brightness
