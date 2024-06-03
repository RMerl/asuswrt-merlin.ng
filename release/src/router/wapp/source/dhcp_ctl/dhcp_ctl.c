/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
 
 /*
 *  Revision History:
 *  Who                      When               What
 *  --------               ----------    -----------------------------------------
 *  Guangbin.Zhong     2018/12/15     First implementation of the dhcp control
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>


#include "dhcp_ctl.h"
#include "debug.h"


#define MAX_IP_STR_LEN 64

/*************************************************
*
*Function name : get_br_ip
*Description   : get it's ip from bridge or some other interface
*Parameter     :
*		@br_name  inout bridge of bridge name
*		@ip_buf   output ip buffer
*
*Return        :
*       0 success , other fail
*
*other         :
*		OS     : openwrt and linux
*************************************************/

int get_bridge_ip(char* br_name, char *ip_buf)
{
    struct ifreq temp;
    struct sockaddr_in *myaddr;
    int fd = 0;
    int ret = -1;
    if (br_name == NULL || ip_buf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"Invalid input parameter!\n");
        return -1;
    }

    snprintf(temp.ifr_name, IFNAMSIZ, "%s", br_name);
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
    	DBGPRINT(RT_DEBUG_ERROR,"Can't create socket!\n");
        return-1;
    }
    ret = ioctl(fd, SIOCGIFADDR, &temp);
    close(fd);
    if(ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR,"Can't get ip for %s!\n", br_name);
		return -1;
	}
    myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
	if (strlen(inet_ntoa(myaddr->sin_addr)) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR,"get ip length is zero!\n");
		return -1;
	}
	snprintf(ip_buf, 128, "%s", inet_ntoa(myaddr->sin_addr));

	DBGPRINT(RT_DEBUG_TRACE,"%s ip is : %s\n", br_name, ip_buf);
    return 0;
}


#ifdef DHCP_CTL_OPENWRT
/*************************************************
*
*Function name : get_br_defalut_ip
*Description   : get bridge ip from openwrt uci config file
*Parameter     :
*		@ip_buf   output ip buffer
*
*Return        :
*       0 success , other fail
*
*other         :
*		OS     : openwrt
*************************************************/

static int get_br_defalut_ip (char *ip_buf)
{
	FILE *fp;
	char command [100] = {0};

	if (NULL == ip_buf) {
		DBGPRINT(RT_DEBUG_ERROR, "[%s],INPUT parameter ip_buf is NULL!\n\n", __func__);
		return -1;
	}

	sprintf(command, "uci -q get network.lan.ipaddr > /tmp/temp_file_br_ip");
	system(command);
	DBGPRINT(RT_DEBUG_TRACE,"command: %s\n",command);
	fp = fopen("/tmp/temp_file_br_ip", "r");
	fgets(ip_buf, MAX_IP_STR_LEN, fp);
	fclose(fp);
	DBGPRINT(RT_DEBUG_TRACE, "default ip of br interface: %s \n", ip_buf);

	if (strlen(ip_buf) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Default ip is unconfiged!\n");
		return -1;
	}

	return 0;
}

/*************************************************
*
*Function name : set_br_default_ip
*Description   : set br to default ip and not enable dhcp server
*Parameter	   :
*		@br_name IN  bridge name
*
*Return 	   :
*		0 success , other fail
*
*other		   :
*		OS	   : openwrt
*************************************************/

int set_br_default_ip(char *br_name){
	char cmd[150];
	char br_ip_str[MAX_IP_STR_LEN] = {0};

	if(NULL == br_name || strlen(br_name) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "input br_name is null!\n");
		return -1;
	}

	memset(br_ip_str, 0, MAX_IP_STR_LEN);
	memset (cmd, 0, sizeof(cmd));

	if (0 != get_br_defalut_ip(br_ip_str)){
		DBGPRINT(RT_DEBUG_OFF, "Set Default ip: 192.168.1.1\n");
		memcpy(br_ip_str,"192.168.1.1",strlen("192.168.1.1"));
	}

	sprintf(cmd, "ifconfig %s %s up;", br_name, br_ip_str);
	system(cmd);
	DBGPRINT(RT_DEBUG_TRACE, "set_br_default_ip: %s \n",cmd);
	return 0;
}

