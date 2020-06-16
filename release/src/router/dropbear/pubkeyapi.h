/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */
#ifndef DROPBEAR_PUBKEY_H
#define DROPBEAR_PUBKEY_H


/* External Public Key API (EPKA) Plug-in Interface
 *
 * See:
 *      https://github.com/fabriziobertocci/dropbear-epka
 * for additional information and examples about this API
 *
 */

struct PluginInstance;
struct PluginSession;

/* API VERSION INFORMATION - 
 * Dropbear will:
 * - Reject any plugin with a major version mismatch
 * - Load and print a warning if the plugin's minor version is HIGHER than
 *   dropbear's minor version (assumes properties are added at the end of
 *   PluginInstance or PluginSession). This is a case of plugin newer than dropbear. 
 * - Reject if the plugin minor version is SMALLER than dropbear one (case
 *   of plugin older than dropbear).
 * - Load (with no warnings) if version match.
 */
#define DROPBEAR_PLUGIN_VERSION_MAJOR     1
#define DROPBEAR_PLUGIN_VERSION_MINOR     0


/* Creates an instance of the plugin.
 *
 * This is the main entry point of the plug-in and should be IMMUTABLE across
 * different API versions. Dropbear will check the version number
 * returned in the api_version to match the version it understands and reject
 * any plugin for which API major version does not match.
 *
 * If the version MINOR is different, dropbear will allow the plugin to run 
 * only if: plugin_MINOR > dropbear_MINOR
 *
 * If plugin_MINOR < dropbear_MINOR or if the MAJOR version is different
 * dropbear will reject the plugin and terminate the execution.
 *
 * addrstring is the IP address of the client.
 *
 * Returns NULL in case of failure, otherwise a void * of the instance that need
 * to be passed to all the subsequent call to the plugin
 */
typedef struct PluginInstance *(* PubkeyExtPlugin_newFn)(int verbose, 
        const char *options,
        const char *addrstring);
#define DROPBEAR_PUBKEY_PLUGIN_FNNAME_NEW               "plugin_new"


/* Validate a client through public key authentication
 *
 * If session has not been already created, creates it and store it 
 * in *sessionInOut.
 * If session is a non-NULL, it will reuse it.
 *
 * Returns DROPBEAR_SUCCESS (0) if success or DROPBEAR_FAILURE (-1) if
 * authentication fails
 */
typedef int (* PubkeyExtPlugin_checkPubKeyFn)(struct PluginInstance *PluginInstance,
        struct PluginSession **sessionInOut,
        const char* algo, 
        unsigned int algolen,
        const unsigned char* keyblob, 
        unsigned int keybloblen,
        const char *username);

/* Notify the plugin that auth completed (after signature verification)
 */
typedef void (* PubkeyExtPlugin_authSuccessFn)(struct PluginSession *session);

/* Deletes a session
 * TODO: Add a reason why the session is terminated. See svr_dropbear_exit (in svr-session.c)
 */
typedef void (* PubkeyExtPlugin_sessionDeleteFn)(struct PluginSession *session);

/* Deletes the plugin instance */
typedef void (* PubkeyExtPlugin_deleteFn)(struct PluginInstance *PluginInstance);


/* The PluginInstance object - A simple container of the pointer to the functions used
 * by Dropbear.
 *
 * A plug-in can extend it to add its own properties
 *
 * The instance is created from the call to the plugin_new() function of the 
 * shared library.
 * The delete_plugin function should delete the object.
 */
struct PluginInstance {
    int                             api_version[2];         /* 0=Major, 1=Minor */

    PubkeyExtPlugin_checkPubKeyFn   checkpubkey;            /* mandatory */
    PubkeyExtPlugin_authSuccessFn   auth_success;           /* optional */
    PubkeyExtPlugin_sessionDeleteFn delete_session;         /* mandatory */
    PubkeyExtPlugin_deleteFn        delete_plugin;          /* mandatory */
};

/*****************************************************************************
 * SESSION
 ****************************************************************************/
/* Returns the options from the session. 
 * The returned buffer will be destroyed when the session is deleted.
 * Option buffer string NULL-terminated
 */
typedef char * (* PubkeyExtPlugin_getOptionsFn)(struct PluginSession *session);


/* An SSH Session. Created during pre-auth and reused during the authentication.
 * The plug-in should delete this object (or any object extending it) from 
 * the delete_session() function.
 *
 * Extend it to cache user and authentication information that can be
 * reused between pre-auth and auth (and to store whatever session-specific
 * variable you need to keep).
 *
 * Store any optional auth options in the auth_options property of the session.
 */
struct PluginSession {
    struct PluginInstance *  plugin_instance;

    PubkeyExtPlugin_getOptionsFn   get_options;
};

#endif
