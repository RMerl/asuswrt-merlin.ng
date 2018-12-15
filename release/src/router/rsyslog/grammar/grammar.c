/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 29 "grammar.y" /* yacc.c:339  */

#define IN_GRAMMAR_Y /* tell parserif.h not to redefine things! */

#include "config.h"
#include <stdio.h>
#include <libestr.h>
#include "rainerscript.h"
#include "parserif.h"
#define YYDEBUG 1
extern int yylineno;
extern char *yytext;

/* keep compile rule clean of errors */
extern int yylex(void);
extern int yyerror(const char*);

#line 83 "grammar.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
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
#line 46 "grammar.y" /* yacc.c:355  */

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

#line 237 "grammar.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 254 "grammar.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   462

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  80
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  161

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   304

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    54,    51,     2,
      60,    61,    52,    49,    62,    50,     2,    53,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    59,
       2,    58,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    63,     2,    64,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,     2,    57,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    55
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   133,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   146,   150,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   171,   175,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   191,   192,   193,   194,   195,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NAME", "FUNC", "BEGINOBJ", "ENDOBJ",
  "BEGIN_INCLUDE", "BEGIN_ACTION", "BEGIN_PROPERTY", "BEGIN_CONSTANT",
  "BEGIN_TPL", "BEGIN_RULESET", "STOP", "SET", "RESET", "UNSET",
  "CONTINUE", "CALL", "CALL_INDIRECT", "LEGACY_ACTION", "LEGACY_RULESET",
  "PRIFILT", "PROPFILT", "BSD_TAG_SELECTOR", "BSD_HOST_SELECTOR",
  "RELOAD_LOOKUP_TABLE_PROCEDURE", "IF", "THEN", "ELSE", "FOREACH",
  "ITERATOR_ASSIGNMENT", "DO", "OR", "AND", "NOT", "VAR", "STRING",
  "NUMBER", "CMP_EQ", "CMP_NE", "CMP_LE", "CMP_GE", "CMP_LT", "CMP_GT",
  "CMP_CONTAINS", "CMP_CONTAINSI", "CMP_STARTSWITH", "CMP_STARTSWITHI",
  "'+'", "'-'", "'&'", "'*'", "'/'", "'%'", "UMINUS", "'{'", "'}'", "'='",
  "';'", "'('", "')'", "','", "'['", "']'", "$accept", "conf", "include",
  "obj", "propconst", "property", "constant", "nvlst", "nv", "value",
  "script", "stmt", "block", "actlst", "s_act", "expr", "fparams", "array",
  "iterator_decl", "arrayelt", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    43,
      45,    38,    42,    47,    37,   304,   123,   125,    61,    59,
      40,    41,    44,    91,    93
};
# endif

#define YYPACT_NINF -51

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-51)))

