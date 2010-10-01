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
 *     without
 *     specific prior written permission.
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
#include "eh.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arpa/inet.h>  /* htons() */
#include <string.h>
#include <unistd.h>

/* 1:ok, 0:bad address, -1:errno */
static int init_ipv4(struct eh_server *self, const char *addr, unsigned port)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)&self->addr;
	int e = 1;

	memset(self, '\0', sizeof(*self));
	self->addrlen = sizeof(*sin);
	self->fd = -1;

	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);

	/* NULL, "" or "0" mean any address */
	if (addr == NULL || addr[0] == '\0' || (addr[0] == '0' && addr[1] == '\0'))
		sin->sin_addr.s_addr = htonl(INADDR_ANY);
	else
		e = inet_pton(sin->sin_family, addr, &sin->sin_addr);

	return e;
}

/* <0 error, >=0 fd */
static int init_tcp(struct eh_server *self)
{
	int fd = socket(self->addr.ss_family, SOCK_STREAM, 0);

	if (fd < 0)
		return -1;
	else {
		int flags = 1;
		struct linger ling = {0, 0}; /* disabled */

		self->fd = fd;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
	}
	return fd;
}

int eh_server_ipv4_tcp(struct eh_server *self, const char *addr, unsigned port)
{
	int e = init_ipv4(self, addr, port);
	if (e != 1)
		return e; /* 0 or -1 */

	if (init_tcp(self) < 0)
		return -1; /* socket() call failed */
	else if (bind(self->fd, (struct sockaddr *)&self->addr, self->addrlen) < 0) {
		close(self->fd);
		self->fd = -1;
		return -1; /* bind() failed */
	}

	return 1;
}

int eh_server_finish(struct eh_server *self)
{
	if (self->fd < 0) {
		close(self->fd);
		self->fd = -1;
	}
	return 0;
}
