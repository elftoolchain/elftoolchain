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
#include "ld_exp.h"
#include "ld_options.h"
#include "ld_script.h"
#include "ld_file.h"
#include "ld_symbols.h"

ELFTC_VCSID("$Id$");

static void _input_file_add(struct ld *ld, struct ld_script_input_file *ldif);
static struct ld_script_variable *_variable_find(struct ld *ld, char *name);

#define _variable_add(v) \
	HASH_ADD_KEYPTR(hh, ld->ld_scp->lds_v, (v)->ldv_name, \
	    strlen((v)->ldv_name), (v))

struct ld_script_cmd *
ld_script_assert(struct ld *ld, struct ld_exp *exp, char *msg)
{
	struct ld_script_assert *a;

	if ((a = calloc(1, sizeof(*a))) == NULL)
		ld_fatal_std(ld, "calloc");
	a->lda_exp = exp;
	a->lda_msg = msg;

	return (ld_script_cmd(ld, LSC_ASSERT, a));
}

struct ld_script_assign *
ld_script_assign(struct ld *ld, struct ld_exp *var, enum ld_script_assign_op op,
    struct ld_exp *val, unsigned provide, unsigned hidden)
{
	struct ld_script_assign *lda;
	struct ld_script_variable *ldv;

	if ((lda = calloc(1, sizeof(*lda))) == NULL)
		ld_fatal_std(ld, "calloc");

	lda->lda_var = var;
	lda->lda_op = op;
	lda->lda_val = val;

	if ((ldv = _variable_find(ld, var->le_name)) == NULL) {
		ldv = calloc(1, sizeof(*ldv));
		if ((ldv->ldv_name = strdup(var->le_name)) == NULL)
			ld_fatal_std(ld, "strdup");
		_variable_add(ldv);
		if (*var->le_name != '.')
			ld_symbols_add_variable(ld, ldv, provide, hidden);
	}

	return (lda);
}

void
ld_script_process_assign(struct ld *ld, struct ld_script_assign *lda)
{
	struct ld_state *ls;
	struct ld_exp *var;
	struct ld_script_variable *ldv;

	ls = &ld->ld_state;
	var = lda->lda_var;
	ldv = _variable_find(ld, var->le_name);
	assert(ldv != NULL);

	ldv->ldv_val = ld_exp_eval(ld, lda->lda_val);
	if (*var->le_name == '.')
		ls->ls_loc_counter = (uint64_t) ldv->ldv_val;
	printf("%s = %#jx\n", var->le_name, (uint64_t) ldv->ldv_val);
}

int64_t
ld_script_variable_value(struct ld *ld, char *name)
{
	struct ld_script_variable *ldv;

	ldv = _variable_find(ld, name);
	assert(ldv != NULL);

	return (ldv->ldv_val);
}

struct ld_script_cmd *
ld_script_cmd(struct ld *ld, enum ld_script_cmd_type type, void *cmd)
{
	struct ld_script_cmd *ldc;

	if ((ldc = calloc(1, sizeof(*ldc))) == NULL)
		ld_fatal_std(ld, "calloc");
	ldc->ldc_type = type;
	ldc->ldc_cmd = cmd;

	return (ldc);
}

void
ld_script_cmd_insert(struct ld_script_cmd_head *head, struct ld_script_cmd *ldc)
{

	STAILQ_INSERT_TAIL(head, ldc, ldc_next);
}

void
ld_script_extern(struct ld *ld, struct ld_script_list *list)
{
	struct ld_script_list *ldl;

	ldl = list;
	while (ldl != NULL) {
		ld_symbols_add_extern(ld, ldl->ldl_entry);
		ldl = ldl->ldl_next;
	}
	ld_script_list_free(list);
}

