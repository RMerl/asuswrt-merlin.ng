/*****************************************************************************
 * Wireless User Tools
 *
 * Wireless helpers
 *****************************************************************************
*/

#if !defined(__WLSS_H__)
#define __WLSS_H__


struct ether_addr;

extern int
wlss_get_key_seq(void *, void *buf, int buflen);

extern int
wlss_authorize(void *, struct ether_addr *ea);

extern int
wlss_deauthorize(void *, struct ether_addr *ea);

extern int
wlss_deauthenticate(void *, struct ether_addr *ea, int reason);
extern int
wlss_get_group_rsc(void *, uint8 *buf, int index);

extern int
wlss_plumb_ptk(void *, struct ether_addr *ea, uint8 *tk, int tk_len, \
			   int cipher);

extern void
wlss_plumb_gtk(void *, uint8 *gtk, uint32 gtk_len, uint32 key_index, \
			   uint32 cipher, uint16 rsc_lo, uint32 rsc_hi, bool primary_key);

extern int
wlss_wl_tkip_countermeasures(void *, int enable);

extern int
wlss_set_ssid(void *, char *ssid);

extern int
wlss_disassoc(void *);

extern int
wlss_get_wpacap(void *, uint8 *cap);

extern int
wlss_get_stainfo(void *, char *macaddr, int len, char *ret_buf, \
				 int ret_buf_len);

extern int
wlss_send_frame(void *, void *pkt, int len);

extern int
wlss_get_bssid(void *, char *ret_buf, int ret_buf_len);

extern int
wlss_get_assoc_info(void *, unsigned char *buf, int length);

extern int
wlss_get_assoc_req_ies(void *, unsigned char *buf, int length);

extern int
wlss_get_cur_etheraddr(void *, uint8 *ret_buf, int ret_buf_len);

extern int
wlss_get_wpaie(void *, uint8 *ret_buf, int ret_buf_len, struct ether_addr *ea);

extern int
wlss_get_event_mask(void *, unsigned char *, size_t);

extern int
wlss_set_event_mask(void *, unsigned char *, size_t);

extern int
wlss_get_btampkey(void *ctx, uint8 *ret_buf, int ret_buf_len,
				struct ether_addr *ea);

extern int
wlss_add_wpsie(void *ctx, void *ie, int ie_len, unsigned type);
	
extern int
wlss_del_wpsie(void *ctx, unsigned type);


#endif /* !defined(__WLSS_H__) */
