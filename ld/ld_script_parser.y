%{
/*-
 * Copyright (c) 2010,2011 Kai Wang
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
#include "ld_options.h"
#include "ld_script.h"
#include "ld_file.h"
#include "ld_exp.h"

ELFTC_VCSID("$Id$");

struct yy_buffer_state;
typedef struct yy_buffer_state *YY_BUFFER_STATE;

extern int yylex(void);
extern int yyparse(void);
extern YY_BUFFER_STATE yy_scan_string(char *yy_str);
extern void yy_delete_buffer(YY_BUFFER_STATE b);
extern int lineno;
extern FILE *yyin;
extern struct ld *ld;
extern char *ldscript_default;

static void yyerror(const char *s);
static void _init_script(void);
static struct ld_script_cmd_head ldss_c, ldso_c;

%}

%token T_ABSOLUTE
%token T_ADDR
%token T_ALIGN
%token T_ALIGNOF
%token T_ASSERT
%token T_AS_NEEDED
%token T_AT
%token T_BIND
%token T_BLOCK
%token T_BYTE
%token T_CONSTANT
%token T_CONSTRUCTORS
%token T_CREATE_OBJECT_SYMBOLS
%token T_DATA_SEGMENT_ALIGN
%token T_DATA_SEGMENT_END
%token T_DATA_SEGMENT_RELRO_END
%token T_DEFINED
%token T_ENTRY
%token T_EXCLUDE_FILE
%token T_EXTERN
%token T_FILEHDR
%token T_FILL
%token T_FLAGS
%token T_FLOAT
%token T_FORCE_COMMON_ALLOCATION
%token T_GROUP
%token T_HLL
%token T_INCLUDE
%token T_INHIBIT_COMMON_ALLOCATION
%token T_INPUT
%token T_KEEP
%token T_LENGTH
%token T_LOADADDR
%token T_LONG
%token T_MAP
%token T_MAX
%token T_MEMORY
%token T_MIN
%token T_NEXT
%token T_NOCROSSREFS
%token T_NOFLOAT
%token T_OPTION
%token T_ORIGIN
%token T_OUTPUT
%token T_OUTPUT_ARCH
%token T_OUTPUT_FORMAT
%token T_PHDRS
%token T_PROVIDE
%token T_PROVIDE_HIDDEN
%token T_QUAD
%token T_REGION_ALIAS
%token T_SEARCH_DIR
%token T_SECTIONS
%token T_SEGMENT_START
%token T_SHORT
%token T_SIZEOF
%token T_SIZEOF_HEADERS
%token T_SORT_BY_ALIGNMENT
%token T_SORT_BY_NAME
%token T_SPECIAL
%token T_SQUAD
%token T_STARTUP
%token T_SUBALIGN
%token T_SYSLIB
%token T_TARGET
%token T_TRUNCATE
%token T_VERSION
%token T_VER_EXTERN
%token T_VER_GLOBAL
%token T_VER_LOCAL

%token T_LSHIFT_E
%token T_RSHIFT_E
%token T_LSHIFT
%token T_RSHIFT
%token T_EQ
%token T_NE
%token T_GE
%token T_LE
%token T_ADD_E
%token T_SUB_E
%token T_MUL_E
%token T_DIV_E
%token T_AND_E
%token T_OR_E
%token T_LOGICAL_AND
%token T_LOGICAL_OR

%right '=' T_AND_E T_OR_E T_MUL_E T_DIV_E T_ADD_E T_SUB_E T_LSHIFT_E T_RSHIFT_E
%right '?' ':'
%left T_LOGICAL_OR
%left T_LOGICAL_AND
%left '|'
%left '&'
%left T_EQ T_NE T_GE T_LE '>' '<'
%left T_LSHIFT T_RSHIFT
%left '+' '-'
%left '*' '/' '%'
%left UNARY

%token <num> T_NUM
%token <str> T_COMMONPAGESIZE
%token <str> T_COPY
%token <str> T_DSECT
%token <str> T_IDENT
%token <str> T_INFO
%token <str> T_MAXPAGESIZE
%token <str> T_MEMORY_ATTR
%token <str> T_NOLOAD
%token <str> T_ONLY_IF_RO
%token <str> T_ONLY_IF_RW
%token <str> T_OVERLAY
%token <str> T_STRING
%token <str> T_WILDCARD

%type <assign> assignment
%type <assign> provide_assignment
%type <assign> provide_hidden_assignment
%type <assign> simple_assignment
%type <exp> expression
%type <exp> function
%type <exp> constant
%type <exp> variable
%type <exp> absolute_function
%type <exp> addr_function
%type <exp> align_function
%type <exp> alignof_function
%type <exp> block_function
%type <exp> data_segment_align_function
%type <exp> data_segment_end_function
%type <exp> data_segment_relro_end_function
%type <exp> defined_function
%type <exp> length_function
%type <exp> loadaddr_function
%type <exp> max_function
%type <exp> min_function
%type <exp> next_function
%type <exp> origin_function
%type <exp> output_section_addr
%type <exp> output_section_align
%type <exp> output_section_fillexp
%type <exp> output_section_lma
%type <exp> output_section_subalign
%type <exp> overlay_vma
%type <exp> phdr_at
%type <exp> segment_start_function
%type <exp> sizeof_function
%type <exp> sizeof_headers_function
%type <input_file> input_file
%type <input_section> input_section
%type <input_section> input_section_desc
%type <input_section> input_section_desc_no_keep
%type <list> as_needed_list
%type <list> ident_list
%type <list> ident_list_nosep
%type <list> input_file_list
%type <list> output_section_addr_and_type
%type <list> output_section_phdr
%type <list> overlay_section_list
%type <list> wildcard_list
%type <num> assign_op
%type <num> overlay_nocref
%type <num> phdr_filehdr
%type <num> phdr_flags
%type <num> phdr_phdrs
%type <overlay_section> overlay_section
%type <phdr> phdr
%type <region> memory_region
%type <str> ident
%type <str> memory_attr
%type <str> output_section_constraint
%type <str> output_section_lma_region
%type <str> output_section_region
%type <str> output_section_type
%type <str> output_section_type_keyword
%type <str> symbolic_constant
%type <str> wildcard
%type <wildcard> wildcard_sort

%union {
	struct ld_exp *exp;
	struct ld_script_assign *assign;
	struct ld_script_list *list;
	struct ld_script_input_file *input_file;
	struct ld_script_phdr *phdr;
	struct ld_script_region *region;
	struct ld_script_sections_output_input *input_section;
	struct ld_script_sections_overlay_section *overlay_section;
	struct ld_wildcard *wildcard;
	char *str;
	int64_t num;
}

%%

script
	: ldscript
	|
	;

ldscript
	: ldscript_command
	| ldscript ldscript_command
	;

expression
	: expression '+' expression {
		$$ = ld_exp_binary(ld, LEOP_ADD, $1, $3);
	}
	| expression '-' expression {
		$$ = ld_exp_binary(ld, LEOP_SUBSTRACT, $1, $3);
	}
	| expression '*' expression {
		$$ = ld_exp_binary(ld, LEOP_MUL, $1, $3);
	}
	| expression '/' expression {
		$$ = ld_exp_binary(ld, LEOP_DIV, $1, $3);
	}
	| expression '%' expression {
		$$ = ld_exp_binary(ld, LEOP_MOD, $1, $3);
	}
	| expression '&' expression {
		$$ = ld_exp_binary(ld, LEOP_AND, $1, $3);
	}
	| expression '|' expression {
		$$ = ld_exp_binary(ld, LEOP_OR, $1, $3);
	}
	| expression '>' expression {
		$$ = ld_exp_binary(ld, LEOP_GREATER, $1, $3);
	}
	| expression '<' expression {
		$$ = ld_exp_binary(ld, LEOP_LESSER, $1, $3);
	}
	| expression T_EQ expression {
		$$ = ld_exp_binary(ld, LEOP_EQUAL, $1, $3);
	}
	| expression T_NE expression {
		$$ = ld_exp_binary(ld, LEOP_NE, $1, $3);
	}
	| expression T_GE expression {
		$$ = ld_exp_binary(ld, LEOP_GE, $1, $3);
	}
	| expression T_LE expression {
		$$ = ld_exp_binary(ld, LEOP_LE, $1, $3);
	}
	| expression T_LSHIFT expression {
		$$ = ld_exp_binary(ld, LEOP_LSHIFT, $1, $3);
	}
	| expression T_RSHIFT expression {
		$$ = ld_exp_binary(ld, LEOP_RSHIFT, $1, $3);
	}
	| expression T_LOGICAL_AND expression {
		$$ = ld_exp_binary(ld, LEOP_LOGICAL_AND, $1, $3);
	}
	| expression T_LOGICAL_OR expression {
		$$ = ld_exp_binary(ld, LEOP_LOGICAL_OR, $1, $3);
	}
	| '!' expression %prec UNARY {
		$$ = ld_exp_unary(ld, LEOP_NOT, $2);
	}
	| '-' expression %prec UNARY {
		$$ = ld_exp_unary(ld, LEOP_MINUS, $2);
	}
	| '~' expression %prec UNARY {
		$$ = ld_exp_unary(ld, LEOP_NEGATION, $2);
	}
	| expression '?' expression ':' expression {
		$$ = ld_exp_trinary(ld, $1, $3, $5);
	}
	| simple_assignment {
		$$ = ld_exp_assign(ld, $1);
	}
	| function
	| constant
	| variable
	| '(' expression ')' { $$ = $2;	}
	;

function
	: absolute_function
	| addr_function
	| align_function
	| alignof_function
	| block_function
	| data_segment_align_function
	| data_segment_end_function
	| data_segment_relro_end_function
	| defined_function
	| length_function
	| loadaddr_function
	| max_function
	| min_function
	| next_function
	| origin_function
	| segment_start_function
	| sizeof_function
	| sizeof_headers_function
	;

absolute_function
	: T_ABSOLUTE '(' expression ')' {
		$$ = ld_exp_unary(ld, LEOP_ABS, $3);
	}
	;

addr_function
	: T_ADDR '(' ident ')' {
		$$ = ld_exp_unary(ld, LEOP_ADDR, ld_exp_name(ld, $3));
	}
	;

align_function
	: T_ALIGN '(' expression ')' {
		$$ = ld_exp_unary(ld, LEOP_ALIGN, $3);
	}
	| T_ALIGN '(' expression ',' expression ')' {
		$$ = ld_exp_binary(ld, LEOP_ALIGN, $3, $5);
	}
	;

alignof_function
	: T_ALIGNOF '(' ident ')' {
		$$ = ld_exp_unary(ld, LEOP_ALIGNOF, ld_exp_name(ld, $3));
	}
	;

block_function
	: T_BLOCK '(' expression ')' {
		$$ = ld_exp_unary(ld, LEOP_BLOCK, $3);
	}
	;

data_segment_align_function
	: T_DATA_SEGMENT_ALIGN '(' expression ',' expression ')' {
		$$ = ld_exp_binary(ld, LEOP_DSA, $3, $5);
	}
	;

data_segment_end_function
	: T_DATA_SEGMENT_END '(' expression ')' {
		$$ = ld_exp_unary(ld, LEOP_DSE, $3);
	}
	;

data_segment_relro_end_function
	: T_DATA_SEGMENT_RELRO_END '(' expression ',' expression ')' {
		$$ = ld_exp_binary(ld, LEOP_DSRE, $3, $5);
	}
	;

defined_function
	: T_DEFINED '(' ident ')' {
		$$ = ld_exp_unary(ld, LEOP_DEFINED, ld_exp_symbol(ld, $3));
	}
	;

length_function
	: T_LENGTH '(' ident ')' {
		$$ = ld_exp_unary(ld, LEOP_LENGTH, ld_exp_name(ld, $3));
	}
	;

loadaddr_function
	: T_LOADADDR '(' ident ')' {
		$$ = ld_exp_unary(ld, LEOP_LOADADDR, ld_exp_name(ld, $3));
	}
	;

max_function
	: T_MAX '(' expression ',' expression ')' {
		$$ = ld_exp_binary(ld, LEOP_MAX, $3, $5);
	}
	;

min_function
	: T_MIN '(' expression ',' expression ')' {
		$$ = ld_exp_binary(ld, LEOP_MIN, $3, $5);
	}
	;

next_function
	: T_NEXT '(' expression ')' {
		$$ = ld_exp_unary(ld, LEOP_NEXT, $3);
	}
	;

origin_function
	: T_ORIGIN '(' ident ')' {
		$$ = ld_exp_unary(ld, LEOP_ORIGIN, ld_exp_name(ld, $3));
	}
	;

segment_start_function
	: T_SEGMENT_START '(' ident ',' expression ')' {
		$$ = ld_exp_binary(ld, LEOP_MIN, ld_exp_name(ld, $3), $5);
	}
	;

sizeof_function
	: T_SIZEOF '(' ident ')' {
		$$ = ld_exp_unary(ld, LEOP_SIZEOF, ld_exp_name(ld, $3));
	}
	;

sizeof_headers_function
	: T_SIZEOF_HEADERS {
		$$ = ld_exp_sizeof_headers(ld);
	}
	;

constant
	: T_NUM {
		$$ = ld_exp_constant(ld, $1);
	}
	| symbolic_constant {
		$$ = ld_exp_symbolic_constant(ld, $1);
	}
	;

symbolic_constant
	: T_CONSTANT '(' T_COMMONPAGESIZE ')' { $$ = $3; }
	| T_CONSTANT '(' T_MAXPAGESIZE ')' { $$ = $3; }
	;

ldscript_command
	: assert_command
	| assignment
	| entry_command
	| extern_command
	| force_common_allocation_command
	| group_command
	| inhibit_common_allocation_command
	| input_command
	| memory_command
	| nocrossrefs_command
	| output_command
	| output_arch_command
	| output_format_command
	| phdrs_command
	| region_alias_command
	| search_dir_command
	| sections_command
	| startup_command
	| target_command
	| version_script_node ';'
	;

assignment
	: simple_assignment
	| provide_assignment
	| provide_hidden_assignment
	;

simple_assignment
	: variable assign_op expression %prec '=' {
		$$ = ld_script_assign(ld, $1, $2, $3, 0, 0);
	}
	;

provide_assignment
	: T_PROVIDE '(' variable '=' expression ')' {
		$$ = ld_script_assign(ld, $3, LSAOP_E, $5, 1, 0);
	}
	;

provide_hidden_assignment
	: T_PROVIDE_HIDDEN '(' variable '=' expression ')' {
		$$ = ld_script_assign(ld, $3, LSAOP_E, $5, 1, 1);
	}
	;

assign_op
	: T_LSHIFT_E { $$ = LSAOP_LSHIFT_E; }
	| T_RSHIFT_E { $$ = LSAOP_RSHIFT_E; }
	| T_ADD_E { $$ = LSAOP_ADD_E; }
	| T_SUB_E { $$ = LSAOP_SUB_E; }
	| T_MUL_E { $$ = LSAOP_MUL_E; }
	| T_DIV_E { $$ = LSAOP_DIV_E; }
	| T_AND_E { $$ = LSAOP_AND_E; }
	| T_OR_E { $$ = LSAOP_OR_E; }
	| '=' { $$ = LSAOP_E; }
	;

assert_command
	: T_ASSERT '(' expression ',' T_STRING ')' {
		ld_script_assert(ld, $3, $5);
	}
	;

entry_command
	: T_ENTRY '(' ident ')' {
		if (ld->ld_scp_entry != NULL)
			free(ld->ld_scp_entry);
		ld->ld_scp_entry = $3;
	}
	;

extern_command
	: T_EXTERN '(' ident_list_nosep ')' { ld_script_extern(ld, $3); }
	;

force_common_allocation_command
	: T_FORCE_COMMON_ALLOCATION { ld->ld_common_alloc = 1; }
	;

group_command
	: T_GROUP '(' input_file_list ')' {
		 ld_script_group(ld, ld_script_list_reverse($3));
	}
	;

inhibit_common_allocation_command
	: T_INHIBIT_COMMON_ALLOCATION { ld->ld_common_no_alloc = 1; }
	;

input_command
	: T_INPUT '(' input_file_list ')' {
		ld_script_input(ld, ld_script_list_reverse($3));
	}
	;

memory_command
	: T_MEMORY '{' memory_region_list '}'
	;

memory_region_list
	: memory_region {
		STAILQ_INSERT_TAIL(&ld->ld_scp->lds_r, $1, ldsr_next);
	}
	| memory_region_list memory_region {
		STAILQ_INSERT_TAIL(&ld->ld_scp->lds_r, $2, ldsr_next);
	}
	;

memory_region
	: ident memory_attr ':' T_ORIGIN '=' expression ',' T_LENGTH '='
	expression {
		ld_script_region(ld, $1, $2, $6, $10);
	}
	;

memory_attr
	: T_MEMORY_ATTR
	| { $$ = NULL; }
	;

nocrossrefs_command
	: T_NOCROSSREFS '(' ident_list_nosep ')' {
		ld_script_nocrossrefs(ld, $3);
	}
	;

output_command
	: T_OUTPUT '(' ident ')' {
		if (ld->ld_output == NULL)
			ld->ld_output = $3;
		else
			free($3);
	}
	;

output_arch_command
	: T_OUTPUT_ARCH '(' ident ')'
	;

output_format_command
	: T_OUTPUT_FORMAT '(' ident ')'
	| T_OUTPUT_FORMAT '(' ident ',' ident ',' ident ')'
	;

phdrs_command
	: T_PHDRS '{' phdr_list '}'
	;

phdr_list
	: phdr {
		STAILQ_INSERT_TAIL(&ld->ld_scp->lds_p, $1, ldsp_next);
	}
	| phdr_list phdr {
		STAILQ_INSERT_TAIL(&ld->ld_scp->lds_p, $2, ldsp_next);
	}

phdr
	: ident ident phdr_filehdr phdr_phdrs phdr_at phdr_flags ';' {
		$$ = ld_script_phdr(ld, $1, $2, $3, $4, $5, $6);
	}
	;

phdr_filehdr
	: T_FILEHDR { $$ = 1; }
	| { $$ = 0; }
	;

phdr_phdrs
	: T_PHDRS { $$ = 1; }
	| { $$ = 0; }
	;

phdr_at
	: T_AT '(' expression ')' { $$ = $3; }
	| { $$ = NULL; }
	;

phdr_flags
	: T_FLAGS '(' T_NUM ')' { $$ = $3; }
	| { $$ = 0; }
	;

region_alias_command
	: T_REGION_ALIAS '(' ident ',' ident ')' {
		ld_script_region_alias(ld, $3, $5);
	}
	;

search_dir_command
	: T_SEARCH_DIR '(' ident ')' {
		ld_file_add_library_path(ld, $3);
		free($3);
	}
	;

sections_command
	: T_SECTIONS '{' sections_command_list '}'
	;

sections_command_list
	: sections_sub_command
	| sections_command_list sections_sub_command
	;

sections_sub_command
	: entry_command
	| assignment
	| output_sections_desc
	| overlay_desc
	| ';'
	;

output_sections_desc
	: ident output_section_addr_and_type ':'
	output_section_lma
	output_section_align
	output_section_subalign
	output_section_constraint
	'{' output_section_command_list '}'
	output_section_region
	output_section_lma_region
	output_section_phdr
	output_section_fillexp {
		ld_script_sections_output(ld, &ldss_c, $1, $2, $4, $5, $6, $7,
		    &ldso_c, $11, $12, ld_script_list_reverse($13), $14);
		STAILQ_INIT(&ldso_c);
	}
	;

output_section_addr_and_type
	: output_section_addr output_section_type {
		$$ = ld_script_list(ld, NULL, $2);
		$$ = ld_script_list(ld, $$, $1);
	}
	| output_section_type {
		$$ = ld_script_list(ld, NULL, NULL);
		$$ = ld_script_list(ld, $$, $1);
	}
	;

output_section_addr
	: expression
	;

output_section_type
	: '(' output_section_type_keyword ')' { $$ = $2; }
	| '(' ')' { $$ = NULL; }
	| { $$ = NULL; }
	;

output_section_type_keyword
	: T_COPY
	| T_DSECT
	| T_INFO
	| T_NOLOAD
	| T_OVERLAY
	;

output_section_lma
	: T_AT '(' expression ')' { $$ = $3; }
	| { $$ = NULL; }
	;

output_section_align
	: T_ALIGN '(' expression ')' { $$ = $3; }
	| { $$ = NULL; }
	;

output_section_subalign
	: T_SUBALIGN '(' expression ')' { $$ = $3; }
	| { $$ = NULL; }
	;

output_section_constraint
	: T_ONLY_IF_RO
	| T_ONLY_IF_RW
	| { $$ = NULL; }
	;

output_section_command_list
	: output_section_command
	| output_section_command_list output_section_command
	;

output_section_command
	: assignment
	| input_section_desc
	| output_section_data
	| output_section_keywords
	| ';'
	;

input_section_desc
	: input_section_desc_no_keep {
		$1->ldoi_keep = 0;
		$$ = $1;
	}
	| T_KEEP '(' input_section_desc_no_keep ')' {
		$3->ldoi_keep = 0;
		$$ = $3;
	}
	;

input_section_desc_no_keep
	: wildcard_sort input_section {
		$2->ldoi_ar = NULL;
		$2->ldoi_file = $1;
		$$ = $2;
	}
	| wildcard_sort ':' wildcard_sort input_section {
		$4->ldoi_ar = $1;
		$4->ldoi_ar = $3;
		$$ = $4;
	}
	;

input_section
	: '(' T_EXCLUDE_FILE '(' wildcard_list ')' wildcard_list ')' {
		$$ = calloc(1, sizeof(struct ld_script_sections_output_input));
		if ($$ == NULL)
			ld_fatal_std(ld, "calloc");
		$$->ldoi_exclude = $4;
		$$->ldoi_sec = $6;
	}
	| '(' wildcard_list ')' {
		$$ = calloc(1, sizeof(struct ld_script_sections_output_input));
		if ($$ == NULL)
			ld_fatal_std(ld, "calloc");
		$$->ldoi_exclude = NULL;
		$$->ldoi_sec = $2;
	}
	;

output_section_data
	: data_type '(' expression ')'
	;

data_type
	: T_BYTE
	| T_SHORT
	| T_LONG
	| T_QUAD
	| T_SQUAD
	;

output_section_keywords
	: T_CREATE_OBJECT_SYMBOLS
	| T_CONSTRUCTORS
	| T_SORT_BY_NAME '(' T_CONSTRUCTORS ')'
	;

output_section_region
	: '>' ident { $$ = $2; }
	| { $$ = NULL; }
	;

output_section_lma_region
	: T_AT '>' ident { $$ = $3; }
	| { $$ = NULL; }
	;

output_section_phdr
	: output_section_phdr ':' ident {
		$$ = ld_script_list(ld, $$, $3);
	}
	| { $$ = NULL; }
	;


output_section_fillexp
	: '=' expression { $$ = $2; }
	| { $$ = NULL; }
	;

overlay_desc
	: T_OVERLAY
	overlay_vma ':'
	overlay_nocref
	output_section_lma
	'{' overlay_section_list '}'
	output_section_region
	output_section_phdr
	output_section_fillexp {
		ld_script_section_overlay(ld, &ldss_c, $2, $4, $5, $7, $9,
		    $10, $11);
	}
	;

overlay_vma
	: expression
	| { $$ = NULL; }
	;

overlay_nocref
	: T_NOCROSSREFS { $$ = 1; }
	| { $$ = 0; }
	;

overlay_section_list
	: overlay_section {
		$$ = ld_script_list(ld, NULL, $1);
	}
	| overlay_section_list overlay_section {
		$$ = ld_script_list(ld, $1, $2);
	}
	;

overlay_section
	: ident
	'{' output_section_command_list '}'
	output_section_phdr
	output_section_fillexp {
		$$ = ld_script_sections_overlay_section(ld, $1, &ldso_c, $5,
		    $6);
		STAILQ_INIT(&ldso_c);
	}
	;

startup_command
	: T_STARTUP '(' ident ')' {
		ld_file_add_first(ld, $3, LFT_UNKNOWN);
		free($3);
	}
	;

target_command
	: T_TARGET '(' ident ')'
	;

version_script_node
	: ident extern_block version_dependency
	| ident version_block version_dependency
	;

extern_block
	: T_VER_EXTERN T_STRING version_block
	;

version_block
	: '{' version_definition_list '}'
	;

version_definition_list
	: version_definition
	| version_definition_list version_definition
	;

version_definition
	: T_VER_GLOBAL
	| T_VER_LOCAL
	| wildcard ';'
	;

version_dependency
	: ident
	|
	;

ident
	: T_IDENT
	| T_STRING
	;

variable
	: ident { $$ = ld_exp_symbol(ld, $1); }
	| '.'  { $$ = ld_exp_symbol(ld, "."); }
	;

wildcard
	: ident 
	| T_WILDCARD
	| '*' { $$ = strdup("*"); }
	| '?' { $$ = strdup("?"); }
	;

wildcard_sort
	: wildcard {
		$$ = ld_wildcard_alloc(ld);
		$$->lw_name = $1;
		$$->lw_sort = LWS_NONE;
	}
	| T_SORT_BY_NAME '(' wildcard ')' {
		$$ = ld_wildcard_alloc(ld);
		$$->lw_name = $3;
		$$->lw_sort = LWS_NAME;
	}
	| T_SORT_BY_NAME '(' T_SORT_BY_NAME '(' wildcard ')' ')' {
		$$ = ld_wildcard_alloc(ld);
		$$->lw_name = $5;
		$$->lw_sort = LWS_NAME;
	}
	| T_SORT_BY_NAME '(' T_SORT_BY_ALIGNMENT '(' wildcard ')' ')' {
		$$ = ld_wildcard_alloc(ld);
		$$->lw_name = $5;
		$$->lw_sort = LWS_NAME_ALIGN;
	}
	| T_SORT_BY_ALIGNMENT '(' wildcard ')' {
		$$ = ld_wildcard_alloc(ld);
		$$->lw_name = $3;
		$$->lw_sort = LWS_ALIGN;
	}
	| T_SORT_BY_ALIGNMENT '(' T_SORT_BY_NAME '(' wildcard ')' ')' {
		$$ = ld_wildcard_alloc(ld);
		$$->lw_name = $5;
		$$->lw_sort = LWS_ALIGN_NAME;
	}
	| T_SORT_BY_ALIGNMENT '(' T_SORT_BY_ALIGNMENT '(' wildcard ')' ')' {
		$$ = ld_wildcard_alloc(ld);
		$$->lw_name = $5;
		$$->lw_sort = LWS_ALIGN;
	}
	;

ident_list
	: ident { $$ = ld_script_list(ld, NULL, $1); }
	| ident_list separator ident { $$ = ld_script_list(ld, $1, $3); }
	;

ident_list_nosep
	: ident { $$ = ld_script_list(ld, NULL, $1); }
	| ident_list_nosep ident { $$ = ld_script_list(ld, $1, $2); }
	;

input_file_list
	: input_file { $$ = ld_script_list(ld, NULL, $1); }
	| input_file_list separator input_file { $$ = ld_script_list(ld, $1, $3); }
	;

input_file
	: ident { $$ = ld_script_input_file(ld, 0, $1); }
	| as_needed_list { $$ = ld_script_input_file(ld, 1, $1); }
	;

as_needed_list
	: T_AS_NEEDED '(' ident_list ')' { $$ = $3; }
	;

wildcard_list
	: wildcard_sort { $$ = ld_script_list(ld, NULL, $1); }
	| wildcard_list wildcard_sort { $$ = ld_script_list(ld, $1, $2); }
	;

separator
	: ','
	|
	;

%%

/* ARGSUSED */
static void
yyerror(const char *s)
{

	(void) s;
	errx(1, "Syntax error in ld script, line %d\n", lineno);
}

static void
_init_script(void)
{

	STAILQ_INIT(&ldss_c);
	STAILQ_INIT(&ldso_c);
}

void
ld_script_parse(const char *name)
{

	_init_script();

	if ((yyin = fopen(name, "r")) == NULL)
		ld_fatal_std(ld, "fopen %s name failed", name);
	if (yyparse() < 0)
		ld_fatal(ld, "unable to parse linker script %s", name);
}

void
ld_script_parse_internal(void)
{
	YY_BUFFER_STATE b;

	_init_script();

	b = yy_scan_string(ldscript_default);
	if (yyparse() < 0)
		ld_fatal(ld, "unable to parse internal linker script");
	yy_delete_buffer(b);
}
