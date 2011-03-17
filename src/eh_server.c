/*
 * This file is part of libeh <http://github.com/amery/libeh>
 *
 * Copyright (c) 2010, Alejandro Mery <amery@geeks.cl>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the author nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>  /* htons() */

#include "eh.h"
#include "eh_socket.h"
#include "eh_connection.h"
#include "eh_server.h"
#include "eh_watcher.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* 1:ok, 0:bad address, -1:errno */
static int init_ipv4(struct sockaddr_in *sin, const char *addr, unsigned port)
{
	int e = 1;

	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);

	/* NULL, "", "0" and "*" mean any address */
	if (addr == NULL || addr[0] == '\0' ||
	    ((addr[0] == '0' || addr[0] == '*') && addr[1] == '\0'))
		sin->sin_addr.s_addr = htonl(INADDR_ANY);
	else
		e = inet_pton(sin->sin_family, addr, &sin->sin_addr);

	return e;
}

static int init_local(struct sockaddr_un *sun, const char *path)
{
	size_t l = 0;

	if (path == NULL) {
		path = "";
	} else {
		l = strlen(path);
		if (l > sizeof(sun->sun_path)-1)
			return 0; /* too long */
	}

	sun->sun_family = AF_LOCAL;
	memcpy(sun->sun_path, path, l+1);

	return 1;
}

/* -1:error, >=0 fd */
static inline int init_tcp(int family, bool cloexec)
{
	int fd = eh_socket(family, SOCK_STREAM, cloexec, true);

	if (fd >= 0) {
		int flags = 1;
		struct linger ling = {0, 0}; /* disabled */

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
	}

	return fd;
}

static void connect_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	struct eh_server *self = w->data;

	if (revents & EV_READ) {
		struct eh_connection *conn = NULL;
		struct sockaddr_storage addr;
		socklen_t addrlen;
		int fd;

try_accept:
		addrlen = sizeof(addr);
		memset(&addr, '\0', sizeof(addr));

		fd = accept(w->fd, (struct sockaddr *)&addr, &addrlen);
		assert(fd < 0 || addr.ss_family != 0);

		if (fd >= 0) {
			if (self->on_connect)
				conn = self->on_connect(self, fd, (struct sockaddr *)&addr, addrlen);
			if (conn == NULL)
				close(fd);
			else
				eh_connection_start(conn, loop);
		} else if (errno == EINTR) {
			goto try_accept;
		} else if (self->on_error && errno != EAGAIN && errno != EWOULDBLOCK) {
			self->on_error(self, loop, EH_SERVER_ACCEPT_ERROR);
		}
	}
	if ((revents & EV_ERROR) && self->on_error)
		self->on_error(self, loop, EH_SERVER_WATCHER_ERROR);
}

/* 1:ok, 0:bad address, -1:errno */
int eh_server_ipv4_tcp(struct eh_server *self, const char *addr, unsigned port, bool cloexec)
{
	struct sockaddr_in sin;
	int fd;

	int e = init_ipv4(&sin, addr, port);
	if (e != 1)
		return e; /* 0 or -1 */

	if ((fd = init_tcp(sin.sin_family, cloexec)) < 0)
		return -1; /* socket() call failed */
	else if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		close(fd);
		return -1; /* bind() failed */
	}

	eh_io_init(&self->connection_watcher, connect_callback, self, fd, EH_READ);

	return 1;
}

int eh_server_local(struct eh_server *self, const char *path, bool cloexec)
{
	struct sockaddr_un sun;
	int fd, e;

	assert(self);
	assert(path);

	e = init_local(&sun, path);
	if (e != 1)
		return e; /* 0 or -1 */
	unlink(path);

	if ((fd = eh_socket(sun.sun_family, SOCK_STREAM, cloexec, true)) < 0)
		return -1; /* socket() call failed */
	else if (bind(fd, (struct sockaddr *)&sun, SUN_LEN(&sun)) < 0) {
		close(fd);
		return -1;
	}

	eh_io_init(&self->connection_watcher, connect_callback, self, fd, EH_READ);
	return 1;
}

int eh_server_listen(struct eh_server *self, unsigned backlog)
{
	return listen(eh_server_fd(self), backlog);
}

void eh_server_finish(struct eh_server *self)
{
	close(eh_server_fd(self));
}

void eh_server_start(struct eh_server *self, struct ev_loop *loop)
{
	ev_io_start(loop, &self->connection_watcher);
}

void eh_server_stop(struct eh_server *self, struct ev_loop *loop)
{
	ev_io_stop(loop, &self->connection_watcher);
	if (self->on_stop)
		self->on_stop(self, loop);
}
