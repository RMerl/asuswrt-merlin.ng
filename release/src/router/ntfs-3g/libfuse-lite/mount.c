/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

    This program can be distributed under the terms of the GNU LGPLv2.
    See the file COPYING.LIB.
*/

#include "config.h"
#include "fuse_i.h"
#include "fuse_opt.h"
#include "mount_util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/mount.h>

#ifdef __SOLARIS__

#define FUSERMOUNT_PROG         "fusermount"
#define FUSE_COMMFD_ENV         "_FUSE_COMMFD"

#ifndef FUSERMOUNT_DIR
#define FUSERMOUNT_DIR "/usr"
#endif /* FUSERMOUNT_DIR */

#ifndef HAVE_FORK
#define fork() vfork()
#endif

#endif /* __SOLARIS__ */

#ifndef MS_DIRSYNC
#define MS_DIRSYNC 128
#endif

enum {
    KEY_KERN_FLAG,
    KEY_KERN_OPT,
    KEY_FUSERMOUNT_OPT,
    KEY_SUBTYPE_OPT,
    KEY_MTAB_OPT,
    KEY_ALLOW_ROOT,
    KEY_RO,
    KEY_HELP,
    KEY_VERSION,
};

struct mount_opts {
    int allow_other;
    int allow_root;
    int ishelp;
    int flags;
#ifdef __SOLARIS__
    int nonempty;
    int blkdev;
    char *fsname;
    char *subtype;
    char *subtype_opt;
#else
    int blkdev;
    char *fsname;
#endif
    char *mtab_opts;
    char *fusermount_opts;
    char *kernel_opts;
};

#define FUSE_MOUNT_OPT(t, p) { t, offsetof(struct mount_opts, p), 1 }

