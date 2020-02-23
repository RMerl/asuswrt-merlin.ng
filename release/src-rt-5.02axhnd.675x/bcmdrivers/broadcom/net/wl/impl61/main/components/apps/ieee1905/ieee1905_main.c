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
 * IEEE1905 Main
 */

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "ieee1905_glue.h"
#include "ieee1905_interface.h"
#include "ieee1905_socket.h"
#include "ieee1905_json.h"
#include "ieee1905_message.h"
#include "ieee1905_netlink.h"
#include "ieee1905_security.h"
#include "ieee1905_trace.h"
#include "ieee1905_plc.h"
#include "ieee1905_ethstat.h"
#include "ieee1905_brutil.h"
#include "ieee1905_udpsocket.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_flowmanager.h"
#include "ieee1905_control.h"
#include "ieee1905_cmsutil.h"
#include "i5ctl.h"
#include "ieee1905.h"
#include <bcmnvram.h>
#include <signal.h>

#define I5_TRACE_MODULE i5TraceMain

#if defined(SUPPORT_IEEE1905_FM)
#pragma message "1905 flow manager support enabled"
#else
#pragma message "1905 flow manager support disabled"
#endif // endif

#if defined(SUPPORT_IEEE1905_AUTO_WDS)
#pragma message "1905 wds auto configuration enabled"
#else
#pragma message "1905 wds auto configuration disabled"
#endif // endif

char *const traceTokens[] = {
  "m", /* module - int or string */
  "l", /* log level */
  "i", /* interface - if applicable */
  NULL
};

#ifdef MULTIAP
static unsigned int i5FetchMultiAPMode()
{
  unsigned int supServiceFlag = 0;

  char *nvval = nvram_safe_get("multiap_mode");
  supServiceFlag = strtoul(nvval, NULL, 0);
  printf("\n\n\nsupServiceFlag %d\n\n\n", supServiceFlag);

  return supServiceFlag;
}
#endif /* MULTIAP */

static int i5FetchRegistrar()
{
  int registrar = 0;

  char *nvval = nvram_safe_get("multiap_registrar");
  registrar = strtol(nvval, NULL, 0);
  printf("\n\n\registrar %d\n\n\n", registrar);

  return registrar;
}

#ifdef MULTIAP

void get_random_mac(unsigned char *outmac)
{
  unsigned char mac[MAC_ADDR_LEN];

  mac[0] = 0x02;
  mac[1] = 0x10;
  mac[2] = 0x18;
  mac[3] = rand() & 0xFF;
  mac[4] = rand() & 0xFF;
  mac[5] = rand() & 0xFF;

  memcpy(outmac, mac, MAC_ADDR_LEN);
}

void dump_datamodel()
{
  printf("\n======================= Data Model START =======================\n");
  i5_dm_device_type* selfDevice = i5DmGetSelfDevice();
  if (NULL == selfDevice) {
    return;
  }

  i5_dm_interface_type *currInterface = (i5_dm_interface_type *)selfDevice->interface_list.ll.next;

  while (currInterface != NULL) {
    printf("IFR " I5_MAC_FMT " \n", I5_MAC_PRM(currInterface->InterfaceId));

    i5_dm_bss_type *bss;
    bss = (i5_dm_bss_type*)currInterface->bss_list.ll.next;
    while (bss != NULL) {
      printf("    BSS " I5_MAC_FMT " \n", I5_MAC_PRM(bss->BSSID));
      i5_dm_clients_type *clients;
      clients = (i5_dm_clients_type*)bss->client_list.ll.next;
      while (clients != NULL) {
        printf("        STA " I5_MAC_FMT " \n", I5_MAC_PRM(clients->mac));
        clients = clients->ll.next;
      }
      bss = bss->ll.next;
    }

    currInterface = currInterface->ll.next;
  }
  printf("======================= Data Model END =======================\n");
}

unsigned char g_bss[MAC_ADDR_LEN];

void add_sta(bcm_usched_handle *handle, void *arg)
{
  unsigned char sta[MAC_ADDR_LEN];
  get_random_mac(sta);
  ieee1905_sta_assoc_disassoc(g_bss, sta, 1, 0, 1, NULL, 0);
}
#endif /* MULTIAP */

void test_ieee1905(void *hdl)
{
#ifdef MULTIAP
  int sta_create = 0;
  bcm_usched_handle *usched_hdl = (bcm_usched_handle*)hdl;
  srand((unsigned)time(NULL));

  i5_dm_device_type* selfDevice = i5DmGetSelfDevice();
  if (NULL == selfDevice) {
    return;
  }
  i5_dm_interface_type *currInterface = (i5_dm_interface_type *)selfDevice->interface_list.ll.next;

  while (currInterface != NULL) {
    int k = rand() % 2;
    int i = 0;
    for (i = 0 ; i < k; i++) {
      unsigned char bss[MAC_ADDR_LEN];
      get_random_mac(bss);
      printf("\n\nAdd BSS " I5_MAC_FMT " To IFR " I5_MAC_FMT " \n\n", I5_MAC_PRM(bss), I5_MAC_PRM(currInterface->InterfaceId));
      ieee1905_add_bss(currInterface->InterfaceId, bss, (unsigned char*)"Test", 4, 0xd024, "eth1", 1);
      dump_datamodel();

      int l = rand() % 2;
      int m = 0;
      for (m = 0 ; m < l; m++) {
        unsigned char sta[MAC_ADDR_LEN];
        get_random_mac(sta);
        if (!sta_create) {
          memcpy(g_bss, bss, MAC_ADDR_LEN);
          bcm_usched_add_timer(usched_hdl, 9000000, 0, add_sta, NULL);
          sta_create = 1;
        }
        printf("\n\nAdd STA " I5_MAC_FMT " to BSS " I5_MAC_FMT " in IFR " I5_MAC_FMT " \n\n", I5_MAC_PRM(sta), I5_MAC_PRM(bss), I5_MAC_PRM(currInterface->InterfaceId));
        ieee1905_sta_assoc_disassoc(bss, sta, 1, 0, 0, NULL, 0);
        dump_datamodel();
      }
    }
    currInterface = currInterface->ll.next;
  }
#endif /* MULTIAP */
  ieee1905_start();
}

