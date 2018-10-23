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
#line 1 "read_config_yy.y" /* yacc.c:339  */

/*
 * (C) 2006-2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Description: configuration file abstract grammar
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include "conntrackd.h"
#include "bitops.h"
#include "cidr.h"
#include "helper.h"
#include "stack.h"
#include <sched.h>
#include <dlfcn.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

extern char *yytext;
extern int   yylineno;

struct ct_conf conf;

static void __kernel_filter_start(void);
static void __kernel_filter_add_state(int value);
static void __max_dedicated_links_reached(void);

struct stack symbol_stack;

enum {
	SYMBOL_HELPER_QUEUE_NUM,
	SYMBOL_HELPER_QUEUE_LEN,
	SYMBOL_HELPER_POLICY_EXPECT_ROOT,
	SYMBOL_HELPER_EXPECT_POLICY_LEAF,
};


#line 123 "read_config_yy.c" /* yacc.c:339  */

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
#ifndef YY_YY_READ_CONFIG_YY_H_INCLUDED
# define YY_YY_READ_CONFIG_YY_H_INCLUDED
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
    T_IPV4_ADDR = 258,
    T_IPV4_IFACE = 259,
    T_PORT = 260,
    T_HASHSIZE = 261,
    T_HASHLIMIT = 262,
    T_MULTICAST = 263,
    T_PATH = 264,
    T_UNIX = 265,
    T_REFRESH = 266,
    T_IPV6_ADDR = 267,
    T_IPV6_IFACE = 268,
    T_BACKLOG = 269,
    T_GROUP = 270,
    T_IGNORE = 271,
    T_LOG = 272,
    T_UDP = 273,
    T_ICMP = 274,
    T_IGMP = 275,
    T_VRRP = 276,
    T_TCP = 277,
    T_LOCK = 278,
    T_BUFFER_SIZE_MAX_GROWN = 279,
    T_EXPIRE = 280,
    T_TIMEOUT = 281,
    T_GENERAL = 282,
    T_SYNC = 283,
    T_STATS = 284,
    T_BUFFER_SIZE = 285,
    T_SYNC_MODE = 286,
    T_ALARM = 287,
    T_FTFW = 288,
    T_CHECKSUM = 289,
    T_WINDOWSIZE = 290,
    T_ON = 291,
    T_OFF = 292,
    T_FOR = 293,
    T_IFACE = 294,
    T_PURGE = 295,
    T_RESEND_QUEUE_SIZE = 296,
    T_ESTABLISHED = 297,
    T_SYN_SENT = 298,
    T_SYN_RECV = 299,
    T_FIN_WAIT = 300,
    T_CLOSE_WAIT = 301,
    T_LAST_ACK = 302,
    T_TIME_WAIT = 303,
    T_CLOSE = 304,
    T_LISTEN = 305,
    T_SYSLOG = 306,
    T_RCVBUFF = 307,
    T_SNDBUFF = 308,
    T_NOTRACK = 309,
    T_POLL_SECS = 310,
    T_FILTER = 311,
    T_ADDRESS = 312,
    T_PROTOCOL = 313,
    T_STATE = 314,
    T_ACCEPT = 315,
    T_FROM = 316,
    T_USERSPACE = 317,
    T_KERNELSPACE = 318,
    T_EVENT_ITER_LIMIT = 319,
    T_DEFAULT = 320,
    T_NETLINK_OVERRUN_RESYNC = 321,
    T_NICE = 322,
    T_IPV4_DEST_ADDR = 323,
    T_IPV6_DEST_ADDR = 324,
    T_SCHEDULER = 325,
    T_TYPE = 326,
    T_PRIO = 327,
    T_NETLINK_EVENTS_RELIABLE = 328,
    T_DISABLE_INTERNAL_CACHE = 329,
    T_DISABLE_EXTERNAL_CACHE = 330,
    T_ERROR_QUEUE_LENGTH = 331,
    T_OPTIONS = 332,
    T_TCP_WINDOW_TRACKING = 333,
    T_EXPECT_SYNC = 334,
    T_HELPER = 335,
    T_HELPER_QUEUE_NUM = 336,
    T_HELPER_QUEUE_LEN = 337,
    T_HELPER_POLICY = 338,
    T_HELPER_EXPECT_TIMEOUT = 339,
    T_HELPER_EXPECT_MAX = 340,
    T_SYSTEMD = 341,
    T_STARTUP_RESYNC = 342,
    T_IP = 343,
    T_PATH_VAL = 344,
    T_NUMBER = 345,
    T_SIGNED_NUMBER = 346,
    T_STRING = 347
  };
#endif
/* Tokens.  */
#define T_IPV4_ADDR 258
#define T_IPV4_IFACE 259
#define T_PORT 260
#define T_HASHSIZE 261
#define T_HASHLIMIT 262
#define T_MULTICAST 263
#define T_PATH 264
#define T_UNIX 265
#define T_REFRESH 266
#define T_IPV6_ADDR 267
#define T_IPV6_IFACE 268
#define T_BACKLOG 269
#define T_GROUP 270
#define T_IGNORE 271
#define T_LOG 272
#define T_UDP 273
#define T_ICMP 274
#define T_IGMP 275
#define T_VRRP 276
#define T_TCP 277
#define T_LOCK 278
#define T_BUFFER_SIZE_MAX_GROWN 279
#define T_EXPIRE 280
#define T_TIMEOUT 281
#define T_GENERAL 282
#define T_SYNC 283
#define T_STATS 284
#define T_BUFFER_SIZE 285
#define T_SYNC_MODE 286
#define T_ALARM 287
#define T_FTFW 288
#define T_CHECKSUM 289
#define T_WINDOWSIZE 290
#define T_ON 291
#define T_OFF 292
#define T_FOR 293
#define T_IFACE 294
#define T_PURGE 295
#define T_RESEND_QUEUE_SIZE 296
#define T_ESTABLISHED 297
#define T_SYN_SENT 298
#define T_SYN_RECV 299
#define T_FIN_WAIT 300
#define T_CLOSE_WAIT 301
#define T_LAST_ACK 302
#define T_TIME_WAIT 303
#define T_CLOSE 304
#define T_LISTEN 305
#define T_SYSLOG 306
#define T_RCVBUFF 307
#define T_SNDBUFF 308
#define T_NOTRACK 309
#define T_POLL_SECS 310
#define T_FILTER 311
#define T_ADDRESS 312
#define T_PROTOCOL 313
#define T_STATE 314
#define T_ACCEPT 315
#define T_FROM 316
#define T_USERSPACE 317
#define T_KERNELSPACE 318
#define T_EVENT_ITER_LIMIT 319
#define T_DEFAULT 320
#define T_NETLINK_OVERRUN_RESYNC 321
#define T_NICE 322
#define T_IPV4_DEST_ADDR 323
#define T_IPV6_DEST_ADDR 324
#define T_SCHEDULER 325
#define T_TYPE 326
#define T_PRIO 327
#define T_NETLINK_EVENTS_RELIABLE 328
#define T_DISABLE_INTERNAL_CACHE 329
#define T_DISABLE_EXTERNAL_CACHE 330
#define T_ERROR_QUEUE_LENGTH 331
#define T_OPTIONS 332
#define T_TCP_WINDOW_TRACKING 333
#define T_EXPECT_SYNC 334
#define T_HELPER 335
#define T_HELPER_QUEUE_NUM 336
#define T_HELPER_QUEUE_LEN 337
#define T_HELPER_POLICY 338
#define T_HELPER_EXPECT_TIMEOUT 339
#define T_HELPER_EXPECT_MAX 340
#define T_SYSTEMD 341
#define T_STARTUP_RESYNC 342
#define T_IP 343
#define T_PATH_VAL 344
#define T_NUMBER 345
#define T_SIGNED_NUMBER 346
#define T_STRING 347

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 58 "read_config_yy.y" /* yacc.c:355  */

	int		val;
	char		*string;