#define YYTABLE_NINF -20

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -51,   230,   -51,     6,   -51,   -51,   -51,   -51,   -51,    14,
      17,    18,   -51,    20,    57,   -51,   -51,   206,   206,   -51,
     -51,   -50,    57,   -32,   -51,   -51,   -51,   -17,   -51,    10,
      15,    26,    27,    41,    -7,     1,    32,   -51,     0,    57,
     -51,   -51,   -51,    57,    57,    25,   295,   -51,   -51,   416,
     -51,   -51,   -51,    57,   322,    61,    66,     7,    45,   -51,
     -51,   -51,   -51,    44,    48,    57,    57,   -51,    52,   -51,
     -51,   249,   -51,   -45,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,   -51,   134,   -51,   225,    38,   206,    70,   206,
     -51,   -26,   -51,   158,   344,   366,   -51,    47,   -51,    68,
     -51,   408,   408,   -11,   -11,   -11,   -11,   -11,   -11,   -11,
     -11,   -11,   -11,    11,    11,    11,   -51,   -51,   -51,   -51,
     -51,    57,   -51,    80,    57,   -51,   -51,   -51,   -51,    -2,
     -51,   182,   -51,   -51,   -51,   -51,   -51,   206,   272,   -51,
     -51,   -51,   -51,   -51,   -51,   -51,   -51,    42,    43,   -51,
     -51
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,    37,    19,    19,    19,    19,    44,     0,
       0,     0,    47,     0,     0,    43,     5,     0,     0,     6,
       7,     0,     0,     0,    36,     3,     4,    26,    40,     0,
       0,     0,     0,     0,     0,     0,     0,    45,     0,     0,
      73,    72,    71,     0,     0,     0,     0,    74,    37,     0,
      38,    33,    34,     0,     0,     0,     0,     0,     0,     9,
      20,     8,    42,    10,     0,     0,     0,    32,     0,    50,
      68,     0,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    24,    75,     0,     0,     0,     0,
      41,     0,    14,     0,     0,     0,    69,     0,    67,     0,
      77,    49,    48,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    62,    63,    61,    64,    65,    66,    39,
      25,     0,    35,    27,     0,    29,    22,    21,    23,     0,
      13,     0,    31,    30,    70,    80,    76,     0,     0,    19,
      19,    11,    15,    16,    12,    28,    78,     0,     0,    17,
      18
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -51,   -51,   -51,   -51,   -51,   -51,   -51,    -4,   -51,   -51,
       8,     3,   -13,   -51,    53,    -8,   -46,    13,   -51,   -51
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    24,    25,   139,   152,   153,    29,    60,   137,
      93,    50,    51,    27,    28,    95,    96,    47,    56,    73
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,    31,    32,    33,    26,    52,    46,   149,   150,   -19,
      53,   136,   -19,    58,    54,     5,    59,   109,    58,   110,
       8,    61,   107,    37,    12,    13,    14,    15,    55,    58,
      58,    69,    62,    63,    57,    70,    71,    45,    86,    87,
      88,    89,    90,    91,    58,    58,    58,    64,   159,   160,
      34,    65,    94,    35,    36,   151,    38,   104,   105,    66,
      68,    38,    72,    89,    90,    91,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   133,   146,   135,    39,    40,    41,
      42,    67,    39,    40,    41,    42,   130,    98,    99,   132,
     102,   134,    43,   101,   103,   145,    94,    43,   144,   147,
     100,   141,    44,   106,   138,    45,     0,    44,     0,     0,
      45,     0,     0,     0,     0,     0,   148,     0,     0,     0,
       0,     0,     0,     0,   155,     0,     0,     0,     0,    48,
       0,     4,     5,     0,   130,   157,   158,     8,     9,    10,
      11,    12,    13,    14,    15,     0,    17,    18,     0,     0,
      21,    22,     0,    48,    23,     4,     5,     0,     0,     0,
       0,     8,     9,    10,    11,    12,    13,    14,    15,     0,
      17,    18,     0,     0,    21,    22,     0,    48,    23,     4,
       5,   129,     0,     0,     0,     8,     9,    10,    11,    12,
      13,    14,    15,     0,    17,    18,     0,     0,    21,    22,
       0,    48,    23,     4,     5,   140,     0,     0,     0,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    17,    18,
       2,     0,    21,    22,     0,     3,    23,     4,     5,   154,
       0,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    74,    75,
      23,     0,    49,     0,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,     0,    74,    75,     0,     0,     0,   131,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,    74,    75,     0,     0,     0,
     108,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,    74,    75,
       0,     0,     0,   156,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      97,     0,     0,     0,    92,    74,    75,     0,     0,     0,
       0,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    74,    75,     0,
       0,     0,     0,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    74,
      75,     0,     0,   142,     0,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    48,     0,     4,     5,   143,     0,     0,     0,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    17,    18,
       0,     0,    21,    22,     0,     0,    23,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91
};

