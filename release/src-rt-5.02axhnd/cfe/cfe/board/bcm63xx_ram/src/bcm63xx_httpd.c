/* milli_httpd - pretty small HTTP server
** A combination of
** micro_httpd - really small HTTP server
** and
** mini_httpd - small HTTP server
**
** Copyright 1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#if (CFG_WEB_SERVER==1)

/** Includes. **/

#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"

#include "cfe_timer.h"
#include "cfe_error.h"
#include "cfe.h"
#include "cfe_iocb.h"
#include "cfe_devfuncs.h"

#include "bsp_config.h"

#include "bcmTag.h"
#include "bcm63xx_util.h"
#include "bcm63xx_httpd_web_var.h"


extern unsigned short g_raw_flash_write;

#if INC_EMMC_FLASH_DRIVER
#include "dev_emmcflash.h"
#endif

#ifdef AC2900
#include "bcm63xx_nvram.h"
#endif
/** Externs. **/

extern char ul_html[];
extern char ulinfo_html[];
extern int ul_html_size;
extern int ulinfo_html_size;

extern int g_console_abort;
extern int g_processing_cmd;

/** Defines. **/

#define SERVER_NAME                 "micro_httpd"
#define SERVER_URL                  "http://www.acme.com/software/micro_httpd/"
#define SERVER_PORT                 80
#define PROTOCOL                    "HTTP/1.0"
#define POST_DATA_START             (unsigned char*) cfe_get_mempool_ptr() 
#define SOCKET_CLOSED               -100
#define NUM_SOCKETS                 4

/* Return codes from cfe_web_gets. */
#define WEB_GETS_DONE               1
#define WEB_GETS_PENDING            2
#define WEB_GETS_ERROR              3

/* HTTP states. */
#define HTTP_INITIAL                0
#define HTTP_READ_FIRST_HDR         1
#define HTTP_READ_REMAINING_HDRS    2
#define HTTP_READ_POST_DATA         3

/* HTTP POST status */
#define UPLOAD_OK                   '0'
#define UPLOAD_FAIL_NO_MEM          '1'
#define UPLOAD_FAIL_NO_FILENAME     '2'
#define UPLOAD_FAIL_ILLEGAL_IMAGE   '3'
#define UPLOAD_FAIL_IMAGE_TOOBIG    '4'
#define UPLOAD_FAIL_CORRUPT_IMAGE   '5'
#define UPLOAD_FAIL_FLASH           '6'
#define UPLOAD_FATAL                '7'
#define UPLOAD_PENDING              '8'
#define UPLOAD_TCP_ERROR            '9'

/* HTTP upload image formats. */
#define NO_IMAGE_FORMAT             0
#define BROADCOM_IMAGE_FORMAT       1
#define FLASH_IMAGE_FORMAT          2
#define FLASH_IMAGE_RAW_FORMAT      3

#define FLASH_IMAGE_FORMAT_DATA1    3 // FLASH_IMAGE_FORMAT_DATA1 = MISC_PARTITION_1
#define FLASH_IMAGE_FORMAT_DATA2    4 // FLASH_IMAGE_FORMAT_DATA1 + MISC_PARTITION_2 
#define FLASH_IMAGE_FORMAT_DATA3    5 // FLASH_IMAGE_FORMAT_DATA1 + MISC_PARTITION_3 

#define MISC_PARTITION_1            0
#define MISC_PARTITION_2            1
#define MISC_PARTITION_3            2

/** Structs. **/

typedef struct
{
    int s;
    int state;
    int web_buf_idx;
    int post_content_length;
    char web_first_buf[128];
    char web_buf[256];
} SOCKET_INFO, *PSOCKET_INFO;

typedef struct
{
    char *wp_name;
    char *wp_content_buf;
    int *wp_content_size;
    char *wp_mime_type;
} WEB_PAGE_MAP, *PWEB_PAGE_MAP;


/** Globals. **/

static int g_listen_idx = 0;
static unsigned char *g_image_start = NULL;
static int g_image_len = 0;
static int g_image_format = NO_IMAGE_FORMAT;
static int g_post_data_in_progress = 0;
static int g_post_data_idx = 0;

static SOCKET_INFO g_socket_info[NUM_SOCKETS];