#line 352 "read_config_yy.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_READ_CONFIG_YY_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 369 "read_config_yy.c" /* yacc.c:358  */

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
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   331

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  95
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  91
/* YYNRULES -- Number of rules.  */
#define YYNRULES  229
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  367

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   347

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,    93,     2,    94,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    93,    93,    94,    97,    98,   101,   102,   103,   104,
     107,   112,   116,   121,   126,   131,   163,   168,   173,   178,
     183,   188,   202,   218,   219,   221,   240,   279,   298,   303,
     323,   329,   335,   341,   347,   353,   367,   383,   384,   386,
     397,   415,   426,   444,   459,   465,   471,   477,   483,   489,
     505,   523,   524,   526,   537,   555,   566,   584,   599,   605,
     611,   617,   623,   629,   635,   640,   645,   647,   648,   651,
     656,   661,   671,   672,   674,   675,   676,   677,   678,   679,
     680,   681,   682,   683,   684,   687,   689,   690,   693,   698,
     703,   715,   723,   735,   736,   738,   743,   748,   753,   758,
     759,   761,   762,   763,   764,   767,   768,   770,   771,   772,
     773,   774,   775,   778,   779,   781,   782,   783,   784,   785,
     788,   793,   798,   803,   808,   813,   818,   823,   828,   829,
     831,   839,   847,   855,   863,   871,   879,   887,   895,   904,
     906,   907,   910,   911,   912,   913,   914,   915,   916,   917,
     918,   919,   920,   921,   922,   923,   924,   925,   926,   927,
     930,   931,   933,   938,   943,   948,   953,   958,   963,   968,
     974,   976,   977,   980,   992,  1001,  1006,  1016,  1021,  1026,
    1031,  1032,  1034,  1043,  1056,  1057,  1059,  1078,  1097,  1116,
    1125,  1147,  1148,  1150,  1208,  1268,  1277,  1291,  1292,  1294,
    1296,  1306,  1307,  1310,  1311,  1312,  1313,  1316,  1321,  1325,
    1330,  1335,  1340,  1372,  1377,  1378,  1381,  1384,  1484,  1485,
    1488,  1491,  1502,  1513,  1534,  1535,  1538,  1539,  1542,  1557
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_IPV4_ADDR", "T_IPV4_IFACE", "T_PORT",
  "T_HASHSIZE", "T_HASHLIMIT", "T_MULTICAST", "T_PATH", "T_UNIX",
  "T_REFRESH", "T_IPV6_ADDR", "T_IPV6_IFACE", "T_BACKLOG", "T_GROUP",
  "T_IGNORE", "T_LOG", "T_UDP", "T_ICMP", "T_IGMP", "T_VRRP", "T_TCP",
  "T_LOCK", "T_BUFFER_SIZE_MAX_GROWN", "T_EXPIRE", "T_TIMEOUT",
  "T_GENERAL", "T_SYNC", "T_STATS", "T_BUFFER_SIZE", "T_SYNC_MODE",
  "T_ALARM", "T_FTFW", "T_CHECKSUM", "T_WINDOWSIZE", "T_ON", "T_OFF",
  "T_FOR", "T_IFACE", "T_PURGE", "T_RESEND_QUEUE_SIZE", "T_ESTABLISHED",
  "T_SYN_SENT", "T_SYN_RECV", "T_FIN_WAIT", "T_CLOSE_WAIT", "T_LAST_ACK",
  "T_TIME_WAIT", "T_CLOSE", "T_LISTEN", "T_SYSLOG", "T_RCVBUFF",
  "T_SNDBUFF", "T_NOTRACK", "T_POLL_SECS", "T_FILTER", "T_ADDRESS",
  "T_PROTOCOL", "T_STATE", "T_ACCEPT", "T_FROM", "T_USERSPACE",
  "T_KERNELSPACE", "T_EVENT_ITER_LIMIT", "T_DEFAULT",
  "T_NETLINK_OVERRUN_RESYNC", "T_NICE", "T_IPV4_DEST_ADDR",
  "T_IPV6_DEST_ADDR", "T_SCHEDULER", "T_TYPE", "T_PRIO",
  "T_NETLINK_EVENTS_RELIABLE", "T_DISABLE_INTERNAL_CACHE",
  "T_DISABLE_EXTERNAL_CACHE", "T_ERROR_QUEUE_LENGTH", "T_OPTIONS",
  "T_TCP_WINDOW_TRACKING", "T_EXPECT_SYNC", "T_HELPER",
  "T_HELPER_QUEUE_NUM", "T_HELPER_QUEUE_LEN", "T_HELPER_POLICY",
  "T_HELPER_EXPECT_TIMEOUT", "T_HELPER_EXPECT_MAX", "T_SYSTEMD",
  "T_STARTUP_RESYNC", "T_IP", "T_PATH_VAL", "T_NUMBER", "T_SIGNED_NUMBER",
  "T_STRING", "'{'", "'}'", "$accept", "configfile", "lines", "line",
  "logfile_bool", "logfile_path", "syslog_bool", "syslog_facility", "lock",
  "refreshtime", "expiretime", "timeout", "purge", "multicast_line",
  "multicast_options", "multicast_option", "udp_line", "udp_options",
  "udp_option", "tcp_line", "tcp_options", "tcp_option", "hashsize",
  "hashlimit", "unix_line", "unix_options", "unix_option", "sync",
  "sync_list", "sync_line", "option_line", "options", "option",
  "expect_list", "expect_item", "sync_mode_alarm", "sync_mode_ftfw",
  "sync_mode_notrack", "sync_mode_alarm_list", "sync_mode_alarm_line",
  "sync_mode_ftfw_list", "sync_mode_ftfw_line", "sync_mode_notrack_list",
  "sync_mode_notrack_line", "disable_internal_cache",
  "disable_external_cache", "resend_queue_size", "startup_resync",
  "window_size", "tcp_states", "tcp_state", "general", "general_list",
  "general_line", "systemd", "netlink_buffer_size",
  "netlink_buffer_size_max_grown", "netlink_overrun_resync",
  "netlink_events_reliable", "nice", "scheduler", "scheduler_options",
  "scheduler_line", "event_iterations_limit", "poll_secs", "filter",
  "filter_list", "filter_item", "filter_protocol_list",
  "filter_protocol_item", "filter_address_list", "filter_address_item",
  "filter_state_list", "filter_state_item", "stats", "stats_list",
  "stat_line", "stat_logfile_bool", "stat_logfile_path",
  "stat_syslog_bool", "stat_syslog_facility", "helper", "helper_list",
  "helper_line", "helper_type", "helper_type_list", "helper_type_line",
  "helper_policy_list", "helper_policy_line", "helper_policy_expect_max",
  "helper_policy_expect_timeout", YY_NULLPTR
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   123,   125
};
# endif

