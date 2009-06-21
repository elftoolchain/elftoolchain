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

#ifndef	GUARD_CPP_DEMANGLE_ARM_H
#define	GUARD_CPP_DEMANGLE_ARM_H

#include <stdbool.h>
#include <string.h>

/**
 * @file cpp_demangle_arm.h
 * @brief Decode function name encoding in ARM.
 *
 * Function name encoding in "The Annotated C++ Reference Manual".
 *
 * Ref : "The Annotated C++ Reference Manual", Margaet A.Ellis,
 *  Bjarne Stroustrup, AT&T Bell Laboratories 1990, pp 122-126.
 */

/**
 * @brief Decode the input string by the ARM style.
 *
 * @return New allocated demangled string or NULL if failed.
 */
char *cpp_demangle_ARM(const char *);

/**
 * @brief Test input string is encoded by the ARM style.
 *
 * @return True if input string is encoded by the ARM style.
 */
bool is_cpp_mangled_ARM(const char *);

#endif /* !GUARD_CPP_DEMANGLE_ARM_H */
