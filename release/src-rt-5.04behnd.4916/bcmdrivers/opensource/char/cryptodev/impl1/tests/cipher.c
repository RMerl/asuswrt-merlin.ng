/*
 * Demo on how to use /dev/crypto device for ciphering.
 *
 * Placed under public domain.
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <crypto/cryptodev.h>

static int debug = 0;

#define	DATA_SIZE	8*1024
#define	BLOCK_SIZE	16
#define	KEY_SIZE	16

static int
test_crypto(int cfd)
{
	char plaintext_raw[DATA_SIZE + 63], *plaintext;
	char ciphertext_raw[DATA_SIZE + 63], *ciphertext;
	char iv[BLOCK_SIZE];
	char key[KEY_SIZE];

	struct session_op sess;
#ifdef CIOCGSESSINFO
	struct session_info_op siop;
#endif
	struct crypt_op cryp;

	memset(&sess, 0, sizeof(sess));
	memset(&cryp, 0, sizeof(cryp));

	memset(key, 0x33,  sizeof(key));
	memset(iv, 0x03,  sizeof(iv));

	/* Get crypto session for AES128 */
	sess.cipher = CRYPTO_AES_CBC;
	sess.keylen = KEY_SIZE;
	sess.key = key;
	if (ioctl(cfd, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		return 1;
	}

#ifdef CIOCGSESSINFO
	siop.ses = sess.ses;
	if (ioctl(cfd, CIOCGSESSINFO, &siop)) {
		perror("ioctl(CIOCGSESSINFO)");
		return 1;
	}
	if (debug)
		printf("requested cipher CRYPTO_AES_CBC, got %s with driver %s\n",
			siop.cipher_info.cra_name, siop.cipher_info.cra_driver_name);

	plaintext = (char *)(((unsigned long)plaintext_raw + siop.alignmask) & ~siop.alignmask);
	ciphertext = (char *)(((unsigned long)ciphertext_raw + siop.alignmask) & ~siop.alignmask);
#else
	plaintext = plaintext_raw;
	ciphertext = ciphertext_raw;
#endif
	memset(plaintext, 0x15, DATA_SIZE);

	/* Encrypt data.in to data.encrypted */
	cryp.ses = sess.ses;
	cryp.len = DATA_SIZE;
	cryp.src = plaintext;
	cryp.dst = ciphertext;
	cryp.iv = iv;
	cryp.op = COP_ENCRYPT;
	if (ioctl(cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return 1;
	}

	if (ioctl(cfd, CIOCFSESSION, &sess.ses)) {
		perror("ioctl(CIOCFSESSION)");
		return 1;
	}

	if (ioctl(cfd, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		return 1;
	}

#ifdef CIOCGSESSINFO
	siop.ses = sess.ses;
	if (ioctl(cfd, CIOCGSESSINFO, &siop)) {
		perror("ioctl(CIOCGSESSINFO)");
		return 1;
	}
	if (debug)
		printf("requested cipher CRYPTO_AES_CBC, got %s with driver %s\n",
			siop.cipher_info.cra_name, siop.cipher_info.cra_driver_name);
#endif

	/* Decrypt data.encrypted to data.decrypted */
	cryp.ses = sess.ses;
	cryp.len = DATA_SIZE;
	cryp.src = ciphertext;
	cryp.dst = ciphertext;
	cryp.iv = iv;
	cryp.op = COP_DECRYPT;
	if (ioctl(cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return 1;
	}

	/* Verify the result */
	if (memcmp(plaintext, ciphertext, DATA_SIZE) != 0) {
		int i;
		fprintf(stderr,
			"FAIL: Decrypted data are different from the input data.\n");
		printf("plaintext:");
		for (i = 0; i < DATA_SIZE; i++) {
			if ((i % 30) == 0)
				printf("\n");
			printf("%02x ", plaintext[i]);
		}
		printf("ciphertext:");
		for (i = 0; i < DATA_SIZE; i++) {
			if ((i % 30) == 0)
				printf("\n");
			printf("%02x ", ciphertext[i]);
		}
		printf("\n");
		return 1;
	} else if (debug)
		printf("Test passed\n");

	/* Finish crypto session */
	if (ioctl(cfd, CIOCFSESSION, &sess.ses)) {
		perror("ioctl(CIOCFSESSION)");
		return 1;
	}

	return 0;
}

static int test_aes(int cfd)
{
	char plaintext1_raw[BLOCK_SIZE + 63], *plaintext1;
	char ciphertext1[BLOCK_SIZE] = { 0xdf, 0x55, 0x6a, 0x33, 0x43, 0x8d, 0xb8, 0x7b, 0xc4, 0x1b, 0x17, 0x52, 0xc5, 0x5e, 0x5e, 0x49 };
	char iv1[BLOCK_SIZE];
	char key1[KEY_SIZE] = { 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	char plaintext2_data[BLOCK_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00 };
	char plaintext2_raw[BLOCK_SIZE + 63], *plaintext2;
	char ciphertext2[BLOCK_SIZE] = { 0xb7, 0x97, 0x2b, 0x39, 0x41, 0xc4, 0x4b, 0x90, 0xaf, 0xa7, 0xb2, 0x64, 0xbf, 0xba, 0x73, 0x87 };
	char iv2[BLOCK_SIZE];
	char key2[KEY_SIZE];

	struct session_op sess;
#ifdef CIOCGSESSINFO
	struct session_info_op siop;
#endif
	struct crypt_op cryp;

	memset(&sess, 0, sizeof(sess));
	memset(&cryp, 0, sizeof(cryp));

	/* Get crypto session for AES128 */
	sess.cipher = CRYPTO_AES_CBC;
	sess.keylen = KEY_SIZE;
	sess.key = key1;
	if (ioctl(cfd, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		return 1;
	}
#ifdef CIOCGSESSINFO
	siop.ses = sess.ses;
	if (ioctl(cfd, CIOCGSESSINFO, &siop)) {
		perror("ioctl(CIOCGSESSINFO)");
		return 1;
	}
	plaintext1 = (char *)(((unsigned long)plaintext1_raw + siop.alignmask) & ~siop.alignmask);
#else
	plaintext1 = plaintext1_raw;
#endif
	memset(plaintext1, 0x0, BLOCK_SIZE);
	memset(iv1, 0x0, sizeof(iv1));

	/* Encrypt data.in to data.encrypted */
	cryp.ses = sess.ses;
	cryp.len = BLOCK_SIZE;
	cryp.src = plaintext1;
	cryp.dst = plaintext1;
	cryp.iv = iv1;
	cryp.op = COP_ENCRYPT;
	if (ioctl(cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return 1;
	}

	/* Verify the result */
	if (memcmp(plaintext1, ciphertext1, BLOCK_SIZE) != 0) {
		fprintf(stderr,
			"FAIL: Decrypted data are different from the input data.\n");
		return 1;
	}

	/* Test 2 */

	memset(key2, 0x0, sizeof(key2));
	memset(iv2, 0x0, sizeof(iv2));

	/* Get crypto session for AES128 */
	sess.cipher = CRYPTO_AES_CBC;
	sess.keylen = KEY_SIZE;
	sess.key = key2;
	if (ioctl(cfd, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		return 1;
	}

#ifdef CIOCGSESSINFO
	siop.ses = sess.ses;
	if (ioctl(cfd, CIOCGSESSINFO, &siop)) {
		perror("ioctl(CIOCGSESSINFO)");
		return 1;
	}
	if (debug)
		printf("requested cipher CRYPTO_AES_CBC, got %s with driver %s\n",
			siop.cipher_info.cra_name, siop.cipher_info.cra_driver_name);

	plaintext2 = (char *)(((unsigned long)plaintext2_raw + siop.alignmask) & ~siop.alignmask);
#else
	plaintext2 = plaintext2_raw;
#endif
	memcpy(plaintext2, plaintext2_data, BLOCK_SIZE);

	/* Encrypt data.in to data.encrypted */
	cryp.ses = sess.ses;
	cryp.len = BLOCK_SIZE;
	cryp.src = plaintext2;
	cryp.dst = plaintext2;
	cryp.iv = iv2;
	cryp.op = COP_ENCRYPT;
	if (ioctl(cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return 1;
	}

	/* Verify the result */
	if (memcmp(plaintext2, ciphertext2, BLOCK_SIZE) != 0) {
		int i;
		fprintf(stderr,
			"FAIL: Decrypted data are different from the input data.\n");
		printf("plaintext:");
		for (i = 0; i < BLOCK_SIZE; i++) {
			if ((i % 30) == 0)
				printf("\n");
			printf("%02x ", plaintext2[i]);
		}
		printf("ciphertext:");
		for (i = 0; i < BLOCK_SIZE; i++) {
			if ((i % 30) == 0)
				printf("\n");
			printf("%02x ", ciphertext2[i]);
		}
		printf("\n");
		return 1;
	}

	if (debug) printf("AES Test passed\n");

	/* Finish crypto session */
	if (ioctl(cfd, CIOCFSESSION, &sess.ses)) {
		perror("ioctl(CIOCFSESSION)");
		return 1;
	}

	return 0;
}

int
main(int argc, char** argv)
{
	int fd = -1, cfd = -1;

	if (argc > 1) debug = 1;

	/* Open the crypto device */
	fd = open("/dev/crypto", O_RDWR, 0);
	if (fd < 0) {
		perror("open(/dev/crypto)");
		return 1;
	}

	/* Clone file descriptor */
	if (ioctl(fd, CRIOGET, &cfd)) {
		perror("ioctl(CRIOGET)");
		return 1;
	}

	/* Set close-on-exec (not really neede here) */
	if (fcntl(cfd, F_SETFD, 1) == -1) {
		perror("fcntl(F_SETFD)");
		return 1;
	}

	/* Run the test itself */
	if (test_aes(cfd))
		return 1;

	if (test_crypto(cfd))
		return 1;

	/* Close cloned descriptor */
	if (close(cfd)) {
		perror("close(cfd)");
		return 1;
	}

	/* Close the original descriptor */
	if (close(fd)) {
		perror("close(fd)");
		return 1;
	}

	return 0;
}