#define YYPACT_NINF -73

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-73)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      91,   -72,   -66,   -63,   -60,    38,    91,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   100,   118,
       6,   120,   -47,   -42,   -39,    53,   -38,   -20,    -4,    43,
      25,   -26,    35,    51,    68,    41,   174,   186,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -18,    64,
      20,    97,    71,    75,   172,    78,    79,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
      96,    47,   -73,   -73,   -73,   -73,   -73,   -73,     4,    94,
     103,   108,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   187,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     132,   -73,   -73,   139,   -73,   155,   -73,   -73,   -73,   175,
     176,   177,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     135,   -73,   -73,   178,    15,   179,   180,   119,   126,   -73,
       3,   -73,    59,   -73,    -3,   -73,   -73,   -73,   137,   138,
     -73,   128,   184,   -73,   -73,   -73,   -73,   -15,    -2,    45,
     -73,   -73,   173,   185,   -73,   -73,     7,   188,   189,   190,
     192,   193,   215,   194,   195,   197,   -73,   -73,    69,   196,
     198,   201,   217,   199,   200,   202,   205,   206,   -73,   -73,
       0,   207,   208,   209,   219,   204,   210,   211,   214,   216,
     213,   -73,   -73,    52,   134,   105,   221,   -11,   -73,   -73,
     212,    63,   -73,   -73,   124,   130,   218,   220,   222,   224,
     226,   228,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   230,   232,   223,   225,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   227,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     233,   234,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,    58,   125,   -73,   -73,     1,     5,    10,    22,
     231,   235,   -73,   -73,   -73,   -73,   -73,   -73,   238,   239,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     191,   -73,   -73,   -73,   -73,   257,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     0,     0,     0,     0,     3,     4,     7,     6,
       8,     9,   140,    72,   201,   214,     1,     5,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   139,   144,
     145,   147,   146,   148,   142,   143,   149,   141,   159,   150,
     151,   155,   156,   157,   158,   152,   153,   154,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    71,    74,    75,
      76,    77,    78,    79,    80,    73,    84,    81,    82,    83,
       0,     0,   200,   202,   203,   204,   205,   206,     0,     0,
       0,     0,   213,   215,   216,    64,    65,    67,    10,    11,
      12,    16,   163,   162,    13,    14,    15,   176,     0,   180,
     175,   164,   165,   166,   169,   171,   167,   168,   160,   161,
       0,    23,    17,     0,    37,     0,    51,    18,    19,     0,
       0,     0,    20,    86,   207,   208,   209,   210,   211,   212,
       0,   221,   222,     0,     0,     0,     0,     0,     0,    23,
       0,    37,     0,    51,     0,    99,   105,   113,     0,     0,
     224,     0,     0,    66,    68,   180,   180,     0,     0,     0,
     177,   181,     0,     0,   170,   172,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,    24,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    35,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,    52,     0,     0,     0,     0,     0,    85,    87,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   173,   174,    22,    25,    27,    26,    28,    30,
      33,    34,    29,    32,    31,    36,    39,    44,    40,    47,
      48,    43,    46,    45,    41,    42,    50,    53,    58,    54,
      61,    62,    57,    60,    59,    55,    56,    63,    96,   101,
     102,   103,   104,   100,     0,     0,     0,     0,    97,   108,
     109,   106,   111,   107,   112,   110,     0,    98,   115,   116,
     114,   117,   118,   119,    88,    89,    90,    91,    93,   218,
       0,     0,   223,   225,   226,   227,   178,   179,   191,   191,
     184,   184,   197,   197,   127,   124,   122,   123,   125,   126,
     120,   121,     0,     0,   229,   228,     0,     0,     0,     0,
     128,   128,    95,    92,    94,   217,   220,   219,     0,     0,
     190,   192,   189,   188,   187,   186,   183,   185,   182,   196,
       0,   198,   195,   193,   194,     0,   132,   130,   131,   133,
     134,   135,   136,   137,   138,   129,   199
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -73,   -73,   -73,   275,   -73,   -73,   -73,   -73,   -73,    86,
      93,    29,    32,   -73,   133,   -73,   -73,   156,   -73,   -73,
     157,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,    99,   -73,   113,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   101,   -73,    19,   -73,
      -1,   -73,    18,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -14,   -73,   -73,   -73,   -73,   -73,
     -73
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,     7,    39,    40,    41,    42,    43,    68,
      69,    70,    71,    72,   150,   187,    73,   152,   199,    74,
     154,   212,    44,    45,    46,   144,   164,     8,    19,    75,
      76,   158,   219,   322,   334,    77,    78,    79,   213,   273,
     214,   281,   215,   290,   291,   282,   283,   284,   285,   350,
     365,     9,    18,    47,    48,    49,    50,    51,    52,    53,
      54,   148,   175,    55,    56,    57,   147,   171,   328,   347,
     326,   341,   330,   351,    10,    20,    83,    84,    85,    86,
      87,    11,    21,    93,    94,   323,   337,   221,   303,   304,
     305
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
     201,   226,   202,   201,   338,   202,   177,   178,   338,   203,
     177,   178,   203,   339,   228,   179,   180,   339,   181,   179,
     180,    12,   181,    80,   161,   296,   297,    13,   343,   162,
      14,   204,   344,    15,   204,   108,   205,   182,    16,   205,
     343,   182,   183,    95,   344,   227,   183,   120,    96,   206,
     207,   101,   206,   207,    97,   184,   185,    81,   229,   184,
     185,   230,   189,    59,   190,   208,   209,   109,   208,   209,
     102,   191,   189,   210,   190,   121,   210,    62,    63,   104,
     105,   191,   298,   137,   138,   123,   103,   111,   112,    98,
      99,   211,    65,   192,   256,   340,   140,   186,   193,   342,
      82,   234,   345,   192,   346,   231,    22,    23,   193,   163,
      24,   194,   195,   124,   345,   107,   348,    25,     1,     2,
       3,   194,   195,    26,    27,   110,    58,   196,   197,    59,
      28,    63,   134,   135,   115,   106,    60,   196,   197,   139,
      61,   113,   100,    62,    63,    65,   268,   300,   301,    64,
     332,    29,   333,   198,   122,    30,    31,   302,    65,   114,
      63,   127,   125,   245,    32,   128,    33,    34,   132,   274,
      35,     4,   133,    36,    65,   275,   167,   168,   169,   286,
     276,   167,   168,   169,   141,   136,    37,   167,   168,   169,
     126,    88,   277,   142,    38,    66,    88,   172,   173,   287,
     143,    89,    90,    91,   129,   130,    89,    90,    91,   276,
     116,   117,    67,   170,    92,   216,   217,   222,   306,   335,
     174,   277,   118,   119,   307,   149,   131,   159,   278,   355,
     220,   218,   151,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   271,   279,   288,   272,   280,   289,   153,   145,
     146,   240,   241,   249,   250,   260,   261,   294,   295,   316,
     317,   318,   319,   320,   321,   232,   224,   225,   155,   156,
     157,   160,   165,   166,   223,   233,   235,   236,   237,   366,
     238,    17,   176,   239,   246,   243,   242,   244,   247,   248,
     252,   251,   253,   254,   255,   257,   262,   259,   258,   269,
     263,   264,   265,   267,   266,   299,   270,   188,   327,   336,
     200,   308,     0,   309,   292,   310,     0,   311,     0,   312,
     314,   313,   315,   324,   325,   349,   353,   354,   293,   352,
     329,   331
};

