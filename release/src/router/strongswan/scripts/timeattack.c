#include <stdio.h>
#include <time.h>

#include <library.h>

typedef bool (*attackfn_t)(void *subj, u_char *data, size_t len);

static void start_timing(struct timespec *start)
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, start);
}

static uint64_t end_timing(struct timespec *start)
{
	struct timespec end;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	return (end.tv_nsec - start->tv_nsec) +
		   (end.tv_sec - start->tv_sec) * 1000000000;
}

static int intcmp(const void *a, const void *b)
{
	return *(uint64_t*)a - *(uint64_t*)b;
}

static uint64_t median(uint64_t *m, int count)
{
	qsort(m, count, sizeof(uint64_t), intcmp);
	return m[count / 2];
}

static bool timeattack(attackfn_t attackfn, void *subj, size_t dlen,
					   u_int iterations, u_int distance)
{
	struct timespec start;
	u_char test[dlen];
	uint64_t mini, maxi, t[256], m[256][10];
	float fastdist = 0, slowdist = 0;
	int i, j, k, l, byte, limit, retry = 0;
	int fastest = 0, slowest = 0;

	memset(test, 0, dlen);

	/* do some iterations to fill caches */
	for (i = 0; i < iterations; i++)
	{
		attackfn(subj, test, dlen);
	}

	for (byte = 0; byte < dlen;)
	{
		memset(t, 0, sizeof(t));
		memset(m, 0, sizeof(m));

		limit = iterations * (retry + 1);

		/* measure timing for all patterns in next byte */
		for (k = 0; k < 10; k++)
		{
			for (j = 0; j < 256; j++)
			{
				for (l = 0; l < 100; l++)
				{
					test[byte] = j;
					start_timing(&start);
					for (i = 0; i < limit; i++)
					{
						attackfn(subj, test, dlen);
					}
					m[j][k] += end_timing(&start);
				}
			}
		}

		for (j = 0; j < 256; j++)
		{
			t[j] = median(m[j], countof(m[j]));
		}

		/* find fastest/slowest runs */
		mini = ~0;
		maxi = 0;
		for (j = 0; j < 256; j++)
		{
			if (t[j] < mini)
			{
				mini = min(t[j], mini);
				fastest = j;
			}
			if (t[j] > maxi)
			{
				maxi = max(t[j], maxi);
				slowest = j;
			}
		}
		/* calculate distance to next result */
		mini = ~0;
		maxi = 0;
		for (j = 0; j < 256; j++)
		{
			if (fastest != j && t[j] < mini)
			{
				mini = min(t[j], mini);
				fastdist = (float)(t[j] - t[fastest]) / distance;
			}
			if (slowest != j && t[j] > maxi)
			{
				maxi = max(t[j], maxi);
				slowdist = (float)(t[slowest] - t[j]) / distance;
			}
		}
		if (fastdist > 1.0f)
		{
			fprintf(stderr, "byte %02d: %02x (fastest, dist %02.2f)\n",
					byte, fastest, fastdist);
			test[byte] = fastest;
			retry = 0;
			byte++;
		}
		else if (slowdist > 1.0f)
		{
			fprintf(stderr, "byte %02d: %02x (slowest, dist %02.2f)\n",
					byte, slowest, slowdist);
			test[byte] = slowest;
			retry = 0;
			byte++;
		}
		else
		{
			if (retry++ > 5 && byte > 0)
			{
				fprintf(stderr, "distance fastest %02.2f (%02x), "
						"slowest %02.2f (%02x), stepping back\n",
						fastdist, fastest, slowdist, slowest);
				test[byte--] = 0;
			}
			else if (retry < 10)
			{
				fprintf(stderr, "distance fastest %02.2f (%02x), "
						"slowest %02.2f (%02x), retrying (%d)\n",
						fastdist, fastest, slowdist, slowest, retry);
			}
			else
			{
				printf("attack failed, giving up\n");
				return FALSE;
			}
		}
	}
	if (attackfn(subj, test, dlen))
	{
		printf("attack successful with %b\n", test, dlen);
		return TRUE;
	}
	printf("attack failed with %b\n", test, dlen);
	return FALSE;
}

