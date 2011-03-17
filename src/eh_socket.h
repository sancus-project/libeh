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
#ifndef _EH_SOCKET_H
#define _EH_SOCKET_H

static inline int eh_socket(int family, int type, bool cloexec, bool nonblock)
{
	int fd;
#ifdef SOCK_CLOEXEC
	if (cloexec)
		type |= SOCK_CLOEXEC;
	else
		type &= ~SOCK_CLOEXEC;
#endif
#ifdef SOCK_NONBLOCK
	if (nonblock)
		type |= SOCK_NONBLOCK;
	else
		type &= ~SOCK_NONBLOCK;
#endif
	if ((fd = socket(family, type, 0)) < 0)
		goto socket_done;

	if (cloexec || nonblock) {
		int fl2, fl = fcntl(fd, F_GETFL);
		if (fl < 0)
			goto socket_failed;
		fl2 = fl;
		if (cloexec)
			fl2 |= FD_CLOEXEC;
		if (nonblock)
			fl2 |= O_NONBLOCK;

		if (fl != fl2 && fcntl(fd, F_SETFL, fl2) < 0)
			goto socket_failed;
		goto socket_done;
	}

socket_failed:
	close(fd);
	fd = -1;
socket_done:
	return fd;
}

ssize_t eh_socket_ntop(char *dst, size_t dst_len, const struct sockaddr *sa, socklen_t sa_len);

#endif /* !_EH_SOCKET_H */
