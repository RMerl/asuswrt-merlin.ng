#ifndef __APPLE__
#define HAVE_MALLOC_H 1
#endif

#define ROOT_SYSCONFDIR "/etc"

#define ENABLE_LIBSPARSE 1

#define DISABLE_BACKTRACE 1
#define HAVE_DIRENT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_GETOPT_H 1
#define HAVE_GETPWUID_R 1
#define HAVE_INTPTR_T 1
#define HAVE_INTTYPES_H 1
#define HAVE_MMAP 1
#define HAVE_SETJMP_H 1
#define HAVE_SNPRINTF 1
#define HAVE_STDLIB_H 1
#define HAVE_STRCASECMP 1
#define HAVE_STRDUP 1
#define HAVE_STRINGS_H 1
#define HAVE_STRNLEN 1
#define HAVE_STRPTIME 1
#define HAVE_SYSCONF 1
#define HAVE_TYPE_SSIZE_T 1
#define HAVE_UNISTD_H 1
#define HAVE_UTIME_H 1

#define HAVE_SYS_STAT_H 1
#if !defined(__APPLE__)
# define HAVE_SYS_SYSMACROS_H 1
#endif
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1

#if defined(_WIN32)
# define HAVE_LINUX_TYPES_H 1
# define HAVE_WINSOCK_H 1
#endif
#if defined(__APPLE__) || defined(__linux__)
# define HAVE_FCNTL 1
# define HAVE_FSYNC 1
# define HAVE_GETPAGESIZE 1
# define HAVE_NET_IF_H 1
# define HAVE_NETINET_IN_H 1
# define HAVE_PREAD 1
# define HAVE_PWRITE 1
# define HAVE_POSIX_MEMALIGN 1
# define HAVE_SYS_IOCTL_H 1
# define HAVE_SYS_MMAN_H 1
# define HAVE_SYS_MOUNT_H 1
# define HAVE_SYS_PARAM_H 1
# define HAVE_SYS_RESOURCE_H 1
# define HAVE_SYS_SELECT_H 1
# define HAVE_SYS_WAIT_H 1
#endif
#if defined(__linux__)
# define HAVE_EXT2_IOCTLS 1
# define HAVE_FALLOCATE 1
# define HAVE_LINUX_FD_H 1
# define HAVE_LINUX_TYPES_H 1
# define HAVE_LSEEK64 1
# define HAVE_LSEEK64_PROTOTYPE 1
# define HAVE_MNTENT_H 1
# define HAVE_PREAD64 1
# define HAVE_PWRITE64 1
# define HAVE_SETMNTENT 1
# define HAVE_SYS_PRCTL_H 1
#endif
