#ifndef CFG_CLIENT_LOOKUP_H
#define CFG_CLIENT_LOOKUP_H

typedef enum AMAS_JSON_FORMAT_S {
    AMAS_JSON_PAP_KEY = 0,
    AMAS_JSON_CLIENT_KEY,
} AMAS_DATA_FORMAT_T;

int get_amas_wired_client_info(struct json_object *json_object_ptr, AMAS_DATA_FORMAT_T DATA_FORMAT, struct json_object *wiredClietListFileObj_in);

/**
 * Retrieves the MAC address of the client's **Parent Node** (CAP/RE) from AMAS information.
 *
 * @param client_mac MAC address string of the client device. (e.g., "00:E0:4C:75:BB:91").
 * @param pap_mac_buffer Output buffer for the Parent AP MAC address (18 bytes required). *
 * @param wiredClietListFileObj Input wired client JSON object content.
 * @return int Returns 1 on success (MAC found); returns 0 otherwise.
 */
int amas_re_get_pap_mac(const char *client_mac, char *pap_mac_buffer, struct json_object* wiredClietListFileObj);
#endif // CFG_CLIENT_LOOKUP_H