CALLBACK(attack_memeq1, bool,
	u_char *subj, u_char *data, size_t len)
{
	return memeq(data, subj, len);
}

CALLBACK(attack_memeq2, bool,
	u_char *subj, u_char *data, size_t len)
{
	return memeq(subj, data, len);
}

CALLBACK(attack_memeq3, bool,
	u_char *subj, u_char *data, size_t len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		if (subj[i] != data[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

CALLBACK(attack_memeq4, bool,
	u_char *subj, u_char *data, size_t len)
{
	int i, m = 0;

	for (i = 0; i < len; i++)
	{
		m |= subj[i] != data[i];
	}
	return !m;
}

CALLBACK(attack_memeq5, bool,
	u_char *subj, u_char *data, size_t len)
{
	return memeq_const(subj, data, len);
}

static bool attack_memeq(char *name, u_int iterations, u_int distance)
{
	struct {
		char *name;
		attackfn_t fn;
	} attacks[] = {
		{ "memeq1", attack_memeq1 },
		{ "memeq2", attack_memeq2 },
		{ "memeq3", attack_memeq3 },
		{ "memeq4", attack_memeq4 },
		{ "memeq5", attack_memeq5 },
	};
	u_char exp[16];
	int i;

	srandom(time(NULL));
	for (i = 0; i < sizeof(exp); i++)
	{
		exp[i] = random();
	}
	fprintf(stderr, "attacking %b\n", exp, sizeof(exp));

	for (i = 0; i < countof(attacks); i++)
	{
		if (streq(name, attacks[i].name))
		{
			return timeattack(attacks[i].fn, exp, sizeof(exp),
							  iterations, distance);
		}
	}
	return FALSE;
}

CALLBACK(attack_chunk1, bool,
	u_char *subj, u_char *data, size_t len)
{
	return chunk_equals(chunk_create(subj, len), chunk_create(data, len));
}

CALLBACK(attack_chunk2, bool,
	u_char *subj, u_char *data, size_t len)
{
	return chunk_equals_const(chunk_create(subj, len), chunk_create(data, len));
}

static bool attack_chunk(char *name, u_int iterations, u_int distance)
{
	struct {
		char *name;
		attackfn_t fn;
	} attacks[] = {
		{ "chunk1", attack_chunk1 },
		{ "chunk2", attack_chunk2 },
	};
	u_char exp[16];
	int i;

	srandom(time(NULL));
	for (i = 0; i < sizeof(exp); i++)
	{
		exp[i] = random();
	}
	fprintf(stderr, "attacking %b\n", exp, sizeof(exp));

	for (i = 0; i < countof(attacks); i++)
	{
		if (streq(name, attacks[i].name))
		{
			return timeattack(attacks[i].fn, exp, sizeof(exp),
							  iterations, distance);
		}
	}
	return FALSE;
}

CALLBACK(attack_aead, bool,
	aead_t *aead, u_char *data, size_t len)
{
	u_char iv[aead->get_iv_size(aead)];

	memset(iv, 0, sizeof(iv));
	return aead->decrypt(aead, chunk_create(data, len), chunk_empty,
						 chunk_from_thing(iv), NULL);
}

static bool attack_aeads(encryption_algorithm_t alg, size_t key_size,
						 u_int iterations, u_int distance)
{
	u_char buf[64];
	aead_t *aead;
	bool res;

	aead = lib->crypto->create_aead(lib->crypto, alg, key_size, 0);
	if (!aead)
	{
		fprintf(stderr, "creating AEAD %N failed\n",
				encryption_algorithm_names, alg);
		return FALSE;
	}
	memset(buf, 0xe3, sizeof(buf));
	if (!aead->set_key(aead, chunk_create(buf, aead->get_key_size(aead))))
	{
		aead->destroy(aead);
		return FALSE;
	}
	memset(buf, 0, aead->get_iv_size(aead));
	if (!aead->encrypt(aead, chunk_create(buf, 0), chunk_empty,
					   chunk_create(buf, aead->get_iv_size(aead)), NULL))
	{
		aead->destroy(aead);
		return FALSE;
	}
	fprintf(stderr, "attacking %b\n", buf, aead->get_icv_size(aead));

	res = timeattack(attack_aead, aead, aead->get_icv_size(aead),
					 iterations, distance);
	aead->destroy(aead);
	return res;
}

CALLBACK(attack_signer, bool,
	signer_t *signer, u_char *data, size_t len)
{
	return signer->verify_signature(signer, chunk_empty, chunk_create(data, len));
}

static bool attack_signers(integrity_algorithm_t alg,
						   u_int iterations, u_int distance)
{
	u_char buf[64];
	signer_t *signer;
	bool res;

	signer = lib->crypto->create_signer(lib->crypto, alg);
	if (!signer)
	{
		fprintf(stderr, "creating signer %N failed\n",
				integrity_algorithm_names, alg);
		return FALSE;
	}
	memset(buf, 0xe3, sizeof(buf));
	if (!signer->set_key(signer, chunk_create(buf, signer->get_key_size(signer))))
	{
		signer->destroy(signer);
		return FALSE;
	}
	if (!signer->get_signature(signer, chunk_empty, buf))
	{
		signer->destroy(signer);
		return FALSE;
	}
	fprintf(stderr, "attacking %b\n", buf, signer->get_block_size(signer));

	res = timeattack(attack_signer, signer, signer->get_block_size(signer),
					 iterations, distance);
	signer->destroy(signer);
	return res;
}

static bool attack_transform(char *name, u_int iterations, u_int distance)
{
	const proposal_token_t *token;

	token = lib->proposal->get_token(lib->proposal, name);
	if (!token)
	{
		fprintf(stderr, "algorithm '%s' unknown\n", name);
		return FALSE;
	}

	switch (token->type)
	{
		case ENCRYPTION_ALGORITHM:
			if (encryption_algorithm_is_aead(token->algorithm))
			{
				return attack_aeads(token->algorithm, token->keysize / 8,
									iterations, distance);
			}
			fprintf(stderr, "can't attack a crypter\n");
			return FALSE;
		case INTEGRITY_ALGORITHM:
			return attack_signers(token->algorithm, iterations, distance);
		default:
			fprintf(stderr, "can't attack a %N\n", transform_type_names, token->type);
			return FALSE;
	}
}

int main(int argc, char *argv[])
{
	library_init(NULL, "timeattack");
	atexit(library_deinit);
	lib->plugins->load(lib->plugins, getenv("PLUGINS") ?: PLUGINS);

	if (argc < 3)
	{
		fprintf(stderr, "usage: %s <attack> <iterations> <distance>\n", argv[0]);
		fprintf(stderr, "  <attack>: memeq[1-5] / chunk[1-2] / aead / signer\n");
		fprintf(stderr, "  <iterations>: number of invocations * 1000\n");
		fprintf(stderr, "  <distance>: time difference in ns for a hit\n");
		fprintf(stderr, "  example: %s memeq1 100 500\n", argv[0]);
		fprintf(stderr, "  example: %s aes128gcm16 100 4000\n", argv[0]);
		return 1;
	}
	if (strpfx(argv[1], "memeq"))
	{
		return !attack_memeq(argv[1], atoi(argv[2]), atoi(argv[3]));
	}
	if (strpfx(argv[1], "chunk"))
	{
		return !attack_chunk(argv[1], atoi(argv[2]), atoi(argv[3]));
	}
	return !attack_transform(argv[1], atoi(argv[2]), atoi(argv[3]));
}
