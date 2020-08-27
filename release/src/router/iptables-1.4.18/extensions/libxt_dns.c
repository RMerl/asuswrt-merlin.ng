#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <netinet/in.h>
#include <xtables.h>
#include <arpa/nameser.h>
//#include "autoconfig.h"
//#include "kernel.h"
#include <linux/netfilter/xt_dns.h>
#include <linux/netfilter/xt_dns_flags.h>

#ifndef DEBUG
#define DEBUG_PRINT(fmt, ...)                                                  \
    { printf("%s(%d):" fmt, __func__, __LINE__, ##__VA_ARGS__); }
#else
#define DEBUG_PRINT(...)
#endif

//#if KERNEL_VERSION >= 3
#define XT_PRINT(fmt, ...) printf(" " fmt, ##__VA_ARGS__)
//#else
//#define XT_PRINT(fmt, ...) printf(fmt " ", ##__VA_ARGS__)
//#endif

#define O_DNS_FLAG_QR '1'
#define O_DNS_FLAG_OPCODE '2'
#define O_DNS_FLAG_AA '3'
#define O_DNS_FLAG_TC '4'
#define O_DNS_FLAG_RD '5'
#define O_DNS_FLAG_RA '6'
#define O_DNS_FLAG_AD '7'
#define O_DNS_FLAG_CD '8'
#define O_DNS_FLAG_RCODE '9'
#define O_DNS_FLAG_QNAME 'a'
#define O_DNS_FLAG_QTYPE 'b'
#define O_DNS_FLAG_RMATCH 'c'
#define O_DNS_FLAG_QNAME_MAXSIZE 'd'

static const struct option dns_opts[] = {
    {.name = "qr", .has_arg = false, .val = O_DNS_FLAG_QR},
    {.name = "opcode", .has_arg = true, .val = O_DNS_FLAG_OPCODE},
    {.name = "aa", .has_arg = false, .val = O_DNS_FLAG_AA},
    {.name = "tc", .has_arg = false, .val = O_DNS_FLAG_TC},
    {.name = "rd", .has_arg = false, .val = O_DNS_FLAG_RD},
    {.name = "ra", .has_arg = false, .val = O_DNS_FLAG_RA},
    {.name = "ad", .has_arg = false, .val = O_DNS_FLAG_AD},
    {.name = "cd", .has_arg = false, .val = O_DNS_FLAG_CD},
    {.name = "rcode", .has_arg = true, .val = O_DNS_FLAG_RCODE},
    {.name = "qname", .has_arg = true, .val = O_DNS_FLAG_QNAME},
    {.name = "qtype", .has_arg = true, .val = O_DNS_FLAG_QTYPE},
    {.name = "rmatch", .has_arg = false, .val = O_DNS_FLAG_RMATCH},
    {.name = "maxsize", .has_arg = true, .val = O_DNS_FLAG_QNAME_MAXSIZE},
    {.name = NULL, .has_arg = false},
};

static void dns_help(void) {
    printf("dns match options:\n"
           "[!] --qr match when response\n"
           "[!] --opcode match\n"
           "      (Flags QUERY,IQUERY,STATUS,NOTIFY,UPDATE)\n"
           "[!] --aa match when Authoritative Answer\n"
           "[!] --tc match when Truncated Response\n"
           "[!] --rd match when Recursion Desired\n"
           "[!] --ra match when Recursion Available\n"
           "[!] --ad match when Authentic Data\n"
           "[!] --cd match when checking Disabled\n"
           "[!] --qname\n"
           "    --rmatch set qname match mode to reverse matching flag\n"
           "[!] --qtype\n"
           "      (Flags ex. A,AAAA,MX,NS,TXT,SOA... )\n"
           "	see. "
           "http://www.iana.org/assignments/dns-parameters/"
           "dns-parameters.xhtml\n"
           "[!] --maxsize qname max size \n");
}

static void dns_init(struct xt_entry_match *m) {
    struct xt_dns *data = (struct xt_dns *)m->data;

    data->qr = data->aa = data->tc = data->rd = false;
    data->ra = data->ad = data->cd = false;

    data->opcode = 0x00;
    data->rcode = 0x00;

    data->qname[0] = 0;
    data->qname_size = 1;
    data->qtype = 0xffff;

    data->invflags = 0x0000;
    data->setflags = 0x0000;

    data->rmatch = false;
    data->maxsize = XT_DNS_MAXSIZE;
}
static uint16_t parse_code_flag(const char *name, const char *flag,
                                const struct dns_flag_names *codes) {
    uint16_t i;
    uint16_t ret = 0;
    for (i = 0; codes[i].name != NULL; i++) {
        if (strcasecmp(codes[i].name, flag) == 0) {
            ret = codes[i].flag;
            break;
        }
    }
    if (codes[i].name == NULL) {
        xtables_error(PARAMETER_PROBLEM, "Unknown %s `%s'", name, flag);
    }
    return ret;
}
#define parse_opcode_flags(flag)                                               \
    parse_code_flag("OPCODE", flag, dns_flag_opcode)
#define parse_rcode_flags(flag) parse_code_flag("RCODE", flag, dns_flag_rcode)
#define parse_qtype_flags(flag) parse_code_flag("QTYPE", flag, dns_flag_qtype)

static void parse_qname(const char *flag, uint8_t *qname) {
    char buffer[XT_DNS_MAXSIZE];
    char *fp;
    fp = buffer;
    while (*flag != '\0') {
        *fp++ = tolower(*flag++);
    }
    *fp = '\0';
    if (ns_name_pton(buffer, qname, XT_DNS_MAXSIZE)) {
        xtables_error(PARAMETER_PROBLEM, "Invalid qname %s '%s'", flag, qname);
    }
}
static int qname_size(const uint8_t *qname) {
    uint8_t len = 0;
    uint8_t llen = 255;
    while (llen != 0 && len < XT_DNS_MAXSIZE) {
        llen = *(qname + len);
        len += llen + 1;
    }
    return len;
}

static int dns_parse(int c, char **argv, int invert, unsigned int *flags,
                     const void *entry, struct xt_entry_match **match) {
    struct xt_dns *data = (struct xt_dns *)(*match)->data;

    switch (c) {
    case O_DNS_FLAG_QR:
        if (*flags & XT_DNS_FLAG_QR) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--qr' allowed");
        }
        data->qr = true;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_QR;
        }
        *flags |= XT_DNS_FLAG_QR;
        break;
    case O_DNS_FLAG_OPCODE:
        if (*flags & XT_DNS_FLAG_OPCODE) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--opcode' allowed");
        }
        data->opcode = parse_opcode_flags(optarg);
        data->setflags |= XT_DNS_FLAG_OPCODE;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_OPCODE;
        }
        *flags |= XT_DNS_FLAG_OPCODE;
        break;
    case O_DNS_FLAG_AA:
        if (*flags & XT_DNS_FLAG_AA) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--aa' allowed");
        }
        data->aa = true;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_AA;
        }
        *flags |= XT_DNS_FLAG_AA;
        break;
    case O_DNS_FLAG_TC:
        if (*flags & XT_DNS_FLAG_TC) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--tc' allowed");
        }
        data->tc = true;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_TC;
        }
        *flags |= XT_DNS_FLAG_TC;
        break;
    case O_DNS_FLAG_RD:
        if (*flags & XT_DNS_FLAG_RD) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--rd' allowed");
        }
        data->rd = true;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_RD;
        }
        *flags |= XT_DNS_FLAG_RD;
        break;
    case O_DNS_FLAG_RA:
        if (*flags & XT_DNS_FLAG_RA) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--ra' allowed");
        }
        data->ra = true;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_RA;
        }
        *flags |= XT_DNS_FLAG_RA;
        break;
    case O_DNS_FLAG_AD:
        if (*flags & XT_DNS_FLAG_AD) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--ad' allowed");
        }
        data->ad = true;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_AD;
        }
        *flags |= XT_DNS_FLAG_AD;
        break;
    case O_DNS_FLAG_CD:
        if (*flags & XT_DNS_FLAG_CD) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--cd' allowed");
        }
        data->cd = true;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_CD;
        }
        *flags |= XT_DNS_FLAG_CD;
        break;
    case O_DNS_FLAG_RCODE:
        if (*flags & XT_DNS_FLAG_RCODE) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--rcode' allowed");
        }
        data->rcode = parse_rcode_flags(optarg);
        data->setflags |= XT_DNS_FLAG_RCODE;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_RCODE;
        }
        *flags |= XT_DNS_FLAG_RCODE;
        break;
    case O_DNS_FLAG_QNAME:
        if (*flags & XT_DNS_FLAG_QNAME) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--qname' allowed");
        }
        parse_qname(optarg, data->qname);
        data->qname_size = qname_size(data->qname);
        data->setflags |= XT_DNS_FLAG_QNAME;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_QNAME;
        }
        *flags |= XT_DNS_FLAG_QNAME;
        break;
    case O_DNS_FLAG_QTYPE:
        if (*flags & XT_DNS_FLAG_QTYPE) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--qtype' allowed");
        }
        data->qtype = htons(parse_qtype_flags(optarg));
        data->setflags |= XT_DNS_FLAG_QTYPE;
        if (invert) {
            data->invflags |= XT_DNS_FLAG_QTYPE;
        }
        *flags |= XT_DNS_FLAG_QTYPE;
        break;
    case O_DNS_FLAG_RMATCH:
        if (*flags & XT_DNS_FLAG_RMATCH) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--rmatch' allowed");
        }
        data->rmatch = true;
        if (invert) {
            xtables_error(PARAMETER_PROBLEM, "can't set invert `--rmatch' ");
        }
        *flags |= XT_DNS_FLAG_RMATCH;
        break;
    case O_DNS_FLAG_QNAME_MAXSIZE:
        if (*flags & XT_DNS_FLAG_QNAME_MAXSIZE) {
            xtables_error(PARAMETER_PROBLEM, "Only one `--maxsize' allowed");
        }
        data->maxsize = atoi(optarg);
        if (invert) {
            data->invflags |= XT_DNS_FLAG_QNAME_MAXSIZE;
        }
        *flags |= XT_DNS_FLAG_QNAME_MAXSIZE;
        break;

    default:
        return 0;
    }
    return 1;
}

