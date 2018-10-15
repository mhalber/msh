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
  [ ] Seems like drapper version is best. I will keep the simpler one like mmm2 if it works for small matrices
*/

// Compile command: g++ -I../../ msh_svd.c -o ../../../bin/svd_test -lopenblas
// Add -DUSE_EIGEN to compile with Eigen
#include <stdlib.h>
#include <malloc.h>
#define MSH_STD_INCLUDE_LIBC_HEADERS
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
extern void dgemm_(char*, char*, int*, int*,int*, double*, double*, int*, double*, int*, double*, double*, int*);
void lapack_simple_mmm_example();
void lapack_svd_example();


// Check : https://github.com/ldfaiztt/cs267-dgemm/tree/master/src 
// Check : https://github.com/ytsutano/dgemm-goto-in-c // not super fast
// Ckeck : https://github.com/cappachu/dgemm // bit faster, but requires sse4


#include <stdio.h>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <emmintrin.h>


#if !defined(BLOCK_SIZE)
#define BLOCK_SIZE 256
#endif

// TODO: Just do 4x4 matrix multiply here
void sse_4x4 (int lda, int K, double* A, double* B, double* C) {
    /* Performs Matrix Multiplication on 4x4 block
     * using SSE intrinsics 
     * load, update, store*/
  // A
  __m128d A_0X_A_1X, A_2X_A_3X;
  // B
  __m128d B_X0, B_X1, B_X2, B_X3;
  // C 
  __m128d C_00_C_10, C_20_C_30,
          C_01_C_11, C_21_C_31,
          C_02_C_12, C_22_C_32,
          C_03_C_13, C_23_C_33;

  // LOAD --------
  // load unaligned
  C_00_C_10 = _mm_loadu_pd(C              );
  C_20_C_30 = _mm_loadu_pd(C           + 2);
  C_01_C_11 = _mm_loadu_pd(C + lda        );
  C_21_C_31 = _mm_loadu_pd(C + lda     + 2);
  C_02_C_12 = _mm_loadu_pd(C + (2*lda)    );
  C_22_C_32 = _mm_loadu_pd(C + (2*lda) + 2);
  C_03_C_13 = _mm_loadu_pd(C + (3*lda)    );
  C_23_C_33 = _mm_loadu_pd(C + (3*lda) + 2);

  for (int k = 0; k < K; ++k) {
    // load aligned
    A_0X_A_1X = _mm_load_pd(A);
    A_2X_A_3X = _mm_load_pd(A+2);
    A += 4;
      
    // load unaligned
    B_X0 = _mm_loaddup_pd(B);
    B_X1 = _mm_loaddup_pd(B+1);
    B_X2 = _mm_loaddup_pd(B+2);
    B_X3 = _mm_loaddup_pd(B+3);
    B += 4;
    // UPDATE ---------
    // C := C + A*B
    C_00_C_10 = _mm_add_pd(C_00_C_10, _mm_mul_pd(A_0X_A_1X, B_X0));
    C_20_C_30 = _mm_add_pd(C_20_C_30, _mm_mul_pd(A_2X_A_3X, B_X0));
    C_01_C_11 = _mm_add_pd(C_01_C_11, _mm_mul_pd(A_0X_A_1X, B_X1));
    C_21_C_31 = _mm_add_pd(C_21_C_31, _mm_mul_pd(A_2X_A_3X, B_X1));
    C_02_C_12 = _mm_add_pd(C_02_C_12, _mm_mul_pd(A_0X_A_1X, B_X2));
    C_22_C_32 = _mm_add_pd(C_22_C_32, _mm_mul_pd(A_2X_A_3X, B_X2));
    C_03_C_13 = _mm_add_pd(C_03_C_13, _mm_mul_pd(A_0X_A_1X, B_X3));
    C_23_C_33 = _mm_add_pd(C_23_C_33, _mm_mul_pd(A_2X_A_3X, B_X3));
  }

  // STORE -------
  _mm_storeu_pd(C              , C_00_C_10);
  _mm_storeu_pd(C           + 2, C_20_C_30);
  _mm_storeu_pd(C + lda        , C_01_C_11);
  _mm_storeu_pd(C + lda     + 2, C_21_C_31);
  _mm_storeu_pd(C + (2*lda)    , C_02_C_12);
  _mm_storeu_pd(C + (2*lda) + 2, C_22_C_32);
  _mm_storeu_pd(C + (3*lda)    , C_03_C_13);
  _mm_storeu_pd(C + (3*lda) + 2, C_23_C_33);
}


