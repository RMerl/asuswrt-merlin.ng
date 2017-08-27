/*
   iecset - change IEC958 status bits on ALSA
   Copyright (C) 2003 by Takashi Iwai <tiwai@suse.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <ctype.h>
#include <alsa/asoundlib.h>

void dump_iec958(snd_aes_iec958_t *iec);

static int get_bool(const char *str)
{
	if (strncmp(str, "yes", 3) == 0 ||
	    strncmp(str, "YES", 3) == 0 ||
	    strncmp(str, "on", 2) == 0 ||
	    strncmp(str, "ON", 2) == 0 ||
	    strncmp(str, "true", 4) == 0 ||
	    strncmp(str, "TRUE", 4) == 0 ||
	    *str == '1')
		return 1;
	return 0;
}

enum {
	CMD_BOOL, CMD_BOOL_INV, CMD_INT
};

enum {
	IDX_PRO, IDX_NOAUDIO, IDX_RATE, IDX_UNLOCK, IDX_SBITS, IDX_WORD, IDX_EMP, IDX_CAT, IDX_NOCOPY, IDX_ORIG,
	IDX_LAST
};

struct cmdtbl {
	const char *name;
	int idx;
	int type;
	const char *desc;
};

static const struct cmdtbl cmds[] = {
	{ "pro", IDX_PRO, CMD_BOOL,
	  "professional (common)\n\toff = consumer mode, on = professional mode" },
	{ "aud", IDX_NOAUDIO, CMD_BOOL_INV,
	  "audio (common)\n\ton = audio mode, off = non-audio mode" },
	{ "rat", IDX_RATE, CMD_INT,
	  "rate (common)\n\tsample rate in Hz (0 = not indicated)" },
	{ "emp", IDX_EMP, CMD_INT,
	  "emphasis (common)\n\t0 = none, 1 = 50/15us, 2 = CCITT" },
	{ "loc", IDX_UNLOCK, CMD_BOOL_INV,
	  "lock (prof.)\n\toff = rate unlocked, on = rate locked" },
	{ "sbi", IDX_SBITS, CMD_INT,
	  "sbits (prof.)\n\tsample bits 2 = 20bit, 4 = 24bit, 6 = undef" },
	{ "wor", IDX_WORD, CMD_INT,
	  "wordlength (prof.)\n\t0=no, 2=22-18bit, 4=23-19bit, 5=24-20bit, 6=20-16bit" },
	{ "cat", IDX_CAT, CMD_INT,
	  "category (consumer)\n\t0-0x7f" },
	{ "cop", IDX_NOCOPY, CMD_BOOL_INV,
	  "copyright (consumer)\n\toff = non-copyright, on = copyright" },
	{ "ori", IDX_ORIG, CMD_BOOL,
	  "original (consumer)\n\toff = 1st-gen, on = original" },
};


static void error(const char *s, int err)
{
	fprintf(stderr, "%s: %s\n", s, snd_strerror(err));
}


static void usage(void)
{
	int i;

	printf("Usage: iecset [options] [cmd arg...]\n");
	printf("Options:\n");
	printf("    -D device   specifies the control device to use\n");
	printf("    -c card     specifies the card number to use (equiv. with -Dhw:#)\n");
	printf("    -n number   specifies the control index number (default = 0)\n");
	printf("    -x          dump the dump the AESx hex code for IEC958 PCM parameters\n");
	printf("    -i          read commands from stdin\n");
	printf("Commands:\n");
	for (i = 0; i < (int)(sizeof(cmds)/sizeof(cmds[0])); i++) {
		printf("    %s\n", cmds[i].desc);
	}
}


/*
 * parse iecset commands
 */
static void parse_command(int *parms, const char *c, const char *arg)
{
	int i;

	for (i = 0; i < (int)(sizeof(cmds)/sizeof(cmds[0])); i++) {
		if (strncmp(c, cmds[i].name, strlen(cmds[i].name)) == 0) {
			int val;
			switch (cmds[i].type) {
			case CMD_BOOL:
				val = get_bool(arg);
				break;
			case CMD_BOOL_INV:
				val = !get_bool(arg);
				break;
			case CMD_INT:
			default:
				val = (int)strtol(arg, NULL, 0);
				break;
			}
			parms[cmds[i].idx] = val;
			return;
		}
	}
}

static char *skipspace(char *line)
{
	char *p;
	for (p = line; *p && isspace(*p); p++)
		;
	return p;
}

/*
 * parse iecset commands from the file
 */
static void parse_file(int *parms, FILE *fp)
{
	char line[1024], *cmd, *arg;
	while (fgets(line, sizeof(line), fp) != NULL) {
		cmd = skipspace(line);
		if (*cmd == '#' || ! *cmd)
			continue;
		for (arg = cmd; *arg && !isspace(*arg); arg++)
			;
		if (! *arg)
			continue;
		*arg++ = 0;
		arg = skipspace(arg);
		if (! *arg)
			continue;
		parse_command(parms, cmd, arg);
	}
}

