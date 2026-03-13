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

static int _get_cmdline(const int pid, char *cmdline, const size_t cmdline_len)
{
	FILE *fp;
	char path[512], buf[2048] = {0}, *ptr;
	long int fsize;
	
	if(!cmdline)
		return 0;

	snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
	fp = fopen(path, "r");
	if(fp)
	{
		memset(cmdline, 0, cmdline_len);
		
		fsize = fread(buf, 1, sizeof(buf), fp);
		ptr = buf;
		while(ptr - buf <  fsize)
		{
			if(*ptr == '\0')
			{
				++ptr;
				continue;
			}

			snprintf(cmdline + strlen(cmdline), cmdline_len - strlen(cmdline), ptr == buf? "%s": " %s", ptr);
			ptr += strlen(ptr);
		}
		fclose(fp);
		return strlen(cmdline);
	}
	return 0;
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
	const char *invalid_caller[] = {"/usr/sbin/httpd", "/usr/sbin/lighttpd", "hotplug2", NULL};
	const char busybox_caller[] = "/bin/busybox";
	const char *invalid_busybox[] = {"crond", NULL};
	const char accept_caller[] = "/bin/sh /usr/sbin/app_";
	int i, j, flag;
	FILE *fp;
	char path[128], line[512];

	//check accept list first
	pid = getpid();
	while(_get_cmdline(pid, cmdline, sizeof(cmdline)) > 0)
	{
		if(!strncmp(cmdline, accept_caller, strlen(accept_caller)))
		{
			return 0;
		}
		ppid = _get_ppid(pid);
		pid = ppid;
		if(!ppid)
			break;
	}

	pid = getpid();
	while(_get_process_path(pid, cmdline, sizeof(cmdline)) > 0)
	{
		for(i = 0; invalid_caller[i]; ++i)
		{
			if((invalid_caller[i][0] == '/' && !strcmp(cmdline, invalid_caller[i])) ||
				(invalid_caller[i][0] != '/' && strstr(cmdline, invalid_caller[i])))
			{
				return 1;
			}
		}

		if(!strcmp(cmdline, busybox_caller))
		{
			//check name
			flag = 0;
			snprintf(path, sizeof(path), "/proc/%d/status", pid);
			fp = fopen(path, "r");
			if(fp)
			{
				while(fgets(line, sizeof(line), fp))
				{
					if (strncmp(line, "Name:", 5) == 0)
					{
						char* token = strtok(line, " \t");
						if (token != NULL)
						{
							token = strtok(NULL, " \t\n");
							if (token != NULL)
							{
								for(j = 0; invalid_busybox[j]; ++j)
								{
									if(!strcmp(token, invalid_busybox[j]))
									{
										flag = 1;
										break;
									}
								}
								if(flag)
									break;
							}
						}
					}
				}
				fclose(fp);
				if(flag)
				{
					return 1;
				}
			}
		}

		ppid = _get_ppid(pid);
		pid = ppid;
		if(!ppid)
		{
			break;
		}
	}
	return 0;
}

int asus_invalid_mnt_path(const char* path)
{
	const char *invalid_path[] = {"/bin", "/usr", "/sbin", "/lib", "/rom", "/www", NULL};
	int i = 0;

	if(path)
	{
		char real_path[PATH_MAX] = {0};

		realpath(path, real_path);
		if (strncmp(real_path, "/", 1))
			return 1;
		if (!strcmp(real_path, "/"))
			return 1;
		while(invalid_path[i])
		{
			if(!strncmp(real_path, invalid_path[i], sizeof(invalid_path[i])))
			{
				return 1;
			}
			++i;
		}
	}
	return 0;
}
