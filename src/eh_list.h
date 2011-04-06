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
#ifndef _EH_LIST_H
#define _EH_LIST_H

/** Double linked list
 */
struct eh_list {
	struct eh_list *prev; /**< previous element on the list */
	struct eh_list *next; /**< next element on the list */
};

/** Initializes a list element */
static inline void eh_list_init(struct eh_list *self)
{
	*self = (struct eh_list) { self, self };
}

/** Injects a list element between two other */
static inline void eh_list_inject(struct eh_list *self, struct eh_list *prev,
				  struct eh_list *next)
{
	self->next = next;
	self->prev = prev;

	next->prev = self;
	prev->next = self;
}

/** Insert element at the begining of a list
 *
 * @param H	List's head
 * @param N	New element
 */
#define eh_list_insert(H, N)	eh_list_inject(N, (H), (H)->next)

/** Insert element at the end of a list
 *
 * @param H	List's head
 * @param N	New element
 */
#define eh_list_append(H, N)	eh_list_inject(N, (H)->prev, (H))

static inline void __eh_list_del(struct eh_list *prev, struct eh_list *next)
{
	next->prev = prev;
	prev->next = next;
}
#define eh_list_del(S)	__eh_list_del((S)->prev, (S)->next)

#define eh_list_foreach(H, I) for(struct eh_list *I = (H)->next; (I) != (H); (I) = (I)->next)
#define eh_list_foreach2(H, I, N) for(struct eh_list *I = (H)->next, *N = (I)->next; (I) != (H); (I) = (N), (N) = (I)->next)

#endif /* !_EH_LIST_H */
