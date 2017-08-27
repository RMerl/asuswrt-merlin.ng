%{
#include <stdio.h>
#include <sys/types.h>
extern void resolvesym(char * token);
%}

%%
\[<[a-f0-9]*>\] { resolvesym(yytext); }
%%

/* Compilation:
 *
 * lex symfix.l
 * cc lex.yy.c -lfl -o symfix
 *
symfix:  symfix.l
	lex symfix.l
	cc lex.yy.c -lfl -o $@
 *
 */

/*---PATRICIA---*/
static inline u_int uintoffset( u_int ukey, u_int offset )
{   return ( (ukey >> offset) & 0x1 ); }
static inline u_int uintbit( u_int ukey, u_int offset )
{   return ( uintoffset(ukey, offset) != 0 ); }
static inline u_char uintbitcmp( u_int ukey1, u_int ukey2, u_int offset )
{   return ( uintbit(ukey1,offset) == uintbit(ukey2,offset) ); }
typedef void ( * uvisitFunc_t )( u_int ukey, void * datap );
typedef struct upnode {
        u_int           _ukey;
        u_int           _bitindex;
        struct upnode   * _uleftp;
        struct upnode   * _urightp;
        void            * _datap;
} upnode_t;
typedef struct uptree {
        struct upnode   * _urootp;
        uvisitFunc_t    _uvisitFuncp;
        u_int       _elem;
        u_int       _dupl;
        u_int       _real;
} uptree_t;
void * data_upn( struct upnode * upn ) { return upn->_datap; }
u_int ukey_upn( struct upnode * upn ) { return upn->_ukey; }
void print_upn( struct upnode * upn )
{
    printf( "Node: ukey<0x%08x:%u> data<0x%08x:%u>\n",
            (int)ukey_upn(upn), ukey_upn(upn),
            (int)data_upn(upn), (u_int)data_upn(upn) );
}
upnode_t * malloc_upn( void )
{
    upnode_t * upn = (upnode_t *)malloc( sizeof(upnode_t) );
    if ( upn == (upnode_t *)NULL )
    {
        fprintf( stderr, "malloc_upn: malloc failure\n");
        abort();
    }
    return upn;
}
upnode_t * create_upn( u_int ukey, u_int bitindex, void * datap )
{
    upnode_t * upn = malloc_upn();
    upn->_ukey = ukey;
    upn->_bitindex = bitindex;
    upn->_datap = datap;
    return upn;
}
void display_upt( u_int ukey, void * datap )
{
    if ( datap != (void*) NULL )
       printf ("-skey[%u] ---> data[0x%08x:%u]\n",
              ukey, (int)datap, (int)datap );
}
uptree_t * create_upt( uvisitFunc_t uvisitfuncp )
{
    upnode_t * urootp;
    if ( uvisitfuncp == (uvisitFunc_t)NULL )
       uvisitfuncp = (uvisitFunc_t)display_upt;
    uptree_t * upt = (uptree_t *) malloc( sizeof(uptree_t) );
    if ( upt == (uptree_t *)NULL )
    {
        fprintf( stderr, "create_upt: malloc failure\n");
        abort();
    }
    upt->_uvisitFuncp = uvisitfuncp;
    upt->_elem = upt->_dupl = upt->_real = 0;
    urootp = create_upn( 0, 31, (void*)NULL );
    urootp->_uleftp = urootp->_urightp = urootp;
    upt->_urootp = urootp;
    return upt;
}
upnode_t * _search_upt( uptree_t * upt, u_int ukey )
{
    if ( upt->_urootp != (upnode_t *) NULL )
    {
        upnode_t * childp = upt->_urootp;
        upnode_t * parentp;
        do {
            parentp = childp;
            if ( uintoffset(ukey, childp->_bitindex) == 0 )
                childp = childp->_uleftp;
            else
                childp = childp->_urightp;
        } while ( parentp->_bitindex > childp->_bitindex ); 
        return ( childp );
    }
    else
    {
        fprintf( stderr, "_search_upt: _urootp = NULL\n");
        abort();
    }
    return ( (upnode_t *)NULL );
}
void * search_upt( uptree_t * upt, u_int ukey )
{
    upnode_t * upn;
    if ( upt == (uptree_t *)NULL )
    {
       fprintf( stderr, "search_upt upt = NULL\n" );
       abort();
    }
    if ( ( upn = _search_upt( upt, ukey ) ) == (upnode_t *)NULL )
        return ( (void*)NULL );
    if ( upn->_ukey == ukey )
       return ( upn->_datap );
    else
       return ( (void*)NULL );
}
void * insert_upt ( uptree_t * upt, u_int ukey, void * datap )
{
    void * _datap = (void*) NULL;
    register int i;
    upnode_t * childp;
    upnode_t * parentp;
    upnode_t * upn;
    if ( upt == (uptree_t *)NULL )
    {
       fprintf( stderr, "insert_upt upt = NULL\n" );
       abort();
    }
    upn = _search_upt( upt, ukey );
    if ( upn->_ukey == ukey )
    {
        _datap = upn->_datap;
        upn->_datap = datap;
        if ( (datap == (void*)NULL) && (_datap != (void*)NULL) )
            upt->_real -= 1;
        if ( _datap != (void*)NULL )
        {
           if ( datap == (void*)NULL )
              upt->_real -= 1;
           else
              upt->_dupl += 1;
        }
        return _datap;
    }
    i = 31;
    while ( uintbit(ukey,i) == uintbit(upn->_ukey,i) )
        i = i - 1;
    childp = upt->_urootp;
    do
    {
        parentp = childp;
        if ( uintoffset(ukey, childp->_bitindex) == 0 )
            childp = childp->_uleftp;
        else
            childp = childp->_urightp;
    }  while ( (childp->_bitindex > i)
             && (parentp->_bitindex > childp->_bitindex) );
    upn = create_upn( ukey, i, datap );
    upt->_elem++; 
    if ( uintoffset( ukey, upn->_bitindex ) == 0 )
    {
        upn->_uleftp = upn;
        upn->_urightp = childp;
    }
    else 
    {
        upn->_urightp =  upn;
        upn->_uleftp = childp;
    }
    if ( uintoffset( ukey, parentp->_bitindex ) == 0 )
        parentp->_uleftp = upn;
    else
        parentp->_urightp = upn;
    return (void*) datap;
}
void _recurse_upt( uptree_t * upt, upnode_t * parentp, uvisitFunc_t ufuncp )
{
    upnode_t * childp;
    childp = parentp->_uleftp;
    if ( childp->_bitindex < parentp->_bitindex )
       _recurse_upt( upt, childp, ufuncp );
    if ( ( parentp != upt->_urootp )
        && ( parentp->_datap != (void*) NULL) )
    {
        (* ufuncp)( parentp->_ukey, parentp->_datap );
    }
    childp  = parentp->_urightp;
    if ( childp->_bitindex < parentp->_bitindex )
        _recurse_upt( upt, childp, ufuncp );
}
void traverse_upt( uptree_t * upt, uvisitFunc_t uvisitfuncp )
{
    if ( upt == (uptree_t *)NULL )
    {
       fprintf( stderr, "traverse_upt upt = NULL\n" );
       abort();
    }
    if (upt->_elem == 0)
       return;
    if ( uvisitfuncp == (uvisitFunc_t) NULL )
        uvisitfuncp = upt->_uvisitFuncp;
    if ( upt->_urootp != (upnode_t *)NULL )
        _recurse_upt( upt, upt->_urootp, uvisitfuncp );
}
void free_upt( u_int ukey, void * datap )
{
    if ( datap ) free( datap );
}
void destroy_upt( uptree_t * upt, uvisitFunc_t ufn )
{
    if ( upt == (uptree_t *)NULL ) 
    {
       fprintf( stderr, "destroy_upt upt = NULL\n" );
       abort();
    }
    if ( ufn == (uvisitFunc_t) NULL ) ufn = free_upt;
    traverse_upt( upt, ufn );
    if ( upt->_urootp ) free ( upt->_urootp );
    free( upt );
}
unsigned int size_upt( uptree_t * upt ) { return upt->_elem; }
unsigned int dupl_upt( uptree_t * upt ) { return upt->_dupl; }
unsigned int real_upt( uptree_t * upt ) { return upt->_real; }
/*---PATRICIA---*/

