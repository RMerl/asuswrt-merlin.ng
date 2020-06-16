#
# Include makefile used by makefile + makefile.shared
#  (GNU make only)

# The version - BEWARE: VERSION, VERSION_PC and VERSION_LT are updated via ./updatemakes.sh
VERSION=1.18.2
VERSION_PC=1.18.2
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
VERSION_LT=1:1

# Compiler and Linker Names
ifndef CROSS_COMPILE
  CROSS_COMPILE:=
endif

# We only need to go through this dance of determining the right compiler if we're using
# cross compilation, otherwise $(CC) is fine as-is.
ifneq (,$(CROSS_COMPILE))
ifeq ($(origin CC),default)
CSTR := "\#ifdef __clang__\nCLANG\n\#endif\n"
ifeq ($(PLATFORM),FreeBSD)
  # XXX: FreeBSD needs extra escaping for some reason
  CSTR := $$$(CSTR)
endif
ifneq (,$(shell echo $(CSTR) | $(CC) -E - | grep CLANG))
  CC := $(CROSS_COMPILE)clang
else
  CC := $(CROSS_COMPILE)gcc
endif # Clang
endif # cc is Make's default
endif # CROSS_COMPILE non-empty

LD:=$(CROSS_COMPILE)ld
AR:=$(CROSS_COMPILE)ar

# Archiver [makes .a files]
#AR=ar
ARFLAGS:=r

ifndef MAKE
# BSDs refer to GNU Make as gmake
ifneq (,$(findstring $(PLATFORM),FreeBSD OpenBSD DragonFly NetBSD))
  MAKE=gmake
else
  MAKE=make
endif
endif

ifndef INSTALL_CMD
$(error your makefile must define INSTALL_CMD)
endif
ifndef UNINSTALL_CMD
$(error your makefile must define UNINSTALL_CMD)
endif

ifndef EXTRALIBS
ifneq ($(shell echo $(CFLAGS) | grep USE_LTM),)
EXTRALIBS=$(shell PKG_CONFIG_PATH=$(LIBPATH)/pkgconfig pkg-config libtommath --libs)
else
ifneq ($(shell echo $(CFLAGS) | grep USE_TFM),)
EXTRALIBS=$(shell PKG_CONFIG_PATH=$(LIBPATH)/pkgconfig pkg-config tomsfastmath --libs)
endif
endif
endif

need-help := $(filter help,$(MAKECMDGOALS))
define print-help
$(if $(need-help),$(info $1 -- $2))
endef

#
# Compilation flags. Note the += does not write over the user's CFLAGS!
#
# Also note that we're extending the environments' CFLAGS.
# If you think that our CFLAGS are not nice you can easily override them
# by giving them as a parameter to make:
#  make CFLAGS="-I./src/headers/ -DLTC_SOURCE ..." ...
#
LTC_CFLAGS += -I./src/headers/ -Wall -Wsign-compare -Wshadow -DLTC_SOURCE

ifdef OLD_GCC
LTC_CFLAGS += -W
# older GCCs can't handle the "rotate with immediate" ROLc/RORc/etc macros
# define this to help
LTC_CFLAGS += -DLTC_NO_ROLC
else
LTC_CFLAGS += -Wextra
# additional warnings
LTC_CFLAGS += -Wsystem-headers -Wbad-function-cast -Wcast-align
LTC_CFLAGS += -Wstrict-prototypes -Wpointer-arith
LTC_CFLAGS += -Wdeclaration-after-statement
LTC_CFLAGS += -Wwrite-strings
endif

LTC_CFLAGS += -Wno-type-limits

ifdef LTC_DEBUG
$(info Debug build)
# compile for DEBUGGING (required for ccmalloc checking!!!)
LTC_CFLAGS += -g3 -DLTC_NO_ASM
ifneq (,$(strip $(LTC_DEBUG)))
LTC_CFLAGS += -DLTC_TEST_DBG=$(LTC_DEBUG)
else
LTC_CFLAGS += -DLTC_TEST_DBG
endif
else

ifdef LTC_SMALL
# optimize for SIZE
LTC_CFLAGS += -Os -DLTC_SMALL_CODE
else

ifndef IGNORE_SPEED
# optimize for SPEED
LTC_CFLAGS += -O3 -funroll-loops

# add -fomit-frame-pointer.  hinders debugging!
LTC_CFLAGS += -fomit-frame-pointer
endif

endif # COMPILE_SMALL
endif # COMPILE_DEBUG


