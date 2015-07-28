#include "bspline.h"
/*
Set TANGENT_TERM_FLAG=1 to enable tangent term.
 */
int cv_Curve_Evolve( Cv_BSpline *curve, double const * ( *speed_fn )(void *), void *speed_fn_params, int Iter, double Time_Step, char TANGENT_TERM_FLAG, double TANGENT_TERM_WEIGHT, CvMat *F_Tangent, double *Alpha )
{
  int j,i, NBasis = curve->Basis->N_b, Order = curve->Basis->K;
  CvMat *N, *M, *P, *F_Total;
  CvMat *A;
  double Nx, Ny, beta_test;
  double const *beta_normal;
  /* ****************************************************************** */
  /*                    Initialising various matrices                   */
  /* ****************************************************************** */
  /* Matrix for computing the new B-Spline control point locations */
  N = cvCreateMat( curve->Basis->N, NBasis, CV_64FC1);
  M = cvCreateMat( NBasis, NBasis, CV_64FC1);
  /* ************************************************************************* */
  /*            Creating the RHS: Force which moves the points                 */
  /* ************************************************************************* */
  /* ******************************************* */
  /*       Initialising the LHS of the Matrix    */
  /* ******************************************* */
  /* Code for closed curve */
  if( curve->Basis->Type == CV_BSPLINE_CLOSED_CURVE )
    {
      for(i = 0; i < curve->Basis->N; i++ )
	{
	  for( j = 0; j < NBasis; j++ )
	    {
	      if ( j <= Order - 2 )
		cvmSet(N,i,j,curve->Basis->Basis[i][j+1]+curve->Basis->Basis[i][NBasis+j+1]);
	      else
		cvmSet( N, i, j, curve->Basis->Basis[i][j+1]);
	    }
	}
    }
  else if( curve->Basis->Type == CV_BSPLINE_OPEN_CURVE )
    {
      for( i = 0; i < curve->Basis->N; i++ )
	for( j = 0;j < NBasis; j++ )
	  cvmSet( N, i, j, curve->Basis->Basis[i][j+1]);
    }
  else
    {
      fprintf( stderr, "Curve type undefined.\n");
      fflush(stderr);
      return(-1);
    }

  /* ********************** */
  /*    LHS is now READY    */
  /* ********************** */
  P = cvCreateMat( NBasis, NBasis, CV_64FC1 );
  A = cvCreateMat( NBasis, curve->Basis->N, CV_64FC1 );
  /* M = (N^T * N) */
  cvMulTransposed( N, M, 1, NULL );
  /* inv(M) */
  cvInvert( M, P, CV_LU );
  /* A = (N^T*N)^(-1)*N^T, aka pseudo-inverse, maybe better ways of computing? */
  /* substitute this by cvInverse(N) and see */
  cvGEMM( P, N, 1.0, NULL, 0.0, A, CV_GEMM_B_T );
  cvReleaseMat( &M );
  cvReleaseMat( &P );
  cvReleaseMat( &N );
  
  /* *********************************************** */
  /*  Finding the change in the control points       */
  /* *********************************************** */
  /* Calculating the motion first */
  if( TANGENT_TERM_FLAG == 1 )
    {
      fprintf(stderr, "Tangent term used.\n");
      F_Total = cvCreateMat( curve->Basis->N, 2, CV_64FC1 );
      cvSetZero(F_Total);
    }
  else
    {
      fprintf(stderr, "Tangent term not used.\n");
      P = cvCreateMat( curve->Basis->N, 2, CV_64FC1);
    }

  N = cvCreateMat( NBasis, 2, CV_64FC1);

  for( i = 1; i <= Iter; i++ )
    {
      /* Returns the displacement in the matrix P */
      /* the return value is *NOT* supposed to release the displacement */
      beta_normal = speed_fn(speed_fn_params);
      /* If tangent term is required.... */
      if( TANGENT_TERM_FLAG == 1 )
	{
#ifdef VERBOSE
	  printf("Calculating tangent term.\n");
#endif
	  cv_GenerateTangentTerm( curve, beta_normal, TANGENT_TERM_WEIGHT, F_Total, F_Tangent, Alpha);
	  // Massive debugging here... checking for change in beta due to tangent term
	  // for ( int n = 0; n < curve->Basis->N; n++)
	  //   {
	  //     beta_test = cvmGet(F_Total, n, 0)*cvmGet(curve->Curve_Normal, n, 0)+ cvmGet(F_Total, n, 1)*cvmGet(curve->Curve_Normal, n, 1);
	  //     printf("%.8f %.8f\n", beta_normal[n], beta_test);
	  //   }

	  cvCvtScale( F_Total, F_Total, Time_Step, 0.0);
	  cvGEMM( A, F_Total, 1.0, NULL, 1.0, N, 0 );
	  cvAdd( N, curve->Control_points, curve->Control_points, NULL);
	}
      else
	{
	  for( int n = 0; n < curve->Basis->N; n++ )
	    {
	      Nx = cvmGet(curve->Curve_Normal, n, 0);
	      Ny = cvmGet(curve->Curve_Normal, n, 1);

	      /* Extracting only the Normal component of the speed */ 
	      /* cvmSet( P, n, 0, beta_normal[n]*Nx ); */
	      /* cvmSet( P, n, 1, beta_normal[n]*Ny ); */

	      cvmSet( P, n, 0, beta_normal[n]*Nx );
	      cvmSet( P, n, 1, beta_normal[n]*Ny );

	    }
	  cvCvtScale( P, P, Time_Step, 0.0);
	  cvGEMM( A, P, 1.0, NULL, 1.0, N, 0 );
	  cvAdd( N, curve->Control_points, curve->Control_points, NULL);

	}

      if( cv_BSpline_Compute(curve, (CV_BSPLINE_COMPUTE_CURVE|CV_BSPLINE_COMPUTE_DERIV1|CV_BSPLINE_COMPUTE_DERIV2), NULL) != 0 )
	{
	  printf("In cv_Curve_Evolve: Error in computing curve or derivatives.\n");
	  cvReleaseMat( &A );
	  if( TANGENT_TERM_FLAG == 1 )
	    cvReleaseMat( &F_Total );
	  return(-1);
	}
      if( icv_BSpline_Compute_Normal( curve ) != 0 )
	{
	  fprintf( stderr, "In cv_BSpline_Read_File: Unable to compute curve normal.\n");
	}

    }
  if( TANGENT_TERM_FLAG == 0 )
    cvReleaseMat( &P );
  cvReleaseMat( &A );
  cvReleaseMat(&N);
  if( TANGENT_TERM_FLAG == 1 )
    cvReleaseMat( &F_Total );

  return 0;
}
