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
#include "eh_server.h"
#include "eh_io.h"
#include "eh.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arpa/inet.h>  /* htons() */
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* 1:ok, 0:bad address, -1:errno */
static int init_ipv4(struct sockaddr_in *sin, const char *addr, unsigned port)
{
	int e = 1;

	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);

	/* NULL, "" or "0" mean any address */
	if (addr == NULL || addr[0] == '\0' || (addr[0] == '0' && addr[1] == '\0'))
		sin->sin_addr.s_addr = htonl(INADDR_ANY);
	else
		e = inet_pton(sin->sin_family, addr, &sin->sin_addr);

	return e;
}

/* -1:error, >=0 fd */
static int init_tcp(int family)
{
	int fd = socket(family, SOCK_STREAM, 0);

	if (fd < 0)
		return -1;
	else {
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

	if ((revents & EV_READ)) {
		struct eh_connection *conn = NULL;
		struct sockaddr_storage addr;
		socklen_t addrlen;

		int fd = accept(self->fd, (struct sockaddr *)&addr, &addrlen);
		if (fd >= 0) {
			if (self->on_connect)
				conn = self->on_connect(self, fd, (struct sockaddr *)&addr, addrlen);
			if (conn == NULL)
				close(fd);
			else
				eh_connection_start(conn, loop);
		} else if (self->on_accept_error) {
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				self->on_accept_error(self, loop, errno);
		}
	}
	if (revents & EV_ERROR) {
		if (self->on_error)
			self->on_error(self, loop);
	}
}

/* 1:ok, 0:bad address, -1:errno */
int eh_server_ipv4_tcp(struct eh_server *self, const char *addr, unsigned port)
{
	struct sockaddr_in sin;

	int e = init_ipv4(&sin, addr, port);
	if (e != 1)
		return e; /* 0 or -1 */

	/* self->fd */
	if ((self->fd = init_tcp(sin.sin_family)) < 0)
		return -1; /* socket() call failed */
	else if (bind(self->fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		close(self->fd);
		self->fd = -1;
		return -1; /* bind() failed */
	}

	eh_io_init(&self->connection_watcher, connect_callback, self, self->fd, EH_READ);

	return 1;
}

int eh_server_listen(struct eh_server *self, unsigned backlog)
{
	return listen(self->fd, backlog);
}

int eh_server_finish(struct eh_server *self)
{
	if (self->fd < 0) {
		close(self->fd);
		self->fd = -1;
	}
	return 0;
}

int eh_server_start(struct eh_server *self, struct ev_loop *loop)
{
	ev_io_start(loop, &self->connection_watcher);

	return 0;
}

void eh_server_stop(struct eh_server *self, struct ev_loop *loop)
{
	ev_io_stop(loop, &self->connection_watcher);
	if (self->on_stop)
		self->on_stop(self, loop);
}
