#include "bspline.h"
//#include "../include/bspline.h"

double cv_BSpline_Length(Cv_BSpline const *curve)
{
  int i;
  double length,g;

  length = 0.0;
  for(i = 0; i <= curve->Basis->N - 1; i++)
    {
      g = hypot( cvmGet(curve->Curve_D1, i, 0), cvmGet(curve->Curve_D1, i,1) );
      length += g;
    }

  length *= (curve->Basis->N_b + 1.0 - curve->Basis->K);
  length /= (curve->Basis->N);

  return length;
}

void cv_BSpline_Curvature(Cv_BSpline const *curve, double *curvature)
{
  int i;
  CvMat *d1,*d2;

  d1 = curve->Curve_D1;
  d2 = curve->Curve_D2;
  //  printf("Curvature function\n"); fflush(stdout);
  for( i = 0; i < curve->Basis->N; i++ )
    {
      curvature[i] = (double)(cvmGet(d1,i,0)*cvmGet(d2, i, 1)-cvmGet(d1, i, 1)*cvmGet(d2, i, 0))/pow( hypot(cvmGet(d1, i, 0),cvmGet(d1, i, 1)), 1.5);
    }

  return;
}


/* This function generates the spline points and the derivatives given the individual spline sums */
/* The spline points are stored in the spline data structure; the derivatives in d1 & d2 */
int icv_BSpline_Compute_Curve( Cv_BSpline *curve, Cv_Basis const *curr_basis )
{
  int i,j,NC;
  double temp1,temp2;

  if(curr_basis->Type == CV_BSPLINE_CLOSED_CURVE)
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Computing closed curve.\n");
#endif
      NC = curr_basis->N_b + curr_basis->K - 1;/* no of ctrl pts */
    }
  else if( curr_basis->Type == CV_BSPLINE_OPEN_CURVE )
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Computing open curve.\n");
#endif

      NC = curr_basis->N_b;
    }
  else
    {
      fprintf(stderr,"Error icv_BSpline_Init: Curve type not set.\n");
      return(-1);
    }
  /* Computing the curve points */
  for( i = 0; i < curr_basis->N; i++)
    {
      temp1 = 0.0;
      temp2 = 0.0;
      
      for( j = 1;j <= NC; j++ )
	{
	  temp1 += (double)( curr_basis->Basis[i][j]*cvmGet( curve->Control_points, (j-1)%curr_basis->N_b, 0 ));
	  temp2 += (double)( curr_basis->Basis[i][j]*cvmGet( curve->Control_points, (j-1)%curr_basis->N_b, 1 ));
	}
      cvmSet( curve->Curve_points, i, 0, temp1 );
      cvmSet( curve->Curve_points, i, 1, temp2 );
    }
  return 0;
}

/* Assumes that the 1st derivative has already been computed */
int icv_BSpline_Compute_Normal(Cv_BSpline *curve)
{
  int n;
  double d1_y, d1_x;
  if( curve == NULL )
    {
      fprintf( stderr,"In icv_BSpline_Compute_Normal: NULL argument(s) passed.\n");
      return(-1);
    }
  else if( curve->Curve_D1 == NULL )
    {
      fprintf(stderr, "In icv_BSpline_Compute_Normal: First derivative not computed.\n");
      return(-1);
    }
  else
    {
      for( n = 0; n < curve->Basis->N; n++ )
	{
	  d1_y = (double)cvmGet(curve->Curve_D1, n, 1);
	  d1_x = (double)cvmGet(curve->Curve_D1, n, 0);
	  cvmSet( curve->Curve_Normal, n, 0, ( -d1_y/sqrt(d1_x*d1_x + d1_y*d1_y) ) );
	  cvmSet( curve->Curve_Normal, n, 1, ( d1_x/sqrt(d1_x*d1_x + d1_y*d1_y) ) );
	}
      return 0;
    }

}
int icv_BSpline_Compute_Deriv1( Cv_BSpline *curve, Cv_Basis const *curr_basis )
{
  int i,j,NC;
  double temp1,temp2;
  if(curr_basis->Type == CV_BSPLINE_CLOSED_CURVE)
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Computing closed curve 1st derivative.\n");
#endif
      NC = curr_basis->N_b + curr_basis->K - 1; /* no of ctrl pts */
    }
  else if(curr_basis->Type == CV_BSPLINE_OPEN_CURVE)
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Computing open curve.\n");
#endif

      NC = curr_basis->N_b;
    }
  else
    {
      fprintf(stderr,"Error icv_BSpline_Init: Curve type not set.\n");
      return(-1);
    }
  /* Computing the curve points */
  for( i = 0; i < curr_basis->N; i++)
    {
      temp1 = 0.0;
      temp2 = 0.0;
      
      for( j = 1;j <= NC; j++ )
	{
	  temp1 += (double)( curr_basis->Deriv1[i][j]*cvmGet( curve->Control_points, (j-1)%curr_basis->N_b, 0 ));
	  temp2 += (double)( curr_basis->Deriv1[i][j]*cvmGet( curve->Control_points, (j-1)%curr_basis->N_b, 1 ));
	}
      cvmSet( curve->Curve_D1, i, 0, temp1 );
      cvmSet( curve->Curve_D1, i, 1, temp2 );
    }
  return 0;
}

