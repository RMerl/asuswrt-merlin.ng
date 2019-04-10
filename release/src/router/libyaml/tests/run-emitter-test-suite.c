#include <yaml.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

bool get_line(FILE * input, char *line);
char *get_anchor(char sigil, char *line, char *anchor);
char *get_tag(char *line, char *tag);
void get_value(char *line, char *value, int *style);

int main(int argc, char *argv[])
{
    FILE *input;
    yaml_emitter_t emitter;
    yaml_event_t event;

    int canonical = 0;
    int unicode = 0;
    char line[1024];

    if (argc == 1)
        input = stdin;
    else if (argc == 2)
        input = fopen(argv[1], "rb");
    else {
        fprintf(stderr, "Usage: libyaml-emitter [<input-file>]\n");
        return 1;
    }
    assert(input);

    if (!yaml_emitter_initialize(&emitter)) {
        fprintf(stderr, "Could not initalize the emitter object\n");
        return 1;
    }
    yaml_emitter_set_output_file(&emitter, stdout);
    yaml_emitter_set_canonical(&emitter, canonical);
    yaml_emitter_set_unicode(&emitter, unicode);

    while (get_line(input, line)) {
        int ok;
        char anchor[256];
        char tag[256];
        int implicit;

        if (strncmp(line, "+STR", 4) == 0) {
            ok = yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
        }
        else if (strncmp(line, "-STR", 4) == 0) {
            ok = yaml_stream_end_event_initialize(&event);
        }
        else if (strncmp(line, "+DOC", 4) == 0) {
            implicit = strncmp(line, "+DOC ---", 8) != 0;
            ok = yaml_document_start_event_initialize(&event, NULL, NULL, NULL, implicit);
        }
        else if (strncmp(line, "-DOC", 4) == 0) {
            implicit = strncmp(line, "-DOC ...", 8) != 0;
            ok = yaml_document_end_event_initialize(&event, implicit);
        }
        else if (strncmp(line, "+MAP", 4) == 0) {
            ok = yaml_mapping_start_event_initialize(&event, (yaml_char_t *)
                                                     get_anchor('&', line, anchor), (yaml_char_t *)
                                                     get_tag(line, tag), 0, YAML_BLOCK_MAPPING_STYLE);
        }
        else if (strncmp(line, "-MAP", 4) == 0) {
            ok = yaml_mapping_end_event_initialize(&event);
        }
        else if (strncmp(line, "+SEQ", 4) == 0) {
            ok = yaml_sequence_start_event_initialize(&event, (yaml_char_t *)
                                                      get_anchor('&', line, anchor), (yaml_char_t *)
                                                      get_tag(line, tag), 0, YAML_BLOCK_SEQUENCE_STYLE);
        }
        else if (strncmp(line, "-SEQ", 4) == 0) {
            ok = yaml_sequence_end_event_initialize(&event);
        }
        else if (strncmp(line, "=VAL", 4) == 0) {
            char value[1024];
            int style;

            get_value(line, value, &style);
            implicit = (get_tag(line, tag) == NULL);

            ok = yaml_scalar_event_initialize(&event, (yaml_char_t *)
                                              get_anchor('&', line, anchor), (yaml_char_t *) get_tag(line, tag), (yaml_char_t *) value, -1, implicit, implicit, style);
        }
        else if (strncmp(line, "=ALI", 4) == 0) {
            ok = yaml_alias_event_initialize(&event, (yaml_char_t *)
                                             get_anchor('*', line, anchor)
                );
        }
        else {
            fprintf(stderr, "Unknown event: '%s'\n", line);
            fflush(stdout);
            return 1;
        }

        if (!ok)
            goto event_error;
        if (!yaml_emitter_emit(&emitter, &event))
            goto emitter_error;
    }

    assert(!fclose(input));
    yaml_emitter_delete(&emitter);
    fflush(stdout);

    return 0;

  emitter_error:
    switch (emitter.error) {
    case YAML_MEMORY_ERROR:
        fprintf(stderr, "Memory error: Not enough memory for emitting\n");
        break;
    case YAML_WRITER_ERROR:
        fprintf(stderr, "Writer error: %s\n", emitter.problem);
        break;
    case YAML_EMITTER_ERROR:
        fprintf(stderr, "Emitter error: %s\n", emitter.problem);
        break;
    default:
        /*
         * Couldn't happen. 
         */
        fprintf(stderr, "Internal error\n");
        break;
    }
    yaml_emitter_delete(&emitter);
    return 1;

  event_error:
    fprintf(stderr, "Memory error: Not enough memory for creating an event\n");
    yaml_emitter_delete(&emitter);
    return 1;
}

bool get_line(FILE * input, char *line)
{
    char *newline;

    if (!fgets(line, 1024 - 1, input))
        return false;

    if ((newline = strchr(line, '\n')) == NULL) {
        fprintf(stderr, "Line too long: '%s'", line);
        abort();
    }
    *newline = '\0';

    return true;
}

char *get_anchor(char sigil, char *line, char *anchor)
{
    char *start;
    char *end;
    if ((start = strchr(line, sigil)) == NULL)
        return NULL;
    start++;
    if ((end = strchr(start, ' ')) == NULL)
        end = line + strlen(line);
    memcpy(anchor, start, end - start);
    anchor[end - start] = '\0';
    return anchor;
}

char *get_tag(char *line, char *tag)
{
    char *start;
    char *end;
    if ((start = strchr(line, '<')) == NULL)
        return NULL;
    if ((end = strchr(line, '>')) == NULL)
        return NULL;
    memcpy(tag, start + 1, end - start - 1);
    tag[end - start - 1] = '\0';
    return tag;
}

void get_value(char *line, char *value, int *style)
{
    int i = 0;
    char *c;
    char *start = NULL;
    char *end = line + strlen(line);

    for (c = line + 4; c < end; c++) {
        if (*c == ' ') {
            start = c + 1;
            if (*start == ':')
                *style = YAML_PLAIN_SCALAR_STYLE;
            else if (*start == '\'')
                *style = YAML_SINGLE_QUOTED_SCALAR_STYLE;
            else if (*start == '"')
                *style = YAML_DOUBLE_QUOTED_SCALAR_STYLE;
            else if (*start == '|')
                *style = YAML_LITERAL_SCALAR_STYLE;
            else if (*start == '>')
                *style = YAML_FOLDED_SCALAR_STYLE;
            else {
                start = NULL;
                continue;
            }
            start++;
            break;
        }
    }
    if (!start)
        abort();

    for (c = start; c < end; c++) {
        if (*c == '\\') {
            if (*++c == '\\')
                value[i++] = '\\';
            else if (*c == '0')
                value[i++] = '\0';
            else if (*c == 'b')
                value[i++] = '\b';
            else if (*c == 'n')
                value[i++] = '\n';
            else if (*c == 'r')
                value[i++] = '\r';
            else if (*c == 't')
                value[i++] = '\t';
            else
                abort();
        }
        else
            value[i++] = *c;
    }
    value[i] = '\0';
}
