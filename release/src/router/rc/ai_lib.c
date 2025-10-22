/*
* Copyright 2025, ASUSTeK Inc.
* All Rights Reserved.
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of ASUSTeK Inc..
*/

/*
	Library support for AI Board services
*/

#include <rc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <shared.h>
#include <json.h>
#include <openssl/md5.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ai_lib.h"

#if defined(RTCONFIG_AI_SERVICE)

/* 
 * define apps commands
 */
static ai_app_cmd_t tz_cmds[] = {
	{APP_TZ, TZ_SETTING, NULL, 20},
	{APP_TZ, TZ_SYNC, NULL, 20}
};

static ai_app_cmd_t portainer_start_cmds[] = {
	{APP_PORTAINER, TZ_SETTING, NULL, 20},
	{APP_PORTAINER, TZ_SYNC, NULL, 20},
	{APP_PORTAINER, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq formal-portainer-asus && docker stop formal-portainer-asus >/dev/null 2>&1 || true", 20},
	{APP_PORTAINER, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq formal-portainer-asus && docker rm -f formal-portainer-asus >/dev/null 2>&1 || true", 20},
	{APP_PORTAINER, CHECK, "docker volume inspect formal_portainer_data_asus >/dev/null 2>&1 || docker volume create formal_portainer_data_asus >/dev/null 2>&1", 20},
	{APP_PORTAINER, DOWNLOAD, "docker images --format '{{.Repository}}:{{.Tag}}' | grep -wq portainer/portainer-ce:2.33.0 || docker pull portainer/portainer-ce:2.33.0 >/dev/null 2>&1", 600},
	{APP_PORTAINER, CREATE, "docker create -p 8000:8000 -p 9443:9443 --name formal-portainer-asus --restart=unless-stopped -v /var/run/docker.sock:/var/run/docker.sock -v formal_portainer_data_asus:/data portainer/portainer-ce:2.33.0 >/dev/null 2>&1", 30},
	{APP_PORTAINER, RUN, "docker start formal-portainer-asus >/dev/null 2>&1", 30}
};

static ai_app_cmd_t portainer_stop_cmds[] = {
	{APP_PORTAINER, TZ_SETTING, NULL, 20},
	{APP_PORTAINER, TZ_SYNC, NULL, 20},
	{APP_PORTAINER, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq formal-portainer-asus && docker stop formal-portainer-asus >/dev/null 2>&1 || true", 20}
};

static ai_app_cmd_t portainer_reset_cmds[] = {
	{APP_PORTAINER, TZ_SETTING, NULL, 20},
	{APP_PORTAINER, TZ_SYNC, NULL, 20},
	{APP_PORTAINER, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq formal-portainer-asus && docker stop formal-portainer-asus >/dev/null 2>&1 || true", 10},
	{APP_PORTAINER, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq formal-portainer-asus && docker rm -f formal-portainer-asus >/dev/null 2>&1 || true", 10},
	{APP_PORTAINER, CLEANUP, "docker volume inspect formal_portainer_data_asus >/dev/null 2>&1 && docker volume rm formal_portainer_data_asus >/dev/null 2>&1 || true", 10},
	{APP_PORTAINER, CLEANUP, "docker image inspect portainer/portainer-ce:2.33.0 >/dev/null 2>&1 && docker rmi -f portainer/portainer-ce:2.33.0 >/dev/null 2>&1 || true", 10}
};

static ai_app_cmd_t frigate_start_cmds[] = {
	{APP_FRIGATE, TZ_SETTING, NULL, 20},
	{APP_FRIGATE, TZ_SYNC, NULL, 20},
	{APP_FRIGATE, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq formal-frigate-asus && docker stop formal-frigate-asus >/dev/null 2>&1 || true", 20},
	{APP_FRIGATE, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq formal-frigate-asus && docker rm -f formal-frigate-asus >/dev/null 2>&1 || true", 20},
	{APP_FRIGATE, CHECK, "docker volume inspect formal_frigate_config_asus >/dev/null 2>&1 || docker volume create formal_frigate_config_asus >/dev/null 2>&1", 20},
	{APP_FRIGATE, CHECK, "docker volume inspect formal_frigate_media_asus >/dev/null 2>&1 || docker volume create formal_frigate_media_asus >/dev/null 2>&1", 20},
	{APP_FRIGATE, DOWNLOAD, "docker images --format '{{.Repository}}:{{.Tag}}' | grep -wq asusnw/frigate:0.15.1 || docker pull asusnw/frigate:0.15.1 >/dev/null 2>&1", 1800},
	{APP_FRIGATE, CREATE, "docker create --privileged --name formal-frigate-asus --restart=unless-stopped --mount type=tmpfs,target=/tmp/cache,tmpfs-size=1000000000 --device /dev/bus/usb:/dev/bus/usb --device /dev/synap:/dev/synap --device /dev/video0:/dev/video0 --device /dev/video1:/dev/video1 --shm-size=256m -v formal_frigate_media_asus:/media/frigate -v formal_frigate_config_asus:/config -v /dev/synap:/dev/synap -v /dev/video0:/dev/video0 -v /dev/video1:/dev/video1 -p 8971:8971 -p 8554:8554 -p 8555:8555/tcp -p 8555:8555/udp asusnw/frigate:0.15.1 >/dev/null 2>&1", 10},
	{APP_FRIGATE, RUN, "docker start formal-frigate-asus >/dev/null 2>&1", 30}
};

static ai_app_cmd_t adguard_start_cmds[] = {
	{APP_ADGUARD, TZ_SETTING, NULL, 20},
	{APP_ADGUARD, TZ_SYNC, NULL, 20},
	{APP_ADGUARD, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq adguard-asus && docker stop adguard-asus >/dev/null 2>&1 || true", 30},
	{APP_ADGUARD, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq adguard-asus && docker rm -f adguard-asus >/dev/null 2>&1 || true", 30},
	{APP_ADGUARD, CHECK, "[ -d /etc/systemd/resolved.conf.d ] || mkdir -p /etc/systemd/resolved.conf.d", 10},
	{APP_ADGUARD, CHECK, "echo '[Resolve]' > /etc/systemd/resolved.conf.d/adguardhome.conf", 10},
	{APP_ADGUARD, CHECK, "echo 'DNS=127.0.0.1' >> /etc/systemd/resolved.conf.d/adguardhome.conf", 10},
	{APP_ADGUARD, CHECK, "echo 'DNSStubListener=no' >> /etc/systemd/resolved.conf.d/adguardhome.conf", 10},
	{APP_ADGUARD, CHECK, "mv /etc/resolv.conf /etc/resolv.conf.backup", 10},
	{APP_ADGUARD, CHECK, "ln -s /run/systemd/resolve/resolv.conf /etc/resolv.conf", 10},
	{APP_ADGUARD, CHECK, "systemctl restart systemd-resolved", 10},
	{APP_ADGUARD, CHECK, "docker volume inspect adguard_work_asus >/dev/null 2>&1 || docker volume create adguard_work_asus >/dev/null 2>&1", 30},
	{APP_ADGUARD, CHECK, "docker volume inspect adguard_conf_asus >/dev/null 2>&1 || docker volume create adguard_conf_asus >/dev/null 2>&1", 30},
	{APP_ADGUARD, DOWNLOAD, "docker images --format '{{.Repository}}:{{.Tag}}' | grep -wq adguard/adguardhome:latest || docker pull adguard/adguardhome:latest >/dev/null 2>&1", 600},
	{APP_ADGUARD, CREATE, "docker create --name adguard-asus --restart=unless-stopped --network host -v adguard_work_asus:/opt/adguardhome/work -v adguard_conf_asus:/opt/adguardhome/conf adguard/adguardhome:latest >/dev/null 2>&1", 30},
	{APP_ADGUARD, RUN, "docker start adguard-asus >/dev/null 2>&1", 30}
};

static ai_app_cmd_t test_cmds[] = {
	{APP_TEST, TZ_SETTING, NULL, 20},
	{APP_TEST, TZ_SYNC, NULL, 20},
	{APP_TEST, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq hello-asus && docker stop hello-asus >/dev/null 2>&1 || true", 60},
	{APP_TEST, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq hello-asus && docker rm -f hello-asus >/dev/null 2>&1 || true", 60},
	{APP_TEST, DOWNLOAD, "docker images --format '{{.Repository}}:{{.Tag}}' | grep -wq hello-world:latest || docker pull hello-world:latest >/dev/null 2>&1", 300},
	{APP_TEST, CREATE, "docker create --pull always --name hello-asus hello-world:latest >/dev/null 2>&1", 30},
	{APP_TEST, RUN, "docker start hello-asus >/dev/null 2>&1", 30}
};

static ai_app_cmd_t ha_start_cmds[] = {
	{APP_HA, TZ_SETTING, NULL, 20},
	{APP_HA, TZ_SYNC, NULL, 20},
	{APP_HA, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq homeassistant-asus && docker stop homeassistant-asus >/dev/null 2>&1 || true", 60},
	{APP_HA, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq homeassistant-asus && docker rm -f homeassistant-asus >/dev/null 2>&1 || true", 60},
	{APP_HA, CHECK, "docker volume inspect homeassistant_config_asus >/dev/null 2>&1 || docker volume create homeassistant_config_asus >/dev/null 2>&1", 30},
	{APP_HA, DOWNLOAD, "docker images --format '{{.Repository}}:{{.Tag}}' | grep -wq homeassistant/home-assistant:latest || docker pull homeassistant/home-assistant:latest >/dev/null 2>&1", 1800},
	{APP_HA, CREATE, "docker create --name homeassistant-asus --privileged --restart=unless-stopped -v homeassistant_config_asus:/config -v /run/dbus:/run/dbus --network=host homeassistant/home-assistant:latest >/dev/null 2>&1", 30},
	{APP_HA, RUN, "docker start homeassistant-asus >/dev/null 2>&1", 30}
};

#if defined(GTBE96_AI)
static ai_app_cmd_t slm_start_cmds[] = {
	{APP_SLM, TZ_SETTING, NULL, 20},
	{APP_SLM, TZ_SYNC, NULL, 20},
	{APP_SLM, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq slm-cn-asus && docker stop slm-cn-asus >/dev/null 2>&1 || true", 60},
	{APP_SLM, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq slm-cn-asus && docker rm -f slm-cn-asus >/dev/null 2>&1 || true", 60},
	{APP_SLM, CHECK, "docker volume inspect slm_cn_data_asus >/dev/null 2>&1 || docker volume create slm_cn_data_asus >/dev/null 2>&1", 30},
	{APP_SLM, CHECK, "tar -xf /home/root/slm-cn-data.archive -C /home/docker/volumes/slm_cn_data_asus/_data/ --strip-components=1 >/dev/null 2>&1", 30},
	{APP_SLM, DOWNLOAD, "docker pull asusnw/slm-cn:latest >/dev/null 2>&1", 600},
	{APP_SLM, CREATE, "docker create --name slm-cn-asus --restart=unless-stopped -v slm_data_asus:/llm/ai_assistant/data -p "AIBOARD_CONTROL_IP":8090:8090 asusnw/slm-cn:latest >/dev/null 2>&1", 30},
	{APP_SLM, RUN, "docker start slm-cn-asus >/dev/null 2>&1", 30}
};
#endif

static ai_app_cmd_t slm_update_cmds[] = {
	{APP_SLM, TZ_SETTING, NULL, 20},
	{APP_SLM, TZ_SYNC, NULL, 20},
	{APP_SLM, CLEANUP, "docker ps --format '{{.Names}}' | grep -wq slm-asus && docker stop slm-asus >/dev/null 2>&1 || true", 60},
	{APP_SLM, CLEANUP, "docker ps -a --format '{{.Names}}' | grep -wq slm-asus && docker rm -f slm-asus >/dev/null 2>&1 || true", 60},
	{APP_SLM, CHECK, "docker volume inspect slm_data_asus >/dev/null 2>&1 || docker volume create slm_data_asus >/dev/null 2>&1", 30},
	{APP_SLM, DOWNLOAD, "/usr/sbin/busybox tftp -r docker_images/slm-data.tar -g -l /home/root/slm-data.archive -b 16384 169.254.0.1 >/dev/null 2>&1", 300},
	{APP_SLM, CHECK, "tar -xf /home/root/slm-data.archive -C /home/docker/volumes/slm_data_asus/_data/ --strip-components=1 >/dev/null 2>&1", 30},
	{APP_SLM, DOWNLOAD, "docker pull asusnw/slm:latest >/dev/null 2>&1", 600},
	{APP_SLM, CREATE, "docker create --name slm-asus --restart=unless-stopped -v slm_data_asus:/llm/ai_assistant/data -p "AIBOARD_CONTROL_IP":8090:8090 asusnw/slm:latest >/dev/null 2>&1", 30},
	{APP_SLM, RUN, "docker start slm-asus >/dev/null 2>&1", 30}
};

static ai_app_cmd_t slmqaver_get_cmds[] = {
	{APP_SLMQAVER, CHECK, "cp /home/docker/volumes/slm_data_asus/_data/version.json /tmp/version.json >/dev/null 2>&1", 30},
	{APP_SLMQAVER, CHECK, "jq-linux-arm64 --arg now $(date '+%s') '.last_update = $now' /tmp/version.json | tee /tmp/qa_version.json >/dev/null 2>&1", 30},
	{APP_SLMQAVER, DOWNLOAD, "/usr/sbin/busybox tftp -p -l /tmp/qa_version.json  -r slmqa/qa_version.json -b 16384 169.254.0.1 >/dev/null 2>&1", 30}
};

static ai_app_cmd_t slmqa_update_cmds[] = {
	{APP_SLMQA, DOWNLOAD, "/usr/sbin/busybox tftp -r slmqa/slm-data.tar -g -l /tmp/slm-data.tar -b 16384 169.254.0.1 >/dev/null 2>&1", 30},
	{APP_SLMQA, DOWNLOAD, "cp /tmp/slm-data.tar /home/root/slm-data.archive >/dev/null 2>&1", 30},
	{APP_SLMQA, CHECK, "tar -xf /tmp/slm-data.tar -C /home/docker/volumes/slm_data_asus/_data/ --strip-components=1 >/dev/null 2>&1", 30},
	{APP_SLMQA, CLEANUP, "docker exec slm-asus /bin/bash -c 'rm /llm/ai_assistant/cached/vectors-*.tsv' >/dev/null 2>&1 || true", 30},
	{APP_SLMQA, RUN, "docker restart slm-asus >/dev/null 2>&1", 30},
	// add return the latest version.json back
	{APP_SLMQA, CHECK, "cp /home/docker/volumes/slm_data_asus/_data/version.json /tmp/version.json >/dev/null 2>&1", 30},
	{APP_SLMQA, CHECK, "jq-linux-arm64 --arg now $(date '+%s') '.last_update = $now' /tmp/version.json | tee /tmp/qa_version.json >/dev/null 2>&1", 30},
	{APP_SLMQA, RUN, "/usr/sbin/busybox tftp -p -l /tmp/qa_version.json  -r slmqa/qa_version.json -b 16384 169.254.0.1 >/dev/null 2>&1", 30}
};

ai_app_item_t ai_app_map[] = {
    {APP_TZ, tz_cmds, sizeof(tz_cmds)/sizeof(tz_cmds[0])},
    {APP_TEST, test_cmds, sizeof(test_cmds)/sizeof(test_cmds[0])},
    {"start_portainer", portainer_start_cmds, sizeof(portainer_start_cmds)/sizeof(portainer_start_cmds[0])},
    {"stop_portainer", portainer_stop_cmds, sizeof(portainer_stop_cmds)/sizeof(portainer_stop_cmds[0])},
    {"reset_portainer", portainer_reset_cmds, sizeof(portainer_reset_cmds)/sizeof(portainer_reset_cmds[0])},
    {"start_frigate", frigate_start_cmds, sizeof(frigate_start_cmds)/sizeof(frigate_start_cmds[0])},
    {"start_homeassistant", ha_start_cmds, sizeof(ha_start_cmds)/sizeof(ha_start_cmds[0])},
    {"start_adguard", adguard_start_cmds, sizeof(adguard_start_cmds)/sizeof(adguard_start_cmds[0])},
#if defined(GTBE96_AI)
    {"start_slm", slm_start_cmds, sizeof(slm_start_cmds)/sizeof(slm_start_cmds[0])},
#endif
    {"update_slm", slm_update_cmds, sizeof(slm_update_cmds)/sizeof(slm_update_cmds[0])},
    {"get_slmqaver", slmqaver_get_cmds, sizeof(slmqaver_get_cmds)/sizeof(slmqaver_get_cmds[0])},
    {"update_slmqa", slmqa_update_cmds, sizeof(slmqa_update_cmds)/sizeof(slmqa_update_cmds[0])}
};

const int ai_app_map_size = sizeof(ai_app_map) / sizeof(ai_app_map[0]);

#endif
