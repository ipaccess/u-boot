/* TomsFastMath, a fast ISO C bignum library.
 * 
 * This project is meant to fill in where LibTomMath
 * falls short.  That is speed ;-)
 *
 * This project is public domain and free for all purposes.
 * 
 * Tom St Denis, tomstdenis@gmail.com
 */
#include "tfm.h"

static fp_int  t;

void fp_to_unsigned_bin(fp_int *a, unsigned char *b)
{
  int     x;

  fp_init_copy(&t, a);

  x = 0;
  while (fp_iszero (&t) == FP_NO) {
      b[x++] = (unsigned char) (t.dp[0] & 255);
      fp_div_2d (&t, 8, &t, NULL);
  }
  fp_reverse (b, x);
}

/* $Source: /cvs/libtom/tomsfastmath/src/bin/fp_to_unsigned_bin.c,v $ */
/* $Revision: 1.2 $ */
/* $Date: 2007/02/27 02:38:44 $ */