/* update iec958 status values
 * return non-zero if the values are modified
 */
static int update_iec958_status(snd_aes_iec958_t *iec958, int *parms)
{
	int changed = 0;
	if (parms[IDX_PRO] >= 0) {
		if (parms[IDX_PRO])
			iec958->status[0] |= IEC958_AES0_PROFESSIONAL;
		else
			iec958->status[0] &= ~IEC958_AES0_PROFESSIONAL;
		changed = 1;
	}
	if (parms[IDX_NOAUDIO] >= 0) {
		if (parms[IDX_NOAUDIO])
			iec958->status[0] |= IEC958_AES0_NONAUDIO;
		else
			iec958->status[0] &= ~IEC958_AES0_NONAUDIO;
		changed = 1;
	}
	if (parms[IDX_RATE] >= 0) {
		if (iec958->status[0] & IEC958_AES0_PROFESSIONAL) {
			iec958->status[0] &= ~IEC958_AES0_PRO_FS;
			switch (parms[IDX_RATE]) {
			case 44100:
				iec958->status[0] |= IEC958_AES0_PRO_FS_44100;
				break;
			case 48000:
				iec958->status[0] |= IEC958_AES0_PRO_FS_48000;
				break;
			case 32000:
				iec958->status[0] |= IEC958_AES0_PRO_FS_32000;
				break;
			}
		} else {
			iec958->status[3] &= ~IEC958_AES3_CON_FS;
			switch (parms[IDX_RATE]) {
			case 22050:
				iec958->status[3] |= IEC958_AES3_CON_FS_22050;
				break;
			case 24000:
				iec958->status[3] |= IEC958_AES3_CON_FS_24000;
				break;
			case 32000:
				iec958->status[3] |= IEC958_AES3_CON_FS_32000;
				break;
			case 44100:
				iec958->status[3] |= IEC958_AES3_CON_FS_44100;
				break;
			case 48000:
				iec958->status[3] |= IEC958_AES3_CON_FS_48000;
				break;
			case 88200:
				iec958->status[3] |= IEC958_AES3_CON_FS_88200;;
				break;
			case 96000:
				iec958->status[3] |= IEC958_AES3_CON_FS_96000;
				break;
			case 176400:
				iec958->status[3] |= IEC958_AES3_CON_FS_176400;
				break;
			case 192000:
				iec958->status[3] |= IEC958_AES3_CON_FS_192000;
				break;
			case 768000:
				iec958->status[3] |= IEC958_AES3_CON_FS_768000;
				break;
			default:
				iec958->status[3] |= IEC958_AES3_CON_FS_NOTID;
				break;
			}
		}
		changed = 1;
	}
	if (parms[IDX_NOCOPY] >= 0) {
		if (! (iec958->status[0] & IEC958_AES0_PROFESSIONAL)) {
			if (parms[IDX_NOCOPY])
				iec958->status[0] |= IEC958_AES0_CON_NOT_COPYRIGHT;
			else
				iec958->status[0] &= ~IEC958_AES0_CON_NOT_COPYRIGHT;
		}
		changed = 1;
	}
	if (parms[IDX_ORIG] >= 0) {
		if (! (iec958->status[0] & IEC958_AES0_PROFESSIONAL)) {
			if (parms[IDX_ORIG])
				iec958->status[1] |= IEC958_AES1_CON_ORIGINAL;
			else
				iec958->status[1] &= ~IEC958_AES1_CON_ORIGINAL;
		}
		changed = 1;
	}
	if (parms[IDX_EMP] >= 0) {
		if (iec958->status[0] & IEC958_AES0_PROFESSIONAL) {
			iec958->status[0] &= ~IEC958_AES0_PRO_EMPHASIS;
			switch (parms[IDX_EMP]) {
			case 0:
				iec958->status[0] |= IEC958_AES0_PRO_EMPHASIS_NONE;
				break;
			case 1:
				iec958->status[0] |= IEC958_AES0_PRO_EMPHASIS_5015;
				break;
			case 2:
				iec958->status[0] |= IEC958_AES0_PRO_EMPHASIS_CCITT;
				break;
			}
		} else {
			if (parms[IDX_EMP])
				iec958->status[0] |= IEC958_AES0_CON_EMPHASIS_5015;
			else
				iec958->status[0] &= ~IEC958_AES0_CON_EMPHASIS_5015;
		}
		changed = 1;
	}
	if (parms[IDX_UNLOCK] >= 0) {
		if (iec958->status[0] & IEC958_AES0_PROFESSIONAL) {
			if (parms[IDX_UNLOCK])
				iec958->status[0] |= IEC958_AES0_PRO_FREQ_UNLOCKED;
			else
				iec958->status[0] &= ~IEC958_AES0_PRO_FREQ_UNLOCKED;
		}
		changed = 1;
	}
	if (parms[IDX_SBITS] >= 0) {
		if (iec958->status[0] & IEC958_AES0_PROFESSIONAL) {
			iec958->status[2] &= ~IEC958_AES2_PRO_SBITS;
			iec958->status[2] |= parms[IDX_SBITS] & 7;
		}
		changed = 1;
	}
	if (parms[IDX_WORD] >= 0) {
		if (iec958->status[0] & IEC958_AES0_PROFESSIONAL) {
			iec958->status[2] &= ~IEC958_AES2_PRO_WORDLEN;
			iec958->status[2] |= (parms[IDX_WORD] & 7) << 3;
		}
		changed = 1;
	}
	if (parms[IDX_CAT] >= 0) {
		if (! (iec958->status[0] & IEC958_AES0_PROFESSIONAL)) {
			iec958->status[1] &= ~IEC958_AES1_CON_CATEGORY;
			iec958->status[1] |= parms[IDX_CAT] & 0x7f;
		}
		changed = 1;
	}

	return changed;
}
		

