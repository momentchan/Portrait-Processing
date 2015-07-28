/* Subroutine to generate B-spline basis functions and their derivatives for uniform open knot vectors.
	C code for An Introduction to NURBS
	by David F. Rogers. Copyright (C) 2000 David F. Rogers,
	All rights reserved.
	
	Name: dbasis.c
	Language: C
	Subroutines called: none
	Book reference: Section 3.10, Ex. 3.18, Alg. p. 283


    b1        = first term of the basis function recursion relation
    b2        = second term of the basis function recursion relation
    c         = order of the B-spline basis function
    d1[]      = array containing the derivative of the basis functions
                d1[1]) contains the derivative of the basis function associated with B1 etc.
    d2[]      = array containing the derivative of the basis functions
                d2[1] contains the derivative of the basis function associated with B1 etc.
    f1        = first term of the first derivative of the basis function recursion relation
    f2        = second term of the first derivative of the basis function recursion relation
    f3        = third term of the first derivative of the basis function recursion relation
    f4        = fourth term of the first derivative of the basis function recursion relation
    npts      = number of defining polygon vertices
    n[]       = array containing the basis functions
                n[1]) contains the basis function associated with B1 etc.
    nplusc    = constant -- npts + c -- maximum knot value
    s1        = first term of the second derivative of the basis function recursion relation
    s2        = second term of the second derivative of the basis function recursion relation
    s3        = third term of the second derivative of the basis function recursion relation
    s4        = fourth term of the second derivative of the basis function recursion relation
    t         = parameter value
    temp[]    = temporary array
    x[]       = knot vector
*/	
#include "bspline.h"

void cv_BSpline_DBasis(int c,double t,int npts,double *x,double *n,double *d1,double *d2)
{
  int nplusc;
  int i,k;
  double b1,b2;
  double f1,f2,f3,f4;
  double s1,s2,s3,s4;
  double *temp;
  double *temp1;
  double *temp2;
  
  nplusc = npts + c;
  
  temp=(double *)malloc((nplusc+1)*sizeof(double));
  temp2=(double *)malloc((nplusc+1)*sizeof(double));
  temp1=(double *)malloc((nplusc+1)*sizeof(double));

  /*    zero the temporary arrays */
  
  for (i = 1; i <= nplusc; i++)
    {
      temp[i] = 0.0;
      temp1[i] = 0.0;
      temp2[i] = 0.0;
    }

  /* calculate the first order basis functions n[i] */
  
  for (i = 1; i<= nplusc-1; i++)
    {
      if (( t >= x[i]) && (t < x[i+1]))
	temp[i] = 1;
      else
	temp[i] = 0;
    }
  
  //  if (t == (double)x[nplusc])
  if( fabs(t-x[nplusc])<=FLT_EPSILON )
    {
      /*    pick up last point	*/
      temp[npts] = 1;
    }
  
  /* calculate the higher order basis functions */

  for (k = 2; k <= c; k++)
    {
      for (i = 1; i <= nplusc-k; i++)
	{
	  /* if (temp[i] != 0) if the lower order basis function is zero skip the calculation */
	  if( temp[i] > FLT_EPSILON )
	    b1 = ((t-x[i])*temp[i])/(x[i+k-1]-x[i]);
	  else
	    b1 = 0;
	  
	  //	  if (temp[i+1] != 0) /* if the lower order basis function is zero skip the calculation */
	  if( temp[i+1] >FLT_EPSILON )
	    b2 = ((x[i+k]-t)*temp[i+1])/(x[i+k]-x[i+1]);
	  else
	    b2 = 0;
	  
	  /* calculate first derivative */
	  
	  //	  if (temp[i] != 0) /* if the lower order basis function is zero skip the calculation */
	  if( temp[i] > FLT_EPSILON )
	    f1 = temp[i]/(x[i+k-1]-x[i]);
	  else
	    f1 = 0;

	  //	  if (temp[i+1] != 0) /* if the lower order basis function is zero skip the calculation */
	  if( temp[i+1] > FLT_EPSILON )
	    f2 = -temp[i+1]/(x[i+k]-x[i+1]);
	  else
	    f2 = 0;
	  

	  //	  if (temp1[i] != 0)  /* if the lower order basis function is zero skip the calculation */
	  if ( fabs(temp1[i]) > FLT_EPSILON )
	    f3 = ((t-x[i])*temp1[i])/(x[i+k-1]-x[i]);
	  else
	    f3 = 0;
	  
	  //	  if (temp1[i+1] != 0) /* if the lower order basis function is zero skip the calculation */
	  if ( fabs(temp1[i+1]) > FLT_EPSILON )
	    f4 = ((x[i+k]-t)*temp1[i+1])/(x[i+k]-x[i+1]);
	  else
	    f4 = 0;
	  
	  /*       calculate second derivative */
	  
	  //	  if (temp1[i] != 0)  /* if the lower order basis function is zero skip the calculation */
	  if ( fabs(temp1[i]) > FLT_EPSILON )
	    s1 = (2*temp1[i])/(x[i+k-1]-x[i]);
	  else
	    s1 = 0;
	  
	  //	  if (temp1[i+1] != 0) /* if the lower order basis function is zero skip the calculation */
	  if( fabs(temp1[i+1]) > FLT_EPSILON )
	    s2 = (-2*temp1[i+1])/(x[i+k]-x[i+1]);
	  else 
	    s2 = 0;
	  
	  //	  if (temp2[i] != 0) /* if the lower order basis function is zero skip the calculation */
	  if( fabs(temp2[i]) > FLT_EPSILON )
	    s3 = ((t-x[i])*temp2[i])/(x[i+k-1]-x[i]);
	  else
	    s3 = 0;
	  
	  //	  if (temp2[i+1] != 0) /* if the lower order basis function is zero skip the calculation */
	  if( fabs(temp2[i+1]) > FLT_EPSILON )
	    s4 = ((x[i+k]-t)*temp2[i+1])/(x[i+k]-x[i+1]);
	  else
	    s4 = 0;
	  
	  temp[i] = b1 + b2;
	  temp1[i] = f1 + f2 + f3 + f4;
	  temp2[i] = s1 + s2 + s3 + s4;
	}
    }
  
  /* put in array; ONLY those which are NOT NULL */
  if( n != NULL )
    {
      for (i = 1; i <= npts; i++) 
	{
	  n[i] = temp[i];
	}
    }

  if( d1 != NULL )
    {
      for(i = 1;i <= npts; i++)
	{
	  d1[i] = temp1[i];
	}
    }

  if( d2 != NULL )
    {
      for(i = 1;i <= npts; i++)
	{
	  d2[i] = temp2[i];
	}
    }

  free(temp);
  free(temp1);
  free(temp2);
}
