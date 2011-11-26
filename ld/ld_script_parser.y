%{
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
#include "ld_file.h"

ELFTC_VCSID("$Id$");

struct yy_buffer_state;
typedef struct yy_buffer_state *YY_BUFFER_STATE;

/*
 * Name list.
 */
struct _list {
	char		*str;
	struct _list	*next;
};

extern int yylex(void);
extern int yyparse(void);
extern YY_BUFFER_STATE yy_scan_string(char *yy_str);
extern void yy_delete_buffer(YY_BUFFER_STATE b);
extern int lineno;
extern FILE *yyin;
extern struct ld *ld;
extern char *ldscript_default;

static void yyerror(const char *s);
static struct _list *_make_list(struct _list *list, char *str);

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
%token T_COMMONPAGESIZE
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
%token T_MAXPAGESIZE
%token T_MEMORY
%token T_MIN
%token T_NEXT
%token T_NOCROSSREFS
%token T_NOFLOAT
%token T_ONLY_IF_RO
%token T_ONLY_IF_RW
%token T_OPTION
%token T_ORIGIN
%token T_OUTPUT
%token T_OUTPUT_ARCH
%token T_OUTPUT_FORMAT
%token T_OVERLAY
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
%token <str> T_MEMORY_ATTR
%token <str> T_IDENT
%token <str> T_WILDCARD
%token <str> T_STRING

%type <str> ident
%type <str> wildcard
%type <str> wildcard_sort
%type <str> variable
%type <list> ident_list
%type <list> ident_list_nosep
%type <list> wildcard_list

%union {
	struct _list *list;
	char *str;
	intmax_t num;
}

%%

script
	: ldscript
	|
	;

ldscript
	: ldscript_statement
	| ldscript ldscript_statement
	;

ldscript_statement
	: ldscript_assignment
	| ldscript_command
	| ';'
	;

ldscript_assignment
	: simple_assignment
	| provide_assignment
	| provide_hidden_assignment
	;

simple_assignment
	: variable assign_op expression %prec '='
	;

provide_assignment
	: T_PROVIDE '(' variable '=' expression ')'
	;

provide_hidden_assignment
	: T_PROVIDE_HIDDEN '(' variable '=' expression ')'
	;

assign_op
	: T_LSHIFT_E
	| T_RSHIFT_E
	| T_ADD_E
	| T_SUB_E
	| T_MUL_E
	| T_DIV_E
	| T_AND_E
	| T_OR_E
	| '='
	;

expression
	: expression '+' expression
	| expression '-' expression
	| expression '*' expression
	| expression '/' expression
	| expression '%' expression
	| expression '&' expression
	| expression '|' expression
	| expression '>' expression
	| expression '<' expression
	| expression T_EQ expression
	| expression T_NE expression
	| expression T_GE expression
	| expression T_LE expression
	| expression T_LSHIFT expression
	| expression T_RSHIFT expression 
	| expression T_LOGICAL_AND expression
	| expression T_LOGICAL_OR expression
	| '!' expression %prec UNARY
	| '-' expression %prec UNARY
	| '~' expression %prec UNARY
	| expression '?' expression ':' expression
	| simple_assignment
	| function
	| constant
	| variable
	| '(' expression ')'
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
	: T_ABSOLUTE '(' expression ')'
	;

addr_function
	: T_ADDR '(' ident ')'
	;

align_function
	: T_ALIGN '(' expression ')'
	| T_ALIGN '(' expression ',' T_NUM ')'
	;

alignof_function
	: T_ALIGNOF '(' ident ')'
	;

block_function
	: T_BLOCK '(' expression ')'
	;

data_segment_align_function
	: T_DATA_SEGMENT_ALIGN '(' T_NUM ',' T_NUM ')'
	;

data_segment_end_function
	: T_DATA_SEGMENT_END '(' expression ')'
	;

data_segment_relro_end_function
	: T_DATA_SEGMENT_RELRO_END '(' T_NUM ',' ident ')'
	;

defined_function
	: T_DEFINED '(' ident ')'
	;

length_function
	: T_LENGTH '(' ident ')'
	;

loadaddr_function
	: T_LOADADDR '(' ident ')'
	;

max_function
	: T_MAX '(' expression ',' expression ')'
	;

min_function
	: T_MIN '(' expression ',' expression ')'
	;

next_function
	: T_NEXT '(' expression ')'
	;

origin_function
	: T_ORIGIN '(' ident ')'
	;

segment_start_function
	: T_SEGMENT_START '(' ident ',' T_NUM ')'
	;

sizeof_function
	: T_SIZEOF '(' ident ')'
	;

sizeof_headers_function
	: T_SIZEOF_HEADERS
	;

constant
	: T_NUM
	| symbolic_constant
	;

symbolic_constant
	: T_CONSTANT '(' T_COMMONPAGESIZE ')'
	| T_CONSTANT '(' T_MAXPAGESIZE ')'
	;

ldscript_command
	: assert_command
	| as_needed_command
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

assert_command
	: T_ASSERT '(' expression ',' T_STRING ')'
	;

as_needed_command
	: T_AS_NEEDED '(' ident_list ')'
	;

entry_command
	: T_ENTRY '(' ident ')'
	;

extern_command
	: T_EXTERN '(' ident_list_nosep ')'
	;

force_common_allocation_command
	: T_FORCE_COMMON_ALLOCATION
	;

group_command
	: T_GROUP '(' ident_list ')'
	;

inhibit_common_allocation_command
	: T_INHIBIT_COMMON_ALLOCATION
	;

input_command
	: T_INPUT '(' ident_list ')'
	;

memory_command
	: T_MEMORY '{' memory_region_list '}'
	;

memory_region_list
	: memory_region
	| memory_region_list memory_region
	;

memory_region
	: ident memory_attr ':' T_ORIGIN '=' T_NUM ',' T_LENGTH '=' T_NUM
	;

memory_attr
	: T_MEMORY_ATTR
	|
	;

nocrossrefs_command
	: T_NOCROSSREFS '(' ident_list_nosep ')'
	;

output_command
	: T_OUTPUT '(' ident ')'
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
	: phdr
	| phdr phdr_list

phdr
	: ident ident phdr_filehdr phdr_phdrs phdr_at phdr_flags ';'
	;

phdr_filehdr
	: T_FILEHDR
	|
	;

phdr_phdrs
	: T_PHDRS
	|
	;

phdr_at
	: T_AT '(' T_NUM ')'
	|
	;

phdr_flags
	: T_FLAGS '(' T_NUM ')'
	|
	;

region_alias_command
	: T_REGION_ALIAS '(' ident ',' ident ')'
	;

search_dir_command
	: T_SEARCH_DIR '(' ident ')'
	{ ld_file_add_library_path(ld, $3); }
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
	| ldscript_assignment
	| output_sections_desc
	| overlay_desc
	| ';'
	;

output_sections_desc
	: ident output_section_addr output_section_type ':'
	output_section_lma
	output_section_align
	output_section_subalign
	output_section_constraint
	'{' output_section_command_list '}'
	output_section_region
	output_section_lma_region
	output_section_phdr
	output_section_fillexp
	;

output_section_addr
	: expression
	|
	;

output_section_type
	: '(' ident_unquoted ')'
	|
	;

output_section_lma
	: T_AT '(' expression ')'
	|
	;

output_section_align
	: T_ALIGN '(' expression ')'
	|
	;

output_section_subalign
	: T_SUBALIGN '(' expression ')'
	|
	;

output_section_constraint
	: T_ONLY_IF_RO
	| T_ONLY_IF_RW
	|
	;

output_section_command_list
	: output_section_command
	| output_section_command_list output_section_command
	;

output_section_command
	: ldscript_assignment
	| input_section_desc
	| output_section_data
	| output_section_keywords
	| ';'
	;

input_section_desc
	: wildcard_sort input_section
	| T_KEEP '(' wildcard_sort input_section ')'
	;

input_section
	: '(' input_file_exclude_list wildcard_list ')'
	|
	;

input_file_exclude_list
	: T_EXCLUDE_FILE '(' wildcard_list ')'
	|
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
	: '>' ident
	|
	;

output_section_lma_region
	: T_AT '>' ident
	|
	;

output_section_phdr
	: output_section_phdr ':' ident
	|
	;

output_section_fillexp
	: '=' expression
	|
	;

overlay_desc
	: T_OVERLAY
	overlay_vma ':'
	overlay_nocref
	overlay_lma
	'{' overlay_section_list '}'
	overlay_region
	overlay_phdr
	overlay_fill
	;

overlay_vma
	: expression
	|
	;

overlay_nocref
	: T_NOCROSSREFS
	|
	;

overlay_lma
	: T_AT '(' expression ')'
	|
	;

overlay_section_list
	: overlay_section
	| overlay_section_list overlay_section
	;

overlay_section
	: ident
	'{' output_section_command_list '}'
	overlay_section_phdr
	overlay_section_fill
	;

overlay_section_phdr
	: overlay_section_phdr ':' ident
	|
	;

overlay_section_fill
	: '=' expression
	|
	;

overlay_region
	: '>' ident
	|
	;

overlay_phdr
	: overlay_phdr ':' ident
	|
	;

overlay_fill
	: '=' expression
	|
	;

startup_command
	: T_STARTUP '(' ident ')'
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

ident_unquoted
	: T_IDENT
	;

variable
	: ident
	| '.'  { $$ = strdup("."); }
	;

wildcard
	: ident
	| T_WILDCARD
	| '*' { $$ = strdup("*"); }
	| '?' { $$ = strdup("?"); }
	;

wildcard_sort
	: wildcard
	| T_SORT_BY_NAME '(' wildcard ')' {
		$$ = $3;
	}
	| T_SORT_BY_NAME '(' T_SORT_BY_NAME '(' wildcard ')' ')' {
		$$ = $5;
	}
	| T_SORT_BY_NAME '(' T_SORT_BY_ALIGNMENT '(' wildcard ')' ')' {
		$$ = $5;
	}
	| T_SORT_BY_ALIGNMENT '(' wildcard ')' {
		$$ = $3;
	}
	| T_SORT_BY_ALIGNMENT '(' T_SORT_BY_NAME '(' wildcard ')' ')' {
		$$ = $5;
	}
	| T_SORT_BY_ALIGNMENT '(' T_SORT_BY_ALIGNMENT '(' wildcard ')' ')' {
		$$ = $5;
	}
	;

ident_list
	: ident { $$ = _make_list(NULL, $1); }
	| ident_list separator ident { $$ = _make_list($1, $3); }
	;

ident_list_nosep
	: ident { $$ = _make_list(NULL, $1); }
	| ident_list_nosep ident { $$ = _make_list($1, $2); }
	;

wildcard_list
	: wildcard_sort { $$ = _make_list(NULL, $1); }
	| wildcard_list wildcard_sort { $$ = _make_list($1, $2); }
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

static struct _list *
_make_list(struct _list *list, char *str)
{
	struct _list *l;

	l = malloc(sizeof(*l));
	if (l == NULL)
		err(1, "malloc");
	l->str = str;
	l->next = list;

	return (l);
}

void
ld_script_parse(const char *name)
{

	if ((yyin = fopen(name, "r")) == NULL)
		ld_fatal_std(ld, "fopen %s name failed", name);
	if (yyparse() < 0)
		ld_fatal(ld, "unable to parse linker script %s", name);
}

void
ld_script_parse_internal(void)
{
	YY_BUFFER_STATE b;

	b = yy_scan_string(ldscript_default);
	if (yyparse() < 0)
		ld_fatal(ld, "unable to parse internal linker script");
	yy_delete_buffer(b);
}