static void
ieee1905_exit()
{
  printf("CLOSING IEEE1905\n");
  ieee1905_deinit();
}

/* Signal handler */
void
ieee1905_signal_hdlr(int sig)
{
	ieee1905_exit();
	exit(1);
}

int main(int argc, char *argv[])
{
  int c;
  int rc = 0;
  void *usched_hdl = NULL;
  unsigned int supServiceFlag = 0;

  //print command line options:
  if (1) {
      printf("STARTING 1905: command line: ");
      for (c = 0; c < argc; c++)
        printf("%s ", argv[c]);
      printf("\n");
  }

  while ((c = getopt(argc, argv, "ta:")) != -1) {
    switch (c) {
      case 't':
      {
        char *subopttok;
        char *value;
        char *opt;
        char *opttok;
        char *endptr;
        int   level   = 0;
        int   module  = 255;
        int   ifindex = 0;
        int   reqOptCnt = 0;

        value = strtok_r(optarg, ",", &subopttok);
        while ( value != NULL ) {
          opt = strtok_r(value, "=", &opttok);
          while ( 1 ) {
            if ( 0 == strcmp(opt, "m") ) {
              opt = strtok_r(NULL, "=", &opttok);
              errno = 0;
              module = strtol(opt, &endptr, 10);
              if ( (errno != 0) || (opt == endptr) ) {
                 break;
              }
              reqOptCnt++;
            }
            else if ( 0 == strcmp(opt, "l") ) {
              opt = strtok_r(NULL, "=", &opttok);
              errno = 0;
              level = strtol(opt, &endptr, 10);
              if ( (errno != 0) || (opt == endptr) ) {
                 break;
              }
              reqOptCnt++;
            }
            else if ( 0 == strcmp(opt, "i") ) {
              opt = strtok_r(NULL, "=", &opttok);
              errno = 0;
              ifindex = strtol(opt, &endptr, 10);
              if ( (errno != 0) || (opt == endptr) ) {
                 ifindex = if_nametoindex(opt);
                 break;
              }
            }
            else {
              printf("No match found for token: /%s/\n", opt);
            }
            break;
          }
          value = strtok_r(NULL, ",", &subopttok);
        }
        if ( reqOptCnt == 2 ) {
          i5TraceSet(module, level, ifindex, NULL);
        }
        else {
          printf("Ignoring invalid trace option\n");
        }
        break;
      }
      case 'a':
      {
        unsigned char al_mac[MAC_ADDR_LEN] = {0};
        unsigned int multiapMode;

        multiapMode = i5FetchMultiAPMode();
        printf("Getting AL MAC Address for Mode[0x%02x]...\n", multiapMode);
        i5WlCfgGet1905MacAddress(multiapMode, al_mac);
        printf("AL MAC["I5_MAC_DELIM_FMT"]\n", I5_MAC_PRM(al_mac));
        return 0;
      }
      default:
        printf("Warning -- unknown option %c\n", c);
    }
  }

  signal(SIGTERM, ieee1905_signal_hdlr);
  signal(SIGINT, ieee1905_signal_hdlr);
  signal(SIGPWR, ieee1905_signal_hdlr);

#ifdef MULTIAP
  supServiceFlag = i5FetchMultiAPMode();
#endif /* MULTIAP */

#if defined(USEBCMUSCHED)
  usched_hdl = (bcm_usched_handle*)bcm_usched_init();
#endif /* USEBCMUSCHED */

  do {
    ieee1905_config config;

    memset(&config, 0, sizeof(config));
    ieee1905_init(usched_hdl, supServiceFlag, i5FetchRegistrar(), NULL, &config);

    test_ieee1905(usched_hdl);
#if defined(USEBCMUSCHED)
    bcm_usched_run((bcm_usched_handle*)usched_hdl);
#else
    i5SocketMain();
#endif /* USEBCMUSCHED */
  } while (0);

  ieee1905_exit();

#if defined(USEBCMUSCHED)
  /* Stop the scheduler and deinit */
  if (usched_hdl) {
    bcm_usched_stop((bcm_usched_handle*)usched_hdl);
    bcm_usched_deinit((bcm_usched_handle*)usched_hdl);
  }
#endif /* USEBCMUSCHED */

  return rc;
}
