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
#ifndef _EH_SERIAL_H
#define _EH_SERIAL_H

#include <stdbool.h>
#include <termios.h>

struct eh_serial {
	int fd;

	struct termios oldtio;
};

int eh_serial_open(struct eh_serial *, const char *, bool cloexec, struct termios *);
int eh_serial_close(struct eh_serial *);

int eh_serial_apply(struct eh_serial *, struct termios *);

/** setup a termios structure to be used as 8N1 serial port
 *
 * @param tio		termios structure to alter
 * @param baudrate	Bnnn value representing the speed of the channel. 0 for skipping.
 */
static inline void eh_serial_setup_8N1(struct termios *tio, speed_t baudrate)
{
	tio->c_cflag |= (CLOCAL|CREAD);

	tio->c_cflag &= ~(CSIZE|PARENB|CSTOPB);
	tio->c_cflag |= CS8;

	if (baudrate) {
		cfsetispeed(tio, baudrate);
		cfsetospeed(tio, baudrate);
	}
}

/** drops unread and unwritten data from a wire */
#define eh_serial_flush(S)	tcflush((S)->fd, TCIOFLUSH)

/** returns the fd of an eh_serial */
#define eh_serial_fd(S)		((S)->fd)

#endif /* !_EH_SERIAL_H */