static const struct fuse_opt fuse_mount_opts[] = {
#ifdef __SOLARIS__
    FUSE_MOUNT_OPT("allow_other",       allow_other),
    FUSE_MOUNT_OPT("allow_root",        allow_root),
    FUSE_MOUNT_OPT("nonempty",          nonempty),
    FUSE_MOUNT_OPT("blkdev",            blkdev),
    FUSE_MOUNT_OPT("fsname=%s",         fsname),
    FUSE_MOUNT_OPT("subtype=%s",        subtype),
    FUSE_OPT_KEY("allow_other",         KEY_KERN_OPT),
    FUSE_OPT_KEY("allow_root",          KEY_ALLOW_ROOT),
    FUSE_OPT_KEY("nonempty",            KEY_FUSERMOUNT_OPT),
    FUSE_OPT_KEY("blkdev",              KEY_FUSERMOUNT_OPT),
    FUSE_OPT_KEY("fsname=",             KEY_FUSERMOUNT_OPT),
    FUSE_OPT_KEY("subtype=",            KEY_SUBTYPE_OPT),
    FUSE_OPT_KEY("large_read",          KEY_KERN_OPT),
    FUSE_OPT_KEY("blksize=",            KEY_KERN_OPT),
    FUSE_OPT_KEY("default_permissions", KEY_KERN_OPT),
    FUSE_OPT_KEY("max_read=",           KEY_KERN_OPT),
    FUSE_OPT_KEY("max_read=",           FUSE_OPT_KEY_KEEP),
    FUSE_OPT_KEY("user=",               KEY_MTAB_OPT),
    FUSE_OPT_KEY("-r",                  KEY_RO),
    FUSE_OPT_KEY("ro",                  KEY_KERN_FLAG),
    FUSE_OPT_KEY("rw",                  KEY_KERN_FLAG),
    FUSE_OPT_KEY("suid",                KEY_KERN_FLAG),
    FUSE_OPT_KEY("nosuid",              KEY_KERN_FLAG),
    FUSE_OPT_KEY("-g",                  KEY_KERN_FLAG),
    FUSE_OPT_KEY("-m",                  KEY_KERN_FLAG),
    FUSE_OPT_KEY("-O",                  KEY_KERN_FLAG),
    FUSE_OPT_KEY("setuid",              KEY_KERN_OPT),
    FUSE_OPT_KEY("nosetuid",            KEY_KERN_OPT),
    FUSE_OPT_KEY("devices",             KEY_KERN_OPT),
    FUSE_OPT_KEY("nodevices",           KEY_KERN_OPT),
    FUSE_OPT_KEY("exec",                KEY_KERN_OPT),
    FUSE_OPT_KEY("noexec",              KEY_KERN_OPT),
    FUSE_OPT_KEY("nbmand",              KEY_KERN_OPT),
    FUSE_OPT_KEY("nonbmand",            KEY_KERN_OPT),
#else /* __SOLARIS__ */
    FUSE_MOUNT_OPT("allow_other",       allow_other),
    FUSE_MOUNT_OPT("allow_root",        allow_root),
    FUSE_MOUNT_OPT("blkdev",            blkdev),
    FUSE_MOUNT_OPT("fsname=%s",         fsname),
    FUSE_OPT_KEY("allow_other",         KEY_KERN_OPT),
    FUSE_OPT_KEY("allow_root",          KEY_ALLOW_ROOT),
    FUSE_OPT_KEY("blkdev",              KEY_FUSERMOUNT_OPT),
    FUSE_OPT_KEY("fsname=",             KEY_FUSERMOUNT_OPT),
    FUSE_OPT_KEY("large_read",          KEY_KERN_OPT),
    FUSE_OPT_KEY("blksize=",            KEY_KERN_OPT),
    FUSE_OPT_KEY("default_permissions", KEY_KERN_OPT),
    FUSE_OPT_KEY("context=",            KEY_KERN_OPT),
    FUSE_OPT_KEY("max_read=",           KEY_KERN_OPT),
    FUSE_OPT_KEY("max_read=",           FUSE_OPT_KEY_KEEP),
    FUSE_OPT_KEY("user=",               KEY_MTAB_OPT),
    FUSE_OPT_KEY("-r",                  KEY_RO),
    FUSE_OPT_KEY("ro",                  KEY_KERN_FLAG),
    FUSE_OPT_KEY("rw",                  KEY_KERN_FLAG),
    FUSE_OPT_KEY("suid",                KEY_KERN_FLAG),
    FUSE_OPT_KEY("nosuid",              KEY_KERN_FLAG),
    FUSE_OPT_KEY("dev",                 KEY_KERN_FLAG),
    FUSE_OPT_KEY("nodev",               KEY_KERN_FLAG),
    FUSE_OPT_KEY("exec",                KEY_KERN_FLAG),
    FUSE_OPT_KEY("noexec",              KEY_KERN_FLAG),
    FUSE_OPT_KEY("async",               KEY_KERN_FLAG),
    FUSE_OPT_KEY("sync",                KEY_KERN_FLAG),
    FUSE_OPT_KEY("dirsync",             KEY_KERN_FLAG),
    FUSE_OPT_KEY("atime",               KEY_KERN_FLAG),
    FUSE_OPT_KEY("noatime",             KEY_KERN_FLAG),
#endif /* __SOLARIS__ */
    FUSE_OPT_KEY("-h",                  KEY_HELP),
    FUSE_OPT_KEY("--help",              KEY_HELP),
    FUSE_OPT_KEY("-V",                  KEY_VERSION),
    FUSE_OPT_KEY("--version",           KEY_VERSION),
    FUSE_OPT_END
};

#ifdef __SOLARIS__

static void mount_help(void)
{
    fprintf(stderr,
            "    -o allow_other         allow access to other users\n"
            "    -o allow_root          allow access to root\n"
            "    -o nonempty            allow mounts over non-empty file/dir\n"
            "    -o default_permissions enable permission checking by kernel\n"
            "    -o fsname=NAME         set filesystem name\n"
            "    -o subtype=NAME        set filesystem type\n"
            "    -o large_read          issue large read requests (2.4 only)\n"
            "    -o max_read=N          set maximum size of read requests\n"
            "\n"
            );
}

static void exec_fusermount(const char *argv[])
{
    execv(FUSERMOUNT_DIR "/" FUSERMOUNT_PROG, (char **) argv);
    execvp(FUSERMOUNT_PROG, (char **) argv);
}

static void mount_version(void)
{
    int pid = fork();
    if (!pid) {
        const char *argv[] = { FUSERMOUNT_PROG, "--version", NULL };
        exec_fusermount(argv);
        _exit(1);
    } else if (pid != -1)
        waitpid(pid, NULL, 0);
}

#endif /* __SOLARIS__ */

struct mount_flags {
    const char *opt;
    unsigned long flag;
    int on;
};

