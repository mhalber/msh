/*
References:
https://vismor.com/documents/network_analysis/matrix_algorithms/matrix_algorithms.php
https://en.wikipedia.org/wiki/LU_decomposition
Solomon Numerical Algorithms
David Bindel Numerical Algorithms
JAMA
*/

/* TODO(maciej): 

*/


#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#include "msh/msh_std.h"


#if defined( MSH_NM_USE_FLOAT )
typedef float msh_nm_scalar_t;
#else
typedef double msh_nm_scalar_t;
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

void msh_nm_transpose( msh_nm_mat_t *A )
{
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
3. Measure best performin algorithm with and without pivoting.
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

  for (int i = 0; i < m; i++) 
  {
    double big = 0;
    for (int j = 0; j < n; j++) 
    {
      double tmp = fabs(A[i * n + j]);
      if (tmp > big) { big = tmp; }
    }
    if (big == 0) { return false; }
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
LU_factor_nr( uint32_t m, uint32_t N, double* A, int* ind )
{
  double* pivot = malloc(sizeof(double) * N);

  for (int i = 0; i < N; i++) 
  {
    double big = 0;
    for (int j = 0; j < N; j++) 
    {
      double tmp = fabs(A[i * N + j]);
      if (tmp > big) { big = tmp; }
    }
    if (big == 0) { return false; }
    pivot[i] = 1.0 / big;
  }
  printf("%f %f %f\n", pivot[0], pivot[1], pivot[2]);

  for (int j = 0; j < N; j++)
  {
    for (int i = 0; i < j; i++) 
    {
      double sum = A[i * N + j];
      for (int k = 0; k < i; k++)
        sum -= A[i * N + k] * A[k * N + j];
      A[i * N + j] = sum;
    }

    double big = 0;
    int imax = j;
    for (int i = j; i < N; i++) {
      double sum = A[i * N + j];
      for (int k = 0; k < j; k++)
        sum -= A[i * N + k] * A[k * N + j];
      A[i * N + j] = sum;
      double tmp = pivot[i] * fabs(sum);
      if (tmp > big) {
        big = tmp;
        imax = i;
      }
    }

    if (imax != j) {
      for (int k = 0; k < N; k++)
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
      for (int i = j + 1; i < N; i++)
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
  for (int i = 0; i < m; i++) {
    piv[i] = i;
  }

  double* LUrowi;
  double* LUcolj = malloc( m * sizeof(double) );
  double* pivot = malloc(sizeof(double) * m);
  for (int i = 0; i < m; i++) 
  {
    double big = 0;
    for (int j = 0; j < n; j++) 
    {
      double tmp = fabs(A[i * n + j]);
      if (tmp > big) { big = tmp; }
    }
    if (big == 0) { return false; }
    pivot[i] = 1.0 / big;
  }
  printf("%f %f %f\n", pivot[0], pivot[1], pivot[2]);

  // Outer loop.
  for (int j = 0; j < n; j++) {

    // Make a copy of the j-th column to localize references.
    for (int i = 0; i < m; i++) {
      LUcolj[i] = A[ i*n +j ];
    }

    // Apply previous transformations.

    for (int i = 0; i < m; i++) {
      LUrowi = &A[i*n];
      // printf("TEST: %d %d\n", i, j);
      // printf("A : %f %f %f\n", LUcolj[0], LUcolj[1], LUcolj[2]);
      // printf("B : %f %f %f\n", LUrowi[0], LUrowi[1], LUrowi[2]);
  
      // Most of the time is spent in the following dot product.

      int kmax = min(i,j);
      double s = 0.0;
      for (int k = 0; k < kmax; k++) {
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
    printf("===========\n");
    int p = j;
    double pmax = 0;
    for (int i = j; i < m; i++) {
      double tmp = pivot[i] * fabs(LUcolj[i]);
      printf("  %f %f %f\n", tmp, pivot[i], fabs(LUcolj[i]));
      if( tmp > pmax ) {
        p = i;
        pmax = tmp;
      }
    }
    
    if (p != j) {
      for (int k = 0; k < n; k++) {
        double t = A[p*n+k]; A[p*n+k] = A[j*n+k]; A[j*n+k] = t;
      }
      piv[j] = p;
      pivot[p] = pivot[j];
    }
    
    nm_print_matrixd(A, 3, 3);

    // Compute multipliers.
    if (j < m && A[j*n+j] != 0.0) {
      double denom = 1.0 / A[j*n+j];
      for (int i = j+1; i < m; i++) {
        A[i*n+j] *= denom;
      }
    }
    nm_print_matrixd(A, 3, 3);
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
  for (i = 0; i < N; i += SM)
  {
    uint32_t i2lim = min( SM, N-i );
    for (j = 0; j < M; j += SM)
    {
      uint32_t j2lim = min( SM, M-j );
      for (k = 0; k < P; k += SM)
      {
        uint32_t k2lim = min( SM, P-k );
        for (i2 = 0, rres = &C[i*M+j], rmul1 = &A[i*P+k]; i2 < i2lim; ++i2, rres += M, rmul1 += P)
        {
          for (k2 = 0, rmul2 = &B[k*M+j]; k2 < k2lim; ++k2, rmul2 += M)
          {
            for (j2 = 0; j2 < j2lim; ++j2)
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
  uint64_t t1, t2;

  mmm1( N, N, N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm1(N, N, N, A, B, C);
  t2 = msh_time_now();
  printf("Time taken by %s is %12.7fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}

void measure_mmm3( const int n_iter, const int N,  double* A, double* B, double* C )
{
  uint64_t t1, t2;

  mmm3( N, N, N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm3(N, N, N, A, B, C);
  t2 = msh_time_now();
  printf("Time taken by %s is %12.7fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
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

int main( int argc, char** argv )
{
#if 0
  enum{ N_SIZES = 256 };
  int32_t sizes[N_SIZES] = { 0 };
  for( int i = 0; i < 256; ++i )
  {
    sizes[i]= i+2;
  }

  for( int32_t i = 0; i < N_SIZES; ++i )
  {
    int32_t N = sizes[i];
    double* A = malloc( N*N*sizeof(double) );
    double* B = malloc( N*N*sizeof(double) );
    double* C = malloc( N*N*sizeof(double) );
    for( int i = 0; i < N*N; ++i )
    {
      A[i] = i;
      B[i] = i;
    }

    printf("Size: %d\n", N );
    measure_mmm1( 10, N, A, B, C );
    measure_mmm3( 10, N, A, B, C );
    free(A);
    free(B);
    free(C);
  }
#endif

#if 0
  double A[12] = { 1, 2, 3, 4,  5, 6, 7, 8,  9, 10, 11, 12 };
  double B[12] = { 1, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0 };
  double C[16] = { 0 };
  mmm3( 4, 4, 3,  A, B, C );
  nm_print_matrix( &A[0], 4, 3 );
  nm_print_matrix( &B[0], 3, 4 );
  nm_print_matrix( &C[0], 4, 4 );
  printf("TEST\n");
#endif 

  // double A[9] = { 6, 1, 4, 1, 2, 7, 3, 5, -1 };
  // double A[9] = { 16, 4, 6, 21, 5, 72, 4, 9, 8 };
  double A[9] = { 3, 5, 7, 2, 8, 1, 2, 1, 3 };
  int piv[3] = {0, 1, 2};
  nm_print_matrixd( &A[0], 3, 3 );
  uint64_t t1, t2;
  t1 = msh_time_now();
  LU_factor_gauss_elimination( 3, 3, &A[0], &piv[0] );
  t2 = msh_time_now();
  double elapsed = msh_time_diff(MSHT_MILLISECONDS, t2, t1);
  printf("Factorization time: %fms\n", elapsed);
  nm_print_matrixd( &A[0], 3, 3 );
  nm_print_matrixi( &piv[0], 1, 3 );
  double b[3] = { 52, 34, 21 };
  LU_solve( 3, 3, &A[0], &piv[0], &b[0] );
  
  printf("%f %f %f\n", b[0], b[1], b[2]);

  // double B[12] = { 6, 1, 4, 3, 5, -1, 4, 5, 8, 1, 2, 7 };
  // nm_print_matrix( &B[0], 4, 3 );
  // LU_gauss_elimination( 4, 3, &B[0] );
  // nm_print_matrix( &B[0], 4, 3 );

}

