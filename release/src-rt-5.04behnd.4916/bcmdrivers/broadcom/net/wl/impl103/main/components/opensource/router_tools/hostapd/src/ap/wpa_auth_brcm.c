#include "includes.h"
#include <sys/socket.h>
#include <net/if.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include "common.h"
#include "hostapd.h"
#include "brcm_nl80211.h"
#include "sta_info.h"
#include "wpa_auth.h"
#include "wpa_auth_i.h"
#include "common/ieee802_11_defs.h"
#include "ap_drv_ops.h"
#include "wpa_auth_brcm.h"

#define WLC_GET_VAR		262 /* get value of named variable */
#define WLC_SET_VAR		263 /* set named variable to value */
#define FALSE			0
#define PASN_CMD_LEN		4
#define WPA_TK_MAX_LEN		32
#define WPA_LTF_KEYSEED_MAX_LEN 48

/* Table 9-149 from 2020 standard */
#define WPA_CIPHER_SUITE_NONE1        BIT(0)    /* None */
#define WPA_CIPHER_SUITE_WEP_40       BIT(1)    /* WEP (40-bit) */
#define WPA_CIPHER_SUITE_TKIP1        BIT(2)    /* TKIP: default for WPA */
#define WPA_CIPHER_SUITE_AES_OCB      BIT(3)    /* AES (OCB) */
#define WPA_CIPHER_SUITE_AES_CCM      BIT(4)    /* AES (CCM) */
#define WPA_CIPHER_SUITE_WEP_104      BIT(5)    /* WEP (104-bit) */
#define WPA_CIPHER_SUITE_BIP          BIT(6)    /* WEP (104-bit) */
#define WPA_CIPHER_SUITE_TPK          BIT(7)    /* Group addressed traffic not allowed */
#define WPA_CIPHER_SUITE_AES_GCM      BIT(8)    /* AES (GCM) */
#define WPA_CIPHER_SUITE_AES_GCM256   BIT(9)    /* AES (GCM256) */
#define WPA_CIPHER_SUITE_CCMP_256     BIT(10)   /* CCMP-256 */

typedef struct pasn_secure_ranging_params {
	u16 sid;
        u8  own_addr[ETHER_ADDR_LEN];
        u8  peer_addr[ETHER_ADDR_LEN];
        u32 cipher;
        u16  tk_len;
        u8  tk[WPA_TK_MAX_LEN];
        u16  ltf_keyseed_len;
        u8  ltf_keyseed[WPA_LTF_KEYSEED_MAX_LEN];
	u16 kdk_len;
	u8  kdk[WPA_KDK_MAX_LEN];
} pasn_secure_ranging_params_t;

typedef u16 wl_pasn_cmd_t;
typedef u16 wl_pasn_session_id_t;
#define WLC_IOCTL_MEDLEN		1536    /* "med" length ioctl buffer required */

typedef struct wl_pasn_iov {
	u16			version; /* structure version will be incremented
					* when header is changed.
					*/
	u16			len;	/* includes the id field and following data. */
	wl_pasn_cmd_t		cmd;	/* sub-command id. */
	wl_pasn_session_id_t	id;	/* session id is the input for session specific commands
					* and a reserved value for global commands.
					*/
	u8			data[];	/* variable */
} wl_pasn_iov_t;

#define PASN_HDR_SIZE   offsetof(wl_pasn_iov_t, id)

#define WL_PASN_VERSION_1	 0x0001
#define	WL_PASN_CMD_SET_SESSION_KEY  19

/**Linux network driver ioctl encoding */
typedef struct wl_ioctl {
        u32 cmd;     /**< common ioctl definition */
        void *buf;      /**< pointer to user buffer */
        u32 len;     /**< length of user buffer */
        u8 set;              /**< 1=set IOCTL; 0=query IOCTL */
        u32 used;    /**< bytes read or written (optional) */
        u32 needed;  /**< bytes needed (optional) */
} wl_ioctl_t;

static int
wl_ioctl(char *name, int cmd, void *buf, int len)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s;
	char buffer[100];

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;

	/* initializing the remaining fields */
	ioc.set = FALSE;
	ioc.used = 0;
	ioc.needed = 0;

	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
	ifr.ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		if ((cmd == WLC_GET_VAR) || (cmd == WLC_SET_VAR)) {
			snprintf(buffer, sizeof(buffer), "%s: WLC_%s_VAR(%s)", name,
			         cmd == WLC_GET_VAR ? "GET" : "SET", (char *)buf);
		}
		perror(buffer);
	}
	/* cleanup */
	close(s);
	return ret;
}