void naive_helper (int lda, int K, double* A, double* B, double* C, int i, int j) {
    double cij = C[j*lda + i];
    for (int k = 0; k < K; ++k) {
        cij += A[k*lda + i] * B[j*lda + k];
    }
    C[j*lda + i] = cij;
}


void do_block (int lda, int M, int N, int K, double* A, double* B, double* C)
{
  // largest multiple of 4 less than M
  int M4_max = (M>>2) << 2;
  // largest multiple of 4 less than N
  int N4_max = (N>>2) << 2;
  
  // pack and align data from A 
  double AA[M4_max*K]; // under allocate
  for(int m=0; m < M4_max; m+=4) {
      double *dst = &AA[m*K];
      double *src = A + m;
      for (int k = 0; k < K; ++k) {
          *dst     = *src;
          *(dst+1) = *(src+1);
          *(dst+2) = *(src+2);
          *(dst+3) = *(src+3);
          dst += 4;
          src += lda;
      }
  }
  // pack and align data from B
  double BB[N4_max*K]; // under allocate
  for(int n=0; n < N4_max; n+=4){
      double *dst = &BB[n*K];
      double *src_0 = B + n*lda;
      double *src_1 = src_0 + lda; 
      double *src_2 = src_1 + lda; 
      double *src_3 = src_2 + lda;
      for (int k = 0; k < K; ++k) {
          *dst++ = *src_0++;
          *dst++ = *src_1++;
          *dst++ = *src_2++;
          *dst++ = *src_3++;
      }
  }

  // compute 4x4's using SSE intrinsics 
  for (int i = 0; i < M4_max; i+=4){
    for (int j = 0; j < N4_max; j+=4){
        sse_4x4(lda, K, &AA[i*K], &BB[j*K], &C[j*lda + i]);
    }
  }
  // compute remaining cells using naive dgemm
  // horizontal sliver
  if(M4_max!=M){
      for (int i=M4_max; i < M; ++i)
          for (int tmp=0; tmp < N; ++tmp) 
              naive_helper(lda, K, A, B, C, i, tmp);
  }
  // vertical sliver + bottom right corner 
  if(N4_max!=N){
      for (int j=N4_max; j < N; ++j)
          for (int tmp=0; tmp < M4_max; ++tmp) 
              naive_helper(lda, K, A, B, C, tmp, j);
  }
}



void square_dgemm (int lda, double* A, double* B, double* C)
{
  /* For each block-row of A */ 
  for (int i = 0; i < lda; i += BLOCK_SIZE)
    /* For each block-column of B */
    for (int j = 0; j < lda; j += BLOCK_SIZE)
      /* Accumulate block dgemms into block of C */
      for (int k = 0; k < lda; k += BLOCK_SIZE)
      {
        /* Correct block dimensions if block "goes off edge of" the matrix */
        int M = min (BLOCK_SIZE, lda-i);
        int N = min (BLOCK_SIZE, lda-j);
        int K = min (BLOCK_SIZE, lda-k);

        /* Perform individual block dgemm */
        do_block(lda, M, N, K, A + i + k*lda, B + k + j*lda, C + i + j*lda);
      }
}



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

