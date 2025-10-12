%{
#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/pktloc.h>
%}

%locations
%error-verbose
%define api.pure
%name-prefix "pktloc_"

%parse-param {void *scanner}
%lex-param {void *scanner}
%expect 1

%union {
	struct rtnl_pktloc *l;
	uint32_t i;
	char *s;
}

%{
extern int pktloc_lex(YYSTYPE *, YYLTYPE *, void *);

static void yyerror(YYLTYPE *locp, void *scanner, const char *msg)
{
	NL_DBG(1, "Error while parsing packet location file: %s\n", msg);
}
%}

%token <i> ERROR NUMBER LAYER ALIGN
%token <s> NAME

%type <i> mask layer align shift
%type <l> location

%destructor { free($$); } NAME

%start input

%%

input:
	/* empty */
	| location input
	;

location:
	NAME align layer NUMBER mask shift
		{
			struct rtnl_pktloc *loc;

			if (!(loc = rtnl_pktloc_alloc())) {
				NL_DBG(1, "Allocating a packet location "
					  "object failed.\n");
				YYABORT;
			}

			loc->name = $1;
			loc->align = $2;
			loc->layer = $3;
			loc->offset = $4;
			loc->mask = $5;
			loc->shift = $6;

			if (rtnl_pktloc_add(loc) < 0) {
				NL_DBG(1, "Duplicate packet location entry "
					  "\"%s\"\n", $1);
			}

			$$ = loc;
		}
	;

align:
	ALIGN
		{ $$ = $1; }
	| NUMBER
		{ $$ = $1; }
	;

layer:
	/* empty */
		{ $$ = TCF_LAYER_NETWORK; }
	| LAYER '+' 
		{ $$ = $1; }
	;

mask:
	/* empty */
		{ $$ = 0; }
	| NUMBER
		{ $$ = $1; }
	;

shift:
	/* empty */
		{ $$ = 0; }
	| NUMBER
		{ $$ = $1; }
	;
