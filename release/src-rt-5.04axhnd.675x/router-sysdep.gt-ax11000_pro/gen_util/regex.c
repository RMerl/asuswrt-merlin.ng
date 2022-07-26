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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "genutil_regex.h"

static int matchhere(RE_NODE *re_list, char *text);
static int matchstar(RE_NODE *c, RE_NODE *regexp, char *text);
static int matchplus(RE_NODE *c, RE_NODE *regexp, char *text);
static int matchcomp(RE_NODE *c, char *text);

RE_NODE* regex_compile(char *regexp)
{
    char c;
    char *p;
    RE_NODE *new = NULL, *tail = NULL, *re_list = NULL;

    for (; regexp[0] != '\0'; regexp++)
    {
        new = (RE_NODE *) malloc(sizeof(RE_NODE));
        new->ccl = NULL;
        new->next = NULL;
        new->ch = '\0';
        c = regexp[0];
        switch (c) {
            case '^':
                new->type = POS_BEGIN;
                break;

            case '$':
                new->type = POS_END;
                break;

            case '.':
                new->type = ANY;
                break;

            case '*':
                new->type = STAR;
                break;

            case '+':
                new->type = PLUS;
                break;

            case '\\':
                new->type = CHAR;
                regexp++;
                new->ch = regexp[0];
                break;

            case '[':
                new->type = COMP;
                new->ccl = strdup(regexp + 1);
                while ((*regexp != '\0') && (*regexp != ']'))
                {
                    regexp++;
                }

                p = new->ccl;
                while ((*p != '\0') && (*p != ']'))
                {
                    p++;
                }
                if (*p == ']')
                {
                    *p = '\0';
                }

                break;

            default:
                new->type = CHAR;
                new->ch = regexp[0];
                break;
        }
        if (re_list == NULL)
        {
            re_list = new;
            tail = re_list;
        }
        else
        {
            tail->next = new;
            tail = new;
        }
        //printf("allocated re node %d\n", new->type);
    }
    return re_list;
}

/* regex_match: search for regexp anywhere in text */
int regex_match(RE_NODE *re_list, char *text)
{
    if (re_list->type == POS_BEGIN)
        return matchhere(re_list->next, text);
    do {    /* must look even if string is empty */
        if (matchhere(re_list, text))
            return 1;
    } while (*text++ != '\0');
    return 0;
}

void regex_free(RE_NODE *re_list)
{
    RE_NODE *p, *q;

    p = re_list;
    while (p != NULL)
    {
        q = p->next;
        if (p->ccl != NULL)
        {
            free(p->ccl);
        }
        free(p);
        p = q;
    }
}

/* matchhere: search for regexp at beginning of text */
static int matchhere(RE_NODE *re_list, char *text)
{
    if (re_list == NULL)
        return 1;
    if ((re_list->next != NULL) && (re_list->next->type == STAR))
        return matchstar(re_list, re_list->next->next, text);
    if ((re_list->next != NULL) && (re_list->next->type == PLUS))
        return matchplus(re_list, re_list->next->next, text);
    if (re_list->type == POS_END && re_list->next == NULL)
        return *text == '\0';
    if (*text != '\0' && (re_list->type == ANY ||
                          (re_list->type == CHAR && re_list->ch == *text) ||
                          (re_list->type == COMP && matchcomp(re_list, text))))
        return matchhere(re_list->next, text + 1);
    return 0;
}

/* matchstar: search for c*regexp at beginning of text */
static int matchstar(RE_NODE *c, RE_NODE *regexp, char *text)
{
    do {    /* a * matches zero or more instances */
        if (matchhere(regexp, text))
            return 1;
    } while (*text != '\0' && (c->type == ANY ||
                               (c->type == COMP && matchcomp(c, text)) ||
                               (c->type == CHAR && *text == c->ch)) &&
                               text++);
    return 0;
}

static int matchplus(RE_NODE *c, RE_NODE *regexp, char *text)
{
    while (*text != '\0' && (c->type == ANY ||
                             (c->type == COMP && matchcomp(c, text)) ||
                             (c->type == CHAR && *text == c->ch)))
    {
        text++;
        if (matchhere(regexp, text))
            return 1;
    }
    return 0;
}

static int matchcomp(RE_NODE *c, char *text)
{
    int i, len;
    len = strlen(c->ccl);
    for (i = 0; i < len; i++)
    {
        if ((i > 0) && (i < len - 1) && (c->ccl[i] == '-'))
        {
            if (*text > c->ccl[i-1] && *text < c->ccl[i+1])
                return 1;
        }
        if (*text == c->ccl[i])
            return 1;
    }
    return 0;
}