static void print_flag(const char *name, bool value, uint16_t mask,
                       uint16_t invflag) {
    if (value) {
        if (mask & invflag) {
            XT_PRINT("!");
        }
        XT_PRINT("--%s", name);
    }
}
static void print_flag_attribute(const char *name, uint16_t value,
                                 uint16_t mask, uint16_t setflags,
                                 uint16_t invflag,
                                 const struct dns_flag_names *codes) {
    int i = 0;
    if (mask & setflags) {
        if (mask & invflag) {
            XT_PRINT("!");
        }
        for (i = 0; codes[i].name != NULL; i++) {
            if (codes[i].flag == value) {
                break;
            }
        }
        if (codes[i].name == NULL) {
            xtables_error(PARAMETER_PROBLEM, "Unknown %s `%d'", name, value);
        }
        XT_PRINT("--%s %s", name, codes[i].name);
    }
}

#define print_flag_opcode(value, setflags, invflags)                           \
    print_flag_attribute("opcode", value, XT_DNS_FLAG_OPCODE, setflags,        \
                         invflags, dns_flag_opcode)
#define print_flag_rcode(value, setflags, invflags)                            \
    print_flag_attribute("rcode", value, XT_DNS_FLAG_RCODE, setflags,          \
                         invflags, dns_flag_rcode)
