#include "sbmips.h"
#include "mipsmacros.h"
#include "bcm_map.h"


#define SETLEDS1(a,b,c,d) \
             li a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;\
             bal board_setleds


#define SETLEDS2(a,b,c,d) \
             li a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;\
             bal board_setleds1
             

#define CHECK_NUM_TIMES (100)

#if defined(_BCM963268_)
    #define GPIO_MODE_EXT_IRQ0 32
#endif


#if !defined(GPIO_PIN)
    /* Usually the restore config. pin EXT_IRQ[0] */
    #define GPIO_PIN (GPIO_MODE_EXT_IRQ0)
#endif

#if !defined(GPIO_VALUE)
    #define GPIO_VALUE 0
#endif

#if (GPIO_PIN > 31)
   #define GPIO_DATA_REG      GPIO_DATA_HI
   #define GPIO_BIT_TO_TEST   ( 1<<(GPIO_PIN-32) )
#else
   #define GPIO_DATA_REG      GPIO_DATA
   #define GPIO_BIT_TO_TEST   ( 1<<(GPIO_PIN) )
#endif


LEAF(gpio_loop)

    move    s0, ra

    SETLEDS1('C', 'K', 'L', 'P')
    
    li      t1, CHECK_NUM_TIMES 

   
checkgpio:    
    li      a1, 0xA5A5
    li      a0, GPIO_DATA_REG
    bal     board_print_register


    SETLEDS2(' ',' ',' ',' ')
    
    li      a1, 0x0
    lw      a0, GPIO_DATA_REG
    bal     board_print_register

    SETLEDS2('I','O',' ',' ')
    SETLEDS2('V','A','L',' ') 
    
    lw      t0, GPIO_DATA_REG
    li      t2, GPIO_BIT_TO_TEST  
    and     t0, t2
    beqz    t0, iszero
    li      t0, 1
    
iszero:    
    move    a0, t0
    bal     board_print_register

    li      t2, GPIO_VALUE    
    beq     t0, t2, deadend

    
    addi    t1, -1
  
    bgtz    t1, checkgpio
    b       2f

deadend:        
    SETLEDS1('L', 'O', 'O', 'P')         
1: 
    nop
    nop
    b       1b
    nop
    nop         

    
2:
    SETLEDS1('N', 'O', 'L', 'P')         

    move    ra, s0
    j       ra
        
END(gpio_loop)

