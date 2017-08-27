#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/asoundlib.h"
#include <signal.h>

static void usage(void)
{
	fprintf(stderr, "usage: rawmidi [options]\n");
	fprintf(stderr, "  options:\n");
	fprintf(stderr, "    -v: verbose mode\n");
	fprintf(stderr, "    -i device-id : test ALSA input device\n");
	fprintf(stderr, "    -o device-id : test ALSA output device\n");
	fprintf(stderr, "    -I node      : test input node\n");
	fprintf(stderr, "    -O node      : test output node\n");
	fprintf(stderr, "    -t: test midi thru\n");
	fprintf(stderr, "  example:\n");
	fprintf(stderr, "    rawmidi -i hw:0,0 -O /dev/midi1\n");
	fprintf(stderr, "    tests input for card 0, device 0, using snd_rawmidi API\n");
	fprintf(stderr, "    and /dev/midi1 using file descriptors\n");
}

int stop=0;

void sighandler(int dum)
{
	stop=1;
}

int main(int argc,char** argv)
{
	int i;
	int err;
	int thru=0;
	int verbose = 0;
	char *device_in = NULL;
	char *device_out = NULL;
	char *node_in = NULL;
	char *node_out = NULL;
	
	int fd_in = -1,fd_out = -1;
	snd_rawmidi_t *handle_in = 0,*handle_out = 0;
	
	if (argc==1) {
		usage();
		exit(0);
	}
	
	for (i = 1 ; i<argc ; i++) {
		if (argv[i][0]=='-') {
			switch (argv[i][1]) {
				case 'h':
					usage();
					break;
				case 'v':
					verbose = 1;
					break;
				case 't':
					thru = 1;
					break;
				case 'i':
					if (i + 1 < argc)
						device_in = argv[++i];
					break;
				case 'I':
					if (i + 1 < argc)
						node_in = argv[++i];
					break;
				case 'o':
					if (i + 1 < argc)
						device_out = argv[++i];
					break;
				case 'O':
					if (i + 1 < argc)
						node_out = argv[++i];
					break;
			}			
		}
	}

	if (verbose) {
		fprintf(stderr,"Using: \n");
		fprintf(stderr,"Input: ");
		if (device_in) {
			fprintf(stderr,"device %s\n",device_in);
		}else if (node_in){
			fprintf(stderr,"%s\n",node_in);	
		}else{
			fprintf(stderr,"NONE\n");
		}
		fprintf(stderr,"Output: ");
		if (device_out) {
			fprintf(stderr,"device %s\n",device_out);
		}else if (node_out){
			fprintf(stderr,"%s\n",node_out);		
		}else{
			fprintf(stderr,"NONE\n");
		}
	}
	
	if (device_in) {
		err = snd_rawmidi_open(&handle_in,NULL,device_in,0);	
		if (err) {
			fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",device_in,err);
		}
	}
	if (node_in && (!node_out || strcmp(node_out,node_in))) {
		fd_in = open(node_in,O_RDONLY);
		if (fd_in<0) {
			fprintf(stderr,"open %s for input failed\n",node_in);
		}	
	}

	signal(SIGINT,sighandler);

	if (device_out) {
		err = snd_rawmidi_open(NULL,&handle_out,device_out,0);
		if (err) {
			fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",device_out,err);
		}
	}
	if (node_out && (!node_in || strcmp(node_out,node_in))) {
		fd_out = open(node_out,O_WRONLY);		
		if (fd_out<0) {
			fprintf(stderr,"open %s for output failed\n",node_out);
		}	
	}

	if (node_in && node_out && strcmp(node_out,node_in)==0) {
		fd_in = fd_out = open(node_out,O_RDWR);		
		if (fd_out<0) {
			fprintf(stderr,"open %s for input and output failed\n",node_out);
		}		
	}



	if (!thru) {
		if (handle_in || fd_in!=-1) {
			fprintf(stderr,"Read midi in\n");
			fprintf(stderr,"Press ctrl-c to stop\n");
		}

		if (handle_in) {
			unsigned char ch;
			while (!stop) {
				snd_rawmidi_read(handle_in,&ch,1);
				if (verbose) {
					fprintf(stderr,"read %02x\n",ch);
				}
			}
		}
		if (fd_in!=-1) {
			unsigned char ch;
			while (!stop) {
				read(fd_in,&ch,1);
				if (verbose) {
					fprintf(stderr,"read %02x\n",ch);
				}
			}	
		}

		if (handle_out || fd_out!=-1) {
			fprintf(stderr,"Writing note on / note off\n");
		}

		if (handle_out) {
			unsigned char ch;
			ch=0x90; snd_rawmidi_write(handle_out,&ch,1);
			ch=60;   snd_rawmidi_write(handle_out,&ch,1);
			ch=100;  snd_rawmidi_write(handle_out,&ch,1);
			snd_rawmidi_drain(handle_out);
			sleep(1);
			ch=0x90; snd_rawmidi_write(handle_out,&ch,1);
			ch=60;   snd_rawmidi_write(handle_out,&ch,1);
			ch=0;    snd_rawmidi_write(handle_out,&ch,1);
			snd_rawmidi_drain(handle_out); 
		}
		if (fd_out!=-1) {
			unsigned char ch;
			ch=0x90; write(fd_out,&ch,1);
			ch=60;   write(fd_out,&ch,1);
			ch=100;  write(fd_out,&ch,1);
			sleep(1);
			ch=0x90; write(fd_out,&ch,1);
			ch=60;   write(fd_out,&ch,1);
			ch=0;    write(fd_out,&ch,1);
		}
	} else {
		if ((handle_in || fd_in!=-1) && (handle_out || fd_out!=-1)) {
			if (verbose) {
				fprintf(stderr,"Testing midi thru in\n");
			}
			while (!stop) {
				unsigned char ch;
			
				if (handle_in) {
					snd_rawmidi_read(handle_in,&ch,1);
				}
				if (fd_in!=-1) {
					read(fd_in,&ch,1);
				}	
				if (verbose) {
					fprintf(stderr,"thru: %02x\n",ch);
				}

				if (handle_out) {
					snd_rawmidi_write(handle_out,&ch,1);
					snd_rawmidi_drain(handle_out); 
				}
				if (fd_out!=-1) {
					write(fd_out,&ch,1);
				}
			}
		}else{
				fprintf(stderr,"Testing midi thru needs both input and output\n");		
				exit(-1);
		}
	}

	if (verbose) {
		fprintf(stderr,"Closing\n");
	}
	
	if (handle_in) {
		snd_rawmidi_drain(handle_in); 
		snd_rawmidi_close(handle_in);	
	}
	if (handle_out) {
		snd_rawmidi_drain(handle_out); 
		snd_rawmidi_close(handle_out);	
	}
	if (fd_in!=-1) {
		close(fd_in);
	}
	if (fd_out!=-1) {
		close(fd_out);
	}

	return 0;
}
