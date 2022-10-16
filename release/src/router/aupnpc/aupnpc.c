#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <shared.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "aupnpc.h"
#include "aupnpc_util.h"

#define AUPNPC_PIDFILE "/var/run/aupnpc.pid"
#define AUPNPC_IPC_MAX_CONNECTION       10
#define AUPNPC_RSP_STR "Received"

pthread_mutex_t work_mutex;
int stop = 0;
EVENT_LIST *evq = NULL;

static int _open_ipc_socket();
static void _close_ipc_socket(int fd);

static int _open_ipc_socket()
{
	int ipc_socket = -1;
	struct sockaddr_un sock_addr_ipc;

	/* IPC Socket */
	if((ipc_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		AUPNPC_DBG("[%s, %d]failed to IPC socket create!\n", __FUNCTION__, __LINE__);
		goto err;
	}

	memset(&sock_addr_ipc, 0, sizeof(sock_addr_ipc));
	sock_addr_ipc.sun_family = AF_UNIX;
	snprintf(sock_addr_ipc.sun_path, sizeof(sock_addr_ipc.sun_path), "%s", AUPNPC_IPC_SOCKET_PATH);
	unlink(AUPNPC_IPC_SOCKET_PATH);

	if(bind(ipc_socket, (struct sockaddr *)&sock_addr_ipc, sizeof(sock_addr_ipc)) < -1)
	{
		AUPNPC_DBG("[%s, %d]failed to IPC socket bind!\n", __FUNCTION__, __LINE__);
		goto err;
	}

	if(listen(ipc_socket, AUPNPC_IPC_MAX_CONNECTION) == -1)
	{
		AUPNPC_DBG("[%s, %d]failed to IPC socket listen!\n", __FUNCTION__, __LINE__);
		goto err;
	}

	return ipc_socket;

err:
	_close_ipc_socket(ipc_socket);
	return -1;
}

static void _close_ipc_socket(int ipc_socket)
{
	if(ipc_socket >= 0)
	{
		close(ipc_socket);
	}
}

static void _dump_aupnpc_msg(AUPNPC_MSG *msg)
{
	if(msg)
	{
		AUPNPC_DBG("Dump msg:\n\t\tact:%d\n\t\tprotocol:%s\n\t\text_port:%d\n\t\tsrc_ip:%s\n\t\tduration:%d\n",
			msg->act, msg->protocol, msg->ext_port, msg->src_ip, msg->duration);
	}
}

static void _ipc_receive_handler(int ipc_socket)
{
	int sockfd = -1;
	char buf[512];
	AUPNPC_MSG *msg;

	if(ipc_socket < 0)
	{
		AUPNPC_DBG("[%s, %d]ipc socket error!\n", __FUNCTION__, __LINE__);
		return;
	}

	/* accept socket */
	sockfd = accept(ipc_socket, NULL, NULL);
	if(sockfd < 0)
	{
		AUPNPC_DBG("[%s, %d]failed to socket accept()!\n", __FUNCTION__, __LINE__);
		return;
	}

	/* read socket */
	if(read_msg_from_ipc_socket(sockfd, buf, sizeof(AUPNPC_MSG), AUPNPC_RSP_STR, strlen(AUPNPC_RSP_STR), 3000, AUPNPC_LOG_PATH) < 0)
	{
		AUPNPC_DBG("[%s, %d]read socket error!\n", __FUNCTION__, __LINE__);
		return;
	}

	//push in queue
	msg = calloc(1, sizeof(AUPNPC_MSG));
	memcpy(msg, buf, sizeof(AUPNPC_MSG));

	_dump_aupnpc_msg(msg);

	//push in queue.
	pthread_mutex_lock(&work_mutex);
	evq_push_msg(&evq, msg);
	pthread_mutex_unlock(&work_mutex);
}

/*******************************************************************
* NAME: write_pid_file
* AUTHOR: Andy Chiu
* CREATE DATE: 2019/12/18
* DESCRIPTION: write the pid file on /var/run
* INPUT: None
* OUTPUT: None
* RETURN: None
* NOTE:
*******************************************************************/
void write_pid_file(void)
{
	int pid_file = 0;
	char pidbuf[8] = {0};
	int pidbuflen = 0;

	pid_file = open(AUPNPC_PIDFILE, O_CREAT | O_RDWR, 0666);
	if(pid_file != -1)
	{
		pidbuflen = snprintf(pidbuf, sizeof(pidbuf), "%d", getpid());
		write(pid_file, pidbuf, pidbuflen);
		close(pid_file);
	}
	else
	{
		AUPNPC_DBG("[%s, %d]Cannot create %s.\n", __FUNCTION__, __LINE__, AUPNPC_PIDFILE);
		exit(0);
	}
}

/*******************************************************************
* NAME: interrupt_handler
* AUTHOR: Andy Chiu
* CREATE DATE: 2019/12/18
* DESCRIPTION: The callback function to handle signal.
* INPUT: None
* OUTPUT: None
* RETURN: None
* NOTE:
*******************************************************************/
void interrupt_handler(int sig)
{
	if(sig == SIGTERM)
	{
		stop = 1;
	}
}



void *handle_event_queue(void *arg)
{
	AUPNPC_MSG *msg;
	const char *wan_if, *wan_ip;
	char cmd[1024];

	while(!stop)
	{
		if(evq)
		{
			wan_if = get_wanface();
			wan_ip = get_wanip();
			if(!wan_if || wan_if[0] == '\0' || !wan_ip || wan_ip[0] == '\0')
			{
				continue;
			}

			do
			{
				pthread_mutex_lock(&work_mutex);
				msg = evq_pop_msg(&evq);
				pthread_mutex_unlock(&work_mutex);

				if(msg)
				{
					memset(cmd, sizeof(cmd), 0);
					strlcpy(cmd, "miniupnpc-new", sizeof(cmd));
					if(msg->act == 1)
					{
						snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -m %s -i -a %s %d %d %s",
							wan_if, wan_ip, msg->ext_port, msg->ext_port, msg->protocol);
						if(msg->duration > 0)
						{
							if(msg->src_ip[0] != '\0')
							{
								snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " %u %s", msg->duration, msg->src_ip);
							}
							else
							{
								snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " %u", msg->duration);
							}
						}
					}
					else
					{
						snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " -m %s -i -d %d %s",
							wan_if, msg->ext_port, msg->protocol);
						if(msg->src_ip[0] != '\0')
						{
							snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " %s", msg->src_ip);
						}
					}
					system(cmd);
					SAFE_FREE(msg);
				}
			}
			while(evq);
		}
		usleep(100000);
	}
}

