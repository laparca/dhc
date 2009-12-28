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
	@file main.cpp
	
	@brief Program entry point
 */
 
 /*!
	\mainpage
	\section intro Introduction
	This program is a distributed bruteforce cryptographic hash cracker intended primarily for password recovery. It can also
	be used as a benchmark of raw integer performance on both CPUs and nVidia GPUs.
	
	The current version only compiles for Linux, however all third-party libraries it uses are available for Windows and OS X
	and the codebase was designed with eventual porting in mind (using wrapper classes around threading APIs, etc.)
	
	The cracker was written by Andrew Zonenberg (http://www.cs.rpi.edu/~zonena/) and the members of RPISEC (http://rpisec.net).
	It is free software; its use is governed by the BSD license below:
	
	\section license License
	 Copyright (c) 2009 RPISEC.                                                  
	 All rights reserved.                                                        
	                                                                             
	 Redistribution and use in source and binary forms, with or without modifi-  
	 cation, are permitted provided that the following conditions are met:       
	                                                                             
	    * Redistributions of source code must retain the above copyright notice  
	      this list of conditions and the following disclaimer.                  
	                                                                             
	    * Redistributions in binary form must reproduce the above copyright      
	      notice, this list of conditions and the following disclaimer in the    
	      documentation and/or other materials provided with the distribution.   
	                                                                             
	    * Neither the name of RPISEC nor the names of its contributors may be    
	      used to endorse or promote products derived from this software without 
	      specific prior written permission.                                     
	                                                                             
	 THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      
	 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        
	 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     
	 NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  
	 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    
	 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      
	 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      
	 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        
	 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          
	 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                
	
	\section deps Dependencies
	TODO
	
	\section conv Conventions
	TODO
	
	\section errs Error handling
	The majority of functions do not return error values. If an error occurs, an exception of class std::string is thrown
	containing a human-readable error message, including the file and line of the source file where the error was detected.
	
	Example:
	<pre>
	Failed to open CUDA module file ptx/YUVDecode.ptx: no such file or directory
	File: /home/azonenberg/Documents/local/programming/cuvis/LowLevel.cpp
	Line: 293
	</pre>
	
	\section pholder Notes on *_placeholder.cpp files
	These empty files are automatically generated during compilation of CUDA kernels and are used by the build system.

	\section Acknowledgements
	
	TODO: Add whatever libcurl needs
 */

#include "agent.h"
#include "version.h"

using namespace std;

string g_server;
string g_hostname;

#ifdef WINDOWS
/*!
	@brief Frequency of performance timer
 */
	__int64 g_perfFreq;
#endif

/*!
	@brief Set to true if we're in unit test mode
 */
bool g_bTesting;

/*!
	@brief Set to true if we're forcing no CPU cracking
 */
bool g_bNoCPU;

/*!
	@brief Set to true if we're forcing no GPU cracking
 */
bool g_bNoGPU;

/*!
	@brief Program entry point
	
	@param argc Argument count
	@param argv Arguments
	
	@return 0 if successful, -1 in case of error
 */