static struct mount_flags mount_flags[] = {
    {"rw",      MS_RDONLY,      0},
    {"ro",      MS_RDONLY,      1},
    {"suid",    MS_NOSUID,      0},
    {"nosuid",  MS_NOSUID,      1},
#ifndef __SOLARIS__
    {"dev",     MS_NODEV,       0},
    {"nodev",   MS_NODEV,       1},
    {"exec",    MS_NOEXEC,      0},
    {"noexec",  MS_NOEXEC,      1},
    {"async",   MS_SYNCHRONOUS, 0},
    {"sync",    MS_SYNCHRONOUS, 1},
    {"atime",   MS_NOATIME,     0},
    {"noatime", MS_NOATIME,     1},
    {"dirsync", MS_DIRSYNC,     1},
#else /* __SOLARIS__ */
    {"-g",      MS_GLOBAL,      1},  /* 1eaf4 */
    {"-m",      MS_NOMNTTAB,    1},  /* 1eb00 */
    {"-O",      MS_OVERLAY,     1},  /* 1eb0c */
#endif /* __SOLARIS__ */
    {NULL,      0,              0}
};

#ifdef __SOLARIS__

/*
 * See comments in fuse_kern_mount()
 */
struct solaris_mount_opts {
    int nosuid;
    int setuid;
    int nosetuid;
    int devices;
    int nodevices;
};

#define SOLARIS_MOUNT_OPT(t, p, n) \
    { t, offsetof(struct solaris_mount_opts, p), n }
static const struct fuse_opt solaris_mnt_opts[] = {
    SOLARIS_MOUNT_OPT("suid",       setuid,         1),
    SOLARIS_MOUNT_OPT("suid",       devices,        1),
    SOLARIS_MOUNT_OPT("nosuid",     nosuid,         1),
    SOLARIS_MOUNT_OPT("setuid",     setuid,         1),
    SOLARIS_MOUNT_OPT("nosetuid",   nosetuid,       1),
    SOLARIS_MOUNT_OPT("devices",    devices,        1),
    SOLARIS_MOUNT_OPT("nodevices",  nodevices,      1),
    FUSE_OPT_END
};

#endif /* __SOLARIS__ */

static void set_mount_flag(const char *s, int *flags)
{
    int i;

    for (i = 0; mount_flags[i].opt != NULL; i++) {
        const char *opt = mount_flags[i].opt;
        if (strcmp(opt, s) == 0) {
            if (mount_flags[i].on)
                *flags |= mount_flags[i].flag;
            else
                *flags &= ~mount_flags[i].flag;
            return;
        }
    }
    fprintf(stderr, "fuse: internal error, can't find mount flag\n");
    abort();
}

static int fuse_mount_opt_proc(void *data, const char *arg, int key,
                               struct fuse_args *outargs)
{
    struct mount_opts *mo = data;

    switch (key) {
    case KEY_ALLOW_ROOT:
        if (fuse_opt_add_opt(&mo->kernel_opts, "allow_other") == -1 ||
            fuse_opt_add_arg(outargs, "-oallow_root") == -1)
            return -1;
        return 0;

    case KEY_RO:
        arg = "ro";
        /* fall through */
    case KEY_KERN_FLAG:
        set_mount_flag(arg, &mo->flags);
        return 0;

    case KEY_KERN_OPT:
        return fuse_opt_add_opt(&mo->kernel_opts, arg);

    case KEY_FUSERMOUNT_OPT:
        return fuse_opt_add_opt(&mo->fusermount_opts, arg);

#ifdef __SOLARIS__
    case KEY_SUBTYPE_OPT:
        return fuse_opt_add_opt(&mo->subtype_opt, arg);
#endif /* __SOLARIS__ */

    case KEY_MTAB_OPT:
        return fuse_opt_add_opt(&mo->mtab_opts, arg);

    case KEY_HELP:
#ifdef __SOLARIS__
        mount_help();
#endif /* __SOLARIS__ */
        mo->ishelp = 1;
        break;

    case KEY_VERSION:
#ifdef __SOLARIS__
        mount_version();
#endif /* __SOLARIS__ */
        mo->ishelp = 1;
        break;
    }
    return 1;
}

#ifdef __SOLARIS__

/* return value:
 * >= 0  => fd
 * -1    => error
 */
