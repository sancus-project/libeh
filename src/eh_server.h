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
#ifndef _EH_SERVER_H
#define _EH_SERVER_H

enum eh_server_error {
	EH_SERVER_ACCEPT_ERROR,
	EH_SERVER_WATCHER_ERROR,
};

struct eh_server {
	ev_io connection_watcher;

	struct eh_connection *(*on_connect) (struct eh_server *, int fd,
					     struct sockaddr *, socklen_t);
	void (*on_stop) (struct eh_server *, struct ev_loop *);

	void (*on_error) (struct eh_server *, struct ev_loop *, enum eh_server_error);
};

static inline int eh_server_fd(struct eh_server *self)
{
	return self->connection_watcher.fd;
}

/*
 * ALERT: eh_server_ipv4_tcp() does NOT initialize the callbacks, only the socket
 *
 * Returns: 1:ok, 0:bad address, -1:errno
 */
int eh_server_ipv4_tcp(struct eh_server *self, const char *addr, unsigned port, bool cloexec);

int eh_server_listen(struct eh_server *self, unsigned backlog);
void eh_server_finish(struct eh_server *self);

void eh_server_start(struct eh_server *self, struct ev_loop *loop);
void eh_server_stop(struct eh_server *self, struct ev_loop *loop);

#endif /* !_EH_SERVER_H */
