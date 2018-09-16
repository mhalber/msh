/*
This aims to be a simple implementation of SVD in c, so that I do not need to bring in bigger
guns like Eigen(C++ templates) or LAPACK(hard to use interface). I guess dlib is an alternative
for seeking other imlementations.

I'd like to compare the performance between my SVD and things like Eigen, LAPACK and numpy

We need some helpers for general matrices of arbitrary size.

The Numerical recipies seems to be indicating that important building blocks are 
Householder decomposition and QR factorization
*/

/* TODO(maciej): 
  [x] Write numpy version    - Numpy is row major
  [x] Write an Eigen version - Eigen is column major by default
  [x] Figure out column major vs row major 
  [x] Compare to dgemm and see if it is time to simply give up...
      -- dgemm_ might be a way to go. Maybe this should start as openblas wrapper.
  [x] Implement simple GEMM
  [ ] Check ulrich drepper code and make it nicer
  [ ] Check the klib guy fastest code. It looks a bit wierd
  [ ] Check codes from fast mmm
  [ ] Run tests and benchmarking to see if produced matrices are in fact correct.
  [ ] Run performance test on Eigen
  [ ] 
  [x] Make sure gemm with blocking works as intended - no segfaults + correct output.
  [ ] Compare the performance with blas dgemm
  [x] Figure out how to use dgemm // there is cblas.h
*/

// Compile command: g++ -I../../ msh_svd.c -o ../../../bin/svd_test -lopenblas
// Add -DUSE_EIGEN to compile with Eigen
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#include "msh/msh_std.h"
#include "OpenBLAS/lapacke.h"

#if defined(USE_EIGEN)
#include <iostream>
#include <eigen3/Eigen/SVD>
#endif

// SLAP - Simple Linear Algebra Package

#if defined( MSH_SLAP_USE_FLOAT )
typedef float msh_slap_scalar_t;
#else
typedef double msh_slap_scalar_t;
#endif

typedef struct msh_slap_vector
{
  size_t rows;
  msh_slap_scalar_t* data;
} msh_slap_vec_t;

typedef struct msh_slap_matrix
{
  size_t cols, rows;
  msh_slap_scalar_t* data;
} msh_slap_mat_t;

void msh_slap_mat_init( msh_slap_mat_t* mat, size_t cols, size_t rows );
void msh_slap_mat_free( msh_slap_mat_t* mat );

void msh_slap_mat_init( msh_slap_mat_t* mat, size_t cols, size_t rows )
{
  mat->cols = cols;
  mat->rows = rows;
  mat->data = (msh_slap_scalar_t*)malloc( cols*rows*sizeof(msh_slap_scalar_t) );
}

void msh_slap_matmatmul_triple_test( msh_slap_mat_t* A, msh_slap_mat_t* B, msh_slap_mat_t* C )
{
}

void msh_slap_transpose( msh_slap_mat_t *A )
{
}

void msh_slap_matmatmul_triple( const msh_slap_mat_t* A, const msh_slap_mat_t* B, 
                                msh_slap_mat_t* C )
{
}

void msh_slap_matmatmul_blocking( const msh_slap_mat_t* A, const msh_slap_mat_t* B, 
                                  msh_slap_mat_t* C )
{
}




// I definietly need to try the aliasing thing that the numerical class is mentioning, especially
// in c and cpp modes

// In this whole exploration of the topic I think we are going to start with implementing helpers,
// like general matrix multiplication etc. Then we will need to revise the code in my icp to check why
// I even need the svd. Trimesh does not use it at all. It's just LDLT and eigen decompositions.

void print_matrix( double* M, int r, int c );
void eigen_example();
extern void dgemm_(char*, char*, int*, int*,int*, double*, double*, int*, double*, int*, double*, double*, int*);
void lapack_simple_mmm_example();
void lapack_svd_example();


