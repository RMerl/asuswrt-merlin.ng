/*
 *  rlm_eap_ikev2.c - Handles that are called from eap
 *
 *  This file is part of rlm_eap_ikev2 freeRADIUS module which implements
 *  EAP-IKEv2 protocol functionality.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Copyright (C) 2005-2006 Krzysztof Rzecki <krzysztof.rzecki@ccns.pl>
 *  Copyright (C) 2005-2006 Rafal Mijal <rafal.mijal@ccns.pl>
 *  Copyright (C) 2005-2006 Piotr Marnik <piotr.marnik@ccns.pl>
 *  Copyright (C) 2005-2006 Pawel Matejski <pawel.matejski@ccns.pl>
 *  Copyright 1999-2007 The FreeRADIUS server project
 *
 */

#include <freeradius-devel/radiusd.h>
#include "eap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include <freeradius-devel/rad_assert.h>
#include "logging_impl.h"
#include <EAPIKEv2/connector.h>
#include "ike_conf.h"


#define PW_IKEV2_CHALLENGE	1
#define PW_IKEV2_RESPONSE		2
#define PW_IKEV2_SUCCESS		3
#define PW_IKEV2_FAILURE		4
#define PW_IKEV2_MAX_CODES	4

#define IKEV2_HEADER_LEN 		4
#define IKEV2_MPPE_KEY_LEN     32


static int set_mppe_keys(eap_handler_t *handler)
{
	uint8_t const *p;
	struct IKEv2Session *session;

	session = ((struct IKEv2Data*)handler->opaque)->session;

	if (session->eapKeyData==NULL){
		INFO(IKEv2_LOG_PREFIX "Key session not available!!!");
		return 1;
	}

	p = session->eapKeyData;
	eap_add_reply(handler->request, "MS-MPPE-Recv-Key", p, IKEV2_MPPE_KEY_LEN);
	p += IKEV2_MPPE_KEY_LEN;
	eap_add_reply(handler->request, "MS-MPPE-Send-Key", p, IKEV2_MPPE_KEY_LEN);
	return 0;
}


// Compose Radius like message from table of output bytes
static int ComposeRadMsg(uint8_t *out,u_int32_t olen, EAP_DS *eap_ds){
	eap_ds->request->type.num = PW_EAP_IKEV2;
	eap_ds->request->code = ((struct EAPHeader *)out)->Code;
	if(eap_ds->request->code<=PW_EAP_REQUEST && olen>4) {
	    int len=(int)ntohs(((struct EAPHeader *)out)->Length);
	    eap_ds->request->type.data = talloc_array(eap_ds->request,
						      uint8_t, len);
	    if (!eap_ds->request->type.data) {
		return 1;
	    }
	    memcpy(eap_ds->request->type.data,out+5,len-5);
	    eap_ds->request->type.length = len-5;
	} else {
	    eap_ds->request->type.data=NULL;
	    eap_ds->request->type.length=0;
	}
	return 0;
}




/**
 * Free memory after EAP-IKEv2 module usage
 */

static int ikev2_detach(void *instance)
{
    struct ikev2_ctx *data = (struct ikev2_ctx *) instance;
    if (data) {
	    Free_ikev2_ctx(data);
	    data=NULL;
    }
    return 0;
}

/**
 * Free memory after finished IKEv2 session
 */

static void ikev2_free_opaque(void *opaque)
{
    DEBUG(IKEv2_LOG_PREFIX "Free session data");
    struct IKEv2Data *ikev2_data=(struct IKEv2Data*)opaque;
    if(ikev2_data->session) {
	if(ikev2_data->session->Status!=IKEv2_SST_ESTABLISHED) {
	    DEBUG(IKEv2_LOG_PREFIX "Unfinished IKEv2 session - cleanup!!!");
	    IKEv2EndSession(ikev2_data->i2,ikev2_data->session);
	    ikev2_data->session=NULL;
	} else {
	    DEBUG(IKEv2_LOG_PREFIX "Unfinished IKEv2 session - keep it!!!");
	    ikev2_data->session=NULL;
	}
    }
    int fastDeleted=FreeSessionIfExpired(ikev2_data->i2,time(NULL));
    if(fastDeleted) {
	DEBUG(IKEv2_LOG_PREFIX "Deleted %d expired IKEv2 sessions",fastDeleted);
    }
    free(ikev2_data);
}



/**
 * Configure EAP-ikev2 handler
 */

