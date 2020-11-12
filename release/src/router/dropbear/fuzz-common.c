#include "includes.h"

#include "includes.h"
#include "fuzz.h"
#include "dbutil.h"
#include "runopts.h"
#include "crypto_desc.h"
#include "session.h"
#include "dbrandom.h"
#include "bignum.h"
#include "atomicio.h"
#include "fuzz-wrapfd.h"

struct dropbear_fuzz_options fuzz;

static void fuzz_dropbear_log(int UNUSED(priority), const char* format, va_list param);
static void load_fixed_hostkeys(void);
static void load_fixed_client_key(void);

void fuzz_common_setup(void) {
	disallow_core();
    fuzz.fuzzing = 1;
    fuzz.wrapfds = 1;
    fuzz.do_jmp = 1;
    fuzz.input = m_malloc(sizeof(buffer));
    _dropbear_log = fuzz_dropbear_log;
    crypto_init();
    fuzz_seed();
    /* let any messages get flushed */
    setlinebuf(stdout);
}

int fuzz_set_input(const uint8_t *Data, size_t Size) {

    fuzz.input->data = (unsigned char*)Data;
    fuzz.input->size = Size;
    fuzz.input->len = Size;
    fuzz.input->pos = 0;

    memset(&ses, 0x0, sizeof(ses));
    memset(&svr_ses, 0x0, sizeof(svr_ses));
    memset(&cli_ses, 0x0, sizeof(cli_ses));
    wrapfd_setup(fuzz.input);

    fuzz_seed();

    return DROPBEAR_SUCCESS;
}

#if DEBUG_TRACE
static void fuzz_dropbear_log(int UNUSED(priority), const char* format, va_list param) {
    if (debug_trace) {
        char printbuf[1024];
        vsnprintf(printbuf, sizeof(printbuf), format, param);
        fprintf(stderr, "%s\n", printbuf);
    }
}
#else
static void fuzz_dropbear_log(int UNUSED(priority), const char* UNUSED(format), va_list UNUSED(param)) {
    /* No print */
}
#endif /* DEBUG_TRACE */

void fuzz_svr_setup(void) {
    fuzz_common_setup();
    
    _dropbear_exit = svr_dropbear_exit;

    char *argv[] = { 
		"dropbear",
        "-E", 
    };

    int argc = sizeof(argv) / sizeof(*argv);
    svr_getopts(argc, argv);

    load_fixed_hostkeys();
}

void fuzz_cli_setup(void) {
    fuzz_common_setup();
    
	_dropbear_exit = cli_dropbear_exit;
	_dropbear_log = cli_dropbear_log;

    char *argv[] = { 
		"dbclient",
		"-y",
        "localhost",
        "uptime"
    };

    int argc = sizeof(argv) / sizeof(*argv);
    cli_getopts(argc, argv);

    load_fixed_client_key();
    /* Avoid password prompt */
    setenv(DROPBEAR_PASSWORD_ENV, "password", 1);
}

#include "fuzz-hostkeys.c"   

static void load_fixed_client_key(void) {

    buffer *b = buf_new(3000);
    sign_key *key;
    enum signkey_type keytype;

    key = new_sign_key();
    keytype = DROPBEAR_SIGNKEY_ANY;
    buf_putbytes(b, keyed25519, keyed25519_len);
    buf_setpos(b, 0);
    if (buf_get_priv_key(b, key, &keytype) == DROPBEAR_FAILURE) {
        dropbear_exit("failed fixed ed25519 hostkey");
    }
    list_append(cli_opts.privkeys, key);

    buf_free(b);
}

static void load_fixed_hostkeys(void) {

    buffer *b = buf_new(3000);
    enum signkey_type type;

    TRACE(("load fixed hostkeys"))

    svr_opts.hostkey = new_sign_key();

    buf_setlen(b, 0);
    buf_putbytes(b, keyr, keyr_len);
    buf_setpos(b, 0);
    type = DROPBEAR_SIGNKEY_RSA;
    if (buf_get_priv_key(b, svr_opts.hostkey, &type) == DROPBEAR_FAILURE) {
        dropbear_exit("failed fixed rsa hostkey");
    }

    buf_setlen(b, 0);
    buf_putbytes(b, keyd, keyd_len);
    buf_setpos(b, 0);
    type = DROPBEAR_SIGNKEY_DSS;
    if (buf_get_priv_key(b, svr_opts.hostkey, &type) == DROPBEAR_FAILURE) {
        dropbear_exit("failed fixed dss hostkey");
    }

    buf_setlen(b, 0);
    buf_putbytes(b, keye, keye_len);
    buf_setpos(b, 0);
    type = DROPBEAR_SIGNKEY_ECDSA_NISTP256;
    if (buf_get_priv_key(b, svr_opts.hostkey, &type) == DROPBEAR_FAILURE) {
        dropbear_exit("failed fixed ecdsa hostkey");
    }

    buf_setlen(b, 0);
    buf_putbytes(b, keyed25519, keyed25519_len);
    buf_setpos(b, 0);
    type = DROPBEAR_SIGNKEY_ED25519;
    if (buf_get_priv_key(b, svr_opts.hostkey, &type) == DROPBEAR_FAILURE) {
        dropbear_exit("failed fixed ed25519 hostkey");
    }

    buf_free(b);
}

