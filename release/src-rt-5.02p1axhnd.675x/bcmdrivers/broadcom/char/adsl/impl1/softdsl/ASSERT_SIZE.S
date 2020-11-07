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
#include "MipsAsm.h"
	
        .text
        .align  2
        .globl  ASSERT_SIZE
        .ent    ASSERT_SIZE    2
ASSERT_SIZE: 
	.word 0xdeadbeef ; 
	break 0x123; 

        .end    ASSERT_SIZE    