static int ikev2_attach(CONF_SECTION *conf, void **instance)
{
    char *default_authtype=NULL;
    char *usersfilename=NULL;
    char *server_authtype=NULL;
    char *server_idtype=NULL;

    CONF_PARSER module_config[] = {
	{  "ca_file", PW_TYPE_STRING_PTR,
	    offsetof(ikev2_ctx,trusted),NULL,NULL },
	{  "private_key_file",PW_TYPE_STRING_PTR,
	    offsetof(ikev2_ctx,pkfile),NULL,NULL },
	{  "private_key_password",PW_TYPE_STRING_PTR,
	    offsetof(ikev2_ctx,pkfile_pwd),NULL,NULL },
	{  "certificate_file", PW_TYPE_STRING_PTR,
	    offsetof(ikev2_ctx,certificate_file),NULL,NULL },
	{  "crl_file", PW_TYPE_STRING_PTR,
	    offsetof(ikev2_ctx,crl_file),NULL,NULL },
	{   "id", PW_TYPE_STRING_PTR,
	    offsetof(ikev2_ctx,id),NULL,NULL },
	{  "fragment_size",PW_TYPE_INTEGER,
	    offsetof(ikev2_ctx,max_fragment_size),NULL,IKEv2_DEFAULT_MAX_FRAGMENT_SIZE_STR},
	{  "dh_counter_max", PW_TYPE_INTEGER,
	    offsetof(ikev2_ctx,DHCounterMax),NULL,IKEv2_DEFAULT_dh_counter_max_STR},
	{  "default_authtype",PW_TYPE_STRING_PTR,
	    0,&default_authtype,"both" },
	{  "usersfile",PW_TYPE_FILE_INPUT,
	    0,&usersfilename,"${confdir}/users" },
	{  "server_authtype",PW_TYPE_STRING_PTR,
	    0,&server_authtype,"secret" },
	{  "idtype",PW_TYPE_STRING_PTR,
	    0,&server_idtype,IKEv2_DEFAULT_IDTYPE_STR},
	{  "certreq",PW_TYPE_BOOLEAN,
	    offsetof(ikev2_ctx,sendCertReq),NULL,"no"},
	{  "fast_dh_exchange",PW_TYPE_BOOLEAN,
	    offsetof(ikev2_ctx,enableFastDHEx),NULL,"no"},
	{  "fast_timer_expire",PW_TYPE_INTEGER,
	    offsetof(ikev2_ctx,fastExpire),NULL,"900"},
	{   "enable_fast_reauth",PW_TYPE_BOOLEAN,
	    offsetof(ikev2_ctx,enableFastReconnect),NULL,"yes"},


 	{ NULL, -1, 0, NULL, NULL }	   /* end the list */
     };

    ikev2_set_log_callback(vxlogf);

    struct ikev2_ctx *i2;


    i2 = Create_ikev2_ctx();
    if (!i2) {
	return -1;
    }
    *instance =i2;

    if (cf_section_parse(conf,i2, module_config) < 0) {
	return -1;
    }
    hexalize(&i2->id,&i2->idlen);

    i2->authtype=rad_get_authtype(server_authtype);
    if(!i2->id) {
	ERROR(IKEv2_LOG_PREFIX "'id' configuration option is required!!!");
	return -1;
    }
    switch(i2->authtype) {
	case IKEv2_AUTH_SK:
	    break;
	case IKEv2_AUTH_CERT:
	    if(!i2->certificate_file || !i2->pkfile) {
		ERROR(IKEv2_LOG_PREFIX "'certificate_file' and 'private_key_file' items are required for 'cert' auth type");
		return -1;
	    }
	    if(!file_exists(i2->certificate_file)) {
		ERROR(IKEv2_LOG_PREFIX "Can not open 'certificate_file' %s",i2->certificate_file);
		return -1;
	    }
	    if(!file_exists(i2->pkfile)) {
		ERROR(IKEv2_LOG_PREFIX "Can not open 'private_key_file' %s",i2->pkfile);
		return -1;
	    }

	    break;
    }
    if(!i2->trusted) {
	AUTH(IKEv2_LOG_PREFIX "'ca_file' item not set, client cert based authentication will fail");
    } else {
	if(!file_exists(i2->trusted)) {
	    ERROR(IKEv2_LOG_PREFIX "Can not open 'ca_file' %s",i2->trusted);
	    return -1;
	}
    }
    if(i2->crl_file) {
	if(!file_exists(i2->crl_file)) {
	    ERROR(IKEv2_LOG_PREFIX "Can not open 'crl_file' %s",i2->crl_file);
	    return -1;
	}
    }

    i2->idtype=IdTypeFromName(server_idtype);
    if(i2->idtype<=0) {
	ERROR(IKEv2_LOG_PREFIX "Unsupported 'idtype': %s",server_idtype);
	return -1;
    }

    if(rad_load_proposals(i2,conf)) {
	ERROR(IKEv2_LOG_PREFIX "Failed to load proposals");
	return -1;
    }

    int res=rad_load_credentials(instance, i2,usersfilename,default_authtype);
    if(res==-1) {
	ERROR(IKEv2_LOG_PREFIX "Error while loading users credentials");
	return -1;
    }

    i2->x509_store = NULL;
    if(CertInit(i2)){
	ERROR(IKEv2_LOG_PREFIX "Error while loading certs/crl");
	return -1;
    }

    return 0;
}



