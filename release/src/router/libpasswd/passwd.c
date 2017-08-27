#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>
#include <unistd.h>
#include <crypt.h>

int compare_passwd_in_shadow(const char *username, const char *passwd)
{
	char *salt, *correct, *p, *supplied;
	struct spwd *shadow_entry;
	
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
	
	if (supplied == NULL) 
		goto ERROR;

	free(salt);
	return !strcmp(supplied, correct);
	
ERROR:
	if(salt)
		free(salt);
	return 0;
}



