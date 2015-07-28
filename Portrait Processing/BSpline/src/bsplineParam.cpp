/* Subroutine to calculate parameter values based on chord distances.

    C code for An Introduction to NURBS
    by David F. Rogers. Copyright (C) 2000 David F. Rogers,
    All rights reserved.
    
    Name: param.c
    Language: C
    Subroutines called: none
    Book reference: Alg. p. 295

    d[]        = array containing the data points
    dpts       = number of data points
    isum       = incremental sum of the chord distances 
    sum        = sum of all the chord distances
    tparm[]    = array containing the parameter values
*/
#include "bspline.h"
// #include "../include/bspline.h"
void cv_BSpline_Param(int dpts,CvMat const *input,double *tparm)
{
  int i;
  int icount;
  
  double sum;
  double isum;
  /* double tempdx; */
  /* double tempdy; */

  sum = 0.0;
  isum = 0.0;
  tparm[1] = 0.0;
  
  /*    calculate the sum of the chord distances for all the data points */
  
  for( i = 0; i< (dpts - 1); i++)
    {

      /* tempdx = pow( ( cvmGet( input, i + 1, 0) - cvmGet( input, i, 0), 2 )); */
      /* tempdx = pow( ( cvmGet( input, i + 1, 1) - cvmGet( input, i, 1), 2 )); */
      /* tempdy = (input[i+1].y - input[i].y)*(input[i+1].y - input[i].y); */
      /*      tempdz = (d[i+5] - d[i+2])*(d[i+5] - d[i+2]);     */
      sum += (double)hypot( cvmGet( input, i + 1, 0) - cvmGet( input, i, 0), cvmGet( input, i + 1, 1) - cvmGet( input, i, 1));
      /* sum += sqrt(tempdx+tempdy); */
      /*     fprintf(stderr,"%d %lf %lf\n",i,input[i].x,input[i].y);*/
    }
  
  /*    calculate the parameter values */

  icount = 2;
  
  for( i = 0; i < (dpts-1); i++)
    {
      /* tempdx = pow( ( cvmGet( input, i + 1, 0) - cvmGet( input, i, 0), 2 )); */
      /* tempdx = pow( ( cvmGet( input, i + 1, 1) - cvmGet( input, i, 1), 2 )); */
      /* isum = isum + sqrt(tempdx+tempdy); */
      isum += (double)hypot( cvmGet( input, i + 1, 0) - cvmGet( input, i, 0), cvmGet( input, i + 1, 1) - cvmGet( input, i, 1));
      tparm[icount] = isum/sum;
      icount = icount + 1;
    }
  return;
}
