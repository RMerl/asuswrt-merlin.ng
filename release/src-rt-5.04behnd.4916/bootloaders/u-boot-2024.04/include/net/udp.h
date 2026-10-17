/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 */

#ifndef __UDP
#define __UDP

/**
 * struct udp_ops - function to handle udp packet
 *
 * This structure provides the function to handle udp packet in
 * the network loop.
 *
 * @prereq: callback called to check the requirement
 * @start: callback called to start the protocol/feature
 * @data: pointer to store private data (used by prereq and start)
 */
struct udp_ops {
	int (*prereq)(void *data);
	int (*start)(void *data);
	void *data;
};

int udp_prereq(void);

int udp_start(void);

/**
 * udp_loop() - network loop for udp protocol
 *
 * Launch a network loop for udp protocol and use callbacks
 * provided in parameter @ops to initialize the loop, and then
 * to handle udp packet.
 *
 * @ops: udp callback
 * @return: 0 if success, otherwise < 0 on error
 */
int udp_loop(struct udp_ops *ops);

#endif
