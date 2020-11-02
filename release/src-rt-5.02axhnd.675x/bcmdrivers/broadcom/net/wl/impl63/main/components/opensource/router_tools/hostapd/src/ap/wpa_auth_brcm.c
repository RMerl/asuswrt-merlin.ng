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
