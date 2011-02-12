/*
 * This file is part of libeh <http://github.com/amery/libeh>
 *
 * Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>
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

#include <stdlib.h>

#include "eh.h"
#include "eh_fmt.h"

#define hexa "0123456789abcdef"
#define CEC "abtnvfr"

static inline size_t eh_fmt_cstr_len(unsigned char c)
{
	if (c > 0x1f && c < 0x7f) { /* ASCII printable characters */
		switch (c) {
		case '"':
		case '\\':
			return 2;
		default:
			return 1;
		}
	} else if (c >= '\a' && c <= '\r') { /* C Character Escape Codes */
		return 2;
	} else if (c == 0) {
		return 2;
	} else { /* not printable, hexa encoded */
		return 4;
	}
}

ssize_t eh_fmt_cstr(char *buf, size_t buf_size, const char *data, size_t data_size)
{
	size_t len = 0;
	while(data_size-- > 0) {
		unsigned char c = *data++;
		size_t l = eh_fmt_cstr_len(c);

		if (unlikely(l > buf_size))
			break;

		switch (l) {
		case 1:
			*buf = c;
			break;
		case 2:
			buf[0] = '\\';
			if (c >= '\a' && c <= '\r')
				c = CEC[c - '\a'];
			else if (c == 0)
				c = '0';
			buf[1] = c;
			break;
		case 4:
			buf[0] = '\\';
			buf[1] = 'x';
			buf[2] = hexa[(c & (0x0f << 4)) >> 4];
			buf[3] = hexa[c & 0x0f];
		}

		buf_size -= l;
		buf += l;
		len += l;
	}
	return len;
}