static WEB_PAGE_MAP g_web_page_map[] =
    {
     {"/", ul_html, &ul_html_size, "text/html"},
     {"/upload.html", ul_html, &ul_html_size, "text/html"},
     {"/uploadinfo.html", ulinfo_html, &ulinfo_html_size, "text/html"},
     {NULL, NULL, 0, NULL}
    };


/** Prototypes. **/

int cfe_web_check(void);
void cfe_web_fg_process(void);
void cfe_web_poll(void *x);
static void cfe_web_listen( int *listen_idx_ptr );
static void cfe_web_bg_process(PSOCKET_INFO si);
static int cfe_web_gets( char *web_buf, int web_buf_size,
    int *web_buf_idx_ptr, int s );
static int read_first_hdr(int s, char *web_buf, int web_buf_size,
    int *web_buf_idx_ptr, int *close_tcp_ptr);
static int read_remaining_hdrs(int s, char *web_buf, int web_buf_size,
    int *web_buf_idx_ptr, int *close_tcp_ptr, int *content_length_ptr);
static char read_post_data( int s, unsigned char *post_data_start,
    int content_length, int *post_data_idx_ptr );
static char parse_post_data( int s, unsigned char *post_data_start,
    int post_data_length, unsigned char **image_start_ptr, int *image_len_ptr,
    int *image_format_ptr  );
static void send_error( int s, int status, char* title, char* extra_header,
    char* text );
static void send_error( int s, int status, char* title, char* extra_header,
    char* text );
static void send_headers( int s, int status, char* title, char* extra_header,
    char* mime_type );
static void send_page( int s, char *path, int send_headers_flag,
    char **substs, int num_substs );
static int cfe_web_tcp_send( int s, char *buf, int size );


/***************************************************************************
 * Function Name: cfe_web_check
 * Description  : Checks if an image has been downloaded through an HTTP POST
 *                request and is ready to be written to flash memory.
 * Returns      : 1 - image is ready to be flashed, 0 - nothing to do
 ***************************************************************************/
int cfe_web_check(void)
{
    return( (g_image_format != NO_IMAGE_FORMAT) ? 1 : 0 );
} /* cfe_web_check */


/***************************************************************************
 * Function Name: cfe_web_process
 * Description  : Calls the appropriate functions to write an image to
 *                flash memory.
 * Returns      : None.
 ***************************************************************************/
void cfe_web_fg_process(void)
{
    int res = -1;
    /* Wait so the uploadinfo web page can be displayed on the browser. */
    cfe_sleep(CFE_HZ * 2);
    if( g_image_format == BROADCOM_IMAGE_FORMAT )
        res = flashImage( g_image_start );
    else
#if INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1
        if(g_raw_flash_write == 1 && g_image_format == FLASH_IMAGE_RAW_FORMAT)
        {
            //attempt raw write 
            res=writeWholeImageRaw(g_image_start, g_image_len, 0);

        }
        else
#endif
        if( g_image_format == FLASH_IMAGE_FORMAT )
            res = writeWholeImage( g_image_start, g_image_len );
#if INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1
        else if( g_image_format == FLASH_IMAGE_FORMAT_DATA1)
            res = writeWholeImageDataPartition( g_image_start, g_image_len, "1" );
        else if( g_image_format == FLASH_IMAGE_FORMAT_DATA2)
            res = writeWholeImageDataPartition( g_image_start, g_image_len, "2" );
        else if( g_image_format == FLASH_IMAGE_FORMAT_DATA3)
            res = writeWholeImageDataPartition( g_image_start, g_image_len, "3" );
#endif

    if( g_image_format != NO_IMAGE_FORMAT && res == 0 )
        softReset(0);
    else
    {
        g_image_format = NO_IMAGE_FORMAT;
        g_processing_cmd = 0;
    }

} /* cfe_web_process */


/***************************************************************************
 * Function Name: cfe_web_poll
 * Description  : The entry point function that is called in the background
 *                at polled intervals.  It listens for and processes HTTP
 *                requests.
 * Returns      : None.
 ***************************************************************************/
