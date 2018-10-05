#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int s;
    struct sockaddr_storage sa;
    socklen_t salen;
    uint16_t port;

    if ((s = socket(PF_INET6, SOCK_STREAM, 0)) < 0) {
        if (errno == EAFNOSUPPORT)
            s = socket(PF_INET, SOCK_STREAM, 0);

        if (s < 0) {
            perror("socket()");
            return 1;
        }
    }

    if (listen(s, 2) < 0) {
        perror("listen()");
        return 2;
    }

    salen = sizeof(sa);
    if (getsockname(s, (struct sockaddr*) &sa, &salen) < 0) {
        perror("getsockname()");
        return 3;
    }

    if (((struct sockaddr*) &sa)->sa_family == AF_INET)
        port = ((struct sockaddr_in*) &sa)->sin_port;
    else
        port = ((struct sockaddr_in6*) &sa)->sin6_port;

    printf("Selected port number %u\n", ntohs(port));

    /* ... hic sunt leones ... */

    sleep(60);

    return 0;
}
