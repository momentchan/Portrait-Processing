/* This is the C implementation of the Gradient Vector Force Snake proposed by Prince */
/* This has been "translated" from MATLAB code downloaded from the website */

/* The input to the code is the edge map,number of iterations and the weight value */
/* The output is the Gradient Vector Force defined over the same grid */
/* Output is a 64bit matrix allocated BEFORE */

#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <bspline.h>

void computeGVF(IplImage const *image, double mu, int ITER, CvMat *u, CvMat *v)
{
	CvMat *input_M,*u_M,*v_M,*u_Laplace,*v_Laplace,*fx,*fy,*mag,*temp;
	float *src,*dest,temp1,val1,val2;
	int i,j,iter;

	CvMat *gvf_image = cvCreateMat( image->height, image->width, CV_32FC1 );
	CvMat *blurred = cvCreateMat( image->height, image->width, CV_32FC1 );
	cvConvert( image, gvf_image);
	cvNormalize( gvf_image, gvf_image, 0.0, 1.0, CV_MINMAX );
	cvSmooth( gvf_image, blurred, CV_GAUSSIAN, 3, 3, 0 );
	// Reusing u & v
	cvSobel( blurred, u, 1, 0, 1);
	cvSobel( blurred, v, 0, 1, 1);
	cvPow( u, u, 2.0 );
	cvPow( v, v, 2.0 );
	cvAdd( v, u, blurred, NULL );
	cvPow( blurred, blurred, 0.5 );

  //  CvSize size;
  printf("Entering GVF function.\n");
  fflush(stdout);

  cvNormalize( blurred, blurred, 0.0, 1.0, CV_MINMAX); 
  /* cv::normalize(input, input, 1.0, 0.0, ); */
  input_M = cvCreateMat( blurred->rows + 2, blurred->cols + 2,CV_32FC1);

  u_M = cvCreateMat( blurred->rows + 2, blurred->cols + 2,CV_32FC1);
  v_M = cvCreateMat( blurred->rows + 2, blurred->cols + 2,CV_32FC1);

  temp = cvCreateMat( blurred->rows + 2, blurred->cols + 2,CV_32FC1);

  u_Laplace = cvCreateMat( blurred->rows + 2, blurred->cols + 2, CV_32FC1);
  v_Laplace = cvCreateMat( blurred->rows + 2, blurred->cols + 2, CV_32FC1);

  mag = cvCreateMat( blurred->rows + 2, blurred->cols + 2, CV_32FC1);

  /* **************************************** */
  /*      Mirroring the edge conditions       */
  /* **************************************** */
  src = blurred->data.fl;
  dest = input_M->data.fl + input_M->cols + 1;

  for( i = 0; i < blurred->rows;i++)
    {
      for( j = 0; j < blurred->cols;j++)
	{
	  *(dest + i*(input_M->cols) + j) = *(src + i*blurred->cols + j);
	}
    }

  /* ********************** */
  /*   Copying the cols     */
  /* ********************* */

  for(i=0;i<input_M->rows;i++)
    {
      cvmSet(input_M,i,0,cvmGet(input_M,i,2));
      cvmSet(input_M,i,input_M->cols-1,cvmGet(input_M,i,input_M->cols-3));
    }

  /* ********************** */
  /*   Copying the rows     */
  /* ********************* */

  for(i=0;i<input_M->cols;i++)
    {
      cvmSet(input_M,0,i,cvmGet(input_M,2,i));
      cvmSet(input_M,input_M->rows-1,i,cvmGet(input_M,input_M->rows-3,i));
    }

  fx = cvCreateMat(input_M->rows,input_M->cols,CV_32FC1);
  fy = cvCreateMat(input_M->rows,input_M->cols,CV_32FC1);

  cvSobel( input_M, fx, 1, 0, 1);
  cvSobel( input_M, fy, 0, 1, 1);

  cvCopy( fx, u_M );
  cvCopy( fy, v_M);

  cvReleaseMat(&input_M);

  cvPow(fx,u_Laplace,2.0);
  cvPow(fy,v_Laplace,2.0);
  cvAdd(u_Laplace,v_Laplace,mag,NULL);

  cvPow( mag, mag, 0.5 );

  src = u_M->data.fl;
  dest = v_M->data.fl;

  for(iter=0;iter<ITER;iter++)
    {

      /* ********************** */
      /*   Copying the rows     */
      /* ********************** */

      for(i=0;i<u_M->cols;i++)
	{
	  *(dest + i)=*(dest + 2*v_M->cols + i);
	  *(src + i)=*(src + 2*u_M->cols + i);

	  *(src + (u_M->rows - 1)*u_M->cols + i)=*(src + (u_M->rows -3)*u_M->cols + i);
	  *(dest + (u_M->rows - 1)*u_M->cols + i)=*(dest + (u_M->rows -3)*u_M->cols + i);
	}
      
      /* ********************** */
      /*   Copying the cols     */
      /* ********************** */

      for(i=0;i<u_M->rows;i++)
	{
	  *(dest + i*u_M->cols)=*(dest + i*u_M->cols + 2);
	  *(src + i*u_M->cols)=*(src + i*u_M->cols + 2);

	  *(dest + i*u_M->cols + u_M->cols -1)=*(dest + i*u_M->cols + u_M->cols - 3);
	  *(src + i*u_M->cols + u_M->cols -1)=*(src + i*u_M->cols + u_M->cols - 3);
	}

      /* ***************************** */
      /* Calculating the Laplacian     */
      /* ***************************** */ 
      cvLaplace(u_M,u_Laplace,1);
      cvLaplace(v_M,v_Laplace,1);
      
      cvSub(u_M,fx,temp,NULL);
      cvMul(mag,temp,temp,1.0);
      cvAddWeighted(u_M,1.0,u_Laplace,mu,0.0,u_M);
      cvSub(u_M,temp,u_M,NULL);

      cvSub(v_M,fy,temp,NULL);
      cvMul(mag,temp,temp,1.0);
      cvAddWeighted(v_M,1.0,v_Laplace,mu,0.0,v_M);
      cvSub(v_M,temp,v_M,NULL);
    }

  /* ********************************************* */
  /*            Unmirroring the Matrix             */
  /* ********************************************* */

  
  for(i=0;i<u->rows;i++)
    {
      for(j=0;j<u->cols;j++)
	{
	  src = u_M->data.fl;
	  dest = u->data.fl;
	  *(dest + i*u->cols + j) = *(src + (i+1)*u_M->cols + j+1);

	  src = v_M->data.fl;
	  dest = v->data.fl;
	  *(dest + i*u->cols + j) = *(src + (i+1)*u_M->cols + j+1);
	}
    }

  src = u->data.fl;
  dest = v->data.fl;


  for(i=0;i<u->rows;i++)
    {
      for(j=0;j<u->cols;j++)
	{
	  val1 = *(src + i*u->cols + j);
	  val2 = *(dest + i*u->cols + j);
	  temp1 = sqrt(val1*val1 +val2*val2);
	  *(src + i*u->cols + j) /=(float)(temp1 + .0000001);
	  *(dest + i*u->cols + j)/=(float)(temp1 + .0000001);
	  //	  printf("%0.3f,%0.3f\n",*(src + i*u->cols + j),*(dest + i*u->cols + j));
	}
    }


  cvReleaseMat(&u_Laplace);
  cvReleaseMat(&v_Laplace);
  cvReleaseMat(&u_M);
  cvReleaseMat(&v_M);
  cvReleaseMat(&mag);
  cvReleaseMat(&fx);
  cvReleaseMat(&fy);
  cvReleaseMat(&temp);
  cvReleaseMat( &blurred );
  cvReleaseMat( &gvf_image );
  
  printf("Leaving GVF Function.\n");
  fflush(stdout);

  return;
}

