#ifndef TEXTBOX_H_INCLUDED
#define TEXTBOX_H_INCLUDED

void show_error(const char *msg, int err);
void show_alsa_error(const char *msg, int err);
void show_text(const char *const *text_lines, unsigned int count,
	       const char *title);
void show_textfile(const char *file_name);

#endif
