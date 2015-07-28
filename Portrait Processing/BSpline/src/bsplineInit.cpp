#include "bspline.h"

int icv_InitBasis( Cv_Basis *basis )
{
  int i, Order, NBasis;
  double *knot,OFFSET,STEP;
  
  if( basis == NULL )
    {
      printf("Error icv_InitBasis: Null pointer passed.\n");
      return -1;
    }
  
  Order = basis->K;
  basis->Refcount = 0;
  
  if( basis->Type == CV_BSPLINE_CLOSED_CURVE )
    {
#ifdef VERBOSE
      printf("In icv_InitBasis: Closed curve\n");fflush(stdout);
#endif
      NBasis = basis->N_b + basis->K - 1;
      OFFSET = (double)Order - 1.0;
      STEP = (double)basis->N_b/(double)basis->N;
      knot=(double *)malloc( (basis->N_b+2*basis->K )*sizeof(double));
      
      for( i = 1; i <= basis->N_b + 2*Order - 1; i++)
	knot[i] = i - 1;
      
#ifdef VERBOSE
      printf("In icv_InitBasis: Allocating memory for closed curve basis..\n");
#endif
      basis->Basis = (double **)malloc((basis->N)*sizeof(double *));
      for( i=0; i < basis->N; i++)
	*(basis->Basis + i) = (double *)malloc((NBasis+1)*sizeof(double));
      
      basis->Deriv1 = (double **)malloc((basis->N)*sizeof(double *));
      for( i=0; i < basis->N; i++)
	*((basis->Deriv1) + i) = (double *)malloc((NBasis + 1)*sizeof(double));

      basis->Deriv2 = (double **)malloc((basis->N)*sizeof(double *));
      for( i = 0; i < basis->N; i++)
	*((basis->Deriv2) + i)=(double *)malloc((NBasis + 1)*sizeof(double));

#ifdef VERBOSE
      printf("In icv_InitBasis: Memory allocation done for closed curve basis.\n");
#endif

    }
  else if( basis->Type == CV_BSPLINE_OPEN_CURVE )
    {
#ifdef VERBOSE
      printf("In icv_InitBasis: Open curve\n"); fflush(stdout);
#endif
      NBasis = basis->N_b;
      knot = (double *) malloc( (NBasis + Order + 1 ) * sizeof(double) );
      for( i = 1; i <= Order; i++ )
	knot[i] = 0.0;
      for( i = Order + 1; i <= NBasis; i++ )
	knot[i] = ( double ) ( i - Order );
      for( i = NBasis + 1; i <= NBasis + Order; i++ )
	knot[i] = (double) ( NBasis + 1 - Order );

      OFFSET = 0.0;
      STEP=((double) ( knot[ NBasis + Order ] - knot[1]) / basis->N );
      basis->Basis = (double **)malloc(( basis->N )*sizeof(double *));

#ifdef VERBOSE
      printf("In icv_InitBasis: Allocating memory for open curve.\n");
#endif
      for( i = 0; i < basis->N; i++ )
	*(basis->Basis + i)  = (double *)malloc((NBasis + 1)*sizeof(double));

      basis->Deriv1 = (double **)malloc( (basis->N) * sizeof(double *));
      for( i = 0; i < basis->N; i++ )
	*(basis->Deriv1+i) = (double *)malloc((NBasis + 1)*sizeof(double));

      (*basis).Deriv2 = (double **)malloc( (basis->N) * sizeof(double *) );
      for(i=0;i<basis->N;i++)
	*(basis->Deriv2+i) = (double *)malloc( (NBasis + 1) * sizeof(double));
#ifdef VERBOSE
      printf("In icv_InitBasis: Memory allocation done for open curve.\n");
#endif
    }
  else
    {
      fprintf(stderr,"In icv_InitBasis: Unable to generate basis functions. Spline Type not set.\n");
      return(-1);
    }

#ifdef VERBOSE
  printf("In icv_InitBasis: Calculating basis function, first and second derivatives.\n");
#endif

  for( i = 0; i < basis->N; i++ )
    cv_BSpline_DBasis(basis->K, OFFSET+i*STEP, NBasis, knot, (basis->Basis)[i], (basis->Deriv1)[i], (basis->Deriv2)[i]);

  /* double sum; */
  /* int j; */
  /* for( i = 0; i < basis->N; i++) */
  /*   { */
  /*     sum = 0.0; */
  /*     for( j = 1; j < NBasis+1; j++) */
  /* 	{ */
  /* 	  sum += basis->Basis[i][j]; */
  /* 	} */
  /*   } */

#ifdef VERBOSE
  printf("In icv_InitBasis: Basis function, first and second derivatives calculated.\n");
#endif

  free(knot);
  return 0;
}
/* 
Can pass a preallocated basis, if NULL, will be allocated 
internally; better that way 
*/
int cv_BSpline_Write_File(char const *file_name, Cv_BSpline const *curve)
{
  FILE *fp;
  if( ( curve == NULL) || (curve->Basis == NULL) )
    {
      fprintf(stderr, "In cv_BSpline_Write_File: incomplete information, cannot write to file.\n");
      return(-1);
    }
  else
    {
      if( ( fp = fopen( file_name, "w")) == NULL )
	{
	  fprintf(stderr,"In cv_BSpline_Write_File: cannot open output file.\n");
	  return(-1);
	}
      else
	{
	  fprintf( fp,"%d\n", (curve->Basis->K) );
	  fprintf( fp,"%d\n", (curve->Basis->N_b) );
	  fprintf( fp,"%d\n", (curve->Basis->N) );
	  fprintf( fp,"%d\n", (curve->Basis->Type) );
	  for( int i=0; i < curve->Basis->N_b; i++ )
	    fprintf( fp, "%lf %lf\n", cvmGet(curve->Control_points, i, 0), cvmGet(curve->Control_points, i, 1));
	  fclose( fp );

	  return 0;
	}
    }
}

