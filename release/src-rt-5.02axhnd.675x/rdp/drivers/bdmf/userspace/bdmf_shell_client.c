/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


/*******************************************************************
 * bdmf_mon_client.c
 *
 * BDMF framework - remote shell client
 *
 * This file is compiled into independent application
 *******************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h> /* to get htonl, ntohl */
#include <arpa/inet.h>  /* to get inet_aton and friends */
#include <netdb.h>
#include <pthread.h>
#include <termios.h>
#ifndef BDMF_SYSTEM_SIM
#include "bdmf_chrdev.h"
#endif

/* #define DEBUG */
#ifdef DEBUG
#define dprintf(fmt, ...)  printf("%s#%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define dprintf(...)
#endif


static int client_sock;
static pthread_t rx_thread;
static pthread_t cli_output_thread;
static int rx_thread_killed;
static int serial_file;         /* serial file handle */
static int session_id = -1;     /* serial session id */
static int cmd_rc;
struct termios orig_termios;

#define BDMF_SERIAL_FILE_NAME "/dev/bdmf_shell"
#define BDMF_MAX_LINE_LENGTH  2048

/* Receive handler */
static void *bdmfmons_client_thread_handler(void *arg)
{
    char ch;

    while (recv(client_sock, &ch, 1, MSG_WAITALL) > 0)
        putchar(ch);

    rx_thread_killed = 1;
    return NULL;
}

static void *bdmfmons_cli_output_thread_handler(void *arg)
{
    char ch;
    int rc;

    while ((rc = read(serial_file, &ch, 1)) > 0)
    {
        putchar(ch);
        fflush(stdout);
    }

    printf("CLI output thread tereminated. rc=%d errno=%s(%d)\n",
        rc, strerror(errno), errno);

    return NULL;
}

static int print_help(int print_usage, char *msg, ...)
    __attribute__((format(printf, 2, 3)));
static int print_help(int print_usage, char *msg, ...)
{
    if (msg)
    {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
    }
    if (print_usage)
    {
        fprintf(stderr, "Usage:\n"
                "\tbdmf_shell server_addr:port [options]\n"
                "\tbdmf_shell domain_socket_addr [options]\n"
                "\tbdmf_shell [-c init | session_id] [-cmd command] [-f script_file] [-fg] [-close]\n"
                "\t\t-c : serial communication. Must be followed by \"init\" or session_id\n"
                "\t\t-cmd command : send BDMF shell command\n"
                "\t\t-f script_file : send script_file content\n"
                "\t\t-fg - interactive mode\n"
                "\t\t-close - close session opened by \"-c init\" - used with \"-c\"\n"
        );
    }
    return -1;
}

static int open_socket_channel(int protocol, struct sockaddr *sa, int len)
{
    int rc;
    char ch;

    /* Create and connect socket */
    client_sock = socket(protocol, SOCK_STREAM, 0);
    if ((client_sock < 0))
    {
        perror("socket");
        exit(-3);
    }
    if (connect(client_sock, sa, len)<0)
    {
        perror("connect");
        exit(-4);
    }

    /* initiate conversation in order to bind */
    ch = '\n';
    if (send(client_sock, &ch, 1, 0)<=0)
        return print_help(0, "failed to initiate conversation\n");

    /* connected and ready.
     * Now
     * - create rx thread that will wait for rx from the socket and print to stdout
     * - start reading from stdin and sending to the socket
     */
    rc = pthread_create(&rx_thread, NULL, bdmfmons_client_thread_handler, NULL);
    if (rc)
        return print_help(0, "failed to create rx thread\n");
    return 0;
}


static int init_serial_session(void)
{
#ifndef BDMF_SYSTEM_SIM
    struct io_param iop = {};
    /* Open new serial session */
    if (ioctl(serial_file, BDMF_CHRDEV_SESSION_INIT, &iop) == -1)
    {
        perror("init session");
        return -1;
    }
    session_id = iop.session_id;
    printf("Session: %d\n", session_id);
#endif
    return session_id;
}

static int close_serial_session(void)
{
#ifndef BDMF_SYSTEM_SIM
    struct io_param iop;
    if (session_id < 0)
        return print_help(0, "Can't close session. session_id is missing\n");
    /* Close serial session */
    iop.session_id = session_id;
    if (ioctl(serial_file, BDMF_CHRDEV_SESSION_CLOSE, &iop) == -1)
    {
        perror("close session");
        return -1;
    }
    session_id = -1;
#endif
    return 0;
}

