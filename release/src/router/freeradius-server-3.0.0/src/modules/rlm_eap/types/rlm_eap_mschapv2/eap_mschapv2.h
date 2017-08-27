#ifndef _EAP_MSCHAPV2_H
#define _EAP_MSCHAPV2_H

RCSIDH(eap_mschapv2_h, "$Id$")

#include "eap.h"

/*
 *	draft-kamath-pppext-eap-mschapv2-00.txt says:
 *
 *	Supplicant		FreeRADIUS
 *			<--	challenge
 *	response	-->
 *			<--	success
 *	success		-->
 *
 *	But what we often see is:
 *
 *	Supplicant		FreeRADIUS
 *			<--	challenge
 *	response	-->
 *			<--	success
 *	ack		-->
 */
#define PW_EAP_MSCHAPV2_ACK		0
#define PW_EAP_MSCHAPV2_CHALLENGE	1
#define PW_EAP_MSCHAPV2_RESPONSE	2
#define PW_EAP_MSCHAPV2_SUCCESS		3
#define PW_EAP_MSCHAPV2_FAILURE		4
#define PW_EAP_MSCHAPV2_CHGPASSWD	7
#define PW_EAP_MSCHAPV2_MAX_CODES	7

#define MSCHAPV2_HEADER_LEN 	5
#define MSCHAPV2_CHALLENGE_LEN  16
#define MSCHAPV2_RESPONSE_LEN  50

typedef struct mschapv2_header_t {
	uint8_t opcode;
	uint8_t mschapv2_id;
	uint8_t ms_length[2];
	uint8_t value_size;
} mschapv2_header_t;

typedef struct mschapv2_opaque_t {
	int		code;
	uint8_t		challenge[MSCHAPV2_CHALLENGE_LEN];
	VALUE_PAIR	*mppe_keys;
	VALUE_PAIR	*reply;
} mschapv2_opaque_t;

#endif /*_EAP_MSCHAPV2_H*/
