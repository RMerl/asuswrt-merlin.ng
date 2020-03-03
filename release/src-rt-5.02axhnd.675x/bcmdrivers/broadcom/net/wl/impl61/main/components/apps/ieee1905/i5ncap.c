/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

/*
 * IEEE1905 control utility
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "i5ctl.h"
#include "ieee1905_datamodel_priv.h"

#include "ieee1905_trace.h"

typedef int (t_I5_NCAP_CMD_FUNC)(int argc, char** argv);

typedef struct t_i5_ncap_cmd
{
  unsigned int       subcmd;
  char               *help;
  t_I5_NCAP_CMD_FUNC *func;
} t_I5_NCAP_CMD;

static int i5Ncap1Handler (int argc, char *argv[]);
static int i5Ncap2Handler (int argc, char *argv[]);
static int i5Ncap3Handler (int argc, char *argv[]);
static int i5Ncap4Handler (int argc, char *argv[]);
static int i5Ncap5Handler (int argc, char *argv[]);
static int i5Ncap7Handler (int argc, char *argv[]);
static int i5Ncap8Handler (int argc, char *argv[]);
static int i5Ncap9Handler (int argc, char *argv[]);

static t_I5_NCAP_CMD ncapCmds[] = {
    {1, "Display all packets\n\n[-i MAC Address] address of interface\n"
                               "-r               received packets\n"
                               "-t               transmitted packets\n", i5Ncap1Handler},
    {2, "Simulate Push button\n", i5Ncap2Handler},
    {3, "Send pre-defined message\n\n[-i MAC Address] destination Address \n"
                                    "-m <msg #>       Message Types: 0 - TopologyDiscovery\n"
                                    "                                1 - TopologyNotification\n"
                                    "                                2 - TopologyQuery\n"
                                    "                                5 - LinkMetricQuery\n",i5Ncap3Handler},
    {4, "Return AL MAC Address of local 1905 node\n", i5Ncap4Handler},
    {5, "Send raw packet\n\n[-i MAC Address]     destination address\n\nPacket must be entered in raw, space separated, hex bytes\n",i5Ncap5Handler},
    {6, "Fragmented Message - NOT IMPLEMENTED\n",NULL},
    {7, "Set Wi-Fi SSID and Password\n\n[-s   SSID]\n[-p   password]\n",i5Ncap7Handler},
    {8, "Set PLC Password (via NMK)\n\n[-p   password]\n",i5Ncap8Handler},
    {9, "TBD -- Set MoCA Password\n\n[-p   password]\n",i5Ncap9Handler},
};

static void i5NcapShowUsage(char *prog_name)
{
    printf("Available '%s' Commands\n", prog_name);
    printf("\n");
    printf("%s -h <#>: detailed help with '%s <#>'\n", prog_name, prog_name);
    printf("\n");
    printf("%s 1 -i <MACAddress> -r -t : packet display\n", prog_name);
    printf("%s 2 : push button\n", prog_name);
    printf("%s 3 -i <MACAddress> -m <##> : send message\n", prog_name);
    printf("%s 4 : display AL MAC Address\n", prog_name);
    printf("%s 5 -i <MACAddress> : send packet\n", prog_name);
    printf("%s 6 -- TBD --\n", prog_name);
    printf("%s 7 -s <SSID> -p <password>\n", prog_name);
    printf("%s 8 -p <password> : set PLC Password and NMK\n", prog_name);
    printf("%s 9 -- TBD --\n", prog_name);
}

static char i5NcapDoesSwitchExist (int argc, char *argv[], char *searchSwitch)
{
  int index = 2;
  for ( ; index < argc ; index ++) {
    if (strcmp(argv[index],searchSwitch) == 0) {
      return 1;
    }
  }
  return 0;
}

static char i5NcapGetSwitchOption (int argc, char *argv[], char *searchSwitch, char **optionArg)
{
  int index = 2;
  for ( ; index < argc - 1; index ++) {
    if (strcmp(argv[index],searchSwitch) == 0) {
      *optionArg = argv[index+1];
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    i5NcapShowUsage(argv[0]);
    return 0;
  }
  else {
    if ((strcmp(argv[1], "-h") == 0) && (argc > 2)) {
      int subCmdNum = atoi(argv[2]);
      int index = 0;
      int const maxIndex = sizeof(ncapCmds) / sizeof(ncapCmds[0]);
      for ( ; index < maxIndex ; index ++) {
        if (ncapCmds[index].subcmd == subCmdNum) {
          printf("\n%s %s\n\n",argv[0],argv[2]);
          printf("%s",ncapCmds[index].help);
          return 0;
        }
      }
      printf("Command %s not known\n", argv[2]);
    }
    else {
      int subCmdNum = atoi(argv[1]);
      int index = 0;
      int const maxIndex = sizeof(ncapCmds) / sizeof(ncapCmds[0]);
      for ( ; index < maxIndex ; index ++) {
        if (ncapCmds[index].subcmd == subCmdNum) {
          if (NULL == ncapCmds[index].func) {
            printf("Command %s not implemented\n", argv[1]);
            break;
          }
          else {
            ncapCmds[index].func(argc, &argv[0]);
            break;
          }
        }
      }
      if (index == maxIndex) {
        printf("Command %s not known\n", argv[1]);
        return -1;
      }

    }
  }

  return 0;
}

static int i5Ncap1Handler (int argc, char *argv[])
{
  char *ifMac = NULL;
  t_I5_API_TRACE_MSG msg = {};
  int rc = 0;
  int sd = 0;

  msg.module_id = i5TracePacket;
  msg.ifindex = 0;
  if (i5NcapDoesSwitchExist(argc, argv, "-t")) {
    msg.depth |= I5_MESSAGE_DIR_TX;
  }
  if (i5NcapDoesSwitchExist(argc, argv, "-r")) {
    msg.depth |= I5_MESSAGE_DIR_RX;
  }

  if (i5NcapGetSwitchOption(argc, argv, "-i", &ifMac)) {
    if (i5String2MacAddr(ifMac, msg.interfaceMac) == NULL) {
      printf("Invalid MAC address.\n");
      return -1;
    }
  }

  sd = i5apiOpen();
  if (-1 == sd) {
    return sd;
  }

  rc = i5apiSendMessage(sd, I5_API_CMD_TRACE, &msg, sizeof msg);
  if ( -1 == rc ) {
    printf("Failed to send data to daemon\n");
  }
  else {
    printf("%s 01 OK\n", argv[0]);
  }

  i5apiClose(sd);
  return rc;
}

static int i5Ncap2Handler (int argc, char *argv[])
{
  int rc = 0;
  int sd = 0;

  sd = i5apiOpen();
  if (-1 == sd) {
    return sd;
  }

  rc = i5apiSendMessage(sd, I5_API_CMD_PUSH_BUTTON, NULL, 0);
  if ( -1 == rc ) {
    printf("Failed to send data to daemon\n");
  }
  else {
    printf("%s 02 OK\n", argv[0]);
  }
  i5apiClose(sd);
  return rc;
}

static int i5Ncap3Handler (int argc, char *argv[])
{
  t_I5_API_GOLDEN_NODE_SEND_MSG  msg = {};
  char *ifMac = NULL;
  char *typeArg = NULL;
  int macAddrValid = 0;

  if (i5NcapGetSwitchOption(argc, argv, "-i", &ifMac)) {
    printf("If mac found: %s\n",ifMac);
    if (i5String2MacAddr(ifMac, msg.macAddr) != NULL) {
      macAddrValid = 1;
    }
  }

  if (i5NcapGetSwitchOption(argc, argv, "-m", &typeArg)) {
    void *pBuf = 0;
    int rc = 0;

    msg.messageId = atoi(typeArg);

    if (((i5MessageTopologyQueryValue == msg.messageId) || (i5MessageLinkMetricQueryValue == msg.messageId)) && (0 == macAddrValid)) {
      printf("Message Type %d requires valid MAC address.  Use %s 3 -m %d -i <xx:xx:xx:xx:xx:xx> \n", msg.messageId, argv[0], msg.messageId);
    }
    else {

      rc = i5apiTransaction(I5_API_CMD_SEND_MESSAGE, &msg, sizeof msg, &pBuf, 0);
      if ( rc != -1 ) {
        t_I5_CTL_CMDRESP_SEND_MSG_TYPE *response = pBuf;
        char *bufChar = pBuf;
        int success = 0;
        int index = 0;
        for ( ; index < sizeof(msg) ; index ++) {
          if (bufChar[index] != 0) {
            success = 1;
            break;
          }
        }
        if (success) {
          printf("%s 03 SMAC=" I5_MAC_DELIM_FMT " MID=%04d (0x%04x)OK\n", argv[0],
                 I5_MAC_PRM(response->srcMacAddr),
                 response->messageId, response->messageId);
        }
        else {
          printf("%s 03 FAIL Bad Message Type \n", argv[0]);
        }
      }
      else {
        printf("Failed to send data to daemon\n");
      }
      if (pBuf) {
        free(pBuf);
      }
    }
  }
  else {
    printf("Message Type Not Found.  Use %s 3 -m <##> \n", argv[0]);
  }
  return 0;
}

static int i5Ncap4Handler (int argc, char *argv[])
{
  void *pBuf = NULL;
  int rc = 0;

  rc = i5apiTransaction(I5_API_CMD_SHOW_AL_MAC, 0, 0, &pBuf, 0);
  if ( rc != -1 ) {
    char *response = pBuf;
    printf("%s 04 AL_ID=" I5_MAC_DELIM_FMT " OK \n", argv[0],
           I5_MAC_PRM(response));
  }
  if (pBuf) {
    free(pBuf);
  }

  return rc;
}

static int i5Ncap5Handler (int argc, char *argv[])
{
  t_I5_API_GOLDEN_NODE_SEND_MSG_BYTES  msg = {};
  unsigned int                         msgSize = 0;
  unsigned int                         copyIndex = 0;
  int sd = 0;

  char buf[100]; /* 32 bytes * 3 chars each + overhead */
  unsigned int packet[1500];
  unsigned int packetSize = 0;
  char *ifMac = NULL;

  if (i5NcapGetSwitchOption(argc, argv, "-i", &ifMac)) {
    if (i5String2MacAddr(ifMac, msg.macAddr) == NULL) {
      printf("Mac Addr Invalid.  Use %s 5 -i <xx:xx:xx:xx:xx:xx>\n", argv[0]);
      return -1;
    }
  }
  else {
    printf("Mac Addr not found.  Use %s 5 -i <xx:xx:xx:xx:xx:xx>\n", argv[0]);
    return -1;
  }

  while (feof(stdin) == 0) {
    int bytesFromThisLine = 0;
    if (fgets(buf, sizeof(buf), stdin) != NULL ) {
      int parseIndex = 0;

      while ((parseIndex < strlen(buf)) && (bytesFromThisLine < 16)) {

        char *nextSpace = strchr(&buf[parseIndex], (int)' ');
        int fieldLength = 1; /* in case the "nextSpace" is NULL, read whatever is left */

        if (NULL != nextSpace) {
          fieldLength = nextSpace - &buf[parseIndex];
        }

        if ((fieldLength > 0) && (fieldLength < 3) && (1 == sscanf(&buf[parseIndex], "%02x", &packet[packetSize] ))) {
          bytesFromThisLine++;
          packetSize ++;
        }

        if (NULL == nextSpace) {
          /* returning NULL means we hit end of string without finding a space, so we're done */
          break;
        }
        parseIndex += fieldLength + 1; /* go one character past the space */
      }

    }
  }

  msgSize = MAC_ADDR_LEN + packetSize;
  for ( ; copyIndex < packetSize; copyIndex++) {
    msg.message[copyIndex] = (unsigned char)packet[copyIndex];
  }

  sd = i5apiOpen();
  if (-1 == sd) {
    return sd;
  }

  if ( -1 == i5apiSendMessage(sd, I5_API_CMD_SEND_BYTES, &msg, msgSize) ) {
    printf("Failed to send data to daemon\n");
  }
  else {
    printf("%s 05 OK\n", argv[0]);
  }
  i5apiClose(sd);

  return 0;
}

