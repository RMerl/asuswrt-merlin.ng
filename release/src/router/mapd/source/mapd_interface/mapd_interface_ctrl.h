#ifndef mapd_interface_CTRL_H
#define mapd_interface_CTRL_H

#include <sys/un.h>

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */
#ifdef SUPPORT_MULTI_AP
/* spec v171027 */
enum MAPRole {
	MAP_ROLE_TEARDOWN = 4,
	MAP_ROLE_FRONTHAUL_BSS = 5,
	MAP_ROLE_BACKHAUL_BSS = 6,
	MAP_ROLE_BACKHAUL_STA = 7,
};

#define FH_BSS (1U << (MAP_ROLE_FRONTHAUL_BSS))
#define BH_BSS (1U << (MAP_ROLE_BACKHAUL_BSS))
#endif
/**
 * struct mapd_interface_ctrl - Internal structure for control interface library
 *
 * This structure is used by the daemon control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */


struct mapd_interface_ctrl
{
	int s;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};

/* Capabilities */
#define DOT11K_SUPPORTED (1U << (0))
#define DOT11V_SUPPORTED (1U << (1))
#define DOT11R_SUPPORTED (1U << (2))
#define MBO_SUPPORTED (1U << (3))

#define _2G_SUPPORTED (1U << (0))
#define _5G_SUPPORTED (1U << (1))

#define MAX_STR_SIZE_MAC 900

/**
 * struct client_db - describe the client database
 *
 * @mac: the mac address of a client
 * @bssid: the bssid of the current bss, note that if the client is not associated 
 * any bss, this field should be zero
 * @capab: the capability of a client, it could be DOT11K_SUPPORTED, DOT11V_SUPPORTED, DOT11R_SUPPORTED
 * or MBO_SUPPORTED
 * @phy_mode: the phy mode of a client
 *		0-MODE_CCK
 *		1-MODE_OFDM
 *		2-MODE_HTMIX
 *		3-MODE_HTGREENFIELD
 *		4-MODE_VHT
 * @max_bw: the maximum bandwidth per band of a client
 *		0-BW_20
 *		1-BW_40
 *		2-BW_80
 *		3-BW_160
 *		4-BW_10
 *		5-BW_5
 *		6-BW_8080
 * @spatial_stream: the number of spartial streams of a client
 * @known_band: the operating band was observed of a client, it could be _2G_SUPPORTED, _5G_SUPPORTED or _2G_SUPPORTED|_5G_SUPPORTED
 * @know_channels: the opperating channels was observed of a client, note that it is mapping to bitmap
 * so this value should be converted to real channel numbers
 */
struct GNU_PACKED client_db {
	unsigned char mac[6];
	unsigned char bssid[6];
	unsigned int capab;
	unsigned char phy_mode;
	unsigned char max_bw[2];
	unsigned char spatial_stream;
	unsigned char know_band;
	unsigned char know_channels[5];
};

/**
 * mapd_interface_ctrl_attach - Register as an event monitor for the control interface
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * Returns: 0 on success, -1 on failure, -2 on timeout
 *
 * This function registers the control interface connection as a monitor for
 * Mand. After a success mapd_interface_ctrl_attach() call, the
 * control interface connection starts receiving event messages that can be
 * read with mapd_interface_ctrl_recv().
 */

int mapd_interface_ctrl_attach(struct mapd_interface_ctrl *ctrl);

/**
 * mapd_interface_ctrl_detach - Unregister event monitor from the control interface
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * Returns: 0 on success, -1 on failure, -2 on timeout
 *
 * This function unregisters the control interface connection as a monitor for
 * mapd events, i.e., cancels the registration done with
 * mapd_interface_ctrl_attach().
 */

int mapd_interface_ctrl_detach(struct mapd_interface_ctrl *ctrl);


/**
 * mapd_interface_ctrl_open - Open a control interface to mapd.
 * @ctrl_path: Path for UNIX domain sockets; ignored if UDP sockets are used.
 * Returns: Pointer to abstract control interface data or %NULL on failure
 *
 * This function is used to open a control interface to mapd.
 * ctrl_path is usually /var/run/mapd. This path
 * is configured in mapd and other programs using the control
 * interface need to use matching path configuration.
 */
struct mapd_interface_ctrl * mapd_interface_ctrl_open(const char *ctrl_path);

/**
 * mapd_interface_ctrl_close - Close a control interface to wapp
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 *
 * This function is used to close a control interface
 */

void mapd_interface_ctrl_close(struct mapd_interface_ctrl *ctrl);

