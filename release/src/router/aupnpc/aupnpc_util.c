#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <shared.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>

#include "aupnpc_util.h"

/*******************************************************************
* NAME: aupnpc_print
* AUTHOR: Andy Chiu
* CREATE DATE: 2019/12/18
* DESCRIPTION: show debug message on the console and write it to the debug log.
* INPUT:
* OUTPUT: None
* RETURN: None
* NOTE:
*******************************************************************/
void aupnpc_print(const char *file, const char *format, ...)
{
	FILE *f, *f2;
	int nfd;
	va_list args;
	struct stat st;
	char buf[256];
	size_t ret;
	struct  timeval    tv;
	struct  timezone   tz;

	gettimeofday(&tv, &tz);

	//dump in console
	if(((nfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) > 0) &&
		(f = fdopen(nfd, "w")))
	{
		va_start(args, format);
		vfprintf(f, format, args);
		va_end(args);
		fclose(f);
		nfd = -1;
	}
	else
	{
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
	}

	if(nfd != -1)
	{
		close(nfd);
	}
	
	if(file)
	{
		if(f_size(file) >= AUPNC_MAX_LOG_SIZE) 
		{
			unlink(file);
		}
		
		f = fopen(file, "a");
		if(f)
		{
			fprintf(f, "<%d>", tv.tv_sec);
			va_start(args, format);
			vfprintf(f, format, args);
			va_end(args);
			fclose(f);
		}
	}
}


int valid_aupnpc_msg(const AUPNPC_MSG *msg)
{
	if(msg)
	{
		if(msg->act == AUPNPC_ACT_ADD || msg->act == AUPNPC_ACT_DEL)
		{
			if(strcmp(msg->protocol, "TCP") && strcmp(msg->protocol, "UDP"))
			{
				return 0;
			}
			if(msg->ext_port < 0 || msg->ext_port > 65535)
			{
				return 0;
			}
			if(msg->src_ip[0] != '\0' && !validate_ip(msg->src_ip))
			{
				return 0;
			}
			return 1;
		}
	}
	return 0;
}



/**
 * @brief Close IPC socket
 *
 * @param fd Socket file descriptor
 */
static void _close_ipc(const int fd, const char *dbg_file)
{
	if(fd >= 0)
	{
		if(dbg_file)
		{
			//aupnpc_print(dbg_file, "[%s, %d]Close IPC socket. fd:%d\n", __FUNCTION__, __LINE__, fd);
		}
		close(fd);
	}
}

/**
 * @brief Create socket and connect to remote server
 *
 * @param ipc_socket_path IPC socket path
 * @return int Connect to remote server result
 *      > 0: File descriptor number
 *      <= -1: Connect error
 */
static int _connect_ipc(const char *ipc_socket_path, const char *dbg_file)
{
	int fd = -1;
	struct sockaddr_un addr;
	int flags, ret;

	if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]ipc socket error! %s\n", __FUNCTION__, __LINE__, strerror(errno));
		}
		goto CONNECT_IPC_ERR;
	}

	if(fd != -1)
	{

		flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK); // Setting nonblock mode.

		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, ipc_socket_path, sizeof(addr.sun_path) - 1);
		ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
		if(ret == 0)  // Success.
		{
			return fd;
		}
		else
		{
			if(errno == EINPROGRESS)    // Still doing IPC connection process.
			{
				int connect_timeout = 5; // 5 times for connect()
				while(connect_timeout-- > 0)
				{
					fd_set rfds, wfds;
					struct timeval tv;

					FD_ZERO(&rfds);
					FD_ZERO(&wfds);
					FD_SET(fd, &rfds);
					FD_SET(fd, &wfds);

					tv.tv_sec = 10; // select time out 10sec
					tv.tv_usec = 0;
					int selret = select(fd + 1, &rfds, &wfds, NULL, &tv);
					switch(selret)
					{
						case -1 : // select error
							goto CONNECT_IPC_ERR;
						case 0 : // select timeout
							goto CONNECT_IPC_ERR;
						default:
							if(FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds))
							{
								ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
								if(errno == EISCONN)
								{
									ret = 0; // Success
								}
								else
								{
									ret = errno;
								}
							}
							else
							{
								continue;
							}
					}
					if(ret == 0)  // Success.
					{
						break;
					}
				}
				if(ret != 0)
				{
					goto CONNECT_IPC_ERR;
				}
			}
			else
			{
				if(dbg_file)
				{
					aupnpc_print(dbg_file, "[%s, %d]ipc connect error! %s\n", __FUNCTION__, __LINE__, strerror(errno));
				}
				goto CONNECT_IPC_ERR;
			}
		}
	}
	return fd;

CONNECT_IPC_ERR:
	_close_ipc(fd, dbg_file);
	return -1;
}

/**
 * @brief Read message from IPC socket
 *
 * @param fd Socket file descriptor
 * @param msg Messages from IPC
 * @param msg_max_len Max length of message
 * @param timeout Reading socket timeout. The time unit is millisecond
 * @return int Reading socket result
 *      0: Read successfully
 *      -1: Read error
 */
