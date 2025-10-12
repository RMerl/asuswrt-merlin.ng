#ifndef NMP_API_H
#define NMP_API_H

#ifdef __cplusplus
extern "C" {
#endif

#define MLO_ALL_MAC_LEN	128

/* Copy each token in wordlist delimited by ascii_60 into word */
#define foreach_60(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, "<")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "<")] = '\0', \
				next = strchr(next, '<'); \
				strlen(word); \
				next = next ? &next[strspn(next, "<")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "<")] = '\0', \
				next = strchr(next, '<'))

void query_mlo_mac(const char *client_mac, char *mlo_buff, int mlo_buff_len);

void query_mlo_all_mac(const char *client_mac, char *mlo_buff, int mlo_buff_len);

void filter_mld_mac(const char *mld_mac, char *raw, int raw_len);

void append_mac_if_unique(char *mlo_all_mac, const char *mac, size_t mlo_all_mac_len);

void check_mlo_mac_and_merge(struct json_object *client, const char *mlo_key, const char *mlo_mac_tmp, char *mlo_all_mac, const int mlo_all_mac_len, int *client_updated);

int process_mld_mac(const char *mld_mac, const char *mlo_buff, char *mlo_mac, size_t mlo_mac_len);

void check_mld_all_mac(const char *mld_mac, char *mlo_all_mac, const int mlo_all_mac_len, int *client_updated);

#ifdef __cplusplus
}
#endif

#endif // NMP_API_H