static int receive_fd(int fd)
{
    struct msghdr msg;
    struct iovec iov;
    char buf[1];
    int rv;
    size_t ccmsg[CMSG_SPACE(sizeof(int)) / sizeof(size_t)];
    struct cmsghdr *cmsg;

    iov.iov_base = buf;
    iov.iov_len = 1;

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    /* old BSD implementations should use msg_accrights instead of
     * msg_control; the interface is different. */
    msg.msg_control = ccmsg;
    msg.msg_controllen = sizeof(ccmsg);

    while(((rv = recvmsg(fd, &msg, 0)) == -1) && errno == EINTR);
    if (rv == -1) {
        perror("recvmsg");
        return -1;
    }
    if(!rv) {
        /* EOF */
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg->cmsg_type != SCM_RIGHTS) {
        fprintf(stderr, "got control message of unknown type %d\n",
                cmsg->cmsg_type);
        return -1;
    }
    return *(int*)CMSG_DATA(cmsg);
}

#endif /* __SOLARIS__ */

void fuse_kern_unmount(const char *mountpoint, int fd)
{
    int res;
#ifdef __SOLARIS__
    int pid;
#endif /* __SOLARIS__ */

    if (!mountpoint)
        return;

    if (fd != -1) {
        struct pollfd pfd;

        pfd.fd = fd;
        pfd.events = 0;
        res = poll(&pfd, 1, 0);
        /* If file poll returns POLLERR on the device file descriptor,
           then the filesystem is already unmounted */
        if (res == 1 && (pfd.revents & POLLERR))
            return;
              /*
               * Need to close file descriptor, otherwise synchronous umount
               * would recurse into filesystem, and deadlock.
               */
        close(fd);
    }
#ifndef __SOLARIS__
    fusermount(1, 0, 1, "", mountpoint);
#else /* __SOLARIS__ */
    if (geteuid() == 0) {
        fuse_mnt_umount("fuse", mountpoint, 1);
        return;
    }

    res = umount2(mountpoint, 2);
    if (res == 0)
        return;

    pid = fork();
    if(pid == -1)
        return;

    if(pid == 0) {
        const char *argv[] =
            { FUSERMOUNT_PROG, "-u", "-q", "-z", "--", mountpoint, NULL };

        exec_fusermount(argv);
        _exit(1);
    }
    waitpid(pid, NULL, 0);
#endif /* __SOLARIS__ */
}

#ifdef __SOLARIS__

static int fuse_mount_fusermount(const char *mountpoint, const char *opts,
                                 int quiet)
{
    int fds[2], pid;
    int res;
    int rv;

    if (!mountpoint) {
        fprintf(stderr, "fuse: missing mountpoint\n");
        return -1;
    }

    res = socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    if(res == -1) {
        perror("fuse: socketpair() failed");
        return -1;
    }

    pid = fork();
    if(pid == -1) {
        perror("fuse: fork() failed");
        close(fds[0]);
        close(fds[1]);
        return -1;
    }

    if(pid == 0) {
        char env[10];
        const char *argv[32];
        int a = 0;

        if (quiet) {
            int fd = open("/dev/null", O_RDONLY);
            dup2(fd, 1);
            dup2(fd, 2);
        }

        argv[a++] = FUSERMOUNT_PROG;
        if (opts) {
            argv[a++] = "-o";
            argv[a++] = opts;
        }
        argv[a++] = "--";
        argv[a++] = mountpoint;
        argv[a++] = NULL;

        close(fds[1]);
        fcntl(fds[0], F_SETFD, 0);
        snprintf(env, sizeof(env), "%i", fds[0]);
        setenv(FUSE_COMMFD_ENV, env, 1);
        exec_fusermount(argv);
        perror("fuse: failed to exec fusermount");
        _exit(1);
    }

    close(fds[0]);
    rv = receive_fd(fds[1]);
    close(fds[1]);
    waitpid(pid, NULL, 0); /* bury zombie */

    return rv;
}

