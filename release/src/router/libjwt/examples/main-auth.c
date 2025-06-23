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
#include <errno.h>
#include <time.h>
#include <libgen.h>

void usage(const char *name)
{
	/* TODO Might want to support JWT input via stdin */
	printf("%s --key some-pub.pem --alg RS256 some-file.jwt\n", name);
	printf("Options:\n"
			"  -k --key KEY  The key to use for verification\n"
			"  -a --alg ALG  The algorithm to use for verification\n"
			"  -c --claim KEY=VALUE   Verify JWT has claim KEY with VALUE\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int exit_status = 0;
	char opt_key_name[200] = "test-rsa256-pub.pem";
	jwt_alg_t opt_alg = JWT_ALG_RS256;
	char opt_jwt_name[200] = "test-rsa256.jwt";
	int claims_count = 0;
	int i = 0;
	unsigned char key[10240];
	size_t key_len;
	FILE *fp_pub_key;
	char jwt_str[2048];
	size_t jwt_len;
	FILE *fp_jwt;
	int ret = 0;
	jwt_valid_t *jwt_valid;
	jwt_t *jwt = NULL;
	int oc = 0;
	char *optstr = "hk:a:c:";
	struct option opttbl[] = {
		{ "help",         no_argument,        NULL, 'h'         },
		{ "key",          required_argument,  NULL, 'k'         },
		{ "alg",          required_argument,  NULL, 'a'         },
		{ "claim",        required_argument,  NULL, 'c'         },
		{ NULL, 0, 0, 0 },
	};
	char *k = NULL, *v = NULL;
	struct kv {
		char *key;
		char *val;
	} opt_claims[100];
	memset(opt_claims, 0, sizeof(opt_claims));


	while ((oc = getopt_long(argc, argv, optstr, opttbl, NULL)) != -1) {
		switch (oc) {
		case 'k':
			strncpy(opt_key_name, optarg, sizeof(opt_key_name));
			opt_key_name[sizeof(opt_key_name) - 1] = '\0';
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

		case 'h':
			usage(basename(argv[0]));
			return 0;

		default: /* '?' */
			usage(basename(argv[0]));
			exit(EXIT_FAILURE);
		}
	}


	if (optind == argc) {
		fprintf(stderr, "Please provide name of jwt file\n");
		exit(EXIT_FAILURE);
	}
	strncpy(opt_jwt_name, argv[optind], sizeof(opt_jwt_name) - 1);
	opt_jwt_name[sizeof(opt_jwt_name) - 1] = '\0';

	fprintf(stderr, "jwt verification: jwt %s pubkey %s algorithm %s\n",
			opt_jwt_name, opt_key_name, jwt_alg_str(opt_alg));

	/* Load pub key */
	fp_pub_key = fopen(opt_key_name, "r");
	key_len = fread(key, 1, sizeof(key), fp_pub_key);
	fclose(fp_pub_key);
	key[key_len] = '\0';
	fprintf(stderr, "pub key loaded %s (%zu)!\n", opt_key_name, key_len);

	/* Load jwt */
	fp_jwt = fopen(opt_jwt_name, "r");
	jwt_len = fread(jwt_str, 1, sizeof(jwt_str), fp_jwt);
	fclose(fp_jwt);
	jwt_str[jwt_len] = '\0';
	fprintf(stderr, "jwt loaded %s (%zu)!\n", opt_jwt_name, jwt_len);

	/* Setup validation */
	ret = jwt_valid_new(&jwt_valid, opt_alg);
	if (ret != 0 || jwt_valid == NULL) {
		fprintf(stderr, "failed to allocate jwt_valid\n");
		goto finish_valid;
	}

	jwt_valid_set_headers(jwt_valid, 1);
	jwt_valid_set_now(jwt_valid, time(NULL));
	for (i = 0; i < claims_count; i++) {
		jwt_valid_add_grant(jwt_valid, opt_claims[i].key, opt_claims[i].val);
	}

	/* Decode jwt */
	ret = jwt_decode(&jwt, jwt_str, key, key_len);
	if (ret != 0 || jwt == NULL) {
		fprintf(stderr, "invalid jwt\n");
		exit_status = 1;
		goto finish;
	}

	fprintf(stderr, "jwt decoded successfully!\n");

	/* Validate jwt */
	if (jwt_validate(jwt, jwt_valid) != 0) {
		fprintf(stderr, "jwt failed to validate: %08x\n", jwt_valid_get_status(jwt_valid));
		jwt_dump_fp(jwt, stderr, 1);
		exit_status = 1;
		goto finish;
	}

	fprintf(stderr, "JWT is authentic! sub: %s\n", jwt_get_grant(jwt, "sub"));

	jwt_dump_fp(jwt, stdout, 1);

finish:
	jwt_free(jwt);
finish_valid:
	jwt_valid_free(jwt_valid);

	return exit_status;
}

