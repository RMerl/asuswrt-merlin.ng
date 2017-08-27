%{
/* -*- Mode: C -*-                                                         */
/*  ====================================================================== */
/*  FILE: icalssyacc.y                                                     */
/*  CREATOR: eric 08 Aug 2000                                              */
/*                                                                         */
/*  DESCRIPTION:                                                           */
/*                                                                         */                                                                       
/*  $Id: icalssyacc.y,v 1.10 2008-01-14 00:35:26 dothebart Exp $           */
/*  $Locker:  $                                                            */
/*                                                                         */                                                                     
/*  (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org        */
/*                                                                         */
/* This program is free software; you can redistribute it and/or modify    */
/* it under the terms of either:                                           */
/*                                                                         */
/*    The LGPL as published by the Free Software Foundation, version       */
/*    2.1, available at: http://www.fsf.org/copyleft/lesser.html           */
/*                                                                         */
/*  Or:                                                                    */
/*                                                                         */
/*    The Mozilla Public License Version 1.0. You may obtain a copy of     */
/*    the License at http://www.mozilla.org/MPL/                           */
/*                                                                         */
/* The Original Code is eric. The Initial Developer of the Original        */
/* Code is Eric Busboom                                                    */
/*                                                                         */
/*  ====================================================================== */
/*#define YYDEBUG 1*/
#include <stdlib.h>
#include <string.h> /* for strdup() */
#include <limits.h> /* for SHRT_MAX*/
#include <libical/ical.h>
#include "icalgauge.h"
#include "icalgaugeimpl.h"

extern struct icalgauge_impl *icalss_yy_gauge;

#define YYPARSE_PARAM yy_globals
#define YYLEX_PARAM yy_globals
#define YY_EXTRA_TYPE  icalgauge_impl*


void sserror(char *s); 

static void ssyacc_add_where(struct icalgauge_impl* impl, char* prop, 
			icalgaugecompare compare , char* value);
static void ssyacc_add_select(struct icalgauge_impl* impl, char* str1);
static void ssyacc_add_from(struct icalgauge_impl* impl, char* str1);
static void set_logic(struct icalgauge_impl* impl,icalgaugelogic l);

/* Don't know why I need this....  */
 
/* older flex version (such as included in OpenBSD) takes a different calling syntax */
#ifdef YYPARSE_PARAM 
int sslex(void *YYPARSE_PARAM); 
#else 
int sslex(void);
#endif 
%}

%union {
	char* v_string;
}


%token <v_string> STRING
%token SELECT FROM WHERE COMMA QUOTE EQUALS NOTEQUALS  LESS GREATER LESSEQUALS
%token GREATEREQUALS AND OR EOL END IS NOT SQLNULL

%%

query_min: SELECT select_list FROM from_list WHERE where_list
	   | SELECT select_list FROM from_list
	   | error { 
                 yyclearin;
		 YYABORT;
           }	
	   ;	

select_list:
	STRING {ssyacc_add_select(icalss_yy_gauge,$1);}
	| select_list COMMA STRING {ssyacc_add_select(icalss_yy_gauge,$3);}
	;


from_list:
	STRING {ssyacc_add_from(icalss_yy_gauge,$1);}
	| from_list COMMA STRING {ssyacc_add_from(icalss_yy_gauge,$3);}
	;

where_clause:
	/* Empty */
	| STRING EQUALS STRING {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_EQUAL,$3); }
	| STRING IS SQLNULL {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_ISNULL,""); }
	| STRING IS NOT SQLNULL {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_ISNOTNULL,""); }
	| STRING NOTEQUALS STRING {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_NOTEQUAL,$3); }
	| STRING LESS STRING {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_LESS,$3); }
	| STRING GREATER STRING {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_GREATER,$3); }
	| STRING LESSEQUALS STRING {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_LESSEQUAL,$3); }
	| STRING GREATEREQUALS STRING {ssyacc_add_where(icalss_yy_gauge,$1,ICALGAUGECOMPARE_GREATEREQUAL,$3); }
	;