/**
 * mapd_interface_ctrl_request - Send a commapd mapd
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * @cmd: the cmd structure pointer
 * @cmd_len: Length of the cmd in bytes
 * @reply: Buffer for the response
 * @reply_len: Reply buffer length
 * @msg_cb: Callback function for unsolicited messages or %NULL if not used
 * Returns: 0 on success, -1 on error (send or receive failed), -2 on timeout
 *
 * This function is used to send commapds to mapd. Received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply. This function will block for up to 10 seconds while waiting
 * for the reply. If unsolicited messages are received, the blocking time may
 * be longer.
 *
 * msg_cb can be used to register a callback function that will be called for
 * unsolicited messages received while waiting for the commapd response. These
 * messages may be received if mapd_interface_ctrl_request() is called at the same time as
 * mapd is sending such a message. This can happen only if
 * the program has used mapd_interface_ctrl_attach() to register itself as a monitor for
 * event messages. Alternatively to msg_cb, programs can register two control
 * interface connections and use one of them for commapds and the other one for
 * receiving event messages, in other words, call mapd_interface_ctrl_attach() only for
 * the control interface connection that will be used for event messages.
 */

int mapd_interface_ctrl_request(struct mapd_interface_ctrl *ctrl, const char *cmd, size_t cmd_len,
        char *reply, size_t *reply_len,
        void (*msg_cb)(char *msg, size_t len));

/**
 * mapd_interface_ctrl_recv - Receive a pending control interface message
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive a pending control interface message. The received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply.

 * mapd_interface_ctrl_recv() is only used for event messages, i.e., mapd_interface_ctrl_attach()
 * must have been used to register the control interface as an event monitor.
 */
int mapd_interface_ctrl_recv(struct mapd_interface_ctrl *ctrl, char *reply, size_t *reply_len);


/**
 * mapd_interface_ctrl_pending - Check whether there are pending event messages
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * Returns: 1 if there are pending messages, 0 if no, or -1 on error
 *
 * This function will check whether there are any pending control interface
 * message available to be received with mapd_interface_ctrl_recv(). mapd_interface_ctrl_pending() is
 * only used for event messages, i.e., mapd_interface_ctrl_attach() must have been used to
 * register the control interface as an event monitor.
 */
int mapd_interface_ctrl_pending(struct mapd_interface_ctrl *ctrl);

/**
 * mapd_interface_ctrl_get_fd - Get file descriptor used by the control interface
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * Returns: File descriptor used for the connection
 *
 * This function can be used to get the file descriptor that is used for the
 * control interface connection. The returned value can be used, e.g., with
 * select() while waiting for multiple events.
 *
 * The returned file descriptor must not be used directly for sending or
 * receiving packets; instead, the library functions mapd_interface_ctrl_request() and
 * mapd_interface_ctrl_recv() must be used for this.
 */
int mapd_interface_ctrl_get_fd(struct mapd_interface_ctrl *ctrl);
#ifdef SUPPORT_MULTI_AP
/**
 * mapd_interface_get_topology - topology information from mapd
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * @topo_buf: used to store the returned topology information
 * @buf_len: the length of the topo_buf, and it will be the length of topo_buf when this function
 * returns.
 * Returns: success return 0, otherwise return -1
 *
 * This function can be used to get the topology information which is in json format
 */
int mapd_interface_get_topology(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_);

/**
 * mapd_interface_set_enrollee_bh_info - set the backhaul info of the enrollee agent
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * @if_mac: the mac address of the interface
 * @if_type: the type of the interface, 0-eth 1-wireless
 * Returns: success return 0, otherwise return -1
 *
 * This function can be used to the backhaul information of the enrollee agent, usually before onboading procedure
 * of the non-existing agent, it should call this API.
 */
int mapd_interface_set_enrollee_bh_info(struct mapd_interface_ctrl *ctrl, char *if_mac, unsigned char if_type);
int mapd_interface_set_acl_block(struct mapd_interface_ctrl *ctrl, unsigned char type, int argc, char *argv[]);

/**
 * mapd_interface_set_bss_role - set the bss role
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * @bssid: the idendity of a bss
 * @role: could be FH_BSS, BH_BSS or FH_BSS|BH_BSS
 * Returns: success return 0, otherwise return -1
 *
 * This function can be used to set the role of some bss, usually this API is used by
 * the exsiting agent or controller to set its bss to map fronthaul or backhaul bss
 */
int mapd_interface_set_bss_role(struct mapd_interface_ctrl *ctrl, unsigned char *bssid, unsigned char role);