void cfe_web_poll(void *x)
{
    static int first_time = 1;
    static int in_cfe_web_poll = 0;

    PSOCKET_INFO si;
    int i;

    if( in_cfe_web_poll == 0 )
    {
        in_cfe_web_poll = 1;

        /* If this is the first time that this function was called, initialize
         * the socket info data array.
         */
        if( first_time == 1 )
        {
            first_time = 0;
            for( i = 0, si = g_socket_info; i < NUM_SOCKETS; i++, si++ )
            {
                si->s = SOCKET_CLOSED;
                si->state = HTTP_READ_FIRST_HDR;
                si->web_buf_idx = 0;
                si->post_content_length = 0;
            }
        }

        /* Check the connection state of each socket. */
        for( i = 0, si = g_socket_info; i < NUM_SOCKETS; i++, si++ )
        {
            cfe_web_listen( &g_listen_idx );
            if( si->s >= 0 )
            {
                unsigned int connflag;
                tcp_status( si->s, &connflag, NULL, NULL );
                if( connflag == TCPSTATUS_CONNECTED )
                {
                    cfe_web_bg_process( si );
                    POLL();
                }
                else
                    if( connflag == TCPSTATUS_NOTCONN )
                    {
                        console_log("web warning: Unexpected TCP disconnect.");
                        tcp_close(si->s);
                        si->s = SOCKET_CLOSED;
                        si->state = HTTP_READ_FIRST_HDR;
                        si->web_buf_idx = 0;
                    }
            }
        }

        in_cfe_web_poll = 0;
    }
} /* cfe_web_poll */


/***************************************************************************
 * Function Name: cfe_web_listen
 * Description  : This function checks to see if TCP listen can be issued
 *                on the HTTP port and issues the listen if it can.
 * Returns      : None.
 ***************************************************************************/
static void cfe_web_listen( int *listen_idx_ptr )
{
    static int port = SERVER_PORT;

    int listen_idx = *listen_idx_ptr;
    PSOCKET_INFO si = &g_socket_info[listen_idx];

    /* If a TCP socket has been opened, check its connection status. */
    if( si->s >= 0 )
    {
        unsigned int connflag;
        tcp_status( si->s, &connflag, NULL, NULL );

        /* If the socket is connection, set the next socket index to listen for
         * a TCP connection.
         */
        if( connflag == TCPSTATUS_CONNECTED )
        {
            listen_idx = (listen_idx + 1) % NUM_SOCKETS;
            si = &g_socket_info[listen_idx];
        }
    }

    /* If the TCP socket has not been opened, open it and listen for a TCP
     * connection.
     */
    if( si->s == SOCKET_CLOSED )
    {
        /* Open the socket in non-blocking mode. */
        POLL();
        if( (si->s = tcp_socket()) >= 0 )
        {
            console_log("web info: Waiting for connection on socket %d.", si->s);
            if( tcp_listen(si->s, port) != 0 )
                console_log("web error: listen error on %d.", si->s);
        }
        else
        {
            console_log("web error %d: Could not create TCP socket.", si->s);
            si->s = SOCKET_CLOSED;
        }
    }

    *listen_idx_ptr = listen_idx;
} /* cfe_web_listen */


/***************************************************************************
 * Function Name: cfe_web_bg_process
 * Description  : This function processes an HTTP request on a socket.
 * Returns      : None.
 ***************************************************************************/
