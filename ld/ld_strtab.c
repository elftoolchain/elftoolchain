/*-
 * Copyright (c) 2012 Kai Wang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "ld.h"
#include "ld_strtab.h"

ELFTC_VCSID("$Id$");

#define	_DEFAULT_STRTAB_SIZE	512

static void _resize_strtab(struct ld *ld, struct ld_strtab *st,
    size_t newsize);

struct ld_strtab *
ld_strtab_alloc(struct ld *ld)
{
	struct ld_strtab *st;

	if ((st = malloc(sizeof(*st))) == NULL)
		ld_fatal_std(ld, "malloc");
	st->st_size = 0;
	st->st_cap = _DEFAULT_STRTAB_SIZE;
	if ((st->st_buf = calloc(1, st->st_cap)) == NULL)
		ld_fatal_std(ld, "calloc");

	return (st);
}

static void
_resize_strtab(struct ld *ld, struct ld_strtab *st, size_t newsize)
{

	assert(st != NULL);
	if ((st->st_buf = realloc(st->st_buf, newsize)) == NULL)
		ld_fatal_std(ld, "realloc");
	st->st_cap = newsize;
}

void
ld_strtab_insert(struct ld *ld, struct ld_strtab *st, const char *s)
{
	const char *r;
	char *b, *c;
	size_t len, slen;
	int append;

	assert(st != NULL && st->st_buf != NULL);

	slen = strlen(s);
	append = 0;
	b = st->st_buf;
	for (c = b; c < b + st->st_size;) {
		len = strlen(c);
		if (!append && len >= slen) {
			r = c + (len - slen);
			if (strcmp(r, s) == 0)
				return;
		} else if (len < slen && len != 0) {
			r = s + (slen - len);
			if (strcmp(c, r) == 0) {
				st->st_size -= len + 1;
				memmove(c, c + len + 1, st->st_size - (c - b));
				append = 1;
				continue;
			}
		}
		c += len + 1;
	}

	while (st->st_size + slen + 1 >= st->st_cap)
		_resize_strtab(ld, st, st->st_cap * 2);

	b = st->st_buf;
	strncpy(&b[st->st_size], s, slen);
	b[st->st_size + slen] = '\0';
	st->st_size += slen + 1;
}

int
ld_strtab_lookup(struct ld_strtab *st, const char *s)
{
	const char	*b, *c, *r;
	size_t		 len, slen;

	slen = strlen(s);
	b = st->st_buf;
	for (c = b; c < b + st->st_size;) {
		len = strlen(c);
		if (len >= slen) {
			r = c + (len - slen);
			if (strcmp(r, s) == 0)
				return (r - b);
		}
		c += len + 1;
	}

	return (-1);
}
