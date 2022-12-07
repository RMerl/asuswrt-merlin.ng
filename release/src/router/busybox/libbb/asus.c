#include "libbb.h"

static int _get_process_path(const int pid, char *real_path, const size_t real_path_len)
{
	char link_path[512];

	if(!real_path)
		return 0;

	snprintf(link_path, sizeof(link_path), "/proc/%d/exe", pid);
	memset(real_path, 0, real_path_len);

	if(-1 == readlink(link_path, real_path, real_path_len))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static int _get_ppid(const int pid)
{
	FILE *fp;
	char path[512], buf[512] = {0}, name[128], val[512];
	int ppid = 0;

	snprintf(path, sizeof(path), "/proc/%d/status", pid);

	fp = fopen(path, "r");
	if(fp)
	{
		while(fgets(buf, sizeof(buf), fp))
		{
			memset(name, 0, sizeof(name));
			memset(val, 0, sizeof(val));
			sscanf(buf, "%[^:]: %s", name, val);
			if(!strcmp(name, "PPid"))
			{
				ppid = atoi(val);
				break;
			}
		}
		fclose(fp);
	}
	return ppid;
}

int asus_check_caller(void)
{
  pid_t ppid, pid;
	char cmdline[2048];
  const char *invalid_caller[] = {"/usr/sbin/httpd", "/usr/sbin/lighttpd", NULL};
  int i;
  FILE *fp;

  pid = getpid();
  while(_get_process_path(pid, cmdline, sizeof(cmdline)) > 0)
  {
    for(i = 0; invalid_caller[i]; ++i)
    {
      if(!strcmp(cmdline, invalid_caller[i]))
      {
        fp = fopen("/tmp/test_busybox", "w");
        if(fp)
        {
          fprintf(fp, "Invalid caller(%s)\n", invalid_caller[i]);
          fclose(fp);
        }
        return 1;
      }
    }
    ppid = _get_ppid(pid);
    pid = ppid;
    if(!ppid)
      break;
  }
  return 0;  
}
