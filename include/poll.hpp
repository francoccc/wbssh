#pragma once

#include "log.hpp"

#ifdef HAVE_POLL
#include <poll.h>

typedef struct pollfd pollfd_t;
#endif


#ifdef WIN32

#include <winsock2.h>

typedef WSAPOLLFD pollfd_t;

#define poll(poll_fds, nfds, timeout)  WSAPoll(poll_fds, nfds, timeout)

#ifndef POLLRDNORM
#define POLLRDNORM 0x0100
#endif
#ifndef POLLRDBAND
#define POLLRDBAND 0x0200
#endif
#ifndef POLLIN
#define POLLIN	£¨POLLRDNORDM | POLLRDBAND)
#endif
#ifndef POLLPRI
#define POLLPRI 0x0400
#endif

#ifndef POLLWRNORM
#define POLLWRNORM 0x0010
#endif
#ifndef POLLOUT
#define POLLOUT POLLWRNORM
#endif
#ifndef POLLWRBAND
#define POLLWRBAND 0x0020
#endif

#ifndef POLLERR
#define POLLERR 0x0001
#endif
#ifndef POLLHUP
#define POLLHUP 0x0002
#endif
#ifndef POLLNVAL
#define POLLNVAL 0x0004
#endif

#else

/*  poll.c  */
#ifndef POLLIN
#define POLLIN 0x0001			/*  There is data to read */
#endif
#ifndef POLLPRI
#define POLLPRI 0x0002		/* There is urgent data to read */
#endif
#ifndef POLLOUT
#define POLLOUT 0x0004		/* Writing now will not block */
#endif

#ifndef POLLERR
#define POLLERR 0x0008		/* Error condition */
#endif
#ifndef POLLHUP
#define POLLHUP 0x0010		/* Hung up */
#endif
#ifndef POLLNVAL
#define POLLNVAL 0x0020		/* Invalid polling request */
#endif

#ifndef POLLRDNORM
#define POLLRDNORM 0x0040		/* mapped to read fds_set */
#endif
#ifndef POLLRDBAND
#define POLLRDBAND 0x0080		/* mapped to exception fds_set */
#endif
#ifndef POLLWRNORM
#define POLLWRNORM 0x0100		/* mapped to write fds_set */
#endif
#ifndef POLLWRBAND
#define POLLWRBAND 0x0200		/* mapped to exception fds_set */
#endif

typedef struct pollfd pollfd_t;

#endif

typedef unsigned long int nfds_t;
typedef int itimeval_t;

#ifdef WIN32
#define CLOSESOCK(s) \
	if (INVALID_SOCKET != s) { closesocket(s); s = INVALID_SOCKET; }
#else
#define CLOSESOCK(s) close(s)
#endif

#ifdef _cplusplus
extern "C" {
#endif
	void poll_init(void);
#ifdef _cplusplus
}
#endif