static const yytype_int16 yycheck[] =
{
       4,     5,     6,     7,     1,    18,    14,     9,    10,     3,
      60,    37,     6,     3,    22,     8,     6,    62,     3,    64,
      13,     6,    68,     3,    17,    18,    19,    20,    60,     3,
       3,    39,     6,     6,    51,    43,    44,    63,    49,    50,
      51,    52,    53,    54,     3,     3,     3,     6,     6,     6,
      36,    58,    49,    36,    36,    57,     4,    65,    66,    58,
      60,     4,    37,    52,    53,    54,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    97,   131,    99,    35,    36,    37,
      38,    59,    35,    36,    37,    38,    93,    36,    32,    61,
      56,    31,    50,    58,    56,    37,   103,    50,    61,    29,
      57,   103,    60,    61,   101,    63,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,     5,
      -1,     7,     8,    -1,   141,   149,   150,    13,    14,    15,
      16,    17,    18,    19,    20,    -1,    22,    23,    -1,    -1,
      26,    27,    -1,     5,    30,     7,     8,    -1,    -1,    -1,
      -1,    13,    14,    15,    16,    17,    18,    19,    20,    -1,
      22,    23,    -1,    -1,    26,    27,    -1,     5,    30,     7,
       8,    57,    -1,    -1,    -1,    13,    14,    15,    16,    17,
      18,    19,    20,    -1,    22,    23,    -1,    -1,    26,    27,
      -1,     5,    30,     7,     8,    57,    -1,    -1,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    -1,    22,    23,
       0,    -1,    26,    27,    -1,     5,    30,     7,     8,    57,
      -1,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    33,    34,
      30,    -1,    56,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    -1,    33,    34,    -1,    -1,    -1,    62,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    33,    34,    -1,    -1,    -1,
      61,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    33,    34,
      -1,    -1,    -1,    61,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      28,    -1,    -1,    -1,    59,    33,    34,    -1,    -1,    -1,
      -1,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    33,    34,    -1,
      -1,    -1,    -1,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    33,
      34,    -1,    -1,    59,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,     5,    -1,     7,     8,    59,    -1,    -1,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    -1,    22,    23,
      -1,    -1,    26,    27,    -1,    -1,    30,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    66,     0,     5,     7,     8,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    30,    67,    68,    76,    78,    79,    72,
      72,    72,    72,    72,    36,    36,    36,     3,     4,    35,
      36,    37,    38,    50,    60,    63,    80,    82,     5,    56,
      76,    77,    77,    60,    80,    60,    83,    51,     3,     6,
      73,     6,     6,     6,     6,    58,    58,    59,    60,    80,
      80,    80,    37,    84,    33,    34,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    59,    75,    76,    80,    81,    28,    36,    32,
      79,    58,    56,    56,    80,    80,    61,    81,    61,    62,
      64,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    57,
      76,    62,    61,    77,    31,    77,    37,    74,    82,    69,
      57,    75,    59,    59,    61,    37,    81,    29,    80,     9,
      10,    57,    70,    71,    57,    77,    61,    72,    72,     6,
       6
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    66,    66,    66,    66,    66,    67,    68,
      68,    68,    68,    68,    69,    69,    69,    70,    71,    72,
      72,    73,    74,    74,    75,    75,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    77,    77,
      78,    78,    79,    79,    79,    79,    79,    79,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    81,    81,    82,    83,    84,
      84
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     2,     3,     3,
       3,     6,     6,     5,     0,     2,     2,     3,     3,     0,
       2,     3,     1,     1,     1,     2,     1,     4,     6,     4,
       5,     5,     3,     2,     2,     4,     1,     1,     1,     3,
       1,     3,     3,     1,     1,     2,     3,     1,     3,     3,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       4,     1,     1,     1,     1,     1,     3,     3,     5,     1,
       3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 134 "grammar.y" /* yacc.c:1646  */
    { cnfDoObj((yyvsp[0].obj)); }
#line 1508 "grammar.c" /* yacc.c:1646  */
    break;

  case 4:
#line 135 "grammar.y" /* yacc.c:1646  */
    { cnfDoScript((yyvsp[0].stmt)); }
#line 1514 "grammar.c" /* yacc.c:1646  */
    break;

  case 5:
#line 136 "grammar.y" /* yacc.c:1646  */
    { cnfDoCfsysline((yyvsp[0].s)); }
#line 1520 "grammar.c" /* yacc.c:1646  */
    break;

  case 6:
#line 137 "grammar.y" /* yacc.c:1646  */
    { cnfDoBSDTag((yyvsp[0].s)); }
#line 1526 "grammar.c" /* yacc.c:1646  */
    break;

  case 7:
#line 138 "grammar.y" /* yacc.c:1646  */
    { cnfDoBSDHost((yyvsp[0].s)); }
#line 1532 "grammar.c" /* yacc.c:1646  */
    break;

  case 8:
#line 139 "grammar.y" /* yacc.c:1646  */
    { includeProcessCnf((yyvsp[-1].nvlst)); }
#line 1538 "grammar.c" /* yacc.c:1646  */
    break;

  case 9:
#line 140 "grammar.y" /* yacc.c:1646  */
    { (yyval.obj) = cnfobjNew((yyvsp[-2].objType), (yyvsp[-1].nvlst)); }
#line 1544 "grammar.c" /* yacc.c:1646  */
    break;

  case 10:
#line 141 "grammar.y" /* yacc.c:1646  */
    { (yyval.obj) = cnfobjNew(CNFOBJ_TPL, (yyvsp[-1].nvlst)); }
#line 1550 "grammar.c" /* yacc.c:1646  */
    break;

  case 11:
#line 143 "grammar.y" /* yacc.c:1646  */
    { (yyval.obj) = cnfobjNew(CNFOBJ_TPL, (yyvsp[-4].nvlst));
					  (yyval.obj)->subobjs = (yyvsp[-1].objlst);
					}
#line 1558 "grammar.c" /* yacc.c:1646  */
    break;

  case 12:
#line 147 "grammar.y" /* yacc.c:1646  */
    { (yyval.obj) = cnfobjNew(CNFOBJ_RULESET, (yyvsp[-4].nvlst));
					  (yyval.obj)->script = (yyvsp[-1].stmt);
					}
#line 1566 "grammar.c" /* yacc.c:1646  */
    break;

  case 13:
#line 151 "grammar.y" /* yacc.c:1646  */
    { (yyval.obj) = cnfobjNew(CNFOBJ_RULESET, (yyvsp[-3].nvlst));
					  (yyval.obj)->script = NULL;
					}
#line 1574 "grammar.c" /* yacc.c:1646  */
    break;

  case 14:
#line 154 "grammar.y" /* yacc.c:1646  */
    { (yyval.objlst) = NULL; }
#line 1580 "grammar.c" /* yacc.c:1646  */
    break;

  case 15:
#line 155 "grammar.y" /* yacc.c:1646  */
    { (yyval.objlst) = objlstAdd((yyvsp[-1].objlst), (yyvsp[0].obj)); }
#line 1586 "grammar.c" /* yacc.c:1646  */
    break;

  case 16:
#line 156 "grammar.y" /* yacc.c:1646  */
    { (yyval.objlst) = objlstAdd((yyvsp[-1].objlst), (yyvsp[0].obj)); }
#line 1592 "grammar.c" /* yacc.c:1646  */
    break;

  case 17:
#line 157 "grammar.y" /* yacc.c:1646  */
    { (yyval.obj) = cnfobjNew(CNFOBJ_PROPERTY, (yyvsp[-1].nvlst)); }
#line 1598 "grammar.c" /* yacc.c:1646  */
    break;

  case 18:
#line 158 "grammar.y" /* yacc.c:1646  */
    { (yyval.obj) = cnfobjNew(CNFOBJ_CONSTANT, (yyvsp[-1].nvlst)); }
#line 1604 "grammar.c" /* yacc.c:1646  */
    break;

  case 19:
#line 159 "grammar.y" /* yacc.c:1646  */
    { (yyval.nvlst) = NULL; }
#line 1610 "grammar.c" /* yacc.c:1646  */
    break;

  case 20:
#line 160 "grammar.y" /* yacc.c:1646  */
    { (yyvsp[0].nvlst)->next = (yyvsp[-1].nvlst); (yyval.nvlst) = (yyvsp[0].nvlst); }
#line 1616 "grammar.c" /* yacc.c:1646  */
    break;

  case 21:
#line 161 "grammar.y" /* yacc.c:1646  */
    { (yyval.nvlst) = nvlstSetName((yyvsp[0].nvlst), (yyvsp[-2].estr)); }
#line 1622 "grammar.c" /* yacc.c:1646  */
    break;

  case 22:
#line 162 "grammar.y" /* yacc.c:1646  */
    { (yyval.nvlst) = nvlstNewStr((yyvsp[0].estr)); }
#line 1628 "grammar.c" /* yacc.c:1646  */
    break;

  case 23:
#line 163 "grammar.y" /* yacc.c:1646  */
    { (yyval.nvlst) = nvlstNewArray((yyvsp[0].arr)); }
#line 1634 "grammar.c" /* yacc.c:1646  */
    break;

  case 24:
#line 164 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 1640 "grammar.c" /* yacc.c:1646  */
    break;

  case 25:
#line 165 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = scriptAddStmt((yyvsp[-1].stmt), (yyvsp[0].stmt)); }
#line 1646 "grammar.c" /* yacc.c:1646  */
    break;

  case 26:
#line 166 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 1652 "grammar.c" /* yacc.c:1646  */
    break;

  case 27:
#line 167 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNew(S_IF);
					  (yyval.stmt)->d.s_if.expr = (yyvsp[-2].expr);
					  (yyval.stmt)->d.s_if.t_then = (yyvsp[0].stmt);
					  (yyval.stmt)->d.s_if.t_else = NULL; }
