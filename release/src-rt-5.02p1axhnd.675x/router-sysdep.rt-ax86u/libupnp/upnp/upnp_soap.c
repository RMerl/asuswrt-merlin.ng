/*
 * Broadcom UPnP library SOAP implementation
 *
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: upnp_soap.c 569048 2015-07-07 02:40:50Z $
 */

#include <upnp.h>

/* Find a statevar with a given name */
UPNP_STATE_VAR	*
find_state_var(UPNP_CONTEXT *context, UPNP_SERVICE *service, char *name)
{
	UPNP_STATE_VAR *statevar;

	for (statevar = service->statevar_table;
		 statevar->name;
		 statevar++) {

		if (strcmp(name, statevar->name) == 0)
			break;
	}
	if (statevar->name == 0)
		return 0;

	return statevar;
}

/*
 * Return token before and character pointer after the delimiter.
 * Specially developed for UPnP SOAP.
 */
static char *
strtok_n(char *s, char *delim, char **endp)
{
	char *p = s;
	char *token = NULL;

	/* pre-condition */
	*endp = NULL;

	if (p == NULL)
		return NULL;

	/* consume leading delimiters */
	while (*p) {
		if (strchr(delim, *p) == NULL) {
			token = p;
			break;
		}

		*p++ = 0;
	}

	if (*p == 0)
		return NULL;

	/* make this token */
	while (*p) {
		if (strchr(delim, *p) != NULL)
			break;

		p++;
	}

	if (*p != 0) {
		*p++ = 0;

		/* consume tailing delimiters */
		while (*p) {
			if (strchr(delim, *p) == NULL)
				break;

			*p++ = 0;
		}

		if (*p)
			*endp = p;
	}

	return token;
}

/* Compare two strings ignore case */
static int
struppercmp(char *s1, char *s2)
{
	int i;
	int len = strlen(s1);
	char x = 0, y = 0;

	/* To upper case */
	for (i = 0; i < len+1; i++) {
		x = toupper(s1[i]);
		y = toupper(s2[i]);
		if (x != y)
			break;
	}

	return (x - y);
}

/* Parse version line in an XML file */
static char *
parse_version(char *str)
{
	char *ptr = str;
	char *start;
	char *value;
	int span;

	/* eat leading white space */
	while (*ptr &&
		(*ptr == ' ' ||
		*ptr == '\t' ||
		*ptr == '\r' ||
		*ptr == '\n')) {
		ptr++;
	}

	if (*ptr != '<')
		return NULL;

	/* locate '<' and '>' */
	start = ptr;

	if (strtok_n(start, ">", &value) == NULL)
		return NULL;

	if ((value - start > 5) && memcmp(start, "<?xml", 5) == 0) {
		start = value;
		span = strspn(start, "\r\n");
		start += span;
	}

	return start;
}

