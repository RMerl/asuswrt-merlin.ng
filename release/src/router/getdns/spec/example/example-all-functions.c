#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <getdns_libevent.h>

#define UNUSED_PARAM(x) ((void)(x))

/* The return values */
getdns_return_t retregular;
char * retcharstar;

/* The args */
int    boolarg;
char * charstararg;
char ** charstarptrarg;
getdns_callback_t callbackarg;
uint16_t regulararg;
uint16_t *regularptrarg;
getdns_transaction_t txidarg;
getdns_transaction_t * txidptrarg;
getdns_namespace_t *namespaceptrarg;
getdns_resolution_t resolutionarg;
getdns_redirects_t redirectsarg;
getdns_transport_t transportarg;
getdns_append_name_t appendnamearg;

getdns_data_type * datatypeptrarg;
getdns_bindata ** bindataptrarg;
getdns_dict * dictarg;
getdns_bindata * bindataarg;
getdns_list * listarg;
getdns_dict ** dictptrarg;
getdns_list ** listptrarg;

size_t sizetarg;
size_t * sizetptrarg;
getdns_context *contextarg = NULL;
uint8_t uint8arg;
uint16_t uint16arg;
uint32_t uint32arg;
uint8_t * uint8ptrarg;
uint16_t * uint16ptrarg;
uint32_t * uint32ptrarg;
void * arrayarg;
void * userarg;
void * allocfunctionarg(size_t foo) {UNUSED_PARAM(foo); return NULL; }
void * reallocfunctionarg(void* foo, size_t bar)
	{UNUSED_PARAM(foo); UNUSED_PARAM(bar); return NULL; }
void deallocfunctionarg(void* foo) {UNUSED_PARAM(foo);}
void * extendedallocfunctionarg(void* userarg, size_t foo)
	{UNUSED_PARAM(userarg); UNUSED_PARAM(foo); return NULL; }
void * extendedreallocfunctionarg(void* userarg, void* foo, size_t bar)
	{UNUSED_PARAM(userarg); UNUSED_PARAM(foo); UNUSED_PARAM(bar); return NULL; }
void extendeddeallocfunctionarg(void* userarg, void* foo)
	{UNUSED_PARAM(userarg); UNUSED_PARAM(foo);}
void setcallbackfunctionarg(getdns_context *foo1, getdns_context_code_t foo2)
	{UNUSED_PARAM(foo1);UNUSED_PARAM(foo2);}