int main(int argc, char* argv[])
{
	try
	{
		g_bNoCPU = false;
		g_bNoGPU = false;
		g_bTesting = false;
		
		//Check arguments
		for(int i=1; i<argc; i++)
		{
			string sarg = argv[i];
			
			if(sarg[0] == '-')
			{				
				if(sarg == "--test")
					g_bTesting = true;
				else if(sarg == "--nogpu")
					g_bNoGPU = true;
				else if(sarg == "--nocpu")
					g_bNoCPU = true;
				else if(sarg == "--version")
				{
					cout << g_version << endl;
					return 0;
				}
				else
					ThrowCustomError("Unrecognized command line argument");
			}
			else if(i == argc-1)
				g_server = argv[argc-1];	//Last argument is always the server
		}
		if(g_server == "" && !g_bTesting)
		{
			cout << "No server specified on command line, using default" << endl;
			g_server = "http://localhost/cracker/";
		}
	
		#ifdef UNIX
			//Initialize signal handler
			signal(SIGINT, OnCtrlC);
		#endif
		
		#ifdef WINDOWS
			//Initialize performance timer
			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&g_perfFreq));
		#endif
		
		//Initialize libcurl
		if(0 != curl_global_init(CURL_GLOBAL_ALL))
			ThrowError("libcurl init failed");		
		
		//Get hostname
		g_hostname = GetHostname();
		
		//Get CPU / GPU count, reserving one CPU per GPU for support
		//TODO: add command line args to force CPU only / GPU only / single device only for debugging
		int gpus = 0;
		#ifdef CUDA_ENABLED
			DriverInterface driver;
			gpus = driver.GetDeviceCount();
			if(g_bNoGPU)
				gpus = 0;
		#endif
		
		//Do tests
		if(g_bTesting)
		{
			cout << "Running unit tests..." << endl;
			
			bool result = DoUnitTests( (gpus > 0) );
			
			//Print result
			cout << "Overall status: ";
			if(result)
				cout << PASS << endl;
			else
				cout << FAIL << endl;
		}
		
		//Run normally
		else
		{
			//Select devices
			vector<ComputeDevice> devices;
			#ifdef CUDA_ENABLED
				for(int i=0; i<gpus; i++)
				{
					//Check specs
					Device tdev(i);
					float gflops = static_cast<float>(tdev.GetClockRate() * 8 * 3 * tdev.GetMultiprocessorCount()) / 1000000;
					
					//CUDA under 50 gflops isnt worth bothering with, skip it
					if(gflops < 50)
						continue;
					
					ComputeDevice dev;
					dev.bGPU = true;
					dev.index = i;
					devices.push_back(dev);
				}
			#endif
			int cpus = max(0UL, static_cast<unsigned long>(GetCpuCores() - devices.size()));
			if(g_bNoCPU)
				cpus = 0;
			
			//Temporary until we get 32 bit hashes up
			#if (X86 == 1)
				cpus = 0;
			#endif
			
			//int cpus = 0;
			for(int i=0; i<cpus; i++)
			{
				ComputeDevice dev;
				dev.bGPU = false;
				dev.index = i;
				devices.push_back(dev);
			}
			
			//Sanity check
			if(devices.size() == 0)
				ThrowCustomError("No compute devices available!");
			
			//Spawn N-1 threads for computing
			for(unsigned int i=1; i<devices.size(); i++)
				Thread mythread(ComputeThread, &devices[i]);
				
			//Run last thread locally
			ComputeThreadProc(&devices[0]);
		}
	}
	catch(std::string err)
	{
		cerr << err.c_str() << endl;
		return -1;
	}
	
	return 0;
}

/*!
	@brief SIGINT handler
	
	@param sig Always SIGINT
 */
void OnCtrlC(int sig)
{
	//Tell server we were killed
	
	//TODO: Set flag to tell threads to die and block until they do so
		
	//Quit
	exit(-1);
}

/*!
	@brief Gets the number of CPU cores
 */
int GetCpuCores()
{
#ifdef LINUX
	return sysconf( _SC_NPROCESSORS_ONLN );
#else
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#endif
}

/*!
	@brief Wrapper for ComputeThreadProc which can be invoked as a thread
	
	@param _pData See ComputeThreadProc()
 */
THREAD_PROTOTYPE(ComputeThread, _pData)
{
	//Error-handling wrapper around ComputeThreadProc
	try
	{
		ComputeThreadProc(_pData);
	}
	catch(std::string err)
	{
		cerr << err.c_str() << endl;
		exit(-1);
	}
	THREAD_RETURN(0);
}

/*!
	@brief Throws an exception of class std::string containing a nicely formatted error message.
	
	Do not call this function directly - use ThrowError() instead.
	@param err User-specified generic error message (ex: "Failed to open settings file")
	@param sys_err Human-readable form of error code (ex: "No such file or directory")
	@param file The file the error was thrown from
	@param line The line the error was thrown from
 */
void DoThrowError(const char* err, const char* sys_err, const char* file, int line)
{
	char* buf = new char[strlen(err) + strlen(sys_err) + strlen(file) + 256];
			
	//Format the error nicely
	sprintf(buf,"%s: %s\n    File: %s\n    Line: %d\n", err, sys_err, file, line);
	string serr(buf);
	delete[] buf;
	throw serr;
}

/*!
	@brief Gets the current time, in seconds.

	While this is always Unix time in the current implementation, in the future or on other platforms it may be another value
	such as clock cycles since system start. It is intended for performance measurement rather than time-of-day applications.
	
	@return Current time
 */
double GetTime()
{
#ifdef LINUX
	timespec t;
	clock_gettime(CLOCK_REALTIME,&t);
	double d = static_cast<double>(t.tv_nsec) / 1E9f;
	d += t.tv_sec;
	return d;
#elif WINDOWS
	__int64 tm;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&tm));
	double ret = tm;
	return ret / g_perfFreq;
#else
	#error Unsupported platform
#endif
}

/*!
	@brief Gets the hostname of the current machine
	
	@return Hostname as an STL string
 */
string GetHostname()
{
#if WINDOWS
	char buf[128];
	DWORD len=128;
	GetComputerNameA(buf,&len);
	return string(buf);
#elif UNIX
	char buf[128];
	FILE* fp = popen("hostname","r");
	if(NULL==fgets(buf,127,fp))
		return "box";
	buf[strlen(buf)-1]='\0';	//trim off trailing newline
	pclose(fp);
	return string(buf);
#else
	#error Unsupported platform
#endif
}
