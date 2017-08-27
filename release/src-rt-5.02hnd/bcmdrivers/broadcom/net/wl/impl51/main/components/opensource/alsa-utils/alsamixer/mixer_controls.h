#ifndef MIXER_CONTROLS_H_INCLUDED
#define MIXER_CONTROLS_H_INCLUDED

#include <alsa/asoundlib.h>

struct control {
	snd_mixer_elem_t *elem;
	char *name;
	unsigned int flags;
#define TYPE_PVOLUME	(1u << 4)
#define TYPE_CVOLUME	(1u << 5)
#define TYPE_PSWITCH	(1u << 6)
#define TYPE_CSWITCH	(1u << 7)
#define TYPE_ENUM	(1u << 8)
#define HAS_VOLUME_0	(1u << 9)
#define HAS_VOLUME_1	(1u << 10)
#define HAS_PSWITCH_0	(1u << 11)
#define HAS_PSWITCH_1	(1u << 12)
#define HAS_CSWITCH_0	(1u << 13)
#define HAS_CSWITCH_1	(1u << 14)
#define IS_MULTICH	(1u << 15)
#define IS_ACTIVE	(1u << 16)
#define MULTICH_MASK	(0x0000f)
	snd_mixer_selem_channel_id_t volume_channels[2];
	snd_mixer_selem_channel_id_t pswitch_channels[2];
	snd_mixer_selem_channel_id_t cswitch_channels[2];
	unsigned int enum_channel_bits;
};

extern struct control *controls;
extern unsigned int controls_count;

bool are_there_any_controls(void);
void create_controls(void);
void free_controls(void);

#endif