// From:https://www.akkadia.org/drepper/cpumemory.pdf
// Does not seem to return correct result though
// This does work, however, SM needs to be at least N not 
#include <emmintrin.h>
#define N 4
double res[N][N] __attribute__ ((aligned (64)));
double mul1[N][N] __attribute__ ((aligned (64)));
double mul2[N][N] __attribute__ ((aligned (64)));
#define SM 4
// (CLS / sizeof (double))

void test()
{
  int i, i2, j, j2, k, k2;
  double *restrict rres;
  double *restrict rmul1;
  double *restrict rmul2;
  for (i = 0; i < N; i += SM)
    for (j = 0; j < N; j += SM)
      for (k = 0; k < N; k += SM)
        for (i2 = 0, rres = &res[i][j],
            rmul1 = &mul1[i][k];
             i2 < SM;
             ++i2, rres += N, rmul1 += N)
          for (k2 = 0, rmul2 = &mul2[k][j];
               k2 < SM; ++k2, rmul2 += N)
            for (j2 = 0; j2 < SM; ++j2)
              rres[j2] += rmul1[k2] * rmul2[j2];

  // for (i = 0; i < N; i += SM)
  //   for (j = 0; j < N; j += SM)
  //     for (k = 0; k < N; k += SM)
  //       for (i2 = 0, rres = &res[i][j], rmul1 = &mul1[i][k]; i2 < SM;
  //           ++i2, rres += N, rmul1 += N)
  //       {
  //         _mm_prefetch (&rmul1[8], _MM_HINT_NTA);
  //         for (k2 = 0, rmul2 = &mul2[k][j]; k2 < SM; ++k2, rmul2 += N)
  //         {
  //           __m128d m1d = _mm_load_sd(&rmul1[k2]);
  //           m1d = _mm_unpacklo_pd(m1d, m1d);
  //           for (j2 = 0; j2 < SM; j2 += 2)
  //           {
  //             __m128d m2 = _mm_load_pd(&rmul2[j2]);
  //             __m128d r2 = _mm_load_pd(&rres[j2]);
  //             _mm_store_pd(&rres[j2],
  //                          _mm_add_pd(_mm_mul_pd(m2, m1d), r2));
  //           }
  //         }
  //       }
  print_matrix( &mul1[0][0], N, N );
  print_matrix( &mul2[0][0], N, N );
  print_matrix( &res[0][0], N, N );
}
#undef N

// Check blocking in  https://github.com/deuxbot/fast-matrix-multiplication
// Check code in      https://github.com/attractivechaos/matmul

void mmm0( const int N, const double* A, const double* B, double* C )
{
  int i, j, k;
  for(i=0; i<N; i++) 
    for(j=0; j<N; j++) 
      for(k=0; k<N; k++) 
        C[i*N+j] += A[i*N+k] * B[k*N+j];
}

// Scalar replacement
void mmm1( const int N, const double* restrict A, const double* restrict B, double* restrict C )
{
  int i, j, k;
  for(i=0; i<N; i++)
  {
    for(j=0; j<N; j++)
    {
      double sum = 0;
      for(k=0; k<N; k++)
      {
        sum += A[i*N+k] * B[k*N+j];
      }
      C[i*N+j] += sum;
    }
  }
}

// Scalar replacement + transpose of B
void mmm2( const int N, const double* restrict A, const double* restrict B, double* restrict C )
{
  int i, j, k;
  double* restrict Bt = malloc( sizeof(double) * N * N );
  for(i=0; i<N; i++)
  {
    for(j=0; j<N; j++)
    {
      Bt[ i * N + j ] = B[ j * N + i ];
    }
  }

  for(i=0; i<N; i++)
  {
    for(j=0; j<N; j++)
    {
      double sum = 0;
      for(k=0; k<N; k++)
      {
        sum += A[i*N+k] * Bt[j*N+k];
      }
      C[i*N+j] += sum;
    }
  }
  free( Bt );
}

