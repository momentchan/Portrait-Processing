/* 
   This file has examples for usage of the library. In this file, we show two examples of gradient based segmentation.
   These are the GVF(Prince et al) and GAC (Casselles et al) model.
   Parameters are tuned for the particular images. The user generally will have to change as per image.

*/
#include "../include/bspline.h"
// By default GVF method is run. Comment the next line to run the GAC model.
//#define SEG_GVF

/* Two examples of usage for gradient based function */
void computeGVF(IplImage const *input, double mu, int ITER, CvMat *u, CvMat *v);  // Gradient Vector Force Model: Prince et al
int computeGAC(IplImage const *image, CvMat *gradient_mag, CvMat *grad_grad_mag); // Geodesic Activee Contour model: Casselles et al


/* Data structure for the speed function; this is similar to the  */
struct Args
{
  Cv_BSpline *curve;
  double *curvature;
  double *beta_normal;
  CvMat *U,*V;
};

/* Geodesic active contour calculator */
double const *gacSpeedFunction( void *Args)
{
  struct Args *dummy = (struct Args *) Args;
  Cv_BSpline *curve = (Cv_BSpline *)(dummy->curve);
  double *curvature = (double *)(dummy->curvature), WEIGHT=0.000001;
  double *beta_normal = (double *)(dummy->beta_normal); 
  CvMat *edge_indicator = (CvMat *)(dummy->U);
  CvMat *grad_edge_indicator = (CvMat *)(dummy->V);
  int i,j,n;
  CvScalar val;
  
  cv_BSpline_Curvature( curve, curvature );
  for( n = 0; n < curve->Basis->N; n++ )
    {
      i = (int) cvmGet(curve->Curve_points, n, 1);
      j = (int) cvmGet(curve->Curve_points, n, 0);
      
      val = cvGet2D( grad_edge_indicator, i, j);
      beta_normal[n] = (double)(WEIGHT*cvmGet(edge_indicator, i,j)*curvature[n] - (val.val[0]*cvmGet(curve->Curve_Normal, n, 0) + val.val[1]*cvmGet(curve->Curve_Normal, n, 1)));
    }
  
  return beta_normal;
}

double const *gvfSpeedFunction(void *Arg)
{
  struct Args *dummy = (struct Args *) Arg;
  Cv_BSpline *curve = (Cv_BSpline *)(dummy->curve);
  double *curvature = (double *)(dummy->curvature);
  double *beta_normal = (double *)(dummy->beta_normal); 
  CvMat *u = (CvMat *)(dummy->U);
  CvMat *v = (CvMat *)(dummy->V);
  int i,j;
  
  
  cv_BSpline_Curvature( curve, curvature );  
  for( int n = 0; n < curve->Basis->N; n++ )
    {
      /* Getting the force from GVF motion field */
      i = (int) cvmGet(curve->Curve_points, n, 1);
      j = (int) cvmGet(curve->Curve_points, n, 0);
      beta_normal[n]=(double)(cvmGet(u,i,j)*cvmGet(curve->Curve_Normal,n,0)+cvmGet(v,i,j)*cvmGet(curve->Curve_Normal,n,1));
      /* Adding curvature term */
      beta_normal[n] += (double)(0.0001*curvature[n]);
      
    }
  return beta_normal;
}

