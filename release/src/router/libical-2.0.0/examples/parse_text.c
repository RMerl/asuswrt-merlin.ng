/* parse_text.c

 */
#include <libical/ical.h>
#include <stdlib.h>

/* The icalparser_get_line routine will create a single *content* line
out of one or more input lines. The content line is all of the
properties and values for a single property, and it can span several
input lines. So, icalparser_get_line will need to be able to get more
data on its own. Read_string is a routine that does this. You can
write your own version of read stream to get data from other types of
files, sockets, etc. */

char* read_stream(char *s, size_t size, void *d)
{
    return fgets(s, (int)size, (FILE*)d);
}

void parse_text(char* argv[])
{
    char* line;
    FILE* stream;
    icalcomponent *c;

    /* Create a new parser object */
    icalparser *parser = icalparser_new();

    stream = fopen(argv[1],"r");

    assert(stream != 0);

    /* Tell the parser what input routie it should use. */
    icalparser_set_gen_data(parser,stream);

    do{

        /* Get a single content line by making one or more calls to
           read_stream()*/
        line = icalparser_get_line(parser,read_stream);

        /* Now, add that line into the parser object. If that line
           completes a component, c will be non-zero */
        c = icalparser_add_line(parser,line);


        if (c != 0){
            char *temp = icalcomponent_as_ical_string_r(c);
            printf("%s", temp);
            free(temp);

            printf("\n---------------\n");

            icalcomponent_free(c);
        }

    } while ( line != 0);


    icalparser_free(parser);
}