void mmm3( const int N, const double* restrict A, const double* restrict B, double* restrict C )
{
  int32_t i, j, k, i0, j0, k0, i0lim, j0lim, k0lim, bs = 4;

  for( i = 0; i < N; i += bs )
  {
    for( j = 0; j < N; j += bs )
    {
      for( k = 0; k < N; k += bs )
      {
        // mini MM
        i0lim = ( (i + bs) > N ) ? N : (i + bs);
        j0lim = ( (j + bs) > N ) ? N : (j + bs);
        k0lim = ( (k + bs) > N ) ? N : (k + bs);

        const double* restrict Ap = &A[ i * N + k ];
        const double* restrict Bp = &B[ k * N + j ];
              double* restrict Cp = &C[ i * N + j ];
        for( i0 = i; i0 < i0lim; ++i0, Cp += N, Ap += N )
        {
          for( k0 = k, Bp = &B[ k * N + j ]; k0 < k0lim; ++k0, Bp += N )
          {
            // double Aval = Ap[k0];
            for( j0 = j; j0 < j0lim ; j0++ )
            {
              // C[ i0 * N + j0 ] += A[ i0 * N + k0 ] * B[ k0 * N + j0 ];
              Cp[ j0 ] += Ap[k0] * Bp[ j0 ];
            }
          }
        }

      }
    }
  }
}