FILE * iFILE, * sFILE;
char * ifname, *symfname;
const char * symnull = "null";
uptree_t * symlistp = (uptree_t*)NULL;
uptree_t * symcachelistp = (uptree_t*)NULL;

void usage( char * cmd )
{
    fprintf( stderr,
            "\tUSAGE: %s -i <InFile> -s <SymbolFile>\n", cmd );
    abort();
}
void getoptions(int argc, char **argv)
{
    int c;
    int index;
    ifname = symfname = (char*)NULL;
    opterr = 0;
    if ( argc == 1 ) usage(argv[0]);
    while ((c = getopt (argc, argv, "i:s:")) !=  -1)
    {
        switch (c) {
            case 'i': ifname = optarg; break;
            case 's': symfname = optarg; break;
            case '?': if (isprint (optopt))
                          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                      else
                          fprintf (stderr,
                                 "Unknown option character `\\x%x'.\n", optopt);
                      usage(argv[0]); break;
           default: usage( argv[0] ); break;
        }
    }
    if ( optind < argc )
    {
       for ( index = optind; index < argc; index++ )
           fprintf( stderr, "\tERROR: Non-option argument %s\n", argv[index] );
       usage( argv[0] );
    }
    if ( !ifname || !symfname ) usage( argv[0] );
    if ( (iFILE = fopen( ifname, "r" )) == (FILE*)NULL )
    {
        fprintf( stderr, "ERROR: opening input file %s\n", ifname );
        perror( "fopen" ); exit(-1);
    }
    yyin = iFILE;
    if ( (sFILE = fopen( symfname, "r" )) == (FILE*)NULL )
        fprintf( stderr, "ERROR: opening symbols file %s\n", symfname );
    return;
}
void initialize( void ) 
{
   iFILE = stdin; sFILE = (FILE*)NULL;
   symlistp = create_upt( NULL );
   symcachelistp = create_upt( NULL );
   insert_upt( symlistp, 0, (char*)symnull );
}   
void getsymbols(void)
{
    int symlen = 512;
    char * symline, * symstr, * symname, * name;
    unsigned int symid;
    if ( sFILE == (FILE*) NULL ) return;
    symline = malloc( symlen + 1 );
    while ( !feof( sFILE ) )
    {
        symlen = 512;
        symlen = getline( &symline, &symlen, sFILE );
        if (symlen == -1 ) return;
        symstr = (char*)strtok( symline, " " );
        if ( symstr == (char*) NULL) continue;
        sscanf(symstr,"%x", &symid);
        name = (char*)strtok( NULL, "\n" );
        symname = malloc( strlen(name) + 1 );
        strcpy( symname, name );
        insert_upt( symlistp, symid, (void*)symname );
    }
}
void display_sym( u_int ukey, void * datap )
{
    if ( datap != (void*) NULL ) printf ("sym[0x%08x] [%s]\n", ukey, datap);
}
u_int searchkey,
       prevkey;
