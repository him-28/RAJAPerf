//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-18, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-738930
//
// All rights reserved.
//
// This file is part of the RAJA Performance Suite.
//
// For details about use and distribution, please read RAJAPerf/LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
///
/// POLYBENCH_3MM kernel reference implementation:
///
/// E := A*B 
/// F := C*D 
/// G := E*F 
///
/// for (Index_type i = 0; i < NI; i++) {
///   for (Index_type j = 0; j < NJ; j++) {
///     E[i][j] = 0.0;
///     for (Index_type k = 0; k < NK; ++k) {
///       E[i][j] += A[i][k] * B[k][j];
///     }
///   }
/// } 
/// for (Index_type j = 0; j < NJ; j++) {
///   for (Index_type l = 0; l < NL; l++) {
///	F[j][l] = 0.0;
///	for (Index_type m = 0; m < NM; ++m) {
///	  F[j][l] += C[j][m] * D[m][l];
///     }
///   }
/// }
/// for (Index_type i = 0; i < NI; i++) {
///   for (Index_type l = 0; l < NL; l++) {
///     G[i][l] = 0.0;
///     for (Index_type j = 0; j < NJ; ++j) {
///	  G[i][l] += E[i][j] * F[j][l];
///     }
///   }
/// }
///

#ifndef RAJAPerf_POLYBENCH_3MM_HPP
#define RAJAPerf_POLYBENCH_3MM_HPP

#define POLYBENCH_3MM_BODY1 \
  E[j + i*nj] = 0.0;

#define POLYBENCH_3MM_BODY2 \
  E[j + i*nj] += A[k + i*nk] * B[j + k*nj];

#define POLYBENCH_3MM_BODY3 \
  F[l + j*nl] = 0.0;

#define POLYBENCH_3MM_BODY4 \
  F[l + j*nl] += C[m + j*nm] * D[l + m*nl];

#define POLYBENCH_3MM_BODY5 \
  G[l + i*nl] = 0.0;

#define POLYBENCH_3MM_BODY6 \
  G[l + i*nl] += E[j + i*nj] * F[l + j*nl];


#include "common/KernelBase.hpp"

namespace rajaperf 
{

class RunParams;

namespace polybench
{

class POLYBENCH_3MM : public KernelBase
{
public:

  POLYBENCH_3MM(const RunParams& params);

  ~POLYBENCH_3MM();


  void setUp(VariantID vid);
  void runKernel(VariantID vid); 
  void updateChecksum(VariantID vid);
  void tearDown(VariantID vid);
  void runCudaVariant(VariantID vid);
  void runOpenMPTargetVariant(VariantID vid);

private:
  Index_type m_ni;
  Index_type m_nj;
  Index_type m_nk;
  Index_type m_nl;
  Index_type m_nm;
  Index_type m_run_reps;
  Real_ptr m_A;
  Real_ptr m_B;
  Real_ptr m_C;
  Real_ptr m_D; 
  Real_ptr m_E;
  Real_ptr m_F;
  Real_ptr m_G;
};

} // end namespace polybench
} // end namespace rajaperf

#endif // closing endif for header file include guard
