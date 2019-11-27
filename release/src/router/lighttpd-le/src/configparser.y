%token_prefix TK_
%extra_argument {config_t *ctx}
%name configparser

%include {
#include "first.h"
#include "configfile.h"
#include "buffer.h"
#include "array.h"
#include "request.h" /* http_request_host_normalize() */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void configparser_push(config_t *ctx, data_config *dc, int isnew) {
  if (isnew) {
    dc->context_ndx = ctx->all_configs->used;
    force_assert(dc->context_ndx > ctx->current->context_ndx);
    array_insert_unique(ctx->all_configs, (data_unset *)dc);
    dc->parent = ctx->current;
    vector_config_weak_push(&dc->parent->children, dc);
  }
  if (ctx->configs_stack.used > 0 && ctx->current->context_ndx == 0) {
    fprintf(stderr, "Cannot use conditionals inside a global { ... } block\n");
    exit(-1);
  }
  vector_config_weak_push(&ctx->configs_stack, ctx->current);
  ctx->current = dc;
}

static data_config *configparser_pop(config_t *ctx) {
  data_config *old = ctx->current;
  ctx->current = vector_config_weak_pop(&ctx->configs_stack);
  return old;
}

/* return a copied variable */
static data_unset *configparser_get_variable(config_t *ctx, const buffer *key) {
  data_unset *du;
  data_config *dc;

#if 0
  fprintf(stderr, "get var %s\n", key->ptr);
#endif
  for (dc = ctx->current; dc; dc = dc->parent) {
#if 0
    fprintf(stderr, "get var on block: %s\n", dc->key->ptr);
    array_print(dc->value, 0);
#endif
    if (NULL != (du = array_get_element(dc->value, key->ptr))) {
      return du->copy(du);
    }
  }
  return NULL;
}

/* op1 is to be eat/return by this function if success, op1->key is not cared
   op2 is left untouch, unreferenced
 */
data_unset *configparser_merge_data(data_unset *op1, const data_unset *op2) {
  /* type mismatch */
  if (op1->type != op2->type) {
    if (op1->type == TYPE_STRING && op2->type == TYPE_INTEGER) {
      data_string *ds = (data_string *)op1;
      buffer_append_int(ds->value, ((data_integer*)op2)->value);
      return op1;
    } else if (op1->type == TYPE_INTEGER && op2->type == TYPE_STRING) {
      data_string *ds = data_string_init();
      buffer_append_int(ds->value, ((data_integer*)op1)->value);
      buffer_append_string_buffer(ds->value, ((data_string*)op2)->value);
      op1->free(op1);
      return (data_unset *)ds;
    } else {
      fprintf(stderr, "data type mismatch, cannot merge\n");
      op1->free(op1);
      return NULL;
    }
  }

  switch (op1->type) {
    case TYPE_STRING:
      buffer_append_string_buffer(((data_string *)op1)->value, ((data_string *)op2)->value);
      break;
    case TYPE_INTEGER:
      ((data_integer *)op1)->value += ((data_integer *)op2)->value;
      break;
    case TYPE_ARRAY: {
      array *dst = ((data_array *)op1)->value;
      array *src = ((data_array *)op2)->value;
      data_unset *du;
      size_t i;

      for (i = 0; i < src->used; i ++) {
        du = (data_unset *)src->data[i];
        if (du) {
          if (du->is_index_key || buffer_is_empty(du->key) || !array_get_element(dst, du->key->ptr)) {
            array_insert_unique(dst, du->copy(du));
          } else {
            fprintf(stderr, "Duplicate array-key '%s'\n", du->key->ptr);
            op1->free(op1);
            return NULL;
          }
        }
      }
      break;
    default:
      force_assert(0);
      break;
    }
  }
  return op1;
}

static int configparser_remoteip_normalize_compat(buffer *rvalue) {
  /* $HTTP["remoteip"] IPv6 accepted with or without '[]' for config compat
   * http_request_host_normalize() expects IPv6 with '[]',
   * and config processing at runtime expects COMP_HTTP_REMOTE_IP
   * compared without '[]', so strip '[]' after normalization */
  buffer *b = buffer_init();
  int rc;

  if (rvalue->ptr[0] != '[') {
      buffer_append_string_len(b, CONST_STR_LEN("["));
      buffer_append_string_buffer(b, rvalue);
      buffer_append_string_len(b, CONST_STR_LEN("]"));
  } else {
      buffer_append_string_buffer(b, rvalue);
  }

  rc = http_request_host_normalize(b);

  if (0 == rc) {
    /* remove surrounding '[]' */
    size_t blen = buffer_string_length(b);
    if (blen > 1) buffer_copy_string_len(rvalue, b->ptr+1, blen-2);
  }

  buffer_free(b);
  return rc;
}

}

