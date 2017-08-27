

#ifndef _wlc_d0_filter_h_
#define _wlc_d0_filter_h_


/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */


/* Forward declaration */
typedef struct d0_filter_info	wlc_d0_filter_info_t;


/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef PACKET_FILTER

/*
*****************************************************************************
* Function:   wlc_d0_filter_attach
*
* Purpose:    Initialize packet filter private context.
*
* Parameters: wlc	(mod)	Common driver context.
*
* Returns:    Pointer to the packet filter private context. Returns NULL on error.
*****************************************************************************
*/
extern wlc_d0_filter_info_t *wlc_d0_filter_attach(wlc_info_t *wlc);


/*
*****************************************************************************
* Function:   wlc_d0_filter_detach
*
* Purpose:    Cleanup packet filter private context.
*
* Parameters: info	(mod)	Packet filter engine private context.
*
* Returns:    Nothing.
*****************************************************************************
*/
extern void wlc_d0_filter_detach(wlc_d0_filter_info_t *info);


/*
*****************************************************************************
* Function:   wlc_pkt_fitler_recv_proc
*
* Purpose:    Process received frames.
*
* Parameters: info	(mod)	Packet filter engine private context.
*				  sdu		(in)	Received packet.
*
* Returns:    TRUE if received packet should be forwarded. FALSE if
*             it should be discarded.
*****************************************************************************
*/
extern bool wlc_d0_filter_recv_proc(wlc_d0_filter_info_t *info, void *sdu);


#else	/* stubs */

#define wlc_d0_filter_attach(a)	(wlc_d0_filter_info_t *)0x0dadbeef
#define wlc_d0_filter_detach(a)	do {} while (0)
#define wlc_d0_filter_recv_proc(a, b)	(TRUE)

#endif /* PACKET_FILTER */

#endif	/* _wlc_d0_filter_h_ */
