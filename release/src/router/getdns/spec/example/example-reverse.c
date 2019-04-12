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
	getdns_list    *answer;
	size_t          n_answers, i;

	(void) context; (void) userarg; /* unused parameters */

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

	if ((r = getdns_dict_get_list(response, "/replies_tree/0/answer", &answer)))
		fprintf(stderr, "Could not get \"answer\" section from first reply in the response");

	else if ((r = getdns_list_get_length(answer, &n_answers)))
		fprintf(stderr, "Could not get replies_tree\'s length");

	else for (i = 0; i < n_answers && r == GETDNS_RETURN_GOOD; i++) {
		getdns_dict    *rr;
		getdns_bindata *dname;
		char           *dname_str;

		if ((r = getdns_list_get_dict(answer, i, &rr)))
			fprintf(stderr, "Could not get rr %zu from answer section", i);

		else if (getdns_dict_get_bindata(rr, "/rdata/ptrdname", &dname))
			continue; /* Not a PTR */

		else if ((r = getdns_convert_dns_name_to_fqdn(dname, &dname_str)))
			fprintf(stderr, "Could not convert PTR dname to string");

		else {
			printf("The dname is %s\n", dname_str);
			free(dname_str);
		}
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
	getdns_context      *context      = NULL;
	struct event_base   *event_base   = NULL;
	getdns_bindata       address_type = { 4, (void *)"IPv4" };
	getdns_bindata       address_data = { 4, (void *)"\x08\x08\x08\x08" };
	getdns_dict         *address      = NULL;
	getdns_dict         *extensions   = NULL;
	/* Could add things here to help identify this call */
	char                *userarg      = NULL;
	getdns_transaction_t transaction_id;

	if ((r = getdns_context_create(&context, 1)))
		fprintf(stderr, "Trying to create the context failed");

	else if (!(event_base = event_base_new()))
		fprintf(stderr, "Trying to create the event base failed.\n");

	else if ((r = getdns_extension_set_libevent_base(context, event_base)))
		fprintf(stderr, "Setting the event base failed");

	else if (!(address = getdns_dict_create()))
		fprintf(stderr, "Could not create address dict.\n");

	else if ((r = getdns_dict_set_bindata(address, "address_type", &address_type)))
		fprintf(stderr, "Could not set address_type in address dict.\n");

	else if ((r = getdns_dict_set_bindata(address, "address_data", &address_data)))
		fprintf(stderr, "Could not set address_data in address dict.\n");

	else if ((r = getdns_hostname( context, address, extensions
	                             , userarg, &transaction_id, callback)))
		fprintf(stderr, "Error scheduling asynchronous request");

	else if (event_base_dispatch(event_base) < 0)
		fprintf(stderr, "Error dispatching events\n");

	/* Clean up */
	if (event_base)
		event_base_free(event_base);

	if (context)
		getdns_context_destroy(context);

	/* Assuming we get here, leave gracefully */
	exit(EXIT_SUCCESS);
}