%parse_failure {
  ctx->ok = 0;
}

input ::= metalines.
metalines ::= metalines metaline.
metalines ::= .
metaline ::= varline.
metaline ::= global.
metaline ::= condlines(A) EOL. { A = NULL; }
metaline ::= include.
metaline ::= include_shell.
metaline ::= EOL.

%type       value                  {data_unset *}
%type       expression             {data_unset *}
%type       aelement               {data_unset *}
%type       condline               {data_config *}
%type       cond_else              {data_config *}
%type       condlines              {data_config *}
%type       aelements              {array *}
%type       array                  {array *}
%type       key                    {buffer *}
%type       stringop               {buffer *}

%type       cond                   {config_cond_t }

%destructor value                  { if ($$) $$->free($$); }
%destructor expression             { if ($$) $$->free($$); }
%destructor aelement               { if ($$) $$->free($$); }
%destructor aelements              { array_free($$); }
%destructor array                  { array_free($$); }
%destructor key                    { buffer_free($$); }
%destructor stringop               { buffer_free($$); }

%token_type                        {buffer *}
%token_destructor                  { buffer_free($$); }

varline ::= key(A) ASSIGN expression(B). {
  if (ctx->ok) {
    buffer_copy_buffer(B->key, A);
    if (strncmp(A->ptr, "env.", sizeof("env.") - 1) == 0) {
      fprintf(stderr, "Setting env variable is not supported in conditional %d %s: %s\n",
          ctx->current->context_ndx,
          ctx->current->key->ptr, A->ptr);
      ctx->ok = 0;
    } else if (NULL == array_get_element(ctx->current->value, B->key->ptr)) {
      array_insert_unique(ctx->current->value, B);
      B = NULL;
    } else {
      fprintf(stderr, "Duplicate config variable in conditional %d %s: %s\n",
              ctx->current->context_ndx,
              ctx->current->key->ptr, B->key->ptr);
      ctx->ok = 0;
      B->free(B);
      B = NULL;
    }
  }
  buffer_free(A);
  A = NULL;
}

varline ::= key(A) APPEND expression(B). {
  if (ctx->ok) {
    array *vars = ctx->current->value;
    data_unset *du;

    if (strncmp(A->ptr, "env.", sizeof("env.") - 1) == 0) {
      fprintf(stderr, "Appending env variable is not supported in conditional %d %s: %s\n",
          ctx->current->context_ndx,
          ctx->current->key->ptr, A->ptr);
      ctx->ok = 0;
    } else if (NULL != (du = array_extract_element(vars, A->ptr)) || NULL != (du = configparser_get_variable(ctx, A))) {
      du = configparser_merge_data(du, B);
      if (NULL == du) {
        ctx->ok = 0;
      }
      else {
        buffer_copy_buffer(du->key, A);
        array_insert_unique(ctx->current->value, du);
      }
      B->free(B);
    } else {
      buffer_copy_buffer(B->key, A);
      array_insert_unique(ctx->current->value, B);
    }
    buffer_free(A);
    A = NULL;
    B = NULL;
  }
}

key(A) ::= LKEY(B). {
  if (strchr(B->ptr, '.') == NULL) {
    A = buffer_init_string("var.");
    buffer_append_string_buffer(A, B);
    buffer_free(B);
    B = NULL;
  } else {
    A = B;
    B = NULL;
  }
}

expression(A) ::= expression(B) PLUS value(C). {
  A = NULL;
  if (ctx->ok) {
    A = configparser_merge_data(B, C);
    if (NULL == A) {
      ctx->ok = 0;
    }
    B = NULL;
    C->free(C);
    C = NULL;
  }
}

expression(A) ::= value(B). {
  A = B;
  B = NULL;
}

