##include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <winsock2.h>
#include "win32_snprintf.h"
#else
/* for IPPROTO_TCP / IPPROTO_UDP */
#include <netinet/in.h>
#endif
#include <ctype.h>
#include "miniwget.h"
#include "miniupnpc.h"
#include "upnpcommands.h"
#include "portlistingparse.h"
#include "upnperrors.h"
#include "miniupnpcstrings.h"




void interrupt_handler(int sig)
{
    if(sig == SIGUSR1)
    {
		
    }
}


int main(int argc, char ** argv)
{
}

