/*
References:
https://vismor.com/documents/network_analysis/matrix_algorithms/matrix_algorithms.php
https://en.wikipedia.org/wiki/LU_decomposition
Solomon Numerical Algorithms
David Bindel Numerical Algorithms
JAMA
*/

/* TODO(maciej): 
 * [ ] Clean this up, boy is it messy
*/


#if defined( MSH_NM_USE_FLOAT )
typedef float msh_nm_scalar_t;
#else
typedef double msh_nm_scalar_t;
#endif

#if defined( MSH_NM_USE_FLOAT )
  #define MSH_NM_EPSILON 1.0E-3;
#else
  #define MSH_NM_EPSILON 1.0E-6;
#endif


typedef struct msh_nm_vector
{
  size_t rows;
  msh_nm_scalar_t* data;
} msh_nm_vec_t;

// Implement this with stb trick for accessing data without referring to 'data'
typedef struct msh_nm_matrix
{
  size_t cols, rows;
  msh_nm_scalar_t* data;
} msh_nm_mat_t;

void msh_nm_mat_init( msh_nm_mat_t* mat, size_t cols, size_t rows );
void msh_nm_mat_free( msh_nm_mat_t* mat );

void msh_nm_mat_init( msh_nm_mat_t* mat, size_t cols, size_t rows )
{
  mat->cols = cols;
  mat->rows = rows;
  mat->data = (msh_nm_scalar_t*)malloc( cols*rows*sizeof(msh_nm_scalar_t) );
}

void msh_nm_multiply( msh_nm_mat_t* A, msh_nm_mat_t* B, msh_nm_mat_t* C )
{
}

void msh_nm_sq_transpose( double *A, int m )
{
  for(int i = 0 ; i < m; ++i )
  {
    for( int j = i; j < m; ++j )
    {
      double tmp = A[i * m + j];
      A[i * m + j] = A[j * m + i];
      A[j * m + i] = tmp;
    }
  }
}

void nm_print_matrixd( double* M, int rows, int cols );
void nm_print_matrixi( int* M, int rows, int cols );
// void msh_nm_lu_decompositon( msh_nm_mat_t *A, msh_nm_vec_t *p )
// {
// 
// }

// msh_nm_vec_t msh_nm_lu_solve(A, p, b) {};

// void msh_nm_solve( msh_nm_ctx_t ) <--- I think this is the final api.


/***************************************************************************************************
LU Decomposition Notes:

LU decomposition decomposes A into a lower triangular matrix L and upper triangular matrix U, such
that:
  A = LU
Solving the system Ax=b using LU factorization amounts solving two triangular systems
A) Solve : Ly = b => y = L^(-1)b
B) Solve : Ux = y => x = U^(-1)y

x = U^(-1)y = U^(-1)L^(-1)b = (LU)^-1b = A^(-1)b

Solving A) and B) can be done efficiently with forward and backward substitution respectively.

There are two main algorithms for implementing LU facrotization:
--> Gaussian Elimination
--> Dolittle's/Crout's Algorithm

Note that there is only a minor difference between Crout and Doolittle's algorithm. See 
(https://vismor.com/documents/network_analysis/matrix_algorithms/matrix_algorithms.php) for details.

All these algorithm require that a_ii element is non-zero, hence the original problem is
transformed to:
  PA = LU
where P is a row-permutation matrix that ensures that all a_ii elements are non-zero. This
is called partial pivoting. Partial pivoting requires that A is invertible. Full pivoting
removes this requirement, but at the cost of speed. Full pivoting:
  PAQ = LU,
where Q is column-permutation matrix.

This library implements gaussian elimination, and Crout algorithm with implicit pivoting.

Main difference between JAMA and NR implementation of Crout algorithm are:
1) JAMA provides nice implementation of crout algorithm, with nice copy optimization for column access
2) NR does do partial implicit pivoting...

This library currently does not provide implementation of full pivoting.


TODO(maciej): 
1. Measure performance for all cases
2. How would one make implicit pivoting optional - seems like asymptotically implicit pivoting is not 
changing assymptotic behaviour, it does require another full pass through memory O(N) reads...
3. Measure best performing algorithm with and without pivoting.
***************************************************************************************************/

// I definietly need to try the aliasing thing that the numerical class is mentioning, especially
// in c and cpp modes

// In this whole exploration of the topic I think we are going to start with implementing helpers,
// like general matrix multiplication etc. Then we will need to revise the code in my icp to check why
// I even need the svd. Trimesh does not use it at all. It's just LDLT and eigen decompositions.



// After Solomn, Figure 2.3.
// Added implicit pivoting.
void 
LU_factor_gauss_elimination( uint32_t m, uint32_t n, double* A, int* piv )
{
  for( int r = 0; r < n; ++r )
  {
    piv[r] = r;
  }

  double* pivot = malloc(sizeof(double) * m);

  for( int i = 0; i < m; i++) 
  {
    double big = 0;
    for( int j = 0; j < n; j++) 
    {
      double tmp = fabs(A[i * n + j]);
      if (tmp > big) { big = tmp; }
    }
    if (big == 0) { return; }
    pivot[i] = 1.0 / big;
  }

  for( int p = 0; p < min(m - 1, n); ++p )
  {
    // find best row.
    int pmax = p;
    double max = 0.0;
    for( int r = p; r < n; ++r )
    {
      double tmp = pivot[r] * fabs(A[r*n+p]);
      if( tmp > max )
      {
        max = tmp;
        pmax = r;
      }
    }
    // Preserve denominator of max element
    double denom = 1.0 / A[pmax*n+p];
    
    // permute rows
    if( pmax != p )
    {
      piv[p] = pmax;
      pivot[pmax] = pivot[p];
      for( int c = 0; c < m; ++c )
      {
        double Atmp = A[p*n+c];
        A[p*n+c] = A[pmax*n+c];
        A[pmax*n+c] = Atmp;
      }
    }

    // Perform gaussian elimnation
    for( int r = p+1; r < m; ++r )
    {
      double s = A[r*n+p] * denom;
      A[r*n+p] = s;
      for( int c = p+1; c < n; ++c )
      {
        A[r*n+c] -= s * A[p*n+c];
      }
    }
  }
  free(pivot);
}

