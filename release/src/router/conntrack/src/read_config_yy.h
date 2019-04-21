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
#line 58 "read_config_yy.y" /* yacc.c:1909  */

	int		val;
	char		*string;

#line 243 "read_config_yy.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_READ_CONFIG_YY_H_INCLUDED  */
