/**
 * logctl - a tool to access lumberjack logs in MongoDB
 *  ... and potentially other sources in the future.
 *
 * Copyright 2012 Ulrike Gerhards and Adiscon GmbH.
 *
 * Copyright 2017 Hugo Soszynski and aDvens
 *
 * long		short

 * level	l	read records with level x
 * severity	s	read records with severity x
 * ret		r	number of records to return
 * skip		k	number of records to skip
 * sys		y	read records of system x
 * msg		m	read records with message containing x
 * datef	f	read records starting on time received x
 * dateu	u	read records until time received x
 *
 * examples:
 *
 * logctl -f 15/05/2012-12:00:00 -u 15/05/2012-12:37:00
 * logctl -s 50 --ret 10
 * logctl -m "closed"
 * logctl -l "INFO"
 * logctl -s 3
 * logctl -y "ubuntu"
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "config.h"
#define _XOPEN_SOURCE 700 /* Need to define POSIX version to use strptime() */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>

/* we need this to avoid issues with older versions of libbson */
#ifndef AIX
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-attributes"
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#endif
#include <mongoc.h>
#include <bson.h>
#ifndef AIX
#pragma GCC diagnostic pop
#endif

#define N 80

static struct option long_options[] =
	{
		{"level",    required_argument, NULL, 'l'},
		{"severity", required_argument, NULL, 's'},
		{"ret",      required_argument, NULL, 'r'},
		{"skip",     required_argument, NULL, 'k'},
		{"sys",      required_argument, NULL, 'y'},
		{"msg",      required_argument, NULL, 'm'},
		{"datef",    required_argument, NULL, 'f'},
		{"dateu",    required_argument, NULL, 'u'},
		{NULL, 0,                       NULL, 0}
	};

struct queryopt
{
	int32_t e_sever;
	int32_t e_ret;
	int32_t e_skip;
	char* e_date;
	char* e_level;
	char* e_msg;
	char* e_sys;
	char* e_dateu;
	int bsever;
	int blevel;
	int bskip;
	int bret;
	int bsys;
	int bmsg;
	int bdate;
	int bdatef;
	int bdateu;
};

struct ofields
{
	const char* msg;
	const char* syslog_tag;
	const char* prog;
	char* date;
	int64_t date_r;
};

struct query_doc
{
	bson_t* query;
};

struct select_doc
{
	bson_t* select;
};

struct db_connect
{
	mongoc_client_t* conn;
};

struct db_collection
{
	mongoc_collection_t* collection;
};

struct db_cursor
{
	mongoc_cursor_t* cursor;
};

struct results
{
	const bson_t* result;
};


static void formater(struct ofields* fields)
{
	char str[N];
	time_t rtime;
	struct tm now;

	rtime = (time_t) (fields->date_r / 1000);
	strftime (str, N, "%b %d %H:%M:%S", gmtime_r (&rtime, &now));
	printf ("%s  %s %s %s\n", str, fields->prog, fields->syslog_tag,
		fields->msg);
}

static struct ofields* get_data(struct results* res)
{
	struct ofields* fields;
	const char* msg;
	const char* prog;
	const char* syslog_tag;
	int64_t date_r;
	bson_iter_t c;

	fields = malloc (sizeof (struct ofields));
	bson_iter_init_find (&c, res->result, "msg");
	if (!(msg = bson_iter_utf8 (&c, NULL)))
	{
		perror ("bson_cursor_get_string()");
		exit (1);
	}

	bson_iter_init_find (&c, res->result, "sys");
	if (!(prog = bson_iter_utf8 (&c, NULL)))
	{
		perror ("bson_cursor_get_string()");
		exit (1);
	}

	bson_iter_init_find (&c, res->result, "syslog_tag");
	if (!(syslog_tag = bson_iter_utf8 (&c, NULL)))
	{
		perror ("bson_cursor_get_string()");
		exit (1);
	}

	bson_iter_init_find (&c, res->result, "time_rcvd");
	if (!(date_r = bson_iter_date_time (&c)))
	{
		perror ("bson_cursor_get_utc_datetime()");
		exit (1);
	}

	fields->msg = msg;
	fields->prog = prog;
	fields->syslog_tag = syslog_tag;
	fields->date_r = date_r;

	return fields;
}

