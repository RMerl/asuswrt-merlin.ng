#include <stdio.h>
#include <getdns/getdns.h>
#include <getdns/getdns_extra.h>

int main()
{
	getdns_return_t r;
	getdns_context *ctx = NULL;

	if ((r = getdns_context_create(&ctx, 1)))
		fprintf(stderr, "Could not create context");

	if (ctx)
		getdns_context_destroy(ctx);

	if (r) {
		fprintf(stderr, ": %s\n", getdns_get_errorstr_by_id(r));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