value(A) ::= key(B). {
  A = NULL;
  if (ctx->ok) {
    if (strncmp(B->ptr, "env.", sizeof("env.") - 1) == 0) {
      char *env;

      if (NULL != (env = getenv(B->ptr + 4))) {
        data_string *ds;
        ds = data_string_init();
        buffer_append_string(ds->value, env);
        A = (data_unset *)ds;
      }
      else {
        fprintf(stderr, "Undefined env variable: %s\n", B->ptr + 4);
        ctx->ok = 0;
      }
    } else if (NULL == (A = configparser_get_variable(ctx, B))) {
      fprintf(stderr, "Undefined config variable: %s\n", B->ptr);
      ctx->ok = 0;
    }
    buffer_free(B);
    B = NULL;
  }
}

value(A) ::= STRING(B). {
  A = (data_unset *)data_string_init();
  buffer_copy_buffer(((data_string *)(A))->value, B);
  buffer_free(B);
  B = NULL;
}

value(A) ::= INTEGER(B). {
  char *endptr;
  A = (data_unset *)data_integer_init();
  errno = 0;
  ((data_integer *)(A))->value = strtol(B->ptr, &endptr, 10);
  /* skip trailing whitespace */
  if (endptr != B->ptr) while (isspace(*endptr)) endptr++;
  if (0 != errno || *endptr != '\0') {
    fprintf(stderr, "error parsing number: '%s'\n", B->ptr);
    ctx->ok = 0;
  }
  buffer_free(B);
  B = NULL;
}
value(A) ::= array(B). {
  A = (data_unset *)data_array_init();
  array_free(((data_array *)(A))->value);
  ((data_array *)(A))->value = B;
  B = NULL;
}
array(A) ::= LPARAN RPARAN. {
  A = array_init();
}
array(A) ::= LPARAN aelements(B) RPARAN. {
  A = B;
  B = NULL;
}

aelements(A) ::= aelements(C) COMMA aelement(B). {
  A = NULL;
  if (ctx->ok) {
    if (buffer_is_empty(B->key) ||
        NULL == array_get_element(C, B->key->ptr)) {
      array_insert_unique(C, B);
      B = NULL;
    } else {
      fprintf(stderr, "Error: duplicate array-key: %s. Please get rid of the duplicate entry.\n",
              B->key->ptr);
      ctx->ok = 0;
      B->free(B);
      B = NULL;
    }

    A = C;
    C = NULL;
  }
}

aelements(A) ::= aelements(C) COMMA. {
  A = C;
  C = NULL;
}

aelements(A) ::= aelement(B). {
  A = NULL;
  if (ctx->ok) {
    A = array_init();
    array_insert_unique(A, B);
    B = NULL;
  }
}

aelement(A) ::= expression(B). {
  A = B;
  B = NULL;
}
aelement(A) ::= stringop(B) ARRAY_ASSIGN expression(C). {
  A = NULL;
  if (ctx->ok) {
    buffer_copy_buffer(C->key, B);
    buffer_free(B);
    B = NULL;

    A = C;
    C = NULL;
  }
}

eols ::= EOL.
eols ::= .

globalstart ::= GLOBAL. {
  data_config *dc;
  dc = (data_config *)array_get_element(ctx->srv->config_context, "global");
  force_assert(dc);
  configparser_push(ctx, dc, 0);
}

global ::= globalstart LCURLY metalines RCURLY. {
  force_assert(ctx->current);
  configparser_pop(ctx);
  force_assert(ctx->current);
}

condlines(A) ::= condlines(B) eols ELSE condline(C). {
  A = NULL;
  if (ctx->ok) {
    if (B->context_ndx >= C->context_ndx) {
      fprintf(stderr, "unreachable else condition\n");
      ctx->ok = 0;
    }
    if (B->cond == CONFIG_COND_ELSE) {
      fprintf(stderr, "unreachable condition following else catch-all\n");
      ctx->ok = 0;
    }
    C->prev = B;
    B->next = C;
    A = C;
    B = NULL;
    C = NULL;
  }
}