int icv_BSpline_Compute_Deriv2( Cv_BSpline *curve, Cv_Basis const *curr_basis )
{
  int i,j,NC;
  double temp1,temp2;
  if(curr_basis->Type == CV_BSPLINE_CLOSED_CURVE)
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Computing closed curve 2nd derivative.\n");
#endif
      NC = curr_basis->N_b + curr_basis->K - 1;/* no of ctrl pts */
    }
  else if(curr_basis->Type == CV_BSPLINE_OPEN_CURVE)
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Computing open curve.\n");
#endif

      NC = curr_basis->N_b;
    }
  else
    {
      fprintf(stderr,"Error icv_BSpline_Init: Curve type not set.\n");
      return(-1);
    }
  /* Computing the curve points */
  for( i = 0; i < curr_basis->N; i++)
    {
      temp1 = 0.0;
      temp2 = 0.0;
      
      for( j = 1;j <= NC; j++ )
	{
	  temp1 += (double)( curr_basis->Deriv2[i][j]*cvmGet( curve->Control_points, (j-1)%curr_basis->N_b, 0 ));
	  temp2 += (double)( curr_basis->Deriv2[i][j]*cvmGet( curve->Control_points, (j-1)%curr_basis->N_b, 1 ));
	}
      cvmSet( curve->Curve_D2, i, 0, temp1 );
      cvmSet( curve->Curve_D2, i, 1, temp2 );
    }
  return 0;
}

int cv_BSpline_Compute( Cv_BSpline *curve, int FLAG, Cv_Basis const *basis )
{
  Cv_Basis const *curr_basis;

  /* use the given basis or .... */
  if(basis != NULL)
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Using external basis.\n");
#endif
      curr_basis = basis;
    }
  /* use the internal basis */
  else
    {
#ifdef VERBOSE
      printf("In cv_BSpline_Compute: Using \"internal\" basis.\n");
#endif
    curr_basis = (*curve).Basis;
    }

  if( FLAG == CV_BSPLINE_COMPUTE_CURVE )
    return icv_BSpline_Compute_Curve( curve, curr_basis );
  else if( FLAG == CV_BSPLINE_COMPUTE_DERIV1 )
    return icv_BSpline_Compute_Deriv1( curve, curr_basis );
  else if( FLAG == CV_BSPLINE_COMPUTE_DERIV2 )
    return icv_BSpline_Compute_Deriv2( curve, curr_basis );
  else if( FLAG == (CV_BSPLINE_COMPUTE_CURVE | CV_BSPLINE_COMPUTE_DERIV1) )
    {
      return ( icv_BSpline_Compute_Curve( curve, curr_basis ) | icv_BSpline_Compute_Deriv1( curve, curr_basis) );
    }
  else if( FLAG == ( CV_BSPLINE_COMPUTE_DERIV1 | CV_BSPLINE_COMPUTE_DERIV2) )
    {
      return ( icv_BSpline_Compute_Deriv1( curve, curr_basis) | icv_BSpline_Compute_Deriv2( curve, curr_basis) );
    }
  else if( FLAG == ( CV_BSPLINE_COMPUTE_DERIV2 | CV_BSPLINE_COMPUTE_CURVE) )
    {
      return ( icv_BSpline_Compute_Deriv2( curve, curr_basis ) | icv_BSpline_Compute_Curve( curve, curr_basis ));
    }
  else if( FLAG == (  CV_BSPLINE_COMPUTE_DERIV2 | CV_BSPLINE_COMPUTE_CURVE | CV_BSPLINE_COMPUTE_DERIV1) )
    {
      return ( icv_BSpline_Compute_Deriv1(curve, curr_basis)| icv_BSpline_Compute_Deriv2( curve, curr_basis ) | icv_BSpline_Compute_Curve( curve, curr_basis ));
    }
  else
    {
      printf("In cv_BSpline_Compute: Wrong flags passed.\n");
      return(-1);
    }
}

