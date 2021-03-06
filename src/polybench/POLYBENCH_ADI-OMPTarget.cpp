
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

#include "POLYBENCH_ADI.hpp"

#include "RAJA/RAJA.hpp"

#if defined(RAJA_ENABLE_TARGET_OPENMP)

#include "common/OpenMPTargetDataUtils.hpp"

#include <iostream>

namespace rajaperf
{
namespace polybench
{

//
// Define thread block size for target execution
//
#define NUMTEAMS 256

#define POLYBENCH_ADI_DATA_SETUP_OMP_TARGET \
  int hid = omp_get_initial_device(); \
  int did = omp_get_default_device(); \
  const Index_type n = m_n; \
  const Index_type tsteps = m_tsteps; \
\
  Real_type DX,DY,DT; \
  Real_type B1,B2; \
  Real_type mul1,mul2; \
  Real_type a,b,c,d,e,f; \
\
  Real_ptr U = m_U; \
  Real_ptr V = m_V; \
  Real_ptr P = m_P; \
  Real_ptr Q = m_Q; \
\
  allocAndInitOpenMPDeviceData(U, m_U, m_n * m_n, did, hid); \
  allocAndInitOpenMPDeviceData(V, m_V, m_n * m_n, did, hid); \
  allocAndInitOpenMPDeviceData(P, m_P, m_n * m_n, did, hid); \
  allocAndInitOpenMPDeviceData(Q, m_Q, m_n * m_n, did, hid); 

#define POLYBENCH_ADI_DATA_TEARDOWN_OMP_TARGET \
  getOpenMPDeviceData(m_U, U, m_n * m_n, hid, did); \
  deallocOpenMPDeviceData(U, did); \
  deallocOpenMPDeviceData(V, did); \
  deallocOpenMPDeviceData(P, did); \
  deallocOpenMPDeviceData(Q, did); 


void POLYBENCH_ADI::runOpenMPTargetVariant(VariantID vid)
{
  const Index_type run_reps = getRunReps();

  if ( vid == Base_OpenMPTarget ) {

    POLYBENCH_ADI_DATA_SETUP_OMP_TARGET;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      POLYBENCH_ADI_BODY1;

      for (Index_type t = 1; t <= tsteps; ++t) { 

        #pragma omp target is_device_ptr(P,Q,U,V) device( did )
        #pragma omp teams distribute parallel for num_teams(NUMTEAMS) schedule(static, 1)
        for (Index_type i = 1; i < n-1; ++i) {
          NEW_POLYBENCH_ADI_BODY2;
          for (Index_type j = 1; j < n-1; ++j) {
            NEW_POLYBENCH_ADI_BODY3;
          }  
          NEW_POLYBENCH_ADI_BODY4;
          for (Index_type k = n-2; k >= 1; --k) {
            NEW_POLYBENCH_ADI_BODY5;
          }  
        }

        #pragma omp target is_device_ptr(P,Q,U,V) device( did )
        #pragma omp teams distribute parallel for num_teams(NUMTEAMS) schedule(static, 1)
        for (Index_type i = 1; i < n-1; ++i) {
          NEW_POLYBENCH_ADI_BODY6;
          for (Index_type j = 1; j < n-1; ++j) {
            NEW_POLYBENCH_ADI_BODY7;
          }
          NEW_POLYBENCH_ADI_BODY8;
          for (Index_type k = n-2; k >= 1; --k) {
            NEW_POLYBENCH_ADI_BODY9;
          }
        }

      } // tsteps

    } // run_reps  
    stopTimer(); 

    POLYBENCH_ADI_DATA_TEARDOWN_OMP_TARGET;  

  } else if ( vid == RAJA_OpenMPTarget ) {

    POLYBENCH_ADI_DATA_SETUP_OMP_TARGET;

    using EXEC_POL =
      RAJA::KernelPolicy<
        RAJA::statement::For<0, RAJA::omp_target_parallel_for_exec<NUMTEAMS>,
          RAJA::statement::Lambda<0>,
          RAJA::statement::For<1, RAJA::seq_exec,
            RAJA::statement::Lambda<1>
          >,
          RAJA::statement::Lambda<2>,
          RAJA::statement::For<2, RAJA::seq_exec,
            RAJA::statement::Lambda<3>
          >
        >
      >;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      POLYBENCH_ADI_BODY1;

      for (Index_type t = 1; t <= tsteps; ++t) {

        RAJA::kernel<EXEC_POL>(
          RAJA::make_tuple(RAJA::RangeSegment{1, n-1},
                           RAJA::RangeSegment{1, n-1},
                           RAJA::RangeStrideSegment{n-2, 0, -1}),

          [=] (Index_type i, Index_type /*j*/, Index_type /*k*/) {
            NEW_POLYBENCH_ADI_BODY2;
          },
          [=] (Index_type i, Index_type j, Index_type /*k*/) {
            NEW_POLYBENCH_ADI_BODY3;
          },
          [=] (Index_type i, Index_type /*j*/, Index_type /*k*/) {
            NEW_POLYBENCH_ADI_BODY4;
          },
          [=] (Index_type i, Index_type /*j*/, Index_type k) {
            NEW_POLYBENCH_ADI_BODY5;
          }
        );

        RAJA::kernel<EXEC_POL>(
          RAJA::make_tuple(RAJA::RangeSegment{1, n-1},
                           RAJA::RangeSegment{1, n-1},
                           RAJA::RangeStrideSegment{n-2, 0, -1}),

          [=] (Index_type i, Index_type /*j*/, Index_type /*k*/) {
            NEW_POLYBENCH_ADI_BODY6;
          },
          [=] (Index_type i, Index_type j, Index_type /*k*/) {
            NEW_POLYBENCH_ADI_BODY7;
          },
          [=] (Index_type i, Index_type /*j*/, Index_type /*k*/) {
            NEW_POLYBENCH_ADI_BODY8;
          },
          [=] (Index_type i, Index_type /*j*/, Index_type k) {
            NEW_POLYBENCH_ADI_BODY9;
          }
        );

      } // tsteps

    } // run_reps
    stopTimer();

    POLYBENCH_ADI_DATA_TEARDOWN_OMP_TARGET;

  } else {
     std::cout << "\n  POLYBENCH_ADI : Unknown OMP Target variant id = " << vid << std::endl;
  }
}    

} // end namespace polybench
} // end namespace rajaperf

#endif  // RAJA_ENABLE_TARGET_OPENMP