/* Parse elments in an XML file */
static char *
parse_element(char *str, char **tag, char **name, char **attr, char **next)
{
	char *ptr = str;
	char *start;
	char *element = NULL;
	char *aux;
	char *value;
	char *value_end = NULL;
	char *next_element;

	/* eat leading white space */
	while (*ptr &&
		(*ptr == ' ' ||
		*ptr == '\t' ||
		*ptr == '\r' ||
		*ptr == '\n')) {
		ptr++;
	}

	if (*ptr != '<')
		return NULL;

	/* locate '<' and '>' */
	start = ptr;

	if (strtok_n(start, ">", &value) == NULL)
		return NULL;

	/* locate attribute */
	strtok_n(start, " \t\r\n", &aux);

	/* parse "<s:element" and convert to "/s:element" */
	ptr = strchr(start, ':');
	if (!ptr) {
		element = start+1;
	}
	else {
		element = ptr+1;
	}

	*start = '/';

	/*
	 * locate "</s:element>";
	 * sometimes, XP forgets to send the balance element
	 */

	/* value can be 0 */
	if (value) {
		/* Find full matched string */
		ptr = value;
		ptr = strstr(ptr, start);
		while (ptr) {
			/* Check "<" and ">" */
			if (*(ptr-1) == '<' && *(ptr + strlen(start)) == '>') {
				/* Terminate value */
				*(ptr-1) = '\0';

				/* Save value end for eat white space */
				value_end = ptr-2;
				break;
			}

			/* Find next string */
			ptr += strlen(start);
			ptr = strstr(ptr, start);
		}
	}
	else {
		ptr = 0;
	}

	if (ptr) {
		ptr += strlen(start) + 1;
		next_element = ptr;

		/* consume leading white spaces */
		while (value <= value_end) {
			if (*value == ' ' ||
			    *value == '\t' ||
			    *value == '\r' ||
			    *value == '\n') {
				/* Skip it */
				value++;
			}
			else {
				break;
			}
		}

		/* consume tailing white spaces */
		while (value_end >= value) {
			if (*value_end == ' ' ||
			    *value_end == '\t' ||
			    *value_end == '\r' ||
			    *value_end == '\n') {
				/*
				 * Null ending the white space,
				 * and moving backward to do
				 * further checking.
				 */
				*value_end = '\0';
				value_end--;
			}
			else {
				break;
			}
		}
	}
	else {
		if (aux && strcmp(aux, "/") == 0) {
			/* <element /> case */
			next_element = value;

			/* value will be null string */
			*aux = 0;
			value = aux;

			/* Reassign aux to NULL */
			aux = NULL;
		}
		else
			next_element = NULL;
	}

	/* output values */
	if (tag) {
		if (element == start+1) {
			*tag = NULL;
		}
		else {
			*tag = start+1;        /* Skip < */
			*(element-1) = 0;      /* Set ":" to NULL */
		}
	}

	if (name)
		*name = element;

	if (attr)
		*attr = aux;

	if (next)
		*next = next_element;

	return value;
}

/* Send SOAP error response */
void
soap_send_error(UPNP_CONTEXT *context, int error)
{
	char time_buf[TIME_BUF_LEN];
	char *err_str;
	int body_len;
	int len;

	switch (error) {
	case SOAP_INVALID_ACTION:
		err_str = "Invalid Action";
		break;

	case SOAP_INVALID_ARGS:
		err_str = "Invalid Argument";
		break;

	case SOAP_ACTION_FAILED:
		err_str = "Action Failed";
		break;

	case SOAP_ARGUMENT_VALUE_INVALID:
		err_str = "Argument Value Invalid";
		break;

	case SOAP_ARGUMENT_VALUE_OUT_OF_RANGE:
		err_str = "Argument Value Out Of Range";
		break;

	case SOAP_OUT_OF_MEMORY:
		err_str = "Out Of Memory";
		break;

	default:
		err_str = context->err_msg;
		break;
	}

	/* format body */
	body_len = snprintf(context->body_buffer, sizeof(context->body_buffer),
			"<s:Envelope "
			"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
			"<s:Body>"
			"<s:Fault>"
			"<faultcode>s:Client</faultcode>"
			"<faultstring>UPnPError</faultstring>"
			"<detail>"
			"<UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">"
			"<errorCode>%d</errorCode>"
			"<errorDescription>%s</errorDescription>"
			"</UPnPError>"
			"</detail>"
			"</s:Fault>"
			"</s:Body>"
			"</s:Envelope>",
			error,
			err_str);

	/* format header */
	upnp_gmt_time(time_buf);  /* get GMT time */
	len = snprintf(context->head_buffer, sizeof(context->head_buffer),
			"HTTP/1.1 500 Internal Server Error\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: text/xml; charset=\"utf-8\"\r\n"
			"Date: %s\r\n"
			"EXT: \r\n"
			"Server: POSIX, UPnP/1.0 %s/%s\r\n"
			"Connection: close\r\n"
			"\r\n"
			"%s",
			body_len,
			time_buf,
			"UPnP Stack",
			UPNP_VERSION_STR,
			context->body_buffer);

	/* send error message */
	if (send(context->fd, context->head_buffer, len, 0) == -1) {
		upnp_syslog(LOG_ERR, "soap_send_error() failed");
	}

	return;
}

static int
soap_action_response_len(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_ACTION *action)
{
	char *p = context->body_buffer;

	int i;
	OUT_ARGUMENT *temp;
	int len;

	/* envlope */
	len = snprintf(context->body_buffer, sizeof(context->body_buffer),
			"<s:Envelope "
			"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
			"<s:Body>\r\n"
			"<u:%sResponse xmlns:u=\"%s:1\">\r\n",
			action->name,
			service->name);
	/* out args */
	for (i = 0; i < action->out_num; i++) {
		temp = context->out_args + i;

		len += strlen("<>") + strlen(temp->name);
		len += temp->tlv.text_strlen;
		len += strlen("</>\r\n") + strlen(temp->name);
	}

	/* closure */
	len += snprintf(p, sizeof(context->body_buffer),
		"</u:%sResponse>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n",
		action->name);

	return len;
}