void fuzz_kex_fakealgos(void) {
    ses.newkeys->recv.crypt_mode = &dropbear_mode_none;
}

void fuzz_get_socket_address(int UNUSED(fd), char **local_host, char **local_port,
                        char **remote_host, char **remote_port, int UNUSED(host_lookup)) {
    if (local_host) {
        *local_host = m_strdup("fuzzlocalhost");
    }
    if (local_port) {
        *local_port = m_strdup("1234");
    }
    if (remote_host) {
        *remote_host = m_strdup("fuzzremotehost");
    }
    if (remote_port) {
        *remote_port = m_strdup("9876");
    }
}

/* cut down version of svr_send_msg_kexdh_reply() that skips slow maths. Still populates structures */
void fuzz_fake_send_kexdh_reply(void) {
    assert(!ses.dh_K);
    m_mp_alloc_init_multi(&ses.dh_K, NULL);
    mp_set_ul(ses.dh_K, 12345678uL);
    finish_kexhashbuf();
}

/* fake version of spawn_command() */
int fuzz_spawn_command(int *ret_writefd, int *ret_readfd, int *ret_errfd, pid_t *ret_pid) {
    *ret_writefd = wrapfd_new();
    *ret_readfd = wrapfd_new();
    if (ret_errfd) {
        *ret_errfd = wrapfd_new();
    }
    *ret_pid = 999;
    return DROPBEAR_SUCCESS;
}

int fuzz_run_preauth(const uint8_t *Data, size_t Size, int skip_kexmaths) {
    static int once = 0;
    if (!once) {
        fuzz_svr_setup();
        fuzz.skip_kexmaths = skip_kexmaths;
        once = 1;
    }

    if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
        return 0;
    }

    /*
      get prefix, allowing for future extensibility. input format is
      string prefix
          uint32 wrapfd seed
          ... to be extended later
      [bytes] ssh input stream
    */

    /* be careful to avoid triggering buffer.c assertions */
    if (fuzz.input->len < 8) {
        return 0;
    }
    size_t prefix_size = buf_getint(fuzz.input);
    if (prefix_size != 4) {
        return 0;
    }
    uint32_t wrapseed = buf_getint(fuzz.input);
    wrapfd_setseed(wrapseed);

    int fakesock = wrapfd_new();

    m_malloc_set_epoch(1);
    if (setjmp(fuzz.jmp) == 0) {
        svr_session(fakesock, fakesock);
        m_malloc_free_epoch(1, 0);
    } else {
        m_malloc_free_epoch(1, 1);
        TRACE(("dropbear_exit longjmped"))
        /* dropbear_exit jumped here */
    }

    return 0;
}

int fuzz_run_client(const uint8_t *Data, size_t Size, int skip_kexmaths) {
    static int once = 0;
    if (!once) {
        fuzz_cli_setup();
        fuzz.skip_kexmaths = skip_kexmaths;
        once = 1;
    }

    if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
        return 0;
    }

    /*
      get prefix, allowing for future extensibility. input format is
      string prefix
          uint32 wrapfd seed
          ... to be extended later
      [bytes] ssh input stream
    */

    /* be careful to avoid triggering buffer.c assertions */
    if (fuzz.input->len < 8) {
        return 0;
    }
    size_t prefix_size = buf_getint(fuzz.input);
    if (prefix_size != 4) {
        return 0;
    }
    uint32_t wrapseed = buf_getint(fuzz.input);
    wrapfd_setseed(wrapseed);

    int fakesock = wrapfd_new();

    m_malloc_set_epoch(1);
    if (setjmp(fuzz.jmp) == 0) {
        cli_session(fakesock, fakesock, NULL, 0);
        m_malloc_free_epoch(1, 0);
    } else {
        m_malloc_free_epoch(1, 1);
        TRACE(("dropbear_exit longjmped"))
        /* dropbear_exit jumped here */
    }

    return 0;
}

const void* fuzz_get_algo(const algo_type *algos, const char* name) {
    const algo_type *t;
    for (t = algos; t->name; t++) {
        if (strcmp(t->name, name) == 0) {
            return t->data;
        }
    }
    assert(0);
}

void fuzz_dump(const unsigned char* data, size_t len) {
    TRACE(("dump %zu", len))
    if (fuzz.dumping) {
        assert(atomicio(vwrite, fuzz.recv_dumpfd, (void*)data, len) == len);
    }
}