bool
LU_factor_nr( uint32_t m, uint32_t N, double* A, int* ind ) // Is this numerical recipies?
{
  double* pivot = malloc(sizeof(double) * N);

  for( int i = 0; i < N; i++) 
  {
    double big = 0;
    for( int j = 0; j < N; j++) 
    {
      double tmp = fabs(A[i * N + j]);
      if (tmp > big) { big = tmp; }
    }
    if (big == 0) { return false; }
    pivot[i] = 1.0 / big;
  }

  for( int j = 0; j < N; j++)
  {
    for( int i = 0; i < j; i++) 
    {
      double sum = A[i * N + j];
      for( int k = 0; k < i; k++)
        sum -= A[i * N + k] * A[k * N + j];
      A[i * N + j] = sum;
    }

    double big = 0;
    int imax = j;
    for( int i = j; i < N; i++) {
      double sum = A[i * N + j];
      for( int k = 0; k < j; k++)
        sum -= A[i * N + k] * A[k * N + j];
      A[i * N + j] = sum;
      double tmp = pivot[i] * fabs(sum);
      if (tmp > big) {
        big = tmp;
        imax = i;
      }
    }

    if (imax != j) {
      for( int k = 0; k < N; k++)
      {
        double tmp = A[imax * N + k];
        A[imax * N + k] = A[j * N + k];
        A[j * N + k] = tmp;
      }
      pivot[imax] = pivot[j];
    }
    ind[j] = imax;
  
    if( A[j * N + j] == 0 )
      return false;
    
    if (j != N - 1) 
    {
      double tmp = 1 / A[j * N + j];
      for( int i = j + 1; i < N; i++)
        A[i * N + j] *= tmp;
    }
  }

  free( pivot );
  return true;
}

// A clever implementation of Dolittle/Crout from JAMA. It combines the 2 for loops b noticing
// that the only difference there is the division of by the scaling element, which is deffered
// till after dot product is computed. NR is the same, but it cannot split the loops, as it
// needs to do implicit pivoting in the middle.
// JAMA also has nice column copy optimization.

// Now this also has implicit pivoting. Cool.
void
LU_factor_jama( uint32_t m, uint32_t n, double* A, int* piv )
{
  for( int i = 0; i < m; i++) {
    piv[i] = i;
  }

  double* LUrowi;
  double* LUcolj = malloc( m * sizeof(double) );
  double* pivot = malloc(sizeof(double) * m);
  for( int i = 0; i < m; i++) 
  {
    double big = 0;
    for( int j = 0; j < n; j++) 
    {
      double tmp = fabs(A[i * n + j]);
      if (tmp > big) { big = tmp; }
    }
    if (big == 0) { return; }
    pivot[i] = 1.0 / big;
  }
  // printf("%f %f %f\n", pivot[0], pivot[1], pivot[2]);

  // Outer loop.
  for( int j = 0; j < n; j++) {

    // Make a copy of the j-th column to localize references.
    for( int i = 0; i < m; i++) {
      LUcolj[i] = A[ i*n +j ];
    }

    // Apply previous transformations.

    for( int i = 0; i < m; i++) {
      LUrowi = &A[i*n];
      // printf("TEST: %d %d\n", i, j);
      // printf("A : %f %f %f\n", LUcolj[0], LUcolj[1], LUcolj[2]);
      // printf("B : %f %f %f\n", LUrowi[0], LUrowi[1], LUrowi[2]);
  
      // Most of the time is spent in the following dot product.

      int kmax = min(i,j);
      double s = 0.0;
      for( int k = 0; k < kmax; k++) {
        s += LUrowi[k]*LUcolj[k];
      }
      
      LUrowi[j] -= s;
      LUcolj[i] -= s;
      // printf("S: %f | kmax: %d\n", s, kmax );
      // printf("A : %f %f %f\n", LUcolj[0], LUcolj[1], LUcolj[2]);
      // printf("B : %f %f %f\n", LUrowi[0], LUrowi[1], LUrowi[2]);
      // printf("----\n");
      
    }
   
    // Find pivot and exchange if necessary.
    // printf("===========\n");
    int p = j;
    double pmax = 0;
    for( int i = j; i < m; i++) {
      double tmp = pivot[i] * fabs(LUcolj[i]);
      // printf("  %f %f %f\n", tmp, pivot[i], fabs(LUcolj[i]));
      if( tmp > pmax ) {
        p = i;
        pmax = tmp;
      }
    }
    
    if (p != j) {
      for( int k = 0; k < n; k++) {
        double t = A[p*n+k]; A[p*n+k] = A[j*n+k]; A[j*n+k] = t;
      }
      piv[j] = p;
      pivot[p] = pivot[j];
    }
    
    // nm_print_matrixd(A, 3, 3);

    // Compute multipliers.
    if (j < m && A[j*n+j] != 0.0) {
      double denom = 1.0 / A[j*n+j];
      for( int i = j+1; i < m; i++) {
        A[i*n+j] *= denom;
      }
    }
    // nm_print_matrixd(A, 3, 3);
  }
}


// TODO(maciej): Pivoting
void 
LU_solve( uint32_t m, uint32_t n, double *A, int* piv, double* b )
{
  // forward
  for( uint32_t i = 0; i < n; ++i )
  {
    int ip = piv[i];
    double alpha = b[ip];
    b[ip] = b[i];
    for( uint32_t j = 0; j < i; ++j )
    {
      alpha -= A[i * n + j]*b[j];
    }
    b[i] = alpha;
  }

  // backward
  for( int32_t i = n-1; i >= 0; --i )
  {
    double alpha = b[i];
    for( uint32_t j = i+1; j < n; ++j )
    {
      alpha -= A[i*n + j]*b[j];
    }
    b[i] = alpha / A[i*n + i];
  }
}

void LU_inverse( uint32_t m, uint32_t n, double* A, int* piv, double* Ainv )
{
  assert(A && piv && Ainv );

  double* b = malloc( m * sizeof(double) );
  int unity_idx = 0;

  for( int i = 0; i < m; ++i )
  {
    memset(b, 0, sizeof(double) * m );
    b[unity_idx++] = 1.0;
    LU_solve( m, n, A, piv, b );
    memcpy( Ainv + i * n, b, m * sizeof(double) );
  }
  msh_nm_sq_transpose( Ainv, 6 );
  free(b);
}

// Check : https://github.com/ldfaiztt/cs267-dgemm/tree/master/src 
// Check : https://github.com/ytsutano/dgemm-goto-in-c // not super fast
// Check : https://github.com/cappachu/dgemm // bit faster, but requires sse4

// Scalar replacement
void mmm1( const int N, const int M, const int P, 
           const double* restrict A, const double* restrict B, double* restrict C )
{
  int32_t i, j, k;
  for( i = 0; i < N; i++)
  {
    for(j = 0; j < M; j++)
    {
      double sum = 0;
      for(k=0; k < P; k++)
      {
        sum += A[i*P+k] * B[k*M+j];
      }
      C[i*M+j] += sum;
    }
  }
}

// Based on: https://www.akkadia.org/drepper/cpumemory.pdf
void mmm3( const int N, const int M, const int P, double* restrict A, double* restrict B, double* restrict C )
{
  uint32_t i, i2, j, j2, k, k2;
  uint32_t SM = 64;
  double *restrict rres;
  double *restrict rmul1;
  double *restrict rmul2;
  for( i = 0; i < N; i += SM)
  {
    uint32_t i2lim = min( SM, N-i );
    for( j = 0; j < M; j += SM)
    {
      uint32_t j2lim = min( SM, M-j );
      for( k = 0; k < P; k += SM)
      {
        uint32_t k2lim = min( SM, P-k );
        for( i2 = 0, rres = &C[i*M+j], rmul1 = &A[i*P+k]; i2 < i2lim; ++i2, rres += M, rmul1 += P)
        {
          for( k2 = 0, rmul2 = &B[k*M+j]; k2 < k2lim; ++k2, rmul2 += M)
          {
            for( j2 = 0; j2 < j2lim; ++j2)
            {
              rres[j2] += rmul1[k2] * rmul2[j2];
            }
          }
        }
      }
    }
  }
}

