

#ifndef _wlc_pkt_filter_h_
#define _wlc_pkt_filter_h_


/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */


/* Forward declaration */
typedef struct pkt_filter_info	wlc_pkt_filter_info_t;


/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef PACKET_FILTER

/*
*****************************************************************************
* Function:   wlc_pkt_filter_attach
*
* Purpose:    Initialize packet filter private context.
*
* Parameters: context	(mod)	Common driver context.
*
* Returns:    Pointer to the packet filter private context. Returns NULL on error.
*****************************************************************************
*/
extern wlc_pkt_filter_info_t *wlc_pkt_filter_attach(void *context);

/*
*****************************************************************************
* Function:   wlc_pkt_filter_detach
*
* Purpose:    Cleanup packet filter private context.
*
* Parameters: info	(mod)	Packet filter engine private context.
*
* Returns:    Nothing.
*****************************************************************************
*/
extern void wlc_pkt_filter_detach(wlc_pkt_filter_info_t *info);


/*
*****************************************************************************
* Function:   wlc_pkt_fitler_recv_proc
*
* Purpose:    Process received frames.
*
* Parameters: info	(mod)	Packet filter engine private context.
*	      sdu	(in)	Received packet..
*
* Returns:    TRUE if received packet should be forwarded. FALSE if
*             it should be discarded.
*****************************************************************************
*/
extern bool wlc_pkt_filter_recv_proc(wlc_pkt_filter_info_t *info, const void *sdu);


/*
*****************************************************************************
* Function:   wlc_pkt_fitler_delete_filters
*
* Purpose:    Delete all filters but remain attached.
*
* Parameters: info	(mod)	Packet filter engine private context.
*
* Returns:    None.
*****************************************************************************
*/
extern void wlc_pkt_fitler_delete_filters(wlc_pkt_filter_info_t *info);


#ifdef BCM_OL_DEV
/*
*****************************************************************************
* Function:   wlc_pkt_fitler_recv_proc_ex
*
* Purpose:    Process received frames.
*
* Parameters:	info	(mod)	Packet filter engine private context.
*		sdu	(in)	Received packet.
*		id      (out)	ID of matching filter
*
* Returns:    TRUE if received packet matches a filter. Otherwise, FALSE.
*****************************************************************************
*/
extern bool wlc_pkt_filter_recv_proc_ex(
			wlc_pkt_filter_info_t *info,
			const void *sdu,
			uint32 *id);
#endif	/* BCM_OL_DEV */

#ifdef BCM_OL_DEV
/*
*****************************************************************************
* Function:   wlc_pkt_filter_add
*
* Purpose:    Install a new packet filter.
*
* Parameters: info               (mod) Packet filter engine context state.
*             pkt_filter         (in)  Filter data from host driver.
*
* Returns:    BCME_OK on success, else BCME_xxx error code.
*****************************************************************************
*/
extern int wlc_pkt_filter_add(wlc_pkt_filter_info_t *info, wl_pkt_filter_t *pkt_filter);
#endif	/* BCM_OL_DEV */

#else	/* stubs */

#define wlc_pkt_filter_attach(a)	(wlc_pkt_filter_info_t *)0x0dadbeef
#define wlc_pkt_filter_detach(a)	do {} while (0)
#define wlc_pkt_filter_recv_proc(a, b)	(TRUE)
#ifdef BCM_OL_DEV
#define wlc_pkt_fitler_delete_filters(a)	do {} while (0)
#define wlc_pkt_filter_add(a, b)		(FALSE)
#endif

#endif /* PACKET_FILTER */

#endif	/* _wlc_pkt_filter_h_ */
