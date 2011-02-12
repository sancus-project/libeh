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
#ifndef _EH_BUFFER_H
#define _EH_BUFFER_H

#include <string.h>	/* memmove() */
#include <stdint.h>	/* uint8_t */
#include <stdbool.h>	/* bool */
#include <sys/types.h>	/* size_t */

struct eh_buffer {
	char *buf;

	uint_fast16_t base;
	uint_fast16_t len;
	uint_fast16_t size;
};

ssize_t eh_buffer_init(struct eh_buffer *self, char *buf, size_t size);

ssize_t eh_buffer_read(struct eh_buffer *self, int fd, bool *eof);
ssize_t eh_buffer_write(struct eh_buffer *self, int fd);

#define eh_buffer_data(B)	((B)->buf + (B)->base)
#define eh_buffer_next(B)	((B)->buf + (B)->base + (B)->len)
#define eh_buffer_len(B)	((B)->len)
#define eh_buffer_free(B)	((B)->size - (B)->len)
#define eh_buffer_freetail(B)	((B)->size - (B)->len - (B)->base)

void eh_buffer_rebase(struct eh_buffer *self);
void eh_buffer_skip(struct eh_buffer *self, size_t bytes);

static inline void eh_buffer_reset(struct eh_buffer *self)
{
	self->base = self->len = 0;
}

ssize_t eh_buffer_append(struct eh_buffer *self, const char *data, size_t len);
ssize_t eh_buffer_appendz(struct eh_buffer *self, const char *str);

#endif
