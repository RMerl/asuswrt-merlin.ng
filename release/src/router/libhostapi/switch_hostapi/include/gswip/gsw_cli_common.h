/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _MXL_MAC_CLI_LIB_H_
#define _MXL_MAC_CLI_LIB_H_

char *findArgParam(int argc, char *argv[], char *name);
int scanStrParamArg(int argc, char *argv[], char *name, size_t size, char *param);
int scanParamArg(int argc, char *argv[], char *name, size_t size, void *param);
void printHex32Value(char *name, unsigned int value, unsigned int bitmapIndicator);
int scanMAC_Arg(int argc, char *argv[], char *name, unsigned char *param);
int scanPMAP_Arg(int argc, char *argv[], char *name, unsigned char *param);
int findStringParam(int argc, char *argv[], char *name);
int scanKey_Arg(int argc, char *argv[], char *name, unsigned char size, char *param);
int scanPMAC_Arg(int argc, char *argv[], char *name, unsigned char *param);
int scanIPv4_Arg(int argc, char *argv[], char *name, unsigned int *param);
int scanIPv6_Arg(int argc, char *argv[], char *name, unsigned short *param);
void printMAC_Address(unsigned char *pMAC);
int checkValidMAC_Address(unsigned char *pMAC);
int scan_advert(int argc, char *argv[], char *name, uint64_t *param);
int print_advert(char *buf, uint32_t size, uint64_t param);

#endif /* _MXL_MAC_CLI_LIB_H_ */
