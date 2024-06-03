#ifndef __UPLOAD_UTIL_H__
#define __UPLOAD_UTIL_H__

int get_web_path_len(char* trans_type, char* server, char* append);
char* get_webpath(const char* trans_type, const char* server, const char* append);
void free_webpath(char* webpath);
char* make_str(const char *fmt, ...);
void free_append_data(char* append_data);
#define get_append_data(...) make_str(__VA_ARGS__)

const char* router_check_token_template = 
"{ \"cusid\" : \"%s\","
" \"user_ticket\" : \"%s\","
" \"devicehashmac\" : \"%s\","
" \"sid\" : \"%s\""
"}"
;

const char* router_file_operation_template = 
"{ \"devicehashmac\" : \"%s\","
" \"access_token\" : \"%s\","
" \"file_name\" : \"%s\""
"}"
;

const char* router_list_file_template = 
"{ \"devicehashmac\" : \"%s\","
" \"access_token\" : \"%s\""
"}"
;

const char* router_get_debug_token_template = 
"{ \"mac\" : \"%s\","
" \"token\" : \"%s\""
"}"
;

const char* s3_req_header_templ = 
"Content-Type: application/json";

const char* s3_req_custom_cookie_templ = 
"OMNISTORE_VER=1_0; path=/;sid=%s";

#endif