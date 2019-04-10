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
	getdns_list    *replies_tree;
	size_t          n_replies, i;

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

	if ((r = getdns_dict_get_list(response, "replies_tree", &replies_tree)))
		fprintf(stderr, "Could not get \"replies_tree\" from response");

	else if ((r = getdns_list_get_length(replies_tree, &n_replies)))
		fprintf(stderr, "Could not get replies_tree\'s length");

	else for (i = 0; i < n_replies && r == GETDNS_RETURN_GOOD; i++) {
		getdns_dict *reply;
		getdns_list *answer;
		size_t       n_answers, j;

		if ((r = getdns_list_get_dict(replies_tree, i, &reply)))
			fprintf(stderr, "Could not get address %zu from just_address_answers", i);

		else if ((r = getdns_dict_get_list(reply, "answer", &answer)))
			fprintf(stderr, "Could not get \"address_data\" from address");

		else if ((r = getdns_list_get_length(answer, &n_answers)))
			fprintf(stderr, "Could not get answer section\'s length");

		else for (j = 0; j < n_answers && r == GETDNS_RETURN_GOOD; j++) {
			getdns_dict    *rr;
			getdns_bindata *address = NULL;

			if ((r = getdns_list_get_dict(answer, j, &rr)))
				fprintf(stderr, "Could net get rr %zu from answer section", j);

			else if (getdns_dict_get_bindata(rr, "/rdata/ipv4_address", &address) == GETDNS_RETURN_GOOD)
				printf("The IPv4 address is ");

			else if (getdns_dict_get_bindata(rr, "/rdata/ipv6_address", &address) == GETDNS_RETURN_GOOD)
				printf("The IPv6 address is ");

			if (address) {
				char *address_str;
				if (!(address_str = getdns_display_ip_address(address))) {
					fprintf(stderr, "Could not convert second address to string");
					r = GETDNS_RETURN_MEMORY_ERROR;
					break;
				}
				printf("%s\n", address_str);
				free(address_str);
			}
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
	getdns_context      *context    = NULL;
	struct event_base   *event_base = NULL;
	getdns_dict         *extensions = NULL;
	char                *query_name = "www.example.com";
	/* Could add things here to help identify this call */
	char                *userarg    = NULL;
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
