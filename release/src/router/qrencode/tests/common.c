#include <stdio.h>
#include <stdlib.h>
#include "common.h"

static int tests = 0;
static int failed = 0;
int assertionFailed = 0;
int assertionNum = 0;
static const char *testName = NULL;
static const char *testFunc = NULL;

const char levelChar[4] = {'L', 'M', 'Q', 'H'};
const char *modeStr[5] = {"nm", "an", "8", "kj", "st"};

int ncmpBin(char *correct, BitStream *bstream, size_t len)
{
	int bit;
	size_t i;
	char *p;

	if(len != BitStream_size(bstream)) {
		printf("Length is not match: %zu, %zu expected.\n", BitStream_size(bstream), len);
		return -1;
	}

	p = correct;
	i = 0;
	while(*p != '\0') {
		while(*p == ' ') {
			p++;
		}
		bit = (*p == '1')?1:0;
		if(bstream->data[i] != bit) return -1;
		i++;
		p++;
		if(i == len) break;
	}

	return 0;
}

int cmpBin(char *correct, BitStream *bstream)
{
	size_t len = 0;
	char *p;


	for(p = correct; *p != '\0'; p++) {
		if(*p != ' ') len++;
	}
	return ncmpBin(correct, bstream, len);
}

void testInit(int tests)
{
	printf("1..%d\n", tests);
}

void testStartReal(const char *func, const char *name)
{
	tests++;
	testName = name;
	testFunc = func;
	assertionFailed = 0;
	assertionNum = 0;
	//printf("_____%d: %s: %s...\n", tests, func, name);
}

void testEnd(int result)
{
	if(result) {
		printf("not ok %d %s: %s\n", tests, testFunc, testName);
		failed++;
	} else {
		printf("ok %d %s: %s\n", tests, testFunc, testName);
	}
}

void testFinish(void)
{
	if(assertionFailed) {
		printf("not ok %d %s: %s (%d assertions failed.)\n", tests, testFunc, testName, assertionFailed);
		failed++;
	} else {
		printf("ok %d %s: %s (%d assertions passed.)\n", tests, testFunc, testName, assertionNum);
	}
}

void testReport(int expectedTests)
{
	printf("Total %d tests, %d fails.\n", tests, failed);
	if(failed) exit(-1);
	if(expectedTests != tests) {
		printf("WARNING: the number of the executed tests (%d) is not equal to the expecetd (%d).\n", tests, expectedTests);
	}
}

int testNum(void)
{
	return tests;
}

void printBinary(unsigned char *data, int length)
{
	int i;

	for(i=0; i<length; i++) {
		printf(data[i]?"1":"0");
	}
	printf("\n");
}

void printBstream(BitStream *bstream)
{
	printBinary(bstream->data, BitStream_size(bstream));
}

void printQRinput(QRinput *input)
{
	QRinput_List *list;
	int i;

	list = input->head;
	while(list != NULL) {
		for(i=0; i<list->size; i++) {
			printf("0x%02x,", list->data[i]);
		}
		list = list->next;
	}
	printf("\n");
}

void printQRinputInfo(QRinput *input)
{
	QRinput_List *list;
	BitStream *b;
	int i, ret;

	printf("QRinput info:\n");
	printf(" version: %d\n", input->version);
	printf(" level  : %c\n", levelChar[input->level]);
	list = input->head;
	i = 0;
	while(list != NULL) {
		i++;
		list = list->next;
	}
	printf("  chunks: %d\n", i);
	b = BitStream_new();
	ret = QRinput_mergeBitStream(input, b);
	if(ret == 0) {
		printf("  bitstream-size: %zu\n", BitStream_size(b));
		BitStream_free(b);
	}

	list = input->head;
	i = 0;
	while(list != NULL) {
		printf("\t#%d: mode = %s, size = %d\n", i, modeStr[list->mode], list->size);
		i++;
		list = list->next;
	}
}

void printQRinputStruct(QRinput_Struct *s)
{
	QRinput_InputList *list;
	int i = 1;

	printf("Struct size: %d\n", s->size);
	printf("Struct parity: %08x\n", s->parity);
	for(list = s->head; list != NULL; list = list->next) {
		printf("Symbol %d - ", i);
		printQRinputInfo(list->input);
		i++;
	}
}

void printFrame(int width, unsigned char *frame)
{
	int x, y;

	for(y=0; y<width; y++) {
		for(x=0; x<width; x++) {
			printf("%02x ", *frame++);
		}
		printf("\n");
	}
}

void printQRcode(QRcode *code)
{
	printFrame(code->width, code->data);
}

void printQRRawCodeFromQRinput(QRinput *input)
{
	QRRawCode *raw;
	int i;

	puts("QRRawCode dump image:");
	raw = QRraw_new(input);
	if(raw == NULL) {
		puts("Failed to generate QRRawCode from this input.\n");
		return;
	}
	for(i=0; i<raw->dataLength; i++) {
		printf(" %02x", raw->datacode[i]);
	}
	for(i=0; i<raw->eccLength; i++) {
		printf(" %02x", raw->ecccode[i]);
	}
	printf("\n");
	QRraw_free(raw);
}

#if HAVE_SDL
/* Experimental debug function */
/* You can call show_QRcode(QRcode *code) to display the QR Code from anywhere
 * in test code using SDL. */
#include <SDL.h>

static void draw_QRcode(QRcode *qrcode, int ox, int oy, int margin, int size, SDL_Surface *surface)
{
	int x, y, width;
	unsigned char *p;
	SDL_Rect rect;
	Uint32 color[2];

	color[0] = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
	color[1] = SDL_MapRGBA(surface->format, 0, 0, 0, 255);
	SDL_FillRect(surface, NULL, color[0]);

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
			SDL_FillRect(surface, &rect, color[*p&1]);
			p++;
		}
	}
}

void show_QRcode(QRcode *qrcode)
{
	SDL_Event event;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *surface;
	SDL_Texture *texture;

	if(!SDL_WasInit(SDL_INIT_VIDEO)) {
		SDL_Init(SDL_INIT_VIDEO);
		atexit(SDL_Quit);
	}
	int width = (qrcode->width + 4 * 2) * 4; //maring = 4, size = 4
	SDL_CreateWindowAndRenderer(width, width, SDL_WINDOW_SHOWN, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	surface = SDL_CreateRGBSurface(0, width, width, 32, 0, 0, 0, 0);

	draw_QRcode(qrcode, 0, 0, 4, 4, surface);

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	fprintf(stderr, "Press any key on the QR Code window to proceed.\n");

	int loop = 1;
	while(loop) {
		SDL_WaitEvent(&event);
		if(event.type == SDL_KEYDOWN) {
			loop = 0;
		}
	}

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
}
#else
void show_QRcode(QRcode *qrcode) {
}
#endif
