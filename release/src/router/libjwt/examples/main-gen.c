/* Copyright (C) 2019 Jeremy Thien <jeremy.thien@gmail.com>
   This file is part of the JWT C Library

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <jwt.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

void usage(const char *name)
{
	printf("%s OPTIONS\n", name);
	printf("Options:\n"
			"  -k --key KEY  The private key to use for signing\n"
			"  -a --alg ALG  The algorithm to use for signing\n"
			"  -c --claim KEY=VALUE  A claim to add to JWT\n"
			"  -j --json '{key1:value1}'  A json to add to JWT\n"
			);
	exit(0);
}

int main(int argc, char *argv[])
{
	char *opt_key_name = "test-rsa256.pem";
	int free_key = 0;
	jwt_alg_t opt_alg = JWT_ALG_RS256;
	time_t iat = time(NULL);

	int oc = 0;
	char *optstr = "hk:a:c:j:";
	struct option opttbl[] = {
		{ "help",         no_argument,        NULL, 'h'         },
		{ "key",          required_argument,  NULL, 'k'         },
		{ "alg",          required_argument,  NULL, 'a'         },
		{ "claim",        required_argument,  NULL, 'c'         },
		{ "json",         required_argument,  NULL, 'j'         },
		{ NULL, 0, 0, 0 },
	};

	char *k = NULL, *v = NULL;
	int claims_count = 0;
	int i = 0;
	unsigned char key[10240];
	size_t key_len = 0;
	FILE *fp_priv_key;
	int ret = 0;
	jwt_t *jwt = NULL;
	struct kv {
		char *key;
		char *val;
	} opt_claims[100];
	memset(opt_claims, 0, sizeof(opt_claims));
	char* opt_json = NULL;

	while ((oc = getopt_long(argc, argv, optstr, opttbl, NULL)) != -1) {
		switch (oc) {
		case 'k':
			opt_key_name = strdup(optarg);
			free_key = 1;
			break;

		case 'a':
			opt_alg = jwt_str_alg(optarg);
			if (opt_alg == JWT_ALG_INVAL) {
				fprintf(stderr, "%s is not supported algorithm, using RS256\n", optarg);
				opt_alg = JWT_ALG_RS256;
			}
			break;

		case 'c':
			k = strtok(optarg, "=");
			if (k) {
				v = strtok(NULL, "=");
				if (v) {
					opt_claims[claims_count].key = strdup(k);
					opt_claims[claims_count].val = strdup(v);
					claims_count++;
				}
			}
			break;
		case 'j':
			if (optarg != NULL) {
				opt_json = strdup(optarg);
			}
			break;

		case 'h':
			usage(basename(argv[0]));
			return 0;

		default: /* '?' */
			usage(basename(argv[0]));
			exit(EXIT_FAILURE);
		}
	}

	fprintf(stderr, "jwtgen: privkey %s algorithm %s\n",
			opt_key_name, jwt_alg_str(opt_alg));

	if (opt_alg > JWT_ALG_NONE) {
		fp_priv_key = fopen(opt_key_name, "r");
		if (fp_priv_key == NULL) {
			perror("Failed to open key file");
			goto finish;
		}
		key_len = fread(key, 1, sizeof(key), fp_priv_key);
		fclose(fp_priv_key);
		key[key_len] = '\0';
		fprintf(stderr, "priv key loaded %s (%zu)!\n", opt_key_name, key_len);
	}

	ret = jwt_new(&jwt);
	if (ret != 0 || jwt == NULL) {
		fprintf(stderr, "invalid jwt\n");
		goto finish;
	}

	ret = jwt_add_grant_int(jwt, "iat", iat);
	for (i = 0; i < claims_count; i++) {
		fprintf(stderr, "Adding claim %s with value %s\n", opt_claims[i].key, opt_claims[i].val);
		jwt_add_grant(jwt, opt_claims[i].key, opt_claims[i].val);
	}

	if (opt_json != NULL) {
		ret = jwt_add_grants_json(jwt, opt_json);
		if (ret != 0) {
			fprintf(stderr, "Input json is invalid\n");
			goto finish;
		}
	}

	ret = jwt_set_alg(jwt, opt_alg, opt_alg == JWT_ALG_NONE ? NULL : key, opt_alg == JWT_ALG_NONE ? 0 : key_len);
	if (ret < 0) {
		fprintf(stderr, "jwt incorrect algorithm\n");
		goto finish;
	}

	jwt_dump_fp(jwt, stderr, 1);

	fprintf(stderr, "jwt algo %s!\n", jwt_alg_str(opt_alg));

	char *out = jwt_encode_str(jwt);
	printf("%s\n", out);

	jwt_free_str(out);
finish:
	if (opt_json != NULL)
		free(opt_json);

	jwt_free(jwt);

	if (free_key)
		free(opt_key_name);

	return 0;
}

