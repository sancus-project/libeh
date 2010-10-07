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
#include "eh_connection.h"
#include "eh_io.h"
#include "eh.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define READ_BUF_SIZE	4096

#include <unistd.h>
#include <errno.h>
#include <assert.h>

/* callbacks */
static void read_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	struct eh_connection *self = w->data;
	struct eh_connection_cb *cb;

	assert(self != NULL);
	assert(self->cb != NULL);
	cb = self->cb;

	if (revents & EV_READ) {
		unsigned char buf[READ_BUF_SIZE];
		ssize_t l = read(w->fd, buf, sizeof(*buf));

		if (l > 0 && cb->on_read) {
			cb->on_read(self, buf, l);
		} else if (l < 0 && cb->on_error &&
			   errno != EAGAIN && errno != EINTR && errno != EWOULDBLOCK) {
			cb->on_error(self, loop, EH_CONNECTION_READ_ERROR);
		}
	}

	if ((revents & EV_ERROR) && cb->on_error)
		cb->on_error(self, loop, EH_CONNECTION_READ_WATCHER_ERROR);
}

static void write_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	struct eh_connection *self = w->data;
	struct eh_connection_cb *cb;

	assert(self != NULL);
	assert(self->cb != NULL);
	cb = self->cb;

	if (revents & EV_WRITE) {
		/* anything to write? */
		ev_io_stop(loop, w);
	}
	if (revents & EV_ERROR && cb->on_error)
		cb->on_error(self, loop, EH_CONNECTION_WRITE_WATCHER_ERROR);
}

/* exported */
int eh_connection_init(struct eh_connection *self, int fd)
{
	eh_io_init(&self->read_watcher, read_callback, self, fd, EH_READ);
	eh_io_init(&self->write_watcher, write_callback, self, fd, EH_WRITE);
	return 1;
}

void eh_connection_finish(struct eh_connection *UNUSED(self))
{
}

void eh_connection_start(struct eh_connection *self, struct ev_loop *loop)
{
	ev_io_start(loop, &self->read_watcher);
	/* TODO: if anything to write, start the write_watcher too */
}

void eh_connection_stop(struct eh_connection *self, struct ev_loop *loop)
{
	ev_io_stop(loop, &self->read_watcher);
	ev_io_stop(loop, &self->write_watcher);
}
