/*
 *  OpenVPN -- An application to securely tunnel IP networks
 *             over a single TCP/UDP port, with support for SSL/TLS-based
 *             session authentication and key exchange,
 *             packet encryption, packet authentication, and
 *             packet compression.
 *
 *  Copyright (C) 2002-2023 OpenVPN Inc <sales@openvpn.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "syshead.h"

#include "buffer.h"
#include "error.h"
#include "win32.h"
#include "init.h"
#include "status.h"
#include "sig.h"
#include "occ.h"
#include "manage.h"
#include "openvpn.h"

#include "memdbg.h"

/* Handle signals */

struct signal_info siginfo_static; /* GLOBAL */

struct signame {
    int value;
    int priority;
    const char *upper;
    const char *lower;
};

static const struct signame signames[] = {
    { SIGINT, 5, "SIGINT",  "sigint"},
    { SIGTERM, 4, "SIGTERM", "sigterm" },
    { SIGHUP, 3, "SIGHUP",  "sighup" },
    { SIGUSR1, 2, "SIGUSR1", "sigusr1" },
    { SIGUSR2, 1, "SIGUSR2", "sigusr2" }
};

int
parse_signal(const char *signame)
{
    int i;
    for (i = 0; i < (int)SIZE(signames); ++i)
    {
        if (!strcmp(signame, signames[i].upper))
        {
            return signames[i].value;
        }
    }
    return -1;
}

static int
signal_priority(int sig)
{
    for (size_t i = 0; i < SIZE(signames); ++i)
    {
        if (sig == signames[i].value)
        {
            return signames[i].priority;
        }
    }
    return -1;
}

const char *
signal_name(const int sig, const bool upper)
{
    int i;
    for (i = 0; i < (int)SIZE(signames); ++i)
    {
        if (sig == signames[i].value)
        {
            return upper ? signames[i].upper : signames[i].lower;
        }
    }
    return "UNKNOWN";
}

const char *
signal_description(const int signum, const char *sigtext)
{
    if (sigtext)
    {
        return sigtext;
    }
    else
    {
        return signal_name(signum, false);
    }
}

void
throw_signal(const int signum)
{
    if (signal_priority(signum) >= signal_priority(siginfo_static.signal_received))
    {
        siginfo_static.signal_received = signum;
        siginfo_static.source = SIG_SOURCE_HARD;
    }
}

void
throw_signal_soft(const int signum, const char *signal_text)
{
    if (signal_priority(signum) >= signal_priority(siginfo_static.signal_received))
    {
        siginfo_static.signal_received = signum;
        siginfo_static.source = SIG_SOURCE_SOFT;
        siginfo_static.signal_text = signal_text;
    }
}

void
signal_reset(struct signal_info *si)
{
    if (si)
    {
        si->signal_received = 0;
        si->signal_text = NULL;
        si->source = SIG_SOURCE_SOFT;
    }
}

void
print_signal(const struct signal_info *si, const char *title, int msglevel)
{
    if (si)
    {
        const char *type = (si->signal_text ? si->signal_text : "");
        const char *t = (title ? title : "process");
        const char *hs = NULL;
        switch (si->source)
        {
            case SIG_SOURCE_SOFT:
                hs = "soft";
                break;

            case SIG_SOURCE_HARD:
                hs = "hard";
                break;

            case SIG_SOURCE_CONNECTION_FAILED:
                hs = "connection failed(soft)";
                break;

            default:
                ASSERT(0);
        }

        switch (si->signal_received)
        {
            case SIGINT:
            case SIGTERM:
                msg(msglevel, "%s[%s,%s] received, %s exiting",
                    signal_name(si->signal_received, true), hs, type, t);
                break;

            case SIGHUP:
            case SIGUSR1:
                msg(msglevel, "%s[%s,%s] received, %s restarting",
                    signal_name(si->signal_received, true), hs, type, t);
                break;

            default:
                msg(msglevel, "Unknown signal %d [%s,%s] received by %s", si->signal_received, hs, type, t);
                break;
        }
    }
    else
    {
        msg(msglevel, "Unknown signal received");
    }
}

