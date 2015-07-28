//#include <bspline.h>
#include "bspline.h"
/*

Returns N points picked by the left mouse click.
Right click to end

Input Parameters:
colour: the image itself
window: window name
number of points

Output:
points:

It is assumed that the input pointer is INITIALISED.
*/

static int count=0;
CvMat *cv_bspline_dummy_points_xxx;
IplImage *cv_bspline_test_image;

void mouse_handler(int event,int x, int y, int flags, void *dud)
{
  CvScalar colour=CV_RGB(250,8,5);
  CvPoint temp1,temp2;

  if(event==CV_EVENT_LBUTTONDOWN)
    {
      cvmSet( cv_bspline_dummy_points_xxx, count, 0, x);
      cvmSet( cv_bspline_dummy_points_xxx, count, 1, y);
      count++;
      /* Drawing Tiny lines around the input point */
      temp1.x=x-3;
      temp1.y=y-3;
      temp2.x=x+3;
      temp2.y=y+3;
      cvLine(cv_bspline_test_image, temp1, temp2, colour, 2, 8);	
      temp1.x=x+3;
      temp1.y=y-3;
      temp2.x=x-3;
      temp2.y=y+3;
      cvLine(cv_bspline_test_image, temp1, temp2, colour, 2, 8);	
      cvShowImage( "GInput", cv_bspline_test_image );
      printf("%d of %d points entered.\r", count, cv_bspline_dummy_points_xxx->rows); fflush(stdout);
    }

  if ( event == CV_EVENT_RBUTTONDOWN )
    {
      return;
    }
}

CvMat *ginput( IplImage const &colour )
{
  int NPoints;
  cv_bspline_test_image = cvCloneImage( &colour );

  printf("Enter the number of points for approximation: ");
  scanf("%d", &NPoints );
  printf("\n");
  cv_bspline_dummy_points_xxx = cvCreateMat(NPoints, 2, CV_64FC1); 

  cvNamedWindow( "GInput", CV_WINDOW_AUTOSIZE );
  cvMoveWindow( "GInput", 200, 200 );
  cvShowImage( "GInput", cv_bspline_test_image );
  cvSetMouseCallback ( "GInput", mouse_handler, NULL );
  cvWaitKey( -1 );

  cvReleaseImage( &cv_bspline_test_image );
  cvDestroyWindow( "GInput" );
  return( cv_bspline_dummy_points_xxx);
}