/* Compose action response message and send */
static void
soap_send_headers(UPNP_CONTEXT *context, int body_len)
{
	char time_buf[TIME_BUF_LEN];
	int head_len;

	/* get GMT time string */
	upnp_gmt_time(time_buf);

	/* Print String */
	head_len = snprintf(context->head_buffer, sizeof(context->head_buffer),
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: text/xml; charset=\"utf-8\"\r\n"
			"Date: %s\r\n"
			"EXT: \r\n"
			"Server: POSIX, UPnP/1.0 %s/%s\r\n"
			"Connection: close\r\n"
			"\r\n",
			body_len,
			time_buf,
			"UPnP Stack",
			UPNP_VERSION_STR);

	send(context->fd, context->head_buffer, head_len, 0);
	return;
}

void
send_action_response(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_ACTION *action)
{
	char *p = context->body_buffer;

	int i;
	OUT_ARGUMENT *temp;
	int body_len;
	int len;

	/* Send headers out */
	body_len = soap_action_response_len(context, service, action);

	soap_send_headers(context, body_len);

	/* Send body envelope */
	len = snprintf(p, sizeof(context->body_buffer),
		"<s:Envelope "
		"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"<s:Body>\r\n"
		"<u:%sResponse xmlns:u=\"%s:1\">\r\n",
		action->name,
		service->name);

	send(context->fd, p, len, 0);

	/* concatenate output values */
	for (i = 0; i < action->out_num; i++) {
		temp = context->out_args + i;

		/* Send <out_argument_name> */
		len = snprintf(p, sizeof(context->body_buffer), "<%s>", temp->name);
		send(context->fd, p, len, 0);

		/* Send out_argument tlv */
		if (temp->tlv.text_strlen)
			send(context->fd, temp->tlv.text, temp->tlv.text_strlen, 0);

		/* Send </out_argument_name> */
		len = snprintf(p, sizeof(context->body_buffer), "</%s>\r\n", temp->name);
		send(context->fd, p, len, 0);
	}

	/* send closure */
	len = snprintf(p, sizeof(context->body_buffer),
		"</u:%sResponse>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n",
		action->name);

	send(context->fd, p, len, 0);
	return;
}

/*
 * SOAP action invoke function.
 *  (1) verify all required input arguments
 *  (2) prepare all output arguements buffer
 *  (3) invoke action function
 *  (4) send back soap action response
 */
int
soap_control(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_ACTION *action)
{
	int error = SOAP_INVALID_ARGS;
	int i;
	OUT_ARGUMENT *temp;

	/* Allocate out argument list */
	if (action->out_num) {
		int size = sizeof(OUT_ARGUMENT) * action->out_num;
		context->out_args = (OUT_ARGUMENT *)calloc(1, size);
		if (context->out_args == NULL)
			return SOAP_OUT_OF_MEMORY;
	}

	/* Prepare out arguments value buffers */
	for (i = 0; i < action->out_num; i++) {
		temp = context->out_args + i;

		temp->name = action->out_argument[i].name;
		temp->statevar = service->statevar_table + action->out_argument[i].related_id;

		/* Prepare value */
		upnp_tlv_init(&temp->tlv, action->out_argument[i].type);
	}

	/* invoke action */
	error = (*action->action)(context, service, context->in_args, context->out_args);
	if (error) {
		/* Some specs need to send notify even with error.
		  * As a result,  the error message from action process
		  * has to be processed sololy before sending notify.
		  */
		soap_send_error(context, error);
		return 0;
	}

	/* Traslate out arguments tlv */
	for (i = 0; i < action->out_num; i++) {
		temp = context->out_args + i;

		/* Traslate the tlv to output string */
		upnp_tlv_translate(&temp->tlv);
	}

	/* Send response out */
	send_action_response(context, service, action);
	return 0;
}

/*
 * Parse the elements and get the action name and argument lists,
 * then invoke SOAP action.
 */