static void getoptions(int argc, char* argv[], struct queryopt* opt)
{
	int iarg;

	while ((iarg = getopt_long (argc, argv, "l:s:r:k:y:f:u:m:",
				    long_options, NULL)) != -1)
	{
		/* check to see if a single character or long option came through */
		switch (iarg)
		{
			/* short option 's' */
			case 's':
				opt->bsever = 1;
				opt->e_sever = atoi (optarg);
				break;
				/* short option 'r' */
			case 'r':
				opt->bret = 1;
				opt->e_ret = atoi (optarg);
				break;
				/* short option 'f' : date from */
			case 'f':
				opt->bdate = 1;
				opt->bdatef = 1;
				opt->e_date = optarg;
				break;
				/* short option 'u': date until */
			case 'u':
				opt->bdate = 1;
				opt->bdateu = 1;
				opt->e_dateu = optarg;
				break;
				/* short option 'k' */
			case 'k':
				opt->bskip = 1;
				opt->e_skip = atoi (optarg);
				break;
				/* short option 'l' */
			case 'l':
				opt->blevel = 1;
				opt->e_level = optarg;
				break;
				/* short option 'm' */
			case 'm':
				opt->bmsg = 1;
				opt->e_msg = optarg;
				break;
				/* short option 'y' */
			case 'y':
				opt->bsys = 1;
				opt->e_sys = optarg;
				break;
			default:
				break;
		}                                /* end switch iarg */
	}                                        /* end while */

}                                                /* end void getoptions */

static struct select_doc* create_select(void)
/* BSON object indicating the fields to return */
{
	struct select_doc* s_doc;

	s_doc = malloc (sizeof (struct select_doc));
	s_doc->select = bson_new ();
	bson_append_utf8 (s_doc->select, "syslog_tag", 10, "s", 1);
	bson_append_utf8 (s_doc->select, "msg", 3, "ERROR", 5);
	bson_append_utf8 (s_doc->select, "sys", 3, "sys", 3);
	bson_append_date_time (s_doc->select, "time_rcvd", 9, 1ll);
	return s_doc;
}

static struct query_doc* create_query(struct queryopt* opt)
{
	struct query_doc* qu_doc;
	bson_t* query_what, * order_what, * msg_what, * date_what;
	struct tm tm;
	time_t t;
	int64_t ts;

	qu_doc = malloc (sizeof (struct query_doc));
	qu_doc->query = bson_new ();
	query_what = bson_new ();
	bson_init (query_what);
	bson_append_document_begin (qu_doc->query, "$query", 6, query_what);
	if (opt->bsever == 1)
	{
		bson_append_int32 (query_what, "syslog_sever", 12,
				   opt->e_sever);
	}
	if (opt->blevel == 1)
	{
		bson_append_utf8 (query_what, "level", 5, opt->e_level, -1);
	}

	if (opt->bmsg == 1)
	{
		msg_what = bson_new ();
		bson_init (msg_what);
		bson_append_document_begin (query_what, "msg", 3, msg_what);
		bson_append_utf8 (msg_what, "$regex", 6, opt->e_msg, -1);
		bson_append_utf8 (msg_what, "$options", 8, "i", 1);
		bson_append_document_end (query_what, msg_what);
	}

	if (opt->bdate == 1)
	{
		date_what = bson_new ();
		bson_init (date_what);
		bson_append_document_begin (query_what, "time_rcvd", 9,
					    date_what);
		if (opt->bdatef == 1)
		{
			tm.tm_isdst = -1;
			strptime (opt->e_date, "%d/%m/%Y-%H:%M:%S", &tm);
			tm.tm_hour = tm.tm_hour + 1;
			t = mktime (&tm);
			ts = 1000 * (int64_t) t;
			bson_append_date_time (date_what, "$gt", 3, ts);
		}

		if (opt->bdateu == 1)
		{
			tm.tm_isdst = -1;
			strptime (opt->e_dateu, "%d/%m/%Y-%H:%M:%S", &tm);
			tm.tm_hour = tm.tm_hour + 1;
			t = mktime (&tm);
			ts = 1000 * (int64_t) t;
			bson_append_date_time (date_what, "$lt", 3, ts);
		}
		bson_append_document_end (query_what, date_what);
	}

	if (opt->bsys == 1)
	{
		bson_append_utf8 (query_what, "sys", 3, opt->e_sys, -1);
	}

	bson_append_document_end (qu_doc->query, query_what);