/*
If dest != NULL 
 return the same pointer on success OR NULL on failure
if dest == NULL
 allocate memory and return it on success OR return NULL on failure
*/
Cv_BSpline* cv_BSpline_Copy( Cv_BSpline const *src, Cv_BSpline *dest )
{
  /* Copying all the specific  points */
  if( src == NULL )
    {
      fprintf(stderr, "In cv_BSpline_Copy: NULL src passed \n");
      return NULL;
    }
  else if( dest == NULL )
    {
#ifdef VERBOSE
      fprintf(stderr, "In cv_BSpline_Copy: NULL dest passed. Allocating memory\n");
#endif
      dest = (Cv_BSpline *)malloc(sizeof( Cv_BSpline ));
      if( dest == NULL )
	{
	  fprintf( stderr, "In cv_BSpline_Copy: destination memory allocation failed.\n");
	  return NULL;
	}
      dest->Curve_points = NULL;
      dest->Curve_D1 = NULL;
      dest->Curve_D2 = NULL;
      dest->Curve_Normal = NULL;
      dest->Control_points = NULL;
      dest->Basis = NULL;
    }

  if( dest->Curve_points != NULL )
    cvCopy(src->Curve_points, dest->Curve_points);
  else
     dest->Curve_points = cvCloneMat(src->Curve_points);

  if( dest->Curve_D1 != NULL )
    cvCopy(src->Curve_D1, dest->Curve_D1);
  else
     dest->Curve_D1 = cvCloneMat(src->Curve_D1);

  if( dest->Curve_D2 != NULL )
    cvCopy(src->Curve_D2, dest->Curve_D2);
  else
     dest->Curve_D2 = cvCloneMat(src->Curve_D2);

  if( dest->Curve_Normal != NULL )
    cvCopy(src->Curve_Normal, dest->Curve_Normal);
  else
     dest->Curve_Normal = cvCloneMat(src->Curve_Normal);

  /* Joining pointers for the basis data */
  if( dest->Basis != NULL )
    {
      dest->Basis->Refcount -= 1;
      icv_Basis_Free( dest->Basis);
      if((dest->Basis->Refcount == 0) && (dest->Basis->DAlloc == 1))
	{
	  free( dest->Basis );
	}
      
    }
  dest->Basis = src->Basis;
  src->Basis->Refcount++;
  return dest;
}

int cv_BSpline_Free(Cv_BSpline *curve)
{
  if( curve == NULL )
    return(0);
  else
    {
      if( curve->Control_points != NULL )
	{
	  cvReleaseMat( &(curve->Control_points) );
	  curve->Control_points = NULL;
	}
      if( curve->Curve_D1 != NULL )
	{
	  cvReleaseMat( &(curve->Curve_D1) );
	  curve->Curve_D1 = NULL;
	}
      if( curve->Curve_D2 != NULL )
	{
	  cvReleaseMat( &(curve->Curve_D2) );
	  curve->Curve_D2 = NULL;
	}
      if( curve->Curve_Normal != NULL )
	{
	  cvReleaseMat( &(curve->Curve_Normal) );
	  curve->Curve_Normal = NULL;
	}
      if( curve->Curve_points != NULL )
	{
	  cvReleaseMat( &(curve->Curve_points) );
	  curve->Curve_points = NULL;
	}
      curve->Basis->Refcount -= 1;
      icv_Basis_Free( curve->Basis);
      if((curve->Basis->Refcount == 0) && (curve->Basis->DAlloc == 1))
	{
	  free( curve->Basis );
	}
      curve->Basis = NULL;
    }
  return(0);
}

