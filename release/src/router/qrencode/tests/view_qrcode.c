#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <SDL.h>
#include <getopt.h>
#include <errno.h>
#include "../config.h"
#include "../qrspec.h"
#include "../qrinput.h"
#include "../split.h"
#include "../qrencode_inner.h"

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture = NULL;
static SDL_Surface *surface = NULL;
static int casesensitive = 1;
static int eightbit = 0;
static int version = 0;
static int size = 4;
static int margin = -1;
static int structured = 0;
static int micro = 0;
static int colorize = 0;
static QRecLevel level = QR_ECLEVEL_L;
static QRencodeMode hint = QR_MODE_8;

static char **textv;
static int textc;

static const struct option options[] = {
	{"help"         , no_argument      , NULL, 'h'},
	{"level"        , required_argument, NULL, 'l'},
	{"size"         , required_argument, NULL, 's'},
	{"symversion"   , required_argument, NULL, 'v'},
	{"margin"       , required_argument, NULL, 'm'},
	{"structured"   , no_argument      , NULL, 'S'},
	{"kanji"        , no_argument      , NULL, 'k'},
	{"casesensitive", no_argument      , NULL, 'c'},
	{"ignorecase"   , no_argument      , NULL, 'i'},
	{"8bit"         , no_argument      , NULL, '8'},
	{"micro"        , no_argument      , NULL, 'M'},
	{"version"      , no_argument      , NULL, 'V'},
	{NULL, 0, NULL, 0}
};

static char *optstring = "hl:s:v:m:Skci8MV";

static char levelChar[4] = {'L', 'M', 'Q', 'H'};
static void usage(int help, int longopt)
{
	fprintf(stderr,
"view_qrcode version %s\n"
"Copyright (C) 2008, 2009, 2010 Kentaro Fukuchi\n", VERSION);
	if(help) {
		if(longopt) {
			fprintf(stderr,
"Usage: view_qrcode [OPTION]... [STRING]\n"
"Encode input data in a QR Code and display.\n\n"
"  -h, --help   display the help message. -h displays only the help of short\n"
"               options.\n\n"
"  -s NUMBER, --size=NUMBER\n"
"               specify module size in dots (pixels). (default=3)\n\n"
"  -l {LMQH}, --level={LMQH}\n"
"               specify error correction level from L (lowest) to H (highest).\n"
"               (default=L)\n\n"
"  -v NUMBER, --symversion=NUMBER\n"
"               specify the version of the symbol. See SYMBOL VERSIONS for more\n"
"               information. (default=auto)\n\n"
"  -m NUMBER, --margin=NUMBER\n"
"               specify the width of the margins. (default=4 (2 for Micro QR)))\n\n"
"  -S, --structured\n"
"               make structured symbols. Version must be specified.\n\n"
"  -k, --kanji  assume that the input text contains kanji (shift-jis).\n\n"
"  -c, --casesensitive\n"
"               encode lower-case alphabet characters in 8-bit mode. (default)\n\n"
"  -i, --ignorecase\n"
"               ignore case distinctions and use only upper-case characters.\n\n"
"  -8, --8bit   encode entire data in 8-bit mode. -k, -c and -i will be ignored.\n\n"
"  -M, --micro  encode in a Micro QR Code. (experimental)\n\n"
"  -V, --version\n"
"               display the version number and copyrights of the qrencode.\n\n"
"  [STRING]     input data. If it is not specified, data will be taken from\n"
"               standard input.\n\n"
"*SYMBOL VERSIONS\n"
"               The symbol versions of QR Code range from Version 1 to Version\n"
"               40. Each version has a different module configuration or number\n"
"               of modules, ranging from Version 1 (21 x 21 modules) up to\n"
"               Version 40 (177 x 177 modules). Each higher version number\n"
"               comprises 4 additional modules per side by default. See\n"
"               http://www.qrcode.com/en/about/version.html for a detailed\n"
"               version list.\n"
			);
		} else {
			fprintf(stderr,
"Usage: view_qrcode [OPTION]... [STRING]\n"
"Encode input data in a QR Code and display.\n\n"
"  -h           display this message.\n"
"  --help       display the usage of long options.\n"
"  -s NUMBER    specify module size in dots (pixels). (default=3)\n"
"  -l {LMQH}    specify error correction level from L (lowest) to H (highest).\n"
"               (default=L)\n"
"  -v NUMBER    specify the version of the symbol. (default=auto)\n"
"  -m NUMBER    specify the width of the margins. (default=4 (2 for Micro))\n"
"  -S           make structured symbols. Version must be specified.\n"
"  -k           assume that the input text contains kanji (shift-jis).\n"
"  -c           encode lower-case alphabet characters in 8-bit mode. (default)\n"
"  -i           ignore case distinctions and use only upper-case characters.\n"
"  -8           encode entire data in 8-bit mode. -k, -c and -i will be ignored.\n"
"  -M           encode in a Micro QR Code.\n"
"  -V           display the version number and copyrights of the qrencode.\n"
"  [STRING]     input data. If it is not specified, data will be taken from\n"
"               standard input.\n"
			);
		}
	}
}

