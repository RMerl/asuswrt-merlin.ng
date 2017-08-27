/*
------------------------------------------------------------------------------
http://burtleburtle.net/bob/rand/isaac.html
rand.c: By Bob Jenkins.  My random number generator, ISAAC.  Public Domain
MODIFIED:
  960327: Creation (addition of randinit, really)
  970719: use context, not global variables, for internal state
  980324: make a portable version
  010626: Note this is public domain
------------------------------------------------------------------------------
*/

RCSID("$Id$")

#include <freeradius-devel/libradius.h>

#define RANDSIZL   (8)  /* I recommend 8 for crypto, 4 for simulations */
#define RANDSIZ    (1<<RANDSIZL)

#define ind(mm,x)  ((mm)[(x>>2)&(RANDSIZ-1)])
#define rngstep(mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = ((a^(mix)) + *(m2++)) & 0xffffffff; \
  *(m++) = y = (ind(mm,x) + a + b) & 0xffffffff; \
  *(r++) = b = (ind(mm,y>>RANDSIZL) + x) & 0xffffffff; \
}

void fr_isaac(fr_randctx *ctx)
{
   register uint32_t a,b,x,y,*m,*mm,*m2,*r,*mend;
   mm=ctx->randmem; r=ctx->randrsl;
   a = ctx->randa; b = (ctx->randb + (++ctx->randc)) & 0xffffffff;
   for (m = mm, mend = m2 = m+(RANDSIZ/2); m<mend; )
   {
      rngstep( a<<13, a, b, mm, m, m2, r, x);
      rngstep( a>>6 , a, b, mm, m, m2, r, x);
      rngstep( a<<2 , a, b, mm, m, m2, r, x);
      rngstep( a>>16, a, b, mm, m, m2, r, x);
   }
   for (m2 = mm; m2<mend; )
   {
      rngstep( a<<13, a, b, mm, m, m2, r, x);
      rngstep( a>>6 , a, b, mm, m, m2, r, x);
      rngstep( a<<2 , a, b, mm, m, m2, r, x);
      rngstep( a>>16, a, b, mm, m, m2, r, x);
   }
   ctx->randb = b; ctx->randa = a;
}


#define mix(a,b,c,d,e,f,g,h) \
{ \
   a^=b<<11; d+=a; b+=c; \
   b^=c>>2;  e+=b; c+=d; \
   c^=d<<8;  f+=c; d+=e; \
   d^=e>>16; g+=d; e+=f; \
   e^=f<<10; h+=e; f+=g; \
   f^=g>>4;  a+=f; g+=h; \
   g^=h<<8;  b+=g; h+=a; \
   h^=a>>9;  c+=h; a+=b; \
}

/* if (flag==1), then use the contents of randrsl[] to initialize mm[]. */
void fr_randinit(fr_randctx *ctx, int flag)
{
  int i;
  uint32_t a,b,c,d,e,f,g,h;
  uint32_t *m,*r;
  ctx->randa = ctx->randb = ctx->randc = 0;
  m=ctx->randmem;
  r=ctx->randrsl;
  a=b=c=d=e=f=g=h=0x9e3779b9;  /* the golden ratio */

  for (i=0; i<4; ++i) {	/* scramble it */
    mix(a,b,c,d,e,f,g,h);
  }

  if (flag) {
    /* initialize using the contents of r[] as the seed */
    for (i=0; i<RANDSIZ; i+=8) {
      a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
      e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];
      mix(a,b,c,d,e,f,g,h);
      m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
      m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
    }
     /* do a second pass to make all of the seed affect all of m */
    for (i=0; i<RANDSIZ; i+=8) {
      a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
      e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];
      mix(a,b,c,d,e,f,g,h);
      m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
      m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
    }
  } else {
    for (i=0; i<RANDSIZ; i+=8) {
      /* fill in mm[] with messy stuff */
      mix(a,b,c,d,e,f,g,h);
      m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
      m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
    }
  }

  fr_isaac(ctx);       /* fill in the first set of results */
  ctx->randcnt=RANDSIZ;  /* prepare to use the first set of results */
}


#ifdef TEST
/*
 *  For testing.  Output should be the same as
 *
 *  http://burtleburtle.net/bob/rand/randvect.txt
 */
int main()
{
  uint32_t i,j;
  fr_randctx ctx;

  ctx.randa = ctx.randb = ctx.randc = (uint32_t)0;

  for (i=0; i<256; ++i) ctx.randrsl[i]=(uint32_t)0;
  fr_randinit(&ctx, 1);
  for (i=0; i<2; ++i) {
    fr_isaac(&ctx);
    for (j=0; j<256; ++j) {
      printf("%.8lx",ctx.randrsl[j]);
      if ((j&7)==7) printf("\n");
    }
  }
}
#endif
