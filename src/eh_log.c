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

#include <string.h>
#include <stddef.h>	/* offsetof() */
#include <stdarg.h>
#include <stdio.h>

#include "eh.h"
#include "eh_list.h"
#include "eh_alloc.h"

#include "eh_log.h"

/*
 * logging subsystem
 */
static struct eh_list loggers;
enum eh_log_level eh_log_default_level = EH_LOG_TRACE; /* MAX */

void eh_log_set_default_level(enum eh_log_level level)
{
	eh_log_default_level = level;
}

void eh_log_init(enum eh_log_level level)
{
	eh_list_init(&loggers);
	eh_log_default_level = level;
}

void eh_log_finish(void)
{
	eh_list_foreach2(&loggers, item, next) {
		struct eh_logger *o = container_of(item, struct eh_logger, loggers);
		eh_logger_del(o);
	}
}

/*
 * loggers allocation
 */
struct eh_logger *eh_logger_new(const char *name)
{
	size_t l = strlen(name)+1;
	struct eh_logger *new = eh_alloc(sizeof(struct eh_logger) + l);
	if (new) {
		memcpy((char *)new->name, name, l);

		new->level = eh_log_default_level;
		eh_list_append(&loggers, &new->loggers);
	}
	return new;
}

struct eh_logger *eh_logger_newf(const char *fmt, ...)
{
	char buf[128]; /* arbitrary size */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return eh_logger_new(buf);
}

void eh_logger_del(struct eh_logger *self)
{
	eh_list_del(&self->loggers);
	eh_free(self);
}