#define MAX_DATA_SIZE (7090 * 16) /* from the specification */
static unsigned char *readStdin(int *length)
{
	unsigned char *buffer;
	int ret;

	buffer = (unsigned char *)malloc(MAX_DATA_SIZE + 1);
	if(buffer == NULL) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(EXIT_FAILURE);
	}

	ret = fread(buffer, 1, MAX_DATA_SIZE, stdin);
	if(ret == 0) {
		fprintf(stderr, "No input data.\n");
		exit(EXIT_FAILURE);
	}
	if(feof(stdin) == 0) {
		fprintf(stderr, "Input data is too large.\n");
		exit(EXIT_FAILURE);
	}

	buffer[ret] = '\0';
	*length = ret;

	return buffer;
}

static void draw_QRcode(QRcode *qrcode, int ox, int oy)
{
	int x, y, width;
	unsigned char *p;
	SDL_Rect rect;
	Uint32 color[8];
	int col;

	color[0] = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
	color[1] = SDL_MapRGBA(surface->format,   0,   0,   0, 255);
	color[2] = SDL_MapRGBA(surface->format, 192, 192, 255, 255);
	color[3] = SDL_MapRGBA(surface->format,   0,   0,  64, 255);
	color[4] = SDL_MapRGBA(surface->format, 255, 255, 192, 255);
	color[5] = SDL_MapRGBA(surface->format,  64,  64,   0, 255);
	color[6] = SDL_MapRGBA(surface->format, 255, 192, 192, 255);
	color[7] = SDL_MapRGBA(surface->format,  64,   0,   0, 255);

	ox += margin * size;
	oy += margin * size;
	width = qrcode->width;
	p = qrcode->data;
	for(y=0; y<width; y++) {
		for(x=0; x<width; x++) {
			rect.x = ox + x * size;
			rect.y = oy + y * size;
			rect.w = size;
			rect.h = size;
			if(!colorize) {
				col = 0;
			} else {
				if(*p & 0x80) {
					col = 6;
				} else if(*p & 0x02) {
					col = 4;
				} else {
					col = 2;
				}
			}
			col += (*p & 1);
			SDL_FillRect(surface, &rect, color[col]);
			p++;
		}
	}
}

static void draw_singleQRcode(QRinput *stream, int mask)
{
	QRcode *qrcode;
	int width;

	QRinput_setVersionAndErrorCorrectionLevel(stream, version, level);
	if(micro) {
		qrcode = QRcode_encodeMaskMQR(stream, mask);
	} else {
		qrcode = QRcode_encodeMask(stream, mask);
	}
	if(qrcode == NULL) {
		width = (11 + margin * 2) * size;
		fprintf(stderr, "Input data does not fit to this setting.\n");
	} else {
		version = qrcode->version;
		width = (qrcode->width + margin * 2) * size;
	}

	SDL_SetWindowSize(window, width, width);
	if(surface != NULL) {
		SDL_FreeSurface(surface);
	}
	surface = SDL_CreateRGBSurface(0, width, width, 32, 0, 0, 0, 0);
	SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));
	if(qrcode) {
		draw_QRcode(qrcode, 0, 0);
	}
	if(texture != NULL) {
		SDL_DestroyTexture(texture);
	}
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	QRcode_free(qrcode);
}

static void draw_structuredQRcode(QRinput_Struct *s)
{
	int i, w, h, n, x, y;
	int swidth;
	QRcode_List *qrcodes, *p;

	qrcodes = QRcode_encodeInputStructured(s);
	if(qrcodes == NULL) return;

	swidth = (qrcodes->code->width + margin * 2) * size;
	n = QRcode_List_size(qrcodes);
	w = (n < 4)?n:4;
	h = (n - 1) / 4 + 1;

	SDL_SetWindowSize(window, swidth * w, swidth * h);
	if(surface != NULL) {
		SDL_FreeSurface(surface);
	}
	surface = SDL_CreateRGBSurface(0, swidth * w, swidth * h, 32, 0, 0, 0, 0);
	SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

	p = qrcodes;
	for(i=0; i<n; i++) {
		x = (i % 4) * swidth;
		y = (i / 4) * swidth;
		draw_QRcode(p->code, x, y);
		p = p->next;
	}
	if(texture != NULL) {
		SDL_DestroyTexture(texture);
	}
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	QRcode_List_free(qrcodes);
}