// Geodesic Active contour model of Casseles et al.

int computeGAC(IplImage const *image, CvMat *gradient_mag, CvMat *grad_grad_mag)
{
/* 
	The motion field for the GAC model is
	C_t = ( g*kappa - ( grad g * N ) )N
	g = gradient_mag
	grad g = grad_grad_mag
*/
	/* Smooth image first */
	CvMat *blurred = cvCreateMat( image->height, image->width, CV_32FC1 );
	CvMat *u,*v;
	u = cvCloneMat(blurred);
	v = cvCloneMat(blurred);

	if( image->nChannels == 1 )
	{
		cvConvert(image, blurred);
		printf("Single channel image.\n"); fflush(stdout);
	}
	else
	{
		IplImage *im = cvCreateImage( cvGetSize( image ), IPL_DEPTH_8U, 1);
		cvCvtColor(image, im, CV_BGR2GRAY);
		cvConvert(im, blurred);
		cvReleaseImage(&im);
		printf("Coloured image.\n"); fflush(stdout);
	}
	cvNormalize( blurred, blurred, 0.0, 1.0, CV_MINMAX, NULL);
	//cvSmooth( blurred, blurred, CV_GAUSSIAN, 5);
	
	cvSobel(blurred, u, 1, 0, 3);
	cvSobel(blurred, v, 0, 1, 3);
	cvPow( u, u, 2.0 );
	cvPow( v, v, 2.0 );
	cvAdd(u, v, gradient_mag); // Ix^2+Iy^2; n=2
	//cvNormalize( gradient_mag, gradient_mag, 0.0, 1.0, CV_MINMAX, NULL);
	//cvPow(gradient_mag, gradient_mag, 0.5);// n=1
	cvAddS( gradient_mag, cvScalarAll(1.0), gradient_mag, NULL);
	cvPow(gradient_mag, gradient_mag, -1.0);
	// Calculating the gradient of the magnitude
	cvSobel( gradient_mag, u, 1, 0, 1);
	cvSobel( gradient_mag, v, 0, 1, 1);
	CvScalar val;

	for( int i = 0; i < u->rows; i++ )
		for( int j = 0; j< u->cols; j++ )
		{
			val.val[0] = cvmGet(u, i, j);
			val.val[1] = cvmGet(v, i, j);
			cvSet2D(grad_grad_mag, i, j, val);
		}

		cvReleaseMat(&u);
		cvReleaseMat(&v);
		cvReleaseMat(&blurred);

	return(0);
}
