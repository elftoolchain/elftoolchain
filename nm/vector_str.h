/*-
 * Copyright (c) 2008 Hyogeol Lee <hyogeollee@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	GUARD_VECTOR_STR_H
#define	GUARD_VECTOR_STR_H

#include <stdbool.h>
#include <stdlib.h>

/**
 * @file vector_str.h
 * @brief Dynamic vector data for string.
 *
 * Resemble to std::vector<std::string> in C++.
 */

/** @brief Dynamic vector data for string. */
struct vector_str {
	/** Current size */
	size_t		size;
	/** Total capacity */
	size_t		capacity;
	/** String array */
	char		**container;
};

#define BUFFER_GROWFACTOR	1.618
#define VECTOR_DEF_CAPACITY	8

/** @brief Deallocate resource in vector_str. */
void	vector_str_dest(struct vector_str *);

/**
 * @brief Find string in vector_str.
 * @param v Destination vector.
 * @param o String to find.
 * @param l Length of the string.
 * @return -1 at failed, 0 at not found, 1 at found.
 */
int	vector_str_find(const struct vector_str *v, const char *o, size_t l);

/**
 * @brief Get new allocated flat string from vector.
 *
 * If l is not NULL, return length of the string.
 * @param v Destination vector.
 * @param l Length of the string.
 * @return NULL at failed or NUL terminated new allocated string.
 */
char	*vector_str_get_flat(const struct vector_str *v, size_t *l);

/**
 * @brief Initialize vector_str.
 * @return false at failed, true at success.
 */
bool	vector_str_init(struct vector_str *);

/**
 * @brief Remove last element in vector_str.
 * @return false at failed, true at success.
 */
bool	vector_str_pop(struct vector_str *);

/**
 * @brief Push back string to vector.
 * @return false at failed, true at success.
 */
bool	vector_str_push(struct vector_str *, const char *, size_t);

/**
 * @brief Push front org vector to det vector.
 * @return false at failed, true at success.
 */
bool	vector_str_push_vector_head(struct vector_str *dst,
	    struct vector_str *org);

/**
 * @brief Get new allocated flat string from vector between begin and end.
 *
 * If r_len is not NULL, string length will be returned.
 * @return NULL at failed or NUL terminated new allocated string.
 */
char	*vector_str_substr(const struct vector_str *v, size_t begin, size_t end,
	    size_t *r_len);

#endif /* !GUARD_VECTOR_STR_H */