void
ld_script_group(struct ld *ld, struct ld_script_list *list)
{
	struct ld_script_list *ldl;

	ld->ld_state.ls_group_level++;
	if (ld->ld_state.ls_group_level > LD_MAX_NESTED_GROUP)
		ld_fatal(ld, "too many nested archive groups");
	ldl = list;
	while (ldl != NULL) {
		_input_file_add(ld, ldl->ldl_entry);
		ldl = ldl->ldl_next;
	}
	ld->ld_state.ls_group_level--;
	ld_script_list_free(list);
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

	ld->ld_state.ls_search_dir = 1;
	ldl = list;
	while (ldl != NULL) {
		_input_file_add(ld, ldl->ldl_entry);
		ldl = ldl->ldl_next;
	}
	ld->ld_state.ls_search_dir = 0;
	ld_script_list_free(list);
}

struct ld_script_input_file *
ld_script_input_file(struct ld *ld, unsigned as_needed, void *in)
{
	struct ld_script_input_file *ldif;

	if ((ldif = calloc(1, sizeof(*ldif))) == NULL)
		ld_fatal_std(ld, "calloc");
	ldif->ldif_as_needed = as_needed;
	if (as_needed)
		ldif->ldif_u.ldif_ldl = in;
	else
		ldif->ldif_u.ldif_name = in;

	return (ldif);
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

void
ld_script_nocrossrefs(struct ld *ld, struct ld_script_list *list)
{
	struct ld_script_nocrossref *ldn;

	if ((ldn = calloc(1, sizeof(*ldn))) == NULL)
		ld_fatal_std(ld, "calloc");
	ldn->ldn_l = list;
	STAILQ_INSERT_TAIL(&ld->ld_scp->lds_n, ldn, ldn_next);
}

struct ld_script_phdr *
ld_script_phdr(struct ld *ld, char *name, char *type, unsigned filehdr,
    unsigned phdrs, struct ld_exp *addr, unsigned flags)
{
	struct ld_script_phdr *ldsp;

	if ((ldsp = calloc(1, sizeof(*ldsp))) == NULL)
		ld_fatal_std(ld, "calloc");

	ldsp->ldsp_name = name;
	ldsp->ldsp_type = type;
	ldsp->ldsp_filehdr = filehdr;
	ldsp->ldsp_phdrs = phdrs;
	ldsp->ldsp_addr = addr;
	ldsp->ldsp_flags = flags;

	return (ldsp);
}

struct ld_script_region *
ld_script_region(struct ld *ld, char *name, char *attr, struct ld_exp *origin,
    struct ld_exp *len)
{
	struct ld_script_region *ldsr;

	if ((ldsr = malloc(sizeof(*ldsr))) == NULL)
		ld_fatal_std(ld, "malloc");

	ldsr->ldsr_name = name;
	ldsr->ldsr_attr = attr;
	ldsr->ldsr_origin = origin;
	ldsr->ldsr_len = len;

	return (ldsr);
}

void
ld_script_region_alias(struct ld *ld, char *alias, char *region)
{
	struct ld_script_region_alias *ldra;

	if ((ldra = calloc(1, sizeof(*ldra))) == NULL)
		ld_fatal_std(ld, "calloc");

	ldra->ldra_alias = alias;
	ldra->ldra_region = region;

	STAILQ_INSERT_TAIL(&ld->ld_scp->lds_a, ldra, ldra_next);
}

static void
_input_file_add(struct ld *ld, struct ld_script_input_file *ldif)
{
	struct ld_state *ls;
	struct ld_script_list *ldl;
	unsigned old_as_needed;

	ls = &ld->ld_state;

	if (!ldif->ldif_as_needed) {
		ld_file_add(ld, ldif->ldif_u.ldif_name, LFT_UNKNOWN);
		free(ldif->ldif_u.ldif_name);
	} else {
		old_as_needed = ls->ls_as_needed;
		ls->ls_as_needed = 1;
		ldl = ldif->ldif_u.ldif_ldl;
		while (ldl != NULL) {
			ld_file_add(ld, ldl->ldl_entry, LFT_UNKNOWN);
			ldl = ldl->ldl_next;
		}
		ls->ls_as_needed = old_as_needed;
		ld_script_list_free(ldif->ldif_u.ldif_ldl);
	}
}

static struct ld_script_variable *
_variable_find(struct ld *ld, char *name)
{
	struct ld_script_variable *ldv;

	HASH_FIND_STR(ld->ld_scp->lds_v, name, ldv);

	return (ldv);
}
