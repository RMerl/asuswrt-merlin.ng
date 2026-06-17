/* $Id: asyncsendto.h,v 1.4 2025/04/03 21:11:34 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2025 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef ASYNCSENDTO_H_INCLUDED
#define ASYNCSENDTO_H_INCLUDED
/*! \file asyncsendto.h
 * \brief queue packets if they are not sent immediatly
 */

/* for fd_set */
#include <sys/select.h>

/*! \brief schedule sendto() call after delay
 *
 * see sendto(2)
 * \param[in] sockfd socket file descriptor
 * \param[in] buf message data
 * \param[in] len message length (bytes)
 * \param[in] flags may include MSG_OOB, MSG_DONTROUTE, MSG_EOR, MSG_DONTWAIT, MSG_EOF, MSG_NOSIGNAL
 * \param[in] dest_addr target address
 * \param[in] addrlen target address length
 * \param[in] src_addr NULL or the IPv6 source address
 * \param[in] delay milliseconds
 * \return -1 on error, 0 if the send is scheduled, or the bytes sent immediately */
ssize_t
sendto_schedule2(int sockfd, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr, socklen_t addrlen,
                 const struct sockaddr_in6 *src_addr,
                 unsigned int delay);

#define sendto_schedule(sockfd, buf, len, flags, dest_addr, addrlen, delay) \
        sendto_schedule2(sockfd, buf, len, flags, dest_addr, addrlen, NULL, delay)

/*! \brief try sendto() at once and schedule if EINTR/EAGAIN/EWOULDBLOCK
 *
 * see sendto(2)
 * \param[in] sockfd socket file descriptor
 * \param[in] buf message data
 * \param[in] len message length (bytes)
 * \param[in] flags may include MSG_OOB, MSG_DONTROUTE, MSG_EOR, MSG_DONTWAIT, MSG_EOF, MSG_NOSIGNAL
 * \param[in] dest_addr target address
 * \param[in] addrlen target address length
 * \return -1 on error, 0 if the send is scheduled, or the bytes sent immediately */
ssize_t
sendto_or_schedule(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen);

/*! \brief try sendto() at once and schedule if EINTR/EAGAIN/EWOULDBLOCK
 *
 * same as sendto_schedule() except it will try to set source address.
 * see sendto(2)
 * \param[in] sockfd socket file descriptor
 * \param[in] buf message data
 * \param[in] len message length (bytes)
 * \param[in] flags may include MSG_OOB, MSG_DONTROUTE, MSG_EOR, MSG_DONTWAIT, MSG_EOF, MSG_NOSIGNAL
 * \param[in] dest_addr target address
 * \param[in] addrlen target address length
 * \param[in] src_addr NULL or the IPv6 source address
 * \return -1 on error, 0 if the send is scheduled, or the bytes sent immediately */
ssize_t
sendto_or_schedule2(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen,
                   const struct sockaddr_in6 *src_addr);

/*! \brief retrieve timestamp to next scheduled packet
 * \param[out] next_send timestamp to next scheduled packet
 * \return number of scheduled sendto or -1 on error */
int get_next_scheduled_send(struct timeval * next_send);

/*! \brief execute sendto() for needed packets
 * \param[in] writefds from select()
 * \return 0 on success, a negative value for the number of errors */
int try_sendto(fd_set * writefds);

/*! \brief update writefds for select() call
 * \param[out] writefds set with relevant values
 * \param[out] max_fd increased if needed
 * \param[in] now
 * \return number of packets to try to send now */
int get_sendto_fds(fd_set * writefds, int * max_fd, const struct timeval * now);

/*! \brief empty the list */
void finalize_sendto(void);

#endif
