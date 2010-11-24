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
#include "eh_watcher.h"
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
	struct eh_connection_cb *cb = self->cb;

	assert(self != NULL);
	assert(self->cb != NULL);
	assert(self->loop == loop);

	if (revents & EV_READ) {
		struct eh_buffer *buf = &self->read_buffer;
		ssize_t l;

		if (eh_buffer_free(buf) == 0) {
			bool close = true;
			if (cb->on_error)
				close = cb->on_error(self, EH_CONNECTION_READ_FULL);
			if (close)
				goto terminate;
		}

try_read:
		l = eh_buffer_read(buf, w->fd);

		if (l == 0) { /* EOF */
			goto terminate;
		} else if (l > 0) { /* has new data, pass over */
			ssize_t rc = eh_buffer_len(buf);
			if (cb->on_read)
				rc = cb->on_read(self, eh_buffer_data(buf), rc);

			if (rc < 0)
				goto terminate;
			else
				eh_buffer_skip(buf, rc);

		} else if (errno == EINTR) {
			goto try_read;
		} else if (errno != EAGAIN && errno != EWOULDBLOCK) {
			bool close = true;
			if (cb->on_error)
				close = cb->on_error(self, EH_CONNECTION_READ_ERROR);
			if (close)
				goto terminate;
		}
	}

	if (revents & EV_ERROR) {
		bool close = true;
		if (cb->on_error)
			close = cb->on_error(self, EH_CONNECTION_READ_WATCHER_ERROR);
		if (close)
			goto terminate;
	}

	return;
terminate:
	eh_connection_stop(self);
	eh_connection_finish(self);
}

static void write_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	struct eh_connection *self = w->data;
	struct eh_connection_cb *cb = self->cb;

	assert(self->cb != NULL);

	if (revents & EV_WRITE) {
		ssize_t wc;
		if (eh_buffer_len(&self->write_buffer) == 0)
			goto stop_it;

try_write:
		wc = eh_buffer_write(&self->write_buffer, w->fd);
		if (wc < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			bool close = true;
			if (errno == EINTR)
				goto try_write;

			if (cb->on_error)
				close = cb->on_error(self, EH_CONNECTION_WRITE_ERROR);
			if (close)
				goto terminate;
		}

		if (eh_buffer_len(&self->write_buffer) == 0) {
stop_it:
			ev_io_stop(loop, w);
		}
	}
	if (revents & EV_ERROR) {
		bool close = true;
		if (cb->on_error)
			close = cb->on_error(self, EH_CONNECTION_WRITE_WATCHER_ERROR);
		if (close)
			goto terminate;
	}

	return;
terminate:
	eh_connection_stop(self);
	eh_connection_finish(self);
}

/** Send some data to the peer
 *
 * For now it only appends to the buffer and starts the write watcher
 */
ssize_t eh_connection_write(struct eh_connection *self, const uint8_t *data, size_t len)
{
	struct eh_buffer *buffer = &self->write_buffer;
	struct eh_connection_cb *cb = self->cb;

	assert(self->cb != NULL);
	assert(data != NULL || len == 0);

	if (len == 0)
		return 0;

try_append:
	if (eh_buffer_append(buffer, data, len) < 0) {
		bool close = true;
		if (cb->on_error)
			close = cb->on_error(self, EH_CONNECTION_WRITE_FULL);

		if (!close)
			goto try_append;
		else
			return -1;
	}

	if (!eh_io_active(&self->write_watcher))
		ev_io_start(self->loop, &self->write_watcher);

	return len;
}

/* exported */
int eh_connection_init(struct eh_connection *self, int fd,
		       uint8_t *read_buf, size_t read_buf_size,
		       uint8_t *write_buf, size_t write_buf_size)
{
	assert(fd >= 0);

	eh_buffer_init(&self->read_buffer, read_buf, read_buf_size);
	eh_buffer_init(&self->write_buffer, write_buf, write_buf_size);

	eh_io_init(&self->read_watcher, read_callback, self, fd, EH_READ);
	eh_io_init(&self->write_watcher, write_callback, self, fd, EH_WRITE);
	return 1;
}

void eh_connection_finish(struct eh_connection *self)
{
	struct eh_connection_cb *cb = self->cb;

	assert(self->cb != NULL);
	assert(!eh_io_active(&self->read_watcher));
	assert(!eh_io_active(&self->write_watcher));

	close(self->read_watcher.fd);

	/* on_close() is mandatory, you need to release the connection somehow */
	assert(cb->on_close);
	cb->on_close(self);
}

void eh_connection_start(struct eh_connection *self, struct ev_loop *loop)
{
	assert(loop != NULL || self->loop != NULL);

	if (loop)
		self->loop = loop;
	else
		loop = self->loop;

	if (!eh_io_active(&self->read_watcher))
		ev_io_start(loop, &self->read_watcher);

	if (eh_buffer_len(&self->write_buffer) > 0 && !eh_io_active(&self->write_watcher))
		ev_io_start(loop, &self->write_watcher);
}

void eh_connection_stop(struct eh_connection *self)
{
	assert(self->loop != NULL);

	if (eh_io_active(&self->read_watcher))
		ev_io_stop(self->loop, &self->read_watcher);

	if (eh_io_active(&self->write_watcher))
		ev_io_stop(self->loop, &self->write_watcher);
}