int icv_Basis_Free(Cv_Basis *basis)
{
  int i;
  if( basis->Refcount == 0 )
    {
      for( i = 0; i< (*basis).N; i++)
	{
	  free(*(basis->Deriv1 + i));
	  free(*(basis->Deriv2 + i));
	  free(*(basis->Basis + i));
	}
      free(basis->Deriv1);
      free(basis->Deriv2);
      free(basis->Basis);
      return(0);
    }
  else
    {
      return(-1);
    }
}


 /* Input can be anything: 3 channel or one channel,image or matrix */
 /* Output is displayed on a 3-Channel 8 bit image */

int cv_BSpline_Image_Save(IplImage const *image, Cv_BSpline const *curve, CvScalar colour, char const *file_name)
{
	IplImage *dummy;
	CvPoint temp1, temp2;

	if( image->nChannels == 3)
		dummy = cvCloneImage( image );
	else
	{
		dummy = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 3);
		cvCvtColor(image, dummy, CV_GRAY2BGR);
	}

	if(dummy == NULL)
	{
		fprintf(stderr,"In cv_BSpline_Image_Save: unable to create temporary image.\n");
		return(-1);
	}
	for( int i = 0; i < curve->Basis->N; i++ )
    {
      temp1.x = (int)cvmGet( curve->Curve_points, i, 0);
      temp1.y = (int)cvmGet( curve->Curve_points, i, 1);
      temp2.x = (int)temp1.x - 1;
      temp2.y = (int)temp1.y - 1;
      cvLine(dummy,temp1,temp2, colour, 1, 8);

      temp1.x = (int)temp1.x + 1;
      temp1.y = (int)temp1.y - 1;
      temp2.x = (int)temp1.x - 1;
      temp2.y = (int)temp1.y + 1;
      cvLine( dummy, temp1, temp2, colour, 1, 8 );
    }
	cvSaveImage( file_name, dummy );

	cvReleaseImage( &dummy );
	return 0;
}
void cv_BSpline_Display(IplImage *image, Cv_BSpline const *curve, CvScalar colour, int Delay)
{
  CvPoint temp2,temp1;
  IplImage *dummy;
  int i;
  CvSize size;
  const char win[]="CvSplineDisplay";

  size = cvGetSize(image);
  dummy = cvCreateImage( size, IPL_DEPTH_8U, 3 );

  if(((IplImage *)image)->nChannels == 1 )
    cvCvtColor((IplImage *)image,dummy,CV_GRAY2BGR);
  else
    cvCopy( image, dummy);

  for( i = 0; i < curve->Basis->N; i++ )
    {
      temp1.x = (int)cvmGet( curve->Curve_points, i, 0);
      temp1.y = (int)cvmGet( curve->Curve_points, i, 1);
      temp2.x = (int)temp1.x - 1;
      temp2.y = (int)temp1.y - 1;
      cvLine(dummy,temp1,temp2,colour,1,8);

      temp1.x = (int)temp1.x + 1;
      temp1.y = (int)temp1.y - 1;
      temp2.x = (int)temp1.x - 1;
      temp2.y = (int)temp1.y + 1;
      cvLine( dummy, temp1, temp2, colour, 1, 8 );
    }
  cvNamedWindow( win, CV_WINDOW_AUTOSIZE);
  cvMoveWindow(win, 100, 100);
  cvShowImage(win, dummy);
  cvWaitKey( Delay );
  cvDestroyWindow( win );
  cvReleaseImage( &dummy );
  return;
}