void measure_mmm1( const int n_iter, const int N, const double* A, const double* B, double* C )
{
  // uint64_t t1, t2;

  mmm1( N, N, N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  // t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm1(N, N, N, A, B, C);
  // t2 = msh_time_now();
  // printf("Time taken by %s is %12.7fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}

void measure_mmm3( const int n_iter, const int N,  double* A, double* B, double* C )
{
  // uint64_t t1, t2;

  mmm3( N, N, N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  // t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm3(N, N, N, A, B, C);
  // t2 = msh_time_now();
  // printf("Time taken by %s is %12.7fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}

void nm_print_matrixd( double* M, int rows, int cols )
{
  /* we interpret rows as columns, to get column major ordering in c */
  for( int y = 0; y < rows; ++y )
  {
    for( int x = 0; x < cols; ++x )
    {
      printf("%8.3f ", M[ y * cols + x ] );
    }
    printf("\n");
  }
  printf("\n");
}

void nm_print_matrixi( int* M, int rows, int cols )
{
  /* we interpret rows as columns, to get column major ordering in c */
  for( int y = 0; y < rows; ++y )
  {
    for( int x = 0; x < cols; ++x )
    {
      printf("%8d ", M[ y * cols + x ] );
    }
    printf("\n");
  }
  printf("\n");
}

// int main( int argc, char** argv )
// {
// #if 0
//   enum{ N_SIZES = 256 };
//   int32_t sizes[N_SIZES] = { 0 };
//   for( int i = 0; i < 256; ++i )
//   {
//     sizes[i]= i+2;
//   }

//   for( int32_t i = 0; i < N_SIZES; ++i )
//   {
//     int32_t N = sizes[i];
//     double* A = malloc( N*N*sizeof(double) );
//     double* B = malloc( N*N*sizeof(double) );
//     double* C = malloc( N*N*sizeof(double) );
//     for( int i = 0; i < N*N; ++i )
//     {
//       A[i] = i;
//       B[i] = i;
//     }

//     printf("Size: %d\n", N );
//     measure_mmm1( 10, N, A, B, C );
//     measure_mmm3( 10, N, A, B, C );
//     free(A);
//     free(B);
//     free(C);
//   }
// #endif

// #if 0
//   double A[12] = { 1, 2, 3, 4,  5, 6, 7, 8,  9, 10, 11, 12 };
//   double B[12] = { 1, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0 };
//   double C[16] = { 0 };
//   mmm3( 4, 4, 3,  A, B, C );
//   nm_print_matrix( &A[0], 4, 3 );
//   nm_print_matrix( &B[0], 3, 4 );
//   nm_print_matrix( &C[0], 4, 4 );
//   printf("TEST\n");
// #endif 

//   // double A[9] = { 6, 1, 4, 1, 2, 7, 3, 5, -1 };
//   // double A[9] = { 16, 4, 6, 21, 5, 72, 4, 9, 8 };
//   double A[9] = { 3, 5, 7, 2, 8, 1, 2, 1, 3 };
//   int piv[3] = {0, 1, 2};
//   nm_print_matrixd( &A[0], 3, 3 );
//   uint64_t t1, t2;
//   t1 = msh_time_now();
//   LU_factor_gauss_elimination( 3, 3, &A[0], &piv[0] );
//   t2 = msh_time_now();
//   double elapsed = msh_time_diff(MSHT_MILLISECONDS, t2, t1);
//   printf("Factorization time: %fms\n", elapsed);
//   nm_print_matrixd( &A[0], 3, 3 );
//   nm_print_matrixi( &piv[0], 1, 3 );
//   double b[3] = { 52, 34, 21 };
//   LU_solve( 3, 3, &A[0], &piv[0], &b[0] );
  
//   printf("%f %f %f\n", b[0], b[1], b[2]);

//   // double B[12] = { 6, 1, 4, 3, 5, -1, 4, 5, 8, 1, 2, 7 };
//   // nm_print_matrix( &B[0], 4, 3 );
//   // LU_gauss_elimination( 4, 3, &B[0] );
//   // nm_print_matrix( &B[0], 4, 3 );

// }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SVD
//  SVD code adapted from SVD implementation in gaps by Thomas Funkhouser
//  Original files by Ronen Barzel and Chuck Rose
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void 
msh_svd_decompose( int32_t m, int32_t n, const msh_nm_scalar_t *A, 
                   msh_nm_scalar_t *U, msh_nm_scalar_t *Sigma, msh_nm_scalar_t *Vt );

void
msh_svd_backsubstitute( int32_t m, int32_t n, 
                        const msh_nm_scalar_t *U, const msh_nm_scalar_t *Sigma, const msh_nm_scalar_t *Vt, 
                        const msh_nm_scalar_t *b, msh_nm_scalar_t *x );


void 
msh_svd_solve( int m, int n, const msh_nm_scalar_t *A, 
               const msh_nm_scalar_t *b, msh_nm_scalar_t *x );

// Source file for SVD (based on code from Numerical Recipes,
// provided by Ronen Barzel and Chuck Rose


/*************************************************************
** Singular-Value Decomposition.
** 
** Had to write this by hand, since the version in Numerical
** Recipes has a bug, carried over from a bug in Golub's
** original algorithm:
        Gene H. Golub & Charles F. Van Loan
        MATRIX COMPUTATIONS
        Johns Hopkins University Press, 1983
        Fifth printing, 1987
        Page 293
        Algorithm 8.3-2: The SVD Algorithm
        ...Find the largest q and the smallest p such that if
                 +-            -+
                 | A11   0   0  |          p
                 |  0   A22  0  |        n-p-q
           A  =  |  0    0  A33 |          q
                 |  0    0   0  |         m-n
                 +-            -+
        then A33 is diagonal and A22 has a nonzero superdiagonal.
        If q = n then quit.
        If any diagonal entry in A22 is zero, then zero the superdiagonal
        element in the same row and go to Repeat...
The last sentence above is the cause of the trouble, in the following case:
                     +-   -+
                     |0 1 0|
                 A = |0 1 1|
                     |0 0 0|
                     +-   -+
In this case, q is 0, A33 is null, and A22 is the same as A.  The instruction
            "if any diagonal entry in A22 is zero, then
             zero the superdiagonal element in the same row"
cannot be applied -- A22 has a diagonal entry that is zero, but there
is no superdiagonal element in that same row.  The proper thing to
do seems to be to special-case: If A22 has 0 in its lower-right
diagonal element, zero the superdiagonal element above it.
**
** 
**
** Since the Num. Rec. code was cribbed from EISPACK or LINPACK
** or some such, which in turn cribbed from other places, I would
** consider an SVD routine suspect unless it works on the following
** test data:
**		a = 	0 1 0
**      		0 1 1
**      		0 0 0
**
** Anyway, Al Barr & I went through the references to figure out
** how it all worked.  I reimplimented it from scratch, first in
** lisp, and now in C.  Martin L. Livesey used the code and reported
** various bugs, which have been fixed.
**
**
** public routines:
**	num_svd()
**	num_svd_backsubst()
**
**	Ronen Barzel   July 1989
**       (bug fixes: Feb 1990)
**	 (ansi-C, backsubst: May 1993)
**
****************************************************************/


#define MSH_NM_MIN(A,B)	(((A)<(B)) ? (A):(B))
#define MSH_NM_MAX(A,B)	(((A)>(B)) ? (A):(B))
#define MSH_NM_ALLOC2D(m,n)	(msh_nm_scalar_t *) malloc((unsigned)(m)*(n)*sizeof(msh_nm_scalar_t))
#define MSH_NM_ALLOC1D(n)	(msh_nm_scalar_t *) malloc((unsigned)(n)*sizeof(msh_nm_scalar_t))
#define MSH_NM_FREE(p)			free((char*)(p))
#define MSH_NM_CLEAR2D(a,m,n)	(void) memset((char*)(a),0,(m)*(n)*sizeof(msh_nm_scalar_t))
#define MSH_NM_COPY2D(d,m,n,s)	(void) memcpy((char*)(d),(char*)(s),(int)((m)*(n)*sizeof(msh_nm_scalar_t)))
#define MSH_NM_REF2D(a,m,n,i,j)	(a[(n)*(i)+(j)])

msh_nm_scalar_t msh_svd__fhypot (msh_nm_scalar_t a, msh_nm_scalar_t b)
{
    return ((msh_nm_scalar_t) sqrt (a*a + b*b));
}

/* householder transformations
**
** looks at the submatrix of A below and to the right of the [i,j]th element
** (inclusive) (where i and j go from 0 to n-1).  Performs a householder
** transformation on the submatrix, to zero out all but the first
** elements of the first column.
**
** Matrix u (a rotation matrix) is transformed by the inverse of the householder
** transformation, so that (old u)(old a) = (new u)(new a)
**
** a is m x n,  u is m x m
**
** If the submatrix is (X1, X2, ...) the householder transformation is
** P_X1,  X1 transforms to (|X1|,0,0,0,...), and the resulting matrix
** is ((|X1|,0,0,...),P_X1 X2, P_X1 X3, ...)
*/


void msh_svd__householder_zero_col(msh_nm_scalar_t *a,msh_nm_scalar_t *u,int i,int j,int m,int n,msh_nm_scalar_t *hv)
  /* hv is a work vector, length m */
{
  msh_nm_scalar_t scale,  /* a scale factor for X1 */
  sigma,  /* the norm^2 of X1 */
  snorm,  /* +/- the norm of X1 */
  f, h, s, dot;
  int k,l;

  /* we will scale X1 by its l1-norm.  squawk! */
  for( scale = 0, k = i; k < m; k++ ) { scale += ((msh_nm_scalar_t) fabs(MSH_NM_REF2D(a,m,n,k,j))); }

  /* if X1 is 0, no point doing anything else */
  if( !scale ) { return; }

  /* divide out the l1-norm, and calculate sigma = |X|^2 */
  for( k = i; k < m; k++ )          { hv[k] = MSH_NM_REF2D(a,m,n,k,j) / scale; }
  for( sigma = 0, k = i; k<m; k++ ) { sigma += hv[k]*hv[k]; }

  /* The householder vector is X1 +/- (|X1|,0,0,...).  We will
  ** contruct this vector in hv and use it to perform the householder
  ** transformation on the matrix.  The plus or minus on the norm
  ** is to reduce roundoff error.  */
  f = hv[i];
  snorm = ((msh_nm_scalar_t) ((f > 0) ? -sqrt(sigma) : sqrt(sigma)));
  h = f*snorm - sigma;	/* eqn 11.2.4 in Press et.al. */
  hv[i] = f - snorm;

  /* set the first column of a to be the (|X1|,0,...) -- this is
  ** what we would get by performing the transformation, but this
  ** way's faster */
  MSH_NM_REF2D(a,m,n,i,j) = scale * snorm;
  for( k=i+1; k<m; k++) { MSH_NM_REF2D(a,m,n,k,j) = 0; }

  /* Now perform the householder transformation on the rest of
  ** the columns.  If the householder vector is X, and -half its norm^2
  ** is h, the householder transform is P_ij : delta_ij + x_i x_j / h.
  ** We don't actually create the P matrix, we just do the calcuation.
  */
  for( k = j+1; k < n; k++ )
  {
    for( dot = 0, l=i; l < m; l++ ) { dot += MSH_NM_REF2D(a,m,n,l,k)*hv[l]; }
    s = dot/h;
    for( l = i; l < m; l++ ) { MSH_NM_REF2D(a,m,n,l,k) += s*hv[l]; }
  }

  /* Similarly, perform the householder transformation on (all)
  ** the rows of u.  Note that it's the rows of u rather than
  ** the columns, because we want U to invert what we've done to a.
  */
  for( k=0; k<m; k++ )
  {
    for( dot = 0, l = i; l < m; l++ ) { dot += MSH_NM_REF2D(u,m,m,k,l)*hv[l]; }
    s = dot/h;
    for( l = i; l < m; l++ ) { MSH_NM_REF2D(u,m,m,k,l) += s*hv[l]; }
  }
}

/* this is the same as msh_svd__householder_zero_col, but with rows
** and cols swapped.  
*/

void msh_svd__householder_zero_row(msh_nm_scalar_t *a,msh_nm_scalar_t *v,int i,int j,int m,int n,msh_nm_scalar_t *hv)
{
  msh_nm_scalar_t scale, sigma,snorm, f, h, s, dot;
  int k,l;

  for( scale = 0, k = j; k < n; k++ ) { scale += ((msh_nm_scalar_t) fabs(MSH_NM_REF2D(a,m,n,i,k))); }
  if( !scale ) { return; }

  for( k = j; k < n; k++ )            { hv[k] = MSH_NM_REF2D(a,m,n,i,k) / scale; }
  for( sigma = 0, k = j; k < n; k++ ) { sigma += hv[k]*hv[k]; }

  f = hv[j];
  snorm = ((msh_nm_scalar_t) ((f > 0) ? -sqrt(sigma) : sqrt(sigma)));
  h = f*snorm - sigma;
  hv[j] = f - snorm;

  MSH_NM_REF2D(a,m,n,i,j) = scale * snorm;
  for( k = j + 1; k < n; k++ ) { MSH_NM_REF2D(a,m,n,i,k) = 0; }

  for( k = i + 1; k < m; k++ )
  {
   for( dot=0, l=j; l<n; l++ ) { dot += MSH_NM_REF2D(a,m,n,k,l)*hv[l]; }
   s = dot/h;
   for( l=j; l<n; l++ ) { MSH_NM_REF2D(a,m,n,k,l) += s*hv[l]; }
  }

  for( k = 0; k < n; k++ )
  {
    for( dot = 0, l = j; l < n; l++ ) { dot += MSH_NM_REF2D(v,n,n,l,k)*hv[l]; }
    s = dot/h;
    for( l = j; l < n; l++ ) { MSH_NM_REF2D(v,n,n,l,k) += s*hv[l]; }
  }
}

/*
** performs a Givens rotation on the ith and jth columns of a, looking
** at the n elements starting at start.  a is mm x nn.
*/
void msh_svd__rotate_cols(int i,int j,msh_nm_scalar_t cos,msh_nm_scalar_t sin,msh_nm_scalar_t* a,int start,int n,int mm,int nn)
{
  int end = start+n, k;
  msh_nm_scalar_t x,y;
  for( k = start; k < end; k++ )
  {
    x = MSH_NM_REF2D(a,mm,nn,k,i);
    y = MSH_NM_REF2D(a,mm,nn,k,j);
    MSH_NM_REF2D(a,mm,nn,k,i) =  cos*x + sin*y;
    MSH_NM_REF2D(a,mm,nn,k,j) = -sin*x + cos*y;
  }
}


/*
** performs a Givens rotation on the ith and jth rows of a, looking
** at the n elements starting at start.  a is mm x nn.
*/
void msh_svd__rotate_rows(int i,int j, msh_nm_scalar_t cos, msh_nm_scalar_t sin, msh_nm_scalar_t* a, int start, int n, int mm, int nn)
{
  int end = start+n, k;
  msh_nm_scalar_t x,y;
  for( k = start; k < end; k++ )
  {
    x = MSH_NM_REF2D( a, mm, nn, i, k);
    y = MSH_NM_REF2D( a, mm, nn, j, k);
    MSH_NM_REF2D( a, mm, nn, i, k) =  cos*x + sin*y;
    MSH_NM_REF2D( a, mm, nn, j, k) = -sin*x + cos*y;
  }
}

/*
** This takes a submatrix of b (from p through z inclusive).  The submatrix
** must be bidiagonal, with a zero in the upper-left corner element.  Mucks
** with the sumatrix so that the top row is completely 0, accumulating
** the rotations into u.  b is m x n.  u is m x min.
**
** Suppose the matrix looks like
**   0 R 0 0 0...
**   0 X X 0 0...
**   0 0 X X 0...
**   ...
** Where R & X's are different & non-zero.  We can rotate the first
** and second rows, giving
**   0 0 R 0 0...
**   0 X X 0 0...
**   0 0 X X 0...
**   ...
** with new R's and X's.  We rotate the first and third rows, moving
** R over again, etc.  till R falls off the end.
*/

void msh_svd__clr_top_supdiag_elt(msh_nm_scalar_t * b,int p,int z,msh_nm_scalar_t * u,int m,int n,int min)
{
  int i;
  msh_nm_scalar_t r, x, hypot, cos, sin;

  for( i = p+1; i <= z; i++ )
  {
    r = MSH_NM_REF2D(b,m,n,p,i);
    x = MSH_NM_REF2D(b,m,n,i,i);
    if( r == 0 ) { break; }
    hypot = ((msh_nm_scalar_t) sqrt (r*r + x*x)); //hypot = pythag(r,x);
    cos = x/hypot;
    sin = r/hypot;
    /* update the diagonal and superdiagonal elements */
    MSH_NM_REF2D(b,m,n,p,i) = 0;
    MSH_NM_REF2D(b,m,n,i,i) = hypot;
    /* Rotate the remainder of rows p and i (only need to
    ** rotate one more element, since the rest are zero) */
    if( i != z ) { msh_svd__rotate_rows(p,i,cos,sin,b,i+1,1,m,n); }
    /* accumulate the rotation into u */
    msh_svd__rotate_cols(i,p,cos,sin,u,0,m,m,min);
  }
}

/*
** This takes a submatrix of b (from p through z inclusive).  The submatrix
** must be bidiagonal, with a zero in the lower-right corner element.  Mucks
** with the sumatrix so that the right column is completely 0, accumulating
** the rotations into v.  b is m x n.  v is min x n
**
** Suppose the matrix looks like
**   X X 0 0 
**   0 X X 0
**   0 0 X R
**   0 0 0 0
** Where R & X's are different & non-zero.  We can rotate the last
** and second-to-last columns, yielding
**   X X 0 0
**   0 X X R
**   0 0 X 0
**   0 0 0 0
** with new R's and X's.  We rotate the last and third-to-last cols, moving
** R over again, etc.  till R falls off the end.
*/

void msh_svd__clr_bot_supdiag_elt(msh_nm_scalar_t * b,int p,int z,msh_nm_scalar_t * v,int m,int n,int min)
{
  int i;
  msh_nm_scalar_t r, x, hypot, cos, sin;

  for( i = z-1; i >= p; i-- )
  {
    r = MSH_NM_REF2D(b,m,n,i,z);
    x = MSH_NM_REF2D(b,m,n,i,i);
    if( r == 0 ) { break; }
    hypot = ((msh_nm_scalar_t) sqrt (r*r + x*x)); //hypot = pythag(r,x);
    cos = x/hypot;
    sin = r/hypot;
    /* update the diagonal and superdiagonal elements */
    MSH_NM_REF2D(b,m,n,i,z) = 0;
    MSH_NM_REF2D(b,m,n,i,i) = hypot;
    /* Rotate the remainder of cols z and i (only need to
    ** rotate one more element, since the rest are zero) */
    if( i != p ) { msh_svd__rotate_cols(i,z,cos,sin,b,i-1,1,m,n); }
    /* accumulate the rotation into v */
    msh_svd__rotate_rows(i,z,cos,sin,v,0,n,min,n);
  }
}

/*
** This takes a submatrix of b (from p through z inclusive).  The submatrix
** must be bidiagonal except that the topmost subdiagonal element is non-zero.
** Mucks with the submatrix to make it bidiagonal, accumulating the rotations
** into u and v.  b is m x n  u is m x min   v is min x n
**
** Suppose the matrix looks like
**   X X 0 0 0...
**   R X X 0 0...
**   0 0 X X 0...
**   ...
** Where R & X's are different & non-zero.  We can rotate the first and
** second rows, giving
**   X X R 0 0...
**   0 X X 0 0...
**   0 0 X X 0...
**   ...
** with new R &X's.  Now rotate the second and third columns, getting
**   X X 0 0 0...
**   0 X X 0 0...
**   0 R X X 0...
**   ...
** which is the same as the initial problem, but decreased in
** size by 1.  Eventually, we'll reach the situation where we have
**      ...
**   ... X X
**   ... R X
** and the row rotation will eliminate R.
*/

void msh_svd__clr_top_subdiag_elt( msh_nm_scalar_t * b, int p, int z, 
                                   msh_nm_scalar_t * u, msh_nm_scalar_t * v, int m, int n, int min)
{
  int i;
  msh_nm_scalar_t x, r, hypot, cos, sin;

  for( i = p ;; i++ ) 
  {
    /* figure out row rotation to zero out the subdiagonal element */
    x = MSH_NM_REF2D(b,m,n,i,i);
    r = MSH_NM_REF2D(b,m,n,i+1,i);
    hypot = msh_svd__fhypot(x,r);
    cos = x/hypot;
    sin = r/hypot;
    /* rotate the leading elements of the row */
    MSH_NM_REF2D(b,m,n,i,i) = hypot;
    MSH_NM_REF2D(b,m,n,i+1,i) = 0;
    /* rotate out the rest of the row */
    msh_svd__rotate_rows(i,i+1,cos,sin,b,i+1,(i+1==z)?1:2,m,n);
    /* accumulate transformation into columns of u */
    msh_svd__rotate_cols(i,i+1,cos,sin,u,0,m,m,min);

    /* end with a row rotation */
    if (i+1==z) break;

    /* figure out column rotation */
    x = MSH_NM_REF2D(b,m,n,i,i+1);
    r = MSH_NM_REF2D(b,m,n,i,i+2);
    hypot = msh_svd__fhypot(x,r);
    cos = x/hypot;
    sin = r/hypot;
    /* rotate the leading elements of the column */
    MSH_NM_REF2D(b,m,n,i,i+1) = hypot;
    MSH_NM_REF2D(b,m,n,i,i+2) = 0;
    /* rotate the rest of the column */
    msh_svd__rotate_cols(i+1,i+2,cos,sin,b,i+1,2,m,n);
    /* accumulate transformation into columns of v */
    msh_svd__rotate_rows(i+1,i+2,cos,sin,v,0,n,min,n);
  }
}

/*
** This is the first part of an implicit-shift QR step.  We do some
** magic eigenvalue calculation, to calculate a Givens rotation for
** the top-left corner.
**
** This rotation is described as part of Golub's Algorithm 3.3-1
**
** This is also described as the implicit QL algorithm for symmetric
** tridiagonal matrices, in Numerical Recipes.  Here's what's going
** on, as far as I can tell:
**
** Hypothetically, one could do a series of QR decompositions of a symmetric
** tridiagonal matrix A.
**   - Ignoring the R's, one serially computes An+1 = Qn^t An Q
**   - Eventually, An will approach diagonal
**   - The rate of convergence goes like A[i,j] = u_i/u_j, where
**     u_i and u_j are the eigenvalues of A
**   - To make it converge faster, we shift the eigenvalues by some amount
**     ("uu" in the code) by decomposing the matrix A - uu I.
**   - uu is computed so as to attempt to minimize (u_i-uu)/(u_j-uu), which
**     is the convergence rate of the shifted matrix.  Ideally uu would
**     be equal to the smallest eigenvalue.  Since we don't know the
**     smallest eigenvalue, we look at the eigenvalues of the
**     trailing 2x2 submatrix.
**   - Rather than actually computing the matrix A - uu I, we just keep
**     track of the shift in the calculation of the coefficients for the
**     rotations that make up the Q's.  Hence the "implicit" in the
**     name of the algorithm.
**
** For SVD, we are looking at a bidiagonal matrix, rather than a tridiagonal
** one.  Thus we will do one more level of implicitness, by computing the
** coefficients we WOULD get were we to consider the tridiagonal matrix
** T = B^t B.
**
** This particular routine just performs the initial rotation on the
** bidiagonal matrix, based on the eigenvalue-shift stuff.  This
** makes the matrix no loger bidiagonal.  Other routines will have to
** clean up the non-bidiagonal terms.  The net rotation that is performed
** by this routine and the cleanup routines is the Q at one step of
** the above iteration.  We don't ever explicitly compute Q, we
** just keep updating the U and V matrices.
**
** b is m x n,   v is min x n
*/

void msh_svd__golub_kahn_svd_rot(msh_nm_scalar_t * b,int p,int q,msh_nm_scalar_t * v,int m,int n,int min)
{
  msh_nm_scalar_t t11, t12, t22;
  msh_nm_scalar_t uu;
  msh_nm_scalar_t b11, b12;
  msh_nm_scalar_t dm,fm,dn,fn;
  msh_nm_scalar_t hypot,cos,sin;
  msh_nm_scalar_t d, s, y, z;

  /* grab the last diagonal and superdiagonal elements of b22 */
  fm = (q-2) < 0 ? 0 : MSH_NM_REF2D(b,m,n,q-2,q-1);
  dm = MSH_NM_REF2D(b,m,n,q-1,q-1);	fn = MSH_NM_REF2D(b,m,n,q-1,q);
      dn = MSH_NM_REF2D(b,m,n,q,q);

  /* create the trailing 2x2 submatrix of T = b22^t b22 */
  t11 = dm*dm + fm * fm;
  t12 = dm * fn;		t22 = dn * dn + fn * fn;

  /* find the eigenvalues of the T matrix.
  **
  ** The quadratic equation for the eigenvalues has coefficients:
  ** a = 1
  ** b = -(t11+t22)
  ** c = t11 t22 - t12 t12
  **
  ** b^2 - 4ac = (t11^2 + t22^2 + 2 t11 t12) - 4 t11 t22 + 4 t12 t12
  **           = (t11 - t22)^2 + 4 t12 t12
  **
  ** sqrt(b^2-4ac) = sqrt((t11 - t22)^2 + 4 t12 t12) -- use "pythag()"
  **
  ** using quadratic formula, we have the eigenvalues:
  ** (u1,u2) = .5 (t11 + t22 +/- pythag(...))
  **         = t22 + .5 (t11 - t22 +/- pythag(...))
  **
  ** We propogate the .5 into the pythag term, yielding golub's equation 8.2-2
  ** for the "wilkinson shift".
  ** [Note:  Golub says to use (t22 + d - signum(d) s) to find the eigenvalue
  ** closer to t22.  He's right.  ]
  **/
  d = ((msh_nm_scalar_t) 0.5)*(t11 - t22);
  s = msh_svd__fhypot (d,t12);
  uu = t22 + d + ((d > 0) ? -s : s);

  /* grab the leading 2x1 of b */
  b11 = MSH_NM_REF2D(b,m,n,p,p);	b12 = MSH_NM_REF2D(b,m,n,p,p+1);

  /* make the leading 2x1 submatrix of T */
  t11 = b11 * b11;
  t12 = b11 * b12;

  /* calculate the rotation that would zero the off-diagonal term of the
  ** shifted matrix T - uu I
  */
  y = t11 - uu;
  z = t12;
  hypot = msh_svd__fhypot (y,z);
  cos = y / hypot;
  sin = z / hypot;

  /* perform the rotation on B.  This sprays some glop into the upper-left
  ** subdiagonal element.
  */
  msh_svd__rotate_cols(p,p+1,cos,sin,b,p,2,m,n);
  /* accumulate the rotation into the rows of v */
  msh_svd__rotate_rows(p,p+1,cos,sin,v,0,n,min,n);
}


/* msh_svd__bidiagonalize
**
** Given a (m x n)
** computes u (m x min)	orthogonal cols
**	    b (m x n)	bidiagonal
**	    v (min x n)	orthogonal rows
** such that a = u b v  (looking at only the min x min leading part of b)
**
** Works by starting with B = A, then performing a series of Householder
** transformations to zero-out the off-diagonal elements, while
** accumulating the transformations into U and V.
**
** b may point to the same array as a, in which case the result
** overwrites the original data.
*/

void msh_svd__bidiagonalize(const msh_nm_scalar_t *a,int m,int n,msh_nm_scalar_t *u,msh_nm_scalar_t *b,msh_nm_scalar_t *v)
{
  int i,j, min=MSH_NM_MIN(m,n);
  msh_nm_scalar_t *usave = u, *vsave = v, *h = MSH_NM_ALLOC1D(MSH_NM_MAX(m,n));

  /* we need square matrices to accumulate the transformations */
  if( min != m ) { u = MSH_NM_ALLOC2D(m,m); }
  if( min != n ) { v = MSH_NM_ALLOC2D(n,n); }

  /* start off with u and v equal to the identity, and b equal to a */
  MSH_NM_CLEAR2D(u,m,m);
  MSH_NM_CLEAR2D(v,n,n);
  for( i=0; i<m; i++) { MSH_NM_REF2D(u,m,m,i,i) = 1; }
  for( i=0; i<n; i++) { MSH_NM_REF2D(v,n,n,i,i) = 1; }
  if (b != a)         { MSH_NM_COPY2D(b,m,n,a); }

  /* walk down the diagonal */
  for( i = 0; i<min; i++) 
  {
    /* zero the entries below b[i,i] */
    msh_svd__householder_zero_col(b,u,i,i,m,n,h);
    /* zero the entries to the right of b[i,i+1] */
    msh_svd__householder_zero_row(b,v,i,i+1,m,n,h);
  }
  /* when m < n (matrix wider than tall), the above
    leaves a non-0 element in the [m-1,m]th spot.
    This is the bottom superdiagonal element of an (m+1)x(m+1);
    use the clear routine to get rid of it. */
  if( min != n )
  {
    msh_svd__clr_bot_supdiag_elt(b,0,min,v,m,n,n);
  }
  /* For non-square arrays, lop off the parts we don't need */
  if( min != m )
  {
    for( i = 0; i < m; i++) 
    {
      for( j = 0; j < min; j++ )
      {
        MSH_NM_REF2D(usave,m,min,i,j)=MSH_NM_REF2D(u,m,m,i,j);
      }
    }
    MSH_NM_FREE(u);
  }
  if (min!=n)
  {
    for( i = 0; i < n; i++ )
    {
      for( j = 0; j < min; j++ )
      {
        MSH_NM_REF2D(vsave,min,n,j,i)=MSH_NM_REF2D(v,n,n,j,i);
      }
    }
    MSH_NM_FREE(v);
  }
  MSH_NM_FREE(h);
}

/*
** Finds the SVD of a bidiagonal matrix.
**
** Zeroes the superdiagonal of a bidiagonal matrix, accumulating
** left- and right-hand transforms into u and v.  The matrix
** is modified in place.
**
** That is, given u, b, v  where b is bidiagonal and u & v are
** rotations, modify the matrices so that (old) u b v = (new) u b v,
** where the new b is diagonal, and u & v are still rotations.
**
** b is m x n,  u is m x min ,  and v is min x n
**
** This is Golub's SVD algorithm (Algorithm 8.3-2)
*/

void msh_svd__bidiagonal_svd(msh_nm_scalar_t *b,int m,int n,msh_nm_scalar_t *u,msh_nm_scalar_t *v)
{
  msh_nm_scalar_t anorm, t;
  int p, q, i, j, z, iter, min=MSH_NM_MIN(m,n);

  /* use the max of the sum of each diagonal element and the
  ** superdiagonal element above it as a norm for the array.
  ** The use of this norm comes from the Numerical Recipes (Press et al)
  ** code.  squawk!
  */

  // CHUCKR: this is where the array access if fouling up
  //for( anorm=MSH_NM_REF2D(b,m,n,0,0), i=1; i<n; i++) {
  for( anorm = MSH_NM_REF2D( b, m, n, 0, 0 ), i = 1 ; i < min ; i++ )
  {
    t = ((msh_nm_scalar_t) (fabs(MSH_NM_REF2D(b,m,n,i,i)) + fabs(MSH_NM_REF2D(b,m,n,i-1,i))));
    if (t > anorm) { anorm = t; }
  }

  /* Each iteration of this loop will zero out the superdiagonal
  ** element above b[q][q] -- i.e. b[q][q] will be a singular value.
  */
  for( q = min-1; q >= 0; q-- )
  {

  /* Each iteration will make the superdiagonal elements smaller,
  ** till they drop below numerical noise.  It ought to converge
  ** by 30 tries.  squawk!  (Increased to 100 tries by funk)
  */
    const int max_iter = 100;
    for( iter=0; iter<max_iter; iter++) 
    {
      
      /* Find a block that has a zero on the diagonal or superdiagonal.
      ** That is, we are breaking up b into three submatrices,
      ** b11, b22, b33, such that b33 is diagonal and b22 has no zeros 
      ** on the superdiagonal.
      ** b33 goes from q+1 to n-1 -- it's the part we already did
      ** b22 goes from p through q -- it's a bidiagonal block
      */

      /* sweep back till we reach a numerically 0 superdiagonal element */
      for( p = q; p>0; p--)
      {
        if (MSH_NM_REF2D(b,m,n,p-1,p) + anorm == anorm)
        {
          MSH_NM_REF2D(b,m,n,p-1,p) = 0;
          break;
        }
      }

      /* if b22 is 1x1, i.e. there is a 0 above b[q,q], then
      ** b[q,q] is the singular value.  Go on to the next
      ** singular value.  (But first, make sure it's
      ** positive.  If it's negative, negate it and the
      ** corresponding row of v)
      */

      if( p == q )
      {
        if( MSH_NM_REF2D( b, m, n, q, q ) < 0 )
        {
          MSH_NM_REF2D(b,m,n,q,q) *= -1;
          for( j = 0; j < n; j++ ) { MSH_NM_REF2D(v,min,n,q,j) *= -1; }
        }
        break;
      }

      /* check for zero on the diagonal */
      for( z= -1, i=q; i>=p; i--) 
      {
        if (MSH_NM_REF2D(b,m,n,i,i)+anorm==anorm)
        {
          z = i;
          break;
        }
      }

      if (z >= 0) 
      {
        if (z == q)/* get rid of zero on the diagonl.  where is it? */
        {
          /* lower-right corner.  clr element above it */
          msh_svd__clr_bot_supdiag_elt(b,0,z,v,m,n,min);
        }
        else
        {
          /* in the middle.  clr elt to its right */
          msh_svd__clr_top_supdiag_elt(b,z,q,u,m,n,min);
        }
      }
      else 
      {
        /* b22 has no zeroes on the diagonal or superdiagonal.
        ** Do magic rotation, leaving glop in the uppermost
        ** subdiagonal element
        */
        msh_svd__golub_kahn_svd_rot(b,p,q,v,m,n,min);
        /* get rid of the glop in the uppermost subdiagonal */
        msh_svd__clr_top_subdiag_elt(b,p,q,u,v,m,n,min);
      }
    }
#if 0
  if (iter>=max_iter)
      (void) fprintf(stderr,"svd: didn't converge after %d iterations\n", max_iter);
#endif
  }
}


/**************************************************************************
**
** SVD
**
** Given a (m x n)
** computes u (m x min)	column-orthonormal
**          w (min x min)	diagonal
**          vt (min x n)	column-orthonormal
** where min=min(m,n),
** such that a = u w vt
** 
** w is returned as a vector of length min
**
**                                                
** NOTE:  the SVD is commonly represented as U W V
**        where U is m x min and V is n x min.  This routine
**        computes the min x n transpose of V, placing the result
**        in (the memory pointed to by) parameter "vt"
**
****************************************************************************/

void 
msh_svd_decompose( int32_t m, int32_t n, const msh_nm_scalar_t *a, 
                   msh_nm_scalar_t *u, msh_nm_scalar_t* w, msh_nm_scalar_t *vt )
{
  int i, j, k, min = MSH_NM_MIN(m,n);
  msh_nm_scalar_t *g, *p, wi;

  g = MSH_NM_ALLOC2D(m,n);
  msh_svd__bidiagonalize(a,m,n,u,g,vt);

  msh_svd__bidiagonal_svd(g,m,n,u,vt);
  for( i = 0; i < min; i++ ) { w[i] = MSH_NM_REF2D(g,m,n,i,i); }

  /* insertion sort singular values.  Note that since
   * the svd algorithm produces values sorted or nearly sorted,
   * an insertion sort is efficient.
   */
  for( i = 1; i < min; i++ )
  {
    if (w[i] > w[i-1]) 
    {
      /* save w[i], and col i of u and row i of vt.  use "g" as buffer */
      wi = w[i];
      p = g;
      for( k = 0; k < m; k++ ) {*p++ = MSH_NM_REF2D(u,m,min,k,i);}
      for( k = 0; k < n; k++ ) {*p++ = MSH_NM_REF2D(vt,min,n,i,k);}
      /* slide columns over */
      for( j = i; j > 0 && wi > w[j-1]; j--) 
      {
        w[j] = w[j-1];
        for( k = 0; k < m; k++ ) { MSH_NM_REF2D(u,m,min,k,j)=MSH_NM_REF2D(u,m,min,k,j-1); }
        for( k = 0; k < n; k++ ) { MSH_NM_REF2D(vt,min,n,j,k)=MSH_NM_REF2D(vt,min,n,j-1,k); }
      }
      /* put original i stuff where we ended up */
      w[j] = wi;
      p=g;
      for( k = 0; k < m; k++ ) { MSH_NM_REF2D(u,m,min,k,j) = *p++; }
      for( k = 0; k < n; k++ ) { MSH_NM_REF2D(vt,min,n,j,k) = *p++; }
    }
  }
      
  MSH_NM_FREE(g);
}




/**************************************************************************
**
** BACKSUBSTITUTE
**
** Given the svd of a matrix A, and a vector B, solves A X = B for
** a vector X.  Takes a condition-threshold factor "eps", singular
** values less than "eps" times the largest value are considered to be
** zero.
**
** input:
**	    u (m x min)			column-orthonormal
**      w (min-element vector)	diagonal elements
**      vt (min x n)		column-orthonormal
**    where min=min(m,n),
**    such that a = u w vt, as decomposed by num_svd()
**	    b (m-element vector)	
**
** output:
** 	    x (n-element vector)
**  
****************************************************************************/

void msh_svd_backsubstitute( int32_t m, int32_t  n, 
                            const msh_nm_scalar_t *u, const msh_nm_scalar_t *w, const msh_nm_scalar_t *vt, 
                            const msh_nm_scalar_t *b, msh_nm_scalar_t *x )
{
  msh_nm_scalar_t eps = MSH_NM_EPSILON;
  const int min = MSH_NM_MIN(m,n);
  const msh_nm_scalar_t thresh = eps * w[0];	/* singular vals are sorted, w[0] is max */
  msh_nm_scalar_t *tmp = MSH_NM_ALLOC1D( min );
  int i, j, k;

  for( j = 0; j < min; j++ )
  {
    msh_nm_scalar_t s = 0;
    if( w[j] >= thresh )
    {
      for( i = 0; i < m; i++ ) { s += MSH_NM_REF2D( u, m, min, i, j ) * b[i]; }
      s /= w[j];
    }
    tmp[j] = s;
  }
  for( k=0; k < n; k++ )
  {
    msh_nm_scalar_t s = 0;
    for( j = 0; j < min; j++ ) { s += MSH_NM_REF2D( vt, min, n, j, k ) * tmp[j]; }
    x[k] = s;
  }
  MSH_NM_FREE(tmp);
}



/**************************************************************************
**
** SOLVE
**
** Given    A (m x n) 
**	    b (m-element vector)	
**
** output:
** 	    x (n-element vector)
**
** Given a matrix A, and a vector B, solves A X = B for
** a vector X.  
**
****************************************************************************/

void 
msh_svd_solve( int32_t m, int32_t n, 
               const msh_nm_scalar_t *A, const msh_nm_scalar_t *b, msh_nm_scalar_t *x )
{
  int32_t min = (m < n) ? m : n;
  size_t mem_size = (m + n + 1) * min * sizeof(msh_nm_scalar_t);
  msh_nm_scalar_t* mem = (msh_nm_scalar_t*)malloc( mem_size );
  msh_nm_scalar_t *U     = mem;
  msh_nm_scalar_t *Sigma = U + (m * min);
  msh_nm_scalar_t *Vt    = Sigma + min;
  assert(U && Sigma && Vt);

  msh_svd_decompose(m, n, A, U, Sigma, Vt);

  msh_svd_backsubstitute(m, n, U, Sigma, Vt, b, x);

  free( mem );
}

#undef MSH_NM_MIN
#undef MSH_NM_MAX
#undef MSH_NM_ALLOC2D
#undef MSH_NM_ALLOC1D
#undef MSH_NM_FREE
#undef MSH_NM_CLEAR2D
#undef MSH_NM_COPY2D
#undef MSH_NM_REF2D


//---------------------------------------------------------------------
// Eigen Decomposition 