/*************************************************
*
*Function name : disable_dhcp_client
*Description   : To disable dhcp client service
*Parameter	   :
*		@br_name    IN  bridge name
*
*Return 	   :
*		void
*other		   :
*		OS	   : openwrt
*************************************************/
void disable_dhcp_client(char *br_name)
{
    char buf[128] = "";
    char cmd[128] = "";
	FILE *fp = NULL;

	sprintf(cmd, "ps | grep 'udhcpc -i %s' |grep -v grep| awk '{print $1}'", br_name);

	fp = popen(cmd, "r");
	if(fp) {
		while(fgets(buf, sizeof(buf) - 1, fp)) {
			DBGPRINT(RT_DEBUG_OFF,"%s pid:%s\n", __func__, buf);
			memset (cmd, 0, sizeof(cmd));
			sprintf(cmd, "kill -15 %s", buf);
			system(cmd);
	    }
	    pclose(fp);
	}
}

/*************************************************
*
*Function name : enable_dhcp_client
*Description   : run dhcp client to get ip from dhcp server for bridge
*Parameter	   :
*		@br_name    IN  bridge name
*
*Return 	   :
*		0 success , other fail
*
*other		   :
*		OS	   : openwrt
*************************************************/

int enable_dhcp_client(char *br_name) {
    char cmd[128]={'\0'};
    char buffer[256]={'\0'};

	if (NULL == br_name || strlen(br_name) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "input br_name  is NULL!\n");
		return -1;
	}
	disable_dhcp_client(br_name);
	disable_dhcp_server();
    memset(buffer,0,sizeof(buffer));
    sprintf(cmd, "udhcpc -i %s &", br_name);
    DBGPRINT(RT_DEBUG_OFF, "%s: %s\n", __func__, cmd);
    system(cmd);
    return 0;
}


/*************************************************
*
*Function name : disable_dhcp_server
*Description   : disable dhcp server service
*Parameter	   :
*		void
*
*Return 	   :
*		void
*
*other		   :
*		OS	   : openwrt
*************************************************/

void disable_dhcp_server() {
	char cmd[128] = {0};
	sprintf(cmd, "uci set dhcp.lan.ignore=1; uci commit; /etc/init.d/dnsmasq reload");
	system(cmd);
	DBGPRINT(RT_DEBUG_TRACE, "[%s]: command: %s\n", __func__,cmd);
}

/*************************************************
*
*Function name : enable_dhcp_server
*Description   : enable dhcp server service
*Parameter	   :
*		@br_name IN  bridge name
*
*Return 	   :
*		0 success , other fail
*
*other		   :
*		OS	   : openwrt
*************************************************/

int enable_dhcp_server(char *br_name){
	char cmd[150];
	char br_ip_str[MAX_IP_STR_LEN] = {0};

	if(NULL == br_name || strlen(br_name) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR, "input br_name is null!\n");
		return -1;
	}

	memset(br_ip_str, 0, MAX_IP_STR_LEN);
	memset (cmd, 0, sizeof(cmd));

	disable_dhcp_client(br_name);
	disable_dhcp_server();
	if (0 != get_br_defalut_ip(br_ip_str)){
		DBGPRINT(RT_DEBUG_OFF, "Set Default ip: 192.168.1.1\n");
		memcpy(br_ip_str,"192.168.1.1",strlen("192.168.1.1"));
	}

	sprintf(cmd, "ifconfig %s %s up; uci set dhcp.lan.ignore=\"\"; uci commit; /etc/init.d/dnsmasq reload", br_name, br_ip_str);
	system(cmd);
	DBGPRINT(RT_DEBUG_TRACE, "run_dhcp_server: %s \n",cmd);
	return 0;
}

#else /*LINUX OS*/

/*************************************************
*
*Function name : get_br_defalut_ip
*Description   : get bridge ip from nvram
*Parameter	   :
*		@ip_buf   output ip buffer
*
*Return 	   :
*		0 success , other fail
*
*other		   :
*		OS	   : linux
*************************************************/

static int get_br_defalut_ip (char *ip_buf)
{
	FILE *fp;
	char command [100] = {0};

	if (NULL == ip_buf) {
		DBGPRINT(RT_DEBUG_ERROR, "INPUT parameter ip_buf is NULL!\n");
		return -1;
	}

	sprintf(command, "nvram_get 2860 lan_ipaddr > /tmp/temp_file_br_ip");
	system(command);
	DBGPRINT(RT_DEBUG_TRACE,"command: %s\n",command);
	fp = fopen("/tmp/temp_file_br_ip", "r");
	fgets(ip_buf, MAX_IP_STR_LEN, fp);
	fclose(fp);
	DBGPRINT(RT_DEBUG_TRACE, "default ip of br interface: %s \n", ip_buf);

	if (strlen(ip_buf) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR,"Default ip is unconfiged!\n");
		return -1;
	}

	return 0;
}

/*************************************************
*
*Function name : set_br_default_ip
*Description   :  set br to default ip and not enable dhcp server
*Parameter	   :
*		@br_name IN  bridge name
*
*Return 	   :
*		0 success , other fail
*
*other		   :
*		OS	   : linux
*************************************************/

