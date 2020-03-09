/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2018, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_MAINLOOP_PUBSUB_H
#define TOR_MAINLOOP_PUBSUB_H

struct pubsub_builder_t;

typedef enum {
   DELIV_NEVER=0,
   DELIV_PROMPT,
   DELIV_IMMEDIATE,
} deliv_strategy_t;

int tor_mainloop_connect_pubsub(struct pubsub_builder_t *builder);
void tor_mainloop_connect_pubsub_events(void);
int tor_mainloop_set_delivery_strategy(const char *msg_channel_name,
                                        deliv_strategy_t strategy);
void tor_mainloop_disconnect_pubsub(void);

#endif /* !defined(TOR_MAINLOOP_PUBSUB_H) */
