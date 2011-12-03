/*-
 * Copyright (c) 2011 Kai Wang
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
#include "ld_options.h"
#include "ld_script.h"
#include "ld_file.h"

ELFTC_VCSID("$Id$");

void
ld_script_assert(struct ld *ld, struct ld_exp *exp, char *msg)
{
	struct ld_script_assert *a;

	if ((a = calloc(1, sizeof(*a))) == NULL)
		ld_fatal_std(ld, "calloc");
	a->lda_exp = exp;
	a->lda_msg = msg;

	ld_script_cmd(ld, LSS_ASSERT, a);
}

void
ld_script_cmd(struct ld *ld, enum ld_script_cmd_type type, void *cmd)
{
	struct ld_script_cmd *c;

	if ((c = calloc(1, sizeof(*c))) == NULL)
		ld_fatal_std(ld, "calloc");
	c->ldc_type = type;
	c->ldc_cmd = cmd;
	STAILQ_INSERT_TAIL(&ld->ld_scp->lds_c, c, ldc_next);
}

void
ld_script_init(struct ld *ld)
{

	ld->ld_scp = calloc(1, sizeof(*ld->ld_scp));
	if (ld->ld_scp == NULL)
		ld_fatal_std(ld, "calloc");

	STAILQ_INIT(&ld->ld_scp->lds_a);
	STAILQ_INIT(&ld->ld_scp->lds_c);
	STAILQ_INIT(&ld->ld_scp->lds_n);
	STAILQ_INIT(&ld->ld_scp->lds_p);
	STAILQ_INIT(&ld->ld_scp->lds_r);
}

void
ld_script_input(struct ld *ld, struct ld_script_list *list)
{
	struct ld_script_list *ldl;

	ld->ld_ls.ls_search_dir = 1;
	ldl = list;
	while (ldl != NULL) {
		ld_file_add(ld, ldl->ldl_entry, LFT_UNKNOWN);
		ldl = ldl->ldl_next;
	}
	ld->ld_ls.ls_search_dir = 0;
	ld_script_list_free(list);
}

struct ld_script_list *
ld_script_list(struct ld *ld, struct ld_script_list *list, void *entry)
{
	struct ld_script_list *ldl;

	if ((ldl = malloc(sizeof(*ldl))) == NULL)
		ld_fatal_std(ld, "malloc");
	ldl->ldl_entry = entry;
	ldl->ldl_next = list;

	return (ldl);
}

void
ld_script_list_free(struct ld_script_list *list)
{
	struct ld_script_list *ldl;

	do {
		ldl = list;
		list = ldl->ldl_next;
		if (ldl->ldl_entry)
			free(ldl->ldl_entry);
		free(ldl);
	} while (list != NULL);
}

struct ld_script_list *
ld_script_list_reverse(struct ld_script_list *list)
{
	struct ld_script_list *root, *next;

	root = NULL;
	while (list != NULL) {
		next = list->ldl_next;
		list->ldl_next = root;
		root = list;
		list = next;
	}

	return (root);
}
