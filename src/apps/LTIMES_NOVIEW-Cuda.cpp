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

#include "LTIMES_NOVIEW.hpp"

#include "RAJA/RAJA.hpp"

#if defined(RAJA_ENABLE_CUDA)

#include "common/CudaDataUtils.hpp"

#include <iostream>

namespace rajaperf 
{
namespace apps
{


#define LTIMES_NOVIEW_DATA_SETUP_CUDA \
  Real_ptr phidat; \
  Real_ptr elldat; \
  Real_ptr psidat; \
\
  Index_type num_d = m_num_d; \
  Index_type num_z = m_num_z; \
  Index_type num_g = m_num_g; \
  Index_type num_m = m_num_m; \
\
  allocAndInitCudaDeviceData(phidat, m_phidat, m_philen); \
  allocAndInitCudaDeviceData(elldat, m_elldat, m_elllen); \
  allocAndInitCudaDeviceData(psidat, m_psidat, m_psilen);

#define LTIMES_NOVIEW_DATA_TEARDOWN_CUDA \
  getCudaDeviceData(m_phidat, phidat, m_philen); \
  deallocCudaDeviceData(phidat); \
  deallocCudaDeviceData(elldat); \
  deallocCudaDeviceData(psidat);

__global__ void ltimes_noview(Real_ptr phidat, Real_ptr elldat, Real_ptr psidat,
                              Index_type num_d, Index_type num_g, Index_type num_m)
{
   Index_type m = threadIdx.x;
   Index_type g = blockIdx.y;
   Index_type z = blockIdx.z;

   for (Index_type d = 0; d < num_d; ++d ) {
     LTIMES_NOVIEW_BODY;
   }
}


void LTIMES_NOVIEW::runCudaVariant(VariantID vid)
{
  const Index_type run_reps = getRunReps();

  if ( vid == Base_CUDA ) {

    LTIMES_NOVIEW_DATA_SETUP_CUDA;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      dim3 nthreads_per_block(num_m, 1, 1);
      dim3 nblocks(1, num_g, num_z);

      ltimes_noview<<<nblocks, nthreads_per_block>>>(phidat, elldat, psidat,
                                                     num_d, num_g, num_m);  

    }
    stopTimer();

    LTIMES_NOVIEW_DATA_TEARDOWN_CUDA;

  } else if ( vid == RAJA_CUDA ) {

    LTIMES_NOVIEW_DATA_SETUP_CUDA;

    using EXEC_POL = 
      RAJA::KernelPolicy<
        RAJA::statement::CudaKernelAsync<
          RAJA::statement::For<1, RAJA::cuda_block_exec,      //z
            RAJA::statement::For<2, RAJA::cuda_block_exec,    //g
              RAJA::statement::For<3, RAJA::cuda_thread_exec, //m
                RAJA::statement::For<0, RAJA::seq_exec,       //d
                  RAJA::statement::Lambda<0>
                >
              >
            >
          >
        >
      >;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      RAJA::kernel<EXEC_POL>( RAJA::make_tuple(RAJA::RangeSegment(0, num_d),
                                               RAJA::RangeSegment(0, num_z),
                                               RAJA::RangeSegment(0, num_g),
                                               RAJA::RangeSegment(0, num_m)),
        [=] __device__ (Index_type d, Index_type z, Index_type g, Index_type m) {
        LTIMES_NOVIEW_BODY;
      });

    }
    stopTimer();

    LTIMES_NOVIEW_DATA_TEARDOWN_CUDA;

  } else {
     std::cout << "\n LTIMES_NOVIEW : Unknown Cuda variant id = " << vid << std::endl;
  }
}

} // end namespace apps
} // end namespace rajaperf

#endif  // RAJA_ENABLE_CUDA
