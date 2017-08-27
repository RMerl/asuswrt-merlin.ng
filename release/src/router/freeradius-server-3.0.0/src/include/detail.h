#ifndef DETAIL_H
#define DETAIL_H
/*
 *	detail.h	Routines to handle detail files.
 *
 * Version:	$Id$
 *
 */

RCSIDH(detail_h, "$Id$")

#ifdef __cplusplus
extern "C" {
#endif

typedef enum detail_state_t {
  STATE_UNOPENED = 0,
  STATE_UNLOCKED,
  STATE_HEADER,
  STATE_READING,
  STATE_QUEUED,
  STATE_RUNNING,
  STATE_NO_REPLY,
  STATE_REPLIED
} detail_state_t;

typedef struct listen_detail_t {
	fr_event_t	*ev;	/* has to be first entry (ugh) */
	int		delay_time;
	char		*filename;
	char		*filename_work;
	VALUE_PAIR	*vps;
	FILE		*fp;
	off_t		offset;
	detail_state_t 	state;
	time_t		timestamp;
	time_t		running;
	fr_ipaddr_t	client_ip;
	int		load_factor; /* 1..100 */
	int		signal;
	int		poll_interval;
	int		retry_interval;
	int		packets;
	int		tries;
	int		one_shot;
	int		outstanding;
	int		max_outstanding;
	int		has_rtt;
	int		srtt;
	int		rttvar;
	uint32_t	counter;
	struct timeval  last_packet;
	RADCLIENT	detail_client;
} listen_detail_t;

int detail_recv(rad_listen_t *listener);
int detail_send(rad_listen_t *listener, REQUEST *request);
void detail_free(rad_listen_t *this);
int detail_print(rad_listen_t const *this, char *buffer, size_t bufsize);
int detail_encode(UNUSED rad_listen_t *this, UNUSED REQUEST *request);
int detail_decode(UNUSED rad_listen_t *this, UNUSED REQUEST *request);
int detail_parse(CONF_SECTION *cs, rad_listen_t *this);

#ifdef __cplusplus
}
#endif

#endif /* DETAIL_H */
