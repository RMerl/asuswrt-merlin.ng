#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifndef _WIN32_WCE
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#endif /* _WIN32_WCE */
#include <ctype.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif /* _MSC_VER */

#ifndef CONFIG_NATIVE_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef __vxworks
#include <sys/uio.h>
#include <sys/time.h>
#endif /* __vxworks */
#endif /* CONFIG_NATIVE_WINDOWS */

#endif /* INCLUDES_H */
