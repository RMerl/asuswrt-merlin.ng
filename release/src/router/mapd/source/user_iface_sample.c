#include "src/utils/includes.h"
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "mapd_interface_ctrl.h"
#include "os.h"
int sock = 0;
#define MAPD_EVENT_PING							0
#define WIRELESS_CLIENT_JOIN_NOTIF				1
#define WIRELESS_CLIENT_LEAVE_NOTIF				2
#define ETHERNET_CLIENT_JOIN_NOTIF				3
#define ETHERNET_CLIENT_LEAVE_NOTIF				4
#define CH_PREF_NOTIF							5
#define HIGHER_LAYER_PAYLOAD					6

#define ETH_ALEN								6
#define ONBOARDING_STATUS_NOTIF					7

#ifdef VENDOR1_FEATURE_EXTEND
#define TRIGGER_MAP_STEERING_NOTIF				8
#define GET_BEACON_REPORT_NOTIF					9
#define GET_BTM_RESPONSE_NOTIF					10
#endif /*VENDOR1_FEATURE_EXTEND*/

#define MAX_SSID_LEN							32
#define MAX_CH_NUM								13
#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]

#define ETH_ONBOARDING_STATE_START				0
#define ETH_ONBOARDING_STATE_DONE				1
#define WIFI_ONBOARDING_STATE_START				0
#define WIFI_ONBOARDING_STATE_DONE				1
struct mapd_interface_ctrl *ctrl_conn = NULL;
struct mapd_user_event
{
	int event_id;
	char event_body[0];
};

struct mapd_user_iface_wireless_client_event
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
};

struct mapd_user_iface_eth_client_event
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char almac[ETH_ALEN];
};

struct mapd_user_iface_ch_pref_event
{
	unsigned char radio_id[ETH_ALEN];
	unsigned char almac[ETH_ALEN];
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char perference;
	unsigned char reason;
};

struct mapd_user_higher_layer_data_event
{
	unsigned int buf_len;
	unsigned char buf[4096];
};

struct mapd_user_onboarding_event
{
	unsigned int bh_type;
	unsigned int onboarding_start_stop;
};