int main()
{

retregular = getdns_general(
  contextarg,
  charstararg,
  uint16arg,
  dictarg,
  arrayarg,
  txidptrarg,
  callbackarg
);

retregular = getdns_address(
  contextarg,
  charstararg,
  dictarg,
  arrayarg,
  txidptrarg,
  callbackarg
);

retregular = getdns_hostname(
  contextarg,
  dictarg,
  dictarg,
  arrayarg,
  txidptrarg,
  callbackarg
);

retregular = getdns_service(
  contextarg,
  charstararg,
  dictarg,
  arrayarg,
  txidptrarg,
  callbackarg
);

retregular = getdns_context_create(
  &contextarg,
  boolarg
);

retregular = getdns_context_create_with_memory_functions(
  &contextarg,
  boolarg,
  allocfunctionarg,
  reallocfunctionarg,
  deallocfunctionarg
);

retregular = getdns_context_create_with_extended_memory_functions(
  &contextarg,
  boolarg,
  userarg,
  extendedallocfunctionarg,
  extendedreallocfunctionarg,
  extendeddeallocfunctionarg
);

getdns_context_destroy(
  contextarg
);

retregular = getdns_cancel_callback(
  contextarg,
  txidarg
);

retregular = getdns_general_sync(
  contextarg,
  charstararg,
  uint16arg,
  dictarg,
  &dictarg
);

retregular = getdns_address_sync(
  contextarg,
  charstararg,
  dictarg,
  &dictarg
);

retregular = getdns_hostname_sync(
  contextarg,
  dictarg,
  dictarg,
  &dictarg
);

retregular = getdns_service_sync(
  contextarg,
  charstararg,
  dictarg,
  &dictarg
);

retregular = getdns_list_get_length(listarg, sizetptrarg);
retregular = getdns_list_get_data_type(listarg, sizetarg, datatypeptrarg);
retregular = getdns_list_get_dict(listarg, sizetarg, dictptrarg);
retregular = getdns_list_get_list(listarg, sizetarg, listptrarg);
retregular = getdns_list_get_bindata(listarg, sizetarg, bindataptrarg);
retregular = getdns_list_get_int(listarg, sizetarg, uint32ptrarg);

retregular = getdns_dict_get_names(dictarg, listptrarg);
retregular = getdns_dict_get_data_type(dictarg, charstararg, datatypeptrarg);
retregular = getdns_dict_get_dict(dictarg, charstararg, dictptrarg);
retregular = getdns_dict_get_list(dictarg, charstararg, listptrarg);
retregular = getdns_dict_get_bindata(dictarg, charstararg, bindataptrarg);
retregular = getdns_dict_get_int(dictarg, charstararg, uint32ptrarg);

listarg = getdns_list_create();
listarg = getdns_list_create_with_context(contextarg);
listarg = getdns_list_create_with_memory_functions(
  allocfunctionarg,
  reallocfunctionarg,
  deallocfunctionarg
);
listarg = getdns_list_create_with_extended_memory_functions(
  userarg,
  extendedallocfunctionarg,
  extendedreallocfunctionarg,
  extendeddeallocfunctionarg
);
getdns_list_destroy(listarg);
retregular =  getdns_list_set_dict(listarg, sizetarg, dictarg);
retregular =  getdns_list_set_list(listarg, sizetarg, listarg);
retregular =  getdns_list_set_bindata(listarg, sizetarg, bindataarg);
retregular =  getdns_list_set_int(listarg, sizetarg, uint32arg);

dictarg = getdns_dict_create();
dictarg = getdns_dict_create_with_context(contextarg);
dictarg = getdns_dict_create_with_memory_functions(
  allocfunctionarg,
  reallocfunctionarg,
  deallocfunctionarg
);
dictarg = getdns_dict_create_with_extended_memory_functions(
  userarg,
  extendedallocfunctionarg,
  extendedreallocfunctionarg,
  extendeddeallocfunctionarg
);
getdns_dict_destroy(dictarg);
retregular =  getdns_dict_set_dict(dictarg, charstararg, dictarg);
retregular =  getdns_dict_set_list(dictarg, charstararg, listarg);
retregular =  getdns_dict_set_bindata(dictarg, charstararg, bindataarg);
retregular =  getdns_dict_set_int(dictarg, charstararg, uint32arg);
retregular =  getdns_dict_remove_name(dictarg, charstararg);

retregular =  getdns_convert_fqdn_to_dns_name(
  charstararg,
  bindataptrarg
);

retregular =  getdns_convert_dns_name_to_fqdn(
  bindataarg,
  charstarptrarg
);

retcharstar = getdns_convert_ulabel_to_alabel(
  charstararg
);

retcharstar = getdns_convert_alabel_to_ulabel(
  charstararg
);

retregular = getdns_validate_dnssec(
  listarg,
  listarg,
  listarg
);

retcharstar = getdns_pretty_print_dict(
  dictarg
);

retcharstar = getdns_display_ip_address(
  bindataarg
);

retregular = getdns_context_set_context_update_callback(
  contextarg,
  setcallbackfunctionarg
);

retregular = getdns_context_set_resolution_type(
  contextarg,
  resolutionarg
);

retregular = getdns_context_set_namespaces(
  contextarg,
  sizetarg,
  namespaceptrarg
);

retregular = getdns_context_set_dns_transport(
  contextarg,
  transportarg
);

retregular = getdns_context_set_limit_outstanding_queries(
  contextarg,
  uint16arg
);

retregular = getdns_context_set_timeout(
  contextarg,
  uint16arg
);

retregular = getdns_context_set_follow_redirects(
  contextarg,
  redirectsarg
);

retregular = getdns_context_set_dns_root_servers(
  contextarg,
  listarg
);

retregular = getdns_context_set_append_name(
  contextarg,
  appendnamearg
);

retregular = getdns_context_set_suffix(
  contextarg,
  listarg
);

retregular = getdns_context_set_dnssec_trust_anchors(
  contextarg,
  listarg
);

retregular = getdns_context_set_dnssec_allowed_skew(
  contextarg,
  uint16arg
);

retregular = getdns_context_set_upstream_recursive_servers(
  contextarg,
  listarg
);

retregular = getdns_context_set_edns_maximum_udp_payload_size(
  contextarg,
  uint16arg
);

retregular = getdns_context_set_edns_extended_rcode(
  contextarg,
  uint8arg
);

retregular = getdns_context_set_edns_version(
  contextarg,
  uint8arg
);

retregular = getdns_context_set_edns_do_bit(
  contextarg,
  uint8arg
);

retregular = getdns_context_set_memory_functions(
  contextarg,
  allocfunctionarg,
  reallocfunctionarg,
  deallocfunctionarg
);

retregular = getdns_context_set_extended_memory_functions(
  contextarg,
  userarg,
  extendedallocfunctionarg,
  extendedreallocfunctionarg,
  extendeddeallocfunctionarg
);

return(0); }  /* End of main() */
