/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_securid.c
 * @brief Supports auth against SecurID servers using OTP h/w tokens.
 *
 * Supports "next-token code" and "new-pin" modes.
 *
 * @copyright 2012  The FreeRADIUS server project
 * @copyright 2012  Alan DeKok <aland@networkradius.com>
 */
#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <ctype.h>

#include "rlm_securid.h"

typedef enum {
	RC_SECURID_AUTH_SUCCESS = 0,
	RC_SECURID_AUTH_FAILURE = -3,
	RC_SECURID_AUTH_ACCESS_DENIED_FAILURE = -4,
	RC_SECURID_AUTH_INVALID_SERVER_FAILURE = -5,
	RC_SECURID_AUTH_CHALLENGE = -17
} SECURID_AUTH_RC;


static const CONF_PARSER module_config[] = {
	{ "timer_expire", PW_TYPE_INTEGER,
	  offsetof(rlm_securid_t, timer_limit), NULL, "600"},
	{ "max_sessions", PW_TYPE_INTEGER,
	  offsetof(rlm_securid_t, max_sessions), NULL, "2048"},
	{ "max_trips_per_session", PW_TYPE_INTEGER,
	  offsetof(rlm_securid_t, max_trips_per_session), NULL, NULL},
	{ "max_round_trips", PW_TYPE_INTEGER,
	  offsetof(rlm_securid_t, max_trips_per_session), NULL, "6"},
	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


static SD_CHAR empty_pin[] = "";

/* comparison function to find session in the tree */
static int securid_session_cmp(void const *a, void const *b)
{
	int rcode;
	SECURID_SESSION const *one = a;
	SECURID_SESSION const *two = b;

	rad_assert(one != NULL);
	rad_assert(two != NULL);

	rcode = fr_ipaddr_cmp(&one->src_ipaddr, &two->src_ipaddr);
	if (rcode != 0) return rcode;

	return memcmp(one->state, two->state, sizeof(one->state));
}


static SECURID_AUTH_RC securidAuth(void *instance, REQUEST *request,
				   char const *username,
				   char const *passcode,
				   char *replyMsgBuffer, size_t replyMsgBufferSize)
{
	rlm_securid_t *inst = (rlm_securid_t *) instance;
	int acm_ret;
	SD_PIN pin_params;
	char new_pin[10];
	char format[30];
	SECURID_SESSION *securid_session = NULL;
	int rc = -1;

	SD_CHAR *securid_user, *securid_pass;

	if (!username) {
		ERROR("SecurID username is NULL");
		return RC_SECURID_AUTH_FAILURE;
	}

	if (!passcode) {
		ERROR("SecurID passcode is NULL for %s user", username);
		return RC_SECURID_AUTH_FAILURE;
	}

	memcpy(&securid_user, &username, sizeof(securid_user));
	memcpy(&securid_pass, &passcode, sizeof(securid_pass));

	*replyMsgBuffer = '\0';

	securid_session = securid_sessionlist_find(inst, request);
	if (!securid_session) {
		/* securid session not found */
		SDI_HANDLE sdiHandle = SDI_HANDLE_NONE;

		acm_ret = SD_Init(&sdiHandle);
		if (acm_ret != ACM_OK) {
			ERROR("Cannot communicate with the ACE/Server");
			return -1;
		}

		acm_ret = SD_Lock(sdiHandle, securid_user);
		if (acm_ret != ACM_OK) {
			ERROR("SecurID: Access denied. Name [%s] lock failed", username);
			return -2;
		}

		acm_ret = SD_Check(sdiHandle, securid_pass, securid_user);
		switch (acm_ret) {
		case ACM_OK:
			/* we are in now */
			RDEBUG("SecurID authentication successful for %s", username);
			SD_Close(sdiHandle);

			return RC_SECURID_AUTH_SUCCESS;

		case ACM_ACCESS_DENIED:
			/* not this time */
			RDEBUG("SecurID Access denied for %s", username);
			SD_Close(sdiHandle);
			return RC_SECURID_AUTH_ACCESS_DENIED_FAILURE;

		case ACM_INVALID_SERVER:
			ERROR("SecurID: Invalid ACE server");
			return RC_SECURID_AUTH_INVALID_SERVER_FAILURE;

		case ACM_NEW_PIN_REQUIRED:
			RDEBUG2("SecurID new pin required for %s", username);

			/* create a new session */
			securid_session = securid_session_alloc();
			securid_session->sdiHandle = sdiHandle; /* save ACE handle for future use */
			securid_session->securidSessionState = NEW_PIN_REQUIRED_STATE;
			securid_session->identity = strdup(username);

			/* Get PIN requirements */
			acm_ret = AceGetPinParams(sdiHandle, &pin_params);

			/* If a system-generated PIN is required */
			if (pin_params.Selectable == CANNOT_CHOOSE_PIN) {
				/* Prompt user to accept a system generated PIN */
				snprintf(replyMsgBuffer, replyMsgBufferSize,
					 "\r\nAre you prepared to accept a new system-generated PIN [y/n]?");
				securid_session->securidSessionState = NEW_PIN_SYSTEM_ACCEPT_STATE;

			} else if (pin_params.Selectable == USER_SELECTABLE) { //may be returned by AM 6.x servers.
				snprintf(replyMsgBuffer, replyMsgBufferSize,
					 "\r\nPress 'y' to generate a new PIN\r\nOR\r\n'n'to enter a new PIN yourself [y/n]");
				securid_session->securidSessionState = NEW_PIN_USER_SELECT_STATE;

			} else {
				if (pin_params.Alphanumeric) {
					strcpy(format, "alphanumeric characters");
				} else {
					strcpy(format, "digits");
				}
				snprintf(replyMsgBuffer, replyMsgBufferSize,
					 " \r\n   Enter your new PIN of %d to %d %s, \r\n		or\r\n   <Ctrl-D> to cancel the New PIN procedure:",
					 pin_params.Min, pin_params.Max, format);
			}

			/* insert new session in the session list */
			securid_sessionlist_add(inst, request, securid_session);

			return RC_SECURID_AUTH_CHALLENGE;

		case ACM_NEXT_CODE_REQUIRED:
			RDEBUG2("Next securid token code required for %s",
				username);

			/* create a new session */
			securid_session = securid_session_alloc();
			securid_session->sdiHandle = sdiHandle;
			securid_session->securidSessionState = NEXT_CODE_REQUIRED_STATE;
			securid_session->identity = strdup(username);

			/* insert new session in the session list */
			securid_sessionlist_add(inst, request, securid_session);

			strlcpy(replyMsgBuffer, "\r\nPlease Enter the Next Code from Your Token:", replyMsgBufferSize);
			return RC_SECURID_AUTH_CHALLENGE;

		default:
			ERROR("SecurID: Unexpected error from ACE/Agent API acm_ret=%d", acm_ret);
			securid_session_free(inst, request, securid_session);
			return RC_SECURID_AUTH_FAILURE;


		}
	} else {
		/* existing session found */
		RDEBUG("Continuing previous session found for user [%s]", username);

		/* continue previous session */
		switch (securid_session->securidSessionState) {
		case NEXT_CODE_REQUIRED_STATE:
			DEBUG2("Securid NEXT_CODE_REQUIRED_STATE: User [%s]", username);
			/* next token code mode */

			acm_ret = SD_Next(securid_session->sdiHandle, securid_pass);
			if (acm_ret == ACM_OK) {
				INFO("Next SecurID token accepted for [%s].", securid_session->identity);
				rc = RC_SECURID_AUTH_SUCCESS;

			} else {
				INFO("SecurID: Next token rejected for [%s].", securid_session->identity);
				rc = RC_SECURID_AUTH_FAILURE;
			}

			/* deallocate session */
			securid_session_free(inst, request, securid_session);
			return rc;

		case NEW_PIN_REQUIRED_STATE:
			RDEBUG2("SecurID NEW_PIN_REQUIRED_STATE for %s",
				username);

			/* save the previous pin */
			if (securid_session->pin) {
				free(securid_session->pin);
				securid_session->pin = NULL;
			}
			securid_session->pin = strdup(passcode);

			strlcpy(replyMsgBuffer, "\r\n		 Please re-enter new PIN:", replyMsgBufferSize);

			/* set next state */
			securid_session->securidSessionState = NEW_PIN_USER_CONFIRM_STATE;

			/* insert the updated session in the session list */
			securid_sessionlist_add(inst, request, securid_session);
			return RC_SECURID_AUTH_CHALLENGE;

		case NEW_PIN_USER_CONFIRM_STATE:
			RDEBUG2("SecurID NEW_PIN_USER_CONFIRM_STATE: User [%s]", username);
			/* compare previous pin and current pin */
			if (!securid_session->pin || strcmp(securid_session->pin, passcode)) {
				RDEBUG2("Pin confirmation failed. Pins do not match [%s] and [%s]",
				       SAFE_STR(securid_session->pin), securid_pass);
				/* pins do not match */

				/* challenge the user again */
				AceGetPinParams(securid_session->sdiHandle, &pin_params);
				if (pin_params.Alphanumeric) {
					strcpy(format, "alphanumeric characters");
				} else {
					strcpy(format, "digits");
				}
				snprintf(replyMsgBuffer, replyMsgBufferSize,
					 " \r\n   Pins do not match--Please try again.\r\n   Enter your new PIN of %d to %d %s, \r\n		or\r\n   <Ctrl-D> to cancel the New PIN procedure:",
					 pin_params.Min, pin_params.Max, format);

				securid_session->securidSessionState = NEW_PIN_REQUIRED_STATE;

				/* insert the updated session in the session list */
				securid_sessionlist_add(inst, request, securid_session);
				rc = RC_SECURID_AUTH_CHALLENGE;

			} else {
				/* pins match */
				RDEBUG2("Pin confirmation succeeded. Pins match");
				acm_ret = SD_Pin(securid_session->sdiHandle, securid_pass);
				if (acm_ret == ACM_NEW_PIN_ACCEPTED) {
					RDEBUG("New SecurID pin accepted for %s.", securid_session->identity);

					securid_session->securidSessionState = NEW_PIN_AUTH_VALIDATE_STATE;

					/* insert the updated session in the session list */
					securid_sessionlist_add(inst, request, securid_session);

					rc = RC_SECURID_AUTH_CHALLENGE;
					strlcpy(replyMsgBuffer, " \r\n\r\nWait for the code on your card to change, then enter new PIN and TokenCode\r\n\r\nEnter PASSCODE:", replyMsgBufferSize);
				} else {
					RDEBUG("SecurID: New SecurID pin rejected for %s.", securid_session->identity);
					SD_Pin(securid_session->sdiHandle, &empty_pin[0]);  /* cancel PIN */


					rc = RC_SECURID_AUTH_FAILURE;

					/* deallocate session */
					securid_session_free(inst, request, securid_session);
				}
			}
			return rc;
		case NEW_PIN_AUTH_VALIDATE_STATE:
			acm_ret = SD_Check(securid_session->sdiHandle, securid_pass, securid_user);
			if (acm_ret == ACM_OK) {
				RDEBUG("New SecurID passcode accepted for %s.",
				       securid_session->identity);
				rc = RC_SECURID_AUTH_SUCCESS;

			} else {
				INFO("SecurID: New passcode rejected for [%s].", securid_session->identity);
				rc = RC_SECURID_AUTH_FAILURE;
			}

			/* deallocate session */
			securid_session_free(inst, request, securid_session);

			return rc;
		case NEW_PIN_SYSTEM_ACCEPT_STATE:
			if (!strcmp(passcode, "y")) {
				AceGetSystemPin(securid_session->sdiHandle, new_pin);

				/* Save the PIN for the next session
				 * continuation */
				if (securid_session->pin) {
					free(securid_session->pin);
					securid_session->pin = NULL;
				}
				securid_session->pin = strdup(new_pin);

				snprintf(replyMsgBuffer, replyMsgBufferSize,
					 "\r\nYour new PIN is: %s\r\nDo you accept this [y/n]?",
					 new_pin);
				securid_session->securidSessionState = NEW_PIN_SYSTEM_CONFIRM_STATE;

				/* insert the updated session in the
				 * session list */
				securid_sessionlist_add(inst, request, securid_session);

				rc = RC_SECURID_AUTH_CHALLENGE;

			} else {
				SD_Pin(securid_session->sdiHandle, &empty_pin[0]); //Cancel new PIN

				/* deallocate session */
				securid_session_free(inst, request,
						     securid_session);

				rc = RC_SECURID_AUTH_FAILURE;
			}

			return rc;

		case NEW_PIN_SYSTEM_CONFIRM_STATE:
			acm_ret = SD_Pin(securid_session->sdiHandle, (SD_CHAR*)securid_session->pin);
			if (acm_ret == ACM_NEW_PIN_ACCEPTED) {
				strlcpy(replyMsgBuffer, " \r\n\r\nPin Accepted. Wait for the code on your card to change, then enter new PIN and TokenCode\r\n\r\nEnter PASSCODE:", replyMsgBufferSize);
				securid_session->securidSessionState = NEW_PIN_AUTH_VALIDATE_STATE;
				/* insert the updated session in the session list */
				securid_sessionlist_add(inst, request, securid_session);
				rc = RC_SECURID_AUTH_CHALLENGE;

			} else {
				SD_Pin(securid_session->sdiHandle, &empty_pin[0]); //Cancel new PIN
				strlcpy(replyMsgBuffer, " \r\n\r\nPin Rejected. Wait for the code on your card to change, then try again.\r\n\r\nEnter PASSCODE:", replyMsgBufferSize);
				/* deallocate session */
				securid_session_free(inst, request,
						     securid_session);
				rc = RC_SECURID_AUTH_FAILURE;
			}

			return rc;

			/* USER_SELECTABLE state should be implemented to preserve compatibility with AM 6.x servers, which can return this state */
		case NEW_PIN_USER_SELECT_STATE:
			if (!strcmp(passcode, "y")) {
				/* User has opted for a system-generated PIN */
				AceGetSystemPin(securid_session->sdiHandle, new_pin);
				snprintf(replyMsgBuffer, replyMsgBufferSize,
					 "\r\nYour new PIN is: %s\r\nDo you accept this [y/n]?",
					 new_pin);
				securid_session->securidSessionState = NEW_PIN_SYSTEM_CONFIRM_STATE;

				/* insert the updated session in the session list */
				securid_sessionlist_add(inst, request,
							securid_session);
				rc = RC_SECURID_AUTH_CHALLENGE;

			} else {
				/* User has opted for a user-defined PIN */
				AceGetPinParams(securid_session->sdiHandle,
						&pin_params);
				if (pin_params.Alphanumeric) {
					strcpy(format, "alphanumeric characters");
				} else {
					strcpy(format, "digits");
				}

				snprintf(replyMsgBuffer, replyMsgBufferSize,
					 " \r\n   Enter your new PIN of %d to %d %s, \r\n		or\r\n   <Ctrl-D> to cancel the New PIN procedure:",
					 pin_params.Min, pin_params.Max, format);
				securid_session->securidSessionState = NEW_PIN_REQUIRED_STATE;

				/* insert the updated session in the session list */
				securid_sessionlist_add(inst, request,
							securid_session);
				rc = RC_SECURID_AUTH_CHALLENGE;
			}

			return rc;

		default:
			ERROR("rlm_securid: Invalid session state %d for user [%s]",
			       securid_session->securidSessionState,
			       username);
			break;
		}
	}

	return 0;

}

/******************************************/
static int mod_detach(void *instance)
{
	rlm_securid_t *inst = (rlm_securid_t *) instance;

	/* delete session tree */
	if (inst->session_tree) {
		rbtree_free(inst->session_tree);
		inst->session_tree = NULL;
	}

	pthread_mutex_destroy(&(inst->session_mutex));

	return 0;
}


static int mod_instantiate(UNUSED CONF_SECTION *conf, void *instance)
{
	rlm_securid_t *inst = instance;

	/*
	 *	Lookup sessions in the tree.  We don't free them in
	 *	the tree, as that's taken care of elsewhere...
	 */
	inst->session_tree = rbtree_create(securid_session_cmp, NULL, 0);
	if (!inst->session_tree) {
		ERROR("rlm_securid: Cannot initialize session tree.");
		return -1;
	}

	pthread_mutex_init(&(inst->session_mutex), NULL);
	return 0;
}


/*
 *	Authenticate the user via one of any well-known password.
 */
static rlm_rcode_t mod_authenticate(void *instance, REQUEST *request)
{
	int rcode;
	rlm_securid_t *inst = instance;
	char  buffer[MAX_STRING_LEN]="";
	char const *username=NULL, *password=NULL;
	VALUE_PAIR *vp;

	/*
	 *	We can only authenticate user requests which HAVE
	 *	a User-Name attribute.
	 */
	if (!request->username) {
		AUTH("rlm_securid: Attribute \"User-Name\" is required for authentication.");
		return RLM_MODULE_INVALID;
	}

	if (!request->password) {
		RAUTH("Attribute \"Password\" is required for authentication.");
		return RLM_MODULE_INVALID;
	}

	/*
	 *	Clear-text passwords are the only ones we support.
	 */
	if (request->password->da->attr != PW_USER_PASSWORD) {
		RAUTH("Attribute \"User-Password\" is required for authentication. Cannot use \"%s\".", request->password->da->name);
		return RLM_MODULE_INVALID;
	}

	/*
	 *	The user MUST supply a non-zero-length password.
	 */
	if (request->password->length == 0) {
		REDEBUG("Password should not be empty");
		return RLM_MODULE_INVALID;
	}

	/*
	 *	shortcuts
	 */
	username = request->username->vp_strvalue;
	password = request->password->vp_strvalue;

	RDEBUG("User [%s] login attempt with password [%s]",
	       username, password);

	rcode = securidAuth(inst, request, username, password,
			    buffer, sizeof(buffer));

	switch (rcode) {
	case RC_SECURID_AUTH_SUCCESS:
		rcode = RLM_MODULE_OK;
		break;

	case RC_SECURID_AUTH_CHALLENGE:
		/* reply with Access-challenge message code (11) */

		/* Generate Prompt attribute */
		vp = paircreate(request->reply, PW_PROMPT, 0);

		rad_assert(vp != NULL);
		vp->vp_integer = 0; /* no echo */
		pairadd(&request->reply->vps, vp);

		/* Mark the packet as a Acceess-Challenge Packet */
		request->reply->code = PW_ACCESS_CHALLENGE;
		RDEBUG("Sending Access-Challenge.");
		rcode = RLM_MODULE_HANDLED;
		break;

	case RC_SECURID_AUTH_FAILURE:
	case RC_SECURID_AUTH_ACCESS_DENIED_FAILURE:
	case RC_SECURID_AUTH_INVALID_SERVER_FAILURE:
	default:
		rcode = RLM_MODULE_REJECT;
		break;
	}

	if (*buffer) pairmake_reply("Reply-Message", buffer, T_OP_EQ);

	return rcode;
}


/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_securid = {
	RLM_MODULE_INIT,
	"securid",
	RLM_TYPE_CHECK_CONFIG_SAFE | RLM_TYPE_HUP_SAFE,   	/* type */
	sizeof(rlm_securid_t),
	module_config,
	mod_instantiate, 		/* instantiation */
	mod_detach, 			/* detach */
	{
		mod_authenticate, 	/* authentication */
		NULL, 			/* authorization */
		NULL, 			/* preaccounting */
		NULL, 			/* accounting */
		NULL, 			/* checksimul */
		NULL, 			/* pre-proxy */
		NULL, 			/* post-proxy */
		NULL			/* post-auth */
	},
};
