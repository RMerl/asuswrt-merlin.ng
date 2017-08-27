/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <unistd.h>
#include <fcntl.h>

#include "kernel_libipsec_router.h"

#include <daemon.h>
#include <hydra.h>
#include <ipsec.h>
#include <collections/hashtable.h>
#include <networking/tun_device.h>
#include <threading/rwlock.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>

typedef struct private_kernel_libipsec_router_t private_kernel_libipsec_router_t;

/**
 * Entry in the TUN device map
 */
typedef struct {
	/** virtual IP (points to internal data of tun) */
	host_t *addr;
	/** underlying TUN file descriptor (cached from tun) */
	int fd;
	/** TUN device */
	tun_device_t *tun;
} tun_entry_t;

/**
 * Single instance of the router
 */
kernel_libipsec_router_t *router;

/**
 * Private data
 */
struct private_kernel_libipsec_router_t {

	/**
	 * Public interface
	 */
	kernel_libipsec_router_t public;

	/**
	 * Default TUN device if kernel interface does not require separate TUN
	 * devices per VIP or for tunnels without VIP.
	 */
	tun_entry_t tun;

	/**
	 * Hashtable that maps virtual IPs to TUN devices (tun_entry_t).
	 */
	hashtable_t *tuns;

	/**
	 * Lock for TUN device map
	 */
	rwlock_t *lock;

	/**
	 * Pipe to signal handle_plain() about changes regarding TUN devices
	 */
	int notify[2];
};

/**
 * Hash function for TUN device map
 */
static u_int tun_entry_hash(tun_entry_t *entry)
{
	return chunk_hash(entry->addr->get_address(entry->addr));
}

/**
 * Comparison function for TUN device map
 */
static bool tun_entry_equals(tun_entry_t *a, tun_entry_t *b)
{
	return a->addr->ip_equals(a->addr, b->addr);
}

/**
 * Outbound callback
 */
static void send_esp(void *data, esp_packet_t *packet)
{
	charon->sender->send_no_marker(charon->sender, (packet_t*)packet);
}

/**
 * Receiver callback
 */
static void receiver_esp_cb(void *data, packet_t *packet)
{
	ipsec->processor->queue_inbound(ipsec->processor,
									esp_packet_create_from_packet(packet));
}

/**
 * Inbound callback
 */
static void deliver_plain(private_kernel_libipsec_router_t *this,
						  ip_packet_t *packet)
{
	tun_device_t *tun;
	tun_entry_t *entry, lookup = {
		.addr = packet->get_destination(packet),
	};

	this->lock->read_lock(this->lock);
	entry = this->tuns->get(this->tuns, &lookup);
	tun = entry ? entry->tun : this->tun.tun;
	tun->write_packet(tun, packet->get_encoding(packet));
	this->lock->unlock(this->lock);
	packet->destroy(packet);
}

/**
 * Create an FD set covering all TUN devices and the read end of the notify pipe
 */
static int collect_fds(private_kernel_libipsec_router_t *this, fd_set *fds)
{
	enumerator_t *enumerator;
	tun_entry_t *entry;
	int maxfd;

	FD_ZERO(fds);
	FD_SET(this->notify[0], fds);
	maxfd = this->notify[0];

	FD_SET(this->tun.fd, fds);
	maxfd = max(maxfd, this->tun.fd);

	this->lock->read_lock(this->lock);
	enumerator = this->tuns->create_enumerator(this->tuns);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		FD_SET(entry->fd, fds);
		maxfd = max(maxfd, entry->fd);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return maxfd + 1;
}

/**
 * Read and process outbound plaintext packet for the given TUN device
 */
static void process_plain(tun_device_t *tun)
{
	chunk_t raw;

	if (tun->read_packet(tun, &raw))
	{
		ip_packet_t *packet;

		packet = ip_packet_create(raw);
		if (packet)
		{
			ipsec->processor->queue_outbound(ipsec->processor, packet);
		}
		else
		{
			DBG1(DBG_KNL, "invalid IP packet read from TUN device");
		}
	}
}

/**
 * Handle waiting data for any TUN device
 */