static void draw_structuredQRcodeFromText(int argc, char **argv)
{
	QRinput_Struct *s;
	QRinput *input;
	int i, ret;

	s = QRinput_Struct_new();
	if(s == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}
	for(i=0; i<argc; i++) {
		input = QRinput_new2(version, level);
		if(input == NULL) {
			fprintf(stderr, "Failed to allocate memory.\n");
			exit(EXIT_FAILURE);
		}
		if(eightbit) {
			ret = QRinput_append(input, QR_MODE_8, strlen(argv[i]), (unsigned char *)argv[i]);
		} else {
			ret = Split_splitStringToQRinput(argv[i], input, hint, casesensitive);
		}
		if(ret < 0) {
			perror("Encoding the input string");
			exit(EXIT_FAILURE);
		}
		ret = QRinput_Struct_appendInput(s, input);
		if(ret < 0) {
			perror("Encoding the input string");
			exit(EXIT_FAILURE);
		}
	}
	ret = QRinput_Struct_insertStructuredAppendHeaders(s);
	if(ret < 0) {
		fprintf(stderr, "Too many inputs.\n");
	}

	draw_structuredQRcode(s);
	QRinput_Struct_free(s);
}

static void draw_structuredQRcodeFromQRinput(QRinput *stream)
{
	QRinput_Struct *s;

	QRinput_setVersion(stream, version);
	QRinput_setErrorCorrectionLevel(stream, level);
	s = QRinput_splitQRinputToStruct(stream);
	if(s != NULL) {
		draw_structuredQRcode(s);
		QRinput_Struct_free(s);
	} else {
		fprintf(stderr, "Input data is too large for this setting.\n");
	}
}

static void view(int mode, QRinput *input)
{
	int flag = 1;
	int mask = -1;
	SDL_Event event;
	int loop;
	int codeChanged = 1;

	while(flag) {
		if(codeChanged) {
			if(mode) {
				draw_structuredQRcodeFromText(textc, textv);
			} else {
				if(structured) {
					draw_structuredQRcodeFromQRinput(input);
				} else {
					draw_singleQRcode(input, mask);
				}
			}
			if(mode || structured) {
				printf("Version %d, Level %c.\n", version, levelChar[level]);
			} else {
				printf("Version %d, Level %c, Mask %d.\n", version, levelChar[level], mask);
			}
		}
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		loop = 1;
		codeChanged = 0;
		while(loop) {
			SDL_WaitEvent(&event);
			if(event.type == SDL_KEYDOWN) {
				codeChanged = 1;
				switch(event.key.keysym.sym) {
				case SDLK_RIGHT:
					version++;
					if(version > QRSPEC_VERSION_MAX)
						version = QRSPEC_VERSION_MAX;
					loop = 0;
					break;
				case SDLK_LEFT:
					version--;
					if(version < 1)
						version = 1;
					loop = 0;
					break;
				case SDLK_UP:
					size++;
					loop = 0;
					break;
				case SDLK_DOWN:
					size--;
					if(size < 1) size = 1;
					loop = 0;
					break;
				case SDLK_0:
				case SDLK_1:
				case SDLK_2:
				case SDLK_3:
				case SDLK_4:
				case SDLK_5:
				case SDLK_6:
				case SDLK_7:
					if(!mode && !structured) {
						mask = (event.key.keysym.sym - SDLK_0);
						loop = 0;
					}
					break;
				case SDLK_8:
					if(!mode && !structured) {
						mask = -1;
						loop = 0;
					}
					break;
				case SDLK_9:
					if(!mode && !structured) {
						mask = -2;
						loop = 0;
					}
					break;
				case SDLK_l:
					level = QR_ECLEVEL_L;
					loop = 0;
					break;
				case SDLK_m:
					level = QR_ECLEVEL_M;
					loop = 0;
					break;
				case SDLK_h:
					level = QR_ECLEVEL_H;
					loop = 0;
					break;
				case SDLK_q:
					level = QR_ECLEVEL_Q;
					loop = 0;
					break;
				case SDLK_c:
					colorize ^= 1;
					loop = 0;
					break;
				case SDLK_ESCAPE:
					loop = 0;
					flag = 0;
					break;
				default:
					break;
				}
				if(event.type == SDL_QUIT) {
					loop = 0;
					flag = 0;
				}
			}
			if (event.type == SDL_WINDOWEVENT) {
				switch (event.window.event) {
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_EXPOSED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					case SDL_WINDOWEVENT_RESIZED:
						loop = 0;
						break;
					default:
						break;
				}
			}
		}
	}
}