static void cfe_web_bg_process(PSOCKET_INFO si)
{
    char post_subst[] = {UPLOAD_FATAL, '\0'};
    char *post_substs[] = {post_subst};
    int close_tcp = 0;

    switch( si->state )
    {
    case HTTP_READ_FIRST_HDR:
        if( read_first_hdr( si->s, si->web_first_buf,
            sizeof(si->web_first_buf), &si->web_buf_idx, &close_tcp ) == 0 )
        {
            /* Not all of the first header has been read yet. Try again later.*/
            break;
        }

        /* The first header has been read. */
        si->state = HTTP_READ_REMAINING_HDRS;

        /* fall thru */

    case HTTP_READ_REMAINING_HDRS:
        if( read_remaining_hdrs( si->s, si->web_buf, sizeof(si->web_buf),
            &si->web_buf_idx, &close_tcp, &si->post_content_length ) )
        {
            if( g_processing_cmd == 0 )
            {
                char *method = NULL;
                char *path = NULL;
                char *ptr = (char *) si->web_first_buf;

                method = gettoken(&ptr);
                if( method )
                    path = gettoken(&ptr);

                /* Process the HTTP request. Only GET and POST are supported. */
                if( method && path )
                {
                    if( !strcmpi( method, "get" ) )
                    {
                        send_page( si->s, path, 1, NULL, 0 );
                        close_tcp = 1;
                    }
                    else
                    {
                        if( !strcmpi( method, "post" ) )
                        {
                            if( g_post_data_in_progress == 0 )
                            {
                                g_post_data_in_progress = 1;
                                si->state = HTTP_READ_POST_DATA;
                            }
                            else
                            {
                                send_error( si->s, 501, "Upload Busy",
                                    (char*) 0,
                                    "An image is already being uploaded." );
                                close_tcp = 1;
                            }
                        }
                        else
                        {
                            send_error( si->s, 501, "Not Implemented",
                                (char*) 0,
                                "That method is not implemented." );
                            close_tcp  = 1;
                        }
                    }
                }
                else
                {
                    send_error( si->s, 400, "Bad Request", (char *) 0,
                        "Can't parse request." );
                    close_tcp  = 1;
                }
            }
            else
            {
                /* A download and flash image command is being executed from
                 * the serial port console.
                 */
                send_error( si->s, 400, "Bad Request", (char *) 0,
                    "Console command is in progress." );
                close_tcp  = 1;
            }
        }

        if( si->state != HTTP_READ_POST_DATA )
            break;

    case HTTP_READ_POST_DATA:
        /* Read the post data, which contains an image to flash, into low
         * memory.
         */
        if( (post_subst[0] = read_post_data( si->s, POST_DATA_START,
            si->post_content_length, &g_post_data_idx )) == UPLOAD_OK )
        {
            /* Verify that the post data is a valid image to flash. */
            post_subst[0] = parse_post_data( si->s, POST_DATA_START,
                g_post_data_idx, (unsigned char **) &g_image_start, &g_image_len,
                &g_image_format );
        }

        switch( post_subst[0] )
        {
        case UPLOAD_PENDING:
            break;

        case UPLOAD_TCP_ERROR:
            close_tcp = 1;
            g_post_data_in_progress = 0;
            g_post_data_idx = 0;
            break;

        case UPLOAD_OK:
            /* Notify foreground to abort the console input so it can
             * write the image to flash memory.
             */
            g_console_abort = 1;

            send_page(si->s, "/uploadinfo.html", 0, post_substs, 1);
            close_tcp = 1;
            g_post_data_idx = 0;
            break;

        default:
            /* The image was downloaded OK but there was a problem with it
             * so it could not be written to flash memory.
             */
            send_page(si->s, "/uploadinfo.html", 0, post_substs, 1);
            close_tcp = 1;
            g_post_data_in_progress = 0;
            g_post_data_idx = 0;
            break;
        }
        break;
    }

    /* Close the socket if the HTTP transaction is done. */
    if( close_tcp )
    {
        POLL();
        tcp_close(si->s);
        si->s = SOCKET_CLOSED;
        si->state = HTTP_READ_FIRST_HDR;
        si->web_buf_idx = 0;
        si->post_content_length = 0;
    }
} /* cfe_web_poll */


/***************************************************************************
 * Function Name: cfe_web_gets
 * Description  : Reads from a socket up to a <CR><LF> or <LF>.  The socket
 *                is non-blocking.
 * Returns      : WEB_GETS_DONE    - Complete line was read.
 *                WEB_GETS_PENDING - Line partially read.
 *                WEB_GETS_ERROR   - Socket error.
 ***************************************************************************/
static int cfe_web_gets( char *web_buf, int web_buf_size,
    int *web_buf_idx_ptr, int s )
{
    int ret = WEB_GETS_PENDING;
    unsigned char ch;
    int web_buf_idx = *web_buf_idx_ptr;
    char *p = web_buf + web_buf_idx;
    int continue_reading = 1;

    while( web_buf_idx < web_buf_size && continue_reading )
    {
        switch( tcp_recv( s, &ch, 1 ) )
        {
        case 0: /* no characters are available to receive */
            continue_reading = 0;
            break;

        case 1: /* character was read */
            if( ch == '\n' )
            {
                *p = '\0';
                continue_reading = 0;
                ret = WEB_GETS_DONE;
            }
            else
                if( ch != '\r' )
                {
                    *p++ = ch;
                    web_buf_idx++;
                }
            break;

        default:
            continue_reading = 0;
            ret = WEB_GETS_ERROR;
            break;
        }
    }

    if( web_buf_idx == web_buf_size )
    {
        web_buf[web_buf_idx - 1] = '\0';
        ret = WEB_GETS_DONE;
    }

    *web_buf_idx_ptr = web_buf_idx;

    return( ret );
} /* cfe_web_gets */


