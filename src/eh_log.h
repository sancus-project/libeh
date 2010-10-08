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

#include <stdio.h>	/* fprintf() */
#include <string.h>	/* strerror() */
#include <errno.h>	/* errno */

#define _err(S)		fputs(S "\n", stderr)
#define _errf(F, ...)	fprintf(stderr, F "\n", __VA_ARGS__)

#define info(S)		_err("I: " S)
#define warn(S)		_err("W: " S)
#define err(S)		_err("E: " S)

#define infof(F, ...)	_errf("I: " F, __VA_ARGS__)
#define warnf(F, ...)	_errf("W: " F, __VA_ARGS__)
#define errf(F, ...)	_errf("E: " F, __VA_ARGS__)

#define syserr(S)	_errf("E: " S ": %s", strerror(errno))
#define syserrf(F, ...)	_errf("E: " F ": %s", __VA_ARGS__, strerror(errno))

#ifndef NDEBUG
#define debug(S)	_errf("D: %s:%u: %s: %s", __FILE__, __LINE__, __func__, S)
#define debugf(F, ...)	_errf("D: %s:%u: %s: " F, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define debug(S)	((void)0)
#define debugf(...)	((void)0)
#endif

#endif
