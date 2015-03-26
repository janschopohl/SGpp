/*
 * StreamingOCLParameters.hpp
 *
 *  Created on: Mar 18, 2015
 *      Author: pfandedd
 */

#include <CL/cl.h>

#ifndef STREAMING_OCL_DEVICE_TYPE
//options: CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_GPU, ...
//this define requires the include of <CL/cl.h>
#define STREAMING_OCL_DEVICE_TYPE CL_DEVICE_TYPE_ALL
#endif

//#ifndef STREAMING_OCL_ENABLE_OPTIMIZATIONS
//#define STREAMING_OCL_ENABLE_OPTIMIZATIONS true
//#endif
//
//#ifndef STREAMING_OCL_USE_LOCAL_MEMORY
//#define STREAMING_OCL_USE_LOCAL_MEMORY true
//#endif
//
//#ifndef STREAMING_OCL_LOCAL_SIZE
//#define STREAMING_OCL_LOCAL_SIZE 256
//#endif
//
//#ifndef STREAMING_OCL_MAX_DIM_UNROLL
//#define STREAMING_OCL_MAX_DIM_UNROLL 10
//#endif
//
//#ifndef STREAMING_OCL_INTERNAL_PRECISION
//#define STREAMING_OCL_INTERNAL_PRECISION double
//#endif
