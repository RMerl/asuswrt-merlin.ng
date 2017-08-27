/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     SELECT = 259,
     FROM = 260,
     WHERE = 261,
     COMMA = 262,
     QUOTE = 263,
     EQUALS = 264,
     NOTEQUALS = 265,
     LESS = 266,
     GREATER = 267,
     LESSEQUALS = 268,
     GREATEREQUALS = 269,
     AND = 270,
     OR = 271,
     EOL = 272,
     END = 273,
     IS = 274,
     NOT = 275,
     SQLNULL = 276
   };
#endif
/* Tokens.  */
#define STRING 258
#define SELECT 259
#define FROM 260
#define WHERE 261
#define COMMA 262
#define QUOTE 263
#define EQUALS 264
#define NOTEQUALS 265
#define LESS 266
#define GREATER 267
#define LESSEQUALS 268
#define GREATEREQUALS 269
#define AND 270
#define OR 271
#define EOL 272
#define END 273
#define IS 274
#define NOT 275
#define SQLNULL 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
	char* v_string;
}
/* Line 1489 of yacc.c.  */
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE sslval;

