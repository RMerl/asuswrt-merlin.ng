#ifndef _EAP_MD5_H
#define _EAP_MD5_H

RCSIDH(eap_md5_h, "$Id$")

#include "eap.h"

#define PW_MD5_CHALLENGE	1
#define PW_MD5_RESPONSE		2
#define PW_MD5_SUCCESS		3
#define PW_MD5_FAILURE		4
#define PW_MD5_MAX_CODES	4

#define MD5_HEADER_LEN 		4
#define MD5_CHALLENGE_LEN 	16

/*
 ****
 * EAP - MD5 doesnot specify code, id & length but chap specifies them,
 *	for generalization purpose, complete header should be sent
 *	and not just value_size, value and name.
 *	future implementation.
 *
 *	Huh? What does that mean?
 */

/* eap packet structure */
typedef struct md5_packet_t {
/*
	uint8_t	code;
	uint8_t	id;
	uint16_t	length;
*/
	uint8_t	value_size;
	uint8_t	value_name[1];
} md5_packet_t;

typedef struct md5_packet {
	unsigned char	code;
	unsigned char	id;
	unsigned short	length;
	unsigned char	value_size;
	unsigned char	*value;
	char		*name;
} MD5_PACKET;

/* function declarations here */

int 		eapmd5_compose(EAP_DS *auth, MD5_PACKET *reply);
MD5_PACKET 	*eapmd5_extract(EAP_DS *auth);
int 		eapmd5_verify(MD5_PACKET *pkt, VALUE_PAIR* pwd, uint8_t *ch);
#endif /*_EAP_MD5_H*/
