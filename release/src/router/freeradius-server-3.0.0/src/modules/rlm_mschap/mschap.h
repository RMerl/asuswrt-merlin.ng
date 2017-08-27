/* Copyright 2006 The FreeRADIUS server project */

#ifndef _MSCHAP_H
#define _MSCHAP_H

RCSIDH(mschap_h, "$Id$")

int mschap_ntpwdhash(uint8_t *out, char const *password);
void mschap_challenge_hash(uint8_t const *peer_challenge,
			    uint8_t const *auth_challenge,
			    char const *user_name, uint8_t *challenge );

void mschap_auth_response(char const *username,
			  uint8_t const *nt_hash_hash,
			  uint8_t const *ntresponse,
			  uint8_t const *peer_challenge, uint8_t const *auth_challenge,
			  char *response);
void mschap_add_reply(REQUEST *request, unsigned char ident,
		      char const *name, char const *value, int len);


#endif /*_MSCHAP_H*/
