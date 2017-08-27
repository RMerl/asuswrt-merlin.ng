#ifndef _LETSENCRYPT_CONTROL_H
#define _LETSENCRYPT_CONTROL_H

int start_letsencrypt(void);
int stop_letsencrypt(void);
int le_acme_main(int argc, char **argv);
int cp_le_cert_key(char *dst_cert, char *dst_key);
int is_le_cert(const char *cert_path);
int cert_key_match(const char *cert_path, const char *key_path);
void run_le_fw_script(void);

#endif