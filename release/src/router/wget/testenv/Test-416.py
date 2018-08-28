#!/usr/bin/env python3
from sys import exit
from test.http_test import HTTPTest
from misc.wget_file import WgetFile

"""
    Ensure that Wget behaves well when the server responds with a HTTP 416
    status code. This test checks both cases:
        1. Server sends no body
        2. Server sends a body
"""
############# File Definitions ###############################################
File1 = "abababababababababababababababababababababababababababababababababab"
File2 = "ababababababababababababababababababab"

A_File = WgetFile ("File1", File1)
B_File = WgetFile ("File1", File1)

C_File = WgetFile ("File2", File2)
D_File = WgetFile ("File2", File1)

E_File = WgetFile ("File3", File1)

WGET_OPTIONS = "-c"
WGET_URLS = [["File1", "File2", "File3"]]

Files = [[A_File, C_File, E_File]]
Existing_Files = [B_File, D_File]

ExpectedReturnCode = 0
ExpectedDownloadedFiles = [B_File, D_File, E_File]

################ Pre and Post Test Hooks #####################################
pre_test = {
    "ServerFiles"       : Files,
    "LocalFiles"        : Existing_Files
}
test_options = {
    "WgetCommands"      : WGET_OPTIONS,
    "Urls"              : WGET_URLS
}
post_test = {
    "ExpectedFiles"     : ExpectedDownloadedFiles,
    "ExpectedRetcode"   : ExpectedReturnCode
}

err = HTTPTest (
                pre_hook=pre_test,
                test_params=test_options,
                post_hook=post_test
).begin ()

exit (err)
