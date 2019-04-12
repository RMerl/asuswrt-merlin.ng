#include <yaml.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void print_escaped(yaml_char_t * str, size_t length);

int main(int argc, char *argv[])
{
    FILE *input;
    yaml_parser_t parser;
    yaml_event_t event;

    if (argc == 1)
        input = stdin;
    else if (argc == 2)
        input = fopen(argv[1], "rb");
    else {
        fprintf(stderr, "Usage: libyaml-parser [<input-file>]\n");
        return 1;
    }
    assert(input);

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Could not initialize the parser object\n");
        return 1;
    }
    yaml_parser_set_input_file(&parser, input);

    while (1) {
        yaml_event_type_t type;
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "Parse error: %s\n", parser.problem);
            return 1;
        }
        type = event.type;

        if (type == YAML_NO_EVENT)
            printf("???\n");
        else if (type == YAML_STREAM_START_EVENT)
            printf("+STR\n");
        else if (type == YAML_STREAM_END_EVENT)
            printf("-STR\n");
        else if (type == YAML_DOCUMENT_START_EVENT) {
            printf("+DOC");
            if (!event.data.document_start.implicit)
                printf(" ---");
            printf("\n");
        }
        else if (type == YAML_DOCUMENT_END_EVENT) {
            printf("-DOC");
            if (!event.data.document_end.implicit)
                printf(" ...");
            printf("\n");
        }
        else if (type == YAML_MAPPING_START_EVENT) {
            printf("+MAP");
            if (event.data.mapping_start.anchor)
                printf(" &%s", event.data.mapping_start.anchor);
            if (event.data.mapping_start.tag)
                printf(" <%s>", event.data.mapping_start.tag);
            printf("\n");
        }
        else if (type == YAML_MAPPING_END_EVENT)
            printf("-MAP\n");
        else if (type == YAML_SEQUENCE_START_EVENT) {
            printf("+SEQ");
            if (event.data.sequence_start.anchor)
                printf(" &%s", event.data.sequence_start.anchor);
            if (event.data.sequence_start.tag)
                printf(" <%s>", event.data.sequence_start.tag);
            printf("\n");
        }
        else if (type == YAML_SEQUENCE_END_EVENT)
            printf("-SEQ\n");
        else if (type == YAML_SCALAR_EVENT) {
            printf("=VAL");
            if (event.data.scalar.anchor)
                printf(" &%s", event.data.scalar.anchor);
            if (event.data.scalar.tag)
                printf(" <%s>", event.data.scalar.tag);
            switch (event.data.scalar.style) {
            case YAML_PLAIN_SCALAR_STYLE:
                printf(" :");
                break;
            case YAML_SINGLE_QUOTED_SCALAR_STYLE:
                printf(" '");
                break;
            case YAML_DOUBLE_QUOTED_SCALAR_STYLE:
                printf(" \"");
                break;
            case YAML_LITERAL_SCALAR_STYLE:
                printf(" |");
                break;
            case YAML_FOLDED_SCALAR_STYLE:
                printf(" >");
                break;
            case YAML_ANY_SCALAR_STYLE:
                abort();
            }
            print_escaped(event.data.scalar.value, event.data.scalar.length);
            printf("\n");
        }
        else if (type == YAML_ALIAS_EVENT)
            printf("=ALI *%s\n", event.data.alias.anchor);
        else
            abort();

        yaml_event_delete(&event);

        if (type == YAML_STREAM_END_EVENT)
            break;
    }

    assert(!fclose(input));
    yaml_parser_delete(&parser);
    fflush(stdout);

    return 0;
}

void print_escaped(yaml_char_t * str, size_t length)
{
    int i;
    char c;

    for (i = 0; i < length; i++) {
        c = *(str + i);
        if (c == '\\')
            printf("\\\\");
        else if (c == '\0')
            printf("\\0");
        else if (c == '\b')
            printf("\\b");
        else if (c == '\n')
            printf("\\n");
        else if (c == '\r')
            printf("\\r");
        else if (c == '\t')
            printf("\\t");
        else
            printf("%c", c);
    }
}