static void handle_tuns(private_kernel_libipsec_router_t *this, fd_set *fds)
{
	enumerator_t *enumerator;
	tun_entry_t *entry;

	if (FD_ISSET(this->tun.fd, fds))
	{
		process_plain(this->tun.tun);
	}

	this->lock->read_lock(this->lock);
	enumerator = this->tuns->create_enumerator(this->tuns);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		if (FD_ISSET(entry->fd, fds))
		{
			process_plain(entry->tun);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

/**
 * Job handling outbound plaintext packets
 */
static job_requeue_t handle_plain(private_kernel_libipsec_router_t *this)
{
	bool oldstate;
	fd_set fds;
	int maxfd;

	maxfd = collect_fds(this, &fds);

	oldstate = thread_cancelability(TRUE);
	if (select(maxfd, &fds, NULL, NULL, NULL) <= 0)
	{
		thread_cancelability(oldstate);
		return JOB_REQUEUE_FAIR;
	}
	thread_cancelability(oldstate);

	if (FD_ISSET(this->notify[0], &fds))
	{	/* list of TUN devices changed, read notification data, rebuild FDs */
		char buf[1];
		while (read(this->notify[0], &buf, sizeof(buf)) == sizeof(buf));
		return JOB_REQUEUE_DIRECT;
	}

	handle_tuns(this, &fds);
	return JOB_REQUEUE_DIRECT;
}

METHOD(kernel_listener_t, tun, bool,
	private_kernel_libipsec_router_t *this, tun_device_t *tun, bool created)
{
	tun_entry_t *entry, lookup;
	char buf[] = {0x01};

	this->lock->write_lock(this->lock);
	if (created)
	{
		INIT(entry,
			.addr = tun->get_address(tun, NULL),
			.fd = tun->get_fd(tun),
			.tun = tun,
		);
		this->tuns->put(this->tuns, entry, entry);
	}
	else
	{
		lookup.addr = tun->get_address(tun, NULL);
		entry = this->tuns->remove(this->tuns, &lookup);
		free(entry);
	}
	/* notify handler thread to recreate FD set */
	ignore_result(write(this->notify[1], buf, sizeof(buf)));
	this->lock->unlock(this->lock);
	return TRUE;
}

METHOD(kernel_libipsec_router_t, get_tun_name, char*,
	private_kernel_libipsec_router_t *this, host_t *vip)
{
	tun_entry_t *entry, lookup = {
		.addr = vip,
	};
	tun_device_t *tun;
	char *name;

	if (!vip)
	{
		return strdup(this->tun.tun->get_name(this->tun.tun));
	}
	this->lock->read_lock(this->lock);
	entry = this->tuns->get(this->tuns, &lookup);
	tun = entry ? entry->tun : this->tun.tun;
	name = strdup(tun->get_name(tun));
	this->lock->unlock(this->lock);
	return name;
}

METHOD(kernel_libipsec_router_t, destroy, void,
	private_kernel_libipsec_router_t *this)
{
	charon->receiver->del_esp_cb(charon->receiver,
								(receiver_esp_cb_t)receiver_esp_cb);
	ipsec->processor->unregister_outbound(ipsec->processor,
										 (ipsec_outbound_cb_t)send_esp);
	ipsec->processor->unregister_inbound(ipsec->processor,
										 (ipsec_inbound_cb_t)deliver_plain);
	hydra->kernel_interface->remove_listener(hydra->kernel_interface,
											 &this->public.listener);
	this->lock->destroy(this->lock);
	this->tuns->destroy(this->tuns);
	close(this->notify[0]);
	close(this->notify[1]);
	router = NULL;
	free(this);
}

/**
 * Set O_NONBLOCK on the given socket.
 */
static bool set_nonblock(int socket)
{
	int flags = fcntl(socket, F_GETFL);
	return flags != -1 && fcntl(socket, F_SETFL, flags | O_NONBLOCK) != -1;
}

/*
 * See header file
 */
kernel_libipsec_router_t *kernel_libipsec_router_create()
{
	private_kernel_libipsec_router_t *this;

	INIT(this,
		.public = {
			.listener = {
				.tun = _tun,
			},
			.get_tun_name = _get_tun_name,
			.destroy = _destroy,
		},
		.tun = {
			.tun = lib->get(lib, "kernel-libipsec-tun"),
		}
	);

	if (pipe(this->notify) != 0 ||
		!set_nonblock(this->notify[0]) || !set_nonblock(this->notify[1]))
	{
		DBG1(DBG_KNL, "creating notify pipe for kernel-libipsec router failed");
		free(this);
		return NULL;
	}

	this->tun.fd = this->tun.tun->get_fd(this->tun.tun);

	this->tuns = hashtable_create((hashtable_hash_t)tun_entry_hash,
								  (hashtable_equals_t)tun_entry_equals, 4);
	this->lock = rwlock_create(RWLOCK_TYPE_DEFAULT);

	hydra->kernel_interface->add_listener(hydra->kernel_interface,
										  &this->public.listener);
	ipsec->processor->register_outbound(ipsec->processor, send_esp, NULL);
	ipsec->processor->register_inbound(ipsec->processor,
									(ipsec_inbound_cb_t)deliver_plain, this);
	charon->receiver->add_esp_cb(charon->receiver,
									(receiver_esp_cb_t)receiver_esp_cb, NULL);
	lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create((callback_job_cb_t)handle_plain, this,
									NULL, (callback_job_cancel_t)return_false));

	router = &this->public;
	return &this->public;
}
