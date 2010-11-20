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
#ifndef _EH_WATCHER_H
#define _EH_WATCHER_H

/*
 * NOTE: the current purpose of this module is to reduce the abuse of casts
 * done within ev.h and be more friendly to gcc strict-aliasing analysis
 */

#include <ev.h>
#include <stdbool.h>

enum eh_io_event {
	EH_READ = EV_READ,
	EH_WRITE = EV_WRITE,
	EH_RW = EV_READ|EV_WRITE
};

#define eh_watcher_set_priority(W,P)	do { \
	(W)->priority = (P); \
} while(0)

#define eh_watcher_set_cb(W, CB)	do { \
	(W)->cb = (CB); \
} while(0)

#define eh_watcher_set_data(W, D)	do { \
	(W)->data = (D); \
} while(0)

#define eh_watcher_init(W,CB) do { \
	(W)->active = (W)->pending = 0; \
	eh_watcher_set_priority((W), 0); \
	eh_watcher_set_cb((W), CB); \
} while(0)

static inline void eh_io_init(ev_io *w, void (*cb) (struct ev_loop *, ev_io *, int),
			      void *data, int fd, enum eh_io_event event)
{
	eh_watcher_init(w, cb);
	ev_io_set(w, fd, event);
	eh_watcher_set_data(w, data);
}

static inline bool eh_io_active(ev_io *w)
{
	return ev_is_active(w);
}

static inline void eh_signal_init(ev_signal *w, void (*cb) (struct ev_loop *, ev_signal *, int),
			      void *data, int signo)
{
	eh_watcher_init(w, cb);
	ev_signal_set(w, signo);
	eh_watcher_set_data(w, data);
}

#define eh_signal_start(W, L)	ev_signal_start(L, W)
#define eh_signal_stop(W, L)	ev_signal_stop(L, W)

#endif /* !_EH_WATCHER_H */