/***************************************************************************
 * Function Name: read_first_hdr
 * Description  : This function reads the first HTTP header which contains
 *                the method (GET, POST), path and protocol.  For example,
 *                GET /upload.html HTTP/1.1
 * Returns      : 1 - First header was read, 0 - was not read.
 ***************************************************************************/
static int read_first_hdr(int s, char *web_buf, int web_buf_size,
    int *web_buf_idx_ptr, int *close_tcp_ptr)
{
    int ret = 0;
    int sts = cfe_web_gets( web_buf, web_buf_size, web_buf_idx_ptr, s );

    switch( sts )
    {
    case WEB_GETS_DONE:
        /* The first HTTP header has been read into web_buf. */
        *web_buf_idx_ptr = 0;
        ret = 1;
        break;

    case WEB_GETS_ERROR:
        console_log("web error: TCP read error.");
        *close_tcp_ptr = 1;
        break;
    }

    return( ret );
} /* read_first_hdr */


/***************************************************************************
 * Function Name: read_remaining_hdrs
 * Description  : This function reads the remaining HTTP headers.
 * Returns      : 1 - Remaining headers were read, 0 - were not read.
 ***************************************************************************/
static int read_remaining_hdrs(int s, char *web_buf, int web_buf_size,
    int *web_buf_idx_ptr, int *close_tcp_ptr, int *content_length_ptr)
{
    int ret = 0;
    int sts = WEB_GETS_DONE;

    while( sts == WEB_GETS_DONE )
    {
        sts = cfe_web_gets( web_buf, web_buf_size, web_buf_idx_ptr, s );
        switch( sts )
        {
        case WEB_GETS_DONE:
            if( *web_buf_idx_ptr == 0 )
            {
                /* The remaining HTTP headers have been read. */
                ret = 1;
                sts = WEB_GETS_PENDING;
            }
            else
            {
                char *p2 = web_buf;
                char *p1 = gettoken(&p2);
                if( !strcmpi( p1, "Content-Length:" ) )
                    *content_length_ptr=atoi(p2);
                *web_buf_idx_ptr = 0;
            }
            break;

        case WEB_GETS_ERROR:
            console_log("web error: TCP read error.");
            *close_tcp_ptr = 1;
            break;
        }
    }

    return( ret );
} /* read_remaining_hdrs */


/***************************************************************************
 * Function Name: read_post_data
 * Description  : This function reads HTTP POST data which is the contents of
 *                a new image to write to flash memory.
 * Returns      : UPLOAD_OK - all data read
 *                UPLOAD_PENDING - not all data read
 *                UPLOAD_TCP_ERROR - TCP error
 ***************************************************************************/
static char read_post_data( int s, unsigned char *post_data_start,
    int content_length, int *post_data_idx_ptr )
{
    char ret = UPLOAD_PENDING;
    int post_data_idx = *post_data_idx_ptr;
    int len;

    do
    {
        len = tcp_recv( s, (unsigned char*)(post_data_start + post_data_idx),
            content_length - post_data_idx );
        post_data_idx += len;
        POLL();
        cfe_web_listen( &g_listen_idx );
    } while( len > 0 && post_data_idx < content_length );

    *post_data_idx_ptr = post_data_idx;

    if( len < 0 )
    {
        console_log("web error: TCP read error receiving post data.");
        ret = UPLOAD_TCP_ERROR;
    }
    else
        if( post_data_idx == content_length )
            ret = UPLOAD_OK;

    return( ret );
} /* read_post_data */


/***************************************************************************
 * Function Name: parse_post_data
 * Description  : This function parses HTTP POST data which is the contents of
 *                a new image to write to flash memory.
 * Returns      : UPLOAD_OK or UPLOAD_xxx error
 ***************************************************************************/