#ifdef VENDOR1_FEATURE_EXTEND
struct mapd_user_iface_trigger_map_steering_event
{
	unsigned char band;
	unsigned char sta_mac[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
	unsigned char cap_11v;
	char ul_rssi;
};

struct mapd_user_iface_get_beacon_report_event
{
	unsigned char band;
	unsigned char sta_mac[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
	unsigned char bssid[ETH_ALEN];
	char dl_rssi;
	unsigned char target_bssid[ETH_ALEN];
	char target_dl_rssi;
};

struct mapd_user_iface_get_btm_response_event
{
	unsigned char band;
	unsigned char sta_mac[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
	unsigned char status_code;
};
#endif /*VENDOR1_FEATURE_EXTEND*/

size_t strlcpy(char *dest, const char *src, size_t siz)
{
	const char *s = src;
	size_t left = siz;

	if (left) {
		/* Copy string up to the maximum size of the dest buffer */
		while (--left != 0) {
			if ((*dest++ = *s++) == '\0')
				break;
		}
	}

	if (left == 0) {
		/* Not enough room for the string; force NUL-termination */
		if (siz != 0)
			*dest = '\0';
		while (*s++)
			; /* determine total src string length */
	}

	return s - src - 1;
}

static int mapd_user_open_connection(const char *ctrl_path)
{
	ctrl_conn = mapd_interface_ctrl_open(ctrl_path);

	if (!ctrl_conn) {
		printf("mapd_interface_ctrl_open failed\n");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int res, i;
	unsigned char event[1024];
	char socket_path[64]={0};
	fd_set rfds;
	struct timeval tv;
	
	struct mapd_user_event *mapd_event = NULL;
	if (argc < 2) {
		printf("arg 1 name, arg 2 ctrl path");
		return -1;
   	}

	printf("count =%d 1=%s 2=%s\n", argc, argv[0], argv[1]);
	os_snprintf(socket_path,sizeof(socket_path),"%s",argv[1]);

	if(mapd_user_open_connection(socket_path) < 0) {
		printf("Can't connect to %s\n", socket_path);
		return -1;
	}
	printf("Succesfully opened connection to mapd\n");

	if(mapd_interface_ctrl_attach(ctrl_conn) < 0) {
		printf("Can't attach to %s\n", socket_path);
		return -1;
	}

	while (1) {
		
		tv.tv_sec = 3;
		tv.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(ctrl_conn->s, &rfds);
		select(ctrl_conn->s + 1, &rfds, NULL, NULL, &tv);
		if (!FD_ISSET(ctrl_conn->s, &rfds))
			continue;
		memset(event, 0, 1024);
		res = recv(ctrl_conn->s, event, 1024, 0);
		if (res < 0) {
			printf("read error: %s\n", strerror(errno));
			continue;
			//return;
		}
		mapd_event = (struct mapd_user_event *)event;
		printf("received msg ID = %d\n", mapd_event->event_id);
		switch (mapd_event->event_id)
		{
			case MAPD_EVENT_PING:
			{
				printf("received ping string --->%s\n", mapd_event->event_body);
				break;
			}
			case WIRELESS_CLIENT_JOIN_NOTIF:
			{
				struct mapd_user_iface_wireless_client_event *client_event;
				client_event = (struct mapd_user_iface_wireless_client_event *)mapd_event->event_body;
				printf("Received join for %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->sta_mac));
				printf("BSSID = %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->bssid));
				printf("SSID = %s\n", client_event->ssid);
				break;
			}
			case WIRELESS_CLIENT_LEAVE_NOTIF:
			{
				struct mapd_user_iface_wireless_client_event *client_event;
				client_event = (struct mapd_user_iface_wireless_client_event *)mapd_event->event_body;
				printf("Received leave for %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->sta_mac));
				printf("BSSID = %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->bssid));
				printf("SSID = %s\n", client_event->ssid);
				break;
			}
			case ETHERNET_CLIENT_JOIN_NOTIF:
			{
				struct mapd_user_iface_eth_client_event *client_event;
				client_event = (struct mapd_user_iface_eth_client_event *)mapd_event->event_body;
				printf("Received join for %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->sta_mac));
				printf("ALMAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->almac));
				break;
			}
			case ETHERNET_CLIENT_LEAVE_NOTIF:
			{
				struct mapd_user_iface_eth_client_event *client_event;
				client_event = (struct mapd_user_iface_eth_client_event *)mapd_event->event_body;
				printf("Received leave for %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->sta_mac));
				printf("ALMAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->almac));
				break;
			}
			case CH_PREF_NOTIF:
			{
				struct mapd_user_iface_ch_pref_event *client_event;
				client_event = (struct mapd_user_iface_ch_pref_event *)mapd_event->event_body;
				printf("Channel Preference Report!!\nRadio ID %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->radio_id));
				printf("ALMAC = %02x:%02x:%02x:%02x:%02x:%02x\nPreference: %d\n",
					PRINT_MAC(client_event->almac), client_event->perference);
				printf("Channel list: ");
				for (i=0 ; i<client_event->ch_num; i++)
					printf("%d ", client_event->ch_list[i]);
				printf("\n");
				break;
			}
			case HIGHER_LAYER_PAYLOAD:
			{
				struct mapd_user_higher_layer_data_event *client_event;
				client_event = (struct mapd_user_higher_layer_data_event *)mapd_event->event_body;

				printf("Higher layer spayload received!\n");
				/* First byte received in application is protocol, rest is payload data */
				printf("Rx data:: Buffer Len:%d, protocol:%d \n",
								client_event->buf_len,
								client_event->buf[0]);

				printf("Payload: ");
				for (i = 0; i < client_event->buf_len - 1; i++)
					printf("%c", client_event->buf[i+1]);
				printf("\n");
				break;
			}
			case ONBOARDING_STATUS_NOTIF:
			{
				struct mapd_user_onboarding_event *client_event;
				client_event = (struct mapd_user_onboarding_event *)mapd_event->event_body;
				if (client_event->bh_type == 0)
					printf("Ethernet onboarding start/stop: %s",
						client_event->onboarding_start_stop ? "STOP": "START");
				else
					printf("Wifi onboarding start/stop: %s",
						client_event->onboarding_start_stop ? "STOP": "START");
				break;
			}
#ifdef VENDOR1_FEATURE_EXTEND
			case TRIGGER_MAP_STEERING_NOTIF:
			{
				struct mapd_user_iface_trigger_map_steering_event *client_event;
				client_event = (struct mapd_user_iface_trigger_map_steering_event *)mapd_event->event_body;
				printf("%sGHz\n", client_event->band == 2 ? "2":"5");
				printf("MAP steering STA %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->sta_mac));
				printf("SSID = %s\n", client_event->ssid);
				printf("11v=%d\n", client_event->cap_11v == 1 ? 1:0);
				printf("UL RSSI:%ddBm\n", client_event->ul_rssi);
				break;
			}
			case GET_BEACON_REPORT_NOTIF:
			{
				struct mapd_user_iface_get_beacon_report_event *client_event;
				client_event = (struct mapd_user_iface_get_beacon_report_event *)mapd_event->event_body;
				printf("%sGHz\n", client_event->band == 2 ? "2":"5");
				printf("MAP steering STA %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->sta_mac));
				printf("SSID = %s\n", client_event->ssid);
				printf("Current AP BSSID = %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->bssid));
				printf("Current AP DL RSSI:%ddbm\n", client_event->dl_rssi);
				printf("Target AP BSSID = %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->target_bssid));
				printf("Target AP DL RSSI:%ddbm\n", client_event->target_dl_rssi);
				break;
			}
			case GET_BTM_RESPONSE_NOTIF:
			{
				struct mapd_user_iface_get_btm_response_event *client_event;
				client_event = (struct mapd_user_iface_get_btm_response_event *)mapd_event->event_body;
				printf("%sGHz\n", client_event->band == 2 ? "2":"5");
				printf("MAP steering STA %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(client_event->sta_mac));
				printf("SSID = %s\n", client_event->ssid);
				printf("Status Code=%u\n", client_event->status_code);
				break;
			}
#endif /*VENDOR1_FEATURE_EXTEND*/
		}
	}
	return 0;
}