void measure_mmm0( const int n_iter, const int N, const double* A, const double* B, double* C )
{
  uint64_t t1, t2;

  mmm0( N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm0(N, A, B, C);
  t2 = msh_time_now();
  printf("Time taken by %s is %fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}

void measure_mmm1( const int n_iter, const int N, const double* A, const double* B, double* C )
{
  uint64_t t1, t2;

  mmm1( N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm1(N, A, B, C);
  t2 = msh_time_now();
  printf("Time taken by %s is %fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}

void measure_mmm2( const int n_iter, const int N, const double* A, const double* B, double* C )
{
  uint64_t t1, t2;

  mmm2( N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm2(N, A, B, C);
  t2 = msh_time_now();
  printf("Time taken by %s is %fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}

void measure_mmm3( const int n_iter, const int N, const double* A, const double* B, double* C )
{
  uint64_t t1, t2;

  mmm3( N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm3(N, A, B, C);
  t2 = msh_time_now();
  printf("Time taken by %s is %fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}

int main( int argc, char** argv )
{
  // test();
  enum{ N = 120 };
  double A[N*N] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  double B[N*N] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  double C[N*N] = { 0 };
  measure_mmm0( 1000, N, &A[0], &B[0], &C[0] );
  measure_mmm1( 1000, N, &A[0], &B[0], &C[0] );
  measure_mmm2( 1000, N, &A[0], &B[0], &C[0] );
  measure_mmm3( 1000, N, &A[0], &B[0], &C[0] );
  // mmm3( N, &A[0], &B[0], &C[0] );
  // print_matrix( C, N, N );
  return 1;
}

void print_matrix( double* M, int rows, int cols )
{
  for( int r = 0; r < rows; ++r )
  {
    for( int c = 0; c < cols; ++c )
    {
      printf("%8.3f ", M[ r * cols + c ] );
    }
    printf("\n");
  }
  printf("\n");
}


void 
lapack_svd_example()
{
  double M[6 * 5] = {
            8.79,  6.11, -9.15,  9.57, -3.49,  9.84,
            9.93,  6.91, -7.93,  1.64,  4.02,  0.15,
            9.83,  5.04,  4.86,  8.83,  9.80, -8.99,
            3.16,  7.98,  3.01,  5.80,  4.27, -5.31
        };
  double s[6] = {0};
  double U[6][6]  = {0};
  double VT[5][5] = {0}; 
  double superb[10] = {0};

  print_matrix(&M[0], 6, 5);
  
  // Reference: https://software.intel.com/en-us/node/521150
  uint64_t t1, t2;
  t1 = msh_time_now();
  // According to numerical recipies there is faster method called 'dgesdd'(exists in OpenBLAS) and 'dbdscr'(does not exist)
  // Interesting point in RNSVD by Tom is that code in Numerical Recipies might be wrong.
  int result = LAPACKE_dgesvd( 
    LAPACK_COL_MAJOR, /* Column/Row storage*/
    'A',              /* Specifies what parts of U will be returned */
    'A',              /* Specifies what parts of V^T will be returned */
    6, 5,             /* Matrix M size */
    &M[0], 6,         /* Input Matrix M + leading dimension of M*/
    &s[0],            /* Singular value array */
    &U[0][0], 6,      /* Matrix U + leading dimension of U */
    &VT[0][0], 5,     /* Matrix V + leading dimension of V */
    &superb[0] 
  );
  t2 = msh_time_now();
  printf( "lapack svd time: %fms\n\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  print_matrix( &s[0], 1, 6 );
  print_matrix( &U[0][0], 6, 6 );
  print_matrix( &VT[0][0], 5, 5 );
}

void
lapack_simple_mmm_example()
{
  msh_slap_mat_t A, B, C;
  #define N 300
  uint64_t t1, t2;
  msh_slap_mat_init( &A, N, N );
  msh_slap_mat_init( &B, N, N );
  msh_slap_mat_init( &C, N, N );
  t1 = msh_time_now();
  for( int i = 0; i < A.cols; ++i )
  {
    for( int j = 0 ; j < A.rows; ++j )
    {
      A.data[ i*A.rows +j ] = (i*A.rows + j) + 1;
    }
  }
  for( int i = 0; i < B.cols; ++i )
  {
    for( int j = 0 ; j < B.rows; ++j )
    {
      B.data[ i*B.rows +j ] = (i*B.rows + j) + 1;
    }
  }
  for( size_t i = 0; i < C.rows; ++i  )
  {
    for( size_t j = 0; j < C.cols; ++j )
    {
      C.data[ i * C.cols + j ] = 0.0;
    }
  }
  t2 = msh_time_now();
  printf("Initialization time: %fms\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  t1 = msh_time_now();
  int lN = N;
  double alpha = 1.0;
  double beta = 0.0;
  char TA = 'N';
  char TB = 'N';
  dgemm_( &TA, &TB, &lN, &lN, &lN, &alpha, A.data, &lN, B.data, &lN, &beta, C.data, &lN );
  t2 = msh_time_now();
  printf("Multiplication time: %fms\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));
  print_matrix( C.data, C.rows, C.cols );
}


#if defined(USE_EIGEN)
void eigen_example()
{
  const int size = 3000;
  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> A;
  A.resize(size,size);
  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> B;
  B.resize(size,size);
  
  A = Eigen::Matrix<double, size, size>::Random();
  B = Eigen::Matrix<double, size, size>::Random();
  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> C;
  C.resize(size,size);
  
  C = A * B;
  // std::cout << C << "\n";

  // NOTE that Eigen documentation notes that for large matrices  BDCSVD is better (similar to lapack dgesdd)
  // double M[6 * 5] = {
  //         8.79,  6.11, -9.15,  9.57, -3.49,  9.84,
  //         9.93,  6.91, -7.93,  1.64,  4.02,  0.15,
  //         9.83,  5.04,  4.86,  8.83,  9.80, -8.99,
  //         5.45, -0.27,  4.85,  0.74, 10.00, -6.02,
  //         3.16,  7.98,  3.01,  5.80,  4.27, -5.31
  //     };

  // using namespace Eigen;
  // Matrix<double, 6, 5> eigM = Map<Matrix<double, 6, 5>>( M );
  // JacobiSVD<Matrix<double, 6, 5>> svd( eigM, ComputeFullU | ComputeFullV );
  // Matrix<double, 6, 6> U = svd.matrixU();
  // Matrix<double, 6, 5> Sigma;
  // Matrix<double, 5, 5> V = svd.matrixV();
  // Sigma.setZero();
  // Sigma.topLeftCorner(5,5) = svd.singularValues().asDiagonal();
  // // std::cout << eigM << std::endl;
  // std::cout << U << std::endl;
  // std::cout << Sigma << std::endl;
  // std::cout << V << std::endl;
  // std::cout << svd.matrixU() * Sigma * svd.matrixV().transpose() << std::endl;
}
#endif
