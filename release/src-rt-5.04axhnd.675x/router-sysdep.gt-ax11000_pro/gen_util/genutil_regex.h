// A simple regex implementation inspired by Rob Pike's code.
// See: https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html

/*
    c    matches any literal character c
    .    matches any single character
    ^    matches the beginning of the input string
    $    matches the end of the input string
    *    matches zero or more occurrences of the previous character
    []   matches character specified in the range.
*/
#ifndef __GENUTIL_REGEX_H__
#define __GENUTIL_REGEX_H__

typedef enum re_type {
    POS_BEGIN,       /* ^ */
    POS_END,         /* $ */
    CHAR,
    ANY,             /* . */
    STAR,            /* * */
    PLUS,            /* + */
    COMP             /* [...] */
} RE_TYPE;

typedef struct re_node {
    RE_TYPE type;   /* CHAR, STAR, etc. */
    char    ch;     /* the character itself */
    char    *ccl;   /* for [...] instead */
    int     nccl;   /* true if class is negated [^...] */
    struct re_node *next;
} RE_NODE;

RE_NODE* regex_compile(char *regexp);
void regex_free(RE_NODE *re_list);
int regex_match(RE_NODE *re_list, char *text);

#endif
