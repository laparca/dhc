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
	@file LowLevel.cpp
	
	@brief Implementation of low-level CUDA routines
 */
 
#include "agent.h"
#include <fstream>
using namespace std;

#ifdef CUDA_ENABLED
///////////////////////////////////////////////////////////////////////////////
//Error handling

/*!
	@brief "Constructor" macro for StringErrPair objects
 */
#define MakeSIP(s) { #s, s }

/*!
	@brief Internal error-string structure
 */
struct StringErrPair
{
	const char* err;
	CUresult code;
};

/*!
	@brief Number of strings in ::g_errorStrings
 */
#define CU_ERR_COUNT	28

/*!
	@brief Map of CUDA driver API error codes to human-readable names.
	
	Used by LLGetErrorString().
 */
static StringErrPair g_errorStrings[CU_ERR_COUNT]=
{
	MakeSIP(CUDA_SUCCESS),
	MakeSIP(CUDA_ERROR_INVALID_VALUE),
	MakeSIP(CUDA_ERROR_OUT_OF_MEMORY),
	MakeSIP(CUDA_ERROR_OUT_OF_MEMORY),
	MakeSIP(CUDA_ERROR_NOT_INITIALIZED),
	MakeSIP(CUDA_ERROR_DEINITIALIZED),
	MakeSIP(CUDA_ERROR_NO_DEVICE),
	MakeSIP(CUDA_ERROR_INVALID_DEVICE),
	MakeSIP(CUDA_ERROR_INVALID_IMAGE),
	MakeSIP(CUDA_ERROR_INVALID_CONTEXT),
	MakeSIP(CUDA_ERROR_CONTEXT_ALREADY_CURRENT),
	MakeSIP(CUDA_ERROR_MAP_FAILED),
	MakeSIP(CUDA_ERROR_UNMAP_FAILED),
	MakeSIP(CUDA_ERROR_ARRAY_IS_MAPPED),
	MakeSIP(CUDA_ERROR_ALREADY_MAPPED),
	MakeSIP(CUDA_ERROR_NO_BINARY_FOR_GPU),
	MakeSIP(CUDA_ERROR_ALREADY_ACQUIRED),
	MakeSIP(CUDA_ERROR_NOT_MAPPED),
	MakeSIP(CUDA_ERROR_INVALID_SOURCE),
	MakeSIP(CUDA_ERROR_FILE_NOT_FOUND),
	MakeSIP(CUDA_ERROR_INVALID_HANDLE),
	MakeSIP(CUDA_ERROR_NOT_FOUND),
	MakeSIP(CUDA_ERROR_NOT_READY),
	MakeSIP(CUDA_ERROR_LAUNCH_FAILED),
	MakeSIP(CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES),
	MakeSIP(CUDA_ERROR_LAUNCH_TIMEOUT),
	MakeSIP(CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING),
	
	//Must be last error
	MakeSIP(CUDA_ERROR_UNKNOWN)
};

const char* LLGetErrorString(CUresult code)
{
	//Check for error
	for(int i=0; i<CU_ERR_COUNT; i++)
	{
		if(g_errorStrings[i].code == code)
			return g_errorStrings[i].err;
	}
	
	//Not found? Return CUDA_ERROR_UNKNOWN
	return g_errorStrings[CU_ERR_COUNT-1].err;
};

///////////////////////////////////////////////////////////////////////////////
//DriverInterface

/*!
	@brief Initializes the CUDA driver API.
 */
DriverInterface::DriverInterface()
{
	//Initialize driver
	CUresult err;
	if(CUDA_SUCCESS != (err=cuInit(0)))
		ThrowCudaLLError("Failed to initialize CUDA driver",err);
}

/*!
	@brief Gets the number of CUDA devices on the system.
 */
int DriverInterface::GetDeviceCount()
{
	int count;
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDeviceGetCount(&count)))
		ThrowCudaLLError("Failed to get CUDA device count", err);
	return count;	
}