/*
 * Call management interface with restart info
 */
void
signal_restart_status(const struct signal_info *si)
{
#ifdef ENABLE_MANAGEMENT
    if (management)
    {
        int state = -1;
        switch (si->signal_received)
        {
            case SIGINT:
            case SIGTERM:
                state = OPENVPN_STATE_EXITING;
                break;

            case SIGHUP:
            case SIGUSR1:
                state = OPENVPN_STATE_RECONNECTING;
                break;
        }

        if (state >= 0)
        {
            management_set_state(management,
                                 state,
                                 si->signal_text ? si->signal_text : signal_name(si->signal_received, true),
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);
        }
    }
#endif /* ifdef ENABLE_MANAGEMENT */
}

#ifndef _WIN32
/* normal signal handler, when we are in event loop */
static void
signal_handler(const int signum)
{
    throw_signal(signum);
    signal(signum, signal_handler);
}
#endif


/* set handlers for unix signals */

#define SM_UNDEF     0
#define SM_PRE_INIT  1
#define SM_POST_INIT 2
static int signal_mode; /* GLOBAL */

void
pre_init_signal_catch(void)
{
#ifndef _WIN32
    signal_mode = SM_PRE_INIT;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
#endif /* _WIN32 */
}

void
post_init_signal_catch(void)
{
#ifndef _WIN32
    signal_mode = SM_POST_INIT;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    signal(SIGPIPE, SIG_IGN);
#endif
}

/* called after daemonization to retain signal settings */
void
restore_signal_state(void)
{
    if (signal_mode == SM_PRE_INIT)
    {
        pre_init_signal_catch();
    }
    else if (signal_mode == SM_POST_INIT)
    {
        post_init_signal_catch();
    }
}

/*
 * Print statistics.
 *
 * Triggered by SIGUSR2 or F2 on Windows.
 */
void
print_status(struct context *c, struct status_output *so)
{
    struct gc_arena gc = gc_new();

    status_reset(so);

    if (dco_enabled(&c->options))
    {
        dco_get_peer_stats(c);
    }

#ifdef ASUSWRT
    if(c->c2.to_link_addr)
    {
        status_printf (so, "REMOTE,%s:%d,Static_Key"
            , inet_ntoa(c->c2.to_link_addr->dest.addr.in4.sin_addr)
            , ntohs(c->c2.to_link_addr->dest.addr.in4.sin_port)
        );
    }
#endif

    status_printf(so, "OpenVPN STATISTICS");
    status_printf(so, "Updated,%s", time_string(0, 0, false, &gc));
    status_printf(so, "TUN/TAP read bytes," counter_format, c->c2.tun_read_bytes);
    status_printf(so, "TUN/TAP write bytes," counter_format, c->c2.tun_write_bytes);
    status_printf(so, "TCP/UDP read bytes," counter_format, c->c2.link_read_bytes + c->c2.dco_read_bytes);
    status_printf(so, "TCP/UDP write bytes," counter_format, c->c2.link_write_bytes + c->c2.dco_write_bytes);
    status_printf(so, "Auth read bytes," counter_format, c->c2.link_read_bytes_auth);
#ifdef USE_COMP
    if (c->c2.comp_context)
    {
        comp_print_stats(c->c2.comp_context, so);
    }
#endif
#ifdef PACKET_TRUNCATION_CHECK
    status_printf(so, "TUN read truncations," counter_format, c->c2.n_trunc_tun_read);
    status_printf(so, "TUN write truncations," counter_format, c->c2.n_trunc_tun_write);
    status_printf(so, "Pre-encrypt truncations," counter_format, c->c2.n_trunc_pre_encrypt);
    status_printf(so, "Post-decrypt truncations," counter_format, c->c2.n_trunc_post_decrypt);
#endif
#ifdef _WIN32
    if (tuntap_defined(c->c1.tuntap))
    {
        const char *extended_msg = tap_win_getinfo(c->c1.tuntap, &gc);
        if (extended_msg)
        {
            status_printf(so, "TAP-WIN32 driver status,\"%s\"", extended_msg);
        }
    }
#endif

    status_printf(so, "END");
    status_flush(so);
    gc_free(&gc);
}