static const yytype_int16 yycheck[] =
{
       3,    16,     5,     3,     3,     5,     3,     4,     3,    12,
       3,     4,    12,    12,    16,    12,    13,    12,    15,    12,
      13,    93,    15,    17,     9,    36,    37,    93,    18,    14,
      93,    34,    22,    93,    34,    61,    39,    34,     0,    39,
      18,    34,    39,    90,    22,    60,    39,    65,    90,    52,
      53,    89,    52,    53,    93,    52,    53,    51,    60,    52,
      53,    16,     3,    11,     5,    68,    69,    93,    68,    69,
      90,    12,     3,    76,     5,    93,    76,    25,    26,    36,
      37,    12,    93,    36,    37,    65,    90,    36,    37,    36,
      37,    94,    40,    34,    94,    94,    92,    94,    39,    94,
      94,    94,    92,    34,    94,    60,     6,     7,    39,    94,
      10,    52,    53,    93,    92,    90,    94,    17,    27,    28,
      29,    52,    53,    23,    24,    90,     8,    68,    69,    11,
      30,    26,    36,    37,    93,    92,    18,    68,    69,    92,
      22,    90,    89,    25,    26,    40,    94,    84,    85,    31,
      92,    51,    94,    94,    90,    55,    56,    94,    40,    91,
      26,    90,    65,    94,    64,    90,    66,    67,    90,    35,
      70,    80,    93,    73,    40,    41,    57,    58,    59,    74,
      75,    57,    58,    59,    90,    89,    86,    57,    58,    59,
      93,    71,    87,    90,    94,    77,    71,    71,    72,    94,
      92,    81,    82,    83,    32,    33,    81,    82,    83,    75,
      36,    37,    94,    94,    94,    78,    79,    89,    94,    94,
      94,    87,    36,    37,    94,    93,    54,    92,    94,    38,
      92,    94,    93,    42,    43,    44,    45,    46,    47,    48,
      49,    50,   213,   214,   215,   213,   214,   215,    93,    62,
      63,    36,    37,    36,    37,    36,    37,    36,    37,    36,
      37,    36,    37,    36,    37,    92,   165,   166,    93,    93,
      93,    93,    93,    93,    90,    90,    88,    88,    88,    22,
      88,     6,   149,    90,    88,    90,    92,    90,    90,    88,
      90,    92,    90,    88,    88,    88,    92,    88,    90,   213,
      90,    90,    88,    90,    88,    93,   213,   151,   309,   323,
     153,    93,    -1,    93,   215,    93,    -1,    93,    -1,    93,
      90,    93,    90,    90,    90,    94,    88,    88,   215,    94,
     311,   313
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    27,    28,    29,    80,    96,    97,    98,   122,   146,
     169,   176,    93,    93,    93,    93,     0,    98,   147,   123,
     170,   177,     6,     7,    10,    17,    23,    24,    30,    51,
      55,    56,    64,    66,    67,    70,    73,    86,    94,    99,
     100,   101,   102,   103,   117,   118,   119,   148,   149,   150,
     151,   152,   153,   154,   155,   158,   159,   160,     8,    11,
      18,    22,    25,    26,    31,    40,    77,    94,   104,   105,
     106,   107,   108,   111,   114,   124,   125,   130,   131,   132,
      17,    51,    94,   171,   172,   173,   174,   175,    71,    81,
      82,    83,    94,   178,   179,    90,    90,    93,    36,    37,
      89,    89,    90,    90,    36,    37,    92,    90,    61,    93,
      90,    36,    37,    90,    91,    93,    36,    37,    36,    37,
      65,    93,    90,    65,    93,    65,    93,    90,    90,    32,
      33,    54,    90,    93,    36,    37,    89,    36,    37,    92,
      92,    90,    90,    92,   120,    62,    63,   161,   156,    93,
     109,    93,   112,    93,   115,    93,    93,    93,   126,    92,
      93,     9,    14,    94,   121,    93,    93,    57,    58,    59,
      94,   162,    71,    72,    94,   157,   109,     3,     4,    12,
      13,    15,    34,    39,    52,    53,    94,   110,   112,     3,
       5,    12,    34,    39,    52,    53,    68,    69,    94,   113,
     115,     3,     5,    12,    34,    39,    52,    53,    68,    69,
      76,    94,   116,   133,   135,   137,    78,    79,    94,   127,
      92,   182,    89,    90,   161,   161,    16,    60,    16,    60,
      16,    60,    92,    90,    94,    88,    88,    88,    88,    90,
      36,    37,    92,    90,    90,    94,    88,    90,    88,    36,
      37,    92,    90,    90,    88,    88,    94,    88,    90,    88,
      36,    37,    92,    90,    90,    88,    88,    90,    94,   104,
     105,   106,   107,   134,    35,    41,    75,    87,    94,   106,
     107,   136,   140,   141,   142,   143,    74,    94,   106,   107,
     138,   139,   140,   142,    36,    37,    36,    37,    93,    93,
      84,    85,    94,   183,   184,   185,    94,    94,    93,    93,
      93,    93,    93,    93,    90,    90,    36,    37,    36,    37,
      36,    37,   128,   180,    90,    90,   165,   165,   163,   163,
     167,   167,    92,    94,   129,    94,   179,   181,     3,    12,
      94,   166,    94,    18,    22,    92,    94,   164,    94,    94,
     144,   168,    94,    88,    88,    38,    42,    43,    44,    45,
      46,    47,    48,    49,    50,   145,    22
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    95,    96,    96,    97,    97,    98,    98,    98,    98,
      99,    99,   100,   101,   101,   102,   103,   104,   105,   106,
     107,   108,   108,   109,   109,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   111,   111,   112,   112,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   114,
     114,   115,   115,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   117,   118,   119,   120,   120,   121,
     121,   122,   123,   123,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   125,   126,   126,   127,   127,
     127,   127,   127,   128,   128,   129,   130,   131,   132,   133,
     133,   134,   134,   134,   134,   135,   135,   136,   136,   136,
     136,   136,   136,   137,   137,   138,   138,   138,   138,   138,
     139,   139,   140,   140,   141,   142,   142,   143,   144,   144,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   146,
     147,   147,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     149,   149,   150,   151,   152,   152,   152,   153,   153,   154,
     155,   156,   156,   157,   157,   158,   159,   160,   160,   160,
     161,   161,   162,   162,   163,   163,   164,   164,   164,   162,
     162,   165,   165,   166,   166,   162,   162,   167,   167,   168,
     169,   170,   170,   171,   171,   171,   171,   172,   172,   173,
     174,   174,   175,   176,   177,   177,   178,   179,   180,   180,
     181,   179,   179,   179,   182,   182,   183,   183,   184,   185
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     4,     5,     0,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     4,     5,     0,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       5,     0,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     4,     0,     2,     2,
       2,     4,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     0,     2,     2,     2,
       2,     2,     4,     0,     2,     1,     5,     5,     5,     0,
       2,     1,     1,     1,     1,     0,     2,     1,     1,     1,
       1,     1,     1,     0,     2,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       4,     0,     2,     2,     2,     2,     2,     4,     6,     6,
       0,     2,     5,     5,     0,     2,     1,     1,     1,     5,
       5,     0,     2,     2,     2,     5,     5,     0,     2,     3,
       4,     0,     2,     1,     1,     1,     1,     2,     2,     2,
       2,     2,     2,     4,     0,     2,     1,     7,     0,     2,
       1,     2,     2,     5,     0,     2,     1,     1,     2,     2
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
        case 10:
#line 108 "read_config_yy.y" /* yacc.c:1646  */
    {
	strncpy(conf.logfile, DEFAULT_LOGFILE, FILENAME_MAXLEN);
}
#line 1756 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 11:
#line 113 "read_config_yy.y" /* yacc.c:1646  */
    {
}
#line 1763 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 12:
#line 117 "read_config_yy.y" /* yacc.c:1646  */
    {
	strncpy(conf.logfile, (yyvsp[0].string), FILENAME_MAXLEN);
}
#line 1771 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 13:
#line 122 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.syslog_facility = DEFAULT_SYSLOG_FACILITY;
}
#line 1779 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 14:
#line 127 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.syslog_facility = -1;
}
#line 1787 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 15:
#line 132 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (!strcmp((yyvsp[0].string), "daemon"))
		conf.syslog_facility = LOG_DAEMON;
	else if (!strcmp((yyvsp[0].string), "local0"))
		conf.syslog_facility = LOG_LOCAL0;
	else if (!strcmp((yyvsp[0].string), "local1"))
		conf.syslog_facility = LOG_LOCAL1;
	else if (!strcmp((yyvsp[0].string), "local2"))
		conf.syslog_facility = LOG_LOCAL2;
	else if (!strcmp((yyvsp[0].string), "local3"))
		conf.syslog_facility = LOG_LOCAL3;
	else if (!strcmp((yyvsp[0].string), "local4"))
		conf.syslog_facility = LOG_LOCAL4;
	else if (!strcmp((yyvsp[0].string), "local5"))
		conf.syslog_facility = LOG_LOCAL5;
	else if (!strcmp((yyvsp[0].string), "local6"))
		conf.syslog_facility = LOG_LOCAL6;
	else if (!strcmp((yyvsp[0].string), "local7"))
		conf.syslog_facility = LOG_LOCAL7;
	else {
		dlog(LOG_WARNING, "'%s' is not a known syslog facility, "
		     "ignoring", (yyvsp[0].string));
		break;
	}

	if (conf.stats.syslog_facility != -1 &&
	    conf.syslog_facility != conf.stats.syslog_facility)
		dlog(LOG_WARNING, "conflicting Syslog facility "
		     "values, defaulting to General");
}
#line 1822 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 16:
#line 164 "read_config_yy.y" /* yacc.c:1646  */
    {
	strncpy(conf.lockfile, (yyvsp[0].string), FILENAME_MAXLEN);
}
#line 1830 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 17:
#line 169 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.refresh = (yyvsp[0].val);
}
#line 1838 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 18:
#line 174 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.cache_timeout = (yyvsp[0].val);
}
#line 1846 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 19:
#line 179 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.commit_timeout = (yyvsp[0].val);
}
#line 1854 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 20:
#line 184 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.purge_timeout = (yyvsp[0].val);
}
#line 1862 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 21:
#line 189 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_MCAST) {
		dlog(LOG_ERR, "cannot use `Multicast' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_MCAST;
	conf.channel[conf.channel_num].channel_type = CHANNEL_MCAST;
	conf.channel[conf.channel_num].channel_flags = CHANNEL_F_BUFFERED;
	conf.channel_num++;
}
#line 1879 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 22:
#line 203 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_MCAST) {
		dlog(LOG_ERR, "cannot use `Multicast' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_MCAST;
	conf.channel[conf.channel_num].channel_type = CHANNEL_MCAST;
	conf.channel[conf.channel_num].channel_flags = CHANNEL_F_DEFAULT |
						       CHANNEL_F_BUFFERED;
	conf.channel_default = conf.channel_num;
	conf.channel_num++;
}
#line 1898 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 25:
#line 222 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();

	if (!inet_aton((yyvsp[0].string), &conf.channel[conf.channel_num].u.mcast.in)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		break;
	}

        if (conf.channel[conf.channel_num].u.mcast.ipproto == AF_INET6) {
		dlog(LOG_WARNING, "your multicast address is IPv4 but "
		     "is binded to an IPv6 interface? "
		     "Surely, this is not what you want");
		break;
	}

	conf.channel[conf.channel_num].u.mcast.ipproto = AF_INET;
}
#line 1920 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 26:
#line 241 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string),
			&conf.channel[conf.channel_num].u.mcast.in);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	if (conf.channel[conf.channel_num].u.mcast.ipproto == AF_INET) {
		dlog(LOG_WARNING, "your multicast address is IPv6 but "
		     "is binded to an IPv4 interface? "
		     "Surely this is not what you want");
		break;
	}

	conf.channel[conf.channel_num].u.mcast.ipproto = AF_INET6;

	if (conf.channel[conf.channel_num].channel_ifname[0] &&
	    !conf.channel[conf.channel_num].u.mcast.ifa.interface_index6) {
		unsigned int idx;

		idx = if_nametoindex((yyvsp[0].string));
		if (!idx) {
			dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
			break;
		}

		conf.channel[conf.channel_num].u.mcast.ifa.interface_index6 = idx;
		conf.channel[conf.channel_num].u.mcast.ipproto = AF_INET6;
	}
}
#line 1962 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 27:
#line 280 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();

	if (!inet_aton((yyvsp[0].string), &conf.channel[conf.channel_num].u.mcast.ifa)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		break;
	}

        if (conf.channel[conf.channel_num].u.mcast.ipproto == AF_INET6) {
		dlog(LOG_WARNING, "your multicast interface is IPv4 but "
		     "is binded to an IPv6 interface? "
		     "Surely, this is not what you want");
		break;
	}

	conf.channel[conf.channel_num].u.mcast.ipproto = AF_INET;
}
#line 1984 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 28:
#line 299 "read_config_yy.y" /* yacc.c:1646  */
    {
	dlog(LOG_WARNING, "`IPv6_interface' not required, ignoring");
}
#line 1992 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 29:
#line 304 "read_config_yy.y" /* yacc.c:1646  */
    {
	unsigned int idx;

	__max_dedicated_links_reached();

	strncpy(conf.channel[conf.channel_num].channel_ifname, (yyvsp[0].string), IFNAMSIZ);

	idx = if_nametoindex((yyvsp[0].string));
	if (!idx) {
		dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
		break;
	}

	if (conf.channel[conf.channel_num].u.mcast.ipproto == AF_INET6) {
		conf.channel[conf.channel_num].u.mcast.ifa.interface_index6 = idx;
		conf.channel[conf.channel_num].u.mcast.ipproto = AF_INET6;
	}
}
#line 2015 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 30:
#line 324 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.mcast.port = (yyvsp[0].val);
}
#line 2024 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 31:
#line 330 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.mcast.sndbuf = (yyvsp[0].val);
}
#line 2033 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 32:
#line 336 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.mcast.rcvbuf = (yyvsp[0].val);
}
#line 2042 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 33:
#line 342 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.mcast.checksum = 0;
}
#line 2051 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 34:
#line 348 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.mcast.checksum = 1;
}
#line 2060 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 35:
#line 354 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_UDP) {
		dlog(LOG_ERR, "cannot use `UDP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_UDP;
	conf.channel[conf.channel_num].channel_type = CHANNEL_UDP;
	conf.channel[conf.channel_num].channel_flags = CHANNEL_F_BUFFERED;
	conf.channel_num++;
}
#line 2077 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 36:
#line 368 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_UDP) {
		dlog(LOG_ERR, "cannot use `UDP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_UDP;
	conf.channel[conf.channel_num].channel_type = CHANNEL_UDP;
	conf.channel[conf.channel_num].channel_flags = CHANNEL_F_DEFAULT |
						       CHANNEL_F_BUFFERED;
	conf.channel_default = conf.channel_num;
	conf.channel_num++;
}
#line 2096 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 39:
#line 387 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();

	if (!inet_aton((yyvsp[0].string), &conf.channel[conf.channel_num].u.udp.server.ipv4)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		break;
	}
	conf.channel[conf.channel_num].u.udp.ipproto = AF_INET;
}
#line 2110 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 40:
#line 398 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string),
			&conf.channel[conf.channel_num].u.udp.server.ipv6);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	conf.channel[conf.channel_num].u.udp.ipproto = AF_INET6;
}
#line 2131 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 41:
#line 416 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();

	if (!inet_aton((yyvsp[0].string), &conf.channel[conf.channel_num].u.udp.client)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		break;
	}
	conf.channel[conf.channel_num].u.udp.ipproto = AF_INET;
}
#line 2145 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 42:
#line 427 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string),
			&conf.channel[conf.channel_num].u.udp.client);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		break;
	} else {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	conf.channel[conf.channel_num].u.udp.ipproto = AF_INET6;
}
#line 2166 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 43:
#line 445 "read_config_yy.y" /* yacc.c:1646  */
    {
	int idx;

	__max_dedicated_links_reached();
	strncpy(conf.channel[conf.channel_num].channel_ifname, (yyvsp[0].string), IFNAMSIZ);

	idx = if_nametoindex((yyvsp[0].string));
	if (!idx) {
		dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
		break;
	}
	conf.channel[conf.channel_num].u.udp.server.ipv6.scope_id = idx;
}
#line 2184 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 44:
#line 460 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.udp.port = (yyvsp[0].val);
}
#line 2193 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 45:
#line 466 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.udp.sndbuf = (yyvsp[0].val);
}
#line 2202 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 46:
#line 472 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.udp.rcvbuf = (yyvsp[0].val);
}
#line 2211 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 47:
#line 478 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.udp.checksum = 0;
}
#line 2220 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 48:
#line 484 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.udp.checksum = 1;
}
#line 2229 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 49:
#line 490 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_TCP) {
		dlog(LOG_ERR, "cannot use `TCP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_TCP;
	conf.channel[conf.channel_num].channel_type = CHANNEL_TCP;
	conf.channel[conf.channel_num].channel_flags = CHANNEL_F_BUFFERED |
						       CHANNEL_F_STREAM |
						       CHANNEL_F_ERRORS;
	conf.channel_num++;
}
#line 2248 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 50:
#line 506 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.channel_type_global != CHANNEL_NONE &&
	    conf.channel_type_global != CHANNEL_TCP) {
		dlog(LOG_ERR, "cannot use `TCP' with other "
		     "dedicated link protocols!");
		exit(EXIT_FAILURE);
	}
	conf.channel_type_global = CHANNEL_TCP;
	conf.channel[conf.channel_num].channel_type = CHANNEL_TCP;
	conf.channel[conf.channel_num].channel_flags = CHANNEL_F_DEFAULT |
						       CHANNEL_F_BUFFERED |
						       CHANNEL_F_STREAM |
						       CHANNEL_F_ERRORS;
	conf.channel_default = conf.channel_num;
	conf.channel_num++;
}
#line 2269 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 53:
#line 527 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();

	if (!inet_aton((yyvsp[0].string), &conf.channel[conf.channel_num].u.tcp.server.ipv4)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		break;
	}
	conf.channel[conf.channel_num].u.tcp.ipproto = AF_INET;
}
#line 2283 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 54:
#line 538 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string),
			&conf.channel[conf.channel_num].u.tcp.server.ipv6);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	conf.channel[conf.channel_num].u.tcp.ipproto = AF_INET6;
}
#line 2304 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 55:
#line 556 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();

	if (!inet_aton((yyvsp[0].string), &conf.channel[conf.channel_num].u.tcp.client)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4 address", (yyvsp[0].string));
		break;
	}
	conf.channel[conf.channel_num].u.tcp.ipproto = AF_INET;
}
#line 2318 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 56:
#line 567 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	int err;

	err = inet_pton(AF_INET6, (yyvsp[0].string),
			&conf.channel[conf.channel_num].u.tcp.client);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6 address", (yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	conf.channel[conf.channel_num].u.tcp.ipproto = AF_INET6;
}
#line 2339 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 57:
#line 585 "read_config_yy.y" /* yacc.c:1646  */
    {
	int idx;

	__max_dedicated_links_reached();
	strncpy(conf.channel[conf.channel_num].channel_ifname, (yyvsp[0].string), IFNAMSIZ);

	idx = if_nametoindex((yyvsp[0].string));
	if (!idx) {
		dlog(LOG_WARNING, "%s is an invalid interface", (yyvsp[0].string));
		break;
	}
	conf.channel[conf.channel_num].u.tcp.server.ipv6.scope_id = idx;
}
#line 2357 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 58:
#line 600 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.tcp.port = (yyvsp[0].val);
}
#line 2366 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 59:
#line 606 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.tcp.sndbuf = (yyvsp[0].val);
}
#line 2375 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 60:
#line 612 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.tcp.rcvbuf = (yyvsp[0].val);
}
#line 2384 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 61:
#line 618 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.tcp.checksum = 0;
}
#line 2393 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 62:
#line 624 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	conf.channel[conf.channel_num].u.tcp.checksum = 1;
}
#line 2402 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 63:
#line 630 "read_config_yy.y" /* yacc.c:1646  */
    {
	__max_dedicated_links_reached();
	CONFIG(channelc).error_queue_length = (yyvsp[0].val);
}
#line 2411 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 64:
#line 636 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.hashsize = (yyvsp[0].val);
}
#line 2419 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 65:
#line 641 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.limit = (yyvsp[0].val);
}
#line 2427 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 69:
#line 652 "read_config_yy.y" /* yacc.c:1646  */
    {
	strcpy(conf.local.path, (yyvsp[0].string));
}
#line 2435 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 70:
#line 657 "read_config_yy.y" /* yacc.c:1646  */
    {
	dlog(LOG_WARNING, "deprecated unix backlog configuration, ignoring.");
}
#line 2443 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 71:
#line 662 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.flags & CTD_STATS_MODE) {
		dlog(LOG_ERR, "cannot use both `Stats' and `Sync' "
		     "clauses in conntrackd.conf");
		exit(EXIT_FAILURE);
	}
	conf.flags |= CTD_SYNC_MODE;
}
#line 2456 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 88:
#line 694 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(sync).tcp_window_tracking = 1;
}
#line 2464 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 89:
#line 699 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(sync).tcp_window_tracking = 0;
}
#line 2472 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 90:
#line 704 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(flags) |= CTD_EXPECT;
	CONFIG(netlink).subsys_id = NFNL_SUBSYS_NONE;
	CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
				 NF_NETLINK_CONNTRACK_UPDATE |
				 NF_NETLINK_CONNTRACK_DESTROY |
				 NF_NETLINK_CONNTRACK_EXP_NEW |
				 NF_NETLINK_CONNTRACK_EXP_UPDATE |
				 NF_NETLINK_CONNTRACK_EXP_DESTROY;
}
#line 2487 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 91:
#line 716 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(netlink).subsys_id = NFNL_SUBSYS_CTNETLINK;
	CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
				 NF_NETLINK_CONNTRACK_UPDATE |
				 NF_NETLINK_CONNTRACK_DESTROY;
}
#line 2498 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 92:
#line 724 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(flags) |= CTD_EXPECT;
	CONFIG(netlink).subsys_id = NFNL_SUBSYS_NONE;
	CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
				 NF_NETLINK_CONNTRACK_UPDATE |
				 NF_NETLINK_CONNTRACK_DESTROY |
				 NF_NETLINK_CONNTRACK_EXP_NEW |
				 NF_NETLINK_CONNTRACK_EXP_UPDATE |
				 NF_NETLINK_CONNTRACK_EXP_DESTROY;
}
#line 2513 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 95:
#line 739 "read_config_yy.y" /* yacc.c:1646  */
    {
	exp_filter_add(STATE(exp_filter), (yyvsp[0].string));
}
#line 2521 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 96:
#line 744 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.flags |= CTD_SYNC_ALARM;
}
#line 2529 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 97:
#line 749 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.flags |= CTD_SYNC_FTFW;
}
#line 2537 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 98:
#line 754 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.flags |= CTD_SYNC_NOTRACK;
}
#line 2545 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 120:
#line 789 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.sync.internal_cache_disable = 1;
}
#line 2553 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 121:
#line 794 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.sync.internal_cache_disable = 0;
}
#line 2561 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 122:
#line 799 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.sync.external_cache_disable = 1;
}
#line 2569 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 123:
#line 804 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.sync.external_cache_disable = 0;
}
#line 2577 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 124:
#line 809 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.resend_queue_size = (yyvsp[0].val);
}
#line 2585 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 125:
#line 814 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.startup_resync = 1;
}
#line 2593 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 126:
#line 819 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.startup_resync = 0;
}
#line 2601 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 127:
#line 824 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.window_size = (yyvsp[0].val);
}
#line 2609 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 130:
#line 832 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_SYN_SENT);

	__kernel_filter_add_state(TCP_CONNTRACK_SYN_SENT);
}
#line 2621 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 131:
#line 840 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_SYN_RECV);

	__kernel_filter_add_state(TCP_CONNTRACK_SYN_RECV);
}
#line 2633 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 132:
#line 848 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_ESTABLISHED);

	__kernel_filter_add_state(TCP_CONNTRACK_ESTABLISHED);
}
#line 2645 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 133:
#line 856 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_FIN_WAIT);

	__kernel_filter_add_state(TCP_CONNTRACK_FIN_WAIT);
}
#line 2657 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 134:
#line 864 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_CLOSE_WAIT);

	__kernel_filter_add_state(TCP_CONNTRACK_CLOSE_WAIT);
}
#line 2669 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 135:
#line 872 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_LAST_ACK);

	__kernel_filter_add_state(TCP_CONNTRACK_LAST_ACK);
}
#line 2681 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 136:
#line 880 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_TIME_WAIT);

	__kernel_filter_add_state(TCP_CONNTRACK_TIME_WAIT);
}
#line 2693 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 137:
#line 888 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_CLOSE);

	__kernel_filter_add_state(TCP_CONNTRACK_CLOSE);
}
#line 2705 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 138:
#line 896 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_add_state(STATE(us_filter),
			    IPPROTO_TCP,
			    TCP_CONNTRACK_LISTEN);

	__kernel_filter_add_state(TCP_CONNTRACK_LISTEN);
}
#line 2717 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 160:
#line 930 "read_config_yy.y" /* yacc.c:1646  */
    { conf.systemd = 1; }