char * prevstr;
void lessthan( u_int ukey, void * datap )
{
    if ( ukey < searchkey){ prevkey=ukey; prevstr = datap;}
}
void _prev_upt( uptree_t * upt, upnode_t * parentp, uvisitFunc_t ufuncp )
{
    upnode_t * childp;
    childp = parentp->_uleftp;
    if ( childp->_bitindex < parentp->_bitindex )
       _recurse_upt( upt, childp, ufuncp );
    if ( ( parentp != upt->_urootp )
        && ( parentp->_datap != (void*) NULL) )
    {
        (* ufuncp)( parentp->_ukey, parentp->_datap );
    }
    childp  = parentp->_urightp;
    if ( childp->_bitindex < parentp->_bitindex )
        _recurse_upt( upt, childp, ufuncp );
}
void resolvesym( char * token )
{
    upnode_t * upn;
    char * datap;
    sscanf(token,"[<%08x>]", &searchkey);
    upn = _search_upt(symlistp,searchkey);
    if ( upn->_ukey == searchkey )
    {
       datap = upn->_datap;
       printf("@[%s+0]", datap);
    }
    else
    {
        upn = _search_upt(symcachelistp,searchkey);
        if ( upn->_ukey == searchkey )
        {
           datap = upn->_datap;
           printf("@[%s]", datap);
        }
        else
        {
            char newsym[256];
            _prev_upt(symlistp, symlistp->_urootp, lessthan);
            sprintf(newsym, "%s+%x", prevstr, searchkey - prevkey);
            datap = malloc( strlen(newsym) + 1);
            strcpy( datap, newsym);
            insert_upt( symcachelistp, searchkey, datap);
            printf("@[%s]", datap);
        }
    }
}
int main(int argc, char ** argv)
{
    if ( argc <2 ) usage( argv[0] );
    initialize();
    getoptions( argc, argv );
    getsymbols();
    if ( symlistp == (uptree_t *)NULL ) 
    {
        fprintf( stderr, "insert_upt upt = NULL\n" );
        abort();
    }
    yylex();
}
