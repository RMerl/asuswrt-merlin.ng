#include <common.h>
#include <command.h>
#include <console.h>
#include <errno.h>
#include <net.h>

#include "uipopt.h"
#include "uip.h" 
#include "uip_arp.h"
#include "httpd.h"
#include "bca_sdk.h"
#include "bca_common.h"

typedef enum
{
	HTTP_IMG_UPGRADE_IDLE,
	HTTP_IMG_UPGRADE_INPROGRESS,
	HTTP_IMG_UPGRADE_OK,
	HTTP_IMG_UPGRADE_WAIT_RESET,
	HTTP_IMG_UPGRADE_FAIL,
} HTTP_IMG_UPGRADE_STATE;

static HTTP_IMG_UPGRADE_STATE http_upgrade_state = HTTP_IMG_UPGRADE_IDLE;
static uint64_t httpd_reset_wait_start_ticks = 0;
static uint64_t httpd_reset_wait_timeout_sec = 0;

extern int (*ip_tap)(uchar *in_packet, int len, struct ip_udp_hdr *ip);

#define HTTPD_RESET_WAIT_SEC	3	/* Amount of seconds to wait before triggering device reset */
#define IPPROTO_TCP		6	/* Trasmission Control Protocol	*/

extern void (*jobs_func)(void);

void send_httpd(void)
{
	volatile uchar *tmpbuf;
	uchar *tx_packet;
	int i;
	
	tx_packet = net_get_async_tx_pkt_buf();
	tmpbuf = tx_packet;

	for(i = 0; i < 40 + UIP_LLH_LEN; i++)
		tmpbuf[i] = uip_buf[i];

	for(; i < uip_len; i++)
		tmpbuf[i] = uip_appdata[i - 40 - UIP_LLH_LEN];

	net_send_packet(tx_packet, uip_len);
}

int httpd_check_net_env(void)
{
	char eth_addr[6];
	
	if(!env_get("ipaddr"))
	{
		printf("HTTPD: ipaddr not defined\n");
		return -1;
	}

	if(!env_get("netmask"))
	{
		printf("HTTPD: netmask not defined\n");
		return -1;
	}
	
	if(!eth_env_get_enetaddr("ethaddr",eth_addr))
	{
		printf("HTTPD: ethaddr not defined\n");
		return -1;
	}
	printf("HTTPD: ready for starting\n");
	return 0;
}

void httpd_start(void)
{
	struct uip_eth_addr eaddr;
	unsigned short ip[2];
	char eth_addr[6];
	char *ip_addr = NULL, *net_mask = NULL;

	uip_init();

	ip_addr = env_get("ipaddr");
	if(ip_addr)
	{
		net_ip = string_to_ip(ip_addr);
	}
	else
	{
		printf("httpd_start: ipaddr not defined\n");
		return;
	}
	net_mask = env_get("netmask");
	if(net_mask)
	{
		net_netmask = string_to_ip(net_mask);
	}
	else
	{
		printf("httpd_start: netmask not defined\n");
		return;
	}

	ip[0] = net_ip.s_addr & 0x0000FFFF;
	ip[1] = (net_ip.s_addr & 0xFFFF0000) >> 16;
	uip_sethostaddr(ip);

	ip[0] = net_netmask.s_addr & 0x0000FFFF;
	ip[1] = (net_netmask.s_addr & 0xFFFF0000) >> 16;
	
	uip_setnetmask(ip);	
	
	if(eth_env_get_enetaddr("ethaddr",eth_addr))
	{
		memcpy(net_ethaddr, eth_addr, 6);
	}
	else
	{
		printf("httpd_start: ethaddr not defined\n");
		return;
	}
	memcpy(eaddr.addr, net_ethaddr, 6);
	uip_setethaddr(eaddr);

	uip_listen(HTONS(80));
}

int httpd_poll_post_process(void)
{
	/* Handle device reset after succesfull image upgrade */
	if( http_upgrade_state == HTTP_IMG_UPGRADE_WAIT_RESET )
	{
		if( get_ticks() - httpd_reset_wait_start_ticks > httpd_reset_wait_timeout_sec * get_tbclk())
			do_reset(NULL, 0, 0, NULL);
	}
}