static void view_simple(const unsigned char *str, int length)
{
	QRinput *input;
	int ret;

	if(micro) {
		input = QRinput_newMQR(version, level);
	} else {
		input = QRinput_new2(version, level);
	}
	if(input == NULL) {
		fprintf(stderr, "Memory allocation error.\n");
		exit(EXIT_FAILURE);
	}
	if(eightbit) {
		ret = QRinput_append(input, QR_MODE_8, length, str);
	} else {
		ret = Split_splitStringToQRinput((char *)str, input, hint, casesensitive);
	}
	if(ret < 0) {
		perror("Encoding the input string");
		exit(EXIT_FAILURE);
	}

	view(0, input);

	QRinput_free(input);
}

static void view_multiText(char **argv, int argc)
{
	textc = argc;
	textv = argv;

	view(1, NULL);
}

int main(int argc, char **argv)
{
	int opt, lindex = -1;
	unsigned char *intext = NULL;
	int length = 0;
	int ret;

	while((opt = getopt_long(argc, argv, optstring, options, &lindex)) != -1) {
		switch(opt) {
			case 'h':
				if(lindex == 0) {
					usage(1, 1);
				} else {
					usage(1, 0);
				}
				exit(EXIT_SUCCESS);
				break;
			case 's':
				size = atoi(optarg);
				if(size <= 0) {
					fprintf(stderr, "Invalid size: %d\n", size);
					exit(EXIT_FAILURE);
				}
				break;
			case 'v':
				version = atoi(optarg);
				if(version < 0) {
					fprintf(stderr, "Invalid version: %d\n", version);
					exit(EXIT_FAILURE);
				}
				break;
			case 'l':
				switch(*optarg) {
					case 'l':
					case 'L':
						level = QR_ECLEVEL_L;
						break;
					case 'm':
					case 'M':
						level = QR_ECLEVEL_M;
						break;
					case 'q':
					case 'Q':
						level = QR_ECLEVEL_Q;
						break;
					case 'h':
					case 'H':
						level = QR_ECLEVEL_H;
						break;
					default:
						fprintf(stderr, "Invalid level: %s\n", optarg);
						exit(EXIT_FAILURE);
						break;
				}
				break;
			case 'm':
				margin = atoi(optarg);
				if(margin < 0) {
					fprintf(stderr, "Invalid margin: %d\n", margin);
					exit(EXIT_FAILURE);
				}
				break;
			case 'S':
				structured = 1;
			case 'k':
				hint = QR_MODE_KANJI;
				break;
			case 'c':
				casesensitive = 1;
				break;
			case 'i':
				casesensitive = 0;
				break;
			case '8':
				eightbit = 1;
				break;
			case 'M':
				micro = 1;
				break;
			case 'V':
				usage(0, 0);
				exit(EXIT_SUCCESS);
				break;
			default:
				fprintf(stderr, "Try `view_qrcode --help' for more information.\n");
				exit(EXIT_FAILURE);
				break;
		}
	}

	if(argc == 1) {
		usage(1, 0);
		exit(EXIT_SUCCESS);
	}

	if(optind < argc) {
		intext = (unsigned char *)argv[optind];
		length = strlen((char *)intext);
	}
	if(intext == NULL) {
		intext = readStdin(&length);
	}

	if(micro && version > MQRSPEC_VERSION_MAX) {
		fprintf(stderr, "Version should be less or equal to %d.\n", MQRSPEC_VERSION_MAX);
		exit(EXIT_FAILURE);
	} else if(!micro && version > QRSPEC_VERSION_MAX) {
		fprintf(stderr, "Version should be less or equal to %d.\n", QRSPEC_VERSION_MAX);
		exit(EXIT_FAILURE);
	}

	if(margin < 0) {
		if(micro) {
			margin = 2;
		} else {
			margin = 4;
		}
	}

	if(micro) {
		if(version == 0) {
			fprintf(stderr, "Version must be specified to encode a Micro QR Code symbol.\n");
			exit(EXIT_FAILURE);
		}
		if(structured) {
			fprintf(stderr, "Micro QR Code does not support structured symbols.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(structured && version == 0) {
		fprintf(stderr, "Version must be specified to encode structured symbols.\n");
		exit(EXIT_FAILURE);
	}

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Failed initializing SDL: %s\n", SDL_GetError());
		return -1;
	}

	ret = SDL_CreateWindowAndRenderer(100, 100, SDL_WINDOW_SHOWN, &window, &renderer);
	if(ret < 0) {
		fprintf(stderr, "Failed to create a window: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	if(structured && (argc - optind > 1)) {
		view_multiText(argv + optind, argc - optind);
	} else {
		view_simple(intext, length);
	}

	SDL_Quit();

	return 0;
}
