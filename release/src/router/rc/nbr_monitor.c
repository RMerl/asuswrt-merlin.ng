#include "shared.h"
#include <json.h>
#include <sys/un.h>
static void nbr_ipc_receive(int sockfd)
{
	int length = 0;
	char buf[2048];
	memset(buf, 0, sizeof(buf));
	if ((length = read(sockfd, buf, sizeof(buf))) <= 0)
	{
		printf("ipc read socket error!\n");
		return;
	}

	_dprintf("IPC Receive: %s <<< RCV EVENT %d >>>\n", buf,uptime());

	json_object *rootObj = json_tokener_parse(buf);
	json_object *cfgObj = NULL;
	json_object *eidObj = NULL;
	json_object_object_get_ex(rootObj, CFG_PREFIX, &cfgObj);
	json_object_object_get_ex(cfgObj, NBR_EVENT_ID, &eidObj);

	//int EID = 0;
	//struct eventHandler *handler = NULL;

	if(eidObj) {
		wl_set_nbr_info();
	}

	json_object_put(rootObj);
}

static int nbr_start_ipc_socket(void)
{

#if defined(RTCONFIG_RALINK_MT7621)    
	Set_RAST_CPU();
#endif	
	struct sockaddr_un addr;
	int sockfd, newsockfd;

	if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		_dprintf("ipc create socket error!\n");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, NBR_IPC_SOCKET_PATH, sizeof(addr.sun_path)-1);

	unlink(NBR_IPC_SOCKET_PATH);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		_dprintf("ipc bind socket error!\n");
		exit(-1);
	}

	if (listen(sockfd, RAST_IPC_MAX_CONNECTION) == -1) {
		_dprintf("ipc listen socket error!\n");
		exit(-1);
	}

	while (1) {
		_dprintf("ipc accept socket...\n");
		if ( (newsockfd = accept(sockfd, NULL, NULL)) == -1) {
			_dprintf("ipc accept socket error!\n");
			continue;
		}

		nbr_ipc_receive(newsockfd);
		close(newsockfd);
	}

	return 0;
}

int nbr_monitor_main(int argc, char *argv[])
{
	//int rrm_nbr_count=0;
	//int rrm_nbr_init=0;

/*
	while(1){
		if(rrm_nbr_init == 0){
			rrm_nbr_init = 1;
			sleep(240);
		}

		if(rrm_nbr_count < 5) {
			rrm_nbr_count++;
			sleep(10);
		} else {
			rrm_nbr_count = 0;
			wl_set_nbr_info();
		}
	}
*/

	nbr_start_ipc_socket();
}