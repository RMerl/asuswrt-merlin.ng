#include <stdio.h>
#include <string.h>
#include "../include/asoundlib.h"

int main(void)
{
	int idx, dev, err;
	snd_ctl_t *handle;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_rawmidi_info_t *rawmidiinfo;
	char str[128];

	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);
	snd_rawmidi_info_alloca(&rawmidiinfo);

	idx = -1;
	while (1) {
		if ((err = snd_card_next(&idx)) < 0) {
			printf("Card next error: %s\n", snd_strerror(err));
			break;
		}
		if (idx < 0)
			break;
		sprintf(str, "hw:CARD=%i", idx);
		if ((err = snd_ctl_open(&handle, str, 0)) < 0) {
			printf("Open error: %s\n", snd_strerror(err));
			continue;
		}
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
			printf("HW info error: %s\n", snd_strerror(err));
			continue;
		}
		printf("Soundcard #%i:\n", idx + 1);
		printf("  card - %i\n", snd_ctl_card_info_get_card(info));
		printf("  id - '%s'\n", snd_ctl_card_info_get_id(info));
		printf("  driver - '%s'\n", snd_ctl_card_info_get_driver(info));
		printf("  name - '%s'\n", snd_ctl_card_info_get_name(info));
		printf("  longname - '%s'\n", snd_ctl_card_info_get_longname(info));
		printf("  mixername - '%s'\n", snd_ctl_card_info_get_mixername(info));
		printf("  components - '%s'\n", snd_ctl_card_info_get_components(info));
		dev = -1;
		while (1) {
			snd_pcm_sync_id_t sync;
			if ((err = snd_ctl_pcm_next_device(handle, &dev)) < 0) {
				printf("  PCM next device error: %s\n", snd_strerror(err));
				break;
			}
			if (dev < 0)
				break;
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
				printf("  PCM info error: %s\n", snd_strerror(err));
				continue;
			}
			printf("PCM info, device #%i:\n", dev);
			printf("  device - %i\n", snd_pcm_info_get_device(pcminfo));
			printf("  subdevice - %i\n", snd_pcm_info_get_subdevice(pcminfo));
			printf("  stream - %i\n", snd_pcm_info_get_stream(pcminfo));
			printf("  card - %i\n", snd_pcm_info_get_card(pcminfo));
			printf("  id - '%s'\n", snd_pcm_info_get_id(pcminfo));
			printf("  name - '%s'\n", snd_pcm_info_get_name(pcminfo));
			printf("  subdevice name - '%s'\n", snd_pcm_info_get_subdevice_name(pcminfo));
			printf("  class - 0x%x\n", snd_pcm_info_get_class(pcminfo));
			printf("  subclass - 0x%x\n", snd_pcm_info_get_subclass(pcminfo));
			printf("  subdevices count - %i\n", snd_pcm_info_get_subdevices_count(pcminfo));
			printf("  subdevices avail - %i\n", snd_pcm_info_get_subdevices_avail(pcminfo));
			sync = snd_pcm_info_get_sync(pcminfo);
			printf("  sync - 0x%x,0x%x,0x%x,0x%x\n", sync.id32[0], sync.id32[1], sync.id32[2], sync.id32[3]);
		}
		dev = -1;
		while (1) {
			if ((err = snd_ctl_rawmidi_next_device(handle, &dev)) < 0) {
				printf("  RAWMIDI next device error: %s\n", snd_strerror(err));
				break;
			}
			if (dev < 0)
				break;
			snd_rawmidi_info_set_device(rawmidiinfo, dev);
			snd_rawmidi_info_set_subdevice(rawmidiinfo, 0);
			snd_rawmidi_info_set_stream(rawmidiinfo, SND_RAWMIDI_STREAM_OUTPUT);
			if ((err = snd_ctl_rawmidi_info(handle, rawmidiinfo)) < 0) {
				printf("  RAWMIDI info error: %s\n", snd_strerror(err));
				continue;
			}
			printf("RAWMIDI info, device #%i:\n", dev);
			printf("  device - %i\n", snd_rawmidi_info_get_device(rawmidiinfo));
			printf("  subdevice - %i\n", snd_rawmidi_info_get_subdevice(rawmidiinfo));
			printf("  stream - %i\n", snd_rawmidi_info_get_stream(rawmidiinfo));
			printf("  card - %i\n", snd_rawmidi_info_get_card(rawmidiinfo));
			printf("  flags - 0x%x\n", snd_rawmidi_info_get_flags(rawmidiinfo));
			printf("  id - '%s'\n", snd_rawmidi_info_get_id(rawmidiinfo));
			printf("  name - '%s'\n", snd_rawmidi_info_get_name(rawmidiinfo));
			printf("  subname - '%s'\n", snd_rawmidi_info_get_subdevice_name(rawmidiinfo));
			printf("  subdevices count - %i\n", snd_rawmidi_info_get_subdevices_count(rawmidiinfo));
			printf("  subdevices avail - %i\n", snd_rawmidi_info_get_subdevices_avail(rawmidiinfo));
		}
		snd_ctl_close(handle);
	}
	
	snd_config_update_free_global();
	return 0;
}
