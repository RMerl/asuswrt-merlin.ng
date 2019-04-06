#include "includes.h"

#include "includes.h"
#include "fuzz.h"
#include "dbutil.h"
#include "runopts.h"
#include "crypto_desc.h"
#include "session.h"
#include "dbrandom.h"
#include "bignum.h"
#include "fuzz-wrapfd.h"

struct dropbear_fuzz_options fuzz;

static void fuzz_dropbear_log(int UNUSED(priority), const char* format, va_list param);
static void load_fixed_hostkeys(void);

void fuzz_common_setup(void) {
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
    wrapfd_setup();

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
        "-E", 
    };

    int argc = sizeof(argv) / sizeof(*argv);
    svr_getopts(argc, argv);

    /* user lookups might be slow, cache it */
    fuzz.pw_name = m_strdup("person");
    fuzz.pw_dir = m_strdup("/tmp");
    fuzz.pw_shell = m_strdup("/bin/zsh");
    fuzz.pw_passwd = m_strdup("!!zzznope");

    load_fixed_hostkeys();
}

static void load_fixed_hostkeys(void) {
#include "fuzz-hostkeys.c"   

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
    mp_set_int(ses.dh_K, 12345678);
    finish_kexhashbuf();
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
      get prefix. input format is
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

    int fakesock = 20;
    wrapfd_add(fakesock, fuzz.input, PLAIN);

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

const void* fuzz_get_algo(const algo_type *algos, const char* name) {
    const algo_type *t;
    for (t = algos; t->name; t++) {
        if (strcmp(t->name, name) == 0) {
            return t->data;
        }
    }
    assert(0);
}