static int fuse_mount_sys(const char *mnt, struct mount_opts *mo,
                          const char *mnt_opts)
{
    char tmp[128];
    const char *devname = "/dev/fuse";
    char *source = NULL;
    char *type = NULL;
    struct stat stbuf;
    int fd;
    int res;

    if (!mnt) {
        fprintf(stderr, "fuse: missing mountpoint\n");
        return -1;
    }

    res = lstat(mnt, &stbuf);
    if (res == -1) {
        fprintf(stderr ,"fuse: failed to access mountpoint %s: %s\n",
                mnt, strerror(errno));
        return -1;
    }

    if (!mo->nonempty) {
        res = fuse_mnt_check_empty("fuse", mnt, stbuf.st_mode, stbuf.st_size);
        if (res == -1)
            return -1;
    }

    fd = open(devname, O_RDWR);
    if (fd == -1) {
        if (errno == ENODEV || errno == ENOENT)
            fprintf(stderr,
                    "fuse: device not found, try 'modprobe fuse' first\n");
        else
            fprintf(stderr, "fuse: failed to open %s: %s\n", devname,
                    strerror(errno));
        return -1;
    }

    snprintf(tmp, sizeof(tmp),  "fd=%i,rootmode=%o,user_id=%i,group_id=%i", fd,
             stbuf.st_mode & S_IFMT, getuid(), getgid());

    res = fuse_opt_add_opt(&mo->kernel_opts, tmp);
    if (res == -1)
        goto out_close;

    source = malloc((mo->fsname ? strlen(mo->fsname) : 0) +
                    (mo->subtype ? strlen(mo->subtype) : 0) +
                    strlen(devname) + 32);

    type = malloc((mo->subtype ? strlen(mo->subtype) : 0) + 32);
    if (!type || !source) {
        fprintf(stderr, "fuse: failed to allocate memory\n");
        goto out_close;
    }

    strcpy(type, mo->blkdev ? "fuseblk" : "fuse");
    if (mo->subtype) {
        strcat(type, ".");
        strcat(type, mo->subtype);
    }
    strcpy(source,
           mo->fsname ? mo->fsname : (mo->subtype ? mo->subtype : devname));

          /* JPA added two final zeroes */
    res = mount(source, mnt, MS_OPTIONSTR|mo->flags, type, NULL, 0,
	    mo->kernel_opts, MAX_MNTOPT_STR, 0, 0);

    if (res == -1 && errno == EINVAL && mo->subtype) {
        /* Probably missing subtype support */
        strcpy(type, mo->blkdev ? "fuseblk" : "fuse");
        if (mo->fsname) {
            if (!mo->blkdev)
                sprintf(source, "%s#%s", mo->subtype, mo->fsname);
        } else {
            strcpy(source, type);
        }
		/* JPA two null args added */
	res = mount(source, mnt, MS_OPTIONSTR|mo->flags, type, NULL, 0,
		    mo->kernel_opts, MAX_MNTOPT_STR, 0, 0);
    }
    if (res == -1) {
        /*
         * Maybe kernel doesn't support unprivileged mounts, in this
         * case try falling back to fusermount
         */
        if (errno == EPERM) {
            res = -2;
        } else {
            int errno_save = errno;
            if (mo->blkdev && errno == ENODEV && !fuse_mnt_check_fuseblk())
                fprintf(stderr, "fuse: 'fuseblk' support missing\n");
            else
                fprintf(stderr, "fuse: mount failed: %s\n",
                        strerror(errno_save));
        }

        goto out_close;
    }

    return fd;

 out_umount:
    umount2(mnt, 2); /* lazy umount */
 out_close:
    free(type);
    free(source);
    close(fd);
    return res;
}

#endif /* __SOLARIS__ */

static int get_mnt_flag_opts(char **mnt_optsp, int flags)
{
    int i;

    if (!(flags & MS_RDONLY) && fuse_opt_add_opt(mnt_optsp, "rw") == -1)
        return -1;

    for (i = 0; mount_flags[i].opt != NULL; i++) {
        if (mount_flags[i].on && (flags & mount_flags[i].flag) &&
	    fuse_opt_add_opt(mnt_optsp, mount_flags[i].opt) == -1)
                return -1;
    }
    return 0;
}

