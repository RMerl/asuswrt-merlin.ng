#include "uip.h"
#include "httpd.h"
#include "fs.h"

#include "generated/index.h" 
#include "generated/flashing.h"
#include "generated/fail.h"
#include "generated/404.h"

#define STATE_NONE		0		// empty state (waiting for request...)
#define STATE_FILE_REQUEST	1		// remote host sent GET request
#define STATE_UPLOAD_REQUEST	2		// remote host sent POST request

#define HTTPD_USE_STATIC_MEM	1		// Do not use dynamic memory allocation for interim values

// ASCII characters
#define ISO_G					0x47	// GET
#define ISO_E					0x45
#define ISO_T					0x54
#define ISO_P					0x50	// POST
#define ISO_O					0x4f
#define ISO_S					0x53
#define ISO_T					0x54
#define ISO_slash				0x2f	// control and other characters
#define ISO_space				0x20
#define ISO_nl					0x0a
#define ISO_cr					0x0d
#define ISO_tab					0x09

// we use this so that we can do without the ctype library
#define is_digit(c)				((c) >= '0' && (c) <= '9')

// debug
//#define DEBUG_UIP


// http app state
struct httpd_state *hs;

static int webfailsafe_post_done = 0;
static int webfailsafe_upload_failed = 0;
static int data_start_found = 0;

int	webfailsafe_is_running = 0;
int	webfailsafe_ready_for_upgrade = 0;

STREAM_TRANSFER_STATUS (*handle_data_stream)(char *data_stream_ptr, unsigned int len, STREAM_TRANSFER_STATE state) = NULL;
STREAM_UPGRADE_STATUS (*get_stream_upgrade_status)(void) = NULL;

static unsigned char post_packet_counter = 0;
static unsigned char packets_per_hash = 0;

// 0x0D -> CR 0x0A -> LF
static char eol[3] = { 0x0d, 0x0a, 0x00 };
static char eol2[5] = { 0x0d, 0x0a, 0x0d, 0x0a, 0x00 };

#if HTTPD_USE_STATIC_MEM
#define HTTPD_STATIC_MEM_SZ	200
static char boundary_value[HTTPD_STATIC_MEM_SZ];
#else
static char *boundary_value;
#endif

// str to int
static int atoi(const char *s){
	int i = 0;

	while(is_digit(*s)){
		i = i * 10 + *(s++) - '0';
	}

	return(i);
}

// print downloading progress
static void httpd_download_progress(void){

	if (packets_per_hash++ == 100)
	{
		putc('#');
		packets_per_hash = 0;
		post_packet_counter++;
	}

	if(post_packet_counter == 39){
		puts("\n         ");
		post_packet_counter = 0;
	}
}

// reset app state
static void httpd_state_reset(void){
	hs->state = STATE_NONE;
	hs->count = 0;
	hs->dataptr = 0;
	hs->upload = 0;
	hs->upload_total = 0;

	data_start_found = 0;
	post_packet_counter = 0;

#if HTTPD_USE_STATIC_MEM
	memset(boundary_value, 0, HTTPD_STATIC_MEM_SZ);
#else
	if(boundary_value){
		free(boundary_value);
	}
#endif	

}

// find and get first chunk of data
static int httpd_findandstore_firstchunk(void){
	char *start = NULL;
	char *end = NULL;
	
#if !HTTPD_USE_STATIC_MEM
	if(!boundary_value){
		return(0);
	}
#endif	

	// chek if we have data in packet
	start = (char *)strstr((char *)uip_appdata, (char *)boundary_value);

	if(start){
		// ok, we have data in this packet!
		// find upgrade type

		end = (char *)strstr((char *)start, "name=\"firmware\"");

		if(end){
			printf("Starting update software\n");
		} else {
			printf("input name not found!\n");
			return(0);
		}

		end = NULL;

		// find start position of the data!
		end = (char *)strstr((char *)start, eol2);

		if(end){
			if((end - (char *)uip_appdata) < uip_len){
				// move pointer over CR LF CR LF
				end += 4;

				// how much data we expect?
				// last part (magic value 6): [CR][LF](boundary length)[-][-][CR][LF]
				hs->upload_total = hs->upload_total - (int)(end - start) - strlen(boundary_value) - 6;

				printf("Upload file size: %d bytes\n", hs->upload_total);

				// We need to check if file which we are going to download
				// has correct size (for every type of upgrade)

				printf("Loading: ");

				// how much data we are storing now?
				hs->upload = (unsigned int)(uip_len - (end - (char *)uip_appdata));

				if(handle_data_stream(end, hs->upload, TRANSFER_START) == STOP_STREAM_DATA)
				{
					httpd_state_reset();
					uip_abort();
					return 0;
				}

				httpd_download_progress();

				return(1);

			}
		} else {
			printf("couldn't find start of data!\n");
		}

	}

	return(0);
}

