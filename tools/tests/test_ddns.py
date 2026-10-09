#!/usr/bin/env python3
"""Run host-side DDNS regressions: python3 tools/tests/test_ddns.py.

Compiles the actual hostname helper, verifier, configuration printf and update
conditions/argv block with mocked NVRAM and network/service boundaries. This
is not a full firmware build and never contacts DNS or a DDNS provider.
"""
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SERVICES = (ROOT / 'release/src/router/rc/services.c').read_text()
WATCHDOG = (ROOT / 'release/src/router/rc/watchdog.c').read_text()
MISC = (ROOT / 'release/src/router/shared/misc.c').read_text()


def function(source, signature):
    start = source.index(signature)
    begin = source.index('{', start)
    depth = 1
    end = begin + 1
    while depth:
        if source[end] == '{':
            depth += 1
        elif source[end] == '}':
            depth -= 1
        end += 1
    return source[start:end]


def condition(source, anchor):
    anchor_at = source.index(anchor)
    start = source.rfind('if (', 0, anchor_at + len(anchor)) + len('if (')
    depth = 1
    end = start
    while depth:
        if source[end] == '(':
            depth += 1
        elif source[end] == ')':
            depth -= 1
        end += 1
    return source[start:end - 1]


PREFIX = r'''
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define WAN_UNIT_FIRST 0
#define WAN_UNIT_MAX 2
#define inet_addr_ inet_addr
static char *server="WWW.NAMECHEAP.COM", *host="home", *domain="example.com";
static char *cached_ip="192.0.2.10", *wan_ip="192.0.2.10";
static int cache_exists=1, selected_unit=0, realip=0, le_enabled=0;
static int dns_fails=0, starts=0, stopped=0, forced=0, ddns_check_count, last_unit=0;
static char queried[256];
static unsigned long resolved_address;
static char *nvram_safe_get(const char *key) {
    if (!strcmp(key,"ddns_server_x") || !strcmp(key,"ddns_server_x_old")) return server;
    if (!strcmp(key,"ddns_hostname_x") || !strcmp(key,"ddns_hostname_old")) return host;
    if (!strcmp(key,"ddns_username_x")) return domain;
    if (!strcmp(key,"ddns_ipaddr")) return cached_ip;
    if (!strcmp(key,"ddns_ipv6_service_old")) return "disabled";
    if (!strcmp(key,"wans_mode")) return "lb";
    return "";
}
static int nvram_match(const char *key, const char *value) {return !strcmp(nvram_safe_get(key),value);}
static int nvram_get_int(const char *key) {
    if (!strcmp(key,"ddns_wan_unit")) return selected_unit;
    if (!strcmp(key,"ddns_realip_x")) return realip;
    if (!strcmp(key,"ddns_last_wan_unit")) return last_unit;
    if (!strcmp(key,"le_enable")) return le_enabled;
    return 0;
}
static int nvram_pf_match(const char *prefix,const char *key,const char *value) {
    (void)prefix; (void)key; return !strcmp(wan_ip,value);
}
static int f_exists(const char *path) {(void)path; return cache_exists;}
static int ipv6_enabled(void) {return 0;}
static int wan_primary_ifunit(void) {return 0;}
static int rtk_wan_primary_ifunit(void) {return 0;}
static int get_first_connected_public_wan_unit(void) {return 0;}
static int get_first_connected_dual_wan_unit(void) {return 0;}
static int is_wan_connect(int unit) {(void)unit; return 1;}
static void logmessage(const char *tag,const char *format,...) {(void)tag; (void)format;}
static void stop_ddns(void) {stopped++;}
static int start_ddns(char *caller,int aidisk) {
    (void)aidisk; starts++; forced=caller && !strcmp(caller,"force"); return 0;
}
static struct hostent *mock_gethostbyname(const char *name) {
    static struct hostent result;
    static char *addresses[2];
    snprintf(queried,sizeof(queried),"%s",name);
    if (dns_fails) return NULL;
    addresses[0]=(char *)&resolved_address; addresses[1]=NULL;
    result.h_addr_list=addresses;
    return &result;
}
#define gethostbyname mock_gethostbyname
'''