#line 1661 "grammar.c" /* yacc.c:1646  */
    break;

  case 28:
#line 171 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNew(S_IF);
					  (yyval.stmt)->d.s_if.expr = (yyvsp[-4].expr);
					  (yyval.stmt)->d.s_if.t_then = (yyvsp[-2].stmt);
					  (yyval.stmt)->d.s_if.t_else = (yyvsp[0].stmt); }
#line 1670 "grammar.c" /* yacc.c:1646  */
    break;

  case 29:
#line 175 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNew(S_FOREACH);
					  (yyval.stmt)->d.s_foreach.iter = (yyvsp[-2].itr);
					  (yyval.stmt)->d.s_foreach.body = (yyvsp[0].stmt);}
#line 1678 "grammar.c" /* yacc.c:1646  */
    break;

  case 30:
#line 178 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewSet((yyvsp[-3].s), (yyvsp[-1].expr), 1); }
#line 1684 "grammar.c" /* yacc.c:1646  */
    break;

  case 31:
#line 179 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewSet((yyvsp[-3].s), (yyvsp[-1].expr), 0); }
#line 1690 "grammar.c" /* yacc.c:1646  */
    break;

  case 32:
#line 180 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewUnset((yyvsp[-1].s)); }
#line 1696 "grammar.c" /* yacc.c:1646  */
    break;

  case 33:
