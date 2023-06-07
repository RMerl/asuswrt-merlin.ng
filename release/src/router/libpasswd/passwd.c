#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>
#include <unistd.h>
#include <crypt.h>

static int asus_libpasswd_openssl_crypt(char *key, char *salt, char *out, int out_len)
{
	dbg("asus_openssl_crypt: check toolchain crypt() support\n");
	int scheme_id = 0, ret = 0;
	FILE *p_fp = NULL;
	char cmd_line[256] = {0}, crypt_buf[256] = {0};

	if(salt && strlen(salt) > 4){
		if(!strncmp(salt, "$1$", 3))
			scheme_id = 1;
		else if(!strncmp(salt, "$5$", 3))
			scheme_id = 5;
		else
			return ret;
	}else
		return ret;

	snprintf(cmd_line, sizeof(cmd_line), "openssl passwd -%d -salt %s %s", scheme_id, salt+3, key);

	if((p_fp = popen(cmd_line, "r")) != NULL){
		if(fgets(crypt_buf, sizeof(crypt_buf), p_fp)){
			if (strlen(crypt_buf) > 0)
				crypt_buf[strlen(crypt_buf)-1] = '\0';
		}
		pclose(p_fp);
	}

	if(crypt_buf[0] != '\0'){
		if(!strncmp(crypt_buf, salt, strlen(salt))){
			strlcpy(out, crypt_buf, out_len);
			ret = 1;
		}
	}

	return ret;
}

int compare_passwd_in_shadow(const char *username, const char *passwd)
{
	char *salt, *correct, *p, *supplied;
	struct spwd *shadow_entry;
	char crypt_buf[256] = {0};

	if(!username || !passwd)
		return 0;

	shadow_entry = getspnam(username);

	if(!shadow_entry)
		return 0;

	correct = shadow_entry->sp_pwdp;
	
	salt = strdup(correct);

	if (salt == NULL) 
		goto ERROR;
	
	p = strchr(salt + 1, '$');
	if (p == NULL) 
		goto ERROR;
	
	p = strchr(p + 1, '$');
	if (p == NULL) 
		goto ERROR;
	p[1] = 0;	

	supplied = crypt(passwd, salt);
	
	if (supplied == NULL) {
		asus_libpasswd_openssl_crypt(passwd, salt, crypt_buf, sizeof(crypt_buf));
		supplied = crypt_buf;
	}
	if(supplied == NULL || *supplied == '\0')
		goto ERROR;

	free(salt);
	return !strcmp(supplied, correct);
	
ERROR:
	if(salt)
		free(salt);
	return 0;
}



