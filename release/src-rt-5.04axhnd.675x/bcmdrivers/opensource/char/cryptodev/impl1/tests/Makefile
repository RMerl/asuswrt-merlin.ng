KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
KBUILD_CFLAGS += -I.. $(CRYPTODEV_CFLAGS)
CFLAGS += -I.. $(CRYPTODEV_CFLAGS)

comp_progs := cipher_comp hash_comp hmac_comp

hostprogs := cipher cipher-aead hmac speed async_cipher async_hmac \
	async_speed sha_speed hashcrypt_speed fullspeed cipher-gcm \
	cipher-aead-srtp ${comp_progs}

example-cipher-objs := cipher.o
example-cipher-aead-objs := cipher-aead.o
example-hmac-objs := hmac.o
example-speed-objs := speed.c
example-fullspeed-objs := fullspeed.c
example-sha-speed-objs := sha_speed.c
example-async-cipher-objs := async_cipher.o
example-async-hmac-objs := async_hmac.o
example-async-speed-objs := async_speed.o
example-hashcrypt-speed-objs := hashcrypt_speed.c

check: $(hostprogs)
	./cipher
	./hmac
	./async_cipher
	./async_hmac
	./cipher-aead-srtp
	./cipher-gcm
	./cipher-aead

clean:
	rm -f *.o *~ $(hostprogs)

${comp_progs}: LDLIBS += -lssl -lcrypto
${comp_progs}: %: %.o openssl_wrapper.o
