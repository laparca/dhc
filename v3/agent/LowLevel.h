/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
******************************************************************************/
 
/*!
	@file LowLevel.h
	
	@brief Declaration of low-level CUDA routines
	
	This file, as well as LowLevel.cpp, was originally written by Andrew Zonenberg for anamigo.com. Permission to distribute
	this code under BSD license has been secured:
	
	[16:46:21] Andrew Zonenberg: Also, I am using my CUDA driver API wrapper classes in the RPISEC cracker
	[16:46:29] … Any objections to me distributing that code under a bsd license?
	[16:46:39] … If not I can reimplement, I just want to avoid the hassle if possible
	[16:47:28] Jason Sanchez: Nope.  That's a service library and you're free to hand it out.
 */
 
#ifndef LowLevel_h
#define LowLevel_h

#ifdef CUDA_ENABLED

/*!
	@brief Driver initialization.
	
	One instance of this class must be created before any other CUDA calls are made, in order to initialize the CUDA driver.
	The object may then be destroyed.
 */
class DriverInterface
{
public:
	DriverInterface();

	static int GetDriverVersion();
	
	static int GetDeviceCount();
};

/*!
	@brief A CUDA device.
	
	This class may be used in any thread, regardless of if the thread is bound to the device in question.
 */
class Device
{
public:
	Device(int iDevice);
	
	//Attribute querying
	int GetAttribute(CUdevice_attribute attrib);
	
	/*!
	@brief Gets the maximum total number of threads allowed per block.
	*/
	int GetMaxThreadsPerBlock()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK); }
	
	/*!
	@brief Gets the maximum number of threads allowed per block in the X direction.
	*/
	int GetMaxBlockX()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X); }
	
	/*!
	@brief Gets the maximum number of threads allowed per block in the Y direction.
	*/
	int GetMaxBlockY()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y); }
	
	/*!
	@brief Gets the maximum number of threads allowed per block in the Z direction.
	*/
	int GetMaxBlockZ()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z); }
	
	/*!
	@brief Gets the maximum number of blocks allowed per grid in the X direction.
	*/
	int GetMaxGridX()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X); }
	
	/*!
	@brief Gets the maximum number of blocks allowed per grid in the Y direction.
	*/
	int GetMaxGridY()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y); }
	
	/*!
	@brief Gets the maximum number of blocks allowed per grid in the Z direction.
	*/
	int GetMaxGridZ()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z); }
	
	/*!
	@brief Gets the maximum amount of shared memory usable by a single thread block.
	*/
	int GetMaxShmem()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK); }
	
	/*!
	@brief Gets the total constant memory available on this device.
	*/
	int GetMaxCmem()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY); }
	
	/*!
	@brief Gets the warp size of the device (SIMD width)
	*/
	int GetWarpSize()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_WARP_SIZE); }
	
	/*!
	@brief Gets the maximum alignment which may be used with cudaMallocPitch().
	*/
	int GetMaxPitch()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_PITCH); }
	
	/*!
	@brief Gets the total number of registers available per thread block.
	*/
	int GetMaxRegisters()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK); }
	
	/*!
	@brief Gets the clock rate of the device in KHz.
	*/
	int GetClockRate()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_CLOCK_RATE); }
	
	/*!
	@brief Gets the alignment, in bytes, which must be used for texture memory/
	*/
	int GetTextureAlignment()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT); }
	
	/*!
	@brief Determines if the device supports computation during a memory bus transfer.
	*/
	bool GetGpuOverlap()
	{ return (GetAttribute(CU_DEVICE_ATTRIBUTE_GPU_OVERLAP) == 1); }
	
	/*!
	@brief Gets the number of multiprocessors (not shader units) on the device.
	*/
	int GetMultiprocessorCount()
	{ return GetAttribute(CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT); }
	
	/*!
	@brief Determines if the device has a timeout on kernel execution.
	
	This is typically the case (display watchdog timer) on a device which is also displaying a GUI and false on a device which
	does not have a monitor attached.
	*/
	bool GetExecTimeout()
	{ return (GetAttribute(CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT) == 1); }
	
	int GetMajorVersion();
	int GetMinorVersion();
	unsigned int GetTotalMem();
	std::string GetName();
	
	void PrintDebugInfo(bool bShowAll = false);
	
	/*!
	@brief Gets the index of this device (CUDA device ID)
	*/
	int GetDeviceID()
	{ return m_nDevice; }
	
	CUjit_target_enum GetJitTarget();	
	