static char parse_post_data( int s, unsigned char *post_data_start,
    int post_data_length, unsigned char **image_start_ptr, int *image_len_ptr,
    int *image_format_ptr  )
{
    char ret = UPLOAD_OK;
    unsigned char *p = post_data_start;
    int boundary_size = 0;
    int misc_partition_to_write=-1;

#ifdef AC2900
    if (NVRAM.noUpdatingFirmware) {
        printf("no Updating!\n");
        return 99;
    }
#endif

    /* Convert the start boundary field into a string.  It will be compared
     * against the end boundary field below.
     */
    while( *p != '\r' && *p != '\n' &&
        (int)(p - post_data_start) < post_data_length )
    {
        p++;
    }

    if( *p == '\r' || *p == '\n' )
    {
        *p++ = '\0';
        boundary_size = strlen((char*)post_data_start);
    }
    else
    {
        console_log("web error: HTTP POST start bound field not found.");
        ret = UPLOAD_FATAL;
    }

    /* Verify that a filename has been entered. */
    if( ret == UPLOAD_OK )
    {
        unsigned char *fname = NULL;
        while( memcmp( p, "\r\n\r\n", strlen("\r\n\r\n") ) )
        {
#if INC_EMMC_FLASH_DRIVER
            if( *p == 'n' && !memcmp( p, "name=\"imgupdrepartition\"", strlen("name=\"imgupdrepartition\"" ) ) )
            {
                p += strlen("name=\"imgupdrepartition\"");
                fname = p + 1;
                p++;
                while( *p != '\r' && *(p+1) != '\n' )
                    p++;
                p+=2; //skip \r\n
                if(*p == '1')
                {
                    emmc_allow_img_update_repartitions(*p);
                }
            }
#endif /* INC_EMMC_FLASH_DRIVER */
            if( *p == 'n' && !memcmp( p, "name=\"mpn\"", strlen("name=\"mpn\"" ) ) )
            {
                p += strlen("name=\"mpn\"");
                fname = p + 1;
                p++;
                while( *p != '\r' && *(p+1) != '\n' )
                    p++;
                p+=2; //skip \r\n
                if(*p-'1' >=0 && *p-'1' <= 3)
                    misc_partition_to_write = *p-'1';
            }
            if( *p == 'f' && !memcmp( p, "filename=", strlen("filename=" ) ) )
            {
                p += strlen("filename=");
                fname = p + 1;
                if( p[0] == '"' && p[1] != '"' )
                {
                    p++;
                    while( *p != '"' && *p != '\r' && *p != '\n' )
                        p++;
                    *p = '\0';
                }
                else
                {
                    console_log("web error: HTTP POST filename not specified.");
                    ret = UPLOAD_FAIL_NO_FILENAME;
                }
                break;
            }

            p++;
        }

        if( fname == NULL )
        {
            console_log("web error: HTTP POST filename field not found.");
            ret = UPLOAD_FATAL;
        }
    }

    /* Find the start of the image which starts after two consecutive
     * carriage return, linefeed pairs.
     */
    if( ret == UPLOAD_OK )
    {
        while( memcmp( p, "\r\n\r\n", strlen("\r\n\r\n") ) )
            p++;

        p += strlen("\r\n\r\n");
        if( p[0] != '\r' || p[1] != '\n' ||
            memcmp(p + 2, post_data_start, boundary_size ) )
        {
            *image_start_ptr = p;
        }
        else
        {
            console_log("web error: HTTP POST no image data.");
            ret = UPLOAD_FAIL_ILLEGAL_IMAGE;
        }
    }

    /* Find the end of the image which contains the same boundary field as
     * at the start of the buffer.
     */
    if( ret == UPLOAD_OK )
    {
        p = post_data_start + post_data_length - 1;
        while( *p == '\r' || *p == '\n' || *p == '-' )
            p--;
        p[1] = '\0';
        p -= boundary_size + 1;
        if( !memcmp( p + strlen("\r\n"), post_data_start, boundary_size ) )
	    *image_len_ptr = (int) (p - *image_start_ptr);
        else
        {
            console_log("web error: HTTP POST end bound field not found.");
            ret = UPLOAD_FATAL;
        }
    }

    /* Verify that the image is (or should be) a Broadcom flash format file or
     * a flash image format.
     */
    if( ret == UPLOAD_OK )
    {
        /* Align the image on a 16 byte boundary */
        if( ((uintptr_t) *image_start_ptr & 0x0f) != 0 )
        {
            unsigned char *dest = (unsigned char *)
                ((uintptr_t) *image_start_ptr & ~0x0f);
            unsigned char *src = *image_start_ptr;
            memmove( dest, src, *image_len_ptr );
            *image_start_ptr = dest;
        }

        if(misc_partition_to_write == -1 )
        {

             if(g_raw_flash_write == 1)
             {
                 console_log("web info: Upload %lu bytes, Raw image format",
                 *image_len_ptr);
                 *image_format_ptr = FLASH_IMAGE_RAW_FORMAT;
             }
            /* Check if the first part of the image is the Broadcom defined TAG
             * record.
             */
            else if( verifyTag( (FILE_TAG *) *image_start_ptr, 0 ) == -1 )
            {
                /* It is not a Broadcom flash format file.  Now check if it is a
                 * flash image format file.  A flash image format file must have a
                 * CRC at the end of the image.
                 */
                unsigned char *image_ptr = *image_start_ptr;
                unsigned int image_len = *image_len_ptr - TOKEN_LEN;
                unsigned int crc = CRC32_INIT_VALUE;

                crc = getCrc32(image_ptr, image_len, crc);      
                if (memcmp(&crc, image_ptr + image_len, CRC_LEN) == 0)
                {
                    console_log("web info: Upload %lu bytes, flash image format.",
                        *image_len_ptr);
                    *image_format_ptr = FLASH_IMAGE_FORMAT;
                }
                else
                {
                    console_log("web info: Upload %lu bytes, invalid image format.",
                        *image_len_ptr);
                    ret = UPLOAD_FAIL_ILLEGAL_IMAGE;
                }
            }
            else
            {
                console_log("web info: Upload %lu bytes, Broadcom image format.",
                        *image_len_ptr);
                *image_format_ptr = BROADCOM_IMAGE_FORMAT;
            }
        }
    }

    //if user selected a data partition, write to it instead of regular image_update partition
    switch(misc_partition_to_write)
    {
        case MISC_PARTITION_1: 
        case MISC_PARTITION_2: 
        case MISC_PARTITION_3: 
            *image_format_ptr  = FLASH_IMAGE_FORMAT_DATA1 + misc_partition_to_write;
            break;
    }

    return( ret );
} /* parse_post_data */


