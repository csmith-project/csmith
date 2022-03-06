/*  Copyright 1992-2021 Intel Corporation.                                 */
/*                                                                         */
/*  This software and the related documents are Intel copyrighted          */
/*  materials, and your use of them is governed by the express license     */
/*  under which they were provided to you ("License"). Unless the License  */
/*  provides otherwise, you may not use, modify, copy, publish,            */
/*  distribute, disclose or transmit this software or the related          */
/*  documents without Intel's prior written permission.                    */
/*                                                                         */
/*  This software and the related documents are provided as is, with no    */
/*  express or implied warranties, other than those that are expressly     */
/*  stated in the License.                                                 */

#ifndef IHC_CHOLESKY_DECOMPOSE
#define IHC_CHOLESKY_DECOMPOSE

#include "HLS/extendedmath.h"
#include "HLS/math.h"
#include "HLS/ac_complex.h"
#include <type_traits>

namespace ihc {
namespace cholesky {

/**
 * Cholesky Decomposition (real)
 * It is recommended to use fp-relaxed for less latency and higher throughput.
 * @tparam FP_T: The floating point type.
 * @tparam VEC_SIZE_PWR: The dot product and matrix access is vectorized with width VEC_SIZE =
 * 2^VEC_SIZE_PWR.
 * @tparam MATRIX_SIZE: The maximum matrix size of this decomposition core. It should be a multiple
 * of the VEC_SIZE.
 * @tparam INNER_SAFELEN: Dependency for index calculation and partial dot product storage. There
 * needs to be at least so many iterations before changing k. A default of 16 is given, which should
 * be suitable in most cases.
 * @tparam OUTER_SAFELEN_OVERWRITE: Although we guess the required safelen and how it grows with the
 * design size, it still depends on how the user interacts with it, the clock target, device family
 * etc. This parameter allow the user to overwrite the outer safelen. If OUTER_SAFELEN_OVERWRITE =
 * -1, the default safelen would be used, which is tuned for single precision dot product on A10
 * with L_iter implemented on local memory and having vectorized access.
 * If II 1 is not achieved, increase the value of OUTER_SAFELEN_OVERWRITE until II 1
 * is achieved.
 * @param A_input: The A matrix input.
 * @param L_iter: The L matrix to be iterated on, with both read and write accesses, and will
 * contain the factored L matrix at the end of execution. It should be vectorized with appropriate
 * width. It does not need to be initialized.
 */

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN = 16,
          int OUTER_SAFELEN_OVERWRITE = -1>
void cholesky_decompose_real(FP_T *A_input, FP_T *L_iter, short n);

/**
 * Cholesky Decomposition (real)
 * It is recommended to use fp-relaxed for less latency and higher throughput.
 * @tparam FP_T: The floating point type.
 * @tparam VEC_SIZE_PWR: The dot product and matrix access is vectorized with width VEC_SIZE =
 * 2^VEC_SIZE_PWR.
 * @tparam MATRIX_SIZE: The maximum matrix size of this decomposition core. It should be a multiple
 * of the VEC_SIZE.
 * @tparam INNER_SAFELEN: Dependency for index calculation and partial dot product storage. There
 * needs to be at least so many iterations before changing k. A default of 16 is given, which should
 * be suitable in most cases.
 * @tparam OUTER_SAFELEN_OVERWRITE: Although we guess the required safelen and how it grows with the
 * design size, it still depends on how the user interacts with it, the clock target, device family
 * etc. This parameter allow the user to overwrite the outer safelen. If OUTER_SAFELEN_OVERWRITE =
 * -1, the default safelen would be used, which is tuned for single precision dot product on A10
 * with L_iter implemented on local memory and having vectorized access.
 * If II 1 is not achieved, increase the value of OUTER_SAFELEN_OVERWRITE until II 1
 * is achieved.
 * @param A_input: The A matrix input.
 * @param L_output: The L matrix output. Only the final result is written to it.
 * @param L_iter: This is the L matrix for the algorithm to iterate on. It should be vectorized with
 * appropriate width. It does not need to be initialized.
 */

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN = 16,
          int OUTER_SAFELEN_OVERWRITE = -1>
void cholesky_decompose_real(FP_T *A_input, FP_T *L_output, FP_T *L_iter, short n);

/**
 * Cholesky Decomposition (complex)
 * It is recommended to use fp-relaxed for less latency and higher throughput.
 * @tparam FP_T: The floating point type.
 * @tparam VEC_SIZE_PWR: The dot product and matrix access is vectorized with width VEC_SIZE =
 * 2^VEC_SIZE_PWR.
 * @tparam MATRIX_SIZE: The maximum matrix size of this decomposition core. It should be a multiple
 * of the VEC_SIZE.
 * @tparam INNER_SAFELEN: Dependency for index calculation and partial dot product storage. There
 * needs to be at least so many iterations before changing k. A default of 16 is given, which should
 * be suitable in most cases.
 * @tparam OUTER_SAFELEN_OVERWRITE: Although we guess the required safelen and how it grows with the
 * design size, it still depends on how the user interacts with it, the clock target, device family
 * etc. This parameter allow the user to overwrite the outer safelen. If OUTER_SAFELEN_OVERWRITE =
 * -1, the default safelen would be used, which is tuned for single precision dot product on A10
 * with L_iter implemented on local memory and having vectorized access.
 * If II 1 is not achieved, increase the value of OUTER_SAFELEN_OVERWRITE until II 1
 * is achieved.
 * @param A_input: The A matrix input.
 * @param L_iter: The L matrix to be iterated on, with both read and write accesses, and will
 * contain the factored L matrix at the end of execution. It should be vectorized with appropriate
 * width. It does not need to be initialized.
 */

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN = 16,
          int OUTER_SAFELEN_OVERWRITE = -1>
void cholesky_decompose_complex(ac_complex<FP_T> *A_input, ac_complex<FP_T> *L_iter, short n);

/**
 * Cholesky Decomposition (complex)
 * It is recommended to use fp-relaxed for less latency and higher throughput.
 * @tparam FP_T: The floating point type.
 * @tparam VEC_SIZE_PWR: The dot product and matrix access is vectorized with width VEC_SIZE =
 * 2^VEC_SIZE_PWR.
 * @tparam MATRIX_SIZE: The maximum matrix size of this decomposition core. It should be a multiple
 * of the VEC_SIZE.
 * @tparam INNER_SAFELEN: Dependency for index calculation and partial dot product storage. There
 * needs to be at least so many iterations before changing k. A default of 16 is given, which should
 * be suitable in most cases.
 * @tparam OUTER_SAFELEN_OVERWRITE: Although we guess the required safelen and how it grows with the
 * design size, it still depends on how the user interacts with it, the clock target, device family
 * etc. This parameter allow the user to overwrite the outer safelen. If OUTER_SAFELEN_OVERWRITE =
 * -1, the default safelen would be used, which is tuned for single precision dot product on A10
 * with L_iter implemented on local memory and having vectorized access.
 * If II 1 is not achieved, increase the value of OUTER_SAFELEN_OVERWRITE until II 1
 * is achieved.
 * @param A_input: The A matrix input.
 * @param L_output: The L matrix output. Only the final result is written to it.
 * @param L_iter: This is the L matrix for the algorithm to iterate on. It should be vectorized with
 * appropriate width. It does not need to be initialized.
 */

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN = 16,
          int OUTER_SAFELEN_OVERWRITE = -1>
void cholesky_decompose_complex(ac_complex<FP_T> *A_input, ac_complex<FP_T> *L_output,
                                ac_complex<FP_T> *L_iter, short n);

/*
 * Cholesky Decomposition (real)
 * Internal implementation
 * @tparam SEPARATE_L_ITER: Decide if the algorithm sperates the final output matrix from the input
 * matrix that it operates on.
 */
template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN = 16,
          int OUTER_SAFELEN_OVERWRITE = -1, bool SEPARATE_L_ITER = true>
void cholesky_decompose_real_internal(FP_T *A_input, FP_T *L_extern, FP_T *L_iter, short n);

/*
 * Cholesky Decomposition (complex)
 * Internal implementation
 * @tparam SEPARATE_L_ITER: Decide if the algorithm sperates the final output matrix from the input
 * matrix that it operates on.
 */
template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN = 16,
          int OUTER_SAFELEN_OVERWRITE = -1, bool SEPARATE_L_ITER = true>
void cholesky_decompose_complex_internal(ac_complex<FP_T> *A_input, ac_complex<FP_T> *L_extern,
                                         ac_complex<FP_T> *L_iter, short n);

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN,
          int OUTER_SAFELEN_OVERWRITE>
void cholesky_decompose_real(FP_T *A_input, FP_T *L_iter, short n) {
  cholesky_decompose_real_internal<FP_T, MATRIX_SIZE, VEC_SIZE_PWR, INNER_SAFELEN,
                                   OUTER_SAFELEN_OVERWRITE, false>(A_input, L_iter, nullptr, n);
}

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN,
          int OUTER_SAFELEN_OVERWRITE>
void cholesky_decompose_real(FP_T *A_input, FP_T *L_output, FP_T *L_iter, short n) {
  cholesky_decompose_real_internal<FP_T, MATRIX_SIZE, VEC_SIZE_PWR, INNER_SAFELEN,
                                   OUTER_SAFELEN_OVERWRITE, true>(A_input, L_output, L_iter, n);
}

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN,
          int OUTER_SAFELEN_OVERWRITE>
void cholesky_decompose_complex(ac_complex<FP_T> *A_input, ac_complex<FP_T> *L_iter, short n) {
  cholesky_decompose_complex_internal<FP_T, MATRIX_SIZE, VEC_SIZE_PWR, INNER_SAFELEN,
                                      OUTER_SAFELEN_OVERWRITE, false>(A_input, L_iter, nullptr, n);
}

template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN,
          int OUTER_SAFELEN_OVERWRITE>
void cholesky_decompose_complex(ac_complex<FP_T> *A_input, ac_complex<FP_T> *L_output,
                                ac_complex<FP_T> *L_iter, short n) {
  cholesky_decompose_complex_internal<FP_T, MATRIX_SIZE, VEC_SIZE_PWR, INNER_SAFELEN,
                                      OUTER_SAFELEN_OVERWRITE, true>(A_input, L_output, L_iter, n);
}

template <class DATA_T, int VEC_SIZE> struct VectorSlice {
  // Hold a piece of vectorized slice of data
  DATA_T data[VEC_SIZE];
};

template <class FP_T, int VEC_SIZE>
FP_T VectorSliceDot(VectorSlice<FP_T, VEC_SIZE> sa, VectorSlice<FP_T, VEC_SIZE> sb) {
  FP_T res = 0.0f;
#pragma unroll
  for (short i = 0; i < VEC_SIZE; i++) {
    res += sa.data[i] * sb.data[i];
  }
  return res;
}

template <class FP_T, int VEC_SIZE>
ac_complex<FP_T> VectorSliceConjDot(VectorSlice<ac_complex<FP_T>, VEC_SIZE> sa,
                                    VectorSlice<ac_complex<FP_T>, VEC_SIZE> sb) {
  ac_complex<FP_T> res = 0.0f;
#pragma unroll
  for (short i = 0; i < VEC_SIZE; i++) {
    res += sa.data[i] * sb.data[i].conj();
  }
  return res;
}

template <class FP_T> FP_T sqrt_internal(FP_T x) { return sqrt(x); }

template <class FP_T> FP_T rsqrt_internal(FP_T x) { return rsqrt(x); }

template <> float sqrt_internal(float x) { return sqrtf(x); }

template <> float rsqrt_internal(float x) { return rsqrtf(x); }

/**
 * Internal Implementation
 */
template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN,
          int OUTER_SAFELEN_OVERWRITE, bool SEPARATE_L_ITER>
void cholesky_decompose_real_internal(FP_T *A_input, FP_T *L_extern, FP_T *L_iter, short n) {

  // Heuristically calculate architecture parameters.
  // INNER_SAFELEN: Dependency for index calculation and partial dot product storage. There needs to
  // be at least so many iterations before changing k. OUTER_SAFELEN: Dependency between consecutive
  // columns. There needs to be at least so many iterations before changing i. VEC_SIZE: The width
  // of dot product and matrix access.
  constexpr int VEC_SIZE = 1 << VEC_SIZE_PWR;
  constexpr int OUTER_SAFELEN =
      OUTER_SAFELEN_OVERWRITE == -1 ? 36 + 4 * VEC_SIZE_PWR : OUTER_SAFELEN_OVERWRITE;
  // k loop is inside i loop. Does not make sense if INNER_SAFELEN > OUTER_SAFELEN
  static_assert(
      OUTER_SAFELEN > INNER_SAFELEN,
      "The safelen for the outer loop should be larger or equal to that for the inner loop.");
  static_assert(
      MATRIX_SIZE % VEC_SIZE == 0,
      "Matrix Size should be a multiple of the vector size. You can zero-pad it if necessary");
  static_assert(std::is_same<float, FP_T>::value || std::is_same<double, FP_T>::value,
                "Only support float and double");

  FP_T root_inv = 0.0f;                                  // Hold the inverse of the root.
  hls_memory FP_T partial_dot[MATRIX_SIZE];              // Hold the partial dot product value.
  hls_register VectorSlice<FP_T, VEC_SIZE> cached_slice; // Cached slice.

  // Buffers for pipelining the loop index calculation.
  hls_register short next_j_end_buff[INNER_SAFELEN] = {0};
  hls_register short next_k_buff[INNER_SAFELEN] = {0};
  hls_register short next_i_buff[INNER_SAFELEN] = {0};

  short j_end = n - 1;
  if (OUTER_SAFELEN > n) {
    j_end = OUTER_SAFELEN;
  }
  // Initial range of j. Note that dummy iteration needs to be inserted if n < OUTER_SAFELEN;
  // j's interval: [i to j_end], closed on both sides.

  short i = 0, j = 0, k = 0;
  // Loop order: i - k - j;
  // i: Column number
  // k: Bank number over vectorized dot product chunks.
  //    For example, when calculation dot product of two rows with length 16 and VEC_SIZE = 4, k=0
  //    goes from 0 to 3, k=1 goes from 4-7 etc.
  // j: Row number.

  // ivdep: We promise the compiler that there won't be memory access to the same element in a
  // particular array for at least "safelen" cycles, so compiler is free to schedule the load and
  // store at most "safelen" cycles apart. The user should target ii=1. However, in the exploration
  // phase you might need to iterate and adjust SAFELEN accordingly to achieve ii=1.

#pragma ivdep array(L_iter) safelen(OUTER_SAFELEN)
#pragma ivdep array(partial_dot) safelen(INNER_SAFELEN)
#pragma speculated_iterations 9 // Helps slightly with fmax.
#pragma ii 1 // Might be helpful for forcing ii=1 at higher clock target
  while (i < n) {
    short k_lim = i / VEC_SIZE; // k interval: [0 - i/VEC_SIZE] closed on both sides.

    bool compute = (j < n);
    // if not dummy, do the work
    // j>=n are dummy iterations, need to predicate operations with side effects.
    bool at_diagonal = (j == i);
    bool first_k = (k == 0);
    bool last_k = (k == k_lim);

    // Load a chunk from local matrix L.
    VectorSlice<FP_T, VEC_SIZE> current_slice;
    if (compute) {
      if (SEPARATE_L_ITER) {
        // If we have a separate local copy of L, we iterate on it.
#pragma unroll
        for (short k_unroll = 0; k_unroll < VEC_SIZE; k_unroll++) {
          current_slice.data[k_unroll] = L_iter[j * MATRIX_SIZE + k * VEC_SIZE + k_unroll];
          // L[j][k*VEC_SIZE+k_unroll]
        }
      } else {
        // If we do not have another copy of L, we directly iterate on the external L.
#pragma unroll
        for (short k_unroll = 0; k_unroll < VEC_SIZE; k_unroll++) {
          current_slice.data[k_unroll] = L_extern[j * MATRIX_SIZE + k * VEC_SIZE + k_unroll];
          // L[j][k*VEC_SIZE+k_unroll]
        }
      }
      // For data outside of the triangular pattern, assign them to zero.
#pragma unroll
      for (short k_unroll = 0; k_unroll < VEC_SIZE; k_unroll++) {
        if (k * VEC_SIZE + k_unroll >= i) {
          current_slice.data[k_unroll] = 0.0f;
        }
      }
    }

    // Hints to reg for fmax.
    // Can be applied to more index related signals for pushing fmax on larger designs.
    bool at_diagonal_reg0 = __fpga_reg(at_diagonal);
    bool at_diagonal_reg1 = __fpga_reg(at_diagonal_reg0);
    bool at_diagonal_reg2 = __fpga_reg(at_diagonal_reg1);

    // Cache the row corresponding to the diagonal element. This is reused multiple times.
    if (at_diagonal_reg2) {
      cached_slice = current_slice;
    }

    if (compute) {
      FP_T previous_dot;
      if (first_k) {
        previous_dot = 0.0f;
      } else {
        previous_dot = partial_dot[j];
      }
      FP_T current_dot =
          VectorSliceDot(current_slice, cached_slice); // current_slice.dot_with(cached_slice);
      // Use fpga_reg to prevent the final adder to be absorbed into the dot product.
      // Achieve lower latency --> shorter wait.
      partial_dot[j] = __fpga_reg(current_dot) + previous_dot;

      if (last_k) {
        // Dot product of the entire row has been finished.
        FP_T a_ij = A_input[j * MATRIX_SIZE + i];
        FP_T l_tmp = a_ij - partial_dot[j];
        FP_T res;
        if (at_diagonal) {
          res = sqrt_internal(l_tmp);
          root_inv = rsqrt_internal(l_tmp);
        } else {
          res = l_tmp * root_inv;
        }
        if (SEPARATE_L_ITER) {
          L_iter[j * MATRIX_SIZE + i] = __fpga_reg(res);
        }
        L_extern[j * MATRIX_SIZE + i] = __fpga_reg(res);
      }
    }

    // Index manipulation for what should happen after the current j runs to j_end.
    // We calculate how many dummy iterations we need here.
    short next_i = i;
    short next_k = k + 1;
    if (k == k_lim) {
      next_i = i + 1;
      next_k = 0;
    }
    short next_k_lim = next_i / VEC_SIZE;
    short next_j_end = n - 1;
    short next_j_end_dummy = next_i + INNER_SAFELEN;
    if (next_j_end_dummy > next_j_end) {
      next_j_end = next_j_end_dummy;
    }
    short next_i2 = next_i + 1;
    short next_tripcount = (n - next_i2);
    if (INNER_SAFELEN > next_tripcount) {
      next_tripcount = INNER_SAFELEN;
    }
    next_tripcount = next_tripcount * k_lim;
    next_j_end_dummy = short(next_i2 + OUTER_SAFELEN) - next_tripcount;
    if (next_j_end_dummy > next_j_end && next_k == next_k_lim) {
      next_j_end = next_j_end_dummy;
    }
    // Only in last k

    // j reached j_end, update i,j,k index and j_end.
#pragma unroll
    for (short i = 0; i < INNER_SAFELEN - 1; i++) {
      next_j_end_buff[i] = next_j_end_buff[i + 1];
      next_k_buff[i] = next_k_buff[i + 1];
      next_i_buff[i] = next_i_buff[i + 1];
    }

    next_j_end_buff[INNER_SAFELEN - 1] = next_j_end;
    next_k_buff[INNER_SAFELEN - 1] = next_k;
    next_i_buff[INNER_SAFELEN - 1] = next_i;

    if (j == j_end) {
      k = next_k_buff[0];
      i = next_i_buff[0];
      j = i;
      j_end = next_j_end_buff[0];
    } else {
      j++;
    }
  }
}

/**
 * Internal Implementation
 */
template <class FP_T, int MATRIX_SIZE, int VEC_SIZE_PWR, int INNER_SAFELEN,
          int OUTER_SAFELEN_OVERWRITE, bool SEPARATE_L_ITER>
void cholesky_decompose_complex_internal(ac_complex<FP_T> *A_input, ac_complex<FP_T> *L_extern,
                                         ac_complex<FP_T> *L_iter, short n) {

  // Heuristically calculate architecture parameters.
  // INNER_SAFELEN: Dependency for index calculation and partial dot product storage. There needs to
  // be at least so many iterations before changing k. OUTER_SAFELEN: Dependency between consecutive
  // columns. There needs to be at least so many iterations before changing i. VEC_SIZE: The width
  // of dot product and matrix access.
  constexpr int VEC_SIZE = 1 << VEC_SIZE_PWR;
  constexpr int OUTER_SAFELEN =
      OUTER_SAFELEN_OVERWRITE == -1 ? 40 + 4 * VEC_SIZE_PWR : OUTER_SAFELEN_OVERWRITE;
  // k loop is inside i loop. Does not make sense if INNER_SAFELEN > OUTER_SAFELEN
  static_assert(
      OUTER_SAFELEN > INNER_SAFELEN,
      "The safelen for the outer loop should be larger or equal to that for the inner loop.");
  static_assert(
      MATRIX_SIZE % VEC_SIZE == 0,
      "Matrix Size should be a multiple of the vector size. You can zero-pad it if necessary");
  static_assert(std::is_same<float, FP_T>::value || std::is_same<double, FP_T>::value,
                "Only support float and double");

  typedef ac_complex<FP_T> FP_COMPLEX_T;

  FP_T root_inv = 0.0f;
  hls_memory FP_COMPLEX_T partial_dot[MATRIX_SIZE];
  hls_register VectorSlice<FP_COMPLEX_T, VEC_SIZE> cached_slice;

  // Buffers for pipelining the loop index calculation.
  hls_register short next_j_end_buff[INNER_SAFELEN] = {0};
  hls_register short next_k_buff[INNER_SAFELEN] = {0};
  hls_register short next_i_buff[INNER_SAFELEN] = {0};

  short j_end = n - 1;
  if (OUTER_SAFELEN > n) {
    j_end = OUTER_SAFELEN;
  }
  // Initial range of j. Note that dummy iteration needs to be inserted if n < OUTER_SAFELEN;
  // j's interval: [i to j_end], closed on both sides.

  short i = 0, j = 0, k = 0;
  // Loop order: i - k - j;
  // i: Column number
  // k: Bank number over vectorized dot product chunks.
  //    For example, when calculation dot product of two rows with length 16 and VEC_SIZE = 4, k=0
  //    goes from 0 to 3, k=1 goes from 4-7 etc.
  // j: Row number.

  // ivdep: We promise the compiler that there won't be memory access to the same element in a
  // particular array for at least "safelen" cycles, so compiler is free to schedule the load and
  // store at most "safelen" cycles apart. The user should target ii=1. However, in the exploration
  // phase you might need to iterate and adjust SAFELEN accordingly to achieve ii=1.

#pragma ivdep array(L_iter) safelen(OUTER_SAFELEN)
#pragma ivdep array(partial_dot) safelen(INNER_SAFELEN)
#pragma speculated_iterations 9
#pragma ii 1 // Might be helpful for forcing ii=1 at higher clock target
  while (i < n) {
    short k_lim = i / VEC_SIZE; // k interval: [0 - i/VEC_SIZE] closed on both sides.

    bool compute = (j < n);
    // if not dummy, do the work
    // j>=n are dummy iterations, need to predicate operations with side effects.
    bool at_diagonal = (j == i);
    bool first_k = (k == 0);
    bool last_k = (k == k_lim);

    // Load a chunk from local matrix L.
    VectorSlice<FP_COMPLEX_T, VEC_SIZE> current_slice;
    if (compute) {
      if (SEPARATE_L_ITER) {
        // If we have a separate local copy of L, we iterate on it.
#pragma unroll
        for (short k_unroll = 0; k_unroll < VEC_SIZE; k_unroll++) {
          current_slice.data[k_unroll] = L_iter[j * MATRIX_SIZE + k * VEC_SIZE + k_unroll];
          // L[j][k*VEC_SIZE+k_unroll]
        }
      } else {
        // If we do not have another copy of L, we directly iterate on the external L.
#pragma unroll
        for (short k_unroll = 0; k_unroll < VEC_SIZE; k_unroll++) {
          current_slice.data[k_unroll] = L_extern[j * MATRIX_SIZE + k * VEC_SIZE + k_unroll];
          // L[j][k*VEC_SIZE+k_unroll]
        }
      }
      // For data outside of the triangular pattern, assign them to zero.
#pragma unroll
      for (short k_unroll = 0; k_unroll < VEC_SIZE; k_unroll++) {
        if (k * VEC_SIZE + k_unroll >= i) {
          current_slice.data[k_unroll] = 0.0f;
        }
      }
    }

    bool at_diagonal_reg0 = __fpga_reg(at_diagonal);
    bool at_diagonal_reg1 = __fpga_reg(at_diagonal_reg0);
    bool at_diagonal_reg2 = __fpga_reg(at_diagonal_reg1);

    if (at_diagonal_reg2) {
      cached_slice = current_slice;
    }

    if (compute) {
      FP_COMPLEX_T previous_dot;
      if (first_k) {
        previous_dot = 0.0f;
      } else {
        previous_dot = partial_dot[j];
      }
      FP_COMPLEX_T current_dot = VectorSliceConjDot(current_slice, cached_slice);
      // Use fpga_reg to prevent the final adder to be absorbed into the dot product.
      // So the latency on this path will be less.
      FP_COMPLEX_T current_dot_reg;
      current_dot_reg.set_r(__fpga_reg(current_dot.real()));
      current_dot_reg.set_i(__fpga_reg(current_dot.imag()));

      partial_dot[j] = current_dot_reg + previous_dot;

      if (last_k) {
        // Dot product of the entire row has been finished.
        FP_COMPLEX_T a_ij = A_input[j * MATRIX_SIZE + i];
        FP_COMPLEX_T l_tmp = a_ij - partial_dot[j];
        FP_COMPLEX_T res;
        if (at_diagonal) {
          res.set_r(sqrt_internal(l_tmp.real()));
          res.set_i(0.0f);
          root_inv = rsqrt_internal(l_tmp.real());
          // The diagonal element should not have imaginary component.
        } else {
          res = l_tmp * root_inv;
        }
        FP_COMPLEX_T res_reg;
        res_reg.set_r(__fpga_reg(res.real()));
        res_reg.set_i(__fpga_reg(res.imag()));
        if (SEPARATE_L_ITER) {
          L_iter[j * MATRIX_SIZE + i] = res_reg;
        }
        L_extern[j * MATRIX_SIZE + i] = res_reg;
      }
    }

    // Index manipulation for what should happen after the current j runs to j_end.
    // We calculate how many dummy iterations we need here.
    short next_i = i;
    short next_k = k + 1;
    if (k == k_lim) {
      next_i = i + 1;
      next_k = 0;
    }
    short next_k_lim = next_i / VEC_SIZE;
    short next_j_end = n - 1;
    short next_j_end_dummy = next_i + INNER_SAFELEN;
    if (next_j_end_dummy > next_j_end) {
      next_j_end = next_j_end_dummy;
    }
    short next_i2 = next_i + 1;
    short next_tripcount = (n - next_i2);
    if (INNER_SAFELEN > next_tripcount) {
      next_tripcount = INNER_SAFELEN;
    }
    next_tripcount = next_tripcount * k_lim;
    next_j_end_dummy = short(next_i2 + OUTER_SAFELEN) - next_tripcount;
    if (next_j_end_dummy > next_j_end && next_k == next_k_lim) {
      next_j_end = next_j_end_dummy;
    }
    // Only in last k

    // j reached j_end, update i,j,k index and j_end.
#pragma unroll
    for (short i = 0; i < INNER_SAFELEN - 1; i++) {
      next_j_end_buff[i] = next_j_end_buff[i + 1];
      next_k_buff[i] = next_k_buff[i + 1];
      next_i_buff[i] = next_i_buff[i + 1];
    }

    next_j_end_buff[INNER_SAFELEN - 1] = next_j_end;
    next_k_buff[INNER_SAFELEN - 1] = next_k;
    next_i_buff[INNER_SAFELEN - 1] = next_i;

    if (j == j_end) {
      k = next_k_buff[0];
      i = next_i_buff[0];
      j = i;
      j_end = next_j_end_buff[0];
    } else {
      j++;
    }
  }
}

} // namespace cholesky
} // namespace ihc
#endif
