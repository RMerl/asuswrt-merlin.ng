#include <u-boot/sha256.h>
#include <string.h>

int main(int argc, char **argv)
{
	char combined[256];
	char sha[32];
	char password[32];
	int i;
	char cset[] =
	    "abcdefghijk#mnopqrstuvwxyzABCDEFGHIJKLMN-PQRSTUVWXYZ@!23456789_*";
	long long pass;
	if (argc != 3) {
		printf("must have exactly 2 args\n");
		printf("usage:   %s secret serialnum\n", argv[0]);
		printf("example:  %s password12345 16777216\n", argv[0]);
		printf(" would generate the password for device sernum 16777216 if the secret is password12345\n");
		exit(1);
	}
	strcpy(combined, argv[1]);
	strcat(combined, argv[2]);
	sha256_csum_wd(combined, strlen(combined), sha, 1024);
	memcpy(&pass, sha, sizeof(pass));
	for (i = 0; i < 8; i++) {
		password[i] = cset[(pass >> (6 * i)) & 0x3f];
	}
	password[8] = 0;
	printf("password:%s\n", password);
}