void
brcm_build_ft_auth_resp_send(void *ctx, const u8 *addr,
		u16 status_code, const u8 *ies, size_t ies_len)
{
	struct sta_info *sta;
	wlc_fbt_auth_resp_t *ft_authresp;
	struct wpa_state_machine *sm;
	struct wpa_group *gsm;
	int len, cmd_len = 0;
	struct hostapd_data *hapd = ctx;
	u8 *resp_ies = NULL;

	sta = ap_get_sta(hapd, addr);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "BRCM: Failed to get station info " MACSTR "\n",
				MAC2STR(addr));
		return;
	}
	sm = sta->wpa_sm;

	cmd_len = strlen("fbt_auth_resp") + 1;

	len = cmd_len +  sizeof(wlc_fbt_auth_resp_t) - sizeof(ft_authresp->ies) + ies_len;

	resp_ies = os_zalloc(len);

	if (resp_ies == NULL) {
		wpa_printf(MSG_DEBUG, "BRCM: Failed to allocate memory "
				"for response ies for ft auth resp\n");
		return;
	}

	memcpy(resp_ies, "fbt_auth_resp", cmd_len);
	ft_authresp = (wlc_fbt_auth_resp_t *) (resp_ies + cmd_len);
	memcpy(ft_authresp->macaddr, addr, ETHER_ADDR_LEN);
	memcpy(ft_authresp->pmk_r1_name, sm->pmk_r1_name, WPA_PMK_NAME_LEN);
	ft_authresp->ie_len = ies_len;
	memcpy(ft_authresp->ies, ies, ies_len);

	memcpy(ft_authresp->ptk.kck, sm->PTK.kck, sm->PTK.kck_len);
	memcpy(ft_authresp->ptk.kek, sm->PTK.kek, sm->PTK.kek_len);
	memcpy(ft_authresp->ptk.tk1, sm->PTK.tk, sizeof(ft_authresp->ptk.tk1));
	memcpy(ft_authresp->ptk.tk2, sm->PTK.tk + sizeof(ft_authresp->ptk.tk1),
		sizeof(ft_authresp->ptk.tk2));

	gsm = sm->group;
	if (gsm == NULL) {
		wpa_printf(MSG_DEBUG, "BRCM: Failed to find GTK "
				"state machine for ft auth resp\n");
		free(resp_ies);
		return;
	}
	ft_authresp->gtk.idx = gsm->GN;
	ft_authresp->gtk.key_len = gsm->GTK_len;
	memcpy(ft_authresp->gtk.key, gsm->GTK[gsm->GN - 1], gsm->GTK_len);

	ft_authresp->status = status_code;
	wl_ioctl(hapd->conf->iface, WLC_SET_VAR, resp_ies, len);
	free(resp_ies);
	return;
}

void brcm_add_ft_ds_sta(struct wpa_state_machine *sm,
		const u8 *addr, u16 status, u8 *ies, size_t ies_len)
{
	struct sta_info *sta;
	wlc_fbt_auth_resp_t *ft_authresp;
	struct wpa_group *gsm;
	int len = 0, cmd_len = 0;
	struct hostapd_data *hapd = sm->wpa_auth->cb_ctx;
	u8 *resp_ies = NULL;

	sta = ap_get_sta(hapd, addr);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "BRCM: Failed to get station info " MACSTR "\n",
				MAC2STR(addr));
		return;
	}

	wpa_printf(MSG_DEBUG, "BRCM: Set ds ft add station " MACSTR "\n",
			MAC2STR(addr));

	cmd_len = strlen("fbt_ds_add_sta") + 1;

	len = cmd_len + sizeof(wlc_fbt_auth_resp_t) - sizeof(ft_authresp->ies) + ies_len;

	resp_ies = os_zalloc(len);

	if (resp_ies == NULL) {
		wpa_printf(MSG_DEBUG, "BRCM: Failed to allocate memory "
			"for response ies \n");
		return;
	}

	memcpy(resp_ies, "fbt_ds_add_sta", cmd_len);
	ft_authresp = (wlc_fbt_auth_resp_t *) (resp_ies + cmd_len);
	memcpy(ft_authresp->macaddr, addr, ETHER_ADDR_LEN);
	memcpy(ft_authresp->pmk_r1_name, sm->pmk_r1_name, WPA_PMK_NAME_LEN);

	ft_authresp->ie_len = ies_len;
	memcpy(ft_authresp->ies, ies, ies_len);

	memcpy(ft_authresp->ptk.kck, sm->PTK.kck, sm->PTK.kck_len);
	memcpy(ft_authresp->ptk.kek, sm->PTK.kek, sm->PTK.kek_len);
	memcpy(ft_authresp->ptk.tk1, sm->PTK.tk, sizeof(ft_authresp->ptk.tk1));
	memcpy(ft_authresp->ptk.tk2, sm->PTK.tk + sizeof(ft_authresp->ptk.tk1),
		sizeof(ft_authresp->ptk.tk2));

	gsm = sm->group;
	if (gsm == NULL) {
		wpa_printf(MSG_DEBUG, "BRCM: Failed to find GTK "
			"state machine \n");
		free(resp_ies);
		return;
	}
	ft_authresp->gtk.idx = gsm->GN;
	ft_authresp->gtk.key_len = gsm->GTK_len;
	memcpy(ft_authresp->gtk.key, gsm->GTK[gsm->GN - 1], gsm->GTK_len);

	ft_authresp->status = status;
	wl_ioctl(hapd->conf->iface, WLC_SET_VAR, resp_ies, len);
	free(resp_ies);
	return;
}