static int i5Ncap7Handler (int argc, char *argv[])
{
  t_I5_API_PASSWORD_SSID_MSG msg;
  char *password = NULL;
  char *ssid = NULL;

  if (i5NcapGetSwitchOption(argc, argv, "-p", &password)) {
    if (strlen(password) > I5_PASSWORD_MAX_LENGTH) {
      printf("Password %s too long.  Max %d digits\n", password, I5_PASSWORD_MAX_LENGTH);
      return -1;
    }
    if (strpbrk((char *)password, "?\"$[\\]+") != NULL) {
      printf("password contains illegal char (%c)\n", strpbrk((char *)password, "?\"$[\\]+")[0] );
      return -1;
    }
    strncpy((char *)msg.password, password, I5_PASSWORD_MAX_LENGTH);
    msg.password[I5_PASSWORD_MAX_LENGTH] = '\0';
  }
  else {
    msg.password[0] = '\0';
  }

  if (i5NcapGetSwitchOption(argc, argv, "-s", &ssid)) {
    if (strlen(ssid) > I5_SSID_MAX_LENGTH) {
      printf("SSID %s too long.  Max %d digits\n", ssid, I5_SSID_MAX_LENGTH);
      return -1;
    }
    if (strpbrk((char *)ssid, "?\"$[\\]+") != NULL) {
      printf("SSID contains illegal char (%c)\n", strpbrk((char *)ssid, "?\"$[\\]+")[0] );
      return -1;
    }
    strncpy((char *)msg.ssid, ssid, I5_SSID_MAX_LENGTH);
    msg.ssid[I5_SSID_MAX_LENGTH] = '\0';
  }
  else {
    msg.ssid[0] = '\0';
  }

  if ((msg.ssid[0] != '\0') || (msg.password[0] != '\0')) {
    int sd = i5apiOpen();
    if (-1 == sd) {
      return sd;
    }

    int rc = i5apiSendMessage(sd, I5_API_CMD_SET_WIFI_PASS_SSID, &msg, sizeof msg);

    if ( -1 == rc ) {
      printf("Failed to send data to daemon\n");
    }
    else {
      printf("%s 07 OK\n", argv[0]);
    }
  }

  return 0;
}

