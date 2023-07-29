#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include <signal.h>


#include <shared.h>




#include "ipc.h"
#include "mnt.h"



int main( int argc, char ** argv )
{

    // ipc start
    awsiot_ipc_start();


    int i = 0;

    while(1) {

    	i++;

    	sleep(2);

	    char *ipc_send_msg = "{   \"topic\" : \"sip_tunnel\" ,   \"msg\" : \"{\\\"aae_sip_connected\\\":\\\"1\\\"}\"  }";


	    cm_sendIpcHandler(AWSIOT_IPC_SOCKET_PATH, ipc_send_msg, strlen(ipc_send_msg));
	    

    	printf("waiting 30s, next msg send\n\n");
	    sleep(30);


    }

    return 0;
}

/*-----------------------------------------------------------*/
