/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2012, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <getopt.h>

#include <shared.h>
#include "aupnpc.h"
#include "aupnpc_util.h"


/**
 * @brief Show the amas_ipc test useage
 *
 */
void usage_help()
{
	printf("\nUsage: aupnpc_ipc -A -p protocol -e external_port [-s remote_ip] [-t duration]\n"
		"       aupnpc_ipc -D -p protocol -e external_port [-s remote_ip]\n"
		"       aupnpc_ipc -h\n"
		"Options:\n"
		"\tprotocol: TCP/UDP\n"
		"\texternal_port: 0-65535\n");
	exit(1);
}

/**
 * @brief Main function
 *
 * @param argc argument counter
 * @param argv argument vector
 * @return int The process exit state
 */
int 	main(int argc, char *argv[])
{
	AUPNPC_MSG msg;
	int c, ret = -1, ext_port, duration;
	char rsp_msg[64] = {0};

	printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	memset(&msg, 0, sizeof(msg));

	if(argc < 3)
	{
		usage_help();
	}

	printf("[%s, %d]\n", __FUNCTION__, __LINE__);

	while((c = getopt(argc, argv, "ADp:e:s:t:h")) != -1)
	{
		switch(c)
		{
			case 'A':
				msg.act = AUPNPC_ACT_ADD;
				break;
			case 'D':
				msg.act = AUPNPC_ACT_DEL;
				break;
			case 'p':
				if(!strcasecmp(optarg, "TCP"))
				{
					strlcpy(msg.protocol, "TCP", sizeof(msg.protocol));
				}
				else if(!strcasecmp(optarg, "UDP"))
				{
					strlcpy(msg.protocol, "UDP", sizeof(msg.protocol));
				}
				else
				{
					usage_help();
				}
				break;
			case 'e':
				ext_port = atoi(optarg);
				if(ext_port < 0 || ext_port > 65535)
				{
					usage_help();
				}
				else
				{
					msg.ext_port = ext_port;
				}
				break;
			case 's':
				if(validate_ip(optarg))
				{
					strlcpy(msg.src_ip, optarg, sizeof(msg.src_ip));
				}
				else
				{
					usage_help();
				}
				break;
			case 't':
				duration = atoi(optarg);
				if(duration <= 0)
				{
					usage_help();
				}
				else
				{
					msg.duration = duration;
				}
				break;
			case 'h':
				usage_help();
				break;
			default:
				printf("Invalid parameters: '%c'", c);
				usage_help();
		}
	}

	printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	//check parameters
	if(!valid_aupnpc_msg(&msg))
	{
		printf("[%s, %d] Invalid parameters\n", __FUNCTION__, __LINE__);
		return 0;
	}

	ret = send_msg_to_ipc_socket(AUPNPC_IPC_SOCKET_PATH, (char *)&msg, sizeof(msg), rsp_msg, sizeof(rsp_msg), 30000, AUPNPC_IPC_LOG_PATH);

	if(!ret)
	{
		if(strlen(rsp_msg) > 0)
		{
			printf("IPC: Get response msg: %s\n", rsp_msg);
		}
		else
		{
			printf("IPC: Doesn't get response.\n");
		}
	}
	else
	{
		printf("Send msg to IPC socket fail.\n");
	}

	printf("[%s, %d]\n", __FUNCTION__, __LINE__);
	return 0;
}