// Based on: https://www.akkadia.org/drepper/cpumemory.pdf
void mmm3( const int N, double* restrict A, double* restrict B, double* restrict C )
{
    uint32_t i, i2, j, j2, k, k2;
    uint32_t SM = 64;
    double *restrict rres;
    double *restrict rmul1;
    double *restrict rmul2;
    for (i = 0; i < N; i += SM)
    {
      uint32_t i2lim = min( SM, N-i );
      for (j = 0; j < N; j += SM)
      {
        uint32_t j2lim = min( SM, N-j );
        for (k = 0; k < N; k += SM)
        {
          uint32_t k2lim = min( SM, N-k );
          for (i2 = 0, rres = &C[i*N+j], rmul1 = &A[i*N+k]; i2 < i2lim; ++i2, rres += N, rmul1 += N)
          {
            for (k2 = 0, rmul2 = &B[k*N+j]; k2 < k2lim; ++k2, rmul2 += N)
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



// Based on : https://github.com/deuxbot/fast-matrix-multiplication
void mmm4( const int N, const double* restrict A, const double* restrict B, double* restrict C )
{
  int i, j, k, ii, jj, kk, Aik, bs = N;
  
  for(ii = 0; ii < N; ii += bs)
    for(kk = 0; kk < N; kk += bs)
      for(jj = 0; jj < N; jj += bs)
        for(i = ii; i < min(N, ii+bs); i++)
          for(k = kk; k < min(N, kk+bs); k++)
          {
            Aik = A[N*i+k];
            for(j = jj; j < min(N, jj+bs); j++)
              C[N*i+j] += Aik * B[N*k+j];
          }
}


void measure_mmm0( const int n_iter, int N, double* A, double* B, double* C )
{
  uint64_t t1, t2;

  // mmm0( N, A, B, C );
  square_dgemm(N, A, B, C);
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  // for( int i = 0; i < n_iter; ++i ) mmm0(N, A, B, C);
  for( int i = 0; i < n_iter; ++i ) square_dgemm(N, A, B, C);
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

void measure_mmm3( const int n_iter, const int N,  double* A, double* B, double* C )
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

void measure_mmm4( const int n_iter, const int N, const double* A, const double* B, double* C )
{
  uint64_t t1, t2;

  mmm4( N, A, B, C );
  for( int i = 0; i < N*N; ++i )
  {
    C[i] = 0;
  }
  t1 = msh_time_now();
  for( int i = 0; i < n_iter; ++i ) mmm4(N, A, B, C);
  t2 = msh_time_now();
  printf("Time taken by %s is %fms\n", __FUNCTION__, msh_time_diff(MSHT_MILLISECONDS, t2, t1)/n_iter );
}


int main( int argc, char** argv )
{
  enum{ N = 1024 };
  double* A = malloc( N*N*sizeof(double) );
  double* B = malloc( N*N*sizeof(double) );
  double* C = malloc( N*N*sizeof(double) );
  for( int i = 0; i < N*N; ++i )
  {
    A[i] = i;
    B[i] = i;
  }
  measure_mmm0( 1, N, A, B, C );
  // measure_mmm1( 1, N, &A[0], &B[0], &C[0] );
  // // measure_mmm2( 100, N, &A[0], &B[0], &C[0] );
  measure_mmm3( 1, N, A, B, C );
  measure_mmm4( 1, N, A, B, C );
  // // measure_mmm5( 100, N, A, B, C );
  // // measure_test( 100, N );

  // mmm4( N, A, B, C );
  // square_dgemm(N, A, B, C);
  printf("%f %f %f %f\n", C[0], C[N-1], C[N*(N-1)], C[N*N-1]);
  lapack_simple_mmm_example();
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
  #define M 512
  uint64_t t1, t2;
  msh_slap_mat_init( &A, M, M );
  msh_slap_mat_init( &B, M, M );
  msh_slap_mat_init( &C, M, M );
  t1 = msh_time_now();
  for( int i = 0; i < A.cols; ++i )
  {
    for( int j = 0 ; j < A.rows; ++j )
    {
      A.data[ i*A.rows +j ] = (i*A.rows + j);
    }
  }
  for( int i = 0; i < B.cols; ++i )
  {
    for( int j = 0 ; j < B.rows; ++j )
    {
      B.data[ i*B.rows +j ] = (i*B.rows + j);
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
  int lN = M;
  double alpha = 1.0;
  double beta = 0.0;
  char TA = 'N';
  char TB = 'N';
  dgemm_( &TA, &TB, &lN, &lN, &lN, &alpha, A.data, &lN, B.data, &lN, &beta, C.data, &lN );
  t2 = msh_time_now();
  printf("Multiplication time: %fms\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));
  printf("%f %f %f %f\n", C.data[0], C.data[M-1], C.data[M*(M-1)], C.data[M*M-1]);
  // print_matrix( C.data, C.rows, C.cols );
}

