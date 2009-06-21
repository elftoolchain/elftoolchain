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

#ifndef	GUARD_DWARF_LINE_NUMBER_H
#define	GUARD_DWARF_LINE_NUMBER_H

#include <sys/queue.h>
#include <sys/types.h>

/**
 * @file dwarf_line_number.h
 * @brief Decode line number information from DWARF debug information.
 *
 * DWARF debug information from http://dwarfstd.org/Dwarf3.pdf
 */

/** @brief List for compilation directory information. */
struct comp_dir_entry {
	/** file name */
	char		*src;
	/** directory */
	char		*dir;
	SLIST_ENTRY(comp_dir_entry) entries;
};

/** @brief List for line number information. */
struct line_info_entry {
	/** address */
	uint64_t	addr;
	/** line number */
	uint64_t	line;
	/** file name with path */
	char		*file;
	SLIST_ENTRY(line_info_entry) entries;
};

SLIST_HEAD(comp_dir_head, comp_dir_entry);
SLIST_HEAD(line_info_head, line_info_entry);

/** @brief Deallocate resource in comp_dir list. */
void	comp_dir_dest(struct comp_dir_head *l);

/** @brief Deallocate resource in line_info list. */
void	line_info_dest(struct line_info_head *l);

/**
 * @brief Get compilation directory information.
 * @param info .debug_info section
 * @param info_len length of info
 * @param abbrev .debug_abbrev section
 * @param abbrev_len length of abbrev
 * @param str .debug_str section. NULL for ignore
 * @param str_len length of str
 * @param l List to contain results. Not gaurantee rollback 'l' if failed.
 * @return 0 at failed, 1 at success.
 */
int	get_dwarf_info(void *info, size_t info_len, void *abbrev,
	    size_t abbrev_len, void *str, size_t str_len,
	    struct comp_dir_head *l);

/**
 * @brief Get line information.
 * @param buf .debug_line section
 * @param size size of buf
 * @param comp_dir Compilation directory information. NULL for ignore.
 * @param out List to contain results. Not gaurantee rollback 'out' if failed.
 * @return 0 at failed, 1 at success.
 */
int	get_dwarf_line_info(void *buf, uint64_t size,
	    struct comp_dir_head *comp_dir, struct line_info_head *out);

#endif /* !GUARD_DWARF_LINE_NUMBER_H */
