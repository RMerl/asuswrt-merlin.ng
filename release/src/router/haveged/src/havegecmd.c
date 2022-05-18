/**
 ** Provide HAVEGE socket communication API
 **
 ** Copyright 2018-2022 Jirka Hladky hladky DOT jiri AT gmail DOT com
 ** Copyright 2018 Werner Fink <werner@suse.de>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 */

#include "config.h"

#include <stddef.h>
#include <stdint.h>

#ifndef NO_COMMAND_MODE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#ifndef HAVE_STRUCT_UCRED
struct ucred
{
   pid_t pid;                      /* PID of sending process.  */
   uid_t uid;                      /* UID of sending process.  */
   gid_t gid;                      /* GID of sending process.  */
};
#endif

#include "havegecmd.h"

int first_byte;
int socket_fd;
static char errmsg[1024];

static int new_root(               /* RETURN: status                        */
   const char *root,               /* IN: path of the new root file system  */
   const volatile char *path,      /* IN: path of the haveged executable    */
   char *const argv[],             /* IN: arguments for the haveged process */
   struct pparams *params)         /* IN: input params                      */
{
   int ret;
   struct stat st;
   const char *realtive = (const char*)&path[0];

   ret = chdir(root);
   if (ret < 0) {
      snprintf(&errmsg[0], sizeof(errmsg)-1,
               "can't change to working directory %s: %s\n",
               root, strerror(errno));
      goto err;
      }
   if (path[0] == '/')
      realtive++;
   ret = fstatat(AT_FDCWD, realtive, &st, 0);
   if (ret < 0) {
      snprintf(&errmsg[0], sizeof(errmsg)-1,
               "can't restart %s: %s\n",
               path, strerror(errno));
      goto err;
      }
   ret = chroot(".");
   if (ret < 0) {
      snprintf(&errmsg[0], sizeof(errmsg)-1,
               "can't change root directory %s\n",
               strerror(errno));
      goto err;
      }
   ret = chdir("/");
   if (ret < 0) {
      snprintf(&errmsg[0], sizeof(errmsg)-1,
               "can't change to working directory / : %s\n",
               strerror(errno));
      goto err;
      }
   ret = execv((const char *)path, argv);
   if (ret < 0) {
      snprintf(&errmsg[0], sizeof(errmsg)-1,
               "can't restart %s: %s\n",
               path, strerror(errno));
   }   
err:
   if (ret < 0)
      print_msg("%s", errmsg);
   return ret;
}

/**
 * Open and listen on a UNIX socket to get command from there
 */
int cmd_listen(                    /* RETURN: UNIX socket file descriptor */
   struct pparams *params)         /* IN: input params                    */
{
   struct sockaddr_un su = {       /* The abstract UNIX socket of haveged */
      .sun_family = AF_UNIX,
      .sun_path = HAVEGED_SOCKET_PATH,
   };
   const int one = 1;
   int fd, ret;

   fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
   if (fd < 0) {
      print_msg("%s: can not open UNIX socket\n", params->daemon);
      goto err;
      }

   ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, (socklen_t)sizeof(one));
   if (ret < 0) {
      close(fd);
      fd = -1;
      print_msg("%s: can not set option for UNIX socket\n", params->daemon);
      goto err;
      }

   ret = bind(fd, (struct sockaddr *)&su, offsetof(struct sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
   if (ret < 0) {
      close(fd);
      fd = -1;
      if (errno != EADDRINUSE)
         print_msg("%s: can not bind a name to UNIX socket\n", params->daemon);
      else
         fd = -2;
      goto err;
      }

   ret = listen(fd, SOMAXCONN);
   if (ret < 0) {
      close(fd);
      fd = -1;
      print_msg("%s: can not listen on UNIX socket\n", params->daemon);
      goto err;
      }
err:
   return fd;
}

/**
 * Open and connect on a UNIX socket to send command over this
 */
int cmd_connect(                   /* RETURN: UNIX socket file descriptor */
   struct pparams *params)         /* IN: input params                    */
{
   struct sockaddr_un su = {       /* The abstract UNIX socket of haveged */
      .sun_family = AF_UNIX,
      .sun_path = HAVEGED_SOCKET_PATH,
      };
   const int one = 1;
   int fd, ret;

   fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
   if (fd < 0) {
      print_msg("%s: can not open UNIX socket\n", params->daemon);
      goto err;
      }

   ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, (socklen_t)sizeof(one));
   if (ret < 0) {
      print_msg("%s: can not set option for UNIX socket\n", params->daemon);
      close(fd);
      fd = -1;
      goto err;
      }

   ret = connect(fd, (struct sockaddr *)&su, offsetof(struct sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
   if (ret < 0) {
      if (errno != ECONNREFUSED)
         print_msg("%s: can not connect on UNIX socket\n", params->daemon);
      close(fd);
      fd = -1;
      goto err;
      }
err:
   return fd;
}

/**
 * Handle arguments in command mode
 */
int getcmd(                        /* RETURN: success or error      */
   char *arg)                      /* handle current known commands */
{
   static const struct {
      const char* cmd;
      const int req;
      const int arg;
      const char* opt;
   } cmds[] = {
      { "root=",      MAGIC_CHROOT, 1, NULL },      /* New root */
      { "close",      MAGIC_CLOSE,  0, NULL },      /* Close socket */
      {0}
   }, *cmd = cmds;
   int ret = -1;

   if (!arg || !*arg)
      goto err;

   optarg = NULL;
   for (; cmd->cmd; cmd++)
      if (cmd->arg) {
         if (strncmp(cmd->cmd, arg, strlen(cmd->cmd)) == 0) {
            optarg = strchr(arg, '=');
            optarg++;
            ret = cmd->req;
            break;
            }
         }
      else {
         if (strcmp(cmd->cmd, arg) == 0) {
            ret = cmd->req;
            break;
            }
         }
err:
   return ret;
}

/**
 * Handle incomming messages from socket
 */
int socket_handler(                /* RETURN: closed file descriptor        */
   int fd,                         /* IN: connected socket file descriptor  */
   const volatile char *path,      /* IN: path of the haveged executable    */
   char *const argv[],             /* IN: arguments for the haveged process */
   struct pparams *params)         /* IN: input params                      */
{
   struct ucred cred = {0};
   unsigned char magic[2], *ptr;
   char *enqry;
   char *optarg = NULL;
   socklen_t clen;
   int ret = -1, len;

   if (fd < 0) {
      print_msg("%s: no connection jet\n", params->daemon);
      }

   ptr = &magic[0];
   len = sizeof(magic);
   ret = safein(fd, ptr, len);
   if (ret < 0) {
      print_msg("%s: can not read from UNIX socket\n", params->daemon);
      goto out;
      }

   if (magic[1] == '\002') {       /* ASCII start of text: read argument provided */
      uint32_t alen;

      ret = receive_uinteger(fd, &alen);
      if (ret < 0) {
         print_msg("%s: can not read from UNIX socket\n", params->daemon);
         goto out;
         }

      optarg = calloc(alen, sizeof(char));
      if (!optarg) {
          print_msg("can not allocate memory for message from UNIX socket");
          goto out;
          }
      ptr = (unsigned char*)optarg;

      ret = safein(fd, ptr, alen);
      if (ret < 0) {
         print_msg("%s: can not read from UNIX socket\n", params->daemon);
         goto out;
         }
      }

   clen = sizeof(struct ucred);
   ret = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &clen);
   if (ret < 0) {
      print_msg("%s: can not get credentials from UNIX socket part1\n", params->daemon);
      goto out;
      }
   if (clen != sizeof(struct ucred)) {
      print_msg("%s: can not get credentials from UNIX socket part2\n", params->daemon);
      goto out;
      }
   if (cred.uid != 0) {
      enqry = ASCII_NAK;

      ptr = (unsigned char *)enqry;
      len = (int)strlen(enqry)+1;
      safeout(fd, ptr, len);
      }

   switch (magic[0]) {
      case MAGIC_CHROOT:
         enqry = ASCII_ACK;

         ret = new_root(optarg, path, argv, params);
         if (ret < 0) {
            uint32_t size = strlen(errmsg);
            safeout(fd, ASCII_STX, strlen(ASCII_STX));
            send_uinteger(fd, size);
            safeout(fd, errmsg, size+1);
            break;
         }

         ptr = (unsigned char *)enqry;
         len = (int)strlen(enqry);
         safeout(fd, ptr, len);

         break;
      case MAGIC_CLOSE:
         enqry = ASCII_ACK;

         close(socket_fd);
         socket_fd = -1;

         ptr = (unsigned char *)enqry;
         len = (int)strlen(enqry);
         safeout(fd, ptr, len);
         argv[0][0] = first_byte;

         break;
      default:
         enqry = ASCII_NAK;

         ptr = (unsigned char *)enqry;
         len = (int)strlen(enqry);
         safeout(fd, ptr, len);
         break;
      }
out:
   if (optarg)
      free(optarg);
   if (fd > 0) {
      close(fd);
      fd = -1;
      }
   return fd;
}

/**
 * Receive incomming messages from socket
 */
ssize_t safein(                    /* RETURN: read bytes                    */
   int fd,                         /* IN: file descriptor                   */
   void *ptr,                      /* OUT: pointer to buffer                */
   size_t len)                      /* IN: size of buffer                    */
{
   int saveerr = errno, avail;
   ssize_t ret = 0;

   if (len > SSIZE_MAX)
      len = SSIZE_MAX;

   ret = ioctl(fd, FIONREAD, &avail);
   if (ret < 0 || avail <=0)
      goto out;

   if (len > (unsigned int) avail)
      len = avail; 

   do {
      errno = saveerr;
      ssize_t p = recv(fd, ptr, len, 0 /* MSG_DONTWAIT */);
      if (p < 0) {
         if (errno == EINTR)
            continue;
         if (errno == EAGAIN || errno == EWOULDBLOCK)
            break;
         print_msg("Unable to read from socket %d: %s", socket_fd, strerror(errno));
         }
      ptr = (char *) ptr + p;
      ret += p;
      len -= p;
      }
   while (len > 0);
out:
   return ret;
}

/**
 * Send outgoing messages to socket
 */
void safeout(                      /* RETURN: nothing                       */
   int fd,                         /* IN: file descriptor                   */
   const void *ptr,                /* IN: pointer to buffer                 */
   size_t len)                     /* IN: size of buffer                    */
{
   int saveerr = errno;

   do {
      ssize_t p = send(fd, ptr, len, MSG_NOSIGNAL);
      if (p < 0) {
         if (errno == EINTR)
                     continue;
         if (errno == EPIPE || errno == EAGAIN || errno == EWOULDBLOCK)
                     break;
         print_msg("Unable to write to socket %d: %s", fd, strerror(errno));
         }
       ptr = (char *) ptr + p;
       len -= p;
       }
   while (len > 0);

   errno = saveerr;
}

/**
 * Send outgoing unsigned integer to socket
 */
void send_uinteger(                /* RETURN: nothing                       */
   int fd,                         /* IN: file descriptor                   */
   uint32_t value)                 /* IN: 32 bit unsigned integer           */
{
   uint8_t buffer[4];

   buffer[0] = (uint8_t)((value >> 24) & 0xFF);
   buffer[1] = (uint8_t)((value >> 16) & 0xFF);
   buffer[2] = (uint8_t)((value >>  8) & 0xFF);
   buffer[3] = (uint8_t)((value >>  0) & 0xFF);

   safeout(fd, buffer, 4 * sizeof(uint8_t));
}

/**
 * Receive incomming unsigned integer from socket
 */
int receive_uinteger(              /* RETURN: status                        */
   int fd,                         /* IN: file descriptor                   */
   uint32_t *value)                /* OUT: 32 bit unsigned integer          */
{
   uint8_t buffer[4];

   if (safein(fd, buffer, 4 * sizeof(uint8_t)) < 0)
      return -1;

   *value = (((uint32_t)buffer[0]) << 24) |
            (((uint32_t)buffer[1]) << 16) |
            (((uint32_t)buffer[2]) <<  8) |
            (((uint32_t)buffer[3]) <<  0);

   return 0;
}

#endif