#line 2723 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 161:
#line 931 "read_config_yy.y" /* yacc.c:1646  */
    { conf.systemd = 0; }
#line 2729 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 162:
#line 934 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.netlink_buffer_size = (yyvsp[0].val);
}
#line 2737 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 163:
#line 939 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.netlink_buffer_size_max_grown = (yyvsp[0].val);
}
#line 2745 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 164:
#line 944 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.nl_overrun_resync = 30;
}
#line 2753 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 165:
#line 949 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.nl_overrun_resync = -1;
}
#line 2761 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 166:
#line 954 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.nl_overrun_resync = (yyvsp[0].val);
}
#line 2769 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 167:
#line 959 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.netlink.events_reliable = 1;
}
#line 2777 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 168:
#line 964 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.netlink.events_reliable = 0;
}
#line 2785 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 169:
#line 969 "read_config_yy.y" /* yacc.c:1646  */
    {
	dlog(LOG_WARNING, "deprecated nice configuration, ignoring. The "
	     "nice value can be set externally with nice(1) and renice(1).");
}
#line 2794 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 173:
#line 981 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (strcasecmp((yyvsp[0].string), "rr") == 0) {
		conf.sched.type = SCHED_RR;
	} else if (strcasecmp((yyvsp[0].string), "fifo") == 0) {
		conf.sched.type = SCHED_FIFO;
	} else {
		dlog(LOG_ERR, "unknown scheduler `%s'", (yyvsp[0].string));
		exit(EXIT_FAILURE);
	}
}
#line 2809 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 174:
#line 993 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.sched.prio = (yyvsp[0].val);
	if (conf.sched.prio < 0 || conf.sched.prio > 99) {
		dlog(LOG_ERR, "`Priority' must be [0, 99]\n", (yyvsp[0].val));
		exit(EXIT_FAILURE);
	}
}
#line 2821 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1002 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(event_iterations_limit) = (yyvsp[0].val);
}
#line 2829 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1007 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.flags |= CTD_POLL;
	conf.poll_kernel_secs = (yyvsp[0].val);
	if (conf.poll_kernel_secs == 0) {
		dlog(LOG_ERR, "`PollSecs' clause must be > 0");
		exit(EXIT_FAILURE);
	}
}
#line 2842 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1017 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(filter_from_kernelspace) = 0;
}
#line 2850 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1022 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(filter_from_kernelspace) = 0;
}
#line 2858 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1027 "read_config_yy.y" /* yacc.c:1646  */
    {
	CONFIG(filter_from_kernelspace) = 1;
}
#line 2866 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1035 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_L4PROTO,
			    CT_FILTER_POSITIVE);

	__kernel_filter_start();
}
#line 2878 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1044 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_L4PROTO,
			    CT_FILTER_NEGATIVE);

	__kernel_filter_start();

	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_L4PROTO,
			      NFCT_FILTER_LOGIC_NEGATIVE);
}
#line 2894 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1060 "read_config_yy.y" /* yacc.c:1646  */
    {
	struct protoent *pent;

	pent = getprotobyname((yyvsp[0].string));
	if (pent == NULL) {
		dlog(LOG_WARNING, "getprotobyname() cannot find "
		     "protocol `%s' in /etc/protocols", (yyvsp[0].string));
		break;
	}
	ct_filter_add_proto(STATE(us_filter), pent->p_proto);

	__kernel_filter_start();

	nfct_filter_add_attr_u32(STATE(filter),
				 NFCT_FILTER_L4PROTO,
				 pent->p_proto);
}
#line 2916 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1079 "read_config_yy.y" /* yacc.c:1646  */
    {
	struct protoent *pent;

	pent = getprotobyname("tcp");
	if (pent == NULL) {
		dlog(LOG_WARNING, "getprotobyname() cannot find "
		     "protocol `tcp' in /etc/protocols");
		break;
	}
	ct_filter_add_proto(STATE(us_filter), pent->p_proto);

	__kernel_filter_start();

	nfct_filter_add_attr_u32(STATE(filter),
				 NFCT_FILTER_L4PROTO,
				 pent->p_proto);
}
#line 2938 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1098 "read_config_yy.y" /* yacc.c:1646  */
    {
	struct protoent *pent;

	pent = getprotobyname("udp");
	if (pent == NULL) {
		dlog(LOG_WARNING, "getprotobyname() cannot find "
					"protocol `udp' in /etc/protocols");
		break;
	}
	ct_filter_add_proto(STATE(us_filter), pent->p_proto);

	__kernel_filter_start();

	nfct_filter_add_attr_u32(STATE(filter),
				 NFCT_FILTER_L4PROTO,
				 pent->p_proto);
}
#line 2960 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1117 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_ADDRESS,
			    CT_FILTER_POSITIVE);

	__kernel_filter_start();
}
#line 2972 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1126 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_ADDRESS,
			    CT_FILTER_NEGATIVE);

	__kernel_filter_start();

	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_SRC_IPV4,
			      NFCT_FILTER_LOGIC_NEGATIVE);
	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_DST_IPV4,
			      NFCT_FILTER_LOGIC_NEGATIVE);
	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_SRC_IPV6,
			      NFCT_FILTER_LOGIC_NEGATIVE);
	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_DST_IPV6,
			      NFCT_FILTER_LOGIC_NEGATIVE);
}
#line 2997 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1151 "read_config_yy.y" /* yacc.c:1646  */
    {
	union inet_address ip;
	char *slash;
	unsigned int cidr = 32;

	memset(&ip, 0, sizeof(union inet_address));

	slash = strchr((yyvsp[0].string), '/');
	if (slash) {
		*slash = '\0';
		cidr = atoi(slash+1);
		if (cidr > 32) {
			dlog(LOG_WARNING, "%s/%d is not a valid network, "
			     "ignoring", (yyvsp[0].string), cidr);
			break;
		}
	}

	if (!inet_aton((yyvsp[0].string), &ip.ipv4)) {
		dlog(LOG_WARNING, "%s is not a valid IPv4, ignoring", (yyvsp[0].string));
		break;
	}

	if (slash && cidr < 32) {
		/* network byte order */
		struct ct_filter_netmask_ipv4 tmp = {
			.ip = ip.ipv4,
			.mask = ipv4_cidr2mask_net(cidr)
		};

		if (!ct_filter_add_netmask(STATE(us_filter), &tmp, AF_INET)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "netmask %s is "
				     "repeated in the ignore pool", (yyvsp[0].string));
		}
	} else {
		if (!ct_filter_add_ip(STATE(us_filter), &ip, AF_INET)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "IP %s is repeated in "
				     "the ignore pool", (yyvsp[0].string));
			if (errno == ENOSPC)
				dlog(LOG_WARNING, "too many IP in the "
				     "ignore pool!");
		}
	}
	__kernel_filter_start();

	/* host byte order */
	struct nfct_filter_ipv4 filter_ipv4 = {
		.addr = ntohl(ip.ipv4),
		.mask = ipv4_cidr2mask_host(cidr),
	};

	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_SRC_IPV4, &filter_ipv4);
	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_DST_IPV4, &filter_ipv4);
}
#line 3058 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1209 "read_config_yy.y" /* yacc.c:1646  */
    {
	union inet_address ip;
	char *slash;
	int cidr = 128;
	struct nfct_filter_ipv6 filter_ipv6;
	int err;

	memset(&ip, 0, sizeof(union inet_address));

	slash = strchr((yyvsp[0].string), '/');
	if (slash) {
		*slash = '\0';
		cidr = atoi(slash+1);
		if (cidr > 128) {
			dlog(LOG_WARNING, "%s/%d is not a valid network, "
			     "ignoring", (yyvsp[0].string), cidr);
			break;
		}
	}

	err = inet_pton(AF_INET6, (yyvsp[0].string), &ip.ipv6);
	if (err == 0) {
		dlog(LOG_WARNING, "%s is not a valid IPv6, ignoring", (yyvsp[0].string));
		break;
	} else if (err < 0) {
		dlog(LOG_ERR, "inet_pton(): IPv6 unsupported!");
		exit(EXIT_FAILURE);
	}

	if (slash && cidr < 128) {
		struct ct_filter_netmask_ipv6 tmp;

		memcpy(tmp.ip, ip.ipv6, sizeof(uint32_t)*4);
		ipv6_cidr2mask_net(cidr, tmp.mask);
		if (!ct_filter_add_netmask(STATE(us_filter), &tmp, AF_INET6)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "netmask %s is "
				     "repeated in the ignore pool", (yyvsp[0].string));
		}
	} else {
		if (!ct_filter_add_ip(STATE(us_filter), &ip, AF_INET6)) {
			if (errno == EEXIST)
				dlog(LOG_WARNING, "IP %s is repeated in "
				     "the ignore pool", (yyvsp[0].string));
			if (errno == ENOSPC)
				dlog(LOG_WARNING, "too many IP in the "
				     "ignore pool!");
		}
	}
	__kernel_filter_start();

	/* host byte order */
	ipv6_addr2addr_host(ip.ipv6, filter_ipv6.addr);
	ipv6_cidr2mask_host(cidr, filter_ipv6.mask);

	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_SRC_IPV6, &filter_ipv6);
	nfct_filter_add_attr(STATE(filter), NFCT_FILTER_DST_IPV6, &filter_ipv6);
}
#line 3121 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1269 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_STATE,
			    CT_FILTER_POSITIVE);

	__kernel_filter_start();
}
#line 3133 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1278 "read_config_yy.y" /* yacc.c:1646  */
    {
	ct_filter_set_logic(STATE(us_filter),
			    CT_FILTER_STATE,
			    CT_FILTER_NEGATIVE);


	__kernel_filter_start();

	nfct_filter_set_logic(STATE(filter),
			      NFCT_FILTER_L4PROTO_STATE,
			      NFCT_FILTER_LOGIC_NEGATIVE);
}
#line 3150 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1297 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (conf.flags & CTD_SYNC_MODE) {
		dlog(LOG_ERR, "cannot use both `Stats' and `Sync' "
		     "clauses in conntrackd.conf");
		exit(EXIT_FAILURE);
	}
	conf.flags |= CTD_STATS_MODE;
}
#line 3163 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1317 "read_config_yy.y" /* yacc.c:1646  */
    {
	strncpy(conf.stats.logfile, DEFAULT_STATS_LOGFILE, FILENAME_MAXLEN);
}
#line 3171 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1322 "read_config_yy.y" /* yacc.c:1646  */
    {
}
#line 3178 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1326 "read_config_yy.y" /* yacc.c:1646  */
    {
	strncpy(conf.stats.logfile, (yyvsp[0].string), FILENAME_MAXLEN);
}
#line 3186 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1331 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.stats.syslog_facility = DEFAULT_SYSLOG_FACILITY;
}
#line 3194 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1336 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.stats.syslog_facility = -1;
}
#line 3202 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1341 "read_config_yy.y" /* yacc.c:1646  */
    {
	if (!strcmp((yyvsp[0].string), "daemon"))
		conf.stats.syslog_facility = LOG_DAEMON;
	else if (!strcmp((yyvsp[0].string), "local0"))
		conf.stats.syslog_facility = LOG_LOCAL0;
	else if (!strcmp((yyvsp[0].string), "local1"))
		conf.stats.syslog_facility = LOG_LOCAL1;
	else if (!strcmp((yyvsp[0].string), "local2"))
		conf.stats.syslog_facility = LOG_LOCAL2;
	else if (!strcmp((yyvsp[0].string), "local3"))
		conf.stats.syslog_facility = LOG_LOCAL3;
	else if (!strcmp((yyvsp[0].string), "local4"))
		conf.stats.syslog_facility = LOG_LOCAL4;
	else if (!strcmp((yyvsp[0].string), "local5"))
		conf.stats.syslog_facility = LOG_LOCAL5;
	else if (!strcmp((yyvsp[0].string), "local6"))
		conf.stats.syslog_facility = LOG_LOCAL6;
	else if (!strcmp((yyvsp[0].string), "local7"))
		conf.stats.syslog_facility = LOG_LOCAL7;
	else {
		dlog(LOG_WARNING, "'%s' is not a known syslog facility, "
		     "ignoring.", (yyvsp[0].string));
		break;
	}

	if (conf.syslog_facility != -1 &&
	    conf.stats.syslog_facility != conf.syslog_facility)
		dlog(LOG_WARNING, "conflicting Syslog facility "
		     "values, defaulting to General");
}
#line 3237 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 213:
#line 1373 "read_config_yy.y" /* yacc.c:1646  */
    {
	conf.flags |= CTD_HELPER;
}
#line 3245 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1385 "read_config_yy.y" /* yacc.c:1646  */
    {
	struct ctd_helper_instance *helper_inst;
	struct ctd_helper *helper;
	struct stack_item *e;
	uint16_t l3proto;
	uint8_t l4proto;

	if (strcmp((yyvsp[-4].string), "inet") == 0)
		l3proto = AF_INET;
	else if (strcmp((yyvsp[-4].string), "inet6") == 0)
		l3proto = AF_INET6;
	else {
		dlog(LOG_ERR, "unknown layer 3 protocol");
		exit(EXIT_FAILURE);
	}

	if (strcmp((yyvsp[-3].string), "tcp") == 0)
		l4proto = IPPROTO_TCP;
	else if (strcmp((yyvsp[-3].string), "udp") == 0)
		l4proto = IPPROTO_UDP;
	else {
		dlog(LOG_ERR, "unknown layer 4 protocol");
		exit(EXIT_FAILURE);
	}

#ifdef BUILD_CTHELPER
	helper = helper_find(CONNTRACKD_LIB_DIR, (yyvsp[-5].string), l4proto, RTLD_NOW);
	if (helper == NULL) {
		dlog(LOG_ERR, "Unknown `%s' helper", (yyvsp[-5].string));
		exit(EXIT_FAILURE);
	}
#else
	dlog(LOG_ERR, "Helper support is disabled, recompile conntrackd");
	exit(EXIT_FAILURE);
#endif

	helper_inst = calloc(1, sizeof(struct ctd_helper_instance));
	if (helper_inst == NULL)
		break;

	helper_inst->l3proto = l3proto;
	helper_inst->l4proto = l4proto;
	helper_inst->helper = helper;

	while ((e = stack_item_pop(&symbol_stack, -1)) != NULL) {

		switch(e->type) {
		case SYMBOL_HELPER_QUEUE_NUM: {
			int *qnum = (int *) &e->data;

			helper_inst->queue_num = *qnum;
			stack_item_free(e);
			break;
		}
		case SYMBOL_HELPER_QUEUE_LEN: {
			int *qlen = (int *) &e->data;

			helper_inst->queue_len = *qlen;
			stack_item_free(e);
			break;
		}
		case SYMBOL_HELPER_POLICY_EXPECT_ROOT: {
			struct ctd_helper_policy *pol =
				(struct ctd_helper_policy *) &e->data;
			struct ctd_helper_policy *matching = NULL;
			int i;

			for (i=0; i<CTD_HELPER_POLICY_MAX; i++) {
				if (strcmp(helper->policy[i].name,
					   pol->name) != 0)
					continue;

				matching = pol;
				break;
			}
			if (matching == NULL) {
				dlog(LOG_ERR, "Unknown policy `%s' in helper "
				     "configuration", pol->name);
				exit(EXIT_FAILURE);
			}
			/* FIXME: First set default policy, then change only
			 * tuned fields, not everything.
			 */
			memcpy(&helper->policy[i], pol,
				sizeof(struct ctd_helper_policy));

			stack_item_free(e);
			break;
		}
		default:
			dlog(LOG_ERR, "Unexpected symbol parsing helper "
			     "policy");
			exit(EXIT_FAILURE);
			break;
		}
	}
	list_add(&helper_inst->head, &CONFIG(cthelper).list);
}
#line 3348 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1492 "read_config_yy.y" /* yacc.c:1646  */
    {
	int *qnum;
	struct stack_item *e;

	e = stack_item_alloc(SYMBOL_HELPER_QUEUE_NUM, sizeof(int));
	qnum = (int *) e->data;
	*qnum = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3362 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1503 "read_config_yy.y" /* yacc.c:1646  */
    {
	int *qlen;
	struct stack_item *e;

	e = stack_item_alloc(SYMBOL_HELPER_QUEUE_LEN, sizeof(int));
	qlen = (int *) e->data;
	*qlen = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3376 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1514 "read_config_yy.y" /* yacc.c:1646  */
    {
	struct stack_item *e;
	struct ctd_helper_policy *policy;

	e = stack_item_pop(&symbol_stack, SYMBOL_HELPER_EXPECT_POLICY_LEAF);
	if (e == NULL) {
		dlog(LOG_ERR, "Helper policy configuration empty, fix your "
		     "configuration file, please");
		exit(EXIT_FAILURE);
		break;
	}

	policy = (struct ctd_helper_policy *) &e->data;
	strncpy(policy->name, (yyvsp[-3].string), CTD_HELPER_NAME_LEN);
	policy->name[CTD_HELPER_NAME_LEN-1] = '\0';
	/* Now object is complete. */
	e->type = SYMBOL_HELPER_POLICY_EXPECT_ROOT;
	stack_item_push(&symbol_stack, e);
}
#line 3400 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 228:
#line 1543 "read_config_yy.y" /* yacc.c:1646  */
    {
	struct stack_item *e;
	struct ctd_helper_policy *policy;

	e = stack_item_pop(&symbol_stack, SYMBOL_HELPER_EXPECT_POLICY_LEAF);
	if (e == NULL) {
		e = stack_item_alloc(SYMBOL_HELPER_EXPECT_POLICY_LEAF,
				     sizeof(struct ctd_helper_policy));
	}
	policy = (struct ctd_helper_policy *) &e->data;
	policy->expect_max = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3418 "read_config_yy.c" /* yacc.c:1646  */
    break;

  case 229:
#line 1558 "read_config_yy.y" /* yacc.c:1646  */
    {
	struct stack_item *e;
	struct ctd_helper_policy *policy;

	e = stack_item_pop(&symbol_stack, SYMBOL_HELPER_EXPECT_POLICY_LEAF);
	if (e == NULL) {
		e = stack_item_alloc(SYMBOL_HELPER_EXPECT_POLICY_LEAF,
				     sizeof(struct ctd_helper_policy));
	}
	policy = (struct ctd_helper_policy *) &e->data;
	policy->expect_timeout = (yyvsp[0].val);
	stack_item_push(&symbol_stack, e);
}
#line 3436 "read_config_yy.c" /* yacc.c:1646  */
    break;


#line 3440 "read_config_yy.c" /* yacc.c:1646  */
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
#line 1572 "read_config_yy.y" /* yacc.c:1906  */


int __attribute__((noreturn))
yyerror(char *msg)
{
	dlog(LOG_ERR, "parsing config file in line (%d), symbol '%s': %s",
	     yylineno, yytext, msg);
	exit(EXIT_FAILURE);
}

static void __kernel_filter_start(void)
{
	if (!STATE(filter)) {
		STATE(filter) = nfct_filter_create();
		if (!STATE(filter)) {
			dlog(LOG_ERR, "cannot create ignore pool!");
			exit(EXIT_FAILURE);
		}
	}
}

static void __kernel_filter_add_state(int value)
{
	__kernel_filter_start();

	struct nfct_filter_proto filter_proto = {
		.proto = IPPROTO_TCP,
		.state = value
	};
	nfct_filter_add_attr(STATE(filter),
			     NFCT_FILTER_L4PROTO_STATE,
			     &filter_proto);
}

static void __max_dedicated_links_reached(void)
{
	if (conf.channel_num >= MULTICHANNEL_MAX) {
		dlog(LOG_ERR, "too many dedicated links in the configuration "
		     "file (Maximum: %d)", MULTICHANNEL_MAX);
		exit(EXIT_FAILURE);
	}
}

int
init_config(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp)
		return -1;

	/* Zero may be a valid facility */
	CONFIG(syslog_facility) = -1;
	CONFIG(stats).syslog_facility = -1;
	CONFIG(netlink).subsys_id = -1;

#ifdef BUILD_SYSTEMD
        CONFIG(systemd) = 1;
#endif /* BUILD_SYSTEMD */

	/* Initialize list of user-space helpers */
	INIT_LIST_HEAD(&CONFIG(cthelper).list);

	stack_init(&symbol_stack);

	yyrestart(fp);
	yyparse();
	fclose(fp);

#ifndef BUILD_SYSTEMD
	if (CONFIG(systemd) == 1) {
		dlog(LOG_WARNING, "systemd runtime support activated but "
		     "conntrackd was built without support "
		     "for it. Recompile conntrackd");
	}
#endif /* BUILD_SYSTEMD */

	/* set to default is not specified */
	if (strcmp(CONFIG(lockfile), "") == 0)
		strncpy(CONFIG(lockfile), DEFAULT_LOCKFILE, FILENAME_MAXLEN);

	/* default to 180 seconds of expiration time: cache entries */
	if (CONFIG(cache_timeout) == 0)
		CONFIG(cache_timeout) = 180;

	/* default to 60 seconds: purge kernel entries */
	if (CONFIG(purge_timeout) == 0)
		CONFIG(purge_timeout) = 60;

	/* default to 60 seconds of refresh time */
	if (CONFIG(refresh) == 0)
		CONFIG(refresh) = 60;

	if (CONFIG(resend_queue_size) == 0)
		CONFIG(resend_queue_size) = 131072;

	/* default to a window size of 300 packets */
	if (CONFIG(window_size) == 0)
		CONFIG(window_size) = 300;

	if (CONFIG(event_iterations_limit) == 0)
		CONFIG(event_iterations_limit) = 100;

	/* default number of bucket of the hashtable that are committed in
	   one run loop. XXX: no option available to tune this value yet. */
	if (CONFIG(general).commit_steps == 0)
		CONFIG(general).commit_steps = 8192;

	/* if overrun, automatically resync with kernel after 30 seconds */
	if (CONFIG(nl_overrun_resync) == 0)
		CONFIG(nl_overrun_resync) = 30;

	/* default to 128 elements in the channel error queue */
	if (CONFIG(channelc).error_queue_length == 0)
		CONFIG(channelc).error_queue_length = 128;

	if (CONFIG(netlink).subsys_id == -1) {
		CONFIG(netlink).subsys_id = NFNL_SUBSYS_CTNETLINK;
		CONFIG(netlink).groups = NF_NETLINK_CONNTRACK_NEW |
					 NF_NETLINK_CONNTRACK_UPDATE |
					 NF_NETLINK_CONNTRACK_DESTROY;
	}

	return 0;
}