int fuse_kern_mount(const char *mountpoint, struct fuse_args *args)
{
    struct mount_opts mo;
    int res = -1;
    char *mnt_opts = NULL;
#ifdef __SOLARIS__
    struct solaris_mount_opts smo;
    struct fuse_args sa = FUSE_ARGS_INIT(0, NULL);
#endif /* __SOLARIS__ */

    memset(&mo, 0, sizeof(mo));
#ifndef __SOLARIS__
    if (getuid())
	    mo.flags = MS_NOSUID | MS_NODEV;
#else /* __SOLARIS__ */
    mo.flags = 0;
    memset(&smo, 0, sizeof(smo));
    if (args != NULL) {
    	while (args->argv[sa.argc] != NULL)
		fuse_opt_add_arg(&sa, args->argv[sa.argc]);
    }
#endif /* __SOLARIS__ */

    if (args &&
        fuse_opt_parse(args, &mo, fuse_mount_opts, fuse_mount_opt_proc) == -1)
#ifndef __SOLARIS__
        return -1;
#else /* __SOLARIS__ */
        goto out; /* if SOLARIS, clean up 'sa' */

    /*
     * In Solaris, nosuid is equivalent to nosetuid + nodevices. We only
     * have MS_NOSUID for mount flags (no MS_(NO)SETUID, etc.). But if
     * we set that as a default, it restricts specifying just nosetuid
     * or nodevices; there is no way for the user to specify setuid +
     * nodevices or vice-verse. So we parse the existing options, then
     * add restrictive defaults if needed.
     */
    if (fuse_opt_parse(&sa, &smo, solaris_mnt_opts, NULL) == -1)
         goto out;
    if (smo.nosuid || (!smo.nodevices && !smo.devices
        && !smo.nosetuid && !smo.setuid)) {
        mo.flags |= MS_NOSUID;
    } else {
        /*
         * Defaults; if neither nodevices|devices,nosetuid|setuid has
         * been specified, add the default negative option string. If
         * both have been specified (i.e., -osuid,nosuid), leave them
         * alone; the last option will have precedence.
         */
        if (!smo.nodevices && !smo.devices)
             if (fuse_opt_add_opt(&mo.kernel_opts, "nodevices") == -1)
                 goto out;
        if (!smo.nosetuid && !smo.setuid)
            if (fuse_opt_add_opt(&mo.kernel_opts, "nosetuid") == -1)
                 goto out;
    }
#endif /* __SOLARIS__ */

    if (mo.allow_other && mo.allow_root) {
        fprintf(stderr, "fuse: 'allow_other' and 'allow_root' options are mutually exclusive\n");
        goto out;
    }
    res = -1;
    if (mo.ishelp)
        goto out;

    if (get_mnt_flag_opts(&mnt_opts, mo.flags) == -1)
        goto out;
#ifndef __SOLARIS__
    if (!(mo.flags & MS_NODEV) && fuse_opt_add_opt(&mnt_opts, "dev") == -1)
        goto out;
    if (!(mo.flags & MS_NOSUID) && fuse_opt_add_opt(&mnt_opts, "suid") == -1)
        goto out;
    if (mo.kernel_opts && fuse_opt_add_opt(&mnt_opts, mo.kernel_opts) == -1)
        goto out;
    if (mo.mtab_opts &&  fuse_opt_add_opt(&mnt_opts, mo.mtab_opts) == -1)
        goto out;
    if (mo.fusermount_opts && fuse_opt_add_opt(&mnt_opts, mo.fusermount_opts) < 0)
        goto out;
    res = fusermount(0, 0, 0, mnt_opts ? mnt_opts : "", mountpoint);
#else /* __SOLARIS__ */
    if (mo.kernel_opts && fuse_opt_add_opt(&mnt_opts, mo.kernel_opts) == -1)
        goto out;
    if (mo.mtab_opts &&  fuse_opt_add_opt(&mnt_opts, mo.mtab_opts) == -1)
        goto out;
    res = fuse_mount_sys(mountpoint, &mo, mnt_opts);
    if (res == -2) {
        if (mo.fusermount_opts &&
            fuse_opt_add_opt(&mnt_opts, mo.fusermount_opts) == -1)
            goto out;

        if (mo.subtype) {
            char *tmp_opts = NULL;

            res = -1;
            if (fuse_opt_add_opt(&tmp_opts, mnt_opts) == -1 ||
                fuse_opt_add_opt(&tmp_opts, mo.subtype_opt) == -1) {
                free(tmp_opts);
                goto out;
            }

            res = fuse_mount_fusermount(mountpoint, tmp_opts, 1);
            free(tmp_opts);
            if (res == -1)
                res = fuse_mount_fusermount(mountpoint, mnt_opts, 0);
        } else {
            res = fuse_mount_fusermount(mountpoint, mnt_opts, 0);
        }
    }
#endif /* __SOLARIS__ */

out:
    free(mnt_opts);
#ifdef __SOLARIS__
    fuse_opt_free_args(&sa);
    free(mo.subtype);
    free(mo.subtype_opt);
#endif /* __SOLARIS__ */
    free(mo.fsname);
    free(mo.fusermount_opts);
    free(mo.kernel_opts);
    free(mo.mtab_opts);
    return res;
}