protected:
	/*!
	@brief Index of this device
	*/
	int m_nDevice;
	
	/*!
	@brief Device properties structure
	*/
	CUdevice m_device;
};

//Thread context
class Module;
class CudaKernel;

/*!
	@brief Thread-level context structure.
	
	One and only one instance of this class must exist in each thread which makes CUDA calls. The context MUST NOT be destroyed
	until all objects referencing it (kernels, streams, and buffers) have been destroyed.
 */
class CudaContext : public DeviceThreadContext
{
public:
	/*!
		@brief Indicates what action to take when a call must block.
	 */
	enum YieldMode
	{
		Auto = CU_CTX_SCHED_AUTO,
		Spin = CU_CTX_SCHED_SPIN,
		Yield = CU_CTX_SCHED_YIELD
	};
	
	CudaContext(Device& device, YieldMode yieldMode=Yield);
	~CudaContext();
	
	void Barrier();

	/*!
		@brief Gets the hardware configuration being targetted by JIT compilation of PTX modules.
		
		See the nVidia documentation for CUjit_target_enum for more details.
		
		@return The highest compute capability supported by this device.
	 */
	CUjit_target_enum GetJitTarget()
	{ return m_jitTarget; }

	/*!
		@brief Gets the device associated with this context.
	 */
	Device& GetDevice()
	{ return m_device; }
	
protected:
	/*!
		@brief The device associated with this context.
	 */
	Device& m_device;
	
	/*!
		@brief The hardware configuration targetted by JIT compilation.
	 */
	CUjit_target_enum m_jitTarget;
	
	/*!
		@brief The underlying CUDA driver API context structure.
	 */
	CUcontext m_context;
};

/*!
	@brief Reference to a CUDA module loaded from a cubin/PTX file.
 */
class Module
{
public:
	Module(std::string fname, CudaContext& context);
	~Module();
	
	CudaKernel* GetKernel(const char* func);
	
	/*!
		@brief Gets a reference to the underlying CUDA driver API object.
	 */
	CUmodule& GetModule()
	{ return m_module; }
	
protected:
	/*!
		@brief Our file name
	 */
	std::string m_fname;

	/*!
		@brief The underlying CUDA driver API object.
	 */
	CUmodule m_module;
};

/*!
	@brief Reference to a compiled CUDA kernel.
 */
class CudaKernel
{
public:
	/*!
		@brief Creates a CudaKernel object from a CUDA driver API handle
		
		@param func CUfunction handle from CUDA driver API
		@param desc Description of this kernel (ex: myfile.ptx!MyKernel)
	 */
	CudaKernel(CUfunction func, std::string desc)
	: m_desc(desc)
	, m_func(func)
	{
	}
	
	//TODO: informational query functions (max threads possible with this configuration, etc).
	
	/*!
		@brief Gets a reference to the underlying CUDA driver API object.
	 */
	CUfunction GetFunction() const
	{ return m_func; }
	
	/*!
		@brief Returns a description of the function this kernel object represents
	 */
	std::string GetDesc() const
	{ return m_desc; }
	
protected:
	/*!
		@brief The kernel's description (ex: myfile.ptx!MyKernel)
	 */
	std::string m_desc;

	/*!
		@brief The underlying CUDA driver API object.
	 */
	CUfunction m_func;
};

/*!
	@brief A parameter to a kernel.
	
	This interface is under review and may be changed.
 */
class KernelParamBase
{
public:
	virtual void Add(CudaKernel ker, int offset) = 0;
	virtual unsigned int GetSize() = 0;
};

/*!
	@brief A parameter to a kernel.
	
	This interface is under review and may be changed.
 */
template<class T> class KernelParam : public KernelParamBase
{
	//Unknown type, throw compiler error
	//TODO: use cuParamSetv
private:
	KernelParam(T val)
	{
	}
};

/*!
	@brief A parameter to a kernel.
	
	This interface is under review and may be changed.
 */