#define print_flag_qtype(value, setflags, invflags)                            \
    print_flag_attribute("qtype", value, XT_DNS_FLAG_QTYPE, setflags,          \
                         invflags, dns_flag_qtype)

static void print_flag_qname(const u_char *qname, uint16_t setflags,
                             uint16_t invflag) {
    char tmp[XT_DNS_MAXSIZE];
    if (XT_DNS_FLAG_QNAME & setflags) {
        if (XT_DNS_FLAG_QNAME & invflag) {
            XT_PRINT("!");
        }
        if (ns_name_ntop(qname, tmp, sizeof(tmp)) == -1)
            xtables_error(PARAMETER_PROBLEM, "Unknown qname %s\n", tmp);
        XT_PRINT("--qname %s", tmp);
    }
}
static void print_maxsize(uint8_t maxsize, uint16_t invflag) {
    if (maxsize != XT_DNS_MAXSIZE) {
        if (XT_DNS_FLAG_QNAME_MAXSIZE & invflag) {
            XT_PRINT("!");
        }
        XT_PRINT("--maxsize %d", maxsize);
    }
}

static void dns_dump(const void *ip, const struct xt_entry_match *match) {
    const struct xt_dns *dns = (struct xt_dns *)match->data;
    print_flag("qr", dns->qr, XT_DNS_FLAG_QR, dns->invflags);
    print_flag_opcode(dns->opcode, dns->setflags, dns->invflags);
    print_flag("aa", dns->aa, XT_DNS_FLAG_AA, dns->invflags);
    print_flag("tc", dns->tc, XT_DNS_FLAG_TC, dns->invflags);
    print_flag("rd", dns->rd, XT_DNS_FLAG_RD, dns->invflags);
    print_flag("ra", dns->ra, XT_DNS_FLAG_RA, dns->invflags);
    print_flag("ad", dns->ad, XT_DNS_FLAG_AD, dns->invflags);
    print_flag("cd", dns->cd, XT_DNS_FLAG_CD, dns->invflags);
    print_flag_rcode(dns->rcode, dns->setflags, dns->invflags);
    print_flag_qname(dns->qname, dns->setflags, dns->invflags);
    print_flag_qtype(ntohs(dns->qtype), dns->setflags, dns->invflags);
    print_flag("rmatch", dns->rmatch, XT_DNS_FLAG_RMATCH, dns->invflags);
    print_maxsize(dns->maxsize, dns->invflags);
}

static void dns_print(const void *ip, const struct xt_entry_match *match,
                      int numeric) {
    XT_PRINT("dns");
    dns_dump(ip, match);
}

static void dns_save(const void *ip, const struct xt_entry_match *match) {
    dns_dump(ip, match);
}

static struct xtables_match dns_match = {
    .family = NFPROTO_UNSPEC,
    .name = "dns",
    .version = XTABLES_VERSION,
    .size = XT_ALIGN(sizeof(struct xt_dns)),
    .userspacesize = XT_ALIGN(sizeof(struct xt_dns)),
    .help = dns_help,
    .init = dns_init,
    .parse = dns_parse,
    .print = dns_print,
    .save = dns_save,
    .extra_opts = dns_opts,
};

void _init(void) {
    xtables_register_match(&dns_match);
}