condlines(A) ::= condlines(B) eols ELSE cond_else(C). {
  A = NULL;
  if (ctx->ok) {
    if (B->context_ndx >= C->context_ndx) {
      fprintf(stderr, "unreachable else condition\n");
      ctx->ok = 0;
    }
    if (B->cond == CONFIG_COND_ELSE) {
      fprintf(stderr, "unreachable condition following else catch-all\n");
      ctx->ok = 0;
    }
  }
  if (ctx->ok) {
    size_t pos;
    data_config *dc;
    dc = (data_config *)array_extract_element(ctx->all_configs, C->key->ptr);
    force_assert(C == dc);
    buffer_copy_buffer(C->key, B->key);
    /*buffer_copy_buffer(C->comp_key, B->comp_key);*/
    /*C->string = buffer_init_buffer(B->string);*/
    pos = buffer_string_length(C->key)-buffer_string_length(B->string)-2;
    switch(B->cond) {
    case CONFIG_COND_NE:
      C->key->ptr[pos] = '='; /* opposite cond */
      /*buffer_copy_string_len(C->op, CONST_STR_LEN("=="));*/
      break;
    case CONFIG_COND_EQ:
      C->key->ptr[pos] = '!'; /* opposite cond */
      /*buffer_copy_string_len(C->op, CONST_STR_LEN("!="));*/
      break;
    case CONFIG_COND_NOMATCH:
      C->key->ptr[pos] = '='; /* opposite cond */
      /*buffer_copy_string_len(C->op, CONST_STR_LEN("=~"));*/
      break;
    case CONFIG_COND_MATCH:
      C->key->ptr[pos] = '!'; /* opposite cond */
      /*buffer_copy_string_len(C->op, CONST_STR_LEN("!~"));*/
      break;
    default: /* should not happen; CONFIG_COND_ELSE checked further above */
      force_assert(0);
    }

    if (NULL == (dc = (data_config *)array_get_element(ctx->all_configs, C->key->ptr))) {
      /* re-insert into ctx->all_configs with new C->key */
      array_insert_unique(ctx->all_configs, (data_unset *)C);
      C->prev = B;
      B->next = C;
    } else {
      fprintf(stderr, "unreachable else condition\n");
      ctx->ok = 0;
      C->free((data_unset *)C);
      C = dc;
    }

    A = C;
    B = NULL;
    C = NULL;
  }
}

condlines(A) ::= condline(B). {
  A = B;
  B = NULL;
}

condline(A) ::= context LCURLY metalines RCURLY. {
  A = NULL;
  if (ctx->ok) {
    data_config *cur;

    cur = ctx->current;
    configparser_pop(ctx);

    force_assert(cur && ctx->current);

    A = cur;
  }
}

cond_else(A) ::= context_else LCURLY metalines RCURLY. {
  A = NULL;
  if (ctx->ok) {
    data_config *cur;

    cur = ctx->current;
    configparser_pop(ctx);

    force_assert(cur && ctx->current);

    A = cur;
  }
}

