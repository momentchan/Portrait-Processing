#include <bspline.h>
//#include <cv.h>
//#include <highgui.h>
/*
This program generates an approximating curve for a given set of
input points. 

If basis == NULL, then the function chooses the 
number of basis & the order of spline(always 4). 
The user will then have to specify whether the 
curve is open or closed. 
If basis is specified, then the internal basis
points to the given bassis and count increased.
Return value is 0 on success, non-0, generally -1
on failure.
*/

int cv_BSpline_Approx(CvMat const *points, Cv_BSpline *curve, int type, Cv_Basis *basis)
{
  Cv_Basis *temp;

  if( curve == NULL )
    {
      printf("In cv_BSpline_Approx: NULL argument passed.\n");
      return(-1);
    }
  /* if no basis func given, use the type information to generate a basis */
  if( basis == NULL )
    {
      temp = (Cv_Basis *)malloc(sizeof(Cv_Basis));
      (*temp).Type = type;
      (*temp).K = 4;
      (*temp).N = points->rows;
      (*temp).N_b = (int)( 0.75 * (points->rows) );
      (*temp).Refcount++;
      if( icv_InitBasis( temp ) )
	{
	  printf("Error in initialising basis.\n");
	  /* Return NULL when a non-NULL pointer is expected */
	  return(-1);
	}
    }
  else
    {
      /* If basis is given but curve->Basis does not exist ... */
      if( curve->Basis == NULL )
	{
	  curve->Basis = basis;
	  basis->Refcount++;
	}
      /* If basis & curve->basis are not same and basis exists... */
      if( basis != curve->Basis )
	{
	  /* Clean the existing basis  and allocate new one */
	  curve->Basis->Refcount -= 1;
	  icv_Basis_Free( curve->Basis);
	  if(( curve->Basis->Refcount == 0) && ( curve->Basis->DAlloc == 1))
	    {
	      free( curve->Basis );
	    }
	}
    }
  if( curve->Control_points == NULL )
    curve->Control_points = cvCreateMat( curve->Basis->N_b, 2, CV_64FC1);
  if( curve->Curve_points == NULL )
    curve->Curve_points = cvCreateMat( curve->Basis->N, 2, CV_64FC1);
  if( curve->Curve_D1 == NULL )
    curve->Curve_D1 = cvCreateMat( curve->Basis->N, 2, CV_64FC1);
  if( curve->Curve_D2 == NULL )
    curve->Curve_D2 = cvCreateMat( curve->Basis->N, 2, CV_64FC1);
  if( curve->Curve_Normal == NULL )
    curve->Curve_Normal = cvCreateMat( curve->Basis->N, 2, CV_64FC1);

  if( icv_Periodic_LS( points, curve ) == 0)
    return 0;
  else
    return(-1);
}

