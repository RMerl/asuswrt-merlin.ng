#define FUZZ_NO_REPLACE_STDERR
#define FUZZ_NO_REPLACE_GETPW
#include "includes.h"

#include "includes.h"
#include "dbutil.h"
#include "runopts.h"
#include "crypto_desc.h"
#include "session.h"
#include "dbrandom.h"
#include "bignum.h"
#include "atomicio.h"
#include "fuzz-wrapfd.h"
#include "fuzz.h"

struct dropbear_fuzz_options fuzz;

static void fuzz_dropbear_log(int UNUSED(priority), const char* format, va_list param);
static void load_fixed_hostkeys(void);
static void load_fixed_client_key(void);

// This runs automatically before main, due to contructor attribute in fuzz.h
void fuzz_early_setup(void) {
    /* Set stderr to point to normal stderr by default */
    fuzz.fake_stderr = stderr;
}

void fuzz_common_setup(void) {
	disallow_core();
    fuzz.fuzzing = 1;
    fuzz.wrapfds = 1;
    fuzz.do_jmp = 1;
    fuzz.input = m_malloc(sizeof(buffer));
    _dropbear_log = fuzz_dropbear_log;
    crypto_init();
    fuzz_seed("start", 5);
    /* let any messages get flushed */
    setlinebuf(stdout);
#if DEBUG_TRACE
    if (debug_trace)
    {
        fprintf(stderr, "Dropbear fuzzer: -v specified, not disabling stderr output\n");
    }
    else
#endif
    if (getenv("DROPBEAR_KEEP_STDERR")) {
        fprintf(stderr, "Dropbear fuzzer: DROPBEAR_KEEP_STDERR, not disabling stderr output\n");
    } 
    else 
    {
        fprintf(stderr, "Dropbear fuzzer: Disabling stderr output\n");
        fuzz.fake_stderr = fopen("/dev/null", "w");
        assert(fuzz.fake_stderr);
    }
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
    // printhex("input", fuzz.input->data, fuzz.input->len);

    fuzz_seed(fuzz.input->data, MIN(fuzz.input->len, 16));

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

void fuzz_svr_hook_preloop() {
    if (fuzz.svr_postauth) {
        ses.authstate.authdone = 1;
        fill_passwd("root");
    }
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
    ses.newkeys->recv.algo_mac = &dropbear_nohash;
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
    *ret_writefd = wrapfd_new_dummy();
    *ret_readfd = wrapfd_new_dummy();
    if (ret_errfd) {
        *ret_errfd = wrapfd_new_dummy();
    }
    if (*ret_writefd == -1 || *ret_readfd == -1 || (ret_errfd && *ret_errfd == -1)) {
        m_close(*ret_writefd);
        m_close(*ret_readfd);
        if (ret_errfd) {
            m_close(*ret_errfd);
        }
        return DROPBEAR_FAILURE;
    } else {
        *ret_pid = 999;
        return DROPBEAR_SUCCESS;

    }
}

/* Fake dropbear_listen, always returns failure for now.
TODO make it sometimes return success with wrapfd_new_dummy() sockets.
Making the listeners fake a new incoming connection will be harder. */
/* Listen on address:port. 
 * Special cases are address of "" listening on everything,
 * and address of NULL listening on localhost only.
 * Returns the number of sockets bound on success, or -1 on failure. On
 * failure, if errstring wasn't NULL, it'll be a newly malloced error
 * string.*/
int fuzz_dropbear_listen(const char* UNUSED(address), const char* UNUSED(port),
        int *UNUSED(socks), unsigned int UNUSED(sockcount), char **errstring, int *UNUSED(maxfd)) {
    if (errstring) {
        *errstring = m_strdup("fuzzing can't listen (yet)");
    }
    return -1;
}

int fuzz_run_server(const uint8_t *Data, size_t Size, int skip_kexmaths, int postauth) {
    static int once = 0;
    if (!once) {
        fuzz_svr_setup();
        fuzz.skip_kexmaths = skip_kexmaths;
        once = 1;
    }

    fuzz.svr_postauth = postauth;

    if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
        return 0;
    }

    uint32_t wrapseed;
    genrandom((void*)&wrapseed, sizeof(wrapseed));
    wrapfd_setseed(wrapseed);

    int fakesock = wrapfd_new_fuzzinput();

    m_malloc_set_epoch(1);
    fuzz.do_jmp = 1;
    if (setjmp(fuzz.jmp) == 0) {
        svr_session(fakesock, fakesock);
        m_malloc_free_epoch(1, 0);
    } else {
        fuzz.do_jmp = 0;
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

    // Allow to proceed sooner
    ses.kexstate.donefirstkex = 1;

    uint32_t wrapseed;
    genrandom((void*)&wrapseed, sizeof(wrapseed));
    wrapfd_setseed(wrapseed);

    int fakesock = wrapfd_new_fuzzinput();

    m_malloc_set_epoch(1);
    fuzz.do_jmp = 1;
    if (setjmp(fuzz.jmp) == 0) {
        cli_session(fakesock, fakesock, NULL, 0);
        m_malloc_free_epoch(1, 0);
    } else {
        fuzz.do_jmp = 0;
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
    if (fuzz.dumping) {
        TRACE(("dump %zu", len))
        assert(atomicio(vwrite, fuzz.recv_dumpfd, (void*)data, len) == len);
    }
}

static struct passwd pwd_root = {
    .pw_name = "root",
    .pw_passwd = "!",
    .pw_uid = 0,
    .pw_gid = 0,
    .pw_dir = "/root",
    .pw_shell = "/bin/sh",
};

static struct passwd pwd_other = {
    .pw_name = "other",
    .pw_passwd = "!",
    .pw_uid = 100,
    .pw_gid = 100,
    .pw_dir = "/home/other",
    .pw_shell = "/bin/sh",
};


/* oss-fuzz runs fuzzers under minijail, without /etc/passwd.
We provide sufficient values for the fuzzers to run */
struct passwd* fuzz_getpwnam(const char *login) {
    if (!fuzz.fuzzing) {
        return getpwnam(login);
    }
    if (strcmp(login, pwd_other.pw_name) == 0) {
        return &pwd_other;
    }
    if (strcmp(login, pwd_root.pw_name) == 0) {
        return &pwd_root;
    }
    return NULL;
}

struct passwd* fuzz_getpwuid(uid_t uid) {
    if (!fuzz.fuzzing) {
        return getpwuid(uid);
    }
    if (uid == pwd_other.pw_uid) {
        return &pwd_other;
    }
    if (uid == pwd_root.pw_uid) {
        return &pwd_root;
    }
    return NULL;
}

