/*
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*      @(#)rpc_parse.h  1.3  90/08/29  (C) 1987 SMI   */

/*
 * rpc_parse.h, Definitions for the RPCL parser 
 */

enum defkind {
	DEF_CONST,
	DEF_STRUCT,
	DEF_UNION,
	DEF_ENUM,
	DEF_TYPEDEF,
	DEF_PROGRAM
};
typedef enum defkind defkind;

typedef char *const_def;

enum relation {
	REL_VECTOR,	/* fixed length array */
	REL_ARRAY,	/* variable length array */
	REL_POINTER,	/* pointer */
	REL_ALIAS,	/* simple */
};
typedef enum relation relation;

struct typedef_def {
	char *old_prefix;
	char *old_type;
	relation rel;
	char *array_max;
};
typedef struct typedef_def typedef_def;

struct enumval_list {
	char *name;
	char *assignment;
	struct enumval_list *next;
};
typedef struct enumval_list enumval_list;

struct enum_def {
	enumval_list *vals;
};
typedef struct enum_def enum_def;

struct declaration {
	char *prefix;
	char *type;
	char *name;
	relation rel;
	char *array_max;
};
typedef struct declaration declaration;

struct decl_list {
	declaration decl;
	struct decl_list *next;
};
typedef struct decl_list decl_list;

struct struct_def {
	decl_list *decls;
};
typedef struct struct_def struct_def;

struct case_list {
	char *case_name;
	int contflag;
	declaration case_decl;
	struct case_list *next;
};
typedef struct case_list case_list;

struct union_def {
	declaration enum_decl;
	case_list *cases;
	declaration *default_decl;
};
typedef struct union_def union_def;

struct arg_list {
	char *argname; /* name of struct for arg*/
	decl_list *decls;
};
	
typedef struct arg_list arg_list;

struct proc_list {
	char *proc_name;
	char *proc_num;
	arg_list args;
	int arg_num;
	char *res_type;
	char *res_prefix;
	struct proc_list *next;
};
typedef struct proc_list proc_list;

struct version_list {
	char *vers_name;
	char *vers_num;
	proc_list *procs;
	struct version_list *next;
};
typedef struct version_list version_list;

struct program_def {
	char *prog_num;
	version_list *versions;
};
typedef struct program_def program_def;

struct definition {
	char *def_name;
	defkind def_kind;
	union {
		const_def co;
		struct_def st;
		union_def un;
		enum_def en;
		typedef_def ty;
		program_def pr;
	} def;
};
typedef struct definition definition;

definition *get_definition();


struct bas_type
{
  char *name;
  int length;
  struct bas_type *next;
};

typedef struct bas_type bas_type;