/*
 *	Initiate the EAP-ikev2 session by sending a challenge to the peer.
 */


static int ikev2_initiate(void *instance, eap_handler_t *handler)
{
    INFO(IKEv2_LOG_PREFIX "Initiate connection!");
// This is the way for silent discarding behavior
//    handler->request->options|=RAD_REQUEST_OPTION_FAKE_REQUEST;
//    handler->request->options|=RAD_REQUEST_OPTION_DONT_CACHE;
//    handler->request->reply->code=0;
//    return 0;

    struct ikev2_ctx *i2=(struct ikev2_ctx*)instance;


    struct IKEv2Session *session;
    handler->free_opaque=ikev2_free_opaque;


    // try get respondent FASTID
    uint8_t *eap_username = handler->request->username->vp_octets;
    session = FindSessionByFastid(i2, (char const *)eap_username);
    if(!session) {
	if(IKEv2BeginSession( i2, &session, IKEv2_STY_INITIATOR ) != IKEv2_RET_OK) {
	    ERROR(IKEv2_LOG_PREFIX "Can't initialize IKEv2 session.");
	    return 1;
	}
    } else {
	DEBUG(IKEv2_LOG_PREFIX "Fast reconnect procedure start");
    }
    session->timestamp=time(NULL);

    struct IKEv2Data *ikev2_data=IKEv2Data_new(i2,session);
    handler->opaque=ikev2_data;


#if 0
    // print session counter
    if(i2->SessionList) {
	int session_count=0;
	struct IKEv2Session *ss;
	ss=i2->SessionList;
	while(ss) {
	    session_count++;
	    //ERROR("XXX scounter -> fastid=[%s]",ss->fastID);
	    ss=ss->pNext;
	}
	ERROR("Session list contains: %d",session_count);
    }
#endif


    uint8_t *sikemsg=NULL;
    u_int32_t slen=0;

    if( IKEv2ProcessMsg( i2, NULL , &sikemsg, &slen, session) != IKEv2_RET_OK )
    {
	ERROR(IKEv2_LOG_PREFIX "Error while processing IKEv2 message");
	return 1;
    }

    uint8_t *out=NULL;
    u_int32_t olen=0;

    if( slen != 0 )
    {
	session->eapMsgID++;
	olen = CreateIKEv2Message(i2, sikemsg, slen, false, 0, session, &out );
	if( session->fragdata )
	    session->sendfrag = true;
    }
    if (olen>0&&out!=NULL){
	if(ComposeRadMsg(out,olen,handler->eap_ds)){
	    free(out);
	    return 0;
	}
	free(out);
    }


    /*
     *	We don't need to authorize the user at this point.
     *
     *	We also don't need to keep the challenge, as it's
     *	stored in 'handler->eap_ds', which will be given back
     *	to us...
     */
    handler->stage = AUTHENTICATE;
    return 1;
}


/*
 *	Authenticate a previously sent challenge.
 */