/***************************************************************************
 * Function Name: send_error
 * Description  : This function sends an HTTP error response to the browser.
 * Returns      : None.
 ***************************************************************************/
static void send_error( int s, int status, char* title, char* extra_header,
    char* text )
{
    int tcpret = 0;
    char buf[128];
    send_headers( s, status, title, extra_header, "text/html" );
    sprintf( (char *) buf, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n"
        "<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status,
        title );
    tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );
    sprintf( (char *) buf, "%s\n", text );
    tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );
    sprintf( (char *) buf, "<HR>\n<ADDRESS><A HREF=\"%s\">%s</A></ADDRESS>\n"
        "</BODY></HTML>\n", SERVER_URL, SERVER_NAME );
    tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );

    if( tcpret < 0 )
        console_log("web error: TCP write error sending error response.");
} /* send_error */


/***************************************************************************
 * Function Name: send_headers
 * Description  : This function sends an HTTP response to the browser.
 * Returns      : None.
 ***************************************************************************/
static void send_headers( int s, int status, char* title, char* extra_header,
    char* mime_type )
{
    int tcpret = 0;
    char buf[128];
    unsigned long secs = (unsigned long) cfe_ticks / CFE_HZ;

    sprintf( buf, "%s %d %s\r\n", PROTOCOL, status, title );
    tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );
    sprintf( buf, "Server: %s\r\n", SERVER_NAME );
    tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );
    sprintf( buf, "Date: Thu, 01 Jan 1970 %2.2d:%2.2d:%2.2d GMT\r\n",
        secs / 3600, (secs % 3600) / 60, secs % 60 );
    tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );
    if ( extra_header != (char*) 0 )
    {
        sprintf( buf, "%s\r\n", extra_header );
        tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );
    }
    if ( mime_type != (char*) 0 )
    {
        sprintf( buf, "Content-Type: %s\r\n", mime_type );
        tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );
    }
    sprintf( buf, "Connection: close\r\n\r\n" );
    tcpret = tcp_send( s, (unsigned char*)buf, strlen(buf) );

    if( tcpret < 0 )
        console_log("web error: TCP write error sending header.");
} /* send_headers */


/***************************************************************************
 * Function Name: send_page
 * Description  : This function sends a web page to the browser.
 * Returns      : None.
 ***************************************************************************/