	order_what = bson_new ();
	bson_init (order_what);
	bson_append_document_begin (qu_doc->query, "$orderby", 8, order_what);
	bson_append_date_time (order_what, "time_rcvd", 9, 1ll);
	bson_append_document_end (qu_doc->query, order_what);

	bson_free (order_what);
	return qu_doc;
}

static struct db_connect* create_conn(void)
{
	struct db_connect* db_conn;

	db_conn = malloc (sizeof (struct db_connect));
	db_conn->conn = mongoc_client_new ("mongodb://localhost:27017");
	if (!db_conn->conn)
	{
		perror ("mongo_sync_connect()");
		exit (1);
	}
	return db_conn;
}

static void close_conn(struct db_connect* db_conn)
{
	mongoc_client_destroy (db_conn->conn);
	free (db_conn);
}

static void free_cursor(struct db_cursor* db_c)
{
	mongoc_cursor_destroy (db_c->cursor);
	free (db_c);
}

static struct db_cursor* launch_query(struct queryopt* opt,
				      __attribute__((unused)) struct select_doc* s_doc,
				      struct query_doc* qu_doc,
				      struct db_collection* db_coll)
{
	struct db_cursor* out;
#if MONGOC_CHECK_VERSION (1, 5, 0) /* Declaration before code (ISO C90) */
	const bson_t* opts = BCON_NEW (
		"skip", BCON_INT32 (opt->e_skip),
		"limit", BCON_INT32 (opt->e_ret)
	);
#endif /* MONGOC_CHECK_VERSION (1, 5, 0) */

	out = malloc (sizeof (struct db_cursor));
	if (!out)
	{
		perror ("mongo_sync_cmd_query()");
		printf ("malloc failed\n");
		exit (1);
	}
#if MONGOC_CHECK_VERSION (1, 5, 0)
	out->cursor = mongoc_collection_find_with_opts (db_coll->collection,
							qu_doc->query, opts,
							NULL);
#else /* !MONGOC_CHECK_VERSION (1, 5, 0) */
	out->cursor = mongoc_collection_find (db_coll->collection,
					      MONGOC_QUERY_NONE,
					      (uint32_t)opt->e_skip,
					      (uint32_t)opt->e_ret, 0,
					      qu_doc->query, s_doc->select,
					      NULL);
#endif /* MONGOC_CHECK_VERSION (1, 5, 0) */
	if (!out->cursor)
	{
		perror ("mongo_sync_cmd_query()");
		printf ("no records found\n");
		exit (1);
	}
	return out;
}

static int cursor_next(struct db_cursor* db_c, struct results* res)
{
	if (mongoc_cursor_next (db_c->cursor, &res->result))
		return true;
	return false;
}

static struct db_collection* get_collection(struct db_connect* db_conn)
{
	struct db_collection* coll;

	coll = malloc (sizeof (struct db_collection));
	coll->collection = mongoc_client_get_collection (db_conn->conn,
							 "syslog", "log");
	return coll;
}

static void release_collection(struct db_collection* db_coll)
{
	mongoc_collection_destroy (db_coll->collection);
	free (db_coll);
}

int main(int argc, char* argv[])
{

	struct queryopt opt;
	struct ofields* fields;
	struct select_doc* s_doc;
	struct query_doc* qu_doc;
	struct db_connect* db_conn;
	struct db_cursor* db_c;
	struct db_collection* db_coll;
	struct results* res;

	memset (&opt, 0, sizeof (struct queryopt));

	mongoc_init (); /* Initialisation of mongo-c-driver */

	getoptions (argc, argv, &opt);
	qu_doc = create_query (&opt); /* create query */
	s_doc = create_select ();
	db_conn = create_conn (); /* create connection */
	db_coll = get_collection (db_conn); /* Get the collection to perform query on */
	db_c = launch_query (&opt, s_doc, qu_doc, db_coll); /* launch the query and get the related cursor */

	res = malloc (sizeof (struct results));
	while (cursor_next (db_c, res)) /* Move cursor & get pointed data */
	{
		fields = get_data (res);
		formater (fields); /* format output */
		free (fields);
	}

	free (res);
	free_cursor (db_c);
	release_collection (db_coll);
	close_conn (db_conn);
	free (s_doc);
	free (qu_doc);

	mongoc_cleanup (); /* Cleanup of mongo-c-driver */

	return (0);
}
