/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_GRAMMAR_H_INCLUDED
# define YY_YY_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NAME = 258,
    FUNC = 259,
    BEGINOBJ = 260,
    ENDOBJ = 261,
    BEGIN_INCLUDE = 262,
    BEGIN_ACTION = 263,
    BEGIN_PROPERTY = 264,
    BEGIN_CONSTANT = 265,
    BEGIN_TPL = 266,
    BEGIN_RULESET = 267,
    STOP = 268,
    SET = 269,
    RESET = 270,
    UNSET = 271,
    CONTINUE = 272,
    CALL = 273,
    CALL_INDIRECT = 274,
    LEGACY_ACTION = 275,
    LEGACY_RULESET = 276,
    PRIFILT = 277,
    PROPFILT = 278,
    BSD_TAG_SELECTOR = 279,
    BSD_HOST_SELECTOR = 280,
    RELOAD_LOOKUP_TABLE_PROCEDURE = 281,
    IF = 282,
    THEN = 283,
    ELSE = 284,
    FOREACH = 285,
    ITERATOR_ASSIGNMENT = 286,
    DO = 287,
    OR = 288,
    AND = 289,
    NOT = 290,
    VAR = 291,
    STRING = 292,
    NUMBER = 293,
    CMP_EQ = 294,
    CMP_NE = 295,
    CMP_LE = 296,
    CMP_GE = 297,
    CMP_LT = 298,
    CMP_GT = 299,
    CMP_CONTAINS = 300,
    CMP_CONTAINSI = 301,
    CMP_STARTSWITH = 302,
    CMP_STARTSWITHI = 303,
    UMINUS = 304
  };
#endif
/* Tokens.  */
#define NAME 258
#define FUNC 259
#define BEGINOBJ 260
#define ENDOBJ 261
#define BEGIN_INCLUDE 262
#define BEGIN_ACTION 263
#define BEGIN_PROPERTY 264
#define BEGIN_CONSTANT 265
#define BEGIN_TPL 266
#define BEGIN_RULESET 267
#define STOP 268
#define SET 269
#define RESET 270
#define UNSET 271
#define CONTINUE 272
#define CALL 273
#define CALL_INDIRECT 274
#define LEGACY_ACTION 275
#define LEGACY_RULESET 276
#define PRIFILT 277
#define PROPFILT 278
#define BSD_TAG_SELECTOR 279
#define BSD_HOST_SELECTOR 280
#define RELOAD_LOOKUP_TABLE_PROCEDURE 281
#define IF 282
#define THEN 283
#define ELSE 284
#define FOREACH 285
#define ITERATOR_ASSIGNMENT 286
#define DO 287
#define OR 288
#define AND 289
#define NOT 290
#define VAR 291
#define STRING 292
#define NUMBER 293
#define CMP_EQ 294
#define CMP_NE 295
#define CMP_LE 296
#define CMP_GE 297
#define CMP_LT 298
#define CMP_GT 299
#define CMP_CONTAINS 300
#define CMP_CONTAINSI 301
#define CMP_STARTSWITH 302
#define CMP_STARTSWITHI 303
#define UMINUS 304

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 46 "grammar.y" /* yacc.c:1909  */

	char *s;
	long long n;
	es_str_t *estr;
	enum cnfobjType objType;
	struct cnfobj *obj;
	struct cnfstmt *stmt;
	struct nvlst *nvlst;
	struct objlst *objlst;
	struct cnfexpr *expr;
	struct cnfarray *arr;
	struct cnffunc *func;
	struct cnffparamlst *fparams;
	struct cnfitr *itr;

#line 168 "grammar.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_H_INCLUDED  */
