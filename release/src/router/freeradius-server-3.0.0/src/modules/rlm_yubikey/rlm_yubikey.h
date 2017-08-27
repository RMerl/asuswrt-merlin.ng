#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <ctype.h>

#include "config.h"

#ifdef HAVE_YKCLIENT
#include <ykclient.h>
#endif

#ifdef HAVE_YUBIKEY
#include <yubikey.h>
#endif

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_yubikey_t {
	char const 		*name;			//!< Instance name.
	int			auth_type;		//!< Our Auth-Type.
	unsigned int		id_len;			//!< The length of the Public ID portion of the OTP string.
	int			decrypt;		//!< Decrypt the OTP string using the yubikey library.
	int			validate;		//!< Validate the OTP string using the ykclient library.
	char const		**uris;			//!< Yubicloud URLs to validate the token against.

#ifdef HAVE_YKCLIENT
	unsigned int		client_id;		//!< Validation API client ID.
	char			*api_key;		//!< Validation API signing key.
	ykclient_t		*ykc;			//!< ykclient configuration.
	fr_connection_pool_t	*conn_pool;		//!< Connection pool instance.
#endif
} rlm_yubikey_t;


/*
 *	decrypt.c - Decryption functions
 */
rlm_rcode_t rlm_yubikey_decrypt(rlm_yubikey_t *inst, REQUEST *request, VALUE_PAIR *otp);

/*
 *	validate.c - Connection pool and validation functions
 */
int rlm_yubikey_ykclient_init(CONF_SECTION *conf, rlm_yubikey_t *inst);

int rlm_yubikey_ykclient_detach(rlm_yubikey_t *inst);

rlm_rcode_t rlm_yubikey_validate(rlm_yubikey_t *inst, REQUEST *request, VALUE_PAIR *otp);