static int send_to_session(char *sbuf)
{
    int rc;
#ifndef BDMF_SYSTEM_SIM
    if (serial_file)
    {
        struct io_param iop;
        /* serial communication */
        if (session_id < 0)
            return print_help(0, "send: Can't send command over serial. session_id is missing\n");
        iop.session_id = session_id;
        strncpy(iop.command, sbuf, sizeof(iop.command));
        rc = ioctl(serial_file, BDMF_CHRDEV_SESSION_SEND, &iop);
        if (!rc)
            cmd_rc = iop.rc;
    }
    else
#endif
    {
        int len = strlen(sbuf);
        /* socket communication */
        if ((rc=send(client_sock, sbuf, len, 0)) != len)
            rc = print_help(0, "send: failed to send command\n");
    }
    return rc;
}

static int putc_to_session(char c)
{
    int rc;
#ifndef BDMF_SYSTEM_SIM
    if (serial_file)
    {
        /* serial communication */
        rc = write(serial_file, &c, 1);
        rc = (rc == 1) ? 0 : ((rc < 0) ? rc : -EINVAL);
    }
    else
#endif
    {
        return -EOPNOTSUPP;
    }
    return rc;
}


/* Raw mode: 1960 magic shit. */
static int terminal_set_raw_mode(void)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    {
        printf("tcgetattr failed. errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | IGNBRK | ICRNL | INPCK | ISTRIP | IXON | IXOFF);

    /* output modes - disable post processing */
    /* raw.c_oflag &= ~(OPOST); */

    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8 /* | ISIG */);

    /* local modes - echoing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0)
    {
        printf("tcsetattr failed. errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

static void terminal_restore_mode(void)
{
    /* Don't even check the return value as it's too late. */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}


