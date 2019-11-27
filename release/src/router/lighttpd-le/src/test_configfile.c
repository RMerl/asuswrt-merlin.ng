#include "configfile-glue.c"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

const struct {
    const char *string;
    const char *rmtstr;
    int rmtfamily;
    int expect;
} rmtmask[] = {
    { "1.0.0.1/1",          "1.0.0.1",         AF_INET, 1 }
   ,{ "254.254.254.254/1",  "254.0.0.1",       AF_INET, 1 }
   ,{ "254.254.254.252/31", "254.254.254.253", AF_INET, 1 }
   ,{ "254.254.254.253/31", "254.254.254.254", AF_INET, 0 }
   ,{ "254.254.254.253/32", "254.254.254.254", AF_INET, 0 }
   ,{ "254.254.254.254/32", "254.254.254.254", AF_INET, 1 }
  #ifdef HAVE_IPV6
   ,{ "2001::/3",           "2001::1",         AF_INET6, 1 }
   ,{ "2f01::/5",           "2701::1",         AF_INET6, 0 }
   ,{ "2f01::/32",          "2f01::1",         AF_INET6, 1 }
   ,{ "2f01::/32",          "2f02::1",         AF_INET6, 0 }
   ,{ "2001::1/127",        "2001::1",         AF_INET6, 1 }
   ,{ "2001::1/127",        "2001::2",         AF_INET6, 0 }
   ,{ "2001::2/128",        "2001::2",         AF_INET6, 1 }
   ,{ "2001::2/128",        "2001::3",         AF_INET6, 0 }
   ,{ "1.0.0.1/1",          "::ffff:1.0.0.1",          AF_INET6, 1 }
   ,{ "254.254.254.254/1",  "::ffff:254.0.0.1",        AF_INET6, 1 }
   ,{ "254.254.254.252/31", "::ffff:254.254.254.253",  AF_INET6, 1 }
   ,{ "254.254.254.253/31", "::ffff:254.254.254.254",  AF_INET6, 0 }
   ,{ "254.254.254.253/32", "::ffff:254.254.254.254",  AF_INET6, 0 }
   ,{ "254.254.254.254/32", "::ffff:254.254.254.254",  AF_INET6, 1 }
   ,{ "::ffff:1.0.0.1/97",          "1.0.0.1",         AF_INET, 1 }
   ,{ "::ffff:254.254.254.254/97",  "254.0.0.1",       AF_INET, 1 }
   ,{ "::ffff:254.254.254.252/127", "254.254.254.253", AF_INET, 1 }
   ,{ "::ffff:254.254.254.253/127", "254.254.254.254", AF_INET, 0 }
   ,{ "::ffff:254.254.254.253/128", "254.254.254.254", AF_INET, 0 }
   ,{ "::ffff:254.254.254.254/128", "254.254.254.254", AF_INET, 1 }
  #endif
};

static void test_configfile_addrbuf_eq_remote_ip_mask (void) {
	int i, m;
	buffer * const s = buffer_init();
	char *slash;
	sock_addr rmt;

	for (i = 0; i < (int)(sizeof(rmtmask)/sizeof(rmtmask[0])); ++i) {
	      #ifndef HAVE_INET_PTON
		rmt.ipv4.sin_family = AF_INET;
		rmt.ipv4.sin_addr.s_addr = inet_addr(rmtmask[i].rmtstr);
	      #else
		if (rmtmask[i].rmtfamily == AF_INET) {
			rmt.ipv4.sin_family = AF_INET;
			inet_pton(AF_INET, rmtmask[i].rmtstr, &rmt.ipv4.sin_addr);
		#ifdef HAVE_IPV6
		} else if (rmtmask[i].rmtfamily == AF_INET6) {
			rmt.ipv6.sin6_family = AF_INET6;
			inet_pton(AF_INET6, rmtmask[i].rmtstr, &rmt.ipv6.sin6_addr);
		#endif
		} else {
			continue;
		}
	      #endif
		buffer_copy_string(s, rmtmask[i].string);
		slash = strchr(s->ptr,'/'); assert(slash);
		m = config_addrbuf_eq_remote_ip_mask(NULL, s, slash, &rmt);
		if (m != rmtmask[i].expect) {
			fprintf(stderr, "failed assertion: %s %s %s\n",
				rmtmask[i].string,
				rmtmask[i].expect ? "==" : "!=",
				rmtmask[i].rmtstr);
			exit(-1);
		}
	}

	buffer_free(s);
}

int main (void) {
	test_configfile_addrbuf_eq_remote_ip_mask();

	return 0;
}

/*
 * stub functions (for linking)
 */
void fd_close_on_exec(int fd) { UNUSED(fd); }
