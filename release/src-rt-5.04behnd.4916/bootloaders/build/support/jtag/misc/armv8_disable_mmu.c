unsigned int sctlr, el;
void dcache_disable(void)
{

#define CR_M            (1 << 0)        /* MMU enable */
#define CR_C            (1 << 2)        /* Dcache enable                */
#define CR_I            (1 << 12)       /* Icache enable */



        asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
        el =  el >> 2;

	if (el == 1)
                asm volatile("mrs %0, sctlr_el1" : "=r" (sctlr) : : "cc");
        else if (el == 2)
                asm volatile("mrs %0, sctlr_el2" : "=r" (sctlr) : : "cc");
        else
                asm volatile("mrs %0, sctlr_el3" : "=r" (sctlr) : : "cc");


        /* if cache isn't enabled no need to disable */
        if (!(sctlr & CR_C))
                return;

        sctlr=sctlr & ~(CR_C|CR_M);
        sctlr=(sctlr & ~CR_I);

	if (el == 1)
                asm volatile("msr sctlr_el1, %0" : : "r" (sctlr) : "cc");
        else if (el == 2)
                asm volatile("msr sctlr_el2, %0" : : "r" (sctlr) : "cc");
        else
                asm volatile("msr sctlr_el3, %0" : : "r" (sctlr) : "cc");

        asm volatile("isb");


}
