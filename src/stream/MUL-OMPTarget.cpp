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

#include "MUL.hpp"

#include "RAJA/RAJA.hpp"

#if defined(RAJA_ENABLE_TARGET_OPENMP)

#include "common/OpenMPTargetDataUtils.hpp"

#include <iostream>

namespace rajaperf 
{
namespace stream
{

//
// Define thread block size for target execution
//
#define NUMTEAMS 256

#define MUL_DATA_SETUP_OMP_TARGET \
  int hid = omp_get_initial_device(); \
  int did = omp_get_default_device(); \
\
  Real_ptr b; \
  Real_ptr c; \
  Real_type alpha = m_alpha; \
\
  allocAndInitOpenMPDeviceData(b, m_b, iend, did, hid); \
  allocAndInitOpenMPDeviceData(c, m_c, iend, did, hid);

#define MUL_DATA_TEARDOWN_OMP_TARGET \
  getOpenMPDeviceData(m_b, b, iend, hid, did); \
  deallocOpenMPDeviceData(b, did); \
  deallocOpenMPDeviceData(c, did);

void MUL::runOpenMPTargetVariant(VariantID vid)
{
  const Index_type run_reps = getRunReps();
  const Index_type ibegin = 0;
  const Index_type iend = getRunSize();

  if ( vid == Base_OpenMPTarget ) {

    MUL_DATA_SETUP_OMP_TARGET;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      #pragma omp target is_device_ptr(b, c) device( did )
      #pragma omp teams distribute parallel for num_teams(NUMTEAMS) schedule(static, 1)
      for (Index_type i = ibegin; i < iend; ++i ) {
        MUL_BODY;
      }

    }
    stopTimer();

    MUL_DATA_TEARDOWN_OMP_TARGET;

  } else if ( vid == RAJA_OpenMPTarget ) {

    MUL_DATA_SETUP_OMP_TARGET;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      RAJA::forall<RAJA::omp_target_parallel_for_exec<NUMTEAMS>>(
        RAJA::RangeSegment(ibegin, iend), [=](Index_type i) {
        MUL_BODY;
      });

    }
    stopTimer();

    MUL_DATA_TEARDOWN_OMP_TARGET;

  } else {
     std::cout << "\n  MUL : Unknown OMP Target variant id = " << vid << std::endl;
  }
}

} // end namespace stream
} // end namespace rajaperf

#endif  // RAJA_ENABLE_TARGET_OPENMP