static int ikev2_authenticate(void *instance, eap_handler_t *handler)
{

	struct ikev2_ctx *i2=(struct ikev2_ctx*)instance;
	INFO(IKEv2_LOG_PREFIX "authenticate" );

	rad_assert(handler->request != NULL);
	rad_assert(handler->stage == AUTHENTICATE);

	//!!!!!if( hdr->Code == EAP_CODE_RESPONSE && hdr->Id == session->MsgID )
	//!!!!! dorobic sprawdzanie czy to nie potwierdzenie odebrania fragmentu!!!
	EAP_DS *eap_ds=handler->eap_ds;
	if (!eap_ds				      ||
			!eap_ds->response			    ||
			(eap_ds->response->code != PW_IKEV2_RESPONSE)  ||
			eap_ds->response->type.num != PW_EAP_IKEV2    ||
			!eap_ds->response->type.data){
		ERROR(IKEv2_LOG_PREFIX "corrupted data");
		return -1;
	}

	//skladanie pakietu
	uint8_t *in=NULL;
	if(!(in=talloc_array(eap_ds, uint8_t, eap_ds->response->length))){
		ERROR(IKEv2_LOG_PREFIX "alloc error");
		return -1;
	}

	rad_assert(in!=NULL);
	struct EAPHeader *hdr = (struct EAPHeader *)in;

	hdr->Code=eap_ds->response->code;
	hdr->Id=eap_ds->response->id;
	hdr->Length=htons(eap_ds->response->length);
	hdr->Type=eap_ds->response->type.num;
	memcpy(in+5,eap_ds->response->type.data,eap_ds->response->length-5);
	//koniec: skladanie pakietu

	uint8_t *out=NULL;
	u_int32_t olen=0;
	struct IKEv2Data *ikev2_data=(struct IKEv2Data*)handler->opaque;
	struct IKEv2Session *session=ikev2_data->session;
	session->timestamp=time(NULL);

	if( !session->fragdata )
		 session->sendfrag = false;
	if( session->sendfrag && !ParseFragmentAck( in, session ) ){
		session->eapMsgID=eap_ds->response->id+1;
		olen = CreateIKEv2Message( i2, NULL, 0, false, hdr->Id, session, (uint8_t **)&out );
		talloc_free(in);
		if(ComposeRadMsg(out,olen,handler->eap_ds)){
				free(out);
				return 0;
	       	}
		free(out);
		return 1;
	}

	uint8_t *ikemsg;
	u_int32_t len;

	session->eapMsgID=eap_ds->response->id+1;

	if( ParseIKEv2Message( in, &ikemsg, &len, session ) )
	{
		if(ikemsg!=NULL) free (ikemsg);
		handler->eap_ds->request->code=PW_EAP_FAILURE;
		INFO(IKEv2_LOG_PREFIX "Discarded packet");
		return 1;
	}

	if( !ikemsg || !len )     // send fragment ack
	{
		if(session->SK_ready) session->include_integ=1;
		olen = CreateFragmentAck( in, &out, session ); // confirm fragment
		talloc_free(in);
		in=NULL;
		if(ComposeRadMsg(out,olen,handler->eap_ds)){
			free(out);
			return 0;
		}
		free(out);
		return 1;
	}
	talloc_free(in);
	in=NULL;

	uint8_t *sikemsg=NULL;   //out message
	u_int32_t slen=0;

	if( IKEv2ProcessMsg( i2, ikemsg, &sikemsg, &slen, session) != IKEv2_RET_OK )
	{
		INFO(IKEv2_LOG_PREFIX "EAP_STATE_DISCARD");
		//session->State = EAP_STATE_DISCARD;
		free(out);
		return 1;
	}

	free( ikemsg );

	if( slen != 0 ) //if there is there is something to send
	{
		olen = CreateIKEv2Message(i2, sikemsg, slen, false, 0, session, &out );
		//bobo: a to co ?
		if( session->fragdata )
			session->sendfrag = true;
	} else {

		if( session->Status == IKEv2_SST_FAILED )
		{
			INFO(IKEv2_LOG_PREFIX "FAILED");
			olen = CreateResultMessage( false, session, &out );
		}
		if( session->Status == IKEv2_SST_ESTABLISHED )
		{
			INFO(IKEv2_LOG_PREFIX "SUCCESS");
			olen = CreateResultMessage( true, session, &out );
			session->fFastReconnect=i2->enableFastReconnect;


			//bobo:session->eapKeyData jest zle zainicjalizowane tutaj !!!!!!!!!!!!!! nie jest NULL!!!!!!!!!!1!!!!!!!!!!!!!!!!!!!!!!!11
			GenEapKeys(session,EAP_IKEv2_KEY_LEN);
			set_mppe_keys(handler);
		}

		// keep sessions in memory, only reference cleared
		ikev2_data->session=NULL;
	}
	if (olen>0&&out!=NULL){
		if(ComposeRadMsg(out,olen,handler->eap_ds)){
			free(out);
			return 0;
		}
	}
	//eap_ds->request->code = PW_EAP_REQUEST;
	free(out);
	return 1;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 */
rlm_eap_module_t rlm_eap_ikev2 = {
	"eap_ikev2",
	ikev2_attach,			/* attach */
	ikev2_initiate,			/* Start the initial request */
	NULL,				/* authorization */
	ikev2_authenticate,		/* authentication */
	ikev2_detach 			/* detach */
};
