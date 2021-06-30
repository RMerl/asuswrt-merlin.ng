/*
 * HEADER Testing netsnmp_parse_ep_str()
 */

struct one_test_data {
    const char *in;
    int res;
    struct netsnmp_ep_str expected;
};

static struct one_test_data test_data[] = {
    { "9999",              1, { "",         "",   "9999" } },
    { ":777",              1, { "",         "",   "777"  } },
    { "hostname:777",      1, { "hostname", "",   "777"  } },
    { "hostname",          1, { "hostname", "",   ""     } },
    { "1.2.3.4",           1, { "1.2.3.4",  "",   ""     } },
    { "hostname@if",       1, { "hostname", "if", ""     } },
    { "hostname@if:833",   1, { "hostname", "if", "833"  } },
    { "[hostname]@if:833", 1, { "hostname", "if", "833"  } },
    { "[hostname]?",       0, {                          } },
    { "[hostname",         0, {                          } },
    { "@if:844",           1, { "",         "if", "844"  } },
    { "::1",               1, { "::1",      "",   ""     } },
    { "::1@if",            1, { "::1",      "if", ""     } },
    { "1::2",              1, { "1::2",     "",   ""     } },
    { "[::1]",             1, { "::1",      "",   ""     } },
    { "[::1]:0",           1, { "::1",      "",   "0"    } },
    { "[::1]:2",           1, { "::1",      "",   "2"    } },
    { "[::1]@if:2",        1, { "::1",      "if", "2"    } },
    { "[::1]:2@if",        0, {                          } },
};

SOCK_STARTUP;

{
    int i;

    for (i = 0; i < sizeof(test_data) / sizeof(test_data[0]); i++) {
        const struct one_test_data *p = &test_data[i];
        struct netsnmp_ep_str ep_str;
        int res;

        memset(&ep_str, 0, sizeof(ep_str));
        res = netsnmp_parse_ep_str(&ep_str, p->in);
        OKF(res == p->res, ("%s: return value %d <> %d", p->in, res, p->res));
        if (res && p->res) {
            OKF(strcmp(ep_str.addr,  p->expected.addr)  == 0,
                ("%s: network address %s <> %s", p->in, ep_str.addr,
                 p->expected.addr));
            OKF(strcmp(ep_str.iface, p->expected.iface) == 0,
                ("%s: network interface %s <> %s", p->in, ep_str.iface,
                 p->expected.iface));
            OKF(strcmp(ep_str.port, p->expected.port) == 0,
                ("%s: port %s <> %s", p->in, ep_str.port, p->expected.port));
        }
    }
}

SOCK_CLEANUP;
