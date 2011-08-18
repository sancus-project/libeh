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
#ifndef _EH_LOG_H
#define _EH_LOG_H

enum eh_log_level {
	EH_LOG_EMERG,
	EH_LOG_ALERT,
	EH_LOG_CRIT,
	EH_LOG_ERR,
	EH_LOG_WARNING,
	EH_LOG_NOTICE,
	EH_LOG_INFO,
	EH_LOG_TRACE,
	EH_LOG_DEBUG,
};

struct eh_logger {
	struct eh_list loggers;

	enum eh_log_level level;
	const char name[];
};

/*
 * logging subsystem
 */
void eh_log_init(enum eh_log_level level);
void eh_log_finish(void);

void eh_log_set_default_level(enum eh_log_level level);

/*
 * logger
 */
struct eh_logger *eh_logger_new(const char *name);
struct eh_logger *eh_logger_newf(const char *fmt, ...) TYPECHECK_PRINTF(1, 2);

struct eh_logger *eh_logger_get(const char *name);
struct eh_logger *eh_logger_getf(const char *fmt, ...) TYPECHECK_PRINTF(1, 2);

void eh_logger_del(struct eh_logger *);

static inline const char *eh_logger_name(struct eh_logger *self)
{
	return self ? self->name : NULL;
}
static inline int eh_logger_level(struct eh_logger *self, enum eh_log_level level)
{
	return self ? self->level >= level : 1;
}
#define eh_logger_set_level(S, L)	do { (S)->level = (L); } while(0)

/*
 * log writter
 */

typedef ssize_t (*eh_log_f) (const char *, enum eh_log_level, int,
			     const char *, size_t,
			     const char *, ssize_t);

void eh_log_set_backend(eh_log_f);

ssize_t eh_log_stderr(const char *name, enum eh_log_level level, int code,
		   const char *dump, size_t dump_len,
		   const char *str, ssize_t str_len);

eh_log_f eh_log_raw;

ssize_t eh_log_rawf(const char *name, enum eh_log_level level, int code,
		   const char *dump, size_t dump_len,
		   const char *fmt, ...) TYPECHECK_PRINTF(6, 7);

/*
 * log filters
 */
#define eh_log2(S, L, C, D, DL, M, ML)	do { \
	if (eh_logger_level(S, L)) \
		eh_log_raw(eh_logger_name(S), L, C, D, DL, M, ML); \
	} while(0)

#define eh_log2f(S, L, C, D, DL, F, ...)	do { \
	if (eh_logger_level(S, L)) \
		eh_log_rawf(eh_logger_name(S), L, C, D, DL, F, __VA_ARGS__); \
	} while(0)

#define eh_log(S, L, C, M, ML)		eh_log2(S, L, C, NULL, 0, M, ML)
#define eh_logf(S, L, C, F, ...)	eh_log2f(S, L, C, NULL, 0, F, __VA_ARGS__)

/*
 * helper wrappers
 */
#define eh_log_emerg2(S, ...)	eh_log2(S,  EH_LOG_EMERG, __VA_ARGS__, -1)
#define eh_log_emerg2f(S, ...)	eh_log2f(S, EH_LOG_EMERG, __VA_ARGS__)
#define eh_log_emerg(S, ...)	eh_log(S,  EH_LOG_EMERG, __VA_ARGS__, -1)
#define eh_log_emergf(S, ...)	eh_logf(S, EH_LOG_EMERG, __VA_ARGS__)

#define eh_log_alert2(S, ...)	eh_log2(S,  EH_LOG_ALERT, __VA_ARGS__, -1)
#define eh_log_alert2f(S, ...)	eh_log2f(S, EH_LOG_ALERT, __VA_ARGS__)
#define eh_log_alert(S, ...)	eh_log(S,  EH_LOG_ALERT, __VA_ARGS__, -1)
#define eh_log_alertf(S, ...)	eh_logf(S, EH_LOG_ALERT, __VA_ARGS__)

#define eh_log_crit2(S, ...)	eh_log2(S,  EH_LOG_CRIT, __VA_ARGS__, -1)
#define eh_log_crit2f(S, ...)	eh_log2f(S, EH_LOG_CRIT, __VA_ARGS__)
#define eh_log_crit(S, ...)	eh_log(S,  EH_LOG_CRIT, __VA_ARGS__, -1)
#define eh_log_critf(S, ...)	eh_logf(S, EH_LOG_CRIT, __VA_ARGS__)

#define eh_log_err2(S, ...)	eh_log2(S,  EH_LOG_ERR, __VA_ARGS__, -1)
#define eh_log_err2f(S, ...)	eh_log2f(S, EH_LOG_ERR, __VA_ARGS__)
#define eh_log_err(S, ...)	eh_log(S,  EH_LOG_ERR, __VA_ARGS__, -1)
#define eh_log_errf(S, ...)	eh_logf(S, EH_LOG_ERR, __VA_ARGS__)