#line 181 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewPRIFILT((yyvsp[-1].s), (yyvsp[0].stmt)); }
#line 1702 "grammar.c" /* yacc.c:1646  */
    break;

  case 34:
#line 182 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewPROPFILT((yyvsp[-1].s), (yyvsp[0].stmt)); }
#line 1708 "grammar.c" /* yacc.c:1646  */
    break;

  case 35:
#line 183 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewReloadLookupTable((yyvsp[-1].fparams));}
#line 1714 "grammar.c" /* yacc.c:1646  */
    break;

  case 36:
#line 184 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = NULL; }
#line 1720 "grammar.c" /* yacc.c:1646  */
    break;

  case 37:
#line 185 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = NULL; parser_errmsg("declarative object '%s' not permitted in action block [stmt]", yytext);}
#line 1726 "grammar.c" /* yacc.c:1646  */
    break;

  case 38:
#line 186 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 1732 "grammar.c" /* yacc.c:1646  */
    break;

  case 39:
#line 187 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[-1].stmt); }
#line 1738 "grammar.c" /* yacc.c:1646  */
    break;

  case 40:
#line 188 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[0].stmt); }
#line 1744 "grammar.c" /* yacc.c:1646  */
    break;

  case 41:
#line 189 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = scriptAddStmt((yyvsp[-2].stmt), (yyvsp[0].stmt)); }
#line 1750 "grammar.c" /* yacc.c:1646  */
    break;

  case 42:
#line 191 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewAct((yyvsp[-1].nvlst)); }
#line 1756 "grammar.c" /* yacc.c:1646  */
    break;

  case 43:
#line 192 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewLegaAct((yyvsp[0].s)); }
#line 1762 "grammar.c" /* yacc.c:1646  */
    break;

  case 44:
#line 193 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNew(S_STOP); }
#line 1768 "grammar.c" /* yacc.c:1646  */
    break;

  case 45:
#line 194 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewCall((yyvsp[0].estr)); }
#line 1774 "grammar.c" /* yacc.c:1646  */
    break;

  case 46:
#line 195 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNew(S_CALL_INDIRECT);
					  (yyval.stmt)->d.s_call_ind.expr = (yyvsp[-1].expr);
					}
#line 1782 "grammar.c" /* yacc.c:1646  */
    break;

  case 47:
#line 198 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = cnfstmtNewContinue(); }
#line 1788 "grammar.c" /* yacc.c:1646  */
    break;

  case 48:
#line 199 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(AND, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1794 "grammar.c" /* yacc.c:1646  */
    break;

  case 49:
#line 200 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(OR, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1800 "grammar.c" /* yacc.c:1646  */
    break;

  case 50:
#line 201 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(NOT, NULL, (yyvsp[0].expr)); }
#line 1806 "grammar.c" /* yacc.c:1646  */
    break;

  case 51:
#line 202 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_EQ, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1812 "grammar.c" /* yacc.c:1646  */
    break;

  case 52:
#line 203 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_NE, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1818 "grammar.c" /* yacc.c:1646  */
    break;

  case 53:
#line 204 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_LE, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1824 "grammar.c" /* yacc.c:1646  */
    break;

  case 54:
#line 205 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_GE, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1830 "grammar.c" /* yacc.c:1646  */
    break;

  case 55:
#line 206 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_LT, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1836 "grammar.c" /* yacc.c:1646  */
    break;

  case 56:
#line 207 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_GT, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1842 "grammar.c" /* yacc.c:1646  */
    break;

  case 57:
