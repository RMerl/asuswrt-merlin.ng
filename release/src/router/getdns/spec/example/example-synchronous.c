#include <stdio.h>
#include <assert.h>
#include <getdns_core_only.h>

int main()
{
	getdns_return_t  r; /* Holder for all function returns */
	getdns_context  *context    = NULL;
	getdns_dict     *response   = NULL;
	getdns_dict     *extensions = NULL;
	getdns_bindata  *address_data;
	char            *first = NULL, *second = NULL;

	/* Create the DNS context for this call */
	if ((r = getdns_context_create(&context, 1)))
		fprintf(stderr, "Trying to create the context failed");

	else if (!(extensions = getdns_dict_create()))
		fprintf(stderr, "Could not create extensions dict.\n");

	else if ((r = getdns_dict_set_int(extensions, "return_both_v4_and_v6", GETDNS_EXTENSION_TRUE)))
		fprintf(stderr, "Trying to set an extension do both IPv4 and IPv6 failed");

	else if ((r = getdns_general_sync(context, "example.com", GETDNS_RRTYPE_A, extensions, &response)))
		fprintf(stderr, "Error scheduling synchronous request");
	
	else if ((r = getdns_dict_get_bindata(response, "/just_address_answers/0/address_data", &address_data)))
		fprintf(stderr, "Could not get first address");

	else if (!(first = getdns_display_ip_address(address_data)))
		fprintf(stderr, "Could not convert first address to string\n");

	else if ((r = getdns_dict_get_bindata(response, "/just_address_answers/1/address_data", &address_data)))
		fprintf(stderr, "Could not get second address");

	else if (!(second = getdns_display_ip_address(address_data)))
		fprintf(stderr, "Could not convert second address to string\n");

	if (first) {
		printf("The address is %s\n", first);
		free(first);
	}
	if (second) {
		printf("The address is %s\n", second);
		free(second);
	}
	/* Clean up */
	if (response)
		getdns_dict_destroy(response); 

	if (extensions)
		getdns_dict_destroy(extensions);

	if (context)
		getdns_context_destroy(context);

	if (r) {
		assert( r != GETDNS_RETURN_GOOD );

		fprintf(stderr, ": %d\n", r);
		exit(EXIT_FAILURE);
	}
	/* Assuming we get here, leave gracefully */
	exit(EXIT_SUCCESS);
}