static int _read_ipc_msg(const int fd, char *msg, const int msg_max_len, const int timeout, const char *dbg_file)
{
	int ret, nleft, nread, len = 0;
	char *pMsg;
	struct pollfd monitor_fd;

	if(fd < 0)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Socket description error.fd = %d\n", __FUNCTION__, __LINE__, fd);
		}
		goto READ_IPC_EVENT_ERROR;
	}

	monitor_fd.fd = fd;
	monitor_fd.events = POLLIN;

	ret = poll(&monitor_fd, 1, timeout);

	if(ret == -1)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Poll socket error! %s\n", __FUNCTION__, __LINE__, strerror(errno));
		}
		goto READ_IPC_EVENT_ERROR;
	}
	else if(ret == 0)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Read Socket timeout! %d\n", __FUNCTION__, __LINE__, strerror(errno));
		}
		goto READ_IPC_EVENT_ERROR;
	}
	else
	{
		if(monitor_fd.revents & POLLIN)
		{
			nleft = msg_max_len;
			pMsg = msg;

			while(nleft > 0)
			{
				if((nread = read(fd, msg, nleft)) < 0)
				{
					if(errno == EINTR)
					{
						nread = 0;  /* and call read() again */
						if(dbg_file)
						{
							aupnpc_print(dbg_file, "[%s, %d]errno == EINTR\n", __FUNCTION__, __LINE__);
						}
					}
					else
					{
						if(dbg_file)
						{
							aupnpc_print(dbg_file, "[%s, %d]Failed to socket read(%d)!\n", __FUNCTION__, __LINE__, errno);
						}
						goto READ_IPC_EVENT_ERROR;
					}
				}
				else if(nread == 0)
				{
					if(dbg_file)
					{
						//aupnpc_print(dbg_file, "[%s, %d]EOF, data received len(%d)\n", __FUNCTION__, __LINE__, len);
					}
					break;    /* EOF */
				}
				nleft -= nread;
				pMsg += nread;
				len += nread;

				if(dbg_file)
				{
					//aupnpc_print(dbg_file, "[%s, %d]Total received len(%d)\n", __FUNCTION__, __LINE__, len);
				}
				if(len > msg_max_len)
				{
					if(dbg_file)
					{
						aupnpc_print(dbg_file, "[%s, %d]Total received len(%d) > msssage max len(%d)\n", __FUNCTION__, __LINE__, len, msg_max_len);
					}
					goto READ_IPC_EVENT_ERROR;
				}
			}
			//AUPNPC_IPC_DBG("Read msg: %s\n", msg);
		}
		else
		{
			if(dbg_file)
			{
				aupnpc_print(dbg_file, "[%s, %d]Poll socket exception revents 0x%x!\n", __FUNCTION__, __LINE__, monitor_fd.revents);
			}
			goto READ_IPC_EVENT_ERROR;
		}
	}

	return 0;

READ_IPC_EVENT_ERROR:
	return -1;
}

/**
 * @brief Send message to IPC socket
 *
 * @param fd Socket file descriptor
 * @param msg Messages are be sent
 * @param timeout Sending socket timeout. The time unit is millisecond
 * @return int Sending socket result
 *      0: Send successfully
 *      -1: Send error
 */
static int _send_ipc_msg(const int fd, const char *msg, const size_t len, const int timeout, const char *dbg_file) // timeout millisecond.
{
	if(strlen(msg) == 0)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]the size of msg is equal to 0\n", __FUNCTION__, __LINE__);
		}
		goto SEND_IPC_EVENT_ERROR;
	}

	int length, ret;
	struct pollfd monitor_fd;
	monitor_fd.fd = fd;
	monitor_fd.events = POLLOUT;

	ret = poll(&monitor_fd, 1, timeout);

	if(ret == -1)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Poll socket error! %s\n", __FUNCTION__, __LINE__, strerror(errno));
		}
		goto SEND_IPC_EVENT_ERROR;
	}
	else if(ret == 0)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Write Socket timeout! %d\n", __FUNCTION__, __LINE__, strerror(errno));
		}
		goto SEND_IPC_EVENT_ERROR;
	}
	else
	{
		if(monitor_fd.revents & POLLOUT)
		{
			if(dbg_file)
			{
				//aupnpc_print(dbg_file, "[%s, %d]Write msg\n", __FUNCTION__, __LINE__);
			}
			length = write(fd, msg, len);
			if(length < 0)
			{
				if(dbg_file)
				{
					aupnpc_print(dbg_file, "[%s, %d]Writing Socket error! %s\n", __FUNCTION__, __LINE__, strerror(errno));
				}
				goto SEND_IPC_EVENT_ERROR;
			}
		}
		else
		{
			if(dbg_file)
			{
				aupnpc_print(dbg_file, "[%s, %d]Poll socket exception revents 0x%x!\n", __FUNCTION__, __LINE__, monitor_fd.revents);
			}
			goto SEND_IPC_EVENT_ERROR;
		}
	}

	return 0;

SEND_IPC_EVENT_ERROR:
	return -1;
}

