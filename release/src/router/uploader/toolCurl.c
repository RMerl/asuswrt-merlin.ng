#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
//#include "include/config.h"
//#include "include/mxml.h"

#include "log.h"

#define CURL_DBG 1


#define CRUL_DEBUG_MSGXXX

char servicegateway[64];
char webrelay[64];
char tokenReply[64];

int json_parse(char* ptr);

#ifdef CRUL_DEBUG_MSG
struct data {
  char trace_ascii; /* 1 or 0 */ 
};

static void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex)
{
  size_t i;
  size_t c;
 
  unsigned int width=0x10; 
     
  if(nohex)
    /* without the hex output, we can fit more on screen */ 
    width = 0x40;
 
  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
          text, (long)size, (long)size);
 
  for(i=0; i<size; i+= width) {  
    fprintf(stream, "%4.4lx: ", (long)i); 
    if(!nohex) {
      /* hex not disabled, show it */ 
      for(c = 0; c < width; c++)
        if(i+c < size)
          fprintf(stream, "%02x ", ptr[i+c]);
        else
          fputs("   ", stream);
    }
 
    for(c = 0; (c < width) && (i+c < size); c++) {
      /* check for 0D0A; if found, skip past and start a new line of output */ 
      if (nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
        i+=(c+2-width);
        break;
      }
      fprintf(stream, "%c",
              (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
      /* check again for 0D0A, to avoid an extra \n if it's at width */ 
      if (nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
        i+=(c+3-width);
        break;
      }
    }
    
    fputc('\n', stream); /* newline */ 
  }
  fflush(stream);
}
 
static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
  struct data *config = (struct data *)userp;
  const char *text;
  (void)handle; /* prevent compiler warning */ 
 
  switch (type) {
  case CURLINFO_TEXT:
    fprintf(stderr, "== Info: %s", data);
  default: /* in case a new one is introduced to shock us */ 
    return 0;
 
  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }
 
  dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
  return 0;
}
#endif

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}
                                                                                                                                                
int tool_curl(int api_id,char* api_url, char* authorization, char* cookie, char* payload, char* action, char* service, char* sid, char* token)
{
  //mxml_node_t		*tree,		/* XML tree */
  //              *node;		/* Node which should be in test.xml */

  struct string s;
  init_string(&s); 

#ifdef CRUL_DEBUG_MSG
  struct data config; 
  config.trace_ascii = 1; /* enable ascii tracing */
#endif   
  
  curl_global_init(CURL_GLOBAL_ALL);  
  CURL *curl = curl_easy_init();
  
  if(curl) {

#ifdef CRUL_DEBUG_MSG  
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
    curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);  
    /* the DEBUGFUNCTION has no effect until we enable VERBOSE */ 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif                            
    
    curl_easy_setopt(curl, CURLOPT_URL, api_url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
    
    /* Header */
    struct curl_slist *plist = curl_slist_append(NULL,authorization);  
    if(action!=NULL) curl_slist_append(plist, action);        
    if(service!=NULL) curl_slist_append(plist, service);
    if(sid!=NULL) curl_slist_append(plist, sid);
    if(token!=NULL) curl_slist_append(plist, token);        
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
    
    /* Cookies */
    if(cookie!=NULL) {
      curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
    }
    
    /* Payload */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);  
    
    /* Response Header & Body */
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

	//curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writefunc);
    //curl_easy_setopt(curl, CURLOPT_HEADERDATA, &s);                                //?³¡?Õuªº«ü?


    CURLcode res = curl_easy_perform(curl);

    // Cdbg(CURL_DBG, "response data -> %s", s.ptr);


    char *loc;
    loc = strstr(s.ptr, "X-Omni-Status-Message: SUCCESS");
  

    free(s.ptr);
    curl_easy_cleanup(curl);


    if(loc == NULL) {
        Cdbg(CURL_DBG, "TSDB insert fail");
        return -1;
    } else {
        Cdbg(CURL_DBG, "TSDB insert success");
    }
  } 

  return 0;
}