int set_br_default_ip(char *br_name)
{
	char cmd[150];
	char br_ip_str[MAX_IP_STR_LEN] = {0};

	if(NULL == br_name || strlen(br_name) <= 0) {
		DBGPRINT(RT_DEBUG_OFF, "input br_name is null!\n");
		return -1;
	}

	memset(br_ip_str, 0, MAX_IP_STR_LEN);
	memset (cmd, 0, sizeof(cmd));

	if (0 != get_br_defalut_ip(br_ip_str)){
		DBGPRINT(RT_DEBUG_OFF, "Set Default ip: 10.10.10.254\n");
		memcpy(br_ip_str,"10.10.10.254",strlen("10.10.10.254")+1);
	}
	sprintf(cmd, "ifconfig %s %s up;", br_name, br_ip_str);
	system(cmd);
	DBGPRINT(RT_DEBUG_TRACE, "set br default ip: %s \n",cmd);
	return 0;
}


/*************************************************
*
*Function name : disable_dhcp_client
*Description   : To disable dhcp client service
*Parameter	   :
*		@br_name    IN  bridge name
*
*Return 	   :
*		void
*other		   :
*		OS	   : linux
*************************************************/
void disable_dhcp_client(char *br_name)
{
    char buf[128] = "";
    char cmd[128] = "";
	FILE *fp = NULL;

	sprintf(cmd, "ps | grep 'udhcpc -i %s' |grep -v grep| awk '{print $1}'", br_name);

	fp = popen(cmd, "r");
	if(fp) {
		while(fgets(buf, sizeof(buf) - 1, fp)) {
			DBGPRINT(RT_DEBUG_OFF,"%s pid:%s\n", __func__, buf);
			memset (cmd, 0, sizeof(cmd));
			sprintf(cmd, "kill -15 %s", buf);
			system(cmd);
	    }
	    pclose(fp);
	}
}

/*************************************************
*
*Function name : enable_dhcp_client
*Description   : run dhcp client to get ip from dhcp server for bridge
*Parameter	   :
*		@br_name	IN	bridge name
*
*Return 	   :
*		0 success , other fail
*
*other		   :
*		OS	   : linux
*************************************************/

int enable_dhcp_client(char *br_name)
{
	char cmd[128]={'\0'};
	char buffer[256]={'\0'};

	if (NULL == br_name || strlen(br_name) <= 0) {
		DBGPRINT(RT_DEBUG_ERROR , "input br_name  is NULL!\n");
		return -1;
	}
	disable_dhcp_client(br_name);
	disable_dhcp_server();
	memset(buffer,0,sizeof(buffer));
	sprintf(cmd, "udhcpc -i %s -s /sbin/udhcpc.sh &", br_name);
	DBGPRINT(RT_DEBUG_OFF, "%s: %s\n", __func__, cmd);
	system(cmd);

	return 0;
}

/*************************************************
*
*Function name : disable_dhcp_server
*Description   : disable dhcp server service
*Parameter	   :
*		void
*
*Return 	   :
*		void
*
*other		   :
*		OS	   : linux
*************************************************/

void disable_dhcp_server()
{
	char cmd[128] = {0};
	sprintf(cmd, "killall -15 udhcpd");
	system(cmd);
	DBGPRINT(RT_DEBUG_TRACE, "disable_dhcp_server!");
}

/*************************************************
*
*Function name : enable_dhcp_server
*Description   : enable dhcp server service
*Parameter	   :
*		@br_name IN  bridge name
*
*Return 	   :
*		0 success , other fail
*
*other		   :
*		OS	   : linux
*************************************************/

int enable_dhcp_server(char *br_name)
{
	char cmd[150];
	char br_ip_str[MAX_IP_STR_LEN] = {0};

	if(NULL == br_name || strlen(br_name) <= 0) {
		DBGPRINT(RT_DEBUG_OFF, "input br_name is null!\n");
		return -1;
	}

	memset(br_ip_str, 0, MAX_IP_STR_LEN);
	memset (cmd, 0, sizeof(cmd));

	disable_dhcp_client(br_name);
	disable_dhcp_server();
	if (0 != get_br_defalut_ip(br_ip_str)){
		DBGPRINT(RT_DEBUG_OFF, "Set Default ip: 10.10.10.254\n");
		memcpy(br_ip_str,"10.10.10.254",strlen("10.10.10.254")+1);
	}
	sprintf(cmd, "ifconfig %s %s up;udhcpd /etc/udhcpd.conf &", br_name, br_ip_str);
	system(cmd);
	DBGPRINT(RT_DEBUG_TRACE, "run_dhcp_server: %s \n",cmd);
	return 0;
}
#endif
