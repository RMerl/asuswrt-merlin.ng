#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <libnuc029.h>

void usage(char *str) {

	printf("Parameter Num Error.\nUsage:\n");
	printf("\t Control Device Addr [0x15] on I2C bus [0]\n\n");
	printf("\t [1] %s r [reqAddr] [bytes]\n", str);
	printf("\t [2] %s w [reqAddr] [Data1] [Data2] [bytes]\n", str);
	printf("\t [3] %s v (mcu ver)\n", str);
	printf("\t [4] %s mute (mute LED ctrl) [val]\n", str);
	printf("\t [5] %s vol (volume LED ctrl) [val]\n", str);
	printf("\t [6] %s led (LED ctrl) [Data1]\n", str);
	printf("\t [7] %s RGB (LED ctrl) [R] [G] [B] [LED] [LED2]\n", str);
	printf("\t [8] %s u [File path]\n", str);

	return;
}

int main(int argc, char *argv[]) {

	int ret = 0;
	if(argc < 2) {
		usage(argv[0]);
		return 0;
	}

	if (!strcmp(argv[1], "r")) {
		unsigned char *buf;
		unsigned char reg = strtol(argv[2], NULL, 16);
		buf = (unsigned char *) calloc (strtol(argv[3], NULL, 10), sizeof(unsigned char));
		read_register(&reg, strtol(argv[3], NULL, 10), buf);
		print_buf(buf, strtol(argv[3], NULL, 10));
		free(buf);
	} else if (!strcmp(argv[1], "w")) {
		if (argc < 6) {
			usage(argv[0]);
			return 0;
		}
		if (strtol(argv[5], NULL, 10) == 1) {
			unsigned char data[2] = {strtol(argv[2], NULL, 16), strtol(argv[3], NULL, 16)};
			ret = write_register(data, sizeof(data));
		} else {
			unsigned char data[3] = {strtol(argv[2], NULL, 16), strtol(argv[3], NULL, 16), strtol(argv[4], NULL, 16)};
			ret = write_register(data, sizeof(data));
		}
		MyDBG("Write:[%s]\n", (ret > 0 ) ? "OK": "NOK");
	} else if (!strcmp(argv[1], "v")) {
		unsigned char data[32] = {0};
		ret = GetMcuVer(data, sizeof(data));
		if (ret > 0) {
			printf("Ver:[%s]\n", data);
		}
	} else if (!strcmp(argv[1], "mute")) {
		ret = SetMuteLED(strtol(argv[2], NULL, 10));
		MyDBG("[%s]\n", (ret > 0 ) ? "OK": "NOK");
	} else if (!strcmp(argv[1], "vol")) {
		ret = SetVolumeLED(strtol(argv[2], NULL, 10));
		MyDBG("[%s]\n", (ret > 0 ) ? "OK": "NOK");
	} else if (!strcmp(argv[1], "led")) {
		ret = SetLED(strtol(argv[2], NULL, 16));
		MyDBG("[%s]\n", (ret > 0 ) ? "OK": "NOK");
	} else if (!strcmp(argv[1], "RGB")) {
		if (argc < 7) {
			usage(argv[0]);
			return 0;
		}
		ret = SetRGB_LED(strtol(argv[2], NULL, 16), strtol(argv[3], NULL, 16), strtol(argv[4], NULL, 16),
				 strtol(argv[5], NULL, 16), strtol(argv[6], NULL, 16));
		MyDBG("[%s]\n", (ret > 0 ) ? "OK": "NOK");
	} else if (!strcmp(argv[1], "u")) {
		ret = UpgradeAPROM(argv[2]);
		printf("[%s]\n", (ret > 0 ) ? "OK": "NOK");
	}

	return 0;
}

