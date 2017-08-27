#include <alsa/asoundlib.h>

void dump_event_filter(snd_seq_client_info_t *client_info) {
	int i, b;

	for (i = 0; i <= 255;) {
		b = snd_seq_client_info_event_filter_check(client_info, i);
		i++;
		printf("%c%s%s", (b ? 'X' : '.'),
			(i % 8 == 0 ? " " : ""),
			(i % 32 == 0 ? "\n" : ""));
	}
	printf("\n");
}

int main(void) {
	snd_seq_client_info_t *client_info;

	snd_seq_client_info_alloca(&client_info);

	printf("first client_info_event_filter                   :\n");
	dump_event_filter(client_info);

	snd_seq_client_info_event_filter_add(client_info, SND_SEQ_EVENT_NOTEON);
	printf("after snd_seq_client_info_event_filter_add(client_info, SND_SEQ_EVENT_NOTEON);\n");
	dump_event_filter(client_info);

	snd_seq_client_info_event_filter_add(client_info, SND_SEQ_EVENT_PGMCHANGE);
	printf("after snd_seq_client_info_event_filter_add(client_info, SND_SEQ_EVENT_PGMCHANGE);\n");
	dump_event_filter(client_info);

	snd_seq_client_info_event_filter_del(client_info, SND_SEQ_EVENT_NOTEON);
	printf("after snd_seq_client_info_event_filter_del(client_info, SND_SEQ_EVENT_NOTEON);\n");
	dump_event_filter(client_info);

	snd_seq_client_info_event_filter_clear(client_info);
	printf("after snd_seq_client_info_event_filter_clear(client_info);\n");
	dump_event_filter(client_info);

	snd_seq_client_info_event_filter_add(client_info, SND_SEQ_EVENT_NOTEON);
	printf("after snd_seq_client_info_event_filter_add(client_info, SND_SEQ_EVENT_NOTEON);\n");
	dump_event_filter(client_info);

	return 0;
}
