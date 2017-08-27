#ifndef _EAP_LEAP_H
#define _EAP_LEAP_H

RCSIDH(eap_leap_h, "$Id$")

#include "eap.h"

#define PW_LEAP_CHALLENGE	1
#define PW_LEAP_RESPONSE	2
#define PW_LEAP_SUCCESS		3
#define PW_LEAP_FAILURE		4
#define PW_LEAP_MAX_CODES	4

/*
 *  Version + unused + count
 */
#define LEAP_HEADER_LEN 	3

/*
 ****
 * EAP - LEAP doesnot specify code, id & length but chap specifies them,
 *	for generalization purpose, complete header should be sent
 *	and not just value_size, value and name.
 *	future implementation.
 */

/* eap packet structure */
typedef struct leap_packet_raw_t {
	/*
	 *  EAP header, followed by type comes before this.
	 */
	uint8_t version;
	uint8_t unused;
	uint8_t count;
	uint8_t challenge[1];	/* 8 or 24, followed by user name */
} leap_packet_raw_t;

/*
 *	Which is decoded into this.
 */
typedef struct leap_packet {
	unsigned char	code;
	unsigned char	id;
	int		length;
	int		count;
	unsigned char	*challenge;
	int		name_len;
	char		*name;
} leap_packet_t;

/*
 *	The information which must be kept around
 *	during the LEAP session.
 */
typedef struct leap_session_t {
	int		stage;
	uint8_t		peer_challenge[8];
	uint8_t		peer_response[24];
} leap_session_t;

/* function declarations here */

int 		eapleap_compose(REQUEST *request, EAP_DS *auth, leap_packet_t *reply);
leap_packet_t 	*eapleap_extract(REQUEST *request, EAP_DS *eap_ds);
leap_packet_t 	*eapleap_initiate(REQUEST *request, EAP_DS *eap_ds, VALUE_PAIR *user_name);
int		eapleap_stage4(REQUEST *request, leap_packet_t *packet, VALUE_PAIR* password, leap_session_t *session);
leap_packet_t	*eapleap_stage6(REQUEST *request, leap_packet_t *packet, VALUE_PAIR *user_name, VALUE_PAIR* password,
				leap_session_t *session);

void eapleap_lmpwdhash(unsigned char const *password,unsigned char *lmhash);
void eapleap_mschap(unsigned char const *win_password, unsigned char const *challenge, unsigned char *response);

#endif /*_EAP_LEAP_H*/