int main(int argc, char *argv[])
{
    /* options */
    int is_serial = 0;      /* -c option */
    int is_init = 0;        /* -c init */
    char *cmd = NULL;       /* -cmd buffer */
    char *script_file = NULL;
    int is_interactive = 0; /* -fg option */
    int is_close = 0;       /* close session */
    int is_lineedit_mode = 0; /* line-edit mode. set automatically if -fg and CONFIG_LINENOISE is set */

    char *addr;
    char *cport;
    int protocol = 0;
    struct sockaddr_un domain_sa;
    struct sockaddr_in tcp_sa;
    struct sockaddr *sa = NULL;
    int len = 0;
    FILE *cmdf = NULL;   /* script file handle */
    int i;
    int rc;
    char *pend;
    char sbuf[BDMF_MAX_LINE_LENGTH] = { 0 };

    if (argc < 2)
        return print_help(1, NULL);

    /* 1st determine if it is serial or socket session */
    if (!strcmp(argv[1], "-c"))
    {
#ifdef BDMF_SYSTEM_SIM
        return print_help(0, "Serial connection is not supported in simulation\n");
#endif
        is_serial = 1;
        /* must be followed by "init" or session_id */
        if (argc < 3)
            return print_help(0, "-c must be followed by \"init\" or numerical session_id\n");
        if (!strcmp(argv[2], "init"))
            is_init = 1;
        else
        {
            session_id = strtol(argv[2], &pend, 0);
            if ((argc < 4) || (session_id <= 0) || (pend && *pend))
                return print_help(1, "Invalid session_id %s after -c or too few parameters\n", argv[2]);
        }
        i = 3;
    }
    else
    {
        /* socket communication */
        addr = argv[1];
        if ((cport=strchr(addr, ':')))
        {
            struct addrinfo hints, *res;
            int rc;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET;
            *(cport++) = 0; /* separate address and port parts */
            rc = getaddrinfo(addr, NULL, &hints, &res);
            if (rc)
            {
                fprintf(stderr, "Unknown host: %s\n", addr);
                return -3;
            }

            sa = (struct sockaddr *)&tcp_sa;
            tcp_sa.sin_family = AF_INET;
            tcp_sa.sin_port = htons(atoi(cport));
            protocol = AF_INET;
            tcp_sa.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
            len=sizeof(struct sockaddr_in);
            freeaddrinfo(res);
        }
        else
        {
            sa = (struct sockaddr *)&domain_sa;
            domain_sa.sun_family = AF_UNIX;  /* local is declared before socket() ^ */
            strncpy(domain_sa.sun_path, addr, sizeof(domain_sa.sun_path));
            len = strlen(domain_sa.sun_path) + sizeof(domain_sa.sun_family);
            protocol = AF_UNIX;
        }
        i = 2;
    }
    /* Parse options */
    for(; i<argc; i++)
    {
        if (!strcmp(argv[i], "-f"))
        {
            ++i;
            if (i == argc)
                return print_help(0, "Script file name expected after -f\n");
            script_file = argv[i];
        }
        else if (!strcmp(argv[i], "-fg"))
            is_interactive = 1;
        else if (!strcmp(argv[i], "-close"))
        {
            if (!is_serial)
                return print_help(0, "-close option can be used only for serial session\n");
            is_close = 1;
        }
        else if (!strcmp(argv[i], "-cmd"))
        {
            /* take the rest of the string as command */
            ++i;
            if (i == argc)
                return print_help(0, "Command is expected after -cmd\n");
            cmd = sbuf;
            for(; i<argc; i++)
            {
                strncat(cmd, argv[i], sizeof(sbuf)-1);
                strncat(cmd, " ", sizeof(sbuf)-1);
            }
            /* replace the last space to \n */
            cmd[strlen(cmd)-1] = '\n';
        }
    }

    /* open serial file if serial session */
    if (is_serial)
    {
        serial_file = open(BDMF_SERIAL_FILE_NAME, O_RDWR);
        if (serial_file <= 0)
            return print_help(0, "Can't open serial file %s\n", BDMF_SERIAL_FILE_NAME);
    }
    else
    {
        rc = open_socket_channel(protocol, sa, len);
        if (rc)
            return rc;
    }

    /* init serial session if needed */
    if (is_init && (rc = init_serial_session())<0)
        goto cleanup;

    /* send command if any */
    if (cmd && (rc=send_to_session(cmd)))
        goto cleanup;

    /* open script file if any */
    if (script_file)
    {
        cmdf = fopen(script_file, "r");
        if (!cmdf)
        {
            rc = print_help(0, "Can't open script file %s\n", script_file);
            goto cleanup;
        }
        while (fgets(sbuf, sizeof(sbuf)-1, cmdf))
        {
            if ((rc=send_to_session(sbuf)))
                goto cleanup;
        }
    }

#define CHAR_EOT 0x04

    dprintf("serial_file=%d interactive=%d\n", serial_file, is_interactive);
    if (is_interactive)
    {
        /* Check if lineedit mode is supported. If not - roll-back to string mode */
        if (serial_file)
        {
            rc = terminal_set_raw_mode();
            dprintf("terminal_set_raw_mode() --> %d\n", rc);
            if (!rc)
            {
                rc = write(serial_file, "\n", 1);
                is_lineedit_mode = (rc == 1);
                dprintf("write() --> %d, lineedit_mode=%d\n", rc, is_lineedit_mode);
                if (!is_lineedit_mode)
                {
                    printf("switching lineedit off. rc=%d\n", rc);
                    terminal_restore_mode();
                }
            }
        }
        if (is_lineedit_mode)
        {
            int c = 0;
            dprintf("Line-edit mode\n");
            rc = pthread_create(&cli_output_thread, NULL, bdmfmons_cli_output_thread_handler, NULL);
            if (rc)
                return print_help(0, "failed to create cli_output thread\n");
            while(!rx_thread_killed && (c = getchar()) >= 0)
            {
                /* Some terminals behave strangely. getchar() can return 0 character for no obvious reason */
                if (!c)
                    continue;
                if (putc_to_session(c))
                    goto cleanup;
            }
            dprintf("rx_thread_killed=%d c=%d\n", rx_thread_killed, c);
        }
        else
        {
            printf("Legacy mode. tab completion is not supported\n");
            send_to_session("\n"); /* provoke printing prompt */
            while(!rx_thread_killed && fgets(sbuf, sizeof(sbuf)-1, stdin))
            {
                if (send_to_session(sbuf))
                    goto cleanup;
            }
        }
    }
    else if (!is_serial)
    {
        char eot[2]={CHAR_EOT, 0};
        send_to_session(eot);
        while(!rx_thread_killed)
            ;
    }

    if (is_serial && is_close)
        close_serial_session();

cleanup:
    if (is_lineedit_mode)
    {
        void *res;
        terminal_restore_mode();
        pthread_cancel(cli_output_thread);
        pthread_join(cli_output_thread, &res);
    }
    if (serial_file)
        close(serial_file);
    if (cmdf)
        fclose(cmdf);
    if (client_sock)
        close(client_sock);

    return rc ? rc : cmd_rc;
}