context ::= DOLLAR SRVVARNAME(B) LBRACKET stringop(C) RBRACKET cond(E) expression(D). {
  data_config *dc;
  buffer *b, *rvalue, *op;

  if (ctx->ok && D->type != TYPE_STRING) {
    fprintf(stderr, "rvalue must be string");
    ctx->ok = 0;
  }

  if (ctx->ok) {
    switch(E) {
    case CONFIG_COND_NE:
      op = buffer_init_string("!=");
      break;
    case CONFIG_COND_EQ:
      op = buffer_init_string("==");
      break;
    case CONFIG_COND_NOMATCH:
      op = buffer_init_string("!~");
      break;
    case CONFIG_COND_MATCH:
      op = buffer_init_string("=~");
      break;
    default:
      force_assert(0);
      return; /* unreachable */
    }

    b = buffer_init();
    buffer_copy_buffer(b, ctx->current->key);
    buffer_append_string(b, "/");
    buffer_append_string_buffer(b, B);
    buffer_append_string_buffer(b, C);
    buffer_append_string_buffer(b, op);
    rvalue = ((data_string*)D)->value;
    buffer_append_string_buffer(b, rvalue);

    if (NULL != (dc = (data_config *)array_get_element(ctx->all_configs, b->ptr))) {
      configparser_push(ctx, dc, 0);
    } else {
      static const struct {
        comp_key_t comp;
        char *comp_key;
        size_t len;
      } comps[] = {
        { COMP_SERVER_SOCKET,      CONST_STR_LEN("SERVER[\"socket\"]"   ) },
        { COMP_HTTP_URL,           CONST_STR_LEN("HTTP[\"url\"]"        ) },
        { COMP_HTTP_HOST,          CONST_STR_LEN("HTTP[\"host\"]"       ) },
        { COMP_HTTP_REFERER,       CONST_STR_LEN("HTTP[\"referer\"]"    ) },
        { COMP_HTTP_USER_AGENT,    CONST_STR_LEN("HTTP[\"useragent\"]"  ) },
        { COMP_HTTP_USER_AGENT,    CONST_STR_LEN("HTTP[\"user-agent\"]"  ) },
        { COMP_HTTP_LANGUAGE,      CONST_STR_LEN("HTTP[\"language\"]"   ) },
        { COMP_HTTP_COOKIE,        CONST_STR_LEN("HTTP[\"cookie\"]"     ) },
        { COMP_HTTP_REMOTE_IP,     CONST_STR_LEN("HTTP[\"remoteip\"]"   ) },
        { COMP_HTTP_REMOTE_IP,     CONST_STR_LEN("HTTP[\"remote-ip\"]"   ) },
        { COMP_HTTP_QUERY_STRING,  CONST_STR_LEN("HTTP[\"querystring\"]") },
        { COMP_HTTP_QUERY_STRING,  CONST_STR_LEN("HTTP[\"query-string\"]") },
        { COMP_HTTP_REQUEST_METHOD, CONST_STR_LEN("HTTP[\"request-method\"]") },
        { COMP_HTTP_SCHEME,        CONST_STR_LEN("HTTP[\"scheme\"]"     ) },
        { COMP_UNSET, NULL, 0 },
      };
      size_t i;

      dc = data_config_init();

      buffer_copy_buffer(dc->key, b);
      buffer_copy_buffer(dc->op, op);
      buffer_copy_buffer(dc->comp_key, B);
      buffer_append_string_len(dc->comp_key, CONST_STR_LEN("[\""));
      buffer_append_string_buffer(dc->comp_key, C);
      buffer_append_string_len(dc->comp_key, CONST_STR_LEN("\"]"));
      dc->cond = E;

      for (i = 0; comps[i].comp_key; i ++) {
        if (buffer_is_equal_string(
              dc->comp_key, comps[i].comp_key, comps[i].len)) {
          dc->comp = comps[i].comp;
          break;
        }
      }
      if (COMP_UNSET == dc->comp) {
        fprintf(stderr, "error comp_key %s", dc->comp_key->ptr);
        ctx->ok = 0;
      }
      else if (COMP_HTTP_REMOTE_IP == dc->comp
               && (dc->cond == CONFIG_COND_EQ || dc->cond == CONFIG_COND_NE)) {
        char * const slash = strchr(rvalue->ptr, '/'); /* CIDR mask */
        char * const colon = strchr(rvalue->ptr, ':'); /* IPv6 */
        if (NULL != slash && slash == rvalue->ptr){/*(skip AF_UNIX /path/file)*/
        }
        else if (NULL != slash) {
          char *nptr;
          const unsigned long nm_bits = strtoul(slash + 1, &nptr, 10);
          if (*nptr || 0 == nm_bits || nm_bits > (NULL != colon ? 128 : 32)) {
            /*(also rejects (slash+1 == nptr) which results in nm_bits = 0)*/
            fprintf(stderr, "invalid or missing netmask: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
          else {
            int rc;
            buffer_string_set_length(rvalue, (size_t)(slash - rvalue->ptr)); /*(truncate)*/
            rc = (NULL == colon)
              ? http_request_host_normalize(rvalue)
              : configparser_remoteip_normalize_compat(rvalue);
            buffer_append_string_len(rvalue, CONST_STR_LEN("/"));
            buffer_append_int(rvalue, (int)nm_bits);
            if (0 != rc) {
              fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
              ctx->ok = 0;
            }
          }
        }
        else {
          int rc = (NULL == colon)
            ? http_request_host_normalize(rvalue)
            : configparser_remoteip_normalize_compat(rvalue);
          if (0 != rc) {
            fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
        }
      }
      else if (COMP_SERVER_SOCKET == dc->comp) {
        /*(redundant with parsing in network.c; not actually required here)*/
        if (rvalue->ptr[0] != ':' /*(network.c special-cases ":" and "[]")*/
            && !(rvalue->ptr[0] == '[' && rvalue->ptr[1] == ']')) {
          if (http_request_host_normalize(rvalue)) {
            fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
        }
      }
      else if (COMP_HTTP_HOST == dc->comp) {
        if (dc->cond == CONFIG_COND_EQ || dc->cond == CONFIG_COND_NE) {
          if (http_request_host_normalize(rvalue)) {
            fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
        }
      }

      if (ctx->ok) switch(E) {
      case CONFIG_COND_NE:
      case CONFIG_COND_EQ:
        dc->string = buffer_init_buffer(rvalue);
        break;
      case CONFIG_COND_NOMATCH:
      case CONFIG_COND_MATCH: {
#ifdef HAVE_PCRE_H
        const char *errptr;
        int erroff, captures;

        if (NULL == (dc->regex =
            pcre_compile(rvalue->ptr, 0, &errptr, &erroff, NULL))) {
          dc->string = buffer_init_string(errptr);
          dc->cond = CONFIG_COND_UNSET;

          fprintf(stderr, "parsing regex failed: %s -> %s at offset %d\n",
              rvalue->ptr, errptr, erroff);

          ctx->ok = 0;
        } else if (NULL == (dc->regex_study =
            pcre_study(dc->regex, 0, &errptr)) &&
                   errptr != NULL) {
          fprintf(stderr, "studying regex failed: %s -> %s\n",
              rvalue->ptr, errptr);
          ctx->ok = 0;
        } else if (0 != (pcre_fullinfo(dc->regex, dc->regex_study, PCRE_INFO_CAPTURECOUNT, &captures))) {
          fprintf(stderr, "getting capture count for regex failed: %s\n",
              rvalue->ptr);
          ctx->ok = 0;
        } else if (captures > 9) {
          fprintf(stderr, "Too many captures in regex, use (?:...) instead of (...): %s\n",
              rvalue->ptr);
          ctx->ok = 0;
        } else {
          dc->string = buffer_init_buffer(rvalue);
        }
#else
        fprintf(stderr, "can't handle '$%s[%s] =~ ...' as you compiled without pcre support. \n"
                        "(perhaps just a missing pcre-devel package ?) \n",
                        B->ptr, C->ptr);
        ctx->ok = 0;
 #endif
        break;
      }

      default:
        fprintf(stderr, "unknown condition for $%s[%s]\n",
                        B->ptr, C->ptr);
        ctx->ok = 0;
        break;
      }

      if (ctx->ok) {
        configparser_push(ctx, dc, 1);
      } else {
        dc->free((data_unset*) dc);
      }
    }

    buffer_free(b);
    buffer_free(op);
    buffer_free(B);
    B = NULL;
    buffer_free(C);
    C = NULL;
    D->free(D);
    D = NULL;
  }
}

context_else ::= . {
  if (ctx->ok) {
    data_config *dc = data_config_init();
    buffer_copy_buffer(dc->key, ctx->current->key);
    buffer_append_string_len(dc->key, CONST_STR_LEN("/"));
    buffer_append_string_len(dc->key, CONST_STR_LEN("else_tmp_token"));
    dc->cond = CONFIG_COND_ELSE;
    configparser_push(ctx, dc, 1);
  }
}

cond(A) ::= EQ. {
  A = CONFIG_COND_EQ;
}
cond(A) ::= MATCH. {
  A = CONFIG_COND_MATCH;
}
cond(A) ::= NE. {
  A = CONFIG_COND_NE;
}
cond(A) ::= NOMATCH. {
  A = CONFIG_COND_NOMATCH;
}

stringop(A) ::= expression(B). {
  A = NULL;
  if (ctx->ok) {
    if (B->type == TYPE_STRING) {
      A = buffer_init_buffer(((data_string*)B)->value);
    } else if (B->type == TYPE_INTEGER) {
      A = buffer_init();
      buffer_copy_int(A, ((data_integer *)B)->value);
    } else {
      fprintf(stderr, "operand must be string");
      ctx->ok = 0;
    }
  }
  B->free(B);
  B = NULL;
}

include ::= INCLUDE stringop(A). {
  if (ctx->ok) {
    if (0 != config_parse_file(ctx->srv, ctx, A->ptr)) {
      ctx->ok = 0;
    }
    buffer_free(A);
    A = NULL;
  }
}

include_shell ::= INCLUDE_SHELL stringop(A). {
  if (ctx->ok) {
    if (0 != config_parse_cmd(ctx->srv, ctx, A->ptr)) {
      ctx->ok = 0;
    }
    buffer_free(A);
    A = NULL;
  }
}
