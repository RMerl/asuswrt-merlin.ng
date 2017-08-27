/*****************************************************************************
 * Wireless User Tools
 *
 * Wireless Method Declarations
 *****************************************************************************
*/

#if !defined(__IWLSS_H__)
#define __IWLSS_H__


/* methods */

struct iwlss {
	void *(*bind)(void *, void (*interest_vector)(void *ctx, void *priv), \
				  int (*)(void *arg, void *frame, int), void *arg);
									
	int (*unbind)(void *, void *ref);

	int (*get_key_seq)(void *, void *buf, int buflen);

	int (*authorize)(void *, struct ether_addr *ea);

	int (*deauthorize)(void *, struct ether_addr *ea);

	int (*deauthenticate)(void *, struct ether_addr *ea, int reason);

	int (*get_group_rsc)(void *, uint8 *buf, int index);

	int (*plumb_ptk)(void *, struct ether_addr *ea, uint8 *tk, int tk_len,
					 int cipher);

	void (*plumb_gtk)(void *, uint8 *gtk, uint32 gtk_len, uint32 key_index, \
					  uint32 cipher, uint16 rsc_lo, uint32 rsc_hi, \
					  bool primary_key);

	int (*wl_tkip_countermeasures)(void *, int enable);

	int (*set_ssid)(void *, char *ssid);

	int (*disassoc)(void *);

	int (*get_wpacap)(void *, uint8 *cap);

	int (*get_stainfo)(void *, char *macaddr, int len, char *ret_buf, \
					   int ret_buf_len);

	int (*send_frame)(void *, void *pkt, int len);

	int (*get_bssid)(void *, char *ret_buf, int ret_buf_len);

	int (*get_assoc_info)(void *, unsigned char *buf, int length);

	int (*get_assoc_req_ies)(void *, unsigned char *buf, int length);

	int (*get_cur_etheraddr)(void *, uint8 *ret_buf, int ret_buf_len);

	int (*get_wpaie)(void *, uint8 *ret_buf, int ret_buf_len, \
					 struct ether_addr *ea); \

	int (*get_btampkey)(void *, uint8 *ret_buf, int ret_buf_len, \
					 struct ether_addr *ea);

	int (*add_wpsie)(void *, void *ie, int ie_len, unsigned type);
						
	int (*del_wpsie) (void *, unsigned type);

	/* remember to update the initializer list when adding new members */

}; /* struct iwlss */

#define IWLSS_INITIALIZER_LIST(f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12, \
							   f13,f14,f15,f16,f17,f18,f19,f20,f21,f22,f23) \
		{f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19,\
		 f20,f21,f22,f23 \
		}


#endif /* !defined(__IWLSS_H__) */
