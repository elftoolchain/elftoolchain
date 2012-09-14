/*-
 * Copyright (c) 2010-2012 Kai Wang
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
 *
 * $Id$
 */

#include "ld.h"
#include "ld_file.h"
#include "ld_path.h"

void
ld_path_add(struct ld *ld, char *path)
{
	struct ld_state *ls;
	struct ld_path *lp;

	assert(ld != NULL && path != NULL);
	ls = &ld->ld_state;

	if ((lp = calloc(1, sizeof(*lp))) == NULL)
		ld_fatal_std(ld, "calloc");

	if ((lp->lp_path = strdup(path)) == NULL)
		ld_fatal_std(ld, "strdup");

	STAILQ_INSERT_TAIL(&ls->ls_lplist, lp, lp_next);
}

void
ld_path_cleanup(struct ld *ld)
{
	struct ld_state *ls;
	struct ld_path *lp, *_lp;

	ls = &ld->ld_state;

	STAILQ_FOREACH_SAFE(lp, &ls->ls_lplist, lp_next, _lp) {
		STAILQ_REMOVE(&ls->ls_lplist, lp, ld_path, lp_next);
		free(lp->lp_path);
		free(lp);
	}
}

void
ld_path_search_file(struct ld *ld, struct ld_file *lf)
{
	struct ld_state *ls;
	struct ld_path *lp;
	struct dirent *dp;
	char fp[PATH_MAX + 1];
	DIR *dirp;
	int found;

	assert(lf != NULL);
	ls = &ld->ld_state;

	fp[0] = '\0';
	found = 0;
	STAILQ_FOREACH(lp, &ls->ls_lplist, lp_next) {
		assert(lp->lp_path != NULL);
		if ((dirp = opendir(lp->lp_path)) == NULL) {
			ld_warn(ld, "opendir failed: %s", strerror(errno));
			continue;
		}

		while ((dp = readdir(dirp)) != NULL) {
			if (!strcmp(dp->d_name, lf->lf_name)) {
				snprintf(fp, sizeof(fp), "%s/%s", lp->lp_path,
				    dp->d_name);
				free(lf->lf_name);
				if ((lf->lf_name = strdup(fp)) == NULL)
					ld_fatal_std(ld, "strdup");
				found = 1;
				goto done;
			}
		}
		(void) closedir(dirp);
	}
done:
	if (!found)
		ld_fatal(ld, "cannot find %s", lf->lf_name);
}

void
ld_path_search_library(struct ld *ld, const char *name)
{
	struct ld_state *ls;
	struct ld_path *lp;
	struct dirent *dp;
	DIR *dirp;
	char fp[PATH_MAX + 1], sfp[PATH_MAX + 1];
	size_t len;
	int found;

	assert(ld != NULL && name != NULL);
	ls = &ld->ld_state;

	len = strlen(name);
	found = 0;
	STAILQ_FOREACH(lp, &ls->ls_lplist, lp_next) {
		assert(lp->lp_path != NULL);
		if ((dirp = opendir(lp->lp_path)) == NULL) {
			ld_warn(ld, "opendir failed: %s", strerror(errno));
			continue;
		}

		fp[0] = sfp[0] = '\0';
		while ((dp = readdir(dirp)) != NULL) {
			if (strncmp(dp->d_name, "lib", 3))
				continue;
			if (strncmp(name, &dp->d_name[3], len))
				continue;
			if (ls->ls_static == 0 &&
			    !strcmp(&dp->d_name[len + 3], ".so")) {
				snprintf(fp, sizeof(fp), "%s/%s", lp->lp_path,
				    dp->d_name);
				ld_file_add(ld, fp, LFT_DSO);
				(void) closedir(dirp);
				found = 1;
				goto done;
			} else if (*sfp == '\0' &&
			    !strcmp(&dp->d_name[len + 3], ".a")) {
				snprintf(sfp, sizeof(sfp), "%s/%s", lp->lp_path,
				    dp->d_name);
				if (ls->ls_static == 1) {
					ld_file_add(ld, sfp, LFT_ARCHIVE);
					(void) closedir(dirp);
					found = 1;
					goto done;
				}
			}
		}
		(void) closedir(dirp);
	}
done:
	if (!found) {
		if (ls->ls_static == 0 && *sfp != '\0') {
			ld_file_add(ld, sfp, LFT_ARCHIVE);
		} else
			ld_fatal(ld, "cannot find -l%s", name);
	}
}
