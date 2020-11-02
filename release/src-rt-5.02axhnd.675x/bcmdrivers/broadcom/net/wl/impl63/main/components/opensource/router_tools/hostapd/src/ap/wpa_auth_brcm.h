#ifndef _wpa_auth_brcm_h_
#define _wpa_auth_brcm_h_

#include "includes.h"
#include "common.h"

#define DOT11_MAX_KEY_SIZE 32

/** PTK key maintained per SCB */
#define RSN_TEMP_ENCR_KEY_LEN 16
#define RSN_KCK_LENGTH	16
#define RSN_KEK_LENGTH	16
#define ETHER_ADDR_LEN	6
#define DHD_IOCTL_MAGIC 0x00444944

typedef struct wpa_ptk_fbt {
	u8 kck[RSN_KCK_LENGTH]; /**< EAPOL-Key Key Confirmation Key (KCK) */
	u8 kek[RSN_KEK_LENGTH]; /**< EAPOL-Key Key Encryption Key (KEK) */
	u8 tk1[RSN_TEMP_ENCR_KEY_LEN]; /**< Temporal Key 1 (TK1) */
	u8 tk2[RSN_TEMP_ENCR_KEY_LEN]; /**< Temporal Key 2 (TK2) */
} wpa_ptk_fbt_t;

/** GTK key maintained per SCB */
typedef struct wpa_bcm_gtk {
	u32 idx;
	u32 key_len;
	u8  key[DOT11_MAX_KEY_SIZE];
} wpa_bcm_gtk_t;

#define WPA2_PMKID_LEN 16

/** FBT Auth Response Data structure */
typedef struct wlc_fbt_auth_resp {
	u8 macaddr[ETHER_ADDR_LEN]; /**< station mac address */
	u8 pad[2];
	u8 pmk_r1_name[WPA2_PMKID_LEN];
	wpa_ptk_fbt_t ptk; /**< pairwise key */
	wpa_bcm_gtk_t gtk; /**< group key */
	u32 ie_len;
	u8 status;  /**< Status of parsing FBT authentication
					Request in application
					*/
	u8 ies[1]; /**< IEs contains MDIE, RSNIE,
					FBTIE (ANonce, SNonce,R0KH-ID, R1KH-ID)
					*/
} wlc_fbt_auth_resp_t;

void
brcm_build_ft_auth_resp_send(void *ctx, const u8 *addr,
	u16 status_code, const u8 *resp_ies, size_t resp_ie_len);
void brcm_add_ft_ds_sta(struct wpa_state_machine *sm,  const u8 *addr,
		u16 status, u8 *resp_ies, size_t resp_ies_len);
#endif /* _wpa_auth_brcm_h_ */