#define eh_log_warn2(S, ...)	eh_log2(S,  EH_LOG_WARNING, __VA_ARGS__, -1)
#define eh_log_warn2f(S, ...)	eh_log2f(S, EH_LOG_WARNING, __VA_ARGS__)
#define eh_log_warn(S, ...)	eh_log(S,  EH_LOG_WARNING, __VA_ARGS__, -1)
#define eh_log_warnf(S, ...)	eh_logf(S, EH_LOG_WARNING, __VA_ARGS__)

#define eh_log_notice2(S, ...)	eh_log2(S,  EH_LOG_NOTICE, __VA_ARGS__, -1)
#define eh_log_notice2f(S, ...)	eh_log2f(S, EH_LOG_NOTICE, __VA_ARGS__)
#define eh_log_notice(S, ...)	eh_log(S,  EH_LOG_NOTICE, __VA_ARGS__, -1)
#define eh_log_noticef(S, ...)	eh_logf(S, EH_LOG_NOTICE, __VA_ARGS__)

#define eh_log_info2(S, ...)	eh_log2(S,  EH_LOG_INFO, __VA_ARGS__, -1)
#define eh_log_info2f(S, ...)	eh_log2f(S, EH_LOG_INFO, __VA_ARGS__)
#define eh_log_info(S, ...)	eh_log(S,  EH_LOG_INFO, __VA_ARGS__, -1)
#define eh_log_infof(S, ...)	eh_logf(S, EH_LOG_INFO, __VA_ARGS__)

#define eh_log_syserr2(S, C, D, DL, M) \
	eh_log_err2f(S, C, D, DL, M ": %s", strerror(errno))
#define eh_log_syserr(S, C, M) \
	eh_log_errf(S, C, M ": %s", strerror(errno))

#define eh_log_syserr2f(S, C, D, DL, F, ...) \
	eh_log_err2f(S, C, D, DL, F ": %s", __VA_ARGS__, strerror(errno))
#define eh_log_syserrf(S, C, F, ...) \
	eh_log_errf(S, C, F ": %s", __VA_ARGS__, strerror(errno))

#ifndef NDEBUG
#define _eh_log_debug(S, L, C, D, DL, M) \
	eh_log2(S, L, C, D, DL, "%s:%u: %s: " M, __FILE__, __LINE__, __func__)
#define _eh_log_debugf(S, L, C, D, DL, F, ...) \
	eh_log2f(S, L, C, D, DL, "%s:%u: %s: " F, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define eh_log_trace2(S, C, D, DL, M)		_eh_log_debug(S,  EH_LOG_TRACE, C, D, DL, M)
#define eh_log_trace2f(S, C, D, DL, F, ...)	_eh_log_debugf(S, EH_LOG_TRACE, C, D, DL, F, __VA_ARGS__)
#define eh_log_trace(S, C, M)			_eh_log_debug(S,  EH_LOG_TRACE, C, NULL, 0, M)
#define eh_log_tracef(S, C, F, ...)		_eh_log_debugf(S, EH_LOG_TRACE, C, NULL, 0, F, __VA_ARGS__)
#define eh_log_debug2(S, C, D, DL, M)		_eh_log_debug(S,  EH_LOG_DEBUG, C, D, DL, M)
#define eh_log_debug2f(S, C, D, DL, F, ...)	_eh_log_debugf(S, EH_LOG_DEBUG, C, D, DL, F, __VA_ARGS__)
#define eh_log_debug(S, C, M)			_eh_log_debug(S,  EH_LOG_DEBUG, C, NULL, 0, M)
#define eh_log_debugf(S, C, F, ...)		_eh_log_debugf(S, EH_LOG_DEBUG, C, NULL, 0, F, __VA_ARGS__)

#else
#define eh_log_trace2(...)
#define eh_log_trace2f(...)
#define eh_log_trace(...)
#define eh_log_tracef(...)
#define eh_log_debug2(...)
#define eh_log_debug2f(...)
#define eh_log_debug(...)
#define eh_log_debugf(...)
#endif

/*
 * legacy API
 */
#define info(S)		eh_log_info(NULL, -1, S)
#define warn(S)		eh_log_warn(NULL, -1, S)
#define err(S)		eh_log_err(NULL, -1, S)

#define infof(F, ...)	eh_log_infof(NULL, -1, F, __VA_ARGS__)
#define warnf(F, ...)	eh_log_warnf(NULL, -1, F, __VA_ARGS__)
#define errf(F, ...)	eh_log_errf(NULL, -1, F, __VA_ARGS__)

#define syserr(S)	eh_log_syserr(NULL, -1, S)
#define syserrf(F, ...)	eh_log_syserrf(NULL, -1, F, __VA_ARGS__)

#define debug(S)	eh_log_debug(NULL, -1, S)
#define debugf(F, ...)	eh_log_debugf(NULL, -1, F, __VA_ARGS__)

#endif