/**
 * mapd_interface_trigger_map_wps - trigger the map onboarding procedure
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * @mac_addr: the mac address of ther local interface on which the map onboadring running on
 * Returns: success return 0, otherwise return -1
 *
 * This function can be used to tigger some intreface to start map onboarding procedure, usually
 * this API can be called for both exsiting map device or non-exsiting map agent to trigger wps procedure.
 */
int mapd_interface_trigger_map_wps(struct mapd_interface_ctrl *ctrl, char *mac_addr);

int mapd_interface_trigger_bh_steer(struct mapd_interface_ctrl *ctrl, char *iface, char *bssid);
/**
 * mapd_interface_trigger_map_wps - trigger the map onboarding procedure
 * @ctrl: Control interface data from mapd_interface_ctrl_open()
 * @dbs_buf: the pointer to the client_db structure array
 * @buf_len: the length of dbs_buf
 * Returns: the number of clients in database
 *
 * This function can be used to get all the client element in client database, the maximum element number 
 * coould be 512
 */
#endif
int mapd_interface_get_client_db(struct mapd_interface_ctrl *ctrl, struct client_db *dbs_buf, size_t *buf_len);



#ifdef SUPPORT_MULTI_AP
int mapd_interface_get_role(struct mapd_interface_ctrl *ctrl,int *role);
int mapd_interface_select_best_ap(struct mapd_interface_ctrl *ctrl);
int mapd_interface_set_rssi_thresh(struct mapd_interface_ctrl *ctrl, char *rssi_thresh );
int mapd_interface_mandate_steer(struct mapd_interface_ctrl *ctrl, char *mac_bh, char *bssid );
int mapd_interface_bh_steer(struct mapd_interface_ctrl *ctrl, char *mac_bh, char *bssid );
int mapd_interface_bh_ConnectionStatus(struct mapd_interface_ctrl *ctrl,char *conn_status);
int mapd_interface_bh_ConnectionType(struct mapd_interface_ctrl *ctrl,char *conn_type);
int mapd_interface_set_ChUtil_thresh(struct mapd_interface_ctrl *ctrl, char *cu_thresh2G , char *cu_thresh5GL, char *cu_thresh5GH);
#endif
int mapd_interface_Set_Steer(struct mapd_interface_ctrl *ctrl,char *set_steer);
#ifdef SUPPORT_MULTI_AP
int mapd_interface_trigger_onboarding(struct mapd_interface_ctrl *ctrl, char *int_name);
int mapd_interface_config_renew(struct mapd_interface_ctrl *ctrl);
int mapd_interface_set_renew(struct mapd_interface_ctrl *ctrl);
int mapd_interface_get_bh_ap(struct mapd_interface_ctrl *ctrl,char *list);
int mapd_interface_get_fh_ap(struct mapd_interface_ctrl *ctrl,char *list);
int mapd_interface_get_conn_status(struct mapd_interface_ctrl *ctrl, int *fhbss_status, int *bhsta_status);
int mapd_interface_set_bh_priority(struct mapd_interface_ctrl *ctrl, unsigned char *_2g, unsigned char *_5gl, unsigned char *_5gh);
int mapd_interface_set_txpower_percentage(struct mapd_interface_ctrl *ctrl, char *almac, char *bandIdx, char* txpower);
int mapd_interface_forceChSwitch(struct mapd_interface_ctrl *ctrl, char *almac, char *channel1, char *channel2, char *channel3);
int mapd_interface_off_ch_scan_req(struct mapd_interface_ctrl *ctrl, int argc, char *argv[]);
int mapd_interface_off_ch_scan_result(struct mapd_interface_ctrl *ctrl, char *almac);
int mapd_interface_off_ch_scan_req_noise(struct mapd_interface_ctrl *ctrl, int argc, char *argv[]);
int mapd_interface_tx_higher_layer_data(struct mapd_interface_ctrl *ctrl, char *almac, char *protocol, char *payload_len, char *payload);
int mapd_interface_get_de_dump(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_);
int mapd_interface_trigger_de_dump(struct mapd_interface_ctrl *ctrl, char *al_mac);
int mapd_interface_get_ch_scan_dump(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_);
int mapd_interface_trigger_ch_scan(struct mapd_interface_ctrl *ctrl, char *al_mac);
int mapd_interface_trigger_ch_plan_R2(struct mapd_interface_ctrl *ctrl, char *band);
int mapd_interface_get_ch_score_dump(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_);
#endif
#endif
