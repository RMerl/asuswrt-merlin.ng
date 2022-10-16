#include "ipaddr.h"

LIST_HEAD(privLanList);

int main(int argc, char *argv[])
{
	struct in_addr addr;
	int ret;

	if(argc == 2) {
		if (inet_aton(argv[1], &addr) == 0) {
			fprintf(stderr, "Invalid address\n");
			exit(EXIT_FAILURE);
		}

		//printf("%s\n", inet_ntoa(addr));
		//exit(EXIT_SUCCESS);
	}

	//printf("check %s[0x%08x] is Private/Public address or not?\n", argv[1], addr.s_addr);

	ret = quick_privLan_chk(&addr);
	if(ret == false) {
		printf("%s is %s\n", inet_ntoa(addr), "Public");
		return 0;
	}
	
	privLanList_init(&privLanList);

	ret = in_privLan(&addr, &privLanList);

	printf("%s is %s\n", inet_ntoa(addr), ret ? "Private" : "Public");

	privLanList_free(&privLanList);

	return 0;
}

