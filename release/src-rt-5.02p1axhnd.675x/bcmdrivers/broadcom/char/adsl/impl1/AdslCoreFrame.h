/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/****************************************************************************
 *
 * AdslCoreFrame.h
 *
 * Description:
 *		This file contains prototypes for AdslCore frame functions
 *
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslCoreFrame.h,v 1.1 2004/04/08 21:24:49 ilyas Exp $
 *
 * $Log: AdslCoreFrame.h,v $
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 *
 *****************************************************************************/

DslFrameDeclareFunctions (AdslCoreFrame)

#define	 AdslCoreAssignDslFrameFunctions(var)	do {					\
	(var).__DslFrameAllocMemForFrames	= AdslCoreFrameAllocMemForFrames;	\
	(var).__DslFrameFreeMemForFrames	= AdslCoreFrameFreeMemForFrames;		\
	(var).__DslFrameAllocFrame		= AdslCoreFrameAllocFrame;			\
	(var).__DslFrameFreeFrame			= AdslCoreFrameFreeFrame;		\
	(var).__DslFrameAllocMemForBuffers= AdslCoreFrameAllocMemForBuffers;	\
	(var).__DslFrameFreeMemForBuffers = AdslCoreFrameFreeMemForBuffers;	\
	(var).__DslFrameAllocBuffer		= AdslCoreFrameAllocBuffer;			\
	(var).__DslFrameFreeBuffer		= AdslCoreFrameFreeBuffer;			\
	(var).__DslFrame2Id				= AdslCoreFrame2Id;					\
	(var).__DslFrameId2Frame			= AdslCoreFrameId2Frame;			\
} while (0)