int
action_process(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	char *p = context->content;
	UPNP_ACTION *action;
	char *element;
	char *next;
	int error = SOAP_INVALID_ARGS;
	char *arg_name;
	char *arg_value;
	int i;
	int seq;
	char *soapaction = upnp_msg_get(context, "SOAPACTION");

	IN_ARGUMENT *in_temp;
	OUT_ARGUMENT *out_temp;

	/* Make sure arguments are empty */
	context->out_args = NULL;
	context->in_args = NULL;

	while (*p &&
		(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
		p++;
	}

	if (memcmp(p, "<?xml", 5) == 0) {
		p = parse_version(p);
		if (p == NULL)
			return R_BAD_REQUEST;
	}

	/* process Envelope */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Envelope") != 0)
		return R_BAD_REQUEST;

	/* process Body */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Body") != 0)
		return R_BAD_REQUEST;

	/* process action */
	p = parse_element(p, 0, &element, 0, 0);
	if (strcmp(element, soapaction) != 0)
		return R_BAD_REQUEST;

	/* find action */
	for (action = service->action_table; action->name; action++) {
		if (strcmp(soapaction, action->name) == 0)
			break;
	}
	if (action->name == NULL) {
		error = SOAP_INVALID_ACTION;
		goto done;
	}

	/* Allocate in argument list */
	context->in_arg_num = action->in_num;
	context->out_arg_num = action->out_num;

	if (action->in_num) {
		int size = sizeof(IN_ARGUMENT) * action->in_num;
		context->in_args = (IN_ARGUMENT *)calloc(1, size);
		if (context->in_args == NULL)
			return SOAP_OUT_OF_MEMORY;
	}

	/* Process in argument list */
	next = p;
	seq = 0;
	while (next) {
		/* Reset error code */
		error = SOAP_INVALID_ARGS;

		arg_value = parse_element(next, 0, &arg_name, 0, &next);
		if (!arg_value)
			break;

		/* Matching in in_argument list */
		for (i = 0; i < action->in_num; i++) {
			if (strcmp(arg_name, action->in_argument[i].name) == 0)
				break;
		}
		if (i == action->in_num) {
			/* WAR, patch for XBOX */
			if (seq >= i)
				goto done;
			i = seq;
		}

		/* It is safe, the zero action->in_num won't come here. */
		in_temp = context->in_args + i;
		if (in_temp->name)
			goto done;	/* Duplicate in argument */

		in_temp->name = action->in_argument[i].name;
		in_temp->statevar = service->statevar_table +
			action->in_argument[i].related_id;

		/* Initialize value */
		upnp_tlv_init(&in_temp->tlv, action->in_argument[i].type);

		error = upnp_tlv_convert(&in_temp->tlv, arg_value);
		if (error)
			goto done;

		seq++;
	}

	/* Make sure all in arguments got */
	for (i = 0; i < action->in_num; i++) {
		in_temp = context->in_args + i;
		if (!in_temp->name)
			goto done;
	}

	/* Invoke control action */
	error = soap_control(context, service, action);
	if (error == 0) {
		/*
		 * Check whether we need further
		 * eventing processing?
		 */
		if (service->evented) {
			/* invoke gena function */
			gena_notify(context, service, NULL, NULL);
			gena_notify_complete(context, service);
		}
	}

done:
	/* Free the values of in arguments */
	if (context->in_args) {
		for (i = 0; i < action->in_num; i++) {
			in_temp = context->in_args + i;
			upnp_tlv_deinit(&in_temp->tlv);
		}

		free(context->in_args);
		context->in_args = NULL;
	}

	if (context->out_args) {
		for (i = 0; i < action->out_num; i++) {
			out_temp = context->out_args + i;
			upnp_tlv_deinit(&out_temp->tlv);
		}

		free(context->out_args);
		context->out_args = NULL;
	}

	/* Check return code */
	if (error)
		soap_send_error(context, error);

	/* Always return 0, because soap error has been sent */
	return 0;
}

/* Send SOAP query response */
void
send_query_response(UPNP_CONTEXT *context, char *value, int value_len)
{
	char *p = context->body_buffer;
	int body_len = value_len;
	int len;

	/* Send headers */
	body_len += strlen(
		"<s:Envelope "
		"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"<s:Body>\r\n"
		"<u:QueryStateVariableResponse xmlns:u=\"urn:schemas-upnp-org:control1-1-0\">\r\n"
		"<return>"
		"</return>\r\n"
		"</u:QueryStateVariableResponse>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n");

	soap_send_headers(context, body_len);

	/* Send body envelope */
	len = snprintf(p, sizeof(context->body_buffer),
		"<s:Envelope "
		"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"<s:Body>\r\n"
		"<u:QueryStateVariableResponse xmlns:u=\"urn:schemas-upnp-org:control1-1-0\">\r\n"
		"<return>");

	send(context->fd, p, len, 0);

	/* Send statevar value */
	send(context->fd, value, value_len, 0);

	/* Send closure */
	len = snprintf(p, sizeof(context->body_buffer),
		"</return>\r\n"
		"</u:QueryStateVariableResponse>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n");

	send(context->fd, p, len, 0);
	return;
}

/* Invoke statevar query function */
int
soap_query(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar)
{
	UPNP_TLV tlv;
	int error = 0;

	/* invoke query function */
	upnp_tlv_init(&tlv, statevar->type);

	if (statevar->func) {
		error = (*statevar->func)(context, service, &tlv);
		if (error != 0)
			goto done;
	}

	/* translate tlv to string */
	if (upnp_tlv_translate(&tlv) == NULL) {
		error = R_ERROR;
		goto done;
	}

	send_query_response(context, tlv.text, tlv.text_strlen);

done:
	/* Clean up tlv */
	upnp_tlv_deinit(&tlv);

	if (error != 0 && error != R_ERROR) {
		soap_send_error(context, error);
		return 0;
	}

	return error;
}

/*
 * Parse the elements and get the statevar name,
 * then invoke query.
 */
int
query_process(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar;
	char *p = context->content;
	char *element;

	while (*p &&
		(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
		p++;
	}

	if (memcmp(p, "<?xml", 5) == 0) {
		p = parse_version(p);
		if (p == NULL)
			return R_BAD_REQUEST;
	}

	/* process Envelope */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Envelope") != 0)
		return R_BAD_REQUEST;

	/* process Body */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || struppercmp(element, "Body") != 0)
		return R_BAD_REQUEST;

	/* process QueryStateVariable */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || strcmp(element, "QueryStateVariable") != 0)
		return R_BAD_REQUEST;

	/* process varName */
	p = parse_element(p, 0, &element, 0, 0);
	if (p == NULL || strcmp(element, "varName") != 0)
		return R_BAD_REQUEST;

	/* find state variable */
	statevar = find_state_var(context, service, p);
	if (statevar == NULL) {
		soap_send_error(context, SOAP_INVALID_ARGS);
		return 0;
	}

	return soap_query(context, service, statevar);
}