template<>
class KernelParam<int> : public KernelParamBase
{
public:
	KernelParam(int val)
	: m_val(val)
	{
	}
	
	virtual void Add(CudaKernel ker, int offset)
	{ cuParamSeti(ker.GetFunction(), offset, m_val); }
	
	virtual unsigned int GetSize()
	{ return sizeof(int); }
	
	int m_val;
};

/*!
	@brief A parameter to a kernel.
	
	This interface is under review and may be changed.
 */
template<>
class KernelParam<float> : public KernelParamBase
{
public:
	KernelParam(float val)
	: m_val(val)
	{
	}
	
	virtual void Add(CudaKernel ker, int offset)
	{ cuParamSetf(ker.GetFunction(), offset, m_val); }
	
	virtual unsigned int GetSize()
	{ return sizeof(float); }
	
	float m_val;
};

/*!
	@brief A parameter to a kernel.
	
	This interface is under review and may be changed.
 */
template<>
class KernelParam<DeviceMemoryBuffer> : public KernelParamBase
{
public:
	KernelParam(DeviceMemoryBuffer& val)
	: m_val(val.GetData())
	{
	}
	
	virtual void Add(CudaKernel ker, int offset)
	{ cuParamSetv(ker.GetFunction(), offset, &m_val, sizeof(m_val)); }
	
	virtual unsigned int GetSize()
	{ return sizeof(void*); }
	
	void* m_val;
};

/*!
	@brief Pointer to a page-locked buffer of host memory.
 */
class HostMemoryBuffer
{
public:
	HostMemoryBuffer(size_t size);
	~HostMemoryBuffer();

	/*!
		@brief Gets a pointer to the buffer.
	 */
	void* GetData()
	{ return m_data; }

	/*!
		@brief Gets the size of the buffer.
	 */
	size_t GetSize()
	{ return m_size; }

protected:
	/*!
		@brief The actual pointer.
	 */
	void* m_data;
	
	/*!
		@brief Size of the buffer.
	 */
	size_t m_size;
};

/*!
	@brief An event which may be added to a CUDA stream
 */
class Event
{
public:
	Event(bool bBlocking = false);
	~Event();
	
	bool Query();
	static float GetDelta(Event& e1, Event& e2);
	
	void Barrier();
	
	friend class Stream;
	
protected:
	/*!
		@brief The underlying CUDA handle
	 */
	CUevent m_event;
};

/*!
	@brief A stream of CUDA operations (kernel calls, events, and memcpys)
 */
class Stream
{
public:
	Stream();
	~Stream();
	
	//Starts profiling (will cause a significant slowdown)
	void StartProfiling();
	
	//Stops profiling and displays an execution trace
	void StopProfiling();
	
	//Adds a kernel call to the stream.
	//Params is a null-terminated array of KernelParam<> objects.
	void AddKernelCall(
		const CudaKernel& func,
		int gridx, int gridy,
		int blockx, int blocky, int blockz,
		KernelParamBase** params
		);
		
	//Adds an event to the stream.
	void AddEvent(Event& event);
		
	//Adds a device-to-host memory copy to the stream.
	//src is automatically page-locked but must be unlocked manually
	//once the stream has finished.
	void AddDtoHMemcpy(HostMemoryBuffer& dest, DeviceMemoryBuffer& src);
	
	//Adds a host-to-device memory copy to the stream.
	//dest is automatically page-locked but must be unlocked manually
	//once the stream has finished.
	void AddHtoDMemcpy(DeviceMemoryBuffer& dest, HostMemoryBuffer& src);
	
	void Barrier();
	
protected:
	/*!
		@brief Indicates if profiling is enabled.
	 */
	bool m_bProfiling;
	
	/*!
		@brief Internal profiling data
	 */
	std::vector<std::string> m_profilingDesc;
	
	/*!
		@brief Internal profiling data
	 */
	std::vector<std::string> m_profilingConfig;
	
	/*!
		@brief Internal profiling data
	 */
	std::vector<double> m_profilingTime;
	

	/*!
		@brief The underlying CUDA API object.
	 */
	CUstream m_stream;
};

#endif //#ifdef CUDA_ENABLED

#endif
