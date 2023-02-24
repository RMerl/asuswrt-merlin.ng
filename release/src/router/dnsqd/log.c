#include <stdio.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include "log.h"


void getFormattedTime(char * const p, int sz) 
{
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(p, sz, "%H:%M:%S", timeinfo);
}

void my_printf(const char *format, ...)
{
#if 1 
	FILE *f;
	int nfd;
	va_list args;
  	if (isFileExist(DNS_DEBUG_TO_FILE))
	{
		nfd = open("/tmp/dnsquery.log", O_WRONLY | O_APPEND | O_CREAT);
 	}
	else
	{
		nfd = open("/dev/console", O_WRONLY | O_NONBLOCK);
 	}
	if(nfd >= 0)
       	{
      		if((f = fdopen(nfd, "w")) != NULL) 
		{
        		va_start(args, format);
			vfprintf(f, format, args);
  			va_end(args);
			fclose(f);
		} 
		else
		{
  			close(nfd);
		}
 	}
#endif
}

int isFileExist(char *fname)
{
	struct stat fstat;
	
	if (lstat(fname,&fstat)==-1)
		return 0;
	if (S_ISREG(fstat.st_mode))
		return 1;
	
	return 0;
}

