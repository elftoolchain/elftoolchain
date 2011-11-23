/*-
 * Copyright (c) 2010 Kai Wang
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "ld.h"
#include "ld_file.h"

ELFTC_VCSID("$Id$");

/*
 * Support routines for input file handling.
 */

void
ld_file_add(struct ld *ld, const char *name, enum ld_file_type type)
{
	struct ld_state *ls;
	struct ld_file *lf;

	assert(ld != NULL && name != NULL);
	ls = &ld->ld_ls;

	if ((lf = calloc(1, sizeof(*lf))) == NULL)
		ld_fatal_std(ld, "calloc");

	if ((lf->lf_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");

	lf->lf_type	     = type;
	lf->lf_whole_archive = ls->ls_whole_archive;
	lf->lf_as_needed     = ls->ls_as_needed;
	lf->lf_group_level   = ls->ls_group_level;

	if (lf->lf_type == LFT_UNKNOWN && ls->ls_itgt != NULL) {
		if (elftc_bfd_target_flavor(ls->ls_itgt) == ETF_BINARY)
			lf->lf_type = LFT_BINARY;
	}

	TAILQ_INSERT_TAIL(&ld->ld_lflist, lf, lf_next);
}

void
ld_file_add_library(struct ld *ld, const char *name)
{
	struct ld_state *ls;
	struct ld_path *lp;
	struct dirent *dp;
	DIR *dirp;
	char fpath[PATH_MAX + 1];
	size_t len;
	int found;

	assert(ld != NULL && name != NULL);
	ls = &ld->ld_ls;

	len = strlen(name);
	found = 0;
	STAILQ_FOREACH(lp, &ls->ls_lplist, lp_next) {
		assert(lp->lp_path != NULL);
		if ((dirp = opendir(lp->lp_path)) == NULL) {
			ld_warn(ld, "opendir failed: %s", strerror(errno));
			continue;
		}

		while ((dp = readdir(dirp)) != NULL) {
			if (strncmp(dp->d_name, "lib", 3))
				continue;
			if (strncmp(name, &dp->d_name[3], len))
				continue;
			if (ls->ls_static == 0 &&
			    !strcmp(&dp->d_name[len + 3], ".so")) {
				snprintf(fpath, sizeof(fpath), "%s/%s",
				    lp->lp_path, dp->d_name);
				ld_file_add(ld, fpath, LFT_DSO);
				(void) closedir(dirp);
				found = 1;
				goto done;
			} else if (!strcmp(&dp->d_name[len + 3], ".a")) {
				snprintf(fpath, sizeof(fpath), "%s/%s",
				    lp->lp_path, dp->d_name);
				ld_file_add(ld, fpath, LFT_ARCHIVE);
				(void) closedir(dirp);
				found = 1;
				goto done;
			}
		}
		(void) closedir(dirp);
	}
done:
	if (!found)
		ld_fatal(ld, "cannot find -l%s", name);
}

void
ld_file_add_library_path(struct ld *ld, char *path)
{
	struct ld_state *ls;
	struct ld_path *lp;

	assert(ld != NULL && path != NULL);
	ls = &ld->ld_ls;

	if ((lp = calloc(1, sizeof(*lp))) == NULL)
		ld_fatal_std(ld, "calloc");

	lp->lp_path = path;
	STAILQ_INSERT_TAIL(&ls->ls_lplist, lp, lp_next);
}

static void
ld_file_load_archive(struct ld *ld, struct ld_file *lf)
{

	(void) ld;
	(void) lf;
}

static void
ld_file_load(struct ld *ld, struct ld_file *lf)
{
	struct stat sb;
	Elf_Kind k;
	GElf_Ehdr ehdr;
	int fd;

	assert(lf != NULL && lf->lf_name != NULL);

	if ((fd = open(lf->lf_name, O_RDONLY)) < 0)
		ld_fatal_std(ld, "%s: open", lf->lf_name);

	if (fstat(fd, &sb) < 0)
		ld_fatal_std(ld, "%s: stat", lf->lf_name);
	if (sb.st_size == 0)
		ld_fatal(ld, "%s: File truncated", lf->lf_name);

	lf->lf_size = sb.st_size;
	if ((lf->lf_mmap = mmap(NULL, lf->lf_size, PROT_READ, MAP_PRIVATE, fd,
	    (off_t) 0)) == MAP_FAILED)
		ld_fatal_std(ld, "%s: mmap", lf->lf_name);
	close(fd);

	if (lf->lf_type == LFT_ARCHIVE || (lf->lf_type == LFT_UNKNOWN &&
	    !strncmp(lf->lf_mmap, ARMAG, SARMAG))) {
		ld_file_load_archive(ld, lf);
		return;
	} else if (lf->lf_type == LFT_BINARY)
		return;

	if ((lf->lf_elf = elf_memory(lf->lf_mmap, lf->lf_size)) == NULL)
		ld_fatal(ld, "%s: elf_memory failed: %s", lf->lf_name,
		    elf_errmsg(elf_errno()));

	k = elf_kind(lf->lf_elf);
	assert(k != ELF_K_AR);
	if (k == ELF_K_NONE)
		ld_fatal(ld, "%s: File format not recognized", lf->lf_name);

	if (gelf_getehdr(lf->lf_elf, &ehdr) == NULL)
		ld_fatal(ld, "%s: gelf_getehdr failed: %s", lf->lf_name,
		    elf_errmsg(elf_errno()));
	switch (ehdr.e_type) {
	case ET_NONE:
		ld_fatal(ld, "%s: ELF type ET_NONE not supported", lf->lf_name);
	case ET_REL:
		lf->lf_type = LFT_RELOCATABLE;
		break;
	case ET_EXEC:
		ld_fatal(ld, "%s: ELF type ET_EXEC not supported yet",
		    lf->lf_name);
	case ET_DYN:
		lf->lf_type = LFT_DSO;
		break;
	case ET_CORE:
		ld_fatal(ld, "%s: ELF type ET_NONE not supported", lf->lf_name);
	default:
		ld_fatal(ld, "%s: unknown ELF type %u", ehdr.e_type);
	}
}

void
ld_file_load_all(struct ld *ld)
{
	struct ld_file *lf;

	TAILQ_FOREACH(lf, &ld->ld_lflist, lf_next) {
		ld_file_load(ld, lf);
	}
}