/**
 * @brief Send a message to remote server and receive a message from remote server
 *
 * @param ipc_socket_path IPC socket path
 * @param msg Message will be sent
 * @param rsp_msg Messages will be received
 * @param rsp_msg_max_len Max length of respond message
 * @param timeout Sending message and receiving message time out. The time unit is millisecond
 * @return int Send message and receive response message result
 *      0: Success
 *      -1: Error
 */
int send_msg_to_ipc_socket(const char *ipc_socket_path, const char *msg, const size_t msg_len, char *rsp_msg, const int rsp_msg_max_len, const int timeout, const char *dbg_file)
{
	int fd = -1;
	int ret = -1;

	if(strlen(msg) == 0)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]the size of msg is equal to 0\n", __FUNCTION__, __LINE__);
		}
		goto SEND_MSG_ERROR;
	}

	fd = _connect_ipc(ipc_socket_path, dbg_file);
	if(fd == -1)
	{
		if(dbg_file)
		{
			//aupnpc_print(dbg_file, "[%s, %d]Connect to %s\n", __FUNCTION__, __LINE__, ipc_socket_path);
		}
		goto SEND_MSG_ERROR;
	}

	ret = _send_ipc_msg(fd, msg, msg_len, timeout, dbg_file);
	if(ret)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Send msg error\n", __FUNCTION__, __LINE__);
		}
		goto SEND_MSG_ERROR;
	}

	ret = _read_ipc_msg(fd, rsp_msg, rsp_msg_max_len, timeout, dbg_file);
	if(ret)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Read msg error\n", __FUNCTION__, __LINE__);
		}
		goto SEND_MSG_ERROR;
	}
	if(dbg_file)
	{
		//aupnpc_print(dbg_file, "[%s, %d]Response msg: %s\n", __FUNCTION__, __LINE__, rsp_msg);
	}

	ret = 0;

SEND_MSG_ERROR:
	_close_ipc(fd, dbg_file);
	return ret;
}

/**
 * @brief Read a message from remote server and send a message to remote server
 *
 * @param fd Socket file descriptor
 * @param msg Messages will be received
 * @param msg_max_len Max length of message
 * @param rsp_msg Response message will be sent
 * @param timeout Sending message and receiving message time out. The time unit is millisecond
 * @return int Receive message and send response message result
 *      0: Success
 *      -1: Error
 */
int read_msg_from_ipc_socket(const int fd, char *msg, const int msg_max_len, const char *rsp_msg, const size_t rsp_msg_len, const int timeout, const char *dbg_file)
{
	int ret = -1;

	if(strlen(rsp_msg) == 0)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]The size of response_msg is equal to 0\n", __FUNCTION__, __LINE__);
		}
		goto READ_MSG_ERROR;
	}

	ret = _read_ipc_msg(fd, msg, msg_max_len, timeout, dbg_file);
	if(ret)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Read msg error\n", __FUNCTION__, __LINE__);
		}
		goto READ_MSG_ERROR;
	}
	if(dbg_file)
	{
		//aupnpc_print(dbg_file, "[%s, %d]Read msg\n", __FUNCTION__, __LINE__);
	}

	ret = _send_ipc_msg(fd, rsp_msg, rsp_msg_len, timeout, dbg_file);
	if(ret)
	{
		if(dbg_file)
		{
			aupnpc_print(dbg_file, "[%s, %d]Send response msg error\n", __FUNCTION__, __LINE__);
		}
		goto READ_MSG_ERROR;
	}
	ret = 0;

READ_MSG_ERROR:
	_close_ipc(fd, dbg_file);
	return ret;
}


static int _evl_append(EVENT_LIST **head, EVENT_LIST *ev)
{
	EVENT_LIST *p;

	if(!head || !ev)
	{
		return -1;
	}

	if(*head == NULL)
	{
		*head = ev;
	}
	else
	{
		p = *head;
		while(p->next)
		{
			p = p->next;
		}
		p->next = ev;
	}
	return 0;
}

int evq_push_msg(EVENT_LIST **head, AUPNPC_MSG *msg)
{
	EVENT_LIST *ev = NULL;

	if(!head || !msg)
	{
		return -1;
	}

	ev = calloc(1, sizeof(EVENT_LIST));
	if(!ev)
	{
		return -1;
	}

	ev->data = msg;
	return _evl_append(head, ev);
}

static EVENT_LIST *_evl_pop(EVENT_LIST **head)
{
	EVENT_LIST *ev = NULL;

	if(!head)
	{
		return NULL;
	}

	ev = *head;
	if((*head)->next)
	{
		*head = (*head)->next;
	}
	else
	{
		*head = NULL;
	}
	return ev;
}

AUPNPC_MSG *evq_pop_msg(EVENT_LIST **head)
{
	EVENT_LIST *ev = NULL;
	AUPNPC_MSG *msg = NULL;

	if(!head)
	{
		return NULL;
	}

	ev = _evl_pop(head);
	if(ev)
	{
		msg = ev->data;
		SAFE_FREE(ev);
		return msg;
	}
	return NULL;
}

