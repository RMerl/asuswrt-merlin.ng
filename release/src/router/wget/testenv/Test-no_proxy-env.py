#!/usr/bin/env python3
import socket
from sys import exit
from test.http_test import HTTPTest
from test.base_test import SKIP_TEST
from test.base_test import HTTP
from misc.wget_file import WgetFile

"""
    This test ensures, that domains with and without leftmost dot defined in
    no_proxy environment variable are accepted by wget. The idea is to use
    non-existing proxy server address and detect whether files are downloaded
    when proxy settings are omitted based on no_proxy environment variable
    value.

    current wget's behavior:
    - "no_proxy=.mit.edu"
      - will match the domain and subdomains e.g. "www.mit.edu" or "www.subdomain.mit.edu" (Case #4)
      - will NOT match the host "mit.edu" (Case #3)
    - "no_proxy=mit.edu"
      - will match the domain and subdomains e.g. "www.mit.edu" or "www.subdomain.mit.edu" (Case #2)
      - will match the host "mit.edu" (Case #1)
    - downside: cannot match only the host
"""

# Check whether the system supports translating localhost subdomains
# to localhost address and if not, skip it.
hostnames_to_check = [
    "working1.localhost",
    "working2.localhost",
    "www.working1.localhost",
    "www.working2.localhost",
]
for hostname in hostnames_to_check:
    try:
        ip = socket.gethostbyname(hostname)
    except socket.gaierror as _:
        # resolution of the name fails
        # return value 77 -> SKIP
        exit(SKIP_TEST)

# File Definitions
File1 = "Would you like some Tea?"
File2 = "With lemon or cream?"

A_File = WgetFile ("File1", File1)
B_File = WgetFile ("File2", File2)

WGET_URLS = [["File1", "File2"]]
WGET_ENVS = {
    "http_proxy": "nonexisting.localhost:8080",
    "no_proxy": "working1.localhost,.working2.localhost"
}

Servers = [HTTP]
Files = [[A_File, B_File]]

ExpectedReturnCodeWorking = 0
ExpectedReturnCodeNotWorking = 4  # network error (non-existing proxy address)

ExpectedDownloadedFilesWorking = [A_File, B_File]

# Pre and Post Test Hooks
test_options = {
    "Urls"                : WGET_URLS,
    "EnvironmentVariables": WGET_ENVS
}
post_test_working = {
    "ExpectedFiles"     : ExpectedDownloadedFilesWorking,
    "ExpectedRetcode"   : ExpectedReturnCodeWorking
}
post_test_not_working = {
    "ExpectedRetcode"   : ExpectedReturnCodeNotWorking
}

# Case #1:
# - Requested domain matches exactly the domain definition in no_proxy.
# - Domain definition in no_proxy is NOT dot-prefixed
# Expected result: proxy settings don't apply and files are downloaded.
pre_case_1 = {
    "ServerFiles"       : Files,
    "Domains"           : ["working1.localhost"]
}

err_case_1 = HTTPTest (
    pre_hook=pre_case_1,
    test_params=test_options,
    post_hook=post_test_working,
    protocols=Servers
).begin ()

# Case #2:
# - Requested domain is sub-domain of a domain definition in no_proxy.
# - Domain definition in no_proxy is NOT dot-prefixed
# Expected result: proxy settings don't apply and files are downloaded.
pre_case_2 = {
    "ServerFiles"       : Files,
    "Domains"           : ["www.working1.localhost"]
}

err_case_2 = HTTPTest (
    pre_hook=pre_case_2,
    test_params=test_options,
    post_hook=post_test_working,
    protocols=Servers
).begin ()

# Case #3:
# - Requested domain matches exactly the domain definition in no_proxy,
#   except for the leftmost dot (".") in no_proxy domain definition.
# - Domain definition in no_proxy IS dot-prefixed
# Expected result: proxy settings apply and files are downloaded. This is
#                  due to the mismatch in leftmost dot.
# NOTE: This is inconsistent with curl's behavior, but has less drawbacks.
pre_case_3 = {
    "ServerFiles"       : Files,
    "Domains"           : ["working2.localhost"]
}

err_case_3 = HTTPTest (
    pre_hook=pre_case_3,
    test_params=test_options,
    post_hook=post_test_not_working,
    protocols=Servers
).begin ()

# Case #4:
# - Requested domain is sub-domain of a domain definition in no_proxy.
# - Domain definition in no_proxy IS dot-prefixed
# Expected result: proxy settings don't apply and files are downloaded.
pre_case_4 = {
    "ServerFiles"       : Files,
    "Domains"           : ["www.working2.localhost"]
}

err_case_4 = HTTPTest (
    pre_hook=pre_case_4,
    test_params=test_options,
    post_hook=post_test_working,
    protocols=Servers
).begin ()

# Case #5
# - Requested domain does not match a domain definition in no_proxy.
# - Requested domain is NOT sub-domain of a domain definition in no_proxy.
# Expected result: proxy settings apply and files are NOT downloaded due to
#                  network error when using proxy with non-existing URL.
pre_case_5 = {
    "ServerFiles"       : Files,
    "Domains"           : ["www.example.localhost"]
}

err_case_5 = HTTPTest (
    pre_hook=pre_case_5,
    test_params=test_options,
    post_hook=post_test_not_working,
    protocols=Servers
).begin ()

# Combine error codes from all test cases
exit (max(err_case_1, err_case_2, err_case_3, err_case_4, err_case_5))