int cv_BSpline_Read_File(char const *input_file, Cv_BSpline *curve, Cv_Basis *basis )
{

  double tempx, tempy;
  int i;
  FILE *file = fopen(input_file, "r");

#ifdef VERBOSE
  printf("Reading file.\n");
#endif

  if(file == NULL)
    {
      fprintf(stderr, "In cv_BSpline_Read_File: input file not found.\n");
      return(-1);
    }
  else 
    {
      if( curve == NULL)
	{
	  fprintf( stderr, "In cv_BSpline_Read_File: Input curve NULL. Returning to calling function.\n");
	  fclose( file );
	  return -1;
	}

#ifdef VERBOSE
      printf("In cv_BSpline_Read_File: NULL Basis passed. Allocating memory.\n");
#endif

      if( basis == NULL )
	{
	  curve->Basis = (Cv_Basis *)malloc(sizeof(Cv_Basis));
	  basis = curve->Basis;
	  curve->Basis->DAlloc = 1;
	}
      else
	{
	  curve->Basis = basis;
	  /*  curve->Basis->DAlloc = 0; Do not change what is given to you */ 
	}

      fscanf( file,"%d\n", &(basis->K) );
      fscanf( file,"%d\n", &(basis->N_b) );
      fscanf( file,"%d\n", &(basis->N) );
      fscanf( file,"%d\n", &(basis->Type) );
      curve->Control_points = cvCreateMat( (*basis).N_b, 2, CV_64FC1);
#ifdef VERBOSE
      printf( "%d\n", (basis->K) );
      printf( "%d\n", (basis->N_b) );
      printf( "%d\n", (basis->N) );
      printf( "%d\n", (basis->Type) );
#endif
      i = 0;
      while(!feof(file))
	{
	  fscanf( file,"%lf,%lf\n", &tempx, &tempy );
	  cvmSet( curve->Control_points, i, 0, tempx );
	  cvmSet( curve->Control_points, i, 1, tempy );
#ifdef VERBOSE
	  printf("%lf %lf\n", cvmGet( curve->Control_points, i, 0 ), cvmGet( curve->Control_points, i, 1) );
#endif
	  i++;
	}

      fclose(file);

#ifdef VERBOSE
      printf("In cv_BSpline_Read_File: file operation over successfully.\nGenerating basis and derivatives.\n");
#endif
      /* Reading part over; now generate the curve */
      /* First generate the basis, 1st and 2nd derivatives */
      if( icv_InitBasis( basis ) )
	{
	  printf("Error in initialisation of basis function.\n");
	  return(-1);
	}
      basis->Refcount++;

      /* Calculate them curve itself and the two derivatives */
      curve->Curve_points = cvCreateMat( (*basis).N, 2, CV_64FC1);
      curve->Curve_D1 = cvCreateMat( (*basis).N, 2, CV_64FC1);
      curve->Curve_D2 = cvCreateMat( (*basis).N, 2, CV_64FC1);
      curve->Curve_Normal = cvCreateMat( (*basis).N, 2, CV_64FC1);

      if( cv_BSpline_Compute(curve, (CV_BSPLINE_COMPUTE_CURVE|CV_BSPLINE_COMPUTE_DERIV1|CV_BSPLINE_COMPUTE_DERIV2), NULL) )
	{
	  printf("Problem in initialising curve.\n");
	  return(-1);
	}
      if( icv_BSpline_Compute_Normal( curve ) != 0 )
	{
	  fprintf( stderr, "In cv_BSpline_Read_File: Unable to compute curve normal.\n");
	}
      /* if( cv_BSpline_Compute(curve, CV_BSPLINE_COMPUTE_DERIV1, NULL) ) */
      /* 	{ */
      /* 	  printf("Problem in initialising curve.\n"); */
      /* 	  return(-1); */
      /* 	} */
      /* if( cv_BSpline_Compute(curve, CV_BSPLINE_COMPUTE_DERIV2, NULL) ) */
      /* 	{ */
      /* 	  printf("Problem in initialising curve.\n"); */
      /* 	  return(-1); */
      /* 	} */

      return(0);
    }
}