int http_rcv(uchar *in_packet, int len, struct ip_udp_hdr *ip)
{
	if(ip->ip_p == IPPROTO_TCP)
	{
		if(len > sizeof(uip_buf))
		{
			printf("TCP/IP packet len %d > uip buffer %d\n", len, sizeof(uip_buf));
			return 1;
		}
		memcpy(uip_buf, in_packet, len);
		uip_arp_ipin();
		uip_input();
		if(uip_len > 0)
		{
			uip_arp_out();
			send_httpd();
		}

		if( uip_aborted() && (http_upgrade_state != HTTP_IMG_UPGRADE_WAIT_RESET) )
			http_upgrade_state = HTTP_IMG_UPGRADE_IDLE;

		/* If upgrade succeeded, reset after sending confirmation page */
		if( http_upgrade_state > HTTP_IMG_UPGRADE_INPROGRESS )
		{
			/* Issue reset after sending update page if upgrade was good */
			switch(http_upgrade_state) 
			{
				case HTTP_IMG_UPGRADE_OK:
					/* Prepare to close TCP connection */
					uip_close();
					/* Set state to waiting for reset */
					http_upgrade_state = HTTP_IMG_UPGRADE_WAIT_RESET;
					/* Arm the reset counter */
					httpd_reset_wait_start_ticks = get_ticks();
					httpd_reset_wait_timeout_sec = HTTPD_RESET_WAIT_SEC;
					printk("Resetting in %llu seconds....\n", httpd_reset_wait_timeout_sec);
				break;

				case HTTP_IMG_UPGRADE_WAIT_RESET:
					/* Do nothing */
				break;

				default:
					http_upgrade_state = HTTP_IMG_UPGRADE_IDLE;
				break;
			}
		}

		return 1;
	}
	return 0;
}

STREAM_UPGRADE_STATUS http_get_upgrade_status(void)
{
	if( http_upgrade_state == HTTP_IMG_UPGRADE_FAIL )
		return STREAM_UPGRADE_ERR;
	else
		return STREAM_UPGRADE_OK;

}

STREAM_TRANSFER_STATUS http_update_image(char *data, unsigned int len, STREAM_TRANSFER_STATE state)
{
	static char* upload_addr = NULL;
	static char* orig_upload_addr = NULL;
	char * s = NULL;
	int ret = -1;

	if( state == TRANSFER_START ) {
		if( http_upgrade_state == HTTP_IMG_UPGRADE_IDLE ) {
			/* pre-set load_addr */
			s = env_get("loadaddr");
			if (s != NULL)
				upload_addr = simple_strtoul(s, NULL, 16);
			else
				upload_addr = load_addr;

			orig_upload_addr = upload_addr;
			http_upgrade_state = HTTP_IMG_UPGRADE_INPROGRESS;
			printf("%s: downloading image to 0x%p\n         ", __FUNCTION__, orig_upload_addr);
		} else {
			printf("%s: ERROR: Image Upgrade already in progress! Ignoring Upgrade request!p\n", __FUNCTION__);
			return STOP_STREAM_DATA;
		}
	}
	
	if( upload_addr ) {
		memcpy(upload_addr, data, len);
		upload_addr += len;
	}

	if( state == TRANSFER_END )
	{
		int img_index = get_img_index_for_upgrade(0);
#if defined(XT8_V2)
		if(check_pkgtb_boardid((void*)orig_upload_addr) != -1 ){
			ret = flash_upgrade_img_bundle(orig_upload_addr, img_index, NULL);
		}else{
			ret = CMD_RET_FAILURE; /* no valid boardid, upgrade fail */
		}
#else
		ret = flash_upgrade_img_bundle(orig_upload_addr, img_index, NULL);
#endif
		upload_addr = NULL;
		orig_upload_addr = NULL;

		if( ret ) {
			printf("ERROR: HTTP Image upgrade failed!!\n");
			http_upgrade_state = HTTP_IMG_UPGRADE_FAIL;
		} else {
			printf("INFO: HTTP Image upgrade successfull!!\n");		
			//FIXME: Check boot once flag before committing?
			commit_image( img_index );
			http_upgrade_state = HTTP_IMG_UPGRADE_OK;
		}
	}

	return NEXT_STREAM_DATA;
}

void http_poll(void)
{
	int ret;
	struct udevice *current;
	static int uip_initalized = 0;

	current = eth_get_dev();

	if(!current || !eth_is_active(current))
	{	
		net_init();
		if (eth_is_on_demand_init()) 
		{
			eth_halt();
			eth_set_current();
			ret = eth_init();
			if (ret < 0) {
				eth_halt();
				unregister_cli_job_cb(http_poll);
				printf("HTTPD: no network interface found, failed to init network\n");
				return;
			}
		} 
		else 
		{
			eth_init_state_only();
		}
	}

	if(!uip_initalized)
	{
		httpd_start();
		handle_data_stream = http_update_image;
		get_stream_upgrade_status = http_get_upgrade_status;
		ip_tap = http_rcv;
		uip_initalized = 1;
	}

	eth_rx();
	httpd_poll_post_process();
}


