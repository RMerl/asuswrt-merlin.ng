#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
	const char *action = NULL, *devpath = NULL, *physdevpath = NULL, *mediastatus = NULL;
	int sd = -1;
	struct sockaddr_un serv_addr_un;
	if (argc > 3)
	{
		action = argv[1];
		devpath = argv[2];
		physdevpath = argv[3];
	}
	memset(&serv_addr_un, 0, sizeof(serv_addr_un));
	serv_addr_un.sun_family = AF_LOCAL;
	strcpy(serv_addr_un.sun_path, "/tmp/hotplug.socket");
	sd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sd >= 0)
	{
		if (connect(sd, (const struct sockaddr*)&serv_addr_un, sizeof(serv_addr_un)) >= 0)
		{
			char data[1024];
			if (!action) action = getenv("ACTION");
			if (action)
			{
				snprintf(data, sizeof(data) - 1, "ACTION=%s", action);
				data[sizeof(data) - 1] = 0;
				send(sd, data, strlen(data) + 1, 0);
			}
			else
			{
				mediastatus = getenv("X_E2_MEDIA_STATUS");
				if (mediastatus)
				{
					snprintf(data, sizeof(data) - 1, "X_E2_MEDIA_STATUS=%s", mediastatus);
					data[sizeof(data) - 1] = 0;
					send(sd, data, strlen(data) + 1, 0);
				}
			}
			if (!devpath)
			{
				devpath = getenv("DEVPATH");
				if (!devpath) devpath = "-";
			}
			snprintf(data, sizeof(data) - 1, "DEVPATH=%s", devpath);
			data[sizeof(data) - 1] = 0;
			send(sd, data, strlen(data) + 1, 0);
			if (!physdevpath)
			{
				physdevpath = getenv("PHYSDEVPATH");
				if (!physdevpath) physdevpath = "-";
			}
			snprintf(data, sizeof(data) - 1, "PHYSDEVPATH=%s", physdevpath);
			data[sizeof(data) - 1] = 0;
			send(sd, data, strlen(data) + 1, 0);
		}
		close(sd);
	}
}