/*!
	@brief Gets the version number of the CUDA driver.
	
	@return Internal CUDA driver version. Note that this is NOT the xxx.xx.x display driver version string.
 */
int DriverInterface::GetDriverVersion()
{
	int ver;
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDriverGetVersion(&ver)))
		ThrowCudaLLError("Failed to get CUDA driver version", err);
	return ver;
}

///////////////////////////////////////////////////////////////////////////////
//Device

/*!
	@brief Initializes a Device object.
	
	@param iDevice CUDA device ID (0 to n-1)
 */
Device::Device(int iDevice)
: m_nDevice(iDevice)
{
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDeviceGet(&m_device, iDevice)))
		ThrowCudaLLError("Failed to open CUDA device", err);
}

/*!
	@brief Prints all attributes to stdout.
	
	@param bShowAll Set to true to show less commonly used values (warp size, max pitch, texture alignment).
 */
void Device::PrintDebugInfo(bool bShowAll)
{
	cout << GetName().c_str() << endl;
	cout << "          Compute capability      " << GetMajorVersion() << "." << GetMinorVersion() << endl;
	cout << "          Max threads per block   " << GetMaxThreadsPerBlock() << endl;
	cout << "          Max block size          (" << GetMaxBlockX() << ", " << GetMaxBlockY() << ", " << GetMaxBlockZ() << ")" << endl;
	cout << "          Max grid size           (" << GetMaxGridX() << ", " << GetMaxGridY() << ", " << GetMaxGridZ() << ")" << endl;
	cout << "          Global memory size (MB) " << GetTotalMem() / (1024*1024) << endl;
	cout << "          Max shmem               " << GetMaxShmem() << endl;
	cout << "          Max cmem                " << GetMaxCmem() << endl;
	if(bShowAll)
	{
		cout << "          Warp size               " << GetWarpSize() << endl;
		cout << "          Max pitch               " << GetMaxPitch() << endl;
	}
	cout << "          Max regs per block      " << GetMaxRegisters() << endl;
	cout << "          Clock rate (MHz)        " << GetClockRate() / 1000 << endl;
	if(bShowAll)
		cout << "          Texture alignment       " << GetTextureAlignment() << endl;
	cout << "          GPU overlap             " << (GetGpuOverlap()?"yes":"no") << endl;
	cout << "          Multiprocessor count    " << GetMultiprocessorCount() << endl;
	cout << "          Execution time limit    " << GetExecTimeout() << endl;
}

/*!
	@brief Gets the value of a device attribute.
	
	@param attrib CUDA device attribute code (see documentation for cuDeviceGetAttribute() in the CUDA driver API). Unless
	querying new attributes not directly supported by the current libcuvis version, the specialized Get*() functions should be
	used instead.
	
	@return Value of the attribute.
 */
int Device::GetAttribute(CUdevice_attribute attrib)
{
	CUresult err;
	int ret;
	if(CUDA_SUCCESS != (err=cuDeviceGetAttribute(&ret, attrib, m_device)))
		ThrowCudaLLError("Failed to get CUDA device attribute", err);
	return ret;
}

/*!
	@brief Gets a descriptive name for this device
	
	@return Name of the device (e.g. "GeForce GTX 285")
 */
std::string Device::GetName()
{
	char buf[128];
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDeviceGetName(buf, 127, m_device)))
		ThrowCudaLLError("Failed to get CUDA device name", err);
	return string(buf);
}

/*!
	@brief Gets the major version of the device's compute capability
 */
int Device::GetMajorVersion()
{
	int maj,min;
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDeviceComputeCapability(&maj, &min, m_device)))
		ThrowCudaLLError("Failed to get CUDA device version", err);
	return maj;
}

/*!
	@brief Gets the minor version of the device's compute capability
 */
int Device::GetMinorVersion()
{
	int maj,min;
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDeviceComputeCapability(&maj, &min, m_device)))
		ThrowCudaLLError("Failed to get CUDA device version", err);
	return min;
}

