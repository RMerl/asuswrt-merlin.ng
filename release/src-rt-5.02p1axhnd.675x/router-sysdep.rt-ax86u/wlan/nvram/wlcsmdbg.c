#include <fcntl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wlcsm_lib_netlink.h>
int running=1;
int listenfd,connfd,n;
struct sockaddr_in servaddr,cliaddr;
#define WLCSM_DEBUG_FILE_PATH ("/var/wlcsm_debug_file")

void debug_stophandler(int sig)
{
    if(running) {
        sendto(connfd,"<<END>>",8,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
    }
    wlcsm_register_process("wlnodebug");
    /* destroy pid file */
    unlink(WLCSM_DEBUG_FILE_PATH);
    running=0;
    raise(SIGKILL);
}

int main(int argc, char**argv)
{
    t_WLCSM_MSG_HDR *msghdr;
    int n;
    socklen_t clilen;
    FILE  *pidfile;
    int flag=1,i=0;
    int debug_sockid=0;
    struct pollfd pfds[2];
    FILE *tempfile;
    int dd=0;
    int reuseaddr=1;
    char buf[MAX_NLRCV_BUF_SIZE];
    char sbuf[MAX_NLRCV_BUF_SIZE];

    if((pidfile = fopen(WLCSM_DEBUG_FILE_PATH,"r"))) {
        fclose(pidfile);
        /* the daemon has been running,don't need to restart again */
        return -1;
    }

    /* create process file to indicate it is running */
    if ((pidfile = fopen(WLCSM_DEBUG_FILE_PATH, "w"))) {
        fprintf(pidfile, "%d\n", getpid());
        fclose(pidfile);
    }

    signal(SIGTERM, debug_stophandler);

    while (running) {
        listenfd=socket(AF_INET,SOCK_STREAM,0);
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
        servaddr.sin_port=htons(32000);
        setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr));
        bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
        listen(listenfd,1);
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
        close(listenfd);
        //debug_sockid=wlcsm_registerg_daemon((unsigned int)getpid());
        debug_sockid=wlcsm_register_process("wldebug");
        if(debug_sockid<=0) {
            printf("could not register debug socket\n");
            return -1;
        } else
            printf("JXUJXU:%s:%d  debug_sockid:%d \r\n",__FUNCTION__,__LINE__,debug_sockid );
        pfds[0].fd=debug_sockid;
        pfds[0].events=POLLIN; /* can read?? */

        pfds[1].fd=connfd;
        pfds[1].events=POLLIN; /* can read?? */

        while (running) {
            poll(pfds,2,-1); /* blocking for reading here from  both sockets */
            if(pfds[0].revents & POLLIN) { /* if recevied from debug msg sockets, send to cleint */
                msghdr=wlcsm_mngr_recv_mesg(sbuf);
                if(msghdr) {
                    sendto(connfd,(char *)(msghdr+1),msghdr->len,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
                }
            } else if(pfds[1].revents & POLLIN) {
                /* if recevied from client*/
                i=read(connfd,buf,1024);
                if(i>0) {
                    printf("received from client:  %s \r\n",buf);
                    if(!strncmp(buf,"<<SETDEBUG",10)) {
                        wlcsm_set_trace_level((buf+10));
                    } else if(!strncmp(buf,"<UPDATESYMBOL>",14)) {
                        fprintf(stderr,"JXUJXU:%s:%d  start get symbol \r\n",__FUNCTION__,__LINE__ );
                        tempfile=fopen("/proc/kallsyms","rt");
                        if(tempfile) {
                            sendto(connfd,"<STARTSYMBOL>",14,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
                            while(fgets(buf,MAX_NLRCV_BUF_SIZE,tempfile)!=NULL) {
				sleep(1);
                                sendto(connfd,buf,strlen(buf),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
                            }
                            sendto(connfd,"<ENDSYMBOL>",12,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
                            fprintf(stderr,"JXUJXU:%s:%d  close symbol, done \r\n",__FUNCTION__,__LINE__ );
                            fclose(tempfile);
                            tempfile=NULL;
                        }

                    } else if(!strncmp(buf,"<BYEBYE>",8)) break;
                }
            }

        }
        wlcsm_register_process("wlnodebug");
        close(connfd);
    }
    /* destroy pid file */
    unlink(WLCSM_DEBUG_FILE_PATH);
    return 0;
}


