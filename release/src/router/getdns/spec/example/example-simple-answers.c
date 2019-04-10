#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <getdns_libevent.h>

/* Set up the callback function, which will also do the processing of the results */
void callback(getdns_context        *context,
              getdns_callback_type_t callback_type,
              getdns_dict           *response, 
              void                  *userarg,
              getdns_transaction_t   transaction_id)
{
	getdns_return_t r;  /* Holder for all function returns */
	uint32_t        status;
	getdns_bindata  *address_data;
	char            *first = NULL, *second = NULL;

	(void) context; /* unused parameter */

	printf( "Callback for query \"%s\" with request ID %"PRIu64".\n"
	      , (char *)userarg, transaction_id );

	switch(callback_type) {
	case GETDNS_CALLBACK_CANCEL:
		printf("Transaction with ID %"PRIu64" was cancelled.\n", transaction_id);
		return;
	case GETDNS_CALLBACK_TIMEOUT:
		printf("Transaction with ID %"PRIu64" timed out.\n", transaction_id);
		return;
	case GETDNS_CALLBACK_ERROR:
		printf("An error occurred for transaction ID %"PRIu64".\n", transaction_id);
		return;
	default: break;
	}
	assert( callback_type == GETDNS_CALLBACK_COMPLETE );

	if ((r = getdns_dict_get_int(response, "status", &status)))
		fprintf(stderr, "Could not get \"status\" from response");

	else if (status != GETDNS_RESPSTATUS_GOOD)
		fprintf(stderr, "The search had no results, and a return value of %"PRIu32".\n", status);

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
	if (r) {
		assert( r != GETDNS_RETURN_GOOD );
		fprintf(stderr, ": %d\n", r);
	}
	getdns_dict_destroy(response); 
}

int main()
{
	getdns_return_t      r;  /* Holder for all function returns */
	getdns_context      *context    = NULL;
	struct event_base   *event_base = NULL;
	getdns_dict         *extensions = NULL;
	char                *query_name = "www.example.com";
	/* Could add things here to help identify this call */
	char                *userarg    = query_name;
	getdns_transaction_t transaction_id;

	if ((r = getdns_context_create(&context, 1)))
		fprintf(stderr, "Trying to create the context failed");

	else if (!(event_base = event_base_new()))
		fprintf(stderr, "Trying to create the event base failed.\n");

	else if ((r = getdns_extension_set_libevent_base(context, event_base)))
		fprintf(stderr, "Setting the event base failed");

	else if ((r = getdns_address( context, query_name, extensions
	                            , userarg, &transaction_id, callback)))
		fprintf(stderr, "Error scheduling asynchronous request");

	else {
		printf("Request with transaction ID %"PRIu64" scheduled.\n", transaction_id);
		if (event_base_dispatch(event_base) < 0)
			fprintf(stderr, "Error dispatching events\n");
	}

	/* Clean up */
	if (event_base)
		event_base_free(event_base);

	if (context)
		getdns_context_destroy(context);

	/* Assuming we get here, leave gracefully */
	exit(EXIT_SUCCESS);
}