// called for http server app
void httpd_appcall(void){
	struct fs_file fsfile;
	unsigned int i;

	switch(uip_conn->lport){
		case HTONS(80):
			// app state
			hs = (struct httpd_state *)(uip_conn->appstate);

			// closed connection
			if(uip_closed()){
				httpd_state_reset();
				uip_close();
				return;
			}

			// aborted connection or time out occured
			if(uip_aborted() || uip_timedout()){
				httpd_state_reset();
				uip_abort();
				return;
			}

			// if we are pooled
			if(uip_poll()){
				if(hs->count++ >= 100){
					httpd_state_reset();
					uip_abort();
				}
				return;
			}

			// new connection
			if(uip_connected()){
				httpd_state_reset();
				return;
			}

			// new data in STATE_NONE
			if(uip_newdata() && hs->state == STATE_NONE){
				// GET or POST request?
				if(uip_appdata[0] == ISO_G && uip_appdata[1] == ISO_E && uip_appdata[2] == ISO_T && (uip_appdata[3] == ISO_space || uip_appdata[3] == ISO_tab)){
					hs->state = STATE_FILE_REQUEST;
				} else if(uip_appdata[0] == ISO_P && uip_appdata[1] == ISO_O && uip_appdata[2] == ISO_S && uip_appdata[3] == ISO_T && (uip_appdata[4] == ISO_space || uip_appdata[4] == ISO_tab)){
					hs->state = STATE_UPLOAD_REQUEST;
				}

				// anything else -> abort the connection!
				if(hs->state == STATE_NONE){
					httpd_state_reset();
					uip_abort();
					return;
				}

				// get file or firmware upload?
				if(hs->state == STATE_FILE_REQUEST){

					// we are looking for GET file name
					for(i = 4; i < 30; i++){
						if(uip_appdata[i] == ISO_space || uip_appdata[i] == ISO_cr || uip_appdata[i] == ISO_nl || uip_appdata[i] == ISO_tab){
							uip_appdata[i] = 0;
							i = 0;
							break;
						}
					}

					if(i != 0){
						printf("request file name too long!\n");
						httpd_state_reset();
						uip_abort();
						return;
					}

					printf("Request for: ");
					printf("%s\n", &uip_appdata[4]);

					// request for /
					if((uip_appdata[4] == ISO_slash && uip_appdata[5] == 0) || !strcmp(&uip_appdata[4], "/index.html")){
						fsfile.data = (char*)index_html;
						fsfile.len = sizeof(index_html);
					} else { 
						// check if we have requested file
						fsfile.data = (char*)__404_html;
						fsfile.len = sizeof(__404_html);
					}

					hs->state = STATE_FILE_REQUEST;
					hs->dataptr = (u8_t *)fsfile.data;
					hs->upload = fsfile.len;

					// send first (and maybe the last) chunk of data
					uip_send(hs->dataptr, (hs->upload > uip_mss() ? uip_mss() : hs->upload));
					return;

				} else if(hs->state == STATE_UPLOAD_REQUEST){
					char *start = NULL;
					char *end = NULL;

					// end bufor data with NULL
					uip_appdata[uip_len] = '\0';

					/*
					 * We got first packet with POST request
					 *
					 * Some browsers don't include first chunk of data in the first
					 * POST request packet (like Google Chrome, IE and Safari)!
					 * So we must now find two values:
					 * - Content-Length
					 * - boundary
					 * Headers with these values can be in any order!
					 * If we don't find these values in first packet, connection will be aborted!
					 *
					 */

					// Content-Length pos
					start = (char *)strstr((char*)uip_appdata, "Content-Length:");

					if(start){
						start += sizeof("Content-Length:");

						// find end of the line with "Content-Length:"
						end = (char *)strstr(start, eol);

						if(end){

							hs->upload_total = atoi(start);
#ifdef DEBUG_UIP
							printf("Expecting %d bytes in body request message\n", hs->upload_total);
#endif

						} else {
							printf("couldn't find 'Content-Length'!\n");
							httpd_state_reset();
							uip_abort();
							return;
						}

					} else {
						printf("couldn't find 'Content-Length'!\n");
						httpd_state_reset();
						uip_abort();
						return;
					}

					// we don't support very small files (< 10 KB)
					if(hs->upload_total < 10240){
						printf("request for upload < 10 KB data!\n");
						httpd_state_reset();
						uip_abort();
						return;
					}

					// boundary value
					start = NULL;
					end = NULL;

					start = (char *)strstr((char *)uip_appdata, "boundary=");

					if(start){
						// move pointer over "boundary="
						start += 9;

						// find end of line with boundary value
						end = (char *)strstr((char *)start, eol);

						if(end){

#if !HTTPD_USE_STATIC_MEM
							// malloc space for boundary value + '--' and '\0'
							boundary_value = (char*)malloc(end - start + 3);
#endif							
							if(boundary_value){

								memcpy(&boundary_value[2], start, end - start);

								// add -- at the begin and 0 at the end
								boundary_value[0] = '-';
								boundary_value[1] = '-';
								boundary_value[end - start + 2] = 0;

#ifdef DEBUG_UIP
								printf("Found boundary value: \"%s\"\n", boundary_value);
#endif

							} else {
								printf("couldn't allocate memory for boundary!\n");
								httpd_state_reset();
								uip_abort();
								return;
							}

						} else {
							printf("couldn't find boundary!\n");
							httpd_state_reset();
							uip_abort();
							return;
						}
					} else {
						printf("couldn't find boundary!\n");
						httpd_state_reset();
						uip_abort();
						return;
					}

					/*
					 * OK, if we are here, it means that we found
					 * Content-Length and boundary values in headers
					 *
					 * We can now try to 'allocate memory' and
					 * find beginning of the data in first packet
					 */

					if(httpd_findandstore_firstchunk()){
						data_start_found = 1;
					} else {
						data_start_found = 0;
					}

					return;

				} /* else if(hs->state == STATE_UPLOAD_REQUEST) */

			} /* uip_newdata() && hs->state == STATE_NONE */

			// if we got ACK from remote host
			if(uip_acked()){
				// if we are in STATE_FILE_REQUEST state
				if(hs->state == STATE_FILE_REQUEST){
					// data which we send last time was received (we got ACK)
					// if we send everything last time -> gently close the connection
					if(hs->upload <= uip_mss()){
						// post upload completed?
						if(webfailsafe_post_done){
							if(!webfailsafe_upload_failed){
								webfailsafe_ready_for_upgrade = 1;
							}

							webfailsafe_post_done = 0;
							webfailsafe_upload_failed = 0;
						}

						httpd_state_reset();
						uip_close();
						return;
					}

					// otherwise, send another chunk of data
					// last time we sent uip_conn->len size of data
					hs->dataptr += uip_conn->len;
					hs->upload -= uip_conn->len;

					uip_send(hs->dataptr, (hs->upload > uip_mss() ? uip_mss() : hs->upload));
				}

				return;

			}

			// if we need to retransmit
			if(uip_rexmit()){
				// if we are in STATE_FILE_REQUEST state
				if(hs->state == STATE_FILE_REQUEST){
					// send again chunk of data without changing pointer and length of data left to send
					uip_send(hs->dataptr, (hs->upload > uip_mss() ? uip_mss() : hs->upload));
				}

				return;

			}

			// if we got new data frome remote host
			if(uip_newdata()){
				// if we are in STATE_UPLOAD_REQUEST state
				if(hs->state == STATE_UPLOAD_REQUEST){
					// end bufor data with NULL
					uip_appdata[uip_len] = '\0';

					// do we have to find start of data?
					if(!data_start_found){

						if(!httpd_findandstore_firstchunk()){
							printf("couldn't find start of data in next packet!\n");
							httpd_state_reset();
							uip_abort();
							return;
						} else {
							data_start_found = 1;
						}
						return;
					}

					hs->upload += (unsigned int)uip_len;

					if(!webfailsafe_upload_failed){
						if(handle_data_stream((char*)uip_appdata, uip_len, (hs->upload >= hs->upload_total) ? TRANSFER_END : TRANSFER_CONTINUE) == STOP_STREAM_DATA)
						{
							httpd_state_reset();
							uip_abort();
							return;
						}
					}

					httpd_download_progress();

					// if we have collected all data
					if(hs->upload >= hs->upload_total){
						// end of post upload
						webfailsafe_post_done = 1;

						// Check if upgrade failed
						if( get_stream_upgrade_status() == STREAM_UPGRADE_ERR )
							webfailsafe_upload_failed = 1;
						else
							webfailsafe_upload_failed = 0;

						// which website will be returned
						if(!webfailsafe_upload_failed){
							fsfile.data = (char*)flashing_html;
							fsfile.len = sizeof(flashing_html);
						} else {
							fsfile.data = (char*)fail_html;
							fsfile.len = sizeof(fail_html);
						}
						httpd_state_reset();
						hs->state = STATE_FILE_REQUEST;
						hs->dataptr = (u8_t *)fsfile.data;
						hs->upload = fsfile.len;

						uip_send(hs->dataptr, (hs->upload > uip_mss() ? uip_mss() : hs->upload));
					}
				}
				return;
			}
			break;

		default:
			// we shouldn't get here... we are listening only on port 80
			uip_abort();
			break;
	}
}


