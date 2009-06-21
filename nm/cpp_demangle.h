/*-
 * Copyright (c) 2007 Hyogeol Lee <hyogeollee@gmail.com>
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

#ifndef	GUARD_CPP_DEMANGLE_H
#define	GUARD_CPP_DEMANGLE_H

#include <stdbool.h>

/**
 * @file cpp_demangle.h
 * @brief Decode IA-64 C++ ABI style.
 *
 * IA-64 standard ABI(Itanium C++ ABI) references.
 *
 * http://www.codesourcery.com/cxx-abi/abi.html#mangling \n
 * http://www.codesourcery.com/cxx-abi/abi-mangling.html
 */

/**
 * @brief Decode the input string by IA-64 C++ ABI style.
 *
 * GNU GCC v3 use IA-64 standard ABI.
 * @return New allocated demangled string or NULL if failed.
 * @todo 1. Testing and more test case. 2. Code cleaning.
 */
char	*cpp_demangle_ia64(const char *);

/**
 * @brief Test input string is mangled by IA-64 C++ ABI style.
 *
 * Test string heads with "_Z" or "_GLOBAL__I_".
 * @return Return 0 at false.
 */
bool	is_cpp_mangled_ia64(const char *);

#endif /* !GUARD_CPP_DEMANGLE_H */
