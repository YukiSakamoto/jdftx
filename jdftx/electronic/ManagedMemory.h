/*-------------------------------------------------------------------
Copyright 2011 Ravishankar Sundararaman

This file is part of JDFTx.

JDFTx is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

JDFTx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JDFTx.  If not, see <http://www.gnu.org/licenses/>.
-------------------------------------------------------------------*/

#ifndef JDFTX_ELECTRONIC_MANAGEDMEMORY_H
#define JDFTX_ELECTRONIC_MANAGEDMEMORY_H

#include <electronic/common.h>

//! Base class for managed-memory objects (that could potentially live on GPUs as well)
class ManagedMemory
{
public:

	//! @brief Return a pointer to the actual data
	//! Return a CPU pointer to the actual data, will move data from GPU to CPU if necessary
	//! In GPU mode, care must be taken when calling this from multiple cpu threads
	//! Only the "GPU Owner" thread may call this when the data is actually on the GPU.
	//! Ideally call once from main thread to get data onto the cpu before launching other cpu threads
	complex* data();

	//! @brief Return a const pointer to the actual data
	//! Return a CPU pointer to the actual data, will move data from GPU to CPU if necessary
	//! In GPU mode, care must be taken when calling this from multiple cpu threads
	//! Only the "GPU Owner" thread may call this when the data is actually on the GPU.
	//! Ideally call once from main thread to get data onto the cpu before launching other cpu threads
	const complex* data() const;

	size_t nData() const { return nElements; } //!< number of data points

	bool isOnGpu() const { return onGpu; } //!< Check where the data is (for #ifdef simplicity exposed even when no GPU_ENABLED)

	#ifdef GPU_ENABLED
	complex* dataGpu(); //!< Get a gpu data pointer (must be called from GPU owner thread)
	const complex* dataGpu() const; //!< Get a const gpu data pointer (must be called from GPU owner thread)
	#endif

	//Utilities to automatically select "preferred" data i.e. GPU when it is enabled, CPU otherwise
	//This eases the overload of CPU/GPU functions based on complex vs complex
	#ifdef GPU_ENABLED
	inline complex* dataPref() { return dataGpu(); }
	inline const complex* dataPref() const { return dataGpu(); }
	#else
	inline complex* dataPref() { return data(); }
	inline const complex* dataPref() const { return data(); }
	#endif

	void write(const char *fname) const; //!< binary-write to a file
	void writea(const char *fname) const; //!< binary-append to a file
	void write(FILE *filep) const; //!< binary-write toa stream
	void read(const char *fname); //!< binary read from a file
	void read(FILE *filep); //!< binary read from a stream
	void read_real(const char *fname); //!< binary read real-part from file, setting imaginary parts to 0
	void read_real(FILE *filep); //!< binary read real-part from stream, setting imaginary parts to 0
	void write_real(const char *fname) const; //!< binary write real-parts to file
	void write_real(FILE *filep) const; //!< binary write real-parts to stream
	void zero(); //!< set all elements to zero
	
protected:
	void memFree(); //!< Do the job of destructor
	void memInit(size_t nElem, bool ontoGpu=false); //!< Do the job of the constructor

	void memMove(ManagedMemory&&); //!< Steal the other object's data (used for move constructors/assignment)
	ManagedMemory(size_t nElements=0, bool onGpu=false); //!< Construct, optionally with data allocation
	~ManagedMemory();

private:
	size_t nElements;
	complex* c; //!< Actual data storage
	bool onGpu; //!< For reduced #ifdef's, this flag is retained even in the absence of gpu support
	#ifdef GPU_ENABLED
	void toCpu(); //!< move data to the CPU
	void toGpu(); //!< move data to the GPU
	#endif
};

//Some common elementwise / vector-like operations
void memcpy(ManagedMemory&, const ManagedMemory&); //!< copy entire object over
void scale(double alpha, ManagedMemory& y); //! scale y *= alpha
void scale(complex alpha, ManagedMemory& y); //! scale y *= alpha
void axpy(complex alpha, const ManagedMemory& x, ManagedMemory& y); //!< standard blas y += alpha*x
double nrm2(const ManagedMemory&); //!< 2-norm, pretending it is a vector
complex dotc(const ManagedMemory& a, const ManagedMemory& b); //!< return a^H b

#endif // JDFTX_ELECTRONIC_MANAGEDMEMORY_H