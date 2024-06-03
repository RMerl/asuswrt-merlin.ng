#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <cutils/android_reboot.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


/* Directories created by init defined in system/rootdir/init.rc */
static char *INIT_DIRS[] = {
    "/system/etc/ppp",
    "/data/misc",
    "/data/local",
    "/data/local/tmp",
    "/data/data",
    "/data/app_private",
    "/data/app",
    NULL
};

static void wipe (const char *path);

static int usage()
{
    fprintf(stderr, "wipe <system|data|all>\n\n"
                    "system means '/system'\n"
                    "data means '/data'\n");

    return -1;
}

int wipe_main (int argc, char *argv[])
{
    char *whatToWipe;

    if (argc != 2) return usage();

    whatToWipe = argv[1];

    if (0 == strcmp (whatToWipe, "system")) {
        fprintf(stdout, "Wiping /system\n");
        wipe ("/system");
        fprintf(stdout, "Done wiping /android\n");
    } else if (0 == strcmp (whatToWipe, "data")) {
        fprintf(stdout, "Wiping /data\n");
        wipe ("/data");
        fprintf(stdout, "Done wiping /data\n");
    } else if (0 == strcmp (whatToWipe, "all")) {
        fprintf(stdout, "Wiping /system and /data\n");
        wipe ("/system");
        wipe ("/data");
        fprintf(stdout, "Done wiping /system and /data\n");
    } else if (0 == strcmp(whatToWipe, "nuke")) {
		int ret;
		fprintf(stdout, "Nuking the device...\n");
		wipe ("/system");
        wipe ("/data");
		fprintf(stdout, "Device nuked! Rebooting...\n");
		ret = android_reboot(ANDROID_RB_RESTART, 0, 0);
	    if (ret < 0) {
	        fprintf(stderr, "Reboot failed, %s\n", strerror(errno));
	        return 1;
	    }
	} else {
        return usage();
    }

    return 0;
}

static char nameBuffer[PATH_MAX];
static struct stat statBuffer;

static void wipe (const char *path) 
{
    DIR *dir;
    struct dirent *de;
    int ret;

    dir = opendir(path);

    if (dir == NULL) {
        fprintf (stderr, "Error opendir'ing %s '%s'\n",
                    path, strerror(errno));
        return;
    }

    char *filenameOffset;

    strcpy(nameBuffer, path);
    strcat(nameBuffer, "/");

    filenameOffset = nameBuffer + strlen(nameBuffer);

    for (;;) {
        de = readdir(dir);

        if (de == NULL) {
            break;
        }

        if (0 == strcmp(de->d_name, ".")
                || 0 == strcmp(de->d_name, "..")
                || 0 == strcmp(de->d_name, "lost+found")
        ) {
            continue;
        }

        strcpy(filenameOffset, de->d_name);

        ret = lstat (nameBuffer, &statBuffer);

        if (ret != 0) {
            fprintf(stderr, "stat() error on '%s' '%s'\n", 
                    nameBuffer, strerror(errno));
        }

        if(S_ISDIR(statBuffer.st_mode)) {
            int i;
            char *newpath;

#if 0
            closedir(dir);
#endif

            newpath = strdup(nameBuffer);
            wipe(newpath);

            /* Leave directories created by init, they have special permissions. */
            for (i = 0; INIT_DIRS[i]; i++) {
                if (strcmp(INIT_DIRS[i], newpath) == 0) {
                    break;
                }
            }
            if (INIT_DIRS[i] == NULL) {
                ret = rmdir(newpath);
                if (ret != 0) {
                    fprintf(stderr, "rmdir() error on '%s' '%s'\n", 
                        newpath, strerror(errno));
                }
            }

            free(newpath);

#if 0
            dir = opendir(path);
            if (dir == NULL) {
                fprintf (stderr, "Error opendir'ing %s '%s'\n",
                            path, strerror(errno));
                return;
            }
#endif

            strcpy(nameBuffer, path);
            strcat(nameBuffer, "/");

        } else {
            ret = unlink(nameBuffer);

            if (ret != 0) {
                fprintf(stderr, "unlink() error on '%s' '%s'\n", 
                    nameBuffer, strerror(errno));
            }
        }
    }

    closedir(dir);

}
