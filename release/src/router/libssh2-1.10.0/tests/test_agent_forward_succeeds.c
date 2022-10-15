#include "session_fixture.h"

#include <libssh2.h>

#include <stdio.h>

const char *USERNAME = "libssh2"; /* set in Dockerfile */
const char *KEY_FILE_PRIVATE = "key_rsa";
const char *KEY_FILE_PUBLIC = "key_rsa.pub"; /* set in Dockerfile */

int test(LIBSSH2_SESSION *session)
{
    int rc;
    LIBSSH2_CHANNEL *channel;

    const char *userauth_list =
        libssh2_userauth_list(session, USERNAME, strlen(USERNAME));
    if(userauth_list == NULL) {
        print_last_session_error("libssh2_userauth_list");
        return 1;
    }

    if(strstr(userauth_list, "publickey") == NULL) {
        fprintf(stderr, "'publickey' was expected in userauth list: %s\n",
                userauth_list);
        return 1;
    }

    rc = libssh2_userauth_publickey_fromfile_ex(
        session, USERNAME, strlen(USERNAME), KEY_FILE_PUBLIC, KEY_FILE_PRIVATE,
        NULL);
    if(rc != 0) {
        print_last_session_error("libssh2_userauth_publickey_fromfile_ex");
        return 1;
    }

    channel = libssh2_channel_open_session(session);
    /* if(channel == NULL) { */
    /*     printf("Error opening channel\n"); */
    /*     return 1; */
    /* } */

    rc = libssh2_channel_request_auth_agent(channel);
    if(rc != 0) {
        fprintf(stderr, "Auth agent request for agent forwarding failed, "
            "error code %d\n", rc);
        return 1;
    }

    return 0;
}
