#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>


#define CV_BSPLINE_OPEN_CURVE 0
#define CV_BSPLINE_CLOSED_CURVE 1

#define CV_BSPLINE_COMPUTE_CURVE 0x1
#define CV_BSPLINE_COMPUTE_DERIV1 0x2
#define CV_BSPLINE_COMPUTE_DERIV2 0x4
#define CV_BSPLINE_COMPUTE_NORMAL 0x8
/* #define CV_BSPLINE_COMPUTE_ALL 6 */
/*
Separate out the basis and the curve data-structures.
This way, same basis can be used for multiple curves
*/
struct Cv_Basis
{
  int    N, K, N_b, Type, Refcount; /* 0->Open Curve;1->closed curve */
  double  **Deriv1, **Deriv2, **Basis;
  char   DAlloc ;
} ;
struct Cv_BSpline
{
  CvMat    *Control_points;  /* Pointer for storage of control points */
  Cv_Basis *Basis; /* This holds the data for the basis, first & sec. derivs */
  CvMat   *Curve_points, *Curve_D1, *Curve_D2, *Curve_Normal; /* Curve points are stored here */
};

/*
Initialisation functions
Currently, there are three ways to initialise a curve.
Initialising a curve is consists of defining the control points.
This is obtained either from ...
*/

/* a) Specifying it manually; not preferred unless you know what you are doing */
/* b) Reading from a file */
int cv_BSpline_Read_File( char const *file, Cv_BSpline *curve, Cv_Basis *basis);/* OK */
int cv_BSpline_Write_File( char const *file_name, Cv_BSpline const *curve);
/* c) Copying from another previously initialised curve */
Cv_BSpline * cv_BSpline_Copy(Cv_BSpline const *src, Cv_BSpline *dest);/* OK */
/* d) Estimating the control points by fitting an approximation curve to a set of curve points */
/* if null given, internal allocation */
int cv_BSpline_Approx(CvMat const *points, Cv_BSpline *curve, int Type, Cv_Basis *basis); /* OK */

/* Internal functions. User should not call these directly by himself unless ... */
int icv_InitBasis( Cv_Basis *basis ); /* OK */
int icv_Basis_Free(Cv_Basis *basis); /* OK */
int icv_Periodic_LS(CvMat const *input, Cv_BSpline *curve);

double cv_BSpline_Length(Cv_BSpline const *curve);/* OK */
int  cv_BSpline_Compute( Cv_BSpline *curve, int flag, Cv_Basis const *basis );
int  cv_BSpline_Free(Cv_BSpline *curve);
int cv_Curve_Evolve( Cv_BSpline *curve, double const *(*speed_fn)(void *), void * speedArgs, int Iter, double Time_Step, char TANGENT_TERM_FLAG, double TANGENT_TERM_WT, CvMat * F_Tangent, double *alpha);
void cv_BSpline_Curvature(Cv_BSpline const *curve, double *curvature);
void cv_BSpline_Basis(int c, double t,int npts,double *x,double *n);
void cv_BSpline_Param(int dpts,CvMat const *input, double *tparm);
void cv_BSpline_DBasis(int c,double t,int npts,double *x,double *n,double *d1,double *d2);
int cv_GenerateTangentTerm(Cv_BSpline const *Curve, double const *beta_normal, double Scale,  CvMat *F_Total, CvMat *F_Tangent, double *Alpha);
int icv_BSpline_Compute_Curve( Cv_BSpline *curve, Cv_Basis *basis );
int icv_BSpline_Compute_Deriv1( Cv_BSpline *curve, Cv_Basis *basis );
int icv_BSpline_Compute_Deriv2( Cv_BSpline *curve, Cv_Basis *basis );
int icv_BSpline_Compute_Normal( Cv_BSpline *curve);
void cv_BSpline_Display(IplImage *image, Cv_BSpline const *curve, CvScalar colour, int Delay);
int icv_BSpline_Compute_Normal(Cv_BSpline *curve, Cv_Basis const *basis);
int cv_BSpline_Manual_Init( IplImage *im, Cv_BSpline *input );
CvMat *ginput( IplImage const &colour );
int cv_BSpline_Image_Save(IplImage const *image, Cv_BSpline const *curve, CvScalar colour, char const *file_name);