/*******************************************************************
* NAME: main
* AUTHOR: Andy Chiu
* CREATE DATE: 2019/12/18
* DESCRIPTION: main function
* INPUT:
* OUTPUT:  None
* RETURN: AUPNPC_SUCCESS or AUPNPC_FAIL
* NOTE:
*******************************************************************/
int main(int argc, char **argv)
{
	int ipc_socket = -1;
	fd_set fdSet;
	struct timeval timeout;
	int res;
	pthread_t a_thread;
	pthread_attr_t thread_attr;

	signal(SIGUSR1, interrupt_handler);	//for test
	signal(SIGTERM, interrupt_handler);

	AUPNPC_DBG("Start aupnpc!\n", __FUNCTION__, __LINE__);

	write_pid_file();

	evq = NULL;

	//create thread to handle the led command queue
	res = pthread_attr_init(&thread_attr);
	if(res)
	{
		AUPNPC_DBG("[%s, %d]hid_led_handler create thread attribute failed!\n", __FUNCTION__, __LINE__);
		return EXIT_FAILURE;
	}

	res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	if(res)
	{
		AUPNPC_DBG("[%s, %d]hid_led_handler setting detached attribute failed!\n", __FUNCTION__, __LINE__);
		return EXIT_FAILURE;
	}

	pthread_mutex_init(&work_mutex, NULL);

	res = pthread_create(&a_thread, &thread_attr, (void *)handle_event_queue, NULL);
	if(res)
	{
		AUPNPC_DBG("[%s, %d]hid_led_handler create thread failed!\n", __FUNCTION__, __LINE__);
		return EXIT_FAILURE;
	}

	ipc_socket = _open_ipc_socket();
	if(ipc_socket == -1)
	{
		return 0;
	}

	while(!stop)
	{
		/* must re- FD_SET before each select() */
		FD_ZERO(&fdSet);

		FD_SET(ipc_socket, &fdSet);

		timeout.tv_sec = 2;
		timeout.tv_usec = 0;

		/* must use ipc_socket+1, not ipc_socket */
		if(select(ipc_socket + 1, &fdSet, NULL, NULL, &timeout) < 0)
		{
			break;
		}

		/* handle packets from IPC */
		if(FD_ISSET(ipc_socket, &fdSet))
		{
			_ipc_receive_handler(ipc_socket);
		}
	}
	_close_ipc_socket(ipc_socket);
	unlink(AUPNPC_PIDFILE);
	pthread_mutex_destroy(&work_mutex);
	pthread_attr_destroy(&thread_attr);
	return 0;
}