/*
 * Handle the triggering and time-wait of explicit
 * exit notification.
 */
static void
process_explicit_exit_notification_init(struct context *c)
{
    msg(M_INFO, "SIGTERM received, sending exit notification to peer");
    /* init the timeout to send the OCC_EXIT messages if cc exit is not
     * enabled and also to exit after waiting for retries of resending of
     * exit messages */
    event_timeout_init(&c->c2.explicit_exit_notification_interval, 1, 0);
    reset_coarse_timers(c);

    signal_reset(c->sig);
    halt_non_edge_triggered_signals();
    c->c2.explicit_exit_notification_time_wait = now;

    /* Check if we are in TLS mode and should send the notification via data
     * channel */
    if (cc_exit_notify_enabled(c))
    {
        send_control_channel_string(c, "EXIT", D_PUSH);
    }
}

void
process_explicit_exit_notification_timer_wakeup(struct context *c)
{
    if (event_timeout_trigger(&c->c2.explicit_exit_notification_interval,
                              &c->c2.timeval,
                              ETT_DEFAULT))
    {
        ASSERT(c->c2.explicit_exit_notification_time_wait && c->options.ce.explicit_exit_notification);
        if (now >= c->c2.explicit_exit_notification_time_wait + c->options.ce.explicit_exit_notification)
        {
            event_timeout_clear(&c->c2.explicit_exit_notification_interval);
            register_signal(c->sig, SIGTERM, "exit-with-notification");
        }
        else if (!cc_exit_notify_enabled(c))
        {
            c->c2.occ_op = OCC_EXIT;
        }
    }
}

/*
 * Process signals
 */

void
remap_signal(struct context *c)
{
    if (c->sig->signal_received == SIGUSR1 && c->options.remap_sigusr1)
    {
        register_signal(c->sig, c->options.remap_sigusr1, c->sig->signal_text);
    }
}

static void
process_sigusr2(struct context *c)
{
    struct status_output *so = status_open(NULL, 0, M_INFO, NULL, 0);
    print_status(c, so);
    status_close(so);
    signal_reset(c->sig);
}

static bool
process_sigterm(struct context *c)
{
    bool ret = true;
    if (c->options.ce.explicit_exit_notification
        && !c->c2.explicit_exit_notification_time_wait)
    {
        process_explicit_exit_notification_init(c);
        ret = false;
    }
    return ret;
}

/**
 * If a restart signal is received during exit-notification, reset the
 * signal and return true. If its a soft restart signal from the event loop
 * which implies the loop cannot continue, remap to SIGTERM to exit promptly.
 */
static bool
ignore_restart_signals(struct context *c)
{
    bool ret = false;
    if ( (c->sig->signal_received == SIGUSR1 || c->sig->signal_received == SIGHUP)
         && event_timeout_defined(&c->c2.explicit_exit_notification_interval) )
    {
        if (c->sig->source == SIG_SOURCE_HARD)
        {
            msg(M_INFO, "Ignoring %s received during exit notification",
                signal_name(c->sig->signal_received, true));
            signal_reset(c->sig);
            ret = true;
        }
        else
        {
            msg(M_INFO, "Converting soft %s received during exit notification to SIGTERM",
                signal_name(c->sig->signal_received, true));
            register_signal(c->sig, SIGTERM, "exit-with-notification");
            ret = false;
        }
    }
    return ret;
}

bool
process_signal(struct context *c)
{
    bool ret = true;

    if (ignore_restart_signals(c))
    {
        ret = false;
    }
    else if (c->sig->signal_received == SIGTERM || c->sig->signal_received == SIGINT)
    {
        ret = process_sigterm(c);
    }
    else if (c->sig->signal_received == SIGUSR2)
    {
        process_sigusr2(c);
        ret = false;
    }
    return ret;
}

void
register_signal(struct signal_info *si, int sig, const char *text)
{
    if (signal_priority(sig) >= signal_priority(si->signal_received))
    {
        si->signal_received = sig;
        si->signal_text = text;
        si->source = SIG_SOURCE_SOFT;
    }
}
