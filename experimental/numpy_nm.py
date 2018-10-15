import numpy as np
import timeit

# print("Hello, World! SVD Example!");
# a = np.array([ [ 8.790, 9.930, 9.830, 5.450, 3.160],
#                [ 6.110, 6.910, 5.040,-0.270, 7.980],
#                [-9.150,-7.930, 4.860, 4.850, 3.010],
#                [ 9.570, 1.640, 8.830, 0.740, 5.800],
#                [-3.490, 4.020, 9.800,10.000, 4.270],
#                [ 9.840, 0.150,-8.990,-6.020,-5.310] ] )

# U, s, Vt = np.linalg.svd( a, full_matrices=True )
# S = np.zeros((6,5))
# np.fill_diagonal(S, s)
# rec_a = np.dot( np.dot( U, S ), Vt )
# print(U)
# print(S)
# print(Vt)

# A = np.array( [ [ 1,  4,  7, 10],
#                 [ 2,  5,  8, 11],
#                 [ 3,  6,  9, 12] ] )
# B = np.array( [ [1, 5], 
#                 [2, 6], 
#                 [3, 7], 
#                 [4, 8] ] )
# A = np.array( [ [1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16] ] )
# B = np.array( [ [1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16] ] )
# A = np.array( [ [1,1,1,1], [1,1,1,1], [1,1,1,1], [1,1,1,1] ] )
# B = np.array( [ [1,1,1,1], [1,1,1,1], [1,1,1,1], [1,1,1,1] ] )

# C = np.dot(A, B)
# print(C)
# print( np.allclose(a, rec_a) )
# print(a.shape, u.shape, s.shape, vt.shape)

N = 1024
A = np.arange(N*N, dtype=np.float64).reshape((N, N))
B = np.arange(N*N, dtype=np.float64).reshape((N, N))
tic = timeit.default_timer()
C = np.dot(A,B)
toc = timeit.default_timer()
print(A[0][0], A[0, N-1], A[N-1, 0], A[N-1,N-1])
print(B[0][0], B[0, N-1], B[N-1, 0], B[N-1,N-1])
print(C[0][0], C[0, N-1], C[N-1, 0], C[N-1,N-1])
print((toc-tic)*1000)
# print(C)