int main(int argc, char **argv)
{
  Cv_BSpline curve;
  int Iter = 20000;
  double Time_Step = 0.1;
  char TANGENT_TERM_FLAG = 1;
  CvMat *F_tangent = NULL;
  double *alpha = NULL;
  struct Args speedArgs;
  IplImage *image;
  double TANGENT_TERM_WT=0.0001;
  char const *input_file = "control_points.dat", *output_file="output_control_points.dat";
  char const *output_image_file="output.png", *input_image_file="pattern.pgm";

  if( ( argc == 2) || (argc == 3) )
    image  = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
  else
    {
      printf("Usage: %s image_file(pattern.pgm) (Iter=5000)\n", argv[0]);
      image  = cvLoadImage( input_image_file, CV_LOAD_IMAGE_GRAYSCALE);
    }

  if( image == NULL )
    {
      fprintf(stderr, "Image unavailable. Exiting.\n");
	  getchar();
      return(-1);
    }
  /* Reading from file */
  if( cv_BSpline_Read_File(input_file, &curve, NULL) != 0)
    {
      printf("Read unsuccessful!\n");
      return(-1);
    }
  printf("Initial curve displayed for 2.5 seconds. Enter any key to continue.\n"); fflush(stdout); 
  cv_BSpline_Display( image, &curve, CV_RGB(255, 0, 0), 2500);
  
  // Uncomment below if you want to enter the tangent term manually
  /*
    printf("Tangent term weight: ");
    scanf("%f", &TANGENT_TERM_WT);
  */


  // Uncomment the next line if you want to initialise the curve manually instead of reading from a file  
  /* cv_BSpline_Manual_Init( image,  &curve ); /\* ALWAYS pass a clean Cv_BSpline to this function *\/ */


  printf("Initial control points:\n"); fflush(stdout);
  for( int i = 0; i < curve.Basis->N_b; i++)
    printf("%lf %lf\n", cvmGet(curve.Control_points, i, 0), cvmGet(curve.Control_points, i, 1));

  if( argc == 3 )
    Iter = atoi( argv[2] );

  /* initialise datastructures common to both speed functions */
  speedArgs.beta_normal = (double *)malloc( sizeof(double)*curve.Basis->N);
  speedArgs.curvature = (double *) malloc(curve.Basis->N*sizeof(double));
  speedArgs.curve = &curve;

  
#ifdef SEG_GVF
  /* GVF motion field calculation */
  printf("Gradient vector force model used for segmentation.\n");
  speedArgs.U = cvCreateMat( image->height, image->width, CV_32FC1 );
  speedArgs.V = cvCreateMat( image->height, image->width, CV_32FC1 );
  computeGVF( image, 0.1, 350, speedArgs.U, speedArgs.V);
  if( cv_Curve_Evolve( &curve, gvfSpeedFunction, &speedArgs, Iter, Time_Step, TANGENT_TERM_FLAG, TANGENT_TERM_WT, F_tangent, alpha) != 0 )
    printf("Evolution not successful.\n");
  else
    {
      printf("Writing final control points to %s\n", output_file);
      cv_BSpline_Write_File(output_file, &curve);
      printf("Output image file written to: %s\n", output_image_file);
      cv_BSpline_Image_Save(image, &curve, CV_RGB(250, 250, 2), output_image_file);
      printf("Displaying final image.Press any key to quit.\n");
      cv_BSpline_Display( image, &curve, CV_RGB(0, 255, 0), -1); 

    }
#else
  /* Geodesic Active Contours motion field calculation */
  printf("Geodesic active contour model used for segmentation.\n");
  speedArgs.U = cvCreateMat( image->height, image->width, CV_32FC1 );
  speedArgs.V = cvCreateMat( image->height, image->width, CV_32FC2 );
  computeGAC(image, speedArgs.U, speedArgs.V);

  if( cv_Curve_Evolve( &curve, gacSpeedFunction, &speedArgs, Iter, Time_Step, TANGENT_TERM_FLAG, TANGENT_TERM_WT, F_tangent, alpha) != 0 )
    printf("Evolution not successful.\n");
  else
    {
      printf("Writing final control points to: %s\n", output_file);
      cv_BSpline_Write_File(output_file, &curve);
      printf("Output image file written to: %s\n", output_image_file);
      cv_BSpline_Image_Save(image, &curve, CV_RGB(250, 250, 2), output_image_file);
      printf("Displaying final image.Press any key to quit.\n");
      cv_BSpline_Display( image, &curve, CV_RGB(0, 255, 0), -1); 
    }
#endif
  
  cvReleaseImage( &image );
  free(speedArgs.beta_normal);
  free(speedArgs.curvature);
  cv_BSpline_Free(&curve);
  cvReleaseImage(&image);
  cvReleaseMat( &speedArgs.U);
  cvReleaseMat( &speedArgs.V);
  return 0;
}