where_list:
	where_clause {set_logic(icalss_yy_gauge,ICALGAUGELOGIC_NONE);}
	| where_list AND where_clause {set_logic(icalss_yy_gauge,ICALGAUGELOGIC_AND);} 
	| where_list OR where_clause {set_logic(icalss_yy_gauge,ICALGAUGELOGIC_OR);}
	;


%%

static void ssyacc_add_where(struct icalgauge_impl* impl, char* str1, 
	icalgaugecompare compare , char* value_str)
{

    struct icalgauge_where *where;
    char *compstr, *propstr, *c, *s,*l;
    
    if ( (where = malloc(sizeof(struct icalgauge_where))) ==0){
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return;
    }

    memset(where,0,sizeof(struct icalgauge_where));
    where->logic = ICALGAUGELOGIC_NONE;
    where->compare = ICALGAUGECOMPARE_NONE;
    where->comp = ICAL_NO_COMPONENT;
    where->prop = ICAL_NO_PROPERTY;

    /* remove enclosing quotes */
    s = value_str;
    if(*s == '\''){
	s++;
    }
    l = s+strlen(s)-1;
    if(*l == '\''){
	*l=0;
    }
	
    where->value = strdup(s);

    /* Is there a period in str1 ? If so, the string specified both a */
    /* component and a property                                       */ 
    if( (c = strrchr(str1,'.')) != 0){
	compstr = str1;
	propstr = c+1;
	*c = '\0';
    } else {
	compstr = 0;
	propstr = str1;
    }


    /* Handle the case where a component was specified */
    if(compstr != 0){
	where->comp = icalenum_string_to_component_kind(compstr);
    } else {
	where->comp = ICAL_NO_COMPONENT;
    }

    where->prop = icalenum_string_to_property_kind(propstr);    

    where->compare = compare;

    if(where->value == 0){
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	free(where->value);
	return;
    }

    pvl_push(impl->where,where);
}

static void set_logic(struct icalgauge_impl* impl,icalgaugelogic l)
{
    pvl_elem e = pvl_tail(impl->where);
    struct icalgauge_where *where = pvl_data(e);

    where->logic = l;
   
}



static void ssyacc_add_select(struct icalgauge_impl* impl, char* str1)
{
    char *c, *compstr, *propstr;
    struct icalgauge_where *where;
    
    /* Uses only the prop and comp fields of the where structure */
    if ( (where = malloc(sizeof(struct icalgauge_where))) ==0){
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return;
    }

    memset(where,0,sizeof(struct icalgauge_where));
    where->logic = ICALGAUGELOGIC_NONE;
    where->compare = ICALGAUGECOMPARE_NONE;
    where->comp = ICAL_NO_COMPONENT;
    where->prop = ICAL_NO_PROPERTY;

    /* Is there a period in str1 ? If so, the string specified both a */
    /* component and a property */
    if( (c = strrchr(str1,'.')) != 0){
	compstr = str1;
	propstr = c+1;
	*c = '\0';
    } else {
	compstr = 0;
	propstr = str1;
    }


    /* Handle the case where a component was specified */
    if(compstr != 0){
	where->comp = icalenum_string_to_component_kind(compstr);
    } else {
	where->comp = ICAL_NO_COMPONENT;
    }


    /* If the property was '*', then accept all properties */
    if(strcmp("*",propstr) == 0) {
	where->prop = ICAL_ANY_PROPERTY; 	    
    } else {
	where->prop = icalenum_string_to_property_kind(propstr);    
    }
    

    if(where->prop == ICAL_NO_PROPERTY){
      free(where);
      icalerror_set_errno(ICAL_BADARG_ERROR);
      return;
    }

    pvl_push(impl->select,where);
}

static void ssyacc_add_from(struct icalgauge_impl* impl, char* str1)
{
    icalcomponent_kind ckind;

    ckind = icalenum_string_to_component_kind(str1);

    if(ckind == ICAL_NO_COMPONENT){
	assert(0);
    }

    pvl_push(impl->from,(void*)ckind);

}


void sserror(char *s){
  fprintf(stderr,"Parse error \'%s\'\n", s);
  icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
}
