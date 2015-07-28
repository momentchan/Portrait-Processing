#include "../include/bspline.h"
/*
Example program for for familiarisation of the user
to the different modes of initialisation of curve
*/

int main(int argc,char **argv)
{
  /*
    METHOD 1:
    Simplest way of initialising a B-Spline curve. If you have all the data stored in a file,
    simply call the function as below with the BSpline curve and file name as arguments. Make sure 
    you pass an allocated data structure for Cv_BSpline. The function will assume
    pre-allocation. All other initialisations are done internally and safely.
  */
  Cv_BSpline *curve1 = NULL;
  if( cv_BSpline_Read_File("control_points.dat", curve1, NULL) != 0)
    printf("First read unsuccessful!\n");
  else
    printf("First read passed.\n");
  /*
    METHOD 1:
    Same with preallocated curve. Just to show the thing works!
  */
  Cv_BSpline curve3 ; 
  if( cv_BSpline_Read_File("control_points.dat", &curve3, NULL) != 0)
    printf("Second read unsuccessful!\n");
  else
    printf("Second read passed.\n");

  /*
    METHOD 2:
    In this example, we explicitly pass a Cv_Basis to the same function. This will be useful if we want
    to explicitly specify the basis. This will be useful in case we want to "share" the same basis data-structure 
    amongst different splines. Do not use if not comfortable.
   */
  Cv_BSpline curve2;
  Cv_Basis basis;
  basis.DAlloc = 0; /* VERY IMPORTANT; MAY LEAD TO MEMORY LEAKS/SEG-FAULTS */
  basis.Refcount = 0; /* VERY IMPORTANT; MAY LEAD TO MEMORY LEAKS/SEG-FAULTS */
  if( cv_BSpline_Read_File("control_points.dat", &curve2, &basis) != 0)
    printf("Third read failed!\n");
  else
    printf("Third read successful.\n");

  /*
    METHOD 3:
    Copy from an existing spline
  */
  if( (curve1 = cv_BSpline_Copy( &curve2, curve1 )) != NULL )
    printf("Copy successful!\n");
  if( (cv_BSpline_Copy( &curve2, &curve3 )) != NULL )
    printf("Copy successful!\n");

  /* 
     METHOD 4:
     Suppose you have a set of points. You want to fit an *approximating*(NOT interpolating) spline over these points.
     This is useful for manual initialisation or the point set obtained from other pre-processing step.
     Following function returns an curve which does the above. Note that the input points are expected in CvMat(N,2,CV_32FC1) format, 
     where N is the number of input points. The curve will NOT pass through the input points. Here we see the example of basis being 
     re-used in the last argument.
     If passed as NULL, a new basis will be internally allocated but we want to "share" the underlying basis function. For the same 
     class("class" not meant in OOP sense) of splines, it makes sense to share the basis.
  */
  if( cv_BSpline_Approx( curve2.Curve_points, curve1, CV_BSPLINE_CLOSED_CURVE, curve2.Basis) == 0 )
    {
      /* printf("Approximation successful. Estimated control points:\n"); */
      /* printf("\nApproximated control points.\n"); */
      /* for( int i = 0; i < curve1->Basis->N_b; i++ ) */
      /* 	printf("%lf %lf\n", cvmGet(curve1->Control_points, i, 0),cvmGet(curve1->Control_points, i, 1)); */

      /* printf("\nOriginal control points:\n"); */
      /* for(int i = 0; i < curve1->Basis->N_b; i++ ) */
      /* 	printf("%lf %lf\n", cvmGet(curve2.Control_points, i, 0),cvmGet(curve2.Control_points, i, 1)); */
    }
  else
    {
      fprintf( stderr, "Approximation unsuccessful.\n");
    }

  /*
    Method 5:
    Initialising from an image by user specification of points
  */
  cv_BSpline_Free( &curve2 );
  IplImage *im;
  if( argc == 1)
     im = cvLoadImage("pattern.pgm", CV_LOAD_IMAGE_GRAYSCALE);
  else
    im = cvLoadImage( argv[1], CV_LOAD_IMAGE_GRAYSCALE);

  if( im != NULL )
    {
      cv_BSpline_Manual_Init( im,  &curve2 ); /* ALWAYS pass a clean Cv_BSpline to this function */
      cv_BSpline_Display( im, &curve2, CV_RGB(200, 50, 1), -1);
    }
  else
    fprintf(stderr, "Image not available.\n");
  
  /* Freeing up finally */
  cvReleaseImage(&im);
  cv_BSpline_Free( &curve2 );
  cv_BSpline_Free( curve1 );
  cv_BSpline_Free( &curve3 );
  free(curve1);
  return 0;
}
