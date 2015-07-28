#include "bspline.h"
//#include "../include/bspline.h"

/* 
   This function returns the tangential force moving the curve 
consisting of sum of tangent and normal forces.
   The user passes his force as the input; it could include tangential 
components also.
   This function will resolve the input force into normal and tangent 
components. It will recalculate
   the tangential component, denoted by alpha, based on the normal 
component, denoted by beta.
   The function expects an already allocated output matrix. The return 
value is Zero on success or non-zero on failure.
   The magnitude of the force is stored in alpha and the tangential 
component ONLY( i.e. alpha*T(i) ) is returned in 
   the matrix F_tangent. The total force alpha*T + beta*N is returned 
in F_Total. alpha & F_Tangent can be set to NULL if 
   not required.
*/
/*
ASSUMING THAT alpha AND F_Tangent ARE PREVIOUSLY ALLOCATED 
*/

int cv_GenerateTangentTerm(Cv_BSpline const *Curve, double const *beta_normal, double Scale, CvMat *F_Total, CvMat *F_Tangent, double *Alpha)
{
  int i;
  double kappa,F_Tangent_X,F_Tangent_Y;
  double Ty, Tx, term1, term2, g, F_total_X, F_total_Y, Speed;
  double Ny, Nx, alpha_curr=0.0;
  double d1_x, d1_y, d2_x, d2_y;

  if( Curve->Basis->Type == CV_BSPLINE_CLOSED_CURVE )
    {
      Speed = 0.0;
      for( i = 1; i<Curve->Basis->N; i++)
	  {
		  d1_x = (double)cvmGet(Curve->Curve_D1, i-1, 0);
		  d1_y = (double)cvmGet(Curve->Curve_D1, i-1, 1);
		  g = (double)hypot( d1_x, d1_y);
		  Speed += g;
	  }
	  Speed /= Curve->Basis->N;
	  
	  /* Solving an ODE with periodic boundary conditions; initialising to Zero. */
      Nx = (double) cvmGet(Curve->Curve_Normal, 0, 0);
      Ny = (double) cvmGet(Curve->Curve_Normal, 0, 1);
      
      d2_x = (double)cvmGet(Curve->Curve_D2, 0, 0);
      d2_y = (double)cvmGet(Curve->Curve_D2, 0, 1);
      
      Tx = Ny;
      Ty = -Nx;
      /* No tangential term as alpha[0] = 0 */
      cvmSet( F_Total, 0, 0, beta_normal[0]*Nx );
      cvmSet( F_Total, 0, 1, beta_normal[0]*Ny );
      
      if( F_Tangent != NULL )
	  {
		  cvmSet( F_Tangent, 0, 0, 0.0 );
		  cvmSet( F_Tangent, 0, 1, 0.0 );
	  }

	  if( Alpha != NULL )
		  Alpha[0] = 0.0;
      
      for( i = 1; i < Curve->Basis->N; i++)
	  {
		  d1_x = (double)cvmGet(Curve->Curve_D1, i-1, 0);
		  d1_y = (double)cvmGet(Curve->Curve_D1, i-1, 1);
	  
		  d2_x = (double)cvmGet(Curve->Curve_D2, i-1, 0);
		  d2_y = (double)cvmGet(Curve->Curve_D2, i-1, 1);
	  
		  Nx = (double)cvmGet(Curve->Curve_Normal, i-1, 0);
		  Ny = (double)cvmGet(Curve->Curve_Normal, i-1, 1);
	  
		  Ty = -Nx; 
		  Tx = Ny; 
		  /* Calculating tangent term alpha; see the TIP paper mentioned in README for an explanation of the eqn */
		  g = (double)hypot( d1_x, d1_y);
		  //	  Speed_per_iter += g;
		  kappa = (d1_x*d2_y - d1_y*d2_x)/pow((double)(d1_x*d1_x + d1_y*d1_y), (double)1.5);
		  term1 = Speed - g;
		  term2 = g * kappa * beta_normal[i-1];
		  alpha_curr += (term1 + term2); 
	  
		  /* Total force = alpha*T + beta_normal*N */
		  F_total_X = Scale * alpha_curr * Tx + beta_normal[i] * Nx;
		  F_total_Y = Scale * alpha_curr * Ty + beta_normal[i] * Ny;
	  
		  if( Alpha != NULL )
			  Alpha[i] = alpha_curr;
		  if( F_Tangent != NULL )
		  {
			  F_Tangent_X = Scale * alpha_curr * Tx;
			  F_Tangent_Y = Scale * alpha_curr * Ty;
	      
			  cvmSet( F_Tangent, i, 0, F_Tangent_X );
			  cvmSet( F_Tangent, i, 1, F_Tangent_Y );
		  }
	  
		  cvmSet( F_Total, i, 0, F_total_X);
		  cvmSet( F_Total, i, 1, F_total_Y);
	  }
      //      Speed_per_iter /= Curve->Basis->N;
      return(0);
    }
  else if( Curve->Basis->Type == CV_BSPLINE_OPEN_CURVE )
    {
      printf("Not yet implemented.\n");
      return(-1);
    }
  else
    {
      fprintf(stderr,"Curve type not set.\n");
      return(-1);
    }
}