static void send_page( int s, char *path, int send_headers_flag,
    char **substs, int num_substs )
{
    PWEB_PAGE_MAP map;
    int num_web_vars = cfe_web_get_num_web_vars();

    /* Find the specified web page. */
    for( map = g_web_page_map; map->wp_name; map++ )
    {
        if( !strcmp( map->wp_name, path ) )
        {
            /* Found the web page. */
            char *p2 = NULL;
            char *p = (char *) map->wp_content_buf;
            int size = *map->wp_content_size;
            int i = 0;
            int j = 0;
            int subst_len;
            char * subst_ptr;

            if( send_headers_flag )
                send_headers( s, 200, "Ok", (char *) 0, map->wp_mime_type );

            /* Make substitutions. */
            while( (i < num_substs || j < num_web_vars) && (p2 = strnchr( p, '<', size )) != NULL )
            {
                if( p2[1] == '%' )
                {
                    /* Init subst variables */
                    subst_len = 0;
                    subst_ptr = NULL;

                    /* Found a substituion pattern. Send up to that point. */
                    if( cfe_web_tcp_send( s, p, (int) (p2 - p) ) < 0 )
                        break;

                    /* First try and match web_var */
                    if( j < num_web_vars )
                    {
                        subst_len = size;
                        subst_ptr = cfe_web_get_web_var(p2, &subst_len);
                    }

                    /* if no web_var match then try the simple substs strings */
                    if( subst_ptr )
                        j++;
                    else
                    {
                        if( i < num_substs )
                        {
                            subst_ptr = substs[i];
                            subst_len = strlen(substs[i]);
                            i++;
                        }
                    }

                    /* Send substitution value. */
                    if( subst_ptr && subst_len )
                    {
                        if( cfe_web_tcp_send( s, subst_ptr, subst_len) < 0 )
                            break;
                    }

                    /* Skip to end of substitution pattern. */
                    p = p2 + 2; /* skip '<%' */
                    while( p[0] != '%' || p[1] != '>' )
                        p++;
                    p += 2; /* skip '%>' */
                }
                else
                {
                    /* Was not a substitution pattern.  Send up that point. */
                    p2++;
                    if( cfe_web_tcp_send( s, p, (int) (p2 - p) ) < 0 )
                        break;

                    p = p2;
                }

                size = *map->wp_content_size - (int)(p-map->wp_content_buf);
            }
            //while( i < num_substs && (p2 = strnchr( p, '<', size )) != NULL )
            //{
            //    if( p2[1] == '%' )
            //    {
            //        /* Found a substituion pattern. Send up to that point. */
            //        if( cfe_web_tcp_send( s, p, (int) (p2 - p) ) < 0 )
            //            break;

            //        /* Send substitution value. */
            //        if( cfe_web_tcp_send( s, substs[i], strlen(substs[i]) ) < 0 )
            //            break;

            //        i++;

            //        /* Skip to end of substitution pattern. */
            //        p = p2 + 2; /* skip '<%' */
            //        while( p[0] != '%' || p[1] != '>' )
            //            p++;
            //        p += 2; /* skip '%.' */
            //    }
            //    else
            //    {
            //        /* Was not a substitution pattern.  Send up that point. */
            //        p2++;
            //        if( cfe_web_tcp_send( s, p, (int) (p2 - p) ) < 0 )
            //            break;

            //        p = p2;
            //    }

            //    size = *map->wp_content_size - (int)(p-map->wp_content_buf);
            //}

            /* Send remaining part of web page after the last substitution. */
            cfe_web_tcp_send( s, p, size );

            break; /* for loop */
        }
    }

    if( map->wp_name == NULL )
        send_error( s, 404, "Not Found", (char*) 0, "File not found." );
} /* send_page */


/***************************************************************************
 * Function Name: cfe_web_tcp_send
 * Description  : Sends data on a TCP non blocking connection and waits for
 *                it to finish.
 * Returns      : > 0 - bytes send, < 0 - TCP error
 ***************************************************************************/
static int cfe_web_tcp_send( int s, char *buf, int size )
{
    int i, len = 0;

    for( i = 0; i < size; i += len )
    {
        POLL();
        cfe_web_listen( &g_listen_idx );
        len = tcp_send( s, (unsigned char*)(buf + i), size - i );
        if( len < 0 )
        {
            console_log("web error: TCP write error sending a web page.");
            break;
        }
    }

    return( len );
} /* cfe_web_tcp_send */

#else

/***************************************************************************
 * Function Name: Functions stubs.
 * Description  : Used when the web server is not compiled into the CFE.
 * Returns      : None.
 ***************************************************************************/

int cfe_web_check(void);
void cfe_web_fg_process(void);
void cfe_web_poll(void *x);

int cfe_web_check(void)
{
    return(0);
}

void cfe_web_fg_process(void)
{
}

void cfe_web_poll(void *x)
{
}

#endif

