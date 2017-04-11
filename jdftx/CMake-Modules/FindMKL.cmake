if(NOT MKL_PATH)
	set(MKL_PATH "/opt/intel/mkl")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8) #64-bit
	set(MKL_LIB_PATH ${MKL_PATH}/lib/intel64)
	find_library(MKL_INTERFACE_LIBRARY NAMES mkl_intel_lp64 PATHS ${MKL_LIB_PATH})
else() #32-bit
	set(MKL_LIB_PATH ${MKL_PATH}/lib/ia32)
	find_library(MKL_INTERFACE_LIBRARY NAMES mkl_intel PATHS ${MKL_LIB_PATH})
endif()

if(NOT ThreadedBLAS)
	find_library(MKL_THREAD_LIBRARY NAMES mkl_sequential PATHS ${MKL_LIB_PATH})
	set(MKL_OPENMP_LIBRARY)
elseif((${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
	find_library(MKL_THREAD_LIBRARY NAMES mkl_gnu_thread PATHS ${MKL_LIB_PATH})
	set(MKL_OPENMP_LIBRARY gomp)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
	find_library(MKL_THREAD_LIBRARY NAMES mkl_intel_thread PATHS ${MKL_LIB_PATH})
	set(MKL_OPENMP_LIBRARY iomp5)
else()
	message(WARNING "Unknown compiler: edit FindMKL.cmake to include appropriate OpenMP link rules.")
endif()

find_library(MKL_CORE_LIBRARY NAMES mkl_core PATHS ${MKL_LIB_PATH})
if(NOT ForceFFTW)
	find_path(FFTW3_INCLUDE_DIR NAMES fftw3_mkl.h PATHS ${MKL_PATH}/include/fftw)
endif()
find_path(MKL_INCLUDE_DIR NAMES mkl_dfti.h PATHS ${MKL_PATH}/include)


if(MKL_INTERFACE_LIBRARY AND MKL_THREAD_LIBRARY AND MKL_CORE_LIBRARY AND (ForceFFTW OR FFTW3_INCLUDE_DIR) AND MKL_INCLUDE_DIR)
	set(MKL_FOUND TRUE)
	set(MKL_LIBRARIES ${MKL_INTERFACE_LIBRARY} ${MKL_THREAD_LIBRARY} ${MKL_CORE_LIBRARY} ${MKL_OPENMP_LIBRARY})
else()
	set(MKL_FOUND FALSE)
endif()


if(MKL_FOUND)
	if(NOT MKL_FIND_QUIETLY)
		message(STATUS "Found MKL: ${MKL_LIBRARIES}")
	endif()
else()
	if(MKL_FIND_REQUIRED)
		if(NOT MKL_INTERFACE_LIBRARY)
			message(FATAL_ERROR "Could not find the MKL interface-layer library (Add -D MKL_PATH=<path> to the cmake commandline for a non-standard installation)")
		endif()
		if(NOT MKL_THREAD_LIBRARY)
			message(FATAL_ERROR "Could not find the MKL sequential thread-layer library (Add -D MKL_PATH=<path> to the cmake commandline for a non-standard installation)")
		endif()
		if(NOT MKL_CORE_LIBRARY)
			message(FATAL_ERROR "Could not find the MKL core library (Add -D MKL_PATH=<path> to the cmake commandline for a non-standard installation)")
		endif()
		if((NOT ForceFFTW) AND (NOT FFTW3_INCLUDE_DIR))
			message(FATAL_ERROR "Could not find the MKL FFTW3 interface library (Add -D MKL_PATH=<path> to the cmake commandline for a non-standard installation)")
		endif()
		if(NOT MKL_INCLUDE_DIR)
			message(FATAL_ERROR "Could not find the MKL headers (Add -D MKL_PATH=<path> to the cmake commandline for a non-standard installation)")
		endif()
	endif()
endif()

