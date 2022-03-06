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

// This is template library for matrix multiplication. A x B = C
// template arguments
//			- T                       - Data type of the matrix elements
//			- t_rowsA                 - Number of rows of A
//			- t_colsA                 - Number of cols of A
//			                            (therefore number of rows of B)
//			- t_colsB                 - Number of cols of B
//			- DOT_VEC_SIZE            - Specifies the number of DSPs to be used in a single iteration.
//			                            It must be a factor of the value of t_colsA.
//			                            When you increase this value, you can achieve better latency 
//			                            but at the cost of using additional DSPs and other resources.
//			                            If you keep this value low, you will have higher latency but lower resource utilization.
//			- BLOCK_SIZE              - Number of elements that will be read from A at a time. 
//			                            Default is same as DOT_VEC_SIZE
//			                            but if the bandwidth of A is less, then you can reduce this number.
//			                            It has to be a factor of DOT_VEC_SIZE though.
//			- RUNNING_SUM_MULT_L      - Deep optimization parameter used for Fmax improvement.       
//			                            Default value is 1
//			                            This value is 2 or higher if number of B columns is less than the 
//			                            accumulation latency (latency to accumulate the partial products of elements of A row and B col).
//			                            Try higher values for better Fmax. 
//			                            The maximum value of RUNNING_SUM_MULT_L is num_iter_per_elem_L(=COLSA/DOT_VEC_SIZE). 
//			                            If it goes higher than the max limit, then higher Fmax is achieved at the cost of more 
//			                            clock cycles and resources which might result in increased overall latency.                                                                                       
//					       
// function arguments
//			- A[][] -  A input 2 dimensional matrix
//			- B[][] -  B input 2 dimensional matrix
//			- C[][] -  C output 2 dimensional matrix
//
//Tip - You can safely ignore compiler warnings like default template arguments for a function template are a C++11 extension [-Wc++11-extensions]. 
template<class T, int t_rowsA, int t_colsA, int t_colsB, int DOT_VEC_SIZE = t_colsA, int BLOCK_SIZE = DOT_VEC_SIZE, int RUNNING_SUM_MULT_L = 1>
void matrix_multiply(T A_local[t_rowsA][t_colsA], T B_local[t_colsA][t_colsB], T C_local[t_colsA][t_colsB]) {
	const int COLSA = t_colsA;
	const int ROWSA = t_rowsA;
	const int COLSB = t_colsB;
	const int ROWSB = COLSA;
	const int ROWSC = ROWSA;
	const int COLSC = COLSB;

	const int num_iter_per_elem_L = (COLSA / DOT_VEC_SIZE);

	T A_local_regs[DOT_VEC_SIZE], A_local_regs_stable[DOT_VEC_SIZE];

	//partial_products represents a memory with depth COLSC
	T running_sums_for_col[COLSC * RUNNING_SUM_MULT_L];

	int i, j, s; // make sure these variables are within bounds of the corresponding comparisons in code below.
	i = -1;
	j = 0;
	s = num_iter_per_elem_L - 1;
	for (int k = 0; k < num_iter_per_elem_L * COLSC * ROWSC + COLSC; ++k) {
		bool last_s_itr = s == num_iter_per_elem_L - 1;
		// load the cache

		// latch A_local_regs
		if (j == 0) {
#pragma unroll
			for (int d = 0; d < DOT_VEC_SIZE; d++) {
				A_local_regs_stable[d] = A_local_regs[d];
			}
		}
#pragma unroll
		for (int d = 0; d < DOT_VEC_SIZE - BLOCK_SIZE; ++d) {
			A_local_regs[d] = A_local_regs[d + BLOCK_SIZE];
		}
		// Start loading a new full row of A, once end of an output row is approaching
		// BLOCKS elements of the row are loaded concurrently
#pragma unroll
		for (int d = 0; d < BLOCK_SIZE; ++d) {
			T val = {};
			if (j >= COLSC - DOT_VEC_SIZE / BLOCK_SIZE) {
				//When padding is zero ((BLOCKS + BLOCKS_PADDING) * VEC_BLOCK_RATIO) == DOT_VECTOR_SIZE
				int rowA, colA;
				if ((s + 1) * DOT_VEC_SIZE == COLSA) {
					rowA = i + 1;
					colA = (j - COLSC + DOT_VEC_SIZE / BLOCK_SIZE) * BLOCK_SIZE + d;
				} else {
					rowA = i;
					colA = (s + 1) * DOT_VEC_SIZE + (j - COLSC + DOT_VEC_SIZE / BLOCK_SIZE) * BLOCK_SIZE + d;
				}
				val = A_local[rowA][colA];
			}
			A_local_regs[d + DOT_VEC_SIZE - BLOCK_SIZE] = val;
		}
		// compute partial products
		T running_sum = 0.0;
#pragma unroll
		for (int d = 0; d < DOT_VEC_SIZE; ++d) {
			running_sum += A_local_regs_stable[d] * B_local[s * DOT_VEC_SIZE + d][j];
		}
		T sum = running_sums_for_col[RUNNING_SUM_MULT_L * COLSC - 1] = (s < RUNNING_SUM_MULT_L ? (T) 0.0 : running_sums_for_col[RUNNING_SUM_MULT_L * COLSC - 1]) + running_sum;
		T final_sum = sum;
#pragma unroll
		for (int d = 1; d < RUNNING_SUM_MULT_L; d++) {
			final_sum += running_sums_for_col[(RUNNING_SUM_MULT_L - d) * COLSC - 1];
		}
		// rotate running sums
		T tmp = running_sums_for_col[0];
#pragma unroll
		for (int d = 0; d < RUNNING_SUM_MULT_L * COLSC - 1; d++) {
			running_sums_for_col[d] = running_sums_for_col[d + 1];
		}
		// only rotate if we need to
		// this should remove the false dependency on the
		// result of the partial addition
		if (num_iter_per_elem_L > RUNNING_SUM_MULT_L) {
			running_sums_for_col[RUNNING_SUM_MULT_L * COLSC - 1] = tmp;
		} else {
			running_sums_for_col[RUNNING_SUM_MULT_L * COLSC - 1] = (T) 0.0; // MATRIX_ELEMENT_ZERO;
		}

		if (last_s_itr && i >= 0) {
			(C_local)[i][j] = final_sum;
		}
		if (j == COLSC - 1) {
			j = 0;
			if (last_s_itr) {
				s = 0;
				i++;
			} else {
				s++;
			}
		} else {
			j++;
		}
	}
}

