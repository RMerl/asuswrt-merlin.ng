#include <stdio.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include "cosql_utils.h"
#include "log.h"
#include "codb_config.h"

int isFileExist(char *fname)
{
	struct stat fstat;
	
	if (lstat(fname,&fstat)==-1)
		return 0;
	if (S_ISREG(fstat.st_mode))
		return 1;
	
	return 0;
}

void dprintf_impl(const char* func, size_t line, sqlite3 *pdb, const char* fmt, ...) 
{
	if (pdb==NULL) {
		return;
	}

	codb_config_t* cfg = cosql_get_config(pdb);
	if (cfg!=NULL && cfg->enable_debug==1) {
		
		va_list ap;

        int nfd = -1;

        if (isFileExist(LIBCODB_DEBUG_TO_FILE)) {
            nfd = open("/tmp/libcodb.log", O_WRONLY | O_APPEND | O_CREAT);
        }
        else {
            nfd = open("/dev/console", O_WRONLY | O_NONBLOCK);
        }

        if(nfd >= 0){
            dprintf(nfd, WHERESTR, func, line);
            va_start(ap, fmt);
            vdprintf(nfd, fmt, ap);
            dprintf(nfd, "\n");
            va_end(ap);
            close(nfd);
        }
	}
}