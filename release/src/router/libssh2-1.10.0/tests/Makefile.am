SUBDIRS = ossfuzz

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include -I$(top_builddir)/src
LDADD = ../src/libssh2.la

if SSHD
noinst_PROGRAMS = ssh2
ssh2_SOURCES = ssh2.c
endif

ctests = simple$(EXEEXT)
TESTS = $(ctests) mansyntax.sh
if SSHD
TESTS += ssh2.sh
endif
check_PROGRAMS = $(ctests)

TESTS_ENVIRONMENT = SSHD=$(SSHD) EXEEXT=$(EXEEXT)
TESTS_ENVIRONMENT += srcdir=$(top_srcdir)/tests builddir=$(top_builddir)/tests

EXTRA_DIST =                                                           \
 CMakeLists.txt                                                        \
 etc/host                                                              \
 etc/host.pub                                                          \
 etc/user                                                              \
 etc/user.pub                                                          \
 key_dsa                                                               \
 key_dsa.pub                                                           \
 key_dsa_wrong                                                         \
 key_dsa_wrong.pub                                                     \
 key_ecdsa                                                             \
 key_ecdsa.pub                                                         \
 key_ed25519                                                           \
 key_ed25519.pub                                                       \
 key_ed25519_encrypted                                                 \
 key_ed25519_encrypted.pub                                             \
 key_rsa                                                               \
 key_rsa.pub                                                           \
 key_rsa_encrypted                                                     \
 key_rsa_encrypted.pub                                                 \
 key_rsa_openssh                                                       \
 key_rsa_openssh.pub                                                   \
 libssh2_config_cmake.h.in                                             \
 mansyntax.sh                                                          \
 openssh_fixture.c                                                     \
 openssh_fixture.h                                                     \
 openssh_server/Dockerfile                                             \
 openssh_server/authorized_keys                                        \
 openssh_server/ca_ecdsa                                               \
 openssh_server/ca_ecdsa.pub                                           \
 openssh_server/ssh_host_ecdsa_key                                     \
 openssh_server/ssh_host_ed25519_key                                   \
 openssh_server/ssh_host_rsa_key                                       \
 runner.c                                                              \
 session_fixture.c                                                     \
 session_fixture.h                                                     \
 simple.c                                                              \
 ssh2.c                                                                \
 ssh2.sh                                                               \
 sshd_fixture.sh.in                                                    \
 test_agent_forward_succeeds.c                                         \
 test_hostkey.c                                                        \
 test_hostkey_hash.c                                                   \
 test_keyboard_interactive_auth_fails_with_wrong_response.c            \
 test_keyboard_interactive_auth_succeeds_with_correct_response.c       \
 test_password_auth_fails_with_wrong_password.c                        \
 test_password_auth_fails_with_wrong_username.c                        \
 test_password_auth_succeeds_with_correct_credentials.c                \
 test_public_key_auth_fails_with_wrong_key.c                           \
 test_public_key_auth_succeeds_with_correct_dsa_key.c                  \
 test_public_key_auth_succeeds_with_correct_ed25519_key.c              \
 test_public_key_auth_succeeds_with_correct_ed25519_key_from_mem.c     \
 test_public_key_auth_succeeds_with_correct_ecdsa_key.c                \
 test_public_key_auth_succeeds_with_correct_signed_ecdsa_key.c         \
 test_public_key_auth_succeeds_with_correct_encrypted_ed25519_key.c    \
 test_public_key_auth_succeeds_with_correct_encrypted_rsa_key.c        \
 test_public_key_auth_succeeds_with_correct_rsa_key.c                  \
 test_public_key_auth_succeeds_with_correct_rsa_openssh_key.c
