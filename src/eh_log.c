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

#include <sys/uio.h>
#include <sys/time.h>

#include "eh.h"
#include "eh_fd.h"
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

/* preallocated loggers */
void eh_logger_init2(struct eh_logger *new, const char *name)
{
	new->name = name;
	new->level = eh_log_default_level;

	/* don't place them on the list until eh_logger_del learns to distinguish */
	eh_list_init(&new->loggers);
}

void eh_logger_init(struct eh_logger *new, char *buf, size_t buf_size, const char *name)
{
	size_t l = strlen(name)+1;

	if (l>buf_size) {
		/* truncate */
		l = buf_size-1;
		buf[l-1] = '\0';
	}

	memcpy(buf, name, l);
	eh_logger_init2(new, buf);
}

void eh_logger_initf(struct eh_logger *new, char *buf, size_t buf_size, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, buf_size, fmt, ap);
	va_end(ap);

	eh_logger_init2(new, buf);
}

/*
 * loggers allocation
 */
struct eh_logger *eh_logger_new(const char *name)
{
	size_t l = strlen(name)+1;
	struct eh_logger *new = eh_alloc(sizeof(struct eh_logger) + l);
	if (new) {
		new->name = (char *)new + sizeof(struct eh_logger);
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

struct eh_logger *eh_logger_get(const char *name)
{
	eh_list_foreach(&loggers, item) {
		struct eh_logger *o = container_of(item, struct eh_logger, loggers);
		if (strcmp(name, o->name) == 0)
			return o;
	}
	return eh_logger_new(name);
}

struct eh_logger *eh_logger_getf(const char *fmt, ...)
{
	char buf[128]; /* arbitrary size */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return eh_logger_get(buf);
}

void eh_logger_del(struct eh_logger *self)
{
	eh_list_del(&self->loggers);
	eh_free(self);
}

/*
 * log writter
 */
static int _eh_log_stderr_timestamp;
void eh_log_stderr_timestamp(int enabled)
{
	_eh_log_stderr_timestamp = (enabled != 0);
}

ssize_t eh_log_stderr(const char *name, enum eh_log_level level, int code,
		   const char *UNUSED(dump), size_t UNUSED(dump_len),
		   const char *str, ssize_t str_len)
{
	struct iovec v[6];
	struct timeval tv;

	char buf[50];
	char *p = buf;
	int l=0, l2, pl=sizeof(buf);

	/* "[%.3f] " */
	if (_eh_log_stderr_timestamp && gettimeofday(&tv, NULL) == 0) {
		l2 = snprintf(p, pl, "[%lu.%06lu] ",
			     (unsigned long)tv.tv_sec,
			     (unsigned long)tv.tv_usec);

		v[l++] = (struct iovec) { p, l2 };
		p += l2;
		pl -= l2;
	}

	/* "<?> " */
	l2 = snprintf(p, pl, "<%u> ", level);
	v[l++] = (struct iovec) { p, l2 };
	p += l2;
	pl -= l2;

	/* "name" */
	if (name) {
		v[l++] = (struct iovec) { (void*)name, (size_t)strlen(name) };

		if (code > 0) {
			/* ": code: " */
			l2 = snprintf(p, pl, ": %u: ", code);
			v[l++] = (struct iovec) { p, l2 };
			p += l2;
			pl -= l2;
		} else {
			/* ": " */
			v[l++] = (struct iovec) { ": ", 2 };
		}
	} else if (code > 0) {
		/* "code: " */
		l2 =snprintf(p, pl, ": %u: "+2, code);
		v[l++] = (struct iovec) { p, l2 };
		p += l2;
		pl -= l2;
	}

	/* "..." */
	if (str_len < 0)
		str_len = strlen(str);
	v[l++] = (struct iovec) { (void*)str, (size_t)str_len };

	/* "\n" */
	v[l++] = (struct iovec) { "\n", 1 };

	return eh_writev(2, v, l);
}

eh_log_f eh_log_raw = eh_log_stderr;

void eh_log_set_backend(eh_log_f f)
{
	eh_log_raw = f;
}

ssize_t eh_log_rawf(const char *name, enum eh_log_level level, int code,
		   const char *dump, size_t dump_len,
		   const char *fmt, ...)
{
	char buf[1024]; /* arbitrary */
	size_t l;
	va_list ap;

	va_start(ap, fmt);
	l = vsnprintf(buf, sizeof(buf), fmt, ap);
	/* TODO: validate l */
	va_end(ap);

	return eh_log_raw(name, level, code, dump, dump_len, buf, l);
}