ifneq ($(findstring clang,$(CC)),)
LTC_CFLAGS += -Wno-typedef-redefinition -Wno-tautological-compare -Wno-builtin-requires-header -Wno-missing-field-initializers
endif
ifneq ($(findstring mingw,$(CC)),)
LTC_CFLAGS += -Wno-shadow -Wno-attributes
endif
ifeq ($(PLATFORM), Darwin)
LTC_CFLAGS += -Wno-nullability-completeness
endif


GIT_VERSION := $(shell { [ -e .git ] && which git 2>/dev/null 1>&2 ; } && { printf git- ; git describe --tags --always --dirty ; } || echo $(VERSION))
ifneq ($(GIT_VERSION),)
LTC_CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

LTC_CFLAGS := $(LTC_CFLAGS) $(CFLAGS)

ifneq ($(findstring -DLTC_PTHREAD,$(LTC_CFLAGS)),)
LTC_LDFLAGS += -pthread
endif

LTC_LDFLAGS := $(LTC_LDFLAGS) $(LDFLAGS)

#List of demo objects
DSOURCES = $(wildcard demos/*.c)
DOBJECTS = $(DSOURCES:.c=.o)

#List of tests headers
THEADERS = $(wildcard tests/*.h)

TEST=test

# Demos that are even somehow useful and could be installed as a system-tool
USEFUL_DEMOS   = hashsum

# Demos that are usable but only rarely make sense to be installed
USEABLE_DEMOS  = ltcrypt sizes constants

# Demos that are used for testing or measuring
TEST_DEMOS     = small tv_gen

# Demos that are in one config broken
#  openssl-enc - can't be build with LTC_EASY
#  timing      - not really broken, but older gcc builds spit warnings
BROKEN_DEMOS   = openssl-enc timing

# Combine demos in groups
UNBROKEN_DEMOS = $(TEST_DEMOS) $(USEABLE_DEMOS) $(USEFUL_DEMOS)
DEMOS          = $(UNBROKEN_DEMOS) $(BROKEN_DEMOS)

#LIBPATH  The directory for libtomcrypt to be installed to.
#INCPATH  The directory to install the header files for libtomcrypt.
#DATAPATH The directory to install the pdf docs.
#BINPATH  The directory to install the binaries provided.
DESTDIR  ?=
PREFIX   ?= /usr/local
LIBPATH  ?= $(PREFIX)/lib
INCPATH  ?= $(PREFIX)/include
DATAPATH ?= $(PREFIX)/share/doc/libtomcrypt/pdf
BINPATH  ?= $(PREFIX)/bin

#Who do we install as?
ifdef INSTALL_USER
USER=$(INSTALL_USER)
else
USER=root
endif

ifdef INSTALL_GROUP
GROUP=$(INSTALL_GROUP)
else
GROUP=wheel
endif


#The first rule is also the default rule and builds the libtomcrypt library.
library: $(call print-help,library,Builds the library) $(LIBNAME)


# List of objects to compile (all goes to libtomcrypt.a)
OBJECTS=src/ciphers/aes/aes.o src/ciphers/aes/aes_enc.o src/ciphers/anubis.o src/ciphers/blowfish.o \
src/ciphers/camellia.o src/ciphers/cast5.o src/ciphers/des.o src/ciphers/kasumi.o src/ciphers/khazad.o \
src/ciphers/kseed.o src/ciphers/multi2.o src/ciphers/noekeon.o src/ciphers/rc2.o src/ciphers/rc5.o \
src/ciphers/rc6.o src/ciphers/safer/safer.o src/ciphers/safer/saferp.o src/ciphers/skipjack.o \
src/ciphers/twofish/twofish.o src/ciphers/xtea.o src/encauth/ccm/ccm_add_aad.o \
src/encauth/ccm/ccm_add_nonce.o src/encauth/ccm/ccm_done.o src/encauth/ccm/ccm_init.o \
src/encauth/ccm/ccm_memory.o src/encauth/ccm/ccm_process.o src/encauth/ccm/ccm_reset.o \
src/encauth/ccm/ccm_test.o src/encauth/chachapoly/chacha20poly1305_add_aad.o \
src/encauth/chachapoly/chacha20poly1305_decrypt.o src/encauth/chachapoly/chacha20poly1305_done.o \
src/encauth/chachapoly/chacha20poly1305_encrypt.o src/encauth/chachapoly/chacha20poly1305_init.o \
src/encauth/chachapoly/chacha20poly1305_memory.o src/encauth/chachapoly/chacha20poly1305_setiv.o \
src/encauth/chachapoly/chacha20poly1305_setiv_rfc7905.o \
src/encauth/chachapoly/chacha20poly1305_test.o src/encauth/eax/eax_addheader.o \
src/encauth/eax/eax_decrypt.o src/encauth/eax/eax_decrypt_verify_memory.o src/encauth/eax/eax_done.o \
src/encauth/eax/eax_encrypt.o src/encauth/eax/eax_encrypt_authenticate_memory.o \
src/encauth/eax/eax_init.o src/encauth/eax/eax_test.o src/encauth/gcm/gcm_add_aad.o \
src/encauth/gcm/gcm_add_iv.o src/encauth/gcm/gcm_done.o src/encauth/gcm/gcm_gf_mult.o \
src/encauth/gcm/gcm_init.o src/encauth/gcm/gcm_memory.o src/encauth/gcm/gcm_mult_h.o \
src/encauth/gcm/gcm_process.o src/encauth/gcm/gcm_reset.o src/encauth/gcm/gcm_test.o \
src/encauth/ocb/ocb_decrypt.o src/encauth/ocb/ocb_decrypt_verify_memory.o \
src/encauth/ocb/ocb_done_decrypt.o src/encauth/ocb/ocb_done_encrypt.o src/encauth/ocb/ocb_encrypt.o \
src/encauth/ocb/ocb_encrypt_authenticate_memory.o src/encauth/ocb/ocb_init.o src/encauth/ocb/ocb_ntz.o \
src/encauth/ocb/ocb_shift_xor.o src/encauth/ocb/ocb_test.o src/encauth/ocb/s_ocb_done.o \
src/encauth/ocb3/ocb3_add_aad.o src/encauth/ocb3/ocb3_decrypt.o src/encauth/ocb3/ocb3_decrypt_last.o \
src/encauth/ocb3/ocb3_decrypt_verify_memory.o src/encauth/ocb3/ocb3_done.o \
src/encauth/ocb3/ocb3_encrypt.o src/encauth/ocb3/ocb3_encrypt_authenticate_memory.o \
src/encauth/ocb3/ocb3_encrypt_last.o src/encauth/ocb3/ocb3_init.o src/encauth/ocb3/ocb3_int_ntz.o \
src/encauth/ocb3/ocb3_int_xor_blocks.o src/encauth/ocb3/ocb3_test.o src/hashes/blake2b.o \
src/hashes/blake2s.o src/hashes/chc/chc.o src/hashes/helper/hash_file.o \
src/hashes/helper/hash_filehandle.o src/hashes/helper/hash_memory.o \
src/hashes/helper/hash_memory_multi.o src/hashes/md2.o src/hashes/md4.o src/hashes/md5.o \
src/hashes/rmd128.o src/hashes/rmd160.o src/hashes/rmd256.o src/hashes/rmd320.o src/hashes/sha1.o \
src/hashes/sha2/sha224.o src/hashes/sha2/sha256.o src/hashes/sha2/sha384.o src/hashes/sha2/sha512.o \
src/hashes/sha2/sha512_224.o src/hashes/sha2/sha512_256.o src/hashes/sha3.o src/hashes/sha3_test.o \
src/hashes/tiger.o src/hashes/whirl/whirl.o src/mac/blake2/blake2bmac.o \
src/mac/blake2/blake2bmac_file.o src/mac/blake2/blake2bmac_memory.o \
src/mac/blake2/blake2bmac_memory_multi.o src/mac/blake2/blake2bmac_test.o src/mac/blake2/blake2smac.o \
src/mac/blake2/blake2smac_file.o src/mac/blake2/blake2smac_memory.o \
src/mac/blake2/blake2smac_memory_multi.o src/mac/blake2/blake2smac_test.o src/mac/f9/f9_done.o \
src/mac/f9/f9_file.o src/mac/f9/f9_init.o src/mac/f9/f9_memory.o src/mac/f9/f9_memory_multi.o \
src/mac/f9/f9_process.o src/mac/f9/f9_test.o src/mac/hmac/hmac_done.o src/mac/hmac/hmac_file.o \
src/mac/hmac/hmac_init.o src/mac/hmac/hmac_memory.o src/mac/hmac/hmac_memory_multi.o \
src/mac/hmac/hmac_process.o src/mac/hmac/hmac_test.o src/mac/omac/omac_done.o src/mac/omac/omac_file.o \
src/mac/omac/omac_init.o src/mac/omac/omac_memory.o src/mac/omac/omac_memory_multi.o \
src/mac/omac/omac_process.o src/mac/omac/omac_test.o src/mac/pelican/pelican.o \
src/mac/pelican/pelican_memory.o src/mac/pelican/pelican_test.o src/mac/pmac/pmac_done.o \
src/mac/pmac/pmac_file.o src/mac/pmac/pmac_init.o src/mac/pmac/pmac_memory.o \
src/mac/pmac/pmac_memory_multi.o src/mac/pmac/pmac_ntz.o src/mac/pmac/pmac_process.o \
src/mac/pmac/pmac_shift_xor.o src/mac/pmac/pmac_test.o src/mac/poly1305/poly1305.o \
src/mac/poly1305/poly1305_file.o src/mac/poly1305/poly1305_memory.o \
src/mac/poly1305/poly1305_memory_multi.o src/mac/poly1305/poly1305_test.o src/mac/xcbc/xcbc_done.o \
src/mac/xcbc/xcbc_file.o src/mac/xcbc/xcbc_init.o src/mac/xcbc/xcbc_memory.o \
src/mac/xcbc/xcbc_memory_multi.o src/mac/xcbc/xcbc_process.o src/mac/xcbc/xcbc_test.o \
src/math/fp/ltc_ecc_fp_mulmod.o src/math/gmp_desc.o src/math/ltm_desc.o src/math/multi.o \
src/math/radix_to_bin.o src/math/rand_bn.o src/math/rand_prime.o src/math/tfm_desc.o src/misc/adler32.o \
src/misc/base64/base64_decode.o src/misc/base64/base64_encode.o src/misc/burn_stack.o \
src/misc/compare_testvector.o src/misc/crc32.o src/misc/crypt/crypt.o src/misc/crypt/crypt_argchk.o \
src/misc/crypt/crypt_cipher_descriptor.o src/misc/crypt/crypt_cipher_is_valid.o \
src/misc/crypt/crypt_constants.o src/misc/crypt/crypt_find_cipher.o \
src/misc/crypt/crypt_find_cipher_any.o src/misc/crypt/crypt_find_cipher_id.o \
src/misc/crypt/crypt_find_hash.o src/misc/crypt/crypt_find_hash_any.o \
src/misc/crypt/crypt_find_hash_id.o src/misc/crypt/crypt_find_hash_oid.o \
src/misc/crypt/crypt_find_prng.o src/misc/crypt/crypt_fsa.o src/misc/crypt/crypt_hash_descriptor.o \
src/misc/crypt/crypt_hash_is_valid.o src/misc/crypt/crypt_inits.o \
src/misc/crypt/crypt_ltc_mp_descriptor.o src/misc/crypt/crypt_prng_descriptor.o \
src/misc/crypt/crypt_prng_is_valid.o src/misc/crypt/crypt_prng_rng_descriptor.o \
src/misc/crypt/crypt_register_all_ciphers.o src/misc/crypt/crypt_register_all_hashes.o \
src/misc/crypt/crypt_register_all_prngs.o src/misc/crypt/crypt_register_cipher.o \
src/misc/crypt/crypt_register_hash.o src/misc/crypt/crypt_register_prng.o src/misc/crypt/crypt_sizes.o \
src/misc/crypt/crypt_unregister_cipher.o src/misc/crypt/crypt_unregister_hash.o \
src/misc/crypt/crypt_unregister_prng.o src/misc/error_to_string.o src/misc/hkdf/hkdf.o \
src/misc/hkdf/hkdf_test.o src/misc/mem_neq.o src/misc/pk_get_oid.o src/misc/pkcs5/pkcs_5_1.o \
src/misc/pkcs5/pkcs_5_2.o src/misc/pkcs5/pkcs_5_test.o src/misc/zeromem.o src/modes/cbc/cbc_decrypt.o \
src/modes/cbc/cbc_done.o src/modes/cbc/cbc_encrypt.o src/modes/cbc/cbc_getiv.o \
src/modes/cbc/cbc_setiv.o src/modes/cbc/cbc_start.o src/modes/cfb/cfb_decrypt.o \
src/modes/cfb/cfb_done.o src/modes/cfb/cfb_encrypt.o src/modes/cfb/cfb_getiv.o \
src/modes/cfb/cfb_setiv.o src/modes/cfb/cfb_start.o src/modes/ctr/ctr_decrypt.o \
src/modes/ctr/ctr_done.o src/modes/ctr/ctr_encrypt.o src/modes/ctr/ctr_getiv.o \
src/modes/ctr/ctr_setiv.o src/modes/ctr/ctr_start.o src/modes/ctr/ctr_test.o \
src/modes/ecb/ecb_decrypt.o src/modes/ecb/ecb_done.o src/modes/ecb/ecb_encrypt.o \
src/modes/ecb/ecb_start.o src/modes/f8/f8_decrypt.o src/modes/f8/f8_done.o src/modes/f8/f8_encrypt.o \
src/modes/f8/f8_getiv.o src/modes/f8/f8_setiv.o src/modes/f8/f8_start.o src/modes/f8/f8_test_mode.o \
src/modes/lrw/lrw_decrypt.o src/modes/lrw/lrw_done.o src/modes/lrw/lrw_encrypt.o \
src/modes/lrw/lrw_getiv.o src/modes/lrw/lrw_process.o src/modes/lrw/lrw_setiv.o \
src/modes/lrw/lrw_start.o src/modes/lrw/lrw_test.o src/modes/ofb/ofb_decrypt.o src/modes/ofb/ofb_done.o \
src/modes/ofb/ofb_encrypt.o src/modes/ofb/ofb_getiv.o src/modes/ofb/ofb_setiv.o \
src/modes/ofb/ofb_start.o src/modes/xts/xts_decrypt.o src/modes/xts/xts_done.o \
src/modes/xts/xts_encrypt.o src/modes/xts/xts_init.o src/modes/xts/xts_mult_x.o \
src/modes/xts/xts_test.o src/pk/asn1/der/bit/der_decode_bit_string.o \
src/pk/asn1/der/bit/der_decode_raw_bit_string.o src/pk/asn1/der/bit/der_encode_bit_string.o \
src/pk/asn1/der/bit/der_encode_raw_bit_string.o src/pk/asn1/der/bit/der_length_bit_string.o \
src/pk/asn1/der/boolean/der_decode_boolean.o src/pk/asn1/der/boolean/der_encode_boolean.o \
src/pk/asn1/der/boolean/der_length_boolean.o src/pk/asn1/der/choice/der_decode_choice.o \
src/pk/asn1/der/generalizedtime/der_decode_generalizedtime.o \
src/pk/asn1/der/generalizedtime/der_encode_generalizedtime.o \
src/pk/asn1/der/generalizedtime/der_length_generalizedtime.o \
src/pk/asn1/der/ia5/der_decode_ia5_string.o src/pk/asn1/der/ia5/der_encode_ia5_string.o \
src/pk/asn1/der/ia5/der_length_ia5_string.o src/pk/asn1/der/integer/der_decode_integer.o \
src/pk/asn1/der/integer/der_encode_integer.o src/pk/asn1/der/integer/der_length_integer.o \
src/pk/asn1/der/object_identifier/der_decode_object_identifier.o \
src/pk/asn1/der/object_identifier/der_encode_object_identifier.o \
src/pk/asn1/der/object_identifier/der_length_object_identifier.o \
src/pk/asn1/der/octet/der_decode_octet_string.o src/pk/asn1/der/octet/der_encode_octet_string.o \
src/pk/asn1/der/octet/der_length_octet_string.o \
src/pk/asn1/der/printable_string/der_decode_printable_string.o \
src/pk/asn1/der/printable_string/der_encode_printable_string.o \
src/pk/asn1/der/printable_string/der_length_printable_string.o \
src/pk/asn1/der/sequence/der_decode_sequence_ex.o \
src/pk/asn1/der/sequence/der_decode_sequence_flexi.o \
src/pk/asn1/der/sequence/der_decode_sequence_multi.o \
src/pk/asn1/der/sequence/der_decode_subject_public_key_info.o \
src/pk/asn1/der/sequence/der_encode_sequence_ex.o \
src/pk/asn1/der/sequence/der_encode_sequence_multi.o \
src/pk/asn1/der/sequence/der_encode_subject_public_key_info.o \
src/pk/asn1/der/sequence/der_length_sequence.o src/pk/asn1/der/sequence/der_sequence_free.o \
src/pk/asn1/der/sequence/der_sequence_shrink.o src/pk/asn1/der/set/der_encode_set.o \
src/pk/asn1/der/set/der_encode_setof.o src/pk/asn1/der/short_integer/der_decode_short_integer.o \
src/pk/asn1/der/short_integer/der_encode_short_integer.o \
src/pk/asn1/der/short_integer/der_length_short_integer.o \
src/pk/asn1/der/teletex_string/der_decode_teletex_string.o \
src/pk/asn1/der/teletex_string/der_length_teletex_string.o \
src/pk/asn1/der/utctime/der_decode_utctime.o src/pk/asn1/der/utctime/der_encode_utctime.o \
src/pk/asn1/der/utctime/der_length_utctime.o src/pk/asn1/der/utf8/der_decode_utf8_string.o \
src/pk/asn1/der/utf8/der_encode_utf8_string.o src/pk/asn1/der/utf8/der_length_utf8_string.o \
src/pk/dh/dh.o src/pk/dh/dh_check_pubkey.o src/pk/dh/dh_export.o src/pk/dh/dh_export_key.o \
src/pk/dh/dh_free.o src/pk/dh/dh_generate_key.o src/pk/dh/dh_import.o src/pk/dh/dh_set.o \
src/pk/dh/dh_set_pg_dhparam.o src/pk/dh/dh_shared_secret.o src/pk/dsa/dsa_decrypt_key.o \
src/pk/dsa/dsa_encrypt_key.o src/pk/dsa/dsa_export.o src/pk/dsa/dsa_free.o \
src/pk/dsa/dsa_generate_key.o src/pk/dsa/dsa_generate_pqg.o src/pk/dsa/dsa_import.o \
src/pk/dsa/dsa_make_key.o src/pk/dsa/dsa_set.o src/pk/dsa/dsa_set_pqg_dsaparam.o \
src/pk/dsa/dsa_shared_secret.o src/pk/dsa/dsa_sign_hash.o src/pk/dsa/dsa_verify_hash.o \
src/pk/dsa/dsa_verify_key.o src/pk/ecc/ecc.o src/pk/ecc/ecc_ansi_x963_export.o \
src/pk/ecc/ecc_ansi_x963_import.o src/pk/ecc/ecc_decrypt_key.o src/pk/ecc/ecc_encrypt_key.o \
src/pk/ecc/ecc_export.o src/pk/ecc/ecc_free.o src/pk/ecc/ecc_get_size.o src/pk/ecc/ecc_import.o \
src/pk/ecc/ecc_make_key.o src/pk/ecc/ecc_shared_secret.o src/pk/ecc/ecc_sign_hash.o \
src/pk/ecc/ecc_sizes.o src/pk/ecc/ecc_test.o src/pk/ecc/ecc_verify_hash.o \
src/pk/ecc/ltc_ecc_is_valid_idx.o src/pk/ecc/ltc_ecc_map.o src/pk/ecc/ltc_ecc_mul2add.o \
src/pk/ecc/ltc_ecc_mulmod.o src/pk/ecc/ltc_ecc_mulmod_timing.o src/pk/ecc/ltc_ecc_points.o \
src/pk/ecc/ltc_ecc_projective_add_point.o src/pk/ecc/ltc_ecc_projective_dbl_point.o \
src/pk/katja/katja_decrypt_key.o src/pk/katja/katja_encrypt_key.o src/pk/katja/katja_export.o \
src/pk/katja/katja_exptmod.o src/pk/katja/katja_free.o src/pk/katja/katja_import.o \
src/pk/katja/katja_make_key.o src/pk/pkcs1/pkcs_1_i2osp.o src/pk/pkcs1/pkcs_1_mgf1.o \
src/pk/pkcs1/pkcs_1_oaep_decode.o src/pk/pkcs1/pkcs_1_oaep_encode.o src/pk/pkcs1/pkcs_1_os2ip.o \
src/pk/pkcs1/pkcs_1_pss_decode.o src/pk/pkcs1/pkcs_1_pss_encode.o src/pk/pkcs1/pkcs_1_v1_5_decode.o \
src/pk/pkcs1/pkcs_1_v1_5_encode.o src/pk/rsa/rsa_decrypt_key.o src/pk/rsa/rsa_encrypt_key.o \
src/pk/rsa/rsa_export.o src/pk/rsa/rsa_exptmod.o src/pk/rsa/rsa_free.o src/pk/rsa/rsa_get_size.o \
src/pk/rsa/rsa_import.o src/pk/rsa/rsa_import_pkcs8.o src/pk/rsa/rsa_import_x509.o \
src/pk/rsa/rsa_make_key.o src/pk/rsa/rsa_set.o src/pk/rsa/rsa_sign_hash.o \
src/pk/rsa/rsa_sign_saltlen_get.o src/pk/rsa/rsa_verify_hash.o src/prngs/chacha20.o src/prngs/fortuna.o \
src/prngs/rc4.o src/prngs/rng_get_bytes.o src/prngs/rng_make_prng.o src/prngs/sober128.o \
src/prngs/sprng.o src/prngs/yarrow.o src/stream/chacha/chacha_crypt.o src/stream/chacha/chacha_done.o \
src/stream/chacha/chacha_ivctr32.o src/stream/chacha/chacha_ivctr64.o \
src/stream/chacha/chacha_keystream.o src/stream/chacha/chacha_setup.o src/stream/chacha/chacha_test.o \
src/stream/rc4/rc4_stream.o src/stream/rc4/rc4_test.o src/stream/sober128/sober128_stream.o \
src/stream/sober128/sober128_test.o

# List of test objects to compile (all goes to libtomcrypt_prof.a)
TOBJECTS=tests/base64_test.o tests/cipher_hash_test.o tests/common.o tests/der_test.o tests/dh_test.o \
tests/dsa_test.o tests/ecc_test.o tests/file_test.o tests/katja_test.o tests/mac_test.o tests/misc_test.o \
tests/modes_test.o tests/mpi_test.o tests/multi_test.o tests/no_prng.o tests/pkcs_1_eme_test.o \
tests/pkcs_1_emsa_test.o tests/pkcs_1_oaep_test.o tests/pkcs_1_pss_test.o tests/pkcs_1_test.o \
tests/prng_test.o tests/rotate_test.o tests/rsa_test.o tests/store_test.o tests/test.o

# The following headers will be installed by "make install"
HEADERS=src/headers/tomcrypt.h src/headers/tomcrypt_argchk.h src/headers/tomcrypt_cfg.h \
src/headers/tomcrypt_cipher.h src/headers/tomcrypt_custom.h src/headers/tomcrypt_hash.h \
src/headers/tomcrypt_mac.h src/headers/tomcrypt_macros.h src/headers/tomcrypt_math.h \
src/headers/tomcrypt_misc.h src/headers/tomcrypt_pk.h src/headers/tomcrypt_pkcs.h \
src/headers/tomcrypt_prng.h

#These are the rules to make certain object files.
src/ciphers/aes/aes.o: src/ciphers/aes/aes.c src/ciphers/aes/aes_tab.c
src/ciphers/twofish/twofish.o: src/ciphers/twofish/twofish.c src/ciphers/twofish/twofish_tab.c
src/hashes/whirl/whirl.o: src/hashes/whirl/whirl.c src/hashes/whirl/whirltab.c
src/hashes/sha2/sha512.o: src/hashes/sha2/sha512.c src/hashes/sha2/sha384.c
src/hashes/sha2/sha512_224.o: src/hashes/sha2/sha512.c src/hashes/sha2/sha512_224.c
src/hashes/sha2/sha512_256.o: src/hashes/sha2/sha512.c src/hashes/sha2/sha512_256.c
src/hashes/sha2/sha256.o: src/hashes/sha2/sha256.c src/hashes/sha2/sha224.c

$(DOBJECTS): LTC_CFLAGS := -Itests $(LTC_CFLAGS)
$(TOBJECTS): LTC_CFLAGS := -Itests $(LTC_CFLAGS)

#Dependencies on *.h
$(OBJECTS): $(HEADERS)
$(DOBJECTS): $(HEADERS) $(THEADERS)
$(TOBJECTS): $(HEADERS) $(THEADERS)

all: $(call print-help,all,Builds the library and all demos and test utils (test $(UNBROKEN_DEMOS) $(BROKEN_DEMOS))) all_test $(BROKEN_DEMOS)

all_test: $(call print-help,all_test,Builds the library and all unbroken demos and test utils (test $(UNBROKEN_DEMOS))) test $(UNBROKEN_DEMOS)

bins: $(call print-help,bins,Builds the library and all useful demos) $(USEFUL_DEMOS)

#build the doxy files (requires Doxygen, tetex and patience)
doxygen: $(call print-help,doxygen,Builds the doxygen html documentation)
	$(MAKE) -C doc/ $@ V=$(V)
doxy: $(call print-help,doxy,Builds the complete doxygen documentation including refman.pdf (takes long to generate))
	$(MAKE) -C doc/ $@ V=$(V)
docs: $(call print-help,docs,Builds the Developer Manual)
	$(MAKE) -C doc/ $@ V=$(V)

doc/crypt.pdf: $(call print-help,doc/crypt.pdf,Builds the Developer Manual)
	$(MAKE) -C doc/ crypt.pdf V=$(V)


install_all: $(call print-help,install_all,Install everything - library bins docs tests) install install_bins install_docs

INSTALL_OPTS ?= -m 644

.common_install: $(LIBNAME)
	install -p -d $(DESTDIR)$(INCPATH)
	install -p -d $(DESTDIR)$(LIBPATH)
	$(INSTALL_CMD) -p $(INSTALL_OPTS) $(LIBNAME) $(DESTDIR)$(LIBPATH)/$(LIBNAME)
	install -p -m 644 $(HEADERS) $(DESTDIR)$(INCPATH)

$(DESTDIR)$(BINPATH):
	install -p -d $(DESTDIR)$(BINPATH)

.common_install_bins: $(USEFUL_DEMOS) $(DESTDIR)$(BINPATH)
	$(INSTALL_CMD) -p -m 775 $(USEFUL_DEMOS) $(DESTDIR)$(BINPATH)

install_docs: $(call print-help,install_docs,Installs the Developer Manual) doc/crypt.pdf
	install -p -d $(DESTDIR)$(DATAPATH)
	install -p -m 644 doc/crypt.pdf $(DESTDIR)$(DATAPATH)

install_test: $(call print-help,install_test,Installs the self-test binary) test $(DESTDIR)$(BINPATH)
	$(INSTALL_CMD) -p -m 775 $< $(DESTDIR)$(BINPATH)

install_hooks: $(call print-help,install_hooks,Installs the git hooks)
	for s in `ls hooks/`; do ln -s ../../hooks/$$s .git/hooks/$$s; done

HEADER_FILES=$(notdir $(HEADERS))
.common_uninstall:
	$(UNINSTALL_CMD) $(DESTDIR)$(LIBPATH)/$(LIBNAME)
	rm $(HEADER_FILES:%=$(DESTDIR)$(INCPATH)/%)

#This rule cleans the source tree of all compiled code, not including the pdf
#documentation.
clean: $(call print-help,clean,Clean everything besides the pdf documentation)
	find . -type f    -name "*.o"   \
               -o -name "*.lo"  \
               -o -name "*.a"   \
               -o -name "*.la"  \
               -o -name "*.obj" \
               -o -name "*.lib" \
               -o -name "*.exe" \
               -o -name "*.dll" \
               -o -name "*.so"  \
               -o -name "*.gcov"\
               -o -name "*.gcda"\
               -o -name "*.gcno"\
               -o -name "*.il"  \
               -o -name "*.dyn" \
               -o -name "*.dpi"  | xargs rm -f
	rm -f $(TIMING) $(TEST) $(DEMOS)
	rm -f *_tv.txt
	rm -f *.pc
	rm -rf `find . -type d -name "*.libs" | xargs`
	$(MAKE) -C doc/ clean

zipup: $(call print-help,zipup,Prepare the archives for a release) doc/crypt.pdf
	@# Update the index, so diff-index won't fail in case the pdf has been created.
	@#   As the pdf creation modifies crypt.tex, git sometimes detects the
	@#   modified file, but misses that it's put back to its original version.
	@git update-index --refresh
	@git diff-index --quiet HEAD -- || ( echo "FAILURE: uncommited changes or not a git" && exit 1 )
	@perl helper.pl --check-all || ( echo "FAILURE: helper.pl --check-all errors" && exit 1 )
	rm -rf libtomcrypt-$(VERSION) crypt-$(VERSION).*
	@# files/dirs excluded from "git archive" are defined in .gitattributes
	git archive --format=tar --prefix=libtomcrypt-$(VERSION)/ HEAD | tar x
	@echo 'fixme check'
	-@(find libtomcrypt-$(VERSION)/ -type f | xargs grep 'FIXM[E]') && echo '############## BEWARE: the "fixme" marker was found !!! ##############' || true
	mkdir -p libtomcrypt-$(VERSION)/doc
	cp doc/crypt.pdf libtomcrypt-$(VERSION)/doc/crypt.pdf
	tar -c libtomcrypt-$(VERSION)/ | xz -6e -c - > crypt-$(VERSION).tar.xz
	zip -9rq crypt-$(VERSION).zip libtomcrypt-$(VERSION)
	rm -rf libtomcrypt-$(VERSION)
	gpg -b -a crypt-$(VERSION).tar.xz
	gpg -b -a crypt-$(VERSION).zip

codecheck: $(call print-help,codecheck,Check the code of the library)
	perl helper.pl -a
	perlcritic *.pl

help: $(call print-help,help,That's what you're currently looking at)
