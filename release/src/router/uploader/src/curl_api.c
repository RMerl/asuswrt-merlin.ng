/***************************************************************************
 *									_	_ ____	_
 *	Project						___| | | |	_ \| |
 *							   / __| | | | |_) | |
 *							  |	(__| |_| |	_ <| |___
 *							   \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 -	2011, Daniel Stenberg, <daniel@haxx.se>, et	al.
 *
 * This	software is	licensed as	described in the file COPYING, which
 * you should have received	as part	of this	distribution. The terms
 * are also	available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge,	publish, distribute	and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to	do so, under the terms of the COPYING file.
 *
 * This	software is	distributed	on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either	express	or implied.
 *
 ***************************************************************************/
/* An example source code that issues a	HTTP POST and we provide the actual
 * data	through	a read callback.
 */
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <curl_api.h>
#include <log.h>

#define	USE_CHUNKED			//	define this	for	unknown	following size
#define	DISABLE_EXPECT	//	if USE_CHUNKED define above, please	add	this definition
#define SKIP_PEER_VERIFICATION
#define SKIP_HOSTNAME_VERIFICATION
#define CURL_DBG 1

#define CURL_OPT_TIMEOUT_10_SEC 10L
#define CURL_OPT_TIMEOUT_20_SEC 20L
#define CURL_OPT_TIMEOUT_60_SEC 60L

static int g_global_inited = 0;

size_t curl_get_size(CURL* curl) {
	size_t length = 0;
	CURLcode code;
	code = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD , &length); 
	return length;
}

static int curl_debug_func(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp) {
  const char *text;
  (void)handle; /* prevent compiler warning */
 
  switch (type) {
  case CURLINFO_TEXT:
    //fprintf(stderr, "== Info: %s", data);
  default: /* in case a new one is introduced to shock us */
    return 0;
 
  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    return 0;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    return 0;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    return 0;
  }
	
  return 0;
}

int	curl_io(const char* request_type, const char*	web_path, const char* custom_header[], const char* cookie, PRWCB pRWCB) {
  	CURL *curl;
		CURLcode res;

#ifdef USE_CHUNKED
  	// charles add :
		struct curl_slist *chunk = NULL;
#endif
	
		if (!g_global_inited) {  // To avoid multi-thread re-init
			curl_global_init(CURL_GLOBAL_ALL);
			g_global_inited = 1;
		}

  	curl = curl_easy_init();

  	if(curl) {

  		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request_type);
	
			/* Set operation timetou to 60 seconds	*/
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_OPT_TIMEOUT_20_SEC);

			/* Set no signal	*/
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			
			/* First set the URL that is about to receive our POST.	*/
			curl_easy_setopt(curl, CURLOPT_URL,	web_path);

			/* Now specify we want to POST data	*/
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
	    
	    /* we want to use our own read function	*/
			//curl_easy_setopt(curl, CURLOPT_READFUNCTION, pReadCb);
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, pRWCB->read_cb);
	    	
	    /* pointer to pass to our read function	*/
			//curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);
	    curl_easy_setopt(curl, CURLOPT_READDATA, pRWCB->pInput);
			//curl_easy_setopt(curl, CURLOPT_READDATA, pReadData);

			/* Set debug function */
		  curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, curl_debug_func);
	    
			/* get verbose debug output	please */
			curl_easy_setopt(curl, CURLOPT_VERBOSE,	1L);
	    	
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pRWCB->write_cb);
			//	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pWriteCb);
			//		pRWCB->pWriteInfo->curlptr = curl;
			
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pRWCB);
			//curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)pWriteData);

			/*
		   If you use POST to a HTTP	1.1	server,	you	can	send data without knowing
		   the size before starting the POST	if you use chunked encoding. You
		   enable this by adding	a header like "Transfer-Encoding: chunked" with
		   CURLOPT_HTTPHEADER. With HTTP	1.0	or without chunked transfer, you must
		   specify the size in the request.
		 	*/

#ifdef USE_CHUNKED
				{
					//struct curl_slist	*chunk = NULL;

					chunk	= curl_slist_append(chunk, "Transfer-Encoding: chunked");
					res =	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
					/* use curl_slist_free_all() after the *perform()	call to	free this
					   list again	*/
				}
#else

				/* Set the expected	POST size. If you want to POST large amounts of	data,
				   consider	CURLOPT_POSTFIELDSIZE_LARGE	*/
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (curl_off_t)pooh.sizeleft);
    
#ifdef DISABLE_EXPECT
				/*
				   Using	POST with HTTP 1.1 implies the use of a	"Expect: 100-continue"
				   header.  You can disable this	header with	CURLOPT_HTTPHEADER as usual.
					 NOTE:	if you want	chunked	transfer too, you need to combine these	two
					 since	you	can	only set one list of headers with CURLOPT_HTTPHEADER. */

				/* A less good option would	be to enforce HTTP 1.0,	but	that might also
				   have	other implications.	*/
				{
					//struct curl_slist	*chunk = NULL;

					chunk	= curl_slist_append(chunk, "Expect:");
					res =	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
					/* use curl_slist_free_all() after the *perform()	call to	free this
					   list again	*/
				}
#endif //DISABLE_EXPECT
#endif //USE_CHUNKED
    		
#if	defined(USE_CHUNKED) &&	defined(DISABLE_EXPECT)
		    {
		    	// charles add :
					//    		struct curl_slist *chunk = NULL;
					int			i =0;
					while(custom_header[i]){
						// Cdbg(CURL_DBG, "custom header =%s",	custom_header[i]);
						chunk =	curl_slist_append(chunk, custom_header[i]);
						i++;
					}
					res	= curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
				}
#endif

				if(cookie != NULL) {
		        curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
		    }

#ifdef SKIP_PEER_VERIFICATION
				/*
				 * If you want to connect to a site who isn't using a certificate that is
				 * signed by one of the certs in the CA bundle you have, you can skip the
				 * verification of the server's certificate. This makes the connection
				 * A LOT LESS SECURE.
				 *
				 * If you have a CA cert for the server stored someplace else than in the
				 * default bundle, then the CURLOPT_CAPATH option might come handy for
				 * you.
				 */ 
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
 
#ifdef SKIP_HOSTNAME_VERIFICATION
				/*
				 * If the site you're connecting to uses a different host name that what
				 * they have mentioned in their server certificate's commonName (or
				 * subjectAltName) fields, libcurl will refuse to connect. You can skip
				 * this check, but this will make the connection less secure.
				 */ 
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
    		
				/* Perform the request,	res	will get the return	code */
				res	= curl_easy_perform(curl);
		
    		if (res == CURLE_OK)
      		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &pRWCB->code);
    		else
      		pRWCB->code = res;

#ifdef USE_CHUNKED
				if(chunk) curl_slist_free_all(chunk); /* free the list again */
#endif

				/* always cleanup */
				curl_easy_cleanup(curl);
        
    }

		return 0;
}

char *curl_get_status_string(int status) {
	return (char *)curl_easy_strerror(status);
}
