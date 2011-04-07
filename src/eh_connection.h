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
#ifndef _EH_CONNECTION_H
#define _EH_CONNECTION_H

#include <ev.h>
#include <stdbool.h>

#include <eh_buffer.h>

enum eh_connection_error {
	EH_CONNECTION_READ_ERROR,
	EH_CONNECTION_WRITE_ERROR,
	EH_CONNECTION_READ_FULL,
	EH_CONNECTION_WRITE_FULL,
	EH_CONNECTION_READ_WATCHER_ERROR,
	EH_CONNECTION_WRITE_WATCHER_ERROR,
};

struct eh_connection;

struct eh_connection_cb {
	ssize_t (*on_read) (struct eh_connection *, char *, size_t);
	void (*on_close) (struct eh_connection *);

	bool (*on_error) (struct eh_connection *, enum eh_connection_error);
};

struct eh_connection {
	ev_io read_watcher;
	ev_io write_watcher;

	struct ev_loop *loop;

	struct eh_buffer read_buffer;
	struct eh_buffer write_buffer;

	struct eh_connection_cb *cb;
};

static inline int eh_connection_fd(struct eh_connection *self)
{
	return self->read_watcher.fd;
}
static inline void eh_connection_reset_readbuffer(struct eh_connection *self)
{
	eh_buffer_reset(&self->read_buffer);
}

int eh_connection_init(struct eh_connection *self, int fd,
		       struct eh_connection_cb *cb,
		       char *read_buf, size_t read_buf_size,
		       char *write_buf, size_t write_buf_size);
void eh_connection_finish(struct eh_connection *self);

void eh_connection_start(struct eh_connection *self, struct ev_loop *loop);
void eh_connection_stop(struct eh_connection *self);

ssize_t eh_connection_write(struct eh_connection *self, const char *buffer,
			    size_t len);

#endif /* !_EH_CONNECTION_H */