/*
 * SOAP process entry;
 * Parse header and decide which control function
 * to go.
 */
int
soap_process(UPNP_CONTEXT *context)
{
	char *soapaction = upnp_msg_get(context, "SOAPACTION");
	char *action_field;
	UPNP_SERVICE *service;

	/* find service */
	service = upnp_get_service_by_control_url(context, context->url);
	if (service == NULL)
		return R_NOT_FOUND;

	action_field = soapaction;
	if (action_field == NULL)
		return R_BAD_REQUEST;

	if (strcmp(action_field, "\"urn:schemas-upnp-org:control-1-0#QueryStateVariable\"") == 0 ||
		strcmp(action_field, "urn:schemas-upnp-org:control-1-0#QueryStateVariable") == 0) {

		/* query control */
		return query_process(context, service);
	}
	else {
		/* action control */
		char *action_name;
		int pos, name_len;

		if (*action_field == '\"')
			action_field++;

		/* compare the service name */
		name_len = strlen(service->name);
		if (memcmp(action_field, service->name, name_len) == 0) {
			 /* get action name */
			pos = strcspn(action_field, "#");
			action_field += pos+1;

			action_name = action_field;
			pos = strcspn(action_field, "\"\t ");
			action_name[pos] = 0;

			/* Renew SOAPACTION */
			upnp_msg_save(context, "SOAPACTION", action_name);

			return action_process(context, service);
		}
		else {
			return R_BAD_REQUEST;
		}
	}
}
