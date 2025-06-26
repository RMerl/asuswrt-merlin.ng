#ifndef NMP_API_H
#define NMP_API_H

#ifdef __cplusplus
extern "C" {
#endif

void query_mlo_mac(const char *client_mac, char *mlo_buff, int mlo_buff_len);

#ifdef __cplusplus
}
#endif

#endif // NMP_API_H