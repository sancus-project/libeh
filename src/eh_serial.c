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
#include "eh.h"
#include "eh_serial.h"
#include "eh_fd.h"
#include "eh_watcher.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>	/* strlen(), memcpy() */

#include <assert.h>

int eh_serial_open(struct eh_serial *self, const char *devname,
			   bool cloexec, struct termios *copy)
{
	assert(self != NULL);
	assert(devname != NULL);

	if ((self->fd = eh_open(devname, O_RDWR|O_NOCTTY|O_NONBLOCK, cloexec, 0)) < 0)
		goto init_done;
	else if (tcgetattr(self->fd, &self->oldtio) < 0)
		goto init_fail;
	else if (copy != NULL)
		memcpy(copy, &self->oldtio, sizeof(*copy));
	goto init_done;

init_fail:
	close(self->fd);
	self->fd = -1;
init_done:
	return self->fd;
}

int eh_serial_apply(struct eh_serial *self, struct termios *tio)
{
	struct termios newtio;

	assert(self != NULL);
	assert(tio != NULL);
	assert(self->fd >= 0);

	/* pre-initialize in case tcgetattr() doesn't really touch all bytes */
	memcpy(&newtio, tio, sizeof(newtio));

	/* TERMIOS(3):
	 * Note that tcsetattr() returns success if any of the requested changes could be successfully
	 * carried out.  Therefore, when making multiple changes it may be necessary to follow this call
	 * with a further call to tcgetattr() to check that all changes have been performed successfully.
	 */
	if ((tcsetattr(self->fd, TCSANOW, tio)) < 0)
		;
	else if (tcgetattr(self->fd, &newtio) < 0)
		;
	else if (memcmp(tio, &newtio, sizeof(newtio)) != 0)
		;
	else
		return 0; /* happy */

	return -1;
}

int eh_serial_close(struct eh_serial *self)
{
	int ret, orig_errno;
	assert(self != NULL);
	assert(self->fd >= 0);

	ret = eh_serial_apply(self, &self->oldtio);
	orig_errno = errno;

	eh_close(&self->fd);

	errno = orig_errno;
	return ret;
}