int icv_Periodic_LS(CvMat const *input, Cv_BSpline *curve)
{
  double *knot;
  double *t; /* Parameter Value */
  double temp=0.0,sum,temp1,temp2,temp3=0.0;
  CvMat *A,*B,*px,*py,*bx,*by;
  int i,j,l, Order, NBasis;
  double **partial;

  Order = curve->Basis->K;
  NBasis = curve->Basis->N_b;
#ifdef VERBOSE
  printf("In icv_Periodic_LS: Entering function.\nOrder: %d, NBasis:%d\nGenerating knot vector.\n", Order, NBasis);
#endif

  /* Create and Generate the knot vector array */
  if( curve->Basis->Type == CV_BSPLINE_CLOSED_CURVE)
    {  
      knot = (double *)malloc((NBasis + 2*Order)*sizeof(double));
      for( i = 1; i <= NBasis + 2*Order - 1; i++ )
		knot[i] = (double)(i - 1);
    }
  else if(curve->Basis->Type == CV_BSPLINE_OPEN_CURVE)
    {
      knot = (double *)malloc(( NBasis + Order + 1 )*sizeof( double ));
      for( i = 1; i <= Order; i++ )
	knot[i] = 0.0;
      
      for( i = Order + 1; i <= NBasis; i++ )
	knot[i] = ( double )(i - Order);
      
      for( i = NBasis + 1; i <= NBasis + Order; i++ )
	knot[i] = (double) (NBasis + 1 - Order);
    }
  else
    {
      fprintf( stderr, "Curve type not set.\n"); fflush(stderr);
      return(-1);
    }
  
  A = cvCreateMat( NBasis, NBasis, CV_64FC1);
  /* Number of points */
  t = (double *)malloc((input->rows + 1)*sizeof(double));

#ifdef VERBOSE
  printf("In icv_Periodic_LS: Arc length parameterisation.\n");
#endif
  cv_BSpline_Param( input->rows, input, t); /* Arc length Parameterisation */
  partial = (double **)malloc(input->rows*sizeof(double *));
  
  for( i = 0; i < input->rows; i++ )
    *(partial+i) = (double *)malloc( (NBasis + Order) * sizeof(double) );
  
  for(i = 0; i < input->rows; i++)
    {
      /* 
	 Rescaling of parameter to lie in the valid range: (k-1)<=t<=N_b+k-1 
	 Number of basis is increased to N_b+k-1 to close the curve
      */
      sum = 0.0;
      if( curve->Basis->Type == CV_BSPLINE_CLOSED_CURVE)
	{
	  cv_BSpline_Basis(Order, Order - 1 + t[i+1]*(NBasis), NBasis + Order - 1, knot, partial[i]);
	  for( j = 1; j <= NBasis + Order - 1; j++ )
	    sum+=partial[i][j];
	}
      else
	{
	  cv_BSpline_Basis( Order, (double)(t[i+1]*(NBasis-Order+1)), NBasis, knot, partial[i]);
	  for ( l = 1; l <= NBasis; l++ )
	    sum += partial[i][l];
	}

      if(fabs(sum - 1.0)>FLT_EPSILON)
	{
	  fprintf( stderr, "ERROR in calculating basis function.\n");
	  return(-1);
	}
    }
  /* Building the matrix */  
  for( i = 0; i < NBasis; i++ )
    {
      for( j = 0; j < NBasis; j++ )
	{
	  sum = 0.0;
	  for( l = 0;  l < input->rows; l++ )
	    {
	      if( curve->Basis->Type == CV_BSPLINE_CLOSED_CURVE)
		{
		  if( j <= Order - 2 )
		    temp = partial[l][NBasis + 1 + j];
		  else
		    temp = 0.0;
		  if( i <= Order - 2 )
		    temp3 = partial[l][NBasis + 1 + i];
		  else
		    temp3 = 0.0;
		}
	      temp1 = partial[l][i+1] + temp3;
	      temp2 = partial[l][j+1] + temp;
	      sum = sum + (temp1*temp2);
	    }
	  cvmSet(A, i, j, sum);
	}
    }
  
  B = cvCreateMat( NBasis, NBasis, CV_64FC1 );
  cvInvert( A, B, CV_LU );

  /* RHS of the Least Squares MAtrix Equations */
  px = cvCreateMat( NBasis, 1, CV_64FC1);
  py = cvCreateMat( NBasis, 1, CV_64FC1);
  
  /* Memory for Control Points */
  bx = cvCreateMat( NBasis, 1, CV_64FC1);
  by = cvCreateMat( NBasis, 1, CV_64FC1);

  for( i = 0; i < NBasis; i++ )
    {
      temp1 = 0.0;
      temp2 = 0.0;
      for( j = 0; j < input->rows; j++)
	{
	  if( curve->Basis->Type == CV_BSPLINE_CLOSED_CURVE)
	    {
	      if( i <= Order - 2 )
		temp = partial[j][NBasis + i + 1];
	      else
		temp=0.0;
	    }
	  temp1 += (double)(cvmGet(input, j, 0) * (partial[j][i+1] + temp));
	  temp2 += (double)(cvmGet(input, j, 1) * (partial[j][i+1] + temp));
	}

      cvmSet( px, i, 0, temp1);
      cvmSet( py, i, 0, temp2);
    }

  /* We get the control points after multiplying with RHS */
  cvGEMM( B, px, 1.0, NULL, 0.0, bx, 0 );
  cvGEMM( B, py, 1.0, NULL, 0.0, by, 0 );

  for( i = 0; i < NBasis; i++ )
    {
      cvmSet( curve->Control_points, i, 0, cvmGet( bx, i, 0));
      cvmSet( curve->Control_points, i, 1, cvmGet( by, i, 0));
    }

  /* Freeing Stuff */
  for( i = 0; i < input->rows; i++)
    free(*(partial+i));
  
  free(partial);
  cvReleaseMat(&A);
  cvReleaseMat(&B);
  cvReleaseMat(&bx);
  cvReleaseMat(&by);
  cvReleaseMat(&px);
  cvReleaseMat(&py);
  free(t);
  free(knot);

#ifdef VERBOSE
  printf("Leaving closed least Squares Estimation.\n");
#endif

  return 0;
}

int cv_BSpline_Manual_Init( IplImage *im, Cv_BSpline *curve )
{
  CvMat *points;
  IplImage *dummy;

  if( im->nChannels == 1 ) // If GrayScale; create a dummy colour version
    {
      dummy = cvCreateImage(cvGetSize(im),im->depth,3);
      cvCvtColor( im, dummy, CV_GRAY2BGR );
    }
  else
    dummy = cvCloneImage(im);

  points = ginput( *dummy);
  printf("\n");
  cvReleaseImage(&dummy);
  /* Check if basis is not allocated */
  curve->Basis = (Cv_Basis *)malloc(sizeof(Cv_Basis));
  curve->Basis->DAlloc = 1;
  curve->Basis->K = 4;
  curve->Basis->N_b = (int)(0.75*points->rows);
  curve->Basis->N = 500;
  curve->Basis->Type = CV_BSPLINE_CLOSED_CURVE;

  if( icv_InitBasis( curve->Basis ) )
    {
      printf("In cv_BSpline_Manual_Init: Error in initialisation of basis function.\n");
      return(-1);
    }

  curve->Basis->Refcount++;
  curve->Control_points = NULL;
  curve->Curve_points = NULL;
  curve->Curve_D1 = NULL;
  curve->Curve_D2 = NULL;
  curve->Curve_Normal = NULL;


  /* printf("Enter the ORDER of spline, K="); */
  /* scanf("%d", &(curve->Basis->K) ); */
  /* curve->Basis->control_points = (CvPoint2D32f *)malloc(input->Basis->N_b*sizeof(CvPoint2D32f)); */

  if( cv_BSpline_Approx( points, curve, CV_BSPLINE_CLOSED_CURVE, curve->Basis ) != 0 )
    {
      fprintf(stderr, "Approximation error.\n"); fflush(stderr);
      return(-1);
    }

  if( cv_BSpline_Compute(curve, (CV_BSPLINE_COMPUTE_CURVE|CV_BSPLINE_COMPUTE_DERIV1|CV_BSPLINE_COMPUTE_DERIV2), NULL) )
    {
      printf("Problem in initialising curve.\n");
      return(-1);
    }
  if( icv_BSpline_Compute_Normal( curve ) != 0 )
    {
      fprintf( stderr, "In cv_BSpline_Read_File: Unable to compute curve normal.\n");
    }
  cvReleaseMat( &points );
  return 0;
}