def harness():
    cached = condition(SERVICES, 'if (!force_update &&')
    invalidate = condition(SERVICES, 'if (force_update ||')
    argv_start = SERVICES.index('char *inadyn_argv[8];')
    argv_end = SERVICES.index('if ((fp = fopen("/etc/inadyn.conf"', argv_start)
    argv_block = SERVICES[argv_start:argv_end]
    url_line = next(line for line in SERVICES.splitlines() if 'ddns-path = ' in line and '/update?domain=' in line)
    force_line = next(line for line in SERVICES.splitlines() if 'int force_update = ' in line)
    return (PREFIX + '\n' + function(MISC, 'int is_valid_domainname(') + '\n'
            + function(MISC, 'char *get_ddns_hostname(') + '\n'
            + function(WATCHDOG, 'void regular_ddns_check(') + '\n'
            + 'static int skip_cached(int force_update) {\n'
            + 'char cache_path[512]="cache", ip6_addr[INET6_ADDRSTRLEN]="", ipv6_service_cur[16]="disabled";\n'
            + '(void)ip6_addr; (void)ipv6_service_cur;\nreturn (' + cached + ');\n}\n'
            + 'static int invalidate_cached(int force_update) {\n'
            + 'char cache_path[512]="cache", ip6_addr[INET6_ADDRSTRLEN]="";\n(void)ip6_addr;\n'
            + 'return (' + invalidate + ');\n}\n'
            + 'static int caller_forces(char *caller) {\n' + force_line + '\nreturn force_update;\n}\n'
            + 'static int forced_argv(int force_update,int asus_ddns,int isAidisk) {\n'
            + '(void)asus_ddns;\n' + argv_block
            + '\nint i, result=0;\nfor(i=0;i<8 && inadyn_argv[i];i++) if(!strcmp(inadyn_argv[i],"--force")) result=1;\n'
            + 'assert(i<8); return result;\n}\n'
            + 'static void check_url(void) {\nFILE *fp=tmpfile(); char buf[256]; assert(fp);\n'
            + url_line + '\nrewind(fp); assert(fgets(buf,sizeof(buf),fp)); fclose(fp);\n'
            + 'assert(strstr(buf,"domain=%u&password=%p&host=%h&ip=%i"));\n}\n'
            + r'''
int main(void) {
    char oversized[300];
    assert(!strcmp(get_ddns_hostname(),"home.example.com"));
    host="@"; assert(!strcmp(get_ddns_hostname(),"example.com"));
    host="office.home"; assert(!strcmp(get_ddns_hostname(),"office.home.example.com"));
    host="home"; domain=""; assert(!*get_ddns_hostname());
    domain="example.com"; host=""; assert(!*get_ddns_hostname());
    host="bad host"; assert(!*get_ddns_hostname());
    host="*"; assert(!*get_ddns_hostname());
    memset(oversized,'a',sizeof(oversized)-1); oversized[sizeof(oversized)-1]=0;
    host="home"; domain=oversized; assert(!*get_ddns_hostname());
    domain="example.com"; server="WWW.DYNDNS.ORG"; host="home.example.com";
    assert(!strcmp(get_ddns_hostname(),host));
    server="WWW.DNSOMATIC.COM"; host="all.dnsomatic.com"; assert(!*get_ddns_hostname());
    server="WWW.TUNNELBROKER.NET"; host="home.example.com"; assert(!*get_ddns_hostname());
    server="WWW.NAMECHEAP.COM"; host="home";
    check_url();
    assert(!caller_forces(NULL)); assert(!caller_forces("watchdog"));
    assert(caller_forces("force")); assert(!caller_forces("force-other"));
    assert(skip_cached(0)); assert(!skip_cached(1));
    assert(!invalidate_cached(0)); assert(invalidate_cached(1));
    cache_exists=0; assert(!skip_cached(0)); assert(invalidate_cached(0)); cache_exists=1;
    cached_ip="192.0.2.20"; assert(!skip_cached(0)); assert(invalidate_cached(0)); cached_ip=wan_ip;
    assert(!forced_argv(0,10,0)); assert(forced_argv(1,10,0));
    assert(forced_argv(1,1,0));
#ifdef RTCONFIG_LETSENCRYPT
    le_enabled=1; assert(forced_argv(0,1,0)); assert(forced_argv(1,1,0));
#endif
    resolved_address=inet_addr("192.0.2.20"); regular_ddns_check();
    assert(!strcmp(queried,"home.example.com")); assert(starts==1 && stopped==1 && forced);
    starts=stopped=forced=0; resolved_address=inet_addr(wan_ip); regular_ddns_check();
    assert(!starts && !stopped);
    dns_fails=1; regular_ddns_check(); assert(!starts && !stopped);
    dns_fails=0; realip=1; regular_ddns_check(); assert(starts==1 && !forced);
    starts=0; last_unit=1; regular_ddns_check(); assert(starts==1 && forced);
    puts("hostname, URL, cache, argv and verifier regressions passed");
    return 0;
}
''')


class DdnsTests(unittest.TestCase):
    def test_feature_combinations(self):
        combinations = [[], ['RTCONFIG_INADYN', 'RTCONFIG_DUALWAN'],
                        ['RTCONFIG_INADYN', 'RTCONFIG_IPV6', 'RTCONFIG_LETSENCRYPT', 'RTCONFIG_DUALWAN'],
                        ['RTCONFIG_INADYN', 'RTCONFIG_IPV6', 'RTCONFIG_MULTIWAN_IF', 'RPAC68U']]
        with tempfile.TemporaryDirectory() as temp:
            source = Path(temp) / 'ddns.c'
            source.write_text(harness())
            for defines in combinations:
                with self.subTest(defines=defines):
                    executable = Path(temp) / 'ddns-test'
                    command = shlex.split(os.environ.get('CC', 'cc')) + [
                        '-std=gnu99', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function',
                        '-Wno-unused-parameter', *['-D' + value for value in defines],
                        str(source), '-o', str(executable)]
                    result = subprocess.run(command, capture_output=True, text=True)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    result = subprocess.run([str(executable)], capture_output=True, text=True)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_refresh_dispatches_force(self):
        refresh = WATCHDOG[WATCHDOG.index('/* Force a DDNS update every'):]
        refresh = refresh[:refresh.index('networkmap_check();')]
        self.assertIn('notify_rc("restart_ddns force")', refresh)
        dispatch = SERVICES[SERVICES.index('else if (strcmp(script, "ddns") == 0)'):]
        dispatch = dispatch[:dispatch.index('else if', len('else if'))]
        self.assertIn('start_ddns(cmd[1], 0)', dispatch)


if __name__ == '__main__':
    unittest.main()