#ifdef CONFIG_PASN
static int brcm_get_cipher_suite_selector(u32 ciphers)
{
	if (ciphers & WPA_CIPHER_CCMP)
		return WPA_CIPHER_SUITE_AES_CCM;
	if (ciphers & WPA_CIPHER_CCMP_256)
		return WPA_CIPHER_SUITE_CCMP_256;
	if (ciphers & WPA_CIPHER_GCMP_256)
		return WPA_CIPHER_SUITE_AES_GCM256;
	if (ciphers & WPA_CIPHER_GCMP)
		return WPA_CIPHER_SUITE_AES_GCM;
	if (ciphers & WPA_CIPHER_GTK_NOT_USED)
		return WPA_CIPHER_SUITE_TPK;
	if (ciphers & WPA_CIPHER_TKIP)
		return WPA_CIPHER_SUITE_TKIP1;

	return WPA_CIPHER_SUITE_NONE1;
}

int brcm_set_secure_ranging_ctx(struct hostapd_data *hapd,
				const u8 *own_addr, const u8 *peer_addr,
				u32 cipher, u8 tk_len, const u8 *tk,
				u8 ltf_keyseed_len,
				const u8 *ltf_keyseed,
				u8 kdk_len, const u8 *kdk)
{
        pasn_secure_ranging_params_t *params;
	int len = 0;
	u8 *pasn_cmd = NULL;
	u16 sid = 1; //invalid session id
	wl_pasn_iov_t *pasn_hdr;

        if (!hapd->driver)
                return 0;

	pasn_cmd = (u8 *)calloc(1, WLC_IOCTL_MEDLEN);

	if (pasn_cmd == NULL) {
		wpa_printf(MSG_ERROR, "%s:%d secure ranging context alloc fail\n",
				__FUNCTION__, __LINE__);
		return -1;
	}
	memset(pasn_cmd, 0, WLC_IOCTL_MEDLEN);
	memcpy(pasn_cmd, "pasn", strlen("pasn"));
	pasn_hdr = (wl_pasn_iov_t *)(pasn_cmd + strlen("pasn") + 1);
	pasn_hdr->version = WL_PASN_VERSION_1;
	pasn_hdr->cmd = WL_PASN_CMD_SET_SESSION_KEY;
	pasn_hdr->id = sid;

	len = strlen("pasn") + 1 +  PASN_HDR_SIZE + sizeof(pasn_secure_ranging_params_t);
	pasn_hdr->len = sizeof(wl_pasn_session_id_t) + sizeof(pasn_secure_ranging_params_t);

	params = (pasn_secure_ranging_params_t *) (pasn_hdr->data);
	params->sid = sid;
	memcpy(params->own_addr, own_addr, ETHER_ADDR_LEN);
	memcpy(params->peer_addr, peer_addr, ETHER_ADDR_LEN);
	params->cipher = brcm_get_cipher_suite_selector(cipher);
        params->tk_len = tk_len;
	memcpy(params->tk, tk, tk_len);
	params->ltf_keyseed_len = ltf_keyseed_len;
	memcpy(params->ltf_keyseed, ltf_keyseed, ltf_keyseed_len);
	params->kdk_len = kdk_len;
	memcpy(params->kdk, kdk, kdk_len);

	wpa_printf(MSG_DEBUG, "%s:pasn_cmd len %d\n",
			__FUNCTION__, len);
	wpa_hexdump(MSG_DEBUG, "pasn cmd", pasn_cmd, len);
	wl_ioctl(hapd->conf->iface, WLC_SET_VAR, pasn_cmd, len);
	free(pasn_cmd);

	return 0;
}
#endif /* CONFIG_PASN */
