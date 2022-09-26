/* $Id: wingenminiupnpcstrings.c,v 1.5 2020/12/03 23:23:10 nanard Exp $ */
/* Project: miniupnp
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * Author: Thomas Bernard
 * Copyright (c) 2005-2020 Thomas Bernard
 * This software is subjects to the conditions detailed
 * in the LICENSE file provided within this distribution */
#include <stdio.h>
#include <windows.h>

/* This program display the Windows version and is used to
 * generate the miniupnpcstrings.h
 * wingenminiupnpcstrings miniupnpcstrings.h.in miniupnpcstrings.h
 */
int main(int argc, char * * argv) {
	char buffer[256];
	OSVERSIONINFO osvi;
	FILE * fin;
	FILE * fout;
	int n;
	char miniupnpcVersion[32];
	/* dwMajorVersion :
       The major version number of the operating system. For more information, see Remarks.
     dwMinorVersion :
       The minor version number of the operating system. For more information, see Remarks.
     dwBuildNumber :
       The build number of the operating system.
     dwPlatformId
       The operating system platform. This member can be the following value.
     szCSDVersion
       A null-terminated string, such as "Service Pack 3", that indicates the
       latest Service Pack installed on the system. If no Service Pack has
       been installed, the string is empty.
   */
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&osvi);

	printf("Windows %lu.%lu Build %lu %s\n",
	       osvi.dwMajorVersion, osvi.dwMinorVersion,
	       osvi.dwBuildNumber, (const char *)&(osvi.szCSDVersion));

	fin = fopen("VERSION", "r");
	fgets(miniupnpcVersion, sizeof(miniupnpcVersion), fin);
	fclose(fin);
	for(n = 0; n < sizeof(miniupnpcVersion); n++) {
		if(miniupnpcVersion[n] < ' ')
			miniupnpcVersion[n] = '\0';
	}
	printf("MiniUPnPc version %s\n", miniupnpcVersion);

	if(argc >= 3) {
		fin = fopen(argv[1], "r");
		if(!fin) {
			fprintf(stderr, "Cannot open %s for reading.\n", argv[1]);
			return 1;
		}
		fout = fopen(argv[2], "w");
		if(!fout) {
			fprintf(stderr, "Cannot open %s for writing.\n", argv[2]);
			fclose(fin);
			return 1;
		}
		n = 0;
		while(fgets(buffer, sizeof(buffer), fin)) {
			if(0 == memcmp(buffer, "#define OS_STRING \"OS/version\"", 30)) {
				sprintf(buffer, "#define OS_STRING \"MSWindows/%ld.%ld.%ld\"\n",
				        osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
			} else if(0 == memcmp(buffer, "#define MINIUPNPC_VERSION_STRING \"version\"", 42)) {
				sprintf(buffer, "#define MINIUPNPC_VERSION_STRING \"%s\"\n",
				        miniupnpcVersion);
			}
			/*fputs(buffer, stdout);*/
			fputs(buffer, fout);
			n++;
		}
		fclose(fin);
		fclose(fout);
		printf("%d lines written to %s.\n", n, argv[2]);
	}
	if(argc >= 4) {
		fout = fopen(argv[3], "w");
		if(fout == NULL) {
			fprintf(stderr, "Cannot open %s for writing.\n", argv[2]);
			return 1;
		} else {
			char * cur, * next;
			fprintf(fout, "#define LIBMINIUPNPC_DOTTED_VERSION \"%s\"\n", miniupnpcVersion);
			next = strchr(miniupnpcVersion, '.');
			if (next && *next) {
				*next = '\0';
				next++;
			}
			fprintf(fout, "#define LIBMINIUPNPC_MAJOR_VERSION %s\n", miniupnpcVersion);
			cur = next;
			next = strchr(cur, '.');
			if (next && *next) {
				*next = '\0';
				next++;
			}
			fprintf(fout, "#define LIBMINIUPNPC_MINOR_VERSION %s\n", cur);
			cur = next;
			next = strchr(cur, '.');
			if (next && *next) {
				*next = '\0';
				next++;
			}
			fprintf(fout, "#define LIBMINIUPNPC_MICRO_VERSION %s\n", cur);
			fclose(fout);
			printf("%s written\n", argv[3]);
		}
	}
  return 0;
}
