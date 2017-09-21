/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef _UDB_IOCTL_H_
#define _UDB_IOCTL_H_

enum
{
	UDB_IOCTL_NR_NA = 0x00,
	UDB_IOCTL_NR_INTERNAL,	/* internal usage */
	UDB_IOCTL_NR_COMMON,
	UDB_IOCTL_NR_WRS,
	UDB_IOCTL_NR_WBL,
	UDB_IOCTL_NR_IQOS,
	UDB_IOCTL_NR_PATROL_TQ,
	UDB_IOCTL_NR_VP,
	UDB_IOCTL_NR_ANOMALY,
	UDB_IOCTL_NR_DLOG,
	UDB_IOCTL_NR_HWNAT,
	UDB_IOCTL_NR_MAX
};

#ifdef __KERNEL__
int udb_ioctl_copy_out(uint8_t nr, uint8_t op, void *buf, uint32_t buf_len, uint32_t *buf_used_len);
int udb_ioctl_copy_in(uint8_t nr, uint8_t op, void *buf, uint32_t buf_len);
int udb_ioctl_copy_none(uint8_t nr, uint8_t op);
#endif

#endif /* _UDB_IOCTL_H_ */