int main(int argc, char **argv)
{
	const char *dev = "default";
	const char *spdif_str = SND_CTL_NAME_IEC958("", PLAYBACK, DEFAULT);
	int spdif_index = -1;
	snd_ctl_t *ctl;
	snd_ctl_elem_list_t *clist;
	snd_ctl_elem_id_t *cid;
	snd_ctl_elem_value_t *cval;
	snd_aes_iec958_t iec958;
	int from_stdin = 0;
	int dumphex = 0;
	int i, c, err;
	unsigned int controls, cidx;
	char tmpname[32];
	int parms[IDX_LAST];

	for (i = 0; i < IDX_LAST; i++)
		parms[i] = -1; /* not set */

	while ((c = getopt(argc, argv, "D:c:n:xhi")) != -1) {
		switch (c) {
		case 'D':
			dev = optarg;
			break;
		case 'c':
			i = atoi(optarg);
			if (i < 0 || i >= 32) {
				fprintf(stderr, "invalid card index %d\n", i);
				return 1;
			}
			sprintf(tmpname, "hw:%d", i);
			dev = tmpname;
			break;
		case 'n':
			spdif_index = atoi(optarg);
			break;
		case 'x':
			dumphex = 1;
			break;
		case 'i':
			from_stdin = 1;
			break;
		default:
			usage();
			return 1;
		}
	}

	if ((err = snd_ctl_open(&ctl, dev, 0)) < 0) {
		error("snd_ctl_open", err);
		return 1;
	}

	snd_ctl_elem_list_alloca(&clist);
	if ((err = snd_ctl_elem_list(ctl, clist)) < 0) {
		error("snd_ctl_elem_list", err);
		return 1;
	}
	if ((err = snd_ctl_elem_list_alloc_space(clist, snd_ctl_elem_list_get_count(clist))) < 0) {
		error("snd_ctl_elem_list_alloc_space", err);
		return 1;
	}
	if ((err = snd_ctl_elem_list(ctl, clist)) < 0) {
		error("snd_ctl_elem_list", err);
		return 1;
	}

	controls = snd_ctl_elem_list_get_used(clist);
	for (cidx = 0; cidx < controls; cidx++) {
		if (!strcmp(snd_ctl_elem_list_get_name(clist, cidx), spdif_str))
			if (spdif_index < 0 ||
			    snd_ctl_elem_list_get_index(clist, cidx) == spdif_index)
				break;
	}
	if (cidx >= controls) {
		fprintf(stderr, "control \"%s\" (index %d) not found\n",
			spdif_str, spdif_index);
		return 1;
	}

	snd_ctl_elem_id_alloca(&cid);
	snd_ctl_elem_list_get_id(clist, cidx, cid);
	snd_ctl_elem_value_alloca(&cval);
	snd_ctl_elem_value_set_id(cval, cid);
	if ((err = snd_ctl_elem_read(ctl, cval)) < 0) {
		error("snd_ctl_elem_read", err);
		return 1;
	}

	snd_ctl_elem_value_get_iec958(cval, &iec958);

	/* parse from stdin */
	if (from_stdin)
		parse_file(parms, stdin);

	/* parse commands */
	for (c = optind; c < argc - 1; c += 2)
		parse_command(parms, argv[c], argv[c + 1]);

	if (update_iec958_status(&iec958, parms)) {
		/* store the values */
		snd_ctl_elem_value_set_iec958(cval, &iec958);
		if ((err = snd_ctl_elem_write(ctl, cval)) < 0) {
			error("snd_ctl_elem_write", err);
			return 1;
		}
		if ((err = snd_ctl_elem_read(ctl, cval)) < 0) {
			error("snd_ctl_elem_write", err);
			return 1;
		}
		snd_ctl_elem_value_get_iec958(cval, &iec958);
	}

	if (dumphex)
		printf("AES0=0x%02x,AES1=0x%02x,AES2=0x%02x,AES3=0x%02x\n",
		       iec958.status[0], iec958.status[1], iec958.status[2], iec958.status[3]);
	else
		dump_iec958(&iec958);

	snd_ctl_close(ctl);
	return 0;
}