#line 208 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_CONTAINS, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1848 "grammar.c" /* yacc.c:1646  */
    break;

  case 58:
#line 209 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_CONTAINSI, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1854 "grammar.c" /* yacc.c:1646  */
    break;

  case 59:
#line 210 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_STARTSWITH, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1860 "grammar.c" /* yacc.c:1646  */
    break;

  case 60:
#line 211 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew(CMP_STARTSWITHI, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1866 "grammar.c" /* yacc.c:1646  */
    break;

  case 61:
#line 212 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew('&', (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1872 "grammar.c" /* yacc.c:1646  */
    break;

  case 62:
#line 213 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew('+', (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1878 "grammar.c" /* yacc.c:1646  */
    break;

  case 63:
#line 214 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew('-', (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1884 "grammar.c" /* yacc.c:1646  */
    break;

  case 64:
#line 215 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew('*', (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1890 "grammar.c" /* yacc.c:1646  */
    break;

  case 65:
#line 216 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew('/', (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1896 "grammar.c" /* yacc.c:1646  */
    break;

  case 66:
#line 217 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew('%', (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1902 "grammar.c" /* yacc.c:1646  */
    break;

  case 67:
#line 218 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 1908 "grammar.c" /* yacc.c:1646  */
    break;

  case 68:
#line 219 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = cnfexprNew('M', NULL, (yyvsp[0].expr)); }
#line 1914 "grammar.c" /* yacc.c:1646  */
    break;

  case 69:
#line 220 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = (struct cnfexpr*) cnffuncNew((yyvsp[-2].estr), NULL); }
#line 1920 "grammar.c" /* yacc.c:1646  */
    break;

  case 70:
#line 221 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = (struct cnfexpr*) cnffuncNew((yyvsp[-3].estr), (yyvsp[-1].fparams)); }
#line 1926 "grammar.c" /* yacc.c:1646  */
    break;

  case 71:
#line 222 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = (struct cnfexpr*) cnfnumvalNew((yyvsp[0].n)); }
#line 1932 "grammar.c" /* yacc.c:1646  */
    break;

  case 72:
#line 223 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = (struct cnfexpr*) cnfstringvalNew((yyvsp[0].estr)); }
#line 1938 "grammar.c" /* yacc.c:1646  */
    break;

  case 73:
#line 224 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = (struct cnfexpr*) cnfvarNew((yyvsp[0].s)); }
#line 1944 "grammar.c" /* yacc.c:1646  */
    break;

  case 74:
#line 225 "grammar.y" /* yacc.c:1646  */
    { (yyval.expr) = (struct cnfexpr*) (yyvsp[0].arr); }
#line 1950 "grammar.c" /* yacc.c:1646  */
    break;

  case 75:
#line 226 "grammar.y" /* yacc.c:1646  */
    { (yyval.fparams) = cnffparamlstNew((yyvsp[0].expr), NULL); }
#line 1956 "grammar.c" /* yacc.c:1646  */
    break;

  case 76:
#line 227 "grammar.y" /* yacc.c:1646  */
    { (yyval.fparams) = cnffparamlstNew((yyvsp[-2].expr), (yyvsp[0].fparams)); }
#line 1962 "grammar.c" /* yacc.c:1646  */
    break;

  case 77:
#line 228 "grammar.y" /* yacc.c:1646  */
    { (yyval.arr) = (yyvsp[-1].arr); }
#line 1968 "grammar.c" /* yacc.c:1646  */
    break;

  case 78:
#line 229 "grammar.y" /* yacc.c:1646  */
    { (yyval.itr) = cnfNewIterator((yyvsp[-3].s), (yyvsp[-1].expr)); }
#line 1974 "grammar.c" /* yacc.c:1646  */
    break;

  case 79:
#line 230 "grammar.y" /* yacc.c:1646  */
    { (yyval.arr) = cnfarrayNew((yyvsp[0].estr)); }
#line 1980 "grammar.c" /* yacc.c:1646  */
    break;

  case 80:
#line 231 "grammar.y" /* yacc.c:1646  */
    { (yyval.arr) = cnfarrayAdd((yyvsp[-2].arr), (yyvsp[0].estr)); }
#line 1986 "grammar.c" /* yacc.c:1646  */
    break;


#line 1990 "grammar.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 233 "grammar.y" /* yacc.c:1906  */

/*
int yyerror(char *s)
{
	printf("parse failure on or before line %d: %s\n", yylineno, s);
	return 0;
}
*/