static int i5Ncap8Handler (int argc, char *argv[])
{
  t_I5_API_PASSWORD_SSID_MSG msg;
  void *pBuf = NULL;
  int rc = 0;
  char *password = NULL;

  if (i5NcapGetSwitchOption(argc, argv, "-p", &password)) {
    if (strlen(password) > I5_PASSWORD_MAX_LENGTH) {
      printf("Password %s too long.  Max %d digits\n", msg.password, I5_PASSWORD_MAX_LENGTH);
      return -1;
    }
  }
  else {
    printf("Password not found.  Use %s 8 -p <password>\n", argv[0]);
    return -1;
  }

  strncpy((char *)msg.password, password, I5_PASSWORD_MAX_LENGTH);
  msg.password[I5_PASSWORD_MAX_LENGTH] = '\0';
  memset(msg.ssid, 0, I5_SSID_MAX_LENGTH+1);

  rc = i5apiTransaction(I5_API_CMD_SET_PLC_PASS_NMK, &msg, sizeof msg, &pBuf, 0);
  if ( -1 == rc ) {
    printf("Failed to send data to daemon\n");
  }
  else {
    if ( ((char*)pBuf)[0] == 0) {
      printf("%s 08 OK\n", argv[0]);
    }
    else {
      printf("%s 08 FAIL Password Set Failed\n", argv[0]);
    }
  }
  if (pBuf) {
    free(pBuf);
  }

  return 0;
}

static int i5Ncap9Handler (int argc, char *argv[])
{
  printf("%s 9\nNot handled yet\n", argv[0]);
  return 0;
}