/*!
	@brief Gets the total global memory available on the device, in bytes.
 */
unsigned int Device::GetTotalMem()
{
	unsigned int mem;
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDeviceTotalMem(&mem, m_device)))
		ThrowCudaLLError("Failed to get CUDA device memory size", err);
	return mem;
}

/*!
	@brief Gets the GPU architecture targetted by JIT compilation.
 */
CUjit_target_enum Device::GetJitTarget()
{
	int maj,min;
	CUresult err;
	if(CUDA_SUCCESS != (err=cuDeviceComputeCapability(&maj, &min, m_device)))
		ThrowCudaLLError("Failed to get CUDA device version", err);
		
	if(maj>1)							//Future device? Target highest we know about (1.3)
		return CU_TARGET_COMPUTE_13;
	else
	{
		switch(min)
		{
		case 0:
			return CU_TARGET_COMPUTE_10;
		case 1:
			return CU_TARGET_COMPUTE_11;
		case 2:
			return CU_TARGET_COMPUTE_12;
		case 3:
			return CU_TARGET_COMPUTE_13;
		default:						//Future device? Target highest we know about (1.3)
			return CU_TARGET_COMPUTE_13;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//CudaContext

/*!
	@brief Initializes a thread context.
	
	@param device The CUDA device to bind this thread to
	@param yieldMode Indicates the action to take on a blocking call: spinlock, yield its time slice, or let the driver decide.
 */
CudaContext::CudaContext(Device& device, YieldMode yieldMode)
: m_device(device)
{
	CUresult err;
	if(CUDA_SUCCESS != (err=cuCtxCreate(&m_context, yieldMode, device.GetDeviceID())))
		ThrowCudaLLError("Failed to create CUDA context", err);
		
	//Save JIT target device
	m_jitTarget = device.GetJitTarget();
}

/*!
	@brief Destroys a CUDA context.
	
	The context MUST NOT be destroyed until all objects referencing it (kernels, streams, and buffers) have been destroyed.
 */
CudaContext::~CudaContext()
{
	cuCtxDetach(m_context);
}

/*!
	@brief Blocks until all CUDA operations in this thread have completed.
 */
void CudaContext::Barrier()
{
	CUresult err;
	if(CUDA_SUCCESS != (err=cuCtxSynchronize()))
		ThrowCudaLLError("Failed to create CUDA context", err);
}

///////////////////////////////////////////////////////////////////////////////
//Module

/*!
	@brief Loads a CUDA module from a .cubin or .ptx file.
	
	The file is first searched for in the current working directory. If it is not found,
	"/usr/share/cuvis/" is prepended to the file name. If this fails, an exception is thrown.
	
	@param fname Name of the file
	@param context The current thread context
 */
Module::Module(std::string fname, CudaContext& context)
{
	//Save our file name
	m_fname = fname;
	if(fname.find("ptx/") == 0)
		m_fname = fname.substr(4);
	
	//Find the file
	FILE* fp = fopen(fname.c_str(), "r");
	if(fp)
		fclose(fp);
	else
	{
		//Not found? Try checking a few other common spots
		string sfname = string("/usr/share/cuvis/") + fname;
		fp = fopen(sfname.c_str(), "r");
		if(fp)
		{
			fclose(fp);
			fname = sfname;
		}
		
		//TODO: check other common spots
	}
	
	//Read the code
	string code;
	ifstream myfile(fname.c_str());
	if(!myfile)
	{
		string strErr = string("Failed to open CUDA module file ") + fname;
		ThrowCustomError(strErr.c_str());
	}
	
	char line[1024];
	while(!myfile.eof())
	{
		myfile.getline(line, 1024);
		code += line;
		code += "\n";
	}
	
	//Set up build flags
	const int numOptions = 3;
	int maxregs = 128;
	float buildtime;
	//CUjit_target_enum target = context.GetJitTarget();
	int optimization = 4;
	CUjit_option options[numOptions]=
	{
		CU_JIT_MAX_REGISTERS,
		CU_JIT_WALL_TIME,
		CU_JIT_OPTIMIZATION_LEVEL
	};
	void* values[numOptions]=
	{
		&maxregs,
		&buildtime,
		&optimization
	};
	
	//TODO: other parameters (including target)
	
	//Load the module
	CUresult err;
	if(CUDA_SUCCESS != (err=cuModuleLoadDataEx(&m_module, code.c_str(), numOptions, options, values)))
		ThrowCudaLLError("Failed to JIT CUDA module", err);
}

/*!
	@brief Unloads a module.
 */
Module::~Module()
{
	cuModuleUnload(m_module);
}

/*!
	@brief Gets a kernel from this module.
	
	The kernel MUST be declared with extern "C" linkage to avoid using decorated names in the module.
		
	@param funcname Name of the kernel.
	@return A new CudaKernel object. You must call delete on this pointer when you are finished using it.
 */
CudaKernel* Module::GetKernel(const char* funcname)
{
	string desc = m_fname + "!" + funcname;
	
	CUfunction func;
	CUresult err;
	if(CUDA_SUCCESS != (err=cuModuleGetFunction(&func, m_module, funcname)))
	{
		string strErr = string("Failed to load device function ") + funcname;
		ThrowCudaLLError(strErr.c_str(), err);
	}
	return new CudaKernel(func, desc);
}

///////////////////////////////////////////////////////////////////////////////
//HostMemoryBuffer
/*!
	@brief Allocates a block of page-locked memory on the host.
	
	Note: Normal pageable memory (allocated with malloc / new) cannot be used in a streaming memcpy. To use data from a pageable
	block in a stream, it must be copied to page-locked memory first.
	
	@param size Size of the buffer to allocate.
 */
HostMemoryBuffer::HostMemoryBuffer(size_t size)
: m_size(size)
{
	CUresult err;
	if(CUDA_SUCCESS != (err=cuMemAllocHost(&m_data, size)))
		ThrowCudaLLError("Failed to allocate host memory", err);
}

/*!
	@brief Frees the buffer.
 */
HostMemoryBuffer::~HostMemoryBuffer()
{
	cuMemFreeHost(m_data);
}

///////////////////////////////////////////////////////////////////////////////
//Event

/*!
	@brief Creates an event
	
	@param bBlocking True if this event should use blocking synchronization
 */
Event::Event(bool bBlocking)
{
	CUresult err = CUDA_SUCCESS;
	if(CUDA_SUCCESS != (err = cuEventCreate(&m_event, bBlocking ? CU_EVENT_BLOCKING_SYNC : CU_EVENT_DEFAULT)))
		ThrowCudaLLError("Failed to create event", err);
}

/*!
	@brief Destroys the event
 */
Event::~Event()
{
	cuEventDestroy(m_event);
}

/*!
	@brief Gets the time delta between two events
	
	Both events must have been recorded before this function is called.
	
	@param e1 The start event
	@param e2 The end event
	
	@return Time between e1 and e2 in ms, with a resolution of around 0.5 us
 */
float Event::GetDelta(Event& e1, Event& e2)
{
	float ret = 0;
	
	CUresult err = CUDA_SUCCESS;
	if(CUDA_SUCCESS != cuEventElapsedTime(&ret, e1.m_event, e2.m_event))
		ThrowCudaLLError("Failed to get event execution time delta", err);
		
	return ret;
}

/*!
	@brief Determines if this event has been executed on the device.
	
	This event must have been submitted to a stream before this function is called.
	
	@return True if this event has been executed.
 */
bool Event::Query()
{
	CUresult res = cuEventQuery(m_event);
	
	//Done?
	if(res == CUDA_SUCCESS)
		return true;
		
	//Not done?
	else if(res == CUDA_ERROR_NOT_READY)
		return false;

	//Something's wrong
	else
	{
		ThrowCudaLLError("Failed to query event completion", res);
		return false;
	}
}

/*!
	@brief Blocks until this event has actually been recorded.
	
	The CUDA 2.2 documentation is a little unclear on error values (some contradictions are apparent in the notes)
	so the return value of cuEventSynchronize() is currently ignored.
	
 */
void Event::Barrier()
{
	cuEventSynchronize(m_event);
}

///////////////////////////////////////////////////////////////////////////////
//Stream

/*!
	@brief Creates a new stream object in the current thread.
 */
Stream::Stream()
: m_bProfiling(false)
{
	CUresult res = cuStreamCreate(&m_stream, 0);
	if(CUDA_SUCCESS != res)
		ThrowCudaLLError("Failed to create stream", res);
}

/*!
	@brief Destroys a stream.
 */
Stream::~Stream()
{
	cuStreamDestroy(m_stream);
}

/*!
	@brief Starts profiling.
	
	In the current libcuvis version this will cause a significant slowdown as all stream operations
	will block until they are finished.
 */
void Stream::StartProfiling()
{
	m_bProfiling = true;
}

/*!
	@brief Stops profiling and displays an execution trace on stdout.
 */
void Stream::StopProfiling()
{
	m_bProfiling = false;
	
	printf("\n\n%-80s %20s %12s\n", "Description", "Configuration", "Time");
	for(unsigned int i=0; i<m_profilingDesc.size(); i++)
		printf("%-80s %-20s %9.2f ms\n", m_profilingDesc[i].c_str(), m_profilingConfig[i].c_str(), m_profilingTime[i] * 1000);
	
	m_profilingDesc.clear();
	m_profilingConfig.clear();
	m_profilingTime.clear();
}

/*!
	@brief Adds an event to the stream.
	
	@param event The event to add
 */
void Stream::AddEvent(Event& event)
{
	CUresult err;
	if(CUDA_SUCCESS != (err=cuEventRecord(event.m_event, m_stream)))
		ThrowCudaLLError("Failed to barrier on stream", err);
}

/*!
	@brief Blocks until all operations in this stream have completed.
 */
void Stream::Barrier()
{
	CUresult err;
	if(CUDA_SUCCESS != (err=cuStreamSynchronize(m_stream)))
		ThrowCudaLLError("Failed to barrier on stream", err);
}

/*!
	@brief Appends a kernel call to this stream.
	
	This is a nonblocking function.
	
	@param func Reference to the kernel being called
	@param gridx Number of thread blocks along the X axis of the grid
	@param gridy Number of thread blocks along the Y axis of the grid
	@param blockx Number of threads per block along the X axis
	@param blocky Number of threads per block along the Y axis
	@param blockz Number of threads per block along the Z axis
	@param params Parameters (null-terminated array of KernelParam<T>* pointers).
 */
void Stream::AddKernelCall(
		const CudaKernel& func,
		int gridx, int gridy,
		int blockx, int blocky, int blockz,
		KernelParamBase** params
		)
{
	Event start;
	if(m_bProfiling)
		AddEvent(start);
	
	//Set up the grid
	CUresult err;
	if(CUDA_SUCCESS != (err=cuFuncSetBlockShape(func.GetFunction(), blockx, blocky, blockz)))
		ThrowCudaLLError("Failed to set kernel block shape", err);
	
	//Add parameters, stopping on NULL
	unsigned int size = 0;
	int paramcount = 0;
	if(params != NULL)
	{
		//Compute parameter size in bytes.
		//Keep going till we hit a limit (no more than 256 params allowed)
		for(int i=0; i<256; i++)
		{
			if(params[i]==0)
				break;
			size += params[i]->GetSize();
			paramcount++;
		}
		
		//Save parameter size
		cuParamSetSize(func.GetFunction(), size);
		
		//Add the parameters
		int offset = 0;
		for(int i=0; i<paramcount; i++)
		{
			params[i]->Add(func, offset);
			offset += params[i]->GetSize();
		}
	}
	
	//Launch the kernel
	if(CUDA_SUCCESS != (err=cuLaunchGridAsync(
		func.GetFunction(),
		gridx,
		gridy,
		m_stream)))
	{
		ThrowCudaLLError("Failed to launch kernel", err);
	}
	
	//Profiling
	if(m_bProfiling)
	{
		//Wait for operation to finish
		Event end;
		AddEvent(end);
		end.Barrier();
		
		//Get elapsed time
		double dt = static_cast<double>(Event::GetDelta(start, end)) / 1000;
		
		//Add data
		char tconfig[128] = "\0";
		sprintf(tconfig, "%d x %d, %d x %d x %d", gridx, gridy, blockx, blocky, blockz);
		char config[128];
		sprintf(config, "%20s", tconfig);
		m_profilingDesc.push_back(func.GetDesc());
		m_profilingConfig.push_back(config);
		m_profilingTime.push_back(dt);
	}
}

/*!
	@brief Adds a device-to-host memory copy to the stream.
	
	This is a non-blocking operation.
	
	@param dest Destination buffer.
	@param src Source buffer.
 */
void Stream::AddDtoHMemcpy(HostMemoryBuffer& dest, DeviceMemoryBuffer& src)
{
	Event start;
	if(m_bProfiling)
		AddEvent(start);
	
	//Sanity check
	if(dest.GetSize() < src.GetSize())
		ThrowCustomError("Failed to start device-to-host memcpy: destination buffer too small");
	
	//Nasty casting required to deal with pointer truncation on devices
	//with 32-bit pointers
	CUresult err;
	if(CUDA_SUCCESS != (err=cuMemcpyDtoHAsync(
		dest.GetData(),
		static_cast<CUdeviceptr>(reinterpret_cast<intptr_t>(src.GetData())),
		src.GetSize(),
		m_stream)))
	{
		ThrowCudaLLError("Failed to start device-to-host memcpy", err);
	}

	//Profiling
	if(m_bProfiling)
	{
		//Wait for operation to finish
		Event end;
		AddEvent(end);
		end.Barrier();
		
		//Get elapsed time
		double dt = static_cast<double>(Event::GetDelta(start, end)) / 1000;
		
		//Add data
		char config[128];
		sprintf(config, "%17.2f KB", static_cast<float>(src.GetSize())/1024);
		m_profilingDesc.push_back("Device-to-host memcpy");
		m_profilingConfig.push_back(config);
		m_profilingTime.push_back(dt);
	}
}

/*!
	@brief Adds a host-to-device memory copy to the stream.
	
	This is a non-blocking operation.
	
	@param dest Destination buffer.
	@param src Source buffer.
 */
void Stream::AddHtoDMemcpy(DeviceMemoryBuffer& dest, HostMemoryBuffer& src)
{
	Event start;
	if(m_bProfiling)
		AddEvent(start);
	
	//Sanity check
	if(dest.GetSize() < src.GetSize())
		ThrowCustomError("Failed to start host-to-device memcpy: destination buffer too small");
	
	//Nasty casting required to deal with pointer truncation on devices
	//with 32-bit pointers
	CUresult err;
	if(CUDA_SUCCESS != (err=cuMemcpyHtoDAsync(
		static_cast<CUdeviceptr>(reinterpret_cast<intptr_t>(dest.GetData())),
		src.GetData(),
		src.GetSize(),
		m_stream)))
	{
		ThrowCudaLLError("Failed to start device-to-host memcpy", err);
	}
	
	//Profiling
	if(m_bProfiling)
	{
		//Wait for operation to finish
		Event end;
		AddEvent(end);
		end.Barrier();
		
		//Get elapsed time
		double dt = static_cast<double>(Event::GetDelta(start, end)) / 1000;
		
		//Add data
		char config[128];
		sprintf(config, "%17.2f KB", static_cast<float>(src.GetSize())/1024);
		m_profilingDesc.push_back("Host-to-device memcpy");
		m_profilingConfig.push_back(config);
		m_profilingTime.push_back(dt);
	}
}


#endif //#ifdef CUDA_ENABLED
