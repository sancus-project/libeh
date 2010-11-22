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

#include "eh_buffer.h"

#include <assert.h>
#include <unistd.h>

/* -Winline doesn't like this as static inline */
static void __eh_buffer_rebase(struct eh_buffer *self)
{
	memmove(self->buf, eh_buffer_data(self), self->len);
	self->base = 0;
}

/**
 * \brief Initializes a buffer using an externally provided chunk of memory.
 */
ssize_t eh_buffer_init(struct eh_buffer *self, uint8_t *buf, size_t size)
{
	assert(self != NULL);
	assert((size == 0 && buf == NULL) || (size > 0 && buf != NULL));
	assert(size <= UINT_LEAST16_MAX);

	*self = (struct eh_buffer) { .buf = buf, .base = 0, .len = 0, .size = size };
	return size;
}

ssize_t eh_buffer_read(struct eh_buffer *self, int fd)
{
	ssize_t l;
	assert(self != NULL);
	assert(fd >= 0);

	if (eh_buffer_free(self) == 0)
		return 0; /* full */

	/* maintainance */
	if (self->base > 0) {
		if (self->len == 0)
			eh_buffer_reset(self);
		else if (self->base + self->len == self->size)
			__eh_buffer_rebase(self);
	}

	l = read(fd, eh_buffer_next(self), eh_buffer_freetail(self));
	if (l > 0)
		self->len += l;
#if 0
	else if (l == 0)
		return EOF; /* how? */
#endif
	return l;
}

ssize_t eh_buffer_write(struct eh_buffer *self, int fd)
{
	ssize_t l;
	assert(self != NULL);
	assert(fd >= 0);

	if (eh_buffer_len(self) == 0) {
		eh_buffer_reset(self);
		return 0; /* empty */
	}

	l = write(fd, eh_buffer_data(self), eh_buffer_len(self));
	if (l == (ssize_t)eh_buffer_len(self))
		eh_buffer_reset(self);
	else if (l > 0) {
		self->len -= l;
		self->base += l;

		/* make some tail space */
		if (self->base + self->len == self->size)
			__eh_buffer_rebase(self);
	}

	return l;
}
